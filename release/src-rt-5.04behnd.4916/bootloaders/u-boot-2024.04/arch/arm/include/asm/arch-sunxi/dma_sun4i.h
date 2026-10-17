/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Roy Spliet <rspliet@ultimaker.com>
 */

#ifndef _SUNXI_DMA_SUN4I_H
#define _SUNXI_DMA_SUN4I_H

struct sunxi_dma_cfg
{
	u32 ctl;		/* 0x00 Control */
	u32 src_addr;		/* 0x04 Source address */
	u32 dst_addr;		/* 0x08 Destination address */
	u32 bc;			/* 0x0C Byte counter */
	u32 res0[2];
	u32 ddma_para;		/* 0x18 extra parameter (dedicated DMA only) */
	u32 res1;
};

struct sunxi_dma
{
	u32 irq_en;			/* 0x000 IRQ enable */
	u32 irq_pend;			/* 0x004 IRQ pending */
	u32 auto_gate;			/* 0x008 auto gating */
	u32 res0[61];
	struct sunxi_dma_cfg ndma[8];	/* 0x100 Normal DMA */
	u32 res1[64];
	struct sunxi_dma_cfg ddma[8];	/* 0x300 Dedicated DMA */
};

enum ddma_drq_type {
	DDMA_DST_DRQ_SRAM = 0,
	DDMA_SRC_DRQ_SRAM = 0,
	DDMA_DST_DRQ_SDRAM = 1,
	DDMA_SRC_DRQ_SDRAM = 1,
	DDMA_DST_DRQ_PATA = 2,
	DDMA_SRC_DRQ_PATA = 2,
	DDMA_DST_DRQ_NAND = 3,
	DDMA_SRC_DRQ_NAND = 3,
	DDMA_DST_DRQ_USB0 = 4,
	DDMA_SRC_DRQ_USB0 = 4,
	DDMA_DST_DRQ_ETHERNET_MAC_TX = 6,
	DDMA_SRC_DRQ_ETHERNET_MAC_RX = 7,
	DDMA_DST_DRQ_SPI1_TX = 8,
	DDMA_SRC_DRQ_SPI1_RX = 9,
	DDMA_DST_DRQ_SECURITY_SYS_TX = 10,
	DDMA_SRC_DRQ_SECURITY_SYS_RX = 11,
	DDMA_DST_DRQ_TCON0 = 14,
	DDMA_DST_DRQ_TCON1 = 15,
	DDMA_DST_DRQ_MSC = 23,
	DDMA_SRC_DRQ_MSC = 23,
	DDMA_DST_DRQ_SPI0_TX = 26,
	DDMA_SRC_DRQ_SPI0_RX = 27,
	DDMA_DST_DRQ_SPI2_TX = 28,
	DDMA_SRC_DRQ_SPI2_RX = 29,
	DDMA_DST_DRQ_SPI3_TX = 30,
	DDMA_SRC_DRQ_SPI3_RX = 31,
};

#define SUNXI_DMA_CTL_SRC_DRQ(a)		((a) & 0x1f)
#define SUNXI_DMA_CTL_MODE_IO			(1 << 5)
#define SUNXI_DMA_CTL_SRC_DATA_WIDTH_32		(2 << 9)
#define SUNXI_DMA_CTL_DST_DRQ(a)		(((a) & 0x1f) << 16)
#define SUNXI_DMA_CTL_DST_DATA_WIDTH_32		(2 << 25)
#define SUNXI_DMA_CTL_TRIGGER			(1 << 31)

#endif /* _SUNXI_DMA_SUN4I_H */
