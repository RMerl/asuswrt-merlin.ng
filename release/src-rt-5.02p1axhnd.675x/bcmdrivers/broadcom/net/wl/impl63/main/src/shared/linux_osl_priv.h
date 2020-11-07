/*
 * Private header file for Linux OS Independent Layer
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: linux_osl_priv.h 785666 2020-04-02 13:56:26Z $
 */

#ifndef _LINUX_OSL_PRIV_H_
#define _LINUX_OSL_PRIV_H_

#define OS_HANDLE_MAGIC		0x1234abcd	/* Magic # to recognize osh */
#define BCM_MEM_FILENAME_LEN	24		/* Mem. filename length */

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
	int	line;
	void 	*osh;
	char	file[BCM_MEM_FILENAME_LEN];
} bcm_mem_link_t;

struct osl_cmn_info {
	atomic_t malloced;
	atomic_t pktalloced;    /* Number of allocated packet buffers */
	spinlock_t dbgmem_lock;
	bcm_mem_link_t *dbgmem_list;
	bcm_mem_link_t *dbgvmem_list;
	spinlock_t pktalloc_lock;
	atomic_t refcount; /* Number of references to this shared structure. */
};
typedef struct osl_cmn_info osl_cmn_t;

#if defined(BCM_BACKPLANE_TIMEOUT)
typedef uint32 (*bpt_cb_fn)(void *ctx, void *addr);
#endif	/* BCM_BACKPLANE_TIMEOUT */

#if defined(BCM_NBUFF)
#include <dhd_nic_common.h>
#endif // endif

struct osl_info {
#ifdef BCM_NBUFF
	osl_pubinfo_nbuff_t pub;
#else
	osl_pubinfo_t pub;
#endif // endif
	uint32  flags;		/* If specific cases to be handled in the OSL */
#ifdef CTFPOOL
	ctfpool_t *ctfpool;
#endif /* CTFPOOL */
	uint magic;
	void *pdev;
	uint failed;
	uint bustype;
	osl_cmn_t *cmn; /* Common OSL related data shred between two OSH's */

	/* for host drivers, a bus handle is needed when reading from and/or writing to dongle
	 * registeres, however ai/si utilities only passes osh handle to R_REG and W_REG. as
	 * a work around, save the bus handle here
	 */
	void *bus_handle;
#if defined(DSLCPE_DELAY)
	shared_osl_t *oshsh;		/* osh shared */
#endif // endif
#ifdef BCMDBG_CTRACE
	spinlock_t ctrace_lock;
	struct list_head ctrace_list;
	int ctrace_num;
#endif /* BCMDBG_CTRACE */
#ifdef	BCM_SECURE_DMA
#ifdef NOT_YET
	struct sec_mem_elem *sec_list_512;
	struct sec_mem_elem *sec_list_base_512;
	struct sec_mem_elem *sec_list_2048;
	struct sec_mem_elem *sec_list_base_2048;
#endif /* NOT_YET */
	struct sec_mem_elem *sec_list_txbuf; /* All data buffers(4K block) except rx post buffers */
	struct sec_mem_elem *sec_list_base_txbuf;
	struct sec_mem_elem *sec_list_rxbuf; /* rxbuf pool 4K each block */
	struct sec_mem_elem *sec_list_base_rxbuf;
	phys_addr_t  contig_base;
	void *contig_base_va;
	phys_addr_t  contig_base_alloc;
	void *contig_base_alloc_va;
	phys_addr_t contig_base_coherent_pa;
	phys_addr_t contig_base_sts_phyrx_pa;
	void *contig_base_coherent_va;
	void *contig_base_txbuf_va;
#ifdef BCMDONGLEHOST
	struct sec_mem_elem *sec_list_rxbufctl; /* rxctl buffer pool 8K each block */
	struct sec_mem_elem *sec_list_base_rxbufctl;
	void *contig_base_rxbufctl_va;
#endif /* BCMDONGLEHOST */
	void *contig_base_rxbuf_va;
	void *contig_base_sts_phyrx_va;
	void *contig_delta_va_pa;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 89)
	struct page *contig_base_cma_page;
#endif /* KERNEL_VERSION(4, 9, 89) */
	uint8* secdma_coherant_pfree;
	int stb_ext_params;
#endif /* BCM_SECURE_DMA */
#if defined(BCM_BACKPLANE_TIMEOUT)
	bpt_cb_fn bpt_cb;
	void *sih;
#endif	/* BCM_BACKPLANE_TIMEOUT */
};

#define OSH_PUB(osh)  (*(osl_pubinfo_t *)(osh))

#endif /* _LINUX_OSL_PRIV_H_ */
