/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Author: Fabrice Gasnier <fabrice.gasnier@st.com>.
 *
 * Originally based on the Linux kernel v4.18 drivers/iio/adc/stm32-adc-core.h.
 */

#ifndef __STM32_ADC_H
#define __STM32_ADC_H

/*
 * STM32 - ADC global register map
 * ________________________________________________________
 * | Offset |                 Register                    |
 * --------------------------------------------------------
 * | 0x000  |                Master ADC1                  |
 * --------------------------------------------------------
 * | 0x100  |                Slave ADC2                   |
 * --------------------------------------------------------
 * | 0x200  |                Slave ADC3                   |
 * --------------------------------------------------------
 * | 0x300  |         Master & Slave common regs          |
 * --------------------------------------------------------
 */
#define STM32_ADC_MAX_ADCS		3
#define STM32_ADCX_COMN_OFFSET		0x300

#include <clk.h>

struct udevice;

/**
 * struct stm32_adc_common - stm32 ADC driver common data (for all instances)
 * @base:		control registers base cpu addr
 * @rate:		clock rate used for analog circuitry
 * @aclk:		clock for the analog circuitry
 * @bclk:		bus clock common for all ADCs
 * @vref:		regulator reference
 * @vref_uv:		reference supply voltage (uV)
 */
struct stm32_adc_common {
	void __iomem *base;
	unsigned long rate;
	struct clk aclk;
	struct clk bclk;
	struct udevice *vref;
	int vref_uv;
};

#endif
