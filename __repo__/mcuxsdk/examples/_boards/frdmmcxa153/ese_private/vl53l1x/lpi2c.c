/*! ***************************************************************************
 *
 * \brief     Low-Power I2C controller polling
 * \file      lpi2c.c
 * \author    Hugo Arends
 * \date      Jauary 2026
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

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void lpi2c_controller_init(void)
{
    // Set clock source
    // MUX: [010] = FRO_HF_DIV
    MRCC0->MRCC_LPI2C0_CLKSEL = MRCC_MRCC_LPI2C0_CLKSEL_MUX(2);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
    // DIV: [0000] = divider value = (DIV+1) = 1
    MRCC0->MRCC_LPI2C0_CLKDIV = 0;

    // Enable modules and leave others unchanged
    // LPI2C0: [1] = Peripheral clock is enabled
    // PORT3: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPI2C0(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);

    // Release modules from reset and leave others unchanged
    // LPI2C0: [1] = Peripheral is released from reset
    // PORT3: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPI2C0(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);

    // Configure P3_27 and P3_28
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [1] = Digital Input Buffer Enable, otherwise pin is used for analog
    //            functions
    // MUX: [0010] = Alternative 2
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [1] = Enables
    // PS:  [1] = Enables internal pullup resistor
    PORT3->PCR[27] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_PE(1) |
        PORT_PCR_PS(1) | PORT_PCR_ODE(1) | PORT_PCR_IBE(1); // LPI2C0_SCL
    PORT3->PCR[28] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_PE(1) |
        PORT_PCR_PS(1) | PORT_PCR_ODE(1) | PORT_PCR_IBE(1); // LPI2C0_SDA

    // From section 36.5 Initialization (NXP, 2024)
    //
    // To initialize the LPI2C controller:
    // 1. Configure Controller Configuration 0 (MCFGR0)�Controller Configuration
    //    3 (MCFGR3) as required by the application.
    // 2. Configure Controller Clock Configuration 0 (MCCR0) and Controller
    //    Clock Configuration 1 (MCCR1) to satisfy the timing requirements of
    //    the I2C mode supported by the application.
    // 3. Enable controller interrupts and DMA requests as required by the
    //    application.
    // 4. Enable the LPI2C controller by writing 1 to MCR[MEN].

    // 1.
    //
    // I2C timing parameters to setup the following specifications (see
    // paragraph 36.6 (NXP, 2024)):
    // I2C mode: FAST
    // Clock frequency: 48 MHz
    // I2C baud rate: 400 kbits/s
    LPI2C0->MCFGR1 = LPI2C_MCFGR1_PRESCALE(0);
    LPI2C0->MCFGR2 = LPI2C_MCFGR2_FILTSDA(1) | LPI2C_MCFGR2_FILTSCL(1);

    // 2.
    //
    // See paragraph 36.6
    LPI2C0->MCCR0 = LPI2C_MCCR0_DATAVD(0x0F) | LPI2C_MCCR0_SETHOLD(0x1D) |
        LPI2C_MCCR0_CLKHI(0x35) | LPI2C_MCCR0_CLKLO(0x3E);

    // 4.
    //
    // MEN: [1] = Controller Enable
    LPI2C0->MCR |= LPI2C_MCR_MEN(1);
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
    return (LPI2C0->MSR & (LPI2C_MSR_BBF_MASK | LPI2C_MSR_MBF_MASK)) != 0;
}

void lpi2c_write(const uint8_t dev_address, const uint8_t reg, uint8_t *p, const uint32_t len)
{
    // Wait as long as bus or controller is busy or timeout
    uint32_t timeout = 1000000;
    while(lpi2c_busy() && --timeout > 0)
    {}

    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK |
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;

    // Timeout?
    if(timeout == 0)
    {
        // Command: 010b - Generate Stop condition on I2C bus
        // Data   : n.a.
        LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);

        return;
    }

    // Note. Four words can be written to the MTDR register, because of the
    //       four word transmit FIFO (8-bit transmit data + 3-bit command).
    //       If more must be written, use the function lpi2c_txfifo_full()
    //       after four words to wait until the FIFO is not full anymore.

    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(dev_address << 1);

    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : index low byte
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(reg);

    for(uint32_t i=0; i<len; ++i)
    {
        // Wait while transmit fifo full or timeout
        timeout = 1000000;
        while(lpi2c0_txfifo_full() && --timeout > 0)
        {}

        // Timeout?
        if(timeout == 0)
        {
            // Command: 010b - Generate Stop condition on I2C bus
            // Data   : n.a.
            LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);

            return;
        }

        // Command: 000b - Transmit the value in DATA[7:0]
        // Data   : data
        LPI2C0->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(p[i]);
    }

    // Wait while transmit fifo full or timeout
    timeout = 1000000;
    while(lpi2c0_txfifo_full() && --timeout > 0)
    {}

    // Command: 010b - Generate Stop condition on I2C bus
    // Data   : n.a.
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);
}

void lpi2c_read(const uint8_t dev_address, const uint8_t reg, uint8_t *p, const uint32_t len)
{
    // Wait as long as bus or controller is busy or timeout
    uint32_t timeout = 1000000;
    while(lpi2c_busy() && --timeout > 0)
    {}

    // Clear all status flags
    LPI2C0->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK |
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;

    // Timeout?
    if(timeout == 0)
    {
        // Command: 010b - Generate Stop condition on I2C bus
        // Data   : n.a.
        LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);

        return;
    }

    // Note. Four words can be written to the MTDR register, because of the
    //       four word transmit FIFO (8-bit transmit data + 3-bit command).
    //       If more must be written, use the function lpi2c_txfifo_full()
    //       after four words to wait until the FIFO is not full anymore.

    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(dev_address << 1);

    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : index low byte
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(reg);

    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + r
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA((dev_address << 1) | 1);

    // Command: 001b - Receive (DATA[7:0] + 1) bytes. DATA[7:0] is used as a
    //          byte counter.
    // Data   : count
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b001) | LPI2C_MTDR_DATA(len - 1);

    // Receive the data
    for(uint32_t i=0; i<len; ++i)
    {
        // Wait while the RXFIFO is empty or timeout
        timeout = 1000000;
        while(lpi2c0_rxfifo_empty() && --timeout > 0)
        {}

        // Timeout?
        if(timeout == 0)
        {
            // Command: 010b - Generate Stop condition on I2C bus
            // Data   : n.a.
            LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);

            return;
        }

        // Read the data
        p[i] = (uint8_t)LPI2C0->MRDR;
    }

    // Wait while transmit fifo full or timeout
    timeout = 1000000;
    while(lpi2c0_txfifo_full() && --timeout > 0)
    {}

    // Command: 010b - Generate Stop condition on I2C bus
    // Data   : n.a.
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b010);
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
