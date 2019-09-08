/**
 * Copyright (c) 2012 - 2018, Nordic Semiconductor ASA
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

#include "sdk_common.h"
#include "ble.h"
#include "ble_midi.h"
#include "ble_srv_common.h"

#define NRF_LOG_MODULE_NAME ble_midi
#define NRF_LOG_LEVEL 3
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define BLE_MIDI_MAX_RX_CHAR_LEN BLE_MIDI_MAX_DATA_LEN /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_MIDI_HEADER_SIZE 1                         /**< Every BLE-MIDI start with a header byte.*/
#define MIDI_CHAR_16 0xE5DB                            /**< 16bit version can be used after the 128bit UUID is registered with the SoftDevice. */
#define MIDI_SERVICE_16 0x0E5A                         /**< 16bit version can be used after the 128bit UUID is registered with the SoftDevice. */

#define MIDIS_SERVICE_UUID                                                                                 \
    {                                                                                                      \
        { 0x00, 0xC7, 0xC4, 0x4E, 0xE3, 0x6C, 0x51, 0xA7, 0x33, 0x4B, 0xE8, 0xED, 0x5A, 0x0E, 0xB8, 0x03 } \
    } /**< Vendor specific UUID of the MIDI Service. */

#define MIDIS_CHAR_UUID                                                                                    \
    {                                                                                                      \
        { 0xF3, 0x6B, 0x10, 0x9D, 0x66, 0xF2, 0xA9, 0xA1, 0x12, 0x41, 0x68, 0x38, 0xDB, 0xE5, 0x72, 0x77 } \
    } /**< Vendor specific UUID of the MIDI characteristic. */

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_midis   MIDI Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_midis_t *p_midis, ble_evt_t const *p_ble_evt) {
    ret_code_t                  err_code;
    ble_midis_evt_t             evt;
    ble_gatts_value_t           gatts_val;
    uint8_t                     cccd_value[2];
    ble_midis_client_context_t *p_client = NULL;

    err_code = blcm_link_ctx_get(p_midis->p_link_ctx_storage, p_ble_evt->evt.gap_evt.conn_handle, (void *)&p_client);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.", p_ble_evt->evt.gap_evt.conn_handle);
    }

    /* Check the hosts CCCD value to inform of readiness to send data using the RX characteristic */
    memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
    gatts_val.p_value = cccd_value;
    gatts_val.len     = sizeof(cccd_value);
    gatts_val.offset  = 0;

    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle, p_midis->midi_char_handles.cccd_handle, &gatts_val);

    if ((err_code == NRF_SUCCESS) && (p_midis->data_handler != NULL) && ble_srv_is_notification_enabled(gatts_val.p_value)) {
        if (p_client != NULL) {
            p_client->is_notification_enabled = true;
        }

        memset(&evt, 0, sizeof(ble_midis_evt_t));
        evt.type        = BLE_MIDI_EVT_NOTIF_STARTED;
        evt.p_midis     = p_midis;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_midis->data_handler(&evt);
    }
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_midis   MIDI Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_midis_t *p_midis, ble_evt_t const *p_ble_evt) {
    NRF_LOG_INFO("Write received.");

    ret_code_t                   err_code;
    ble_midis_evt_t              evt;
    ble_midis_client_context_t * p_client;
    ble_gatts_evt_write_t const *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    err_code = blcm_link_ctx_get(p_midis->p_link_ctx_storage, p_ble_evt->evt.gatts_evt.conn_handle, (void *)&p_client);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.", p_ble_evt->evt.gatts_evt.conn_handle);
    }

    memset(&evt, 0, sizeof(ble_midis_evt_t));
    evt.p_midis     = p_midis;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx  = p_client;

    if ((p_evt_write->handle == p_midis->midi_char_handles.cccd_handle) && (p_evt_write->len == 2)) {
        if (p_client != NULL) {
            NRF_LOG_INFO("cccd write");
            if (ble_srv_is_notification_enabled(p_evt_write->data)) {
                p_client->is_notification_enabled = true;
                evt.type                          = BLE_MIDI_EVT_NOTIF_STARTED;
            } else {
                p_client->is_notification_enabled = false;
                evt.type                          = BLE_MIDI_EVT_NOTIF_STOPPED;
            }

            if (p_midis->data_handler != NULL) {
                p_midis->data_handler(&evt);
            }
        }
    } else if ((p_evt_write->handle == p_midis->midi_char_handles.value_handle) && (p_midis->data_handler != NULL)) {
        NRF_LOG_HEXDUMP_INFO(p_evt_write->data, p_evt_write->len);

        NRF_LOG_DEBUG("Header %x", p_evt_write->data[0]);

        p_midis->data_handler(&evt);
    } else {
        // Do Nothing. This event is not relevant for this service.
    }
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_midis   MIDI Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_hvx_tx_complete(ble_midis_t *p_midis, ble_evt_t const *p_ble_evt) {
    ret_code_t                  err_code;
    ble_midis_evt_t             evt;
    ble_midis_client_context_t *p_client;

    err_code = blcm_link_ctx_get(p_midis->p_link_ctx_storage, p_ble_evt->evt.gatts_evt.conn_handle, (void *)&p_client);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.", p_ble_evt->evt.gatts_evt.conn_handle);
        return;
    }

    if (p_client->is_notification_enabled) {
        memset(&evt, 0, sizeof(ble_midis_evt_t));
        evt.type        = BLE_MIDI_EVT_TX_RDY;
        evt.p_midis     = p_midis;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_midis->data_handler(&evt);
    }
}

