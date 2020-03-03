/***************************************************************************
 <:copyright-BRCM:2016:DUAL/GPL:standard
 
    Copyright (c) 2016 Broadcom 
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
 ***************************************************************************/

#ifndef __DEV_BCM63XX_EMMC_COMMON_H
#define __DEV_BCM63XX_EMMC_COMMON_H

#include "bcm_hwdefs.h"
#include "emmc_base_defs.h"

/* Full pathnames of EMMC CFE logical partitions e.g "emmcflash0.bootfsx" */
#define EMMC_CFE_PNAME_GPT(img)         (EMMC_DATA_CFE_DEV_STR "." EMMC_PNAME_STR_GPT(img))
#define EMMC_CFE_PNAME_CFE              (EMMC_BOOT1_CFE_DEV_STR "." EMMC_PNAME_STR_CFE)
#define EMMC_CFE_PNAME_CFE_2            (EMMC_BOOT2_CFE_DEV_STR "." EMMC_PNAME_STR_CFE)
#define EMMC_CFE_PNAME_NVRAM            (EMMC_DATA_CFE_DEV_STR "." EMMC_PNAME_STR_NVRAM)
#define EMMC_CFE_PNAME_BOOTFS(img)      (EMMC_DATA_CFE_DEV_STR "." EMMC_PNAME_STR_BOOTFS(img))
#define EMMC_CFE_PNAME_ROOTFS(img)      (EMMC_DATA_CFE_DEV_STR "." EMMC_PNAME_STR_ROOTFS(img))
#define EMMC_CFE_PNAME_MDATA(img, idx)  (EMMC_DATA_CFE_DEV_STR "." EMMC_PNAME_STR_MDATA(img, idx))
#define EMMC_CFE_PNAME_DATA             (EMMC_DATA_CFE_DEV_STR "." EMMC_PNAME_STR_DATA)
#define EMMC_CFE_PNAME_MISC(idx)        (EMMC_DATA_CFE_DEV_STR "." EMMC_PNAME_STR_MISC(idx))

/* Format strings for full pathnames for use with sprintf, all indices replaced by %d */
#define EMMC_CFE_PNAME_FMT_STR_BOOTFS   (EMMC_DATA_CFE_DEV_STR "." XSTR(EMMC_PNAME_BOOTFS) "%d")
#define EMMC_CFE_PNAME_FMT_STR_ROOTFS   (EMMC_DATA_CFE_DEV_STR "." XSTR(EMMC_PNAME_ROOTFS) "%d")
#define EMMC_CFE_PNAME_FMT_STR_MDATA    (EMMC_DATA_CFE_DEV_STR "." XSTR(EMMC_PNAME_MDATA) "%d_%d")
#define EMMC_CFE_PNAME_FMT_STR_MISC     (EMMC_DATA_CFE_DEV_STR "." XSTR(EMMC_PNAME_MISC) "%d")

/* Format strings for linux full pathnames of emmc devices */
#define EMMC_LINUX_DEV_NAME_FMTSTR       "/dev/mmcblk%dp%d"

int parse_emmc_bootfs_common( int fh, uint64_t offset, char * filename, char * dev_buffer, char * data_buffer, int * data_length,
                                     int block_size, int (*read_func)(int, unsigned long long, unsigned char *, int ) );

int load_file_from_next_bootfs( char * fname, char * buf , int * boot_img_id, int * img_idx,
                                       int (*bootfs_parsefunc)( char * , char * , char * , int * ) );

int get_seqnum_from_emmc_mdata(  int image_num, int * sequence_num, int * committed_flag,
                                 int (*bootfs_parsefunc)( char * , char * , char * , int * ) );

int update_inmemory_metadata_data(  char * metadata_ptr, char * filename, uint8_t * data, uint32_t data_len);

#endif /* __DEV_BCM63XX_EMMC_COMMON_H */
