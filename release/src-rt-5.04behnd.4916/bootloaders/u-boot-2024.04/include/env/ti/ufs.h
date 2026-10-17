/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com
 *
 * Environment variable definitions for UFS on TI boards.
 */

#ifndef __TI_UFS_H
#define __TI_UFS_H

#define DEFAULT_UFS_TI_ARGS \
	"scsirootfstype=ext4 rootwait\0" \
	"ufs_finduuid=part uuid scsi ${bootpart} uuid\0" \
	"args_ufs=setenv devtype scsi;setenv bootpart 1:1;" \
	"run ufs_finduuid;setenv bootargs console = ${console} " \
		"${optargs}" \
		"root=PARTUUID=${uuid} rw " \
		"rootfstype=${scsirootfstype};" \
		"setenv devtype scsi;" \
		"setenv bootpart 1:1\0" \
	"init_ufs=ufs init; scsi scan; run args_ufs\0" \
	"get_kern_ufs=load ${devtype} ${bootpart} ${loadaddr} ${bootdir}/${name_kern}\0" \
	"get_fdt_ufs=load ${devtype} ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"get_overlay_ufs=" \
		"fdt address ${fdtaddr};" \
		"fdt resize 0x100000;" \
		"for overlay in $name_overlays;" \
		"do;" \
		"load scsi ${bootpart} ${dtboaddr} ${bootdir}/${overlay} && " \
		"fdt apply ${dtboaddr};" \
		"done;\0"

#endif
