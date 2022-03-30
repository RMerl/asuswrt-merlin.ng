/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * LCD controller Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __LCDC_H__
#define __LCDC_H__

/* LCD module registers */
typedef struct lcd_ctrl {
	u32 ssar;		/* 0x00 Screen Start Address Register */
	u32 sr;			/* 0x04 LCD Size Register */
	u32 vpw;		/* 0x08 Virtual Page Width Register */
	u32 cpr;		/* 0x0C Cursor Position Register */
	u32 cwhb;		/* 0x10 Cursor Width Height and Blink Register */
	u32 ccmr;		/* 0x14 Color Cursor Mapping Register */
	u32 pcr;		/* 0x18 Panel Configuration Register */
	u32 hcr;		/* 0x1C Horizontal Configuration Register */
	u32 vcr;		/* 0x20 Vertical Configuration Register */
	u32 por;		/* 0x24 Panning Offset Register */
	u32 scr;		/* 0x28 Sharp Configuration Register */
	u32 pccr;		/* 0x2C PWM Contrast Control Register */
	u32 dcr;		/* 0x30 DMA Control Register */
	u32 rmcr;		/* 0x34 Refresh Mode Control Register */
	u32 icr;		/* 0x38 Refresh Mode Control Register */
	u32 ier;		/* 0x3C Interrupt Enable Register */
	u32 isr;		/* 0x40 Interrupt Status Register */
	u32 res[4];
	u32 gwsar;		/* 0x50 Graphic Window Start Address Register */
	u32 gwsr;		/* 0x54 Graphic Window Size Register */
	u32 gwvpw;		/* 0x58 Graphic Window Virtual Page Width Register */
	u32 gwpor;		/* 0x5C Graphic Window Panning Offset Register */
	u32 gwpr;		/* 0x60 Graphic Window Position Register */
	u32 gwcr;		/* 0x64 Graphic Window Control Register */
	u32 gwdcr;		/* 0x68 Graphic Window DMA Control Register */
} lcd_t;

typedef struct lcdbg_ctrl {
	u32 bglut[255];
} lcdbg_t;

typedef struct lcdgw_ctrl {
	u32 gwlut[255];
} lcdgw_t;

/* Bit definitions and macros for LCDC_LSSAR */
#define LCDC_SSAR_SSA(x)		(((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for LCDC_LSR */
#define LCDC_SR_XMAX(x)			(((x)&0x0000003F)<<20)
#define LCDC_SR_YMAX(x)			((x)&0x000003FF)

/* Bit definitions and macros for LCDC_LVPWR */
#define LCDC_VPWR_VPW(x)		(((x)&0x000003FF)

/* Bit definitions and macros for LCDC_LCPR */
#define LCDC_CPR_CC(x)			(((x)&0x00000003)<<30)
#define LCDC_CPR_CC_AND			(0xC0000000)
#define LCDC_CPR_CC_XOR			(0x80000000)
#define LCDC_CPR_CC_OR			(0x40000000)
#define LCDC_CPR_CC_TRANSPARENT		(0x00000000)
#define LCDC_CPR_OP			(0x10000000)
#define LCDC_CPR_CXP(x)			(((x)&0x000003FF)<<16)
#define LCDC_CPR_CYP(x)			((x)&0x000003FF)

/* Bit definitions and macros for LCDC_LCWHBR */
#define LCDC_CWHBR_BK_EN		(0x80000000)
#define LCDC_CWHBR_CW(x)		(((x)&0x0000001F)<<24)
#define LCDC_CWHBR_CH(x)		(((x)&0x0000001F)<<16)
#define LCDC_CWHBR_BD(x)		((x)&0x000000FF)

/* Bit definitions and macros for LCDC_LCCMR */
#define LCDC_CCMR_CUR_COL_R(x)		(((x)&0x0000003F)<<12)
#define LCDC_CCMR_CUR_COL_G(x)		(((x)&0x0000003F)<<6)
#define LCDC_CCMR_CUR_COL_B(x)		((x)&0x0000003F)

/* Bit definitions and macros for LCDC_LPCR */
#define LCDC_PCR_PANEL_TYPE(x)		(((x)&0x00000003)<<30)
#define LCDC_PCR_MODE_TFT		(0xC0000000)
#define LCDC_PCR_MODE_CSTN		(0x40000000)
#define LCDC_PCR_MODE_MONOCHROME	(0x00000000)
#define LCDC_PCR_TFT			(0x80000000)
#define LCDC_PCR_COLOR			(0x40000000)
#define LCDC_PCR_PBSIZ(x)		(((x)&0x00000003)<<28)
#define LCDC_PCR_PBSIZ_8		(0x30000000)
#define LCDC_PCR_PBSIZ_4		(0x20000000)
#define LCDC_PCR_PBSIZ_2		(0x10000000)
#define LCDC_PCR_PBSIZ_1		(0x00000000)
#define LCDC_PCR_BPIX(x)		(((x)&0x00000007)<<25)
#define LCDC_PCR_BPIX_18bpp		(0x0C000000)
#define LCDC_PCR_BPIX_16bpp		(0x0A000000)
#define LCDC_PCR_BPIX_12bpp		(0x08000000)
#define LCDC_PCR_BPIX_8bpp		(0x06000000)
#define LCDC_PCR_BPIX_4bpp		(0x04000000)
#define LCDC_PCR_BPIX_2bpp		(0x02000000)
#define LCDC_PCR_BPIX_1bpp		(0x00000000)
#define LCDC_PCR_PIXPOL			(0x01000000)
#define LCDC_PCR_FLM			(0x00800000)
#define LCDC_PCR_LPPOL			(0x00400000)
#define LCDC_PCR_CLKPOL			(0x00200000)
#define LCDC_PCR_OEPOL			(0x00100000)
#define LCDC_PCR_SCLKIDLE		(0x00080000)
#define LCDC_PCR_ENDSEL			(0x00040000)
#define LCDC_PCR_SWAP_SEL		(0x00020000)
#define LCDC_PCR_REV_VS			(0x00010000)
#define LCDC_PCR_ACDSEL			(0x00008000)
#define LCDC_PCR_ACD(x)			(((x)&0x0000007F)<<8)
#define LCDC_PCR_SCLKSEL		(0x00000080)
#define LCDC_PCR_SHARP			(0x00000040)
#define LCDC_PCR_PCD(x)			((x)&0x0000003F)

