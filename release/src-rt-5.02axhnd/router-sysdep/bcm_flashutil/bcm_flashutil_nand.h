/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 ************************************************************************/


#ifndef _BCM_FLASHUTIL_NAND_H__
#define _BCM_FLASHUTIL_NAND_H_

#include <stdint.h>
#include <mtd/mtd-user.h>

#include "bcm_hwdefs.h"

#define IMAGE_VERSION_FILE_NAME "image_version"

/** Get info about the flash.  Currently, just returns the type of flash,
 *  but in the future, could return more useful info.
 *
 *  @flags (OUT)  Bit field containing info about the flash type.
 *
 *  @return CmsRet enum.
 */

/* Used by bcm_imgif, bcm_imgutil, image.c*/
int writeImageToNand(unsigned char *string, int size);
unsigned char *nandUpdateSeqNum(unsigned char *imagePtr, int imageSize, int blkLen, int seq, int *found);
int nand_image_type(unsigned char * buf);
int partitionBooted(int partition);
mtd_info_t *get_mtd_device_handle(const char *check, int *mtd_fd, int *mtdblock_fd);
void put_mtd_device(mtd_info_t *mtd, int mtd_fd, int mtdblock_fd);
int nandEraseBlk(mtd_info_t *mtd, int blk_addr, int mtd_fd);
int nandWriteBlk(mtd_info_t *mtd, int blk_addr, int data_len, unsigned char *data_ptr,
  int mtd_fd, int write_JFFS2_clean_marker);
int flashCferam(mtd_info_t *mtd, int mtd_fd, int rsrvd_for_cferam,
  unsigned char *cferam_ptr, unsigned char *cferam_ptr2);
int handleCferom(mtd_info_t *mtd0, char *image_ptr, unsigned int image_size, unsigned int wfiFlags,
  void *inMemNvramData_buf);
int flashCferom(unsigned char *image_ptr, unsigned int size);
int validateWfiTag(void *wt, int blksize, uint32_t btrmEnabled);

unsigned int nvramDataOffset(const mtd_info_t * mtd __attribute__((unused)));

/* Used by bcm_flashutil.c */
int nandReadNvramData(void *nvramData);
int nandGetSequenceNumber(int imageNumber);
int nandGetImageVersion(uint8_t *imagePtr, int imageSize, char *image_name, int image_name_len);
int nandWriteBootImageState( int newState );
int nandReadBootImageState( void );
int nandUpdateSequenceNumber(int incSeqNumPart, int seqPart1, int seqPart2);
int nandIsBootDevice(void);
int nandGetBootedValue(void);
int nandGetBootPartition(void);
int nandCommit( int partition, char *commit_flag );

/* used by housekeeping task that keeps nvram mirror and cferom integrity */
int is_cferom_offset(unsigned char *buffer, int offset, int *img_size, int *cferom_crc, unsigned char **);
int is_nvram_offset(unsigned char *buffer, int offset, int nvram_sign_check, int *img_size, PNVRAM_DATA);
void create_boot_block_info_file(char *hunt_ptr, int size_to_search, PNVRAM_DATA pnd, mtd_info_t *mtd);
#define RSA_S_MODULUS       2048                 /* Bits in a key modulus     */
#define RSA_S_MODULUS8      (RSA_S_MODULUS/8)    /* Bytes in a key modulus    */
#define BTRM_SBI_UNAUTH_MGC_NUM_1            183954
#define BTRM_SBI_UNAUTH_MGC_NUM_2            145257


#endif /* _BCM_FLASHUTIL_NAND_H_ */
