#include <linux/hashtable.h>
#include <linux/if_vlan.h>
#include <linux/jhash.h>
#include <rdpa_api.h>
#include <net/pkt_cls.h>
#include <net/tc_act/tc_gact.h>
#include <net/tc_act/tc_mirred.h>
#include <net/tc_act/tc_vlan.h>
#include <net/tc_act/tc_skbedit.h>

#include <linux/blog_rule.h>
#include <bcmenet_common.h>

#include "bcmenet_tc.h"
#include "enet.h"

static int crumbs_info_iterations = 111111;
static DECLARE_BITMAP(ds_priorities, RDPA_IC_MAX_PRIORITY + 1);
static DECLARE_BITMAP(us_priorities, RDPA_IC_MAX_PRIORITY + 1);

/*
 * This the incomplete definition of struct fl_flow_key which is defined
 * in net/sched/cls_flower.c.
 * Unfortunately there is no Linux kernel API to get indev_ifindex.
 */
struct fl_flow_key {
	int	indev_ifindex;
};

struct flow_ops {
    void (*dfo_init_flow)(struct net_device *txdev, rdpa_ic_info_t *flow, bdmf_object_handle vlan_obj);
    int (*dfo_set_port_action)(struct net_device *txdev, bdmf_object_handle ig_obj
                                    , bdmf_index flow_idx, bdmf_object_handle vlan_obj);
};

struct ic_entry {
    struct hlist_node ic_hn;
    rdpa_ic_cfg_t ic_cfg;
    rdpa_traffic_dir ic_dir;
    bdmf_object_handle ic_obj;
    int ic_refcount;
    int ic_prio;
};

struct flow_entry {
    struct hlist_node fe_hn;
    rdpa_ic_info_t fe_flow;
    bdmf_object_handle fe_ig_obj;
    bdmf_index fe_flow_idx;
    rdpa_stat_t fe_stat;
    uint64_t fe_last_used;
    int fe_refcount;
};

struct chain_filter {
    int cf_indev_ifindex;
    uint32_t cf_ic_fields;
    rdpa_ic_key_t cf_key;
};

enum chain_action_type { CA_EGRESS_REDIRECT, CA_INGRESS_REDIRECT, CA_EGRESS_MIRROR
            , CA_VLAN, CA_GOTO, CA_SET_QUEUE_ID, CA_SET_PORT, CA_DROP, CA_CONTINUE, CA_PASS };

struct chain_action {
    enum chain_action_type ca_type;
    union {
        rdpa_vlan_action_cfg_t ca_rdpa_vlan_action;
        struct net_device *ca_redirect_dev;
        u32 ca_goto_chain;
        u32 ca_queue_id;
        u32 ca_port_id;
    };
};

struct chain_actions {
    int cha_count;
    struct chain_action cha_acts[];
};

struct chain {
    struct list_head ch_list;
    u32 ch_chain_index;
    u32 ch_prio;
    struct chain_filter ch_filter;
    struct chain_actions ch_acts;
};

struct manipulate_vlan {
    struct chain_filter mav_fil;
    struct net_device *mav_ig_dev;
    struct net_device *mav_eg_dev;
    int mav_queue_id;
    int mav_wan_flow;
    rdpa_vlan_action_cfg_t mav_act;
};

enum command_type {
    CMD_MANIPULATE_VLAN, CMD_SET_VLAN_VID
};

struct vlan_vid_params {
    char vvp_name[IFNAMSIZ];
    uint16_t vvp_vid;
};

struct command_info {
    enum command_type cmd_type;
    union {
        struct manipulate_vlan cmd_manipulate_vlan;
        struct vlan_vid_params cmd_vlan_vid_params;
    };
};

struct command {
    struct list_head cmd_list;
    struct crumb *cmd_last_cu;
    struct command_info cmd_info;
};

struct vlan_flow_result {
    rdpa_ic_cfg_t vfr_ic_cfg;
    rdpa_traffic_dir vfr_ic_dir;
    rdpa_ic_info_t vfr_flow;
};

struct command_result {
    union {
        struct vlan_flow_result cr_vlan_flow;
        struct vlan_vid_params cr_vlan_vid_params;
    };
};

struct active_command {
    struct hlist_node ac_hn;
    struct list_head ac_list;
    struct command_info ac_info;
    int ac_generation;
    struct crumb *ac_last_cu;
    struct command_result ac_res;
};

enum crumb_type { CU_INGRESS_DEV, CU_IG_CHAIN, CU_EGRESS_DEV, CU_UPPER_DEV
        , CU_BRIDGE_DEV, CU_LOWER_DEV, CU_EG_CHAIN, CU_EGRESS_MIRROR };

struct crumb {
    struct crumb *cu_parent;
    int cu_refcnt;
    struct list_head cu_list;
    enum crumb_type cu_type;
    union {
        struct net_device *cu_netdev;
        struct chain *cu_chain;
    };
};

struct crumb_match {
    struct list_head cm_list;
    const char *cm_error;
    struct crumb *cm_crumb;
    struct chain_filter cm_cur_fil;
    struct chain_actions *cm_acts;
    int cm_i;
    struct manipulate_vlan cm_mav;
};

static DEFINE_HASHTABLE(active_commands_hash, 10); // 1K
static DEFINE_HASHTABLE(ic_cache, 10); // 1K
static DEFINE_HASHTABLE(flow_cache, 10); // 1K
static LIST_HEAD(active_enet_devs);
static LIST_HEAD(active_commands_list);

static bool tcf_vlan_push_prio_exists(const struct tc_action *a)
{
	bool res;

	rcu_read_lock();
	res = rcu_dereference(to_vlan(a)->vlan_p)->tcfv_push_prio_exists;
	rcu_read_unlock();

	return res;
}

static bool tcf_vlan_push_vid_exists(const struct tc_action *a)
{
	bool res;

	rcu_read_lock();
	res = rcu_dereference(to_vlan(a)->vlan_p)->tcfv_vlan_id_exists;
	rcu_read_unlock();

	return res;
}

// lifted from bcmdrivers/opensource/char/rdpa_mw/impl1/rdpa_mw_vlan.c
static bool vlan_action_find(const rdpa_vlan_action_cfg_t *action
                    , rdpa_traffic_dir dir, bdmf_object_handle *vlan_action_obj)
{
    bdmf_object_handle obj = NULL;
    rdpa_vlan_action_cfg_t action_tmp;
    rdpa_traffic_dir dir_tmp;

    *vlan_action_obj = NULL;

    while ((obj = bdmf_get_next(rdpa_vlan_action_drv(), obj, NULL)))
    {
        rdpa_vlan_action_dir_get(obj, &dir_tmp);
        if (dir != dir_tmp)
            continue;

        rdpa_vlan_action_action_get(obj, &action_tmp);
        if (!memcmp(action, &action_tmp, sizeof(action_tmp)))
        {
            *vlan_action_obj = obj;
            return true;
        }
    }
    return false;
}

// lifted from bcmdrivers/opensource/char/rdpa_mw/impl1/rdpa_mw_vlan.c
static int ds_dal_enabled_get(bdmf_object_handle port_obj)
{
    rdpa_port_dp_cfg_t cfg;

    rdpa_port_cfg_get(port_obj, &cfg);
    return cfg.dal_enable;
}

static void parse_vlan_push_action(const struct tc_action *a, rdpa_vlan_action_cfg_t *rva) {
    rva->parm[0].vid = tcf_vlan_push_vid(a);
    rva->parm[0].pbit = tcf_vlan_push_prio(a);
    rva->parm[0].tpid = be16_to_cpu(tcf_vlan_push_proto(a));
}

static void enet_flower_parse_vlan(const struct tc_action *a, struct chain_actions *ch) {
    switch (tcf_vlan_action(a)) {
        case TCA_VLAN_ACT_POP:
            ch->cha_acts[ch->cha_count].ca_type = CA_VLAN;
            ch->cha_acts[ch->cha_count].ca_rdpa_vlan_action.cmd |= RDPA_VLAN_CMD_POP;
            ch->cha_count++;
            break;
        case TCA_VLAN_ACT_PUSH: {
            rdpa_vlan_action_cfg_t *rva = &ch->cha_acts[ch->cha_count].ca_rdpa_vlan_action;

            ch->cha_acts[ch->cha_count].ca_type = CA_VLAN;
            ch->cha_acts[ch->cha_count].ca_rdpa_vlan_action.cmd |= RDPA_VLAN_CMD_PUSH_ALWAYS;
            parse_vlan_push_action(a, rva);
            if (tcf_vlan_push_prio_exists(a)) {
                if (rva->parm[0].pbit == 0xff)
                    rva->parm[0].pbit = 0; /* no REMARK and pbit 0 means copy */
                else
                    ch->cha_acts[ch->cha_count].ca_rdpa_vlan_action.cmd |= RDPA_VLAN_CMD_REMARK;
            }
            ch->cha_count++;
            break;
        }
        case TCA_VLAN_ACT_MODIFY:
            ch->cha_acts[ch->cha_count].ca_type = CA_VLAN;
            ch->cha_acts[ch->cha_count].ca_rdpa_vlan_action.cmd |= RDPA_VLAN_CMD_TPID_REMARK;
            parse_vlan_push_action(a, &ch->cha_acts[ch->cha_count].ca_rdpa_vlan_action);
            if (tcf_vlan_push_vid_exists(a))
                ch->cha_acts[ch->cha_count].ca_rdpa_vlan_action.cmd |= RDPA_VLAN_CMD_REPLACE;
            if (tcf_vlan_push_prio_exists(a))
                ch->cha_acts[ch->cha_count].ca_rdpa_vlan_action.cmd |= RDPA_VLAN_CMD_REMARK;
            ch->cha_count++;
            break;
        default:
            printk(KERN_ERR "unknown action %d proto %x vid %d\n", tcf_vlan_action(a)
                    , be16_to_cpu(tcf_vlan_push_proto(a)), tcf_vlan_push_vid(a));
            break;
    }
}

