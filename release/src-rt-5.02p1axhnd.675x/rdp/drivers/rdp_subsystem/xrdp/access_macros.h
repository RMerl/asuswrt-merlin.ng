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

#if defined(CONFIG_GPL_RDP_GEN) || defined(CONFIG_GPL_RDP)
#include "access_logging.h"
#endif

#ifndef ACCESS_LOG
#define ACCESS_LOG_ENABLE_SET(_e)
#define ACCESS_LOG(_op, _a, _v, _sz)
#endif

#define RDP_BLOCK_SIZE          0x1000000

#if defined(BCM63158)
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
#ifndef XRDP_EMULATION
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
#else
static inline uint32_t swap4bytes(uint32_t a)
{
    __asm__("rev %0, %1" : "=r" (a) : "r" (a));
    return a;
}
#endif
#else
#define swap2bytes(x)  __swap2bytes(x)
#define swap4bytes(x)  __swap4bytes(x)
#define swap4bytes64(x)  __swap4bytes64(x)
#endif /* __ARMEL__ */

#else /* FIRMWARE_LITTLE_ENDIAN */

#define swap2bytes(x)  (x)
#define swap4bytes(x)  (x)
#define swap4bytes64(x)  (x)

#endif /* FIRMWARE_LITTLE_ENDIAN */
#else /* XRDP_EMULATION */

#define swap2bytes(x)  __swap2bytes(x)
#define swap4bytes(x)  __swap4bytes(x)
#define swap4bytes64(x)  __swap4bytes64(x)

#endif

#define SWAPBYTES(buffer, len) \
    do { \
        uint8_t _i; \
        for (_i = 0; _i < len; _i += sizeof(uint32_t)) \
            *((uint32_t *)(&(buffer[_i]))) = swap4bytes(*((uint32_t *)(&(buffer[_i])))); \
    } while (0)


/*************************/
/* cpu_to_le / cpu_to_be */
/*************************/
/*
 * Endian swapping macros that work on any CPU.
 * Swap between CPU byte order and Big Endian byte order
 */
#if defined(_BYTE_ORDER_LITTLE_ENDIAN_) || \
    (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
    (defined(__BYTE_ORDER__) && defined(__LITTLE_ENDIAN__) && __BYTE_ORDER__ == __LITTLE_ENDIAN__)

#if !defined(LINUX_KERNEL) && !defined(__KERNEL__) && !defined(_CFE_)
#define cpu_to_le32(x)   (x)
#define cpu_to_le16(x)   (x)
#define cpu_to_be16(x)   swap2bytes(x)
#define cpu_to_be32(x)   swap4bytes(x)
#endif

#else

#if !defined(LINUX_KERNEL) && !defined(__KERNEL__) && !defined(_CFE_)
#define cpu_to_le32(x)   swap4bytes(x)
#define cpu_to_le16(x)   swap2bytes(x)
#define cpu_to_be16(x)   (x)
#define cpu_to_be32(x)   (x)
#endif

#endif


/******************/
/* Device address */
/******************/
#ifndef XRDP_EMULATION

#if !defined(RDP_SIM)

#define DEVICE_ADDRESS(_a) ((volatile uint8_t * const)((uintptr_t)(_a)))

#else /* RDP_SIM */

#define SOC_BASE_ADDRESS        0x82000000
extern uint8_t *soc_base_address;
#define DEVICE_ADDRESS(_a) \
    (((bdmf_phys_addr_t)(_a) < SOC_BASE_ADDRESS) ? \
        ((volatile uint8_t * const)(soc_base_address + RDP_BLOCK_SIZE + ((bdmf_phys_addr_t)(_a) & WAN_BLOCK_ADDRESS_MASK))) \
        : ((volatile uint8_t * const)(soc_base_address + ((bdmf_phys_addr_t)(_a) & (RDP_BLOCK_SIZE - 1)))))

#endif

#else /* XRDP_EMULATION */

#define DEVICE_ADDRESS(_a) ((volatile uint8_t * const)((uint32_t)(_a)))

#endif

/**********/
/* MEMSET */
/**********/
#if !defined(XRDP_EMULATION) // SEE BELOW for XRDP_EMULATION version
#if defined(_CFE_)

#include "lib_types.h"
#include "lib_string.h"

#define MEMSET(a, v, sz)                \
    do {\
        lib_memset(a, v, sz); \
        ACCESS_LOG(ACCESS_LOG_OP_MEMSET, a, v, sz); \
    } while (0)

#elif defined(RDP_SIM)
#define MEMSET(a, v, sz)                memset(a, v, sz)
#else

#if defined(DUAL_ISSUE)
#define MEMSET(a, v, sz) _xrdp__memset(a, v, sz)
#else
#define MEMSET(a, v, sz)                memset_io(a, v, sz)
#endif

#endif
#endif // !defined(XRDP_EMULATION)

/******************/
/* Memory Barrier */
/******************/
#if !defined(RDP_SIM) && (defined LINUX_KERNEL || __KERNEL__)
 #include "linux/types.h"
 #include <asm/barrier.h>
 #define WMB()  wmb() /* memory barrier */
 #define RMB()  rmb() /* memory barrier */
#else
 #define WMB()  /* */
 #define RMB()  /* */
#endif


/************************/
/* Registers and memory */
/************************/
#ifndef XRDP_EMULATION

/* Registers */
#if defined(RDP_SIM)

#define VAL32(_a)               (*(volatile uint32_t *)(DEVICE_ADDRESS(_a)))
#define READ_8(a, r)            (*(volatile uint8_t *)&(r) = *(volatile uint8_t *)DEVICE_ADDRESS(a))
#define READ_16(a, r)           do { \
                                       uint16_t u16 = *(volatile uint16_t *)DEVICE_ADDRESS(a); \
                                       *(volatile uint16_t *)&(r) = swap2bytes(u16); \
                                } while (0)
