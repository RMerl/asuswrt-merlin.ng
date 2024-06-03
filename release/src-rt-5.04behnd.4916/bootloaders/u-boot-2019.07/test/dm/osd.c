// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <display_options.h>
#include <dm.h>
#include <dm/test.h>
#include <test/ut.h>
#include <video_osd.h>
#include <asm/test.h>

#include "../../drivers/video/sandbox_osd.h"

const uint memsize = 2 * 10 * 10;

static void split(u8 *mem, uint size, u8 *text, u8 *colors)
{
	int i;
	u16 *p = (u16 *)mem;

	for (i = 0; i < size; i++) {
		colors[i] = p[i] % 0x100;
		text[i] = p[i] / 0x100;
	}
}

static void print_mem(u8 *mem, uint width, uint height)
{
	const uint memsize = 2 * 10 * 10;
	u8 colors[memsize / 2];
	u8 text[memsize / 2];
	int i;

	split(mem, memsize / 2, text, colors);

	for (i = 0; i < width * height; i++) {
		printf("%c", text[i]);
		if (i > 0 && ((i + 1) % width) == 0)
			printf("\n");
	}

	printf("\n");

	for (i = 0; i < width * height; i++) {
		printf("%c", colors[i]);
		if (i > 0 && ((i + 1) % width) == 0)
			printf("\n");
	}
}

static int dm_test_osd_basics(struct unit_test_state *uts)
{
	struct udevice *dev;
	u8 mem[memsize + 1];
	u8 colors[memsize / 2];
	u8 text[memsize / 2];
	struct video_osd_info info;

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO_OSD, &dev));

	video_osd_get_info(dev, &info);

	ut_asserteq(10, info.width);
	ut_asserteq(10, info.height);
	ut_asserteq(1, info.major_version);
	ut_asserteq(0, info.minor_version);

	ut_assertok(sandbox_osd_get_mem(dev, mem, memsize));
	split(mem, memsize / 2, text, colors);

	ut_assertok(memcmp(text, "          "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          ", memsize / 2));

	ut_assertok(memcmp(colors, "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk", memsize / 2));

	print_mem(mem, 10, 10);

	ut_assertok(video_osd_print(dev, 1, 1, COLOR_RED, "Blah"));

	ut_assertok(sandbox_osd_get_mem(dev, mem, memsize));
	split(mem, memsize / 2, text, colors);

	ut_assertok(memcmp(text, "          "
				 " Blah     "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          "
				 "          ", memsize / 2));

	ut_assertok(memcmp(colors, "kkkkkkkkkk"
				   "krrrrkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk"
				   "kkkkkkkkkk", memsize / 2));

	print_mem(mem, 10, 10);

	return 0;
}

DM_TEST(dm_test_osd_basics, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_osd_extended(struct unit_test_state *uts)
{
	struct udevice *dev;
	u8 mem[memsize + 1];
	u8 colors[memsize / 2];
	u8 text[memsize / 2];
	struct video_osd_info info;
	u16 val;

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO_OSD, &dev));

	ut_assertok(video_osd_set_size(dev, 20, 5));

	video_osd_get_info(dev, &info);

	ut_asserteq(20, info.width);
	ut_asserteq(5, info.height);
	ut_asserteq(1, info.major_version);
	ut_asserteq(0, info.minor_version);

	ut_assertok(sandbox_osd_get_mem(dev, mem, memsize));
	split(mem, memsize / 2, text, colors);

	ut_assertok(memcmp(text, "                    "
				 "                    "
				 "                    "
				 "                    "
				 "                    ", memsize / 2));

	ut_assertok(memcmp(colors, "kkkkkkkkkkkkkkkkkkkk"
				   "kkkkkkkkkkkkkkkkkkkk"
				   "kkkkkkkkkkkkkkkkkkkk"
				   "kkkkkkkkkkkkkkkkkkkk"
				   "kkkkkkkkkkkkkkkkkkkk", memsize / 2));

	print_mem(mem, 20, 5);

	/* Draw green border */
	val = '-' * 0x100 + 'g';
	ut_assertok(video_osd_set_mem(dev, 1, 0, (u8 *)&val, 2, 18));
	ut_assertok(video_osd_set_mem(dev, 1, 4, (u8 *)&val, 2, 18));
	ut_assertok(video_osd_print(dev, 0, 1, COLOR_GREEN, "|"));
	ut_assertok(video_osd_print(dev, 0, 2, COLOR_GREEN, "|"));
	ut_assertok(video_osd_print(dev, 0, 3, COLOR_GREEN, "|"));
	ut_assertok(video_osd_print(dev, 19, 1, COLOR_GREEN, "|"));
	ut_assertok(video_osd_print(dev, 19, 2, COLOR_GREEN, "|"));
	ut_assertok(video_osd_print(dev, 19, 3, COLOR_GREEN, "|"));
	ut_assertok(video_osd_print(dev, 0, 0, COLOR_GREEN, "+"));
	ut_assertok(video_osd_print(dev, 19, 0, COLOR_GREEN, "+"));
	ut_assertok(video_osd_print(dev, 19, 4, COLOR_GREEN, "+"));
	ut_assertok(video_osd_print(dev, 0, 4, COLOR_GREEN, "+"));

	/* Add menu caption and entries */
	ut_assertok(video_osd_print(dev, 5, 0, COLOR_GREEN, " OSD menu "));
	ut_assertok(video_osd_print(dev, 2, 1, COLOR_BLUE, " *  Entry 1"));
	ut_assertok(video_osd_print(dev, 2, 2, COLOR_BLUE, "(*) Entry 2"));
	ut_assertok(video_osd_print(dev, 2, 3, COLOR_BLUE, " *  Entry 3"));

	ut_assertok(sandbox_osd_get_mem(dev, mem, memsize));
	split(mem, memsize / 2, text, colors);

	print_mem(mem, 20, 5);

	ut_assertok(memcmp(text, "+---- OSD menu ----+"
				 "|  *  Entry 1      |"
				 "| (*) Entry 2      |"
				 "|  *  Entry 3      |"
				 "+------------------+", memsize / 2));

	ut_assertok(memcmp(colors, "gggggggggggggggggggg"
				   "gkbbbbbbbbbbbkkkkkkg"
				   "gkbbbbbbbbbbbkkkkkkg"
				   "gkbbbbbbbbbbbkkkkkkg"
				   "gggggggggggggggggggg", memsize / 2));

	return 0;
}

DM_TEST(dm_test_osd_extended, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
