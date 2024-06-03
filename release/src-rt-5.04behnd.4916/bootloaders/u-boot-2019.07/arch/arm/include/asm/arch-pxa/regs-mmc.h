/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 */

#ifndef __REGS_MMC_H__
#define __REGS_MMC_H__

#define MMC0_BASE	0x41100000
#define MMC1_BASE	0x42000000

int pxa_mmc_register(int card_index);

struct pxa_mmc_regs {
	uint32_t	strpcl;
	uint32_t	stat;
	uint32_t	clkrt;
	uint32_t	spi;
	uint32_t	cmdat;
	uint32_t	resto;
	uint32_t	rdto;
	uint32_t	blklen;
	uint32_t	nob;
	uint32_t	prtbuf;
	uint32_t	i_mask;
	uint32_t	i_reg;
	uint32_t	cmd;
	uint32_t	argh;
	uint32_t	argl;
	uint32_t	res;
	uint32_t	rxfifo;
	uint32_t	txfifo;
};

/* MMC_STRPCL */
#define MMC_STRPCL_STOP_CLK		(1 << 0)
#define MMC_STRPCL_START_CLK		(1 << 1)

/* MMC_STAT */
#define MMC_STAT_END_CMD_RES		(1 << 13)
#define MMC_STAT_PRG_DONE		(1 << 12)
#define MMC_STAT_DATA_TRAN_DONE		(1 << 11)
#define MMC_STAT_CLK_EN			(1 << 8)
#define MMC_STAT_RECV_FIFO_FULL		(1 << 7)
#define MMC_STAT_XMIT_FIFO_EMPTY	(1 << 6)
#define MMC_STAT_RES_CRC_ERROR		(1 << 5)
#define MMC_STAT_SPI_READ_ERROR_TOKEN	(1 << 4)
#define MMC_STAT_CRC_READ_ERROR		(1 << 3)
#define MMC_STAT_CRC_WRITE_ERROR	(1 << 2)
#define MMC_STAT_TIME_OUT_RESPONSE	(1 << 1)
#define MMC_STAT_READ_TIME_OUT		(1 << 0)

/* MMC_CLKRT */
#define MMC_CLKRT_20MHZ			0
#define MMC_CLKRT_10MHZ			1
#define MMC_CLKRT_5MHZ			2
#define MMC_CLKRT_2_5MHZ		3
#define MMC_CLKRT_1_25MHZ		4
#define MMC_CLKRT_0_625MHZ		5
#define MMC_CLKRT_0_3125MHZ		6

/* MMC_SPI */
#define MMC_SPI_EN			(1 << 0)
#define MMC_SPI_CS_EN			(1 << 2)
#define MMC_SPI_CS_ADDRESS		(1 << 3)
#define MMC_SPI_CRC_ON			(1 << 1)

/* MMC_CMDAT */
#define MMC_CMDAT_SD_4DAT		(1 << 8)
#define MMC_CMDAT_MMC_DMA_EN		(1 << 7)
#define MMC_CMDAT_INIT			(1 << 6)
#define MMC_CMDAT_BUSY			(1 << 5)
#define MMC_CMDAT_BCR			(MMC_CMDAT_BUSY | MMC_CMDAT_INIT)
#define MMC_CMDAT_STREAM		(1 << 4)
#define MMC_CMDAT_WRITE			(1 << 3)
#define MMC_CMDAT_DATA_EN		(1 << 2)
#define MMC_CMDAT_R0			0
#define MMC_CMDAT_R1			1
#define MMC_CMDAT_R2			2
#define MMC_CMDAT_R3			3

/* MMC_RESTO */
#define MMC_RES_TO_MAX_MASK		0x7f

/* MMC_RDTO */
#define MMC_READ_TO_MAX_MASK		0xffff

/* MMC_BLKLEN */
#define MMC_BLK_LEN_MAX_MASK		0x3ff

/* MMC_PRTBUF */
#define MMC_PRTBUF_BUF_PART_FULL	(1 << 0)

/* MMC_I_MASK */
#define MMC_I_MASK_TXFIFO_WR_REQ	(1 << 6)
#define MMC_I_MASK_RXFIFO_RD_REQ	(1 << 5)
#define MMC_I_MASK_CLK_IS_OFF		(1 << 4)
#define MMC_I_MASK_STOP_CMD		(1 << 3)
#define MMC_I_MASK_END_CMD_RES		(1 << 2)
#define MMC_I_MASK_PRG_DONE		(1 << 1)
#define MMC_I_MASK_DATA_TRAN_DONE	(1 << 0)
#define MMC_I_MASK_ALL			0x7f


/* MMC_I_REG */
#define MMC_I_REG_TXFIFO_WR_REQ		(1 << 6)
#define MMC_I_REG_RXFIFO_RD_REQ		(1 << 5)
#define MMC_I_REG_CLK_IS_OFF		(1 << 4)
#define MMC_I_REG_STOP_CMD		(1 << 3)
#define MMC_I_REG_END_CMD_RES		(1 << 2)
#define MMC_I_REG_PRG_DONE		(1 << 1)
#define MMC_I_REG_DATA_TRAN_DONE	(1 << 0)

/* MMC_CMD */
#define MMC_CMD_INDEX_MAX		0x6f

#define MMC_R1_IDLE_STATE		0x01
#define MMC_R1_ERASE_STATE		0x02
#define MMC_R1_ILLEGAL_CMD		0x04
#define MMC_R1_COM_CRC_ERR		0x08
#define MMC_R1_ERASE_SEQ_ERR		0x01
#define MMC_R1_ADDR_ERR			0x02
#define MMC_R1_PARAM_ERR		0x04

#define MMC_R1B_WP_ERASE_SKIP		0x0002
#define MMC_R1B_ERR			0x0004
#define MMC_R1B_CC_ERR			0x0008
#define MMC_R1B_CARD_ECC_ERR		0x0010
#define MMC_R1B_WP_VIOLATION		0x0020
#define MMC_R1B_ERASE_PARAM		0x0040
#define MMC_R1B_OOR			0x0080
#define MMC_R1B_IDLE_STATE		0x0100
#define MMC_R1B_ERASE_RESET		0x0200
#define MMC_R1B_ILLEGAL_CMD		0x0400
#define MMC_R1B_COM_CRC_ERR		0x0800
#define MMC_R1B_ERASE_SEQ_ERR		0x1000
#define MMC_R1B_ADDR_ERR		0x2000
#define MMC_R1B_PARAM_ERR		0x4000

#endif	/* __REGS_MMC_H__ */
