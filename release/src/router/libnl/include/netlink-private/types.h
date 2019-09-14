/*
 * netlink-private/types.h	Netlink Types (Private)
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2013 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#ifndef NETLINK_LOCAL_TYPES_H_
#define NETLINK_LOCAL_TYPES_H_

#include <netlink/list.h>
#include <netlink/route/link.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/route.h>
#include <netlink/idiag/idiagnl.h>
#include <netlink/netfilter/ct.h>
#include <netlink-private/route/tc-api.h>
#include <linux/tc_act/tc_mirred.h>

#define NL_SOCK_BUFSIZE_SET	(1<<0)
#define NL_SOCK_PASSCRED	(1<<1)
#define NL_OWN_PORT		(1<<2)
#define NL_MSG_PEEK		(1<<3)
#define NL_NO_AUTO_ACK		(1<<4)

#define NL_MSG_CRED_PRESENT 1

struct nl_cache_ops;
struct nl_sock;
struct nl_object;
struct nl_hash_table;

struct nl_cb
{
	nl_recvmsg_msg_cb_t	cb_set[NL_CB_TYPE_MAX+1];
	void *			cb_args[NL_CB_TYPE_MAX+1];

	nl_recvmsg_err_cb_t	cb_err;
	void *			cb_err_arg;

	/** May be used to replace nl_recvmsgs with your own implementation
	 * in all internal calls to nl_recvmsgs. */
	int			(*cb_recvmsgs_ow)(struct nl_sock *,
						  struct nl_cb *);

	/** Overwrite internal calls to nl_recv, must return the number of
	 * octets read and allocate a buffer for the received data. */
	int			(*cb_recv_ow)(struct nl_sock *,
					      struct sockaddr_nl *,
					      unsigned char **,
					      struct ucred **);

	/** Overwrites internal calls to nl_send, must send the netlink
	 * message. */
	int			(*cb_send_ow)(struct nl_sock *,
					      struct nl_msg *);

	int			cb_refcnt;
	/** indicates the callback that is currently active */
	enum nl_cb_type		cb_active;
};

struct nl_sock
{
	struct sockaddr_nl	s_local;
	struct sockaddr_nl	s_peer;
	int			s_fd;
	int			s_proto;
	unsigned int		s_seq_next;
	unsigned int		s_seq_expect;
	int			s_flags;
	struct nl_cb *		s_cb;
	size_t			s_bufsize;
};

struct nl_cache
{
	struct nl_list_head	c_items;
	int			c_nitems;
	int                     c_iarg1;
	int                     c_iarg2;
	int			c_refcnt;
	unsigned int		c_flags;
	struct nl_hash_table *	hashtable;
	struct nl_cache_ops *   c_ops;
};

struct nl_cache_assoc
{
	struct nl_cache *	ca_cache;
	change_func_t		ca_change;
	void *			ca_change_data;
};

struct nl_cache_mngr
{
	int			cm_protocol;
	int			cm_flags;
	int			cm_nassocs;
	struct nl_sock *	cm_sock;
	struct nl_sock *	cm_sync_sock;
	struct nl_cache_assoc *	cm_assocs;
};

struct nl_parser_param;

#define LOOSE_COMPARISON	1

#define NL_OBJ_MARK		1

struct nl_data
{
	size_t			d_size;
	void *			d_data;
};

struct nl_addr
{
	int			a_family;
	unsigned int		a_maxsize;
	unsigned int		a_len;
	int			a_prefixlen;
	int			a_refcnt;
	char			a_addr[0];
};

struct nl_msg
{
	int			nm_protocol;
	int			nm_flags;
	struct sockaddr_nl	nm_src;
	struct sockaddr_nl	nm_dst;
	struct ucred		nm_creds;
	struct nlmsghdr *	nm_nlh;
	size_t			nm_size;
	int			nm_refcnt;
};

struct rtnl_link_map
{
	uint64_t lm_mem_start;
	uint64_t lm_mem_end;
	uint64_t lm_base_addr;
	uint16_t lm_irq;
	uint8_t  lm_dma;
	uint8_t  lm_port;
};

