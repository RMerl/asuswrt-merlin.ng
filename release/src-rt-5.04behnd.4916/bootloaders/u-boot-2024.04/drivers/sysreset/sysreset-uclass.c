// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_SYSRESET

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <hang.h>
#include <log.h>
#include <regmap.h>
#include <spl.h>
#include <sysreset.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/global_data.h>

int sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct sysreset_ops *ops = sysreset_get_ops(dev);

	if (!ops->request)
		return -ENOSYS;

	return ops->request(dev, type);
}

int sysreset_get_status(struct udevice *dev, char *buf, int size)
{
	struct sysreset_ops *ops = sysreset_get_ops(dev);

	if (!ops->get_status)
		return -ENOSYS;

	return ops->get_status(dev, buf, size);
}

int sysreset_get_last(struct udevice *dev)
{
	struct sysreset_ops *ops = sysreset_get_ops(dev);

	if (!ops->get_last)
		return -ENOSYS;

	return ops->get_last(dev);
}

int sysreset_walk(enum sysreset_t type)
{
	struct udevice *dev;
	int ret = -ENOSYS;

	while (ret != -EINPROGRESS && type < SYSRESET_COUNT) {
		for (uclass_first_device(UCLASS_SYSRESET, &dev);
		     dev;
		     uclass_next_device(&dev)) {
			ret = sysreset_request(dev, type);
			if (ret == -EINPROGRESS)
				break;
		}
		type++;
	}
    
	return ret;
}

int sysreset_get_last_walk(void)
{
	struct udevice *dev;
	int value = -ENOENT;

	for (uclass_first_device(UCLASS_SYSRESET, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		int ret;

		ret = sysreset_get_last(dev);
		if (ret >= 0) {
			value = ret;
			break;
		}
	}

	return value;
}

void sysreset_walk_halt(enum sysreset_t type)
{
#if defined(CONFIG_SMC_BASED) && defined(CONFIG_BCMBCA_NO_SMC_BOOT)
    // VVV temporary code only for bring up of XIP mode
    *((volatile unsigned int*)0x842100B8) |= 0x40000000;
#else
	int ret;

	ret = sysreset_walk(type);

	/* Wait for the reset to take effect */
	if (ret == -EINPROGRESS)
		mdelay(100);

	/* Still no reset? Give up */
	if (spl_phase() <= PHASE_SPL)
		log_err("no sysreset\n");
	else
		log_err("System reset not supported on this platform\n");
	hang();
#endif
}

/**
 * reset_cpu() - calls sysreset_walk(SYSRESET_WARM)
 */
void reset_cpu(void)
{
	sysreset_walk_halt(SYSRESET_WARM);
}


#if IS_ENABLED(CONFIG_SYSRESET_CMD_RESET)
int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	enum sysreset_t reset_type = SYSRESET_COLD;

	if (argc > 2)
		return CMD_RET_USAGE;

	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'w') {
		reset_type = SYSRESET_WARM;
	}

	printf("resetting ...\n");
	mdelay(100);

	sysreset_walk_halt(reset_type);

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_SYSRESET_CMD_POWEROFF)
int do_poweroff(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	puts("poweroff ...\n");
	mdelay(100);

	ret = sysreset_walk(SYSRESET_POWER_OFF);

	if (ret == -EINPROGRESS)
		mdelay(1000);

	/*NOTREACHED when power off*/
	return CMD_RET_FAILURE;
}
#endif

UCLASS_DRIVER(sysreset) = {
	.id		= UCLASS_SYSRESET,
	.name		= "sysreset",
};
