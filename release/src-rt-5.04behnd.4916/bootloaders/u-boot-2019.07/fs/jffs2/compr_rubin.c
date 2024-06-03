/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001 Red Hat, Inc.
 *
 * Created by Arjan van de Ven <arjanv@redhat.com>
 *
 * Heavily modified by Russ Dill <Russ.Dill@asu.edu> in an attempt at
 * a little more speed.
 *
 * The original JFFS, from which the design for JFFS2 was derived,
 * was designed and implemented by Axis Communications AB.
 *
 * The contents of this file are subject to the Red Hat eCos Public
 * License Version 1.1 (the "Licence"); you may not use this file
 * except in compliance with the Licence.  You may obtain a copy of
 * the Licence at http://www.redhat.com/
 *
 * Software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing rights and
 * limitations under the Licence.
 *
 * The Original Code is JFFS2 - Journalling Flash File System, version 2
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License version 2 (the "GPL"), in
 * which case the provisions of the GPL are applicable instead of the
 * above.  If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the RHEPL, indicate your decision by
 * deleting the provisions above and replace them with the notice and
 * other provisions required by the GPL.  If you do not delete the
 * provisions above, a recipient may use your version of this file
 * under either the RHEPL or the GPL.
 *
 * $Id: compr_rubin.c,v 1.2 2002/01/24 22:58:42 rfeany Exp $
 *
 */

#include <config.h>
#include <jffs2/jffs2.h>
#include <jffs2/compr_rubin.h>


void rubin_do_decompress(unsigned char *bits, unsigned char *in,
			 unsigned char *page_out, __u32 destlen)
{
	register char *curr = (char *)page_out;
	char *end = (char *)(page_out + destlen);
	register unsigned long temp;
	register unsigned long result;
	register unsigned long p;
	register unsigned long q;
	register unsigned long rec_q;
	register unsigned long bit;
	register long i0;
	unsigned long i;

	/* init_pushpull */
	temp = *(u32 *) in;
	bit = 16;

	/* init_rubin */
	q = 0;
	p = (long) (2 * UPPER_BIT_RUBIN);

	/* init_decode */
	rec_q = (in[0] << 8) | in[1];

	while (curr < end) {
		/* in byte */

		result = 0;
		for (i = 0; i < 8; i++) {
			/* decode */

			while ((q & UPPER_BIT_RUBIN) || ((p + q) <= UPPER_BIT_RUBIN)) {
				q &= ~UPPER_BIT_RUBIN;
				q <<= 1;
				p <<= 1;
				rec_q &= ~UPPER_BIT_RUBIN;
				rec_q <<= 1;
				rec_q |= (temp >> (bit++ ^ 7)) & 1;
				if (bit > 31) {
					u32 *p = (u32 *)in;
					bit = 0;
					temp = *(++p);
					in = (unsigned char *)p;
				}
			}
			i0 =  (bits[i] * p) >> 8;

			if (i0 <= 0) i0 = 1;
			/* if it fails, it fails, we have our crc
			if (i0 >= p) i0 = p - 1; */

			result >>= 1;
			if (rec_q < q + i0) {
				/* result |= 0x00; */
				p = i0;
			} else {
				result |= 0x80;
				p -= i0;
				q += i0;
			}
		}
		*(curr++) = result;
	}
}

void dynrubin_decompress(unsigned char *data_in, unsigned char *cpage_out,
		   unsigned long sourcelen, unsigned long dstlen)
{
	unsigned char bits[8];
	int c;

	for (c=0; c<8; c++)
		bits[c] = (256 - data_in[c]);

	rubin_do_decompress(bits, data_in+8, cpage_out, dstlen);
}
