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

#include "bcm_flashutil_nand.h"

#define FLASH_INFO_FLAG_NOR    0x0001
#define FLASH_INFO_FLAG_NAND   0x0002
#define FLASH_INFO_FLAG_EMMC   0x0003

#define PSI_FILE_NAME           "/data/psi"
#define PSI_BACKUP_FILE_NAME    "/data/psibackup"
#define SCRATCH_PAD_FILE_NAME   "/data/scratchpad"
#define SYSLOG_FILE_NAME        "/data/syslog"


#define IDENT_TAG               "@(#) $imageversion: "

#define MAX_MTD_NAME_SIZE         64

/** Get info about the flash.  Currently, just returns the type of flash,
 *  but in the future, could return more useful info.
 *
 *  @flags (OUT)  Bit field containing info about the flash type.
 *
 *  @return CmsRet enum.
 */

int getFlashInfo(unsigned int *flags);
int getSequenceNumber(int imageNumber);
int getNextSequenceNumber(int seqNumImg1, int seqNumImg2);
int setBootImageState(int newState);
int getBootImageState(void);
int getBootedValue(void);
int readNvramData(void *pNvramData);
unsigned int otp_is_btrm_boot(void);
unsigned int otp_is_boot_secure(void);
unsigned int otp_is_boot_mfg_secure(void);
unsigned int get_chip_id(void);
unsigned int get_flash_type(void);
int getImageVersion(uint8_t *imagePtr, int imageSize, char *image_name,
  int image_name_len);
int getBootPartition( void );
int commit( int partition, char *string );
#endif /* _BCM_FLASHUTIL_H_ */
