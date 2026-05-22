/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      February 2024
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
 *
 * \copyright 2024 HAN University of Applied Sciences. All Rights Reserved.
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
#include <board.h>

#include "leds.h"
#include "leds6.h"
#include "switches.h"
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

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    leds_init();
    leds6_init();
    sw_init();
    sw4_init();

    int dir = 0;
    int delay = 1000000;

    while(1)
    {
        // ---------------------------------------------------------------------
        if(dir == 0)
        {
            // Blocking loop of six LEDs from D1 to D6
            for(uint32_t i = 0; i < 6; i++)
            {
                // Set LEDs
                leds6_set(1 << i);

                // Delay
                for(volatile uint32_t j = 0; j < delay; j++)
                {}
            }
        }
        else
        {
            // Blocking loop of six LEDs from D6 to D1
            for(int32_t i = 5; i >= 0; i--)
            {
                // Set LEDs
                leds6_set(1 << i);

                // Delay
                for(volatile uint32_t j = 0; j < delay; j++)
                {}
            }
        }

        // ---------------------------------------------------------------------
        if(k1_pressed())
        {
            dir = 1;
        }

        if(k2_pressed())
        {
            dir = 0;
        }

        if(k3_pressed())
        {
            delay = 1000000 / 4;
        }

        if(k4_pressed())
        {
            delay = 1000000;
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
