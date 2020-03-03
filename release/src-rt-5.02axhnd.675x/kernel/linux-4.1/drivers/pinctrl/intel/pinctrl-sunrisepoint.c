/*
 * Intel Sunrisepoint PCH pinctrl/GPIO driver
 *
 * Copyright (C) 2015, Intel Corporation
 * Authors: Mathias Nyman <mathias.nyman@linux.intel.com>
 *          Mika Westerberg <mika.westerberg@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/acpi.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-intel.h"

#define SPT_PAD_OWN	0x020
#define SPT_PADCFGLOCK	0x0a0
#define SPT_HOSTSW_OWN	0x0d0
#define SPT_GPI_IE	0x120

#define SPT_COMMUNITY(b, s, e)				\
	{						\
		.barno = (b),				\
		.padown_offset = SPT_PAD_OWN,		\
		.padcfglock_offset = SPT_PADCFGLOCK,	\
		.hostown_offset = SPT_HOSTSW_OWN,	\
		.ie_offset = SPT_GPI_IE,		\
		.pin_base = (s),			\
		.npins = ((e) - (s) + 1),		\
	}

/* Sunrisepoint-LP */
static const struct pinctrl_pin_desc sptlp_pins[] = {
	/* GPP_A */
	PINCTRL_PIN(0, "RCINB"),
	PINCTRL_PIN(1, "LAD_0"),
	PINCTRL_PIN(2, "LAD_1"),
	PINCTRL_PIN(3, "LAD_2"),
	PINCTRL_PIN(4, "LAD_3"),
	PINCTRL_PIN(5, "LFRAMEB"),
	PINCTRL_PIN(6, "SERIQ"),
	PINCTRL_PIN(7, "PIRQAB"),
	PINCTRL_PIN(8, "CLKRUNB"),
	PINCTRL_PIN(9, "CLKOUT_LPC_0"),
	PINCTRL_PIN(10, "CLKOUT_LPC_1"),
	PINCTRL_PIN(11, "PMEB"),
	PINCTRL_PIN(12, "BM_BUSYB"),
	PINCTRL_PIN(13, "SUSWARNB_SUS_PWRDNACK"),
	PINCTRL_PIN(14, "SUS_STATB"),
	PINCTRL_PIN(15, "SUSACKB"),
	PINCTRL_PIN(16, "SD_1P8_SEL"),
	PINCTRL_PIN(17, "SD_PWR_EN_B"),
	PINCTRL_PIN(18, "ISH_GP_0"),
	PINCTRL_PIN(19, "ISH_GP_1"),
	PINCTRL_PIN(20, "ISH_GP_2"),
	PINCTRL_PIN(21, "ISH_GP_3"),
	PINCTRL_PIN(22, "ISH_GP_4"),
	PINCTRL_PIN(23, "ISH_GP_5"),
	/* GPP_B */
	PINCTRL_PIN(24, "CORE_VID_0"),
	PINCTRL_PIN(25, "CORE_VID_1"),
	PINCTRL_PIN(26, "VRALERTB"),
	PINCTRL_PIN(27, "CPU_GP_2"),
	PINCTRL_PIN(28, "CPU_GP_3"),
	PINCTRL_PIN(29, "SRCCLKREQB_0"),
	PINCTRL_PIN(30, "SRCCLKREQB_1"),
	PINCTRL_PIN(31, "SRCCLKREQB_2"),
	PINCTRL_PIN(32, "SRCCLKREQB_3"),
	PINCTRL_PIN(33, "SRCCLKREQB_4"),
	PINCTRL_PIN(34, "SRCCLKREQB_5"),
	PINCTRL_PIN(35, "EXT_PWR_GATEB"),
	PINCTRL_PIN(36, "SLP_S0B"),
	PINCTRL_PIN(37, "PLTRSTB"),
	PINCTRL_PIN(38, "SPKR"),
	PINCTRL_PIN(39, "GSPI0_CSB"),
	PINCTRL_PIN(40, "GSPI0_CLK"),
	PINCTRL_PIN(41, "GSPI0_MISO"),
	PINCTRL_PIN(42, "GSPI0_MOSI"),
	PINCTRL_PIN(43, "GSPI1_CSB"),
	PINCTRL_PIN(44, "GSPI1_CLK"),
	PINCTRL_PIN(45, "GSPI1_MISO"),
	PINCTRL_PIN(46, "GSPI1_MOSI"),
	PINCTRL_PIN(47, "SML1ALERTB"),
	/* GPP_C */
	PINCTRL_PIN(48, "SMBCLK"),
	PINCTRL_PIN(49, "SMBDATA"),
	PINCTRL_PIN(50, "SMBALERTB"),
	PINCTRL_PIN(51, "SML0CLK"),
	PINCTRL_PIN(52, "SML0DATA"),
	PINCTRL_PIN(53, "SML0ALERTB"),
	PINCTRL_PIN(54, "SML1CLK"),
	PINCTRL_PIN(55, "SML1DATA"),
	PINCTRL_PIN(56, "UART0_RXD"),
	PINCTRL_PIN(57, "UART0_TXD"),
	PINCTRL_PIN(58, "UART0_RTSB"),
	PINCTRL_PIN(59, "UART0_CTSB"),
	PINCTRL_PIN(60, "UART1_RXD"),
	PINCTRL_PIN(61, "UART1_TXD"),
	PINCTRL_PIN(62, "UART1_RTSB"),
	PINCTRL_PIN(63, "UART1_CTSB"),
	PINCTRL_PIN(64, "I2C0_SDA"),
	PINCTRL_PIN(65, "I2C0_SCL"),
	PINCTRL_PIN(66, "I2C1_SDA"),
	PINCTRL_PIN(67, "I2C1_SCL"),
	PINCTRL_PIN(68, "UART2_RXD"),
	PINCTRL_PIN(69, "UART2_TXD"),
	PINCTRL_PIN(70, "UART2_RTSB"),
	PINCTRL_PIN(71, "UART2_CTSB"),
	/* GPP_D */
	PINCTRL_PIN(72, "SPI1_CSB"),
	PINCTRL_PIN(73, "SPI1_CLK"),
	PINCTRL_PIN(74, "SPI1_MISO_IO_1"),
	PINCTRL_PIN(75, "SPI1_MOSI_IO_0"),
	PINCTRL_PIN(76, "FLASHTRIG"),
	PINCTRL_PIN(77, "ISH_I2C0_SDA"),
	PINCTRL_PIN(78, "ISH_I2C0_SCL"),
	PINCTRL_PIN(79, "ISH_I2C1_SDA"),
	PINCTRL_PIN(80, "ISH_I2C1_SCL"),
	PINCTRL_PIN(81, "ISH_SPI_CSB"),
	PINCTRL_PIN(82, "ISH_SPI_CLK"),
	PINCTRL_PIN(83, "ISH_SPI_MISO"),
	PINCTRL_PIN(84, "ISH_SPI_MOSI"),
	PINCTRL_PIN(85, "ISH_UART0_RXD"),
	PINCTRL_PIN(86, "ISH_UART0_TXD"),
	PINCTRL_PIN(87, "ISH_UART0_RTSB"),
	PINCTRL_PIN(88, "ISH_UART0_CTSB"),
	PINCTRL_PIN(89, "DMIC_CLK_1"),
	PINCTRL_PIN(90, "DMIC_DATA_1"),
	PINCTRL_PIN(91, "DMIC_CLK_0"),
	PINCTRL_PIN(92, "DMIC_DATA_0"),
	PINCTRL_PIN(93, "SPI1_IO_2"),
	PINCTRL_PIN(94, "SPI1_IO_3"),
	PINCTRL_PIN(95, "SSP_MCLK"),
	/* GPP_E */
	PINCTRL_PIN(96, "SATAXPCIE_0"),
	PINCTRL_PIN(97, "SATAXPCIE_1"),
	PINCTRL_PIN(98, "SATAXPCIE_2"),
	PINCTRL_PIN(99, "CPU_GP_0"),
	PINCTRL_PIN(100, "SATA_DEVSLP_0"),
	PINCTRL_PIN(101, "SATA_DEVSLP_1"),
	PINCTRL_PIN(102, "SATA_DEVSLP_2"),
	PINCTRL_PIN(103, "CPU_GP_1"),
	PINCTRL_PIN(104, "SATA_LEDB"),
	PINCTRL_PIN(105, "USB2_OCB_0"),
	PINCTRL_PIN(106, "USB2_OCB_1"),
	PINCTRL_PIN(107, "USB2_OCB_2"),
	PINCTRL_PIN(108, "USB2_OCB_3"),
	PINCTRL_PIN(109, "DDSP_HPD_0"),
	PINCTRL_PIN(110, "DDSP_HPD_1"),
	PINCTRL_PIN(111, "DDSP_HPD_2"),
	PINCTRL_PIN(112, "DDSP_HPD_3"),
	PINCTRL_PIN(113, "EDP_HPD"),
	PINCTRL_PIN(114, "DDPB_CTRLCLK"),
	PINCTRL_PIN(115, "DDPB_CTRLDATA"),
	PINCTRL_PIN(116, "DDPC_CTRLCLK"),
	PINCTRL_PIN(117, "DDPC_CTRLDATA"),
	PINCTRL_PIN(118, "DDPD_CTRLCLK"),
	PINCTRL_PIN(119, "DDPD_CTRLDATA"),
	/* GPP_F */
	PINCTRL_PIN(120, "SSP2_SCLK"),
	PINCTRL_PIN(121, "SSP2_SFRM"),
	PINCTRL_PIN(122, "SSP2_TXD"),
	PINCTRL_PIN(123, "SSP2_RXD"),
	PINCTRL_PIN(124, "I2C2_SDA"),
	PINCTRL_PIN(125, "I2C2_SCL"),
	PINCTRL_PIN(126, "I2C3_SDA"),
	PINCTRL_PIN(127, "I2C3_SCL"),
	PINCTRL_PIN(128, "I2C4_SDA"),
	PINCTRL_PIN(129, "I2C4_SCL"),
	PINCTRL_PIN(130, "I2C5_SDA"),
	PINCTRL_PIN(131, "I2C5_SCL"),
	PINCTRL_PIN(132, "EMMC_CMD"),
	PINCTRL_PIN(133, "EMMC_DATA_0"),
	PINCTRL_PIN(134, "EMMC_DATA_1"),
	PINCTRL_PIN(135, "EMMC_DATA_2"),
	PINCTRL_PIN(136, "EMMC_DATA_3"),
	PINCTRL_PIN(137, "EMMC_DATA_4"),
	PINCTRL_PIN(138, "EMMC_DATA_5"),
	PINCTRL_PIN(139, "EMMC_DATA_6"),
	PINCTRL_PIN(140, "EMMC_DATA_7"),
	PINCTRL_PIN(141, "EMMC_RCLK"),
	PINCTRL_PIN(142, "EMMC_CLK"),
	PINCTRL_PIN(143, "GPP_F_23"),
	/* GPP_G */
	PINCTRL_PIN(144, "SD_CMD"),
	PINCTRL_PIN(145, "SD_DATA_0"),
	PINCTRL_PIN(146, "SD_DATA_1"),
	PINCTRL_PIN(147, "SD_DATA_2"),
	PINCTRL_PIN(148, "SD_DATA_3"),
	PINCTRL_PIN(149, "SD_CDB"),
	PINCTRL_PIN(150, "SD_CLK"),
	PINCTRL_PIN(151, "SD_WP"),
};

