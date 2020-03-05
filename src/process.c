// Process.c is the application layer.  All function calls are hardware ambivalent.
//
#include <stdio.h>
#include <string.h> 
#include <stdint.h>                // Find integer types like "uint8_t"  
#include "Eve2_81x.h"              // Matrix Orbital Eve2 Driver
#include "Raspberry_AL.h"            // include the hardware specific abstraction layer header for the specific hardware in use.
#include "MatrixEve2Conf.h"        // Header for EVE2 Display configuration settings
#include "process.h"               // Every c file has it's header and this is the one for this file

// The size of the buffer we use for data transfers.  It is a tiny buffer in Arduino Uno
#define COPYBUFSIZE WorkBuffSz
#define NUM_IMAGES                 8
#define LINE_LIMIT                35     // should never exceed 33

uint32_t Time2CheckTouch = 0;            // "Private" variable holding time of next check for user input
uint32_t PressTimeout = 0;               // "Private" variable counting down time you can spend pressing the screen

void MakeScreen_Bitmap_RGB(uint32_t FlashAddress, uint32_t RAMAddress, uint32_t size)
{
  Cmd_Flash_Read(RAMAddress, FlashAddress, size);
  
  Send_CMD(CMD_DLSTART);
  Send_CMD(CLEAR(1,1,1));
  
  Send_CMD(BEGIN(BITMAPS));
  Send_CMD(BITMAP_SOURCE(0L));
  Send_CMD(BITMAP_LAYOUT(RGB565, 480L*2, 272));
  Send_CMD(BITMAP_SIZE(NEAREST, BORDER, BORDER, 480, 272));
  Send_CMD(VERTEX2F(0, 0));
  Send_CMD(END()); 
  Send_CMD(DISPLAY());
  Send_CMD(CMD_SWAP);
  UpdateFIFO();                                      
}

void MakeScreen_Bitmap_JPEG(uint32_t FlashAddress, uint32_t RAMAddress, uint32_t size)
{
  Send_CMD(CMD_FLASHSOURCE);
  Send_CMD(FlashAddress);
  Send_CMD(CMD_LOADIMAGE);
  Send_CMD(RAMAddress);
  Send_CMD(OPT_FLASH);
  
  UpdateFIFO();                                                       // Trigger the CoProcessor to start processing commands out of the FIFO
  Wait4CoProFIFOEmpty();                                              // wait here until the coprocessor has read and executed every pending command.

  Send_CMD(CMD_DLSTART);
  Cmd_SetRotate(2);
  Send_CMD(CLEAR(1,1,1));
  Send_CMD(BEGIN(BITMAPS));
  //Cmd_SetRotate(2);
  Send_CMD(BITMAP_SOURCE(0L));
  Send_CMD(BITMAP_LAYOUT(RGB565, 272L*2, 480));
  Send_CMD(BITMAP_SIZE(NEAREST, BORDER, BORDER, 272, 480));
  Send_CMD(VERTEX2F(16 * 208, 0));
  Send_CMD(END()); 
  Send_CMD(DISPLAY());
  Send_CMD(CMD_SWAP);
  UpdateFIFO();                                      
}

bool FlashAttach(void)
{
  Send_CMD(CMD_FLASHATTACH);
  UpdateFIFO();                                                       // Trigger the CoProcessor to start processing commands out of the FIFO
  Wait4CoProFIFOEmpty();                                              // wait here until the coprocessor has read and executed every pending command.

  uint8_t FlashStatus = rd8(REG_FLASH_STATUS + RAM_REG);
  if (FlashStatus != FLASH_STATUS_BASIC)
  {
    return false;
  }
  return true;
}

bool FlashDetach(void)
{
  Send_CMD(CMD_FLASHDETACH);
  UpdateFIFO();                                                       // Trigger the CoProcessor to start processing commands out of the FIFO
  Wait4CoProFIFOEmpty();                                              // wait here until the coprocessor has read and executed every pending command.

  uint8_t FlashStatus = rd8(REG_FLASH_STATUS + RAM_REG);
  if (FlashStatus != FLASH_STATUS_DETACHED)
  {
    Log("FlashDetach: NOT detached\n");
    return false;
  }
  return true;
}

bool FlashFast(void)
{
  Cmd_Flash_Fast();
  UpdateFIFO();                                                       // Trigger the CoProcessor to start processing commands out of the FIFO
  Wait4CoProFIFOEmpty();                                              // wait here until the coprocessor has read and executed every pending command.
  
  uint8_t FlashStatus = rd8(REG_FLASH_STATUS + RAM_REG);
  if (FlashStatus != FLASH_STATUS_FULL)
  {
    Log("FlashFast: NOT full mode\n");
    return false;
  }
  return true;
}

