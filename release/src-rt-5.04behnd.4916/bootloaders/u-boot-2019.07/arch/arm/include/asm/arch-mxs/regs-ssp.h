/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 SSP Register Definitions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on code from LTIB:
 * Copyright 2008-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#ifndef __MX28_REGS_SSP_H__
#define __MX28_REGS_SSP_H__

#include <asm/mach-imx/regs-common.h>

#ifndef	__ASSEMBLY__
#if defined(CONFIG_MX23)
struct mxs_ssp_regs {
	mxs_reg_32(hw_ssp_ctrl0)
	mxs_reg_32(hw_ssp_cmd0)
	mxs_reg_32(hw_ssp_cmd1)
	mxs_reg_32(hw_ssp_compref)
	mxs_reg_32(hw_ssp_compmask)
	mxs_reg_32(hw_ssp_timing)
	mxs_reg_32(hw_ssp_ctrl1)
	mxs_reg_32(hw_ssp_data)
	mxs_reg_32(hw_ssp_sdresp0)
	mxs_reg_32(hw_ssp_sdresp1)
	mxs_reg_32(hw_ssp_sdresp2)
	mxs_reg_32(hw_ssp_sdresp3)
	mxs_reg_32(hw_ssp_status)

	uint32_t	reserved1[12];

	mxs_reg_32(hw_ssp_debug)
	mxs_reg_32(hw_ssp_version)
};
#elif defined(CONFIG_MX28)
struct mxs_ssp_regs {
	mxs_reg_32(hw_ssp_ctrl0)
	mxs_reg_32(hw_ssp_cmd0)
	mxs_reg_32(hw_ssp_cmd1)
	mxs_reg_32(hw_ssp_xfer_size)
	mxs_reg_32(hw_ssp_block_size)
	mxs_reg_32(hw_ssp_compref)
	mxs_reg_32(hw_ssp_compmask)
	mxs_reg_32(hw_ssp_timing)
	mxs_reg_32(hw_ssp_ctrl1)
	mxs_reg_32(hw_ssp_data)
	mxs_reg_32(hw_ssp_sdresp0)
	mxs_reg_32(hw_ssp_sdresp1)
	mxs_reg_32(hw_ssp_sdresp2)
	mxs_reg_32(hw_ssp_sdresp3)
	mxs_reg_32(hw_ssp_ddr_ctrl)
	mxs_reg_32(hw_ssp_dll_ctrl)
	mxs_reg_32(hw_ssp_status)
	mxs_reg_32(hw_ssp_dll_sts)
	mxs_reg_32(hw_ssp_debug)
	mxs_reg_32(hw_ssp_version)
};
#endif

static inline int mxs_ssp_bus_id_valid(int bus)
{
#if defined(CONFIG_MX23)
	const unsigned int mxs_ssp_chan_count = 2;
#elif defined(CONFIG_MX28)
	const unsigned int mxs_ssp_chan_count = 4;
#endif

	if (bus >= mxs_ssp_chan_count)
		return 0;

	if (bus < 0)
		return 0;

	return 1;
}

static inline int mxs_ssp_clock_by_bus(unsigned int clock)
{
#if defined(CONFIG_MX23)
	return 0;
#elif defined(CONFIG_MX28)
	return clock;
#endif
}

static inline struct mxs_ssp_regs *mxs_ssp_regs_by_bus(unsigned int port)
{
	switch (port) {
	case 0:
		return (struct mxs_ssp_regs *)MXS_SSP0_BASE;
	case 1:
		return (struct mxs_ssp_regs *)MXS_SSP1_BASE;
#ifdef CONFIG_MX28
	case 2:
		return (struct mxs_ssp_regs *)MXS_SSP2_BASE;
	case 3:
		return (struct mxs_ssp_regs *)MXS_SSP3_BASE;
#endif
	default:
		return NULL;
	}
}
#endif

