/*
*  Copyright 2011, Broadcom Corporation
*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

/*                Copyright(c) 2002-2010 Broadcom Corporation                 */

#if !defined(Teknovus_h)
#define Teknovus_h
#if defined(__cplusplus)
extern "C" {
#endif
#ifdef __KERNEL__
#include <linux/delay.h>
#endif

#include "bcm_epon_common.h"

// BITFIELD_ENDIAN_LITTLE
/// _LITTLE, for compilers that lay out bitfields LSB first,  
/// or _BIG, for MSB first, as the case may be.  May be better
/// defined on the compiler command line for flexibility.
#ifdef __LITTLE_ENDIAN__
#endif

#ifndef BULK
#define BULK
#endif
#ifndef FAST
#define FAST
#endif
#ifndef CODE
#define CODE
#endif


#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif


#ifndef LOW_TRUE
#define LOW_TRUE  0
#endif
#ifndef HIGH_FALSE
#define HIGH_FALSE 1
#endif


#ifndef NULL
#define NULL 0
#endif

#ifdef __LITTLE_ENDIAN
#define CPU_ENDIAN_LITTLE
#define BITFIELD_ENDIAN_LITTLE
#else
#define CPU_ENDIAN_BIG
#endif

#define COMPILER_IS_32_BIT
#define _ARC  COMPILER_IS_32_BIT

#ifdef __KERNEL__
#ifndef DelayMs
#define DelayMs(n) mdelay(n)
#endif

#ifndef DelayUs
#define DelayUs(n) udelay(n)
#endif

#ifndef DelayNs
#define DelayNs(n) ndelay(n)
#endif
#endif

/*****************/
/* Generic swaps */
/*****************/
////////////////////////////////////////////////////////////////////////////////
/// \brief unsigned 32-bit Byte swapping
///
/// This macro will swap the byte order for a 32-bit value from Little Endian
/// to Big Endian or vice versa
///
/// \param x    32-bit value with byte order 0x01020304
///
/// \return     32-bit value with byte order 0x04030201
///
////////////////////////////////////////////////////////////////////////////////
#define _BswapU32(x)         ((((U32)(x) << 24) & 0xFF000000) | \
                            (((U32)(x)  >> 24) & 0x000000FF) | \
                            (((U32)(x)  << 8)  & 0x00FF0000) | \
                            (((U32)(x)  >> 8)  & 0x0000FF00) )


////////////////////////////////////////////////////////////////////////////////
/// \brief unsigned 16-bit Byte swapping
///
/// This macro will swap the byte order for a 16-bit value from Little Endian
/// to Big Endian or vice versa
///
/// \param x    16-bit value with byte order 0x0102
///
/// \return     16-bit value with byte order 0x0201
///
////////////////////////////////////////////////////////////////////////////////
#define _BswapU16(x)         ((((U16)(x) >> 8)  & 0x00FF) | \
                            (((U16) (x) << 8)  & 0xFF00))

/**************************/
/* swap2bytes, swap4bytes */
/**************************/
#if defined(CPU_ENDIAN_LITTLE)

#if defined(__ARMEL__)
static inline uint16_t BswapU16(uint16_t a)
{
    __asm__("rev16 %0, %1" : "=r" (a) : "r" (a));
    return a;
}

#if defined(CONFIG_ARM64)
static inline uint32_t BswapU32(uint32_t a)
{
    __asm__("rev32 %0, %1" : "=r" (a) : "r" (a));
    return a;
}
#else
static inline uint32_t BswapU32(uint32_t a)
{
    __asm__("rev %0, %1" : "=r" (a) : "r" (a));
    return a;
}
#endif
#else
#define BswapU16(x)  _BswapU16(x)
#define BswapU32(x)  _BswapU32(x)
#endif /* __ARMEL__ */

#else /* CPU_ENDIAN_LITTLE */

#define BswapU16(x)  (x)
#define BswapU32(x)  (x)

#endif /* CPU_ENDIAN_LITTLE */

#define EPON_NTOHS(x)		BswapU16(x)
#define EPON_NTOHL(x)		BswapU32(x)

#define EPON_HTONS(x)		BswapU16(x)
#define EPON_HTONL(x)		BswapU32(x)


// bit field manipulation
#define TkGetField(field, var)  (((unsigned int)(var) & field##Msk) >> field##Sft)
#define TkPutField(field, var)  (((unsigned int)(var) << field##Sft) & field##Msk)

