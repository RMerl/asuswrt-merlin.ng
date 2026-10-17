/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef __RVU_H__
#define __RVU_H__

#include <asm/arch/csrs/csrs-rvu.h>

#define ALIGNED		__aligned(CONFIG_SYS_CACHELINE_SIZE)

#define Q_SIZE_16		0ULL /* 16 entries */
#define Q_SIZE_64		1ULL /* 64 entries */
#define Q_SIZE_256		2ULL
#define Q_SIZE_1K		3ULL
#define Q_SIZE_4K		4ULL
#define Q_SIZE_16K		5ULL
#define Q_SIZE_64K		6ULL
#define Q_SIZE_256K		7ULL
#define Q_SIZE_1M		8ULL /* Million entries */
#define Q_SIZE_MIN		Q_SIZE_16
#define Q_SIZE_MAX		Q_SIZE_1M

#define Q_COUNT(x)		(16ULL << (2 * (x)))
#define Q_SIZE(x, n)		((ilog2(x) - (n)) / 2)

/* Admin queue info */

/* Since we intend to add only one instruction at a time,
 * keep queue size to it's minimum.
 */
#define AQ_SIZE			Q_SIZE_16
/* HW head & tail pointer mask */
#define AQ_PTR_MASK		0xFFFFF

struct qmem {
	void		*base;
	dma_addr_t	iova;
	size_t		alloc_sz;
	u32		qsize;
	u8		entry_sz;
};

struct admin_queue {
	struct qmem inst;
	struct qmem res;
};

struct rvu_af {
	struct udevice *dev;
	void __iomem *af_base;
	struct nix_af *nix_af;
};

struct rvu_pf {
	struct udevice *dev;
	struct udevice *afdev;
	void __iomem *pf_base;
	struct nix *nix;
	u8 pfid;
	int nix_lfid;
	int npa_lfid;
};

/**
 * Store 128 bit value
 *
 * @param[out]	dest	pointer to destination address
 * @param	val0	first 64 bits to write
 * @param	val1	second 64 bits to write
 */
static inline void st128(void *dest, u64 val0, u64 val1)
{
	__asm__ __volatile__("stp %x[x0], %x[x1], [%[pm]]" :
		: [x0]"r"(val0), [x1]"r"(val1), [pm]"r"(dest)
		: "memory");
}

/**
 * Load 128 bit value
 *
 * @param[in]	source		pointer to 128 bits of data to load
 * @param[out]	val0		first 64 bits of data
 * @param[out]	val1		second 64 bits of data
 */
static inline void ld128(const u64 *src, u64 *val0, u64 *val1)
{
	__asm__ __volatile__ ("ldp %x[x0], %x[x1], [%[pm]]" :
		 : [x0]"r"(*val0), [x1]"r"(*val1), [pm]"r"(src));
}

void qmem_free(struct qmem *q);
int qmem_alloc(struct qmem *q, u32 qsize, size_t entry_sz);

/**
 * Allocates an admin queue for instructions and results
 *
 * @param	aq	admin queue to allocate for
 * @param	qsize	Number of entries in the queue
 * @param	inst_size	Size of each instruction
 * @param	res_size	Size of each result
 *
 * Return:	-ENOMEM on error, 0 on success
 */
int rvu_aq_alloc(struct admin_queue *aq, unsigned int qsize,
		 size_t inst_size, size_t res_size);

/**
 * Frees an admin queue
 *
 * @param	aq	Admin queue to free
 */
void rvu_aq_free(struct admin_queue *aq);

void rvu_get_lfid_for_pf(int pf, int *nixid, int *npaid);

#endif /* __RVU_H__ */
