/*
 * Math component interface file.
 * API for 64 bit calculation and for combined 32 bit/64 bit calculation.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id:$
 */

#ifndef _bcm_math64_h_
#define _bcm_math64_h_

#include <typedefs.h>

/* ============================================
 * Basic calculation routines
 * ============================================
 */

/* Calculates a + b where a is 64 bit and b is 32 bit (64 bit + 32 bit -> 64 bit).
 *
 * r_hi - pointer to high 32 bits of a and of the result of a+b
 * r_lo - pointer to low 32 bits of a and of the result of a+b
 * offset - the value of b
 */
void math_add_64(uint32 *r_hi, uint32 *r_lo, uint32 offset);

/* Calculates a - b where a is 64 bit and b is 32 bit (64 bit - 32 bit -> 64 bit).
 *
 * r_hi - pointer to high 32 bits of a and of the result of a-b
 * r_lo - pointer to low 32 bits of a and of the result of a-b
 * offset - the value of b
 */
void math_sub_64(uint32 *r_hi, uint32 *r_lo, uint32 offset);

/* Calculates a * b + c (32 bit * 32 bit + 32 bit -> 64 bit).
 *
 * r_high - pointer to the high 32 bits of the result of a*b+c
 * r_low - pointer to the low 32 bits of the result of a*b+c
 * a - unsigned 32 bit number
 * b - unsigned 32 bit number
 * c - unsigned 32 bit number
 */
void math_uint64_multiple_add(uint32 *r_high, uint32 *r_low, uint32 a, uint32 b, uint32 c);

/* Calculates a / b (64 bit / 32 bit -> 32 bit).
 *
 * r - pointer to result of a/b (32 bit quotient)
 * a_high - high 32 bits of a (64 bit dividend)
 * a_low - low 32 bits of a (64 bit dividend)
 * b - unsigned 32 bit divisor
 */
void math_uint64_divide(uint32 *r, uint32 a_high, uint32 a_low, uint32 b);

/* Calculates a / b (64 bit / 32 bit -> 64 bit).
 *
 * a_high - high 32 bits of a (64 bit dividend)
 * a_low - low 32 bits of a (64 bit dividend)
 * b - unsigned 32 bit divisor
 * return - result of a/b (64 bit quotient)
 */
uint64 math_div_64(uint32 a_high, uint32 a_low, uint32 b);

/* Calculates a >> b. Returns only lower 32 bits (64 bit >> 32 bit -> 32 bit).
 *
 * r - pointer to 32 bit result of a>>b
 * a_high - high 32 bits of a
 * a_low - low 32 bits of a
 * b - the number of bits to shift right
 */
void math_uint64_right_shift(uint32 *r, uint32 a_high, uint32 a_low, uint32 b);

/* Does left shift of unsigned 64 bit number (64 bit << 32 bit -> 64 bit).
 *
 * num - unsigned 64 bit number
 * shift_amt - the number of bits to shift left
 * return - 64 bit result of num<<shift_amt
 */
uint64 math_shl_64(uint64 num, uint8 shift_amt);

/* Does right shift of unsigned 64 bit number (64 bit << 32 bit -> 64 bit).
 *
 * num - unsigned 64 bit number
 * shift_amt - the number of bits to shift right
 * return - 64 bit result of num>>shift_amt
 */
uint64 math_shr_64(uint64 num, uint8 shift_amt);

/* ============================================
 * Fixed point routines
 * ============================================
 */

/* Does unsigned 64 bit fixed point multiplication (64 bit * 64 bit -> 64 bit). */
uint64 math_fp_mult_64(uint64 val1, uint64 val2, uint8 nf1, uint8 nf2, uint8 nf_res);

/* Does unsigned 64 bit by 32 bit fixed point division. */
uint8 math_fp_div_64(uint64 num, uint32 den, uint8 nf_num, uint8 nf_den, uint32 *div_out);

/* Finds the number of bits available for shifting in unsigned 64 bit number. */
uint8 math_fp_calc_head_room_64(uint64 num);

/* Does unsigned 64 bit fixed point floor */
uint32 math_fp_floor_64(uint64 num, uint8 floor_pos);

/* Does unsigned 64 bit fixed point rounding */
uint32 math_fp_round_64(uint64 num, uint8 rnd_pos);

/* Does unsigned 64 bit fixed point ceiling */
uint32 math_fp_ceil_64(uint64 num, uint8 ceil_pos);

#endif /* _bcm_math64_h_ */
