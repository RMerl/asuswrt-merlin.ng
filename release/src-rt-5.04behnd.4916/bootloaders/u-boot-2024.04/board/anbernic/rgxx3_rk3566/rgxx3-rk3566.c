// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Chris Morgan <macromorgan@hotmail.com>
 */

#include <abuf.h>
#include <adc.h>
#include <asm/io.h>
#include <display.h>
#include <dm.h>
#include <dm/lists.h>
#include <env.h>
#include <fdt_support.h>
#include <linux/delay.h>
#include <mipi_dsi.h>
#include <mmc.h>
#include <panel.h>
#include <pwm.h>
#include <stdlib.h>
#include <video_bridge.h>

#define GPIO0_BASE		0xfdd60000
#define GPIO4_BASE		0xfe770000
#define GPIO_SWPORT_DR_L	0x0000
#define GPIO_SWPORT_DR_H	0x0004
#define GPIO_SWPORT_DDR_L	0x0008
#define GPIO_SWPORT_DDR_H	0x000c
#define GPIO_A0			BIT(0)
#define GPIO_C5			BIT(5)
#define GPIO_C6			BIT(6)
#define GPIO_C7			BIT(7)

#define GPIO_WRITEMASK(bits)	((bits) << 16)

#define DTB_DIR			"rockchip/"

struct rg3xx_model {
	const u16 adc_value;
	const char *board;
	const char *board_name;
	const char *fdtfile;
	const bool detect_panel;
};

enum rgxx3_device_id {
	RG353M,
	RG353P,
	RG353V,
	RG503,
	RGB30,
	RK2023,
	RGARCD,
	/* Devices with duplicate ADC value */
	RG353PS,
	RG353VS,
	RGARCS,
};

static const struct rg3xx_model rg3xx_model_details[] = {
	[RG353M] = {
		.adc_value = 517, /* Observed average from device */
		.board = "rk3566-anbernic-rg353m",
		.board_name = "RG353M",
		/* Device is identical to RG353P. */
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353p.dtb",
		.detect_panel = 1,
	},
	[RG353P] = {
		.adc_value = 860, /* Documented value of 860 */
		.board = "rk3566-anbernic-rg353p",
		.board_name = "RG353P",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353p.dtb",
		.detect_panel = 1,
	},
	[RG353V] = {
		.adc_value = 695, /* Observed average from device */
		.board = "rk3566-anbernic-rg353v",
		.board_name = "RG353V",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353v.dtb",
		.detect_panel = 1,
	},
	[RG503] = {
		.adc_value = 1023, /* Observed average from device */
		.board = "rk3566-anbernic-rg503",
		.board_name = "RG503",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg503.dtb",
		.detect_panel = 0,
	},
	[RGB30] = {
		.adc_value = 383, /* Gathered from second hand information */
		.board = "rk3566-powkiddy-rgb30",
		.board_name = "RGB30",
		.fdtfile = DTB_DIR "rk3566-powkiddy-rgb30.dtb",
		.detect_panel = 0,
	},
	[RK2023] = {
		.adc_value = 635, /* Observed average from device */
		.board = "rk3566-powkiddy-rk2023",
		.board_name = "RK2023",
		.fdtfile = DTB_DIR "rk3566-powkiddy-rk2023.dtb",
		.detect_panel = 0,
	},
	[RGARCD] = {
		.adc_value = 183, /* Observed average from device */
		.board = "rk3566-anbernic-rg-arc-d",
		.board_name = "Anbernic RG ARC-D",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg-arc-d.dtb",
		.detect_panel = 0,
	},
	/* Devices with duplicate ADC value */
	[RG353PS] = {
		.adc_value = 860, /* Observed average from device */
		.board = "rk3566-anbernic-rg353ps",
		.board_name = "RG353PS",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353ps.dtb",
		.detect_panel = 1,
	},
	[RG353VS] = {
		.adc_value = 695, /* Gathered from second hand information */
		.board = "rk3566-anbernic-rg353vs",
		.board_name = "RG353VS",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353vs.dtb",
		.detect_panel = 1,
	},
	[RGARCS] = {
		.adc_value = 183, /* Observed average from device */
		.board = "rk3566-anbernic-rg-arc-s",
		.board_name = "Anbernic RG ARC-S",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg-arc-s.dtb",
		.detect_panel = 0,
	},
};

struct rg353_panel {
	const u16 id;
	const char *panel_compat[2];
};

static const struct rg353_panel rg353_panel_details[] = {
	{
		.id = 0x3052,
		.panel_compat[0] = "anbernic,rg353p-panel",
		.panel_compat[1] = "newvision,nv3051d",
	},
	{
		.id = 0x3821,
		.panel_compat[0] = "anbernic,rg353v-panel-v2",
		.panel_compat[1] = NULL,
	},
};

/*
 * Start LED very early so user knows device is on. Set color
 * to red.
 */
