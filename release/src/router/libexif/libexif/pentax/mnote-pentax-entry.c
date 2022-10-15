/* mnote-pentax-entry.c
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
#include "mnote-pentax-entry.h"

#include <libexif/i18n.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libexif/exif-format.h>
#include <libexif/exif-utils.h>
#include <libexif/exif-entry.h>


#define CF(format,target,v,maxlen)                              \
{                                                               \
	if (format != target) {                                 \
		snprintf (v, maxlen,	                        \
			_("Invalid format '%s', "               \
			"expected '%s'."),                      \
			exif_format_get_name (format),          \
			exif_format_get_name (target));         \
		break;                                          \
	}                                                       \
}

#define CC(number,target,v,maxlen)                                      \
{                                                                       \
	if (number != target) {                                         \
		snprintf (v, maxlen,                                    \
			_("Invalid number of components (%i, "          \
			"expected %i)."), (int) number, (int) target);  \
		break;                                                  \
	}                                                               \
}

#define CC2(number,t1,t2,v,maxlen)                                      \
{                                                                       \
	if ((number != t1) && (number != t2)) {                         \
		snprintf (v, maxlen,                                    \
			_("Invalid number of components (%i, "          \
			"expected %i or %i)."), (int) number,		\
			(int) t1, (int) t2);  				\
		break;                                                  \
	}                                                               \
}

static const struct {
	MnotePentaxTag tag;
	struct {
		int index;
		const char *string;
	} elem[33];
} items[] = {
#ifndef NO_VERBOSE_TAG_DATA
  { MNOTE_PENTAX_TAG_MODE,
    { {0, N_("Auto")},
      {1, N_("Night scene")},
      {2, N_("Manual")},
      {4, N_("Multi-exposure")},
      {0, NULL}}},
  { MNOTE_PENTAX_TAG_QUALITY,
    { {0, N_("Good")},
      {1, N_("Better")},
      {2, N_("Best")},{0,NULL}}},
  { MNOTE_PENTAX_TAG_FOCUS,
    { {2, N_("Custom")},
      {3, N_("Auto")},
      {0, NULL}}},
  { MNOTE_PENTAX_TAG_FLASH,
    { {1, N_("Auto")},
      {2, N_("Flash on")},
      {4, N_("Flash off")},
      {6, N_("Red-eye reduction")},
      {0, NULL}}},
  { MNOTE_PENTAX_TAG_WHITE_BALANCE,
    { {0, N_("Auto")},
      {1, N_("Daylight")},
      {2, N_("Shade")},
      {3, N_("Tungsten")},
      {4, N_("Fluorescent")},
      {5, N_("Manual")},
      {0, NULL}}},
  { MNOTE_PENTAX_TAG_SHARPNESS,
    { {0, N_("Normal")},
      {1, N_("Soft")},
      {2, N_("Hard")},
      {0, NULL}}},
  { MNOTE_PENTAX_TAG_CONTRAST,
    { {0, N_("Normal")},
      {1, N_("Low")},
      {2, N_("High")},
      {0, NULL}}},
  { MNOTE_PENTAX_TAG_SATURATION,
    { {0, N_("Normal")},
      {1, N_("Low")},
      {2, N_("High")},
      {0, NULL}}},
  { MNOTE_PENTAX_TAG_ISO_SPEED,
    { {10,  N_("100")},
      {16,  N_("200")},
      {100, N_("100")},
      {200, N_("200")},
      { 0,  NULL}}},
  { MNOTE_PENTAX_TAG_COLOR,
    { {1, N_("Full")},
      {2, N_("Black & white")},
      {3, N_("Sepia")},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_MODE,
    { {0, N_("Auto")},
      {1, N_("Night scene")},
      {2, N_("Manual")},
      {4, N_("Multi-exposure")},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_QUALITY,
    { {0, N_("Good")},
      {1, N_("Better")},
      {2, N_("Best")},
      {3, N_("TIFF")},
      {4, N_("RAW")},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_IMAGE_SIZE,
    { {0, "640x480"},
      {1, N_("Full")},
      {2, "1024x768"},
      {3, "1280x960"},
      {4, "1600x1200"},
      {5, "2048x1536"},
      {8, N_("2560x1920 or 2304x1728")},
      {9, "3072x2304"},
      {10, "3264x2448"},
      {19, "320x240"},
      {20, "2288x1712"},
      {21, "2592x1944"},
      {22, N_("2304x1728 or 2592x1944")},
      {23, "3056x2296"},
      {25, N_("2816x2212 or 2816x2112")},
      {27, "3648x2736"},
      {36, "3008x2008"},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_PICTURE_MODE,
    { {0, N_("Program")},
      {2, N_("Program AE")},
      {3, N_("Manual")},
      {5, N_("Portrait")},
      {6, N_("Landscape")},
      {8, N_("Sport")},
      {9, N_("Night scene")},
      {11, N_("Soft")},
      {12, N_("Surf & snow")},
      {13, N_("Sunset or candlelight")},
      {14, N_("Autumn")},
      {15, N_("Macro")},
      {17, N_("Fireworks")},
      {18, N_("Text")},
      {19, N_("Panorama")},
      {30, N_("Self portrait")},
      {31, N_("Illustrations")},
      {33, N_("Digital filter")},
      {37, N_("Museum")},
      {38, N_("Food")},
      {40, N_("Green mode")},
      {49, N_("Light pet")},
      {50, N_("Dark pet")},
      {51, N_("Medium pet")},
      {53, N_("Underwater")},
      {54, N_("Candlelight")},
      {55, N_("Natural skin tone")},
      {56, N_("Synchro sound record")},
      {58, N_("Frame composite")},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_FLASH_MODE,
    { {0x0000, N_("Auto, did not fire")},
      {0x0001, N_("Off")},
      {0x0003, N_("Auto, did not fire, red-eye reduction")},
      {0x0100, N_("Auto, fired")},
      {0x0102, N_("On")},
      {0x0103, N_("Auto, fired, red-eye reduction")},
      {0x0104, N_("On, red-eye reduction")},
      {0x0105, N_("On, wireless")},
      {0x0108, N_("On, soft")},
      {0x0109, N_("On, slow-sync")},
      {0x010a, N_("On, slow-sync, red-eye reduction")},
      {0x010b, N_("On, trailing-curtain sync")},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_FOCUS_MODE,
    { {0, N_("Normal")},
      {1, N_("Macro")},
      {2, N_("Infinity")},
      {3, N_("Manual")},
      {5, N_("Pan focus")},
      {16, N_("AF-S")},
      {17, N_("AF-C")},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_AFPOINT_SELECTED,
    { {1, N_("Upper-left")},
      {2, N_("Top")},
      {3, N_("Upper-right")},
      {4, N_("Left")},
      {5, N_("Mid-left")},
      {6, N_("Center")},
      {7, N_("Mid-right")},
      {8, N_("Right")},
      {9, N_("Lower-left")},
      {10, N_("Bottom")},
      {11, N_("Lower-right")},
      {0xfffe, N_("Fixed center")},
      {0xffff, N_("Auto")},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_AUTO_AFPOINT,
    { {0, N_("Multiple")},
      {1, N_("Top-left")},
      {2, N_("Top-center")},
      {3, N_("Top-right")},
      {4, N_("Left")},
      {5, N_("Center")},
      {6, N_("Right")},
      {7, N_("Bottom-left")},
      {8, N_("Bottom-center")},
      {9, N_("Bottom-right")},
      {0xffff, N_("None")},
      {0, NULL}}},
  { MNOTE_PENTAX2_TAG_WHITE_BALANCE,
    { {0, N_("Auto")},
      {1, N_("Daylight")},
      {2, N_("Shade")},
      {3, N_("Fluorescent")},
      {4, N_("Tungsten")},
      {5, N_("Manual")},
      {6, N_("Daylight fluorescent")},
      {7, N_("Day white fluorescent")},
      {8, N_("White fluorescent")},
      {9, N_("Flash")},
      {10, N_("Cloudy")},
      {0xfffe, N_("Unknown")},
      {0xffff, N_("User selected")},
      {0, NULL}}},
  {MNOTE_CASIO2_TAG_BESTSHOT_MODE, 
    { {0, N_("Off")},
      {1, N_("On")},
      {0, NULL}}},
#endif
  {0, {{0, NULL}}}
};

/* Two-component values */
static const struct {
	MnotePentaxTag tag;
	struct {
		int index1, index2;
		const char *string;
	} elem[39];
} items2[] = {
#ifndef NO_VERBOSE_TAG_DATA
  { MNOTE_PENTAX2_TAG_IMAGE_SIZE,
    { {0, 0, "2304x1728"},
      {4, 0, "1600x1200"},
      {5, 0, "2048x1536"},
      {8, 0, "2560x1920"},
      {34, 0, "1536x1024"},
      {36, 0, N_("3008x2008 or 3040x2024")},
      {37, 0, "3008x2000"},
      {35, 1, "2400x1600"},
      {32, 2, "960x480"},
      {33, 2, "1152x768"},
      {34, 2, "1536x1024"},
      {0,  0, NULL}}},
  { MNOTE_PENTAX2_TAG_PICTURE_MODE,
    { {0,   0, N_("Auto")},
      {5,   0, N_("Portrait")},
      {53,  0, N_("Underwater")},
      {255, 0, N_("Digital filter?")},
      {5,   1, N_("Portrait")},
      {9,   1, N_("Night scene")},
      {13,  1, N_("Candlelight")},
      {15,  1, N_("Macro")},
      {53,  1, N_("Underwater")},
      {0,   2, N_("Program AE")},
      {5,   2, N_("Portrait")},
      {6,   2, N_("Landscape")},
      {0,   0, NULL}}},
#endif
  {0, {{0, 0, NULL}}}
};

