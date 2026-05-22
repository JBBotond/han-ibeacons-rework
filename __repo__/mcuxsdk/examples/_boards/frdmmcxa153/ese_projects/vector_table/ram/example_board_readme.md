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
- Two Interrupt vector tables are used in this demo: the default vector table stored in FLASH and the relocated vector table stored in RAM. The relocated vector table has two updated interrupt handlers: the SysTick handler and the GPIO1 IRQ handler.
- Initially, the green LED blinks, indicating that the SysTick handler stored in the default vector table at FLASH address 0x00000000, is used.
- Pressing SW3, which triggers the GPIO1 IRQ handler, will print:

    ```TXT
    SW3: Triggered by function from vector table in FLASH
    ```

- Press the SW2 switch to relocate the vector table to RAM. The red LED will blink, indicating that the SysTick handler stored in the relocated vector table in RAM is used.
- Pressing SW3, which triggers the relocated GPIO1 IRQ handler, will now print:

    ```TXT
    SW3: Triggered by function from vector table in RAM
    ```
