/*
 * linux/arch/arm/plat-omap/dma.c
 *
 * Copyright (C) 2003 - 2008 Nokia Corporation
 * Author: Juha Yrjölä <juha.yrjola@nokia.com>
 * DMA channel linking for 1610 by Samuel Ortiz <samuel.ortiz@nokia.com>
 * Graphics DMA and LCD DMA graphics tranformations
 * by Imre Deak <imre.deak@nokia.com>
 * OMAP2/3 support Copyright (C) 2004-2007 Texas Instruments, Inc.
 * Merged to support both OMAP1 and OMAP2 by Tony Lindgren <tony@atomide.com>
 * Some functions based on earlier dma-omap.c Copyright (C) 2001 RidgeRun, Inc.
 *
 * Copyright (C) 2009 Texas Instruments
 * Added OMAP4 support - Santosh Shilimkar <santosh.shilimkar@ti.com>
 *
 * Support functions for the OMAP internal DMA channels.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * Converted DMA library into DMA platform driver.
 *	- G, Manjunath Kondaiah <manjugk@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <linux/omap-dma.h>

/*
 * MAX_LOGICAL_DMA_CH_COUNT: the maximum number of logical DMA
 * channels that an instance of the SDMA IP block can support.  Used
 * to size arrays.  (The actual maximum on a particular SoC may be less
 * than this -- for example, OMAP1 SDMA instances only support 17 logical
 * DMA channels.)
 */
#define MAX_LOGICAL_DMA_CH_COUNT		32

#undef DEBUG

#ifndef CONFIG_ARCH_OMAP1
enum { DMA_CH_ALLOC_DONE, DMA_CH_PARAMS_SET_DONE, DMA_CH_STARTED,
	DMA_CH_QUEUED, DMA_CH_NOTSTARTED, DMA_CH_PAUSED, DMA_CH_LINK_ENABLED
};

enum { DMA_CHAIN_STARTED, DMA_CHAIN_NOTSTARTED };
#endif

#define OMAP_DMA_ACTIVE			0x01
#define OMAP2_DMA_CSR_CLEAR_MASK	0xffffffff

#define OMAP_FUNC_MUX_ARM_BASE		(0xfffe1000 + 0xec)

static struct omap_system_dma_plat_info *p;
static struct omap_dma_dev_attr *d;
static void omap_clear_dma(int lch);
static int omap_dma_set_prio_lch(int lch, unsigned char read_prio,
				 unsigned char write_prio);
static int enable_1510_mode;
static u32 errata;

static struct omap_dma_global_context_registers {
	u32 dma_irqenable_l0;
	u32 dma_irqenable_l1;
	u32 dma_ocp_sysconfig;
	u32 dma_gcr;
} omap_dma_global_context;

struct dma_link_info {
	int *linked_dmach_q;
	int no_of_lchs_linked;

	int q_count;
	int q_tail;
	int q_head;

	int chain_state;
	int chain_mode;

};

static struct dma_link_info *dma_linked_lch;

#ifndef CONFIG_ARCH_OMAP1

/* Chain handling macros */
#define OMAP_DMA_CHAIN_QINIT(chain_id)					\
	do {								\
		dma_linked_lch[chain_id].q_head =			\
		dma_linked_lch[chain_id].q_tail =			\
		dma_linked_lch[chain_id].q_count = 0;			\
	} while (0)
#define OMAP_DMA_CHAIN_QFULL(chain_id)					\
		(dma_linked_lch[chain_id].no_of_lchs_linked ==		\
		dma_linked_lch[chain_id].q_count)
#define OMAP_DMA_CHAIN_QLAST(chain_id)					\
	do {								\
		((dma_linked_lch[chain_id].no_of_lchs_linked-1) ==	\
		dma_linked_lch[chain_id].q_count)			\
	} while (0)
#define OMAP_DMA_CHAIN_QEMPTY(chain_id)					\
		(0 == dma_linked_lch[chain_id].q_count)
#define __OMAP_DMA_CHAIN_INCQ(end)					\
	((end) = ((end)+1) % dma_linked_lch[chain_id].no_of_lchs_linked)
#define OMAP_DMA_CHAIN_INCQHEAD(chain_id)				\
	do {								\
		__OMAP_DMA_CHAIN_INCQ(dma_linked_lch[chain_id].q_head);	\
		dma_linked_lch[chain_id].q_count--;			\
	} while (0)

#define OMAP_DMA_CHAIN_INCQTAIL(chain_id)				\
	do {								\
		__OMAP_DMA_CHAIN_INCQ(dma_linked_lch[chain_id].q_tail);	\
		dma_linked_lch[chain_id].q_count++; \
	} while (0)
#endif

static int dma_lch_count;
static int dma_chan_count;
static int omap_dma_reserve_channels;

static spinlock_t dma_chan_lock;
static struct omap_dma_lch *dma_chan;

static inline void disable_lnk(int lch);
static void omap_disable_channel_irq(int lch);
static inline void omap_enable_channel_irq(int lch);

#define REVISIT_24XX()		printk(KERN_ERR "FIXME: no %s on 24xx\n", \
						__func__);

#ifdef CONFIG_ARCH_OMAP15XX
/* Returns 1 if the DMA module is in OMAP1510-compatible mode, 0 otherwise */
static int omap_dma_in_1510_mode(void)
{
	return enable_1510_mode;
}
#else
#define omap_dma_in_1510_mode()		0
#endif

#ifdef CONFIG_ARCH_OMAP1
static inline void set_gdma_dev(int req, int dev)
{
	u32 reg = OMAP_FUNC_MUX_ARM_BASE + ((req - 1) / 5) * 4;
	int shift = ((req - 1) % 5) * 6;
	u32 l;

	l = omap_readl(reg);
	l &= ~(0x3f << shift);
	l |= (dev - 1) << shift;
	omap_writel(l, reg);
}
#else
#define set_gdma_dev(req, dev)	do {} while (0)
#define omap_readl(reg)		0
#define omap_writel(val, reg)	do {} while (0)
#endif

