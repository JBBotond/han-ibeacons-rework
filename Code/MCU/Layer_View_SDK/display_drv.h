/*! ***************************************************************************
 * \brief  display_drv — MCXA153 implementation of display.h (Layer 4)
 * \file   display_drv.h
 *
 * Wraps tft_lcd (ILI9341 240x320) behind the portable display HAL.
 * Orientation is fixed to ORIENTATION_90 (landscape 320x240).
 *****************************************************************************/
#ifndef DISPLAY_DRV_H
#define DISPLAY_DRV_H

#include "display.h"

/* Re-export color macros from tft_lcd so callers need only display_drv.h */
#include "tft_lcd.h"

#endif /* DISPLAY_DRV_H */
