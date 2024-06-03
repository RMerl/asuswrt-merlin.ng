// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com
 *  Author: Peter Ujfalusi <peter.ujfalusi@ti.com>
 */
#define pr_fmt(fmt) "udma: " fmt

#include <common.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <malloc.h>
#include <asm/dma-mapping.h>
#include <dm.h>
#include <dm/read.h>
#include <dm/of_access.h>
#include <dma.h>
#include <dma-uclass.h>
#include <linux/delay.h>
#include <dt-bindings/dma/k3-udma.h>
#include <linux/soc/ti/k3-navss-ringacc.h>
#include <linux/soc/ti/cppi5.h>
#include <linux/soc/ti/ti-udma.h>
#include <linux/soc/ti/ti_sci_protocol.h>

#include "k3-udma-hwdef.h"

#if BITS_PER_LONG == 64
#define RINGACC_RING_USE_PROXY	(0)
#else
#define RINGACC_RING_USE_PROXY	(1)
#endif

struct udma_chan;

enum udma_mmr {
	MMR_GCFG = 0,
	MMR_RCHANRT,
	MMR_TCHANRT,
	MMR_LAST,
};

static const char * const mmr_names[] = {
	"gcfg", "rchanrt", "tchanrt"
};

struct udma_tchan {
	void __iomem *reg_rt;

	int id;
	struct k3_nav_ring *t_ring; /* Transmit ring */
	struct k3_nav_ring *tc_ring; /* Transmit Completion ring */
};

struct udma_rchan {
	void __iomem *reg_rt;

	int id;
	struct k3_nav_ring *fd_ring; /* Free Descriptor ring */
	struct k3_nav_ring *r_ring; /* Receive ring*/
};

struct udma_rflow {
	int id;
};

struct udma_dev {
	struct device *dev;
	void __iomem *mmrs[MMR_LAST];

	struct k3_nav_ringacc *ringacc;

	u32 features;

	int tchan_cnt;
	int echan_cnt;
	int rchan_cnt;
	int rflow_cnt;
	unsigned long *tchan_map;
	unsigned long *rchan_map;
	unsigned long *rflow_map;

	struct udma_tchan *tchans;
	struct udma_rchan *rchans;
	struct udma_rflow *rflows;

	struct udma_chan *channels;
	u32 psil_base;

	u32 ch_count;
	const struct ti_sci_handle *tisci;
	const struct ti_sci_rm_udmap_ops *tisci_udmap_ops;
	const struct ti_sci_rm_psil_ops *tisci_psil_ops;
	u32  tisci_dev_id;
	u32  tisci_navss_dev_id;
	bool is_coherent;
};

struct udma_chan {
	struct udma_dev *ud;
	char name[20];

	struct udma_tchan *tchan;
	struct udma_rchan *rchan;
	struct udma_rflow *rflow;

	u32 bcnt; /* number of bytes completed since the start of the channel */

	bool pkt_mode; /* TR or packet */
	bool needs_epib; /* EPIB is needed for the communication or not */
	u32 psd_size; /* size of Protocol Specific Data */
	u32 metadata_size; /* (needs_epib ? 16:0) + psd_size */
	int slave_thread_id;
	u32 src_thread;
	u32 dst_thread;
	u32 static_tr_type;

	u32 id;
	enum dma_direction dir;

	struct cppi5_host_desc_t *desc_tx;
	u32 hdesc_size;
	bool in_use;
	void	*desc_rx;
	u32	num_rx_bufs;
	u32	desc_rx_cur;

};

#define UDMA_CH_1000(ch)		(ch * 0x1000)
#define UDMA_CH_100(ch)			(ch * 0x100)
#define UDMA_CH_40(ch)			(ch * 0x40)

#ifdef PKTBUFSRX
#define UDMA_RX_DESC_NUM PKTBUFSRX
#else
#define UDMA_RX_DESC_NUM 4
#endif

/* Generic register access functions */
static inline u32 udma_read(void __iomem *base, int reg)
{
	u32 v;

	v = __raw_readl(base + reg);
	pr_debug("READL(32): v(%08X)<--reg(%p)\n", v, base + reg);
	return v;
}

static inline void udma_write(void __iomem *base, int reg, u32 val)
{
	pr_debug("WRITEL(32): v(%08X)-->reg(%p)\n", val, base + reg);
	__raw_writel(val, base + reg);
}

static inline void udma_update_bits(void __iomem *base, int reg,
				    u32 mask, u32 val)
{
	u32 tmp, orig;

	orig = udma_read(base, reg);
	tmp = orig & ~mask;
	tmp |= (val & mask);

	if (tmp != orig)
		udma_write(base, reg, tmp);
}

/* TCHANRT */
static inline u32 udma_tchanrt_read(struct udma_tchan *tchan, int reg)
{
	if (!tchan)
		return 0;
	return udma_read(tchan->reg_rt, reg);
}

static inline void udma_tchanrt_write(struct udma_tchan *tchan,
				      int reg, u32 val)
{
	if (!tchan)
		return;
	udma_write(tchan->reg_rt, reg, val);
}

/* RCHANRT */
static inline u32 udma_rchanrt_read(struct udma_rchan *rchan, int reg)
{
	if (!rchan)
		return 0;
	return udma_read(rchan->reg_rt, reg);
}

static inline void udma_rchanrt_write(struct udma_rchan *rchan,
				      int reg, u32 val)
{
	if (!rchan)
		return;
	udma_write(rchan->reg_rt, reg, val);
}

static inline int udma_navss_psil_pair(struct udma_dev *ud, u32 src_thread,
				       u32 dst_thread)
{
	dst_thread |= UDMA_PSIL_DST_THREAD_ID_OFFSET;
	return ud->tisci_psil_ops->pair(ud->tisci,
					ud->tisci_navss_dev_id,
					src_thread, dst_thread);
}

static inline int udma_navss_psil_unpair(struct udma_dev *ud, u32 src_thread,
					 u32 dst_thread)
{
	dst_thread |= UDMA_PSIL_DST_THREAD_ID_OFFSET;
	return ud->tisci_psil_ops->unpair(ud->tisci,
					  ud->tisci_navss_dev_id,
					  src_thread, dst_thread);
}

static inline char *udma_get_dir_text(enum dma_direction dir)
{
	switch (dir) {
	case DMA_DEV_TO_MEM:
		return "DEV_TO_MEM";
	case DMA_MEM_TO_DEV:
		return "MEM_TO_DEV";
	case DMA_MEM_TO_MEM:
		return "MEM_TO_MEM";
	case DMA_DEV_TO_DEV:
		return "DEV_TO_DEV";
	default:
		break;
	}

	return "invalid";
}

