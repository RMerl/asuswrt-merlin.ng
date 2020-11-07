/*****************************************************************************
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
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

/*****************************************************************************
*    Description:
*
*      The BCM CPE software Image Incremental Flashing (imageIf) library
*      implementation. This library runs on top of bcm_flashutil. It uses
*      the incremental flashing mechanism to avoid holding the entire image
*      in RAM, thus reduces RAM usage.
*      This library is expected to be platform-independent. The underlying
*      bcm_flashutil library handles the platform-specific operations.
*      Notes about the library:
*      - It supports FLASH format image, with a Whole Flash Image WFI_TAG
*        (length TOKEN_LEN) trailer. It supports NAND flash.
*      - It assumes CFEROM stays on erase block #0 only.
*      - It does not handle BROADCOM format image or NOR flash.
*      - It does not handle the configuration file upgrade, and messages
*        related to the configuration file validation.
*
*****************************************************************************/

/* ---- Include Files ----------------------------------------------------- */

#include <fcntl.h>
#include <mtd/mtd-user.h>
#include <linux/jffs2.h>
#include <linux/errno.h>

#include "bcmTag.h"
#include "genutil_crc.h"
#include "board.h"
#include "bcm_flashutil.h"
#include "bcm_flashutil_private.h"
#include "bcm_flashutil_nand.h"
#include "bcm_imgif.h"
#include "bcm_imgif_nand.h"

#include "flash_api.h"

/* ---- Private Constants and Types --------------------------------------- */

/* #define CC_IMGIF_DEBUG 1 */

#define MTD_NAME_IMAGE          "image"
#define MTD_NAME_IMAGE_UPDATE   "image_update"
#define CFERAM_NAME_LEN         20
#define CFERAM_SIZE             (9 * eraseSize)
#define IMG_TRAILER_LEN         TOKEN_LEN
#define BCM_BCMFS_TAG_OFFSET    0x100
#define FLASH_ERASE_ITERATIONS  3

#define getCtx() (&imgifCtx)
#define getCtxByHandle(h) ((imgif_ctx_t*)h)
#define copyDataToBucket(ctxP, usrP, copyLen) \
{ \
    memcpy(ctxP->currP, usrP, copyLen); \
    ctxP->currP += copyLen; \
    ctxP->curSize += copyLen; \
    ctxP->usrBytes += copyLen; \
}
#define getProcessedDataLen(ctxP) ((ctxP->curSize > ctxP->eraseSize)? \
  ctxP->eraseSize : ctxP->curSize);
#define getFlashOffsetForCferam(s) (9 * s)

