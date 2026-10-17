/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#ifndef __MENU_H__
#define __MENU_H__

struct cli_ch_state;
struct menu;

struct menu *menu_create(char *title, int timeout, int prompt,
				void (*display_statusline)(struct menu *),
				void (*item_data_print)(void *),
				char *(*item_choice)(void *),
				void *item_choice_data);
int menu_default_set(struct menu *m, char *item_key);
int menu_get_choice(struct menu *m, void **choice);
int menu_item_add(struct menu *m, char *item_key, void *item_data);
int menu_destroy(struct menu *m);
int menu_default_choice(struct menu *m, void **choice);

/**
 * menu_show() Show a boot menu
 *
 * This shows a menu and lets the user select an option. The menu is defined by
 * environment variables (see README.bootmenu).
 *
 * This function doesn't normally return, but if the users requests the command
 * problem, it will.
 *
 * @bootdelay: Delay to wait before running the default menu option (0 to run
 *		the entry immediately)
 * Return: If it returns, it always returns -1 to indicate that the boot should
 *	be aborted and the command prompt should be provided
 */
int menu_show(int bootdelay);

struct bootmenu_data {
	int delay;			/* delay for autoboot */
	int active;			/* active menu entry */
	int count;			/* total count of menu entries */
	struct bootmenu_entry *first;	/* first menu entry */
};

/** enum bootmenu_key - keys that can be returned by the bootmenu */
enum bootmenu_key {
	BKEY_NONE = 0,
	BKEY_UP,
	BKEY_DOWN,
	BKEY_SELECT,
	BKEY_QUIT,
	BKEY_SAVE,

	/* 'extra' keys, which are used by menus but not cedit */
	BKEY_PLUS,
	BKEY_MINUS,
	BKEY_SPACE,

	BKEY_COUNT,

	/* Keys from here on are not used by cedit */
	BKEY_FIRST_EXTRA = BKEY_PLUS,
};

/**
 * bootmenu_autoboot_loop() - handle autobooting if no key is pressed
 *
 * This shows a prompt to allow the user to press a key to interrupt auto boot
 * of the first menu option.
 *
 * It then waits for the required time (menu->delay in seconds) for a key to be
 * pressed. If nothing is pressed in that time, @key returns KEY_SELECT
 * indicating that the current option should be chosen.
 *
 * @menu: Menu being processed
 * @esc: Set to 1 if the escape key is pressed, otherwise not updated
 * Returns: code for the key the user pressed:
 *	enter: KEY_SELECT
 *	Ctrl-C: KEY_QUIT
 *	anything else: KEY_NONE
 */
enum bootmenu_key bootmenu_autoboot_loop(struct bootmenu_data *menu,
					 struct cli_ch_state *cch);

/**
 * bootmenu_loop() - handle waiting for a keypress when autoboot is disabled
 *
 * This is used when the menu delay is negative, indicating that the delay has
 * elapsed, or there was no delay to begin with.
 *
 * It reads a character and processes it, returning a menu-key code if a
 * character is recognised
 *
 * @menu: Menu being processed
 * @esc: On input, a non-zero value indicates that an escape sequence has
 *	resulted in that many characters so far. On exit this is updated to the
 *	new number of characters
 * Returns: code for the key the user pressed:
 *	enter: BKEY_SELECT
 *	Ctrl-C: BKEY_QUIT
 *	Up arrow: BKEY_UP
 *	Down arrow: BKEY_DOWN
 *	Escape (by itself): BKEY_QUIT
 *	Plus: BKEY_PLUS
 *	Minus: BKEY_MINUS
 *	Space: BKEY_SPACE
 */
enum bootmenu_key bootmenu_loop(struct bootmenu_data *menu,
				struct cli_ch_state *cch);

/**
 * bootmenu_conv_key() - Convert a U-Boot keypress into a menu key
 *
 * @ichar: Keypress to convert (ASCII, including control characters)
 * Returns: Menu key that corresponds to @ichar, or BKEY_NONE if none
 */
enum bootmenu_key bootmenu_conv_key(int ichar);

#endif /* __MENU_H__ */
