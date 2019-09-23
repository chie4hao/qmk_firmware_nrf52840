
#include "host_driver.h"
#include "nrf_drv_saadc.h"

host_driver_t *nrf5_ble_driver(void);

/**@brief Function for starting advertising.
 */
void advertising_start(bool erase_bonds);
void advertising_without_whitelist(void);
void restart_advertising_id(uint8_t id);
void idle_state_handle(void);
void ble_service_init(void);
void deep_sleep_mode_enter(void);
void ble_disconnect(void);
void reset_power_save_counter(void);
void battery_level_update(nrf_saadc_value_t value, uint16_t size);
// void send_midis(void);
