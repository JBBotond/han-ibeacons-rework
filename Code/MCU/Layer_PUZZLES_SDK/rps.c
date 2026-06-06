#include "rps.h"
#include <stdlib.h>

extern volatile uint32_t ms;

#define ZONE_L  1365   /* Rock:     touch_x < 1365           */
#define ZONE_R  2730   /* Scissors: touch_x >= 2730, else Paper */

typedef enum { ROCK = 0, PAPER = 1, SCISSORS = 2 } choice_t;
typedef enum { RPS_CHOOSE, RPS_RESULT, RPS_DONE }  rps_state_t;

static rps_state_t state        = RPS_CHOOSE;
static uint32_t    result_timer = 0;
static choice_t    player_choice;
static choice_t    cpu_choice;
static bool        round_won;
static bool        round_draw;

static const char *choice_name[] = { "Rock", "Paper", "Scissors" };

static bool beats(choice_t a, choice_t b)
{
    return (a == ROCK && b == SCISSORS) || (a == PAPER && b == ROCK) || (a == SCISSORS && b == PAPER);
}

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

/* -----------------------------------------------------------------------
 * Choose screen
 *   y  0-23 : title banner  "~ PICK YOUR MOVE! ~"
 *   y 24-31 : bright colour strips (one per column)
 *   y 32-209: column bodies  (Rock/Paper/Scissors)
 *   y210-239: dark footer with per-column TAP! hints
 * --------------------------------------------------------------------- */
static void draw_choose_screen(void)
{
    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);

    /* Title banner */
    fill_rect(0, 0, 320, 24, RGB(3, 3, 18));
    lcd_set_font(Dialog_bold_16);
    lcd_put_string(40, 4, "~ PICK YOUR MOVE! ~", RGB_YELLOW, RGB(3, 3, 18));

    /* Bright colour accent strips */
    fill_rect(0,   24, 107, 8, RGB_BLUE);
    fill_rect(107, 24, 107, 8, RGB_GREEN);
    fill_rect(214, 24, 106, 8, RGB_ORANGE);

    /* Column bodies */
    uint16_t c_rock  = RGB(2,  8, 28);
    uint16_t c_paper = RGB(2, 32,  4);
    uint16_t c_scis  = RGB(28, 10,  2);
    fill_rect(0,   32, 107, 178, c_rock);
    fill_rect(107, 32, 107, 178, c_paper);
    fill_rect(214, 32, 106, 178, c_scis);

    /* Big letter — draw shadow then bright white on top */
    lcd_set_font(Monospaced_bold_32);
    lcd_put_string(45, 90, "R", RGB(1, 4, 14), c_rock);   /* shadow */
    lcd_put_string(43, 88, "R", RGB_WHITE,     c_rock);   /* main   */
    lcd_put_string(152, 90, "P", RGB(1, 16, 2), c_paper);
    lcd_put_string(150, 88, "P", RGB_WHITE,     c_paper);
    lcd_put_string(259, 90, "S", RGB(14, 5, 1), c_scis);
    lcd_put_string(257, 88, "S", RGB_WHITE,     c_scis);

    /* Centred labels */
    lcd_set_font(Dialog_plain_12);
    lcd_put_string(35,  140, "ROCK",     RGB_WHITE, c_rock);
    lcd_put_string(138, 140, "PAPER",    RGB_WHITE, c_paper);
    lcd_put_string(231, 140, "SCISSORS", RGB_WHITE, c_scis);

    /* Footer with per-column tap hints */
    fill_rect(0, 210, 320, 30, RGB(3, 3, 15));
    lcd_put_string(30,  218, "TAP!", RGB(10, 30, 31), RGB(3, 3, 15));
    lcd_put_string(137, 218, "TAP!", RGB(10, 31, 10), RGB(3, 3, 15));
    lcd_put_string(233, 218, "TAP!", RGB(31, 20, 10), RGB(3, 3, 15));
}

/* -----------------------------------------------------------------------
 * Result screen — three distinct kid-friendly looks
 * --------------------------------------------------------------------- */
