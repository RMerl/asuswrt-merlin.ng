// SPDX-License-Identifier: GPL-2.0
/*
 * RZ/G2L Pin Function Controller
 *
 * Copyright (C) 2021-2023 Renesas Electronics Corp.
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <renesas/rzg2l-pfc.h>
#include <reset.h>

struct rzg2l_pfc_driver_data {
	uint num_dedicated_pins;
	uint num_ports;
	const u32 *gpio_configs;
};

struct rzg2l_dedicated_configs {
	const char *name;
	u32 config;
};

/*
 * We need to ensure that the module clock is enabled and all resets are
 * de-asserted before using either the gpio or pinctrl functionality. Error
 * handling can be quite simple here as if the PFC cannot be enabled then we
 * will not be able to progress with the boot anyway.
 */
int rzg2l_pfc_enable(struct udevice *dev)
{
	struct reset_ctl_bulk rsts;
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get gpio module clock\n");
		return ret;
	}

	ret = clk_enable(&clk);
	if (ret < 0) {
		dev_err(dev, "failed to enable gpio module clock\n");
		return ret;
	}

	ret = reset_get_bulk(dev, &rsts);
	if (ret < 0) {
		dev_err(dev, "failed to get reset lines\n");
		return ret;
	}

	ret = reset_deassert_bulk(&rsts);
	if (ret < 0) {
		dev_err(dev, "failed to de-assert reset lines\n");
		return ret;
	}

	return 0;
}

bool rzg2l_port_validate(const struct rzg2l_pfc_data *data, u32 port, u8 pin)
{
	return (port < data->num_ports) &&
	       (pin < RZG2L_GPIO_PORT_GET_PINCNT(data->gpio_configs[port]));
}

/* Decode a pin selector, returning the port index and setting *pin to the pin
 * index. Returns -1 on error, which can be checked directly or by calling
 * rzg2l_port_validate().
 */
static int rzg2l_selector_decode(const struct rzg2l_pfc_data *data,
				 unsigned int selector,
				 u8 *pin)
{
	int port;

	selector -= data->num_dedicated_pins;
	for (port = 0; port < data->num_ports; port++) {
		u8 num_pins = RZG2L_GPIO_PORT_GET_PINCNT(data->gpio_configs[port]);
		if (selector < num_pins) {
			*pin = (u8)selector;
			return port;
		}
		selector -= num_pins;
	}
	return -EINVAL;
}

static unsigned int rzg2l_selector_encode(const struct rzg2l_pfc_data *data,
					  u32 port, u8 pin)
{
	unsigned int selector = data->num_dedicated_pins + pin;
	u32 i;

	for (i = 0; i < port; i++)
		selector += RZG2L_GPIO_PORT_GET_PINCNT(data->gpio_configs[i]);

	return selector;
}

