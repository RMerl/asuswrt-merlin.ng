/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * cpu.h
 *
 * AM33xx specific header file
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef _AM33XX_CPU_H
#define _AM33XX_CPU_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#include <asm/arch/hardware.h>

#define CL_BIT(x)			(0 << x)

/* Timer register bits */
#define TCLR_ST				BIT(0)	/* Start=1 Stop=0 */
#define TCLR_AR				BIT(1)	/* Auto reload */
#define TCLR_PRE			BIT(5)	/* Pre-scaler enable */
#define TCLR_PTV_SHIFT			(2)	/* Pre-scaler shift value */
#define TCLR_PRE_DISABLE		CL_BIT(5) /* Pre-scalar disable */
#define TCLR_CE				BIT(6)	/* compare mode enable */
#define TCLR_SCPWM			BIT(7)	/* pwm outpin behaviour */
#define TCLR_TCM			BIT(8)	/* edge detection of input pin*/
#define TCLR_TRG_SHIFT			(10)	/* trigmode on pwm outpin */
#define TCLR_PT				BIT(12)	/* pulse/toggle mode of outpin*/
#define TCLR_CAPTMODE			BIT(13) /* capture mode */
#define TCLR_GPOCFG			BIT(14)	/* 0=output,1=input */

#define TCFG_RESET			BIT(0)	/* software reset */
#define TCFG_EMUFREE			BIT(1)	/* behaviour of tmr on debug */
#define TCFG_IDLEMOD_SHIFT		(2)	/* power management */

/* cpu-id for AM43XX AM33XX and TI81XX family */
#define AM437X				0xB98C
#define AM335X				0xB944
#define TI81XX				0xB81E
#define DEVICE_ID			(CTRL_BASE + 0x0600)
#define DEVICE_ID_MASK			0x1FFF
#define PACKAGE_TYPE_SHIFT		16
#define PACKAGE_TYPE_MASK		(3 << 16)

/* Package Type */
#define PACKAGE_TYPE_UNDEFINED		0x0
#define PACKAGE_TYPE_ZCZ		0x1
#define PACKAGE_TYPE_ZCE		0x2
#define PACKAGE_TYPE_RESERVED		0x3

/* MPU max frequencies */
#define AM335X_ZCZ_300			0x1FEF
#define AM335X_ZCZ_600			0x1FAF
#define AM335X_ZCZ_720			0x1F2F
#define AM335X_ZCZ_800			0x1E2F
#define AM335X_ZCZ_1000			0x1C2F
#define AM335X_ZCE_300			0x1FDF
#define AM335X_ZCE_600			0x1F9F

/* This gives the status of the boot mode pins on the evm */
#define SYSBOOT_MASK			(BIT(0) | BIT(1) | BIT(2)\
					| BIT(3) | BIT(4))

#define PRM_RSTCTRL_RESET		0x01
#define PRM_RSTST_WARM_RESET_MASK	0x232

/* EMIF Control register bits */
#define EMIF_CTRL_DEVOFF	BIT(0)

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
#include <asm/ti-common/omap_wdt.h>

