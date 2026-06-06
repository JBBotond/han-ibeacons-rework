/*! ***************************************************************************
 * \brief  rssi_bar — Layer 6 View: "hotter/colder" proximity HUD
 * \file   rssi_bar.c
 *
 * Adopted verbatim (logic) from a colleague's SDK modification. Only the bar
 * height (4 -> 6 px) and the file header were changed to fit the KidBytes
 * View layer; the smoothing math and color mapping are the colleague's.
 *
 * Behaviour: an 8-sample rolling average of RSSI drives a bar across the top
 * of the 320x240 landscape screen. Blue = far, Red = close. The kid walks
 * "hotter" (bar fills, turns red) or "colder" (bar empties, turns blue).
 *****************************************************************************/
#include "rssi_bar.h"
#include "tft_lcd.h"

/* Top strip of the 320x240 landscape display */
#define BAR_Y        0
#define BAR_H        6
#define BAR_W      320

/* RSSI range: -90 dBm = empty bar (far), -30 dBm = full bar (close) */
#define RSSI_FAR   (-90)
#define RSSI_CLOSE (-30)

/* Rolling average over the last N readings (N x 250 ms = 2 s of history).
 * Body-shadowing and orientation dips typically last < 500 ms, so 8 samples
 * smooths them out while still tracking a genuine position change. */
#define SMOOTH_SAMPLES 8

static int8_t  g_samples[SMOOTH_SAMPLES];
static uint8_t g_head = 0;       /* next write slot                         */
static uint8_t g_count = 0;      /* how many valid samples are in the ring  */

void rssi_bar_reset(void)
{
    for (uint8_t i = 0; i < SMOOTH_SAMPLES; i++)
        g_samples[i] = (int8_t)RSSI_FAR;
    g_head  = 0;
    g_count = SMOOTH_SAMPLES;  /* buffer full of "far" readings */
}

void rssi_bar_update(int8_t rssi)
{
    g_samples[g_head] = rssi;
    g_head = (g_head + 1) % SMOOTH_SAMPLES;
    if (g_count < SMOOTH_SAMPLES) g_count++;
}

static int8_t smoothed_rssi(void)
{
    if (g_count == 0) return (int8_t)RSSI_FAR;

    int32_t sum = 0;
    for (uint8_t i = 0; i < g_count; i++)
        sum += g_samples[i];
    return (int8_t)(sum / (int32_t)g_count);
}

void rssi_bar_draw(void)
{
    int8_t rssi = smoothed_rssi();

    int range  = RSSI_CLOSE - RSSI_FAR;   /* 60 */
    int offset = (int)rssi - RSSI_FAR;
    if (offset < 0)     offset = 0;
    if (offset > range) offset = range;

    /* fill = 0 (far/blue) .. 320 (close/red) */
    uint16_t fill      = (uint16_t)((uint32_t)offset * BAR_W / range);
    uint16_t bar_color = (fill < BAR_W / 2) ? RGB_BLUE : RGB_RED;
    uint16_t trough    = RGB(4, 4, 4);   /* near-black background */

    /* Draw full trough — 320x6 = 1920 px < 80x80 = 6400 framebuffer capacity */
    for (uint32_t k = 0; k < (uint32_t)BAR_W * BAR_H; k++)
        lcd_framebuffer[k] = trough;
    lcd_set_area(0, BAR_Y, BAR_W, BAR_H);
    lcd_write_pixels(lcd_framebuffer, (uint32_t)BAR_W * BAR_H);

    /* Overlay the filled (proximity) portion on top */
    if (fill > 0)
    {
        for (uint32_t k = 0; k < (uint32_t)fill * BAR_H; k++)
            lcd_framebuffer[k] = bar_color;
        lcd_set_area(0, BAR_Y, fill, BAR_H);
        lcd_write_pixels(lcd_framebuffer, (uint32_t)fill * BAR_H);
    }
}
