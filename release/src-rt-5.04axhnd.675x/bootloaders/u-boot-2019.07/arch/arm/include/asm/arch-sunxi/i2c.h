/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 - Hans de Goede <hdegoede@redhat.com>
 */
#ifndef _SUNXI_I2C_H_
#define _SUNXI_I2C_H_

#include <asm/arch/cpu.h>

#ifdef CONFIG_I2C0_ENABLE
#define CONFIG_I2C_MVTWSI_BASE0	SUNXI_TWI0_BASE
#endif
#ifdef CONFIG_I2C1_ENABLE
#define CONFIG_I2C_MVTWSI_BASE1	SUNXI_TWI1_BASE
#endif
#ifdef CONFIG_I2C2_ENABLE
#define CONFIG_I2C_MVTWSI_BASE2	SUNXI_TWI2_BASE
#endif
#ifdef CONFIG_I2C3_ENABLE
#define CONFIG_I2C_MVTWSI_BASE3	SUNXI_TWI3_BASE
#endif
#ifdef CONFIG_I2C4_ENABLE
#define CONFIG_I2C_MVTWSI_BASE4	SUNXI_TWI4_BASE
#endif
#ifdef CONFIG_R_I2C_ENABLE
#define CONFIG_I2C_MVTWSI_BASE5 SUNXI_R_TWI_BASE
#endif

/* This is abp0-clk on sun4i/5i/7i / abp1-clk on sun6i/sun8i which is 24MHz */
#define CONFIG_SYS_TCLK		24000000

#endif
