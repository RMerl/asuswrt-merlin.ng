/*
 * Copyright (C) 2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2009-2013  Security Innovation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdlib.h>
#include <string.h>

#include "ntru_convert.h"

/**
 * 3-bit to 2-trit conversion tables: 2 represents -1
 */
static uint8_t const bits_2_trit1[] = {0, 0, 0, 1, 1, 1, 2, 2};
static uint8_t const bits_2_trit2[] = {0, 1, 2, 0, 1, 2, 0, 1};

/**
 * See header.
 */
void ntru_bits_2_trits(uint8_t const *octets, uint16_t num_trits, uint8_t *trits)
{
	uint32_t bits24, bits3, shift;

	while (num_trits >= 16)
	{
		/* get next three octets */
		bits24  = ((uint32_t)(*octets++)) << 16;
		bits24 |= ((uint32_t)(*octets++)) <<  8;
		bits24 |=  (uint32_t)(*octets++);

		/* for each 3 bits in the three octets, output 2 trits */
		bits3 = (bits24 >> 21) & 0x7;
		*trits++ = bits_2_trit1[bits3];
		*trits++ = bits_2_trit2[bits3];

		bits3 = (bits24 >> 18) & 0x7;
		*trits++ = bits_2_trit1[bits3];
		*trits++ = bits_2_trit2[bits3];

		bits3 = (bits24 >> 15) & 0x7;
		*trits++ = bits_2_trit1[bits3];
		*trits++ = bits_2_trit2[bits3];

		bits3 = (bits24 >> 12) & 0x7;
		*trits++ = bits_2_trit1[bits3];
		*trits++ = bits_2_trit2[bits3];

		bits3 = (bits24 >>  9) & 0x7;
		*trits++ = bits_2_trit1[bits3];
		*trits++ = bits_2_trit2[bits3];

		bits3 = (bits24 >>  6) & 0x7;
		*trits++ = bits_2_trit1[bits3];
		*trits++ = bits_2_trit2[bits3];

		bits3 = (bits24 >>  3) & 0x7;
		*trits++ = bits_2_trit1[bits3];
		*trits++ = bits_2_trit2[bits3];

		bits3 = bits24 & 0x7;
		*trits++ = bits_2_trit1[bits3];
		*trits++ = bits_2_trit2[bits3];

		num_trits -= 16;
	}
	if (num_trits == 0)
	{
		return;
	}

	/* get three octets */
	bits24  = ((uint32_t)(*octets++)) << 16;
	bits24 |= ((uint32_t)(*octets++)) <<  8;
	bits24 |=  (uint32_t)(*octets++);

	shift = 21;
	while (num_trits)
	{
		/**
		 * for each 3 bits in the three octets, output up to 2 trits
		 * until all trits needed are produced
		 */
		bits3 = (bits24 >> shift) & 0x7;
		shift -= 3;
		*trits++ = bits_2_trit1[bits3];
		if (--num_trits)
		{
			*trits++ = bits_2_trit2[bits3];
			--num_trits;
		}
	}
}

/**
 * See header.
 */
bool ntru_trits_2_bits(uint8_t const *trits, uint32_t num_trits, uint8_t *octets)
{
	bool all_trits_valid = TRUE;
	uint32_t bits24, bits3, shift;

	while (num_trits >= 16)
	{
		/* convert each 2 trits to 3 bits and pack */
		bits3  = *trits++ * 3;
		bits3 += *trits++;
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 = (bits3 << 21);

		bits3  = *trits++ * 3;
		bits3 += *trits++;
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 |= (bits3 << 18);

		bits3  = *trits++ * 3;
		bits3 += *trits++;
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 |= (bits3 << 15);

		bits3  = *trits++ * 3;
		bits3 += *trits++;
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 |= (bits3 << 12);

		bits3  = *trits++ * 3;
		bits3 += *trits++;
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 |= (bits3 <<  9);

		bits3  = *trits++ * 3;
		bits3 += *trits++;
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 |= (bits3 <<  6);

		bits3  = *trits++ * 3;
		bits3 += *trits++;
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 |= (bits3 <<  3);

		bits3  = *trits++ * 3;
		bits3 += *trits++;
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 |= bits3;

		num_trits -= 16;

		/* output three octets */
		*octets++ = (uint8_t)((bits24 >> 16) & 0xff);
		*octets++ = (uint8_t)((bits24 >>  8) & 0xff);
		*octets++ = (uint8_t)(bits24 & 0xff);
	}

	bits24 = 0;
	shift = 21;
	while (num_trits)
	{
		/* convert each 2 trits to 3 bits and pack */
		bits3 = *trits++ * 3;
		if (--num_trits)
		{
			bits3 += *trits++;
			--num_trits;
		}
		if (bits3 > 7)
		{
			bits3 = 7;
			all_trits_valid = FALSE;
		}
		bits24 |= (bits3 << shift);
		shift -= 3;
	}

	/* output three octets */
	*octets++ = (uint8_t)((bits24 >> 16) & 0xff);
	*octets++ = (uint8_t)((bits24 >>  8) & 0xff);
	*octets++ = (uint8_t)(bits24 & 0xff);

	return all_trits_valid;
}

/**
 * See header
 */
