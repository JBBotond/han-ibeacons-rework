/*
 * Bluetooth Blinky Control - ON/OFF commands
 */

#include "MCXA153.h"

void lpuart2_init(void);
char lpuart2_getchar(void);
uint8_t lpuart2_data_available(void);
void gpio_init(void);
void delay_ms(uint32_t ms);

int main(void)
{
    gpio_init();
    lpuart2_init();
    
    char buffer[4];
    uint8_t idx = 0;
    uint8_t blink_enabled = 0;
    uint32_t last_toggle = 0;
    
    while(1)
    {
        // Check for commands
        if(lpuart2_data_available())
        {
            char c = lpuart2_getchar();
            
            if(c == '\r' || c == '\n')
            {
                if(idx == 2 && buffer[0] == 'O' && buffer[1] == 'N')
                {
                    blink_enabled = 1;
                }
                else if(idx == 3 && buffer[0] == 'O' && buffer[1] == 'F' && buffer[2] == 'F')
                {
                    blink_enabled = 0;
                    GPIO3->PSOR = (1<<12);  // LED OFF
                }
                idx = 0;
            }
            else if(idx < 3)
            {
                buffer[idx++] = c;
            }
        }
        
        // Blink if enabled
        if(blink_enabled)
        {
            if(++last_toggle >= 50000)
            {
                GPIO3->PTOR = (1<<12);
                last_toggle = 0;
            }
        }
    }
}

void lpuart2_init(void)
{
    MRCC0->MRCC_LPUART2_CLKSEL = MRCC_MRCC_LPUART2_CLKSEL_MUX(0b010);
    MRCC0->MRCC_LPUART2_CLKDIV = 0;
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART2(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPUART2(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);
    
    PORT1->PCR[4] = PORT_PCR_LK(1) | PORT_PCR_MUX(3) | PORT_PCR_IBE(1);
    PORT1->PCR[5] = PORT_PCR_LK(1) | PORT_PCR_MUX(3);
    
    LPUART2->BAUD = LPUART_BAUD_OSR(0b01111) | LPUART_BAUD_SBR(312);
    LPUART2->CTRL = LPUART_CTRL_TE(1) | LPUART_CTRL_RE(1);
}

char lpuart2_getchar(void)
{
    while(!(LPUART2->STAT & LPUART_STAT_RDRF_MASK));
    return (char)(LPUART2->DATA & 0xFF);
}

uint8_t lpuart2_data_available(void)
{
    return (LPUART2->STAT & LPUART_STAT_RDRF_MASK) ? 1 : 0;
}

void gpio_init(void)
{
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    
    PORT3->PCR[12] = PORT_PCR_LK(1) | PORT_PCR_MUX(0);
    GPIO3->PSOR = (1<<12);
    GPIO3->PDDR |= (1<<12);
}
