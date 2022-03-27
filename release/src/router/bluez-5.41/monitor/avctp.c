/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "lib/bluetooth.h"

#include "src/shared/util.h"
#include "bt.h"
#include "packet.h"
#include "display.h"
#include "l2cap.h"
#include "uuid.h"
#include "keys.h"
#include "sdp.h"
#include "avctp.h"

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
#define AVC_SUBUNIT_TUNER		0x05
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
#define AVRCP_GET_TOTAL_NUMBER_OF_ITEMS	0x75
#define AVRCP_SEARCH			0x80
#define AVRCP_ADD_TO_NOW_PLAYING	0x90
#define AVRCP_GENERAL_REJECT		0xA0

/* Packet types */
#define AVRCP_PACKET_TYPE_SINGLE	0x00
#define AVRCP_PACKET_TYPE_START		0x01
#define AVRCP_PACKET_TYPE_CONTINUING	0x02
#define AVRCP_PACKET_TYPE_END		0x03

/* player attributes */
#define AVRCP_ATTRIBUTE_ILEGAL		0x00
#define AVRCP_ATTRIBUTE_EQUALIZER	0x01
#define AVRCP_ATTRIBUTE_REPEAT_MODE	0x02
#define AVRCP_ATTRIBUTE_SHUFFLE		0x03
#define AVRCP_ATTRIBUTE_SCAN		0x04

/* media attributes */
#define AVRCP_MEDIA_ATTRIBUTE_ILLEGAL	0x00
#define AVRCP_MEDIA_ATTRIBUTE_TITLE	0x01
#define AVRCP_MEDIA_ATTRIBUTE_ARTIST	0x02
#define AVRCP_MEDIA_ATTRIBUTE_ALBUM	0x03
#define AVRCP_MEDIA_ATTRIBUTE_TRACK	0x04
#define AVRCP_MEDIA_ATTRIBUTE_TOTAL	0x05
#define AVRCP_MEDIA_ATTRIBUTE_GENRE	0x06
#define AVRCP_MEDIA_ATTRIBUTE_DURATION	0x07

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

/* Media Item Type */
#define AVRCP_MEDIA_PLAYER_ITEM_TYPE	0x01
#define AVRCP_FOLDER_ITEM_TYPE			0x02
#define AVRCP_MEDIA_ELEMENT_ITEM_TYPE	0x03

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

struct avctp_frame {
	uint8_t hdr;
	uint8_t pt;
	uint16_t pid;
	struct l2cap_frame l2cap_frame;
};

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
	case AVC_SUBUNIT_TUNER:
		return "Tuner";
	case AVC_SUBUNIT_CA:
		return "CA";
	case AVC_SUBUNIT_CAMERA:
		return "Camera";
	case AVC_SUBUNIT_PANEL:
		return "Panel";
	case AVC_SUBUNIT_BULLETIN_BOARD:
		return "Bulletin Board";
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
		return "Range Out of Bounds";
	case AVRCP_STATUS_MEDIA_IN_USE:
		return "Media in Use";
	case AVRCP_STATUS_IS_DIRECTORY:
		return "UID is a Directory";
	case AVRCP_STATUS_NOW_PLAYING_LIST_FULL:
		return "Now Playing List Full";
	case AVRCP_STATUS_SEARCH_NOT_SUPPORTED:
		return "Search Not Supported";
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
	case AVRCP_GET_TOTAL_NUMBER_OF_ITEMS:
		return "GetTotalNumOfItems";
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
			return "All Track Shuffle";
		case 0x03:
			return "Group Shuffle";
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

