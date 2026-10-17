// SPDX-License-Identifier: GPL-2.0+
/*
 * Implementation of a scene, a collection of text/image/menu items in an expo
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

int scene_new(struct expo *exp, const char *name, uint id, struct scene **scnp)
{
	struct scene *scn;

	scn = calloc(1, sizeof(struct scene));
	if (!scn)
		return log_msg_ret("expo", -ENOMEM);
	scn->name = strdup(name);
	if (!scn->name) {
		free(scn);
		return log_msg_ret("name", -ENOMEM);
	}

	abuf_init(&scn->buf);
	if (!abuf_realloc(&scn->buf, EXPO_MAX_CHARS + 1)) {
		free(scn->name);
		free(scn);
		return log_msg_ret("buf", -ENOMEM);
	}
	abuf_init(&scn->entry_save);

	INIT_LIST_HEAD(&scn->obj_head);
	scn->id = resolve_id(exp, id);
	scn->expo = exp;
	list_add_tail(&scn->sibling, &exp->scene_head);

	*scnp = scn;

	return scn->id;
}

void scene_obj_destroy(struct scene_obj *obj)
{
	if (obj->type == SCENEOBJT_MENU)
		scene_menu_destroy((struct scene_obj_menu *)obj);
	free(obj->name);
	free(obj);
}

void scene_destroy(struct scene *scn)
{
	struct scene_obj *obj, *next;

	list_for_each_entry_safe(obj, next, &scn->obj_head, sibling)
		scene_obj_destroy(obj);

	abuf_uninit(&scn->entry_save);
	abuf_uninit(&scn->buf);
	free(scn->name);
	free(scn);
}

int scene_title_set(struct scene *scn, uint id)
{
	scn->title_id = id;

	return 0;
}

int scene_obj_count(struct scene *scn)
{
	struct scene_obj *obj;
	int count = 0;

	list_for_each_entry(obj, &scn->obj_head, sibling)
		count++;

	return count;
}

void *scene_obj_find(const struct scene *scn, uint id, enum scene_obj_t type)
{
	struct scene_obj *obj;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (obj->id == id &&
		    (type == SCENEOBJT_NONE || obj->type == type))
			return obj;
	}

	return NULL;
}

void *scene_obj_find_by_name(struct scene *scn, const char *name)
{
	struct scene_obj *obj;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (!strcmp(name, obj->name))
			return obj;
	}

	return NULL;
}

int scene_obj_add(struct scene *scn, const char *name, uint id,
		  enum scene_obj_t type, uint size, struct scene_obj **objp)
{
	struct scene_obj *obj;

	obj = calloc(1, size);
	if (!obj)
		return log_msg_ret("obj", -ENOMEM);
	obj->name = strdup(name);
	if (!obj->name) {
		free(obj);
		return log_msg_ret("name", -ENOMEM);
	}

	obj->id = resolve_id(scn->expo, id);
	obj->scene = scn;
	obj->type = type;
	list_add_tail(&obj->sibling, &scn->obj_head);
	*objp = obj;

	return obj->id;
}

int scene_img(struct scene *scn, const char *name, uint id, char *data,
	      struct scene_obj_img **imgp)
{
	struct scene_obj_img *img;
	int ret;

	ret = scene_obj_add(scn, name, id, SCENEOBJT_IMAGE,
			    sizeof(struct scene_obj_img),
			    (struct scene_obj **)&img);
	if (ret < 0)
		return log_msg_ret("obj", ret);

	img->data = data;

	if (imgp)
		*imgp = img;

	return img->obj.id;
}

int scene_txt(struct scene *scn, const char *name, uint id, uint str_id,
	      struct scene_obj_txt **txtp)
{
	struct scene_obj_txt *txt;
	int ret;

	ret = scene_obj_add(scn, name, id, SCENEOBJT_TEXT,
			    sizeof(struct scene_obj_txt),
			    (struct scene_obj **)&txt);
	if (ret < 0)
		return log_msg_ret("obj", ret);

	txt->str_id = str_id;

	if (txtp)
		*txtp = txt;

	return txt->obj.id;
}

int scene_txt_str(struct scene *scn, const char *name, uint id, uint str_id,
		  const char *str, struct scene_obj_txt **txtp)
{
	struct scene_obj_txt *txt;
	int ret;

	ret = expo_str(scn->expo, name, str_id, str);
	if (ret < 0)
		return log_msg_ret("str", ret);
	if (str_id && ret != str_id)
		return log_msg_ret("id", -EEXIST);
	str_id = ret;

	ret = scene_obj_add(scn, name, id, SCENEOBJT_TEXT,
			    sizeof(struct scene_obj_txt),
			    (struct scene_obj **)&txt);
	if (ret < 0)
		return log_msg_ret("obj", ret);

	txt->str_id = str_id;

	if (txtp)
		*txtp = txt;

	return txt->obj.id;
}

int scene_txt_set_font(struct scene *scn, uint id, const char *font_name,
		       uint font_size)
{
	struct scene_obj_txt *txt;

	txt = scene_obj_find(scn, id, SCENEOBJT_TEXT);
	if (!txt)
		return log_msg_ret("find", -ENOENT);
	txt->font_name = font_name;
	txt->font_size = font_size;

	return 0;
}

int scene_obj_set_pos(struct scene *scn, uint id, int x, int y)
{
	struct scene_obj *obj;

	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("find", -ENOENT);
	obj->dim.x = x;
	obj->dim.y = y;

	return 0;
}

int scene_obj_set_size(struct scene *scn, uint id, int w, int h)
{
	struct scene_obj *obj;

	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("find", -ENOENT);
	obj->dim.w = w;
	obj->dim.h = h;

	return 0;
}

int scene_obj_set_hide(struct scene *scn, uint id, bool hide)
{
	int ret;

	ret = scene_obj_flag_clrset(scn, id, SCENEOF_HIDE,
				    hide ? SCENEOF_HIDE : 0);
	if (ret)
		return log_msg_ret("flg", ret);

	return 0;
}

int scene_obj_flag_clrset(struct scene *scn, uint id, uint clr, uint set)
{
	struct scene_obj *obj;

	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("find", -ENOENT);
	obj->flags &= ~clr;
	obj->flags |= set;

	return 0;
}

int scene_obj_get_hw(struct scene *scn, uint id, int *widthp)
{
	struct scene_obj *obj;

	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("find", -ENOENT);

	switch (obj->type) {
	case SCENEOBJT_NONE:
	case SCENEOBJT_MENU:
	case SCENEOBJT_TEXTLINE:
		break;
	case SCENEOBJT_IMAGE: {
		struct scene_obj_img *img = (struct scene_obj_img *)obj;
		ulong width, height;
		uint bpix;

		video_bmp_get_info(img->data, &width, &height, &bpix);
		if (widthp)
			*widthp = width;
		return height;
	}
	case SCENEOBJT_TEXT: {
		struct scene_obj_txt *txt = (struct scene_obj_txt *)obj;
		struct expo *exp = scn->expo;
		struct vidconsole_bbox bbox;
		const char *str;
		int len, ret;

		str = expo_get_str(exp, txt->str_id);
		if (!str)
			return log_msg_ret("str", -ENOENT);
		len = strlen(str);

		/* if there is no console, make it up */
		if (!exp->cons) {
			if (widthp)
				*widthp = 8 * len;
			return 16;
		}

		ret = vidconsole_measure(scn->expo->cons, txt->font_name,
					 txt->font_size, str, &bbox);
		if (ret)
			return log_msg_ret("mea", ret);
		if (widthp)
			*widthp = bbox.x1;

		return bbox.y1;
	}
	}

	return 0;
}

