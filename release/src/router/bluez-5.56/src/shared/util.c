// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>

#include "src/shared/util.h"

void *btd_malloc(size_t size)
{
	if (__builtin_expect(!!size, 1)) {
		void *ptr;

		ptr = malloc(size);
		if (ptr)
			return ptr;

		fprintf(stderr, "failed to allocate %zu bytes\n", size);
		abort();
	}

	return NULL;
}

void util_debug_va(util_debug_func_t function, void *user_data,
				const char *format, va_list va)
{
	char str[78];

	if (!function || !format)
		return;

	vsnprintf(str, sizeof(str), format, va);

	function(str, user_data);
}

void util_debug(util_debug_func_t function, void *user_data,
						const char *format, ...)
{
	va_list ap;

	if (!function || !format)
		return;

	usleep(20*1000);
	va_start(ap, format);
	util_debug_va(function, user_data, format, ap);
	va_end(ap);
}

void util_hexdump(const char dir, const unsigned char *buf, size_t len,
				util_debug_func_t function, void *user_data)
{
	static const char hexdigits[] = "0123456789abcdef";
	char str[68];
	size_t i;

	if (!function || !len)
		return;

	str[0] = dir;

	for (i = 0; i < len; i++) {
		str[((i % 16) * 3) + 1] = ' ';
		str[((i % 16) * 3) + 2] = hexdigits[buf[i] >> 4];
		str[((i % 16) * 3) + 3] = hexdigits[buf[i] & 0xf];
		str[(i % 16) + 51] = isprint(buf[i]) ? buf[i] : '.';

		if ((i + 1) % 16 == 0) {
			str[49] = ' ';
			str[50] = ' ';
			str[67] = '\0';
			function(str, user_data);
			str[0] = ' ';
		}
	}

	if (i % 16 > 0) {
		size_t j;
		for (j = (i % 16); j < 16; j++) {
			str[(j * 3) + 1] = ' ';
			str[(j * 3) + 2] = ' ';
			str[(j * 3) + 3] = ' ';
			str[j + 51] = ' ';
		}
		str[49] = ' ';
		str[50] = ' ';
		str[67] = '\0';
		function(str, user_data);
	}
}

/* Helper for getting the dirent type in case readdir returns DT_UNKNOWN */
unsigned char util_get_dt(const char *parent, const char *name)
{
	char filename[PATH_MAX];
	struct stat st;

	snprintf(filename, PATH_MAX, "%s/%s", parent, name);
	if (lstat(filename, &st) == 0 && S_ISDIR(st.st_mode))
		return DT_DIR;

	return DT_UNKNOWN;
}

/* Helpers for bitfield operations */

/* Find unique id in range from 1 to max but no bigger then
 * sizeof(int) * 8. ffs() is used since it is POSIX standard
 */
uint8_t util_get_uid(unsigned int *bitmap, uint8_t max)
{
	uint8_t id;

	id = ffs(~*bitmap);

	if (!id || id > max)
		return 0;

	*bitmap |= 1u << (id - 1);

	return id;
}

/* Clear id bit in bitmap */
void util_clear_uid(unsigned int *bitmap, uint8_t id)
{
	if (!id)
		return;

	*bitmap &= ~(1u << (id - 1));
}

