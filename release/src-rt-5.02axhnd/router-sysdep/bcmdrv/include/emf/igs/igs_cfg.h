/*
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: igs_cfg.h $
 */

#ifndef _IGS_CFG_H_
#define _IGS_CFG_H_

#ifdef BCM_NBUFF_WLMCAST_IPV6
#include <bcmipv6.h>
#endif // endif
#define SUCCESS                     0
#define FAILURE                     -1

#define IGSCFG_MAX_ARG_SIZE         1024

#define IGSCFG_CMD_BR_ADD           1
#define IGSCFG_CMD_BR_DEL           2
#define IGSCFG_CMD_BR_LIST          3
#define IGSCFG_CMD_IGSDB_LIST       4
#define IGSCFG_CMD_RTPORT_LIST      5
#define IGSCFG_CMD_IGS_STATS        6
#ifdef BCM_NBUFF_WLMCAST_IPV6
#define IGSCFG_CMD_IGSDB_LIST_IPV6	7
#define IGSCFG_CMD_IGS_STATS_IPV6	8
#endif // endif

#define IGSCFG_OPER_TYPE_GET        1
#define IGSCFG_OPER_TYPE_SET        2

#define IGSCFG_STATUS_SUCCESS       1
#define IGSCFG_STATUS_FAILURE       2
#define IGSCFG_STATUS_CMD_UNKNOWN   3
#define IGSCFG_STATUS_OPER_UNKNOWN  4
#define IGSCFG_STATUS_INVALID_IF    5

typedef struct igs_cfg_request
{
	uint8   inst_id[16];              /* Bridge name as instance id */
	uint32  command_id;               /* Command identifier */
	uint32  oper_type;                /* Operation type: GET, SET */
	uint32  status;                   /* Command status */
	uint32  size;                     /* Size of the argument */
	uint8   arg[IGSCFG_MAX_ARG_SIZE]; /* Command arguments */
} igs_cfg_request_t;

typedef struct igs_cfg_sdb_list
{
	uint32  num_entries;              /* Num of entries in IGSDB */
	struct sdb_entry
	{
#ifdef BCM_NBUFF_WLMCAST_IPV6
		union {
			uint32 mgrp_ip;			  /* Multicast group address */
			struct ipv6_addr mgrp_ipv6;
		};
		union {
			uint32 mh_ip;			  /* Member IP address */
			struct ipv6_addr mh_ipv6;
		};
#else
		uint32 mgrp_ip;			  /* Multicast group address */
		uint32 mh_ip;			  /* Member IP address */
#endif // endif
		uint8  if_name[16];       /* Interface member is present */
	} sdb_entry[0];
} igs_cfg_sdb_list_t;

typedef struct igs_cfg_rtport
{
	uint32  mr_ip;                  /* IP address of mcast router */
	uint8   if_name[16];            /* Name of the interface */
} igs_cfg_rtport_t;

typedef struct igs_cfg_rtport_list
{
	uint32           num_entries;     /* Number of entries in RTPORT list */
	igs_cfg_rtport_t rtport_entry[0]; /* Interface entry data */
} igs_cfg_rtport_list_t;

/*
 * IGMP Snooping Layer Statistics
 */
typedef struct igs_stats
{
	uint32  igmp_packets;             /* IGMP packets received */
	uint32  igmp_queries;             /* IGMP membership quries received */
	uint32  igmp_reports;             /* IGMP membership reports */
	uint32  igmp_v2reports;           /* IGMP v2 membership reports */
	uint32  igmp_leaves;              /* IGMP membership leaves */
	uint32  igmp_not_handled;         /* IGMP frames not handled */
	uint32  igmp_mem_timeouts;        /* IGMP membership timeouts */
	uint32  igmp_frames_fwd;          /* IGMP membership quries received */
	uint32  igmp_frames_sentup;       /* IGMP membership reports seen */
	uint32  igmp_mcast_groups;        /* Current total of mcast groups */
	uint32  igmp_mcast_members;       /* Current total of mcast members */
	uint32  igmp_missed_report_to;    /* Missed membership report TO count */
} igs_stats_t;

extern void igs_cfg_request_process(igs_cfg_request_t *cfg);

#endif /* _IGS_CFG_H_ */
