/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2013 Vincent Bernat <bernat@luffy.cx>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "fixedpoint.h"

/* This is not a general purpose fixed point library. First, there is no
 * arithmetic. Second, some functions assume that the total precision does not
 * exceed 64 bits.
 */

#ifdef ENABLE_LLDPMED

#ifndef ntohll
# define ntohll(x)						\
	(((u_int64_t)(ntohl((int)(((x) << 32) >> 32))) << 32) |	\
	    (unsigned int)ntohl(((int)((x) >> 32))))
#endif

/**
 * Convert a string to fixed point number.
 *
 * @param repr String to convert.
 * @param end If not NULL, will contain a pointer to the character after the
 *            last character used in the conversion.
 * @param intbits Number of bits to represent the integer part.
 * @param fltbits Number of bits to represent the float part.
 * @return A fixed point number.
 *
 * If there is an overflow, there will be a truncation. Moreover, the fraction
 * part will be rounded to the nearest possible power of two representation. The
 * point will depend on the number of decimal provided with the fraction
 * part.
 */
struct fp_number
fp_strtofp(const char *repr, char **end,
    unsigned intbits, unsigned fltbits)
{
	char *endptr = NULL, *e2;
	struct fp_number result = {
		.integer = { 0, intbits },
		.fraction = { 0, fltbits, 0 }
	};
	result.integer.value = strtoll(repr, &endptr, 10);
	if (result.integer.value >= (1LL << (intbits - 1)))
		result.integer.value = (1LL << (intbits - 1)) - 1;
	else if (result.integer.value < ~(1LL << (intbits - 1)) + 1)
		result.integer.value = ~(1LL << (intbits - 1)) + 1;
	if (*endptr == '.') {
		long long precision = 1;
		e2 = endptr + 1;
		result.fraction.value = strtoll(e2, &endptr, 10);
		/* Convert to a representation in power of two. Get the
		 * precision from the number of digits provided. This is NOT the
		 * value of the higher bits in the binary representation: we
		 * consider that if the user inputs, 0.9375, it means to
		 * represent anything between 0 and 0.9999 with the same
		 * precision. Therefore, we don't have only 4 bits of precision
		 * but 14. */
		while (e2++ != endptr) precision *= 10;
		result.fraction.value <<= fltbits;
		result.fraction.value /= precision;
		result.fraction.precision = (precision == 1)?1:
		    (sizeof(precision) * 8 - __builtin_clzll(precision - 1));
		if (result.fraction.precision > fltbits)
			result.fraction.precision = fltbits;
	}
	if (end) *end = endptr;
	return result;
}

/**
 * Get a string representation of a fixed point number.
 *
 * @param fp Fixed point number.
 * @param suffix If not NULL, use the first character when positive and the
 *               second one when negative instead of prefixing by `-`.
 * @return the string representation
 *
 * Since we convert from binary to decimal, we are as precise as the binary
 * representation.
 */
char *
fp_fptostr(struct fp_number fp, const char *suffix)
{
	char *result = NULL;
	char *frac = NULL;
	int negative = (fp.integer.value < 0);
	if (fp.fraction.value == 0)
		frac = strdup("");
	else {
		long long decimal = fp.fraction.value;
		long long precision = 1;
		int len = 0;
		while ((1LL << fp.fraction.precision) > precision) {
			precision *= 10;
			len += 1;
		}
		/* We did round-up, when converting from decimal. We round-down
		 * to have some coherency. */
		precision /= 10; len -= 1;
		if (precision == 0) precision = 1;
		decimal *= precision;
		decimal >>= fp.fraction.bits;
		if (asprintf(&frac, ".%0*llu", len, decimal) == -1)
			return NULL;
	}
	if (asprintf(&result, "%s%llu%s%c",
		(suffix == NULL && negative) ? "-" : "",
		(negative) ? (-fp.integer.value) : fp.integer.value,
		frac,
		(suffix && !negative) ? suffix[0] :
		(suffix && negative) ? suffix[1] : ' ') == -1) {
		free(frac);
		return NULL;
	}
	free(frac);
	if (!suffix) result[strlen(result) - 1] = '\0';
	return result;
}