#define	SSP_CTRL0_SFTRST			(1 << 31)
#define	SSP_CTRL0_CLKGATE			(1 << 30)
#define	SSP_CTRL0_RUN				(1 << 29)
#define	SSP_CTRL0_SDIO_IRQ_CHECK		(1 << 28)
#define	SSP_CTRL0_LOCK_CS			(1 << 27)
#define	SSP_CTRL0_IGNORE_CRC			(1 << 26)
#define	SSP_CTRL0_READ				(1 << 25)
#define	SSP_CTRL0_DATA_XFER			(1 << 24)
#define	SSP_CTRL0_BUS_WIDTH_MASK		(0x3 << 22)
#define	SSP_CTRL0_BUS_WIDTH_OFFSET		22
#define	SSP_CTRL0_BUS_WIDTH_ONE_BIT		(0x0 << 22)
#define	SSP_CTRL0_BUS_WIDTH_FOUR_BIT		(0x1 << 22)
#define	SSP_CTRL0_BUS_WIDTH_EIGHT_BIT		(0x2 << 22)
#define	SSP_CTRL0_WAIT_FOR_IRQ			(1 << 21)
#define	SSP_CTRL0_WAIT_FOR_CMD			(1 << 20)
#define	SSP_CTRL0_LONG_RESP			(1 << 19)
#define	SSP_CTRL0_CHECK_RESP			(1 << 18)
#define	SSP_CTRL0_GET_RESP			(1 << 17)
#define	SSP_CTRL0_ENABLE			(1 << 16)

#ifdef CONFIG_MX23
#define	SSP_CTRL0_XFER_COUNT_OFFSET		0
#define	SSP_CTRL0_XFER_COUNT_MASK		0xffff
#endif

