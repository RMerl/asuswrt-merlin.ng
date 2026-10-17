/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated.
 * Sricharan R	  <r.sricharan@ti.com>
 *
 * Derived from OMAP4 done by:
 *	Aneesh V <aneesh@ti.com>
 *
 * TI OMAP5 AND DRA7XX common configuration settings
 *
 * For more details, please see the technical documents listed at
 * https://www.ti.com/product/omap5432
 */

#ifndef __CONFIG_TI_OMAP5_COMMON_H
#define __CONFIG_TI_OMAP5_COMMON_H

/* Use General purpose timer 1 */
#define CFG_SYS_TIMERBASE		GPT2_BASE

#include <linux/stringify.h>

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

#include <configs/ti_armv7_omap.h>

/*
 * Hardware drivers
 */
#define CFG_SYS_NS16550_CLK		48000000

/*
 * Environment setup
 */

#ifndef DFUARGS
#define DFUARGS
#endif

#include <env/ti/mmc.h>
#include <env/ti/nand.h>

#ifndef CONSOLEDEV
#define CONSOLEDEV "ttyS2"
#endif

#ifndef PARTS_DEFAULT
/*
 * Default GPT tables for eMMC (Linux and Android). Notes:
 *   1. Keep partitions aligned to erase group size (512 KiB) when possible
 *   2. Keep partitions in sync with DFU_ALT_INFO_EMMC (see dfu.h)
 *   3. Keep 'bootloader' partition (U-Boot proper) start address in sync with
 *      CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR (see common/spl/Kconfig)
 */
#define PARTS_DEFAULT \
	/* Linux partitions */ \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=bootloader,start=384K,size=1792K,uuid=${uuid_gpt_bootloader};" \
	"name=rootfs,start=2688K,size=-,uuid=${uuid_gpt_rootfs}\0" \
	/* Android partitions */ \
	"partitions_android=" \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=xloader,start=128K,size=256K,uuid=${uuid_gpt_xloader};" \
	"name=bootloader,size=2048K,uuid=${uuid_gpt_bootloader};" \
	"name=uboot-env,start=2432K,size=256K,uuid=${uuid_gpt_reserved};" \
	"name=misc,size=128K,uuid=${uuid_gpt_misc};" \
	"name=boot_a,size=20M,uuid=${uuid_gpt_boot_a};" \
	"name=boot_b,size=20M,uuid=${uuid_gpt_boot_b};" \
	"name=dtbo_a,size=8M,uuid=${uuid_gpt_dtbo_a};" \
	"name=dtbo_b,size=8M,uuid=${uuid_gpt_dtbo_b};" \
	"name=vbmeta_a,size=64K,uuid=${uuid_gpt_vbmeta_a};" \
	"name=vbmeta_b,size=64K,uuid=${uuid_gpt_vbmeta_b};" \
	"name=recovery,size=64M,uuid=${uuid_gpt_recovery};" \
	"name=super,size=2560M,uuid=${uuid_gpt_super};" \
	"name=metadata,size=16M,uuid=${uuid_gpt_metadata};" \
	"name=userdata,size=-,uuid=${uuid_gpt_userdata}"
#endif /* PARTS_DEFAULT */

#if defined(CONFIG_CMD_AVB)
#define AVB_VERIFY_CHECK "if run avb_verify; then " \
				"echo AVB verification OK.;" \
				"set bootargs $bootargs $avb_bootargs;" \
			"else " \
				"echo AVB verification failed.;" \
			"exit; fi;"
#define AVB_VERIFY_CMD "avb_verify=avb init 1; avb verify $slot_suffix;\0"
#else
#define AVB_VERIFY_CHECK ""
#define AVB_VERIFY_CMD ""
#endif

#define CONTROL_PARTITION "misc"

#if defined(CONFIG_CMD_AB_SELECT)
#define AB_SELECT_SLOT \
	"if part number mmc 1 " CONTROL_PARTITION " control_part_number; " \
	"then " \
		"echo " CONTROL_PARTITION \
			" partition number:${control_part_number};" \
		"ab_select slot_name mmc ${mmcdev}:${control_part_number};" \
	"else " \
		"echo " CONTROL_PARTITION " partition not found;" \
		"exit;" \
	"fi;" \
	"setenv slot_suffix _${slot_name};"
