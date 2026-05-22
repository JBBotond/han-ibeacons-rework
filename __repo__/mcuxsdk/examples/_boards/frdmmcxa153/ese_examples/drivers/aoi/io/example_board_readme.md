# AOI - IO

## Overview
AOI IO example project

## Hardware requirements
- FRDM-MCXA153 board
- Type-C USB cable
- Jumper wires
- Logic analyzer or oscilloscope

## Board settings
- Default

## Preparation
- Connect jumper wires as follows:

    | From  | To              |
    |:-----:|:---------------:|
    | P3_12 | P1_0 - TRIG_IN0 |
    | P3_13 | P1_1 - TRIG_IN1 |

- Connect the logic analyzer (or oscilloscope) as follows to monitor the input and output signals:

    | Pin  | Signal               |
    |:----:|:--------------------:|
    | P1_0 | P1_0 - TRIG_IN0      |
    | P1_1 | P1_1 - TRIG_IN1      |
    | P1_2 | P1_2 - EXT TRIG_OUT0 |
    | P1_3 | P1_3 - EXT TRIG_OUT1 |

- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

## Result
- The RGB LED shows yellow. The pins connected to the red LED (P3_12) and green LED (P3_13) are connected to the AOI input pins P1_0 and P1_1 to generate input signals (see timing diagrams below).
- Use the logic analyzer (or oscilloscope) to verify the following behavior of the signals:
    - P1_2 - EXT TRIG_OUT0. It is the AND of P1_0 and P1_1. It is high only when both P1_0 and P1_1 are high.
    - P1_3 - EXT TRIG_OUT1. It is the XOR of P1_0 and P1_1. It is high when either P1_0 or P1_1 is high, but not both.
- This behavior is depicted in the following timing diagrams:

```TXT
Input signals:

P1_0: (TRIG_IN0)       ____|‾‾‾‾‾‾‾‾|________|‾‾‾‾‾‾‾‾|________

P1_1: (TRIG_IN1)       ________|‾‾‾‾‾‾‾‾|________|‾‾‾‾‾‾‾‾|____

Output signals:

P1_2: (EXT TRIG_OUT0)  ________|‾‾‾‾|____________|‾‾‾‾|________

P1_3: (EXT TRIG_OUT1)  ____|‾‾‾|____|‾‾‾|____|‾‾‾|____|‾‾‾|____
                                                                -> t
```
