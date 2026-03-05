/*
 * Simple Blinky - Red LED D15 (P3_0) toggles every 1 second
 */

#include "MCXA153.h"

#define RED_LED_PIN  0    // P3_0 = D15 RGB Red

volatile uint32_t tick_count = 0;

void SysTick_Handler(void)
{
    tick_count++;

    if(tick_count >= 1000)
    {
        GPIO3->PTOR  = (1 << RED_LED_PIN);  // toggle every 1000ms
        tick_count = 0;
    }
}

void gpio_init(void)
{
    // Enable clock and release from reset for GPIO3 and PORT3
    MRCC0->MRCC_GLB_CC1_SET  = MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_CC1_SET  = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);

    // P3_0 = GPIO (MUX=0), locked
    PORT3->PCR[RED_LED_PIN] = PORT_PCR_LK(1) | PORT_PCR_MUX(0);

    // LED OFF initially (active low = high)
    GPIO3->PSOR = (1 << RED_LED_PIN);

    // Set as output
    GPIO3->PDDR |= (1 << RED_LED_PIN);
}

int main(void)
{
    gpio_init();

    // SysTick: 96MHz / 96000 = 1ms per tick
    SysTick->LOAD = 96000 - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk    |
                    SysTick_CTRL_ENABLE_Msk;

    while(1) {}
}