#ifndef CONFIG_AM43XX
/* Encapsulating core pll registers */
struct cm_wkuppll {
	unsigned int wkclkstctrl;	/* offset 0x00 */
	unsigned int wkctrlclkctrl;	/* offset 0x04 */
	unsigned int wkgpio0clkctrl;	/* offset 0x08 */
	unsigned int wkl4wkclkctrl;	/* offset 0x0c */
	unsigned int timer0clkctrl;	/* offset 0x10 */
	unsigned int resv2[3];
	unsigned int idlestdpllmpu;	/* offset 0x20 */
	unsigned int sscdeltamstepdllmpu; /* off  0x24 */
	unsigned int sscmodfreqdivdpllmpu; /* off 0x28 */
	unsigned int clkseldpllmpu;	/* offset 0x2c */
	unsigned int resv4[1];
	unsigned int idlestdpllddr;	/* offset 0x34 */
	unsigned int resv5[2];
	unsigned int clkseldpllddr;	/* offset 0x40 */
	unsigned int resv6[4];
	unsigned int clkseldplldisp;	/* offset 0x54 */
	unsigned int resv7[1];
	unsigned int idlestdpllcore;	/* offset 0x5c */
	unsigned int resv8[2];
	unsigned int clkseldpllcore;	/* offset 0x68 */
	unsigned int resv9[1];
	unsigned int idlestdpllper;	/* offset 0x70 */
	unsigned int resv10[2];
	unsigned int clkdcoldodpllper;	/* offset 0x7c */
	unsigned int divm4dpllcore;	/* offset 0x80 */
	unsigned int divm5dpllcore;	/* offset 0x84 */
	unsigned int clkmoddpllmpu;	/* offset 0x88 */
	unsigned int clkmoddpllper;	/* offset 0x8c */
	unsigned int clkmoddpllcore;	/* offset 0x90 */
	unsigned int clkmoddpllddr;	/* offset 0x94 */
	unsigned int clkmoddplldisp;	/* offset 0x98 */
	unsigned int clkseldpllper;	/* offset 0x9c */
	unsigned int divm2dpllddr;	/* offset 0xA0 */
	unsigned int divm2dplldisp;	/* offset 0xA4 */
	unsigned int divm2dpllmpu;	/* offset 0xA8 */
	unsigned int divm2dpllper;	/* offset 0xAC */
	unsigned int resv11[1];
	unsigned int wkup_uart0ctrl;	/* offset 0xB4 */
	unsigned int wkup_i2c0ctrl;	/* offset 0xB8 */
	unsigned int wkup_adctscctrl;	/* offset 0xBC */
	unsigned int resv12;
	unsigned int timer1clkctrl;	/* offset 0xC4 */
	unsigned int resv13[4];
	unsigned int divm6dpllcore;	/* offset 0xD8 */
};

/**
 * Encapsulating peripheral functional clocks
 * pll registers
 */
struct cm_perpll {
	unsigned int l4lsclkstctrl;	/* offset 0x00 */
	unsigned int l3sclkstctrl;	/* offset 0x04 */
	unsigned int l4fwclkstctrl;	/* offset 0x08 */
	unsigned int l3clkstctrl;	/* offset 0x0c */
	unsigned int resv1;
	unsigned int cpgmac0clkctrl;	/* offset 0x14 */
	unsigned int lcdclkctrl;	/* offset 0x18 */
	unsigned int usb0clkctrl;	/* offset 0x1C */
	unsigned int resv2;
	unsigned int tptc0clkctrl;	/* offset 0x24 */
	unsigned int emifclkctrl;	/* offset 0x28 */
	unsigned int ocmcramclkctrl;	/* offset 0x2c */
	unsigned int gpmcclkctrl;	/* offset 0x30 */
	unsigned int mcasp0clkctrl;	/* offset 0x34 */
	unsigned int uart5clkctrl;	/* offset 0x38 */
	unsigned int mmc0clkctrl;	/* offset 0x3C */
	unsigned int elmclkctrl;	/* offset 0x40 */
	unsigned int i2c2clkctrl;	/* offset 0x44 */
	unsigned int i2c1clkctrl;	/* offset 0x48 */
	unsigned int spi0clkctrl;	/* offset 0x4C */
	unsigned int spi1clkctrl;	/* offset 0x50 */
	unsigned int resv3[3];
	unsigned int l4lsclkctrl;	/* offset 0x60 */
	unsigned int l4fwclkctrl;	/* offset 0x64 */
	unsigned int mcasp1clkctrl;	/* offset 0x68 */
	unsigned int uart1clkctrl;	/* offset 0x6C */
	unsigned int uart2clkctrl;	/* offset 0x70 */
	unsigned int uart3clkctrl;	/* offset 0x74 */
	unsigned int uart4clkctrl;	/* offset 0x78 */
	unsigned int timer7clkctrl;	/* offset 0x7C */
	unsigned int timer2clkctrl;	/* offset 0x80 */
	unsigned int timer3clkctrl;	/* offset 0x84 */
	unsigned int timer4clkctrl;	/* offset 0x88 */
	unsigned int resv4[8];
	unsigned int gpio1clkctrl;	/* offset 0xAC */
	unsigned int gpio2clkctrl;	/* offset 0xB0 */
	unsigned int gpio3clkctrl;	/* offset 0xB4 */
	unsigned int resv5;
	unsigned int tpccclkctrl;	/* offset 0xBC */
	unsigned int dcan0clkctrl;	/* offset 0xC0 */
	unsigned int dcan1clkctrl;	/* offset 0xC4 */
	unsigned int resv6;
	unsigned int epwmss1clkctrl;	/* offset 0xCC */
	unsigned int emiffwclkctrl;	/* offset 0xD0 */
	unsigned int epwmss0clkctrl;	/* offset 0xD4 */
	unsigned int epwmss2clkctrl;	/* offset 0xD8 */
	unsigned int l3instrclkctrl;	/* offset 0xDC */
	unsigned int l3clkctrl;		/* Offset 0xE0 */
	unsigned int resv8[2];
	unsigned int timer5clkctrl;	/* offset 0xEC */
	unsigned int timer6clkctrl;	/* offset 0xF0 */
	unsigned int mmc1clkctrl;	/* offset 0xF4 */
	unsigned int mmc2clkctrl;	/* offset 0xF8 */
	unsigned int resv9[8];
	unsigned int l4hsclkstctrl;	/* offset 0x11C */
	unsigned int l4hsclkctrl;	/* offset 0x120 */
	unsigned int resv10[8];
	unsigned int cpswclkstctrl;	/* offset 0x144 */
	unsigned int lcdcclkstctrl;	/* offset 0x148 */
};

