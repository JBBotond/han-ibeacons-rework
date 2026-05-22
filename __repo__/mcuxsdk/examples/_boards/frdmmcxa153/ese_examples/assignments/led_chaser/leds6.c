/*! ***************************************************************************
 *
 * \brief     LEDs initialisation and control for FRDM-MCXA153 board
 * \file      leds6.c
 * \author    Hugo Arends
 * \date      January 2026
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
 *
 * \copyright 2026 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include "leds6.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

/*! \brief Initialize the pins to control the six LEDs.
 *
 * The pins are configured in GPIO mode as digital outputs. The pin value is
 * set to 0 (LEDs off) during initialization.
 *
 * The LEDs are connected as follows:
 * - P2_12: D1
 * - P2_16: D2
 * - P2_13: D3
 * - P2_6 : D4
 * - P3_14: D5
 * - P3_15: D6
 * - GND  : GND
 */
void leds6_init(void)
{
    // TBD
}

/*!
 * \brief Set the state of the six LEDs.
 *
 * Use this function to turn on or off the LEDs D1 to D6 on the
 * breakout board. The LSB of the value corresponds to D1, the next bit to D2,
 * and so on up to D6.

 * \param value Bit vector controlling the six LEDs (D1 to D6). Each bit set
 *              to 1 turns on the corresponding LED, each bit set to 0 turns
 *              off the corresponding LED. D1 is the LSB.
 */
void leds6_set(uint8_t value)
{
    // TBD
}
