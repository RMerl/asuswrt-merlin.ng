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


#ifndef __CMS_IMAGE_H__
#define __CMS_IMAGE_H__

#include "cms.h"
#include "bcm_imgutil_def.h" 


/*!\file cms_image.h
 * \brief Header file for public flash image functions.
 *
 * These functions can be included by any application, including GPL applications.
 * The image can be software images (cfe+fs+kernel, fs+kernel) or config files.
 *
 */


/** Minimum size for image uploads.
 *  Config file uploads can be less than 2K.
 */
#define  CMS_IMAGE_MIN_LEN    2048


/** For NAND ftpd upload, image size is not known and just set to 64MG as the max len
 * for memory allocation now and can be changed if needed later on.
 */
#define CMS_IMAGE_MAX_ALLOC_LEN    64 * 1024 * 1024

/** For NAND ftpd upload, the min required image size for image uploads for malloc is set to 4 MG
 * for memory allocation now and can be changed if needed. 
 */
#define CMS_IMAGE_REQUIRED_LEN        4 * 1024 * 1024

/* The original definition of this enum is in tr69c/inc/tr69cdefs.h
 * It is copied here so that tr69c, cwmpd and cgi can all refer it
 * for image download.
 */
typedef enum {
   eFirmwareUpgrade=1,
   eWebContent     =2,
   eVendorConfig   =3,
   eVendorLog      =4,
   eFirmwareStored =6
} eFileType;


/*!\structure CmsImageTransferStats
 * \Keep track of upload/download image statistics
 */
typedef struct
{
   UBOOL8 isDownload;         /**< indicate whether transfer was a download(true) or upload(false). */
   UINT8  fileType;           /**< CmsImageFormat */
   UINT32 fileSize;           /**< size of file/image */
   UINT32 faultCode;          /**< fault code as defined in TR69 Amendment 5 */
   char faultStr[BUFLEN_64];  /**< fault string as defined in TR69 Amendment 5; but reduce to 64 byte instead of 256 */
   UINT32 startTime;          /**< record at start of transfer number of seconds since Jan 1 1970. */
   UINT32 completeTime;       /**< record at end of transfer number of seconds since Jan 1 1970. */
} CmsImageTransferStats;

/** Validate the given image and return the image format of the given image.
 * 
 * The image can be a broadcom image, whole image, or config file.
 * This function will also verify that the image will fit in flash.
 *
 * @param imagePtr (IN) image to be parsed.
 * @param imageLen (IN) Length of image in bytes.
 * @param msgHandle (IN) message handle from the caller.
 * 
 * @return CmsImageFormat enum.
 */
CmsImageFormat cmsImg_validateImage(const char *imagePtr, UINT32 imageLen, void *msgHandle);


/** Write the image to flash.
 *  The image can be a config file, a broadcom image, or flash image.
 *  This function will validate the image before writing to flash.
 *
 * @param imagePtr (IN) image to be written.  Surprisingly, for cfe+kernel+fs
 *                      flash writes, the image is modified, so we cannot
 *                      declare this parameter as const.
 * @param imageLen (IN) Length of image in bytes.
 * @param msgHandle (IN) message handle from the caller.
 * 
 * @return CmsRet enum.
 */
CmsRet cmsImg_writeImage(char *imagePtr, UINT32 imageLen, void *msgHandle);

CmsRet cmsImg_writeImage_noReboot(char *imagePtr, UINT32 imageLen, void *msgHandle);

/** Just calls cmsImg_writeValidatedImageEx with opts=0 */
CmsRet cmsImg_writeValidatedImage(char *imagePtr, UINT32 imageLen,
                                  CmsImageFormat format, void *msgHandle);

CmsRet cmsImg_writeValidatedImage_noReboot(char *imagePtr, UINT32 imageLen,
                                  CmsImageFormat format, void *msgHandle);

CmsImageFormat cmsImg_parseImgHdr(UINT8 *bufP, UINT32 bufLen);


