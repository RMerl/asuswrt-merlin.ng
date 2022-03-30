/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Reinhard Pfau <reinhard.pfau@gdsys.cc>
 */

#ifndef _MVEBU_EFUSE_H
#define _MVEBU_EFUSE_H

#include <common.h>

struct efuse_val {
	union {
		struct {
			u8 d[8];
		} bytes;
		struct {
			u16 d[4];
		} words;
		struct {
			u32 d[2];
		} dwords;
	};
	u32 lock;
};

#if defined(CONFIG_ARMADA_38X)

enum efuse_line {
	EFUSE_LINE_SECURE_BOOT = 24,
	EFUSE_LINE_PUBKEY_DIGEST_0 = 26,
	EFUSE_LINE_PUBKEY_DIGEST_1 = 27,
	EFUSE_LINE_PUBKEY_DIGEST_2 = 28,
	EFUSE_LINE_PUBKEY_DIGEST_3 = 29,
	EFUSE_LINE_PUBKEY_DIGEST_4 = 30,
	EFUSE_LINE_CSK_0_VALID = 31,
	EFUSE_LINE_CSK_1_VALID = 32,
	EFUSE_LINE_CSK_2_VALID = 33,
	EFUSE_LINE_CSK_3_VALID = 34,
	EFUSE_LINE_CSK_4_VALID = 35,
	EFUSE_LINE_CSK_5_VALID = 36,
	EFUSE_LINE_CSK_6_VALID = 37,
	EFUSE_LINE_CSK_7_VALID = 38,
	EFUSE_LINE_CSK_8_VALID = 39,
	EFUSE_LINE_CSK_9_VALID = 40,
	EFUSE_LINE_CSK_10_VALID = 41,
	EFUSE_LINE_CSK_11_VALID = 42,
	EFUSE_LINE_CSK_12_VALID = 43,
	EFUSE_LINE_CSK_13_VALID = 44,
	EFUSE_LINE_CSK_14_VALID = 45,
	EFUSE_LINE_CSK_15_VALID = 46,
	EFUSE_LINE_FLASH_ID = 47,
	EFUSE_LINE_BOX_ID = 48,

	EFUSE_LINE_MIN = 0,
	EFUSE_LINE_MAX = 63,
};

#endif

int mvebu_efuse_init_hw(void);

int mvebu_read_efuse(int nr, struct efuse_val *val);

int mvebu_write_efuse(int nr, struct efuse_val *val);

int mvebu_lock_efuse(int nr);

#endif
