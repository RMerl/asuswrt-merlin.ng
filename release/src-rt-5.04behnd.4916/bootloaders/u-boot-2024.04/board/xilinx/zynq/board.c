// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 * (C) Copyright 2013 - 2018 Xilinx, Inc.
 */

#include <common.h>
#include <debug_uart.h>
#include <dfu.h>
#include <init.h>
#include <log.h>
#include <dm/uclass.h>
#include <env.h>
#include <env_internal.h>
#include <fdtdec.h>
#include <fpga.h>
#include <malloc.h>
#include <memalign.h>
#include <mmc.h>
#include <watchdog.h>
#include <wdt.h>
#include <zynqpl.h>
#include <asm/global_data.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include "../common/board.h"

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_DEBUG_UART_BOARD_INIT)
void board_debug_uart_init(void)
{
	/* Add initialization sequence if UART is not configured */
}
#endif

int board_init(void)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		printf("Silicon version:\t%d\n", zynq_get_silicon_version());

	if (CONFIG_IS_ENABLED(DM_I2C) && CONFIG_IS_ENABLED(I2C_EEPROM))
		xilinx_read_eeprom();

	return 0;
}

int board_late_init(void)
{
	int env_targets_len = 0;
	const char *mode;
	char *new_targets;
	char *env_targets;

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	if (!IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG))
		return 0;

	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
	case ZYNQ_BM_QSPI:
		mode = "qspi";
		env_set("modeboot", "qspiboot");
		break;
	case ZYNQ_BM_NAND:
		mode = "nand";
		env_set("modeboot", "nandboot");
		break;
	case ZYNQ_BM_NOR:
		mode = "nor";
		env_set("modeboot", "norboot");
		break;
	case ZYNQ_BM_SD:
		mode = "mmc0";
		env_set("modeboot", "sdboot");
		break;
	case ZYNQ_BM_JTAG:
		mode = "jtag pxe dhcp";
		env_set("modeboot", "jtagboot");
		break;
	default:
		mode = "";
		env_set("modeboot", "");
		break;
	}

	/*
	 * One terminating char + one byte for space between mode
	 * and default boot_targets
	 */
	env_targets = env_get("boot_targets");
	if (env_targets)
		env_targets_len = strlen(env_targets);

	new_targets = calloc(1, strlen(mode) + env_targets_len + 2);
	if (!new_targets)
		return -ENOMEM;

	sprintf(new_targets, "%s %s", mode,
		env_targets ? env_targets : "");

	env_set("boot_targets", new_targets);

	return board_late_init_xilinx();
}

#if !defined(CFG_SYS_SDRAM_BASE) && !defined(CFG_SYS_SDRAM_SIZE)
int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	zynq_ddrc_init();

	return 0;
}
#else
int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
				    CFG_SYS_SDRAM_SIZE);

	zynq_ddrc_init();

	return 0;
}
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = zynq_slcr_get_boot_mode() & ZYNQ_BM_MASK;

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode) {
	case ZYNQ_BM_SD:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_FAT))
			return ENVL_FAT;
		if (IS_ENABLED(CONFIG_ENV_IS_IN_EXT4))
			return ENVL_EXT4;
		return ENVL_NOWHERE;
	case ZYNQ_BM_NAND:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_NAND))
			return ENVL_NAND;
		if (IS_ENABLED(CONFIG_ENV_IS_IN_UBI))
			return ENVL_UBI;
		return ENVL_NOWHERE;
	case ZYNQ_BM_NOR:
	case ZYNQ_BM_QSPI:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		return ENVL_NOWHERE;
	case ZYNQ_BM_JTAG:
	default:
		return ENVL_NOWHERE;
	}
}

#if defined(CONFIG_SET_DFU_ALT_INFO)

#define DFU_ALT_BUF_LEN                SZ_1K

void set_dfu_alt_info(char *interface, char *devstr)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	if (env_get("dfu_alt_info"))
		return;

	memset(buf, 0, sizeof(buf));

	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
	case ZYNQ_BM_SD:
		snprintf(buf, DFU_ALT_BUF_LEN,
			 "mmc 0=boot.bin fat 0 1;"
			 "%s fat 0 1", CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
		break;
#if defined(CONFIG_SPL_SPI_LOAD)
	case ZYNQ_BM_QSPI:
		snprintf(buf, DFU_ALT_BUF_LEN,
			 "sf 0:0=boot.bin raw 0 0x1500000;"
			 "%s raw 0x%x 0x500000",
			 CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
			 CONFIG_SYS_SPI_U_BOOT_OFFS);
		break;
#endif
	default:
		return;
	}

	env_set("dfu_alt_info", buf);
	puts("DFU alt info setting: done\n");
}
#endif
