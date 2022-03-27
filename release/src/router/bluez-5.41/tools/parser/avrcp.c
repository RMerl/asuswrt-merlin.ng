/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011 Intel Corporation.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "parser.h"

/* ctype entries */
#define AVC_CTYPE_CONTROL		0x0
#define AVC_CTYPE_STATUS		0x1
#define AVC_CTYPE_SPECIFIC_INQUIRY	0x2
#define AVC_CTYPE_NOTIFY		0x3
#define AVC_CTYPE_GENERAL_INQUIRY	0x4
#define AVC_CTYPE_NOT_IMPLEMENTED	0x8
#define AVC_CTYPE_ACCEPTED		0x9
#define AVC_CTYPE_REJECTED		0xA
#define AVC_CTYPE_IN_TRANSITION		0xB
#define AVC_CTYPE_STABLE		0xC
#define AVC_CTYPE_CHANGED		0xD
#define AVC_CTYPE_INTERIM		0xF

/* subunit type */
#define AVC_SUBUNIT_MONITOR		0x00
#define AVC_SUBUNIT_AUDIO		0x01
#define AVC_SUBUNIT_PRINTER		0x02
#define AVC_SUBUNIT_DISC		0x03
#define AVC_SUBUNIT_TAPE		0x04
#define AVC_SUBUNIT_TURNER		0x05
#define AVC_SUBUNIT_CA			0x06
#define AVC_SUBUNIT_CAMERA		0x07
#define AVC_SUBUNIT_PANEL		0x09
#define AVC_SUBUNIT_BULLETIN_BOARD	0x0a
#define AVC_SUBUNIT_CAMERA_STORAGE	0x0b
#define AVC_SUBUNIT_VENDOR_UNIQUE	0x0c
#define AVC_SUBUNIT_EXTENDED		0x1e
#define AVC_SUBUNIT_UNIT		0x1f

/* opcodes */
#define AVC_OP_VENDORDEP		0x00
#define AVC_OP_UNITINFO			0x30
#define AVC_OP_SUBUNITINFO		0x31
#define AVC_OP_PASSTHROUGH		0x7c

/* operands in passthrough commands */
#define AVC_PANEL_VOLUME_UP		0x41
#define AVC_PANEL_VOLUME_DOWN		0x42
#define AVC_PANEL_MUTE			0x43
#define AVC_PANEL_PLAY			0x44
#define AVC_PANEL_STOP			0x45
#define AVC_PANEL_PAUSE			0x46
#define AVC_PANEL_RECORD		0x47
#define AVC_PANEL_REWIND		0x48
#define AVC_PANEL_FAST_FORWARD		0x49
#define AVC_PANEL_EJECT			0x4a
#define AVC_PANEL_FORWARD		0x4b
#define AVC_PANEL_BACKWARD		0x4c

/* Packet types */
#define AVRCP_PACKET_TYPE_SINGLE	0x00
#define AVRCP_PACKET_TYPE_START		0x01
#define AVRCP_PACKET_TYPE_CONTINUING	0x02
#define AVRCP_PACKET_TYPE_END		0x03

/* pdu ids */
#define AVRCP_GET_CAPABILITIES		0x10
#define AVRCP_LIST_PLAYER_ATTRIBUTES	0x11
#define AVRCP_LIST_PLAYER_VALUES	0x12
#define AVRCP_GET_CURRENT_PLAYER_VALUE	0x13
#define AVRCP_SET_PLAYER_VALUE		0x14
#define AVRCP_GET_PLAYER_ATTRIBUTE_TEXT	0x15
#define AVRCP_GET_PLAYER_VALUE_TEXT	0x16
#define AVRCP_DISPLAYABLE_CHARSET	0x17
#define AVRCP_CT_BATTERY_STATUS		0x18
#define AVRCP_GET_ELEMENT_ATTRIBUTES	0x20
#define AVRCP_GET_PLAY_STATUS		0x30
#define AVRCP_REGISTER_NOTIFICATION	0x31
#define AVRCP_REQUEST_CONTINUING	0x40
#define AVRCP_ABORT_CONTINUING		0x41
#define AVRCP_SET_ABSOLUTE_VOLUME	0x50
#define AVRCP_SET_ADDRESSED_PLAYER	0x60
#define AVRCP_SET_BROWSED_PLAYER	0x70
#define AVRCP_GET_FOLDER_ITEMS		0x71
#define AVRCP_CHANGE_PATH		0x72
#define AVRCP_GET_ITEM_ATTRIBUTES	0x73
#define AVRCP_PLAY_ITEM			0x74
#define AVRCP_SEARCH			0x80
#define AVRCP_ADD_TO_NOW_PLAYING	0x90
#define AVRCP_GENERAL_REJECT		0xA0

/* notification events */
#define AVRCP_EVENT_PLAYBACK_STATUS_CHANGED		0x01
#define AVRCP_EVENT_TRACK_CHANGED			0x02
#define AVRCP_EVENT_TRACK_REACHED_END			0x03
#define AVRCP_EVENT_TRACK_REACHED_START			0x04
#define AVRCP_EVENT_PLAYBACK_POS_CHANGED		0x05
#define AVRCP_EVENT_BATT_STATUS_CHANGED			0x06
#define AVRCP_EVENT_SYSTEM_STATUS_CHANGED		0x07
#define AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED	0x08
#define AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED		0x09
#define AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED		0x0a
#define AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED		0x0b
#define AVRCP_EVENT_UIDS_CHANGED			0x0c
#define AVRCP_EVENT_VOLUME_CHANGED			0x0d

/* error statuses */
#define AVRCP_STATUS_INVALID_COMMAND			0x00
#define AVRCP_STATUS_INVALID_PARAMETER			0x01
#define AVRCP_STATUS_NOT_FOUND				0x02
#define AVRCP_STATUS_INTERNAL_ERROR			0x03
#define AVRCP_STATUS_SUCCESS				0x04
#define AVRCP_STATUS_UID_CHANGED			0x05
#define AVRCP_STATUS_INVALID_DIRECTION			0x07
#define AVRCP_STATUS_NOT_DIRECTORY			0x08
#define AVRCP_STATUS_DOES_NOT_EXIST			0x09
#define AVRCP_STATUS_INVALID_SCOPE			0x0a
#define AVRCP_STATUS_OUT_OF_BOUNDS			0x0b
#define AVRCP_STATUS_IS_DIRECTORY			0x0c
#define AVRCP_STATUS_MEDIA_IN_USE			0x0d
#define AVRCP_STATUS_NOW_PLAYING_LIST_FULL		0x0e
#define AVRCP_STATUS_SEARCH_NOT_SUPPORTED		0x0f
#define AVRCP_STATUS_SEARCH_IN_PROGRESS			0x10
#define AVRCP_STATUS_INVALID_PLAYER_ID			0x11
#define AVRCP_STATUS_PLAYER_NOT_BROWSABLE		0x12
#define AVRCP_STATUS_PLAYER_NOT_ADDRESSED		0x13
#define AVRCP_STATUS_NO_VALID_SEARCH_RESULTS		0x14
#define AVRCP_STATUS_NO_AVAILABLE_PLAYERS		0x15
#define AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED		0x16

/* player attributes */
#define AVRCP_ATTRIBUTE_ILEGAL		0x00
#define AVRCP_ATTRIBUTE_EQUALIZER	0x01
#define AVRCP_ATTRIBUTE_REPEAT_MODE	0x02
#define AVRCP_ATTRIBUTE_SHUFFLE		0x03
#define AVRCP_ATTRIBUTE_SCAN		0x04

