//Raspberry Pi GPIO Library (to be removed)
#include <bcm2835.h>

//Standard Libraries
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h> 
#include <stdbool.h>
#include <sys/time.h>

//File Handling Libraries
#include <fcntl.h>
#include <unistd.h>

//SPI Bus Libraries
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>

//EVE3 Libraries
#include "Eve2_81x.h"           
#include "MatrixEve2Conf.h"      // Header for EVE2 Display configuration settings
#include "process.h"
#include "Raspberry_AL.h"



//File myFile;
char LogBuf[WorkBuffSz];

static const char *device = "/dev/spidev0.0";
uint32_t mode_bits = 0;//SPI_NO_CS;
uint32_t speed = 10000000;
uint16_t delay = 0;
uint8_t word_size = 8;
int fd;

int main(int argc, char **argv)
{
  if (argc < 3) pabort("Incorrect Number of Arguments!");
  // Initializations.  Order is important
  struct timeval start, end;
  double elapsedTime;
  gettimeofday(&start, NULL);

  if (GlobalInit() != 0) return 1;
  printf("Global Init Finished\n");
  FT81x_Init();
  printf("FT81X Init Finished\n");

  //Attach and Erase the onboard flash
  FlashAttach();                           // Attach flash
  //FlashErase();

  //Load image(s) onto the onboard flash

  FlashLoad(argv[1]);

  //Set the Onboard flash to faster QSPI mode
  if (!FlashFast()) {
    printf("Could not set Flash to fast QSPI mode!\n");
    return 1;
  }
  
  //Set Brightness (0-127) and Backlight PWM Freq (250-10000)
  wr8(REG_PWM_DUTY + RAM_REG, 127);
  wr16(REG_PWM_HZ + RAM_REG, 1000);

  if (strcmp(argv[2], "jpg") == 0) {
    printf("Loading JPG\n");
    MakeScreen_Bitmap_JPEG(RAM_FLASH | 4096, RAM_G, 0);
  } else if (strcmp(argv[2], "rgb") == 0) {
    MakeScreen_Bitmap_RGB(4096, RAM_G, 261120);
  } else if (strcmp(argv[2], "gif") == 0) {
    while (1) {
      wr8(REG_PLAY_CONTROL + RAM_REG, 1);
      Send_CMD(CMD_DLSTART);
      Send_CMD(CLEAR(1, 1, 1));
      Send_CMD(COLOR_RGB(255, 255, 255));

      Send_CMD(CMD_FLASHSOURCE);
      Send_CMD(RAM_FLASH | 4096);
      Send_CMD(CMD_PLAYVIDEO);
      Send_CMD(OPT_FLASH | OPT_NOTEAR);

      UpdateFIFO();
      Wait4CoProFIFOEmpty();
    }
  }

  gettimeofday(&end, NULL);
  elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0;   // us to ms
  printf("Took %f ms\n", elapsedTime);

}

//Adapted from the Arduino Library

int GlobalInit(void) {

  //Return Value and SPI File Descriptor
  int ret = 0;
  fd;

  //SPI Variables
  uint32_t mode;
  uint8_t bits = 8;

  //Open SPI Device
  fd = open(device, O_RDWR);
  if (fd < 0) {
    pabort("Cannot open SPI device\n");
  }

  //Set SPI Mode
  ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode_bits);
  if (ret == -1) {
    pabort("Can't set SPI mode\n");
  }

  //Set SPI Word Size
  ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &word_size);
  if (ret == -1) {
    pabort("Can't set SPI word size\n");
  }

  //Set SPI frequency
  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1) {
    pabort("Can't set SPI frequency\n");
  }

  if (!bcm2835_init()){
    printf("bcm2835_init failed. Are you running as root??\n");
    return -1;
  }

  printf("SPI Bus Successfully Initialized!\n");

  bcm2835_spi_end();

  if (!bcm2835_spi_begin()) {
    printf("bcm2835_spi_begin failed. Are you running as root??\n");
    return -1;
  }

  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // Set MSB First
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // SPI Mode 0

  bcm2835_gpio_fsel(EvePDN_PIN, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_write(EvePDN_PIN, LOW);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32); // Set the Clock Divider to

  bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
  bcm2835_gpio_fsel(EveChipSelect_PIN, BCM2835_GPIO_FSEL_OUTP);

  bcm2835_gpio_write(EveChipSelect_PIN, HIGH);
  return ret;
}

int LCD_Init() {
  // Initializations.  Order is important
  Eve_Reset_HW();
  printf("Global Init Finished\n");
  FT81x_Init();
  printf("FT81X Init Finished\n");

  //Attach and Erase the onboard flash
  if (!FlashAttach()) {                           // Attach flash
    pabort("Could not Attach LCD Flash!");
  }
  FlashErase();

  MyDelay(300);
  //Send_CMD(HCMD_SLEEP);
}


// Send a single byte through SPI
void SPI_WriteByte(uint8_t data)
{
  //bcm2835_gpio_write(EveChipSelect_PIN, LOW);
  //bcm2835_spi_transfer(data);
  //bcm2835_gpio_write(EveChipSelect_PIN, HIGH);

  
  int ret;
  struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long) &data,
		.rx_buf = (unsigned long) NULL,
		.len = 1,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = word_size,
	};

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 1) pabort("can't send spi message");
}

// Send a series of bytes (contents of a buffer) through SPI
void SPI_WriteBuffer(uint8_t *Buffer, uint32_t Length)
{
  /*bcm2835_gpio_write(EveChipSelect_PIN, LOW);
  bcm2835_spi_transfern(Buffer, Length);
  bcm2835_gpio_write(EveChipSelect_PIN, HIGH);*/
  SPI_ReadWriteBuffer(Buffer, NULL, Length, Length);
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

void SPI_ReadWriteBuffer(uint8_t *readbuf, uint8_t *writebuf, uint32_t readSize, uint32_t writeSize) {
  int ret;
  struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long) readbuf,
		.rx_buf = (unsigned long) writebuf,
		.len = readSize,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = word_size,
	};

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 1) pabort("can't send spi message");
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
}

// A millisecond delay wrapper for the Arduino function
void MyDelay(uint32_t DLY)
{
   bcm2835_delay(DLY);
}

static void pabort(const char *s)
{
	perror(s);
	abort();
}

void DebugPrint(char *str)
{
  printf("%s", str);
}

//================================== File Reading Functions ====================================

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