#define	SSP_CMD0_SOFT_TERMINATE			(1 << 26)
#define	SSP_CMD0_DBL_DATA_RATE_EN		(1 << 25)
#define	SSP_CMD0_PRIM_BOOT_OP_EN		(1 << 24)
#define	SSP_CMD0_BOOT_ACK_EN			(1 << 23)
#define	SSP_CMD0_SLOW_CLKING_EN			(1 << 22)
#define	SSP_CMD0_CONT_CLKING_EN			(1 << 21)
#define	SSP_CMD0_APPEND_8CYC			(1 << 20)
#if defined(CONFIG_MX23)
#define	SSP_CMD0_BLOCK_SIZE_MASK		(0xf << 16)
#define	SSP_CMD0_BLOCK_SIZE_OFFSET		16
#define	SSP_CMD0_BLOCK_COUNT_MASK		(0xff << 8)
#define	SSP_CMD0_BLOCK_COUNT_OFFSET		8
#endif
#define	SSP_CMD0_CMD_MASK			0xff
#define	SSP_CMD0_CMD_OFFSET			0
#define	SSP_CMD0_CMD_MMC_GO_IDLE_STATE		0x00
#define	SSP_CMD0_CMD_MMC_SEND_OP_COND		0x01
#define	SSP_CMD0_CMD_MMC_ALL_SEND_CID		0x02
#define	SSP_CMD0_CMD_MMC_SET_RELATIVE_ADDR	0x03
#define	SSP_CMD0_CMD_MMC_SET_DSR		0x04
#define	SSP_CMD0_CMD_MMC_RESERVED_5		0x05
#define	SSP_CMD0_CMD_MMC_SWITCH			0x06
#define	SSP_CMD0_CMD_MMC_SELECT_DESELECT_CARD	0x07
#define	SSP_CMD0_CMD_MMC_SEND_EXT_CSD		0x08
#define	SSP_CMD0_CMD_MMC_SEND_CSD		0x09
#define	SSP_CMD0_CMD_MMC_SEND_CID		0x0a
#define	SSP_CMD0_CMD_MMC_READ_DAT_UNTIL_STOP	0x0b
#define	SSP_CMD0_CMD_MMC_STOP_TRANSMISSION	0x0c
#define	SSP_CMD0_CMD_MMC_SEND_STATUS		0x0d
#define	SSP_CMD0_CMD_MMC_BUSTEST_R		0x0e
#define	SSP_CMD0_CMD_MMC_GO_INACTIVE_STATE	0x0f
#define	SSP_CMD0_CMD_MMC_SET_BLOCKLEN		0x10
#define	SSP_CMD0_CMD_MMC_READ_SINGLE_BLOCK	0x11
#define	SSP_CMD0_CMD_MMC_READ_MULTIPLE_BLOCK	0x12
#define	SSP_CMD0_CMD_MMC_BUSTEST_W		0x13
#define	SSP_CMD0_CMD_MMC_WRITE_DAT_UNTIL_STOP	0x14
#define	SSP_CMD0_CMD_MMC_SET_BLOCK_COUNT	0x17
#define	SSP_CMD0_CMD_MMC_WRITE_BLOCK		0x18
#define	SSP_CMD0_CMD_MMC_WRITE_MULTIPLE_BLOCK	0x19
#define	SSP_CMD0_CMD_MMC_PROGRAM_CID		0x1a
#define	SSP_CMD0_CMD_MMC_PROGRAM_CSD		0x1b
#define	SSP_CMD0_CMD_MMC_SET_WRITE_PROT		0x1c
#define	SSP_CMD0_CMD_MMC_CLR_WRITE_PROT		0x1d
#define	SSP_CMD0_CMD_MMC_SEND_WRITE_PROT	0x1e
#define	SSP_CMD0_CMD_MMC_ERASE_GROUP_START	0x23
#define	SSP_CMD0_CMD_MMC_ERASE_GROUP_END	0x24
#define	SSP_CMD0_CMD_MMC_ERASE			0x26
#define	SSP_CMD0_CMD_MMC_FAST_IO		0x27
#define	SSP_CMD0_CMD_MMC_GO_IRQ_STATE		0x28
#define	SSP_CMD0_CMD_MMC_LOCK_UNLOCK		0x2a
#define	SSP_CMD0_CMD_MMC_APP_CMD		0x37
#define	SSP_CMD0_CMD_MMC_GEN_CMD		0x38
#define	SSP_CMD0_CMD_SD_GO_IDLE_STATE		0x00
#define	SSP_CMD0_CMD_SD_ALL_SEND_CID		0x02
#define	SSP_CMD0_CMD_SD_SEND_RELATIVE_ADDR	0x03
#define	SSP_CMD0_CMD_SD_SET_DSR			0x04
#define	SSP_CMD0_CMD_SD_IO_SEND_OP_COND		0x05
#define	SSP_CMD0_CMD_SD_SELECT_DESELECT_CARD	0x07
#define	SSP_CMD0_CMD_SD_SEND_CSD		0x09
#define	SSP_CMD0_CMD_SD_SEND_CID		0x0a
#define	SSP_CMD0_CMD_SD_STOP_TRANSMISSION	0x0c
#define	SSP_CMD0_CMD_SD_SEND_STATUS		0x0d
#define	SSP_CMD0_CMD_SD_GO_INACTIVE_STATE	0x0f
#define	SSP_CMD0_CMD_SD_SET_BLOCKLEN		0x10
#define	SSP_CMD0_CMD_SD_READ_SINGLE_BLOCK	0x11
#define	SSP_CMD0_CMD_SD_READ_MULTIPLE_BLOCK	0x12
#define	SSP_CMD0_CMD_SD_WRITE_BLOCK		0x18
#define	SSP_CMD0_CMD_SD_WRITE_MULTIPLE_BLOCK	0x19
#define	SSP_CMD0_CMD_SD_PROGRAM_CSD		0x1b
#define	SSP_CMD0_CMD_SD_SET_WRITE_PROT		0x1c
#define	SSP_CMD0_CMD_SD_CLR_WRITE_PROT		0x1d
#define	SSP_CMD0_CMD_SD_SEND_WRITE_PROT		0x1e
#define	SSP_CMD0_CMD_SD_ERASE_WR_BLK_START	0x20
#define	SSP_CMD0_CMD_SD_ERASE_WR_BLK_END	0x21
#define	SSP_CMD0_CMD_SD_ERASE_GROUP_START	0x23
#define	SSP_CMD0_CMD_SD_ERASE_GROUP_END		0x24
#define	SSP_CMD0_CMD_SD_ERASE			0x26
#define	SSP_CMD0_CMD_SD_LOCK_UNLOCK		0x2a
#define	SSP_CMD0_CMD_SD_IO_RW_DIRECT		0x34
#define	SSP_CMD0_CMD_SD_IO_RW_EXTENDED		0x35
#define	SSP_CMD0_CMD_SD_APP_CMD			0x37
#define	SSP_CMD0_CMD_SD_GEN_CMD			0x38

#define	SSP_CMD1_CMD_ARG_MASK			0xffffffff
#define	SSP_CMD1_CMD_ARG_OFFSET			0

#if defined(CONFIG_MX28)
#define	SSP_XFER_SIZE_XFER_COUNT_MASK		0xffffffff
#define	SSP_XFER_SIZE_XFER_COUNT_OFFSET		0

#define	SSP_BLOCK_SIZE_BLOCK_COUNT_MASK		(0xffffff << 4)
#define	SSP_BLOCK_SIZE_BLOCK_COUNT_OFFSET	4
#define	SSP_BLOCK_SIZE_BLOCK_SIZE_MASK		0xf
#define	SSP_BLOCK_SIZE_BLOCK_SIZE_OFFSET	0
#endif

