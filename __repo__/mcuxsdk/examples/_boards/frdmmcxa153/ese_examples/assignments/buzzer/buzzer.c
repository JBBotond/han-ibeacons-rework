/*! ***************************************************************************
 *
 * \brief     Low level driver for a buzzer using the Standard Counter or Timer
 *            (CTIMER)
 * \file      buzzer.c
 * \author    Hugo Arends
 * \date      January 2026
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
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
 ******************************************************************************/
#include "buzzer.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
extern volatile uint32_t ms;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

/*!
 * \brief Initializes the buzzer driver
 *
 * Resources:
 * - P3_30 | CT0_MAT2
 *
 * Configures CTIMER0 to generate a 50% PWM signal on MAT2 with a frequency
 * range of 0 Hz to 20 kHz.
 */
void buzzer_init(void)
{
    // TBD
}

/*!
 * \brief Sets the buzzer frequency
 *
 * \param frequency_hz Frequency in Hz. Set to 0 to turn off the buzzer.
 *                     Maximum frequency is 20 kHz.
 */
void buzzer_set(uint32_t frequency_hz)
{
    // TBD
}

/*!
 * \brief Plays a melody in a blocking way. This means that the function
 *        does not return until the melody has finished playing.
 *
 * \param melody Pointer to the melody to play
 */
void buzzer_playmelody_blocking(const melody_t *melody)
{
    for(uint32_t i = 0; i < melody->length; i++)
    {
        int note = melody->notes[i];
        int duration = melody->durations[i];

        buzzer_set(note);

        // Calculate duration in milliseconds
        uint32_t note_duration_ms = 1000UL / duration;

        // Wait for the duration of the note
        uint32_t current_ms = ms;
        while(ms < current_ms + note_duration_ms)
        {}

        // Brief pause between notes (25% of the note duration)
        buzzer_set(0);
        current_ms = ms;
        while(ms < current_ms + (note_duration_ms / 4))
        {}
    }
}
