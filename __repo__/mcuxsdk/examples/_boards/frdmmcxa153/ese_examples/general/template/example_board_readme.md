Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA153 board
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
- Open the VCOM (115200-8n1) in a terminal application of your choice.
- Verify that after microcontroller reset a message appears in the terminal application.
- Verify that typing the characters 'r', 'g' and 'b' in the terminal application toggles the corresponding RBG LED.
- Verify that pressing SW2 button toggles the green LED on/off and prints "SW2" in terminal application.
- Verify that pressing SW3 button toggles the red LED on/off and prints "SW3" in terminal application.
