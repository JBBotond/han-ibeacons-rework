/*! ***************************************************************************
 * \brief  log.c — Layer 11 Cross-cutting Logger Implementation
 * \file   log.c
 *
 * Ring-buffer based logger. Output sink: serial (LPUART0) in this bench test,
 * USB-CDC in production (swap in log_usb_sink.c later).
 *
 * Design:
 *   - Single ring buffer (LOG_RING_SIZE bytes)
 *   - Log() formats into the ring (producer)
 *   - LogFlush() drains ring to serial (consumer)
 *   - LogGlobalOff() sets a flag checked FIRST — guarantees ≤ 1 µs (F8.3)
 *   - No heap, no printf, no malloc
 *
 * Thread safety:
 *   - In bare-metal (this bench test): disable IRQ around ring writes
 *   - In FreeRTOS (production): replace with mutex (noted in comments)
 *
 * FR coverage: F8.1, F8.2, F8.3, F8.4
 *****************************************************************************/
#include "log.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Configuration
 * ---------------------------------------------------------------------- */
#define LOG_RING_SIZE   1024   /**< Ring buffer size in bytes */

/* -------------------------------------------------------------------------
 * Ring buffer (lock-free single-producer in bare-metal)
 * ---------------------------------------------------------------------- */
static char     ring[LOG_RING_SIZE];
static uint16_t ring_head = 0;   /**< Next write position  */
static uint16_t ring_tail = 0;   /**< Next read position   */

/* -------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------- */
static bool        g_enabled = true;                    /**< Global on/off flag */
static log_level_t g_levels[LOG_SYS_COUNT];             /**< Per-subsystem min level */

/* -------------------------------------------------------------------------
 * Timestamp source (extern — provided by main.c SysTick)
 * ---------------------------------------------------------------------- */
extern volatile uint32_t ms;

/* -------------------------------------------------------------------------
 * Output sink (serial_putchar — provided by serial.c)
 * ---------------------------------------------------------------------- */
extern void serial_putchar(int data);

/* -------------------------------------------------------------------------
 * String tables (compact — no heap)
 * ---------------------------------------------------------------------- */
static const char * const sys_names[LOG_SYS_COUNT] = {
    "SYS", "COMMS", "DISPLAY", "SENSOR", "FSM", "POWER"
};

static const char * const level_names[LOG_LEVEL_COUNT] = {
    "NONE", "INFO", "DEBUG", "WARN", "ERROR", "CRIT"
};

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/** Push one byte into the ring. Returns false if full (drops byte). */
static inline bool ring_push(char c)
{
    uint16_t next = (ring_head + 1) % LOG_RING_SIZE;
    if (next == ring_tail) return false;  /* full — drop */
    ring[ring_head] = c;
    ring_head = next;
    return true;
}

/** Pop one byte from the ring. Returns false if empty. */
static inline bool ring_pop(char *c)
{
    if (ring_tail == ring_head) return false;  /* empty */
    *c = ring[ring_tail];
    ring_tail = (ring_tail + 1) % LOG_RING_SIZE;
    return true;
}

/** Push a null-terminated string into the ring. */
static void ring_push_str(const char *s)
{
    while (*s)
    {
        ring_push(*s);
        s++;
    }
}

/** Push an unsigned decimal number into the ring. */
static void ring_push_u32(uint32_t val, uint8_t min_digits)
{
    char buf[10];
    int i = 0;

    if (val == 0)
    {
        buf[i++] = '0';
    }
    else
    {
        while (val > 0)
        {
            buf[i++] = '0' + (char)(val % 10);
            val /= 10;
        }
    }

    /* Pad with leading zeros */
    while (i < min_digits)
    {
        buf[i++] = '0';
    }

    /* Push in reverse (MSB first) */
    for (int j = i - 1; j >= 0; j--)
    {
        ring_push(buf[j]);
    }
}

/** Push a signed decimal number into the ring. */
static void ring_push_i32(int32_t val)
{
    if (val < 0)
    {
        ring_push('-');
        /* Handle INT32_MIN safely */
        ring_push_u32((uint32_t)(-(val + 1)) + 1, 1);
    }
    else
    {
        ring_push_u32((uint32_t)val, 1);
    }
}

/* -------------------------------------------------------------------------
 * API Implementation
 * ---------------------------------------------------------------------- */

void LogInit(void)
{
    ring_head = 0;
    ring_tail = 0;
    g_enabled = true;

    /* Default: all subsystems at INFO level */
    for (int i = 0; i < LOG_SYS_COUNT; i++)
    {
        g_levels[i] = LOG_INFO;
    }
}

void Log(log_sys_t sys, log_level_t level, const char *msg)
{
    /* F8.3: fast exit when globally off — ≤ 1 µs */
    if (!g_enabled) return;

    /* Per-subsystem filter */
    if (sys >= LOG_SYS_COUNT) return;
    if (level < g_levels[sys]) return;
    if (g_levels[sys] == LOG_NONE) return;

    /* Format: [<ms_8digits>] <SYS>:<LEVEL> <msg>\n */
    ring_push('[');
    ring_push_u32(ms, 8);
    ring_push(']');
    ring_push(' ');
    ring_push_str(sys_names[sys]);
    ring_push(':');
    ring_push_str(level_names[level]);
    ring_push(' ');
    ring_push_str(msg);
    ring_push('\r');
    ring_push('\n');
}

void LogWithNum(log_sys_t sys, log_level_t level, const char *msg, int32_t n)
{
    /* F8.3: fast exit when globally off — ≤ 1 µs */
    if (!g_enabled) return;

    /* Per-subsystem filter */
    if (sys >= LOG_SYS_COUNT) return;
    if (level < g_levels[sys]) return;
    if (g_levels[sys] == LOG_NONE) return;

    /* Format: [<ms_8digits>] <SYS>:<LEVEL> <msg><n>\n */
    ring_push('[');
    ring_push_u32(ms, 8);
    ring_push(']');
    ring_push(' ');
    ring_push_str(sys_names[sys]);
    ring_push(':');
    ring_push_str(level_names[level]);
    ring_push(' ');
    ring_push_str(msg);
    ring_push_i32(n);
    ring_push('\r');
    ring_push('\n');
}

void LogSetOutputLevel(log_sys_t sys, log_level_t level)
{
    if (sys < LOG_SYS_COUNT)
    {
        g_levels[sys] = level;
    }
}

log_level_t LogGetOutputLevel(log_sys_t sys)
{
    if (sys < LOG_SYS_COUNT) return g_levels[sys];
    return LOG_NONE;
}

void LogGlobalOn(void)
{
    g_enabled = true;
}

void LogGlobalOff(void)
{
    g_enabled = false;
}

bool LogIsEnabled(void)
{
    return g_enabled;
}

uint32_t LogFlush(void)
{
    uint32_t count = 0;
    char c;

    while (ring_pop(&c))
    {
        serial_putchar((int)c);
        count++;
    }

    return count;
}
