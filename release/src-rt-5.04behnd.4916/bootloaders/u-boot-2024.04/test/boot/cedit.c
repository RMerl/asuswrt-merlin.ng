// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <cedit.h>
#include <env.h>
#include <expo.h>
#include <mapmem.h>
#include <dm/ofnode.h>
#include <test/ut.h>
#include "bootstd_common.h"
#include <test/cedit-test.h>
#include "../../boot/scene_internal.h"

/* Check the cedit command */
static int cedit_base(struct unit_test_state *uts)
{
	extern struct expo *cur_exp;
	struct scene_obj_menu *menu;
	struct scene_obj_txt *txt;
	struct expo *exp;
	struct scene *scn;

	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	console_record_reset_enable();

	/*
	 * ^N  Move down to second menu
	 * ^M  Open menu
	 * ^N  Move down to second item
	 * ^M  Select item
	 * \e  Quit
	 */
	console_in_puts("\x0e\x0d\x0e\x0d\e");
	ut_assertok(run_command("cedit run", 0));

	exp = cur_exp;
	scn = expo_lookup_scene_id(exp, exp->scene_id);
	ut_assertnonnull(scn);

	menu = scene_obj_find(scn, scn->highlight_id, SCENEOBJT_NONE);
	ut_assertnonnull(menu);

	txt = scene_obj_find(scn, menu->title_id, SCENEOBJT_NONE);
	ut_assertnonnull(txt);
	ut_asserteq_str("AC Power", expo_get_str(exp, txt->str_id));

	ut_asserteq(ID_AC_ON, menu->cur_item_id);

	return 0;
}
BOOTSTD_TEST(cedit_base, 0);

