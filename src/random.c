/*
 * random.c
 *
 *  Created on: Sep 11, 2021
 *      Author: zychosen
 */

#include "random.h"

unsigned int seed = 0;

inline void Seed(unsigned int s) {
	seed = s;
}

inline unsigned int random(void) {
	seed = (seed*a + c) % m;
	return seed;
}




