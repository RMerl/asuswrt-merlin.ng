/*
 * Basic types and constants relating to 802.11ah standard.
 * This is a portion of 802.11ah definition. The rest are in 802.11.h.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: 802.11ah.h 776824 2019-07-10 20:45:53Z $
 */

#ifndef _802_11ah_h_
#define _802_11ah_h_

#include <typedefs.h>

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* S1G Action IDs, table 9-493 */
#define S1G_ACTION_TWT_SETUP		6
#define S1G_ACTION_TWT_TEARDOWN		7
#define S1G_ACTION_TWT_INFO		11

/* S1G Action frame offsets */
#define S1G_AF_CAT_OFF			0
#define S1G_AF_ACT_OFF			1

/* TWT Setup */
#define S1G_AF_TWT_SETUP_TOKEN_OFF	2
#define S1G_AF_TWT_SETUP_TWT_IE_OFF	3

/* TWT Teardown */
#define S1G_AF_TWT_TEARDOWN_FLOW_OFF	2

/* TWT Information */
#define S1G_AF_TWT_INFO_OFF		2

/* TWT element - top */
BWL_PRE_PACKED_STRUCT struct twt_ie_top {
	uint8 id;
	uint8 len;
	uint8 ctrl;			/* Control field */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_ie_top twt_ie_top_t;

/* Control field (figure 9-679) 8 bits */
#define TWT_CTRL_NDP_PAGING_IND		(1 << 0)	/* NDP Paging Indication */
#define TWT_CTRL_RESP_PM_MODE		(1 << 1)	/* Respondor PM Mode */
#define TWT_CTRL_NEGOTIATION_MASK	0x0c		/* Negotiation Mask */
#define TWT_CTRL_NEGOTIATION_SHIFT	2		/* Negotiation Shift (bit 2-3) */
#define TWT_CTRL_INFORMATION_DIS	(1 << 4)	/* TWT Information Frame Disabled */

/* Control Negotiation Type subfield (table 9-298a) */
#define TWT_CTRL_NEGO_INDV_REQ_RSP	0		/* Negotiation 0 Individual Req/Rsp */
#define TWT_CTRL_NEGO_INDV_SCHED	1		/* Negotiation 1 Individual Scheduled */
#define TWT_CTRL_NEGO_BCAST_BEACON	2		/* Negotiation 2 Bcast Beacon */
#define TWT_CTRL_NEGO_BCAST_MGMT	3		/* Negotiation 3 Bcast MGMT frames */

/* Individual TWT parameter Set - figure 9-589av2 */
/* The individual TWT parameter set is of dynamic, however the for 11ax supported version is the
 * only supported version. So that is the one being defined here. This means no support for
 * grouping, which also make Targe Wake Time field of len 8, and no NDP paging
 */
#define TWT_INDV_TARGET_WAKE_TIME_SZ	8		/* (Optional) 8 Octets Target Wake Time */

BWL_PRE_PACKED_STRUCT struct twt_ie_indv {
	uint16 request_type;			/* Request Type */
	uint8 twt[TWT_INDV_TARGET_WAKE_TIME_SZ]; /* Target Wake Time */
	uint8 wake_duration;			/* Nominal Minimum TWT Wake Duration */
	uint16 wake_interval_mantissa;		/* TWT Wake Interval Mantissa */
	uint8 channel;				/* TWT Channel */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_ie_indv twt_ie_indv_t;

/* Broadcast TWT parameter Set - figure 9-679b */
BWL_PRE_PACKED_STRUCT struct twt_ie_bcast {
	uint16 request_type;			/* Request Type */
	uint16 twt;				/* Target Wake Time */
	uint8 wake_duration;			/* Nominal Minimum TWT Wake Duration */
	uint16 wake_interval_mantissa;		/* TWT Wake Interval Mantissa */
	uint16 bcast_info;			/* Broadcast TWT Info */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_ie_bcast twt_ie_bcast_t;

/* Request Type field (figure 9-589ax) bit position and field width */
#define TWT_REQ_TYPE_REQUEST_IDX	0	/* TWT Request */
#define TWT_REQ_TYPE_REQUEST_FSZ	1
#define TWT_REQ_TYPE_SETUP_CMD_IDX	1	/* TWT Setup Command */
#define TWT_REQ_TYPE_SETUP_CMD_FSZ	3
#define TWT_REQ_TYPE_TRIGGER_IDX	4	/* Trigger */
#define TWT_REQ_TYPE_TRIGGER_FSZ	1
#define TWT_REQ_TYPE_IMPL_LAST_IDX	5	/* Implicit/Last Broadcast Parameter Set */
#define TWT_REQ_TYPE_IMPL_LAST_FSZ	1
#define TWT_REQ_TYPE_FLOW_TYPE_IDX	6	/* Flow Type (Announced/Unannounced) */
#define TWT_REQ_TYPE_FLOW_TYPE_FSZ	1
#define TWT_REQ_TYPE_FLOW_ID_IDX	7	/* Flow Identifier/Broadcast TWT Recommendation */
#define TWT_REQ_TYPE_FLOW_ID_FSZ	3
#define TWT_REQ_TYPE_WAKE_EXP_IDX	10	/* Wake Interval Exponent */
#define TWT_REQ_TYPE_WAKE_EXP_FSZ	5
#define TWT_REQ_TYPE_PROTECTION_IDX	15	/* Protection */
#define TWT_REQ_TYPE_PROTECTION_FSZ	1

/* Setup Command field (table 9-262k) - Values, store in TWT_REQ_TYPE_SETUP_CMD */
#define TWT_SETUP_CMD_REQUEST_TWT	0	/* Request TWT */
#define TWT_SETUP_CMD_SUGGEST_TWT	1	/* Suggest TWT */
#define TWT_SETUP_CMD_DEMAND_TWT	2	/* Demand TWT */
#define TWT_SETUP_CMD_GRPING_TWT	3	/* Grouping TWT */
#define TWT_SETUP_CMD_ACCEPT_TWT	4	/* Accept TWT */
#define TWT_SETUP_CMD_ALTER_TWT		5	/* Alternate TWT */
#define TWT_SETUP_CMD_DICTATE_TWT	6	/* Dictate TWT */
#define TWT_SETUP_CMD_REJECT_TWT	7	/* Reject TWT */

/* Broadcast TWT Info subfield (figure 9-589ay1) bit position and field width */
#define TWT_BCAST_INFO_PERS_EXP_IDX	0	/* Broadcast TWT Request Persistence Exponent */
#define TWT_BCAST_INFO_PERS_EXP_FSZ	3
#define TWT_BCAST_INFO_ID_IDX		3	/* Broadcast TWT ID */
#define TWT_BCAST_INFO_ID_FSZ		5
#define TWT_BCAST_INFO_PERS_MAN_IDX	8	/* Broadcast TWT Request Persistence Mantissa */
#define TWT_BCAST_INFO_PERS_MAN_FSZ	8

/* TWT Group Assignment Info - 9 octets (long format) or 3 octets (short format) or 0 octet */
/* Group Assignment Info field - short format - Zero Offset Preset field is 0 */
BWL_PRE_PACKED_STRUCT struct twt_grp_short {
	uint8 grpid_n_0off;	/* Group ID and Zero Offset Present */
	uint16 unit_n_off;	/* TWT Unit and TWT Offset */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_grp_short twt_grp_short_t;

/* Group Assignment Info field - long format - Zero Offset Preset field is 1 */
#define TWT_ZERO_OFF_GRP_LEN		6

BWL_PRE_PACKED_STRUCT struct twt_grp_long {
	uint8 grpid_n_0off;			/* Group ID and Zero Offset Present */
	uint8 grp_0off[TWT_ZERO_OFF_GRP_LEN];	/* Zero Offset of Group */
	uint16 unit_n_off;			/* Unit and Offset */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_grp_long twt_grp_long_t;

/* TWT Unit and TWT Offset field */
#define TWT_UNIT_MASK			0x000f		/* TWT Unit */
#define TWT_OFFSET_MASK			0xfff0		/* TWT Offset */
#define TWT_OFFSET_SHIFT		4

/* TWT Unit field (table 9-290) */
#define TWT_UNIT_32us			0
#define TWT_UNIT_256us			1
#define TWT_UNIT_1024us			2
#define TWT_UNIT_8ms192us		3
#define TWT_UNIT_32ms768us		4
#define TWT_UNIT_262ms144us		5
#define TWT_UNIT_1s048576us		6
#define TWT_UNIT_8s388608us		7
#define TWT_UNIT_33s554432us		8
#define TWT_UNIT_268s435456us		9
#define TWT_UNIT_1073s741824us		10
#define TWT_UNIT_8589s934592us		11

/* NDP Paging field - 4 octets or 0 octet */
typedef uint32 twt_ndp_paging_t;

#define TWT_NDP_PAGING_PID		0x000001ff	/* P-ID */
#define TWT_NDP_PAGING_MAX_PERIOD	0x0001fe00	/* Max NDP Paging Period */
#define TWT_NDP_PAGING_PART_TSF_OFF	0x001e0000	/* Partial TSF Offset */
#define TWT_NDP_PAGING_ACTION		0x00e00000	/* Action */
#define TWT_NDP_PAGING_MIN_SLEEP	0x3f000000	/* Min Sleep Duration */

/* Action field (table 9-291) */
#define TWT_ACTION_SEND_PSP_TRIG	0	/* Send a PS-Poll or uplink trigger frame */
#define TWT_ACTION_WAKE_MIN_SLEEP	1	/* Wake up at the time indicated by
						 * Min Sleep Duration
						 */
#define TWT_ACTION_WAKE_RCV_BCN		2	/* Wake up to receive the Beacon */
#define TWT_ACTION_WAKE_RCV_DTIM	3	/* Wake up to receive the DTIM Beacon */
#define TWT_ACTION_WAKE_IND_TIME	4	/* Wakeup at the time indicated by the sum of
						 * the Min Sleep Duration field and the ASD subfield
						 * in the APDI field of the NDP Paging frame
						 */

/* TWT Teardown - TWT Flow field format (figure 9-939(a)) 8 bits */
#define TWT_TEARDOWN_ID_BCAST_MASK	0x1f
#define TWT_TEARDOWN_ID_INDV_MASK	0x7
#define TWT_TEARDOWN_ID_SHIFT		0
#define TWT_TEARDOWN_NEGO_TYPE_MASK	0x60
#define TWT_TEARDOWN_NEGO_TYPE_SHIFT	5
#define TWT_TEARDOWN_ALL_TWT		0x80

/* TWT Information field byte 0 */
#define TWT_INFO_FLOW_ID_MASK		0x07
#define TWT_INFO_RESP_REQ		0x08
#define TWT_INFO_NEXT_TWT_REQ		0x10
#define TWT_INFO_NEXT_TWT_SIZE_MASK	0x60
#define TWT_INFO_NEXT_TWT_SIZE_SHIFT	5
#define TWT_INFO_ALL_TWT		0x80

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* _802_11ah_h_ */
