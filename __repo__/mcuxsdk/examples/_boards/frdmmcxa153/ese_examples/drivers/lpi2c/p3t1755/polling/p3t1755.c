/*! ***************************************************************************
 *
 * \brief     P3T1755 - I3C, I 2 C-bus interface, 0.5 C accuracy, digital
 *            temperature sensor
 * \file      p3t1755.c
 * \author    Hugo Arends
 * \date      June 2025
 *
 * \see       NXP. (2024). P3T1755 - I3C, I 2 C-bus interface, 0.5 C accuracy,
 *            digital temperature sensor - Product data sheet. Rev. 1.1,
 *            04/01/2023. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
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
#include "p3t1755.h"
#include "lpi2c.h"

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
void p3t1755_init(void)
{
    lpi2c_controller_init();

    #ifdef DEBUG

    // Check connectivity by reading the control register. The POR value is
    // 0x28 (NXP, 2023).
    //
    uint8_t reg = p3t1755_get_configuration_reg();

    if(reg != 0x28)
    {
        // Error
        while(1)
        {}
    }

    #endif
}

uint8_t p3t1755_get_configuration_reg(void)
{
    uint8_t data = 0;

    // Device address: 0b1001000 (P3T1755)
    // Pointer byte: 0b00000001 (Configuration register)
    lpi2c_read(0b1001000, 0b00000001, &data, 1);

    return data;
}

void p3t1755_set_configuration_reg(uint8_t val)
{
    // Device address: 0b1001000 (P3T1755)
    // Pointer byte: 0b00000001 (Configuration register)
    // Send one byte: val
    lpi2c_write(0b1001000, 0b00000001, &val, 1);
}

float p3t1755_get_temperature(void)
{
    uint8_t data[2] = {0};

    // Get temperature data from the P3T1755
    // Device address: 0b1001000 (P3T1755)
    // Pointer byte: 0b00000000 (Temperature register)
    lpi2c_read(0b1001000, 0b00000000, data, 2);

    // Calculate temperature
    uint16_t temp_data = (int16_t)(data[0] << 4) | (data[1] >> 4);

    float temperature = 0;

    // Positive temperature?
    if((temp_data & 0b0000100000000000) == 0)
    {
        temperature = temp_data * 0.0625f;
    }
    else
    {
        temp_data = (~temp_data) & 0x0FFF;
        temperature = -((temp_data + 1) * 0.0625f);
    }

    return temperature;
}
