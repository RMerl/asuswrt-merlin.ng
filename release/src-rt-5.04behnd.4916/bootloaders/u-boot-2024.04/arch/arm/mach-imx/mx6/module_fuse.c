// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/module_fuse.h>
#include <linux/errno.h>

static struct fuse_entry_desc mx6_fuse_descs[] = {
#if defined(CONFIG_MX6ULL)
	{MODULE_TSC, "/soc/aips-bus@2000000/tsc@2040000", 0x430, 22},
	{MODULE_ADC2, "/soc/aips-bus@2100000/adc@219c000", 0x430, 23},
	{MODULE_EPDC, "/soc/aips-bus@2200000/epdc@228c000", 0x430, 24},
	{MODULE_ESAI, "/soc/aips-bus@2000000/spba-bus@2000000/esai@2024000", 0x430, 25},
	{MODULE_FLEXCAN1, "/soc/aips-bus@2000000/can@2090000", 0x430, 26},
	{MODULE_FLEXCAN2, "/soc/aips-bus@2000000/can@2094000", 0x430, 27},
	{MODULE_SPDIF, "/soc/aips-bus@2000000/spba-bus@2000000/spdif@2004000", 0x440, 2},
	{MODULE_EIM, "/soc/aips-bus@2100000/weim@21b8000", 0x440, 3},
	{MODULE_SD1, "/soc/aips-bus@2100000/usdhc@2190000", 0x440, 4},
	{MODULE_SD2, "/soc/aips-bus@2100000/usdhc@2194000", 0x440, 5},
	{MODULE_QSPI1, "/soc/aips-bus@2100000/qspi@21e0000", 0x440, 6},
	{MODULE_GPMI, "/soc/gpmi-nand@1806000", 0x440, 7},
	{MODULE_APBHDMA, "/soc/dma-apbh@1804000", 0x440, 7},
	{MODULE_LCDIF, "/soc/aips-bus@2100000/lcdif@21c8000", 0x440, 8},
	{MODULE_PXP, "/soc/aips-bus@2100000/pxp@21cc000", 0x440, 9},
	{MODULE_CSI, "/soc/aips-bus@2100000/csi@21c4000", 0x440, 10},
	{MODULE_ADC1, "/soc/aips-bus@2100000/adc@2198000", 0x440, 11},
	{MODULE_ENET1, "/soc/aips-bus@2100000/ethernet@2188000", 0x440, 12},
	{MODULE_ENET2, "/soc/aips-bus@2000000/ethernet@20b4000", 0x440, 13},
	{MODULE_DCP, "/soc/aips-bus@2200000/dcp@2280000", 0x440, 14},
	{MODULE_USB_OTG2, "/soc/aips-bus@2100000/usb@2184200", 0x440, 15},
	{MODULE_SAI2, "/soc/aips-bus@2000000/spba-bus@2000000/sai@202c000", 0x440, 24},
	{MODULE_SAI3, "/soc/aips-bus@2000000/spba-bus@2000000/sai@2030000", 0x440, 24},
	{MODULE_DCP_CRYPTO, "/soc/aips-bus@2200000/dcp@2280000", 0x440, 25},
	{MODULE_UART5, "/soc/aips-bus@2100000/serial@21f4000", 0x440, 26},
	{MODULE_UART6, "/soc/aips-bus@2100000/serial@21fc000", 0x440, 26},
	{MODULE_UART7, "/soc/aips-bus@2000000/spba-bus@2000000/serial@2018000", 0x440, 26},
	{MODULE_UART8, "/soc/aips-bus@2200000/serial@2288000", 0x440, 26},
	{MODULE_PWM5, "/soc/aips-bus@2000000/pwm@20f0000", 0x440, 27},
	{MODULE_PWM6, "/soc/aips-bus@2000000/pwm@20f4000", 0x440, 27},
	{MODULE_PWM7, "/soc/aips-bus@2000000/pwm@20f8000", 0x440, 27},
	{MODULE_PWM8, "/soc/aips-bus@2000000/pwm@20fc000", 0x440, 27},
	{MODULE_ECSPI3, "/soc/aips-bus@2000000/spba-bus@2000000/ecspi@2010000", 0x440, 28},
	{MODULE_ECSPI4, "/soc/aips-bus@2000000/spba-bus@2000000/ecspi@2014000", 0x440, 28},
	{MODULE_I2C3, "/soc/aips-bus@2100000/i2c@21a8000", 0x440, 29},
	{MODULE_I2C4, "/soc/aips-bus@2100000/i2c@21f8000", 0x440, 29},
	{MODULE_GPT2, "/soc/aips-bus@2000000/gpt@20e8000", 0x440, 30},
	{MODULE_EPIT2, "/soc/aips-bus@2000000/epit@20d4000", 0x440, 31},
	/* Paths for older imx tree: */
	{MODULE_TSC, "/soc/aips-bus@02000000/tsc@02040000", 0x430, 22},
	{MODULE_ADC2, "/soc/aips-bus@02100000/adc@0219c000", 0x430, 23},
	{MODULE_EPDC, "/soc/aips-bus@02200000/epdc@0228c000", 0x430, 24},
	{MODULE_ESAI, "/soc/aips-bus@02000000/spba-bus@02000000/esai@02024000", 0x430, 25},
	{MODULE_FLEXCAN1, "/soc/aips-bus@02000000/can@02090000", 0x430, 26},
	{MODULE_FLEXCAN2, "/soc/aips-bus@02000000/can@02094000", 0x430, 27},
	{MODULE_SPDIF, "/soc/aips-bus@02000000/spba-bus@02000000/spdif@02004000", 0x440, 2},
	{MODULE_EIM, "/soc/aips-bus@02100000/weim@021b8000", 0x440, 3},
	{MODULE_SD1, "/soc/aips-bus@02100000/usdhc@02190000", 0x440, 4},
	{MODULE_SD2, "/soc/aips-bus@02100000/usdhc@02194000", 0x440, 5},
	{MODULE_QSPI1, "/soc/aips-bus@02100000/qspi@021e0000", 0x440, 6},
	{MODULE_GPMI, "/soc/gpmi-nand@01806000", 0x440, 7},
	{MODULE_APBHDMA, "/soc/dma-apbh@01804000", 0x440, 7},
	{MODULE_LCDIF, "/soc/aips-bus@02100000/lcdif@021c8000", 0x440, 8},
	{MODULE_PXP, "/soc/aips-bus@02100000/pxp@021cc000", 0x440, 9},
	{MODULE_CSI, "/soc/aips-bus@02100000/csi@021c4000", 0x440, 10},
	{MODULE_ADC1, "/soc/aips-bus@02100000/adc@02198000", 0x440, 11},
	{MODULE_ENET1, "/soc/aips-bus@02100000/ethernet@02188000", 0x440, 12},
	{MODULE_ENET2, "/soc/aips-bus@02000000/ethernet@020b4000", 0x440, 13},
	{MODULE_DCP, "/soc/aips-bus@02200000/dcp@02280000", 0x440, 14},
	{MODULE_USB_OTG2, "/soc/aips-bus@02100000/usb@02184200", 0x440, 15},
	{MODULE_SAI2, "/soc/aips-bus@02000000/spba-bus@02000000/sai@0202c000", 0x440, 24},
	{MODULE_SAI3, "/soc/aips-bus@02000000/spba-bus@02000000/sai@02030000", 0x440, 24},
	{MODULE_DCP_CRYPTO, "/soc/aips-bus@02200000/dcp@02280000", 0x440, 25},
	{MODULE_UART5, "/soc/aips-bus@02100000/serial@021f4000", 0x440, 26},
	{MODULE_UART6, "/soc/aips-bus@02100000/serial@021fc000", 0x440, 26},
	{MODULE_UART7, "/soc/aips-bus@02000000/spba-bus@02000000/serial@02018000", 0x440, 26},
	{MODULE_UART8, "/soc/aips-bus@02200000/serial@02288000", 0x440, 26},
	{MODULE_PWM5, "/soc/aips-bus@02000000/pwm@020f0000", 0x440, 27},
	{MODULE_PWM6, "/soc/aips-bus@02000000/pwm@020f4000", 0x440, 27},
	{MODULE_PWM7, "/soc/aips-bus@02000000/pwm@020f8000", 0x440, 27},
	{MODULE_PWM8, "/soc/aips-bus@02000000/pwm@020fc000", 0x440, 27},
	{MODULE_ECSPI3, "/soc/aips-bus@02000000/spba-bus@02000000/ecspi@02010000", 0x440, 28},
	{MODULE_ECSPI4, "/soc/aips-bus@02000000/spba-bus@02000000/ecspi@02014000", 0x440, 28},
	{MODULE_I2C3, "/soc/aips-bus@02100000/i2c@021a8000", 0x440, 29},
	{MODULE_I2C4, "/soc/aips-bus@02100000/i2c@021f8000", 0x440, 29},
	{MODULE_GPT2, "/soc/aips-bus@02000000/gpt@020e8000", 0x440, 30},
	{MODULE_EPIT2, "/soc/aips-bus@02000000/epit@020d4000", 0x440, 31},
#elif defined(CONFIG_MX6UL)
	{MODULE_TSC, "/soc/aips-bus@2000000/tsc@2040000", 0x430, 22},
	{MODULE_ADC2, "/soc/aips-bus@2100000/adc@219c000", 0x430, 23},
	{MODULE_SIM1, "/soc/aips-bus@2100000/sim@218c000", 0x430, 24},
	{MODULE_SIM2, "/soc/aips-bus@2100000/sim@21b4000", 0x430, 25},
	{MODULE_FLEXCAN1, "/soc/aips-bus@2000000/can@2090000", 0x430, 26},
	{MODULE_FLEXCAN2, "/soc/aips-bus@2000000/can@2094000", 0x430, 27},
	{MODULE_SPDIF, "/soc/aips-bus@2000000/spba-bus@2000000/spdif@2004000", 0x440, 2},
	{MODULE_EIM, "/soc/aips-bus@2100000/weim@21b8000", 0x440, 3},
	{MODULE_SD1, "/soc/aips-bus@2100000/usdhc@2190000", 0x440, 4},
	{MODULE_SD2, "/soc/aips-bus@2100000/usdhc@2194000", 0x440, 5},
	{MODULE_QSPI1, "/soc/aips-bus@2100000/qspi@21e0000", 0x440, 6},
	{MODULE_GPMI, "/soc/gpmi-nand@1806000", 0x440, 7},
	{MODULE_APBHDMA, "/soc/dma-apbh@1804000", 0x440, 7},
	{MODULE_LCDIF, "/soc/aips-bus@2100000/lcdif@21c8000", 0x440, 8},
	{MODULE_PXP, "/soc/aips-bus@2100000/pxp@21cc000", 0x440, 9},
	{MODULE_CSI, "/soc/aips-bus@2100000/csi@21c4000", 0x440, 10},
	{MODULE_ADC1, "/soc/aips-bus@2100000/adc@2198000", 0x440, 11},
	{MODULE_ENET1, "/soc/aips-bus@2100000/ethernet@2188000", 0x440, 12},
	{MODULE_ENET2, "/soc/aips-bus@2000000/ethernet@20b4000", 0x440, 13},
	{MODULE_CAAM, "/soc/aips-bus@2100000/caam@2140000", 0x440, 14},
	{MODULE_USB_OTG2, "/soc/aips-bus@2100000/usb@2184200", 0x440, 15},
	{MODULE_SAI2, "/soc/aips-bus@2000000/spba-bus@2000000/sai@202c000", 0x440, 24},
	{MODULE_SAI3, "/soc/aips-bus@2000000/spba-bus@2000000/sai@2030000", 0x440, 24},
	{MODULE_BEE, "/soc/aips-bus@2000000/bee@2044000", 0x440, 25},
	{MODULE_UART5, "/soc/aips-bus@2100000/serial@21f4000", 0x440, 26},
	{MODULE_UART6, "/soc/aips-bus@2100000/serial@21fc000", 0x440, 26},
	{MODULE_UART7, "/soc/aips-bus@2000000/spba-bus@2000000/serial@2018000", 0x440, 26},
	{MODULE_UART8, "/soc/aips-bus@2000000/spba-bus@2000000/serial@2024000", 0x440, 26},
	{MODULE_PWM5, "/soc/aips-bus@2000000/pwm@20f0000", 0x440, 27},
	{MODULE_PWM6, "/soc/aips-bus@2000000/pwm@20f4000", 0x440, 27},
	{MODULE_PWM7, "/soc/aips-bus@2000000/pwm@20f8000", 0x440, 27},
	{MODULE_PWM8, "/soc/aips-bus@2000000/pwm@20fc000", 0x440, 27},
	{MODULE_ECSPI3, "/soc/aips-bus@2000000/spba-bus@2000000/ecspi@2010000", 0x440, 28},
	{MODULE_ECSPI4, "/soc/aips-bus@2000000/spba-bus@2000000/ecspi@2014000", 0x440, 28},
	{MODULE_I2C3, "/soc/aips-bus@2100000/i2c@21a8000", 0x440, 29},
	{MODULE_I2C4, "/soc/aips-bus@2100000/i2c@21f8000", 0x440, 29},
	{MODULE_GPT2, "/soc/aips-bus@2000000/gpt@20e8000", 0x440, 30},
	{MODULE_EPIT2, "/soc/aips-bus@2000000/epit@20d4000", 0x440, 31},
	/* Paths for older imx tree: */
	{MODULE_TSC, "/soc/aips-bus@02000000/tsc@02040000", 0x430, 22},
	{MODULE_ADC2, "/soc/aips-bus@02100000/adc@0219c000", 0x430, 23},
	{MODULE_SIM1, "/soc/aips-bus@02100000/sim@0218c000", 0x430, 24},
	{MODULE_SIM2, "/soc/aips-bus@02100000/sim@021b4000", 0x430, 25},
	{MODULE_FLEXCAN1, "/soc/aips-bus@02000000/can@02090000", 0x430, 26},
	{MODULE_FLEXCAN2, "/soc/aips-bus@02000000/can@02094000", 0x430, 27},
	{MODULE_SPDIF, "/soc/aips-bus@02000000/spba-bus@02000000/spdif@02004000", 0x440, 2},
	{MODULE_EIM, "/soc/aips-bus@02100000/weim@021b8000", 0x440, 3},
	{MODULE_SD1, "/soc/aips-bus@02100000/usdhc@02190000", 0x440, 4},
	{MODULE_SD2, "/soc/aips-bus@02100000/usdhc@02194000", 0x440, 5},
	{MODULE_QSPI1, "/soc/aips-bus@02100000/qspi@021e0000", 0x440, 6},
	{MODULE_GPMI, "/soc/gpmi-nand@01806000", 0x440, 7},
	{MODULE_APBHDMA, "/soc/dma-apbh@01804000", 0x440, 7},
	{MODULE_LCDIF, "/soc/aips-bus@02100000/lcdif@021c8000", 0x440, 8},
	{MODULE_PXP, "/soc/aips-bus@02100000/pxp@021cc000", 0x440, 9},
	{MODULE_CSI, "/soc/aips-bus@02100000/csi@021c4000", 0x440, 10},
	{MODULE_ADC1, "/soc/aips-bus@02100000/adc@02198000", 0x440, 11},
	{MODULE_ENET1, "/soc/aips-bus@02100000/ethernet@02188000", 0x440, 12},
	{MODULE_ENET2, "/soc/aips-bus@02000000/ethernet@020b4000", 0x440, 13},
	{MODULE_CAAM, "/soc/aips-bus@02100000/caam@2140000", 0x440, 14},
	{MODULE_USB_OTG2, "/soc/aips-bus@02100000/usb@02184200", 0x440, 15},
	{MODULE_SAI2, "/soc/aips-bus@02000000/spba-bus@02000000/sai@0202c000", 0x440, 24},
	{MODULE_SAI3, "/soc/aips-bus@02000000/spba-bus@02000000/sai@02030000", 0x440, 24},
	{MODULE_BEE, "/soc/aips-bus@02000000/bee@02044000", 0x440, 25},
	{MODULE_UART5, "/soc/aips-bus@02100000/serial@021f4000", 0x440, 26},
	{MODULE_UART6, "/soc/aips-bus@02100000/serial@021fc000", 0x440, 26},
	{MODULE_UART7, "/soc/aips-bus@02000000/spba-bus@02000000/serial@02018000", 0x440, 26},
	{MODULE_UART8, "/soc/aips-bus@02000000/spba-bus@02000000/serial@02024000", 0x440, 26},
	{MODULE_PWM5, "/soc/aips-bus@02000000/pwm@020f0000", 0x440, 27},
	{MODULE_PWM6, "/soc/aips-bus@02000000/pwm@020f4000", 0x440, 27},
	{MODULE_PWM7, "/soc/aips-bus@02000000/pwm@020f8000", 0x440, 27},
	{MODULE_PWM8, "/soc/aips-bus@02000000/pwm@020fc000", 0x440, 27},
	{MODULE_ECSPI3, "/soc/aips-bus@02000000/spba-bus@02000000/ecspi@02010000", 0x440, 28},
	{MODULE_ECSPI4, "/soc/aips-bus@02000000/spba-bus@02000000/ecspi@02014000", 0x440, 28},
	{MODULE_I2C3, "/soc/aips-bus@02100000/i2c@021a8000", 0x440, 29},
	{MODULE_I2C4, "/soc/aips-bus@02100000/i2c@021f8000", 0x440, 29},
	{MODULE_GPT2, "/soc/aips-bus@02000000/gpt@020e8000", 0x440, 30},
	{MODULE_EPIT2, "/soc/aips-bus@02000000/epit@020d4000", 0x440, 31},
#endif
};

