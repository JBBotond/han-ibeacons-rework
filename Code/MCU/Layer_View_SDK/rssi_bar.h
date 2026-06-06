/*! ***************************************************************************
 * \brief  rssi_bar — Layer 6 View: "hotter/colder" proximity HUD
 * \file   rssi_bar.h
 *
 * Adopted verbatim from a colleague's SDK modification
 * (mcuxsdk/Layer_PUZZLES_SDK-MODIFICATION_FROM_COLLEAGUE/games/rssi_bar).
 * It is NOT a puzzle, so it is NOT in puzzle_registry — it is a View element
 * fed by the Model (location_engine RSSI). The FSM (Layer 5) is untouched;
 * the proximity bar is driven from scan_task_poll() during LOCATOR only.
 *
 * Depends on: tft_lcd.h (Layer 4 display driver).
 * Depended on by: main.c scan_task_poll (Layer 5 glue).
 *****************************************************************************/
#ifndef RSSI_BAR_H
#define RSSI_BAR_H

#include <stdint.h>

/**
 * Update the RSSI value used to compute the proximity bar fill.
 * Call this whenever a new scan result arrives.
 * Pass -90 (or any value <= RSSI_FAR) when signal is lost.
 */
void rssi_bar_update(int8_t rssi);

/**
 * Redraw the proximity bar at the top of the screen.
 * Blue = far (bar mostly empty), Red = close (bar mostly full).
 * Safe to call at any time — 320x6 = 1920 pixels fits within the
 * 80x80 framebuffer. Assumes ORIENTATION_90 (320x240 landscape).
 */
void rssi_bar_draw(void);

/**
 * Reset the rolling-average buffer to "far" (all samples = -90).
 * Call this when the FSM advances to a new target beacon so the bar
 * starts empty instead of showing stale proximity from the old target.
 */
void rssi_bar_reset(void);

#endif /* RSSI_BAR_H */
