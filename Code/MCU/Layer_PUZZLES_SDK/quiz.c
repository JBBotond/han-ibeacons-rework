#include "quiz.h"
#include <stdlib.h>

extern volatile uint32_t ms;

/* -----------------------------------------------------------------------
 * Touch zones — same axis as RPS: left / middle / right
 * --------------------------------------------------------------------- */
#define ZONE_L  1365
#define ZONE_R  2730

/* Screen split:
 *   y  0-69  : question header (dark background)
 *   y 70-239 : three answer columns
 */
#define HEADER_H   70
#define COL_H     (240 - HEADER_H)   /* 170 px */

/* Column colours */
#define C_LEFT   RGB(2,  8, 28)   /* dark blue   */
#define C_MID    RGB(2, 32,  4)   /* dark green  */
#define C_RIGHT  RGB(28, 10,  2)  /* dark orange */
#define C_HEAD   RGB(3,  3, 18)   /* dark navy   */

/* -----------------------------------------------------------------------
 * Question pool  (question string, three options, correct option index)
 * --------------------------------------------------------------------- */
typedef struct { const char *q; int opts[3]; int correct; } quiz_q_t;

static const quiz_q_t pool[] = {
    { "2 + 3 = ?",   { 5,  7,  4 }, 0 },
    { "4 + 5 = ?",   { 8,  9,  7 }, 1 },
    { "6 + 7 = ?",   { 12, 13, 11}, 1 },
    { "3 + 8 = ?",   { 10, 9,  11}, 2 },
    { "10 - 4 = ?",  { 8,  6,  3 }, 1 },
    { "7 - 3 = ?",   { 5,  4,  2 }, 1 },
    { "9 - 5 = ?",   { 3,  5,  4 }, 2 },
    { "3 x 3 = ?",   { 9,  6,  12}, 0 },
    { "2 x 5 = ?",   { 8,  10, 7 }, 1 },
    { "4 x 2 = ?",   { 6,  9,  8 }, 2 },
    { "10 / 2 = ?",  { 5,  3,  7 }, 0 },
    { "6 / 2 = ?",   { 4,  3,  6 }, 1 },
    { "9 / 3 = ?",   { 4,  2,  3 }, 2 },
    { "5 + 6 = ?",   { 10, 12, 11}, 2 },
    { "8 - 2 = ?",   { 5,  7,  6 }, 2 },
};
#define POOL_SIZE  15

/* -----------------------------------------------------------------------
 * State
 * --------------------------------------------------------------------- */
typedef enum { QZ_SHOW, QZ_FEEDBACK } quiz_state_t;

static quiz_state_t state;
static uint32_t     feedback_timer;
static bool         feedback_correct;
static int          correct_count;       /* how many answered right so far  */
static int          sel[3];              /* indices into pool for this round */
static int          cur;                 /* which question we are on (0-2)  */

/* -----------------------------------------------------------------------
 * Helpers
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

/* Write a small integer (0-99) centred inside a column. */
static void put_number_centred(int n, uint16_t col_x, uint16_t col_w, uint16_t bg)
{
    char s[4];
    if (n >= 10) { s[0] = '0' + n / 10; s[1] = '0' + n % 10; s[2] = '\0'; }
    else         { s[0] = '0' + n;       s[1] = '\0'; }

    /* Monospaced_bold_32: each char ~22 px wide */
    uint16_t text_w = (n >= 10) ? 44u : 22u;
    uint16_t x = col_x + (col_w > text_w ? (col_w - text_w) / 2u : 0u);
    uint16_t y = HEADER_H + (COL_H - 32u) / 2u;   /* vertically centred in column */

    lcd_set_font(Monospaced_bold_32);
    lcd_put_string(x, y, s, RGB_WHITE, bg);
}

/* -----------------------------------------------------------------------
 * Screen drawing
 * --------------------------------------------------------------------- */
