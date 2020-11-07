/*****************************************************************************
 * <:copyright-BRCM:2016:DUAL/GPL:standard
 *
 *    Copyright (c) 2016 Broadcom
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
* Description:
*
* This file contains the BCM unified software image flashing utility
* (imgutil) library implementation.
*
* This library runs on top of several utilities: devCtl (NOR, BCM image
* flashing), bcm_flashutil (NAND & UBI flashing), and bcm_imgif (NAND
* incremental flashing). It includes the following functions:
*   - Provide a unified flash programming interface to application clients.
*     The clients do not need to be aware of the image format, and the
*     underlying flashing mechanism.
*   - Support the following image formats:
*     * Raw SPI-NOR, raw NAND, raw pure-UBI, and BCM image format (without
*       the bundle image header).
*     * Bundled image with image header, where multiple image files are
*       concatenated, and only one actually needs to be programmed.
*   - Support the whole image upgrade and incremental image upgrade. In
*     the NAND flash case, the library provides the option to either flash
*     at the end, or flash on-the-fly.

* Basic design concept:
*   - Receive enough data to determine the image type.
*     * Bundle image
*     * Configuration file.
*     * Non-bundle image: BCM image(tag in head), UBI, NOR flash, NAND, UBI.
* A: No bundle image header:
*  - NOR flash: buffer the whole image. When done, validate and then
*    call devCtl flashing API.
*  - NAND flash and incremental flashing disabled: buffer the whole
*    image. When done, validate and then call incremental flashing API.
*  - BCM image (tag in head): buffer the whole image. When done, validate
*    and then call devCtl flashing API.
*  - UBI image: buffer the whole image. When done, validate and then call
*    bcm_flashutil API (will be replaced by incremental API later).
*  - NAND and incremental enabled: call incremental flashing API.
* B: Bundle image header:
*  - Image header parsing, find the correct start offset of
*    applicable image (criteria: chip ID, optionally board ID), and find
*    the other fields in the header (image len, image flags, CRC, version,
*    etc.).
*  - Discard non-applicable image buffer.
*  - After header parsing: same as (A).
*
* Additional notes:
*  - The upgrade related inter-process messaging remains in the client
*    (e.g. cmsImg_sendLoadStartingMsg()).
*  - It is client's responsibility to set the image state for the next
*    reboot, and reboot the system after the upgrade, using devCtl APIs.
*
* Future extensions:
*  - Incremental flashing for the pure UBI image.
*  - Incremental flashing for the compressed image (decompress on-the-fly).
*  - The bundle image may include both software image and configuration
*    file?
*
*****************************************************************************/

/* ---- Include Files ----------------------------------------------------- */

#include "bcm_flashutil.h"
#include "bcm_imgutil_api.h"
#include "bcmTag.h"
#include "genutil_crc.h"
#include "bcm_imgutil.h"
#include "bcm_imgif.h"


/* ---- Private Constants and Types --------------------------------------- */

/* #define CC_IMGUTIL_DEBUG 0 */ 
#define IMGUTIL_HDR_BUF_SIZE    0x100000
#define MTD_NAME_IMAGE          "image"
#define MTD_NAME_IMAGE_UPDATE   "image_update"

typedef enum
{
    IMGUTIL_STATE_IDLE = 0,
    IMGUTIL_STATE_HDR,
    IMGUTIL_STATE_IMG,
    IMGUTIL_STATE_NO_IMG,
    IMGUTIL_STATE_ERR
} imgutil_upgrade_state_e;

/*
 * Bundled image header information related to upgrade
 * (parsed, not the original header):
 *   - chip_family ID
 *   - board id (specific board, or any)
 *   - image size
 *   - image flags (compressed, etc.)
 *     Suggest to add:
 *   - image crc value
 *   - image version
 *   - image format (BCM, FLASH, CFG)
 *   - flash type (NOR, NAND)
 */
#define IMGHDR_CHIPID_LEN       0x8
#define IMGHDR_BOARDID_LEN      0x10
#define IMGHDR_VERSION_LEN      0x20

typedef struct
{
    UINT8 chipId[IMGHDR_CHIPID_LEN];
    UINT8 boardId[IMGHDR_BOARDID_LEN];
    UINT8 version[IMGHDR_VERSION_LEN];
    UINT32 imgSize;
    UINT32 flags;
    UINT32 offSet;
    UINT32 crc;
    CmsImageFormat format;
    imgutil_flash_type_e flashType;
} imgutil_imghdr_info_t;

