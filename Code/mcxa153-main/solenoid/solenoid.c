#include "board.h"
#include <stdbool.h>

void solenoid_init(void) {
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);

    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO2(1);

    PORT2->PCR[7] = PORT_PCR_LK(1) | PORT_PCR_MUX(0);

    GPIO2->PCOR |= (1<<7);

    GPIO2->PDDR |= (1<<7);
}
void solenoid_lock(bool logicLevel) {
    if(logicLevel == false) 
        GPIO2->PSOR |= (1<<7);
    if(logicLevel == true) 
        GPIO2->PCOR |= (1<<7);
}