char *
mnote_pentax_entry_get_value (MnotePentaxEntry *entry,
			      char *val, unsigned int maxlen)
{
	ExifLong vl;
	ExifSLong vsl;
	ExifShort vs, vs2;
	ExifSShort vss;
	int i = 0, j = 0;

	if (!entry) return (NULL);

	memset (val, 0, maxlen);
	maxlen--;

	switch (entry->tag) {
	  case MNOTE_PENTAX_TAG_MODE:
	  case MNOTE_PENTAX_TAG_QUALITY:
	  case MNOTE_PENTAX_TAG_FOCUS:
	  case MNOTE_PENTAX_TAG_FLASH:
	  case MNOTE_PENTAX_TAG_WHITE_BALANCE:
	  case MNOTE_PENTAX_TAG_SHARPNESS:
	  case MNOTE_PENTAX_TAG_CONTRAST:
	  case MNOTE_PENTAX_TAG_SATURATION:
	  case MNOTE_PENTAX_TAG_ISO_SPEED:
	  case MNOTE_PENTAX_TAG_COLOR:
	  case MNOTE_PENTAX2_TAG_MODE:
	  case MNOTE_PENTAX2_TAG_QUALITY:
	  case MNOTE_PENTAX2_TAG_FLASH_MODE:
	  case MNOTE_PENTAX2_TAG_FOCUS_MODE:
	  case MNOTE_PENTAX2_TAG_AFPOINT_SELECTED:
	  case MNOTE_PENTAX2_TAG_AUTO_AFPOINT:
	  case MNOTE_PENTAX2_TAG_WHITE_BALANCE:
	  case MNOTE_PENTAX2_TAG_PICTURE_MODE:
	  case MNOTE_PENTAX2_TAG_IMAGE_SIZE:
	  case MNOTE_CASIO2_TAG_BESTSHOT_MODE:
		CF (entry->format, EXIF_FORMAT_SHORT, val, maxlen);
		CC2 (entry->components, 1, 2, val, maxlen);
		if (entry->components == 1) {
			vs = exif_get_short (entry->data, entry->order);

			/* search the tag */
			for (i = 0; (items[i].tag && items[i].tag != entry->tag); i++);
			if (!items[i].tag) {
				snprintf (val, maxlen,
					  _("Internal error (unknown value %hu)"), vs);
			  	break;
			}

			/* find the value */
			for (j = 0; items[i].elem[j].string &&
			    (items[i].elem[j].index < vs); j++);
			if (items[i].elem[j].index != vs) {
				snprintf (val, maxlen,
					  _("Internal error (unknown value %hu)"), vs);
				break;
			}
			strncpy (val, _(items[i].elem[j].string), maxlen);
		} else {
			/* Two-component values */
			CF (entry->format, EXIF_FORMAT_SHORT, val, maxlen);
			CC2 (entry->components, 1, 2, val, maxlen);
			vs = exif_get_short (entry->data, entry->order);
			vs2 = ((unsigned int)exif_get_short (entry->data+2, entry->order)) << 16;

			/* search the tag */
			for (i = 0; (items2[i].tag && items2[i].tag != entry->tag); i++);
			if (!items2[i].tag) {
				snprintf (val, maxlen,
					  _("Internal error (unknown value %hu %hu)"), vs, vs2);
			  	break;
			}

			/* find the value */
			for (j = 0; items2[i].elem[j].string && ((items2[i].elem[j].index2 < vs2)
				|| ((items2[i].elem[j].index2 == vs2) && (items2[i].elem[j].index1 < vs))); j++);
			if ((items2[i].elem[j].index1 != vs) || (items2[i].elem[j].index2 != vs2)) {
				snprintf (val, maxlen,
					  _("Internal error (unknown value %hi %hi)"), vs, vs2);
				break;
			}
			strncpy (val, _(items2[i].elem[j].string), maxlen);
		}
		break;

	case MNOTE_PENTAX_TAG_ZOOM:
		CF (entry->format, EXIF_FORMAT_LONG, val, maxlen);
		CC (entry->components, 1, val, maxlen);
		vl = exif_get_long (entry->data, entry->order);
		snprintf (val, maxlen, "%lu", (long unsigned) vl);
		break;
	case MNOTE_PENTAX_TAG_PRINTIM:
		CF (entry->format, EXIF_FORMAT_UNDEFINED, val, maxlen);
		CC (entry->components, 124, val, maxlen);
		snprintf (val, maxlen, _("%i bytes unknown data"),
			entry->size);
		break;
	case MNOTE_PENTAX_TAG_TZ_CITY:
	case MNOTE_PENTAX_TAG_TZ_DST:
		CF (entry->format, EXIF_FORMAT_UNDEFINED, val, maxlen);
		CC (entry->components, 4, val, maxlen);
		strncpy (val, (char*)entry->data, MIN(maxlen, entry->size));
		break;
	case MNOTE_PENTAX2_TAG_DATE:
		CF (entry->format, EXIF_FORMAT_UNDEFINED, val, maxlen);
		CC (entry->components, 4, val, maxlen);
		/* Note: format is UNDEFINED, not SHORT -> order is fixed: MOTOROLA */
		vs = exif_get_short (entry->data, EXIF_BYTE_ORDER_MOTOROLA);
		snprintf (val, maxlen, "%hi:%02i:%02i", vs, entry->data[2], entry->data[3]);
		break;
	case MNOTE_PENTAX2_TAG_TIME:
		CF (entry->format, EXIF_FORMAT_UNDEFINED, val, maxlen);
		CC2 (entry->components, 3, 4, val, maxlen);
		snprintf (val, maxlen, "%02i:%02i:%02i", entry->data[0], entry->data[1], entry->data[2]);
		break;
	default:
		switch (entry->format) {
		case EXIF_FORMAT_ASCII:
		  strncpy (val, (char *)entry->data, MIN(maxlen, entry->size));
		  break;
		case EXIF_FORMAT_SHORT:
		  {
			const unsigned char *data = entry->data;
			size_t k, len = strlen(val), sizeleft;

			sizeleft = entry->size;
			for(k=0; k<entry->components; k++) {
				if (sizeleft < 2)
					break;
				vs = exif_get_short (data, entry->order);
				snprintf (val+len, maxlen-len, "%hu ", vs);
				len = strlen(val);
				data += 2;
				sizeleft -= 2;
			}
		  }
		  break;
		case EXIF_FORMAT_SSHORT:
		  {
			const unsigned char *data = entry->data;
			size_t k, len = strlen(val), sizeleft;

			sizeleft = entry->size;
			for(k=0; k<entry->components; k++) {
				if (sizeleft < 2)
					break;
				vss = exif_get_sshort (data, entry->order);
				snprintf (val+len, maxlen-len, "%hi ", vss);
				len = strlen(val);
				data += 2;
				sizeleft -= 2;
			}
		  }
		  break;
		case EXIF_FORMAT_LONG:
		  {
			const unsigned char *data = entry->data;
			size_t k, len = strlen(val), sizeleft;

			sizeleft = entry->size;
			for(k=0; k<entry->components; k++) {
				if (sizeleft < 4)
					break;
				vl = exif_get_long (data, entry->order);
				snprintf (val+len, maxlen-len, "%lu ", (long unsigned) vl);
				len = strlen(val);
				data += 4;
				sizeleft -= 4;
			}
		  }
		  break;
		case EXIF_FORMAT_SLONG:
		  {
			const unsigned char *data = entry->data;
			size_t k, len = strlen(val), sizeleft;

			sizeleft = entry->size;
			for(k=0; k<entry->components; k++) {
				if (sizeleft < 4)
					break;
				vsl = exif_get_slong (data, entry->order);
				snprintf (val+len, maxlen-len, "%li ", (long int) vsl);
				len = strlen(val);
				data += 4;
				sizeleft -= 4;
			}
		  }
		  break;
		case EXIF_FORMAT_UNDEFINED:
		default:
		  snprintf (val, maxlen, _("%i bytes unknown data"),
			  entry->size);
		  break;
		}
		break;
	}

	return val;
}
