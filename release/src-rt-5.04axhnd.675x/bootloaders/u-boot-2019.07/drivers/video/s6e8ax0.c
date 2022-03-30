// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <asm/arch/mipi_dsim.h>

#include "exynos/exynos_mipi_dsi_lowlevel.h"
#include "exynos/exynos_mipi_dsi_common.h"

static void s6e8ax0_panel_cond(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	int reverse = dsim_dev->dsim_lcd_dev->reverse_panel;
	static const unsigned char data_to_send[] = {
		0xf8, 0x3d, 0x35, 0x00, 0x00, 0x00, 0x8d, 0x00, 0x4c,
		0x6e, 0x10, 0x27, 0x7d, 0x3f, 0x10, 0x00, 0x00, 0x20,
		0x04, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x02, 0x08, 0x08,
		0x23, 0x23, 0xc0, 0xc8, 0x08, 0x48, 0xc1, 0x00, 0xc3,
		0xff, 0xff, 0xc8
	};

	static const unsigned char data_to_send_reverse[] = {
		0xf8, 0x19, 0x35, 0x00, 0x00, 0x00, 0x93, 0x00, 0x3c,
		0x7d, 0x08, 0x27, 0x7d, 0x3f, 0x00, 0x00, 0x00, 0x20,
		0x04, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x02, 0x08, 0x08,
		0x23, 0x23, 0xc0, 0xc1, 0x01, 0x41, 0xc1, 0x00, 0xc1,
		0xf6, 0xf6, 0xc1
	};

	if (reverse) {
		ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send_reverse,
			ARRAY_SIZE(data_to_send_reverse));
	} else {
		ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send, ARRAY_SIZE(data_to_send));
	}
}

static void s6e8ax0_display_cond(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xf2, 0x80, 0x03, 0x0d
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_gamma_cond(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	/* 7500K 2.2 Set : 30cd */
	static const unsigned char data_to_send[] = {
		0xfa, 0x01, 0x60, 0x10, 0x60, 0xf5, 0x00, 0xff, 0xad,
		0xaf, 0xba, 0xc3, 0xd8, 0xc5, 0x9f, 0xc6, 0x9e, 0xc1,
		0xdc, 0xc0, 0x00, 0x61, 0x00, 0x5a, 0x00, 0x74,
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_gamma_update(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xf7, 0x03
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_to_send,
			ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_etc_source_control(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xf6, 0x00, 0x02, 0x00
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_etc_pentile_control(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xb6, 0x0c, 0x02, 0x03, 0x32, 0xff, 0x44, 0x44, 0xc0,
		0x00
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_etc_mipi_control1(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xe1, 0x10, 0x1c, 0x17, 0x08, 0x1d
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_etc_mipi_control2(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xe2, 0xed, 0x07, 0xc3, 0x13, 0x0d, 0x03
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_etc_power_control(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xf4, 0xcf, 0x0a, 0x12, 0x10, 0x19, 0x33, 0x02
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
		data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_etc_mipi_control3(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xe3, 0x40
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_to_send,
		       ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_etc_mipi_control4(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xe4, 0x00, 0x00, 0x14, 0x80, 0x00, 0x00, 0x00
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
		data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_elvss_set(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xb1, 0x04, 0x00
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
			data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_display_on(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0x29, 0x00
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_SHORT_WRITE, data_to_send,
		       ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_sleep_out(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0x11, 0x00
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_SHORT_WRITE, data_to_send,
		       ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_apply_level1_key(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xf0, 0x5a, 0x5a
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
		data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_apply_mtp_key(struct mipi_dsim_device *dsim_dev)
{
	struct mipi_dsim_master_ops *ops = dsim_dev->master_ops;
	static const unsigned char data_to_send[] = {
		0xf1, 0x5a, 0x5a
	};

	ops->cmd_write(dsim_dev, MIPI_DSI_DCS_LONG_WRITE,
		data_to_send, ARRAY_SIZE(data_to_send));
}

static void s6e8ax0_panel_init(struct mipi_dsim_device *dsim_dev)
{
	/*
	 * in case of setting gamma and panel condition at first,
	 * it shuold be setting like below.
	 * set_gamma() -> set_panel_condition()
	 */

	s6e8ax0_apply_level1_key(dsim_dev);
	s6e8ax0_apply_mtp_key(dsim_dev);

	s6e8ax0_sleep_out(dsim_dev);
	mdelay(5);
	s6e8ax0_panel_cond(dsim_dev);
	s6e8ax0_display_cond(dsim_dev);
	s6e8ax0_gamma_cond(dsim_dev);
	s6e8ax0_gamma_update(dsim_dev);

	s6e8ax0_etc_source_control(dsim_dev);
	s6e8ax0_elvss_set(dsim_dev);
	s6e8ax0_etc_pentile_control(dsim_dev);
	s6e8ax0_etc_mipi_control1(dsim_dev);
	s6e8ax0_etc_mipi_control2(dsim_dev);
	s6e8ax0_etc_power_control(dsim_dev);
	s6e8ax0_etc_mipi_control3(dsim_dev);
	s6e8ax0_etc_mipi_control4(dsim_dev);
}

static int s6e8ax0_panel_set(struct mipi_dsim_device *dsim_dev)
{
	s6e8ax0_panel_init(dsim_dev);

	return 0;
}

static void s6e8ax0_display_enable(struct mipi_dsim_device *dsim_dev)
{
	s6e8ax0_display_on(dsim_dev);
}

static struct mipi_dsim_lcd_driver s6e8ax0_dsim_ddi_driver = {
	.name = "s6e8ax0",
	.id = -1,

	.mipi_panel_init = s6e8ax0_panel_set,
	.mipi_display_on = s6e8ax0_display_enable,
};

void s6e8ax0_init(void)
{
	exynos_mipi_dsi_register_lcd_driver(&s6e8ax0_dsim_ddi_driver);
}