static inline bool udma_is_chan_running(struct udma_chan *uc)
{
	u32 trt_ctl = 0;
	u32 rrt_ctl = 0;

	switch (uc->dir) {
	case DMA_DEV_TO_MEM:
		rrt_ctl = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_CTL_REG);
		pr_debug("%s: rrt_ctl: 0x%08x (peer: 0x%08x)\n",
			 __func__, rrt_ctl,
			 udma_rchanrt_read(uc->rchan,
					   UDMA_RCHAN_RT_PEER_RT_EN_REG));
		break;
	case DMA_MEM_TO_DEV:
		trt_ctl = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_CTL_REG);
		pr_debug("%s: trt_ctl: 0x%08x (peer: 0x%08x)\n",
			 __func__, trt_ctl,
			 udma_tchanrt_read(uc->tchan,
					   UDMA_TCHAN_RT_PEER_RT_EN_REG));
		break;
	case DMA_MEM_TO_MEM:
		trt_ctl = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_CTL_REG);
		rrt_ctl = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_CTL_REG);
		break;
	default:
		break;
	}

	if (trt_ctl & UDMA_CHAN_RT_CTL_EN || rrt_ctl & UDMA_CHAN_RT_CTL_EN)
		return true;

	return false;
}

static int udma_is_coherent(struct udma_chan *uc)
{
	return uc->ud->is_coherent;
}

static int udma_pop_from_ring(struct udma_chan *uc, dma_addr_t *addr)
{
	struct k3_nav_ring *ring = NULL;
	int ret = -ENOENT;

	switch (uc->dir) {
	case DMA_DEV_TO_MEM:
		ring = uc->rchan->r_ring;
		break;
	case DMA_MEM_TO_DEV:
		ring = uc->tchan->tc_ring;
		break;
	case DMA_MEM_TO_MEM:
		ring = uc->tchan->tc_ring;
		break;
	default:
		break;
	}

	if (ring && k3_nav_ringacc_ring_get_occ(ring))
		ret = k3_nav_ringacc_ring_pop(ring, addr);

	return ret;
}

static void udma_reset_rings(struct udma_chan *uc)
{
	struct k3_nav_ring *ring1 = NULL;
	struct k3_nav_ring *ring2 = NULL;

	switch (uc->dir) {
	case DMA_DEV_TO_MEM:
		ring1 = uc->rchan->fd_ring;
		ring2 = uc->rchan->r_ring;
		break;
	case DMA_MEM_TO_DEV:
		ring1 = uc->tchan->t_ring;
		ring2 = uc->tchan->tc_ring;
		break;
	case DMA_MEM_TO_MEM:
		ring1 = uc->tchan->t_ring;
		ring2 = uc->tchan->tc_ring;
		break;
	default:
		break;
	}

	if (ring1)
		k3_nav_ringacc_ring_reset_dma(ring1, 0);
	if (ring2)
		k3_nav_ringacc_ring_reset(ring2);
}

static void udma_reset_counters(struct udma_chan *uc)
{
	u32 val;

	if (uc->tchan) {
		val = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_BCNT_REG);
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_BCNT_REG, val);

		val = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_SBCNT_REG);
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_SBCNT_REG, val);

		val = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_PCNT_REG);
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_PCNT_REG, val);

		val = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_PEER_BCNT_REG);
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_PEER_BCNT_REG, val);
	}

	if (uc->rchan) {
		val = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_BCNT_REG);
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_BCNT_REG, val);

		val = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_SBCNT_REG);
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_SBCNT_REG, val);

		val = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_PCNT_REG);
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_PCNT_REG, val);

		val = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_PEER_BCNT_REG);
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_PEER_BCNT_REG, val);
	}

	uc->bcnt = 0;
}

static inline int udma_stop_hard(struct udma_chan *uc)
{
	pr_debug("%s: ENTER (chan%d)\n", __func__, uc->id);

	switch (uc->dir) {
	case DMA_DEV_TO_MEM:
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_PEER_RT_EN_REG, 0);
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_CTL_REG, 0);
		break;
	case DMA_MEM_TO_DEV:
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_CTL_REG, 0);
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_PEER_RT_EN_REG, 0);
		break;
	case DMA_MEM_TO_MEM:
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_CTL_REG, 0);
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_CTL_REG, 0);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int udma_start(struct udma_chan *uc)
{
	/* Channel is already running, no need to proceed further */
	if (udma_is_chan_running(uc))
		goto out;

	pr_debug("%s: chan:%d dir:%s (static_tr_type: %d)\n",
		 __func__, uc->id, udma_get_dir_text(uc->dir),
		 uc->static_tr_type);

	/* Make sure that we clear the teardown bit, if it is set */
	udma_stop_hard(uc);

	/* Reset all counters */
	udma_reset_counters(uc);

	switch (uc->dir) {
	case DMA_DEV_TO_MEM:
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_CTL_REG,
				   UDMA_CHAN_RT_CTL_EN);

		/* Enable remote */
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_PEER_RT_EN_REG,
				   UDMA_PEER_RT_EN_ENABLE);

		pr_debug("%s(rx): RT_CTL:0x%08x PEER RT_ENABLE:0x%08x\n",
			 __func__,
			 udma_rchanrt_read(uc->rchan,
					   UDMA_RCHAN_RT_CTL_REG),
			 udma_rchanrt_read(uc->rchan,
					   UDMA_RCHAN_RT_PEER_RT_EN_REG));
		break;
	case DMA_MEM_TO_DEV:
		/* Enable remote */
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_PEER_RT_EN_REG,
				   UDMA_PEER_RT_EN_ENABLE);

		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_CTL_REG,
				   UDMA_CHAN_RT_CTL_EN);

		pr_debug("%s(tx): RT_CTL:0x%08x PEER RT_ENABLE:0x%08x\n",
			 __func__,
			 udma_rchanrt_read(uc->rchan,
					   UDMA_TCHAN_RT_CTL_REG),
			 udma_rchanrt_read(uc->rchan,
					   UDMA_TCHAN_RT_PEER_RT_EN_REG));
		break;
	case DMA_MEM_TO_MEM:
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_CTL_REG,
				   UDMA_CHAN_RT_CTL_EN);
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_CTL_REG,
				   UDMA_CHAN_RT_CTL_EN);

		break;
	default:
		return -EINVAL;
	}

	pr_debug("%s: DONE chan:%d\n", __func__, uc->id);
out:
	return 0;
}

static inline void udma_stop_mem2dev(struct udma_chan *uc, bool sync)
{
	int i = 0;
	u32 val;

	udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_CTL_REG,
			   UDMA_CHAN_RT_CTL_EN |
			   UDMA_CHAN_RT_CTL_TDOWN);

	val = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_CTL_REG);

	while (sync && (val & UDMA_CHAN_RT_CTL_EN)) {
		val = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_CTL_REG);
		udelay(1);
		if (i > 1000) {
			printf(" %s TIMEOUT !\n", __func__);
			break;
		}
		i++;
	}

	val = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_PEER_RT_EN_REG);
	if (val & UDMA_PEER_RT_EN_ENABLE)
		printf("%s: peer not stopped TIMEOUT !\n", __func__);
}

