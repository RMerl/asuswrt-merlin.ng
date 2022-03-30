/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
 */

#ifndef _SCC_H
#define _SCC_H

#define DMA_READ		0	/* SCC read  DMA		*/
#define DMA_WRITE		1	/* SCC write DMA		*/

#define DMA_LINEAR		0	/* DMA linear buffer access method */
#define DMA_CYCLIC		1	/* DMA cyclic buffer access method */

#define DMA_START		0	/* DMA command - start DMA	*/
#define DMA_STOP		1	/* DMA command - stop  DMA	*/
#define DMA_START_FH_RESET	2	/* DMA command - start DMA reset FH */
#define DMA_TAKEOVER		15	/* DMA command - commit the DMA conf */

#define AGU_ACTIVE		0	/* enable AGU address calculation */
#define AGU_BYPASS		1	/* set AGU to bypass mode	*/

#define USE_NO_FH		0	/* order the DMA to not use FH	*/
#define USE_FH			1	/* order the DMA to work with FH*/

#define SCC_DBG_IDLE		0	/* DEBUG status (idle interfaces) */
#define SCC_DBG_SYNC_RES	0x0001	/* synchronuous reset		*/

#define SCC_TO_IMMEDIATE	1	/* takeover command issued immediately*/
#define TO_DMA_CFG		2	/* takeover command for the DMA config*/

#define DMA_CMD_RESET		0
#define DMA_CMD_SETUP		1
#define DMA_CMD_START		2
#define DMA_CMD_STOP		3

#define DMA_STATE_RESET		0
#define DMA_STATE_SETUP		1
#define DMA_STATE_START		2
#define DMA_STATE_ERROR		3

#define SRMD			0
#define STRM_D			1
#define STRM_P			2

/*
 * Slowest Monterey domain is DVP 27 MHz (324/27 = 12; 12*16 = 192 CPU clocks)
 */
#define RESET_TIME		2	/* cycle calc see in SCC_Reset	*/

struct scc_descriptor {
	char *pu_name;		/* PU identifier			*/
	char *scc_instance;	/* SCC Name				*/
	u32 profile;		/* SCC VCI_D profile			*/

	u32 base_address;	/* base address of the SCC unit reg shell*/

	/* SCS Interconnect configuration */
	u32 p_scc_id;		/* instance number of SCC unit		*/
	u32 p_mci_id;		/* memory channel ID			*/

	/* DMA Registers configuration */
	u32 p_dma_channels_rd;	/* Number of Read DMA channels		*/
	u32 p_dma_channels_wr;	/* Number of Write DMA channels		*/

	u32 p_dma_packet_desc;	/* Number of packet descriptors		*/
	u32 p_dma_mci_desc;	/* Number of MCI_CFG Descriptors	*/

	int use_fh;		/* the flag tells if SCC uses an FH	*/

	int p_si2ocp_id;	/* instance number of SI2OCP unit	*/
	int hw_dma_cfg;		/* HW or SW DMA config flag		*/
	int hw_dma_start;	/* HW or SW DMA start/stop flag		*/

	u32 *buffer_tag_list;	/* list of the buffer tags available	*/
	u32 *csize_list;	/* list of the valid CSIZE values	*/
};

struct scc_dma_state {
	u32 scc_id:8;		/* SCC id				*/
	u32 dma_id:8;		/* DMA id, used for match with array idx*/
	u32 buffer_tag:8;	/* mem buf tag, assigned to this DMA	*/
	u32 dma_status:2;	/* state of DMA, of the DMA_STATE_ const*/
	u32 dma_drs:2;		/* DMA dir, either DMA_READ or DMA_WRITE*/
	u32 dma_cmd:4;		/* last executed command on this DMA	*/
};

union scc_cmd {
	u32 reg;
	struct {
		u32 res1:19;	/* reserved				*/
		u32 drs:1;	/* DMA Register Set			*/
		u32 rid:2;	/* Register Identifier			*/
		u32 id:6;	/* DMA Identifier			*/
		u32 action:4;	/* DMA Command encoding			*/
	} bits;
};

union scc_dma_cfg {
	u32 reg;
	struct {
		u32 res1:17;		/* reserved			*/
		u32 agu_mode:1;		/* AGU Mode			*/
		u32 res2:1;		/* reserved			*/
		u32 fh_mode:1;		/* Fifo Handler			*/
		u32 buffer_type:1;	/* Defines type of mem buffers	*/
		u32 mci_cfg_id:1;	/* MCI_CFG register selector	*/
		u32 packet_cfg_id:1;	/* PACKET_CFG register selector	*/
		u32 buffer_id:8;	/* DMA Buffer Identifier	*/
	} bits;
};

union scc_debug {
	u32 reg;
	struct {
		u32 res1:20;	/* reserved				*/
		u32 arg:8;	/* SCC Debug Command Argument (#)	*/
		u32 cmd:4;	/* SCC Debug Command Register		*/
	} bits;
};

