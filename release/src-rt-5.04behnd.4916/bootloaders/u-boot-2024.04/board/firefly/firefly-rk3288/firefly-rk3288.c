// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>
#include <hang.h>
#include <led.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/ofnode.h>

#ifdef CONFIG_SPL_BUILD
static int setup_led(void)
{
#ifdef CONFIG_SPL_LED
	struct udevice *dev;
	char *led_name;
	int ret;

	led_name = ofnode_conf_read_str("u-boot,boot-led");
	if (!led_name)
		return 0;
	ret = led_get_by_label(led_name, &dev);
	if (ret) {
		debug("%s: get=%d\n", __func__, ret);
		return ret;
	}
	ret = led_set_state(dev, LEDST_ON);
	if (ret)
		return ret;
#endif

	return 0;
}

void spl_board_init(void)
{
	int ret;

	ret = setup_led();
	if (ret) {
		debug("LED ret=%d\n", ret);
		hang();
	}
}
#endif
