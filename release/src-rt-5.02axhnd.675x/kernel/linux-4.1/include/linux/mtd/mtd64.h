#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
 * drivers/mtd/mtd64.h
 *
 <:copyright-BRCM:2012:DUAL/GPL:standard
 
    Copyright (c) 2012 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
 :> 
 *
 * Data structures for MTD 64 bit ops (Borrowed heavily from broadcom NAND
 * controller brcmnand_priv.h)
 * 
 * when		who		what
 * 20080805	sidc		Original coding
 */

#ifndef _MTD64_H_
#define _MTD64_H_

#include <generated/autoconf.h>

/*
 * 64 bit arithmetics 
 */
#include <asm-generic/gcclib.h>
#include <linux/bitmap.h>

#define LONGLONG_TO_BITS (sizeof(uint64_t)*BITS_PER_UNIT)

/*
 * Create a 64 bit number out of a 32 bit 
 */
static inline int64_t mtd64_constructor(long hi, unsigned long low) 
{
	DIunion ull;

	ull.s.high = hi;
	ull.s.low = low;

	return ull.ll;
}


/*
 * Allow inline printing of 64 bit integer values
 */
static inline char *mtd64_sprintf(char* msg, int64_t offset)
{
	DIunion llw;

	llw.ll = offset;
	sprintf(msg, "%08x:%08x", llw.s.high, llw.s.low);
	return msg;
}

static inline int mtd64_is_positive(int64_t ll)
{
	DIunion u;

	u.ll = ll;
	return ((int) u.s.high > 0 || (((int) u.s.high) == 0 && ((unsigned int) u.s.low) > 0));
}

static inline int mtd64_is_positiveorzero(int64_t ll)
{
	DIunion u;

	u.ll = ll;
	return ((int) u.s.high >= 0 || (((int) u.s.high) == 0 && ((unsigned int) u.s.low) >= 0));
}

/*
 * Returns low DWord
 */
static inline uint32_t mtd64_ll_low(int64_t ll)
{
	DIunion ull;

	ull.ll = ll;
	return (uint32_t) ull.s.low;
}

/*
 * Returns high DWord
 */
static inline int32_t mtd64_ll_high(int64_t ll)
{
	DIunion ull;

	ull.ll = ll;
	return (int32_t) ull.s.high;
}
  
static inline int mtd64_ll_ffs(uint64_t ll)
{
	DIunion ull;
	int res;

	ull.ll = ll;
	res = ffs(ull.s.low);
	if (res)
		return res;
	res = ffs(ull.s.high);
	return (32 + res);
}

#if 0
/*
 * Returns (ll >> shift)
 */
static inline uint64_t mtd64_rshft(uint64_t ll, int shift)
{
	DIunion src, res;

	src.ll = ll;
	bitmap_shift_right((unsigned long*) &res, (unsigned long*) &src, shift, LONGLONG_TO_BITS);
	return res.ll;
}
#define mtd64_rshft32(ll,s) mtd64_rshft(ll, s)

/*
 * Returns (ul << shift) with ul a 32-bit unsigned integer.  Returned value is a 64bit integer
 */
static inline uint64_t mtd64_lshft32(uint64_t ll, int shift)
{
	DIunion src, res;

	src.ll = ll;
	bitmap_shift_left((unsigned long*) &res, (unsigned long*) &src, shift, LONGLONG_TO_BITS);
	return res.ll;
}

/* 
 * returns (left + right)
 */
static inline int64_t mtd64_add(int64_t left, int64_t right)
{
	DIunion l, r, sum;

	l.ll = left;
	r.ll = right;

	add_ssaaaa(sum.s.high, sum.s.low, l.s.high, l.s.low, r.s.high, r.s.low);
	return sum.ll;
}

/*
 * returns (left + right), with right being a 32-bit integer
 */
static inline int64_t mtd64_add32(int64_t left, int right)
{
	DIunion l, r, sum;

	l.ll = left;
	r.s.high = 0;
	r.s.low = right;

	add_ssaaaa(sum.s.high, sum.s.low, l.s.high, l.s.low, r.s.high, r.s.low);
	return sum.ll;
}

/*
 * returns (left - right)
 */