static const char *type2str(uint8_t type)
{
	switch (type) {
	case AVRCP_MEDIA_PLAYER_ITEM_TYPE:
		return "Media Player";
	case AVRCP_FOLDER_ITEM_TYPE:
		return "Folder";
	case AVRCP_MEDIA_ELEMENT_ITEM_TYPE:
		return "Media Element";
	default:
		return "Unknown";
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

static const char *foldertype2str(uint8_t type)
{
	switch (type) {
	case 0x00:
		return "Mixed";
	case 0x01:
		return "Titles";
	case 0x02:
		return "Albums";
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

static bool avrcp_passthrough_packet(struct avctp_frame *avctp_frame,
								uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t op, len;

	if (!l2cap_frame_get_u8(frame, &op))
		return false;

	print_field("%*cOperation: 0x%02x (%s %s)", (indent - 8), ' ', op,
				op2str(op), op & 0x80 ? "Released" : "Pressed");

	if (!l2cap_frame_get_u8(frame, &len))
		return false;

	print_field("%*cLength: 0x%02x", (indent - 8), ' ', len);

	packet_hexdump(frame->data, frame->size);
	return true;
}

static bool avrcp_get_capabilities(struct avctp_frame *avctp_frame,
					uint8_t ctype, uint8_t len,
					uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t cap, count;
	int i;

	if (!l2cap_frame_get_u8(frame, &cap))
		return false;

	print_field("%*cCapabilityID: 0x%02x (%s)", (indent - 8), ' ', cap,
								cap2str(cap));

	if (len == 1)
		return true;

	if (!l2cap_frame_get_u8(frame, &count))
		return false;

	print_field("%*cCapabilityCount: 0x%02x", (indent - 8), ' ', count);

	switch (cap) {
	case 0x2:
		for (; count > 0; count--) {
			uint8_t company[3];

			if (!l2cap_frame_get_u8(frame, &company[0]) ||
				!l2cap_frame_get_u8(frame, &company[1]) ||
				!l2cap_frame_get_u8(frame, &company[2]))
				return false;

			print_field("%*c%s: 0x%02x%02x%02x", (indent - 8), ' ',
					cap2str(cap), company[0], company[1],
					company[2]);
		}
		break;
	case 0x3:
		for (i = 0; count > 0; count--, i++) {
			uint8_t event;

			if (!l2cap_frame_get_u8(frame, &event))
				return false;

			print_field("%*c%s: 0x%02x (%s)", (indent - 8), ' ',
					cap2str(cap), event, event2str(event));
		}
		break;
	default:
		packet_hexdump(frame->data, frame->size);
	}

	return true;
}

static bool avrcp_list_player_attributes(struct avctp_frame *avctp_frame,
						uint8_t ctype, uint8_t len,
						uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t num;
	int i;

	if (len == 0)
		return true;

	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	print_field("%*cAttributeCount: 0x%02x", (indent - 8), ' ', num);

	for (i = 0; num > 0; num--, i++) {
		uint8_t attr;

		if (!l2cap_frame_get_u8(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%02x (%s)", (indent - 8), ' ',
							attr, attr2str(attr));
	}

	return true;
}

static bool avrcp_list_player_values(struct avctp_frame *avctp_frame,
					uint8_t ctype, uint8_t len,
					uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	static uint8_t attr = 0;
	uint8_t num;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (!l2cap_frame_get_u8(frame, &attr))
		return false;

	print_field("%*cAttributeID: 0x%02x (%s)", (indent - 8), ' ',
						attr, attr2str(attr));

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	print_field("%*cValueCount: 0x%02x", (indent - 8), ' ', num);

	for (; num > 0; num--) {
		uint8_t value;

		if (!l2cap_frame_get_u8(frame, &value))
			return false;

		print_field("%*cValueID: 0x%02x (%s)", (indent - 8),
					' ', value, value2str(attr, value));
	}

	return true;
}

static bool avrcp_get_current_player_value(struct avctp_frame *avctp_frame,
						uint8_t ctype, uint8_t len,
						uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t num;

	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	print_field("%*cAttributeCount: 0x%02x", (indent - 8), ' ', num);

	for (; num > 0; num--) {
		uint8_t attr;

		if (!l2cap_frame_get_u8(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%02x (%s)", (indent - 8),
						' ', attr, attr2str(attr));
	}

	return true;

response:
	print_field("%*cValueCount: 0x%02x", (indent - 8), ' ', num);

	for (; num > 0; num--) {
		uint8_t attr, value;

		if (!l2cap_frame_get_u8(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%02x (%s)", (indent - 8),
						' ', attr, attr2str(attr));

		if (!l2cap_frame_get_u8(frame, &value))
			return false;

		print_field("%*cValueID: 0x%02x (%s)", (indent - 8),
					' ', value, value2str(attr, value));
	}

	return true;
}

static bool avrcp_set_player_value(struct avctp_frame *avctp_frame,
					uint8_t ctype, uint8_t len,
					uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t num;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		return true;

	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	print_field("%*cAttributeCount: 0x%02x", (indent - 8), ' ', num);

	for (; num > 0; num--) {
		uint8_t attr, value;

		if (!l2cap_frame_get_u8(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%02x (%s)", (indent - 8), ' ',
							attr, attr2str(attr));

		if (!l2cap_frame_get_u8(frame, &value))
			return false;

		print_field("%*cValueID: 0x%02x (%s)", (indent - 8), ' ',
						value, value2str(attr, value));
	}

	return true;
}

static bool avrcp_get_player_attribute_text(struct avctp_frame *avctp_frame,
						uint8_t ctype, uint8_t len,
						uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t num;

	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	print_field("%*cAttributeCount: 0x%02x", (indent - 8), ' ', num);

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	for (; num > 0; num--) {
		uint8_t attr;

		if (!l2cap_frame_get_u8(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%02x (%s)", (indent - 8),
						' ', attr, attr2str(attr));
	}

	return true;

response:
	for (; num > 0; num--) {
		uint8_t attr, len;
		uint16_t charset;

		if (!l2cap_frame_get_u8(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%02x (%s)", (indent - 8),
						' ', attr, attr2str(attr));

		if (!l2cap_frame_get_be16(frame, &charset))
			return false;

		print_field("%*cCharsetID: 0x%04x (%s)", (indent - 8),
					' ', charset, charset2str(charset));

		if (!l2cap_frame_get_u8(frame, &len))
			return false;

		print_field("%*cStringLength: 0x%02x", (indent - 8), ' ', len);

		printf("String: ");
		for (; len > 0; len--) {
			uint8_t c;

			if (!l2cap_frame_get_u8(frame, &c))
				return false;

			printf("%1c", isprint(c) ? c : '.');
		}
		printf("\n");
	}

	return true;
}

static bool avrcp_get_player_value_text(struct avctp_frame *avctp_frame,
					uint8_t ctype, uint8_t len,
					uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	static uint8_t attr = 0;
	uint8_t num;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (!l2cap_frame_get_u8(frame, &attr))
		return false;

	print_field("%*cAttributeID: 0x%02x (%s)", (indent - 8), ' ',
						attr, attr2str(attr));

	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	print_field("%*cValueCount: 0x%02x", (indent - 8), ' ', num);

	for (; num > 0; num--) {
		uint8_t value;

		if (!l2cap_frame_get_u8(frame, &value))
			return false;

		print_field("%*cValueID: 0x%02x (%s)", (indent - 8),
				' ', value, value2str(attr, value));
	}

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	print_field("%*cValueCount: 0x%02x", (indent - 8), ' ', num);

	for (; num > 0; num--) {
		uint8_t value, len;
		uint16_t charset;

		if (!l2cap_frame_get_u8(frame, &value))
			return false;

		print_field("%*cValueID: 0x%02x (%s)", (indent - 8), ' ',
						value, value2str(attr, value));

		if (!l2cap_frame_get_be16(frame, &charset))
			return false;

		print_field("%*cCharsetIDID: 0x%02x (%s)", (indent - 8), ' ',
						charset, charset2str(charset));

		if (!l2cap_frame_get_u8(frame, &len))
			return false;

		print_field("%*cStringLength: 0x%02x", (indent - 8), ' ', len);

		printf("String: ");
		for (; len > 0; len--) {
			uint8_t c;

			if (!l2cap_frame_get_u8(frame, &c))
				return false;

			printf("%1c", isprint(c) ? c : '.');
		}
		printf("\n");
	}

	return true;
}

static bool avrcp_displayable_charset(struct avctp_frame *avctp_frame,
					uint8_t ctype, uint8_t len,
					uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t num;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		return true;

	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	print_field("%*cCharsetCount: 0x%02x", (indent - 8), ' ', num);

	for (; num > 0; num--) {
		uint16_t charset;

		if (!l2cap_frame_get_be16(frame, &charset))
			return false;

		print_field("%*cCharsetID: 0x%04x (%s)", (indent - 8),
					' ', charset, charset2str(charset));
	}

	return true;
}

static bool avrcp_get_element_attributes(struct avctp_frame *avctp_frame,
						uint8_t ctype, uint8_t len,
						uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint64_t id;
	uint8_t num;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (!l2cap_frame_get_be64(frame, &id))
		return false;

	print_field("%*cIdentifier: 0x%jx (%s)", (indent - 8), ' ',
					id, id ? "Reserved" : "PLAYING");

	if (!l2cap_frame_get_u8(frame, &num))
		return false;

	print_field("%*cAttributeCount: 0x%02x", (indent - 8), ' ', num);

	for (; num > 0; num--) {
		uint32_t attr;

		if (!l2cap_frame_get_le32(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%08x (%s)", (indent - 8),
					' ', attr, mediattr2str(attr));
	}

	return true;

response:
	switch (avctp_frame->pt) {
	case AVRCP_PACKET_TYPE_SINGLE:
	case AVRCP_PACKET_TYPE_START:
		if (!l2cap_frame_get_u8(frame, &num))
			return false;

		avrcp_continuing.num = num;
		print_field("%*cAttributeCount: 0x%02x", (indent - 8),
								' ', num);
		len--;
		break;
	case AVRCP_PACKET_TYPE_CONTINUING:
	case AVRCP_PACKET_TYPE_END:
		num = avrcp_continuing.num;

		if (avrcp_continuing.size > 0) {
			char attrval[UINT8_MAX] = {0};
			uint16_t size;
			uint8_t idx;

			if (avrcp_continuing.size > len) {
				size = len;
				avrcp_continuing.size -= len;
			} else {
				size = avrcp_continuing.size;
				avrcp_continuing.size = 0;
			}

			for (idx = 0; size > 0; idx++, size--) {
				uint8_t c;

				if (!l2cap_frame_get_u8(frame, &c))
					goto failed;

				sprintf(&attrval[idx], "%1c",
							isprint(c) ? c : '.');
			}
			print_field("%*cContinuingAttributeValue: %s",
						(indent - 8), ' ', attrval);

			len -= size;
		}
		break;
	default:
		goto failed;
	}

	while (num > 0 && len > 0) {
		uint32_t attr;
		uint16_t charset, attrlen;
		uint8_t idx;
		char attrval[UINT8_MAX] = {0};

		if (!l2cap_frame_get_be32(frame, &attr))
			goto failed;

		print_field("%*cAttribute: 0x%08x (%s)", (indent - 8),
						' ', attr, mediattr2str(attr));

		if (!l2cap_frame_get_be16(frame, &charset))
			goto failed;

		print_field("%*cCharsetID: 0x%04x (%s)", (indent - 8),
				' ', charset, charset2str(charset));

		if (!l2cap_frame_get_be16(frame, &attrlen))
			goto failed;

		print_field("%*cAttributeValueLength: 0x%04x",
						(indent - 8), ' ', attrlen);

		len -= sizeof(attr) + sizeof(charset) + sizeof(attrlen);
		num--;

		for (idx = 0; attrlen > 0 && len > 0; idx++, attrlen--, len--) {
			uint8_t c;

			if (!l2cap_frame_get_u8(frame, &c))
				goto failed;

			sprintf(&attrval[idx], "%1c", isprint(c) ? c : '.');
		}
		print_field("%*cAttributeValue: %s", (indent - 8),
								' ', attrval);

		if (attrlen > 0)
			avrcp_continuing.size = attrlen;
	}

	avrcp_continuing.num = num;
	return true;

failed:
	avrcp_continuing.num = 0;
	avrcp_continuing.size = 0;
	return false;
}

static bool avrcp_get_play_status(struct avctp_frame *avctp_frame,
					uint8_t ctype, uint8_t len,
					uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint32_t interval;
	uint8_t status;

	if (ctype <= AVC_CTYPE_GENERAL_INQUIRY)
		return true;

	if (!l2cap_frame_get_be32(frame, &interval))
		return false;

	print_field("%*cSongLength: 0x%08x (%u miliseconds)",
					(indent - 8), ' ', interval, interval);

	if (!l2cap_frame_get_be32(frame, &interval))
		return false;

	print_field("%*cSongPosition: 0x%08x (%u miliseconds)",
					(indent - 8), ' ', interval, interval);

	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cPlayStatus: 0x%02x (%s)", (indent - 8),
					' ', status, playstatus2str(status));

	return true;
}

static bool avrcp_register_notification(struct avctp_frame *avctp_frame,
					uint8_t ctype, uint8_t len,
					uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t event, status;
	uint16_t uid;
	uint32_t interval;
	uint64_t id;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (!l2cap_frame_get_u8(frame, &event))
		return false;

	print_field("%*cEventID: 0x%02x (%s)", (indent - 8),
						' ', event, event2str(event));

	if (!l2cap_frame_get_be32(frame, &interval))
		return false;

	print_field("%*cInterval: 0x%08x (%u seconds)",
					(indent - 8), ' ', interval, interval);

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &event))
		return false;

	print_field("%*cEventID: 0x%02x (%s)", (indent - 8),
						' ', event, event2str(event));

	switch (event) {
	case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
		if (!l2cap_frame_get_u8(frame, &status))
			return false;

		print_field("%*cPlayStatus: 0x%02x (%s)", (indent - 8),
					' ', status, playstatus2str(status));
		break;
	case AVRCP_EVENT_TRACK_CHANGED:
		if (!l2cap_frame_get_be64(frame, &id))
			return false;

		print_field("%*cIdentifier: 0x%16" PRIx64 " (%" PRIu64 ")",
						(indent - 8), ' ', id, id);
		break;
	case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		if (!l2cap_frame_get_be32(frame, &interval))
			return false;

		print_field("%*cPosition: 0x%08x (%u miliseconds)",
					(indent - 8), ' ', interval, interval);
		break;
	case AVRCP_EVENT_BATT_STATUS_CHANGED:
		if (!l2cap_frame_get_u8(frame, &status))
			return false;

		print_field("%*cBatteryStatus: 0x%02x (%s)", (indent - 8),
					' ', status, status2str(status));

		break;
	case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
		if (!l2cap_frame_get_u8(frame, &status))
			return false;

		print_field("%*cSystemStatus: 0x%02x ", (indent - 8),
								' ', status);
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
			printf("(UNKNOWN)\n");
			break;
		}
		break;
	case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
		if (!l2cap_frame_get_u8(frame, &status))
			return false;

		print_field("%*cAttributeCount: 0x%02x", (indent - 8),
								' ', status);

		for (; status > 0; status--) {
			uint8_t attr, value;

			if (!l2cap_frame_get_u8(frame, &attr))
				return false;

			print_field("%*cAttributeID: 0x%02x (%s)",
				(indent - 8), ' ', attr, attr2str(attr));

			if (!l2cap_frame_get_u8(frame, &value))
				return false;

			print_field("%*cValueID: 0x%02x (%s)", (indent - 8),
					' ', value, value2str(attr, value));
		}
		break;
	case AVRCP_EVENT_VOLUME_CHANGED:
		if (!l2cap_frame_get_u8(frame, &status))
			return false;

		status &= 0x7F;

		print_field("%*cVolume: %.2f%% (%d/127)", (indent - 8),
						' ', status/1.27, status);
		break;
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		if (!l2cap_frame_get_be16(frame, &uid))
			return false;

		print_field("%*cPlayerID: 0x%04x (%u)", (indent - 8),
								' ', uid, uid);

		if (!l2cap_frame_get_be16(frame, &uid))
			return false;

		print_field("%*cUIDCounter: 0x%04x (%u)", (indent - 8),
								' ', uid, uid);
		break;
	case AVRCP_EVENT_UIDS_CHANGED:
		if (!l2cap_frame_get_be16(frame, &uid))
			return false;

		print_field("%*cUIDCounter: 0x%04x (%u)", (indent - 8),
								' ', uid, uid);
		break;
	}

	return true;
}

static bool avrcp_set_absolute_volume(struct avctp_frame *avctp_frame,
						uint8_t ctype, uint8_t len,
						uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t value;

	if (!l2cap_frame_get_u8(frame, &value))
		return false;

	value &= 0x7F;
	print_field("%*cVolume: %.2f%% (%d/127)", (indent - 8),
						' ', value/1.27, value);

	return true;
}

static bool avrcp_set_addressed_player(struct avctp_frame *avctp_frame,
						uint8_t ctype, uint8_t len,
						uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint16_t id;
	uint8_t status;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (!l2cap_frame_get_be16(frame, &id))
		return false;

	print_field("%*cPlayerID: 0x%04x (%u)", (indent - 8), ' ', id, id);

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", (indent - 8), ' ',
						status, error2str(status));

	return true;
}

static bool avrcp_play_item(struct avctp_frame *avctp_frame, uint8_t ctype,
						uint8_t len, uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint64_t uid;
	uint16_t uidcounter;
	uint8_t scope, status;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (!l2cap_frame_get_u8(frame, &scope))
		return false;

	print_field("%*cScope: 0x%02x (%s)", (indent - 8), ' ',
						scope, scope2str(scope));

	if (!l2cap_frame_get_be64(frame, &uid))
		return false;

	print_field("%*cUID: 0x%16" PRIx64 " (%" PRIu64 ")", (indent - 8),
								' ', uid, uid);

	if (!l2cap_frame_get_be16(frame, &uidcounter))
		return false;

	print_field("%*cUIDCounter: 0x%04x (%u)", (indent - 8), ' ',
							uidcounter, uidcounter);

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", (indent - 8), ' ', status,
							error2str(status));

	return true;
}

static bool avrcp_add_to_now_playing(struct avctp_frame *avctp_frame,
						uint8_t ctype, uint8_t len,
						uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint64_t uid;
	uint16_t uidcounter;
	uint8_t scope, status;

	if (ctype > AVC_CTYPE_GENERAL_INQUIRY)
		goto response;

	if (!l2cap_frame_get_u8(frame, &scope))
		return false;

	print_field("%*cScope: 0x%02x (%s)", (indent - 8), ' ',
						scope, scope2str(scope));

	if (!l2cap_frame_get_be64(frame, &uid))
		return false;

	print_field("%*cUID: 0x%16" PRIx64 " (%" PRIu64 ")", (indent - 8),
								' ', uid, uid);

	if (!l2cap_frame_get_be16(frame, &uidcounter))
		return false;

	print_field("%*cUIDCounter: 0x%04x (%u)", (indent - 8), ' ',
							uidcounter, uidcounter);

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", (indent - 8), ' ', status,
							error2str(status));

	return true;
}

struct avrcp_ctrl_pdu_data {
	uint8_t pduid;
	bool (*func) (struct avctp_frame *avctp_frame, uint8_t ctype,
						uint8_t len, uint8_t indent);
};

static const struct avrcp_ctrl_pdu_data avrcp_ctrl_pdu_table[] = {
	{ 0x10, avrcp_get_capabilities			},
	{ 0x11, avrcp_list_player_attributes		},
	{ 0x12, avrcp_list_player_values		},
	{ 0x13, avrcp_get_current_player_value		},
	{ 0x14, avrcp_set_player_value			},
	{ 0x15, avrcp_get_player_attribute_text		},
	{ 0x16, avrcp_get_player_value_text		},
	{ 0x17, avrcp_displayable_charset		},
	{ 0x20, avrcp_get_element_attributes		},
	{ 0x30, avrcp_get_play_status			},
	{ 0x31, avrcp_register_notification		},
	{ 0x50, avrcp_set_absolute_volume		},
	{ 0x60, avrcp_set_addressed_player		},
	{ 0x74, avrcp_play_item				},
	{ 0x90, avrcp_add_to_now_playing		},
	{ }
};

static bool avrcp_rejected_packet(struct l2cap_frame *frame, uint8_t indent)
{
	uint8_t status;

	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cError: 0x%02x (%s)", (indent - 8), ' ', status,
							error2str(status));

	return true;
}

static bool avrcp_pdu_packet(struct avctp_frame *avctp_frame, uint8_t ctype,
								uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t pduid;
	uint16_t len;
	int i;
	const struct avrcp_ctrl_pdu_data *ctrl_pdu_data = NULL;

	if (!l2cap_frame_get_u8(frame, &pduid))
		return false;

	if (!l2cap_frame_get_u8(frame, &avctp_frame->pt))
		return false;

	if (!l2cap_frame_get_be16(frame, &len))
		return false;

	print_indent(indent, COLOR_OFF, "AVRCP: ", pdu2str(pduid), COLOR_OFF,
			" pt %s len 0x%04x", pt2str(avctp_frame->pt), len);

	if (frame->size != len)
		return false;

	if (ctype == 0xA)
		return avrcp_rejected_packet(frame, indent + 2);

	for (i = 0; avrcp_ctrl_pdu_table[i].func; i++) {
		if (avrcp_ctrl_pdu_table[i].pduid == pduid) {
			ctrl_pdu_data = &avrcp_ctrl_pdu_table[i];
			break;
		}
	}

	if (!ctrl_pdu_data || !ctrl_pdu_data->func) {
		packet_hexdump(frame->data, frame->size);
		return true;
	}

	return ctrl_pdu_data->func(avctp_frame, ctype, len, indent + 2);
}

static bool avrcp_control_packet(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;

	uint8_t ctype, address, subunit, opcode, company[3], indent = 2;

	if (!l2cap_frame_get_u8(frame, &ctype) ||
				!l2cap_frame_get_u8(frame, &address) ||
				!l2cap_frame_get_u8(frame, &opcode))
		return false;

	print_field("AV/C: %s: address 0x%02x opcode 0x%02x",
				ctype2str(ctype), address, opcode);

	subunit = address >> 3;

	print_field("%*cSubunit: %s", indent, ' ', subunit2str(subunit));

	print_field("%*cOpcode: %s", indent, ' ', opcode2str(opcode));

	/* Skip non-panel subunit packets */
	if (subunit != 0x09) {
		packet_hexdump(frame->data, frame->size);
		return true;
	}

	/* Not implemented should not contain any operand */
	if (ctype == 0x8) {
		packet_hexdump(frame->data, frame->size);
		return true;
	}

	switch (opcode) {
	case 0x7c:
		return avrcp_passthrough_packet(avctp_frame, 10);
	case 0x00:
		if (!l2cap_frame_get_u8(frame, &company[0]) ||
				!l2cap_frame_get_u8(frame, &company[1]) ||
				!l2cap_frame_get_u8(frame, &company[2]))
			return false;

		print_field("%*cCompany ID: 0x%02x%02x%02x", indent, ' ',
					company[0], company[1], company[2]);

		return avrcp_pdu_packet(avctp_frame, ctype, 10);
	default:
		packet_hexdump(frame->data, frame->size);
		return true;
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

static bool avrcp_change_path(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint64_t uid;
	uint32_t items;
	uint16_t uidcounter;
	uint8_t dir, status, indent = 2;

	if (avctp_frame->hdr & 0x02)
		goto response;

	if (frame->size < 11) {
		print_field("%*cPDU Malformed", indent, ' ');
		packet_hexdump(frame->data, frame->size);
		return false;
	}

	if (!l2cap_frame_get_be16(frame, &uidcounter))
		return false;

	print_field("%*cUIDCounter: 0x%04x (%u)", indent, ' ',
					uidcounter, uidcounter);

	if (!l2cap_frame_get_u8(frame, &dir))
		return false;

	print_field("%*cDirection: 0x%02x (%s)", indent, ' ',
					dir, dir2str(dir));

	if (!l2cap_frame_get_be64(frame, &uid))
		return false;

	print_field("%*cFolderUID: 0x%16" PRIx64 " (%" PRIu64 ")", indent, ' ',
					uid, uid);

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", indent, ' ',
					status, error2str(status));

	if (frame->size == 1)
		return false;

	if (!l2cap_frame_get_be32(frame, &items))
		return false;

	print_field("%*cNumber of Items: 0x%04x (%u)", indent, ' ',
					items, items);

	return true;
}


static struct {
	const char *str;
	bool reserved;
} features_table[] = {
	/* Ignore passthrough bits */
	[58] = { "Advanced Control Player" },
	[59] = { "Browsing" },
	[60] = { "Searching" },
	[61] = { "AddToNowPlaying" },
	[62] = { "Unique UIDs" },
	[63] = { "OnlyBrowsableWhenAddressed" },
	[64] = { "OnlySearchableWhenAddressed" },
	[65] = { "NowPlaying" },
	[66] = { "UIDPersistency" },
	/* 67-127 reserved */
	[67 ... 127] = { .reserved = true },
};

static void print_features(uint8_t features[16], uint8_t indent)
{
	int i;

	for (i = 0; i < 127; i++) {
		if (!(features[i / 8] & (1 << (i % 8))))
			continue;

		if (features_table[i].reserved) {
			print_text(COLOR_WHITE_BG, "Unknown bit %u", i);
			continue;
		}

		if (!features_table[i].str)
			continue;

		print_field("%*c%s", indent, ' ', features_table[i].str);
	}
}

static bool avrcp_media_player_item(struct avctp_frame *avctp_frame,
							uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint16_t id, charset, namelen;
	uint8_t type, status, i;
	uint32_t subtype;
	uint8_t features[16];

	if (!l2cap_frame_get_be16(frame, &id))
		return false;

	print_field("%*cPlayerID: 0x%04x (%u)", indent, ' ', id, id);

	if (!l2cap_frame_get_u8(frame, &type))
		return false;

	print_field("%*cPlayerType: 0x%04x (%s)", indent, ' ',
						type, playertype2str(type));

	if (!l2cap_frame_get_be32(frame, &subtype))
		return false;

	print_field("%*cPlayerSubType: 0x%08x (%s)", indent, ' ',
					subtype, playersubtype2str(subtype));

	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cPlayStatus: 0x%02x (%s)", indent, ' ',
						status, playstatus2str(status));

	printf("%*cFeatures: 0x", indent+8, ' ');

	for (i = 0; i < 16; i++) {
		if (!l2cap_frame_get_u8(frame, &features[i]))
			return false;

		printf("%02x", features[i]);
	}

	printf("\n");

	print_features(features, indent + 2);

	if (!l2cap_frame_get_be16(frame, &charset))
		return false;

	print_field("%*cCharsetID: 0x%04x (%s)", indent, ' ',
						charset, charset2str(charset));

	if (!l2cap_frame_get_be16(frame, &namelen))
		return false;

	print_field("%*cNameLength: 0x%04x (%u)", indent, ' ',
						namelen, namelen);

	printf("%*cName: ", indent+8, ' ');
	for (; namelen > 0; namelen--) {
		uint8_t c;

		if (!l2cap_frame_get_u8(frame, &c))
			return false;
		printf("%1c", isprint(c) ? c : '.');
	}
	printf("\n");

	return true;
}

static bool avrcp_folder_item(struct avctp_frame *avctp_frame,
							uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t type, playable;
	uint16_t charset, namelen;
	uint64_t uid;

	if (frame->size < 14) {
		printf("PDU Malformed\n");
		return false;
	}

	if (!l2cap_frame_get_be64(frame, &uid))
		return false;

	print_field("%*cFolderUID: 0x%16" PRIx64 " (%" PRIu64 ")", indent, ' ',
						uid, uid);

	if (!l2cap_frame_get_u8(frame, &type))
		return false;

	print_field("%*cFolderType: 0x%02x (%s)", indent, ' ',
						type, foldertype2str(type));

	if (!l2cap_frame_get_u8(frame, &playable))
		return false;

	print_field("%*cIsPlayable: 0x%02x (%s)", indent, ' ', playable,
					playable & 0x01 ? "True" : "False");

	if (!l2cap_frame_get_be16(frame, &charset))
		return false;

	print_field("%*cCharsetID: 0x%04x (%s)", indent, ' ',
					charset, charset2str(charset));

	if (!l2cap_frame_get_be16(frame, &namelen))
		return false;

	print_field("%*cNameLength: 0x%04x (%u)", indent, ' ',
					namelen, namelen);

	print_field("%*cName: ", indent, ' ');
	for (; namelen > 0; namelen--) {
		uint8_t c;
		if (!l2cap_frame_get_u8(frame, &c))
			return false;

		printf("%1c", isprint(c) ? c : '.');
	}

	return true;
}

static bool avrcp_attribute_entry_list(struct avctp_frame *avctp_frame,
						uint8_t indent, uint8_t count)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;

	for (; count > 0; count--) {
		uint32_t attr;
		uint16_t charset;
		uint8_t len;

		if (!l2cap_frame_get_be32(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%08x (%s)", indent, ' ',
						attr, mediattr2str(attr));

		if (!l2cap_frame_get_be16(frame, &charset))
			return false;

		print_field("%*cCharsetID: 0x%04x (%s)", indent, ' ',
						charset, charset2str(charset));

		if (!l2cap_frame_get_u8(frame, &len))
			return false;

		print_field("%*cAttributeLength: 0x%02x (%u)", indent, ' ',
						len, len);

		print_field("%*cAttributeValue: ", indent, ' ');
		for (; len > 0; len--) {
			uint8_t c;

			if (!l2cap_frame_get_u8(frame, &c))
				return false;

			printf("%1c", isprint(c) ? c : '.');
		}
	}

	return true;
}

static bool avrcp_media_element_item(struct avctp_frame *avctp_frame,
							uint8_t indent)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint64_t uid;
	uint16_t charset, namelen;
	uint8_t type, count;

	if (!l2cap_frame_get_be64(frame, &uid))
		return false;

	print_field("%*cElementUID: 0x%16" PRIx64 " (%" PRIu64 ")",
					indent, ' ', uid, uid);

	if (!l2cap_frame_get_u8(frame, &type))
		return false;

	print_field("%*cElementType: 0x%02x (%s)", indent, ' ',
					type, elementtype2str(type));

	if (!l2cap_frame_get_be16(frame, &charset))
		return false;

	print_field("%*cCharsetID: 0x%04x (%s)", indent, ' ',
					charset, charset2str(charset));

	if (!l2cap_frame_get_be16(frame, &namelen))
		return false;

	print_field("%*cNameLength: 0x%04x (%u)", indent, ' ',
					namelen, namelen);

	print_field("%*cName: ", indent, ' ');
	for (; namelen > 0; namelen--) {
		uint8_t c;
		if (!l2cap_frame_get_u8(frame, &c))
			return false;

		printf("%1c", isprint(c) ? c : '.');
	}

	if (!l2cap_frame_get_u8(frame, &count))
		return false;

	print_field("%*cAttributeCount: 0x%02x (%u)", indent, ' ',
						count, count);

	if (!avrcp_attribute_entry_list(avctp_frame, indent, count))
		return false;

	return true;
}

static bool avrcp_general_reject(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t status, indent = 2;

	if (avctp_frame->hdr & 0x02)
		goto response;

	print_field("%*cPDU Malformed", indent, ' ');
	packet_hexdump(frame->data, frame->size);

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", indent, ' ',
				status, error2str(status));

	return true;
}

static bool avrcp_get_total_number_of_items(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint32_t num_of_items;
	uint16_t uidcounter;
	uint8_t scope, status, indent = 2;

	if (avctp_frame->hdr & 0x02)
		goto response;

	if (frame->size < 4) {
		printf("PDU Malformed\n");
		packet_hexdump(frame->data, frame->size);
		return false;
	}

	if (!l2cap_frame_get_u8(frame, &scope))
		return false;

	print_field("%*cScope: 0x%02x (%s)", (indent - 8), ' ',
						scope, scope2str(scope));

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", indent, ' ',
				status, error2str(status));

	if (frame->size == 1)
		return false;

	if (!l2cap_frame_get_be16(frame, &uidcounter))
		return false;

	print_field("%*cUIDCounter: 0x%04x (%u)", indent, ' ',
				uidcounter, uidcounter);

	if (!l2cap_frame_get_be32(frame, &num_of_items))
		return false;

	print_field("%*cNumber of Items: 0x%04x (%u)", indent, ' ',
				num_of_items, num_of_items);

	return true;
}

