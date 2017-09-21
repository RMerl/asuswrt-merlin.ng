/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef _UDB_IOCTL_COMMON_H_
#define _UDB_IOCTL_COMMON_H_

#define MAX_NAMELEN 31
typedef struct shnagent_nl_upnp_ioctl_t
{
	uint8_t		friendly_name[MAX_NAMELEN + 1];
	uint8_t		manufacturer[MAX_NAMELEN + 1];
} shnagent_nl_upnp_ioctl;

enum
{
	UDB_IOCTL_COMMON_OP_NA = 0,
	UDB_IOCTL_COMMON_OP_GET_DPI_CONFIG,
	UDB_IOCTL_COMMON_OP_SET_DPI_CONFIG,
	UDB_IOCTL_COMMON_OP_GET_USER,
	UDB_IOCTL_COMMON_OP_GET_APP,
	UDB_IOCTL_COMMON_OP_GET_APP_BW_RESET,
	UDB_IOCTL_COMMON_OP_SET_APP_PATROL,
	UDB_IOCTL_COMMON_OP_GET_APP_PATROL,
	UDB_IOCTL_COMMON_OP_SET_WPR_CONF,
	UDB_IOCTL_COMMON_OP_SET_WPR_ENABLE,
	UDB_IOCTL_COMMON_OP_SET_WPR_DISABLE,
	UDB_IOCTL_COMMON_OP_SET_REDIRECT_URL,
	UDB_IOCTL_COMMON_OP_MAX
};

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#ifndef IFACE_NAME_SIZE
#define IFACE_NAME_SIZE 16
#endif

#if TMCFG_E_UDB_CORE_RULE_FORMAT_V2
#define META_NAME_SSTR_LEN 64
#define META_NAME_LSTR_LEN 128
#endif

#define AVAILABLE	(uint8_t)1
#define NOT_AVAILABLE	(uint8_t)0

#define MAX_REDIRECT_URL_LEN 512

typedef struct wpr_config
{
	uint8_t period_min;
	uint8_t max_num;
	uint8_t url[MAX_REDIRECT_URL_LEN];
} wpr_config_t;

#define DEVID_MAX_USER 253
#define MAX_APP_PER_USER 128
#define DEVID_APP_RATE_TABLE_POOL_SIZE	(DEVID_MAX_USER * MAX_APP_PER_USER)

typedef struct devdb_ioc_entry
{
	uint16_t vendor_id;
	uint16_t name_id;   // v1.0: os_id
	uint16_t class_id;
	uint16_t cat_id;    // v1.0: type_id
	uint16_t dev_id;
	uint16_t family_id;
} devdb_ioctl_entry_t;

typedef struct udb_ioc_os
{
	devdb_ioctl_entry_t de; //!< Device entry identifier. Should be unique.
	/*!
	 * \brief The priority of currently matched os. This is used so that
	 * callers know when they must update the os. For short, update whenever the
	 * device is matched a higher priority rule.
	 */
	uint16_t de_prio;
} udb_ioctl_os_t;

#define UDB_ENTRY_HOST_NAME_SIZE 32

#define MAX_WRS_CAT_NUM 128
typedef struct udb_ioc_entry
{
	uint8_t uid;
	uint8_t mac[6];

	uint8_t ipv4[4];
	uint8_t ipv6[16];

	char host_name[UDB_ENTRY_HOST_NAME_SIZE];

	udb_ioctl_os_t os;

	uint64_t ts;
	uint64_t ts_create;

#if TMCFG_E_UDB_CORE_RULE_FORMAT_V2
	uint32_t used_time_sec;
#endif
#if TMCFG_E_UDB_CORE_SHN_QUERY
	shnagent_nl_upnp_ioctl upnp_data;
#endif
#if TMCFG_E_UDB_CORE_URL_QUERY
	uint32_t wrs_stat[MAX_WRS_CAT_NUM];
#endif
	uint8_t available;
} udb_ioctl_entry_t;

typedef struct app_ioc_entry
{
	uint8_t uid;
	uint8_t cat_id;
	uint16_t app_id;

	uint64_t last_elapsed_ts;

	uint64_t down_recent_accl;
	uint32_t down_recent_accl_pkt;
	uint64_t up_recent_accl;
	uint32_t up_recent_accl_pkt;

	uint8_t available;
} app_ioctl_entry_t;

#define UDB_PATROL_LOG_SIZE 500

typedef struct app_patrol_list_ioc_entry
{
	uint8_t uid;
	uint8_t mac[6];
	uint8_t cat_id;
	uint16_t app_id;

	int app_meta_idx;

	uint64_t time;
	uint8_t flag;
	uint8_t available;
} app_patrol_list_ioc_entry_t;

typedef struct app_bw_ioc_entry
{
	uint8_t uid;
	uint8_t cat_id;
	uint16_t app_id;

	int app_meta_idx;

	uint64_t down_recent;
	uint64_t up_recent;

	uint8_t available;
} app_bw_ioctl_entry_t;

typedef struct patrol_ioc_app
{
	uint8_t cat_id;
	uint16_t app_id;
	
} patrol_ioc_app_t;

typedef struct patrol_ioc_pfile
{
	uint16_t pfile_id;
	uint32_t app_cnt;
	patrol_ioc_app_t app_entry[0];
	
} patrol_ioc_pfile_t;

typedef struct patrol_ioc_pfile_ptr
{
	uint16_t pfile_cnt;
	patrol_ioc_pfile_t pfile_entry[0];
	
} patrol_ioc_pfile_ptr_t;

typedef struct patrol_ioc_mac
{
	uint8_t mac[6];
	uint16_t pfile_id;

} patrol_ioc_mac_t;

typedef struct patrol_ioc_mac_ptr
{
	uint16_t mac_cnt;
	patrol_ioc_mac_t mac_entry[0];
	
} patrol_ioc_mac_ptr_t;

#ifdef __KERNEL__
int udb_ioctl_common_op_copy_in(uint8_t op, void *buf, uint32_t buf_len);
int udb_ioctl_common_op_copy_out(uint8_t op, void *buf, uint32_t buf_len, uint32_t *buf_used_len);
int udb_ioctl_common_op_copy_none(uint8_t op);
#endif

#endif /* _UDB_IOCTL_COMMON_H_ */

