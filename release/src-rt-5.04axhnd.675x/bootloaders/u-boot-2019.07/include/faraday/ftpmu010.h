/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 */

/*
 * Power Management Unit
 */
#ifndef __FTPMU010_H
#define __FTPMU010_H

#ifndef __ASSEMBLY__
struct ftpmu010 {
	unsigned int	IDNMBR0;	/* 0x00 */
	unsigned int	reserved0;	/* 0x04 */
	unsigned int	OSCC;		/* 0x08 */
	unsigned int	PMODE;		/* 0x0C */
	unsigned int	PMCR;		/* 0x10 */
	unsigned int	PED;		/* 0x14 */
	unsigned int	PEDSR;		/* 0x18 */
	unsigned int	reserved1;	/* 0x1C */
	unsigned int	PMSR;		/* 0x20 */
	unsigned int	PGSR;		/* 0x24 */
	unsigned int	MFPSR;		/* 0x28 */
	unsigned int	MISC;		/* 0x2C */
	unsigned int	PDLLCR0;	/* 0x30 */
	unsigned int	PDLLCR1;	/* 0x34 */
	unsigned int	AHBMCLKOFF;	/* 0x38 */
	unsigned int	APBMCLKOFF;	/* 0x3C */
	unsigned int	DCSRCR0;	/* 0x40 */
	unsigned int	DCSRCR1;	/* 0x44 */
	unsigned int	DCSRCR2;	/* 0x48 */
	unsigned int	SDRAMHTC;	/* 0x4C */
	unsigned int	PSPR0;		/* 0x50 */
	unsigned int	PSPR1;		/* 0x54 */
	unsigned int	PSPR2;		/* 0x58 */
	unsigned int	PSPR3;		/* 0x5C */
	unsigned int	PSPR4;		/* 0x60 */
	unsigned int	PSPR5;		/* 0x64 */
	unsigned int	PSPR6;		/* 0x68 */
	unsigned int	PSPR7;		/* 0x6C */
	unsigned int	PSPR8;		/* 0x70 */
	unsigned int	PSPR9;		/* 0x74 */
	unsigned int	PSPR10;		/* 0x78 */
	unsigned int	PSPR11;		/* 0x7C */
	unsigned int	PSPR12;		/* 0x80 */
	unsigned int	PSPR13;		/* 0x84 */
	unsigned int	PSPR14;		/* 0x88 */
	unsigned int	PSPR15;		/* 0x8C */
	unsigned int	AHBDMA_RACCS;	/* 0x90 */
	unsigned int	reserved2;	/* 0x94 */
	unsigned int	reserved3;	/* 0x98 */
	unsigned int	JSS;		/* 0x9C */
	unsigned int	CFC_RACC;	/* 0xA0 */
	unsigned int	SSP1_RACC;	/* 0xA4 */
	unsigned int	UART1TX_RACC;	/* 0xA8 */
	unsigned int	UART1RX_RACC;	/* 0xAC */
	unsigned int	UART2TX_RACC;	/* 0xB0 */
	unsigned int	UART2RX_RACC;	/* 0xB4 */
	unsigned int	SDC_RACC;	/* 0xB8 */
	unsigned int	I2SAC97_RACC;	/* 0xBC */
	unsigned int	IRDATX_RACC;	/* 0xC0 */
	unsigned int	reserved4;	/* 0xC4 */
	unsigned int	USBD_RACC;	/* 0xC8 */
	unsigned int	IRDARX_RACC;	/* 0xCC */
	unsigned int	IRDA_RACC;	/* 0xD0 */
	unsigned int	ED0_RACC;	/* 0xD4 */
	unsigned int	ED1_RACC;	/* 0xD8 */
};
#endif /* __ASSEMBLY__ */

/*
 * ID Number 0 Register
 */
#define FTPMU010_ID_A320A	0x03200000
#define FTPMU010_ID_A320C	0x03200010
#define FTPMU010_ID_A320D	0x03200030

/*
 * OSC Control Register
 */
#define FTPMU010_OSCC_OSCH_TRI		(1 << 11)
#define FTPMU010_OSCC_OSCH_STABLE	(1 << 9)
#define FTPMU010_OSCC_OSCH_OFF		(1 << 8)