void spl_board_init(void)
{
	/* Set GPIO0_C5, GPIO0_C6, and GPIO0_C7 to output. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) | \
	       (GPIO_C7 | GPIO_C6 | GPIO_C5),
	       (GPIO0_BASE + GPIO_SWPORT_DDR_H));
	/* Set GPIO0_C5 and GPIO_C6 to 0 and GPIO0_C7 to 1. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) | GPIO_C7,
	       (GPIO0_BASE + GPIO_SWPORT_DR_H));
}

/*
 * Buzz the buzzer so the user knows something is going on. Make it
 * optional in case PWM is disabled.
 */
void __maybe_unused startup_buzz(void)
{
	struct udevice *dev;
	int err;

	err = uclass_get_device(UCLASS_PWM, 0, &dev);
	if (err)
		printf("pwm not found\n");

	pwm_set_enable(dev, 0, 1);
	mdelay(200);
	pwm_set_enable(dev, 0, 0);
}

/*
 * Provide the bare minimum to identify the panel for the RG353
 * series. Since we don't have a working framebuffer device, no
 * need to init the panel; just identify it and provide the
 * clocks so we know what to set the different clock values to.
 */

static const struct display_timing rg353_default_timing = {
	.pixelclock.typ		= 24150000,
	.hactive.typ		= 640,
	.hfront_porch.typ	= 40,
	.hback_porch.typ	= 80,
	.hsync_len.typ		= 2,
	.vactive.typ		= 480,
	.vfront_porch.typ	= 18,
	.vback_porch.typ	= 28,
	.vsync_len.typ		= 2,
	.flags			= DISPLAY_FLAGS_HSYNC_HIGH |
				  DISPLAY_FLAGS_VSYNC_HIGH,
};

static int anbernic_rg353_panel_get_timing(struct udevice *dev,
					   struct display_timing *timings)
{
	memcpy(timings, &rg353_default_timing, sizeof(*timings));

	return 0;
}

static int anbernic_rg353_panel_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO |
			   MIPI_DSI_MODE_VIDEO_BURST |
			   MIPI_DSI_MODE_EOT_PACKET |
			   MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct panel_ops anbernic_rg353_panel_ops = {
	.get_display_timing = anbernic_rg353_panel_get_timing,
};

U_BOOT_DRIVER(anbernic_rg353_panel) = {
	.name		= "anbernic_rg353_panel",
	.id		= UCLASS_PANEL,
	.ops		= &anbernic_rg353_panel_ops,
	.probe		= anbernic_rg353_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
};

int rgxx3_detect_display(void)
{
	struct udevice *dev;
	struct mipi_dsi_device *dsi;
	struct mipi_dsi_panel_plat *mplat;
	const struct rg353_panel *panel;
	int ret = 0;
	int i;
	u8 panel_id[2];

	/*
	 * Take panel out of reset status.
	 * Set GPIO4_A0 to output.
	 */
	writel(GPIO_WRITEMASK(GPIO_A0) | GPIO_A0,
	       (GPIO4_BASE + GPIO_SWPORT_DDR_L));
	/* Set GPIO4_A0 to 1. */
	writel(GPIO_WRITEMASK(GPIO_A0) | GPIO_A0,
	       (GPIO4_BASE + GPIO_SWPORT_DR_L));

	/* Probe the DSI controller. */
	ret = uclass_get_device_by_name(UCLASS_VIDEO_BRIDGE,
					"dsi@fe060000", &dev);
	if (ret) {
		printf("DSI host not probed: %d\n", ret);
		return ret;
	}

	/* Probe the DSI panel. */
	ret = device_bind_driver_to_node(dev, "anbernic_rg353_panel",
					 "anbernic_rg353_panel",
					 dev_ofnode(dev), NULL);
	if (ret) {
		printf("Failed to probe RG353 panel: %d\n", ret);
		return ret;
	}

	/*
	 * Attach the DSI controller which will also probe and attach
	 * the DSIDPHY.
	 */
	ret = video_bridge_attach(dev);
	if (ret) {
		printf("Failed to attach DSI controller: %d\n", ret);
		return ret;
	}

	/*
	 * Get the panel which should have already been probed by the
	 * video_bridge_attach() function.
	 */
	ret = uclass_first_device_err(UCLASS_PANEL, &dev);
	if (ret) {
		printf("Panel device error: %d\n", ret);
		return ret;
	}

	/* Now call the panel via DSI commands to get the panel ID. */
	mplat = dev_get_plat(dev);
	dsi = mplat->device;
	mipi_dsi_set_maximum_return_packet_size(dsi, sizeof(panel_id));
	ret = mipi_dsi_dcs_read(dsi, MIPI_DCS_GET_DISPLAY_ID, &panel_id,
				sizeof(panel_id));
	if (ret < 0) {
		printf("Unable to read panel ID: %d\n", ret);
		return ret;
	}

	/* Get the correct panel compatible from the table. */
	for (i = 0; i < ARRAY_SIZE(rg353_panel_details); i++) {
		if (rg353_panel_details[i].id == ((panel_id[0] << 8) |
						 panel_id[1])) {
			panel = &rg353_panel_details[i];
			break;
		}
	}

	if (!panel) {
		printf("Unable to identify panel_id %x\n",
		       (panel_id[0] << 8) | panel_id[1]);
		return -EINVAL;
	}

	env_set("panel", panel->panel_compat[0]);

	return 0;
}

/* Detect which Anbernic RGXX3 device we are using so as to load the
 * correct devicetree for Linux. Set an environment variable once
 * found. The detection depends on the value of ADC channel 1, the
 * presence of an eMMC on mmc0, and querying the DSI panel.
 */
int rgxx3_detect_device(void)
{
	u32 adc_info;
	int ret, i;
	int board_id = -ENXIO;
	struct mmc *mmc;

	ret = adc_channel_single_shot("saradc@fe720000", 1, &adc_info);
	if (ret) {
		printf("Read SARADC failed with error %d\n", ret);
		return ret;
	}

	/*
	 * Get the correct device from the table. The ADC value is
	 * determined by a resistor on ADC channel 0. The hardware
	 * design calls for no more than a 1% variance on the
	 * resistor, so assume a +- value of 15 should be enough.
	 */
	for (i = 0; i < ARRAY_SIZE(rg3xx_model_details); i++) {
		u32 adc_min = rg3xx_model_details[i].adc_value - 15;
		u32 adc_max = rg3xx_model_details[i].adc_value + 15;

		if (adc_min < adc_info && adc_max > adc_info) {
			board_id = i;
			break;
		}
	}

	/*
	 * Try to access the eMMC on an RG353V, RG353P, or RG Arc D.
	 * If it's missing, it's an RG353VS, RG353PS, or RG Arc S.
	 * Note we could also check for a touchscreen at 0x1a on i2c2.
	 */
	if (board_id == RG353V || board_id == RG353P || board_id == RGARCD) {
		mmc = find_mmc_device(0);
		if (mmc) {
			ret = mmc_init(mmc);
			if (ret) {
				if (board_id == RG353V)
					board_id = RG353VS;
				else if (board_id == RG353P)
					board_id = RG353PS;
				else
					board_id = RGARCS;
			}
		}
	}

	if (board_id < 0)
		return board_id;

	env_set("board", rg3xx_model_details[board_id].board);
	env_set("board_name",
		rg3xx_model_details[board_id].board_name);
	env_set("fdtfile", rg3xx_model_details[board_id].fdtfile);

	/* Skip panel detection for when it is not needed. */
	if (!rg3xx_model_details[board_id].detect_panel)
		return 0;

	/* Warn but don't fail for errors in auto-detection of the panel. */
	ret = rgxx3_detect_display();
	if (ret)
		printf("Failed to detect panel type\n");

	return 0;
}

int rk_board_late_init(void)
{
	int ret;

	ret = rgxx3_detect_device();
	if (ret) {
		printf("Unable to detect device type: %d\n", ret);
		return ret;
	}

	/* Turn off red LED and turn on orange LED. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) | GPIO_C6,
	       (GPIO0_BASE + GPIO_SWPORT_DR_H));

	if (IS_ENABLED(CONFIG_DM_PWM))
		startup_buzz();

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	const struct rg353_panel *panel = NULL;
	int node, ret, i;
	char *env;

	/* No fixups necessary for the RG503 */
	env = env_get("board_name");
	if (env && (!strcmp(env, rg3xx_model_details[RG503].board_name)))
		return 0;

	/* Change the model name of the RG353M */
	if (env && (!strcmp(env, rg3xx_model_details[RG353M].board_name)))
		fdt_setprop(blob, 0, "model",
			    rg3xx_model_details[RG353M].board_name,
			    sizeof(rg3xx_model_details[RG353M].board_name));

	env = env_get("panel");
	if (!env) {
		printf("Can't get panel env\n");
		return 0;
	}

	/*
	 * Check if the environment variable doesn't equal the panel.
	 * If it doesn't, update the devicetree to the correct panel.
	 */
	node = fdt_path_offset(blob, "/dsi@fe060000/panel@0");
	if (!(node > 0)) {
		printf("Can't find the DSI node\n");
		return -ENODEV;
	}

	ret = fdt_node_check_compatible(blob, node, env);
	if (ret < 0)
		return -ENODEV;

	/* Panels match, return 0. */
	if (!ret)
		return 0;

	/* Panels don't match, search by first compatible value. */
	for (i = 0; i < ARRAY_SIZE(rg353_panel_details); i++) {
		if (!strcmp(env, rg353_panel_details[i].panel_compat[0])) {
			panel = &rg353_panel_details[i];
			break;
		}
	}

	if (!panel) {
		printf("Unable to identify panel by compat string\n");
		return -ENODEV;
	}

	/* Set the compatible with the auto-detected values */
	fdt_setprop_string(blob, node, "compatible", panel->panel_compat[0]);
	if (panel->panel_compat[1])
		fdt_appendprop_string(blob, node, "compatible",
				      panel->panel_compat[1]);

	return 0;
}
