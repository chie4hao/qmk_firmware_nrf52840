#include <stdint.h>

int     i2c_init(void);
void    i2c_uninit(void);
uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout);
uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length);
uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout);
uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout);

int  i2cs_init(void);
void i2cs_uninit(void);
void i2cs_prepare(uint8_t* const dat, uint8_t len);
