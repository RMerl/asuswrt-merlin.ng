/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
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

/*!\file bcm_flashutil.h
 * \brief Public header file for the bcm_flashutil library.
 */

#include <stdint.h>

#define FLASH_INFO_FLAG_NOR    0x0001
#define FLASH_INFO_FLAG_NAND   0x0002
#define FLASH_INFO_FLAG_EMMC   0x0003

#define PSI_FILE_NAME           "/data/psi"
#define PSI_BACKUP_FILE_NAME    "/data/psibackup"
#define SCRATCH_PAD_FILE_NAME   "/data/scratchpad"
#define SYSLOG_FILE_NAME        "/data/syslog"
#define IMAGE_VERSION_FILE_NAME "image_version"


#define MAX_MTD_NAME_SIZE         64

/** Get info about the flash.  Currently, just returns the type of flash,
 *  but in the future, could return more useful info.
 *
 *  @flags (OUT)  Bit field containing info about the flash type.
 *
 *  @return CmsRet enum.
 */
int getFlashInfo(unsigned int *flags);


/** Get the sequence number of the image
 *
 * @param image (IN)   image number to get the sequence number from (1 or 2)
 *
 * @return int         sequence number
 */
int devCtl_getSequenceNumber(int image);


int getSequenceNumber(int imageNumber);
int getNextSequenceNumber(int seqNumImg1, int seqNumImg2);

/** Set the boot image state.
 * @param state (IN)   BOOT_SET_NEW_IMAGE, BOOT_SET_OLD_IMAGE,
 *                     BOOT_SET_NEW_IMAGE_ONCE,
 *                     BOOT_SET_PART1_IMAGE, BOOT_SET_PART2_IMAGE,
 *                     BOOT_SET_PART1_IMAGE_ONCE, BOOT_SET_PART2_IMAGE_ONCE
 *
 * @return BcmRet enum.
 */
int devCtl_setImageState(int state);
int setBootImageState(int newState);

/** Get the boot image state.
 *
 * @return             BOOT_SET_PART1_IMAGE, BOOT_SET_PART2_IMAGE,
 *                     BOOT_SET_PART1_IMAGE_ONCE, BOOT_SET_PART2_IMAGE_ONCE
 *
 */
int devCtl_getImageState(void);
int getBootImageState(void);

/** Get the booted image partition.
 *
 * @return             BOOTED_PART1_IMAGE, BOOTED_PART2_IMAGE
 */
int devCtl_getBootedImagePartition(void);
int getBootedValue(void);


int readNvramData(void *pNvramData);
unsigned int otp_is_btrm_boot(void);
unsigned int otp_is_boot_secure(void);
unsigned int otp_is_boot_mfg_secure(void);
unsigned int get_chip_id(void);
unsigned int get_flash_type(void);


/** Get the booted image id.
 *
 * @return             BOOTED_NEW_IMAGE, BOOTED_OLD_IMAGE
 */
int devCtl_getBootedImageId(void);


/** Get image version string.
 *
 * @return number of bytes copied into verStr
 */
int devCtl_getImageVersion(int partition, char *verStr, int verStrSize);
int getImageVersion(uint8_t *imagePtr, int imageSize, char *image_name,
                    int image_name_len);


int getBootPartition( void );
int commit( int partition, char *string );
int writeImageToNand(unsigned char *string, int size);


#endif /* _BCM_FLASHUTIL_H_ */
