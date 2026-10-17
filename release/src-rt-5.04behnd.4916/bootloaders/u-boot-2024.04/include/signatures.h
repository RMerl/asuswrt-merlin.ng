/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __SIGNATURES_H__
#define __SIGNATURES_H__

/**
 * Returns a 16-bit signature built from 2 ASCII characters.
 *
 * This macro returns a 16-bit value built from the two ASCII characters
 * specified by A and B.
 *
 * @A: The first ASCII character.
 * @B: The second ASCII character.
 *
 * @return: A 16-bit value built from the two ASCII characters specified by
 *          A and B.
 */
#define SIGNATURE_16(A, B)	((A) | (B << 8))

/**
 * Returns a 32-bit signature built from 4 ASCII characters.
 *
 * This macro returns a 32-bit value built from the four ASCII characters
 * specified by A, B, C, and D.
 *
 * @A: The first ASCII character.
 * @B: The second ASCII character.
 * @C: The third ASCII character.
 * @D: The fourth ASCII character.
 *
 * @return: A 32-bit value built from the two ASCII characters specified by
 *          A, B, C and D.
 */
#define SIGNATURE_32(A, B, C, D)	\
	(SIGNATURE_16(A, B) | (SIGNATURE_16(C, D) << 16))

/**
 * Returns a 64-bit signature built from 8 ASCII characters.
 *
 * This macro returns a 64-bit value built from the eight ASCII characters
 * specified by A, B, C, D, E, F, G,and H.
 *
 * @A: The first ASCII character.
 * @B: The second ASCII character.
 * @C: The third ASCII character.
 * @D: The fourth ASCII character.
 * @E: The fifth ASCII character.
 * @F: The sixth ASCII character.
 * @G: The seventh ASCII character.
 * @H: The eighth ASCII character.
 *
 * @return: A 64-bit value built from the two ASCII characters specified by
 *          A, B, C, D, E, F, G and H.
 */
#define SIGNATURE_64(A, B, C, D, E, F, G, H)	\
	(SIGNATURE_32(A, B, C, D) | ((u64)(SIGNATURE_32(E, F, G, H)) << 32))

#endif /* __SIGNATURES_H__ */