static const char * const rzg2l_gpio_names[] = {
	"P0_0", "P0_1", "P0_2", "P0_3", "P0_4", "P0_5", "P0_6", "P0_7",
	"P1_0", "P1_1", "P1_2", "P1_3", "P1_4", "P1_5", "P1_6", "P1_7",
	"P2_0", "P2_1", "P2_2", "P2_3", "P2_4", "P2_5", "P2_6", "P2_7",
	"P3_0", "P3_1", "P3_2", "P3_3", "P3_4", "P3_5", "P3_6", "P3_7",
	"P4_0", "P4_1", "P4_2", "P4_3", "P4_4", "P4_5", "P4_6", "P4_7",
	"P5_0", "P5_1", "P5_2", "P5_3", "P5_4", "P5_5", "P5_6", "P5_7",
	"P6_0", "P6_1", "P6_2", "P6_3", "P6_4", "P6_5", "P6_6", "P6_7",
	"P7_0", "P7_1", "P7_2", "P7_3", "P7_4", "P7_5", "P7_6", "P7_7",
	"P8_0", "P8_1", "P8_2", "P8_3", "P8_4", "P8_5", "P8_6", "P8_7",
	"P9_0", "P9_1", "P9_2", "P9_3", "P9_4", "P9_5", "P9_6", "P9_7",
	"P10_0", "P10_1", "P10_2", "P10_3", "P10_4", "P10_5", "P10_6", "P10_7",
	"P11_0", "P11_1", "P11_2", "P11_3", "P11_4", "P11_5", "P11_6", "P11_7",
	"P12_0", "P12_1", "P12_2", "P12_3", "P12_4", "P12_5", "P12_6", "P12_7",
	"P13_0", "P13_1", "P13_2", "P13_3", "P13_4", "P13_5", "P13_6", "P13_7",
	"P14_0", "P14_1", "P14_2", "P14_3", "P14_4", "P14_5", "P14_6", "P14_7",
	"P15_0", "P15_1", "P15_2", "P15_3", "P15_4", "P15_5", "P15_6", "P15_7",
	"P16_0", "P16_1", "P16_2", "P16_3", "P16_4", "P16_5", "P16_6", "P16_7",
	"P17_0", "P17_1", "P17_2", "P17_3", "P17_4", "P17_5", "P17_6", "P17_7",
	"P18_0", "P18_1", "P18_2", "P18_3", "P18_4", "P18_5", "P18_6", "P18_7",
	"P19_0", "P19_1", "P19_2", "P19_3", "P19_4", "P19_5", "P19_6", "P19_7",
	"P20_0", "P20_1", "P20_2", "P20_3", "P20_4", "P20_5", "P20_6", "P20_7",
	"P21_0", "P21_1", "P21_2", "P21_3", "P21_4", "P21_5", "P21_6", "P21_7",
	"P22_0", "P22_1", "P22_2", "P22_3", "P22_4", "P22_5", "P22_6", "P22_7",
	"P23_0", "P23_1", "P23_2", "P23_3", "P23_4", "P23_5", "P23_6", "P23_7",
	"P24_0", "P24_1", "P24_2", "P24_3", "P24_4", "P24_5", "P24_6", "P24_7",
	"P25_0", "P25_1", "P25_2", "P25_3", "P25_4", "P25_5", "P25_6", "P25_7",
	"P26_0", "P26_1", "P26_2", "P26_3", "P26_4", "P26_5", "P26_6", "P26_7",
	"P27_0", "P27_1", "P27_2", "P27_3", "P27_4", "P27_5", "P27_6", "P27_7",
	"P28_0", "P28_1", "P28_2", "P28_3", "P28_4", "P28_5", "P28_6", "P28_7",
	"P29_0", "P29_1", "P29_2", "P29_3", "P29_4", "P29_5", "P29_6", "P29_7",
	"P30_0", "P30_1", "P30_2", "P30_3", "P30_4", "P30_5", "P30_6", "P30_7",
	"P31_0", "P31_1", "P31_2", "P31_3", "P31_4", "P31_5", "P31_6", "P31_7",
	"P32_0", "P32_1", "P32_2", "P32_3", "P32_4", "P32_5", "P32_6", "P32_7",
	"P33_0", "P33_1", "P33_2", "P33_3", "P33_4", "P33_5", "P33_6", "P33_7",
	"P34_0", "P34_1", "P34_2", "P34_3", "P34_4", "P34_5", "P34_6", "P34_7",
	"P35_0", "P35_1", "P35_2", "P35_3", "P35_4", "P35_5", "P35_6", "P35_7",
	"P36_0", "P36_1", "P36_2", "P36_3", "P36_4", "P36_5", "P36_6", "P36_7",
	"P37_0", "P37_1", "P37_2", "P37_3", "P37_4", "P37_5", "P37_6", "P37_7",
	"P38_0", "P38_1", "P38_2", "P38_3", "P38_4", "P38_5", "P38_6", "P38_7",
	"P39_0", "P39_1", "P39_2", "P39_3", "P39_4", "P39_5", "P39_6", "P39_7",
	"P40_0", "P40_1", "P40_2", "P40_3", "P40_4", "P40_5", "P40_6", "P40_7",
	"P41_0", "P41_1", "P41_2", "P41_3", "P41_4", "P41_5", "P41_6", "P41_7",
	"P42_0", "P42_1", "P42_2", "P42_3", "P42_4", "P42_5", "P42_6", "P42_7",
	"P43_0", "P43_1", "P43_2", "P43_3", "P43_4", "P43_5", "P43_6", "P43_7",
	"P44_0", "P44_1", "P44_2", "P44_3", "P44_4", "P44_5", "P44_6", "P44_7",
	"P45_0", "P45_1", "P45_2", "P45_3", "P45_4", "P45_5", "P45_6", "P45_7",
	"P46_0", "P46_1", "P46_2", "P46_3", "P46_4", "P46_5", "P46_6", "P46_7",
	"P47_0", "P47_1", "P47_2", "P47_3", "P47_4", "P47_5", "P47_6", "P47_7",
	"P48_0", "P48_1", "P48_2", "P48_3", "P48_4", "P48_5", "P48_6", "P48_7",
};