static void add_ca_redirect(const struct tc_action *a, enum chain_action_type cat, struct chain_actions *ch) {
    struct net_device *out_dev = tcf_mirred_dev(a);

    ch->cha_acts[ch->cha_count].ca_type = cat;
    ch->cha_acts[ch->cha_count].ca_redirect_dev = out_dev;
    ch->cha_count++;
}

static void enet_flower_parse_actions(struct tcf_exts *exts, struct chain_actions *ch, u32 prio) {
    const struct tc_action *a;
    int i;

    ch->cha_count = 0;
    tcf_exts_for_each_action(i, a, exts) {
        if (is_tcf_gact_ok(a)) {
            ch->cha_acts[ch->cha_count].ca_type = CA_PASS;
            ch->cha_count++;
        } else if (is_tcf_gact_shot(a)) {
            ch->cha_acts[ch->cha_count].ca_type = CA_DROP;
            ch->cha_count++;
        } else if (__is_tcf_gact_act(a, TC_ACT_UNSPEC, false)) {
            /* TC_ACT_UNSPEC is continue. See iproute2 tc_util.c */
            ch->cha_acts[ch->cha_count].ca_type = CA_CONTINUE;
            ch->cha_count++;
        } else if (is_tcf_gact_trap(a)) {
            printk(KERN_ERR "unexpected is_tcf_gact_trap\n");
        } else if (is_tcf_gact_goto_chain(a)) {
            ch->cha_acts[ch->cha_count].ca_type = CA_GOTO;
            ch->cha_acts[ch->cha_count].ca_goto_chain = tcf_gact_goto_chain_index(a);
            ch->cha_count++;
        } else if (is_tcf_mirred_egress_redirect(a)) {
            add_ca_redirect(a, CA_EGRESS_REDIRECT, ch);
        } else if (a->ops && a->ops->type == TCA_ACT_MIRRED
                        && to_mirred(a)->tcfm_eaction == TCA_INGRESS_REDIR) {
            add_ca_redirect(a, CA_INGRESS_REDIRECT, ch);
        } else if (is_tcf_mirred_egress_mirror(a)) {
            add_ca_redirect(a, CA_EGRESS_MIRROR, ch);
        } else if (is_tcf_vlan(a)) {
            enet_flower_parse_vlan(a, ch);
        } else if (a->ops && a->ops->type == TCA_ACT_SKBEDIT) {
            struct tcf_skbedit_params *ps;

            rcu_read_lock();
            ps = rcu_dereference(to_skbedit(a)->params);
            if (ps->mask == SKBMARK_Q_M) {
                ch->cha_acts[ch->cha_count].ca_type = CA_SET_QUEUE_ID;
                ch->cha_acts[ch->cha_count].ca_queue_id = SKBMARK_GET_Q(ps->mark);
                ch->cha_count++;
            } else if (ps->mask == SKBMARK_PORT_M) {
                ch->cha_acts[ch->cha_count].ca_type = CA_SET_PORT;
                ch->cha_acts[ch->cha_count].ca_port_id = SKBMARK_GET_PORT(ps->mark);
                ch->cha_count++;
            } else {
                printk(KERN_ERR "got unknown SKBEDIT 0x%x 0x%x\n", ps->mark, ps->mask);
            }
            rcu_read_unlock();
        } else {
            printk(KERN_ERR "unknown tc action %d prio %u:%x\n"
                    , a->ops ? a->ops->type : -1, TC_H_MAJ(prio) >> 16, TC_H_MIN(prio));
        }
    }
}

static void reset_chain_filter(struct chain_filter *chf) {
    memset(chf, 0, sizeof(*chf));
    chf->cf_indev_ifindex = -1;
}

static struct chain *alloc_chain(int capacity) {
    struct chain *ch;
    const int ch_sz = sizeof(*ch) + sizeof(struct chain_actions) + capacity * sizeof(struct chain_action);

    ch = kmalloc(ch_sz, GFP_KERNEL);
    BUG_ON(!ch);

    memset(ch, 0, ch_sz);
    reset_chain_filter(&ch->ch_filter);
    return ch;
}

static void parse_flower_filter(struct tc_cls_flower_offload *f, struct chain_filter *chf) {
    if (f->mask->indev_ifindex)
        chf->cf_indev_ifindex = f->key->indev_ifindex;
    if (dissector_uses_key(f->dissector, FLOW_DISSECTOR_KEY_VLAN)) {
        struct flow_dissector_key_vlan *key = skb_flow_dissector_target(f->dissector, FLOW_DISSECTOR_KEY_VLAN, f->key);
        struct flow_dissector_key_vlan *mask = skb_flow_dissector_target(f->dissector
                                                        , FLOW_DISSECTOR_KEY_VLAN, f->mask);

        if (mask->vlan_id) {
            chf->cf_ic_fields |= RDPA_IC_MASK_OUTER_VID;
            chf->cf_key.outer_vid = key->vlan_id;
        }
        if (mask->vlan_priority != 0) {
            chf->cf_ic_fields |= RDPA_IC_MASK_OUTER_PBIT;
            chf->cf_key.outer_pbits = key->vlan_priority;
        }
        if (ntohs(f->common.protocol) != ETH_P_ALL) {
            chf->cf_ic_fields |= RDPA_IC_MASK_OUTER_TPID;
            chf->cf_key.outer_tpid = ntohs(f->common.protocol);
        }
    }

    if (dissector_uses_key(f->dissector, FLOW_DISSECTOR_KEY_CVLAN)) {
        struct flow_dissector_key_vlan *key = skb_flow_dissector_target(f->dissector
                                                        , FLOW_DISSECTOR_KEY_CVLAN, f->key);
        struct flow_dissector_key_vlan *mask = skb_flow_dissector_target(f->dissector
                                                        , FLOW_DISSECTOR_KEY_CVLAN, f->mask);

        if (mask->vlan_id) {
            chf->cf_ic_fields |= RDPA_IC_MASK_INNER_VID;
            chf->cf_key.inner_vid = key->vlan_id;
        }
        if (mask->vlan_priority != 0) {
            chf->cf_ic_fields |= RDPA_IC_MASK_INNER_PBIT;
            chf->cf_key.inner_pbits = key->vlan_priority;
        }
        if (mask->vlan_tpid) {
            chf->cf_ic_fields |= RDPA_IC_MASK_INNER_TPID;
            chf->cf_key.inner_tpid = ntohs(key->vlan_tpid);
        }
    }

    if (dissector_uses_key(f->dissector, FLOW_DISSECTOR_KEY_BASIC)) {
        struct flow_dissector_key_basic *key =
            skb_flow_dissector_target(f->dissector,
                    FLOW_DISSECTOR_KEY_BASIC,
                    f->key);
        struct flow_dissector_key_basic *mask =
            skb_flow_dissector_target(f->dissector,
                    FLOW_DISSECTOR_KEY_BASIC,
                    f->mask);
        uint16_t n_proto_key = ntohs(key->n_proto);
        uint16_t n_proto_mask = ntohs(mask->n_proto);

        if (mask->ip_proto) {
            chf->cf_ic_fields |= RDPA_IC_MASK_IP_PROTOCOL;
            chf->cf_key.protocol = key->ip_proto;
        }

        if (n_proto_mask && n_proto_key != ETH_P_ALL) {
            chf->cf_ic_fields |= RDPA_IC_MASK_ETHER_TYPE;
            chf->cf_key.etype = n_proto_key;
        }
    }

    if (dissector_uses_key(f->dissector, FLOW_DISSECTOR_KEY_NUM_OF_VLANS)) {
        struct flow_dissector_key_num_of_vlans *key = skb_flow_dissector_target(f->dissector
                            , FLOW_DISSECTOR_KEY_NUM_OF_VLANS, f->key);

        chf->cf_ic_fields |= RDPA_IC_MASK_NUM_OF_VLANS;
        chf->cf_key.number_of_vlans = key->num_of_vlans;
    }
}

static bdmf_object_handle dev2rdpa_port_obj(struct net_device *root_dev)
{
    bcm_netdev_priv_info_get_cb_fn_t priv_info_get = bcm_netdev_ext_field_get(root_dev, bcm_netdev_cb_fn);
    bcm_netdev_priv_info_out_t info_out = {};

    if (!priv_info_get || priv_info_get(root_dev, BCM_NETDEV_TO_RDPA_PORT_OBJ, &info_out))
        return NULL;

    return info_out.bcm_netdev_to_rdpa_port_obj.rdpa_port_obj;
}

static int new_ingress_class(rdpa_traffic_dir tra_dir, const rdpa_ic_cfg_t *cfg, bdmf_object_handle *ig_obj) {
    BDMF_MATTR_ALLOC(ig_attrs, rdpa_ingress_class_drv());
    int ret;

    rdpa_ingress_class_dir_set(ig_attrs, tra_dir);
    rdpa_ingress_class_cfg_set(ig_attrs, cfg);
    ret = bdmf_new_and_set(rdpa_ingress_class_drv(), NULL, ig_attrs, ig_obj);
    BDMF_MATTR_FREE(ig_attrs);
    return ret;
}

static uint32_t ic_hash(rdpa_traffic_dir tra_dir, const rdpa_ic_cfg_t *cfg) {
    return jhash(cfg, sizeof(*cfg), tra_dir);
}

static bool ic_entry_eq(rdpa_traffic_dir tra_dir, const rdpa_ic_cfg_t *cfg, const struct ic_entry *en) {
    return !memcmp(&en->ic_cfg, cfg, sizeof(*cfg)) && en->ic_dir == tra_dir;
}

static struct ic_entry *find_ic_cache(rdpa_traffic_dir tra_dir, const rdpa_ic_cfg_t *cfg, uint32_t *pkey) {
    const uint32_t key = ic_hash(tra_dir, cfg);
    struct ic_entry *en;

    if (pkey)
        *pkey = key;

    hash_for_each_possible(ic_cache, en, ic_hn, key) {
        if (ic_entry_eq(tra_dir, cfg, en))
            return en;
    }
    return NULL;
}

static struct ic_entry *new_or_old_ingress_class(rdpa_traffic_dir tra_dir, rdpa_ic_cfg_t *cfg) {
    struct ic_entry *en;
    uint32_t key;
    int rc;

