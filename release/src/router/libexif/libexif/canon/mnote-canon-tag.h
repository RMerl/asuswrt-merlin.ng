/* mnote-canon-tag.h
 *
 * Copyright (c) 2002 Lutz Mueller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA.
 */

#ifndef LIBEXIF_MNOTE_CANON_TAG_H
#define LIBEXIF_MNOTE_CANON_TAG_H

#include <libexif/exif-data.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum _MnoteCanonTag {
	MNOTE_CANON_TAG_UNKNOWN_0	= 0x0,
	MNOTE_CANON_TAG_SETTINGS_1	= 0x1, /* subtree */
	MNOTE_CANON_TAG_FOCAL_LENGTH	= 0x2,
	MNOTE_CANON_TAG_UNKNOWN_3	= 0x3,
	MNOTE_CANON_TAG_SETTINGS_2	= 0x4, /* subtree */
	MNOTE_CANON_TAG_PANORAMA	= 0x5,
	MNOTE_CANON_TAG_IMAGE_TYPE	= 0x6,
	MNOTE_CANON_TAG_FIRMWARE	= 0x7,
	MNOTE_CANON_TAG_IMAGE_NUMBER	= 0x8,
	MNOTE_CANON_TAG_OWNER		= 0x9,
	MNOTE_CANON_TAG_UNKNOWN_10	= 0xa,
	MNOTE_CANON_TAG_SERIAL_NUMBER	= 0xc,
	MNOTE_CANON_TAG_CAMERA_INFO	= 0xd,
	MNOTE_CANON_TAG_FILE_LENGTH	= 0xe,
	MNOTE_CANON_TAG_CUSTOM_FUNCS	= 0xf,
	MNOTE_CANON_TAG_MODEL_ID	= 0x10,
	MNOTE_CANON_TAG_MOVIE_INFO	= 0x11, /* subtree */
	MNOTE_CANON_TAG_AF_INFO		= 0x12, /* subtree */
	MNOTE_CANON_TAG_THUMBNAIL_VALID_AREA		= 0x13,
	MNOTE_CANON_TAG_SERIAL_NUMBER_FORMAT		= 0x15,
	MNOTE_CANON_TAG_SUPER_MACRO	= 0x1a,
	MNOTE_CANON_TAG_DATE_STAMP_MODE	= 0x1c,
	MNOTE_CANON_TAG_MY_COLORS	= 0x1d,	/* subtree */
	MNOTE_CANON_TAG_FIRMWARE_REVISION	= 0x1e,
	MNOTE_CANON_TAG_CATEGORIES	= 0x23,
	MNOTE_CANON_TAG_FACEDETECT1	= 0x24,	/* subtree */
	MNOTE_CANON_TAG_FACEDETECT2	= 0x25,	/* subtree */
	MNOTE_CANON_TAG_AF_INFO2	= 0x26,	/* subtree */
	MNOTE_CANON_TAG_CONTRAST_INFO	= 0x27,	/* subtree */
	MNOTE_CANON_TAG_IMAGE_UNIQUE_ID	= 0x28,
	MNOTE_CANON_TAG_WB_INFO		= 0x29,	/* subtree */
	MNOTE_CANON_TAG_FACEDETECT3	= 0x2f,	/* subtree */
	MNOTE_CANON_TAG_TIMEINFO	= 0x35,	/* subtree */
	MNOTE_CANON_TAG_BATTERY_TYPE	= 0x38,
	MNOTE_CANON_TAG_AF_INFO3	= 0x3c,	/* subtree */
	MNOTE_CANON_TAG_RAW_DATA_OFFSET	= 0x81,
	MNOTE_CANON_TAG_ORIGINAL_DECISION_DATA_OFFSET	= 0x83,
	MNOTE_CANON_TAG_PERSONAL_FUNCTIONS		= 0x91,	/* subtree */
	MNOTE_CANON_TAG_PERSONAL_FUNCTIONS_VALUES	= 0x92,	/* subtree */
	MNOTE_CANON_TAG_FILE_INFO		= 0x93,	/* subtree */
	MNOTE_CANON_TAG_LENS_MODEL		= 0x95,
	MNOTE_CANON_TAG_CMOS_SERIAL		= 0x96,
	MNOTE_CANON_TAG_DUST_REMOVAL_DATA	= 0x97,
	MNOTE_CANON_TAG_CROP_INFO		= 0x98,	/* subtree */
	MNOTE_CANON_TAG_CUSTOM_FUNCTIONS2	= 0x99,	/* subtree */
	MNOTE_CANON_TAG_ASPECT_INFO		= 0x9a,	/* subtree */
	MNOTE_CANON_TAG_COLOR_INFORMATION	= 0xa0,	/* subtree */
	MNOTE_CANON_TAG_TONE_CURVE_TABLE	= 0xa1,
	MNOTE_CANON_TAG_SHARPNESS_TABLE		= 0xa2,
	MNOTE_CANON_TAG_SHARPNESS_FREQ_TABLE	= 0xa3,
	MNOTE_CANON_TAG_WHITEBALANCE_TABLE	= 0xa3,
	MNOTE_CANON_TAG_COLOR_BALANCE		= 0xa9,	/* subtree */
	MNOTE_CANON_TAG_MEASURED_COLOR		= 0xaa,	/* subtree */
	MNOTE_CANON_TAG_COLOR_TEMPERATURE	= 0xae,
	MNOTE_CANON_TAG_CANON_FLAGS		= 0xb0,	/* subtree */
	MNOTE_CANON_TAG_MODIFIED_INFO		= 0xb1,	/* subtree */
	MNOTE_CANON_TAG_TONECURVE_MATCHING	= 0xb2,
	MNOTE_CANON_TAG_WHITEBALANCE_MATCHING	= 0xb3,
	MNOTE_CANON_TAG_COLOR_SPACE		= 0xb4,
	MNOTE_CANON_TAG_PREVIEW_IMAGE_INFO	= 0xb6,	/* subtree */
	MNOTE_CANON_TAG_VRD_OFFSET		= 0xd0,
	MNOTE_CANON_TAG_SENSOR_INFO		= 0xe0,	/* subtree */
	MNOTE_CANON_TAG_WB_PACKET		= 0x4001, /* subtree */
	MNOTE_CANON_TAG_COLOR_INFO		= 0x4003, /* subtree */
	MNOTE_CANON_TAG_FLAVOR			= 0x4005,
	MNOTE_CANON_TAG_PICTURESTYLE_USERDEF	= 0x4008,
	MNOTE_CANON_TAG_PICTURESTYLE_PC		= 0x4009,
	MNOTE_CANON_TAG_CUSTOMPICTURE_STYLE_FN	= 0x4010,
	MNOTE_CANON_TAG_AF_MICRO_ADJUST		= 0x4013, /* subtree */
	MNOTE_CANON_TAG_VIGNETTING_CORRECT	= 0x4015, /* subtree */
	MNOTE_CANON_TAG_VIGNETTING_CORRECT2	= 0x4016, /* subtree */
	MNOTE_CANON_TAG_LIGHTINGOPT		= 0x4018, /* subtree */
	MNOTE_CANON_TAG_LENS_INFO		= 0x4019, /* subtree */
	MNOTE_CANON_TAG_AMBIENCE_INFO		= 0x4020, /* subtree */
	MNOTE_CANON_TAG_MULTI_EXPOSURE		= 0x4021, /* subtree */
	MNOTE_CANON_TAG_FILTER_INFO		= 0x4024, /* subtree */
	MNOTE_CANON_TAG_HDR_INFO		= 0x4025, /* subtree */
	MNOTE_CANON_TAG_AF_CONFIG		= 0x4028, /* subtree */
	MNOTE_CANON_TAG_RAW_BURST_INFO		= 0x403f  /* subtree */
};
typedef enum _MnoteCanonTag MnoteCanonTag;

const char *mnote_canon_tag_get_name        (MnoteCanonTag);
const char *mnote_canon_tag_get_name_sub    (MnoteCanonTag, unsigned int, ExifDataOption);
const char *mnote_canon_tag_get_title       (MnoteCanonTag);
const char *mnote_canon_tag_get_title_sub   (MnoteCanonTag, unsigned int, ExifDataOption);
const char *mnote_canon_tag_get_description (MnoteCanonTag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(LIBEXIF_MNOTE_CANON_TAG_H) */
