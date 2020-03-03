/*
 * EDMA3 support for DaVinci
 *
 * Copyright (C) 2006-2009 Texas Instruments.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/edma.h>
#include <linux/dma-mapping.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_dma.h>
#include <linux/of_irq.h>
#include <linux/pm_runtime.h>

#include <linux/platform_data/edma.h>

/* Offsets matching "struct edmacc_param" */
#define PARM_OPT		0x00
#define PARM_SRC		0x04
#define PARM_A_B_CNT		0x08
#define PARM_DST		0x0c
#define PARM_SRC_DST_BIDX	0x10
#define PARM_LINK_BCNTRLD	0x14
#define PARM_SRC_DST_CIDX	0x18
#define PARM_CCNT		0x1c

#define PARM_SIZE		0x20

/* Offsets for EDMA CC global channel registers and their shadows */
#define SH_ER		0x00	/* 64 bits */
#define SH_ECR		0x08	/* 64 bits */
#define SH_ESR		0x10	/* 64 bits */
#define SH_CER		0x18	/* 64 bits */
#define SH_EER		0x20	/* 64 bits */
#define SH_EECR		0x28	/* 64 bits */
#define SH_EESR		0x30	/* 64 bits */
#define SH_SER		0x38	/* 64 bits */
#define SH_SECR		0x40	/* 64 bits */
#define SH_IER		0x50	/* 64 bits */
#define SH_IECR		0x58	/* 64 bits */
#define SH_IESR		0x60	/* 64 bits */
#define SH_IPR		0x68	/* 64 bits */
#define SH_ICR		0x70	/* 64 bits */
#define SH_IEVAL	0x78
#define SH_QER		0x80
#define SH_QEER		0x84
#define SH_QEECR	0x88
#define SH_QEESR	0x8c
#define SH_QSER		0x90
#define SH_QSECR	0x94
#define SH_SIZE		0x200

/* Offsets for EDMA CC global registers */
#define EDMA_REV	0x0000
#define EDMA_CCCFG	0x0004
#define EDMA_QCHMAP	0x0200	/* 8 registers */
#define EDMA_DMAQNUM	0x0240	/* 8 registers (4 on OMAP-L1xx) */
#define EDMA_QDMAQNUM	0x0260
#define EDMA_QUETCMAP	0x0280
#define EDMA_QUEPRI	0x0284
#define EDMA_EMR	0x0300	/* 64 bits */
#define EDMA_EMCR	0x0308	/* 64 bits */
#define EDMA_QEMR	0x0310
#define EDMA_QEMCR	0x0314
#define EDMA_CCERR	0x0318
#define EDMA_CCERRCLR	0x031c
#define EDMA_EEVAL	0x0320
#define EDMA_DRAE	0x0340	/* 4 x 64 bits*/
#define EDMA_QRAE	0x0380	/* 4 registers */
#define EDMA_QUEEVTENTRY	0x0400	/* 2 x 16 registers */
#define EDMA_QSTAT	0x0600	/* 2 registers */
#define EDMA_QWMTHRA	0x0620
#define EDMA_QWMTHRB	0x0624
#define EDMA_CCSTAT	0x0640

#define EDMA_M		0x1000	/* global channel registers */
#define EDMA_ECR	0x1008
#define EDMA_ECRH	0x100C
#define EDMA_SHADOW0	0x2000	/* 4 regions shadowing global channels */
#define EDMA_PARM	0x4000	/* 128 param entries */

#define PARM_OFFSET(param_no)	(EDMA_PARM + ((param_no) << 5))

#define EDMA_DCHMAP	0x0100  /* 64 registers */

/* CCCFG register */
#define GET_NUM_DMACH(x)	(x & 0x7) /* bits 0-2 */
#define GET_NUM_PAENTRY(x)	((x & 0x7000) >> 12) /* bits 12-14 */
#define GET_NUM_EVQUE(x)	((x & 0x70000) >> 16) /* bits 16-18 */
#define GET_NUM_REGN(x)		((x & 0x300000) >> 20) /* bits 20-21 */
#define CHMAP_EXIST		BIT(24)

#define EDMA_MAX_DMACH           64
#define EDMA_MAX_PARAMENTRY     512

/*****************************************************************************/

static void __iomem *edmacc_regs_base[EDMA_MAX_CC];

static inline unsigned int edma_read(unsigned ctlr, int offset)
{
	return (unsigned int)__raw_readl(edmacc_regs_base[ctlr] + offset);
}

static inline void edma_write(unsigned ctlr, int offset, int val)
{
	__raw_writel(val, edmacc_regs_base[ctlr] + offset);
}
static inline void edma_modify(unsigned ctlr, int offset, unsigned and,
		unsigned or)
{
	unsigned val = edma_read(ctlr, offset);
	val &= and;
	val |= or;
	edma_write(ctlr, offset, val);
}
static inline void edma_and(unsigned ctlr, int offset, unsigned and)
{
	unsigned val = edma_read(ctlr, offset);
	val &= and;
	edma_write(ctlr, offset, val);
}
static inline void edma_or(unsigned ctlr, int offset, unsigned or)
{
	unsigned val = edma_read(ctlr, offset);
	val |= or;
	edma_write(ctlr, offset, val);
}
static inline unsigned int edma_read_array(unsigned ctlr, int offset, int i)
{
	return edma_read(ctlr, offset + (i << 2));
}
static inline void edma_write_array(unsigned ctlr, int offset, int i,
		unsigned val)
{
	edma_write(ctlr, offset + (i << 2), val);
}
static inline void edma_modify_array(unsigned ctlr, int offset, int i,
		unsigned and, unsigned or)
{
	edma_modify(ctlr, offset + (i << 2), and, or);
}
static inline void edma_or_array(unsigned ctlr, int offset, int i, unsigned or)
{
	edma_or(ctlr, offset + (i << 2), or);
}
static inline void edma_or_array2(unsigned ctlr, int offset, int i, int j,
		unsigned or)
{
	edma_or(ctlr, offset + ((i*2 + j) << 2), or);
}
static inline void edma_write_array2(unsigned ctlr, int offset, int i, int j,
		unsigned val)
{
	edma_write(ctlr, offset + ((i*2 + j) << 2), val);
}
static inline unsigned int edma_shadow0_read(unsigned ctlr, int offset)
{
	return edma_read(ctlr, EDMA_SHADOW0 + offset);
}
static inline unsigned int edma_shadow0_read_array(unsigned ctlr, int offset,
		int i)
{
	return edma_read(ctlr, EDMA_SHADOW0 + offset + (i << 2));
}
static inline void edma_shadow0_write(unsigned ctlr, int offset, unsigned val)
{
	edma_write(ctlr, EDMA_SHADOW0 + offset, val);
}
static inline void edma_shadow0_write_array(unsigned ctlr, int offset, int i,
		unsigned val)
{
	edma_write(ctlr, EDMA_SHADOW0 + offset + (i << 2), val);
}
static inline unsigned int edma_parm_read(unsigned ctlr, int offset,
		int param_no)
{
	return edma_read(ctlr, EDMA_PARM + offset + (param_no << 5));
}
static inline void edma_parm_write(unsigned ctlr, int offset, int param_no,
		unsigned val)
{
	edma_write(ctlr, EDMA_PARM + offset + (param_no << 5), val);
}
static inline void edma_parm_modify(unsigned ctlr, int offset, int param_no,
		unsigned and, unsigned or)
{
	edma_modify(ctlr, EDMA_PARM + offset + (param_no << 5), and, or);
}
static inline void edma_parm_and(unsigned ctlr, int offset, int param_no,
		unsigned and)
{
	edma_and(ctlr, EDMA_PARM + offset + (param_no << 5), and);
}
static inline void edma_parm_or(unsigned ctlr, int offset, int param_no,
		unsigned or)
{
	edma_or(ctlr, EDMA_PARM + offset + (param_no << 5), or);
}