    en = find_ic_cache(tra_dir, cfg, &key);
    if (en) {
        en->ic_refcount++;
        return en;
    }

    {
        long unsigned *bm = tra_dir == rdpa_dir_ds ? ds_priorities : us_priorities;
        const int p = find_first_zero_bit(bm, RDPA_IC_MAX_PRIORITY + 1);
        if (p > RDPA_IC_MAX_PRIORITY) {
            printk(KERN_ERR "priority bitmap is full\n");
            return NULL;
        }
        cfg->prty = p;
        set_bit(p, bm);
    }

    en = kmalloc(sizeof(*en), GFP_KERNEL);
    BUG_ON(!en);

    rc = new_ingress_class(tra_dir, cfg, &en->ic_obj);
    if (rc) {
        printk(KERN_ERR "new_ingress_class failure %d\n", rc);
        kfree(en);
        return NULL;
    }

    memcpy(&en->ic_cfg, cfg, sizeof(*cfg));
    en->ic_cfg.prty = 0;
    en->ic_dir = tra_dir;
    en->ic_refcount = 1;
    en->ic_prio = cfg->prty;
    hash_add(ic_cache, &en->ic_hn, key);
    return en;
}

static int new_or_old_vlan_action(rdpa_traffic_dir tra_dir, const rdpa_vlan_action_cfg_t *v_act
                                        , bdmf_object_handle *v_obj) {
    BDMF_MATTR_ALLOC(vlan_attrs, rdpa_vlan_action_drv());
    int ret;

    if (vlan_action_find(v_act, tra_dir, v_obj))
        return 0;

    rdpa_vlan_action_dir_set(vlan_attrs, tra_dir);
    rdpa_vlan_action_action_set(vlan_attrs, v_act);
    ret = bdmf_new_and_set(rdpa_vlan_action_drv(), NULL, vlan_attrs, v_obj);
    BDMF_MATTR_FREE(vlan_attrs);
    return ret;
}

static void add_chain_filter(struct list_head *chs, struct chain *ch) {
    struct chain *cur;

    /* to insert same priorities in last in last out order, iterate in reverse */
    list_for_each_entry_reverse(cur, chs, ch_list) {
        if (ch->ch_prio >= cur->ch_prio) {
            /* list_add inserts the entry after cur */
            chs = &cur->ch_list;
            break;
        }
    }
    list_add(&ch->ch_list, chs);
}

static void remove_chain(struct list_head *chs, const struct tc_cls_common_offload *cmn) {
    struct chain *ch, *tmp;

    list_for_each_entry_safe(ch, tmp, chs, ch_list) {
        /* More then one chain can have given priority */
        if (ch->ch_prio == cmn->prio && ch->ch_chain_index == cmn->chain_index) {
            list_del(&ch->ch_list);
            kfree(ch);
        }
    }
}

static struct chain *alloc_dev_chain(struct tcf_exts *exts, const struct tc_cls_common_offload *cmn) {
    struct chain *ch = alloc_chain(exts->nr_actions);

    ch->ch_chain_index = cmn->chain_index;
    ch->ch_prio = cmn->prio;
    enet_flower_parse_actions(exts, &ch->ch_acts, ch->ch_prio);
    return ch;
}

static void direct_set_ds_forwarding(struct net_device *txdev, rdpa_ic_info_t *flow, bdmf_object_handle vlan_obj) {
    flow->result.forw_mode = rdpa_forwarding_mode_flow;
    flow->result.port_egress_obj = dev2rdpa_port_obj(txdev);
    flow->result.vlan_action = vlan_obj;
}

static int direct_set_port_action(struct net_device *txdev, bdmf_object_handle ig_obj
                                    , bdmf_index flow_idx, bdmf_object_handle vlan_obj) {
    (void) txdev;
    (void) ig_obj;
    (void) flow_idx;
    (void) vlan_obj;
    return 0;
}

static void bridged_set_ds_forwarding(struct net_device *txdev, rdpa_ic_info_t *flow, bdmf_object_handle vlan_obj) {
    (void) txdev;
    (void) vlan_obj;
    flow->result.forw_mode = rdpa_forwarding_mode_pkt;
    flow->result.port_egress_obj = dev2rdpa_port_obj(txdev); /* XXX: port_egress_obj is ignored for packet based
                                                                forwarding */
}

static int bridged_set_port_action(struct net_device *txdev, bdmf_object_handle ig_obj
                                    , bdmf_index flow_idx, bdmf_object_handle vlan_obj) {
    rdpa_port_action_key_t pa_key;
    rdpa_port_action_t pa = { .drop = 0 };

    pa_key.flow = flow_idx;
    pa_key.port_egress_obj = dev2rdpa_port_obj(txdev);
    pa.vlan_action = vlan_obj;
    return rdpa_ingress_class_port_action_set(ig_obj, &pa_key, &pa);
}

static uint32_t flow_hash(bdmf_object_handle ig_obj, const rdpa_ic_info_t *flow) {
    return jhash(&flow->key, sizeof(flow->key), (uint32_t) (unsigned long) ig_obj);
}

static bool flow_entry_eq(bdmf_object_handle ig_obj, const rdpa_ic_info_t *flow, const struct flow_entry *en) {
    return !memcmp(&en->fe_flow.key, &flow->key, sizeof(flow->key)) && en->fe_ig_obj == ig_obj;
}

static struct flow_entry *find_flow_cache(bdmf_object_handle ig_obj, const rdpa_ic_info_t *flow, uint32_t *pkey) {
    const uint32_t key = flow_hash(ig_obj, flow);
    struct flow_entry *en;

    if (pkey)
        *pkey = key;

    hash_for_each_possible(flow_cache, en, fe_hn, key) {
        if (flow_entry_eq(ig_obj, flow, en))
            return en;
    }

    return NULL;
}

static int new_or_old_flow(struct ic_entry *ic_en, bdmf_index *flow_idx, rdpa_ic_info_t *flow) {
    struct flow_entry *en;
    uint32_t key;
    int rc;

    en = find_flow_cache(ic_en->ic_obj, flow, &key);
    if (en) {
        *flow_idx = en->fe_flow_idx;
        en->fe_refcount++;
        /*
         * We've found pre-existing flow with the same key. Still there is
         * a good chance that the new flow result action is different.
         * 
         * Note that the flows arrive in their priority order. Therefore
         * the newer result should override the older.
         *
         * Therefore we replace flow result and release old vlan_action if needed.
         *
         */
        if (memcmp(&en->fe_flow.result, &flow->result, sizeof(flow->result))) {
            if (en->fe_flow.result.vlan_action)
                bdmf_put(en->fe_flow.result.vlan_action);
            memcpy(&en->fe_flow.result, &flow->result, sizeof(flow->result));
            return rdpa_ingress_class_flow_set(ic_en->ic_obj, *flow_idx, flow);
        } else
            return 0;
    }

    rc = rdpa_ingress_class_flow_add(ic_en->ic_obj, flow_idx, flow);
    if (rc)
        return rc;

    en = kmalloc(sizeof(*en), GFP_KERNEL);
    BUG_ON(!en);

    memcpy(&en->fe_flow, flow, sizeof(*flow));
    en->fe_ig_obj = ic_en->ic_obj;
    en->fe_flow_idx = *flow_idx;
    en->fe_last_used = 0;
    en->fe_refcount = 1;
    memset(&en->fe_stat, 0, sizeof(en->fe_stat));
    hash_add(flow_cache, &en->fe_hn, key);
    ic_en->ic_refcount++;
    return 0;
}

static void ic_entry_put(struct ic_entry *ic_en) {
    ic_en->ic_refcount--;
    if (ic_en->ic_refcount > 0)
        return;

    hash_del(&ic_en->ic_hn);
    bdmf_destroy(ic_en->ic_obj);
    {
        long unsigned *bm = ic_en->ic_dir == rdpa_dir_ds ? ds_priorities : us_priorities;
        clear_bit(ic_en->ic_prio, bm);
    }
    kfree(ic_en);
}

static int on_ds_flow_vlan(const struct flow_ops *ops, struct net_device *txdev
                                , const rdpa_vlan_action_cfg_t *vlan_act, struct vlan_flow_result *res) {
    bdmf_object_handle vlan_obj;
    struct ic_entry *ic_en;
    bdmf_index flow_idx;
    int rc;

    ic_en = new_or_old_ingress_class(rdpa_dir_ds, &res->vfr_ic_cfg);
    if (!ic_en)
        return -1;

    rc = new_or_old_vlan_action(rdpa_dir_ds, vlan_act, &vlan_obj);
    if (rc)
        return rc;

    res->vfr_flow.result.action = rdpa_forward_action_forward;
    ops->dfo_init_flow(txdev, &res->vfr_flow, vlan_obj);

    rc = new_or_old_flow(ic_en, &flow_idx, &res->vfr_flow);
    if (rc) {
        printk(KERN_ERR "rdpa_ingress_class_flow_add failure %d\n", rc);
        return rc;
    }

    rc = ops->dfo_set_port_action(txdev, ic_en->ic_obj, flow_idx, vlan_obj);
    ic_entry_put(ic_en); // was incremented by new_or_old_flow
    return rc;
}