#if defined(CC_IMGIF_DEBUG)
#define imgif_debug(fmt, arg...) \
  printf("%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define DUMPIMGIFCTX(cxtP) dumpImgifCtx(ctxP)
#define DUMPWFITAG(wtP) dumpWfiTag(wtP)
#else
#define imgif_debug(fmt, arg...)
#define DUMPIMGIFCTX(cxtP)
#define DUMPWFITAG(wtP)
#endif

#define imgif_error(fmt, arg...) \
  printf("ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)


/* IMGIF NAND context. */
typedef struct
{
    /*
     * FLASH info:
     *
     * flashInfo - NOR or NAND.
     * flashSize - total flash size.
     * nvramDataP - nvram data from flash, used for cferom flashing.
     * eraseSize - flash erase size.
     * secureBoot - boot information.
     * btrmEnable - boot information.
     */
    UINT32 flashInfo;
    UINT32 flashSize;
    NVRAM_DATA *nvramDataP;
    UINT32 eraseSize;
    UBOOL8 secureBoot;
    UBOOL8 btrmEnable;

    /* MTD info for MTD_NAME_IMAGE_UPDATE. */
    mtd_info_t *mtdP;
    int mtdFd;

    /*
     * Software image info:
     *
     * fmtParserCb - image format parser callback function.
     * calCrc32Cb - application-specific CRC algorithm, OMCI uses I.363.5.
     * imgFormat - only flash-format image is supported.
     * cferamFname - cferam name embedded in file.
     * imgInfoBi - built-in image information (size, crc).
     * imgInfoExt - application-provided image information.
     * imgSize - calculated image size.
     * crcBody - image content crc (excluding WFI_TAG).
     * crcAll - whole image crc.
     * wfiFlags - whole flash image flags, refer WFI_TAG.
     * imageType - image type, either JFFS2 (which includes UBI split) or
         pureUBI.
     * cferomFound - image includes cferom (optional).
     * cferamSize - cferam size, always = eraseSize.
     * cferomSize - cferom size, always = eraseSize.
     * cferamDirOffset - cferam dirp offset.
     * cferamP - saved CFERAM sequence number block local copy before flashing.
     * cferamP2 - saved second CFERAM sequence number block (if needed, pureUBI for example) local copy before flashing.
     * cferomP - saved CFEROM block local copy before flashing.
     */
    IMG_FORMAT_PARSER_CB fmtParserCb;
    CAL_CRC32_CB calCrc32Cb;
    UINT32 imgFormat;
    imgif_img_info_t imgInfoBi;
    imgif_img_info_t imgInfoExt;
    UINT32 imgSize;
    UINT32 crcBody;
    UINT32 crcAll;
    UINT32 wfiFlags;
    UINT32 imageType;
    UBOOL8 cferomFound;
    UINT32 cferamSize;
    UINT32 cferomSize;
    UINT8 *cferamP;
    UINT8 *cferamP2;
    UINT8 *cferomP;

    /*
     * Run time upgrade info:
     *
     * upgradeState - refer imgif_state_e.
     * usrBlks - processed blocks from user, not necessarily = flashedBlks.
     * usrBytes - copied bytes from user.
     * flashedBlks - programmed flash blocks.
     * flashedBytes - programmed flash bytes.
     * procBytes - processed user bytes.
     * startP - bucket starting pointer.
     * endP - bucket end pointer.
     * currP - current image segment end pointer.
     * curSize - current image segment size.
     * flashBlkCferam - cferam end address on flash.
     * flashBlkAddr - current flash programming addr.
     * cferamSeqNum - cferam sequence number.
     * cferamSeqBlksFound - the number of discovered cferam blocks with a
         sequence number.
     * writingUbifs - ubifs split marker.
     * lastB - last block processing.
     */
    imgif_state_e upgradeState;
    UINT32 bucketSize;
    UINT32 usrBlks;
    UINT32 usrBytes;
    UINT32 flashedBlks;
    UINT32 flashedBytes;
    UINT32 procBytes;
    UINT32 curSize;
    UINT8 *startP;
    UINT8 *currP;
    UINT8 *endP;
    UINT32 flashBlkCferam;
    UINT32 flashBlkAddr;
    int cferamSeqNum;
    int cferamSeqBlksFound;
    UBOOL8 writingUbifs;
    UBOOL8 lastB;
    UBOOL8 tagFound;
} imgif_ctx_t;

/* ---- Private Function Prototypes --------------------------------------- */

static imgif_ret_e imgif_ctx_init(imgif_ctx_t *ctxP);
static imgif_ret_e imgif_ctx_clean(imgif_ctx_t *ctxP);
static imgif_ret_e checkCferom(imgif_ctx_t *ctxP);
static imgif_ret_e procData(imgif_ctx_t *ctxP);
static imgif_ret_e procSearchCferam(imgif_ctx_t *ctxP);
static imgif_ret_e procFlashFs(imgif_ctx_t *ctxP);
static imgif_ret_e procUpdateAndFlashCferam(imgif_ctx_t *ctxP);
static imgif_ret_e procFlashCferom(imgif_ctx_t *ctxP);
static void postProcMoveData(imgif_ctx_t *ctxP);
static UINT32 getFlashType(void);
static int initSeqNum(void);
static UINT32 getFlashImgCrc(UINT8 *crcP);
static void eraseCferamBlock(imgif_ctx_t *ctxP);
static void eraseRemainingBlocks(imgif_ctx_t *ctxP);
static int flashNormalBlock(imgif_ctx_t *ctxP, UINT8 *bufP, UINT32 writeLen);
static UBOOL8 hasEnoughDataToProc(imgif_ctx_t *ctxP);
void dumpImgifCtx(imgif_ctx_t *ctxP);
void dumpWfiTag(WFI_TAG *wtP);

/* ---- Public Variables -------------------------------------------------- */

/* ---- Private Variables ------------------------------------------------- */

static imgif_ctx_t imgifCtx;


/* ---- Functions --------------------------------------------------------- */
/*****************************************************************************
*  FUNCTION:  imgif_nand_open
*  PURPOSE:   Initialize IMGIF context.
*  PARAMETERS:
*      fmtParserCb (IN) Image parser callback function.
       calCrc32Cb (IN) Application-specific CRC algorithm callback function.
*  RETURNS:
*      Pointer to the IMGIF context - successful operation.
*      NULL - failed operation, for example, another software upgrade already
*             in progress.
*  NOTES:
*      None.
*****************************************************************************/
IMGIF_HANDLE imgif_nand_open(IMG_FORMAT_PARSER_CB fmtParserCb,
  CAL_CRC32_CB calCrc32Cb)
{
    imgif_ctx_t *ctxP;
    mtd_info_t *mtdP;
    int mtdFd;
    imgif_ret_e ret;

    imgif_debug("Enter.");

    ctxP = getCtx();
    DUMPIMGIFCTX(ctxP);

    if (ctxP->startP != NULL)
    {
        imgif_error("Another upgrade is in progress.");
        return (IMGIF_HANDLE)NULL;
    }

    memset(ctxP, 0x0, sizeof(imgif_ctx_t));
    ctxP->flashInfo = getFlashType();

    if ((ctxP->flashInfo & FLASH_INFO_FLAG_NAND) == 0)
    {
        imgif_error("This library does not support NOR flash.");
        return (IMGIF_HANDLE)NULL;
    }

    mtdP = get_mtd_device_handle(MTD_NAME_IMAGE, &mtdFd, 0);
    if (mtdP == 0)
    {
        imgif_error("get_mtd_device_handle(%s) failed", MTD_NAME_IMAGE);
        return (IMGIF_HANDLE)NULL;
    }
    else
    {
        put_mtd_device(mtdP, mtdFd, -1);
    }

    mtdP = get_mtd_device_handle(MTD_NAME_IMAGE_UPDATE, &mtdFd, 0);
    if (mtdP == NULL)
    {
        imgif_error("get_mtd_device_handle(%s) failed", MTD_NAME_IMAGE_UPDATE);
        return (IMGIF_HANDLE)NULL;
    }
    else
    {
        ctxP->mtdP = mtdP;
        ctxP->mtdFd = mtdFd;
        ctxP->eraseSize = mtdP->erasesize;
        ctxP->flashSize = mtdP->size;
    }

    ret = imgif_ctx_init(ctxP);
    if (ret != IMGIF_PROC_OK)
    {
        imgif_error("imgif_ctx_init() failed.");
        return (IMGIF_HANDLE)NULL;
    }

    ctxP->upgradeState = IMGIF_UPGRADE_SEARCH;

    ctxP->fmtParserCb = fmtParserCb;
    ctxP->calCrc32Cb = calCrc32Cb;
    if (ctxP->calCrc32Cb != NULL)
    {
        ctxP->calCrc32Cb(0, &ctxP->crcAll, NULL, 0);
    }

    DUMPIMGIFCTX(ctxP);

    return (IMGIF_HANDLE)ctxP;
}

/*****************************************************************************
*  FUNCTION:  imgif_nand_set_image_info
*  PURPOSE:   Set image info obtained externally.
*  PARAMETERS:
       h (IN) - IMGIF context pointer.
       imgInfoExtP (OUT) - image info pointer.
*  RETURNS:
*      0 - successful operation.
*      < 0 - failed operation.
*  NOTES:
*      This function may be invoked at any time. However, it is typically
*      invoked at the end of the upgrade, when user has received the whole
*      image, and has obtained the image integrity information.
*****************************************************************************/
int imgif_nand_set_image_info(IMGIF_HANDLE h, imgif_img_info_t *imgInfoExtP)
{
    imgif_ctx_t *ctxP;

    imgif_debug("Enter, h=%p, info=%p.", h, imgInfoExtP);

    ctxP = getCtxByHandle(h);
    if ((ctxP != NULL) && (imgInfoExtP != NULL))
    {
        ctxP->imgInfoExt.bitmask = imgInfoExtP->bitmask;
        ctxP->imgInfoExt.size = imgInfoExtP->size;
        ctxP->imgInfoExt.crc = imgInfoExtP->crc;
        return 0;
    }

    return -1;
}

/*****************************************************************************
*  FUNCTION:  imgif_nand_get_flash_info
*  PURPOSE:   Obtain flash info.
*  PARAMETERS:
       h (IN) - IMGIF context pointer.
       flashInfoP (OUT) - flash info pointer.
*  RETURNS:
*      0 - successful operation.
*      < 0 - failed operation.
*  NOTES:
*      This function may be invoked at any time.
*****************************************************************************/
int imgif_nand_get_flash_info(imgif_flash_info_t *flashInfoP)
{
    mtd_info_t *mtdP;
    int mtdFd;

    imgif_debug("Enter, info=%p.", flashInfoP);

    flashInfoP->flashType = getFlashType();

    mtdP = get_mtd_device_handle(MTD_NAME_IMAGE_UPDATE, &mtdFd, 0);
    if (mtdP == NULL)
    {
        imgif_error("get_mtd_device_handle(%s) failed", MTD_NAME_IMAGE_UPDATE);
        return -1;
    }
    else
    {
        flashInfoP->eraseSize = mtdP->erasesize;
        flashInfoP->flashSize = mtdP->size;
    }

    return 0;
}

/*****************************************************************************
*  FUNCTION:  imgif_nand_write
*  PURPOSE:   Write image block to IMGIF.
*  PARAMETERS:
*      h (IN) - IMGIF context pointer.
*      dataP (IN) - image block data pointer.
*      len (IN) - image block size.
*  RETURNS:
*      >=0 - number of bytes written.
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
int imgif_nand_write(IMGIF_HANDLE h, UINT8 *dataP, int len)
{
    imgif_ctx_t *ctxP;
    int remainLen = len;
    int copyLen = 0;
    int writeBytes = 0;
    UINT8 *usrP = dataP;
    imgif_ret_e ret;

    if ((h == NULL) || (dataP == NULL))
    {
        imgif_error("Invalid params");
        return -1;
    }

    ctxP = getCtxByHandle(h);

    imgif_debug("Enter, dataP=%p, len=%d.", dataP, len);
    DUMPIMGIFCTX(ctxP);

    if (ctxP->calCrc32Cb != NULL)
    {
        ctxP->calCrc32Cb(1, &ctxP->crcAll, dataP, len);
    }

    do
    {
        copyLen = ((ctxP->currP + remainLen) > ctxP->endP) ?
          (ctxP->endP - ctxP->currP) : remainLen;
        imgif_debug("Copy from user, usrP=%p, copyLen=%d.", usrP, copyLen);
        if (copyLen == 0)
        {
            break;
        }

        copyDataToBucket(ctxP, usrP, copyLen);

        usrP += copyLen;
        remainLen -= copyLen;
        writeBytes += copyLen;
        if (hasEnoughDataToProc(ctxP))
        {
            ret = procData(ctxP);
            if (ret == IMGIF_PROC_ERR)
            {
                /* Abort. */
                writeBytes = -1;
                break;
            }
            ctxP->usrBlks++;
        }

        DUMPIMGIFCTX(ctxP);

    } while (remainLen > 0);

    if (writeBytes != len)
    {
        imgif_error("imgif_write() len=%d, written=%d", len, writeBytes);
    }

    DUMPIMGIFCTX(ctxP);

    imgif_debug("Exit, len=%d, written=%d", len, writeBytes);

    return writeBytes;
}

