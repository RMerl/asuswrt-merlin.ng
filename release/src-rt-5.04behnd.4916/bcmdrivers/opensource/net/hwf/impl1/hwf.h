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

#include <net/netfilter/nf_conntrack_tuple.h>

#define HWF_INVALID_HW_IDX  -1
#define HWF_ERROR -1
#define HWF_SUCCESS 0

#define HWF_ALLOW 0
#define HWF_DROP 1

#define HWF_MODNAME "Broadcom HW Firewall Module"
#define HWF_VERSION	"v1.0"

#define HWF_MAX_VSERVERS 128
#define HWF_MAX_RATELIMITERS 16

#define HWF_DEBUG_ERROR 1
#define HWF_DEBUG_WARN  2
#define HWF_DEBUG_INFO  3
#define HWF_DEBUG_ALL  4

typedef struct {
	uint32_t pps_rate;               /**< Committed Information Rate (CIR) - in PPS (20 - 1M) */
	uint32_t pps_burst_size;         /**< Committed Burst Size (CBS) - in PPS (1 - 10K) */
	uint64_t committed_rate;         /**< Committed Information Rate (CIR) - bps (100K-10G) */
	uint32_t committed_burst_size;   /**< Committed Burst Size (CBS) - bytes (1K-100M) */
	uint16_t ref_count;
	struct {
		uint16_t valid:1;
		uint16_t rsvd:1;
	};
	int64_t  index;   /**< HW ratelimiter index */
	uint64_t hit_cnt; /*returned by HW */ 
	uint64_t drop_cnt; /*returned by HW */ 
	hwfctl_ratelimit_type_t type; 
	char name[HWF_MAX_KEY_NAME_LEN]; /* name- used only in host drv */
} bcm_hwf_ratelimiter_t;

typedef struct {
	union nf_inet_addr daddr;
	uint16_t l3num;
	uint16_t dst_port;
	uint8_t l4_proto;
	bcm_hwf_ratelimiter_t *ratelimiter;
	struct {
		uint16_t valid:1;
		uint16_t rsvd:15;
	};
	char name[HWF_MAX_KEY_NAME_LEN];
} bcm_hwf_vserver_t;

typedef struct {
	struct hlist_node hnode; /* hash entry */
	struct nf_conntrack_tuple tuple;
	long  hwid;
	union {
		uint16_t flags;
		struct {
			uint16_t is_static:1; /* configured entry */
			uint16_t is_exp:1;
			uint16_t is_ratelimiter:1;
			uint16_t ratelimiter_id:6;   /* HWF driver ratelimiter table index */
			uint16_t rsvd:7;
		};
	};
	uint16_t refcnt; /* protected by lock no need of atomic */
} bcm_hwf_obj_t;

typedef struct {
	struct hlist_node hnode; /* hash entry */
	union nf_inet_addr l3addr;
	unsigned long expires;
	unsigned long idle_timeout;/*when 0 entires */
	int32_t total_count;
	uint32_t max_count;
	uint32_t max_limit;

	uint32_t rate_limit_drops; /*drops due to rate exceed*/
	uint32_t max_limit_drops; /*drops due to max conn limit*/
	uint16_t l3proto;
	union {
		uint16_t flags;
		struct {
			uint16_t is_static:1;
			uint16_t rsvd:15;
		};
	};
	struct {
		unsigned long interval;
		unsigned long expires;
		uint32_t limit;
		uint32_t count; 
		uint16_t burst;
		uint16_t max_burst;
		uint32_t after_max_limit; /*rate after max_limit reached */
		uint32_t after_max_count;
	} rate;
} bcm_host_obj_t;

/* 512 hash buckets */
#define HOST_HASH_EXP 9
#define HOST_HASHTBL_SIZE (1<<HOST_HASH_EXP)
struct bcm_hosts {
	DECLARE_HASHTABLE(hashtbl, HOST_HASH_EXP);
 	spinlock_t lock;
	unsigned long interval; /* periodic refresh interval on hosts */
	unsigned long next_refresh; /* time for next refresh*/
	uint32_t idle_timeout; /* timeout after ct_count of host is 0 */
	uint32_t num_hosts;
	uint32_t max_hosts;
	uint32_t default_ct_max;
	unsigned long ct_rate_interval;
	uint32_t default_ct_rate_limit;
	uint32_t after_max_ct_rate;
	uint32_t default_ct_burst;
	uint32_t total_ct_count;
	uint64_t total_pkt_drops;
	uint64_t nohost_pkt_drops;
	uint32_t nohost_pkt_rate; /*pkts/sec to be allowed when host is not present*/
	uint32_t nohost_pkt_count;
	unsigned long nohost_rate_refresh;
	bool allow_dynamic_hosts;
};

struct hosts_iter_state {
	struct hlist_head *hash;
	unsigned int htable_size;
	unsigned int bucket;
};

int bcm_hwf_enable(bool enable);
int bcm_hwf_expect_lookup_enable(bool enable);
int bcm_hwf_wan_miss_ratelimit_enable(bool enable);
int bcm_hwf_lan_ct_limit_enable(bool enable);
int bcm_hwf_status(hwfctl_data_t *hwfctl);

int bcm_hwf_limit_host_obj_add(hwfctl_hosts_t *host);
int bcm_hwf_limit_host_obj_update(hwfctl_hosts_t *host);
int bcm_hwf_limit_host_obj_delete(hwfctl_hosts_t *host);
int bcm_hwf_limit_hosts_cfg_set(hwfctl_data_t *hwfctl);
int bcm_hwf_limit_hosts_cfg_get(hwfctl_data_t *hwfctl);

int bcm_hwf_ratelimiter_add(hwfctl_ratelimiter_t *new_ratelimiter);
int bcm_hwf_ratelimiter_update(hwfctl_ratelimiter_t *new_ratelimiter);
int bcm_hwf_ratelimiter_delete(const char *name);


int bcm_hwf_vserver_add(hwfctl_vserver_t *ctl_vserver);
int bcm_hwf_vserver_update(hwfctl_vserver_t *ctl_vserver);
int bcm_hwf_vserver_delete(const char *name);

void static inline hwf_swap_ipaddr(union nf_inet_addr *addr)
{
	addr->all[0] = ntohl(addr->all[0]);
	addr->all[1] = ntohl(addr->all[1]);
	addr->all[2] = ntohl(addr->all[2]);
	addr->all[3] = ntohl(addr->all[3]);
}
