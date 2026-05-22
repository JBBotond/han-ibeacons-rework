/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      January 2026
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
 *
 * \copyright 2026 HAN University of Applied Sciences. All Rights Reserved.
 *            \r\n\r\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \r\n\r\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \r\n\r\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>

#include "leds.h"
#include "oled.h"
#include "serial.h"
#include "switches.h"

#include "mastermind.h"
#include "mastermind_solver.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#ifdef DEBUG
#define TARGETSTR "Debug"
#else
#define TARGETSTR "Release"
#endif

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
void redraw_main_screen(void);
char get_user_input(const char *prompt, const uint16_t x, const uint16_t y,
    char min, char max);

void game(void);
void solver(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
volatile uint32_t ms = 0;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    serial_init(115200);
    SysTick_Config(48000);
    leds_init();
    sw_init();

    oled_init();

    printf("Mastermind\r\r\n");
    printf("%s build %s %s\r\r\n", TARGETSTR, __DATE__, __TIME__);

    oled_setorientation(1);
    oled_setfont(Monospaced_bold_12);

    while(1)
    {
        redraw_main_screen();
        oled_putstring(0, 24, "SW2: game");
        oled_putstring(0, 39, "SW3: solver");
        oled_update();

        bool sw2_was_pressed = false;
        bool sw3_was_pressed = false;

        do
        {
            sw2_was_pressed = sw2_pressed();
            sw3_was_pressed = sw3_pressed();
        } while ((sw2_was_pressed == false) && (sw3_was_pressed == false));

        redraw_main_screen();
        oled_putstring(0, 24, "SW2: to set number");
        oled_putstring(0, 39, "SW3: to confirm");
        oled_update();

        uint32_t timeout_ms = ms + 2500;

        while(ms < timeout_ms)
        {}

        if(sw2_was_pressed)
        {
            game();
        }
        else if (sw3_was_pressed)
        {
            solver();
        }
    }
}

void redraw_main_screen(void)
{
    oled_clearscreen();
    oled_putstring(0, 0, "MASTERMIND");
    oled_drawline(0, 15, 127, 15);
}

void game(void)
{
    unsigned char code[4] = {0};
    unsigned char secret_code[4] = {0};
    int turn = 1;

    mm_result_t result;

    // Set secret code, based on current value of SysTick counter
    secret_code[0] = 1 + (((SysTick->VAL) >> 0) % 6);
    secret_code[1] = 1 + (((SysTick->VAL) >> 1) % 6);
    secret_code[2] = 1 + (((SysTick->VAL) >> 2) % 6);
    secret_code[3] = 1 + (((SysTick->VAL) >> 3) % 6);

    set_secret_code(secret_code);

    redraw_main_screen();
    oled_update();

    do
    {
        char input_str[32] = {0};
        strcpy(input_str, "Enter code: ");

        // Get code from user
        code[0] = get_user_input(input_str, 0, 16, 1, 6);
        input_str[12] = code[0] + '0';

        code[1] = get_user_input(input_str, 0, 16, 1, 6);
        input_str[13] = code[1] + '0';

        code[2] = get_user_input(input_str, 0, 16, 1, 6);
        input_str[14] = code[2] + '0';

        code[3] = get_user_input(input_str, 0, 16, 1, 6);
        input_str[15] = code[3] + '0';

        turn++;

        result = check_secret_code(code);

        oled_putstring(0, 16, "Enter code:     ");
        oled_putstring(0, 35, "Correct n+p: ");
        oled_putchar(result.correct_num_and_pos + '0');
        oled_putstring(0, 50, "Correct n  : ");
        oled_putchar(result.correct_num + '0');
        oled_update();

    } while(result.correct_num_and_pos != 4 && turn <= 10);

    redraw_main_screen();

    if(result.correct_num_and_pos == 4)
    {
        oled_putstring(0, 24, "Code cracked!");
    }
    else
    {
        oled_putstring(0, 24, "Code not cracked!");
        get_secret_code(code);
    }

    oled_putstring(0, 40, "Code: ");
    oled_putchar(code[0] + '0');
    oled_putchar(code[1] + '0');
    oled_putchar(code[2] + '0');
    oled_putchar(code[3] + '0');
    oled_update();

    uint32_t timeout_ms = ms + 5000;

    while(ms < timeout_ms)
    {}

    oled_drawbitmap(bonsai);
    oled_putstring(0, 24, " ");
    oled_putchar(code[0] + '0');
    oled_putchar(code[1] + '0');
    oled_putchar(code[2] + '0');
    oled_putchar(code[3] + '0');
    oled_update();
    oled_update();

    // Wait for switch press before returning to main menu
    while((sw2_pressed() == false) && (sw3_pressed() == false))
    {}
}

void solver(void)
{
    unsigned char n = 0, p = 0;
    unsigned char code[4] = {0};

    solver_reset();

    do
    {
        solver_next_code(code, p, n);

        do
        {
            redraw_main_screen();
            oled_putstring(0, 24, "Try code: ");
            oled_putchar(code[0] + '0');
            oled_putchar(code[1] + '0');
            oled_putchar(code[2] + '0');
            oled_putchar(code[3] + '0');
            oled_update();

            p = get_user_input("Correct n+p: ",0, 45, 0, 4);

            n = get_user_input("Correct n  : ", 0, 45, 0, 4);

            if((n + p) > 4)
            {
                redraw_main_screen();
                oled_putstring(0, 24, "Impossible, try");
                oled_putstring(0, 39, "again");
                oled_update();

                uint32_t timeout_ms = ms + 2000;
                while(ms < timeout_ms)
                {}
            }

        } while ((n + p) > 4);

    }while(p != 4);

    redraw_main_screen();
    oled_putstring(0, 30, "Code cracked: ");
    oled_putchar(code[0] + '0');
    oled_putchar(code[1] + '0');
    oled_putchar(code[2] + '0');
    oled_putchar(code[3] + '0');
    oled_update();

    uint32_t timeout_ms = ms + 5000;

    while(ms < timeout_ms)
    {}

    oled_drawbitmap(bonsai);
    oled_putstring(0, 24, " ");
    oled_putchar(code[0] + '0');
    oled_putchar(code[1] + '0');
    oled_putchar(code[2] + '0');
    oled_putchar(code[3] + '0');
    oled_update();
    oled_update();

    // Wait for switch press before returning to main menu
    while((sw2_pressed() == false) && (sw3_pressed() == false))
    {}
}

char get_user_input(const char *prompt, const uint16_t x, const uint16_t y, char min, char max)
{
    bool done = false;
    char input = min;

    oled_putstring(x, y, prompt);
    oled_putchar(input + '0');
    oled_drawline(7*strlen(prompt), y+14, 7*strlen(prompt) + 7, y+14);
    oled_update();

    do
    {
        if(sw2_pressed())
        {
            input++;

            input = (input > max) ? min : input;

            oled_putstring(x, y, prompt);
            oled_putchar(input + '0');
            oled_drawline(7*strlen(prompt), y+14, 7*strlen(prompt) + 7, y+14);
            oled_update();
        }

        if(sw3_pressed())
        {
            done = true;
        }

    } while(done == false);

    return input;
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;
}