/*****************************************************************************
*  FUNCTION:  imgif_nand_close
*  PURPOSE:   Close IMGIF context.
*  PARAMETERS:
*      h (IN) - IMGIF context pointer.
*      abortFlag (IN) - TRUE: user aborts the operation.
*  RETURNS:
*      0 - successful operation.
*      -1 - failed operation.
*  NOTES:
*      User may stop an upgrade at any time, when there is an operator,
*      command, protocol failure, or unmatched image size and/or CRC values
*      between IMGIF and user.
*****************************************************************************/
int imgif_nand_close(IMGIF_HANDLE h, UBOOL8 abortFlag)
{
    imgif_ctx_t *ctxP;
    imgif_ret_e imgifRet;
    UINT8 *tagP;
    int blkSize;
    WFI_TAG wt;
    int ret = -1;

    if (h == NULL)
    {
        imgif_error("Invalid params");
        return -1;
    }

    ctxP = getCtxByHandle(h);
    imgif_debug("Enter, h=%p, flag=%d.", h, abortFlag);

    if (abortFlag == TRUE)
    {
        ret = 0;
        goto imgif_close_exit;
    }

    if (ctxP->curSize < IMG_TRAILER_LEN)
    {
        imgif_error("Unexpected state, curSize=%d", ctxP->curSize);
        goto imgif_close_exit;
    }

    tagP = ctxP->currP - IMG_TRAILER_LEN;
    memcpy(&wt, tagP, sizeof(wt));

    DUMPWFITAG(&wt);

    DUMPIMGIFCTX(ctxP);

    blkSize = ctxP->eraseSize / 1024;

    if (validateWfiTag(&wt, blkSize, ctxP->btrmEnable) < 0)
    {
        imgif_error("Unexpected WFI tag content");
        goto imgif_close_exit;
    }

    ctxP->imgInfoBi.bitmask = IMG_INFO_BITMASK_CRC;
    ctxP->imgInfoBi.crc = getFlashImgCrc(tagP);
    ctxP->wfiFlags = wt.wfiFlags;

    /* Image integrity check. */

    ctxP->imgSize = ctxP->usrBytes;

    if (ctxP->calCrc32Cb != NULL)
    {
        ctxP->calCrc32Cb(2, &ctxP->crcAll, NULL, 0);
    }

    if ((ctxP->imgInfoExt.bitmask & IMG_INFO_BITMASK_CRC) &&
      (ctxP->imgInfoExt.crc != ctxP->crcAll))
    {
        imgif_error("Unmatched file CRC values, ext=0x%08x, cal=0x%08x",
          ctxP->imgInfoExt.crc, ctxP->crcAll);
        goto imgif_close_exit;
    }

    if ((ctxP->imgInfoBi.bitmask & IMG_INFO_BITMASK_CRC) &&
      (ctxP->imgInfoBi.crc != ctxP->crcBody))
    {
        imgif_error("Unmatched file CRC values, bi=0x%08x, cal=0x%08x",
          ctxP->imgInfoBi.crc, ctxP->crcBody);
        goto imgif_close_exit;
    }

    if ((ctxP->imgInfoExt.bitmask & IMG_INFO_BITMASK_SIZE) &&
      (ctxP->imgInfoExt.size != ctxP->imgSize))
    {
        imgif_error("Unmatched file size values, ext=0x%08x, cal=0x%08x",
          ctxP->imgInfoExt.size, ctxP->imgSize);
        goto imgif_close_exit;
    }

    ctxP->upgradeState = IMGIF_UPGRADE_FS_FINAL;
    ctxP->lastB = TRUE;

    /* Adjust ctx, not to flash IMG_TRAILER_LEN. */
    ctxP->currP -= IMG_TRAILER_LEN;
    ctxP->curSize -= IMG_TRAILER_LEN;

    /* Flash remaining data. */
    if (ctxP->curSize > 0)
    {
        imgifRet = procData(ctxP);
        if (imgifRet == IMGIF_PROC_OK)
        {
            postProcMoveData(ctxP);
        }
        else if (imgifRet == IMGIF_PROC_ERR)
        {
            imgif_error("procData() failed, ret=%d.", imgifRet);
            goto imgif_close_exit;
        }
    }

    /*
     * Erase remaining blocks. This follows existing full-image upgrade
     * implementation, however it may not be required.
     */
    eraseRemainingBlocks(ctxP);

    /* Flash cferam. */
    imgifRet = procUpdateAndFlashCferam(ctxP);
    if (imgifRet == IMGIF_PROC_ERR)
    {
        imgif_error("procUpdateAndFlashCferam() failed, ret=%d.", imgifRet);
        goto imgif_close_exit;
    }

    /* Flash cferom, must be the last one. */
    if (ctxP->cferomFound == TRUE)
    {
        if (handleCferom(ctxP->mtdP, (char*)ctxP->cferomP, ctxP->cferomSize, ctxP->wfiFlags,
          ctxP->nvramDataP) < 0)
        {
            imgif_error("handleCferom() failed.");
            goto imgif_close_exit;
        }

        imgifRet = procFlashCferom(ctxP);
        if (imgifRet == IMGIF_PROC_ERR)
        {
            imgif_error("procFlashCferom() failed, ret=%d.", imgifRet);
            goto imgif_close_exit;
        }
    }

    ret = 0;

imgif_close_exit:
    DUMPIMGIFCTX(ctxP);
    imgif_ctx_clean(ctxP);
    return ret;
}