static const struct {
	uint16_t uuid;
	const char *str;
} uuid16_table[] = {
	{ 0x0001, "SDP"						},
	{ 0x0003, "RFCOMM"					},
	{ 0x0005, "TCS-BIN"					},
	{ 0x0007, "ATT"						},
	{ 0x0008, "OBEX"					},
	{ 0x000f, "BNEP"					},
	{ 0x0010, "UPNP"					},
	{ 0x0011, "HIDP"					},
	{ 0x0012, "Hardcopy Control Channel"			},
	{ 0x0014, "Hardcopy Data Channel"			},
	{ 0x0016, "Hardcopy Notification"			},
	{ 0x0017, "AVCTP"					},
	{ 0x0019, "AVDTP"					},
	{ 0x001b, "CMTP"					},
	{ 0x001e, "MCAP Control Channel"			},
	{ 0x001f, "MCAP Data Channel"				},
	{ 0x0100, "L2CAP"					},
	/* 0x0101 to 0x0fff undefined */
	{ 0x1000, "Service Discovery Server Service Class"	},
	{ 0x1001, "Browse Group Descriptor Service Class"	},
	{ 0x1002, "Public Browse Root"				},
	/* 0x1003 to 0x1100 undefined */
	{ 0x1101, "Serial Port"					},
	{ 0x1102, "LAN Access Using PPP"			},
	{ 0x1103, "Dialup Networking"				},
	{ 0x1104, "IrMC Sync"					},
	{ 0x1105, "OBEX Object Push"				},
	{ 0x1106, "OBEX File Transfer"				},
	{ 0x1107, "IrMC Sync Command"				},
	{ 0x1108, "Headset"					},
	{ 0x1109, "Cordless Telephony"				},
	{ 0x110a, "Audio Source"				},
	{ 0x110b, "Audio Sink"					},
	{ 0x110c, "A/V Remote Control Target"			},
	{ 0x110d, "Advanced Audio Distribution"			},
	{ 0x110e, "A/V Remote Control"				},
	{ 0x110f, "A/V Remote Control Controller"		},
	{ 0x1110, "Intercom"					},
	{ 0x1111, "Fax"						},
	{ 0x1112, "Headset AG"					},
	{ 0x1113, "WAP"						},
	{ 0x1114, "WAP Client"					},
	{ 0x1115, "PANU"					},
	{ 0x1116, "NAP"						},
	{ 0x1117, "GN"						},
	{ 0x1118, "Direct Printing"				},
	{ 0x1119, "Reference Printing"				},
	{ 0x111a, "Basic Imaging Profile"			},
	{ 0x111b, "Imaging Responder"				},
	{ 0x111c, "Imaging Automatic Archive"			},
	{ 0x111d, "Imaging Referenced Objects"			},
	{ 0x111e, "Handsfree"					},
	{ 0x111f, "Handsfree Audio Gateway"			},
	{ 0x1120, "Direct Printing Refrence Objects Service"	},
	{ 0x1121, "Reflected UI"				},
	{ 0x1122, "Basic Printing"				},
	{ 0x1123, "Printing Status"				},
	{ 0x1124, "Human Interface Device Service"		},
	{ 0x1125, "Hardcopy Cable Replacement"			},
	{ 0x1126, "HCR Print"					},
	{ 0x1127, "HCR Scan"					},
	{ 0x1128, "Common ISDN Access"				},
	/* 0x1129 and 0x112a undefined */
	{ 0x112d, "SIM Access"					},
	{ 0x112e, "Phonebook Access Client"			},
	{ 0x112f, "Phonebook Access Server"			},
	{ 0x1130, "Phonebook Access"				},
	{ 0x1131, "Headset HS"					},
	{ 0x1132, "Message Access Server"			},
	{ 0x1133, "Message Notification Server"			},
	{ 0x1134, "Message Access Profile"			},
	{ 0x1135, "GNSS"					},
	{ 0x1136, "GNSS Server"					},
	{ 0x1137, "3D Display"					},
	{ 0x1138, "3D Glasses"					},
	{ 0x1139, "3D Synchronization"				},
	{ 0x113a, "MPS Profile"					},
	{ 0x113b, "MPS Service"					},
	/* 0x113c to 0x11ff undefined */
	{ 0x1200, "PnP Information"				},
	{ 0x1201, "Generic Networking"				},
	{ 0x1202, "Generic File Transfer"			},
	{ 0x1203, "Generic Audio"				},
	{ 0x1204, "Generic Telephony"				},
	{ 0x1205, "UPNP Service"				},
	{ 0x1206, "UPNP IP Service"				},
	{ 0x1300, "UPNP IP PAN"					},
	{ 0x1301, "UPNP IP LAP"					},
	{ 0x1302, "UPNP IP L2CAP"				},
	{ 0x1303, "Video Source"				},
	{ 0x1304, "Video Sink"					},
	{ 0x1305, "Video Distribution"				},
	/* 0x1306 to 0x13ff undefined */
	{ 0x1400, "HDP"						},
	{ 0x1401, "HDP Source"					},
	{ 0x1402, "HDP Sink"					},
	/* 0x1403 to 0x17ff undefined */
	{ 0x1800, "Generic Access Profile"			},
	{ 0x1801, "Generic Attribute Profile"			},
	{ 0x1802, "Immediate Alert"				},
	{ 0x1803, "Link Loss"					},
	{ 0x1804, "Tx Power"					},
	{ 0x1805, "Current Time Service"			},
	{ 0x1806, "Reference Time Update Service"		},
	{ 0x1807, "Next DST Change Service"			},
	{ 0x1808, "Glucose"					},
	{ 0x1809, "Health Thermometer"				},
	{ 0x180a, "Device Information"				},
	/* 0x180b and 0x180c undefined */
	{ 0x180d, "Heart Rate"					},
	{ 0x180e, "Phone Alert Status Service"			},
	{ 0x180f, "Battery Service"				},
	{ 0x1810, "Blood Pressure"				},
	{ 0x1811, "Alert Notification Service"			},
	{ 0x1812, "Human Interface Device"			},
	{ 0x1813, "Scan Parameters"				},
	{ 0x1814, "Running Speed and Cadence"			},
	{ 0x1815, "Automation IO"				},
	{ 0x1816, "Cycling Speed and Cadence"			},
	/* 0x1817 undefined */
	{ 0x1818, "Cycling Power"				},
	{ 0x1819, "Location and Navigation"			},
	{ 0x181a, "Environmental Sensing"			},
	{ 0x181b, "Body Composition"				},
	{ 0x181c, "User Data"					},
	{ 0x181d, "Weight Scale"				},
	{ 0x181e, "Bond Management"				},
	{ 0x181f, "Continuous Glucose Monitoring"		},
	{ 0x1820, "Internet Protocol Support"			},
	{ 0x1821, "Indoor Positioning"				},
	{ 0x1822, "Pulse Oximeter"				},
	{ 0x1823, "HTTP Proxy"					},
	{ 0x1824, "Transport Discovery"				},
	{ 0x1825, "Object Transfer"				},
	{ 0x1826, "Fitness Machine"				},
	{ 0x1827, "Mesh Provisioning"				},
	{ 0x1828, "Mesh Proxy"					},
	/* 0x1829 to 0x27ff undefined */
	{ 0x2800, "Primary Service"				},
	{ 0x2801, "Secondary Service"				},
	{ 0x2802, "Include"					},
	{ 0x2803, "Characteristic"				},
	/* 0x2804 to 0x28ff undefined */
	{ 0x2900, "Characteristic Extended Properties"		},
	{ 0x2901, "Characteristic User Description"		},
	{ 0x2902, "Client Characteristic Configuration"		},
	{ 0x2903, "Server Characteristic Configuration"		},
	{ 0x2904, "Characteristic Format"			},
	{ 0x2905, "Characteristic Aggregate Formate"		},
	{ 0x2906, "Valid Range"					},
	{ 0x2907, "External Report Reference"			},
	{ 0x2908, "Report Reference"				},
	{ 0x2909, "Number of Digitals"				},
	{ 0x290a, "Value Trigger Setting"			},
	{ 0x290b, "Environmental Sensing Configuration"		},
	{ 0x290c, "Environmental Sensing Measurement"		},
	{ 0x290d, "Environmental Sensing Trigger Setting"	},
	{ 0x290e, "Time Trigger Setting"			},
	/* 0x290f to 0x29ff undefined */
	{ 0x2a00, "Device Name"					},
	{ 0x2a01, "Appearance"					},
	{ 0x2a02, "Peripheral Privacy Flag"			},
	{ 0x2a03, "Reconnection Address"			},
	{ 0x2a04, "Peripheral Preferred Connection Parameters"	},
	{ 0x2a05, "Service Changed"				},
	{ 0x2a06, "Alert Level"					},
	{ 0x2a07, "Tx Power Level"				},
	{ 0x2a08, "Date Time"					},
	{ 0x2a09, "Day of Week"					},
	{ 0x2a0a, "Day Date Time"				},
	/* 0x2a0b undefined */
	{ 0x2a0c, "Exact Time 256"				},
	{ 0x2a0d, "DST Offset"					},
	{ 0x2a0e, "Time Zone"					},
	{ 0x2a0f, "Local Time Information"			},
	/* 0x2a10 undefined */
	{ 0x2a11, "Time with DST"				},
	{ 0x2a12, "Time Accuracy"				},
	{ 0x2a13, "Time Source"					},
	{ 0x2a14, "Reference Time Information"			},
	/* 0x2a15 undefined */
	{ 0x2a16, "Time Update Control Point"			},
	{ 0x2a17, "Time Update State"				},
	{ 0x2a18, "Glucose Measurement"				},
	{ 0x2a19, "Battery Level"				},
	/* 0x2a1a and 0x2a1b undefined */
	{ 0x2a1c, "Temperature Measurement"			},
	{ 0x2a1d, "Temperature Type"				},
	{ 0x2a1e, "Intermediate Temperature"			},
	/* 0x2a1f and 0x2a20 undefined */
	{ 0x2a21, "Measurement Interval"			},
	{ 0x2a22, "Boot Keyboard Input Report"			},
	{ 0x2a23, "System ID"					},
	{ 0x2a24, "Model Number String"				},
	{ 0x2a25, "Serial Number String"			},
	{ 0x2a26, "Firmware Revision String"			},
	{ 0x2a27, "Hardware Revision String"			},
	{ 0x2a28, "Software Revision String"			},
	{ 0x2a29, "Manufacturer Name String"			},
	{ 0x2a2a, "IEEE 11073-20601 Regulatory Cert. Data List"	},
	{ 0x2a2b, "Current Time"				},
	{ 0x2a2c, "Magnetic Declination"			},
	/* 0x2a2d to 0x2a30 undefined */
	{ 0x2a31, "Scan Refresh"				},
	{ 0x2a32, "Boot Keyboard Output Report"			},
	{ 0x2a33, "Boot Mouse Input Report"			},
	{ 0x2a34, "Glucose Measurement Context"			},
	{ 0x2a35, "Blood Pressure Measurement"			},
	{ 0x2a36, "Intermediate Cuff Pressure"			},
	{ 0x2a37, "Heart Rate Measurement"			},
	{ 0x2a38, "Body Sensor Location"			},
	{ 0x2a39, "Heart Rate Control Point"			},
	/* 0x2a3a to 0x2a3e undefined */
	{ 0x2a3f, "Alert Status"				},
	{ 0x2a40, "Ringer Control Point"			},
	{ 0x2a41, "Ringer Setting"				},
	{ 0x2a42, "Alert Category ID Bit Mask"			},
	{ 0x2a43, "Alert Category ID"				},
	{ 0x2a44, "Alert Notification Control Point"		},
	{ 0x2a45, "Unread Alert Status"				},
	{ 0x2a46, "New Alert"					},
	{ 0x2a47, "Supported New Alert Category"		},
	{ 0x2a48, "Supported Unread Alert Category"		},
	{ 0x2a49, "Blood Pressure Feature"			},
	{ 0x2a4a, "HID Information"				},
	{ 0x2a4b, "Report Map"					},
	{ 0x2a4c, "HID Control Point"				},
	{ 0x2a4d, "Report"					},
	{ 0x2a4e, "Protocol Mode"				},
	{ 0x2a4f, "Scan Interval Window"			},
	{ 0x2a50, "PnP ID"					},
	{ 0x2a51, "Glucose Feature"				},
	{ 0x2a52, "Record Access Control Point"			},
	{ 0x2a53, "RSC Measurement"				},
	{ 0x2a54, "RSC Feature"					},
	{ 0x2a55, "SC Control Point"				},
	{ 0x2a56, "Digital"					},
	/* 0x2a57 undefined */
	{ 0x2a58, "Analog"					},
	/* 0x2a59 undefined */
	{ 0x2a5a, "Aggregate"					},
	{ 0x2a5b, "CSC Measurement"				},
	{ 0x2a5c, "CSC Feature"					},
	{ 0x2a5d, "Sensor Location"				},
	/* 0x2a5e to 0x2a62 undefined */
	{ 0x2a63, "Cycling Power Measurement"			},
	{ 0x2a64, "Cycling Power Vector"			},
	{ 0x2a65, "Cycling Power Feature"			},
	{ 0x2a66, "Cycling Power Control Point"			},
	{ 0x2a67, "Location and Speed"				},
	{ 0x2a68, "Navigation"					},
	{ 0x2a69, "Position Quality"				},
	{ 0x2a6a, "LN Feature"					},
	{ 0x2a6b, "LN Control Point"				},
	{ 0x2a6c, "Elevation"					},
	{ 0x2a6d, "Pressure"					},
	{ 0x2a6e, "Temperature"					},
	{ 0x2a6f, "Humidity"					},
	{ 0x2a70, "True Wind Speed"				},
	{ 0x2a71, "True Wind Direction"				},
	{ 0x2a72, "Apparent Wind Speed"				},
	{ 0x2a73, "Apparent Wind Direction"			},
	{ 0x2a74, "Gust Factor"					},
	{ 0x2a75, "Pollen Concentration"			},
	{ 0x2a76, "UV Index"					},
	{ 0x2a77, "Irradiance"					},
	{ 0x2a78, "Rainfall"					},
	{ 0x2a79, "Wind Chill"					},
	{ 0x2a7a, "Heat Index"					},
	{ 0x2a7b, "Dew Point"					},
	{ 0x2a7c, "Trend"					},
	{ 0x2a7d, "Descriptor Value Changed"			},
	{ 0x2a7e, "Aerobic Heart Rate Lower Limit"		},
	{ 0x2a7f, "Aerobic Threshold"				},
	{ 0x2a80, "Age"						},
	{ 0x2a81, "Anaerobic Heart Rate Lower Limit"		},
	{ 0x2a82, "Anaerobic Heart Rate Upper Limit"		},
	{ 0x2a83, "Anaerobic Threshold"				},
	{ 0x2a84, "Aerobic Heart Rate Upper Limit"		},
	{ 0x2a85, "Date of Birth"				},
	{ 0x2a86, "Date of Threshold Assessment"		},
	{ 0x2a87, "Email Address"				},
	{ 0x2a88, "Fat Burn Heart Rate Lower Limit"		},
	{ 0x2a89, "Fat Burn Heart Rate Upper Limit"		},
	{ 0x2a8a, "First Name"					},
	{ 0x2a8b, "Five Zone Heart Rate Limits"			},
	{ 0x2a8c, "Gender"					},
	{ 0x2a8d, "Heart Rate Max"				},
	{ 0x2a8e, "Height"					},
	{ 0x2a8f, "Hip Circumference"				},
	{ 0x2a90, "Last Name"					},
	{ 0x2a91, "Maximum Recommended Heart Rate"		},
	{ 0x2a92, "Resting Heart Rate"				},
	{ 0x2a93, "Sport Type for Aerobic/Anaerobic Thresholds"	},
	{ 0x2a94, "Three Zone Heart Rate Limits"		},
	{ 0x2a95, "Two Zone Heart Rate Limit"			},
	{ 0x2a96, "VO2 Max"					},
	{ 0x2a97, "Waist Circumference"				},
	{ 0x2a98, "Weight"					},
	{ 0x2a99, "Database Change Increment"			},
	{ 0x2a9a, "User Index"					},
	{ 0x2a9b, "Body Composition Feature"			},
	{ 0x2a9c, "Body Composition Measurement"		},
	{ 0x2a9d, "Weight Measurement"				},
	{ 0x2a9e, "Weight Scale Feature"			},
	{ 0x2a9f, "User Control Point"				},
	{ 0x2aa0, "Magnetic Flux Density - 2D"			},
	{ 0x2aa1, "Magnetic Flux Density - 3D"			},
	{ 0x2aa2, "Language"					},
	{ 0x2aa3, "Barometric Pressure Trend"			},
	{ 0x2aa4, "Bond Management Control Point"		},
	{ 0x2aa5, "Bond Management Feature"			},
	{ 0x2aa6, "Central Address Resolution"			},
	{ 0x2aa7, "CGM Measurement"				},
	{ 0x2aa8, "CGM Feature"					},
	{ 0x2aa9, "CGM Status"					},
	{ 0x2aaa, "CGM Session Start Time"			},
	{ 0x2aab, "CGM Session Run Time"			},
	{ 0x2aac, "CGM Specific Ops Control Point"		},
	{ 0x2aad, "Indoor Positioning Configuration"		},
	{ 0x2aae, "Latitude"					},
	{ 0x2aaf, "Longitude"					},
	{ 0x2ab0, "Local North Coordinate"			},
	{ 0x2ab1, "Local East Coordinate"			},
	{ 0x2ab2, "Floor Number"				},
	{ 0x2ab3, "Altitude"					},
	{ 0x2ab4, "Uncertainty"					},
	{ 0x2ab5, "Location Name"				},
	{ 0x2ab6, "URI"						},
	{ 0x2ab7, "HTTP Headers"				},
	{ 0x2ab8, "HTTP Status Code"				},
	{ 0x2ab9, "HTTP Entity Body"				},
	{ 0x2aba, "HTTP Control Point"				},
	{ 0x2abb, "HTTPS Security"				},
	{ 0x2abc, "TDS Control Point"				},
	{ 0x2abd, "OTS Feature"					},
	{ 0x2abe, "Object Name"					},
	{ 0x2abf, "Object Type"					},
	{ 0x2ac0, "Object Size"					},
	{ 0x2ac1, "Object First-Created"			},
	{ 0x2ac2, "Object Last-Modified"			},
	{ 0x2ac3, "Object ID"					},
	{ 0x2ac4, "Object Properties"				},
	{ 0x2ac5, "Object Action Control Point"			},
	{ 0x2ac6, "Object List Control Point"			},
	{ 0x2ac7, "Object List Filter"				},
	{ 0x2ac8, "Object Changed"				},
	{ 0x2ac9, "Resolvable Private Address Only"		},
	/* 0x2aca and 0x2acb undefined */
	{ 0x2acc, "Fitness Machine Feature"			},
	{ 0x2acd, "Treadmill Data"				},
	{ 0x2ace, "Cross Trainer Data"				},
	{ 0x2acf, "Step Climber Data"				},
	{ 0x2ad0, "Stair Climber Data"				},
	{ 0x2ad1, "Rower Data"					},
	{ 0x2ad2, "Indoor Bike Data"				},
	{ 0x2ad3, "Training Status"				},
	{ 0x2ad4, "Supported Speed Range"			},
	{ 0x2ad5, "Supported Inclination Range"			},
	{ 0x2ad6, "Supported Resistance Level Range"		},
	{ 0x2ad7, "Supported Heart Rate Range"			},
	{ 0x2ad8, "Supported Power Range"			},
	{ 0x2ad9, "Fitness Machine Control Point"		},
	{ 0x2ada, "Fitness Machine Status"			},
	{ 0x2adb, "Mesh Provisioning Data In"			},
	{ 0x2adc, "Mesh Provisioning Data Out"			},
	{ 0x2add, "Mesh Proxy Data In"				},
	{ 0x2ade, "Mesh Proxy Data Out"				},
	{ 0x2b29, "Client Supported Features"			},
	{ 0x2b2A, "Database Hash"				},
	/* vendor defined */
	{ 0xfeff, "GN Netcom"					},
	{ 0xfefe, "GN ReSound A/S"				},
	{ 0xfefd, "Gimbal, Inc."				},
	{ 0xfefc, "Gimbal, Inc."				},
	{ 0xfefb, "Telit Wireless Solutions (Formerly Stollmann E+V GmbH)" },
	{ 0xfefa, "PayPal, Inc."				},
	{ 0xfef9, "PayPal, Inc."				},
	{ 0xfef8, "Aplix Corporation"				},
	{ 0xfef7, "Aplix Corporation"				},
	{ 0xfef6, "Wicentric, Inc."				},
	{ 0xfef5, "Dialog Semiconductor GmbH"			},
	{ 0xfef4, "Google"					},
	{ 0xfef3, "Google"					},
	{ 0xfef2, "CSR"						},
	{ 0xfef1, "CSR"						},
	{ 0xfef0, "Intel"					},
	{ 0xfeef, "Polar Electro Oy "				},
	{ 0xfeee, "Polar Electro Oy "				},
	{ 0xfeed, "Tile, Inc."					},
	{ 0xfeec, "Tile, Inc."					},
	{ 0xfeeb, "Swirl Networks, Inc."			},
	{ 0xfeea, "Swirl Networks, Inc."			},
	{ 0xfee9, "Quintic Corp."				},
	{ 0xfee8, "Quintic Corp."				},
	{ 0xfee7, "Tencent Holdings Limited."			},
	{ 0xfee6, "Silvair, Inc."				},
	{ 0xfee5, "Nordic Semiconductor ASA"			},
	{ 0xfee4, "Nordic Semiconductor ASA"			},
	{ 0xfee3, "Anki, Inc."					},
	{ 0xfee2, "Anki, Inc."					},
	{ 0xfee1, "Anhui Huami Information Technology Co., Ltd. " },
	{ 0xfee0, "Anhui Huami Information Technology Co., Ltd. " },
	{ 0xfedf, "Design SHIFT"				},
	{ 0xfede, "Coin, Inc."					},
	{ 0xfedd, "Jawbone"					},
	{ 0xfedc, "Jawbone"					},
	{ 0xfedb, "Perka, Inc."					},
	{ 0xfeda, "ISSC Technologies Corp. "			},
	{ 0xfed9, "Pebble Technology Corporation"		},
	{ 0xfed8, "Google"					},
	{ 0xfed7, "Broadcom"					},
	{ 0xfed6, "Broadcom"					},
	{ 0xfed5, "Plantronics Inc."				},
	{ 0xfed4, "Apple, Inc."					},
	{ 0xfed3, "Apple, Inc."					},
	{ 0xfed2, "Apple, Inc."					},
	{ 0xfed1, "Apple, Inc."					},
	{ 0xfed0, "Apple, Inc."					},
	{ 0xfecf, "Apple, Inc."					},
	{ 0xfece, "Apple, Inc."					},
	{ 0xfecd, "Apple, Inc."					},
	{ 0xfecc, "Apple, Inc."					},
	{ 0xfecb, "Apple, Inc."					},
	{ 0xfeca, "Apple, Inc."					},
	{ 0xfec9, "Apple, Inc."					},
	{ 0xfec8, "Apple, Inc."					},
	{ 0xfec7, "Apple, Inc."					},
	{ 0xfec6, "Kocomojo, LLC"				},
	{ 0xfec5, "Realtek Semiconductor Corp."			},
	{ 0xfec4, "PLUS Location Systems"			},
	{ 0xfec3, "360fly, Inc."				},
	{ 0xfec2, "Blue Spark Technologies, Inc."		},
	{ 0xfec1, "KDDI Corporation"				},
	{ 0xfec0, "KDDI Corporation"				},
	{ 0xfebf, "Nod, Inc."					},
	{ 0xfebe, "Bose Corporation"				},
	{ 0xfebd, "Clover Network, Inc"				},
	{ 0xfebc, "Dexcom Inc"					},
	{ 0xfebb, "adafruit industries"				},
	{ 0xfeba, "Tencent Holdings Limited"			},
	{ 0xfeb9, "LG Electronics"				},
	{ 0xfeb8, "Facebook, Inc."				},
	{ 0xfeb7, "Facebook, Inc."				},
	{ 0xfeb6, "Vencer Co., Ltd"				},
	{ 0xfeb5, "WiSilica Inc."				},
	{ 0xfeb4, "WiSilica Inc."				},
	{ 0xfeb3, "Taobao"					},
	{ 0xfeb2, "Microsoft Corporation"			},
	{ 0xfeb1, "Electronics Tomorrow Limited"		},
	{ 0xfeb0, "Nest Labs Inc"				},
	{ 0xfeaf, "Nest Labs Inc"				},
	{ 0xfeae, "Nokia"					},
	{ 0xfead, "Nokia"					},
	{ 0xfeac, "Nokia"					},
	{ 0xfeab, "Nokia"					},
	{ 0xfeaa, "Google"					},
	{ 0xfea9, "Savant Systems LLC"				},
	{ 0xfea8, "Savant Systems LLC"				},
	{ 0xfea7, "UTC Fire and Security"			},
	{ 0xfea6, "GoPro, Inc."					},
	{ 0xfea5, "GoPro, Inc."					},
	{ 0xfea4, "Paxton Access Ltd"				},
	{ 0xfea3, "ITT Industries"				},
	{ 0xfea2, "Intrepid Control Systems, Inc."		},
	{ 0xfea1, "Intrepid Control Systems, Inc."		},
	{ 0xfea0, "Google"					},
	{ 0xfe9f, "Google"					},
	{ 0xfe9e, "Dialog Semiconductor B.V."			},
	{ 0xfe9d, "Mobiquity Networks Inc"			},
	{ 0xfe9c, "GSI Laboratories, Inc."			},
	{ 0xfe9b, "Samsara Networks, Inc"			},
	{ 0xfe9a, "Estimote"					},
	{ 0xfe99, "Currant Inc"					},
	{ 0xfe98, "Currant Inc"					},
	{ 0xfe97, "Tesla Motors Inc."				},
	{ 0xfe96, "Tesla Motors Inc."				},
	{ 0xfe95, "Xiaomi Inc."					},
	{ 0xfe94, "OttoQ In"					},
	{ 0xfe93, "OttoQ In"					},
	{ 0xfe92, "Jarden Safety & Security"			},
	{ 0xfe91, "Shanghai Imilab Technology Co.,Ltd"		},
	{ 0xfe90, "JUMA"					},
	{ 0xfe8f, "CSR"						},
	{ 0xfe8e, "ARM Ltd"					},
	{ 0xfe8d, "Interaxon Inc."				},
	{ 0xfe8c, "TRON Forum"					},
	{ 0xfe8b, "Apple, Inc."					},
	{ 0xfe8a, "Apple, Inc."					},
	{ 0xfe89, "B&O Play A/S"				},
	{ 0xfe88, "SALTO SYSTEMS S.L."				},
	{ 0xfe87, "Qingdao Yeelink Information Technology Co., Ltd. ( 青岛亿联客信息技术有限公司 )" },
	{ 0xfe86, "HUAWEI Technologies Co., Ltd. ( 华为技术有限公司 )"	},
	{ 0xfe85, "RF Digital Corp"				},
	{ 0xfe84, "RF Digital Corp"				},
	{ 0xfe83, "Blue Bite"					},
	{ 0xfe82, "Medtronic Inc."				},
	{ 0xfe81, "Medtronic Inc."				},
	{ 0xfe80, "Doppler Lab"					},
	{ 0xfe7f, "Doppler Lab"					},
	{ 0xfe7e, "Awear Solutions Ltd"				},
	{ 0xfe7d, "Aterica Health Inc."				},
	{ 0xfe7c, "Telit Wireless Solutions (Formerly Stollmann E+V GmbH)" },
	{ 0xfe7b, "Orion Labs, Inc."				},
	{ 0xfe7a, "Bragi GmbH"					},
	{ 0xfe79, "Zebra Technologies"				},
	{ 0xfe78, "Hewlett-Packard Company"			},
	{ 0xfe77, "Hewlett-Packard Company"			},
	{ 0xfe76, "TangoMe"					},
	{ 0xfe75, "TangoMe"					},
	{ 0xfe74, "unwire"					},
	{ 0xfe73, "Abbott (formerly St. Jude Medical, Inc.)"	},
	{ 0xfe72, "Abbott (formerly St. Jude Medical, Inc.)"	},
	{ 0xfe71, "Plume Design Inc"				},
	{ 0xfe70, "Beijing Jingdong Century Trading Co., Ltd."	},
	{ 0xfe6f, "LINE Corporation"				},
	{ 0xfe6e, "The University of Tokyo "			},
	{ 0xfe6d, "The University of Tokyo "			},
	{ 0xfe6c, "TASER International, Inc."			},
	{ 0xfe6b, "TASER International, Inc."			},
	{ 0xfe6a, "Kontakt Micro-Location Sp. z o.o."		},
	{ 0xfe69, "Capsule Technologies Inc."			},
	{ 0xfe68, "Capsule Technologies Inc."			},
	{ 0xfe67, "Lab Sensor Solutions"			},
	{ 0xfe66, "Intel Corporation "				},
	{ 0xfe65, "CHIPOLO d.o.o. "				},
	{ 0xfe64, "Siemens AG"					},
	{ 0xfe63, "Connected Yard, Inc. "			},
	{ 0xfe62, "Indagem Tech LLC "				},
	{ 0xfe61, "Logitech International SA "			},
	{ 0xfe60, "Lierda Science & Technology Group Co., Ltd."	},
	{ 0xfe5f, "Eyefi, Inc."					},
	{ 0xfe5e, "Plastc Corporation "				},
	{ 0xfe5d, "Grundfos A/S "				},
	{ 0xfe5c, "million hunters GmbH "			},
	{ 0xfe5b, "GT-tronics HK Ltd"				},
	{ 0xfe5a, "Cronologics Corporation"			},
	{ 0xfe59, "Nordic Semiconductor ASA "			},
	{ 0xfe58, "Nordic Semiconductor ASA "			},
	{ 0xfe57, "Dotted Labs "				},
	{ 0xfe56, "Google Inc. "				},
	{ 0xfe55, "Google Inc. "				},
	{ 0xfe54, "Motiv, Inc. "				},
	{ 0xfe53, "3M"						},
	{ 0xfe52, "SetPoint Medical "				},
	{ 0xfe51, "SRAM "					},
	{ 0xfe50, "Google Inc."					},
	{ 0xfe4f, "Molekule, Inc."				},
	{ 0xfe4e, "NTT docomo "					},
	{ 0xfe4d, "Casambi Technologies Oy"			},
	{ 0xfe4c, "Volkswagen AG "				},
	{ 0xfe4b, "Signify Netherlands B.V. (formerly Philips Lighting B.V.)" },
	{ 0xfe4a, "OMRON HEALTHCARE Co., Ltd."			},
	{ 0xfe49, "SenionLab AB"				},
	{ 0xfe48, "General Motors "				},
	{ 0xfe47, "General Motors "				},
	{ 0xfe46, "B&O Play A/S "				},
	{ 0xfe45, "Snapchat Inc"				},
	{ 0xfe44, "SK Telecom "					},
	{ 0xfe43, "Andreas Stihl AG & Co. KG"			},
	{ 0xfe42, "Nets A/S "					},
	{ 0xfe41, "Inugo Systems Limited"			},
	{ 0xfe40, "Inugo Systems Limited"			},
	{ 0xfe3f, "Friday Labs Limited"				},
	{ 0xfe3e, "BD Medical"					},
	{ 0xfe3d, "BD Medical"					},
	{ 0xfe3c, "alibaba"					},
	{ 0xfe3b, "Dobly Laboratories"				},
	{ 0xfe3a, "TTS Tooltechnic Systems AG & Co. KG"		},
	{ 0xfe39, "TTS Tooltechnic Systems AG & Co. KG"		},
	{ 0xfe38, "Spaceek LTD"					},
	{ 0xfe37, "Spaceek LTD"					},
	{ 0xfe36, "HUAWEI Technologies Co., Ltd"		},
	{ 0xfe35, "HUAWEI Technologies Co., Ltd"		},
	{ 0xfe34, "SmallLoop LLC"				},
	{ 0xfe33, "CHIPOLO d.o.o."				},
	{ 0xfe32, "Pro-Mark, Inc."				},
	{ 0xfe31, "Volkswagen AG"				},
	{ 0xfe30, "Volkswagen AG"				},
	{ 0xfe2f, "CRESCO Wireless, Inc"			},
	{ 0xfe2e, "ERi,Inc."					},
	{ 0xfe2d, "SMART INNOVATION Co.,Ltd"			},
	{ 0xfe2c, "Google"					},
	{ 0xfe2b, "ITT Industries"				},
	{ 0xfe2a, "DaisyWorks, Inc."				},
	{ 0xfe29, "Gibson Innovations"				},
	{ 0xfe28, "Ayla Networks"				},
	{ 0xfe27, "Google"					},
	{ 0xfe26, "Google"					},
	{ 0xfe25, "Apple, Inc. "				},
	{ 0xfe24, "August Home Inc"				},
	{ 0xfe23, "Zoll Medical Corporation"			},
	{ 0xfe22, "Zoll Medical Corporation"			},
	{ 0xfe21, "Bose Corporation"				},
	{ 0xfe20, "Emerson"					},
	{ 0xfe1f, "Garmin International, Inc."			},
	{ 0xfe1e, "Smart Innovations Co., Ltd"			},
	{ 0xfe1d, "Illuminati Instrument Corporation"		},
	{ 0xfe1c, "NetMedia, Inc."				},
	{ 0xfe1b, "Tyto Life LLC"				},
	{ 0xfe1a, "Tyto Life LLC"				},
	{ 0xfe19, "Google, Inc"					},
	{ 0xfe18, "Runtime, Inc."				},
	{ 0xfe17, "Telit Wireless Solutions GmbH"		},
	{ 0xfe16, "Footmarks, Inc."				},
	{ 0xfe15, "Amazon.com Services, Inc.."			},
	{ 0xfe14, "Flextronics International USA Inc."		},
	{ 0xfe13, "Apple Inc."					},
	{ 0xfe12, "M-Way Solutions GmbH"			},
	{ 0xfe11, "GMC-I Messtechnik GmbH"			},
	{ 0xfe10, "Lapis Semiconductor Co., Ltd."		},
	{ 0xfe0f, "Signify Netherlands B.V. (formerly Philips Lighting B.V.)" },
	{ 0xfe0e, "Setec Pty Ltd"				},
	{ 0xfe0d, "Procter & Gamble"				},
	{ 0xfe0c, "Procter & Gamble"				},
	{ 0xfe0b, "ruwido austria gmbh"				},
	{ 0xfe0a, "ruwido austria gmbh"				},
	{ 0xfe09, "Pillsy, Inc."				},
	{ 0xfe08, "Microsoft"					},
	{ 0xfe07, "Sonos, Inc."					},
	{ 0xfe06, "Qualcomm Technologies, Inc."			},
	{ 0xfe05, "CORE Transport Technologies NZ Limited "	},
	{ 0xfe04, "OpenPath Security Inc"			},
	{ 0xfe03, "Amazon.com Services, Inc."			},
	{ 0xfe02, "Robert Bosch GmbH"				},
	{ 0xfe01, "Duracell U.S. Operations Inc."		},
	{ 0xfe00, "Amazon.com Services, Inc."			},
	{ 0xfdff, "OSRAM GmbH"					},
	{ 0xfdfe, "ADHERIUM(NZ) LIMITED"			},
	{ 0xfdfd, "RecursiveSoft Inc."				},
	{ 0xfdfc, "Optrel AG"					},
	{ 0xfdfb, "Tandem Diabetes Care"			},
	{ 0xfdfa, "Tandem Diabetes Care"			},
	{ 0xfdf9, "INIA"					},
	{ 0xfdf8, "Onvocal"					},
	{ 0xfdf7, "HP Inc."					},
	{ 0xfdf6, "AIAIAI ApS"					},
	{ 0xfdf5, "Milwaukee Electric Tools"			},
	{ 0xfdf4, "O. E. M. Controls, Inc."			},
	{ 0xfdf3, "Amersports"					},
	{ 0xfdf2, "AMICCOM Electronics Corporation"		},
	{ 0xfdf1, "LAMPLIGHT Co.,Ltd"				},
	{ 0xfdf0, "Google Inc."					},
	{ 0xfdef, "ART AND PROGRAM, INC."			},
	{ 0xfdee, "Huawei Technologies Co., Ltd."		},
	{ 0xfded, "Pole Star"					},
	{ 0xfdec, "Mannkind Corporation"			},
	{ 0xfdeb, "Syntronix Corporation"			},
	{ 0xfdea, "SeeScan, Inc"				},
	{ 0xfde9, "Spacesaver Corporation"			},
	{ 0xfde8, "Robert Bosch GmbH"				},
	{ 0xfde7, "SECOM Co., LTD"				},
	{ 0xfde6, "Intelletto Technologies Inc"			},
	{ 0xfde5, "SMK Corporation "				},
	{ 0xfde4, "JUUL Labs, Inc."				},
	{ 0xfde3, "Abbott Diabetes Care"			},
	{ 0xfde2, "Google Inc."					},
	{ 0xfde1, "Fortin Electronic Systems "			},
	{ 0xfde0, "John Deere"					},
	{ 0xfddf, "Harman International"			},
	{ 0xfdde, "Noodle Technology Inc. "			},
	{ 0xfddd, "Arch Systems Inc"				},
	{ 0xfddc, "4iiii Innovations Inc."			},
	{ 0xfddb, "Samsung Electronics Co., Ltd. "		},
	{ 0xfdda, "MHCS"					},
	{ 0xfdd9, "Jiangsu Teranovo Tech Co., Ltd."		},
	{ 0xfdd8, "Jiangsu Teranovo Tech Co., Ltd."		},
	{ 0xfdd7, "Emerson"					},
	{ 0xfdd6, "Ministry of Supply "				},
	{ 0xfdd5, "Brompton Bicycle Ltd"			},
	{ 0xfdd4, "LX Solutions Pty Limited"			},
	{ 0xfdd3, "FUBA Automotive Electronics GmbH"		},
	{ 0xfdd2, "Bose Corporation"				},
	{ 0xfdd1, "Huawei Technologies Co., Ltd "		},
	{ 0xfdd0, "Huawei Technologies Co., Ltd "		},
	{ 0xfdcf, "Nalu Medical, Inc"				},
	{ 0xfdce, "SENNHEISER electronic GmbH & Co. KG"		},
	{ 0xfdcd, "Qingping Technology (Beijing) Co., Ltd."	},
	{ 0xfdcc, "Shoof Technologies"				},
	{ 0xfdcb, "Meggitt SA"					},
	{ 0xfdca, "Fortin Electronic Systems "			},
	{ 0xfdc9, "Busch-Jaeger Elektro GmbH"			},
	{ 0xfdc8, "Hach – Danaher"				},
	{ 0xfdc7, "Eli Lilly and Company"			},
	{ 0xfdc6, "Eli Lilly and Company"			},
	{ 0xfdc5, "Automatic Labs"				},
	{ 0xfdc4, "Simavita (Aust) Pty Ltd"			},
	{ 0xfdc3, "Baidu Online Network Technology (Beijing) Co., Ltd" },
	{ 0xfdc2, "Baidu Online Network Technology (Beijing) Co., Ltd" },
	{ 0xfdc1, "Hunter Douglas"				},
	{ 0xfdc0, "Hunter Douglas"				},
	{ 0xfdbf, "California Things Inc. "			},
	{ 0xfdbe, "California Things Inc. "			},
	{ 0xfdbd, "Clover Network, Inc."			},
	{ 0xfdbc, "Emerson"					},
	{ 0xfdbb, "Profoto"					},
	{ 0xfdba, "Comcast Cable Corporation"			},
	{ 0xfdb9, "Comcast Cable Corporation"			},
	{ 0xfdb8, "LivaNova USA Inc."				},
	{ 0xfdb7, "LivaNova USA Inc."				},
	{ 0xfdb6, "GWA Hygiene GmbH"				},
	{ 0xfdb5, "ECSG"					},
	{ 0xfdb4, "HP Inc"					},
	{ 0xfdb3, "Audiodo AB"					},
	{ 0xfdb2, "Portable Multimedia Ltd "			},
	{ 0xfdb1, "Proxy Technologies, Inc."			},
	{ 0xfdb0, "Proxy Technologies, Inc."			},
	{ 0xfdaf, "Wiliot LTD"					},
	{ 0xfdae, "Houwa System Design, k.k."			},
	{ 0xfdad, "Houwa System Design, k.k."			},
	{ 0xfdac, "Tentacle Sync GmbH"				},
	{ 0xfdab, "Xiaomi Inc."					},
	{ 0xfdaa, "Xiaomi Inc."					},
	{ 0xfda9, "Rhombus Systems, Inc."			},
	{ 0xfda8, "PSA Peugeot Citroën"				},
	{ 0xfda7, "WWZN Information Technology Company Limited"	},
	{ 0xfda6, "WWZN Information Technology Company Limited"	},
	{ 0xfda5, "Neurostim OAB, Inc."				},
	{ 0xfda4, "Inseego Corp."				},
	{ 0xfda3, "Inseego Corp."				},
	{ 0xfda2, "Groove X, Inc"				},
	{ 0xfda1, "Groove X, Inc"				},
	{ 0xfda0, "Secugen Corporation"				},
	{ 0xfd9f, "VitalTech Affiliates LLC"			},
	{ 0xfd9e, "The Coca-Cola Company"			},
	{ 0xfd9d, "Gastec Corporation"				},
	{ 0xfd9c, "Huawei Technologies Co., Ltd."		},
	{ 0xfd9b, "Huawei Technologies Co., Ltd."		},
	{ 0xfd9a, "Huawei Technologies Co., Ltd."		},
	{ 0xfd99, "ABB Oy"					},
	{ 0xfd98, "Disney Worldwide Services, Inc."		},
	{ 0xfd97, "June Life, Inc."				},
	{ 0xfd96, "Google LLC"					},
	{ 0xfd95, "Rigado"					},
	{ 0xfd94, "Hewlett Packard Enterprise"			},
	{ 0xfd93, "Bayerische Motoren Werke AG"			},
	{ 0xfd92, "Qualcomm Technologies International, Ltd. (QTIL)" },
	{ 0xfd91, "Groove X, Inc."				},
	{ 0xfd90, "Guangzhou SuperSound Information Technology Co.,Ltd" },
	{ 0xfd8f, "Matrix ComSec Pvt. Ltd."			},
	{ 0xfd8e, "Motorola Solutions"				},
	{ 0xfd8d, "quip NYC Inc."				},
	{ 0xfd8c, "Google LLC"					},
	{ 0xfd8b, "Jigowatts Inc."				},
	{ 0xfd8a, "Signify Netherlands B.V."			},
	{ 0xfd89, "Urbanminded LTD"				},
	{ 0xfd88, "Urbanminded LTD"				},
	{ 0xfd87, "Google LLC"					},
	{ 0xfd86, "Abbott"					},
	{ 0xfd85, "Husqvarna AB"				},
	{ 0xfd84, "Tile, Inc."					},
	{ 0xfd83, "iNFORM Technology GmbH"			},
	{ 0xfd82, "Sony Corporation"				},
	{ 0xfd81, "CANDY HOUSE, Inc."				},
	{ 0xfd80, "Phindex Technologies, Inc"			},
	{ 0xfd7f, "Husqvarna AB"				},
	{ 0xfd7e, "Samsung Electronics Co., Ltd."		},
	{ 0xfd7d, "Center for Advanced Research Wernher Von Braun" },
	{ 0xfd7c, "Toshiba Information Systems(Japan) Corporation" },
	{ 0xfd7b, "WYZE LABS, INC."				},
	{ 0xfd7a, "Withings"					},
	{ 0xfd79, "Withings"					},
	{ 0xfd78, "Withings"					},
	{ 0xfd77, "Withings"					},
	{ 0xfd76, "Insulet Corporation"				},
	{ 0xfd75, "Insulet Corporation"				},
	{ 0xfd74, "BRControls Products BV"			},
	{ 0xfd73, "BRControls Products BV"			},
	{ 0xfd72, "Logitech International SA"			},
	{ 0xfd71, "GN Hearing A/S"				},
	{ 0xfd70, "GuangDong Oppo Mobile Telecommunications Corp., Ltd." },
	{ 0xfd6f, "Apple, Inc."					},
	{ 0xfd6e, "Polidea sp. z o.o."				},
	{ 0xfd6d, "Sigma Elektro GmbH"				},
	{ 0xfd6c, "Samsung Electronics Co., Ltd."		},
	{ 0xfd6b, " rapitag GmbH"				},
	{ 0xfd6a, "Emerson"					},
	{ 0xfd69, "Samsung Electronics Co., Ltd."		},
	{ 0xfd68, "Ubique Innovation AG"			},
	{ 0xfd67, "Montblanc Simplo GmbH"			},
	{ 0xfd66, "Zebra Technologies Corporation"		},
	{ 0xfd65, "Razer Inc."					},
	{ 0xfd64, "INRIA"					},
	{ 0xfd63, "Fitbit, Inc."				},
	{ 0xfd62, "Fitbit, Inc."				},
	{ 0xfd61, "Arendi AG"					},
	{ 0xfd60, "Sercomm Corporation"				},
	{ 0xfd5f, "Oculus VR, LLC"				},
	/* SDO defined */
	{ 0xfffc, "AirFuel Alliance"				},
	{ 0xfffe, "Alliance for Wireless Power (A4WP)"		},
	{ 0xfffd, "Fast IDentity Online Alliance (FIDO)"	},
	{ }
};