/**
 * scene_render_background() - Render the background for an object
 *
 * @obj: Object to render
 * @box_only: true to show a box around the object, but keep the normal
 * background colour inside
 */
static void scene_render_background(struct scene_obj *obj, bool box_only)
{
	struct expo *exp = obj->scene->expo;
	const struct expo_theme *theme = &exp->theme;
	struct vidconsole_bbox bbox, label_bbox;
	struct udevice *dev = exp->display;
	struct video_priv *vid_priv;
	struct udevice *cons = exp->cons;
	struct vidconsole_colour old;
	enum colour_idx fore, back;
	uint inset = theme->menu_inset;

	/* draw a background for the object */
	if (CONFIG_IS_ENABLED(SYS_WHITE_ON_BLACK)) {
		fore = VID_BLACK;
		back = VID_WHITE;
	} else {
		fore = VID_LIGHT_GRAY;
		back = VID_BLACK;
	}

	/* see if this object wants to render a background */
	if (scene_obj_calc_bbox(obj, &bbox, &label_bbox))
		return;

	vidconsole_push_colour(cons, fore, back, &old);
	vid_priv = dev_get_uclass_priv(dev);
	video_fill_part(dev, label_bbox.x0 - inset, label_bbox.y0 - inset,
			label_bbox.x1 + inset, label_bbox.y1 + inset,
			vid_priv->colour_fg);
	vidconsole_pop_colour(cons, &old);
	if (box_only) {
		video_fill_part(dev, label_bbox.x0, label_bbox.y0,
				label_bbox.x1, label_bbox.y1,
				vid_priv->colour_bg);
	}
}