static int on_us_flow_vlan(const struct manipulate_vlan *mav, struct vlan_flow_result *res) {
    bdmf_object_handle vlan_obj;
    struct ic_entry *ic_en;
    bdmf_index flow_idx;
    bcm_netdev_priv_info_get_cb_fn_t priv_info_get = bcm_netdev_ext_field_get(mav->mav_ig_dev, bcm_netdev_cb_fn);
    bcm_netdev_priv_info_out_t info_out = {};
    int rc;

    if (!priv_info_get || priv_info_get(mav->mav_ig_dev, BCM_NETDEV_TO_RDPA_PORT_OBJ, &info_out))
        return -1;

    res->vfr_flow.key.port_ingress_obj = info_out.bcm_netdev_to_rdpa_port_obj.rdpa_port_obj;
    res->vfr_ic_cfg.field_mask |= RDPA_IC_MASK_INGRESS_PORT;

    ic_en = new_or_old_ingress_class(rdpa_dir_us, &res->vfr_ic_cfg);
    if (!ic_en)
        return -1;

    rc = new_or_old_vlan_action(rdpa_dir_us, &mav->mav_act, &vlan_obj);
    if (rc)
        return rc;

    res->vfr_flow.result.action = rdpa_forward_action_forward;
    res->vfr_flow.result.forw_mode = rdpa_forwarding_mode_flow;
    res->vfr_flow.result.vlan_action = vlan_obj;
    res->vfr_flow.result.port_egress_obj = dev2rdpa_port_obj(mav->mav_eg_dev);

    rc = new_or_old_flow(ic_en, &flow_idx, &res->vfr_flow);
    if (rc) {
        printk(KERN_ERR "on_us_flow_vlan: new_or_old_flow failure %d\n", rc);
        return rc;
    }
    
    ic_entry_put(ic_en); // was incremented by new_or_old_flow
    return rc;
}

static const struct flow_ops bridged_flow_ops = {
    .dfo_init_flow = bridged_set_ds_forwarding,
    .dfo_set_port_action = bridged_set_port_action,
};

static const struct flow_ops direct_flow_ops = {
    .dfo_init_flow = direct_set_ds_forwarding,
    .dfo_set_port_action = direct_set_port_action,
};

static bool is_our_dev(struct net_device *dev) {
    enetx_netdev *ex;

    list_for_each_entry(ex, &active_enet_devs, enet_devs) {
        if (ex->enet_netdev == dev)
            return true;
    }
    return false;
}

static bool is_mav_ds(const struct manipulate_vlan *mav) {
    return is_netdev_wan(mav->mav_ig_dev) && !is_netdev_wan(mav->mav_eg_dev);
}

static bool is_mav_us(const struct manipulate_vlan *mav) {
    return is_netdev_wan(mav->mav_eg_dev) && !is_netdev_wan(mav->mav_ig_dev);
}

static void interpret_vlan_command(const struct manipulate_vlan *mav, struct vlan_flow_result *res) {
    memset(res, 0, sizeof(*res));
    res->vfr_ic_cfg.type = RDPA_IC_TYPE_FLOW;
    res->vfr_ic_cfg.field_mask = mav->mav_fil.cf_ic_fields;
    res->vfr_flow.key = mav->mav_fil.cf_key;
    if (mav->mav_queue_id != -1)
        res->vfr_flow.result.queue_id = mav->mav_queue_id;
    if (mav->mav_wan_flow != -1)
        res->vfr_flow.result.wan_flow = mav->mav_wan_flow;

    if (is_mav_ds(mav)) {
        res->vfr_ic_dir = rdpa_dir_ds;
        on_ds_flow_vlan(ds_dal_enabled_get(dev2rdpa_port_obj(mav->mav_ig_dev))
                    ? &bridged_flow_ops : &direct_flow_ops, mav->mav_eg_dev, &mav->mav_act, res);
    } else if (is_mav_us(mav)) {
        res->vfr_ic_dir = rdpa_dir_us;
        on_us_flow_vlan(mav, res);
    } else {
        printk(KERN_ERR "unknown direction ig %s(%d), eg %s(%d)!\n"
                    , mav->mav_ig_dev->name, is_netdev_wan(mav->mav_ig_dev)
                    , mav->mav_eg_dev->name, is_netdev_wan(mav->mav_eg_dev));
    }
    res->vfr_ic_cfg.prty = 0; /* reset priority to 0. See how new_or_old_ingress_class sets prty after hashing it */
}

static void set_vlan_vid(const struct vlan_vid_params *vvp, int enable) {
    bdmf_object_handle vlan_object;

    if (rdpa_vlan_get(vvp->vvp_name, &vlan_object))
        return;

    rdpa_vlan_vid_enable_set(vlan_object, vvp->vvp_vid, enable);
    bdmf_put(vlan_object);
}

static void interpret_command(const struct command_info *cmd, struct command_result *res) {
    switch (cmd->cmd_type) {
    case CMD_MANIPULATE_VLAN:
        interpret_vlan_command(&cmd->cmd_manipulate_vlan, &res->cr_vlan_flow);
        break;
    case CMD_SET_VLAN_VID:
        memcpy(&res->cr_vlan_vid_params, &cmd->cmd_vlan_vid_params, IFNAMSIZ);
        set_vlan_vid(&cmd->cmd_vlan_vid_params, 1);
        break;
    };
}

static void revert_vlan_flow_result(const struct vlan_flow_result *res) {
    struct flow_entry *flow_en;
    struct ic_entry *ic_en;
    int rc;

    ic_en = find_ic_cache(res->vfr_ic_dir, &res->vfr_ic_cfg, NULL);
    if (!ic_en)
        return;

    flow_en = find_flow_cache(ic_en->ic_obj, &res->vfr_flow, NULL);
    if (!flow_en)
        return;

    flow_en->fe_refcount--;
    if (flow_en->fe_refcount > 0)
        return;

    hash_del(&flow_en->fe_hn);
    rc = rdpa_ingress_class_flow_delete(ic_en->ic_obj, flow_en->fe_flow_idx);
    if (rc) {
        printk(KERN_ERR "rdpa_ingress_class_flow_delete failed %d\n", rc);
        return;
    }

    if (flow_en->fe_flow.result.vlan_action)
        bdmf_put(flow_en->fe_flow.result.vlan_action);

    kfree(flow_en);
    ic_entry_put(ic_en);
}

static void revert_command_result(enum command_type cmd_type, struct command_result *res) {
    switch (cmd_type) {
    case CMD_MANIPULATE_VLAN:
        revert_vlan_flow_result(&res->cr_vlan_flow);
        break;
    case CMD_SET_VLAN_VID:
        set_vlan_vid(&res->cr_vlan_vid_params, 0);
        break;
    };
}

static void free_last_crumb(struct crumb *cu) {
    do {
        struct crumb *c;

        BUG_ON(cu->cu_refcnt < 1);
        if (cu->cu_refcnt > 1) {
            cu->cu_refcnt--;
            break;
        }
        c = cu;
        cu = cu->cu_parent;
        kfree(c);
    } while (cu);
}

static void interpret_commands(struct list_head *cmds) {
    struct active_command *ac;
    struct hlist_node *tmp;
    static int generation;
    unsigned int bkt;

    generation++;

    while (!list_empty(cmds)) {
        struct command *cmd = list_first_entry(cmds, struct command, cmd_list);
        const uint32_t key = jhash(&cmd->cmd_info, sizeof(cmd->cmd_info), 0);

        hash_for_each_possible(active_commands_hash, ac, ac_hn, key) {
            if (!memcmp(&ac->ac_info, &cmd->cmd_info, sizeof(cmd->cmd_info))) {
                ac->ac_generation = generation;
                goto del_cmd;
            }
        }

        ac = kmalloc(sizeof(*ac), GFP_KERNEL);
        BUG_ON(!ac);

        memcpy(&ac->ac_info, &cmd->cmd_info, sizeof(cmd->cmd_info));
        ac->ac_generation = generation;
        ac->ac_last_cu = cmd->cmd_last_cu;

        interpret_command(&cmd->cmd_info, &ac->ac_res);
        hash_add(active_commands_hash, &ac->ac_hn, key);
        list_add_tail(&ac->ac_list, &active_commands_list);

del_cmd:
        list_del(&cmd->cmd_list);
        kfree(cmd);
    }

    hash_for_each_safe(active_commands_hash, bkt, tmp, ac, ac_hn) {
        if (ac->ac_generation < generation) {
            revert_command_result(ac->ac_info.cmd_type, &ac->ac_res);
            hash_del(&ac->ac_hn);
            list_del(&ac->ac_list);
            free_last_crumb(ac->ac_last_cu);
            kfree(ac);
        }
    }
}

static void update_vlan_flow_stats(struct tcf_exts *exts, const struct vlan_flow_result *res) {
    struct flow_entry *flow_en;
    struct ic_entry *ic_en;
    rdpa_stat_t flow_stat;
    uint64_t packets = 0;
    uint64_t bytes = 0;

    ic_en = find_ic_cache(res->vfr_ic_dir, &res->vfr_ic_cfg, NULL);
    if (!ic_en)
        return;

    flow_en = find_flow_cache(ic_en->ic_obj, &res->vfr_flow, NULL);
    if (!flow_en)
        return;

    rdpa_ingress_class_flow_stat_get(ic_en->ic_obj, flow_en->fe_flow_idx, &flow_stat);
    if (flow_stat.packets > flow_en->fe_stat.packets) {
        packets = flow_stat.packets - flow_en->fe_stat.packets;
        flow_en->fe_stat.packets = flow_stat.packets;
    }
    if (flow_stat.bytes > flow_en->fe_stat.bytes) {
        bytes = flow_stat.bytes - flow_en->fe_stat.bytes;
        flow_en->fe_stat.bytes = flow_stat.bytes;
    }

    if (packets || bytes)
        flow_en->fe_last_used = jiffies;

    tcf_exts_stats_update(exts, bytes, packets, flow_en->fe_last_used);
}

static bool crumbs_have_prio(const struct crumb *cu, u32 prio) {
    for (; cu; cu = cu->cu_parent) {
        const bool has_chain = cu->cu_type == CU_EG_CHAIN || cu->cu_type == CU_IG_CHAIN;
        if (has_chain && cu->cu_chain->ch_prio == prio)
            return true;
    }
    return false;
}

static void update_chain_stats(struct tcf_exts *exts, const struct tc_cls_common_offload *cmn) {
    struct active_command *ac;
    unsigned int bkt;

    hash_for_each(active_commands_hash, bkt, ac, ac_hn) {
        if (ac->ac_info.cmd_type == CMD_MANIPULATE_VLAN && crumbs_have_prio(ac->ac_last_cu, cmn->prio))
            update_vlan_flow_stats(exts, &ac->ac_res.cr_vlan_flow);
    }
}

