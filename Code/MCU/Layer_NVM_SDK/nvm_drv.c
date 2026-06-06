/*! ***************************************************************************
 * \brief  nvm_drv.c — Layer 4 NVM Driver Implementation
 * \file   nvm_drv.c
 *
 * Uses MCXA153 ROM API (fsl_romapi.h) for flash erase/program.
 * Wear-leveling: 32 slots × 64 bytes in the last 2 KiB of flash.
 * Sequence number identifies the most recent record.
 *****************************************************************************/
#include "nvm_drv.h"
#include <MCXA153.h>
#include <string.h>
#include <stdint.h>

/* -------------------------------------------------------------------------
 * ROM API access — direct, no SDK headers needed.
 *
 * The MCXA153 ROM contains a flash driver accessed via a function pointer
 * table at address 0x03003FE0. We define the minimal structures here
 * to avoid pulling in fsl_common.h and its entire driver chain.
 * ---------------------------------------------------------------------- */

typedef int32_t status_t;

#define kStatus_Success  0
#define FLASH_API_ERASE_KEY  0x6B65666CUL  /* "lfek" in little-endian */

/** Flash configuration (filled by flash_init) */
typedef struct
{
    uint32_t PFlashBlockBase;
    uint32_t PFlashTotalSize;
    uint32_t PFlashBlockCount;
    uint32_t PFlashPageSize;
    uint32_t PFlashSectorSize;
    /* FFR config (we don't use it but need the struct size to match) */
    uint32_t ffrBlockBase;
    uint32_t ffrTotalSize;
    uint32_t ffrPageSize;
    uint32_t ffrSectorSize;
    uint32_t cfpaPageVersion;
    uint32_t cfpaPageOffset;
} rom_flash_config_t;

/** Flash driver interface (function pointer table in ROM) */
typedef struct
{
    status_t (*flash_init)(rom_flash_config_t *config);
    status_t (*flash_erase_sector)(rom_flash_config_t *config, uint32_t start, uint32_t lengthInBytes, uint32_t key);
    status_t (*flash_program_phrase)(rom_flash_config_t *config, uint32_t start, uint8_t *src, uint32_t lengthInBytes);
    status_t (*flash_program_page)(rom_flash_config_t *config, uint32_t start, uint8_t *src, uint32_t lengthInBytes);
    status_t (*flash_verify_program)(rom_flash_config_t *config, uint32_t start, uint32_t lengthInBytes,
                                     const uint8_t *expectedData, uint32_t *failedAddress, uint32_t *failedData);
    status_t (*flash_verify_erase_phrase)(rom_flash_config_t *config, uint32_t start, uint32_t lengthInBytes);
    status_t (*flash_verify_erase_page)(rom_flash_config_t *config, uint32_t start, uint32_t lengthInBytes);
    status_t (*flash_verify_erase_sector)(rom_flash_config_t *config, uint32_t start, uint32_t lengthInBytes);
    status_t (*flash_get_property)(rom_flash_config_t *config, uint32_t whichProperty, uint32_t *value);
} rom_flash_driver_t;

/** Bootloader tree (root of ROM API) */
typedef struct
{
    void (*run_bootloader)(void *arg);
    const rom_flash_driver_t *flash_driver;
    void (*jump)(void *arg);
} rom_bootloader_tree_t;

/** ROM API base address for MCXA153 */
#define ROM_API_BASE_ADDR  0x03003FE0UL
#define ROM_API  ((const rom_bootloader_tree_t *)ROM_API_BASE_ADDR)
#define FLASH_DRV  (ROM_API->flash_driver)

/** Flash property tags */
#define kFLASH_PropertySectorSize  0x00U
#define kFLASH_PropertyTotalSize   0x01U

/* -------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------- */
static rom_flash_config_t g_flash_config;
static uint32_t       g_sector_size = 0;
static uint32_t       g_next_seq = 1;
static uint8_t        g_next_slot = 0;
static bool           g_initialized = false;

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/** Read a slot directly from flash (memory-mapped). */
static const nvm_record_t *slot_ptr(uint8_t slot_idx)
{
    return (const nvm_record_t *)(NVM_BASE_ADDR + (uint32_t)slot_idx * NVM_SLOT_SIZE);
}

