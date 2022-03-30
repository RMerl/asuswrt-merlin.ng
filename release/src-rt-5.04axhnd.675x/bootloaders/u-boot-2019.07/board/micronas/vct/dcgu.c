// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Original Author Guenter Gebhardt
 * Copyright (C) 2006 Micronas GmbH
 */

#include <common.h>
#include <linux/errno.h>

#include "vct.h"

int dcgu_set_clk_switch(enum dcgu_hw_module module, enum dcgu_switch setup)
{
	u32 enable;
	union dcgu_clk_en1 en1;
	union dcgu_clk_en2 en2;

	switch (setup) {
	case DCGU_SWITCH_ON:
		enable = 1;
		break;
	case DCGU_SWITCH_OFF:
		enable = 0;
		break;
	default:
		printf("%s:%i:Invalid clock switch: %i\n", __FILE__, __LINE__,
		       setup);
		return -EINVAL;
	}

	if (module == DCGU_HW_MODULE_CPU)
		en2.reg = reg_read(DCGU_CLK_EN2(DCGU_BASE));
	else
		en1.reg = reg_read(DCGU_CLK_EN1(DCGU_BASE));

	switch (module) {
	case DCGU_HW_MODULE_MSMC:
		en1.bits.en_clkmsmc = enable;
		break;
	case DCGU_HW_MODULE_SSI_S:
		en1.bits.en_clkssi_s = enable;
		break;
	case DCGU_HW_MODULE_SSI_M:
		en1.bits.en_clkssi_m = enable;
		break;
	case DCGU_HW_MODULE_SMC:
		en1.bits.en_clksmc = enable;
		break;
	case DCGU_HW_MODULE_EBI:
		en1.bits.en_clkebi = enable;
		break;
	case DCGU_HW_MODULE_USB_PLL:
		en1.bits.en_usbpll = enable;
		break;
	case DCGU_HW_MODULE_USB_60:
		en1.bits.en_clkusb60 = enable;
		break;
	case DCGU_HW_MODULE_USB_24:
		en1.bits.en_clkusb24 = enable;
		break;
	case DCGU_HW_MODULE_UART_2:
		en1.bits.en_clkuart2 = enable;
		break;
	case DCGU_HW_MODULE_UART_1:
		en1.bits.en_clkuart1 = enable;
		break;
	case DCGU_HW_MODULE_PERI:
		en1.bits.en_clkperi20 = enable;
		break;
	case DCGU_HW_MODULE_CPU:
		en2.bits.en_clkcpu = enable;
		break;
	case DCGU_HW_MODULE_I2S:
		en1.bits.en_clk_i2s_dly = enable;
		break;
	case DCGU_HW_MODULE_ABP_SCC:
		en1.bits.en_clk_scc_abp = enable;
		break;
	case DCGU_HW_MODULE_SPDIF:
		en1.bits.en_clk_dtv_spdo = enable;
		break;
	case DCGU_HW_MODULE_AD:
		en1.bits.en_clkad = enable;
		break;
	case DCGU_HW_MODULE_MVD:
		en1.bits.en_clkmvd = enable;
		break;
	case DCGU_HW_MODULE_TSD:
		en1.bits.en_clktsd = enable;
		break;
	case DCGU_HW_MODULE_GA:
		en1.bits.en_clkga = enable;
		break;
	case DCGU_HW_MODULE_DVP:
		en1.bits.en_clkdvp = enable;
		break;
	case DCGU_HW_MODULE_MR2:
		en1.bits.en_clkmr2 = enable;
		break;
	case DCGU_HW_MODULE_MR1:
		en1.bits.en_clkmr1 = enable;
		break;
	default:
		printf("%s:%i:Invalid hardware module: %i\n", __FILE__,
		       __LINE__, module);
		return -EINVAL;
	}

	/*
	 * The reg_read() following the reg_write() below forces the write to
	 * be really done on the bus.
	 * Otherwise the clock may not be switched on when this API function
	 * returns, which may cause an bus error if a registers of the hardware
	 * module connected to the clock is accessed.
	 */
	if (module == DCGU_HW_MODULE_CPU) {
		reg_write(DCGU_CLK_EN2(DCGU_BASE), en2.reg);
		en2.reg = reg_read(DCGU_CLK_EN2(DCGU_BASE));
	} else {
		reg_write(DCGU_CLK_EN1(DCGU_BASE), en1.reg);
		en1.reg = reg_read(DCGU_CLK_EN1(DCGU_BASE));
	}

	return 0;
}

