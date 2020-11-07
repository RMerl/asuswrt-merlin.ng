/*
 * IGSL Command Line Utility: This utility can be used to add/remove
 * snooping capability on desired bridge interface.
 *
 * Copyright 2019 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: igsu.c 775533 2019-06-03 16:36:58Z $
 */
#include <stdio.h>
#include <sys/types.h>
#include <typedefs.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if defined(linux)
#include "igsu_linux.h"
#else /* !defined(linux) */
#error "Unsupported osl"
#endif /* defined(linux) */
#include <igs_cfg.h>
#include "igsu.h"

#define MAX_DATA_SIZE  sizeof(igs_cfg_request_t)

static int igs_cfg_bridge_add(char *argv[]);
static int igs_cfg_bridge_del(char *argv[]);
static int igs_cfg_bridge_list(char *argv[]);
static int igs_cfg_sdb_list(char *argv[]);
static int igs_cfg_rtport_list(char *argv[]);
static int igs_cfg_stats_show(char *argv[]);

static igs_cmd_arg_t cmd_args[] =
{
	{
		"add",
		"bridge",
		igs_cfg_bridge_add,
		IGS_ARGC_ADD_BRIDGE
	},
	{
		"del",
		"bridge",
		igs_cfg_bridge_del,
		IGS_ARGC_DEL_BRIDGE
	},
	{
		"list",
		"bridge",
		igs_cfg_bridge_list,
		IGS_ARGC_LIST_BRIDGE
	},
	{
		"list",
		"sdb",
		igs_cfg_sdb_list,
		IGS_ARGC_LIST_IGSDB
	},
	{
		"list",
		"rtport",
		igs_cfg_rtport_list,
		IGS_ARGC_LIST_RTPORT
	},
	{
		"show",
		"stats",
		igs_cfg_stats_show,
		IGS_ARGC_SHOW_STATS
	}
};

static void
igs_usage(FILE *fid)
{
	fprintf(fid, IGS_USAGE);
	return;
}

static int
igs_cfg_bridge_add(char *argv[])
{
	igs_cfg_request_t req;

	bzero((char *)&req, sizeof(igs_cfg_request_t));

	strncpy((char *)req.inst_id, argv[3], sizeof(req.inst_id)-1);
	req.command_id = IGSCFG_CMD_BR_ADD;
	req.oper_type = IGSCFG_OPER_TYPE_SET;

	/* Send request to kernel */
	if (igs_cfg_request_send(&req, sizeof(igs_cfg_request_t)) < 0)
	{
		fprintf(stderr, "Unable to send request to IGS\n");
		return (FAILURE);
	}

	if (req.status != IGSCFG_STATUS_SUCCESS)
	{
		fprintf(stderr, "%s\n", req.arg);
		return (FAILURE);
	}

	return (SUCCESS);
}

static int
igs_cfg_bridge_del(char *argv[])
{
	igs_cfg_request_t req;

	bzero((char *)&req, sizeof(igs_cfg_request_t));

	strncpy((char *)req.inst_id, argv[3], sizeof(req.inst_id)-1);
	req.command_id = IGSCFG_CMD_BR_DEL;
	req.oper_type = IGSCFG_OPER_TYPE_SET;

	/* Send request to kernel */
	if (igs_cfg_request_send(&req, sizeof(igs_cfg_request_t)) < 0)
	{
		fprintf(stderr, "Unable to send request to IGS\n");
		return (FAILURE);
	}

	if (req.status != IGSCFG_STATUS_SUCCESS)
	{
		fprintf(stderr, "%s\n", req.arg);
		return (FAILURE);
	}

	return (SUCCESS);
}

static int
igs_cfg_bridge_list(char *argv[])
{
	fprintf(stdout, "TBD\n");
	return (SUCCESS);
}

#ifdef BCM_NBUFF_WLMCAST_IPV6