#define IFQDISCSIZ	32

struct rtnl_link
{
	NLHDR_COMMON

	char				l_name[IFNAMSIZ];
	uint32_t			l_family;
	uint32_t			l_arptype;
	uint32_t			l_index;
	uint32_t			l_flags;
	uint32_t			l_change;
	uint32_t 			l_mtu;
	uint32_t			l_link;
	uint32_t			l_txqlen;
	uint32_t			l_weight;
	uint32_t			l_master;
	struct nl_addr *		l_addr;
	struct nl_addr *		l_bcast;
	char				l_qdisc[IFQDISCSIZ];
	struct rtnl_link_map		l_map;
	uint64_t			l_stats[RTNL_LINK_STATS_MAX+1];
	uint32_t			l_flag_mask;
	uint32_t			l_num_vf;
	uint8_t				l_operstate;
	uint8_t				l_linkmode;
	/* 2 byte hole */
	char *				l_info_kind;
	struct rtnl_link_info_ops *	l_info_ops;
	void *				l_af_data[AF_MAX];
	void *				l_info;
	char *				l_ifalias;
	uint32_t			l_promiscuity;
	uint32_t			l_num_tx_queues;
	uint32_t			l_num_rx_queues;
	uint32_t			l_group;
	uint8_t				l_carrier;
	/* 3 byte hole */
	struct rtnl_link_af_ops *	l_af_ops;
	struct nl_data *		l_phys_port_id;
	int				l_ns_fd;
	pid_t				l_ns_pid;
};

struct rtnl_ncacheinfo
{
	uint32_t nci_confirmed;	/**< Time since neighbour validty was last confirmed */
	uint32_t nci_used;	/**< Time since neighbour entry was last ued */
	uint32_t nci_updated;	/**< Time since last update */
	uint32_t nci_refcnt;	/**< Reference counter */
};


struct rtnl_neigh
{
	NLHDR_COMMON
	uint32_t	n_family;
	uint32_t	n_ifindex;
	uint16_t	n_state;
	uint8_t		n_flags;
	uint8_t		n_type;
	struct nl_addr *n_lladdr;
	struct nl_addr *n_dst;
	uint32_t	n_probes;
	struct rtnl_ncacheinfo n_cacheinfo;
	uint32_t                n_state_mask;
	uint32_t                n_flag_mask;
	uint32_t		n_master;
};


struct rtnl_addr_cacheinfo
{
	/* Preferred lifetime in seconds, ticking from when the message gets constructed */
	uint32_t aci_prefered;

	/* Valid lifetime in seconds, ticking from when the message gets constructed */
	uint32_t aci_valid;

	/* Timestamp of creation in 1/100s since boottime, clock_gettime(CLOCK_MONOTONIC) */
	uint32_t aci_cstamp;

	/* Timestamp of last update in 1/100s since boottime, clock_gettime(CLOCK_MONOTONIC) */
	uint32_t aci_tstamp;
};

struct rtnl_addr
{
	NLHDR_COMMON

	uint8_t		a_family;
	uint8_t		a_prefixlen;
	uint8_t		a_scope;
	uint32_t	a_flags;
	uint32_t	a_ifindex;

	struct nl_addr *a_peer;
	struct nl_addr *a_local;
	struct nl_addr *a_bcast;
	struct nl_addr *a_anycast;
	struct nl_addr *a_multicast;

	struct rtnl_addr_cacheinfo a_cacheinfo;

	char a_label[IFNAMSIZ];
	uint32_t a_flag_mask;
	struct rtnl_link *a_link;
};

struct rtnl_nexthop
{
	uint8_t			rtnh_flags;
	uint8_t			rtnh_flag_mask;
	uint8_t			rtnh_weight;
	/* 1 byte spare */
	uint32_t		rtnh_ifindex;
	struct nl_addr *	rtnh_gateway;
	uint32_t		ce_mask; /* HACK to support attr macros */
	struct nl_list_head	rtnh_list;
	uint32_t		rtnh_realms;
};

