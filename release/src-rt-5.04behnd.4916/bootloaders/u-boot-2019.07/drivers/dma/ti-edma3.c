// SPDX-License-Identifier: GPL-2.0+
/*
 * Enhanced Direct Memory Access (EDMA3) Controller
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * Author: Ivan Khoronzhuk <ivan.khoronzhuk@ti.com>
 */

#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <dma-uclass.h>
#include <asm/omap_common.h>
#include <asm/ti-common/ti-edma3.h>

#define EDMA3_SL_BASE(slot)			(0x4000 + ((slot) << 5))
#define EDMA3_SL_MAX_NUM			512
#define EDMA3_SLOPT_FIFO_WIDTH_MASK		(0x7 << 8)

#define EDMA3_QCHMAP(ch)			0x0200 + ((ch) << 2)
#define EDMA3_CHMAP_PARSET_MASK			0x1ff
#define EDMA3_CHMAP_PARSET_SHIFT		0x5
#define EDMA3_CHMAP_TRIGWORD_SHIFT		0x2

#define EDMA3_QEMCR				0x314
#define EDMA3_IPR				0x1068
#define EDMA3_IPRH				0x106c
#define EDMA3_ICR				0x1070
#define EDMA3_ICRH				0x1074
#define EDMA3_QEECR				0x1088
#define EDMA3_QEESR				0x108c
#define EDMA3_QSECR				0x1094

#define EDMA_FILL_BUFFER_SIZE			512

struct ti_edma3_priv {
	u32 base;
};

static u8 edma_fill_buffer[EDMA_FILL_BUFFER_SIZE] __aligned(ARCH_DMA_MINALIGN);

/**
 * qedma3_start - start qdma on a channel
 * @base: base address of edma
 * @cfg: pinter to struct edma3_channel_config where you can set
 * the slot number to associate with, the chnum, which corresponds
 * your quick channel number 0-7, complete code - transfer complete code
 * and trigger slot word - which has to correspond to the word number in
 * edma3_slot_layout struct for generating event.
 *
 */
void qedma3_start(u32 base, struct edma3_channel_config *cfg)
{
	u32 qchmap;

	/* Clear the pending int bit */
	if (cfg->complete_code < 32)
		__raw_writel(1 << cfg->complete_code, base + EDMA3_ICR);
	else
		__raw_writel(1 << cfg->complete_code, base + EDMA3_ICRH);

	/* Map parameter set and trigger word 7 to quick channel */
	qchmap = ((EDMA3_CHMAP_PARSET_MASK & cfg->slot)
		  << EDMA3_CHMAP_PARSET_SHIFT) |
		  (cfg->trigger_slot_word << EDMA3_CHMAP_TRIGWORD_SHIFT);

	__raw_writel(qchmap, base + EDMA3_QCHMAP(cfg->chnum));

	/* Clear missed event if set*/
	__raw_writel(1 << cfg->chnum, base + EDMA3_QSECR);
	__raw_writel(1 << cfg->chnum, base + EDMA3_QEMCR);

	/* Enable qdma channel event */
	__raw_writel(1 << cfg->chnum, base + EDMA3_QEESR);
}

/**
 * edma3_set_dest - set initial DMA destination address in parameter RAM slot
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @dst: physical address of destination (memory, controller FIFO, etc)
 * @addressMode: INCR, except in very rare cases
 * @width: ignored unless @addressMode is FIFO, else specifies the
 *	width to use when addressing the fifo (e.g. W8BIT, W32BIT)
 *
 * Note that the destination address is modified during the DMA transfer
 * according to edma3_set_dest_index().
 */
void edma3_set_dest(u32 base, int slot, u32 dst, enum edma3_address_mode mode,
		    enum edma3_fifo_width width)
{
	u32 opt;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	opt = __raw_readl(&rg->opt);
	if (mode == FIFO)
		opt = (opt & EDMA3_SLOPT_FIFO_WIDTH_MASK) |
		       (EDMA3_SLOPT_DST_ADDR_CONST_MODE |
			EDMA3_SLOPT_FIFO_WIDTH_SET(width));
	else
		opt &= ~EDMA3_SLOPT_DST_ADDR_CONST_MODE;

