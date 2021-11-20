#ifndef __PKT_HDR_H_INCLUDED__
#define __PKT_HDR_H_INCLUDED__

/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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

#undef PRINT
#if defined( __KERNEL__ )
#include "bcm_OS_Deps.h"
#define  PRINT                  printk
#else
#include <stdint.h>             /* ISO C99 7.18 Integer types */
#include <stdio.h>
#define  PRINT                  printf
#define __force
#endif

#include "bcm_mm.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef  NULL
#define NULL                        ((void*)0)
#define NULL_IX8                    ((__force uint8_t)0xFF)
#ifndef NULL_STMT
#define NULL_STMT                   do { /* EMPTY BODY */ } while (0)
#endif

#define SECONDS                     * HZ
#define MINUTES                     * 60 SECONDS
#define KBYTES                      * 1024
#define MBYTES                      * 1024 KBYTES

#define REG_RD(addr,val)            (val)=(*(volatile uint32_t *)(addr))
#define REG_WR(addr,val)            (*(volatile uint32_t *)(addr))=(val)

#ifndef OFFSETOF
#define OFFSETOF(stype, member)     ((size_t) &((struct stype *)0)->member)
#endif
#define RELOC(base, stype, member)  ((base) + OFFSETOF(stype, member))

/* IP Dot Decimal Notation formating */
#define IP4DDN                      " <%03u.%03u.%03u.%03u>"
#define IP4PDDN                     " <%03u.%03u.%03u.%03u:%05u>"
#define IP4(ip)                     ((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1], \
                                    ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]

#define IP6HEX                  "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x>"
#define IP6PHEX                 "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%u>"

#define IP6(ip)                                               \
    ntohs(((uint16_t*)&ip)[0]), ntohs(((uint16_t*)&ip)[1]),   \
    ntohs(((uint16_t*)&ip)[2]), ntohs(((uint16_t*)&ip)[3]),   \
    ntohs(((uint16_t*)&ip)[4]), ntohs(((uint16_t*)&ip)[5]),   \
    ntohs(((uint16_t*)&ip)[6]), ntohs(((uint16_t*)&ip)[7])

#define MACHEX                  "<%02x:%02x:%02x:%02x:%02x:%02x>"
#define MAC(mac)                                                        \
   (mac)[0], (mac)[1], (mac)[2], (mac)[3], (mac)[4], (mac)[5]

#define DIV32(ix)                   ((ix) >> 5)
#define MOD32(ix)                   ((ix) & 31)
#define MUL32(ix)                   ((ix) << 5)

#ifndef ROUNDDN
#define ROUNDDN(addr, align)        ( (addr) & ~((align) - 1) )
#endif
#ifndef ROUNDUP
#define ROUNDUP(addr, align)        ( ((addr) + (align) - 1) & ~((align) - 1) )
#endif

/*
 * Hooks: Generic Function Pointer 
 */
typedef int (* HOOKV)( void );                              /* int func ptr no args */
typedef int (* HOOKP)( void * arg_vp );                     /* int func ptr with one void * arg */
typedef int (* HOOKP2)( void * arg_vp, void * arg_vp2 );    /* int func ptr with two void * arg */
typedef int (* HOOKP3)( void * arg_vp, void * arg_vp2,
        void * arg_vp3);                                    /* int func ptr with three void * arg */
typedef int (* HOOKP32)( void * arg_vp, uint32_t arg32 );   /* int func ptr with one void * and one uint32_t arg */
typedef int (* HOOK16)( uint16_t arg16 );                   /* int func ptr with uint16_t arg */
typedef int (* HOOK32)( uint32_t arg32 );                   /* int func ptr with uint32_t arg */
typedef int (* HOOK2PARM)( uint32_t parm1, uint32_t parm2 );/* int func ptr with two parameters */
typedef int (* HOOK3PARM)( uint32_t parm1, unsigned long parm2, 
        unsigned long parm3);                               /* int func ptr with three parameters */
typedef int (* HOOK4PARM)( uint32_t parm1, unsigned long parm2, 
        unsigned long parm3, unsigned long parm4);          /* int func ptr with four parameters */

/*
 * 6 Byte BRCM tag and 4 byte BRCM tag. This structure provides a layout where
 * the bcmhdr follws the 14byte ethhdr and the ether type now becomes h_proto.
 */
typedef uint16_t hProto_t;

