/*
 * Custom ioctl defines for
 * BAD AP Manager of
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlioctl_bad_ap_manager.h 687754 2017-03-02 12:14:29Z $
 */

#ifndef wl_ioctl_bam_h
#define wl_ioctl_bam_h

/* define version for BAM
 * MAJOR : shall be changed for followings
 * - if iovar architect is changed
 * - if iovar buffer length is changed
 * - if usage of sub command is changed
 * MINOR : shall be changed for followings
 * - if new sub feature is added
 */
#define WL_BAM_IOV_MAJOR_VER	1
#define WL_BAM_IOV_MINOR_VER	1
#define WL_BAM_IOV_MAJOR_VER_SHIFT	8
#define WL_BAM_IOV_VERSION \
	((WL_BAM_IOV_MAJOR_VER << WL_BAM_IOV_MAJOR_VER_SHIFT) | WL_BAM_IOV_MINOR_VER)
#define WL_BAM_IOV_GET_MAJOR(x) (x >> WL_BAM_IOV_MAJOR_VER_SHIFT)
#define WL_BAM_IOV_GET_MINOR(x) (x & 0xFF)

/* define ioctl buffer length */
#define BAM_IOCTL_BUF_LEN	256

/* define BAD AP types */
#define BAM_TYPE_NONE		0
#define BAM_TYPE_ALL		0xffffffff

#define BAM_TYPE_NO_BCN	0x00000001
#define BAM_TYPE_LAZY		0x00000002
#define BAM_TYPE_HNR		0x00000004
#define BAM_TYPE_BQSEQ		0x00000008

#define BAM_CMD_NAME		"bam"

enum wl_bam_cmd_ids {
	WL_BAM_CMD_ENABLE_V1 = 1,
	WL_BAM_CMD_DISABLE_V1 = 2,
	WL_BAM_CMD_CONFIG_V1 = 3,
	WL_BAM_CMD_DUMP_V1 = 4,

	WL_BAM_CMD_LAST
};

enum wl_bam_iov_ap {
	BAM_IOV_AP_UNKNOWN = 1,
	BAM_IOV_AP_BAD = 2,
	BAM_IOV_AP_NORM = 3
};

struct wl_bam_iov_enable_v1 {
	uint32 bad_type;
};

struct wl_bam_iov_config_v1 {
	uint32 bad_type;
	uint8 config[0];
};

struct wl_bam_iov_dump_v1 {
	uint32 bad_type;
	uint32 count;
	uint8 dump[0];
};

/* START SUB-MODULE : BAM_TYPE_NO_BCN */
#define BAM_TYPE_NO_BCN_STR	"bcn"

#define BAM_BCN_DECISION_BAD_MIN	5u
#define BAM_BCN_DECISION_BAD_MAX	100u
#define BAM_BCN_DECISION_NORM_MIN	100u
#define BAM_BCN_DECISION_NORM_MAX	65535u
#define BAM_BCN_RETRY_UNKNOWN_MIN	1u
#define BAM_BCN_RETRY_UNKNOWN_MAX	100u
#define BAM_BCN_RETRY_BAD_MIN		1u
#define BAM_BCN_RETRY_BAD_MAX		100u
#define BAM_BCN_NULL_PERIOD_MIN		50u
#define BAM_BCN_NULL_PERIOD_MAX		1000u
#define BAM_BCN_WAIT_PKT_TIME_MIN	100u
#define BAM_BCN_WAIT_PKT_TIME_MAX	1000u
#define BAM_BCN_TRIG_TIME_MIN		500u
#define BAM_BCN_TRIG_TIME_MAX		2000u

struct wl_bam_iov_bcn_config_v1 {
	uint16	decision_cnt_bad;	/* decision number to define as bad AP */
	uint16	decision_cnt_normal;	/* decision number to define as normal AP */
	uint16	retry_max_bad;		/* null send retry count for bad AP */
	uint16	retry_max_unknown;	/* null send retry count if AP type is not fixed */
	uint16	null_send_prd;		/* default period between null pkts */
	uint16	wait_pkt_time;		/* wait pkts after recieve beacon */
	uint16	training_trig_time;	/* trigger bad training if no beacons for this time */
};

struct wl_bam_iov_dump_bcn_elem_v1 {
	uint16	wlc_idx; /* wlc idx of this bsscfg info */
	uint16	bss_idx; /* bsscfg idx of this bsscfg info */
	uint16	ap_type; /* ap type of this bsscfg info */
	uint16	state; /* state number */
	uint16	retry_cnt; /* retry count of NULL sending for this trial */
	uint16	bad_cnt;	/* number of bad operation count */
	uint16	normal_cnt;	/* number of normal operation count */
	uint16	padding;
};

/* END SUB-MODULE : BAM_TYPE_NO_BCN */

/* START SUB-MODULE : BAM_TYPE_LAZY */
#define BAM_TYPE_LAZY_STR "lazy"

/* END SUB-MODULE : BAM_TYPE_LAZY */

/* START SUB-MODULE : BAM_TYPE_HNR */
#define BAM_TYPE_HNR_STR	"honor"

/* END SUB-MODULE : BAM_TYPE_HNR */

/* START SUB-MODULE : BAM_TYPE_HNR */
#define BAM_TYPE_BQSEQ_STR	"bqseq"

/* END SUB-MODULE : BAM_TYPE_HNR */

#endif /* wl_ioctl_bam_h */