static const struct {
	const char *uuid;
	const char *str;
} uuid128_table[] = {
	{ "a3c87500-8ed3-4bdf-8a39-a01bebede295",
		"Eddystone Configuration Service"			},
	{ "a3c87501-8ed3-4bdf-8a39-a01bebede295", "Capabilities"	},
	{ "a3c87502-8ed3-4bdf-8a39-a01bebede295", "Active Slot"		},
	{ "a3c87503-8ed3-4bdf-8a39-a01bebede295",
		"Advertising Interval"					},
	{ "a3c87504-8ed3-4bdf-8a39-a01bebede295", "Radio Tx Power"	},
	{ "a3c87505-8ed3-4bdf-8a39-a01bebede295",
		"(Advanced) Advertised Tx Power"			},
	{ "a3c87506-8ed3-4bdf-8a39-a01bebede295", "Lock State"		},
	{ "a3c87507-8ed3-4bdf-8a39-a01bebede295", "Unlock"		},
	{ "a3c87508-8ed3-4bdf-8a39-a01bebede295", "Public ECDH Key"	},
	{ "a3c87509-8ed3-4bdf-8a39-a01bebede295", "EID Identity Key"	},
	{ "a3c8750a-8ed3-4bdf-8a39-a01bebede295", "ADV Slot Data"	},
	{ "a3c8750b-8ed3-4bdf-8a39-a01bebede295",
		"(Advanced) Factory reset"				},
	{ "a3c8750c-8ed3-4bdf-8a39-a01bebede295",
		"(Advanced) Remain Connectable"				},
	/* BBC micro:bit Bluetooth Profiles */
	{ "e95d0753-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Accelerometer Service"			},
	{ "e95dca4b-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Accelerometer Data"				},
	{ "e95dfb24-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Accelerometer Period"				},
	{ "e95df2d8-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Magnetometer Service"				},
	{ "e95dfb11-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Magnetometer Data"				},
	{ "e95d386c-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Magnetometer Period"				},
	{ "e95d9715-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Magnetometer Bearing"				},
	{ "e95d9882-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Button Service"				},
	{ "e95dda90-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Button A State"				},
	{ "e95dda91-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Button B State"				},
	{ "e95d127b-251d-470a-a062-fa1922dfa9a8",
		"MicroBit IO PIN Service"				},
	{ "e95d8d00-251d-470a-a062-fa1922dfa9a8", "MicroBit PIN Data"	},
	{ "e95d5899-251d-470a-a062-fa1922dfa9a8",
		"MicroBit PIN AD Configuration"				},
	{ "e95dd822-251d-470a-a062-fa1922dfa9a8", "MicroBit PWM Control" },
	{ "e95dd91d-251d-470a-a062-fa1922dfa9a8", "MicroBit LED Service" },
	{ "e95d7b77-251d-470a-a062-fa1922dfa9a8", "MicroBit LED Matrix state" },
	{ "e95d93ee-251d-470a-a062-fa1922dfa9a8", "MicroBit LED Text"	},
	{ "e95d0d2d-251d-470a-a062-fa1922dfa9a8", "MicroBit Scrolling Delay" },
	{ "e95d93af-251d-470a-a062-fa1922dfa9a8", "MicroBit Event Service" },
	{ "e95db84c-251d-470a-a062-fa1922dfa9a8", "MicroBit Requirements" },
	{ "e95d9775-251d-470a-a062-fa1922dfa9a8", "MicroBit Event Data" },
	{ "e95d23c4-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Client Requirements"				},
	{ "e95d5404-251d-470a-a062-fa1922dfa9a8", "MicroBit Client Events" },
	{ "e95d93b0-251d-470a-a062-fa1922dfa9a8",
		"MicroBit DFU Control Service"				},
	{ "e95d93b1-251d-470a-a062-fa1922dfa9a8", "MicroBit DFU Control" },
	{ "e95d6100-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Temperature Service"				},
	{ "e95d1b25-251d-470a-a062-fa1922dfa9a8",
		"MicroBit Temperature Period"				},
	/* Nordic UART Port Emulation */
	{ "6e400001-b5a3-f393-e0a9-e50e24dcca9e", "Nordic UART Service" },
	{ "6e400002-b5a3-f393-e0a9-e50e24dcca9e", "Nordic UART TX"	},
	{ "6e400003-b5a3-f393-e0a9-e50e24dcca9e", "Nordic UART RX"	},
	{ }
};