struct rtnl_route
{
	NLHDR_COMMON

	uint8_t			rt_family;
	uint8_t			rt_dst_len;
	uint8_t			rt_src_len;
	uint8_t			rt_tos;
	uint8_t			rt_protocol;
	uint8_t			rt_scope;
	uint8_t			rt_type;
	uint8_t			rt_nmetrics;
	uint32_t		rt_flags;
	struct nl_addr *	rt_dst;
	struct nl_addr *	rt_src;
	uint32_t		rt_table;
	uint32_t		rt_iif;
	uint32_t		rt_prio;
	uint32_t		rt_metrics[RTAX_MAX];
	uint32_t		rt_metrics_mask;
	uint32_t		rt_nr_nh;
	struct nl_addr *	rt_pref_src;
	struct nl_list_head	rt_nexthops;
	struct rtnl_rtcacheinfo	rt_cacheinfo;
	uint32_t		rt_flag_mask;
};

struct rtnl_rule
{
	NLHDR_COMMON
	uint8_t		r_family;
	uint8_t		r_action;
	uint8_t		r_dsfield; /* ipv4 only */
	uint8_t		r_unused;
	uint32_t	r_table;
	uint32_t	r_flags;
	uint32_t	r_prio;
	uint32_t	r_mark;
	uint32_t	r_mask;
	uint32_t	r_goto;
	uint32_t	r_flow; /* ipv4 only */
	struct nl_addr *r_src;
	struct nl_addr *r_dst;
	char		r_iifname[IFNAMSIZ];
	char		r_oifname[IFNAMSIZ];
};

struct rtnl_neightbl_parms
{
	/**
	 * Interface index of the device this parameter set is assigned
	 * to or 0 for the default set.
	 */
	uint32_t		ntp_ifindex;

	/**
	 * Number of references to this parameter set.
	 */
	uint32_t		ntp_refcnt;

	/**
	 * Queue length for pending arp requests, i.e. the number of
	 * packets which are accepted from other layers while the
	 * neighbour address is still being resolved
	 */
	uint32_t		ntp_queue_len;

	/**
	 * Number of requests to send to the user level ARP daemon.
	 * Specify 0 to disable.
	 */
	uint32_t		ntp_app_probes;

	/**
	 * Maximum number of retries for unicast solicitation.
	 */
	uint32_t		ntp_ucast_probes;

	/**
	 * Maximum number of retries for multicast solicitation.
	 */
	uint32_t		ntp_mcast_probes;

	/**
	 * Base value in milliseconds to ompute reachable time, see RFC2461.
	 */
	uint64_t		ntp_base_reachable_time;

	/**
	 * Actual reachable time (read-only)
	 */
	uint64_t		ntp_reachable_time;	/* secs */

	/**
	 * The time in milliseconds between retransmitted Neighbor
	 * Solicitation messages.
	 */
	uint64_t		ntp_retrans_time;

	/**
	 * Interval in milliseconds to check for stale neighbour
	 * entries.
	 */
	uint64_t		ntp_gc_stale_time;	/* secs */

	/**
	 * Delay in milliseconds for the first time probe if
	 * the neighbour is reachable.
	 */
	uint64_t		ntp_probe_delay;	/* secs */

	/**
	 * Maximum delay in milliseconds of an answer to a neighbour
	 * solicitation message.
	 */
	uint64_t		ntp_anycast_delay;

	/**
	 * Minimum age in milliseconds before a neighbour entry
	 * may be replaced.
	 */
	uint64_t		ntp_locktime;

	/**
	 * Delay in milliseconds before answering to an ARP request
	 * for which a proxy ARP entry exists.
	 */
	uint64_t		ntp_proxy_delay;

	/**
	 * Queue length for the delayed proxy arp requests.
	 */
	uint32_t		ntp_proxy_qlen;

	/**
	 * Mask of available parameter attributes
	 */
	uint32_t		ntp_mask;
};

