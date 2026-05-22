Overview
========
Shell for the oled_sdcard_buzzer shield on the FRDM-MCXA153 board.

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
- Connect a type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run the application.

Result
======
- Open the VCOM (115200-8n1) in a terminal application of your choice.
- The terminal will display a shell.

```TXT
Embedded Shell Framework
Type 'help' for available commands
(Use TAB for auto-completion of commands and files, backspace to edit)

/>
```

- Type `help` to see the list of available commands.