/* Bit definitions and macros for LCDC_LHCR */
#define LCDC_HCR_H_WIDTH(x)		(((x)&0x0000003F)<<26)
#define LCDC_HCR_H_WAIT_1(x)		(((x)&0x000000FF)<<8)
#define LCDC_HCR_H_WAIT_2(x)		((x)&0x000000FF)

/* Bit definitions and macros for LCDC_LVCR */
#define LCDC_VCR_V_WIDTH(x)		(((x)&0x0000003F)<<26)
#define LCDC_VCR_V_WAIT_1(x)		(((x)&0x000000FF)<<8)
#define LCDC_VCR_V_WAIT_2(x)		((x)&0x000000FF)

/* Bit definitions and macros for LCDC_SCR */
#define LCDC_SCR_PS_R_DELAY(x)		(((x)&0x0000003F) << 26)
#define LCDC_SCR_CLS_R_DELAY(x)		(((x)&0x000000FF) << 16)
#define LCDC_SCR_RTG_DELAY(x)		(((x)&0x0000000F) << 8)
#define LCDC_SCR_GRAY2(x)		(((x)&0x0000000F) << 4)
#define LCDC_SCR_GRAY1(x)		((x)&&0x0000000F)

/* Bit definitions and macros for LCDC_LPCCR */
#define LCDC_PCCR_CLS_HI_WID(x)		(((x)&0x000001FF)<<16)
#define LCDC_PCCR_LDMSK			(0x00008000)
#define LCDC_PCCR_SCR(x)		(((x)&0x00000003)<<9)
#define LCDC_PCCR_SCR_LCDCLK		(0x00000400)
#define LCDC_PCCR_SCR_PIXCLK		(0x00000200)
#define LCDC_PCCR_SCR_LNPULSE		(0x00000000)
#define LCDC_PCCR_CC_EN			(0x00000100)
#define LCDC_PCCR_PW(x)			((x)&0x000000FF)

/* Bit definitions and macros for LCDC_LDCR */
#define LCDC_DCR_BURST			(0x80000000)
#define LCDC_DCR_HM(x)			(((x)&0x0000001F)<<16)
#define LCDC_DCR_TM(x)			((x)&0x0000001F)

/* Bit definitions and macros for LCDC_LRMCR */
#define LCDC_RMCR_SEL_REF		(0x00000001)

/* Bit definitions and macros for LCDC_LICR */
#define LCDC_ICR_GW_INT_CON		(0x00000010)
#define LCDC_ICR_INTSYN			(0x00000004)
#define LCDC_ICR_INTCON			(0x00000001)

/* Bit definitions and macros for LCDC_LIER */
#define LCDC_IER_GW_UDR			(0x00000080)
#define LCDC_IER_GW_ERR			(0x00000040)
#define LCDC_IER_GW_EOF			(0x00000020)
#define LCDC_IER_GW_BOF			(0x00000010)
#define LCDC_IER_UDR			(0x00000008)
#define LCDC_IER_ERR			(0x00000004)
#define LCDC_IER_EOF			(0x00000002)
#define LCDC_IER_BOF			(0x00000001)

/* Bit definitions and macros for LCDC_LGWSAR */
#define LCDC_GWSAR_GWSA(x)		(((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for LCDC_LGWSR */
#define LCDC_GWSR_GWW(x)		(((x)&0x0000003F)<<20)
#define LCDC_GWSR_GWH(x)		((x)&0x000003FF)

/* Bit definitions and macros for LCDC_LGWVPWR */
#define LCDC_GWVPWR_GWVPW(x)		((x)&0x000003FF)

/* Bit definitions and macros for LCDC_LGWPOR */
#define LCDC_GWPOR_GWPO(x)		((x)&0x0000001F)

/* Bit definitions and macros for LCDC_LGWPR */
#define LCDC_GWPR_GWXP(x)		(((x)&0x000003FF)<<16)
#define LCDC_GWPR_GWYP(x)		((x)&0x000003FF)

/* Bit definitions and macros for LCDC_LGWCR */
#define LCDC_GWCR_GWAV(x)		(((x)&0x000000FF)<<24)
#define LCDC_GWCR_GWCKE			(0x00800000)
#define LCDC_LGWCR_GWE			(0x00400000)
#define LCDC_LGWCR_GW_RVS		(0x00200000)
#define LCDC_LGWCR_GWCKR(x)		(((x)&0x0000003F)<<12)
#define LCDC_LGWCR_GWCKG(x)		(((x)&0x0000003F)<<6)
#define LCDC_LGWCR_GWCKB(x)		((x)&0x0000003F)

/* Bit definitions and macros for LCDC_LGWDCR */
#define LCDC_LGWDCR_GWBT		(0x80000000)
#define LCDC_LGWDCR_GWHM(x)		(((x)&0x0000001F)<<16)
#define LCDC_LGWDCR_GWTM(x)		((x)&0x0000001F)

#endif				/* __LCDC_H__ */
