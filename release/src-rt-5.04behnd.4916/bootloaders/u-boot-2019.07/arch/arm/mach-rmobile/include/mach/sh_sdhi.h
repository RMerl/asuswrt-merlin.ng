/* SPDX-License-Identifier: GPL-2.0 */
/*
 * drivers/mmc/sh-sdhi.h
 *
 * SD/MMC driver for Renesas rmobile ARM SoCs
 *
 * Copyright (C) 2013-2017 Renesas Electronics Corporation
 * Copyright (C) 2008-2009 Renesas Solutions Corp.
 */

#ifndef _SH_SDHI_H
#define _SH_SDHI_H

#define SDHI_CMD			(0x0000 >> 1)
#define SDHI_PORTSEL			(0x0004 >> 1)
#define SDHI_ARG0			(0x0008 >> 1)
#define SDHI_ARG1			(0x000C >> 1)
#define SDHI_STOP			(0x0010 >> 1)
#define SDHI_SECCNT			(0x0014 >> 1)
#define SDHI_RSP00			(0x0018 >> 1)
#define SDHI_RSP01			(0x001C >> 1)
#define SDHI_RSP02			(0x0020 >> 1)
#define SDHI_RSP03			(0x0024 >> 1)
#define SDHI_RSP04			(0x0028 >> 1)
#define SDHI_RSP05			(0x002C >> 1)
#define SDHI_RSP06			(0x0030 >> 1)
#define SDHI_RSP07			(0x0034 >> 1)
#define SDHI_INFO1			(0x0038 >> 1)
#define SDHI_INFO2			(0x003C >> 1)
#define SDHI_INFO1_MASK			(0x0040 >> 1)
#define SDHI_INFO2_MASK			(0x0044 >> 1)
#define SDHI_CLK_CTRL			(0x0048 >> 1)
#define SDHI_SIZE			(0x004C >> 1)
#define SDHI_OPTION			(0x0050 >> 1)
#define SDHI_ERR_STS1			(0x0058 >> 1)
#define SDHI_ERR_STS2			(0x005C >> 1)
#define SDHI_BUF0			(0x0060 >> 1)
#define SDHI_SDIO_MODE			(0x0068 >> 1)
#define SDHI_SDIO_INFO1			(0x006C >> 1)
#define SDHI_SDIO_INFO1_MASK		(0x0070 >> 1)
#define SDHI_CC_EXT_MODE		(0x01B0 >> 1)
#define SDHI_SOFT_RST			(0x01C0 >> 1)
#define SDHI_VERSION			(0x01C4 >> 1)
#define SDHI_HOST_MODE			(0x01C8 >> 1)
#define SDHI_SDIF_MODE			(0x01CC >> 1)
#define SDHI_EXT_SWAP			(0x01E0 >> 1)
#define SDHI_SD_DMACR			(0x0324 >> 1)

/* SDHI CMD VALUE */
#define CMD_MASK			0x0000ffff

/* SDHI_PORTSEL */
#define USE_1PORT			(1 << 8) /* 1 port */

/* SDHI_ARG */
#define ARG0_MASK			0x0000ffff
#define ARG1_MASK			0x0000ffff

/* SDHI_STOP */
#define STOP_SEC_ENABLE			(1 << 8)

/* SDHI_INFO1 */
#define INFO1_RESP_END			(1 << 0)
#define INFO1_ACCESS_END		(1 << 2)
#define INFO1_CARD_RE			(1 << 3)
#define INFO1_CARD_IN			(1 << 4)
#define INFO1_ISD0CD			(1 << 5)
#define INFO1_WRITE_PRO			(1 << 7)
#define INFO1_DATA3_CARD_RE		(1 << 8)
#define INFO1_DATA3_CARD_IN		(1 << 9)
#define INFO1_DATA3			(1 << 10)

/* SDHI_INFO2 */
#define INFO2_CMD_ERROR			(1 << 0)
#define INFO2_CRC_ERROR			(1 << 1)
#define INFO2_END_ERROR			(1 << 2)
#define INFO2_TIMEOUT			(1 << 3)
#define INFO2_BUF_ILL_WRITE		(1 << 4)
#define INFO2_BUF_ILL_READ		(1 << 5)
#define INFO2_RESP_TIMEOUT		(1 << 6)
#define INFO2_SDDAT0			(1 << 7)
#define INFO2_BRE_ENABLE		(1 << 8)
#define INFO2_BWE_ENABLE		(1 << 9)
#define INFO2_CBUSY			(1 << 14)
#define INFO2_ILA			(1 << 15)
#define INFO2_ALL_ERR			(0x807f)