struct bcmhdr {
    uint32_t brcm_tag;
    uint16_t h_proto;
} __attribute__((packed));

struct bcmhdr2 {
    uint16_t brcm_tag;
    uint16_t h_proto;
} __attribute__((packed));


/*
 *------------------------------------------------------------------------------
 * Function     : _short2buf
 * Description  : Read a short int into a character buffer.
 *------------------------------------------------------------------------------
 */
static inline void _short2buf(uint8_t * buf, uint16_t val)
{
    buf[0] = (__force uint8_t)(val >> 8);
    buf[1] = (__force uint8_t)(val >> 0);
}

/*
 *------------------------------------------------------------------------------
 * Function     : _long2buf
 * Description  : Read an int into a character buffer.
 *------------------------------------------------------------------------------
 */
static inline
void _long2buf(uint8_t * buf, uint32_t val)
{
    buf[0] = (__force uint8_t)(val >> 24);
    buf[1] = (__force uint8_t)(val >> 16);
    buf[2] = (__force uint8_t)(val >>  8);
    buf[3] = (__force uint8_t)(val >>  0);
}

/*
 *------------------------------------------------------------------------------
 * Function     : _buf2short
 * Description  : Convert a character buffer of two bytes to a short int.
 *------------------------------------------------------------------------------
 */