static int modify_chains(enum tc_setup_type type, void *type_data, struct list_head *chs)
{
    switch (type) {
    case TC_SETUP_CLSFLOWER: {
        struct tc_cls_flower_offload *f = type_data;

        switch (f->command) {
        case TC_CLSFLOWER_REPLACE: {
            struct chain *ch = alloc_dev_chain(f->exts, &f->common);

            parse_flower_filter(f, &ch->ch_filter);

            /* add_chain_filter takes cf_ic_fields into account, should be called after parse_flower_filter */
            add_chain_filter(chs, ch);
            break;
        }
        case TC_CLSFLOWER_DESTROY:
            remove_chain(chs, &f->common);
            return 0;
        case TC_CLSFLOWER_STATS: {
            update_chain_stats(f->exts, &f->common);
            return 0;
        }
        default:
                return -EOPNOTSUPP;
        }
        return 0;
    }
    case TC_SETUP_CLSMATCHALL: {
        struct tc_cls_matchall_offload *f = type_data;

        switch (f->command) {
        case TC_CLSMATCHALL_REPLACE: {
            struct chain *ch = alloc_dev_chain(f->exts, &f->common);

            ch->ch_filter.cf_ic_fields = RDPA_IC_MASK_ANY;
            add_chain_filter(chs, ch);
            return 0;
        }
        case TC_CLSMATCHALL_DESTROY:
            remove_chain(chs, &f->common);
            return 0;
        default:
            return -EOPNOTSUPP;
        }
    }
    case TC_SETUP_CLSU32: {
        struct tc_cls_u32_offload *cls_u32 = type_data;

        switch (cls_u32->command) {
        case TC_CLSU32_NEW_KNODE:
        case TC_CLSU32_REPLACE_KNODE: {
            struct chain *ch = alloc_dev_chain(cls_u32->knode.exts, &cls_u32->common);

            if (cls_u32->knode.mask == SKBMARK_PORT_M) {
                ch->ch_filter.cf_ic_fields |= RDPA_IC_MASK_GEM_FLOW;
                ch->ch_filter.cf_key.gem_flow = SKBMARK_GET_PORT(cls_u32->knode.val);
            } else  {
                printk(KERN_ERR "bad TC_CLSU32_REPLACE_KNODE 0x%x 0x%x\n", cls_u32->knode.val, cls_u32->knode.mask);
                goto bad_u32;
            }

            add_chain_filter(chs, ch);
            return 0;

bad_u32:
            kfree(ch);
            return -EOPNOTSUPP;
        }
        case TC_CLSU32_DELETE_KNODE:
        case TC_CLSU32_DELETE_HNODE:
            remove_chain(chs, &cls_u32->common);
            return 0;
        default:
            printk(KERN_ERR "unexpected TC_SETUP_CLSU32 command: %d\n", cls_u32->command);
            return -EOPNOTSUPP;
        }
    }
    default:
        printk(KERN_ERR "unknown modify_chains %d\n", type);
        return -EOPNOTSUPP;
    }
    return 0;
}

static int enet_setup_tc_block_ig(enum tc_setup_type type, void *type_data, void *cb_priv)
{
    enetx_netdev *ex = netdev_priv(cb_priv);
    return modify_chains(type, type_data, &ex->ig_chains);
}

static int enet_setup_tc_block_eg(enum tc_setup_type type, void *type_data, void *cb_priv)
{
    enetx_netdev *ex = netdev_priv(cb_priv);
    return modify_chains(type, type_data, &ex->eg_chains);
}

static int enet_setup_tc_block(struct net_device *dev, struct tc_block_offload *f)
{
    int ret = -EOPNOTSUPP;
    tc_setup_cb_t *cb;

    if (f->binder_type == TCF_BLOCK_BINDER_TYPE_CLSACT_INGRESS) {
        cb = enet_setup_tc_block_ig;
    } else if (f->binder_type == TCF_BLOCK_BINDER_TYPE_CLSACT_EGRESS) {
        cb = enet_setup_tc_block_eg;
    } else {
        return -EOPNOTSUPP;
    }

	switch (f->command) {
	case TC_BLOCK_BIND:
        ret = tcf_block_cb_register(f->block, cb, dev, dev, f->extack);
        break;
	case TC_BLOCK_UNBIND:
        tcf_block_cb_unregister(f->block, cb, dev);
        return 0;
	default:
		return -EOPNOTSUPP;
	}

    return ret;
}

int bcmenet_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data) {
    switch (type) {
    case TC_SETUP_BLOCK:
        return enet_setup_tc_block(dev, type_data);
    case TC_SETUP_QDISC_RED:
        return -EOPNOTSUPP;
    case TC_SETUP_QDISC_PRIO:
        return -EOPNOTSUPP;
    default:
        return -EOPNOTSUPP;
    }
}

static void init_crumb_match(struct crumb *cr, struct crumb_match *cm) {
    memset(cm, 0, sizeof(*cm));
    cm->cm_crumb = cr;
    reset_chain_filter(&cm->cm_mav.mav_fil);
    reset_chain_filter(&cm->cm_cur_fil);
    cm->cm_mav.mav_queue_id = -1;
    cm->cm_mav.mav_wan_flow = -1;
}

static const char *crumb_match_acts(struct crumb_match *cm) {
    const char *err = NULL;

    BUG_ON(!cm->cm_acts);

    switch (cm->cm_acts->cha_acts[cm->cm_i].ca_type) {
        case CA_VLAN: {
            const rdpa_vlan_action_cfg_t *cur = &cm->cm_acts->cha_acts[cm->cm_i].ca_rdpa_vlan_action;
            cm->cm_mav.mav_act.cmd |= cur->cmd;
            if (cur->cmd & RDPA_VLAN_CMD_PUSH_ALWAYS) {
                cm->cm_cur_fil.cf_ic_fields |= RDPA_IC_MASK_NUM_OF_VLANS;
                cm->cm_cur_fil.cf_key.number_of_vlans++;
            }
            if (cur->cmd & RDPA_VLAN_CMD_POP) {
                cm->cm_cur_fil.cf_ic_fields |= RDPA_IC_MASK_NUM_OF_VLANS;
                cm->cm_cur_fil.cf_key.number_of_vlans--;
            }
            if (cur->cmd & RDPA_VLAN_CMD_REMARK) {
                cm->cm_mav.mav_act.parm[0].pbit = cur->parm[0].pbit;
                cm->cm_cur_fil.cf_ic_fields |= RDPA_IC_MASK_OUTER_PBIT;
                cm->cm_cur_fil.cf_key.outer_pbits = cur->parm[0].pbit;
            }
            if (cur->cmd & (RDPA_VLAN_CMD_PUSH_ALWAYS | RDPA_VLAN_CMD_REPLACE)) {
                cm->cm_mav.mav_act.parm[0].vid = cur->parm[0].vid;
                cm->cm_cur_fil.cf_ic_fields |= RDPA_IC_MASK_OUTER_VID;
                cm->cm_cur_fil.cf_key.outer_vid = cur->parm[0].vid;
            }
            if (cur->cmd & (RDPA_VLAN_CMD_PUSH_ALWAYS | RDPA_VLAN_CMD_TPID_REMARK))
                cm->cm_mav.mav_act.parm[0].tpid = cur->parm[0].tpid;
            break;
        }
        case CA_SET_QUEUE_ID:
            cm->cm_mav.mav_queue_id = cm->cm_acts->cha_acts[cm->cm_i].ca_queue_id;
            break;
        case CA_SET_PORT:
            cm->cm_mav.mav_wan_flow = cm->cm_acts->cha_acts[cm->cm_i].ca_port_id;
            break;
        case CA_DROP:
            err = "unhandled drop";
            break;
        case CA_CONTINUE:
        case CA_PASS:
        case CA_EGRESS_REDIRECT:
        case CA_INGRESS_REDIRECT:
        case CA_EGRESS_MIRROR:
        case CA_GOTO:
            break;
    }
    cm->cm_i++;
    if (cm->cm_i >= cm->cm_acts->cha_count) {
        cm->cm_i = 0;
        cm->cm_acts = NULL;
    }
    return err;
}

static const char *crumb_match_type(struct crumb_match *cm) {
    const char *err = NULL;

    BUG_ON(!cm->cm_crumb);
    switch (cm->cm_crumb->cu_type) {
        case CU_INGRESS_DEV:
            cm->cm_mav.mav_ig_dev = cm->cm_crumb->cu_netdev;
            break;
        case CU_EG_CHAIN: {
            const int i = cm->cm_crumb->cu_chain->ch_filter.cf_indev_ifindex;

            if (i < 0)
                goto eg_fall;

            if (cm->cm_cur_fil.cf_indev_ifindex == -1) {
                err = "no lower dev";
                goto out;
            }

            if (cm->cm_cur_fil.cf_indev_ifindex != i) {
                err = "no lower match";
                goto out;
            }
        }
eg_fall:
            /* fall through */
        case CU_IG_CHAIN: {
            struct chain_filter *a = &cm->cm_mav.mav_fil;
            const struct chain_filter *b = &cm->cm_crumb->cu_chain->ch_filter;
            struct chain_filter *c = &cm->cm_cur_fil;

            cm->cm_i = 0;
            cm->cm_acts = &cm->cm_crumb->cu_chain->ch_acts;

            if (a->cf_indev_ifindex == -1 && b->cf_indev_ifindex != -1)
                a->cf_indev_ifindex = b->cf_indev_ifindex;
            else if (a->cf_indev_ifindex != -1 && b->cf_indev_ifindex != -1) {
                err = "two indevs";
                goto out;
            }

#define U(K, F) do {\
    if (!(b->cf_ic_fields & K)) \
        break; \
    if (c->cf_ic_fields & K) { \
        if (c->cf_key.F != b->cf_key.F) { \
            err = "no cur match: " #F; \
            goto out; \
        } \
    } else if (a->cf_ic_fields & K) { \
        err = "duplicate: " #F; \
        goto out; \
    } else { \
        a->cf_ic_fields |= K; \
        a->cf_key.F = b->cf_key.F; \
        c->cf_ic_fields |= K; \
        c->cf_key.F = b->cf_key.F; \
    } \
} while (0)
            U(RDPA_IC_MASK_NUM_OF_VLANS, number_of_vlans);
            U(RDPA_IC_MASK_OUTER_VID, outer_vid);
            U(RDPA_IC_MASK_OUTER_TPID, outer_tpid);
            U(RDPA_IC_MASK_OUTER_PBIT, outer_pbits);
            U(RDPA_IC_MASK_ETHER_TYPE, etype);
            U(RDPA_IC_MASK_IP_PROTOCOL, protocol);
            U(RDPA_IC_MASK_INNER_VID, inner_vid);
            U(RDPA_IC_MASK_INNER_PBIT, inner_pbits);
            U(RDPA_IC_MASK_INNER_TPID, inner_tpid);
            U(RDPA_IC_MASK_INGRESS_PORT, port_ingress_obj);
            U(RDPA_IC_MASK_GEM_FLOW, gem_flow);
#undef U
            break;
        }
        case CU_EGRESS_DEV:
            cm->cm_mav.mav_eg_dev = cm->cm_crumb->cu_netdev;
            break;
        case CU_LOWER_DEV:
            cm->cm_cur_fil.cf_indev_ifindex = cm->cm_crumb->cu_netdev->ifindex;
            break;
        case CU_UPPER_DEV:
            break;
        case CU_EGRESS_MIRROR:
            break;
        case CU_BRIDGE_DEV:
            break;
    }