#define TkOvwrtField(field, fieldVal, regVal)              \
    ((regVal) & ~field##Msk) | (((unsigned int)(fieldVal) << field##Sft) & field##Msk)

#define TkMaxValField(field) TkGetField(field, field##Msk)
#define FullMsk32 0xFFFFFFFFUL
#define MakeMsk32(Msb,Lsb) (((FullMsk32)>>(31-(Msb)+(Lsb))) << (Lsb))
#define MakeSft32(Msb,Lsb) (Lsb)

#define __GetBit(b, p) ((0x##b##UL & (1UL << (4*p))) >> (3*p))
#define TkBits(b) \
    __GetBit(b,0) | __GetBit(b,1) | __GetBit(b,2) | __GetBit(b,3) |     \
    __GetBit(b,4) | __GetBit(b,5) | __GetBit(b,6) | __GetBit(b,7)

/// provides access to bytes of a U16
typedef union
    {
    U16 u16;
    U8  array[2];
#if defined(CPU_ENDIAN_BIG)
    struct { U8 msb; U8 lsb; } bytes;
#elif defined(CPU_ENDIAN_LITTLE)
    struct { U8 lsb; U8 msb; } bytes;
#else
#error Must define CPU_ENDIAN_BIG or _LITTLE
#endif
    } PACK MultiByte16;

/// provides access to words/bytes of a U32
typedef union
    {
    U32 u32;
    U8  array[4];
    U16 warray[2];
#ifndef _BYTE_ORDER_LITTLE_ENDIAN_
    struct { MultiByte16 msw; MultiByte16 lsw; } words;
#else
    struct { MultiByte16 lsw; MultiByte16 msw; } words;
#endif
    } PACK MultiByte32;


/// provides access to words/bytes of a U48
typedef union
    {
    U48 u48;
    U8  array[6];
    U16 warray[3];
#if defined(CPU_ENDIAN_BIG)
    struct { MultiByte16 msw; MultiByte32 lsw; } words;
#elif defined(CPU_ENDIAN_LITTLE)
    struct { MultiByte32 lsw; MultiByte16 msw; } words;
#else
#error Must define CPU_ENDIAN_BIG or _LITTLE
#endif
    } PACK MultiByte48;


/// provides access to words/bytes of a U48
typedef union
    {
    U64 u64;
    U32 dword[2];
    U16 warray[4];
    U8  array[8];
#if defined(CPU_ENDIAN_BIG)
    struct { MultiByte32 msw; MultiByte32 lsw; } words;
    struct { U16 hiWord; U48 val; } u48;
#elif defined(CPU_ENDIAN_LITTLE)
    struct { MultiByte32 lsw; MultiByte32 msw; } words;
    struct { U48 val; U16 hiWord; } u48;
#else
#error Must define CPU_ENDIAN_BIG or _LITTLE
#endif
    } PACK MultiByte64;

typedef union
    {
    U64 qword[2];
    U32 dword[4];
    U16 word[8];
    U8  byte[16];
    } PACK MultiByte128;

////////////////////////////////////////////////////////////////////////////////
/// \brief Test whether the given bits are set in a value
///
/// \param val  The value to test
/// \param bits The bits to test for
///
/// \return
/// TRUE if all bits are set in the value, FALSE otherwise
////////////////////////////////////////////////////////////////////////////////
#define TestBitsSet(val, bits)  (((val) & (bits)) == (bits))


////////////////////////////////////////////////////////////////////////////////
/// \brief Test whether any of the given bits are set in a value
///
/// \param val  The value to test
/// \param bits The bits to test for
///
/// \return
/// TRUE if any of the bits are set in the value, FALSE otherwise
////////////////////////////////////////////////////////////////////////////////
#define TestBitsAny(val, bits)  (((val) & (bits)) != 0)


////////////////////////////////////////////////////////////////////////////////
/// \brief Test if t1 is less than t2, accounting for over/underflow
///
/// \param t1   First value
/// \param t2   Second value
///
/// \return
/// TRUE if t1 < t2, FALSE otherwise
////////////////////////////////////////////////////////////////////////////////
#define U16LessThan(t1, t2)     ((U16)(((U16)(t1) - (U16)(t2))) >= 0x8000)
#define U32LessThan(t1, t2)     ((U32)(((U32)(t1) - (U32)(t2))) >= 0x80000000)


////////////////////////////////////////////////////////////////////////////////
/// \brief  Convert a pointer to a U24 to a U32 value
///
/// \param  p   Pointer to U24
///
/// \return U32 value of U24
////////////////////////////////////////////////////////////////////////////////
#define U24ToU32(p)                     \
    ((((U32)((U8*)(p))[0]) << 16) +     \
     (((U32)((U8*)(p))[1]) << 8) +      \
     ((U32)((U8*)(p))[2]))


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the difference between p1 and p2 in bytes
///
/// \param p1   First pointer
/// \param p2   Second pointer
///
/// \return
/// p1 - p2 in bytes
////////////////////////////////////////////////////////////////////////////////
#define PointerDiff(p1, p2)     (((U8*)(p1)) - ((U8*)(p2)))


////////////////////////////////////////////////////////////////////////////////
/// \brief Add length bytes to pointer p
///
/// \param p    The pointer
/// \param len  Number of bytes to add to pointer
///
/// \return
/// p + len bytes
////////////////////////////////////////////////////////////////////////////////
#define PointerAdd(p, len)      (((U8*)(p)) + (len))


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the minimum of a and b
///
/// \param a    First value
/// \param b    Second value
///
/// \return
/// minimum of a and b
////////////////////////////////////////////////////////////////////////////////
#define Min(a, b)              (((a) < (b)) ? (a) : (b))


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the maximum of a and b
///
/// \param  a   First value
/// \param  b   Second value
///
/// \return maximum of a and b
////////////////////////////////////////////////////////////////////////////////
#define Max(a, b)              (((a) > (b)) ? (a) : (b))


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the ceiling of a / b
///
/// \param a    First value
/// \param b    Second value
///
/// \return
/// ceiling of a / b
////////////////////////////////////////////////////////////////////////////////
#define Ceil(a, b)              (((a) + ((b) - 1)) / (b))


////////////////////////////////////////////////////////////////////////////////
/// \brief  Perform safe subtraction of unsigned values
///
/// \param a    First value
/// \param b    Second value
///
/// \return a - b, or 0 if b > a
////////////////////////////////////////////////////////////////////////////////
#define SafeSub(a, b)              (((a) > (b)) ? ((a) - (b)) : 0)


#define ArrayLength(arr) (sizeof(arr) / sizeof(arr[0]))


////////////////////////////////////////////////////////////////////////////////
/// \brief unsigned enum comparision
///
/// \param a    First value
/// \param b    Second value
///
/// \return
/// a < b
////////////////////////////////////////////////////////////////////////////////
#if defined (_ARC)
#define EnumLessThan(a, b)              ((a) < (b))
#else
//lint -emacro(568,EnumLessThan)
#define EnumLessThan(a, b)              (((a) >= 0) && ((a) < (b)))
#endif

extern U32 volatile __REGISTER_SPACE[];
	
#if !defined(REGSIMULATE)
#define ORENREG_BASE       0x12000000
#define MACREGSPACE          0x1
#define EPONREG_BASE        (U32)(0x130fc000 | 0xA0000000)
#else
#define MACREGSPACE         0x600
#define EPONREG_BASE        (U32)(&__REGISTER_SPACE[0])//0x130fc000//
#endif
#define TkOnuAddr(off)      (EPONREG_BASE + ((off) << 2))
#define TkOnuReg(off)       (*(volatile U32 *)TkOnuAddr(off))
#define TkOnuTable(off)     ((volatile U32 *)TkOnuAddr(off))


/// *addr
extern 
U32 OnuRegRead (volatile U32* addr);

/// *addr = val;
extern 
U32 OnuRegWrite (volatile U32* addr, U32 val);


extern
U32 OnuRegAnd (volatile U32* addr, U32 val);


/// (*addr & ~val)
extern
U32 OnuRegAndNot (volatile U32* addr, U32 val);



/// *addr |= val;
extern 
U32 OnuRegOrEq (volatile U32* addr, U32 val);


/// *addr &= val;
extern 
U32 OnuRegAndEq (volatile U32* addr, U32 val);


/// *addr &= ~val;
extern 
U32 OnuRegAndEqNot (volatile U32* addr, U32 val);

#define OnuRegBitsSet(reg, bits)    TestBitsSet(OnuRegRead(reg), (bits))


#define OnuRegFieldRead(addr, field) \
		(((U32)(OnuRegRead((addr))) & field##Msk) >> field##Sft)

#define OnuRegFieldWrite(addr, field, val ) \
		OnuRegWrite ((addr),                            \
                (OnuRegRead((addr)) & ~field##Msk) |        \
                (((U32)(val) << field##Sft) & field##Msk))

#define OnuRegTableRead(addr, index) \
	OnuRegRead(&(addr[(index) % addr##Count]))
#define OnuRegTableAnd(addr, index, val) \
	OnuRegAnd(&(addr[(index) % addr##Count]) , (val))
#define OnuRegTableAndNot(addr, index, val) \
	OnuRegAnd(&(addr[(index) % addr##Count]) , (val))
#define OnuRegTableFieldRead(addr, index, field) \
	((U32)(OnuRegRead(&(addr[(index) % addr##Count])) & field##Msk) \
        >> field##Sft)
#define OnuRegTableWrite(addr, index, val) \
	OnuRegWrite(&(addr[(index) % addr##Count]), (val))
#define OnuRegTableOrEq(addr, index, val) \
	OnuRegOrEq(&(addr[(index) % addr##Count]) , (val))
#define OnuRegTableAndEq(addr, index, val) \
	 OnuRegAndEq(&(addr[(index) % addr##Count]) , (val))
#define OnuRegTableAndEqNot(addr, index, val) \
	OnuRegAndEqNot(&(addr[(index) % addr##Count]) , (val))
#define OnuRegTableWriteField(addr, index, field, val) \
	OnuRegFieldWrite(&(addr[(index) % addr##Count]) , field, (val))


#if defined(__cplusplus)
}
#endif

#endif // Teknovus
