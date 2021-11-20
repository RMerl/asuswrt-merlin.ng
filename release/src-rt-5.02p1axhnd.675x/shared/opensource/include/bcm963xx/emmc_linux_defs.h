/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/                       

/***********************************************************************/
/*                                                                     */
/*   MODULE:  emmc_linux_defs.h                                        */
/*   PURPOSE: linux emmc related definition.                           */
/*                                                                     */
/***********************************************************************/
#ifndef _EMMC_LINUX_DEFS_H
#define _EMMC_LINUX_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "emmc_base_defs.h"

#define EMMC_DEV_LINUX_PREFIX            "mmcblk"
#define EMMC_DEV_PHYS_BOOT_PART0_NAME    "mmcblk0boot0"
#define EMMC_DEV_PHYS_BOOT_PART1_NAME    "mmcblk0boot1"

/* Full pathnames of EMMC linux logical partitions e.g "/dev/bootfsx" */
#define EMMC_DEV_PNAME_CFE              ("/dev/" EMMC_DEV_PHYS_BOOT_PART0_NAME)
#define EMMC_DEV_PNAME_NVRAM            ("/dev/" EMMC_PNAME_STR_NVRAM)
#define EMMC_DEV_PNAME_BOOTFS(img)      ("/dev/" EMMC_PNAME_STR_BOOTFS(img))
#define EMMC_DEV_PNAME_ROOTFS(img)      ("/dev/" EMMC_PNAME_STR_ROOTFS(img))
#define EMMC_DEV_PNAME_MDATA(img, idx)  ("/dev/" EMMC_PNAME_STR_MDATA(img, idx))
#define EMMC_DEV_PNAME_DATA             ("/dev/" EMMC_PNAME_STR_DATA)
#define EMMC_DEV_PNAME_MISC(idx)        ("/dev/" EMMC_PNAME_STR_MISC(idx))

#define EMMC_DEV_PNAME_FMT_STR_BOOTFS   ("/dev/" XSTR(EMMC_PNAME_BOOTFS) "%d")
#define EMMC_DEV_PNAME_FMT_STR_ROOTFS   ("/dev/" XSTR(EMMC_PNAME_ROOTFS) "%d")
#define EMMC_DEV_PNAME_FMT_STR_MDATA    ("/dev/" XSTR(EMMC_PNAME_MDATA) "%d_%d")
#define EMMC_DEV_PNAME_FMT_STR_MISC     ("/dev/" XSTR(EMMC_PNAME_MISC) "%d")

/* Need to write a 0 to this path to enable boot partition writes */
#define EMMC_BOOT_PARTITION_UNLOCK_CMD  ("echo 0 > /sys/block/" EMMC_DEV_PHYS_BOOT_PART0_NAME "/force_ro")

#ifdef __cplusplus
}
#endif

#endif /*_EMMC_LINUX_DEFS_H */