static inline void set_bits(int offset, int len, unsigned long *p)
{
	for (; len > 0; len--)
		set_bit(offset + (len - 1), p);
}

static inline void clear_bits(int offset, int len, unsigned long *p)
{
	for (; len > 0; len--)
		clear_bit(offset + (len - 1), p);
}

/*****************************************************************************/

/* actual number of DMA channels and slots on this silicon */
struct edma {
	/* how many dma resources of each type */
	unsigned	num_channels;
	unsigned	num_region;
	unsigned	num_slots;
	unsigned	num_tc;
	enum dma_event_q 	default_queue;

	/* list of channels with no even trigger; terminated by "-1" */
	const s8	*noevent;

	struct edma_soc_info *info;

	/* The edma_inuse bit for each PaRAM slot is clear unless the
	 * channel is in use ... by ARM or DSP, for QDMA, or whatever.
	 */
	DECLARE_BITMAP(edma_inuse, EDMA_MAX_PARAMENTRY);

	/* The edma_unused bit for each channel is clear unless
	 * it is not being used on this platform. It uses a bit
	 * of SOC-specific initialization code.
	 */
	DECLARE_BITMAP(edma_unused, EDMA_MAX_DMACH);

	unsigned	irq_res_start;
	unsigned	irq_res_end;

	struct dma_interrupt_data {
		void (*callback)(unsigned channel, unsigned short ch_status,
				void *data);
		void *data;
	} intr_data[EDMA_MAX_DMACH];
};

static struct edma *edma_cc[EDMA_MAX_CC];
static int arch_num_cc;

/* dummy param set used to (re)initialize parameter RAM slots */
static const struct edmacc_param dummy_paramset = {
	.link_bcntrld = 0xffff,
	.ccnt = 1,
};

static const struct of_device_id edma_of_ids[] = {
	{ .compatible = "ti,edma3", },
	{}
};

/*****************************************************************************/

static void map_dmach_queue(unsigned ctlr, unsigned ch_no,
		enum dma_event_q queue_no)
{
	int bit = (ch_no & 0x7) * 4;

	/* default to low priority queue */
	if (queue_no == EVENTQ_DEFAULT)
		queue_no = edma_cc[ctlr]->default_queue;

	queue_no &= 7;
	edma_modify_array(ctlr, EDMA_DMAQNUM, (ch_no >> 3),
			~(0x7 << bit), queue_no << bit);
}

static void assign_priority_to_queue(unsigned ctlr, int queue_no,
		int priority)
{
	int bit = queue_no * 4;
	edma_modify(ctlr, EDMA_QUEPRI, ~(0x7 << bit),
			((priority & 0x7) << bit));
}

/**
 * map_dmach_param - Maps channel number to param entry number
 *
 * This maps the dma channel number to param entry numberter. In
 * other words using the DMA channel mapping registers a param entry
 * can be mapped to any channel
 *
 * Callers are responsible for ensuring the channel mapping logic is
 * included in that particular EDMA variant (Eg : dm646x)
 *
 */
static void map_dmach_param(unsigned ctlr)
{
	int i;
	for (i = 0; i < EDMA_MAX_DMACH; i++)
		edma_write_array(ctlr, EDMA_DCHMAP , i , (i << 5));
}

static inline void
setup_dma_interrupt(unsigned lch,
	void (*callback)(unsigned channel, u16 ch_status, void *data),
	void *data)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(lch);
	lch = EDMA_CHAN_SLOT(lch);

	if (!callback)
		edma_shadow0_write_array(ctlr, SH_IECR, lch >> 5,
				BIT(lch & 0x1f));

	edma_cc[ctlr]->intr_data[lch].callback = callback;
	edma_cc[ctlr]->intr_data[lch].data = data;

	if (callback) {
		edma_shadow0_write_array(ctlr, SH_ICR, lch >> 5,
				BIT(lch & 0x1f));
		edma_shadow0_write_array(ctlr, SH_IESR, lch >> 5,
				BIT(lch & 0x1f));
	}
}

static int irq2ctlr(int irq)
{
	if (irq >= edma_cc[0]->irq_res_start && irq <= edma_cc[0]->irq_res_end)
		return 0;
	else if (irq >= edma_cc[1]->irq_res_start &&
		irq <= edma_cc[1]->irq_res_end)
		return 1;

	return -1;
}

/******************************************************************************
 *
 * DMA interrupt handler
 *
 *****************************************************************************/
static irqreturn_t dma_irq_handler(int irq, void *data)
{
	int ctlr;
	u32 sh_ier;
	u32 sh_ipr;
	u32 bank;

	ctlr = irq2ctlr(irq);
	if (ctlr < 0)
		return IRQ_NONE;

	dev_dbg(data, "dma_irq_handler\n");

	sh_ipr = edma_shadow0_read_array(ctlr, SH_IPR, 0);
	if (!sh_ipr) {
		sh_ipr = edma_shadow0_read_array(ctlr, SH_IPR, 1);
		if (!sh_ipr)
			return IRQ_NONE;
		sh_ier = edma_shadow0_read_array(ctlr, SH_IER, 1);
		bank = 1;
	} else {
		sh_ier = edma_shadow0_read_array(ctlr, SH_IER, 0);
		bank = 0;
	}

	do {
		u32 slot;
		u32 channel;

		dev_dbg(data, "IPR%d %08x\n", bank, sh_ipr);

		slot = __ffs(sh_ipr);
		sh_ipr &= ~(BIT(slot));

		if (sh_ier & BIT(slot)) {
			channel = (bank << 5) | slot;
			/* Clear the corresponding IPR bits */
			edma_shadow0_write_array(ctlr, SH_ICR, bank,
					BIT(slot));
			if (edma_cc[ctlr]->intr_data[channel].callback)
				edma_cc[ctlr]->intr_data[channel].callback(
					EDMA_CTLR_CHAN(ctlr, channel),
					EDMA_DMA_COMPLETE,
					edma_cc[ctlr]->intr_data[channel].data);
		}
	} while (sh_ipr);

	edma_shadow0_write(ctlr, SH_IEVAL, 1);
	return IRQ_HANDLED;
}

/******************************************************************************
 *
 * DMA error interrupt handler
 *
 *****************************************************************************/
