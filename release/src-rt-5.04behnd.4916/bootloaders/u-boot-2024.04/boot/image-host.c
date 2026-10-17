// SPDX-License-Identifier: GPL-2.0+
/*
 * Image code used by host tools (and not boards)
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <time.h>

void memmove_wd(void *to, void *from, size_t len, ulong chunksz)
{
	memmove(to, from, len);
}

void genimg_print_size(uint32_t size)
{
	printf("%d Bytes = %.2f KiB = %.2f MiB\n", size, (double)size / 1.024e3,
	       (double)size / 1.048576e6);
}

void genimg_print_time(time_t timestamp)
{
	printf("%s", ctime(&timestamp));
}
