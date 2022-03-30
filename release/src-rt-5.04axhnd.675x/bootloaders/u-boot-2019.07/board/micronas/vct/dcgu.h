/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
 */

#ifndef _DCGU_H
#define _DCGU_H

enum dcgu_switch {
	DCGU_SWITCH_OFF,	/* Switch off				*/
	DCGU_SWITCH_ON		/* Switch on				*/
};

enum dcgu_hw_module {
	DCGU_HW_MODULE_DCGU,	/* Selects digital clock gen. unit	*/

	DCGU_HW_MODULE_MIC32_SCI, /* Selects MIC32 SoC interface	*/
	DCGU_HW_MODULE_SCI,	/* Selects SCI target agent port modules*/

	DCGU_HW_MODULE_MR1,	/* Selects first MPEG reader module	*/
	DCGU_HW_MODULE_MR2,	/* Selects second MPEG reader module	*/
	DCGU_HW_MODULE_MVD,	/* Selects MPEG video decoder module	*/
	DCGU_HW_MODULE_DVP,	/* Selects dig video processing module	*/
	DCGU_HW_MODULE_CVE,	/* Selects color video encoder module	*/
	DCGU_HW_MODULE_VID_ENC,	/* Selects video encoder module		*/

	DCGU_HW_MODULE_SSI_S,	/* Selects slave sync serial interface	*/
	DCGU_HW_MODULE_SSI_M,	/* Selects master sync serial interface	*/

	DCGU_HW_MODULE_GA,	/* Selects graphics accelerator module	*/
	DCGU_HW_MODULE_DGPU,	/* Selects digital graphics processing	*/

	DCGU_HW_MODULE_UART_1,	/* Selects first UART module		*/
	DCGU_HW_MODULE_UART_2,	/* Selects second UART module		*/

	DCGU_HW_MODULE_AD,	/* Selects audio decoder module		*/
	DCGU_HW_MODULE_ABP_DTV,	/* Selects audio baseband processing	*/
	DCGU_HW_MODULE_ABP_SCC,	/* Selects audio base band processor SCC*/
	DCGU_HW_MODULE_SPDIF,	/* Selects sony philips digital interf.	*/

	DCGU_HW_MODULE_TSIO,	/* Selects trasnport stream input/output*/
	DCGU_HW_MODULE_TSD,	/* Selects trasnport stream decoder	*/
	DCGU_HW_MODULE_TSD_KEY,	/* Selects trasnport stream decoder key	*/

	DCGU_HW_MODULE_USBH,	/* Selects USB hub module		*/
	DCGU_HW_MODULE_USB_PLL,	/* Selects USB phase locked loop module	*/
	DCGU_HW_MODULE_USB_60,	/* Selects USB 60 module		*/
	DCGU_HW_MODULE_USB_24,	/* Selects USB 24 module		*/

	DCGU_HW_MODULE_PERI,	/* Selects all mod connected to clkperi20*/
	DCGU_HW_MODULE_WDT,	/* Selects wtg timer mod con to clkperi20*/
	DCGU_HW_MODULE_I2C1,	/* Selects first I2C mod con to clkperi20*/
	DCGU_HW_MODULE_I2C2,	/* Selects 2nd I2C mod con to clkperi20	*/
	DCGU_HW_MODULE_GPIO1,	/* Selects gpio module 1		*/
	DCGU_HW_MODULE_GPIO2,	/* Selects gpio module 2		*/

	DCGU_HW_MODULE_GPT,	/* Selects gpt mod connected to clkperi20*/
	DCGU_HW_MODULE_PWM,	/* Selects pwm mod connected to clkperi20*/

	DCGU_HW_MODULE_MPC,	/* Selects multi purpose cipher module	*/
	DCGU_HW_MODULE_MPC_KEY,	/* Selects multi purpose cipher key	*/

	DCGU_HW_MODULE_COM,	/* Selects COM unit module		*/
	DCGU_HW_MODULE_VCTY_CORE, /* Selects VCT-Y core module		*/
	DCGU_HW_MODULE_FWSRAM,	/* Selects firmware SRAM module		*/

	DCGU_HW_MODULE_EBI,	/* Selects external bus interface module*/
	DCGU_HW_MODULE_I2S,	/* Selects integrated interchip sound	*/
	DCGU_HW_MODULE_MSMC,	/* Selects memory stick and mmc module	*/
	DCGU_HW_MODULE_SMC,	/* Selects smartcard interface module	*/