/* media attributes */
#define AVRCP_MEDIA_ATTRIBUTE_ILLEGAL	0x0
#define AVRCP_MEDIA_ATTRIBUTE_TITLE	0x1
#define AVRCP_MEDIA_ATTRIBUTE_ARTIST	0x2
#define AVRCP_MEDIA_ATTRIBUTE_ALBUM	0x3
#define AVRCP_MEDIA_ATTRIBUTE_TRACK	0x4
#define AVRCP_MEDIA_ATTRIBUTE_TOTAL	0x5
#define AVRCP_MEDIA_ATTRIBUTE_GENRE	0x6
#define AVRCP_MEDIA_ATTRIBUTE_DURATION	0x7

/* play status */
#define AVRCP_PLAY_STATUS_STOPPED	0x00
#define AVRCP_PLAY_STATUS_PLAYING	0x01
#define AVRCP_PLAY_STATUS_PAUSED	0x02
#define AVRCP_PLAY_STATUS_FWD_SEEK	0x03
#define AVRCP_PLAY_STATUS_REV_SEEK	0x04
#define AVRCP_PLAY_STATUS_ERROR		0xFF

/* media scope */
#define AVRCP_MEDIA_PLAYER_LIST		0x00
#define AVRCP_MEDIA_PLAYER_VFS		0x01
#define AVRCP_MEDIA_SEARCH		0x02
#define AVRCP_MEDIA_NOW_PLAYING		0x03

static struct avrcp_continuing {
	uint16_t num;
	uint16_t size;
} avrcp_continuing;

static const char *ctype2str(uint8_t ctype)
{
	switch (ctype & 0x0f) {
	case AVC_CTYPE_CONTROL:
		return "Control";
	case AVC_CTYPE_STATUS:
		return "Status";
	case AVC_CTYPE_SPECIFIC_INQUIRY:
		return "Specific Inquiry";
	case AVC_CTYPE_NOTIFY:
		return "Notify";
	case AVC_CTYPE_GENERAL_INQUIRY:
		return "General Inquiry";
	case AVC_CTYPE_NOT_IMPLEMENTED:
		return "Not Implemented";
	case AVC_CTYPE_ACCEPTED:
		return "Accepted";
	case AVC_CTYPE_REJECTED:
		return "Rejected";
	case AVC_CTYPE_IN_TRANSITION:
		return "In Transition";
	case AVC_CTYPE_STABLE:
		return "Stable";
	case AVC_CTYPE_CHANGED:
		return "Changed";
	case AVC_CTYPE_INTERIM:
		return "Interim";
	default:
		return "Unknown";
	}
}

static const char *opcode2str(uint8_t opcode)
{
	switch (opcode) {
	case AVC_OP_VENDORDEP:
		return "Vendor Dependent";
	case AVC_OP_UNITINFO:
		return "Unit Info";
	case AVC_OP_SUBUNITINFO:
		return "Subunit Info";
	case AVC_OP_PASSTHROUGH:
		return "Passthrough";
	default:
		return "Unknown";
	}
}

static const char *pt2str(uint8_t pt)
{
	switch (pt) {
	case AVRCP_PACKET_TYPE_SINGLE:
		return "Single";
	case AVRCP_PACKET_TYPE_START:
		return "Start";
	case AVRCP_PACKET_TYPE_CONTINUING:
		return "Continuing";
	case AVRCP_PACKET_TYPE_END:
		return "End";
	default:
		return "Unknown";
	}
}

static const char *pdu2str(uint8_t pduid)
{
	switch (pduid) {
	case AVRCP_GET_CAPABILITIES:
		return "GetCapabilities";
	case AVRCP_LIST_PLAYER_ATTRIBUTES:
		return "ListPlayerApplicationSettingAttributes";
	case AVRCP_LIST_PLAYER_VALUES:
		return "ListPlayerApplicationSettingValues";
	case AVRCP_GET_CURRENT_PLAYER_VALUE:
		return "GetCurrentPlayerApplicationSettingValue";
	case AVRCP_SET_PLAYER_VALUE:
		return "SetPlayerApplicationSettingValue";
	case AVRCP_GET_PLAYER_ATTRIBUTE_TEXT:
		return "GetPlayerApplicationSettingAttributeText";
	case AVRCP_GET_PLAYER_VALUE_TEXT:
		return "GetPlayerApplicationSettingValueText";
	case AVRCP_DISPLAYABLE_CHARSET:
		return "InformDisplayableCharacterSet";
	case AVRCP_CT_BATTERY_STATUS:
		return "InformBatteryStatusOfCT";
	case AVRCP_GET_ELEMENT_ATTRIBUTES:
		return "GetElementAttributes";
	case AVRCP_GET_PLAY_STATUS:
		return "GetPlayStatus";
	case AVRCP_REGISTER_NOTIFICATION:
		return "RegisterNotification";
	case AVRCP_REQUEST_CONTINUING:
		return "RequestContinuingResponse";
	case AVRCP_ABORT_CONTINUING:
		return "AbortContinuingResponse";
	case AVRCP_SET_ABSOLUTE_VOLUME:
		return "SetAbsoluteVolume";
	case AVRCP_SET_ADDRESSED_PLAYER:
		return "SetAddressedPlayer";
	case AVRCP_SET_BROWSED_PLAYER:
		return "SetBrowsedPlayer";
	case AVRCP_GET_FOLDER_ITEMS:
		return "GetFolderItems";
	case AVRCP_CHANGE_PATH:
		return "ChangePath";
	case AVRCP_GET_ITEM_ATTRIBUTES:
		return "GetItemAttributes";
	case AVRCP_PLAY_ITEM:
		return "PlayItem";
	case AVRCP_SEARCH:
		return "Search";
	case AVRCP_ADD_TO_NOW_PLAYING:
		return "AddToNowPlaying";
	case AVRCP_GENERAL_REJECT:
		return "GeneralReject";
	default:
		return "Unknown";
	}
}

static char *cap2str(uint8_t cap)
{
	switch (cap) {
	case 0x2:
		return "CompanyID";
	case 0x3:
		return "EventsID";
	default:
		return "Unknown";
	}
}

static char *event2str(uint8_t event)
{
	switch (event) {
	case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
		return "EVENT_PLAYBACK_STATUS_CHANGED";
	case AVRCP_EVENT_TRACK_CHANGED:
		return "EVENT_TRACK_CHANGED";
	case AVRCP_EVENT_TRACK_REACHED_END:
		return "EVENT_TRACK_REACHED_END";
	case AVRCP_EVENT_TRACK_REACHED_START:
		return "EVENT_TRACK_REACHED_START";
	case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		return "EVENT_PLAYBACK_POS_CHANGED";
	case AVRCP_EVENT_BATT_STATUS_CHANGED:
		return "EVENT_BATT_STATUS_CHANGED";
	case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
		return "EVENT_SYSTEM_STATUS_CHANGED";
	case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
		return "EVENT_PLAYER_APPLICATION_SETTING_CHANGED";
	case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:
		return "EVENT_NOW_PLAYING_CONTENT_CHANGED";
	case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
		return "EVENT_AVAILABLE_PLAYERS_CHANGED";
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		return "EVENT_ADDRESSED_PLAYER_CHANGED";
	case AVRCP_EVENT_UIDS_CHANGED:
		return "EVENT_UIDS_CHANGED";
	case AVRCP_EVENT_VOLUME_CHANGED:
		return "EVENT_VOLUME_CHANGED";
	default:
		return "Reserved";
	}
}

