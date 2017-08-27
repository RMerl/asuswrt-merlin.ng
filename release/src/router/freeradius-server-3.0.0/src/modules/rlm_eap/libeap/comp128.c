/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file comp128.c
 * @brief Implementations of comp128v1, comp128v2, comp128v3 algorithms
 *
 * Comp128v1 was inspired by code from:
 *  Marc Briceno <marc@scard.org>, Ian Goldberg <iang@cs.berkeley.edu>,
 *  and David Wagner <daw@cs.berkeley.edu>
 *
 * But it has been fully rewritten (Sylvain Munaut <tnt@246tNt.com>) from various PDFs found online
 * describing the algorithm because the licence of the code referenced above was unclear.
 * A comment snippet from the original code is included below, it describes where the doc came
 * from and how the algorithm was reverse engineered.
 *
 * Comp128v2 & v3 is a port of the python code from:
 *   http://www.hackingprojects.net/
 *
 * @note The above GPL license only applies to comp128v1, the license for comp128v2 and comp128v3 is unknown.
 *
 * @copyright 2013 The FreeRADIUS server project
 * @copyright 2013 Hacking projects [http://www.hackingprojects.net/]
 * @copyright 2009 Sylvain Munaut <tnt@246tNt.com>
 */

#include "comp128.h"
#include <stdio.h>
/* 512 bytes */
static uint8_t const comp128v1_t0[] = {
	102, 177, 186, 162, 2,   156, 112, 75,  55,  25,  8,   12,  251, 193, 246, 188,
	109, 213, 151, 53,  42,  79,  191, 115, 233, 242, 164, 223, 209, 148, 108, 161,
	252, 37,  244, 47,  64,  211, 6,   237, 185, 160, 139, 113, 76,  138, 59,  70,
	67,  26,  13,  157, 63,  179, 221, 30,  214, 36,  166, 69,  152, 124, 207, 116,
	247, 194, 41,  84,  71,  1,   49,  14,  95,  35,  169, 21,  96,  78,  215, 225,
	182, 243, 28,  92,  201, 118, 4,   74,  248, 128, 17,  11,  146, 132, 245, 48,
	149, 90,  120, 39,  87,  230, 106, 232, 175, 19,  126, 190, 202, 141, 137, 176,
	250, 27,  101, 40,  219, 227, 58,  20,  51,  178, 98,  216, 140, 22,  32,  121,
	61,  103, 203, 72,  29,  110, 85,  212, 180, 204, 150, 183, 15,  66,  172, 196,
	56,  197, 158, 0,   100, 45,  153, 7,   144, 222, 163, 167, 60,  135, 210, 231,
	174, 165, 38,  249, 224, 34,  220, 229, 217, 208, 241, 68,  206, 189, 125, 255,
	239, 54,  168, 89,  123, 122, 73,  145, 117, 234, 143, 99,  129, 200, 192, 82,
	104, 170, 136, 235, 93,  81,  205, 173, 236, 94,  105, 52,  46,  228, 198, 5,
	57,  254, 97,  155, 142, 133, 199, 171, 187, 50,  65,  181, 127, 107, 147, 226,
	184, 218, 131, 33,  77,  86,  31,  44,  88,  62,  238, 18,  24,  43,  154, 23,
	80,  159, 134, 111, 9,   114, 3,   91,  16,  130, 83,  10,  195, 240, 253, 119,
	177, 102, 162, 186, 156, 2,   75,  112, 25,  55,  12,  8,   193, 251, 188, 246,
	213, 109, 53,  151, 79,  42,  115, 191, 242, 233, 223, 164, 148, 209, 161, 108,
	37,  252, 47,  244, 211, 64,  237, 6,   160, 185, 113, 139, 138, 76,  70,  59,
	26,  67,  157, 13,  179, 63,  30,  221, 36,  214, 69,  166, 124, 152, 116, 207,
	194, 247, 84,  41,  1,   71,  14,  49,  35,  95,  21,  169, 78,  96,  225, 215,
	243, 182, 92,  28,  118, 201, 74,  4,   128, 248, 11,  17,  132, 146, 48,  245,
	90,  149, 39,  120, 230, 87,  232, 106, 19,  175, 190, 126, 141, 202, 176, 137,
	27,  250, 40,  101, 227, 219, 20,  58,  178, 51,  216, 98,  22,  140, 121, 32,
	103, 61,  72,  203, 110, 29,  212, 85,  204, 180, 183, 150, 66,  15,  196, 172,
	197, 56,  0,   158, 45,  100, 7,   153, 222, 144, 167, 163, 135, 60,  231, 210,
	165, 174, 249, 38,  34,  224, 229, 220, 208, 217, 68,  241, 189, 206, 255, 125,
	54,  239, 89,  168, 122, 123, 145, 73,  234, 117, 99,  143, 200, 129, 82,  192,
	170, 104, 235, 136, 81,  93,  173, 205, 94,  236, 52,  105, 228, 46,  5,   198,
	254, 57,  155, 97,  133, 142, 171, 199, 50,  187, 181, 65,  107, 127, 226, 147,
	218, 184, 33,  131, 86,  77,  44,  31,  62,  88,  18,  238, 43,  24,  23,  154,
	159, 80,  111, 134, 114, 9,   91,  3,   130, 16,  10,  83,  240, 195, 119, 253};

/* 256 bytes */
static uint8_t const comp128v1_t1[] = {
	19,  11,  80,  114, 43,  1,   69,  94,  39,  18,  127, 117, 97,  3,   85,  43,
	27,  124, 70,  83,  47,  71,  63,  10,  47,  89,  79,  4,   14,  59,  11,  5,
	35,  107, 103, 68,  21,  86,  36,  91,  85,  126, 32,  50,  109, 94,  120, 6,
	53,  79,  28,  45,  99,  95,  41,  34,  88,  68,  93,  55,  110, 125, 105, 20,
	90,  80,  76,  96,  23,  60,  89,  64,  121, 56,  14,  74,  101, 8,   19,  78,
	76,  66,  104, 46,  111, 50,  32,  3,   39,  0,   58,  25,  92,  22,  18,  51,
	57,  65,  119, 116, 22,  109, 7,   86,  59,  93,  62,  110, 78,  99,  77,  67,
	12,  113, 87,  98,  102, 5,   88,  33,  38,  56,  23,  8,   75,  45,  13,  75,
	95,  63,  28,  49,  123, 120, 20,  112, 44,  30,  15,  98,  106, 2,   103, 29,
	82,  107, 42,  124, 24,  30,  41,  16,  108, 100, 117, 40,  73,  40,  7,   114,
	82,  115, 36,  112, 12,  102, 100, 84,  92,  48,  72,  97,  9,   54,  55,  74,
	113, 123, 17,  26,  53,  58,  4,   9,   69,  122, 21,  118, 42,  60,  27,  73,
	118, 125, 34,  15,  65,  115, 84,  64,  62,  81,  70,  1,   24,  111, 121, 83,
	104, 81,  49,  127, 48,  105, 31,  10,  6,   91,  87,  37,  16,  54,  116, 126,
	31,  38,  13,  0,   72,  106, 77,  61,  26,  67,  46,  29,  96,  37,  61,  52,
	101, 17,  44,  108, 71,  52,  66,  57,  33,  51,  25,  90,  2,   119, 122, 35};

/* 128 bytes */
static uint8_t const comp128v1_t2[] = {
	52,  50,  44,   6,  21,  49,  41,  59,  39,  51,  25,  32,  51,  47,  52,  43,
	37,  4,   40,  34,  61,  12,  28,   4,  58,  23,   8,  15,  12,  22,   9,  18,
	55,  10,  33,  35,  50,   1,  43,   3,  57,  13,  62,  14,   7,  42,  44,  59,
	62,  57,  27,   6,   8,  31,  26,  54,  41,  22,  45,  20,  39,   3,  16,  56,
	48,  2,   21,  28,  36,  42,  60,  33,  34,  18,   0,  11,  24,  10,  17,  61,
	29,  14,  45,  26,  55,  46,  11,  17,  54,  46,   9,  24,  30,  60,  32,   0,
	20,  38,  2,   30,  58,  35,   1,  16,  56,  40,  23,  48,  13,  19,  19,  27,
	31,  53,  47,  38,  63,  15,  49,   5,  37,  53,  25,  36,  63,  29,   5,   7};

/* 64 bytes */
static uint8_t const comp128v1_t3[] = {
	1,   5,   29,  6,   25,  1,   18,  23,  17,  19,  0,   9,   24,  25,  6,   31,
	28,  20,  24,  30,  4,   27,  3,   13,  15,  16,  14,  18,  4,   3,   8,   9,
	20,  0,   12,  26,  21,  8,   28,  2,   29,  2,   15,  7,   11,  22,  14,  10,
	17,  21,  12,  30,  26,  27,  16,  31,  11,  7,   13,  23,  10,  5,   22,  19};

/* 32 bytes */
static uint8_t const comp128v1_t4[] = {
	15,  12,  10,  4,   1,   14,  11,  7,   5,   0,   14,  7,   1,   2,   13,  8,
	10,  3,   4,   9,   6,   0,   3,   2,   5,   6,   8,   9,   11,  13,  15,  12};

static uint8_t const *_comp128_table[] = { comp128v1_t0, comp128v1_t1, comp128v1_t2, comp128v1_t3, comp128v1_t4 };

/* 256 bytes */
static uint8_t const comp128v23_t0[] = {
	197, 235, 60,  151, 98,  96,  3,   100, 248, 118, 42,  117, 172, 211, 181, 203,
	61,  126, 156, 87,  149, 224, 55,  132, 186, 63,  238, 255, 85,  83,  152, 33,
	160, 184, 210, 219, 159, 11,  180, 194, 130, 212, 147, 5,   215, 92,  27,  46,
	113, 187, 52,  25,  185, 79,  221, 48,  70,  31,  101, 15,  195, 201, 50,  222,
	137, 233, 229, 106, 122, 183, 178, 177, 144, 207, 234, 182, 37,  254, 227, 231,
	54,  209, 133, 65,  202, 69,  237, 220, 189, 146, 120, 68,  21,  125, 38,  30,
	2,   155, 53,  196, 174, 176, 51,  246, 167, 76,  110, 20,  82,  121, 103, 112,
	56,  173, 49,  217, 252, 0,   114, 228, 123, 12,  93,  161, 253, 232, 240, 175,
	67,  128, 22,  158, 89,  18,  77,  109, 190, 17,  62,  4,   153, 163, 59,  145,
	138, 7,   74,  205, 10,  162, 80,  45,  104, 111, 150, 214, 154, 28,  191, 169,
	213, 88,  193, 198, 200, 245, 39,  164, 124, 84,  78,  1,   188, 170, 23,  86,
	226, 141, 32,  6,   131, 127, 199, 40,  135, 16,  57,  71,  91,  225, 168, 242,
	206, 97,  166, 44,  14,  90,  236, 239, 230, 244, 223, 108, 102, 119, 148, 251,
	29,  216, 8,   9,   249, 208, 24,  105, 94,  34,  64,  95,  115, 72,  134, 204,
	43,  247, 243, 218, 47,  58,  73,  107, 241, 179, 116, 66,  36,  143, 81,  250,
	139, 19,  13,  142, 140, 129, 192, 99,  171, 157, 136, 41,  75,  35,  165, 26};

/* 256 bytes */
static uint8_t const comp128v23_t1[] = {
	170, 42,  95,  141, 109, 30,  71,  89,  26,  147, 231, 205, 239, 212, 124, 129,
	216, 79,  15,  185, 153, 14,  251, 162, 0,   241, 172, 197, 43,  10,  194, 235,
	6,   20,  72,  45,  143, 104, 161, 119, 41,  136, 38,  189, 135, 25,  93,  18,
	224, 171, 252, 195, 63,  19,  58,  165, 23,  55,  133, 254, 214, 144, 220, 178,
	156, 52,  110, 225, 97,  183, 140, 39,  53,  88,  219, 167, 16,  198, 62,  222,
	76,  139, 175, 94,  51,  134, 115, 22,  67,  1,   249, 217, 3,   5,   232, 138,
	31,  56,  116, 163, 70,  128, 234, 132, 229, 184, 244, 13,  34,  73,  233, 154,
	179, 131, 215, 236, 142, 223, 27,  57,  246, 108, 211, 8,   253, 85,  66,  245,
	193, 78,  190, 4,   17,  7,   150, 127, 152, 213, 37,  186, 2,   243, 46,  169,
	68,  101, 60,  174, 208, 158, 176, 69,  238, 191, 90,  83,  166, 125, 77,  59,
	21,  92,  49,  151, 168, 99,  9,   50,  146, 113, 117, 228, 65,  230, 40,  82,
	54,  237, 227, 102, 28,  36,  107, 24,  44,  126, 206, 201, 61,  114, 164, 207,
	181, 29,  91,  64,  221, 255, 48,  155, 192, 111, 180, 210, 182, 247, 203, 148,
	209, 98,  173, 11,  75,  123, 250, 118, 32,  47,  240, 202, 74,  177, 100, 80,
	196, 33,  248, 86,  157, 137, 120, 130, 84,  204, 122, 81,  242, 188, 200, 149,
	226, 218, 160, 187, 106, 35,  87,  105, 96,  145, 199, 159, 12,  121, 103, 112};

static inline void _comp128_compression_round(uint8_t *x, int n, const uint8_t *tbl)
{
	int i, j, m, a, b, y, z;
	m = 4 - n;
	for (i = 0; i < (1 << n); i++) {
		for (j = 0; j < (1 << m); j++) {
			a = j + i * (2 << m);
			b = a + (1 << m);
			y = (x[a] + (x[b] << 1)) & ((32 << m) - 1);
			z = ((x[a] << 1) + x[b]) & ((32 << m) - 1);
			x[a] = tbl[y];
			x[b] = tbl[z];
		}
	}
}

static inline void _comp128_compression(uint8_t *x)
{
	int n;
	for (n = 0; n < 5; n++) {
		_comp128_compression_round(x, n, _comp128_table[n]);
	}
}

static inline void _comp128_bitsfrombytes(uint8_t *x, uint8_t *bits)
{
	int i;

	memset(bits, 0x00, 128);
	for (i = 0; i < 128; i++) {
		if (x[i >> 2] & (1 << (3 - (i & 3)))) {
			bits[i] = 1;
		}
	}
}

static inline void _comp128_permutation(uint8_t *x, uint8_t *bits)
{
	int i;
	memset(&x[16], 0x00, 16);
	for (i = 0; i < 128; i++) {
		x[(i >> 3) + 16] |= bits[(i * 17) & 127] << (7 - (i & 7));
	}
}

/** Calculate comp128v1 sres and kc from ki and rand
 *
 * This code derived from a leaked document from the GSM standards.
 * Some missing pieces were filled in by reverse-engineering a working SIM.
 * We have verified that this is the correct COMP128 algorithm.
 *
 * The first page of the document identifies it as
 * 	_Technical Information: GSM System Security Study_.
 * 	10-1617-01, 10th June 1988.
 * The bottom of the title page is marked
 * 	Racal Research Ltd.
 * 	Worton Drive, Worton Grange Industrial Estate,
 * 	Reading, Berks. RG2 0SB, England.
 * 	Telephone: Reading (0734) 868601   Telex: 847152
 * The relevant bits are in Part I, Section 20 (pages 66--67).  Enjoy!
 *
 * Note: There are three typos in the spec (discovered by reverse-engineering).
 * First, "z = (2 * x[n] + x[n]) mod 2^(9-j)" should clearly read
 *	"z = (2 * x[m] + x[n]) mod 2^(9-j)".
 * Second, the "k" loop in the "Form bits from bytes" section is severely
 * botched: the k index should run only from 0 to 3, and clearly the range
 * on "the (8-k)th bit of byte j" is also off (should be 0..7, not 1..8,
 * to be consistent with the subsequent section).
 * Third, SRES is taken from the first 8 nibbles of x[], not the last 8 as
 * claimed in the document.  (And the document doesn't specify how Kc is
 * derived, but that was also easily discovered with reverse engineering.)
 * All of these typos have been corrected in the following code.
 *
 * @param[out] sres 4 byte value derived from ki and rand.
 * @param[out] kc 12 byte value derived from ki and rand.
 * @param[in] ki known only by the SIM and AuC (us in this case).
 * @param[in] rand 16 bytes of randomness.
 */
void comp128v1(uint8_t *sres, uint8_t *kc, uint8_t const *ki, uint8_t const *rand)
{
	int i;
	uint8_t x[32], bits[128];

	/* x[16-31] = RAND */
	memcpy(&x[16], rand, 16);

	/*
	 *	Round 1-7
	 */
	for (i=0; i < 7; i++) {
		/* x[0-15] = Ki */
		memcpy(x, ki, 16);

		/* Compression */
		_comp128_compression(x);

		/* FormBitFromBytes */
		_comp128_bitsfrombytes(x, bits);

		/* Permutation */
		_comp128_permutation(x, bits);
	}

	/*
	 * 	Round 8 (final)
	 * 	x[0-15] = Ki
	 */
	memcpy(x, ki, 16);

	/* Compression */
	_comp128_compression(x);

	/* Output stage */
	for (i = 0; i < 8; i += 2) {
		sres[i >> 1] = x[i] << 4 | x[i + 1];
	}

	for (i = 0; i < 12; i += 2) {
		kc[i>>1] = (x[i + 18] << 6) |
		           (x[i + 19] << 2) |
		           (x[i + 20] >> 2);
	}

	kc[6] = (x[30] << 6) | (x[31] << 2);
	kc[7] = 0;
}

static void _comp128v23(uint8_t *rand, uint8_t const *kxor)
{
	uint8_t temp[16];
	uint8_t km_rm[32];

	int j, i, k, z;

	memset(&temp, 0, sizeof(temp));
	memcpy(km_rm, rand, 16);
	memcpy(km_rm + 16, kxor, 16);
	memset(rand, 0, 16);

	for (i = 0; i < 5; i++) {
		j = 0;

		for (z = 0; z < 16; z++) {
			temp[z] = comp128v23_t0[comp128v23_t1[km_rm[16 + z]] ^ km_rm[z]];
		}

		while ((1 << i) > j) {
			k = 0;

			while ((1 << (4 - i)) > k) {
				km_rm[(((2 * k) + 1) << i) + j] =
					comp128v23_t0[comp128v23_t1[temp[(k << i) + j]] ^ (km_rm[(k << i) + 16 + j])];
				km_rm[(k << (i + 1)) + j] = temp[(k << i) + j];
				k++;
			}
			j++;
		}
	}

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 8; j++) {
			rand[i] = rand[i] ^ (((km_rm[(19 * (j + 8 * i) + 19) % 256 / 8] >> (3 * j + 3) % 8) & 1) << j);
		}
	}
}