/* Encapsulating Display pll registers */
struct cm_dpll {
	unsigned int resv1;
	unsigned int clktimer7clk;	/* offset 0x04 */
	unsigned int clktimer2clk;	/* offset 0x08 */
	unsigned int clktimer3clk;	/* offset 0x0C */
	unsigned int clktimer4clk;	/* offset 0x10 */
	unsigned int resv2;
	unsigned int clktimer5clk;	/* offset 0x18 */
	unsigned int clktimer6clk;	/* offset 0x1C */
	unsigned int resv3[2];
	unsigned int clktimer1clk;	/* offset 0x28 */
	unsigned int resv4[2];
	unsigned int clklcdcpixelclk;	/* offset 0x34 */
};

struct prm_device_inst {
	unsigned int prm_rstctrl;
	unsigned int prm_rsttime;
	unsigned int prm_rstst;
};
#else
/* Encapsulating core pll registers */
struct cm_wkuppll {
	unsigned int resv0[136];
	unsigned int wkl4wkclkctrl;	/* offset 0x220 */
	unsigned int resv1[7];
	unsigned int usbphy0clkctrl;	/* offset 0x240 */
	unsigned int resv112;
	unsigned int usbphy1clkctrl;	/* offset 0x248 */
	unsigned int resv113[45];
	unsigned int wkclkstctrl;	/* offset 0x300 */
	unsigned int resv2[15];
	unsigned int wkup_i2c0ctrl;	/* offset 0x340 */
	unsigned int resv3;
	unsigned int wkup_uart0ctrl;	/* offset 0x348 */
	unsigned int resv4[5];
	unsigned int wkctrlclkctrl;	/* offset 0x360 */
	unsigned int resv5;
	unsigned int wkgpio0clkctrl;	/* offset 0x368 */

	unsigned int resv6[109];
	unsigned int clkmoddpllcore;	/* offset 0x520 */
	unsigned int idlestdpllcore;	/* offset 0x524 */
	unsigned int resv61;
	unsigned int clkseldpllcore;	/* offset 0x52C */
	unsigned int resv7[2];
	unsigned int divm4dpllcore;	/* offset 0x538 */
	unsigned int divm5dpllcore;	/* offset 0x53C */
	unsigned int divm6dpllcore;	/* offset 0x540 */

