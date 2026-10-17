/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author:	Yanhong Wang<yanhong.wang@starfivetech.com>
 *
 */

#ifndef _STARFIVE_VISIONFIVE2_H
#define _STARFIVE_VISIONFIVE2_H

#define RISCV_MMODE_TIMERBASE		0x2000000
#define RISCV_MMODE_TIMEROFF		0xbff8
#define RISCV_MMODE_TIMER_FREQ		4000000
#define RISCV_SMODE_TIMER_FREQ		4000000

#define __io

#define TYPE_GUID_SPL		"2E54B353-1271-4842-806F-E436D6AF6985"
#define TYPE_GUID_UBOOT	"BC13C2FF-59E6-4262-A352-B275FD6F7172"
#define TYPE_GUID_SYSTEM	"EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"

#define PARTS_DEFAULT							\
		"name=spl,start=2M,size=2M,type=${type_guid_gpt_loader1};" \
		"name=uboot,size=4MB,type=${type_guid_gpt_loader2};"		\
		"name=system,size=-,bootable,type=${type_guid_gpt_system};"

#define CFG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0x40200000\0" \
	"kernel_comp_addr_r=0x88000000\0" \
	"kernel_comp_size=0x4000000\0" \
	"fdt_addr_r=0x46000000\0" \
	"fdtoverlay_addr_r=0x45800000\0" \
	"scriptaddr=0x43900000\0" \
	"pxefile_addr_r=0x45900000\0" \
	"ramdisk_addr_r=0x46100000\0" \
	"type_guid_gpt_loader1=" TYPE_GUID_SPL "\0" \
	"type_guid_gpt_loader2=" TYPE_GUID_UBOOT "\0" \
	"type_guid_gpt_system=" TYPE_GUID_SYSTEM "\0" \
	"partitions=" PARTS_DEFAULT "\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* _STARFIVE_VISIONFIVE2_H */
