#ifndef _PPC_BYTEORDER_H
#define _PPC_BYTEORDER_H

#include <asm/types.h>

#ifdef __GNUC__

static __inline__ unsigned ld_le16(const volatile unsigned short *addr)
{
	unsigned val;

	__asm__ __volatile__ ("lhbrx %0,0,%1" : "=r" (val) : "r" (addr), "m" (*addr));
	return val;
}

static __inline__ void st_le16(volatile unsigned short *addr, const unsigned val)
{
	__asm__ __volatile__ ("sthbrx %1,0,%2" : "=m" (*addr) : "r" (val), "r" (addr));
}

static __inline__ unsigned ld_le32(const volatile unsigned *addr)
{
	unsigned val;

	__asm__ __volatile__ ("lwbrx %0,0,%1" : "=r" (val) : "r" (addr), "m" (*addr));
	return val;
}

static __inline__ void st_le32(volatile unsigned *addr, const unsigned val)
{
	__asm__ __volatile__ ("stwbrx %1,0,%2" : "=m" (*addr) : "r" (val), "r" (addr));
}

/* alas, egcs sounds like it has a bug in this code that doesn't use the
   inline asm correctly, and can cause file corruption. Until I hear that
   it's fixed, I can live without the extra speed. I hope. */
#if !(__GNUC__ >= 2 && __GNUC_MINOR__ >= 90)
#if 0
#  define __arch_swab16(x) ld_le16(&x)
#  define __arch_swab32(x) ld_le32(&x)
#else
static __inline__ __attribute__((const)) __u16 ___arch__swab16(__u16 value)
{
	__u16 result;

	__asm__("rlwimi %0,%1,8,16,23"
	    : "=r" (result)
	    : "r" (value), "0" (value >> 8));
	return result;
}

static __inline__ __attribute__((const)) __u32 ___arch__swab32(__u32 value)
{
	__u32 result;

	__asm__("rlwimi %0,%1,24,16,23\n\t"
	    "rlwimi %0,%1,8,8,15\n\t"
	    "rlwimi %0,%1,24,0,7"
	    : "=r" (result)
	    : "r" (value), "0" (value >> 24));
	return result;
}
#define __arch__swab32(x) ___arch__swab32(x)
#define __arch__swab16(x) ___arch__swab16(x)
#endif /* 0 */

#endif

/* The same, but returns converted value from the location pointer by addr. */
#define __arch__swab16p(addr) ld_le16(addr)
#define __arch__swab32p(addr) ld_le32(addr)

/* The same, but do the conversion in situ, ie. put the value back to addr. */
#define __arch__swab16s(addr) st_le16(addr,*addr)
#define __arch__swab32s(addr) st_le32(addr,*addr)

#endif /* __GNUC__ */

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define __BYTEORDER_HAS_U64__
#endif
#include <linux/byteorder/big_endian.h>

#endif /* _PPC_BYTEORDER_H */
