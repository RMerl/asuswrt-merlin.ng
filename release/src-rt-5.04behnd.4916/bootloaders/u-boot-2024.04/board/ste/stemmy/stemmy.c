// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */
#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <log.h>
#include <stdlib.h>
#include <asm/global_data.h>
#include <asm/setup.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

/* Parse atags provided by Samsung bootloader to get available memory */
static ulong fw_mach __section(".data");
static ulong fw_atags __section(".data");

static const struct tag *fw_atags_copy;
static uint fw_atags_size;

void save_boot_params(ulong r0, ulong r1, ulong r2, ulong r3)
{
	fw_mach = r1;
	fw_atags = r2;
	save_boot_params_ret();
}

static const struct tag *fw_atags_get(void)
{
	const struct tag *tags = (const struct tag *)fw_atags;

	if (tags->hdr.tag != ATAG_CORE) {
		log_err("Invalid atags: tag 0x%x at %p\n", tags->hdr.tag, tags);
		return NULL;
	}

	return tags;
}

int dram_init(void)
{
	const struct tag *t, *tags = fw_atags_get();

	if (!tags)
		return -EINVAL;

	for_each_tag(t, tags) {
		if (t->hdr.tag != ATAG_MEM)
			continue;

		debug("Memory: %#x-%#x (size %#x)\n", t->u.mem.start,
		      t->u.mem.start + t->u.mem.size, t->u.mem.size);
		gd->ram_size += t->u.mem.size;
	}
	return 0;
}

int dram_init_banksize(void)
{
	const struct tag *t, *tags = fw_atags_get();
	unsigned int bank = 0;

	if (!tags)
		return -EINVAL;

	for_each_tag(t, tags) {
		if (t->hdr.tag != ATAG_MEM)
			continue;

		gd->bd->bi_dram[bank].start = t->u.mem.start;
		gd->bd->bi_dram[bank].size = t->u.mem.size;
		if (++bank == CONFIG_NR_DRAM_BANKS)
			break;
	}
	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = fw_mach;
	gd->bd->bi_boot_params = fw_atags;
	return 0;
}

static void parse_serial(const struct tag_serialnr *serialnr)
{
	char serial[17];

	if (env_get("serial#"))
		return;

	sprintf(serial, "%08x%08x", serialnr->high, serialnr->low);
	env_set("serial#", serial);
}

#define SBL_BOARD "board_id="
#define SBL_LCDTYPE "lcdtype="
static ulong board_id = 0;
static ulong lcdtype = 0;

static void parse_cmdline(const struct tag_cmdline *cmdline)
{
	char *buf;

	/* Export this to sbl_cmdline (secondary boot loader command line) */
	env_set("sbl_cmdline", cmdline->cmdline);

	buf = strstr(cmdline->cmdline, SBL_BOARD);
	if (!buf)
		return;
	buf += strlen(SBL_BOARD);

	board_id = simple_strtoul(buf, NULL, 10);

	buf = strstr(cmdline->cmdline, SBL_LCDTYPE);
	if (!buf)
		return;
	buf += strlen(SBL_LCDTYPE);

	lcdtype = simple_strtoul(buf, NULL, 10);
}

/*
 * The downstream/vendor kernel (provided by Samsung) uses ATAGS for booting.
 * It also requires an extremely long cmdline provided by the primary bootloader
 * that is not suitable for booting mainline.
 *
 * Since downstream is the only user of ATAGS, we emulate the behavior of the
 * Samsung bootloader by generating only the initrd atag in U-Boot, and copying
 * all other ATAGS as-is from the primary bootloader.
 */
static inline bool skip_atag(u32 tag)
{
	return (tag == ATAG_NONE || tag == ATAG_CORE ||
		tag == ATAG_INITRD || tag == ATAG_INITRD2);
}

static void copy_atags(const struct tag *tags)
{
	const struct tag *t;
	struct tag *copy;

	if (!tags)
		return;

	/* Calculate necessary size for tags we want to copy */
	for_each_tag(t, tags) {
		if (skip_atag(t->hdr.tag))
			continue;

		if (t->hdr.tag == ATAG_SERIAL)
			parse_serial(&t->u.serialnr);

		if (t->hdr.tag == ATAG_CMDLINE)
			parse_cmdline(&t->u.cmdline);

		fw_atags_size += t->hdr.size * sizeof(u32);
	}

	if (!fw_atags_size)
		return;  /* No tags to copy */

	copy = malloc(fw_atags_size);
	if (!copy)
		return;
	fw_atags_copy = copy;

	/* Copy tags */
	for_each_tag(t, tags) {
		if (skip_atag(t->hdr.tag))
			continue;

		memcpy(copy, t, t->hdr.size * sizeof(u32));
		copy = tag_next(copy);
	}
}

