// SPDX-License-Identifier: GPL-2.0+
/*
 * Implementation of a menu in a scene
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EXPO

#include <common.h>
#include <dm.h>
#include <expo.h>
#include <malloc.h>
#include <mapmem.h>
#include <menu.h>
#include <video.h>
#include <video_console.h>
#include <linux/input.h>
#include "scene_internal.h"

static void scene_menuitem_destroy(struct scene_menitem *item)
{
	free(item->name);
	free(item);
}

void scene_menu_destroy(struct scene_obj_menu *menu)
{
	struct scene_menitem *item, *next;

	list_for_each_entry_safe(item, next, &menu->item_head, sibling)
		scene_menuitem_destroy(item);
}

struct scene_menitem *scene_menuitem_find(const struct scene_obj_menu *menu,
					  int id)
{
	struct scene_menitem *item;

	list_for_each_entry(item, &menu->item_head, sibling) {
		if (item->id == id)
			return item;
	}

	return NULL;
}

struct scene_menitem *scene_menuitem_find_seq(const struct scene_obj_menu *menu,
					      uint seq)
{
	struct scene_menitem *item;
	uint i;

	i = 0;
	list_for_each_entry(item, &menu->item_head, sibling) {
		if (i == seq)
			return item;
		i++;
	}

	return NULL;
}

/**
 * update_pointers() - Update the pointer object and handle highlights
 *
 * @menu: Menu to update
 * @id: ID of menu item to select/deselect
 * @point: true if @id is being selected, false if it is being deselected
 */
static int update_pointers(struct scene_obj_menu *menu, uint id, bool point)
{
	struct scene *scn = menu->obj.scene;
	const bool stack = scn->expo->popup;
	const struct scene_menitem *item;
	int ret;

	item = scene_menuitem_find(menu, id);
	if (!item)
		return log_msg_ret("itm", -ENOENT);

	/* adjust the pointer object to point to the selected item */
	if (menu->pointer_id && item && point) {
		struct scene_obj *label;

		label = scene_obj_find(scn, item->label_id, SCENEOBJT_NONE);

		ret = scene_obj_set_pos(scn, menu->pointer_id,
					menu->obj.dim.x + 200, label->dim.y);
		if (ret < 0)
			return log_msg_ret("ptr", ret);
	}

	if (stack) {
		point &= scn->highlight_id == menu->obj.id;
		scene_obj_flag_clrset(scn, item->label_id, SCENEOF_POINT,
				      point ? SCENEOF_POINT : 0);
	}

	return 0;
}

/**
 * menu_point_to_item() - Point to a particular menu item
 *
 * Sets the currently pointed-to / highlighted menu item
 */
static void menu_point_to_item(struct scene_obj_menu *menu, uint item_id)
{
	if (menu->cur_item_id)
		update_pointers(menu, menu->cur_item_id, false);
	menu->cur_item_id = item_id;
	update_pointers(menu, item_id, true);
}

void scene_menu_calc_bbox(struct scene_obj_menu *menu,
			  struct vidconsole_bbox *bbox,
			  struct vidconsole_bbox *label_bbox)
{
	const struct expo_theme *theme = &menu->obj.scene->expo->theme;
	const struct scene_menitem *item;

	bbox->valid = false;
	scene_bbox_union(menu->obj.scene, menu->title_id, 0, bbox);

	label_bbox->valid = false;

	list_for_each_entry(item, &menu->item_head, sibling) {
		scene_bbox_union(menu->obj.scene, item->label_id,
				 theme->menu_inset, bbox);
		scene_bbox_union(menu->obj.scene, item->key_id, 0, bbox);
		scene_bbox_union(menu->obj.scene, item->desc_id, 0, bbox);
		scene_bbox_union(menu->obj.scene, item->preview_id, 0, bbox);

		/* Get the bounding box of all labels */
		scene_bbox_union(menu->obj.scene, item->label_id,
				 theme->menu_inset, label_bbox);
	}

