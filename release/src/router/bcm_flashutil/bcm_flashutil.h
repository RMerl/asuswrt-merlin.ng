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


#ifndef _BCM_FLASHUTIL_H__
#define _BCM_FLASHUTIL_H_

#include <mtd/mtd-user.h>

#define FLASH_INFO_FLAG_NOR    0x0001
#define FLASH_INFO_FLAG_NAND   0x0002

#define IMAGE_VERSION_FILE_NAME "image_version"

/** Get info about the flash.  Currently, just returns the type of flash,
 *  but in the future, could return more useful info.
 *
 *  @flags (OUT)  Bit field containing info about the flash type.
 *
 *  @return CmsRet enum.
 */

int writeImageToNand(unsigned char *string, int size);
int getFlashInfo(unsigned int *flags);
unsigned char *nandUpdateSeqNum(unsigned char *imagePtr, int imageSize, int blkLen, int seq, int *found);
int get_image_version(uint8_t *imagePtr, int imageSize, int erasesize, char *image_name, int image_name_len);
int getSequenceNumber(int imageNumber);
int setBootImageState(int newState);
int getBootImageState(void);
int image_type(unsigned char * buf);
int partitionBooted(int partition);

/* Incremental flashing. */

mtd_info_t *get_mtd_device_nm(const char *check, int *mtd_fd);
void put_mtd_device(mtd_info_t *mtd, int mtd_fd);
int nandEraseBlk(mtd_info_t *mtd, int blk_addr, int mtd_fd);
int nandWriteBlk(mtd_info_t *mtd, int blk_addr, int data_len, unsigned char *data_ptr,
  int mtd_fd, int write_JFFS2_clean_marker);
char *getCferamName(void);
int getSequenceNumber(int imageNumber);
int flashCferam(mtd_info_t *mtd, int mtd_fd, int rsrvd_for_cferam,
  unsigned char *cferam_ptr, unsigned char *cferam_ptr2);
int readNvramData(void *pNvramData);
int handleCferom(mtd_info_t *mtd0, char *image_ptr, unsigned int wfiFlags,
  void *inMemNvramData_buf);
int flashCferom(unsigned char *image_ptr, int size);
int validateWfiTag(void *wt, int blksize, uint32_t btrmEnabled);

unsigned int otp_is_btrm_boot(void);
unsigned int otp_is_boot_secure(void);

int getImageVersion(uint8_t *imagePtr, int imageSize, char *image_name,
  int image_name_len);

#endif /* _BCM_FLASHUTIL_H_ */