#define	SSP_COMPREF_REFERENCE_MASK		0xffffffff
#define	SSP_COMPREF_REFERENCE_OFFSET		0

#define	SSP_COMPMASK_MASK_MASK			0xffffffff
#define	SSP_COMPMASK_MASK_OFFSET		0

#define	SSP_TIMING_TIMEOUT_MASK			(0xffff << 16)
#define	SSP_TIMING_TIMEOUT_OFFSET		16
#define	SSP_TIMING_CLOCK_DIVIDE_MASK		(0xff << 8)
#define	SSP_TIMING_CLOCK_DIVIDE_OFFSET		8
#define	SSP_TIMING_CLOCK_RATE_MASK		0xff
#define	SSP_TIMING_CLOCK_RATE_OFFSET		0

#define	SSP_CTRL1_SDIO_IRQ			(1 << 31)
#define	SSP_CTRL1_SDIO_IRQ_EN			(1 << 30)
#define	SSP_CTRL1_RESP_ERR_IRQ			(1 << 29)
#define	SSP_CTRL1_RESP_ERR_IRQ_EN		(1 << 28)
#define	SSP_CTRL1_RESP_TIMEOUT_IRQ		(1 << 27)
#define	SSP_CTRL1_RESP_TIMEOUT_IRQ_EN		(1 << 26)
#define	SSP_CTRL1_DATA_TIMEOUT_IRQ		(1 << 25)
#define	SSP_CTRL1_DATA_TIMEOUT_IRQ_EN		(1 << 24)
#define	SSP_CTRL1_DATA_CRC_IRQ			(1 << 23)
#define	SSP_CTRL1_DATA_CRC_IRQ_EN		(1 << 22)
#define	SSP_CTRL1_FIFO_UNDERRUN_IRQ		(1 << 21)
#define	SSP_CTRL1_FIFO_UNDERRUN_EN		(1 << 20)
#define	SSP_CTRL1_CEATA_CCS_ERR_IRQ		(1 << 19)
#define	SSP_CTRL1_CEATA_CCS_ERR_IRQ_EN		(1 << 18)
#define	SSP_CTRL1_RECV_TIMEOUT_IRQ		(1 << 17)
#define	SSP_CTRL1_RECV_TIMEOUT_IRQ_EN		(1 << 16)
#define	SSP_CTRL1_FIFO_OVERRUN_IRQ		(1 << 15)
#define	SSP_CTRL1_FIFO_OVERRUN_IRQ_EN		(1 << 14)
#define	SSP_CTRL1_DMA_ENABLE			(1 << 13)
#define	SSP_CTRL1_CEATA_CCS_ERR_EN		(1 << 12)
#define	SSP_CTRL1_SLAVE_OUT_DISABLE		(1 << 11)
#define	SSP_CTRL1_PHASE				(1 << 10)
#define	SSP_CTRL1_POLARITY			(1 << 9)
#define	SSP_CTRL1_SLAVE_MODE			(1 << 8)
#define	SSP_CTRL1_WORD_LENGTH_MASK		(0xf << 4)
#define	SSP_CTRL1_WORD_LENGTH_OFFSET		4
#define	SSP_CTRL1_WORD_LENGTH_RESERVED0		(0x0 << 4)
#define	SSP_CTRL1_WORD_LENGTH_RESERVED1		(0x1 << 4)
#define	SSP_CTRL1_WORD_LENGTH_RESERVED2		(0x2 << 4)
#define	SSP_CTRL1_WORD_LENGTH_FOUR_BITS		(0x3 << 4)
#define	SSP_CTRL1_WORD_LENGTH_EIGHT_BITS	(0x7 << 4)
#define	SSP_CTRL1_WORD_LENGTH_SIXTEEN_BITS	(0xf << 4)
#define	SSP_CTRL1_SSP_MODE_MASK			0xf
#define	SSP_CTRL1_SSP_MODE_OFFSET		0
#define	SSP_CTRL1_SSP_MODE_SPI			0x0
#define	SSP_CTRL1_SSP_MODE_SSI			0x1
#define	SSP_CTRL1_SSP_MODE_SD_MMC		0x3
#define	SSP_CTRL1_SSP_MODE_MS			0x4

#define	SSP_DATA_DATA_MASK			0xffffffff
#define	SSP_DATA_DATA_OFFSET			0

