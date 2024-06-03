/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef __ACCESS_MACROS_H_INCLUDED
#define __ACCESS_MACROS_H_INCLUDED

#include <asm/byteorder.h>
#include <asm/io.h>
#include <linux/types.h>

#define swap2bytes cpu_to_be16
#define swap4bytes cpu_to_be32

#define DEVICE_ADDRESS(_a) ((volatile uint8_t * const)(_a))

#define MEMSET(a, v, sz)                memset_io(a, v, sz)

#define WMB()  /* */

#define VAL32(_a) (*(volatile uint32_t *)(DEVICE_ADDRESS(_a)))
#define READ_32(a, r) (*(volatile uint32_t *)&(r) = readl(DEVICE_ADDRESS(a)))
#define WRITE_32(a, r) writel(*(uint32_t *)&(r), DEVICE_ADDRESS(a))

#define MGET_8(a)               readb(a)
#define MGET_16(a)              swap2bytes(readw(a))
#define MGET_32(a)              swap4bytes(readl(a))

#define MWRITE_8(a, r) writeb((uint8_t)(r), a)
#define MWRITE_16(a, r) writew(swap2bytes((uint16_t)(r)), a)
#define MWRITE_32(a, r) MWRITE_I_32(a, 0, r)
#define MWRITE_I_32(a, i, r) \
	do {\
		(*((volatile uint32_t *)(a) + (i)) = swap4bytes((uint32_t)(r))); \
	} while (0)

#define MREAD_8(a, r)           ((r) = MGET_8(a))
#define MREAD_16(a, r)          ((r) = MGET_16(a))
#define MREAD_32(a, r)          ((r) = MGET_32(a))

#define MEMSET_32(a, v_32, sz_32)  \
	do { \
		int __i;\
		for (__i=0; __i<sz_32; __i++)\
		MWRITE_I_32(a, __i, v_32);\
	} while (0)

/**************************/
/* Bit-field access macros
: v     -  value
: lsbn  - ls_bit_number
: fw    - field_width
: a     - address
: rv    - read_value      */
/**************************/
#define RDP_FIELD_GET(v, lsbn, fw)          (((v)>>(lsbn)) & ((unsigned)(1 << (fw)) - 1))
#define RDP_FIELD_MGET_32(a, lsbn, fw)      (RDP_FIELD_GET(MGET_32(a), (lsbn), (fw)))
#define RDP_FIELD_MGET_16(a, lsbn, fw)      (RDP_FIELD_GET(MGET_16(a), (lsbn), (fw)))
#define RDP_FIELD_MGET_8(a, lsbn, fw)       (RDP_FIELD_GET(MGET_8(a) , (lsbn), (fw)))

#define RDP_FIELD_MREAD_16(a, lsbn, fw, rv) (rv = RDP_FIELD_MGET_16((a),   (lsbn), (fw)))

#define RDP_FIELD_SET(value, ls_bit_number, field_width, write_value) \
	do { \
		uint32_t  mask; \
		mask = ((1 << (field_width)) - 1) << (ls_bit_number); \
		value &=  ~mask; \
		value |= (write_value) << (ls_bit_number); \
	} while (0)

#define RDP_FIELD_MWRITE_32(address, ls_bit_number, field_width, write_value) \
	do { \
		uint32_t current_value = MGET_32(address); \
		RDP_FIELD_SET(current_value, ls_bit_number, field_width, write_value); \
		MWRITE_32(address, current_value); \
	} while (0)

#define RDP_FIELD_MWRITE_16(address, ls_bit_number, field_width, write_value) \
	do { \
		uint16_t current_value = MGET_16(address); \
		RDP_FIELD_SET(current_value, ls_bit_number, field_width, write_value); \
		MWRITE_16(address, current_value); \
	} while (0)

#define RDP_FIELD_MWRITE_8(address, ls_bit_number, field_width, write_value) \
	do { \
		uint8_t  current_value = MGET_8(address); \
		RDP_FIELD_SET(current_value, ls_bit_number, field_width, write_value); \
		MWRITE_8(address, current_value); \
	} while (0)

#endif