/* unified sdb list function for both IPv4 and IPv6 */
static int
_igs_cfg_sdb_list_(int type, char *inst_id)
{
	igs_cfg_request_t req;
	igs_cfg_sdb_list_t *list;
	int32 i;

	strncpy((char *)req.inst_id, inst_id, sizeof(req.inst_id)-1);
	req.inst_id[sizeof(req.inst_id)-1] = '\0';

	req.command_id = (type == AF_INET6) ? IGSCFG_CMD_IGSDB_LIST_IPV6:IGSCFG_CMD_IGSDB_LIST;
	req.oper_type = IGSCFG_OPER_TYPE_GET;
	req.size = sizeof(req.arg);
	list = (igs_cfg_sdb_list_t *)req.arg;

	if (igs_cfg_request_send(&req, sizeof(igs_cfg_request_t)) < 0)
	{
		fprintf(stderr, "Unable to send request to IGS\n");
		return (FAILURE);
	}

	if (req.status != IGSCFG_STATUS_SUCCESS)
	{
		fprintf(stderr, "Unable to get the IGSDB list\n");
		fprintf(stderr, "%s\n", req.arg);
		return (FAILURE);
	}

	fprintf(stdout, "-------| IPv%d |---------------\n", (type == AF_INET) ? 4:6);
	fprintf(stdout, "Group               Members           %s\n", (type == AF_INET) ? "Dev":"");

	for (i = 0; i < list->num_entries; i++)
	{
		/* need to use two fprintf as ipaddr_str() use global pointer */
		fprintf(stdout, "%s    ",
				ipaddr_str(type, &(list->sdb_entry[i].mgrp_ip)));
		fprintf(stdout, "%s    %s\n",
				ipaddr_str(type, &(list->sdb_entry[i].mh_ip)),
				(type == AF_INET)?  list->sdb_entry[i].if_name :"");
	}

	fprintf(stdout, "----------------------------------\n", type);

	return (SUCCESS);
}

static int
igs_cfg_sdb_list(char *argv[])
{
	_igs_cfg_sdb_list_(AF_INET, argv[3]);
	_igs_cfg_sdb_list_(AF_INET6, argv[3]);
}

#else

static int
igs_cfg_sdb_list(char *argv[])
{
	igs_cfg_request_t req;
	igs_cfg_sdb_list_t *list;
	bool first_row = TRUE;
	int32 i;

	strncpy((char *)req.inst_id, argv[3], sizeof(req.inst_id)-1);
	req.inst_id[sizeof(req.inst_id)-1] = '\0';

	req.command_id = IGSCFG_CMD_IGSDB_LIST;
	req.oper_type = IGSCFG_OPER_TYPE_GET;
	req.size = sizeof(req.arg);
	list = (igs_cfg_sdb_list_t *)req.arg;

	if (igs_cfg_request_send(&req, sizeof(igs_cfg_request_t)) < 0)
	{
		fprintf(stderr, "Unable to send request to IGS\n");
		return (FAILURE);
	}

	if (req.status != IGSCFG_STATUS_SUCCESS)
	{
		fprintf(stderr, "Unable to get the IGSDB list\n");
		fprintf(stderr, "%s\n", req.arg);
		return (FAILURE);
	}

	fprintf(stdout, "Group           Members         Interface\n");

	for (i = 0; i < list->num_entries; i++)
	{
		first_row = TRUE;
		fprintf(stdout, "%08x        ", list->sdb_entry[i].mgrp_ip);
		if (first_row)
		{
			fprintf(stdout, "%08x        ", list->sdb_entry[i].mh_ip);
			fprintf(stdout, "%s\n", list->sdb_entry[i].if_name);
			first_row = FALSE;
			continue;
		}
		fprintf(stdout, "                ");
		fprintf(stdout, "%08x        ", list->sdb_entry[i].mh_ip);
		fprintf(stdout, "%s\n", list->sdb_entry[i].if_name);
	}

	return (SUCCESS);
}
#endif /* BCM_NBUFF_WLMCAST_IPV6 */

