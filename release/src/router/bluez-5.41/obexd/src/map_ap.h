/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2010-2011  Nokia Corporation
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

/* List of OBEX application parameters tags as per MAP specification. */
enum map_ap_tag {
	MAP_AP_MAXLISTCOUNT		= 0x01,		/* uint16_t	*/
	MAP_AP_STARTOFFSET		= 0x02,		/* uint16_t	*/
	MAP_AP_FILTERMESSAGETYPE	= 0x03,		/* uint8_t	*/
	MAP_AP_FILTERPERIODBEGIN	= 0x04,		/* char *	*/
	MAP_AP_FILTERPERIODEND		= 0x05,		/* char *	*/
	MAP_AP_FILTERREADSTATUS		= 0x06,		/* uint8_t	*/
	MAP_AP_FILTERRECIPIENT		= 0x07,		/* char *	*/
	MAP_AP_FILTERORIGINATOR		= 0x08,		/* char *	*/
	MAP_AP_FILTERPRIORITY		= 0x09,		/* uint8_t	*/
	MAP_AP_ATTACHMENT		= 0x0A,		/* uint8_t	*/
	MAP_AP_TRANSPARENT		= 0x0B,		/* uint8_t	*/
	MAP_AP_RETRY			= 0x0C,		/* uint8_t	*/
	MAP_AP_NEWMESSAGE		= 0x0D,		/* uint8_t	*/
	MAP_AP_NOTIFICATIONSTATUS	= 0x0E,		/* uint8_t	*/
	MAP_AP_MASINSTANCEID		= 0x0F,		/* uint8_t	*/
	MAP_AP_PARAMETERMASK		= 0x10,		/* uint32_t	*/
	MAP_AP_FOLDERLISTINGSIZE	= 0x11,		/* uint16_t	*/
	MAP_AP_MESSAGESLISTINGSIZE	= 0x12,		/* uint16_t	*/
	MAP_AP_SUBJECTLENGTH		= 0x13,		/* uint8_t	*/
	MAP_AP_CHARSET			= 0x14,		/* uint8_t	*/
	MAP_AP_FRACTIONREQUEST		= 0x15,		/* uint8_t	*/
	MAP_AP_FRACTIONDELIVER		= 0x16,		/* uint8_t	*/
	MAP_AP_STATUSINDICATOR		= 0x17,		/* uint8_t	*/
	MAP_AP_STATUSVALUE		= 0x18,		/* uint8_t	*/
	MAP_AP_MSETIME			= 0x19,		/* char *	*/
};
