/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 Andes Technology Corp
 * Macpaul Lin <macpaul@andestech.com>
 */

/*
 * Andes Power Control Unit
 */
#ifndef __ANDES_PCU_H
#define __ANDES_PCU_H

#ifndef __ASSEMBLY__

struct pcs {
	unsigned int	cr;		/* PCSx Configuration (clock scaling) */
	unsigned int	parm;		/* PCSx Parameter*/
	unsigned int	stat1;		/* PCSx Status 1 */
	unsigned int	stat2;		/* PCSx Stusts 2 */
	unsigned int	pdd;		/* PCSx PDD */
};

struct andes_pcu {
	unsigned int	rev;		/* 0x00 - PCU Revision */
	unsigned int	spinfo;		/* 0x04 - Scratch Pad Info */
	unsigned int	rsvd1[2];	/* 0x08-0x0C: Reserved */
	unsigned int	soc_id;		/* 0x10 - SoC ID */
	unsigned int	soc_ahb;	/* 0x14 - SoC AHB configuration */
	unsigned int	soc_apb;	/* 0x18 - SoC APB configuration */
	unsigned int	rsvd2;		/* 0x1C */
	unsigned int	dcsrcr0;	/* 0x20 - Driving Capability
						and Slew Rate Control 0 */
	unsigned int	dcsrcr1;	/* 0x24 - Driving Capability
						and Slew Rate Control 1 */
	unsigned int	dcsrcr2;	/* 0x28 - Driving Capability
						and Slew Rate Control 2 */
	unsigned int	rsvd3;		/* 0x2C */
	unsigned int	mfpsr0;		/* 0x30 - Multi-Func Port Setting 0 */
	unsigned int	mfpsr1;		/* 0x34 - Multi-Func Port Setting 1 */
	unsigned int	dmaes;		/* 0x38 - DMA Engine Selection */
	unsigned int	rsvd4;		/* 0x3C */
	unsigned int	oscc;		/* 0x40 - OSC Control */
	unsigned int	pwmcd;		/* 0x44 - PWM Clock divider */
	unsigned int	socmisc;	/* 0x48 - SoC Misc. */
	unsigned int	rsvd5[13];	/* 0x4C-0x7C: Reserved */
	unsigned int	bsmcr;		/* 0x80 - BSM Controrl */
	unsigned int	bsmst;		/* 0x84 - BSM Status */
	unsigned int	wes;		/* 0x88 - Wakeup Event Sensitivity*/
	unsigned int	west;		/* 0x8C - Wakeup Event Status */
	unsigned int	rsttiming;	/* 0x90 - Reset Timing  */
	unsigned int	intr_st;	/* 0x94 - PCU Interrupt Status */
	unsigned int	rsvd6[2];	/* 0x98-0x9C: Reserved */
	struct pcs	pcs1;		/* 0xA0-0xB0: PCS1 (clock scaling) */
	unsigned int	pcsrsvd1[3];	/* 0xB4-0xBC: Reserved */
	struct pcs	pcs2;		/* 0xC0-0xD0: PCS2 (AHB clock gating) */
	unsigned int	pcsrsvd2[3];	/* 0xD4-0xDC: Reserved */
	struct pcs	pcs3;		/* 0xE0-0xF0: PCS3 (APB clock gating) */
	unsigned int	pcsrsvd3[3];	/* 0xF4-0xFC: Reserved */
	struct pcs	pcs4;		/* 0x100-0x110: PCS4 main PLL scaling */
	unsigned int	pcsrsvd4[3];	/* 0x114-0x11C: Reserved */
	struct pcs	pcs5;		/* 0x120-0x130: PCS5 PCI PLL scaling */
	unsigned int	pcsrsvd5[3];	/* 0x134-0x13C: Reserved */
	struct pcs	pcs6;		/* 0x140-0x150: PCS6 AC97 PLL scaling */
	unsigned int	pcsrsvd6[3];	/* 0x154-0x15C: Reserved */
	struct pcs	pcs7;		/* 0x160-0x170: PCS7 GMAC PLL scaling */
	unsigned int	pcsrsvd7[3];	/* 0x174-0x17C: Reserved */
	struct pcs	pcs8;		/* 0x180-0x190: PCS8 voltage scaling */
	unsigned int	pcsrsvd8[3];	/* 0x194-0x19C: Reserved */
	struct pcs	pcs9;		/* 0x1A0-0x1B0: PCS9 power control */
	unsigned int	pcsrsvd9[93];	/* 0x1B4-0x3FC: Reserved */
	unsigned int	pmspdm[40];	/* 0x400-0x4fC: Power Manager
							Scratch Pad Memory 0 */
};
#endif /* __ASSEMBLY__ */

