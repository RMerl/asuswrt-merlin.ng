// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

/* Tegra20 pin multiplexing functions */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/pinmux.h>

/*
 * This defines the order of the pin mux control bits in the registers. For
 * some reason there is no correspendence between the tristate, pin mux and
 * pullup/pulldown registers.
 */
enum pmux_ctlid {
	/* 0: APB_MISC_PP_PIN_MUX_CTL_A_0 */
	MUXCTL_UAA,
	MUXCTL_UAB,
	MUXCTL_UAC,
	MUXCTL_UAD,
	MUXCTL_UDA,
	MUXCTL_RESERVED5,
	MUXCTL_ATE,
	MUXCTL_RM,

	MUXCTL_ATB,
	MUXCTL_RESERVED9,
	MUXCTL_ATD,
	MUXCTL_ATC,
	MUXCTL_ATA,
	MUXCTL_KBCF,
	MUXCTL_KBCE,
	MUXCTL_SDMMC1,

	/* 16: APB_MISC_PP_PIN_MUX_CTL_B_0 */
	MUXCTL_GMA,
	MUXCTL_GMC,
	MUXCTL_HDINT,
	MUXCTL_SLXA,
	MUXCTL_OWC,
	MUXCTL_SLXC,
	MUXCTL_SLXD,
	MUXCTL_SLXK,

	MUXCTL_UCA,
	MUXCTL_UCB,
	MUXCTL_DTA,
	MUXCTL_DTB,
	MUXCTL_RESERVED28,
	MUXCTL_DTC,
	MUXCTL_DTD,
	MUXCTL_DTE,

	/* 32: APB_MISC_PP_PIN_MUX_CTL_C_0 */
	MUXCTL_DDC,
	MUXCTL_CDEV1,
	MUXCTL_CDEV2,
	MUXCTL_CSUS,
	MUXCTL_I2CP,
	MUXCTL_KBCA,
	MUXCTL_KBCB,
	MUXCTL_KBCC,

	MUXCTL_IRTX,
	MUXCTL_IRRX,
	MUXCTL_DAP1,
	MUXCTL_DAP2,
	MUXCTL_DAP3,
	MUXCTL_DAP4,
	MUXCTL_GMB,
	MUXCTL_GMD,

	/* 48: APB_MISC_PP_PIN_MUX_CTL_D_0 */
	MUXCTL_GME,
	MUXCTL_GPV,
	MUXCTL_GPU,
	MUXCTL_SPDO,
	MUXCTL_SPDI,
	MUXCTL_SDB,
	MUXCTL_SDC,
	MUXCTL_SDD,

	MUXCTL_SPIH,
	MUXCTL_SPIG,
	MUXCTL_SPIF,
	MUXCTL_SPIE,
	MUXCTL_SPID,
	MUXCTL_SPIC,
	MUXCTL_SPIB,
	MUXCTL_SPIA,

	/* 64: APB_MISC_PP_PIN_MUX_CTL_E_0 */
	MUXCTL_LPW0,
	MUXCTL_LPW1,
	MUXCTL_LPW2,
	MUXCTL_LSDI,
	MUXCTL_LSDA,
	MUXCTL_LSPI,
	MUXCTL_LCSN,
	MUXCTL_LDC,

	MUXCTL_LSCK,
	MUXCTL_LSC0,
	MUXCTL_LSC1,
	MUXCTL_LHS,
	MUXCTL_LVS,
	MUXCTL_LM0,
	MUXCTL_LM1,
	MUXCTL_LVP0,

	/* 80: APB_MISC_PP_PIN_MUX_CTL_F_0 */
	MUXCTL_LD0,
	MUXCTL_LD1,
	MUXCTL_LD2,
	MUXCTL_LD3,
	MUXCTL_LD4,
	MUXCTL_LD5,
	MUXCTL_LD6,
	MUXCTL_LD7,

	MUXCTL_LD8,
	MUXCTL_LD9,
	MUXCTL_LD10,
	MUXCTL_LD11,
	MUXCTL_LD12,
	MUXCTL_LD13,
	MUXCTL_LD14,
	MUXCTL_LD15,

	/* 96: APB_MISC_PP_PIN_MUX_CTL_G_0 */
	MUXCTL_LD16,
	MUXCTL_LD17,
	MUXCTL_LHP1,
	MUXCTL_LHP2,
	MUXCTL_LVP1,
	MUXCTL_LHP0,
	MUXCTL_RESERVED102,
	MUXCTL_LPP,

	MUXCTL_LDI,
	MUXCTL_PMC,
	MUXCTL_CRTP,
	MUXCTL_PTA,
	MUXCTL_RESERVED108,
	MUXCTL_KBCD,
	MUXCTL_GPU7,
	MUXCTL_DTF,