	DCGU_HW_MODULE_IRQC,	/* Selects interrupt C module		*/
	DCGU_HW_MODULE_TOP,	/* Selects top level pinmux module	*/
	DCGU_HW_MODULE_SRAM,	/* Selects SRAM module			*/
	DCGU_HW_MODULE_EIC,	/* Selects External Interrupt controller*/
	DCGU_HW_MODULE_CPU,	/* Selects CPU subsystem module		*/
	DCGU_HW_MODULE_SCC,	/* Selects SCC module			*/
	DCGU_HW_MODULE_MM,	/* Selects Memory Manager module	*/
	DCGU_HW_MODULE_BCU,	/* Selects Buffer Configuration Unit	*/
	DCGU_HW_MODULE_FH,	/* Selects FIFO Handler module		*/
	DCGU_HW_MODULE_IMU,	/* Selects Interrupt Management Unit	*/
	DCGU_HW_MODULE_MDU,	/* Selects MCI Debug Unit module	*/
	DCGU_HW_MODULE_SI2OCP	/* Selects Standard Interface to OCP bridge*/
};

union dcgu_clk_en1 {
	u32 reg;
	struct {
		u32 res1:8;		/* reserved			*/
		u32 en_clkmsmc:1;	/* Enable bit for clkmsmc (#)	*/
		u32 en_clkssi_s:1;	/* Enable bit for clkssi_s (#)	*/
		u32 en_clkssi_m:1;	/* Enable bit for clkssi_m (#)	*/
		u32 en_clksmc:1;	/* Enable bit for clksmc (#)	*/
		u32 en_clkebi:1;	/* Enable bit for clkebi (#)	*/
		u32 en_usbpll:1;	/* Enable bit for the USB PLL	*/
		u32 en_clkusb60:1;	/* Enable bit for clkusb60 (#)	*/
		u32 en_clkusb24:1;	/* Enable bit for clkusb24 (#)	*/
		u32 en_clkuart2:1;	/* Enable bit for clkuart2 (#)	*/
		u32 en_clkuart1:1;	/* Enable bit for clkuart1 (#)	*/
		u32 en_clkperi20:1;	/* Enable bit for clkperi20 (#)	*/
		u32 res2:3;		/* reserved			*/
		u32 en_clk_i2s_dly:1;	/* Enable bit for clk_scc_abp	*/
		u32 en_clk_scc_abp:1;	/* Enable bit for clk_scc_abp	*/
		u32 en_clk_dtv_spdo:1;	/* Enable bit for clk_dtv_spdo	*/
		u32 en_clkad:1;		/* Enable bit for clkad (#)	*/
		u32 en_clkmvd:1;	/* Enable bit for clkmvd (#)	*/
		u32 en_clktsd:1;	/* Enable bit for clktsd (#)	*/
		u32 en_clkga:1;		/* Enable bit for clkga (#)	*/
		u32 en_clkdvp:1;	/* Enable bit for clkdvp (#)	*/
		u32 en_clkmr2:1;	/* Enable bit for clkmr2 (#)	*/
		u32 en_clkmr1:1;	/* Enable bit for clkmr1 (#)	*/
	} bits;
};

union dcgu_clk_en2 {
	u32 reg;
	struct {
		u32 res1:31;		/* reserved			*/
		u32 en_clkcpu:1;	/* Enable bit for clkcpu	*/
	} bits;
};

union dcgu_reset_unit1 {
	u32 reg;
	struct {
		u32 res1:1;
		u32 swreset_clkmsmc:1;
		u32 swreset_clkssi_s:1;
		u32 swreset_clkssi_m:1;
		u32 swreset_clksmc:1;
		u32 swreset_clkebi:1;
		u32 swreset_clkusb60:1;
		u32 swreset_clkusb24:1;
		u32 swreset_clkuart2:1;
		u32 swreset_clkuart1:1;
		u32 swreset_pwm:1;
		u32 swreset_gpt:1;
		u32 swreset_i2c2:1;
		u32 swreset_i2c1:1;
		u32 swreset_gpio2:1;
		u32 swreset_gpio1:1;
		u32 swreset_clkcpu:1;
		u32 res2:2;
		u32 swreset_clk_i2s_dly:1;
		u32 swreset_clk_scc_abp:1;
		u32 swreset_clk_dtv_spdo:1;
		u32 swreset_clkad:1;
		u32 swreset_clkmvd:1;
		u32 swreset_clktsd:1;
		u32 swreset_clktsio:1;
		u32 swreset_clkga:1;
		u32 swreset_clkmpc:1;
		u32 swreset_clkcve:1;
		u32 swreset_clkdvp:1;
		u32 swreset_clkmr2:1;
		u32 swreset_clkmr1:1;
	} bits;
};

int dcgu_set_clk_switch(enum dcgu_hw_module module, enum dcgu_switch setup);
int dcgu_set_reset_switch(enum dcgu_hw_module module, enum dcgu_switch setup);

#endif /* _DCGU_H */