static int
igs_cfg_rtport_list(char *argv[])
{
	igs_cfg_request_t req;
	igs_cfg_rtport_list_t *list;
	int32 i;

	bzero((char *)&req, sizeof(igs_cfg_request_t));

	strncpy((char *)req.inst_id, argv[3], sizeof(req.inst_id)-1);
	req.command_id = IGSCFG_CMD_RTPORT_LIST;
	req.oper_type = IGSCFG_OPER_TYPE_GET;
	req.size = sizeof(req.arg);

	/* Send request to kernel */
	if (igs_cfg_request_send(&req, sizeof(igs_cfg_request_t)) < 0)
	{
		fprintf(stderr, "Unable to send request to IGS\n");
		return (FAILURE);
	}

	if (req.status != IGSCFG_STATUS_SUCCESS)
	{
		fprintf(stderr, "%s\n", req.arg);
		return (FAILURE);
	}

	fprintf(stdout, "Router          Interface\n");

	list = (igs_cfg_rtport_list_t *)req.arg;
	for (i = 0; i < list->num_entries; i++)
	{
		fprintf(stdout, "%08x        ", list->rtport_entry[i].mr_ip);
		fprintf(stdout, "%-15s", list->rtport_entry[i].if_name);
	}

	fprintf(stdout, "\n");

	return (SUCCESS);
}

static int
igs_cfg_stats_show(char *argv[])
{
	igs_cfg_request_t req;
	igs_stats_t *igss;

	strncpy((char *)req.inst_id, argv[3], sizeof(req.inst_id)-1);
	req.inst_id[sizeof(req.inst_id)-1] = '\0';

	req.command_id = IGSCFG_CMD_IGS_STATS;
	req.oper_type = IGSCFG_OPER_TYPE_GET;
	req.size = sizeof(igs_stats_t);
	igss = (igs_stats_t *)req.arg;

	if (igs_cfg_request_send(&req, sizeof(igs_cfg_request_t)) < 0)
	{
		fprintf(stderr, "Unable to send request to IGS\n");
		return (FAILURE);
	}

	if (req.status != IGSCFG_STATUS_SUCCESS)
	{
		fprintf(stderr, "Unable to get the IGS stats\n");
		return (FAILURE);
	}

	fprintf(stdout, "IgmpPkts        IgmpQueries     "
	        "IgmpReports     IgmpV2Reports   IgmpLeaves\n");
	fprintf(stdout, "%-15d %-15d %-15d %-15d %d\n",
	        igss->igmp_packets, igss->igmp_queries,
	        igss->igmp_reports, igss->igmp_v2reports,
	        igss->igmp_leaves);
	fprintf(stdout, "IgmpNotHandled  McastGroups     "
	        "McastMembers    MemTimeouts\n");
	fprintf(stdout, "%-15d %-15d %-15d %d\n",
	        igss->igmp_not_handled, igss->igmp_mcast_groups,
	        igss->igmp_mcast_members, igss->igmp_mem_timeouts);

	return (SUCCESS);
}

int
#if defined(linux)
main(int argc, char *argv[])
#else /* !defined(linux) */
#error "Unsupported osl"
#endif /* defined(linux) */
{
	int j, ret;
	bool cmd_syntax = FALSE;

	if (argc < 2)
	{
		igs_usage(stdout);
		return (SUCCESS);
	}

	/* Find the command type */
	for (j = 0; j < sizeof(cmd_args)/sizeof(cmd_args[0]); j++)
	{
		if ((strcmp(argv[1], cmd_args[j].cmd_oper_str) == 0) &&
		    (argc - 1 == cmd_args[j].arg_count) &&
		    ((cmd_args[j].cmd_id_str == NULL) ||
		    ((argv[2] != NULL) &&
		    (strcmp(argv[2], cmd_args[j].cmd_id_str) == 0))))
		{
			cmd_syntax = TRUE;
			break;
		}
	}

	if (!cmd_syntax)
	{
		igs_usage(stdout);
		return (SUCCESS);
	}

	/* Call the command processing function. This function parses
	 * prepare command request and sends it kernel.
	 */
	if ((ret = cmd_args[j].input(argv)) < 0)
	{
		fprintf(stderr, "Command failed\n");
		return (FAILURE);
	}

	return (SUCCESS);
}