#define NTBLNAMSIZ	32

/**
 * Neighbour table
 * @ingroup neightbl
 */
struct rtnl_neightbl
{
	NLHDR_COMMON

	char			nt_name[NTBLNAMSIZ];
	uint32_t		nt_family;
	uint32_t		nt_gc_thresh1;
	uint32_t		nt_gc_thresh2;
	uint32_t		nt_gc_thresh3;
	uint64_t		nt_gc_interval;
	struct ndt_config	nt_config;
	struct rtnl_neightbl_parms nt_parms;
	struct ndt_stats	nt_stats;
};

struct rtnl_ratespec
{
	uint8_t			rs_cell_log;
	uint16_t		rs_overhead;
	int16_t			rs_cell_align;
	uint16_t		rs_mpu;
	uint32_t		rs_rate;
};

struct rtnl_tstats
{
	struct {
		uint64_t            bytes;
		uint64_t            packets;
	} tcs_basic;

	struct {
		uint32_t            bps;
		uint32_t            pps;
	} tcs_rate_est;

	struct {
		uint32_t            qlen;
		uint32_t            backlog;
		uint32_t            drops;
		uint32_t            requeues;
		uint32_t            overlimits;
	} tcs_queue;
};

#define TCKINDSIZ	32

#define NL_TC_GENERIC(pre)				\
	NLHDR_COMMON					\
	uint32_t		pre ##_family;		\
	uint32_t		pre ##_ifindex;		\
	uint32_t		pre ##_handle;		\
	uint32_t		pre ##_parent;		\
	uint32_t		pre ##_info;		\
	uint32_t		pre ##_mtu;		\
	uint32_t		pre ##_mpu;		\
	uint32_t		pre ##_overhead;	\
	uint32_t		pre ##_linktype;	\
	char			pre ##_kind[TCKINDSIZ];	\
	struct nl_data *	pre ##_opts;		\
	uint64_t		pre ##_stats[RTNL_TC_STATS_MAX+1]; \
	struct nl_data *	pre ##_xstats;		\
	struct nl_data *	pre ##_subdata;		\
	struct rtnl_link *	pre ##_link;		\
	struct rtnl_tc_ops *	pre ##_ops;		\
	enum rtnl_tc_type	pre ##_type

struct rtnl_tc
{
	NL_TC_GENERIC(tc);
};

struct rtnl_qdisc
{
	NL_TC_GENERIC(q);
};

struct rtnl_class
{
	NL_TC_GENERIC(c);
};

struct rtnl_cls
{
	NL_TC_GENERIC(c);
	uint16_t		c_prio;
	uint16_t		c_protocol;
};

struct rtnl_act
{
	NL_TC_GENERIC(c);
	struct rtnl_act *	a_next;
};

struct rtnl_mirred
{
	struct tc_mirred m_parm;
};

struct rtnl_u32
{
	uint32_t		cu_divisor;
	uint32_t		cu_hash;
	uint32_t		cu_classid;
	uint32_t		cu_link;
	struct nl_data *	cu_pcnt;
	struct nl_data *	cu_selector;
	struct rtnl_act*	cu_act;
	struct nl_data *	cu_police;
	char			cu_indev[IFNAMSIZ];
	int			cu_mask;
};

struct rtnl_cgroup
{
	struct rtnl_ematch_tree *cg_ematch;
	int			cg_mask;
};

struct rtnl_fw
{
	uint32_t		cf_classid;
	struct nl_data *	cf_act;
	struct nl_data *	cf_police;
	char			cf_indev[IFNAMSIZ];
	uint32_t		cf_fwmask;
	int			cf_mask;
};

struct rtnl_ematch
{
	uint16_t		e_id;
	uint16_t		e_kind;
	uint16_t		e_flags;
	uint16_t		e_index;
	size_t			e_datalen;

	struct nl_list_head	e_childs;
	struct nl_list_head	e_list;
	struct rtnl_ematch_ops *e_ops;

	void *			e_data;
};