/* ---- Private Functions ------------------------------------------------- */

static imgif_ret_e imgif_ctx_init(imgif_ctx_t *ctxP)
{
    int seq;

    ctxP->bucketSize = ctxP->eraseSize + IMG_TRAILER_LEN;
    ctxP->startP = malloc(ctxP->bucketSize);
    if (ctxP->startP == NULL)
    {
        imgif_error("malloc(%d) failed.", ctxP->bucketSize);
        goto init_exit;
    }

    ctxP->crcBody = CRC_INITIAL_VALUE;
    ctxP->crcAll = CRC_INITIAL_VALUE;

    ctxP->currP = ctxP->startP;
    ctxP->endP = ctxP->startP + ctxP->bucketSize;

    ctxP->nvramDataP = malloc(sizeof(NVRAM_DATA));
    if (ctxP->nvramDataP == NULL)
    {
        imgif_error("malloc(%d) failed.", (int)sizeof(NVRAM_DATA));
        goto init_exit;
    }

    if (readNvramData(ctxP->nvramDataP) <= 0)
    {
        imgif_error("readNvramData() failed");
        goto init_exit;
    }

    seq = initSeqNum();
    if (seq < 0)
    {
        imgif_error("initSeqNum() failed, seq=%d", seq);
        goto init_exit;
    }

    ctxP->cferamSeqNum = seq;
    ctxP->upgradeState = IMGIF_UPGRADE_IDLE;

    /* Information only. */
    ctxP->secureBoot = otp_is_boot_secure();
    ctxP->btrmEnable = otp_is_btrm_boot();

    return IMGIF_PROC_OK;

init_exit:
    imgif_ctx_clean(ctxP);
    return IMGIF_PROC_ERR;
}