/*
 * PCU Revision Register (ro)
 */
#define ANDES_PCU_REV_NUMBER_PCS(x)	(((x) >> 0) & 0xff)
#define ANDES_PCU_REV_VER(x)		(((x) >> 16) & 0xffff)

/*
 * Scratch Pad Info Register (ro)
 */
#define ANDES_PCU_SPINFO_SIZE(x)	(((x) >> 0) & 0xff)
#define ANDES_PCU_SPINFO_OFFSET(x)	(((x) >> 8) & 0xf)

/*
 * SoC ID Register (ro)
 */
#define ANDES_PCU_SOC_ID_VER_MINOR(x)	(((x) >> 0) & 0xf)
#define ANDES_PCU_SOC_ID_VER_MAJOR(x)	(((x) >> 4) & 0xfff)
#define ANDES_PCU_SOC_ID_DEVICEID(x)	(((x) >> 16) & 0xffff)

/*
 * SoC AHB Configuration Register (ro)
 */
#define ANDES_PCU_SOC_AHB_AHBC(x)		((x) << 0)
#define ANDES_PCU_SOC_AHB_APBREG(x)		((x) << 1)
#define ANDES_PCU_SOC_AHB_APB(x)		((x) << 2)
#define ANDES_PCU_SOC_AHB_DLM1(x)		((x) << 3)
#define ANDES_PCU_SOC_AHB_SPIROM(x)		((x) << 4)
#define ANDES_PCU_SOC_AHB_DDR2C(x)		((x) << 5)
#define ANDES_PCU_SOC_AHB_DDR2MEM(x)		((x) << 6)
#define ANDES_PCU_SOC_AHB_DMAC(x)		((x) << 7)
#define ANDES_PCU_SOC_AHB_DLM2(x)		((x) << 8)
#define ANDES_PCU_SOC_AHB_GPU(x)		((x) << 9)
#define ANDES_PCU_SOC_AHB_GMAC(x)		((x) << 12)
#define ANDES_PCU_SOC_AHB_IDE(x)		((x) << 13)
#define ANDES_PCU_SOC_AHB_USBOTG(x)		((x) << 14)
#define ANDES_PCU_SOC_AHB_INTC(x)		((x) << 15)
#define ANDES_PCU_SOC_AHB_LPCIO(x)		((x) << 16)
#define ANDES_PCU_SOC_AHB_LPCREG(x)		((x) << 17)
#define ANDES_PCU_SOC_AHB_PCIIO(x)		((x) << 18)
#define ANDES_PCU_SOC_AHB_PCIMEM(x)		((x) << 19)
#define ANDES_PCU_SOC_AHB_L2CC(x)		((x) << 20)
#define ANDES_PCU_SOC_AHB_AHB2AHBREG(x)		((x) << 27)
#define ANDES_PCU_SOC_AHB_AHB2AHBMEM0(x)	((x) << 28)
#define ANDES_PCU_SOC_AHB_AHB2AHBMEM1(x)	((x) << 29)
#define ANDES_PCU_SOC_AHB_AHB2AHBMEM2(x)	((x) << 30)
#define ANDES_PCU_SOC_AHB_AHB2AHBMEM3(x)	((x) << 31)