struct rtnl_ematch_tree
{
	uint16_t		et_progid;
	struct nl_list_head	et_list;

};

struct rtnl_dsmark_qdisc
{
	uint16_t	qdm_indices;
	uint16_t	qdm_default_index;
	uint32_t	qdm_set_tc_index;
	uint32_t	qdm_mask;
};

struct rtnl_dsmark_class
{
	uint8_t		cdm_bmask;
	uint8_t		cdm_value;
	uint32_t	cdm_mask;
};

struct rtnl_fifo
{
	uint32_t	qf_limit;
	uint32_t	qf_mask;
};

struct rtnl_prio
{
	uint32_t	qp_bands;
	uint8_t		qp_priomap[TC_PRIO_MAX+1];
	uint32_t	qp_mask;
};

struct rtnl_tbf
{
	uint32_t		qt_limit;
	struct rtnl_ratespec	qt_rate;
	uint32_t		qt_rate_bucket;
	uint32_t		qt_rate_txtime;
	struct rtnl_ratespec	qt_peakrate;
	uint32_t		qt_peakrate_bucket;
	uint32_t		qt_peakrate_txtime;
	uint32_t		qt_mask;
};

struct rtnl_sfq
{
	uint32_t	qs_quantum;
	uint32_t	qs_perturb;
	uint32_t	qs_limit;
	uint32_t	qs_divisor;
	uint32_t	qs_flows;
	uint32_t	qs_mask;
};

struct rtnl_netem_corr
{
	uint32_t	nmc_delay;
	uint32_t	nmc_loss;
	uint32_t	nmc_duplicate;
};

struct rtnl_netem_reo
{
	uint32_t	nmro_probability;
	uint32_t	nmro_correlation;
};

struct rtnl_netem_crpt
{
	uint32_t	nmcr_probability;
	uint32_t	nmcr_correlation;
};

struct rtnl_netem_dist
{
	int16_t	*	dist_data;
	size_t		dist_size;
};

struct rtnl_netem
{
	uint32_t		qnm_latency;
	uint32_t		qnm_limit;
	uint32_t		qnm_loss;
	uint32_t		qnm_gap;
	uint32_t		qnm_duplicate;
	uint32_t		qnm_jitter;
	uint32_t		qnm_mask;
	struct rtnl_netem_corr	qnm_corr;
	struct rtnl_netem_reo	qnm_ro;
	struct rtnl_netem_crpt	qnm_crpt;
	struct rtnl_netem_dist  qnm_dist;
};

struct rtnl_htb_qdisc
{
	uint32_t		qh_rate2quantum;
	uint32_t		qh_defcls;
	uint32_t		qh_mask;
	uint32_t		qh_direct_pkts;
};

struct rtnl_htb_class
{
	uint32_t		ch_prio;
	struct rtnl_ratespec	ch_rate;
	struct rtnl_ratespec	ch_ceil;
	uint32_t		ch_rbuffer;
	uint32_t		ch_cbuffer;
	uint32_t		ch_quantum;
	uint32_t		ch_mask;
	uint32_t		ch_level;
};

struct rtnl_cbq
{
	struct tc_cbq_lssopt    cbq_lss;
	struct tc_ratespec      cbq_rate;
	struct tc_cbq_wrropt    cbq_wrr;
	struct tc_cbq_ovl       cbq_ovl;
	struct tc_cbq_fopt      cbq_fopt;
	struct tc_cbq_police    cbq_police;
};

struct rtnl_red
{
	uint32_t	qr_limit;
	uint32_t	qr_qth_min;
	uint32_t	qr_qth_max;
	uint8_t		qr_flags;
	uint8_t		qr_wlog;
	uint8_t		qr_plog;
	uint8_t		qr_scell_log;
	uint32_t	qr_mask;
};

struct rtnl_plug
{
	int             action;
	uint32_t        limit;
};

struct rtnl_fq_codel
{
	int		fq_limit;
	uint32_t	fq_target;
	uint32_t	fq_interval;
	int		fq_flows;
	uint32_t	fq_quantum;
	int		fq_ecn;
	uint32_t	fq_mask;
};