#ifdef CONFIG_ARCH_OMAP1
void omap_set_dma_priority(int lch, int dst_port, int priority)
{
	unsigned long reg;
	u32 l;

	if (dma_omap1()) {
		switch (dst_port) {
		case OMAP_DMA_PORT_OCP_T1:	/* FFFECC00 */
			reg = OMAP_TC_OCPT1_PRIOR;
			break;
		case OMAP_DMA_PORT_OCP_T2:	/* FFFECCD0 */
			reg = OMAP_TC_OCPT2_PRIOR;
			break;
		case OMAP_DMA_PORT_EMIFF:	/* FFFECC08 */
			reg = OMAP_TC_EMIFF_PRIOR;
			break;
		case OMAP_DMA_PORT_EMIFS:	/* FFFECC04 */
			reg = OMAP_TC_EMIFS_PRIOR;
			break;
		default:
			BUG();
			return;
		}
		l = omap_readl(reg);
		l &= ~(0xf << 8);
		l |= (priority & 0xf) << 8;
		omap_writel(l, reg);
	}
}
#endif

#ifdef CONFIG_ARCH_OMAP2PLUS
void omap_set_dma_priority(int lch, int dst_port, int priority)
{
	u32 ccr;

	ccr = p->dma_read(CCR, lch);
	if (priority)
		ccr |= (1 << 6);
	else
		ccr &= ~(1 << 6);
	p->dma_write(ccr, CCR, lch);
}
#endif
EXPORT_SYMBOL(omap_set_dma_priority);

void omap_set_dma_transfer_params(int lch, int data_type, int elem_count,
				  int frame_count, int sync_mode,
				  int dma_trigger, int src_or_dst_synch)
{
	u32 l;

	l = p->dma_read(CSDP, lch);
	l &= ~0x03;
	l |= data_type;
	p->dma_write(l, CSDP, lch);

	if (dma_omap1()) {
		u16 ccr;

		ccr = p->dma_read(CCR, lch);
		ccr &= ~(1 << 5);
		if (sync_mode == OMAP_DMA_SYNC_FRAME)
			ccr |= 1 << 5;
		p->dma_write(ccr, CCR, lch);

		ccr = p->dma_read(CCR2, lch);
		ccr &= ~(1 << 2);
		if (sync_mode == OMAP_DMA_SYNC_BLOCK)
			ccr |= 1 << 2;
		p->dma_write(ccr, CCR2, lch);
	}

	if (dma_omap2plus() && dma_trigger) {
		u32 val;

		val = p->dma_read(CCR, lch);

		/* DMA_SYNCHRO_CONTROL_UPPER depends on the channel number */
		val &= ~((1 << 23) | (3 << 19) | 0x1f);
		val |= (dma_trigger & ~0x1f) << 14;
		val |= dma_trigger & 0x1f;

		if (sync_mode & OMAP_DMA_SYNC_FRAME)
			val |= 1 << 5;
		else
			val &= ~(1 << 5);

		if (sync_mode & OMAP_DMA_SYNC_BLOCK)
			val |= 1 << 18;
		else
			val &= ~(1 << 18);

		if (src_or_dst_synch == OMAP_DMA_DST_SYNC_PREFETCH) {
			val &= ~(1 << 24);	/* dest synch */
			val |= (1 << 23);	/* Prefetch */
		} else if (src_or_dst_synch) {
			val |= 1 << 24;		/* source synch */
		} else {
			val &= ~(1 << 24);	/* dest synch */
		}
		p->dma_write(val, CCR, lch);
	}

	p->dma_write(elem_count, CEN, lch);
	p->dma_write(frame_count, CFN, lch);
}
EXPORT_SYMBOL(omap_set_dma_transfer_params);

void omap_set_dma_write_mode(int lch, enum omap_dma_write_mode mode)
{
	if (dma_omap2plus()) {
		u32 csdp;

		csdp = p->dma_read(CSDP, lch);
		csdp &= ~(0x3 << 16);
		csdp |= (mode << 16);
		p->dma_write(csdp, CSDP, lch);
	}
}
EXPORT_SYMBOL(omap_set_dma_write_mode);

void omap_set_dma_channel_mode(int lch, enum omap_dma_channel_mode mode)
{
	if (dma_omap1() && !dma_omap15xx()) {
		u32 l;

		l = p->dma_read(LCH_CTRL, lch);
		l &= ~0x7;
		l |= mode;
		p->dma_write(l, LCH_CTRL, lch);
	}
}
EXPORT_SYMBOL(omap_set_dma_channel_mode);

/* Note that src_port is only for omap1 */
void omap_set_dma_src_params(int lch, int src_port, int src_amode,
			     unsigned long src_start,
			     int src_ei, int src_fi)
{
	u32 l;

	if (dma_omap1()) {
		u16 w;

		w = p->dma_read(CSDP, lch);
		w &= ~(0x1f << 2);
		w |= src_port << 2;
		p->dma_write(w, CSDP, lch);
	}

	l = p->dma_read(CCR, lch);
	l &= ~(0x03 << 12);
	l |= src_amode << 12;
	p->dma_write(l, CCR, lch);

	p->dma_write(src_start, CSSA, lch);

	p->dma_write(src_ei, CSEI, lch);
	p->dma_write(src_fi, CSFI, lch);
}
EXPORT_SYMBOL(omap_set_dma_src_params);

void omap_set_dma_params(int lch, struct omap_dma_channel_params *params)
{
	omap_set_dma_transfer_params(lch, params->data_type,
				     params->elem_count, params->frame_count,
				     params->sync_mode, params->trigger,
				     params->src_or_dst_synch);
	omap_set_dma_src_params(lch, params->src_port,
				params->src_amode, params->src_start,
				params->src_ei, params->src_fi);

	omap_set_dma_dest_params(lch, params->dst_port,
				 params->dst_amode, params->dst_start,
				 params->dst_ei, params->dst_fi);
	if (params->read_prio || params->write_prio)
		omap_dma_set_prio_lch(lch, params->read_prio,
				      params->write_prio);
}
EXPORT_SYMBOL(omap_set_dma_params);

