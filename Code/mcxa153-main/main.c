#include <board.h>
#include <stdio.h>

#include "serial.h"
#include "lpuart2.h"
#include "functions/functions.h"
#include "solenoid/solenoid.h"
#include "display/resources/fonts.h"
#include "display/resources/bitmaps.h"
#include "display/resources/animations.h"
#include "display/tft_lcd/lpspi_master.h"
#include "display/tft_lcd/tft_lcd.h"
#include "display/resources/screens.h"

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

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
//  use events to handle box current state
int BOX_CURRENT_STATE = BOX_INIT_STATE;
int BOX_MODE = NONE;

int distanceToBeacon = 0;
//  remove const for calibration
const int treshold = 10;

volatile uint32_t ms = 0;

//  enum type user for setting LCD orientation
static orientation_t orientation = ORIENTATION_270;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    serial_init(115200);
    lpuart2_init(9600);

    //  set up SysTick for interrupt every 1ms
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);
    SysTick_Config(96000);

    printf("Ibeacon project\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);
    
    lcd_init();
    lcd_orientation(orientation);
    lcd_clear(RGB_BLACK);

    //  peripherals initialize
    box_init();
    led_init();
    
    lcd_set_font(Dialog_bold_16);
    lcd_put_string(0, 0, "Ibeacons project", RGB_LIME, RGB_BLACK);

    e_init_done();
    BOX_CURRENT_STATE = BOX_SELECT_MODE;

    //  send command to scan for ibeacons
    //atSendCommand(atCommand);

    while(1)
    {
        //  --------------------------------
        //  main switch() statement for game
        switch (BOX_CURRENT_STATE)
        {
            case BOX_SELECT_MODE:
                if(BOX_MODE == NONE) {
                    box_select_mode(&BOX_MODE);
                    if(BOX_MODE == GAME) {
                        led_red_on();
                        e_enter_game();
                        BOX_CURRENT_STATE = BOX_GAME_MODE_INIT;
                    }
                    else if (BOX_MODE == ADMIN) {
                        led_green_on();

                        printf("Admin mode not implemented yet, freezing program! \r\n");
                        while(1) {}
                    }
                }

                break;
            
            case BOX_GAME_MODE_INIT:
                box_game_mode_init();
                e_start_scan();
                BOX_CURRENT_STATE = BOX_GAME_SCAN;

                break;

            case BOX_GAME_SCAN:
                //  box_game_scan return true if ibeacons found, false if no ibeacons found
                if(!box_game_scan()) 
                    printf("Warning! No ibeacons found. \r\n");
                e_draw_distance();
                BOX_CURRENT_STATE = BOX_DISPLAY_DISTANCE;
 
                break;
                
            case BOX_DISPLAY_DISTANCE:
                box_display_distance();
                
                if(e_ibeacon_found(distanceToBeacon, treshold))
                    BOX_CURRENT_STATE = BOX_GAME_HINTS;
                break;

            case BOX_GAME_HINTS:
                box_game_hints();
                
                if(e_all_ibeacons_found())
                    BOX_CURRENT_STATE = BOX_GAME_FINAL;
                else {
                    e_next_ibeacon();
                    BOX_CURRENT_STATE = BOX_GAME_SCAN;
                }
                
                break;
            
            case BOX_GAME_FINAL:
                //  unlock box and finalize game here
                box_game_final();

                e_game_over();

                //  wait before resetting and locking again?

                BOX_CURRENT_STATE = BOX_SELECT_MODE;

            default:
                break;
        }

        //  --------------------------------

        // Data available from serial?
            if(serial_rxcnt() > 0)
            {
                // Get the data from serial (LPUART0)
                uint8_t data = serial_getchar();

                // Forward the data to LPUART2
                lpuart2_putchar(data);
            }

            // Data available from module?
            if(lpuart2_rxcnt() > 0)
            {
                // Get the data from LPUART2
                uint8_t data = lpuart2_getchar();

                // Forward the data to serial (LPUART0)
                serial_putchar(data);
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

const orientation_t orientations[] =
    {
        ORIENTATION_0,
        ORIENTATION_90,
        ORIENTATION_180,
        ORIENTATION_270
    };