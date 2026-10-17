/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2004 Patrik Kluba,
 *		      University of Szeged, Hungary
 *
 * For licensing information, see the file 'LICENCE' in the
 * jffs2 directory.
 *
 * $Id: compr_lzo.c,v 1.3 2004/06/23 16:34:39 havasi Exp $
 *
 */

/*
   LZO1X-1 (and -999) compression module for jffs2
   based on the original LZO sources
*/

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/*
   Original copyright notice follows:

   lzo1x_9x.c -- implementation of the LZO1X-999 compression algorithm
   lzo_ptr.h -- low-level pointer constructs
   lzo_swd.ch -- sliding window dictionary
   lzoconf.h -- configuration for the LZO real-time data compression library
   lzo_mchw.ch -- matching functions using a window
   minilzo.c -- mini subset of the LZO real-time data compression library
   config1x.h -- configuration for the LZO1X algorithm
   lzo1x.h -- public interface of the LZO1X compression algorithm

   These files are part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
*/

/*

	2004-02-16  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Initial release
					-removed all 16 bit code
					-all sensitive data will be on 4 byte boundary
					-removed check parts for library use
					-removed all but LZO1X-* compression

*/


#include <config.h>
#include <linux/stddef.h>
#include <jffs2/jffs2.h>
#include <jffs2/compr_rubin.h>

/* Integral types that have *exactly* the same number of bits as a lzo_voidp */
typedef unsigned long lzo_ptr_t;
typedef long lzo_sptr_t;

/* data type definitions */
#define U32 unsigned long
#define S32 signed long
#define I32 long
#define U16 unsigned short
#define S16 signed short
#define I16 short
#define U8 unsigned char
#define S8 signed char
#define I8 char

#define M1_MAX_OFFSET	0x0400
#define M2_MAX_OFFSET	0x0800
#define M3_MAX_OFFSET	0x4000
#define M4_MAX_OFFSET	0xbfff

#define __COPY4(dst,src)  * (lzo_uint32p)(dst) = * (const lzo_uint32p)(src)
#define COPY4(dst,src)	__COPY4((lzo_ptr_t)(dst),(lzo_ptr_t)(src))

#define TEST_IP		(ip < ip_end)
#define TEST_OP		(op <= op_end)

#define NEED_IP(x) \
	    if ((lzo_uint)(ip_end - ip) < (lzo_uint)(x))  goto input_overrun
#define NEED_OP(x) \
	    if ((lzo_uint)(op_end - op) < (lzo_uint)(x))  goto output_overrun
#define TEST_LOOKBEHIND(m_pos,out)    if (m_pos < out) goto lookbehind_overrun

typedef U32 lzo_uint32;
typedef I32 lzo_int32;
typedef U32 lzo_uint;
typedef I32 lzo_int;
typedef int lzo_bool;

#define lzo_byte		U8
#define lzo_bytep		U8 *
#define lzo_charp		char *
#define lzo_voidp		void *
#define lzo_shortp		short *
#define lzo_ushortp		unsigned short *
#define lzo_uint32p		lzo_uint32 *
#define lzo_int32p		lzo_int32 *
#define lzo_uintp		lzo_uint *
#define lzo_intp		lzo_int *
#define lzo_voidpp		lzo_voidp *
#define lzo_bytepp		lzo_bytep *
#define lzo_sizeof_dict_t		sizeof(lzo_bytep)

#define LZO_E_OK		    0
#define LZO_E_ERROR		    (-1)
#define LZO_E_OUT_OF_MEMORY	    (-2)	/* not used right now */
#define LZO_E_NOT_COMPRESSIBLE	    (-3)	/* not used right now */
#define LZO_E_INPUT_OVERRUN	    (-4)
#define LZO_E_OUTPUT_OVERRUN	    (-5)
#define LZO_E_LOOKBEHIND_OVERRUN    (-6)
#define LZO_E_EOF_NOT_FOUND	    (-7)
#define LZO_E_INPUT_NOT_CONSUMED    (-8)

#define PTR(a)				((lzo_ptr_t) (a))
#define PTR_LINEAR(a)		PTR(a)
#define PTR_ALIGNED_4(a)	((PTR_LINEAR(a) & 3) == 0)
#define PTR_ALIGNED_8(a)	((PTR_LINEAR(a) & 7) == 0)
#define PTR_ALIGNED2_4(a,b)	(((PTR_LINEAR(a) | PTR_LINEAR(b)) & 3) == 0)
#define PTR_ALIGNED2_8(a,b)	(((PTR_LINEAR(a) | PTR_LINEAR(b)) & 7) == 0)
#define PTR_LT(a,b)			(PTR(a) < PTR(b))
#define PTR_GE(a,b)			(PTR(a) >= PTR(b))
#define PTR_DIFF(a,b)		((lzo_ptrdiff_t) (PTR(a) - PTR(b)))
#define pd(a,b)			((lzo_uint) ((a)-(b)))

typedef ptrdiff_t lzo_ptrdiff_t;

