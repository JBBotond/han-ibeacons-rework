#include "board.h"
#include "functions.h"
#include <stdio.h>
#include <stdbool.h>
#include "solenoid/solenoid.h"

void init_button(void);
bool buttonA_pressed(void);
bool buttonB_pressed(void);

volatile int buttonA_count = 0;
volatile int buttonB_count = 0;

//debug functions
void led_init(void) {
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);

    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);

    PORT3->PCR[12] = PORT_PCR_LK(1);
    PORT3->PCR[12] = PORT_PCR_LK(1);
    
    GPIO3->PSOR |= (1<<12);
    GPIO3->PSOR |= (1<<13);

    GPIO3->PDDR |= (1<<12);
    GPIO3->PDDR |= (1<<13);

}
void led_green_on(void) {
    GPIO3->PCOR = (1<<13);
}
void led_red_on(void)
{
    GPIO3->PCOR = (1<<12);
}
void led_green_off(void) {
    GPIO3->PSOR = (1<<13);
}
void led_red_off(void)
{
    GPIO3->PSOR = (1<<12);
}

void box_init(void) {
    //initialize gpio
    init_button();
    solenoid_init();
    solenoid_lock(1);

    printf("Solenoid init & locked + button initialized! \r\n");
}

//state functions
void box_select_mode(int *mode) {
    if(buttonA_pressed()) {
        *mode = GAME;
        printf("Game mode selected! \r\n");
    }
    else if(buttonB_pressed()) {
        *mode = ADMIN;
        printf("Admin mode selected! \r\n");
    }

}
void box_shutdown(void) {}
void box_game_mode_init(void) {
    // initialize everything needed for game to run correctly

    //start the main game loop here

    printf("Game mode is initialized \r\n");


}
void box_admin_mode_init(void) {}
void box_debug_mode(void) {}

//  void for now, BOOL LATER!!
bool box_game_scan(void) {
    printf("Scanning for ibeacons...");
    int beacon_cnt = 0;
    //  scanning code by Danyil
    //  UUID-s in global buffer

    // ibeacon scan returns number of ibeacons
    
    if(beacon_cnt > 0)
        return true;
    return false;
}
void box_display_distance(void) {
    //  display game element goes here

    printf("Display:      __...... 0 m \r\n");
}
void box_game_hints(void) {
    printf("Found!. Locating next ibeacon... \r\n");


}
void box_game_final(void) {
    printf("All ibeacons found. Unlocking box! /r/n");

    solenoid_lock(0);
    printf("Solenoid powered /r/n");
}
void box_admin_settings(void) {}
void box_admin_connect(void) {}

//event functions
void e_init_done(void) {

    printf("Initial state exited \r\n");
}
void e_enter_game(void) {
    //  prepare everything needed for game mode here

    printf("Entering game mode \r\n");
}
void e_enter_admin(void) {
    //  prepare admin mode

    printf("Entering admin mode \r\n");
}
void e_start_scan(void) {
    //  prepare system for BLE scan and data storage

    printf("Preparing for BLE scan \r\n");
}
void e_admin_local(void) {}
void e_admin_pc(void) {}
void e_next_ibeacon(void) {
    //  set flag for ibeacon being found and select next ibeacon

    printf("Next ibeacon selected by game \r\n");
}
bool e_ibeacon_found(int distance, const int treshold) {
    //  returns whether the player has gotten close enough to the ibeacon
    if(distance <= treshold) {
        printf("Player close enough, ibeacon collected \r\n");
        return true;
    }
    else {
        printf("Not close enough to any ibeacons! \r\n");
        return false;
    }
}
bool e_all_ibeacons_found(void) {
    //  return true if all ibeacons have been found
    if(1 == 1)
        return true;
    else return false;

}
void e_draw_distance(void) {
    //  convert raw ibeacon data to actual distances here
    //  prepare data for display

    //  distance can be parsed as pointer from main !
    
    printf("Distances calculated! Note: only calculate if necessary \r\n");
    printf("Nearest ibeacon: 0 m \r\n");
}
void e_shutdown(void) {}
void e_game_over(void) {
    //  reset everything, prepare to return to init_state
    led_red_off();
    led_green_off();
    /// relay_set(1);

    BOX_MODE = NONE;

    printf("Session over. Reseting to select mode screen... \r\n");
}

//  init functions

void init_button(void) {
    //initialize built-in switch for debugging purposes

    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO1(1);
    MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_PORT1(1);

    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO1(1);
    MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_PORT1(1);

    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_PORT3(1);

    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_PORT3(1);

    PORT1->PCR[7] = PORT_PCR_MUX(0) | PORT_PCR_IBE(1);
    GPIO1->ICR[7] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1010);

    PORT3->PCR[29] = PORT_PCR_MUX(0) | PORT_PCR_IBE(1);
    GPIO3->ICR[29] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1010);

    NVIC_SetPriority(GPIO1_IRQn, 3);
    NVIC_ClearPendingIRQ(GPIO1_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);

    NVIC_SetPriority(GPIO3_IRQn, 3);
    NVIC_ClearPendingIRQ(GPIO3_IRQn);
    NVIC_EnableIRQ(GPIO3_IRQn);

    __enable_irq();
}

bool buttonA_pressed(void) {
    if(buttonA_count > 0) {

        buttonA_count --;
        
        return true;
    }
    
    return false;
}

bool buttonB_pressed(void) {
    if(buttonB_count > 0) {

        buttonB_count --;
        
        return true;
    }
    
    return false;
}

void GPIO1_IRQHandler(void) {
    NVIC_ClearPendingIRQ(GPIO1_IRQn);

    if((GPIO1->ISFR[0] & GPIO_ISFR_ISF7(1)) != 0) {

        GPIO1->ISFR[0] = GPIO_ISFR_ISF7(1);

        buttonA_count++;
        printf("Button A is pressed \r\n");
    }
}
void GPIO2_IRQHandler(void) {}
void GPIO3_IRQHandler(void) {
    NVIC_ClearPendingIRQ(GPIO3_IRQn);

    if((GPIO3->ISFR[0] & GPIO_ISFR_ISF29(1)) != 0) {

        GPIO3->ISFR[0] = GPIO_ISFR_ISF29(1);

        buttonB_count++;
        printf("Button B is pressed \r\n");
    }
}

//  ibeacon functions 
void atSendCommand(char *atCommand) {
    while(*atCommand) {
        lpuart2_putchar(*atCommand++);
    }
}