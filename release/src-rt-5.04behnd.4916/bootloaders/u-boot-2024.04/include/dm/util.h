/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 */

#ifndef __DM_UTIL_H
#define __DM_UTIL_H

struct dm_stats;

#if CONFIG_IS_ENABLED(DM_WARN)
#define dm_warn(fmt...) log(LOGC_DM, LOGL_WARNING, ##fmt)
#else
#define dm_warn(fmt...) log(LOGC_DM, LOGL_DEBUG, ##fmt)
#endif

struct list_head;

/**
 * list_count_items() - Count number of items in a list
 *
 * @param head:		Head of list
 * Return: number of items, or 0 if empty
 */
int list_count_items(struct list_head *head);

/**
 * Dump out a tree of all devices starting @uclass
 *
 * @dev_name: udevice name
 * @extended: true if forword-matching expected
 * @sort: Sort by uclass name
 */
void dm_dump_tree(char *dev_name, bool extended, bool sort);

/*
 * Dump out a list of uclasses and their devices
 *
 * @uclass: uclass name
 * @extended: true if forword-matching expected
 */
void dm_dump_uclass(char *uclass, bool extended);

#ifdef CONFIG_DEBUG_DEVRES
/* Dump out a list of device resources */
void dm_dump_devres(void);
#else
static inline void dm_dump_devres(void)
{
}
#endif

/* Dump out a list of drivers */
void dm_dump_drivers(void);

/* Dump out a list with each driver's compatibility strings */
void dm_dump_driver_compat(void);

/* Dump out a list of drivers with static platform data */
void dm_dump_static_driver_info(void);

/**
 * dm_dump_mem() - Dump stats on memory usage in driver model
 *
 * @mem: Stats to dump
 */
void dm_dump_mem(struct dm_stats *stats);

#if CONFIG_IS_ENABLED(OF_PLATDATA_INST) && CONFIG_IS_ENABLED(READ_ONLY)
void *dm_priv_to_rw(void *priv);
#else
static inline void *dm_priv_to_rw(void *priv)
{
	return priv;
}
#endif

#endif
