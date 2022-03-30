/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 */

#ifndef __DM_UTIL_H
#define __DM_UTIL_H

#ifdef CONFIG_DM_WARN
void dm_warn(const char *fmt, ...);
#else
static inline void dm_warn(const char *fmt, ...)
{
}
#endif

struct list_head;

/**
 * list_count_items() - Count number of items in a list
 *
 * @param head:		Head of list
 * @return number of items, or 0 if empty
 */
int list_count_items(struct list_head *head);

/* Dump out a tree of all devices */
void dm_dump_all(void);

/* Dump out a list of uclasses and their devices */
void dm_dump_uclass(void);

#ifdef CONFIG_DEBUG_DEVRES
/* Dump out a list of device resources */
void dm_dump_devres(void);
#else
static inline void dm_dump_devres(void)
{
}
#endif

/**
 * Check if an of node should be or was bound before relocation.
 *
 * Devicetree nodes can be marked as needed to be bound
 * in the loader stages via special devicetree properties.
 *
 * Before relocation this function can be used to check if nodes
 * are required in either SPL or TPL stages.
 *
 * After relocation and jumping into the real U-Boot binary
 * it is possible to determine if a node was bound in one of
 * SPL/TPL stages.
 *
 * There are 3 settings currently in use
 * -
 * - u-boot,dm-pre-reloc: legacy and indicates any of TPL or SPL
 *   Existing platforms only use it to indicate nodes needed in
 *   SPL. Should probably be replaced by u-boot,dm-spl for
 *   existing platforms.
 * @node: of node
 *
 * Returns true if node is needed in SPL/TL, false otherwise.
 */
bool dm_ofnode_pre_reloc(ofnode node);

#endif