static inline void udma_stop_dev2mem(struct udma_chan *uc, bool sync)
{
	int i = 0;
	u32 val;

	udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_PEER_RT_EN_REG,
			   UDMA_PEER_RT_EN_ENABLE |
			   UDMA_PEER_RT_EN_TEARDOWN);

	val = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_CTL_REG);

	while (sync && (val & UDMA_CHAN_RT_CTL_EN)) {
		val = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_CTL_REG);
		udelay(1);
		if (i > 1000) {
			printf("%s TIMEOUT !\n", __func__);
			break;
		}
		i++;
	}

	val = udma_rchanrt_read(uc->rchan, UDMA_RCHAN_RT_PEER_RT_EN_REG);
	if (val & UDMA_PEER_RT_EN_ENABLE)
		printf("%s: peer not stopped TIMEOUT !\n", __func__);
}

static inline int udma_stop(struct udma_chan *uc)
{
	pr_debug("%s: chan:%d dir:%s\n",
		 __func__, uc->id, udma_get_dir_text(uc->dir));

	udma_reset_counters(uc);
	switch (uc->dir) {
	case DMA_DEV_TO_MEM:
		udma_stop_dev2mem(uc, true);
		break;
	case DMA_MEM_TO_DEV:
		udma_stop_mem2dev(uc, true);
		break;
	case DMA_MEM_TO_MEM:
		udma_rchanrt_write(uc->rchan, UDMA_RCHAN_RT_CTL_REG, 0);
		udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_CTL_REG, 0);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static void udma_poll_completion(struct udma_chan *uc, dma_addr_t *paddr)
{
	int i = 1;

	while (udma_pop_from_ring(uc, paddr)) {
		udelay(1);
		if (!(i % 1000000))
			printf(".");
		i++;
	}
}

#define UDMA_RESERVE_RESOURCE(res)					\
static struct udma_##res *__udma_reserve_##res(struct udma_dev *ud,	\
					       int id)			\
{									\
	if (id >= 0) {							\
		if (test_bit(id, ud->res##_map)) {			\
			dev_err(ud->dev, "res##%d is in use\n", id);	\
			return ERR_PTR(-ENOENT);			\
		}							\
	} else {							\
		id = find_first_zero_bit(ud->res##_map, ud->res##_cnt); \
		if (id == ud->res##_cnt) {				\
			return ERR_PTR(-ENOENT);			\
		}							\
	}								\
									\
	__set_bit(id, ud->res##_map);					\
	return &ud->res##s[id];						\
}

UDMA_RESERVE_RESOURCE(tchan);
UDMA_RESERVE_RESOURCE(rchan);
UDMA_RESERVE_RESOURCE(rflow);

static int udma_get_tchan(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;

	if (uc->tchan) {
		dev_dbg(ud->dev, "chan%d: already have tchan%d allocated\n",
			uc->id, uc->tchan->id);
		return 0;
	}

	uc->tchan = __udma_reserve_tchan(ud, -1);
	if (IS_ERR(uc->tchan))
		return PTR_ERR(uc->tchan);

	pr_debug("chan%d: got tchan%d\n", uc->id, uc->tchan->id);

	return 0;
}

static int udma_get_rchan(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;

	if (uc->rchan) {
		dev_dbg(ud->dev, "chan%d: already have rchan%d allocated\n",
			uc->id, uc->rchan->id);
		return 0;
	}

	uc->rchan = __udma_reserve_rchan(ud, -1);
	if (IS_ERR(uc->rchan))
		return PTR_ERR(uc->rchan);

	pr_debug("chan%d: got rchan%d\n", uc->id, uc->rchan->id);

	return 0;
}

static int udma_get_chan_pair(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	int chan_id, end;

	if ((uc->tchan && uc->rchan) && uc->tchan->id == uc->rchan->id) {
		dev_info(ud->dev, "chan%d: already have %d pair allocated\n",
			 uc->id, uc->tchan->id);
		return 0;
	}

	if (uc->tchan) {
		dev_err(ud->dev, "chan%d: already have tchan%d allocated\n",
			uc->id, uc->tchan->id);
		return -EBUSY;
	} else if (uc->rchan) {
		dev_err(ud->dev, "chan%d: already have rchan%d allocated\n",
			uc->id, uc->rchan->id);
		return -EBUSY;
	}

	/* Can be optimized, but let's have it like this for now */
	end = min(ud->tchan_cnt, ud->rchan_cnt);
	for (chan_id = 0; chan_id < end; chan_id++) {
		if (!test_bit(chan_id, ud->tchan_map) &&
		    !test_bit(chan_id, ud->rchan_map))
			break;
	}

	if (chan_id == end)
		return -ENOENT;

	__set_bit(chan_id, ud->tchan_map);
	__set_bit(chan_id, ud->rchan_map);
	uc->tchan = &ud->tchans[chan_id];
	uc->rchan = &ud->rchans[chan_id];

	pr_debug("chan%d: got t/rchan%d pair\n", uc->id, chan_id);

	return 0;
}

static int udma_get_rflow(struct udma_chan *uc, int flow_id)
{
	struct udma_dev *ud = uc->ud;

	if (uc->rflow) {
		dev_dbg(ud->dev, "chan%d: already have rflow%d allocated\n",
			uc->id, uc->rflow->id);
		return 0;
	}

	if (!uc->rchan)
		dev_warn(ud->dev, "chan%d: does not have rchan??\n", uc->id);

	uc->rflow = __udma_reserve_rflow(ud, flow_id);
	if (IS_ERR(uc->rflow))
		return PTR_ERR(uc->rflow);

	pr_debug("chan%d: got rflow%d\n", uc->id, uc->rflow->id);
	return 0;
}

static void udma_put_rchan(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;

	if (uc->rchan) {
		dev_dbg(ud->dev, "chan%d: put rchan%d\n", uc->id,
			uc->rchan->id);
		__clear_bit(uc->rchan->id, ud->rchan_map);
		uc->rchan = NULL;
	}
}

static void udma_put_tchan(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;

	if (uc->tchan) {
		dev_dbg(ud->dev, "chan%d: put tchan%d\n", uc->id,
			uc->tchan->id);
		__clear_bit(uc->tchan->id, ud->tchan_map);
		uc->tchan = NULL;
	}
}

static void udma_put_rflow(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;

	if (uc->rflow) {
		dev_dbg(ud->dev, "chan%d: put rflow%d\n", uc->id,
			uc->rflow->id);
		__clear_bit(uc->rflow->id, ud->rflow_map);
		uc->rflow = NULL;
	}
}

static void udma_free_tx_resources(struct udma_chan *uc)
{
	if (!uc->tchan)
		return;

	k3_nav_ringacc_ring_free(uc->tchan->t_ring);
	k3_nav_ringacc_ring_free(uc->tchan->tc_ring);
	uc->tchan->t_ring = NULL;
	uc->tchan->tc_ring = NULL;

	udma_put_tchan(uc);
}

static int udma_alloc_tx_resources(struct udma_chan *uc)
{
	struct k3_nav_ring_cfg ring_cfg;
	struct udma_dev *ud = uc->ud;
	int ret;

	ret = udma_get_tchan(uc);
	if (ret)
		return ret;

	uc->tchan->t_ring = k3_nav_ringacc_request_ring(
				ud->ringacc, uc->tchan->id,
				RINGACC_RING_USE_PROXY);
	if (!uc->tchan->t_ring) {
		ret = -EBUSY;
		goto err_tx_ring;
	}

	uc->tchan->tc_ring = k3_nav_ringacc_request_ring(
				ud->ringacc, -1, RINGACC_RING_USE_PROXY);
	if (!uc->tchan->tc_ring) {
		ret = -EBUSY;
		goto err_txc_ring;
	}

	memset(&ring_cfg, 0, sizeof(ring_cfg));
	ring_cfg.size = 16;
	ring_cfg.elm_size = K3_NAV_RINGACC_RING_ELSIZE_8;
	ring_cfg.mode = K3_NAV_RINGACC_RING_MODE_MESSAGE;

	ret = k3_nav_ringacc_ring_cfg(uc->tchan->t_ring, &ring_cfg);
	ret |= k3_nav_ringacc_ring_cfg(uc->tchan->tc_ring, &ring_cfg);

	if (ret)
		goto err_ringcfg;

	return 0;

err_ringcfg:
	k3_nav_ringacc_ring_free(uc->tchan->tc_ring);
	uc->tchan->tc_ring = NULL;
err_txc_ring:
	k3_nav_ringacc_ring_free(uc->tchan->t_ring);
	uc->tchan->t_ring = NULL;
err_tx_ring:
	udma_put_tchan(uc);

	return ret;
}

static void udma_free_rx_resources(struct udma_chan *uc)
{
	if (!uc->rchan)
		return;

	k3_nav_ringacc_ring_free(uc->rchan->fd_ring);
	k3_nav_ringacc_ring_free(uc->rchan->r_ring);
	uc->rchan->fd_ring = NULL;
	uc->rchan->r_ring = NULL;

	udma_put_rflow(uc);
	udma_put_rchan(uc);
}

static int udma_alloc_rx_resources(struct udma_chan *uc)
{
	struct k3_nav_ring_cfg ring_cfg;
	struct udma_dev *ud = uc->ud;
	int fd_ring_id;
	int ret;

	ret = udma_get_rchan(uc);
	if (ret)
		return ret;

	/* For MEM_TO_MEM we don't need rflow or rings */
	if (uc->dir == DMA_MEM_TO_MEM)
		return 0;

	ret = udma_get_rflow(uc, uc->rchan->id);
	if (ret) {
		ret = -EBUSY;
		goto err_rflow;
	}

	fd_ring_id = ud->tchan_cnt + ud->echan_cnt + uc->rchan->id;

	uc->rchan->fd_ring = k3_nav_ringacc_request_ring(
				ud->ringacc, fd_ring_id,
				RINGACC_RING_USE_PROXY);
	if (!uc->rchan->fd_ring) {
		ret = -EBUSY;
		goto err_rx_ring;
	}

	uc->rchan->r_ring = k3_nav_ringacc_request_ring(
				ud->ringacc, -1, RINGACC_RING_USE_PROXY);
	if (!uc->rchan->r_ring) {
		ret = -EBUSY;
		goto err_rxc_ring;
	}

	memset(&ring_cfg, 0, sizeof(ring_cfg));
	ring_cfg.size = 16;
	ring_cfg.elm_size = K3_NAV_RINGACC_RING_ELSIZE_8;
	ring_cfg.mode = K3_NAV_RINGACC_RING_MODE_MESSAGE;

	ret = k3_nav_ringacc_ring_cfg(uc->rchan->fd_ring, &ring_cfg);
	ret |= k3_nav_ringacc_ring_cfg(uc->rchan->r_ring, &ring_cfg);

	if (ret)
		goto err_ringcfg;

	return 0;

err_ringcfg:
	k3_nav_ringacc_ring_free(uc->rchan->r_ring);
	uc->rchan->r_ring = NULL;
err_rxc_ring:
	k3_nav_ringacc_ring_free(uc->rchan->fd_ring);
	uc->rchan->fd_ring = NULL;
err_rx_ring:
	udma_put_rflow(uc);
err_rflow:
	udma_put_rchan(uc);

	return ret;
}

static int udma_alloc_tchan_sci_req(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	int tc_ring = k3_nav_ringacc_get_ring_id(uc->tchan->tc_ring);
	struct ti_sci_msg_rm_udmap_tx_ch_cfg req;
	u32 mode;
	int ret;

	if (uc->pkt_mode)
		mode = TI_SCI_RM_UDMAP_CHAN_TYPE_PKT_PBRR;
	else
		mode = TI_SCI_RM_UDMAP_CHAN_TYPE_3RDP_BCOPY_PBRR;

	req.valid_params = TI_SCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID;
	req.nav_id = ud->tisci_dev_id;
	req.index = uc->tchan->id;
	req.tx_chan_type = mode;
	if (uc->dir == DMA_MEM_TO_MEM)
		req.tx_fetch_size = sizeof(struct cppi5_desc_hdr_t) >> 2;
	else
		req.tx_fetch_size = cppi5_hdesc_calc_size(uc->needs_epib,
							  uc->psd_size,
							  0) >> 2;
	req.txcq_qnum = tc_ring;

	ret = ud->tisci_udmap_ops->tx_ch_cfg(ud->tisci, &req);
	if (ret)
		dev_err(ud->dev, "tisci tx alloc failed %d\n", ret);

	return ret;
}

static int udma_alloc_rchan_sci_req(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	int fd_ring = k3_nav_ringacc_get_ring_id(uc->rchan->fd_ring);
	int rx_ring = k3_nav_ringacc_get_ring_id(uc->rchan->r_ring);
	int tc_ring = k3_nav_ringacc_get_ring_id(uc->tchan->tc_ring);
	struct ti_sci_msg_rm_udmap_rx_ch_cfg req = { 0 };
	struct ti_sci_msg_rm_udmap_flow_cfg flow_req = { 0 };
	u32 mode;
	int ret;

	if (uc->pkt_mode)
		mode = TI_SCI_RM_UDMAP_CHAN_TYPE_PKT_PBRR;
	else
		mode = TI_SCI_RM_UDMAP_CHAN_TYPE_3RDP_BCOPY_PBRR;

	req.valid_params = TI_SCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID;
	req.nav_id = ud->tisci_dev_id;
	req.index = uc->rchan->id;
	req.rx_chan_type = mode;
	if (uc->dir == DMA_MEM_TO_MEM) {
		req.rx_fetch_size = sizeof(struct cppi5_desc_hdr_t) >> 2;
		req.rxcq_qnum = tc_ring;
	} else {
		req.rx_fetch_size = cppi5_hdesc_calc_size(uc->needs_epib,
							  uc->psd_size,
							  0) >> 2;
		req.rxcq_qnum = rx_ring;
	}
	if (uc->rflow->id != uc->rchan->id && uc->dir != DMA_MEM_TO_MEM) {
		req.flowid_start = uc->rflow->id;
		req.flowid_cnt = 1;
		req.valid_params |=
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_START_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID;
	}

	ret = ud->tisci_udmap_ops->rx_ch_cfg(ud->tisci, &req);
	if (ret) {
		dev_err(ud->dev, "tisci rx %u cfg failed %d\n",
			uc->rchan->id, ret);
		return ret;
	}
	if (uc->dir == DMA_MEM_TO_MEM)
		return ret;

	flow_req.valid_params =
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_EINFO_PRESENT_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_PSINFO_PRESENT_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_ERROR_HANDLING_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DESC_TYPE_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_QNUM_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_SEL_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_SEL_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_SEL_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_SEL_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ0_QNUM_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ1_QNUM_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ2_QNUM_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ3_QNUM_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_PS_LOCATION_VALID;

	flow_req.nav_id = ud->tisci_dev_id;
	flow_req.flow_index = uc->rflow->id;

	if (uc->needs_epib)
		flow_req.rx_einfo_present = 1;
	else
		flow_req.rx_einfo_present = 0;

	if (uc->psd_size)
		flow_req.rx_psinfo_present = 1;
	else
		flow_req.rx_psinfo_present = 0;

	flow_req.rx_error_handling = 0;
	flow_req.rx_desc_type = 0;
	flow_req.rx_dest_qnum = rx_ring;
	flow_req.rx_src_tag_hi_sel = 2;
	flow_req.rx_src_tag_lo_sel = 4;
	flow_req.rx_dest_tag_hi_sel = 5;
	flow_req.rx_dest_tag_lo_sel = 4;
	flow_req.rx_fdq0_sz0_qnum = fd_ring;
	flow_req.rx_fdq1_qnum = fd_ring;
	flow_req.rx_fdq2_qnum = fd_ring;
	flow_req.rx_fdq3_qnum = fd_ring;
	flow_req.rx_ps_location = 0;

	ret = ud->tisci_udmap_ops->rx_flow_cfg(ud->tisci, &flow_req);
	if (ret)
		dev_err(ud->dev, "tisci rx %u flow %u cfg failed %d\n",
			uc->rchan->id, uc->rflow->id, ret);

	return ret;
}

static int udma_alloc_chan_resources(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	int ret;

	pr_debug("%s: chan:%d as %s\n",
		 __func__, uc->id, udma_get_dir_text(uc->dir));

	switch (uc->dir) {
	case DMA_MEM_TO_MEM:
		/* Non synchronized - mem to mem type of transfer */
		ret = udma_get_chan_pair(uc);
		if (ret)
			return ret;

		ret = udma_alloc_tx_resources(uc);
		if (ret)
			goto err_free_res;

		ret = udma_alloc_rx_resources(uc);
		if (ret)
			goto err_free_res;

		uc->src_thread = ud->psil_base + uc->tchan->id;
		uc->dst_thread = (ud->psil_base + uc->rchan->id) | 0x8000;
		break;
	case DMA_MEM_TO_DEV:
		/* Slave transfer synchronized - mem to dev (TX) trasnfer */
		ret = udma_alloc_tx_resources(uc);
		if (ret)
			goto err_free_res;

		uc->src_thread = ud->psil_base + uc->tchan->id;
		uc->dst_thread = uc->slave_thread_id;
		if (!(uc->dst_thread & 0x8000))
			uc->dst_thread |= 0x8000;

		break;
	case DMA_DEV_TO_MEM:
		/* Slave transfer synchronized - dev to mem (RX) trasnfer */
		ret = udma_alloc_rx_resources(uc);
		if (ret)
			goto err_free_res;

		uc->src_thread = uc->slave_thread_id;
		uc->dst_thread = (ud->psil_base + uc->rchan->id) | 0x8000;

		break;
	default:
		/* Can not happen */
		pr_debug("%s: chan:%d invalid direction (%u)\n",
			 __func__, uc->id, uc->dir);
		return -EINVAL;
	}

	/* We have channel indexes and rings */
	if (uc->dir == DMA_MEM_TO_MEM) {
		ret = udma_alloc_tchan_sci_req(uc);
		if (ret)
			goto err_free_res;

		ret = udma_alloc_rchan_sci_req(uc);
		if (ret)
			goto err_free_res;
	} else {
		/* Slave transfer */
		if (uc->dir == DMA_MEM_TO_DEV) {
			ret = udma_alloc_tchan_sci_req(uc);
			if (ret)
				goto err_free_res;
		} else {
			ret = udma_alloc_rchan_sci_req(uc);
			if (ret)
				goto err_free_res;
		}
	}

	if (udma_is_chan_running(uc)) {
		dev_warn(ud->dev, "chan%d: is running!\n", uc->id);
		udma_stop(uc);
		if (udma_is_chan_running(uc)) {
			dev_err(ud->dev, "chan%d: won't stop!\n", uc->id);
			goto err_free_res;
		}
	}

	/* PSI-L pairing */
	ret = udma_navss_psil_pair(ud, uc->src_thread, uc->dst_thread);
	if (ret) {
		dev_err(ud->dev, "k3_nav_psil_request_link fail\n");
		goto err_free_res;
	}

	return 0;

err_free_res:
	udma_free_tx_resources(uc);
	udma_free_rx_resources(uc);
	uc->slave_thread_id = -1;
	return ret;
}

static void udma_free_chan_resources(struct udma_chan *uc)
{
	/* Some configuration to UDMA-P channel: disable, reset, whatever */

	/* Release PSI-L pairing */
	udma_navss_psil_unpair(uc->ud, uc->src_thread, uc->dst_thread);

	/* Reset the rings for a new start */
	udma_reset_rings(uc);
	udma_free_tx_resources(uc);
	udma_free_rx_resources(uc);

	uc->slave_thread_id = -1;
	uc->dir = DMA_MEM_TO_MEM;
}

static int udma_get_mmrs(struct udevice *dev)
{
	struct udma_dev *ud = dev_get_priv(dev);
	int i;

	for (i = 0; i < MMR_LAST; i++) {
		ud->mmrs[i] = (uint32_t *)devfdt_get_addr_name(dev,
				mmr_names[i]);
		if (!ud->mmrs[i])
			return -EINVAL;
	}

	return 0;
}

#define UDMA_MAX_CHANNELS	192

static int udma_probe(struct udevice *dev)
{
	struct dma_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct udma_dev *ud = dev_get_priv(dev);
	int i, ret;
	u32 cap2, cap3;
	struct udevice *tmp;
	struct udevice *tisci_dev = NULL;

	ret = udma_get_mmrs(dev);
	if (ret)
		return ret;

	ret = uclass_get_device_by_phandle(UCLASS_MISC, dev,
					   "ti,ringacc", &tmp);
	ud->ringacc = dev_get_priv(tmp);
	if (IS_ERR(ud->ringacc))
		return PTR_ERR(ud->ringacc);

	ud->psil_base = dev_read_u32_default(dev, "ti,psil-base", 0);
	if (!ud->psil_base) {
		dev_info(dev,
			 "Missing ti,psil-base property, using %d.\n", ret);
		return -EINVAL;
	}

	ret = uclass_get_device_by_name(UCLASS_FIRMWARE, "dmsc", &tisci_dev);
	if (ret) {
		debug("TISCI RA RM get failed (%d)\n", ret);
		ud->tisci = NULL;
		return 0;
	}
	ud->tisci = (struct ti_sci_handle *)
			 (ti_sci_get_handle_from_sysfw(tisci_dev));

	ret = dev_read_u32_default(dev, "ti,sci", 0);
	if (!ret) {
		dev_err(dev, "TISCI RA RM disabled\n");
		ud->tisci = NULL;
	}

	if (ud->tisci) {
		ofnode navss_ofnode = ofnode_get_parent(dev_ofnode(dev));

		ud->tisci_dev_id = -1;
		ret = dev_read_u32(dev, "ti,sci-dev-id", &ud->tisci_dev_id);
		if (ret) {
			dev_err(dev, "ti,sci-dev-id read failure %d\n", ret);
			return ret;
		}

		ud->tisci_navss_dev_id = -1;
		ret = ofnode_read_u32(navss_ofnode, "ti,sci-dev-id",
				      &ud->tisci_navss_dev_id);
		if (ret) {
			dev_err(dev, "navss sci-dev-id read failure %d\n", ret);
			return ret;
		}

		ud->tisci_udmap_ops = &ud->tisci->ops.rm_udmap_ops;
		ud->tisci_psil_ops = &ud->tisci->ops.rm_psil_ops;
	}

	ud->is_coherent = dev_read_bool(dev, "dma-coherent");

	cap2 = udma_read(ud->mmrs[MMR_GCFG], 0x28);
	cap3 = udma_read(ud->mmrs[MMR_GCFG], 0x2c);

	ud->rflow_cnt = cap3 & 0x3fff;
	ud->tchan_cnt = cap2 & 0x1ff;
	ud->echan_cnt = (cap2 >> 9) & 0x1ff;
	ud->rchan_cnt = (cap2 >> 18) & 0x1ff;
	ud->ch_count  = ud->tchan_cnt + ud->rchan_cnt;

	dev_info(dev,
		 "Number of channels: %u (tchan: %u, echan: %u, rchan: %u dev-id %u)\n",
		 ud->ch_count, ud->tchan_cnt, ud->echan_cnt, ud->rchan_cnt,
		 ud->tisci_dev_id);
	dev_info(dev, "Number of rflows: %u\n", ud->rflow_cnt);

	ud->channels = devm_kcalloc(dev, ud->ch_count, sizeof(*ud->channels),
				    GFP_KERNEL);
	ud->tchan_map = devm_kcalloc(dev, BITS_TO_LONGS(ud->tchan_cnt),
				     sizeof(unsigned long), GFP_KERNEL);
	ud->tchans = devm_kcalloc(dev, ud->tchan_cnt,
				  sizeof(*ud->tchans), GFP_KERNEL);
	ud->rchan_map = devm_kcalloc(dev, BITS_TO_LONGS(ud->rchan_cnt),
				     sizeof(unsigned long), GFP_KERNEL);
	ud->rchans = devm_kcalloc(dev, ud->rchan_cnt,
				  sizeof(*ud->rchans), GFP_KERNEL);
	ud->rflow_map = devm_kcalloc(dev, BITS_TO_LONGS(ud->rflow_cnt),
				     sizeof(unsigned long), GFP_KERNEL);
	ud->rflows = devm_kcalloc(dev, ud->rflow_cnt,
				  sizeof(*ud->rflows), GFP_KERNEL);

	if (!ud->channels || !ud->tchan_map || !ud->rchan_map ||
	    !ud->rflow_map || !ud->tchans || !ud->rchans || !ud->rflows)
		return -ENOMEM;

	for (i = 0; i < ud->tchan_cnt; i++) {
		struct udma_tchan *tchan = &ud->tchans[i];

		tchan->id = i;
		tchan->reg_rt = ud->mmrs[MMR_TCHANRT] + UDMA_CH_1000(i);
	}

	for (i = 0; i < ud->rchan_cnt; i++) {
		struct udma_rchan *rchan = &ud->rchans[i];

		rchan->id = i;
		rchan->reg_rt = ud->mmrs[MMR_RCHANRT] + UDMA_CH_1000(i);
	}

	for (i = 0; i < ud->rflow_cnt; i++) {
		struct udma_rflow *rflow = &ud->rflows[i];

		rflow->id = i;
	}

	for (i = 0; i < ud->ch_count; i++) {
		struct udma_chan *uc = &ud->channels[i];

		uc->ud = ud;
		uc->id = i;
		uc->slave_thread_id = -1;
		uc->tchan = NULL;
		uc->rchan = NULL;
		uc->dir = DMA_MEM_TO_MEM;
		sprintf(uc->name, "UDMA chan%d\n", i);
		if (!i)
			uc->in_use = true;
	}

	pr_debug("UDMA(rev: 0x%08x) CAP0-3: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
		 udma_read(ud->mmrs[MMR_GCFG], 0),
		 udma_read(ud->mmrs[MMR_GCFG], 0x20),
		 udma_read(ud->mmrs[MMR_GCFG], 0x24),
		 udma_read(ud->mmrs[MMR_GCFG], 0x28),
		 udma_read(ud->mmrs[MMR_GCFG], 0x2c));

	uc_priv->supported = DMA_SUPPORTS_MEM_TO_MEM | DMA_SUPPORTS_MEM_TO_DEV;

	return ret;
}

static int *udma_prep_dma_memcpy(struct udma_chan *uc, dma_addr_t dest,
				 dma_addr_t src, size_t len)
{
	u32 tc_ring_id = k3_nav_ringacc_get_ring_id(uc->tchan->tc_ring);
	struct cppi5_tr_type15_t *tr_req;
	int num_tr;
	size_t tr_size = sizeof(struct cppi5_tr_type15_t);
	u16 tr0_cnt0, tr0_cnt1, tr1_cnt0;
	unsigned long dummy;
	void *tr_desc;
	size_t desc_size;

	if (len < SZ_64K) {
		num_tr = 1;
		tr0_cnt0 = len;
		tr0_cnt1 = 1;
	} else {
		unsigned long align_to = __ffs(src | dest);

		if (align_to > 3)
			align_to = 3;
		/*
		 * Keep simple: tr0: SZ_64K-alignment blocks,
		 *		tr1: the remaining
		 */
		num_tr = 2;
		tr0_cnt0 = (SZ_64K - BIT(align_to));
		if (len / tr0_cnt0 >= SZ_64K) {
			dev_err(uc->ud->dev, "size %zu is not supported\n",
				len);
			return NULL;
		}

		tr0_cnt1 = len / tr0_cnt0;
		tr1_cnt0 = len % tr0_cnt0;
	}

	desc_size = cppi5_trdesc_calc_size(num_tr, tr_size);
	tr_desc = dma_alloc_coherent(desc_size, &dummy);
	if (!tr_desc)
		return NULL;
	memset(tr_desc, 0, desc_size);

	cppi5_trdesc_init(tr_desc, num_tr, tr_size, 0, 0);
	cppi5_desc_set_pktids(tr_desc, uc->id, 0x3fff);
	cppi5_desc_set_retpolicy(tr_desc, 0, tc_ring_id);

	tr_req = tr_desc + tr_size;

	cppi5_tr_init(&tr_req[0].flags, CPPI5_TR_TYPE15, false, true,
		      CPPI5_TR_EVENT_SIZE_COMPLETION, 1);
	cppi5_tr_csf_set(&tr_req[0].flags, CPPI5_TR_CSF_SUPR_EVT);

	tr_req[0].addr = src;
	tr_req[0].icnt0 = tr0_cnt0;
	tr_req[0].icnt1 = tr0_cnt1;
	tr_req[0].icnt2 = 1;
	tr_req[0].icnt3 = 1;
	tr_req[0].dim1 = tr0_cnt0;

	tr_req[0].daddr = dest;
	tr_req[0].dicnt0 = tr0_cnt0;
	tr_req[0].dicnt1 = tr0_cnt1;
	tr_req[0].dicnt2 = 1;
	tr_req[0].dicnt3 = 1;
	tr_req[0].ddim1 = tr0_cnt0;

	if (num_tr == 2) {
		cppi5_tr_init(&tr_req[1].flags, CPPI5_TR_TYPE15, false, true,
			      CPPI5_TR_EVENT_SIZE_COMPLETION, 0);
		cppi5_tr_csf_set(&tr_req[1].flags, CPPI5_TR_CSF_SUPR_EVT);

		tr_req[1].addr = src + tr0_cnt1 * tr0_cnt0;
		tr_req[1].icnt0 = tr1_cnt0;
		tr_req[1].icnt1 = 1;
		tr_req[1].icnt2 = 1;
		tr_req[1].icnt3 = 1;

		tr_req[1].daddr = dest + tr0_cnt1 * tr0_cnt0;
		tr_req[1].dicnt0 = tr1_cnt0;
		tr_req[1].dicnt1 = 1;
		tr_req[1].dicnt2 = 1;
		tr_req[1].dicnt3 = 1;
	}

	cppi5_tr_csf_set(&tr_req[num_tr - 1].flags, CPPI5_TR_CSF_EOP);

	if (!udma_is_coherent(uc)) {
		flush_dcache_range((u64)tr_desc,
				   ALIGN((u64)tr_desc + desc_size,
					 ARCH_DMA_MINALIGN));
	}

	k3_nav_ringacc_ring_push(uc->tchan->t_ring, &tr_desc);

	return 0;
}

static int udma_transfer(struct udevice *dev, int direction,
			 void *dst, void *src, size_t len)
{
	struct udma_dev *ud = dev_get_priv(dev);
	/* Channel0 is reserved for memcpy */
	struct udma_chan *uc = &ud->channels[0];
	dma_addr_t paddr = 0;
	int ret;

	ret = udma_alloc_chan_resources(uc);
	if (ret)
		return ret;

	udma_prep_dma_memcpy(uc, (dma_addr_t)dst, (dma_addr_t)src, len);
	udma_start(uc);
	udma_poll_completion(uc, &paddr);
	udma_stop(uc);

	udma_free_chan_resources(uc);
	return 0;
}

static int udma_request(struct dma *dma)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct udma_chan *uc;
	unsigned long dummy;
	int ret;

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}

	uc = &ud->channels[dma->id];
	ret = udma_alloc_chan_resources(uc);
	if (ret) {
		dev_err(dma->dev, "alloc dma res failed %d\n", ret);
		return -EINVAL;
	}

	uc->hdesc_size = cppi5_hdesc_calc_size(uc->needs_epib,
					       uc->psd_size, 0);
	uc->hdesc_size = ALIGN(uc->hdesc_size, ARCH_DMA_MINALIGN);

	if (uc->dir == DMA_MEM_TO_DEV) {
		uc->desc_tx = dma_alloc_coherent(uc->hdesc_size, &dummy);
		memset(uc->desc_tx, 0, uc->hdesc_size);
	} else {
		uc->desc_rx = dma_alloc_coherent(
				uc->hdesc_size * UDMA_RX_DESC_NUM, &dummy);
		memset(uc->desc_rx, 0, uc->hdesc_size * UDMA_RX_DESC_NUM);
	}

	uc->in_use = true;
	uc->desc_rx_cur = 0;
	uc->num_rx_bufs = 0;

	return 0;
}

static int udma_free(struct dma *dma)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct udma_chan *uc;

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}
	uc = &ud->channels[dma->id];

	if (udma_is_chan_running(uc))
		udma_stop(uc);
	udma_free_chan_resources(uc);

	uc->in_use = false;

	return 0;
}