static bool avrcp_search_item(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint32_t items;
	uint16_t charset, namelen, uidcounter;
	uint8_t status, indent = 2;

	if (avctp_frame->hdr & 0x02)
		goto response;

	if (frame->size < 4) {
		printf("PDU Malformed\n");
		packet_hexdump(frame->data, frame->size);
		return false;
	}

	if (!l2cap_frame_get_be16(frame, &charset))
		return false;

	print_field("%*cCharsetID: 0x%04x (%s)", indent, ' ',
				charset, charset2str(charset));

	if (!l2cap_frame_get_be16(frame, &namelen))
		return false;

	print_field("%*cLength: 0x%04x (%u)", indent, ' ', namelen, namelen);

	printf("%*cString: ", indent+8, ' ');
	for (; namelen > 0; namelen--) {
		uint8_t c;

		if (!l2cap_frame_get_u8(frame, &c))
			return false;

		printf("%1c", isprint(c) ? c : '.');
	}

	printf("\n");

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", indent, ' ',
				status, error2str(status));

	if (frame->size == 1)
		return false;

	if (!l2cap_frame_get_be16(frame, &uidcounter))
		return false;

	print_field("%*cUIDCounter: 0x%04x (%u)", indent, ' ',
				uidcounter, uidcounter);

	if (!l2cap_frame_get_be32(frame, &items))
		return false;

	print_field("%*cNumber of Items: 0x%04x (%u)", indent, ' ',
				items, items);

	return true;
}