const char *bt_uuid16_to_str(uint16_t uuid)
{
	int i;

	for (i = 0; uuid16_table[i].str; i++) {
		if (uuid16_table[i].uuid == uuid)
			return uuid16_table[i].str;
	}

	return "Unknown";
}

const char *bt_uuid32_to_str(uint32_t uuid)
{
	if ((uuid & 0xffff0000) == 0x0000)
		return bt_uuid16_to_str(uuid & 0x0000ffff);

	return "Unknown";
}

const char *bt_uuidstr_to_str(const char *uuid)
{
	uint32_t val;
	size_t len;
	int i;

	if (!uuid)
		return NULL;

	len = strlen(uuid);

	if (len < 36) {
		char *endptr = NULL;

		val = strtol(uuid, &endptr, 0);
		if (!endptr || *endptr != '\0')
			return NULL;

		if (val > UINT16_MAX)
			return bt_uuid32_to_str(val);

		return bt_uuid16_to_str(val);
	}

	if (len != 36)
		return NULL;

	for (i = 0; uuid128_table[i].str; i++) {
		if (strcasecmp(uuid128_table[i].uuid, uuid) == 0)
			return uuid128_table[i].str;
	}

	if (strncasecmp(uuid + 8, "-0000-1000-8000-00805f9b34fb", 28))
		return "Vendor specific";

	if (sscanf(uuid, "%08x-0000-1000-8000-00805f9b34fb", &val) != 1)
		return NULL;

	return bt_uuid32_to_str(val);
}