static inline int64_t mtd64_sub(int64_t left, int64_t right)
{
	DIunion l, r, diff;

	l.ll = left;
	r.ll = right;

	sub_ddmmss(diff.s.high, diff.s.low, l.s.high, l.s.low, r.s.high, r.s.low);
	return diff.ll;
}

/*
 * returns (left - right)
 */
static inline int64_t mtd64_sub32(int64_t left, int  right)
{
	DIunion l, r, diff;

	l.ll = left;
	r.s.low = right;
	r.s.high = 0;

	sub_ddmmss(diff.s.high, diff.s.low, l.s.high, l.s.low, r.s.high, r.s.low);
	return diff.ll;
}

static inline int mtd64_notequals(int64_t left, int64_t right)
{
	DIunion l, r;

	l.ll = left;
	r.ll = right;

	if (l.s.high == r.s.high && l.s.low == r.s.low) 
		return 0;
	return 1;
}

static inline int mtd64_equals(int64_t left, int64_t right)
{
	DIunion l, r;

	l.ll = left;
	r.ll = right;

	if (l.s.high == r.s.high && l.s.low == r.s.low) 
		return 1;
	return 0;
}

static inline int mtd64_is_greater(int64_t left, int64_t right)
{
	return mtd64_is_positive(mtd64_sub(left, right));
}

static inline int mtd64_is_gteq(int64_t left, int64_t right)
{
	return mtd64_is_positiveorzero(mtd64_sub(left, right));
}

static inline int mtd64_is_less(int64_t left, int64_t right)
{
	return mtd64_is_positive(mtd64_sub(right, left));
}

static inline int mtd64_is_lteq(int64_t left, int64_t right)
{
	return mtd64_is_positiveorzero(mtd64_sub(right, left));
}

/*
 * Returns (left & right)
 */
static inline uint64_t mtd64_and(uint64_t left, uint64_t right)
{
	uint64_t res;
	bitmap_and((unsigned long*) &res, (unsigned long*) &left, (unsigned long*) &right, LONGLONG_TO_BITS);
	return res;
}

static inline uint64_t mtd64_or(uint64_t left, uint64_t right)
{
	uint64_t res;
	bitmap_or((unsigned long *) &res, (unsigned long *) &left, (unsigned long *) &right, LONGLONG_TO_BITS);
	return res;
}

/*
 * Multiply 2 32-bit integer, result is 64bit
 */
static inline uint64_t mtd64_mul(unsigned int left, unsigned int right)
{
	DIunion llw;
	
	umul_ppmm(llw.s.high, llw.s.low, left, right);
	return llw.ll;
}

/*
 * res 		Result
 * high:low 	u64 bit 
 * base		Divisor
 * rem		Remainder
 */
#define do_mtd_div64_32(res, high, low, base, rem) ({ \
        unsigned long __quot, __mod; \
        unsigned long __cf, __tmp, __tmp2, __i; \
        \
        __asm__(".set   push\n\t" \
                ".set   noat\n\t" \
                ".set   noreorder\n\t" \
                "move   %2, $0\n\t" \
                "move   %3, $0\n\t" \
                "b      1f\n\t" \
                " li    %4, 0x21\n" \
                "0:\n\t" \
                "sll    $1, %0, 0x1\n\t" \
                "srl    %3, %0, 0x1f\n\t" \
                "or     %0, $1, %5\n\t" \
                "sll    %1, %1, 0x1\n\t" \
                "sll    %2, %2, 0x1\n" \
                "1:\n\t" \
                "bnez   %3, 2f\n\t" \
                " sltu  %5, %0, %z6\n\t" \
                "bnez   %5, 3f\n" \
                "2:\n\t" \
                " addiu %4, %4, -1\n\t" \
                "subu   %0, %0, %z6\n\t" \
                "addiu  %2, %2, 1\n" \
                "3:\n\t" \
                "bnez   %4, 0b\n\t" \
                " srl   %5, %1, 0x1f\n\t" \
                ".set   pop" \
                : "=&r" (__mod), "=&r" (__tmp), "=&r" (__quot), "=&r" (__cf), \
                  "=&r" (__i), "=&r" (__tmp2) \
                : "Jr" (base), "0" (high), "1" (low)); \
        \
        (res) = __quot; \
        (rem) = __mod; \
        __quot; })

#endif
#endif
#endif // CONFIG_BCM_KF_MTD_BCMNAND
