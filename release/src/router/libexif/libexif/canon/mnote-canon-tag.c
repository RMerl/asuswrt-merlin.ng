/* mnote-canon-tag.c
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

#include <config.h>
#include "mnote-canon-tag.h"

#include <stdlib.h>

#include <libexif/i18n.h>

static const struct {
	MnoteCanonTag tag;
	const char *name;
	const char *title;
	const char *description;
} table[] = {
#ifndef NO_VERBOSE_TAG_STRINGS
	{MNOTE_CANON_TAG_SETTINGS_1, "Settings1", N_("Settings (First Part)"), ""},
	{MNOTE_CANON_TAG_FOCAL_LENGTH, "FocalLength", N_("Focal Length"), ""},
	{MNOTE_CANON_TAG_SETTINGS_2, "Settings2", N_("Settings (Second Part)"), ""},
	{MNOTE_CANON_TAG_PANORAMA, "Panorama", N_("Panorama"), ""},
	{MNOTE_CANON_TAG_IMAGE_TYPE, "ImageType", N_("Image Type"), ""},
	{MNOTE_CANON_TAG_FIRMWARE, "FirmwareVersion", N_("Firmware Version"), ""},
	{MNOTE_CANON_TAG_IMAGE_NUMBER, "ImageNumber", N_("Image Number"), ""},
	{MNOTE_CANON_TAG_OWNER, "OwnerName", N_("Owner Name"), ""},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, "ColorInformation", N_("Color Information"), ""},
	{MNOTE_CANON_TAG_SERIAL_NUMBER, "SerialNumber", N_("Serial Number"), ""},
	{MNOTE_CANON_TAG_CAMERA_INFO, "CameraInfo", N_("Camera Info"), ""},
	{MNOTE_CANON_TAG_FILE_LENGTH, "FileLength", N_("File Length"), ""},
	{MNOTE_CANON_TAG_CUSTOM_FUNCS, "CustomFunctions", N_("Custom Functions"), ""},
	{MNOTE_CANON_TAG_MODEL_ID,"ModelID", N_("Model ID"), ""},
	{MNOTE_CANON_TAG_MOVIE_INFO,"MovieInfo", N_("Movie Info"), ""},
	{MNOTE_CANON_TAG_AF_INFO,"AFInfo", N_("AF Info"), ""},
	{MNOTE_CANON_TAG_THUMBNAIL_VALID_AREA,"ThumbnailValidArea", N_("Thumbnail Valid Area"), ""},
	{MNOTE_CANON_TAG_SERIAL_NUMBER_FORMAT,"SerialNumberFormat", N_("Serial Number Format"), ""},
	{MNOTE_CANON_TAG_SUPER_MACRO,"SuperMacro", N_("Super Macro"), ""},
	{MNOTE_CANON_TAG_DATE_STAMP_MODE,"DateStampMode", N_("Date Stamp Mode"), ""},
	{MNOTE_CANON_TAG_MY_COLORS,"MyColors", N_("My Colors"), ""},
	{MNOTE_CANON_TAG_FIRMWARE_REVISION,"FirmwareRevision", N_("Firmware Revision"), ""},
	{MNOTE_CANON_TAG_CATEGORIES,"Categories", N_("Categories"), ""},
	{MNOTE_CANON_TAG_FACEDETECT1,"FaceDetect1", N_("Face Detect 1"), ""},
	{MNOTE_CANON_TAG_FACEDETECT2,"FaceDetect2", N_("Face Detect 2"), ""},
	{MNOTE_CANON_TAG_AF_INFO2,"AFInfo2", N_("AF Info 2"), ""},
	{MNOTE_CANON_TAG_CONTRAST_INFO,"ContrastInfo", N_("Contrast Info"), ""},
	{MNOTE_CANON_TAG_IMAGE_UNIQUE_ID,"ImageUniqueID", N_("Image Unique ID"), ""},
	{MNOTE_CANON_TAG_WB_INFO,"WBInfo", N_("WB Info"), ""},
	{MNOTE_CANON_TAG_FACEDETECT3,"FaceDetect3", N_("Face Detect 3"), ""},
	{MNOTE_CANON_TAG_TIMEINFO,"TimeInfo", N_("Time Info"), ""},
	{MNOTE_CANON_TAG_BATTERY_TYPE,"Battery Type", N_("Battery Type"), ""},
	{MNOTE_CANON_TAG_AF_INFO3,"AFInfo3", N_("AF Info 3"), ""},
	{MNOTE_CANON_TAG_RAW_DATA_OFFSET,"RawDataOffset", N_("Raw Data Offset"), ""},
	{MNOTE_CANON_TAG_ORIGINAL_DECISION_DATA_OFFSET,"OriginalDecisionDataOffset", N_("Original Decision Data Offset"), ""},
	{MNOTE_CANON_TAG_PERSONAL_FUNCTIONS,"PesonalFunctions", N_("Personal Functions"), ""},
	{MNOTE_CANON_TAG_PERSONAL_FUNCTIONS_VALUES,"PersonalFunctionsValues", N_("Personal Functions Values"), ""},
	{MNOTE_CANON_TAG_FILE_INFO,"FileInfo", N_("File Info"), ""},
	{MNOTE_CANON_TAG_LENS_MODEL,"LensModel", N_("Lens Model"), ""},
	{MNOTE_CANON_TAG_CMOS_SERIAL,"CMOSSerialNumber", N_("CMOS Serial Number"), ""},
	{MNOTE_CANON_TAG_DUST_REMOVAL_DATA,"DustRemovalData", N_("Dust Removal Data"), ""},
	{MNOTE_CANON_TAG_CROP_INFO,"CropInfo", N_("Crop Info"), ""},
	{MNOTE_CANON_TAG_CUSTOM_FUNCTIONS2,"CustomFunctions2", N_("Custom Functions 2"), ""},
	{MNOTE_CANON_TAG_ASPECT_INFO,"AspectInfo", N_("Aspect Info"), ""},
	{MNOTE_CANON_TAG_TONE_CURVE_TABLE,"ToneCurveTable", N_("Tone Curve Table"), ""},
	{MNOTE_CANON_TAG_SHARPNESS_TABLE,"SharpnessTable", N_("Sharpness Table"), ""},
	{MNOTE_CANON_TAG_SHARPNESS_FREQ_TABLE,"SharpnessFreqTable", N_("Sharpness Frequency Table"), ""},
	{MNOTE_CANON_TAG_WHITEBALANCE_TABLE,"WhitebalanceTable", N_("Whitebalance Table"), ""},
	{MNOTE_CANON_TAG_COLOR_BALANCE,"ColorBalance", N_("Color Balance"), ""},
	{MNOTE_CANON_TAG_MEASURED_COLOR,"MeasuredColor", N_("Measured Color"), ""},
	{MNOTE_CANON_TAG_COLOR_TEMPERATURE,"ColorTemperature", N_("Color Temperature"), ""},
	{MNOTE_CANON_TAG_CANON_FLAGS,"CanonFlags", N_("Canon Flags"), ""},
	{MNOTE_CANON_TAG_MODIFIED_INFO,"ModifiedInfo", N_("Modified Info"), ""},
	{MNOTE_CANON_TAG_TONECURVE_MATCHING,"TonecurveMatching", N_("Tonecurve Matching"), ""},
	{MNOTE_CANON_TAG_WHITEBALANCE_MATCHING,"WhitebalanceMatching", N_("Whitebalance Matching"), ""},
	{MNOTE_CANON_TAG_COLOR_SPACE,"ColorSpace", N_("Color Space"), ""},
	{MNOTE_CANON_TAG_PREVIEW_IMAGE_INFO,"PreviewImageInfo", N_("Preview Image Info"), ""},
	{MNOTE_CANON_TAG_VRD_OFFSET,"VRDOffset", N_("VRD Offset"), ""},
	{MNOTE_CANON_TAG_SENSOR_INFO,"SensorInfo", N_("Sensor Info"), ""},
	{MNOTE_CANON_TAG_WB_PACKET,"WBPacket", N_("WB Packet"), ""},
	{MNOTE_CANON_TAG_FLAVOR,"Flavor", N_("Flavor"), ""},
	{MNOTE_CANON_TAG_PICTURESTYLE_USERDEF,"PictureStyleUserDef", N_("Picture Style Userdefined"), ""},
	{MNOTE_CANON_TAG_PICTURESTYLE_PC,"PictureStylePC", N_("Picture Style PC"), ""},
	{MNOTE_CANON_TAG_CUSTOMPICTURE_STYLE_FN,"CustomPictureStyleFN", N_("Custom Picture Style Filename"), ""},
	{MNOTE_CANON_TAG_AF_MICRO_ADJUST,"AFMicroAdjust", N_("AF Micro Adjust"), ""},
	{MNOTE_CANON_TAG_VIGNETTING_CORRECT,"VignettingCorrect", N_("Vignetting Correct"), ""},
	{MNOTE_CANON_TAG_VIGNETTING_CORRECT2,"VignettingCorrect2", N_("Vignetting Correct 2"), ""},
	{MNOTE_CANON_TAG_LIGHTINGOPT,"LightingOpt", N_("LightingOpt"), ""},
	{MNOTE_CANON_TAG_LENS_INFO,"LensInfo", N_("Lens Info"), ""},
	{MNOTE_CANON_TAG_AMBIENCE_INFO,"AmbienceInfo", N_("Ambience_Info"), ""},
	{MNOTE_CANON_TAG_MULTI_EXPOSURE,"MultiExposure", N_("Multi Exposure"), ""},
	{MNOTE_CANON_TAG_FILTER_INFO,"FilterInfo", N_("Filter Info"), ""},
	{MNOTE_CANON_TAG_HDR_INFO,"HDRInfo", N_("HDR Info"), ""},
	{MNOTE_CANON_TAG_AF_CONFIG,"AFConfig", N_("AF Config"), ""},
	{MNOTE_CANON_TAG_RAW_BURST_INFO,"RawBurstInfo", N_("Raw Burst Info"), ""},
#endif
	{0, NULL, NULL, NULL}
};

static const struct {
	MnoteCanonTag tag;
	unsigned int subtag;
	const char *name;
} table_sub[] = {
#ifndef NO_VERBOSE_TAG_STRINGS
	{MNOTE_CANON_TAG_SETTINGS_1,  0, N_("Macro Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1,  1, N_("Self-timer")},
	{MNOTE_CANON_TAG_SETTINGS_1,  2, N_("Quality")},
	{MNOTE_CANON_TAG_SETTINGS_1,  3, N_("Flash Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1,  4, N_("Drive Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1,  6, N_("Focus Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1,  8, N_("Record Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1,  9, N_("Image Size")},
	{MNOTE_CANON_TAG_SETTINGS_1, 10, N_("Easy Shooting Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1, 11, N_("Digital Zoom")},
	{MNOTE_CANON_TAG_SETTINGS_1, 12, N_("Contrast")},
	{MNOTE_CANON_TAG_SETTINGS_1, 13, N_("Saturation")},
	{MNOTE_CANON_TAG_SETTINGS_1, 14, N_("Sharpness")},
	{MNOTE_CANON_TAG_SETTINGS_1, 15, N_("ISO")},
	{MNOTE_CANON_TAG_SETTINGS_1, 16, N_("Metering Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1, 17, N_("Focus Range")},
	{MNOTE_CANON_TAG_SETTINGS_1, 18, N_("AF Point")},
	{MNOTE_CANON_TAG_SETTINGS_1, 19, N_("Exposure Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1, 21, N_("Lens Type")},
	{MNOTE_CANON_TAG_SETTINGS_1, 22, N_("Long Focal Length of Lens")},
	{MNOTE_CANON_TAG_SETTINGS_1, 23, N_("Short Focal Length of Lens")},
	{MNOTE_CANON_TAG_SETTINGS_1, 24, N_("Focal Units per mm")},
	{MNOTE_CANON_TAG_SETTINGS_1, 25, N_("Maximal Aperture")},
	{MNOTE_CANON_TAG_SETTINGS_1, 26, N_("Minimal Aperture")},
	{MNOTE_CANON_TAG_SETTINGS_1, 27, N_("Flash Activity")},
	{MNOTE_CANON_TAG_SETTINGS_1, 28, N_("Flash Details")},
	{MNOTE_CANON_TAG_SETTINGS_1, 31, N_("Focus Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1, 32, N_("AE Setting")},
	{MNOTE_CANON_TAG_SETTINGS_1, 33, N_("Image Stabilization")},
	{MNOTE_CANON_TAG_SETTINGS_1, 34, N_("Display Aperture")},
	{MNOTE_CANON_TAG_SETTINGS_1, 35, N_("Zoom Source Width")},
	{MNOTE_CANON_TAG_SETTINGS_1, 36, N_("Zoom Target Width")},
	{MNOTE_CANON_TAG_SETTINGS_1, 38, N_("Spot Metering Mode")},
	{MNOTE_CANON_TAG_SETTINGS_1, 39, N_("Photo Effect")},
	{MNOTE_CANON_TAG_SETTINGS_1, 40, N_("Manual Flash Output")},
	{MNOTE_CANON_TAG_SETTINGS_1, 41, N_("Color Tone")},
	{MNOTE_CANON_TAG_SETTINGS_1, 45, N_("SRAW Quality")},
	{MNOTE_CANON_TAG_FOCAL_LENGTH, 0, N_("Focal Type")},
	{MNOTE_CANON_TAG_FOCAL_LENGTH, 1, N_("Focal Length")},
	{MNOTE_CANON_TAG_FOCAL_LENGTH, 2, N_("Focal Plane X Size")},
	{MNOTE_CANON_TAG_FOCAL_LENGTH, 3, N_("Focal Plane Y Size")},
	{MNOTE_CANON_TAG_SETTINGS_2, 0, N_("Auto ISO")},
	{MNOTE_CANON_TAG_SETTINGS_2, 1, N_("Shot ISO")},
	{MNOTE_CANON_TAG_SETTINGS_2, 2, N_("Measured EV")},
	{MNOTE_CANON_TAG_SETTINGS_2, 3, N_("Target Aperture")},
	{MNOTE_CANON_TAG_SETTINGS_2, 4, N_("Target Exposure Time")},
	{MNOTE_CANON_TAG_SETTINGS_2, 5, N_("Exposure Compensation")},
	{MNOTE_CANON_TAG_SETTINGS_2, 6, N_("White Balance")},
	{MNOTE_CANON_TAG_SETTINGS_2, 7, N_("Slow Shutter")},
	{MNOTE_CANON_TAG_SETTINGS_2, 8, N_("Sequence Number")},
	{MNOTE_CANON_TAG_SETTINGS_2, 9, N_("Optical Zoom Code")},
	{MNOTE_CANON_TAG_SETTINGS_2, 11, N_("Camera Temperature")},
	{MNOTE_CANON_TAG_SETTINGS_2, 12, N_("Flash Guide Number")},
	{MNOTE_CANON_TAG_SETTINGS_2, 13, N_("AF Point")},
	{MNOTE_CANON_TAG_SETTINGS_2, 14, N_("Flash Exposure Compensation")},
	{MNOTE_CANON_TAG_SETTINGS_2, 15, N_("AE Bracketing")},
	{MNOTE_CANON_TAG_SETTINGS_2, 16, N_("AE Bracket Value")},
	{MNOTE_CANON_TAG_SETTINGS_2, 17, N_("Control Mode")},
	{MNOTE_CANON_TAG_SETTINGS_2, 18, N_("Focus Distance Upper")},
	{MNOTE_CANON_TAG_SETTINGS_2, 19, N_("Focus Distance Lower")},
	{MNOTE_CANON_TAG_SETTINGS_2, 20, N_("F-Number")},
	{MNOTE_CANON_TAG_SETTINGS_2, 21, N_("Exposure Time")},
	{MNOTE_CANON_TAG_SETTINGS_2, 22, N_("Measured EV 2")},
	{MNOTE_CANON_TAG_SETTINGS_2, 23, N_("Bulb Duration")},
	{MNOTE_CANON_TAG_SETTINGS_2, 25, N_("Camera Type")},
	{MNOTE_CANON_TAG_SETTINGS_2, 26, N_("Auto Rotate")},
	{MNOTE_CANON_TAG_SETTINGS_2, 27, N_("ND Filter")},
	{MNOTE_CANON_TAG_SETTINGS_2, 28, N_("Self-timer")},
	{MNOTE_CANON_TAG_SETTINGS_2, 32, N_("Manual Flash Output")},
	{MNOTE_CANON_TAG_PANORAMA, 2, N_("Panorama Frame")},
	{MNOTE_CANON_TAG_PANORAMA, 5, N_("Panorama Direction")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 0, N_("Tone Curve")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 1, N_("Sharpness")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 2, N_("Sharpness Frequency")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 3, N_("Sensor Red Level")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 4, N_("Sensor Blue Level")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 5, N_("White Balance Red")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 6, N_("White Balance Blue")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 7, N_("White Balance")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 8, N_("Color Temperature")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 9, N_("Picture Style")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 10, N_("Digital Gain")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 11, N_("White Balance Shift AB")},
	{MNOTE_CANON_TAG_COLOR_INFORMATION, 12, N_("White Balance Shift GM")},
#endif
	{0, 0, NULL}
};

const char *
mnote_canon_tag_get_name (MnoteCanonTag t)
{
	unsigned int i;

	for (i = 0; i < sizeof (table) / sizeof (table[0]); i++)
		if (table[i].tag == t) return table[i].name; /* do not translate */
	return NULL;
}

