/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Padmavathi Venna <padma.v@samsung.com>
 */

#ifndef __ASM_ARCH_EXYNOS_COMMON_SPI_H_
#define __ASM_ARCH_EXYNOS_COMMON_SPI_H_

#ifndef __ASSEMBLY__

/* SPI peripheral register map; padded to 64KB */
struct exynos_spi {
	unsigned int		ch_cfg;		/* 0x00 */
	unsigned char		reserved0[4];
	unsigned int		mode_cfg;	/* 0x08 */
	unsigned int		cs_reg;		/* 0x0c */
	unsigned char		reserved1[4];
	unsigned int		spi_sts;	/* 0x14 */
	unsigned int		tx_data;	/* 0x18 */
	unsigned int		rx_data;	/* 0x1c */
	unsigned int		pkt_cnt;	/* 0x20 */
	unsigned char		reserved2[4];
	unsigned int		swap_cfg;	/* 0x28 */
	unsigned int		fb_clk;		/* 0x2c */
	unsigned char		padding[0xffd0];
};

#define EXYNOS_SPI_MAX_FREQ	50000000

#define SPI_TIMEOUT_MS		10
#define SF_READ_DATA_CMD	0x3

/* SPI_CHCFG */
#define SPI_CH_HS_EN		(1 << 6)
#define SPI_CH_RST		(1 << 5)
#define SPI_SLAVE_MODE		(1 << 4)
#define SPI_CH_CPOL_L		(1 << 3)
#define SPI_CH_CPHA_B		(1 << 2)
#define SPI_RX_CH_ON		(1 << 1)
#define SPI_TX_CH_ON		(1 << 0)

/* SPI_MODECFG */
#define SPI_MODE_CH_WIDTH_WORD	(0x2 << 29)
#define SPI_MODE_BUS_WIDTH_WORD	(0x2 << 17)

/* SPI_CSREG */
#define SPI_SLAVE_SIG_INACT	(1 << 0)

/* SPI_STS */
#define SPI_ST_TX_DONE		(1 << 25)
#define SPI_FIFO_LVL_MASK	0x1ff
#define SPI_TX_LVL_OFFSET	6
#define SPI_RX_LVL_OFFSET	15

/* Feedback Delay */
#define SPI_CLK_BYPASS		(0 << 0)
#define SPI_FB_DELAY_90		(1 << 0)
#define SPI_FB_DELAY_180	(2 << 0)
#define SPI_FB_DELAY_270	(3 << 0)

/* Packet Count */
#define SPI_PACKET_CNT_EN	(1 << 16)

/* Swap config */
#define SPI_TX_SWAP_EN		(1 << 0)
#define SPI_TX_BYTE_SWAP	(1 << 2)
#define SPI_TX_HWORD_SWAP	(1 << 3)
#define SPI_TX_BYTE_SWAP	(1 << 2)
#define SPI_RX_SWAP_EN		(1 << 4)
#define SPI_RX_BYTE_SWAP	(1 << 6)
#define SPI_RX_HWORD_SWAP	(1 << 7)

#endif /* __ASSEMBLY__ */
#endif