static irqreturn_t dma_ccerr_handler(int irq, void *data)
{
	int i;
	int ctlr;
	unsigned int cnt = 0;

	ctlr = irq2ctlr(irq);
	if (ctlr < 0)
		return IRQ_NONE;

	dev_dbg(data, "dma_ccerr_handler\n");

	if ((edma_read_array(ctlr, EDMA_EMR, 0) == 0) &&
	    (edma_read_array(ctlr, EDMA_EMR, 1) == 0) &&
	    (edma_read(ctlr, EDMA_QEMR) == 0) &&
	    (edma_read(ctlr, EDMA_CCERR) == 0))
		return IRQ_NONE;

	while (1) {
		int j = -1;
		if (edma_read_array(ctlr, EDMA_EMR, 0))
			j = 0;
		else if (edma_read_array(ctlr, EDMA_EMR, 1))
			j = 1;
		if (j >= 0) {
			dev_dbg(data, "EMR%d %08x\n", j,
					edma_read_array(ctlr, EDMA_EMR, j));
			for (i = 0; i < 32; i++) {
				int k = (j << 5) + i;
				if (edma_read_array(ctlr, EDMA_EMR, j) &
							BIT(i)) {
					/* Clear the corresponding EMR bits */
					edma_write_array(ctlr, EDMA_EMCR, j,
							BIT(i));
					/* Clear any SER */
					edma_shadow0_write_array(ctlr, SH_SECR,
								j, BIT(i));
					if (edma_cc[ctlr]->intr_data[k].
								callback) {
						edma_cc[ctlr]->intr_data[k].
						callback(
						EDMA_CTLR_CHAN(ctlr, k),
						EDMA_DMA_CC_ERROR,
						edma_cc[ctlr]->intr_data
						[k].data);
					}
				}
			}
		} else if (edma_read(ctlr, EDMA_QEMR)) {
			dev_dbg(data, "QEMR %02x\n",
				edma_read(ctlr, EDMA_QEMR));
			for (i = 0; i < 8; i++) {
				if (edma_read(ctlr, EDMA_QEMR) & BIT(i)) {
					/* Clear the corresponding IPR bits */
					edma_write(ctlr, EDMA_QEMCR, BIT(i));
					edma_shadow0_write(ctlr, SH_QSECR,
								BIT(i));

					/* NOTE:  not reported!! */
				}
			}
		} else if (edma_read(ctlr, EDMA_CCERR)) {
			dev_dbg(data, "CCERR %08x\n",
				edma_read(ctlr, EDMA_CCERR));
			/* FIXME:  CCERR.BIT(16) ignored!  much better
			 * to just write CCERRCLR with CCERR value...
			 */
			for (i = 0; i < 8; i++) {
				if (edma_read(ctlr, EDMA_CCERR) & BIT(i)) {
					/* Clear the corresponding IPR bits */
					edma_write(ctlr, EDMA_CCERRCLR, BIT(i));

					/* NOTE:  not reported!! */
				}
			}
		}
		if ((edma_read_array(ctlr, EDMA_EMR, 0) == 0) &&
		    (edma_read_array(ctlr, EDMA_EMR, 1) == 0) &&
		    (edma_read(ctlr, EDMA_QEMR) == 0) &&
		    (edma_read(ctlr, EDMA_CCERR) == 0))
			break;
		cnt++;
		if (cnt > 10)
			break;
	}
	edma_write(ctlr, EDMA_EEVAL, 1);
	return IRQ_HANDLED;
}

static int reserve_contiguous_slots(int ctlr, unsigned int id,
				     unsigned int num_slots,
				     unsigned int start_slot)
{
	int i, j;
	unsigned int count = num_slots;
	int stop_slot = start_slot;
	DECLARE_BITMAP(tmp_inuse, EDMA_MAX_PARAMENTRY);

	for (i = start_slot; i < edma_cc[ctlr]->num_slots; ++i) {
		j = EDMA_CHAN_SLOT(i);
		if (!test_and_set_bit(j, edma_cc[ctlr]->edma_inuse)) {
			/* Record our current beginning slot */
			if (count == num_slots)
				stop_slot = i;

			count--;
			set_bit(j, tmp_inuse);

			if (count == 0)
				break;
		} else {
			clear_bit(j, tmp_inuse);

			if (id == EDMA_CONT_PARAMS_FIXED_EXACT) {
				stop_slot = i;
				break;
			} else {
				count = num_slots;
			}
		}
	}

	/*
	 * We have to clear any bits that we set
	 * if we run out parameter RAM slots, i.e we do find a set
	 * of contiguous parameter RAM slots but do not find the exact number
	 * requested as we may reach the total number of parameter RAM slots
	 */
	if (i == edma_cc[ctlr]->num_slots)
		stop_slot = i;

	j = start_slot;
	for_each_set_bit_from(j, tmp_inuse, stop_slot)
		clear_bit(j, edma_cc[ctlr]->edma_inuse);

	if (count)
		return -EBUSY;

	for (j = i - num_slots + 1; j <= i; ++j)
		memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(j),
			&dummy_paramset, PARM_SIZE);

	return EDMA_CTLR_CHAN(ctlr, i - num_slots + 1);
}

static int prepare_unused_channel_list(struct device *dev, void *data)
{
	struct platform_device *pdev = to_platform_device(dev);
	int i, count, ctlr;
	struct of_phandle_args  dma_spec;

	if (dev->of_node) {
		count = of_property_count_strings(dev->of_node, "dma-names");
		if (count < 0)
			return 0;
		for (i = 0; i < count; i++) {
			if (of_parse_phandle_with_args(dev->of_node, "dmas",
						       "#dma-cells", i,
						       &dma_spec))
				continue;

			if (!of_match_node(edma_of_ids, dma_spec.np)) {
				of_node_put(dma_spec.np);
				continue;
			}

			clear_bit(EDMA_CHAN_SLOT(dma_spec.args[0]),
				  edma_cc[0]->edma_unused);
			of_node_put(dma_spec.np);
		}
		return 0;
	}

	/* For non-OF case */
	for (i = 0; i < pdev->num_resources; i++) {
		if ((pdev->resource[i].flags & IORESOURCE_DMA) &&
				(int)pdev->resource[i].start >= 0) {
			ctlr = EDMA_CTLR(pdev->resource[i].start);
			clear_bit(EDMA_CHAN_SLOT(pdev->resource[i].start),
				  edma_cc[ctlr]->edma_unused);
		}
	}

	return 0;
}

/*-----------------------------------------------------------------------*/

static bool unused_chan_list_done;

/* Resource alloc/free:  dma channels, parameter RAM slots */

/**
 * edma_alloc_channel - allocate DMA channel and paired parameter RAM
 * @channel: specific channel to allocate; negative for "any unmapped channel"
 * @callback: optional; to be issued on DMA completion or errors
 * @data: passed to callback
 * @eventq_no: an EVENTQ_* constant, used to choose which Transfer
 *	Controller (TC) executes requests using this channel.  Use
 *	EVENTQ_DEFAULT unless you really need a high priority queue.
 *
 * This allocates a DMA channel and its associated parameter RAM slot.
 * The parameter RAM is initialized to hold a dummy transfer.
 *
 * Normal use is to pass a specific channel number as @channel, to make
 * use of hardware events mapped to that channel.  When the channel will
 * be used only for software triggering or event chaining, channels not
 * mapped to hardware events (or mapped to unused events) are preferable.
 *
 * DMA transfers start from a channel using edma_start(), or by
 * chaining.  When the transfer described in that channel's parameter RAM
 * slot completes, that slot's data may be reloaded through a link.
 *
 * DMA errors are only reported to the @callback associated with the
 * channel driving that transfer, but transfer completion callbacks can
 * be sent to another channel under control of the TCC field in
 * the option word of the transfer's parameter RAM set.  Drivers must not
 * use DMA transfer completion callbacks for channels they did not allocate.
 * (The same applies to TCC codes used in transfer chaining.)
 *
 * Returns the number of the channel, else negative errno.
 */
int edma_alloc_channel(int channel,
		void (*callback)(unsigned channel, u16 ch_status, void *data),
		void *data,
		enum dma_event_q eventq_no)
{
	unsigned i, done = 0, ctlr = 0;
	int ret = 0;

	if (!unused_chan_list_done) {
		/*
		 * Scan all the platform devices to find out the EDMA channels
		 * used and clear them in the unused list, making the rest
		 * available for ARM usage.
		 */
		ret = bus_for_each_dev(&platform_bus_type, NULL, NULL,
				prepare_unused_channel_list);
		if (ret < 0)
			return ret;

		unused_chan_list_done = true;
	}

	if (channel >= 0) {
		ctlr = EDMA_CTLR(channel);
		channel = EDMA_CHAN_SLOT(channel);
	}

