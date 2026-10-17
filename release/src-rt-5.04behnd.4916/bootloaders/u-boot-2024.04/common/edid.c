// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2010
 * Petr Stetiar <ynezz@true.cz>
 *
 * Contains stolen code from ddcprobe project which is:
 * Copyright (C) Nalin Dahyabhai <bigfun@pobox.com>
 */

#include <common.h>
#include <edid.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <linux/ctype.h>
#include <linux/string.h>

int edid_check_info(struct edid1_info *edid_info)
{
	if ((edid_info == NULL) || (edid_info->version == 0))
		return -1;

	if (memcmp(edid_info->header, "\x0\xff\xff\xff\xff\xff\xff\x0", 8))
		return -1;

	if (edid_info->version == 0xff && edid_info->revision == 0xff)
		return -1;

	return 0;
}

int edid_check_checksum(u8 *edid_block)
{
	u8 checksum = 0;
	int i;

	for (i = 0; i < 128; i++)
		checksum += edid_block[i];

	return (checksum == 0) ? 0 : -EINVAL;
}

int edid_get_ranges(struct edid1_info *edid, unsigned int *hmin,
		    unsigned int *hmax, unsigned int *vmin,
		    unsigned int *vmax)
{
	int i;
	struct edid_monitor_descriptor *monitor;

	*hmin = *hmax = *vmin = *vmax = 0;
	if (edid_check_info(edid))
		return -1;

	for (i = 0; i < ARRAY_SIZE(edid->monitor_details.descriptor); i++) {
		monitor = &edid->monitor_details.descriptor[i];
		if (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE) {
			*hmin = monitor->data.range_data.horizontal_min;
			*hmax = monitor->data.range_data.horizontal_max;
			*vmin = monitor->data.range_data.vertical_min;
			*vmax = monitor->data.range_data.vertical_max;
			return 0;
		}
	}
	return -1;
}

/* Set all parts of a timing entry to the same value */
static void set_entry(struct timing_entry *entry, u32 value)
{
	entry->min = value;
	entry->typ = value;
	entry->max = value;
}

/**
 * decode_timing() - Decoding an 18-byte detailed timing record
 *
 * @buf:	Pointer to EDID detailed timing record
 * @timing:	Place to put timing
 */
