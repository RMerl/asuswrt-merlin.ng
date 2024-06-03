/*
 * ti814x_evm.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_TI814X_EVM_H
#define __CONFIG_TI814X_EVM_H

#include <asm/arch/omap.h>

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
#define CONFIG_SYS_MALLOC_LEN		(1024 << 10)
#define CONFIG_MACH_TYPE		MACH_TYPE_TI8148EVM

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG		/* for ramdisk support */

/* commands to include */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80200000\0" \
	"fdtaddr=0x80F80000\0" \
	"rdaddr=0x81000000\0" \
	"bootfile=/boot/uImage\0" \
	"fdtfile=\0" \
	"console=ttyO0,115200n8\0" \
	"optargs=\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 ro\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"ramroot=/dev/ram0 rw ramdisk_size=65536 initrd=${rdaddr},64M\0" \
	"ramrootfstype=ext2\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
	"ramargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${ramroot} " \
		"rootfstype=${ramrootfstype}\0" \
	"loadramdisk=fatload mmc ${mmcdev} ${rdaddr} ramdisk.gz\0" \
	"loaduimagefat=fatload mmc ${mmcdev} ${loadaddr} ${bootfile}\0" \
	"loaduimage=ext2load mmc ${mmcdev}:2 ${loadaddr} ${bootfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"ramboot=echo Booting from ramdisk ...; " \
		"run ramargs; " \
		"bootm ${loadaddr}\0" \
	"fdtfile=ti814x-evm.dtb\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"echo SD/MMC found on device ${mmcdev};" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run loaduimage; then " \
			"run mmcboot;" \
		"fi;" \
	"fi;" \

/* Clock Defines */
#define V_OSCK			24000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)


/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START \
					+ PHYS_DRAM_1_SIZE - (8 << 12))

#define CONFIG_SYS_LOAD_ADDR		0x81000000	/* Default */

/**
 * Physical Memory Map
 */
#define PHYS_DRAM_1_SIZE		0x20000000	/* 512MB */
#define CONFIG_MAX_RAM_BANK_SIZE	(1024 << 20)	/* 1024MB */

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_ADDR		(NON_SECURE_SRAM_END - \
					 GENERATED_GBL_DATA_SIZE)

/**
 * Platform/Board specific defs
 */
#define CONFIG_SYS_TIMERBASE		0x4802E000
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(48000000)
#define CONFIG_SYS_NS16550_COM1		0x48020000	/* Base EVM has UART0 */

/* CPU */
#define CONFIG_ARCH_CPU_INIT

#define CONFIG_ENV_OVERWRITE

/* Defines for SPL */
#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)

#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION     1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME        "u-boot.img"

#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_SPI_U_BOOT_SIZE	0x40000

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80800000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

/*
 * Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/* Ethernet */
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT         10
#define CONFIG_PHY_ET1011C
#define CONFIG_PHY_ET1011C_TX_CLK_FIX

#endif	/* ! __CONFIG_TI814X_EVM_H */
