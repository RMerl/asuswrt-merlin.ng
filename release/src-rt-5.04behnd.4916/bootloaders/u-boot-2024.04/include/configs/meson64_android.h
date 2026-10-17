/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Android Amlogic Meson 64bits SoCs
 *
 * Copyright (C) 2019 Baylibre, SAS
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#ifndef __MESON64_ANDROID_CONFIG_H
#define __MESON64_ANDROID_CONFIG_H

#include <linux/sizes.h>

#ifndef BOOT_PARTITION
#define BOOT_PARTITION "boot"
#endif

#ifndef LOGO_PARTITION
#define LOGO_PARTITION "logo"
#endif

#ifndef CONTROL_PARTITION
#define CONTROL_PARTITION "misc"
#endif

#ifndef EXTRA_ANDROID_ENV_SETTINGS
#define EXTRA_ANDROID_ENV_SETTINGS ""
#endif

#if defined(CONFIG_CMD_AVB)
#define AVB_VERIFY_CHECK \
	"if test \"${force_avb}\" -eq 1; then " \
		"if run avb_verify; then " \
			"echo AVB verification OK.;" \
			"setenv bootargs \"$bootargs $avb_bootargs\";" \
		"else " \
			"echo AVB verification failed.;" \
		"exit; fi;" \
	"else " \
		"setenv bootargs \"$bootargs androidboot.verifiedbootstate=orange\";" \
		"echo Running without AVB...; "\
	"fi;"

#define AVB_VERIFY_CMD "avb_verify=avb init ${mmcdev}; avb verify $slot_suffix;\0"
#else
#define AVB_VERIFY_CHECK ""
#define AVB_VERIFY_CMD ""
#endif

#if defined(CONFIG_CMD_AB_SELECT)
#define ANDROIDBOOT_GET_CURRENT_SLOT_CMD "get_current_slot=" \
	"if part number mmc ${mmcdev} " CONTROL_PARTITION " control_part_number; " \
	"then " \
		"echo " CONTROL_PARTITION \
			" partition number:${control_part_number};" \
		"ab_select current_slot mmc ${mmcdev}:${control_part_number};" \
	"else " \
		"echo " CONTROL_PARTITION " partition not found;" \
	"fi;\0"

#define AB_SELECT_SLOT \
	"run get_current_slot; " \
	"if test -e \"${current_slot}\"; " \
	"then " \
		"setenv slot_suffix _${current_slot}; " \
	"else " \
		"echo current_slot not found;" \
		"exit;" \
	"fi;"

#define AB_SELECT_ARGS \
	"setenv bootargs_ab androidboot.slot_suffix=${slot_suffix}; " \
	"echo A/B cmdline addition: ${bootargs_ab};" \
	"setenv bootargs ${bootargs} ${bootargs_ab};"

#define AB_BOOTARGS " androidboot.force_normal_boot=1"
#define RECOVERY_PARTITION "boot"
#else
#define AB_SELECT_SLOT ""
#define AB_SELECT_ARGS " "
#define ANDROIDBOOT_GET_CURRENT_SLOT_CMD ""
#define AB_BOOTARGS " "
#define RECOVERY_PARTITION "recovery"
#endif

#if defined(CONFIG_CMD_ABOOTIMG)
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
	"if test $board_name = sei510; then " \
		"echo \"  Reading DTB for sei510...\"; " \
		"setenv dtb_index 0;" \
	"elif test $board_name = sei610; then " \
		"echo \"  Reading DTB for sei610...\"; " \
		"setenv dtb_index 1;" \
	"elif test $board_name = vim3l; then " \
		"echo \"  Reading DTB for vim3l...\"; " \
		"setenv dtb_index 2;" \
	"elif test $board_name = vim3; then " \
		"echo \"  Reading DTB for vim3...\"; " \
		"setenv dtb_index 3;" \
	"else " \
		"echo Error: Android boot is not supported for $board_name; " \
		"exit; " \
	"fi; " \
	"abootimg get dtb --index=$dtb_index dtb_start dtb_size; " \
	"cp.b $dtb_start $fdt_addr_r $dtb_size; " \
	"fdt addr $fdt_addr_r  0x80000; " \
	"if test $board_name = sei510; then " \
		"echo \"  Reading DTBO for sei510...\"; " \
		"setenv dtbo_index 0;" \
	"elif test $board_name = sei610; then " \
		"echo \"  Reading DTBO for sei610...\"; " \
		"setenv dtbo_index 1;" \
	"elif test $board_name = vim3l; then " \
		"echo \"  Reading DTBO for vim3l...\"; " \
		"setenv dtbo_index 2;" \
	"elif test $board_name = vim3; then " \
		"echo \"  Reading DTBO for vim3...\"; " \
		"setenv dtbo_index 3;" \
	"else " \
		"echo Error: Android boot is not supported for $board_name; " \
		"exit; " \
	"fi; " \
	"part start mmc ${mmcdev} dtbo${slot_suffix} p_dtbo_start; " \
	"part size mmc ${mmcdev} dtbo${slot_suffix} p_dtbo_size; " \
	"mmc read ${dtboaddr} ${p_dtbo_start} ${p_dtbo_size}; " \
	"echo \"  Applying DTBOs...\"; " \
	"adtimg addr $dtboaddr; " \
	"adtimg get dt --index=$dtbo_index dtbo0_addr; " \
	"fdt apply $dtbo0_addr;" \
	"setenv bootargs \"$bootargs androidboot.dtbo_idx=$dtbo_index \";"\

