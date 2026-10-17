// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com
 *  Author: Peter Ujfalusi <peter.ujfalusi@ti.com>
 */
#define pr_fmt(fmt) "udma: " fmt

#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <malloc.h>
#include <linux/bitops.h>
#include <linux/dma-mapping.h>
#include <linux/sizes.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/read.h>
#include <dm/of_access.h>
#include <dma.h>
#include <dma-uclass.h>
#include <linux/delay.h>
#include <linux/bitmap.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/soc/ti/k3-navss-ringacc.h>
#include <linux/soc/ti/cppi5.h>
#include <linux/soc/ti/ti-udma.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <linux/soc/ti/cppi5.h>

#include "k3-udma-hwdef.h"
#include "k3-psil-priv.h"

#define K3_UDMA_MAX_RFLOWS 1024

struct udma_chan;

enum k3_dma_type {
	DMA_TYPE_UDMA = 0,
	DMA_TYPE_BCDMA,
	DMA_TYPE_PKTDMA,
};

enum udma_mmr {
	MMR_GCFG = 0,
	MMR_BCHANRT,
	MMR_RCHANRT,
	MMR_TCHANRT,
	MMR_RCHAN,
	MMR_TCHAN,
	MMR_RFLOW,
	MMR_LAST,
};

static const char * const mmr_names[] = {
	[MMR_GCFG] = "gcfg",
	[MMR_BCHANRT] = "bchanrt",
	[MMR_RCHANRT] = "rchanrt",
	[MMR_TCHANRT] = "tchanrt",
	[MMR_RCHAN] = "rchan",
	[MMR_TCHAN] = "tchan",
	[MMR_RFLOW] = "rflow",
};

struct udma_tchan {
	void __iomem *reg_chan;
	void __iomem *reg_rt;

	int id;
	struct k3_nav_ring *t_ring; /* Transmit ring */
	struct k3_nav_ring *tc_ring; /* Transmit Completion ring */
	int tflow_id; /* applicable only for PKTDMA */

};

#define udma_bchan udma_tchan

struct udma_rflow {
	void __iomem *reg_rflow;
	int id;
	struct k3_nav_ring *fd_ring; /* Free Descriptor ring */
	struct k3_nav_ring *r_ring; /* Receive ring */
};

struct udma_rchan {
	void __iomem *reg_chan;
	void __iomem *reg_rt;

	int id;
};

struct udma_oes_offsets {
	/* K3 UDMA Output Event Offset */
	u32 udma_rchan;

	/* BCDMA Output Event Offsets */
	u32 bcdma_bchan_data;
	u32 bcdma_bchan_ring;
	u32 bcdma_tchan_data;
	u32 bcdma_tchan_ring;
	u32 bcdma_rchan_data;
	u32 bcdma_rchan_ring;

	/* PKTDMA Output Event Offsets */
	u32 pktdma_tchan_flow;
	u32 pktdma_rchan_flow;
};

#define UDMA_FLAG_PDMA_ACC32		BIT(0)
#define UDMA_FLAG_PDMA_BURST		BIT(1)
#define UDMA_FLAG_TDTYPE		BIT(2)

struct udma_match_data {
	enum k3_dma_type type;
	u32 psil_base;
	bool enable_memcpy_support;
	u32 flags;
	u32 statictr_z_mask;
	struct udma_oes_offsets oes;

	u8 tpl_levels;
	u32 level_start_idx[];
};

enum udma_rm_range {
	RM_RANGE_BCHAN = 0,
	RM_RANGE_TCHAN,
	RM_RANGE_RCHAN,
	RM_RANGE_RFLOW,
	RM_RANGE_TFLOW,
	RM_RANGE_LAST,
};

struct udma_tisci_rm {
	const struct ti_sci_handle *tisci;
	const struct ti_sci_rm_udmap_ops *tisci_udmap_ops;
	u32  tisci_dev_id;

	/* tisci information for PSI-L thread pairing/unpairing */
	const struct ti_sci_rm_psil_ops *tisci_psil_ops;
	u32  tisci_navss_dev_id;

	struct ti_sci_resource *rm_ranges[RM_RANGE_LAST];
};

struct udma_dev {
	struct udevice *dev;
	void __iomem *mmrs[MMR_LAST];

	struct udma_tisci_rm tisci_rm;
	struct k3_nav_ringacc *ringacc;

	u32 features;

	int bchan_cnt;
	int tchan_cnt;
	int echan_cnt;
	int rchan_cnt;
	int rflow_cnt;
	int tflow_cnt;
	unsigned long *bchan_map;
	unsigned long *tchan_map;
	unsigned long *rchan_map;
	unsigned long *rflow_map;
	unsigned long *rflow_map_reserved;
	unsigned long *tflow_map;

	struct udma_bchan *bchans;
	struct udma_tchan *tchans;
	struct udma_rchan *rchans;
	struct udma_rflow *rflows;

	struct udma_match_data *match_data;

	struct udma_chan *channels;
	u32 psil_base;

	u32 ch_count;
};

struct udma_chan_config {
	u32 psd_size; /* size of Protocol Specific Data */
	u32 metadata_size; /* (needs_epib ? 16:0) + psd_size */
	u32 hdesc_size; /* Size of a packet descriptor in packet mode */
	int remote_thread_id;
	u32 atype;
	u32 src_thread;
	u32 dst_thread;
	enum psil_endpoint_type ep_type;
	enum udma_tp_level channel_tpl; /* Channel Throughput Level */

	/* PKTDMA mapped channel */
	int mapped_channel_id;
	/* PKTDMA default tflow or rflow for mapped channel */
	int default_flow_id;

	enum dma_direction dir;

	unsigned int pkt_mode:1; /* TR or packet */
	unsigned int needs_epib:1; /* EPIB is needed for the communication or not */
	unsigned int enable_acc32:1;
	unsigned int enable_burst:1;
	unsigned int notdpkt:1; /* Suppress sending TDC packet */
};

struct udma_chan {
	struct udma_dev *ud;
	char name[20];

	struct udma_bchan *bchan;
	struct udma_tchan *tchan;
	struct udma_rchan *rchan;
	struct udma_rflow *rflow;

	struct ti_udma_drv_chan_cfg_data cfg_data;

	u32 bcnt; /* number of bytes completed since the start of the channel */

	struct udma_chan_config config;

	u32 id;

	struct cppi5_host_desc_t *desc_tx;
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
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;

	dst_thread |= UDMA_PSIL_DST_THREAD_ID_OFFSET;

	return tisci_rm->tisci_psil_ops->pair(tisci_rm->tisci,
					      tisci_rm->tisci_navss_dev_id,
					      src_thread, dst_thread);
}

