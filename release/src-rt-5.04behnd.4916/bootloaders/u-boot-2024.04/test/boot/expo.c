// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <expo.h>
#include <menu.h>
#include <video.h>
#include <linux/input.h>
#include <test/suites.h>
#include <test/ut.h>
#include "bootstd_common.h"
#include <test/cedit-test.h>
#include "../../boot/scene_internal.h"

enum {
	/* scenes */
	SCENE1		= 7,
	SCENE2,

	/* objects */
	OBJ_LOGO,
	OBJ_TEXT,
	OBJ_TEXT2,
	OBJ_MENU,
	OBJ_MENU_TITLE,

	/* strings */
	STR_SCENE_TITLE,

	STR_TEXT,
	STR_TEXT2,
	STR_MENU_TITLE,
	STR_POINTER_TEXT,

	STR_ITEM1_LABEL,
	STR_ITEM1_DESC,
	STR_ITEM1_KEY,
	STR_ITEM1_PREVIEW,

	STR_ITEM2_LABEL,
	STR_ITEM2_DESC,
	STR_ITEM2_KEY,
	STR_ITEM2_PREVIEW,

	/* menu items */
	ITEM1,
	ITEM1_LABEL,
	ITEM1_DESC,
	ITEM1_KEY,
	ITEM1_PREVIEW,

	ITEM2,
	ITEM2_LABEL,
	ITEM2_DESC,
	ITEM2_KEY,
	ITEM2_PREVIEW,

	/* pointer to current item */
	POINTER_TEXT,
};

#define BAD_POINTER	((void *)1)

/* names for various things */
#define EXPO_NAME	"my menus"
#define SCENE_NAME1	"main"
#define SCENE_NAME2	"second"
#define SCENE_TITLE	"Main Menu"
#define LOGO_NAME	"logo"

