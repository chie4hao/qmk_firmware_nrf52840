#include <string.h>
#include "eeprom.h"
#include "fds.h"
#include "nrf_log.h"

#define FILE_ID 0x1000
#define RECORD_KEY 0x1001

#define EEPROM_SIZE 36

__ALIGN(4)
static uint8_t           buffer[EEPROM_SIZE] __attribute__((aligned(4)));
static bool              fds_inited         = false;
static volatile bool     fds_update         = false;
static volatile uint32_t fds_update_counter = 0;

// FDS record update after 500 ms.
#ifndef FDS_UPDATE_TIMEOUT
#    define FDS_UPDATE_TIMEOUT 500
#endif

static fds_record_desc_t  record_desc = {0};
static fds_record_t const record      = {.file_id = FILE_ID, .key = RECORD_KEY, .data.p_data = &buffer, .data.length_words = (EEPROM_SIZE + 3) / sizeof(uint32_t)};
/* Array to map FDS return values to strings. */

char const *fds_err_str[] = {
    "FDS_SUCCESS", "FDS_ERR_OPERATION_TIMEOUT", "FDS_ERR_NOT_INITIALIZED", "FDS_ERR_UNALIGNED_ADDR", "FDS_ERR_INVALID_ARG", "FDS_ERR_NULL_ARG", "FDS_ERR_NO_OPEN_RECORDS", "FDS_ERR_NO_SPACE_IN_FLASH", "FDS_ERR_NO_SPACE_IN_QUEUES", "FDS_ERR_RECORD_TOO_LARGE", "FDS_ERR_NOT_FOUND", "FDS_ERR_NO_PAGES", "FDS_ERR_USER_LIMIT_REACHED", "FDS_ERR_CRC_CHECK_FAILED", "FDS_ERR_BUSY", "FDS_ERR_INTERNAL",
}; /* Array to map FDS events to strings. */
static char const *fds_evt_str[] = {
    "FDS_EVT_INIT", "FDS_EVT_WRITE", "FDS_EVT_UPDATE", "FDS_EVT_DEL_RECORD", "FDS_EVT_DEL_FILE", "FDS_EVT_GC",
};

static void eeprom_write() {
    ret_code_t rc = fds_record_write(&record_desc, &record);
    APP_ERROR_CHECK(rc);
}

void eeprom_update() {
    if (!fds_update) {
        return;
    }
    if (fds_update_counter > FDS_UPDATE_TIMEOUT) {
        fds_update_counter = 0;
        fds_update         = false;
        ret_code_t rc      = fds_record_update(&record_desc, &record);
        APP_ERROR_CHECK(rc);
    } else {
        fds_update_counter += KEYBOARD_SCAN_INTERVAL;
    }
}

static bool volatile m_fds_initialized = false;