static int udma_enable(struct dma *dma)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct udma_chan *uc;
	int ret;

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}
	uc = &ud->channels[dma->id];

	ret = udma_start(uc);

	return ret;
}

static int udma_disable(struct dma *dma)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct udma_chan *uc;
	int ret = 0;

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}
	uc = &ud->channels[dma->id];

	if (udma_is_chan_running(uc))
		ret = udma_stop(uc);
	else
		dev_err(dma->dev, "%s not running\n", __func__);

	return ret;
}

static int udma_send(struct dma *dma, void *src, size_t len, void *metadata)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct cppi5_host_desc_t *desc_tx;
	dma_addr_t dma_src = (dma_addr_t)src;
	struct ti_udma_drv_packet_data packet_data = { 0 };
	dma_addr_t paddr;
	struct udma_chan *uc;
	u32 tc_ring_id;
	int ret;

	if (metadata)
		packet_data = *((struct ti_udma_drv_packet_data *)metadata);

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}
	uc = &ud->channels[dma->id];

	if (uc->dir != DMA_MEM_TO_DEV)
		return -EINVAL;

	tc_ring_id = k3_nav_ringacc_get_ring_id(uc->tchan->tc_ring);

	desc_tx = uc->desc_tx;

	cppi5_hdesc_reset_hbdesc(desc_tx);

	cppi5_hdesc_init(desc_tx,
			 uc->needs_epib ? CPPI5_INFO0_HDESC_EPIB_PRESENT : 0,
			 uc->psd_size);
	cppi5_hdesc_set_pktlen(desc_tx, len);
	cppi5_hdesc_attach_buf(desc_tx, dma_src, len, dma_src, len);
	cppi5_desc_set_pktids(&desc_tx->hdr, uc->id, 0x3fff);
	cppi5_desc_set_retpolicy(&desc_tx->hdr, 0, tc_ring_id);
	/* pass below information from caller */
	cppi5_hdesc_set_pkttype(desc_tx, packet_data.pkt_type);
	cppi5_desc_set_tags_ids(&desc_tx->hdr, 0, packet_data.dest_tag);

	if (!udma_is_coherent(uc)) {
		flush_dcache_range((u64)dma_src,
				   ALIGN((u64)dma_src + len,
					 ARCH_DMA_MINALIGN));
		flush_dcache_range((u64)desc_tx,
				   ALIGN((u64)desc_tx + uc->hdesc_size,
					 ARCH_DMA_MINALIGN));
	}

	ret = k3_nav_ringacc_ring_push(uc->tchan->t_ring, &uc->desc_tx);
	if (ret) {
		dev_err(dma->dev, "TX dma push fail ch_id %lu %d\n",
			dma->id, ret);
		return ret;
	}

	udma_poll_completion(uc, &paddr);

	return 0;
}

