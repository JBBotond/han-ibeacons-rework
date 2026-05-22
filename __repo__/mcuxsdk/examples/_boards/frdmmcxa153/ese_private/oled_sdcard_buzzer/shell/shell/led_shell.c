/*! ***************************************************************************
 *
 * \brief     Shell module for LEDs
 * \file      led_shell.c
 * \author    Hugo Arends
 *            Mostly generated using GitHub Copilot
 * \date      February 2026
 *
 * \copyright 2026 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
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
#include <stdio.h>
#include <string.h>
#include "led_shell.h"
#include "leds.h"

void cmd_led_on(const char *args) {
    if (!args || strlen(args) == 0) {
        printf("Usage: on r|g|b or on all\n");
        printf("Available LEDs: r, g, b\n");
        return;
    }

    if (strcmp(args, "all") == 0) {
        led_green_on();
        led_blue_on();
        led_red_on();
        printf("Turned on all LEDs\n");
    }
    else if (strcmp(args, "r") == 0) {
        led_red_on();
        printf("Turned on red LED\n");
    }
    else if (strcmp(args, "g") == 0) {
        led_green_on();
        printf("Turned on green LED\n");
    }
    else if (strcmp(args, "b") == 0) {
        led_blue_on();
        printf("Turned on blue LED\n");
    } else {
        printf("Unknown LED: %s\n", args);
    }
}

void cmd_led_off(const char *args) {
    if (!args || strlen(args) == 0) {
        printf("Usage: off r|g|b or off all\n");
        printf("Available LEDs: r, g, b\n");
        return;
    }

    if (strcmp(args, "all") == 0) {
        led_green_off();
        led_blue_off();
        led_red_off();
        printf("Turned off all LEDs\n");
    }
    else if (strcmp(args, "r") == 0) {
        led_red_off();
        printf("Turned off red LED\n");
    }
    else if (strcmp(args, "g") == 0) {
        led_green_off();
        printf("Turned off green LED\n");
    }
    else if (strcmp(args, "b") == 0) {
        led_blue_off();
        printf("Turned off blue LED\n");
    } else {
        printf("Unknown LED: %s\n", args);
    }
}

void cmd_led_toggle(const char *args) {

    if (!args || strlen(args) == 0) {
        printf("Usage: toggle r|g|b or toggle all\n");
        printf("Available LEDs: r, g, b\n");
        return;
    }

    if (strcmp(args, "all") == 0) {
        led_green_toggle();
        led_blue_toggle();
        led_red_toggle();
        printf("Toggled all LEDs\n");
    }
    else if (strcmp(args, "r") == 0) {
        led_red_toggle();
        printf("Toggled red LED\n");
    }
    else if (strcmp(args, "g") == 0) {
        led_green_toggle();
        printf("Toggled green LED\n");
    }
    else if (strcmp(args, "b") == 0) {
        led_blue_toggle();
        printf("Toggled blue LED\n");
    } else {
        printf("Unknown LED: %s\n", args);
    }
}

void led_shell_init(shell_context_t *ctx) {
    // Register LED commands
    shell_command_t led_commands[] = {
        {"on", "Turn on LED (usage: on r|g|b|all)", cmd_led_on},
        {"off", "Turn off LED (usage: off r|g|b|all)", cmd_led_off},
        {"toggle", "Toggle LED (usage: toggle r|g|b|all)", cmd_led_toggle},
    };

    for (size_t i = 0; i < sizeof(led_commands) / sizeof(led_commands[0]); i++) {
        if (ctx->command_count < 32) {
            ctx->commands[ctx->command_count++] = led_commands[i];
        }
    }

    printf("LED shell initialized\n");
}