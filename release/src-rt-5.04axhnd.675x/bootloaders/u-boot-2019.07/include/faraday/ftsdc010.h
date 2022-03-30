/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Faraday FTSDC010 Secure Digital Memory Card Host Controller
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef __FTSDC010_H
#define __FTSDC010_H

#ifndef __ASSEMBLY__

/* sd controller register */
struct ftsdc010_mmc {
	unsigned int	cmd;		/* 0x00 - command reg		*/
	unsigned int	argu;		/* 0x04 - argument reg		*/
	unsigned int	rsp0;		/* 0x08 - response reg0		*/
	unsigned int	rsp1;		/* 0x0c - response reg1		*/
	unsigned int	rsp2;		/* 0x10 - response reg2		*/
	unsigned int	rsp3;		/* 0x14 - response reg3		*/
	unsigned int	rsp_cmd;	/* 0x18 - responded cmd reg	*/
	unsigned int	dcr;		/* 0x1c - data control reg	*/
	unsigned int	dtr;		/* 0x20 - data timer reg	*/
	unsigned int	dlr;		/* 0x24 - data length reg	*/
	unsigned int	status;		/* 0x28 - status reg		*/
	unsigned int	clr;		/* 0x2c - clear reg		*/
	unsigned int	int_mask;	/* 0x30 - intrrupt mask reg	*/
	unsigned int	pcr;		/* 0x34 - power control reg	*/
	unsigned int	ccr;		/* 0x38 - clock contorl reg	*/
	unsigned int	bwr;		/* 0x3c - bus width reg		*/
	unsigned int	dwr;		/* 0x40 - data window reg	*/
#ifndef CONFIG_FTSDC010_SDIO
	unsigned int	feature;	/* 0x44 - feature reg		*/
	unsigned int	rev;		/* 0x48 - revision reg		*/
#else
	unsigned int	mmc_intr_time;	/* 0x44 - MMC int resp time reg	*/
	unsigned int	gpo;		/* 0x48 - gerenal purpose output */
	unsigned int	reserved[8];	/* 0x50 - 0x68 reserved		*/
	unsigned int	sdio_ctrl1;	/* 0x6c - SDIO control reg 1	*/
	unsigned int	sdio_ctrl2;	/* 0x70 - SDIO control reg 2	*/
	unsigned int	sdio_status;	/* 0x74 - SDIO status regi	*/
	unsigned int	reserved1[9];	/* 0x78 - 0x98	reserved	*/
	unsigned int	feature;	/* 0x9c - feature reg		*/
	unsigned int	rev;		/* 0xa0 - revision reg		*/
#endif /* CONFIG_FTSDC010_SDIO */
};

struct mmc_host {
	struct ftsdc010_mmc *reg;
	unsigned int version;		/* SDHCI spec. version */
	unsigned int clock;		/* Current clock (MHz) */
	unsigned int fifo_len;		/* bytes */
	unsigned int last_opcode;	/* Last OP Code */
	unsigned int card_type;		/* Card type */
};

/* functions */
int ftsdc010_mmc_init(int dev_index);

#endif	/* __ASSEMBLY__ */

/* global defines */
#define FTSDC010_CMD_RETRY			0x100000
#define FTSDC010_PIO_RETRY			100	/* pio retry times */
#define FTSDC010_DELAY_UNIT			100	/* 100 us */

/* define from Linux kernel - include/linux/mmc/card.h */
#define MMC_TYPE_SDIO				2	/* SDIO card */

/* define for mmc layer */
#define MMC_DATA_BOTH_DIR			(MMC_DATA_WRITE | MMC_DATA_READ)

/* this part is strange */
#define FTSDC010_SDIO_CTRL1_REG			0x0000006C
#define FTSDC010_SDIO_CTRL2_REG			0x0000006C
#define FTSDC010_SDIO_STATUS_REG		0x00000070

/* 0x00 - command register */
#define FTSDC010_CMD_IDX(x)			(((x) & 0x3f) << 0)
#define FTSDC010_CMD_NEED_RSP			(1 << 6)
#define FTSDC010_CMD_LONG_RSP			(1 << 7)
#define FTSDC010_CMD_APP_CMD			(1 << 8)
#define FTSDC010_CMD_CMD_EN			(1 << 9)
#define FTSDC010_CMD_SDC_RST			(1 << 10)
#define FTSDC010_CMD_MMC_INT_STOP		(1 << 11)

/* 0x18 - responded command register */
#define FTSDC010_RSP_CMD_IDX(x)			(((x) >> 0) & 0x3f)
#define FTSDC010_RSP_CMD_APP			(1 << 6)

/* 0x1c - data control register */
#define FTSDC010_DCR_BLK_SIZE(x)		(((x) & 0xf) << 0)
#define FTSDC010_DCR_DATA_WRITE			(1 << 4)
#define FTSDC010_DCR_DMA_EN			(1 << 5)
#define FTSDC010_DCR_DATA_EN			(1 << 6)
#ifdef CONFIG_FTSDC010_SDIO
#define FTSDC010_DCR_FIFOTH			(1 << 7)
#define FTSDC010_DCR_DMA_TYPE(x)		(((x) & 0x3) << 8)
#define FTSDC010_DCR_FIFO_RST			(1 << 10)
#endif /* CONFIG_FTSDC010_SDIO */

