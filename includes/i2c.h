/*
 * i2c.h
 *
 *  Created on: Aug 22, 2021
 *      Author: zychosen
 */

#ifndef I2C_H_
#define I2C_H_

#include <msp430f5529.h>
#include <stdint.h>

void i2c_setup(uint8_t);
inline void i2c_transmit_multiple_bytes(uint8_t *, uint16_t);
inline void i2c_transmit_byte(uint8_t);
inline void i2c_start(void);
inline void i2c_stop(void);

#endif /* I2C_H_ */
