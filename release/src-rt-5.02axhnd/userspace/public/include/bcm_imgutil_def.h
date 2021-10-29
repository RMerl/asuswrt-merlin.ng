/***********************************************************************
 *
 *  Copyright (c) 2016  Broadcom
 *  All Rights Reserved
 *
 * <:label-BRCM:2016:DUAL/GPL:standard
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

#ifndef __BCM_IMGUTIL_DEF_H__
#define __BCM_IMGUTIL_DEF_H__

#include "os_defs.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/** Options for cmsImg_writeValidatedImageEx: write to partition1 */
#define CMS_IMAGE_WR_OPT_PART1        0x10

/** Options for cmsImg_writeValidatedImageEx: write to partition2 */
#define CMS_IMAGE_WR_OPT_PART2        0x20

/** Options for cmsImg_writeValidatedImageEx: Do not reboot after writing
  * image to non-active partition */
#define CMS_IMAGE_WR_OPT_NO_REBOOT    0x80

#define CMS_IMAGE_OVERHEAD          256

#define CMS_CONFIG_FILE_DETECTION_LENGTH 64

/*!\enum CmsImageFormat
 * \brief cms image formats that we recognize.
 *
 * Starting with release 4.14L.01, the CMS_IMAGE_FORMAT_PART1,
 * CMS_IMAGE_FORMAT_PART2, and CMS_IMAGE_FORMAT_NO_REBOOT enums
 * have been removed from cmsImageFormat enum since these are not formats.
 * Rather, they are flags which control the behavior of the system
 * when writing an image.  See cmsImage_writeValidatedImageEx.
 */
typedef enum
{
    CMS_IMAGE_FORMAT_INVALID=0,   /**< invalid or unrecognized format */
    CMS_IMAGE_FORMAT_BROADCOM=1,  /**< broadcom image (with our header) */
    CMS_IMAGE_FORMAT_FLASH=2,     /**< raw flash image */
    CMS_IMAGE_FORMAT_XML_CFG=3,   /**< CMS XML config file */
    CMS_IMAGE_FORMAT_MODSW_LINUXPFP=4, /**< modular software Linux Primary Firmware Patch */
    CMS_IMAGE_FORMAT_MODSW_LINUXEE_DU=5, /**< modular software Linux Execution Environment Deployment Unit */
	CMS_IMAGE_FORMAT_PART1=0x10,   /**< Specify to write to partition1 */
    CMS_IMAGE_FORMAT_PART2=0x20,   /**< Specify to write to partition2 */
    CMS_IMAGE_FORMAT_NO_REBOOT=0x80, /**< Do not reboot after flashing  */
    CMS_IMAGE_FORMAT_BUNDLE=0xF1
} CmsImageFormat;

#define imgutil_img_format_e CmsImageFormat

enum ComboParsingStates {combo_init, get_header_tag, buffering_complete_header,
    in_image, rolling, fail, not_combo, combo_done};


#endif /*__BCM_IMGUTIL_DEF_H__ */
