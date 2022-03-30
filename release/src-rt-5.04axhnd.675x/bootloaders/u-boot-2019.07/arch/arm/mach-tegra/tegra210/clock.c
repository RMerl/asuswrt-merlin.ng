// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2015
 * NVIDIA Corporation <www.nvidia.com>
 */

/* Tegra210 Clock control functions */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sysctr.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/timer.h>
#include <div64.h>
#include <fdtdec.h>

/*
 * Clock types that we can use as a source. The Tegra210 has muxes for the
 * peripheral clocks, and in most cases there are four options for the clock
 * source. This gives us a clock 'type' and exploits what commonality exists
 * in the device.
 *
 * Letters are obvious, except for T which means CLK_M, and S which means the
 * clock derived from 32KHz. Beware that CLK_M (also called OSC in the
 * datasheet) and PLL_M are different things. The former is the basic
 * clock supplied to the SOC from an external oscillator. The latter is the
 * memory clock PLL.
 *
 * See definitions in clock_id in the header file.
 */
enum clock_type_id {
	CLOCK_TYPE_AXPT,	/* PLL_A, PLL_X, PLL_P, CLK_M */
	CLOCK_TYPE_MCPA,	/* and so on */
	CLOCK_TYPE_MCPT,
	CLOCK_TYPE_PCM,
	CLOCK_TYPE_PCMT,
	CLOCK_TYPE_PDCT,
	CLOCK_TYPE_ACPT,
	CLOCK_TYPE_ASPTE,
	CLOCK_TYPE_PDD2T,
	CLOCK_TYPE_PCST,
	CLOCK_TYPE_DP,

	CLOCK_TYPE_PC2CC3M,
	CLOCK_TYPE_PC2CC3S_T,
	CLOCK_TYPE_PC2CC3M_T,
	CLOCK_TYPE_PC2CC3M_T16,	/* PC2CC3M_T, but w/16-bit divisor (I2C) */
	CLOCK_TYPE_MC2CC3P_A,
	CLOCK_TYPE_M,
	CLOCK_TYPE_MCPTM2C2C3,
	CLOCK_TYPE_PC2CC3T_S,
	CLOCK_TYPE_AC2CC3P_TS2,
	CLOCK_TYPE_PC01C00_C42C41TC40,

	CLOCK_TYPE_COUNT,
	CLOCK_TYPE_NONE = -1,   /* invalid clock type */
};

enum {
	CLOCK_MAX_MUX   = 8     /* number of source options for each clock */
};

/*
 * Clock source mux for each clock type. This just converts our enum into
 * a list of mux sources for use by the code.
 *
 * Note:
 *  The extra column in each clock source array is used to store the mask
 *  bits in its register for the source.
 */
#define CLK(x) CLOCK_ID_ ## x
static enum clock_id clock_source[CLOCK_TYPE_COUNT][CLOCK_MAX_MUX+1] = {
	{ CLK(AUDIO),	CLK(XCPU),	CLK(PERIPH),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(MEMORY),	CLK(CGENERAL),	CLK(PERIPH),	CLK(AUDIO),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(MEMORY),	CLK(CGENERAL),	CLK(PERIPH),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(MEMORY),	CLK(NONE),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(MEMORY),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),	CLK(DISPLAY),	CLK(CGENERAL),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(AUDIO),	CLK(CGENERAL),	CLK(PERIPH),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(AUDIO),	CLK(SFROM32KHZ),	CLK(PERIPH),	CLK(OSC),
		CLK(EPCI),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_29},
	{ CLK(PERIPH),	CLK(NONE),	CLK(DISPLAY),	CLK(NONE),
		CLK(NONE),	CLK(DISPLAY2),	CLK(OSC),	CLK(NONE),
		MASK_BITS_31_29},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(SFROM32KHZ),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_28},
	/* CLOCK_TYPE_DP */
	{ CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_28},

	/* Additional clock types on Tegra114+ */
	/* CLOCK_TYPE_PC2CC3M */
	{ CLK(PERIPH),	CLK(CGENERAL2),	CLK(CGENERAL),	CLK(CGENERAL3),
		CLK(MEMORY),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_29},
	/* CLOCK_TYPE_PC2CC3S_T */
	{ CLK(PERIPH),	CLK(CGENERAL2),	CLK(CGENERAL),	CLK(CGENERAL3),
		CLK(SFROM32KHZ), CLK(NONE),	CLK(OSC),	CLK(NONE),
		MASK_BITS_31_29},
	/* CLOCK_TYPE_PC2CC3M_T */
	{ CLK(PERIPH),	CLK(CGENERAL2),	CLK(CGENERAL),	CLK(CGENERAL3),
		CLK(MEMORY),	CLK(NONE),	CLK(OSC),	CLK(NONE),
		MASK_BITS_31_29},
	/* CLOCK_TYPE_PC2CC3M_T, w/16-bit divisor (I2C) */
	{ CLK(PERIPH),	CLK(CGENERAL2),	CLK(CGENERAL),	CLK(CGENERAL3),
		CLK(MEMORY),	CLK(NONE),	CLK(OSC),	CLK(NONE),
		MASK_BITS_31_29},
	/* CLOCK_TYPE_MC2CC3P_A */
	{ CLK(MEMORY),	CLK(CGENERAL2),	CLK(CGENERAL),	CLK(CGENERAL3),
		CLK(PERIPH),	CLK(NONE),	CLK(AUDIO),	CLK(NONE),
		MASK_BITS_31_29},
	/* CLOCK_TYPE_M */
	{ CLK(MEMORY),		CLK(NONE),	CLK(NONE),	CLK(NONE),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	/* CLOCK_TYPE_MCPTM2C2C3 */
	{ CLK(MEMORY),	CLK(CGENERAL),	CLK(PERIPH),	CLK(OSC),
		CLK(MEMORY2),	CLK(CGENERAL2),	CLK(CGENERAL3),	CLK(NONE),
		MASK_BITS_31_29},
	/* CLOCK_TYPE_PC2CC3T_S */
	{ CLK(PERIPH),	CLK(CGENERAL2),	CLK(CGENERAL),	CLK(CGENERAL3),
		CLK(OSC),	CLK(NONE),	CLK(SFROM32KHZ), CLK(NONE),
		MASK_BITS_31_29},
	/* CLOCK_TYPE_AC2CC3P_TS2 */
	{ CLK(AUDIO),	CLK(CGENERAL2),	CLK(CGENERAL),	CLK(CGENERAL3),
		CLK(PERIPH),	CLK(NONE),	CLK(OSC),	CLK(SRC2),
		MASK_BITS_31_29},
	/* CLOCK_TYPE_PC01C00_C42C41TC40 */
	{ CLK(PERIPH),	CLK(CGENERAL_1), CLK(CGENERAL_0), CLK(NONE),
		CLK(CGENERAL4_2), CLK(CGENERAL4_1), CLK(OSC), CLK(CGENERAL4_0),
		MASK_BITS_31_29},
};