static const char *error2str(uint8_t status)
{
	switch (status) {
	case AVRCP_STATUS_INVALID_COMMAND:
		return "Invalid Command";
	case AVRCP_STATUS_INVALID_PARAMETER:
		return "Invalid Parameter";
	case AVRCP_STATUS_NOT_FOUND:
		return "Not Found";
	case AVRCP_STATUS_INTERNAL_ERROR:
		return "Internal Error";
	case AVRCP_STATUS_SUCCESS:
		return "Success";
	case AVRCP_STATUS_UID_CHANGED:
		return "UID Changed";
	case AVRCP_STATUS_INVALID_DIRECTION:
		return "Invalid Direction";
	case AVRCP_STATUS_NOT_DIRECTORY:
		return "Not a Directory";
	case AVRCP_STATUS_DOES_NOT_EXIST:
		return "Does Not Exist";
	case AVRCP_STATUS_INVALID_SCOPE:
		return "Invalid Scope";
	case AVRCP_STATUS_OUT_OF_BOUNDS:
		return "Range Out of Bonds";
	case AVRCP_STATUS_MEDIA_IN_USE:
		return "Media in Use";
	case AVRCP_STATUS_IS_DIRECTORY:
		return "UID is a Directory";
	case AVRCP_STATUS_NOW_PLAYING_LIST_FULL:
		return "Now Playing List Full";
	case AVRCP_STATUS_SEARCH_NOT_SUPPORTED:
		return "Seach Not Supported";
	case AVRCP_STATUS_SEARCH_IN_PROGRESS:
		return "Search in Progress";
	case AVRCP_STATUS_INVALID_PLAYER_ID:
		return "Invalid Player ID";
	case AVRCP_STATUS_PLAYER_NOT_BROWSABLE:
		return "Player Not Browsable";
	case AVRCP_STATUS_PLAYER_NOT_ADDRESSED:
		return "Player Not Addressed";
	case AVRCP_STATUS_NO_VALID_SEARCH_RESULTS:
		return "No Valid Search Result";
	case AVRCP_STATUS_NO_AVAILABLE_PLAYERS:
		return "No Available Players";
	case AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED:
		return "Addressed Player Changed";
	default:
		return "Unknown";
	}
}

static void avrcp_rejected_dump(int level, struct frame *frm, uint16_t len)
{
	uint8_t status;

	p_indent(level, frm);

	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	status = p_get_u8(frm);
	printf("Error: 0x%02x (%s)\n", status, error2str(status));
}

static void avrcp_get_capabilities_dump(int level, struct frame *frm, uint16_t len)
{
	uint8_t cap;
	uint8_t count;

	p_indent(level, frm);

	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	cap = p_get_u8(frm);
	printf("CapabilityID: 0x%02x (%s)\n", cap, cap2str(cap));

	if (len == 1)
		return;

	p_indent(level, frm);

	count = p_get_u8(frm);
	printf("CapabilityCount: 0x%02x\n", count);

	switch (cap) {
	case 0x2:
		for (; count > 0; count--) {
			int i;

			p_indent(level, frm);

			printf("%s: 0x", cap2str(cap));
			for (i = 0; i < 3; i++)
				printf("%02x", p_get_u8(frm));
			printf("\n");
		}
		break;
	case 0x3:
		for (; count > 0; count--) {
			uint8_t event;

			p_indent(level, frm);

			event = p_get_u8(frm);
			printf("%s: 0x%02x (%s)\n", cap2str(cap), event,
							event2str(event));
		}
		break;
	default:
		raw_dump(level, frm);
	}
}

static const char *attr2str(uint8_t attr)
{
	switch (attr) {
	case AVRCP_ATTRIBUTE_ILEGAL:
		return "Illegal";
	case AVRCP_ATTRIBUTE_EQUALIZER:
		return "Equalizer ON/OFF Status";
	case AVRCP_ATTRIBUTE_REPEAT_MODE:
		return "Repeat Mode Status";
	case AVRCP_ATTRIBUTE_SHUFFLE:
		return "Shuffle ON/OFF Status";
	case AVRCP_ATTRIBUTE_SCAN:
		return "Scan ON/OFF Status";
	default:
		return "Unknown";
	}
}

static void avrcp_list_player_attributes_dump(int level, struct frame *frm,
								uint16_t len)
{
	uint8_t num;

	if (len == 0)
		return;

	p_indent(level, frm);

	num = p_get_u8(frm);
	printf("AttributeCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint8_t attr;

		p_indent(level, frm);

		attr = p_get_u8(frm);
		printf("AttributeID: 0x%02x (%s)\n", attr, attr2str(attr));
	}
}

static const char *value2str(uint8_t attr, uint8_t value)
{
	switch (attr) {
	case AVRCP_ATTRIBUTE_ILEGAL:
		return "Illegal";
	case AVRCP_ATTRIBUTE_EQUALIZER:
		switch (value) {
		case 0x01:
			return "OFF";
		case 0x02:
			return "ON";
		default:
			return "Reserved";
		}
	case AVRCP_ATTRIBUTE_REPEAT_MODE:
		switch (value) {
		case 0x01:
			return "OFF";
		case 0x02:
			return "Single Track Repeat";
		case 0x03:
			return "All Track Repeat";
		case 0x04:
			return "Group Repeat";
		default:
			return "Reserved";
		}
	case AVRCP_ATTRIBUTE_SHUFFLE:
		switch (value) {
		case 0x01:
			return "OFF";
		case 0x02:
			return "All Track Suffle";
		case 0x03:
			return "Group Suffle";
		default:
			return "Reserved";
		}
	case AVRCP_ATTRIBUTE_SCAN:
		switch (value) {
		case 0x01:
			return "OFF";
		case 0x02:
			return "All Track Scan";
		case 0x03:
			return "Group Scan";
		default:
			return "Reserved";
		}
	default:
		return "Unknown";
	}
}

static void avrcp_list_player_values_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	static uint8_t attr = 0; /* Remember attribute */
	uint8_t num;

	p_indent(level, frm);

	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	attr = p_get_u8(frm);
	printf("AttributeID: 0x%02x (%s)\n", attr, attr2str(attr));

	return;

response:
	num = p_get_u8(frm);
	printf("ValueCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint8_t value;

		p_indent(level, frm);

		value = p_get_u8(frm);
		printf("ValueID: 0x%02x (%s)\n", value,
						value2str(attr, value));
	}
}

static void avrcp_get_current_player_value_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint8_t num;

	p_indent(level, frm);

	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	num = p_get_u8(frm);
	printf("AttributeCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint8_t attr;

		p_indent(level, frm);

		attr = p_get_u8(frm);
		printf("AttributeID: 0x%02x (%s)\n", attr, attr2str(attr));
	}

	return;

response:
	num = p_get_u8(frm);
	printf("ValueCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint8_t attr, value;

		p_indent(level, frm);

		attr = p_get_u8(frm);
		printf("AttributeID: 0x%02x (%s)\n", attr, attr2str(attr));

		p_indent(level, frm);

		value = p_get_u8(frm);
		printf("ValueID: 0x%02x (%s)\n", value,
						value2str(attr, value));
	}
}

static void avrcp_set_player_value_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint8_t num;

	p_indent(level, frm);

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		return;

	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	num = p_get_u8(frm);
	printf("AttributeCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint8_t attr, value;

		p_indent(level, frm);

		attr = p_get_u8(frm);
		printf("AttributeID: 0x%02x (%s)\n", attr, attr2str(attr));

		p_indent(level, frm);

		value = p_get_u8(frm);
		printf("ValueID: 0x%02x (%s)\n", value,
						value2str(attr, value));
	}
}

