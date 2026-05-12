#include <stdbool.h>
#include "lpuart2.h"

//debug functions
void led_init(void);
void led_green_on(void);
void led_red_on(void);
void led_green_off(void);
void led_red_off(void);

enum box_state {
    BOX_INIT_STATE,
    BOX_SELECT_MODE,
    BOX_SHUTDOWN,
    BOX_GAME_MODE_INIT,
    BOX_ADMIN_MODE_INIT,
    BOX_DEBUG_MODE,
    BOX_GAME_SCAN,
    BOX_DISPLAY_DISTANCE,
    BOX_GAME_HINTS,
    BOX_GAME_FINAL,
    BOX_ADMIN_SETTINGS,
    BOX_ADMIN_CONNECT
};

enum box_event {
    E_INIT_DONE,
    E_SHUTDOWN,
    E_ENTER_GAME,
    E_ENTER_ADMIN,
    E_START_SCAN,
    E_ADMIN_LOCAL,
    E_ADMIN_PC,
    E_NEXT_IBEACON,
    E_IBEACON_FOUND,
    E_ALL_IBEACONS_FOUND,
    E_DRAW_DISTANCE
};

enum box_mode {
    NONE,
    GAME,
    ADMIN
};

//state functions
void box_init(void);
void box_select_mode(int *mode);
void box_shutdown(void);
void box_game_mode_init(void);
void box_admin_mode_init(void);
void box_debug_mode(void);
bool box_game_scan(void);
void box_display_distance(void);
void box_game_hints(void);
void box_game_final(void);
void box_admin_settings(void);
void box_admin_connect(void);

//event functions
void e_init_done(void);
void e_shutdown(void);
void e_enter_game(void);
void e_enter_admin(void);
void e_start_scan(void);
void e_admin_local(void);
void e_admin_pc(void);
void e_next_ibeacon(void);
bool e_ibeacon_found(int distance, const int treshold);
bool e_all_ibeacons_found(void);
void e_draw_distance(void);

void e_game_over(void);

//  ibeacon functions
void atSendCommand(char *atCommand);

extern int BOX_CURRENT_STATE;
extern int BOX_MODE;