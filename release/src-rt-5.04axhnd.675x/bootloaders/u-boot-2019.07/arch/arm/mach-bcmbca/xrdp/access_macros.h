// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */
/*
	
*/

#ifndef __ACCESS_MACROS_H_INCLUDED
#define __ACCESS_MACROS_H_INCLUDED

#include <linux/types.h>
#include <linux/io.h>
#include <asm/barriers.h>
#include "access_logging.h"

/*****************/
/* Generic swaps */
/*****************/
static inline uint16_t __swap2bytes(uint16_t a)
{
	return (a << 8) | (a >> 8);
}
static inline uint32_t __swap4bytes(uint32_t a)
{
	return (a << 24) | ((a & 0xFF00) << 8) |
		((a & 0xFF0000) >> 8) | (a >> 24);
}

static inline uint64_t __swap4bytes64(uint64_t a)
{
	return __swap4bytes(a) |
		((uint64_t)__swap4bytes((a>>32) & 0xFFFFFFFF) << 32);
}

/**************************/
/* swap2bytes, swap4bytes */
/**************************/
#if defined(_BYTE_ORDER_LITTLE_ENDIAN_)
#if defined(CONFIG_ARM)
static inline uint16_t swap2bytes(uint16_t a)
{
	__asm__("rev16 %0, %1" : "=r" (a) : "r" (a));
	return a;
}

#if defined(CONFIG_ARM64)
static inline uint32_t swap4bytes(uint32_t a)
{
	__asm__("rev32 %0, %1" : "=r" (a) : "r" (a));
	return a;
}

/* reverses the 4 bytes in each 32-bit element of Xm*/
static inline uint64_t swap4bytes64(uint64_t a)
{
	__asm__("rev32 %0, %1" : "=r" (a) : "r" (a));
	return a;
}
#else
static inline uint32_t swap4bytes(uint32_t a)
{
	__asm__("rev %0, %1" : "=r" (a) : "r" (a));
	return a;
}
#endif
#else
#define swap2bytes(x) __swap2bytes(x)
#define swap4bytes(x) __swap4bytes(x)
#define swap4bytes64(x) __swap4bytes64(x)
#endif /* CONFIG_ARM */
#else /* _BYTE_ORDER_LITTLE_ENDIAN_ */
#define swap2bytes(x) (x)
#define swap4bytes(x) (x)
#define swap4bytes64(x) (x)
#endif /* _BYTE_ORDER_LITTLE_ENDIAN_ */

#define SWAPBYTES(_buffer, _len) \
	do { \
		uint8_t _i; \
		for (_i = 0; _i < _len; _i += sizeof(uint32_t)) \
			*((uint32_t *)(&(_buffer[_i]))) = \
				swap4bytes(*((uint32_t *)(&(_buffer[_i])))); \
	} while (0)

/******************/
/* Device address */
/******************/
#define DEVICE_ADDRESS(_a) ((volatile uint8_t * const)((uintptr_t)(_a)))

/**********/
/* MEMSET */
/**********/
#if defined(DUAL_ISSUE)
#define MEMSET(a, v, sz)	_xrdp__memset(a, v, sz)
#else
#define MEMSET(a, v, sz)	memset_io(a, v, sz)
#endif

/******************/
/* Memory Barrier */
/******************/
#define WMB()	__iowmb() /* memory barrier */
#define RMB()	__iormb() /* memory barrier */

/************************/
/* Registers and memory */
/************************/

/* Registers */
#define VAL32(_a) (*(volatile uint32_t *)(DEVICE_ADDRESS(_a)))
#define READ_8(a, r) \
	(*(volatile uint8_t *)&(r) = *(volatile uint8_t *)DEVICE_ADDRESS(a))
#define READ_16(a, r) \
	(*(volatile uint16_t *)&(r) = *(volatile uint16_t *)DEVICE_ADDRESS(a))
#define READ_32(a, r) \
	(*(volatile uint32_t *)&(r) = *(volatile uint32_t *)DEVICE_ADDRESS(a))

