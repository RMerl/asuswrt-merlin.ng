/*
    <:copyright-BRCM:2015:DUAL/GPL:standard

       Copyright (c) 2015 Broadcom
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
*/

#ifndef __ACCESS_MACROS_H_INCLUDED
#define __ACCESS_MACROS_H_INCLUDED


#if !defined(__UBOOT__)
#include <linux/version.h>
#endif

#include "rdd_fw_defs.h"

#if defined(CONFIG_GPL_RDP_GEN) || defined(CONFIG_GPL_RDP)
#include "access_logging.h"
#endif

#ifndef ACCESS_LOG
#define ACCESS_LOG_ENABLE_SET(_e)
#define ACCESS_LOG(_op, _a, _v, _sz)
#endif

#define RDP_BLOCK_SIZE          0x1000000

#if defined(BCM_DSL_XRDP)
#define WAN_BLOCK_ADDRESS_MASK  0x3FFFF
#else
#define WAN_BLOCK_ADDRESS_MASK  0xFFFF
#endif

/*****************/
/* Generic swaps */
/*****************/
static inline uint16_t __swap2bytes(uint16_t a)
{
    return (a << 8) | (a >> 8);
}
static inline uint32_t __swap4bytes(uint32_t a)
{
    return (a << 24) |
             ((a & 0xFF00) << 8) |
             ((a & 0xFF0000) >> 8) |
             (a >> 24);
}

static inline uint64_t __swap4bytes64(uint64_t a)
{
   return  __swap4bytes(a) |
           ((uint64_t)__swap4bytes((a>>32) & 0xFFFFFFFF) << 32);
}

/**************************/
/* swap2bytes, swap4bytes */
/**************************/
#if defined(_BYTE_ORDER_LITTLE_ENDIAN_)
#if defined(__ARMEL__) || defined(__AARCH64EL__)
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
static inline void swap_4_words(uint32_t *src, uint32_t *dst)
{
    /* Using this Aarch64 Assembly optimization to reduce descriptor read time
    * algo: load pair of 64bit registers from src
    * swap32 each of them
    * store pair 64bit registers to dst */ 

    register uint64_t dword0 asm ("x9");
    register uint64_t dword1 asm ("x10");

    asm volatile("LDP   %1, %2,[%0]; \
        REV32  %1, %1; \
        REV32  %2, %2; \
        STP %1, %2, [%3];" \
        :  "=r" (src), "=r" (dword0), "=r" (dword1), "=r" (dst) \
        : "0" (src), "3" (dst));
}
#else
static inline uint32_t swap4bytes(uint32_t a)
{
    __asm__("rev %0, %1" : "=r" (a) : "r" (a));
    return a;
}
static inline uint64_t swap4bytes64(uint64_t a)
{
    uint32_t *b = (uint32_t *)&a;

    b[0] = swap4bytes(b[0]);
    b[1] = swap4bytes(b[1]);

    return a;
}
static inline void swap_4_words(uint32_t *src, uint32_t *dst)
{
    dst[0] = swap4bytes(src[0]);
    dst[1] = swap4bytes(src[1]);
    dst[2] = swap4bytes(src[2]);
    dst[3] = swap4bytes(src[3]);
}
#endif
#else
#define swap2bytes(x)  __swap2bytes(x)
#define swap4bytes(x)  __swap4bytes(x)
#define swap4bytes64(x)  __swap4bytes64(x)
#define swap_4_words(x, y) \
    do { \
        uint32_t *src = (uint32_t *)(x); \
        uint32_t *dst = (uint32_t *)(y); \
        dst[0] = swap4bytes(src[0]); \
        dst[1] = swap4bytes(src[1]); \
        dst[2] = swap4bytes(src[2]); \
        dst[3] = swap4bytes(src[3]); \
    } while (0)
#endif /* __ARMEL__ */

#else /* FIRMWARE_LITTLE_ENDIAN */

#define swap2bytes(x)  (x)
#define swap4bytes(x)  (x)
#define swap4bytes64(x)  (x)
#define swap_4_words(x, y) \
    do { \
        uint32_t *src = (uint32_t *)(x); \
        uint32_t *dst = (uint32_t *)(y); \
        dst[0] = src[0]; \
        dst[1] = src[1]; \
        dst[2] = src[2]; \
        dst[3] = src[3]; \
    } while (0)

#endif /* FIRMWARE_LITTLE_ENDIAN */

#define SWAPBYTES(_buf, _len) \
    do { \
        uint8_t _i; \
        uint32_t len = (uint32_t)(_len); \
        uint32_t *v32_p = (uint32_t *)(_buf); \
        if(len%4) BDMF_TRACE_ERR("ERROR: Len %u is not multiple of 4\n",len);\
        /*if((uintptr_t)v32_p%4) BDMF_TRACE_ERR("ERROR: Pointer %px is not 4 byte aligned\n",v32_p);*/\
        for (_i = 0; _i < (len/4); _i++) \
            v32_p[_i] = swap4bytes(v32_p[_i]); \
    } while (0)

