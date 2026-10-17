// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <env.h>
#include <env_internal.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/bug.h>

DECLARE_GLOBAL_DATA_PTR;

static struct env_driver *_env_driver_lookup(enum env_location loc)
{
	struct env_driver *drv;
	const int n_ents = ll_entry_count(struct env_driver, env_driver);
	struct env_driver *entry;

	drv = ll_entry_start(struct env_driver, env_driver);
	for (entry = drv; entry != drv + n_ents; entry++) {
		if (loc == entry->location)
			return entry;
	}

	/* Not found */
	return NULL;
}

static enum env_location env_locations[] = {
#ifdef CONFIG_ENV_IS_IN_EEPROM
	ENVL_EEPROM,
#endif
#ifdef CONFIG_ENV_IS_IN_EXT4
	ENVL_EXT4,
#endif
#ifdef CONFIG_ENV_IS_IN_FAT
	ENVL_FAT,
#endif
#ifdef CONFIG_ENV_IS_IN_FLASH
	ENVL_FLASH,
#endif
#ifdef CONFIG_ENV_IS_IN_MMC
	ENVL_MMC,
#endif
#ifdef CONFIG_ENV_IS_IN_NAND
	ENVL_NAND,
#endif
#ifdef CONFIG_ENV_IS_IN_NVRAM
	ENVL_NVRAM,
#endif
#ifdef CONFIG_ENV_IS_IN_REMOTE
	ENVL_REMOTE,
#endif
#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	ENVL_SPI_FLASH,
#endif
#ifdef CONFIG_ENV_IS_IN_BOOT_MAGIC
	ENVL_BOOT_MAGIC,
#endif
#ifdef CONFIG_ENV_IS_IN_VFBIO
	ENVL_VFBIO,
#endif
#ifdef CONFIG_ENV_IS_IN_UBI
	ENVL_UBI,
#endif
#ifdef CONFIG_ENV_IS_NOWHERE
	ENVL_NOWHERE,
#endif
};

static bool env_has_inited(enum env_location location)
{
	return gd->env_has_init & BIT(location);
}

static void env_set_inited(enum env_location location)
{
	/*
	 * We're using a 32-bits bitmask stored in gd (env_has_init)
	 * using the above enum value as the bit index. We need to
	 * make sure that we're not overflowing it.
	 */
	BUILD_BUG_ON(ENVL_COUNT > BITS_PER_LONG);

	gd->env_has_init |= BIT(location);
}

/**
 * arch_env_get_location() - Returns the best env location for an arch
 * @op: operations performed on the environment
 * @prio: priority between the multiple environments, 0 being the
 *        highest priority
 *
 * This will return the preferred environment for the given priority.
 * This is overridable by architectures if they need to and has lower
 * priority than board side env_get_location() override.
 *
 * All implementations are free to use the operation, the priority and
 * any other data relevant to their choice, but must take into account
 * the fact that the lowest prority (0) is the most important location
 * in the system. The following locations should be returned by order
 * of descending priorities, from the highest to the lowest priority.
 *
 * Returns:
 * an enum env_location value on success, a negative error code otherwise
 */
__weak enum env_location arch_env_get_location(enum env_operation op, int prio)
{
	if (prio >= ARRAY_SIZE(env_locations))
		return ENVL_UNKNOWN;

	return env_locations[prio];
}

/**
 * env_get_location() - Returns the best env location for a board
 * @op: operations performed on the environment
 * @prio: priority between the multiple environments, 0 being the
 *        highest priority
 *
 * This will return the preferred environment for the given priority.
 * This is overridable by boards if they need to.
 *
 * All implementations are free to use the operation, the priority and
 * any other data relevant to their choice, but must take into account
 * the fact that the lowest prority (0) is the most important location
 * in the system. The following locations should be returned by order
 * of descending priorities, from the highest to the lowest priority.
 *
 * Returns:
 * an enum env_location value on success, a negative error code otherwise
 */
__weak enum env_location env_get_location(enum env_operation op, int prio)
{
	return arch_env_get_location(op, prio);
}

/**
 * env_driver_lookup() - Finds the most suited environment location
 * @op: operations performed on the environment
 * @prio: priority between the multiple environments, 0 being the
 *        highest priority
 *
 * This will try to find the available environment with the highest
 * priority in the system.
 *
 * Returns:
 * NULL on error, a pointer to a struct env_driver otherwise
 */
static struct env_driver *env_driver_lookup(enum env_operation op, int prio)
{
	enum env_location loc = env_get_location(op, prio);
	struct env_driver *drv;

	if (loc == ENVL_UNKNOWN)
		return NULL;

	drv = _env_driver_lookup(loc);
	if (!drv) {
		debug("%s: No environment driver for location %d\n", __func__,
		      loc);
		return NULL;
	}

	return drv;
}