static const char *charset2str(uint16_t charset)
{
	switch (charset) {
	case 1:
	case 2:
		return "Reserved";
	case 3:
		return "ASCII";
	case 4:
		return "ISO_8859-1";
	case 5:
		return "ISO_8859-2";
	case 6:
		return "ISO_8859-3";
	case 7:
		return "ISO_8859-4";
	case 8:
		return "ISO_8859-5";
	case 9:
		return "ISO_8859-6";
	case 10:
		return "ISO_8859-7";
	case 11:
		return "ISO_8859-8";
	case 12:
		return "ISO_8859-9";
	case 106:
		return "UTF-8";
	default:
		return "Unknown";
	}
}

static void avrcp_get_player_attribute_text_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint8_t num;

	p_indent(level, frm);

	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	num = p_get_u8(frm);
	printf("AttributeCount: 0x%02x\n", num);

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	for (; num > 0; num--) {
		uint8_t attr;

		p_indent(level, frm);

		attr = p_get_u8(frm);
		printf("AttributeID: 0x%02x (%s)\n", attr, attr2str(attr));
	}

	return;

response:
	for (; num > 0; num--) {
		uint8_t attr, len;
		uint16_t charset;

		p_indent(level, frm);

		attr = p_get_u8(frm);
		printf("AttributeID: 0x%02x (%s)\n", attr, attr2str(attr));

		p_indent(level, frm);

		charset = p_get_u16(frm);
		printf("CharsetID: 0x%04x (%s)\n", charset,
							charset2str(charset));

		p_indent(level, frm);

		len = p_get_u8(frm);
		printf("StringLength: 0x%02x\n", len);

		p_indent(level, frm);

		printf("String: ");
		for (; len > 0; len--) {
			uint8_t c = p_get_u8(frm);
			printf("%1c", isprint(c) ? c : '.');
		}
		printf("\n");
	}
}

static void avrcp_get_player_value_text_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	static uint8_t attr = 0; /* Remember attribute */
	uint8_t num;

	p_indent(level, frm);

	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	attr = p_get_u8(frm);
	printf("AttributeID: 0x%02x (%s)\n", attr, attr2str(attr));

	p_indent(level, frm);

	num = p_get_u8(frm);
	printf("ValueCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint8_t value;

		p_indent(level, frm);

		value = p_get_u8(frm);
		printf("ValueID: 0x%02x (%s)\n", value,
						value2str(attr, value));
	}

	return;

response:
	num = p_get_u8(frm);
	printf("ValueCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint8_t value, len;
		uint16_t charset;

		p_indent(level, frm);

		value = p_get_u8(frm);
		printf("ValueID: 0x%02x (%s)\n", value,
						value2str(attr, value));

		p_indent(level, frm);

		charset = p_get_u16(frm);
		printf("CharsetID: 0x%04x (%s)\n", charset,
							charset2str(charset));

		p_indent(level, frm);

		len = p_get_u8(frm);
		printf("StringLength: 0x%02x\n", len);

		p_indent(level, frm);

		printf("String: ");
		for (; len > 0; len--) {
			uint8_t c = p_get_u8(frm);
			printf("%1c", isprint(c) ? c : '.');
		}
		printf("\n");
	}
}

static void avrcp_displayable_charset(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint8_t num;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		return;

	p_indent(level, frm);

	if (len < 2) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	num = p_get_u8(frm);
	printf("CharsetCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint16_t charset;

		p_indent(level, frm);

		charset = p_get_u16(frm);
		printf("CharsetID: 0x%04x (%s)\n", charset,
							charset2str(charset));
	}
}

static const char *status2str(uint8_t status)
{
	switch (status) {
	case 0x0:
		return "NORMAL";
	case 0x1:
		return "WARNING";
	case 0x2:
		return "CRITICAL";
	case 0x3:
		return "EXTERNAL";
	case 0x4:
		return "FULL_CHARGE";
	default:
		return "Reserved";
	}
}

static void avrcp_ct_battery_status_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint8_t status;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		return;

	p_indent(level, frm);

	status = p_get_u8(frm);
	printf("BatteryStatus: 0x%02x (%s)\n", status, status2str(status));
}

static const char *mediattr2str(uint32_t attr)
{
	switch (attr) {
	case AVRCP_MEDIA_ATTRIBUTE_ILLEGAL:
		return "Illegal";
	case AVRCP_MEDIA_ATTRIBUTE_TITLE:
		return "Title";
	case AVRCP_MEDIA_ATTRIBUTE_ARTIST:
		return "Artist";
	case AVRCP_MEDIA_ATTRIBUTE_ALBUM:
		return "Album";
	case AVRCP_MEDIA_ATTRIBUTE_TRACK:
		return "Track";
	case AVRCP_MEDIA_ATTRIBUTE_TOTAL:
		return "Track Total";
	case AVRCP_MEDIA_ATTRIBUTE_GENRE:
		return "Genre";
	case AVRCP_MEDIA_ATTRIBUTE_DURATION:
		return "Track duration";
	default:
		return "Reserved";
	}
}

static void avrcp_get_element_attributes_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len,
						uint8_t pt)
{
	uint64_t id;
	uint8_t num;

	p_indent(level, frm);

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (len < 9) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	id = p_get_u64(frm);
	printf("Identifier: 0x%jx (%s)\n", id, id ? "Reserved" : "PLAYING");

	p_indent(level, frm);

	num = p_get_u8(frm);
	printf("AttributeCount: 0x%02x\n", num);

	for (; num > 0; num--) {
		uint32_t attr;

		p_indent(level, frm);

		attr = p_get_u32(frm);
		printf("Attribute: 0x%08x (%s)\n", attr, mediattr2str(attr));
	}

	return;

response:
	if (pt == AVRCP_PACKET_TYPE_SINGLE || pt == AVRCP_PACKET_TYPE_START) {
		if (len < 1) {
			printf("PDU Malformed\n");
			raw_dump(level, frm);
			return;
		}

		num = p_get_u8(frm);
		avrcp_continuing.num = num;
		printf("AttributeCount: 0x%02x\n", num);
		len--;
	} else {
		num = avrcp_continuing.num;

		if (avrcp_continuing.size > 0) {
			uint16_t size;

			if (avrcp_continuing.size > len) {
				size = len;
				avrcp_continuing.size -= len;
			} else {
				size = avrcp_continuing.size;
				avrcp_continuing.size = 0;
			}

			printf("ContinuingAttributeValue: ");
			for (; size > 0; size--) {
				uint8_t c = p_get_u8(frm);
				printf("%1c", isprint(c) ? c : '.');
			}
			printf("\n");

			len -= size;
		}
	}

	while (num > 0 && len > 0) {
		uint32_t attr;
		uint16_t charset, attrlen;

		p_indent(level, frm);

		attr = p_get_u32(frm);
		printf("Attribute: 0x%08x (%s)\n", attr, mediattr2str(attr));

		p_indent(level, frm);

		charset = p_get_u16(frm);
		printf("CharsetID: 0x%04x (%s)\n", charset,
							charset2str(charset));

		p_indent(level, frm);
		attrlen = p_get_u16(frm);
		printf("AttributeValueLength: 0x%04x\n", attrlen);

		len -= sizeof(attr) + sizeof(charset) + sizeof(attrlen);
		num--;

		p_indent(level, frm);

		printf("AttributeValue: ");
		for (; attrlen > 0 && len > 0; attrlen--, len--) {
			uint8_t c = p_get_u8(frm);
			printf("%1c", isprint(c) ? c : '.');
		}
		printf("\n");

		if (attrlen > 0)
			avrcp_continuing.size = attrlen;
	}

	avrcp_continuing.num = num;
}

