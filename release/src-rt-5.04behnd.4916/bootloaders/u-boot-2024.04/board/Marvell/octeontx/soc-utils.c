// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <dm.h>
#include <dm/util.h>
#include <errno.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <asm/arch/board.h>

int read_platform(void)
{
	int plat = PLATFORM_HW;

	const char *model = fdt_get_board_model();

	if (model && !strncmp(model, "ASIM-", 5))
		plat = PLATFORM_ASIM;
	if (model && !strncmp(model, "EMUL-", 5))
		plat = PLATFORM_EMULATOR;
	return plat;
}

static inline u64 read_midr(void)
{
	u64 result;

	asm ("mrs %[rd],MIDR_EL1" : [rd] "=r" (result));
	return result;
}

u8 read_partnum(void)
{
	return ((read_midr() >> 4) & 0xFF);
}

const char *read_board_name(void)
{
	return fdt_get_board_model();
}

bool read_alt_pkg(void)
{
	return false;
}