/*
 * Clock type for each peripheral clock source. We put the name in each
 * record just so it is easy to match things up
 */
#define TYPE(name, type) type
static enum clock_type_id clock_periph_type[PERIPHC_COUNT] = {
	/* 0x00 */
	TYPE(PERIPHC_I2S2,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2S3,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_SPDIF_OUT,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_SPDIF_IN,	CLOCK_TYPE_PC2CC3M),
	TYPE(PERIPHC_PWM,	CLOCK_TYPE_PC2CC3S_T),
	TYPE(PERIPHC_05h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC2,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_SBC3,	CLOCK_TYPE_PC2CC3M_T),

	/* 0x08 */
	TYPE(PERIPHC_08h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_I2C1,	CLOCK_TYPE_PC2CC3M_T16),
	TYPE(PERIPHC_I2C5,	CLOCK_TYPE_PC2CC3M_T16),
	TYPE(PERIPHC_0bh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_0ch,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC1,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_DISP1,	CLOCK_TYPE_PDD2T),
	TYPE(PERIPHC_DISP2,	CLOCK_TYPE_PDD2T),

	/* 0x10 */
	TYPE(PERIPHC_10h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_11h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VI,	CLOCK_TYPE_MC2CC3P_A),
	TYPE(PERIPHC_13h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SDMMC1,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_SDMMC2,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_16h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_17h,	CLOCK_TYPE_NONE),

	/* 0x18 */
	TYPE(PERIPHC_18h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SDMMC4,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_VFIR,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_1Bh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_1Ch,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_HSI,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_UART1,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_UART2,	CLOCK_TYPE_PC2CC3M_T),

	/* 0x20 */
	TYPE(PERIPHC_HOST1X,	CLOCK_TYPE_MC2CC3P_A),
	TYPE(PERIPHC_21h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_22h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_23h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_24h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_25h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_I2C2,	CLOCK_TYPE_PC2CC3M_T16),
	TYPE(PERIPHC_EMC,	CLOCK_TYPE_MCPTM2C2C3),

	/* 0x28 */
	TYPE(PERIPHC_UART3,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_29h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VI_SENSOR,	CLOCK_TYPE_MC2CC3P_A),
	TYPE(PERIPHC_2bh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_2ch,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC4,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_I2C3,	CLOCK_TYPE_PC2CC3M_T16),
	TYPE(PERIPHC_SDMMC3,	CLOCK_TYPE_PC2CC3M_T),

	/* 0x30 */
	TYPE(PERIPHC_UART4,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_UART5,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_VDE,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_OWR,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_NOR,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_CSITE,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_I2S1,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_DTV,	CLOCK_TYPE_NONE),

	/* 0x38 */
	TYPE(PERIPHC_38h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_39h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_3ah,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_3bh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_MSENC,	CLOCK_TYPE_MC2CC3P_A),
	TYPE(PERIPHC_TSEC,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_3eh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_OSC,	CLOCK_TYPE_NONE),

	/* 0x40 */
	TYPE(PERIPHC_40h,	CLOCK_TYPE_NONE),	/* start with 0x3b0 */
	TYPE(PERIPHC_MSELECT,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_TSENSOR,	CLOCK_TYPE_PC2CC3T_S),
	TYPE(PERIPHC_I2S4,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2S5,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2C4,	CLOCK_TYPE_PC2CC3M_T16),
	TYPE(PERIPHC_SBC5,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_SBC6,	CLOCK_TYPE_PC2CC3M_T),

	/* 0x48 */
	TYPE(PERIPHC_AUDIO,	CLOCK_TYPE_AC2CC3P_TS2),
	TYPE(PERIPHC_49h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_4ah,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_4bh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_4ch,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_HDA2CODEC2X, CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_ACTMON,	CLOCK_TYPE_PC2CC3S_T),
	TYPE(PERIPHC_EXTPERIPH1, CLOCK_TYPE_ASPTE),

	/* 0x50 */
	TYPE(PERIPHC_EXTPERIPH2, CLOCK_TYPE_ASPTE),
	TYPE(PERIPHC_EXTPERIPH3, CLOCK_TYPE_ASPTE),
	TYPE(PERIPHC_52h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_I2CSLOW,	CLOCK_TYPE_PC2CC3S_T),
	TYPE(PERIPHC_SYS,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_55h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_56h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_57h,	CLOCK_TYPE_NONE),

	/* 0x58 */
	TYPE(PERIPHC_58h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_59h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_5ah,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_5bh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SATAOOB,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SATA,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_HDA,	CLOCK_TYPE_PC2CC3M_T),
	TYPE(PERIPHC_5fh,	CLOCK_TYPE_NONE),

	/* 0x60 */
	TYPE(PERIPHC_XUSB_CORE_HOST, CLOCK_TYPE_NONE),
	TYPE(PERIPHC_XUSB_FALCON, CLOCK_TYPE_NONE),
	TYPE(PERIPHC_XUSB_FS,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_XUSB_CORE_DEV, CLOCK_TYPE_NONE),
	TYPE(PERIPHC_XUSB_SS,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_CILAB,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_CILCD,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_CILE,	CLOCK_TYPE_NONE),

	/* 0x68 */
	TYPE(PERIPHC_DSIA_LP,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_DSIB_LP,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_ENTROPY,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_DVFS_REF,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_DVFS_SOC,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_TRACECLKIN, CLOCK_TYPE_NONE),
	TYPE(PERIPHC_6eh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_6fh,	CLOCK_TYPE_NONE),

