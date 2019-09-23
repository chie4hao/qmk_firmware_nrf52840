#include <string.h>

#include "i2c_master.h"
#include "nrfx_twim.h"
#include "nrf_log.h"
#include "nrf_pwr_mgmt.h"

#ifndef I2C1_SCL
#    define I2C1_SCL 27
#endif
#ifndef I2C1_SDA
#    define I2C1_SDA 26
#endif
#ifndef CONFIG_I2C_FREQ
#    define CONFIG_I2C_FREQ NRF_TWIM_FREQ_400K
#endif

static const nrfx_twim_t m_twim_master = NRFX_TWIM_INSTANCE(0);
static volatile int      twim_complete = 0;
static volatile bool     twim_enable   = false;
static uint8_t twi_transfer_buffer[10];

void twim_evt_handler(nrfx_twim_evt_t const* p_event, void* p_context) { twim_complete = p_event->type + 1; }

int i2c_init(void) {
    const nrfx_twim_config_t config = {.scl = I2C1_SCL, .sda = I2C1_SDA, .frequency = CONFIG_I2C_FREQ, .interrupt_priority = 5, .hold_bus_uninit = true};
    int                      res    = (int)nrfx_twim_init(&m_twim_master, &config, twim_evt_handler, NULL);
    // nrfx_twim_enable(&m_twim_master);
    i2c_start();
    return res;
}

static void i2c_write_register(uint8_t addr, uint8_t reg, uint8_t data) {
    twi_transfer_buffer[0] = reg;
    twi_transfer_buffer[1] = data;
    i2c_transmit(addr << 1, twi_transfer_buffer, 2, 0);
}

void i2c_start(void) {
    nrfx_twim_enable(&m_twim_master);
    twim_enable = true;
#ifdef IS31FL3737
    i2c_write_register(DRIVER_ADDR_1, 0xFE, 0xC5);
    i2c_write_register(DRIVER_ADDR_1, 0xFD, 0x03);
    i2c_write_register(DRIVER_ADDR_1, 0x00, 0x01);
#endif
}

void i2c_stop(void) {
#ifdef IS31FL3737
    i2c_write_register(DRIVER_ADDR_1, 0xFE, 0xC5);
    i2c_write_register(DRIVER_ADDR_1, 0xFD, 0x03);
    i2c_write_register(DRIVER_ADDR_1, 0x00, 0x00);
#endif
    twim_enable = false;
    nrfx_twim_disable(&m_twim_master);
}

uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout) {
    if (!twim_enable) {
        return 0;
    }
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
    if (!twim_enable) {
        return 0;
    }
    nrfx_err_t res;
    twim_complete = 0;
    res           = nrfx_twim_rx(&m_twim_master, address >> 1, data, length);
    int cnt       = 0;
    if (res == NRFX_SUCCESS) {
        while (twim_complete == 0) {
            nrf_pwr_mgmt_run();
            if (cnt++ == 100) break;
        }
    }
    if (cnt >= 100) NRF_LOG_INFO("Twim rx timeout");
    return twim_complete == 1 ? 0 : 1;
}
