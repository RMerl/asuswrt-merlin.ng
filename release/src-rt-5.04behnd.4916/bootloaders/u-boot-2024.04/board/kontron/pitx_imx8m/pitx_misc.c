// SPDX-License-Identifier: GPL-2.0+
#include <asm/arch/imx8mq_pins.h>
#include <asm-generic/gpio.h>
#include <asm/mach-imx/gpio.h>

/*
 *   BRD_REV1   BRD_REV0
 *      0         0         n/a
 *      0         1         n/a
 *      1         0         2GB LPDDR4
 *      1         1         4GB LPDDR4
 */

#define BRD_REV0  IMX_GPIO_NR(5, 0)
#define BRD_REV1  IMX_GPIO_NR(5, 1)

static iomux_v3_cfg_t const brdrev_pads[] = {
	IMX8MQ_PAD_SAI3_TXC__GPIO5_IO0 | MUX_PAD_CTRL(PAD_CTL_PUE),
	IMX8MQ_PAD_SAI3_TXD__GPIO5_IO1 | MUX_PAD_CTRL(PAD_CTL_PUE),
	IMX8MQ_PAD_SAI3_MCLK__GPIO5_IO2 | MUX_PAD_CTRL(PAD_CTL_PUE),
};

int get_pitx_board_variant(void)
{
	int variant = 0;

	imx_iomux_v3_setup_multiple_pads(brdrev_pads, ARRAY_SIZE(brdrev_pads));

	gpio_request(BRD_REV0, "BRD_REV0");
	gpio_direction_input(BRD_REV0);
	gpio_request(BRD_REV1, "BRD_REV1");
	gpio_direction_input(BRD_REV1);

	variant |= !!gpio_get_value(BRD_REV0) << 0;
	variant |= !!gpio_get_value(BRD_REV1) << 1;

	return variant;
}
