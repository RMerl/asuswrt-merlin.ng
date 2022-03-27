/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2002  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
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

#ifndef __L2CAP_H
#define __L2CAP_H

/* L2CAP command codes */
#define L2CAP_COMMAND_REJ	0x01
#define L2CAP_CONN_REQ		0x02
#define L2CAP_CONN_RSP		0x03
#define L2CAP_CONF_REQ		0x04
#define L2CAP_CONF_RSP		0x05
#define L2CAP_DISCONN_REQ	0x06
#define L2CAP_DISCONN_RSP	0x07
#define L2CAP_ECHO_REQ		0x08
#define L2CAP_ECHO_RSP		0x09
#define L2CAP_INFO_REQ		0x0a
#define L2CAP_INFO_RSP		0x0b
#define L2CAP_CREATE_REQ	0x0c
#define L2CAP_CREATE_RSP	0x0d
#define L2CAP_MOVE_REQ		0x0e
#define L2CAP_MOVE_RSP		0x0f
#define L2CAP_MOVE_CFM		0x10
#define L2CAP_MOVE_CFM_RSP	0x11

/* L2CAP extended feature mask */
#define L2CAP_FEAT_FLOWCTL	0x00000001
#define L2CAP_FEAT_RETRANS	0x00000002
#define L2CAP_FEAT_BIDIR_QOS	0x00000004
#define L2CAP_FEAT_ERTM		0x00000008
#define L2CAP_FEAT_STREAMING	0x00000010
#define L2CAP_FEAT_FCS		0x00000020
#define L2CAP_FEAT_EXT_FLOW	0x00000040
#define L2CAP_FEAT_FIXED_CHAN	0x00000080
#define L2CAP_FEAT_EXT_WINDOW	0x00000100
#define L2CAP_FEAT_UCD		0x00000200

/* L2CAP Control Field bit masks */
#define L2CAP_CTRL_SAR_MASK		0xC000
#define L2CAP_CTRL_REQSEQ_MASK		0x3F00
#define L2CAP_CTRL_TXSEQ_MASK		0x007E
#define L2CAP_CTRL_SUPERVISE_MASK	0x000C

#define L2CAP_CTRL_RETRANS		0x0080
#define L2CAP_CTRL_FINAL		0x0080
#define L2CAP_CTRL_POLL			0x0010
#define L2CAP_CTRL_FRAME_TYPE		0x0001 /* I- or S-Frame */

#define L2CAP_CTRL_TXSEQ_SHIFT		1
#define L2CAP_CTRL_SUPER_SHIFT		2
#define L2CAP_CTRL_REQSEQ_SHIFT		8
#define L2CAP_CTRL_SAR_SHIFT		14

#define L2CAP_EXT_CTRL_TXSEQ_MASK	0xFFFC0000
#define L2CAP_EXT_CTRL_SAR_MASK		0x00030000
#define L2CAP_EXT_CTRL_SUPERVISE_MASK	0x00030000
#define L2CAP_EXT_CTRL_REQSEQ_MASK	0x0000FFFC

#define L2CAP_EXT_CTRL_POLL		0x00040000
#define L2CAP_EXT_CTRL_FINAL		0x00000002
#define L2CAP_EXT_CTRL_FRAME_TYPE	0x00000001 /* I- or S-Frame */

#define L2CAP_EXT_CTRL_REQSEQ_SHIFT	2
#define L2CAP_EXT_CTRL_SAR_SHIFT	16
#define L2CAP_EXT_CTRL_SUPER_SHIFT	16
#define L2CAP_EXT_CTRL_TXSEQ_SHIFT	18

/* L2CAP Supervisory Function */
#define L2CAP_SUPER_RR		0x00
#define L2CAP_SUPER_REJ		0x01
#define L2CAP_SUPER_RNR		0x02
#define L2CAP_SUPER_SREJ	0x03

/* L2CAP Segmentation and Reassembly */
#define L2CAP_SAR_UNSEGMENTED	0x00
#define L2CAP_SAR_START		0x01
#define L2CAP_SAR_END		0x02
#define L2CAP_SAR_CONTINUE	0x03

#define L2CAP_SDULEN_SIZE	2

/* L2CAP fixed channels */
#define L2CAP_FC_L2CAP		0x02
#define L2CAP_FC_CONNLESS	0x04
#define L2CAP_FC_A2MP		0x08

/* L2CAP structures */
typedef struct {
	uint16_t	len;
	uint16_t	cid;
} __attribute__ ((packed)) l2cap_hdr;
#define L2CAP_HDR_SIZE 4

typedef struct {
	uint8_t		code;
	uint8_t		ident;
	uint16_t	len;
} __attribute__ ((packed)) l2cap_cmd_hdr;
#define L2CAP_CMD_HDR_SIZE 4

typedef struct {
	uint16_t	reason;
} __attribute__ ((packed)) l2cap_cmd_rej;
#define L2CAP_CMD_REJ_SIZE 2

typedef struct {
	uint16_t	psm;
	uint16_t	scid;
} __attribute__ ((packed)) l2cap_conn_req;
#define L2CAP_CONN_REQ_SIZE 4

