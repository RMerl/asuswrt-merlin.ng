#ifndef _I386_BYTEORDER_H
#define _I386_BYTEORDER_H

#include <asm/types.h>

#ifdef __GNUC__


static __inline__ __u32 ___arch__swab32(__u32 x)
{
	__asm__("bswap %0" : "=r" (x) : "0" (x));

	return x;
}

#define _constant_swab16(x) ((__u16)(				\
	(((__u16)(x) & (__u16)0x00ffU) << 8) |			\
	(((__u16)(x) & (__u16)0xff00U) >> 8)))

static __inline__ __u16 ___arch__swab16(__u16 x)
{
#if CONFIG_IS_ENABLED(X86_64)
	return _constant_swab16(x);
#else
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/ \
		: "=q" (x) \
		:  "0" (x)); \
		return x;
#endif
}

#define __arch__swab32(x) ___arch__swab32(x)
#define __arch__swab16(x) ___arch__swab16(x)

#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#endif /* __GNUC__ */

#include <linux/byteorder/little_endian.h>

#endif /* _I386_BYTEORDER_H */
