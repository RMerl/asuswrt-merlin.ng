// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2021 Texas Instruments Incorporated - https://www.ti.com
 */

#define UDMA_RCHAN_RFLOW_RNG_FLOWID_CNT_SHIFT	(16)

/* How SRC/DST tag should be updated by UDMA in the descriptor's Word 3 */
#define UDMA_RFLOW_SRCTAG_NONE		0
#define UDMA_RFLOW_SRCTAG_CFG_TAG	1
#define UDMA_RFLOW_SRCTAG_FLOW_ID	2
#define UDMA_RFLOW_SRCTAG_SRC_TAG	4

#define UDMA_RFLOW_DSTTAG_NONE		0
#define UDMA_RFLOW_DSTTAG_CFG_TAG	1
#define UDMA_RFLOW_DSTTAG_FLOW_ID	2
#define UDMA_RFLOW_DSTTAG_DST_TAG_LO	4
#define UDMA_RFLOW_DSTTAG_DST_TAG_HI	5

#define UDMA_RFLOW_RFC_DEFAULT	\
	((UDMA_RFLOW_SRCTAG_NONE <<  UDMA_RFLOW_RFC_SRC_TAG_HI_SEL_SHIFT) | \
	 (UDMA_RFLOW_SRCTAG_SRC_TAG << UDMA_RFLOW_RFC_SRC_TAG_LO_SEL_SHIFT) | \
	 (UDMA_RFLOW_DSTTAG_DST_TAG_HI << UDMA_RFLOW_RFC_DST_TAG_HI_SEL_SHIFT) | \
	 (UDMA_RFLOW_DSTTAG_DST_TAG_LO << UDMA_RFLOW_RFC_DST_TAG_LO_SE_SHIFT))

#define UDMA_RFLOW_RFx_REG_FDQ_SIZE_SHIFT	(16)

/* TCHAN */
static inline u32 udma_tchan_read(struct udma_tchan *tchan, int reg)
{
	if (!tchan)
		return 0;
	return udma_read(tchan->reg_chan, reg);
}

static inline void udma_tchan_write(struct udma_tchan *tchan, int reg, u32 val)
{
	if (!tchan)
		return;
	udma_write(tchan->reg_chan, reg, val);
}

static inline void udma_tchan_update_bits(struct udma_tchan *tchan, int reg,
					  u32 mask, u32 val)
{
	if (!tchan)
		return;
	udma_update_bits(tchan->reg_chan, reg, mask, val);
}

/* RCHAN */
static inline u32 udma_rchan_read(struct udma_rchan *rchan, int reg)
{
	if (!rchan)
		return 0;
	return udma_read(rchan->reg_chan, reg);
}

static inline void udma_rchan_write(struct udma_rchan *rchan, int reg, u32 val)
{
	if (!rchan)
		return;
	udma_write(rchan->reg_chan, reg, val);
}

static inline void udma_rchan_update_bits(struct udma_rchan *rchan, int reg,
					  u32 mask, u32 val)
{
	if (!rchan)
		return;
	udma_update_bits(rchan->reg_chan, reg, mask, val);
}

/* RFLOW */
static inline u32 udma_rflow_read(struct udma_rflow *rflow, int reg)
{
	if (!rflow)
		return 0;
	return udma_read(rflow->reg_rflow, reg);
}

static inline void udma_rflow_write(struct udma_rflow *rflow, int reg, u32 val)
{
	if (!rflow)
		return;
	udma_write(rflow->reg_rflow, reg, val);
}

static inline void udma_rflow_update_bits(struct udma_rflow *rflow, int reg,
					  u32 mask, u32 val)
{
	if (!rflow)
		return;
	udma_update_bits(rflow->reg_rflow, reg, mask, val);
}