static bool avrcp_get_item_attributes(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint64_t uid;
	uint16_t uidcounter;
	uint8_t scope, count, status, indent = 2;

	if (avctp_frame->hdr & 0x02)
		goto response;

	if (frame->size < 12) {
		print_field("%*cPDU Malformed", indent, ' ');
		packet_hexdump(frame->data, frame->size);
		return false;
	}

	if (!l2cap_frame_get_u8(frame, &scope))
		return false;

	print_field("%*cScope: 0x%02x (%s)", indent, ' ',
					scope, scope2str(scope));

	if (!l2cap_frame_get_be64(frame, &uid))
		return false;

	print_field("%*cUID: 0x%016" PRIx64 " (%" PRIu64 ")", indent,
					' ', uid, uid);

	if (!l2cap_frame_get_be16(frame, &uidcounter))
		return false;

	print_field("%*cUIDCounter: 0x%04x (%u)", indent, ' ',
					uidcounter, uidcounter);

	if (!l2cap_frame_get_u8(frame, &count))
		return false;

	print_field("%*cAttributeCount: 0x%02x (%u)", indent, ' ',
					count, count);

	for (; count > 0; count--) {
		uint32_t attr;

		if (!l2cap_frame_get_be32(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%08x (%s)", indent, ' ',
					attr, mediattr2str(attr));
	}

	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", indent, ' ',
					status, error2str(status));

	if (frame->size == 1)
		return false;

	if (!l2cap_frame_get_u8(frame, &count))
		return false;

	print_field("%*cAttributeCount: 0x%02x (%u)", indent, ' ',
					count, count);

	if (!avrcp_attribute_entry_list(avctp_frame, indent, count))
		return false;

	return true;
}

