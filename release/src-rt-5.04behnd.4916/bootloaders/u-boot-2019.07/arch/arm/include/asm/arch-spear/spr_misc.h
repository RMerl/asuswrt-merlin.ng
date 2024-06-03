/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#ifndef _SPR_MISC_H
#define _SPR_MISC_H

struct misc_regs {
	u32 auto_cfg_reg;	/* 0x0 */
	u32 armdbg_ctr_reg;	/* 0x4 */
	u32 pll1_cntl;		/* 0x8 */
	u32 pll1_frq;		/* 0xc */
	u32 pll1_mod;		/* 0x10 */
	u32 pll2_cntl;		/* 0x14 */
	u32 pll2_frq;		/* 0x18 */
	u32 pll2_mod;		/* 0x1C */
	u32 pll_ctr_reg;	/* 0x20 */
	u32 amba_clk_cfg;	/* 0x24 */
	u32 periph_clk_cfg;	/* 0x28 */
	u32 periph1_clken;	/* 0x2C */
	u32 soc_core_id;	/* 0x30 */
	u32 ras_clken;		/* 0x34 */
	u32 periph1_rst;	/* 0x38 */
	u32 periph2_rst;	/* 0x3C */
	u32 ras_rst;		/* 0x40 */
	u32 prsc1_clk_cfg;	/* 0x44 */
	u32 prsc2_clk_cfg;	/* 0x48 */
	u32 prsc3_clk_cfg;	/* 0x4C */
	u32 amem_cfg_ctrl;	/* 0x50 */
	u32 expi_clk_cfg;	/* 0x54 */
	u32 reserved_1;		/* 0x58 */
	u32 clcd_synth_clk;	/* 0x5C */
	u32 irda_synth_clk;	/* 0x60 */
	u32 uart_synth_clk;	/* 0x64 */
	u32 gmac_synth_clk;	/* 0x68 */
	u32 ras_synth1_clk;	/* 0x6C */
	u32 ras_synth2_clk;	/* 0x70 */
	u32 ras_synth3_clk;	/* 0x74 */
	u32 ras_synth4_clk;	/* 0x78 */
	u32 arb_icm_ml1;	/* 0x7C */
	u32 arb_icm_ml2;	/* 0x80 */
	u32 arb_icm_ml3;	/* 0x84 */
	u32 arb_icm_ml4;	/* 0x88 */
	u32 arb_icm_ml5;	/* 0x8C */
	u32 arb_icm_ml6;	/* 0x90 */
	u32 arb_icm_ml7;	/* 0x94 */
	u32 arb_icm_ml8;	/* 0x98 */
	u32 arb_icm_ml9;	/* 0x9C */
	u32 dma_src_sel;	/* 0xA0 */
	u32 uphy_ctr_reg;	/* 0xA4 */
	u32 gmac_ctr_reg;	/* 0xA8 */
	u32 port_bridge_ctrl;	/* 0xAC */
	u32 reserved_2[4];	/* 0xB0--0xBC */
	u32 prc1_ilck_ctrl_reg;	/* 0xC0 */
	u32 prc2_ilck_ctrl_reg;	/* 0xC4 */
	u32 prc3_ilck_ctrl_reg;	/* 0xC8 */
	u32 prc4_ilck_ctrl_reg;	/* 0xCC */
	u32 prc1_intr_ctrl_reg;	/* 0xD0 */
	u32 prc2_intr_ctrl_reg;	/* 0xD4 */
	u32 prc3_intr_ctrl_reg;	/* 0xD8 */
	u32 prc4_intr_ctrl_reg;	/* 0xDC */
	u32 powerdown_cfg_reg;	/* 0xE0 */
	u32 ddr_1v8_compensation;	/* 0xE4  */
	u32 ddr_2v5_compensation;	/* 0xE8 */
	u32 core_3v3_compensation;	/* 0xEC */
	u32 ddr_pad;		/* 0xF0 */
	u32 bist1_ctr_reg;	/* 0xF4 */
	u32 bist2_ctr_reg;	/* 0xF8 */
	u32 bist3_ctr_reg;	/* 0xFC */
	u32 bist4_ctr_reg;	/* 0x100 */
	u32 bist5_ctr_reg;	/* 0x104 */
	u32 bist1_rslt_reg;	/* 0x108 */
	u32 bist2_rslt_reg;	/* 0x10C */
	u32 bist3_rslt_reg;	/* 0x110 */
	u32 bist4_rslt_reg;	/* 0x114 */
	u32 bist5_rslt_reg;	/* 0x118 */
	u32 syst_error_reg;	/* 0x11C */
	u32 reserved_3[0x1FB8];	/* 0x120--0x7FFC */
	u32 ras_gpp1_in;	/* 0x8000 */
	u32 ras_gpp2_in;	/* 0x8004 */
	u32 ras_gpp1_out;	/* 0x8008 */
	u32 ras_gpp2_out;	/* 0x800C */
};

