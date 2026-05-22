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
2.  Download the program to the target board.
3.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
===============
- Start the debugger and single step through the code. Make sure the disassembly window is selected and that the core registers are visible.
- At every step, one or more core registers and/or the stack is updated. Make sure to understand why these registers are updated and what information is stored on the stack.
