/**
 * Copyright (c) 2012 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_app_hids_keyboard_main main.c
 * @{
 * @ingroup ble_sdk_app_hids_keyboard
 * @brief HID Keyboard Sample Application main file.
 *
 * This file contains is the source code for a sample application using the HID, Battery and Device
 * Information Services for implementing a simple keyboard functionality.
 * Pressing Button 0 will send text 'hello' to the connected peer. On receiving output report,
 * it toggles the state of LED 2 on the mother board based on whether or not Caps Lock is on.
 * This application uses the @ref app_scheduler.
 *
 * Also it would accept pairing requests from any peer device.
 */

#include <stdint.h>
#include <string.h>

#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "app_timer.h"
#include "fds.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "peer_manager_handler.h"
#include "adc.h"
// #include "ble_midi.h"
#include "ble_dfu.h"
#include "nrf_bootloader_info.h"
#include "nrf_power.h"
#include "nrf_drv_wdt.h"

#include "ble_service.h"
#include "ble_hids_service.h"
#include "outputselect.h"
#include "i2c_master.h"

uint8_t              keyboard_ble_led_stats = 0;
static volatile bool ble_disable            = true; /**< Handle of the current connection. */
#ifdef POWER_SAVE_TIMEOUT
static uint16_t power_save_counter = 0;
#endif
nrf_drv_wdt_channel_id m_channel_id;

#define DEVICE_NAME "Nordic_Keyboard"           /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME "NordicSemiconductor" /**< Manufacturer. Will be passed to Device Information Service. */

#define APP_BLE_OBSERVER_PRIO 3 /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG 1  /**< A tag identifying the SoftDevice BLE configuration. */

#define BATTERY_LEVEL_MEAS_INTERVAL APP_TIMER_TICKS(2000) /**< Battery level measurement interval (ticks). */

#define PNP_ID_VENDOR_ID_SOURCE 0x02  /**< Vendor ID Source. */
#define PNP_ID_VENDOR_ID 0x1915       /**< Vendor ID. */
#define PNP_ID_PRODUCT_ID 0xEEEE      /**< Product ID. */
#define PNP_ID_PRODUCT_VERSION 0x0001 /**< Product Version. */

#define APP_ADV_FAST_INTERVAL 0x0028 /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL 0x0C80 /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */

#define APP_ADV_FAST_DURATION 3000  /**< The advertising duration of fast advertising in units of 10 milliseconds. */
#define APP_ADV_SLOW_DURATION 18000 /**< The advertising duration of slow advertising in units of 10 milliseconds. */

/*lint -emacro(524, MIN_CONN_INTERVAL) // Loss of precision */
#define MIN_CONN_INTERVAL MSEC_TO_UNITS(7.5, UNIT_1_25_MS) /**< Minimum connection interval (7.5 ms) */
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(30, UNIT_1_25_MS)  /**< Maximum connection interval (30 ms). */
#define SLAVE_LATENCY 6                                    /**< Slave latency. */
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(430, UNIT_10_MS)    /**< Connection supervisory timeout (430 ms). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(30000) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT 3                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND 1                               /**< Perform bonding. */
#define SEC_PARAM_MITM 0                               /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC 0                               /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS 0                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES BLE_GAP_IO_CAPS_NONE /**< No I/O capabilities. */
#define SEC_PARAM_OOB 0                                /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE 7                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE 16                      /**< Maximum encryption key size. */

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define ISSI_REG_CONFIGURATION 0x00
#define ISSI_COMMANDREGISTER 0xFD
#define ISSI_COMMANDREGISTER_WRITELOCK 0xFE
#define ISSI_PAGE_FUNCTION 0x03

APP_TIMER_DEF(m_battery_timer_id); /**< Battery timer. */

BLE_BAS_DEF(m_bas);                 /**< Structure used to identify the battery service. */
NRF_BLE_GATT_DEF(m_gatt);           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising); /**< Advertising module instance. */

// BLE_MIDI_DEF(m_midi, NRF_SDH_BLE_TOTAL_LINK_COUNT);
// #define MIDI_SERVICE_UUID_TYPE          BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the MIDI Service (vendor specific). */