/* SYNTH_CLK value*/
#define SYNTH23			0x00020003

/* PLLx_FRQ value */
#if defined(CONFIG_SPEAR3XX)
#define FREQ_332		0xA600010C
#define FREQ_266		0x8500010C
#elif defined(CONFIG_SPEAR600)
#define FREQ_332		0xA600010F
#define FREQ_266		0x8500010F
#endif

/* PLL_CTR_REG   */
#define MEM_CLK_SEL_MSK		0x70000000
#define MEM_CLK_HCLK		0x00000000
#define MEM_CLK_2HCLK		0x10000000
#define MEM_CLK_PLL2		0x30000000

#define EXPI_CLK_CFG_LOW_COMPR	0x2000
#define EXPI_CLK_CFG_CLK_EN	0x0400
#define EXPI_CLK_CFG_RST	0x0200
#define EXPI_CLK_SYNT_EN	0x0010
#define EXPI_CLK_CFG_SEL_PLL2	0x0004
#define EXPI_CLK_CFG_INT_CLK_EN	0x0001

#define PLL2_CNTL_6UA		0x1c00
#define PLL2_CNTL_SAMPLE	0x0008
#define PLL2_CNTL_ENABLE	0x0004
#define PLL2_CNTL_RESETN	0x0002
#define PLL2_CNTL_LOCK		0x0001

/* AUTO_CFG_REG value */
#define MISC_SOCCFGMSK                  0x0000003F
#define MISC_SOCCFG30                   0x0000000C
#define MISC_SOCCFG31                   0x0000000D
#define MISC_NANDDIS			0x00020000

/* PERIPH_CLK_CFG value */
#define MISC_GPT3SYNTH			0x00000400
#define MISC_GPT4SYNTH			0x00000800
#define CONFIG_SPEAR_UART48M		0
#define CONFIG_SPEAR_UARTCLKMSK		(0x1 << 4)

/* PRSC_CLK_CFG value */
/*
 * Fout = Fin / (2^(N+1) * (M + 1))
 */
#define MISC_PRSC_N_1			0x00001000
#define MISC_PRSC_M_9			0x00000009
#define MISC_PRSC_N_4			0x00004000
#define MISC_PRSC_M_399			0x0000018F
#define MISC_PRSC_N_6			0x00006000
#define MISC_PRSC_M_2593		0x00000A21
#define MISC_PRSC_M_124			0x0000007C
#define MISC_PRSC_CFG			(MISC_PRSC_N_1 | MISC_PRSC_M_9)

/* PERIPH1_CLKEN, PERIPH1_RST value */
#define MISC_USBDENB			0x01000000
#define MISC_ETHENB			0x00800000
#define MISC_SMIENB			0x00200000
#define MISC_GPIO3ENB			0x00040000
#define MISC_GPT3ENB			0x00010000
#define MISC_SSP3ENB			0x00004000
#define MISC_GPIO4ENB			0x00002000
#define MISC_GPT2ENB			0x00000800
#define MISC_FSMCENB			0x00000200
#define MISC_I2CENB			0x00000080
#define MISC_SSP2ENB			0x00000040
#define MISC_SSP1ENB			0x00000020
#define MISC_UART0ENB			0x00000008