/*
 * SoC APB Configuration Register (ro)
 */
#define ANDES_PCU_SOC_APB_CFC(x)	((x) << 1)
#define ANDES_PCU_SOC_APB_SSP(x)	((x) << 2)
#define ANDES_PCU_SOC_APB_UART1(x)	((x) << 3)
#define ANDES_PCU_SOC_APB_SDC(x)	((x) << 5)
#define ANDES_PCU_SOC_APB_AC97I2S(x)	((x) << 6)
#define ANDES_PCU_SOC_APB_UART2(x)	((x) << 8)
#define ANDES_PCU_SOC_APB_PCU(x)	((x) << 16)
#define ANDES_PCU_SOC_APB_TMR(x)	((x) << 17)
#define ANDES_PCU_SOC_APB_WDT(x)	((x) << 18)
#define ANDES_PCU_SOC_APB_RTC(x)	((x) << 19)
#define ANDES_PCU_SOC_APB_GPIO(x)	((x) << 20)
#define ANDES_PCU_SOC_APB_I2C(x)	((x) << 22)
#define ANDES_PCU_SOC_APB_PWM(x)	((x) << 23)

/*
 * Driving Capability and Slew Rate Control Register 0 (rw)
 */
#define ANDES_PCU_DCSRCR0_TRIAHB(x)	(((x) & 0x1f) << 0)
#define ANDES_PCU_DCSRCR0_LPC(x)	(((x) & 0xf) << 8)
#define ANDES_PCU_DCSRCR0_ULPI(x)	(((x) & 0xf) << 12)
#define ANDES_PCU_DCSRCR0_GMAC(x)	(((x) & 0xf) << 16)
#define ANDES_PCU_DCSRCR0_GPU(x)	(((x) & 0xf) << 20)

/*
 * Driving Capability and Slew Rate Control Register 1 (rw)
 */
#define ANDES_PCU_DCSRCR1_I2C(x)	(((x) & 0xf) << 0)

/*
 * Driving Capability and Slew Rate Control Register 2 (rw)
 */
#define ANDES_PCU_DCSRCR2_UART1(x)	(((x) & 0xf) << 0)
#define ANDES_PCU_DCSRCR2_UART2(x)	(((x) & 0xf) << 4)
#define ANDES_PCU_DCSRCR2_AC97(x)	(((x) & 0xf) << 8)
#define ANDES_PCU_DCSRCR2_SPI(x)	(((x) & 0xf) << 12)
#define ANDES_PCU_DCSRCR2_SD(x)		(((x) & 0xf) << 16)
#define ANDES_PCU_DCSRCR2_CFC(x)	(((x) & 0xf) << 20)
#define ANDES_PCU_DCSRCR2_GPIO(x)	(((x) & 0xf) << 24)
#define ANDES_PCU_DCSRCR2_PCU(x)	(((x) & 0xf) << 28)

/*
 * Multi-function Port Setting Register 0 (rw)
 */
#define ANDES_PCU_MFPSR0_PCIMODE(x)		((x) << 0)
#define ANDES_PCU_MFPSR0_IDEMODE(x)		((x) << 1)
#define ANDES_PCU_MFPSR0_MINI_TC01(x)		((x) << 2)
#define ANDES_PCU_MFPSR0_AHB_DEBUG(x)		((x) << 3)
#define ANDES_PCU_MFPSR0_AHB_TARGET(x)		((x) << 4)
#define ANDES_PCU_MFPSR0_DEFAULT_IVB(x)		(((x) & 0x7) << 28)
#define ANDES_PCU_MFPSR0_DEFAULT_ENDIAN(x)	((x) << 31)

/*
 * Multi-function Port Setting Register 1 (rw)
 */