/** Write a validated image in known format to flash.
 *  The image can be a config file, a broadcom image, or flash image.
 *
 * @param imagePtr (IN) image to be written.  Surprisingly, for cfe+kernel+fs
 *                      flash writes, the image is modified, so we cannot
 *                      declare this parameter as const.
 * @param imageLen (IN) Length of image in bytes.
 * @param format   (IN) CmsImageFormat of the image.
 * @param msgHandle (IN) message handle from the caller.
 * @param opts      (IN) Additional options, see CMS_IMAGE_WR_OPT_XXX above.
 * 
 * @return CmsRet enum.
 */
CmsRet cmsImg_writeValidatedImageEx(char *imagePtr, UINT32 imageLen,
                                    CmsImageFormat format, void *msgHandle,
                                    UINT32 opts);


void cmsImg_reboot(void);


/** Return the number of bytes available in the flash for storing an image.
 * 
 * Note this function does not return the size of the entire flash.
 * Rather, it returns the number of bytes available in
 * the flash for storing the image.
 * 
 * @return the number of bytes available in the flash for storing an image.
 */
UINT32 cmsImg_getImageFlashSize(void);


/** Return the number of bytes available in the flash for storing a config file.
 *
 * Note, if compressed config file is enabled, this function will return
 * a number that is larger than the actual number of bytes available in
 * the flash for storing the config file
 *
 * @return the number of bytes available for storing a config file.
 */
UINT32 cmsImg_getConfigFlashSize(void);


/** Return the actual number of bytes available in the flash for storing a config file.
 *
 * This returns the actual number of bytes available in the flash for
 * storing a config file.  External callers should not use this function.
 *
 * @return the number of bytes available for storing a config file.
 */
UINT32 cmsImg_getRealConfigFlashSize(void);


/** Return TRUE if the flash has space for a backup config file.
 *
 * @return TRUE if the flash has space for a backup config file.
 */
UBOOL8 cmsImg_isBackupConfigFlashAvailable(void);


/** Return the size, in bytes, of the broadcom image tag header.
 * 
 * @return the number of bytes in the broadcom image tag header that is present
 *         at the beginning of broadcom images.
 */
UINT32 cmsImg_getBroadcomImageTagSize(void);


/** Return true if the image will fit in the flash.
 * 
 * Compares the given image length with the image flash size minus a
 * CMS_IMAGE_OVERHEAD margin.  Do we really need to have this margin?
 * Currently, only httpd uses this code.
 * 
 * @param imageLen (IN) Length of image in bytes.
 * 
 * @return True if the image will fit in the flash.
 */
UBOOL8 cmsImg_willFitInFlash(UINT32 imageLen);


#define CMS_CONFIG_XML_TAG "<?xml version="

/** Check the first CMS_CONFIG_FILE_DETECTION_LENGTH bytes of the given
 *  buffer to see if this is a config file.
 * 
 * @param buf (IN) buffer containing the image to be analyzed.
 * 
 * @return TRUE if the image is likely to be a config file.
 */
UBOOL8 cmsImg_isConfigFileLikely(const char *buf);


/** Given a config buf, return the BDK component name corresponding to the buf.
 *
 * Note: the buffer could be a CMS Classic monolithic config file, so return
 * value will be BDK_COMP_CMS_CLASSIC.
 * Or the buffer could be a single segment of a config file generated by
 * BDK distrubuted MDM, in which case the return value will be one of the BDK
 * components defined by bdk.h.
 * Or NULL if it could not be identified.
 * Or ERROR_INVALID_CONFIG if the config file is really badly formatted.
 * or BDK_Concatenated_Config if it is a BDK concatenated config file.
 */
const char* cmsImg_configTagToComponentName(const char *buf);

/** Return count of number of sections in given config buf.
 */
UINT32 cmsImg_countConfigSections(const char *buf);


/** cmsImg_sendLoadStartingMsg() and cmsImg_sendLoadDoneMsg() are obsolete.
 *
 * On modern systems which have more memory and memory efficient image util
 * API's, there is no need to do anything prior to download, so there is no
 * need to call these functions.
 */
void cmsImg_sendLoadStartingMsg(void *msgHandle, const char *connIfName); 
void cmsImg_sendLoadDoneMsg(void *msgHandle); 


/** Send a reboot request msg to smd.
 * 
 * @param msgHandle (IN) the message handle to use to send the message.
 */