	if (channel < 0) {
		for (i = 0; i < arch_num_cc; i++) {
			channel = 0;
			for (;;) {
				channel = find_next_bit(edma_cc[i]->edma_unused,
						edma_cc[i]->num_channels,
						channel);
				if (channel == edma_cc[i]->num_channels)
					break;
				if (!test_and_set_bit(channel,
						edma_cc[i]->edma_inuse)) {
					done = 1;
					ctlr = i;
					break;
				}
				channel++;
			}
			if (done)
				break;
		}
		if (!done)
			return -ENOMEM;
	} else if (channel >= edma_cc[ctlr]->num_channels) {
		return -EINVAL;
	} else if (test_and_set_bit(channel, edma_cc[ctlr]->edma_inuse)) {
		return -EBUSY;
	}

	/* ensure access through shadow region 0 */
	edma_or_array2(ctlr, EDMA_DRAE, 0, channel >> 5, BIT(channel & 0x1f));

	/* ensure no events are pending */
	edma_stop(EDMA_CTLR_CHAN(ctlr, channel));
	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(channel),
			&dummy_paramset, PARM_SIZE);

	if (callback)
		setup_dma_interrupt(EDMA_CTLR_CHAN(ctlr, channel),
					callback, data);

	map_dmach_queue(ctlr, channel, eventq_no);

	return EDMA_CTLR_CHAN(ctlr, channel);
}
EXPORT_SYMBOL(edma_alloc_channel);


/**
 * edma_free_channel - deallocate DMA channel
 * @channel: dma channel returned from edma_alloc_channel()
 *
 * This deallocates the DMA channel and associated parameter RAM slot
 * allocated by edma_alloc_channel().
 *
 * Callers are responsible for ensuring the channel is inactive, and
 * will not be reactivated by linking, chaining, or software calls to
 * edma_start().
 */
void edma_free_channel(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel >= edma_cc[ctlr]->num_channels)
		return;

	setup_dma_interrupt(channel, NULL, NULL);
	/* REVISIT should probably take out of shadow region 0 */

	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(channel),
			&dummy_paramset, PARM_SIZE);
	clear_bit(channel, edma_cc[ctlr]->edma_inuse);
}
EXPORT_SYMBOL(edma_free_channel);

/**
 * edma_alloc_slot - allocate DMA parameter RAM
 * @slot: specific slot to allocate; negative for "any unused slot"
 *
 * This allocates a parameter RAM slot, initializing it to hold a
 * dummy transfer.  Slots allocated using this routine have not been
 * mapped to a hardware DMA channel, and will normally be used by
 * linking to them from a slot associated with a DMA channel.
 *
 * Normal use is to pass EDMA_SLOT_ANY as the @slot, but specific
 * slots may be allocated on behalf of DSP firmware.
 *
 * Returns the number of the slot, else negative errno.
 */
int edma_alloc_slot(unsigned ctlr, int slot)
{
	if (!edma_cc[ctlr])
		return -EINVAL;

	if (slot >= 0)
		slot = EDMA_CHAN_SLOT(slot);

	if (slot < 0) {
		slot = edma_cc[ctlr]->num_channels;
		for (;;) {
			slot = find_next_zero_bit(edma_cc[ctlr]->edma_inuse,
					edma_cc[ctlr]->num_slots, slot);
			if (slot == edma_cc[ctlr]->num_slots)
				return -ENOMEM;
			if (!test_and_set_bit(slot, edma_cc[ctlr]->edma_inuse))
				break;
		}
	} else if (slot < edma_cc[ctlr]->num_channels ||
			slot >= edma_cc[ctlr]->num_slots) {
		return -EINVAL;
	} else if (test_and_set_bit(slot, edma_cc[ctlr]->edma_inuse)) {
		return -EBUSY;
	}

	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(slot),
			&dummy_paramset, PARM_SIZE);

	return EDMA_CTLR_CHAN(ctlr, slot);
}
EXPORT_SYMBOL(edma_alloc_slot);

/**
 * edma_free_slot - deallocate DMA parameter RAM
 * @slot: parameter RAM slot returned from edma_alloc_slot()
 *
 * This deallocates the parameter RAM slot allocated by edma_alloc_slot().
 * Callers are responsible for ensuring the slot is inactive, and will
 * not be activated.
 */
void edma_free_slot(unsigned slot)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_channels ||
		slot >= edma_cc[ctlr]->num_slots)
		return;

	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(slot),
			&dummy_paramset, PARM_SIZE);
	clear_bit(slot, edma_cc[ctlr]->edma_inuse);
}
EXPORT_SYMBOL(edma_free_slot);


/**
 * edma_alloc_cont_slots- alloc contiguous parameter RAM slots
 * The API will return the starting point of a set of
 * contiguous parameter RAM slots that have been requested
 *
 * @id: can only be EDMA_CONT_PARAMS_ANY or EDMA_CONT_PARAMS_FIXED_EXACT
 * or EDMA_CONT_PARAMS_FIXED_NOT_EXACT
 * @count: number of contiguous Paramter RAM slots
 * @slot  - the start value of Parameter RAM slot that should be passed if id
 * is EDMA_CONT_PARAMS_FIXED_EXACT or EDMA_CONT_PARAMS_FIXED_NOT_EXACT
 *
 * If id is EDMA_CONT_PARAMS_ANY then the API starts looking for a set of
 * contiguous Parameter RAM slots from parameter RAM 64 in the case of
 * DaVinci SOCs and 32 in the case of DA8xx SOCs.
 *
 * If id is EDMA_CONT_PARAMS_FIXED_EXACT then the API starts looking for a
 * set of contiguous parameter RAM slots from the "slot" that is passed as an
 * argument to the API.
 *
 * If id is EDMA_CONT_PARAMS_FIXED_NOT_EXACT then the API initially tries
 * starts looking for a set of contiguous parameter RAMs from the "slot"
 * that is passed as an argument to the API. On failure the API will try to
 * find a set of contiguous Parameter RAM slots from the remaining Parameter
 * RAM slots
 */
int edma_alloc_cont_slots(unsigned ctlr, unsigned int id, int slot, int count)
{
	/*
	 * The start slot requested should be greater than
	 * the number of channels and lesser than the total number
	 * of slots
	 */
	if ((id != EDMA_CONT_PARAMS_ANY) &&
		(slot < edma_cc[ctlr]->num_channels ||
		slot >= edma_cc[ctlr]->num_slots))
		return -EINVAL;

	/*
	 * The number of parameter RAM slots requested cannot be less than 1
	 * and cannot be more than the number of slots minus the number of
	 * channels
	 */
	if (count < 1 || count >
		(edma_cc[ctlr]->num_slots - edma_cc[ctlr]->num_channels))
		return -EINVAL;

	switch (id) {
	case EDMA_CONT_PARAMS_ANY:
		return reserve_contiguous_slots(ctlr, id, count,
						 edma_cc[ctlr]->num_channels);
	case EDMA_CONT_PARAMS_FIXED_EXACT:
	case EDMA_CONT_PARAMS_FIXED_NOT_EXACT:
		return reserve_contiguous_slots(ctlr, id, count, slot);
	default:
		return -EINVAL;
	}

}
EXPORT_SYMBOL(edma_alloc_cont_slots);

/**
 * edma_free_cont_slots - deallocate DMA parameter RAM slots
 * @slot: first parameter RAM of a set of parameter RAM slots to be freed
 * @count: the number of contiguous parameter RAM slots to be freed
 *
 * This deallocates the parameter RAM slots allocated by
 * edma_alloc_cont_slots.
 * Callers/applications need to keep track of sets of contiguous
 * parameter RAM slots that have been allocated using the edma_alloc_cont_slots
 * API.
 * Callers are responsible for ensuring the slots are inactive, and will
 * not be activated.
 */