static void udma_alloc_tchan_raw(struct udma_chan *uc)
{
	u32 mode, fetch_size;

	if (uc->config.pkt_mode)
		mode = UDMA_CHAN_CFG_CHAN_TYPE_PACKET_PBRR;
	else
		mode = UDMA_CHAN_CFG_CHAN_TYPE_3RDP_BC_PBRR;

	udma_tchan_update_bits(uc->tchan, UDMA_TCHAN_TCFG_REG,
			       UDMA_CHAN_CFG_CHAN_TYPE_MASK, mode);

	if (uc->config.dir == DMA_MEM_TO_MEM)
		fetch_size = sizeof(struct cppi5_desc_hdr_t) >> 2;
	else
		fetch_size = cppi5_hdesc_calc_size(uc->config.needs_epib,
						   uc->config.psd_size, 0) >> 2;

	udma_tchan_update_bits(uc->tchan, UDMA_TCHAN_TCFG_REG,
			       UDMA_CHAN_CFG_FETCH_SIZE_MASK, fetch_size);
	udma_tchan_write(uc->tchan, UDMA_TCHAN_TCQ_REG,
			 k3_nav_ringacc_get_ring_id(uc->tchan->tc_ring));
}

static void udma_alloc_rchan_raw(struct udma_chan *uc)
{
	struct udma_dev *ud = uc->ud;
	int fd_ring = k3_nav_ringacc_get_ring_id(uc->rflow->fd_ring);
	int rx_ring = k3_nav_ringacc_get_ring_id(uc->rflow->r_ring);
	int tc_ring = k3_nav_ringacc_get_ring_id(uc->tchan->tc_ring);
	u32 rx_einfo_present = 0, rx_psinfo_present = 0;
	u32 mode, fetch_size, rxcq_num;

	if (uc->config.pkt_mode)
		mode = UDMA_CHAN_CFG_CHAN_TYPE_PACKET_PBRR;
	else
		mode = UDMA_CHAN_CFG_CHAN_TYPE_3RDP_BC_PBRR;

	udma_rchan_update_bits(uc->rchan, UDMA_RCHAN_RCFG_REG,
			       UDMA_CHAN_CFG_CHAN_TYPE_MASK, mode);

	if (uc->config.dir == DMA_MEM_TO_MEM) {
		fetch_size = sizeof(struct cppi5_desc_hdr_t) >> 2;
		rxcq_num = tc_ring;
	} else {
		fetch_size = cppi5_hdesc_calc_size(uc->config.needs_epib,
						   uc->config.psd_size, 0) >> 2;
		rxcq_num = rx_ring;
	}

	udma_rchan_update_bits(uc->rchan, UDMA_RCHAN_RCFG_REG,
			       UDMA_CHAN_CFG_FETCH_SIZE_MASK, fetch_size);
	udma_rchan_write(uc->rchan, UDMA_RCHAN_RCQ_REG, rxcq_num);

	if (uc->config.dir == DMA_MEM_TO_MEM)
		return;

	if (ud->match_data->type == DMA_TYPE_UDMA &&
	    uc->rflow->id != uc->rchan->id &&
	    uc->config.dir != DMA_MEM_TO_MEM)
		udma_rchan_write(uc->rchan, UDMA_RCHAN_RFLOW_RNG_REG, uc->rflow->id |
				 1 << UDMA_RCHAN_RFLOW_RNG_FLOWID_CNT_SHIFT);

	if (uc->config.needs_epib)
		rx_einfo_present = UDMA_RFLOW_RFA_EINFO;

	if (uc->config.psd_size)
		rx_psinfo_present = UDMA_RFLOW_RFA_PSINFO;

	udma_rflow_write(uc->rflow, UDMA_RFLOW_REG(A),
			 rx_einfo_present | rx_psinfo_present | rxcq_num);

	udma_rflow_write(uc->rflow, UDMA_RFLOW_REG(C), UDMA_RFLOW_RFC_DEFAULT);
	udma_rflow_write(uc->rflow, UDMA_RFLOW_REG(D),
			 fd_ring | fd_ring << UDMA_RFLOW_RFx_REG_FDQ_SIZE_SHIFT);
	udma_rflow_write(uc->rflow, UDMA_RFLOW_REG(E),
			 fd_ring | fd_ring << UDMA_RFLOW_RFx_REG_FDQ_SIZE_SHIFT);
	udma_rflow_write(uc->rflow, UDMA_RFLOW_REG(G), fd_ring);
	udma_rflow_write(uc->rflow, UDMA_RFLOW_REG(H),
			 fd_ring | fd_ring << UDMA_RFLOW_RFx_REG_FDQ_SIZE_SHIFT);
}
