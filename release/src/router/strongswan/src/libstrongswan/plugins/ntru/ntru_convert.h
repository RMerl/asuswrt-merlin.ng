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

/**
 * @defgroup ntru_convert ntru_convert
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_CONVERT_H_
#define NTRU_CONVERT_H_

#include <library.h>

/**
 * Each 3 bits in an array of octets is converted to 2 trits in an array
 * of trits.
 *
 * @param octets		pointer to array of octets
 * @param num_trits		number of trits to produce
 * @param trits			address for array of trits
 */
void ntru_bits_2_trits(uint8_t const *octets, uint16_t num_trits,
					   uint8_t *trits);

/**
 * Each 2 trits in an array of trits is converted to 3 bits, and the bits
 * are packed in an array of octets.  A multiple of 3 octets is output.
 * Any bits in the final octets not derived from trits are zero.
 *
 * @param trits				pointer to array of trits
 * @param num_trits			number of trits to convert
 * @param octets			address for array of octets
 * @return					TRUE if all trits were valid
 *                     		FALSE if invalid trits were found
 */
bool ntru_trits_2_bits(uint8_t const *trits, uint32_t num_trits,
					   uint8_t *octets);

/**
 * Takes an array of coefficients mod 4 and packs the results into an
 * octet string.
 *
 * @param num_coeffs		number of coefficients
 * @param coeffs			pointer to coefficients
 * @param octets			address for octets
 */
void ntru_coeffs_mod4_2_octets(uint16_t num_coeffs, uint16_t const *coeffs,
							   uint8_t *octets);

/**
 * Packs 5 trits in an octet, where a trit is 0, 1, or 2 (-1).
 *
 * @param trits				pointer to trits
 * @param octet				address for octet
 */
void ntru_trits_2_octet(uint8_t const *trits, uint8_t *octet);

/**
 * Unpacks an octet to 5 trits, where a trit is 0, 1, or 2 (-1).
 *
 * @param octet				octet to be unpacked
 * @param trits				address for trits
 */
void ntru_octet_2_trits(uint8_t  octet, uint8_t *trits);

/**
 *
 * Converts a list of the nonzero indices of a polynomial into an array of
 * trits.
 *
 * @param in_len			no. of indices
 * @param in				pointer to list of indices
 * @param plus1				if list is +1 coefficients
 * @param out				address of output polynomial
 */
void ntru_indices_2_trits(uint16_t in_len, uint16_t const *in, bool plus1,
						  uint8_t *out);

/**
 * Unpacks an array of N trits and creates a list of array indices 
 * corresponding to trits = +1, and list of array indices corresponding to
 * trits = -1.
 *
 * @param in				pointer to packed-trit octets
 * @param num_trits			no. of packed trits
 * @param indices_plus1		address for indices of +1 trits
 * @param indices_minus1	address for indices of -1 trits
 */
void ntru_packed_trits_2_indices(uint8_t const *in, uint16_t num_trits,
								 uint16_t *indices_plus1,
								 uint16_t *indices_minus1);

/**
 * Takes a list of array indices corresponding to elements whose values
 * are +1 or -1, and packs the N-element array of trits described by these
 * lists into octets, 5 trits per octet.
 *
 * @param indices			pointer to indices
 * @param num_plus1			no. of indices for +1 trits
 * @param num_minus1		no. of indices for -1 trits
 * @param num_trits			N, no. of trits in array
 * @param buf				temp buf, N octets
 * @param out				address for packed octet
 */
void ntru_indices_2_packed_trits(uint16_t const *indices, uint16_t num_plus1,
								 uint16_t num_minus1, uint16_t num_trits,
								 uint8_t *buf, uint8_t *out);

/**
 * Packs an array of n-bit elements into an array of
 * ((in_len * n_bits) + 7) / 8 octets, 8 < n_bits < 16.
 *
 * @param in_len			no. of elements to be packed
 * @param in				ptr to elements to be packed
 * @param n_bits			no. of bits in input element
 * @param out				addr for output octets
 */
void ntru_elements_2_octets(uint16_t in_len, uint16_t const *in, uint8_t n_bits,
							uint8_t *out);

/**
 * Unpacks an octet string into an array of ((in_len * 8) / n_bits)
 * n-bit elements, 8 < n < 16.  Any extra bits are discarded.
 *
 * @param in_len			no. of octets to be unpacked
 * @param in				ptr to octets to be unpacked
 * @param n_bits			no. of bits in output element
 * @param out				addr for output elements
 */
void ntru_octets_2_elements(uint16_t in_len, uint8_t const *in, uint8_t n_bits,
							uint16_t *out);

#endif /** NTRU_CONVERT_H_ @}*/