struct flnl_request
{
	NLHDR_COMMON

	struct nl_addr *	lr_addr;
	uint32_t		lr_fwmark;
	uint8_t			lr_tos;
	uint8_t			lr_scope;
	uint8_t			lr_table;
};


struct flnl_result
{
	NLHDR_COMMON

	struct flnl_request *	fr_req;
	uint8_t			fr_table_id;
	uint8_t			fr_prefixlen;
	uint8_t			fr_nh_sel;
	uint8_t			fr_type;
	uint8_t			fr_scope;
	uint32_t		fr_error;
};

#define GENL_OP_HAS_POLICY	1
#define GENL_OP_HAS_DOIT	2
#define GENL_OP_HAS_DUMPIT	4

struct genl_family_op
{
	uint32_t		o_id;
	uint32_t		o_flags;

	struct nl_list_head	o_list;
};

struct genl_family_grp {
        struct genl_family      *family;        /* private */
        struct nl_list_head     list;           /* private */
        char                    name[GENL_NAMSIZ];
        u_int32_t               id;
};

struct genl_family
{
	NLHDR_COMMON

	uint16_t		gf_id;
	char 			gf_name[GENL_NAMSIZ];
	uint32_t		gf_version;
	uint32_t		gf_hdrsize;
	uint32_t		gf_maxattr;

	struct nl_list_head	gf_ops;
	struct nl_list_head	gf_mc_grps;
};

union nfnl_ct_proto
{
	struct {
		uint16_t	src;
		uint16_t	dst;
	} port;
	struct {
		uint16_t	id;
		uint8_t		type;
		uint8_t		code;
	} icmp;
};

struct nfnl_ct_dir {
	struct nl_addr *	src;
	struct nl_addr *	dst;
	union nfnl_ct_proto	proto;
	uint64_t		packets;
	uint64_t		bytes;
};

union nfnl_ct_protoinfo {
	struct {
		uint8_t		state;
	} tcp;
};

struct nfnl_ct {
	NLHDR_COMMON

	uint8_t			ct_family;
	uint8_t			ct_proto;
	union nfnl_ct_protoinfo	ct_protoinfo;

	uint32_t		ct_status;
	uint32_t		ct_status_mask;
	uint32_t		ct_timeout;
	uint32_t		ct_mark;
	uint32_t		ct_use;
	uint32_t		ct_id;
	uint16_t		ct_zone;

	struct nfnl_ct_dir	ct_orig;
	struct nfnl_ct_dir	ct_repl;

	struct nfnl_ct_timestamp ct_tstamp;
};

union nfnl_exp_protodata {
	struct {
		uint16_t	src;
		uint16_t	dst;
	} port;
	struct {
		uint16_t	id;
		uint8_t		type;
		uint8_t		code;
	} icmp;
};

// Allow for different master/expect l4 protocols
struct nfnl_exp_proto
{
	uint8_t						l4protonum;
	union nfnl_exp_protodata	l4protodata;
};

struct nfnl_exp_dir {
	struct nl_addr *		src;
	struct nl_addr *		dst;
	struct nfnl_exp_proto	proto;
};

struct nfnl_exp {
	NLHDR_COMMON

	uint8_t			exp_family;
	uint32_t		exp_timeout;
	uint32_t		exp_id;
	uint16_t		exp_zone;
	uint32_t		exp_class;
	uint32_t		exp_flags;
	char *			exp_helper_name;
	char *			exp_fn;
	uint8_t			exp_nat_dir;

	struct nfnl_exp_dir		exp_expect;
	struct nfnl_exp_dir		exp_master;
	struct nfnl_exp_dir		exp_mask;
	struct nfnl_exp_dir		exp_nat;
};

struct nfnl_log {
	NLHDR_COMMON