static int udma_receive(struct dma *dma, void **dst, void *metadata)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct cppi5_host_desc_t *desc_rx;
	dma_addr_t buf_dma;
	struct udma_chan *uc;
	u32 buf_dma_len, pkt_len;
	u32 port_id = 0;
	int ret;

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}
	uc = &ud->channels[dma->id];

	if (uc->dir != DMA_DEV_TO_MEM)
		return -EINVAL;
	if (!uc->num_rx_bufs)
		return -EINVAL;

	ret = k3_nav_ringacc_ring_pop(uc->rchan->r_ring, &desc_rx);
	if (ret && ret != -ENODATA) {
		dev_err(dma->dev, "rx dma fail ch_id:%lu %d\n", dma->id, ret);
		return ret;
	} else if (ret == -ENODATA) {
		return 0;
	}

	/* invalidate cache data */
	if (!udma_is_coherent(uc)) {
		invalidate_dcache_range((ulong)desc_rx,
					(ulong)(desc_rx + uc->hdesc_size));
	}

	cppi5_hdesc_get_obuf(desc_rx, &buf_dma, &buf_dma_len);
	pkt_len = cppi5_hdesc_get_pktlen(desc_rx);

	/* invalidate cache data */
	if (!udma_is_coherent(uc)) {
		invalidate_dcache_range((ulong)buf_dma,
					(ulong)(buf_dma + buf_dma_len));
	}

	cppi5_desc_get_tags_ids(&desc_rx->hdr, &port_id, NULL);

	*dst = (void *)buf_dma;
	uc->num_rx_bufs--;

	return pkt_len;
}