out:
    return err;
}

static bool crumb_match_good(struct crumb_match *cm) {
    const char *err = NULL;
    
    if (cm->cm_acts) {
        err = "match has acts";
        goto out;
    }

    if (!cm->cm_mav.mav_ig_dev) {
        err = "match has no ig";
        goto out;
    }

    if (!cm->cm_mav.mav_eg_dev) {
        err = "match has no eg";
        goto out;
    }

    if (!is_mav_us(&cm->cm_mav) && !is_mav_ds(&cm->cm_mav)) {
        err = "neither us nor ds";
        goto out;
    }

out:
    cm->cm_error = err;
    return (err == NULL);
}

static struct crumb *alloc_crumb(enum crumb_type ct, struct crumb *pare) {
    struct crumb *cu = kmalloc(sizeof(struct crumb), GFP_KERNEL);

    BUG_ON(!cu);
    cu->cu_type = ct;
    cu->cu_parent = pare;
    cu->cu_refcnt = 1;
    if (pare)
        pare->cu_refcnt++;
    return cu;
}

static void cm_list_add(struct crumb_match *cm, struct list_head *lh) {
    BUG_ON(!cm->cm_crumb);
    BUG_ON(cm->cm_crumb->cu_refcnt != 1);
    list_add_tail(&cm->cm_list, lh);
}

static void init_matches(struct list_head *bc_head) {
    enetx_netdev *ex;

    list_for_each_entry(ex, &active_enet_devs, enet_devs) {
        struct crumb *cu = alloc_crumb(CU_INGRESS_DEV, NULL);
        struct crumb_match *cm = kmalloc(sizeof(struct crumb_match), GFP_KERNEL);

        BUG_ON(!cm);
        cu->cu_netdev = ex->enet_netdev;
        init_crumb_match(cu, cm);

        cm->cm_error = NULL;
        cm_list_add(cm, bc_head);
    }
}

static enum crumb_type dev_crumb_type(struct net_device *dev, bool is_upper) {
    return is_our_dev(dev) ? CU_EGRESS_DEV
        : netif_is_bridge_master(dev) ? CU_BRIDGE_DEV : is_upper ? CU_UPPER_DEV : CU_LOWER_DEV;

}

static void add_crumb_dev(struct net_device *dev, enum crumb_type cut, struct crumb *pare, struct list_head *crumbs) {
    struct crumb *cu = alloc_crumb(cut, pare);

    cu->cu_netdev = dev;
    list_add_tail(&cu->cu_list, crumbs);
}

static void add_upper_devs(struct crumb *last_cu, struct list_head *cus) {
    struct list_head *iter;
    struct net_device *dev;

    netdev_for_each_upper_dev_rcu(last_cu->cu_netdev, dev, iter) {
        /* vfrwd cannot be normal upper, we can only jump there */
        if (netif_is_bridge_master(dev))
            add_crumb_dev(dev, dev_crumb_type(dev, true), last_cu, cus);
    }
}

static void add_chain_crumb(struct chain *ch, struct crumb *last_cu, bool is_ingress, struct list_head *cus) {
    struct crumb *cu = alloc_crumb(is_ingress ? CU_IG_CHAIN : CU_EG_CHAIN, last_cu);

    cu->cu_chain = ch;
    list_add_tail(&cu->cu_list, cus);
}


static void add_crumb_chains(struct crumb *last_cu, bool is_ingress, struct list_head *cus) {
    enetx_netdev *ex = netdev_priv(last_cu->cu_netdev);
    struct chain *ch;

    list_for_each_entry(ch, is_ingress ? &ex->ig_chains : &ex->eg_chains, ch_list) {
        /* non-0 chains are being jumped into */
        if (ch->ch_chain_index)
            continue;
        
        add_chain_crumb(ch, last_cu, is_ingress, cus);
    }
}

static const char *jump_through_chain(enetx_netdev *(*get_dev)(bool is_ingress, void *ctx), void *ctx
            , struct crumb *last_cu, bool is_ingress, struct list_head *cus) {
    struct chain *ch = last_cu->cu_chain;
    struct chain_action *cha = &ch->ch_acts.cha_acts[ch->ch_acts.cha_count - 1];
    const char *err = NULL;

    switch (cha->ca_type) {
        case CA_GOTO: {
            enetx_netdev *ex = get_dev(is_ingress, ctx);
            if (!ex) {
                err = "no dev cu";
                goto out;
            }

            list_for_each_entry(ch, is_ingress ? &ex->ig_chains : &ex->eg_chains, ch_list) {
                if (ch->ch_chain_index != cha->ca_goto_chain)
                    continue;

                add_chain_crumb(ch, last_cu, is_ingress, cus);
            }
            if (list_empty(cus))
                err = "no goto found";
            goto out;
        }
        case CA_EGRESS_REDIRECT: {
            add_crumb_dev(cha->ca_redirect_dev, dev_crumb_type(cha->ca_redirect_dev, true), last_cu, cus);
            goto out;
        }
        case CA_CONTINUE: {
            struct chain_action *prev;
            if (ch->ch_acts.cha_count < 2) {
                err = "continue is short";
                goto out;
            }

            prev = &ch->ch_acts.cha_acts[ch->ch_acts.cha_count - 2];
            if (prev->ca_type != CA_EGRESS_MIRROR) {
                err = "continue without mirror";
                goto out;
            }

            add_crumb_dev(prev->ca_redirect_dev, CU_EGRESS_MIRROR, last_cu, cus);
            goto out;
        }
        default:
            goto out;
    }
out:
    return err;
}

static const char *make_next_crumb(enetx_netdev *(*get_dev)(bool is_ingress, void *ctx), void *ctx
                , struct crumb *last_cu, struct list_head *cus) {
    const char *err = NULL;

    switch (last_cu->cu_type) {
        case CU_INGRESS_DEV: {
            add_crumb_chains(last_cu, true, cus);
            if (!list_empty(cus))
                goto out;

            add_upper_devs(last_cu, cus);
            break;
        }
        case CU_EG_CHAIN: {
            err = jump_through_chain(get_dev, ctx, last_cu, false, cus);
            goto out;
        }
        case CU_IG_CHAIN: {
            err = jump_through_chain(get_dev, ctx, last_cu, true, cus);
            goto out;
        }
        case CU_EGRESS_MIRROR:
        case CU_UPPER_DEV: {
            add_upper_devs(last_cu, cus);
            goto out;
        }
        case CU_LOWER_DEV: {
            struct list_head *iter;
            struct net_device *dev;

            netdev_for_each_lower_dev(last_cu->cu_netdev, dev, iter) {
                if (!list_empty(cus)) {
                    err = "more than one for lower";
                    goto out;
                }
                add_crumb_dev(dev, dev_crumb_type(dev, false), last_cu, cus);
            }
            goto out;
        }
        case CU_EGRESS_DEV: {
            add_crumb_chains(last_cu, false, cus);
            goto out;
        }
        case CU_BRIDGE_DEV: {
            struct list_head *iter;
            struct net_device *dev;
            const struct crumb *prev = last_cu->cu_parent;

            if (prev->cu_type != CU_UPPER_DEV && prev->cu_type != CU_INGRESS_DEV
                        && prev->cu_type != CU_EGRESS_MIRROR) {
                err = "unknown prev under bridge";
                goto out;
            }

            netdev_for_each_lower_dev(last_cu->cu_netdev, dev, iter) {
                if (dev == prev->cu_netdev)
                    continue;
                add_crumb_dev(dev, dev_crumb_type(dev, false), last_cu, cus);
            }
            goto out;
        }
    }
out:
    return err;
}

static void show_filters(struct seq_file *m, const struct chain_filter *fil) {
#define P(K, F) do {\
    if (fil->cf_ic_fields & K) \
        seq_printf(m, ", %s=%d", #K, fil->cf_key.F); \
} while (0)
    P(RDPA_IC_MASK_NUM_OF_VLANS, number_of_vlans);
    P(RDPA_IC_MASK_OUTER_VID, outer_vid);
    P(RDPA_IC_MASK_OUTER_TPID, outer_tpid);
    P(RDPA_IC_MASK_OUTER_PBIT, outer_pbits);
    P(RDPA_IC_MASK_ETHER_TYPE, etype);
    P(RDPA_IC_MASK_IP_PROTOCOL, protocol);
    P(RDPA_IC_MASK_INNER_VID, inner_vid);
    P(RDPA_IC_MASK_INNER_PBIT, inner_pbits);
    P(RDPA_IC_MASK_INNER_TPID, inner_tpid);
    P(RDPA_IC_MASK_GEM_FLOW, gem_flow);
#undef P
}

