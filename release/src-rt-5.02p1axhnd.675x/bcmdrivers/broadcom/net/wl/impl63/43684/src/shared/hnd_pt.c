/** @file hnd_pt.c
 *
 * HND memory partition.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: hnd_pt.c 668357 2016-11-03 03:23:06Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <hnd_pt.h>
#ifdef RTE_CONS
#include <rte_cons.h>
#endif // endif

#ifndef HND_PT_GIANT
#ifdef DMA_TX_FREE
#warning "DMA_TX_FREE defined without HND_PT_GIANT set!"
#endif /* DMA_TX_FREE */
#endif /* HND_PT_GIANT */

static void *mem_pt_get(uint size);
static bool mem_pt_put(void *pblk);
static void mem_pt_append(void);

#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
static void mem_pt_printuse(void *arg, int argc, char *argv[]);
#endif // endif

/*
 * ======HND====== Partition allocation:
 */
void *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
hnd_malloc_ptblk(uint size, const char *file, int line)
#else
hnd_malloc_ptblk(uint size)
#endif // endif
{
	return mem_pt_get(size);
}

int
hnd_free_pt(void *where)
{
	return mem_pt_put(where);
}

void
hnd_append_ptblk(void)
{
	mem_pt_append();
}

/* Each partition has a overhead management structure, which contains the partition specific
 * attributes. But within the partition, each subblock has no overhead.
 *  subblocks are linked together using the first 4 bytes as pointers, which will be
 *    available to use once assigned
 *  partition link list free pointer always point to the available empty subblock
 */

/* XXX BLKSIZE is a tunable in unit of bytes. It must be multiple of 4 Bytes for alignment.
 * The blocks are intended to be used for usb rx dma buffer, which must support
 *     BCMEXTRAHDROOM + MAXPKTBUFSZ(= PKTBUFSZ + LBUFSZ(60))
 * or  LBUFSZ + BCM_RPC_TP_HOST_AGG_MAX_BYTE since usb rx dma buffer skips BCMEXTRAHDROOM
 */
#define MEM_PT_BLKSIZE	(LBUFSZ + 3400)

/* XXX This tunable is in unit of bytes. It depends on desired throughput.
 * For usb rx dma case, the failure is not fatal, but will stall dma receiving.
 * With typical host->dongle->ap uplink tcp traffic experiments(host->dongle agg on)
 *  16(58.5K)->40% request failure, average tput ~110Mbps
 *  18(65.9K)->25% request failure, average tput ~115Mbps
 *  20(73.2K)->10% request failure, average tput ~118Mbps
 * BMAC: Theoretically, high driver can push down 48MPDU(3 AMPDU) or 24*4K buffer
 * avoid call with return failure, one spare one is needed to be robust
 * So 25 is ideal. But it adds stress to other system malloc requests
 * Note, this may exceed certain debug dongle driver image size like 4322-bmac/rom-ag-debug
 */
#ifndef MEM_PT_BLKNUM
#if defined(DMA_TX_FREE)
#ifdef BCMDBG
#define MEM_PT_BLKNUM	5	/* minimal buffers for debug build with less free memory */
#else
#define MEM_PT_BLKNUM	22
#endif /* BCMDBG */
#else
#define MEM_PT_BLKNUM	25
#endif /* DMA_TX_FREE */
#endif /* MEM_PT_BLKNUM */

#define MEM_PT_POOL	(MEM_PT_BLKSIZE * MEM_PT_BLKNUM)
#define MEM_PT_GAINT_THRESHOLD	(MAXPKTBUFSZ + LBUFSZ)	/* only bigger pkt use this method */

typedef struct {	/* memory control block */
	void *mem_pt_addr;	/* Pointer to beginning of memory partition */
	void *mem_pt_freell; /* free blocks link list head */
	hnd_lowmem_free_t *lowmem_free_list; /* list of lowmem free functions */
	uint32 mem_pt_blk_size; /* Size (in bytes) of each block of memory */
	uint32 mem_pt_blk_total; /* Total number of blocks in this partition */
	uint32 mem_pt_blk_free; /* Number of memory blocks remaining in this partition */
	uint32 cnt_req;		/* counter: malloc request */
	uint32 cnt_fail;	/* counter: malloc fail */
} pt_mcb_t;

static char *mem_partition1 = NULL;
static pt_mcb_t g_mem_pt_tbl;
static osl_t *g_osh = NULL;

static uint16 mem_pt_create(char *addr, int size, uint32 blksize);

/** initialize memory partition manager */
void
mem_pt_init(osl_t *osh)
{
	pt_mcb_t *pmem;

	g_osh = osh;

	pmem = &g_mem_pt_tbl;

	pmem->mem_pt_addr = (void *)0;
	pmem->mem_pt_freell = NULL;
	pmem->mem_pt_blk_size = 0;
	pmem->mem_pt_blk_free = 0;
	pmem->mem_pt_blk_total = 0;

	/* create partitions */
	mem_partition1 = MALLOC_ALIGN(g_osh, MEM_PT_POOL, 4);
	ASSERT(mem_partition1 != NULL);
	mem_pt_create(mem_partition1, MEM_PT_POOL, MEM_PT_BLKSIZE);

#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
	if (!hnd_cons_add_cmd("tu", mem_pt_printuse, 0))
		return;
#endif // endif
}

