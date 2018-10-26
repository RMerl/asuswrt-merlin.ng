/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#ifndef PDC_H
#define PDC_H

#include <linux/scatterlist.h>

#define PDC_SUCCESS  0

#define reg_read32(a)  __raw_readl((void __iomem *)(a))
#define reg_write32(v,a) __raw_writel((v), (void __iomem *)(a))

/* Length of DMA header prepended to data */
#define CRYPTO_RX_DMA_OFFSET    4

#define BCM_HDR_LEN  8

#define PDC_RESP_HDR_LEN (CRYPTO_RX_DMA_OFFSET + BCM_HDR_LEN)

#define RING_ENTRY_SIZE   sizeof(struct dma64dd)

#define PDC_RX_BUDGET     32
#define PDC_RING_ENTRIES  512 /* this must be a power of 2, maximum of 512*/
#define PDC_RING_SIZE    (PDC_RING_ENTRIES * RING_ENTRY_SIZE)
#define PDC_RING_ALIGN   (8 * 1024)

/* descriptor bumping macros */
#define INCRD(i, c) (((i) + (c)) & ((PDC_RING_ENTRIES) - 1))
#define NEXTD(i) (((i) + 1) & ((PDC_RING_ENTRIES) - 1))
#define PREVD(i) (((i) - 1) & ((PDC_RING_ENTRIES) - 1))
#define DESCRCOUNT(head, tail) (((head) - (tail)) & ((PDC_RING_ENTRIES) - 1))
#define DESCRSPACE(head, tail) DESCRCOUNT((tail), ((head) + 1))

/*
 * dma descriptor
 *   - descriptors are only read by the hardware, never written back
 */
struct dma64dd {
	u32 ctrl1;      /* misc control bits */
	u32 ctrl2;      /* buffer count and address extension */
	u32 addrlow;    /* memory address of the date buffer, bits 31:0 */
	u32 addrhigh;   /* memory address of the date buffer, bits 63:32 */
};

/* dma registers per channel(xmt or rcv) */
struct dma64_regs {
	u32  control;   /* enable, et al */
	u32  ptr;       /* last descriptor posted to chip */
	u32  addrlow;   /* descriptor ring base address low 32-bits */
	u32  addrhigh;  /* descriptor ring base address bits 63:32 */
	u32  status0;   /* current descriptor, xmt state */
	u32  status1;   /* active descriptor, xmt error */
};

/* dma registers */
struct dma64 {
	volatile struct dma64_regs dmaxmt;  /* dma tx */
	u32          pad0[2];
	volatile struct dma64_regs dmarcv;  /* dma rx */
	u32          pad1[2];
};

/* structure for allocating/freeing DMA rings */
struct pdc_ring_alloc {
	dma_addr_t	pbase;   /* base physical address */
	void		*vbase;  /* base kernel virtual address */
	u32		size;    /* size in bytes */
};

struct desc_cnt {
	u16 rx;
	u16 tx;
};

/* spu dma state structure */
struct pdc_state {

	/* synchronize access to this PDC state structure */
	spinlock_t pdc_lock;

	/* DMA channel ID */
	u32 channel;

	/* Platform device for this PDC instance */
	struct platform_device *pdev;

	/* The base virtual address of SPU DMA tx/rx descriptor rings.
	 * Corresponding DMA address and size of ring allocation.
	 */
	struct dma_pool *dma_pool;
	struct pdc_ring_alloc tx_ring_alloc;
	struct pdc_ring_alloc rx_ring_alloc;

	volatile struct dma64_regs *txregs_64; /* dma tx engine registers */
	volatile struct dma64_regs *rxregs_64; /* dma rx engine registers */

	/* Arrays of PDC_RING_ENTRIES descriptors */
	volatile struct dma64dd   *txd_64;  /* tx descriptor ring */
	volatile struct dma64dd   *rxd_64;  /* rx descriptor ring */

	u8           *resp_header;    /* pointer to memory to store BCM and response headers */
	dma_addr_t    resp_header_dma;

	u8          bcm_header[BCM_HDR_LEN]; /* array containing TX BCM header */
	dma_addr_t  bcm_header_dma;

	u32  txin; /* Index of next tx descriptor to reclaim. */
	u32  txout; /* Index of next tx descriptor to post. */

	u32  rxin; /* Index of next rx descriptor to reclaim. */
	u32  rxout; /* Index of next rx descriptor to post. */

	u32  last_rx_curr; /* Saved value of current hardware rx descriptor index. */
	void *rxp_ctx[PDC_RING_ENTRIES]; /* opaque context associated with frame */
	struct desc_cnt numd[PDC_RING_ENTRIES]; /* Number of rx and tx descriptors associated with the message */

	struct list_head msg_list;
	u32 msg_count;

	/* counters */
	u32  requests;        /* number of SPU requests submitted */
	u32  replies;         /* number of SPU replies received */
	u32  txnobuf;         /* count of tx ring full */
	u32  rxnobuf;         /* count of rx ring full */
	u32  rx_oflow;        /* count of rx overflows */
};

struct pdc_global {
	u8 num_chan;        /* Number of channels the PDC supports */
	atomic_t next_chan; /* channel to use for the next set of DMA requests */
	u32 int_mask;
	struct pdc_state *pdc_state;
	unsigned int pdc_irq;
	void __iomem *pdc_reg_vbase;

	struct dma_pool *dma_pool;     /* dma pool for descriptor allocation */
	struct dentry *debugfs_stats;  /* debug FS stats file for this SPU */
	struct dentry *debugfs_dir;
	struct file_operations debugfs_fo;

	struct task_struct *work_thread;
	wait_queue_head_t wait_queue;
	int work_avail;
};

struct brcm_message {
	struct scatterlist *src;
	struct scatterlist *dst;
	int error;
	void (*rx_callback)(struct brcm_message *msg);
	struct list_head list;
};

#endif /* PDC_H */
