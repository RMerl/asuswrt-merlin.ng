/*****************************************************************************
 *
 *  Copyright (c) 2015  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2015:DUAL/GPL:standard
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