#define AB_SELECT_ARGS \
	"setenv bootargs_ab androidboot.slot_suffix=${slot_suffix}; " \
	"echo A/B cmdline addition: ${bootargs_ab};" \
	"setenv bootargs ${bootargs} ${bootargs_ab};"
#else
#define AB_SELECT_SLOT ""
#define AB_SELECT_ARGS ""
#endif

/*
 * Prepares complete device tree blob for current board (for Android boot).
 *
 * Boot image or recovery image should be loaded into $loadaddr prior to running
 * these commands. The logic of these commnads is next:
 *
 *   1. Read correct DTB for current SoC/board from boot image in $loadaddr
 *      to $fdtaddr
 *   2. Merge all needed DTBO for current board from 'dtbo' partition into read
 *      DTB
 *   3. User should provide $fdtaddr as 3rd argument to 'bootm'
 */
#define PREPARE_FDT \
	"echo Preparing FDT...; " \
	"if test $board_name = am57xx_evm_reva3; then " \
		"echo \"  Reading DTBO partition...\"; " \
		"part start mmc ${mmcdev} dtbo${slot_suffix} p_dtbo_start; " \
		"part size mmc ${mmcdev} dtbo${slot_suffix} p_dtbo_size; " \
		"mmc read ${dtboaddr} ${p_dtbo_start} ${p_dtbo_size}; " \
		"echo \"  Reading DTB for AM57x EVM RevA3...\"; " \
		"abootimg get dtb --index=0 dtb_start dtb_size; " \
		"cp.b $dtb_start $fdtaddr $dtb_size; " \
		"fdt addr $fdtaddr 0x80000; " \
		"echo \"  Applying DTBOs for AM57x EVM RevA3...\"; " \
		"adtimg addr $dtboaddr; " \
		"adtimg get dt --index=0 dtbo0_addr dtbo0_size; " \
		"fdt apply $dtbo0_addr; " \
		"adtimg get dt --index=1 dtbo1_addr dtbo1_size; " \
		"fdt apply $dtbo1_addr; " \
	"elif test $board_name = beagle_x15_revc; then " \
		"echo \"  Reading DTB for Beagle X15 RevC...\"; " \
		"abootimg get dtb --index=0 dtb_start dtb_size; " \
		"cp.b $dtb_start $fdtaddr $dtb_size; " \
		"fdt addr $fdtaddr 0x80000; " \
	"else " \
		"echo Error: Android boot is not supported for $board_name; " \
		"exit; " \
	"fi; " \

#define DEFAULT_COMMON_BOOT_TI_ARGS \
	"console=" CONSOLEDEV ",115200n8\0" \
	"fdtfile=undefined\0" \
	"finduuid=part uuid mmc 0:2 uuid\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M\0" \
	AVB_VERIFY_CMD \
	"partitions=" PARTS_DEFAULT "\0" \
	"optargs=\0" \
	"dofastboot=0\0" \
	"emmc_android_boot=" \
		"setenv mmcdev 1; " \
		"mmc dev $mmcdev; " \
		"mmc rescan; " \
		AB_SELECT_SLOT \
		"if bcb load " __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) " " \
		CONTROL_PARTITION "; then " \
			"setenv ardaddr -; " \
			"if bcb test command = bootonce-bootloader; then " \
				"echo Android: Bootloader boot...; " \
				"bcb clear command; bcb store; " \
				"fastboot 1; " \
				"exit; " \
			"elif bcb test command = boot-recovery; then " \
				"echo Android: Recovery boot...; " \
				"setenv ardaddr $loadaddr;" \
				"setenv apart recovery; " \
			"else " \
				"echo Android: Normal boot...; " \
				"setenv ardaddr $loadaddr; " \
				"setenv apart boot${slot_suffix}; " \
			"fi; " \
		"else " \
			"echo Warning: BCB is corrupted or does not exist; " \
			"echo Android: Normal boot...; " \
		"fi; " \
		"setenv eval_bootargs setenv bootargs $bootargs; " \
		"run eval_bootargs; " \
		"setenv machid fe6; " \
		AVB_VERIFY_CHECK \
		AB_SELECT_ARGS \
		"if part start mmc $mmcdev $apart boot_start; then " \
			"part size mmc $mmcdev $apart boot_size; " \
			"mmc read $loadaddr $boot_start $boot_size; " \
			PREPARE_FDT \
			"bootm $loadaddr $ardaddr $fdtaddr; " \
		"else " \
			"echo $apart partition not found; " \
			"exit; " \
		"fi;\0"