	/* 0x70 */
	TYPE(PERIPHC_EMC_LATENCY, CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SOC_THERM,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_72h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_73h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_74h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_75h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VI_SENSOR2, CLOCK_TYPE_NONE),
	TYPE(PERIPHC_I2C6,	CLOCK_TYPE_PC2CC3M_T16),

	/* 0x78 */
	TYPE(PERIPHC_78h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_EMC_DLL,	CLOCK_TYPE_MCPTM2C2C3),
	TYPE(PERIPHC_7ah,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_CLK72MHZ,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_7ch,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_7dh,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VIC,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_7Fh,	CLOCK_TYPE_NONE),

	/* 0x80 */
	TYPE(PERIPHC_SDMMC_LEGACY_TM,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NVDEC,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NVJPG,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NVENC,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_84h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_85h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_86h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_87h,	CLOCK_TYPE_NONE),

	/* 0x88 */
	TYPE(PERIPHC_88h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_89h,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_DMIC3,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_APE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_QSPI,	CLOCK_TYPE_PC01C00_C42C41TC40),
	TYPE(PERIPHC_VI_I2C,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_USB2_HSIC_TRK, CLOCK_TYPE_NONE),
	TYPE(PERIPHC_PEX_SATA_USB_RX_BYP, CLOCK_TYPE_NONE),

	/* 0x90 */
	TYPE(PERIPHC_MAUD,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_TSECB,	CLOCK_TYPE_NONE),
};

/*
 * This array translates a periph_id to a periphc_internal_id
 *
 * Not present/matched up:
 *	uint vi_sensor;	 _VI_SENSOR_0,		0x1A8
 *	SPDIF - which is both 0x08 and 0x0c
 *
 */
#define NONE(name) (-1)
#define OFFSET(name, value) PERIPHC_ ## name
#define INTERNAL_ID(id) (id & 0x000000ff)
static s8 periph_id_to_internal_id[PERIPH_ID_COUNT] = {
	/* Low word: 31:0 */
	NONE(CPU),
	NONE(COP),
	NONE(TRIGSYS),
	NONE(ISPB),
	NONE(RESERVED4),
	NONE(TMR),
	PERIPHC_UART1,
	PERIPHC_UART2,	/* and vfir 0x68 */

	/* 8 */
	NONE(GPIO),
	PERIPHC_SDMMC2,
	PERIPHC_SPDIF_IN,
	PERIPHC_I2S2,
	PERIPHC_I2C1,
	NONE(RESERVED13),
	PERIPHC_SDMMC1,
	PERIPHC_SDMMC4,

	/* 16 */
	NONE(TCW),
	PERIPHC_PWM,
	PERIPHC_I2S3,
	NONE(RESERVED19),
	PERIPHC_VI,
	NONE(RESERVED21),
	NONE(USBD),
	NONE(ISP),

	/* 24 */
	NONE(RESERVED24),
	NONE(RESERVED25),
	PERIPHC_DISP2,
	PERIPHC_DISP1,
	PERIPHC_HOST1X,
	NONE(VCP),
	PERIPHC_I2S1,
	NONE(CACHE2),

	/* Middle word: 63:32 */
	NONE(MEM),
	NONE(AHBDMA),
	NONE(APBDMA),
	NONE(RESERVED35),
	NONE(RESERVED36),
	NONE(STAT_MON),
	NONE(RESERVED38),
	NONE(FUSE),

	/* 40 */
	NONE(KFUSE),
	PERIPHC_SBC1,		/* SBCx = SPIx */
	PERIPHC_NOR,
	NONE(RESERVED43),
	PERIPHC_SBC2,
	NONE(XIO),
	PERIPHC_SBC3,
	PERIPHC_I2C5,

	/* 48 */
	NONE(DSI),
	NONE(RESERVED49),
	PERIPHC_HSI,
	NONE(RESERVED51),
	NONE(CSI),
	NONE(RESERVED53),
	PERIPHC_I2C2,
	PERIPHC_UART3,

	/* 56 */
	NONE(MIPI_CAL),
	PERIPHC_EMC,
	NONE(USB2),
	NONE(USB3),
	NONE(RESERVED60),
	PERIPHC_VDE,
	NONE(BSEA),
	NONE(BSEV),

	/* Upper word 95:64 */
	NONE(RESERVED64),
	PERIPHC_UART4,
	PERIPHC_UART5,
	PERIPHC_I2C3,
	PERIPHC_SBC4,
	PERIPHC_SDMMC3,
	NONE(PCIE),
	PERIPHC_OWR,

	/* 72 */
	NONE(AFI),
	PERIPHC_CSITE,
	NONE(PCIEXCLK),
	NONE(AVPUCQ),
	NONE(LA),
	NONE(TRACECLKIN),
	NONE(SOC_THERM),
	NONE(DTV),

	/* 80 */
	NONE(RESERVED80),
	PERIPHC_I2CSLOW,
	NONE(DSIB),
	PERIPHC_TSEC,
	NONE(RESERVED84),
	NONE(RESERVED85),
	NONE(RESERVED86),
	NONE(EMUCIF),

	/* 88 */
	NONE(RESERVED88),
	NONE(XUSB_HOST),
	NONE(RESERVED90),
	PERIPHC_MSENC,
	NONE(RESERVED92),
	NONE(RESERVED93),
	NONE(RESERVED94),
	NONE(XUSB_DEV),

	/* V word: 31:0 */
	NONE(CPUG),
	NONE(CPULP),
	NONE(V_RESERVED2),
	PERIPHC_MSELECT,
	NONE(V_RESERVED4),
	PERIPHC_I2S4,
	PERIPHC_I2S5,
	PERIPHC_I2C4,

	/* 104 */
	PERIPHC_SBC5,
	PERIPHC_SBC6,
	PERIPHC_AUDIO,
	NONE(APBIF),
	NONE(V_RESERVED12),
	NONE(V_RESERVED13),
	NONE(V_RESERVED14),
	PERIPHC_HDA2CODEC2X,

	/* 112 */
	NONE(ATOMICS),
	NONE(V_RESERVED17),
	NONE(V_RESERVED18),
	NONE(V_RESERVED19),
	NONE(V_RESERVED20),
	NONE(V_RESERVED21),
	NONE(V_RESERVED22),
	PERIPHC_ACTMON,

	/* 120 */
	NONE(EXTPERIPH1),
	NONE(EXTPERIPH2),
	NONE(EXTPERIPH3),
	NONE(OOB),
	PERIPHC_SATA,
	PERIPHC_HDA,
	NONE(TZRAM),
	NONE(SE),

	/* W word: 31:0 */
	NONE(HDA2HDMICODEC),
	NONE(SATACOLD),
	NONE(W_RESERVED2),
	NONE(W_RESERVED3),
	NONE(W_RESERVED4),
	NONE(W_RESERVED5),
	NONE(W_RESERVED6),
	NONE(W_RESERVED7),

	/* 136 */
	NONE(CEC),
	NONE(W_RESERVED9),
	NONE(W_RESERVED10),
	NONE(W_RESERVED11),
	NONE(W_RESERVED12),
	NONE(W_RESERVED13),
	NONE(XUSB_PADCTL),
	NONE(W_RESERVED15),

	/* 144 */
	NONE(W_RESERVED16),
	NONE(W_RESERVED17),
	NONE(W_RESERVED18),
	NONE(W_RESERVED19),
	NONE(W_RESERVED20),
	NONE(ENTROPY),
	NONE(DDS),
	NONE(W_RESERVED23),

	/* 152 */
	NONE(W_RESERVED24),
	NONE(W_RESERVED25),
	NONE(W_RESERVED26),
	NONE(DVFS),
	NONE(XUSB_SS),
	NONE(W_RESERVED29),
	NONE(W_RESERVED30),
	NONE(W_RESERVED31),

	/* X word: 31:0 */
	NONE(SPARE),
	NONE(X_RESERVED1),
	NONE(X_RESERVED2),
	NONE(X_RESERVED3),
	NONE(CAM_MCLK),
	NONE(CAM_MCLK2),
	PERIPHC_I2C6,
	NONE(X_RESERVED7),

	/* 168 */
	NONE(X_RESERVED8),
	NONE(X_RESERVED9),
	NONE(X_RESERVED10),
	NONE(VIM2_CLK),
	NONE(X_RESERVED12),
	NONE(X_RESERVED13),
	NONE(EMC_DLL),
	NONE(X_RESERVED15),

	/* 176 */
	NONE(X_RESERVED16),
	NONE(CLK72MHZ),
	NONE(VIC),
	NONE(X_RESERVED19),
	NONE(X_RESERVED20),
	NONE(DPAUX),
	NONE(SOR0),
	NONE(X_RESERVED23),

	/* 184 */
	NONE(GPU),
	NONE(X_RESERVED25),
	NONE(X_RESERVED26),
	NONE(X_RESERVED27),
	NONE(X_RESERVED28),
	NONE(X_RESERVED29),
	NONE(X_RESERVED30),
	NONE(X_RESERVED31),

	/* Y: 192 (192 - 223) */
	NONE(Y_RESERVED0),
	PERIPHC_SDMMC_LEGACY_TM,
	PERIPHC_NVDEC,
	PERIPHC_NVJPG,
	NONE(Y_RESERVED4),
	PERIPHC_DMIC3,		/* 197 */
	PERIPHC_APE,		/* 198 */
	NONE(Y_RESERVED7),

	/* 200 */
	NONE(Y_RESERVED8),
	NONE(Y_RESERVED9),
	NONE(Y_RESERVED10),
	NONE(Y_RESERVED11),
	NONE(Y_RESERVED12),
	NONE(Y_RESERVED13),
	NONE(Y_RESERVED14),
	NONE(Y_RESERVED15),

	/* 208 */
	PERIPHC_VI_I2C,		/* 208 */
	NONE(Y_RESERVED17),
	NONE(Y_RESERVED18),
	PERIPHC_QSPI,		/* 211 */
	NONE(Y_RESERVED20),
	NONE(Y_RESERVED21),
	NONE(Y_RESERVED22),
	NONE(Y_RESERVED23),

	/* 216 */
	NONE(Y_RESERVED24),
	NONE(Y_RESERVED25),
	NONE(Y_RESERVED26),
	PERIPHC_NVENC,		/* 219 */
	NONE(Y_RESERVED28),
	NONE(Y_RESERVED29),
	NONE(Y_RESERVED30),
	NONE(Y_RESERVED31),
};