/** Scan all slots to find the one with the highest valid sequence number. */
static void scan_slots(void)
{
    uint32_t max_seq = 0;
    uint8_t  max_slot = 0;
    bool     found = false;

    for (uint8_t i = 0; i < NVM_SLOT_COUNT; i++)
    {
        const nvm_record_t *r = slot_ptr(i);
        if (r->magic == NVM_MAGIC && r->seq > max_seq)
        {
            max_seq = r->seq;
            max_slot = i;
            found = true;
        }
    }

    if (found)
    {
        g_next_seq = max_seq + 1;
        g_next_slot = (max_slot + 1) % NVM_SLOT_COUNT;
    }
    else
    {
        g_next_seq = 1;
        g_next_slot = 0;
    }
}

/* -------------------------------------------------------------------------
 * API Implementation
 * ---------------------------------------------------------------------- */

bool nvm_drv_init(void)
{
    /* Initialize ROM API flash driver */
    status_t status = FLASH_DRV->flash_init(&g_flash_config);
    if (status != kStatus_Success)
    {
        return false;
    }

    /* Query sector size */
    FLASH_DRV->flash_get_property(&g_flash_config, kFLASH_PropertySectorSize, &g_sector_size);

    /* Scan existing records */
    scan_slots();

    g_initialized = true;
    return true;
}

bool nvm_drv_write(const nvm_record_t *record)
{
    if (!g_initialized || record == NULL) return false;

    /* Prepare the record with sequence number */
    nvm_record_t wr;
    memcpy(&wr, record, sizeof(nvm_record_t));
    wr.magic = NVM_MAGIC;
    wr.seq = g_next_seq;

    /* Calculate target address */
    uint32_t target_addr = NVM_BASE_ADDR + (uint32_t)g_next_slot * NVM_SLOT_SIZE;

    /* Check if we need to erase (slot not empty = 0xFF) */
    const nvm_record_t *existing = slot_ptr(g_next_slot);
    bool needs_erase = (existing->magic != 0xFFFFFFFFUL);

    if (needs_erase)
    {
        /* Erase the sector containing this slot.
         * Note: erasing wipes all slots in that sector.
         * For simplicity, erase the entire NVM region when wrapping. */
        uint32_t sector_start = target_addr & ~(g_sector_size - 1);

        __disable_irq();
        status_t st = FLASH_DRV->flash_erase_sector(&g_flash_config, sector_start, g_sector_size,
                                                    FLASH_API_ERASE_KEY);
        __enable_irq();

        if (st != kStatus_Success) return false;
    }

    /* Program the record (must be phrase-aligned, 16-byte minimum) */
    __disable_irq();
    status_t st = FLASH_DRV->flash_program_phrase(&g_flash_config, target_addr,
                                                  (uint8_t *)&wr, NVM_SLOT_SIZE);
    __enable_irq();

    if (st != kStatus_Success) return false;

    /* Verify */
    uint32_t failed_addr, failed_data;
    st = FLASH_DRV->flash_verify_program(&g_flash_config, target_addr, NVM_SLOT_SIZE,
                                         (const uint8_t *)&wr, &failed_addr, &failed_data);

    if (st != kStatus_Success) return false;

    /* Advance to next slot */
    g_next_seq++;
    g_next_slot = (g_next_slot + 1) % NVM_SLOT_COUNT;

    return true;
}

bool nvm_drv_read_latest(nvm_record_t *record)
{
    if (!g_initialized || record == NULL) return false;

    uint32_t max_seq = 0;
    const nvm_record_t *best = NULL;

    for (uint8_t i = 0; i < NVM_SLOT_COUNT; i++)
    {
        const nvm_record_t *r = slot_ptr(i);
        if (r->magic == NVM_MAGIC && r->seq > max_seq)
        {
            max_seq = r->seq;
            best = r;
        }
    }

    if (best == NULL) return false;

    memcpy(record, best, sizeof(nvm_record_t));
    return true;
}

uint32_t nvm_drv_get_seq(void)
{
    return g_next_seq;
}

uint32_t nvm_drv_get_sector_size(void)
{
    return g_sector_size;
}

bool nvm_drv_erase_all(void)
{
    if (!g_initialized) return false;

    /* Erase all sectors in the NVM region */
    uint32_t addr = NVM_BASE_ADDR;
    while (addr < NVM_BASE_ADDR + NVM_SIZE)
    {
        __disable_irq();
        status_t st = FLASH_DRV->flash_erase_sector(&g_flash_config, addr, g_sector_size,
                                                    FLASH_API_ERASE_KEY);
        __enable_irq();

        if (st != kStatus_Success) return false;
        addr += g_sector_size;
    }

    /* Reset state */
    g_next_seq = 1;
    g_next_slot = 0;

    return true;
}

