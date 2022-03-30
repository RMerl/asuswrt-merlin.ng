/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Fuzhou Rockchip Electronics Co., Ltd
 * author: Eric Gao <eric.gao@rock-chips.com>
 */

#ifndef ROCKCHIP_MIPI_DSI_H
#define ROCKCHIP_MIPI_DSI_H

/*
 * All these mipi controller register declaration provide reg address offset,
 * bits width, bit offset for a specified register bits. With these message, we
 * can set or clear every bits individually for a 32bit widthregister. We use
 * DSI_HOST_BITS macro definition to combinat these message using the following
 * format: val(32bit) = addr(16bit) | width(8bit) | offest(8bit)
 * For example:
 *    #define SHUTDOWNZ           DSI_HOST_BITS(0x004, 1, 0)
 * means SHUTDOWNZ is a signal reg bit with bit offset qual 0,and it's reg addr
 * offset is 0x004.The conbinat result  = (0x004 << 16) | (1 << 8) | 0
 */
#define ADDR_SHIFT 16
#define BITS_SHIFT 8
#define OFFSET_SHIFT 0
#define DSI_HOST_BITS(addr, bits, bit_offset) \
((addr << ADDR_SHIFT) | (bits << BITS_SHIFT) | (bit_offset << OFFSET_SHIFT))

