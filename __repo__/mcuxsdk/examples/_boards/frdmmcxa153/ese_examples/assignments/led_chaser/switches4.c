/*! ***************************************************************************
 *
 * \brief     Switches example GPIO input with interrupt handling
 * \file      switches.c
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
#include "switches4.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint32_t k1_cnt = 0;
static volatile uint32_t k2_cnt = 0;
static volatile uint32_t k3_cnt = 0;
static volatile uint32_t k4_cnt = 0;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

/*! \brief Initialize the pins to control the four switches.
 *
 * The pins are configured in GPIO mode as digital inputs. Since the switches
 * breakout board has no pullup resistors, the internal pullup resistors are
 * enabled. Futhermore, interrupts are enabled on the falling edge, i.e. when
 * a switch is pressed.
 *
 * The switches are connected as follows:
 * - P1_4: K1
 * - P1_5: K2
 * - P2_3: K3
 * - P2_1: K4
 * - GND : GND
 */
void sw4_init(void)
{
    // TBD
}


// TBD - Copy-and-paste the interrupt handlers here



/*! \brief Check if switch Kn is pressed.
 *
 * \return true if Kn is pressed, false otherwise
 */
bool k1_pressed(void)
{
    // TBD

    return false;
}

bool k2_pressed(void)
{
    // TBD

    return false;
}

bool k3_pressed(void)
{
    // TBD

    return false;
}

bool k4_pressed(void)
{
    // TBD

    return false;
}