/**
 * create a fixed-sized memory partition
 * Arguments   : base, is the starting address of the memory partition
 *               blksize  is the size (in bytes) of each block in the memory partition
 * Return: nblks successfully allocated out the memory area
 * NOTE: no alignment adjust for base. assume users knows what they want and have done that
*/
static uint16
mem_pt_create(char *base, int size, uint32 blksize)
{
	pt_mcb_t *pmem;
	uint8 *pblk;
	void **plink;
	uint num;

	if (blksize < sizeof(void *)) { /* Must contain space for at least a pointer */
		return 0;
	}

	pmem = &g_mem_pt_tbl;

	/* Create linked list of free memory blocks */
	plink = (void **)base;
	pblk = (uint8 *)base + blksize;
	num = 0;

	while (size >= blksize) {
		num++;
		*plink = (void *)pblk;
		plink  = (void **)pblk;
		pblk += blksize;
		size -= blksize;
	}
	*plink = NULL;	/* Last memory block points to NULL */

	pmem->mem_pt_addr = base;
	pmem->mem_pt_freell = base;
	pmem->mem_pt_blk_free = num;
	pmem->mem_pt_blk_total = num;
	pmem->mem_pt_blk_size = blksize;

	printf("mem_pt_create: addr %p, blksize %d, totblk %d free %d leftsize %d\n",
	       base, pmem->mem_pt_blk_size, pmem->mem_pt_blk_total,
	       pmem->mem_pt_blk_free, size);
	return num;
}

void
hnd_pt_lowmem_register(hnd_lowmem_free_t *lowmem_free_elt)
{
	pt_mcb_t *pmem = &g_mem_pt_tbl;

	lowmem_free_elt->next = pmem->lowmem_free_list;
	pmem->lowmem_free_list = lowmem_free_elt;
}

void
hnd_pt_lowmem_unregister(hnd_lowmem_free_t *lowmem_free_elt)
{
	pt_mcb_t *pmem = &g_mem_pt_tbl;
	hnd_lowmem_free_t **prev_ptr = &pmem->lowmem_free_list;
	hnd_lowmem_free_t *elt = pmem->lowmem_free_list;

	while (elt) {
		if (elt == lowmem_free_elt) {
			*prev_ptr = elt->next;
			return;
		}
		prev_ptr = &elt->next;
		elt = elt->next;
	}
}

static void
mem_pt_lowmem_run(pt_mcb_t *pmem)
{
	hnd_lowmem_free_t *free_elt;

	for (free_elt = pmem->lowmem_free_list;
	     free_elt != NULL && pmem->mem_pt_blk_free == 0;
	     free_elt = free_elt->next)
		(free_elt->free_fn)(free_elt->free_arg);
}

static void *
mem_pt_get(uint size)
{
	pt_mcb_t *pmem = &g_mem_pt_tbl;
	void *pblk;

	if (size > pmem->mem_pt_blk_size) {
		printf("request partition fixbuf size too big %d\n", size);
		ASSERT(0);
		return NULL;
	}

	pmem->cnt_req++;
	if (pmem->mem_pt_blk_free == 0)
		mem_pt_lowmem_run(pmem);

	if (pmem->mem_pt_blk_free > 0) {
		pblk = pmem->mem_pt_freell;

		/* Adjust the freeblk pointer to next block */
		pmem->mem_pt_freell = *(void **)pblk;
		pmem->mem_pt_blk_free--;
		/* printf("mem_pt_get %p, freell %p\n", pblk, pmem->mem_pt_freell); */
		return (pblk);
	} else {
		pmem->cnt_fail++;
		return NULL;
	}
}

static bool
mem_pt_put(void *pblk)
{
	pt_mcb_t *pmem = &g_mem_pt_tbl;

	if (pmem->mem_pt_blk_free >= pmem->mem_pt_blk_total) {
		/* Make sure all blocks not already returned */
		return FALSE;
	}

	/* printf("mem_pt_put %p, freell %p\n", pblk, pmem->mem_pt_freell); */

	*(void **)pblk = pmem->mem_pt_freell;
	pmem->mem_pt_freell = pblk;
	pmem->mem_pt_blk_free++;
	return TRUE;
}

#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
static void
mem_pt_printuse(void *arg, int argc, char *argv[])
{
	printf("Partition: blksize %u totblk %u freeblk %u, malloc_req %u, fail %u (%u%%)\n",
	       g_mem_pt_tbl.mem_pt_blk_size, g_mem_pt_tbl.mem_pt_blk_total,
	       g_mem_pt_tbl.mem_pt_blk_free, g_mem_pt_tbl.cnt_req, g_mem_pt_tbl.cnt_fail,
	       (g_mem_pt_tbl.cnt_req == 0) ? 0 :
	       (100 * g_mem_pt_tbl.cnt_fail) / g_mem_pt_tbl.cnt_req);
}
#endif /* RTE_CONS && !BCM_BOOTLOADER */

static void
mem_pt_append(void)
{
#ifdef MEM_PT_BLKNUM_APPEND
#define MEM_PT_SIZE (MEM_PT_BLKSIZE * MEM_PT_BLKNUM_APPEND)

	char *mem_pt = NULL;
	int i = 0;
	pt_mcb_t *pmem = &g_mem_pt_tbl;

	mem_pt = MALLOC_ALIGN(g_osh, MEM_PT_SIZE, 4);
	if (mem_pt != NULL) {
		pmem->mem_pt_blk_total += MEM_PT_BLKNUM_APPEND;
		for (i = 0; i < MEM_PT_BLKNUM_APPEND; i++)
			mem_pt_put(&mem_pt[i * MEM_PT_BLKSIZE]);
	}
#endif /* MEM_PT_BLKNUM_APPEND */
}