#define BOOT_CMD "bootm ${loadaddr} ${loadaddr} ${fdt_addr_r};"

#else
#define PREPARE_FDT " "
#define BOOT_CMD "bootm ${loadaddr};"
#endif

#define BOOTENV_DEV_FASTBOOT(devtypeu, devtypel, instance) \
	"bootcmd_fastboot=" \
		"setenv run_fastboot 0;" \
		"if test \"${boot_source}\" = \"usb\"; then " \
			"echo Fastboot forced by usb rom boot;" \
			"setenv run_fastboot 1;" \
		"fi;" \
		"if test \"${run_fastboot}\" -eq 0; then " \
			"if gpt verify mmc ${mmcdev} ${partitions}; then; " \
			"else " \
				"echo Broken MMC partition scheme;" \
				"setenv run_fastboot 1;" \
			"fi; " \
		"fi;" \
		"if test \"${run_fastboot}\" -eq 0; then " \
			"if bcb load " __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) " " \
			CONTROL_PARTITION "; then " \
				"if bcb test command = bootonce-bootloader; then " \
					"echo BCB: Bootloader boot...; " \
					"bcb clear command; bcb store; " \
					"setenv run_fastboot 1;" \
				"elif bcb test command = boot-fastboot; then " \
					"echo BCB: fastboot userspace boot...; " \
					"setenv force_recovery 1;" \
				"fi; " \
			"else " \
				"echo Warning: BCB is corrupted or does not exist; " \
			"fi;" \
		"fi;" \
		"if test \"${run_fastboot}\" -eq 1; then " \
			"echo Running Fastboot...;" \
			"fastboot " __stringify(CONFIG_FASTBOOT_USB_DEV) "; " \
		"fi\0"

#define BOOTENV_DEV_NAME_FASTBOOT(devtypeu, devtypel, instance)	\
		"fastboot "

#define BOOTENV_DEV_RECOVERY(devtypeu, devtypel, instance) \
	"bootcmd_recovery=" \
		"pinmux dev pinctrl@14;" \
		"pinmux dev pinctrl@40;" \
		"setenv run_recovery 0;" \
		"if run check_button; then " \
			"echo Recovery button is pressed;" \
			"setenv run_recovery 1;" \
		"fi; " \
		"if bcb load " __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) " " \
		CONTROL_PARTITION "; then " \
			"if bcb test command = boot-recovery; then " \
				"echo BCB: Recovery boot...; " \
				"setenv run_recovery 1;" \
			"fi;" \
		"else " \
			"echo Warning: BCB is corrupted or does not exist; " \
		"fi;" \
		"if test \"${skip_recovery}\" -eq 1; then " \
			"echo Recovery skipped by environment;" \
			"setenv run_recovery 0;" \
		"fi;" \
		"if test \"${force_recovery}\" -eq 1; then " \
			"echo Recovery forced by environment;" \
			"setenv run_recovery 1;" \
		"fi;" \
		"if test \"${run_recovery}\" -eq 1; then " \
			"echo Running Recovery...;" \
			"mmc dev ${mmcdev};" \
			"setenv bootargs \"${bootargs} androidboot.serialno=${serial#}\";" \
			AB_SELECT_SLOT \
			AB_SELECT_ARGS \
			AVB_VERIFY_CHECK \
			"part start mmc ${mmcdev} " RECOVERY_PARTITION "${slot_suffix} boot_start;" \
			"part size mmc ${mmcdev} " RECOVERY_PARTITION "${slot_suffix} boot_size;" \
			"if mmc read ${loadaddr} ${boot_start} ${boot_size}; then " \
				PREPARE_FDT \
				"echo Running Android Recovery...;" \
				BOOT_CMD \
			"fi;" \
			"echo Failed to boot Android...;" \
			"reset;" \
		"fi\0"

