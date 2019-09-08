
#include <stdint.h>

#include "ble_hids.h"
#include "bsp.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "host_driver.h"

#define KBD_BLE_HID_DESC_NUM 0

#ifdef EXTRAKEY_ENABLE
#    define EXTRA_BLE_HID_DESC_NUM (KBD_BLE_HID_DESC_NUM + 2)
#else
#    define EXTRA_BLE_HID_DESC_NUM (KBD_BLE_HID_DESC_NUM + 0)
#endif

#ifdef MOUSE_ENABLE
#    define MOUSE_BLE_HID_DESC_NUM (EXTRA_BLE_HID_DESC_NUM + 1)
#else
#    define MOUSE_BLE_HID_DESC_NUM (EXTRA_BLE_HID_DESC_NUM + 0)
#endif

#define NUM_BLE_HID_DESC (MOUSE_BLE_HID_DESC_NUM + 1)

#ifndef BLE_NKRO
#    define KBD_INPUT_REPORT_KEYS_MAX_LEN 8 /**< Maximum length of the Input Report characteristic. */
#else
#    define KBD_INPUT_REPORT_KEYS_MAX_LEN NKRO_EPSIZE
#endif

#define MOUSE_INPUT_REPORT_KEYS_MAX_LEN 5
#define SYSTEM_INPUT_REPORT_KEYS_MAX_LEN 2
#define CONSUMER_INPUT_REPORT_KEYS_MAX_LEN 2

extern uint8_t  keyboard_ble_led_stats;
extern uint16_t m_conn_handle; /**< Handle of the current connection. */
extern uint8_t  keyboard_protocol;
void            hids_kbd_init(void);
void            keys_send(uint8_t report_index, uint16_t len, uint8_t* p_data);