#define ANDES_PCU_MFPSR1_SUSPEND(x)		((x) << 0)
#define ANDES_PCU_MFPSR1_PWM0(x)		((x) << 1)
#define ANDES_PCU_MFPSR1_PWM1(x)		((x) << 2)
#define ANDES_PCU_MFPSR1_AC97CLKOUT(x)		((x) << 3)
#define ANDES_PCU_MFPSR1_PWREN(x)		((x) << 4)
#define ANDES_PCU_MFPSR1_PME(x)			((x) << 5)
#define ANDES_PCU_MFPSR1_I2C(x)			((x) << 6)
#define ANDES_PCU_MFPSR1_UART1(x)		((x) << 7)
#define ANDES_PCU_MFPSR1_UART2(x)		((x) << 8)
#define ANDES_PCU_MFPSR1_SPI(x)			((x) << 9)
#define ANDES_PCU_MFPSR1_SD(x)			((x) << 10)
#define ANDES_PCU_MFPSR1_GPUPLLSRC(x)		((x) << 27)
#define ANDES_PCU_MFPSR1_DVOMODE(x)		((x) << 28)
#define ANDES_PCU_MFPSR1_HSMP_FAST_REQ(x)	((x) << 29)
#define ANDES_PCU_MFPSR1_AHB_FAST_REQ(x)	((x) << 30)
#define ANDES_PCU_MFPSR1_PMUR_EXT_INT(x)	((x) << 31)

/*
 * DMA Engine Selection Register (rw)
 */
#define ANDES_PCU_DMAES_AC97RX(x)		((x) << 2)
#define ANDES_PCU_DMAES_AC97TX(x)		((x) << 3)
#define ANDES_PCU_DMAES_UART1RX(x)		((x) << 4)
#define ANDES_PCU_DMAES_UART1TX(x)		((x) << 5)
#define ANDES_PCU_DMAES_UART2RX(x)		((x) << 6)
#define ANDES_PCU_DMAES_UART2TX(x)		((x) << 7)
#define ANDES_PCU_DMAES_SDDMA(x)		((x) << 8)
#define ANDES_PCU_DMAES_CFCDMA(x)		((x) << 9)

/*
 * OSC Control Register (rw)
 */
#define ANDES_PCU_OSCC_OSCH_OFF(x)	((x) << 0)
#define ANDES_PCU_OSCC_OSCH_STABLE(x)	((x) << 1)
#define ANDES_PCU_OSCC_OSCH_TRI(x)	((x) << 2)
#define ANDES_PCU_OSCC_OSCH_RANGE(x)	(((x) & 0x3) << 4)
#define ANDES_PCU_OSCC_OSCH2_RANGE(x)	(((x) & 0x3) << 6)
#define ANDES_PCU_OSCC_OSCH3_RANGE(x)	(((x) & 0x3) << 8)

/*
 * PWM Clock Divider Register (rw)
 */
#define ANDES_PCU_PWMCD_PWMDIV(x)	(((x) & 0xf) << 0)

/*
 * SoC Misc. Register (rw)
 */
