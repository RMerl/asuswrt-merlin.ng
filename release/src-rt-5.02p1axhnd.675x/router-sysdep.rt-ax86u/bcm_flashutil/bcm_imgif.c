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
#include "bcm_flashutil.h"
#include "bcm_imgif.h"
#include "bcm_imgif_nand.h"
#include "bcm_imgif_emmc.h"

/* ---- Constants and Types ----------------------------------------------- */
/* IMGIF context. */
typedef struct
{
    IMGIF_HANDLE (*imgif_open_cb)(IMG_FORMAT_PARSER_CB fmtParserCb,
      CAL_CRC32_CB calCrc32Cb);
    int (*imgif_write_cb)(IMGIF_HANDLE h, UINT8 *dataP, int len);
    int (*imgif_close_cb)(IMGIF_HANDLE h, UBOOL8 abortFlag);
    int (*imgif_set_image_info_cb)(IMGIF_HANDLE h, imgif_img_info_t *imgInfoP);
    int (*imgif_get_flash_info_cb)(imgif_flash_info_t *flashInfoP);
    unsigned int flash_type;
} imgif_ctx_t;

/* ---- Private Function Prototypes --------------------------------------- */
/* ---- Public Variables -------------------------------------------------- */
/* ---- Private Variables ------------------------------------------------- */
static imgif_ctx_t imgif_ctx = {NULL, NULL, NULL, NULL, NULL, 0};
/* ---- Functions --------------------------------------------------------- */

/*****************************************************************************
*  FUNCTION:  imgif_init
*  PURPOSE:   Initialize imgif callbacks.
*  PARAMETERS:
*      None.
*  RETURNS:
*      TRUE - supported and enabled. FALSE - else.
*  NOTES:
*      None.
*****************************************************************************/
static void imgif_init(void)
{
    /* Ignore redundant calls to this function */
    if( imgif_ctx.flash_type )
        return;

    if( getFlashInfo(&imgif_ctx.flash_type) >= 0 )
    {
        switch( imgif_ctx.flash_type )
        {
#ifdef SUPPORT_INCREMENTAL_FLASHING
            case FLASH_INFO_FLAG_NAND:
                imgif_ctx.imgif_open_cb = imgif_nand_open;
                imgif_ctx.imgif_write_cb = imgif_nand_write;
                imgif_ctx.imgif_close_cb = imgif_nand_close;
                imgif_ctx.imgif_set_image_info_cb = imgif_nand_set_image_info;
                imgif_ctx.imgif_get_flash_info_cb = imgif_nand_get_flash_info;
                printf("imgif: Enabling NAND incremental flashing functions!\n");
            break;
            
            case FLASH_INFO_FLAG_EMMC:
                imgif_ctx.imgif_open_cb = imgif_emmc_open;
                imgif_ctx.imgif_write_cb = imgif_emmc_write;
                imgif_ctx.imgif_close_cb = imgif_emmc_close;
                imgif_ctx.imgif_set_image_info_cb = imgif_emmc_set_image_info;
                imgif_ctx.imgif_get_flash_info_cb = imgif_emmc_get_flash_info;
                printf("imgif: Enabling EMMC incremental flashing functions!\n");
            break;
#endif
            default:
                printf("imgif: Incremental flashing not supported!\n");
            break;
        }
    }
    else
    {
        printf("imgif: Error! Cannot determine flash type!\n");
    }
        
}


/*****************************************************************************
*  FUNCTION:  imgif_open
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
IMGIF_HANDLE imgif_open(IMG_FORMAT_PARSER_CB fmtParserCb,
  CAL_CRC32_CB calCrc32Cb)
{
    if( !imgif_ctx.flash_type )
        imgif_init();

    if( imgif_ctx.imgif_open_cb )
        return (imgif_ctx.imgif_open_cb( fmtParserCb, calCrc32Cb));
    else
    {
        printf("imgif: Error! imgif not initialized!\n");
        return(NULL);
    }
}

/*****************************************************************************
*  FUNCTION:  imgif_write
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
int imgif_write(IMGIF_HANDLE h, UINT8 *dataP, int len)
{
    if( imgif_ctx.imgif_write_cb )
        return (imgif_ctx.imgif_write_cb( h, dataP, len ));
    else
    {
        printf("imgif: Error! imgif not initialized!\n");
        return(-1);
    }
}

/*****************************************************************************
*  FUNCTION:  imgif_close
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
int imgif_close(IMGIF_HANDLE h, UBOOL8 abortFlag)
{
    if( imgif_ctx.imgif_close_cb )
        return (imgif_ctx.imgif_close_cb( h, abortFlag ));
    else
    {
        printf("imgif: Error! imgif not initialized!\n");
        return(-1);
    }
}

/*****************************************************************************
*  FUNCTION:  imgif_set_image_info
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
int imgif_set_image_info(IMGIF_HANDLE h, imgif_img_info_t *imgInfoExtP)
{
    if( !imgif_ctx.flash_type )
        imgif_init();

    if( imgif_ctx.imgif_set_image_info_cb )
        return (imgif_ctx.imgif_set_image_info_cb( h, imgInfoExtP ));
    else
    {
        printf("imgif: Error! imgif not initialized!\n");
        return(-1);
    }
}

/*****************************************************************************
*  FUNCTION:  imgif_get_flash_info
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
int imgif_get_flash_info(imgif_flash_info_t *flashInfoP)
{
    if( !imgif_ctx.flash_type )
        imgif_init();

    if( imgif_ctx.imgif_get_flash_info_cb )
        return (imgif_ctx.imgif_get_flash_info_cb( flashInfoP ));
    else
    {
        printf("imgif: Error! imgif not initialized!\n");
        return(-1);
    }
}

/*****************************************************************************
*  FUNCTION:  imgif_get_enable_mode
*  PURPOSE:   Check whether incremental flashing is supported and enabled.
*  PARAMETERS:
*      None.
*  RETURNS:
*      TRUE - supported and enabled. FALSE - else.
*  NOTES:
*      None.
*****************************************************************************/
UBOOL8 imgif_get_enable_mode(void)
{
    UBOOL8 mode = FALSE;

    if( !imgif_ctx.flash_type )
        imgif_init();

    switch( imgif_ctx.flash_type )
    {
        case FLASH_INFO_FLAG_NAND:
        case FLASH_INFO_FLAG_EMMC:
#ifdef SUPPORT_INCREMENTAL_FLASHING
            mode = TRUE;
#endif /* SUPPORT_INCREMENTAL_FLASHING */
        break;

        default:
            mode = FALSE;
        break;
    }

    return mode;
}