int dcgu_set_reset_switch(enum dcgu_hw_module module, enum dcgu_switch setup)
{
	union dcgu_reset_unit1 val;
	u32 enable;

	switch (setup) {
	case DCGU_SWITCH_ON:
		enable = 1;
		break;
	case DCGU_SWITCH_OFF:
		enable = 0;
		break;
	default:
		printf("%s:%i:Invalid reset switch: %i\n", __FILE__, __LINE__,
		       setup);
		return -EINVAL;
	}

	val.reg = reg_read(DCGU_RESET_UNIT1(DCGU_BASE));
	switch (module) {
	case DCGU_HW_MODULE_MSMC:
		val.bits.swreset_clkmsmc = enable;
		break;
	case DCGU_HW_MODULE_SSI_S:
		val.bits.swreset_clkssi_s = enable;
		break;
	case DCGU_HW_MODULE_SSI_M:
		val.bits.swreset_clkssi_m = enable;
		break;
	case DCGU_HW_MODULE_SMC:
		val.bits.swreset_clksmc = enable;
		break;
	case DCGU_HW_MODULE_EBI:
		val.bits.swreset_clkebi = enable;
		break;
	case DCGU_HW_MODULE_USB_60:
		val.bits.swreset_clkusb60 = enable;
		break;
	case DCGU_HW_MODULE_USB_24:
		val.bits.swreset_clkusb24 = enable;
		break;
	case DCGU_HW_MODULE_UART_2:
		val.bits.swreset_clkuart2 = enable;
		break;
	case DCGU_HW_MODULE_UART_1:
		val.bits.swreset_clkuart1 = enable;
		break;
	case DCGU_HW_MODULE_PWM:
		val.bits.swreset_pwm = enable;
		break;
	case DCGU_HW_MODULE_GPT:
		val.bits.swreset_gpt = enable;
		break;
	case DCGU_HW_MODULE_I2C2:
		val.bits.swreset_i2c2 = enable;
		break;
	case DCGU_HW_MODULE_I2C1:
		val.bits.swreset_i2c1 = enable;
		break;
	case DCGU_HW_MODULE_GPIO2:
		val.bits.swreset_gpio2 = enable;
		break;
	case DCGU_HW_MODULE_GPIO1:
		val.bits.swreset_gpio1 = enable;
		break;
	case DCGU_HW_MODULE_CPU:
		val.bits.swreset_clkcpu = enable;
		break;
	case DCGU_HW_MODULE_I2S:
		val.bits.swreset_clk_i2s_dly = enable;
		break;
	case DCGU_HW_MODULE_ABP_SCC:
		val.bits.swreset_clk_scc_abp = enable;
		break;
	case DCGU_HW_MODULE_SPDIF:
		val.bits.swreset_clk_dtv_spdo = enable;
		break;
	case DCGU_HW_MODULE_AD:
		val.bits.swreset_clkad = enable;
		break;
	case DCGU_HW_MODULE_MVD:
		val.bits.swreset_clkmvd = enable;
		break;
	case DCGU_HW_MODULE_TSD:
		val.bits.swreset_clktsd = enable;
		break;
	case DCGU_HW_MODULE_TSIO:
		val.bits.swreset_clktsio = enable;
		break;
	case DCGU_HW_MODULE_GA:
		val.bits.swreset_clkga = enable;
		break;
	case DCGU_HW_MODULE_MPC:
		val.bits.swreset_clkmpc = enable;
		break;
	case DCGU_HW_MODULE_CVE:
		val.bits.swreset_clkcve = enable;
		break;
	case DCGU_HW_MODULE_DVP:
		val.bits.swreset_clkdvp = enable;
		break;
	case DCGU_HW_MODULE_MR2:
		val.bits.swreset_clkmr2 = enable;
		break;
	case DCGU_HW_MODULE_MR1:
		val.bits.swreset_clkmr1 = enable;
		break;
	default:
		printf("%s:%i:Invalid hardware module: %i\n", __FILE__,
		       __LINE__, module);
		return -EINVAL;
	}
	reg_write(DCGU_RESET_UNIT1(DCGU_BASE), val.reg);

	return 0;
}