static void crumb_command_show(struct seq_file *m, const struct command_info *cinfo) {
    switch (cinfo->cmd_type) {
        case CMD_SET_VLAN_VID:
            seq_printf(m, "  set vlan vid %d for %s\n", cinfo->cmd_vlan_vid_params.vvp_vid
                            , cinfo->cmd_vlan_vid_params.vvp_name);
            break;
        case CMD_MANIPULATE_VLAN: {
            const rdpa_vlan_action_cfg_t *va = &cinfo->cmd_manipulate_vlan.mav_act;
            show_filters(m, &cinfo->cmd_manipulate_vlan.mav_fil);
            seq_printf(m, "  manipulate 0x%x(", va->cmd);
#define P(K) do {\
    if (va->cmd & K) \
        seq_printf(m, ", %s", #K); \
} while (0)
            P(RDPA_VLAN_CMD_POP);
            P(RDPA_VLAN_CMD_PUSH_ALWAYS);
            P(RDPA_VLAN_CMD_REPLACE);
            P(RDPA_VLAN_CMD_REMARK);
            P(RDPA_VLAN_CMD_TPID_REMARK);
#undef P
            seq_printf(m, ") vid=%d, pbit=%d, tpid=%x\n", va->parm[0].vid, va->parm[0].pbit, va->parm[0].tpid);
            break;
        }
    }
}

static void show_chain_prio(struct seq_file *m, const char *s, const struct chain *ch) {
    seq_printf(m, " %s(%d)=%u:%x", s, ch->ch_chain_index, TC_H_MAJ(ch->ch_prio) >> 16, TC_H_MIN(ch->ch_prio) /* proto */);
}

static void show_crumbs_list(struct seq_file *m, struct list_head *crs) {
    struct crumb *cu;

    list_for_each_entry(cu, crs, cu_list) {
        switch (cu->cu_type) {
            case CU_INGRESS_DEV:
                seq_printf(m, " ingress=%s", cu->cu_netdev->name);
                break;
            case CU_EGRESS_DEV:
                seq_printf(m, " egress=%s", cu->cu_netdev->name);
                break;
            case CU_UPPER_DEV:
                seq_printf(m, " upper=%s", cu->cu_netdev->name);
                break;
            case CU_EGRESS_MIRROR:
                seq_printf(m, " mirror=%s", cu->cu_netdev->name);
                break;
            case CU_LOWER_DEV:
                seq_printf(m, " lower=%s", cu->cu_netdev->name);
                break;
            case CU_BRIDGE_DEV:
                seq_printf(m, " bridge=%s", cu->cu_netdev->name);
                break;
            case CU_IG_CHAIN:
                show_chain_prio(m, "ig_chain", cu->cu_chain);
                break;
            case CU_EG_CHAIN:
                show_chain_prio(m, "eg_chain", cu->cu_chain);
                break;
        }
    }
    seq_printf(m, "\n");
}

static void show_crumb_leaf(struct seq_file *m, struct crumb *cu) {
    LIST_HEAD(cus);
    for (; cu; cu = cu->cu_parent)
        list_add(&cu->cu_list, &cus);
    show_crumbs_list(m, &cus);
}

static void crumb_match_show(struct seq_file *m, const struct crumb_match *cm) {
    seq_printf(m, "%s  > ig %s, eg %s, queue id %d, wan flow %d"
        , cm->cm_error ? cm->cm_error : ""
        , cm->cm_mav.mav_ig_dev ? cm->cm_mav.mav_ig_dev->name : "-"
        , cm->cm_mav.mav_eg_dev ? cm->cm_mav.mav_eg_dev->name : "-"
        , cm->cm_mav.mav_queue_id, cm->cm_mav.mav_wan_flow);
    show_filters(m, &cm->cm_mav.mav_fil);
    seq_printf(m, "; cur");
    show_filters(m, &cm->cm_cur_fil);
    seq_printf(m, "\n");
    show_crumb_leaf(m, cm->cm_crumb);
}

static struct command *alloc_cmd(enum command_type ct, struct crumb_match *cm) {
    struct command *cmd = kmalloc(sizeof(*cmd), GFP_KERNEL);

    BUG_ON(!cmd);
    memset(cmd, 0, sizeof(*cmd)); /* we are going to hash it later */
    cmd->cmd_info.cmd_type = ct;
    BUG_ON(!cm->cm_crumb);
    cmd->cmd_last_cu = cm->cm_crumb;
    cmd->cmd_last_cu->cu_refcnt++;
    return cmd;
}

static struct command *alloc_vlan_cmd(struct crumb_match *cm) {
    struct command *cmd = alloc_cmd(CMD_MANIPULATE_VLAN, cm);
    memcpy(&cmd->cmd_info.cmd_manipulate_vlan, &cm->cm_mav, sizeof(cm->cm_mav));
    return cmd;
}

static enetx_netdev *get_cm_dev(bool is_ingress, void *c) {
    struct crumb_match *cm = c;
    struct net_device *dev = is_ingress ? cm->cm_mav.mav_ig_dev : cm->cm_mav.mav_eg_dev;
    return dev ? netdev_priv(dev) : NULL;
}

static void iterate_match(struct list_head *todos, struct list_head *goods, struct list_head *bads) {
    struct crumb_match *cm = list_first_entry(todos, struct crumb_match, cm_list);
    LIST_HEAD(cus);

    BUG_ON(!cm);
    list_del(&cm->cm_list);

    BUG_ON(!cm->cm_crumb);
    BUG_ON(cm->cm_error);
    BUG_ON(cm->cm_acts);

    cm->cm_error = crumb_match_type(cm);
    if (cm->cm_error)
        goto bad;

    while (cm->cm_acts) {
        cm->cm_error = crumb_match_acts(cm);
        if (cm->cm_error)
            goto bad;
    }

    cm->cm_error = make_next_crumb(get_cm_dev, cm, cm->cm_crumb, &cus);
    if (cm->cm_error)
        goto bad;

    if (list_empty(&cus))
        goto good;

    while (!list_empty(&cus)) {
        struct crumb *cu = list_last_entry(&cus, struct crumb, cu_list);
        struct crumb_match *tm;

        list_del(&cu->cu_list);
        if (list_empty(&cus))
            tm = cm;
        else {
            tm = kmalloc(sizeof(struct crumb_match), GFP_KERNEL);
            memcpy(tm, cm, sizeof(struct crumb_match));
        }
        tm->cm_crumb = cu;
        cm_list_add(tm, todos);
    }
    return;

good:
    if (crumb_match_good(cm))
        cm_list_add(cm, goods);
    else
        goto bad;
    return;

bad:
    while (!list_empty(&cus)) {
        struct crumb *cu = list_first_entry(&cus, struct crumb, cu_list);
        list_del(&cu->cu_list);
        free_last_crumb(cu);
    }
    cm_list_add(cm, bads);
    return;
}

static int run_matches(struct list_head *todos, struct list_head *goods, struct list_head *bads) {
    int i;

    init_matches(todos);
    for (i = 0; i < crumbs_info_iterations && !list_empty(todos); i++)
        iterate_match(todos, goods, bads);

    return i;
}

static void free_matches(struct list_head *cms) {
    while (!list_empty(cms)) {
        struct crumb_match *cm = list_first_entry(cms, struct crumb_match, cm_list);

        list_del(&cm->cm_list);
        free_last_crumb(cm->cm_crumb);
        kfree(cm);
    }
}

static void show_crumb_match_list(struct seq_file *m, struct list_head *lh) {
    struct crumb_match *cm;

    list_for_each_entry(cm, lh, cm_list) {
        crumb_match_show(m, cm);
    }
}

static int matches_info_show(struct seq_file *m, void *v) {
    int iters;

    LIST_HEAD(bads);
    LIST_HEAD(todos);
    LIST_HEAD(goods);

    iters = run_matches(&todos, &goods, &bads);
    seq_printf(m, "========= todos %d ==============\n", iters);
    show_crumb_match_list(m, &todos);
    seq_printf(m, "========= goods ==============\n");
    show_crumb_match_list(m, &goods);
    seq_printf(m, "========= bads ==============\n");
    show_crumb_match_list(m, &bads);

    free_matches(&todos);
    free_matches(&goods);
    free_matches(&bads);
    return 0;
}

static int matches_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, matches_info_show, NULL);
}

static ssize_t crumbs_info_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) 
{
    char buf[16] = {0};
    long num;
    int ret;

    if (*ppos > 0 || count > 15)
        return -EFAULT;

    if (copy_from_user(buf, ubuf ,count))
        return -EFAULT;

    ret = kstrtol(buf, 10, &num);
    if (ret)
        return ret;

    crumbs_info_iterations = num;
    *ppos = strlen(buf);
    return strlen(buf);
}

static const struct file_operations matches_info_ops = {
    .open    = matches_info_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
    .write = crumbs_info_write
};

static int active_commands_show(struct seq_file *m, void *v) {
    struct active_command *ac;

    list_for_each_entry(ac, &active_commands_list, ac_list) {
        seq_printf(m, "generation %d\n", ac->ac_generation);
        show_crumb_leaf(m, ac->ac_last_cu);
        crumb_command_show(m, &ac->ac_info);
    }
    return 0;
}

static int active_commands_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, active_commands_show, NULL);
}

static const struct file_operations active_commands_info_ops = {
    .open    = active_commands_info_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static void show_chain(struct seq_file *m, enetx_netdev *ex, bool is_ingress) {
    struct chain *ch;

    list_for_each_entry(ch, is_ingress ? &ex->ig_chains : &ex->eg_chains, ch_list) {
        seq_printf(m, "%s", ex->enet_netdev->name);
        show_chain_prio(m, is_ingress ? "i" : "e", ch);
        show_filters(m, &ch->ch_filter);
        seq_printf(m, "\n");
    }
}

static int chains_show(struct seq_file *m, void *v) {
    enetx_netdev *ex;

    list_for_each_entry(ex, &active_enet_devs, enet_devs) {
        show_chain(m, ex, true);
        show_chain(m, ex, false);
    }
    return 0;
}

static int chains_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, chains_show, NULL);
}