// static ble_uuid_t m_adv_uuids[]           =                                          /**< Universally unique service identifier. */
// {
//     {BLE_UUID_MIDI_SERVICE, MIDI_SERVICE_UUID_TYPE}
// };
// static uint16_t   m_ble_midi_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the MIDI service module. */

static pm_peer_id_t m_peer_id;
static uint32_t     m_whitelist_peer_cnt;
static pm_peer_id_t m_whitelist_peers[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
static void         sleep_mode_enter(void);

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};

static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void *p_context) {
    if (state == NRF_SDH_EVT_STATE_DISABLED) {
        // Softdevice was disabled before going into reset. Inform bootloader to skip CRC on next boot.
        nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        // Go to system off.
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

/* nrf_sdh state observer. */
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) = {
    .handler = buttonless_dfu_sdh_state_observer,
};

static void advertising_config_get(ble_adv_modes_config_t *p_config) {
    memset(p_config, 0, sizeof(ble_adv_modes_config_t));

    p_config->ble_adv_fast_enabled  = true;
    p_config->ble_adv_fast_interval = APP_ADV_FAST_INTERVAL;
    p_config->ble_adv_fast_timeout  = APP_ADV_FAST_DURATION;
}

static void disconnect(uint16_t conn_handle, void *p_context) {
    UNUSED_PARAMETER(p_context);

    ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", conn_handle, err_code);
    } else {
        NRF_LOG_DEBUG("Disconnected connection handle %d", conn_handle);
    }
}

