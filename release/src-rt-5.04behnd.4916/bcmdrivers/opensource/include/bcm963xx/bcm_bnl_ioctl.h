#ifndef __BCM_BNL_IOCTL_H_INCLUDED__
#define __BCM_BNL_IOCTL_H_INCLUDED__
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 *******************************************************************************
 * File Name : bcm_bnl_ioctl.h
 *
 *******************************************************************************
 */

#include "bcm_score_defs.h"

#define BNL_DEV_NAME		"bnl"
#define BNL_NUM_DEVICES		1
#define BNL_DRV_DEV_NAME	"/dev/" BNL_DEV_NAME


#define BNL_MAX_BINS 8
#define BNL_MAX_BIN_CFGS 8
#define BNL_MAX_BIN_LIMITS (BNL_MAX_BINS -1)
#define BCM_MAX_BNL_STREAMS SCORE_NUM_OF_STREAM_IDX
#define BNL_MAX_PROFILES SCORE_MAX_DCP_NUM
#define BNL_STREAM_ID_INVALID SCORE_STREAM_ID_INVALID

/* Functional interface return status */
#define BNL_ERROR                (-1)    /* Functional interface error     */
#define BNL_SUCCESS              0       /* Functional interface success   */

#define BNL_PROFILE_ID_INVALID 0xFF

/*
 * Ioctl definitions.
 */
typedef enum {
	/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
	 * processor. Hence start all IOCTL values from 100 to prevent conflicts
	 */
	BNL_IOCTL_SYS=100,
	BNL_IOCTL_CTRL_INIT,
	BNL_IOCTL_PROFILE_ADD,
	BNL_IOCTL_PROFILE_DELETE,
	BNL_IOCTL_PROFILE_STATUS,
	BNL_IOCTL_PROFILE_CONFIG_SET,
	BNL_IOCTL_PROFILE_CONFIG_GET,
	BNL_IOCTL_STREAM_MON_START,
	BNL_IOCTL_STREAM_MON_STOP,
	BNL_IOCTL_STREAM_STATS_GET,
	BNL_IOCTL_STREAM_DUMP, /* similar to stas, but has cfg info, tuple ect..*/
	BNL_IOCTL_STREAM_TABLE_DUMP, /* dump all valid streams */
	BNL_IOCTL_MAX
} bnl_ioctl_t;

typedef struct bnlctl_profile_stats {
	uint16_t profile_id;

	uint32_t cumm_alloc;
	uint32_t cumm_free;
	uint32_t cumm_alloc_failures;
	uint32_t notsval_ct_skip;
	uint32_t notsval_pkt_skip;
	uint32_t udp_no_quic_ct_skip;

	uint64_t total_pkt_drops;
	uint64_t total_ct_violations;
} bnlctl_profile_stats_t;

typedef struct bnlctl_profile_cfg {
	union {
		struct {
			uint16_t ctrl_ring_enable:1;/*supported only with ADD */
			uint16_t raw_data_ring_enable:1;/*supported only with ADD */
			uint16_t config_ignore_pure_ack_for_rtt:1;
			uint16_t config_noreply_timeout:1;
			uint16_t config_bin:1;
			uint16_t rsvd:11;
		};
		uint16_t flags;
	};
    
    uint16_t noreply_timeout_val;
    uint8_t ignore_pure_ack_for_rtt_val;
    uint8_t bin_id;
    uint32_t bin_limits[BNL_MAX_BIN_LIMITS];

	uint16_t profile_id; /*return value */
} bnlctl_profile_cfg_t;

typedef struct bnlctl_profile {
	uint16_t profile_id;
	int cmd_fd; /*file descriptor used for I/O with driver */

	/* TODO add rings  */
} bnlctl_profile_t;

typedef struct bnlctl_stream_start_cfg {
	score_stream_tuple_t tuple;
	uint16_t profile_id;
	score_bnl_stream_cfg_t bnl_cfg;
	uint16_t stream_id; /*retun value*/
} bnlctl_stream_start_cfg_t;

typedef struct bnlctl_stream_stop_cfg {
	uint16_t stream_id;
	uint16_t profile_id;
} bnlctl_stream_stop_cfg_t;

typedef struct bnl_stream_rtt_stats {
	uint32_t min;
	uint32_t max;
	uint64_t sum;
	uint64_t square_sum;
	uint32_t count;
	uint32_t timeout_cnt;
	uint32_t no_slot_cnt;
	uint32_t skip_cnt;
	uint32_t bin_data[BNL_MAX_BINS];
} bnl_stream_rtt_stats_t;

typedef struct bnlctl_stream_stats {
	/*input*/
	uint16_t stream_id;
	uint16_t profile_id;
	/*output*/
	bnl_stream_rtt_stats_t rtt_orig;
	bnl_stream_rtt_stats_t rtt_reply;
} bnlctl_stream_stats_t;

typedef struct bnlctl_stream_dump {
	/*input*/
	uint16_t profile_id;
	uint16_t stream_id; /*in/out*/
    uint16_t cur_idx; /*used for tbl dump */
	/*output*/
    uint16_t next_idx;/*used for tbl dump */
	uint16_t app_group_id;
	score_stream_tuple_t orig_tuple;
	score_stream_tuple_t reply_tuple;
	uint32_t rtt_orig_bin_limits[BNL_MAX_BIN_LIMITS];
	uint32_t rtt_reply_bin_limits[BNL_MAX_BIN_LIMITS];
	bnl_stream_rtt_stats_t rtt_orig;
	bnl_stream_rtt_stats_t rtt_reply;
} bnlctl_stream_dump_t;

typedef union bnlctl_data {
	bnlctl_profile_stats_t profile_stats;
	bnlctl_profile_cfg_t profile_cfg;
	bnlctl_stream_start_cfg_t start_cfg;
	bnlctl_stream_stop_cfg_t stop_cfg;
	bnlctl_stream_stats_t stream_stats;
	bnlctl_stream_dump_t stream_dump;
} bnlctl_data_t;

#endif /* __BCM_BNL_IOCTL_H_INCLUDED__ */