#if defined(XRDP_SIMPLE_NET)
/*this version use read modify write method to save writes in CFE GPL mode */
#define WRITE_8(a, r) \
	do { \
		uint8_t temp; \
		uint8_t temp1 = *(uint8_t *)&r; \
		READ_8(a, temp); \
		if ((temp != temp1) || (temp != 0)) { \
			(*(volatile uint8_t *)DEVICE_ADDRESS(a) = *(uint8_t *)&(r)); \
			ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint8_t *)&(r), 1); \
		} \
	} while (0)
#define WRITE_16(a, r) \
	do { \
		uint16_t temp; \
		uint16_t temp1 = *(uint16_t *)&r; \
		READ_16(a, temp); \
		if ((temp != temp1) || (temp != 0)) { \
			(*(volatile uint16_t *)DEVICE_ADDRESS(a) = *(uint16_t *)&(r)); \
			ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint16_t *)&(r), 2); \
		} \
	} while (0)
#define WRITE_32(a, r) \
	do { \
		uint32_t temp; \
		uint32_t temp1 = *(uint32_t *)&r; \
		READ_32(a, temp); \
		if ((temp != temp1) || (temp != 0)) { \
			(*(volatile uint32_t *)DEVICE_ADDRESS(a) = *(uint32_t *)&(r)); \
			ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint32_t *)&(r), 4); \
		} \
	} while (0)
#else
#define WRITE_8(a, r) \
	do { \
		(*(volatile uint8_t *)DEVICE_ADDRESS(a) = *(uint8_t *)&(r)); \
		ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint8_t *)&(r), 1); \
	} while (0)
#define WRITE_16(a, r) \
	do { \
		(*(volatile uint16_t *)DEVICE_ADDRESS(a) = *(uint16_t *)&(r)); \
		ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint16_t *)&(r), 2); \
	} while (0)
#define WRITE_32(a, r) \
	do { \
		(*(volatile uint32_t *)DEVICE_ADDRESS(a) = *(uint32_t *)&(r)); \
		ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint32_t *)&(r), 4); \
	} while (0)
#endif

#define READ_I_8(a, i, r) (*(volatile uint8_t *)&(r) = *((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)))
#define READ_I_16(a, i, r) (*(volatile uint16_t *)&(r) = *((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)))
#define READ_I_32(a, i, r) (*(volatile uint32_t *)&(r) = *((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)))

#define WRITE_I_8(a, i, r) \
	do { \
		(*((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)) = *(uint8_t *)&(r)); \
		ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint8_t *)DEVICE_ADDRESS(a) + (i), *(uint8_t *)&(r), 1); \
	} while (0)
#define WRITE_I_16(a, i, r) \
	do { \
		(*((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)) = *(uint16_t *)&(r)); \
		ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint16_t *)DEVICE_ADDRESS(a) + (i), *(uint16_t *)&(r), 2); \
	} while (0)
#define WRITE_I_32(a, i, r) \
	do { \
		(*((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)) = *(uint32_t *)&(r)); \
		ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint32_t *)DEVICE_ADDRESS(a) + (i), *(uint32_t *)&(r), 4); \
	} while (0)


/* Memory */
#define MGET_8(a)	(*(volatile uint8_t *)(a))
#define MGET_16(a)	swap2bytes(*(volatile uint16_t *)(a))
#define MGET_32(a)	swap4bytes(*(volatile uint32_t *)(a))

#define MGET_I_8(a, i)	(*((volatile uint8_t *)(a) + (i)))
#define MGET_I_16(a, i)	swap2bytes(*((volatile uint16_t *)(a) + (i)))
#define MGET_I_32(a, i)	swap4bytes(*((volatile uint32_t *)(a) + (i)))

#define MWRITE_8(a, r) \
	do { \
		(*(volatile uint8_t *)(a) = (uint8_t)(r)); \
		ACCESS_LOG(ACCESS_LOG_OP_MWRITE, (a), (r), 1); \
	} while (0)
#define MWRITE_16(a, r) \
	do { \
		(*(volatile uint16_t *)(a) = swap2bytes((uint16_t)(r))); \
		ACCESS_LOG(ACCESS_LOG_OP_MWRITE, (a), swap2bytes((uint16_t)(r)), 2); \
	} while (0)

#define MWRITE_32(a, r) \
	do { \
		(*(volatile uint32_t *)(a) = swap4bytes((uint32_t)(r))); \
		ACCESS_LOG(ACCESS_LOG_OP_MWRITE, (a), swap4bytes((uint32_t)(r)), 4); \
	} while (0)