void omap_set_dma_src_data_pack(int lch, int enable)
{
	u32 l;

	l = p->dma_read(CSDP, lch);
	l &= ~(1 << 6);
	if (enable)
		l |= (1 << 6);
	p->dma_write(l, CSDP, lch);
}
EXPORT_SYMBOL(omap_set_dma_src_data_pack);

void omap_set_dma_src_burst_mode(int lch, enum omap_dma_burst_mode burst_mode)
{
	unsigned int burst = 0;
	u32 l;

	l = p->dma_read(CSDP, lch);
	l &= ~(0x03 << 7);

	switch (burst_mode) {
	case OMAP_DMA_DATA_BURST_DIS:
		break;
	case OMAP_DMA_DATA_BURST_4:
		if (dma_omap2plus())
			burst = 0x1;
		else
			burst = 0x2;
		break;
	case OMAP_DMA_DATA_BURST_8:
		if (dma_omap2plus()) {
			burst = 0x2;
			break;
		}
		/*
		 * not supported by current hardware on OMAP1
		 * w |= (0x03 << 7);
		 * fall through
		 */
	case OMAP_DMA_DATA_BURST_16:
		if (dma_omap2plus()) {
			burst = 0x3;
			break;
		}
		/*
		 * OMAP1 don't support burst 16
		 * fall through
		 */
	default:
		BUG();
	}

	l |= (burst << 7);
	p->dma_write(l, CSDP, lch);
}
EXPORT_SYMBOL(omap_set_dma_src_burst_mode);

/* Note that dest_port is only for OMAP1 */
void omap_set_dma_dest_params(int lch, int dest_port, int dest_amode,
			      unsigned long dest_start,
			      int dst_ei, int dst_fi)
{
	u32 l;

	if (dma_omap1()) {
		l = p->dma_read(CSDP, lch);
		l &= ~(0x1f << 9);
		l |= dest_port << 9;
		p->dma_write(l, CSDP, lch);
	}

	l = p->dma_read(CCR, lch);
	l &= ~(0x03 << 14);
	l |= dest_amode << 14;
	p->dma_write(l, CCR, lch);

	p->dma_write(dest_start, CDSA, lch);

	p->dma_write(dst_ei, CDEI, lch);
	p->dma_write(dst_fi, CDFI, lch);
}
EXPORT_SYMBOL(omap_set_dma_dest_params);

void omap_set_dma_dest_data_pack(int lch, int enable)
{
	u32 l;

	l = p->dma_read(CSDP, lch);
	l &= ~(1 << 13);
	if (enable)
		l |= 1 << 13;
	p->dma_write(l, CSDP, lch);
}
EXPORT_SYMBOL(omap_set_dma_dest_data_pack);

void omap_set_dma_dest_burst_mode(int lch, enum omap_dma_burst_mode burst_mode)
{
	unsigned int burst = 0;
	u32 l;

	l = p->dma_read(CSDP, lch);
	l &= ~(0x03 << 14);

	switch (burst_mode) {
	case OMAP_DMA_DATA_BURST_DIS:
		break;
	case OMAP_DMA_DATA_BURST_4:
		if (dma_omap2plus())
			burst = 0x1;
		else
			burst = 0x2;
		break;
	case OMAP_DMA_DATA_BURST_8:
		if (dma_omap2plus())
			burst = 0x2;
		else
			burst = 0x3;
		break;
	case OMAP_DMA_DATA_BURST_16:
		if (dma_omap2plus()) {
			burst = 0x3;
			break;
		}
		/*
		 * OMAP1 don't support burst 16
		 * fall through
		 */
	default:
		printk(KERN_ERR "Invalid DMA burst mode\n");
		BUG();
		return;
	}
	l |= (burst << 14);
	p->dma_write(l, CSDP, lch);
}
EXPORT_SYMBOL(omap_set_dma_dest_burst_mode);

static inline void omap_enable_channel_irq(int lch)
{
	/* Clear CSR */
	if (dma_omap1())
		p->dma_read(CSR, lch);
	else
		p->dma_write(OMAP2_DMA_CSR_CLEAR_MASK, CSR, lch);

	/* Enable some nice interrupts. */
	p->dma_write(dma_chan[lch].enabled_irqs, CICR, lch);
}

static inline void omap_disable_channel_irq(int lch)
{
	/* disable channel interrupts */
	p->dma_write(0, CICR, lch);
	/* Clear CSR */
	if (dma_omap1())
		p->dma_read(CSR, lch);
	else
		p->dma_write(OMAP2_DMA_CSR_CLEAR_MASK, CSR, lch);
}

void omap_enable_dma_irq(int lch, u16 bits)
{
	dma_chan[lch].enabled_irqs |= bits;
}
EXPORT_SYMBOL(omap_enable_dma_irq);

void omap_disable_dma_irq(int lch, u16 bits)
{
	dma_chan[lch].enabled_irqs &= ~bits;
}
EXPORT_SYMBOL(omap_disable_dma_irq);

static inline void enable_lnk(int lch)
{
	u32 l;

	l = p->dma_read(CLNK_CTRL, lch);

	if (dma_omap1())
		l &= ~(1 << 14);

	/* Set the ENABLE_LNK bits */
	if (dma_chan[lch].next_lch != -1)
		l = dma_chan[lch].next_lch | (1 << 15);

#ifndef CONFIG_ARCH_OMAP1
	if (dma_omap2plus())
		if (dma_chan[lch].next_linked_ch != -1)
			l = dma_chan[lch].next_linked_ch | (1 << 15);
#endif

	p->dma_write(l, CLNK_CTRL, lch);
}

static inline void disable_lnk(int lch)
{
	u32 l;

	l = p->dma_read(CLNK_CTRL, lch);

	/* Disable interrupts */
	omap_disable_channel_irq(lch);

	if (dma_omap1()) {
		/* Set the STOP_LNK bit */
		l |= 1 << 14;
	}

	if (dma_omap2plus()) {
		/* Clear the ENABLE_LNK bit */
		l &= ~(1 << 15);
	}

	p->dma_write(l, CLNK_CTRL, lch);
	dma_chan[lch].flags &= ~OMAP_DMA_ACTIVE;
}

