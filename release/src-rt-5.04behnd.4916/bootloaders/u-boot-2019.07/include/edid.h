/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2010
 * Petr Stetiar <ynezz@true.cz>
 *
 * Contains stolen code from ddcprobe project which is:
 * Copyright (C) Nalin Dahyabhai <bigfun@pobox.com>
 */

#ifndef __EDID_H_
#define __EDID_H_

#include <linux/types.h>

/* Size of the EDID data */
#define EDID_SIZE	128
#define EDID_EXT_SIZE	256

/* OUI of HDMI vendor specific data block */
#define HDMI_IEEE_OUI 0x000c03

#define GET_BIT(_x, _pos) \
	(((_x) >> (_pos)) & 1)
#define GET_BITS(_x, _pos_msb, _pos_lsb) \
	(((_x) >> (_pos_lsb)) & ((1 << ((_pos_msb) - (_pos_lsb) + 1)) - 1))

/* Aspect ratios used in EDID info. */
enum edid_aspect {
	ASPECT_625 = 0,
	ASPECT_75,
	ASPECT_8,
	ASPECT_5625,
};

/* Detailed timing information used in EDID v1.x */
struct edid_detailed_timing {
	unsigned char pixel_clock[2];
#define EDID_DETAILED_TIMING_PIXEL_CLOCK(_x) \
	(((((uint32_t)(_x).pixel_clock[1]) << 8) + \
	 (_x).pixel_clock[0]) * 10000)
	unsigned char horizontal_active;
	unsigned char horizontal_blanking;
	unsigned char horizontal_active_blanking_hi;
#define EDID_DETAILED_TIMING_HORIZONTAL_ACTIVE(_x) \
	((GET_BITS((_x).horizontal_active_blanking_hi, 7, 4) << 8) + \
	 (_x).horizontal_active)
#define EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(_x) \
	((GET_BITS((_x).horizontal_active_blanking_hi, 3, 0) << 8) + \
	 (_x).horizontal_blanking)
	unsigned char vertical_active;
	unsigned char vertical_blanking;
	unsigned char vertical_active_blanking_hi;
#define EDID_DETAILED_TIMING_VERTICAL_ACTIVE(_x) \
	((GET_BITS((_x).vertical_active_blanking_hi, 7, 4) << 8) + \
	 (_x).vertical_active)
#define EDID_DETAILED_TIMING_VERTICAL_BLANKING(_x) \
	((GET_BITS((_x).vertical_active_blanking_hi, 3, 0) << 8) + \
	 (_x).vertical_blanking)
	unsigned char hsync_offset;
	unsigned char hsync_pulse_width;
	unsigned char vsync_offset_pulse_width;
	unsigned char hsync_vsync_offset_pulse_width_hi;
#define EDID_DETAILED_TIMING_HSYNC_OFFSET(_x) \
	((GET_BITS((_x).hsync_vsync_offset_pulse_width_hi, 7, 6) << 8) + \
	 (_x).hsync_offset)
#define EDID_DETAILED_TIMING_HSYNC_PULSE_WIDTH(_x) \
	((GET_BITS((_x).hsync_vsync_offset_pulse_width_hi, 5, 4) << 8) + \
	 (_x).hsync_pulse_width)
#define EDID_DETAILED_TIMING_VSYNC_OFFSET(_x) \
	((GET_BITS((_x).hsync_vsync_offset_pulse_width_hi, 3, 2) << 4) + \
	 GET_BITS((_x).vsync_offset_pulse_width, 7, 4))
#define EDID_DETAILED_TIMING_VSYNC_PULSE_WIDTH(_x) \
	((GET_BITS((_x).hsync_vsync_offset_pulse_width_hi, 1, 0) << 4) + \
	 GET_BITS((_x).vsync_offset_pulse_width, 3, 0))
	unsigned char himage_size;
	unsigned char vimage_size;
	unsigned char himage_vimage_size_hi;
#define EDID_DETAILED_TIMING_HIMAGE_SIZE(_x) \
	((GET_BITS((_x).himage_vimage_size_hi, 7, 4) << 8) + (_x).himage_size)
#define EDID_DETAILED_TIMING_VIMAGE_SIZE(_x) \
	((GET_BITS((_x).himage_vimage_size_hi, 3, 0) << 8) + (_x).vimage_size)
	unsigned char hborder;
	unsigned char vborder;
	unsigned char flags;
#define EDID_DETAILED_TIMING_FLAG_INTERLACED(_x) \
	GET_BIT((_x).flags, 7)
#define EDID_DETAILED_TIMING_FLAG_STEREO(_x) \
	GET_BITS((_x).flags, 6, 5)
#define EDID_DETAILED_TIMING_FLAG_DIGITAL_COMPOSITE(_x) \
	GET_BITS((_x).flags, 4, 3)
#define EDID_DETAILED_TIMING_FLAG_POLARITY(_x) \
	GET_BITS((_x).flags, 2, 1)
#define EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(_x) \
	GET_BIT((_x).flags, 2)
#define EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(_x) \
	GET_BIT((_x).flags, 1)
#define EDID_DETAILED_TIMING_FLAG_INTERLEAVED(_x) \
	GET_BIT((_x).flags, 0)
} __attribute__ ((__packed__));

