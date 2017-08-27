/*
    Copyright (c)  2006, 2007		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  GPL v2 or any later

    See COPYING for the status of this software.
*/

#include <stdlib.h>
#include <unistd.h>

#include "traceroute.h"


uint16_t in_csum (const void *ptr, size_t len) {
	const uint16_t *p = (const uint16_t *) ptr;
	size_t nw = len / 2;
	unsigned int sum = 0;
	uint16_t res;

	while (nw--)  sum += *p++;

	if (len & 0x1)
	    sum += htons (*((unsigned char *) p) << 8);

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	res = ~sum;
	if (!res)  res = ~0;

	return res;
}