/*
 * PLL divider shift/mask tables for all PLL IDs.
 */
struct clk_pll_info tegra_pll_info_table[CLOCK_ID_PLL_COUNT] = {
	/*
	 * NOTE: If kcp_mask/kvco_mask == 0, they're not used in that PLL (PLLC, etc.)
	 *       If lock_ena or lock_det are >31, they're not used in that PLL (PLLC, etc.)
	 */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 10, .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F,
	  .lock_ena = 32,  .lock_det = 27, .kcp_shift = 0, .kcp_mask = 0, .kvco_shift = 0, .kvco_mask = 0 },	/* PLLC */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F,
	  .lock_ena = 4,  .lock_det = 27, .kcp_shift = 1, .kcp_mask = 3, .kvco_shift = 0, .kvco_mask = 1 },	/* PLLM */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 10, .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F,
	  .lock_ena = 18, .lock_det = 27, .kcp_shift = 0, .kcp_mask = 3, .kvco_shift = 2, .kvco_mask = 1 },	/* PLLP */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F,
	  .lock_ena = 28, .lock_det = 27, .kcp_shift = 25, .kcp_mask = 3, .kvco_shift = 24, .kvco_mask = 1 },	/* PLLA */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 16, .p_mask = 0x1F,
	  .lock_ena = 29, .lock_det = 27, .kcp_shift = 25, .kcp_mask = 3, .kvco_shift = 24, .kvco_mask = 1 },	/* PLLU */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 11, .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x07,
	  .lock_ena = 18, .lock_det = 27, .kcp_shift = 23, .kcp_mask = 3, .kvco_shift = 22, .kvco_mask = 1 },	/* PLLD */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F,
	  .lock_ena = 18, .lock_det = 27, .kcp_shift = 1, .kcp_mask = 3, .kvco_shift = 0, .kvco_mask = 1 },	/* PLLX */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 0,  .p_mask = 0,
	  .lock_ena = 9,  .lock_det = 11, .kcp_shift = 6, .kcp_mask = 3, .kvco_shift = 0, .kvco_mask = 1 },	/* PLLE */
	{ .m_shift = 0, .m_mask = 0, .n_shift = 0, .n_mask = 0, .p_shift = 0, .p_mask = 0,
	  .lock_ena = 0, .lock_det = 0, .kcp_shift = 0, .kcp_mask = 0, .kvco_shift = 0, .kvco_mask = 0 },	/* PLLS (gone)*/
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8, .n_mask = 0xFF,  .p_shift = 19,  .p_mask = 0x1F,
	  .lock_ena = 30, .lock_det = 27, .kcp_shift = 25, .kcp_mask = 3, .kvco_shift = 24, .kvco_mask = 1 },	/* PLLDP */
};