#define READ_32(a, r)           do { \
                                       uint32_t u32 = *(volatile uint32_t *)DEVICE_ADDRESS(a); \
                                       *(volatile uint32_t *)&(r) = swap4bytes(u32); \
                                } while (0)

#define WRITE_8(a, r)           (*(volatile uint8_t *)DEVICE_ADDRESS(a) = *(uint8_t *)&(r))
#define WRITE_16(a, r)          (*(volatile uint16_t *)DEVICE_ADDRESS(a) = swap2bytes(*(uint16_t *)&(r)))
#define WRITE_32(a, r)          (*(volatile uint32_t *)DEVICE_ADDRESS(a) = swap4bytes(*(uint32_t *)&(r)))

#define READ_I_8(a, i, r)       (*(volatile uint8_t *)&(r) = *((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)))
#define READ_I_16(a, i, r)      do { \
                                    uint16_t u16 = *((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)); \
                                    *(volatile uint16_t *)&(r) = swap2bytes(u16); \
                                } while (0)
#define READ_I_32(a, i, r)      do { \
                                    uint32_t u32 = *((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)); \
                                    *(volatile uint32_t *)&(r) = swap4bytes(u32); \
                                } while (0)

#define WRITE_I_8(a, i, r)      (*((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)) = *(uint8_t *)&(r))
#define WRITE_I_16(a, i, r) (*((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)) = swap2bytes(*(uint16_t *)&(r)))
#define WRITE_I_32(a, i, r) (*((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)) = swap4bytes(*(uint32_t *)&(r)))

#define do_div(x,y)             (x = x/y);
#else /* defined(RDP_SIM) */

#define VAL32(_a)       (*(volatile uint32_t *)(DEVICE_ADDRESS(_a)))
#define READ_8(a, r)        (*(volatile uint8_t *)&(r) = *(volatile uint8_t *)DEVICE_ADDRESS(a))
#define READ_16(a, r)       (*(volatile uint16_t *)&(r) = *(volatile uint16_t *)DEVICE_ADDRESS(a))
#define READ_32(a, r)           (*(volatile uint32_t *)&(r) = *(volatile uint32_t *)DEVICE_ADDRESS(a))

#define WRITE_8(a, r) \
                do {\
                    (*(volatile uint8_t *)DEVICE_ADDRESS(a) = *(uint8_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint8_t *)&(r), 1); \
                } while (0)