union scc_softwareconfiguration {
	u32 reg;
	struct {
		u32 res1:28;		/* reserved			*/
		u32 clock_status:1;	/* clock on/off			*/
		u32 packet_select:1;	/* active SCC packet id		*/
		u32 enable_status:1;	/* enabled [1/0]		*/
		u32 active_status:1;	/* 1=active  0=reset		*/
	} bits;
};

/*
 * System on Chip Channel ID
 */
enum scc_id {
	SCC_NULL = -1,		/* illegal SCC identifier		*/
	SCC_FE_3DCOMB_WR,	/* SCC_FE_3DCOMB Write channel		*/
	SCC_FE_3DCOMB_RD,	/* SCC_FE_3DCOMB Read channel		*/
	SCC_DI_TNR_WR,		/* SCC_DI_TNR Write channel		*/
	SCC_DI_TNR_FIELD_RD,	/* SCC_DI_TNR_FIELD Read channel	*/
	SCC_DI_TNR_FRAME_RD,	/* SCC_DI_TNR_FRAME Read channel	*/
	SCC_DI_MVAL_WR,		/* SCC_DI_MVAL Write channel		*/
	SCC_DI_MVAL_RD,		/* SCC_DI_MVAL Read channel		*/
	SCC_RC_FRAME_WR,	/* SCC_RC_FRAME Write channel		*/
	SCC_RC_FRAME0_RD,	/* SCC_RC_FRAME0 Read channel		*/
	SCC_OPT_FIELD0_RD,	/* SCC_OPT_FIELD0 Read channel		*/
	SCC_OPT_FIELD1_RD,	/* SCC_OPT_FIELD1 Read channel		*/
	SCC_OPT_FIELD2_RD,	/* SCC_OPT_FIELD2 Read channel		*/
	SCC_PIP_FRAME_WR,	/* SCC_PIP_FRAME Write channel		*/
	SCC_PIP_FRAME_RD,	/* SCC_PIP_FRAME Read channel		*/
	SCC_DP_AGPU_RD,		/* SCC_DP_AGPU Read channel		*/
	SCC_EWARP_RW,		/* SCC_EWARP Read/Write channel		*/
	SCC_DP_OSD_RD,		/* SCC_DP_OSD Read channel		*/
	SCC_DP_GRAPHIC_RD,	/* SCC_DP_GRAPHIC Read channel		*/
	SCC_DVP_OSD_RD,		/* SCC_DVP_OSD Read channel		*/
	SCC_DVP_VBI_RD,		/* SCC_DVP_VBI Read channel		*/
	SCC_TSIO_WR,		/* SCC_TSIO Write channel		*/
	SCC_TSIO_RD,		/* SCC_TSIO Read channel		*/
	SCC_TSD_WR,		/* SCC_TSD Write channel		*/
	SCC_VD_UD_ST_RW,	/* SCC_VD_UD_ST Read/Write channel	*/
	SCC_VD_FRR_RD,		/* SCC_VD_FRR Read channel		*/
	SCC_VD_FRW_DISP_WR,	/* SCC_VD_FRW_DISP Write channel	*/
	SCC_MR_VD_M_Y_RD,	/* SCC_MR_VD_M_Y Read channel		*/
	SCC_MR_VD_M_C_RD,	/* SCC_MR_VD_M_C Read channel		*/
	SCC_MR_VD_S_Y_RD,	/* SCC_MR_VD_S_Y Read channel		*/
	SCC_MR_VD_S_C_RD,	/* SCC_MR_VD_S_C Read channel		*/
	SCC_GA_WR,		/* SCC_GA Write channel			*/
	SCC_GA_SRC1_RD,		/* SCC_GA_SRC1 Read channel		*/
	SCC_GA_SRC2_RD,		/* SCC_GA_SRC2 Read channel		*/
	SCC_AD_RD,		/* SCC_AD Read channel			*/
	SCC_AD_WR,		/* SCC_AD Write channel			*/
	SCC_ABP_RD,		/* SCC_ABP Read channel			*/
	SCC_ABP_WR,		/* SCC_ABP Write channel		*/
	SCC_EBI_RW,		/* SCC_EBI Read/Write channel		*/
	SCC_USB_RW,		/* SCC_USB Read/Write channel		*/
	SCC_CPU1_SPDMA_RW,	/* SCC_CPU1_SPDMA Read/Write channel	*/
	SCC_CPU1_BRIDGE_RW,	/* SCC_CPU1_BRIDGE Read/Write channel	*/
	SCC_MAX			/* maximum limit on the SCC id		*/
};

int scc_set_usb_address_generation_mode(u32 agu_mode);
int scc_dma_cmd(enum scc_id id, u32 cmd, u32 dma_id, u32 drs);
int scc_setup_dma(enum scc_id id, u32 buffer_tag,
		  u32 type, u32 fh_mode, u32 drs, u32 dma_id);
int scc_enable(enum scc_id id, u32 value);
int scc_reset(enum scc_id id, u32 value);

#endif /* _SCC_H */