static void decode_timing(u8 *buf, struct display_timing *timing)
{
	uint x_mm, y_mm;
	unsigned int ha, hbl, hso, hspw, hborder;
	unsigned int va, vbl, vso, vspw, vborder;
	struct edid_detailed_timing *t = (struct edid_detailed_timing *)buf;

	/* Edid contains pixel clock in terms of 10KHz */
	set_entry(&timing->pixelclock, (buf[0] + (buf[1] << 8)) * 10000);
	x_mm = (buf[12] + ((buf[14] & 0xf0) << 4));
	y_mm = (buf[13] + ((buf[14] & 0x0f) << 8));
	ha = (buf[2] + ((buf[4] & 0xf0) << 4));
	hbl = (buf[3] + ((buf[4] & 0x0f) << 8));
	hso = (buf[8] + ((buf[11] & 0xc0) << 2));
	hspw = (buf[9] + ((buf[11] & 0x30) << 4));
	hborder = buf[15];
	va = (buf[5] + ((buf[7] & 0xf0) << 4));
	vbl = (buf[6] + ((buf[7] & 0x0f) << 8));
	vso = ((buf[10] >> 4) + ((buf[11] & 0x0c) << 2));
	vspw = ((buf[10] & 0x0f) + ((buf[11] & 0x03) << 4));
	vborder = buf[16];

	set_entry(&timing->hactive, ha);
	set_entry(&timing->hfront_porch, hso);
	set_entry(&timing->hback_porch, hbl - hso - hspw);
	set_entry(&timing->hsync_len, hspw);

	set_entry(&timing->vactive, va);
	set_entry(&timing->vfront_porch, vso);
	set_entry(&timing->vback_porch, vbl - vso - vspw);
	set_entry(&timing->vsync_len, vspw);

	timing->flags = 0;
	if (EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_HSYNC_HIGH;
	else
		timing->flags |= DISPLAY_FLAGS_HSYNC_LOW;
	if (EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_VSYNC_HIGH;
	else
		timing->flags |= DISPLAY_FLAGS_VSYNC_LOW;

	if (EDID_DETAILED_TIMING_FLAG_INTERLACED(*t))
		timing->flags = DISPLAY_FLAGS_INTERLACED;

	debug("Detailed mode clock %u Hz, %d mm x %d mm\n"
	      "               %04x %04x %04x %04x hborder %x\n"
	      "               %04x %04x %04x %04x vborder %x\n",
	      timing->pixelclock.typ,
	      x_mm, y_mm,
	      ha, ha + hso, ha + hso + hspw,
	      ha + hbl, hborder,
	      va, va + vso, va + vso + vspw,
	      va + vbl, vborder);
}

/**
 * Check if HDMI vendor specific data block is present in CEA block
 * @param info	CEA extension block
 * Return: true if block is found
 */
static bool cea_is_hdmi_vsdb_present(struct edid_cea861_info *info)
{
	u8 end, i = 0;

	/* check for end of data block */
	end = info->dtd_offset;
	if (end == 0)
		end = sizeof(info->data);
	if (end < 4 || end > sizeof(info->data))
		return false;
	end -= 4;

	while (i < end) {
		/* Look for vendor specific data block of appropriate size */
		if ((EDID_CEA861_DB_TYPE(*info, i) == EDID_CEA861_DB_VENDOR) &&
		    (EDID_CEA861_DB_LEN(*info, i) >= 5)) {
			u8 *db = &info->data[i + 1];
			u32 oui = db[0] | (db[1] << 8) | (db[2] << 16);

			if (oui == HDMI_IEEE_OUI)
				return true;
		}
		i += EDID_CEA861_DB_LEN(*info, i) + 1;
	}

	return false;
}

static bool edid_find_valid_timing(void *buf, int count,
				   struct display_timing *timing,
				   bool (*mode_valid)(void *priv,
					const struct display_timing *timing),
				   void *mode_valid_priv)
{
	struct edid_detailed_timing *t = buf;
	bool found = false;
	int i;

	for (i = 0; i < count && !found; i++, t++)
		if (EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) != 0) {
			decode_timing((u8 *)t, timing);
			if (mode_valid)
				found = mode_valid(mode_valid_priv,
						   timing);
			else
				found = true;
		}

	return found;
}

int edid_get_timing_validate(u8 *buf, int buf_size,
			     struct display_timing *timing,
			     int *panel_bits_per_colourp,
			     bool (*mode_valid)(void *priv,
					const struct display_timing *timing),
			     void *mode_valid_priv)
{
	struct edid1_info *edid = (struct edid1_info *)buf;
	bool found;

	if (buf_size < sizeof(*edid) || edid_check_info(edid)) {
		debug("%s: Invalid buffer\n", __func__);
		return -EINVAL;
	}

	if (!EDID1_INFO_VIDEO_INPUT_DIGITAL(*edid)) {
		debug("%s: Not a digital display\n", __func__);
		return -ENOSYS;
	}

	if (!EDID1_INFO_FEATURE_PREFERRED_TIMING_MODE(*edid)) {
		debug("%s: No preferred timing\n", __func__);
		return -ENOENT;
	}

	/* Look for detailed timing in base EDID */
	found = edid_find_valid_timing(edid->monitor_details.descriptor, 4,
				       timing, mode_valid, mode_valid_priv);

	/* Look for detailed timing in CTA-861 Extension Block */
	if (!found && edid->extension_flag && buf_size >= EDID_EXT_SIZE) {
		struct edid_cea861_info *info =
			(struct edid_cea861_info *)(buf + sizeof(*edid));

		if (info->extension_tag == EDID_CEA861_EXTENSION_TAG) {
			int count = EDID_CEA861_DTD_COUNT(*info);
			int offset = info->dtd_offset;
			int size = count * sizeof(struct edid_detailed_timing);

			if (offset >= 4 && offset + size < EDID_SIZE)
				found = edid_find_valid_timing(
					(u8 *)info + offset, count, timing,
					mode_valid, mode_valid_priv);
		}
	}

	if (!found)
		return -EINVAL;

	if (edid->version != 1 || edid->revision < 4) {
		debug("%s: EDID version %d.%d does not have required info\n",
		      __func__, edid->version, edid->revision);
		*panel_bits_per_colourp = -1;
	} else  {
		*panel_bits_per_colourp =
			((edid->video_input_definition & 0x70) >> 3) + 4;
	}

	timing->hdmi_monitor = false;
	if (edid->extension_flag && (buf_size >= EDID_EXT_SIZE)) {
		struct edid_cea861_info *info =
			(struct edid_cea861_info *)(buf + sizeof(*edid));

		if (info->extension_tag == EDID_CEA861_EXTENSION_TAG)
			timing->hdmi_monitor = cea_is_hdmi_vsdb_present(info);
	}

	return 0;
}

int edid_get_timing(u8 *buf, int buf_size, struct display_timing *timing,
		    int *panel_bits_per_colourp)
{
	return edid_get_timing_validate(buf, buf_size, timing,
					panel_bits_per_colourp, NULL, NULL);
}


/**
 * Snip the tailing whitespace/return of a string.
 *
 * @param string	The string to be snipped
 * Return: the snipped string
 */
static char *snip(char *string)
{
	char *s;

	/*
	 * This is always a 13 character buffer
	 * and it's not always terminated.
	 */
	string[12] = '\0';
	s = &string[strlen(string) - 1];

	while (s >= string && (isspace(*s) || *s == '\n' || *s == '\r' ||
			*s == '\0'))
		*(s--) = '\0';

	return string;
}

/**
 * Print an EDID monitor descriptor block
 *
 * @param monitor	The EDID monitor descriptor block
 * @have_timing		Modifies to 1 if the desciptor contains timing info
 */
static void edid_print_dtd(struct edid_monitor_descriptor *monitor,
			   unsigned int *have_timing)
{
	unsigned char *bytes = (unsigned char *)monitor;
	struct edid_detailed_timing *timing =
			(struct edid_detailed_timing *)monitor;

	if (bytes[0] == 0 && bytes[1] == 0) {
		if (monitor->type == EDID_MONITOR_DESCRIPTOR_SERIAL)
			printf("Monitor serial number: %s\n",
			       snip(monitor->data.string));
		else if (monitor->type == EDID_MONITOR_DESCRIPTOR_ASCII)
			printf("Monitor ID: %s\n",
			       snip(monitor->data.string));
		else if (monitor->type == EDID_MONITOR_DESCRIPTOR_NAME)
			printf("Monitor name: %s\n",
			       snip(monitor->data.string));
		else if (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE)
			printf("Monitor range limits, horizontal sync: "
			       "%d-%d kHz, vertical refresh: "
			       "%d-%d Hz, max pixel clock: "
			       "%d MHz\n",
			       monitor->data.range_data.horizontal_min,
			       monitor->data.range_data.horizontal_max,
			       monitor->data.range_data.vertical_min,
			       monitor->data.range_data.vertical_max,
			       monitor->data.range_data.pixel_clock_max * 10);
	} else {
		uint32_t pixclock, h_active, h_blanking, v_active, v_blanking;
		uint32_t h_total, v_total, vfreq;

		pixclock = EDID_DETAILED_TIMING_PIXEL_CLOCK(*timing);
		h_active = EDID_DETAILED_TIMING_HORIZONTAL_ACTIVE(*timing);
		h_blanking = EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*timing);
		v_active = EDID_DETAILED_TIMING_VERTICAL_ACTIVE(*timing);
		v_blanking = EDID_DETAILED_TIMING_VERTICAL_BLANKING(*timing);

		h_total = h_active + h_blanking;
		v_total = v_active + v_blanking;
		if (v_total > 0 && h_total > 0)
			vfreq = pixclock / (v_total * h_total);
		else
			vfreq = 1; /* Error case */
		printf("\t%dx%d\%c\t%d Hz (detailed)\n", h_active,
		       v_active, h_active > 1000 ? ' ' : '\t', vfreq);
		*have_timing = 1;
	}
}