static bool avrcp_get_folder_items(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint8_t scope, count, status, indent = 2;
	uint32_t start, end;
	uint16_t uid, num;

	if (avctp_frame->hdr & 0x02)
		goto response;

	if (!l2cap_frame_get_u8(frame, &scope))
		return false;

	print_field("%*cScope: 0x%02x (%s)", indent, ' ',
					scope, scope2str(scope));

	if (!l2cap_frame_get_be32(frame, &start))
		return false;

	print_field("%*cStartItem: 0x%08x (%u)", indent, ' ', start, start);

	if (!l2cap_frame_get_be32(frame, &end))
		return false;

	print_field("%*cEndItem: 0x%08x (%u)", indent, ' ', end, end);

	if (!l2cap_frame_get_u8(frame, &count))
		return false;

	print_field("%*cAttributeCount: 0x%02x (%u)", indent, ' ',
						count, count);

	for (; count > 0; count--) {
		uint16_t attr;

		if (!l2cap_frame_get_be16(frame, &attr))
			return false;

		print_field("%*cAttributeID: 0x%08x (%s)", indent, ' ',
					attr, mediattr2str(attr));
	}

	return false;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", indent, ' ',
				status, error2str(status));

	if (!l2cap_frame_get_be16(frame, &uid))
		return false;

	print_field("%*cUIDCounter: 0x%04x (%u)", indent, ' ', uid, uid);

	if (!l2cap_frame_get_be16(frame, &num))
		return false;

	print_field("%*cNumOfItems: 0x%04x (%u)", indent, ' ', num, num);

	for (; num > 0; num--) {
		uint8_t type;
		uint16_t len;

		if (!l2cap_frame_get_u8(frame, &type))
			return false;

		if (!l2cap_frame_get_be16(frame, &len))
			return false;

		print_field("%*cItem: 0x%02x (%s) ", indent, ' ',
					type, type2str(type));
		print_field("%*cLength: 0x%04x (%u)", indent, ' ', len, len);

		switch (type) {
		case AVRCP_MEDIA_PLAYER_ITEM_TYPE:
			avrcp_media_player_item(avctp_frame, indent);
			break;
		case AVRCP_FOLDER_ITEM_TYPE:
			avrcp_folder_item(avctp_frame, indent);
			break;
		case AVRCP_MEDIA_ELEMENT_ITEM_TYPE:
			avrcp_media_element_item(avctp_frame, indent);
			break;
		default:
			print_field("%*cUnknown Media Item type", indent, ' ');
			packet_hexdump(frame->data, frame->size);
			break;
		}
	}
	return true;
}

