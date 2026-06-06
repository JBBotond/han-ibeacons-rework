#include "start.h"
#include <stdlib.h>

extern volatile uint32_t ms;

/* KidBytes integration: set true on a correct round so the puzzle plugin
 * adapter (puzzle_registry.c) can report the puzzle solved to the FSM. */
extern bool g_simon_solved;

#define PATTERN_SIZE 4
#define SHOW_MS      1000

// Touch thresholds — same axis as RPS game: x=horizontal, y=vertical
#define MIDDLE_X  2048
#define MIDDLE_Y  2048

// Screen (ORIENTATION_90): 320w x 240h
#define HALF_W 160
#define HALF_H 120

typedef enum { RED, GREEN, YELLOW, BLUE } color_e;
typedef enum { GAME_SHOW, GAME_INPUT, GAME_WIN, GAME_LOSE } game_state_t;

static const uint16_t bright[4] = { RGB_RED,        RGB_GREEN,     RGB_YELLOW,     RGB_BLUE      };
static const uint16_t dim[4]    = { RGB(16, 0,  0), RGB(0, 32, 0), RGB(16, 32, 0), RGB(0, 0, 16) };

static color_e      gen_pattern[PATTERN_SIZE];
static color_e      user_input[PATTERN_SIZE];
static int          input_idx = 0;
static int          show_idx  = 0;
static uint32_t     timer     = 0;
static game_state_t state     = GAME_SHOW;

// Fill a rectangle by tiling 80x80 blocks (same approach as lcd_clear)
// Each lcd_write_pixels call resets the address to start of window (0x2C),
// so we must set a new area for each tile.
static void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    for (uint32_t k = 0; k < (uint32_t)LCD_FRAMEBUFFER_WIDTH * LCD_FRAMEBUFFER_HEIGHT; k++) {
        lcd_framebuffer[k] = color;
    }
    for (uint16_t ty = 0; ty < h; ty += LCD_FRAMEBUFFER_HEIGHT) {
        uint16_t th = (h - ty < LCD_FRAMEBUFFER_HEIGHT) ? (h - ty) : LCD_FRAMEBUFFER_HEIGHT;
        for (uint16_t tx = 0; tx < w; tx += LCD_FRAMEBUFFER_WIDTH) {
            uint16_t tw = (w - tx < LCD_FRAMEBUFFER_WIDTH) ? (w - tx) : LCD_FRAMEBUFFER_WIDTH;
            lcd_set_area(x + tx, y + ty, tw, th);
            lcd_write_pixels(lcd_framebuffer, (uint32_t)tw * th);
        }
    }
}

// Full-screen color flash with step number in center
static void show_color_screen(int idx) {
    uint16_t bg = bright[gen_pattern[idx]];
    lcd_clear(bg);
    lcd_orientation(ORIENTATION_90);
    lcd_set_font(Dialog_bold_16);
    char num[2] = { '1' + idx, '\0' };
    lcd_put_string(155, 111, num, RGB_WHITE, bg);
}

// Touch mapping — same axis logic as the working RPS game:
//   lcd_touch_x = HORIZONTAL (left=small, right=large), threshold 2048 at screen center
//   lcd_touch_y = VERTICAL   (top=large, bottom=small), threshold 2048 at screen center
//   RED    top-left:     x < 2048, y > 2048
//   GREEN  top-right:    x > 2048, y > 2048
//   YELLOW bottom-left:  x < 2048, y < 2048
//   BLUE   bottom-right: x > 2048, y < 2048
static void draw_input_board(void) {
    fill_rect(0,      0,      HALF_W, HALF_H, dim[RED]);
    fill_rect(HALF_W, 0,      HALF_W, HALF_H, dim[GREEN]);
    fill_rect(0,      HALF_H, HALF_W, HALF_H, dim[YELLOW]);
    fill_rect(HALF_W, HALF_H, HALF_W, HALF_H, dim[BLUE]);
    lcd_set_font(Dialog_bold_16);
    lcd_put_string(65,  52,  "RED",    RGB_WHITE, dim[RED]);
    lcd_put_string(210, 52,  "GREEN",  RGB_WHITE, dim[GREEN]);
    lcd_put_string(35,  172, "YELLOW", RGB_WHITE, dim[YELLOW]);
    lcd_put_string(220, 172, "BLUE",   RGB_WHITE, dim[BLUE]);
}

