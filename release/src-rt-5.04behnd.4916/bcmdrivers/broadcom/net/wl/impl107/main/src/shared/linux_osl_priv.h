/*
 * Private header file for Linux OS Independent Layer
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: linux_osl_priv.h 849694 2024-12-20 20:21:10Z $
 */

#ifndef _LINUX_OSL_PRIV_H_
#define _LINUX_OSL_PRIV_H_

#define OS_HANDLE_MAGIC		0x1234abcd	/* Magic # to recognize osh */
#define BCM_MEM_FILENAME_LEN	24		/* Mem. filename length */
#define BCM_MEM_FUNCTION_LEN	48		/* Mem. function name length */

/* dependancy check */
#if !defined(BCMPCIE) && defined(DHD_USE_STATIC_CTRLBUF)
#error "DHD_USE_STATIC_CTRLBUF suppored PCIE target only"
#endif /* !BCMPCIE && DHD_USE_STATIC_CTRLBUF */

#ifdef CONFIG_DHD_USE_STATIC_BUF
#ifdef DHD_USE_STATIC_CTRLBUF
#define DHD_SKB_1PAGE_BUFSIZE	(PAGE_SIZE*1)
#define DHD_SKB_2PAGE_BUFSIZE	(PAGE_SIZE*2)
#define DHD_SKB_4PAGE_BUFSIZE	(PAGE_SIZE*4)

#define PREALLOC_FREE_MAGIC	0xFEDC
#define PREALLOC_USED_MAGIC	0xFCDE
#else
#define DHD_SKB_HDRSIZE		336
#define DHD_SKB_1PAGE_BUFSIZE	((PAGE_SIZE*1)-DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE	((PAGE_SIZE*2)-DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE	((PAGE_SIZE*4)-DHD_SKB_HDRSIZE)
#endif /* DHD_USE_STATIC_CTRLBUF */

#define STATIC_BUF_MAX_NUM	16
#define STATIC_BUF_SIZE	(PAGE_SIZE*2)
#define STATIC_BUF_TOTAL_LEN	(STATIC_BUF_MAX_NUM * STATIC_BUF_SIZE)

typedef struct bcm_static_buf {
	spinlock_t static_lock;
	unsigned char *buf_ptr;
	unsigned char buf_use[STATIC_BUF_MAX_NUM];
} bcm_static_buf_t;

extern bcm_static_buf_t *bcm_static_buf;

#ifdef DHD_USE_STATIC_CTRLBUF
#define STATIC_PKT_4PAGE_NUM	0
#define DHD_SKB_MAX_BUFSIZE	DHD_SKB_2PAGE_BUFSIZE
#elif defined(ENHANCED_STATIC_BUF)
#define STATIC_PKT_4PAGE_NUM	1
#define DHD_SKB_MAX_BUFSIZE	DHD_SKB_4PAGE_BUFSIZE
#else
#define STATIC_PKT_4PAGE_NUM	0
#define DHD_SKB_MAX_BUFSIZE	DHD_SKB_2PAGE_BUFSIZE
#endif /* DHD_USE_STATIC_CTRLBUF */

#ifdef DHD_USE_STATIC_CTRLBUF
#define STATIC_PKT_1PAGE_NUM	0
#define STATIC_PKT_2PAGE_NUM	128
#else
#define STATIC_PKT_1PAGE_NUM	8
#define STATIC_PKT_2PAGE_NUM	8
#endif /* DHD_USE_STATIC_CTRLBUF */

#define STATIC_PKT_1_2PAGE_NUM	\
	((STATIC_PKT_1PAGE_NUM) + (STATIC_PKT_2PAGE_NUM))
#define STATIC_PKT_MAX_NUM	\
	((STATIC_PKT_1_2PAGE_NUM) + (STATIC_PKT_4PAGE_NUM))

typedef struct bcm_static_pkt {
#ifdef DHD_USE_STATIC_CTRLBUF
	struct sk_buff *skb_8k[STATIC_PKT_2PAGE_NUM];
	unsigned char pkt_invalid[STATIC_PKT_2PAGE_NUM];
	spinlock_t osl_pkt_lock;
	uint32 last_allocated_index;
#else
	struct sk_buff *skb_4k[STATIC_PKT_1PAGE_NUM];
	struct sk_buff *skb_8k[STATIC_PKT_2PAGE_NUM];
#ifdef ENHANCED_STATIC_BUF
	struct sk_buff *skb_16k;
#endif /* ENHANCED_STATIC_BUF */
	struct semaphore osl_pkt_sem;
#endif /* DHD_USE_STATIC_CTRLBUF */
	unsigned char pkt_use[STATIC_PKT_MAX_NUM];
} bcm_static_pkt_t;