	MUXCTL_NONE = -1,
};

/*
 * And this defines the order of the pullup/pulldown controls which are again
 * in a different order
 */
enum pmux_pullid {
	/* 0: APB_MISC_PP_PULLUPDOWN_REG_A_0 */
	PUCTL_ATA,
	PUCTL_ATB,
	PUCTL_ATC,
	PUCTL_ATD,
	PUCTL_ATE,
	PUCTL_DAP1,
	PUCTL_DAP2,
	PUCTL_DAP3,

	PUCTL_DAP4,
	PUCTL_DTA,
	PUCTL_DTB,
	PUCTL_DTC,
	PUCTL_DTD,
	PUCTL_DTE,
	PUCTL_DTF,
	PUCTL_GPV,

	/* 16: APB_MISC_PP_PULLUPDOWN_REG_B_0 */
	PUCTL_RM,
	PUCTL_I2CP,
	PUCTL_PTA,
	PUCTL_GPU7,
	PUCTL_KBCA,
	PUCTL_KBCB,
	PUCTL_KBCC,
	PUCTL_KBCD,

	PUCTL_SPDI,
	PUCTL_SPDO,
	PUCTL_GPSLXAU,
	PUCTL_CRTP,
	PUCTL_SLXC,
	PUCTL_SLXD,
	PUCTL_SLXK,

	/* 32: APB_MISC_PP_PULLUPDOWN_REG_C_0 */
	PUCTL_CDEV1,
	PUCTL_CDEV2,
	PUCTL_SPIA,
	PUCTL_SPIB,
	PUCTL_SPIC,
	PUCTL_SPID,
	PUCTL_SPIE,
	PUCTL_SPIF,

	PUCTL_SPIG,
	PUCTL_SPIH,
	PUCTL_IRTX,
	PUCTL_IRRX,
	PUCTL_GME,
	PUCTL_RESERVED45,
	PUCTL_XM2D,
	PUCTL_XM2C,

	/* 48: APB_MISC_PP_PULLUPDOWN_REG_D_0 */
	PUCTL_UAA,
	PUCTL_UAB,
	PUCTL_UAC,
	PUCTL_UAD,
	PUCTL_UCA,
	PUCTL_UCB,
	PUCTL_LD17,
	PUCTL_LD19_18,

	PUCTL_LD21_20,
	PUCTL_LD23_22,
	PUCTL_LS,
	PUCTL_LC,
	PUCTL_CSUS,
	PUCTL_DDRC,
	PUCTL_SDC,
	PUCTL_SDD,

	/* 64: APB_MISC_PP_PULLUPDOWN_REG_E_0 */
	PUCTL_KBCF,
	PUCTL_KBCE,
	PUCTL_PMCA,
	PUCTL_PMCB,
	PUCTL_PMCC,
	PUCTL_PMCD,
	PUCTL_PMCE,
	PUCTL_CK32,

	PUCTL_UDA,
	PUCTL_SDMMC1,
	PUCTL_GMA,
	PUCTL_GMB,
	PUCTL_GMC,
	PUCTL_GMD,
	PUCTL_DDC,
	PUCTL_OWC,

	PUCTL_NONE = -1
};

/* Convenient macro for defining pin group properties */
#define PINALL(pingrp, f0, f1, f2, f3, mux, pupd)	\
	{						\
		.funcs = {				\
			PMUX_FUNC_ ## f0,		\
			PMUX_FUNC_ ## f1,		\
			PMUX_FUNC_ ## f2,		\
			PMUX_FUNC_ ## f3,		\
		},					\
		.ctl_id = mux,				\
		.pull_id = pupd				\
	}

