/*
 * Basic types and constants relating to 802.11ah standard.
 * This is a portion of 802.11ah definition. The rest are in 802.11.h.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: 802.11ah.h 627195 2016-03-24 00:57:47Z $
 */

#ifndef _802_11ah_h_
#define _802_11ah_h_

#include <typedefs.h>

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/**
 * TWT IE (sec 8.4.2.196)
 */

/* TWT element - top */
BWL_PRE_PACKED_STRUCT struct twt_ie_top {
	uint8 id;
	uint8 len;
	uint8 ctrl;		/* Control */
	uint16 req_type;	/* Request Type */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_ie_top twt_ie_top_t;

/* Control field (figure 8-557ax) */
#define TWT_CTRL_NDP_PAGING_IND	0x01	/* NDP Paging Indication */
#define TWT_CTRL_RESP_PM_MODE	0x02	/* Respondor PM Mode */
#define TWT_CTRL_BCAST		0x04	/* Broadcast */

/* Requestor Type field (figure 8-557ay) */
#define TWT_REQ_TYPE_REQUEST		0x0001	/* Request */
#define TWT_REQ_TYPE_SETUP_CMD_MASK	0x000e	/* Setup Command */
#define TWT_REQ_TYPE_SETUP_CMD_SHIFT	1
#define TWT_REQ_TYPE_TRIGGER		0x0010	/* Trigger */
#define TWT_REQ_TYPE_IMPLICIT		0x0020	/* Implicit */
#define TWT_REQ_TYPE_FLOW_TYPE		0x0040	/* Flow Type */
#define TWT_REQ_TYPE_FLOW_ID_MASK	0x0380	/* Flow Identifier */
#define TWT_REQ_TYPE_FLOW_ID_SHIFT	7
#define TWT_REQ_TYPE_WAKE_EXP_MASK	0x7c00	/* Wake Interval Exponent */
#define TWT_REQ_TYPE_WAKE_EXP_SHIFT	10
#define TWT_REQ_TYPE_PROTECTION		0x8000	/* Protection */

/* Setup Command field (table 8-248I) */
#define TWT_SETUP_CMD_REQUEST_TWT	0	/* Request TWT */
#define TWT_SETUP_CMD_SUGGEST_TWT	1	/* Suggest TWT */
#define TWT_SETUP_CMD_DEMAND_TWT	2	/* Demand TWT */
#define TWT_SETUP_CMD_GRPING_TWT	3	/* Grouping TWT */
#define TWT_SETUP_CMD_ACCEPT_TWT	4	/* Accept TWT */
#define TWT_SETUP_CMD_ALTER_TWT		5	/* Alternate TWT */
#define TWT_SETUP_CMD_DICTATE_TWT	6	/* Dictate TWT */
#define TWT_SETUP_CMD_REJECT_TWT	7	/* Reject TWT */

/* Flow Identifier field (table 8-248I1) */
#define TWT_BCAST_FLOW_ID_NO_CONSTRAINS	0	/* No constrains on the frame transmitted during
						 * a broadcast TWT SP.
						 */
#define TWT_BCAST_FLOW_ID_NO_RAND_RU	1	/* Do not contain RUs for random access */
#define TWT_BCAST_FLOW_ID_RAND_RU	2	/* Contain RUs for random access */

/* Target Wake Time - 8 octets or 0 octet */
typedef uint64 twt_target_wake_time_t;	/* 64 bit TSF time of TWT Responding STA */

/* TWT Group Assignment Info - 9 octets (long format) or 3 octets (short format) or 0 octet */
/* Group Assignment Info field - short format - Zero Offset Preset field is 0 */
BWL_PRE_PACKED_STRUCT struct twt_grp_short {
	uint8 grpid_n_0off;	/* Group ID and Zero Offset Present */
	uint16 unit_n_off;	/* TWT Unit and TWT Offset */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_grp_short twt_grp_short_t;

/* Group Assignment Info field - long format - Zero Offset Preset field is 1 */
#define TWT_ZERO_OFF_GRP_LEN 6
BWL_PRE_PACKED_STRUCT struct twt_grp_long {
	uint8 grpid_n_0off;	/* Group ID and Zero Offset Present */
	uint8 grp_0off[TWT_ZERO_OFF_GRP_LEN];	/* Zero Offset of Group */
	uint16 unit_n_off;	/* Unit and Offset */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_grp_long twt_grp_long_t;

/* TWT Unit and TWT Offset field */
#define TWT_UNIT_MASK		0x000f		/* TWT Unit */
#define TWT_OFFSET_MASK		0xfff0		/* TWT Offset */
#define TWT_OFFSET_SHIFT	4

/* TWT Unit field (table 8-248m) */
#define TWT_UNIT_32us		0
#define TWT_UNIT_256us		1
#define TWT_UNIT_1024us		2
#define TWT_UNIT_8ms192us	3
#define TWT_UNIT_32ms768us	4
#define TWT_UNIT_262ms144us	5
#define TWT_UNIT_1s048576us	6
#define TWT_UNIT_8s388608us	7
#define TWT_UNIT_33s554432us	8
#define TWT_UNIT_268s435456us	9
#define TWT_UNIT_1073s741824us	10
#define TWT_UNIT_8589s934592us	11

/* TWT element - bottom */
BWL_PRE_PACKED_STRUCT struct twt_ie_bottom {
	uint8 nom_wake_dur;		/* Nominal Minimum Wake Duration */
	uint16 wake_int_mant;		/* TWT Wake Interval Mantissa */
	uint8 channel;			/* TWT Channel */
	/* NDP Paging field */
} BWL_POST_PACKED_STRUCT;

typedef struct twt_ie_bottom twt_ie_bottom_t;

/* Nominal Minimum Wake Duration */
#define TWT_NOM_WAKE_DUR_UNIT	256	/* Nominal Minimum Wake Duration is in 256us units */

/* NDP Paging field - 4 octets or 0 octet */
typedef uint32 twt_ndp_paging_t;

#define TWT_NDP_PAGING_PID		0x000001ff	/* P-ID */
#define TWT_NDP_PAGING_MAX_PERIOD	0x0001fe00	/* Max NDP Paging Period */
#define TWT_NDP_PAGING_PART_TSF_OFF	0x001e0000	/* Partial TSF Offset */
#define TWT_NDP_PAGING_ACTION		0x00e00000	/* Action */
#define TWT_NDP_PAGING_MIN_SLEEP	0x3f000000	/* Min Sleep Duration */

/* Action field (table 8-248n) */
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

/* TWT Teardown Flow field */
#define TWT_TEARDOWN_FLOW_ID_MASK	0x07

/* TWT Information field byte 0 */
#define TWT_INFO_FLOW_ID_MASK		0x07
#define TWT_INFO_RESP_REQ		0x08
#define TWT_INFO_NEXT_TWT_REQ		0x10
#define TWT_INFO_NEXT_TWT_SIZE_MASK	0x60
#define TWT_INFO_NEXT_TWT_SIZE_SHIFT	0x5

/* Next TWT Subfield Size field */
#define TWT_INFO_NEXT_TWT_SIZE_0	0	/* 0 byte */
#define TWT_INFO_NEXT_TWT_SIZE_32	1	/* 4 bytes */
#define TWT_INFO_NEXT_TWT_SIZE_48	2	/* 6 bytes */
#define TWT_INFO_NEXT_TWT_SIZE_64	3	/* 8 bytes */

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* _802_11ah_h_ */
