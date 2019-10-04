/**
 * Copyright (c) 2017 - 2019, Nordic Semiconductor ASA
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

#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf5_usb.h"
#include "ble_service.h"
#include "ble.h"

#include "keyboard.h"
#include "eeprom.h"
#ifdef MIDI_ENABLE
#include "qmk_midi.h"
#endif

uint16_t m_conn_handle     = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */
uint8_t  keyboard_protocol = 1;

APP_TIMER_DEF(m_keyboard_scan_timer); /**< keyboard scan timer. */

#ifndef KEYBOARD_SCAN_INTERVAL
#    define KEYBOARD_SCAN_INTERVAL 4
#endif

static void keyboard_scan_handler(void* p_context) {
    UNUSED_PARAMETER(p_context);
    keyboard_task();
    eeprom_update();
}

static void log_init(void) {
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void timers_init(void) {
    ret_code_t err_code;
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

int main(void) {
    keyboard_setup();
    keyboard_init();

    ret_code_t ret;

    log_init();
    timers_init();

    usb_keyboard_init();
    ble_service_init();

    #ifdef MIDI_ENABLE
        setup_midi();
    #endif

    ret = app_timer_create(&m_keyboard_scan_timer, APP_TIMER_MODE_REPEATED, keyboard_scan_handler);
    APP_ERROR_CHECK(ret);
    ret = app_timer_start(m_keyboard_scan_timer, APP_TIMER_TICKS(KEYBOARD_SCAN_INTERVAL), NULL);

    if (!NRF_USBD->ENABLE) {
        advertising_start(false);
    }

    while (true) {
        while (app_usbd_event_queue_process()) {
            /* Nothing to do */
        }
        idle_state_handle();
    }
}