	/*
	 * subtract the final menuitem's gap to keep the insert the same top
	 * and bottom
	 */
	label_bbox->y1 -= theme->menuitem_gap_y;
}

int scene_menu_calc_dims(struct scene_obj_menu *menu)
{
	struct vidconsole_bbox bbox, label_bbox;
	const struct scene_menitem *item;

	scene_menu_calc_bbox(menu, &bbox, &label_bbox);

	/* Make all labels the same size */
	if (label_bbox.valid) {
		list_for_each_entry(item, &menu->item_head, sibling) {
			scene_obj_set_size(menu->obj.scene, item->label_id,
					   label_bbox.x1 - label_bbox.x0,
					   label_bbox.y1 - label_bbox.y0);
		}
	}

	if (bbox.valid) {
		menu->obj.dim.w = bbox.x1 - bbox.x0;
		menu->obj.dim.h = bbox.y1 - bbox.y0;
	}

	return 0;
}

int scene_menu_arrange(struct scene *scn, struct scene_obj_menu *menu)
{
	const bool open = menu->obj.flags & SCENEOF_OPEN;
	struct expo *exp = scn->expo;
	const bool stack = exp->popup;
	const struct expo_theme *theme = &exp->theme;
	struct scene_menitem *item;
	uint sel_id;
	int x, y;
	int ret;

	x = menu->obj.dim.x;
	y = menu->obj.dim.y;
	if (menu->title_id) {
		ret = scene_obj_set_pos(scn, menu->title_id, menu->obj.dim.x, y);
		if (ret < 0)
			return log_msg_ret("tit", ret);

		ret = scene_obj_get_hw(scn, menu->title_id, NULL);
		if (ret < 0)
			return log_msg_ret("hei", ret);

		if (stack)
			x += 200;
		else
			y += ret * 2;
	}

	/*
	 * Currently everything is hard-coded to particular columns so this
	 * won't work on small displays and looks strange if the font size is
	 * small. This can be updated once text measuring is supported in
	 * vidconsole
	 */
	sel_id = menu->cur_item_id;
	list_for_each_entry(item, &menu->item_head, sibling) {
		bool selected;
		int height;

		ret = scene_obj_get_hw(scn, item->label_id, NULL);
		if (ret < 0)
			return log_msg_ret("get", ret);
		height = ret;

		if (item->flags & SCENEMIF_GAP_BEFORE)
			y += height;

		/* select an item if not done already */
		if (!sel_id)
			sel_id = item->id;

		selected = sel_id == item->id;

		/*
		 * Put the label on the left, then leave a space for the
		 * pointer, then the key and the description
		 */
		ret = scene_obj_set_pos(scn, item->label_id,
					x + theme->menu_inset, y);
		if (ret < 0)
			return log_msg_ret("nam", ret);
		scene_obj_set_hide(scn, item->label_id,
				   stack && !open && !selected);

		if (item->key_id) {
			ret = scene_obj_set_pos(scn, item->key_id, x + 230, y);
			if (ret < 0)
				return log_msg_ret("key", ret);
		}

		if (item->desc_id) {
			ret = scene_obj_set_pos(scn, item->desc_id, x + 280, y);
			if (ret < 0)
				return log_msg_ret("des", ret);
		}

		if (item->preview_id) {
			bool hide;

			/*
			 * put all previews on top of each other, on the right
			 * size of the display
			 */
			ret = scene_obj_set_pos(scn, item->preview_id, -4, y);
			if (ret < 0)
				return log_msg_ret("prev", ret);

			hide = menu->cur_item_id != item->id;
			ret = scene_obj_set_hide(scn, item->preview_id, hide);
			if (ret < 0)
				return log_msg_ret("hid", ret);
		}

		if (!stack || open)
			y += height + theme->menuitem_gap_y;
	}

	if (sel_id)
		menu_point_to_item(menu, sel_id);

	return 0;
}

