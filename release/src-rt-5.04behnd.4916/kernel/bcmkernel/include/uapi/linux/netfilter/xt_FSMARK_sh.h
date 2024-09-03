#ifndef _XT_FSMARK_SH_H
#define _XT_FSMARK_SH_H

#if IS_ENABLED(CONFIG_BCM_NF_FSMARK)  // Flow Stats NF ID Mark
// shared inline functions for tc, iptables, ebtables

#include <linux/netfilter/xt_FSMARK.h>
#include <linux/bcm_log_mod.h>
#include <linux/bcm_log.h>

static inline void fsmark_count(struct sk_buff *skb, uint16_t id)
{
    bcmFun_t *update_slow_flow_stat = bcmFun_get(BCM_FUN_ID_FLOW_STAT_NF_UPDATE_SLOW);
    fsmark_slow_stat_ctxt ctxt;
    
    if (update_slow_flow_stat) 
    {
        ctxt.id = id;
        ctxt.len = skb->len;
        ctxt.is_mcast = (skb->pkt_type == PACKET_MULTICAST) ? 1 : 0;
        update_slow_flow_stat(&ctxt);
        //DEBUG_FSMARK("fc skb[%llx] blog[%llx] id=%d len=%d\n", (__u64)skb&0xffffffff, (__u64)(skb->blog_p)&0xffffffff, id, skb->len);
    }
}

static inline fsmark_cond_t *fsmark_get_cond_p(struct sk_buff *skb, uint16_t id)
{
    int ndx;
    for (ndx=0; ndx<MAX_NF_FSMARK_QUERY_PER_PKT; ndx++)
        if (skb->bcm_ext.fsmark_conds.a[ndx].conds && skb->bcm_ext.fsmark_conds.a[ndx].id == id)
            return &(skb->bcm_ext.fsmark_conds.a[ndx]);
    return NULL;
}

static inline fsmark_cond_t *fsmark_get_new_cond_p(struct sk_buff *skb)
{
    int ndx;
    for (ndx=0; ndx<MAX_NF_FSMARK_QUERY_PER_PKT; ndx++)
        if (skb->bcm_ext.fsmark_conds.a[ndx].conds == 0)
            return &(skb->bcm_ext.fsmark_conds.a[ndx]);
    return NULL;
}

static inline int fsmark_update(struct sk_buff *skb, uint16_t id)
{
	fsmark_cond_t *cond_p;
	if (id >= MAX_NF_FSMARK_QUERIES) {
        ERROR_FSMARK("query ID %d out of bound max (%d)!!\n", id, MAX_NF_FSMARK_QUERIES-1);
        return -1;
	}
	cond_p = fsmark_get_cond_p(skb, id);
    if (!cond_p) {
        cond_p = fsmark_get_new_cond_p(skb);
        if (!cond_p) {
            ERROR_FSMARK("too many queries match this packet max (%d)!!\n",
                MAX_NF_FSMARK_QUERY_PER_PKT);
            return -1;
        }
        cond_p->id = id;
        cond_p->conds = 1;
        fsmark_count(skb, id);
    }

    return 0;
}

#else
int fsmark_update(struct sk_buff *skb, uint16_t id) {return -1;}
#endif

#endif /*_XT_FSMARK_SH_H*/