u32 check_module_fused(enum fuse_module_type module)
{
	u32 i, reg;

	for (i = 0; i < ARRAY_SIZE(mx6_fuse_descs); i++) {
		if (mx6_fuse_descs[i].module == module) {
			reg = readl(OCOTP_BASE_ADDR +
				    mx6_fuse_descs[i].fuse_word_offset);
			if (reg & BIT(mx6_fuse_descs[i].fuse_bit_offset))
				return 1; /* disabled */
			else
				return 0; /* enabled */
		}
	}

	return  0; /* Not has a fuse, always enabled */
}

#ifdef CONFIG_OF_SYSTEM_SETUP
int ft_system_setup(void *blob, struct bd_info *bd)
{
	const char *status = "disabled";
	u32 i, reg;
	int rc, off;

	for (i = 0; i < ARRAY_SIZE(mx6_fuse_descs); i++) {
		reg = readl(OCOTP_BASE_ADDR +
			    mx6_fuse_descs[i].fuse_word_offset);
		if (reg & BIT(mx6_fuse_descs[i].fuse_bit_offset)) {
			off = fdt_path_offset(blob,
					      mx6_fuse_descs[i].node_path);

			if (off < 0)
				continue; /* Not found, skip it */
add_status:
			rc = fdt_setprop(blob, off, "status", status,
					 strlen(status) + 1);
			if (rc) {
				if (rc == -FDT_ERR_NOSPACE) {
					rc = fdt_increase_size(blob, 512);
					if (!rc)
						goto add_status;
				}
				printf("Unable to update property %s:%s, err=%s\n", mx6_fuse_descs[i].node_path, "status", fdt_strerror(rc));
			} else {
				printf("Modify %s disabled\n", mx6_fuse_descs[i].node_path);
			}
		}
	}

	return 0;
}
#endif