void ntru_coeffs_mod4_2_octets(uint16_t num_coeffs, uint16_t const *coeffs, uint8_t *octets)
{
    uint8_t bits2;
    int shift, i;

	*octets = 0;
	shift = 6;
	for (i = 0; i < num_coeffs; i++)
	{
		bits2 = (uint8_t)(coeffs[i] & 0x3);
		*octets |= bits2 << shift;
		shift -= 2;
		if (shift < 0)
		{
			++octets;
			*octets = 0;
			shift = 6;
		}
	}
}

/**
 * See header.
 */
void ntru_trits_2_octet(uint8_t const *trits, uint8_t *octet)
{
	int i;

	*octet = 0;
	for (i = 4; i >= 0; i--)
	{
		*octet = (*octet * 3) + trits[i];
	}
}

/**
 * See header.
 */
void ntru_octet_2_trits(uint8_t octet, uint8_t *trits)
{
	int i;

	for (i = 0; i < 5; i++)
	{
		trits[i] = octet % 3;
		octet = (octet - trits[i]) / 3;
	}
}

/**
 * See header.
 */
void ntru_indices_2_trits(uint16_t in_len, uint16_t const *in, bool plus1,
						  uint8_t *out)
{
	uint8_t trit = plus1 ? 1 : 2;
	int  i;

    for (i = 0; i < in_len; i++)
	{
		out[in[i]] = trit;
	}
}

/**
 * See header.
 */
void ntru_packed_trits_2_indices(uint8_t const *in, uint16_t num_trits,
								 uint16_t *indices_plus1,
								 uint16_t *indices_minus1)
{
	uint8_t trits[5];
	uint16_t i = 0;
	int j;

	while (num_trits >= 5)
	{
		ntru_octet_2_trits(*in++, trits);
		num_trits -= 5;
		for (j = 0; j < 5; j++, i++)
		{
			if (trits[j] == 1)
			{
				*indices_plus1 = i;
				++indices_plus1;
			}
			else if (trits[j] == 2)
			{
				*indices_minus1 = i;
				++indices_minus1;
			}
		}
    }
	if (num_trits)
	{
		ntru_octet_2_trits(*in, trits);
		for (j = 0; num_trits && (j < 5); j++, i++)
		{
			if (trits[j] == 1)
			{
				*indices_plus1 = i;
				++indices_plus1;
			}
			else if (trits[j] == 2)
			{
				*indices_minus1 = i;
				++indices_minus1;
			}
			--num_trits;
		}
	}
}

/**
 * See header.
 */
void ntru_indices_2_packed_trits(uint16_t const *indices, uint16_t num_plus1,
								 uint16_t num_minus1, uint16_t num_trits,
								 uint8_t *buf, uint8_t *out)
{
	/* convert indices to an array of trits */
	memset(buf, 0, num_trits);
	ntru_indices_2_trits(num_plus1, indices, TRUE, buf);
	ntru_indices_2_trits(num_minus1, indices + num_plus1, FALSE, buf);

	/* pack the array of trits */
	while (num_trits >= 5)
	{
		ntru_trits_2_octet(buf, out);
		num_trits -= 5;
		buf += 5;
		++out;
	}
	if (num_trits)
	{
		uint8_t trits[5];

		memcpy(trits, buf, num_trits);
		memset(trits + num_trits, 0, sizeof(trits) - num_trits);
		ntru_trits_2_octet(trits, out);
	}
}

/**
 * See header
 */
void ntru_elements_2_octets(uint16_t in_len, uint16_t const *in, uint8_t n_bits,
							uint8_t *out)
{
	uint16_t temp;
	int shift, i;

	/* pack */
	temp = 0;
	shift = n_bits - 8;
	i = 0;
	while (i < in_len)
	{
		/* add bits to temp to fill an octet and output the octet */
		temp |= in[i] >> shift;
		*out++ = (uint8_t)(temp & 0xff);
		shift = 8 - shift;
		if (shift < 1)
		{
			/* next full octet is in current input word */
			shift += n_bits;
			temp = 0;
		}
		else
		{
			/* put remaining bits of input word in temp as partial octet,
			 * and increment index to next input word
			 */
			temp = in[i] << (uint16_t)shift;
			++i;
		}
		shift = n_bits - shift;
	}

	/* output any bits remaining in last input word */
	if (shift != n_bits - 8)
	{
		*out++ = (uint8_t)(temp & 0xff);
	}
}


/**
 * See header.
 */
void ntru_octets_2_elements(uint16_t in_len, uint8_t const *in, uint8_t n_bits,
							uint16_t *out)
{
	uint16_t  temp;
	uint16_t  mask = (1 << n_bits) - 1;
	int shift, i;

	/* unpack */
	temp = 0;
	shift = n_bits;
	i = 0;
	while (i < in_len)
	{
		shift = 8 - shift;
		if (shift < 0)
		{
			/* the current octet will not fill the current element */
			shift += n_bits;
		}
		else
		{
			/* add bits from the current octet to fill the current element and
			 * output the element
			 */
			temp |= ((uint16_t)in[i]) >> shift;
			*out++ = temp & mask;
			temp = 0;
		}

		/* add the remaining bits of the current octet to start an element */
		shift = n_bits - shift;
		temp |= ((uint16_t)in[i]) << shift;
		++i;
	}
}
