/*! ***************************************************************************
 *
 * \brief     Definitions of animations
 * \file      animations.h
 * \author    Hugo Arends
 * \date      February 2026
 *
 * \remark    Add a new animation as follows
 *
 * 1. Create an animation using the great online tool Wokwi animator
 *    https://animator.wokwi.com/
 * 2. Add all the generated code in the file animations.c
 * 3. Add an external declaration to the file animations.h
 * 4. Draw a single frame of the animation by passing the pointer to the
 *    function oled_drawanimation()
 *
 * Example:
 *
 *      while(1)
 *      {
 *          // Wait for interrupt
 *          __WFI();
 *
 *          uint32_t current_ms = ms;
 *
 *          // Interval milliseconds passed?
 *          if((current_ms - previous_ms) >= ANIMATION_DELAY_MS)
 *          {
 *              previous_ms = current_ms;
 *
 *              static int32_t frame_cnt = 0;
 *              oled_drawanimation(32, 0, animation_rocket[frame_cnt],
 *                  ANIMATION_WIDTH, ANIMATION_HEIGHT);
 *              oled_update();
 *
 *              // Next frame
 *              frame_cnt++;
 *              if(frame_cnt >= ANIMATION_FRAME_COUNT)
 *              {
 *                  frame_cnt = 0;
 *              }
 *          }
 *      }
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
#ifndef ANIMATIONS_H_
#define ANIMATIONS_H_

// This example supports fixed frame settings:
// - frames of 64x64 pixels
// - 28 frames
// - 42 ms delay between frames
#define ANIMATION_WIDTH       (64)
#define ANIMATION_HEIGHT      (64)
#define ANIMATION_FRAME_COUNT (28)
#define ANIMATION_DELAY_MS    (42)

extern const unsigned char animation_map_pointer[][512];
extern const unsigned char animation_rocket[][512];

#endif // ANIMATIONS_H_
