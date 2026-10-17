// SPDX-License-Identifier: GPL-2.0+
/*
 * Common init part for boards based on SDM845
 *
 * (C) Copyright 2021 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#include <button.h>
#include <init.h>
#include <env.h>
#include <common.h>
#include <asm/system.h>
#include <asm/gpio.h>
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

void reset_cpu(void)
{
	psci_system_reset();
}

__weak int board_init(void)
{
	return 0;
}

/* Check for vol- and power buttons */
__weak int misc_init_r(void)
{
	struct udevice *btn;
	int ret;
	enum button_state_t state;

	ret = button_get_by_label("pwrkey", &btn);
	if (ret < 0) {
		printf("Couldn't find power button!\n");
		return ret;
	}

	state = button_get_state(btn);
	if (state == BUTTON_ON) {
		env_set("key_power", "1");
		printf("Power button pressed\n");
	} else {
		env_set("key_power", "0");
	}

	/*
	 * search for kaslr address, set by primary bootloader by searching first
	 * 0x100 relocated bytes at u-boot's initial load address range
	 */
	uintptr_t start = gd->ram_base;
	uintptr_t end = start + 0x800000;
	u8 *addr = (u8 *)start;
	phys_addr_t *relocaddr = (phys_addr_t *)gd->relocaddr;
	u32 block_size = 0x1000;

	while (memcmp(addr, relocaddr, 0x100) && (uintptr_t)addr < end)
		addr += block_size;

	if ((uintptr_t)addr >= end)
		printf("KASLR not found in range 0x%lx - 0x%lx", start, end);
	else
		env_set_addr("KASLR", addr);

	return 0;
}
