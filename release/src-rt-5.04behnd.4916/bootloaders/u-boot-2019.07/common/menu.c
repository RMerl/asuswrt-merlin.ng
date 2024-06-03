// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#include <common.h>
#include <cli.h>
#include <malloc.h>
#include <errno.h>
#include <linux/list.h>

#include "menu.h"

/*
 * Internally, each item in a menu is represented by a struct menu_item.
 *
 * These items will be alloc'd and initialized by menu_item_add and destroyed
 * by menu_item_destroy, and the consumer of the interface never sees that
 * this struct is used at all.
 */
struct menu_item {
	char *key;
	void *data;
	struct list_head list;
};

/*
 * The menu is composed of a list of items along with settings and callbacks
 * provided by the user. An incomplete definition of this struct is available
 * in menu.h, but the full definition is here to prevent consumers from
 * relying on its contents.
 */
struct menu {
	struct menu_item *default_item;
	int timeout;
	char *title;
	int prompt;
	void (*item_data_print)(void *);
	char *(*item_choice)(void *);
	void *item_choice_data;
	struct list_head items;
};

/*
 * An iterator function for menu items. callback will be called for each item
 * in m, with m, a pointer to the item, and extra being passed to callback. If
 * callback returns a value other than NULL, iteration stops and the value
 * return by callback is returned from menu_items_iter.  This allows it to be
 * used for search type operations. It is also safe for callback to remove the
 * item from the list of items.
 */
static inline void *menu_items_iter(struct menu *m,
		void *(*callback)(struct menu *, struct menu_item *, void *),
		void *extra)
{
	struct list_head *pos, *n;
	struct menu_item *item;
	void *ret;

	list_for_each_safe(pos, n, &m->items) {
		item = list_entry(pos, struct menu_item, list);

		ret = callback(m, item, extra);

		if (ret)
			return ret;
	}

	return NULL;
}

/*
 * Print a menu_item. If the consumer provided an item_data_print function
 * when creating the menu, call it with a pointer to the item's private data.
 * Otherwise, print the key of the item.
 */
static inline void *menu_item_print(struct menu *m,
				struct menu_item *item,
				void *extra)
{
	if (!m->item_data_print) {
		puts(item->key);
		putc('\n');
	} else {
		m->item_data_print(item->data);
	}

	return NULL;
}

/*
 * Free the memory used by a menu item. This includes the memory used by its
 * key.
 */
static inline void *menu_item_destroy(struct menu *m,
				struct menu_item *item,
				void *extra)
{
	if (item->key)
		free(item->key);

	free(item);

	return NULL;
}

__weak void menu_display_statusline(struct menu *m)
{
}

/*
 * Display a menu so the user can make a choice of an item. First display its
 * title, if any, and then each item in the menu.
 */
static inline void menu_display(struct menu *m)
{
	if (m->title) {
		puts(m->title);
		putc('\n');
	}
	menu_display_statusline(m);

	menu_items_iter(m, menu_item_print, NULL);
}

/*
 * Check if an item's key matches a provided string, pointed to by extra. If
 * extra is NULL, an item with a NULL key will match. Otherwise, the item's
 * key has to match according to strcmp.
 *
 * This is called via menu_items_iter, so it returns a pointer to the item if
 * the key matches, and returns NULL otherwise.
 */
static inline void *menu_item_key_match(struct menu *m,
			struct menu_item *item, void *extra)
{
	char *item_key = extra;

	if (!item_key || !item->key) {
		if (item_key == item->key)
			return item;

		return NULL;
	}

	if (strcmp(item->key, item_key) == 0)
		return item;

	return NULL;
}

/*
 * Find the first item with a key matching item_key, if any exists.
 */
static inline struct menu_item *menu_item_by_key(struct menu *m,
							char *item_key)
{
	return menu_items_iter(m, menu_item_key_match, item_key);
}

/*
 * Set *choice to point to the default item's data, if any default item was
 * set, and returns 1. If no default item was set, returns -ENOENT.
 */
int menu_default_choice(struct menu *m, void **choice)
{
	if (m->default_item) {
		*choice = m->default_item->data;
		return 1;
	}

	return -ENOENT;
}

/*
 * Displays the menu and asks the user to choose an item. *choice will point
 * to the private data of the item the user chooses. The user makes a choice
 * by inputting a string matching the key of an item. Invalid choices will
 * cause the user to be prompted again, repeatedly, until the user makes a
 * valid choice. The user can exit the menu without making a choice via ^c.
 *
 * Returns 1 if the user made a choice, or -EINTR if they bail via ^c.
 */
static inline int menu_interactive_choice(struct menu *m, void **choice)
{
	char cbuf[CONFIG_SYS_CBSIZE];
	struct menu_item *choice_item = NULL;
	int readret;

	while (!choice_item) {
		cbuf[0] = '\0';

		menu_display(m);

		if (!m->item_choice) {
			readret = cli_readline_into_buffer("Enter choice: ",
							   cbuf, m->timeout);

			if (readret >= 0) {
				choice_item = menu_item_by_key(m, cbuf);
				if (!choice_item)
					printf("%s not found\n", cbuf);
			} else if (readret == -1)  {
				printf("<INTERRUPT>\n");
				return -EINTR;
			} else {
				return menu_default_choice(m, choice);
			}
		} else {
			char *key = m->item_choice(m->item_choice_data);

			if (key)
				choice_item = menu_item_by_key(m, key);
		}

		if (!choice_item)
			m->timeout = 0;
	}

	*choice = choice_item->data;

	return 1;
}

