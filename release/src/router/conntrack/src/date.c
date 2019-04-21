/*
 * (C) 2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "date.h"
#include <stdlib.h>
#include <string.h>

static struct timeval now;

int do_gettimeofday(void)
{
	return gettimeofday(&now, NULL);
}

void gettimeofday_cached(struct timeval *tv)
{
	memcpy(tv, &now, sizeof(struct timeval));
}

int time_cached(void)
{
	return now.tv_sec;
}
