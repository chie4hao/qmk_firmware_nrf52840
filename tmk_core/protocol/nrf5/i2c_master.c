#include <string.h>

#include "i2c_master.h"
#include "nrfx_twim.h"
#include "nrf_log.h"
#include "nrf_pwr_mgmt.h"

#ifndef CONFIG_PIN_SCL
#    define CONFIG_PIN_SCL 27
#endif
#ifndef CONFIG_PIN_SDA
#    define CONFIG_PIN_SDA 26
#endif
#ifndef CONFIG_I2C_FREQ
#    define CONFIG_I2C_FREQ NRF_TWIM_FREQ_400K
#endif

static const nrfx_twim_t m_twim_master = NRFX_TWIM_INSTANCE(0);
static volatile int      twim_complete = 0;

void twim_evt_handler(nrfx_twim_evt_t const* p_event, void* p_context) { twim_complete = p_event->type + 1; }

int i2c_init(void) {
    const nrfx_twim_config_t config = {.scl = CONFIG_PIN_SCL, .sda = CONFIG_PIN_SDA, .frequency = CONFIG_I2C_FREQ, .interrupt_priority = 5, .hold_bus_uninit = true};
    int                      res    = (int)nrfx_twim_init(&m_twim_master, &config, twim_evt_handler, NULL);
    nrfx_twim_enable(&m_twim_master);
    return res;
}

void i2c_uninit(void) { nrfx_twim_uninit(&m_twim_master); }

uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout) {
    nrfx_err_t res;
    twim_complete = 0;

    res = nrfx_twim_tx(&m_twim_master, address >> 1, data, length, false);
    if (res != NRFX_SUCCESS) {
        NRF_LOG_INFO("Twim error:%d", res);
    }

    int cnt = 0;
    if (res == NRFX_SUCCESS) {
        while (twim_complete == 0) {
            nrf_pwr_mgmt_run();
            if (++cnt == 100) break;
        }
    }
    if (cnt == 100) {
        NRF_LOG_INFO("Twim tx timeout");
    }
    if (twim_complete != 1) {
        NRF_LOG_INFO("twim_uncomplete:%d %d %d", length, data[0], data[1]);
    }
    return twim_complete == 1 ? 0 : 1;
}

uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length) {
    nrfx_err_t res;
    twim_complete = 0;
    res           = nrfx_twim_rx(&m_twim_master, address >> 1, data, length);
    int cnt       = 0;
    if (res == NRFX_SUCCESS) {
        while (twim_complete == 0) {
            nrf_pwr_mgmt_run();
            if (cnt++ == 700) break;
        }
    }
    if (cnt >= 700) NRF_LOG_INFO("Twim rx timeout");
    return twim_complete == 1 ? 0 : 1;
}

uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout) {
    i2c_transmit(devaddr, &regaddr, 1, 0);
    return i2c_receive(devaddr, data, length);
}

uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout) {
    static uint8_t buffer[256];
    buffer[0] = regaddr;
    memcpy(&buffer[1], data, length);
    return i2c_transmit(devaddr, buffer, length + 1, 0);
    return 0;
}
