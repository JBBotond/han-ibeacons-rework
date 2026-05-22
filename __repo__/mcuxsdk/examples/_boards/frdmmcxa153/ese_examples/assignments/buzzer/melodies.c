/*! ***************************************************************************
 *
 * \brief     Melodies
 * \file      melodies.c
 * \author    Hugo Arends
 * \date      January 2026
 *
 * \see       https://github.com/hibit-dev/buzzer/
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
#include "melodies.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Beep beep melody
// -----------------------------------------------------------------------------
const int beepbeep_notes[] =
{
    NOTE_C6, REST, NOTE_C6,
};

const int beepbeep_durations[] =
{
    16, 16, 16,
};

const melody_t beepbeep_melody =
{
    .notes = beepbeep_notes,
    .durations = beepbeep_durations,
    .length = sizeof(beepbeep_notes) / sizeof(beepbeep_notes[0]),
};

// -----------------------------------------------------------------------------
// Nokia melody
// -----------------------------------------------------------------------------
const int nokia_notes[] =
{
    NOTE_E5, NOTE_D5, NOTE_FS4, NOTE_GS4,
    NOTE_CS5, NOTE_B4, NOTE_D4, NOTE_E4,
    NOTE_B4, NOTE_A4, NOTE_CS4, NOTE_E4,
    NOTE_A4
};

const int nokia_durations[] =
{
    8, 8, 4, 4,
    8, 8, 4, 4,
    8, 8, 4, 4,
    2
};

const melody_t nokia_melody =
{
    .notes = nokia_notes,
    .durations = nokia_durations,
    .length = sizeof(nokia_notes) / sizeof(nokia_notes[0]),
};

// -----------------------------------------------------------------------------
// Mario Bros. melody
// -----------------------------------------------------------------------------
const int mariobros_notes[] =
{
    NOTE_E5, NOTE_E5, REST, NOTE_E5, REST, NOTE_C5, NOTE_E5,
    NOTE_G5, REST, NOTE_G4, REST,
    NOTE_C5, NOTE_G4, REST, NOTE_E4,
    NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
    NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
    REST, NOTE_E5,NOTE_C5, NOTE_D5, NOTE_B4,
    NOTE_C5, NOTE_G4, REST, NOTE_E4,
    NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
    NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
    REST, NOTE_E5,NOTE_C5, NOTE_D5, NOTE_B4,

    REST, NOTE_G5, NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_E5,
    REST, NOTE_GS4, NOTE_A4, NOTE_C4, REST, NOTE_A4, NOTE_C5, NOTE_D5,
    REST, NOTE_DS5, REST, NOTE_D5,
    NOTE_C5, REST,

    REST, NOTE_G5, NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_E5,
    REST, NOTE_GS4, NOTE_A4, NOTE_C4, REST, NOTE_A4, NOTE_C5, NOTE_D5,
    REST, NOTE_DS5, REST, NOTE_D5,
    NOTE_C5, REST,

    NOTE_C5, NOTE_C5, NOTE_C5, REST, NOTE_C5, NOTE_D5,
    NOTE_E5, NOTE_C5, NOTE_A4, NOTE_G4,

    NOTE_C5, NOTE_C5, NOTE_C5, REST, NOTE_C5, NOTE_D5, NOTE_E5,
    REST,
    NOTE_C5, NOTE_C5, NOTE_C5, REST, NOTE_C5, NOTE_D5,
    NOTE_E5, NOTE_C5, NOTE_A4, NOTE_G4,
    NOTE_E5, NOTE_E5, REST, NOTE_E5, REST, NOTE_C5, NOTE_E5,
    NOTE_G5, REST, NOTE_G4, REST,
    NOTE_C5, NOTE_G4, REST, NOTE_E4,

    NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
    NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
    REST, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_B4,

    NOTE_C5, NOTE_G4, REST, NOTE_E4,
    NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
    NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
    REST, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_B4,

    NOTE_E5, NOTE_C5, NOTE_G4, REST, NOTE_GS4,
    NOTE_A4, NOTE_F5, NOTE_F5, NOTE_A4,
    NOTE_D5, NOTE_A5, NOTE_A5, NOTE_A5, NOTE_G5, NOTE_F5,

    NOTE_E5, NOTE_C5, NOTE_A4, NOTE_G4,
    NOTE_E5, NOTE_C5, NOTE_G4, REST, NOTE_GS4,
    NOTE_A4, NOTE_F5, NOTE_F5, NOTE_A4,
    NOTE_B4, NOTE_F5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_D5,
    NOTE_C5, NOTE_E4, NOTE_E4, NOTE_C4,

    NOTE_E5, NOTE_C5, NOTE_G4, REST, NOTE_GS4,
    NOTE_A4, NOTE_F5, NOTE_F5, NOTE_A4,
    NOTE_D5, NOTE_A5, NOTE_A5, NOTE_A5, NOTE_G5, NOTE_F5,

    NOTE_E5, NOTE_C5, NOTE_A4, NOTE_G4,
    NOTE_E5, NOTE_C5, NOTE_G4, REST, NOTE_GS4,
    NOTE_A4, NOTE_F5, NOTE_F5, NOTE_A4,
    NOTE_B4, NOTE_F5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_D5,
    NOTE_C5, NOTE_E4, NOTE_E4, NOTE_C4,
    NOTE_C5, NOTE_C5, NOTE_C5, REST, NOTE_C5, NOTE_D5, NOTE_E5,
    REST,

    NOTE_C5, NOTE_C5, NOTE_C5, REST, NOTE_C5, NOTE_D5,
    NOTE_E5, NOTE_C5, NOTE_A4, NOTE_G4,
    NOTE_E5, NOTE_E5, REST, NOTE_E5, REST, NOTE_C5, NOTE_E5,
    NOTE_G5, REST, NOTE_G4, REST,
    NOTE_E5, NOTE_C5, NOTE_G4, REST, NOTE_GS4,
    NOTE_A4, NOTE_F5, NOTE_F5, NOTE_A4,
    NOTE_D5, NOTE_A5, NOTE_A5, NOTE_A5, NOTE_G5, NOTE_F5,

    NOTE_E5, NOTE_C5, NOTE_A4, NOTE_G4,
    NOTE_E5, NOTE_C5, NOTE_G4, REST, NOTE_GS4,
    NOTE_A4, NOTE_F5, NOTE_F5, NOTE_A4,
    NOTE_B4, NOTE_F5, NOTE_F5, NOTE_F5, NOTE_E5, NOTE_D5,
    NOTE_C5, NOTE_E4, NOTE_E4, NOTE_C4,

    // Game over sound
    NOTE_C5, NOTE_G4, NOTE_E4,
    NOTE_A4, NOTE_B4, NOTE_A4, NOTE_GS4, NOTE_AS4, NOTE_GS4,
    NOTE_G4, NOTE_D4, NOTE_E4
};

const int mariobros_durations[] =
{
    8, 8, 8, 8, 8, 8, 8,
    4, 4, 8, 4,
    4, 8, 4, 4,
    4, 4, 8, 4,
    8, 8, 8, 4, 8, 8,
    8, 4,8, 8, 4,
    4, 8, 4, 4,
    4, 4, 8, 4,
    8, 8, 8, 4, 8, 8,
    8, 4,8, 8, 4,


    4, 8, 8, 8, 4, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    4, 4, 8, 4,
    2, 2,

    4, 8, 8, 8, 4, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    4, 4, 8, 4,
    2, 2,

    8, 4, 8, 8, 8, 4,
    8, 4, 8, 2,

    8, 4, 8, 8, 8, 8, 8,
    1,
    8, 4, 8, 8, 8, 4,
    8, 4, 8, 2,
    8, 8, 8, 8, 8, 8, 4,
    4, 4, 4, 4,
    4, 8, 4, 4,

    4, 4, 8, 4,
    8, 8, 8, 4, 8, 8,
    8, 4, 8, 8, 4,

    4, 8, 4, 4,
    4, 4, 8, 4,
    8, 8, 8, 4, 8, 8,
    8, 4, 8, 8, 4,

    8, 4, 8, 4, 4,
    8, 4, 8, 2,
    8, 8, 8, 8, 8, 8,

    8, 4, 8, 2,
    8, 4, 8, 4, 4,
    8, 4, 8, 2,
    8, 4, 8, 8, 8, 8,
    8, 4, 8, 2,

    8, 4, 8, 4, 4,
    8, 4, 8, 2,
    8, 8, 8, 8, 8, 8,

    8, 4, 8, 2,
    8, 4, 8, 4, 4,
    8, 4, 8, 2,
    8, 4, 8, 8, 8, 8,
    8, 4, 8, 2,
    8, 4, 8, 8, 8, 8, 8,
    1,

    8, 4, 8, 8, 8, 4,
    8, 4, 8, 2,
    8, 8, 8, 8, 8, 8, 4,
    4, 4, 4, 4,
    8, 4, 8, 4, 4,
    8, 4, 8, 2,
    8, 8, 8, 8, 8, 8,

    8, 4, 8, 2,
    8, 4, 8, 4, 4,
    8, 4, 8, 2,
    8, 4, 8, 8, 8, 8,
    8, 4, 8, 2,

    //game over sound
    4, 4, 4,
    8, 8, 8, 8, 8, 8,
    8, 8, 2
};

const melody_t mariobros_melody =
{
    .notes = mariobros_notes,
    .durations = mariobros_durations,
    .length = sizeof(mariobros_notes) / sizeof(mariobros_notes[0]),
};
