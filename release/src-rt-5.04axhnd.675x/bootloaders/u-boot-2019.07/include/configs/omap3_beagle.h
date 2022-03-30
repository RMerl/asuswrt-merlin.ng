/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Configuration settings for the TI OMAP3530 Beagle board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ti_omap3_common.h>

/*
 * We are only ever GP parts and will utilize all of the "downloaded image"
 * area in SRAM which starts at 0x40200000 and ends at 0x4020FFFF (64KB).
 */

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* NAND */
#if defined(CONFIG_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT      64
#define CONFIG_SYS_NAND_PAGE_SIZE       2048
#define CONFIG_SYS_NAND_OOBSIZE         64
#define CONFIG_SYS_NAND_BLOCK_SIZE      (128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS   NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS          {2, 3, 4, 5, 6, 7, 8, 9,\
                                         10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE         512
#define CONFIG_SYS_NAND_ECCBYTES        3
#define CONFIG_NAND_OMAP_ECCSCHEME      OMAP_ECC_BCH8_CODE_HW_DETECTION_SW
#define CONFIG_SYS_NAND_U_BOOT_OFFS     0x80000
#define CONFIG_SYS_ENV_SECT_SIZE        SZ_128K
#define CONFIG_ENV_OFFSET               0x260000
#define CONFIG_ENV_ADDR                 0x260000
#define CONFIG_ENV_OVERWRITE
/* NAND: SPL falcon mode configs */
#if defined(CONFIG_SPL_OS_BOOT)
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS 0x2a0000
#endif /* CONFIG_SPL_OS_BOOT */
#endif /* CONFIG_NAND */

/* USB EHCI */
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	147

/* Enable Multi Bus support for I2C */
#define CONFIG_I2C_MULTI_BUS

/* DSS Support */

/* TWL4030 LED Support */

/* Environment */
#define CONFIG_ENV_SIZE                 SZ_128K

#define CONFIG_PREBOOT                  "usb start"

#define MEM_LAYOUT_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV

#define BOOTENV_DEV_LEGACY_MMC(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"setenv mmcdev " #instance "; " \
	"run mmcboot\0"
#define BOOTENV_DEV_NAME_LEGACY_MMC(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#if defined(CONFIG_NAND)

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"if test ${mtdids} = '' || test ${mtdparts} = '' ; then " \
		"echo NAND boot disabled: No mtdids and/or mtdparts; " \
	"else " \
		"run nandboot; " \
	"fi\0"
#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(LEGACY_MMC, legacy_mmc, 0) \
	func(UBIFS, ubifs, 0) \
	func(NAND, nand, 0)

#else /* !CONFIG_NAND */

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(LEGACY_MMC, legacy_mmc, 0)

#endif /* CONFIG_NAND */

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdt_high=0xffffffff\0" \
	"console=ttyO2,115200n8\0" \
	"bootdir=/boot\0" \
	"bootenv=uEnv.txt\0" \
	"bootfile=zImage\0" \
	"bootpart=0:2\0" \
	"bootubivol=rootfs\0" \
	"bootubipart=rootfs\0" \
	"usbtty=cdc_acm\0" \
	"mpurate=auto\0" \
	"buddy=none\0" \
	"camera=none\0" \
	"vram=12M\0" \
	"dvimode=640x480MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"defaultargs=setenv defargs " \
		"mpurate=${mpurate} " \
		"buddy=${buddy} "\
		"camera=${camera} "\
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay}\0" \
	"optargs=\0" \
	"findfdt=" \
		"if test $beaglerev = AxBx; then " \
			"setenv fdtfile omap3-beagle.dtb; fi; " \
		"if test $beaglerev = Cx; then " \
			"setenv fdtfile omap3-beagle.dtb; fi; " \
		"if test $beaglerev = C4; then " \
			"setenv fdtfile omap3-beagle.dtb; fi; " \
		"if test $beaglerev = xMAB; then " \
			"setenv fdtfile omap3-beagle-xm-ab.dtb; fi; " \
		"if test $beaglerev = xMC; then " \
			"setenv fdtfile omap3-beagle-xm.dtb; fi; " \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine device tree to use; fi\0" \
	"mmcdev=0\0" \
	"mmcpart=2\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"mmcargs=run defaultargs; setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${defargs} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"userbutton_xm=gpio input 4;\0" \
	"userbutton_nonxm=gpio input 7;\0" \
	"userbutton=if gpio input 173; then " \
			"run userbutton_xm; " \
		"else " \
			"run userbutton_nonxm; " \
		"fi;\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"ext4bootenv=ext4load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc${mmcdev} ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"mmcbootenv=setenv bootpart ${mmcdev}:${mmcpart}; " \
		"mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"if run userbutton; then " \
				"setenv bootenv uEnv.txt;" \
			"else " \
				"setenv bootenv user.txt;" \
			"fi;" \
			"run loadbootenv && run importbootenv; " \
			"run ext4bootenv && run importbootenv; " \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...; " \
				"run uenvcmd; " \
			"fi; " \
		"fi\0" \
	"validatefdt=" \
		"if test $beaglerev = xMAB; then " \
			"if test ! -e mmc ${bootpart} ${bootdir}/${fdtfile}; then " \
				"setenv fdtfile omap3-beagle-xm.dtb; " \
			"fi; " \
		"fi; \0" \
	"loadimage=ext4load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"loaddtb=run validatefdt; ext4load mmc ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"mmcboot=run mmcbootenv; " \
		"if run loadimage && run loaddtb; then " \
			"echo Booting ${bootdir}/${bootfile} from mmc ${bootpart} ...; " \
			"run mmcargs; " \
			"if test ${bootfile} = uImage; then " \
				"bootm ${loadaddr} - ${fdtaddr}; " \
			"fi; " \
			"if test ${bootfile} = zImage; then " \
				"bootz ${loadaddr} - ${fdtaddr}; " \
			"fi; " \
		"fi\0" \
	"nandroot=ubi0:rootfs ubi.mtd=rootfs rw\0" \
	"nandrootfstype=ubifs rootwait\0" \
	"nandargs=run defaultargs; setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${defargs} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"nandboot=if nand read ${loadaddr} kernel && nand read ${fdtaddr} dtb; then " \
			"echo Booting uImage from NAND MTD 'kernel' partition ...; " \
			"run nandargs; " \
			"bootm ${loadaddr} - ${fdtaddr}; " \
		"fi\0" \
	"loadramdisk=ext4load mmc ${bootpart} ${rdaddr} ${bootdir}/${ramdisk}\0" \
	"ramdisk=rootfs.ext2.gz.uboot\0" \
	"ramdisk_size=16384\0" \
	"ramroot=/dev/ram rw\0" \
	"ramrootfstype=ext2\0" \
	"ramargs=run defaultargs; setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${defargs} " \
		"${optargs} " \
		"root=${ramroot} ramdisk_size=${ramdisk_size} " \
	 	"rootfstype=${ramrootfstype}\0" \
	"ramboot=run mmcbootenv; " \
		"if run loadimage && run loaddtb && run loadramdisk; then " \
			"echo Booting ${bootdir}/${bootfile} from mmc ${bootpart} w/ramdisk ...; " \
			"run ramargs; " \
			"bootz ${loadaddr} ${rdaddr} ${fdtaddr}; " \
		"fi\0" \
	BOOTENV

#endif /* __CONFIG_H */
