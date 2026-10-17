/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef _ROCKCHIP_COMMON_H_
#define _ROCKCHIP_COMMON_H_
#include <linux/sizes.h>

#ifndef CFG_CPUID_OFFSET
#define CFG_CPUID_OFFSET	0x7
#endif

#ifndef CONFIG_SPL_BUILD

#define BOOT_TARGETS	"mmc1 mmc0 nvme scsi usb pxe dhcp spi"

#ifdef CONFIG_ARM64
#define ROOT_UUID "B921B045-1DF0-41C3-AF44-4C6F280D3FAE;\0"
#else
#define ROOT_UUID "69DAD710-2CE4-4E3C-B16C-21A1D49ABED3;\0"
#endif
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=loader1,start=32K,size=4000K,uuid=${uuid_gpt_loader1};" \
	"name=loader2,start=8MB,size=4MB,uuid=${uuid_gpt_loader2};" \
	"name=trust,size=4M,uuid=${uuid_gpt_atf};" \
	"name=boot,size=112M,bootable,uuid=${uuid_gpt_boot};" \
	"name=rootfs,size=-,uuid="ROOT_UUID

#endif

#endif /* _ROCKCHIP_COMMON_H_ */
