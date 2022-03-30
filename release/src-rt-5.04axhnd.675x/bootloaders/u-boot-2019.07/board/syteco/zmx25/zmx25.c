// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) 2011 Graf-Syteco, Matthias Weisser
 * <weisserm@arcor.de>
 *
 * Based on tx25.c:
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * Based on imx27lite.c:
 *   Copyright (C) 2008,2009 Eric Jarrige <jorasse@users.sourceforge.net>
 *   Copyright (C) 2009 Ilya Yanok <yanok@emcraft.com>
 * And:
 *   RedBoot tx25_misc.c Copyright (C) 2009 Red Hat
 */
#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx25.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init()
{
	static const iomux_v3_cfg_t sdhc1_pads[] = {
		NEW_PAD_CTRL(MX25_PAD_SD1_CMD__SD1_CMD, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_CLK__SD1_CLK, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_DATA0__SD1_DATA0, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_DATA1__SD1_DATA1, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_DATA2__SD1_DATA2, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_DATA3__SD1_DATA3, NO_PAD_CTRL),
	};

	static const iomux_v3_cfg_t dig_out_pads[] = {
		MX25_PAD_CSI_D8__GPIO_1_7, /* Ouput 1 Ctrl */
		MX25_PAD_CSI_D7__GPIO_1_6, /* Ouput 2 Ctrl */
		NEW_PAD_CTRL(MX25_PAD_CSI_D6__GPIO_1_31, 0), /* Ouput 1 Stat */
		NEW_PAD_CTRL(MX25_PAD_CSI_D5__GPIO_1_30, 0), /* Ouput 2 Stat */
	};

	static const iomux_v3_cfg_t led_pads[] = {
		MX25_PAD_CSI_D9__GPIO_4_21,
		MX25_PAD_CSI_D4__GPIO_1_29,
	};

	static const iomux_v3_cfg_t can_pads[] = {
		NEW_PAD_CTRL(MX25_PAD_GPIO_A__CAN1_TX, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_GPIO_B__CAN1_RX, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_GPIO_C__CAN2_TX, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_GPIO_D__CAN2_RX, NO_PAD_CTRL),
	};

	static const iomux_v3_cfg_t i2c3_pads[] = {
		MX25_PAD_CSPI1_SS1__I2C3_DAT,
		MX25_PAD_GPIO_E__I2C3_CLK,
	};

	icache_enable();

	/* Setup of core voltage selection pin to run at 1.4V */
	imx_iomux_v3_setup_pad(MX25_PAD_EXT_ARMCLK__GPIO_3_15); /* VCORE */
	gpio_direction_output(IMX_GPIO_NR(3, 15), 1);

	/* Setup of SD card pins*/
	imx_iomux_v3_setup_multiple_pads(sdhc1_pads, ARRAY_SIZE(sdhc1_pads));

	/* Setup of digital output for USB power and OC */
	imx_iomux_v3_setup_pad(MX25_PAD_CSI_D3__GPIO_1_28); /* USB Power */
	gpio_direction_output(IMX_GPIO_NR(1, 28), 1);

	imx_iomux_v3_setup_pad(MX25_PAD_CSI_D2__GPIO_1_27); /* USB OC */
	gpio_direction_input(IMX_GPIO_NR(1, 18));

	/* Setup of digital output control pins */
	imx_iomux_v3_setup_multiple_pads(dig_out_pads,
						ARRAY_SIZE(dig_out_pads));

	/* Switch both output drivers off */
	gpio_direction_output(IMX_GPIO_NR(1, 7), 0);
	gpio_direction_output(IMX_GPIO_NR(1, 6), 0);

	/* Setup of key input pin */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX25_PAD_KPP_ROW0__GPIO_2_29, 0));
	gpio_direction_input(IMX_GPIO_NR(2, 29));

	/* Setup of status LED outputs */
	imx_iomux_v3_setup_multiple_pads(led_pads, ARRAY_SIZE(led_pads));

	/* Switch both LEDs off */
	gpio_direction_output(IMX_GPIO_NR(4, 21), 0);
	gpio_direction_output(IMX_GPIO_NR(1, 29), 0);

	/* Setup of CAN1 and CAN2 signals */
	imx_iomux_v3_setup_multiple_pads(can_pads, ARRAY_SIZE(can_pads));

	/* Setup of I2C3 signals */
	imx_iomux_v3_setup_multiple_pads(i2c3_pads, ARRAY_SIZE(i2c3_pads));

	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int board_late_init(void)
{
	const char *e;

#ifdef CONFIG_FEC_MXC
/*
 * FIXME: need to revisit this
 * The original code enabled PUE and 100-k pull-down without PKE, so the right
 * value here is likely:
 *	0 for no pull
 * or:
 *	PAD_CTL_PUS_100K_DOWN for 100-k pull-down
 */
#define FEC_OUT_PAD_CTRL	0

	static const iomux_v3_cfg_t fec_pads[] = {
		MX25_PAD_FEC_TX_CLK__FEC_TX_CLK,
		MX25_PAD_FEC_RX_DV__FEC_RX_DV,
		MX25_PAD_FEC_RDATA0__FEC_RDATA0,
		NEW_PAD_CTRL(MX25_PAD_FEC_TDATA0__FEC_TDATA0, FEC_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_FEC_TX_EN__FEC_TX_EN, FEC_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_FEC_MDC__FEC_MDC, FEC_OUT_PAD_CTRL),
		MX25_PAD_FEC_MDIO__FEC_MDIO,
		MX25_PAD_FEC_RDATA1__FEC_RDATA1,
		NEW_PAD_CTRL(MX25_PAD_FEC_TDATA1__FEC_TDATA1, FEC_OUT_PAD_CTRL),

		MX25_PAD_UPLL_BYPCLK__GPIO_3_16, /* LAN-RESET */
		MX25_PAD_UART2_CTS__FEC_RX_ER, /* FEC_RX_ERR */
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));

	/* assert PHY reset (low) */
	gpio_direction_output(IMX_GPIO_NR(3, 16), 0);

	udelay(5000);

	/* deassert PHY reset */
	gpio_set_value(IMX_GPIO_NR(3, 16), 1);

	udelay(5000);
#endif

	e = env_get("gs_base_board");
	if (e != NULL) {
		if (strcmp(e, "G283") == 0) {
			int key = gpio_get_value(IMX_GPIO_NR(2, 29));

			if (key) {
				/* Switch on both LEDs to inidcate boot mode */
				gpio_set_value(IMX_GPIO_NR(1, 29), 0);
				gpio_set_value(IMX_GPIO_NR(4, 21), 0);

				env_set("preboot", "run gs_slow_boot");
			} else
				env_set("preboot", "run gs_fast_boot");
		}
	}

	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM,
				PHYS_SDRAM_SIZE);
	return 0;
}