int scene_menu(struct scene *scn, const char *name, uint id,
	       struct scene_obj_menu **menup)
{
	struct scene_obj_menu *menu;
	int ret;

	ret = scene_obj_add(scn, name, id, SCENEOBJT_MENU,
			    sizeof(struct scene_obj_menu),
			    (struct scene_obj **)&menu);
	if (ret < 0)
		return log_msg_ret("obj", -ENOMEM);

	if (menup)
		*menup = menu;
	INIT_LIST_HEAD(&menu->item_head);

	return menu->obj.id;
}

static struct scene_menitem *scene_menu_find_key(struct scene *scn,
						  struct scene_obj_menu *menu,
						  int key)
{
	struct scene_menitem *item;

	list_for_each_entry(item, &menu->item_head, sibling) {
		if (item->key_id) {
			struct scene_obj_txt *txt;
			const char *str;

			txt = scene_obj_find(scn, item->key_id, SCENEOBJT_TEXT);
			if (txt) {
				str = expo_get_str(scn->expo, txt->str_id);
				if (str && *str == key)
					return item;
			}
		}
	}

	return NULL;
}

int scene_menu_send_key(struct scene *scn, struct scene_obj_menu *menu, int key,
			struct expo_action *event)
{
	const bool open = menu->obj.flags & SCENEOF_OPEN;
	struct scene_menitem *item, *cur, *key_item;

	cur = NULL;
	key_item = NULL;

	if (!list_empty(&menu->item_head)) {
		list_for_each_entry(item, &menu->item_head, sibling) {
			/* select an item if not done already */
			if (menu->cur_item_id == item->id) {
				cur = item;
				break;
			}
		}
	}

	if (!cur)
		return -ENOTTY;

	switch (key) {
	case BKEY_UP:
		if (item != list_first_entry(&menu->item_head,
					     struct scene_menitem, sibling)) {
			item = list_entry(item->sibling.prev,
					  struct scene_menitem, sibling);
			event->type = EXPOACT_POINT_ITEM;
			event->select.id = item->id;
			log_debug("up to item %d\n", event->select.id);
		}
		break;
	case BKEY_DOWN:
		if (!list_is_last(&item->sibling, &menu->item_head)) {
			item = list_entry(item->sibling.next,
					  struct scene_menitem, sibling);
			event->type = EXPOACT_POINT_ITEM;
			event->select.id = item->id;
			log_debug("down to item %d\n", event->select.id);
		}
		break;
	case BKEY_SELECT:
		event->type = EXPOACT_SELECT;
		event->select.id = item->id;
		log_debug("select item %d\n", event->select.id);
		break;
	case BKEY_QUIT:
		if (scn->expo->popup && open) {
			event->type = EXPOACT_CLOSE;
			event->select.id = menu->obj.id;
		} else {
			event->type = EXPOACT_QUIT;
			log_debug("menu quit\n");
		}
		break;
	case '0'...'9':
		key_item = scene_menu_find_key(scn, menu, key);
		if (key_item) {
			event->type = EXPOACT_SELECT;
			event->select.id = key_item->id;
		}
		break;
	}

	menu_point_to_item(menu, item->id);

	return 0;
}

int scene_menuitem(struct scene *scn, uint menu_id, const char *name, uint id,
		   uint key_id, uint label_id, uint desc_id, uint preview_id,
		   uint flags, struct scene_menitem **itemp)
{
	struct scene_obj_menu *menu;
	struct scene_menitem *item;

	menu = scene_obj_find(scn, menu_id, SCENEOBJT_MENU);
	if (!menu)
		return log_msg_ret("find", -ENOENT);

	/* Check that the text ID is valid */
	if (!scene_obj_find(scn, label_id, SCENEOBJT_TEXT))
		return log_msg_ret("txt", -EINVAL);

	item = calloc(1, sizeof(struct scene_menitem));
	if (!item)
		return log_msg_ret("item", -ENOMEM);
	item->name = strdup(name);
	if (!item->name) {
		free(item);
		return log_msg_ret("name", -ENOMEM);
	}

	item->id = resolve_id(scn->expo, id);
	item->key_id = key_id;
	item->label_id = label_id;
	item->desc_id = desc_id;
	item->preview_id = preview_id;
	item->flags = flags;
	list_add_tail(&item->sibling, &menu->item_head);

	if (itemp)
		*itemp = item;

	return item->id;
}