int env_load(void)
{
	struct env_driver *drv;
	int best_prio = -1;
	int prio;

	if (CONFIG_IS_ENABLED(ENV_WRITEABLE_LIST)) {
		/*
		 * When using a list of writeable variables, the baseline comes
		 * from the built-in default env. So load this first.
		 */
		env_set_default(NULL, 0);
	}

	for (prio = 0; (drv = env_driver_lookup(ENVOP_LOAD, prio)); prio++) {
		int ret;

		if (!env_has_inited(drv->location))
			continue;

		printf("Loading Environment from %s... ", drv->name);
		/*
		 * In error case, the error message must be printed during
		 * drv->load() in some underlying API, and it must be exactly
		 * one message.
		 */
		ret = drv->load();
		if (!ret) {
			printf("OK\n");
			gd->env_load_prio = prio;

			return 0;
		} else if (ret == -ENOMSG) {
			/* Handle "bad CRC" case */
			if (best_prio == -1)
				best_prio = prio;
		} else {
			debug("Failed (%d)\n", ret);
		}
	}

	/*
	 * In case of invalid environment, we set the 'default' env location
	 * to the best choice, i.e.:
	 *   1. Environment location with bad CRC, if such location was found
	 *   2. Otherwise use the location with highest priority
	 *
	 * This way, next calls to env_save() will restore the environment
	 * at the right place.
	 */
	if (best_prio >= 0)
		debug("Selecting environment with bad CRC\n");
	else
		best_prio = 0;

	gd->env_load_prio = best_prio;

	return -ENODEV;
}

int env_reload(void)
{
	struct env_driver *drv;

	drv = env_driver_lookup(ENVOP_LOAD, gd->env_load_prio);
	if (drv) {
		int ret;

		printf("Loading Environment from %s... ", drv->name);

		if (!env_has_inited(drv->location)) {
			printf("not initialized\n");
			return -ENODEV;
		}

		ret = drv->load();
		if (ret)
			printf("Failed (%d)\n", ret);
		else
			printf("OK\n");

		if (!ret)
			return 0;
	}

	return -ENODEV;
}

int env_save(void)
{
	struct env_driver *drv;

	drv = env_driver_lookup(ENVOP_SAVE, gd->env_load_prio);
	if (drv) {
		int ret;

		printf("Saving Environment to %s... ", drv->name);
		if (!drv->save) {
			printf("not possible\n");
			return -ENODEV;
		}

		if (!env_has_inited(drv->location)) {
			printf("not initialized\n");
			return -ENODEV;
		}

		ret = drv->save();
		if (ret)
			printf("Failed (%d)\n", ret);
		else
			printf("OK\n");

		if (!ret)
			return 0;
	}

	return -ENODEV;
}

int env_erase(void)
{
	struct env_driver *drv;

	drv = env_driver_lookup(ENVOP_ERASE, gd->env_load_prio);
	if (drv) {
		int ret;

		if (!drv->erase) {
			printf("not possible\n");
			return -ENODEV;
		}

		if (!env_has_inited(drv->location)) {
			printf("not initialized\n");
			return -ENODEV;
		}

		printf("Erasing Environment on %s... ", drv->name);
		ret = drv->erase();
		if (ret)
			printf("Failed (%d)\n", ret);
		else
			printf("OK\n");

		if (!ret)
			return 0;
	}

	return -ENODEV;
}

int env_init(void)
{
	struct env_driver *drv;
	int ret = -ENOENT;
	int prio;

	for (prio = 0; (drv = env_driver_lookup(ENVOP_INIT, prio)); prio++) {
		if (!drv->init || !(ret = drv->init()))
			env_set_inited(drv->location);
		if (ret == -ENOENT)
			env_set_inited(drv->location);

		debug("%s: Environment %s init done (ret=%d)\n", __func__,
		      drv->name, ret);

		if (gd->env_valid == ENV_INVALID)
			ret = -ENOENT;
	}

	if (!prio)
		return -ENODEV;

	if (ret == -ENOENT) {
		gd->env_addr = (ulong)&default_environment[0];
		gd->env_valid = ENV_VALID;

		return 0;
	}

	return ret;
}

int env_select(const char *name)
{
	struct env_driver *drv;
	const int n_ents = ll_entry_count(struct env_driver, env_driver);
	struct env_driver *entry;
	int prio;
	bool found = false;

	printf("Select Environment on %s: ", name);

	/* search ENV driver by name */
	drv = ll_entry_start(struct env_driver, env_driver);
	for (entry = drv; entry != drv + n_ents; entry++) {
		if (!strcmp(entry->name, name)) {
			found = true;
			break;
		}
	}

	if (!found) {
		printf("driver not found\n");
		return -ENODEV;
	}

	/* search priority by driver */
	for (prio = 0; (drv = env_driver_lookup(ENVOP_INIT, prio)); prio++) {
		if (entry->location == env_get_location(ENVOP_LOAD, prio)) {
			/* when priority change, reset the ENV flags */
			if (gd->env_load_prio != prio) {
				gd->env_load_prio = prio;
				gd->env_valid = ENV_INVALID;
				gd->flags &= ~GD_FLG_ENV_DEFAULT;
			}
			printf("OK\n");
			return 0;
		}
	}
	printf("priority not found\n");

	return -ENODEV;
}
