// SPDX-License-Identifier: BSD-3-Clause
/*
 * This file is copied from the coreboot repository as part of
 * the libpayload project:
 *
 * Copyright 2014 Google Inc.
 */

#include <common.h>

union overlay64 {
	u64 longw;
	struct {
		u32 lower;
		u32 higher;
	} words;
};

u64 __ashldi3(u64 num, unsigned int shift)
{
	union overlay64 output;

	output.longw = num;
	if (shift >= 32) {
		output.words.higher = output.words.lower << (shift - 32);
		output.words.lower = 0;
	} else {
		if (!shift)
			return num;
		output.words.higher = (output.words.higher << shift) |
			(output.words.lower >> (32 - shift));
		output.words.lower = output.words.lower << shift;
	}
	return output.longw;
}

u64 __lshrdi3(u64 num, unsigned int shift)
{
	union overlay64 output;

	output.longw = num;
	if (shift >= 32) {
		output.words.lower = output.words.higher >> (shift - 32);
		output.words.higher = 0;
	} else {
		if (!shift)
			return num;
		output.words.lower = output.words.lower >> shift |
			(output.words.higher << (32 - shift));
		output.words.higher = output.words.higher >> shift;
	}
	return output.longw;
}

#define MAX_32BIT_UINT ((((u64)1) << 32) - 1)

static u64 _64bit_divide(u64 dividend, u64 divider, u64 *rem_p)
{
	u64 result = 0;

	/*
	 * If divider is zero - let the rest of the system care about the
	 * exception.
	 */
	if (!divider)
		return 1 / (u32)divider;

	/* As an optimization, let's not use 64 bit division unless we must. */
	if (dividend <= MAX_32BIT_UINT) {
		if (divider > MAX_32BIT_UINT) {
			result = 0;
			if (rem_p)
				*rem_p = divider;
		} else {
			result = (u32)dividend / (u32)divider;
			if (rem_p)
				*rem_p = (u32)dividend % (u32)divider;
		}
		return result;
	}

	while (divider <= dividend) {
		u64 locald = divider;
		u64 limit = __lshrdi3(dividend, 1);
		int shifts = 0;

		while (locald <= limit) {
			shifts++;
			locald = locald + locald;
		}
		result |= __ashldi3(1, shifts);
		dividend -= locald;
	}

	if (rem_p)
		*rem_p = dividend;

	return result;
}

u64 __udivdi3(u64 num, u64 den)
{
	return _64bit_divide(num, den, NULL);
}

u64 __umoddi3(u64 num, u64 den)
{
	u64 v = 0;

	_64bit_divide(num, den, &v);
	return v;
}