	unsigned int resv8[7];
	unsigned int clkmoddpllmpu;	/* offset 0x560 */
	unsigned int idlestdpllmpu;	/* offset 0x564 */
	unsigned int resv9;
	unsigned int clkseldpllmpu;	/* offset 0x56c */
	unsigned int divm2dpllmpu;	/* offset 0x570 */

	unsigned int resv10[11];
	unsigned int clkmoddpllddr;	/* offset 0x5A0 */
	unsigned int idlestdpllddr;	/* offset 0x5A4 */
	unsigned int resv11;
	unsigned int clkseldpllddr;	/* offset 0x5AC */
	unsigned int divm2dpllddr;	/* offset 0x5B0 */

	unsigned int resv12[11];
	unsigned int clkmoddpllper;	/* offset 0x5E0 */
	unsigned int idlestdpllper;	/* offset 0x5E4 */
	unsigned int resv13;
	unsigned int clkseldpllper;	/* offset 0x5EC */
	unsigned int divm2dpllper;	/* offset 0x5F0 */
	unsigned int resv14[8];
	unsigned int clkdcoldodpllper;	/* offset 0x614 */

	unsigned int resv15[2];
	unsigned int clkmoddplldisp;	/* offset 0x620 */
	unsigned int resv16[2];
	unsigned int clkseldplldisp;	/* offset 0x62C */
	unsigned int divm2dplldisp;	/* offset 0x630 */
};

/*
 * Encapsulating peripheral functional clocks
 * pll registers
 */
struct cm_perpll {
	unsigned int l3clkstctrl;	/* offset 0x00 */
	unsigned int resv0[7];
	unsigned int l3clkctrl;		/* Offset 0x20 */
	unsigned int resv112[7];
	unsigned int l3instrclkctrl;	/* offset 0x40 */
	unsigned int resv2[3];
	unsigned int ocmcramclkctrl;	/* offset 0x50 */
	unsigned int resv3[9];
	unsigned int tpccclkctrl;	/* offset 0x78 */
	unsigned int resv4;
	unsigned int tptc0clkctrl;	/* offset 0x80 */

	unsigned int resv5[7];
	unsigned int l4hsclkctrl;	/* offset 0x0A0 */
	unsigned int resv6;
	unsigned int l4fwclkctrl;	/* offset 0x0A8 */
	unsigned int resv7[85];
	unsigned int l3sclkstctrl;	/* offset 0x200 */
	unsigned int resv8[7];
	unsigned int gpmcclkctrl;	/* offset 0x220 */
	unsigned int resv9[5];
	unsigned int mcasp0clkctrl;	/* offset 0x238 */
	unsigned int resv10;
	unsigned int mcasp1clkctrl;	/* offset 0x240 */
	unsigned int resv11;
	unsigned int mmc2clkctrl;	/* offset 0x248 */
	unsigned int resv12[3];
	unsigned int qspiclkctrl;       /* offset 0x258 */
	unsigned int resv121;
	unsigned int usb0clkctrl;	/* offset 0x260 */
	unsigned int resv122;
	unsigned int usb1clkctrl;	/* offset 0x268 */
	unsigned int resv13[101];
	unsigned int l4lsclkstctrl;	/* offset 0x400 */
	unsigned int resv14[7];
	unsigned int l4lsclkctrl;	/* offset 0x420 */
	unsigned int resv15;
	unsigned int dcan0clkctrl;	/* offset 0x428 */
	unsigned int resv16;
	unsigned int dcan1clkctrl;	/* offset 0x430 */
	unsigned int resv17[13];
	unsigned int elmclkctrl;	/* offset 0x468 */