#define WRITE_16(a, r) \
                do {\
                    (*(volatile uint16_t *)DEVICE_ADDRESS(a) = *(uint16_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint16_t *)&(r), 2); \
                } while (0)
#define WRITE_32(a, r) \
                do {\
                    (*(volatile uint32_t *)DEVICE_ADDRESS(a) = *(uint32_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, DEVICE_ADDRESS(a), *(uint32_t *)&(r), 4); \
                } while (0)

#define READ_I_8(a, i, r)       (*(volatile uint8_t *)&(r) = *((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)))
#define READ_I_16(a, i, r)  (*(volatile uint16_t *)&(r) = *((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)))
#define READ_I_32(a, i, r)  (*(volatile uint32_t *)&(r) = *((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)))

#define WRITE_I_8(a, i, r) \
                do {\
                    (*((volatile uint8_t *)DEVICE_ADDRESS(a) + (i)) = *(uint8_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint8_t *)DEVICE_ADDRESS(a) + (i), *(uint8_t *)&(r), 1); \
                } while (0)
#define WRITE_I_16(a, i, r) \
                do {\
                    (*((volatile uint16_t *)DEVICE_ADDRESS(a) + (i)) = *(uint16_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint16_t *)DEVICE_ADDRESS(a) + (i), *(uint16_t *)&(r), 2); \
                } while (0)
#define WRITE_I_32(a, i, r) \
                do {\
                    (*((volatile uint32_t *)DEVICE_ADDRESS(a) + (i)) = *(uint32_t *)&(r)); \
                    ACCESS_LOG(ACCESS_LOG_OP_WRITE, (volatile uint32_t *)DEVICE_ADDRESS(a) + (i), *(uint32_t *)&(r), 4); \
                } while (0)

#endif

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

#if defined(_CFE_) || defined(RDP_SIM)

#define MREAD_BLK_8(d, s, sz) memcpy(d, s, sz)
#define MREAD_BLK_16(d, s, sz) memcpy(d, s, sz)
#define MREAD_BLK_32(d, s, sz) memcpy(d, s, sz)

#else

#if defined(DUAL_ISSUE)
    #define MREAD_BLK_8(d, s, sz)  _xrdp__memcpy(d, s, sz)
#else /* defined(DUAL_ISSUE) */
    #define MREAD_BLK_8(d, s, sz)  memcpy_fromio(d, s, sz)
#endif
#define MREAD_BLK_16(d, s, sz) \
    do { \
        uint32_t i, val; \
        for (i = 0; i < (sz/sizeof(uint16_t)); i++) \
        { \
            val = *((volatile uint16_t *)(s) + (i)); \
            MREAD_I_16((uintptr_t)(d), i, val); \
        } \
    } while (0)

#define MREAD_BLK_32(d, s, sz) \
    do { \
        uint32_t i, val; \
        for (i = 0; i < (sz/sizeof(uint32_t)); i++) \
        { \
            val = *((volatile uint32_t *)(s) + (i)); \
            MREAD_I_32((uintptr_t)(d), i, val); \
        } \
    } while (0)

#endif
#else /* XRDP_EMULATION */

typedef void(*write32_p)(uint32_t, uint32_t, int);
typedef void(*write16_p)(uint32_t, uint16_t, int);
typedef void(*write8_p)(uint32_t, uint8_t, int);
typedef uint32_t(*read32_p)(uint32_t);
typedef uint16_t(*read16_p)(uint32_t);
typedef uint8_t(*read8_p)(uint32_t);

extern write32_p write32;
extern write16_p write16;
extern write8_p write8;
extern read32_p read32;
extern read16_p read16;
extern read8_p read8;

/* Registers */
#define READ_8(a, r)            ((r) = read8(a))
#define READ_16(a, r)           ((r) = read16(a))
#define READ_32(a, r)           ((r) = read32(a))

#define WRITE_8(a, r)           (write8(a, r, 0))
#define WRITE_16(a, r)          (write16(a, r, 0))
#define WRITE_32(a, r)          (write32(a, r, 0))

#define READ_I_8(a, i, r)       ((r) = read8((uint32_t)a + i))
#define READ_I_16(a, i, r)      ((r) = read16((uint32_t)a + 2*i))
#define READ_I_32(a, i, r)      ((r) = read32((uint32_t)a + 4*i))

#define WRITE_I_8(a, i, r)  (write8((uint32_t)a + i, r, 0))
#define WRITE_I_16(a, i, r) (write16((uint32_t)a + 2*i, r, 0))
#define WRITE_I_32(a, i, r) (write32((uint32_t)a + 4*i, r, 0))

/* Memory */
#define MGET_8(a)              (read8(a))
#define MGET_16(a)              (swap2bytes(read16(a)))
#define MGET_32(a)              (swap4bytes(read32(a)))


#define MREAD_8(a, r)           ((r) = MGET_8(a))
#define MREAD_16(a, r)          ((r) = MGET_16(a))
#define MREAD_32(a, r)          ((r) = MGET_32(a))

#define MWRITE_8(a, r)        (write8(a, r, 0))
#define MWRITE_16(a, r)       (write16(a, r, 0))
#define MWRITE_32(a, r)       (write32(a, r, 0))

#define MGET_I_8(a, i)         ((i) = read8((uint32_t)a + i))
#define MGET_I_16(a, i)         ((i) = swap2bytes(read16((uint32_t)a + 2*i)))
#define MGET_I_32(a, i)         ((i) = swap4bytes(read32((uint32_t)a + 4*i)))

#define MREAD_I_8(a, i, r)      ((r) = MGET_I_8((a), (i)))
#define MREAD_I_16(a, i, r)     ((r) = MGET_I_16((a), (i)))
#define MREAD_I_32(a, i, r)     ((r) = MGET_I_32((a), (i)))

#define MWRITE_I_8(a, i, r)    (write8((uint32_t)a + i, r, 0))
#define MWRITE_I_16(a, i, r)    (write16((uint32_t)a + 2*i, (uint16_t)(r), 0))
#define MWRITE_I_32(a, i, r)    (write32((uint32_t)a + 4*i, (uint32_t)(r), 0))
#define MWRITE_I_64(a, i, r)    (write64((uint32_t)a + 8*i, (uint64_t)(r), 0))

#define MREAD_BLK_8(d, s, sz) \
    do { \
        uint32_t i; \
        for (i = 0; i < (sz); i++) \
             MREAD_8((uint32_t)(s) + i, (d[i])); \
    } while (0)
#define MREAD_BLK_16(d, s, sz) \
    do { \
        uint32_t i; \
        for (i = 0; i < (sz); i+=2) \
            MREAD_16((uint32_t)(s) + i, (d[i/2])); \
    } while (0)
#define MREAD_BLK_32(d, s, sz) \
    do { \
        uint32_t i; \
        for (i = 0; i < (sz); i+=4) \
            MREAD_32((uint32_t)(s) + i, (d[i/4])); \
    } while (0)

// Use write8 and write 32 macros so that  in RTL simulations this is
// done using backdoor memory reads and writes.
#define MEMSET(a, v, sz) \
    do {                                                            \
        uint32_t bytes_left = (uint32_t) sz;                        \
        uint32_t addr = (uint32_t) a;                               \
        uint32_t v32 = (v << 24) | (v << 16) | (v << 8) | v;        \
                                                                    \
        while(bytes_left)                                           \
        {                                                           \
            if ((addr & 3) || (bytes_left < 4))                     \
            {                                                       \
                write8(addr, (unsigned char) v, 0);                 \
                addr += 1;                                          \
                bytes_left -= 1;                                    \
            } else {                                                \
                write32(addr, v32, 0);                              \
                addr += 4;                                          \
                bytes_left -= 4;                                    \
            }                                                       \
        }                                                           \
    } while (0)

#endif /* XRDP_EMULATION */


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

#ifndef XRDP_EMULATION
#if defined(_CFE_) || defined(RDP_SIM)
#define MWRITE_BLK_8(d, s, sz) memcpy(d, s, sz)
#else
#if defined(DUAL_ISSUE)
#define MWRITE_BLK_8(d, s, sz) _xrdp__memcpy(d, s, sz)
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
#else /* XRDP_EMULATION */
#define MWRITE_BLK_8(d, s, sz) \
    do { \
        uint32_t i, val; \
        for (i = 0; i < (sz); i++) \
        { \
            val = *((volatile uint8_t *)(s) + (i)); \
            MWRITE_I_8((uint32_t)(d), i, val); \
        } \
    } while (0)
#define MWRITE_BLK_16(d, s, sz) \
    do { \
        uint32_t i, val; \
        for (i = 0; i < (sz / 2); i++) { \
            val = *((volatile uint16_t *)(s) + (i)); \
            MWRITE_I_16((uint32_t)d, i, val); \
        } \
    } while (0)
#define MWRITE_BLK_32(d, s, sz) \
    do { \
        uint32_t i, val; \
        for (i = 0; i < (sz / 4); i++) { \
            val = *((volatile uint32_t *)(s) + (i)); \
            MWRITE_I_32((uint32_t)d, i, val); \
        } \
    } while (0)
#endif

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





//#define MWRITE_BLK_32_SWAP(d, s, sz)      { uint32_t i, val; for ( i = 0; i < ( sz / 4 ); i++ ){ val = *((volatile uint32_t*)(s) + (i)); MWRITE_I_32( d, i, val ); } }

/**************************/
/* Bit-field access macros
: v     -  value
: lsbn  - ls_bit_number
: fw    - field_width
: a     - address
: rv    - read_value      */
/**************************/
#define FIELD_GET(v, lsbn, fw)          (((v)>>(lsbn)) & ((unsigned)(1 << (fw)) - 1))

#define FIELD_MGET_32(a, lsbn, fw)      (FIELD_GET(MGET_32(a), (lsbn), (fw)))
#define FIELD_MGET_16(a, lsbn, fw)      (FIELD_GET(MGET_16(a), (lsbn), (fw)))
#define FIELD_MGET_8(a, lsbn, fw)       (FIELD_GET(MGET_8(a) , (lsbn), (fw)))

#define FIELD_MREAD_8(a, lsbn, fw, rv)  (rv = FIELD_MGET_8((a),   (lsbn), (fw)))
#define FIELD_MREAD_16(a, lsbn, fw, rv) (rv = FIELD_MGET_16((a),   (lsbn), (fw)))
#define FIELD_MREAD_32(a, lsbn, fw, rv) (rv = FIELD_MGET_32((a),   (lsbn), (fw)))

#define FIELD_SET(value, ls_bit_number, field_width, write_value) \
    do { \
        uint32_t  mask; \
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
            uint8_t  current_value = MGET_8(address); \
            FIELD_SET(current_value, ls_bit_number, field_width, write_value); \
            MWRITE_8(address, current_value); \
        } while (0)

#define GROUP_MREAD_I_8(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_8))
#define GROUP_MREAD_I_16(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_16))
#define GROUP_MREAD_I_32(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_32))

#define GROUP_MREAD_8(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_8))
#define GROUP_MREAD_16(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_16))
#define GROUP_MREAD_32(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_32))

#define GROUP_FIELD_MREAD_8(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_8))
#define GROUP_FIELD_MREAD_16(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_16))
#define GROUP_FIELD_MREAD_32(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, width, rdd_size_32))

#define GROUP_MWRITE_I_8(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_8)
#define GROUP_MWRITE_I_16(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_16)
#define GROUP_MWRITE_I_32(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_32)

#define GROUP_MWRITE_8(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_8)
#define GROUP_MWRITE_16(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_16)
#define GROUP_MWRITE_32(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_32)

#define GROUP_FIELD_MWRITE_8(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_8)
#define GROUP_FIELD_MWRITE_16(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_16)
#define GROUP_FIELD_MWRITE_32(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_32)

#if !defined(_CFE_) && !defined(RDP_SIM) && defined(DUAL_ISSUE)

volatile void *_xrdp__memcpy(volatile void *dst, const volatile void *src, size_t len) ;
volatile void *_xrdp__memset(volatile void *dst, int val, size_t len);

#endif /* !defined(_CFE_) && !defined(RDP_SIM) && defined(DUAL_ISSUE) */

#endif /* __ACCESS_MACROS_H_INCLUDED */
