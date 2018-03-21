#include <tommath_private.h>
#ifdef BN_MP_IMPORT_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is a library that provides multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library was designed directly after the MPI library by
 * Michael Fromberger but has been written from scratch with
 * additional optimizations in place.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tstdenis82@gmail.com, http://libtom.org
 */

/* based on gmp's mpz_import.
 * see http://gmplib.org/manual/Integer-Import-and-Export.html
 */
int mp_import(mp_int* rop, size_t count, int order, size_t size, 
                            int endian, size_t nails, const void* op) {
	int result;
	size_t odd_nails, nail_bytes, i, j;
	unsigned char odd_nail_mask;

	mp_zero(rop);

	if (endian == 0) {
		union {
			unsigned int i;
			char c[4];
		} lint;
		lint.i = 0x01020304;

		endian = (lint.c[0] == 4) ? -1 : 1;
	}

	odd_nails = (nails % 8);
	odd_nail_mask = 0xff;
	for (i = 0; i < odd_nails; ++i) {
		odd_nail_mask ^= (1 << (7 - i));
	}
	nail_bytes = nails / 8;

	for (i = 0; i < count; ++i) {
		for (j = 0; j < (size - nail_bytes); ++j) {
			unsigned char byte = *(
					(unsigned char*)op + 
					(((order == 1) ? i : ((count - 1) - i)) * size) +
					((endian == 1) ? (j + nail_bytes) : (((size - 1) - j) - nail_bytes))
				);

			if (
				(result = mp_mul_2d(rop, ((j == 0) ? (8 - odd_nails) : 8), rop)) != MP_OKAY) {
				return result;
			}

			rop->dp[0] |= (j == 0) ? (byte & odd_nail_mask) : byte;
			rop->used  += 1;
		}
	}

	mp_clamp(rop);

	return MP_OKAY;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