static inline void omap2_enable_irq_lch(int lch)
{
	u32 val;
	unsigned long flags;

	if (dma_omap1())
		return;

	spin_lock_irqsave(&dma_chan_lock, flags);
	/* clear IRQ STATUS */
	p->dma_write(1 << lch, IRQSTATUS_L0, lch);
	/* Enable interrupt */
	val = p->dma_read(IRQENABLE_L0, lch);
	val |= 1 << lch;
	p->dma_write(val, IRQENABLE_L0, lch);
	spin_unlock_irqrestore(&dma_chan_lock, flags);
}

static inline void omap2_disable_irq_lch(int lch)
{
	u32 val;
	unsigned long flags;

	if (dma_omap1())
		return;

	spin_lock_irqsave(&dma_chan_lock, flags);
	/* Disable interrupt */
	val = p->dma_read(IRQENABLE_L0, lch);
	val &= ~(1 << lch);
	p->dma_write(val, IRQENABLE_L0, lch);
	/* clear IRQ STATUS */
	p->dma_write(1 << lch, IRQSTATUS_L0, lch);
	spin_unlock_irqrestore(&dma_chan_lock, flags);
}

int omap_request_dma(int dev_id, const char *dev_name,
		     void (*callback)(int lch, u16 ch_status, void *data),
		     void *data, int *dma_ch_out)
{
	int ch, free_ch = -1;
	unsigned long flags;
	struct omap_dma_lch *chan;

	WARN(strcmp(dev_name, "DMA engine"), "Using deprecated platform DMA API - please update to DMA engine");

	spin_lock_irqsave(&dma_chan_lock, flags);
	for (ch = 0; ch < dma_chan_count; ch++) {
		if (free_ch == -1 && dma_chan[ch].dev_id == -1) {
			free_ch = ch;
			/* Exit after first free channel found */
			break;
		}
	}
	if (free_ch == -1) {
		spin_unlock_irqrestore(&dma_chan_lock, flags);
		return -EBUSY;
	}
	chan = dma_chan + free_ch;
	chan->dev_id = dev_id;

	if (p->clear_lch_regs)
		p->clear_lch_regs(free_ch);

	if (dma_omap2plus())
		omap_clear_dma(free_ch);

	spin_unlock_irqrestore(&dma_chan_lock, flags);

	chan->dev_name = dev_name;
	chan->callback = callback;
	chan->data = data;
	chan->flags = 0;

#ifndef CONFIG_ARCH_OMAP1
	if (dma_omap2plus()) {
		chan->chain_id = -1;
		chan->next_linked_ch = -1;
	}
#endif

	chan->enabled_irqs = OMAP_DMA_DROP_IRQ | OMAP_DMA_BLOCK_IRQ;

	if (dma_omap1())
		chan->enabled_irqs |= OMAP1_DMA_TOUT_IRQ;
	else if (dma_omap2plus())
		chan->enabled_irqs |= OMAP2_DMA_MISALIGNED_ERR_IRQ |
			OMAP2_DMA_TRANS_ERR_IRQ;

	if (dma_omap16xx()) {
		/* If the sync device is set, configure it dynamically. */
		if (dev_id != 0) {
			set_gdma_dev(free_ch + 1, dev_id);
			dev_id = free_ch + 1;
		}
		/*
		 * Disable the 1510 compatibility mode and set the sync device
		 * id.
		 */
		p->dma_write(dev_id | (1 << 10), CCR, free_ch);
	} else if (dma_omap1()) {
		p->dma_write(dev_id, CCR, free_ch);
	}

	if (dma_omap2plus()) {
		omap_enable_channel_irq(free_ch);
		omap2_enable_irq_lch(free_ch);
	}

	*dma_ch_out = free_ch;

	return 0;
}
EXPORT_SYMBOL(omap_request_dma);

void omap_free_dma(int lch)
{
	unsigned long flags;

	if (dma_chan[lch].dev_id == -1) {
		pr_err("omap_dma: trying to free unallocated DMA channel %d\n",
		       lch);
		return;
	}

	/* Disable interrupt for logical channel */
	if (dma_omap2plus())
		omap2_disable_irq_lch(lch);

	/* Disable all DMA interrupts for the channel. */
	omap_disable_channel_irq(lch);

	/* Make sure the DMA transfer is stopped. */
	p->dma_write(0, CCR, lch);

	/* Clear registers */
	if (dma_omap2plus())
		omap_clear_dma(lch);

	spin_lock_irqsave(&dma_chan_lock, flags);
	dma_chan[lch].dev_id = -1;
	dma_chan[lch].next_lch = -1;
	dma_chan[lch].callback = NULL;
	spin_unlock_irqrestore(&dma_chan_lock, flags);
}
EXPORT_SYMBOL(omap_free_dma);

/**
 * @brief omap_dma_set_global_params : Set global priority settings for dma
 *
 * @param arb_rate
 * @param max_fifo_depth
 * @param tparams - Number of threads to reserve : DMA_THREAD_RESERVE_NORM
 * 						   DMA_THREAD_RESERVE_ONET
 * 						   DMA_THREAD_RESERVE_TWOT
 * 						   DMA_THREAD_RESERVE_THREET
 */
void
omap_dma_set_global_params(int arb_rate, int max_fifo_depth, int tparams)
{
	u32 reg;

	if (dma_omap1()) {
		printk(KERN_ERR "FIXME: no %s on 15xx/16xx\n", __func__);
		return;
	}

	if (max_fifo_depth == 0)
		max_fifo_depth = 1;
	if (arb_rate == 0)
		arb_rate = 1;

	reg = 0xff & max_fifo_depth;
	reg |= (0x3 & tparams) << 12;
	reg |= (arb_rate & 0xff) << 16;

	p->dma_write(reg, GCR, 0);
}
EXPORT_SYMBOL(omap_dma_set_global_params);

/**
 * @brief omap_dma_set_prio_lch : Set channel wise priority settings
 *
 * @param lch
 * @param read_prio - Read priority
 * @param write_prio - Write priority
 * Both of the above can be set with one of the following values :
 * 	DMA_CH_PRIO_HIGH/DMA_CH_PRIO_LOW
 */
