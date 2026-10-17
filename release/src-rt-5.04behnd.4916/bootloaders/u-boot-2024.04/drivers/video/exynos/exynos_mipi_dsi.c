// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <fdtdec.h>
#include <asm/global_data.h>
#include <dm/devres.h>
#include <linux/libfdt.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <asm/arch/dsim.h>
#include <asm/arch/mipi_dsim.h>
#include <asm/arch/power.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>

#include "exynos_mipi_dsi_lowlevel.h"
#include "exynos_mipi_dsi_common.h"

#define master_to_driver(a)	(a->dsim_lcd_drv)
#define master_to_device(a)	(a->dsim_lcd_dev)

DECLARE_GLOBAL_DATA_PTR;

struct mipi_dsim_ddi {
	int				bus_id;
	struct list_head		list;
	struct mipi_dsim_lcd_device	*dsim_lcd_dev;
	struct mipi_dsim_lcd_driver	*dsim_lcd_drv;
};

static LIST_HEAD(dsim_ddi_list);
static LIST_HEAD(dsim_lcd_dev_list);

int exynos_mipi_dsi_register_lcd_device(struct mipi_dsim_lcd_device *lcd_dev)
{
	struct mipi_dsim_ddi *dsim_ddi;

	if (!lcd_dev) {
		debug("mipi_dsim_lcd_device is NULL.\n");
		return -EFAULT;
	}

	if (!lcd_dev->name) {
		debug("dsim_lcd_device name is NULL.\n");
		return -EFAULT;
	}

	dsim_ddi = kzalloc(sizeof(struct mipi_dsim_ddi), GFP_KERNEL);
	if (!dsim_ddi) {
		debug("failed to allocate dsim_ddi object.\n");
		return -EFAULT;
	}

	dsim_ddi->dsim_lcd_dev = lcd_dev;

	list_add_tail(&dsim_ddi->list, &dsim_ddi_list);

	return 0;
}

struct mipi_dsim_ddi
	*exynos_mipi_dsi_find_lcd_device(struct mipi_dsim_lcd_driver *lcd_drv)
{
	struct mipi_dsim_ddi *dsim_ddi;
	struct mipi_dsim_lcd_device *lcd_dev;

	list_for_each_entry(dsim_ddi, &dsim_ddi_list, list) {
		lcd_dev = dsim_ddi->dsim_lcd_dev;
		if (!lcd_dev)
			continue;

		if (lcd_drv->id >= 0) {
			if ((strcmp(lcd_drv->name, lcd_dev->name)) == 0 &&
					lcd_drv->id == lcd_dev->id) {
				/**
				 * bus_id would be used to identify
				 * connected bus.
				 */
				dsim_ddi->bus_id = lcd_dev->bus_id;

				return dsim_ddi;
			}
		} else {
			if ((strcmp(lcd_drv->name, lcd_dev->name)) == 0) {
				/**
				 * bus_id would be used to identify
				 * connected bus.
				 */
				dsim_ddi->bus_id = lcd_dev->bus_id;

				return dsim_ddi;
			}
		}

		kfree(dsim_ddi);
		list_del(&dsim_ddi_list);
	}

	return NULL;
}

int exynos_mipi_dsi_register_lcd_driver(struct mipi_dsim_lcd_driver *lcd_drv)
{
	struct mipi_dsim_ddi *dsim_ddi;

	if (!lcd_drv) {
		debug("mipi_dsim_lcd_driver is NULL.\n");
		return -EFAULT;
	}

	if (!lcd_drv->name) {
		debug("dsim_lcd_driver name is NULL.\n");
		return -EFAULT;
	}

	dsim_ddi = exynos_mipi_dsi_find_lcd_device(lcd_drv);
	if (!dsim_ddi) {
		debug("mipi_dsim_ddi object not found.\n");
		return -EFAULT;
	}

	dsim_ddi->dsim_lcd_drv = lcd_drv;

	debug("registered panel driver(%s) to mipi-dsi driver.\n",
		lcd_drv->name);

	return 0;

}

