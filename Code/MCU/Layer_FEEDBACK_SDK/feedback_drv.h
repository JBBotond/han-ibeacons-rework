/*! ***************************************************************************
 * \brief  feedback_drv.h — Layer 4 Feedback Driver
 * \file   feedback_drv.h
 *
 * Drives LED cues and buzzer tones for puzzle/unlock events.
 *
 * Hardware:
 *   LED:    On-board RGB LED on GPIO3 (P3_0 blue, P3_12 red, P3_13 green)
 *   Buzzer: Passive buzzer on P2_4 (CT1_MAT0, Alt 4) — J1 pin 6
 *           CTIMER1 generates PWM at configurable frequency (1–4 kHz)
 *
 * Cue definitions (from architecture Layer 6 V_FB):
 *   puzzle_cue:  10 ms green LED flash (F3.2)
 *   unlock_cue:  200 ms blue LED + buzzer 2 kHz tone (F4.3)
 *   error_cue:   100 ms red LED flash + buzzer 500 Hz (F2.3 retry)
 *   final_cue:   3 s celebration — alternating colors + ascending tones (F3.6)
 *
 * Dependencies: <MCXA153.h>
 * Depended on by: main_fsm (Layer 5), V_FB (Layer 6)
 *
 * FR coverage: F3.2 (puzzle solved cue), F4.3 (unlock cue), F2.3 (retry cue)
 *****************************************************************************/
#ifndef FEEDBACK_DRV_H
#define FEEDBACK_DRV_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

/** Buzzer frequencies in Hz */
#define BUZZER_FREQ_LOW     500    /**< Error / retry tone       */
#define BUZZER_FREQ_MID     1000   /**< Notification tone        */
#define BUZZER_FREQ_HIGH    2000   /**< Success / unlock tone    */
#define BUZZER_FREQ_CELE    3000   /**< Celebration high tone    */

/** Cue durations in ms */
#define CUE_PUZZLE_MS       10     /**< F3.2: puzzle solved cue  */
#define CUE_UNLOCK_MS       200    /**< F4.3: unlock cue         */
#define CUE_ERROR_MS        100    /**< F2.3: retry cue          */
#define CUE_FINAL_MS        3000   /**< F3.6: celebration        */

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the feedback driver.
 *
 * Configures:
 *   - GPIO3 for RGB LED (same pins as leds.c but we own them here)
 *   - CTIMER1 for buzzer PWM on P2_4 (CT1_MAT0)
 *
 * Starts with LED off and buzzer silent.
 */
void feedback_drv_init(void);

/**
 * \brief Puzzle solved cue (F3.2).
 *
 * 10 ms green LED flash. Non-blocking (uses SysTick ms counter).
 * Call feedback_drv_update() in main loop to complete the cue.
 */
void feedback_cue_puzzle(void);

/**
 * \brief Unlock cue (F4.3).
 *
 * 200 ms blue LED + 2 kHz buzzer tone.
 * Call feedback_drv_update() in main loop to complete the cue.
 */
void feedback_cue_unlock(void);

/**
 * \brief Error / retry cue (F2.3).
 *
 * 100 ms red LED + 500 Hz buzzer tone.
 * Call feedback_drv_update() in main loop to complete the cue.
 */
void feedback_cue_error(void);

/**
 * \brief Celebration cue (F3.6).
 *
 * 3 s alternating colors + ascending tones.
 * Call feedback_drv_update() in main loop to complete the cue.
 */
void feedback_cue_final(void);

/**
 * \brief Update the feedback state machine (call from main loop).
 *
 * Handles timing of LED on/off and buzzer start/stop.
 * Returns true while a cue is still active, false when idle.
 */
bool feedback_drv_update(void);

/**
 * \brief Immediately stop all feedback (LED off, buzzer silent).
 */
void feedback_drv_stop(void);

/**
 * \brief Set buzzer frequency directly (for testing/calibration).
 * \param freq_hz  Frequency in Hz (0 = silent).
 */
void feedback_buzzer_set_freq(uint16_t freq_hz);

/**
 * \brief Turn buzzer off.
 */
void feedback_buzzer_off(void);

/**
 * \brief Check if a cue is currently playing.
 */
bool feedback_drv_is_active(void);

#endif /* FEEDBACK_DRV_H */