/* IMGUTIL context. */
typedef struct
{
    /* FLASH type: NAND/NOR. */
    UINT32 flashType;
    UINT32 flashSize;
    UINT32 eraseSize;

    /*
     * Bundle header information:
     * bundleHdrExistB - whether the bundle header is found.
     * imgHdr - header for the desired image.
     */
    UBOOL8 bundleHdrExistB;
    imgutil_imghdr_info_t imgHdrInfo;
    int combo_parsing_state;
    Comboimg_header_tag combo_hdr_tag;
    Comboimg_individual_img_tag indiv_hdr_tag;

    /* Client inputs. */
    UINT32 maxBufSize;
    UBOOL8 forceWholeFlashB;
    UINT32 options;
    IMGUTIL_VALIDATE_IMG_FUNC clientValidateFuncP;
    IMGUTIL_WRITE_IMG_FUNC clientFlashFuncP;
    IMGUTIL_CAL_CRC32_FUNC calCrc32FuncP;
    void *clientCtxP;

    /*
     * Run time upgrade info:
     * hdrBufP - pointer to the image header buffer. After the header of the
     *   expected image is parsed, the header is copied to imgBufP, and
     *   hdrBufP is not used any more.
     * hdrBufSize - allocated buffer size for hdrBufP.
     * imgBufP - pointer to the whole image to be flashed. not used for
     *   the incremental flashing case.
     * imgBufSize - allocated buffer size for imgBufP.
     * usrBytes - accumulated copied bytes from _write().
     * accImgSize - accumulated image size from _write(), for both whole and
     *  incremental cases. For the combo image case, only the expected image
     *  is counted.
     * crc - whole image crc.
     * imgStartP - starting point of the image to be flashed.
     */
    UBOOL8 wholeFlashB;
    imgutil_upgrade_state_e upgradeState;
    UINT8 *hdrBufP;
    UINT32 hdrBufSize;
    UINT8 *imgBufP;
    UINT32 imgBufSize;
    UINT32 usrBytes;
    UINT32 accImgSize;
    UINT32 crc;
    UINT8 *imgStartP;
    UINT8 *currP;
    UINT8 *endP;
    IMGIF_HANDLE imgifHandle;

    /*
     * Software image info:
     * imgFormat - parsed image format (not from header).
     * imgInfoBi - built-in image information (not from header).
     * imgInfoEx - client-provided image information.
     */
    UINT32 imgFormat;
    imgutil_img_info_t imgInfoBi;
    imgutil_img_info_t imgInfoEx;
} imgutil_ctx_t;

#define getCtx() (&imgutilCtx)
#define getCtxByHandle(h) ((imgutil_ctx_t*)h)
#define copyDataToBucket(ctxP, usrP, copyLen) \
{ \
    memcpy(ctxP->currP, usrP, copyLen); \
    ctxP->currP += copyLen; \
    ctxP->accImgSize += copyLen; \
    ctxP->usrBytes += copyLen; \
}