#define FTSDC010_DCR_DMA_TYPE_1			0x0	/* Single r/w	*/
#define FTSDC010_DCR_DMA_TYPE_4			0x1	/* Burst 4 r/w	*/
#define FTSDC010_DCR_DMA_TYPE_8			0x2	/* Burst 8 r/w	*/

#define FTSDC010_DCR_BLK_BYTES(x)		(ffs(x) - 1)	/* 1B - 2048B */

/* CPRM related define */
#define FTSDC010_CPRM_DATA_CHANGE_ENDIAN_EN	0x000008
#define FTSDC010_CPRM_DATA_SWAP_HL_EN		0x000010

/* 0x28 - status register */
#define FTSDC010_STATUS_RSP_CRC_FAIL		(1 << 0)
#define FTSDC010_STATUS_DATA_CRC_FAIL		(1 << 1)
#define FTSDC010_STATUS_RSP_TIMEOUT		(1 << 2)
#define FTSDC010_STATUS_DATA_TIMEOUT		(1 << 3)
#define FTSDC010_STATUS_RSP_CRC_OK		(1 << 4)
#define FTSDC010_STATUS_DATA_CRC_OK		(1 << 5)
#define FTSDC010_STATUS_CMD_SEND		(1 << 6)
#define FTSDC010_STATUS_DATA_END		(1 << 7)
#define FTSDC010_STATUS_FIFO_URUN		(1 << 8)
#define FTSDC010_STATUS_FIFO_ORUN		(1 << 9)
#define FTSDC010_STATUS_CARD_CHANGE		(1 << 10)
#define FTSDC010_STATUS_CARD_DETECT		(1 << 11)
#define FTSDC010_STATUS_WRITE_PROT		(1 << 12)
#ifdef CONFIG_FTSDC010_SDIO
#define FTSDC010_STATUS_CP_READY		(1 << 13) /* reserved ? */
#define FTSDC010_STATUS_CP_BUF_READY		(1 << 14) /* reserved ? */
#define FTSDC010_STATUS_PLAIN_TEXT_READY	(1 << 15) /* reserved ? */
#define FTSDC010_STATUS_SDIO_IRPT		(1 << 16) /* SDIO card intr */
#define FTSDC010_STATUS_DATA0_STATUS		(1 << 17)
#endif /* CONFIG_FTSDC010_SDIO */
#define FTSDC010_STATUS_RSP_ERROR	\
	(FTSDC010_STATUS_RSP_CRC_FAIL | FTSDC010_STATUS_RSP_TIMEOUT)
#define FTSDC010_STATUS_RSP_MASK	\
	(FTSDC010_STATUS_RSP_ERROR | FTSDC010_STATUS_RSP_CRC_OK)
#define FTSDC010_STATUS_DATA_ERROR	\
	(FTSDC010_STATUS_DATA_CRC_FAIL | FTSDC010_STATUS_DATA_TIMEOUT)
#define FTSDC010_STATUS_DATA_MASK	\
	(FTSDC010_STATUS_DATA_ERROR | FTSDC010_STATUS_DATA_CRC_OK \
	| FTSDC010_STATUS_DATA_END)

/* 0x2c - clear register */
#define FTSDC010_CLR_RSP_CRC_FAIL		(1 << 0)
#define FTSDC010_CLR_DATA_CRC_FAIL		(1 << 1)
#define FTSDC010_CLR_RSP_TIMEOUT		(1 << 2)
#define FTSDC010_CLR_DATA_TIMEOUT		(1 << 3)
#define FTSDC010_CLR_RSP_CRC_OK			(1 << 4)
#define FTSDC010_CLR_DATA_CRC_OK		(1 << 5)
#define FTSDC010_CLR_CMD_SEND			(1 << 6)
#define FTSDC010_CLR_DATA_END			(1 << 7)
#define FTSDC010_STATUS_FIFO_URUN		(1 << 8) /* reserved ? */
#define FTSDC010_STATUS_FIFO_ORUN		(1 << 9) /* reserved ? */
#define FTSDC010_CLR_CARD_CHANGE		(1 << 10)
#ifdef CONFIG_FTSDC010_SDIO
#define FTSDC010_CLR_SDIO_IRPT			(1 << 16)
#endif /* CONFIG_FTSDC010_SDIO */