/**
 * Get the manufacturer name from an EDID info.
 *
 * @param edid_info     The EDID info to be printed
 * @param name		Returns the string of the manufacturer name
 */
static void edid_get_manufacturer_name(struct edid1_info *edid, char *name)
{
	name[0] = EDID1_INFO_MANUFACTURER_NAME_CHAR1(*edid) + 'A' - 1;
	name[1] = EDID1_INFO_MANUFACTURER_NAME_CHAR2(*edid) + 'A' - 1;
	name[2] = EDID1_INFO_MANUFACTURER_NAME_CHAR3(*edid) + 'A' - 1;
	name[3] = '\0';
}

void edid_print_info(struct edid1_info *edid_info)
{
	int i;
	char manufacturer[4];
	unsigned int have_timing = 0;
	uint32_t serial_number;

	if (edid_check_info(edid_info)) {
		printf("Not a valid EDID\n");
		return;
	}

	printf("EDID version: %d.%d\n",
	       edid_info->version, edid_info->revision);

	printf("Product ID code: %04x\n", EDID1_INFO_PRODUCT_CODE(*edid_info));

	edid_get_manufacturer_name(edid_info, manufacturer);
	printf("Manufacturer: %s\n", manufacturer);

	serial_number = EDID1_INFO_SERIAL_NUMBER(*edid_info);
	if (serial_number != 0xffffffff) {
		if (strcmp(manufacturer, "MAG") == 0)
			serial_number -= 0x7000000;
		if (strcmp(manufacturer, "OQI") == 0)
			serial_number -= 456150000;
		if (strcmp(manufacturer, "VSC") == 0)
			serial_number -= 640000000;
	}
	printf("Serial number: %08x\n", serial_number);
	printf("Manufactured in week: %d year: %d\n",
	       edid_info->week, edid_info->year + 1990);

	printf("Video input definition: %svoltage level %d%s%s%s%s%s\n",
	       EDID1_INFO_VIDEO_INPUT_DIGITAL(*edid_info) ?
	       "digital signal, " : "analog signal, ",
	       EDID1_INFO_VIDEO_INPUT_VOLTAGE_LEVEL(*edid_info),
	       EDID1_INFO_VIDEO_INPUT_BLANK_TO_BLACK(*edid_info) ?
	       ", blank to black" : "",
	       EDID1_INFO_VIDEO_INPUT_SEPARATE_SYNC(*edid_info) ?
	       ", separate sync" : "",
	       EDID1_INFO_VIDEO_INPUT_COMPOSITE_SYNC(*edid_info) ?
	       ", composite sync" : "",
	       EDID1_INFO_VIDEO_INPUT_SYNC_ON_GREEN(*edid_info) ?
	       ", sync on green" : "",
	       EDID1_INFO_VIDEO_INPUT_SERRATION_V(*edid_info) ?
	       ", serration v" : "");

	printf("Monitor is %s\n",
	       EDID1_INFO_FEATURE_RGB(*edid_info) ? "RGB" : "non-RGB");

	printf("Maximum visible display size: %d cm x %d cm\n",
	       edid_info->max_size_horizontal,
	       edid_info->max_size_vertical);

	printf("Power management features: %s%s, %s%s, %s%s\n",
	       EDID1_INFO_FEATURE_ACTIVE_OFF(*edid_info) ?
	       "" : "no ", "active off",
	       EDID1_INFO_FEATURE_SUSPEND(*edid_info) ? "" : "no ", "suspend",
	       EDID1_INFO_FEATURE_STANDBY(*edid_info) ? "" : "no ", "standby");

	printf("Estabilished timings:\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_720X400_70(*edid_info))
		printf("\t720x400\t\t70 Hz (VGA 640x400, IBM)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_720X400_88(*edid_info))
		printf("\t720x400\t\t88 Hz (XGA2)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_640X480_60(*edid_info))
		printf("\t640x480\t\t60 Hz (VGA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_640X480_67(*edid_info))
		printf("\t640x480\t\t67 Hz (Mac II, Apple)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_640X480_72(*edid_info))
		printf("\t640x480\t\t72 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_640X480_75(*edid_info))
		printf("\t640x480\t\t75 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_800X600_56(*edid_info))
		printf("\t800x600\t\t56 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_800X600_60(*edid_info))
		printf("\t800x600\t\t60 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_800X600_72(*edid_info))
		printf("\t800x600\t\t72 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_800X600_75(*edid_info))
		printf("\t800x600\t\t75 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_832X624_75(*edid_info))
		printf("\t832x624\t\t75 Hz (Mac II)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_1024X768_87I(*edid_info))
		printf("\t1024x768\t87 Hz Interlaced (8514A)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_1024X768_60(*edid_info))
		printf("\t1024x768\t60 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_1024X768_70(*edid_info))
		printf("\t1024x768\t70 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_1024X768_75(*edid_info))
		printf("\t1024x768\t75 Hz (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_1280X1024_75(*edid_info))
		printf("\t1280x1024\t75 (VESA)\n");
	if (EDID1_INFO_ESTABLISHED_TIMING_1152X870_75(*edid_info))
		printf("\t1152x870\t75 (Mac II)\n");

	/* Standard timings. */
	printf("Standard timings:\n");
	for (i = 0; i < ARRAY_SIZE(edid_info->standard_timings); i++) {
		unsigned int aspect = 10000;
		unsigned int x, y;
		unsigned char xres, vfreq;

		xres = EDID1_INFO_STANDARD_TIMING_XRESOLUTION(*edid_info, i);
		vfreq = EDID1_INFO_STANDARD_TIMING_VFREQ(*edid_info, i);
		if ((xres != vfreq) ||
		    ((xres != 0) && (xres != 1)) ||
		    ((vfreq != 0) && (vfreq != 1))) {
			switch (EDID1_INFO_STANDARD_TIMING_ASPECT(*edid_info,
					i)) {
			case ASPECT_625:
				aspect = 6250;
				break;
			case ASPECT_75:
				aspect = 7500;
				break;
			case ASPECT_8:
				aspect = 8000;
				break;
			case ASPECT_5625:
				aspect = 5625;
				break;
			}
			x = (xres + 31) * 8;
			y = x * aspect / 10000;
			printf("\t%dx%d%c\t%d Hz\n", x, y,
			       x > 1000 ? ' ' : '\t', (vfreq & 0x3f) + 60);
			have_timing = 1;
		}
	}

	/* Detailed timing information. */
	for (i = 0; i < ARRAY_SIZE(edid_info->monitor_details.descriptor);
			i++) {
		edid_print_dtd(&edid_info->monitor_details.descriptor[i],
			       &have_timing);
	}

	if (!have_timing)
		printf("\tNone\n");
}
