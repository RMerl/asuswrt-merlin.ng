// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <linux/stddef.h>
#include <adc.h>
#include <asm/io.h>
#include <dm.h>
#include <env.h>
#include <stdlib.h>

#define DTB_DIR			"rockchip/"

struct oga_model {
	const u16 adc_value;
	const char *board;
	const char *board_name;
	const char *fdtfile;
};

enum oga_device_id {
	OGA,
	OGA_V11,
	OGS,
};

/*
 * All ADC values from schematic of Odroid Go Advance Black Edition.
 * Value for OGS is inferred based on schematic and observed values.
 */
static const struct oga_model oga_model_details[] = {
	[OGA] = {
		856,
		"rk3326-odroid-go2",
		"ODROID-GO Advance",
		DTB_DIR "rk3326-odroid-go2.dtb",
	},
	[OGA_V11] = {
		677,
		"rk3326-odroid-go2-v11",
		"ODROID-GO Advance Black Edition",
		DTB_DIR "rk3326-odroid-go2-v11.dtb",
	},
	[OGS] = {
		85,
		"rk3326-odroid-go3",
		"ODROID-GO Super",
		DTB_DIR "rk3326-odroid-go3.dtb",
	},
};

/* Detect which Odroid Go Advance device we are using so as to load the
 * correct devicetree for Linux. Set an environment variable once
 * found. The detection depends on the value of ADC channel 0.
 */
int oga_detect_device(void)
{
	u32 adc_info;
	int ret, i;
	int board_id = -ENXIO;

	ret = adc_channel_single_shot("saradc@ff288000", 0, &adc_info);
	if (ret) {
		printf("Read SARADC failed with error %d\n", ret);
		return ret;
	}

	/*
	 * Get the correct device from the table. The ADC value is
	 * determined by a resistor on ADC channel 0. The manufacturer
	 * accounted for this with a 5% tolerance, so assume a +- value
	 * of 50 should be enough.
	 */
	for (i = 0; i < ARRAY_SIZE(oga_model_details); i++) {
		u32 adc_min = oga_model_details[i].adc_value - 50;
		u32 adc_max = oga_model_details[i].adc_value + 50;

		if (adc_min < adc_info && adc_max > adc_info) {
			board_id = i;
			break;
		}
	}

	if (board_id < 0)
		return board_id;

	env_set("board", oga_model_details[board_id].board);
	env_set("board_name",
		oga_model_details[board_id].board_name);
	env_set("fdtfile", oga_model_details[board_id].fdtfile);

	return 0;
}

int rk_board_late_init(void)
{
	int ret;

	ret = oga_detect_device();
	if (ret) {
		printf("Unable to detect device type: %d\n", ret);
		return ret;
	}

	return 0;
}