struct mipi_dsim_ddi
	*exynos_mipi_dsi_bind_lcd_ddi(struct mipi_dsim_device *dsim,
			const char *name)
{
	struct mipi_dsim_ddi *dsim_ddi;
	struct mipi_dsim_lcd_driver *lcd_drv;
	struct mipi_dsim_lcd_device *lcd_dev;

	list_for_each_entry(dsim_ddi, &dsim_ddi_list, list) {
		lcd_drv = dsim_ddi->dsim_lcd_drv;
		lcd_dev = dsim_ddi->dsim_lcd_dev;
		if (!lcd_drv || !lcd_dev)
			continue;

		debug("lcd_drv->id = %d, lcd_dev->id = %d\n",
					lcd_drv->id, lcd_dev->id);

		if ((strcmp(lcd_drv->name, name) == 0)) {
			lcd_dev->master = dsim;

			dsim->dsim_lcd_dev = lcd_dev;
			dsim->dsim_lcd_drv = lcd_drv;

			return dsim_ddi;
		}
	}

	return NULL;
}

/* define MIPI-DSI Master operations. */
static struct mipi_dsim_master_ops master_ops = {
	.cmd_write			= exynos_mipi_dsi_wr_data,
	.get_dsim_frame_done		= exynos_mipi_dsi_get_frame_done_status,
	.clear_dsim_frame_done		= exynos_mipi_dsi_clear_frame_done,
};

int exynos_mipi_dsi_init(struct exynos_platform_mipi_dsim *dsim_pd)
{
	struct mipi_dsim_device *dsim;
	struct mipi_dsim_config *dsim_config;
	struct mipi_dsim_ddi *dsim_ddi;

	dsim = kzalloc(sizeof(struct mipi_dsim_device), GFP_KERNEL);
	if (!dsim) {
		debug("failed to allocate dsim object.\n");
		return -EFAULT;
	}

	/* get mipi_dsim_config. */
	dsim_config = dsim_pd->dsim_config;
	if (dsim_config == NULL) {
		debug("failed to get dsim config data.\n");
		return -EFAULT;
	}

	dsim->pd = dsim_pd;
	dsim->dsim_config = dsim_config;
	dsim->master_ops = &master_ops;

	/* bind lcd ddi matched with panel name. */
	dsim_ddi = exynos_mipi_dsi_bind_lcd_ddi(dsim, dsim_pd->lcd_panel_name);
	if (!dsim_ddi) {
		debug("mipi_dsim_ddi object not found.\n");
		return -ENOSYS;
	}
	if (dsim_pd->lcd_power)
		dsim_pd->lcd_power();

	if (dsim_pd->mipi_power)
		dsim_pd->mipi_power();

	/* phy_enable(unsigned int dev_index, unsigned int enable) */
	if (dsim_pd->phy_enable)
		dsim_pd->phy_enable(0, 1);

	set_mipi_clk();

	exynos_mipi_dsi_init_dsim(dsim);
	exynos_mipi_dsi_init_link(dsim);
	exynos_mipi_dsi_set_hs_enable(dsim);

	/* set display timing. */
	exynos_mipi_dsi_set_display_mode(dsim, dsim->dsim_config);

	/* initialize mipi-dsi client(lcd panel). */
	if (dsim_ddi->dsim_lcd_drv && dsim_ddi->dsim_lcd_drv->mipi_panel_init) {
		dsim_ddi->dsim_lcd_drv->mipi_panel_init(dsim);
		dsim_ddi->dsim_lcd_drv->mipi_display_on(dsim);
	}

	debug("mipi-dsi driver(%s mode) has been probed.\n",
		(dsim_config->e_interface == DSIM_COMMAND) ?
			"CPU" : "RGB");

	return 0;
}

