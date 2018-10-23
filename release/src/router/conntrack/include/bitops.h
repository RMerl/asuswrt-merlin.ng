#ifndef _BITOPS_H_
#define _BITOPS_H_

#include <stdint.h>

static inline void set_bit_u32(int nr, uint32_t *addr)
{
	addr[nr >> 5] |= (1UL << (nr & 31));
}

static inline void unset_bit_u32(int nr, uint32_t *addr)
{
	addr[nr >> 5] &= ~(1UL << (nr & 31));
}

static inline int test_bit_u32(int nr, const uint32_t *addr)
{
	return ((1UL << (nr & 31)) & (addr[nr >> 5])) != 0;
}

static inline void set_bit_u16(int nr, uint16_t *addr)
{
	addr[nr >> 4] |= (1UL << (nr & 15));
}

static inline void unset_bit_u16(int nr, uint16_t *addr)
{
	addr[nr >> 4] &= ~(1UL << (nr & 15));
}

static inline int test_bit_u16(int nr, const uint16_t *addr)
{
	return ((1UL << (nr & 15)) & (addr[nr >> 4])) != 0;
}

#endif
