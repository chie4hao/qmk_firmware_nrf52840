/*
Copyright 2018 Sekigon

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include "adc.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_drv_saadc.h"
#include "nrfx_saadc.h"
#include "ble_service.h"

#define SAMPLES_BUFFER_LEN 4
static nrf_saadc_value_t adc_buffer[2][SAMPLES_BUFFER_LEN];

static void adc_event_handler(nrf_drv_saadc_evt_t const* p_event) {
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {
        ret_code_t        err_code;
        nrf_saadc_value_t value = 0;
        for (int i = 0; i < p_event->data.done.size; i++) {
            value += p_event->data.done.p_buffer[i];
            // NRF_LOG_INFO("%d asdlfkj %d\r\n", i, p_event->data.done.p_buffer[i]);
        }

        battery_level_update(value, p_event->data.done.size);

        err_code = nrfx_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_BUFFER_LEN);

        APP_ERROR_CHECK(err_code);
    }
}

void adc_init() {
    ret_code_t                 err_code;
    nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN3);

    err_code = nrf_drv_saadc_init(NULL, adc_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_saadc_buffer_convert(adc_buffer[0], SAMPLES_BUFFER_LEN);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_saadc_buffer_convert(adc_buffer[1], SAMPLES_BUFFER_LEN);
    APP_ERROR_CHECK(err_code);
}

void adc_start() { nrfx_saadc_sample(); }
