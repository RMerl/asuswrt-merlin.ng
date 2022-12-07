/* mnote-olympus-tag.c:
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
#include "mnote-olympus-tag.h"

#include <libexif/i18n.h>
#include <libexif/exif-utils.h>

#include <stdlib.h>

static const struct {
	MnoteOlympusTag tag;
	const char *name;
	const char *title;
	const char *description;
} table[] = {
#ifndef NO_VERBOSE_TAG_STRINGS
	/* Nikon v2 */
	{MNOTE_NIKON_TAG_FIRMWARE,     "Firmware", N_("Firmware Version"), ""},
	{MNOTE_NIKON_TAG_ISO,          "ISO", N_("ISO Setting"), ""},
	{MNOTE_NIKON_TAG_COLORMODE1,   "ColorMode1", N_("Color Mode (?)"), ""},
	{MNOTE_NIKON_TAG_QUALITY,      "Quality", N_("Quality"), ""},
	{MNOTE_NIKON_TAG_WHITEBALANCE, "WhiteBalance", N_("White Balance"), ""},
	{MNOTE_NIKON_TAG_SHARPENING,   "Sharpening",   N_("Image Sharpening"), ""},
	{MNOTE_NIKON_TAG_FOCUSMODE,    "FocusMode",   N_("Focus Mode"), ""},
	{MNOTE_NIKON_TAG_FLASHSETTING, "FlashSetting",   N_("Flash Setting"), ""},
	{MNOTE_NIKON_TAG_FLASHMODE,    "FlashMode",    N_("Flash Mode"), ""},
	{MNOTE_NIKON_TAG_WHITEBALANCEFINE,"WhiteBalanceFine",N_("White Balance Fine Adjustment"), ""},
	{MNOTE_NIKON_TAG_WHITEBALANCERB,  "WhiteBalanceRB", N_("White Balance RB"), ""},
	{MNOTE_NIKON_TAG_PROGRAMSHIFT,  "ProgramShift", N_("Program Shift"), ""},
	{MNOTE_NIKON_TAG_ISOSELECTION,    "ISOSelection", N_("ISO Selection"), ""},
	{MNOTE_NIKON_TAG_PREVIEWIMAGE_IFD_POINTER, "PreviewImage", N_("Preview Image IFD"), N_("Offset of the preview image directory (IFD) inside the file.")},
	{MNOTE_NIKON_TAG_EXPOSUREDIFF,    "ExposureDiff", N_("Exposurediff ?"), ""},
	{MNOTE_NIKON_TAG_FLASHEXPCOMPENSATION, "FlashExpCompensation", N_("Flash Exposure Compensation"), ""},
	{MNOTE_NIKON_TAG_ISO2,            "ISO", N_("ISO Setting"), ""},
	{MNOTE_NIKON_TAG_IMAGEBOUNDARY,   "ImageBoundary", N_("Image Boundary"), ""},
	{MNOTE_NIKON_TAG_EXTERNALFLASHEXPCOMPENSATION,  "ExternalFlashExpCompensation", N_("External Flash Exposure Compensation"), ""},
	{MNOTE_NIKON_TAG_FLASHEXPOSUREBRACKETVAL,  "FlashExposureBracketVal", N_("Flash Exposure Bracket Value"), ""},
	{MNOTE_NIKON_TAG_EXPOSUREBRACKETVAL,  "ExposureBracketVal", N_("Exposure Bracket Value"), ""},
	{MNOTE_NIKON_TAG_IMAGEADJUSTMENT, "ImageAdjustment", N_("Image Adjustment"), ""},
	{MNOTE_NIKON_TAG_TONECOMPENSATION, "ToneCompensation", N_("Tone Compensation"), ""},
	{MNOTE_NIKON_TAG_ADAPTER,         "Adapter", N_("Adapter"), ""},
	{MNOTE_NIKON_TAG_LENSTYPE,        "LensType", N_("Lens Type"), ""},
	{MNOTE_NIKON_TAG_LENS,            "Lens", N_("Lens"), ""},
	{MNOTE_NIKON_TAG_MANUALFOCUSDISTANCE, "ManualFocusDistance", N_("Manual Focus Distance"), ""},
	{MNOTE_NIKON_TAG_DIGITALZOOM,     "DigitalZoom", N_("Digital Zoom"), ""},
	{MNOTE_NIKON_TAG_FLASHUSED,       "FlashUsed", N_("Flash Used"), ""},
	{MNOTE_NIKON_TAG_AFFOCUSPOSITION, "AFFocusPosition", N_("AF Focus Position"), ""},
	{MNOTE_NIKON_TAG_BRACKETING,      "Bracketing", N_("Bracketing"), ""},
	{MNOTE_NIKON_TAG_UNKNOWN_0X008A,  NULL, NULL, NULL},
	{MNOTE_NIKON_TAG_LENS_FSTOPS,     "LensFStops", N_("Lens F Stops"), ""},
	{MNOTE_NIKON_TAG_CURVE,           "Curve,", N_("Contrast Curve"), ""},
	{MNOTE_NIKON_TAG_COLORMODE,       "ColorMode,", N_("Color Mode"), ""},
	{MNOTE_NIKON_TAG_LIGHTTYPE,       "LightType,", N_("Light Type"), ""},
	{MNOTE_NIKON_TAG_SHOTINFO,	  "ShotInfo", N_("Shot Info"), ""},
	{MNOTE_NIKON_TAG_HUE,             "Hue", N_("Hue Adjustment"), ""},
	{MNOTE_NIKON_TAG_SATURATION,      "Saturation", N_("Saturation"), ""},
	{MNOTE_NIKON_TAG_NOISEREDUCTION,  "NoiseReduction,", N_("Noise Reduction"), ""},
	{MNOTE_NIKON_TAG_COLORBALANCE,    "ColorBalance", N_("Color Balance"), ""},
	{MNOTE_NIKON_TAG_LENSDATA,        "LensData", N_("Lens Data"), ""},
	{MNOTE_NIKON_TAG_SENSORPIXELSIZE, "SensorPixelSize", N_("Sensor Pixel Size"), ""},
	{MNOTE_NIKON_TAG_UNKNOWN_0X009B,  NULL, NULL, NULL},
	{MNOTE_NIKON_TAG_RETOUCHHISTORY,  "RetouchHistory", N_("Retouch History"), ""},
	{MNOTE_NIKON_TAG_SERIALNUMBER,    "SerialNumber", N_("Serial Number"), ""},
	{MNOTE_NIKON_TAG_IMAGE_DATASIZE,  "ImageDataSize", N_("Image Data Size"), N_("Size of compressed image data in bytes.")},
	{MNOTE_NIKON_TAG_UNKNOWN_0X00A3,  NULL, NULL, NULL},
	{MNOTE_NIKON_TAG_TOTALPICTURES,   "TotalPictures,", N_("Total Number of Pictures Taken"), ""},
	{MNOTE_NIKON_TAG_FLASHINFO,       "FlashInfo", N_("Flash Info"), ""},
	{MNOTE_NIKON_TAG_OPTIMIZATION,    "Optimization,", N_("Optimize Image"), ""},
	{MNOTE_NIKON_TAG_SATURATION,      "Saturation", N_("Saturation"), ""},
	{MNOTE_NIKON_TAG_VARIPROGRAM,     "VariProgram", N_("Vari Program"), ""},
	{MNOTE_NIKON_TAG_CAPTUREEDITORDATA, "CaptureEditorData", N_("Capture Editor Data"), ""},
	{MNOTE_NIKON_TAG_CAPTUREEDITORVER, "CaptureEditorVer", N_("Capture Editor Version"), ""},
	{MNOTE_NIKON_TAG_UNKNOWN_0X0E0E,  NULL, NULL, NULL},
	{MNOTE_NIKON_TAG_UNKNOWN_0X0E10,  NULL, NULL, NULL},
	{MNOTE_NIKON_TAG_CROPHISPEED,    "CropHiSpeed", N_("Crop HiSpeed"), ""},
	{MNOTE_NIKON_TAG_EXPOSURETUNING, "ExposureTuning", N_("Exposure Tuning"), ""},
	{MNOTE_NIKON_TAG_COLORSPACE,     "ColorSpace", N_("Color Space"), ""},
	{MNOTE_NIKON_TAG_VRINFO,         "VRInfo", N_("VR Info"), ""},
	{MNOTE_NIKON_TAG_IMAGEAUTHENTICATION, "ImageAuthentication", N_("Image Authentication"), ""},
	{MNOTE_NIKON_TAG_FACEDETECT,     "FaceDetect", N_("Face Detect"), ""},
	{MNOTE_NIKON_TAG_ACTIVEDLIGHTING,"ActiveDLighting", N_("Active DLighting"), ""},
	{MNOTE_NIKON_TAG_PICTURECONTROLDATA, "PictureControlData", N_("Picture Control Data"), ""},
	{MNOTE_NIKON_TAG_WORLDTIME,      "WorldTime", N_("World Time"), ""},
	{MNOTE_NIKON_TAG_ISOINFO,        "ISOInfo", N_("ISO Info"), ""},
	{MNOTE_NIKON_TAG_VIGNETTECONTROL,"VignetteControl", N_("Vignette Control"), ""},
	{MNOTE_NIKON_TAG_DISTORTINFO,    "DistortInfo", N_("Distort Info"), ""},
	{MNOTE_NIKON_TAG_SHUTTERMODE,    "ShutterMode", N_("Shutter Mode"), ""},
	{MNOTE_NIKON_TAG_HDRINFO,        "HDRInfo", N_("HDR Info"), ""},
	{MNOTE_NIKON_TAG_MECHANICALSHUTTERCOUNT, "MechanicalShutterCount", N_("Mechanical Shutter Count"), ""},
	{MNOTE_NIKON_TAG_LOCATIONINFO,   "LocationInfo", N_("MNOTE_NIKON_TAG_LOCATIONINFO"), ""},
	{MNOTE_NIKON_TAG_BLACKLEVEL,     "BlackLevel", N_("Black Level"), ""},
	{MNOTE_NIKON_TAG_IMAGESIZERAW,   "ImageSizeRaw", N_("Image Size Raw"), ""},
	{MNOTE_NIKON_TAG_CROPAREA,       "CropArea", N_("Crop Area"), ""},
	{MNOTE_NIKON_TAG_NIKONSETTINGS,  "NikonSettings", N_("Nikon Settings"), ""},
	{MNOTE_NIKON_TAG_COLORTEMPERATUREAUTO, "ColorTemperatureAuto", N_("Color Temperature Auto"), ""},
	{MNOTE_NIKON_TAG_SERIALNUMBER2,  "SerialNumber2", N_("Serial Number 2"), ""},
	{MNOTE_NIKON_TAG_SATURATION2,    "Saturation2", N_("Saturation 2"), ""},
	{MNOTE_NIKON_TAG_MULTIEXPOSURE,  "MultiExposure", N_("Multi Exposure"), ""},
	{MNOTE_NIKON_TAG_HIGHISONR,      "HighISONr", N_("High ISO Noise Reduction"), ""},
	{MNOTE_NIKON_TAG_TONINGEFFECT,   "ToningEffect", N_("Toning Effect"), ""},
	{MNOTE_NIKON_TAG_POWERUPTIME,    "PowerupTime", N_("Powerup Time"), ""},
	{MNOTE_NIKON_TAG_AFINFO2,        "AFInfo2", N_("AF Info 2"), ""},
	{MNOTE_NIKON_TAG_FILEINFO,       "FileInfo", N_("File Info"), ""},
	{MNOTE_NIKON_TAG_RETOUCHINFO,    "RetouchInfo", N_("Retouch Info"), ""},
	{MNOTE_NIKON_TAG_PREVIEWIMAGE,   "PreviewImage", N_("Preview Image"), ""},
	{MNOTE_NIKON1_TAG_UNKNOWN_0X0002, NULL, NULL, NULL},
	{MNOTE_NIKON1_TAG_QUALITY,        "Quality", N_("Quality"), ""},
	{MNOTE_NIKON1_TAG_COLORMODE,      "ColorMode,", N_("Color Mode"), ""},
	{MNOTE_NIKON1_TAG_IMAGEADJUSTMENT, "ImageAdjustment", N_("Image Adjustment"), ""},
	{MNOTE_NIKON1_TAG_CCDSENSITIVITY, "CCDSensitivity", N_("CCD Sensitivity"), ""},
	{MNOTE_NIKON1_TAG_WHITEBALANCE,   "WhiteBalance", N_("White Balance"), ""},
	{MNOTE_NIKON1_TAG_FOCUS,          "Focus", N_("Focus"), ""},
	{MNOTE_NIKON1_TAG_UNKNOWN_0X0009, NULL, NULL, NULL},
	{MNOTE_NIKON1_TAG_DIGITALZOOM,    "DigitalZoom", N_("Digital Zoom"), ""},
	{MNOTE_NIKON1_TAG_CONVERTER,      "Converter", N_("Converter"), ""},

	/* Olympus & some Sanyo */
	{MNOTE_OLYMPUS_TAG_THUMBNAILIMAGE, "ThumbnailImage", N_("Thumbnail Image"), ""},
	{MNOTE_OLYMPUS_TAG_MODE, "Mode", N_("Speed/Sequence/Panorama Direction"), ""},
	{MNOTE_OLYMPUS_TAG_QUALITY, "Quality", N_("Quality"), ""},
	{MNOTE_OLYMPUS_TAG_MACRO, "Macro", N_("Macro"), ""},
	{MNOTE_OLYMPUS_TAG_BWMODE, "BWMode", N_("Black & White Mode"), ""},
	{MNOTE_OLYMPUS_TAG_DIGIZOOM, "DigiZoom", N_("Digital Zoom"), ""},
	{MNOTE_OLYMPUS_TAG_FOCALPLANEDIAGONAL, "FocalPlaneDiagonal", N_("Focal Plane Diagonal"), ""},
	{MNOTE_OLYMPUS_TAG_LENSDISTORTION, "LensDistortionParams", N_("Lens Distortion Parameters"), ""},
	{MNOTE_OLYMPUS_TAG_VERSION, "FirmwareVersion", N_("Firmware Version"), ""},
	{MNOTE_OLYMPUS_TAG_INFO, "Info", N_("Info"), ""},
	{MNOTE_OLYMPUS_TAG_ID, "CameraID", N_("Camera ID"), ""},
	{MNOTE_OLYMPUS_TAG_PRECAPTUREFRAMES, "PreCaptureFrames", N_("Precapture Frames"), ""},
	{MNOTE_OLYMPUS_TAG_WHITEBOARD, "WhiteBoard", N_("White Board"), ""},
	{MNOTE_OLYMPUS_TAG_ONETOUCHWB, "OneTouchWB", N_("One Touch White Balance"), ""},
	{MNOTE_OLYMPUS_TAG_WHITEBALANCEBRACKET, "WhiteBalanceBracket", N_("White Balance Bracket"), ""},
	{MNOTE_OLYMPUS_TAG_WHITEBALANCEBIAS, "WhiteBalanceBias", N_("White Balance Bias"), ""},
	{MNOTE_OLYMPUS_TAG_DATADUMP, "DataDump", N_("Data Dump"), NULL},
	{MNOTE_OLYMPUS_TAG_UNKNOWN_4, NULL, NULL, NULL},
	{MNOTE_OLYMPUS_TAG_SHUTTERSPEED, "ShutterSpeed", N_("Shutter Speed"), ""},
	{MNOTE_OLYMPUS_TAG_ISOVALUE, "ISOValue", N_("ISO Value"), ""},
	{MNOTE_OLYMPUS_TAG_APERTUREVALUE, "ApertureValue", N_("Aperture Value"), ""},
	{MNOTE_OLYMPUS_TAG_BRIGHTNESSVALUE, "BrightnessValue", N_("Brightness Value"), ""},
	{MNOTE_OLYMPUS_TAG_FLASHMODE, "FlashMode", N_("Flash Mode"), ""},
	{MNOTE_OLYMPUS_TAG_FLASHDEVICE, "FlashDevice", N_("Flash Device"), ""},
	{MNOTE_OLYMPUS_TAG_EXPOSURECOMP, "ExposureCompensation", N_("Exposure Compensation"), ""},
	{MNOTE_OLYMPUS_TAG_SENSORTEMPERATURE, "SensorTemperature", N_("Sensor Temperature"), ""},
	{MNOTE_OLYMPUS_TAG_LENSTEMPERATURE, "LensTemperature", N_("Lens Temperature"), ""},
	{MNOTE_OLYMPUS_TAG_LIGHTCONDITION, "LightCondition", N_("Light Condition"), ""},
	{MNOTE_OLYMPUS_TAG_FOCUSRANGE, "FocusRange", N_("Focus Range"), ""},
	{MNOTE_OLYMPUS_TAG_MANFOCUS, "FocusMode", N_("Focus Mode"), "Automatic or manual focusing mode"},
	{MNOTE_OLYMPUS_TAG_FOCUSDIST, "ManualFocusDistance", N_("Manual Focus Distance"), ""},
	{MNOTE_OLYMPUS_TAG_ZOOMSTEPCOUNT, "ZoomStepCount", N_("Zoom Step Count"), ""},
	{MNOTE_OLYMPUS_TAG_FOCUSSTEPCOUNT, "FocusStepCount", N_("Focus Step Count"), ""},
	{MNOTE_OLYMPUS_TAG_SHARPNESS, "Sharpness", N_("Sharpness Setting"), ""},
	{MNOTE_OLYMPUS_TAG_FLASHCHARGELEVEL, "FlashChargeLevel", N_("Flash Charge Level"), ""},
	{MNOTE_OLYMPUS_TAG_COLORMATRIX, "ColorMatrix", N_("Color Matrix"), ""},
	{MNOTE_OLYMPUS_TAG_BLACKLEVEL, "BlackLevel", N_("Black Level"), ""},
	{MNOTE_OLYMPUS_TAG_WBALANCE, "WhiteBalance", N_("White Balance Setting"), ""},
	{MNOTE_OLYMPUS_TAG_REDBALANCE, "RedBalance", N_("Red Balance"), ""},
	{MNOTE_OLYMPUS_TAG_BLUEBALANCE, "BlueBalance", N_("Blue Balance"), ""},
	{MNOTE_OLYMPUS_TAG_COLORMATRIXNUMBER, "ColorMatrixNumber", N_("Color Matrix Number"), ""},
	{MNOTE_OLYMPUS_TAG_SERIALNUMBER2, "SerialNumber", N_("Serial Number"), ""},
	{MNOTE_OLYMPUS_TAG_FLASHEXPOSURECOMP, "FlashExposureComp", N_("Flash Exposure Comp"), ""},
	{MNOTE_OLYMPUS_TAG_INTERNALFLASHTABLE, "InternalFlashTable", N_("Internal Flash Table"), ""},
	{MNOTE_OLYMPUS_TAG_EXTERNALFLASHGVALUE, "ExternalFlashGValue", N_("External Flash G Value"), ""},
	{MNOTE_OLYMPUS_TAG_EXTERNALFLASHBOUNCE, "ExternalFlashBounce", N_("External Flash Bounce"), ""},
	{MNOTE_OLYMPUS_TAG_EXTERNALFLASHZOOM, "ExternalFlashZoom", N_("External Flash Zoom"), ""},
	{MNOTE_OLYMPUS_TAG_EXTERNALFLASHMODE, "ExternalFlashMode", N_("External Flash Mode"), ""},
	{MNOTE_OLYMPUS_TAG_CONTRAST, "Contrast", N_("Contrast Setting"), ""},
	{MNOTE_OLYMPUS_TAG_SHARPNESSFACTOR, "SharpnessFactor", N_("Sharpness Factor"), ""},
	{MNOTE_OLYMPUS_TAG_COLORCONTROL, "ColorControl", N_("Color Control"), ""},
	{MNOTE_OLYMPUS_TAG_IMAGEWIDTH, "OlympusImageWidth", N_("Olympus Image Width"), ""},
	{MNOTE_OLYMPUS_TAG_IMAGEHEIGHT, "OlympusImageHeight", N_("Olympus Image Height"), ""},
	{MNOTE_OLYMPUS_TAG_SCENEDETECT, "SceneDetect", N_("Scene Detect"), ""},
	{MNOTE_OLYMPUS_TAG_COMPRESSIONRATIO, "CompressionRatio", N_("Compression Ratio"), ""},
	{MNOTE_OLYMPUS_TAG_PREVIEWIMAGEVALID, "PreviewImageValid", N_("Preview Image Valid"), ""},
	{MNOTE_OLYMPUS_TAG_AFRESULT, "AFResult", N_("AF Result"), ""},
	{MNOTE_OLYMPUS_TAG_CCDSCANMODE, "CCDScanMode", N_("CCD Scan Mode"), ""},
	{MNOTE_OLYMPUS_TAG_NOISEREDUCTION, "NoiseReduction", N_("Noise Reduction"), ""},
	{MNOTE_OLYMPUS_TAG_INFINITYLENSSTEP, "InfinityLensStep", N_("Infinity Lens Step"), ""},
	{MNOTE_OLYMPUS_TAG_NEARLENSSTEP, "NearLensStep", N_("Near Lens Step"), ""},
	{MNOTE_OLYMPUS_TAG_LIGHTVALUECENTER, "LightValueCenter", N_("Light Value Center"), ""},
	{MNOTE_OLYMPUS_TAG_LIGHTVALUEPERIPHERY, "LightValuePeriphery", N_("Light Value Periphery"), ""},

	/* Sanyo */
	{MNOTE_SANYO_TAG_SEQUENTIALSHOT, "SequentialShot", N_("Sequential Shot"), ""},
	{MNOTE_SANYO_TAG_WIDERANGE, "WideRange", N_("Wide Range"), ""},
	{MNOTE_SANYO_TAG_COLORADJUSTMENTMODE, "ColorAdjustmentMode", N_("Color Adjustment Mode"), ""},
	{MNOTE_SANYO_TAG_FOCUSMODE, "FocusMode", N_("Focus Mode"), ""},
	{MNOTE_SANYO_TAG_QUICKSHOT, "QuickShot", N_("Quick Shot"), ""},
	{MNOTE_SANYO_TAG_SELFTIMER, "SelfTimer", N_("Self-timer"), ""},
	{MNOTE_SANYO_TAG_VOICEMEMO, "VoiceMemo", N_("Voice Memo"), ""},
	{MNOTE_SANYO_TAG_RECORDSHUTTERRELEASE, "RecordShutterRelease", N_("Record Shutter Release"), ""},
	{MNOTE_SANYO_TAG_FLICKERREDUCE, "FlickerReduce", N_("Flicker Reduce"), ""},
	{MNOTE_SANYO_TAG_OPTICALZOOM, "OpticalZoom", N_("Optical Zoom"), ""},
	{MNOTE_SANYO_TAG_DIGITALZOOM, "DigitalZoom", N_("Digital Zoom"), ""},
	{MNOTE_SANYO_TAG_LIGHTSOURCESPECIAL, "LightSourceSpecial", N_("Light Source Special"), ""},
	{MNOTE_SANYO_TAG_RESAVED, "Resaved", N_("Resaved"), ""},
	{MNOTE_SANYO_TAG_CCDSENSITIVITY, "CCDSensitivity", N_("CCD Sensitivity"), ""},
	{MNOTE_SANYO_TAG_SCENESELECT, "SceneSelect", N_("Scene Select"), ""},
	{MNOTE_SANYO_TAG_MANUALFOCUSDISTANCE, "ManualFocusDistance", N_("Manual Focus Distance"), ""},
	{MNOTE_SANYO_TAG_SEQUENCESHOTINTERVAL, "SequenceShotInterval", N_("Sequence Shot Interval"), ""},

	/* Epson */
	{MNOTE_EPSON_TAG_IMAGE_WIDTH, "EpsonImageWidth", N_("Epson Image Width"), ""},
	{MNOTE_EPSON_TAG_IMAGE_HEIGHT, "EpsonImageHeight", N_("Epson Image Height"), ""},
	{MNOTE_EPSON_TAG_SOFTWARE, "EpsonSoftware", N_("Epson Software Version"), ""},
#endif
	{0, NULL, NULL, NULL}
};

const char *
mnote_olympus_tag_get_name (MnoteOlympusTag t)
{
	unsigned int i;

	for (i = 0; i < sizeof (table) / sizeof (table[0]); i++)
		if (table[i].tag == t) return (table[i].name);
	return NULL;
}

const char *
mnote_olympus_tag_get_title (MnoteOlympusTag t)
{
	unsigned int i;

	(void) bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	for (i = 0; i < sizeof (table) / sizeof (table[0]); i++)
		if (table[i].tag == t) return (_(table[i].title));
	return NULL;
}

const char *
mnote_olympus_tag_get_description (MnoteOlympusTag t)
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