#define MWRITE_I_8(a, i, r) \
	do { \
		(*((volatile uint8_t *)(a) + (i)) = (uint8_t)(r)); \
		ACCESS_LOG(ACCESS_LOG_OP_MWRITE, ((volatile uint8_t *)(a) + (i)), (r), 1); \
	} while (0)
#define MWRITE_I_16(a, i, r) \
	do { \
		(*((volatile uint16_t *)(a) + (i)) = swap2bytes((uint16_t)(r))); \
		ACCESS_LOG(ACCESS_LOG_OP_MWRITE, ((volatile uint16_t *)(a) + (i)), swap2bytes((uint16_t)(r)), 2); \
	} while (0)
#define MWRITE_I_32(a, i, r) \
	do { \
		(*((volatile uint32_t *)(a) + (i)) = swap4bytes((uint32_t)(r))); \
		ACCESS_LOG(ACCESS_LOG_OP_MWRITE, ((volatile uint32_t *)(a) + (i)), swap4bytes((uint32_t)(r)), 4); \
	} while (0)
#define MWRITE_I_32_NOLOG(a, i, r) \
	do { \
		(*((volatile uint32_t *)(a) + (i)) = swap4bytes((uint32_t)(r))); \
	} while (0)

#if defined(CONFIG_ARM64)
#define MWRITE_I_64(a, i, r)	(*((volatile uint64_t *)(a) + (i)) = swap4bytes64((uint64_t)(r)))
#endif

#define MREAD_8(a, r)	((r) = MGET_8(a))
#define MREAD_16(a, r)	((r) = MGET_16(a))
#define MREAD_32(a, r)	((r) = MGET_32(a))

#define MREAD_I_8(a, i, r)	((r) = MGET_I_8((a), (i)))
#define MREAD_I_16(a, i, r)	((r) = MGET_I_16((a), (i)))
#define MREAD_I_32(a, i, r)	((r) = MGET_I_32((a), (i)))

#if defined(DUAL_ISSUE)
#define MREAD_BLK_8(d, s, sz)	_xrdp__memcpy(d, s, sz)
#define MREAD_BLK_16(d, s, sz)	_xrdp__memcpy(d, s, sz)
#define MREAD_BLK_32(d, s, sz)	_xrdp__memcpy(d, s, sz)
#else
#define MREAD_BLK_8(d, s, sz)	memcpy_fromio(d, s, sz)
#define MREAD_BLK_16(d, s, sz)	memcpy_fromio(d, s, sz)
#define MREAD_BLK_32(d, s, sz)	memcpy_fromio(d, s, sz)
#endif


/*************/
/* MEMSET_32 */
/*************/
#define MEMSET_32(a, v_32, sz_32) \
	do { \
		int __i; \
		for (__i=0; __i<sz_32; __i++) \
			MWRITE_I_32_NOLOG(a, __i, v_32); \
		ACCESS_LOG(ACCESS_LOG_OP_MEMSET_32, a, v_32, sz_32); \
	} while (0)

#if defined(_CFE_) || defined(RDP_SIM)
#define MWRITE_BLK_8(d, s, sz) memcpy(d, s, sz)
#else
#if defined(CONFIG_BCM6878) || defined(CONFIG_BCM63146)
#define MWRITE_BLK_8(d, s, sz) \
	do { \
		uint32_t i, val; \
		for (i = 0; i < (sz); i++) { \
			val = *((volatile uint8_t *)(s) + (i)); \
			MWRITE_I_8((uint32_t)(d), i, val); \
		} \
	} while (0)
#else
#if defined(CONFIG_ARM64)
#define MWRITE_BLK_8(d, s, sz) memcpy_toio(d, s, sz)
#else
#define MWRITE_BLK_8(d, s, sz) memcpy(d, s, sz)
#endif
#endif
#endif

#define MWRITE_BLK_16(d, s, sz) \
	do { \
		uint32_t i, val; \
		for (i = 0; i < (sz / 2); i++) { \
			val = *((volatile uint16_t *)(s) + (i)); \
			MWRITE_I_16(d, i, val); \
		} \
	} while (0)