/*
 * Get the oscillator frequency, from the corresponding hardware configuration
 * field. Note that Tegra30+ support 3 new higher freqs, but we map back
 * to the old T20 freqs. Support for the higher oscillators is TBD.
 */
enum clock_osc_freq clock_get_osc_freq(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	reg = readl(&clkrst->crc_osc_ctrl);
	reg = (reg & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;
	/*
	 * 0 = 13MHz, 1 = 16.8MHz, 4 = 19.2MHz, 5 = 38.4MHz,
	 * 8 = 12MHz, 9 = 48MHz,  12 = 26MHz
	 */
	if (reg == 5) {
		debug("OSC_FREQ is 38.4MHz (%d) ...\n", reg);
		/* Map it to the 5th CLOCK_OSC_ enum, i.e. 4 */
		return 4;
	}

	/*
	 * Map to most common (T20) freqs (except 38.4, handled above):
	 *  13/16.8 = 0, 19.2 = 1, 12/48 = 2, 26 = 3
	 */
	return reg >> 2;
}

/* Returns a pointer to the clock source register for a peripheral */
u32 *get_periph_source_reg(enum periph_id periph_id)
{
	struct clk_rst_ctlr *clkrst =
		(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	enum periphc_internal_id internal_id;

	/* Coresight is a special case */
	if (periph_id == PERIPH_ID_CSI)
		return &clkrst->crc_clk_src[PERIPH_ID_CSI+1];

	assert(periph_id >= PERIPH_ID_FIRST && periph_id < PERIPH_ID_COUNT);
	internal_id = INTERNAL_ID(periph_id_to_internal_id[periph_id]);
	assert(internal_id != -1);

	if (internal_id < PERIPHC_VW_FIRST)
		/* L, H, U */
		return &clkrst->crc_clk_src[internal_id];

	if (internal_id < PERIPHC_X_FIRST) {
		/* VW */
		internal_id -= PERIPHC_VW_FIRST;
		return &clkrst->crc_clk_src_vw[internal_id];
	}

	if (internal_id < PERIPHC_Y_FIRST) {
		/* X */
		internal_id -= PERIPHC_X_FIRST;
		return &clkrst->crc_clk_src_x[internal_id];
	}

	/* Y */
	internal_id -= PERIPHC_Y_FIRST;
	return &clkrst->crc_clk_src_y[internal_id];
}

int get_periph_clock_info(enum periph_id periph_id, int *mux_bits,
			  int *divider_bits, int *type)
{
	enum periphc_internal_id internal_id;

	if (!clock_periph_id_isvalid(periph_id))
		return -1;

	internal_id = periph_id_to_internal_id[periph_id];
	if (!periphc_internal_id_isvalid(internal_id))
		return -1;

	*type = clock_periph_type[internal_id];
	if (!clock_type_id_isvalid(*type))
		return -1;

	*mux_bits = clock_source[*type][CLOCK_MAX_MUX];

	if (*type == CLOCK_TYPE_PC2CC3M_T16)
		*divider_bits = 16;
	else
		*divider_bits = 8;

	return 0;
}

enum clock_id get_periph_clock_id(enum periph_id periph_id, int source)
{
	enum periphc_internal_id internal_id;
	int type;

	if (!clock_periph_id_isvalid(periph_id))
		return CLOCK_ID_NONE;

	internal_id = periph_id_to_internal_id[periph_id];
	if (!periphc_internal_id_isvalid(internal_id))
		return CLOCK_ID_NONE;

	type = clock_periph_type[internal_id];
	if (!clock_type_id_isvalid(type))
		return CLOCK_ID_NONE;

	return clock_source[type][source];
}

/**
 * Given a peripheral ID and the required source clock, this returns which
 * value should be programmed into the source mux for that peripheral.
 *
 * There is special code here to handle the one source type with 5 sources.
 *
 * @param periph_id	peripheral to start
 * @param source	PLL id of required parent clock
 * @param mux_bits	Set to number of bits in mux register: 2 or 4
 * @param divider_bits Set to number of divider bits (8 or 16)
 * @return mux value (0-4, or -1 if not found)
 */
int get_periph_clock_source(enum periph_id periph_id,
	enum clock_id parent, int *mux_bits, int *divider_bits)
{
	enum clock_type_id type;
	int mux, err;

	err = get_periph_clock_info(periph_id, mux_bits, divider_bits, &type);
	assert(!err);

	for (mux = 0; mux < CLOCK_MAX_MUX; mux++)
		if (clock_source[type][mux] == parent)
			return mux;

	/* if we get here, either us or the caller has made a mistake */
	printf("Caller requested bad clock: periph=%d, parent=%d\n", periph_id,
	       parent);
	return -1;
}

void clock_set_enable(enum periph_id periph_id, int enable)
{
	struct clk_rst_ctlr *clkrst =
		(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 *clk;
	u32 reg;

	/* Enable/disable the clock to this peripheral */
	assert(clock_periph_id_isvalid(periph_id));
	if ((int)periph_id < (int)PERIPH_ID_VW_FIRST)
		clk = &clkrst->crc_clk_out_enb[PERIPH_REG(periph_id)];
	else if ((int)periph_id < (int)PERIPH_ID_X_FIRST)
		clk = &clkrst->crc_clk_out_enb_vw[PERIPH_REG(periph_id)];
	else if ((int)periph_id < (int)PERIPH_ID_Y_FIRST)
		clk = &clkrst->crc_clk_out_enb_x;
	else
		clk = &clkrst->crc_clk_out_enb_y;

	reg = readl(clk);
	if (enable)
		reg |= PERIPH_MASK(periph_id);
	else
		reg &= ~PERIPH_MASK(periph_id);
	writel(reg, clk);
}

void reset_set_enable(enum periph_id periph_id, int enable)
{
	struct clk_rst_ctlr *clkrst =
		(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 *reset;
	u32 reg;

	/* Enable/disable reset to the peripheral */
	assert(clock_periph_id_isvalid(periph_id));
	if (periph_id < PERIPH_ID_VW_FIRST)
		reset = &clkrst->crc_rst_dev[PERIPH_REG(periph_id)];
	else if ((int)periph_id < (int)PERIPH_ID_X_FIRST)
		reset = &clkrst->crc_rst_dev_vw[PERIPH_REG(periph_id)];
	else if ((int)periph_id < (int)PERIPH_ID_Y_FIRST)
		reset = &clkrst->crc_rst_devices_x;
	else
		reset = &clkrst->crc_rst_devices_y;

	reg = readl(reset);
	if (enable)
		reg |= PERIPH_MASK(periph_id);
	else
		reg &= ~PERIPH_MASK(periph_id);
	writel(reg, reset);
}

#ifdef CONFIG_OF_CONTROL
/*
 * Convert a device tree clock ID to our peripheral ID. They are mostly
 * the same but we are very cautious so we check that a valid clock ID is
 * provided.
 *
 * @param clk_id    Clock ID according to tegra210 device tree binding
 * @return peripheral ID, or PERIPH_ID_NONE if the clock ID is invalid
 */
enum periph_id clk_id_to_periph_id(int clk_id)
{
	if (clk_id > PERIPH_ID_COUNT)
		return PERIPH_ID_NONE;

	switch (clk_id) {
	case PERIPH_ID_RESERVED4:
	case PERIPH_ID_RESERVED25:
	case PERIPH_ID_RESERVED35:
	case PERIPH_ID_RESERVED36:
	case PERIPH_ID_RESERVED38:
	case PERIPH_ID_RESERVED43:
	case PERIPH_ID_RESERVED49:
	case PERIPH_ID_RESERVED53:
	case PERIPH_ID_RESERVED64:
	case PERIPH_ID_RESERVED84:
	case PERIPH_ID_RESERVED85:
	case PERIPH_ID_RESERVED86:
	case PERIPH_ID_RESERVED88:
	case PERIPH_ID_RESERVED90:
	case PERIPH_ID_RESERVED92:
	case PERIPH_ID_RESERVED93:
	case PERIPH_ID_RESERVED94:
	case PERIPH_ID_V_RESERVED2:
	case PERIPH_ID_V_RESERVED4:
	case PERIPH_ID_V_RESERVED17:
	case PERIPH_ID_V_RESERVED18:
	case PERIPH_ID_V_RESERVED19:
	case PERIPH_ID_V_RESERVED20:
	case PERIPH_ID_V_RESERVED21:
	case PERIPH_ID_V_RESERVED22:
	case PERIPH_ID_W_RESERVED2:
	case PERIPH_ID_W_RESERVED3:
	case PERIPH_ID_W_RESERVED4:
	case PERIPH_ID_W_RESERVED5:
	case PERIPH_ID_W_RESERVED6:
	case PERIPH_ID_W_RESERVED7:
	case PERIPH_ID_W_RESERVED9:
	case PERIPH_ID_W_RESERVED10:
	case PERIPH_ID_W_RESERVED11:
	case PERIPH_ID_W_RESERVED12:
	case PERIPH_ID_W_RESERVED13:
	case PERIPH_ID_W_RESERVED15:
	case PERIPH_ID_W_RESERVED16:
	case PERIPH_ID_W_RESERVED17:
	case PERIPH_ID_W_RESERVED18:
	case PERIPH_ID_W_RESERVED19:
	case PERIPH_ID_W_RESERVED20:
	case PERIPH_ID_W_RESERVED23:
	case PERIPH_ID_W_RESERVED29:
	case PERIPH_ID_W_RESERVED30:
	case PERIPH_ID_W_RESERVED31:
		return PERIPH_ID_NONE;
	default:
		return clk_id;
	}
}
#endif /* CONFIG_OF_CONTROL */

/*
 * T210 redefines PLLP_OUT2 as PLLP_VCO/DIVP, so do different OUT1-4 setup here.
 * PLLP_BASE/MISC/etc. is already set up for 408MHz in the BootROM.
 */
void tegra210_setup_pllp(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	/* Set PLLP_OUT1, 3 & 4 freqs to 9.6, 102 & 204MHz */

	/* OUT1 */
	/* Assert RSTN before enable */
	reg = PLLP_OUT1_RSTN_EN;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_PERIPH].pll_out[0]);
	/* Set divisor and reenable */
	reg = (IN_408_OUT_9_6_DIVISOR << PLLP_OUT1_RATIO)
		| PLLP_OUT1_OVR | PLLP_OUT1_CLKEN | PLLP_OUT1_RSTN_DIS;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_PERIPH].pll_out[0]);

	/* OUT3, 4 */
	/* Assert RSTN before enable */
	reg = PLLP_OUT4_RSTN_EN | PLLP_OUT3_RSTN_EN;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_PERIPH].pll_out[1]);
	/* Set divisor and reenable */
	reg = (IN_408_OUT_204_DIVISOR << PLLP_OUT4_RATIO)
		| PLLP_OUT4_OVR | PLLP_OUT4_CLKEN | PLLP_OUT4_RSTN_DIS
		| (IN_408_OUT_102_DIVISOR << PLLP_OUT3_RATIO)
		| PLLP_OUT3_OVR | PLLP_OUT3_CLKEN | PLLP_OUT3_RSTN_DIS;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_PERIPH].pll_out[1]);

	/*
	 * NOTE: If you want to change PLLP_OUT2 away from 204MHz,
	 * you can change PLLP_BASE DIVP here. Currently defaults
	 * to 1, which is 2^1, or 2, so PLLP_OUT2 is 204MHz.
	 * See Table 13 in section 5.1.4 in T210 TRM for more info.
	 */
}