static void draw_result_screen(void)
{
    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);

    if (round_draw) {
        /* ---- DRAW : cyan/teal theme ---- */
        uint16_t bg = RGB(3, 12, 18);
        fill_rect(0, 0, 320, 240, bg);
        fill_rect(0,   0, 320, 8, RGB_CYAN);
        fill_rect(0, 232, 320, 8, RGB_CYAN);

        lcd_set_font(Dialog_bold_16);
        lcd_put_string(87, 52, "ITS A TIE!", RGB(0, 6, 10), bg);  /* shadow */
        lcd_put_string(85, 50, "ITS A TIE!", RGB_CYAN,      bg);

        lcd_set_font(Dialog_plain_12);
        lcd_put_string(70, 88,  "Nobody wins this round!", RGB_WHITE, bg);
        lcd_put_string(100, 106, "Try Again!",             RGB_CYAN,  bg);

    } else if (round_won) {
        /* ---- WIN : bright green/gold celebration ---- */
        uint16_t bg = RGB(3, 18, 3);
        fill_rect(0, 0, 320, 240, bg);

        /* Rainbow stripes top and bottom */
        fill_rect(0,   0,   107, 10, RGB_RED);
        fill_rect(107, 0,   107, 10, RGB_YELLOW);
        fill_rect(214, 0,   106, 10, RGB_GREEN);
        fill_rect(0,   230, 107, 10, RGB_GREEN);
        fill_rect(107, 230, 107, 10, RGB_YELLOW);
        fill_rect(214, 230, 106, 10, RGB_RED);

        /* Star decorations */
        lcd_set_font(Dialog_bold_16);
        lcd_put_string(12,  35, "*", RGB_YELLOW, bg);
        lcd_put_string(296, 35, "*", RGB_YELLOW, bg);
        lcd_put_string(12, 188, "*", RGB_YELLOW, bg);
        lcd_put_string(296,188, "*", RGB_YELLOW, bg);

        /* Big win text with shadow */
        lcd_set_font(Monospaced_bold_32);
        lcd_put_string(74, 57, "YOU WIN!", RGB(1, 9, 1), bg);  /* shadow */
        lcd_put_string(72, 55, "YOU WIN!", RGB_YELLOW,   bg);

        lcd_set_font(Dialog_plain_12);
        lcd_put_string(65, 102, "Whack-A-Mole is next!", RGB_WHITE, bg);

    } else {
        /* ---- LOSE : red/orange theme ---- */
        uint16_t bg = RGB(22, 4, 2);
        fill_rect(0, 0, 320, 240, bg);
        fill_rect(0,   0, 320, 10, RGB_RED);
        fill_rect(0, 230, 320, 10, RGB_RED);

        lcd_set_font(Dialog_bold_16);
        lcd_put_string(107, 42, "OH NO!",   RGB(10, 0, 0), bg);  /* shadow */
        lcd_put_string(105, 40, "OH NO!",   RGB_YELLOW,    bg);
        lcd_put_string(92,  70, "YOU LOSE", RGB_WHITE,     bg);

        lcd_set_font(Dialog_plain_12);
        lcd_put_string(100, 106, "Try Again!",  RGB_ORANGE, bg);
    }

    /* ---- Shared: show what each side picked ---- */
    uint16_t bg = round_won ? RGB(3,18,3) : (round_draw ? RGB(3,12,18) : RGB(22,4,2));
    lcd_set_font(Dialog_plain_12);
    lcd_put_string(15,  162, "You: ", RGB_WHITE, bg);
    lcd_put_string(62,  162, choice_name[player_choice], RGB_WHITE, bg);
    lcd_put_string(175, 162, "CPU: ", RGB_WHITE, bg);
    lcd_put_string(220, 162, choice_name[cpu_choice],    RGB_WHITE, bg);

    if (round_won)
        lcd_put_string(35, 210, "Touch to start Whack-A-Mole!", RGB_GRAY, bg);
    else
        lcd_put_string(70, 210, "Touch to play again",         RGB_GRAY, bg);
}

/* -----------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

void rps_init(void)
{
    state = RPS_CHOOSE;
    draw_choose_screen();
}

void rps_input_detect(void)
{
    if (state == RPS_RESULT) {
        if (round_won) { state = RPS_DONE; }
        else           { state = RPS_CHOOSE; draw_choose_screen(); }
        return;
    }
    if (state != RPS_CHOOSE) return;

    if      (lcd_touch_x < ZONE_L) player_choice = ROCK;
    else if (lcd_touch_x < ZONE_R) player_choice = PAPER;
    else                            player_choice = SCISSORS;

    srand(ms);
    cpu_choice = (choice_t)(rand() % 3);

    round_draw = (player_choice == cpu_choice);
    round_won  = !round_draw && beats(player_choice, cpu_choice);

    state        = RPS_RESULT;
    result_timer = ms;
    draw_result_screen();
}

/* Returns true when the player won and the host should start Whack-a-Mole. */
bool rps_game_tick(void)
{
    if (state == RPS_RESULT && ms - result_timer >= 4000) {
        if (round_won) { state = RPS_DONE; }
        else           { state = RPS_CHOOSE; draw_choose_screen(); }
    }
    return (state == RPS_DONE);
}