int misc_init_r(void)
{
	copy_atags(fw_atags_get());
	return 0;
}

void setup_board_tags(struct tag **in_params)
{
	if (!fw_atags_copy)
		return;

	/*
	 * fw_atags_copy contains only full "struct tag" (plus data)
	 * so copying it bytewise here should be fine.
	 */
	memcpy(*in_params, fw_atags_copy, fw_atags_size);
	*(u8 **)in_params += fw_atags_size;
}

/* These numbers are unique per product but not across all products */
#define SAMSUNG_CODINA_LCD_LMS380KF01 4
#define SAMSUNG_CODINA_LCD_S6D27A1 13
#define SAMSUNG_SKOMER_LCD_HVA40WV1 10
#define SAMSUNG_SKOMER_LCD_NT35512 12

static void codina_patch_display(void *fdt)
{
	int node;
	int ret;

	node = fdt_path_offset(fdt, "/spi-gpio-0/panel");
	if (node < 0) {
		printf("cannot find Codina panel node\n");
		return;
	}
	if (lcdtype == SAMSUNG_CODINA_LCD_LMS380KF01) {
		ret = fdt_setprop_string(fdt, node, "compatible", "samsung,lms380kf01");
		if (ret < 0)
			printf("could not set LCD compatible\n");
		else
			printf("updated LCD compatible to LMS380KF01\n");
	} else if (lcdtype == SAMSUNG_CODINA_LCD_S6D27A1) {
		ret = fdt_setprop_string(fdt, node, "compatible", "samsung,s6d27a1");
		if (ret < 0)
			printf("could not set LCD compatible\n");
		else
			printf("updated LCD compatible to S6D27A1\n");
	} else {
		printf("unknown LCD type\n");
	}
}

static void skomer_kyle_patch_display(void *fdt)
{
	int node;
	int ret;

	node = fdt_path_offset(fdt, "/soc/mcde/dsi/panel");
	if (node < 0) {
		printf("cannot find Skomer/Kyle panel node\n");
		return;
	}
	if (lcdtype == SAMSUNG_SKOMER_LCD_HVA40WV1) {
		ret = fdt_setprop_string(fdt, node, "compatible", "hydis,hva40wv1");
		if (ret < 0)
			printf("could not set LCD compatible\n");
		else
			printf("updated LCD compatible to Hydis HVA40WV1\n");
	} else if (lcdtype == SAMSUNG_SKOMER_LCD_NT35512) {
		/*
		 * FIXME: This panel is actually a BOE product, but we don't know
		 * the exact product name, so the compatible for the NT35512
		 * is used for the time being. The vendor drivers also call it NT35512.
		 */
		ret = fdt_setprop_string(fdt, node, "compatible", "novatek,nt35512");
		if (ret < 0)
			printf("could not set LCD compatible\n");
		else
			printf("updated LCD compatible to Novatek NT35512\n");
	} else {
		printf("unknown LCD type\n");
	}
}

