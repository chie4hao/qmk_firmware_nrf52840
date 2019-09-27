/* Copyright 2017 Fred Sundvik
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bootloader.h"
#include "nrf_log.h"
#include "nrf_power.h"
#include "nrf_bootloader_info.h"

void bootloader_jump(void) {
    NRF_LOG_DEBUG("In ble_dfu_buttonless_bootloader_start_finalize\r\n");
    sd_power_gpregret_clr(0, 0xffffffff);
    sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
    sd_nvic_SystemReset();
}
