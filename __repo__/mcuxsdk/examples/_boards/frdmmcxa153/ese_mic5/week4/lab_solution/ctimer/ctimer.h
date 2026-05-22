/*! ***************************************************************************
 *
 * \brief     Low level driver for the Standard Counter or Timer (CTIMER)
 * \file      ctimer.h
 * \author    Hugo Arends
 * \date      January 2025
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
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
 ******************************************************************************/
#ifndef CTIMER_H
#define CTIMER_H

#include <MCXA153.h>
#include <stdbool.h>
#include "datetime.h"

#include "FreeRTOS.h"
#include "semphr.h"

// -----------------------------------------------------------------------------
// Shared type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Shared variables
// -----------------------------------------------------------------------------
extern SemaphoreHandle_t xCtimerOneSecondSemaphore;
extern SemaphoreHandle_t xCtimerAlarmSemaphore;

// -----------------------------------------------------------------------------
// Shared function prototypes
// -----------------------------------------------------------------------------
void ctimer_init(void);
void ctimer_get(datetime_t *datetime);
void ctimer_set(const datetime_t *datetime);
void ctimer_setalarm(const uint32_t offset_in_seconds);

#endif // CTIMER_H