int edma_free_cont_slots(unsigned slot, int count)
{
	unsigned ctlr, slot_to_free;
	int i;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_channels ||
		slot >= edma_cc[ctlr]->num_slots ||
		count < 1)
		return -EINVAL;

	for (i = slot; i < slot + count; ++i) {
		ctlr = EDMA_CTLR(i);
		slot_to_free = EDMA_CHAN_SLOT(i);

		memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(slot_to_free),
			&dummy_paramset, PARM_SIZE);
		clear_bit(slot_to_free, edma_cc[ctlr]->edma_inuse);
	}

	return 0;
}
EXPORT_SYMBOL(edma_free_cont_slots);

/*-----------------------------------------------------------------------*/

/* Parameter RAM operations (i) -- read/write partial slots */

/**
 * edma_set_src - set initial DMA source address in parameter RAM slot
 * @slot: parameter RAM slot being configured
 * @src_port: physical address of source (memory, controller FIFO, etc)
 * @addressMode: INCR, except in very rare cases
 * @fifoWidth: ignored unless @addressMode is FIFO, else specifies the
 *	width to use when addressing the fifo (e.g. W8BIT, W32BIT)
 *
 * Note that the source address is modified during the DMA transfer
 * according to edma_set_src_index().
 */
void edma_set_src(unsigned slot, dma_addr_t src_port,
				enum address_mode mode, enum fifo_width width)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		unsigned int i = edma_parm_read(ctlr, PARM_OPT, slot);

		if (mode) {
			/* set SAM and program FWID */
			i = (i & ~(EDMA_FWID)) | (SAM | ((width & 0x7) << 8));
		} else {
			/* clear SAM */
			i &= ~SAM;
		}
		edma_parm_write(ctlr, PARM_OPT, slot, i);

		/* set the source port address
		   in source register of param structure */
		edma_parm_write(ctlr, PARM_SRC, slot, src_port);
	}
}
EXPORT_SYMBOL(edma_set_src);

/**
 * edma_set_dest - set initial DMA destination address in parameter RAM slot
 * @slot: parameter RAM slot being configured
 * @dest_port: physical address of destination (memory, controller FIFO, etc)
 * @addressMode: INCR, except in very rare cases
 * @fifoWidth: ignored unless @addressMode is FIFO, else specifies the
 *	width to use when addressing the fifo (e.g. W8BIT, W32BIT)
 *
 * Note that the destination address is modified during the DMA transfer
 * according to edma_set_dest_index().
 */
void edma_set_dest(unsigned slot, dma_addr_t dest_port,
				 enum address_mode mode, enum fifo_width width)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		unsigned int i = edma_parm_read(ctlr, PARM_OPT, slot);

		if (mode) {
			/* set DAM and program FWID */
			i = (i & ~(EDMA_FWID)) | (DAM | ((width & 0x7) << 8));
		} else {
			/* clear DAM */
			i &= ~DAM;
		}
		edma_parm_write(ctlr, PARM_OPT, slot, i);
		/* set the destination port address
		   in dest register of param structure */
		edma_parm_write(ctlr, PARM_DST, slot, dest_port);
	}
}
EXPORT_SYMBOL(edma_set_dest);

/**
 * edma_get_position - returns the current transfer point
 * @slot: parameter RAM slot being examined
 * @dst:  true selects the dest position, false the source
 *
 * Returns the position of the current active slot
 */
dma_addr_t edma_get_position(unsigned slot, bool dst)
{
	u32 offs, ctlr = EDMA_CTLR(slot);

	slot = EDMA_CHAN_SLOT(slot);

	offs = PARM_OFFSET(slot);
	offs += dst ? PARM_DST : PARM_SRC;

	return edma_read(ctlr, offs);
}

/**
 * edma_set_src_index - configure DMA source address indexing
 * @slot: parameter RAM slot being configured
 * @src_bidx: byte offset between source arrays in a frame
 * @src_cidx: byte offset between source frames in a block
 *
 * Offsets are specified to support either contiguous or discontiguous
 * memory transfers, or repeated access to a hardware register, as needed.
 * When accessing hardware registers, both offsets are normally zero.
 */
void edma_set_src_index(unsigned slot, s16 src_bidx, s16 src_cidx)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		edma_parm_modify(ctlr, PARM_SRC_DST_BIDX, slot,
				0xffff0000, src_bidx);
		edma_parm_modify(ctlr, PARM_SRC_DST_CIDX, slot,
				0xffff0000, src_cidx);
	}
}
EXPORT_SYMBOL(edma_set_src_index);

/**
 * edma_set_dest_index - configure DMA destination address indexing
 * @slot: parameter RAM slot being configured
 * @dest_bidx: byte offset between destination arrays in a frame
 * @dest_cidx: byte offset between destination frames in a block
 *
 * Offsets are specified to support either contiguous or discontiguous
 * memory transfers, or repeated access to a hardware register, as needed.
 * When accessing hardware registers, both offsets are normally zero.
 */
void edma_set_dest_index(unsigned slot, s16 dest_bidx, s16 dest_cidx)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		edma_parm_modify(ctlr, PARM_SRC_DST_BIDX, slot,
				0x0000ffff, dest_bidx << 16);
		edma_parm_modify(ctlr, PARM_SRC_DST_CIDX, slot,
				0x0000ffff, dest_cidx << 16);
	}
}
EXPORT_SYMBOL(edma_set_dest_index);

/**
 * edma_set_transfer_params - configure DMA transfer parameters
 * @slot: parameter RAM slot being configured
 * @acnt: how many bytes per array (at least one)
 * @bcnt: how many arrays per frame (at least one)
 * @ccnt: how many frames per block (at least one)
 * @bcnt_rld: used only for A-Synchronized transfers; this specifies
 *	the value to reload into bcnt when it decrements to zero
 * @sync_mode: ASYNC or ABSYNC
 *
 * See the EDMA3 documentation to understand how to configure and link
 * transfers using the fields in PaRAM slots.  If you are not doing it
 * all at once with edma_write_slot(), you will use this routine
 * plus two calls each for source and destination, setting the initial
 * address and saying how to index that address.
 *
 * An example of an A-Synchronized transfer is a serial link using a
 * single word shift register.  In that case, @acnt would be equal to
 * that word size; the serial controller issues a DMA synchronization
 * event to transfer each word, and memory access by the DMA transfer
 * controller will be word-at-a-time.
 *
 * An example of an AB-Synchronized transfer is a device using a FIFO.
 * In that case, @acnt equals the FIFO width and @bcnt equals its depth.
 * The controller with the FIFO issues DMA synchronization events when
 * the FIFO threshold is reached, and the DMA transfer controller will
 * transfer one frame to (or from) the FIFO.  It will probably use
 * efficient burst modes to access memory.
 */
void edma_set_transfer_params(unsigned slot,
		u16 acnt, u16 bcnt, u16 ccnt,
		u16 bcnt_rld, enum sync_dimension sync_mode)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		edma_parm_modify(ctlr, PARM_LINK_BCNTRLD, slot,
				0x0000ffff, bcnt_rld << 16);
		if (sync_mode == ASYNC)
			edma_parm_and(ctlr, PARM_OPT, slot, ~SYNCDIM);
		else
			edma_parm_or(ctlr, PARM_OPT, slot, SYNCDIM);
		/* Set the acount, bcount, ccount registers */
		edma_parm_write(ctlr, PARM_A_B_CNT, slot, (bcnt << 16) | acnt);
		edma_parm_write(ctlr, PARM_CCNT, slot, ccnt);
	}
}
EXPORT_SYMBOL(edma_set_transfer_params);