void clock_early_init(void)
{
	struct clk_rst_ctlr *clkrst =
		(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	struct clk_pll_info *pllinfo = &tegra_pll_info_table[CLOCK_ID_DISPLAY];
	u32 data;

	tegra210_setup_pllp();

	/*
	 * PLLC output frequency set to 600Mhz
	 * PLLD output frequency set to 925Mhz
	 */
	switch (clock_get_osc_freq()) {
	case CLOCK_OSC_FREQ_12_0: /* OSC is 12Mhz */
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 12, 0, 8);
		clock_set_rate(CLOCK_ID_DISPLAY, 925, 12, 0, 12);
		break;

	case CLOCK_OSC_FREQ_26_0: /* OSC is 26Mhz */
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 26, 0, 8);
		clock_set_rate(CLOCK_ID_DISPLAY, 925, 26, 0, 12);
		break;

	case CLOCK_OSC_FREQ_13_0: /* OSC is 13Mhz */
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 13, 0, 8);
		clock_set_rate(CLOCK_ID_DISPLAY, 925, 13, 0, 12);
		break;
	case CLOCK_OSC_FREQ_19_2:
		clock_set_rate(CLOCK_ID_CGENERAL, 125, 4, 0, 0);
		clock_set_rate(CLOCK_ID_DISPLAY, 96, 2, 0, 12);
		break;
	case CLOCK_OSC_FREQ_38_4:
		clock_set_rate(CLOCK_ID_CGENERAL, 125, 8, 0, 0);
		clock_set_rate(CLOCK_ID_DISPLAY, 96, 4, 0, 0);
		break;
	default:
		/*
		 * These are not supported. It is too early to print a
		 * message and the UART likely won't work anyway due to the
		 * oscillator being wrong.
		 */
		break;
	}

	/* PLLC_MISC1: Turn IDDQ off. NOTE: T210 PLLC_MISC_1 maps to pll_misc */
	clrbits_le32(&clkrst->crc_pll[CLOCK_ID_CGENERAL].pll_misc,
		     (1 << PLLC_IDDQ));
	udelay(2);

	/*
	 * PLLC_MISC: Take PLLC out of reset. NOTE: T210 PLLC_MISC maps
	 * to pll_out[1]
	 */
	clrbits_le32(&clkrst->crc_pll[CLOCK_ID_CGENERAL].pll_out[1],
		     (1 << PLLC_RESET));
	udelay(2);

	/* PLLD_MISC: Set CLKENABLE and LOCK_DETECT bits */
	data = (1 << PLLD_ENABLE_CLK) | (1 << pllinfo->lock_ena);
	writel(data, &clkrst->crc_pll[CLOCK_ID_DISPLAY].pll_misc);
	udelay(2);
}