/*   PERIPH_CLK_CFG   */
#define  XTALTIMEEN		0x00000001
#define  PLLTIMEEN		0x00000002
#define  CLCDCLK_SYNTH		0x00000000
#define  CLCDCLK_48MHZ		0x00000004
#define  CLCDCLK_EXT		0x00000008
#define  UARTCLK_MASK		(0x1 << 4)
#define  UARTCLK_48MHZ		0x00000000
#define  UARTCLK_SYNTH		0x00000010
#define  IRDACLK_48MHZ		0x00000000
#define  IRDACLK_SYNTH		0x00000020
#define  IRDACLK_EXT		0x00000040
#define  RTC_DISABLE		0x00000080
#define  GPT1CLK_48MHZ		0x00000000
#define  GPT1CLK_SYNTH		0x00000100
#define  GPT2CLK_48MHZ		0x00000000
#define  GPT2CLK_SYNTH		0x00000200
#define  GPT3CLK_48MHZ		0x00000000
#define  GPT3CLK_SYNTH		0x00000400
#define  GPT4CLK_48MHZ		0x00000000
#define  GPT4CLK_SYNTH		0x00000800
#define  GPT5CLK_48MHZ		0x00000000
#define  GPT5CLK_SYNTH		0x00001000
#define  GPT1_FREEZE		0x00002000
#define  GPT2_FREEZE		0x00004000
#define  GPT3_FREEZE		0x00008000
#define  GPT4_FREEZE		0x00010000
#define  GPT5_FREEZE		0x00020000

/*  PERIPH1_CLKEN bits  */
#define PERIPH_ARM1_WE		0x00000001
#define PERIPH_ARM1		0x00000002
#define PERIPH_ARM2		0x00000004
#define PERIPH_UART1		0x00000008
#define PERIPH_UART2		0x00000010
#define PERIPH_SSP1		0x00000020
#define PERIPH_SSP2		0x00000040
#define PERIPH_I2C		0x00000080
#define PERIPH_JPEG		0x00000100
#define PERIPH_FSMC		0x00000200
#define PERIPH_FIRDA		0x00000400
#define PERIPH_GPT4		0x00000800
#define PERIPH_GPT5		0x00001000
#define PERIPH_GPIO4		0x00002000
#define PERIPH_SSP3		0x00004000
#define PERIPH_ADC		0x00008000
#define PERIPH_GPT3		0x00010000
#define PERIPH_RTC		0x00020000
#define PERIPH_GPIO3		0x00040000
#define PERIPH_DMA		0x00080000
#define PERIPH_ROM		0x00100000
#define PERIPH_SMI		0x00200000
#define PERIPH_CLCD		0x00400000
#define PERIPH_GMAC		0x00800000
#define PERIPH_USBD		0x01000000
#define PERIPH_USBH1		0x02000000
#define PERIPH_USBH2		0x04000000
#define PERIPH_MPMC		0x08000000
#define PERIPH_RAMW		0x10000000
#define PERIPH_MPMC_EN		0x20000000
#define PERIPH_MPMC_WE		0x40000000
#define PERIPH_MPMCMSK		0x60000000

#define PERIPH_CLK_ALL		0x0FFFFFF8
#define PERIPH_RST_ALL		0x00000004

/* DDR_PAD values */
#define DDR_PAD_CNF_MSK		0x0000ffff
#define DDR_PAD_SW_CONF		0x00060000
#define DDR_PAD_SSTL_SEL	0x00000001
#define DDR_PAD_DRAM_TYPE	0x00008000

/* DDR_COMP values */
#define DDR_COMP_ACCURATE	0x00000010

/* SoC revision stuff */
#define SOC_PRI_SHFT		16
#define SOC_SEC_SHFT		8

/* Revision definitions */
#define SOC_SPEAR_NA		0

/*
 * The definitons have started from
 * 101 for SPEAr6xx
 * 201 for SPEAr3xx
 * 301 for SPEAr13xx
 */
#define SOC_SPEAR600_AA		101
#define SOC_SPEAR600_AB		102
#define SOC_SPEAR600_BA		103
#define SOC_SPEAR600_BB		104
#define SOC_SPEAR600_BC		105
#define SOC_SPEAR600_BD		106

#define SOC_SPEAR300		201
#define SOC_SPEAR310		202
#define SOC_SPEAR320		203

extern int get_socrev(void);
int fsmc_nand_switch_ecc(uint32_t eccstrength);

#endif
