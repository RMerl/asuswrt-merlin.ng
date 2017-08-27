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

#ifndef __UDB_COMMON_H__
#define __UDB_COMMON_H__

typedef enum
{
	TDTS_HOOK_NF_FORD = 0
	, TDTS_HOOK_NF_LIN
	, TDTS_HOOK_NF_LOUT
	, TDTS_HOOK_NF_PRE_CTF
	, TDTS_HOOK_NF_PRE_FC
	, TDTS_HOOK_NF_FORD_RTL
} tdts_hook_t;

typedef enum
{
	TDTS_RES_DROP = 0
	, TDTS_RES_ACCEPT
	, TDTS_RES_STOLEN
	, TDTS_RES_CONTINUE
} tdts_res_t;

typedef enum
{
	TDTS_ACT_SCAN = 0
	, TDTS_ACT_BYPASS
	, TDTS_ACT_DROP
} tdts_act_t;

typedef enum
{
	TDTS_UDB_CT_EVT_NEW = 0
	, TDTS_UDB_CT_EVT_DESTROY
} tdts_udb_ct_evt_t;

typedef enum
{
	SKB_FLAG_UPLOAD = 0
	, SKB_FLAG_LOOPBACK
	, SKB_FLAG_L2_HEADER
	, SKB_FLAG_L3_HEADER
	, SKB_FLAG_L4_HEADER
	, SKB_FLAG_ICMP_DEST_UNREACHABLE
	, SKB_FLAG_MAX
} skb_flag_t;

#define SET_SKB_UPLOAD(f, b) 	do { if (b) { (f) |= (1 << SKB_FLAG_UPLOAD); } else { (f) &= ~(1 << SKB_FLAG_UPLOAD); } } while (0)
#define SET_SKB_LOOPBACK(f, b)	do { if (b) { (f) |= (1 << SKB_FLAG_LOOPBACK); } else { (f) &= ~(1 << SKB_FLAG_LOOPBACK); } } while (0)
#define SET_SKB_L2_HEADER(f, b)	do { if	(b) { (f) |= (1 << SKB_FLAG_L2_HEADER); } else { (f) &= ~(1 << SKB_FLAG_L2_HEADER); } } while (0)
#define SET_SKB_L3_HEADER(f, b)	do { if	(b) { (f) |= (1 << SKB_FLAG_L3_HEADER); } else { (f) &= ~(1 << SKB_FLAG_L3_HEADER); } } while (0)
#define SET_SKB_L4_HEADER(f, b)	do { if	(b) { (f) |= (1 << SKB_FLAG_L4_HEADER); } else { (f) &= ~(1 << SKB_FLAG_L4_HEADER); } } while (0)
#define SET_SKB_ICMP_DEST_UNREACHABLE(f, b) do { if	(b) { (f) |= (1 << SKB_FLAG_ICMP_DEST_UNREACHABLE); } else { (f) &= ~(1 << SKB_FLAG_ICMP_DEST_UNREACHABLE); } } while (0)

#define IS_SKB_UPLOAD(f)		((f) & (1 << SKB_FLAG_UPLOAD))
#define IS_SKB_LOOPBACK(f)		((f) & (1 << SKB_FLAG_LOOPBACK))
#define IS_SKB_L2_HEADER(f)		((f) & (1 << SKB_FLAG_L2_HEADER))
#define IS_SKB_L3_HEADER(f)		((f) & (1 << SKB_FLAG_L3_HEADER))
#define IS_SKB_L4_HEADER(f)		((f) & (1 << SKB_FLAG_L4_HEADER))
#define IS_SKB_ICMP_DEST_UNREACHABLE(f) ((f) & (1 << SKB_FLAG_ICMP_DEST_UNREACHABLE))

typedef enum
{
	SKB_TYPE_INVL = -1
	, SKB_TYPE_MISC
	, SKB_TYPE_DHCP
	, SKB_TYPE_DNS
	, SKB_TYPE_TCP_SYN
} skb_type_t;

typedef enum
{
	SKB_ETH_P_OTHER = 0
	, SKB_ETH_P_IP
	, SKB_ETH_P_IPV6
} skb_eth_proto_t;

