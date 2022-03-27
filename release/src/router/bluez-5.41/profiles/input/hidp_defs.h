/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2014  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2014       Google Inc.
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

#ifndef __HIDP_DEFS_H
#define __HIDP_DEFS_H

/* HIDP header masks */
#define HIDP_HEADER_TRANS_MASK			0xf0
#define HIDP_HEADER_PARAM_MASK			0x0f

/* HIDP transaction types */
#define HIDP_TRANS_HANDSHAKE			0x00
#define HIDP_TRANS_HID_CONTROL			0x10
#define HIDP_TRANS_GET_REPORT			0x40
#define HIDP_TRANS_SET_REPORT			0x50
#define HIDP_TRANS_GET_PROTOCOL			0x60
#define HIDP_TRANS_SET_PROTOCOL			0x70
#define HIDP_TRANS_GET_IDLE			0x80
#define HIDP_TRANS_SET_IDLE			0x90
#define HIDP_TRANS_DATA				0xa0
#define HIDP_TRANS_DATC				0xb0

/* HIDP handshake results */
#define HIDP_HSHK_SUCCESSFUL			0x00
#define HIDP_HSHK_NOT_READY			0x01
#define HIDP_HSHK_ERR_INVALID_REPORT_ID		0x02
#define HIDP_HSHK_ERR_UNSUPPORTED_REQUEST	0x03
#define HIDP_HSHK_ERR_INVALID_PARAMETER		0x04
#define HIDP_HSHK_ERR_UNKNOWN			0x0e
#define HIDP_HSHK_ERR_FATAL			0x0f

/* HIDP control operation parameters */
#define HIDP_CTRL_NOP				0x00
#define HIDP_CTRL_HARD_RESET			0x01
#define HIDP_CTRL_SOFT_RESET			0x02
#define HIDP_CTRL_SUSPEND			0x03
#define HIDP_CTRL_EXIT_SUSPEND			0x04
#define HIDP_CTRL_VIRTUAL_CABLE_UNPLUG		0x05

/* HIDP data transaction headers */
#define HIDP_DATA_RTYPE_MASK			0x03
#define HIDP_DATA_RSRVD_MASK			0x0c
#define HIDP_DATA_RTYPE_OTHER			0x00
#define HIDP_DATA_RTYPE_INPUT			0x01
#define HIDP_DATA_RTYPE_OUPUT			0x02
#define HIDP_DATA_RTYPE_FEATURE			0x03

/* HIDP protocol header parameters */
#define HIDP_PROTO_BOOT				0x00
#define HIDP_PROTO_REPORT			0x01

#define HIDP_VIRTUAL_CABLE_UNPLUG		0
#define HIDP_BOOT_PROTOCOL_MODE			1
#define HIDP_BLUETOOTH_VENDOR_ID		9
#define HIDP_WAITING_FOR_RETURN			10
#define HIDP_WAITING_FOR_SEND_ACK		11

#endif /* __HIDP_DEFS_H */