/**
 * edma_link - link one parameter RAM slot to another
 * @from: parameter RAM slot originating the link
 * @to: parameter RAM slot which is the link target
 *
 * The originating slot should not be part of any active DMA transfer.
 */
void edma_link(unsigned from, unsigned to)
{
	unsigned ctlr_from, ctlr_to;

	ctlr_from = EDMA_CTLR(from);
	from = EDMA_CHAN_SLOT(from);
	ctlr_to = EDMA_CTLR(to);
	to = EDMA_CHAN_SLOT(to);

	if (from >= edma_cc[ctlr_from]->num_slots)
		return;
	if (to >= edma_cc[ctlr_to]->num_slots)
		return;
	edma_parm_modify(ctlr_from, PARM_LINK_BCNTRLD, from, 0xffff0000,
				PARM_OFFSET(to));
}
EXPORT_SYMBOL(edma_link);

/**
 * edma_unlink - cut link from one parameter RAM slot
 * @from: parameter RAM slot originating the link
 *
 * The originating slot should not be part of any active DMA transfer.
 * Its link is set to 0xffff.
 */
void edma_unlink(unsigned from)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(from);
	from = EDMA_CHAN_SLOT(from);

	if (from >= edma_cc[ctlr]->num_slots)
		return;
	edma_parm_or(ctlr, PARM_LINK_BCNTRLD, from, 0xffff);
}
EXPORT_SYMBOL(edma_unlink);

/*-----------------------------------------------------------------------*/

/* Parameter RAM operations (ii) -- read/write whole parameter sets */

/**
 * edma_write_slot - write parameter RAM data for slot
 * @slot: number of parameter RAM slot being modified
 * @param: data to be written into parameter RAM slot
 *
 * Use this to assign all parameters of a transfer at once.  This
 * allows more efficient setup of transfers than issuing multiple
 * calls to set up those parameters in small pieces, and provides
 * complete control over all transfer options.
 */
void edma_write_slot(unsigned slot, const struct edmacc_param *param)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot >= edma_cc[ctlr]->num_slots)
		return;
	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(slot), param,
			PARM_SIZE);
}
EXPORT_SYMBOL(edma_write_slot);

/**
 * edma_read_slot - read parameter RAM data from slot
 * @slot: number of parameter RAM slot being copied
 * @param: where to store copy of parameter RAM data
 *
 * Use this to read data from a parameter RAM slot, perhaps to
 * save them as a template for later reuse.
 */
void edma_read_slot(unsigned slot, struct edmacc_param *param)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot >= edma_cc[ctlr]->num_slots)
		return;
	memcpy_fromio(param, edmacc_regs_base[ctlr] + PARM_OFFSET(slot),
			PARM_SIZE);
}
EXPORT_SYMBOL(edma_read_slot);

/*-----------------------------------------------------------------------*/

/* Various EDMA channel control operations */

/**
 * edma_pause - pause dma on a channel
 * @channel: on which edma_start() has been called
 *
 * This temporarily disables EDMA hardware events on the specified channel,
 * preventing them from triggering new transfers on its behalf
 */
void edma_pause(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		unsigned int mask = BIT(channel & 0x1f);

		edma_shadow0_write_array(ctlr, SH_EECR, channel >> 5, mask);
	}
}
EXPORT_SYMBOL(edma_pause);

/**
 * edma_resume - resumes dma on a paused channel
 * @channel: on which edma_pause() has been called
 *
 * This re-enables EDMA hardware events on the specified channel.
 */
void edma_resume(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		unsigned int mask = BIT(channel & 0x1f);

		edma_shadow0_write_array(ctlr, SH_EESR, channel >> 5, mask);
	}
}
EXPORT_SYMBOL(edma_resume);

int edma_trigger_channel(unsigned channel)
{
	unsigned ctlr;
	unsigned int mask;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);
	mask = BIT(channel & 0x1f);

	edma_shadow0_write_array(ctlr, SH_ESR, (channel >> 5), mask);

	pr_debug("EDMA: ESR%d %08x\n", (channel >> 5),
		 edma_shadow0_read_array(ctlr, SH_ESR, (channel >> 5)));
	return 0;
}
EXPORT_SYMBOL(edma_trigger_channel);

/**
 * edma_start - start dma on a channel
 * @channel: channel being activated
 *
 * Channels with event associations will be triggered by their hardware
 * events, and channels without such associations will be triggered by
 * software.  (At this writing there is no interface for using software
 * triggers except with channels that don't support hardware triggers.)
 *
 * Returns zero on success, else negative errno.
 */
int edma_start(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		int j = channel >> 5;
		unsigned int mask = BIT(channel & 0x1f);

		/* EDMA channels without event association */
		if (test_bit(channel, edma_cc[ctlr]->edma_unused)) {
			pr_debug("EDMA: ESR%d %08x\n", j,
				edma_shadow0_read_array(ctlr, SH_ESR, j));
			edma_shadow0_write_array(ctlr, SH_ESR, j, mask);
			return 0;
		}

		/* EDMA channel with event association */
		pr_debug("EDMA: ER%d %08x\n", j,
			edma_shadow0_read_array(ctlr, SH_ER, j));
		/* Clear any pending event or error */
		edma_write_array(ctlr, EDMA_ECR, j, mask);
		edma_write_array(ctlr, EDMA_EMCR, j, mask);
		/* Clear any SER */
		edma_shadow0_write_array(ctlr, SH_SECR, j, mask);
		edma_shadow0_write_array(ctlr, SH_EESR, j, mask);
		pr_debug("EDMA: EER%d %08x\n", j,
			edma_shadow0_read_array(ctlr, SH_EER, j));
		return 0;
	}

	return -EINVAL;
}
EXPORT_SYMBOL(edma_start);

/**
 * edma_stop - stops dma on the channel passed
 * @channel: channel being deactivated
 *
 * When @lch is a channel, any active transfer is paused and
 * all pending hardware events are cleared.  The current transfer
 * may not be resumed, and the channel's Parameter RAM should be
 * reinitialized before being reused.
 */
void edma_stop(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		int j = channel >> 5;
		unsigned int mask = BIT(channel & 0x1f);

		edma_shadow0_write_array(ctlr, SH_EECR, j, mask);
		edma_shadow0_write_array(ctlr, SH_ECR, j, mask);
		edma_shadow0_write_array(ctlr, SH_SECR, j, mask);
		edma_write_array(ctlr, EDMA_EMCR, j, mask);

		pr_debug("EDMA: EER%d %08x\n", j,
				edma_shadow0_read_array(ctlr, SH_EER, j));

		/* REVISIT:  consider guarding against inappropriate event
		 * chaining by overwriting with dummy_paramset.
		 */
	}
}
EXPORT_SYMBOL(edma_stop);

/******************************************************************************
 *
 * It cleans ParamEntry qand bring back EDMA to initial state if media has
 * been removed before EDMA has finished.It is usedful for removable media.
 * Arguments:
 *      ch_no     - channel no
 *
 * Return: zero on success, or corresponding error no on failure
 *
 * FIXME this should not be needed ... edma_stop() should suffice.
 *
 *****************************************************************************/

void edma_clean_channel(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		int j = (channel >> 5);
		unsigned int mask = BIT(channel & 0x1f);

		pr_debug("EDMA: EMR%d %08x\n", j,
				edma_read_array(ctlr, EDMA_EMR, j));
		edma_shadow0_write_array(ctlr, SH_ECR, j, mask);
		/* Clear the corresponding EMR bits */
		edma_write_array(ctlr, EDMA_EMCR, j, mask);
		/* Clear any SER */
		edma_shadow0_write_array(ctlr, SH_SECR, j, mask);
		edma_write(ctlr, EDMA_CCERRCLR, BIT(16) | BIT(1) | BIT(0));
	}
}
EXPORT_SYMBOL(edma_clean_channel);

