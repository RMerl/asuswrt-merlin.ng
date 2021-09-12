/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2020 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Helpers around bitmaps */

#include "lldpd.h"

/*
 * Set vlan id in the bitmap
 */
void
bitmap_set(uint32_t *bmap, uint16_t vlan_id)
{
	if (vlan_id < MAX_VLAN)
		bmap[vlan_id / 32] |= (((uint32_t) 1) << (vlan_id % 32));
}

/*
 * Checks if the bitmap is empty
 */
int
bitmap_isempty(uint32_t *bmap)
{
	int i;

	for (i = 0; i < VLAN_BITMAP_LEN; i++) {
		if (bmap[i] != 0)
			return 0;
	}

	return 1;
}

/*
 * Calculate the number of bits set in the bitmap to get total
 * number of VLANs
 */
unsigned int
bitmap_numbits(uint32_t *bmap)
{
	unsigned int num = 0;

	for (int i = 0; (i < VLAN_BITMAP_LEN); i++) {
		uint32_t v = bmap[i];
		v = v - ((v >> 1) & 0x55555555);
		v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
		num += (((v + (v >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
	}

	return num;
}