	unsigned int resv18[3];
	unsigned int gpio1clkctrl;	/* offset 0x478 */
	unsigned int resv19;
	unsigned int gpio2clkctrl;	/* offset 0x480 */
	unsigned int resv20;
	unsigned int gpio3clkctrl;	/* offset 0x488 */
	unsigned int resv41;
	unsigned int gpio4clkctrl;	/* offset 0x490 */
	unsigned int resv42;
	unsigned int gpio5clkctrl;	/* offset 0x498 */
	unsigned int resv21[3];

	unsigned int i2c1clkctrl;	/* offset 0x4A8 */
	unsigned int resv22;
	unsigned int i2c2clkctrl;	/* offset 0x4B0 */
	unsigned int resv23[3];
	unsigned int mmc0clkctrl;	/* offset 0x4C0 */
	unsigned int resv24;
	unsigned int mmc1clkctrl;	/* offset 0x4C8 */

	unsigned int resv25[13];
	unsigned int spi0clkctrl;	/* offset 0x500 */
	unsigned int resv26;
	unsigned int spi1clkctrl;	/* offset 0x508 */
	unsigned int resv27[9];
	unsigned int timer2clkctrl;	/* offset 0x530 */
	unsigned int resv28;
	unsigned int timer3clkctrl;	/* offset 0x538 */
	unsigned int resv29;
	unsigned int timer4clkctrl;	/* offset 0x540 */
	unsigned int resv30[5];
	unsigned int timer7clkctrl;	/* offset 0x558 */

	unsigned int resv31[9];
	unsigned int uart1clkctrl;	/* offset 0x580 */
	unsigned int resv32;
	unsigned int uart2clkctrl;	/* offset 0x588 */
	unsigned int resv33;
	unsigned int uart3clkctrl;	/* offset 0x590 */
	unsigned int resv34;
	unsigned int uart4clkctrl;	/* offset 0x598 */
	unsigned int resv35;
	unsigned int uart5clkctrl;	/* offset 0x5A0 */
	unsigned int resv36[5];
	unsigned int usbphyocp2scp0clkctrl;	/* offset 0x5B8 */
	unsigned int resv361;
	unsigned int usbphyocp2scp1clkctrl;	/* offset 0x5C0 */
	unsigned int resv3611[79];

	unsigned int emifclkstctrl;	/* offset 0x700 */
	unsigned int resv362[7];
	unsigned int emifclkctrl;	/* offset 0x720 */
	unsigned int resv37[3];
	unsigned int emiffwclkctrl;	/* offset 0x730 */
	unsigned int resv371;
	unsigned int otfaemifclkctrl;	/* offset 0x738 */
	unsigned int resv38[57];
	unsigned int lcdclkctrl;	/* offset 0x820 */
	unsigned int resv39[183];
	unsigned int cpswclkstctrl;	/* offset 0xB00 */
	unsigned int resv40[7];
	unsigned int cpgmac0clkctrl;	/* offset 0xB20 */
};

struct cm_device_inst {
	unsigned int cm_clkout1_ctrl;
	unsigned int cm_dll_ctrl;
};

struct prm_device_inst {
	unsigned int rstctrl;
	unsigned int rstst;
	unsigned int rsttime;
	unsigned int sram_count;
	unsigned int ldo_sram_core_set;	/* offset 0x10 */
	unsigned int ldo_sram_core_ctr;
	unsigned int ldo_sram_mpu_setu;
	unsigned int ldo_sram_mpu_ctrl;
	unsigned int io_count;		/* offset 0x20 */
	unsigned int io_pmctrl;
	unsigned int vc_val_bypass;
	unsigned int resv1;
	unsigned int emif_ctrl;		/* offset 0x30 */
};

struct cm_dpll {
	unsigned int resv1;
	unsigned int clktimer2clk;	/* offset 0x04 */
	unsigned int resv2[11];
	unsigned int clkselmacclk;	/* offset 0x34 */ 
};
#endif /* CONFIG_AM43XX */

/* Control Module RTC registers */
struct cm_rtc {
	unsigned int rtcclkctrl;	/* offset 0x0 */
	unsigned int clkstctrl;		/* offset 0x4 */
};

