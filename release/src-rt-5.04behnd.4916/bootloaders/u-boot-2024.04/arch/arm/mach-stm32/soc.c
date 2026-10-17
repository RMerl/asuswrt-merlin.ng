// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#include <common.h>
#include <init.h>
#include <asm/io.h>
#include <asm/armv7_mpu.h>

int arch_cpu_init(void)
{
	int i;

	struct mpu_region_config stm32_region_config[] = {
		/*
		 * Make SDRAM area cacheable & executable.
		 */
#if defined(CONFIG_STM32F4)
		{ 0x00000000, REGION_0, XN_DIS, PRIV_RW_USR_RW,
		O_I_WB_RD_WR_ALLOC, REGION_512MB },
#endif

		{ 0x90000000, REGION_1, XN_DIS, PRIV_RW_USR_RW,
		SHARED_WRITE_BUFFERED, REGION_256MB },

#if defined(CONFIG_STM32F7) || defined(CONFIG_STM32H7)
		{ 0xC0000000, REGION_0, XN_DIS, PRIV_RW_USR_RW,
		O_I_WB_RD_WR_ALLOC, REGION_512MB },
#endif
	};

	disable_mpu();
	for (i = 0; i < ARRAY_SIZE(stm32_region_config); i++)
		mpu_config(&stm32_region_config[i]);
	enable_mpu();

	return 0;
}