/**
 * scene_obj_render() - Render an object
 *
 */
static int scene_obj_render(struct scene_obj *obj, bool text_mode)
{
	struct scene *scn = obj->scene;
	struct expo *exp = scn->expo;
	const struct expo_theme *theme = &exp->theme;
	struct udevice *dev = exp->display;
	struct udevice *cons = text_mode ? NULL : exp->cons;
	int x, y, ret;

	x = obj->dim.x;
	y = obj->dim.y;

	switch (obj->type) {
	case SCENEOBJT_NONE:
		break;
	case SCENEOBJT_IMAGE: {
		struct scene_obj_img *img = (struct scene_obj_img *)obj;

		if (!cons)
			return -ENOTSUPP;
		ret = video_bmp_display(dev, map_to_sysmem(img->data), x, y,
					true);
		if (ret < 0)
			return log_msg_ret("img", ret);
		break;
	}
	case SCENEOBJT_TEXT: {
		struct scene_obj_txt *txt = (struct scene_obj_txt *)obj;
		const char *str;

		if (!cons)
			return -ENOTSUPP;

		if (txt->font_name || txt->font_size) {
			ret = vidconsole_select_font(cons,
						     txt->font_name,
						     txt->font_size);
		} else {
			ret = vidconsole_select_font(cons, NULL, 0);
		}
		if (ret && ret != -ENOSYS)
			return log_msg_ret("font", ret);
		str = expo_get_str(exp, txt->str_id);
		if (str) {
			struct video_priv *vid_priv;
			struct vidconsole_colour old;
			enum colour_idx fore, back;

			if (CONFIG_IS_ENABLED(SYS_WHITE_ON_BLACK)) {
				fore = VID_BLACK;
				back = VID_WHITE;
			} else {
				fore = VID_LIGHT_GRAY;
				back = VID_BLACK;
			}

			vid_priv = dev_get_uclass_priv(dev);
			if (obj->flags & SCENEOF_POINT) {
				vidconsole_push_colour(cons, fore, back, &old);
				video_fill_part(dev, x - theme->menu_inset, y,
						x + obj->dim.w,
						y + obj->dim.h,
						vid_priv->colour_bg);
			}
			vidconsole_set_cursor_pos(cons, x, y);
			vidconsole_put_string(cons, str);
			if (obj->flags & SCENEOF_POINT)
				vidconsole_pop_colour(cons, &old);
		}
		break;
	}
	case SCENEOBJT_MENU: {
		struct scene_obj_menu *menu = (struct scene_obj_menu *)obj;

		if (exp->popup && (obj->flags & SCENEOF_OPEN)) {
			if (!cons)
				return -ENOTSUPP;

			/* draw a background behind the menu items */
			scene_render_background(obj, false);
		}
		/*
		 * With a vidconsole, the text and item pointer are rendered as
		 * normal objects so we don't need to do anything here. The menu
		 * simply controls where they are positioned.
		 */
		if (cons)
			return -ENOTSUPP;

		ret = scene_menu_display(menu);
		if (ret < 0)
			return log_msg_ret("img", ret);

		break;
	}
	case SCENEOBJT_TEXTLINE:
		if (obj->flags & SCENEOF_OPEN)
			scene_render_background(obj, true);
		break;
	}

	return 0;
}