static imgif_ret_e imgif_ctx_clean(imgif_ctx_t *ctxP)
{
    if (ctxP->startP != NULL)
    {
        free(ctxP->startP);
        ctxP->startP = NULL;
        ctxP->endP = NULL;
        ctxP->currP = NULL;
    }

    if (ctxP->nvramDataP != NULL)
    {
        free(ctxP->nvramDataP);
        ctxP->nvramDataP = NULL;
    }

    if (ctxP->mtdP != NULL)
    {
        put_mtd_device(ctxP->mtdP, ctxP->mtdFd, -1);
        ctxP->mtdP = NULL;
        ctxP->mtdFd = 0;
    }

    if (ctxP->cferomP != NULL)
        free(ctxP->cferomP);

    if (ctxP->cferamP != NULL)
        free(ctxP->cferamP);

    if (ctxP->cferamP2 != NULL)
        free(ctxP->cferamP2);

    ctxP->cferamSeqNum = -1;

    return IMGIF_PROC_OK;
}

void dumpImgifCtx(imgif_ctx_t *ctxP)
{
    printf("--------------------------------------------------------------\n");
    printf("Software image info:\n");
    printf("\tformat=%d\n", ctxP->imgFormat);

    printf("\timgInfo(ext): bitmask=%x, crc=0x%08x, size=%d\n",
      ctxP->imgInfoExt.bitmask, ctxP->imgInfoExt.crc, ctxP->imgInfoExt.size);
    printf("\timgInfo(bi):  bitmask=%x, crc=0x%08x, size=%d\n",
      ctxP->imgInfoBi.bitmask, ctxP->imgInfoBi.crc, ctxP->imgInfoBi.size);
    printf("\timgInfo(cal): crcBody=0x%08x, crcAll=0x%08x, size=%d\n",
      ctxP->crcBody, ctxP->crcAll, ctxP->imgSize);

    printf("\tcferam: saved=%p, seq=%d\n",
      ctxP->cferamP, ctxP->cferamSeqNum);
    printf("\tcferam2: saved=%p, seq=%d\n",
      ctxP->cferamP2, ctxP->cferamSeqNum);
    printf("\tcferom: found=%d, saved=%p\n",
      ctxP->cferomFound, ctxP->cferomP);
    printf("Run time upgrade info:\n");
    printf("\tstate=%d, lastB=%d\n", ctxP->upgradeState, ctxP->lastB);
    printf("\tbucketSize=%d, startP=%p, endP=%p, currP=%p,\n"
      "\teraseSize=0x%x, curSize=%d\n",
      ctxP->bucketSize, ctxP->startP, ctxP->endP, ctxP->currP,
      ctxP->eraseSize, ctxP->curSize);
    printf("\tuser: blocks=%d, bytes=%d\n",
      ctxP->usrBlks, ctxP->usrBytes);
    printf("\tflash: addr=%u, cferam=%u, blocks=%d, bytes=%d\n",
      ctxP->flashBlkAddr, ctxP->flashBlkCferam,
      ctxP->flashedBlks, ctxP->flashedBytes);
    printf("--------------------------------------------------------------\n");
}

void dumpWfiTag(WFI_TAG *wtP)
{
    printf("WFI tag:\n");
    printf("\t CRC:        0x%08x\n", wtP->wfiCrc);
    printf("\t Version:    0x%08x\n", wtP->wfiVersion);
    printf("\t Chip ID:    0x%08x\n", wtP->wfiChipId);
    printf("\t Flash Type: 0x%08x\n", wtP->wfiFlashType);
    printf("\t Flags:      0x%08x\n", wtP->wfiFlags);
}

static int initSeqNum(void)
{
    int seq = -1;

    /* Get sequence number of booted partition. */
    seq = getSequenceNumber(getBootPartition());
    imgif_debug("Get seq# %d.", seq);

    if (seq == -1)
    {
        return seq;
    }

    return (seq);
}

