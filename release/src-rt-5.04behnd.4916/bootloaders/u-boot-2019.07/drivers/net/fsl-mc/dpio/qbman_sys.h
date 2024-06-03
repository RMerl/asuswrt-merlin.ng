/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Freescale Semiconductor
 */

/* qbman_sys_decl.h and qbman_sys.h are the two platform-specific files in the
 * driver. They are only included via qbman_private.h, which is itself a
 * platform-independent file and is included by all the other driver source.
 *
 * qbman_sys_decl.h is included prior to all other declarations and logic, and
 * it exists to provide compatibility with any linux interfaces our
 * single-source driver code is dependent on (eg. kmalloc). Ie. this file
 * provides linux compatibility.
 *
 * This qbman_sys.h header, on the other hand, is included *after* any common
 * and platform-neutral declarations and logic in qbman_private.h, and exists to
 * implement any platform-specific logic of the qbman driver itself. Ie. it is
 * *not* to provide linux compatibility.
 */

/* Trace the 3 different classes of read/write access to QBMan. #undef as
 * required. */
#undef QBMAN_CCSR_TRACE
#undef QBMAN_CINH_TRACE
#undef QBMAN_CENA_TRACE

/* Temporarily define this to get around the fact that cache enabled mapping is
 * not working right now. Will remove this after uboot could map the cache
 * enabled portal memory.
 */
#define QBMAN_CINH_ONLY

static inline void word_copy(void *d, const void *s, unsigned int cnt)
{
	uint32_t *dd = d;
	const uint32_t *ss = s;

	while (cnt--)
		*(dd++) = *(ss++);
}

/* Currently, the CENA support code expects each 32-bit word to be written in
 * host order, and these are converted to hardware (little-endian) order on
 * command submission. However, 64-bit quantities are must be written (and read)
 * as two 32-bit words with the least-significant word first, irrespective of
 * host endianness. */
static inline void u64_to_le32_copy(void *d, const uint64_t *s,
					unsigned int cnt)
{
	uint32_t *dd = d;
	const uint32_t *ss = (const uint32_t *)s;

	while (cnt--) {
		/* TBD: the toolchain was choking on the use of 64-bit types up
		 * until recently so this works entirely with 32-bit variables.
		 * When 64-bit types become usable again, investigate better
		 * ways of doing this. */
#if defined(__BIG_ENDIAN)
		*(dd++) = ss[1];
		*(dd++) = ss[0];
		ss += 2;
#else
		*(dd++) = *(ss++);
		*(dd++) = *(ss++);
#endif
	}
}
static inline void u64_from_le32_copy(uint64_t *d, const void *s,
					unsigned int cnt)
{
	const uint32_t *ss = s;
	uint32_t *dd = (uint32_t *)d;

	while (cnt--) {
#if defined(__BIG_ENDIAN)
		dd[1] = *(ss++);
		dd[0] = *(ss++);
		dd += 2;
#else
		*(dd++) = *(ss++);
		*(dd++) = *(ss++);
#endif
	}
}

/* Convert a host-native 32bit value into little endian */
#if defined(__BIG_ENDIAN)
static inline uint32_t make_le32(uint32_t val)
{
	return ((val & 0xff) << 24) | ((val & 0xff00) << 8) |
		((val & 0xff0000) >> 8) | ((val & 0xff000000) >> 24);
}
#else
#define make_le32(val) (val)
#endif
static inline void make_le32_n(uint32_t *val, unsigned int num)
{
	while (num--) {
		*val = make_le32(*val);
		val++;
	}
}

	/******************/
	/* Portal access  */
	/******************/
struct qbman_swp_sys {
	/* On GPP, the sys support for qbman_swp is here. The CENA region isi
	 * not an mmap() of the real portal registers, but an allocated
	 * place-holder, because the actual writes/reads to/from the portal are
	 * marshalled from these allocated areas using QBMan's "MC access
	 * registers". CINH accesses are atomic so there's no need for a
	 * place-holder. */
	void *cena;
	void __iomem *addr_cena;
	void __iomem *addr_cinh;
};

/* P_OFFSET is (ACCESS_CMD,0,12) - offset within the portal
 * C is (ACCESS_CMD,12,1) - is inhibited? (0==CENA, 1==CINH)
 * SWP_IDX is (ACCESS_CMD,16,10) - Software portal index
 * P is (ACCESS_CMD,28,1) - (0==special portal, 1==any portal)
 * T is (ACCESS_CMD,29,1) - Command type (0==READ, 1==WRITE)
 * E is (ACCESS_CMD,31,1) - Command execute (1 to issue, poll for 0==complete)
 */

static inline void qbman_cinh_write(struct qbman_swp_sys *s, uint32_t offset,
				    uint32_t val)
{
	__raw_writel(val, s->addr_cinh + offset);
#ifdef QBMAN_CINH_TRACE
	pr_info("qbman_cinh_write(%p:0x%03x) 0x%08x\n",
		s->addr_cinh, offset, val);
#endif
}

static inline uint32_t qbman_cinh_read(struct qbman_swp_sys *s, uint32_t offset)
{
	uint32_t reg = __raw_readl(s->addr_cinh + offset);

#ifdef QBMAN_CINH_TRACE
	pr_info("qbman_cinh_read(%p:0x%03x) 0x%08x\n",
		s->addr_cinh, offset, reg);
#endif
	return reg;
}

static inline void *qbman_cena_write_start(struct qbman_swp_sys *s,
						uint32_t offset)
{
	void *shadow = s->cena + offset;

#ifdef QBMAN_CENA_TRACE
	pr_info("qbman_cena_write_start(%p:0x%03x) %p\n",
		s->addr_cena, offset, shadow);
#endif
	BUG_ON(offset & 63);
	dcbz(shadow);
	return shadow;
}

