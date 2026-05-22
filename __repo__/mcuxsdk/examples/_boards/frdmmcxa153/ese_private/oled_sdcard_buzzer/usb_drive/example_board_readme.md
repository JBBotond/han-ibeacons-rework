Overview
========
USB drive (MSC) for the oled_sdcard_buzzer shield on the FRDM-MCXA153 board.

Hardware requirements
=====================
- FRDM-MCXA153 board
- 2x Type-C USB cable
- OLED_SDCARD_BUZZER shield

Board settings
==============
- Default

Preparation
===========
- Connect the OLED_SDCARD_BUZZER shield to the FRDM-MCXA153 board using the MikroBUS connector.
- Connect a type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Connect a type-C USB cable between the host PC and the MCU-USB port (J8) on the target board.
- Build the application.
- Run the application.

Result
======
- Open the VCOM (115200-8n1) in a terminal application of your choice.
- The terminal will display messages indicating the status of the application.
- A new drive appears in the host PC's file explorer.
