/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#ifndef __MENU_H__
#define __MENU_H__

struct menu;

struct menu *menu_create(char *title, int timeout, int prompt,
				void (*item_data_print)(void *),
				char *(*item_choice)(void *),
				void *item_choice_data);
int menu_default_set(struct menu *m, char *item_key);
int menu_get_choice(struct menu *m, void **choice);
int menu_item_add(struct menu *m, char *item_key, void *item_data);
int menu_destroy(struct menu *m);
void menu_display_statusline(struct menu *m);
int menu_default_choice(struct menu *m, void **choice);

#if defined(CONFIG_MENU_SHOW)
int menu_show(int bootdelay);
#endif
#endif /* __MENU_H__ */
