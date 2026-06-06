#include "v_screen.h"
#include "display_drv.h"
#include "fonts.h"
#include <stdio.h>

/* X offset where the value text starts (after "Puzzle: ") */
#define VAL_X  80

/* Top margin (px) reserved for the rssi_bar proximity HUD (6 px bar + gap).
 * The bar lives at y=0..5; the text rows start below it so they never clash. */
#define TOP_MARGIN  12

void v_screen_draw(void)
{
    display_clear(RGB_BLACK);

    lcd_set_font(Dialog_bold_16);
    uint8_t h = Dialog_bold_16[1];

    display_put_string(0, TOP_MARGIN,           "Room:",   RGB_WHITE, RGB_BLACK);
    display_put_string(0, TOP_MARGIN + 2 * h,   "Next:",   RGB_WHITE, RGB_BLACK);
    display_put_string(0, TOP_MARGIN + 4 * h,   "Puzzle:", RGB_WHITE, RGB_BLACK);
}

void v_screen_update(uint8_t room, uint8_t next, const char *puzzle_hint)
{
    char buf[16];
    lcd_set_font(Dialog_bold_16);
    uint8_t h = Dialog_bold_16[1];

    /* Display is human-facing: room indices are 0-based internally, but kids
     * and judges read 1-based "Room 1..5". Add 1 here only for the screen. */
    snprintf(buf, sizeof(buf), "%u   ", (unsigned)(room + 1));
    display_put_string(VAL_X, TOP_MARGIN, buf, RGB_GREEN, RGB_BLACK);

    snprintf(buf, sizeof(buf), "%u   ", (unsigned)(next + 1));
    display_put_string(VAL_X, TOP_MARGIN + 2 * h, buf, RGB_CYAN, RGB_BLACK);

    snprintf(buf, sizeof(buf), "%-10s", puzzle_hint);
    display_put_string(VAL_X, TOP_MARGIN + 4 * h, buf, RGB_YELLOW, RGB_BLACK);
}

/* -------------------------------------------------------------------------
 * Waypoint guidance screen — NO game info, just big HOTTER/COLDER words that
 * track the kid's distance to the transition beacon (driven by RSSI).
 * 5 warmth levels from filtered RSSI; only redraws when the level changes so
 * the screen doesn't flicker every scan.
 * ---------------------------------------------------------------------- */
static int8_t s_wp_last_level = -1;

void v_screen_waypoint_reset(void)
{
    s_wp_last_level = -1;   /* force a full redraw on the next call */
}

void v_screen_waypoint(int8_t rssi)
{
    /* Map RSSI to a 0..4 warmth level. Tune the edges on the bench. */
    int8_t level;
    if      (rssi >= -58) level = 4;   /* basically on top of it  */
    else if (rssi >= -66) level = 3;   /* hot                     */
    else if (rssi >= -74) level = 2;   /* warm                    */
    else if (rssi >= -82) level = 1;   /* cool                    */
    else                  level = 0;   /* cold / far              */

    if (level == s_wp_last_level) return;   /* no change → no redraw */
    s_wp_last_level = level;

    const char *word;
    uint16_t color;
    switch (level)
    {
    case 4: word = "RED HOT!";  color = RGB_RED;    break;
    case 3: word = "HOTTER";    color = RGB_ORANGE; break;
    case 2: word = "WARM";      color = RGB_YELLOW; break;
    case 1: word = "COLDER";    color = RGB_CYAN;   break;
    default:word = "COLD";      color = RGB_BLUE;   break;
    }

    /* Clear, then draw the big word + a hint line. The proximity bar is drawn
     * separately by the scan task right after this. */
    display_clear(RGB_BLACK);
    lcd_set_font(Dialog_bold_16);
    display_put_string(90, 90,  "FIND THE NEXT SPOT", RGB_WHITE, RGB_BLACK);
    display_put_string(110, 120, word, color, RGB_BLACK);
    display_put_string(40, 150,
        (level >= 3) ? "You're nearly there!"
                     : (level >= 1) ? "Keep moving..."
                                    : "Too far - turn around", RGB_GRAY, RGB_BLACK);
}