typedef struct usr_msg_header
{
	int msg_len;
	int usr_pid;
	int msg_type;
} usr_msg_hdr_t;

typedef struct skb_tuples
{
	uint8_t ip_ver;
	uint8_t proto;

	uint16_t sport;
	uint16_t dport;

	union
	{
		uint8_t ipv4[4];
		uint8_t ipv6[16];
	} sip;

	union
	{
		uint8_t ipv4[4];
		uint8_t ipv6[16];
	} dip;

} skb_tuples_t;

enum
{
	CT_FLAG_NEW = 0
	, CT_FLAG_MCAST
	, CT_FLAG_LOCAL
	, CT_FLAG_UNTRACK
	, CT_FLAG_OTHER_PROTO
	, CT_FLAG_REPLY
	, CT_FLAG_MAX
};

#define SET_CT_NEW(f, b)	do { if (b) { (f) |= (1 << CT_FLAG_NEW); } else { (f) &= ~(1 << CT_FLAG_NEW); } } while (0)
#define SET_CT_MCAST(f, b)	do { if (b) { (f) |= (1 << CT_FLAG_MCAST); } else { (f) &= ~(1 << CT_FLAG_MCAST); } } while (0)
#define SET_CT_LOCAL(f, b)	do { if (b) { (f) |= (1 << CT_FLAG_LOCAL); } else { (f) &= ~(1 << CT_FLAG_LOCAL); } } while (0)
#define SET_CT_UNTRACK(f, b)	 do { if (b) { (f) |= (1 << CT_FLAG_UNTRACK); } else { (f) &= ~(1 << CT_FLAG_UNTRACK); } } while (0)
#define SET_CT_OTHER_PROTO(f, b) do { if (b) { (f) |= (1 << CT_FLAG_OTHER_PROTO); } else { (f) &= ~(1 << CT_FLAG_OTHER_PROTO); } } while (0)
#define SET_CT_REPLY(f, b)	do { if (b) { (f) |= (1 << CT_FLAG_REPLY); } else { (f) &= ~(1 << CT_FLAG_REPLY); } } while (0)

#define IS_CT_NEW(f)			((f) & (1 << CT_FLAG_NEW))
#define IS_CT_MCAST(f)			((f) & (1 << CT_FLAG_MCAST))
#define IS_CT_LOCAL(f)			((f) & (1 << CT_FLAG_LOCAL))
#define IS_CT_UNTRACK(f)		((f) & (1 << CT_FLAG_UNTRACK))
#define IS_CT_OTHER_PROTO(f)	((f) & (1 << CT_FLAG_OTHER_PROTO))
#define IS_CT_REPLY(f)			((f) & (1 << CT_FLAG_REPLY))

typedef struct tdts_net_device
{
	void		*fw_sk;
	void		*fw_indev;
	void		*fw_outdev;
	void		*fw_send;
} tdts_net_device_t;

typedef struct tdts_udb_param
{
	skb_tuples_t	skb_tuples;
	uint32_t	skb_flag;
	uint32_t	skb_len;
	uint32_t	skb_payload_len;
	skb_type_t	skb_type;
	skb_eth_proto_t	skb_eth_p;
	uint8_t		*skb_eth_src;
	void		*skb_dev;
	uint32_t	*skb_mark;
	void		*skb_ptr;

	uint32_t	ct_flag;
	uint32_t	*ct_mark;
	void		*ct_ptr;
	void		*ct_extra;
	void 		*fast_path_data;

	tdts_hook_t	hook;
	uint32_t	error_code;
	tdts_net_device_t dev;
	
	int		(*async_send)(void *, tdts_res_t);
	int		(*send_wrs_query_to_user)(usr_msg_hdr_t *, uint8_t *);
	int		(*send_redir_page)(void *, char *, int, char *, unsigned, char *, unsigned, uint8_t *, tdts_net_device_t *);
	int		(*send_wpr_page)(void *, char *, tdts_net_device_t *);
#if TMCFG_E_UDB_CORE_SHN_QUERY
	int		(*shnagent_cb)(usr_msg_hdr_t *, uint8_t *);
#endif
} tdts_udb_param_t;

