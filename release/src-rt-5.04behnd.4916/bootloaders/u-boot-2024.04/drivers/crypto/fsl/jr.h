/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 *
 */

#ifndef __JR_H
#define __JR_H

#include <linux/compiler.h>
#include "fsl_sec.h"
#include "type.h"
#include <misc.h>

#define JR_SIZE 4
/* Timeout currently defined as 10 sec */
#define CFG_USEC_DEQ_TIMEOUT	10000000U

#define DEFAULT_JR_ID		0
#define DEFAULT_JR_LIODN	0
#define DEFAULT_IRQ		0	/* Interrupts not to be configured */

#define MCFGR_SWRST       ((uint32_t)(1)<<31) /* Software Reset */
#define MCFGR_DMA_RST     ((uint32_t)(1)<<28) /* DMA Reset */
#define MCFGR_PS_SHIFT          16
#define MCFGR_AWCACHE_SHIFT	8
#define MCFGR_AWCACHE_MASK	(0xf << MCFGR_AWCACHE_SHIFT)
#define MCFGR_ARCACHE_SHIFT	12
#define MCFGR_ARCACHE_MASK	(0xf << MCFGR_ARCACHE_SHIFT)

#define JR_INTMASK	  0x00000001
#define JRCR_RESET                  0x01
#define JRINT_ERR_HALT_INPROGRESS   0x4
#define JRINT_ERR_HALT_MASK         0xc
#define JRNSLIODN_SHIFT		16
#define JRNSLIODN_MASK		0x0fff0000
#define JRSLIODN_SHIFT		0
#define JRSLIODN_MASK		0x00000fff

#define JRDID_MS_PRIM_DID	BIT(0)
#define JRDID_MS_PRIM_TZ	BIT(4)
#define JRDID_MS_TZ_OWN		BIT(15)

#define JQ_DEQ_ERR		(-1)
#define JQ_DEQ_TO_ERR		(-2)
#define JQ_ENQ_ERR		(-3)

#define RNG4_MAX_HANDLES	2

enum {
	/* Run caam jobring descriptor(in buf) */
	CAAM_JR_RUN_DESC,
};

struct op_ring {
	caam_dma_addr_t desc;
	uint32_t status;
} __packed;

struct jr_info {
	void (*callback)(uint32_t status, void *arg);
	caam_dma_addr_t desc_phys_addr;
	uint32_t desc_len;
	uint32_t op_done;
	void *arg;
};

struct jobring {
	int jq_id;
	int irq;
	int liodn;
	/* Head is the index where software would enq the descriptor in
	 * the i/p ring
	 */
	int head;
	/* Tail index would be used by s/w ehile enqueuing to determine if
	 * there is any space left in the s/w maintained i/p rings
	 */
	/* Also in case of deq tail will be incremented only in case of
	 * in-order job completion
	 */
	int tail;
	/* Read index of the output ring. It may not match with tail in case
	 * of out of order completetion
	 */
	int read_idx;
	/* Write index to input ring. Would be always equal to head */
	int write_idx;
	/* Size of the rings. */
	int size;
	/* Op ring size aligned to cache line size */
	int op_size;
	/* The ip and output rings have to be accessed by SEC. So the
	 * pointers will ahve to point to the housekeeping region provided
	 * by SEC
	 */
	/*Circular  Ring of i/p descriptors */
	caam_dma_addr_t *input_ring;
	/* Circular Ring of o/p descriptors */
	/* Circula Ring containing info regarding descriptors in i/p
	 * and o/p ring
	 */
	/* This ring can be on the stack */
	struct jr_info info[JR_SIZE];
	struct op_ring *output_ring;
	/* Offset in CCSR to the SEC engine to which this JR belongs */
	uint32_t sec_offset;

};

struct result {
	int done;
	uint32_t status;
};

/*
 * struct caam_regs - CAAM initialization register interface
 *
 * Interface to caam memory map, jobring register, jobring storage.
 */
struct caam_regs {
	ccsr_sec_t *sec;	/*caam initialization registers*/
	struct jr_regs *regs;	/*jobring configuration registers*/
	u8 jrid;		/*id to identify a jobring*/
	/*Private sub-storage for a single JobR*/
	struct jobring jr[CONFIG_SYS_FSL_MAX_NUM_OF_SEC];
};

void caam_jr_strstatus(u32 status);
int run_descriptor_jr(uint32_t *desc);

#endif
