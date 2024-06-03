/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Boot related environment variable definitions on TI boards.
 *
 * (C) Copyright 2017 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#ifndef __TI_BOOT_H
#define __TI_BOOT_H

#ifndef CONSOLEDEV
#define CONSOLEDEV "ttyO2"
#endif

#define VBMETA_PART_SIZE		(64 * 1024)

#if defined(CONFIG_LIBAVB)
#define VBMETA_PART \
	"name=vbmeta,size=" __stringify(VBMETA_PART_SIZE) \
	",uuid=${uuid_gpt_vbmeta};"
#else
#define VBMETA_PART			""
#endif

#ifndef PARTS_DEFAULT
/* Define the default GPT table for eMMC */
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
	"name=recovery,size=40M,uuid=${uuid_gpt_recovery};" \
	"name=boot,size=10M,uuid=${uuid_gpt_boot};" \
	"name=system,size=1024M,uuid=${uuid_gpt_system};" \
	"name=vendor,size=256M,uuid=${uuid_gpt_vendor};" \
	VBMETA_PART \
	"name=userdata,size=-,uuid=${uuid_gpt_userdata}"
#endif /* PARTS_DEFAULT */

#if defined(CONFIG_CMD_AVB)
#define AVB_VERIFY_CHECK "if run avb_verify; then " \
				"echo AVB verification OK.;" \
				"set bootargs $bootargs $avb_bootargs;" \
			"else " \
				"echo AVB verification failed.;" \
			"exit; fi;"
#define AVB_VERIFY_CMD "avb_verify=avb init 1; avb verify;\0"
#else
#define AVB_VERIFY_CHECK ""
#define AVB_VERIFY_CMD ""
#endif

#define DEFAULT_COMMON_BOOT_TI_ARGS \
	"console=" CONSOLEDEV ",115200n8\0" \
	"fdtfile=undefined\0" \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M\0" \
	AVB_VERIFY_CMD \
	"partitions=" PARTS_DEFAULT "\0" \
	"optargs=\0" \
	"dofastboot=0\0" \
	"emmc_linux_boot=" \
		"echo Trying to boot Linux from eMMC ...; " \
		"setenv mmcdev 1; " \
		"setenv bootpart 1:2; " \
		"setenv mmcroot /dev/mmcblk0p2 rw; " \
		"run mmcboot;\0" \
	"emmc_android_boot=" \
		"echo Trying to boot Android from eMMC ...; " \
		"run update_to_fit; " \
		"setenv eval_bootargs setenv bootargs $bootargs; " \
		"run eval_bootargs; " \
		"setenv mmcdev 1; " \
		"setenv machid fe6; " \
		"mmc dev $mmcdev; " \
		"mmc rescan; " \
		AVB_VERIFY_CHECK \
		"part start mmc ${mmcdev} boot boot_start; " \
		"part size mmc ${mmcdev} boot boot_size; " \
		"mmc read ${loadaddr} ${boot_start} ${boot_size}; " \
		"bootm ${loadaddr}#${fdtfile};\0 "

#ifdef CONFIG_OMAP54XX

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

#define CONFIG_BOOTCOMMAND \
	"if test ${dofastboot} -eq 1; then " \
		"echo Boot fastboot requested, resetting dofastboot ...;" \
		"setenv dofastboot 0; saveenv;" \
		"echo Booting into fastboot ...; " \
		"fastboot " __stringify(CONFIG_FASTBOOT_USB_DEV) "; " \
	"fi;" \
	"if test ${boot_fit} -eq 1; then "	\
		"run update_to_fit;"	\
	"fi;"	\
	"run findfdt; " \
	"run envboot; " \
	"run mmcboot;" \
	"run emmc_linux_boot; " \
	"run emmc_android_boot; " \
	""

#endif /* CONFIG_OMAP54XX */

#endif /* __TI_BOOT_H */
