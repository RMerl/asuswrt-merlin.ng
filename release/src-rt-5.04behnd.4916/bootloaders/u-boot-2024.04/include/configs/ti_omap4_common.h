/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated.
 * Aneesh V       <aneesh@ti.com>
 * Steve Sakoman  <steve@sakoman.com>
 *
 * TI OMAP4 common configuration settings
 */

#ifndef __CONFIG_TI_OMAP4_COMMON_H
#define __CONFIG_TI_OMAP4_COMMON_H

#ifndef CONFIG_SYS_L2CACHE_OFF
#define CFG_SYS_PL310_BASE	0x48242000
#endif

/* Get CPU defs */
#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/* Use General purpose timer 1 */
#define CFG_SYS_TIMERBASE		GPT2_BASE

#include <configs/ti_armv7_omap.h>

/*
 * Hardware drivers
 */
#define CFG_SYS_NS16550_CLK		48000000
#if !CONFIG_IS_ENABLED(DM_SERIAL)
#define CFG_SYS_NS16550_COM3		UART3_BASE
#endif

/*
 * Environment setup
 */
#define BOOTENV_DEV_LEGACY_MMC(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"setenv mmcdev " #instance"; "\
	"setenv bootpart " #instance":2 ; "\
	"run mmcboot\0"

#define BOOTENV_DEV_NAME_LEGACY_MMC(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(LEGACY_MMC, legacy_mmc, 0) \
	func(MMC, mmc, 1) \
	func(LEGACY_MMC, legacy_mmc, 1) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#include <env/ti/mmc.h>

#define CFG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	DEFAULT_MMC_TI_ARGS \
	DEFAULT_FIT_TI_ARGS \
	"console=ttyO2,115200n8\0" \
	"fdtfile=undefined\0" \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M\0" \
	"loaduimage=load mmc ${mmcdev} ${loadaddr} uImage\0" \
	"uimageboot=echo Booting from mmc${mmcdev} ...; " \
		"run args_mmc; " \
		"bootm ${loadaddr}\0" \
	"findfdt="\
		"if test $board_name = sdp4430; then " \
			"setenv fdtfile omap4-sdp.dtb; fi; " \
		"if test $board_name = panda; then " \
			"setenv fdtfile omap4-panda.dtb; fi;" \
		"if test $board_name = panda-a4; then " \
			"setenv fdtfile omap4-panda-a4.dtb; fi;" \
		"if test $board_name = panda-es; then " \
			"setenv fdtfile omap4-panda-es.dtb; fi;" \
		"if test $board_name = duovero; then " \
			"setenv fdtfile omap4-duovero-parlor.dtb; fi;" \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine device tree to use; fi; \0" \
	BOOTENV

/*
 * Defines for SPL
 * It is known that this will break HS devices. Since the current size of
 * SPL is overlapped with public stack and breaking non HS devices to boot.
 * So moving TEXT_BASE down to non-HS limit.
 */

#ifdef CONFIG_SPL_BUILD
/* No need for i2c in SPL mode as we will use SRI2C for PMIC access on OMAP4 */
#endif

#endif /* __CONFIG_TI_OMAP4_COMMON_H */