static int
omap_dma_set_prio_lch(int lch, unsigned char read_prio,
		      unsigned char write_prio)
{
	u32 l;

	if (unlikely((lch < 0 || lch >= dma_lch_count))) {
		printk(KERN_ERR "Invalid channel id\n");
		return -EINVAL;
	}
	l = p->dma_read(CCR, lch);
	l &= ~((1 << 6) | (1 << 26));
	if (d->dev_caps & IS_RW_PRIORITY)
		l |= ((read_prio & 0x1) << 6) | ((write_prio & 0x1) << 26);
	else
		l |= ((read_prio & 0x1) << 6);

	p->dma_write(l, CCR, lch);

	return 0;
}


/*
 * Clears any DMA state so the DMA engine is ready to restart with new buffers
 * through omap_start_dma(). Any buffers in flight are discarded.
 */
static void omap_clear_dma(int lch)
{
	unsigned long flags;

	local_irq_save(flags);
	p->clear_dma(lch);
	local_irq_restore(flags);
}

void omap_start_dma(int lch)
{
	u32 l;

	/*
	 * The CPC/CDAC register needs to be initialized to zero
	 * before starting dma transfer.
	 */
	if (dma_omap15xx())
		p->dma_write(0, CPC, lch);
	else
		p->dma_write(0, CDAC, lch);

	if (!omap_dma_in_1510_mode() && dma_chan[lch].next_lch != -1) {
		int next_lch, cur_lch;
		char dma_chan_link_map[MAX_LOGICAL_DMA_CH_COUNT];

		/* Set the link register of the first channel */
		enable_lnk(lch);

		memset(dma_chan_link_map, 0, sizeof(dma_chan_link_map));
		dma_chan_link_map[lch] = 1;

		cur_lch = dma_chan[lch].next_lch;
		do {
			next_lch = dma_chan[cur_lch].next_lch;

			/* The loop case: we've been here already */
			if (dma_chan_link_map[cur_lch])
				break;
			/* Mark the current channel */
			dma_chan_link_map[cur_lch] = 1;

			enable_lnk(cur_lch);
			omap_enable_channel_irq(cur_lch);

			cur_lch = next_lch;
		} while (next_lch != -1);
	} else if (IS_DMA_ERRATA(DMA_ERRATA_PARALLEL_CHANNELS))
		p->dma_write(lch, CLNK_CTRL, lch);

	omap_enable_channel_irq(lch);

	l = p->dma_read(CCR, lch);

	if (IS_DMA_ERRATA(DMA_ERRATA_IFRAME_BUFFERING))
			l |= OMAP_DMA_CCR_BUFFERING_DISABLE;
	l |= OMAP_DMA_CCR_EN;

	/*
	 * As dma_write() uses IO accessors which are weakly ordered, there
	 * is no guarantee that data in coherent DMA memory will be visible
	 * to the DMA device.  Add a memory barrier here to ensure that any
	 * such data is visible prior to enabling DMA.
	 */
	mb();
	p->dma_write(l, CCR, lch);

	dma_chan[lch].flags |= OMAP_DMA_ACTIVE;
}
EXPORT_SYMBOL(omap_start_dma);

void omap_stop_dma(int lch)
{
	u32 l;

	/* Disable all interrupts on the channel */
	omap_disable_channel_irq(lch);

	l = p->dma_read(CCR, lch);
	if (IS_DMA_ERRATA(DMA_ERRATA_i541) &&
			(l & OMAP_DMA_CCR_SEL_SRC_DST_SYNC)) {
		int i = 0;
		u32 sys_cf;

		/* Configure No-Standby */
		l = p->dma_read(OCP_SYSCONFIG, lch);
		sys_cf = l;
		l &= ~DMA_SYSCONFIG_MIDLEMODE_MASK;
		l |= DMA_SYSCONFIG_MIDLEMODE(DMA_IDLEMODE_NO_IDLE);
		p->dma_write(l , OCP_SYSCONFIG, 0);

		l = p->dma_read(CCR, lch);
		l &= ~OMAP_DMA_CCR_EN;
		p->dma_write(l, CCR, lch);

		/* Wait for sDMA FIFO drain */
		l = p->dma_read(CCR, lch);
		while (i < 100 && (l & (OMAP_DMA_CCR_RD_ACTIVE |
					OMAP_DMA_CCR_WR_ACTIVE))) {
			udelay(5);
			i++;
			l = p->dma_read(CCR, lch);
		}
		if (i >= 100)
			pr_err("DMA drain did not complete on lch %d\n", lch);
		/* Restore OCP_SYSCONFIG */
		p->dma_write(sys_cf, OCP_SYSCONFIG, lch);
	} else {
		l &= ~OMAP_DMA_CCR_EN;
		p->dma_write(l, CCR, lch);
	}

	/*
	 * Ensure that data transferred by DMA is visible to any access
	 * after DMA has been disabled.  This is important for coherent
	 * DMA regions.
	 */
	mb();

	if (!omap_dma_in_1510_mode() && dma_chan[lch].next_lch != -1) {
		int next_lch, cur_lch = lch;
		char dma_chan_link_map[MAX_LOGICAL_DMA_CH_COUNT];

		memset(dma_chan_link_map, 0, sizeof(dma_chan_link_map));
		do {
			/* The loop case: we've been here already */
			if (dma_chan_link_map[cur_lch])
				break;
			/* Mark the current channel */
			dma_chan_link_map[cur_lch] = 1;

			disable_lnk(cur_lch);

			next_lch = dma_chan[cur_lch].next_lch;
			cur_lch = next_lch;
		} while (next_lch != -1);
	}

	dma_chan[lch].flags &= ~OMAP_DMA_ACTIVE;
}
EXPORT_SYMBOL(omap_stop_dma);

/*
 * Allows changing the DMA callback function or data. This may be needed if
 * the driver shares a single DMA channel for multiple dma triggers.
 */
