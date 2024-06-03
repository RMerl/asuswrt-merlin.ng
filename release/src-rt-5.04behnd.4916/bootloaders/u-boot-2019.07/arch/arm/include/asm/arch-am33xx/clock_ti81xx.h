/*
 * ti81xx.h
 *
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Antoine Tenart, <atenart@adeneo-embedded.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#ifndef _CLOCK_TI81XX_H_
#define _CLOCK_TI81XX_H_

#define PRCM_MOD_EN     0x2

#define CM_DEFAULT_BASE (PRCM_BASE + 0x0500)
#define CM_ALWON_BASE   (PRCM_BASE + 0x1400)

struct cm_def {
	unsigned int resv0[2];
	unsigned int l3fastclkstctrl;
	unsigned int resv1[1];
	unsigned int pciclkstctrl;
	unsigned int resv2[1];
	unsigned int ducaticlkstctrl;
	unsigned int resv3[1];
	unsigned int emif0clkctrl;
	unsigned int emif1clkctrl;
	unsigned int dmmclkctrl;
	unsigned int fwclkctrl;
	unsigned int resv4[10];
	unsigned int usbclkctrl;
	unsigned int resv5[1];
	unsigned int sataclkctrl;
	unsigned int resv6[4];
	unsigned int ducaticlkctrl;
	unsigned int pciclkctrl;
};

struct cm_alwon {
	unsigned int l3slowclkstctrl;
	unsigned int ethclkstctrl;
	unsigned int l3medclkstctrl;
	unsigned int mmu_clkstctrl;
	unsigned int mmucfg_clkstctrl;
	unsigned int ocmc0clkstctrl;
#if defined(CONFIG_TI814X)
	unsigned int vcpclkstctrl;
#elif defined(CONFIG_TI816X)
	unsigned int ocmc1clkstctrl;
#endif
	unsigned int mpuclkstctrl;
	unsigned int sysclk4clkstctrl;
	unsigned int sysclk5clkstctrl;
	unsigned int sysclk6clkstctrl;
	unsigned int rtcclkstctrl;
	unsigned int l3fastclkstctrl;
	unsigned int resv0[67];
	unsigned int mcasp0clkctrl;
	unsigned int mcasp1clkctrl;
	unsigned int mcasp2clkctrl;
	unsigned int mcbspclkctrl;
	unsigned int uart0clkctrl;
	unsigned int uart1clkctrl;
	unsigned int uart2clkctrl;
	unsigned int gpio0clkctrl;
	unsigned int gpio1clkctrl;
	unsigned int i2c0clkctrl;
	unsigned int i2c1clkctrl;
#if defined(CONFIG_TI814X)
	unsigned int mcasp345clkctrl;
	unsigned int atlclkctrl;
	unsigned int mlbclkctrl;
	unsigned int pataclkctrl;
	unsigned int resv1[1];
	unsigned int uart3clkctrl;
	unsigned int uart4clkctrl;
	unsigned int uart5clkctrl;
#elif defined(CONFIG_TI816X)
	unsigned int resv1[1];
	unsigned int timer1clkctrl;
	unsigned int timer2clkctrl;
	unsigned int timer3clkctrl;
	unsigned int timer4clkctrl;
	unsigned int timer5clkctrl;
	unsigned int timer6clkctrl;
	unsigned int timer7clkctrl;
#endif
	unsigned int wdtimerclkctrl;
	unsigned int spiclkctrl;
	unsigned int mailboxclkctrl;
	unsigned int spinboxclkctrl;
	unsigned int mmudataclkctrl;
	unsigned int resv2[2];
	unsigned int mmucfgclkctrl;
#if defined(CONFIG_TI814X)
	unsigned int resv3[2];
#elif defined(CONFIG_TI816X)
	unsigned int resv3[1];
	unsigned int sdioclkctrl;
#endif
	unsigned int ocmc0clkctrl;
#if defined(CONFIG_TI814X)
	unsigned int vcpclkctrl;
#elif defined(CONFIG_TI816X)
	unsigned int ocmc1clkctrl;
#endif
	unsigned int resv4[2];
	unsigned int controlclkctrl;
	unsigned int resv5[2];
	unsigned int gpmcclkctrl;
	unsigned int ethernet0clkctrl;
	unsigned int ethernet1clkctrl;
	unsigned int mpuclkctrl;
#if defined(CONFIG_TI814X)
	unsigned int debugssclkctrl;
#elif defined(CONFIG_TI816X)
	unsigned int resv6[1];
#endif
	unsigned int l3clkctrl;
	unsigned int l4hsclkctrl;
	unsigned int l4lsclkctrl;
	unsigned int rtcclkctrl;
	unsigned int tpccclkctrl;
	unsigned int tptc0clkctrl;
	unsigned int tptc1clkctrl;
	unsigned int tptc2clkctrl;
	unsigned int tptc3clkctrl;
#if defined(CONFIG_TI814X)
	unsigned int resv6[4];
	unsigned int dcan01clkctrl;
	unsigned int mmchs0clkctrl;
	unsigned int mmchs1clkctrl;
	unsigned int mmchs2clkctrl;
	unsigned int custefuseclkctrl;
#elif defined(CONFIG_TI816X)
	unsigned int sr0clkctrl;
	unsigned int sr1clkctrl;
#endif
};

#endif /* _CLOCK_TI81XX_H_ */