static inline void qbman_cena_write_complete(struct qbman_swp_sys *s,
						uint32_t offset, void *cmd)
{
	const uint32_t *shadow = cmd;
	int loop;

#ifdef QBMAN_CENA_TRACE
	pr_info("qbman_cena_write_complete(%p:0x%03x) %p\n",
		s->addr_cena, offset, shadow);
	hexdump(cmd, 64);
#endif
	for (loop = 15; loop >= 0; loop--)
#ifdef QBMAN_CINH_ONLY
		__raw_writel(shadow[loop], s->addr_cinh +
					 offset + loop * 4);
#else
		__raw_writel(shadow[loop], s->addr_cena +
					 offset + loop * 4);
#endif
}

static inline void *qbman_cena_read(struct qbman_swp_sys *s, uint32_t offset)
{
	uint32_t *shadow = s->cena + offset;
	unsigned int loop;

#ifdef QBMAN_CENA_TRACE
	pr_info("qbman_cena_read(%p:0x%03x) %p\n",
		s->addr_cena, offset, shadow);
#endif

	for (loop = 0; loop < 16; loop++)
#ifdef QBMAN_CINH_ONLY
		shadow[loop] = __raw_readl(s->addr_cinh + offset
					+ loop * 4);
#else
		shadow[loop] = __raw_readl(s->addr_cena + offset
					+ loop * 4);
#endif
#ifdef QBMAN_CENA_TRACE
	hexdump(shadow, 64);
#endif
	return shadow;
}

static inline void qbman_cena_invalidate_prefetch(struct qbman_swp_sys *s,
						  uint32_t offset)
{
}

	/******************/
	/* Portal support */
	/******************/

/* The SWP_CFG portal register is special, in that it is used by the
 * platform-specific code rather than the platform-independent code in
 * qbman_portal.c. So use of it is declared locally here. */
#define QBMAN_CINH_SWP_CFG   0xd00

/* For MC portal use, we always configure with
 * DQRR_MF is (SWP_CFG,20,3) - DQRR max fill (<- 0x4)
 * EST is (SWP_CFG,16,3) - EQCR_CI stashing threshold (<- 0x0)
 * RPM is (SWP_CFG,12,2) - RCR production notification mode (<- 0x3)
 * DCM is (SWP_CFG,10,2) - DQRR consumption notification mode (<- 0x2)
 * EPM is (SWP_CFG,8,2) - EQCR production notification mode (<- 0x3)
 * SD is (SWP_CFG,5,1) - memory stashing drop enable (<- FALSE)
 * SP is (SWP_CFG,4,1) - memory stashing priority (<- TRUE)
 * SE is (SWP_CFG,3,1) - memory stashing enable (<- 0x0)
 * DP is (SWP_CFG,2,1) - dequeue stashing priority (<- TRUE)
 * DE is (SWP_CFG,1,1) - dequeue stashing enable (<- 0x0)
 * EP is (SWP_CFG,0,1) - EQCR_CI stashing priority (<- FALSE)
 */
static inline uint32_t qbman_set_swp_cfg(uint8_t max_fill, uint8_t wn,
					uint8_t est, uint8_t rpm, uint8_t dcm,
					uint8_t epm, int sd, int sp, int se,
					int dp, int de, int ep)
{
	uint32_t reg;

	reg = e32_uint8_t(20, (uint32_t)(3 + (max_fill >> 3)), max_fill) |
		e32_uint8_t(16, 3, est) | e32_uint8_t(12, 2, rpm) |
		e32_uint8_t(10, 2, dcm) | e32_uint8_t(8, 2, epm) |
		e32_int(5, 1, sd) | e32_int(4, 1, sp) | e32_int(3, 1, se) |
		e32_int(2, 1, dp) | e32_int(1, 1, de) | e32_int(0, 1, ep) |
		e32_uint8_t(14, 1, wn);
	return reg;
}

static inline int qbman_swp_sys_init(struct qbman_swp_sys *s,
				     const struct qbman_swp_desc *d,
				     uint8_t dqrr_size)
{
	uint32_t reg;

	s->addr_cena = d->cena_bar;
	s->addr_cinh = d->cinh_bar;
	s->cena = (void *)valloc(CONFIG_SYS_PAGE_SIZE);
	if (!s->cena) {
		printf("Could not allocate page for cena shadow\n");
		return -1;
	}
	memset((void *)s->cena, 0x00, CONFIG_SYS_PAGE_SIZE);

#ifdef QBMAN_CHECKING
	/* We should never be asked to initialise for a portal that isn't in
	 * the power-on state. (Ie. don't forget to reset portals when they are
	 * decommissioned!)
	 */
	reg = qbman_cinh_read(s, QBMAN_CINH_SWP_CFG);
	BUG_ON(reg);
#endif
#ifdef QBMAN_CINH_ONLY
	reg = qbman_set_swp_cfg(dqrr_size, 1, 0, 3, 2, 3, 0, 1, 0, 1, 0, 0);
#else
	reg = qbman_set_swp_cfg(dqrr_size, 0, 0, 3, 2, 3, 0, 1, 0, 1, 0, 0);
#endif
	qbman_cinh_write(s, QBMAN_CINH_SWP_CFG, reg);
	reg = qbman_cinh_read(s, QBMAN_CINH_SWP_CFG);
	if (!reg) {
		printf("The portal is not enabled!\n");
		free(s->cena);
		return -1;
	}
	return 0;
}

static inline void qbman_swp_sys_finish(struct qbman_swp_sys *s)
{
	free((void *)s->cena);
}
