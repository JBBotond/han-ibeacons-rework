/*! ***************************************************************************
 * \brief  log.h — Layer 11 Cross-cutting Logger API (F8)
 * \file   log.h
 *
 * Single logging API used by every driver, the FSM, and the model.
 * printf is banned (F8.1). This replaces it.
 *
 * API:
 *   Log(sys, level, msg);
 *   LogWithNum(sys, level, msg, n);
 *   LogSetOutputLevel(sys, level);
 *   LogGlobalOn();
 *   LogGlobalOff();
 *
 * Performance contract (F8.3):
 *   WHEN LogGlobalOff() is called, subsequent log calls SHALL add
 *   <= 1 us each on the FRDM-MCXA153.
 *
 * Dependencies: <stdint.h>, <stdbool.h>
 * Depended on by: ALL layers (drivers, FSM, model, view)
 *
 * FR coverage: F8.1 (exclusive API), F8.2 (6 levels × 6 subsystems),
 *              F8.3 (≤1 µs when off), F8.4 (newline-terminated ASCII)
 *****************************************************************************/
#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Subsystems (sys) — F8.2
 * ---------------------------------------------------------------------- */
typedef enum
{
    LOG_SYS_SYS     = 0,   /**< System / boot / init          */
    LOG_SYS_COMMS   = 1,   /**< BLE, UART, USB communications */
    LOG_SYS_DISPLAY = 2,   /**< TFT display, V_SCREEN, V_ICONS */
    LOG_SYS_SENSOR  = 3,   /**< iBeacon / RSSI / location     */
    LOG_SYS_FSM     = 4,   /**< main_fsm state transitions    */
    LOG_SYS_POWER   = 5,   /**< PWR, WWDT, brown-out          */
    LOG_SYS_COUNT   = 6
} log_sys_t;

/* -------------------------------------------------------------------------
 * Levels — F8.2
 * ---------------------------------------------------------------------- */
typedef enum
{
    LOG_NONE     = 0,   /**< Output disabled for this subsystem */
    LOG_INFO     = 1,   /**< Normal operation milestones        */
    LOG_DEBUG    = 2,   /**< Verbose development data           */
    LOG_WARN     = 3,   /**< Recoverable anomaly                */
    LOG_ERROR    = 4,   /**< Non-recoverable but system alive   */
    LOG_CRIT     = 5,   /**< System about to halt / reset       */
    LOG_LEVEL_COUNT = 6
} log_level_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the logger (ring buffer + output sink).
 *
 * Must be called once at boot, after serial_init().
 * Sets all subsystems to LOG_INFO by default.
 */
void LogInit(void);

/**
 * \brief Log a text message.
 * \param sys    Subsystem tag.
 * \param level  Severity level.
 * \param msg    Null-terminated ASCII string (no newline needed).
 *
 * Output format (F8.4 — newline-terminated ASCII):
 *   [<ms>] <SYS>:<LEVEL> <msg>\n
 *
 * Example:
 *   [00001234] COMMS:INFO HM-10 connected
 */
void Log(log_sys_t sys, log_level_t level, const char *msg);

/**
 * \brief Log a text message with one numeric value.
 * \param sys    Subsystem tag.
 * \param level  Severity level.
 * \param msg    Null-terminated ASCII string.
 * \param n      Numeric value appended as signed decimal.
 *
 * Example:
 *   [00001500] SENSOR:DEBUG rssi=-69
 */
void LogWithNum(log_sys_t sys, log_level_t level, const char *msg, int32_t n);

/**
 * \brief Set the minimum output level for a subsystem.
 * \param sys    Subsystem to configure.
 * \param level  Minimum level to output. LOG_NONE disables all output.
 *
 * Messages with severity < level are silently dropped.
 * Example: LogSetOutputLevel(LOG_SYS_DISPLAY, LOG_NONE) silences display logs.
 */
void LogSetOutputLevel(log_sys_t sys, log_level_t level);

/**
 * \brief Get the current output level for a subsystem.
 */
log_level_t LogGetOutputLevel(log_sys_t sys);

/**
 * \brief Enable global log output.
 *
 * Restores per-subsystem filtering. Default state at boot.
 */
void LogGlobalOn(void);

/**
 * \brief Disable ALL log output globally.
 *
 * After this call, Log() and LogWithNum() return in ≤ 1 µs (F8.3).
 * The ring buffer is not touched. Per-subsystem levels are preserved.
 */
void LogGlobalOff(void);

/**
 * \brief Check if global logging is enabled.
 */
bool LogIsEnabled(void);

/**
 * \brief Flush pending log entries to the output sink (USB-CDC / serial).
 *
 * Called by logger_task in the production firmware, or by main loop
 * in this bench test. Non-blocking if ring is empty.
 *
 * \return Number of bytes flushed.
 */
uint32_t LogFlush(void);

#endif /* LOG_H */
