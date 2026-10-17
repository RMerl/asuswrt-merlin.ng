/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (c) 2017 DENX Software Engineering GmbH, Heiko Schocher <hs@denx.de>
 * Copyright (c) 2019 Bosch Thermotechnik GmbH
 * Copyright (c) 2022 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

#ifndef __IMX6Q_ACC_H
#define __IMX6Q_ACC_H

#include <linux/sizes.h>
#include "mx6_common.h"

#ifdef CONFIG_SYS_BOOT_EMMC
#define MMC_ROOTFS_DEV 0
#define MMC_ROOTFS_PART 2
#endif

#ifdef CONFIG_SYS_BOOT_EMMC
/* eMMC Boot */
#define ENV_EXTRA \
	"mmcdev=" __stringify(MMC_ROOTFS_DEV) "\0" \
	"mmcpart=" __stringify(MMC_ROOTFS_PART) "\0" \
	"fitpart=1\0" \
	"optargs=ro quiet systemd.gpt_auto=false\0" \
	"production=1\0" \
	"mmcautodetect=yes\0" \
	"mmcrootfstype=ext4\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"mmcargs=run finduuid; setenv bootargs " \
		"root=PARTUUID=${uuid} ${optargs} rootfstype=${mmcrootfstype}\0" \
	"mmc_mmc_fit=run env_persist; run setbm; run mmcloadfit; " \
		"run auth_fit_or_reset; run mmcargs addcon; " \
		"bootm ${fit_addr}#${bootconf}\0" \
	"bootset=0\0" \
	"setbm=if test ${bootset} -eq 1; " \
		"then setenv mmcpart 4; setenv fitpart 3; " \
		"else; setenv mmcpart 2; setenv fitpart 1; fi\0" \
	"handle_ustate=if test ${ustate} -eq 2; then setenv ustate 3; fi\0" \
	"switch_bootset=if test ${bootset} -eq 1; then setenv bootset 0; " \
		"else; setenv bootset 1;fi\0" \
	"env_persisted=0\0" \
	"env_persist=if test ${env_persisted} != 1; " \
		"then env set env_persisted 1; run save_env; fi;\0" \
	"save_env=env save; env save\0" \
	"altbootcmd=run handle_ustate; run switch_bootset; run save_env; run bootcmd\0"

#define CFG_ENV_FLAGS_LIST_STATIC \
	"bootset:bw," \
	"clone_pending:bw," \
	"endurance_test:bw," \
	"env_persisted:bw," \
	"factory_reset:bw," \
	"fdtcontroladdr:xw," \
	"fitpart:dw," \
	"mmcpart:dw," \
	"production:bw," \
	"ustate:dw"

#else
/* SD Card boot */
#define ENV_EXTRA \
	"mmcdev=1\0" \
	"fitpart=1\0" \
	"rootpart=2\0" \
	"optargs=ro systemd.gpt_auto=false\0" \
	"finduuid=part uuid mmc ${mmcdev}:${rootpart} uuid\0" \
	"mmcargs=run finduuid;setenv bootargs root=PARTUUID=${uuid} ${optargs}\0" \
	"mmc_mmc_fit=run mmcloadfit; run auth_fit_or_reset; run mmcargs addcon; " \
		"bootm ${fit_addr}#${bootconf}\0"

#endif

/* Default environment */
#define CFG_EXTRA_ENV_SETTINGS \
	"bootconf=conf-imx6q-bosch-acc.dtb\0"\
	"mmcfit_name=fitImage\0" \
	"mmcloadfit=ext4load mmc ${mmcdev}:${fitpart} ${fit_addr} ${mmcfit_name}\0" \
	"auth_fit_or_reset=hab_auth_img ${fit_addr} ${filesize} || reset\0" \
	"console=ttymxc0\0" \
	"addcon=setenv bootargs ${bootargs} console=${console},${baudrate}\0" \
	"fit_addr=19000000\0" \
	ENV_EXTRA

/* Physical Memory Map */
#define PHYS_SDRAM                      MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE           PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR        IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE        IRAM_SIZE

/* SPL */
#ifdef CONFIG_SPL

#ifdef CONFIG_SPL_BUILD
#define CFG_SYS_FSL_USDHC_NUM 2

#ifdef CONFIG_SYS_BOOT_EMMC

/* Boot from eMMC */
#define CFG_SYS_FSL_ESDHC_ADDR 1

#else

/* Boot from SD-card */
#  define CFG_SYS_FSL_ESDHC_ADDR	0

#endif

#endif
#endif

#define CFG_MXC_USB_PORTSC            (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS             0

#endif /* __IMX6Q_ACC_H */
