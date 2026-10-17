/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __OMAP3_SPI_H_
#define __OMAP3_SPI_H_

/* per-register bitmasks */
#define OMAP3_MCSPI_SYSCONFIG_SMARTIDLE (2 << 3)
#define OMAP3_MCSPI_SYSCONFIG_ENAWAKEUP BIT(2)
#define OMAP3_MCSPI_SYSCONFIG_AUTOIDLE	BIT(0)
#define OMAP3_MCSPI_SYSCONFIG_SOFTRESET BIT(1)

#define OMAP3_MCSPI_SYSSTATUS_RESETDONE BIT(0)

#define OMAP3_MCSPI_MODULCTRL_SINGLE	BIT(0)
#define OMAP3_MCSPI_MODULCTRL_MS	BIT(2)
#define OMAP3_MCSPI_MODULCTRL_STEST	BIT(3)

#define OMAP3_MCSPI_CHCONF_PHA		BIT(0)
#define OMAP3_MCSPI_CHCONF_POL		BIT(1)
#define OMAP3_MCSPI_CHCONF_CLKD_MASK	GENMASK(5, 2)
#define OMAP3_MCSPI_CHCONF_EPOL		BIT(6)
#define OMAP3_MCSPI_CHCONF_WL_MASK	GENMASK(11, 7)
#define OMAP3_MCSPI_CHCONF_TRM_RX_ONLY	BIT(12)
#define OMAP3_MCSPI_CHCONF_TRM_TX_ONLY	BIT(13)
#define OMAP3_MCSPI_CHCONF_TRM_MASK	GENMASK(13, 12)
#define OMAP3_MCSPI_CHCONF_DMAW		BIT(14)
#define OMAP3_MCSPI_CHCONF_DMAR		BIT(15)
#define OMAP3_MCSPI_CHCONF_DPE0		BIT(16)
#define OMAP3_MCSPI_CHCONF_DPE1		BIT(17)
#define OMAP3_MCSPI_CHCONF_IS		BIT(18)
#define OMAP3_MCSPI_CHCONF_TURBO	BIT(19)
#define OMAP3_MCSPI_CHCONF_FORCE	BIT(20)

#define OMAP3_MCSPI_CHSTAT_RXS		BIT(0)
#define OMAP3_MCSPI_CHSTAT_TXS		BIT(1)
#define OMAP3_MCSPI_CHSTAT_EOT		BIT(2)

#define OMAP3_MCSPI_CHCTRL_EN		BIT(0)
#define OMAP3_MCSPI_CHCTRL_DIS		(0 << 0)

#define OMAP3_MCSPI_WAKEUPENABLE_WKEN	BIT(0)
#define MCSPI_PINDIR_D0_IN_D1_OUT	0
#define MCSPI_PINDIR_D0_OUT_D1_IN	1

#define OMAP3_MCSPI_MAX_FREQ		48000000
#define SPI_WAIT_TIMEOUT		10

#define OMAP4_MCSPI_REG_OFFSET	0x100

#define OMAP4_MCSPI_CHAN_NB	4

/* OMAP3 McSPI registers */
struct mcspi_channel {
	unsigned int chconf;		/* 0x2C, 0x40, 0x54, 0x68 */
	unsigned int chstat;		/* 0x30, 0x44, 0x58, 0x6C */
	unsigned int chctrl;		/* 0x34, 0x48, 0x5C, 0x70 */
	unsigned int tx;		/* 0x38, 0x4C, 0x60, 0x74 */
	unsigned int rx;		/* 0x3C, 0x50, 0x64, 0x78 */
};

struct mcspi {
	unsigned char res1[0x10];
	unsigned int sysconfig;		/* 0x10 */
	unsigned int sysstatus;		/* 0x14 */
	unsigned int irqstatus;		/* 0x18 */
	unsigned int irqenable;		/* 0x1C */
	unsigned int wakeupenable;	/* 0x20 */
	unsigned int syst;		/* 0x24 */
	unsigned int modulctrl;		/* 0x28 */
	struct mcspi_channel channel[OMAP4_MCSPI_CHAN_NB];
	/* channel0: 0x2C - 0x3C, bus 0 & 1 & 2 & 3 */
	/* channel1: 0x40 - 0x50, bus 0 & 1 */
	/* channel2: 0x54 - 0x64, bus 0 & 1 */
	/* channel3: 0x68 - 0x78, bus 0 */
};

struct omap3_spi_plat {
	struct mcspi *regs;
	unsigned int pin_dir:1;
};
#endif
