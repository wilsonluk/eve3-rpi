// This file acts as a header for the .ino file.  Since the .ino file is the hardware abstraction layer
// in Arduino world, it also contains prototypes for abstracted hardware calls in addition to prototypes
// for abstracted time functions which in other compilers would just be other .c and .h files

#ifndef __RPIAL_H
#define __RPIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>              // Find integer types like "uint8_t"  
#include <stdbool.h>             // Find type "bool"

// defines related to hardware and relevant only to the hardware abstraction layer (this and .ino files)
#define EveChipSelect_PIN          RPI_V2_GPIO_P1_24
#define EvePDN_PIN                 RPI_GPIO_P1_12

#define SPISpeed            10000000

#define WorkBuffSz 64UL
extern char LogBuf[WorkBuffSz];         // The singular universal data array used for all things including logging

#define \
    Log(...)  \
        ({ \
            printf(__VA_ARGS__); \
        }) // Stuff string and parms via sprintf and output
// #define Log(...) // Liberate (a lot of) RAM by uncommenting this empty definition (remove all serial logging)

int GlobalInit(void);
int LCD_Init();

// Hardware peripheral abstraction function prototypes
uint8_t ReadPin(uint8_t);
void SetPin(uint8_t, bool);
void SD_Init(void);
void SPI_Enable(void);
void SPI_Disable(void);
void SPI_Write(uint8_t data);
void SPI_WriteByte(uint8_t data);
void SPI_WriteBuffer(uint8_t *Buffer, uint32_t Length);
void SPI_ReadBuffer(uint8_t *Buffer, uint32_t Length);
void SPI_ReadWriteBuffer(uint8_t *readbuf, uint8_t *writebuf, uint32_t readSize, uint32_t writeSize);

// These functions encapsulate Arduino library functions
void DebugPrint(char *str);
void MyDelay(uint32_t DLY);
uint32_t MyMillis(void);
void Eve_Reset_HW(void);

//Helper Functions
static void pabort(const char* s);

// Function encapsulation for file operations
bool FileExists(char *filename);
FILE* FileOpen(char *filename, uint8_t mode);
bool FileClose(FILE* fd);
uint8_t FileReadByte(FILE* fd);
void FileReadBuf(FILE* fd, uint8_t *data, uint32_t NumBytes);
uint32_t FileSize(FILE* fd);
uint32_t FilePosition(FILE* fd);
bool FileSeek(uint32_t offset, FILE* fd);

#ifdef __cplusplus
}
#endif

#endif
