
#include "libnetlink.h"
#include <linux/tc_act/tc_gact.h>

static int tc_insert_gact(int action, int tca_id, struct nlmsghdr *n)
{
    struct tc_gact p = { 0 };
    struct rtattr *tail;

    p.action = action;

    tail = addattr_nest(n, MAX_MSG, tca_id);
    addattr_l(n, MAX_MSG, TCA_GACT_PARMS, &p, sizeof(p));
    addattr_nest_end(n, tail);

    return 0;
}

static int tc_insert_actions(int tca_id, struct nlmsghdr *n, int iq_prio, int action, uint16_t reason)
{
    struct rtattr *tail, *tail2;
    const char *gact = "gact";
    int action_prio = 1;
    
    tail2 = addattr_nest(n, MAX_MSG, tca_id);
    
    if (iq_prio== 1)
    {
        tail = addattr_nest(n, MAX_MSG, action_prio++);
        addattr_l(n, MAX_MSG, TCA_ACT_KIND, gact, strlen(gact) + 1);
        tc_insert_gact(TC_ACT_IQ_PRIO | 1, TCA_ACT_OPTIONS | NLA_F_NESTED, n);
        addattr_nest_end(n, tail);
    }

    if (action == 2)
    {
        tail = addattr_nest(n, MAX_MSG, action_prio++);
        addattr_l(n, MAX_MSG, TCA_ACT_KIND, gact, strlen(gact) + 1);
        tc_insert_gact(TC_ACT_RTRAP | reason, TCA_ACT_OPTIONS | NLA_F_NESTED, n);
        addattr_nest_end(n, tail);
    }

    addattr_nest_end(n, tail2);
    return 0;
}

static int ip_proto2flower_id(uint16_t ip_proto)
{
    switch (ip_proto)
    {
    case IPPROTO_UDP:
        return TCA_FLOWER_KEY_UDP_DST;
    case IPPROTO_TCP:
        return TCA_FLOWER_KEY_TCP_DST;
    default:
        TC_NL_ERROR(-EINVAL, "ip_proto %d not handled", ip_proto);
        break;
    }

    return TCA_FLOWER_UNSPEC;
}

static int tc_flower_parse_fopt(struct nlmsghdr *n, uint16_t ip_proto, uint16_t port, int iq_prio,
    int action, uint16_t reason)
{
    int ret;
    struct rtattr *tail;
    struct tcmsg *t = NLMSG_DATA(n);
    __be16 tc_proto = TC_H_MIN(t->tcm_info);

    tail = (struct rtattr *) (((void *) n) + NLMSG_ALIGN(n->nlmsg_len));
    addattr_l(n, MAX_MSG, TCA_OPTIONS, NULL, 0);

    /* ip_proto */
    addattr8(n, MAX_MSG, TCA_FLOWER_KEY_IP_PROTO, ip_proto);
    /* dst_port */
    addattr16(n, MAX_MSG, ip_proto2flower_id(ip_proto), htons(port));

    tc_insert_actions(TCA_FLOWER_ACT, n, iq_prio, action, reason);

    addattr32(n, MAX_MSG, TCA_FLOWER_FLAGS, TCA_CLS_FLAGS_SKIP_SW);

    if (tc_proto != htons(ETH_P_ALL)) {
        ret = addattr16(n, MAX_MSG, TCA_FLOWER_KEY_ETH_TYPE, tc_proto);
        if (ret)
            return ret;
    }

    tail->rta_len = (((void *)n)+n->nlmsg_len) - (void *)tail;

    return 0;
}

static int tc_add_flower_filter(struct rtnl_handle *rth, int if_index, uint16_t filter_type, uint16_t filter_flags,
    uint16_t filter_prio, int chain_index, int iq_prio, uint16_t protocol, uint16_t ip_proto,
    uint16_t port, int action, uint16_t reason)
{
    struct {
        struct nlmsghdr n;
        struct tcmsg        t;
        char            buf[MAX_MSG];
    } req = {
        .n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcmsg)),
        .n.nlmsg_flags = NLM_F_REQUEST | NLM_F_EXCL | filter_flags,
        .n.nlmsg_type = filter_type,
        .t.tcm_family = AF_UNSPEC,
    };
    const char *k = "flower";

    /* prio & protocol */
    req.t.tcm_info = TC_H_MAKE(filter_prio<<16, htons(protocol));
    /* ingresss */
    req.t.tcm_parent = TC_H_MAKE(TC_H_CLSACT, TC_H_MIN_INGRESS);
    /* chain index */
    addattr32(&req.n, sizeof(req), TCA_CHAIN, chain_index);
    /* filter */
    addattr_l(&req.n, sizeof(req), TCA_KIND, k, strlen(k)+1);

    req.t.tcm_ifindex = if_index;

    if (tc_flower_parse_fopt(&req.n, ip_proto, port, iq_prio, action, reason))
        return -1;

    if (rtnl_talk(rth, &req.n) < 0) {
        TC_NL_PRINT("We have an error talking to the kernel\n");
        return 2;
    }

    return 0;
}

static int _tc_insert_L4port(uint16_t filter_type, uint16_t filter_flags, const char *if_name,
    uint16_t filter_prio, int chain_index, int iq_prio, uint16_t protocol, uint16_t ip_proto,
    uint16_t port, int action, uint16_t reason)
{
    int rc;
    struct rtnl_handle rth;
    struct net_device *dev = dev_get_by_name(&init_net, if_name);

    if (!dev)
        return -1;

    rc = rtnl_open(&rth, 0);
    if (rc)
    {
        TC_NL_ERROR(rc, "open failed");
        return rc;
    }

    rc = tc_add_flower_filter(&rth, dev->ifindex, filter_type, filter_flags, filter_prio, chain_index,
        iq_prio, protocol, ip_proto, port, action, reason);
    if (rc)
    {
        TC_NL_ERROR(rc, "do failed");
        return rc;
    }
    
    rtnl_close(&rth);
    return 0;
}

int tc_insert_L4port(const char *if_name, uint16_t filter_prio, int chain_index, int iq_prio,
    uint16_t ip_proto, uint16_t port, int action, uint16_t reason)
{
    int rc;

    rc = _tc_insert_L4port(RTM_NEWTFILTER, NLM_F_CREATE, if_name, filter_prio, chain_index,
        iq_prio, ETH_P_IP, ip_proto, port, action, reason);
    rc = rc ? rc : _tc_insert_L4port(RTM_NEWTFILTER, NLM_F_CREATE, if_name, filter_prio + 1, chain_index,
        iq_prio, ETH_P_IPV6, ip_proto, port, action, reason);

    return rc;
}

EXPORT_SYMBOL(tc_insert_L4port);

int tc_remove_L4port(const char *if_name, uint16_t filter_prio, int chain_index,
    uint16_t ip_proto, uint16_t port, int action, uint16_t reason)
{
    int rc;

    rc = _tc_insert_L4port(RTM_DELTFILTER, 0, if_name, filter_prio, chain_index, 0,
        ETH_P_IP, ip_proto, port, action, reason);
    rc = rc ? rc : _tc_insert_L4port(RTM_DELTFILTER, 0, if_name, filter_prio + 1, chain_index, 0,
        ETH_P_IPV6, ip_proto, port, action, reason);

    return rc;
}

EXPORT_SYMBOL(tc_remove_L4port);
