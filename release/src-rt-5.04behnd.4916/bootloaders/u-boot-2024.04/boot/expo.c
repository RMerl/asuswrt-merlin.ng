// SPDX-License-Identifier: GPL-2.0+
/*
 * Implementation of a expo, a collection of scenes providing menu options
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EXPO

#include <common.h>
#include <dm.h>
#include <expo.h>
#include <malloc.h>
#include <video.h>
#include "scene_internal.h"

int expo_new(const char *name, void *priv, struct expo **expp)
{
	struct expo *exp;

	exp = calloc(1, sizeof(struct expo));
	if (!exp)
		return log_msg_ret("expo", -ENOMEM);
	exp->name = strdup(name);
	if (!exp->name) {
		free(exp);
		return log_msg_ret("name", -ENOMEM);
	}
	exp->priv = priv;
	INIT_LIST_HEAD(&exp->scene_head);
	INIT_LIST_HEAD(&exp->str_head);

	*expp = exp;

	return 0;
}

static void estr_destroy(struct expo_string *estr)
{
	free(estr);
}

void expo_destroy(struct expo *exp)
{
	struct scene *scn, *next;
	struct expo_string *estr, *enext;

	list_for_each_entry_safe(scn, next, &exp->scene_head, sibling)
		scene_destroy(scn);

	list_for_each_entry_safe(estr, enext, &exp->str_head, sibling)
		estr_destroy(estr);

	free(exp->name);
	free(exp);
}

uint resolve_id(struct expo *exp, uint id)
{
	log_debug("resolve id %d\n", id);
	if (!id)
		id = exp->next_id++;
	else if (id >= exp->next_id)
		exp->next_id = id + 1;

	return id;
}

void expo_set_dynamic_start(struct expo *exp, uint dyn_start)
{
	exp->next_id = dyn_start;
}

int expo_str(struct expo *exp, const char *name, uint id, const char *str)
{
	struct expo_string *estr;

	estr = calloc(1, sizeof(struct expo_string));
	if (!estr)
		return log_msg_ret("obj", -ENOMEM);

	estr->id = resolve_id(exp, id);
	estr->str = str;
	list_add_tail(&estr->sibling, &exp->str_head);

	return estr->id;
}

const char *expo_get_str(struct expo *exp, uint id)
{
	struct expo_string *estr;

	list_for_each_entry(estr, &exp->str_head, sibling) {
		if (estr->id == id)
			return estr->str;
	}

	return NULL;
}

int expo_set_display(struct expo *exp, struct udevice *dev)
{
	struct udevice *cons;
	int ret;

	ret = device_find_first_child_by_uclass(dev, UCLASS_VIDEO_CONSOLE,
						&cons);
	if (ret)
		return log_msg_ret("con", ret);

	exp->display = dev;
	exp->cons = cons;

	return 0;
}

int expo_calc_dims(struct expo *exp)
{
	struct scene *scn;
	int ret;

	if (!exp->cons)
		return log_msg_ret("dim", -ENOTSUPP);

	list_for_each_entry(scn, &exp->scene_head, sibling) {
		/*
		 * Do the menus last so that all the menus' text objects
		 * are dimensioned
		 */
		ret = scene_calc_dims(scn, false);
		if (ret)
			return log_msg_ret("scn", ret);
		ret = scene_calc_dims(scn, true);
		if (ret)
			return log_msg_ret("scn", ret);
	}

	return 0;
}

void expo_set_text_mode(struct expo *exp, bool text_mode)
{
	exp->text_mode = text_mode;
}

struct scene *expo_lookup_scene_id(struct expo *exp, uint scene_id)
{
	struct scene *scn;

	list_for_each_entry(scn, &exp->scene_head, sibling) {
		if (scn->id == scene_id)
			return scn;
	}

	return NULL;
}

int expo_set_scene_id(struct expo *exp, uint scene_id)
{
	struct scene *scn;
	int ret;

	scn = expo_lookup_scene_id(exp, scene_id);
	if (!scn)
		return log_msg_ret("id", -ENOENT);
	ret = scene_arrange(scn);
	if (ret)
		return log_msg_ret("arr", ret);

	exp->scene_id = scene_id;

	return 0;
}

int expo_first_scene_id(struct expo *exp)
{
	struct scene *scn;

	if (list_empty(&exp->scene_head))
		return -ENOENT;

	scn = list_first_entry(&exp->scene_head, struct scene, sibling);

	return scn->id;
}

int expo_render(struct expo *exp)
{
	struct udevice *dev = exp->display;
	struct video_priv *vid_priv = dev_get_uclass_priv(dev);
	struct scene *scn = NULL;
	enum colour_idx back;
	u32 colour;
	int ret;

	back = CONFIG_IS_ENABLED(SYS_WHITE_ON_BLACK) ? VID_BLACK : VID_WHITE;
	colour = video_index_to_colour(vid_priv, back);
	ret = video_fill(dev, colour);
	if (ret)
		return log_msg_ret("fill", ret);

	if (exp->scene_id) {
		scn = expo_lookup_scene_id(exp, exp->scene_id);
		if (!scn)
			return log_msg_ret("scn", -ENOENT);

		ret = scene_render(scn);
		if (ret)
			return log_msg_ret("ren", ret);
	}

	video_sync(dev, true);

	return scn ? 0 : -ECHILD;
}

int expo_send_key(struct expo *exp, int key)
{
	struct scene *scn = NULL;

	if (exp->scene_id) {
		int ret;

		scn = expo_lookup_scene_id(exp, exp->scene_id);
		if (!scn)
			return log_msg_ret("scn", -ENOENT);

		ret = scene_send_key(scn, key, &exp->action);
		if (ret)
			return log_msg_ret("key", ret);

		/* arrange it to get any changes */
		ret = scene_arrange(scn);
		if (ret)
			return log_msg_ret("arr", ret);
	}

	return scn ? 0 : -ECHILD;
}

int expo_action_get(struct expo *exp, struct expo_action *act)
{
	*act = exp->action;
	exp->action.type = EXPOACT_NONE;

	return act->type == EXPOACT_NONE ? -EAGAIN : 0;
}

int expo_apply_theme(struct expo *exp, ofnode node)
{
	struct scene *scn;
	struct expo_theme *theme = &exp->theme;
	int ret;

	log_debug("Applying theme %s\n", ofnode_get_name(node));

	memset(theme, '\0', sizeof(struct expo_theme));
	ofnode_read_u32(node, "font-size", &theme->font_size);
	ofnode_read_u32(node, "menu-inset", &theme->menu_inset);
	ofnode_read_u32(node, "menuitem-gap-y", &theme->menuitem_gap_y);

	list_for_each_entry(scn, &exp->scene_head, sibling) {
		ret = scene_apply_theme(scn, theme);
		if (ret)
			return log_msg_ret("app", ret);
	}

	return 0;
}

int expo_iter_scene_objs(struct expo *exp, expo_scene_obj_iterator iter,
			 void *priv)
{
	struct scene *scn;
	int ret;

	list_for_each_entry(scn, &exp->scene_head, sibling) {
		ret = scene_iter_objs(scn, iter, priv);
		if (ret)
			return log_msg_ret("wr", ret);
	}

	return 0;
}