void ble_midis_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context) {
    if ((p_context == NULL) || (p_ble_evt == NULL)) {
        return;
    }

    ble_midis_t *p_midis = (ble_midis_t *)p_context;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_midis, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_midis, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_hvx_tx_complete(p_midis, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_midis_init(ble_midis_t *p_midis, ble_midis_init_t const *p_midis_init) {
    ret_code_t            err_code;
    ble_uuid_t            ble_uuid;
    ble_uuid128_t         midis_base_uuid = MIDIS_SERVICE_UUID;
    ble_add_char_params_t add_char_params;

    VERIFY_PARAM_NOT_NULL(p_midis);
    VERIFY_PARAM_NOT_NULL(p_midis_init);

    // Initialize the service structure.
    p_midis->data_handler = p_midis_init->data_handler;
    p_midis->max_data_len = NRF_BLE_MIDI_MAX_DATA_LEN_DEFAULT;

    // Add a vendor specific base UUID for the MIDI Service.
    err_code = sd_ble_uuid_vs_add(&midis_base_uuid, &p_midis->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_midis->uuid_type;
    ble_uuid.uuid = MIDI_SERVICE_16;

    // Add the MIDI Service to the GATT database.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_midis->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add a vendor specific base UUID for the characteristic.
    ble_uuid_t    char_uuid;
    ble_uuid128_t midis_char_uuid = MIDIS_CHAR_UUID;

    err_code = sd_ble_uuid_vs_add(&midis_char_uuid, &char_uuid.type);
    VERIFY_SUCCESS(err_code);

    char_uuid.uuid = MIDI_CHAR_16;

    // Add the MIDI Characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid                     = MIDI_CHAR_16;
    add_char_params.uuid_type                = char_uuid.type;
    add_char_params.max_len                  = BLE_MIDI_MAX_RX_CHAR_LEN;
    add_char_params.p_init_value             = NULL;
    add_char_params.init_len                 = 0;
    add_char_params.is_var_len               = true;
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.write_wo_resp = 1;
    add_char_params.char_props.notify        = 1;
    add_char_params.char_props.read          = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_midis->service_handle, &add_char_params, &p_midis->midi_char_handles);
}

/**@brief function for encoding a header byte.
 *        6 most significant bits of the time (13 bit) is encoded.
 *
 * @param[in] p_timestamp  pointer to header byte.
 * @param[in] timestamp_ms Pointer to the time (ms) that will be encoded into the header byte.
 *
 */
static void header_encode(uint8_t *header, uint32_t timestamp_ms) {
    uint32_t timestamp_13_bit = (timestamp_ms % 0x1FFF);
    *header                   = (0x80 | (timestamp_13_bit >> 7));
}

/**@brief function for encoding a timestamp byte.
 *        7 least significant bits of the time (13 bit) is encoded.
 *
 * @param[in] p_timestamp  pointer to timestamp byte.
 * @param[in] timestamp_ms Pointer to the time (ms) that will be encoded into the timestamp byte.
 *
 */
static void timestamp_encode(uint8_t *p_timestamp, uint32_t *p_timestamp_ms) {
    uint32_t timestamp_13_bit = (*p_timestamp_ms % 0x1FFF);
    *p_timestamp              = (0x80 | (timestamp_13_bit & 0xFF));
}

/**@brief function for checking if the status byte is the common type.
 *
 * @param[in] p_status pointer to a status byte.
 *
 * @retval true  The status is common type.
 * @retval false The status is not common type.
 */
static bool status_is_system_common(uint8_t *p_status) {
    if ((*p_status == MIDI_SYS_CMN_FRAME) || (*p_status == MIDI_SYS_CMN_SONG_POINTER) || (*p_status == MIDI_SYS_CMN_TUNE_REQUEST) || (*p_status == MIDI_SYS_CMN_TUNE_REQUEST)) {
        return true;
    } else {
        return false;
    }
}

/**@brief function for checking if the status byte is the real-time type.
 *
 * @param[in] p_status pointer to a status byte.
 *
 * @retval true  The status is real-time type.
 * @retval false The status is not real-time type.
 */
static bool status_is_system_realtime(uint8_t *p_status) {
    if ((*p_status == MIDI_SYS_RT_TIMING) || (*p_status == MIDI_SYS_RT_START) || (*p_status == MIDI_SYS_RT_CONTINUE) || (*p_status == MIDI_SYS_RT_STOP) || (*p_status == MIDI_SYS_RT_SENSE) || (*p_status == MIDI_SYS_RT_RESET)) {
        return true;
    } else {
        return false;
    }
}

/**@brief function for checking if the status byte is sysex type.
 *
 * @param[in] p_status pointer to a status byte.
 *
 * @retval true  The status is sysex type.
 * @retval false The status is not sysex type.
 */
static bool status_is_system_sysex(uint8_t *p_status) {
    if ((*p_status == MIDI_SYS_SYSEX_START) || (*p_status == MIDI_SYS_SYSEX_END)) {
        return true;
    } else {
        return false;
    }
}

/**@brief function for checking if the status byte is channel type.
 *
 * @param[in] p_status pointer to a status byte.
 *
 * @retval true  The status is channel type.
 * @retval false The status is not channel type.
 */
static bool status_is_channel(uint8_t *p_status) {
    if ((*p_status & 0xF0) == MIDI_CH_NOTE_OFF || (*p_status & 0xF0) == MIDI_CH_NOTE_ON || (*p_status & 0xF0) == MIDI_CH_POLY_AFTER_TOUCH || (*p_status & 0xF0) == MIDI_CH_CTRL_CHANGE || (*p_status & 0xF0) == MIDI_CH_PRGM_CAHNGE || (*p_status & 0xF0) == MIDI_CH_CHANNEL_AFTER_TOUCH || (*p_status & 0xF0) == MIDI_CH_PITCH_WHEEL) {
        return true;
    } else {
        return false;
    }
}

/**@brief function for checking if a byte matches the format of status.
 *
 * @param[in] p_status pointer to a status byte.
 *
 * @retval true  The byte has status format.
 * @retval false The byte doesn't have status format.
 */
static bool byte_is_status(uint8_t *p_byte) {
    if (status_is_channel(p_byte) || status_is_system_sysex(p_byte) || status_is_system_common(p_byte) || status_is_system_realtime(p_byte)) {
        return true;
    }
    return false;
}

/**@brief   Function for notifying the currently encoded MIDI message buffer to the peer.
 *
 * @details This function should be called by ble_midi_msgs_send().
 *
 * @param[in]     p_midis     Pointer to the MIDI Service structure.
 * @param[in]     conn_handle Connection Handle of the destination client.
 *
 * @retval NRF_SUCCESS If the MIDI message buffer was sent to the peer.
 *
 */
uint32_t ble_midi_msg_notify(ble_midis_t *p_midis, uint16_t conn_handle) {
    ret_code_t                  err_code;
    ble_gatts_hvx_params_t      hvx_params;
    ble_midis_client_context_t *p_client;

    VERIFY_PARAM_NOT_NULL(p_midis);

    err_code = blcm_link_ctx_get(p_midis->p_link_ctx_storage, conn_handle, (void *)&p_client);
    VERIFY_SUCCESS(err_code);

    if ((conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL)) {
        return NRF_ERROR_NOT_FOUND;
    }

    if (!p_client->is_notification_enabled) {
        return NRF_ERROR_INVALID_STATE;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    NRF_LOG_HEXDUMP_INFO(p_midis->midi_notif, p_midis->notif_encode_index)

    hvx_params.handle = p_midis->midi_char_handles.value_handle;
    hvx_params.p_data = p_midis->midi_notif;
    hvx_params.p_len  = &p_midis->notif_encode_index;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}

void ble_midi_data_len_set(ble_midis_t *p_midis, uint16_t len) { p_midis->max_data_len = len; }

/**@brief function for calculating the resulting BLE packet size, given an array of MIDI messages.
 *
 * @param[in] msg MIDI message array
 *
 * @retval number of bytes in the notification if created using msg.
 */
static uint16_t ble_midi_packet_size(uint8_array_t msg) {
    uint16_t size = msg.size + BLE_MIDI_HEADER_SIZE;

    for (uint16_t i = 0; i < msg.size; i++) {
        if (byte_is_status(&(msg.p_data[i]))) {
            size++;
        }
    }
    return size;
}

ret_code_t ble_midi_msgs_send(ble_midis_t *p_midis, uint32_t *p_timestamp_ms, uint16_t conn_handle, uint8_array_t msg) {
    uint16_t size = ble_midi_packet_size(msg);

    if (size > p_midis->max_data_len) {
        return NRF_ERROR_INVALID_LENGTH;
    }

    // Make sure previous state is cleared
    p_midis->notif_encode_index = 0;
    memset(p_midis->midi_notif, 0, sizeof(p_midis->midi_notif));

    // Encode header
    header_encode(&p_midis->midi_notif[p_midis->notif_encode_index++], *p_timestamp_ms);

    // traverse buffer, add timestamps where there is status
    for (uint16_t i = 0; i < msg.size; i++) {
        if (byte_is_status(&(msg.p_data[i]))) {
            timestamp_encode(&p_midis->midi_notif[p_midis->notif_encode_index++], p_timestamp_ms);
        }
        p_midis->midi_notif[p_midis->notif_encode_index++] = msg.p_data[i];
    }
    return ble_midi_msg_notify(p_midis, conn_handle);
}