static void draw_question(void)
{
    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);

    const quiz_q_t *q = &pool[sel[cur]];

    /* ---- Header ---- */
    fill_rect(0, 0, 320, HEADER_H, C_HEAD);

    /* Progress: "Q 1/3", "Q 2/3", "Q 3/3" */
    lcd_set_font(Dialog_plain_12);
    char prog[] = "Q 1 / 3";
    prog[2] = '1' + cur;
    lcd_put_string(5, 5, prog, RGB_GRAY, C_HEAD);

    /* Bright accent strip at bottom of header */
    fill_rect(0, HEADER_H - 6, 320, 6, RGB_YELLOW);

    /* Question text — large, centred */
    lcd_set_font(Dialog_bold_16);
    /* Estimate width: ~12 px per char in bold_16 */
    uint16_t qlen = 0;
    for (const char *p = q->q; *p; p++) qlen++;
    uint16_t qx = (320u > qlen * 12u) ? (320u - qlen * 12u) / 2u : 0u;
    lcd_put_string(qx, 22, q->q, RGB_YELLOW, C_HEAD);

    /* ---- Three answer columns ---- */
    fill_rect(0,   HEADER_H, 107, COL_H, C_LEFT);
    fill_rect(107, HEADER_H, 107, COL_H, C_MID);
    fill_rect(214, HEADER_H, 106, COL_H, C_RIGHT);

    /* Bright accent strip at top of each column */
    fill_rect(0,   HEADER_H, 107, 6, RGB_BLUE);
    fill_rect(107, HEADER_H, 107, 6, RGB_GREEN);
    fill_rect(214, HEADER_H, 106, 6, RGB_ORANGE);

    put_number_centred(q->opts[0], 0,   107, C_LEFT);
    put_number_centred(q->opts[1], 107, 107, C_MID);
    put_number_centred(q->opts[2], 214, 106, C_RIGHT);

    /* TAP hints at the bottom */
    lcd_set_font(Dialog_plain_12);
    lcd_put_string(30,  225, "TAP!", RGB(10, 30, 31), C_LEFT);
    lcd_put_string(137, 225, "TAP!", RGB(10, 31, 10), C_MID);
    lcd_put_string(233, 225, "TAP!", RGB(31, 20, 10), C_RIGHT);
}

static void draw_correct(void)
{
    uint16_t bg = RGB(3, 20, 3);
    fill_rect(0, 0, 320, 240, bg);
    fill_rect(0,   0, 320, 10, RGB_GREEN);
    fill_rect(0, 230, 320, 10, RGB_GREEN);

    lcd_set_font(Dialog_bold_16);
    lcd_put_string(20,  35, "*", RGB_YELLOW, bg);
    lcd_put_string(290, 35, "*", RGB_YELLOW, bg);

    /* Shadow then bright text */
    lcd_put_string(87, 97,  "CORRECT!", RGB(1, 10, 1), bg);
    lcd_put_string(85, 95,  "CORRECT!", RGB_YELLOW,    bg);

    if (cur + 1 < 3) {
        lcd_set_font(Dialog_plain_12);
        lcd_put_string(90, 138, "Next question!", RGB_WHITE, bg);
    } else {
        lcd_set_font(Dialog_plain_12);
        lcd_put_string(65, 138, "All done! Amazing work!", RGB_WHITE, bg);
    }
}

static void draw_wrong(void)
{
    uint16_t bg = RGB(22, 8, 2);
    fill_rect(0, 0, 320, 240, bg);
    fill_rect(0,   0, 320, 10, RGB_ORANGE);
    fill_rect(0, 230, 320, 10, RGB_ORANGE);

    lcd_set_font(Dialog_bold_16);
    lcd_put_string(107, 77, "OOPS!",      RGB(10, 3, 0), bg);
    lcd_put_string(105, 75, "OOPS!",      RGB_YELLOW,    bg);
    lcd_put_string(60,  110, "Try Again!", RGB_WHITE,    bg);
}

/* -----------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

void quiz_init(void)
{
    correct_count = 0;
    cur           = 0;
    state         = QZ_SHOW;

    /* Pick 3 distinct questions from the pool */
    srand(ms);
    bool used[POOL_SIZE] = {false};
    for (int i = 0; i < 3; i++) {
        int idx;
        do { idx = rand() % POOL_SIZE; } while (used[idx]);
        used[idx] = true;
        sel[i]    = idx;
    }

    draw_question();
}

void quiz_input_detect(void)
{
    if (state != QZ_SHOW) return;

    int tapped;
    if      (lcd_touch_x < ZONE_L) tapped = 0;
    else if (lcd_touch_x < ZONE_R) tapped = 1;
    else                            tapped = 2;

    feedback_correct = (tapped == pool[sel[cur]].correct);
    state            = QZ_FEEDBACK;
    feedback_timer   = ms;

    if (feedback_correct) {
        correct_count++;
        draw_correct();
    } else {
        draw_wrong();
    }
}

/* Returns true when all 3 questions answered correctly. */
bool quiz_game_tick(void)
{
    if (state != QZ_FEEDBACK) return false;

    if (ms - feedback_timer >= 1200u) {
        if (feedback_correct) {
            cur++;
            if (cur >= 3) return true;   /* quiz complete */
        }
        state = QZ_SHOW;
        draw_question();
    }
    return false;
}