int exynos_dsim_config_parse_dt(const void *blob, struct mipi_dsim_config *dt,
				struct mipi_dsim_lcd_device *lcd_dt)
{
	int node;

	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS_MIPI_DSI);
	if (node <= 0) {
		printf("exynos_mipi_dsi: Can't get device node for mipi dsi\n");
		return -ENODEV;
	}

	dt->e_interface = fdtdec_get_int(blob, node,
				"samsung,dsim-config-e-interface", 0);

	dt->e_virtual_ch = fdtdec_get_int(blob, node,
				"samsung,dsim-config-e-virtual-ch", 0);

	dt->e_pixel_format = fdtdec_get_int(blob, node,
				"samsung,dsim-config-e-pixel-format", 0);

	dt->e_burst_mode = fdtdec_get_int(blob, node,
				"samsung,dsim-config-e-burst-mode", 0);

	dt->e_no_data_lane = fdtdec_get_int(blob, node,
				"samsung,dsim-config-e-no-data-lane", 0);

	dt->e_byte_clk = fdtdec_get_int(blob, node,
				"samsung,dsim-config-e-byte-clk", 0);

	dt->hfp = fdtdec_get_int(blob, node,
				"samsung,dsim-config-hfp", 0);

	dt->p = fdtdec_get_int(blob, node,
					  "samsung,dsim-config-p", 0);
	dt->m = fdtdec_get_int(blob, node,
					  "samsung,dsim-config-m", 0);
	dt->s = fdtdec_get_int(blob, node,
					  "samsung,dsim-config-s", 0);

	dt->pll_stable_time = fdtdec_get_int(blob, node,
				"samsung,dsim-config-pll-stable-time", 0);

	dt->esc_clk = fdtdec_get_int(blob, node,
				"samsung,dsim-config-esc-clk", 0);

	dt->stop_holding_cnt = fdtdec_get_int(blob, node,
				"samsung,dsim-config-stop-holding-cnt", 0);

	dt->bta_timeout = fdtdec_get_int(blob, node,
				"samsung,dsim-config-bta-timeout", 0);

	dt->rx_timeout = fdtdec_get_int(blob, node,
				"samsung,dsim-config-rx-timeout", 0);

	lcd_dt->name = fdt_getprop(blob, node, "samsung,dsim-device-name",
				   NULL);

	lcd_dt->id = fdtdec_get_int(blob, node,
				"samsung,dsim-device-id", 0);

	lcd_dt->bus_id = fdtdec_get_int(blob, node,
				"samsung,dsim-device-bus_id", 0);

	lcd_dt->reverse_panel = fdtdec_get_int(blob, node,
				"samsung,dsim-device-reverse-panel", 0);

	return 0;
}

void exynos_init_dsim_platform_data(vidinfo_t *vid)
{
	static struct mipi_dsim_config dsim_config_dt;
	static struct exynos_platform_mipi_dsim dsim_platform_data_dt;
	static struct mipi_dsim_lcd_device mipi_lcd_device_dt;

	if (exynos_dsim_config_parse_dt(gd->fdt_blob, &dsim_config_dt,
					&mipi_lcd_device_dt))
		debug("Can't get proper dsim config.\n");

	strcpy(dsim_platform_data_dt.lcd_panel_name, mipi_lcd_device_dt.name);
	dsim_platform_data_dt.dsim_config = &dsim_config_dt;
	dsim_platform_data_dt.mipi_power = mipi_power;
	dsim_platform_data_dt.phy_enable = set_mipi_phy_ctrl;
	dsim_platform_data_dt.lcd_panel_info = (void *)vid;

	mipi_lcd_device_dt.platform_data = (void *)&dsim_platform_data_dt;
	exynos_mipi_dsi_register_lcd_device(&mipi_lcd_device_dt);

	vid->dsim_platform_data_dt = &dsim_platform_data_dt;
}
