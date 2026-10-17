// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <misc.h>
#include <net.h>
#include <asm/io.h>

#include "rvu.h"

int qmem_alloc(struct qmem *q, u32 qsize, size_t entry_sz)
{
	q->base = memalign(CONFIG_SYS_CACHELINE_SIZE, qsize * entry_sz);
	if (!q->base)
		return -ENOMEM;
	q->entry_sz = entry_sz;
	q->qsize = qsize;
	q->alloc_sz = (size_t)qsize * entry_sz;
	q->iova = (dma_addr_t)(q->base);
	debug("NIX: qmem alloc for (%d * %d = %ld bytes) at %p\n",
	      q->qsize, q->entry_sz, q->alloc_sz, q->base);
	return 0;
}

void qmem_free(struct qmem *q)
{
	if (q->base)
		free(q->base);
	memset(q, 0, sizeof(*q));
}

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
		 size_t inst_size, size_t res_size)
{
	int err;

	err = qmem_alloc(&aq->inst, qsize, inst_size);
	if (err)
		return err;
	err = qmem_alloc(&aq->res, qsize, res_size);
	if (err)
		qmem_free(&aq->inst);

	return err;
}

/**
 * Frees an admin queue
 *
 * @param	aq	Admin queue to free
 */
void rvu_aq_free(struct admin_queue *aq)
{
	qmem_free(&aq->inst);
	qmem_free(&aq->res);
	memset(aq, 0, sizeof(*aq));
}