static const unsigned sptlp_spi0_pins[] = { 39, 40, 41, 42 };
static const unsigned sptlp_spi1_pins[] = { 43, 44, 45, 46 };
static const unsigned sptlp_uart0_pins[] = { 56, 57, 58, 59 };
static const unsigned sptlp_uart1_pins[] = { 60, 61, 62, 63 };
static const unsigned sptlp_uart2_pins[] = { 68, 69, 71, 71 };
static const unsigned sptlp_i2c0_pins[] = { 64, 65 };
static const unsigned sptlp_i2c1_pins[] = { 66, 67 };
static const unsigned sptlp_i2c2_pins[] = { 124, 125 };
static const unsigned sptlp_i2c3_pins[] = { 126, 127 };
static const unsigned sptlp_i2c4_pins[] = { 128, 129 };
static const unsigned sptlp_i2c4b_pins[] = { 85, 86 };
static const unsigned sptlp_i2c5_pins[] = { 130, 131 };
static const unsigned sptlp_ssp2_pins[] = { 120, 121, 122, 123 };
static const unsigned sptlp_emmc_pins[] = {
	132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
};
static const unsigned sptlp_sd_pins[] = {
	144, 145, 146, 147, 148, 149, 150, 151,
};

static const struct intel_pingroup sptlp_groups[] = {
	PIN_GROUP("spi0_grp", sptlp_spi0_pins, 1),
	PIN_GROUP("spi1_grp", sptlp_spi1_pins, 1),
	PIN_GROUP("uart0_grp", sptlp_uart0_pins, 1),
	PIN_GROUP("uart1_grp", sptlp_uart1_pins, 1),
	PIN_GROUP("uart2_grp", sptlp_uart2_pins, 1),
	PIN_GROUP("i2c0_grp", sptlp_i2c0_pins, 1),
	PIN_GROUP("i2c1_grp", sptlp_i2c1_pins, 1),
	PIN_GROUP("i2c2_grp", sptlp_i2c2_pins, 1),
	PIN_GROUP("i2c3_grp", sptlp_i2c3_pins, 1),
	PIN_GROUP("i2c4_grp", sptlp_i2c4_pins, 1),
	PIN_GROUP("i2c4b_grp", sptlp_i2c4b_pins, 3),
	PIN_GROUP("i2c5_grp", sptlp_i2c5_pins, 1),
	PIN_GROUP("ssp2_grp", sptlp_ssp2_pins, 1),
	PIN_GROUP("emmc_grp", sptlp_emmc_pins, 1),
	PIN_GROUP("sd_grp", sptlp_sd_pins, 1),
};