/**
 * Turn a fixed point number into its representation in a buffer.
 *
 * @param fp Fixed point number.
 * @param buf Output buffer.
 * @param shift Number of bits to skip at the beginning of the buffer.
 *
 * The representation of a fixed point number is the precision (always 6 bits
 * because we assume that int part + frac part does not exceed 64 bits), the
 * integer part and the fractional part.
 */
void
fp_fptobuf(struct fp_number fp, unsigned char *buf, unsigned shift)
{
	unsigned long long value = (fp.integer.value >= 0) ?
	    ((fp.integer.value << fp.fraction.bits) + fp.fraction.value) :
	    (~(((unsigned long long)(-fp.integer.value) << fp.fraction.bits) +
		fp.fraction.value) + 1);
	unsigned long long ints[] = { fp.integer.bits + fp.fraction.precision,
				      value };
	unsigned int bits[] = { 6,
				fp.integer.bits + fp.fraction.bits };

	unsigned i, obit, o;
	for (i = 0, obit = 8 - (shift % 8), o = shift / 8; i < 2;) {
		if (obit > bits[i]) {
			/* We need to clear bits that will be overwritten but do not touch other bits */
			if (bits[i] != 0) {
				buf[o] = buf[o] & (~((1 << obit) - 1) |
				    ((1 << (obit - bits[i])) - 1));
				buf[o] = buf[o] |
				    ((ints[i] & ((1 << bits[i]) - 1)) << (obit - bits[i]));
				obit -= bits[i];
			}
			i++;
		} else {
			/* As in the other branch... */
			buf[o] = buf[o] & (~((1 << obit) - 1));
			buf[o] = buf[o] |
			    ((ints[i] >> (bits[i] - obit)) & ((1 << obit) - 1));
			bits[i] -= obit;
			obit = 8;
			o++;
		}
	}
}

/**
 * Parse a fixed point number from a buffer.
 *
 * @param buf Input buffer
 * @param intbits Number of bits used for integer part.
 * @param fltbits Number of bits used for fractional part.
 * @param shift Number of bits to skip at the beginning of the buffer.
 *
 * @return the parsed fixed point number.
 *
 * The representation is the same as for @c fp_fptobuf().
 */
struct fp_number
fp_buftofp(const unsigned char *buf,
    unsigned intbits, unsigned fltbits,
    unsigned shift)
{
	unsigned long long value = 0, precision = 0;
	unsigned long long *ints[] = { &precision,
				       &value };
	unsigned int bits[] = { 6,
				intbits + fltbits };

	unsigned o, ibit, i;
	for (o = 0, ibit = 8 - (shift % 8), i = shift / 8; o < 2;) {
		if (ibit > bits[o]) {
			if (bits[o] > 0) {
				*ints[o] = *ints[o] | ((buf[i] >> (ibit - bits[o])) & ((1ULL << bits[o]) - 1));
				ibit -= bits[o];
			}
			o++;
		} else {
			*ints[o] = *ints[o] | ((buf[i] & ((1ULL << ibit) - 1)) << (bits[o] - ibit));
			bits[o] -= ibit;
			ibit = 8;
			i++;
		}
	}

	/* Don't handle too low precision */
	if (precision > intbits)
		precision -= intbits;
	else
		precision = intbits;

	int negative = !!(value & (1ULL << (intbits + fltbits - 1)));
	if (negative) value = (~value + 1) & ((1ULL << (intbits + fltbits - 1)) - 1);
	struct fp_number result = {
		.integer = { value >> fltbits, intbits },
		.fraction = { value & ((1ULL << fltbits) - 1), fltbits, precision }
	};
	if (negative) result.integer.value = -result.integer.value;

	return result;
}

/**
 * Negate a fixed point number.
 */
struct fp_number
fp_negate(struct fp_number fp)
{
	unsigned intbits = fp.integer.bits;
	struct fp_number result = fp;
	result.integer.value = -result.integer.value;
	if (result.integer.value >= (1LL << (intbits - 1)))
		result.integer.value = (1LL << (intbits - 1)) - 1;
	else if (result.integer.value < ~(1LL << (intbits - 1)) + 1)
		result.integer.value = ~(1LL << (intbits - 1)) + 1;
	return result;
}

#endif