enum edid_monitor_descriptor_types {
	EDID_MONITOR_DESCRIPTOR_SERIAL = 0xff,
	EDID_MONITOR_DESCRIPTOR_ASCII = 0xfe,
	EDID_MONITOR_DESCRIPTOR_RANGE = 0xfd,
	EDID_MONITOR_DESCRIPTOR_NAME = 0xfc,
};

struct edid_monitor_descriptor {
	uint16_t zero_flag_1;
	unsigned char zero_flag_2;
	unsigned char type;
	unsigned char zero_flag_3;
	union {
		char string[13];
		struct {
			unsigned char vertical_min;
			unsigned char vertical_max;
			unsigned char horizontal_min;
			unsigned char horizontal_max;
			unsigned char pixel_clock_max;
			unsigned char gtf_data[8];
		} range_data;
	} data;
} __attribute__ ((__packed__));

struct edid1_info {
	unsigned char header[8];
	unsigned char manufacturer_name[2];
#define EDID1_INFO_MANUFACTURER_NAME_ZERO(_x) \
	GET_BIT(((_x).manufacturer_name[0]), 7)
#define EDID1_INFO_MANUFACTURER_NAME_CHAR1(_x) \
	GET_BITS(((_x).manufacturer_name[0]), 6, 2)
#define EDID1_INFO_MANUFACTURER_NAME_CHAR2(_x) \
	((GET_BITS(((_x).manufacturer_name[0]), 1, 0) << 3) + \
	 GET_BITS(((_x).manufacturer_name[1]), 7, 5))
#define EDID1_INFO_MANUFACTURER_NAME_CHAR3(_x) \
	GET_BITS(((_x).manufacturer_name[1]), 4, 0)
	unsigned char product_code[2];
#define EDID1_INFO_PRODUCT_CODE(_x) \
	(((uint16_t)(_x).product_code[1] << 8) + (_x).product_code[0])
	unsigned char serial_number[4];
#define EDID1_INFO_SERIAL_NUMBER(_x) \
	(((uint32_t)(_x).serial_number[3] << 24) + \
	 ((_x).serial_number[2] << 16) + ((_x).serial_number[1] << 8) + \
	 (_x).serial_number[0])
	unsigned char week;
	unsigned char year;
	unsigned char version;
	unsigned char revision;
	unsigned char video_input_definition;
#define EDID1_INFO_VIDEO_INPUT_DIGITAL(_x) \
	GET_BIT(((_x).video_input_definition), 7)
#define EDID1_INFO_VIDEO_INPUT_VOLTAGE_LEVEL(_x) \
	GET_BITS(((_x).video_input_definition), 6, 5)
#define EDID1_INFO_VIDEO_INPUT_BLANK_TO_BLACK(_x) \
	GET_BIT(((_x).video_input_definition), 4)
#define EDID1_INFO_VIDEO_INPUT_SEPARATE_SYNC(_x) \
	GET_BIT(((_x).video_input_definition), 3)
#define EDID1_INFO_VIDEO_INPUT_COMPOSITE_SYNC(_x) \
	GET_BIT(((_x).video_input_definition), 2)
#define EDID1_INFO_VIDEO_INPUT_SYNC_ON_GREEN(_x) \
	GET_BIT(((_x).video_input_definition), 1)
#define EDID1_INFO_VIDEO_INPUT_SERRATION_V(_x) \
	GET_BIT(((_x).video_input_definition), 0)
	unsigned char max_size_horizontal;
	unsigned char max_size_vertical;
	unsigned char gamma;
	unsigned char feature_support;
#define EDID1_INFO_FEATURE_STANDBY(_x) \
	GET_BIT(((_x).feature_support), 7)
#define EDID1_INFO_FEATURE_SUSPEND(_x) \
	GET_BIT(((_x).feature_support), 6)
#define EDID1_INFO_FEATURE_ACTIVE_OFF(_x) \
	GET_BIT(((_x).feature_support), 5)
#define EDID1_INFO_FEATURE_DISPLAY_TYPE(_x) \
	GET_BITS(((_x).feature_support), 4, 3)
#define EDID1_INFO_FEATURE_RGB(_x) \
	GET_BIT(((_x).feature_support), 2)
#define EDID1_INFO_FEATURE_PREFERRED_TIMING_MODE(_x) \
	GET_BIT(((_x).feature_support), 1)
#define EDID1_INFO_FEATURE_DEFAULT_GTF_SUPPORT(_x) \
	GET_BIT(((_x).feature_support), 0)
	unsigned char color_characteristics[10];
	unsigned char established_timings[3];
#define EDID1_INFO_ESTABLISHED_TIMING_720X400_70(_x) \
	GET_BIT(((_x).established_timings[0]), 7)
#define EDID1_INFO_ESTABLISHED_TIMING_720X400_88(_x) \
	GET_BIT(((_x).established_timings[0]), 6)
#define EDID1_INFO_ESTABLISHED_TIMING_640X480_60(_x) \
	GET_BIT(((_x).established_timings[0]), 5)
#define EDID1_INFO_ESTABLISHED_TIMING_640X480_67(_x) \
	GET_BIT(((_x).established_timings[0]), 4)
#define EDID1_INFO_ESTABLISHED_TIMING_640X480_72(_x) \
	GET_BIT(((_x).established_timings[0]), 3)
#define EDID1_INFO_ESTABLISHED_TIMING_640X480_75(_x) \
	GET_BIT(((_x).established_timings[0]), 2)
#define EDID1_INFO_ESTABLISHED_TIMING_800X600_56(_x) \
	GET_BIT(((_x).established_timings[0]), 1)
#define EDID1_INFO_ESTABLISHED_TIMING_800X600_60(_x) \
	GET_BIT(((_x).established_timings[0]), 0)
#define EDID1_INFO_ESTABLISHED_TIMING_800X600_72(_x) \
	GET_BIT(((_x).established_timings[1]), 7)
#define EDID1_INFO_ESTABLISHED_TIMING_800X600_75(_x) \
	GET_BIT(((_x).established_timings[1]), 6)
#define EDID1_INFO_ESTABLISHED_TIMING_832X624_75(_x) \
	GET_BIT(((_x).established_timings[1]), 5)
#define EDID1_INFO_ESTABLISHED_TIMING_1024X768_87I(_x) \
	GET_BIT(((_x).established_timings[1]), 4)
#define EDID1_INFO_ESTABLISHED_TIMING_1024X768_60(_x) \
	GET_BIT(((_x).established_timings[1]), 3)
#define EDID1_INFO_ESTABLISHED_TIMING_1024X768_70(_x) \
	GET_BIT(((_x).established_timings[1]), 2)
#define EDID1_INFO_ESTABLISHED_TIMING_1024X768_75(_x) \
	GET_BIT(((_x).established_timings[1]), 1)
#define EDID1_INFO_ESTABLISHED_TIMING_1280X1024_75(_x) \
	GET_BIT(((_x).established_timings[1]), 0)
#define EDID1_INFO_ESTABLISHED_TIMING_1152X870_75(_x) \
	GET_BIT(((_x).established_timings[2]), 7)
	struct {
		unsigned char xresolution;
		unsigned char aspect_vfreq;
	} __attribute__((__packed__)) standard_timings[8];
#define EDID1_INFO_STANDARD_TIMING_XRESOLUTION(_x, _i) \
	(((_x).standard_timings[_i]).xresolution)
#define EDID1_INFO_STANDARD_TIMING_ASPECT(_x, _i) \
	GET_BITS(((_x).standard_timings[_i].aspect_vfreq), 7, 6)
#define EDID1_INFO_STANDARD_TIMING_VFREQ(_x, _i) \
	GET_BITS(((_x).standard_timings[_i].aspect_vfreq), 5, 0)
	union {
		unsigned char timing[72];
		struct edid_monitor_descriptor descriptor[4];
	} monitor_details;
	unsigned char extension_flag;
	unsigned char checksum;
} __attribute__ ((__packed__));