#define	SSP_SDRESP0_RESP0_MASK			0xffffffff
#define	SSP_SDRESP0_RESP0_OFFSET		0

#define	SSP_SDRESP1_RESP1_MASK			0xffffffff
#define	SSP_SDRESP1_RESP1_OFFSET		0

#define	SSP_SDRESP2_RESP2_MASK			0xffffffff
#define	SSP_SDRESP2_RESP2_OFFSET		0

#define	SSP_SDRESP3_RESP3_MASK			0xffffffff
#define	SSP_SDRESP3_RESP3_OFFSET		0

#define	SSP_DDR_CTRL_DMA_BURST_TYPE_MASK	(0x3 << 30)
#define	SSP_DDR_CTRL_DMA_BURST_TYPE_OFFSET	30
#define	SSP_DDR_CTRL_NIBBLE_POS			(1 << 1)
#define	SSP_DDR_CTRL_TXCLK_DELAY_TYPE		(1 << 0)

#define	SSP_DLL_CTRL_REF_UPDATE_INT_MASK	(0xf << 28)
#define	SSP_DLL_CTRL_REF_UPDATE_INT_OFFSET	28
#define	SSP_DLL_CTRL_SLV_UPDATE_INT_MASK	(0xff << 20)
#define	SSP_DLL_CTRL_SLV_UPDATE_INT_OFFSET	20
#define	SSP_DLL_CTRL_SLV_OVERRIDE_VAL_MASK	(0x3f << 10)
#define	SSP_DLL_CTRL_SLV_OVERRIDE_VAL_OFFSET	10
#define	SSP_DLL_CTRL_SLV_OVERRIDE		(1 << 9)
#define	SSP_DLL_CTRL_GATE_UPDATE		(1 << 7)
#define	SSP_DLL_CTRL_SLV_DLY_TARGET_MASK	(0xf << 3)
#define	SSP_DLL_CTRL_SLV_DLY_TARGET_OFFSET	3
#define	SSP_DLL_CTRL_SLV_FORCE_UPD		(1 << 2)
#define	SSP_DLL_CTRL_RESET			(1 << 1)
#define	SSP_DLL_CTRL_ENABLE			(1 << 0)

#define	SSP_STATUS_PRESENT			(1 << 31)
#define	SSP_STATUS_MS_PRESENT			(1 << 30)
#define	SSP_STATUS_SD_PRESENT			(1 << 29)
#define	SSP_STATUS_CARD_DETECT			(1 << 28)
#define	SSP_STATUS_DMABURST			(1 << 22)
#define	SSP_STATUS_DMASENSE			(1 << 21)
#define	SSP_STATUS_DMATERM			(1 << 20)
#define	SSP_STATUS_DMAREQ			(1 << 19)
#define	SSP_STATUS_DMAEND			(1 << 18)
#define	SSP_STATUS_SDIO_IRQ			(1 << 17)
#define	SSP_STATUS_RESP_CRC_ERR			(1 << 16)
#define	SSP_STATUS_RESP_ERR			(1 << 15)
#define	SSP_STATUS_RESP_TIMEOUT			(1 << 14)
#define	SSP_STATUS_DATA_CRC_ERR			(1 << 13)
#define	SSP_STATUS_TIMEOUT			(1 << 12)
#define	SSP_STATUS_RECV_TIMEOUT_STAT		(1 << 11)
#define	SSP_STATUS_CEATA_CCS_ERR		(1 << 10)
#define	SSP_STATUS_FIFO_OVRFLW			(1 << 9)
#define	SSP_STATUS_FIFO_FULL			(1 << 8)
#define	SSP_STATUS_FIFO_EMPTY			(1 << 5)
#define	SSP_STATUS_FIFO_UNDRFLW			(1 << 4)
#define	SSP_STATUS_CMD_BUSY			(1 << 3)
#define	SSP_STATUS_DATA_BUSY			(1 << 2)
#define	SSP_STATUS_BUSY				(1 << 0)

#define	SSP_DLL_STS_REF_SEL_MASK		(0x3f << 8)
#define	SSP_DLL_STS_REF_SEL_OFFSET		8
#define	SSP_DLL_STS_SLV_SEL_MASK		(0x3f << 2)
#define	SSP_DLL_STS_SLV_SEL_OFFSET		2
#define	SSP_DLL_STS_REF_LOCK			(1 << 1)
#define	SSP_DLL_STS_SLV_LOCK			(1 << 0)

