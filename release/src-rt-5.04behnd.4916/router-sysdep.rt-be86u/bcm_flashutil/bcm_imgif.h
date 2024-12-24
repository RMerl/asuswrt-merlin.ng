/*****************************************************************************
 *
 *  Copyright (c) 2015  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2015:DUAL/GPL:standard
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
:>
 *
 ****************************************************************************/

#ifndef BCM_IMGIF_H
#define BCM_IMGIF_H

/* ---- Include Files ----------------------------------------------------- */

#include "bcm_imgutil_def.h"


/* ---- Constants and Types ----------------------------------------------- */
#define IMGIF_API_VERSION      (1)

typedef void *IMGIF_HANDLE;

/*
 * Image information. The source may be one of the following:
 * - Built-in attribute included in the image.
 * - Derived/calculated from the image.
 * - Obtained externally.
 */
typedef struct
{
#define IMG_INFO_BITMASK_SIZE 0x1
#define IMG_INFO_BITMASK_CRC  0x2
    UINT32 bitmask;
    UINT32 size;
    UINT32 crc;
} imgif_img_info_t;

typedef struct
{
    UINT32 flashType;
    UINT32 flashSize;
    UINT32 eraseSize;
} imgif_flash_info_t;


typedef enum
{
    IMGIF_UPGRADE_IDLE = 0,
    IMGIF_UPGRADE_SEARCH,       /* Search image header, cferom, cferam, etc. */
    IMGIF_UPGRADE_FS,           /* Flashing. */
    IMGIF_UPGRADE_FS_FINAL,
    IMGIF_UPGRADE_CFEROM,
} imgif_state_e;

typedef enum
{
    IMGIF_PROC_OK = 0,
    IMGIF_PROC_ERR,
    IMGIF_PROC_NOT_FOUND,
    IMGIF_PROC_FOUND
} imgif_ret_e;

typedef imgutil_img_format_e (*IMG_FORMAT_PARSER_CB)(UINT8 *bufP,
  UINT32 bufLen);
typedef void (*CAL_CRC32_CB)(UINT32 stage, UINT32 *crcInP, UINT8 *bufP,
  UINT32 bufLen);


/* ---- Variable Externs -------------------------------------------------- */


/* ---- Function Prototypes ----------------------------------------------- */
IMGIF_HANDLE imgif_open(IMG_FORMAT_PARSER_CB fmtParserCb,
  CAL_CRC32_CB calCrc32Cb);
int imgif_write(IMGIF_HANDLE h, UINT8 *dataP, int len);
int imgif_close(IMGIF_HANDLE h, UBOOL8 abortFlag);
int imgif_set_image_info(IMGIF_HANDLE h, imgif_img_info_t *imgInfoP);
int imgif_get_flash_info(imgif_flash_info_t *flashInfoP);
UBOOL8 imgif_get_enable_mode(void);


#endif /* CMS_IMGIF_H */