#if defined(CC_IMGUTIL_DEBUG)
#define imgutil_debug(fmt, arg...) \
  printf("%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define DUMPIMGUTILCTX(cxtP) dumpImgutilCtx(ctxP)
#else
#define imgutil_debug(fmt, arg...)
#define DUMPIMGUTILCTX(cxtP)
#endif

#define imgutil_error(fmt, arg...) \
  printf("ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)

#define imgutil_info(fmt, arg...) \
  printf("%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)

/* ---- Private Function Prototypes --------------------------------------- */

static imgutil_ret_e hdrBufInit(imgutil_ctx_t *ctxP);
static imgutil_ret_e hdrBufClean(imgutil_ctx_t *ctxP);
static imgutil_ret_e imgutil_ctx_clean(imgutil_ctx_t *ctxP);

#if defined(CC_IMGUTIL_DEBUG)
static void dumpImgutilCtx(imgutil_ctx_t *ctxP);
#endif
static UINT32 getFlashType(void);
static UBOOL8 hasEnoughDataToProcImgHdr(UINT32 bufSize);
static void parseImgHdr(imgutil_ctx_t *ctxP);
static imgutil_ret_e procSingleImgHdr(imgutil_ctx_t *ctxP);
static int procComboImgHdr(imgutil_ctx_t *ctxP, UINT8 *dataP, int len,
  int *offsetP);
static int validateAndFlashConfigFile(imgutil_ctx_t *ctxP);
static int validateAndFlashWholeImage(imgutil_ctx_t *ctxP);
static imgutil_ret_e validateImageWithExtInfo(imgutil_ctx_t *ctxP,
  imgutil_img_info_t *imgInfoInP);
static void imgInfoConvert(imgutil_img_info_t *imgUtilInfoP,
  imgif_img_info_t *imgIfInfoP);
static imgutil_ret_e getIncImageVersion(char *imageVersion, UINT32 len);


/* ---- Public Variables -------------------------------------------------- */


/* ---- Private Variables ------------------------------------------------- */

static imgutil_ctx_t imgutilCtx;


/* ---- Functions --------------------------------------------------------- */

/*****************************************************************************
*  FUNCTION:  img_util_get_incflash_mode
*  PURPOSE:   Check whether incremental flashing is supported and enabled.
*  PARAMETERS:
*      None.
*  RETURNS:
*      TRUE - supported and enabled. FALSE - else.
*  NOTES:
*      None.
*****************************************************************************/
UBOOL8 img_util_get_incflash_mode(void)
{
    return imgif_get_enable_mode();
}

/*****************************************************************************
*  FUNCTION:  img_util_get_flash_info
*  PURPOSE:   Obtain flash info.
*  PARAMETERS:
       h (IN) - IMGIF context pointer.
       flashInfoP (OUT) - flash info pointer.
*  RETURNS:
*      0 - successful operation.
*      < 0 - failed operation.
*  NOTES:
*      This function may be invoked at any time after img_util_open().
*****************************************************************************/
int img_util_get_flash_info(IMGUTIL_HANDLE h, imgutil_flash_info_t *flashInfoP)
{
    imgutil_ctx_t *ctxP;

    ctxP = getCtxByHandle(h);
    if ((ctxP != NULL) && (flashInfoP != NULL))
    {
        flashInfoP->flashType = ctxP->flashType;
        flashInfoP->flashSize = ctxP->flashSize;
        flashInfoP->eraseSize = ctxP->eraseSize;
        return 0;
    }

    return -1;
}

/*****************************************************************************
*  FUNCTION:  img_util_open
*  PURPOSE:   Initialize flash write context.
*  PARAMETERS:
*      openParmsP (IN) - pointer to the open parameters.
*  RETURNS:
*      Pointer to the flash write context - successful operation.
*      NULL - failed operation, for example, another software upgrade already
*             in progress.
*  NOTES:
*      During open(), if maxBufSize is 0 for the whole flashing case,
*      attempt to malloc() maximum possible size. If not successful retry
*      with smaller sizes.
*****************************************************************************/
IMGUTIL_HANDLE img_util_open(imgutil_open_parms_t *openParmsP)
{
    imgutil_ctx_t *ctxP = NULL;
    imgutil_ret_e ret;

    if (openParmsP == NULL)
    {
        imgutil_error("Invalid paramrs");
        return IMGUTIL_HANDLE_INVALID;
    }
    
    if( bcmImg_ComboImageInit() != IMGUTIL_PROC_OK )
        return IMGUTIL_HANDLE_INVALID;

    imgutil_debug("Enter, maxBufSize=%d, forceWholeFlashB=%d, options=%d",
      openParmsP->maxBufSize, openParmsP->forceWholeFlashB, openParmsP->options);

    ctxP = getCtx();
    memset(ctxP, 0x0, sizeof(imgutil_ctx_t));

    /* Check if incremental flashing is enabled */
    if ( imgif_get_enable_mode() )
    {
        /* Retrieve flashInfo from imgif api */
        imgif_flash_info_t flashInfo;
        if( imgif_get_flash_info(&flashInfo) == 0 )
        {
            ctxP->flashType = flashInfo.flashType;
            ctxP->flashSize = flashInfo.flashSize;
            ctxP->eraseSize = flashInfo.eraseSize;
        }
        else
        {
            imgutil_error("Cant retrieve flashinfo from imgif!");
            return IMGUTIL_HANDLE_INVALID;
        }
    }
    else
    {
        ctxP->flashSize = bcmImg_getImageFlashSize();
        ctxP->flashType = getFlashType();
    }

    /* Reject the request if the estimated image size does not fit. */
    if ((openParmsP->maxBufSize != 0) && (openParmsP->maxBufSize > ctxP->flashSize))
    {
        imgutil_error("Image size %d does not fit flash size %d.",
          openParmsP->maxBufSize, ctxP->flashSize);
        return IMGUTIL_HANDLE_INVALID;
    }


    ret = hdrBufInit(ctxP);
    if (ret != IMGUTIL_PROC_OK)
    {
        imgutil_error("hdrBufInit() failed.");
        return IMGUTIL_HANDLE_INVALID;
    }


    ctxP->imgStartP = ctxP->hdrBufP;
    ctxP->currP = ctxP->hdrBufP;
    ctxP->endP = ctxP->hdrBufP + ctxP->hdrBufSize;

    ctxP->maxBufSize = openParmsP->maxBufSize;
    ctxP->forceWholeFlashB = openParmsP->forceWholeFlashB;
    ctxP->options = openParmsP->options;
    ctxP->calCrc32FuncP = openParmsP->calCrc32Func;
    ctxP->clientValidateFuncP = openParmsP->clientValidateFuncP;
    ctxP->clientFlashFuncP = openParmsP->clientFlashFuncP;
    ctxP->clientCtxP = openParmsP->clientCtxP;
    if (openParmsP->calStdCrcB == TRUE)
    {
        ctxP->calCrc32FuncP = genUtl_getCrc32Staged;
    }

    if (ctxP->calCrc32FuncP != NULL)
    {
        ctxP->calCrc32FuncP(0, &ctxP->crc, NULL, 0);
    }

    ctxP->upgradeState = IMGUTIL_STATE_HDR;

    ctxP->combo_parsing_state = combo_init;

    DUMPIMGUTILCTX(ctxP);

    return (IMGUTIL_HANDLE)ctxP;
}

/*****************************************************************************
*  FUNCTION:  img_util_write
*  PURPOSE:   Write image block to IMGUTIL.
*  PARAMETERS:
*      h (IN) - IMGUTIL context pointer.
*      dataP (IN) - image block data pointer.
*      len (IN) - image block size.
*  RETURNS:
*      >=0 - number of bytes written.
*      < 0 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
int img_util_write(IMGUTIL_HANDLE h, UINT8 *dataP, int len)
{
    imgutil_ctx_t *ctxP;
    int remainLen = 0;
    int copyLen = 0;
    int newLen = len;
    int skipTailLen = 0;
    int expectedTailLen = 0;
    int writeBytes = 0;
    int comboParseRetVal = 0;
    UINT8 *usrP = NULL;
    int offset = 0;
    imgutil_ret_e ret;

    imgutil_debug("Enter, len=%d.", len);

    if ((h == NULL) || (dataP == NULL))
    {
        imgutil_error("Invalid params");
        return -1;
    }

    ctxP = getCtxByHandle(h);

    DUMPIMGUTILCTX(ctxP);

    if (ctxP->calCrc32FuncP != NULL)
    {
        ctxP->calCrc32FuncP(1, &ctxP->crc, dataP, len);
    }

    if (!bcmImg_ComboImageParsingDone())
    {
        comboParseRetVal = procComboImgHdr(ctxP, dataP, newLen, &offset);
        if (comboParseRetVal < 0) /* Indicates acute failure. */
        {
            writeBytes = -1;
            ctxP->upgradeState = IMGUTIL_STATE_NO_IMG;
            return -1;
        }

        if (bcmImg_ComboImageParsingDone())
        {
            if (bcmImg_IsValidCombo())
            {
                imgutil_info("combo parsing is done and valid \n");
                ctxP->usrBytes += offset;
                dataP += offset;
                newLen -= offset;
                imgutil_debug("procComboImgHdr(), dataP=%p, len=%d, offset=%d",
                  dataP, newLen, offset);

                imgutil_accept_range_ctx_t *ar_ctx = (imgutil_accept_range_ctx_t*)(ctxP->clientCtxP); 

                if(ar_ctx->accept_ranges && ar_ctx->range_begin)
                {
                    imgutil_info("Server supports Range. Need to restart download with Range header. exiting img_util_write\n");
                    ctxP->usrBytes = 0;
                    ctxP->crc = 0;
                    //i think the next line should be removed
                    //ctxP->bundleHdrExistB = FALSE;
                    //bcmImg_ComboImageInit();
                    goto write_exit;
                }
            }
            else
            {
                imgutil_info("This isn't a combo image, proceed to test other image types\n");
                /* Not combo, continue. */
            }
        }
        else /* Still parsing header or rolling to the image. */
        {
            ctxP->usrBytes += newLen;
            writeBytes = newLen;
            offset = 0;
            goto write_exit;
        }
    }

    /*
     * If the expected image is received completely, no need to buffer
     * following images.
     */
    if ((ctxP->upgradeState == IMGUTIL_STATE_IMG) &&
      (ctxP->bundleHdrExistB == TRUE))
    {
        if ((ctxP->accImgSize + newLen) >= ctxP->indiv_hdr_tag.image_len)
        {
            expectedTailLen = ctxP->indiv_hdr_tag.image_len - ctxP->accImgSize;
            skipTailLen = newLen - expectedTailLen;
            offset += skipTailLen;
            newLen = expectedTailLen;
            if (newLen == 0)
            {
                goto write_exit;
            }
        }
    }

    remainLen = newLen;

    /* Incremental flashing. */
    if ((ctxP->upgradeState == IMGUTIL_STATE_IMG) &&
      (ctxP->imgifHandle != NULL))
    {
        writeBytes = imgif_write(ctxP->imgifHandle, dataP, newLen);
        if ((writeBytes < 0) || (writeBytes != newLen))
        {
            imgutil_error("imgif_write() failed, towrite=%d, written=%d",
              newLen, writeBytes);
        }
        else
        {
            ctxP->accImgSize += newLen;
            ctxP->usrBytes += newLen;
        }

        return (writeBytes + offset);
    }

    usrP = dataP;
    do
    {
        copyLen = ((ctxP->currP + remainLen) > ctxP->endP) ?
          (ctxP->endP - ctxP->currP) : remainLen;
        imgutil_debug("Copy from user, usrP=%p, copyLen=%d.", usrP, copyLen);
        if (copyLen == 0)
        {
            break;
        }

        /* Either a header buffer or an image buffer. */
        copyDataToBucket(ctxP, usrP, copyLen);

        usrP += copyLen;
        remainLen -= copyLen;
        writeBytes += copyLen;

        if ((ctxP->upgradeState == IMGUTIL_STATE_HDR) &&
          (hasEnoughDataToProcImgHdr(ctxP->accImgSize) == TRUE))
        {
            parseImgHdr(ctxP);
            ret = procSingleImgHdr(ctxP);
            if (ret != IMGUTIL_PROC_OK)
            {
                /* Abort. */
                writeBytes = -1;
                break;
            }
        }

        DUMPIMGUTILCTX(ctxP);

    } while (remainLen > 0);

    if (writeBytes != newLen)
    {
        imgutil_error("Error, len=%d, written=%d", newLen, writeBytes);
    }

write_exit:
    imgutil_debug("Exit, len=%d, written=%d", len, writeBytes + offset);

    DUMPIMGUTILCTX(ctxP);

    return (writeBytes + offset);
}

/*****************************************************************************
*  FUNCTION:  img_util_close
*  PURPOSE:   Close IMGUTIL context in successful condition.
*  PARAMETERS:
*      h (IN) - IMGUTIL context pointer.
*      imgInfoInP (IN) - image info obtained externally. The client protocol
*        may provide additional image integrity (size, CRC) information at
*        the end of the upgrade.
*      imgInfoOutP (OUT) - info (e.g. version) obtained from the image,
*        either from the bundle header or from the image itself. A client
*        may need the information.
*  RETURNS:
*      0 - successful operation.
*      < 0 - failed operation.
*  NOTES:
*      The client should provide callback functions for the configuration
*      file validation and upgrade, which are tightly coupled with CMS.
*
*      The flashing time for a whole image can be long (> 10s). Certain
*      client protocol specification (e.g. OMCI) expects timely responses
*      from the client agent during the flashing operation. To avoid the
*      blocking condition, the client may choose to invoke this API from a
*      different thread.
*****************************************************************************/
int img_util_close(IMGUTIL_HANDLE h, imgutil_img_info_t *imgInfoInP,
  imgutil_img_info_t *imgInfoOutP)
{
    imgutil_ctx_t *ctxP;
    imgutil_ret_e ret = IMGUTIL_PROC_OK;
    int rc = -1;

    if (h == NULL)
    {
        imgutil_error("Invalid params");
        return -1;
    }

    imgutil_debug("Enter, imgInfoInP=%p, imgInfoOutP=%p",
      imgInfoInP, imgInfoOutP);

    ctxP = getCtxByHandle(h);

    DUMPIMGUTILCTX(ctxP);

    if (ctxP->upgradeState == IMGUTIL_STATE_NO_IMG)
        goto close_exit;

    if (ctxP->upgradeState != IMGUTIL_STATE_IMG)
    {
        imgutil_error("Invalid state");
        goto close_exit;
    }

    if (ctxP->calCrc32FuncP != NULL)
    {
        ctxP->calCrc32FuncP(2, &ctxP->crc, NULL, 0);
    }

    if (imgInfoInP != NULL)
    {
        ret = validateImageWithExtInfo(ctxP, imgInfoInP);
        if (ret != IMGUTIL_PROC_OK)
        {
            imgutil_error("validateImageWithExtInfo() failed");
            goto close_exit;
        }
    }

    if (ctxP->wholeFlashB == TRUE)
    {
        if (ctxP->imgFormat == CMS_IMAGE_FORMAT_XML_CFG)
        {
            if ((ctxP->clientValidateFuncP == NULL) ||
              (ctxP->clientFlashFuncP == NULL))
            {
                imgutil_error("Invalid client function for config file");
                goto close_exit;
            }

            rc = validateAndFlashConfigFile(ctxP);
        }
        else
        {
            rc = validateAndFlashWholeImage(ctxP);
        }
    }
    else
    {
        /* Incrementatl flashing. */
        rc = imgif_close(ctxP->imgifHandle, 0);
    }

    if ((rc >= 0) && (imgInfoOutP != NULL))
    {
        imgInfoOutP->bitmask = IMG_INFO_BITMASK_SIZE;
        imgInfoOutP->size = ctxP->usrBytes;
    }

close_exit:
    imgutil_ctx_clean(ctxP);

    return rc;
}

/*****************************************************************************
*  FUNCTION:  img_util_abort
*  PURPOSE:   Close IMGUTIL context in error condition.
*  PARAMETERS:
*      h (IN) - IMGUTIL context pointer.
*  RETURNS:
*      0 - successful operation.
*      < 0 - failed operation.
*  NOTES:
*      Client may stop an upgrade at any time, when there is an operator
*      command, protocol failure, or unmatched image size and/or CRC values
*      between IMGUTIL and client.
*****************************************************************************/
int img_util_abort(IMGUTIL_HANDLE h)
{
    imgutil_ctx_t *ctxP;
    int rc = 0;

    imgutil_debug("Enter");

    if (h == NULL)
    {
        imgutil_error("Invalid params");
        return -1;
    }

    ctxP = getCtxByHandle(h);
    DUMPIMGUTILCTX(ctxP);

    if (ctxP->imgifHandle != NULL)
    {
        rc = imgif_close(ctxP->imgifHandle, 1);
    }

    imgutil_ctx_clean(ctxP);
    imgutil_info("rc = %d", rc);

    return rc;
}

/*****************************************************************************
*  FUNCTION:  img_util_get_imginfo
*  PURPOSE:   Get image information from library.
*  PARAMETERS:
*      h (IN) - IMGUTIL context pointer.
*      bitmask (IN) - image info bitmask.
*      imgInfoP (OUT) - image info pointer.
*  RETURNS:
*      0 - successful operation.
*      < 0 - failed operation.
*  NOTES:
*      This function may be invoked right before img_util_close(), after last
*      img_util_write().
*****************************************************************************/
int img_util_get_imginfo(IMGUTIL_HANDLE h, imgutil_info_src_e src,
  UINT32 bitmask, imgutil_img_info_t *imgInfoP)
{
    imgutil_ctx_t *ctxP;
    imgutil_ret_e ret;
    UINT32 finalCrc;
    int rc = 0;

    imgutil_debug("Enter");

    if ((h == NULL) || (imgInfoP == NULL))
    {
        imgutil_error("Invalid params");
        return -1;
    }

    ctxP = getCtxByHandle(h);

    if (src == IMGINFO_SRC_CALCOMBO)
    {
        if (bitmask & IMGUTIL_IMGINFO_BIT_SIZE)
        {
            imgInfoP->bitmask |= IMGUTIL_IMGINFO_BIT_SIZE;
            imgInfoP->size = ctxP->usrBytes;
        }

        if (bitmask & IMGUTIL_IMGINFO_BIT_CRC)
        {
            if (ctxP->calCrc32FuncP != NULL)
            {
                finalCrc = ctxP->crc;
                ctxP->calCrc32FuncP(2, &finalCrc, NULL, 0);
                imgInfoP->bitmask |= IMGUTIL_IMGINFO_BIT_CRC;
                imgInfoP->crc = finalCrc;
            }
        }
    }
    else if (src == IMGINFO_SRC_BLTIN)
    {
        if (bitmask & IMGUTIL_IMGINFO_BIT_VERSION)
        {
            if (ctxP->wholeFlashB == TRUE)
            {
                ret = bcmImg_getImageVersion((char*)ctxP->imgBufP,
                  ctxP->accImgSize, (char*)imgInfoP->version,
                  IMGUTIL_VERSION_LEN);
                if (ret == IMGUTIL_PROC_OK)
                {
                    imgInfoP->bitmask |= IMGUTIL_IMGINFO_BIT_VERSION;
                }
                else
                {
                    imgutil_error("bcmImg_getImageVersion() failed.");
                    rc = -1;
                }
            }
            else
            {
                getIncImageVersion((char*)imgInfoP->version,
                  IMGUTIL_VERSION_LEN);
            }
        }
    }

   return rc;
}


/* ---- Private Functions ------------------------------------------------- */

/* Allocate a small buffer to store the image header or the whole config. */
static imgutil_ret_e hdrBufInit(imgutil_ctx_t *ctxP)
{
    UINT32 hdrBufSize;
    UINT32 configFlashSize;

    configFlashSize = bcmImg_getConfigFlashSize();
    hdrBufSize = (configFlashSize > IMGUTIL_HDR_BUF_SIZE)?
      configFlashSize : IMGUTIL_HDR_BUF_SIZE;

    ctxP->hdrBufP = malloc(hdrBufSize);
    if (ctxP->hdrBufP == NULL)
    {
        imgutil_error("malloc(%d) failed.", hdrBufSize);
        goto init_exit;
    }

    ctxP->hdrBufSize = hdrBufSize;
    return IMGUTIL_PROC_OK;

init_exit:
    ctxP->hdrBufSize = 0;
    return IMGUTIL_PROC_ERR;
}

static imgutil_ret_e hdrBufClean(imgutil_ctx_t *ctxP)
{
    if (ctxP->hdrBufP != NULL)
    {
        free(ctxP->hdrBufP);
        ctxP->hdrBufP = NULL;
    }

    ctxP->hdrBufSize = 0;
    return IMGUTIL_PROC_OK;
}

/* Allocate the buffer to store the whole image. */
static imgutil_ret_e imgBufInit(imgutil_ctx_t *ctxP)
{
    UINT8 *bufP = NULL;
    UINT32 bufSize;

    bufP = bcmImg_allocImgBuf(ctxP->maxBufSize, &bufSize);
    if (bufP == NULL)
    {
        imgutil_error("bcmImg_allocImgBuf(%d) failed.", ctxP->maxBufSize);
        goto init_exit;
    }

    printf("imgutil: allocated %d byte buffer to hold image.", bufSize);
    ctxP->imgBufP = bufP;
    ctxP->imgBufSize = bufSize;
    return IMGUTIL_PROC_OK;

init_exit:
    ctxP->imgBufSize = 0;
    return IMGUTIL_PROC_ERR;
}

static imgutil_ret_e imgBufClean(imgutil_ctx_t *ctxP)
{
    if (ctxP->imgBufP != NULL)
    {
        free(ctxP->imgBufP);
        ctxP->imgBufP = NULL;
    }

    ctxP->imgBufSize = 0;
    return IMGUTIL_PROC_OK;
}

static imgutil_ret_e imgutil_ctx_clean(imgutil_ctx_t *ctxP)
{
    hdrBufClean(ctxP);
    imgBufClean(ctxP);

    memset(ctxP, 0x0, sizeof(imgutil_ctx_t));

    DUMPIMGUTILCTX(ctxP);

    return IMGUTIL_PROC_OK;
}

#if defined(CC_IMGUTIL_DEBUG)
static void dumpImgutilCtx(imgutil_ctx_t *ctxP)
{
    printf("--------------------------------------------------------------\n");
    printf("\tFlash:      type=%d, size=%d, eraseSize=%d\n",
      ctxP->flashType, ctxP->flashSize, ctxP->eraseSize);
    printf("\tUser input: maxBufSize=%d, forceWholeFlashB=%d, options=%d\n",
      ctxP->maxBufSize, ctxP->forceWholeFlashB, ctxP->options);
    printf("\tBuffer:     hdrBufP=%p, hdrBufSize=%d, imgBufP=%p, imgBufSize=%d\n",
      ctxP->hdrBufP, ctxP->hdrBufSize, ctxP->imgBufP, ctxP->imgBufSize);
    printf("\tPointer:    currP=%p, endP=%p\n", ctxP->currP, ctxP->endP);
    printf("\tUpgrade:    format=%d, wholeFlashB=%d, state=%d, usrBytes=%d, "
      "accImgSize=%d\n",
      ctxP->imgFormat, ctxP->wholeFlashB, ctxP->upgradeState, ctxP->usrBytes,
      ctxP->accImgSize);
    if (ctxP->bundleHdrExistB)
    {
        printf("\tCombo Hdr:  len=%d, crc=0x%08x, count=%d, "
          "flags=0x%08x, noffset=%d, ext=%d\n",
          ctxP->combo_hdr_tag.header_len, ctxP->combo_hdr_tag.header_crc,
          ctxP->combo_hdr_tag.image_count, ctxP->combo_hdr_tag.header_flags,
          ctxP->combo_hdr_tag.next_tag_offset,
          ctxP->combo_hdr_tag.extended_combo_header);
        printf("\tIndiv Hdr:  chipid=0x%x, imageLen=%d, offset=%d, "
          "flags=0x%08x, noffset=%d, ext=%d\n",
          ctxP->indiv_hdr_tag.chip_id, ctxP->indiv_hdr_tag.image_len,
          ctxP->indiv_hdr_tag.image_offset, ctxP->indiv_hdr_tag.image_flags,
          ctxP->indiv_hdr_tag.next_tag_offset,
          ctxP->indiv_hdr_tag.extended_image_header);
    }
    printf("--------------------------------------------------------------\n");
}
#endif

UINT32 getFlashType(void)
{
    unsigned int flags = 0;
    UINT32 flashType = 0;
    int ret;

    ret = getFlashInfo(&flags);
    if (ret >= 0) /* OK */
    {
        flashType = flags;
    }
    else
    {
        imgutil_error("getFlashInfo() failed, ret=%d.", ret);
    }

    return flashType;
}

int validateAndFlashConfigFile(imgutil_ctx_t *ctxP)
{
    int rc = 0;

    imgutil_debug("Enter, bufP=%p, size=%d", ctxP->imgStartP, ctxP->accImgSize);

    /* Config file always in hdrBufP. */
    rc = ctxP->clientValidateFuncP(ctxP->imgStartP, ctxP->accImgSize,
      ctxP->clientCtxP);
    if (rc < 0)
    {
        imgutil_error("clientValidateFuncP() failed");
        return -1;
    }

    rc = ctxP->clientFlashFuncP(ctxP->imgStartP, ctxP->accImgSize,
      ctxP->clientCtxP);
    if (rc < 0)
    {
        imgutil_error("clientFlashFuncP() failed");
        return -1;
    }

    return rc;
}

static int validateAndFlashWholeImage(imgutil_ctx_t *ctxP)
{
    int noReboot = 0;
    CmsRet cmsRet;

    imgutil_debug("Enter, bufP=%p, size=%d, format=%d, options=%d",
      ctxP->imgBufP, ctxP->accImgSize, ctxP->imgFormat, ctxP->options);

    ctxP->imgFormat = bcmImg_validateImage((char*)ctxP->imgBufP,
      ctxP->accImgSize);
    if (ctxP->imgFormat == CMS_IMAGE_FORMAT_INVALID)
    {
        imgutil_error("bcmImg_validateImage() failed");
        return -1;
    }

    cmsRet = bcmImg_flashImage((char*)ctxP->imgBufP, ctxP->accImgSize,
      ctxP->imgFormat, ctxP->options, &noReboot);
    if (cmsRet != CMSRET_SUCCESS)
    {
        imgutil_error("bcmImg_flashImage() failed");
        return -1;
    }

    printf("Image flash done\n");
    imgutil_debug("noReboot=%d", noReboot);
    if (noReboot == 0)
    {
        imgutil_debug("bcmImg_reboot()");
        bcmImg_reboot();
    }

    return 0;
}

static UBOOL8 hasEnoughDataToProcImgHdr(UINT32 bufSize)
{
    UBOOL8 enough = FALSE;

    if ((bufSize >= sizeof(FILE_TAG)) &&
      (bufSize >= CMS_CONFIG_FILE_DETECTION_LENGTH) &&
      (bufSize >= sizeof(Comboimg_header_tag)))
    {
        enough = TRUE;
    }

    return enough;
}

/*
 * Scan through the combo image till reach the expected individual image,
 * pass the single image pointer back to caller.
 */
static int procComboImgHdr(imgutil_ctx_t *ctxP, UINT8 *dataP, int len,
  int *offsetP)
{
    UINT32 imageLen;
    UINT32 consumed;
    imgutil_ret_e utilRet;

    utilRet = bcmImg_ComboImageParseHeader((char*)dataP,
      len, &consumed, &imageLen, &ctxP->combo_parsing_state,
      &(ctxP->combo_hdr_tag), &(ctxP->indiv_hdr_tag), ctxP->clientCtxP);
    
    if (utilRet == IMGUTIL_PROC_NOT_FOUND)
       return -1; 
    else if (utilRet != IMGUTIL_PROC_OK)
    {
        imgutil_error("bcmImg_ComboImageParseHeader() failed,"
           "dataP=%p, len=%d, consumed=%d, imageLen=%d, state=%d",
           dataP, len, consumed, imageLen, ctxP->combo_parsing_state);
        return -1;
    }

    if (bcmImg_IsValidCombo())
    {
        ctxP->bundleHdrExistB = TRUE;
    }

    *offsetP = consumed;
    return 0;
}

/*
 * This function is used to parse a single image only (standalone single
 * image or an individual image inside the combo.
 */
static void parseImgHdr(imgutil_ctx_t *ctxP)
{
    CmsImageFormat imgFormat = CMS_IMAGE_FORMAT_FLASH;

    imgutil_debug("Enter, imgStartP=%p, accImgSize=%d",
      ctxP->imgStartP, ctxP->accImgSize);

    imgFormat = bcmImg_parseImgHdr(ctxP->imgStartP, ctxP->accImgSize);
    ctxP->imgFormat = imgFormat;
    ctxP->upgradeState = IMGUTIL_STATE_IMG;

    imgutil_debug("Exit, imgStartP=%p, accImgSize=%d",
      ctxP->imgStartP, ctxP->accImgSize);
}

/*
 * This function is used to process a single image only (standalone single
 * image or an individual image inside the combo.
 */
static imgutil_ret_e procSingleImgHdr(imgutil_ctx_t *ctxP)
{
    int byteCount;

    if (ctxP->imgFormat == CMS_IMAGE_FORMAT_XML_CFG)
    {
        /* Do nothing, the hdrBuf is sufficient for XML_CFG. */
        ctxP->wholeFlashB = TRUE;
        goto proc_exit;
    }

    /* Only apply forceWholeFlashB flag to CMS_IMAGE_FORMAT_FLASH. */
    if ((ctxP->forceWholeFlashB == TRUE) ||
      (ctxP->flashType == FLASH_INFO_FLAG_NOR))
    {
        ctxP->wholeFlashB = TRUE;
    }

    if (ctxP->wholeFlashB == TRUE)
    {
        if (imgBufInit(ctxP) != IMGUTIL_PROC_OK)
        {
            imgutil_error("imgBufInit() failed.");
            return IMGUTIL_PROC_ERR;
        }

        /* Copy the content from header buffer to image buffer. */
        memcpy(ctxP->imgBufP, ctxP->imgStartP, ctxP->accImgSize);
        ctxP->currP = ctxP->imgBufP + ctxP->accImgSize;
        ctxP->endP = ctxP->imgBufP + ctxP->imgBufSize;

        /* Header buffer is not needed from this point on. */
        hdrBufClean(ctxP);
        ctxP->imgStartP = ctxP->imgBufP;
    }
    else
    {
        /* Call incremental flashing API, flush out accumulated buffer. */
        ctxP->imgifHandle = imgif_open(NULL, ctxP->calCrc32FuncP);
        if (ctxP->imgifHandle == NULL)
        {
            imgutil_error("imgif_open() failed.");
            return IMGUTIL_PROC_ERR;
        }

        byteCount = imgif_write(ctxP->imgifHandle, (UINT8*)ctxP->imgStartP,
          ctxP->accImgSize);
        if ((byteCount < 0) || ((UINT32)byteCount != ctxP->accImgSize))
        {
            imgutil_error("imgif_write() failed, towrite=%d, written=%d",
              ctxP->accImgSize, byteCount);
        }

        /* Header buffer is not needed from this point on. */
        hdrBufClean(ctxP);
        ctxP->currP = NULL;
        ctxP->endP = NULL;
    }

proc_exit:
    return IMGUTIL_PROC_OK;
}

imgutil_ret_e validateImageWithExtInfo(imgutil_ctx_t *ctxP,
  imgutil_img_info_t *imgInfoInP)
{
    imgutil_ret_e rc = IMGUTIL_PROC_OK;

    if ((imgInfoInP->bitmask & IMG_INFO_BITMASK_SIZE) &&
      (imgInfoInP->size != ctxP->usrBytes))
    {
        imgutil_error("Unmatched file size values, ext=%d, cal=%d",
          imgInfoInP->size, ctxP->usrBytes);
        rc = IMGUTIL_PROC_ERR;
    }

    if ((ctxP->bundleHdrExistB == FALSE) &&
      (ctxP->imgifHandle != NULL))
    {
        imgif_img_info_t imgIfInfo;

        memset(&imgIfInfo, 0x0, sizeof(imgIfInfo));
        imgInfoConvert(imgInfoInP, &imgIfInfo);
        imgif_set_image_info(ctxP->imgifHandle, &imgIfInfo);
    }

    return rc;
}

static void imgInfoConvert(imgutil_img_info_t *imgUtilInfoP,
  imgif_img_info_t *imgIfInfoP)
{
    imgIfInfoP->bitmask |= (imgUtilInfoP->bitmask & IMGUTIL_IMGINFO_BIT_SIZE) ?
      IMG_INFO_BITMASK_SIZE : 0;
    imgIfInfoP->bitmask |= (imgUtilInfoP->bitmask & IMGUTIL_IMGINFO_BIT_CRC) ?
      IMG_INFO_BITMASK_CRC : 0;
    imgIfInfoP->size = imgUtilInfoP->size;
    imgIfInfoP->crc = imgUtilInfoP->crc;
}

/* Retrieve the image version in incremental flashing case. */
static imgutil_ret_e getIncImageVersion(char *imageVersion, UINT32 len)
{
    FILE *fp;
    imgutil_ret_e ret = IMGUTIL_PROC_ERR;

    fp = fopen("/tmp/imageIdent", "r");
    if (fp)
    {
        if (fgets(imageVersion, len, fp) != NULL)
        {
            ret = IMGUTIL_PROC_OK;
        }
        fclose(fp);
    }

    return ret;
}