/* A normal pin group where the mux name and pull-up name match */
#define PIN(pingrp, f0, f1, f2, f3) \
	PINALL(pingrp, f0, f1, f2, f3, MUXCTL_##pingrp, PUCTL_##pingrp)

/* A pin group where the pull-up name doesn't have a 1-1 mapping */
#define PINP(pingrp, f0, f1, f2, f3, pupd) \
	PINALL(pingrp, f0, f1, f2, f3, MUXCTL_##pingrp, PUCTL_##pupd)

/* A pin group number which is not used */
#define PIN_RESERVED \
	PIN(NONE, RSVD1, RSVD2, RSVD3, RSVD4)

#define DRVGRP(drvgrp) \
	PINALL(drvgrp, RSVD1, RSVD2, RSVD3, RSVD4, MUXCTL_NONE, PUCTL_NONE)

static const struct pmux_pingrp_desc tegra20_pingroups[] = {
	PIN(ATA,    IDE,       NAND,      GMI,       RSVD4),
	PIN(ATB,    IDE,       NAND,      GMI,       SDIO4),
	PIN(ATC,    IDE,       NAND,      GMI,       SDIO4),
	PIN(ATD,    IDE,       NAND,      GMI,       SDIO4),
	PIN(CDEV1,  OSC,       PLLA_OUT,  PLLM_OUT1, AUDIO_SYNC),
	PIN(CDEV2,  OSC,       AHB_CLK,   APB_CLK,   PLLP_OUT4),
	PIN(CSUS,   PLLC_OUT1, PLLP_OUT2, PLLP_OUT3, VI_SENSOR_CLK),
	PIN(DAP1,   DAP1,      RSVD2,     GMI,       SDIO2),

	PIN(DAP2,   DAP2,      TWC,       RSVD3,     GMI),
	PIN(DAP3,   DAP3,      RSVD2,     RSVD3,     RSVD4),
	PIN(DAP4,   DAP4,      RSVD2,     GMI,       RSVD4),
	PIN(DTA,    RSVD1,     SDIO2,     VI,        RSVD4),
	PIN(DTB,    RSVD1,     RSVD2,     VI,        SPI1),
	PIN(DTC,    RSVD1,     RSVD2,     VI,        RSVD4),
	PIN(DTD,    RSVD1,     SDIO2,     VI,        RSVD4),
	PIN(DTE,    RSVD1,     RSVD2,     VI,        SPI1),

	PINP(GPU,   PWM,       UARTA,     GMI,       RSVD4,         GPSLXAU),
	PIN(GPV,    PCIE,      RSVD2,     RSVD3,     RSVD4),
	PIN(I2CP,   I2C,       RSVD2,     RSVD3,     RSVD4),
	PIN(IRTX,   UARTA,     UARTB,     GMI,       SPI4),
	PIN(IRRX,   UARTA,     UARTB,     GMI,       SPI4),
	PIN(KBCB,   KBC,       NAND,      SDIO2,     MIO),
	PIN(KBCA,   KBC,       NAND,      SDIO2,     EMC_TEST0_DLL),
	PINP(PMC,   PWR_ON,    PWR_INTR,  RSVD3,     RSVD4,         NONE),

	PIN(PTA,    I2C2,      HDMI,      GMI,       RSVD4),
	PIN(RM,     I2C,       RSVD2,     RSVD3,     RSVD4),
	PIN(KBCE,   KBC,       NAND,      OWR,       RSVD4),
	PIN(KBCF,   KBC,       NAND,      TRACE,     MIO),
	PIN(GMA,    UARTE,     SPI3,      GMI,       SDIO4),
	PIN(GMC,    UARTD,     SPI4,      GMI,       SFLASH),
	PIN(SDMMC1, SDIO1,     RSVD2,     UARTE,     UARTA),
	PIN(OWC,    OWR,       RSVD2,     RSVD3,     RSVD4),

	PIN(GME,    RSVD1,     DAP5,      GMI,       SDIO4),
	PIN(SDC,    PWM,       TWC,       SDIO3,     SPI3),
	PIN(SDD,    UARTA,     PWM,       SDIO3,     SPI3),
	PIN_RESERVED,
	PINP(SLXA,  PCIE,      SPI4,      SDIO3,     SPI2,          CRTP),
	PIN(SLXC,   SPDIF,     SPI4,      SDIO3,     SPI2),
	PIN(SLXD,   SPDIF,     SPI4,      SDIO3,     SPI2),
	PIN(SLXK,   PCIE,      SPI4,      SDIO3,     SPI2),

	PIN(SPDI,   SPDIF,     RSVD2,     I2C,       SDIO2),
	PIN(SPDO,   SPDIF,     RSVD2,     I2C,       SDIO2),
	PIN(SPIA,   SPI1,      SPI2,      SPI3,      GMI),
	PIN(SPIB,   SPI1,      SPI2,      SPI3,      GMI),
	PIN(SPIC,   SPI1,      SPI2,      SPI3,      GMI),
	PIN(SPID,   SPI2,      SPI1,      SPI2_ALT,  GMI),
	PIN(SPIE,   SPI2,      SPI1,      SPI2_ALT,  GMI),
	PIN(SPIF,   SPI3,      SPI1,      SPI2,      RSVD4),

	PIN(SPIG,   SPI3,      SPI2,      SPI2_ALT,  I2C),
	PIN(SPIH,   SPI3,      SPI2,      SPI2_ALT,  I2C),
	PIN(UAA,    SPI3,      MIPI_HS,   UARTA,     ULPI),
	PIN(UAB,    SPI2,      MIPI_HS,   UARTA,     ULPI),
	PIN(UAC,    OWR,       RSVD2,     RSVD3,     RSVD4),
	PIN(UAD,    UARTB,     SPDIF,     UARTA,     SPI4),
	PIN(UCA,    UARTC,     RSVD2,     GMI,       RSVD4),
	PIN(UCB,    UARTC,     PWM,       GMI,       RSVD4),

	PIN_RESERVED,
	PIN(ATE,    IDE,       NAND,      GMI,       RSVD4),
	PIN(KBCC,   KBC,       NAND,      TRACE,     EMC_TEST1_DLL),
	PIN_RESERVED,
	PIN_RESERVED,
	PIN(GMB,    IDE,       NAND,      GMI,       GMI_INT),
	PIN(GMD,    RSVD1,     NAND,      GMI,       SFLASH),
	PIN(DDC,    I2C2,      RSVD2,     RSVD3,     RSVD4),

	/* 64 */
	PINP(LD0,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD1,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD2,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD3,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD4,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD5,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD6,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD7,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),

	PINP(LD8,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD9,   DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD10,  DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD11,  DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD12,  DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD13,  DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD14,  DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD15,  DISPA,     DISPB,     XIO,       RSVD4,         LD17),

	PINP(LD16,  DISPA,     DISPB,     XIO,       RSVD4,         LD17),
	PINP(LD17,  DISPA,     DISPB,     RSVD3,     RSVD4,         LD17),
	PINP(LHP0,  DISPA,     DISPB,     RSVD3,     RSVD4,         LD21_20),
	PINP(LHP1,  DISPA,     DISPB,     RSVD3,     RSVD4,         LD19_18),
	PINP(LHP2,  DISPA,     DISPB,     RSVD3,     RSVD4,         LD19_18),
	PINP(LVP0,  DISPA,     DISPB,     RSVD3,     RSVD4,         LC),
	PINP(LVP1,  DISPA,     DISPB,     RSVD3,     RSVD4,         LD21_20),
	PINP(HDINT, HDMI,      RSVD2,     RSVD3,     RSVD4,         LC),

	PINP(LM0,   DISPA,     DISPB,     SPI3,      RSVD4,         LC),
	PINP(LM1,   DISPA,     DISPB,     RSVD3,     CRT,           LC),
	PINP(LVS,   DISPA,     DISPB,     XIO,       RSVD4,         LC),
	PINP(LSC0,  DISPA,     DISPB,     XIO,       RSVD4,         LC),
	PINP(LSC1,  DISPA,     DISPB,     SPI3,      HDMI,          LS),
	PINP(LSCK,  DISPA,     DISPB,     SPI3,      HDMI,          LS),
	PINP(LDC,   DISPA,     DISPB,     RSVD3,     RSVD4,         LS),
	PINP(LCSN,  DISPA,     DISPB,     SPI3,      RSVD4,         LS),

	/* 96 */
	PINP(LSPI,  DISPA,     DISPB,     XIO,       HDMI,          LC),
	PINP(LSDA,  DISPA,     DISPB,     SPI3,      HDMI,          LS),
	PINP(LSDI,  DISPA,     DISPB,     SPI3,      RSVD4,         LS),
	PINP(LPW0,  DISPA,     DISPB,     SPI3,      HDMI,          LS),
	PINP(LPW1,  DISPA,     DISPB,     RSVD3,     RSVD4,         LS),
	PINP(LPW2,  DISPA,     DISPB,     SPI3,      HDMI,          LS),
	PINP(LDI,   DISPA,     DISPB,     RSVD3,     RSVD4,         LD23_22),
	PINP(LHS,   DISPA,     DISPB,     XIO,       RSVD4,         LC),

	PINP(LPP,   DISPA,     DISPB,     RSVD3,     RSVD4,         LD23_22),
	PIN_RESERVED,
	PIN(KBCD,   KBC,       NAND,      SDIO2,     MIO),
	PIN(GPU7,   RTCK,      RSVD2,     RSVD3,     RSVD4),
	PIN(DTF,    I2C3,      RSVD2,     VI,        RSVD4),
	PIN(UDA,    SPI1,      RSVD2,     UARTD,     ULPI),
	PIN(CRTP,   CRT,       RSVD2,     RSVD3,     RSVD4),
	PINP(SDB,   UARTA,     PWM,       SDIO3,     SPI2,          NONE),

	/* these pin groups only have pullup and pull down control */
	DRVGRP(CK32),
	DRVGRP(DDRC),
	DRVGRP(PMCA),
	DRVGRP(PMCB),
	DRVGRP(PMCC),
	DRVGRP(PMCD),
	DRVGRP(PMCE),
	DRVGRP(XM2C),
	DRVGRP(XM2D),
};
const struct pmux_pingrp_desc *tegra_soc_pingroups = tegra20_pingroups;
