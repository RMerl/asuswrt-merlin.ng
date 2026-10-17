/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __RAND_H
#define __RAND_H

#define RAND_MAX -1U

/**
 * srand() - Set the random-number seed value
 *
 * This can be used to restart the pseudo-random-number sequence from a known
 * point. This affects future calls to rand() to start from that point
 *
 * @seed: New seed
 */
void srand(unsigned int seed);

/**
 * rand() - Get a 32-bit pseudo-random number
 *
 * Return:	next random number in the sequence
 */
unsigned int rand(void);

/**
 * rand_r() - Get a 32-bit pseudo-random number
 *
 * This version of the function allows multiple sequences to be used at the
 * same time, since it requires the caller to store the seed value.
 *
 * @seedp:	seed value to use, updated on exit
 * Return:	 next random number in the sequence
 */
unsigned int rand_r(unsigned int *seedp);

#endif