#define MWRITE_BLK_32(d, s, sz) \
	do { \
		uint32_t i, val; \
		for (i = 0; i < (sz / 4); i++) { \
			val = *((volatile uint32_t *)(s) + (i)); \
			MWRITE_I_32(d, i, val); \
		} \
	} while (0)

#define MWRITE_BLK_32_SWAP(d, s, sz) \
	do { \
		uint32_t i, val; \
		for ( i = 0; i < ( sz / 4 ); i++ ) { \
			val = *((volatile uint32_t*)(s) + (i)); \
			val = swap4bytes(val); MWRITE_I_32( d, i, val ); \
		} \
	} while (0)

#define MWRITE_BLK_16_SWAP(d, s, sz) \
	do { \
		uint32_t i, val; \
		for (i = 0; i < (sz / 2); i++) { \
			val = *((volatile uint16_t *)(s) + (i)); \
			val = swap2bytes(val); \
			MWRITE_I_16((uint32_t)d, i, val); \
		} \
	} while (0)

/**************************/
/* Bit-field access macros
: v	 -  value
: lsbn  - ls_bit_number
: fw	- field_width
: a	 - address
: rv	- read_value	  */
/**************************/
#define FIELD_GET(v, lsbn, fw)	(((v)>>(lsbn)) & ((unsigned)(1 << (fw)) - 1))

#define FIELD_MGET_32(a, lsbn, fw)	(FIELD_GET(MGET_32(a), (lsbn), (fw)))
#define FIELD_MGET_16(a, lsbn, fw)	(FIELD_GET(MGET_16(a), (lsbn), (fw)))
#define FIELD_MGET_8(a, lsbn, fw)	(FIELD_GET(MGET_8(a) , (lsbn), (fw)))

#define FIELD_MREAD_8(a, lsbn, fw, rv)	(rv = FIELD_MGET_8((a), (lsbn), (fw)))
#define FIELD_MREAD_16(a, lsbn, fw, rv)	(rv = FIELD_MGET_16((a), (lsbn), (fw)))
#define FIELD_MREAD_32(a, lsbn, fw, rv)	(rv = FIELD_MGET_32((a), (lsbn), (fw)))

#define FIELD_SET(value, ls_bit_number, field_width, write_value) \
	do { \
		uint32_t mask; \
		mask = ((1 << (field_width)) - 1) << (ls_bit_number); \
		value &=  ~mask; \
		value |= (write_value) << (ls_bit_number); \
	} while (0)

#define FIELD_MWRITE_32(address, ls_bit_number, field_width, write_value) \
	do { \
		uint32_t current_value = MGET_32(address); \
		FIELD_SET(current_value, ls_bit_number, field_width, write_value); \
		MWRITE_32(address, current_value); \
	} while (0)

#define FIELD_MWRITE_16(address, ls_bit_number, field_width, write_value) \
	do { \
		uint16_t current_value = MGET_16(address); \
		FIELD_SET(current_value, ls_bit_number, field_width, write_value); \
		MWRITE_16(address, current_value); \
	} while (0)

#define FIELD_MWRITE_8(address, ls_bit_number, field_width, write_value) \
	do { \
		uint8_t current_value = MGET_8(address); \
		FIELD_SET(current_value, ls_bit_number, field_width, write_value); \
		MWRITE_8(address, current_value); \
	} while (0)

#define GROUP_MREAD_I_8(group, addr, i, ret)	(ret = _rdd_i_read(group, (addr), i, rdd_size_8))
#define GROUP_MREAD_I_16(group, addr, i, ret)	(ret = _rdd_i_read(group, (addr), i, rdd_size_16))
#define GROUP_MREAD_I_32(group, addr, i, ret)	(ret = _rdd_i_read(group, (addr), i, rdd_size_32))

#define GROUP_MREAD_8(group, addr, ret)		(ret = _rdd_i_read(group, (addr), 0, rdd_size_8))
#define GROUP_MREAD_16(group, addr, ret)	(ret = _rdd_i_read(group, (addr), 0, rdd_size_16))
#define GROUP_MREAD_32(group, addr, ret)	(ret = _rdd_i_read(group, (addr), 0, rdd_size_32))