static void fds_evt_handler(fds_evt_t const *p_evt) {
    NRF_LOG_INFO("Event: %s received (%s)", fds_evt_str[p_evt->id], fds_err_str[p_evt->result]);

    switch (p_evt->id) {
        case FDS_EVT_INIT:
            if (p_evt->result == FDS_SUCCESS) {
                m_fds_initialized = true;

                fds_stat_t stat = {0};
                ret_code_t rc   = fds_stat(&stat);
                APP_ERROR_CHECK(rc);
                NRF_LOG_INFO("Found %d valid records.", stat.valid_records);
                NRF_LOG_INFO("Found %d dirty records (ready to be garbage collected).", stat.dirty_records);

                if (stat.dirty_records > 34) {
                    fds_gc();
                }
            }
            break;

        case FDS_EVT_WRITE: {
            if (p_evt->result == FDS_SUCCESS) {
                NRF_LOG_INFO("Record ID:\t0x%04x", p_evt->write.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x", p_evt->write.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->write.record_key);
            }
        } break;

        case FDS_EVT_DEL_RECORD: {
            if (p_evt->result == FDS_SUCCESS) {
                NRF_LOG_INFO("Record ID:\t0x%04x", p_evt->del.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x", p_evt->del.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->del.record_key);
            }
        } break;

        case FDS_EVT_GC: {
            if (p_evt->result == FDS_SUCCESS) {
                NRF_LOG_INFO("gc completed");
            }
        }

        default:
            break;
    }
}

/**@brief   Sleep until an event is received. */
static void power_manage(void) {
#ifdef SOFTDEVICE_PRESENT
    (void)sd_app_evt_wait();
#else
    __WFE();
#endif
}

static void eeprom_init(void) {
    ret_code_t rc;

    (void)fds_register(fds_evt_handler);
    rc = fds_init();
    APP_ERROR_CHECK(rc);

    while (!m_fds_initialized) {
        power_manage();
    }

    fds_find_token_t ftok = {0};

    memset(&ftok, 0x00, sizeof(fds_find_token_t));
    rc = fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok);

    if (rc == FDS_SUCCESS) {
        fds_flash_record_t flash_record = {0};

        rc = fds_record_open(&record_desc, &flash_record);
        APP_ERROR_CHECK(rc);

        memcpy(buffer, flash_record.p_data, EEPROM_SIZE);

        rc = fds_record_close(&record_desc);
        APP_ERROR_CHECK(rc);
    } else {
        eeprom_write();
    }
    fds_inited = true;
}

uint8_t eeprom_read_byte(const uint8_t *addr) {
    if (!fds_inited) {
        eeprom_init();
    }
    uintptr_t offset = (uintptr_t)addr;
    return buffer[offset];
}

void eeprom_write_byte(uint8_t *addr, uint8_t value) {
    if (!fds_inited) {
        eeprom_init();
    }
    uintptr_t offset   = (uintptr_t)addr;
    buffer[offset]     = value;
    fds_update         = true;
    fds_update_counter = 0;
}

uint16_t eeprom_read_word(const uint16_t *addr) {
    const uint8_t *p = (const uint8_t *)addr;
    return eeprom_read_byte(p) | (eeprom_read_byte(p + 1) << 8);
}

uint32_t eeprom_read_dword(const uint32_t *addr) {
    const uint8_t *p = (const uint8_t *)addr;
    return eeprom_read_byte(p) | (eeprom_read_byte(p + 1) << 8) | (eeprom_read_byte(p + 2) << 16) | (eeprom_read_byte(p + 3) << 24);
}

void eeprom_read_block(void *buf, const void *addr, uint32_t len) {
    const uint8_t *p    = (const uint8_t *)addr;
    uint8_t *      dest = (uint8_t *)buf;
    while (len--) {
        *dest++ = eeprom_read_byte(p++);
    }
}

void eeprom_write_word(uint16_t *addr, uint16_t value) {
    if (!fds_inited) {
        eeprom_init();
    }
    uintptr_t p        = (uintptr_t)(uint8_t *)addr;
    buffer[p++]        = value;
    buffer[p]          = value >> 8;
    fds_update         = true;
    fds_update_counter = 0;
}

void eeprom_write_dword(uint32_t *addr, uint32_t value) {
    if (!fds_inited) {
        eeprom_init();
    }
    uintptr_t p        = (uintptr_t)(uint8_t *)addr;
    buffer[p++]        = value;
    buffer[p++]        = value >> 8;
    buffer[p++]        = value >> 16;
    buffer[p]          = value >> 24;
    fds_update         = true;
    fds_update_counter = 0;
}

void eeprom_write_block(const void *buf, void *addr, uint32_t len) {
    if (!fds_inited) {
        eeprom_init();
    }
    uintptr_t      p   = (uintptr_t)(uint8_t *)addr;
    const uint8_t *src = (const uint8_t *)buf;
    while (len--) {
        buffer[p++] = *src++;
    }
    fds_update         = true;
    fds_update_counter = 0;
}

void eeprom_update_byte(uint8_t *addr, uint8_t value) { eeprom_write_byte(addr, value); }

void eeprom_update_word(uint16_t *addr, uint16_t value) { eeprom_write_word(addr, value); }

void eeprom_update_dword(uint32_t *addr, uint32_t value) { eeprom_write_dword(addr, value); }

void eeprom_update_block(const void *buf, void *addr, uint32_t len) { eeprom_write_block(buf, addr, len); }

// Dummy eeprom

// #define EEPROM_SIZE 32
// static uint8_t buffer[EEPROM_SIZE];

// uint8_t eeprom_read_byte(const uint8_t *addr) {
// 	uintptr_t offset = (uintptr_t)addr;
// 	return buffer[offset];
// }

// void eeprom_write_byte(uint8_t *addr, uint8_t value) {
// 	uintptr_t offset = (uintptr_t)addr;
// 	buffer[offset] = value;
// }

// uint16_t eeprom_read_word(const uint16_t *addr) {
// 	const uint8_t *p = (const uint8_t *)addr;
// 	return eeprom_read_byte(p) | (eeprom_read_byte(p+1) << 8);
// }

// uint32_t eeprom_read_dword(const uint32_t *addr) {
// 	const uint8_t *p = (const uint8_t *)addr;
// 	return eeprom_read_byte(p) | (eeprom_read_byte(p+1) << 8)
// 		| (eeprom_read_byte(p+2) << 16) | (eeprom_read_byte(p+3) << 24);
// }

// void eeprom_read_block(void *buf, const void *addr, uint32_t len) {
// 	const uint8_t *p = (const uint8_t *)addr;
// 	uint8_t *dest = (uint8_t *)buf;
// 	while (len--) {
// 		*dest++ = eeprom_read_byte(p++);
// 	}
// }

// void eeprom_write_word(uint16_t *addr, uint16_t value) {
// 	uint8_t *p = (uint8_t *)addr;
// 	eeprom_write_byte(p++, value);
// 	eeprom_write_byte(p, value >> 8);
// }

// void eeprom_write_dword(uint32_t *addr, uint32_t value) {
// 	uint8_t *p = (uint8_t *)addr;
// 	eeprom_write_byte(p++, value);
// 	eeprom_write_byte(p++, value >> 8);
// 	eeprom_write_byte(p++, value >> 16);
// 	eeprom_write_byte(p, value >> 24);
// }

// void eeprom_write_block(const void *buf, void *addr, uint32_t len) {
// 	uint8_t *p = (uint8_t *)addr;
// 	const uint8_t *src = (const uint8_t *)buf;
// 	while (len--) {
// 		eeprom_write_byte(p++, *src++);
// 	}
// }

// void eeprom_update_byte(uint8_t *addr, uint8_t value) {
// 	eeprom_write_byte(addr, value);
// }

// void eeprom_update_word(uint16_t *addr, uint16_t value) {
// 	uint8_t *p = (uint8_t *)addr;
// 	eeprom_write_byte(p++, value);
// 	eeprom_write_byte(p, value >> 8);
// }

// void eeprom_update_dword(uint32_t *addr, uint32_t value) {
// 	uint8_t *p = (uint8_t *)addr;
// 	eeprom_write_byte(p++, value);
// 	eeprom_write_byte(p++, value >> 8);
// 	eeprom_write_byte(p++, value >> 16);
// 	eeprom_write_byte(p, value >> 24);
// }

// void eeprom_update_block(const void *buf, void *addr, uint32_t len) {
// 	uint8_t *p = (uint8_t *)addr;
// 	const uint8_t *src = (const uint8_t *)buf;
// 	while (len--) {
// 		eeprom_write_byte(p++, *src++);
// 	}
// }