u32 esdhc_fused(ulong base_addr)
{
	switch (base_addr) {
	case USDHC1_BASE_ADDR:
		return check_module_fused(MODULE_SD1);
	case USDHC2_BASE_ADDR:
		return check_module_fused(MODULE_SD2);
#ifdef USDHC3_BASE_ADDR
	case USDHC3_BASE_ADDR:
		return check_module_fused(MODULE_SD3);
#endif
#ifdef USDHC4_BASE_ADDR
	case USDHC4_BASE_ADDR:
		return check_module_fused(MODULE_SD4);
#endif
	default:
		return 0;
	}
}

u32 ecspi_fused(ulong base_addr)
{
	switch (base_addr) {
	case ECSPI1_BASE_ADDR:
		return check_module_fused(MODULE_ECSPI1);
	case ECSPI2_BASE_ADDR:
		return check_module_fused(MODULE_ECSPI2);
	case ECSPI3_BASE_ADDR:
		return check_module_fused(MODULE_ECSPI3);
	case ECSPI4_BASE_ADDR:
		return check_module_fused(MODULE_ECSPI4);
#ifdef ECSPI5_BASE_ADDR
	case ECSPI5_BASE_ADDR:
		return check_module_fused(MODULE_ECSPI5);
#endif
	default:
		return 0;
	}
}