enum edid_cea861_db_types {
	EDID_CEA861_DB_AUDIO = 0x01,
	EDID_CEA861_DB_VIDEO = 0x02,
	EDID_CEA861_DB_VENDOR = 0x03,
	EDID_CEA861_DB_SPEAKER = 0x04,
};

struct edid_cea861_info {
	unsigned char extension_tag;
#define EDID_CEA861_EXTENSION_TAG	0x02
	unsigned char revision;
	unsigned char dtd_offset;
	unsigned char dtd_count;
#define EDID_CEA861_SUPPORTS_UNDERSCAN(_x) \
	GET_BIT(((_x).dtd_count), 7)
#define EDID_CEA861_SUPPORTS_BASIC_AUDIO(_x) \
	GET_BIT(((_x).dtd_count), 6)
#define EDID_CEA861_SUPPORTS_YUV444(_x) \
	GET_BIT(((_x).dtd_count), 5)
#define EDID_CEA861_SUPPORTS_YUV422(_x) \
	GET_BIT(((_x).dtd_count), 4)
#define EDID_CEA861_DTD_COUNT(_x) \
	GET_BITS(((_x).dtd_count), 3, 0)
	unsigned char data[124];
#define EDID_CEA861_DB_TYPE(_x, offset) \
	GET_BITS((_x).data[offset], 7, 5)
#define EDID_CEA861_DB_LEN(_x, offset) \
	GET_BITS((_x).data[offset], 4, 0)
} __attribute__ ((__packed__));