// RED=top-left, GREEN=top-right, YELLOW=bottom-left, BLUE=bottom-right
static void flash_quadrant(color_e c) {
    uint16_t qx = (c == GREEN || c == BLUE)  ? HALF_W : 0;
    uint16_t qy = (c == YELLOW || c == BLUE) ? HALF_H : 0;
    fill_rect(qx, qy, HALF_W, HALF_H, bright[c]);
    uint32_t t = ms;
    while (ms - t < 250);
    fill_rect(qx, qy, HALF_W, HALF_H, dim[c]);
}

// Random permutation of all 4 colors so each appears exactly once
void generate_pattern(void) {
    color_e pool[4] = { RED, GREEN, YELLOW, BLUE };
    srand(ms);
    for (int k = 3; k > 0; k--) {
        int j = rand() % (k + 1);
        color_e tmp = pool[k]; pool[k] = pool[j]; pool[j] = tmp;
    }
    for (int k = 0; k < PATTERN_SIZE; k++) gen_pattern[k] = pool[k];
}

void input_detect(void) {
    if (state != GAME_INPUT || input_idx >= PATTERN_SIZE) return;

    color_e touched;
    bool    valid = true;

    if      (lcd_touch_x < MIDDLE_X && lcd_touch_y < MIDDLE_Y) touched = RED;
    else if (lcd_touch_x > MIDDLE_X && lcd_touch_y < MIDDLE_Y) touched = GREEN;
    else if (lcd_touch_x < MIDDLE_X && lcd_touch_y > MIDDLE_Y) touched = YELLOW;
    else if (lcd_touch_x > MIDDLE_X && lcd_touch_y > MIDDLE_Y) touched = BLUE;
    else valid = false;

    if (valid) {
        user_input[input_idx++] = touched;
        flash_quadrant(touched);
    }
}

void game_tick(void) {
    uint32_t now = ms;

    switch (state) {
        case GAME_SHOW:
            if (timer == 0) {
                show_color_screen(show_idx);
                timer = now ? now : 1;
            } else if (now - timer >= SHOW_MS) {
                show_idx++;
                if (show_idx >= PATTERN_SIZE) {
                    state     = GAME_INPUT;
                    input_idx = 0;
                    timer     = 0;
                    draw_input_board();
                } else {
                    show_color_screen(show_idx);
                    timer = now ? now : 1;
                }
            }
            break;

        case GAME_INPUT:
            if (input_idx >= PATTERN_SIZE) {
                bool correct = true;
                for (int k = 0; k < PATTERN_SIZE; k++) {
                    if (user_input[k] != gen_pattern[k]) { correct = false; break; }
                }
                state = correct ? GAME_WIN : GAME_LOSE;
                timer = now;
                if (correct) g_simon_solved = true;  /* KidBytes: report solved to FSM */
                lcd_clear(RGB_BLACK);
                lcd_orientation(ORIENTATION_90);
                lcd_set_font(Dialog_bold_16);
                if (correct)
                    lcd_put_string(95, 111, "Correct!", RGB_GREEN, RGB_BLACK);
                else
                    lcd_put_string(110, 111, "Wrong!", RGB_RED, RGB_BLACK);
            }
            break;

        case GAME_WIN:
        case GAME_LOSE:
            if (now - timer >= 2000) {
                generate_pattern();
                show_idx  = 0;
                timer     = 0;
                state     = GAME_SHOW;
                input_idx = 0;
            }
            break;
    }
}

void start_screen(void) {
    lcd_set_font(Dialog_bold_16);
    uint8_t fh = Dialog_bold_16[1];
    lcd_put_string(85, 0, "Simon Says", RGB_WHITE, RGB_BLUE);
    lcd_set_font(Dialog_plain_12);
    fh = Dialog_plain_12[1];
    lcd_put_string(0, 2 * fh, "Watch the 4 colors (1-2-3-4),", RGB_WHITE, RGB_BLACK);
    lcd_put_string(0, 3 * fh, "then tap them in that order!", RGB_WHITE, RGB_BLACK);
}