int scene_menu_set_title(struct scene *scn, uint id, uint title_id)
{
	struct scene_obj_menu *menu;
	struct scene_obj_txt *txt;

	menu = scene_obj_find(scn, id, SCENEOBJT_MENU);
	if (!menu)
		return log_msg_ret("menu", -ENOENT);

	/* Check that the ID is valid */
	if (title_id) {
		txt = scene_obj_find(scn, title_id, SCENEOBJT_TEXT);
		if (!txt)
			return log_msg_ret("txt", -EINVAL);
	}

	menu->title_id = title_id;

	return 0;
}

int scene_menu_set_pointer(struct scene *scn, uint id, uint pointer_id)
{
	struct scene_obj_menu *menu;
	struct scene_obj *obj;

	menu = scene_obj_find(scn, id, SCENEOBJT_MENU);
	if (!menu)
		return log_msg_ret("menu", -ENOENT);

	/* Check that the ID is valid */
	if (pointer_id) {
		obj = scene_obj_find(scn, pointer_id, SCENEOBJT_NONE);
		if (!obj)
			return log_msg_ret("obj", -EINVAL);
	}

	menu->pointer_id = pointer_id;

	return 0;
}

int scene_menu_display(struct scene_obj_menu *menu)
{
	struct scene *scn = menu->obj.scene;
	struct scene_obj_txt *pointer;
	struct expo *exp = scn->expo;
	struct scene_menitem *item;
	const char *pstr;

	printf("U-Boot    :    Boot Menu\n\n");
	if (menu->title_id) {
		struct scene_obj_txt *txt;
		const char *str;

		txt = scene_obj_find(scn, menu->title_id, SCENEOBJT_TEXT);
		if (!txt)
			return log_msg_ret("txt", -EINVAL);

		str = expo_get_str(exp, txt->str_id);
		printf("%s\n\n", str);
	}

	if (list_empty(&menu->item_head))
		return 0;

	pointer = scene_obj_find(scn, menu->pointer_id, SCENEOBJT_TEXT);
	pstr = expo_get_str(scn->expo, pointer->str_id);

	list_for_each_entry(item, &menu->item_head, sibling) {
		struct scene_obj_txt *key = NULL, *label = NULL;
		struct scene_obj_txt *desc = NULL;
		const char *kstr = NULL, *lstr = NULL, *dstr = NULL;

		key = scene_obj_find(scn, item->key_id, SCENEOBJT_TEXT);
		if (key)
			kstr = expo_get_str(exp, key->str_id);

		label = scene_obj_find(scn, item->label_id, SCENEOBJT_TEXT);
		if (label)
			lstr = expo_get_str(exp, label->str_id);

		desc = scene_obj_find(scn, item->desc_id, SCENEOBJT_TEXT);
		if (desc)
			dstr = expo_get_str(exp, desc->str_id);

		printf("%3s  %3s  %-10s  %s\n",
		       pointer && menu->cur_item_id == item->id ? pstr : "",
		       kstr, lstr, dstr);
	}

	return -ENOTSUPP;
}

int scene_menu_render_deps(struct scene *scn, struct scene_obj_menu *menu)
{
	struct scene_menitem *item;

	scene_render_deps(scn, menu->title_id);
	scene_render_deps(scn, menu->cur_item_id);
	scene_render_deps(scn, menu->pointer_id);

	list_for_each_entry(item, &menu->item_head, sibling) {
		scene_render_deps(scn, item->key_id);
		scene_render_deps(scn, item->label_id);
		scene_render_deps(scn, item->desc_id);
	}

	return 0;
}