	__raw_writel(opt, &rg->opt);
	__raw_writel(dst, &rg->dst);
}

/**
 * edma3_set_dest_index - configure DMA destination address indexing
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @bidx: byte offset between destination arrays in a frame
 * @cidx: byte offset between destination frames in a block
 *
 * Offsets are specified to support either contiguous or discontiguous
 * memory transfers, or repeated access to a hardware register, as needed.
 * When accessing hardware registers, both offsets are normally zero.
 */
void edma3_set_dest_index(u32 base, unsigned slot, int bidx, int cidx)
{
	u32 src_dst_bidx;
	u32 src_dst_cidx;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	src_dst_bidx = __raw_readl(&rg->src_dst_bidx);
	src_dst_cidx = __raw_readl(&rg->src_dst_cidx);

	__raw_writel((src_dst_bidx & 0x0000ffff) | (bidx << 16),
		     &rg->src_dst_bidx);
	__raw_writel((src_dst_cidx & 0x0000ffff) | (cidx << 16),
		     &rg->src_dst_cidx);
}

/**
 * edma3_set_dest_addr - set destination address for slot only
 */
void edma3_set_dest_addr(u32 base, int slot, u32 dst)
{
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));
	__raw_writel(dst, &rg->dst);
}

/**
 * edma3_set_src - set initial DMA source address in parameter RAM slot
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @src_port: physical address of source (memory, controller FIFO, etc)
 * @mode: INCR, except in very rare cases
 * @width: ignored unless @addressMode is FIFO, else specifies the
 *	width to use when addressing the fifo (e.g. W8BIT, W32BIT)
 *
 * Note that the source address is modified during the DMA transfer
 * according to edma3_set_src_index().
 */
void edma3_set_src(u32 base, int slot, u32 src, enum edma3_address_mode mode,
		   enum edma3_fifo_width width)
{
	u32 opt;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	opt = __raw_readl(&rg->opt);
	if (mode == FIFO)
		opt = (opt & EDMA3_SLOPT_FIFO_WIDTH_MASK) |
		       (EDMA3_SLOPT_DST_ADDR_CONST_MODE |
			EDMA3_SLOPT_FIFO_WIDTH_SET(width));
	else
		opt &= ~EDMA3_SLOPT_DST_ADDR_CONST_MODE;

	__raw_writel(opt, &rg->opt);
	__raw_writel(src, &rg->src);
}

/**
 * edma3_set_src_index - configure DMA source address indexing
 * @base: base address of edma
 * @slot: parameter RAM slot being configured
 * @bidx: byte offset between source arrays in a frame
 * @cidx: byte offset between source frames in a block
 *
 * Offsets are specified to support either contiguous or discontiguous
 * memory transfers, or repeated access to a hardware register, as needed.
 * When accessing hardware registers, both offsets are normally zero.
 */
void edma3_set_src_index(u32 base, unsigned slot, int bidx, int cidx)
{
	u32 src_dst_bidx;
	u32 src_dst_cidx;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	src_dst_bidx = __raw_readl(&rg->src_dst_bidx);
	src_dst_cidx = __raw_readl(&rg->src_dst_cidx);

	__raw_writel((src_dst_bidx & 0xffff0000) | bidx,
		     &rg->src_dst_bidx);
	__raw_writel((src_dst_cidx & 0xffff0000) | cidx,
		     &rg->src_dst_cidx);
}

/**
 * edma3_set_src_addr - set source address for slot only
 */
void edma3_set_src_addr(u32 base, int slot, u32 src)
{
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));
	__raw_writel(src, &rg->src);
}

