#include "v_fb.h"
#include "leds.h"

extern volatile uint32_t ms;

void v_fb_init(void)
{
    leds_init();
    led_green_off();
}

void v_fb_puzzle_cue(void)
{
    led_green_on();
    uint32_t start = ms;
    while ((ms - start) < 10) {}
    led_green_off();
}
