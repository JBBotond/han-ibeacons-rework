/*! ***************************************************************************
 *
 * \brief     Low-Power I2C controller
 * \file      lpi2c.c
 * \author    Hugo Arends
 * \date      February 2026
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
#include "lpi2c.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
static bool lpi2c0_txfifo_full(void);
static bool lpi2c0_rxfifo_empty(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void lpi2c_controller_init(void)
{
    // TBD
}

/*!
 * \brief Checks if the LPI2C controller is busy
 *
 * This function checks Bus Busy Flag and the Controller Busy Flag in the
 * Controller Status register (MSR). If one of both is set, the controller is
 * busy and the function returns true.
 *
 * \return true if the controller is busy, false otherwise
 */
bool lpi2c_busy(void)
{
    // TBD

    return false;
}

void lpi2c_write(const uint8_t dev_address, const uint8_t reg, uint8_t *p, const uint32_t len)
{
    // TBD
}

void lpi2c_read(const uint8_t dev_address, const uint8_t reg, uint8_t *p, const uint32_t len)
{
    // TBD
}

inline bool lpi2c0_txfifo_full(void)
{
    uint32_t n = (LPI2C0->MFSR & LPI2C_MFSR_TXCOUNT_MASK) >> LPI2C_MFSR_TXCOUNT_SHIFT;

    // See reference manual section 36.2.2 Features (NXP, 2024)
    //
    // Command and transmit FIFO of 4 words (8-bit transmit data + 3-bit command)
    return n == 4;
}

inline bool lpi2c0_rxfifo_empty(void)
{
    uint32_t n = (LPI2C0->MFSR & LPI2C_MFSR_RXCOUNT_MASK) >> LPI2C_MFSR_RXCOUNT_SHIFT;

    return n == 0;
}