static int udma_of_xlate(struct dma *dma, struct ofnode_phandle_args *args)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct udma_chan *uc = &ud->channels[0];
	ofnode chconf_node, slave_node;
	char prop[50];
	u32 val;

	for (val = 0; val < ud->ch_count; val++) {
		uc = &ud->channels[val];
		if (!uc->in_use)
			break;
	}

	if (val == ud->ch_count)
		return -EBUSY;

	uc->dir = DMA_DEV_TO_MEM;
	if (args->args[2] == UDMA_DIR_TX)
		uc->dir = DMA_MEM_TO_DEV;

	slave_node = ofnode_get_by_phandle(args->args[0]);
	if (!ofnode_valid(slave_node)) {
		dev_err(ud->dev, "slave node is missing\n");
		return -EINVAL;
	}

	snprintf(prop, sizeof(prop), "ti,psil-config%u", args->args[1]);
	chconf_node = ofnode_find_subnode(slave_node, prop);
	if (!ofnode_valid(chconf_node)) {
		dev_err(ud->dev, "Channel configuration node is missing\n");
		return -EINVAL;
	}

	if (!ofnode_read_u32(chconf_node, "linux,udma-mode", &val)) {
		if (val == UDMA_PKT_MODE)
			uc->pkt_mode = true;
	}

	if (!ofnode_read_u32(chconf_node, "statictr-type", &val))
		uc->static_tr_type = val;

	uc->needs_epib = ofnode_read_bool(chconf_node, "ti,needs-epib");
	if (!ofnode_read_u32(chconf_node, "ti,psd-size", &val))
		uc->psd_size = val;
	uc->metadata_size = (uc->needs_epib ? 16 : 0) + uc->psd_size;

	if (ofnode_read_u32(slave_node, "ti,psil-base", &val)) {
		dev_err(ud->dev, "ti,psil-base is missing\n");
		return -EINVAL;
	}

	uc->slave_thread_id = val + args->args[1];

	dma->id = uc->id;
	pr_debug("Allocated dma chn:%lu epib:%d psdata:%u meta:%u thread_id:%x\n",
		 dma->id, uc->needs_epib,
		 uc->psd_size, uc->metadata_size,
		 uc->slave_thread_id);

	return 0;
}

