/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ti_armv7_common.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * The various ARMv7 SoCs from TI all share a number of IP blocks when
 * implementing a given feature.  Rather than define these in every
 * board or even SoC common file, we define a common file to be re-used
 * in all cases.  While technically true that some of these details are
 * configurable at the board design, they are common throughout SoC
 * reference platforms as well as custom designs and become de facto
 * standards.
 */

#ifndef __CONFIG_TI_ARMV7_COMMON_H__
#define __CONFIG_TI_ARMV7_COMMON_H__

/* Support both device trees and ATAGs. */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Our DDR memory always starts at 0x80000000 and U-Boot shall have
 * relocated itself to higher in memory by the time this value is used.
 * However, set this to a 32MB offset to allow for easier Linux kernel
 * booting as the default is often used as the kernel load address.
 */
#define CONFIG_SYS_LOAD_ADDR		0x82000000

/*
 * We setup defaults based on constraints from the Linux kernel, which should
 * also be safe elsewhere.  We have the default load at 32MB into DDR (for
 * the kernel), FDT above 128MB (the maximum location for the end of the
 * kernel), and the ramdisk 512KB above that (allowing for hopefully never
 * seen large trees).  We say all of this must be within the first 256MB
 * as that will normally be within the kernel lowmem and thus visible via
 * bootm_size and we only run on platforms with 256MB or more of memory.
 */
#define DEFAULT_LINUX_BOOT_ENV \
	"loadaddr=0x82000000\0" \
	"kernel_addr_r=0x82000000\0" \
	"fdtaddr=0x88000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"rdaddr=0x88080000\0" \
	"ramdisk_addr_r=0x88080000\0" \
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"bootm_size=0x10000000\0" \
	"boot_fdt=try\0"

#define DEFAULT_FIT_TI_ARGS \
	"boot_fit=0\0" \
	"fit_loadaddr=0x90000000\0" \
	"fit_bootfile=fitImage\0" \
	"update_to_fit=setenv loadaddr ${fit_loadaddr}; setenv bootfile ${fit_bootfile}\0" \
	"loadfit=run args_mmc; bootm ${loadaddr}#${fdtfile};\0" \

/*
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

#ifndef CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_INIT_SP_ADDR         (NON_SECURE_SRAM_END - \
						GENERATED_GBL_DATA_SIZE)
#endif

/* Timer information. */
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/* If DM_I2C, enable non-DM I2C support */
#if !defined(CONFIG_DM_I2C)
#define CONFIG_I2C
#define CONFIG_SYS_I2C
#endif

/*
 * The following are general good-enough settings for U-Boot.  We set a
 * large malloc pool as we generally have a lot of DDR, and we opt for
 * function over binary size in the main portion of U-Boot as this is
 * generally easily constrained later if needed.  We enable the config
 * options that give us information in the environment about what board
 * we are on so we do not need to rely on the command prompt.  We set a
 * console baudrate of 115200 and use the default baud rate table.
 */
#define CONFIG_SYS_MALLOC_LEN		SZ_32M
#define CONFIG_ENV_OVERWRITE		/* Overwrite ethaddr / serial# */

/* As stated above, the following choices are optional. */

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS		64

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		1024
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * When we have SPI, NOR or NAND flash we expect to be making use of
 * mtdparts, both for ease of use in U-Boot and for passing information
 * on to the Linux kernel.
 */

/*
 * Our platforms make use of SPL to initalize the hardware (primarily
 * memory) enough for full U-Boot to be loaded. We make use of the general
 * SPL framework found under common/spl/.  Given our generally common memory
 * map, we set a number of related defaults and sizes here.
 */
#if !defined(CONFIG_NOR_BOOT) && \
	!(defined(CONFIG_QSPI_BOOT) && defined(CONFIG_AM43XX))

/*
 * We also support Falcon Mode so that the Linux kernel can be booted
 * directly from SPL. This is not currently available on HS devices.
 */

/*
 * Place the image at the start of the ROM defined image space (per
 * CONFIG_SPL_TEXT_BASE and we limit our size to the ROM-defined
 * downloaded image area minus 1KiB for scratch space.  We initalize DRAM as
 * soon as we can so that we can place stack, malloc and BSS there.  We load
 * U-Boot itself into memory at 0x80800000 for legacy reasons (to not conflict
 * with older SPLs).  We have our BSS be placed 2MiB after this, to allow for
 * the default Linux kernel address of 0x80008000 to work with most sized
 * kernels, in the Falcon Mode case.  We have the SPL malloc pool at the end
 * of the BSS area.  We suggest that the stack be placed at 32MiB after the
 * start of DRAM to allow room for all of the above (handled in Kconfig).
 */
#ifndef CONFIG_SPL_BSS_START_ADDR
#define CONFIG_SPL_BSS_START_ADDR	0x80a00000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */
#endif
#ifndef CONFIG_SYS_SPL_MALLOC_START
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					 CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_8M
#endif
#ifndef CONFIG_SPL_MAX_SIZE
#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)
#endif


/* FAT sd card locations. */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#ifndef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"
#endif

#ifdef CONFIG_SPL_OS_BOOT
/* FAT */
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME		"uImage"
#define CONFIG_SPL_FS_LOAD_ARGS_NAME		"args"

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x1700  /* address 0x2E0000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x1500  /* address 0x2A0000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x200   /* 256KiB */
#endif

/* General parts of the framework, required. */

#ifdef CONFIG_NAND
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#endif
#endif /* !CONFIG_NOR_BOOT */

/* Generic Environment Variables */

#ifdef CONFIG_CMD_NET
#define NETARGS \
	"static_ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}" \
		"::off\0" \
	"nfsopts=nolock\0" \
	"rootpath=/export/rootfs\0" \
	"netloadimage=tftp ${loadaddr} ${bootfile}\0" \
	"netloadfdt=tftp ${fdtaddr} ${fdtfile}\0" \
	"netargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} rw " \
		"ip=dhcp\0" \
	"netboot=echo Booting from network ...; " \
		"setenv autoload no; " \
		"dhcp; " \
		"run netloadimage; " \
		"run netloadfdt; " \
		"run netargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0"
#else
#define NETARGS ""
#endif

#endif	/* __CONFIG_TI_ARMV7_COMMON_H__ */
