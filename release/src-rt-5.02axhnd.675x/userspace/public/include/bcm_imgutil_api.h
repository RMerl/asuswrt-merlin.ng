/*****************************************************************************
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
:>
 *
 ****************************************************************************/

#ifndef BCM_IMGUTIL_API_H
#define BCM_IMGUTIL_API_H

/* ---- Include Files ----------------------------------------------------- */

#include "os_defs.h"
#include "bcmTag.h"
#include "bcm_imgutil_def.h"


/* ---- Constants and Types ----------------------------------------------- */

#define IMGUTIL_VERSION_LEN     0x20

typedef void *IMGUTIL_HANDLE;

/* Write to partition 1 or 2 (remove the similar definition from CMS). */
typedef enum
{
    IMGUTIL_WR_OPT_AUTO = 0,
    IMGUTIL_WR_OPT_PART1 = 0x10,
    IMGUTIL_WR_OPT_PART2 = 0x20
} imgutil_wr_opt_e;

/*
 * Image information. The source may be one of the following:
 * - Built-in attributes included in the image or image header.
 * - Derived/calculated from the image (combo or individual).
 * - From client.
 */
typedef enum
{
    IMGINFO_SRC_BLTIN    = 0x01,
    IMGINFO_SRC_IMGHDR   = 0x02,
    IMGINFO_SRC_CALINDIV = 0x03,
    IMGINFO_SRC_CALCOMBO = 0x04,
    IMGINFO_SRC_EXT      = 0x05
} imgutil_info_src_e;

typedef struct
{
#define IMGUTIL_IMGINFO_BIT_SIZE    0x1
#define IMGUTIL_IMGINFO_BIT_CRC     0x2
#define IMGUTIL_IMGINFO_BIT_FORMAT  0x4
#define IMGUTIL_IMGINFO_BIT_VERSION 0x8
    UINT32 bitmask;
    UINT32 size;
    UINT32 crc;
    UINT32 imgFormat;
    UINT8 version[IMGUTIL_VERSION_LEN];
} imgutil_img_info_t;

typedef enum
{
    IMGUTIL_FLASH_TYPE_NOR  = 0x01,
    IMGUTIL_FLASH_TYPE_NAND = 0x02
} imgutil_flash_type_e;

typedef struct
{
    /* As defined in imgutil_flash_type_e. */
    UINT32 flashType;
    /* Total size (in bytes) of the flash device. */
    UINT32 flashSize;
    /* Erase block size (in bytes). */
    UINT32 eraseSize;
} imgutil_flash_info_t;

typedef enum
{
    IMGUTIL_ERR_BHDR_INVALID = -2,
    IMGUTIL_ERR_FLASH_UNMATCH = -3,
    IMGUTIL_ERR_MEM_LIMIT = -4,     /* No enough memory to hold the image. */
    IMGUTIL_ERR_IMG_INVALID = -5,   /* CRC, size error, etc. */
    IMGUTIL_ERR_OPER_FAILED = -6
} imgutil_error_code_e;

#define IMGUTIL_HANDLE_INVALID  ((IMGUTIL_HANDLE)NULL)

typedef void (*IMGUTIL_CAL_CRC32_FUNC)(UINT32 stage, UINT32 *crcInP,
  UINT8 *bufP, UINT32 bufLen);
typedef imgutil_img_format_e (*IMGUTIL_VALIDATE_IMG_FUNC)(UINT8 *imageP,
  UINT32 imageLen, void *usrDataP);
typedef int (*IMGUTIL_WRITE_IMG_FUNC)(UINT8 *imageP, UINT32 imageLen,
  void *usrDataP);

/*
 * maxBufSize - 0: estimate the max image size from flash; non-zero:
 *   client-specified value.
 * forceWholeFlashB - 1: force whole image upgrade, 0 otherwise (flashing mode
 *   is determined by image and flash type).
 * options - options related to partition.
 * calCrc32Func - client-provided CRC algorithm callback function, if it is
 *   different from the built-in CRC32.
 * clientValidateFuncP - client-provided image validation function.
 * clientFlashFuncP - client-provided image flashing function.
 * clientCtxP - client-provided context.
 */
typedef struct
{
    UINT32 maxBufSize;
    UBOOL8 forceWholeFlashB;
    UBOOL8 calStdCrcB;
    imgutil_wr_opt_e options;
    IMGUTIL_CAL_CRC32_FUNC calCrc32Func;
    IMGUTIL_VALIDATE_IMG_FUNC clientValidateFuncP;
    IMGUTIL_WRITE_IMG_FUNC clientFlashFuncP;
    void *clientCtxP;
} imgutil_open_parms_t;


/* ---- Variable Externs -------------------------------------------------- */


/* ---- Function Prototypes ----------------------------------------------- */

UBOOL8 img_util_get_incflash_mode(void);

int img_util_get_flash_info(IMGUTIL_HANDLE h, imgutil_flash_info_t *flashInfoP);

IMGUTIL_HANDLE img_util_open(imgutil_open_parms_t *openParmsP);

int img_util_write(IMGUTIL_HANDLE h, UINT8 *dataP, int len);

int img_util_close(IMGUTIL_HANDLE h, imgutil_img_info_t *imgInfoInP,
  imgutil_img_info_t *imgInfoOutP);

int img_util_abort(IMGUTIL_HANDLE h);

int img_util_get_imginfo(IMGUTIL_HANDLE h, imgutil_info_src_e src,
  UINT32 bitmask, imgutil_img_info_t *imgInfoP);


#endif /* BCM_IMGUTIL_API_H */