static const char * const sptlp_spi0_groups[] = { "spi0_grp" };
static const char * const sptlp_spi1_groups[] = { "spi0_grp" };
static const char * const sptlp_uart0_groups[] = { "uart0_grp" };
static const char * const sptlp_uart1_groups[] = { "uart1_grp" };
static const char * const sptlp_uart2_groups[] = { "uart2_grp" };
static const char * const sptlp_i2c0_groups[] = { "i2c0_grp" };
static const char * const sptlp_i2c1_groups[] = { "i2c1_grp" };
static const char * const sptlp_i2c2_groups[] = { "i2c2_grp" };
static const char * const sptlp_i2c3_groups[] = { "i2c3_grp" };
static const char * const sptlp_i2c4_groups[] = { "i2c4_grp", "i2c4b_grp" };
static const char * const sptlp_i2c5_groups[] = { "i2c5_grp" };
static const char * const sptlp_ssp2_groups[] = { "ssp2_grp" };
static const char * const sptlp_emmc_groups[] = { "emmc_grp" };
static const char * const sptlp_sd_groups[] = { "sd_grp" };

static const struct intel_function sptlp_functions[] = {
	FUNCTION("spi0", sptlp_spi0_groups),
	FUNCTION("spi1", sptlp_spi1_groups),
	FUNCTION("uart0", sptlp_uart0_groups),
	FUNCTION("uart1", sptlp_uart1_groups),
	FUNCTION("uart2", sptlp_uart2_groups),
	FUNCTION("i2c0", sptlp_i2c0_groups),
	FUNCTION("i2c1", sptlp_i2c1_groups),
	FUNCTION("i2c2", sptlp_i2c2_groups),
	FUNCTION("i2c3", sptlp_i2c3_groups),
	FUNCTION("i2c4", sptlp_i2c4_groups),
	FUNCTION("i2c5", sptlp_i2c5_groups),
	FUNCTION("ssp2", sptlp_ssp2_groups),
	FUNCTION("emmc", sptlp_emmc_groups),
	FUNCTION("sd", sptlp_sd_groups),
};