uint8_t nvm_drv_get_record_count(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < NVM_SLOT_COUNT; i++)
    {
        const nvm_record_t *r = slot_ptr(i);
        if (r->magic == NVM_MAGIC)
        {
            count++;
        }
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Config blob persistence (own sector at NVM_CONFIG_ADDR)
 * ---------------------------------------------------------------------- */

/** 16-byte header that precedes the payload in the config sector. */
typedef struct __attribute__((packed, aligned(4)))
{
    uint32_t magic;   /**< NVM_CONFIG_MAGIC when valid */
    uint32_t len;     /**< payload byte count          */
    uint32_t crc;     /**< CRC32 of the payload        */
    uint32_t pad;     /**< padding to 16 bytes (phrase) */
} nvm_config_hdr_t;

/** Simple CRC32 (poly 0xEDB88320, no table — small + good enough for integrity). */
static uint32_t crc32_calc(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFUL;
    for (uint32_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
        {
            uint32_t mask = -(crc & 1U);
            crc = (crc >> 1) ^ (0xEDB88320UL & mask);
        }
    }
    return ~crc;
}

/** Max config payload we will persist. config_store_t grew when beacons gained
 *  a role + puzzle_id and capacity rose to 16 (~300 B). 512 gives headroom and
 *  is still a tiny SRAM staging buffer vs the 8 KB config sector. */
#define NVM_CONFIG_MAX_PAYLOAD  512

bool nvm_drv_config_write(const void *data, uint32_t len)
{
    if (!g_initialized || data == NULL) return false;
    if (len == 0 || len > NVM_CONFIG_MAX_PAYLOAD) return false;

    uint32_t total = sizeof(nvm_config_hdr_t) + len;
    uint32_t prog_len = (total + 15U) & ~15U;   /* round up to phrase (16 B) */
    if (prog_len > NVM_CONFIG_SIZE) return false;

    /* Small staging buffer (NOT the whole sector — protects SRAM). */
    uint8_t buf[sizeof(nvm_config_hdr_t) + NVM_CONFIG_MAX_PAYLOAD];
    memset(buf, 0xFF, prog_len);

    nvm_config_hdr_t hdr;
    hdr.magic = NVM_CONFIG_MAGIC;
    hdr.len   = len;
    hdr.crc   = crc32_calc((const uint8_t *)data, len);
    hdr.pad   = 0;
    memcpy(buf, &hdr, sizeof(hdr));
    memcpy(buf + sizeof(hdr), data, len);

    /* Erase the config sector (its own — does NOT touch session records). */
    __disable_irq();
    status_t st = FLASH_DRV->flash_erase_sector(&g_flash_config, NVM_CONFIG_ADDR,
                                                NVM_CONFIG_SIZE, FLASH_API_ERASE_KEY);
    __enable_irq();
    if (st != kStatus_Success) return false;

    /* Program header + payload (phrase-aligned). */
    __disable_irq();
    st = FLASH_DRV->flash_program_phrase(&g_flash_config, NVM_CONFIG_ADDR, buf, prog_len);
    __enable_irq();
    if (st != kStatus_Success) return false;

    /* Verify. */
    uint32_t fa, fd;
    st = FLASH_DRV->flash_verify_program(&g_flash_config, NVM_CONFIG_ADDR, prog_len,
                                         buf, &fa, &fd);
    return (st == kStatus_Success);
}

bool nvm_drv_config_read(void *data, uint32_t len)
{
    if (!g_initialized || data == NULL) return false;

    const nvm_config_hdr_t *hdr = (const nvm_config_hdr_t *)NVM_CONFIG_ADDR;

    if (hdr->magic != NVM_CONFIG_MAGIC) return false;  /* never saved */
    if (hdr->len != len) return false;                 /* layout changed */

    const uint8_t *payload = (const uint8_t *)(NVM_CONFIG_ADDR + sizeof(nvm_config_hdr_t));
    if (crc32_calc(payload, len) != hdr->crc) return false;  /* corrupt */

    memcpy(data, payload, len);
    return true;
}

bool nvm_drv_config_erase(void)
{
    if (!g_initialized) return false;

    /* Erase the dedicated config sector. Next boot finds no magic and the
     * box falls back to config_init() factory defaults. */
    __disable_irq();
    status_t st = FLASH_DRV->flash_erase_sector(&g_flash_config, NVM_CONFIG_ADDR,
                                                NVM_CONFIG_SIZE, FLASH_API_ERASE_KEY);
    __enable_irq();

    return (st == kStatus_Success);
}
