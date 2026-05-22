Overview
========
Use the LPSPI module to communicate with an LCD-PAR-S035.

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- Shield V3
- LCD-PAR-S035 shield

Board settings
==============
- Default

LCD-PAR-S035 settings
=====================
- SW1 IM[2:0]: 111 (4-wire 8bit SPI)

Preparation
===========
- Connect the LCD-PAR-S035 shield to the FRDM-MCXA153 board using the PMOD connector.
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Debug the application.

- By default, the application uses RGB888 images. Change the define BYTES_PER_PIXEL in the file lcd_par_s035.h to use a different image format.

Result
======
- The application shows an image on the LCD screen.
- Press the SW3 button to loop color patterns on the LCD screen.