/* Check base expo support */
static int expo_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct expo *exp;
	ulong start_mem;
	char name[100];
	int i;

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));

	start_mem = ut_check_free();

	exp = NULL;
	strcpy(name, EXPO_NAME);
	ut_assertok(expo_new(name, NULL, &exp));
	*name = '\0';
	ut_assertnonnull(exp);
	ut_asserteq(0, exp->scene_id);
	ut_asserteq(0, exp->next_id);

	/* Make sure the name was allocated */
	ut_assertnonnull(exp->name);
	ut_asserteq_str(EXPO_NAME, exp->name);

	ut_assertok(expo_set_display(exp, dev));
	expo_destroy(exp);
	ut_assertok(ut_check_delta(start_mem));

	/* test handling out-of-memory conditions */
	for (i = 0; i < 2; i++) {
		struct expo *exp2;

		malloc_enable_testing(i);
		exp2 = BAD_POINTER;
		ut_asserteq(-ENOMEM, expo_new(EXPO_NAME, NULL, &exp2));
		ut_asserteq_ptr(BAD_POINTER, exp2);
		malloc_disable_testing();
	}

	return 0;
}
BOOTSTD_TEST(expo_base, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check creating a scene */
static int expo_scene(struct unit_test_state *uts)
{
	struct scene *scn;
	struct expo *exp;
	ulong start_mem;
	char name[100];
	int id, title_id;

	start_mem = ut_check_free();

	ut_assertok(expo_new(EXPO_NAME, NULL, &exp));

	scn = NULL;
	ut_asserteq(0, exp->next_id);
	strcpy(name, SCENE_NAME1);
	id = scene_new(exp, name, SCENE1, &scn);
	*name = '\0';
	ut_assertnonnull(scn);
	ut_asserteq(SCENE1, id);
	ut_asserteq(SCENE1 + 1, exp->next_id);
	ut_asserteq_ptr(exp, scn->expo);

	/* Make sure the name was allocated */
	ut_assertnonnull(scn->name);
	ut_asserteq_str(SCENE_NAME1, scn->name);

	/* Set the title */
	title_id = expo_str(exp, "title", STR_SCENE_TITLE, SCENE_TITLE);
	ut_assert(title_id >= 0);

	/* Use an allocated ID - this will be allocated after the title str */
	scn = NULL;
	id = scene_new(exp, SCENE_NAME2, 0, &scn);
	ut_assertnonnull(scn);
	ut_assertok(scene_title_set(scn, title_id));
	ut_asserteq(STR_SCENE_TITLE + 1, id);
	ut_asserteq(STR_SCENE_TITLE + 2, exp->next_id);
	ut_asserteq_ptr(exp, scn->expo);

	ut_asserteq_str(SCENE_NAME2, scn->name);
	ut_asserteq(title_id, scn->title_id);

	expo_destroy(exp);

	ut_assertok(ut_check_delta(start_mem));

	return 0;
}
BOOTSTD_TEST(expo_scene, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check creating a scene with objects */
static int expo_object(struct unit_test_state *uts)
{
	struct scene_obj_img *img;
	struct scene_obj_txt *txt;
	struct scene *scn;
	struct expo *exp;
	ulong start_mem;
	char name[100];
	char *data;
	int id;

	start_mem = ut_check_free();

	ut_assertok(expo_new(EXPO_NAME, NULL, &exp));
	id = scene_new(exp, SCENE_NAME1, SCENE1, &scn);
	ut_assert(id > 0);

	ut_asserteq(0, scene_obj_count(scn));

	data = NULL;
	strcpy(name, LOGO_NAME);
	id = scene_img(scn, name, OBJ_LOGO, data, &img);
	ut_assert(id > 0);
	*name = '\0';
	ut_assertnonnull(img);
	ut_asserteq(OBJ_LOGO, id);
	ut_asserteq(OBJ_LOGO + 1, exp->next_id);
	ut_asserteq_ptr(scn, img->obj.scene);
	ut_asserteq(SCENEOBJT_IMAGE, img->obj.type);

	ut_asserteq_ptr(data, img->data);

	/* Make sure the name was allocated */
	ut_assertnonnull(scn->name);
	ut_asserteq_str(SCENE_NAME1, scn->name);

	ut_asserteq(1, scene_obj_count(scn));

	id = scene_txt_str(scn, "text", OBJ_TEXT, STR_TEXT, "my string", &txt);
	ut_assert(id > 0);
	ut_assertnonnull(txt);
	ut_asserteq(OBJ_TEXT, id);
	ut_asserteq(SCENEOBJT_TEXT, txt->obj.type);
	ut_asserteq(2, scene_obj_count(scn));

	/* Check passing NULL as the final parameter */
	id = scene_txt_str(scn, "text2", OBJ_TEXT2, STR_TEXT2, "another string",
			   NULL);
	ut_assert(id > 0);
	ut_asserteq(3, scene_obj_count(scn));

	expo_destroy(exp);

	ut_assertok(ut_check_delta(start_mem));

	return 0;
}
BOOTSTD_TEST(expo_object, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check setting object attributes and using themes */
static int expo_object_attr(struct unit_test_state *uts)
{
	struct scene_obj_menu *menu;
	struct scene_obj_img *img;
	struct scene_obj_txt *txt;
	struct scene *scn;
	struct expo *exp;
	ulong start_mem;
	char name[100];
	ofnode node;
	char *data;
	int id;

	start_mem = ut_check_free();

	ut_assertok(expo_new(EXPO_NAME, NULL, &exp));
	id = scene_new(exp, SCENE_NAME1, SCENE1, &scn);
	ut_assert(id > 0);

	data = NULL;
	id = scene_img(scn, LOGO_NAME, OBJ_LOGO, data, &img);
	ut_assert(id > 0);

	ut_assertok(scene_obj_set_pos(scn, OBJ_LOGO, 123, 456));
	ut_asserteq(123, img->obj.dim.x);
	ut_asserteq(456, img->obj.dim.y);

	ut_asserteq(-ENOENT, scene_obj_set_pos(scn, OBJ_TEXT2, 0, 0));

	id = scene_txt_str(scn, "text", OBJ_TEXT, STR_TEXT, "my string", &txt);
	ut_assert(id > 0);

	strcpy(name, "font2");
	ut_assertok(scene_txt_set_font(scn, OBJ_TEXT, name, 42));
	ut_asserteq_ptr(name, txt->font_name);
	ut_asserteq(42, txt->font_size);

	ut_asserteq(-ENOENT, scene_txt_set_font(scn, OBJ_TEXT2, name, 42));

	id = scene_menu(scn, "main", OBJ_MENU, &menu);
	ut_assert(id > 0);

	ut_assertok(scene_menu_set_title(scn, OBJ_MENU, OBJ_TEXT));

	ut_asserteq(-ENOENT, scene_menu_set_title(scn, OBJ_TEXT2, OBJ_TEXT));
	ut_asserteq(-EINVAL, scene_menu_set_title(scn, OBJ_MENU, OBJ_TEXT2));

	node = ofnode_path("/bootstd/theme");
	ut_assert(ofnode_valid(node));
	ut_assertok(expo_apply_theme(exp, node));
	ut_asserteq(30, txt->font_size);

	expo_destroy(exp);

	ut_assertok(ut_check_delta(start_mem));

	return 0;
}
BOOTSTD_TEST(expo_object_attr, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/**
 * struct test_iter_priv - private data for expo-iterator test
 *
 * @count: number of scene objects
 * @menu_count: number of menus
 * @fail_at: item ID at which to return an error
 */
struct test_iter_priv {
	int count;
	int menu_count;
	int fail_at;
};

int h_test_iter(struct scene_obj *obj, void *vpriv)
{
	struct test_iter_priv *priv = vpriv;

	if (priv->fail_at == obj->id)
		return -EINVAL;

	priv->count++;
	if (obj->type == SCENEOBJT_MENU)
		priv->menu_count++;

	return 0;
}

/* Check creating a scene with a menu */
static int expo_object_menu(struct unit_test_state *uts)
{
	struct scene_obj_menu *menu;
	struct scene_menitem *item;
	int id, label_id, desc_id, key_id, pointer_id, preview_id;
	struct scene_obj_txt *ptr, *name1, *desc1, *key1, *tit, *prev1;
	struct test_iter_priv priv;
	struct scene *scn;
	struct expo *exp;
	ulong start_mem;

	start_mem = ut_check_free();

	ut_assertok(expo_new(EXPO_NAME, NULL, &exp));
	id = scene_new(exp, SCENE_NAME1, SCENE1, &scn);
	ut_assert(id > 0);

	id = scene_menu(scn, "main", OBJ_MENU, &menu);
	ut_assert(id > 0);
	ut_assertnonnull(menu);
	ut_asserteq(OBJ_MENU, id);
	ut_asserteq(SCENEOBJT_MENU, menu->obj.type);
	ut_asserteq(0, menu->title_id);
	ut_asserteq(0, menu->pointer_id);

	ut_assertok(scene_obj_set_pos(scn, OBJ_MENU, 50, 400));
	ut_asserteq(50, menu->obj.dim.x);
	ut_asserteq(400, menu->obj.dim.y);

	id = scene_txt_str(scn, "title", OBJ_MENU_TITLE, STR_MENU_TITLE,
			   "Main Menu", &tit);
	ut_assert(id > 0);
	ut_assertok(scene_menu_set_title(scn, OBJ_MENU, OBJ_MENU_TITLE));
	ut_asserteq(OBJ_MENU_TITLE, menu->title_id);

	pointer_id = scene_txt_str(scn, "cur_item", POINTER_TEXT,
				   STR_POINTER_TEXT, ">", &ptr);
	ut_assert(pointer_id > 0);

	ut_assertok(scene_menu_set_pointer(scn, OBJ_MENU, POINTER_TEXT));
	ut_asserteq(POINTER_TEXT, menu->pointer_id);

	label_id = scene_txt_str(scn, "label1", ITEM1_LABEL, STR_ITEM1_LABEL,
				 "Play", &name1);
	ut_assert(label_id > 0);

	desc_id = scene_txt_str(scn, "desc1", ITEM1_DESC, STR_ITEM1_DESC,
				"Lord Melchett", &desc1);
	ut_assert(desc_id > 0);

	key_id = scene_txt_str(scn, "item1-key", ITEM1_KEY, STR_ITEM1_KEY, "1",
			       &key1);
	ut_assert(key_id > 0);

	preview_id = scene_txt_str(scn, "item1-preview", ITEM1_PREVIEW,
				   STR_ITEM1_PREVIEW, "(preview1)", &prev1);
	ut_assert(preview_id > 0);

	id = scene_menuitem(scn, OBJ_MENU, "linux", ITEM1, ITEM1_KEY,
			    ITEM1_LABEL, ITEM1_DESC, ITEM1_PREVIEW, 0, &item);
	ut_asserteq(ITEM1, id);
	ut_asserteq(id, item->id);
	ut_asserteq(key_id, item->key_id);
	ut_asserteq(label_id, item->label_id);
	ut_asserteq(desc_id, item->desc_id);
	ut_asserteq(preview_id, item->preview_id);

	ut_assertok(scene_arrange(scn));

	/* arranging the scene should cause the first item to become current */
	ut_asserteq(id, menu->cur_item_id);

	/* the title should be at the top */
	ut_asserteq(menu->obj.dim.x, tit->obj.dim.x);
	ut_asserteq(menu->obj.dim.y, tit->obj.dim.y);

	/* the first item should be next */
	ut_asserteq(menu->obj.dim.x, name1->obj.dim.x);
	ut_asserteq(menu->obj.dim.y + 32, name1->obj.dim.y);

	ut_asserteq(menu->obj.dim.x + 230, key1->obj.dim.x);
	ut_asserteq(menu->obj.dim.y + 32, key1->obj.dim.y);

	ut_asserteq(menu->obj.dim.x + 200, ptr->obj.dim.x);
	ut_asserteq(menu->obj.dim.y + 32, ptr->obj.dim.y);

	ut_asserteq(menu->obj.dim.x + 280, desc1->obj.dim.x);
	ut_asserteq(menu->obj.dim.y + 32, desc1->obj.dim.y);

	ut_asserteq(-4, prev1->obj.dim.x);
	ut_asserteq(menu->obj.dim.y + 32, prev1->obj.dim.y);
	ut_asserteq(true, prev1->obj.flags & SCENEOF_HIDE);

	/* check iterating through scene items */
	memset(&priv, '\0', sizeof(priv));
	ut_assertok(expo_iter_scene_objs(exp, h_test_iter, &priv));
	ut_asserteq(7, priv.count);
	ut_asserteq(1, priv.menu_count);

	/* check the iterator failing part way through iteration */
	memset(&priv, '\0', sizeof(priv));
	priv.fail_at = key_id;
	ut_asserteq(-EINVAL, expo_iter_scene_objs(exp, h_test_iter, &priv));

	/* 2 items (preview_id and the menuitem) are after key_id, 7 - 2 = 5 */
	ut_asserteq(5, priv.count);

	/* menu is first, so is still processed */
	ut_asserteq(1, priv.menu_count);

	expo_destroy(exp);

	ut_assertok(ut_check_delta(start_mem));

	return 0;
}
BOOTSTD_TEST(expo_object_menu, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check rendering a scene */
static int expo_render_image(struct unit_test_state *uts)
{
	struct scene_obj_menu *menu;
	struct scene *scn, *scn2;
	struct expo_action act;
	struct scene_obj *obj;
	struct udevice *dev;
	struct expo *exp;
	int id;

	console_record_reset_enable();
	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));

	ut_assertok(expo_new(EXPO_NAME, NULL, &exp));
	id = scene_new(exp, SCENE_NAME1, SCENE1, &scn);
	ut_assert(id > 0);
	ut_assertok(expo_set_display(exp, dev));

	id = scene_img(scn, "logo", OBJ_LOGO, video_get_u_boot_logo(), NULL);
	ut_assert(id > 0);
	ut_assertok(scene_obj_set_pos(scn, OBJ_LOGO, 50, 20));

	id = scene_txt_str(scn, "text", OBJ_TEXT, STR_TEXT, "my string", NULL);
	ut_assert(id > 0);
	ut_assertok(scene_txt_set_font(scn, OBJ_TEXT, "cantoraone_regular",
				       40));
	ut_assertok(scene_obj_set_pos(scn, OBJ_TEXT, 400, 100));

	id = scene_txt_str(scn, "text", OBJ_TEXT2, STR_TEXT2, "another string",
			   NULL);
	ut_assert(id > 0);
	ut_assertok(scene_txt_set_font(scn, OBJ_TEXT2, "nimbus_sans_l_regular",
				       60));
	ut_assertok(scene_obj_set_pos(scn, OBJ_TEXT2, 200, 600));

	id = scene_menu(scn, "main", OBJ_MENU, &menu);
	ut_assert(id > 0);

	id = scene_txt_str(scn, "title", OBJ_MENU_TITLE, STR_MENU_TITLE,
			   "Main Menu", NULL);
	ut_assert(id > 0);
	ut_assertok(scene_menu_set_title(scn, OBJ_MENU, OBJ_MENU_TITLE));

	id = scene_txt_str(scn, "cur_item", POINTER_TEXT, STR_POINTER_TEXT, ">",
			   NULL);
	ut_assert(id > 0);
	ut_assertok(scene_menu_set_pointer(scn, OBJ_MENU, POINTER_TEXT));

	id = scene_txt_str(scn, "label1", ITEM1_LABEL, STR_ITEM1_LABEL, "Play",
			   NULL);
	ut_assert(id > 0);
	id = scene_txt_str(scn, "item1 txt", ITEM1_DESC, STR_ITEM1_DESC,
			   "Lord Melchett", NULL);
	ut_assert(id > 0);
	id = scene_txt_str(scn, "item1-key", ITEM1_KEY, STR_ITEM1_KEY, "1",
			   NULL);
	ut_assert(id > 0);
	id = scene_img(scn, "item1-preview", ITEM1_PREVIEW,
		       video_get_u_boot_logo(), NULL);
	id = scene_menuitem(scn, OBJ_MENU, "item1", ITEM1, ITEM1_KEY,
			    ITEM1_LABEL, ITEM1_DESC, ITEM1_PREVIEW, 0, NULL);
	ut_assert(id > 0);

	id = scene_txt_str(scn, "label2", ITEM2_LABEL, STR_ITEM2_LABEL, "Now",
			   NULL);
	ut_assert(id > 0);
	id = scene_txt_str(scn, "item2 txt", ITEM2_DESC, STR_ITEM2_DESC,
			   "Lord Percy", NULL);
	ut_assert(id > 0);
	id = scene_txt_str(scn, "item2-key", ITEM2_KEY, STR_ITEM2_KEY, "2",
			   NULL);
	ut_assert(id > 0);
	id = scene_img(scn, "item2-preview", ITEM2_PREVIEW,
		       video_get_u_boot_logo(), NULL);
	ut_assert(id > 0);

	id = scene_menuitem(scn, OBJ_MENU, "item2", ITEM2, ITEM2_KEY,
			    ITEM2_LABEL, ITEM2_DESC, ITEM2_PREVIEW, 0, NULL);
	ut_assert(id > 0);

	ut_assertok(scene_obj_set_pos(scn, OBJ_MENU, 50, 400));

	scn2 = expo_lookup_scene_id(exp, SCENE1);
	ut_asserteq_ptr(scn, scn2);
	scn2 = expo_lookup_scene_id(exp, SCENE2);
	ut_assertnull(scn2);

	/* render without a scene */
	ut_asserteq(-ECHILD, expo_render(exp));

	ut_assertok(expo_calc_dims(exp));
	ut_assertok(scene_arrange(scn));

	/* check dimensions of text */
	obj = scene_obj_find(scn, OBJ_TEXT, SCENEOBJT_NONE);
	ut_assertnonnull(obj);
	ut_asserteq(400, obj->dim.x);
	ut_asserteq(100, obj->dim.y);
	ut_asserteq(126, obj->dim.w);
	ut_asserteq(40, obj->dim.h);

	/* check dimensions of image */
	obj = scene_obj_find(scn, OBJ_LOGO, SCENEOBJT_NONE);
	ut_assertnonnull(obj);
	ut_asserteq(50, obj->dim.x);
	ut_asserteq(20, obj->dim.y);
	ut_asserteq(160, obj->dim.w);
	ut_asserteq(160, obj->dim.h);

	/* check dimensions of menu labels - both should be the same width */
	obj = scene_obj_find(scn, ITEM1_LABEL, SCENEOBJT_NONE);
	ut_assertnonnull(obj);
	ut_asserteq(50, obj->dim.x);
	ut_asserteq(436, obj->dim.y);
	ut_asserteq(29, obj->dim.w);
	ut_asserteq(18, obj->dim.h);

	obj = scene_obj_find(scn, ITEM2_LABEL, SCENEOBJT_NONE);
	ut_assertnonnull(obj);
	ut_asserteq(50, obj->dim.x);
	ut_asserteq(454, obj->dim.y);
	ut_asserteq(29, obj->dim.w);
	ut_asserteq(18, obj->dim.h);

	/* check dimensions of menu */
	obj = scene_obj_find(scn, OBJ_MENU, SCENEOBJT_NONE);
	ut_assertnonnull(obj);
	ut_asserteq(50, obj->dim.x);
	ut_asserteq(400, obj->dim.y);
	ut_asserteq(160, obj->dim.w);
	ut_asserteq(160, obj->dim.h);

	/* render it */
	expo_set_scene_id(exp, SCENE1);
	ut_assertok(expo_render(exp));

	/* move down */
	ut_assertok(expo_send_key(exp, BKEY_DOWN));

	ut_assertok(expo_action_get(exp, &act));

	ut_asserteq(EXPOACT_POINT_ITEM, act.type);
	ut_asserteq(ITEM2, act.select.id);
	ut_assertok(expo_render(exp));

	/* make sure only the preview for the second item is shown */
	obj = scene_obj_find(scn, ITEM1_PREVIEW, SCENEOBJT_NONE);
	ut_asserteq(true, obj->flags & SCENEOF_HIDE);

	obj = scene_obj_find(scn, ITEM2_PREVIEW, SCENEOBJT_NONE);
	ut_asserteq(false, obj->flags & SCENEOF_HIDE);

	/* select it */
	ut_assertok(expo_send_key(exp, BKEY_SELECT));

	ut_assertok(expo_action_get(exp, &act));
	ut_asserteq(EXPOACT_SELECT, act.type);
	ut_asserteq(ITEM2, act.select.id);

	/* make sure the action doesn't come again */
	ut_asserteq(-EAGAIN, expo_action_get(exp, &act));

	/* make sure there was no console output */
	ut_assert_console_end();

	/* now try in text mode */
	expo_set_text_mode(exp, true);
	ut_assertok(expo_render(exp));

	ut_assert_nextline("U-Boot    :    Boot Menu");
	ut_assert_nextline("%s", "");
	ut_assert_nextline("Main Menu");
	ut_assert_nextline("%s", "");
	ut_assert_nextline("       1  Play        Lord Melchett");
	ut_assert_nextline("  >    2  Now         Lord Percy");

	/* Move back up to the first item */
	ut_assertok(expo_send_key(exp, BKEY_UP));

	ut_assertok(expo_action_get(exp, &act));

	ut_asserteq(EXPOACT_POINT_ITEM, act.type);
	ut_asserteq(ITEM1, act.select.id);

	ut_assertok(expo_render(exp));
	ut_assert_nextline("U-Boot    :    Boot Menu");
	ut_assert_nextline("%s", "");
	ut_assert_nextline("Main Menu");
	ut_assert_nextline("%s", "");
	ut_assert_nextline("  >    1  Play        Lord Melchett");
	ut_assert_nextline("       2  Now         Lord Percy");

	ut_assert_console_end();

	expo_destroy(exp);

	return 0;
}
BOOTSTD_TEST(expo_render_image, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check building an expo from a devicetree description */
static int expo_test_build(struct unit_test_state *uts)
{
	struct scene_obj_menu *menu;
	struct scene_menitem *item;
	struct scene_obj_txt *txt;
	struct scene_obj *obj;
	struct scene *scn;
	struct expo *exp;
	int count;
	ofnode node;

	node = ofnode_path("/cedit");
	ut_assert(ofnode_valid(node));
	ut_assertok(expo_build(node, &exp));

	ut_asserteq_str("name", exp->name);
	ut_asserteq(0, exp->scene_id);
	ut_asserteq(ID_DYNAMIC_START + 24, exp->next_id);
	ut_asserteq(false, exp->popup);

	/* check the scene */
	scn = expo_lookup_scene_id(exp, ID_SCENE1);
	ut_assertnonnull(scn);
	ut_asserteq_str("main", scn->name);
	ut_asserteq(ID_SCENE1, scn->id);
	ut_asserteq(ID_DYNAMIC_START + 1, scn->title_id);
	ut_asserteq(0, scn->highlight_id);

	/* check the title */
	txt = scene_obj_find(scn, scn->title_id, SCENEOBJT_NONE);
	ut_assertnonnull(txt);
	obj = &txt->obj;
	ut_asserteq_ptr(scn, obj->scene);
	ut_asserteq_str("title", obj->name);
	ut_asserteq(scn->title_id, obj->id);
	ut_asserteq(SCENEOBJT_TEXT, obj->type);
	ut_asserteq(0, obj->flags);
	ut_asserteq_str("Test Configuration", expo_get_str(exp, txt->str_id));

	/* check the menu */
	menu = scene_obj_find(scn, ID_CPU_SPEED, SCENEOBJT_NONE);
	obj = &menu->obj;
	ut_asserteq_ptr(scn, obj->scene);
	ut_asserteq_str("cpu-speed", obj->name);
	ut_asserteq(ID_CPU_SPEED, obj->id);
	ut_asserteq(SCENEOBJT_MENU, obj->type);
	ut_asserteq(0, obj->flags);

	txt = scene_obj_find(scn, menu->title_id, SCENEOBJT_NONE);
	ut_asserteq_str("CPU speed", expo_get_str(exp, txt->str_id));

	ut_asserteq(0, menu->cur_item_id);
	ut_asserteq(0, menu->pointer_id);

	/* check the items */
	item = list_first_entry(&menu->item_head, struct scene_menitem,
				sibling);
	ut_asserteq_str("00", item->name);
	ut_asserteq(ID_CPU_SPEED_1, item->id);
	ut_asserteq(0, item->key_id);
	ut_asserteq(0, item->desc_id);
	ut_asserteq(0, item->preview_id);
	ut_asserteq(0, item->flags);

	txt = scene_obj_find(scn, item->label_id, SCENEOBJT_NONE);
	ut_asserteq_str("2 GHz", expo_get_str(exp, txt->str_id));

	count = 0;
	list_for_each_entry(item, &menu->item_head, sibling)
		count++;
	ut_asserteq(3, count);

	expo_destroy(exp);

	return 0;
}
BOOTSTD_TEST(expo_test_build, UT_TESTF_DM);