/*
 * edma_clear_event - clear an outstanding event on the DMA channel
 * Arguments:
 *	channel - channel number
 */
void edma_clear_event(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel >= edma_cc[ctlr]->num_channels)
		return;
	if (channel < 32)
		edma_write(ctlr, EDMA_ECR, BIT(channel));
	else
		edma_write(ctlr, EDMA_ECRH, BIT(channel - 32));
}
EXPORT_SYMBOL(edma_clear_event);

/*
 * edma_assign_channel_eventq - move given channel to desired eventq
 * Arguments:
 *	channel - channel number
 *	eventq_no - queue to move the channel
 *
 * Can be used to move a channel to a selected event queue.
 */
void edma_assign_channel_eventq(unsigned channel, enum dma_event_q eventq_no)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel >= edma_cc[ctlr]->num_channels)
		return;

	/* default to low priority queue */
	if (eventq_no == EVENTQ_DEFAULT)
		eventq_no = edma_cc[ctlr]->default_queue;
	if (eventq_no >= edma_cc[ctlr]->num_tc)
		return;

	map_dmach_queue(ctlr, channel, eventq_no);
}
EXPORT_SYMBOL(edma_assign_channel_eventq);

static int edma_setup_from_hw(struct device *dev, struct edma_soc_info *pdata,
			      struct edma *edma_cc, int cc_id)
{
	int i;
	u32 value, cccfg;
	s8 (*queue_priority_map)[2];

	/* Decode the eDMA3 configuration from CCCFG register */
	cccfg = edma_read(cc_id, EDMA_CCCFG);

	value = GET_NUM_REGN(cccfg);
	edma_cc->num_region = BIT(value);

	value = GET_NUM_DMACH(cccfg);
	edma_cc->num_channels = BIT(value + 1);

	value = GET_NUM_PAENTRY(cccfg);
	edma_cc->num_slots = BIT(value + 4);

	value = GET_NUM_EVQUE(cccfg);
	edma_cc->num_tc = value + 1;

	dev_dbg(dev, "eDMA3 CC%d HW configuration (cccfg: 0x%08x):\n", cc_id,
		cccfg);
	dev_dbg(dev, "num_region: %u\n", edma_cc->num_region);
	dev_dbg(dev, "num_channel: %u\n", edma_cc->num_channels);
	dev_dbg(dev, "num_slot: %u\n", edma_cc->num_slots);
	dev_dbg(dev, "num_tc: %u\n", edma_cc->num_tc);

	/* Nothing need to be done if queue priority is provided */
	if (pdata->queue_priority_mapping)
		return 0;

	/*
	 * Configure TC/queue priority as follows:
	 * Q0 - priority 0
	 * Q1 - priority 1
	 * Q2 - priority 2
	 * ...
	 * The meaning of priority numbers: 0 highest priority, 7 lowest
	 * priority. So Q0 is the highest priority queue and the last queue has
	 * the lowest priority.
	 */
	queue_priority_map = devm_kzalloc(dev,
					  (edma_cc->num_tc + 1) * sizeof(s8),
					  GFP_KERNEL);
	if (!queue_priority_map)
		return -ENOMEM;

	for (i = 0; i < edma_cc->num_tc; i++) {
		queue_priority_map[i][0] = i;
		queue_priority_map[i][1] = i;
	}
	queue_priority_map[i][0] = -1;
	queue_priority_map[i][1] = -1;

	pdata->queue_priority_mapping = queue_priority_map;
	/* Default queue has the lowest priority */
	pdata->default_queue = i - 1;

	return 0;
}

#if IS_ENABLED(CONFIG_OF) && IS_ENABLED(CONFIG_DMADEVICES)

static int edma_xbar_event_map(struct device *dev, struct device_node *node,
			       struct edma_soc_info *pdata, size_t sz)
{
	const char pname[] = "ti,edma-xbar-event-map";
	struct resource res;
	void __iomem *xbar;
	s16 (*xbar_chans)[2];
	size_t nelm = sz / sizeof(s16);
	u32 shift, offset, mux;
	int ret, i;

	xbar_chans = devm_kzalloc(dev, (nelm + 2) * sizeof(s16), GFP_KERNEL);
	if (!xbar_chans)
		return -ENOMEM;

	ret = of_address_to_resource(node, 1, &res);
	if (ret)
		return -ENOMEM;

	xbar = devm_ioremap(dev, res.start, resource_size(&res));
	if (!xbar)
		return -ENOMEM;

	ret = of_property_read_u16_array(node, pname, (u16 *)xbar_chans, nelm);
	if (ret)
		return -EIO;

	/* Invalidate last entry for the other user of this mess */
	nelm >>= 1;
	xbar_chans[nelm][0] = xbar_chans[nelm][1] = -1;

	for (i = 0; i < nelm; i++) {
		shift = (xbar_chans[i][1] & 0x03) << 3;
		offset = xbar_chans[i][1] & 0xfffffffc;
		mux = readl(xbar + offset);
		mux &= ~(0xff << shift);
		mux |= xbar_chans[i][0] << shift;
		writel(mux, (xbar + offset));
	}

	pdata->xbar_chans = (const s16 (*)[2]) xbar_chans;
	return 0;
}

static int edma_of_parse_dt(struct device *dev,
			    struct device_node *node,
			    struct edma_soc_info *pdata)
{
	int ret = 0;
	struct property *prop;
	size_t sz;
	struct edma_rsv_info *rsv_info;

	rsv_info = devm_kzalloc(dev, sizeof(struct edma_rsv_info), GFP_KERNEL);
	if (!rsv_info)
		return -ENOMEM;
	pdata->rsv = rsv_info;

	prop = of_find_property(node, "ti,edma-xbar-event-map", &sz);
	if (prop)
		ret = edma_xbar_event_map(dev, node, pdata, sz);

	return ret;
}

static struct of_dma_filter_info edma_filter_info = {
	.filter_fn = edma_filter_fn,
};

static struct edma_soc_info *edma_setup_info_from_dt(struct device *dev,
						      struct device_node *node)
{
	struct edma_soc_info *info;
	int ret;

	info = devm_kzalloc(dev, sizeof(struct edma_soc_info), GFP_KERNEL);
	if (!info)
		return ERR_PTR(-ENOMEM);

	ret = edma_of_parse_dt(dev, node, info);
	if (ret)
		return ERR_PTR(ret);

	dma_cap_set(DMA_SLAVE, edma_filter_info.dma_cap);
	dma_cap_set(DMA_CYCLIC, edma_filter_info.dma_cap);
	of_dma_controller_register(dev->of_node, of_dma_simple_xlate,
				   &edma_filter_info);

	return info;
}
#else
static struct edma_soc_info *edma_setup_info_from_dt(struct device *dev,
						      struct device_node *node)
{
	return ERR_PTR(-ENOSYS);
}
#endif

