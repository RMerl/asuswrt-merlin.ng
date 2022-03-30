// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Stephen Warren
 */

#include <config.h>
#include <phys2bus.h>

unsigned long phys_to_bus(unsigned long phys)
{
#ifndef CONFIG_BCM2835
	return 0xc0000000 | phys;
#else
	return 0x40000000 | phys;
#endif
}

unsigned long bus_to_phys(unsigned long bus)
{
	return bus & ~0xc0000000;
}
