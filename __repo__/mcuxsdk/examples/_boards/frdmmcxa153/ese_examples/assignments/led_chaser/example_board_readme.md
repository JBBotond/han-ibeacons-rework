Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA153 board
- A 6-LED breakout board or 6 individual LEDs with suitable series resistors
- A 4-switch breakout board or 4 individual push-buttons
- Breadboard and jumper wires
- Personal Computer

Board settings
============
No special settings are required.

Prepare the Demo
===============
1.  Connect a Type-C USB cable between the host PC and the MCU-Link port(J15) on the target board.
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
After successfully implementaing the assignments, you should observe the following behavior:
- The LEDs light up in a chaser pattern from D1 to D6 and back.
- Pressing switch 1 changes the direction to run from D6 to D1.
- Pressing switch 2 changes the direction to run from D1 to D6.
- Pressing switch 3 increases the speed of the LED chaser.
- Pressing switch 4 decreases the speed of the LED chaser.