int scene_arrange(struct scene *scn)
{
	struct scene_obj *obj;
	int ret;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		switch (obj->type) {
		case SCENEOBJT_NONE:
		case SCENEOBJT_IMAGE:
		case SCENEOBJT_TEXT:
			break;
		case SCENEOBJT_MENU: {
			struct scene_obj_menu *menu;

			menu = (struct scene_obj_menu *)obj,
			ret = scene_menu_arrange(scn, menu);
			if (ret)
				return log_msg_ret("arr", ret);
			break;
		}
		case SCENEOBJT_TEXTLINE: {
			struct scene_obj_textline *tline;

			tline = (struct scene_obj_textline *)obj,
			ret = scene_textline_arrange(scn, tline);
			if (ret)
				return log_msg_ret("arr", ret);
			break;
		}
		}
	}

	return 0;
}

int scene_render_deps(struct scene *scn, uint id)
{
	struct scene_obj *obj;
	int ret;

	if (!id)
		return 0;
	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("obj", -ENOENT);

	if (!(obj->flags & SCENEOF_HIDE)) {
		ret = scene_obj_render(obj, false);
		if (ret && ret != -ENOTSUPP)
			return log_msg_ret("ren", ret);

		switch (obj->type) {
		case SCENEOBJT_NONE:
		case SCENEOBJT_IMAGE:
		case SCENEOBJT_TEXT:
			break;
		case SCENEOBJT_MENU:
			scene_menu_render_deps(scn,
					       (struct scene_obj_menu *)obj);
			break;
		case SCENEOBJT_TEXTLINE:
			scene_textline_render_deps(scn,
					(struct scene_obj_textline *)obj);
			break;
		}
	}

	return 0;
}

int scene_render(struct scene *scn)
{
	struct expo *exp = scn->expo;
	struct scene_obj *obj;
	int ret;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (!(obj->flags & SCENEOF_HIDE)) {
			ret = scene_obj_render(obj, exp->text_mode);
			if (ret && ret != -ENOTSUPP)
				return log_msg_ret("ren", ret);
		}
	}

	/* render any highlighted object on top of the others */
	if (scn->highlight_id && !exp->text_mode) {
		ret = scene_render_deps(scn, scn->highlight_id);
		if (ret && ret != -ENOTSUPP)
			return log_msg_ret("dep", ret);
	}

	return 0;
}

/**
 * send_key_obj() - Handle a keypress for moving between objects
 *
 * @scn: Scene to receive the key
 * @key: Key to send (KEYCODE_UP)
 * @event: Returns resulting event from this keypress
 * Returns: 0 if OK, -ve on error
 */
