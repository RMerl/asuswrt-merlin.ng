/*
 *  LZO1X Decompressor from MiniLZO
 *
 *  Copyright (C) 1996-2005 Markus F.X.J. Oberhumer <markus@oberhumer.com>
 *
 *  The full LZO package can be found at:
 *  http://www.oberhumer.com/opensource/lzo/
 *
 *  Changed for kernel use by:
 *  Nitin Gupta <nitingupta910@gmail.com>
 *  Richard Purdie <rpurdie@openedhand.com>
 */

#include <common.h>
#include <linux/lzo.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include "lzodefs.h"

#define HAVE_IP(x, ip_end, ip) ((size_t)(ip_end - ip) < (x))
#define HAVE_OP(x, op_end, op) ((size_t)(op_end - op) < (x))
#define HAVE_LB(m_pos, out, op) (m_pos < out || m_pos >= op)

#define COPY4(dst, src)	\
		put_unaligned(get_unaligned((const u32 *)(src)), (u32 *)(dst))

static const unsigned char lzop_magic[] = {
	0x89, 0x4c, 0x5a, 0x4f, 0x00, 0x0d, 0x0a, 0x1a, 0x0a
};

#define HEADER_HAS_FILTER	0x00000800L


bool lzop_is_valid_header(const unsigned char *src)
{
	int i;
	/* read magic: 9 first bytes */
	for (i = 0; i < ARRAY_SIZE(lzop_magic); i++) {
		if (*src++ != lzop_magic[i])
			return false;
	}
	return true;
}

static inline const unsigned char *parse_header(const unsigned char *src)
{
	u16 version;
	int i;

	if (!lzop_is_valid_header(src))
		return NULL;

	/* skip header */
	src += 9;

	/* get version (2bytes), skip library version (2),
	 * 'need to be extracted' version (2) and
	 * method (1) */
	version = get_unaligned_be16(src);
	src += 7;
	if (version >= 0x0940)
		src++;
	if (get_unaligned_be32(src) & HEADER_HAS_FILTER)
		src += 4; /* filter info */

	/* skip flags, mode and mtime_low */
	src += 12;
	if (version >= 0x0940)
		src += 4;	/* skip mtime_high */

	i = *src++;
	/* don't care about the file name, and skip checksum */
	src += i + 4;

	return src;
}

int lzop_decompress(const unsigned char *src, size_t src_len,
		    unsigned char *dst, size_t *dst_len)
{
	unsigned char *start = dst;
	const unsigned char *send = src + src_len;
	u32 slen, dlen;
	size_t tmp, remaining;
	int r;

	src = parse_header(src);
	if (!src)
		return LZO_E_ERROR;

	remaining = *dst_len;
	while (src < send) {
		/* read uncompressed block size */
		dlen = get_unaligned_be32(src);
		src += 4;

		/* exit if last block */
		if (dlen == 0) {
			*dst_len = dst - start;
			return LZO_E_OK;
		}

		/* read compressed block size, and skip block checksum info */
		slen = get_unaligned_be32(src);
		src += 8;

		if (slen <= 0 || slen > dlen)
			return LZO_E_ERROR;

		/* abort if buffer ran out of room */
		if (dlen > remaining)
			return LZO_E_OUTPUT_OVERRUN;

		/* When the input data is not compressed at all,
		 * lzo1x_decompress_safe will fail, so call memcpy()
		 * instead */
		if (dlen == slen) {
			memcpy(dst, src, slen);
		} else {
			/* decompress */
			tmp = dlen;
			r = lzo1x_decompress_safe((u8 *)src, slen, dst, &tmp);

			if (r != LZO_E_OK) {
				*dst_len = dst - start;
				return r;
			}

			if (dlen != tmp)
				return LZO_E_ERROR;
		}

		src += slen;
		dst += dlen;
		remaining -= dlen;
	}

	return LZO_E_INPUT_OVERRUN;
}

