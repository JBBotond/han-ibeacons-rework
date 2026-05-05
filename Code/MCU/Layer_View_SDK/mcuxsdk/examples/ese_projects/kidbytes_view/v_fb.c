#include "v_fb.h"
#include "leds.h"
#include <MCXA153.h>

extern volatile uint32_t ms;

/* Buzzer on P3_6 (active buzzer TMB12A05: HIGH = sound) */
#define BUZZER_PIN  6

static void delay_ms(uint32_t d)
{
    uint32_t start = ms;
    while ((ms - start) < d) {}
}

void v_fb_init(void)
{
    leds_init();
    led_green_off();

    /* Enable PORT3 and GPIO3 clocks (may already be enabled by leds_init) */
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);

    /* P3_6 as GPIO output */
    PORT3->PCR[BUZZER_PIN] = PORT_PCR_MUX(0); /* ALT0 = GPIO */
    GPIO3->PDOR &= ~(1U << BUZZER_PIN);       /* start LOW (off) */
    GPIO3->PDDR |= (1U << BUZZER_PIN);        /* output */
}

void v_fb_puzzle_cue(void)
{
    led_green_on();
    delay_ms(10);
    led_green_off();
}

void v_fb_unlock_cue(void)
{
    /* Buzzer on for 200 ms + LED blink */
    led_blue_on();
    GPIO3->PSOR = (1U << BUZZER_PIN);
    delay_ms(200);
    GPIO3->PCOR = (1U << BUZZER_PIN);
    led_blue_off();
}