#define ANDES_PCU_SOCMISC_RSCPUA(x)		((x) << 0)
#define ANDES_PCU_SOCMISC_RSCPUB(x)		((x) << 1)
#define ANDES_PCU_SOCMISC_RSPCI(x)		((x) << 2)
#define ANDES_PCU_SOCMISC_USBWAKE(x)		((x) << 3)
#define ANDES_PCU_SOCMISC_EXLM_WAITA(x)		(((x) & 0x3) << 4)
#define ANDES_PCU_SOCMISC_EXLM_WAITB(x)		(((x) & 0x3) << 6)
#define ANDES_PCU_SOCMISC_DDRPLL_BYPASS(x)	(((x) << 8)
#define ANDES_PCU_SOCMISC_300MHZSEL(x)		(((x) << 9)
#define ANDES_PCU_SOCMISC_DDRDLL_SRST(x)	(((x) << 10)
#define ANDES_PCU_SOCMISC_DDRDDQ_TEST(x)	(((x) << 11)
#define ANDES_PCU_SOCMISC_DDRDLL_TEST(x)	(((x) << 12)
#define ANDES_PCU_SOCMISC_GPUPLL_BYPASS(x)	(((x) << 13)
#define ANDES_PCU_SOCMISC_ENCPUA(x)		(((x) << 14)
#define ANDES_PCU_SOCMISC_ENCPUB(x)		(((x) << 15)
#define ANDES_PCU_SOCMISC_PWON_PWBTN(x)		(((x) << 16)
#define ANDES_PCU_SOCMISC_PWON_GPIO1(x)		(((x) << 17)
#define ANDES_PCU_SOCMISC_PWON_GPIO2(x)		(((x) << 18)
#define ANDES_PCU_SOCMISC_PWON_GPIO3(x)		(((x) << 19)
#define ANDES_PCU_SOCMISC_PWON_GPIO4(x)		(((x) << 20)
#define ANDES_PCU_SOCMISC_PWON_GPIO5(x)		(((x) << 21)
#define ANDES_PCU_SOCMISC_PWON_WOL(x)		(((x) << 22)
#define ANDES_PCU_SOCMISC_PWON_RTC(x)		(((x) << 23)
#define ANDES_PCU_SOCMISC_PWON_RTCALM(x)	(((x) << 24)
#define ANDES_PCU_SOCMISC_PWON_XDBGIN(x)	(((x) << 25)
#define ANDES_PCU_SOCMISC_PWON_PME(x)		(((x) << 26)
#define ANDES_PCU_SOCMISC_PWON_PWFAIL(x)	(((x) << 27)
#define ANDES_PCU_SOCMISC_CPUA_SRSTED(x)	(((x) << 28)
#define ANDES_PCU_SOCMISC_CPUB_SRSTED(x)	(((x) << 29)
#define ANDES_PCU_SOCMISC_WD_RESET(x)		(((x) << 30)
#define ANDES_PCU_SOCMISC_HW_RESET(x)		(((x) << 31)

/*
 * BSM Control Register (rw)
 */
#define ANDES_PCU_BSMCR_LINK0(x)	(((x) & 0xf) << 0)
#define ANDES_PCU_BSMCR_LINK1(x)	(((x) & 0xf) << 4)
#define ANDES_PCU_BSMCR_SYNCSRC(x)	(((x) & 0xf) << 24)
#define ANDES_PCU_BSMCR_CMD(x)		(((x) & 0x7) << 28)
#define ANDES_PCU_BSMCR_IE(x)		((x) << 31)

/*
 * BSM Status Register
 */
#define ANDES_PCU_BSMSR_CI0(x)		(((x) & 0xf) << 0)
#define ANDES_PCU_BSMSR_CI1(x)		(((x) & 0xf) << 4)
#define ANDES_PCU_BSMSR_SYNCSRC(x)	(((x) & 0xf) << 24)
#define ANDES_PCU_BSMSR_BSMST(x)	(((x) & 0xf) << 28)

/*
 * Wakeup Event Sensitivity Register (rw)
 */
#define ANDES_PCU_WESR_POLOR(x)		(((x) & 0xff) << 0)

/*
 * Wakeup Event Status Register (ro)
 */
#define ANDES_PCU_WEST_SIG(x)		(((x) & 0xff) << 0)

/*
 * Reset Timing Register
 */
#define ANDES_PCU_RSTTIMING_RG0(x)	(((x) & 0xff) << 0)
#define ANDES_PCU_RSTTIMING_RG1(x)	(((x) & 0xff) << 8)
#define ANDES_PCU_RSTTIMING_RG2(x)	(((x) & 0xff) << 16)
#define ANDES_PCU_RSTTIMING_RG3(x)	(((x) & 0xff) << 24)

/*
 * PCU Interrupt Status Register
 */
