#include <board.h>
#include <stdio.h>

#include "serial/serial.h"
#include "functions/functions.h"
#include "serial2lpuart/lpuart2.h"
#include "solenoid/solenoid.h"

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

//  buffer used for holding at commands to be sent
char *atCommand = "AT+DISI?\r\n";
// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    serial_init(115200);
    lpuart2_init(9600);

    printf("Ibeacon project\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    box_init();
    led_init();
    e_init_done();
    BOX_CURRENT_STATE = BOX_SELECT_MODE;

    atSendCommand(atCommand);

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