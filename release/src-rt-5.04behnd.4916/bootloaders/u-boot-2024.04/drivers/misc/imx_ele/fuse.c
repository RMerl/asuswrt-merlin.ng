// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <console.h>
#include <errno.h>
#include <fuse.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/imx-regs.h>
#include <env.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define FUSE_BANKS	64
#define WORDS_PER_BANKS 8

struct fsb_map_entry {
	s32 fuse_bank;
	u32 fuse_words;
	bool redundancy;
};

struct ele_map_entry {
	s32 fuse_bank;
	u32 fuse_words;
	u32 fuse_offset;
	u32 ele_index;
};

#if defined(CONFIG_IMX8ULP)
#define FSB_OTP_SHADOW	0x800

struct fsb_map_entry fsb_mapping_table[] = {
	{ 3, 8 },
	{ 4, 8 },
	{ -1, 48 }, /* Reserve 48 words */
	{ 5, 8 },
	{ 6, 8 },
	{ 8,  4, true },
	{ 24, 4, true },
	{ 26, 4, true },
	{ 27, 4, true },
	{ 28, 8 },
	{ 29, 8 },
	{ 30, 8 },
	{ 31, 8 },
	{ 37, 8 },
	{ 38, 8 },
	{ 39, 8 },
	{ 40, 8 },
	{ 41, 8 },
	{ 42, 8 },
	{ 43, 8 },
	{ 44, 8 },
	{ 45, 8 },
	{ 46, 8 },
};

/* None ECC banks such like Redundancy or Bit protect */
u32 nonecc_fuse_banks[] = {
	0, 1, 8, 12, 16, 22, 24, 25, 26, 27, 36, 41, 51, 56
};

struct ele_map_entry ele_api_mapping_table[] = {
	{ 1, 8 },	/* LOCK */
	{ 2, 8 },	/* ECID */
	{ 7, 4, 0, 1 },	/* OTP_UNIQ_ID */
	{ 15, 8 }, /* OEM SRK HASH */
	{ 23, 1, 4, 2 }, /* OTFAD */
	{ 25, 8 }, /* Test config2 */
	{ 26, 8 }, /* PMU */
	{ 27, 8 }, /* Test flow/USB */
	{ 32, 8 }, /* GP1 */
	{ 33, 8 }, /* GP2 */
	{ 34, 8 }, /* GP3 */
	{ 35, 8 }, /* GP4 */
	{ 36, 8 }, /* GP5 */
	{ 49, 8 }, /* GP8 */
	{ 50, 8 }, /* GP9 */
	{ 51, 8 }, /* GP10 */
};
#elif defined(CONFIG_ARCH_IMX9)
#define FSB_OTP_SHADOW	0x8000

struct fsb_map_entry fsb_mapping_table[] = {
	{ 0, 8 },
	{ 1, 8 },
	{ 2, 8 },
	{ 3, 8 },
	{ 4, 8 },
	{ 5, 8 },
	{ 6, 4 },
	{ -1, 260 },
	{ 39, 8 },
	{ 40, 8 },
	{ 41, 8 },
	{ 42, 8 },
	{ 43, 8 },
	{ 44, 8 },
	{ 45, 8 },
	{ 46, 8 },
	{ 47, 8 },
	{ 48, 8 },
	{ 49, 8 },
	{ 50, 8 },
	{ 51, 8 },
	{ 52, 8 },
	{ 53, 8 },
	{ 54, 8 },
	{ 55, 8 },
	{ 56, 8 },
	{ 57, 8 },
	{ 58, 8 },
	{ 59, 8 },
	{ 60, 8 },
	{ 61, 8 },
	{ 62, 8 },
	{ 63, 8 },
};

struct ele_map_entry ele_api_mapping_table[] = {
	{ 7, 1, 7, 63 },
	{ 16, 8, },
	{ 17, 8, },
	{ 22, 1, 6 },
	{ 23, 1, 4 },
};
#endif

static s32 map_fsb_fuse_index(u32 bank, u32 word, bool *redundancy)
{
	s32 size = ARRAY_SIZE(fsb_mapping_table);
	s32 i, word_pos = 0;

	/* map the fuse from ocotp fuse map to FSB*/
	for (i = 0; i < size; i++) {
		if (fsb_mapping_table[i].fuse_bank != -1 &&
		    fsb_mapping_table[i].fuse_bank == bank &&
		    fsb_mapping_table[i].fuse_words > word) {
			break;
		}

		word_pos += fsb_mapping_table[i].fuse_words;
	}

	if (i == size)
		return -1; /* Failed to find */

	if (fsb_mapping_table[i].redundancy) {
		*redundancy = true;
		return (word >> 1) + word_pos;
	}

	*redundancy = false;
	return word + word_pos;
}