const char *
mnote_canon_tag_get_name_sub (MnoteCanonTag t, unsigned int s, ExifDataOption o)
{
	unsigned int i;
	int tag_found = 0;

	for (i = 0; i < sizeof (table_sub) / sizeof (table_sub[0]); i++) {
		if (table_sub[i].tag == t) {
			if (table_sub[i].subtag == s)
				return table_sub[i].name;
			tag_found = 1;
		}
	}
	if (!tag_found || !(o & EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS))
		return mnote_canon_tag_get_name (t);
	else
		return NULL;
}

const char *
mnote_canon_tag_get_title (MnoteCanonTag t)
{
	unsigned int i;

	(void) bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	for (i = 0; i < sizeof (table) / sizeof (table[0]); i++)
		if (table[i].tag == t) return (_(table[i].title));
	return NULL;
}

const char *
mnote_canon_tag_get_title_sub (MnoteCanonTag t, unsigned int s, ExifDataOption o)
{
	unsigned int i;
	int tag_found = 0;

	for (i = 0; i < sizeof (table_sub) / sizeof (table_sub[0]); i++) {
		if (table_sub[i].tag == t) {
			if (table_sub[i].subtag == s)
				return _(table_sub[i].name);
			tag_found = 1;
		}
	}
	if (!tag_found || !(o & EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS))
		return mnote_canon_tag_get_title (t);
	else
		return NULL;
}

const char *
mnote_canon_tag_get_description (MnoteCanonTag t)
{
	unsigned int i;

	for (i = 0; i < sizeof (table) / sizeof (table[0]); i++)
		if (table[i].tag == t) {
			if (!table[i].description || !*table[i].description)
				return "";
			(void) bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
			return _(table[i].description);
		}
	return NULL;
}
