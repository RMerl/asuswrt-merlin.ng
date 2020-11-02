/*
 * Fundamental types and constants relating to WFA MBO
 * (Multiband Operation)
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
 * $Id$
 */

#ifndef _MBO_H_
#define _MBO_H_

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* WiFi MBO OUI values */
#define MBO_OUI         WFA_OUI      /* WiFi OUI 50:6F:9A */
/* oui_type field identifying the type and version of the MBO IE. */
#define MBO_OUI_TYPE    WFA_OUI_TYPE_MBO /* OUI Type/Version */
/* IEEE 802.11 vendor specific information element. */
#define MBO_IE_ID          0xdd

#define MBO_ATTR_HDR_LEN         2      /* ID + 1-byte length field */

/* MBO attributes as defined in the mbo spec */
enum {
	MBO_ATTR_MBO_AP_CAPABILITY = 1,
	MBO_ATTR_NON_PREF_CHAN_REPORT = 2,
	MBO_ATTR_CELL_DATA_CAP = 3,
	MBO_ATTR_ASSOC_DISALLOWED = 4,
	MBO_ATTR_CELL_DATA_CONN_PREF = 5,
	MBO_ATTR_TRANS_REASON_CODE = 6,
	MBO_ATTR_TRANS_REJ_REASON_CODE = 7,
	MBO_ATTR_ASSOC_RETRY_DELAY = 8
};

typedef BWL_PRE_PACKED_STRUCT struct wifi_mbo_ie_s {
	uint8 id;      /* IE ID: MBO_IE_ID 0xDD */
	uint8 len;     /* IE length */
	uint8 oui[WFA_OUI_LEN]; /* MBO_OUI 50:6F:9A */
	uint8 oui_type;   /* MBO_OUI_TYPE 0x16 */
	uint8 attr[1]; /* var len attributes */
} BWL_POST_PACKED_STRUCT wifi_mbo_ie_t;

#define MBO_IE_HDR_SIZE (OFFSETOF(wifi_mbo_ie_t, attr))
/* oui:3 bytes + oui type:1 byte */
#define MBO_IE_NO_ATTR_LEN  4

/* MBO AP Capability Attribute */
typedef BWL_PRE_PACKED_STRUCT struct wifi_mbo_ap_cap_ind_attr_s {
	/* Attribute ID - 0x01. */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* AP capability bitmap */
	uint8 cap_ind;
} BWL_POST_PACKED_STRUCT wifi_mbo_ap_cap_ind_attr_t;

/* Association Disallowed attribute */
typedef BWL_PRE_PACKED_STRUCT struct wifi_mbo_assoc_disallowed_attr_s {
	/* Attribute ID - 0x04. */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* Reason of not accepting new association */
	uint8 reason_code;
} BWL_POST_PACKED_STRUCT wifi_mbo_assoc_disallowed_attr_t;

/* Transition Reason Code Attribute */
typedef BWL_PRE_PACKED_STRUCT struct wifi_mbo_trans_reason_code_attr_s {
	/* Attribute ID - 0x06. */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* Reason of transition recommendation */
	uint8 trans_reason_code;
} BWL_POST_PACKED_STRUCT wifi_mbo_trans_reason_code_attr_t;

/* Transition Rejection Reason Code Attribute */
typedef BWL_PRE_PACKED_STRUCT struct wifi_mbo_trans_rej_reason_code_attr_s {
	/* Attribute ID - 0x07. */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* Reason of transition rejection */
	uint8 trans_rej_reason_code;
} BWL_POST_PACKED_STRUCT wifi_mbo_trans_rej_reason_code_attr_t;

/* Assoc Retry Delay Attribute */
typedef BWL_PRE_PACKED_STRUCT struct wifi_mbo_assoc_retry_delay_attr_s {
	/* Attribute ID - 0x08. */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* No of Seconds before next assoc attempt */
	uint16 reassoc_delay;
} BWL_POST_PACKED_STRUCT wifi_mbo_assoc_retry_delay_attr_t;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* __MBO_H__ */
