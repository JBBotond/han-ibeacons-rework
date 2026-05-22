/*! ***************************************************************************
 *
 * \brief     Unity configuration
 * \file      unity_config.h
 * \author    Hugo Arends - HAN Embedded Vision and Machine Learning
 * \author
 * \date      January 2026
 *
 * \note      This file is used to configure Unity settings in one place.
 *            Refer to Unity/docs/UnityConfigurationGuide.md for more settings.
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

#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

#include <stdint.h>

extern volatile uint32_t ms;
extern uint32_t get_ms(void);

#define UNITY_OUTPUT_COLOR
// #define UNITY_PRINT_EOL() ({ UNITY_OUTPUT_CHAR('\r'); UNITY_OUTPUT_CHAR('\n'); })

#define UNITY_INCLUDE_EXEC_TIME
#define UNITY_CLOCK_MS get_ms

#endif // UNITY_CONFIG_H
