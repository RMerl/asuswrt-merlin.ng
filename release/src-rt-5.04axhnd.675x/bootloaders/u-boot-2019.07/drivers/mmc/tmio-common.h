/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef __TMIO_COMMON_H__
#define __TMIO_COMMON_H__

#define TMIO_SD_CMD			0x000	/* command */
#define   TMIO_SD_CMD_NOSTOP		BIT(14)	/* No automatic CMD12 issue */
#define   TMIO_SD_CMD_MULTI		BIT(13)	/* multiple block transfer */
#define   TMIO_SD_CMD_RD		BIT(12)	/* 1: read, 0: write */
#define   TMIO_SD_CMD_DATA		BIT(11)	/* data transfer */
#define   TMIO_SD_CMD_APP		BIT(6)	/* ACMD preceded by CMD55 */
#define   TMIO_SD_CMD_NORMAL		(0 << 8)/* auto-detect of resp-type */
#define   TMIO_SD_CMD_RSP_NONE		(3 << 8)/* response: none */
#define   TMIO_SD_CMD_RSP_R1		(4 << 8)/* response: R1, R5, R6, R7 */
#define   TMIO_SD_CMD_RSP_R1B		(5 << 8)/* response: R1b, R5b */
#define   TMIO_SD_CMD_RSP_R2		(6 << 8)/* response: R2 */
#define   TMIO_SD_CMD_RSP_R3		(7 << 8)/* response: R3, R4 */
#define TMIO_SD_ARG			0x008	/* command argument */
#define TMIO_SD_STOP			0x010	/* stop action control */
#define   TMIO_SD_STOP_SEC		BIT(8)	/* use sector count */
#define   TMIO_SD_STOP_STP		BIT(0)	/* issue CMD12 */
#define TMIO_SD_SECCNT			0x014	/* sector counter */
#define TMIO_SD_RSP10			0x018	/* response[39:8] */
#define TMIO_SD_RSP32			0x020	/* response[71:40] */
#define TMIO_SD_RSP54			0x028	/* response[103:72] */
#define TMIO_SD_RSP76			0x030	/* response[127:104] */
#define TMIO_SD_INFO1			0x038	/* IRQ status 1 */
#define   TMIO_SD_INFO1_CD		BIT(5)	/* state of card detect */
#define   TMIO_SD_INFO1_INSERT		BIT(4)	/* card inserted */
#define   TMIO_SD_INFO1_REMOVE		BIT(3)	/* card removed */
#define   TMIO_SD_INFO1_CMP		BIT(2)	/* data complete */
#define   TMIO_SD_INFO1_RSP		BIT(0)	/* response complete */
#define TMIO_SD_INFO2			0x03c	/* IRQ status 2 */
#define   TMIO_SD_INFO2_ERR_ILA	BIT(15)	/* illegal access err */
#define   TMIO_SD_INFO2_CBSY		BIT(14)	/* command busy */
#define   TMIO_SD_INFO2_SCLKDIVEN	BIT(13)	/* command setting reg ena */
#define   TMIO_SD_INFO2_BWE		BIT(9)	/* write buffer ready */
#define   TMIO_SD_INFO2_BRE		BIT(8)	/* read buffer ready */
#define   TMIO_SD_INFO2_DAT0		BIT(7)	/* SDDAT0 */
#define   TMIO_SD_INFO2_ERR_RTO	BIT(6)	/* response time out */
#define   TMIO_SD_INFO2_ERR_ILR	BIT(5)	/* illegal read err */
#define   TMIO_SD_INFO2_ERR_ILW	BIT(4)	/* illegal write err */
#define   TMIO_SD_INFO2_ERR_TO		BIT(3)	/* time out error */
#define   TMIO_SD_INFO2_ERR_END	BIT(2)	/* END bit error */
#define   TMIO_SD_INFO2_ERR_CRC	BIT(1)	/* CRC error */
#define   TMIO_SD_INFO2_ERR_IDX	BIT(0)	/* cmd index error */
#define TMIO_SD_INFO1_MASK		0x040
#define TMIO_SD_INFO2_MASK		0x044
#define TMIO_SD_CLKCTL			0x048	/* clock divisor */
#define   TMIO_SD_CLKCTL_DIV_MASK	0x104ff
#define   TMIO_SD_CLKCTL_DIV1024	BIT(16)	/* SDCLK = CLK / 1024 */
#define   TMIO_SD_CLKCTL_DIV512	BIT(7)	/* SDCLK = CLK / 512 */
#define   TMIO_SD_CLKCTL_DIV256	BIT(6)	/* SDCLK = CLK / 256 */
#define   TMIO_SD_CLKCTL_DIV128	BIT(5)	/* SDCLK = CLK / 128 */
#define   TMIO_SD_CLKCTL_DIV64		BIT(4)	/* SDCLK = CLK / 64 */
#define   TMIO_SD_CLKCTL_DIV32		BIT(3)	/* SDCLK = CLK / 32 */
#define   TMIO_SD_CLKCTL_DIV16		BIT(2)	/* SDCLK = CLK / 16 */
#define   TMIO_SD_CLKCTL_DIV8		BIT(1)	/* SDCLK = CLK / 8 */
#define   TMIO_SD_CLKCTL_DIV4		BIT(0)	/* SDCLK = CLK / 4 */
#define   TMIO_SD_CLKCTL_DIV2		0	/* SDCLK = CLK / 2 */
#define   TMIO_SD_CLKCTL_DIV1		BIT(10)	/* SDCLK = CLK */
#define   TMIO_SD_CLKCTL_RCAR_DIV1	0xff	/* SDCLK = CLK (RCar ver.) */
#define   TMIO_SD_CLKCTL_OFFEN		BIT(9)	/* stop SDCLK when unused */
#define   TMIO_SD_CLKCTL_SCLKEN	BIT(8)	/* SDCLK output enable */
#define TMIO_SD_SIZE			0x04c	/* block size */
#define TMIO_SD_OPTION			0x050
#define   TMIO_SD_OPTION_WIDTH_MASK	(5 << 13)
#define   TMIO_SD_OPTION_WIDTH_1	(4 << 13)
#define   TMIO_SD_OPTION_WIDTH_4	(0 << 13)
#define   TMIO_SD_OPTION_WIDTH_8	(1 << 13)
#define TMIO_SD_BUF			0x060	/* read/write buffer */
#define TMIO_SD_EXTMODE		0x1b0
#define   TMIO_SD_EXTMODE_DMA_EN	BIT(1)	/* transfer 1: DMA, 0: pio */
#define TMIO_SD_SOFT_RST		0x1c0
#define TMIO_SD_SOFT_RST_RSTX		BIT(0)	/* reset deassert */
#define TMIO_SD_VERSION		0x1c4	/* version register */
#define TMIO_SD_VERSION_IP		0xff	/* IP version */
#define TMIO_SD_HOST_MODE		0x1c8
#define TMIO_SD_IF_MODE		0x1cc
#define   TMIO_SD_IF_MODE_DDR		BIT(0)	/* DDR mode */
#define TMIO_SD_VOLT			0x1e4	/* voltage switch */
#define   TMIO_SD_VOLT_MASK		(3 << 0)
#define   TMIO_SD_VOLT_OFF		(0 << 0)
#define   TMIO_SD_VOLT_330		(1 << 0)/* 3.3V signal */
#define   TMIO_SD_VOLT_180		(2 << 0)/* 1.8V signal */
#define TMIO_SD_DMA_MODE		0x410
#define   TMIO_SD_DMA_MODE_DIR_RD	BIT(16)	/* 1: from device, 0: to dev */
#define   TMIO_SD_DMA_MODE_ADDR_INC	BIT(0)	/* 1: address inc, 0: fixed */
#define TMIO_SD_DMA_CTL		0x414
#define   TMIO_SD_DMA_CTL_START	BIT(0)	/* start DMA (auto cleared) */
#define TMIO_SD_DMA_RST		0x418
#define   TMIO_SD_DMA_RST_RD		BIT(9)
#define   TMIO_SD_DMA_RST_WR		BIT(8)
#define TMIO_SD_DMA_INFO1		0x420
#define   TMIO_SD_DMA_INFO1_END_RD2	BIT(20)	/* DMA from device is complete (uniphier) */
#define   TMIO_SD_DMA_INFO1_END_RD	BIT(17)	/* DMA from device is complete (renesas) */
#define   TMIO_SD_DMA_INFO1_END_WR	BIT(16)	/* DMA to device is complete */
#define TMIO_SD_DMA_INFO1_MASK		0x424
#define TMIO_SD_DMA_INFO2		0x428
#define   TMIO_SD_DMA_INFO2_ERR_RD	BIT(17)
#define   TMIO_SD_DMA_INFO2_ERR_WR	BIT(16)
#define TMIO_SD_DMA_INFO2_MASK		0x42c
#define TMIO_SD_DMA_ADDR_L		0x440
#define TMIO_SD_DMA_ADDR_H		0x444