bool FlashErase(void)
{
  Log("FlashErase: Erasing Flash\n");
  Send_CMD(CMD_FLASHERASE);
  UpdateFIFO();                                                       // Trigger the CoProcessor to start processing commands out of the FIFO
  Wait4CoProFIFOEmpty();                                              // wait here until the coprocessor has read and executed every pending command.
  Log("FlashErase: Finished Erase\n");
}

// Load the output.bin file into flash if it exists
// Return indicates whether it was done or not
bool FlashLoad(char *filename)
{
  // upload file to flash
  if( !FileTransfer2Flash(filename, RAM_FLASH)) {
    Log("FlashLoad: File Operation Failed\n");
    return false;
  }
  return true;
}

// FileTransfer2Flash
// Transfer a file from SD card to Eve attached flash
// FlashAddress should be on a 4K boundary (multiple of 0x1000) - this is really important
bool FileTransfer2Flash(char *filename, uint32_t FlashAddress)
{
  uint8_t FlashStatus;
  uint32_t FlashSize;
  
  printf("In FileTransfer2Flash\n");
  //Read bin file
  FILE* fp = fopen(filename, "rb");

  if (fp) {
    printf("Opened File\n"); 
    // Read file size
    printf("Seeking to end\n");
    fseek(fp, 0, SEEK_END);
    uint32_t Remaining = ftell(fp);
    printf("Rewinding\n");
    rewind(fp);
    uint32_t FileSectors = Remaining / 0x1000;                        // Number of 4K sectors required to store file
    printf("FOUND %s, SIZE: %d\n", filename, Remaining);

    if(Remaining % 0x1000)                                            // If the file is not on 4K boundary (and when is it?)
      FileSectors++;                                                  // Add a 4K sector for the partial
    
    // Calculate file fitting and number of 4K blocks
    FlashSize = rd32(REG_FLASH_SIZE + RAM_REG) * 0x100000;            // A flash of 8M bytes which will report size = 8
    uint32_t SectorsAvailable = (FlashSize - (FlashAddress - RAM_FLASH)) / 0x1000;    // Number of 4K sectors available in the flash chip from the requested base address
    if ( FileSectors > SectorsAvailable )
    {
      printf("FT2F: File does not fit\n");
      return false;                                                   // We can not continue without flash space to store file
    }

    uint32_t BufPerSector = 0x1000 / COPYBUFSIZE;
    uint16_t chunk_size = 256;
    uint8_t file_buf[chunk_size];
    // Write file in sectors to RAM_G at the working RAM space (RAM_G_WORKING)
    for (uint16_t h = 0; h < FileSectors; h++ )                           // Loop through file data in 4K chunks until all 4K chunks have been processed
    {
      //printf("Remaining: %d\n", Remaining);
      
      for (uint16_t i = 0; i < 0x1000 / chunk_size; i++) {
        if (Remaining >= chunk_size) {
          FileReadBuf(fp, file_buf, chunk_size);
        } else {
          printf("Writing Last Chunk with Padding\n");
          for ( uint16_t j = 0; j < chunk_size; j++ ) {
            if(Remaining) {
              file_buf[j] = FileReadByte(fp);
              Remaining--;
            } else {
              file_buf[j] = 0xFF;
            }
          }
        }
        WriteBlockRAM(RAM_G_WORKING + (i * chunk_size), file_buf, chunk_size);
        /*Send_CMD(CMD_FLASHWRITE);
        Send_CMD(chunk_size*i + FlashAddress + ((uint32_t)h * 0x1000));
        Send_CMD(chunk_size);
        wrx(FifoWriteLocation + RAM_CMD, chunk_size, file_buf);
        FifoWriteLocation += chunk_size;                                // Increment the Write Address by the size of a command - which we just sent
        FifoWriteLocation %= FT_CMD_FIFO_SIZE;                           // Wrap the address to the FIFO space
        UpdateFIFO();                                                       // Trigger the CoProcessor to start processing commands out of the FIFO
        Wait4CoProFIFOEmpty();                                              // wait here until the coprocessor has read and executed every pending command.
        */Remaining -= chunk_size;
      }
      Send_CMD(CMD_FLASHUPDATE);                                          // Transfer data from RAM_G into flash automatically erasing 4K sectors as required
      Send_CMD(FlashAddress + ((uint32_t)h * 0x1000));                    // Destination address in Flash 
      Send_CMD(RAM_G_WORKING);                                            // Source address in RAM_G
      Send_CMD(0x1000);                                                   // Number of bytes to transfer (4096)
      UpdateFIFO();                                                       // Trigger the CoProcessor to start processing commands out of the FIFO
      Wait4CoProFIFOEmpty();
    }
  } else {
    printf("File Error!!!\n");
  }
  fclose(fp);
  return true;
}