#define DEFAULT_FDT_TI_ARGS \
	"findfdt="\
		"if test $board_name = omap5_uevm; then " \
			"setenv fdtfile omap5-uevm.dtb; fi; " \
		"if test $board_name = dra7xx; then " \
			"setenv fdtfile dra7-evm.dtb; fi;" \
		"if test $board_name = dra72x-revc; then " \
			"setenv fdtfile dra72-evm-revc.dtb; fi;" \
		"if test $board_name = dra72x; then " \
			"setenv fdtfile dra72-evm.dtb; fi;" \
		"if test $board_name = dra71x; then " \
			"setenv fdtfile dra71-evm.dtb; fi;" \
		"if test $board_name = dra76x_acd; then " \
			"setenv fdtfile dra76-evm.dtb; fi;" \
		"if test $board_name = beagle_x15; then " \
			"setenv fdtfile am57xx-beagle-x15.dtb; fi;" \
		"if test $board_name = beagle_x15_revb1; then " \
			"setenv fdtfile am57xx-beagle-x15-revb1.dtb; fi;" \
		"if test $board_name = beagle_x15_revc; then " \
			"setenv fdtfile am57xx-beagle-x15-revc.dtb; fi;" \
		"if test $board_name = am5729_beagleboneai; then " \
			"setenv fdtfile am5729-beagleboneai.dtb; fi;" \
		"if test $board_name = am572x_idk; then " \
			"setenv fdtfile am572x-idk.dtb; fi;" \
		"if test $board_name = am574x_idk; then " \
			"setenv fdtfile am574x-idk.dtb; fi;" \
		"if test $board_name = am57xx_evm; then " \
			"setenv fdtfile am57xx-beagle-x15.dtb; fi;" \
		"if test $board_name = am57xx_evm_reva3; then " \
			"setenv fdtfile am57xx-beagle-x15.dtb; fi;" \
		"if test $board_name = am571x_idk; then " \
			"setenv fdtfile am571x-idk.dtb; fi;" \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine device tree to use; fi; \0"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	DEFAULT_MMC_TI_ARGS \
	DEFAULT_FIT_TI_ARGS \
	DEFAULT_COMMON_BOOT_TI_ARGS \
	DEFAULT_FDT_TI_ARGS \
	DFUARGS \
	NETARGS \
	NANDARGS \
	BOOTENV

/*
 * SPL related defines.  The Public RAM memory map the ROM defines the
 * area between 0x40300000 and 0x4031E000 as a download area for OMAP5.
 * On DRA7xx/AM57XX the download area is between 0x40300000 and 0x4037E000.
 * We set CONFIG_SPL_DISPLAY_PRINT to have omap_rev_string() called and
 * print some information.
 */
#ifdef CONFIG_TI_SECURE_DEVICE
/*
 * For memory booting on HS parts, the first 4KB of the internal RAM is
 * reserved for secure world use and the flash loader image is
 * preceded by a secure certificate. The SPL will therefore run in internal
 * RAM from address 0x40301350 (0x40300000+0x1000(reserved)+0x350(cert)).
 */
#define TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ	0x1000
/* If no specific start address is specified then the secure EMIF
 * region will be placed at the end of the DDR space. In order to prevent
 * the main u-boot relocation from clobbering that memory and causing a
 * firewall violation, we tell u-boot that memory is protected RAM (PRAM)
 */
#if (CONFIG_TI_SECURE_EMIF_REGION_START == 0)
#define CFG_PRAM (CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE) >> 10
#endif
#else
/*
 * For all booting on GP parts, the flash loader image is
 * downloaded into internal RAM at address 0x40300000.
 */
#endif

#endif /* __CONFIG_TI_OMAP5_COMMON_H */