/** Calculate comp128v2 or comp128v3 sres and kc from ki and rand
 *
 * @param[out] sres 4 byte value derived from ki and rand.
 * @param[out] kc 8 byte value derived from ki and rand.
 * @param[in] ki known only by the SIM and AuC (us in this case).
 * @param[in] rand 16 bytes of randomness.
 * @param[in] v2 if true we use version comp128-2 else we use comp128-3.

 */
void comp128v23(uint8_t *sres, uint8_t *kc, uint8_t const *ki, uint8_t const *rand, bool v2)
{
	uint8_t k_mix[16];
	uint8_t rand_mix[16];
	uint8_t katyvasz[16];
	uint8_t buffer[16];

	/* Every day IM suffling... */
	int i;

	for (i = 0; i < 8; i++) {
		k_mix[i] = ki[15 - i];
		k_mix[15 - i] = ki[i];
	}

	for (i = 0; i < 8; i++) {
		rand_mix[i] = rand[15 - i];
		rand_mix[15 - i] = rand[i];
	}

	for (i = 0; i < 16; i++) {
		katyvasz[i] = k_mix[i] ^ rand_mix[i];
	}

	for (i = 0; i < 8; i++) {
		_comp128v23(rand_mix, katyvasz);
	}

	for (i = 0; i < 16; i++) {
		buffer[i] = rand_mix[15 - i];
	}

	if (v2) {
		buffer[15] = 0x00;
		buffer[14] = 4 * (buffer[14] >> 2);
	}

	for (i = 0; i < 4; i++) {
		buffer[8 + i - 4] = buffer[8 + i];
		buffer[8 + i] = buffer[8 + i + 4];
	}

	/*
	 *	The algorithm uses 16 bytes until this point, but only 12 bytes are effective
	 *	also 12 bytes coming out from the SIM card.
	 */
	memcpy(sres, buffer, 4);
	memcpy(kc, buffer + 4, 8);
}

