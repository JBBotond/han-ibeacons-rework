/*! ***************************************************************************
 *
 * \brief     Low level driver for the AOI (And-Or-Invert) module
 * \file      aoi.c
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
 ******************************************************************************/
#include "aoi.h"

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
void aoi_init(void)
{
    // Enable modules and leave others unchanged
    // AOI0: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_AOI0(1);

    // Release modules from reset and leave others unchanged
    // AOI0: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_AOI0(1);

    // Enable modules and leave others unchanged
    // INPUTMUX0: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_INPUTMUX0(1);

    // Release modules from reset and leave others unchanged
    // INPUTMUX0: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_INPUTMUX0(1);


    // Enable modules and leave others unchanged
    // GPIO1: [1] = Peripheral clock is enabled
    // PORT1: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO1(1);

    // Release modules from reset and leave others unchanged
    // GPIO1: [1] = Peripheral is released from reset
    // PORT1: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO1(1);


    // P1_0: Alternative 1 (TRIG_IN0)
    PORT1->PCR[0] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(1) | PORT_PCR_PE(1) | PORT_PCR_PS(1);

    // P1_1: Alternative 1 (TRIG_IN1)
    PORT1->PCR[1] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(1) | PORT_PCR_PE(1) | PORT_PCR_PS(1);

    // P1_2: Alternative 1 ((EXT) TRIG_OUT0)
    PORT1->PCR[2] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(1);

    // P1_3: Alternative 1 ((EXT) TRIG_OUT1)
    PORT1->PCR[3] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(1);


    // Set inputs for AOI. The mapping is as follows.

    // AOI event 0
    // AOI0_MUX[0]  -> AOI input A0 : TRIG_IN0
    // AOI0_MUX[1]  -> AOI input B0 : TRIG_IN1
    // AOI0_MUX[2]  -> AOI input C0 : nc
    // AOI0_MUX[3]  -> AOI input D0 : nc

    // AOI event 1
    // AOI0_MUX[4]  -> AOI input A1 : TRIG_IN0
    // AOI0_MUX[5]  -> AOI input B1 : TRIG_IN1
    // AOI0_MUX[6]  -> AOI input C1 : nc
    // AOI0_MUX[7]  -> AOI input D1 : nc

    // AOI event 2
    // AOI0_MUX[8]  -> AOI input A2 : not used
    // AOI0_MUX[9]  -> AOI input B2 : not used
    // AOI0_MUX[10] -> AOI input C2 : not used
    // AOI0_MUX[11] -> AOI input D2 : not used

    // AOI event 3
    // AOI0_MUX[12] -> AOI input A3 : not used
    // AOI0_MUX[13] -> AOI input B3 : not used
    // AOI0_MUX[14] -> AOI input C3 : not used
    // AOI0_MUX[15] -> AOI input D3 : not used


    // 10_0101b - TRIG_IN0 input is selected
    INPUTMUX0->AOI0_MUX[0] = INPUTMUX_AOI0_MUXA_AOI0_MUX_INP(0b100011);

    // 10_0110b - TRIG_IN1 input is selected
    INPUTMUX0->AOI0_MUX[1] = INPUTMUX_AOI0_MUXA_AOI0_MUX_INP(0b100100);

    // 10_0101b - TRIG_IN0 input is selected
    INPUTMUX0->AOI0_MUX[4] = INPUTMUX_AOI0_MUXA_AOI0_MUX_INP(0b100011);

    // 10_0110b - TRIG_IN1 input is selected
    INPUTMUX0->AOI0_MUX[5] = INPUTMUX_AOI0_MUXA_AOI0_MUX_INP(0b100100);


    // Set inputs for EXT triggers. The mapping is as follows.

    // EXT_TRIG[0]  -> EXT trigger 0 input : AOI0_OUT0 (AOI event 0)
    // EXT_TRIG[1]  -> EXT trigger 1 input : AOI0_OUT1 (AOI event 1)
    // EXT_TRIG[2]  -> EXT trigger 2 input : not used
    // EXT_TRIG[3]  -> EXT trigger 3 input : not used
    // EXT_TRIG[4]  -> EXT trigger 4 input : not used
    // EXT_TRIG[5]  -> EXT trigger 5 input : not used
    // EXT_TRIG6[0] -> EXT trigger 6 input : not used
    // EXT_TRIG6[1] -> EXT trigger 7 input : not used

    // AOI0_OUT0 input is selected
    INPUTMUX0->EXT_TRIG[0] = INPUTMUX_EXT_TRIGA_EXT_TRIG_INP(0b00010);

    // AOI0_OUT1 input is selected
    INPUTMUX0->EXT_TRIG[1] = INPUTMUX_EXT_TRIGA_EXT_TRIG_INP(0b00011);


    // Configure the AOI event logic for AOI event 0 (AOI0_OUT0)
    // A & B
    AOI0->BFCRT[0].BFCRT01 =
        AOI_BFCRT01_PT0_AC(1) |
        AOI_BFCRT01_PT0_BC(1) |
        AOI_BFCRT01_PT0_CC(3) |
        AOI_BFCRT01_PT0_DC(3) |

        AOI_BFCRT01_PT1_AC(0) |
        AOI_BFCRT01_PT1_BC(3) |
        AOI_BFCRT01_PT1_CC(3) |
        AOI_BFCRT01_PT1_DC(3);

    AOI0->BFCRT[0].BFCRT23 =
        AOI_BFCRT23_PT2_AC(0) |
        AOI_BFCRT23_PT2_BC(3) |
        AOI_BFCRT23_PT2_CC(3) |
        AOI_BFCRT23_PT2_DC(3) |

        AOI_BFCRT23_PT3_AC(0) |
        AOI_BFCRT23_PT3_BC(3) |
        AOI_BFCRT23_PT3_CC(3) |
        AOI_BFCRT23_PT3_DC(3);

    // Configure the AOI event logic for AOI event 1 (AOI0_OUT1)
    // (A & ~B) + (~A & B)
    AOI0->BFCRT[1].BFCRT01 =
        AOI_BFCRT01_PT0_AC(1) |
        AOI_BFCRT01_PT0_BC(2) |
        AOI_BFCRT01_PT0_CC(3) |
        AOI_BFCRT01_PT0_DC(3) |

        AOI_BFCRT01_PT1_AC(2) |
        AOI_BFCRT01_PT1_BC(1) |
        AOI_BFCRT01_PT1_CC(3) |
        AOI_BFCRT01_PT1_DC(3);

    AOI0->BFCRT[1].BFCRT23 =
        AOI_BFCRT23_PT2_AC(0) |
        AOI_BFCRT23_PT2_BC(0) |
        AOI_BFCRT23_PT2_CC(0) |
        AOI_BFCRT23_PT2_DC(0) |

        AOI_BFCRT23_PT3_AC(0) |
        AOI_BFCRT23_PT3_BC(0) |
        AOI_BFCRT23_PT3_CC(0) |
        AOI_BFCRT23_PT3_DC(0);
}