#define FTPMU010_OSCC_OSCL_TRI		(1 << 3)
#define FTPMU010_OSCC_OSCL_RTCLSEL	(1 << 2)
#define FTPMU010_OSCC_OSCL_STABLE	(1 << 1)
#define FTPMU010_OSCC_OSCL_OFF		(1 << 0)

/*
 * Power Mode Register
 */
#define FTPMU010_PMODE_DIVAHBCLK_MASK	(0x7 << 4)
#define FTPMU010_PMODE_DIVAHBCLK_2	(0x0 << 4)
#define FTPMU010_PMODE_DIVAHBCLK_3	(0x1 << 4)
#define FTPMU010_PMODE_DIVAHBCLK_4	(0x2 << 4)
#define FTPMU010_PMODE_DIVAHBCLK_6	(0x3 << 4)
#define FTPMU010_PMODE_DIVAHBCLK_8	(0x4 << 4)
#define FTPMU010_PMODE_DIVAHBCLK(pmode)	(((pmode) >> 4) & 0x7)
#define FTPMU010_PMODE_FCS		(1 << 2)
#define FTPMU010_PMODE_TURBO		(1 << 1)
#define FTPMU010_PMODE_SLEEP		(1 << 0)

/*
 * Power Manager Status Register
 */
#define FTPMU010_PMSR_SMR	(1 << 10)

#define FTPMU010_PMSR_RDH	(1 << 2)
#define FTPMU010_PMSR_PH	(1 << 1)
#define FTPMU010_PMSR_CKEHLOW	(1 << 0)

/*
 * Multi-Function Port Setting Register
 */
#define FTPMU010_MFPSR_DEBUGSEL		(1 << 17)
#define FTPMU010_MFPSR_DMA0PINSEL	(1 << 16)
#define FTPMU010_MFPSR_DMA1PINSEL	(1 << 15)
#define FTPMU010_MFPSR_MODEMPINSEL	(1 << 14)
#define FTPMU010_MFPSR_AC97CLKOUTSEL	(1 << 13)
#define FTPMU010_MFPSR_PWM1PINSEL	(1 << 11)
#define FTPMU010_MFPSR_PWM0PINSEL	(1 << 10)
#define FTPMU010_MFPSR_IRDACLKSEL	(1 << 9)
#define FTPMU010_MFPSR_UARTCLKSEL	(1 << 8)
#define FTPMU010_MFPSR_SSPCLKSEL	(1 << 6)
#define FTPMU010_MFPSR_I2SCLKSEL	(1 << 5)
#define FTPMU010_MFPSR_AC97CLKSEL	(1 << 4)
#define FTPMU010_MFPSR_AC97PINSEL	(1 << 3)
#define FTPMU010_MFPSR_TRIAHBDIS	(1 << 1)
#define FTPMU010_MFPSR_TRIAHBDBG	(1 << 0)

/*
 * PLL/DLL Control Register 0
 * Note:
 *  1. FTPMU010_PDLLCR0_HCLKOUTDIS:
 *	Datasheet indicated it starts at bit #21 which was wrong.
 *  2. FTPMU010_PDLLCR0_DLLFRAG:
 * 	Datasheet indicated it has 2 bit which was wrong.
 */
#define FTPMU010_PDLLCR0_HCLKOUTDIS(cr0)	(((cr0) & 0xf) << 20)
#define FTPMU010_PDLLCR0_DLLFRAG(cr0)		(1 << 19)
#define FTPMU010_PDLLCR0_DLLSTSEL		(1 << 18)
#define FTPMU010_PDLLCR0_DLLSTABLE		(1 << 17)
#define FTPMU010_PDLLCR0_DLLDIS			(1 << 16)
#define FTPMU010_PDLLCR0_PLL1FRANG(cr0)		(((cr0) & 0x3) << 12)
#define FTPMU010_PDLLCR0_PLL1NS(cr0)		(((cr0) & 0x1ff) << 3)
#define FTPMU010_PDLLCR0_PLL1STSEL		(1 << 2)
#define FTPMU010_PDLLCR0_PLL1STABLE		(1 << 1)
#define FTPMU010_PDLLCR0_PLL1DIS		(1 << 0)

/*
 * SDRAM Signal Hold Time Control Register
 */