static bool avrcp_set_browsed_player(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint32_t items;
	uint16_t id, uids, charset;
	uint8_t status, folders, indent = 2;

	if (avctp_frame->hdr & 0x02)
		goto response;

	if (!l2cap_frame_get_be16(frame, &id))
		return false;

	print_field("%*cPlayerID: 0x%04x (%u)", indent, ' ', id, id);
	return true;

response:
	if (!l2cap_frame_get_u8(frame, &status))
		return false;

	print_field("%*cStatus: 0x%02x (%s)", indent, ' ', status,
							error2str(status));

	if (!l2cap_frame_get_be16(frame, &uids))
		return false;

	print_field("%*cUIDCounter: 0x%04x (%u)", indent, ' ', uids, uids);

	if (!l2cap_frame_get_be32(frame, &items))
		return false;

	print_field("%*cNumber of Items: 0x%08x (%u)", indent, ' ',
								items, items);

	if (!l2cap_frame_get_be16(frame, &charset))
		return false;

	print_field("%*cCharsetID: 0x%04x (%s)", indent, ' ', charset,
							charset2str(charset));

	if (!l2cap_frame_get_u8(frame, &folders))
		return false;

	print_field("%*cFolder Depth: 0x%02x (%u)", indent, ' ', folders,
								folders);

	for (; folders > 0; folders--) {
		uint8_t len;

		if (!l2cap_frame_get_u8(frame, &len))
			return false;

		if (!len) {
			print_field("%*cFolder: <empty>", indent, ' ');
			continue;
		}

		printf("%*cFolder: ", indent+8, ' ');
		for (; len > 0; len--) {
			uint8_t c;

			if (!l2cap_frame_get_u8(frame, &c))
				return false;

			printf("%1c", isprint(c) ? c : '.');
		}
		printf("\n");
	}

	return true;
}

