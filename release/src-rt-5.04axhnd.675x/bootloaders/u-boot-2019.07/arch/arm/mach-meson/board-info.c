// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Julien Masson <jmasson@baylibre.com>
 * (C) Copyright 2019 Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <regmap.h>
#include <syscon.h>

#define AO_SEC_SD_CFG8		0xe0
#define AO_SEC_SOCINFO_OFFSET	AO_SEC_SD_CFG8

#define SOCINFO_MAJOR	GENMASK(31, 24)
#define SOCINFO_PACK	GENMASK(23, 16)
#define SOCINFO_MINOR	GENMASK(15, 8)
#define SOCINFO_MISC	GENMASK(7, 0)

static const struct meson_gx_soc_id {
	const char *name;
	unsigned int id;
} soc_ids[] = {
	{ "GXBB",   0x1f },
	{ "GXTVBB", 0x20 },
	{ "GXL",    0x21 },
	{ "GXM",    0x22 },
	{ "TXL",    0x23 },
	{ "TXLX",   0x24 },
	{ "AXG",    0x25 },
	{ "GXLX",   0x26 },
	{ "TXHD",   0x27 },
	{ "G12A",   0x28 },
	{ "G12B",   0x29 },
};

static const struct meson_gx_package_id {
	const char *name;
	unsigned int major_id;
	unsigned int pack_id;
	unsigned int pack_mask;
} soc_packages[] = {
	{ "S905",   0x1f, 0,    0x20 }, /* pack_id != 0x20 */
	{ "S905H",  0x1f, 0x3,  0xf },  /* pack_id & 0xf == 0x3 */
	{ "S905M",  0x1f, 0x20, 0xf0 }, /* pack_id == 0x20 */
	{ "S905D",  0x21, 0,    0xf0 },
	{ "S905X",  0x21, 0x80, 0xf0 },
	{ "S905W",  0x21, 0xa0, 0xf0 },
	{ "S905L",  0x21, 0xc0, 0xf0 },
	{ "S905M2", 0x21, 0xe0, 0xf0 },
	{ "S805X",  0x21, 0x30, 0xf0 },
	{ "S805Y",  0x21, 0xb0, 0xf0 },
	{ "S912",   0x22, 0,    0x0 },  /* Only S912 is known for GXM */
	{ "962X",   0x24, 0x10, 0xf0 },
	{ "962E",   0x24, 0x20, 0xf0 },
	{ "A113X",  0x25, 0x37, 0xff },
	{ "A113D",  0x25, 0x22, 0xff },
	{ "S905D2", 0x28, 0x10, 0xf0 },
	{ "S905X2", 0x28, 0x40, 0xf0 },
	{ "S922X",  0x29, 0x40, 0xf0 },
};

DECLARE_GLOBAL_DATA_PTR;

static inline unsigned int socinfo_to_major(u32 socinfo)
{
	return FIELD_GET(SOCINFO_MAJOR, socinfo);
}

static inline unsigned int socinfo_to_minor(u32 socinfo)
{
	return FIELD_GET(SOCINFO_MINOR, socinfo);
}

static inline unsigned int socinfo_to_pack(u32 socinfo)
{
	return FIELD_GET(SOCINFO_PACK, socinfo);
}

static inline unsigned int socinfo_to_misc(u32 socinfo)
{
	return FIELD_GET(SOCINFO_MISC, socinfo);
}

static const char *socinfo_to_package_id(u32 socinfo)
{
	unsigned int pack = socinfo_to_pack(socinfo);
	unsigned int major = socinfo_to_major(socinfo);
	int i;

	for (i = 0 ; i < ARRAY_SIZE(soc_packages) ; ++i) {
		if (soc_packages[i].major_id == major &&
		    soc_packages[i].pack_id ==
		    (pack & soc_packages[i].pack_mask))
			return soc_packages[i].name;
	}

	return "Unknown";
}

static const char *socinfo_to_soc_id(u32 socinfo)
{
	unsigned int id = socinfo_to_major(socinfo);
	int i;

	for (i = 0 ; i < ARRAY_SIZE(soc_ids) ; ++i) {
		if (soc_ids[i].id == id)
			return soc_ids[i].name;
	}

	return "Unknown";
}

static void print_board_model(void)
{
	const char *model;
	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	printf("Model: %s\n", model ? model : "Unknown");
}

int show_board_info(void)
{
	struct regmap *regmap;
	int nodeoffset, ret;
	ofnode node;
	unsigned int socinfo;

	/* find the offset of compatible node */
	nodeoffset = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
						   "amlogic,meson-gx-ao-secure");
	if (nodeoffset < 0)
		return 0;

	/* check if chip-id is available */
	if (!fdt_getprop(gd->fdt_blob, nodeoffset, "amlogic,has-chip-id", NULL))
		return 0;

	/* get regmap from the syscon node */
	node = offset_to_ofnode(nodeoffset);
	regmap = syscon_node_to_regmap(node);
	if (IS_ERR(regmap)) {
		printf("%s: failed to get regmap\n", __func__);
		return 0;
	}

	/* read soc info */
	ret = regmap_read(regmap, AO_SEC_SOCINFO_OFFSET, &socinfo);
	if (ret && !socinfo) {
		printf("%s: invalid chipid value\n", __func__);
		return 0;
	}

	/* print board information */
	print_board_model();
	printf("Soc:   Amlogic Meson %s (%s) Revision %x:%x (%x:%x)\n",
	       socinfo_to_soc_id(socinfo),
	       socinfo_to_package_id(socinfo),
	       socinfo_to_major(socinfo),
	       socinfo_to_minor(socinfo),
	       socinfo_to_pack(socinfo),
	       socinfo_to_misc(socinfo));

	return 0;
}