u32 usb_fused(ulong base_addr)
{
	int i = (base_addr - USB_BASE_ADDR) / 0x200;

	return check_module_fused(MODULE_USB_OTG1 + i);
}

u32 qspi_fused(ulong base_addr)
{
	switch (base_addr) {
#ifdef QSPI1_BASE_ADDR
	case QSPI1_BASE_ADDR:
		return check_module_fused(MODULE_QSPI1);
#endif

#ifdef QSPI2_BASE_ADDR
	case QSPI2_BASE_ADDR:
		return check_module_fused(MODULE_QSPI2);
#endif
	default:
		return 0;
	}
}

u32 i2c_fused(ulong base_addr)
{
	switch (base_addr) {
	case I2C1_BASE_ADDR:
		return check_module_fused(MODULE_I2C1);
	case I2C2_BASE_ADDR:
		return check_module_fused(MODULE_I2C2);
	case I2C3_BASE_ADDR:
		return check_module_fused(MODULE_I2C3);
#ifdef I2C4_BASE_ADDR
	case I2C4_BASE_ADDR:
		return check_module_fused(MODULE_I2C4);
#endif
	}

	return 0;
}

u32 enet_fused(ulong base_addr)
{
	switch (base_addr) {
	case ENET_BASE_ADDR:
		return check_module_fused(MODULE_ENET1);
#ifdef ENET2_BASE_ADDR
	case ENET2_BASE_ADDR:
		return check_module_fused(MODULE_ENET2);
#endif
	default:
		return 0;
	}
}
