/*! ***************************************************************************
 * \brief  nvm_drv.h — Layer 4 Non-Volatile Memory Driver
 * \file   nvm_drv.h
 *
 * Persists session state to on-chip flash using the MCXA153 ROM API.
 * Data survives complete power removal (F7.2, F7.3).
 *
 * Storage layout:
 *   Last 2 KiB of flash (0x1F800–0x1FFFF) reserved for NVM.
 *   Divided into slots of 64 bytes each → 32 slots per 2 KiB.
 *   Wear-leveling: writes rotate through slots sequentially.
 *   Each slot has a 4-byte sequence number — highest = most recent.
 *
 * Session record (persisted on each FSM state transition):
 *   { magic, seq, room_id, timestamp_ms, outcome, route_progress }
 *
 * Dependencies: fsl_romapi.h (ROM API), <MCXA153.h>
 * Depended on by: main_fsm (Layer 5), log_model (Layer 7)
 *
 * FR coverage: F7.2 (persist within 50 ms), F7.3 (recover after power cut)
 *****************************************************************************/
#ifndef NVM_DRV_H
#define NVM_DRV_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

/** NVM storage region (last 8 KiB sector of 128 KiB flash)
 *  Sector size on MCXA153 = 8192 bytes. Must be sector-aligned.
 */
#define NVM_BASE_ADDR       0x0001E000UL
#define NVM_SIZE            0x00002000UL  /* 8192 bytes = 1 full sector */

/** Config-blob region — its OWN sector, one below the session region.
 *  Session records are erased on every FSM transition, so the persistent
 *  venue config (rooms + route + secret) MUST live in a separate sector
 *  to survive. Sector-aligned at 0x1C000 (free: code ends well below).
 */
#define NVM_CONFIG_ADDR     0x0001C000UL
#define NVM_CONFIG_SIZE     0x00002000UL  /* 8192 bytes = 1 full sector */
#define NVM_CONFIG_MAGIC    0x4B424346UL  /* "KBCF" = KidBytes ConFig    */

/** Slot size (must be phrase-aligned = multiple of 16 bytes) */
#define NVM_SLOT_SIZE       64

/** Number of slots in the NVM region */
#define NVM_SLOT_COUNT      (NVM_SIZE / NVM_SLOT_SIZE)  /* 128 */

/** Magic value to identify valid slots */
#define NVM_MAGIC           0x4B425356UL  /* "KBSV" = KidBytes Session Valid */

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/** Outcome of a room visit */
typedef enum
{
    NVM_OUTCOME_ENTERED   = 0,   /**< Kid entered the room              */
    NVM_OUTCOME_PUZZLE_OK = 1,   /**< Puzzle solved correctly            */
    NVM_OUTCOME_PUZZLE_FAIL = 2, /**< Puzzle attempt failed              */
    NVM_OUTCOME_UNLOCKED  = 3,   /**< Lock opened (FINAL state)         */
    NVM_OUTCOME_ADMIN     = 4    /**< Admin action                       */
} nvm_outcome_t;

/** Session record — 64 bytes, phrase-aligned */
typedef struct __attribute__((packed, aligned(4)))
{
    uint32_t magic;          /**< NVM_MAGIC if valid                    */
    uint32_t seq;            /**< Sequence number (monotonic, highest = newest) */
    uint8_t  room_id;        /**< Current room (1–5, or 0 = none)      */
    uint8_t  route_progress; /**< Rooms completed so far (0–5)         */
    uint8_t  outcome;        /**< nvm_outcome_t                        */
    uint8_t  reserved1;      /**< Padding                              */
    uint32_t timestamp_ms;   /**< SysTick ms at time of write          */
    uint32_t session_id;     /**< Session counter (increments on boot) */
    uint8_t  room_times[5];  /**< Time spent in each room (seconds, capped at 255) */
    uint8_t  reserved2[3];   /**< Padding to 32 bytes                  */
    uint8_t  spare[32];      /**< Future use (total = 64 bytes)        */
} nvm_record_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the NVM driver.
 *
 * Calls FLASH_Init() via ROM API, queries sector size, and scans
 * existing slots to find the most recent valid record.
 *
 * \return true if flash initialized successfully.
 */
bool nvm_drv_init(void);

/**
 * \brief Write a session record to the next available slot.
 * \param record  Pointer to the record to persist.
 * \return true if write + verify succeeded, false on flash error.
 *
 * Automatically handles wear-leveling (rotates through 32 slots).
 * When all slots are used, erases the sector and starts over.
 * Total time: < 50 ms (F7.2).
 */
bool nvm_drv_write(const nvm_record_t *record);

/**
 * \brief Read the most recent valid session record.
 * \param record  Pointer to buffer to fill with the latest record.
 * \return true if a valid record was found, false if NVM is empty/erased.
 */
bool nvm_drv_read_latest(nvm_record_t *record);

/**
 * \brief Get the current sequence number (for the next write).
 */
uint32_t nvm_drv_get_seq(void);

/**
 * \brief Get the flash sector size (queried from ROM API at init).
 */
uint32_t nvm_drv_get_sector_size(void);

/**
 * \brief Erase the entire NVM region (factory reset).
 * \return true if erase succeeded.
 */
bool nvm_drv_erase_all(void);

/**
 * \brief Get the number of valid records currently stored.
 */
uint8_t nvm_drv_get_record_count(void);

/* -------------------------------------------------------------------------
 * Config blob persistence (F5.2 / F7.3 — venue config survives power cut)
 *
 * Stored in its OWN sector (NVM_CONFIG_ADDR), separate from the session
 * records above, because those are erased on every FSM transition. The blob
 * is opaque to this driver: the caller (admin/config layer) marshals its
 * struct in/out. A header { magic, len, crc } guards integrity.
 * ---------------------------------------------------------------------- */

/**
 * \brief Persist an opaque config blob to the dedicated config sector.
 * \param data  Pointer to the bytes to store.
 * \param len   Number of bytes (<= NVM_CONFIG_SIZE - 16 header bytes).
 * \return true if erase + program + verify all succeeded.
 *
 * Erases the config sector, writes a 16-byte header (magic + len + crc32),
 * then the payload. Single-shot (no wear-leveling) — config changes rarely.
 */
bool nvm_drv_config_write(const void *data, uint32_t len);

/**
 * \brief Load a previously persisted config blob.
 * \param data  Buffer to receive the bytes.
 * \param len   Exact expected length (must match the stored len).
 * \return true if a valid blob (magic + len + crc all match) was loaded.
 *
 * Returns false when the sector is blank (never saved) or fails the CRC,
 * so the caller can fall back to factory defaults.
 */
bool nvm_drv_config_read(void *data, uint32_t len);

/**
 * \brief Erase the config sector (forget the saved venue config).
 * \return true if the erase succeeded.
 *
 * After this, the next boot finds no valid blob and falls back to
 * config_init() factory defaults.
 */
bool nvm_drv_config_erase(void);

#endif /* NVM_DRV_H */