static const char *playstatus2str(uint8_t status)
{
	switch (status) {
	case AVRCP_PLAY_STATUS_STOPPED:
		return "STOPPED";
	case AVRCP_PLAY_STATUS_PLAYING:
		return "PLAYING";
	case AVRCP_PLAY_STATUS_PAUSED:
		return "PAUSED";
	case AVRCP_PLAY_STATUS_FWD_SEEK:
		return "FWD_SEEK";
	case AVRCP_PLAY_STATUS_REV_SEEK:
		return "REV_SEEK";
	case AVRCP_PLAY_STATUS_ERROR:
		return "ERROR";
	default:
		return "Unknown";
	}
}

static void avrcp_get_play_status_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint32_t interval;
	uint8_t status;

	if (ctype <= AVC_CTYPE_GENERAL_INQUIRY)
		return;

	p_indent(level, frm);

	if (len < 9) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	interval = p_get_u32(frm);
	printf("SongLength: 0x%08x (%u miliseconds)\n", interval, interval);

	p_indent(level, frm);

	interval = p_get_u32(frm);
	printf("SongPosition: 0x%08x (%u miliconds)\n", interval, interval);

	p_indent(level, frm);

	status = p_get_u8(frm);
	printf("PlayStatus: 0x%02x (%s)\n", status, playstatus2str(status));
}

static void avrcp_register_notification_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint8_t event, status;
	uint16_t uid;
	uint32_t interval;
	uint64_t id;

	p_indent(level, frm);

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (len < 5) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	event = p_get_u8(frm);
	printf("EventID: 0x%02x (%s)\n", event, event2str(event));

	p_indent(level, frm);

	interval = p_get_u32(frm);
	printf("Interval: 0x%08x (%u seconds)\n", interval, interval);

	return;

response:
	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	event = p_get_u8(frm);
	printf("EventID: 0x%02x (%s)\n", event, event2str(event));

	p_indent(level, frm);

	switch (event) {
	case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
		status = p_get_u8(frm);
		printf("PlayStatus: 0x%02x (%s)\n", status,
						playstatus2str(status));
		break;
	case AVRCP_EVENT_TRACK_CHANGED:
		id = p_get_u64(frm);
		printf("Identifier: 0x%16" PRIx64 " (%" PRIu64 ")\n", id, id);
		break;
	case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		interval = p_get_u32(frm);
		printf("Position: 0x%08x (%u miliseconds)\n", interval,
								interval);
		break;
	case AVRCP_EVENT_BATT_STATUS_CHANGED:
		status = p_get_u8(frm);
		printf("BatteryStatus: 0x%02x (%s)\n", status,
							status2str(status));
		break;
	case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
		status = p_get_u8(frm);
		printf("SystemStatus: 0x%02x ", status);
		switch (status) {
		case 0x00:
			printf("(POWER_ON)\n");
			break;
		case 0x01:
			printf("(POWER_OFF)\n");
			break;
		case 0x02:
			printf("(UNPLUGGED)\n");
			break;
		default:
			printf("(UNKOWN)\n");
			break;
		}
		break;
	case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
		status = p_get_u8(frm);
		printf("AttributeCount: 0x%02x\n", status);

		for (; status > 0; status--) {
			uint8_t attr, value;

			p_indent(level, frm);

			attr = p_get_u8(frm);
			printf("AttributeID: 0x%02x (%s)\n", attr,
							attr2str(attr));

			p_indent(level, frm);

			value = p_get_u8(frm);
			printf("ValueID: 0x%02x (%s)\n", value,
						value2str(attr, value));
		}
		break;
	case AVRCP_EVENT_VOLUME_CHANGED:
		status = p_get_u8(frm) & 0x7F;
		printf("Volume: %.2f%% (%d/127)\n", status/1.27, status);
		break;
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		uid = p_get_u16(frm);
		printf("PlayerID: 0x%04x (%u)\n", uid, uid);


		p_indent(level, frm);

		uid = p_get_u16(frm);
		printf("UIDCounter: 0x%04x (%u)\n", uid, uid);
		break;
	case AVRCP_EVENT_UIDS_CHANGED:
		uid = p_get_u16(frm);
		printf("UIDCounter: 0x%04x (%u)\n", uid, uid);
		break;
	}
}

static void avrcp_set_absolute_volume_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint8_t value;

	p_indent(level, frm);

	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	value = p_get_u8(frm) & 0x7F;
	printf("Volume: %.2f%% (%d/127)\n", value/1.27, value);
}

static void avrcp_set_addressed_player(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint16_t id;
	uint8_t status;

	p_indent(level, frm);

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (len < 2) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	id = p_get_u16(frm);
	printf("PlayerID: 0x%04x (%u)\n", id, id);
	return;

response:
	if (len < 1) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));
}

static void avrcp_set_browsed_player_dump(int level, struct frame *frm,
						uint8_t hdr, uint16_t len)
{
	uint32_t items;
	uint16_t id, uids, charset;
	uint8_t status, folders;

	p_indent(level, frm);

	if (hdr & 0x02)
		goto response;

	if (len < 2) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	id = p_get_u16(frm);
	printf("PlayerID: 0x%04x (%u)\n", id, id);
	return;

response:
	if (len != 1 && len < 10) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));

	if (len == 1)
		return;

	p_indent(level, frm);

	uids = p_get_u16(frm);
	printf("UIDCounter: 0x%04x (%u)\n", uids, uids);

	p_indent(level, frm);

	items = p_get_u32(frm);
	printf("Number of Items: 0x%08x (%u)\n", items, items);

	p_indent(level, frm);

	charset = p_get_u16(frm);
	printf("CharsetID: 0x%04x (%s)\n", charset, charset2str(charset));

	p_indent(level, frm);

	folders = p_get_u8(frm);
	printf("Folder Depth: 0x%02x (%u)\n", folders, folders);

	for (; folders > 0; folders--) {
		uint16_t len;

		p_indent(level, frm);

		len = p_get_u8(frm);
		printf("Folder: ");
		for (; len > 0; len--) {
			uint8_t c = p_get_u8(frm);
			printf("%1c", isprint(c) ? c : '.');
		}
		printf("\n");
	}
}

static const char *scope2str(uint8_t scope)
{
	switch (scope) {
	case AVRCP_MEDIA_PLAYER_LIST:
		return "Media Player List";
	case AVRCP_MEDIA_PLAYER_VFS:
		return "Media Player Virtual Filesystem";
	case AVRCP_MEDIA_SEARCH:
		return "Search";
	case AVRCP_MEDIA_NOW_PLAYING:
		return "Now Playing";
	default:
		return "Unknown";
	}
}

static void avrcp_play_item_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint64_t uid;
	uint32_t uidcounter;
	uint8_t scope, status;

	p_indent(level, frm);

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (len < 11) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	scope = p_get_u8(frm);
	printf("Scope: 0x%02x (%s)\n", scope, scope2str(scope));

	p_indent(level, frm);

	uid = p_get_u64(frm);
	printf("UID: 0x%16" PRIx64 " (%" PRIu64 ")\n", uid, uid);

	p_indent(level, frm);

	uidcounter = p_get_u16(frm);
	printf("UIDCounter: 0x%04x (%u)\n", uidcounter, uidcounter);

	return;

response:
	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));
}