/**
 * edma3_set_transfer_params - configure DMA transfer parameters
 * @base: base address of edma
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
 * all at once with edma3_write_slot(), you will use this routine
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
void edma3_set_transfer_params(u32 base, int slot, int acnt,
			       int bcnt, int ccnt, u16 bcnt_rld,
			       enum edma3_sync_dimension sync_mode)
{
	u32 opt;
	u32 link_bcntrld;
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	link_bcntrld = __raw_readl(&rg->link_bcntrld);

	__raw_writel((bcnt_rld << 16) | (0x0000ffff & link_bcntrld),
		     &rg->link_bcntrld);

	opt = __raw_readl(&rg->opt);
	if (sync_mode == ASYNC)
		__raw_writel(opt & ~EDMA3_SLOPT_AB_SYNC, &rg->opt);
	else
		__raw_writel(opt | EDMA3_SLOPT_AB_SYNC, &rg->opt);

	/* Set the acount, bcount, ccount registers */
	__raw_writel((bcnt << 16) | (acnt & 0xffff), &rg->a_b_cnt);
	__raw_writel(0xffff & ccnt, &rg->ccnt);
}

/**
 * edma3_write_slot - write parameter RAM data for slot
 * @base: base address of edma
 * @slot: number of parameter RAM slot being modified
 * @param: data to be written into parameter RAM slot
 *
 * Use this to assign all parameters of a transfer at once.  This
 * allows more efficient setup of transfers than issuing multiple
 * calls to set up those parameters in small pieces, and provides
 * complete control over all transfer options.
 */
void edma3_write_slot(u32 base, int slot, struct edma3_slot_layout *param)
{
	int i;
	u32 *p = (u32 *)param;
	u32 *addr = (u32 *)(base + EDMA3_SL_BASE(slot));

	for (i = 0; i < sizeof(struct edma3_slot_layout)/4; i += 4)
		__raw_writel(*p++, addr++);
}

/**
 * edma3_read_slot - read parameter RAM data from slot
 * @base: base address of edma
 * @slot: number of parameter RAM slot being copied
 * @param: where to store copy of parameter RAM data
 *
 * Use this to read data from a parameter RAM slot, perhaps to
 * save them as a template for later reuse.
 */
void edma3_read_slot(u32 base, int slot, struct edma3_slot_layout *param)
{
	int i;
	u32 *p = (u32 *)param;
	u32 *addr = (u32 *)(base + EDMA3_SL_BASE(slot));

	for (i = 0; i < sizeof(struct edma3_slot_layout)/4; i += 4)
		*p++ = __raw_readl(addr++);
}

void edma3_slot_configure(u32 base, int slot, struct edma3_slot_config *cfg)
{
	struct edma3_slot_layout *rg;

	rg = (struct edma3_slot_layout *)(base + EDMA3_SL_BASE(slot));

	__raw_writel(cfg->opt, &rg->opt);
	__raw_writel(cfg->src, &rg->src);
	__raw_writel((cfg->bcnt << 16) | (cfg->acnt & 0xffff), &rg->a_b_cnt);
	__raw_writel(cfg->dst, &rg->dst);
	__raw_writel((cfg->dst_bidx << 16) |
		     (cfg->src_bidx & 0xffff), &rg->src_dst_bidx);
	__raw_writel((cfg->bcntrld << 16) |
		     (cfg->link & 0xffff), &rg->link_bcntrld);
	__raw_writel((cfg->dst_cidx << 16) |
		     (cfg->src_cidx & 0xffff), &rg->src_dst_cidx);
	__raw_writel(0xffff & cfg->ccnt, &rg->ccnt);
}

/**
 * edma3_check_for_transfer - check if transfer coplete by checking
 * interrupt pending bit. Clear interrupt pending bit if complete.
 * @base: base address of edma
 * @cfg: pinter to struct edma3_channel_config which was passed
 * to qedma3_start when you started qdma channel
 *
 * Return 0 if complete, 1 if not.
 */
int edma3_check_for_transfer(u32 base, struct edma3_channel_config *cfg)
{
	u32 inum;
	u32 ipr_base;
	u32 icr_base;

	if (cfg->complete_code < 32) {
		ipr_base = base + EDMA3_IPR;
		icr_base = base + EDMA3_ICR;
		inum = 1 << cfg->complete_code;
	} else {
		ipr_base = base + EDMA3_IPRH;
		icr_base = base + EDMA3_ICRH;
		inum = 1 << (cfg->complete_code - 32);
	}

	/* check complete interrupt */
	if (!(__raw_readl(ipr_base) & inum))
		return 1;

	/* clean up the pending int bit */
	__raw_writel(inum, icr_base);

	return 0;
}

