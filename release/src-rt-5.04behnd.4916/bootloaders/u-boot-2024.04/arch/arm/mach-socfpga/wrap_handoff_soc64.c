// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020-2021 Intel Corporation <www.intel.com>
 *
 */

#include <asm/arch/handoff_soc64.h>
#include <asm/io.h>
#include <common.h>
#include <errno.h>
#include "log.h"

#ifndef __ASSEMBLY__
#include <asm/types.h>
enum endianness {
	LITTLE_ENDIAN = 0,
	BIG_ENDIAN,
	UNKNOWN_ENDIANNESS
};
#endif

static enum endianness check_endianness(u32 handoff)
{
	switch (handoff) {
	case SOC64_HANDOFF_MAGIC_BOOT:
	case SOC64_HANDOFF_MAGIC_MUX:
	case SOC64_HANDOFF_MAGIC_IOCTL:
	case SOC64_HANDOFF_MAGIC_FPGA:
	case SOC64_HANDOFF_MAGIC_DELAY:
	case SOC64_HANDOFF_MAGIC_CLOCK:
	case SOC64_HANDOFF_MAGIC_MISC:
		return BIG_ENDIAN;
#if IS_ENABLED(CONFIG_TARGET_SOCFPGA_N5X)
	case SOC64_HANDOFF_DDR_UMCTL2_MAGIC:
		debug("%s: umctl2 handoff data\n", __func__);
		return LITTLE_ENDIAN;
	case SOC64_HANDOFF_DDR_PHY_MAGIC:
		debug("%s: PHY handoff data\n", __func__);
		return LITTLE_ENDIAN;
	case SOC64_HANDOFF_DDR_PHY_INIT_ENGINE_MAGIC:
		debug("%s: PHY engine handoff data\n", __func__);
		return LITTLE_ENDIAN;
#endif
	default:
		debug("%s: Unknown endianness!!\n", __func__);
		return UNKNOWN_ENDIANNESS;
	}
}

static int getting_endianness(void *handoff_address, enum endianness *endian_t)
{
	/* Checking handoff data is little endian ? */
	*endian_t = check_endianness(readl(handoff_address));

	if (*endian_t == UNKNOWN_ENDIANNESS) {
		/* Trying to check handoff data is big endian? */
		*endian_t = check_endianness(swab32(readl(handoff_address)));
		if (*endian_t == UNKNOWN_ENDIANNESS) {
			debug("%s: Cannot find HANDOFF MAGIC ", __func__);
			debug("at addr 0x%p\n", (u32 *)handoff_address);
			return -EPERM;
		}
	}

	return 0;
}

int socfpga_get_handoff_size(void *handoff_address)
{
	u32 size;
	int ret;
	enum endianness endian_t;

	ret = getting_endianness(handoff_address, &endian_t);
	if (ret)
		return ret;

	size = readl(handoff_address + SOC64_HANDOFF_OFFSET_LENGTH);
	if (endian_t == BIG_ENDIAN)
		size = swab32(size);

	size = (size - SOC64_HANDOFF_OFFSET_DATA) / sizeof(u32);

	debug("%s: handoff address = 0x%p handoff size = 0x%08x\n", __func__,
	      (u32 *)handoff_address, size);

	return size;
}

int socfpga_handoff_read(void *handoff_address, void *table, u32 table_len)
{
	u32 temp;
	u32 *table_x32 = table;
	u32 i = 0;
	int ret;
	enum endianness endian_t;

	ret = getting_endianness(handoff_address, &endian_t);
	if (ret)
		return ret;

	temp = readl(handoff_address + SOC64_HANDOFF_OFFSET_DATA +
		    (i * sizeof(u32)));

	if (endian_t == BIG_ENDIAN) {
		debug("%s: Handoff addr = 0x%p ", __func__, (u32 *)handoff_address);
		debug("Handoff table address = 0x%p ", table_x32);
		debug("table length = 0x%x\n", table_len);
		debug("%s: handoff data =\n{\n", __func__);
		*table_x32 = swab32(temp);
	} else if (endian_t == LITTLE_ENDIAN) {
		debug(" {\n");
		*table_x32 = temp;
	}

	debug(" No.%d Addr 0x%08x: ", i, *table_x32);

	for (i = 1; i < table_len; i++) {
		table_x32++;

		temp = readl(handoff_address +
			     SOC64_HANDOFF_OFFSET_DATA +
			     (i * sizeof(u32)));

		if (endian_t == BIG_ENDIAN)
			*table_x32 = swab32(temp);
		else if (endian_t == LITTLE_ENDIAN)
			*table_x32 = temp;

		if (!(i % 2))
			debug(" No.%d Addr 0x%08x: ", i,
			      *table_x32);
		else
			debug(" 0x%08x\n", *table_x32);
	}
	debug("\n}\n");

	return 0;
}