static const struct intel_community sptlp_communities[] = {
	SPT_COMMUNITY(0, 0, 47),
	SPT_COMMUNITY(1, 48, 119),
	SPT_COMMUNITY(2, 120, 151),
};

static const struct intel_pinctrl_soc_data sptlp_soc_data = {
	.pins = sptlp_pins,
	.npins = ARRAY_SIZE(sptlp_pins),
	.groups = sptlp_groups,
	.ngroups = ARRAY_SIZE(sptlp_groups),
	.functions = sptlp_functions,
	.nfunctions = ARRAY_SIZE(sptlp_functions),
	.communities = sptlp_communities,
	.ncommunities = ARRAY_SIZE(sptlp_communities),
};

static const struct acpi_device_id spt_pinctrl_acpi_match[] = {
	{ "INT344B", (kernel_ulong_t)&sptlp_soc_data },
	{ }
};
MODULE_DEVICE_TABLE(acpi, spt_pinctrl_acpi_match);

static int spt_pinctrl_probe(struct platform_device *pdev)
{
	const struct intel_pinctrl_soc_data *soc_data;
	const struct acpi_device_id *id;

	id = acpi_match_device(spt_pinctrl_acpi_match, &pdev->dev);
	if (!id || !id->driver_data)
		return -ENODEV;

	soc_data = (const struct intel_pinctrl_soc_data *)id->driver_data;
	return intel_pinctrl_probe(pdev, soc_data);
}

static const struct dev_pm_ops spt_pinctrl_pm_ops = {
	SET_LATE_SYSTEM_SLEEP_PM_OPS(intel_pinctrl_suspend,
				     intel_pinctrl_resume)
};

static struct platform_driver spt_pinctrl_driver = {
	.probe = spt_pinctrl_probe,
	.remove = intel_pinctrl_remove,
	.driver = {
		.name = "sunrisepoint-pinctrl",
		.acpi_match_table = spt_pinctrl_acpi_match,
		.pm = &spt_pinctrl_pm_ops,
	},
};

static int __init spt_pinctrl_init(void)
{
	return platform_driver_register(&spt_pinctrl_driver);
}
subsys_initcall(spt_pinctrl_init);

static void __exit spt_pinctrl_exit(void)
{
	platform_driver_unregister(&spt_pinctrl_driver);
}
module_exit(spt_pinctrl_exit);

MODULE_AUTHOR("Mathias Nyman <mathias.nyman@linux.intel.com>");
MODULE_AUTHOR("Mika Westerberg <mika.westerberg@linux.intel.com>");
MODULE_DESCRIPTION("Intel Sunrisepoint PCH pinctrl/GPIO driver");
MODULE_LICENSE("GPL v2");