static bool avrcp_browsing_packet(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	uint16_t len;
	uint8_t pduid;

	if (!l2cap_frame_get_u8(frame, &pduid))
		return false;

	if (!l2cap_frame_get_be16(frame, &len))
		return false;

	print_field("AVRCP: %s: len 0x%04x", pdu2str(pduid), len);

	switch (pduid) {
	case AVRCP_SET_BROWSED_PLAYER:
		avrcp_set_browsed_player(avctp_frame);
		break;
	case AVRCP_GET_FOLDER_ITEMS:
		avrcp_get_folder_items(avctp_frame);
		break;
	case AVRCP_CHANGE_PATH:
		avrcp_change_path(avctp_frame);
		break;
	case AVRCP_GET_ITEM_ATTRIBUTES:
		avrcp_get_item_attributes(avctp_frame);
		break;
	case AVRCP_GET_TOTAL_NUMBER_OF_ITEMS:
		avrcp_get_total_number_of_items(avctp_frame);
		break;
	case AVRCP_SEARCH:
		avrcp_search_item(avctp_frame);
		break;
	case AVRCP_GENERAL_REJECT:
		avrcp_general_reject(avctp_frame);
		break;
	default:
		packet_hexdump(frame->data, frame->size);
	}

	return true;
}

static void avrcp_packet(struct avctp_frame *avctp_frame)
{
	struct l2cap_frame *frame = &avctp_frame->l2cap_frame;
	bool ret;

	switch (frame->psm) {
	case 0x17:
		ret = avrcp_control_packet(avctp_frame);
		break;
	case 0x1B:
		ret = avrcp_browsing_packet(avctp_frame);
		break;
	default:
		packet_hexdump(frame->data, frame->size);
		return;
	}

	if (!ret) {
		print_text(COLOR_ERROR, "PDU malformed");
		packet_hexdump(frame->data, frame->size);
	}
}