/* alignment required by the DMA engine of this controller */
#define TMIO_SD_DMA_MINALIGN		0x10

struct tmio_sd_plat {
	struct mmc_config		cfg;
	struct mmc			mmc;
};

struct tmio_sd_priv {
	void __iomem			*regbase;
	unsigned int			version;
	u32				caps;
	u32				read_poll_flag;
#define TMIO_SD_CAP_NONREMOVABLE	BIT(0)	/* Nonremovable e.g. eMMC */
#define TMIO_SD_CAP_DMA_INTERNAL	BIT(1)	/* have internal DMA engine */
#define TMIO_SD_CAP_DIV1024		BIT(2)	/* divisor 1024 is available */
#define TMIO_SD_CAP_64BIT		BIT(3)	/* Controller is 64bit */
#define TMIO_SD_CAP_16BIT		BIT(4)	/* Controller is 16bit */
#define TMIO_SD_CAP_RCAR_GEN2		BIT(5)	/* Renesas RCar version of IP */
#define TMIO_SD_CAP_RCAR_GEN3		BIT(6)	/* Renesas RCar version of IP */
#define TMIO_SD_CAP_RCAR_UHS		BIT(7)	/* Renesas RCar UHS/SDR modes */
#define TMIO_SD_CAP_RCAR		\
	(TMIO_SD_CAP_RCAR_GEN2 | TMIO_SD_CAP_RCAR_GEN3)
#ifdef CONFIG_DM_REGULATOR
	struct udevice *vqmmc_dev;
#endif
#if CONFIG_IS_ENABLED(CLK)
	struct clk			clk;
#endif
#if CONFIG_IS_ENABLED(RENESAS_SDHI)
	u8				tap_set;
	u8				nrtaps;
	bool				needs_adjust_hs400;
	bool				adjust_hs400_enable;
	u8				adjust_hs400_offset;
	u8				adjust_hs400_calibrate;
#endif
	ulong (*clk_get_rate)(struct tmio_sd_priv *);
};

int tmio_sd_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
		      struct mmc_data *data);
int tmio_sd_set_ios(struct udevice *dev);
int tmio_sd_get_cd(struct udevice *dev);

int tmio_sd_bind(struct udevice *dev);
int tmio_sd_probe(struct udevice *dev, u32 quirks);

u32 tmio_sd_readl(struct tmio_sd_priv *priv, unsigned int reg);
void tmio_sd_writel(struct tmio_sd_priv *priv,
		     u32 val, unsigned int reg);

#endif /* __TMIO_COMMON_H__ */
