#ifndef __HWF_IOCTL_H_INCLUDED__
#define __HWF_IOCTL_H_INCLUDED__
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 *******************************************************************************
 * File Name : hwf_ioctl.h
 *
 *******************************************************************************
 */

#define HWF_DEV_NAME		"hwf"
#define HWF_NUM_DEVICES		1
#define HWF_DRV_DEV_NAME	"/dev/" HWF_DEV_NAME

#define HWF_MAX_KEY_NAME_LEN 24
/*
 * Ioctl definitions.
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
 * processor. Hence start all IOCTL values from 100 to prevent conflicts
 */
	hwfctl_ioctl_sys	= 100,
	hwfctl_ioctl_max
} hwfctl_ioctl_t;

typedef enum {
	hwfctl_subsys_system,
	hwfctl_subsys_vserver,
	hwfctl_subsys_hosts,
	hwfctl_subsys_ratelimiters,
	hwfctl_subsys_stats,
	hwfctl_subsys_flow,
	hwfctl_subsys_max
} hwfctl_subsys_t;

typedef enum {
	hwfctl_op_status,
	hwfctl_op_set,
	hwfctl_op_get,
	hwfctl_op_getnext,
	hwfctl_op_add,
	hwfctl_op_del,
	hwfctl_op_del_by_name,
	hwfctl_op_update,
	hwfctl_op_update_by_name,
	hwfctl_op_show,
	hwfctl_op_max
} hwfctl_op_t;


typedef struct {
	uint32_t ct_max;
	uint32_t ct_count;
	uint32_t ct_hw_max;
	uint32_t ct_hw_count;
	uint32_t ct_fail;
	uint32_t ct_hw_fail;

	uint32_t ct_cumm_insert;
	uint32_t ct_cumm_remove;

	uint16_t vsererver_max;
	uint16_t vsererver_count;

	uint32_t hosts_default_ct_max;
	uint32_t hosts_default_ct_rate;
	uint32_t hosts_default_ct_burst;
	uint32_t nohost_pkt_rate;
	uint32_t after_max_ct_rate;

	struct {
		uint16_t hwf_enable     : 1;
		uint16_t expect_lookup_enable	: 1;
		uint16_t wan_miss_ratelimit  : 1;
		uint16_t lan_ct_limit   : 1;
		uint16_t allow_dynamic_hosts  : 1;
		uint16_t unused         : 11;
	} config;

	struct {
		uint16_t hwf_enable     : 1;
		uint16_t expect_lookup_enable 	: 1;
		uint16_t wan_miss_ratelimit  : 1;
		uint16_t nohost_pkt_rate   : 1;
		uint16_t after_max_ct_rate   : 1;
		uint16_t lan_ct_limit   : 1;
		uint16_t allow_dynamic_hosts  : 1;
		uint16_t unused         : 9;
	} valid;	

}hwfctl_info_t;

typedef enum {
	hwfctl_ratelimit_vserver=1,
	hwfctl_ratelimit_wan_miss_all,
	hwfctl_ratelimit_type_max
}hwfctl_ratelimit_type_t;

typedef struct {
	uint32_t dst_ip[4];
	uint16_t dst_port;
	uint8_t l4_proto;
	uint8_t is_ipv6;
	char name[HWF_MAX_KEY_NAME_LEN];
	char ratelimiter_name[HWF_MAX_KEY_NAME_LEN];
} hwfctl_vserver_t;

typedef struct {
	int64_t index; /*returned by HW */ 
	uint64_t hit_cnt; /*returned by HW */ 
	uint64_t drop_cnt; /*returned by HW */ 
	uint32_t rate;
	uint32_t burst;
	hwfctl_ratelimit_type_t type; 
	char name[HWF_MAX_KEY_NAME_LEN];
} hwfctl_ratelimiter_t;

typedef struct {
	uint32_t ct_max;
	uint32_t ct_rate;
	uint32_t ct_burst;
	uint32_t ipaddr[4];
	uint8_t is_ipv6;
} hwfctl_hosts_t;


typedef struct {
	uint64_t total_pkt_drops; 
	uint64_t total_ct_violations; 
}hwfctl_stats_t;

typedef struct {
	uint32_t src_ip[4];
	uint32_t dst_ip[4];
	uint16_t src_port;
	uint16_t dst_port;
	uint8_t l4_proto;
	uint8_t is_ipv6;
	uint32_t hits;
} hwfctl_flow_t;

typedef struct {
	hwfctl_subsys_t subsys;
	hwfctl_op_t op;
	char name[HWF_MAX_KEY_NAME_LEN];
	int rc;
	int next_idx;
	hwfctl_info_t info;
	hwfctl_stats_t stats;
	union {
		hwfctl_vserver_t vserver;
		hwfctl_hosts_t hosts;
		hwfctl_ratelimiter_t ratelimiter;
		hwfctl_flow_t flow;
	};
} hwfctl_data_t;

#endif /* __HWF_IOCTL_H_INCLUDED__ */
