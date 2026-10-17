/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ti_armv7_common.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - https://www.ti.com/
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

/*
 * We setup defaults based on constraints from the Linux kernel, which should
 * also be safe elsewhere.  We have the default load at 32MB into DDR (for
 * the kernel), FDT above 128MB (the maximum location for the end of the
 * kernel), and the ramdisk 512KB above that (allowing for hopefully never
 * seen large trees).  We say all of this must be within the first 256MB
 * as that will normally be within the kernel lowmem and thus visible via
 * bootm_size and we only run on platforms with 256MB or more of memory.
 *
 * As a temporary storage for DTBO blobs (which should be applied into DTB
 * blob), we use the location 15.5 MB above the ramdisk. If someone wants to
 * use ramdisk bigger than 15.5 MB, then DTBO can be loaded and applied to DTB
 * blob before loading the ramdisk, as DTBO location is only used as a temporary
 * storage, and can be re-used after 'fdt apply' command is done.
 */
#define DEFAULT_LINUX_BOOT_ENV \
	"loadaddr=0x82000000\0" \
	"kernel_addr_r=0x82000000\0" \
	"fdtaddr=0x88000000\0" \
	"dtboaddr=0x89000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"fdtoverlay_addr_r=0x89000000\0" \
	"rdaddr=0x88080000\0" \
	"ramdisk_addr_r=0x88080000\0" \
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"bootm_size=0x10000000\0" \
	"boot_fdt=try\0"

#define DEFAULT_FIT_TI_ARGS \
	"boot_fit=0\0" \
	"addr_fit=0x90000000\0" \
	"name_fit=fitImage\0" \
	"update_to_fit=setenv loadaddr ${addr_fit}; setenv bootfile ${name_fit}\0" \
	"get_overlaystring=" \
		"for overlay in $name_overlays;" \
		"do;" \
		"setenv overlaystring ${overlaystring}'#'${overlay};" \
		"done;\0" \
	"get_fit_config=setexpr name_fit_config gsub / _ conf-${fdtfile}\0" \
	"run_fit=run get_fit_config; bootm ${addr_fit}#${name_fit_config}${overlaystring}\0" \

/*
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */
#define CFG_SYS_SDRAM_BASE		0x80000000

/* If DM_I2C, enable non-DM I2C support */

/*
 * The following are general good-enough settings for U-Boot.  We set a
 * large malloc pool as we generally have a lot of DDR, and we opt for
 * function over binary size in the main portion of U-Boot as this is
 * generally easily constrained later if needed.  We enable the config
 * options that give us information in the environment about what board
 * we are on so we do not need to rely on the command prompt.  We set a
 * console baudrate of 115200 and use the default baud rate table.
 */

/* As stated above, the following choices are optional. */

/* Console I/O Buffer Size */
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

#ifdef CONFIG_SPL_OS_BOOT
/* FAT */

/* RAW SD card / eMMC */
#endif

/* General parts of the framework, required. */

#ifdef CONFIG_MTD_RAW_NAND
#define CFG_SYS_NAND_U_BOOT_START	CONFIG_TEXT_BASE
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

#ifdef CONFIG_ARM64
#ifdef CONFIG_DISTRO_DEFAULTS
#ifdef CONFIG_CMD_PXE
# define BOOT_TARGET_PXE(func) func(PXE, pxe, na)
#else
# define BOOT_TARGET_PXE(func)
#endif

#ifdef CONFIG_CMD_DHCP
# define BOOT_TARGET_DHCP(func) func(DHCP, dhcp, na)
#else
# define BOOT_TARGET_DHCP(func)
#endif

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_MMC(func) \
	func(TI_MMC, ti_mmc, na) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1)
#else
#define BOOT_TARGET_MMC(func)
#endif

#define BOOTENV_DEV_TI_MMC(devtypeu, devtypel, instance)

#define BOOTENV_DEV_NAME_TI_MMC(devtyeu, devtypel, instance)		\
	"ti_mmc "

#ifdef CONFIG_CMD_USB
# define BOOT_TARGET_USB(func)	func(USB, usb, 0)
#else
# define BOOT_TARGET_USB(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_MMC(func) \
	BOOT_TARGET_USB(func) \
	BOOT_TARGET_PXE(func) \
	BOOT_TARGET_DHCP(func)

#include <config_distro_bootcmd.h>

/* Incorporate settings into the U-Boot environment */
#define CFG_EXTRA_ENV_SETTINGS					\
	BOOTENV

#endif /* CONFIG_DISTRO_DEFAULTS */

#endif /* CONFIG_ARM64 */

#endif	/* __CONFIG_TI_ARMV7_COMMON_H__ */
