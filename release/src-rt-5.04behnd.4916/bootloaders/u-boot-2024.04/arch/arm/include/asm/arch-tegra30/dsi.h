/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __ASM_ARCH_TEGRA_DSI_H
#define __ASM_ARCH_TEGRA_DSI_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

/* Register definitions for the Tegra display serial interface */

/* DSI syncpoint register 0x000 ~ 0x002 */
struct dsi_syncpt_reg {
	/* Address 0x000 ~ 0x002 */
	uint incr_syncpt;		/* _INCR_SYNCPT_0 */
	uint incr_syncpt_ctrl;		/* _INCR_SYNCPT_CNTRL_0 */
	uint incr_syncpt_err;		/* _INCR_SYNCPT_ERROR_0 */
};

/* DSI misc register 0x008 ~ 0x015 */
struct dsi_misc_reg {
	/* Address 0x008 ~ 0x015 */
	uint ctxsw;			/* _CTXSW_0 */
	uint dsi_rd_data;		/* _DSI_RD_DATA_0 */
	uint dsi_wr_data;		/* _DSI_WR_DATA_0 */
	uint dsi_pwr_ctrl;		/* _DSI_POWER_CONTROL_0 */
	uint int_enable;		/* _INT_ENABLE_0 */
	uint int_status;		/* _INT_STATUS_0 */
	uint int_mask;			/* _INT_MASK_0 */
	uint host_dsi_ctrl;		/* _HOST_DSI_CONTROL_0 */
	uint dsi_ctrl;			/* _DSI_CONTROL_0 */
	uint dsi_sol_delay;		/* _DSI_SOL_DELAY_0 */
	uint dsi_max_threshold;		/* _DSI_MAX_THRESHOLD_0 */
	uint dsi_trigger;		/* _DSI_TRIGGER_0 */
	uint dsi_tx_crc;		/* _DSI_TX_CRC_0 */
	uint dsi_status;		/* _DSI_STATUS_0 */
};

/* DSI init sequence register 0x01a ~ 0x022 */
struct dsi_init_seq_reg {
	/* Address 0x01a ~ 0x022 */
	uint dsi_init_seq_ctrl;		/* _DSI_INIT_SEQ_CONTROL_0 */
	uint dsi_init_seq_data_0;	/* _DSI_INIT_SEQ_DATA_0_0 */
	uint dsi_init_seq_data_1;	/* _DSI_INIT_SEQ_DATA_1_0 */
	uint dsi_init_seq_data_2;	/* _DSI_INIT_SEQ_DATA_2_0 */
	uint dsi_init_seq_data_3;	/* _DSI_INIT_SEQ_DATA_3_0 */
	uint dsi_init_seq_data_4;	/* _DSI_INIT_SEQ_DATA_4_0 */
	uint dsi_init_seq_data_5;	/* _DSI_INIT_SEQ_DATA_5_0 */
	uint dsi_init_seq_data_6;	/* _DSI_INIT_SEQ_DATA_6_0 */
	uint dsi_init_seq_data_7;	/* _DSI_INIT_SEQ_DATA_7_0 */
};

/* DSI packet sequence register 0x023 ~ 0x02e */
struct dsi_pkt_seq_reg {
	/* Address 0x023 ~ 0x02e */
	uint dsi_pkt_seq_0_lo;		/* _DSI_PKT_SEQ_0_LO_0 */
	uint dsi_pkt_seq_0_hi;		/* _DSI_PKT_SEQ_0_HI_0 */
	uint dsi_pkt_seq_1_lo;		/* _DSI_PKT_SEQ_1_LO_0 */
	uint dsi_pkt_seq_1_hi;		/* _DSI_PKT_SEQ_1_HI_0 */
	uint dsi_pkt_seq_2_lo;		/* _DSI_PKT_SEQ_2_LO_0 */
	uint dsi_pkt_seq_2_hi;		/* _DSI_PKT_SEQ_2_HI_0 */
	uint dsi_pkt_seq_3_lo;		/* _DSI_PKT_SEQ_3_LO_0 */
	uint dsi_pkt_seq_3_hi;		/* _DSI_PKT_SEQ_3_HI_0 */
	uint dsi_pkt_seq_4_lo;		/* _DSI_PKT_SEQ_4_LO_0 */
	uint dsi_pkt_seq_4_hi;		/* _DSI_PKT_SEQ_4_HI_0 */
	uint dsi_pkt_seq_5_lo;		/* _DSI_PKT_SEQ_5_LO_0 */
	uint dsi_pkt_seq_5_hi;		/* _DSI_PKT_SEQ_5_HI_0 */
};