/* DWC_DSI_VERSION_0x3133302A */
#define VERSION				DSI_HOST_BITS(0x000, 32, 0)
#define SHUTDOWNZ			DSI_HOST_BITS(0x004, 1, 0)
#define TO_CLK_DIVISION		DSI_HOST_BITS(0x008, 8, 8)
#define TX_ESC_CLK_DIVISION	DSI_HOST_BITS(0x008, 8, 0)
#define DPI_VCID			DSI_HOST_BITS(0x00c, 2, 0)
#define EN18_LOOSELY		DSI_HOST_BITS(0x010, 1, 8)
#define DPI_COLOR_CODING	DSI_HOST_BITS(0x010, 4, 0)
#define COLORM_ACTIVE_LOW	DSI_HOST_BITS(0x014, 1, 4)
#define SHUTD_ACTIVE_LOW	DSI_HOST_BITS(0x014, 1, 3)
#define HSYNC_ACTIVE_LOW	DSI_HOST_BITS(0x014, 1, 2)
#define VSYNC_ACTIVE_LOW	DSI_HOST_BITS(0x014, 1, 1)
#define DATAEN_ACTIVE_LOW	DSI_HOST_BITS(0x014, 1, 0)
#define OUTVACT_LPCMD_TIME	DSI_HOST_BITS(0x018, 8, 16)
#define INVACT_LPCMD_TIME	DSI_HOST_BITS(0x018, 8, 0)
#define CRC_RX_EN			DSI_HOST_BITS(0x02c, 1, 4)
#define ECC_RX_EN			DSI_HOST_BITS(0x02c, 1, 3)
#define BTA_EN				DSI_HOST_BITS(0x02c, 1, 2)
#define EOTP_RX_EN			DSI_HOST_BITS(0x02c, 1, 1)
#define EOTP_TX_EN			DSI_HOST_BITS(0x02c, 1, 0)
#define GEN_VID_RX			DSI_HOST_BITS(0x030, 2, 0)
#define CMD_VIDEO_MODE		DSI_HOST_BITS(0x034, 1, 0)
#define VPG_ORIENTATION		DSI_HOST_BITS(0x038, 1, 24)
#define VPG_MODE			DSI_HOST_BITS(0x038, 1, 20)
#define VPG_EN				DSI_HOST_BITS(0x038, 1, 16)
#define LP_CMD_EN			DSI_HOST_BITS(0x038, 1, 15)
#define FRAME_BTA_ACK_EN	DSI_HOST_BITS(0x038, 1, 14)
#define LP_HFP_EN			DSI_HOST_BITS(0x038, 1, 13)
#define LP_HBP_EN			DSI_HOST_BITS(0x038, 1, 12)
#define LP_VACT_EN			DSI_HOST_BITS(0x038, 1, 11)
#define LP_VFP_EN			DSI_HOST_BITS(0x038, 1, 10)
#define LP_VBP_EN			DSI_HOST_BITS(0x038, 1, 9)
#define LP_VSA_EN			DSI_HOST_BITS(0x038, 1, 8)
#define VID_MODE_TYPE		DSI_HOST_BITS(0x038, 2, 0)
#define VID_PKT_SIZE		DSI_HOST_BITS(0x03c, 14, 0)
#define NUM_CHUNKS			DSI_HOST_BITS(0x040, 13, 0)
#define NULL_PKT_SIZE		DSI_HOST_BITS(0x044, 13, 0)
#define VID_HSA_TIME		DSI_HOST_BITS(0x048, 12, 0)
#define VID_HBP_TIME		DSI_HOST_BITS(0x04c, 12, 0)
#define VID_HLINE_TIME		DSI_HOST_BITS(0x050, 15, 0)
#define VID_VSA_LINES		DSI_HOST_BITS(0x054, 10, 0)
#define VID_VBP_LINES		DSI_HOST_BITS(0x058, 10, 0)
#define VID_VFP_LINES		DSI_HOST_BITS(0x05c, 10, 0)
#define VID_ACTIVE_LINES	DSI_HOST_BITS(0x060, 14, 0)
#define EDPI_CMD_SIZE		DSI_HOST_BITS(0x064, 16, 0)
#define MAX_RD_PKT_SIZE		DSI_HOST_BITS(0x068, 1, 24)
#define DCS_LW_TX			DSI_HOST_BITS(0x068, 1, 19)
#define DCS_SR_0P_TX		DSI_HOST_BITS(0x068, 1, 18)
#define DCS_SW_1P_TX		DSI_HOST_BITS(0x068, 1, 17)
#define DCS_SW_0P_TX		DSI_HOST_BITS(0x068, 1, 16)
#define GEN_LW_TX			DSI_HOST_BITS(0x068, 1, 14)
#define GEN_SR_2P_TX		DSI_HOST_BITS(0x068, 1, 13)
#define GEN_SR_1P_TX		DSI_HOST_BITS(0x068, 1, 12)
#define GEN_SR_0P_TX		DSI_HOST_BITS(0x068, 1, 11)
#define GEN_SW_2P_TX		DSI_HOST_BITS(0x068, 1, 10)
#define GEN_SW_1P_TX		DSI_HOST_BITS(0x068, 1, 9)
#define GEN_SW_0P_TX		DSI_HOST_BITS(0x068, 1, 8)
#define ACK_RQST_EN			DSI_HOST_BITS(0x068, 1, 1)
#define TEAR_FX_EN			DSI_HOST_BITS(0x068, 1, 0)
#define GEN_WC_MSBYTE		DSI_HOST_BITS(0x06c, 14, 16)
#define GEN_WC_LSBYTE		DSI_HOST_BITS(0x06c, 8, 8)
#define GEN_VC				DSI_HOST_BITS(0x06c, 2, 6)
#define GEN_DT				DSI_HOST_BITS(0x06c, 6, 0)
#define GEN_PLD_DATA		DSI_HOST_BITS(0x070, 32, 0)
#define GEN_RD_CMD_BUSY		DSI_HOST_BITS(0x074, 1, 6)
#define GEN_PLD_R_FULL		DSI_HOST_BITS(0x074, 1, 5)
#define GEN_PLD_R_EMPTY		DSI_HOST_BITS(0x074, 1, 4)
#define GEN_PLD_W_FULL		DSI_HOST_BITS(0x074, 1, 3)
#define GEN_PLD_W_EMPTY		DSI_HOST_BITS(0x074, 1, 2)
#define GEN_CMD_FULL		DSI_HOST_BITS(0x074, 1, 1)
#define GEN_CMD_EMPTY		DSI_HOST_BITS(0x074, 1, 0)
#define HSTX_TO_CNT			DSI_HOST_BITS(0x078, 16, 16)
#define LPRX_TO_CNT			DSI_HOST_BITS(0x078, 16, 0)
#define HS_RD_TO_CNT		DSI_HOST_BITS(0x07c, 16, 0)
#define LP_RD_TO_CNT		DSI_HOST_BITS(0x080, 16, 0)
#define PRESP_TO_MODE		DSI_HOST_BITS(0x084, 1, 24)
#define HS_WR_TO_CNT		DSI_HOST_BITS(0x084, 16, 0)
#define LP_WR_TO_CNT		DSI_HOST_BITS(0x088, 16, 0)
#define BTA_TO_CNT			DSI_HOST_BITS(0x08c, 16, 0)
#define AUTO_CLKLANE_CTRL	DSI_HOST_BITS(0x094, 1, 1)
#define PHY_TXREQUESTCLKHS	DSI_HOST_BITS(0x094, 1, 0)
#define PHY_HS2LP_TIME_CLK_LANE	DSI_HOST_BITS(0x098, 10, 16)
#define PHY_HS2HS_TIME_CLK_LANE	DSI_HOST_BITS(0x098, 10, 0)
#define PHY_HS2LP_TIME		DSI_HOST_BITS(0x09c, 8, 24)
#define PHY_LP2HS_TIME		DSI_HOST_BITS(0x09c, 8, 16)
#define MAX_RD_TIME			DSI_HOST_BITS(0x09c, 15, 0)
#define PHY_FORCEPLL		DSI_HOST_BITS(0x0a0, 1, 3)
#define PHY_ENABLECLK		DSI_HOST_BITS(0x0a0, 1, 2)
#define PHY_RSTZ			DSI_HOST_BITS(0x0a0, 1, 1)
#define PHY_SHUTDOWNZ		DSI_HOST_BITS(0x0a0, 1, 0)
#define PHY_STOP_WAIT_TIME	DSI_HOST_BITS(0x0a4, 8, 8)
#define N_LANES				DSI_HOST_BITS(0x0a4, 2, 0)
#define PHY_TXEXITULPSLAN	DSI_HOST_BITS(0x0a8, 1, 3)
#define PHY_TXREQULPSLAN	DSI_HOST_BITS(0x0a8, 1, 2)
#define PHY_TXEXITULPSCLK	DSI_HOST_BITS(0x0a8, 1, 1)
#define PHY_TXREQULPSCLK	DSI_HOST_BITS(0x0a8, 1, 0)
#define PHY_TX_TRIGGERS		DSI_HOST_BITS(0x0ac, 4, 0)
#define PHYSTOPSTATECLKLANE	DSI_HOST_BITS(0x0b0, 1, 2)
#define PHYLOCK				DSI_HOST_BITS(0x0b0, 1, 0)
#define PHY_TESTCLK			DSI_HOST_BITS(0x0b4, 1, 1)
#define PHY_TESTCLR			DSI_HOST_BITS(0x0b4, 1, 0)
#define PHY_TESTEN			DSI_HOST_BITS(0x0b8, 1, 16)
#define PHY_TESTDOUT		DSI_HOST_BITS(0x0b8, 8, 8)
#define PHY_TESTDIN			DSI_HOST_BITS(0x0b8, 8, 0)
#define PHY_TEST_CTRL1		DSI_HOST_BITS(0x0b8, 17, 0)
#define PHY_TEST_CTRL0		DSI_HOST_BITS(0x0b4, 2, 0)
#define INT_ST0				DSI_HOST_BITS(0x0bc, 21, 0)
#define INT_ST1				DSI_HOST_BITS(0x0c0, 18, 0)
#define INT_MKS0			DSI_HOST_BITS(0x0c4, 21, 0)
#define INT_MKS1			DSI_HOST_BITS(0x0c8, 18, 0)
#define INT_FORCE0			DSI_HOST_BITS(0x0d8, 21, 0)
#define INT_FORCE1			DSI_HOST_BITS(0x0dc, 18, 0)

