/*
 * i2c.c
 *
 *  Created on: Aug 22, 2021
 *      Author: zychosen
 */
#include "i2c.h"

void i2c_setup(uint8_t address) {
	P3SEL |= BIT0 | BIT1;                    /* select i2c mode on Port 3 */

	/* configure i2c control register */
	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 = (UCMST | UCMODE_3 | UCSYNC);   /* I2C master mode */
	UCB0CTL1 = UCSSEL_2;                      /* 1MHz SMCLK */
	UCB0BR0 = 12;                             /* I2C bus speed ~83KHz */
	UCB0BR1 = 0;
	UCB0I2CSA = address;                      /* Set slave address */
	UCB0CTL1 &= ~UCSWRST;
}

inline void i2c_start(void) {
	while(UCB0CTL1 & UCTXSTP);        /* While stop condition is asserted don't start */
	UCB0CTL1 |= (UCTR | UCTXSTT);     /* configure as transmitter (UCTR = 1) */
}

inline void i2c_stop(void) {
	while(!(UCB0IFG & UCTXIFG));      /* Loop until TX interrupt is pending (ready for new data) */
	while(UCB0CTL1 & UCTXSTT);        /* Loop until start condition is done */
	UCB0CTL1 |= UCTXSTP;              /* Generate stop condition */
}

inline void i2c_transmit_multiple_bytes(uint8_t *byte, uint16_t byteCtr) {
	while(!(UCB0IFG & UCTXIFG));         /* Loop until data is ready to be transmitted */
	while (byteCtr--) {                  /* Check if required number of bytes are transmitted */
		while(!(UCB0IFG & UCTXIFG));
		UCB0TXBUF = *byte++;             /* Load TX buffer with byte to transmit and increment byte pointer */
	}
}

inline void i2c_transmit_byte(uint8_t byte) {
	while(!(UCB0IFG & UCTXIFG));
	UCB0TXBUF = byte;
}