#define BOOTENV_DEV_NAME_RECOVERY(devtypeu, devtypel, instance)	\
		"recovery "

#define BOOTENV_DEV_SYSTEM(devtypeu, devtypel, instance) \
	"bootcmd_system=" \
		"echo Loading Android " BOOT_PARTITION " partition...;" \
		"mmc dev ${mmcdev};" \
		"setenv bootargs ${bootargs} androidboot.serialno=${serial#};" \
		AB_SELECT_SLOT \
		AB_SELECT_ARGS \
		AVB_VERIFY_CHECK \
		"part start mmc ${mmcdev} " BOOT_PARTITION "${slot_suffix} boot_start;" \
		"part size mmc ${mmcdev} " BOOT_PARTITION "${slot_suffix} boot_size;" \
		"if mmc read ${loadaddr} ${boot_start} ${boot_size}; then " \
			PREPARE_FDT \
			"setenv bootargs \"${bootargs} " AB_BOOTARGS "\"  ; " \
			"echo Running Android...;" \
			BOOT_CMD \
		"fi;" \
		"echo Failed to boot Android...;\0"

#define BOOTENV_DEV_NAME_SYSTEM(devtypeu, devtypel, instance)	\
		"system "

#define BOOTENV_DEV_PANIC(devtypeu, devtypel, instance) \
	"bootcmd_panic=" \
		"fastboot " __stringify(CONFIG_FASTBOOT_USB_DEV) "; " \
		"reset\0"

#define BOOTENV_DEV_NAME_PANIC(devtypeu, devtypel, instance)	\
		"panic "

#define BOOT_TARGET_DEVICES(func) \
	func(FASTBOOT, fastboot, na) \
	func(RECOVERY, recovery, na) \
	func(SYSTEM, system, na) \
	func(PANIC, panic, na) \

#define PREBOOT_LOAD_LOGO \
	"if test \"${boot_source}\" != \"usb\" && " \
		"gpt verify mmc ${mmcdev} ${partitions}; then; " \
		"mmc dev ${mmcdev};" \
		"part start mmc ${mmcdev} " LOGO_PARTITION " boot_start;" \
		"part size mmc ${mmcdev} " LOGO_PARTITION " boot_size;" \
		"if mmc read ${loadaddr} ${boot_start} ${boot_size}; then " \
			"bmp display ${loadaddr} m m;" \
		"fi;" \
	"fi;"

#define CFG_EXTRA_ENV_SETTINGS                                     \
	EXTRA_ANDROID_ENV_SETTINGS                                    \
	"partitions=" PARTS_DEFAULT "\0"                              \
	"mmcdev=2\0"                                                  \
	"fastboot_raw_partition_bootloader=0x1 0xfff mmcpart 1\0"     \
	"fastboot_raw_partition_bootenv=0x0 0xfff mmcpart 2\0"        \
	ANDROIDBOOT_GET_CURRENT_SLOT_CMD                              \
	AVB_VERIFY_CMD                                                \
	"force_avb=0\0"                                               \
	"gpio_recovery=88\0"                                          \
	"check_button=gpio input ${gpio_recovery};test $? -eq 0;\0"   \
	"load_logo=" PREBOOT_LOAD_LOGO "\0"			      \
	"stdin=" STDIN_CFG "\0"                                       \
	"stdout=" STDOUT_CFG "\0"                                     \
	"stderr=" STDOUT_CFG "\0"                                     \
	"dtboaddr=0x08200000\0"                                       \
	"loadaddr=0x01080000\0"                                       \
	"fdt_addr_r=0x01000000\0"                                     \
	"scriptaddr=0x08000000\0"                                     \
	"kernel_addr_r=0x01080000\0"                                  \
	"pxefile_addr_r=0x01080000\0"                                 \
	"ramdisk_addr_r=0x13000000\0"                                 \
	"fdtfile=amlogic/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0"        \
	BOOTENV

#include <configs/meson64.h>

#endif /* __MESON64_ANDROID_CONFIG_H */
