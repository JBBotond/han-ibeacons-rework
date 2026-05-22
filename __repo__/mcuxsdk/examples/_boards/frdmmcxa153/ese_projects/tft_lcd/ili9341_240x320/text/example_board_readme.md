Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA153 board
- 2.8 inch TFT Display 240*320 pixels with Touchscreen - SPI - ILI9341 - 3.3V
- Personal Computer

Board settings
============
No special settings are required.

Prepare the Demo
===============
1. Connect the FRDM-MCXA153 board and the TFT LCD module as follows

    |   FRDM-MCXA153    |   TFT LCD    |
    |:-----------------:|:------------:|
    | P2_13/LPSPI1_SDO  | TFT_LCD_MOSI |
    | P2_12/LPSPI1_SCK  | TFT_LCD_CLK  |
    | P2_16/LPSPI1_SDI  | TFT_LCD_MISO |
    | P2_6/LPSPI1_PCS1  | TFT_LCD_CS1  |
    | P3_11/LPSPI1_PCS0 | TFT_LCD_CS2  |
    | P1_5              | TFT_LCD_RES  |
    | P2_1              | TFT_LCD_DC   |
    | P2_2              | TFT_LCD_BLK  |
    | P2_3              | TFT_LCD_PEN  |
    | 3V3               | VCC          |
    | GND               | GND          |

2. Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
3. Build the application.
4. Run or debug the application.

Running the demo
===============
- The TFT LCD module displays texts and responds to touch input.
