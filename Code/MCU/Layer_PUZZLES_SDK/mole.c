#include "mole.h"
#include <stdlib.h>

extern volatile uint32_t ms;

/* -----------------------------------------------------------------------
 * Game constants
 * --------------------------------------------------------------------- */
#define GAME_MS       30000u
#define VISIBLE_MS     1500u
#define GAP_MS          400u
#define HIT_FLASH_MS    350u
#define TOPBAR_PERIOD   250u
#define WIN_SCORE          10   /* need this many hits to win            */

#define MOLE_W      60
#define MOLE_HEIGHT 50
#define FIELD_Y     36          /* topbar occupies y = 0..35             */

/* Fixed WHACK! banner — always drawn at the same spot so erase is exact.
 * Positioned just inside the field, above the first stripe (y = 91).   */
#define WHACK_X     90
#define WHACK_Y     (FIELD_Y + 5)   /* = 41 */
#define WHACK_W    130
#define WHACK_H     24

/* Colours */
#define C_GRASS   RGB(3, 38, 4)
#define C_TOPBAR  RGB(5,  3, 1)
#define C_ENDSCR  RGB(3, 18, 5)
#define C_LOSESCR RGB(22, 4, 2)

/* -----------------------------------------------------------------------
 * State
 * --------------------------------------------------------------------- */
typedef enum { WAM_COUNTDOWN, WAM_PLAYING, WAM_HIT, WAM_END, WAM_LOSE } wam_state_t;

static wam_state_t state;
static uint32_t    state_timer;
static uint32_t    game_start;
static uint32_t    bar_timer;
static int         score;
static uint16_t    mole_x, mole_y;
static bool        mole_on;
static int         countdown_n;
static bool        end_touched;

/* -----------------------------------------------------------------------
 * fill_rect helper
 * --------------------------------------------------------------------- */
static void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    for (uint32_t k = 0; k < (uint32_t)LCD_FRAMEBUFFER_WIDTH * LCD_FRAMEBUFFER_HEIGHT; k++)
        lcd_framebuffer[k] = color;
    for (uint16_t ty = 0; ty < h; ty += LCD_FRAMEBUFFER_HEIGHT) {
        uint16_t th = (h - ty < LCD_FRAMEBUFFER_HEIGHT) ? (h - ty) : LCD_FRAMEBUFFER_HEIGHT;
        for (uint16_t tx = 0; tx < w; tx += LCD_FRAMEBUFFER_WIDTH) {
            uint16_t tw = (w - tx < LCD_FRAMEBUFFER_WIDTH) ? (w - tx) : LCD_FRAMEBUFFER_WIDTH;
            lcd_set_area(x + tx, y + ty, tw, th);
            lcd_write_pixels(lcd_framebuffer, (uint32_t)tw * th);
        }
    }
}

/* Redraw the three decorative stripe lines.  Must be called after any
 * fill that covers part of the field, so stripes are never erased.     */
static void restore_stripes(void)
{
    fill_rect(0, FIELD_Y + 55,  320, 4, RGB(2, 28, 3));
    fill_rect(0, FIELD_Y + 113, 320, 4, RGB(2, 28, 3));
    fill_rect(0, FIELD_Y + 163, 320, 4, RGB(2, 28, 3));
}

/* -----------------------------------------------------------------------
 * Mole sprite
 * --------------------------------------------------------------------- */
static void draw_mole(bool show)
{
    if (!show) {
        fill_rect(mole_x, mole_y, MOLE_W, MOLE_HEIGHT, C_GRASS);
        restore_stripes();   /* repair any stripe the mole was sitting on */
        return;
    }

    uint16_t dk   = RGB(13,  7,  2);
    uint16_t br   = RGB(20, 12,  4);
    uint16_t tan  = RGB(26, 22, 12);
    uint16_t pink = RGB_PINK;

    fill_rect(mole_x+8,  mole_y,    44,  8, dk);
    fill_rect(mole_x+2,  mole_y+7,  56, 36, dk);
    fill_rect(mole_x+8,  mole_y+43, 44,  7, dk);
    fill_rect(mole_x+5,  mole_y+7,  50, 34, br);
    fill_rect(mole_x+10, mole_y+9,  40, 28, tan);

    fill_rect(mole_x+14, mole_y+13,  7, 6, RGB_BLACK);
    fill_rect(mole_x+39, mole_y+13,  7, 6, RGB_BLACK);
    fill_rect(mole_x+15, mole_y+14,  3, 3, RGB_WHITE);
    fill_rect(mole_x+40, mole_y+14,  3, 3, RGB_WHITE);

    fill_rect(mole_x+23, mole_y+22, 14,  9, pink);
    fill_rect(mole_x+25, mole_y+24,  4,  5, dk);
    fill_rect(mole_x+31, mole_y+24,  4,  5, dk);

    fill_rect(mole_x+18, mole_y+33, 24,  3, dk);
    fill_rect(mole_x+18, mole_y+36,  5,  3, dk);
    fill_rect(mole_x+37, mole_y+36,  5,  3, dk);
}