/**
 * qedma3_stop - stops dma on the channel passed
 * @base: base address of edma
 * @cfg: pinter to struct edma3_channel_config which was passed
 * to qedma3_start when you started qdma channel
 */
void qedma3_stop(u32 base, struct edma3_channel_config *cfg)
{
	/* Disable qdma channel event */
	__raw_writel(1 << cfg->chnum, base + EDMA3_QEECR);

	/* clean up the interrupt indication */
	if (cfg->complete_code < 32)
		__raw_writel(1 << cfg->complete_code, base + EDMA3_ICR);
	else
		__raw_writel(1 << cfg->complete_code, base + EDMA3_ICRH);

	/* Clear missed event if set*/
	__raw_writel(1 << cfg->chnum, base + EDMA3_QSECR);
	__raw_writel(1 << cfg->chnum, base + EDMA3_QEMCR);

	/* Clear the channel map */
	__raw_writel(0, base + EDMA3_QCHMAP(cfg->chnum));
}

void __edma3_transfer(unsigned long edma3_base_addr, unsigned int edma_slot_num,
		      void *dst, void *src, size_t len, size_t s_len)
{
	struct edma3_slot_config        slot;
	struct edma3_channel_config     edma_channel;
	int                             b_cnt_value = 1;
	int                             rem_bytes  = 0;
	int                             a_cnt_value = len;
	unsigned int                    addr = (unsigned int) (dst);
	unsigned int                    max_acnt  = 0x7FFFU;

	if (len > s_len) {
		b_cnt_value = (len / s_len);
		rem_bytes = (len % s_len);
		a_cnt_value = s_len;
	} else if (len > max_acnt) {
		b_cnt_value = (len / max_acnt);
		rem_bytes  = (len % max_acnt);
		a_cnt_value = max_acnt;
	}

	slot.opt        = 0;
	slot.src        = ((unsigned int) src);
	slot.acnt       = a_cnt_value;
	slot.bcnt       = b_cnt_value;
	slot.ccnt       = 1;
	if (len == s_len)
		slot.src_bidx = a_cnt_value;
	else
		slot.src_bidx = 0;
	slot.dst_bidx   = a_cnt_value;
	slot.src_cidx   = 0;
	slot.dst_cidx   = 0;
	slot.link       = EDMA3_PARSET_NULL_LINK;
	slot.bcntrld    = 0;
	slot.opt        = EDMA3_SLOPT_TRANS_COMP_INT_ENB |
			  EDMA3_SLOPT_COMP_CODE(0) |
			  EDMA3_SLOPT_STATIC | EDMA3_SLOPT_AB_SYNC;

	edma3_slot_configure(edma3_base_addr, edma_slot_num, &slot);
	edma_channel.slot = edma_slot_num;
	edma_channel.chnum = 0;
	edma_channel.complete_code = 0;
	 /* set event trigger to dst update */
	edma_channel.trigger_slot_word = EDMA3_TWORD(dst);

	qedma3_start(edma3_base_addr, &edma_channel);
	edma3_set_dest_addr(edma3_base_addr, edma_channel.slot, addr);

	while (edma3_check_for_transfer(edma3_base_addr, &edma_channel))
		;
	qedma3_stop(edma3_base_addr, &edma_channel);

	if (rem_bytes != 0) {
		slot.opt        = 0;
		if (len == s_len)
			slot.src =
				(b_cnt_value * max_acnt) + ((unsigned int) src);
		else
			slot.src = (unsigned int) src;
		slot.acnt       = rem_bytes;
		slot.bcnt       = 1;
		slot.ccnt       = 1;
		slot.src_bidx   = rem_bytes;
		slot.dst_bidx   = rem_bytes;
		slot.src_cidx   = 0;
		slot.dst_cidx   = 0;
		slot.link       = EDMA3_PARSET_NULL_LINK;
		slot.bcntrld    = 0;
		slot.opt        = EDMA3_SLOPT_TRANS_COMP_INT_ENB |
				  EDMA3_SLOPT_COMP_CODE(0) |
				  EDMA3_SLOPT_STATIC | EDMA3_SLOPT_AB_SYNC;
		edma3_slot_configure(edma3_base_addr, edma_slot_num, &slot);
		edma_channel.slot = edma_slot_num;
		edma_channel.chnum = 0;
		edma_channel.complete_code = 0;
		/* set event trigger to dst update */
		edma_channel.trigger_slot_word = EDMA3_TWORD(dst);

		qedma3_start(edma3_base_addr, &edma_channel);
		edma3_set_dest_addr(edma3_base_addr, edma_channel.slot, addr +
				    (max_acnt * b_cnt_value));
		while (edma3_check_for_transfer(edma3_base_addr, &edma_channel))
			;
		qedma3_stop(edma3_base_addr, &edma_channel);
	}
}

