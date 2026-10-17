/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#ifndef __EXTENSION_SUPPORT_H
#define __EXTENSION_SUPPORT_H

struct extension {
	struct list_head list;
	char name[32];
	char owner[32];
	char version[32];
	char overlay[32];
	char other[32];
};

/**
 * extension_board_scan - Add system-specific function to scan extension board.
 * @param extension_list	List of extension board information to update.
 * Return: the number of extension.
 *
 * This function is called if CONFIG_CMD_EXTENSION is defined.
 * Needs to fill the list extension_list with elements.
 * Each element need to be allocated to an extension structure.
 *
 */
int extension_board_scan(struct list_head *extension_list);

#endif /* __EXTENSION_SUPPORT_H */