/* DSI packet length register 0x033 ~ 0x037 */
struct dsi_pkt_len_reg {
	/* Address 0x033 ~ 0x037 */
	uint dsi_dcs_cmds;		/* _DSI_DCS_CMDS_0 */
	uint dsi_pkt_len_0_1;		/* _DSI_PKT_LEN_0_1_0 */
	uint dsi_pkt_len_2_3;		/* _DSI_PKT_LEN_2_3_0 */
	uint dsi_pkt_len_4_5;		/* _DSI_PKT_LEN_4_5_0 */
	uint dsi_pkt_len_6_7;		/* _DSI_PKT_LEN_6_7_0 */
};

/* DSI PHY timing register 0x03c ~ 0x03f */
struct dsi_timing_reg {
	/* Address 0x03c ~ 0x03f */
	uint dsi_phy_timing_0;		/* _DSI_PHY_TIMING_0_0 */
	uint dsi_phy_timing_1;		/* _DSI_PHY_TIMING_1_0 */
	uint dsi_phy_timing_2;		/* _DSI_PHY_TIMING_2_0 */
	uint dsi_bta_timing;		/* _DSI_BTA_TIMING_0 */
};

/* DSI timeout register 0x044 ~ 0x046 */
struct dsi_timeout_reg {
	/* Address 0x044 ~ 0x046 */
	uint dsi_timeout_0;		/* _DSI_TIMEOUT_0_0 */
	uint dsi_timeout_1;		/* _DSI_TIMEOUT_1_0 */
	uint dsi_to_tally;		/* _DSI_TO_TALLY_0 */
};

/* DSI PAD control register 0x04b ~ 0x04e */
struct dsi_pad_ctrl_reg {
	/* Address 0x04b ~ 0x04e */
	uint pad_ctrl;			/* _PAD_CONTROL_0 */
	uint pad_ctrl_cd;		/* _PAD_CONTROL_CD_0 */
	uint pad_cd_status;		/* _PAD_CD_STATUS_0 */
	uint dsi_vid_mode_control;	/* _DSI_VID_MODE_CONTROL_0 */
};

/* Display Serial Interface (DSI_) regs */
struct dsi_ctlr {
	struct dsi_syncpt_reg syncpt;	/* SYNCPT register 0x000 ~ 0x002 */
	uint reserved0[5];		/* reserved_0[5] */

	struct dsi_misc_reg misc;	/* MISC register 0x008 ~ 0x015 */
	uint reserved1[4];		/* reserved_1[4] */

	struct dsi_init_seq_reg init;	/* INIT register 0x01a ~ 0x022 */
	struct dsi_pkt_seq_reg pkt;	/* PKT register 0x023 ~ 0x02e */
	uint reserved2[4];		/* reserved_2[4] */

	struct dsi_pkt_len_reg len;	/* LEN registers 0x033 ~ 0x037 */
	uint reserved3[4];		/* reserved_3[4] */

	struct dsi_timing_reg ptiming;	/* TIMING registers 0x03c ~ 0x03f */
	uint reserved4[4];		/* reserved_4[4] */

	struct dsi_timeout_reg timeout;	/* TIMEOUT registers 0x044 ~ 0x046 */
	uint reserved5[4];		/* reserved_5[4] */

	struct dsi_pad_ctrl_reg pad;	/* PAD registers 0x04b ~ 0x04e */
};

#define DSI_POWER_CONTROL_ENABLE	BIT(0)

#define DSI_HOST_CONTROL_FIFO_RESET	BIT(21)
#define DSI_HOST_CONTROL_CRC_RESET	BIT(20)
#define DSI_HOST_CONTROL_TX_TRIG_SOL	(0 << 12)
#define DSI_HOST_CONTROL_TX_TRIG_FIFO	(1 << 12)
#define DSI_HOST_CONTROL_TX_TRIG_HOST	(2 << 12)
#define DSI_HOST_CONTROL_RAW		BIT(6)
#define DSI_HOST_CONTROL_HS		BIT(5)
#define DSI_HOST_CONTROL_FIFO_SEL	BIT(4)
#define DSI_HOST_CONTROL_IMM_BTA	BIT(3)
#define DSI_HOST_CONTROL_PKT_BTA	BIT(2)
#define DSI_HOST_CONTROL_CS		BIT(1)
#define DSI_HOST_CONTROL_ECC		BIT(0)

