/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2010 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  MyungJoo Ham <myungjoo.ham@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_ADC_H_
#define __ASM_ARM_ARCH_ADC_H_

#define ADC_V2_CON1_SOFT_RESET		(0x2 << 1)
#define ADC_V2_CON1_STC_EN		0x1

#define ADC_V2_CON2_OSEL(x)		(((x) & 0x1) << 10)
#define OSEL_2S				0x0
#define OSEL_BINARY			0x1
#define ADC_V2_CON2_ESEL(x)		(((x) & 0x1) << 9)
#define ESEL_ADC_EVAL_TIME_40CLK	0x0
#define ESEL_ADC_EVAL_TIME_20CLK	0x1
#define ADC_V2_CON2_HIGHF(x)		(((x) & 0x1) << 8)
#define HIGHF_CONV_RATE_30KSPS		0x0
#define HIGHF_CONV_RATE_600KSPS		0x1
#define ADC_V2_CON2_C_TIME(x)		(((x) & 0x7) << 4)
#define ADC_V2_CON2_CHAN_SEL_MASK	0xf
#define ADC_V2_CON2_CHAN_SEL(x)		((x) & ADC_V2_CON2_CHAN_SEL_MASK)

#define ADC_V2_GET_STATUS_FLAG(x)	(((x) >> 2) & 0x1)
#define FLAG_CONV_END			0x1

#define ADC_V2_INT_DISABLE		0x0
#define ADC_V2_INT_ENABLE		0x1
#define INT_NOT_GENERATED		0x0
#define INT_GENERATED			0x1

#define ADC_V2_VERSION			0x80000008

#define ADC_V2_MAX_CHANNEL		9

/* For default 8 time convertion with sample rate 600 kSPS - 15us timeout */
#define ADC_V2_CONV_TIMEOUT_US		15

#define ADC_V2_DAT_MASK			0xfff

#ifndef __ASSEMBLY__
struct s5p_adc {
	unsigned int adccon;
	unsigned int adctsc;
	unsigned int adcdly;
	unsigned int adcdat0;
	unsigned int adcdat1;
	unsigned int adcupdn;
	unsigned int adcclrint;
	unsigned int adcmux;
	unsigned int adcclrintpndnup;
};

struct exynos_adc_v2 {
	unsigned int con1;
	unsigned int con2;
	unsigned int status;
	unsigned int dat;
	unsigned int int_en;
	unsigned int int_status;
	unsigned int reserved[2];
	unsigned int version;
};
#endif

#endif /* __ASM_ARM_ARCH_ADC_H_ */