int omap_set_dma_callback(int lch,
			  void (*callback)(int lch, u16 ch_status, void *data),
			  void *data)
{
	unsigned long flags;

	if (lch < 0)
		return -ENODEV;

	spin_lock_irqsave(&dma_chan_lock, flags);
	if (dma_chan[lch].dev_id == -1) {
		printk(KERN_ERR "DMA callback for not set for free channel\n");
		spin_unlock_irqrestore(&dma_chan_lock, flags);
		return -EINVAL;
	}
	dma_chan[lch].callback = callback;
	dma_chan[lch].data = data;
	spin_unlock_irqrestore(&dma_chan_lock, flags);

	return 0;
}
EXPORT_SYMBOL(omap_set_dma_callback);

/*
 * Returns current physical source address for the given DMA channel.
 * If the channel is running the caller must disable interrupts prior calling
 * this function and process the returned value before re-enabling interrupt to
 * prevent races with the interrupt handler. Note that in continuous mode there
 * is a chance for CSSA_L register overflow between the two reads resulting
 * in incorrect return value.
 */
dma_addr_t omap_get_dma_src_pos(int lch)
{
	dma_addr_t offset = 0;

	if (dma_omap15xx())
		offset = p->dma_read(CPC, lch);
	else
		offset = p->dma_read(CSAC, lch);

	if (IS_DMA_ERRATA(DMA_ERRATA_3_3) && offset == 0)
		offset = p->dma_read(CSAC, lch);

	if (!dma_omap15xx()) {
		/*
		 * CDAC == 0 indicates that the DMA transfer on the channel has
		 * not been started (no data has been transferred so far).
		 * Return the programmed source start address in this case.
		 */
		if (likely(p->dma_read(CDAC, lch)))
			offset = p->dma_read(CSAC, lch);
		else
			offset = p->dma_read(CSSA, lch);
	}

	if (dma_omap1())
		offset |= (p->dma_read(CSSA, lch) & 0xFFFF0000);

	return offset;
}
EXPORT_SYMBOL(omap_get_dma_src_pos);

/*
 * Returns current physical destination address for the given DMA channel.
 * If the channel is running the caller must disable interrupts prior calling
 * this function and process the returned value before re-enabling interrupt to
 * prevent races with the interrupt handler. Note that in continuous mode there
 * is a chance for CDSA_L register overflow between the two reads resulting
 * in incorrect return value.
 */
dma_addr_t omap_get_dma_dst_pos(int lch)
{
	dma_addr_t offset = 0;

	if (dma_omap15xx())
		offset = p->dma_read(CPC, lch);
	else
		offset = p->dma_read(CDAC, lch);

	/*
	 * omap 3.2/3.3 erratum: sometimes 0 is returned if CSAC/CDAC is
	 * read before the DMA controller finished disabling the channel.
	 */
	if (!dma_omap15xx() && offset == 0) {
		offset = p->dma_read(CDAC, lch);
		/*
		 * CDAC == 0 indicates that the DMA transfer on the channel has
		 * not been started (no data has been transferred so far).
		 * Return the programmed destination start address in this case.
		 */
		if (unlikely(!offset))
			offset = p->dma_read(CDSA, lch);
	}

	if (dma_omap1())
		offset |= (p->dma_read(CDSA, lch) & 0xFFFF0000);

	return offset;
}
EXPORT_SYMBOL(omap_get_dma_dst_pos);

int omap_get_dma_active_status(int lch)
{
	return (p->dma_read(CCR, lch) & OMAP_DMA_CCR_EN) != 0;
}
EXPORT_SYMBOL(omap_get_dma_active_status);

int omap_dma_running(void)
{
	int lch;

	if (dma_omap1())
		if (omap_lcd_dma_running())
			return 1;

	for (lch = 0; lch < dma_chan_count; lch++)
		if (p->dma_read(CCR, lch) & OMAP_DMA_CCR_EN)
			return 1;

	return 0;
}

/*
 * lch_queue DMA will start right after lch_head one is finished.
 * For this DMA link to start, you still need to start (see omap_start_dma)
 * the first one. That will fire up the entire queue.
 */
void omap_dma_link_lch(int lch_head, int lch_queue)
{
	if (omap_dma_in_1510_mode()) {
		if (lch_head == lch_queue) {
			p->dma_write(p->dma_read(CCR, lch_head) | (3 << 8),
								CCR, lch_head);
			return;
		}
		printk(KERN_ERR "DMA linking is not supported in 1510 mode\n");
		BUG();
		return;
	}

	if ((dma_chan[lch_head].dev_id == -1) ||
	    (dma_chan[lch_queue].dev_id == -1)) {
		pr_err("omap_dma: trying to link non requested channels\n");
		dump_stack();
	}

	dma_chan[lch_head].next_lch = lch_queue;
}
EXPORT_SYMBOL(omap_dma_link_lch);

/*----------------------------------------------------------------------------*/

#ifdef CONFIG_ARCH_OMAP1

static int omap1_dma_handle_ch(int ch)
{
	u32 csr;

	if (enable_1510_mode && ch >= 6) {
		csr = dma_chan[ch].saved_csr;
		dma_chan[ch].saved_csr = 0;
	} else
		csr = p->dma_read(CSR, ch);
	if (enable_1510_mode && ch <= 2 && (csr >> 7) != 0) {
		dma_chan[ch + 6].saved_csr = csr >> 7;
		csr &= 0x7f;
	}
	if ((csr & 0x3f) == 0)
		return 0;
	if (unlikely(dma_chan[ch].dev_id == -1)) {
		pr_warn("Spurious interrupt from DMA channel %d (CSR %04x)\n",
			ch, csr);
		return 0;
	}
	if (unlikely(csr & OMAP1_DMA_TOUT_IRQ))
		pr_warn("DMA timeout with device %d\n", dma_chan[ch].dev_id);
	if (unlikely(csr & OMAP_DMA_DROP_IRQ))
		pr_warn("DMA synchronization event drop occurred with device %d\n",
			dma_chan[ch].dev_id);
	if (likely(csr & OMAP_DMA_BLOCK_IRQ))
		dma_chan[ch].flags &= ~OMAP_DMA_ACTIVE;
	if (likely(dma_chan[ch].callback != NULL))
		dma_chan[ch].callback(ch, csr, dma_chan[ch].data);

	return 1;
}