/* Timer 32 bit registers */
struct gptimer {
	unsigned int tidr;		/* offset 0x00 */
	unsigned char res1[12];
	unsigned int tiocp_cfg;		/* offset 0x10 */
	unsigned char res2[12];
	unsigned int tier;		/* offset 0x20 */
	unsigned int tistatr;		/* offset 0x24 */
	unsigned int tistat;		/* offset 0x28 */
	unsigned int tisr;		/* offset 0x2c */
	unsigned int tcicr;		/* offset 0x30 */
	unsigned int twer;		/* offset 0x34 */
	unsigned int tclr;		/* offset 0x38 */
	unsigned int tcrr;		/* offset 0x3c */
	unsigned int tldr;		/* offset 0x40 */
	unsigned int ttgr;		/* offset 0x44 */
	unsigned int twpc;		/* offset 0x48 */
	unsigned int tmar;		/* offset 0x4c */
	unsigned int tcar1;		/* offset 0x50 */
	unsigned int tscir;		/* offset 0x54 */
	unsigned int tcar2;		/* offset 0x58 */
};

/* UART Registers */
struct uart_sys {
	unsigned int resv1[21];
	unsigned int uartsyscfg;	/* offset 0x54 */
	unsigned int uartsyssts;	/* offset 0x58 */
};

/* VTP Registers */
struct vtp_reg {
	unsigned int vtp0ctrlreg;
};

/* Control Status Register */
struct ctrl_stat {
	unsigned int resv1[16];
	unsigned int statusreg;		/* ofset 0x40 */
	unsigned int resv2[51];
	unsigned int secure_emif_sdram_config;	/* offset 0x0110 */
	unsigned int resv3[319];
	unsigned int dev_attr;
};

/* AM33XX GPIO registers */
#define OMAP_GPIO_REVISION		0x0000
#define OMAP_GPIO_SYSCONFIG		0x0010
#define OMAP_GPIO_SYSSTATUS		0x0114
#define OMAP_GPIO_IRQSTATUS1		0x002c
#define OMAP_GPIO_IRQSTATUS2		0x0030
#define OMAP_GPIO_IRQSTATUS_SET_0	0x0034
#define OMAP_GPIO_IRQSTATUS_SET_1	0x0038
#define OMAP_GPIO_CTRL			0x0130
#define OMAP_GPIO_OE			0x0134
#define OMAP_GPIO_DATAIN		0x0138
#define OMAP_GPIO_DATAOUT		0x013c
#define OMAP_GPIO_LEVELDETECT0		0x0140
#define OMAP_GPIO_LEVELDETECT1		0x0144
#define OMAP_GPIO_RISINGDETECT		0x0148
#define OMAP_GPIO_FALLINGDETECT		0x014c
#define OMAP_GPIO_DEBOUNCE_EN		0x0150
#define OMAP_GPIO_DEBOUNCE_VAL		0x0154
#define OMAP_GPIO_CLEARDATAOUT		0x0190
#define OMAP_GPIO_SETDATAOUT		0x0194

/* Control Device Register */

 /* Control Device Register */
#define MREQPRIO_0_SAB_INIT1_MASK	0xFFFFFF8F
#define MREQPRIO_0_SAB_INIT0_MASK	0xFFFFFFF8
#define MREQPRIO_1_DSS_MASK		0xFFFFFF8F

struct ctrl_dev {
	unsigned int deviceid;		/* offset 0x00 */
	unsigned int resv1[7];
	unsigned int usb_ctrl0;		/* offset 0x20 */
	unsigned int resv2;
	unsigned int usb_ctrl1;		/* offset 0x28 */
	unsigned int resv3;
	unsigned int macid0l;		/* offset 0x30 */
	unsigned int macid0h;		/* offset 0x34 */
	unsigned int macid1l;		/* offset 0x38 */
	unsigned int macid1h;		/* offset 0x3c */
	unsigned int resv4[4];
	unsigned int miisel;		/* offset 0x50 */
	unsigned int resv5[7];
	unsigned int mreqprio_0;	/* offset 0x70 */
	unsigned int mreqprio_1;	/* offset 0x74 */
	unsigned int resv6[97];
	unsigned int efuse_sma;		/* offset 0x1FC */
};