static UBOOL8 hasEnoughDataToProc(imgif_ctx_t *ctxP)
{
    UBOOL8 enough = FALSE;

    if (ctxP->lastB == TRUE)
    {
        enough = TRUE;
    }
    else
    {
        if ((ctxP->curSize >= IMG_TRAILER_LEN) &&
          (ctxP->curSize - IMG_TRAILER_LEN) >= ctxP->eraseSize)
        {
            enough = TRUE;
        }
    }

    return enough;
}

static UINT32 getFlashType(void)
{
    unsigned int flags = 0;
    UINT32 flashType = 0;
    int ret;

    ret = getFlashInfo(&flags);
    if (ret >= 0) /* OK */
    {
        if (flags & FLASH_INFO_FLAG_NAND)
        {
            flashType = FLASH_INFO_FLAG_NAND;
        }
    }
    else
    {
        imgif_error("getFlashInfo() failed, ret=%d.", ret);
    }

    return flashType;
}

/* Get CRC value from WFI tag. */
static UINT32 getFlashImgCrc(UINT8 *crcP)
{
    UINT32 imageCrc;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    imageCrc = crcP[0] | (crcP[1] << 8) | (crcP[2] << 16) | (crcP[3] << 24);
#else
    imageCrc = (crcP[0] << 24) | (crcP[1] << 16) | (crcP[2] << 8) | crcP[3];
#endif
    return imageCrc;
}

static void eraseCferamBlock(imgif_ctx_t *ctxP)
{
    UINT32 blkAddr;

    for (blkAddr = 0; blkAddr < ctxP->flashBlkCferam;
      blkAddr += ctxP->eraseSize)
    {
        imgif_debug("nandEraseBlk() blkAddr=0x%x, "
          "erasesize=%u, blkCferam=0x%x\n",
          blkAddr, ctxP->eraseSize, ctxP->flashBlkCferam);
        nandEraseBlk(ctxP->mtdP, blkAddr, ctxP->mtdFd);
    }
}

static int flashNormalBlock(imgif_ctx_t *ctxP, UINT8 *bufP, UINT32 writeLen)
{
    UINT8 *tagP;
    UINT8 iterations = 0;
    UBOOL8 eraseOK = FALSE;

    if (writeLen == 0)
    {
        imgif_error("writeLen = 0");
        return -1;
    }

    printf(".");
    fflush(stdout);

    for (; (ctxP->flashBlkAddr < ctxP->flashSize);
      ctxP->flashBlkAddr += ctxP->eraseSize, iterations = 0)
    {
        do
        {
            if (nandEraseBlk(ctxP->mtdP, ctxP->flashBlkAddr, ctxP->mtdFd) == 0)
            {
                eraseOK = TRUE;
                break;
            }
            else
            {
                imgif_error("nandEraseBlk() failed, blkAddr=0x%x, iteration=%d",
                  ctxP->flashBlkAddr, iterations);
            }

        } while (++iterations < FLASH_ERASE_ITERATIONS);

        if (eraseOK == TRUE)
        {
            break;
        }
    }

    if (eraseOK == FALSE)
    {
        imgif_error("nandEraseBlk() failed, blkAddr=0x%x, flashsize=0x%x",
          ctxP->flashBlkAddr, ctxP->flashSize);
        return -1;
    }

    imgif_debug("nandWriteBlk(), blkAddr=0x%x, erasesize=%u, "
      "bufP=%p, writeLen=%u",
      ctxP->flashBlkAddr, ctxP->eraseSize, bufP, writeLen);

    if (nandWriteBlk(ctxP->mtdP, ctxP->flashBlkAddr, writeLen, bufP,
      ctxP->mtdFd, !ctxP->writingUbifs) != 0)
    {
        imgif_error("Error writing block 0x%8x, len%d, skipping\n",
          ctxP->flashBlkAddr, writeLen);
        return -1;
    }
    else
    {
        if (writeLen >= ctxP->eraseSize)
        {
            tagP = bufP + writeLen - BCM_BCMFS_TAG_OFFSET;
            if (!strncmp(BCM_BCMFS_TAG, (char*)tagP, strlen(BCM_BCMFS_TAG)))
            {
                if (!strncmp(BCM_BCMFS_TYPE_UBIFS,
                  (char*)tagP + strlen(BCM_BCMFS_TAG),
                  strlen(BCM_BCMFS_TYPE_UBIFS)))
                {
                    ctxP->writingUbifs = TRUE;
                    printf("U");
                    fflush(stdout);
                }
            }
        }
        else
        {
            /* Must be the last write. */
            imgif_debug("Writing last block 0x%8x, len%d",
              ctxP->flashBlkAddr, writeLen);
        }
    }

    return 0;
}

static void postProcMoveData(imgif_ctx_t *ctxP)
{
    UINT32 moveBufLen;

    if (ctxP->curSize > ctxP->eraseSize)
    {
        moveBufLen = ctxP->curSize - ctxP->eraseSize;
        memcpy(ctxP->startP, (ctxP->currP - moveBufLen),
          moveBufLen);
        ctxP->currP = ctxP->startP + moveBufLen;
        ctxP->curSize -= ctxP->eraseSize;
    }
    else
    {
        ctxP->currP = ctxP->startP;
        ctxP->curSize = 0;
    }
}