static void send_key_obj(struct scene *scn, struct scene_obj *obj, int key,
			 struct expo_action *event)
{
	switch (key) {
	case BKEY_UP:
		while (obj != list_first_entry(&scn->obj_head, struct scene_obj,
					       sibling)) {
			obj = list_entry(obj->sibling.prev,
					 struct scene_obj, sibling);
			if (scene_obj_can_highlight(obj)) {
				event->type = EXPOACT_POINT_OBJ;
				event->select.id = obj->id;
				log_debug("up to obj %d\n", event->select.id);
				break;
			}
		}
		break;
	case BKEY_DOWN:
		while (!list_is_last(&obj->sibling, &scn->obj_head)) {
			obj = list_entry(obj->sibling.next, struct scene_obj,
					 sibling);
			if (scene_obj_can_highlight(obj)) {
				event->type = EXPOACT_POINT_OBJ;
				event->select.id = obj->id;
				log_debug("down to obj %d\n", event->select.id);
				break;
			}
		}
		break;
	case BKEY_SELECT:
		if (scene_obj_can_highlight(obj)) {
			event->type = EXPOACT_OPEN;
			event->select.id = obj->id;
			log_debug("open obj %d\n", event->select.id);
		}
		break;
	case BKEY_QUIT:
		event->type = EXPOACT_QUIT;
		log_debug("obj quit\n");
		break;
	}
}

int scene_send_key(struct scene *scn, int key, struct expo_action *event)
{
	struct scene_obj *obj;
	int ret;

	event->type = EXPOACT_NONE;

	/*
	 * In 'popup' mode, arrow keys move betwen objects, unless a menu is
	 * opened
	 */
	if (scn->expo->popup) {
		obj = NULL;
		if (scn->highlight_id) {
			obj = scene_obj_find(scn, scn->highlight_id,
					     SCENEOBJT_NONE);
		}
		if (!obj)
			return 0;

		if (!(obj->flags & SCENEOF_OPEN)) {
			send_key_obj(scn, obj, key, event);
			return 0;
		}

		switch (obj->type) {
		case SCENEOBJT_NONE:
		case SCENEOBJT_IMAGE:
		case SCENEOBJT_TEXT:
			break;
		case SCENEOBJT_MENU: {
			struct scene_obj_menu *menu;

			menu = (struct scene_obj_menu *)obj,
			ret = scene_menu_send_key(scn, menu, key, event);
			if (ret)
				return log_msg_ret("key", ret);
			break;
		}
		case SCENEOBJT_TEXTLINE: {
			struct scene_obj_textline *tline;

			tline = (struct scene_obj_textline *)obj,
			ret = scene_textline_send_key(scn, tline, key, event);
			if (ret)
				return log_msg_ret("key", ret);
			break;
		}
		}
		return 0;
	}

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (obj->type == SCENEOBJT_MENU) {
			struct scene_obj_menu *menu;

			menu = (struct scene_obj_menu *)obj,
			ret = scene_menu_send_key(scn, menu, key, event);
			if (ret)
				return log_msg_ret("key", ret);
			break;
		}
	}

	return 0;
}

int scene_obj_calc_bbox(struct scene_obj *obj, struct vidconsole_bbox *bbox,
			struct vidconsole_bbox *label_bbox)
{
	switch (obj->type) {
	case SCENEOBJT_NONE:
	case SCENEOBJT_IMAGE:
	case SCENEOBJT_TEXT:
		return -ENOSYS;
	case SCENEOBJT_MENU: {
		struct scene_obj_menu *menu = (struct scene_obj_menu *)obj;

		scene_menu_calc_bbox(menu, bbox, label_bbox);
		break;
	}
	case SCENEOBJT_TEXTLINE: {
		struct scene_obj_textline *tline;

		tline = (struct scene_obj_textline *)obj;
		scene_textline_calc_bbox(tline, bbox, label_bbox);
		break;
	}
	}

	return 0;
}