static inline
uint16_t _buf2short(uint8_t * buf)
{
    return ( (buf[0] << 8) | (buf[1] << 0) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : _buf2long
 * Description  : Convert a character buffer of 4 bytes to an int.
 *------------------------------------------------------------------------------
 */
static inline
uint32_t _buf2long(uint8_t * buf)
{
    return (__force uint32_t)
           ( (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3] << 0) );
}

#if defined(CONFIG_CPU_LITTLE_ENDIAN) || defined(CONFIG_ARM)
/*
 *------------------------------------------------------------------------------
 * Function     : _read32_align16
 * Description  : Read a 32bit value from a 16 bit aligned data stream.
 *------------------------------------------------------------------------------
 */
static inline
uint32_t _read32_align16(uint16_t * from)
{
    return (__force uint32_t)( (from[1] << 16) | (from[0]) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : _write32_align16
 * Description  : Write a 32bit value to a 16bit aligned data stream.
 *------------------------------------------------------------------------------
 */
static inline
void _write32_align16( uint16_t * to, uint32_t from )
{
    to[1] = (__force uint16_t)(from >> 16);
    to[0] = (__force uint16_t)(from >>  0);
}
#else
/*
 *------------------------------------------------------------------------------
 * Function     : _read32_align16
 * Description  : Read a 32bit value from a 16 bit aligned data stream.
 *------------------------------------------------------------------------------
 */
static inline
uint32_t _read32_align16(uint16_t * from)
{
    return (__force uint32_t)( (from[0] << 16) | (from[1]) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : _write32_align16
 * Description  : Write a 32bit value to a 16bit aligned data stream.
 *------------------------------------------------------------------------------
 */
static inline
void _write32_align16( uint16_t * to, uint32_t from )
{
    to[0] = (__force uint16_t)(from >> 16);
    to[1] = (__force uint16_t)(from >>  0);
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function     : __u32cpy
 * Description  : 32bit aligned data copy of uncommon lengths.
 * CAUTION      : No check is done to ensure that bytes is even
 *------------------------------------------------------------------------------
 */
static inline
void __u32cpy( uint32_t * dst_p, const uint32_t * src_p, uint32_t bytes )
{
    // assuming: (bytes % sizeof(uint32_t) == 0 !!!
    do {
        *dst_p++ = *src_p++;
    } while ( bytes -= sizeof(uint32_t) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : __u16cpy
 * Description  : 16bit aligned data copy of uncommon lengths.
 * CAUTION      : No check is done to ensure that bytes is even
 *------------------------------------------------------------------------------
 */
static inline
void __u16cpy( uint16_t * dst_p, const uint16_t * src_p, uint32_t bytes )
{
    if (bytes)
    {
        // assuming: (bytes % sizeof(uint16_t) == 0 !!!
        do {
            *dst_p++ = *src_p++;
        } while ( bytes -= sizeof(uint16_t) );
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : _u16cpy
 * Description  : 16bit aligned data copy of a few small lengths.
 * CAUTION      : Code bloat. Use only for small sizes.
 *------------------------------------------------------------------------------
 */
static inline
void _u16cpy( uint16_t * dst, const uint16_t * src, const uint32_t bytes )
{
    switch ( bytes )    /* common lengths, using half word alignment copy */
    {
        case 14: *(dst+ 6)=*(src+ 6);
        case 12: *(dst+ 5)=*(src+ 5);
        case 10: *(dst+ 4)=*(src+ 4);
        case  8: *(dst+ 3)=*(src+ 3);
        case  6: *(dst+ 2)=*(src+ 2);
        case  4: *(dst+ 1)=*(src+ 1);
        case  2: *(dst+ 0)=*(src+ 0);
                 break;
        default:
                 __u16cpy( dst, src, (uint32_t)bytes );
                 break;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : _blkcpy
 * Description  : 32bit or 16bit aligned data copy when src is word aligned
 *------------------------------------------------------------------------------
 */
static inline
void _blkcpy( void * dst, const void * src, const uint32_t bytes )
{

    /*TODO add __u64cpy for 64bit platfroms */

      /* destination pointer is word aligned ? */
    if ( ((uintptr_t)dst & (sizeof(uint32_t)-1)) == 0 )
        __u32cpy( (uint32_t*)dst, (const uint32_t *)src, bytes );
    else
        __u16cpy( (uint16_t*)dst, (const uint16_t *)src, bytes );
}

/*
 *------------------------------------------------------------------------------
 * Function     : __u16cmp
 * Description  : 16bit aligned data compare of uncommon lengths.
 * CAUTION      : No check is done to ensure that bytes is even
 *------------------------------------------------------------------------------
 */
static inline
int __u16cmp( uint16_t * dst_p, const uint16_t * src_p, uint32_t bytes )
{
    // assuming: (bytes % sizeof(uint16_t) == 0 !!!
    do {
        if ( *dst_p++ != *src_p++ ) return -1;
    } while ( bytes -= sizeof(uint16_t) );

    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _u16cmp
 * Description  : 16bit aligned data compare of a few small lengths.
 * CAUTION      : Code bloat. Use only for small sizes.
 *------------------------------------------------------------------------------
 */
static inline
int _u16cmp( uint16_t * dst, const uint16_t * src, const uint32_t bytes )
{
    switch ( bytes )    /* common lengths, using half word alignment copy */
    {
        case 14: if (*(dst+ 6) != *(src+ 6)) break;
        case 12: if (*(dst+ 5) != *(src+ 5)) break;
        case 10: if (*(dst+ 4) != *(src+ 4)) break;
        case  8: if (*(dst+ 3) != *(src+ 3)) break;
        case  6: if (*(dst+ 2) != *(src+ 2)) break;
        case  4: if (*(dst+ 1) != *(src+ 1)) break;
        case  2: if (*(dst+ 0) != *(src+ 0)) break;
                 return 0;
        default:
                 return __u16cmp( dst, src, (uint32_t)bytes );
    }
    return -1;
}


/*
 *------------------------------------------------------------------------------
 * Internet Checksum: 1's complement of "1's complement sum"
 * Ref: http://www.netfor2.com/checksum.html
 *------------------------------------------------------------------------------
 */

/*
 *------------------------------------------------------------------------------
 * Function     : _compute_icsum16
 * Description  : Compute delta checksum for an incremental 16bit modification.
 *------------------------------------------------------------------------------
 */
static inline
uint16_t _compute_icsum16(uint16_t csum16, uint16_t old16, uint16_t new16)
{
    register uint32_t csum32;

    /* build delta checksum */
    csum32 = ( (__force uint32_t)(csum16 ^ 0xFFFF)
             + (__force uint32_t)(old16  ^ 0xFFFF)
             + (__force uint32_t)new16
             );
    while (csum32 >> 16)/* factor in carry over to effect 1's complement sum */
        csum32 = (csum32 & 0xFFFF) + (csum32 >> 16);

    return ((__force uint16_t)csum32 ^ 0xFFFF); /* 1's complement */
}

/*
 *------------------------------------------------------------------------------
 * Function     : _compute_icsum32
 * Description  : Compute delta checksum for an incremental 32bit modification.
 *------------------------------------------------------------------------------
 */
static inline
uint16_t _compute_icsum32(uint16_t csum16, uint32_t old32, uint32_t new32)
{
    register uint16_t *optr = (uint16_t *)&old32;
    register uint16_t *nptr = (uint16_t *)&new32;
    register uint32_t csum32;

    /* build delta checksum */
    csum32 = ( (__force uint32_t)(csum16  ^ 0xFFFF)
             + (__force uint32_t)(optr[0] ^ 0xFFFF)
             + (__force uint32_t)(optr[1] ^ 0xFFFF)
             + (__force uint32_t)nptr[0]
             + (__force uint32_t)nptr[1]
             );
    while (csum32 >> 16)/* factor in carry over to effect 1's complement sum */
        csum32 = (csum32 & 0xFFFF) + (csum32 >> 16);

    return ((__force uint16_t)csum32 ^ 0xFFFF); /* 1's complement */
}

/*
 *------------------------------------------------------------------------------
 * Function     : _compute_icsum 
 * Description  : Compute delta checksum for an incremental 16bit and a 32bit
 *                modification.
 *------------------------------------------------------------------------------
 */
static inline
uint16_t _compute_icsum(uint16_t csum16, uint32_t old32, uint32_t new32,
                        uint16_t old16, uint16_t new16)
{
    register uint16_t *optr = (uint16_t *)&old32;
    register uint16_t *nptr = (uint16_t *)&new32;
    register uint32_t csum32;

    /* build delta checksum */
    csum32 = ( (__force uint32_t)(csum16  ^ 0xFFFF)
             + (__force uint32_t)(optr[0] ^ 0xFFFF)
             + (__force uint32_t)(optr[1] ^ 0xFFFF)
             + (__force uint32_t)(old16   ^ 0xFFFF)
             + (__force uint32_t)nptr[0]
             + (__force uint32_t)nptr[1]
             + (__force uint32_t)new16
             );
    while (csum32 >> 16)/* factor in carry over to effect 1's complement sum */
        csum32 = (csum32 & 0xFFFF) + (csum32 >> 16);

    return ((__force uint16_t)csum32 ^ 0xFFFF);    /* 1's complement */
}

/*
 *------------------------------------------------------------------------------
 * Function     : _apply_icsum
 * Description  : Apply a delta checksum computed from incremental modifications
 *------------------------------------------------------------------------------
 */
static inline
uint16_t _apply_icsum( uint16_t csum16, uint32_t delta32)
{
    uint32_t csum32 = (__force uint32_t)csum16 + delta32;

    while (csum32 >> 16)/* factor in carry over to effect 1's complement sum */
        csum32 = (csum32 & 0xFFFF) + (csum32 >> 16);

    return ((__force uint16_t)csum32);
}


/*
 *------------------------------------------------------------------------------
 * Function     : _crc16ccitt
 * Description  : Build a CCITT CRC16 of a character buffer of a specified size
 *------------------------------------------------------------------------------
 */
static inline
uint16_t _crc16ccitt(uint8_t * pBuffer, uint32_t uBufSize)
{
    uint32_t i;
    uint16_t uCcitt16;

    uCcitt16 = 0xFFFF;

    for (i = 0; i < uBufSize; i++)
    {
        uCcitt16 = (uCcitt16 >> 8) | (uCcitt16 << 8);
        uCcitt16 ^= pBuffer[i];
        uCcitt16 ^= (uCcitt16 & 0xFF) >> 4;
        uCcitt16 ^= (uCcitt16 << 8) << 4;
        uCcitt16 ^= ((uCcitt16 & 0xFF) << 4) << 1;
    }

    return uCcitt16;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _hash 
 * Description  : Computes a simple hash from a 32bit value.
 *------------------------------------------------------------------------------
 */
static inline
uint32_t _hash( uint32_t hash_val )
{
    hash_val ^= ( hash_val >> 16 );
    hash_val ^= ( hash_val >>  8 );
    hash_val ^= ( hash_val >>  3 );

    return ( hash_val );
}

/*
 *******************************************************************************
 *      Bit Manipulation routines
 *******************************************************************************
 */
static inline
void _bitprint(uint32_t word)
{
    int i;
    uint32_t mask = 0x1 << 31;
    for ( i=1; i <= 32; i++ )
    {
        if ( word & mask ) PRINT("1"); else PRINT("0");
        if (( i % 8 ) == 0) PRINT("  ");
        mask = mask >> 1;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : _count_leading_zeros
 * Description  : Count Leading Zeros in a 32-bit bitmap
 *------------------------------------------------------------------------------
 */
static inline
int _count_leading_zeros(uint32_t bitmap)
{
#if defined(__mips__) && defined (__GNUC__) && defined( __KERNEL__ )
    register int zeros;
        /* Count leading (higher order) zeros */
    __asm__ volatile (
                "clz    %0, %1 \n"
                : "=r" (zeros)
                : "r"  (bitmap));
    return zeros;
#else
    int shifts = 0;
    while (bitmap) { shifts++; bitmap >>= 1; }
    return (32U - shifts);
#endif  /* ! defined(__KERNEL__) */
}

/*
 *------------------------------------------------------------------------------
 *
 * Function     : _find_trailing_set_bit
 * Description  :
 * Find Trailing Set Bit in a 32bit bitmap
 *
 * Return the position of the trailing set bit in I, or -ve if none are set.
 * The least-significant bit is position 0, the most-significant is 31
 *
 *  E.g.                         Higher order   -------  Lower order
 *  for a bitmap 0x0F0FF080 = 0b 00001111 00001111 11110000 10000000
 *                               ^                          ^      ^
 *                              31                          7      0
 *      7 = _find_trailing_set_bit(0x0F0FF080)
 *     -1 = _find_trailing_set_bit 0x0)
 *
 * PS. In the case of ffs() from string.h, the value returned is 1 .. 32.
 *     where a returned value 0 implies that no bit is set.
 *
 * Using MIPS assembly instruction clz, we need 5 compute shadow instructions
 *
 *------------------------------------------------------------------------------
 */
static inline
int _find_trailing_set_bit(uint32_t bitmap)
{
    int zeros;
        /* Clear all ones except for the trailing "lowest order" 1 */
    bitmap = (bitmap ^ (bitmap - 1)) & bitmap;
        /* bitmap would now have a single 1 bit and the rest all 0s */
    zeros = _count_leading_zeros(bitmap);
    return 31U - zeros;  /* excluding the 1, trailing zeros */
}

/*
 *------------------------------------------------------------------------------
 *
 * Function     : _find_trailing_set_bit_64
 * Description  :
 * Find Trailing Set Bit in a 64bit bitmap
 *
 * Return the position of the trailing set bit in I, or -ve if none are set.
 * The least-significant bit is position 0, the most-significant is 63
 *
 *------------------------------------------------------------------------------
 */
static inline
int _find_trailing_set_bit_64(uint64_t bitmap)
{
    int zeros;
    uint32_t bitmap_l, bitmap_h;

    bitmap_l = (uint32_t)(bitmap & 0xffffffff);
    zeros = _find_trailing_set_bit(bitmap_l);
    if(zeros >= 0)
        return zeros;

    bitmap_h = (uint32_t)(bitmap>>32);
    zeros = _find_trailing_set_bit(bitmap_h);
    if(zeros >= 0)
        return (zeros + 32U);
    
    return zeros;    
}

/*
 *------------------------------------------------------------------------------
 * Function     : _count_set_bits
 * Description  : Count set bits in a 32-bit bitmap
 *------------------------------------------------------------------------------
 */

static inline
uint32_t _count_set_bits(uint32_t bitmap)
{
    register uint32_t count;
        /* loops as many times as set bits, hence efficieny */
    for (count=0; bitmap; count++)
        bitmap &= bitmap - 1;   /* Clear least significant bit */
    return count;
}


static inline
uint32_t __count_set_bits(uint32_t bitmap)
{
    uint32_t count = bitmap;
    count = ((count & 0x55555555) + ((count >> 1) & 0x55555555));
    count = ((count & 0x33333333) + ((count >> 2) & 0x33333333));
    count = ((count & 0x0f0f0f0f) + ((count >> 4) & 0x0f0f0f0f));
    count %= 255U;
    return count;
}

/*
 *******************************************************************************
 *                      Double Linked List Macros
 *******************************************************************************
 *
 * All dll operations must be performed on a pre-initialized node.
 * Inserting an uninitialized node into a list effectively initialized it.
 *
 * When a node is deleted from a list, you may initialize it to avoid corruption
 * incurred by double deletion. You may skip initialization if the node is
 * immediately inserted into another list.
 *
 * By placing a Dll_t element at the start of a struct, you may cast a PDll_t
 * to the struct or vice versa.
 *
 * Example of declaring an initializing someList and inserting nodeA, nodeB
 *
 *     typedef struct item {
 *         Dll_t node;
 *         int someData;
 *     } Item_t;
 *     Item_t nodeA, nodeB, nodeC;
 *     nodeA.someData = 11111, nodeB.someData = 22222, nodeC.someData = 33333;
 *
 *     Dll_t someList;
 *     dll_init( &someList );
 *
 *     dll_append(  &someList, (PDll_t) &nodeA );
 *     dll_prepend( &someList, &nodeB.node );
 *     dll_insert( (PDll_t)&nodeC, &nodeA.node );
 *
 *     dll_delete( (PDll_t) &nodeB );
 *
 * Example of a for loop to walk someList of node_p
 *
 *   extern void mydisplay( Item_t * item_p );
 *
 *   PDll_t item_p, next_p;
 *   for ( item_p = dll_head_p( &someList );
 *         ! dll_end( &someList, item_p);
 *         item_p = next_p )
 *   {
 *       next_p = dll_next_p(item_p);
 *       ... use item_p at will, including removing it from list ...
 *       mydisplay( (PItem_t)item_p );
 *   }
 *
 */
#ifndef _dll_t_
#define _dll_t_

#if !defined(_envelope_of)
/* derived from container_of, without "const", for gcc -Wcast-qual compile */
#define _envelope_of(ptr, type, member) \
({ \
	typeof(((type *)0)->member) *__mptr = (ptr); \
	(type *)((char *)__mptr - offsetof(type, member)); \
})
#endif /* _envelope_of */


typedef struct dll_t dll_t; /* common to wlan bcmutils.h and brcm_dll.h */
typedef struct dll_t {
	dll_t * next_p;
	dll_t * prev_p;
} Dll_t, * PDll_t;

#define DLL_STRUCT_INITIALIZER(struct_name, dll_name) \
	{ .next_p = &(struct_name).dll_name, .prev_p = &(struct_name).dll_name }

#define dll_init(node_p)        ((node_p)->next_p = (node_p)->prev_p = (node_p))

/* dll macros returing a "dll_t *" */
#define dll_head_p(list_p)      ((list_p)->next_p)
#define dll_tail_p(list_p)      ((list_p)->prev_p)

#define dll_next_p(node_p)      ((node_p)->next_p)
#define dll_prev_p(node_p)      ((node_p)->prev_p)

#define dll_empty(list_p)       ((list_p)->next_p == (list_p))
#define dll_end(list_p, node_p) ((list_p) == (node_p))

/* inserts the node new_p "after" the node at_p */
#define dll_insert(new_p, at_p) \
({ \
	(new_p)->next_p = (at_p)->next_p; \
	(new_p)->prev_p = (at_p); \
	(at_p)->next_p = (new_p); \
	(new_p)->next_p->prev_p = (new_p); \
})

#define dll_append(list_p, node_p)      dll_insert((node_p), dll_tail_p(list_p))
#define dll_prepend(list_p, node_p)     dll_insert((node_p), (list_p))

/* deletes a node from any list that it "may" be in, if at all. */
#define dll_delete(node_p) \
({ \
	(node_p)->prev_p->next_p = (node_p)->next_p; \
	(node_p)->next_p->prev_p = (node_p)->prev_p; \
})

/**
 * dll_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define dll_for_each(pos, head) \
	for (pos = (head)->next_p; pos != (head); pos = pos->next_p)

/* Take all elements of list A and join them to the tail of list B.
 * List A must not be empty and list A will be returned as an empty list.
 */
#define dll_join(listA_p, listB_p) \
({ \
	dll_t *_listB_p = (listB_p); \
	dll_t *headA_p  = dll_head_p(listA_p); \
	dll_t *tailA_p  = dll_tail_p(listA_p); \
	dll_t *tailB_p  = dll_tail_p(listB_p); \
	/* Link up list B's tail to list A's head */ \
	headA_p->prev_p = tailB_p; \
	tailB_p->next_p = headA_p; \
	/* Make list A's tail to be list B's new tail */ \
	tailA_p->next_p = (listB_p); \
	_listB_p->prev_p = tailA_p; \
	dll_init(listA_p); \
})

#endif  /* ! defined(_dll_t_) */

#ifdef __cplusplus
}
#endif

#endif  /* defined(__PKT_HDR_H_INCLUDED__) */
