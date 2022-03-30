/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Broadcom Corporation.
 */

/*
 * Bitfield operations
 *
 * These are generic bitfield operations which allow manipulation of variable
 * width bitfields within a word. One use of this would be to use data tables
 * to determine how to reprogram fields within R/W hardware registers.
 *
 * Example:
 *
 *            old_reg_val
 * +--------+----+---+--+-----+----------+
 * |        |    |   |  | old |          |
 * +--------+----+---+--+-----+----------+
 *
 *            new_reg_val
 * +--------+----+---+--+-----+----------+
 * |        |    |   |  | new |          |
 * +--------+----+---+--+-----+----------+
 *
 * mask = bitfield_mask(10, 5);
 * old = bitfield_extract(old_reg_val, 10, 5);
 * new_reg_val = bitfield_replace(old_reg_val, 10, 5, new);
 *
 * or
 *
 * mask = bitfield_mask(10, 5);
 * old = bitfield_extract_by_mask(old_reg_val, mask);
 * new_reg_val = bitfield_replace_by_mask(old_reg_val, mask, new);
 *
 * The numbers 10 and 5 could for example come from data
 * tables which describe all bitfields in all registers.
 */

#include <linux/types.h>

/* Produces a mask of set bits covering a range of a uint value */
static inline uint bitfield_mask(uint shift, uint width)
{
	return ((1 << width) - 1) << shift;
}

/* Extract the value of a bitfield found within a given register value */
static inline uint bitfield_extract(uint reg_val, uint shift, uint width)
{
	return (reg_val & bitfield_mask(shift, width)) >> shift;
}

/*
 * Replace the value of a bitfield found within a given register value
 * Returns the newly modified uint value with the replaced field.
 */
static inline uint bitfield_replace(uint reg_val, uint shift, uint width,
				    uint bitfield_val)
{
	uint mask = bitfield_mask(shift, width);

	return (reg_val & ~mask) | ((bitfield_val << shift) & mask);
}

/* Produces a shift of the bitfield given a mask */
static inline uint bitfield_shift(uint mask)
{
	return mask ? ffs(mask) - 1 : 0;
}

/* Extract the value of a bitfield found within a given register value */
static inline uint bitfield_extract_by_mask(uint reg_val, uint mask)
{
	uint shift = bitfield_shift(mask);

	return (reg_val & mask) >> shift;
}

/*
 * Replace the value of a bitfield found within a given register value
 * Returns the newly modified uint value with the replaced field.
 */
static inline uint bitfield_replace_by_mask(uint reg_val, uint mask,
					    uint bitfield_val)
{
	uint shift = bitfield_shift(mask);

	return (reg_val & ~mask) | ((bitfield_val << shift) & mask);
}
