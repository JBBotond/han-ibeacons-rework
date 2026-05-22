/*! ***************************************************************************
 *
 * \brief     Low level driver for the Low Power Serial Peripheral Interface
 *            (LPSPI) in master mode
 * \file      lpspi.h
 * \author    Hugo Arends
 * \date      March 2026
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
#ifndef LPSPI_H
#define LPSPI_H

#include <MCXA153.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Shared type definitions
// -----------------------------------------------------------------------------
typedef enum lpspi_pcs
{
    LPSPI_PCS0 = 0,
    LPSPI_PCS1 = 1,
    LPSPI_PCS2 = 2,
    LPSPI_PCS3 = 3
} lpspi_pcs_t;

// -----------------------------------------------------------------------------
// Shared variables
// -----------------------------------------------------------------------------
extern volatile bool touch_detected;;

// -----------------------------------------------------------------------------
// Shared function prototypes
// -----------------------------------------------------------------------------
void lpspi_master_init(void);
bool lpspi_busy(void);
void lpspi_transmit_command(uint8_t *tx_buffer, const uint32_t n);
void lpspi_transmit_data(uint8_t *tx_buffer, const uint32_t n);
void lpspi_receive(uint8_t *rx_buffer, const uint32_t n);
void lpspi_transceive_command(uint8_t *tx_buffer, uint8_t *rx_buffer, const uint32_t n);
void lpspi_transceive_data(uint8_t *tx_buffer, uint8_t *rx_buffer, const uint32_t n);
void lpspi_set_dummy(uint8_t dummy);
void lpspi_set_pcs(lpspi_pcs_t pcs);
void lpspi_set_prescale(uint32_t prescale);

#endif // LPSPI_H
