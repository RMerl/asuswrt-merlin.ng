/*
 * WARNING: Do *NOT* ever include this file, only for internal use!
 */
#ifndef _NFCT_BITOPS_H_
#define _NFCT_BITOPS_H_

static inline void set_bit(int nr, uint32_t *addr)
{
	addr[nr >> 5] |= (1UL << (nr & 31));
}

static inline void unset_bit(int nr, uint32_t *addr)
{
	addr[nr >> 5] &= ~(1UL << (nr & 31));
}

static inline void set_bit_u16(int nr, uint16_t *addr)
{
	addr[nr >> 4] |= (1UL << (nr & 15));
}

static inline void unset_bit_u16(int nr, uint16_t *addr)
{
	addr[nr >> 4] &= ~(1UL << (nr & 15));
}

static inline void
set_bitmask_u32(uint32_t *buf1, const uint32_t *buf2, int len)
{
	int i;

	for (i=0; i<len; i++)
		buf1[i] |= buf2[i];
}

static inline void
unset_bitmask_u32(uint32_t *buf1, const uint32_t *buf2, int len)
{
	int i;

	for (i=0; i<len; i++)
		buf1[i] &= ~buf2[i];
}

static inline int test_bit(int nr, const uint32_t *addr)
{
	return ((1UL << (nr & 31)) & (addr[nr >> 5])) != 0;
}

static inline int 
test_bitmask_u32(const uint32_t *buf1, const uint32_t *buf2, int len)
{
	int i;

	for (i=0; i<len; i++) {
		if ((buf1[i] & buf2[i]) != buf2[i]) {
			return 0;
		}
	}
	return 1;
}

static inline int
test_bitmask_u32_or(const uint32_t *buf1, const uint32_t *buf2, int len)
{
	int i;

	for (i=0; i<len; i++) {
		if (buf1[i] & buf2[i]) {
			return 1;
		}
	}
	return 0;
}

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#endif
