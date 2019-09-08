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
/**@file
 *
 * @defgroup ble_midis MIDI Service
 * @{
 * @ingroup  ble_sdk_srv
 * @brief    MIDI Service implementation.
 *
 * @details
 *
 * Initializing the ble_midi_service
 *   call ble_midis_init() before advertisig.
 *   Wait for a peer to connect and to enable services (BLE_MIDI_EVT_NOTIF_STARTED).
 *
 * Sending a BLE packet of MIDI messages.
 *   ble_midi_msgs_send() takes a buffer of MIDI messages. Timestamps will be prepended to status bytes.
 *   All MIDI BLE packets must being with a header byte. This gets prepended to the first byte of the
 *   buffer of MIDI messages.
 *
 *  For example, a buffer provided like this
 *                             {status, data, data, status, data, data, }
 *  Would be encoded like this
 *                             {HEADER, TIMESTAMP, status, data,data, TIMESTAMP, status, data, data}
 *
 *  Protocol is based on
 *   "Specification for MIDI over Bluetooth Low Energy (BLE-MIDI) available at http://www.midi.org".
 *
 * @note    The application must register this module as BLE event observer using the
 *          NRF_SDH_BLE_OBSERVER macro. Example:
 *          @code
 *              ble_midis_t instance;
 *              NRF_SDH_BLE_OBSERVER(anything, BLE_MIDI_BLE_OBSERVER_PRIO,
 *                                   ble_midis_on_ble_evt, &instance);
 *          @endcode
 */
#ifndef BLE_MIDI_H__
#    define BLE_MIDI_H__

#    include <stdint.h>
#    include <stdbool.h>
#    include "sdk_config.h"
#    include "ble.h"
#    include "ble_srv_common.h"
#    include "nrf_sdh_ble.h"
#    include "ble_link_ctx_manager.h"

