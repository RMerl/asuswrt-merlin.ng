// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <adc.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define KEY_DOWN_MIN_VAL        0
#define KEY_DOWN_MAX_VAL        30

/*
 * Two board variants whith adc channel 3 is for board id
 * v10: 1024, v11: 512
 * v10: adc channel 0 for dnl key
 * v11: adc channel 1 for dnl key
 */
int rockchip_dnl_key_pressed(void)
{
	unsigned int key_val, id_val;
	int key_ch;

	if (adc_channel_single_shot("saradc", 3, &id_val)) {
		printf("%s read board id failed\n", __func__);
		return false;
	}

	if (abs(id_val - 1024) <= 30)
		key_ch = 0;
	else
		key_ch = 1;

	if (adc_channel_single_shot("saradc", key_ch, &key_val)) {
		printf("%s read adc key val failed\n", __func__);
		return false;
	}

	if (key_val >= KEY_DOWN_MIN_VAL && key_val <= KEY_DOWN_MAX_VAL)
		return true;
	else
		return false;
}