unsigned int clk_m_get_rate(unsigned parent_rate)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 value, div;

	value = readl(&clkrst->crc_spare_reg0);
	div = ((value >> 2) & 0x3) + 1;

	return parent_rate / div;
}

void arch_timer_init(void)
{
	struct sysctr_ctlr *sysctr = (struct sysctr_ctlr *)NV_PA_TSC_BASE;
	u32 freq, val;

	freq = clock_get_rate(CLOCK_ID_CLK_M);
	debug("%s: clk_m freq is %dHz [0x%08X]\n", __func__, freq, freq);

	if (current_el() == 3)
		asm("msr cntfrq_el0, %0\n" : : "r" (freq));

	/* Only Tegra114+ has the System Counter regs */
	debug("%s: setting CNTFID0 to 0x%08X\n", __func__, freq);
	writel(freq, &sysctr->cntfid0);

	val = readl(&sysctr->cntcr);
	val |= TSC_CNTCR_ENABLE | TSC_CNTCR_HDBG;
	writel(val, &sysctr->cntcr);
	debug("%s: TSC CNTCR = 0x%08X\n", __func__, val);
}

#define PLLREFE_MISC			0x4c8
#define  PLLREFE_MISC_LOCK		BIT(27)
#define  PLLREFE_MISC_IDDQ		BIT(24)

#define PLLREFE_BASE			0x4c4
#define  PLLREFE_BASE_BYPASS		BIT(31)
#define  PLLREFE_BASE_ENABLE		BIT(30)
#define  PLLREFE_BASE_REF_DIS		BIT(29)
#define  PLLREFE_BASE_KCP(kcp)		(((kcp) & 0x3) << 27)
#define  PLLREFE_BASE_KVCO		BIT(26)
#define  PLLREFE_BASE_DIVP(p)		(((p) & 0x1f) << 16)
#define  PLLREFE_BASE_DIVN(n)		(((n) & 0xff) << 8)
#define  PLLREFE_BASE_DIVM(m)		(((m) & 0xff) << 0)

static int tegra_pllref_enable(void)
{
	u32 value;
	unsigned long start;

	/*
	 * This sequence comes from Tegra X1 TRM section "Cold Boot, with no
	 * Recovery Mode or Boot from USB", sub-section "PLLREFE".
	 */

	value = readl(NV_PA_CLK_RST_BASE + PLLREFE_MISC);
	value &= ~PLLREFE_MISC_IDDQ;
	writel(value, NV_PA_CLK_RST_BASE + PLLREFE_MISC);

	udelay(5);

	value = PLLREFE_BASE_ENABLE |
		PLLREFE_BASE_KCP(0) |
		PLLREFE_BASE_DIVP(0) |
		PLLREFE_BASE_DIVN(0x41) |
		PLLREFE_BASE_DIVM(4);
	writel(value, NV_PA_CLK_RST_BASE + PLLREFE_BASE);

	debug("waiting for pllrefe lock\n");
	start = get_timer(0);
	while (get_timer(start) < 250) {
		value = readl(NV_PA_CLK_RST_BASE + PLLREFE_MISC);
		if (value & PLLREFE_MISC_LOCK)
			break;
	}
	if (!(value & PLLREFE_MISC_LOCK)) {
		debug("  timeout\n");
		return -ETIMEDOUT;
	}
	debug("  done\n");

	return 0;
}

#define PLLE_SS_CNTL 0x68
#define  PLLE_SS_CNTL_SSCINCINTR(x) (((x) & 0x3f) << 24)
#define  PLLE_SS_CNTL_SSCINC(x) (((x) & 0xff) << 16)
#define  PLLE_SS_CNTL_SSCINVERT (1 << 15)
#define  PLLE_SS_CNTL_SSCCENTER (1 << 14)
#define  PLLE_SS_CNTL_SSCBYP (1 << 12)
#define  PLLE_SS_CNTL_INTERP_RESET (1 << 11)
#define  PLLE_SS_CNTL_BYPASS_SS (1 << 10)
#define  PLLE_SS_CNTL_SSCMAX(x) (((x) & 0x1ff) << 0)

