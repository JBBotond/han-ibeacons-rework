Overview
========
Unit test project to verify the functionality of the oled_sdcard_buzzer shield on the FRDM-MCXA153 board.

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- OLED_SDCARD_BUZZER shield

Board settings
==============
- Default

Preparation
===========
- Connect the OLED_SDCARD_BUZZER shield to the FRDM-MCXA153 board using the MikroBUS connector.
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run the application.

Result
======
- Open the VCOM (115200-8n1) in a terminal application of your choice.
- The terminal will display messages indicating the status of the OLED display, SD card, and buzzer tests.
- Press the reset button on the board to rerun the tests.