void cmsUtil_sendRequestRebootMsg(void *msgHandle);


/** Given a socket file descriptor, return the interface that the socket is bound to.
 *
 * This function is useful during image upload when we need to know if the 
 * image data will be coming from the LAN side or WAN side, and if WAN side, 
 * which interface on the WAN it will be coming from (so we can bring down the
 * other WAN interfaces to save memory.)
 *
 * @param socketfd    (IN) the socket of the connection
 * @param connIfName (OUT) on successful return, this buffer will contain the
 *                         linux interface name that is associated with the socket.
 *                         Caller must supply a buffer of at least CMS_IFNAME_LENGTH bytes.
 *
 * @return CmsRet enum.
 */
CmsRet cmsImg_saveIfNameFromSocket(SINT32 socketfd, char *connIfName);

/** If the image has bcmTag, return TRUE, and fill the broadcom tagged image length
 *
 * @param imageBuf (IN) ImageBuf to be checked
 * @param imageLen (OUT) Length of image in bytes if it is a bcmTagged image.
 * 
 * @return TRUE/FALSE.
 */
UBOOL8 cmsImg_isBcmTaggedImage(const char *imageBuf, UINT32 *imageSize);

#ifdef SUPPORT_TR69C_AUTONOMOUS_TRANSFER_COMPLETE
/** Store the image statistics into scratch pad.
 *  After flashing image or config file successfully,
 *  system reboots.  So, it is neccessary to store
 *  statistic of the upload/download process.
 *
 * @param stats (IN) a pointer this structure CmsImageTransferStats
 *                   to be stored
 * @return CmsRet enum.
 */
CmsRet cmsImg_storeImageTransferStats(const CmsImageTransferStats *stats);

/** Send a autonomous transfer complete msg to smd.
 * 
 * @param msgHandle (IN) the message handle to use to send the message.
 * @param pStats (IN) the pointer to CmsImageTransferStats
 */
void cmsImg_sendAutonomousTransferCompleteMsg(void *msgHandle, const CmsImageTransferStats *pStats);

/** Get the file transfer statistics from scratch pad.
 *
 * @param stats (OUT) a pointer this structure CmsImageTransferStats
 *                   to be stored
 * @return CmsRet enum.
 */
CmsRet cmsImg_getImageTransferStats(CmsImageTransferStats *pStats);

/** Clear the file transfer statistics from scratch pad stored with CMS_IMAGE_TRANSFER_STATS_PSP_KEY
 *
 * @return CmsRet enum.
 */
CmsRet cmsImg_clearImageTransferStats(void);

#define CMS_IMAGE_TRANSFER_STATS_PSP_KEY  "xferStats"
#endif /* SUPPORT_TR69C_AUTONOMOUS_TRANSFER_COMPLETE */

CmsImageFormat cmsImg_ConfigFileValidate(UINT8 *imagePtr, UINT32 imageLen,
  void *usrDataP);
int cmsImg_ConfigFileWrite(UINT8 *imagePtr, UINT32 imageLen, void *usrDataP);


void cmsImg_parseImageVersionForSwVersion(const char *imageVersion,
                                          char *swVersion, SINT32 len);
CmsRet cmsImg_getIdent(SINT32 part, char *key, char *value, SINT32 len);

/** Get version tag info from the firmware image in the specified partition.
 *
 * @param (IN) part: partition number, either 1 or 2.
 * @param (OUT) fullVersionBuf: buffer to hold the "imageversion" key.  Must be
 *              at least IMAGE_VERSION_TAG_SIZE+1 bytes long.
 *              Example: 5041GW_PURE1812211109
 * @param (OUT) shortVersionBuf: if not null, will hold the short version key ("imgversion");
 *              If this key is not in image, will parse the short version from the fullVersionBuf.
 *              Must be at least IMAGE_VERSION_TAG_SIZE+1 bytes long.
 *              Example: 5.04L.01
 * @return CmsRet;
 */
CmsRet cmsImg_getFirmwareNameVersion(SINT32 part,
                                     char *fullVersionBuf,
                                     char *shortVersionBuf);

#endif /*__CMS_IMAGE_H__*/