#define	SSP_DEBUG_DATACRC_ERR_MASK		(0xf << 28)
#define	SSP_DEBUG_DATACRC_ERR_OFFSET		28
#define	SSP_DEBUG_DATA_STALL			(1 << 27)
#define	SSP_DEBUG_DAT_SM_MASK			(0x7 << 24)
#define	SSP_DEBUG_DAT_SM_OFFSET			24
#define	SSP_DEBUG_DAT_SM_DSM_IDLE		(0x0 << 24)
#define	SSP_DEBUG_DAT_SM_DSM_WORD		(0x2 << 24)
#define	SSP_DEBUG_DAT_SM_DSM_CRC1		(0x3 << 24)
#define	SSP_DEBUG_DAT_SM_DSM_CRC2		(0x4 << 24)
#define	SSP_DEBUG_DAT_SM_DSM_END		(0x5 << 24)
#define	SSP_DEBUG_MSTK_SM_MASK			(0xf << 20)
#define	SSP_DEBUG_MSTK_SM_OFFSET		20
#define	SSP_DEBUG_MSTK_SM_MSTK_IDLE		(0x0 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_CKON		(0x1 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_BS1		(0x2 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_TPC		(0x3 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_BS2		(0x4 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_HDSHK		(0x5 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_BS3		(0x6 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_RW		(0x7 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_CRC1		(0x8 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_CRC2		(0x9 << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_BS0		(0xa << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_END1		(0xb << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_END2W		(0xc << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_END2R		(0xd << 20)
#define	SSP_DEBUG_MSTK_SM_MSTK_DONE		(0xe << 20)
#define	SSP_DEBUG_CMD_OE			(1 << 19)
#define	SSP_DEBUG_DMA_SM_MASK			(0x7 << 16)
#define	SSP_DEBUG_DMA_SM_OFFSET			16
#define	SSP_DEBUG_DMA_SM_DMA_IDLE		(0x0 << 16)
#define	SSP_DEBUG_DMA_SM_DMA_DMAREQ		(0x1 << 16)
#define	SSP_DEBUG_DMA_SM_DMA_DMAACK		(0x2 << 16)
#define	SSP_DEBUG_DMA_SM_DMA_STALL		(0x3 << 16)
#define	SSP_DEBUG_DMA_SM_DMA_BUSY		(0x4 << 16)
#define	SSP_DEBUG_DMA_SM_DMA_DONE		(0x5 << 16)
#define	SSP_DEBUG_DMA_SM_DMA_COUNT		(0x6 << 16)
#define	SSP_DEBUG_MMC_SM_MASK			(0xf << 12)
#define	SSP_DEBUG_MMC_SM_OFFSET			12
#define	SSP_DEBUG_MMC_SM_MMC_IDLE		(0x0 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_CMD		(0x1 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_TRC		(0x2 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_RESP		(0x3 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_RPRX		(0x4 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_TX			(0x5 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_CTOK		(0x6 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_RX			(0x7 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_CCS		(0x8 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_PUP		(0x9 << 12)
#define	SSP_DEBUG_MMC_SM_MMC_WAIT		(0xa << 12)
#define	SSP_DEBUG_CMD_SM_MASK			(0x3 << 10)
#define	SSP_DEBUG_CMD_SM_OFFSET			10
#define	SSP_DEBUG_CMD_SM_CSM_IDLE		(0x0 << 10)
#define	SSP_DEBUG_CMD_SM_CSM_INDEX		(0x1 << 10)
#define	SSP_DEBUG_CMD_SM_CSM_ARG		(0x2 << 10)
#define	SSP_DEBUG_CMD_SM_CSM_CRC		(0x3 << 10)
#define	SSP_DEBUG_SSP_CMD			(1 << 9)
#define	SSP_DEBUG_SSP_RESP			(1 << 8)
#define	SSP_DEBUG_SSP_RXD_MASK			0xff
#define	SSP_DEBUG_SSP_RXD_OFFSET		0

#define	SSP_VERSION_MAJOR_MASK			(0xff << 24)
#define	SSP_VERSION_MAJOR_OFFSET		24
#define	SSP_VERSION_MINOR_MASK			(0xff << 16)
#define	SSP_VERSION_MINOR_OFFSET		16
#define	SSP_VERSION_STEP_MASK			0xffff
#define	SSP_VERSION_STEP_OFFSET			0

#endif /* __MX28_REGS_SSP_H__ */