#define GROUP_FIELD_MREAD_8(group, addr, lsb, width, ret) \
	(ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_8))
#define GROUP_FIELD_MREAD_16(group, addr, lsb, width, ret) \
	(ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_16))
#define GROUP_FIELD_MREAD_32(group, addr, lsb, width, ret) \
	(ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_32))

#define GROUP_MWRITE_I_8(group, addr, i, val)	_rdd_i_write(group, (addr), val, i, rdd_size_8)
#define GROUP_MWRITE_I_16(group, addr, i, val)	_rdd_i_write(group, (addr), val, i, rdd_size_16)
#define GROUP_MWRITE_I_32(group, addr, i, val)	_rdd_i_write(group, (addr), val, i, rdd_size_32)

#define GROUP_MWRITE_8(group, addr, val)	_rdd_i_write(group, (addr), val, 0, rdd_size_8)
#define GROUP_MWRITE_16(group, addr, val)	_rdd_i_write(group, (addr), val, 0, rdd_size_16)
#define GROUP_MWRITE_32(group, addr, val)	_rdd_i_write(group, (addr), val, 0, rdd_size_32)

#define GROUP_FIELD_MWRITE_8(group, addr, lsb, width, val) \
	_rdd_field_write(group, (addr), val, lsb, width, rdd_size_8)
#define GROUP_FIELD_MWRITE_16(group, addr, lsb, width, val) \
	_rdd_field_write(group, (addr), val, lsb, width, rdd_size_16)
#define GROUP_FIELD_MWRITE_32(group, addr, lsb, width, val) \
	_rdd_field_write(group, (addr), val, lsb, width, rdd_size_32)

#if defined(DUAL_ISSUE)
static inline void *_xrdp__memcpy(void *dst, void const *src, size_t len) 
{
    uint8_t *u8dst_ptr = (uint8_t *)dst;
    uint8_t const *u8src_ptr = (uint8_t const *)src;
    uint src_alignment = (uint)((uintptr_t)src & (uintptr_t)(sizeof(uint32_t) - 1));
    uint dst_alignment = (uint)((uintptr_t)dst & (uintptr_t)(sizeof(uint32_t) - 1));

    if (len >= sizeof(uint32_t) &&  src_alignment == dst_alignment ) 
    {
        while (((uintptr_t)u8src_ptr & (uintptr_t)(sizeof(uint32_t) - 1)) != 0) 
        {
            *u8dst_ptr++ = *u8src_ptr++;
            len--;
        }
        {
            uint32_t *u32dst_ptr = (uint32_t *)u8dst_ptr;
            uint32_t const *u32src_ptr = (uint32_t const *)u8src_ptr;
    
            while (len >= sizeof(uint32_t)) 
            {
                *u32dst_ptr++ = *u32src_ptr++;
                len -= sizeof(uint32_t);
            }
            u8dst_ptr = (uint8_t *)u32dst_ptr;
            u8src_ptr = (uint8_t const *)u32src_ptr;
        }
    }
    while (len--) 
    {
         *u8dst_ptr++ = *u8src_ptr++;
    }
    return dst;
}

static inline void *_xrdp__memset(void *dst, int v, size_t len)
{
    uint8_t  *u8dst_ptr = (uint8_t *)dst;
    uint8_t  c_val = (uint8_t)v;
    uint32_t u32_val = (uint32_t)c_val<<24 || (uint32_t)c_val<<16 || (uint32_t)c_val<<8 || (uint32_t)c_val<<24;

    while (len && (((uintptr_t)u8dst_ptr & (uintptr_t)(sizeof(uint32_t) - 1)) != 0))
    {
        *u8dst_ptr++ = c_val;
        len--;
    }
    
    {
        uint32_t *u32dst_ptr = (uint32_t *)u8dst_ptr;
        while ( len >= sizeof(uint32_t) ) 
        {
            *u32dst_ptr++ = u32_val;
            len -= sizeof(uint32_t);
        }
    }

    while (len) 
    {
        *u8dst_ptr++ = c_val;
        len--;
    }
    
    return dst;
}

#endif /* defined(DUAL_ISSUE) */

#endif /* __ACCESS_MACROS_H_INCLUDED */
