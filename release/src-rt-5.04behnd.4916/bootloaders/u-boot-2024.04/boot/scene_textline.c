// SPDX-License-Identifier: GPL-2.0+
/*
 * Implementation of a menu in a scene
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EXPO

#include <common.h>
#include <expo.h>
#include <menu.h>
#include <video_console.h>
#include "scene_internal.h"

int scene_textline(struct scene *scn, const char *name, uint id, uint max_chars,
		   struct scene_obj_textline **tlinep)
{
	struct scene_obj_textline *tline;
	char *buf;
	int ret;

	if (max_chars >= EXPO_MAX_CHARS)
		return log_msg_ret("chr", -E2BIG);

	ret = scene_obj_add(scn, name, id, SCENEOBJT_TEXTLINE,
			    sizeof(struct scene_obj_textline),
			    (struct scene_obj **)&tline);
	if (ret < 0)
		return log_msg_ret("obj", -ENOMEM);
	abuf_init(&tline->buf);
	if (!abuf_realloc(&tline->buf, max_chars + 1))
		return log_msg_ret("buf", -ENOMEM);
	buf = abuf_data(&tline->buf);
	*buf = '\0';
	tline->pos = max_chars;
	tline->max_chars = max_chars;

	if (tlinep)
		*tlinep = tline;

	return tline->obj.id;
}

void scene_textline_calc_bbox(struct scene_obj_textline *tline,
			      struct vidconsole_bbox *bbox,
			      struct vidconsole_bbox *edit_bbox)
{
	const struct expo_theme *theme = &tline->obj.scene->expo->theme;

	bbox->valid = false;
	scene_bbox_union(tline->obj.scene, tline->label_id, 0, bbox);
	scene_bbox_union(tline->obj.scene, tline->edit_id, 0, bbox);

	edit_bbox->valid = false;
	scene_bbox_union(tline->obj.scene, tline->edit_id, theme->menu_inset,
			 edit_bbox);
}

int scene_textline_calc_dims(struct scene_obj_textline *tline)
{
	struct scene *scn = tline->obj.scene;
	struct vidconsole_bbox bbox;
	struct scene_obj_txt *txt;
	int ret;

	txt = scene_obj_find(scn, tline->edit_id, SCENEOBJT_NONE);
	if (!txt)
		return log_msg_ret("dim", -ENOENT);

	ret = vidconsole_nominal(scn->expo->cons, txt->font_name,
				 txt->font_size, tline->max_chars, &bbox);
	if (ret)
		return log_msg_ret("nom", ret);

	if (bbox.valid) {
		tline->obj.dim.w = bbox.x1 - bbox.x0;
		tline->obj.dim.h = bbox.y1 - bbox.y0;

		scene_obj_set_size(scn, tline->edit_id, tline->obj.dim.w,
				   tline->obj.dim.h);
	}

	return 0;
}

int scene_textline_arrange(struct scene *scn, struct scene_obj_textline *tline)
{
	const bool open = tline->obj.flags & SCENEOF_OPEN;
	bool point;
	int x, y;
	int ret;

	x = tline->obj.dim.x;
	y = tline->obj.dim.y;
	if (tline->label_id) {
		ret = scene_obj_set_pos(scn, tline->label_id, tline->obj.dim.x,
					y);
		if (ret < 0)
			return log_msg_ret("tit", ret);

		ret = scene_obj_set_pos(scn, tline->edit_id,
					tline->obj.dim.x + 200, y);
		if (ret < 0)
			return log_msg_ret("tit", ret);

		ret = scene_obj_get_hw(scn, tline->label_id, NULL);
		if (ret < 0)
			return log_msg_ret("hei", ret);

		y += ret * 2;
	}

	point = scn->highlight_id == tline->obj.id;
	point &= !open;
	scene_obj_flag_clrset(scn, tline->edit_id, SCENEOF_POINT,
			      point ? SCENEOF_POINT : 0);

	return 0;
}

int scene_textline_send_key(struct scene *scn, struct scene_obj_textline *tline,
			    int key, struct expo_action *event)
{
	const bool open = tline->obj.flags & SCENEOF_OPEN;

	log_debug("key=%d\n", key);
	switch (key) {
	case BKEY_QUIT:
		if (open) {
			event->type = EXPOACT_CLOSE;
			event->select.id = tline->obj.id;

			/* Copy the backup text from the scene buffer */
			memcpy(abuf_data(&tline->buf), abuf_data(&scn->buf),
			       abuf_size(&scn->buf));
		} else {
			event->type = EXPOACT_QUIT;
			log_debug("menu quit\n");
		}
		break;
	case BKEY_SELECT:
		if (!open)
			break;
		event->type = EXPOACT_CLOSE;
		event->select.id = tline->obj.id;
		key = '\n';
		fallthrough;
	default: {
		struct udevice *cons = scn->expo->cons;
		int ret;

		ret = vidconsole_entry_restore(cons, &scn->entry_save);
		if (ret)
			return log_msg_ret("sav", ret);
		ret = cread_line_process_ch(&scn->cls, key);
		ret = vidconsole_entry_save(cons, &scn->entry_save);
		if (ret)
			return log_msg_ret("sav", ret);
		break;
	}
	}

	return 0;
}

int scene_textline_render_deps(struct scene *scn,
			       struct scene_obj_textline *tline)
{
	const bool open = tline->obj.flags & SCENEOF_OPEN;
	struct udevice *cons = scn->expo->cons;
	struct scene_obj_txt *txt;
	int ret;

	scene_render_deps(scn, tline->label_id);
	scene_render_deps(scn, tline->edit_id);

	/* show the vidconsole cursor if open */
	if (open) {
		/* get the position within the field */
		txt = scene_obj_find(scn, tline->edit_id, SCENEOBJT_NONE);
		if (!txt)
			return log_msg_ret("cur", -ENOENT);

		if (txt->font_name || txt->font_size) {
			ret = vidconsole_select_font(cons,
						     txt->font_name,
						     txt->font_size);
		} else {
			ret = vidconsole_select_font(cons, NULL, 0);
		}

		ret = vidconsole_entry_restore(cons, &scn->entry_save);
		if (ret)
			return log_msg_ret("sav", ret);

		vidconsole_set_cursor_visible(cons, true, txt->obj.dim.x,
					      txt->obj.dim.y, scn->cls.num);
	}

	return 0;
}

int scene_textline_open(struct scene *scn, struct scene_obj_textline *tline)
{
	struct udevice *cons = scn->expo->cons;
	struct scene_obj_txt *txt;
	int ret;

	/* Copy the text into the scene buffer in case the edit is cancelled */
	memcpy(abuf_data(&scn->buf), abuf_data(&tline->buf),
	       abuf_size(&scn->buf));

	/* get the position of the editable */
	txt = scene_obj_find(scn, tline->edit_id, SCENEOBJT_NONE);
	if (!txt)
		return log_msg_ret("cur", -ENOENT);

	vidconsole_set_cursor_pos(cons, txt->obj.dim.x, txt->obj.dim.y);
	vidconsole_entry_start(cons);
	cli_cread_init(&scn->cls, abuf_data(&tline->buf), tline->max_chars);
	scn->cls.insert = true;
	ret = vidconsole_entry_save(cons, &scn->entry_save);
	if (ret)
		return log_msg_ret("sav", ret);

	return 0;
}
