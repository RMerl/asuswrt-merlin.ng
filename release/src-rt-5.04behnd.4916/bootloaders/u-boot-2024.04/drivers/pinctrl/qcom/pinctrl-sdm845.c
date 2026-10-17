// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm SDM845 pinctrl
 *
 * (C) Copyright 2021 Dzmitry Sankouski <dsankouski@gmail.com>
 * (C) Copyright 2023 Linaro Ltd.
 *
 */

#include <common.h>
#include <dm.h>

#include "pinctrl-qcom.h"

#define NORTH	0x00500000
#define SOUTH	0x00900000
#define EAST	0x00100000

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup9", 1},
	{"gpio", 0},
};

static const unsigned int sdm845_pin_offsets[] = {
	[0] = EAST,    [1] = EAST,    [2] = EAST,    [3] = EAST,    [4] = NORTH,
	[5] = NORTH,   [6] = NORTH,   [7] = NORTH,   [8] = EAST,    [9] = EAST,
	[10] = EAST,   [11] = EAST,   [12] = SOUTH,  [13] = SOUTH,  [14] = SOUTH,
	[15] = SOUTH,  [16] = SOUTH,  [17] = SOUTH,  [18] = SOUTH,  [19] = SOUTH,
	[20] = SOUTH,  [21] = SOUTH,  [22] = SOUTH,  [23] = SOUTH,  [24] = SOUTH,
	[25] = SOUTH,  [26] = SOUTH,  [27] = EAST,   [28] = EAST,   [29] = EAST,
	[30] = EAST,   [31] = NORTH,  [32] = NORTH,  [33] = NORTH,  [34] = NORTH,
	[35] = SOUTH,  [36] = SOUTH,  [37] = SOUTH,  [38] = NORTH,  [39] = EAST,
	[40] = SOUTH,  [41] = EAST,   [42] = EAST,   [43] = EAST,   [44] = EAST,
	[45] = EAST,   [46] = EAST,   [47] = EAST,   [48] = EAST,   [49] = NORTH,
	[50] = NORTH,  [51] = NORTH,  [52] = NORTH,  [53] = NORTH,  [54] = NORTH,
	[55] = NORTH,  [56] = NORTH,  [57] = NORTH,  [58] = NORTH,  [59] = NORTH,
	[60] = NORTH,  [61] = NORTH,  [62] = NORTH,  [63] = NORTH,  [64] = NORTH,
	[65] = NORTH,  [66] = NORTH,  [67] = NORTH,  [68] = NORTH,  [69] = EAST,
	[70] = EAST,   [71] = EAST,   [72] = EAST,   [73] = EAST,   [74] = EAST,
	[75] = EAST,   [76] = EAST,   [77] = EAST,   [78] = EAST,   [79] = NORTH,
	[80] = NORTH,  [81] = NORTH,  [82] = NORTH,  [83] = NORTH,  [84] = NORTH,
	[85] = EAST,   [86] = EAST,   [87] = EAST,   [88] = EAST,   [89] = SOUTH,
	[90] = SOUTH,  [91] = SOUTH,  [92] = SOUTH,  [93] = SOUTH,  [94] = SOUTH,
	[95] = SOUTH,  [96] = SOUTH,  [97] = NORTH,  [98] = NORTH,  [99] = NORTH,
	[100] = NORTH, [101] = NORTH, [102] = NORTH, [103] = NORTH, [104] = NORTH,
	[105] = NORTH, [106] = NORTH, [107] = NORTH, [108] = NORTH, [109] = NORTH,
	[110] = NORTH, [111] = NORTH, [112] = NORTH, [113] = NORTH, [114] = NORTH,
	[115] = NORTH, [116] = NORTH, [117] = NORTH, [118] = NORTH, [119] = NORTH,
	[120] = NORTH, [121] = NORTH, [122] = EAST,  [123] = EAST,  [124] = EAST,
	[125] = EAST,  [126] = EAST,  [127] = NORTH, [128] = NORTH, [129] = NORTH,
	[130] = NORTH, [131] = NORTH, [132] = NORTH, [133] = NORTH, [134] = NORTH,
	[135] = NORTH, [136] = NORTH, [137] = NORTH, [138] = NORTH, [139] = NORTH,
	[140] = NORTH, [141] = NORTH, [142] = NORTH, [143] = NORTH, [144] = NORTH,
	[145] = NORTH, [146] = NORTH, [147] = NORTH, [148] = NORTH, [149] = NORTH,
};

static const char *sdm845_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sdm845_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
	return pin_name;
}

static unsigned int sdm845_get_function_mux(unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sdm845_data = {
	.pin_data = {
		.pin_offsets = sdm845_pin_offsets,
		.pin_count = ARRAY_SIZE(sdm845_pin_offsets),
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sdm845_get_function_name,
	.get_function_mux = sdm845_get_function_mux,
	.get_pin_name = sdm845_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sdm845-pinctrl", .data = (ulong)&sdm845_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_sdm845) = {
	.name		= "pinctrl_sdm845",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