int ft_board_setup(void *fdt, struct bd_info *bd)
{
	const char *str;
	int node;
	int ret;

	printf("stemmy patch: DTB at 0x%08lx\n", (ulong)fdt);

	/* Inspect FDT to see what we've got here */
	ret = fdt_check_header(fdt);
	if (ret < 0) {
		printf("invalid DTB\n");
		return ret;
	}
	node = fdt_path_offset(fdt, "/");
	if (node < 0) {
		printf("cannot find root node\n");
		return node;
	}
	str = fdt_stringlist_get(fdt, node, "compatible", 0, NULL);
	if (!str) {
		printf("could not find board compatible\n");
		return -1;
	}

	if (!strcmp(str, "samsung,janice")) {
		switch(board_id) {
		case 7:
			printf("Janice GT-I9070 Board Rev 0.0\n");
			break;
		case 8:
			printf("Janice GT-I9070 Board Rev 0.1\n");
			break;
		case 9:
			printf("Janice GT-I9070 Board Rev 0.2\n");
			break;
		case 10:
			printf("Janice GT-I9070 Board Rev 0.3\n");
			break;
		case 11:
			printf("Janice GT-I9070 Board Rev 0.4\n");
			break;
		case 12:
			printf("Janice GT-I9070 Board Rev 0.5\n");
			break;
		case 13:
			printf("Janice GT-I9070 Board Rev 0.6\n");
			break;
		default:
			break;
		}
	} else if (!strcmp(str, "samsung,gavini")) {
		switch(board_id) {
		case 7:
			printf("Gavini GT-I8530 Board Rev 0.0\n");
			break;
		case 8:
			printf("Gavini GT-I8530 Board Rev 0.0A\n");
			break;
		case 9:
			printf("Gavini GT-I8530 Board Rev 0.0B\n");
			break;
		case 10:
			printf("Gavini GT-I8530 Board Rev 0.0A_EMUL\n");
			break;
		case 11:
			printf("Gavini GT-I8530 Board Rev 0.0C\n");
			break;
		case 12:
			printf("Gavini GT-I8530 Board Rev 0.0D\n");
			break;
		case 13:
			printf("Gavini GT-I8530 Board Rev 0.1\n");
			break;
		case 14:
			printf("Gavini GT-I8530 Board Rev 0.3\n");
			break;
		default:
			break;
		}
	} else if (!strcmp(str, "samsung,codina")) {
		switch(board_id) {
		case 7:
			printf("Codina GT-I8160 Board Rev 0.0\n");
			break;
		case 8:
			printf("Codina GT-I8160 Board Rev 0.1\n");
			break;
		case 9:
			printf("Codina GT-I8160 Board Rev 0.2\n");
			break;
		case 10:
			printf("Codina GT-I8160 Board Rev 0.3\n");
			break;
		case 11:
			printf("Codina GT-I8160 Board Rev 0.4\n");
			break;
		case 12:
			printf("Codina GT-I8160 Board Rev 0.5\n");
			break;
		default:
			break;
		}
		codina_patch_display(fdt);
	} else if (!strcmp(str, "samsung,codina-tmo")) {
		switch(board_id) {
		case 0x101:
			printf("Codina SGH-T599 Board pre-Rev 0.0\n");
			break;
		case 0x102:
			printf("Codina SGH-T599 Board Rev 0.0\n");
			break;
		case 0x103:
			printf("Codina SGH-T599 Board Rev 0.1\n");
			break;
		case 0x104:
			printf("Codina SGH-T599 Board Rev 0.2\n");
			break;
		case 0x105:
			printf("Codina SGH-T599 Board Rev 0.4\n");
			break;
		case 0x106:
			printf("Codina SGH-T599 Board Rev 0.6\n");
			break;
		case 0x107:
			printf("Codina SGH-T599 Board Rev 0.7\n");
			break;
		default:
			break;
		}
		codina_patch_display(fdt);
	} else if (!strcmp(str, "samsung,golden")) {
		switch(board_id) {
		case 0x102:
			printf("Golden GT-I8190 Board SW bringup\n");
			break;
		case 0x103:
			printf("Golden GT-I8190 Board Rev 0.2\n");
			break;
		case 0x104:
			printf("Golden GT-I8190 Board Rev 0.3\n");
			break;
		case 0x105:
			printf("Golden GT-I8190 Board Rev 0.4\n");
			break;
		case 0x106:
			printf("Golden GT-I8190 Board Rev 0.5\n");
			break;
		case 0x107:
			printf("Golden GT-I8190 Board Rev 0.6\n");
			break;
		default:
			break;
		}
	} else if (!strcmp(str, "samsung,skomer")) {
		switch(board_id) {
		case 0x101:
			printf("Skomer GT-S7710 Board Rev 0.0\n");
			break;
		case 0x102:
			printf("Skomer GT-S7710 Board Rev 0.1\n");
			break;
		case 0x103:
			printf("Skomer GT-S7710 Board Rev 0.2\n");
			break;
		case 0x104:
			printf("Skomer GT-S7710 Board Rev 0.3\n");
			break;
		case 0x105:
			printf("Skomer GT-S7710 Board Rev 0.4\n");
			break;
		case 0x106:
			printf("Skomer GT-S7710 Board Rev 0.5\n");
			break;
		case 0x107:
			printf("Skomer GT-S7710 Board Rev 0.6\n");
			break;
		case 0x108:
			printf("Skomer GT-S7710 Board Rev 0.7\n");
			break;
		case 0x109:
			printf("Skomer GT-S7710 Board Rev 0.8\n");
			break;
		default:
			break;
		}
		skomer_kyle_patch_display(fdt);
	} else if (!strcmp(str, "samsung,kyle")) {
		switch(board_id) {
		case 0x101:
			printf("Kyle SGH-I407 Board Rev 0.0\n");
			break;
		case 0x102:
			printf("Kyle SGH-I407 Board Rev 0.1\n");
			break;
		case 0x103:
			printf("Kyle SGH-I407 Board Rev 0.2\n");
			break;
		case 0x104:
			printf("Kyle SGH-I407 Board Rev 0.3\n");
			break;
		case 0x105:
			printf("Kyle SGH-I407 Board Rev 0.4\n");
			break;
		case 0x106:
			printf("Kyle SGH-I407 Board Rev 0.5\n");
			break;
		case 0x107:
			printf("Kyle SGH-I407 Board Rev 0.6\n");
			break;
		default:
			break;
		}
		skomer_kyle_patch_display(fdt);
	}

	return 0;
}