static const struct file_operations chains_info_ops = {
    .open    = chains_info_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static struct command *try_vlan_object_command(struct crumb_match *cm) {
    struct net_device *vfrwd_dev = NULL;
    struct command *cmd = NULL;
    bool is_pkt_based = false;
    bool has_vid = false;
    struct crumb *cu;

    if (!is_mav_ds(&cm->cm_mav))
        return NULL;

    is_pkt_based = ds_dal_enabled_get(dev2rdpa_port_obj(cm->cm_mav.mav_ig_dev));
    if (!is_pkt_based)
        goto out;

    has_vid = cm->cm_mav.mav_act.cmd & (RDPA_VLAN_CMD_PUSH_ALWAYS | RDPA_VLAN_CMD_REPLACE);
    if (!has_vid)
        goto out;

    for (cu = cm->cm_crumb; cu; cu = cu->cu_parent) {
        if (cu->cu_type == CU_LOWER_DEV && is_netdev_vfrwd(cu->cu_netdev)) {
            vfrwd_dev = cu->cu_netdev;
            break;
        }
    }
    if (!vfrwd_dev)
        goto out;

    cmd = alloc_cmd(CMD_SET_VLAN_VID, cm);
    cmd->cmd_info.cmd_vlan_vid_params.vvp_vid = cm->cm_mav.mav_act.parm[0].vid;
    strcpy(cmd->cmd_info.cmd_vlan_vid_params.vvp_name, vfrwd_dev->name);
out:
    return cmd;
}

static void match_crumb_trigger(struct list_head *cmds) {
    struct crumb_match *cm;
    LIST_HEAD(bads);
    LIST_HEAD(todos);
    LIST_HEAD(goods);

    run_matches(&todos, &goods, &bads);
    list_for_each_entry(cm, &goods, cm_list) {
        struct command *cmd = try_vlan_object_command(cm);
        if (cmd)
            list_add_tail(&cmd->cmd_list, cmds);

        cmd = alloc_vlan_cmd(cm);
        list_add_tail(&cmd->cmd_list, cmds);
    }

    free_matches(&todos);
    free_matches(&goods);
    free_matches(&bads);
}

int bcmenet_tc_trigger(int argc, char **argv) {
    LIST_HEAD(cmds);

    match_crumb_trigger(&cmds);
    interpret_commands(&cmds);
    return 0;
}

static void fill_blog_rule(const struct manipulate_vlan *mav, blogRule_t *rule) {
    uint8_t to_tag = 0;

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_MASK_NUM_OF_VLANS)
        rule->filter.nbrOfVlanTags = mav->mav_fil.cf_key.number_of_vlans;

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_MASK_OUTER_VID) {
        rule->filter.vlan[0].mask.h_vlan_TCI |= VLAN_VID_MASK;
        rule->filter.vlan[0].value.h_vlan_TCI &= ~VLAN_VID_MASK;
        rule->filter.vlan[0].value.h_vlan_TCI |= 
            mav->mav_fil.cf_key.outer_vid & VLAN_VID_MASK;
    }

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_MASK_OUTER_TPID)
        rule->filter.eth.mask.h_proto = rule->filter.eth.value.h_proto = mav->mav_fil.cf_key.outer_tpid;

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_MASK_OUTER_PBIT) {
        rule->filter.vlan[0].mask.h_vlan_TCI |= VLAN_PRIO_MASK;
        rule->filter.vlan[0].value.h_vlan_TCI &= ~VLAN_PRIO_MASK;
        rule->filter.vlan[0].value.h_vlan_TCI |= 
            (mav->mav_fil.cf_key.outer_pbits << VLAN_PRIO_SHIFT) & VLAN_PRIO_MASK;
    }

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_ETHER_TYPE) {
        switch (rule->filter.nbrOfVlanTags) {
            case 0:
                rule->filter.eth.mask.h_proto = rule->filter.eth.value.h_proto = mav->mav_fil.cf_key.etype;
                break;
            case 1:
                rule->filter.vlan[0].mask.h_vlan_encapsulated_proto
                    = rule->filter.vlan[0].value.h_vlan_encapsulated_proto = mav->mav_fil.cf_key.etype;
                break;
            case 2:
                rule->filter.vlan[1].mask.h_vlan_encapsulated_proto
                    = rule->filter.vlan[1].value.h_vlan_encapsulated_proto = mav->mav_fil.cf_key.etype;
                break;
            default:
                printk(KERN_ERR "bad nbrOfVlanTags for proto\n");
        }
    }

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_MASK_IP_PROTOCOL) {
        rule->filter.ipv4.mask.ip_proto = rule->filter.ipv4.value.ip_proto = mav->mav_fil.cf_key.protocol;
    }

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_MASK_INNER_VID) {
        rule->filter.vlan[1].mask.h_vlan_TCI |= VLAN_VID_MASK;
        rule->filter.vlan[1].value.h_vlan_TCI &= ~VLAN_VID_MASK;
        rule->filter.vlan[1].value.h_vlan_TCI |= mav->mav_fil.cf_key.inner_vid;
    }

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_MASK_INNER_PBIT) {
        rule->filter.vlan[1].mask.h_vlan_TCI |= VLAN_PRIO_MASK;
        rule->filter.vlan[1].value.h_vlan_TCI &= ~VLAN_PRIO_MASK;
        rule->filter.vlan[1].value.h_vlan_TCI |= mav->mav_fil.cf_key.inner_pbits;
    }

    if (mav->mav_fil.cf_ic_fields & RDPA_IC_MASK_GEM_FLOW)
        rule->filter.skb.markPort = mav->mav_fil.cf_key.gem_flow;

    // Apparently RDPA_IC_MASK_INGRESS_PORT, port_ingress_obj is not used in blog.

    if (mav->mav_act.cmd & RDPA_VLAN_CMD_POP) {
        rule->action[rule->actionCount].cmd = BLOG_RULE_CMD_POP_VLAN_HDR;
        rule->actionCount++;
    }

    if (mav->mav_act.cmd & RDPA_VLAN_CMD_PUSH_ALWAYS) {
        rule->action[rule->actionCount].cmd = BLOG_RULE_CMD_PUSH_VLAN_HDR;
        rule->actionCount++;

        rule->action[rule->actionCount].cmd = BLOG_RULE_CMD_SET_VID;
        to_tag = rule->action[rule->actionCount].toTag = rule->filter.nbrOfVlanTags;
        rule->action[rule->actionCount].vid = mav->mav_act.parm[0].vid;
        rule->actionCount++;
    }

    if ((mav->mav_act.cmd & RDPA_VLAN_CMD_REPLACE) && rule->filter.nbrOfVlanTags > 0) {
        rule->action[rule->actionCount].cmd = BLOG_RULE_CMD_SET_VID;
        to_tag = rule->action[rule->actionCount].toTag = rule->filter.nbrOfVlanTags - 1;
        rule->action[rule->actionCount].vid = mav->mav_act.parm[0].vid;
        rule->actionCount++;
    }

    if (mav->mav_act.cmd & RDPA_VLAN_CMD_REMARK) {
        rule->action[rule->actionCount].cmd = BLOG_RULE_CMD_SET_PBITS;
        rule->action[rule->actionCount].toTag = to_tag;
        rule->action[rule->actionCount].pbits = mav->mav_act.parm[0].pbit;
        rule->actionCount++;
    }

    // We set RDPA_VLAN_CMD_TPID_REMARK unconditionally due to lacking kernel exists flag
    // Skip it here for now.
}

static int blog_rule_tc_hook(struct blog_t *blog_p, struct net_device *wan_virt, struct net_device *lan_virt) {
    const struct net_device *rx_dev = wan_virt ? netdev_path_get_root(wan_virt) : NULL;
    const struct net_device *tx_dev = lan_virt ? netdev_path_get_root(lan_virt) : NULL;
    blogRule_t *root_rule = blog_p->blogRule_p;
    const struct active_command *ac;
    blogRule_t *last_rule = NULL;
    blogRule_t *cur_rule;

    blog_p->blogRule_p = NULL;
    list_for_each_entry_reverse(ac, &active_commands_list, ac_list) {
        const struct manipulate_vlan *mav;
        bool lower_found = false;
        const struct crumb *cu;

        if (ac->ac_info.cmd_type != CMD_MANIPULATE_VLAN)
            continue;

        mav = &ac->ac_info.cmd_manipulate_vlan;
        if (mav->mav_ig_dev != rx_dev)
            continue;

        if (mav->mav_eg_dev != tx_dev)
            continue;

        for (cu = ac->ac_last_cu; cu; cu = cu->cu_parent) {
            if (lower_found) {
                if (cu->cu_type == CU_EGRESS_MIRROR && cu->cu_netdev == wan_virt) {
                    lower_found = false; /* for the next active command iteration */
                    goto found;
                }
            } else {
                if (cu->cu_type == CU_LOWER_DEV && cu->cu_netdev == lan_virt)
                    lower_found = true;
            }
        }
        continue;

found:
        cur_rule = blog_rule_alloc();
        BUG_ON(!cur_rule);

        if (root_rule)
            memcpy(cur_rule, root_rule, sizeof(*cur_rule));
        else
            blog_rule_init(cur_rule);

        fill_blog_rule(mav, cur_rule);
        if (last_rule)
            last_rule->next_p = cur_rule;
        else
            blog_p->blogRule_p = cur_rule;
        last_rule = cur_rule;
    }

    if (root_rule)
        blog_rule_free(root_rule);

    return 0;
}

void bcmenet_tc_init_procs(struct proc_dir_entry *parent) {
    proc_create("matches", 0, parent, &matches_info_ops);
    proc_create("active_commands", 0, parent, &active_commands_info_ops);
    proc_create("chains", 0, parent, &chains_info_ops);

    blogRuleTcHook = blog_rule_tc_hook;
}

void bcmenet_tc_enet_open(struct net_device *dev) {
    enetx_netdev *ex = netdev_priv(dev);

    list_add_tail(&ex->enet_devs, &active_enet_devs);
}

void bcmenet_tc_enet_stop(struct net_device *dev) {
    enetx_netdev *ex = netdev_priv(dev);

    list_del(&ex->enet_devs);
}
