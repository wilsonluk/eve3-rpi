#include <bcm2835.h>
#include <stdio.h>
#include <string.h> 
#include <stdbool.h>
#include "Eve2_81x.h"           
#include "MatrixEve2Conf.h"      // Header for EVE2 Display configuration settings
#include "process.h"
#include "Raspberry_AL.h"

//File myFile;
char LogBuf[WorkBuffSz];

int main(int argc, char **argv)
{
  // Initializations.  Order is important
  if (GlobalInit() != 0) return 1;
  printf("Global Init Finished\n");
  FT81x_Init();
  printf("FT81X Init Finished\n");

  //Attach and Erase the onboard flash
  FlashAttach();                           // Attach flash
  FlashErase();
  
  //Load image(s) onto the onboard flash
  FlashLoad("./images/mudkip.raw");
  
  //Set the Onboard flash to faster QSPI mode
  if (!FlashFast()) {
    printf("Could not set Flash to fast QSPI mode!\n");
    return 1;
  }
  
  //Set Brightness (0-127) and Backlight PWM Freq (250-10000)
  wr8(REG_PWM_DUTY + RAM_REG, 128);
  wr16(REG_PWM_HZ + RAM_REG, 2000);
  
  //Offset of 4096 Bytes for blob file
  uint32_t imageAddress = 4096;
  MakeScreen_Bitmap_RGB(imageAddress, RAM_G, 261120);
  
  
  /*FlashLoad();                           // Copy output.bin to Eve attached flash
  printf("copied to flash\n");

  if(!FlashFast())                         // Set flash to fast mode - QSPI
  {
    FlashLoad();                           // Copy output.bin to Eve attached flash
  }

  wr8(REG_PWM_DUTY + RAM_REG, 128);        // set backlight

  if ( FlashGetFileParms() )               // Stuff the file list buffer from flash
  {
    uint8_t Index = 2;                     // The first two files that we parse are always the blob and the map
    while(1)                               // display the pictures in output.bin sequentially
    {
      MakeScreen_Bitmap(Index++);
      if(Index > 7) Index = 2;
      MyDelay(2000);
    }
  }
  else
  {
    printf("Bad flash parsing\n");
    while(1);                              // Can not continue if the flash is not right (unless this is a thing and you could reload here potentially)   
  }*/
}

//Adapted from the Arduino Library

int GlobalInit(void)
{
  if (!bcm2835_init()){
    printf("bcm2835_init failed. Are you running as root??\n");
    return 1;
  }

  bcm2835_spi_end();
  MyDelay(2000);

  if (!bcm2835_spi_begin()) {
    printf("bcm2835_spi_begin failed. Are you running as root??\n");
    return 1;
  }

  Eve_Reset_HW();

  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // Set MSB First
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // SPI Mode 0

  bcm2835_gpio_fsel(EvePDN_PIN, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_write(EvePDN_PIN, LOW);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2048); // Set the Clock Divider to

  bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
  bcm2835_gpio_fsel(EveChipSelect_PIN, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_write(EveChipSelect_PIN, HIGH);
}

// Send a single byte through SPI
void SPI_WriteByte(uint8_t data)
{
  bcm2835_gpio_write(EveChipSelect_PIN, LOW);
  bcm2835_spi_transfer(data);
  bcm2835_gpio_write(EveChipSelect_PIN, HIGH);
}

// Send a series of bytes (contents of a buffer) through SPI
void SPI_WriteBuffer(uint8_t *Buffer, uint32_t Length)
{
  bcm2835_gpio_write(EveChipSelect_PIN, LOW);
  bcm2835_spi_transfern(Buffer, Length);
  bcm2835_gpio_write(EveChipSelect_PIN, HIGH);
}

// Send a byte through SPI as part of a larger transmission.  Does not enable/disable SPI CS
void SPI_Write(uint8_t data)
{
  bcm2835_spi_transfer(data);
}

// Read a series of bytes from SPI and store them in a buffer
void SPI_ReadBuffer(uint8_t *Buffer, uint32_t Length)
{
  uint8_t a = bcm2835_spi_transfer(0x0); // dummy read
  while (Length--)
  {
    uint8_t temp = bcm2835_spi_transfer(0x0);
    *(Buffer++) = temp;
  }
}

// Enable SPI by activating chip select line
void SPI_Enable(void)
{
  bcm2835_gpio_write(EveChipSelect_PIN, LOW);
}

// Disable SPI by deasserting the chip select line
void SPI_Disable(void)
{
  bcm2835_gpio_write(EveChipSelect_PIN, HIGH);
}

void Eve_Reset_HW(void)
{
  // Reset Eve
  bcm2835_gpio_write(EvePDN_PIN, LOW);  
  MyDelay(300);                              // delay
  bcm2835_gpio_write(EvePDN_PIN, HIGH);
  MyDelay(300);                              // delay
}

// A millisecond delay wrapper for the Arduino function
void MyDelay(uint32_t DLY)
{
   bcm2835_delay(DLY);
}

//================================== File Reading Functions ====================================

// Read the touch digitizer calibration matrix values from the Eve and write them to a file
void SaveTouchMatrix(void)
{
  /*uint8_t count = 0;
  uint32_t data;
  uint32_t address = REG_TOUCH_TRANSFORM_A + RAM_REG;*/
  
//  Log("Enter SaveTouchMatrix\n");
  
  // If the file exists already from previous run, then delete it.
  /*if(SD.exists("tmatrix.txt"))
  {
    SD.remove("tmatrix.txt");
    MyDelay(50);
  }*/
  
  /*FileOpen("tmatrix.txt", FILEWRITE);
  if(!myFileIsOpen())
  {
//    Log("No create file\n");
    FileClose();
    return false;
  }
  
  do
  {
    data = rd32(address + (count * 4));
//    Log("TM%dw: 0x%08lx\n", count, data);
    FileWrite(data & 0xff);                // Little endian file storage to match Eve
    FileWrite((data >> 8) & 0xff);
    FileWrite((data >> 16) & 0xff);
    FileWrite((data >> 24) & 0xff);
    count++;
  }while(count < 6);
  FileClose();*/
//  Log("Matrix Saved\n\n");
}

// Read the touch digitizer calibration matrix values from a file and write them to the Eve.
bool LoadTouchMatrix(void)
{
  /*uint8_t count = 0;
  uint32_t data;
  uint32_t address = REG_TOUCH_TRANSFORM_A + RAM_REG;
  
  FileOpen("tmatrix.txt", FILEREAD);
  if(!myFileIsOpen())
  {
//    Log("tmatrix.txt not open\n");
    FileClose();
    return false;
  }
  
  do
  {
    data = FileReadByte() +  ((uint32_t)FileReadByte() << 8) + ((uint32_t)FileReadByte() << 16) + ((uint32_t)FileReadByte() << 24);
//    Log("TM%dr: 0x%08lx\n", count, data);
    wr32(address + (count * 4), data);
    count++;
  }while(count < 6);
  
  FileClose();
//  Log("Matrix Loaded \n\n");
  return true;*/
  return false;
}

// Read a single byte from a file
uint8_t FileReadByte(FILE* fp)
{
  uint8_t temp;
  fread(&temp, 1, 1, fp);
  return temp;
}

// Read bytes from a file into a provided buffer
void FileReadBuf(FILE* fp, uint8_t *data, uint32_t NumBytes)
{
  fread(data, 1, NumBytes, fp);
}