static irqreturn_t omap1_dma_irq_handler(int irq, void *dev_id)
{
	int ch = ((int) dev_id) - 1;
	int handled = 0;

	for (;;) {
		int handled_now = 0;

		handled_now += omap1_dma_handle_ch(ch);
		if (enable_1510_mode && dma_chan[ch + 6].saved_csr)
			handled_now += omap1_dma_handle_ch(ch + 6);
		if (!handled_now)
			break;
		handled += handled_now;
	}

	return handled ? IRQ_HANDLED : IRQ_NONE;
}

#else
#define omap1_dma_irq_handler	NULL
#endif

#ifdef CONFIG_ARCH_OMAP2PLUS

static int omap2_dma_handle_ch(int ch)
{
	u32 status = p->dma_read(CSR, ch);

	if (!status) {
		if (printk_ratelimit())
			pr_warn("Spurious DMA IRQ for lch %d\n", ch);
		p->dma_write(1 << ch, IRQSTATUS_L0, ch);
		return 0;
	}
	if (unlikely(dma_chan[ch].dev_id == -1)) {
		if (printk_ratelimit())
			pr_warn("IRQ %04x for non-allocated DMA channel %d\n",
				status, ch);
		return 0;
	}
	if (unlikely(status & OMAP_DMA_DROP_IRQ))
		pr_info("DMA synchronization event drop occurred with device %d\n",
			dma_chan[ch].dev_id);
	if (unlikely(status & OMAP2_DMA_TRANS_ERR_IRQ)) {
		printk(KERN_INFO "DMA transaction error with device %d\n",
		       dma_chan[ch].dev_id);
		if (IS_DMA_ERRATA(DMA_ERRATA_i378)) {
			u32 ccr;

			ccr = p->dma_read(CCR, ch);
			ccr &= ~OMAP_DMA_CCR_EN;
			p->dma_write(ccr, CCR, ch);
			dma_chan[ch].flags &= ~OMAP_DMA_ACTIVE;
		}
	}
	if (unlikely(status & OMAP2_DMA_SECURE_ERR_IRQ))
		printk(KERN_INFO "DMA secure error with device %d\n",
		       dma_chan[ch].dev_id);
	if (unlikely(status & OMAP2_DMA_MISALIGNED_ERR_IRQ))
		printk(KERN_INFO "DMA misaligned error with device %d\n",
		       dma_chan[ch].dev_id);

	p->dma_write(status, CSR, ch);
	p->dma_write(1 << ch, IRQSTATUS_L0, ch);
	/* read back the register to flush the write */
	p->dma_read(IRQSTATUS_L0, ch);

	/* If the ch is not chained then chain_id will be -1 */
	if (dma_chan[ch].chain_id != -1) {
		int chain_id = dma_chan[ch].chain_id;
		dma_chan[ch].state = DMA_CH_NOTSTARTED;
		if (p->dma_read(CLNK_CTRL, ch) & (1 << 15))
			dma_chan[dma_chan[ch].next_linked_ch].state =
							DMA_CH_STARTED;
		if (dma_linked_lch[chain_id].chain_mode ==
						OMAP_DMA_DYNAMIC_CHAIN)
			disable_lnk(ch);

		if (!OMAP_DMA_CHAIN_QEMPTY(chain_id))
			OMAP_DMA_CHAIN_INCQHEAD(chain_id);

		status = p->dma_read(CSR, ch);
		p->dma_write(status, CSR, ch);
	}

	if (likely(dma_chan[ch].callback != NULL))
		dma_chan[ch].callback(ch, status, dma_chan[ch].data);

	return 0;
}

/* STATUS register count is from 1-32 while our is 0-31 */
static irqreturn_t omap2_dma_irq_handler(int irq, void *dev_id)
{
	u32 val, enable_reg;
	int i;

	val = p->dma_read(IRQSTATUS_L0, 0);
	if (val == 0) {
		if (printk_ratelimit())
			printk(KERN_WARNING "Spurious DMA IRQ\n");
		return IRQ_HANDLED;
	}
	enable_reg = p->dma_read(IRQENABLE_L0, 0);
	val &= enable_reg; /* Dispatch only relevant interrupts */
	for (i = 0; i < dma_lch_count && val != 0; i++) {
		if (val & 1)
			omap2_dma_handle_ch(i);
		val >>= 1;
	}

	return IRQ_HANDLED;
}

static struct irqaction omap24xx_dma_irq = {
	.name = "DMA",
	.handler = omap2_dma_irq_handler,
};

#else
static struct irqaction omap24xx_dma_irq;
#endif

/*----------------------------------------------------------------------------*/

/*
 * Note that we are currently using only IRQENABLE_L0 and L1.
 * As the DSP may be using IRQENABLE_L2 and L3, let's not
 * touch those for now.
 */
void omap_dma_global_context_save(void)
{
	omap_dma_global_context.dma_irqenable_l0 =
		p->dma_read(IRQENABLE_L0, 0);
	omap_dma_global_context.dma_irqenable_l1 =
		p->dma_read(IRQENABLE_L1, 0);
	omap_dma_global_context.dma_ocp_sysconfig =
		p->dma_read(OCP_SYSCONFIG, 0);
	omap_dma_global_context.dma_gcr = p->dma_read(GCR, 0);
}

void omap_dma_global_context_restore(void)
{
	int ch;

	p->dma_write(omap_dma_global_context.dma_gcr, GCR, 0);
	p->dma_write(omap_dma_global_context.dma_ocp_sysconfig,
		OCP_SYSCONFIG, 0);
	p->dma_write(omap_dma_global_context.dma_irqenable_l0,
		IRQENABLE_L0, 0);
	p->dma_write(omap_dma_global_context.dma_irqenable_l1,
		IRQENABLE_L1, 0);

	if (IS_DMA_ERRATA(DMA_ROMCODE_BUG))
		p->dma_write(0x3 , IRQSTATUS_L0, 0);

	for (ch = 0; ch < dma_chan_count; ch++)
		if (dma_chan[ch].dev_id != -1)
			omap_clear_dma(ch);
}

