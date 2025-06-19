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

#ifndef BCM_IMGIF_NAND_H
#define BCM_IMGIF_NAND_H

/* ---- Include Files ----------------------------------------------------- */

#include "bcm_imgif.h"

/* ---- Constants and Types ----------------------------------------------- */
/* ---- Variable Externs -------------------------------------------------- */
/* ---- Function Prototypes ----------------------------------------------- */

IMGIF_HANDLE imgif_nand_open(IMG_FORMAT_PARSER_CB fmtParserCb,
  CAL_CRC32_CB calCrc32Cb);
int imgif_nand_write(IMGIF_HANDLE h, UINT8 *dataP, int len);
int imgif_nand_close(IMGIF_HANDLE h, UBOOL8 abortFlag);
int imgif_nand_set_image_info(IMGIF_HANDLE h, imgif_img_info_t *imgInfoP);
int imgif_nand_get_flash_info(imgif_flash_info_t *flashInfoP);


#endif /* CMS_IMGIF_NAND_H */
