/**
  ******************************************************************************
  * File    benchnmark.c
  * Author  H. Arends
  * Brief   Functions for benchmarking
  *
  * Copyright (C) 2020 HAN University of Applied Sciences. All Rights Reserved.
  *
  * Permission is hereby granted, free of charge, to any person obtaining a
  * copy of this software and associated documentation files (the "Software"),
  * to deal in the Software without restriction, including without limitation
  * the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the
  * Software is furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  * IN THE SOFTWARE.
  ******************************************************************************
  */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "benchmark.h"
#include <string.h>

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function implementation
// -----------------------------------------------------------------------------

// Init SysTick timer:
// - 1 MHz
// - Maximum reload value
// - Disable SysTick exception
// - Enable the timer
void benchmark_init(void)
{
    // SysTick timer runs at 1 MHz
    MRCC0->MRCC_SYSTICK_CLKSEL = MRCC_MRCC_SYSTICK_CLKSEL_MUX(1);
    MRCC0->MRCC_SYSTICK_CLKDIV = MRCC_MRCC_SYSTICK_CLKDIV_DIV(0);

    SysTick->LOAD = 0x00FFFFFF;
    SysTick->VAL = 0;
    SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk);
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void benchmark_start(benchmark_t *b)
{
    b->start = SysTick->VAL;
}

void benchmark_stop(benchmark_t *b)
{
    b->stop = SysTick->VAL;
}

float benchmark_get_us(benchmark_t *b)
{
    if(b->start < b->stop)
    {
        b->start = b->start + SysTick->LOAD;
    }

    // Calculate microseconds
    // The SystickTimer runs at 1 MHz
    return (float)(b->start - b->stop);
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
