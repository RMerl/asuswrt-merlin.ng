// SPDX-License-Identifier: GPL-2.0+
/*
 * Building an expo from an FDT description
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EXPO

#include <common.h>
#include <expo.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <dm/ofnode.h>
#include <linux/libfdt.h>

/**
 * struct build_info - Information to use when building
 *
 * @str_for_id: String for each ID in use, NULL if empty. The string is NULL
 *	if there is nothing for this ID. Since ID 0 is never used, the first
 *	element of this array is always NULL
 * @str_count: Number of entries in @str_for_id
 * @err_node: Node being processed (for error reporting)
 * @err_prop: Property being processed (for error reporting)
 */
struct build_info {
	const char **str_for_id;
	int str_count;
	ofnode err_node;
	const char *err_prop;
};

/**
 * add_txt_str - Add a string or lookup its ID, then add to expo
 *
 * @info: Build information
 * @node: Node describing scene
 * @scn: Scene to add to
 * @find_name: Name to look for (e.g. "title"). This will find a property called
 * "title" if it exists, else will look up the string for "title-id"
 * Return: ID of added string, or -ve on error
 */
int add_txt_str(struct build_info *info, ofnode node, struct scene *scn,
		const char *find_name, uint obj_id)
{
	const char *text;
	uint str_id;
	int ret;

	info->err_prop = find_name;
	text = ofnode_read_string(node, find_name);
	if (!text) {
		char name[40];
		u32 id;

		snprintf(name, sizeof(name), "%s-id", find_name);
		ret = ofnode_read_u32(node, name, &id);
		if (ret)
			return log_msg_ret("id", -ENOENT);

		if (id >= info->str_count)
			return log_msg_ret("id", -E2BIG);
		text = info->str_for_id[id];
		if (!text)
			return log_msg_ret("id", -EINVAL);
	}

	ret = expo_str(scn->expo, find_name, 0, text);
	if (ret < 0)
		return log_msg_ret("add", ret);
	str_id = ret;

	ret = scene_txt_str(scn, find_name, obj_id, str_id, text, NULL);
	if (ret < 0)
		return log_msg_ret("add", ret);

	return ret;
}

/**
 * add_txt_str_list - Add a list string or lookup its ID, then add to expo
 *
 * @info: Build information
 * @node: Node describing scene
 * @scn: Scene to add to
 * @find_name: Name to look for (e.g. "title"). This will find a string-list
 * property called "title" if it exists, else will look up the string in the
 * "title-id" string list.
 * Return: ID of added string, or -ve on error
 */
int add_txt_str_list(struct build_info *info, ofnode node, struct scene *scn,
		     const char *find_name, int index, uint obj_id)
{
	const char *text;
	uint str_id;
	int ret;

	ret = ofnode_read_string_index(node, find_name, index, &text);
	if (ret) {
		char name[40];
		u32 id;

		snprintf(name, sizeof(name), "%s-id", find_name);
		ret = ofnode_read_u32_index(node, name, index, &id);
		if (ret)
			return log_msg_ret("id", -ENOENT);

		if (id >= info->str_count)
			return log_msg_ret("id", -E2BIG);
		text = info->str_for_id[id];
		if (!text)
			return log_msg_ret("id", -EINVAL);
	}

	ret = expo_str(scn->expo, find_name, 0, text);
	if (ret < 0)
		return log_msg_ret("add", ret);
	str_id = ret;

	ret = scene_txt_str(scn, find_name, obj_id, str_id, text, NULL);
	if (ret < 0)
		return log_msg_ret("add", ret);

	return ret;
}

/*
 * build_element() - Handle creating a text object from a label
 *
 * Look up a property called @label or @label-id and create a string for it
 */
int build_element(void *ldtb, int node, const char *label)
{
	return 0;
}

/**
 * read_strings() - Read in the list of strings
 *
 * Read the strings into an ID-indexed list, so they can be used for building
 * an expo. The strings are in a /strings node and each has its own subnode
 * containing the ID and the string itself:
 *
 * example {
 *    id = <123>;
 *    value = "This is a test";
 * };
 *
 * Future work may add support for unicode and multiple languages
 *
 * @info: Build information
 * @root: Root node to read from
 * Returns: 0 if OK, -ENOMEM if out of memory, -EINVAL if there is a format
 * error
 */
