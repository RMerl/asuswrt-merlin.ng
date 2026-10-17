/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Amarula Solutions(India)
 *
 * SF distro configurations.
 */

#ifndef __DISTRO_SF_CONFIG_H
#define __DISTRO_SF_CONFIG_H

#if IS_ENABLED(CONFIG_CMD_SF)
#define BOOTENV_SHARED_SF(devtypel)				\
	#devtypel "_boot="					\
	"if " #devtypel " probe ${busnum}; then "		\
		"devtype=" #devtypel "; "			\
		"run scan_sf_for_scripts; "			\
	"fi\0"
#define BOOTENV_DEV_SF(devtypeu, devtypel, instance)		\
	"bootcmd_" #devtypel #instance "="			\
		"busnum=" #instance "; "			\
		"run " #devtypel "_boot\0"
#define BOOTENV_DEV_NAME_SF(devtypeu, devtypel, instance)	\
	#devtypel #instance " "
#else
#define BOOTENV_SHARED_SF(devtypel)
#define BOOTENV_DEV_SF \
	BOOT_TARGET_DEVICES_references_SF_without_CONFIG_CMD_SF
#define BOOTENV_DEV_NAME_SF \
	BOOT_TARGET_DEVICES_references_SF_without_CONFIG_CMD_SF

#endif /* CONFIG_CMD_SF */

#define BOOTENV_SF \
	BOOTENV_SHARED_SF(sf) \
	"scan_sf_for_scripts="					\
		"${devtype} read ${scriptaddr} "		\
			"${script_offset_f} ${script_size_f}; "	\
		"source ${scriptaddr}; "			\
		"echo SCRIPT FAILED: continuing...\0"

#endif /* __DISTRO_SF_CONFIG_H */
