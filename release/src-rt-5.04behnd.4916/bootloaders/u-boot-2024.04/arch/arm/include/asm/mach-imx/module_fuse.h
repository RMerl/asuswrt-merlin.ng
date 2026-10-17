/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#ifndef __MODULE_FUSE_H__
#define __MODULE_FUSE_H__

enum fuse_module_type {
	MODULE_TSC,
	MODULE_ADC1,
	MODULE_ADC2,
	MODULE_SIM1,
	MODULE_SIM2,
	MODULE_FLEXCAN1,
	MODULE_FLEXCAN2,
	MODULE_SPDIF,
	MODULE_EIM,
	MODULE_SD1,
	MODULE_SD2,
	MODULE_SD3,
	MODULE_SD4,
	MODULE_QSPI1,
	MODULE_QSPI2,
	MODULE_GPMI,
	MODULE_APBHDMA,
	MODULE_LCDIF,
	MODULE_PXP,
	MODULE_CSI,
	MODULE_ENET1,
	MODULE_ENET2,
	MODULE_CAAM,
	MODULE_USB_OTG1,
	MODULE_USB_OTG2,
	MODULE_SAI2,
	MODULE_SAI3,
	MODULE_BEE,
	MODULE_UART1,
	MODULE_UART2,
	MODULE_UART3,
	MODULE_UART4,
	MODULE_UART5,
	MODULE_UART6,
	MODULE_UART7,
	MODULE_UART8,
	MODULE_PWM5,
	MODULE_PWM6,
	MODULE_PWM7,
	MODULE_PWM8,
	MODULE_ECSPI1,
	MODULE_ECSPI2,
	MODULE_ECSPI3,
	MODULE_ECSPI4,
	MODULE_ECSPI5,
	MODULE_I2C1,
	MODULE_I2C2,
	MODULE_I2C3,
	MODULE_I2C4,
	MODULE_GPT1,
	MODULE_GPT2,
	MODULE_EPIT1,
	MODULE_EPIT2,
	MODULE_EPDC,
	MODULE_ESAI,
	MODULE_DCP,
	MODULE_DCP_CRYPTO,
};

struct fuse_entry_desc {
	enum fuse_module_type module;
	const char *node_path;
	u32 fuse_word_offset;
	u32 fuse_bit_offset;
	u32 status;
};

#if !IS_ENABLED(CONFIG_IMX_MODULE_FUSE)
static inline u32 check_module_fused(enum fuse_module_type module)
{
	return 0;
};

static inline u32 esdhc_fused(ulong base_addr)
{
	return 0;
};

static inline u32 ecspi_fused(ulong base_addr)
{
	return 0;
};

static inline u32 uart_fused(ulong base_addr)
{
	return 0;
};

static inline u32 usb_fused(ulong base_addr)
{
	return 0;
};

static inline u32 qspi_fused(ulong base_addr)
{
	return 0;
};

static inline u32 i2c_fused(ulong base_addr)
{
	return 0;
};

static inline u32 enet_fused(ulong base_addr)
{
	return 0;
};
#else
u32 check_module_fused(enum fuse_module_type module);
u32 esdhc_fused(ulong base_addr);
u32 ecspi_fused(ulong base_addr);
u32 uart_fused(ulong base_addr);
u32 usb_fused(ulong base_addr);
u32 qspi_fused(ulong base_addr);
u32 i2c_fused(ulong base_addr);
u32 enet_fused(ulong base_addr);
#endif
#endif /* __MODULE_FUSE_H__ */
