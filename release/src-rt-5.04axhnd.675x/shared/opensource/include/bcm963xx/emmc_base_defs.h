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
/*   MODULE:  emmc_base_defs.h                                         */
/*   PURPOSE: Base emmc related definition.                            */
/*                                                                     */
/***********************************************************************/
#ifndef _EMMC_BASE_DEFS_H
#define _EMMC_BASE_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#define STR_CONCAT2(a, b) a##b
#define STR_CONCAT(a, b) STR_CONCAT2(a, b)
#define TO_STR(s) #s
#define XSTR(s) TO_STR(s)

#define EMMC_PNAME_INT(part, img)               STR_CONCAT(EMMC_PNAME_##part, img)
#define EMMC_PNAME(part, img)                   XSTR(EMMC_PNAME_INT(part, img))
#define EMMC_PNAME_IMG_IDX_INT(part, img, idx)  STR_CONCAT(EMMC_PNAME_##part, img##_##idx)
#define EMMC_PNAME_IMG_IDX(part, img, idx)      XSTR(EMMC_PNAME_IMG_IDX_INT(part, img, idx))

/* CFE device indices for EMMC physical partitions */
#define EMMC_DATA_CFE_DEV_IDX   0
#define EMMC_BOOT1_CFE_DEV_IDX  1
#define EMMC_BOOT2_CFE_DEV_IDX  2
#define EMMC_CFE_DEV_PREFIX     emmcflash

/* CFE device prefixes for EMMC physical partitions e.g. "emmcflash0" or "emmcflash1" */
#define EMMC_DATA_CFE_DEV            STR_CONCAT(EMMC_CFE_DEV_PREFIX, EMMC_DATA_CFE_DEV_IDX)
#define EMMC_DATA_CFE_DEV_STR        XSTR(EMMC_DATA_CFE_DEV)
#define EMMC_BOOT1_CFE_DEV           STR_CONCAT(EMMC_CFE_DEV_PREFIX, EMMC_BOOT1_CFE_DEV_IDX)
#define EMMC_BOOT1_CFE_DEV_STR       XSTR(EMMC_BOOT1_CFE_DEV)
#define EMMC_BOOT2_CFE_DEV           STR_CONCAT(EMMC_CFE_DEV_PREFIX, EMMC_BOOT2_CFE_DEV_IDX)
#define EMMC_BOOT2_CFE_DEV_STR       XSTR(EMMC_BOOT2_CFE_DEV)

/* Names of EMMC CFE logical partition */
#define EMMC_PNAME_GPT      gpt
#define EMMC_PNAME_CFE      cfe
#define EMMC_PNAME_NVRAM    nvram
#define EMMC_PNAME_BOOTFS   bootfs
#define EMMC_PNAME_ROOTFS   rootfs
#define EMMC_PNAME_MDATA    mdata
#define EMMC_PNAME_DATA     data
#define EMMC_PNAME_MISCP    misc
#define EMMC_PNAME_UNALLOC  unalloc

/* Strings of EMMC CFE logical part names e.g. "bootfs1", "nvram", "rootfs1", "mdata1_1" */
#define EMMC_PNAME_STR_GPT(img)         EMMC_PNAME(GPT, img)
#define EMMC_PNAME_STR_CFE              XSTR(EMMC_PNAME_CFE)
#define EMMC_PNAME_STR_NVRAM            XSTR(EMMC_PNAME_NVRAM)
#define EMMC_PNAME_STR_BOOTFS(img)      EMMC_PNAME(BOOTFS, img)
#define EMMC_PNAME_STR_ROOTFS(img)      EMMC_PNAME(ROOTFS, img)
#define EMMC_PNAME_STR_MDATA(img, idx)  EMMC_PNAME_IMG_IDX(MDATA, img, idx)
#define EMMC_PNAME_STR_DATA             XSTR(EMMC_PNAME_DATA)
#define EMMC_PNAME_STR_MISC(idx)        EMMC_PNAME(MISCP, idx)
#define EMMC_PNAME_STR_UNALLOC          XSTR(EMMC_PNAME_UNALLOC)

/* Format strings for linux full pathnames of emmc devices */
#define EMMC_LINUX_DEV_NAME_FMTSTR       "/dev/mmcblk%dp%d"

/* Partition, Image number and indices */
#define EMMC_IMG_IDX_START          1   // Img numbers start from 1
#define EMMC_NUM_IMGS               2   // Max 2 images supported
#define EMMC_NUM_MDATA              2   // Max 2 metadata copies supported
#define EMMC_NUM_MISC               4   // Max 4 misc partitions supported
#define EMMC_FLASH_MAX_LOGICAL_PARTS   128 // Max 128 logical partition per physical partitions(GPT based limit)
#define EMMC_MAX_DATA_PARTS         EMMC_FLASH_MAX_LOGICAL_PARTS  
#define EMMC_MAX_PART_NAME          30  // Maximum partition name is 30 length 

/* Default block sizes: Note that eMMC devices can
 * support read/write block sizes of upto 4K, but 
 * by default all devices need to be in 512B access
 * mode. Since neither CFE, nor Linux can change the
 * default block size, we assume that all devices
 * will be in default mode on our systems 
 */
#define EMMC_DFLT_BLOCK_SIZE    512
#define EMMC_DFLT_BLOCK_SHIFT     9   /* 512Byte : 9bit */

/* EMMC Logical Blocks(LB) and Logical Block Addresses (LBA) 
 * -Must ensure that ALIGN_BYTES is greater than the GPT header sizes
 * -Must ensure that any offline image creation tool must align partitions
 *  to EMMC_PART_ALIGN_BYTES */
#define EMMC_LB_SIZE                EMMC_DFLT_BLOCK_SIZE    // EMMC Logical block size
#define EMMC_PART_ALIGN_LB          2048 // EMMC logical partition LBAs will be aligned to 2048 LBs
#define EMMC_PART_ALIGN_BYTES       (EMMC_LB_SIZE * EMMC_PART_ALIGN_LB)

/* Default partition sizes and offsets */
#define EMMC_DFLT_NVRAM_SIZE        (sizeof(NVRAM_DATA) + 1024) & ~(1024 -1)
#define EMMC_DFLT_BOOTFS_SIZE       (100*1024*1024)
#define EMMC_DFLT_ROOTFS_SIZE       (100*1024*1024)
#define EMMC_DFLT_MDATA_SIZE        (32*1024)
#define EMMC_DFLT_DATA_SIZE         (100*1024*1024)

#ifdef __cplusplus
}
#endif

#endif /* _BCM_HWDEFS_H */