#define ANDES_PCU_INTR_ST_BSM(x)	((x) << 0)
#define ANDES_PCU_INTR_ST_PCS1(x)	((x) << 1)
#define ANDES_PCU_INTR_ST_PCS2(x)	((x) << 2)
#define ANDES_PCU_INTR_ST_PCS3(x)	((x) << 3)
#define ANDES_PCU_INTR_ST_PCS4(x)	((x) << 4)
#define ANDES_PCU_INTR_ST_PCS5(x)	((x) << 5)
#define ANDES_PCU_INTR_ST_PCS6(x)	((x) << 6)
#define ANDES_PCU_INTR_ST_PCS7(x)	((x) << 7)
#define ANDES_PCU_INTR_ST_PCS8(x)	((x) << 8)
#define ANDES_PCU_INTR_ST_PCS9(x)	((x) << 9)

/*
 * PCSx Configuration Register
 */
#define ANDES_PCU_PCSX_CR_WAKEUP_EN(x)	(((x) & 0xff) << 0)
#define ANDES_PCU_PCSX_CR_LW(x)		(((x) & 0xf) << 16)
#define ANDES_PCU_PCSX_CR_LS(x)		(((x) & 0xf) << 20)
#define ANDES_PCU_PCSX_CR_TYPE(x)	(((x) >> 28) & 0x7)	/* (ro) */

/*
 * PCSx Parameter Register (rw)
 */
#define ANDES_PCU_PCSX_PARM_NEXT(x)	(((x) & 0xffffff) << 0)
#define ANDES_PCU_PCSX_PARM_SYNCSRC(x)	(((x) & 0xf) << 24)
#define ANDES_PCU_PCSX_PARM_PCSCMD(x)	(((x) & 0x7) << 28)
#define ANDES_PCU_PCSX_PARM_IE(x)	(((x) << 31)

/*
 * PCSx Status Register 1
 */
#define ANDES_PCU_PCSX_STAT1_ERRNO(x)	(((x) & 0xf) << 0)
#define ANDES_PCU_PCSX_STAT1_ST(x)	(((x) & 0x7) << 28)

/*
 * PCSx Status Register 2
 */
#define ANDES_PCU_PCSX_STAT2_CRNTPARM(x)	(((x) & 0xffffff) << 0)
#define ANDES_PCU_PCSX_STAT2_SYNCSRC(x)		(((x) & 0xf) << 24)

/*
 * PCSx PDD Register
 * This is reserved for PCS(1-7)
 */
#define ANDES_PCU_PCS8_PDD_1BYTE(x)		(((x) & 0xff) << 0)
#define ANDES_PCU_PCS8_PDD_2BYTE(x)		(((x) & 0xff) << 8)
#define ANDES_PCU_PCS8_PDD_3BYTE(x)		(((x) & 0xff) << 16)
#define ANDES_PCU_PCS8_PDD_4BYTE(x)		(((x) & 0xff) << 24)

#define ANDES_PCU_PCS9_PDD_TIME1(x)		(((x) & 0x3f) << 0)
#define ANDES_PCU_PCS9_PDD_TIME2(x)		(((x) & 0x3f) << 6)
#define ANDES_PCU_PCS9_PDD_TIME3(x)		(((x) & 0x3f) << 12)
#define ANDES_PCU_PCS9_PDD_TIME4(x)		(((x) & 0x3f) << 18)
#define ANDES_PCU_PCS9_PDD_TICKTYPE(x)		((x) << 24)
#define ANDES_PCU_PCS9_PDD_GPU_SRST(x)		((x) << 27)
#define ANDES_PCU_PCS9_PDD_PWOFFTIME(x)		(((x) & 0x3) << 28)
#define ANDES_PCU_PCS9_PDD_SUS2DRAM(x)		((x) << 30)
#define ANDES_PCU_PCS9_PDD_CLRPWOFF_FLAG(x)	((x) << 31)

#endif	/* __ANDES_PCU_H */
