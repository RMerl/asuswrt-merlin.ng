/*
 * card_select.c - select a card by list or device name
 * Copyright (c) Clemens Ladisch <clemens@ladisch.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "aconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <alsa/asoundlib.h>
#include <menu.h>
#include "gettext_curses.h"
#include "die.h"
#include "mem.h"
#include "utils.h"
#include "colors.h"
#include "widget.h"
#include "mixer_widget.h"
#include "device_name.h"
#include "card_select.h"

struct card {
	struct card *next;
	char *indexstr;
	char *name;
	char *device_name;
};

static struct widget list_widget;
static struct card first_card;
static ITEM **items;
static MENU *menu;
static ITEM *initial_item;

static void on_key_enter(void)
{
	ITEM *item = current_item(menu);
	if (item) {
		struct card *card = item_userptr(item);
		if (card->device_name) {
			if (select_card_by_name(card->device_name))
				list_widget.close();
		} else {
			create_device_name_form();
		}
	}
}

static void on_menu_key(int key)
{
	static const struct {
		int key;
		int request;
	} key_map[] = {
		{ KEY_DOWN, REQ_DOWN_ITEM },
		{ KEY_UP, REQ_UP_ITEM },
		{ KEY_HOME, REQ_FIRST_ITEM },
		{ KEY_NPAGE, REQ_SCR_DPAGE },
		{ KEY_PPAGE, REQ_SCR_UPAGE },
		{ KEY_BEG, REQ_FIRST_ITEM },
		{ KEY_END, REQ_LAST_ITEM },
	};
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(key_map); ++i)
		if (key_map[i].key == key) {
			menu_driver(menu, key_map[i].request);
			break;
		}
}

static void on_handle_key(int key)
{
	switch (key) {
	case 27:
	case KEY_CANCEL:
	case 'q':
	case 'Q':
		list_widget.close();
		break;
	case 10:
	case 13:
	case KEY_ENTER:
		on_key_enter();
		break;
	default:
		on_menu_key(key);
		break;
	}
}

static bool create(void)
{
	int rows, columns;
	const char *title;

	if (screen_lines < 3 || screen_cols < 10) {
		beep();
		list_widget.close();
		return FALSE;
	}
	scale_menu(menu, &rows, &columns);
	rows += 2;
	columns += 2;
	if (rows > screen_lines)
		rows = screen_lines;
	if (columns > screen_cols)
		columns = screen_cols;

	widget_init(&list_widget, rows, columns, SCREEN_CENTER, SCREEN_CENTER,
		    attr_menu, WIDGET_BORDER | WIDGET_SUBWINDOW);

	title = _("Sound Card");
	mvwprintw(list_widget.window, 0, (columns - 2 - get_mbs_width(title)) / 2, " %s ", title);
	set_menu_win(menu, list_widget.window);
	set_menu_sub(menu, list_widget.subwindow);
	return TRUE;
}

static void on_window_size_changed(void)
{
	unpost_menu(menu);
	if (!create())
		return;
	post_menu(menu);
}

static void on_close(void)
{
	unsigned int i;
	struct card *card, *next_card;

	unpost_menu(menu);
	free_menu(menu);
	for (i = 0; items[i]; ++i)
		free_item(items[i]);
	free(items);
	for (card = first_card.next; card; card = next_card) {
		next_card = card->next;
		free(card->indexstr);
		free(card->name);
		free(card->device_name);
		free(card);
	}
	widget_free(&list_widget);
}

void close_card_select_list(void)
{
	on_close();
}

static struct widget list_widget = {
	.handle_key = on_handle_key,
	.window_size_changed = on_window_size_changed,
	.close = on_close,
};

static int get_cards(void)
{
	int count, number, err;
	snd_ctl_t *ctl;
	snd_ctl_card_info_t *info;
	char buf[16];
	struct card *card, *prev_card;

	first_card.indexstr = "-";
	first_card.name = _("(default)");
	first_card.device_name = "default";
	count = 1;

	snd_ctl_card_info_alloca(&info);
	prev_card = &first_card;
	number = -1;
	for (;;) {
		err = snd_card_next(&number);
		if (err < 0)
			fatal_alsa_error(_("cannot enumerate sound cards"), err);
		if (number < 0)
			break;
		sprintf(buf, "hw:%d", number);
		err = snd_ctl_open(&ctl, buf, 0);
		if (err < 0)
			continue;
		err = snd_ctl_card_info(ctl, info);
		snd_ctl_close(ctl);
		if (err < 0)
			continue;
		card = ccalloc(1, sizeof *card);
		sprintf(buf, "%d", number);
		card->indexstr = cstrdup(buf);
		card->name = cstrdup(snd_ctl_card_info_get_name(info));
		sprintf(buf, "hw:%d", number);
		card->device_name = cstrdup(buf);
		prev_card->next = card;
		prev_card = card;
		++count;
	}

	card = ccalloc(1, sizeof *card);
	card->indexstr = cstrdup(" ");
	card->name = cstrdup(_("enter device name..."));
	prev_card->next = card;
	++count;

	return count;
}

static void create_list_items(int cards)
{
	int i;
	struct card *card;
	ITEM *item;

	initial_item = NULL;
	items = ccalloc(cards + 1, sizeof(ITEM*));
	i = 0;
	for (card = &first_card; card; card = card->next) {
		item = new_item(card->indexstr, card->name);
		if (!item)
			fatal_error("cannot create menu item");
		set_item_userptr(item, card);
		items[i++] = item;
		if (!initial_item &&
		    mixer_device_name &&
		    (!card->device_name ||
		     !strcmp(card->device_name, mixer_device_name)))
			initial_item = item;
	}
	assert(i == cards);
}

void create_card_select_list(void)
{
	int cards;

	cards = get_cards();
	create_list_items(cards);

	menu = new_menu(items);
	if (!menu)
		fatal_error("cannot create menu");
	set_menu_fore(menu, attr_menu_selected);
	set_menu_back(menu, attr_menu);
	set_menu_mark(menu, NULL);
	if (initial_item)
		set_current_item(menu, initial_item);
	set_menu_spacing(menu, 2, 1, 1);
	menu_opts_on(menu, O_SHOWDESC);

	if (!create())
		return;

	post_menu(menu);
}
