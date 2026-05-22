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
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>

#include "buzzer.h"
#include "ff.h"
#include "oled.h"
#include "serial.h"

#include "unity.h"

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
void test_systick(void);
void test_oled(void);
void test_sdcard_write(void);
void test_sdcard_read(void);
void test_buzzer(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
volatile uint32_t ms = 0;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
// This function is called before each test
void setUp(void)
{
}

// This function is called after each test
void tearDown(void)
{
}

uint32_t get_ms(void)
{
    return ms;
}

int main(void)
{
    serial_init(115200);
    SysTick_Config(48000);

    printf("oled_sdcard_buzzer unit test project\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);
    printf("\r\n");

    UNITY_BEGIN();
    RUN_TEST(test_systick);
    RUN_TEST(test_sdcard_write);
    RUN_TEST(test_sdcard_read);
    RUN_TEST(test_oled);
    RUN_TEST(test_buzzer);
    UNITY_END();

    oled_clearscreen();

    while(1)
    {
        // Wait for interrupt
        __WFI();

        uint32_t current_ms = ms;
        static uint32_t previous_ms = 0;

        // Interval milliseconds passed?
        if((current_ms - previous_ms) >= ANIMATION_DELAY_MS)
        {
            previous_ms = current_ms;

            static int32_t frame_cnt = 0;
            oled_drawanimation(32, 0, animation_rocket[frame_cnt],
                ANIMATION_WIDTH, ANIMATION_HEIGHT);
            oled_update();

            // Next frame
            frame_cnt++;
            if(frame_cnt >= ANIMATION_FRAME_COUNT)
            {
                frame_cnt = 0;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;
}

// -----------------------------------------------------------------------------
void test_systick(void)
{
    uint32_t current_ms = ms;

    // Blocking wait
    while ((ms - current_ms) < 1000)
    {}

    TEST_ASSERT_TRUE((ms - current_ms) >= 1000);
}

// -----------------------------------------------------------------------------

// Data used by both SDCARD write and read tests
char *filename = "unit_test.txt";
const char *text =
    "This is a test file created by the\r\n"    \
    "oled_sdcard_buzzer unit test project.\r\n" \
    "\r\n"                                      \
    "made by ESE\r\n";

void test_sdcard_write(void)
{
    FRESULT fr;

    // FatFs work area needed for each volume
    FATFS FatFs;

    // File object needed for each open file
    FIL Fil;

    // Give a work area to the default drive
    f_mount(&FatFs, "", 0);

    // Open file
    fr = f_open(&Fil, filename, FA_WRITE | FA_CREATE_ALWAYS);

    // Verify
    TEST_ASSERT_EQUAL(FR_OK, fr);

    if(fr == FR_OK)
    {
        unsigned int bytes_written;

        // Write data to the file
        fr = f_write(&Fil, text, strlen(text), &bytes_written);

        // Verify
        TEST_ASSERT_EQUAL(FR_OK, fr);
        TEST_ASSERT_EQUAL(strlen(text), bytes_written);

        // Close the file
        f_close(&Fil);
    }

    f_unmount("");
}

// -----------------------------------------------------------------------------
void test_sdcard_read(void)
{
    FRESULT fr;

    // FatFs work area needed for each volume
    FATFS FatFs;

    // File object needed for each open file
    FIL Fil;

    // Give a work area to the default drive
    f_mount(&FatFs, "", 0);

    // Open
    fr = f_open(&Fil, filename, FA_READ);

    // Verify
    TEST_ASSERT_EQUAL(FR_OK, fr);

    if(fr == FR_OK)
    {
        char line[100] = {0};
        unsigned int bytes_written;

        // Read data from the file
        fr = f_read(&Fil, line, sizeof(line), &bytes_written);

        // Verify
        TEST_ASSERT_EQUAL(FR_OK, fr);
        TEST_ASSERT_EQUAL_STRING(text, line);
        TEST_ASSERT_EQUAL(strlen(text), bytes_written);

        // Close the file
        f_close(&Fil);
    }

    f_unmount("");
}

// -----------------------------------------------------------------------------
void test_oled(void)
{
    // Initialize the OLED display
    oled_init();

    // Verify that the OLED is connected by checking the NDF flag
    // The led driver is interrupt driven, so the oled_init(), function will
    // return before the first I2C transfer is completed.
    // Therefore, we wait some time here to allow the transfer to complete.
    uint32_t start_ms = ms;
    while ((ms - start_ms) < 100)
    {}

    TEST_ASSERT_MESSAGE(((LPI2C0->MSR & LPI2C_MSR_NDF_MASK) == 0),
        "OLED not connected");

    // Display NXP logo and text
    oled_setorientation(1);
    oled_drawbitmap(nxp_logo);
    oled_setfont(Monospaced_plain_10);
    oled_putstring(53, 42, "MCX");
    oled_putstring(38, 53, "A Series");
    oled_update();
}

// -----------------------------------------------------------------------------
void test_buzzer(void)
{
    // Initialize the buzzer
    buzzer_init();

    // Play a melody
    buzzer_playmelody_blocking(&beepbeep_melody);

    TEST_IGNORE_MESSAGE("Verify buzzer playing beep-beep (audible check)");
}
