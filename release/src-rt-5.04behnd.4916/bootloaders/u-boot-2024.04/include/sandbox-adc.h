/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#ifndef _SANDBOX_ADC_H_
#define _SANDBOX_ADC_H_

#define SANDBOX_ADC_DEVNAME		"adc@0"
#define SANDBOX_ADC_DATA_MASK		0xffff /* 16-bits resolution */
#define SANDBOX_ADC_CHANNELS		4
#define SANDBOX_ADC_CHANNEL0_DATA	0x0
#define SANDBOX_ADC_CHANNEL1_DATA	0x1000
#define SANDBOX_ADC_CHANNEL2_DATA	0x2000
#define SANDBOX_ADC_CHANNEL3_DATA	0x3000

enum sandbox_adc_mode {
	SANDBOX_ADC_MODE_SINGLE_CHANNEL = 0,
	SANDBOX_ADC_MODE_MULTI_CHANNEL,
};

enum sandbox_adc_status {
	SANDBOX_ADC_INACTIVE = 0,
	SANDBOX_ADC_ACTIVE,
};

#define SANDBOX_ADC_VSS_VALUE		0

#endif