/* -----------------------------------------------------------------------
 * Field background
 * --------------------------------------------------------------------- */
static void draw_field(void)
{
    fill_rect(0, FIELD_Y, 320, 240 - FIELD_Y, C_GRASS);
    restore_stripes();
}

/* -----------------------------------------------------------------------
 * Top bar — score on left, TIME: label, then shrinking bar on right.
 * The label and bar are SEPARATE so the bar never covers the text.
 * --------------------------------------------------------------------- */
static void draw_topbar(void)
{
    fill_rect(0, 0, 320, FIELD_Y, C_TOPBAR);

    lcd_set_font(Dialog_plain_12);

    /* Score */
    char s[] = "HIT: 00";
    s[5] = (score >= 10) ? ('0' + score / 10) : ' ';
    s[6] = '0' + score % 10;
    lcd_put_string(5, 12, s, RGB_YELLOW, C_TOPBAR);

    /* TIME: label — to the LEFT of the bar, not inside it */
    lcd_put_string(115, 12, "TIME:", RGB_WHITE, C_TOPBAR);

    /* Timer bar: x=165 to x=315 (150 px) */
    uint32_t elapsed = ms - game_start;
    uint32_t rem     = (elapsed >= GAME_MS) ? 0 : (GAME_MS - elapsed);
    uint16_t bar_w   = (uint16_t)((uint32_t)rem * 150u / GAME_MS);
    uint16_t bar_col = (bar_w > 100) ? RGB_GREEN : (bar_w > 50 ? RGB_YELLOW : RGB_RED);

    fill_rect(165, 8, 150, 20, RGB(8, 4, 4));        /* dark trough        */
    if (bar_w > 0)
        fill_rect(165, 8, bar_w, 20, bar_col);        /* remaining-time fill */
}

/* -----------------------------------------------------------------------
 * Countdown panel
 * --------------------------------------------------------------------- */
static void draw_countdown(void)
{
    uint16_t bg = RGB(0, 10, 20);
    fill_rect(85, 70, 150, 110, bg);

    lcd_set_font(Dialog_bold_16);
    lcd_put_string(108, 82, "Get Ready!", RGB_WHITE, bg);

    lcd_set_font(Monospaced_bold_32);
    if (countdown_n > 0) {
        char c[2] = { '0' + countdown_n, '\0' };
        lcd_put_string(148, 112, c, RGB_YELLOW, bg);
    } else {
        lcd_put_string(107, 112, "GO!", RGB_GREEN, bg);
    }
}

/* -----------------------------------------------------------------------
 * Win end screen (score >= WIN_SCORE)
 * --------------------------------------------------------------------- */
static void draw_end_screen(void)
{
    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);
    fill_rect(0, 0, 320, 240, C_ENDSCR);
    fill_rect(0,   0, 320, 10, RGB_GREEN);
    fill_rect(0, 230, 320, 10, RGB_GREEN);

    lcd_set_font(Dialog_bold_16);
    lcd_put_string(75, 28, "TIME'S UP!", RGB_YELLOW, C_ENDSCR);

    lcd_set_font(Monospaced_bold_32);
    char sc[4];
    if (score >= 10) { sc[0] = '0' + score/10; sc[1] = '0' + score%10; sc[2] = '\0'; }
    else             { sc[0] = '0' + score;     sc[1] = '\0'; }
    lcd_put_string(140, 78, sc, RGB_YELLOW, C_ENDSCR);

    lcd_set_font(Dialog_bold_16);
    lcd_put_string(95, 125, "MOLES HIT!", RGB_WHITE, C_ENDSCR);

    lcd_set_font(Dialog_plain_12);
    if      (score >= 20) lcd_put_string(55, 168, "WOW! You're a pro!",        RGB_YELLOW, C_ENDSCR);
    else if (score >= 15) lcd_put_string(70, 168, "Great job! Well done!",     RGB_GREEN,  C_ENDSCR);
    else                  lcd_put_string(55, 168, "You made it! Nice work!",   RGB_WHITE,  C_ENDSCR);

    lcd_put_string(60, 210, "Touch for Simon Says!", RGB_GRAY, C_ENDSCR);
}

/* -----------------------------------------------------------------------
 * Lose screen (score < WIN_SCORE)
 * --------------------------------------------------------------------- */
static void draw_lose_screen(void)
{
    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);
    fill_rect(0, 0, 320, 240, C_LOSESCR);
    fill_rect(0,   0, 320, 10, RGB_RED);
    fill_rect(0, 230, 320, 10, RGB_RED);

    lcd_set_font(Dialog_bold_16);
    lcd_put_string(75, 25, "TIME'S UP!", RGB_YELLOW, C_LOSESCR);

    lcd_set_font(Monospaced_bold_32);
    char sc[4];
    if (score >= 10) { sc[0] = '0' + score/10; sc[1] = '0' + score%10; sc[2] = '\0'; }
    else             { sc[0] = '0' + score;     sc[1] = '\0'; }
    lcd_put_string(140, 72, sc, RGB_RED, C_LOSESCR);

    lcd_set_font(Dialog_bold_16);
    lcd_put_string(95, 118, "MOLES HIT", RGB_WHITE,  C_LOSESCR);

    lcd_set_font(Dialog_plain_12);
    lcd_put_string(60, 156, "Need 10 or more to win!", RGB_YELLOW, C_LOSESCR);
    lcd_put_string(95, 176, "Try Again!",              RGB_ORANGE, C_LOSESCR);

    lcd_put_string(60, 210, "Touch to play again!",   RGB_GRAY,   C_LOSESCR);
}