/**
 * Print the EDID info.
 *
 * @param edid_info	The EDID info to be printed
 */
void edid_print_info(struct edid1_info *edid_info);

/**
 * Check the EDID info.
 *
 * @param info  The EDID info to be checked
 * @return 0 on valid, or -1 on invalid
 */
int edid_check_info(struct edid1_info *info);

/**
 * Check checksum of a 128 bytes EDID data block
 *
 * @param edid_block	EDID block data
 *
 * @return 0 on success, or a negative errno on error
 */
int edid_check_checksum(u8 *edid_block);

/**
 * Get the horizontal and vertical rate ranges of the monitor.
 *
 * @param edid	The EDID info
 * @param hmin	Returns the minimum horizontal rate
 * @param hmax	Returns the maxium horizontal rate
 * @param vmin	Returns the minimum vertical rate
 * @param vmax	Returns the maxium vertical rate
 * @return 0 on success, or -1 on error
 */
int edid_get_ranges(struct edid1_info *edid, unsigned int *hmin,
		    unsigned int *hmax, unsigned int *vmin,
		    unsigned int *vmax);

struct display_timing;

/**
 * edid_get_timing() - Get basic digital display parameters
 *
 * @param buf		Buffer containing EDID data
 * @param buf_size	Size of buffer in bytes
 * @param timing	Place to put preferring timing information
 * @param panel_bits_per_colourp	Place to put the number of bits per
 *			colour supported by the panel. This will be set to
 *			-1 if not available
 * @return 0 if timings are OK, -ve on error
 */
int edid_get_timing(u8 *buf, int buf_size, struct display_timing *timing,
		    int *panel_bits_per_colourp);

#endif /* __EDID_H_ */
