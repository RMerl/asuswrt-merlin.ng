// SPDX-License-Identifier: GPL-2.0+
/*
 * Control PWM channels
 *
 * Copyright (c) 2020 SiFive, Inc
 * author: Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <command.h>
#include <dm.h>
#include <pwm.h>

enum pwm_cmd {
	PWM_SET_INVERT,
	PWM_SET_CONFIG,
	PWM_SET_ENABLE,
	PWM_SET_DISABLE,
};

static int do_pwm(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	const char *str_cmd, *str_channel = NULL, *str_enable = NULL;
	const char *str_pwm = NULL, *str_period = NULL, *str_duty = NULL;
	enum pwm_cmd sub_cmd;
	struct udevice *dev;
	u32 channel, pwm_enable, pwm_dev, period_ns = 0, duty_ns = 0;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;

	str_cmd = argv[1];
	argc -= 2;
	argv += 2;

	str_pwm = *argv;
	argc--;
	argv++;

	if (!str_pwm)
		return CMD_RET_USAGE;

	switch (*str_cmd) {
	case 'i':
		sub_cmd = PWM_SET_INVERT;
		if (argc != 2)
			return CMD_RET_USAGE;
		break;
	case 'c':
		sub_cmd = PWM_SET_CONFIG;
		if (argc != 3)
			return CMD_RET_USAGE;
		break;
	case 'e':
		sub_cmd = PWM_SET_ENABLE;
		if (argc != 1)
			return CMD_RET_USAGE;
		break;
	case 'd':
		sub_cmd = PWM_SET_DISABLE;
		if (argc != 1)
			return CMD_RET_USAGE;
		break;
	default:
		return CMD_RET_USAGE;
	}

	pwm_dev = dectoul(str_pwm, NULL);
	ret = uclass_get_device(UCLASS_PWM, pwm_dev, &dev);
	if (ret) {
		printf("pwm: '%s' not found\n", str_pwm);
		return cmd_process_error(cmdtp, ret);
	}

	str_channel = *argv;
	channel = dectoul(str_channel, NULL);
	argc--;
	argv++;

	if (sub_cmd == PWM_SET_INVERT) {
		str_enable = *argv;
		pwm_enable = dectoul(str_enable, NULL);
		ret = pwm_set_invert(dev, channel, pwm_enable);
	} else if (sub_cmd == PWM_SET_CONFIG) {
		str_period = *argv;
		argc--;
		argv++;
		period_ns = dectoul(str_period, NULL);

		str_duty = *argv;
		duty_ns = dectoul(str_duty, NULL);

		ret = pwm_set_config(dev, channel, period_ns, duty_ns);
	} else if (sub_cmd == PWM_SET_ENABLE) {
		ret = pwm_set_enable(dev, channel, 1);
	} else if (sub_cmd == PWM_SET_DISABLE) {
		ret = pwm_set_enable(dev, channel, 0);
	}

	if (ret) {
		printf("error(%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(pwm, 6, 0, do_pwm,
	   "control pwm channels",
	   "invert <pwm_dev_num> <channel> <polarity> - invert polarity\n"
	   "pwm config <pwm_dev_num> <channel> <period_ns> <duty_ns> - config PWM\n"
	   "pwm enable <pwm_dev_num> <channel> - enable PWM output\n"
	   "pwm disable <pwm_dev_num> <channel> - disable PWM output\n"
	   "Note: All input values are in decimal");