	uint16_t		log_group;
	uint8_t			log_copy_mode;
	uint32_t		log_copy_range;
	uint32_t		log_flush_timeout;
	uint32_t		log_alloc_size;
	uint32_t		log_queue_threshold;
	uint32_t		log_flags;
	uint32_t		log_flag_mask;
};

struct nfnl_log_msg {
	NLHDR_COMMON

	uint8_t			log_msg_family;
	uint8_t			log_msg_hook;
	uint16_t		log_msg_hwproto;
	uint32_t		log_msg_mark;
	struct timeval		log_msg_timestamp;
	uint32_t		log_msg_indev;
	uint32_t		log_msg_outdev;
	uint32_t		log_msg_physindev;
	uint32_t		log_msg_physoutdev;
	uint8_t			log_msg_hwaddr[8];
	int			log_msg_hwaddr_len;
	void *			log_msg_payload;
	int			log_msg_payload_len;
	char *			log_msg_prefix;
	uint32_t		log_msg_uid;
	uint32_t		log_msg_gid;
	uint32_t		log_msg_seq;
	uint32_t		log_msg_seq_global;
};

struct nfnl_queue {
	NLHDR_COMMON

	uint16_t		queue_group;
	uint32_t		queue_maxlen;
	uint32_t		queue_copy_range;
	uint8_t			queue_copy_mode;
};

struct nfnl_queue_msg {
	NLHDR_COMMON

	uint16_t		queue_msg_group;
	uint8_t			queue_msg_family;
	uint8_t			queue_msg_hook;
	uint16_t		queue_msg_hwproto;
	uint32_t		queue_msg_packetid;
	uint32_t		queue_msg_mark;
	struct timeval		queue_msg_timestamp;
	uint32_t		queue_msg_indev;
	uint32_t		queue_msg_outdev;
	uint32_t		queue_msg_physindev;
	uint32_t		queue_msg_physoutdev;
	uint8_t			queue_msg_hwaddr[8];
	int			queue_msg_hwaddr_len;
	void *			queue_msg_payload;
	int			queue_msg_payload_len;
	uint32_t		queue_msg_verdict;
};

struct ematch_quoted {
	char *	data;
	size_t	len;
	int	index;
};

struct idiagnl_meminfo {
	NLHDR_COMMON

	uint32_t idiag_rmem;
	uint32_t idiag_wmem;
	uint32_t idiag_fmem;
	uint32_t idiag_tmem;
};

struct idiagnl_vegasinfo {
	NLHDR_COMMON

	uint32_t tcpv_enabled;
	uint32_t tcpv_rttcnt;
	uint32_t tcpv_rtt;
	uint32_t tcpv_minrtt;
};

struct idiagnl_msg {
	NLHDR_COMMON

	uint8_t			    idiag_family;
	uint8_t			    idiag_state;
	uint8_t			    idiag_timer;
	uint8_t			    idiag_retrans;
	uint16_t		    idiag_sport;
	uint16_t		    idiag_dport;
	struct nl_addr *	    idiag_src;
	struct nl_addr *	    idiag_dst;
	uint32_t		    idiag_ifindex;
	uint32_t		    idiag_expires;
	uint32_t		    idiag_rqueue;
	uint32_t		    idiag_wqueue;
	uint32_t		    idiag_uid;
	uint32_t		    idiag_inode;

	uint8_t			    idiag_tos;
	uint8_t			    idiag_tclass;
	uint8_t			    idiag_shutdown;
	char *			    idiag_cong;
	struct idiagnl_meminfo *    idiag_meminfo;
	struct idiagnl_vegasinfo *  idiag_vegasinfo;
	struct tcp_info		    idiag_tcpinfo;
	uint32_t		    idiag_skmeminfo[IDIAG_SK_MEMINFO_VARS];
};

struct idiagnl_req {
	NLHDR_COMMON

	uint8_t			idiag_family;
	uint8_t			idiag_ext;
	struct nl_addr *	idiag_src;
	struct nl_addr *	idiag_dst;
	uint32_t		idiag_ifindex;
	uint32_t		idiag_states;
	uint32_t		idiag_dbs;
};
#endif
