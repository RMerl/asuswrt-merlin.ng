// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <dm/uclass-internal.h>

/*
 * Swap mmc0 and mmc1 in boot_targets if booted from SD-Card.
 *
 * If bootsource is uSD-card we can assume that we want to use the
 * SD-Card instead of the eMMC as first boot_target for distroboot.
 * We only want to swap the defaults and not any custom environment a
 * user has set. We exit early if a changed boot_targets environment
 * is detected.
 */
int setup_boottargets(void)
{
	const char *boot_device =
		ofnode_read_chosen_string("u-boot,spl-boot-device");
	char env_default[sizeof(BOOT_TARGETS)];
	char *env;
	int ret;

	if (!boot_device) {
		debug("%s: /chosen/u-boot,spl-boot-device not set\n",
		      __func__);
		return -1;
	}
	debug("%s: booted from %s\n", __func__, boot_device);

	ret = env_get_default_into("boot_targets", env_default, sizeof(env_default));
	if (ret < 0)
		env_default[0] = '\0';
	env = env_get("boot_targets");
	if (!env) {
		debug("%s: boot_targets does not exist\n", __func__);
		return -1;
	}
	debug("%s: boot_targets current: %s - default: %s\n",
	      __func__, env, env_default);

	if (strcmp(env_default, env) != 0) {
		debug("%s: boot_targets not default, don't change it\n",
		      __func__);
		return 0;
	}

	/*
	 * Make the default boot medium between SD Card and eMMC, the one that
	 * was used to load U-Boot proper. If SPI-NOR flash was used, keep
	 * original default order.
	 */
	struct udevice *devp;

	if (uclass_find_device_by_ofnode(UCLASS_MMC, ofnode_path(boot_device), &devp)) {
		debug("%s: not reordering boot_targets, bootdev %s != MMC\n",
		      __func__, boot_device);
		return 0;
	}

	char *mmc0, *mmc1;

	mmc0 = strstr(env, "mmc0");
	mmc1 = strstr(env, "mmc1");

	if (!mmc0 || !mmc1) {
		debug("%s: only one mmc boot_target found\n", __func__);
		return -1;
	}

	/*
	 * If mmc0 comes first in the boot order and U-Boot proper was
	 * loaded from mmc1, swap mmc0 and mmc1 in the list.
	 * If mmc1 comes first in the boot order and U-Boot proper was
	 * loaded from mmc0, swap mmc0 and mmc1 in the list.
	 */
	if ((mmc0 < mmc1 && devp->seq_ == 1) ||
	    (mmc0 > mmc1 && devp->seq_ == 0)) {
		mmc0[3] = '1';
		mmc1[3] = '0';
		debug("%s: set boot_targets to: %s\n", __func__, env);
		env_set("boot_targets", env);
	}

	return 0;
}

int mmc_get_env_dev(void)
{
	const char *boot_device =
		ofnode_read_chosen_string("u-boot,spl-boot-device");
	struct udevice *devp;

	if (!boot_device) {
		debug("%s: /chosen/u-boot,spl-boot-device not set\n",
		      __func__);
#ifdef CONFIG_SYS_MMC_ENV_DEV
		return CONFIG_SYS_MMC_ENV_DEV;
#else
		return 0;
#endif
	}

	debug("%s: booted from %s\n", __func__, boot_device);

	if (uclass_find_device_by_ofnode(UCLASS_MMC, ofnode_path(boot_device), &devp))
#ifdef CONFIG_SYS_MMC_ENV_DEV
		return CONFIG_SYS_MMC_ENV_DEV;
#else
		return 0;
#endif

	debug("%s: get MMC ENV from mmc%d\n", __func__, devp->seq_);

	return devp->seq_;
}

enum env_location arch_env_get_location(enum env_operation op, int prio)
{
	const char *boot_device =
		ofnode_read_chosen_string("u-boot,spl-boot-device");
	struct udevice *devp;

	if (prio > 0)
		return ENVL_UNKNOWN;

	if (!boot_device) {
		debug("%s: /chosen/u-boot,spl-boot-device not set\n",
		      __func__);
		return ENVL_NOWHERE;
	}

	debug("%s: booted from %s\n", __func__, boot_device);

	if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH) &&
	    !uclass_find_device_by_ofnode(UCLASS_SPI_FLASH, ofnode_path(boot_device), &devp))
		return ENVL_SPI_FLASH;

	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC) &&
	    !uclass_find_device_by_ofnode(UCLASS_MMC, ofnode_path(boot_device), &devp))
		return ENVL_MMC;

	printf("%s: No environment available: booted from %s but U-Boot config does not allow loading environment from it.",
	       __func__, boot_device);

	return ENVL_NOWHERE;
}