static imgif_ret_e checkCferom(imgif_ctx_t *ctxP)
{
    UBOOL8 cferomExist = FALSE;

    cferomExist = nand_image_type(ctxP->startP) ?
      FALSE : TRUE;

    if (cferomExist == FALSE)
    {
        return IMGIF_PROC_NOT_FOUND;
    }

    ctxP->cferomP = realloc(ctxP->cferomP, ctxP->cferomSize + ctxP->eraseSize);
    if (ctxP->cferomP == NULL)
    {
        imgif_error("malloc(%d) failed.", ctxP->eraseSize);
        return IMGIF_PROC_ERR;
    }

    memcpy(ctxP->cferomP + ctxP->cferomSize, ctxP->startP, ctxP->eraseSize);
    ctxP->cferomFound = TRUE;
    ctxP->cferomSize += ctxP->eraseSize;
    return IMGIF_PROC_FOUND;
}

static imgif_ret_e parseImgHdr(imgif_ctx_t *ctxP)
{
    CmsImageFormat imgFormat = CMS_IMAGE_FORMAT_INVALID;

    if (ctxP->fmtParserCb != NULL)
    {
        imgFormat = ctxP->fmtParserCb(ctxP->startP,
          ctxP->curSize);
    }

    /* FORMAT_INVALID is also used for unknown. */
    if (imgFormat != CMS_IMAGE_FORMAT_INVALID)
    {
        if (imgFormat != CMS_IMAGE_FORMAT_FLASH)
        {
            imgif_error("Image format %d not supported.\n",
              imgFormat);
            return IMGIF_PROC_ERR;
        }
        else
        {
            ctxP->imgFormat = imgFormat;
        }
    }

    return IMGIF_PROC_OK;
}

static imgif_ret_e procData(imgif_ctx_t *ctxP)
{
    imgif_state_e curState, newState;
    imgif_ret_e ret = IMGIF_PROC_OK;
    UBOOL8 moveOn = FALSE;
    UINT32 procBytes = getProcessedDataLen(ctxP);
    UBOOL8 flashFs = FALSE;
    UINT32 crc = genUtl_getCrc32(ctxP->startP, procBytes, ctxP->crcBody);
    curState = ctxP->upgradeState;
    newState = curState;

    if (!ctxP->imageType)
    {
        if ((ctxP->imageType = nand_image_type(ctxP->startP)))
        { // image type is not determined until after CFEROM block
            /* We're past the CFEROM, adjust the flash address accordingly. */
            if (ctxP->cferomP)
            { // check to see if CFEROM size matches expected size
                int mtd_fd;
                mtd_info_t *mtd = get_mtd_device_handle("nvram", &mtd_fd, 0);

                if( mtd > 0 )
                {
                    if (mtd->size != ctxP->cferomSize)
                    {
                        imgif_error("ERROR!!! Image CFEROM size of 0x%x is not equal to the nvram partition size of 0x%x", ctxP->cferomSize, mtd->size);
                        put_mtd_device(mtd, mtd_fd, -1);
                        return IMGIF_PROC_ERR;
                    }
                }
                else
                {
                    imgif_error("Failed to get nvram mtd device!!!");
                    return IMGIF_PROC_ERR;
                }
            }

            ctxP->flashBlkCferam = getFlashOffsetForCferam(ctxP->eraseSize);
            ctxP->flashBlkAddr = ctxP->flashBlkCferam;
            eraseCferamBlock(ctxP); // this must go here after we have determined imagetype so that ctxP parameters are populated properly, CFEROM won't have been written as it was saved, and CFERAM blocks will not yet have been written
        }
    }

    if (curState == IMGIF_UPGRADE_SEARCH)
    {
        /* Parse image header. */
        if (!ctxP->imageType)
        {
            if (parseImgHdr(ctxP) != IMGIF_PROC_OK)
            {
                return IMGIF_PROC_ERR;
            }

            /* If exists, cferom is in the first block(s). */
            ret = checkCferom(ctxP);
            if (ret == IMGIF_PROC_FOUND)
            {
                /* Skip the cferom. */
                moveOn = TRUE;
            }
        }
        else
        {
            ret = procSearchCferam(ctxP);

            if (ret == IMGIF_PROC_FOUND)
            { // CFERAM block with sequence number found
                /* Skip the cferam. */
                moveOn = TRUE;
                ctxP->writingUbifs = FALSE;
            }
            else if (ret == IMGIF_PROC_NOT_FOUND)
            {
                flashFs = TRUE;
            }

            if ( (ctxP->cferamP) && ((ctxP->imageType == JFFS2_IMAGE) || (ctxP->cferamP2)) )
            { // this allows for finalization of image upgrade which lets state machine know that CFERAM sequence number blocks have been found
                newState = IMGIF_UPGRADE_FS;
            }
        }
    }
    else if ((curState == IMGIF_UPGRADE_FS) ||
      (curState == IMGIF_UPGRADE_FS_FINAL))
    {
        flashFs = TRUE;
    }

    if (flashFs == TRUE)
    {
        ret = procFlashFs(ctxP);
        if (ret == IMGIF_PROC_OK)
        {
            moveOn = TRUE;
            ctxP->flashBlkAddr += ctxP->eraseSize;
        }
        else
        {
            return IMGIF_PROC_ERR;
        }
    }

    if (moveOn == TRUE)
    {
        ctxP->procBytes += procBytes;
        ctxP->crcBody = crc;
        postProcMoveData(ctxP);
    }
    ctxP->upgradeState = newState;

    return IMGIF_PROC_OK;
}

