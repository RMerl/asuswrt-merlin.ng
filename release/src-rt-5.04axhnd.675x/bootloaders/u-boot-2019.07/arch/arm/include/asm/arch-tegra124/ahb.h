/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA124_AHB_H_
#define _TEGRA124_AHB_H_

struct ahb_ctlr {
	u32 reserved0;			/* 00h */
	u32 arbitration_disable;	/* _ARBITRATION_DISABLE_0,	04h */
	u32 arbitration_priority_ctrl;	/* _ARBITRATION_PRIORITY_CTRL_0,08h */
	u32 arbitration_usr_protect;	/* _ARBITRATION_USR_PROTECT_0,	0ch */
	u32 gizmo_ahb_mem;		/* _GIZMO_AHB_MEM_0,		10h */
	u32 gizmo_apb_dma;		/* _GIZMO_APB_DMA_0,		14h */
	u32 reserved6[2];		/* 18h, 1ch */
	u32 gizmo_usb;			/* _GIZMO_USB_0,		20h */
	u32 gizmo_ahb_xbar_bridge;	/* _GIZMO_AHB_XBAR_BRIDGE_0,	24h */
	u32 gizmo_cpu_ahb_bridge;	/* _GIZMO_CPU_AHB_BRIDGE_0,	28h */
	u32 gizmo_cop_ahb_bridge;	/* _GIZMO_COP_AHB_BRIDGE_0,	2ch */
	u32 gizmo_xbar_apb_ctlr;	/* _GIZMO_XBAR_APB_CTLR_0,	30h */
	u32 gizmo_vcp_ahb_bridge;	/* _GIZMO_VCP_AHB_BRIDGE_0,	34h */
	u32 reserved13[2];		/* 38h, 3ch */
	u32 gizmo_nand;			/* _GIZMO_NAND_0,		40h */
	u32 reserved15;			/* 44h */
	u32 gizmo_sdmmc4;		/* _GIZMO_SDMMC4_0,		48h */
	u32 reserved17;			/* 4ch */
	u32 gizmo_se;			/* _GIZMO_SE_0,			50h */
	u32 gizmo_tzram;		/* _GIZMO_TZRAM_0,		54h */
	u32 reserved20[3];		/* 58h, 5ch, 60h */
	u32 gizmo_bsev;			/* _GIZMO_BSEV_0,		64h */
	u32 reserved22[3];		/* 68h, 6ch, 70h */
	u32 gizmo_bsea;			/* _GIZMO_BSEA_0,		74h */
	u32 gizmo_nor;			/* _GIZMO_NOR_0,		78h */
	u32 gizmo_usb2;			/* _GIZMO_USB2_0,		7ch */
	u32 gizmo_usb3;			/* _GIZMO_USB3_0,		80h */
	u32 gizmo_sdmmc1;		/* _GIZMO_SDMMC1_0,		84h */
	u32 gizmo_sdmmc2;		/* _GIZMO_SDMMC2_0,		88h */
	u32 gizmo_sdmmc3;		/* _GIZMO_SDMMC3_0,		8ch */
	u32 reserved30[13];		/* 90h ~ c0h */
	u32 ahb_wrq_empty;		/* _AHB_WRQ_EMPTY_0,		c4h */
	u32 reserved32[5];		/* c8h ~ d8h */
	u32 ahb_mem_prefetch_cfg_x;	/* _AHB_MEM_PREFETCH_CFG_X_0,	dch */
	u32 arbitration_xbar_ctrl;	/* _ARBITRATION_XBAR_CTRL_0,	e0h */
	u32 ahb_mem_prefetch_cfg3;	/* _AHB_MEM_PREFETCH_CFG3_0,	e4h */
	u32 ahb_mem_prefetch_cfg4;	/* _AHB_MEM_PREFETCH_CFG3_0,	e8h */
	u32 avp_ppcs_rd_coh_status;	/* _AVP_PPCS_RD_COH_STATUS_0,	ech */
	u32 ahb_mem_prefetch_cfg1;	/* _AHB_MEM_PREFETCH_CFG1_0,	f0h */
	u32 ahb_mem_prefetch_cfg2;	/* _AHB_MEM_PREFETCH_CFG2_0,	f4h */
	u32 ahbslvmem_status;		/* _AHBSLVMEM_STATUS_0, f8h */
	/* _ARBITRATION_AHB_MEM_WRQUE_MST_ID_0, fch */
	u32 arbitration_ahb_mem_wrque_mst_id;
	u32 arbitration_cpu_abort_addr;	/* _ARBITRATION_CPU_ABORT_ADDR_0,100h */
	u32 arbitration_cpu_abort_info;	/* _ARBITRATION_CPU_ABORT_INFO_0,104h */
	u32 arbitration_cop_abort_addr;	/* _ARBITRATION_COP_ABORT_ADDR_0,108h */
	u32 arbitration_cop_abort_info;	/* _ARBITRATION_COP_ABORT_INFO_0,10ch */
	u32 reserved46[4];		/* 110h ~ 11ch */
	u32 avpc_mccif_fifoctrl;	/* _AVPC_MCCIF_FIFOCTRL_0,	120h */
	u32 timeout_wcoal_avpc;		/* _TIMEOUT_WCOAL_AVPC_0,	124h */
	u32 mpcorelp_mccif_fifoctrl;	/* _MPCORELP_MCCIF_FIFOCTRL_0,	128h */
	u32 mpcore_mccif_fifoctrl;	/* _MPCORE_MCCIF_FIFOCTRL_0,	12ch */
	u32 axicif_fastsync_ctrl;	/* AXICIF_FASTSYNC_CTRL_0,	130h */
	u32 axicif_fastsync_statistics;	/* _AXICIF_FASTSYNC_STATISTICS_0,134h */
	/* _AXICIF_FASTSYNC0_CPUCLK_TO_MCCLK_0,	138h */
	u32 axicif_fastsync0_cpuclk_to_mcclk;
	/* _AXICIF_FASTSYNC1_CPUCLK_TO_MCCLK_0, 13ch */
	u32 axicif_fastsync1_cpuclk_to_mcclk;
	/* _AXICIF_FASTSYNC2_CPUCLK_TO_MCCLK_0, 140h */
	u32 axicif_fastsync2_cpuclk_to_mcclk;
	/* _AXICIF_FASTSYNC0_MCCLK_TO_CPUCLK_0, 144h */
	u32 axicif_fastsync0_mcclk_to_cpuclk;
	/* _AXICIF_FASTSYNC1_MCCLK_TO_CPUCLK_0, 148h */
	u32 axicif_fastsync1_mcclk_to_cpuclk;
	/* _AXICIF_FASTSYNC2_MCCLK_TO_CPUCLK_0, 14ch */
	u32 axicif_fastsync2_mcclk_to_cpuclk;
};

#define PPSB_STOPCLK_ENABLE	(1 << 2)

#define GIZ_ENABLE_SPLIT	(1 << 0)
#define GIZ_ENB_FAST_REARB	(1 << 2)
#define GIZ_DONT_SPLIT_AHB_WR	(1 << 7)

#define GIZ_USB_IMMEDIATE	(1 << 18)

/* AHB_ARBITRATION_XBAR_CTRL_0 0xe0 */
#define ARBITRATION_XBAR_CTRL_PPSB_ENABLE	(1 << 2)

#endif	/* _TEGRA124_AHB_H_ */
