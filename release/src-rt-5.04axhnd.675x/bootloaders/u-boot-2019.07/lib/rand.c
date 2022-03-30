// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple xorshift PRNG
 *   see http://www.jstatsoft.org/v08/i14/paper
 *
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
 */

#include <common.h>

static unsigned int y = 1U;

unsigned int rand_r(unsigned int *seedp)
{
	*seedp ^= (*seedp << 13);
	*seedp ^= (*seedp >> 17);
	*seedp ^= (*seedp << 5);

	return *seedp;
}

unsigned int rand(void)
{
	return rand_r(&y);
}

void srand(unsigned int seed)
{
	y = seed;
}