void avctp_packet(const struct l2cap_frame *frame)
{
	struct l2cap_frame *l2cap_frame;
	struct avctp_frame avctp_frame;
	const char *pdu_color;

	l2cap_frame_pull(&avctp_frame.l2cap_frame, frame, 0);

	l2cap_frame = &avctp_frame.l2cap_frame;

	if (!l2cap_frame_get_u8(l2cap_frame, &avctp_frame.hdr) ||
			!l2cap_frame_get_be16(l2cap_frame, &avctp_frame.pid)) {
		print_text(COLOR_ERROR, "frame too short");
		packet_hexdump(frame->data, frame->size);
		return;
	}

	if (frame->in)
		pdu_color = COLOR_MAGENTA;
	else
		pdu_color = COLOR_BLUE;

	print_indent(6, pdu_color, "AVCTP", "", COLOR_OFF,
				" %s: %s: type 0x%02x label %d PID 0x%04x",
				frame->psm == 23 ? "Control" : "Browsing",
				avctp_frame.hdr & 0x02 ? "Response" : "Command",
				avctp_frame.hdr & 0x0c, avctp_frame.hdr >> 4,
				avctp_frame.pid);

	if (avctp_frame.pid == 0x110e || avctp_frame.pid == 0x110c)
		avrcp_packet(&avctp_frame);
	else
		packet_hexdump(frame->data, frame->size);
}