extern bcm_static_pkt_t *bcm_static_skb;
#endif /* CONFIG_DHD_USE_STATIC_BUF */

typedef struct bcm_mem_link {
	struct bcm_mem_link *prev;
	struct bcm_mem_link *next;
	uint	size;
#ifdef BCMDBG_MEM
	int	line;
	void	*osh;
	char	file[BCM_MEM_FILENAME_LEN];
	char	func[BCM_MEM_FUNCTION_LEN];
#endif
} bcm_mem_link_t;

struct osl_cmn_info {
	atomic_t malloced;
	atomic_t pktalloced;    /* Number of allocated packet buffers */
	atomic_t persistently;	/* Number of allocated packet buffers persistently */
	spinlock_t dbgmem_lock;
#ifdef CONFIG_BCM_TINY_KCORE
	unsigned long dbgmem_flags;
	bcm_mem_link_t *pending_freelist;
	bool use_pending_freelist;
	ulong pending_free_count;
#endif
	bcm_mem_link_t *dbgmem_list;
	bcm_mem_link_t *dbgvmem_list;
#ifdef BCMDBG_DMAMEM
	bcm_mem_link_t *dbgdma_consistent_mem_list;	/**< tracks osl_dma_alloc_consistent() */
	bcm_mem_link_t *dbgdma_contiguous_mem_list;	/**< tracks osl_dma_alloc_contiguous() */
#endif /* BCMDBG_DMAMEM */
	uint dma_alloc_consistent_bytes;
	uint dma_alloc_contiguous_bytes;
	spinlock_t pktalloc_lock;
};
typedef struct osl_cmn_info osl_cmn_t;

#if defined(BCM_BACKPLANE_TIMEOUT)
typedef uint32 (*bpt_cb_fn)(void *ctx, void *addr);
#endif	/* BCM_BACKPLANE_TIMEOUT */

#if defined(BCM_NBUFF)
#include <dhd_nic_common.h>
#endif

#if defined(BCM_DMA_MALLOC)
typedef struct dma_mem_s dma_mem_t;
struct dma_mem_s {
	dll_t node;
	uint32 size;
	uint32 arena;
	void *buf;
	dma_addr_t buf_pa;
};

#define DMA_MEMT_BANK_NUM	8
#define DMA_MEMT_PER_BANK	1024

typedef struct dma_arena_s dma_arena_t;
struct dma_arena_s {
	uint size;
	void *base;
	dma_addr_t base_pa;
};

#define	DMA_MEM_ARENA_NUM	16
#define	DMA_ARENA_BLOCK_SIZE	(2*1024*1024)

#define DMA_MIN_ALIGN_BITS	4
#define DMA_MAX_ALIGN_BITS	20
#define DMA_MIN_ALIGN		(1 << DMA_MIN_ALIGN_BITS)
#endif /* BCM_DMA_MALLOC */

struct osl_info {
#ifdef BCM_NBUFF
	osl_pubinfo_nbuff_t pub;
#else
	osl_pubinfo_t pub;
#endif
	uint32  flags;		/* If specific cases to be handled in the OSL */
	uint magic;
	void *pdev;
	uint failed;
	enum bustype_e bustype;
	osl_cmn_t *cmn; /* Common OSL related data shred between two OSH's */

	uint unit;
	void *bus_handle;
#if defined(DSLCPE_DELAY)
	shared_osl_t *oshsh;		/* osh shared */
#endif
#ifdef BCMDBG_CTRACE
	spinlock_t ctrace_lock;
	struct list_head ctrace_list;
	int ctrace_num;
#endif /* BCMDBG_CTRACE */

#if defined(BCM_BACKPLANE_TIMEOUT)
	bpt_cb_fn bpt_cb;
	void *sih;
#endif	/* BCM_BACKPLANE_TIMEOUT */

	void *timers_context;

#if defined(BCM_DMA_MALLOC)
	dma_mem_t *dma_memt_bank[DMA_MEMT_BANK_NUM];
	int dma_memt_bank_count;

	dll_t dma_memt_pool;
	dll_t dma_memfree;
	dll_t dma_meminuse;

	dma_arena_t dma_arena[DMA_MEM_ARENA_NUM];
	int dma_arena_count;
#endif /* BCM_DMA_ALLOC */
};

#define OSH_PUB(osh)  (*(osl_pubinfo_t *)(osh))

#endif /* _LINUX_OSL_PRIV_H_ */
