/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

/* Proxy PDU Types */
#define PROXY_NETWORK_PDU	0x00
#define PROXY_MESH_BEACON	0x01
#define PROXY_CONFIG_PDU	0x02
#define PROXY_PROVISIONING_PDU	0x03

#define CTL		0x80
#define TTL_MASK	0x7f
#define SEQ_MASK	0xffffff

#define CREDFLAG_MASK	0x1000
#define APP_IDX_MASK	0x0fff
#define APP_IDX_DEV	0x7fff
#define APP_IDX_ANY	0x8000
#define APP_IDX_NET	0xffff
#define APP_IDX_INVALID	0xffff

#define NET_IDX_INVALID	0xffff
#define NET_IDX_PRIMARY	0x0000

#define KEY_CACHE_SIZE 64
#define FRND_CACHE_MAX 32

#define UNASSIGNED_ADDRESS	0x0000
#define PROXIES_ADDRESS		0xfffc
#define FRIENDS_ADDRESS		0xfffd
#define RELAYS_ADDRESS		0xfffe
#define ALL_NODES_ADDRESS	0xffff
#define VIRTUAL_ADDRESS_LOW	0x8000
#define VIRTUAL_ADDRESS_HIGH	0xbfff
#define GROUP_ADDRESS_LOW	0xc000
#define GROUP_ADDRESS_HIGH	0xff00

#define DEFAULT_TTL		0xff

#define PRIMARY_ELEMENT_IDX	0x00

#define MAX_UNSEG_LEN	15 /* msg_len == 11 + sizeof(MIC) */
#define MAX_SEG_LEN	12 /* UnSeg length - 3 octets overhead */
#define SEG_MAX(len)	(((len) <= MAX_UNSEG_LEN) ? 0 : \
		(((len) - 1) / MAX_SEG_LEN))
#define SEG_OFF(seg)	((seg) * MAX_SEG_LEN)
#define MAX_SEG_TO_LEN(seg)	((seg) ? SEG_OFF((seg) + 1) : MAX_UNSEG_LEN)


#define IS_UNASSIGNED(x)	((x) == UNASSIGNED_ADDRESS)
#define IS_UNICAST(x)		(((x) > UNASSIGNED_ADDRESS) && \
					((x) < VIRTUAL_ADDRESS_LOW))
#define IS_VIRTUAL(x)		(((x) >= VIRTUAL_ADDRESS_LOW) && \
					((x) <= VIRTUAL_ADDRESS_HIGH))
#define IS_GROUP(x)		(((x) >= GROUP_ADDRESS_LOW) && \
					((x) <= GROUP_ADDRESS_HIGH))
#define IS_ALL_NODES(x)		((x) == ALL_NODES_ADDRESS)

#define SEGMENTED	0x80
#define UNSEGMENTED	0x00
#define SEG_HDR_SHIFT	31
#define IS_SEGMENTED(hdr)	(!!((hdr) & (true << SEG_HDR_SHIFT)))

#define KEY_ID_MASK	0x7f
#define KEY_AID_MASK	0x3f
#define KEY_ID_AKF	0x40
#define KEY_AID_SHIFT	0
#define AKF_HDR_SHIFT	30
#define KEY_HDR_SHIFT	24
#define HAS_APP_KEY(hdr)	(!!((hdr) & (true << AKF_HDR_SHIFT)))

#define OPCODE_MASK	0x7f
#define OPCODE_HDR_SHIFT	24
#define RELAY		0x80
#define RELAY_HDR_SHIFT	23
#define SZMIC		0x80
#define SZMIC_HDR_SHIFT	23
#define SEQ_ZERO_MASK	0x1fff
#define SEQ_ZERO_HDR_SHIFT	10
#define IS_RELAYED(hdr)	(!!((hdr) & (true << RELAY_HDR_SHIFT)))
#define HAS_MIC64(hdr)	(!!((hdr) & (true << SZMIC_HDR_SHIFT)))

#define SEG_MASK	0x1f
#define SEGO_HDR_SHIFT	5
#define SEGN_HDR_SHIFT	0
#define SEG_TOTAL(hdr)	(((hdr) >> SEGN_HDR_SHIFT) & SEG_MASK)
/* Proxy Configuration Opcodes */
#define PROXY_OP_SET_FILTER_TYPE	0x00
#define PROXY_OP_FILTER_ADD		0x01
#define PROXY_OP_FILTER_DEL		0x02
#define PROXY_OP_FILTER_STATUS		0x03

/* Proxy Filter Defines */
#define PROXY_FILTER_WHITELIST		0x00
#define PROXY_FILTER_BLACKLIST		0x01

/* Network Tranport Opcodes */
#define NET_OP_SEG_ACKNOWLEDGE		0x00
#define NET_OP_FRND_POLL		0x01
#define NET_OP_FRND_UPDATE		0x02
#define NET_OP_FRND_REQUEST		0x03
#define NET_OP_FRND_OFFER		0x04
#define NET_OP_FRND_CLEAR		0x05
#define NET_OP_FRND_CLEAR_CONFIRM	0x06

#define NET_OP_PROXY_SUB_ADD		0x07
#define NET_OP_PROXY_SUB_REMOVE		0x08
#define NET_OP_PROXY_SUB_CONFIRM	0x09
#define NET_OP_HEARTBEAT		0x0a

/* Key refresh state on the mesh */
#define NET_KEY_REFRESH_PHASE_NONE	0x00
#define NET_KEY_REFRESH_PHASE_ONE	0x01
#define NET_KEY_REFRESH_PHASE_TWO	0x02
#define NET_KEY_REFRESH_PHASE_THREE	0x03

#define MESH_FEATURE_RELAY	1
#define MESH_FEATURE_PROXY	2
#define MESH_FEATURE_FRIEND	4
#define MESH_FEATURE_LPN	8

#define MESH_MAX_ACCESS_PAYLOAD		380

#define MESH_STATUS_SUCCESS		0x00
#define MESH_STATUS_INVALID_ADDRESS	0x01
#define MESH_STATUS_INVALID_MODEL	0x02
#define MESH_STATUS_INVALID_APPKEY	0x03
#define MESH_STATUS_INVALID_NETKEY	0x04
#define MESH_STATUS_INSUFF_RESOURCES	0x05
#define MESH_STATUS_IDX_ALREADY_STORED	0x06
#define MESH_STATUS_INVALID_PUB_PARAM	0x07
#define MESH_STATUS_NOT_SUB_MOD		0x08
#define MESH_STATUS_STORAGE_FAIL	0x09
#define MESH_STATUS_FEAT_NOT_SUP	0x0a
#define MESH_STATUS_CANNOT_UPDATE	0x0b
#define MESH_STATUS_CANNOT_REMOVE	0x0c
#define MESH_STATUS_CANNOT_BIND		0x0d
#define MESH_STATUS_UNABLE_CHANGE_STATE	0x0e
#define MESH_STATUS_CANNOT_SET		0x0f
#define MESH_STATUS_UNSPECIFIED_ERROR	0x10
#define MESH_STATUS_INVALID_BINDING	0x11