static void avrcp_add_to_now_playing_dump(int level, struct frame *frm,
						uint8_t ctype, uint16_t len)
{
	uint64_t uid;
	uint32_t uidcounter;
	uint8_t scope, status;

	p_indent(level, frm);

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (len < 11) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	scope = p_get_u8(frm);
	printf("Scope: 0x%02x (%s)\n", scope, scope2str(scope));

	p_indent(level, frm);

	uid = p_get_u64(frm);
	printf("UID: 0x%16" PRIx64 " (%" PRIu64 ")\n", uid, uid);

	p_indent(level, frm);

	uidcounter = p_get_u16(frm);
	printf("UIDCounter: 0x%04x (%u)\n", uidcounter, uidcounter);

	return;

response:
	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));
}

static void avrcp_pdu_dump(int level, struct frame *frm, uint8_t ctype)
{
	uint8_t pduid, pt;
	uint16_t len;

	p_indent(level, frm);

	pduid = p_get_u8(frm);
	pt = p_get_u8(frm);
	len = p_get_u16(frm);

	printf("AVRCP: %s: pt %s len 0x%04x\n", pdu2str(pduid),
							pt2str(pt), len);

	if (len != frm->len) {
		p_indent(level, frm);
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	if (ctype == AVC_CTYPE_REJECTED) {
		avrcp_rejected_dump(level + 1, frm, len);
		return;
	}

	switch (pduid) {
	case AVRCP_GET_CAPABILITIES:
		avrcp_get_capabilities_dump(level + 1, frm, len);
		break;
	case AVRCP_LIST_PLAYER_ATTRIBUTES:
		avrcp_list_player_attributes_dump(level + 1, frm, len);
		break;
	case AVRCP_LIST_PLAYER_VALUES:
		avrcp_list_player_values_dump(level + 1, frm, ctype, len);
		break;
	case AVRCP_GET_CURRENT_PLAYER_VALUE:
		avrcp_get_current_player_value_dump(level + 1, frm, ctype,
									len);
		break;
	case AVRCP_SET_PLAYER_VALUE:
		avrcp_set_player_value_dump(level + 1, frm, ctype, len);
		break;
	case AVRCP_GET_PLAYER_ATTRIBUTE_TEXT:
		avrcp_get_player_attribute_text_dump(level + 1, frm, ctype,
									len);
		break;
	case AVRCP_GET_PLAYER_VALUE_TEXT:
		avrcp_get_player_value_text_dump(level + 1, frm, ctype, len);
		break;
	case AVRCP_DISPLAYABLE_CHARSET:
		avrcp_displayable_charset(level + 1, frm, ctype, len);
		break;
	case AVRCP_CT_BATTERY_STATUS:
		avrcp_ct_battery_status_dump(level + 1, frm, ctype, len);
		break;
	case AVRCP_GET_ELEMENT_ATTRIBUTES:
		avrcp_get_element_attributes_dump(level + 1, frm, ctype, len,
									pt);
		break;
	case AVRCP_GET_PLAY_STATUS:
		avrcp_get_play_status_dump(level + 1, frm, ctype, len);
		break;
	case AVRCP_REGISTER_NOTIFICATION:
		avrcp_register_notification_dump(level + 1, frm, ctype, len);
		break;
	case AVRCP_SET_ABSOLUTE_VOLUME:
		avrcp_set_absolute_volume_dump(level + 1, frm, ctype, len);
		break;
	case AVRCP_SET_ADDRESSED_PLAYER:
		avrcp_set_addressed_player(level + 1, frm, ctype, len);
		break;
	case AVRCP_PLAY_ITEM:
		avrcp_play_item_dump(level + 1, frm, ctype, len);
		break;
	case AVRCP_ADD_TO_NOW_PLAYING:
		avrcp_add_to_now_playing_dump(level + 1, frm, ctype, len);
		break;
	default:
		raw_dump(level, frm);
	}
}

static char *op2str(uint8_t op)
{
	switch (op & 0x7f) {
	case AVC_PANEL_VOLUME_UP:
		return "VOLUME UP";
	case AVC_PANEL_VOLUME_DOWN:
		return "VOLUME DOWN";
	case AVC_PANEL_MUTE:
		return "MUTE";
	case AVC_PANEL_PLAY:
		return "PLAY";
	case AVC_PANEL_STOP:
		return "STOP";
	case AVC_PANEL_PAUSE:
		return "PAUSE";
	case AVC_PANEL_RECORD:
		return "RECORD";
	case AVC_PANEL_REWIND:
		return "REWIND";
	case AVC_PANEL_FAST_FORWARD:
		return "FAST FORWARD";
	case AVC_PANEL_EJECT:
		return "EJECT";
	case AVC_PANEL_FORWARD:
		return "FORWARD";
	case AVC_PANEL_BACKWARD:
		return "BACKWARD";
	default:
		return "UNKNOWN";
	}
}


static void avrcp_passthrough_dump(int level, struct frame *frm)
{
	uint8_t op, len;

	p_indent(level, frm);

	op = p_get_u8(frm);
	printf("Operation: 0x%02x (%s %s)\n", op, op2str(op),
					op & 0x80 ? "Released" : "Pressed");

	p_indent(level, frm);

	len = p_get_u8(frm);

	printf("Lenght: 0x%02x\n", len);

	raw_dump(level, frm);
}

static const char *subunit2str(uint8_t subunit)
{
	switch (subunit) {
	case AVC_SUBUNIT_MONITOR:
		return "Monitor";
	case AVC_SUBUNIT_AUDIO:
		return "Audio";
	case AVC_SUBUNIT_PRINTER:
		return "Printer";
	case AVC_SUBUNIT_DISC:
		return "Disc";
	case AVC_SUBUNIT_TAPE:
		return "Tape";
	case AVC_SUBUNIT_TURNER:
		return "Turner";
	case AVC_SUBUNIT_CA:
		return "CA";
	case AVC_SUBUNIT_CAMERA:
		return "Camera";
	case AVC_SUBUNIT_PANEL:
		return "Panel";
	case AVC_SUBUNIT_BULLETIN_BOARD:
		return "Bulleting Board";
	case AVC_SUBUNIT_CAMERA_STORAGE:
		return "Camera Storage";
	case AVC_SUBUNIT_VENDOR_UNIQUE:
		return "Vendor Unique";
	case AVC_SUBUNIT_EXTENDED:
		return "Extended to next byte";
	case AVC_SUBUNIT_UNIT:
		return "Unit";
	default:
		return "Reserved";
	}
}

static const char *playertype2str(uint8_t type)
{
	switch (type & 0x0F) {
	case 0x01:
		return "Audio";
	case 0x02:
		return "Video";
	case 0x03:
		return "Audio, Video";
	case 0x04:
		return "Audio Broadcasting";
	case 0x05:
		return "Audio, Audio Broadcasting";
	case 0x06:
		return "Video, Audio Broadcasting";
	case 0x07:
		return "Audio, Video, Audio Broadcasting";
	case 0x08:
		return "Video Broadcasting";
	case 0x09:
		return "Audio, Video Broadcasting";
	case 0x0A:
		return "Video, Video Broadcasting";
	case 0x0B:
		return "Audio, Video, Video Broadcasting";
	case 0x0C:
		return "Audio Broadcasting, Video Broadcasting";
	case 0x0D:
		return "Audio, Audio Broadcasting, Video Broadcasting";
	case 0x0E:
		return "Video, Audio Broadcasting, Video Broadcasting";
	case 0x0F:
		return "Audio, Video, Audio Broadcasting, Video Broadcasting";
	}

	return "None";
}

static const char *playersubtype2str(uint32_t subtype)
{
	switch (subtype & 0x03) {
	case 0x01:
		return "Audio Book";
	case 0x02:
		return "Podcast";
	case 0x03:
		return "Audio Book, Podcast";
	}

	return "None";
}

static void avrcp_media_player_item_dump(int level, struct frame *frm,
								uint16_t len)
{
	uint16_t id, charset, namelen;
	uint8_t type, status;
	uint32_t subtype;
	uint64_t features[2];

	p_indent(level, frm);

	if (len < 28) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	id = p_get_u16(frm);
	printf("PlayerID: 0x%04x (%u)\n", id, id);

	p_indent(level, frm);

	type = p_get_u8(frm);
	printf("PlayerType: 0x%04x (%s)\n", type, playertype2str(type));

	p_indent(level, frm);

	subtype = p_get_u32(frm);
	printf("PlayerSubtype: 0x%08x (%s)\n", subtype,
						playersubtype2str(subtype));

	p_indent(level, frm);

	status = p_get_u8(frm);
	printf("PlayStatus: 0x%02x (%s)\n", status, playstatus2str(status));

	p_indent(level, frm);

	p_get_u128(frm, &features[0], &features[1]);
	printf("Features: 0x%16" PRIx64 "%16" PRIx64 "\n", features[1],
								features[0]);

	p_indent(level, frm);

	charset = p_get_u16(frm);
	printf("CharsetID: 0x%04x (%s)\n", charset, charset2str(charset));

	p_indent(level, frm);

	namelen = p_get_u16(frm);
	printf("NameLength: 0x%04x (%u)\n", namelen, namelen);

	p_indent(level, frm);

	printf("Name: ");
	for (; namelen > 0; namelen--) {
		uint8_t c = p_get_u8(frm);
		printf("%1c", isprint(c) ? c : '.');
	}
	printf("\n");
}

static const char *foldertype2str(uint8_t type)
{
	switch (type) {
	case 0x00:
		return "Mixed";
	case 0x01:
		return "Titles";
	case 0x02:
		return "Albuns";
	case 0x03:
		return "Artists";
	case 0x04:
		return "Genres";
	case 0x05:
		return "Playlists";
	case 0x06:
		return "Years";
	}

	return "Reserved";
}

static void avrcp_folder_item_dump(int level, struct frame *frm, uint16_t len)
{
	uint8_t type, playable;
	uint16_t charset, namelen;
	uint64_t uid;

	p_indent(level, frm);

	if (len < 14) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	uid = p_get_u64(frm);
	printf("FolderUID: 0x%16" PRIx64 " (%" PRIu64 ")\n", uid, uid);

	p_indent(level, frm);

	type = p_get_u8(frm);
	printf("FolderType: 0x%02x (%s)\n", type, foldertype2str(type));

	p_indent(level, frm);

	playable = p_get_u8(frm);
	printf("IsPlayable: 0x%02x (%s)\n", playable,
					playable & 0x01 ? "True" : "False");

	p_indent(level, frm);

	charset = p_get_u16(frm);
	printf("CharsetID: 0x%04x (%s)\n", charset, charset2str(charset));

	p_indent(level, frm);

	namelen = p_get_u16(frm);
	printf("NameLength: 0x%04x (%u)\n", namelen, namelen);

	p_indent(level, frm);

	printf("Name: ");
	for (; namelen > 0; namelen--) {
		uint8_t c = p_get_u8(frm);
		printf("%1c", isprint(c) ? c : '.');
	}
	printf("\n");
}

static const char *elementtype2str(uint8_t type)
{
	switch (type) {
	case 0x00:
		return "Audio";
	case 0x01:
		return "Video";
	}

	return "Reserved";
}

static void avrcp_attribute_entry_list_dump(int level, struct frame *frm,
								uint8_t count)
{
	for (; count > 0; count--) {
		uint32_t attr;
		uint16_t charset;
		uint8_t len;

		p_indent(level, frm);

		attr = p_get_u32(frm);
		printf("AttributeID: 0x%08x (%s)\n", attr, mediattr2str(attr));

		p_indent(level, frm);

		charset = p_get_u16(frm);
		printf("CharsetID: 0x%04x (%s)\n", charset,
							charset2str(charset));

		p_indent(level, frm);

		len = p_get_u16(frm);
		printf("AttributeLength: 0x%04x (%u)\n", len, len);

		p_indent(level, frm);

		printf("AttributeValue: ");
		for (; len > 0; len--) {
			uint8_t c = p_get_u8(frm);
			printf("%1c", isprint(c) ? c : '.');
		}
		printf("\n");
	}
}

static void avrcp_media_element_item_dump(int level, struct frame *frm,
								uint16_t len)
{
	uint64_t uid;
	uint16_t charset, namelen;
	uint8_t type, count;

	p_indent(level, frm);

	if (len < 14) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	uid = p_get_u64(frm);
	printf("ElementUID: 0x%16" PRIx64 " (%" PRIu64 ")\n", uid, uid);

	p_indent(level, frm);

	type = p_get_u8(frm);
	printf("ElementType: 0x%02x (%s)\n", type, elementtype2str(type));

	p_indent(level, frm);

	charset = p_get_u16(frm);
	printf("CharsetID: 0x%04x (%s)\n", charset, charset2str(charset));

	p_indent(level, frm);

	namelen = p_get_u16(frm);
	printf("NameLength: 0x%04x (%u)\n", namelen, namelen);

	p_indent(level, frm);

	printf("Name: ");
	for (; namelen > 0; namelen--) {
		uint8_t c = p_get_u8(frm);
		printf("%1c", isprint(c) ? c : '.');
	}
	printf("\n");

	p_indent(level, frm);

	count = p_get_u8(frm);
	printf("AttributeCount: 0x%02x (%u)\n", count, count);

	avrcp_attribute_entry_list_dump(level, frm, count);
}

static void avrcp_get_folder_items_dump(int level, struct frame *frm,
						uint8_t hdr, uint16_t len)
{
	uint8_t scope, count, status;
	uint32_t start, end;
	uint16_t uid, num;

	p_indent(level, frm);

	if (hdr & 0x02)
		goto response;

	if (len < 10) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	scope = p_get_u8(frm);
	printf("Scope: 0x%02x (%s)\n", scope, scope2str(scope));

	p_indent(level, frm);

	start = p_get_u32(frm);
	printf("StartItem: 0x%08x (%u)\n", start, start);

	p_indent(level, frm);

	end = p_get_u32(frm);
	printf("EndItem: 0x%08x (%u)\n", end, end);

	p_indent(level, frm);

	count = p_get_u8(frm);
	printf("AttributeCount: 0x%02x (%u)\n", count, count);

	for (; count > 0; count--) {
		uint32_t attr;

		p_indent(level, frm);

		attr = p_get_u32(frm);
		printf("AttributeID: 0x%08x (%s)\n", attr, mediattr2str(attr));
	}

	return;

response:
	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));

	if (len == 1)
		return;

	p_indent(level, frm);

	uid = p_get_u16(frm);
	printf("UIDCounter: 0x%04x (%u)\n", uid, uid);

	p_indent(level, frm);

	num = p_get_u16(frm);
	printf("Number of Items: 0x%04x (%u)\n", num, num);

	for (; num > 0; num--) {
		uint8_t type;
		uint16_t len;

		p_indent(level, frm);

		type = p_get_u8(frm);
		len = p_get_u16(frm);
		switch (type) {
		case 0x01:
			printf("Item: 0x01 (Media Player)) ");
			printf("Length: 0x%04x (%u)\n", len, len);
			avrcp_media_player_item_dump(level, frm, len);
			break;
		case 0x02:
			printf("Item: 0x02 (Folder) ");
			printf("Length: 0x%04x (%u)\n", len, len);
			avrcp_folder_item_dump(level, frm, len);
			break;
		case 0x03:
			printf("Item: 0x03 (Media Element) ");
			printf("Length: 0x%04x (%u)\n", len, len);
			avrcp_media_element_item_dump(level, frm, len);
			break;
		}
	}
}