static int edma_probe(struct platform_device *pdev)
{
	struct edma_soc_info	**info = pdev->dev.platform_data;
	struct edma_soc_info    *ninfo[EDMA_MAX_CC] = {NULL};
	s8		(*queue_priority_mapping)[2];
	int			i, j, off, ln, found = 0;
	int			status = -1;
	const s16		(*rsv_chans)[2];
	const s16		(*rsv_slots)[2];
	const s16		(*xbar_chans)[2];
	int			irq[EDMA_MAX_CC] = {0, 0};
	int			err_irq[EDMA_MAX_CC] = {0, 0};
	struct resource		*r[EDMA_MAX_CC] = {NULL};
	struct resource		res[EDMA_MAX_CC];
	char			res_name[10];
	struct device_node	*node = pdev->dev.of_node;
	struct device		*dev = &pdev->dev;
	int			ret;
	struct platform_device_info edma_dev_info = {
		.name = "edma-dma-engine",
		.dma_mask = DMA_BIT_MASK(32),
		.parent = &pdev->dev,
	};

	if (node) {
		/* Check if this is a second instance registered */
		if (arch_num_cc) {
			dev_err(dev, "only one EDMA instance is supported via DT\n");
			return -ENODEV;
		}

		ninfo[0] = edma_setup_info_from_dt(dev, node);
		if (IS_ERR(ninfo[0])) {
			dev_err(dev, "failed to get DT data\n");
			return PTR_ERR(ninfo[0]);
		}

		info = ninfo;
	}

	if (!info)
		return -ENODEV;

	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		dev_err(dev, "pm_runtime_get_sync() failed\n");
		return ret;
	}

	for (j = 0; j < EDMA_MAX_CC; j++) {
		if (!info[j]) {
			if (!found)
				return -ENODEV;
			break;
		}
		if (node) {
			ret = of_address_to_resource(node, j, &res[j]);
			if (!ret)
				r[j] = &res[j];
		} else {
			sprintf(res_name, "edma_cc%d", j);
			r[j] = platform_get_resource_byname(pdev,
						IORESOURCE_MEM,
						res_name);
		}
		if (!r[j]) {
			if (found)
				break;
			else
				return -ENODEV;
		} else {
			found = 1;
		}

		edmacc_regs_base[j] = devm_ioremap_resource(&pdev->dev, r[j]);
		if (IS_ERR(edmacc_regs_base[j]))
			return PTR_ERR(edmacc_regs_base[j]);

		edma_cc[j] = devm_kzalloc(&pdev->dev, sizeof(struct edma),
					  GFP_KERNEL);
		if (!edma_cc[j])
			return -ENOMEM;

		/* Get eDMA3 configuration from IP */
		ret = edma_setup_from_hw(dev, info[j], edma_cc[j], j);
		if (ret)
			return ret;

		edma_cc[j]->default_queue = info[j]->default_queue;

		dev_dbg(&pdev->dev, "DMA REG BASE ADDR=%p\n",
			edmacc_regs_base[j]);

		for (i = 0; i < edma_cc[j]->num_slots; i++)
			memcpy_toio(edmacc_regs_base[j] + PARM_OFFSET(i),
					&dummy_paramset, PARM_SIZE);

		/* Mark all channels as unused */
		memset(edma_cc[j]->edma_unused, 0xff,
			sizeof(edma_cc[j]->edma_unused));

		if (info[j]->rsv) {

			/* Clear the reserved channels in unused list */
			rsv_chans = info[j]->rsv->rsv_chans;
			if (rsv_chans) {
				for (i = 0; rsv_chans[i][0] != -1; i++) {
					off = rsv_chans[i][0];
					ln = rsv_chans[i][1];
					clear_bits(off, ln,
						  edma_cc[j]->edma_unused);
				}
			}

			/* Set the reserved slots in inuse list */
			rsv_slots = info[j]->rsv->rsv_slots;
			if (rsv_slots) {
				for (i = 0; rsv_slots[i][0] != -1; i++) {
					off = rsv_slots[i][0];
					ln = rsv_slots[i][1];
					set_bits(off, ln,
						edma_cc[j]->edma_inuse);
				}
			}
		}

		/* Clear the xbar mapped channels in unused list */
		xbar_chans = info[j]->xbar_chans;
		if (xbar_chans) {
			for (i = 0; xbar_chans[i][1] != -1; i++) {
				off = xbar_chans[i][1];
				clear_bits(off, 1,
					   edma_cc[j]->edma_unused);
			}
		}

		if (node) {
			irq[j] = irq_of_parse_and_map(node, 0);
			err_irq[j] = irq_of_parse_and_map(node, 2);
		} else {
			char irq_name[10];

			sprintf(irq_name, "edma%d", j);
			irq[j] = platform_get_irq_byname(pdev, irq_name);

			sprintf(irq_name, "edma%d_err", j);
			err_irq[j] = platform_get_irq_byname(pdev, irq_name);
		}
		edma_cc[j]->irq_res_start = irq[j];
		edma_cc[j]->irq_res_end = err_irq[j];

		status = devm_request_irq(dev, irq[j], dma_irq_handler, 0,
					  "edma", dev);
		if (status < 0) {
			dev_dbg(&pdev->dev,
				"devm_request_irq %d failed --> %d\n",
				irq[j], status);
			return status;
		}

		status = devm_request_irq(dev, err_irq[j], dma_ccerr_handler, 0,
					  "edma_error", dev);
		if (status < 0) {
			dev_dbg(&pdev->dev,
				"devm_request_irq %d failed --> %d\n",
				err_irq[j], status);
			return status;
		}

		for (i = 0; i < edma_cc[j]->num_channels; i++)
			map_dmach_queue(j, i, info[j]->default_queue);

		queue_priority_mapping = info[j]->queue_priority_mapping;

		/* Event queue priority mapping */
		for (i = 0; queue_priority_mapping[i][0] != -1; i++)
			assign_priority_to_queue(j,
						queue_priority_mapping[i][0],
						queue_priority_mapping[i][1]);

		/* Map the channel to param entry if channel mapping logic
		 * exist
		 */
		if (edma_read(j, EDMA_CCCFG) & CHMAP_EXIST)
			map_dmach_param(j);

		for (i = 0; i < edma_cc[j]->num_region; i++) {
			edma_write_array2(j, EDMA_DRAE, i, 0, 0x0);
			edma_write_array2(j, EDMA_DRAE, i, 1, 0x0);
			edma_write_array(j, EDMA_QRAE, i, 0x0);
		}
		edma_cc[j]->info = info[j];
		arch_num_cc++;

		edma_dev_info.id = j;
		platform_device_register_full(&edma_dev_info);
	}

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int edma_pm_resume(struct device *dev)
{
	int i, j;

	for (j = 0; j < arch_num_cc; j++) {
		struct edma *cc = edma_cc[j];

		s8 (*queue_priority_mapping)[2];

		queue_priority_mapping = cc->info->queue_priority_mapping;

		/* Event queue priority mapping */
		for (i = 0; queue_priority_mapping[i][0] != -1; i++)
			assign_priority_to_queue(j,
						 queue_priority_mapping[i][0],
						 queue_priority_mapping[i][1]);

		/*
		 * Map the channel to param entry if channel mapping logic
		 * exist
		 */
		if (edma_read(j, EDMA_CCCFG) & CHMAP_EXIST)
			map_dmach_param(j);

		for (i = 0; i < cc->num_channels; i++) {
			if (test_bit(i, cc->edma_inuse)) {
				/* ensure access through shadow region 0 */
				edma_or_array2(j, EDMA_DRAE, 0, i >> 5,
					       BIT(i & 0x1f));

				setup_dma_interrupt(i,
						    cc->intr_data[i].callback,
						    cc->intr_data[i].data);
			}
		}
	}

	return 0;
}
#endif

static const struct dev_pm_ops edma_pm_ops = {
	SET_LATE_SYSTEM_SLEEP_PM_OPS(NULL, edma_pm_resume)
};

static struct platform_driver edma_driver = {
	.driver = {
		.name	= "edma",
		.pm	= &edma_pm_ops,
		.of_match_table = edma_of_ids,
	},
	.probe = edma_probe,
};

static int __init edma_init(void)
{
	return platform_driver_probe(&edma_driver, edma_probe);
}
arch_initcall(edma_init);