/* -----------------------------------------------------------------------
 * Random mole placement
 * --------------------------------------------------------------------- */
static void place_mole(void)
{
    srand(ms);
    mole_x = (uint16_t)(rand() % (320 - MOLE_W));
    mole_y = (uint16_t)(FIELD_Y + rand() % (240 - FIELD_Y - MOLE_HEIGHT));
}

/* -----------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

void mole_init(void)
{
    state       = WAM_COUNTDOWN;
    score       = 0;
    mole_on     = false;
    countdown_n = 3;
    end_touched = false;
    game_start  = 0;
    state_timer = ms;
    bar_timer   = 0;

    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);
    draw_field();
    draw_countdown();
}

void mole_input_detect(void)
{
    /* Touch on lose screen restarts the game immediately */
    if (state == WAM_LOSE) { mole_init(); return; }

    if (state == WAM_END)  { end_touched = true; return; }

    if (state != WAM_PLAYING || !mole_on) return;

    /* Map raw ADC (0-4095) → screen pixels.
     * Center: MIDDLE_X=2048 → screen x=160, so scale = 5/64.           */
    uint16_t sx = (uint16_t)((uint32_t)lcd_touch_x * 5u >> 6);
    uint16_t sy = (uint16_t)((uint32_t)lcd_touch_y * 15u >> 8);

    if (sx >= mole_x && sx < mole_x + MOLE_W &&
        sy >= mole_y && sy < mole_y + MOLE_HEIGHT) {

        score++;
        mole_on = false;
        draw_mole(false);   /* erase mole + restore_stripes inside */

        /* WHACK! banner at fixed position — erase will use the exact
         * same coordinates, so no residual yellow.                      */
        fill_rect(WHACK_X, WHACK_Y, WHACK_W, WHACK_H, RGB_YELLOW);
        lcd_set_font(Dialog_bold_16);
        lcd_put_string(WHACK_X + 5, WHACK_Y + 4, "WHACK!", RGB_BLACK, RGB_YELLOW);

        state       = WAM_HIT;
        state_timer = ms;
        draw_topbar();
    }
}

/* Returns true only when the player won (score >= WIN_SCORE) and the
 * end-screen timeout/touch has fired, signalling the host to start
 * Simon Says.  Returns false for the lose path (game retries itself). */
bool mole_game_tick(void)
{
    uint32_t now = ms;

    /* ---- Lose screen: touch handled in mole_input_detect, auto-retry 4 s ---- */
    if (state == WAM_LOSE) {
        if (now - state_timer >= 4000u) mole_init();
        return false;
    }

    /* ---- Win end screen ---- */
    if (state == WAM_END) return (end_touched || now - state_timer >= 5000u);

    /* ---- Global 30 s timer ---- */
    if (state != WAM_COUNTDOWN && now - game_start >= GAME_MS) {
        if (mole_on) { draw_mole(false); mole_on = false; }
        state_timer = now;
        if (score >= WIN_SCORE) {
            state = WAM_END;
            draw_end_screen();
        } else {
            state = WAM_LOSE;
            draw_lose_screen();
        }
        return false;
    }

    /* ---- Per-state logic ---- */
    switch (state) {

        case WAM_COUNTDOWN:
            if (now - state_timer >= 1000u) {
                state_timer = now;
                if (countdown_n > 0) {
                    countdown_n--;
                    draw_countdown();
                } else {
                    state       = WAM_PLAYING;
                    game_start  = now;
                    bar_timer   = now;
                    state_timer = now;
                    draw_field();
                    draw_topbar();
                }
            }
            break;

        case WAM_PLAYING:
            if (!mole_on && now - state_timer >= GAP_MS) {
                place_mole();
                draw_mole(true);
                mole_on     = true;
                state_timer = now;
            }
            if (mole_on && now - state_timer >= VISIBLE_MS) {
                draw_mole(false);
                mole_on     = false;
                state_timer = now;
            }
            break;

        case WAM_HIT:
            if (now - state_timer >= HIT_FLASH_MS) {
                /* Erase WHACK! banner with exactly the same rect that was drawn */
                fill_rect(WHACK_X, WHACK_Y, WHACK_W, WHACK_H, C_GRASS);
                state       = WAM_PLAYING;
                mole_on     = false;
                state_timer = now;
            }
            break;

        case WAM_END:
        case WAM_LOSE:
            break;
    }

    /* Throttled topbar refresh during play */
    if ((state == WAM_PLAYING || state == WAM_HIT) && now - bar_timer >= TOPBAR_PERIOD) {
        bar_timer = now;
        draw_topbar();
    }

    return false;
}