static int read_strings(struct build_info *info, ofnode root)
{
	ofnode strings, node;

	strings = ofnode_find_subnode(root, "strings");
	if (!ofnode_valid(strings))
		return log_msg_ret("str", -EINVAL);

	ofnode_for_each_subnode(node, strings) {
		const char *val;
		int ret;
		u32 id;

		info->err_node = node;
		ret = ofnode_read_u32(node, "id", &id);
		if (ret)
			return log_msg_ret("id", -ENOENT);
		val = ofnode_read_string(node, "value");
		if (!val)
			return log_msg_ret("val", -EINVAL);

		if (id >= info->str_count) {
			int new_count = info->str_count + 20;
			void *new_arr;

			new_arr = realloc(info->str_for_id,
					  new_count * sizeof(char *));
			if (!new_arr)
				return log_msg_ret("id", -ENOMEM);
			memset(new_arr + info->str_count, '\0',
			       (new_count - info->str_count) * sizeof(char *));
			info->str_for_id = new_arr;
			info->str_count = new_count;
		}

		info->str_for_id[id] = val;
	}

	return 0;
}

/**
 * list_strings() - List the available strings with their IDs
 *
 * @info: Build information
 */
static void list_strings(struct build_info *info)
{
	int i;

	for (i = 0; i < info->str_count; i++) {
		if (info->str_for_id[i])
			printf("%3d %s\n", i, info->str_for_id[i]);
	}
}

/**
 * menu_build() - Build a menu and add it to a scene
 *
 * See doc/develop/expo.rst for a description of the format
 *
 * @info: Build information
 * @node: Node containing the menu description
 * @scn: Scene to add the menu to
 * @id: ID for the menu
 * @objp: Returns the object pointer
 * Returns: 0 if OK, -ENOMEM if out of memory, -EINVAL if there is a format
 * error, -ENOENT if there is a references to a non-existent string
 */
static int menu_build(struct build_info *info, ofnode node, struct scene *scn,
		      uint id, struct scene_obj **objp)
{
	struct scene_obj_menu *menu;
	uint title_id, menu_id;
	const u32 *item_ids;
	int ret, size, i;
	const char *name;

	name = ofnode_get_name(node);

	ret = scene_menu(scn, name, id, &menu);
	if (ret < 0)
		return log_msg_ret("men", ret);
	menu_id = ret;

	/* Set the title */
	ret = add_txt_str(info, node, scn, "title", 0);
	if (ret < 0)
		return log_msg_ret("tit", ret);
	title_id = ret;
	ret = scene_menu_set_title(scn, menu_id, title_id);
	if (ret)
		return log_msg_ret("set", ret);

	item_ids = ofnode_read_prop(node, "item-id", &size);
	if (!item_ids)
		return log_msg_ret("itm", -EINVAL);
	if (!size || size % sizeof(u32))
		return log_msg_ret("isz", -EINVAL);
	size /= sizeof(u32);

	for (i = 0; i < size; i++) {
		struct scene_menitem *item;
		uint label, key, desc;

		ret = add_txt_str_list(info, node, scn, "item-label", i, 0);
		if (ret < 0 && ret != -ENOENT)
			return log_msg_ret("lab", ret);
		label = max(0, ret);

		ret = add_txt_str_list(info, node, scn, "key-label", i, 0);
		if (ret < 0 && ret != -ENOENT)
			return log_msg_ret("key", ret);
		key = max(0, ret);

		ret = add_txt_str_list(info, node, scn, "desc-label", i, 0);
		if (ret < 0  && ret != -ENOENT)
			return log_msg_ret("lab", ret);
		desc = max(0, ret);

		ret = scene_menuitem(scn, menu_id, simple_xtoa(i),
				     fdt32_to_cpu(item_ids[i]), key, label,
				     desc, 0, 0, &item);
		if (ret < 0)
			return log_msg_ret("mi", ret);
	}
	*objp = &menu->obj;

	return 0;
}

static int textline_build(struct build_info *info, ofnode node,
			  struct scene *scn, uint id, struct scene_obj **objp)
{
	struct scene_obj_textline *ted;
	uint ted_id, edit_id;
	const char *name;
	u32 max_chars;
	int ret;

	name = ofnode_get_name(node);

	info->err_prop = "max-chars";
	ret = ofnode_read_u32(node, "max-chars", &max_chars);
	if (ret)
		return log_msg_ret("max", -ENOENT);

	ret = scene_textline(scn, name, id, max_chars, &ted);
	if (ret < 0)
		return log_msg_ret("ted", ret);
	ted_id = ret;

	/* Set the title */
	ret = add_txt_str(info, node, scn, "title", 0);
	if (ret < 0)
		return log_msg_ret("tit", ret);
	ted->label_id = ret;

	/* Setup the editor */
	info->err_prop = "edit-id";
	ret = ofnode_read_u32(node, "edit-id", &id);
	if (ret)
		return log_msg_ret("id", -ENOENT);
	edit_id = ret;

