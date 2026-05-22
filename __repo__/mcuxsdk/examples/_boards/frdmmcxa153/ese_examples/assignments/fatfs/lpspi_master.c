/*! ***************************************************************************
 *
 * \brief     Low level driver for the Low Power Serial Peripheral Interface
 *            (LPSPI) in master mode
 * \file      lpspi_master.c
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
 ******************************************************************************/
#include "lpspi_master.h"
#include <stdlib.h>


// NOTE 1. This file contains "TBD" comments where the actual implementation
//         code should be placed. Instead of replacing all "TBD" comments with
//         code, you can also copy-and-past the entire file from the
//         'ese_examples/drivers/lpspi/loopback/polling' example.



// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
static bool lpspi0_txfifo_full(void);
static bool lpspi0_rxfifo_empty(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
// static uint8_t _dummy = 0xFF;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void lpspi_master_init(void)
{
    // TBD
}

/*!
 * \brief Checks if the LPSPI controller is busy
 *
 * This function checks Module Busy Flag in the Status register (SR).
 * If it is set, the controller is busy and the function returns true.
 *
 * \return true if the controller is busy, false otherwise
 */
bool lpspi_busy(void)
{
    // TBD

    return false;
}

void lpspi_transmit(uint8_t *tx_buffer, const uint32_t n)
{
    // TBD
}

void lpspi_receive(uint8_t *rx_buffer, const uint32_t n)
{
    // TBD
}

void lpspi_transceive(uint8_t *tx_buffer, uint8_t *rx_buffer, const uint32_t n)
{
    // TBD
}

/*!
 * \brief Sets the dummy byte that is transmitted during reception
 *
 * This function sets the dummy byte that is transmitted when only receiving
 * data from a slave device.
 *
 * \param dummy The dummy byte to be used
 */
void lpspi_set_dummy(uint8_t dummy)
{
    // TBD
}

inline bool lpspi0_txfifo_full(void)
{
    // TBD

    return false;
}

inline bool lpspi0_rxfifo_empty(void)
{
    // TBD

    return true;
}