static const char *dir2str(uint8_t dir)
{
	switch (dir) {
	case 0x00:
		return "Folder Up";
	case 0x01:
		return "Folder Down";
	}

	return "Reserved";
}

static void avrcp_change_path_dump(int level, struct frame *frm, uint8_t hdr,
								uint16_t len)
{
	uint64_t uid;
	uint32_t items;
	uint16_t uidcounter;
	uint8_t dir, status;

	p_indent(level, frm);

	if (hdr & 0x02)
		goto response;

	if (len < 11) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	uidcounter = p_get_u16(frm);
	printf("UIDCounter: 0x%04x (%u)\n", uidcounter, uidcounter);

	p_indent(level, frm);

	dir = p_get_u8(frm);
	printf("Direction: 0x%02x (%s)\n", dir, dir2str(dir));

	p_indent(level, frm);

	uid = p_get_u64(frm);
	printf("FolderUID: 0x%16" PRIx64 " (%" PRIu64 ")\n", uid, uid);

	return;

response:
	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));

	if (len == 1)
		return;

	p_indent(level, frm);

	items = p_get_u32(frm);
	printf("Number of Items: 0x%04x (%u)", items, items);
}

static void avrcp_get_item_attributes_dump(int level, struct frame *frm,
						uint8_t hdr, uint16_t len)
{
	uint64_t uid;
	uint32_t uidcounter;
	uint8_t scope, count, status;

	p_indent(level, frm);

	if (hdr & 0x02)
		goto response;

	if (len < 12) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	scope = p_get_u8(frm);
	printf("Scope: 0x%02x (%s)\n", scope, scope2str(scope));

	p_indent(level, frm);

	uid = p_get_u64(frm);
	printf("UID: 0x%16" PRIx64 " (%" PRIu64 ")\n", uid, uid);

	p_indent(level, frm);

	uidcounter = p_get_u16(frm);
	printf("UIDCounter: 0x%04x (%u)\n", uidcounter, uidcounter);

	p_indent(level, frm);

	count = p_get_u8(frm);
	printf("AttributeCount: 0x%02x (%u)\n", count, count);

	for (; count > 0; count--) {
		uint32_t attr;

		p_indent(level, frm);

		attr = p_get_u32(frm);
		printf("AttributeID: 0x%08x (%s)\n", attr, mediattr2str(attr));
	}

	return;

response:
	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));

	if (len == 1)
		return;

	p_indent(level, frm);

	count = p_get_u8(frm);
	printf("AttributeCount: 0x%02x (%u)\n", count, count);

	avrcp_attribute_entry_list_dump(level, frm, count);
}