	ret = scene_txt_str(scn, "edit", edit_id, 0, abuf_data(&ted->buf),
			    NULL);
	if (ret < 0)
		return log_msg_ret("add", ret);
	ted->edit_id = ret;

	return 0;
}

/**
 * obj_build() - Build an expo object and add it to a scene
 *
 * See doc/develop/expo.rst for a description of the format
 *
 * @info: Build information
 * @node: Node containing the object description
 * @scn: Scene to add the object to
 * Returns: 0 if OK, -ENOMEM if out of memory, -EINVAL if there is a format
 * error, -ENOENT if there is a references to a non-existent string
 */
static int obj_build(struct build_info *info, ofnode node, struct scene *scn)
{
	struct scene_obj *obj;
	const char *type;
	u32 id, val;
	int ret;

	log_debug("- object %s\n", ofnode_get_name(node));
	ret = ofnode_read_u32(node, "id", &id);
	if (ret)
		return log_msg_ret("id", -ENOENT);

	type = ofnode_read_string(node, "type");
	if (!type)
		return log_msg_ret("typ", -EINVAL);

	if (!strcmp("menu", type))
		ret = menu_build(info, node, scn, id, &obj);
	else if (!strcmp("textline", type))
		ret = textline_build(info, node, scn, id, &obj);
	else
		ret = -EOPNOTSUPP;
	if (ret)
		return log_msg_ret("bld", ret);

	if (!ofnode_read_u32(node, "start-bit", &val))
		obj->start_bit = val;
	if (!ofnode_read_u32(node, "bit-length", &val))
		obj->bit_length = val;

	return 0;
}

/**
 * scene_build() - Build a scene and all its objects
 *
 * See doc/develop/expo.rst for a description of the format
 *
 * @info: Build information
 * @node: Node containing the scene description
 * @scn: Scene to add the object to
 * Returns: 0 if OK, -ENOMEM if out of memory, -EINVAL if there is a format
 * error, -ENOENT if there is a references to a non-existent string
 */
static int scene_build(struct build_info *info, ofnode scn_node,
		       struct expo *exp)
{
	const char *name;
	struct scene *scn;
	uint id, title_id;
	ofnode node;
	int ret;

	info->err_node = scn_node;
	name = ofnode_get_name(scn_node);
	log_debug("Building scene %s\n", name);
	ret = ofnode_read_u32(scn_node, "id", &id);
	if (ret)
		return log_msg_ret("id", -ENOENT);

	ret = scene_new(exp, name, id, &scn);
	if (ret < 0)
		return log_msg_ret("scn", ret);

	ret = add_txt_str(info, scn_node, scn, "title", 0);
	if (ret < 0)
		return log_msg_ret("tit", ret);
	title_id = ret;
	scene_title_set(scn, title_id);

	ret = add_txt_str(info, scn_node, scn, "prompt", 0);
	if (ret < 0)
		return log_msg_ret("pr", ret);

	ofnode_for_each_subnode(node, scn_node) {
		info->err_node = node;
		ret = obj_build(info, node, scn);
		if (ret < 0)
			return log_msg_ret("mit", ret);
	}

	return 0;
}

int build_it(struct build_info *info, ofnode root, struct expo **expp)
{
	ofnode scenes, node;
	struct expo *exp;
	u32 dyn_start;
	int ret;

	ret = read_strings(info, root);
	if (ret)
		return log_msg_ret("str", ret);
	if (_DEBUG)
		list_strings(info);
	info->err_node = root;

	ret = expo_new("name", NULL, &exp);
	if (ret)
		return log_msg_ret("exp", ret);

	if (!ofnode_read_u32(root, "dynamic-start", &dyn_start))
		expo_set_dynamic_start(exp, dyn_start);

	scenes = ofnode_find_subnode(root, "scenes");
	if (!ofnode_valid(scenes))
		return log_msg_ret("sno", -EINVAL);

	ofnode_for_each_subnode(node, scenes) {
		ret = scene_build(info, node, exp);
		if (ret < 0)
			return log_msg_ret("scn", ret);
	}
	*expp = exp;

	return 0;
}

int expo_build(ofnode root, struct expo **expp)
{
	struct build_info info;
	struct expo *exp;
	int ret;

	memset(&info, '\0', sizeof(info));
	ret = build_it(&info, root, &exp);
	if (ret) {
		char buf[120];
		int node_ret;

		node_ret = ofnode_get_path(info.err_node, buf, sizeof(buf));
		log_warning("Build failed at node %s, property %s\n",
			    node_ret ? ofnode_get_name(info.err_node) : buf,
			    info.err_prop);

		return log_msg_ret("bui", ret);
	}
	*expp = exp;

	return 0;
}
