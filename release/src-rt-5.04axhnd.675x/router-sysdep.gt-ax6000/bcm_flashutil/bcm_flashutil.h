/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
#define FLASH_INFO_FLAG_EMMC   0x0004

#define PSI_FILE_NAME           "/data/psi"
#define PSI_BACKUP_FILE_NAME    "/data/psibackup"
#define SCRATCH_PAD_FILE_NAME   "/data/scratchpad"
#define SYSLOG_FILE_NAME        "/data/syslog"
#define IMAGE_VERSION_FILE_NAME "image_version"

#define MAX_MTD_NAME_SIZE         64
#define MAX_PSI_FILE_NAME_LEN     64


/***********************************************************************************
 *                         REBOOT REASON and STATUS                                *
 ***********************************************************************************/

#define BCM_BOOT_REASON_REBOOT         (0x00000000)
#define BCM_BOOT_REASON_ACTIVATE       (0x00000001)
#define BCM_BOOT_REASON_PANIC          (0x00000002)
#define BCM_BOOT_REASON_WATCHDOG       (0x00000004)

#define BCM_BOOT_PHASE_MASK            (0x000000F0)
#define BCM_BOOT_PHASE_UBOOT           (0x00000010)
#define BCM_BOOT_PHASE_LINUX_START     (0x00000020)
#define BCM_BOOT_PHASE_LINUX_RUN       (0x00000030)

#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000



#define UBOOT_ENV_MAGIC 0x75456e76
#define PROC_BOOT_MAGIC  "/proc/environment/env_boot_magic"
#define BOOT_MAGIC_OFFS  4096
#define PROC_ENV_RAW     "/proc/environment/raw"

typedef struct
{
    uint32_t magic;
    uint32_t size;
    uint32_t crc;
} UBOOT_ENV_HDR;

/***********************************************************************************
 *                         NEW FLASH LAYOUT METADATA STRUCTS                       *
 ***********************************************************************************/
#define PKGTB_METADATA_SIZE 256                                 //FIXME: Must match uboot value
#define PKGTB_METADATA_RDWR_SIZE (PKGTB_METADATA_SIZE + 1024 )  //FIXME: Must match uboot value
#define PKGTB_METADATA_MAX_SIZE 4096 

typedef struct
{
    unsigned int crc;
    char data[1024];
} __attribute__((__packed__)) env_t;

typedef struct
{
    unsigned int word0;
    unsigned int size;
    env_t mdata_obj;
} __attribute__((__packed__)) MDATA;


/** Get info about the flash.  Currently, just returns the type of flash,
 *  but in the future, could return more useful info.  Note the results are
 *  cached, so that the second and subsequent calls just return the cached
 *  result.
 *
 *  @flags (OUT)  Bit field containing info about the flash type.
 *
 *  @return CmsRet enum.
 */
int getFlashInfo(unsigned int *flags);


/** Non-cached version of the function.  This is more expensive.  Most
 *  callers should just call getFlashInfo().
 */
int getFlashInfoUncached(unsigned int *flags);


/** Get total size of the flash
 *
 *  @size (OUT)  kilobyte(KB)
 *
 *  @return BcmRet enum
 */
int getFlashTotalSize(unsigned int *size);

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


/** Return the number of bytes available in the flash for storing an image.
 *
 * @return number of bytes.
*/
unsigned int bcmImg_getImageFlashSize(void);

unsigned int bcmImg_getBroadcomImageTagSize(void);

unsigned char bcmImg_willFitInFlash(unsigned int imageSize);


/** Use some heuristics to determine if this is a config file (and not an
 *  image file).
 *
 *@ return 1 if likely to be config file, else 0.
 */
unsigned char bcmImg_isConfigFileLikely(const char *buf);


/** The following config file functions are for backward compatibility only.
 *  In reality, on NAND systems, these are just files, so there is no need
 *  to call these functions.
 */
unsigned int bcmImg_getConfigFlashSize(void);
unsigned int bcmImg_getRealConfigFlashSize(void);
unsigned char bcmImg_isBackupConfigFlashAvailable(void);


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


/** Recommended minimum size for getImageVersion.
 *
 *  A imageversion string looks like this:  $imageversion: 5041test3G0861904 $
 */
#define IMAGE_VERSION_TAG_SIZE          64

/** Get image version string ("imageversion").
 *
 *  This function has support for legacy NAND flash and also some processing
 *  of the returned string.
 *  Recommend either using the higher layer cmsImg_ functions or the low level
 *  bcmFlash_getIdent() function below.
 *
 * @return number of bytes copied into verStr
 */
int devCtl_getImageVersion(int partition, char *verStr, int verStrSize);
int getImageVersion(uint8_t *imagePtr, int imageSize, char *image_name,
                    int image_name_len);

/** Get the ident signature from a partition.
 * @param part (IN)    partition to search, 1=first, 2=second, 3=boot, 4=non-boot
 *        key  (IN)    ident key value, leave blank to return all ident values
 *        buf  (OUT)   buffer for return ident string
 *        len  (IN)    length of provided buffer
 *
 * @return             size of string on success, 0 if nothing found, -1 for error
 *                     if this value is greater than len, there was more found than
 *                     could fit in the buffer
 */
int bcmFlash_getIdent(int part, int *start, int *end, const char *key, char *buf, int len);


int getBootPartition( void );
int getUpgradePartition( void );
int setImgValidStatus( int partition, int * status );
int getImgValidStatus( int partition, int * status );
int setImgSeqNum( int partition, int seq );
int getImgSeqNum( int partition, int * seq );
int commit( int partition, char *string );
int writeImageToNand(unsigned char *string, int size);
int isLegacyFlashLayout(void);
uint64_t getAvailImgSpace(int update_img_idx);
uint64_t getAvailLoaderSpace(int update_img_idx);
int synchLoaderEnv(char * loader_fname);
uint64_t getSysfsBytes(char *pathname);

#endif /* _BCM_FLASHUTIL_H_ */
