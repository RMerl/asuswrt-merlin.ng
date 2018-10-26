#if defined(CONFIG_BCM_KF_RUNNER)
#ifndef BL_OPS_H
#define BL_OPS_H

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RUNNER_RG) || defined(CONFIG_BCM_RUNNER_RG_MODULE)

#include <linux/types.h>
#include <net/netfilter/nf_conntrack.h>

struct bl_ops_t {
	void (*net_sched_sch_tbf_tbf_change)(void *_q, void *_sch, void *_qopt, int max_size);
	void (*net_sched_sch_tbf_tbf_destroy)(void *_q, void *_sch);
	void (*net_sched_sch_prio_prio_classify)(struct sk_buff *skb, u32 band);
        void (*net_netfilter_nf_conntrack_ftp)(struct nf_conn *ct, int ctinfo, struct nf_conntrack_expect *exp,
            int ftptype);
	void (*net_ipv4_netfilter_nf_nat_ftp)(struct nf_conn *ct, u_int16_t port, char buffer[], int ctinfo);
	void (*net_ipv4_netfilter_nf_nat_sip)(struct nf_conn *ct, u_int16_t port, int dir);
	void (*net_ipv4_netfilter_nf_nat_rtsp)(int num, int ctinfo, struct nf_conn *ct, u_int16_t loport,
            u_int16_t hiport);
	void (*net_ipv4_netfilter_ip_tables_check_match)(void *_m, void *_par, const void *_ip);
	void (*net_ipv4_netfilter_ip_tables___do_replace)(void *_oldinfo, void *_newinfo);
	void (*net_ipv4_netfilter_ip_tables_do_replace)(void *_oldinfo);
	void (*net_netfilter_xt_PORTTRIG_trigger_refresh)(void *_trig);
	void (*net_netfilter_xt_PORTTRIG_trigger_delete)(void *_trig);
	void (*net_netfilter_xt_PORTTRIG_trigger_new)(struct nf_conn *ct, __be32 srcip, __be32 dstip, __be16 port_start, __be16 port_end, __be16 protocol);
	void (*net_netfilter_nf_conntrack_core_destroy_conntrack)(struct nf_conn *ct);
	int (*net_netfilter_nf_conntrack_core_death_by_timeout)(struct nf_conn *ct);
	void (*net_netfilter_nf_conntrack_core_nf_conntrack_confirm)(struct nf_conn *ct, struct sk_buff  *skb);
	void (*net_netfilter_nf_conntrack_core_nf_conntrack_in)(struct nf_conn *ct, struct sk_buff  *skb);
	void (*net_netfilter_nf_conntrack_core_nf_conntrack_alloc)(struct nf_conn *ct);
	void (*net_netfilter_nf_conntrack_core_nf_conntrack_free)(struct nf_conn *ct);
};

#define BL_OPS(op)     { if (bl_ops) bl_ops->op; }
#define BL_OPS_CR(op)  { if (bl_ops && (bl_ops->op)) return; }

extern struct bl_ops_t *bl_ops;

#else /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */

#define BL_OPS(op)
#define BL_OPS_CR(op)

#endif /* CONFIG_BCM_RUNNER_RG || CONFIG_BCM_RUNNER_RG_MODULE */
#endif /* CONFIG_BCM_RUNNER */

#endif /* BL_OPS_H */
#endif /* CONFIG_BCM_KF_RUNNER */