static inline int udma_navss_psil_unpair(struct udma_dev *ud, u32 src_thread,
					 u32 dst_thread)
{
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;

	dst_thread |= UDMA_PSIL_DST_THREAD_ID_OFFSET;

	return tisci_rm->tisci_psil_ops->unpair(tisci_rm->tisci,
						tisci_rm->tisci_navss_dev_id,
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

#include "k3-udma-u-boot.c"

static void udma_reset_uchan(struct udma_chan *uc)
{
	memset(&uc->config, 0, sizeof(uc->config));
	uc->config.remote_thread_id = -1;
	uc->config.mapped_channel_id = -1;
	uc->config.default_flow_id = -1;
}

static inline bool udma_is_chan_running(struct udma_chan *uc)
{
	u32 trt_ctl = 0;
	u32 rrt_ctl = 0;

	switch (uc->config.dir) {
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

static int udma_pop_from_ring(struct udma_chan *uc, dma_addr_t *addr)
{
	struct k3_nav_ring *ring = NULL;
	int ret = -ENOENT;

	switch (uc->config.dir) {
	case DMA_DEV_TO_MEM:
		ring = uc->rflow->r_ring;
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

	switch (uc->config.dir) {
	case DMA_DEV_TO_MEM:
		ring1 = uc->rflow->fd_ring;
		ring2 = uc->rflow->r_ring;
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
		k3_nav_ringacc_ring_reset_dma(ring1, k3_nav_ringacc_ring_get_occ(ring1));
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

		if (!uc->bchan) {
			val = udma_tchanrt_read(uc->tchan, UDMA_TCHAN_RT_PEER_BCNT_REG);
			udma_tchanrt_write(uc->tchan, UDMA_TCHAN_RT_PEER_BCNT_REG, val);
		}
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

	switch (uc->config.dir) {
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

	pr_debug("%s: chan:%d dir:%s\n",
		 __func__, uc->id, udma_get_dir_text(uc->config.dir));

	/* Make sure that we clear the teardown bit, if it is set */
	udma_stop_hard(uc);

	/* Reset all counters */
	udma_reset_counters(uc);

	switch (uc->config.dir) {
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
			 udma_tchanrt_read(uc->tchan,
					   UDMA_TCHAN_RT_CTL_REG),
			 udma_tchanrt_read(uc->tchan,
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
		 __func__, uc->id, udma_get_dir_text(uc->config.dir));

	udma_reset_counters(uc);
	switch (uc->config.dir) {
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

static struct udma_rflow *__udma_reserve_rflow(struct udma_dev *ud, int id)
{
	DECLARE_BITMAP(tmp, K3_UDMA_MAX_RFLOWS);

	if (id >= 0) {
		if (test_bit(id, ud->rflow_map)) {
			dev_err(ud->dev, "rflow%d is in use\n", id);
			return ERR_PTR(-ENOENT);
		}
	} else {
		bitmap_or(tmp, ud->rflow_map, ud->rflow_map_reserved,
			  ud->rflow_cnt);

		id = find_next_zero_bit(tmp, ud->rflow_cnt, ud->rchan_cnt);
		if (id >= ud->rflow_cnt)
			return ERR_PTR(-ENOENT);
	}

	__set_bit(id, ud->rflow_map);
	return &ud->rflows[id];
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

static int udma_get_tchan(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;

	if (uc->tchan) {
		dev_dbg(ud->dev, "chan%d: already have tchan%d allocated\n",
			uc->id, uc->tchan->id);
		return 0;
	}

	uc->tchan = __udma_reserve_tchan(ud, uc->config.mapped_channel_id);
	if (IS_ERR(uc->tchan))
		return PTR_ERR(uc->tchan);

	if (ud->tflow_cnt) {
		int tflow_id;

		/* Only PKTDMA have support for tx flows */
		if (uc->config.default_flow_id >= 0)
			tflow_id = uc->config.default_flow_id;
		else
			tflow_id = uc->tchan->id;

		if (test_bit(tflow_id, ud->tflow_map)) {
			dev_err(ud->dev, "tflow%d is in use\n", tflow_id);
			__clear_bit(uc->tchan->id, ud->tchan_map);
			uc->tchan = NULL;
			return -ENOENT;
		}

		uc->tchan->tflow_id = tflow_id;
		__set_bit(tflow_id, ud->tflow_map);
	} else {
		uc->tchan->tflow_id = -1;
	}

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

	uc->rchan = __udma_reserve_rchan(ud, uc->config.mapped_channel_id);
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
		if (uc->tchan->tflow_id >= 0)
			__clear_bit(uc->tchan->tflow_id, ud->tflow_map);
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
	struct udma_tchan *tchan;
	int ring_idx, ret;

	ret = udma_get_tchan(uc);
	if (ret)
		return ret;

	tchan = uc->tchan;
	if (tchan->tflow_id >= 0)
		ring_idx = tchan->tflow_id;
	else
		ring_idx = ud->bchan_cnt + tchan->id;

	ret = k3_nav_ringacc_request_rings_pair(ud->ringacc, ring_idx, -1,
						&uc->tchan->t_ring,
						&uc->tchan->tc_ring);
	if (ret) {
		ret = -EBUSY;
		goto err_tx_ring;
	}

	memset(&ring_cfg, 0, sizeof(ring_cfg));
	ring_cfg.size = 16;
	ring_cfg.elm_size = K3_NAV_RINGACC_RING_ELSIZE_8;
	ring_cfg.mode = K3_NAV_RINGACC_RING_MODE_RING;

	ret = k3_nav_ringacc_ring_cfg(uc->tchan->t_ring, &ring_cfg);
	ret |= k3_nav_ringacc_ring_cfg(uc->tchan->tc_ring, &ring_cfg);

	if (ret)
		goto err_ringcfg;

	return 0;

err_ringcfg:
	k3_nav_ringacc_ring_free(uc->tchan->tc_ring);
	uc->tchan->tc_ring = NULL;
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

        if (uc->rflow) {
		k3_nav_ringacc_ring_free(uc->rflow->fd_ring);
		k3_nav_ringacc_ring_free(uc->rflow->r_ring);
		uc->rflow->fd_ring = NULL;
		uc->rflow->r_ring = NULL;

		udma_put_rflow(uc);
	}

	udma_put_rchan(uc);
}

static int udma_alloc_rx_resources(struct udma_chan *uc)
{
	struct k3_nav_ring_cfg ring_cfg;
	struct udma_dev *ud = uc->ud;
	struct udma_rflow *rflow;
	int fd_ring_id;
	int ret;

	ret = udma_get_rchan(uc);
	if (ret)
		return ret;

	/* For MEM_TO_MEM we don't need rflow or rings */
	if (uc->config.dir == DMA_MEM_TO_MEM)
		return 0;

	if (uc->config.default_flow_id >= 0)
		ret = udma_get_rflow(uc, uc->config.default_flow_id);
	else
		ret = udma_get_rflow(uc, uc->rchan->id);

	if (ret) {
		ret = -EBUSY;
		goto err_rflow;
	}

	rflow = uc->rflow;
	if (ud->tflow_cnt) {
		fd_ring_id = ud->tflow_cnt + rflow->id;
	} else {
		fd_ring_id = ud->bchan_cnt + ud->tchan_cnt + ud->echan_cnt +
			uc->rchan->id;
	}

	ret = k3_nav_ringacc_request_rings_pair(ud->ringacc, fd_ring_id, -1,
						&rflow->fd_ring, &rflow->r_ring);
	if (ret) {
		ret = -EBUSY;
		goto err_rx_ring;
	}

	memset(&ring_cfg, 0, sizeof(ring_cfg));
	ring_cfg.size = 16;
	ring_cfg.elm_size = K3_NAV_RINGACC_RING_ELSIZE_8;
	ring_cfg.mode = K3_NAV_RINGACC_RING_MODE_RING;

	ret = k3_nav_ringacc_ring_cfg(rflow->fd_ring, &ring_cfg);
	ret |= k3_nav_ringacc_ring_cfg(rflow->r_ring, &ring_cfg);
	if (ret)
		goto err_ringcfg;

	return 0;

err_ringcfg:
	k3_nav_ringacc_ring_free(rflow->r_ring);
	rflow->r_ring = NULL;
	k3_nav_ringacc_ring_free(rflow->fd_ring);
	rflow->fd_ring = NULL;
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
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;
	u32 mode;
	int ret;

	if (uc->config.pkt_mode)
		mode = TI_SCI_RM_UDMAP_CHAN_TYPE_PKT_PBRR;
	else
		mode = TI_SCI_RM_UDMAP_CHAN_TYPE_3RDP_BCOPY_PBRR;

	req.valid_params = TI_SCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID;
	req.nav_id = tisci_rm->tisci_dev_id;
	req.index = uc->tchan->id;
	req.tx_chan_type = mode;
	if (uc->config.dir == DMA_MEM_TO_MEM)
		req.tx_fetch_size = sizeof(struct cppi5_desc_hdr_t) >> 2;
	else
		req.tx_fetch_size = cppi5_hdesc_calc_size(uc->config.needs_epib,
							  uc->config.psd_size,
							  0) >> 2;
	req.txcq_qnum = tc_ring;

	ret = tisci_rm->tisci_udmap_ops->tx_ch_cfg(tisci_rm->tisci, &req);
	if (ret) {
		dev_err(ud->dev, "tisci tx alloc failed %d\n", ret);
		return ret;
	}

	/*
	 * Above TI SCI call handles firewall configuration, cfg
	 * register configuration still has to be done locally in
	 * absence of RM services.
	 */
	if (IS_ENABLED(CONFIG_K3_DM_FW))
		udma_alloc_tchan_raw(uc);

	return 0;
}

static int udma_alloc_rchan_sci_req(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	int fd_ring = k3_nav_ringacc_get_ring_id(uc->rflow->fd_ring);
	int rx_ring = k3_nav_ringacc_get_ring_id(uc->rflow->r_ring);
	int tc_ring = k3_nav_ringacc_get_ring_id(uc->tchan->tc_ring);
	struct ti_sci_msg_rm_udmap_rx_ch_cfg req = { 0 };
	struct ti_sci_msg_rm_udmap_flow_cfg flow_req = { 0 };
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;
	u32 mode;
	int ret;

	if (uc->config.pkt_mode)
		mode = TI_SCI_RM_UDMAP_CHAN_TYPE_PKT_PBRR;
	else
		mode = TI_SCI_RM_UDMAP_CHAN_TYPE_3RDP_BCOPY_PBRR;

	req.valid_params = TI_SCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID |
			TI_SCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID;
	req.nav_id = tisci_rm->tisci_dev_id;
	req.index = uc->rchan->id;
	req.rx_chan_type = mode;
	if (uc->config.dir == DMA_MEM_TO_MEM) {
		req.rx_fetch_size = sizeof(struct cppi5_desc_hdr_t) >> 2;
		req.rxcq_qnum = tc_ring;
	} else {
		req.rx_fetch_size = cppi5_hdesc_calc_size(uc->config.needs_epib,
							  uc->config.psd_size,
							  0) >> 2;
		req.rxcq_qnum = rx_ring;
	}
	if (ud->match_data->type == DMA_TYPE_UDMA &&
	    uc->rflow->id != uc->rchan->id &&
	    uc->config.dir != DMA_MEM_TO_MEM) {
		req.flowid_start = uc->rflow->id;
		req.flowid_cnt = 1;
		req.valid_params |= TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_START_VALID |
				    TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID;
	}

	ret = tisci_rm->tisci_udmap_ops->rx_ch_cfg(tisci_rm->tisci, &req);
	if (ret) {
		dev_err(ud->dev, "tisci rx %u cfg failed %d\n",
			uc->rchan->id, ret);
		return ret;
	}
	if (uc->config.dir == DMA_MEM_TO_MEM)
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

	flow_req.nav_id = tisci_rm->tisci_dev_id;
	flow_req.flow_index = uc->rflow->id;

	if (uc->config.needs_epib)
		flow_req.rx_einfo_present = 1;
	else
		flow_req.rx_einfo_present = 0;

	if (uc->config.psd_size)
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

	ret = tisci_rm->tisci_udmap_ops->rx_flow_cfg(tisci_rm->tisci,
						     &flow_req);
	if (ret) {
		dev_err(ud->dev, "tisci rx %u flow %u cfg failed %d\n",
			uc->rchan->id, uc->rflow->id, ret);
		return ret;
	}

	/*
	 * Above TI SCI call handles firewall configuration, cfg
	 * register configuration still has to be done locally in
	 * absence of RM services.
	 */
	if (IS_ENABLED(CONFIG_K3_DM_FW))
		udma_alloc_rchan_raw(uc);

	return 0;
}

static int udma_alloc_chan_resources(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	int ret;

	pr_debug("%s: chan:%d as %s\n",
		 __func__, uc->id, udma_get_dir_text(uc->config.dir));

	switch (uc->config.dir) {
	case DMA_MEM_TO_MEM:
		/* Non synchronized - mem to mem type of transfer */
		uc->config.pkt_mode = false;
		ret = udma_get_chan_pair(uc);
		if (ret)
			return ret;

		ret = udma_alloc_tx_resources(uc);
		if (ret)
			goto err_free_res;

		ret = udma_alloc_rx_resources(uc);
		if (ret)
			goto err_free_res;

		uc->config.src_thread = ud->psil_base + uc->tchan->id;
		uc->config.dst_thread = (ud->psil_base + uc->rchan->id) | 0x8000;
		break;
	case DMA_MEM_TO_DEV:
		/* Slave transfer synchronized - mem to dev (TX) trasnfer */
		ret = udma_alloc_tx_resources(uc);
		if (ret)
			goto err_free_res;

		uc->config.src_thread = ud->psil_base + uc->tchan->id;
		uc->config.dst_thread = uc->config.remote_thread_id;
		uc->config.dst_thread |= 0x8000;

		break;
	case DMA_DEV_TO_MEM:
		/* Slave transfer synchronized - dev to mem (RX) trasnfer */
		ret = udma_alloc_rx_resources(uc);
		if (ret)
			goto err_free_res;

		uc->config.src_thread = uc->config.remote_thread_id;
		uc->config.dst_thread = (ud->psil_base + uc->rchan->id) | 0x8000;

		break;
	default:
		/* Can not happen */
		pr_debug("%s: chan:%d invalid direction (%u)\n",
			 __func__, uc->id, uc->config.dir);
		return -EINVAL;
	}

	/* We have channel indexes and rings */
	if (uc->config.dir == DMA_MEM_TO_MEM) {
		ret = udma_alloc_tchan_sci_req(uc);
		if (ret)
			goto err_free_res;

		ret = udma_alloc_rchan_sci_req(uc);
		if (ret)
			goto err_free_res;
	} else {
		/* Slave transfer */
		if (uc->config.dir == DMA_MEM_TO_DEV) {
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
	ret = udma_navss_psil_pair(ud, uc->config.src_thread, uc->config.dst_thread);
	if (ret) {
		dev_err(ud->dev, "k3_nav_psil_request_link fail\n");
		goto err_free_res;
	}

	return 0;

err_free_res:
	udma_free_tx_resources(uc);
	udma_free_rx_resources(uc);
	uc->config.remote_thread_id = -1;
	return ret;
}

static void udma_free_chan_resources(struct udma_chan *uc)
{
	/* Hard reset UDMA channel */
	udma_stop_hard(uc);
	udma_reset_counters(uc);

	/* Release PSI-L pairing */
	udma_navss_psil_unpair(uc->ud, uc->config.src_thread, uc->config.dst_thread);

	/* Reset the rings for a new start */
	udma_reset_rings(uc);
	udma_free_tx_resources(uc);
	udma_free_rx_resources(uc);

	uc->config.remote_thread_id = -1;
	uc->config.dir = DMA_MEM_TO_MEM;
}

static const char * const range_names[] = {
	[RM_RANGE_BCHAN] = "ti,sci-rm-range-bchan",
	[RM_RANGE_TCHAN] = "ti,sci-rm-range-tchan",
	[RM_RANGE_RCHAN] = "ti,sci-rm-range-rchan",
	[RM_RANGE_RFLOW] = "ti,sci-rm-range-rflow",
	[RM_RANGE_TFLOW] = "ti,sci-rm-range-tflow",
};

static int udma_get_mmrs(struct udevice *dev)
{
	struct udma_dev *ud = dev_get_priv(dev);
	u32 cap2, cap3, cap4;
	int i;

	ud->mmrs[MMR_GCFG] = dev_read_addr_name_ptr(dev, mmr_names[MMR_GCFG]);
	if (!ud->mmrs[MMR_GCFG])
		return -EINVAL;

	cap2 = udma_read(ud->mmrs[MMR_GCFG], 0x28);
	cap3 = udma_read(ud->mmrs[MMR_GCFG], 0x2c);

	switch (ud->match_data->type) {
	case DMA_TYPE_UDMA:
		ud->rflow_cnt = cap3 & 0x3fff;
		ud->tchan_cnt = cap2 & 0x1ff;
		ud->echan_cnt = (cap2 >> 9) & 0x1ff;
		ud->rchan_cnt = (cap2 >> 18) & 0x1ff;
		break;
	case DMA_TYPE_BCDMA:
		ud->bchan_cnt = cap2 & 0x1ff;
		ud->tchan_cnt = (cap2 >> 9) & 0x1ff;
		ud->rchan_cnt = (cap2 >> 18) & 0x1ff;
		break;
	case DMA_TYPE_PKTDMA:
		cap4 = udma_read(ud->mmrs[MMR_GCFG], 0x30);
		ud->tchan_cnt = cap2 & 0x1ff;
		ud->rchan_cnt = (cap2 >> 18) & 0x1ff;
		ud->rflow_cnt = cap3 & 0x3fff;
		ud->tflow_cnt = cap4 & 0x3fff;
		break;
	default:
		return -EINVAL;
	}

	for (i = 1; i < MMR_LAST; i++) {
		if (i == MMR_BCHANRT && ud->bchan_cnt == 0)
			continue;
		if (i == MMR_TCHANRT && ud->tchan_cnt == 0)
			continue;
		if (i == MMR_RCHANRT && ud->rchan_cnt == 0)
			continue;

		ud->mmrs[i] = dev_read_addr_name_ptr(dev, mmr_names[i]);
		if (!ud->mmrs[i])
			return -EINVAL;
	}

	return 0;
}

static int udma_setup_resources(struct udma_dev *ud)
{
	struct udevice *dev = ud->dev;
	int i;
	struct ti_sci_resource_desc *rm_desc;
	struct ti_sci_resource *rm_res;
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;

	ud->tchan_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->tchan_cnt),
					   sizeof(unsigned long), GFP_KERNEL);
	ud->tchans = devm_kcalloc(dev, ud->tchan_cnt, sizeof(*ud->tchans),
				  GFP_KERNEL);
	ud->rchan_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->rchan_cnt),
					   sizeof(unsigned long), GFP_KERNEL);
	ud->rchans = devm_kcalloc(dev, ud->rchan_cnt, sizeof(*ud->rchans),
				  GFP_KERNEL);
	ud->rflow_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->rflow_cnt),
					   sizeof(unsigned long), GFP_KERNEL);
	ud->rflow_map_reserved = devm_kcalloc(dev, BITS_TO_LONGS(ud->rflow_cnt),
					      sizeof(unsigned long),
					      GFP_KERNEL);
	ud->rflows = devm_kcalloc(dev, ud->rflow_cnt, sizeof(*ud->rflows),
				  GFP_KERNEL);

	if (!ud->tchan_map || !ud->rchan_map || !ud->rflow_map ||
	    !ud->rflow_map_reserved || !ud->tchans || !ud->rchans ||
	    !ud->rflows)
		return -ENOMEM;

	/*
	 * RX flows with the same Ids as RX channels are reserved to be used
	 * as default flows if remote HW can't generate flow_ids. Those
	 * RX flows can be requested only explicitly by id.
	 */
	bitmap_set(ud->rflow_map_reserved, 0, ud->rchan_cnt);

	/* Get resource ranges from tisci */
	for (i = 0; i < RM_RANGE_LAST; i++) {
		if (i == RM_RANGE_BCHAN || i == RM_RANGE_TFLOW)
			continue;

		tisci_rm->rm_ranges[i] =
			devm_ti_sci_get_of_resource(tisci_rm->tisci, dev,
						    tisci_rm->tisci_dev_id,
						    (char *)range_names[i]);
	}

	/* tchan ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_TCHAN];
	if (IS_ERR(rm_res)) {
		bitmap_zero(ud->tchan_map, ud->tchan_cnt);
	} else {
		bitmap_fill(ud->tchan_map, ud->tchan_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->tchan_map, rm_desc->start,
				     rm_desc->num);
		}
	}

	/* rchan and matching default flow ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_RCHAN];
	if (IS_ERR(rm_res)) {
		bitmap_zero(ud->rchan_map, ud->rchan_cnt);
		bitmap_zero(ud->rflow_map, ud->rchan_cnt);
	} else {
		bitmap_fill(ud->rchan_map, ud->rchan_cnt);
		bitmap_fill(ud->rflow_map, ud->rchan_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->rchan_map, rm_desc->start,
				     rm_desc->num);
			bitmap_clear(ud->rflow_map, rm_desc->start,
				     rm_desc->num);
		}
	}

	/* GP rflow ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_RFLOW];
	if (IS_ERR(rm_res)) {
		bitmap_clear(ud->rflow_map, ud->rchan_cnt,
			     ud->rflow_cnt - ud->rchan_cnt);
	} else {
		bitmap_set(ud->rflow_map, ud->rchan_cnt,
			   ud->rflow_cnt - ud->rchan_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->rflow_map, rm_desc->start,
				     rm_desc->num);
		}
	}

	return 0;
}

static int bcdma_setup_resources(struct udma_dev *ud)
{
	int i;
	struct udevice *dev = ud->dev;
	struct ti_sci_resource_desc *rm_desc;
	struct ti_sci_resource *rm_res;
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;

	ud->bchan_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->bchan_cnt),
					   sizeof(unsigned long), GFP_KERNEL);
	ud->bchans = devm_kcalloc(dev, ud->bchan_cnt, sizeof(*ud->bchans),
				  GFP_KERNEL);
	ud->tchan_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->tchan_cnt),
					   sizeof(unsigned long), GFP_KERNEL);
	ud->tchans = devm_kcalloc(dev, ud->tchan_cnt, sizeof(*ud->tchans),
				  GFP_KERNEL);
	ud->rchan_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->rchan_cnt),
					   sizeof(unsigned long), GFP_KERNEL);
	ud->rchans = devm_kcalloc(dev, ud->rchan_cnt, sizeof(*ud->rchans),
				  GFP_KERNEL);
	ud->rflows = devm_kcalloc(dev, ud->rchan_cnt, sizeof(*ud->rflows),
				  GFP_KERNEL);

	if (!ud->bchan_map || !ud->tchan_map || !ud->rchan_map ||
	    !ud->bchans || !ud->tchans || !ud->rchans ||
	    !ud->rflows)
		return -ENOMEM;

	/* Get resource ranges from tisci */
	for (i = 0; i < RM_RANGE_LAST; i++) {
		if (i == RM_RANGE_RFLOW || i == RM_RANGE_TFLOW)
			continue;

		tisci_rm->rm_ranges[i] =
			devm_ti_sci_get_of_resource(tisci_rm->tisci, dev,
						    tisci_rm->tisci_dev_id,
						    (char *)range_names[i]);
	}

	/* bchan ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_BCHAN];
	if (IS_ERR(rm_res)) {
		bitmap_zero(ud->bchan_map, ud->bchan_cnt);
	} else {
		bitmap_fill(ud->bchan_map, ud->bchan_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->bchan_map, rm_desc->start,
				     rm_desc->num);
			dev_dbg(dev, "ti-sci-res: bchan: %d:%d\n",
				rm_desc->start, rm_desc->num);
		}
	}

	/* tchan ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_TCHAN];
	if (IS_ERR(rm_res)) {
		bitmap_zero(ud->tchan_map, ud->tchan_cnt);
	} else {
		bitmap_fill(ud->tchan_map, ud->tchan_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->tchan_map, rm_desc->start,
				     rm_desc->num);
			dev_dbg(dev, "ti-sci-res: tchan: %d:%d\n",
				rm_desc->start, rm_desc->num);
		}
	}

	/* rchan ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_RCHAN];
	if (IS_ERR(rm_res)) {
		bitmap_zero(ud->rchan_map, ud->rchan_cnt);
	} else {
		bitmap_fill(ud->rchan_map, ud->rchan_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->rchan_map, rm_desc->start,
				     rm_desc->num);
			dev_dbg(dev, "ti-sci-res: rchan: %d:%d\n",
				rm_desc->start, rm_desc->num);
		}
	}

	return 0;
}

static int pktdma_setup_resources(struct udma_dev *ud)
{
	int i;
	struct udevice *dev = ud->dev;
	struct ti_sci_resource *rm_res;
	struct ti_sci_resource_desc *rm_desc;
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;

	ud->tchan_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->tchan_cnt),
					   sizeof(unsigned long), GFP_KERNEL);
	ud->tchans = devm_kcalloc(dev, ud->tchan_cnt, sizeof(*ud->tchans),
				  GFP_KERNEL);
	ud->rchan_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->rchan_cnt),
					   sizeof(unsigned long), GFP_KERNEL);
	ud->rchans = devm_kcalloc(dev, ud->rchan_cnt, sizeof(*ud->rchans),
				  GFP_KERNEL);
	ud->rflow_map = devm_kcalloc(dev, BITS_TO_LONGS(ud->rflow_cnt),
				     sizeof(unsigned long),
				     GFP_KERNEL);
	ud->rflows = devm_kcalloc(dev, ud->rflow_cnt, sizeof(*ud->rflows),
				  GFP_KERNEL);
	ud->tflow_map = devm_kmalloc_array(dev, BITS_TO_LONGS(ud->tflow_cnt),
					   sizeof(unsigned long), GFP_KERNEL);

	if (!ud->tchan_map || !ud->rchan_map || !ud->tflow_map || !ud->tchans ||
	    !ud->rchans || !ud->rflows || !ud->rflow_map)
		return -ENOMEM;

	/* Get resource ranges from tisci */
	for (i = 0; i < RM_RANGE_LAST; i++) {
		if (i == RM_RANGE_BCHAN)
			continue;

		tisci_rm->rm_ranges[i] =
			devm_ti_sci_get_of_resource(tisci_rm->tisci, dev,
						    tisci_rm->tisci_dev_id,
						    (char *)range_names[i]);
	}

	/* tchan ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_TCHAN];
	if (IS_ERR(rm_res)) {
		bitmap_zero(ud->tchan_map, ud->tchan_cnt);
	} else {
		bitmap_fill(ud->tchan_map, ud->tchan_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->tchan_map, rm_desc->start,
				     rm_desc->num);
			dev_dbg(dev, "ti-sci-res: tchan: %d:%d\n",
				rm_desc->start, rm_desc->num);
		}
	}

	/* rchan ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_RCHAN];
	if (IS_ERR(rm_res)) {
		bitmap_zero(ud->rchan_map, ud->rchan_cnt);
	} else {
		bitmap_fill(ud->rchan_map, ud->rchan_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->rchan_map, rm_desc->start,
				     rm_desc->num);
			dev_dbg(dev, "ti-sci-res: rchan: %d:%d\n",
				rm_desc->start, rm_desc->num);
		}
	}

	/* rflow ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_RFLOW];
	if (IS_ERR(rm_res)) {
		/* all rflows are assigned exclusively to Linux */
		bitmap_zero(ud->rflow_map, ud->rflow_cnt);
	} else {
		bitmap_fill(ud->rflow_map, ud->rflow_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->rflow_map, rm_desc->start,
				     rm_desc->num);
			dev_dbg(dev, "ti-sci-res: rflow: %d:%d\n",
				rm_desc->start, rm_desc->num);
		}
	}

	/* tflow ranges */
	rm_res = tisci_rm->rm_ranges[RM_RANGE_TFLOW];
	if (IS_ERR(rm_res)) {
		/* all tflows are assigned exclusively to Linux */
		bitmap_zero(ud->tflow_map, ud->tflow_cnt);
	} else {
		bitmap_fill(ud->tflow_map, ud->tflow_cnt);
		for (i = 0; i < rm_res->sets; i++) {
			rm_desc = &rm_res->desc[i];
			bitmap_clear(ud->tflow_map, rm_desc->start,
				     rm_desc->num);
			dev_dbg(dev, "ti-sci-res: tflow: %d:%d\n",
				rm_desc->start, rm_desc->num);
		}
	}

	return 0;
}

static int setup_resources(struct udma_dev *ud)
{
	struct udevice *dev = ud->dev;
	int ch_count, ret;

	switch (ud->match_data->type) {
	case DMA_TYPE_UDMA:
		ret = udma_setup_resources(ud);
		break;
	case DMA_TYPE_BCDMA:
		ret = bcdma_setup_resources(ud);
		break;
	case DMA_TYPE_PKTDMA:
		ret = pktdma_setup_resources(ud);
		break;
	default:
		return -EINVAL;
	}

	if (ret)
		return ret;

	ch_count  = ud->bchan_cnt + ud->tchan_cnt + ud->rchan_cnt;
	if (ud->bchan_cnt)
		ch_count -= bitmap_weight(ud->bchan_map, ud->bchan_cnt);
	ch_count -= bitmap_weight(ud->tchan_map, ud->tchan_cnt);
	ch_count -= bitmap_weight(ud->rchan_map, ud->rchan_cnt);
	if (!ch_count)
		return -ENODEV;

	ud->channels = devm_kcalloc(dev, ch_count, sizeof(*ud->channels),
				    GFP_KERNEL);
	if (!ud->channels)
		return -ENOMEM;

	switch (ud->match_data->type) {
	case DMA_TYPE_UDMA:
		dev_dbg(dev,
			"Channels: %d (tchan: %u, rchan: %u, gp-rflow: %u)\n",
			ch_count,
			ud->tchan_cnt - bitmap_weight(ud->tchan_map,
						      ud->tchan_cnt),
			ud->rchan_cnt - bitmap_weight(ud->rchan_map,
						      ud->rchan_cnt),
			ud->rflow_cnt - bitmap_weight(ud->rflow_map,
						      ud->rflow_cnt));
		break;
	case DMA_TYPE_BCDMA:
		dev_dbg(dev,
			"Channels: %d (bchan: %u, tchan: %u, rchan: %u)\n",
			ch_count,
			ud->bchan_cnt - bitmap_weight(ud->bchan_map,
						      ud->bchan_cnt),
			ud->tchan_cnt - bitmap_weight(ud->tchan_map,
						      ud->tchan_cnt),
			ud->rchan_cnt - bitmap_weight(ud->rchan_map,
						      ud->rchan_cnt));
		break;
	case DMA_TYPE_PKTDMA:
		dev_dbg(dev,
			"Channels: %d (tchan: %u, rchan: %u)\n",
			ch_count,
			ud->tchan_cnt - bitmap_weight(ud->tchan_map,
						      ud->tchan_cnt),
			ud->rchan_cnt - bitmap_weight(ud->rchan_map,
						      ud->rchan_cnt));
		break;
	default:
		break;
	}

	return ch_count;
}

static int udma_probe(struct udevice *dev)
{
	struct dma_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct udma_dev *ud = dev_get_priv(dev);
	int i, ret;
	struct udevice *tmp;
	struct udevice *tisci_dev = NULL;
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;
	ofnode navss_ofnode = ofnode_get_parent(dev_ofnode(dev));


	ud->match_data = (void *)dev_get_driver_data(dev);
	ret = udma_get_mmrs(dev);
	if (ret)
		return ret;

	ud->psil_base = ud->match_data->psil_base;

	ret = uclass_get_device_by_phandle(UCLASS_FIRMWARE, dev,
					   "ti,sci", &tisci_dev);
	if (ret) {
		debug("Failed to get TISCI phandle (%d)\n", ret);
		tisci_rm->tisci = NULL;
		return -EINVAL;
	}
	tisci_rm->tisci = (struct ti_sci_handle *)
			  (ti_sci_get_handle_from_sysfw(tisci_dev));

	tisci_rm->tisci_dev_id = -1;
	ret = dev_read_u32(dev, "ti,sci-dev-id", &tisci_rm->tisci_dev_id);
	if (ret) {
		dev_err(dev, "ti,sci-dev-id read failure %d\n", ret);
		return ret;
	}

	tisci_rm->tisci_navss_dev_id = -1;
	ret = ofnode_read_u32(navss_ofnode, "ti,sci-dev-id",
			      &tisci_rm->tisci_navss_dev_id);
	if (ret) {
		dev_err(dev, "navss sci-dev-id read failure %d\n", ret);
		return ret;
	}

	tisci_rm->tisci_udmap_ops = &tisci_rm->tisci->ops.rm_udmap_ops;
	tisci_rm->tisci_psil_ops = &tisci_rm->tisci->ops.rm_psil_ops;

	if (ud->match_data->type == DMA_TYPE_UDMA) {
		ret = uclass_get_device_by_phandle(UCLASS_MISC, dev,
						   "ti,ringacc", &tmp);
		ud->ringacc = dev_get_priv(tmp);
	} else {
		struct k3_ringacc_init_data ring_init_data;

		ring_init_data.tisci = ud->tisci_rm.tisci;
		ring_init_data.tisci_dev_id = ud->tisci_rm.tisci_dev_id;
		if (ud->match_data->type == DMA_TYPE_BCDMA) {
			ring_init_data.num_rings = ud->bchan_cnt +
						   ud->tchan_cnt +
						   ud->rchan_cnt;
		} else {
			ring_init_data.num_rings = ud->rflow_cnt +
						   ud->tflow_cnt;
		}

		ud->ringacc = k3_ringacc_dmarings_init(dev, &ring_init_data);
	}
	if (IS_ERR(ud->ringacc))
		return PTR_ERR(ud->ringacc);

	ud->dev = dev;
	ud->ch_count = setup_resources(ud);
	if (ud->ch_count <= 0)
		return ud->ch_count;

	for (i = 0; i < ud->bchan_cnt; i++) {
		struct udma_bchan *bchan = &ud->bchans[i];

		bchan->id = i;
		bchan->reg_rt = ud->mmrs[MMR_BCHANRT] + i * 0x1000;
	}

	for (i = 0; i < ud->tchan_cnt; i++) {
		struct udma_tchan *tchan = &ud->tchans[i];

		tchan->id = i;
		tchan->reg_chan = ud->mmrs[MMR_TCHAN] + UDMA_CH_100(i);
		tchan->reg_rt = ud->mmrs[MMR_TCHANRT] + UDMA_CH_1000(i);
	}

	for (i = 0; i < ud->rchan_cnt; i++) {
		struct udma_rchan *rchan = &ud->rchans[i];

		rchan->id = i;
		rchan->reg_chan = ud->mmrs[MMR_RCHAN] + UDMA_CH_100(i);
		rchan->reg_rt = ud->mmrs[MMR_RCHANRT] + UDMA_CH_1000(i);
	}

	for (i = 0; i < ud->rflow_cnt; i++) {
		struct udma_rflow *rflow = &ud->rflows[i];

		rflow->id = i;
		rflow->reg_rflow = ud->mmrs[MMR_RFLOW] + UDMA_CH_40(i);
	}

	for (i = 0; i < ud->ch_count; i++) {
		struct udma_chan *uc = &ud->channels[i];

		uc->ud = ud;
		uc->id = i;
		uc->config.remote_thread_id = -1;
		uc->bchan = NULL;
		uc->tchan = NULL;
		uc->rchan = NULL;
		uc->config.mapped_channel_id = -1;
		uc->config.default_flow_id = -1;
		uc->config.dir = DMA_MEM_TO_MEM;
		sprintf(uc->name, "UDMA chan%d\n", i);
		if (!i)
			uc->in_use = true;
	}

	pr_debug("%s(rev: 0x%08x) CAP0-3: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
		 dev->name,
		 udma_read(ud->mmrs[MMR_GCFG], 0),
		 udma_read(ud->mmrs[MMR_GCFG], 0x20),
		 udma_read(ud->mmrs[MMR_GCFG], 0x24),
		 udma_read(ud->mmrs[MMR_GCFG], 0x28),
		 udma_read(ud->mmrs[MMR_GCFG], 0x2c));

	uc_priv->supported = DMA_SUPPORTS_MEM_TO_MEM | DMA_SUPPORTS_MEM_TO_DEV;

	return ret;
}

static int udma_push_to_ring(struct k3_nav_ring *ring, void *elem)
{
	u64 addr = 0;

	memcpy(&addr, &elem, sizeof(elem));
	return k3_nav_ringacc_ring_push(ring, &addr);
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

	flush_dcache_range((unsigned long)tr_desc,
			   ALIGN((unsigned long)tr_desc + desc_size,
				 ARCH_DMA_MINALIGN));

	udma_push_to_ring(uc->tchan->t_ring, tr_desc);

	return 0;
}

#define TISCI_BCDMA_BCHAN_VALID_PARAMS (			\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_EXTENDED_CH_TYPE_VALID)

#define TISCI_BCDMA_TCHAN_VALID_PARAMS (			\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_SUPR_TDPKT_VALID)

#define TISCI_BCDMA_RCHAN_VALID_PARAMS (			\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID)

#define TISCI_UDMA_TCHAN_VALID_PARAMS (				\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_EINFO_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_PSWORDS_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID |		\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_SUPR_TDPKT_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID |		\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID |		\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID)

#define TISCI_UDMA_RCHAN_VALID_PARAMS (				\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID |		\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID |		\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID |		\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_SHORT_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_LONG_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_START_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID |	\
	TI_SCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID)

static int bcdma_tisci_m2m_channel_config(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;
	const struct ti_sci_rm_udmap_ops *tisci_ops = tisci_rm->tisci_udmap_ops;
	struct ti_sci_msg_rm_udmap_tx_ch_cfg req_tx = { 0 };
	struct udma_bchan *bchan = uc->bchan;
	int ret = 0;

	req_tx.valid_params = TISCI_BCDMA_BCHAN_VALID_PARAMS;
	req_tx.nav_id = tisci_rm->tisci_dev_id;
	req_tx.extended_ch_type = TI_SCI_RM_BCDMA_EXTENDED_CH_TYPE_BCHAN;
	req_tx.index = bchan->id;

	ret = tisci_ops->tx_ch_cfg(tisci_rm->tisci, &req_tx);
	if (ret)
		dev_err(ud->dev, "bchan%d cfg failed %d\n", bchan->id, ret);

	return ret;
}

static struct udma_bchan *__bcdma_reserve_bchan(struct udma_dev *ud, int id)
{
	if (id >= 0) {
		if (test_bit(id, ud->bchan_map)) {
			dev_err(ud->dev, "bchan%d is in use\n", id);
			return ERR_PTR(-ENOENT);
		}
	} else {
		id = find_next_zero_bit(ud->bchan_map, ud->bchan_cnt, 0);
		if (id == ud->bchan_cnt)
			return ERR_PTR(-ENOENT);
	}
	__set_bit(id, ud->bchan_map);
	return &ud->bchans[id];
}

static int bcdma_get_bchan(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;

	if (uc->bchan) {
		dev_err(ud->dev, "chan%d: already have bchan%d allocated\n",
			uc->id, uc->bchan->id);
		return 0;
	}

	uc->bchan = __bcdma_reserve_bchan(ud, -1);
	if (IS_ERR(uc->bchan))
		return PTR_ERR(uc->bchan);

	uc->tchan = uc->bchan;

	return 0;
}

static void bcdma_put_bchan(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;

	if (uc->bchan) {
		dev_dbg(ud->dev, "chan%d: put bchan%d\n", uc->id,
			uc->bchan->id);
		__clear_bit(uc->bchan->id, ud->bchan_map);
		uc->bchan = NULL;
		uc->tchan = NULL;
	}
}

static void bcdma_free_bchan_resources(struct udma_chan *uc)
{
	if (!uc->bchan)
		return;

	k3_nav_ringacc_ring_free(uc->bchan->tc_ring);
	k3_nav_ringacc_ring_free(uc->bchan->t_ring);
	uc->bchan->tc_ring = NULL;
	uc->bchan->t_ring = NULL;

	bcdma_put_bchan(uc);
}

static int bcdma_alloc_bchan_resources(struct udma_chan *uc)
{
	struct k3_nav_ring_cfg ring_cfg;
	struct udma_dev *ud = uc->ud;
	int ret;

	ret = bcdma_get_bchan(uc);
	if (ret)
		return ret;

	ret = k3_nav_ringacc_request_rings_pair(ud->ringacc, uc->bchan->id, -1,
						&uc->bchan->t_ring,
						&uc->bchan->tc_ring);
	if (ret) {
		ret = -EBUSY;
		goto err_ring;
	}

	memset(&ring_cfg, 0, sizeof(ring_cfg));
	ring_cfg.size = 16;
	ring_cfg.elm_size = K3_NAV_RINGACC_RING_ELSIZE_8;
	ring_cfg.mode = K3_NAV_RINGACC_RING_MODE_RING;

	ret = k3_nav_ringacc_ring_cfg(uc->bchan->t_ring, &ring_cfg);
	if (ret)
		goto err_ringcfg;

	return 0;

err_ringcfg:
	k3_nav_ringacc_ring_free(uc->bchan->tc_ring);
	uc->bchan->tc_ring = NULL;
	k3_nav_ringacc_ring_free(uc->bchan->t_ring);
	uc->bchan->t_ring = NULL;
err_ring:
	bcdma_put_bchan(uc);

	return ret;
}

static int bcdma_tisci_tx_channel_config(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;
	const struct ti_sci_rm_udmap_ops *tisci_ops = tisci_rm->tisci_udmap_ops;
	struct udma_tchan *tchan = uc->tchan;
	struct ti_sci_msg_rm_udmap_tx_ch_cfg req_tx = { 0 };
	int ret = 0;

	req_tx.valid_params = TISCI_BCDMA_TCHAN_VALID_PARAMS;
	req_tx.nav_id = tisci_rm->tisci_dev_id;
	req_tx.index = tchan->id;
	req_tx.tx_supr_tdpkt = uc->config.notdpkt;
	if (uc->config.ep_type == PSIL_EP_PDMA_XY &&
	    ud->match_data->flags & UDMA_FLAG_TDTYPE) {
		/* wait for peer to complete the teardown for PDMAs */
		req_tx.valid_params |=
				TI_SCI_MSG_VALUE_RM_UDMAP_CH_TX_TDTYPE_VALID;
		req_tx.tx_tdtype = 1;
	}

	ret = tisci_ops->tx_ch_cfg(tisci_rm->tisci, &req_tx);
	if (ret)
		dev_err(ud->dev, "tchan%d cfg failed %d\n", tchan->id, ret);

	return ret;
}

#define pktdma_tisci_tx_channel_config bcdma_tisci_tx_channel_config

static int pktdma_tisci_rx_channel_config(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	struct udma_tisci_rm *tisci_rm = &ud->tisci_rm;
	const struct ti_sci_rm_udmap_ops *tisci_ops = tisci_rm->tisci_udmap_ops;
	struct ti_sci_msg_rm_udmap_rx_ch_cfg req_rx = { 0 };
	struct ti_sci_msg_rm_udmap_flow_cfg flow_req = { 0 };
	int ret = 0;

	req_rx.valid_params = TISCI_BCDMA_RCHAN_VALID_PARAMS;
	req_rx.nav_id = tisci_rm->tisci_dev_id;
	req_rx.index = uc->rchan->id;

	ret = tisci_ops->rx_ch_cfg(tisci_rm->tisci, &req_rx);
	if (ret) {
		dev_err(ud->dev, "rchan%d cfg failed %d\n", uc->rchan->id, ret);
		return ret;
	}

	flow_req.valid_params =
		TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_EINFO_PRESENT_VALID |
		TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_PSINFO_PRESENT_VALID |
		TI_SCI_MSG_VALUE_RM_UDMAP_FLOW_ERROR_HANDLING_VALID;

	flow_req.nav_id = tisci_rm->tisci_dev_id;
	flow_req.flow_index = uc->rflow->id;

	if (uc->config.needs_epib)
		flow_req.rx_einfo_present = 1;
	else
		flow_req.rx_einfo_present = 0;
	if (uc->config.psd_size)
		flow_req.rx_psinfo_present = 1;
	else
		flow_req.rx_psinfo_present = 0;
	flow_req.rx_error_handling = 0;

	ret = tisci_ops->rx_flow_cfg(tisci_rm->tisci, &flow_req);

	if (ret)
		dev_err(ud->dev, "flow%d config failed: %d\n", uc->rflow->id,
			ret);

	return ret;
}

static int bcdma_alloc_chan_resources(struct udma_chan *uc)
{
	int ret;

	uc->config.pkt_mode = false;

	switch (uc->config.dir) {
	case DMA_MEM_TO_MEM:
		/* Non synchronized - mem to mem type of transfer */
		dev_dbg(uc->ud->dev, "%s: chan%d as MEM-to-MEM\n", __func__,
			uc->id);

		ret = bcdma_alloc_bchan_resources(uc);
		if (ret)
			return ret;

		ret = bcdma_tisci_m2m_channel_config(uc);
		break;
	default:
		/* Can not happen */
		dev_err(uc->ud->dev, "%s: chan%d invalid direction (%u)\n",
			__func__, uc->id, uc->config.dir);
		return -EINVAL;
	}

	/* check if the channel configuration was successful */
	if (ret)
		goto err_res_free;

	if (udma_is_chan_running(uc)) {
		dev_warn(uc->ud->dev, "chan%d: is running!\n", uc->id);
		udma_stop(uc);
		if (udma_is_chan_running(uc)) {
			dev_err(uc->ud->dev, "chan%d: won't stop!\n", uc->id);
			goto err_res_free;
		}
	}

	udma_reset_rings(uc);

	return 0;

err_res_free:
	bcdma_free_bchan_resources(uc);
	udma_free_tx_resources(uc);
	udma_free_rx_resources(uc);

	udma_reset_uchan(uc);

	return ret;
}

static int pktdma_alloc_chan_resources(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	int ret;

	switch (uc->config.dir) {
	case DMA_MEM_TO_DEV:
		/* Slave transfer synchronized - mem to dev (TX) trasnfer */
		dev_dbg(uc->ud->dev, "%s: chan%d as MEM-to-DEV\n", __func__,
			uc->id);

		ret = udma_alloc_tx_resources(uc);
		if (ret) {
			uc->config.remote_thread_id = -1;
			return ret;
		}

		uc->config.src_thread = ud->psil_base + uc->tchan->id;
		uc->config.dst_thread = uc->config.remote_thread_id;
		uc->config.dst_thread |= K3_PSIL_DST_THREAD_ID_OFFSET;

		ret = pktdma_tisci_tx_channel_config(uc);
		break;
	case DMA_DEV_TO_MEM:
		/* Slave transfer synchronized - dev to mem (RX) trasnfer */
		dev_dbg(uc->ud->dev, "%s: chan%d as DEV-to-MEM\n", __func__,
			uc->id);

		ret = udma_alloc_rx_resources(uc);
		if (ret) {
			uc->config.remote_thread_id = -1;
			return ret;
		}

		uc->config.src_thread = uc->config.remote_thread_id;
		uc->config.dst_thread = (ud->psil_base + uc->rchan->id) |
					K3_PSIL_DST_THREAD_ID_OFFSET;

		ret = pktdma_tisci_rx_channel_config(uc);
		break;
	default:
		/* Can not happen */
		dev_err(uc->ud->dev, "%s: chan%d invalid direction (%u)\n",
			__func__, uc->id, uc->config.dir);
		return -EINVAL;
	}

	/* check if the channel configuration was successful */
	if (ret)
		goto err_res_free;

	/* PSI-L pairing */
	ret = udma_navss_psil_pair(ud, uc->config.src_thread, uc->config.dst_thread);
	if (ret) {
		dev_err(ud->dev, "PSI-L pairing failed: 0x%04x -> 0x%04x\n",
			uc->config.src_thread, uc->config.dst_thread);
		goto err_res_free;
	}

	if (udma_is_chan_running(uc)) {
		dev_warn(ud->dev, "chan%d: is running!\n", uc->id);
		udma_stop(uc);
		if (udma_is_chan_running(uc)) {
			dev_err(ud->dev, "chan%d: won't stop!\n", uc->id);
			goto err_res_free;
		}
	}

	udma_reset_rings(uc);

	if (uc->tchan)
		dev_dbg(ud->dev,
			"chan%d: tchan%d, tflow%d, Remote thread: 0x%04x\n",
			uc->id, uc->tchan->id, uc->tchan->tflow_id,
			uc->config.remote_thread_id);
	else if (uc->rchan)
		dev_dbg(ud->dev,
			"chan%d: rchan%d, rflow%d, Remote thread: 0x%04x\n",
			uc->id, uc->rchan->id, uc->rflow->id,
			uc->config.remote_thread_id);
	return 0;

err_res_free:
	udma_free_tx_resources(uc);
	udma_free_rx_resources(uc);

	udma_reset_uchan(uc);

	return ret;
}

static int udma_transfer(struct udevice *dev, int direction,
			 dma_addr_t dst, dma_addr_t src, size_t len)
{
	struct udma_dev *ud = dev_get_priv(dev);
	/* Channel0 is reserved for memcpy */
	struct udma_chan *uc = &ud->channels[0];
	dma_addr_t paddr = 0;
	int ret;

	switch (ud->match_data->type) {
	case DMA_TYPE_UDMA:
		ret = udma_alloc_chan_resources(uc);
		break;
	case DMA_TYPE_BCDMA:
		ret = bcdma_alloc_chan_resources(uc);
		break;
	default:
		return -EINVAL;
	};
	if (ret)
		return ret;

	udma_prep_dma_memcpy(uc, dst, src, len);
	udma_start(uc);
	udma_poll_completion(uc, &paddr);
	udma_stop(uc);

	switch (ud->match_data->type) {
	case DMA_TYPE_UDMA:
		udma_free_chan_resources(uc);
		break;
	case DMA_TYPE_BCDMA:
		bcdma_free_bchan_resources(uc);
		break;
	default:
		return -EINVAL;
	};

	return 0;
}

static int udma_request(struct dma *dma)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct udma_chan_config *ucc;
	struct udma_chan *uc;
	unsigned long dummy;
	int ret;

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}

	uc = &ud->channels[dma->id];
	ucc = &uc->config;
	switch (ud->match_data->type) {
	case DMA_TYPE_UDMA:
		ret = udma_alloc_chan_resources(uc);
		break;
	case DMA_TYPE_BCDMA:
		ret = bcdma_alloc_chan_resources(uc);
		break;
	case DMA_TYPE_PKTDMA:
		ret = pktdma_alloc_chan_resources(uc);
		break;
	default:
		return -EINVAL;
	}
	if (ret) {
		dev_err(dma->dev, "alloc dma res failed %d\n", ret);
		return -EINVAL;
	}

	if (uc->config.dir == DMA_MEM_TO_DEV) {
		uc->desc_tx = dma_alloc_coherent(ucc->hdesc_size, &dummy);
		memset(uc->desc_tx, 0, ucc->hdesc_size);
	} else {
		uc->desc_rx = dma_alloc_coherent(
				ucc->hdesc_size * UDMA_RX_DESC_NUM, &dummy);
		memset(uc->desc_rx, 0, ucc->hdesc_size * UDMA_RX_DESC_NUM);
	}

	uc->in_use = true;
	uc->desc_rx_cur = 0;
	uc->num_rx_bufs = 0;

	if (uc->config.dir == DMA_DEV_TO_MEM) {
		uc->cfg_data.flow_id_base = uc->rflow->id;
		uc->cfg_data.flow_id_cnt = 1;
	}

	return 0;
}

static int udma_rfree(struct dma *dma)
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

	udma_navss_psil_unpair(ud, uc->config.src_thread,
			       uc->config.dst_thread);

	bcdma_free_bchan_resources(uc);
	udma_free_tx_resources(uc);
	udma_free_rx_resources(uc);
	udma_reset_uchan(uc);

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

	if (uc->config.dir != DMA_MEM_TO_DEV)
		return -EINVAL;

	tc_ring_id = k3_nav_ringacc_get_ring_id(uc->tchan->tc_ring);

	desc_tx = uc->desc_tx;

	cppi5_hdesc_reset_hbdesc(desc_tx);

	cppi5_hdesc_init(desc_tx,
			 uc->config.needs_epib ? CPPI5_INFO0_HDESC_EPIB_PRESENT : 0,
			 uc->config.psd_size);
	cppi5_hdesc_set_pktlen(desc_tx, len);
	cppi5_hdesc_attach_buf(desc_tx, dma_src, len, dma_src, len);
	cppi5_desc_set_pktids(&desc_tx->hdr, uc->id, 0x3fff);
	cppi5_desc_set_retpolicy(&desc_tx->hdr, 0, tc_ring_id);
	/* pass below information from caller */
	cppi5_hdesc_set_pkttype(desc_tx, packet_data.pkt_type);
	cppi5_desc_set_tags_ids(&desc_tx->hdr, 0, packet_data.dest_tag);

	flush_dcache_range((unsigned long)dma_src,
			   ALIGN((unsigned long)dma_src + len,
				 ARCH_DMA_MINALIGN));
	flush_dcache_range((unsigned long)desc_tx,
			   ALIGN((unsigned long)desc_tx + uc->config.hdesc_size,
				 ARCH_DMA_MINALIGN));

	ret = udma_push_to_ring(uc->tchan->t_ring, uc->desc_tx);
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
	struct udma_chan_config *ucc;
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
	ucc = &uc->config;

	if (uc->config.dir != DMA_DEV_TO_MEM)
		return -EINVAL;
	if (!uc->num_rx_bufs)
		return -EINVAL;

	ret = k3_nav_ringacc_ring_pop(uc->rflow->r_ring, &desc_rx);
	if (ret && ret != -ENODATA) {
		dev_err(dma->dev, "rx dma fail ch_id:%lu %d\n", dma->id, ret);
		return ret;
	} else if (ret == -ENODATA) {
		return 0;
	}

	/* invalidate cache data */
	invalidate_dcache_range((ulong)desc_rx,
				(ulong)(desc_rx + ucc->hdesc_size));

	cppi5_hdesc_get_obuf(desc_rx, &buf_dma, &buf_dma_len);
	pkt_len = cppi5_hdesc_get_pktlen(desc_rx);

	/* invalidate cache data */
	invalidate_dcache_range((ulong)buf_dma,
				(ulong)(buf_dma + buf_dma_len));

	cppi5_desc_get_tags_ids(&desc_rx->hdr, &port_id, NULL);

	*dst = (void *)buf_dma;
	uc->num_rx_bufs--;

	return pkt_len;
}

static int udma_of_xlate(struct dma *dma, struct ofnode_phandle_args *args)
{
	struct udma_chan_config *ucc;
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct udma_chan *uc = &ud->channels[0];
	struct psil_endpoint_config *ep_config;
	u32 val;

	for (val = 0; val < ud->ch_count; val++) {
		uc = &ud->channels[val];
		if (!uc->in_use)
			break;
	}

	if (val == ud->ch_count)
		return -EBUSY;

	ucc = &uc->config;
	ucc->remote_thread_id = args->args[0];
	if (ucc->remote_thread_id & K3_PSIL_DST_THREAD_ID_OFFSET)
		ucc->dir = DMA_MEM_TO_DEV;
	else
		ucc->dir = DMA_DEV_TO_MEM;

	ep_config = psil_get_ep_config(ucc->remote_thread_id);
	if (IS_ERR(ep_config)) {
		dev_err(ud->dev, "No configuration for psi-l thread 0x%04x\n",
			uc->config.remote_thread_id);
		ucc->dir = DMA_MEM_TO_MEM;
		ucc->remote_thread_id = -1;
		return false;
	}

	ucc->pkt_mode = ep_config->pkt_mode;
	ucc->channel_tpl = ep_config->channel_tpl;
	ucc->notdpkt = ep_config->notdpkt;
	ucc->ep_type = ep_config->ep_type;

	if (ud->match_data->type == DMA_TYPE_PKTDMA &&
	    ep_config->mapped_channel_id >= 0) {
		ucc->mapped_channel_id = ep_config->mapped_channel_id;
		ucc->default_flow_id = ep_config->default_flow_id;
	} else {
		ucc->mapped_channel_id = -1;
		ucc->default_flow_id = -1;
	}

	ucc->needs_epib = ep_config->needs_epib;
	ucc->psd_size = ep_config->psd_size;
	ucc->metadata_size = (ucc->needs_epib ? CPPI5_INFO0_HDESC_EPIB_SIZE : 0) + ucc->psd_size;

	ucc->hdesc_size = cppi5_hdesc_calc_size(ucc->needs_epib,
						ucc->psd_size, 0);
	ucc->hdesc_size = ALIGN(ucc->hdesc_size, ARCH_DMA_MINALIGN);

	dma->id = uc->id;
	pr_debug("Allocated dma chn:%lu epib:%d psdata:%u meta:%u thread_id:%x\n",
		 dma->id, ucc->needs_epib,
		 ucc->psd_size, ucc->metadata_size,
		 ucc->remote_thread_id);

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

	if (uc->config.dir != DMA_DEV_TO_MEM)
		return -EINVAL;

	if (uc->num_rx_bufs >= UDMA_RX_DESC_NUM)
		return -EINVAL;

	desc_num = uc->desc_rx_cur % UDMA_RX_DESC_NUM;
	desc_rx = uc->desc_rx + (desc_num * uc->config.hdesc_size);
	dma_dst = (dma_addr_t)dst;

	cppi5_hdesc_reset_hbdesc(desc_rx);

	cppi5_hdesc_init(desc_rx,
			 uc->config.needs_epib ? CPPI5_INFO0_HDESC_EPIB_PRESENT : 0,
			 uc->config.psd_size);
	cppi5_hdesc_set_pktlen(desc_rx, size);
	cppi5_hdesc_attach_buf(desc_rx, dma_dst, size, dma_dst, size);

	flush_dcache_range((unsigned long)desc_rx,
			   ALIGN((unsigned long)desc_rx + uc->config.hdesc_size,
				 ARCH_DMA_MINALIGN));

	udma_push_to_ring(uc->rflow->fd_ring, desc_rx);

	uc->num_rx_bufs++;
	uc->desc_rx_cur++;

	return 0;
}

static int udma_get_cfg(struct dma *dma, u32 id, void **data)
{
	struct udma_dev *ud = dev_get_priv(dma->dev);
	struct udma_chan *uc;

	if (dma->id >= (ud->rchan_cnt + ud->tchan_cnt)) {
		dev_err(dma->dev, "invalid dma ch_id %lu\n", dma->id);
		return -EINVAL;
	}

	switch (id) {
	case TI_UDMA_CHAN_PRIV_INFO:
		uc = &ud->channels[dma->id];
		*data = &uc->cfg_data;
		return 0;
	}

	return -EINVAL;
}

static const struct dma_ops udma_ops = {
	.transfer	= udma_transfer,
	.of_xlate	= udma_of_xlate,
	.request	= udma_request,
	.rfree		= udma_rfree,
	.enable		= udma_enable,
	.disable	= udma_disable,
	.send		= udma_send,
	.receive	= udma_receive,
	.prepare_rcv_buf = udma_prepare_rcv_buf,
	.get_cfg	= udma_get_cfg,
};

static struct udma_match_data am654_main_data = {
	.type = DMA_TYPE_UDMA,
	.psil_base = 0x1000,
	.enable_memcpy_support = true,
	.statictr_z_mask = GENMASK(11, 0),
	.oes = {
		.udma_rchan = 0x200,
	},
	.tpl_levels = 2,
	.level_start_idx = {
		[0] = 8, /* Normal channels */
		[1] = 0, /* High Throughput channels */
	},
};

static struct udma_match_data am654_mcu_data = {
	.type = DMA_TYPE_UDMA,
	.psil_base = 0x6000,
	.enable_memcpy_support = true,
	.statictr_z_mask = GENMASK(11, 0),
	.oes = {
		.udma_rchan = 0x200,
	},
	.tpl_levels = 2,
	.level_start_idx = {
		[0] = 2, /* Normal channels */
		[1] = 0, /* High Throughput channels */
	},
};

static struct udma_match_data j721e_main_data = {
	.type = DMA_TYPE_UDMA,
	.psil_base = 0x1000,
	.enable_memcpy_support = true,
	.flags = UDMA_FLAG_PDMA_ACC32 | UDMA_FLAG_PDMA_BURST | UDMA_FLAG_TDTYPE,
	.statictr_z_mask = GENMASK(23, 0),
	.oes = {
		.udma_rchan = 0x400,
	},
	.tpl_levels = 3,
	.level_start_idx = {
		[0] = 16, /* Normal channels */
		[1] = 4, /* High Throughput channels */
		[2] = 0, /* Ultra High Throughput channels */
	},
};

static struct udma_match_data j721e_mcu_data = {
	.type = DMA_TYPE_UDMA,
	.psil_base = 0x6000,
	.enable_memcpy_support = true,
	.flags = UDMA_FLAG_PDMA_ACC32 | UDMA_FLAG_PDMA_BURST | UDMA_FLAG_TDTYPE,
	.statictr_z_mask = GENMASK(23, 0),
	.oes = {
		.udma_rchan = 0x400,
	},
	.tpl_levels = 2,
	.level_start_idx = {
		[0] = 2, /* Normal channels */
		[1] = 0, /* High Throughput channels */
	},
};

static struct udma_match_data am64_bcdma_data = {
	.type = DMA_TYPE_BCDMA,
	.psil_base = 0x2000, /* for tchan and rchan, not applicable to bchan */
	.enable_memcpy_support = true, /* Supported via bchan */
	.flags = UDMA_FLAG_PDMA_ACC32 | UDMA_FLAG_PDMA_BURST | UDMA_FLAG_TDTYPE,
	.statictr_z_mask = GENMASK(23, 0),
	.oes = {
		.bcdma_bchan_data = 0x2200,
		.bcdma_bchan_ring = 0x2400,
		.bcdma_tchan_data = 0x2800,
		.bcdma_tchan_ring = 0x2a00,
		.bcdma_rchan_data = 0x2e00,
		.bcdma_rchan_ring = 0x3000,
	},
	/* No throughput levels */
};

static struct udma_match_data am64_pktdma_data = {
	.type = DMA_TYPE_PKTDMA,
	.psil_base = 0x1000,
	.enable_memcpy_support = false,
	.flags = UDMA_FLAG_PDMA_ACC32 | UDMA_FLAG_PDMA_BURST | UDMA_FLAG_TDTYPE,
	.statictr_z_mask = GENMASK(23, 0),
	.oes = {
		.pktdma_tchan_flow = 0x1200,
		.pktdma_rchan_flow = 0x1600,
	},
	/* No throughput levels */
};

static const struct udevice_id udma_ids[] = {
	{
		.compatible = "ti,am654-navss-main-udmap",
		.data = (ulong)&am654_main_data,
	},
	{
		.compatible = "ti,am654-navss-mcu-udmap",
		.data = (ulong)&am654_mcu_data,
	}, {
		.compatible = "ti,j721e-navss-main-udmap",
		.data = (ulong)&j721e_main_data,
	}, {
		.compatible = "ti,j721e-navss-mcu-udmap",
		.data = (ulong)&j721e_mcu_data,
	},
	{
		.compatible = "ti,am64-dmss-bcdma",
		.data = (ulong)&am64_bcdma_data,
	},
	{
		.compatible = "ti,am64-dmss-pktdma",
		.data = (ulong)&am64_pktdma_data,
	},
	{ /* Sentinel */ },
};

U_BOOT_DRIVER(ti_edma3) = {
	.name	= "ti-udma",
	.id	= UCLASS_DMA,
	.of_match = udma_ids,
	.ops	= &udma_ops,
	.probe	= udma_probe,
	.priv_auto	= sizeof(struct udma_dev),
};