static int
lzo1x_decompress (const lzo_byte * in, lzo_uint in_len,
		  lzo_byte * out, lzo_uintp out_len, lzo_voidp wrkmem)
{
	register lzo_byte *op;
	register const lzo_byte *ip;
	register lzo_uint t;

	register const lzo_byte *m_pos;

	const lzo_byte *const ip_end = in + in_len;
	lzo_byte *const op_end = out + *out_len;

	*out_len = 0;

	op = out;
	ip = in;

	if (*ip > 17)
	{
		t = *ip++ - 17;
		if (t < 4)
			goto match_next;
		NEED_OP (t);
		NEED_IP (t + 1);
		do
			*op++ = *ip++;
		while (--t > 0);
		goto first_literal_run;
	}

	while (TEST_IP && TEST_OP)
	{
		t = *ip++;
		if (t >= 16)
			goto match;
		if (t == 0)
		{
			NEED_IP (1);
			while (*ip == 0)
			{
				t += 255;
				ip++;
				NEED_IP (1);
			}
			t += 15 + *ip++;
		}
		NEED_OP (t + 3);
		NEED_IP (t + 4);
		if (PTR_ALIGNED2_4 (op, ip))
		{
			COPY4 (op, ip);

			op += 4;
			ip += 4;
			if (--t > 0)
			{
				if (t >= 4)
				{
					do
					{
						COPY4 (op, ip);
						op += 4;
						ip += 4;
						t -= 4;
					}
					while (t >= 4);
					if (t > 0)
						do
							*op++ = *ip++;
						while (--t > 0);
				}
				else
					do
						*op++ = *ip++;
					while (--t > 0);
			}
		}
		else
		{
			*op++ = *ip++;
			*op++ = *ip++;
			*op++ = *ip++;
			do
				*op++ = *ip++;
			while (--t > 0);
		}
	      first_literal_run:

		t = *ip++;
		if (t >= 16)
			goto match;

		m_pos = op - (1 + M2_MAX_OFFSET);
		m_pos -= t >> 2;
		m_pos -= *ip++ << 2;
		TEST_LOOKBEHIND (m_pos, out);
		NEED_OP (3);
		*op++ = *m_pos++;
		*op++ = *m_pos++;
		*op++ = *m_pos;

		goto match_done;

		while (TEST_IP && TEST_OP)
		{
		      match:
			if (t >= 64)
			{
				m_pos = op - 1;
				m_pos -= (t >> 2) & 7;
				m_pos -= *ip++ << 3;
				t = (t >> 5) - 1;
				TEST_LOOKBEHIND (m_pos, out);
				NEED_OP (t + 3 - 1);
				goto copy_match;

			}
			else if (t >= 32)
			{
				t &= 31;
				if (t == 0)
				{
					NEED_IP (1);
					while (*ip == 0)
					{
						t += 255;
						ip++;
						NEED_IP (1);
					}
					t += 31 + *ip++;
				}

				m_pos = op - 1;
				m_pos -= (ip[0] >> 2) + (ip[1] << 6);

				ip += 2;
			}
			else if (t >= 16)
			{
				m_pos = op;
				m_pos -= (t & 8) << 11;

				t &= 7;
				if (t == 0)
				{
					NEED_IP (1);
					while (*ip == 0)
					{
						t += 255;
						ip++;
						NEED_IP (1);
					}
					t += 7 + *ip++;
				}

				m_pos -= (ip[0] >> 2) + (ip[1] << 6);

				ip += 2;
				if (m_pos == op)
					goto eof_found;
				m_pos -= 0x4000;
			}
			else
			{

				m_pos = op - 1;
				m_pos -= t >> 2;
				m_pos -= *ip++ << 2;
				TEST_LOOKBEHIND (m_pos, out);
				NEED_OP (2);
				*op++ = *m_pos++;
				*op++ = *m_pos;

				goto match_done;
			}

			TEST_LOOKBEHIND (m_pos, out);
			NEED_OP (t + 3 - 1);
			if (t >= 2 * 4 - (3 - 1)
			    && PTR_ALIGNED2_4 (op, m_pos))
			{
				COPY4 (op, m_pos);
				op += 4;
				m_pos += 4;
				t -= 4 - (3 - 1);
				do
				{
					COPY4 (op, m_pos);
					op += 4;
					m_pos += 4;
					t -= 4;
				}
				while (t >= 4);
				if (t > 0)
					do
						*op++ = *m_pos++;
					while (--t > 0);
			}
			else

			{
			      copy_match:
				*op++ = *m_pos++;
				*op++ = *m_pos++;
				do
					*op++ = *m_pos++;
				while (--t > 0);
			}

		      match_done:
			t = ip[-2] & 3;

			if (t == 0)
				break;

		      match_next:
			NEED_OP (t);
			NEED_IP (t + 1);
			do
				*op++ = *ip++;
			while (--t > 0);
			t = *ip++;
		}
	}
	*out_len = op - out;
	return LZO_E_EOF_NOT_FOUND;

      eof_found:
	*out_len = op - out;
	return (ip == ip_end ? LZO_E_OK :
		(ip <
		 ip_end ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN));

      input_overrun:
	*out_len = op - out;
	return LZO_E_INPUT_OVERRUN;

      output_overrun:
	*out_len = op - out;
	return LZO_E_OUTPUT_OVERRUN;

      lookbehind_overrun:
	*out_len = op - out;
	return LZO_E_LOOKBEHIND_OVERRUN;
}

int lzo_decompress(unsigned char *data_in, unsigned char *cpage_out,
		      u32 srclen, u32 destlen)
{
	lzo_uint outlen = destlen;
	return lzo1x_decompress (data_in, srclen, cpage_out, &outlen, NULL);
}
