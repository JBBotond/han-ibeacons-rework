/*! ***************************************************************************
 * \brief  button_drv.h — Layer 4 Button Driver
 * \file   button_drv.h
 *
 * Debounced button input using GPIO interrupt + SysTick-based debounce.
 *
 * Hardware (on-board FRDM-MCXA153):
 *   SW2: P3_29 (GPIO3) — active LOW, falling edge interrupt
 *   SW3: P1_7  (GPIO1) — active LOW, falling edge interrupt
 *
 * Debounce: 20 ms lockout after each valid press (software, no LPIT needed).
 * ISR latency budget: ≤ 10 ms (F2, F3 — "Button → event ≤ 10 ms").
 *
 * Event model: press events are queued in a small ring (4 slots).
 * The FSM polls button_drv_get_event() each cycle.
 *
 * Dependencies: <MCXA153.h>, SysTick ms counter
 * Depended on by: main_fsm (Layer 5) — puzzle input, admin trigger
 *
 * FR coverage: F2 (puzzle input), F3 (display interaction)
 *****************************************************************************/
#ifndef BUTTON_DRV_H
#define BUTTON_DRV_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

#define BUTTON_DEBOUNCE_MS   20    /**< Lockout period after valid press */
#define BUTTON_EVENT_QUEUE   8     /**< Max queued press events          */

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/** Button identifiers */
typedef enum
{
    BTN_NONE = 0,
    BTN_SW2  = 1,   /**< P3_29 — left button on FRDM board  */
    BTN_SW3  = 2    /**< P1_7  — right button on FRDM board */
} button_id_t;

/** Button event (what the FSM consumes) */
typedef struct
{
    button_id_t id;        /**< Which button was pressed     */
    uint32_t    timestamp; /**< ms tick when press occurred  */
} button_event_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the button driver.
 *
 * Configures GPIO1 (P1_7) and GPIO3 (P3_29) as inputs with falling-edge
 * interrupts. Enables NVIC for both GPIO ports.
 */
void button_drv_init(void);

/**
 * \brief Get the next button event from the queue.
 * \param evt  Pointer to event struct to fill.
 * \return true if an event was available, false if queue empty.
 *
 * Non-blocking. Call from main loop or scan_task.
 */
bool button_drv_get_event(button_event_t *evt);

/**
 * \brief Get the number of pending events in the queue.
 */
uint8_t button_drv_pending(void);

/**
 * \brief Flush all pending events (e.g., on state transition).
 */
void button_drv_flush(void);

/**
 * \brief Get total press count for a button (lifetime, for diagnostics).
 */
uint32_t button_drv_get_count(button_id_t id);

#endif /* BUTTON_DRV_H */