/* Bandwidth Limiter Portion of the L3Fast Configuration Register */
#define BW_LIMITER_BW_FRAC_MASK         0xFFFFFFE0
#define BW_LIMITER_BW_INT_MASK          0xFFFFFFF0
#define BW_LIMITER_BW_WATERMARK_MASK    0xFFFFF800

struct l3f_cfg_bwlimiter {
	u32 padding0[2];
	u32 modena_init0_bw_fractional;
	u32 modena_init0_bw_integer;
	u32 modena_init0_watermark_0;
};

/* gmii_sel register defines */
#define GMII1_SEL_MII		0x0
#define GMII1_SEL_RMII		0x1
#define GMII1_SEL_RGMII		0x2
#define GMII2_SEL_MII		0x0
#define GMII2_SEL_RMII		0x4
#define GMII2_SEL_RGMII		0x8
#define RGMII1_IDMODE		BIT(4)
#define RGMII2_IDMODE		BIT(5)
#define RMII1_IO_CLK_EN		BIT(6)
#define RMII2_IO_CLK_EN		BIT(7)

#define MII_MODE_ENABLE		(GMII1_SEL_MII | GMII2_SEL_MII)
#define RMII_MODE_ENABLE        (GMII1_SEL_RMII | GMII2_SEL_RMII)
#define RGMII_MODE_ENABLE	(GMII1_SEL_RGMII | GMII2_SEL_RGMII)
#define RGMII_INT_DELAY		(RGMII1_IDMODE | RGMII2_IDMODE)
#define RMII_CHIPCKL_ENABLE     (RMII1_IO_CLK_EN | RMII2_IO_CLK_EN)

/* PWMSS */
struct pwmss_regs {
	unsigned int idver;
	unsigned int sysconfig;
	unsigned int clkconfig;
	unsigned int clkstatus;
};
#define ECAP_CLK_EN		BIT(0)
#define ECAP_CLK_STOP_REQ	BIT(1)
#define EPWM_CLK_EN		BIT(8)
#define EPWM_CLK_STOP_REQ	BIT(9)

struct pwmss_ecap_regs {
	unsigned int tsctr;
	unsigned int ctrphs;
	unsigned int cap1;
	unsigned int cap2;
	unsigned int cap3;
	unsigned int cap4;
	unsigned int resv1[4];
	unsigned short ecctl1;
	unsigned short ecctl2;
};

struct pwmss_epwm_regs {
	unsigned short tbctl;
	unsigned short tbsts;
	unsigned short tbphshr;
	unsigned short tbphs;
	unsigned short tbcnt;
	unsigned short tbprd;
	unsigned short res1;
	unsigned short cmpctl;
	unsigned short cmpahr;
	unsigned short cmpa;
	unsigned short cmpb;
	unsigned short aqctla;
	unsigned short aqctlb;
	unsigned short aqsfrc;
	unsigned short aqcsfrc;
	unsigned short dbctl;
	unsigned short dbred;
	unsigned short dbfed;
	unsigned short tzsel;
	unsigned short tzctl;
	unsigned short tzflg;
	unsigned short tzclr;
	unsigned short tzfrc;
	unsigned short etsel;
	unsigned short etps;
	unsigned short etflg;
	unsigned short etclr;
	unsigned short etfrc;
	unsigned short pcctl;
	unsigned int res2[66];
	unsigned short hrcnfg;
};

/* Capture Control register 2 */
#define ECTRL2_SYNCOSEL_MASK	(0x03 << 6)
#define ECTRL2_MDSL_ECAP	BIT(9)
#define ECTRL2_CTRSTP_FREERUN	BIT(4)
#define ECTRL2_PLSL_LOW		BIT(10)
#define ECTRL2_SYNC_EN		BIT(5)

#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#endif /* _AM33XX_CPU_H */