#define FTPMU010_SDRAMHTC_RCLK_DLY(x)		(((x) & 0xf) << 28)
#define FTPMU010_SDRAMHTC_CTL_WCLK_DLY(x)	(((x) & 0xf) << 24)
#define FTPMU010_SDRAMHTC_DAT_WCLK_DLY(x)	(((x) & 0xf) << 20)
#define FTPMU010_SDRAMHTC_EBICTRL_DCSR		(1 << 18)
#define FTPMU010_SDRAMHTC_EBIDATA_DCSR		(1 << 17)
#define FTPMU010_SDRAMHTC_SDRAMCS_DCSR		(1 << 16)
#define FTPMU010_SDRAMHTC_SDRAMCTL_DCSR		(1 << 15)
#define FTPMU010_SDRAMHTC_CKE_DCSR		(1 << 14)
#define FTPMU010_SDRAMHTC_DQM_DCSR		(1 << 13)
#define FTPMU010_SDRAMHTC_SDCLK_DCSR		(1 << 12)

#ifndef __ASSEMBLY__
void ftpmu010_32768osc_enable(void);
void ftpmu010_dlldis_disable(void);
void ftpmu010_mfpsr_diselect_dev(unsigned int dev);
void ftpmu010_mfpsr_select_dev(unsigned int dev);
void ftpmu010_sdram_clk_disable(unsigned int cr0);
void ftpmu010_sdramhtc_set(unsigned int val);
#endif

#ifdef __ASSEMBLY__
#define FTPMU010_IDNMBR0	0x00
#define FTPMU010_reserved0	0x04
#define FTPMU010_OSCC		0x08
#define FTPMU010_PMODE		0x0C
#define FTPMU010_PMCR		0x10
#define FTPMU010_PED		0x14
#define FTPMU010_PEDSR		0x18
#define FTPMU010_reserved1	0x1C
#define FTPMU010_PMSR		0x20
#define FTPMU010_PGSR		0x24
#define FTPMU010_MFPSR		0x28
#define FTPMU010_MISC		0x2C
#define FTPMU010_PDLLCR0	0x30
#define FTPMU010_PDLLCR1	0x34
#define FTPMU010_AHBMCLKOFF	0x38
#define FTPMU010_APBMCLKOFF	0x3C
#define FTPMU010_DCSRCR0	0x40
#define FTPMU010_DCSRCR1	0x44
#define FTPMU010_DCSRCR2	0x48
#define FTPMU010_SDRAMHTC	0x4C
#define FTPMU010_PSPR0		0x50
#define FTPMU010_PSPR1		0x54
#define FTPMU010_PSPR2		0x58
#define FTPMU010_PSPR3		0x5C
#define FTPMU010_PSPR4		0x60
#define FTPMU010_PSPR5		0x64
#define FTPMU010_PSPR6		0x68
#define FTPMU010_PSPR7		0x6C
#define FTPMU010_PSPR8		0x70
#define FTPMU010_PSPR9		0x74
#define FTPMU010_PSPR10		0x78
#define FTPMU010_PSPR11		0x7C
#define FTPMU010_PSPR12		0x80
#define FTPMU010_PSPR13		0x84
#define FTPMU010_PSPR14		0x88
#define FTPMU010_PSPR15		0x8C
#define FTPMU010_AHBDMA_RACCS	0x90
#define FTPMU010_reserved2	0x94
#define FTPMU010_reserved3	0x98
#define FTPMU010_JSS		0x9C
#define FTPMU010_CFC_RACC	0xA0
#define FTPMU010_SSP1_RACC	0xA4
#define FTPMU010_UART1TX_RACC	0xA8
#define FTPMU010_UART1RX_RACC	0xAC
#define FTPMU010_UART2TX_RACC	0xB0
#define FTPMU010_UART2RX_RACC	0xB4
#define FTPMU010_SDC_RACC	0xB8
#define FTPMU010_I2SAC97_RACC	0xBC
#define FTPMU010_IRDATX_RACC	0xC0
#define FTPMU010_reserved4	0xC4
#define FTPMU010_USBD_RACC	0xC8
#define FTPMU010_IRDARX_RACC	0xCC
#define FTPMU010_IRDA_RACC	0xD0
#define FTPMU010_ED0_RACC	0xD4
#define FTPMU010_ED1_RACC	0xD8
#endif /* __ASSEMBLY__ */

#endif	/* __FTPMU010_H */
