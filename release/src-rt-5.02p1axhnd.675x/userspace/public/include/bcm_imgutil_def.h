/***********************************************************************
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


typedef struct {
    uint8_t accept_ranges; /* true means server accepts range */
    unsigned long int range_begin;  /* start of indivitual image in the combo stream */ 
    unsigned long int range_end;    /* end of individual image in the combo stream */
    uint8_t stage; /* stage 0 - HTTP GET combo. stage 1 - HTTP GET individual image */
} imgutil_accept_range_ctx_t;

#endif /*__BCM_IMGUTIL_DEF_H__ */