enum
{
	BMP_TYPE_APP = 0,
	BMP_TYPE_CC,
	BMP_TYPE_SEC,
};

typedef int (*dpi_l3_scan_t)(void *skb, tdts_pkt_parameter_t *pkt_param);
#if TMCFG_E_UDB_CORE_RULE_FORMAT_V2
typedef int (*dpi_set_binding_ver_t)(unsigned int, unsigned int);
#endif

typedef struct
{
	char *wan_dev;
	unsigned int user_timeout;
	unsigned int app_timeout;
	unsigned int app_idle_time;
	dpi_l3_scan_t dpi_l3_scan;
#if TMCFG_E_UDB_CORE_RULE_FORMAT_V2
	dpi_set_binding_ver_t dpi_set_binding_ver;
#endif
#if TMCFG_E_CORE_PORT_SCAN_DETECTION
	unsigned short *port_scan_tholds;
#endif
} udb_init_param_t;

extern void udb_core_init_pkt_parameter(tdts_pkt_parameter_t *param, uint16_t req_flag, tdts_hook_t hook);

extern tdts_res_t udb_core_do_fastpath_action(tdts_udb_param_t *fw_param);
extern tdts_act_t udb_core_get_action(tdts_udb_param_t *fw_param, tdts_hook_t hook, bool new_conn);
extern tdts_res_t udb_core_policy_match(tdts_udb_param_t *fw_param);

extern int udb_core_udb_init(udb_init_param_t *udb_init_param);
extern void udb_core_udb_exit(void);
extern int udb_core_ct_extra_init(unsigned int sn);
extern void udb_core_ct_extra_exit(void);
extern void udb_core_ct_event_handler(void *conntrack, uint32_t mark, int ct_evt);

extern int tdts_core_ioctl_udb_op_get(char *tbl, uint32_t tbl_len, uint32_t *tbl_used_len);
extern int tdts_core_ioctl_dpi_conf_op_set(char *tbl, uint32_t tbl_len);
extern int tdts_core_ioctl_dpi_conf_op_get_value(char *buf, uint32_t tbl_len, uint32_t *tbl_used_len);

extern int tdts_core_ioctl_op_app_patrol_construct(char *tbl, uint32_t tbl_len);
extern int tdts_core_ioctl_app_op_get(char *tbl, uint32_t tbl_len, uint32_t *tbl_used_len);
extern int tdts_core_ioctl_app_patrol_list_get(char *tbl, uint32_t tbl_len, uint32_t *tbl_used_len);
extern int tdts_core_ioctl_app_bw_get_clean(char *tbl, uint32_t tbl_len, uint32_t *tbl_used_len);

extern int udb_core_set_dpi_cfg(unsigned int cfg);
extern int tdts_core_dpi_conf_seq_print(void *m, void *v);
extern int udb_read_procmem(void *m, void *v);
extern int tdts_core_fw_usr_msg_handler(uint8_t *msg, int size, int pid, int type);
extern int tdts_core_show_app_patrol(void *m, void *v);
extern int tdts_core_show_ford_drop(void *m, void *v);
extern int tdts_core_cte_stat_seq_print(void *m, void *v);

extern int tdts_core_update_devid_un_http_ua(uint8_t *addr, uint8_t ip_ver, uint8_t *data);
extern int tdts_core_update_devid_un_bootp(uint8_t *addr, uint8_t ip_ver, uint8_t *data);
extern int tdts_core_update_upnp(uint8_t *data, uint32_t index, void *cb);

extern int tdts_core_udb_mem_seq_print(void *m, void *v);
extern int tdts_core_udb_memtrack_print(void *m, void *v);

extern int tdts_core_ioctl_op_set_wpr_conf(char *buf, uint32_t tbl_len);
extern int tdts_core_ioctl_op_wpr_switch(uint32_t flag);
extern int tdts_core_wan_detection(uint8_t *dev_name, uint32_t len);
#endif	// __UDB_COMMON_H__