static void avrcp_search_dump(int level, struct frame *frm, uint8_t hdr,
								uint16_t len)
{
	uint32_t uidcounter, items;
	uint16_t charset, namelen;
	uint8_t status;

	p_indent(level, frm);

	if (hdr & 0x02)
		goto response;

	if (len < 4) {
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	charset = p_get_u16(frm);
	printf("CharsetID: 0x%04x (%s)\n", charset, charset2str(charset));

	p_indent(level, frm);

	namelen = p_get_u16(frm);
	printf("Length: 0x%04x (%u)\n", namelen, namelen);

	p_indent(level, frm);

	printf("String: ");
	for (; namelen > 0; namelen--) {
		uint8_t c = p_get_u8(frm);
		printf("%1c", isprint(c) ? c : '.');
	}
	printf("\n");

	return;

response:
	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));

	if (len == 1)
		return;

	p_indent(level, frm);

	uidcounter = p_get_u16(frm);
	printf("UIDCounter: 0x%04x (%u)\n", uidcounter, uidcounter);

	p_indent(level, frm);

	items = p_get_u32(frm);
	printf("Number of Items: 0x%04x (%u)", items, items);
}

static void avrcp_general_reject_dump(int level, struct frame *frm,
						uint8_t hdr, uint16_t len)
{
	uint8_t status;

	p_indent(level, frm);

	if (hdr & 0x02)
		goto response;

	printf("PDU Malformed\n");
	raw_dump(level, frm);
	return;

response:
	status = p_get_u8(frm);
	printf("Status: 0x%02x (%s)\n", status, error2str(status));
}

static void avrcp_browsing_dump(int level, struct frame *frm, uint8_t hdr)
{
	uint8_t pduid;
	uint16_t len;

	pduid = p_get_u8(frm);
	len = p_get_u16(frm);

	printf("AVRCP: %s: len 0x%04x\n", pdu2str(pduid), len);

	if (len != frm->len) {
		p_indent(level, frm);
		printf("PDU Malformed\n");
		raw_dump(level, frm);
		return;
	}

	switch (pduid) {
	case AVRCP_SET_BROWSED_PLAYER:
		avrcp_set_browsed_player_dump(level + 1, frm, hdr, len);
		break;
	case AVRCP_GET_FOLDER_ITEMS:
		avrcp_get_folder_items_dump(level + 1, frm, hdr, len);
		break;
	case AVRCP_CHANGE_PATH:
		avrcp_change_path_dump(level + 1, frm, hdr, len);
		break;
	case AVRCP_GET_ITEM_ATTRIBUTES:
		avrcp_get_item_attributes_dump(level + 1, frm, hdr, len);
		break;
	case AVRCP_SEARCH:
		avrcp_search_dump(level + 1, frm, hdr, len);
		break;
	case AVRCP_GENERAL_REJECT:
		avrcp_general_reject_dump(level + 1, frm, hdr, len);
		break;
	default:
		raw_dump(level, frm);
	}
}

static void avrcp_control_dump(int level, struct frame *frm)
{
	uint8_t ctype, address, subunit, opcode, company[3];
	int i;

	ctype = p_get_u8(frm);
	address = p_get_u8(frm);
	opcode = p_get_u8(frm);

	printf("AV/C: %s: address 0x%02x opcode 0x%02x\n", ctype2str(ctype),
							address, opcode);

	p_indent(level + 1, frm);

	subunit = address >> 3;
	printf("Subunit: %s\n", subunit2str(subunit));

	p_indent(level + 1, frm);

	printf("Opcode: %s\n", opcode2str(opcode));

	/* Skip non-panel subunit packets */
	if (subunit != AVC_SUBUNIT_PANEL) {
		raw_dump(level, frm);
		return;
	}

	/* Not implemented should not contain any operand */
	if (ctype == AVC_CTYPE_NOT_IMPLEMENTED) {
		raw_dump(level, frm);
		return;
	}

	switch (opcode) {
	case AVC_OP_PASSTHROUGH:
		avrcp_passthrough_dump(level + 1, frm);
		break;
	case AVC_OP_VENDORDEP:
		p_indent(level + 1, frm);

		printf("Company ID: 0x");
		for (i = 0; i < 3; i++) {
			company[i] = p_get_u8(frm);
			printf("%02x", company[i]);
		}
		printf("\n");

		avrcp_pdu_dump(level + 1, frm, ctype);
		break;
	default:
		raw_dump(level, frm);
	}
}

void avrcp_dump(int level, struct frame *frm, uint8_t hdr, uint16_t psm)
{
	p_indent(level, frm);

	switch (psm) {
		case 0x17:
			avrcp_control_dump(level, frm);
			break;
		case 0x1B:
			avrcp_browsing_dump(level, frm, hdr);
			break;
		default:
			raw_dump(level, frm);
	}
}