typedef struct {
	uint16_t	dcid;
	uint16_t	scid;
	uint16_t	result;
	uint16_t	status;
} __attribute__ ((packed)) l2cap_conn_rsp;
#define L2CAP_CONN_RSP_SIZE 8

/* connect result */
#define L2CAP_CR_SUCCESS	0x0000
#define L2CAP_CR_PEND		0x0001
#define L2CAP_CR_BAD_PSM	0x0002
#define L2CAP_CR_SEC_BLOCK	0x0003
#define L2CAP_CR_NO_MEM		0x0004

/* connect status */
#define L2CAP_CS_NO_INFO	0x0000
#define L2CAP_CS_AUTHEN_PEND	0x0001
#define L2CAP_CS_AUTHOR_PEND	0x0002

typedef struct {
	uint16_t	dcid;
	uint16_t	flags;
	uint8_t		data[0];
} __attribute__ ((packed)) l2cap_conf_req;
#define L2CAP_CONF_REQ_SIZE 4

typedef struct {
	uint16_t	scid;
	uint16_t	flags;
	uint16_t	result;
	uint8_t		data[0];
} __attribute__ ((packed)) l2cap_conf_rsp;
#define L2CAP_CONF_RSP_SIZE 6

#define L2CAP_CONF_SUCCESS	0x0000
#define L2CAP_CONF_UNACCEPT	0x0001
#define L2CAP_CONF_REJECT	0x0002
#define L2CAP_CONF_UNKNOWN	0x0003
#define L2CAP_CONF_PENDING	0x0004
#define L2CAP_CONF_EFS_REJECT	0x0005

typedef struct {
	uint8_t		type;
	uint8_t		len;
	uint8_t		val[0];
} __attribute__ ((packed)) l2cap_conf_opt;
#define L2CAP_CONF_OPT_SIZE 2

#define L2CAP_CONF_MTU		0x01
#define L2CAP_CONF_FLUSH_TO	0x02
#define L2CAP_CONF_QOS		0x03
#define L2CAP_CONF_RFC		0x04
#define L2CAP_CONF_FCS		0x05
#define L2CAP_CONF_EFS		0x06
#define L2CAP_CONF_EWS		0x07

#define L2CAP_CONF_MAX_SIZE	22

#define L2CAP_MODE_BASIC	0x00
#define L2CAP_MODE_RETRANS	0x01
#define L2CAP_MODE_FLOWCTL	0x02
#define L2CAP_MODE_ERTM		0x03
#define L2CAP_MODE_STREAMING	0x04

#define L2CAP_SERVTYPE_NOTRAFFIC	0x00
#define L2CAP_SERVTYPE_BESTEFFORT	0x01
#define L2CAP_SERVTYPE_GUARANTEED	0x02

typedef struct {
	uint16_t	dcid;
	uint16_t	scid;
} __attribute__ ((packed)) l2cap_disconn_req;
#define L2CAP_DISCONN_REQ_SIZE 4

typedef struct {
	uint16_t	dcid;
	uint16_t	scid;
} __attribute__ ((packed)) l2cap_disconn_rsp;
#define L2CAP_DISCONN_RSP_SIZE 4

typedef struct {
	uint16_t	type;
} __attribute__ ((packed)) l2cap_info_req;
#define L2CAP_INFO_REQ_SIZE 2

typedef struct {
	uint16_t	type;
	uint16_t	result;
	uint8_t		data[0];
} __attribute__ ((packed)) l2cap_info_rsp;
#define L2CAP_INFO_RSP_SIZE 4

/* info type */
#define L2CAP_IT_CL_MTU		0x0001
#define L2CAP_IT_FEAT_MASK	0x0002

/* info result */
#define L2CAP_IR_SUCCESS	0x0000
#define L2CAP_IR_NOTSUPP	0x0001

typedef struct {
	uint16_t	psm;
	uint16_t	scid;
	uint8_t		id;
} __attribute__ ((packed)) l2cap_create_req;
#define L2CAP_CREATE_REQ_SIZE 5

typedef struct {
	uint16_t	dcid;
	uint16_t	scid;
	uint16_t	result;
	uint16_t	status;
} __attribute__ ((packed)) l2cap_create_rsp;
#define L2CAP_CREATE_RSP_SIZE 8

typedef struct {
	uint16_t	icid;
	uint8_t		id;
} __attribute__ ((packed)) l2cap_move_req;
#define L2CAP_MOVE_REQ_SIZE 3

typedef struct {
	uint16_t	icid;
	uint16_t	result;
} __attribute__ ((packed)) l2cap_move_rsp;
#define L2CAP_MOVE_RSP_SIZE 4

typedef struct {
	uint16_t	icid;
	uint16_t	result;
} __attribute__ ((packed)) l2cap_move_cfm;
#define L2CAP_MOVE_CFM_SIZE 4

typedef struct {
	uint16_t	icid;
} __attribute__ ((packed)) l2cap_move_cfm_rsp;
#define L2CAP_MOVE_CFM_RSP_SIZE 2

#endif /* __L2CAP_H */