/*
 * menu_default_set() - Sets the default choice for the menu. This is safe to
 * call more than once on a menu.
 *
 * m - Points to a menu created by menu_create().
 *
 * item_key - Points to a string that, when compared using strcmp, matches the
 * key for an existing item in the menu.
 *
 * Returns 1 if successful, -EINVAL if m is NULL, or -ENOENT if no item with a
 * key matching item_key is found.
 */
int menu_default_set(struct menu *m, char *item_key)
{
	struct menu_item *item;

	if (!m)
		return -EINVAL;

	item = menu_item_by_key(m, item_key);

	if (!item)
		return -ENOENT;

	m->default_item = item;

	return 1;
}

/*
 * menu_get_choice() - Returns the user's selected menu entry, or the default
 * if the menu is set to not prompt or the timeout expires. This is safe to
 * call more than once.
 *
 * m - Points to a menu created by menu_create().
 *
 * choice - Points to a location that will store a pointer to the selected
 * menu item. If no item is selected or there is an error, no value will be
 * written at the location it points to.
 *
 * Returns 1 if successful, -EINVAL if m or choice is NULL, -ENOENT if no
 * default has been set and the menu is set to not prompt or the timeout
 * expires, or -EINTR if the user exits the menu via ^c.
 */
int menu_get_choice(struct menu *m, void **choice)
{
	if (!m || !choice)
		return -EINVAL;

	if (!m->prompt)
		return menu_default_choice(m, choice);

	return menu_interactive_choice(m, choice);
}

/*
 * menu_item_add() - Adds or replaces a menu item. Note that this replaces the
 * data of an item if it already exists, but doesn't change the order of the
 * item.
 *
 * m - Points to a menu created by menu_create().
 *
 * item_key - Points to a string that will uniquely identify the item.  The
 * string will be copied to internal storage, and is safe to discard after
 * passing to menu_item_add.
 *
 * item_data - An opaque pointer associated with an item. It is never
 * dereferenced internally, but will be passed to the item_data_print, and
 * will be returned from menu_get_choice if the menu item is selected.
 *
 * Returns 1 if successful, -EINVAL if m is NULL, or -ENOMEM if there is
 * insufficient memory to add the menu item.
 */
int menu_item_add(struct menu *m, char *item_key, void *item_data)
{
	struct menu_item *item;

	if (!m)
		return -EINVAL;

	item = menu_item_by_key(m, item_key);

	if (item) {
		item->data = item_data;
		return 1;
	}

	item = malloc(sizeof *item);
	if (!item)
		return -ENOMEM;

	item->key = strdup(item_key);

	if (!item->key) {
		free(item);
		return -ENOMEM;
	}

	item->data = item_data;

	list_add_tail(&item->list, &m->items);

	return 1;
}

/*
 * menu_create() - Creates a menu handle with default settings
 *
 * title - If not NULL, points to a string that will be displayed before the
 * list of menu items. It will be copied to internal storage, and is safe to
 * discard after passing to menu_create().
 *
 * timeout - A delay in seconds to wait for user input. If 0, timeout is
 * disabled, and the default choice will be returned unless prompt is 1.
 *
 * prompt - If 0, don't ask for user input unless there is an interrupted
 * timeout. If 1, the user will be prompted for input regardless of the value
 * of timeout.
 *
 * item_data_print - If not NULL, will be called for each item when the menu
 * is displayed, with the pointer to the item's data passed as the argument.
 * If NULL, each item's key will be printed instead.  Since an item's key is
 * what must be entered to select an item, the item_data_print function should
 * make it obvious what the key for each entry is.
 *
 * item_choice - If not NULL, will be called when asking the user to choose an
 * item. Returns a key string corresponding to the chosen item or NULL if
 * no item has been selected.
 *
 * item_choice_data - Will be passed as the argument to the item_choice function
 *
 * Returns a pointer to the menu if successful, or NULL if there is
 * insufficient memory available to create the menu.
 */
struct menu *menu_create(char *title, int timeout, int prompt,
				void (*item_data_print)(void *),
				char *(*item_choice)(void *),
				void *item_choice_data)
{
	struct menu *m;

	m = malloc(sizeof *m);

	if (!m)
		return NULL;

	m->default_item = NULL;
	m->prompt = prompt;
	m->timeout = timeout;
	m->item_data_print = item_data_print;
	m->item_choice = item_choice;
	m->item_choice_data = item_choice_data;

	if (title) {
		m->title = strdup(title);
		if (!m->title) {
			free(m);
			return NULL;
		}
	} else
		m->title = NULL;


	INIT_LIST_HEAD(&m->items);

	return m;
}

/*
 * menu_destroy() - frees the memory used by a menu and its items.
 *
 * m - Points to a menu created by menu_create().
 *
 * Returns 1 if successful, or -EINVAL if m is NULL.
 */
int menu_destroy(struct menu *m)
{
	if (!m)
		return -EINVAL;

	menu_items_iter(m, menu_item_destroy, NULL);

	if (m->title)
		free(m->title);

	free(m);

	return 1;
}