#define DSI_CONTROL_HS_CLK_CTRL		BIT(20)
#define DSI_CONTROL_CHANNEL(c)		(((c) & 0x3) << 16)
#define DSI_CONTROL_FORMAT(f)		(((f) & 0x3) << 12)
#define DSI_CONTROL_TX_TRIG(x)		(((x) & 0x3) <<  8)
#define DSI_CONTROL_LANES(n)		(((n) & 0x3) <<  4)
#define DSI_CONTROL_DCS_ENABLE		BIT(3)
#define DSI_CONTROL_SOURCE(s)		(((s) & 0x1) <<  2)
#define DSI_CONTROL_VIDEO_ENABLE	BIT(1)
#define DSI_CONTROL_HOST_ENABLE		BIT(0)

#define DSI_TRIGGER_HOST		BIT(1)
#define DSI_TRIGGER_VIDEO		BIT(0)

#define DSI_STATUS_IDLE			BIT(10)
#define DSI_STATUS_UNDERFLOW		BIT(9)
#define DSI_STATUS_OVERFLOW		BIT(8)

#define DSI_TIMING_FIELD(value, period, hwinc) \
	((DIV_ROUND_CLOSEST(value, period) - (hwinc)) & 0xff)

#define DSI_TIMEOUT_LRX(x)		(((x) & 0xffff) << 16)
#define DSI_TIMEOUT_HTX(x)		(((x) & 0xffff) <<  0)
#define DSI_TIMEOUT_PR(x)		(((x) & 0xffff) << 16)
#define DSI_TIMEOUT_TA(x)		(((x) & 0xffff) <<  0)

#define DSI_TALLY_TA(x)			(((x) & 0xff) << 16)
#define DSI_TALLY_LRX(x)		(((x) & 0xff) <<  8)
#define DSI_TALLY_HTX(x)		(((x) & 0xff) <<  0)

#define DSI_PAD_CONTROL_PAD_PULLDN_ENAB(x)	(((x) & 0x1) << 28)
#define DSI_PAD_CONTROL_PAD_SLEWUPADJ(x)	(((x) & 0x7) << 24)
#define DSI_PAD_CONTROL_PAD_SLEWDNADJ(x)	(((x) & 0x7) << 20)
#define DSI_PAD_CONTROL_PAD_PREEMP_EN(x)	(((x) & 0x1) << 19)
#define DSI_PAD_CONTROL_PAD_PDIO_CLK(x)		(((x) & 0x1) << 18)
#define DSI_PAD_CONTROL_PAD_PDIO(x)		(((x) & 0x3) << 16)
#define DSI_PAD_CONTROL_PAD_LPUPADJ(x)		(((x) & 0x3) << 14)
#define DSI_PAD_CONTROL_PAD_LPDNADJ(x)		(((x) & 0x3) << 12)

/*
 * pixel format as used in the DSI_CONTROL_FORMAT field
 */
enum tegra_dsi_format {
	TEGRA_DSI_FORMAT_16P,
	TEGRA_DSI_FORMAT_18NP,
	TEGRA_DSI_FORMAT_18P,
	TEGRA_DSI_FORMAT_24P,
};

/* DSI calibration in VI region */
#define TEGRA_VI_BASE			0x54080000

#define CSI_CILA_MIPI_CAL_CONFIG_0	0x22a
#define  MIPI_CAL_TERMOSA(x)		(((x) & 0x1f) << 0)

#define CSI_CILB_MIPI_CAL_CONFIG_0	0x22b
#define  MIPI_CAL_TERMOSB(x)		(((x) & 0x1f) << 0)

#define CSI_CIL_PAD_CONFIG		0x229
#define  PAD_CIL_PDVREG(x)		(((x) & 0x01) << 1)

#define CSI_DSI_MIPI_CAL_CONFIG		0x234
#define  MIPI_CAL_HSPDOSD(x)		(((x) & 0x1f) << 16)
#define  MIPI_CAL_HSPUOSD(x)		(((x) & 0x1f) << 8)

#define CSI_MIPIBIAS_PAD_CONFIG		0x235
#define  PAD_DRIV_DN_REF(x)		(((x) & 0x7) << 16)
#define  PAD_DRIV_UP_REF(x)		(((x) & 0x7) << 8)

#endif /* __ASM_ARCH_TEGRA_DSI_H */