int scene_calc_dims(struct scene *scn, bool do_menus)
{
	struct scene_obj *obj;
	int ret;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		switch (obj->type) {
		case SCENEOBJT_NONE:
		case SCENEOBJT_TEXT:
		case SCENEOBJT_IMAGE: {
			int width;

			if (!do_menus) {
				ret = scene_obj_get_hw(scn, obj->id, &width);
				if (ret < 0)
					return log_msg_ret("get", ret);
				obj->dim.w = width;
				obj->dim.h = ret;
			}
			break;
		}
		case SCENEOBJT_MENU: {
			struct scene_obj_menu *menu;

			if (do_menus) {
				menu = (struct scene_obj_menu *)obj;

				ret = scene_menu_calc_dims(menu);
				if (ret)
					return log_msg_ret("men", ret);
			}
			break;
		}
		case SCENEOBJT_TEXTLINE: {
			struct scene_obj_textline *tline;

			tline = (struct scene_obj_textline *)obj;
			ret = scene_textline_calc_dims(tline);
			if (ret)
				return log_msg_ret("men", ret);

			break;
		}
		}
	}

	return 0;
}

int scene_apply_theme(struct scene *scn, struct expo_theme *theme)
{
	struct scene_obj *obj;
	int ret;

	/* Avoid error-checking optional items */
	scene_txt_set_font(scn, scn->title_id, NULL, theme->font_size);

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		switch (obj->type) {
		case SCENEOBJT_NONE:
		case SCENEOBJT_IMAGE:
		case SCENEOBJT_MENU:
		case SCENEOBJT_TEXTLINE:
			break;
		case SCENEOBJT_TEXT:
			scene_txt_set_font(scn, obj->id, NULL,
					   theme->font_size);
			break;
		}
	}

	ret = scene_arrange(scn);
	if (ret)
		return log_msg_ret("arr", ret);

	return 0;
}

void scene_set_highlight_id(struct scene *scn, uint id)
{
	scn->highlight_id = id;
}

void scene_highlight_first(struct scene *scn)
{
	struct scene_obj *obj;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (scene_obj_can_highlight(obj)) {
			scene_set_highlight_id(scn, obj->id);
			return;
		}
	}
}

static int scene_obj_open(struct scene *scn, struct scene_obj *obj)
{
	int ret;

	switch (obj->type) {
	case SCENEOBJT_NONE:
	case SCENEOBJT_IMAGE:
	case SCENEOBJT_MENU:
	case SCENEOBJT_TEXT:
		break;
	case SCENEOBJT_TEXTLINE:
		ret = scene_textline_open(scn,
					  (struct scene_obj_textline *)obj);
		if (ret)
			return log_msg_ret("op", ret);
		break;
	}

	return 0;
}

int scene_set_open(struct scene *scn, uint id, bool open)
{
	struct scene_obj *obj;
	int ret;

	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("find", -ENOENT);

	if (open) {
		ret = scene_obj_open(scn, obj);
		if (ret)
			return log_msg_ret("op", ret);
	}

	ret = scene_obj_flag_clrset(scn, id, SCENEOF_OPEN,
				    open ? SCENEOF_OPEN : 0);
	if (ret)
		return log_msg_ret("flg", ret);

	return 0;
}

int scene_iter_objs(struct scene *scn, expo_scene_obj_iterator iter,
		    void *priv)
{
	struct scene_obj *obj;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		int ret;

		ret = iter(obj, priv);
		if (ret)
			return log_msg_ret("itr", ret);
	}

	return 0;
}

int scene_bbox_union(struct scene *scn, uint id, int inset,
		     struct vidconsole_bbox *bbox)
{
	struct scene_obj *obj;

	if (!id)
		return 0;
	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("obj", -ENOENT);
	if (bbox->valid) {
		bbox->x0 = min(bbox->x0, obj->dim.x - inset);
		bbox->y0 = min(bbox->y0, obj->dim.y);
		bbox->x1 = max(bbox->x1, obj->dim.x + obj->dim.w + inset);
		bbox->y1 = max(bbox->y1, obj->dim.y + obj->dim.h);
	} else {
		bbox->x0 = obj->dim.x - inset;
		bbox->y0 = obj->dim.y;
		bbox->x1 = obj->dim.x + obj->dim.w + inset;
		bbox->y1 = obj->dim.y + obj->dim.h;
		bbox->valid = true;
	}

	return 0;
}
