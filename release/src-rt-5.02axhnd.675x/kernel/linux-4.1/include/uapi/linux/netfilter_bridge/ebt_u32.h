/*
 *  ebt_u32
 *
 *	Authors:
 *  extend by Broadcom at Jan 24, 2019
 *
 *
 */

#ifndef __LINUX_BRIDGE_EBT_U32_H
#define __LINUX_BRIDGE_EBT_U32_H

#include <linux/types.h>

enum ebt_u32_ops {
	EBT_U32_AND,
	EBT_U32_LEFTSH,
	EBT_U32_RIGHTSH,
	EBT_U32_AT,
};

struct ebt_u32_location_element {
	__u32 number;
	__u8 nextop;
};

struct ebt_u32_value_element {
	__u32 min;
	__u32 max;
};

/*
 * Any way to allow for an arbitrary number of elements?
 * For now, I settle with a limit of 10 each.
 */
#define EBT_U32_MAXSIZE 10

#define EBT_U32_MATCH "u32"


struct ebt_u32_test {
	struct ebt_u32_location_element location[EBT_U32_MAXSIZE+1];
	struct ebt_u32_value_element value[EBT_U32_MAXSIZE+1];
	__u8 nnums;
	__u8 nvalues;
};

struct ebt_u32_info {
	struct ebt_u32_test tests[EBT_U32_MAXSIZE+1];
	__u8 ntests;
	__u8 invert;
};

#endif