static const u32 r9a07g044_gpio_configs[] = {
	RZG2L_GPIO_PORT_PACK(2, 0x10, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x11, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x12, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x13, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x14, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(3, 0x15, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x16, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(3, 0x17, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(3, 0x18, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x19, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x1a, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x1b, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x1c, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(3, 0x1d, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x1e, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x1f, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x20, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(3, 0x21, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x22, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x23, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(3, 0x24, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x25, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x26, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x27, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x28, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x29, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x2a, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x2b, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x2c, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH0)),
	RZG2L_GPIO_PORT_PACK(2, 0x2d, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(2, 0x2e, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(2, 0x2f, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(2, 0x30, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(2, 0x31, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(2, 0x32, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(2, 0x33, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(2, 0x34, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(3, 0x35, RZG2L_MPXED_ETH_PIN_FUNCS(PIN_CFG_IO_VMC_ETH1)),
	RZG2L_GPIO_PORT_PACK(2, 0x36, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(3, 0x37, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(3, 0x38, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(2, 0x39, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(5, 0x3a, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(4, 0x3b, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(4, 0x3c, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(4, 0x3d, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(4, 0x3e, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(4, 0x3f, RZG2L_MPXED_PIN_FUNCS),
	RZG2L_GPIO_PORT_PACK(5, 0x40, RZG2L_MPXED_PIN_FUNCS),
};

static const struct {
	struct rzg2l_dedicated_configs common[35];
	struct rzg2l_dedicated_configs rzg2l_pins[7];
} rzg2l_dedicated_pins = {
	.common = {
		{ "NMI", RZG2L_SINGLE_PIN_PACK(0x1, 0,
		 (PIN_CFG_FILONOFF | PIN_CFG_FILNUM | PIN_CFG_FILCLKSEL)) },
		{ "TMS/SWDIO", RZG2L_SINGLE_PIN_PACK(0x2, 0,
		 (PIN_CFG_IOLH_A | PIN_CFG_SR | PIN_CFG_IEN)) },
		{ "TDO", RZG2L_SINGLE_PIN_PACK(0x3, 0,
		 (PIN_CFG_IOLH_A | PIN_CFG_SR | PIN_CFG_IEN)) },
		{ "AUDIO_CLK1", RZG2L_SINGLE_PIN_PACK(0x4, 0, PIN_CFG_IEN) },
		{ "AUDIO_CLK2", RZG2L_SINGLE_PIN_PACK(0x4, 1, PIN_CFG_IEN) },
		{ "SD0_CLK", RZG2L_SINGLE_PIN_PACK(0x6, 0,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_CMD", RZG2L_SINGLE_PIN_PACK(0x6, 1,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_RST#", RZG2L_SINGLE_PIN_PACK(0x6, 2,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_DATA0", RZG2L_SINGLE_PIN_PACK(0x7, 0,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_DATA1", RZG2L_SINGLE_PIN_PACK(0x7, 1,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_DATA2", RZG2L_SINGLE_PIN_PACK(0x7, 2,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_DATA3", RZG2L_SINGLE_PIN_PACK(0x7, 3,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_DATA4", RZG2L_SINGLE_PIN_PACK(0x7, 4,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_DATA5", RZG2L_SINGLE_PIN_PACK(0x7, 5,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_DATA6", RZG2L_SINGLE_PIN_PACK(0x7, 6,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD0_DATA7", RZG2L_SINGLE_PIN_PACK(0x7, 7,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD0)) },
		{ "SD1_CLK", RZG2L_SINGLE_PIN_PACK(0x8, 0,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_SD1)) },
		{ "SD1_CMD", RZG2L_SINGLE_PIN_PACK(0x8, 1,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD1)) },
		{ "SD1_DATA0", RZG2L_SINGLE_PIN_PACK(0x9, 0,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD1)) },
		{ "SD1_DATA1", RZG2L_SINGLE_PIN_PACK(0x9, 1,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD1)) },
		{ "SD1_DATA2", RZG2L_SINGLE_PIN_PACK(0x9, 2,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD1)) },
		{ "SD1_DATA3", RZG2L_SINGLE_PIN_PACK(0x9, 3,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IEN | PIN_CFG_IO_VMC_SD1)) },
		{ "QSPI0_SPCLK", RZG2L_SINGLE_PIN_PACK(0xa, 0,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI0_IO0", RZG2L_SINGLE_PIN_PACK(0xa, 1,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI0_IO1", RZG2L_SINGLE_PIN_PACK(0xa, 2,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI0_IO2", RZG2L_SINGLE_PIN_PACK(0xa, 3,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI0_IO3", RZG2L_SINGLE_PIN_PACK(0xa, 4,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI0_SSL", RZG2L_SINGLE_PIN_PACK(0xa, 5,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI_RESET#", RZG2L_SINGLE_PIN_PACK(0xc, 0,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI_WP#", RZG2L_SINGLE_PIN_PACK(0xc, 1,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "WDTOVF_PERROUT#", RZG2L_SINGLE_PIN_PACK(0xd, 0, (PIN_CFG_IOLH_A | PIN_CFG_SR)) },
		{ "RIIC0_SDA", RZG2L_SINGLE_PIN_PACK(0xe, 0, PIN_CFG_IEN) },
		{ "RIIC0_SCL", RZG2L_SINGLE_PIN_PACK(0xe, 1, PIN_CFG_IEN) },
		{ "RIIC1_SDA", RZG2L_SINGLE_PIN_PACK(0xe, 2, PIN_CFG_IEN) },
		{ "RIIC1_SCL", RZG2L_SINGLE_PIN_PACK(0xe, 3, PIN_CFG_IEN) },
	},
	.rzg2l_pins = {
		{ "QSPI_INT#", RZG2L_SINGLE_PIN_PACK(0xc, 2, (PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI1_SPCLK", RZG2L_SINGLE_PIN_PACK(0xb, 0,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI1_IO0", RZG2L_SINGLE_PIN_PACK(0xb, 1,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI1_IO1", RZG2L_SINGLE_PIN_PACK(0xb, 2,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI1_IO2", RZG2L_SINGLE_PIN_PACK(0xb, 3,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI1_IO3", RZG2L_SINGLE_PIN_PACK(0xb, 4,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR  | PIN_CFG_IO_VMC_QSPI)) },
		{ "QSPI1_SSL", RZG2L_SINGLE_PIN_PACK(0xb, 5,
		 (PIN_CFG_IOLH_B | PIN_CFG_SR | PIN_CFG_IO_VMC_QSPI)) },
	}
};

static void rzg2l_rmw_pin_config(const struct rzg2l_pfc_data *data, u32 offset,
				 u8 pin, u32 mask, u32 val)
{
	void __iomem *addr = data->base + offset;

	/* handle _L/_H for 32-bit register read/write */
	if (pin >= 4) {
		pin -= 4;
		addr += 4;
	}

	clrsetbits_le32(addr, mask << (pin * 8), val << (pin * 8));
}

static int rzg2l_get_pins_count(struct udevice *dev)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);

	return data->num_dedicated_pins + data->num_pins;
}

static const char *rzg2l_get_pin_name(struct udevice *dev, unsigned int selector)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	int port;
	u8 pin;

	if (selector < data->num_dedicated_pins) {
		if (selector >= ARRAY_SIZE(rzg2l_dedicated_pins.common)) {
			unsigned int u = selector - ARRAY_SIZE(rzg2l_dedicated_pins.common);
			return rzg2l_dedicated_pins.rzg2l_pins[u].name;
		} else {
			return rzg2l_dedicated_pins.common[selector].name;
		}
	}

	port = rzg2l_selector_decode(data, selector, &pin);
	if (port < 0)
		return "(invalid pin)";
	return rzg2l_gpio_names[pin + 8 * port];
}

static int rzg2l_pinconf_set(struct udevice *dev, unsigned int pin_selector,
			     unsigned int param, unsigned int argument)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	u32 cfg, port_offset;
	u8 pin;

	if (pin_selector >= data->num_dedicated_pins) {
		/* The pin selector refers to a multiplexed pin */
		int port = rzg2l_selector_decode(data, pin_selector, &pin);
		if (port < 0) {
			dev_err(dev, "Invalid pin selector %u:%u\n", port, pin);
			return port;
		}

		cfg = data->gpio_configs[port];
		port_offset = P(port);
	} else {
		/* The pin selector refers to a dedicated function pin */
		const struct rzg2l_dedicated_configs *dedicated_config;

		if (pin_selector >= data->num_dedicated_pins) {
			dev_err(dev, "Invalid dedicated pin %u\n", pin_selector);
			return -EINVAL;
		}

		if (pin_selector >= ARRAY_SIZE(rzg2l_dedicated_pins.common)) {
			pin_selector -= ARRAY_SIZE(rzg2l_dedicated_pins.common);
			dedicated_config = &rzg2l_dedicated_pins.rzg2l_pins[pin_selector];
		} else {
			dedicated_config = &rzg2l_dedicated_pins.common[pin_selector];
		}

		port_offset = RZG2L_SINGLE_PIN_GET_PORT_OFFSET(dedicated_config->config);
		pin = RZG2L_SINGLE_PIN_GET_BIT(dedicated_config->config);
		cfg = RZG2L_SINGLE_PIN_GET_CFGS(dedicated_config->config);
	}

	switch (param) {
	case PIN_CONFIG_INPUT_ENABLE: {
		if (!(cfg & PIN_CFG_IEN)) {
			dev_err(dev, "pin does not support IEN\n");
			return -EINVAL;
		}

		dev_dbg(dev, "port off %u:%u set IEN=%u\n",
			port_offset, pin, argument);
		rzg2l_rmw_pin_config(data, IEN(port_offset), pin, IEN_MASK, !!argument);
		break;
	}

	case PIN_CONFIG_POWER_SOURCE: {
		u32 pwr_reg = 0x0;

		/* argument is in mV */
		if (argument != 1800 && argument != 3300) {
			dev_err(dev, "Invalid mV %u\n", argument);
			return -EINVAL;
		}

		/*
		 * TODO: PIN_CFG_IO_VMC_ETH0 & PIN_CFG_IO_VMC_ETH1 will be
		 * handled when the RZ/G2L Ethernet driver is added.
		 */
		if (cfg & PIN_CFG_IO_VMC_SD0) {
			dev_dbg(dev, "port off %u:%u set SD_CH 0 PVDD=%u\n",
				port_offset, pin, argument);
			pwr_reg = SD_CH(0);
		} else if (cfg & PIN_CFG_IO_VMC_SD1) {
			dev_dbg(dev, "port off %u:%u set SD_CH 1 PVDD=%u\n",
				port_offset, pin, argument);
			pwr_reg = SD_CH(1);
		} else if (cfg & PIN_CFG_IO_VMC_QSPI) {
			dev_dbg(dev, "port off %u:%u set QSPI PVDD=%u\n",
				port_offset, pin, argument);
			pwr_reg = QSPI;
		} else {
			dev_dbg(dev, "pin power source is not selectable\n");
			return -EINVAL;
		}

		writel((argument == 1800) ? PVDD_1800 : PVDD_3300,
		       data->base + pwr_reg);
		break;
	}

	default:
		dev_err(dev, "Invalid pinconf parameter\n");
		return -EOPNOTSUPP;
	}

	return 0;
}

static int rzg2l_pinmux_property_set(struct udevice *dev, u32 pinmux_group)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	u32 port, pin, func, pfc_state;
	u8 pmc_state;

	func = RZG2L_PINMUX_TO_FUNC(pinmux_group);
	if (func > 5) {
		dev_err(dev, "Invalid pin function %u\n", func);
		return -EINVAL;
	}

	port = RZG2L_PINMUX_TO_PORT(pinmux_group);
	pin = RZG2L_PINMUX_TO_PIN(pinmux_group);
	if (!rzg2l_port_validate(data, port, pin)) {
		dev_err(dev, "Invalid pin selector %u:%u\n", port, pin);
		return -EINVAL;
	}

	/* Check current PMC & PFC to decide if we need to change anything. */
	pmc_state = readb(data->base + PMC(port)) & BIT(pin);
	pfc_state = (readl(data->base + PFC(port)) >> (pin * 4)) & PFC_MASK;
	if (pmc_state && pfc_state == func)
		return 0;

	dev_dbg(dev, "pinmux port %u pin %u func %u\n", port, pin, func);

	/* Set pin to 'Non-use (Hi-Z input protection)'  */
	clrbits_le16(data->base + PM(port), PM_MASK << (pin * 2));

	/* Temporarily switch to GPIO mode with PMC register */
	clrbits_8(data->base + PMC(port), BIT(pin));

	/* Set the PWPR register to allow PFC register to write */
	writel(0x0, data->base + PWPR);        /* B0WI=0, PFCWE=0 */
	writel(PWPR_PFCWE, data->base + PWPR);  /* B0WI=0, PFCWE=1 */

	/* Select Pin function mode with PFC register */
	clrsetbits_le32(data->base + PFC(port), PFC_MASK << (pin * 4),
			func << (pin * 4));

	/* Set the PWPR register to be write-protected */
	writel(0x0, data->base + PWPR);        /* B0WI=0, PFCWE=0 */
	writel(PWPR_B0WI, data->base + PWPR);  /* B0WI=1, PFCWE=0 */

	/* Switch to Peripheral pin function with PMC register */
	setbits_8(data->base + PMC(port), BIT(pin));

	return rzg2l_selector_encode(data, port, pin);
}

static int rzg2l_get_pin_muxing(struct udevice *dev, unsigned int selector,
				char *buf, int size)
{
	const struct rzg2l_pfc_data *data =
		(const struct rzg2l_pfc_data *)dev_get_driver_data(dev);
	u32 pmc_state;
	int port;
	u8 pin;

	if (selector < data->num_dedicated_pins) {
		snprintf(buf, size, rzg2l_get_pin_name(dev, selector));
		return 0;
	}

	port = rzg2l_selector_decode(data, selector, &pin);
	if (port < 0) {
		dev_err(dev, "Invalid pin selector %u:%u\n", port, pin);
		return port;
	}

	pmc_state = readb(data->base + PMC(port)) & BIT(pin);
	if (pmc_state) {
		u32 pfc_state = (readl(data->base + PFC(port)) >> (pin * 4)) & PFC_MASK;
		snprintf(buf, size, "Function %d", pfc_state);
		return 0;
	}

	snprintf(buf, size, "GPIO");
	return 0;
}

static const struct pinconf_param rzg2l_pinconf_params[] = {
	{ "input-enable",	PIN_CONFIG_INPUT_ENABLE,	1 },
	{ "power-source",	PIN_CONFIG_POWER_SOURCE,	3300 /* mV */ },
};

static const struct pinctrl_ops rzg2l_pinctrl_ops  = {
	.get_pins_count		= rzg2l_get_pins_count,
	.get_pin_name		= rzg2l_get_pin_name,

	.pinconf_num_params	= ARRAY_SIZE(rzg2l_pinconf_params),
	.pinconf_params		= rzg2l_pinconf_params,
	.pinconf_set		= rzg2l_pinconf_set,

	.pinmux_property_set	= rzg2l_pinmux_property_set,
	.set_state		= pinctrl_generic_set_state,
	.get_pin_muxing		= rzg2l_get_pin_muxing,
};

static int rzg2l_pinctrl_probe(struct udevice *dev)
{
	return rzg2l_pfc_enable(dev);
}

U_BOOT_DRIVER(rzg2l_pfc_pinctrl) = {
	.name		= "rzg2l-pfc-pinctrl",
	.id		= UCLASS_PINCTRL,
	.ops		= &rzg2l_pinctrl_ops,
	.probe		= rzg2l_pinctrl_probe,
};

static const struct rzg2l_pfc_driver_data r9a07g044_driver_data = {
	.num_dedicated_pins = ARRAY_SIZE(rzg2l_dedicated_pins.common) +
			      ARRAY_SIZE(rzg2l_dedicated_pins.rzg2l_pins),
	.num_ports = ARRAY_SIZE(r9a07g044_gpio_configs),
	.gpio_configs = r9a07g044_gpio_configs,
};

static const struct udevice_id rzg2l_pfc_ids[] = {
	{ .compatible = "renesas,r9a07g044-pinctrl", .data = (ulong)&r9a07g044_driver_data },
	{ /* sentinel */ }
};

static int rzg2l_pfc_bind(struct udevice *parent)
{
	struct rzg2l_pfc_driver_data *driver_data;
	struct rzg2l_pfc_data *data;
	struct udevice *pinctrl_dev;
	struct driver *drv;
	unsigned int i;
	int ret;

	driver_data =
		(struct rzg2l_pfc_driver_data *)dev_get_driver_data(parent);
	if (!driver_data)
		return -EINVAL;
	data = devm_kmalloc(parent, sizeof(*data), 0);
	if (!data)
		return -ENOMEM;

	data->base = dev_read_addr_ptr(parent);
	if (!data->base)
		return -EINVAL;
	data->num_dedicated_pins = driver_data->num_dedicated_pins;
	data->num_ports = driver_data->num_ports;
	data->gpio_configs = driver_data->gpio_configs;

	data->num_pins = 0;
	for (i = 0; i < data->num_ports; i++)
		data->num_pins += RZG2L_GPIO_PORT_GET_PINCNT(data->gpio_configs[i]);
	dev_dbg(parent, "%u dedicated pins, %u muxed ports, %u muxed pins\n",
		data->num_dedicated_pins, data->num_ports, data->num_pins);

	drv = lists_driver_lookup_name("rzg2l-pfc-pinctrl");
	if (!drv)
		return -ENOENT;

	ret = device_bind_with_driver_data(parent, drv, parent->name,
					   (ulong)data, dev_ofnode(parent),
					   &pinctrl_dev);

	if (!ret && IS_ENABLED(CONFIG_RZG2L_GPIO)) {
		drv = lists_driver_lookup_name("rzg2l-pfc-gpio");
		if (!drv) {
			device_unbind(pinctrl_dev);
			return -ENOENT;
		}

		ret = device_bind_with_driver_data(parent, drv, parent->name,
						   (ulong)data,
						   dev_ofnode(parent), NULL);
		if (ret)
			device_unbind(pinctrl_dev);
	}

	return ret;
}

U_BOOT_DRIVER(rzg2l_pfc) = {
	.name		= "rzg2l-pfc",
	.id		= UCLASS_NOP,
	.of_match	= rzg2l_pfc_ids,
	.bind		= rzg2l_pfc_bind,
};
