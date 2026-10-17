/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2017-2019 Intel Corporation <www.intel.com>
 *
 */

#ifndef __CONFIG_SOCFPGA_SOC64_COMMON_H__
#define __CONFIG_SOCFPGA_SOC64_COMMON_H__

#include <asm/arch/base_addr_soc64.h>
#include <asm/arch/handoff_soc64.h>
#include <linux/stringify.h>

/*
 * U-Boot general configurations
 */
/* sysmgr.boot_scratch_cold4 & 5 (64bit) will be used for PSCI_CPU_ON call */
#define CPU_RELEASE_ADDR		0xFFD12210

/*
 * U-Boot console configurations
 */

/* Extend size of kernel image for uncompression */

/*
 * U-Boot run time memory configurations
 */
#define CFG_SYS_INIT_RAM_ADDR	0xFFE00000
#define CFG_SYS_INIT_RAM_SIZE	0x40000

/*
 * U-Boot environment configurations
 */

/*
 * Environment variable
 */
#define CFG_EXTRA_ENV_SETTINGS \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"bootfile=" CONFIG_BOOTFILE "\0" \
	"fdt_addr=8000000\0" \
	"fdtimage=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"mmcroot=/dev/mmcblk0p2\0" \
	"mmcboot=setenv bootargs " CONFIG_BOOTARGS \
		" root=${mmcroot} rw rootwait;" \
		"booti ${loadaddr} - ${fdt_addr}\0" \
	"mmcload=mmc rescan;" \
		"load mmc 0:1 ${loadaddr} ${bootfile};" \
		"load mmc 0:1 ${fdt_addr} ${fdtimage}\0" \
	"mmcfitboot=setenv bootargs " CONFIG_BOOTARGS \
		" root=${mmcroot} rw rootwait;" \
		"bootm ${loadaddr}\0" \
	"mmcfitload=mmc rescan;" \
		"load mmc 0:1 ${loadaddr} ${bootfile}\0" \
	"linux_qspi_enable=if sf probe; then " \
		"echo Enabling QSPI at Linux DTB...;" \
		"fdt addr ${fdt_addr}; fdt resize;" \
		"fdt set /soc/spi@ff8d2000 status okay;" \
		"fdt set /soc/clkmgr/clocks/qspi_clk clock-frequency " \
		" ${qspi_clock}; fi; \0" \
	"scriptaddr=0x02100000\0" \
	"scriptfile=u-boot.scr\0" \
	"fatscript=if fatload mmc 0:1 ${scriptaddr} ${scriptfile};" \
		   "then source ${scriptaddr}; fi\0" \
	"socfpga_legacy_reset_compat=1\0"

/*
 * External memory configurations
 */
#define PHYS_SDRAM_1			0x0
#define PHYS_SDRAM_1_SIZE		(1 * 1024 * 1024 * 1024)
#define CFG_SYS_SDRAM_BASE		0

/*
 * Serial / UART configurations
 */
#define CFG_SYS_NS16550_CLK		100000000

/*
 * SDMMC configurations
 */
/*
 * Flash configurations
 */

/*
 * L4 Watchdog
 */
#ifdef CONFIG_TARGET_SOCFPGA_STRATIX10
#ifndef __ASSEMBLY__
unsigned int cm_get_l4_sys_free_clk_hz(void);
#define CFG_DW_WDT_CLOCK_KHZ		(cm_get_l4_sys_free_clk_hz() / 1000)
#endif
#else
#define CFG_DW_WDT_CLOCK_KHZ		100000
#endif

/*
 * SPL memory layout
 *
 * On chip RAM
 * 0xFFE0_0000 ...... Start of OCRAM
 * SPL code, rwdata
 * empty space
 * 0xFFEx_xxxx ...... Top of stack (grows down)
 * 0xFFEy_yyyy ...... Global Data
 * 0xFFEz_zzzz ...... Malloc prior relocation (size CONFIG_SYS_MALLOC_F_LEN)
 * 0xFFE3_F000 ...... Hardware handdoff blob (size 4KB)
 * 0xFFE3_FFFF ...... End of OCRAM
 *
 * SDRAM
 * 0x0000_0000 ...... Start of SDRAM_1
 * unused / empty space for image loading
 * Size 64MB   ...... MALLOC (size CONFIG_SPL_SYS_MALLOC_SIZE)
 * Size 1MB    ...... BSS (size CONFIG_SPL_BSS_MAX_SIZE)
 * 0x8000_0000 ...... End of SDRAM_1 (assume 2GB)
 *
 */

#endif	/* __CONFIG_SOCFPGA_SOC64_COMMON_H__ */
