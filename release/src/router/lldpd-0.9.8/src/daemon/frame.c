/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2009 Vincent Bernat <bernat@luffy.cx>
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

#include "lldpd.h"

/**
 * Compute the checksum as 16-bit word.
 */
u_int16_t
frame_checksum(const u_char *cp, int len, int cisco)
{
	unsigned int sum = 0, v = 0;
	int oddbyte = 0;

	while ((len -= 2) >= 0) {
		sum += *cp++ << 8;
		sum += *cp++;
	}
	if ((oddbyte = len & 1) != 0)
		v = *cp;

	/* The remaining byte seems to be handled oddly by Cisco. From function
	 * dissect_cdp() in wireshark. 2014/6/14,zhengy@yealink.com:
	 *
	 * CDP doesn't adhere to RFC 1071 section 2. (B). It incorrectly assumes
	 * checksums are calculated on a big endian platform, therefore i.s.o.
	 * padding odd sized data with a zero byte _at the end_ it sets the last
	 * big endian _word_ to contain the last network _octet_. This byteswap
	 * has to be done on the last octet of network data before feeding it to
	 * the Internet checksum routine.
	 * CDP checksumming code has a bug in the addition of this last _word_
	 * as a signed number into the long word intermediate checksum. When
	 * reducing this long to word size checksum an off-by-one error can be
	 * made. This off-by-one error is compensated for in the last _word_ of
	 * the network data.
	 */
	if (oddbyte) {
		if (cisco) {
			if (v & 0x80) {
				sum += 0xff << 8;
				sum += v - 1;
			} else {
				sum += v;
			}
		} else {
			sum += v << 8;
		}
	}

      	sum = (sum >> 16) + (sum & 0xffff);
      	sum += sum >> 16;
      	return (0xffff & ~sum);
}