#    ifdef __cplusplus
extern "C" {
#    endif

/**@brief   Macro for defining a ble_midis instance.
 *
 * @param     _name              Name of the instance.
 * @param[in] _midis_max_clients Maximum number of MIDI clients connected at a time.
 * @hideinitializer
 */

#    define BLE_MIDI_BLE_OBSERVER_PRIO 2

#    define BLE_MIDI_DEF(_name, _midis_max_clients)                                                                             \
        BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage), (_midis_max_clients), sizeof(ble_midis_client_context_t)); \
        static ble_midis_t _name = {.p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage)};                                 \
        NRF_SDH_BLE_OBSERVER(_name##_obs, BLE_MIDI_BLE_OBSERVER_PRIO, ble_midis_on_ble_evt, &_name)

#    define BLE_UUID_MIDI_SERVICE 0x0E5A

#    define OPCODE_LENGTH 1
#    define HANDLE_LENGTH 2

#    define NRF_BLE_MIDI_MAX_DATA_LEN_DEFAULT BLE_GATT_ATT_MTU_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH

/* Channel messages.
   4 most significant bytes is the type, 4 least significant is the channel.
   Example: 0x83 is note off channel 4*/
#    define MIDI_CH_NOTE_OFF 0x80
#    define MIDI_CH_NOTE_ON 0x90
#    define MIDI_CH_POLY_AFTER_TOUCH 0xA0
#    define MIDI_CH_CTRL_CHANGE 0xB0
#    define MIDI_CH_PRGM_CAHNGE 0xC0
#    define MIDI_CH_CHANNEL_AFTER_TOUCH 0xD0
#    define MIDI_CH_PITCH_WHEEL 0xE0

/* System Exclusive. */
#    define MIDI_SYS_SYSEX_START 0xF0
#    define MIDI_SYS_SYSEX_END 0xF7

/* System Common. */
#    define MIDI_SYS_CMN_FRAME 0xF1
#    define MIDI_SYS_CMN_SONG_POINTER 0xF2
#    define MIDI_SYS_CMN_SONG_SELECT 0xF3
#    define MIDI_SYS_CMN_TUNE_REQUEST 0xF6

/* System Real-Time. */
#    define MIDI_SYS_RT_TIMING 0xF8
#    define MIDI_SYS_RT_START 0xFA
#    define MIDI_SYS_RT_CONTINUE 0xFB
#    define MIDI_SYS_RT_STOP 0xFC
#    define MIDI_SYS_RT_SENSE 0xFE
#    define MIDI_SYS_RT_RESET 0xFF

/* Midi channels
   4 most significant bytes is the type, 4 least significant is the channel.
   Example: 0x83 is note off channel 4 */

#    define MIDI_CHANNEL_1 0x00
#    define MIDI_CHANNEL_2 0x01
#    define MIDI_CHANNEL_3 0x02
#    define MIDI_CHANNEL_4 0x03
#    define MIDI_CHANNEL_5 0x04
#    define MIDI_CHANNEL_6 0x05
#    define MIDI_CHANNEL_7 0x06
#    define MIDI_CHANNEL_8 0x07
#    define MIDI_CHANNEL_9 0x08
#    define MIDI_CHANNEL_10 0x09
#    define MIDI_CHANNEL_11 0x0A
#    define MIDI_CHANNEL_12 0x0B
#    define MIDI_CHANNEL_13 0x0C
#    define MIDI_CHANNEL_14 0x0D
#    define MIDI_CHANNEL_15 0x0E
#    define MIDI_CHANNEL_16 0x0F

#    define MIDI_CHANNEL_MIN MIDI_CHANNEL_1
#    define MIDI_CHANNEL_MAX MIDI_CHANNEL_16

#    define MIDI_VELOCITY_MAX 0x7F
#    define MIDI_NOTE_MAX 0x7F
#    define MIDI_DATA_MAX 0x7F

/*Example note values as defines. Common to have Middle C as 0x3C. 128 Notes in total. */
#    define MIDI_NOTE_C8 0x78
#    define MIDI_NOTE_C7 0x6C
#    define MIDI_NOTE_C6 0x60
#    define MIDI_NOTE_C5 0x54
#    define MIDI_NOTE_C4 0x48
#    define MIDI_NOTE_C3 0x3C
#    define MIDI_NOTE_C2 0x30
#    define MIDI_NOTE_C1 0x24
#    define MIDI_NOTE_C0 0x18
#    define MIDI_NOTE_C_MINUS_1 0x0C
#    define MIDI_NITE_C_MINUS_2 0x00

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the MIDI Service module. */
#    if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
#        define BLE_MIDI_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#    else
#        define BLE_MIDI_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
#        warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#    endif

/**@brief   MIDI Service event types. */
typedef enum {
    BLE_MIDI_EVT_RX_DATA,       /**< Data received. */
    BLE_MIDI_EVT_TX_RDY,        /**< Service is ready to accept new data to be transmitted. */
    BLE_MIDI_EVT_NOTIF_STARTED, /**< Notification has been enabled. */
    BLE_MIDI_EVT_NOTIF_STOPPED  /**< Notification has been disabled. */
} ble_midis_evt_type_t;

/* Forward declaration of the ble_midis_t type. */
typedef struct ble_midis_s ble_midis_t;

/**@brief   MIDI Service @ref BLE_MIDI_EVT_RX_DATA event data.
 *
 * @details This structure is passed to an event when @ref BLE_MIDI_EVT_RX_DATA occurs.
 */
typedef struct {
    uint8_t const* p_data; /**< A pointer to the buffer with received data. */
    uint16_t       length; /**< Length of received data. */
} ble_midis_evt_rx_data_t;

/**@brief MIDI Service client context structure.
 *
 * @details This structure contains state context related to hosts.
 */
typedef struct {
    bool is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
} ble_midis_client_context_t;

/**@brief   MIDI Service event structure.
 *
 * @details This structure is passed to an event coming from service.
 */
typedef struct {
    uint8_t header;
    uint8_t timestamp;

    ble_midis_evt_type_t        type;        /**< Event type. */
    ble_midis_t*                p_midis;     /**< A pointer to the instance. */
    uint16_t                    conn_handle; /**< Connection handle. */
    ble_midis_client_context_t* p_link_ctx;  /**< A pointer to the link context. */
    union {
        ble_midis_evt_rx_data_t rx_data; /**< @ref BLE_MIDI_EVT_RX_DATA event data. */
    } params;

} ble_midis_evt_t;

/**@brief MIDI Service event handler type. */
typedef void (*ble_midis_data_handler_t)(ble_midis_evt_t* p_evt);

/**@brief   MIDI Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * must fill this structure and pass it to the service using the @ref ble_midi_init
 *          function.
 */
typedef struct {
    ble_midis_data_handler_t data_handler;      /**< Event handler to be called for handling received data. */
    bool                     make_data_running; /*format data to be running even if the message supplied contains a status byte. (shed the status byte if possible). Timestamps will also be ignored unless mandatory. */
} ble_midis_init_t;

/**@brief   MIDI Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_midis_s {
    uint8_t                        uuid_type;          /**< UUID type for MIDI Service Base UUID. */
    uint16_t                       service_handle;     /**< Handle of MIDI Service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t       midi_char_handles;  /**< Handles related to the RX characteristic (as provided by the SoftDevice). */
    blcm_link_ctx_storage_t* const p_link_ctx_storage; /**< Pointer to link context storage with handles of all current connections and its context. */
    ble_midis_data_handler_t       data_handler;       /**< Event handler to be called for handling received data. */

    uint16_t max_data_len;                              /**< Max length of notifications. See ble_midi_data_len_set() */
    uint8_t  midi_notif[NRF_SDH_BLE_GATT_MAX_MTU_SIZE]; /**< buffer to store encoded MIDI messages that will be notified to a peer. */
    uint16_t notif_encode_index;                        /**< Index to help keep track of data as the MIDI messages get traversed and added to the midi_notif buffer.*/
};