int lzo1x_decompress_safe(const unsigned char *in, size_t in_len,
			unsigned char *out, size_t *out_len)
{
	const unsigned char * const ip_end = in + in_len;
	unsigned char * const op_end = out + *out_len;
	const unsigned char *ip = in, *m_pos;
	unsigned char *op = out;
	size_t t;

	*out_len = 0;

	if (*ip > 17) {
		t = *ip++ - 17;
		if (t < 4)
			goto match_next;
		if (HAVE_OP(t, op_end, op))
			goto output_overrun;
		if (HAVE_IP(t + 1, ip_end, ip))
			goto input_overrun;
		do {
			*op++ = *ip++;
		} while (--t > 0);
		goto first_literal_run;
	}

	while ((ip < ip_end)) {
		t = *ip++;
		if (t >= 16)
			goto match;
		if (t == 0) {
			if (HAVE_IP(1, ip_end, ip))
				goto input_overrun;
			while (*ip == 0) {
				t += 255;
				ip++;
				if (HAVE_IP(1, ip_end, ip))
					goto input_overrun;
			}
			t += 15 + *ip++;
		}
		if (HAVE_OP(t + 3, op_end, op))
			goto output_overrun;
		if (HAVE_IP(t + 4, ip_end, ip))
			goto input_overrun;

		COPY4(op, ip);
		op += 4;
		ip += 4;
		if (--t > 0) {
			if (t >= 4) {
				do {
					COPY4(op, ip);
					op += 4;
					ip += 4;
					t -= 4;
				} while (t >= 4);
				if (t > 0) {
					do {
						*op++ = *ip++;
					} while (--t > 0);
				}
			} else {
				do {
					*op++ = *ip++;
				} while (--t > 0);
			}
		}

first_literal_run:
		t = *ip++;
		if (t >= 16)
			goto match;
		m_pos = op - (1 + M2_MAX_OFFSET);
		m_pos -= t >> 2;
		m_pos -= *ip++ << 2;

		if (HAVE_LB(m_pos, out, op))
			goto lookbehind_overrun;

		if (HAVE_OP(3, op_end, op))
			goto output_overrun;
		*op++ = *m_pos++;
		*op++ = *m_pos++;
		*op++ = *m_pos;

		goto match_done;

		do {
match:
			if (t >= 64) {
				m_pos = op - 1;
				m_pos -= (t >> 2) & 7;
				m_pos -= *ip++ << 3;
				t = (t >> 5) - 1;
				if (HAVE_LB(m_pos, out, op))
					goto lookbehind_overrun;
				if (HAVE_OP(t + 3 - 1, op_end, op))
					goto output_overrun;
				goto copy_match;
			} else if (t >= 32) {
				t &= 31;
				if (t == 0) {
					if (HAVE_IP(1, ip_end, ip))
						goto input_overrun;
					while (*ip == 0) {
						t += 255;
						ip++;
						if (HAVE_IP(1, ip_end, ip))
							goto input_overrun;
					}
					t += 31 + *ip++;
				}
				m_pos = op - 1;
				m_pos -= get_unaligned_le16(ip) >> 2;
				ip += 2;
			} else if (t >= 16) {
				m_pos = op;
				m_pos -= (t & 8) << 11;

				t &= 7;
				if (t == 0) {
					if (HAVE_IP(1, ip_end, ip))
						goto input_overrun;
					while (*ip == 0) {
						t += 255;
						ip++;
						if (HAVE_IP(1, ip_end, ip))
							goto input_overrun;
					}
					t += 7 + *ip++;
				}
				m_pos -= get_unaligned_le16(ip) >> 2;
				ip += 2;
				if (m_pos == op)
					goto eof_found;
				m_pos -= 0x4000;
			} else {
				m_pos = op - 1;
				m_pos -= t >> 2;
				m_pos -= *ip++ << 2;

				if (HAVE_LB(m_pos, out, op))
					goto lookbehind_overrun;
				if (HAVE_OP(2, op_end, op))
					goto output_overrun;

				*op++ = *m_pos++;
				*op++ = *m_pos;
				goto match_done;
			}

			if (HAVE_LB(m_pos, out, op))
				goto lookbehind_overrun;
			if (HAVE_OP(t + 3 - 1, op_end, op))
				goto output_overrun;

			if (t >= 2 * 4 - (3 - 1) && (op - m_pos) >= 4) {
				COPY4(op, m_pos);
				op += 4;
				m_pos += 4;
				t -= 4 - (3 - 1);
				do {
					COPY4(op, m_pos);
					op += 4;
					m_pos += 4;
					t -= 4;
				} while (t >= 4);
				if (t > 0)
					do {
						*op++ = *m_pos++;
					} while (--t > 0);
			} else {
copy_match:
				*op++ = *m_pos++;
				*op++ = *m_pos++;
				do {
					*op++ = *m_pos++;
				} while (--t > 0);
			}
match_done:
			t = ip[-2] & 3;
			if (t == 0)
				break;
match_next:
			if (HAVE_OP(t, op_end, op))
				goto output_overrun;
			if (HAVE_IP(t + 1, ip_end, ip))
				goto input_overrun;

			*op++ = *ip++;
			if (t > 1) {
				*op++ = *ip++;
				if (t > 2)
					*op++ = *ip++;
			}

			t = *ip++;
		} while (ip < ip_end);
	}

	*out_len = op - out;
	return LZO_E_EOF_NOT_FOUND;

eof_found:
	*out_len = op - out;
	return (ip == ip_end ? LZO_E_OK :
		(ip < ip_end ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN));
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
