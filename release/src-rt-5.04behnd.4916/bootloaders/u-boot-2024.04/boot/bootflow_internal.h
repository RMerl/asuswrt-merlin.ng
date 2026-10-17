/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Internal header file for bootflow
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __BOOTFLOW_INTERNAL_H
#define __BOOTFLOW_INTERNAL_H

/* expo IDs for elements of the bootflow menu */
enum {
	START,

	/* strings */
	STR_PROMPT,
	STR_MENU_TITLE,
	STR_POINTER,

	/* scene */
	MAIN,

	/* objects */
	OBJ_U_BOOT_LOGO,
	OBJ_MENU,
	OBJ_PROMPT,
	OBJ_MENU_TITLE,
	OBJ_POINTER,

	/* strings for menu items */
	STR_LABEL = 100,
	STR_DESC = 200,
	STR_KEY = 300,

	/* menu items / components (bootflow number is added to these) */
	ITEM = 400,
	ITEM_LABEL = 500,
	ITEM_DESC = 600,
	ITEM_KEY = 700,
	ITEM_PREVIEW = 800,

	/* left margin for the main menu */
	MARGIN_LEFT	 = 100,
};

#endif /* __BOOTFLOW_INTERNAL_H */