/*************************/
/* cpu_to_le / cpu_to_be */
/*************************/
/*
 * Endian swapping macros that work on any CPU.
 * Swap between CPU byte order and Big Endian byte order
 */
#if !defined(LINUX_KERNEL) && !defined(__KERNEL__) && !defined(_CFE_)
#if defined(_BYTE_ORDER_LITTLE_ENDIAN_) || \
    (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
    (defined(__BYTE_ORDER__) && defined(__LITTLE_ENDIAN__) && __BYTE_ORDER__ == __LITTLE_ENDIAN__)

#define cpu_to_le32(x)   (x)
#define cpu_to_le16(x)   (x)
#define cpu_to_be16(x)   swap2bytes(x)
#define cpu_to_be32(x)   swap4bytes(x)

#else

#define cpu_to_le32(x)   swap4bytes(x)
#define cpu_to_le16(x)   swap2bytes(x)
#define cpu_to_be16(x)   (x)
#define cpu_to_be32(x)   (x)
#endif /*BYTE_ORDER */

#endif /* LINUX_KERNEL */

/* arch dependend  definitions */ 
#include "bcm_rdp_arch.h"
#if defined(RDP_ARCH_SIM)
#include "access_macros_sim.h"
#elif defined(RDP_ARCH_BOARD) || defined (RDP_ARCH_QEMU_SIM)
#include "access_macros_board.h"
#elif defined(RDP_ARCH_BOOT)
#include "access_macros_boot.h"
#else /* no RDP arch */
#error "no arch defined"
#endif /* RDP_ARCH_SIM */

/************************/
/* Registers and memory */
/************************/

/* Memory */
#define MGET_8(a)               (*(volatile uint8_t *)(a))
#define MGET_16(a)              swap2bytes(*(volatile uint16_t *)(a))
#define MGET_32(a)              swap4bytes(*(volatile uint32_t *)(a))

#define MGET_I_8(a, i)          (*((volatile uint8_t *)(a) + (i)))
#define MGET_I_16(a, i)         swap2bytes(*((volatile uint16_t *)(a) + (i)))
#define MGET_I_32(a, i)         swap4bytes(*((volatile uint32_t *)(a) + (i)))

#define MWRITE_8(a, r) \
                do {\
                    (*(volatile uint8_t *)(a) = (uint8_t)(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_MWRITE, (a), (r), 1); \
                } while (0)
#define MWRITE_16(a, r) \
                do {\
                    (*(volatile uint16_t *)(a) = swap2bytes((uint16_t)(r))); \
                    ACCESS_LOG(ACCESS_LOG_OP_MWRITE, (a), swap2bytes((uint16_t)(r)), 2); \
                } while (0)

#define MWRITE_32(a, r) \
                do {\
                    (*(volatile uint32_t *)(a) = swap4bytes((uint32_t)(r))); \
                    ACCESS_LOG(ACCESS_LOG_OP_MWRITE, (a), swap4bytes((uint32_t)(r)), 4); \
                } while (0)

#define MWRITE_I_8(a, i, r) \
                do {\
                    (*((volatile uint8_t *)(a) + (i)) = (uint8_t)(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_MWRITE, ((volatile uint8_t *)(a) + (i)), (r), 1); \
                } while (0)
#define MWRITE_I_8_NOLOG(a, i, r) \
                do {\
                    (*((volatile uint8_t *)(a) + (i)) = (uint8_t)(r)); \
                } while (0)
#define MWRITE_I_16(a, i, r) \
                do {\
                    (*((volatile uint16_t *)(a) + (i)) = swap2bytes((uint16_t)(r))); \
                    ACCESS_LOG(ACCESS_LOG_OP_MWRITE, ((volatile uint16_t *)(a) + (i)), swap2bytes((uint16_t)(r)), 2); \
                } while (0)
#define MWRITE_I_32(a, i, r) \
                do {\
                    (*((volatile uint32_t *)(a) + (i)) = swap4bytes((uint32_t)(r))); \
                    ACCESS_LOG(ACCESS_LOG_OP_MWRITE, ((volatile uint32_t *)(a) + (i)), swap4bytes((uint32_t)(r)), 4); \
                } while (0)
#define MWRITE_I_32_NOLOG(a, i, r) \
                do {\
                    (*((volatile uint32_t *)(a) + (i)) = swap4bytes((uint32_t)(r))); \
                } while (0)
#if defined(CONFIG_ARM64)
#define MWRITE_I_64(a, i, r)    (*((volatile uint64_t *)(a) + (i)) = swap4bytes64((uint64_t)(r)))
#endif

#define MREAD_8(a, r)           ((r) = MGET_8(a))
#define MREAD_16(a, r)          ((r) = MGET_16(a))
#define MREAD_32(a, r)          ((r) = MGET_32(a))

#define MREAD_I_8(a, i, r)      ((r) = MGET_I_8((a), (i)))
#define MREAD_I_16(a, i, r)     ((r) = MGET_I_16((a), (i)))
#define MREAD_I_32(a, i, r)     ((r) = MGET_I_32((a), (i)))

/*************/
/* MEMSET_8 */
/*************/
#define MEMSET_8(a, v_8, sz_8)  \
    do { \
        int __i;\
        for (__i=0; __i<sz_8; __i++)\
            MWRITE_I_8_NOLOG(a, __i, v_8);\
        ACCESS_LOG(ACCESS_LOG_OP_MEMSET_8, a, v_32, sz_32); \
    } while (0)

/*************/
/* MEMSET_32 */
/*************/
#define MEMSET_32(a, v_32, sz_32)  \
    do { \
        int __i;\
        for (__i=0; __i<sz_32; __i++)\
            MWRITE_I_32_NOLOG(a, __i, v_32);\
        ACCESS_LOG(ACCESS_LOG_OP_MEMSET_32, a, v_32, sz_32); \
    } while (0)

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

#define MWRITE_BLK_32_SWAP(d, s, sz)      { uint32_t i, val; for ( i = 0; i < ( sz / 4 ); i++ ){ val = *((volatile uint32_t*)(s) + (i));val = swap4bytes(val); MWRITE_I_32( d, i, val ); } }

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

#define RDP_FIELD_MREAD_8(a, lsbn, fw, rv)  (rv = RDP_FIELD_MGET_8((a),   (lsbn), (fw)))
#define RDP_FIELD_MREAD_16(a, lsbn, fw, rv) (rv = RDP_FIELD_MGET_16((a),   (lsbn), (fw)))
#define RDP_FIELD_MREAD_32(a, lsbn, fw, rv) (rv = RDP_FIELD_MGET_32((a),   (lsbn), (fw)))

#define RDP_FIELD_SET(value, ls_bit_number, field_width, write_value) \
    do { \
        uint32_t  mask; \
        mask = ((1 << (field_width)) - 1) << (ls_bit_number); \
        value &=  ~mask; \
        value |= (write_value & ((1 << (field_width)) - 1)) << (ls_bit_number); \
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

#define GROUP_MREAD_I_8(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_8))
#define GROUP_MREAD_I_16(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_16))
#define GROUP_MREAD_I_32(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_32))

#define GROUP_MREAD_8(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_8))
#define GROUP_MREAD_16(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_16))
#define GROUP_MREAD_32(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_32))

#define CORE_MREAD_8(group, addr, ret, core_id) (ret = _rdd_i_read_single_core(group, (addr), 0, rdd_size_8, core_id))
#define CORE_MREAD_16(group, addr, ret, core_id) (ret = _rdd_i_read_single_core(group, (addr), 0, rdd_size_16, core_id))
#define CORE_MREAD_32(group, addr, ret, core_id) (ret = _rdd_i_read_single_core(group, (addr), 0, rdd_size_32, core_id))

#define GROUP_RDP_FIELD_MREAD_8(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_8))
#define GROUP_RDP_FIELD_MREAD_16(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_16))
#define GROUP_RDP_FIELD_MREAD_32(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_32))

#define CORE_RDP_FIELD_MREAD_8(group, addr, lsb, width, ret, core_id) (ret = _rdd_field_read_single_core(group, (addr), lsb, width, rdd_size_8, core_id))
#define CORE_RDP_FIELD_MREAD_16(group, addr, lsb, width, ret, core_id) (ret = _rdd_field_read_single_core(group, (addr), lsb, width, rdd_size_16, core_id))
#define CORE_RDP_FIELD_MREAD_32(group, addr, lsb, width, ret, core_id) (ret = _rdd_field_read_single_core(group, (addr), lsb, width, rdd_size_32, core_id))

#define GROUP_MWRITE_I_8(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_8)
#define GROUP_MWRITE_I_16(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_16)
#define GROUP_MWRITE_I_32(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_32)

#define GROUP_MWRITE_8(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_8)
#define GROUP_MWRITE_16(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_16)
#define GROUP_MWRITE_32(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_32)

#define CORE_MWRITE_8(group, addr, val, core_id) _rdd_i_write_single_core(group, (addr), val, 0, rdd_size_8, core_id)
#define CORE_MWRITE_16(group, addr, val, core_id) _rdd_i_write_single_core(group, (addr), val, 0, rdd_size_16, core_id)
#define CORE_MWRITE_32(group, addr, val, core_id) _rdd_i_write_single_core(group, (addr), val, 0, rdd_size_32, core_id)

#define GROUP_RDP_FIELD_MWRITE_8(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_8)
#define GROUP_RDP_FIELD_MWRITE_16(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_16)
#define GROUP_RDP_FIELD_MWRITE_32(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_32)

#define CORE_RDP_FIELD_MWRITE_8(group, addr, lsb, width, val, core_id) _rdd_field_write_single_core(group, (addr), val, lsb, width, rdd_size_8, core_id)
#define CORE_RDP_FIELD_MWRITE_16(group, addr, lsb, width, val, core_id) _rdd_field_write_single_core(group, (addr), val, lsb, width, rdd_size_16, core_id)
#define CORE_RDP_FIELD_MWRITE_32(group, addr, lsb, width, val, core_id) _rdd_field_write_single_core(group, (addr), val, lsb, width, rdd_size_32, core_id)

#endif /* __ACCESS_MACROS_H_INCLUDED */
