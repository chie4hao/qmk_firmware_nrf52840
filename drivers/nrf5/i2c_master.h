#include <stdint.h>

int     i2c_init(void);
void    i2c_start(void);
void    i2c_stop(void);
uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout);
uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length);