/* Check the cedit write_fdt and read_fdt commands */
static int cedit_fdt(struct unit_test_state *uts)
{
	struct scene_obj_textline *tline;
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	struct scene_obj_menu *menu;
	ulong addr = 0x1000;
	struct ofprop prop;
	struct scene *scn;
	oftree tree;
	ofnode node;
	char *str;
	void *fdt;
	int i;

	console_record_reset_enable();
	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	ut_asserteq(ID_SCENE1, cedit_prepare(cur_exp, &vid_priv, &scn));

	/* get a menu to fiddle with */
	menu = scene_obj_find(scn, ID_CPU_SPEED, SCENEOBJT_MENU);
	ut_assertnonnull(menu);
	menu->cur_item_id = ID_CPU_SPEED_2;

	/* get a textline to fiddle with too */
	tline = scene_obj_find(scn, ID_MACHINE_NAME, SCENEOBJT_TEXTLINE);
	ut_assertnonnull(tline);
	str = abuf_data(&tline->buf);
	strcpy(str, "my-machine");

	ut_assertok(run_command("cedit write_fdt hostfs - settings.dtb", 0));
	ut_assertok(run_commandf("load hostfs - %lx settings.dtb", addr));
	ut_assert_nextlinen("1024 bytes read");

	fdt = map_sysmem(addr, 1024);
	tree = oftree_from_fdt(fdt);
	node = ofnode_find_subnode(oftree_root(tree), CEDIT_NODE_NAME);
	ut_assert(ofnode_valid(node));

	ut_asserteq(ID_CPU_SPEED_2,
		    ofnode_read_u32_default(node, "cpu-speed", 0));
	ut_asserteq_str("2.5 GHz", ofnode_read_string(node, "cpu-speed-str"));
	ut_asserteq_str("my-machine", ofnode_read_string(node, "machine-name"));

	/* There should only be 5 properties */
	for (i = 0, ofnode_first_property(node, &prop); ofprop_valid(&prop);
	     i++, ofnode_next_property(&prop))
		;
	ut_asserteq(5, i);

	ut_assert_console_end();

	/* reset the expo */
	menu->cur_item_id = ID_CPU_SPEED_1;
	*str = '\0';

	/* load in the settings and make sure they update */
	ut_assertok(run_command("cedit read_fdt hostfs - settings.dtb", 0));
	ut_asserteq(ID_CPU_SPEED_2, menu->cur_item_id);
	ut_asserteq_str("my-machine", ofnode_read_string(node, "machine-name"));

	ut_assertnonnull(menu);
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(cedit_fdt, 0);

/* Check the cedit write_env and read_env commands */
static int cedit_env(struct unit_test_state *uts)
{
	struct scene_obj_textline *tline;
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	struct scene_obj_menu *menu;
	struct scene *scn;
	char *str;

	console_record_reset_enable();
	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	ut_asserteq(ID_SCENE1, cedit_prepare(cur_exp, &vid_priv, &scn));

	/* get a menu to fiddle with */
	menu = scene_obj_find(scn, ID_CPU_SPEED, SCENEOBJT_MENU);
	ut_assertnonnull(menu);
	menu->cur_item_id = ID_CPU_SPEED_2;

	/* get a textline to fiddle with too */
	tline = scene_obj_find(scn, ID_MACHINE_NAME, SCENEOBJT_TEXTLINE);
	ut_assertnonnull(tline);
	str = abuf_data(&tline->buf);
	strcpy(str, "my-machine");

	ut_assertok(run_command("cedit write_env -v", 0));
	ut_assert_nextlinen("c.cpu-speed=7");
	ut_assert_nextlinen("c.cpu-speed-str=2.5 GHz");
	ut_assert_nextlinen("c.power-loss=10");
	ut_assert_nextlinen("c.power-loss-str=Always Off");
	ut_assert_nextlinen("c.machine-name=my-machine");
	ut_assert_console_end();

	ut_asserteq(7, env_get_ulong("c.cpu-speed", 10, 0));
	ut_asserteq_str("2.5 GHz", env_get("c.cpu-speed-str"));
	ut_asserteq_str("my-machine", env_get("c.machine-name"));

	/* reset the expo */
	menu->cur_item_id = ID_CPU_SPEED_1;
	*str = '\0';

	ut_assertok(run_command("cedit read_env -v", 0));
	ut_assert_nextlinen("c.cpu-speed=7");
	ut_assert_nextlinen("c.power-loss=10");
	ut_assert_nextlinen("c.machine-name=my-machine");
	ut_assert_console_end();

	ut_asserteq(ID_CPU_SPEED_2, menu->cur_item_id);
	ut_asserteq_str("my-machine", env_get("c.machine-name"));

	return 0;
}
BOOTSTD_TEST(cedit_env, 0);

/* Check the cedit write_cmos and read_cmos commands */
static int cedit_cmos(struct unit_test_state *uts)
{
	struct scene_obj_menu *menu, *menu2;
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	struct scene *scn;

	console_record_reset_enable();
	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	ut_asserteq(ID_SCENE1, cedit_prepare(cur_exp, &vid_priv, &scn));

	/* get the menus to fiddle with */
	menu = scene_obj_find(scn, ID_CPU_SPEED, SCENEOBJT_MENU);
	ut_assertnonnull(menu);
	menu->cur_item_id = ID_CPU_SPEED_2;

	menu2 = scene_obj_find(scn, ID_POWER_LOSS, SCENEOBJT_MENU);
	ut_assertnonnull(menu2);
	menu2->cur_item_id = ID_AC_MEMORY;

	ut_assertok(run_command("cedit write_cmos -v", 0));
	ut_assert_nextlinen("Write 2 bytes from offset 80 to 84");
	ut_assert_console_end();

	/* reset the expo */
	menu->cur_item_id = ID_CPU_SPEED_1;
	menu2->cur_item_id = ID_AC_OFF;

	ut_assertok(run_command("cedit read_cmos -v", 0));
	ut_assert_nextlinen("Read 2 bytes from offset 80 to 84");
	ut_assert_console_end();

	ut_asserteq(ID_CPU_SPEED_2, menu->cur_item_id);
	ut_asserteq(ID_AC_MEMORY, menu2->cur_item_id);

	return 0;
}
BOOTSTD_TEST(cedit_cmos, 0);