static s32 map_ele_fuse_index(u32 bank, u32 word)
{
	s32 size = ARRAY_SIZE(ele_api_mapping_table);
	s32 i;

	/* map the fuse from ocotp fuse map to FSB*/
	for (i = 0; i < size; i++) {
		if (ele_api_mapping_table[i].fuse_bank != -1 &&
		    ele_api_mapping_table[i].fuse_bank == bank) {
			if (word >= ele_api_mapping_table[i].fuse_offset &&
			    word < (ele_api_mapping_table[i].fuse_offset +
			    ele_api_mapping_table[i].fuse_words))
				break;
		}
	}

	if (i == size)
		return -1; /* Failed to find */

	if (ele_api_mapping_table[i].ele_index != 0)
		return ele_api_mapping_table[i].ele_index;

	return ele_api_mapping_table[i].fuse_bank * 8 + word;
}

#if defined(CONFIG_IMX8ULP)
int fuse_sense(u32 bank, u32 word, u32 *val)
{
	s32 word_index;
	bool redundancy;

	if (bank >= FUSE_BANKS || word >= WORDS_PER_BANKS || !val)
		return -EINVAL;

	word_index = map_fsb_fuse_index(bank, word, &redundancy);
	if (word_index >= 0) {
		*val = readl((ulong)FSB_BASE_ADDR + FSB_OTP_SHADOW + (word_index << 2));
		if (redundancy)
			*val = (*val >> ((word % 2) * 16)) & 0xFFFF;

		return 0;
	}

	word_index = map_ele_fuse_index(bank, word);
	if (word_index >= 0) {
		u32 data[4];
		u32 res, size = 4;
		int ret;

		/* Only UID return 4 words */
		if (word_index != 1)
			size = 1;

		ret = ele_read_common_fuse(word_index, data, size, &res);
		if (ret) {
			printf("ahab read fuse failed %d, 0x%x\n", ret, res);
			return ret;
		}

		if (word_index == 1) {
			*val = data[word]; /* UID */
		} else if (word_index == 2) {
			/*
			 * OTFAD 3 bits as follow:
			 * bit 0: OTFAD_ENABLE
			 * bit 1: OTFAD_DISABLE_OVERRIDE
			 * bit 2: KEY_BLOB_EN
			 */
			*val = data[0] << 3;
		} else {
			*val = data[0];
		}

		return 0;
	}

	return -ENOENT;
}
#elif defined(CONFIG_ARCH_IMX9)
int fuse_sense(u32 bank, u32 word, u32 *val)
{
	s32 word_index;
	bool redundancy;

	if (bank >= FUSE_BANKS || word >= WORDS_PER_BANKS || !val)
		return -EINVAL;

	word_index = map_fsb_fuse_index(bank, word, &redundancy);
	if (word_index >= 0) {
		*val = readl((ulong)FSB_BASE_ADDR + FSB_OTP_SHADOW + (word_index << 2));
		if (redundancy)
			*val = (*val >> ((word % 2) * 16)) & 0xFFFF;

		return 0;
	}

	word_index = map_ele_fuse_index(bank, word);
	if (word_index >= 0) {
		u32 data;
		u32 res, size = 1;
		int ret;

		ret = ele_read_common_fuse(word_index, &data, size, &res);
		if (ret) {
			printf("ahab read fuse failed %d, 0x%x\n", ret, res);
			return ret;
		}

		*val = data;

		return 0;
	}

	return -ENOENT;
}
#endif

int fuse_read(u32 bank, u32 word, u32 *val)
{
	return fuse_sense(bank, word, val);
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	u32 res;
	int ret;
	bool lock = false;

	if (bank >= FUSE_BANKS || word >= WORDS_PER_BANKS || !val)
		return -EINVAL;

	/* Lock 8ULP ECC fuse word, so second programming will return failure.
	 * iMX9 OTP can protect ECC fuse, so not need it
	 */
#if defined(CONFIG_IMX8ULP)
	u32 i;
	for (i = 0; i < ARRAY_SIZE(nonecc_fuse_banks); i++) {
		if (nonecc_fuse_banks[i] == bank)
			break;
	}

	if (i == ARRAY_SIZE(nonecc_fuse_banks))
		lock = true;
#endif

	ret = ele_write_fuse((bank * 8 + word), val, lock, &res);
	if (ret) {
		printf("ahab write fuse failed %d, 0x%x\n", ret, res);
		return ret;
	}

	return 0;
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	printf("Override fuse to i.MX8ULP in u-boot is forbidden\n");
	return -EPERM;
}