static imgif_ret_e procSearchCferam(imgif_ctx_t *ctxP)
{
    unsigned char *cferamP;

    /*
     * Check and process cferam number in cferam block which contains
     * the cferam number.
     */
    cferamP = nandUpdateSeqNum(ctxP->startP, ctxP->eraseSize, ctxP->eraseSize,
      ctxP->cferamSeqNum, &ctxP->cferamSeqBlksFound);
    if (cferamP == NULL)
    {
        imgif_debug("Not a block containing cferam");
    }
    else
    { // CFERAM block with sequence number found
        imgif_debug("Found a block containing cferam, startP=%p, cferamP=%p",
          ctxP->startP, cferamP);

        /* Found. */
        ctxP->cferamSize = ctxP->eraseSize;

        if (ctxP->cferamP == NULL)
        {
            if ((ctxP->cferamP = malloc(ctxP->cferamSize)) == NULL)
            {
                imgif_error("malloc(%d) failed.", ctxP->cferamSize);
                return(IMGIF_PROC_ERR);
            }

            memcpy(ctxP->cferamP, cferamP, ctxP->cferamSize);
            return(IMGIF_PROC_FOUND);
        }

        if ((ctxP->cferamP2 = malloc(ctxP->cferamSize)) == NULL)
        {
            imgif_error("malloc(%d) failed.", ctxP->cferamSize);
            return(IMGIF_PROC_ERR);
        }

        memcpy(ctxP->cferamP2, cferamP, ctxP->cferamSize);
        return(IMGIF_PROC_FOUND);
    }

    return(IMGIF_PROC_NOT_FOUND);
}

static int scanIdent(UINT8 *bufP, UINT32 writeLen)
{ // scan a block for the ident image version information which was saved as uncompressed file image_ident and create a tmp file with this tag
    unsigned int i;

    for (i = 0; i < (writeLen - (strlen(IDENT_TAG) + 3)); i++)
    {
        if (!strncmp(IDENT_TAG, (char *)(bufP + i), strlen(IDENT_TAG)))
        { // found beginning of ident tag
            int j = 0, k = i + strlen(IDENT_TAG);
            char tag[256];

             while( (k < writeLen) && (bufP[k] > ' ') && (bufP[k] <= '~') )
                tag[j++] = bufP[k++];

            if (bufP[k] == ' ')
            { // whole ident tag found
                FILE *fp;

                tag[j++] = 0;

                fp = fopen("/tmp/imageIdent", "w");
                if (fp != NULL)
                {
                    fwrite(tag, 1, j, fp);
                    fclose(fp);
                    return(1);
                }
            }
        }
    }

    return(0);
}

static imgif_ret_e procFlashFs(imgif_ctx_t *ctxP)
{
    int writeLen;
    imgif_ret_e ret = IMGIF_PROC_ERR;

    writeLen = (ctxP->curSize >= ctxP->eraseSize) ?
      (int)ctxP->eraseSize : (int)ctxP->curSize;

    if (!ctxP->tagFound)
        ctxP->tagFound = scanIdent(ctxP->startP, writeLen);

    if (flashNormalBlock(ctxP, ctxP->startP, writeLen) == 0)
    {
        ctxP->flashedBytes += writeLen;
        ctxP->flashedBlks++;
        ret = IMGIF_PROC_OK;
    }

    return ret;
}

static imgif_ret_e procUpdateAndFlashCferam(imgif_ctx_t *ctxP)
{
    int ret;

    if (ctxP->cferamP == NULL)
    {
        imgif_error("Invalid cferam pointer.");
        return IMGIF_PROC_ERR;
    }

    ret = flashCferam(ctxP->mtdP, ctxP->mtdFd, (int)ctxP->flashBlkCferam,
      ctxP->cferamP, ctxP->cferamP2);
    if (ret < 0)
    {
        imgif_error("flashCferam(%p, 0x%x, %u, %p) failed.",
          ctxP->mtdP, ctxP->mtdFd, ctxP->flashBlkCferam, ctxP->cferamP);
        eraseCferamBlock(ctxP);
        return IMGIF_PROC_ERR;
    }

    ctxP->flashedBytes += ctxP->cferamSize;
    ctxP->flashedBlks++;

    return IMGIF_PROC_OK;
}

static imgif_ret_e procFlashCferom(imgif_ctx_t *ctxP)
{
    int ret;

    ret = flashCferom(ctxP->cferomP, ctxP->cferomSize);
    if (ret < 0)
    {
        imgif_error("flashCferom(%p, 0x%x, %d) failed.",
          ctxP->cferomP, ctxP->wfiFlags, ctxP->cferomSize);
    }

    ctxP->flashedBytes += ctxP->cferomSize;
    ctxP->flashedBlks += ctxP->cferomSize/ctxP->eraseSize;

    return IMGIF_PROC_OK;
}

static void eraseRemainingBlocks(imgif_ctx_t *ctxP)
{
    UINT32 blkAddr;

    for (blkAddr = ctxP->flashBlkAddr; blkAddr < (UINT32)ctxP->mtdP->size;
      blkAddr += ctxP->eraseSize)
    {
        printf(".");
        fflush(stdout);
        imgif_debug("nandEraseBlk() blkAddr=0x%x, erasesize=%u\n",
          blkAddr, ctxP->eraseSize);
        nandEraseBlk(ctxP->mtdP, blkAddr, ctxP->mtdFd);
    }
}