/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event) {
    switch (event) {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE: {
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");

            // Prevent device from advertising on disconnect.
            ble_adv_modes_config_t config;
            advertising_config_get(&config);
            config.ble_adv_on_disconnect_disabled = true;
            ble_advertising_modes_config_set(&m_advertising, &config);

            // Disconnect all other bonded devices that currently are connected.
            // This is required to receive a service changed indication
            // on bootup after a successful (or aborted) Device Firmware Update.
            uint32_t conn_count = ble_conn_state_for_each_connected(disconnect, NULL);
            NRF_LOG_INFO("Disconnected %d links.", conn_count);
            break;
        }

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            NRF_LOG_ERROR("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}

static void dfu_init(void) {
    uint32_t                  err_code;
    ble_dfu_buttonless_init_t dfus_init = {0};

    dfus_init.evt_handler = ble_dfu_evt_handler;

    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
// void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name) { app_error_handler(DEAD_BEEF, line_num, p_file_name); }

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void) {
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void advertising_start(bool erase_bonds) {
    if (erase_bonds == true) {
        delete_bonds();
    } else {
        if (!ble_disable) {
            return;
        }
        ret_code_t ret;

        ble_adv_modes_config_t options;
        memset(&options, 0, sizeof(options));
        options.ble_adv_whitelist_enabled          = true;
        options.ble_adv_directed_high_duty_enabled = true;
        options.ble_adv_directed_enabled           = false;
        options.ble_adv_directed_interval          = 0;
        options.ble_adv_directed_timeout           = 0;
        options.ble_adv_fast_enabled               = true;
        options.ble_adv_fast_interval              = APP_ADV_FAST_INTERVAL;
        options.ble_adv_fast_timeout               = APP_ADV_FAST_DURATION;
        options.ble_adv_slow_enabled               = true;
        options.ble_adv_slow_interval              = APP_ADV_SLOW_INTERVAL;
        options.ble_adv_slow_timeout               = APP_ADV_SLOW_DURATION;
        options.ble_adv_on_disconnect_disabled     = false;
        ble_advertising_modes_config_set(&m_advertising, &options);

        m_whitelist_peer_cnt = BLE_GAP_DEVICE_IDENTITIES_MAX_COUNT;
        ret                  = pm_peer_id_list(m_whitelist_peers, &m_whitelist_peer_cnt, PM_PEER_ID_INVALID, PM_PEER_ID_LIST_SKIP_NO_ID_ADDR);
        APP_ERROR_CHECK(ret);

        NRF_LOG_INFO("advertising_start: m_whitelist_peer_cnt %d, MAX_PEERS_WLIST %d", m_whitelist_peer_cnt, BLE_GAP_WHITELIST_ADDR_MAX_COUNT);

        ret = pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt);
        APP_ERROR_CHECK(ret);

        ret = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
        if (ret != NRF_ERROR_NOT_SUPPORTED) {
            APP_ERROR_CHECK(ret);
        }

        ret = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(ret);
        ble_disable = false;
    }
}

void advertising_without_whitelist() {
    if (m_conn_handle == BLE_CONN_HANDLE_INVALID) {
        if (ble_disable == true) {
            advertising_start(false);
        }
        uint32_t err_code;
        err_code = ble_advertising_restart_without_whitelist(&m_advertising);
        if (err_code != NRF_ERROR_INVALID_STATE) {
            APP_ERROR_CHECK(err_code);
        }
    }
}

void ble_disconnect(void) {
    ret_code_t ret;
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        ret_code_t ret;
        ret = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        if (ret != NRF_ERROR_INVALID_STATE) {
            APP_ERROR_CHECK(ret);
        }
        ble_disable = true;
    } else {
        ret = sd_ble_gap_adv_stop(m_advertising.adv_handle);
        if (ret != NRF_ERROR_INVALID_STATE) {
            APP_ERROR_CHECK(ret);
        }
        ble_disable = true;
    }
}

void restart_advertising_id(uint8_t id) {
    ret_code_t ret;

    if (!ble_disable && m_conn_handle == BLE_CONN_HANDLE_INVALID) {
        ret = sd_ble_gap_adv_stop(m_advertising.adv_handle);
        if (ret != NRF_ERROR_INVALID_STATE) {
            APP_ERROR_CHECK(ret);
        }
    }

    m_whitelist_peer_cnt = BLE_GAP_DEVICE_IDENTITIES_MAX_COUNT;
    ret_code_t err_code  = pm_peer_id_list(m_whitelist_peers, &m_whitelist_peer_cnt, PM_PEER_ID_INVALID, PM_PEER_ID_LIST_SKIP_NO_ID_ADDR);
    APP_ERROR_CHECK(err_code);
    if (id > m_whitelist_peer_cnt) {
        return;
    }
    m_whitelist_peer_cnt = 1;
    m_whitelist_peers[0] = m_whitelist_peers[id];

    ret = pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt);
    APP_ERROR_CHECK(ret);

    ret = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
    if (ret != NRF_ERROR_NOT_SUPPORTED) {
        APP_ERROR_CHECK(ret);
    }

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        ble_adv_modes_config_t options;
        memset(&options, 0, sizeof(options));
        options.ble_adv_whitelist_enabled          = true;
        options.ble_adv_directed_high_duty_enabled = false;
        options.ble_adv_directed_enabled           = false;
        options.ble_adv_fast_enabled               = true;
        options.ble_adv_fast_interval              = APP_ADV_FAST_INTERVAL;
        options.ble_adv_fast_timeout               = APP_ADV_FAST_DURATION;
        options.ble_adv_slow_enabled               = true;
        options.ble_adv_slow_interval              = APP_ADV_SLOW_INTERVAL;
        options.ble_adv_slow_timeout               = APP_ADV_SLOW_DURATION;
        ble_advertising_modes_config_set(&m_advertising, &options);

        ret = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        if (ret != NRF_ERROR_INVALID_STATE) {
            APP_ERROR_CHECK(ret);
        }
    } else {
        ret = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(ret);
        ble_disable = false;
    }
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const *p_evt) {
    // pm_handler_on_pm_evt(p_evt);
    // pm_handler_flash_clean(p_evt);
    ret_code_t err_code;

    switch (p_evt->evt_id) {
        case PM_EVT_BONDED_PEER_CONNECTED: {
            NRF_LOG_DEBUG("Connected to a previously bonded device.");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED: {
            NRF_LOG_INFO("Connected to device %d.", p_evt->peer_id);
            NRF_LOG_DEBUG("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d, id: %d.", ble_conn_state_role(p_evt->conn_handle), p_evt->conn_handle, p_evt->params.conn_sec_succeeded.procedure, p_evt->peer_id);

            m_peer_id = p_evt->peer_id;
        } break;

        case PM_EVT_STORAGE_FULL: {
            NRF_LOG_INFO("PM storage full");
            err_code = fds_gc();
            if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES) {
                // Retry.
            } else {
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ: {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED: {
            NVIC_SystemReset();
        } break;

        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
            if (p_evt->params.peer_data_update_succeeded.flash_changed && (p_evt->params.peer_data_update_succeeded.data_id == PM_PEER_DATA_ID_BONDING)) {
                NRF_LOG_INFO("New Bond, add the peer to the whitelist if possible");
                // Note: You should check on what kind of white list policy your application should use.
                NRF_LOG_INFO("pm_evt_handler: m_whitelist_peer_cnt %d, MAX_PEERS_WLIST %d", m_whitelist_peer_cnt + 1, BLE_GAP_WHITELIST_ADDR_MAX_COUNT);
                // Note: You should check on what kind of white list policy your application should use.

                m_whitelist_peer_cnt = BLE_GAP_DEVICE_IDENTITIES_MAX_COUNT;
                err_code             = pm_peer_id_list(m_whitelist_peers, &m_whitelist_peer_cnt, PM_PEER_ID_INVALID, PM_PEER_ID_LIST_SKIP_NO_ID_ADDR);
                APP_ERROR_CHECK(err_code);

                err_code = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
                if (err_code != NRF_ERROR_NOT_SUPPORTED) {
                    APP_ERROR_CHECK(err_code);
                }
                err_code = pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt);
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}

/**@brief Function for performing a battery measurement, and update the Battery Level characteristic in the Battery Service.
 */
void battery_level_update(nrf_saadc_value_t value, uint16_t size) {
    ret_code_t err_code;
    uint8_t    battery_level;

    uint32_t voltage = (value * 6 * 600 * 143 / 256 / 100 / size);
    NRF_LOG_INFO("Battery voltage: %d mV", voltage);

    if (voltage > 4200) {
        battery_level = 100;
    } else if (voltage > 4000) {
        battery_level = 82 + 18 * (voltage - 4000) / 200;
    } else if (voltage > 3800) {
        battery_level = 44 + 38 * (voltage - 3800) / 200;
    } else if (voltage > 3740) {
        battery_level = 20 + 24 * (voltage - 3740) / 160;
    } else if (voltage > 3680) {
        battery_level = 10 + (voltage - 3680) / 6;
    } else if (voltage > 3000) {
        battery_level = (voltage - 3000) / 68;
    } else {
        battery_level = 0;
    }

    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY) && (err_code != NRF_ERROR_RESOURCES) && (err_code != NRF_ERROR_FORBIDDEN) && (err_code != NRF_ERROR_INVALID_STATE) && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {
        APP_ERROR_HANDLER(err_code);
    }
}

/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void *p_context) {
    UNUSED_PARAMETER(p_context);
#ifdef POWER_SAVE_TIMEOUT
    if (power_save_counter++ > POWER_SAVE_TIMEOUT / 2) {
        sleep_mode_enter();
    }
#endif
#ifdef KBD_WDT_ENABLE
    nrf_drv_wdt_channel_feed(m_channel_id);
#endif
    adc_start();
}

void reset_power_save_counter() {
#ifdef POWER_SAVE_TIMEOUT
    power_save_counter = 0;
#endif
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void) {
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    // err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HID_KEYBOARD);
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_HID);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

// /**@brief Function for handling events from the GATT library. */
// void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
// {
//     if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
//     {
//         m_ble_midi_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
//         NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_midi_max_data_len, m_ble_midi_max_data_len);
//         ble_midi_data_len_set(&m_midi, m_ble_midi_max_data_len);
//     }
//     NRF_LOG_INFO("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
//                   p_gatt->att_mtu_desired_central,
//                   p_gatt->att_mtu_desired_periph);
// }

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void) {
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);

    // ret_code_t err_code;

    // err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    // APP_ERROR_CHECK(err_code);

    // err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    // APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error) { APP_ERROR_HANDLER(nrf_error); }

/**@brief Function for initializing the Queued Write Module.
 */
static void qwr_init(void) {
    ret_code_t         err_code;
    nrf_ble_qwr_init_t qwr_init_obj = {0};

    qwr_init_obj.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init_obj);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing Device Information Service.
 */
static void dis_init(void) {
    ret_code_t       err_code;
    ble_dis_init_t   dis_init_obj;
    ble_dis_pnp_id_t pnp_id;

    pnp_id.vendor_id_source = PNP_ID_VENDOR_ID_SOURCE;
    pnp_id.vendor_id        = PNP_ID_VENDOR_ID;
    pnp_id.product_id       = PNP_ID_PRODUCT_ID;
    pnp_id.product_version  = PNP_ID_PRODUCT_VERSION;

    memset(&dis_init_obj, 0, sizeof(dis_init_obj));

    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
    dis_init_obj.p_pnp_id = &pnp_id;

    dis_init_obj.dis_char_rd_sec = SEC_JUST_WORKS;

    err_code = ble_dis_init(&dis_init_obj);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing Battery Service.
 */
static void bas_init(void) {
    ret_code_t     err_code;
    ble_bas_init_t bas_init_obj;

    memset(&bas_init_obj, 0, sizeof(bas_init_obj));

    bas_init_obj.evt_handler          = NULL;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref         = NULL;
    bas_init_obj.initial_batt_level   = 100;

    bas_init_obj.bl_rd_sec        = SEC_JUST_WORKS;
    bas_init_obj.bl_cccd_wr_sec   = SEC_JUST_WORKS;
    bas_init_obj.bl_report_rd_sec = SEC_JUST_WORKS;

    err_code = ble_bas_init(&m_bas, &bas_init_obj);
    APP_ERROR_CHECK(err_code);
}

// /**@brief Function for handling the data from the MIDI Service.
//  *
//  * @param[in] p_evt MIDI Service event.
//  */
// static void midis_data_handler(ble_midis_evt_t * p_evt)
// {

//     if (p_evt->type == BLE_MIDI_EVT_NOTIF_STARTED)
//     {
//         NRF_LOG_INFO("Notifications enabled.");
//     }
//     if (p_evt->type == BLE_MIDI_EVT_NOTIF_STOPPED)
//     {
//         NRF_LOG_INFO("Notifications disabled.");
//     }
//     if (p_evt->type == BLE_MIDI_EVT_RX_DATA)
//     {
//         NRF_LOG_DEBUG("Received data from MIDI Service.");
//         NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
//     }

// }

// static void midi_init(void)
// {
//     uint32_t           err_code;
//     ble_midis_init_t   midis_init;

//     // Initialize MIDI Service.
//     memset(&midis_init, 0, sizeof(midis_init));

//     midis_init.data_handler = midis_data_handler;
//     midis_init.make_data_running = true;

//     err_code = ble_midis_init(&m_midi, &midis_init);
//     APP_ERROR_CHECK(err_code);
// }

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void) {
    qwr_init();
    dis_init();
    dfu_init();
    bas_init();
    hids_kbd_init();

    // midi_init();
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error) { APP_ERROR_HANDLER(nrf_error); }

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void) {
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Timer initialization.
 */
static void timers_init(void) {
    ret_code_t err_code;

    // Create battery timer.
    err_code = app_timer_create(&m_battery_timer_id, APP_TIMER_MODE_REPEATED, battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting timers.
 */
static void timers_start(void) {
    ret_code_t err_code;

    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void) {
#ifdef IS31FL3737
    i2c_stop();
#endif
    NRF_LOG_INFO("Sleep mode");
    ret_code_t err_code;

    static const uint8_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
    static const uint8_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        nrf_gpio_cfg_output(row_pins[i]);
        nrf_gpio_pin_clear(row_pins[i]);
    }
    for (uint8_t i = 0; i < MATRIX_COLS; i++) {
        nrf_gpio_cfg_sense_input(col_pins[i], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    }

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    while (1) {
    }
    APP_ERROR_CHECK(err_code);
}

void deep_sleep_mode_enter(void) {
#ifdef IS31FL3737
    i2c_stop();
#endif
    NRF_LOG_INFO("Deep sleep mode");
    ret_code_t err_code;

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    while (1) {
    }
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {
    ret_code_t err_code;

    switch (ble_adv_evt) {
        case BLE_ADV_EVT_DIRECTED_HIGH_DUTY:
            NRF_LOG_INFO("High Duty Directed advertising.");
            // err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
            // APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_DIRECTED:
            NRF_LOG_INFO("Directed advertising.");
            // err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
            // APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");

            // err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            // APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_SLOW:
            NRF_LOG_INFO("Slow advertising.");
            // err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
            // APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_FAST_WHITELIST:
            NRF_LOG_INFO("Fast advertising with whitelist.");
            // err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            // APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_SLOW_WHITELIST:
            NRF_LOG_INFO("Slow advertising with whitelist.");
            // err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            // APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        case BLE_ADV_EVT_WHITELIST_REQUEST: {
            ble_gap_addr_t whitelist_addrs[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t  whitelist_irks[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            uint32_t       addr_cnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            uint32_t       irk_cnt  = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

            err_code = pm_whitelist_get(whitelist_addrs, &addr_cnt, whitelist_irks, &irk_cnt);
            APP_ERROR_CHECK(err_code);
            NRF_LOG_DEBUG("pm_whitelist_get returns %d addr in whitelist and %d irk whitelist", addr_cnt, irk_cnt);

            // Apply the whitelist.
            err_code = ble_advertising_whitelist_reply(&m_advertising, whitelist_addrs, addr_cnt, whitelist_irks, irk_cnt);
            APP_ERROR_CHECK(err_code);
        } break;  // BLE_ADV_EVT_WHITELIST_REQUEST

        case BLE_ADV_EVT_PEER_ADDR_REQUEST: {
            pm_peer_data_bonding_t peer_bonding_data;

            // Only Give peer address if we have a handle to the bonded peer.
            if (m_peer_id != PM_PEER_ID_INVALID) {
                err_code = pm_peer_data_bonding_load(m_peer_id, &peer_bonding_data);
                if (err_code != NRF_ERROR_NOT_FOUND) {
                    APP_ERROR_CHECK(err_code);
                    ble_gap_addr_t *p_peer_addr = &(peer_bonding_data.peer_ble_id.id_addr_info);
                    err_code                    = ble_advertising_peer_addr_reply(&m_advertising, p_peer_addr);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break;  // BLE_ADV_EVT_PEER_ADDR_REQUEST

        default:
            break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context) {
    ret_code_t err_code;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            // err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            // APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code      = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);

            ble_adv_modes_config_t options;
            memset(&options, 0, sizeof(options));
            options.ble_adv_whitelist_enabled          = true;
            options.ble_adv_directed_high_duty_enabled = true;
            options.ble_adv_directed_enabled           = false;
            options.ble_adv_directed_interval          = 0;
            options.ble_adv_directed_timeout           = 0;
            options.ble_adv_fast_enabled               = true;
            options.ble_adv_fast_interval              = APP_ADV_FAST_INTERVAL;
            options.ble_adv_fast_timeout               = APP_ADV_FAST_DURATION;
            options.ble_adv_slow_enabled               = true;
            options.ble_adv_slow_interval              = APP_ADV_SLOW_INTERVAL;
            options.ble_adv_slow_timeout               = APP_ADV_SLOW_DURATION;

            ble_advertising_modes_config_set(&m_advertising, &options);

            set_output(OUTPUT_BLUETOOTH);
            // if (!NRF_USBD->ENABLE) {
            //     set_output(OUTPUT_BLUETOOTH);
            // }

            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;

            // Reset m_caps_on variable. Upon reconnect, the HID host will re-send the Output
            // report containing the Caps lock state.
            // disabling alert 3. signal - used for capslock ON
            keyboard_ble_led_stats = 0;

            if (ble_disable) {
                ret_code_t ret;
                ret = sd_ble_gap_adv_stop(m_advertising.adv_handle);
                if (ret != NRF_ERROR_INVALID_STATE) {
                    APP_ERROR_CHECK(ret);
                }
            }
            // err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
            // APP_ERROR_CHECK(err_code);

            break;  // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys = {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            // // Send next key event
            // (void) buffer_dequeue(true);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void) {
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code           = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for the Event Scheduler initialization.
 */
// static void scheduler_init(void) { APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE); }

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void) {
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void ble_advertising_error_handler(uint32_t nrf_error) { APP_ERROR_HANDLER(nrf_error); }

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void) {
    uint32_t               err_code;
    uint8_t                adv_flags;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    adv_flags                            = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = adv_flags;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_whitelist_enabled          = true;
    init.config.ble_adv_directed_high_duty_enabled = true;
    init.config.ble_adv_directed_enabled           = false;
    init.config.ble_adv_directed_interval          = 0;
    init.config.ble_adv_directed_timeout           = 0;
    init.config.ble_adv_fast_enabled               = true;
    init.config.ble_adv_fast_interval              = APP_ADV_FAST_INTERVAL;
    init.config.ble_adv_fast_timeout               = APP_ADV_FAST_DURATION;
    init.config.ble_adv_slow_enabled               = true;
    init.config.ble_adv_slow_interval              = APP_ADV_SLOW_INTERVAL;
    init.config.ble_adv_slow_timeout               = APP_ADV_SLOW_DURATION;

    init.evt_handler   = on_adv_evt;
    init.error_handler = ble_advertising_error_handler;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void) {
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
void idle_state_handle(void) {
    if (NRF_LOG_PROCESS() == false) {
        nrf_pwr_mgmt_run();
    }
}

// #include "timer.h"

// uint32_t app_timer_ms(uint32_t ticks)
// {
// 	float numerator = ((float)APP_TIMER_CONFIG_RTC_FREQUENCY + 1.0f) * 1000.0f;
// 	float denominator = (float)APP_TIMER_CLOCK_FREQ;
// 	float ms_per_tick = numerator / denominator;

// 	uint32_t ms = ms_per_tick * ticks;

// 	return ms;
// }

// void send_midis() {
//     uint32_t timestamp_ticks  = app_timer_cnt_get();
//     uint32_t timestamp_ms     = app_timer_ms(timestamp_ticks);
//     uint32_t err_code;
//     uint8_t midi_msg_buffer[] =
//         {
//             (MIDI_CH_NOTE_ON | MIDI_CHANNEL_1),  //Status       (Channel)
//             60,                                  //Data1        (C note)
//             MIDI_VELOCITY_MAX,                   //Data2        (Velocity)
//             64,                                  //Running data (E note)
//             MIDI_VELOCITY_MAX,                   //Running data (Velocity)
//             MIDI_SYS_RT_CONTINUE,                //Status       (System Real Time)
//             (MIDI_CH_NOTE_ON | MIDI_CHANNEL_1),  //Status       (Channel)
//             67,                                  //Data1        (G Note)
//             MIDI_VELOCITY_MAX                    //Data2        (Velocity)
//         };

//         uint8_array_t midi_array;
//         midi_array.p_data = midi_msg_buffer;
//         midi_array.size   = sizeof(midi_msg_buffer);
//         err_code = ble_midi_msgs_send(&m_midi,
//                            &timestamp_ms,
//                            m_conn_handle,
//                            midi_array);
//     NRF_LOG_INFO("send_midis err_code: %d",err_code);
// }

/**@brief Function for application main entry.
 */

#ifdef KBD_WDT_ENABLE
static void wdt_init() {
    uint32_t             err_code = NRF_SUCCESS;
    nrf_drv_wdt_config_t config   = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code                      = nrf_drv_wdt_init(&config, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
}
#endif
void ble_service_init(void) {
    // Initialize.
    adc_init();
    timers_init();
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    peer_manager_init();

    // Start execution.
    NRF_LOG_INFO("HID Keyboard example started.");
#ifdef KBD_WDT_ENABLE
    wdt_init();
#endif
    timers_start();
}
