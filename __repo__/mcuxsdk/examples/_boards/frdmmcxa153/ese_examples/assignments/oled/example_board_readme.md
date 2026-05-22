Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA153 board
- 128x64 OLED (SSD1306) display (e.g., on the *oled_sdcard_buzzer shield*)
- Personal Computer

Board settings
============
No special settings are required.

Prepare the Demo
===============
1.  Connect a Type-C USB cable between the host PC and the MCU-Link port(J15) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
===============
- The OLED display shows two alternating logos.
