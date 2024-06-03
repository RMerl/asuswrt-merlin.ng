/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#ifndef __STM32MP1_SMC_H__
#define __STM32MP1_SMC_H__

#include <linux/arm-smccc.h>

/*
 * SMC function IDs for STM32 Service queries
 * STM32 SMC services use the space between 0x82000000 and 0x8200FFFF
 * like this is defined in SMC calling Convention by ARM
 * for SiP (silicon Partner)
 * http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html
 */
#define STM32_SMC_VERSION		0x82000000

/* Secure Service access from Non-secure */
#define STM32_SMC_BSEC			0x82001003

/* Service for BSEC */
#define STM32_SMC_READ_SHADOW		0x01
#define STM32_SMC_PROG_OTP		0x02
#define STM32_SMC_WRITE_SHADOW		0x03
#define STM32_SMC_READ_OTP		0x04
#define STM32_SMC_READ_ALL		0x05
#define STM32_SMC_WRITE_ALL		0x06

/* SMC error codes */
#define STM32_SMC_OK			0x0
#define STM32_SMC_NOT_SUPPORTED		-1
#define STM32_SMC_FAILED		-2
#define STM32_SMC_INVALID_PARAMS	-3

#define stm32_smc_exec(svc, op, data1, data2) \
	stm32_smc(svc, op, data1, data2, NULL)

#ifdef CONFIG_ARM_SMCCC
static inline u32 stm32_smc(u32 svc, u8 op, u32 data1, u32 data2, u32 *result)
{
	struct arm_smccc_res res;

	arm_smccc_smc(svc, op, data1, data2, 0, 0, 0, 0, &res);

	if (res.a0) {
		pr_err("%s: Failed to exec in secure mode (err = %ld)\n",
		       __func__, res.a0);
		return -EINVAL;
	}
	if (result)
		*result = (u32)res.a1;

	return 0;
}
#else
static inline u32 stm32_smc(u32 svc, u8 op, u32 data1, u32 data2, u32 *result)
{
	return 0;
}
#endif

#endif /* __STM32MP1_SMC_H__ */