/* 0x30 - interrupt mask register */
#define FTSDC010_INT_MASK_RSP_CRC_FAIL		(1 << 0)
#define FTSDC010_INT_MASK_DATA_CRC_FAIL		(1 << 1)
#define FTSDC010_INT_MASK_RSP_TIMEOUT		(1 << 2)
#define FTSDC010_INT_MASK_DATA_TIMEOUT		(1 << 3)
#define FTSDC010_INT_MASK_RSP_CRC_OK		(1 << 4)
#define FTSDC010_INT_MASK_DATA_CRC_OK		(1 << 5)
#define FTSDC010_INT_MASK_CMD_SEND		(1 << 6)
#define FTSDC010_INT_MASK_DATA_END		(1 << 7)
#define FTSDC010_INT_MASK_FIFO_URUN		(1 << 8)
#define FTSDC010_INT_MASK_FIFO_ORUN		(1 << 9)
#define FTSDC010_INT_MASK_CARD_CHANGE		(1 << 10)
#ifdef CONFIG_FTSDC010_SDIO
#define FTSDC010_INT_MASK_CP_READY		(1 << 13)
#define FTSDC010_INT_MASK_CP_BUF_READY		(1 << 14)
#define FTSDC010_INT_MASK_PLAIN_TEXT_READY	(1 << 15)
#define FTSDC010_INT_MASK_SDIO_IRPT		(1 << 16)
#define FTSDC010_STATUS_DATA0_STATUS		(1 << 17)
#endif /* CONFIG_FTSDC010_SDIO */

/* ? */
#define FTSDC010_CARD_INSERT			0x0
#define FTSDC010_CARD_REMOVE			FTSDC010_STATUS_REG_CARD_DETECT

/* 0x34 - power control register */
#define FTSDC010_PCR_POWER(x)			(((x) & 0xf) << 0)
#define FTSDC010_PCR_POWER_ON			(1 << 4)

/* 0x38 - clock control register */
#define FTSDC010_CCR_CLK_DIV(x)			(((x) & 0x7f) << 0)
#define FTSDC010_CCR_CLK_SD			(1 << 7) /* 0: MMC, 1: SD */
#define FTSDC010_CCR_CLK_DIS			(1 << 8)
#define FTSDC010_CCR_CLK_HISPD			(1 << 9) /* high speed */

/* card type */
#define FTSDC010_CARD_TYPE_SD			FTSDC010_CLOCK_REG_CARD_TYPE
#define FTSDC010_CARD_TYPE_MMC			0x0

/* 0x3c - bus width register */
#define FTSDC010_BWR_MODE_1BIT      (1 << 0) /* 1 bit mode enabled */
#define FTSDC010_BWR_MODE_8BIT      (1 << 1) /* 8 bit mode enabled */
#define FTSDC010_BWR_MODE_4BIT      (1 << 2) /* 4 bit mode enabled */
#define FTSDC010_BWR_MODE_MASK      (7 << 0)
#define FTSDC010_BWR_MODE_SHIFT     (0)
#define FTSDC010_BWR_CAPS_1BIT      (0 << 3) /* 1 bits mode supported */
#define FTSDC010_BWR_CAPS_4BIT      (1 << 3) /* 1,4 bits mode supported */
#define FTSDC010_BWR_CAPS_8BIT      (2 << 3) /* 1,4,8 bits mode supported */
#define FTSDC010_BWR_CAPS_MASK      (3 << 3)
#define FTSDC010_BWR_CAPS_SHIFT     (3)
#define FTSDC010_BWR_CARD_DETECT    (1 << 5)

/* 0x44 or 0x9c - feature register */
#define FTSDC010_FEATURE_FIFO_DEPTH(x)		(((x) >> 0) & 0xff)
#define FTSDC010_FEATURE_CPRM_FUNCTION		(1 << 8)

#define FTSDC010_FIFO_DEPTH_4			0x04
#define FTSDC010_FIFO_DEPTH_8			0x08
#define FTSDC010_FIFO_DEPTH_16			0x10

/* 0x48 or 0xa0 - revision register */
#define FTSDC010_REV_REVISION(x)		(((x) & 0xff) >> 0)
#define FTSDC010_REV_MINOR(x)			(((x) & 0xff00) >> 8)
#define FTSDC010_REV_MAJOR(x)			(((x) & 0xffff0000) >> 16)

#ifdef CONFIG_FTSDC010_SDIO
/* 0x44 - general purpose output */
#define FTSDC010_GPO_PORT(x)			(((x) & 0xf) << 0)

/* 0x6c - sdio control register 1 */
#define FTSDC010_SDIO_CTRL1_SDIO_BLK_SIZE(x)	(((x) & 0xfff) << 0)
#define FTSDC010_SDIO_CTRL1_SDIO_BLK_MODE	(1 << 12)
#define FTSDC010_SDIO_CTRL1_READ_WAIT_EN	(1 << 13)
#define FTSDC010_SDIO_CTRL1_SDIO_ENABLE		(1 << 14)
#define FTSDC010_SDIO_CTRL1_SDIO_BLK_NO(x)	(((x) & 0x1ff) << 15)

/* 0x70 - sdio control register 2 */
#define FTSDC010_SDIO_CTRL2_SUSP_READ_WAIT	(1 << 0)
#define FTSDC010_SDIO_CTRL2_SUSP_CMD_ABORT	(1 << 1)

/* 0x74 - sdio status register */
#define FTSDC010_SDIO_STATUS_SDIO_BLK_CNT(x)	(((x) >> 0) & 0x1ffff)
#define FTSDC010_SDIO_STATUS_FIFO_REMAIN_NO(x)	(((x) >> 17) & 0xef)

#endif /* CONFIG_FTSDC010_SDIO */

#endif /* __FTSDC010_H */
