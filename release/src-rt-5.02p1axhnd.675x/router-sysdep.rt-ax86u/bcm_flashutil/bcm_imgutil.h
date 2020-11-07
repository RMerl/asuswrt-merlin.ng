/****************************************************************************
 *
 *  Copyright (c) 2016  Broadcom
 *  All Rights Reserved
 *
 * <:label-BRCM:2016:DUAL/GPL:standard
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
****************************************************************************/

/* Split from cms_image.h. */

#ifndef _BCM_IMGUTIL_H_
#define _BCM_IMGUTIL_H_

/* ---- Include Files ----------------------------------------------------- */

#include "os_defs.h"
#include "bcm_imgutil_def.h"


/* ---- Constants and Types ----------------------------------------------- */

#ifndef CMS_IMAGE_REQUIRED_LEN
#define CMS_IMAGE_REQUIRED_LEN          (4 * 1024 * 1024)
#endif

/* Options: write to partition 1/2. */
#define CMS_IMAGE_WR_OPT_PART1          0x10
#define CMS_IMAGE_WR_OPT_PART2          0x20

/* Options: Do not reboot after writing image to non-active partition. */
#define CMS_IMAGE_WR_OPT_NO_REBOOT      0x80

#define CMS_IMAGE_OVERHEAD              256
#define CMS_CONFIG_FILE_DETECTION_LENGTH 64

typedef enum
{
    IMGUTIL_PROC_OK = 0,
    IMGUTIL_PROC_ERR,
    IMGUTIL_PROC_INVALID_IMG,
    IMGUTIL_PROC_NOT_FOUND,
    IMGUTIL_PROC_FOUND,
    IMGUTIL_PROC_MORE_DATA /* Need more data. */
} imgutil_ret_e;


/* ---- Variable Externs -------------------------------------------------- */

/* ---- Function Prototypes ----------------------------------------------- */

UINT8* bcmImg_allocImgBuf(UINT32 maxBufSize, UINT32 *allocBufSizeP);
UINT32 bcmImg_getImageFlashSize(void);
CmsRet bcmImg_flashImage(char *imagePtr, UINT32 imageLen,
  CmsImageFormat format, UINT32 opts, int *noReboot);
CmsImageFormat bcmImg_parseImgHdr(UINT8 *bufP, UINT32 bufLen);
void bcmImg_reboot(void);
UBOOL8 bcmImg_willFitInFlash(UINT32 imageSize);
UBOOL8 bcmImg_isBackupConfigFlashAvailable(void);
imgutil_ret_e bcmImg_getImageVersion(char *imagePtr, int imageLen, char *imageName,
  int imageNameLen);
CmsImageFormat bcmImg_validateImage(const char *imageBuf, UINT32 imageLen);
UBOOL8 bcmImg_isBcmTaggedImage(const char *imageBuf, UINT32 *imageSize);
UINT32 bcmImg_getConfigFlashSize(void);

imgutil_ret_e bcmImg_ComboImageInit(void);
imgutil_ret_e bcmImg_ComboImageIdentify(const char *imageBuf, UINT32 imageLen);
imgutil_ret_e bcmImg_ComboImageParseHeader(char *imageBuf, UINT32 imageLen,
  UINT32 *consumed, UINT32 *image_len, int *parsingState,
  Comboimg_header_tag *comboTagP, Comboimg_individual_img_tag *indvTagP,
  imgutil_accept_range_ctx_t *ar_ctx);
UBOOL8 bcmImg_ComboImageParsingDone(void);
UBOOL8 bcmImg_IsValidCombo(void);

#endif /*_BCM_IMGUTIL_H_*/