int udma_prepare_rcv_buf(struct dma *dma, void *dst, size_t size)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct cppi5_host_desc_t *desc_rx;
	dma_addr_t dma_dst;
	struct udma_chan *uc;
	u32 desc_num;

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}
	uc = &ud->channels[dma->id];

	if (uc->dir != DMA_DEV_TO_MEM)
		return -EINVAL;

	if (uc->num_rx_bufs >= UDMA_RX_DESC_NUM)
		return -EINVAL;

	desc_num = uc->desc_rx_cur % UDMA_RX_DESC_NUM;
	desc_rx = uc->desc_rx + (desc_num * uc->hdesc_size);
	dma_dst = (dma_addr_t)dst;

	cppi5_hdesc_reset_hbdesc(desc_rx);

	cppi5_hdesc_init(desc_rx,
			 uc->needs_epib ? CPPI5_INFO0_HDESC_EPIB_PRESENT : 0,
			 uc->psd_size);
	cppi5_hdesc_set_pktlen(desc_rx, size);
	cppi5_hdesc_attach_buf(desc_rx, dma_dst, size, dma_dst, size);

	if (!udma_is_coherent(uc)) {
		flush_dcache_range((u64)desc_rx,
				   ALIGN((u64)desc_rx + uc->hdesc_size,
					 ARCH_DMA_MINALIGN));
	}

	k3_nav_ringacc_ring_push(uc->rchan->fd_ring, &desc_rx);

	uc->num_rx_bufs++;
	uc->desc_rx_cur++;

	return 0;
}

static const struct dma_ops udma_ops = {
	.transfer	= udma_transfer,
	.of_xlate	= udma_of_xlate,
	.request	= udma_request,
	.free		= udma_free,
	.enable		= udma_enable,
	.disable	= udma_disable,
	.send		= udma_send,
	.receive	= udma_receive,
	.prepare_rcv_buf = udma_prepare_rcv_buf,
};

static const struct udevice_id udma_ids[] = {
	{ .compatible = "ti,k3-navss-udmap" },
	{ }
};

U_BOOT_DRIVER(ti_edma3) = {
	.name	= "ti-udma",
	.id	= UCLASS_DMA,
	.of_match = udma_ids,
	.ops	= &udma_ops,
	.probe	= udma_probe,
	.priv_auto_alloc_size = sizeof(struct udma_dev),
};