void __edma3_fill(unsigned long edma3_base_addr, unsigned int edma_slot_num,
		  void *dst, u8 val, size_t len)
{
	int xfer_len;
	int max_xfer = EDMA_FILL_BUFFER_SIZE * 65535;

	memset((void *)edma_fill_buffer, val, sizeof(edma_fill_buffer));

	while (len) {
		xfer_len = len;
		if (xfer_len > max_xfer)
			xfer_len = max_xfer;

		__edma3_transfer(edma3_base_addr, edma_slot_num, dst,
				 edma_fill_buffer, xfer_len,
				 EDMA_FILL_BUFFER_SIZE);
		len -= xfer_len;
		dst += xfer_len;
	}
}

#ifndef CONFIG_DMA

void edma3_transfer(unsigned long edma3_base_addr, unsigned int edma_slot_num,
		    void *dst, void *src, size_t len)
{
	__edma3_transfer(edma3_base_addr, edma_slot_num, dst, src, len, len);
}

void edma3_fill(unsigned long edma3_base_addr, unsigned int edma_slot_num,
		void *dst, u8 val, size_t len)
{
	__edma3_fill(edma3_base_addr, edma_slot_num, dst, val, len);
}

#else

static int ti_edma3_transfer(struct udevice *dev, int direction, void *dst,
			     void *src, size_t len)
{
	struct ti_edma3_priv *priv = dev_get_priv(dev);

	/* enable edma3 clocks */
	enable_edma3_clocks();

	switch (direction) {
	case DMA_MEM_TO_MEM:
		__edma3_transfer(priv->base, 1, dst, src, len, len);
		break;
	default:
		pr_err("Transfer type not implemented in DMA driver\n");
		break;
	}

	/* disable edma3 clocks */
	disable_edma3_clocks();

	return 0;
}

static int ti_edma3_ofdata_to_platdata(struct udevice *dev)
{
	struct ti_edma3_priv *priv = dev_get_priv(dev);

	priv->base = devfdt_get_addr(dev);

	return 0;
}

static int ti_edma3_probe(struct udevice *dev)
{
	struct dma_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->supported = DMA_SUPPORTS_MEM_TO_MEM;

	return 0;
}

static const struct dma_ops ti_edma3_ops = {
	.transfer	= ti_edma3_transfer,
};

static const struct udevice_id ti_edma3_ids[] = {
	{ .compatible = "ti,edma3" },
	{ }
};

U_BOOT_DRIVER(ti_edma3) = {
	.name	= "ti_edma3",
	.id	= UCLASS_DMA,
	.of_match = ti_edma3_ids,
	.ops	= &ti_edma3_ops,
	.ofdata_to_platdata = ti_edma3_ofdata_to_platdata,
	.probe	= ti_edma3_probe,
	.priv_auto_alloc_size = sizeof(struct ti_edma3_priv),
};
#endif /* CONFIG_DMA */