static const struct {
	uint16_t val;
	bool generic;
	const char *str;
} appearance_table[] = {
	{    0, true,  "Unknown"		},
	{   64, true,  "Phone"			},
	{  128, true,  "Computer"		},
	{  192, true,  "Watch"			},
	{  193, false, "Sports Watch"		},
	{  256, true,  "Clock"			},
	{  320, true,  "Display"		},
	{  384, true,  "Remote Control"		},
	{  448, true,  "Eye-glasses"		},
	{  512, true,  "Tag"			},
	{  576, true,  "Keyring"		},
	{  640, true,  "Media Player"		},
	{  704, true,  "Barcode Scanner"	},
	{  768, true,  "Thermometer"		},
	{  769, false, "Thermometer: Ear"	},
	{  832, true,  "Heart Rate Sensor"	},
	{  833, false, "Heart Rate Belt"	},
	{  896, true,  "Blood Pressure"		},
	{  897, false, "Blood Pressure: Arm"	},
	{  898, false, "Blood Pressure: Wrist"	},
	{  960, true,  "Human Interface Device"	},
	{  961, false, "Keyboard"		},
	{  962, false, "Mouse"			},
	{  963, false, "Joystick"		},
	{  964, false, "Gamepad"		},
	{  965, false, "Digitizer Tablet"	},
	{  966, false, "Card Reader"		},
	{  967, false, "Digital Pen"		},
	{  968, false, "Barcode Scanner"	},
	{ 1024, true,  "Glucose Meter"		},
	{ 1088, true,  "Running Walking Sensor"			},
	{ 1089, false, "Running Walking Sensor: In-Shoe"	},
	{ 1090, false, "Running Walking Sensor: On-Shoe"	},
	{ 1091, false, "Running Walking Sensor: On-Hip"		},
	{ 1152, true,  "Cycling"				},
	{ 1153, false, "Cycling: Cycling Computer"		},
	{ 1154, false, "Cycling: Speed Sensor"			},
	{ 1155, false, "Cycling: Cadence Sensor"		},
	{ 1156, false, "Cycling: Power Sensor"			},
	{ 1157, false, "Cycling: Speed and Cadence Sensor"	},
	{ 1216, true,  "Undefined"				},

	{ 3136, true,  "Pulse Oximeter"				},
	{ 3137, false, "Pulse Oximeter: Fingertip"		},
	{ 3138, false, "Pulse Oximeter: Wrist Worn"		},
	{ 3200, true,  "Weight Scale"				},
	{ 3264, true,  "Undefined"				},

	{ 5184, true,  "Outdoor Sports Activity"		},
	{ 5185, false, "Location Display Device"		},
	{ 5186, false, "Location and Navigation Display Device"	},
	{ 5187, false, "Location Pod"				},
	{ 5188, false, "Location and Navigation Pod"		},
	{ 5248, true,  "Undefined"				},
	{ }
};

