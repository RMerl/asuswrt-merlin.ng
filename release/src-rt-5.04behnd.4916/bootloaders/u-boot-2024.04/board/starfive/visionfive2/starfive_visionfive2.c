// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdt_support.h>
#include <env.h>
#include <asm/arch/eeprom.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;
#define JH7110_L2_PREFETCHER_BASE_ADDR		0x2030000
#define JH7110_L2_PREFETCHER_HART_OFFSET	0x2000
#define FDTFILE_VISIONFIVE2_1_2A \
	"starfive/jh7110-starfive-visionfive-2-v1.2a.dtb"
#define FDTFILE_VISIONFIVE2_1_3B \
	"starfive/jh7110-starfive-visionfive-2-v1.3b.dtb"

/* enable U74-mc hart1~hart4 prefetcher */
static void enable_prefetcher(void)
{
	u8 hart;
	u32 *reg;

	/* JH7110 use U74MC CORE IP, it include five cores(one S7 and four U7),
	 * but only U7 cores support prefetcher configuration
	 */
	for (hart = 1; hart < 5; hart++) {
		reg = (void *)(u64)(JH7110_L2_PREFETCHER_BASE_ADDR
					+ hart * JH7110_L2_PREFETCHER_HART_OFFSET);

		mb(); /* memory barrier */
		setbits_le32(reg, 0x1);
		mb(); /* memory barrier */
	}
}

/**
 * set_fdtfile() - set the $fdtfile variable based on the board revision
 */
static void set_fdtfile(void)
{
	u8 version;
	const char *fdtfile;

	version = get_pcb_revision_from_eeprom();
	switch (version) {
	case 'a':
	case 'A':
		fdtfile = FDTFILE_VISIONFIVE2_1_2A;
	        break;

	case 'b':
	case 'B':
	default:
		fdtfile = FDTFILE_VISIONFIVE2_1_3B;
	        break;
	};

	env_set("fdtfile", fdtfile);
}

int board_init(void)
{
	enable_caches();
	enable_prefetcher();

	return 0;
}

int board_late_init(void)
{
	if (CONFIG_IS_ENABLED(ID_EEPROM))
		set_fdtfile();

	return 0;
}

void *board_fdt_blob_setup(int *err)
{
	*err = 0;
	if (IS_ENABLED(CONFIG_OF_SEPARATE) || IS_ENABLED(CONFIG_OF_BOARD)) {
		if (gd->arch.firmware_fdt_addr)
			return (ulong *)(uintptr_t)gd->arch.firmware_fdt_addr;
	}

	return (ulong *)_end;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	return fdt_fixup_memory(blob, 0x40000000, gd->ram_size);
}
