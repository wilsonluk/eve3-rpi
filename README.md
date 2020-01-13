This code contains functions to perform the following operations:

- Read a file from the local RPi filesystem and transfer it to the Eve Flash
- An example of displaying bitmaps (RGB565 format) directly out of flash.

Designed for Matrix Orbital EVE2 SPI TFT Displays incorporating BT81x chips and QSPI flash

  https://www.matrixorbital.com/ftdi-eve/eve-bt815

- This code makes use of the Matrix Orbital EVE2 Library found here: 

  https://github.com/MatrixOrbital/EVE2-Library

- Matrix Orbital EVE2 SPI TFT display information can be found at: https://www.matrixorbital.com/ftdi-eve

- The Raspberry Pi's bultin GPIO pins are used to interface with the Eve over SPI, using the bcm2835 library

  https://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md
  
  https://www.airspayce.com/mikem/bcm2835/