const char *bt_appear_to_str(uint16_t appearance)
{
	const char *str = NULL;
	int i, type = 0;

	for (i = 0; appearance_table[i].str; i++) {
		if (appearance_table[i].generic) {
			if (appearance < appearance_table[i].val)
				break;
			type = i;
		}

		if (appearance_table[i].val == appearance) {
			str = appearance_table[i].str;
			break;
		}
	}

	if (!str)
		str = appearance_table[type].str;

	return str;
}

char *strdelimit(char *str, char *del, char c)
{
	char *dup;

	if (!str)
		return NULL;

	dup = strdup(str);
	if (dup[0] == '\0')
		return dup;

	while (del[0] != '\0') {
		char *rep = dup;

		while ((rep = strchr(rep, del[0])))
			rep[0] = c;

		del++;
	}

	return dup;
}

int strsuffix(const char *str, const char *suffix)
{
	int len;
	int suffix_len;

	if (!str || !suffix)
		return -1;

	if (str[0] == '\0' && suffix[0] != '\0')
		return -1;

	if (suffix[0] == '\0' && str[0] != '\0')
		return -1;

	len = strlen(str);
	suffix_len = strlen(suffix);
	if (len < suffix_len)
		return -1;

	return strncmp(str + len - suffix_len, suffix, suffix_len);
}