/* SDHI_INFO1_MASK */
#define INFO1M_RESP_END			(1 << 0)
#define INFO1M_ACCESS_END		(1 << 2)
#define INFO1M_CARD_RE			(1 << 3)
#define INFO1M_CARD_IN			(1 << 4)
#define INFO1M_DATA3_CARD_RE		(1 << 8)
#define INFO1M_DATA3_CARD_IN		(1 << 9)
#define INFO1M_ALL			(0xffff)
#define INFO1M_SET			(INFO1M_RESP_END |	\
					INFO1M_ACCESS_END |	\
					INFO1M_DATA3_CARD_RE |	\
					INFO1M_DATA3_CARD_IN)

/* SDHI_INFO2_MASK */
#define INFO2M_CMD_ERROR		(1 << 0)
#define INFO2M_CRC_ERROR		(1 << 1)
#define INFO2M_END_ERROR		(1 << 2)
#define INFO2M_TIMEOUT			(1 << 3)
#define INFO2M_BUF_ILL_WRITE		(1 << 4)
#define INFO2M_BUF_ILL_READ		(1 << 5)
#define INFO2M_RESP_TIMEOUT		(1 << 6)
#define INFO2M_BRE_ENABLE		(1 << 8)
#define INFO2M_BWE_ENABLE		(1 << 9)
#define INFO2M_ILA			(1 << 15)
#define INFO2M_ALL			(0xffff)
#define INFO2M_ALL_ERR			(0x807f)

/* SDHI_CLK_CTRL */
#define CLK_ENABLE			(1 << 8)

/* SDHI_OPTION */
#define OPT_BUS_WIDTH_M			(5 << 13)	/* 101b (15-13bit) */
#define OPT_BUS_WIDTH_1			(4 << 13)	/* bus width = 1 bit */
#define OPT_BUS_WIDTH_4			(0 << 13)	/* bus width = 4 bit */
#define OPT_BUS_WIDTH_8			(1 << 13)	/* bus width = 8 bit */

/* SDHI_ERR_STS1 */
#define ERR_STS1_CRC_ERROR		((1 << 11) | (1 << 10) | (1 << 9) | \
					(1 << 8) | (1 << 5))
#define ERR_STS1_CMD_ERROR		((1 << 4) | (1 << 3) | (1 << 2) | \
					(1 << 1) | (1 << 0))

/* SDHI_ERR_STS2 */
#define ERR_STS2_RES_TIMEOUT		(1 << 0)
#define ERR_STS2_RES_STOP_TIMEOUT	((1 << 0) | (1 << 1))
#define ERR_STS2_SYS_ERROR		((1 << 6) | (1 << 5) | (1 << 4) | \
					(1 << 3) | (1 << 2) | (1 << 1) | \
					(1 << 0))

/* SDHI_SDIO_MODE */
#define SDIO_MODE_ON			(1 << 0)
#define SDIO_MODE_OFF			(0 << 0)

/* SDHI_SDIO_INFO1 */
#define SDIO_INFO1_IOIRQ		(1 << 0)
#define SDIO_INFO1_EXPUB52		(1 << 14)
#define SDIO_INFO1_EXWT			(1 << 15)

/* SDHI_SDIO_INFO1_MASK */
#define SDIO_INFO1M_CLEAR		((1 << 1) | (1 << 2))
#define SDIO_INFO1M_ON			((1 << 15) | (1 << 14) | (1 << 2) | \
					 (1 << 1) | (1 << 0))

/* SDHI_EXT_SWAP */
#define SET_SWAP			((1 << 6) | (1 << 7))	/* SWAP */

/* SDHI_SOFT_RST */
#define SOFT_RST_ON			(0 << 0)
#define SOFT_RST_OFF			(1 << 0)

#define	CLKDEV_SD_DATA			25000000	/* 25 MHz */
#define CLKDEV_HS_DATA			50000000	/* 50 MHz */
#define CLKDEV_MMC_DATA			20000000	/* 20MHz */
#define	CLKDEV_INIT			400000		/* 100 - 400 KHz */

/* For quirk */
#define SH_SDHI_QUIRK_16BIT_BUF		BIT(0)
#define SH_SDHI_QUIRK_64BIT_BUF		BIT(1)

int sh_sdhi_init(unsigned long addr, int ch, unsigned long quirks);

#endif /* _SH_SDHI_H */