#define PLLE_BASE 0x0e8
#define  PLLE_BASE_ENABLE (1 << 31)
#define  PLLE_BASE_PLDIV_CML(x) (((x) & 0x1f) << 24)
#define  PLLE_BASE_NDIV(x) (((x) & 0xff) << 8)
#define  PLLE_BASE_MDIV(x) (((x) & 0xff) << 0)

#define PLLE_MISC 0x0ec
#define  PLLE_MISC_IDDQ_SWCTL (1 << 14)
#define  PLLE_MISC_IDDQ_OVERRIDE_VALUE (1 << 13)
#define  PLLE_MISC_LOCK (1 << 11)
#define  PLLE_PTS (1 << 8)
#define  PLLE_MISC_KCP(x) (((x) & 0x3) << 6)
#define  PLLE_MISC_VREG_CTRL(x) (((x) & 0x3) << 2)
#define  PLLE_MISC_KVCO (1 << 0)

#define PLLE_AUX 0x48c
#define  PLLE_AUX_SS_SEQ_INCLUDE (1 << 31)
#define  PLLE_AUX_REF_SEL_PLLREFE (1 << 28)
#define  PLLE_AUX_SEQ_ENABLE (1 << 24)
#define  PLLE_AUX_SS_SWCTL (1 << 6)
#define  PLLE_AUX_ENABLE_SWCTL (1 << 4)
#define  PLLE_AUX_USE_LOCKDET (1 << 3)

int tegra_plle_enable(void)
{
	u32 value;
	unsigned long start;

	/* PLLREF feeds PLLE */
	tegra_pllref_enable();

	/*
	 * This sequence comes from Tegra X1 TRM section "Cold Boot, with no
	 * Recovery Mode or Boot from USB", sub-section "PLLEs".
	 */

	/* 1. Select XTAL as the source */

	value = readl(NV_PA_CLK_RST_BASE + PLLE_AUX);
	value &= ~PLLE_AUX_REF_SEL_PLLREFE;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_AUX);

	value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
	value &= ~PLLE_MISC_IDDQ_OVERRIDE_VALUE;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_MISC);

	/* 2. Wait 5 us */
	udelay(5);

	/*
	 * 3. Program the following registers to generate a low jitter 100MHz
	 * clock.
	 */

	value = readl(NV_PA_CLK_RST_BASE + PLLE_BASE);
	value &= ~PLLE_BASE_PLDIV_CML(0x1f);
	value &= ~PLLE_BASE_NDIV(0xff);
	value &= ~PLLE_BASE_MDIV(0xff);
	value |= PLLE_BASE_PLDIV_CML(0xe);
	value |= PLLE_BASE_NDIV(0x7d);
	value |= PLLE_BASE_MDIV(2);
	writel(value, NV_PA_CLK_RST_BASE + PLLE_BASE);

	value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
	value |= PLLE_PTS;
	value &= ~PLLE_MISC_KCP(3);
	value &= ~PLLE_MISC_VREG_CTRL(3);
	value &= ~PLLE_MISC_KVCO;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_MISC);

	value = readl(NV_PA_CLK_RST_BASE + PLLE_BASE);
	value |= PLLE_BASE_ENABLE;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_BASE);

	/* 4. Wait for LOCK */

	debug("waiting for plle lock\n");
	start = get_timer(0);
	while (get_timer(start) < 250) {
		value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
		if (value & PLLE_MISC_LOCK)
			break;
	}
	if (!(value & PLLE_MISC_LOCK)) {
		debug("  timeout\n");
		return -ETIMEDOUT;
	}
	debug("  done\n");

	/* 5. Enable SSA */

	value = readl(NV_PA_CLK_RST_BASE + PLLE_SS_CNTL);
	value &= ~PLLE_SS_CNTL_SSCINC(0xff);
	value |= PLLE_SS_CNTL_SSCINC(1);
	value &= ~PLLE_SS_CNTL_SSCINCINTR(0x3f);
	value |= PLLE_SS_CNTL_SSCINCINTR(0x23);
	value &= ~PLLE_SS_CNTL_SSCMAX(0x1fff);
	value |= PLLE_SS_CNTL_SSCMAX(0x21);
	value &= ~PLLE_SS_CNTL_SSCINVERT;
	value &= ~PLLE_SS_CNTL_SSCCENTER;
	value &= ~PLLE_SS_CNTL_BYPASS_SS;
	value &= ~PLLE_SS_CNTL_SSCBYP;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_SS_CNTL);

	/* 6. Wait 300 ns */

	udelay(1);
	value &= ~PLLE_SS_CNTL_INTERP_RESET;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_SS_CNTL);

	/* 7. Enable HW power sequencer for PLLE */

	value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
	value &= ~PLLE_MISC_IDDQ_SWCTL;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_MISC);

	value = readl(NV_PA_CLK_RST_BASE + PLLE_AUX);
	value &= ~PLLE_AUX_SS_SWCTL;
	value &= ~PLLE_AUX_ENABLE_SWCTL;
	value |= PLLE_AUX_SS_SEQ_INCLUDE;
	value |= PLLE_AUX_USE_LOCKDET;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_AUX);

	/* 8. Wait 1 us */

	udelay(1);
	value |= PLLE_AUX_SEQ_ENABLE;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_AUX);

	return 0;
}

struct periph_clk_init periph_clk_init_table[] = {
	{ PERIPH_ID_SBC1, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC2, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC3, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC4, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC5, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC6, CLOCK_ID_PERIPH },
	{ PERIPH_ID_HOST1X, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SDMMC1, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SDMMC2, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SDMMC3, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SDMMC4, CLOCK_ID_PERIPH },
	{ PERIPH_ID_PWM, CLOCK_ID_SFROM32KHZ },
	{ PERIPH_ID_I2C1, CLOCK_ID_PERIPH },
	{ PERIPH_ID_I2C2, CLOCK_ID_PERIPH },
	{ PERIPH_ID_I2C3, CLOCK_ID_PERIPH },
	{ PERIPH_ID_I2C4, CLOCK_ID_PERIPH },
	{ PERIPH_ID_I2C5, CLOCK_ID_PERIPH },
	{ PERIPH_ID_I2C6, CLOCK_ID_PERIPH },
	{ -1, },
};
