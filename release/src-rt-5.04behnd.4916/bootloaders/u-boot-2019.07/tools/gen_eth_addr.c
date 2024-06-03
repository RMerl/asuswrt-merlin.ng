// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Murray Jensen <Murray.Jensen@cmst.csiro.au>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int
main(int argc, char *argv[])
{
    unsigned long ethaddr_low, ethaddr_high;

    srand(time(0) + (getpid() << 8));

    /*
     * setting the 2nd LSB in the most significant byte of
     * the address makes it a locally administered ethernet
     * address
     */
    ethaddr_high = (rand() & 0xfeff) | 0x0200;
    ethaddr_low = rand();

    printf("%02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n",
	ethaddr_high >> 8, ethaddr_high & 0xff,
	ethaddr_low >> 24, (ethaddr_low >> 16) & 0xff,
	(ethaddr_low >> 8) & 0xff, ethaddr_low & 0xff);

    return (0);
}