#define CODE_HS_RX_CLOCK	0x34
#define CODE_HS_RX_LANE0	0x44
#define CODE_HS_RX_LANE1	0x54
#define CODE_HS_RX_LANE2	0x84
#define CODE_HS_RX_LANE3	0x94

#define CODE_PLL_VCORANGE_VCOCAP	0x10
#define CODE_PLL_CPCTRL	0x11
#define CODE_PLL_LPF_CP 0x12
#define CODE_PLL_INPUT_DIV_RAT	0x17
#define CODE_PLL_LOOP_DIV_RAT	0x18
#define CODE_PLL_INPUT_LOOP_DIV_RAT	0x19
#define CODE_BANDGAP_BIAS_CTRL	0x20
#define CODE_TERMINATION_CTRL	0x21
#define CODE_AFE_BIAS_BANDGAP_ANOLOG 0x22

#define CODE_HSTXDATALANEREQUSETSTATETIME	0x70
#define CODE_HSTXDATALANEPREPARESTATETIME	0x71
#define CODE_HSTXDATALANEHSZEROSTATETIME	0x72

/* Transmission mode between vop and MIPI controller */
enum vid_mode_type_t {
	NON_BURST_SYNC_PLUSE = 0,
	NON_BURST_SYNC_EVENT,
	BURST_MODE,
};

enum cmd_video_mode {
	VIDEO_MODE = 0,
	CMD_MODE,
};

/* Indicate MIPI DSI color mode */
enum dpi_color_coding {
	DPI_16BIT_CFG_1 = 0,
	DPI_16BIT_CFG_2,
	DPI_16BIT_CFG_3,
	DPI_18BIT_CFG_1,
	DPI_18BIT_CFG_2,
	DPI_24BIT,
	DPI_20BIT_YCBCR_422_LP,
	DPI_24BIT_YCBCR_422,
	DPI_16BIT_YCBCR_422,
	DPI_30BIT,
	DPI_36BIT,
	DPI_12BIT_YCBCR_420,
};

/* Indicate which VOP the MIPI DSI use, bit or little one */
enum  vop_id {
	VOP_B = 0,
	VOP_L,
};

#endif /* end of ROCKCHIP_MIPI_DSI_H */