#if 0
#include <stdlib.h>
#include <ctype.h>
static int hextoint(char x)
{
	x = toupper(x);
	if (x >= 'A' && x <= 'F') {
		return x-'A' + 10;
	} else if (x >= '0' && x <= '9') {
		return x-'0';
	}

	fprintf(stderr, "Bad input.\n");

	exit(1);
}

int main(int argc, char **argv)
{
	uint8_t rand[16], key[16], sres[4], kc[8];
	int version;
	int i;

	if ((argc != 4) ||
	    (strlen(argv[1]) != 34) || (strlen(argv[2]) != 34) ||
	    (strncmp(argv[1], "0x", 2) != 0) || (strncmp(argv[2], "0x", 2) != 0) ||
	    !(version = atoi(argv[3]))) {
		error:
		fprintf(stderr, "Usage: %s 0x<key> 0x<rand> [1|2|3]\n", argv[0]);
		exit(1);
	}

	for (i = 0; i < 16; i++) {
		key[i] = (hextoint(argv[1][(2 * i) + 2]) << 4) | hextoint(argv[1][(2 * i) + 3]);
	}

	for (i = 0; i < 16; i++) {
		rand[i] = (hextoint(argv[2][(2 * i) + 2]) << 4) | hextoint(argv[2][(2 * i) + 3]);
	}

	switch (version) {
		case 3:
			comp128v23(sres, kc, key, rand, false);
			break;
		case 2:
			comp128v23(sres, kc, key, rand, true);
			break;
		case 1:
			comp128v1(sres, kc, key, rand);
			break;
		default:
			fprintf(stderr, "Invalid version, must be 1,2 or 3");
			goto error;
	}

	/* Output in vector format <Ki>,<rand>,<sres><Kc> */
	for (i = 0; i < 16; i++) {
		printf("%02X", key[i]);
	}
	printf(",");
	for (i = 0; i < 16; i++) {
		printf("%02X", rand[i]);
	}
	printf(",");
	for (i = 0; i < 4; i++) {
		printf("%02X", sres[i]);
	}
	for (i = 0; i < 8; i++) {
		printf("%02X", kc[i]);
	}
	printf("\n");

	return 0;
}
#endif