struct omap_system_dma_plat_info *omap_get_plat_info(void)
{
	return p;
}
EXPORT_SYMBOL_GPL(omap_get_plat_info);

static int omap_system_dma_probe(struct platform_device *pdev)
{
	int ch, ret = 0;
	int dma_irq;
	char irq_name[4];
	int irq_rel;

	p = pdev->dev.platform_data;
	if (!p) {
		dev_err(&pdev->dev,
			"%s: System DMA initialized without platform data\n",
			__func__);
		return -EINVAL;
	}

	d			= p->dma_attr;
	errata			= p->errata;

	if ((d->dev_caps & RESERVE_CHANNEL) && omap_dma_reserve_channels
			&& (omap_dma_reserve_channels < d->lch_count))
		d->lch_count	= omap_dma_reserve_channels;

	dma_lch_count		= d->lch_count;
	dma_chan_count		= dma_lch_count;
	enable_1510_mode	= d->dev_caps & ENABLE_1510_MODE;

	dma_chan = devm_kcalloc(&pdev->dev, dma_lch_count,
				sizeof(struct omap_dma_lch), GFP_KERNEL);
	if (!dma_chan) {
		dev_err(&pdev->dev, "%s: kzalloc fail\n", __func__);
		return -ENOMEM;
	}


	if (dma_omap2plus()) {
		dma_linked_lch = kzalloc(sizeof(struct dma_link_info) *
						dma_lch_count, GFP_KERNEL);
		if (!dma_linked_lch) {
			ret = -ENOMEM;
			goto exit_dma_lch_fail;
		}
	}

	spin_lock_init(&dma_chan_lock);
	for (ch = 0; ch < dma_chan_count; ch++) {
		omap_clear_dma(ch);
		if (dma_omap2plus())
			omap2_disable_irq_lch(ch);

		dma_chan[ch].dev_id = -1;
		dma_chan[ch].next_lch = -1;

		if (ch >= 6 && enable_1510_mode)
			continue;

		if (dma_omap1()) {
			/*
			 * request_irq() doesn't like dev_id (ie. ch) being
			 * zero, so we have to kludge around this.
			 */
			sprintf(&irq_name[0], "%d", ch);
			dma_irq = platform_get_irq_byname(pdev, irq_name);

			if (dma_irq < 0) {
				ret = dma_irq;
				goto exit_dma_irq_fail;
			}

			/* INT_DMA_LCD is handled in lcd_dma.c */
			if (dma_irq == INT_DMA_LCD)
				continue;

			ret = request_irq(dma_irq,
					omap1_dma_irq_handler, 0, "DMA",
					(void *) (ch + 1));
			if (ret != 0)
				goto exit_dma_irq_fail;
		}
	}

	if (d->dev_caps & IS_RW_PRIORITY)
		omap_dma_set_global_params(DMA_DEFAULT_ARB_RATE,
				DMA_DEFAULT_FIFO_DEPTH, 0);

	if (dma_omap2plus() && !(d->dev_caps & DMA_ENGINE_HANDLE_IRQ)) {
		strcpy(irq_name, "0");
		dma_irq = platform_get_irq_byname(pdev, irq_name);
		if (dma_irq < 0) {
			dev_err(&pdev->dev, "failed: request IRQ %d", dma_irq);
			ret = dma_irq;
			goto exit_dma_lch_fail;
		}
		ret = setup_irq(dma_irq, &omap24xx_dma_irq);
		if (ret) {
			dev_err(&pdev->dev, "set_up failed for IRQ %d for DMA (error %d)\n",
				dma_irq, ret);
			goto exit_dma_lch_fail;
		}
	}

	/* reserve dma channels 0 and 1 in high security devices on 34xx */
	if (d->dev_caps & HS_CHANNELS_RESERVED) {
		pr_info("Reserving DMA channels 0 and 1 for HS ROM code\n");
		dma_chan[0].dev_id = 0;
		dma_chan[1].dev_id = 1;
	}
	p->show_dma_caps();
	return 0;

exit_dma_irq_fail:
	dev_err(&pdev->dev, "unable to request IRQ %d for DMA (error %d)\n",
		dma_irq, ret);
	for (irq_rel = 0; irq_rel < ch;	irq_rel++) {
		dma_irq = platform_get_irq(pdev, irq_rel);
		free_irq(dma_irq, (void *)(irq_rel + 1));
	}

exit_dma_lch_fail:
	return ret;
}

static int omap_system_dma_remove(struct platform_device *pdev)
{
	int dma_irq;

	if (dma_omap2plus()) {
		char irq_name[4];
		strcpy(irq_name, "0");
		dma_irq = platform_get_irq_byname(pdev, irq_name);
		if (dma_irq >= 0)
			remove_irq(dma_irq, &omap24xx_dma_irq);
	} else {
		int irq_rel = 0;
		for ( ; irq_rel < dma_chan_count; irq_rel++) {
			dma_irq = platform_get_irq(pdev, irq_rel);
			free_irq(dma_irq, (void *)(irq_rel + 1));
		}
	}
	return 0;
}

static struct platform_driver omap_system_dma_driver = {
	.probe		= omap_system_dma_probe,
	.remove		= omap_system_dma_remove,
	.driver		= {
		.name	= "omap_dma_system"
	},
};

static int __init omap_system_dma_init(void)
{
	return platform_driver_register(&omap_system_dma_driver);
}
arch_initcall(omap_system_dma_init);

static void __exit omap_system_dma_exit(void)
{
	platform_driver_unregister(&omap_system_dma_driver);
}

MODULE_DESCRIPTION("OMAP SYSTEM DMA DRIVER");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRIVER_NAME);
MODULE_AUTHOR("Texas Instruments Inc");

/*
 * Reserve the omap SDMA channels using cmdline bootarg
 * "omap_dma_reserve_ch=". The valid range is 1 to 32
 */
static int __init omap_dma_cmdline_reserve_ch(char *str)
{
	if (get_option(&str, &omap_dma_reserve_channels) != 1)
		omap_dma_reserve_channels = 0;
	return 1;
}

__setup("omap_dma_reserve_ch=", omap_dma_cmdline_reserve_ch);