/**@brief   Function for initializing the MIDI Service.
 *
 * @param[in] p_midis      Pointer to the MIDI Service structure.
 * @param[in] p_midis_init Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was successfully initialized. Otherwise, an error code is returned.
 * @retval NRF_ERROR_NULL If either of the pointers p_midis or p_midis_init is NULL.
 */
uint32_t ble_midis_init(ble_midis_t* p_midis, ble_midis_init_t const* p_midis_init);

/**@brief   Function for changing the max length of notications
 *
 * @details This function is meant to be when the gatt handling library reports updated data length
 *          for the connection (NRF_BLE_GATT_EVT_ATT_MTU_UPDATED). If this function is never called
 *          data size remains NRF_BLE_MIDI_MAX_DATA_LEN_DEFAULT.
 *
 * @param[in] p_midis      Pointer to the MIDI Service structure.
 * @param[in] p_midis_init maximum data length.
 *
 */
void ble_midi_data_len_set(ble_midis_t* p_midis, uint16_t len);

/**@brief   Function for handling the MIDI Service's BLE events.
 *
 * @details The MIDI Service expects the application to call this function each time an
 * event is received from the SoftDevice. This function processes the event if it
 * is relevant and calls the MIDI Service event handler of the
 * application if necessary.
 *
 * @param[in] p_ble_evt     Event received from the SoftDevice.
 * @param[in] p_context     MIDI Service structure.
 */
void ble_midis_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_context);

/**@brief   Function for initializing the MIDI Service.
 *
 * @param[in] p_midis      Pointer to the MIDI Service structure.
 * @param[in] timestamp_ms timestamp in 32 bit. converted to 13 bit.
 *                         6 most significant bits become the header byte timestamp.
 *                         7 least significant bits of the time is encoded into timestamp bytes.
 * @param[in] conn_handle  Connection Handle of the destination client.
 * @param[in] msg          Buffer of MIDI messages to be sent in a BLE MIDI packet.
 *
 */
ret_code_t ble_midi_msgs_send(ble_midis_t* p_midis, uint32_t* p_timestamp_ms, uint16_t conn_handle, uint8_array_t msg);
#    ifdef __cplusplus
}
#    endif

#endif  // BLE_MIDI_H__

/** @} */
