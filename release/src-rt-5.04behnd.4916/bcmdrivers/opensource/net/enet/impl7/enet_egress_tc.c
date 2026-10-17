/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom 
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

/*
 *  Created on: Oct/2023
 *      Author: stefan.chulski@broadcom.com
 */ 

#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <net/pkt_cls.h>
#include "enet.h"
#include "port.h"
#include "enet_egress_tc.h"
#include "bcm_tc.h"

static egress_tm_parms tm_parms;
static u32 queue_size_sysfs;

extern int port_by_netdev(struct net_device *dev, enetx_port_t **match);
static u32 enet_port_def_queue_size(struct net_device *dev, u8 prio);

static int enet_sp_mall_replace(enetx_port_t *port, struct tc_cls_matchall_offload *f)
{
    struct flow_action_entry *act;
    struct port_police_entry *entry;
    int ret = 0;

#if defined(ENET_TC_INGRESS_CONFIG)
    if (f->common.chain_index == BCM_TC_CHAIN_ID_IC_INGRESS_FILTER ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_RX ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_PRIMARY ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_FLOW_MISS)
        return 0;
#endif

    if (!flow_offload_has_one_action(&f->rule->action)) {
        NL_SET_ERR_MSG(f->common.extack, "Only singular actions are supported");
        return -EOPNOTSUPP;
    }

    if (f->common.chain_index) {
        NL_SET_ERR_MSG(f->common.extack, "Only chain 0 is supported");
        return -EOPNOTSUPP;
    }
    act = &f->rule->action.entries[0];

    switch (act->id) {
    case FLOW_ACTION_POLICE:
        if (port->tc_setup_policer) {
            ret = port->tc_setup_policer(act);
        } else {
            ret = -EOPNOTSUPP;
        }
        if (!ret) {
            entry = kzalloc(sizeof(struct port_police_entry), GFP_KERNEL);
            if (!entry)
                return -ENOMEM;

            entry->cookie = f->cookie;
            entry->index = act->police.index;
            list_add_tail(&entry->list, &port->policer_list);
        }
        return ret;
    default:
        return -EOPNOTSUPP;
   }
}

static int enet_sp_mall_destroy(enetx_port_t *port, struct tc_cls_matchall_offload *f)
{
    struct port_police_entry *entry, *temp;

#if defined(ENET_TC_INGRESS_CONFIG)
    if (f->common.chain_index == BCM_TC_CHAIN_ID_IC_INGRESS_FILTER ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_RX ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_PRIMARY ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_FLOW_MISS)
        return 0;
#endif

    list_for_each_entry_safe(entry, temp, &port->policer_list, list) {
        if (entry->cookie == f->cookie) {
            if (port->tc_remove_policer)
                port->tc_remove_policer(entry->index);
            list_del(&entry->list);
            kfree(entry);
            return 0;
        }
    }
    return -EEXIST;
}

static int enet_sp_mall_stats(enetx_port_t *port, struct tc_cls_matchall_offload *f)
{
#if defined(ENET_TC_INGRESS_CONFIG)
    if (f->common.chain_index == BCM_TC_CHAIN_ID_IC_INGRESS_FILTER ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_RX ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_PRIMARY ||
        f->common.chain_index == BCM_TC_CHAIN_ID_IC_FLOW_MISS)
        return 0;
#endif

    return -EOPNOTSUPP;
}

static int enet_sp_flow_block_mall_cb(enetx_port_t *port, struct tc_cls_matchall_offload *f)
{
    switch (f->command) {
    case TC_CLSMATCHALL_REPLACE:
        return enet_sp_mall_replace(port, f);
    case TC_CLSMATCHALL_DESTROY:
        return enet_sp_mall_destroy(port, f);
    case TC_CLSMATCHALL_STATS:
        return enet_sp_mall_stats(port, f);
    default:
        return -EOPNOTSUPP;
    }
}

static int enet_setup_policer_tc_block_cb(enum tc_setup_type type, void *type_data, void *cb_priv)
{
    switch (type) {
    case TC_SETUP_CLSMATCHALL:
        return enet_sp_flow_block_mall_cb(cb_priv, type_data);
    default:
        /* TC requires all offload callbacks not to fail in case of skip_sw is used otherwise it fails the call. */
        return 0;
    }
}

static LIST_HEAD(enet_block_cb_list);

int enet_setup_policer_tc_block(struct net_device *dev, void *type_data)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;

    return flow_block_cb_setup_simple(type_data,
					  &enet_block_cb_list,
					  enet_setup_policer_tc_block_cb,
					  port, port, true);

}

int enet_setup_mqprio(struct net_device *dev,
			       struct tc_mqprio_qopt_offload *mqprio)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;

	if (mqprio->qopt.hw != TC_MQPRIO_HW_OFFLOAD_TCS)
		return 0;

    if (port->tc_setup_mqprio) {
        return port->tc_setup_mqprio(port, mqprio);
    }
    return -EOPNOTSUPP;
}

static struct egress_tm_node_dict node_dict[] = {
   {"PON", EGRESS_TM_PON},
   {"TCONT/LLID", EGRESS_TM_TCONT},
   {"ETS_PARENT", EGRESS_TM_ETS_PARENT},
   {"ETS_CHILD", EGRESS_TM_ETS_CHILD},
   {"HTB_PARENT", EGRESS_TM_HTB_PARENT},
   {"HTB_CHILD", EGRESS_TM_HTB_CHILD},
   {"RED", EGRESS_TM_RED},
   {"WRED", EGRESS_TM_GRED},
   {"PIE", EGRESS_TM_PIE},
   {"DUALQ_LAQM", EGRESS_TM_DUALQ_LAQM},
   {NULL, -1} // End of the dictionary marker
};

static struct egress_tm_node_dict state_dict[] = {
   {"NEW", EGRESS_NODE_NEW},
   {"CONFIGURED", EGRESS_NODE_CONF},
   {"CHANGED", EGRESS_NODE_CHANGED},
   {NULL, -1} // End of the dictionary marker
};

static const char* port_type_string[EGRESS_DEV_MAX] = {
   "DEV_ETH",
   "DEV_EPON",
   "DEV_GPON",
   "DEV_SVCQ"
};

static struct egress_tm_speed_dict speed_dict[] = {
    {EGRESS_DEV_ETH, SPEED_10, 0, EGRESS_DEF_Q_SIZE_1_MB, { NULL, NULL }},
    {EGRESS_DEV_ETH, SPEED_100, 0, EGRESS_DEF_Q_SIZE_1_MB, { NULL, NULL }},
    {EGRESS_DEV_ETH, SPEED_1000, 0, EGRESS_DEF_Q_SIZE_1_MB, { NULL, NULL }},
    {EGRESS_DEV_ETH, SPEED_2500, 0, EGRESS_DEF_Q_SIZE_4_MB, { NULL, NULL }},
    {EGRESS_DEV_ETH, SPEED_5000, 0, EGRESS_DEF_Q_SIZE_4_MB, { NULL, NULL }},
    {EGRESS_DEV_ETH, SPEED_10000, 0, EGRESS_DEF_Q_SIZE_4_MB, { NULL, NULL }},
    {EGRESS_DEV_ETH, SPEED_UNKNOWN, 0, EGRESS_DEF_Q_SIZE_1_MB, { NULL, NULL }},
    {EGRESS_DEV_EPON, SPEED_1000, 0, EGRESS_DEF_ETH_Q_SZ_LE_EPON, { NULL, NULL }},
    {EGRESS_DEV_EPON, SPEED_2500, 0, EGRESS_DEF_ETH_Q_SZ_LE_EPON, { NULL, NULL }},
    {EGRESS_DEV_EPON, SPEED_5000, 0, EGRESS_DEF_Q_SIZE_4_MB, { NULL, NULL }},
    {EGRESS_DEV_EPON, SPEED_10000, 0, EGRESS_DEF_Q_SIZE_4_MB, { NULL, NULL }},
    {EGRESS_DEV_EPON, SPEED_UNKNOWN, 0, EGRESS_DEF_Q_SIZE_1_MB, { NULL, NULL }},
    {EGRESS_DEV_GPON, SPEED_1000, 0, EGRESS_DEF_Q_SIZE_1_MB, { NULL, NULL }},
    {EGRESS_DEV_GPON, SPEED_2500, 0, EGRESS_DEF_Q_SIZE_1_MB, { NULL, NULL }},
    {EGRESS_DEV_GPON, SPEED_5000, 0, EGRESS_DEF_Q_SIZE_4_MB, { NULL, NULL }},
    {EGRESS_DEV_GPON, SPEED_10000, 0, EGRESS_DEF_Q_SIZE_4_MB, { NULL, NULL }},
    {EGRESS_DEV_GPON, SPEED_UNKNOWN, 0, EGRESS_DEF_Q_SIZE_1_MB, { NULL, NULL }} 
};

static DECLARE_HASHTABLE(def_speed_hash_table, 8);

/* general sysfs command */
static ssize_t enet_tc_egress_sysfs_dump_caps(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    memcpy((void *)buf, &tm_parms, sizeof(egress_tm_parms));

    return sizeof(egress_tm_parms);
}

static void enet_update_caps(enetx_port_t *port)
{
    if (port->tc_get_egress_tm_caps) {
        memset(&tm_parms, 0, sizeof(egress_tm_parms));
        port->tc_get_egress_tm_caps(port, &tm_parms); 
    }
}

static void enet_print_caps(enetx_port_t *port)
{
    if (port->tc_get_egress_tm_caps) {
        printk(KERN_INFO "schedCaps     = 0x%x\n", tm_parms.schedCaps);
        printk(KERN_INFO "    EGRESS_TM_SP_CAPABLE      = %s\n", (tm_parms.schedCaps & EGRESS_TM_SP_CAPABLE) ? "Yes" : "No");
        printk(KERN_INFO "    EGRESS_TM_WRR_CAPABLE     = %s\n", (tm_parms.schedCaps & EGRESS_TM_WRR_CAPABLE) ? "Yes" : "No");
        printk(KERN_INFO "    EGRESS_TM_SP_WRR_CAPABLE  = %s\n", (tm_parms.schedCaps & EGRESS_TM_SP_WRR_CAPABLE) ? "Yes" : "No");
        printk(KERN_INFO "    EGRESS_TM_1LEVEL_CAPABLE  = %s\n", (tm_parms.schedCaps & EGRESS_TM_1LEVEL_CAPABLE) ? "Yes" : "No");
        printk(KERN_INFO "maxQueues     = %d\n", tm_parms.maxQueues);
        printk(KERN_INFO "maxSpQueues   = %d\n", tm_parms.maxSpQueues);
        printk(KERN_INFO "portShaper    = %s\n", tm_parms.portShaper ? "Yes" : "No");
        printk(KERN_INFO "queueShaper   = %s\n", tm_parms.queueShaper ? "Yes" : "No"); 
        printk(KERN_INFO "dualRate      = %s\n", tm_parms.dualRate ? "Yes" : "No");
        printk(KERN_INFO "cfgFlags      = 0x%x\n", tm_parms.cfgFlags);
        printk(KERN_INFO "numQueues     = %d\n", tm_parms.numQueues);
        printk(KERN_INFO "dpiQueueExt   = %s\n", tm_parms.dpiQueueExt ? "Yes" : "No");
    }
}

static void enet_print_node(enetx_port_t *port, egress_tm_node *node, int offset, u32 flags)
{
    egress_tm_node *tmp;
    char *padding;

    if (!node)
        return;

    padding = kmalloc(offset + 1, GFP_KERNEL);
    if (!padding) {
        egress_tm_err("No memory\n");
        return;
    }

    memset(padding, ' ', offset);
    padding[offset] = '\0';

    if (!(flags & EGRESS_TM_DUMP_FLAG_SHORT)) {
        printk(KERN_INFO "%s%s(%d)", padding, node_dict[node->type].name, node_dict[node->type].type);
        printk(KERN_INFO "%s handle_ext    = 0x%llx (%llx:%llx)\n", padding, (node->handle & EGRESS_TM_HANDLE_ID_MASK),
            EGRESS_TM_HANDLE_ID_HIGH(node->handle), (node->handle & EGRESS_TM_HANDLE_ID_LOW_MASK));
        printk(KERN_INFO "%s handle_int    = 0x%llx\n", padding, node->handle);
        printk(KERN_INFO "%s parentid      = 0x%llx\n", padding, node->parentid);
        printk(KERN_INFO "%s num_of_childs = %d\n", padding, node->num_of_childs);
        printk(KERN_INFO "%s state(%d)      = %s\n", padding, node->state, state_dict[node->state].name);
        printk(KERN_INFO "%s Is internal   = %s\n", padding, EGRESS_TM_HAS_SUBID(node->handle) ? "Yes" : "No");

        if (node_dict[node->type].type == EGRESS_TM_TCONT) {
            printk(KERN_INFO "%s  id         = %d\n", padding, node->ponc.id);
            printk(KERN_INFO "%s  Is control = %s\n", padding, node->ponc.control ? "Yes" : "No");
        } else if (node_dict[node->type].type == EGRESS_TM_ETS_PARENT) {
            int sum = 0;

            enet_update_caps(port);
            if (port->tc_ets_update_stat) {
                port->tc_ets_update_stat(port, node);
            }
            list_for_each_entry(tmp, &node->childs, my_list)
                if (!tmp->queue.quanta)
                    sum++;
            if (sum == node->ets.num_queues) {
                tm_parms.cfgFlags = EGRESS_TM_CAP_SCHED_TYPE_SP;
                printk(KERN_INFO "%s SCHED(%d): Strict priority\n", padding, node->ets.tm_sched_mode);
            } else if (!sum) {
                tm_parms.cfgFlags = EGRESS_TM_CAP_SCHED_TYPE_WRR;
                printk(KERN_INFO "%s SCHED(%d): WRR\n", padding, node->ets.tm_sched_mode);
            } else {
                tm_parms.cfgFlags = EGRESS_TM_CAP_SCHED_TYPE_SP_WRR;
                printk(KERN_INFO "%s SCHED(%d): Strict priority(%d) and WRR(%d)\n",
                    padding, node->ets.tm_sched_mode ,sum, node->ets.num_queues - sum);
            }
            printk(KERN_INFO "%s  type             = %d\n", padding, node->ets.type);
            printk(KERN_INFO "%s  level            = %d\n", padding, node->ets.level);
            printk(KERN_INFO "%s  index            = %lld\n", padding, node->ets.index);
            printk(KERN_INFO "%s  queues           = %d\n", padding, node->ets.num_queues);
            printk(KERN_INFO "%s  num_sp_queues    = %d\n", padding, node->ets.num_sp_queues);
            printk(KERN_INFO "%s  num_subsidiaries = %d\n", padding, node->ets.num_subsidiaries);
            printk(KERN_INFO "%s  Statistics:\n", padding);
            printk(KERN_INFO "%s   Sent    - %lld packets %lld bytes\n", padding, node->ets.sent.packets, node->ets.sent.bytes);
            printk(KERN_INFO "%s   Dropped - %lld packets %lld bytes\n", padding, node->ets.dropped.packets, node->ets.dropped.bytes);

            tm_parms.numQueues = node->ets.num_queues;
            if (flags & EGRESS_TM_DUMP_FLAG_CAP) {
                enet_print_caps(port);
            }
        } else if (node_dict[node->type].type == EGRESS_TM_ETS_CHILD) {
            printk(KERN_INFO "%s EGRESS QUEUE\n", padding);
            printk(KERN_INFO "%s  qid                     = %d\n", padding, node->queue.qid);
            printk(KERN_INFO "%s  priority                = %d\n", padding, node->queue.priority);
            printk(KERN_INFO "%s  priomap                 = %d\n", padding, node->queue.priomap);
            printk(KERN_INFO "%s  quanta                  = %d\n", padding, node->queue.quanta);
            printk(KERN_INFO "%s  weight                  = %d\n", padding, node->queue.weights);
            printk(KERN_INFO "%s  size                    = %d\n", padding, node->queue.size);
            printk(KERN_INFO "%s  reserved_packet_buffers = %d\n", padding, node->queue.reserved_packet_buffers);
            printk(KERN_INFO "%s  has_subsidiaries        = %d\n", padding, node->queue.has_subsidiaries);
            printk(KERN_INFO "%s  Statistics:\n", padding);
            printk(KERN_INFO "%s   Sent    - %lld packets %lld bytes\n", padding, node->queue.sent.packets, node->queue.sent.bytes);
            printk(KERN_INFO "%s   Dropped - %lld packets %lld bytes\n", padding, node->queue.dropped.packets, node->queue.dropped.bytes);
        } else if (node_dict[node->type].type == EGRESS_TM_HTB_CHILD) {
            printk(KERN_INFO "%s RATE LIMITER\n", padding);
            printk(KERN_INFO "%s  rate  = %lld\n", padding, BYTES_TO_BITS(node->htb.rate));
            printk(KERN_INFO "%s  ceil  = %lld\n", padding, BYTES_TO_BITS(node->htb.ceil));
            printk(KERN_INFO "%s  index = %lld\n", padding, node->htb.index);
        } else if (node_dict[node->type].type == EGRESS_TM_RED) {
            printk(KERN_INFO "%s RED\n", padding);
            printk(KERN_INFO "%s  min       = %d\n", padding, node->red.low_class.min);
            printk(KERN_INFO "%s  max       = %d\n", padding, node->red.low_class.max);
            printk(KERN_INFO "%s  drop_prob = %d\n", padding, node->red.low_class.drop_prob);
        } else if (node_dict[node->type].type == EGRESS_TM_GRED) {
            printk(KERN_INFO "%s WRED\n", padding);
            printk(KERN_INFO "%s  low.min        = %d\n", padding, node->red.low_class.min);
            printk(KERN_INFO "%s  low.max        = %d\n", padding, node->red.low_class.max);
            printk(KERN_INFO "%s  low.drop_prob  = %d\n", padding, node->red.low_class.drop_prob);
            printk(KERN_INFO "%s  high.min       = %d\n", padding, node->red.high_class.min);
            printk(KERN_INFO "%s  high.max       = %d\n", padding, node->red.high_class.max);
            printk(KERN_INFO "%s  high.drop_prob = %d\n", padding, node->red.high_class.drop_prob);
        }

        printk(KERN_INFO "\n");
    } else {
        printk(KERN_INFO "%s%s(%d) = %llx:%llx\n", padding, node_dict[node->type].name, node_dict[node->type].type,
                                                   EGRESS_TM_HANDLE_ID_HIGH(node->handle), (node->handle & EGRESS_TM_HANDLE_ID_LOW_MASK));
    }
    kfree(padding);
}

/* general sysfs command */
static void enet_print_node_tree(enetx_port_t *port, egress_tm_node *node, int offset, u32 flags)
{
    egress_tm_node *tmp;

    if (!node)
        return;

    enet_print_node(port, node, offset, flags);
    list_for_each_entry_reverse(tmp, &node->childs, my_list)
        enet_print_node_tree(port, tmp, offset + 4, flags);
}

static void enet_tc_egress_update_tree_state(egress_tm_node *node, egress_tm_node_state state)
{
    egress_tm_node *tmp;

    if (!node)
        return;
    node->state = state;
    list_for_each_entry(tmp, &node->childs, my_list)
        enet_tc_egress_update_tree_state(tmp, state);
}

static void enet_get_port_by_name(enetx_port_t **port, char *name)
{
    struct net_device *dev;

    *port = NULL;
    dev = dev_get_by_name(&init_net, name);
    if (dev) {
        port_by_netdev(dev, port);
        if (!*port) {
            egress_tm_err("%s wrong port\n", name);
        }
        dev_put(dev);
    } else {
        egress_tm_err("%s port not found\n", name);
    }
}

static ssize_t enet_tc_egress_sysfs_init_config(struct kobject *kobj, 
                        struct kobj_attribute *attr, const char *buf, size_t count)
{
    static char name[IFNAMSIZ];
    enetx_port_t *port;

    sscanf(buf,"%16s", name);
    enet_get_port_by_name(&port, name);
    if (port) {
        rtnl_lock();
        if (port->tc_setup_egress_tm) {
            if (port->tc_setup_egress_tm(port, true)) {
                egress_tm_err("%s fail set TM EGRESS\n", name);
                rtnl_unlock();
                return -EINVAL;
            } else {
                enet_tc_egress_update_tree_state(port->root_node, EGRESS_NODE_CONF);
            }
        }
        rtnl_unlock();
    }

    return count;
}

static ssize_t enet_tc_egress_sysfs_modify_config(struct kobject *kobj, 
                        struct kobj_attribute *attr, const char *buf, size_t count)
{
    static char name[IFNAMSIZ];
    enetx_port_t *port;

    sscanf(buf,"%16s", name);
    enet_get_port_by_name(&port, name);
    if (port) {
        rtnl_lock();
        if (port->tc_setup_egress_tm) {
            if (port->tc_setup_egress_tm(port, false)) {
                egress_tm_err("%s fail modify TM EGRESS\n", name);
                rtnl_unlock();
                return -EINVAL;
            } else {
                enet_tc_egress_update_tree_state(port->root_node, EGRESS_NODE_CONF);
            }
        }
        rtnl_unlock();
    }

    return count;
}

static int enet_tc_egress_parse_queue_handle(char *handle_name, u64 *handle) {
    char parent_id[8], queue_id[8];
    unsigned long hex1, hex2;
    int result;

    char *colon_pos = strchr(handle_name, ':');
    if (!colon_pos) {
        egress_tm_err(KERN_ERR "Colon not found in string\n");
        return -EINVAL;
    }

    strncpy(parent_id, handle_name, colon_pos - handle_name);
    parent_id[colon_pos - handle_name] = '\0';

    strncpy(queue_id, colon_pos + 1, sizeof(queue_id) - 1);
    queue_id[sizeof(queue_id) - 1] = '\0';

    result = kstrtoul(parent_id, 16, &hex1);
    if (result) {
        egress_tm_err(KERN_ERR "Failed to convert parent_id to hex\n");
        return result;
    }

    result = kstrtoul(queue_id, 16, &hex2);
    if (result) {
        egress_tm_err(KERN_ERR "Failed to convert queue_id to hex\n");
        return result;
    }

    *handle = EGRESS_TM_HANDLE_BUILD_HANDLE(hex1, hex2);
    *handle = EGRESS_TM_HANDLE(*handle, EGRESS_TM_CLASS_ID);

    return 0;
}

static ssize_t enet_tc_egress_sysfs_print_tree(struct kobject *kobj, 
                        struct kobj_attribute *attr, const char *buf, size_t count)
{
    char name[IFNAMSIZ], opt1[32];
    enetx_port_t *port;
    u32 flags = 0;
    int ret;

    ret = sscanf(buf,"%16s %32s", name, opt1);
    enet_get_port_by_name(&port, name);
    if (!ret) {
        egress_tm_err("Unsupported dump command\n");
        return count;
    } else if (ret == 2) {
        egress_tm_node *entry;
        u64 handle;

        if (!strcmp(opt1, "-s")) {
            flags |= EGRESS_TM_DUMP_FLAG_SHORT;
        } else if (!strcmp(opt1, "capability") || !strcmp(opt1, "cap") || !strcmp(opt1, "-cap")) {
            flags |= EGRESS_TM_DUMP_FLAG_CAP;
        } else if (!strncmp(opt1, "0x", 2)) {
            ret = kstrtoull(opt1, 16, &handle);
            if (ret) {
                egress_tm_err("Failed to node handle\n");
                return count;
            }
            rtnl_lock();
            hash_for_each_possible(port->nodes_hash_table, entry, hnode, handle) {
                if (entry->handle == handle)
                    enet_print_node(port, entry, 0, 0);
            }
            rtnl_unlock();
            return count;
        } else {
            if (!enet_tc_egress_parse_queue_handle(opt1, &handle)) {
                rtnl_lock();
                hash_for_each_possible(port->nodes_hash_table, entry, hnode, handle) {
                    if (entry->handle == handle)
                        enet_print_node(port, entry, 0, 0);
                }

                handle &= ~EGRESS_TM_HANDLE(0, EGRESS_TM_CLASS_ID);
                hash_for_each_possible(port->nodes_hash_table, entry, hnode, handle) {
                    if (entry->handle == handle)
                        enet_print_node(port, entry, 0, EGRESS_TM_DUMP_FLAG_CAP);
                }
                rtnl_unlock();
                return count; 
            }

            egress_tm_err("Unsupported dump command\n");
            return -EINVAL;           
        }
    }

    if (port) {
        rtnl_lock();
        if (flags & EGRESS_TM_DUMP_FLAG_CAP) {
            enet_update_caps(port);
            enet_print_caps(port);
        } else {
            enet_print_node_tree(port, port->root_node, 0, flags);
        }
        rtnl_unlock();
    }

    return count;
}

static ssize_t enet_tc_egress_sysfs_help(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int count = 0;

    count += sprintf(buf, "Egress TM general commands(<> - mandatory, [] - optional) :\n");
    count += sprintf(buf + count, "echo <interface_name> > /sys/kernel/tc_egress/init                          - Initialize Egress TM tree\n");
    count += sprintf(buf + count, "echo <interface_name> > /sys/kernel/tc_egress/modify                        - Modify Egress TM tree\n");
    count += sprintf(buf + count, "echo <interface_name> [-s/X:Y/int_handler/cap]> /sys/kernel/tc_egress/dump  - Dump Egress TM tree\n");

    return count;
}

static struct kobj_attribute print_tree = __ATTR(dump, 0660, enet_tc_egress_sysfs_dump_caps, enet_tc_egress_sysfs_print_tree);
static struct kobj_attribute init_config = __ATTR(init, 0220, NULL, enet_tc_egress_sysfs_init_config);
static struct kobj_attribute modify_config = __ATTR(modify, 0220, NULL, enet_tc_egress_sysfs_modify_config);
static struct kobj_attribute help = __ATTR(help, 0440, enet_tc_egress_sysfs_help, NULL);

static struct kobject *tc_obj_ref;

/* queue sysfs command */
static ssize_t enet_egress_sysfs_queue_res_buf(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    egress_tm_node *entry;
    char name[IFNAMSIZ], handle_name[16];
    u32 reserved_packet_buffers;
    enetx_port_t *port;
    u64 handle;
    int ret;

    ret = sscanf(buf,"%16s %16s %u", name, handle_name, &reserved_packet_buffers);
    if (ret != 3) {
        egress_tm_err("Wrong sysfs input\n");
        return -EINVAL;
    }

    ret = enet_tc_egress_parse_queue_handle(handle_name, &handle);
    if (ret) {
        egress_tm_err("Wrong queue ID\n");
        return -EINVAL;
    }

    enet_get_port_by_name(&port, name);
    if (port) {
        rtnl_lock();
        hash_for_each_possible(port->nodes_hash_table, entry, hnode, handle) {
            if (entry->handle == handle) {
                if (entry->type == EGRESS_TM_ETS_CHILD) {
                    if (entry->state == EGRESS_NODE_CONF && entry->queue.reserved_packet_buffers != reserved_packet_buffers)
                        entry->state = EGRESS_NODE_CHANGED;
                    entry->queue.reserved_packet_buffers = reserved_packet_buffers;
                    rtnl_unlock();
                    return count;
                } else {
                    egress_tm_err("%s node is not ETS queue node\n", name);
                }
            }
        }
        rtnl_unlock();
        egress_tm_err("Port(%s) queue %lld not found\n", name, EGRESS_TM_HANDLE_ID2QUEUE(handle));
    }

    return -EINVAL;
}

static ssize_t enet_egress_sysfs_queue_size(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    egress_tm_node *entry;
    char name[IFNAMSIZ], handle_name[16];
    enetx_port_t *port;
    u64 handle;
    int size;
    int ret;

    ret = sscanf(buf,"%16s %16s %d", name, handle_name, &size);
    if (ret != 3) {
        egress_tm_err("Wrong sysfs input\n");
        return -EINVAL;
    }

    ret = enet_tc_egress_parse_queue_handle(handle_name, &handle);
    if (ret) {
        egress_tm_err("Wrong queue ID\n");
        return -EINVAL;
    }

    enet_get_port_by_name(&port, name);

    if (size < 0 && size != -1) {
        egress_tm_err("Wrong queue size\n");
        return -EINVAL;
    }

    if (port) {
        rtnl_lock();
        hash_for_each_possible(port->nodes_hash_table, entry, hnode, handle) {
            if (entry->handle == handle) {
                if (entry->type == EGRESS_TM_ETS_CHILD) {
                    if (entry->state == EGRESS_NODE_CONF && entry->queue.size != size)
                        entry->state = EGRESS_NODE_CHANGED;
                    if (size == -1)
                        size = enet_port_def_queue_size(port->dev, entry->queue.priority);
                    entry->queue.size = size;
                    rtnl_unlock();
                    return count;
                } else {
                    egress_tm_err("%s node is not ETS queue node\n", name);
                }
            }
        }
        rtnl_unlock();
        egress_tm_err("Port(%s) queue %lld not found\n", name, EGRESS_TM_HANDLE_ID2QUEUE(handle));
    }

    return -EINVAL;
}

static ssize_t enet_egress_sysfs_get_queue_size_write(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    egress_tm_node *entry;
    char name[IFNAMSIZ], handle_name[16];
    enetx_port_t *port;
    u64 handle;
    int ret;

    ret = sscanf(buf,"%16s %16s", name, handle_name);
    if (ret != 2) {
        egress_tm_err("Wrong sysfs input\n");
        return -EINVAL;
    }

    ret = enet_tc_egress_parse_queue_handle(handle_name, &handle);
    if (ret) {
        egress_tm_err("Wrong queue ID\n");
        return -EINVAL;
    }

    enet_get_port_by_name(&port, name);

    if (port) {
        rtnl_lock();
        hash_for_each_possible(port->nodes_hash_table, entry, hnode, handle) {
            if (entry->handle == handle) {
                if (entry->type == EGRESS_TM_ETS_CHILD) {
                    queue_size_sysfs = entry->queue.size;
                    rtnl_unlock();
                    return count;
                } else {
                    egress_tm_err("%s node is not ETS queue node\n", name);
                }
            }
        }
        rtnl_unlock();
        egress_tm_err("Port(%s) queue %lld not found\n", name, EGRESS_TM_HANDLE_ID2QUEUE(handle));
    }

    return -EINVAL;
}

static ssize_t enet_egress_sysfs_get_queue_size_read(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%u", queue_size_sysfs);
}

static ssize_t enet_egress_sysfs_set_def_queue_size(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct egress_tm_speed_dict *node;
    u32 type, speed, queue_size, prio;
    u64 key;
    int i;

    if (sscanf(buf,"%u %u %u %u", &type, &speed, &prio, &queue_size) != 4) {
        egress_tm_err("Wrong sysfs input\n");
        return -EINVAL;
    }
 
    if (type >= EGRESS_DEV_MAX) {
        egress_tm_err("Wrong interface type\n");
        goto out;
    }

    if (type == EGRESS_DEV_SVCQ) {
        egress_tm_err("Service queue default queue size change not supported\n");
        goto out;        
    }

    if (prio >= EGRESS_MAX_PRIO) {
        egress_tm_err("Wrong priority, max priority is %d\n", EGRESS_MAX_PRIO - 1);
        goto out;
    }

    for (i = 0; i < EGRESS_MAX_SPEEDS; i++) {
        if (speed_dict[i].speed == speed)
            break;
    }
    if (i == EGRESS_MAX_SPEEDS) {
        egress_tm_err("Unsupported speed %d\n", i);
        goto out;
    }

    rtnl_lock();
    key = EGRESS_DEF_SPEED_KEY(type, speed, prio);
    /* Check if exist */
    hash_for_each_possible(def_speed_hash_table, node, hnode, key) {
        if (EGRESS_DEF_SPEED_KEY(node->type, node->speed, node->prio) == key) {
            node->queue_size = queue_size;
            rtnl_unlock();
            return count;
        }
    }
    /* Add new entry */
    node = kcalloc(1, sizeof(struct egress_tm_speed_dict), GFP_KERNEL);
    if (!node) {
        egress_tm_err("No memory\n");
        rtnl_unlock();
        return count;
    }
    node->type = type;
    node->speed = speed;
    node->prio = prio;
    node->queue_size = queue_size;
    hash_add(def_speed_hash_table, &node->hnode, EGRESS_DEF_SPEED_KEY(type, speed, prio));
    rtnl_unlock();

    return count;

out:
    return -EINVAL;
}

static ssize_t enet_egress_sysfs_get_def_queue_size(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct egress_tm_speed_dict *node;
    bool print_def;
    u64 key;
    int i;

    printk(KERN_INFO "Default Queue sizes:\n");
    for (i = 0; i < EGRESS_MAX_DEF_SPEEDS; i++) {
        /* Skip GPON and EPON - 10 and 100 speeds */
        if (((i/(EGRESS_MAX_PRIO*EGRESS_MAX_SPEEDS) == EGRESS_DEV_EPON) || (i/(EGRESS_MAX_PRIO*EGRESS_MAX_SPEEDS) == EGRESS_DEV_GPON)) &&
            ((speed_dict[(i/EGRESS_MAX_PRIO)%EGRESS_MAX_SPEEDS].speed  == SPEED_10) || (speed_dict[(i/EGRESS_MAX_PRIO)%EGRESS_MAX_SPEEDS].speed == SPEED_100)))
            continue;

        print_def = true;
        key = EGRESS_DEF_SPEED_KEY(i/(EGRESS_MAX_PRIO*EGRESS_MAX_SPEEDS), speed_dict[(i/EGRESS_MAX_PRIO)%EGRESS_MAX_SPEEDS].speed, i%EGRESS_MAX_PRIO);
        hash_for_each_possible(def_speed_hash_table, node, hnode, key) {
            if (EGRESS_DEF_SPEED_KEY(node->type, node->speed, node->prio) == key) {
                printk(KERN_INFO "Port_type(%s)\tspeed = %d prio = %d\tQueue Size=%d\n", port_type_string[node->type], node->speed, node->prio, node->queue_size);
                print_def = false;

            }
        }
        if (!print_def)
            continue;

        /* Not found, get 0 queue default */
        key = EGRESS_DEF_SPEED_KEY(i/(EGRESS_MAX_PRIO*EGRESS_MAX_SPEEDS), speed_dict[(i/EGRESS_MAX_PRIO)%EGRESS_MAX_SPEEDS].speed, 0);
        hash_for_each_possible(def_speed_hash_table, node, hnode, key) {
            if (EGRESS_DEF_SPEED_KEY(node->type, node->speed, node->prio) == key)
                printk(KERN_INFO "Port_type(%s)\tspeed = %d prio = %d\tQueue Size=%d\n", port_type_string[node->type], node->speed, i%EGRESS_MAX_PRIO, node->queue_size);
        }
    }

    return 0;
}

static ssize_t enet_tc_egress_sysfs_queue_help(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int count = 0;

    count += sprintf(buf, "Egress TM queue commands:\n");
    count += sprintf(buf + count, "echo <interface type> <speed> <priority> <new queue size> > /sys/kernel/tc_egress/queue_config/default_sizes      - Change default queue size\n");
    count += sprintf(buf + count, "echo <interface name> <queue handle ID X:Y> <value> > /sys/kernel/tc_egress/queue_config/size                     - Change specific queue size\n");
    count += sprintf(buf + count, "echo <interface name> <queue handle ID X:Y> > /sys/kernel/tc_egress/queue_config/get_size                         - Get specific queue size\n");
    count += sprintf(buf + count, "cat /sys/kernel/tc_egress/queue_config/get_size                                                                   - Return specific queue size\n");
    count += sprintf(buf + count, "echo <interface name> <queue handle ID X:Y> <value> > /sys/kernel/tc_egress/queue_config/reserved_packet_buffers  - Change specific queue reserved packet buffers\n");

    return count;
}

static struct kobj_attribute queue_reserved_packet_buffers = __ATTR(reserved_packet_buffers, 0220, NULL, enet_egress_sysfs_queue_res_buf);
static struct kobj_attribute queue_size = __ATTR(size, 0220, NULL, enet_egress_sysfs_queue_size);
static struct kobj_attribute get_size = __ATTR(get_size, 0660, enet_egress_sysfs_get_queue_size_read, enet_egress_sysfs_get_queue_size_write);
static struct kobj_attribute def_queue_size = __ATTR(default_sizes, 0660, enet_egress_sysfs_get_def_queue_size, enet_egress_sysfs_set_def_queue_size);
static struct kobj_attribute queue_help = __ATTR(help, 0440, enet_tc_egress_sysfs_queue_help, NULL);

static struct kobject *queue_obj_ref;

static struct attribute *queue_attrs[] = {
    &queue_reserved_packet_buffers.attr,
    &queue_size.attr,
    &get_size.attr,
    &def_queue_size.attr,
    &queue_help.attr,
    NULL,
};

static struct attribute_group queue_attr_group = {
    .attrs = queue_attrs,
};

void enet_create_def_speeds_hash_table(void)
{
    int i;

    /* populate only 0 priority */
    for (i = 0; i < (sizeof(speed_dict)/sizeof(struct egress_tm_speed_dict)); i++) {
        hash_add(def_speed_hash_table, &speed_dict[i].hnode, EGRESS_DEF_SPEED_KEY(speed_dict[i].type, speed_dict[i].speed, speed_dict[i].prio));
    }
}

void enet_create_tc_egress_sysfs(void)
{
    int ret;

    /* Creating a directory in /sys/kernel/ */
    tc_obj_ref = kobject_create_and_add("tc_egress", kernel_kobj);
 
    /* Creating /sys/kernel/tc_egress sysfs files for etx_value */
    if (sysfs_create_file(tc_obj_ref, &print_tree.attr))
        egress_tm_err("Cannot create dump sysfs file\n");
    if (sysfs_create_file(tc_obj_ref, &init_config.attr))
        egress_tm_err("Cannot create init sysfs file\n");
    if (sysfs_create_file(tc_obj_ref, &modify_config.attr))
        egress_tm_err("Cannot create modify sysfs file\n");
    if (sysfs_create_file(tc_obj_ref, &help.attr))
        egress_tm_err("Cannot create help sysfs file\n");

    /* Creating a directory in /sys/kernel/tc_egress/queue_config */
    queue_obj_ref = kobject_create_and_add("queue_config", tc_obj_ref);

    /* Creating /sys/kernel/tc_egress/queue_config sysfs files for etx_value */
    ret = sysfs_create_group(queue_obj_ref, &queue_attr_group);
    if (ret) {
        egress_tm_err("Cannot create /sys/kernel/tc_egress/queue_config/ sysfs group\n");
        sysfs_remove_file(tc_obj_ref, &print_tree.attr);
        sysfs_remove_file(tc_obj_ref, &init_config.attr);
        sysfs_remove_file(tc_obj_ref, &modify_config.attr);
        sysfs_remove_file(tc_obj_ref, &help.attr);
        kobject_put(queue_obj_ref); 
        kobject_put(tc_obj_ref);       
    }

    enet_create_def_speeds_hash_table();
}

void enet_remove_tc_egress_sysfs(void)
{
    if (queue_obj_ref) {
        sysfs_remove_group(queue_obj_ref, &queue_attr_group);
        kobject_put(queue_obj_ref);
    }
    if (tc_obj_ref) {
        sysfs_remove_file(tc_obj_ref, &print_tree.attr);
        sysfs_remove_file(tc_obj_ref, &init_config.attr);
        sysfs_remove_file(tc_obj_ref, &modify_config.attr);
        kobject_put(tc_obj_ref);
    }
}

static int enet_create_child_node(enetx_port_t *port, egress_tm_node **entry, egress_tm_node *parent,
                                  u64 handle, egress_tm_node_type type)
{
    egress_tm_node *node;

    *entry = NULL;
    if (!parent) {
        egress_tm_err("Parent node of TM wasn't configured\n");
        return -EINVAL;
    }

    list_for_each_entry(node, &parent->childs, my_list) {
        if (node->handle == handle) {
            *entry = node;
            return EEXIST;
        }
    }

    hash_for_each_possible(port->nodes_hash_table, node, hnode, handle) {
        if (node->handle ==handle) {
            egress_tm_warn("Node handle=%lld already in hash\n", node->handle);
        }
    }

    node = kzalloc(sizeof(egress_tm_node), GFP_KERNEL);
    if (!node)
        return -ENOMEM;

    *entry = node;
    node->parent = parent;
    node->parentid = parent->handle;
    node->handle = handle;
    node->type = type;
    node->state = EGRESS_NODE_NEW;
    INIT_LIST_HEAD(&node->childs);

    list_add(&node->my_list, &parent->childs);
    hash_add(port->nodes_hash_table, &node->hnode, node->handle);
    parent->num_of_childs++;

    return 0;
}

static egress_tm_node *enet_get_up_level_ets(egress_tm_node *entry)
{
    egress_tm_node *par_entry = entry->parent; /* parent CLASS */

    /* Check if root node */
    if (!par_entry)
        return NULL;

    /* Check if parent is ETS(queue) and return parent */
    if (par_entry->type == EGRESS_TM_ETS_CHILD)
        return par_entry;

    /* Check if grandparent is ETS(queue) and return parent */
    par_entry = par_entry->parent; /* parent Qdisc */
    par_entry = par_entry->parent; /* parent ETS CLASS? */
    if (par_entry && par_entry->type == EGRESS_TM_ETS_CHILD)
        return par_entry;    

    /* not supported topology */
    return NULL;
}

static int enet_delete_child_node(enetx_port_t *port, egress_tm_node *parent, u64 handle)
{
    egress_tm_node *node, *tmp;

    if (!parent) {
        egress_tm_err("Parent node of TM wasn't configured\n");
        return -EINVAL;           
    }

    list_for_each_entry_safe(node, tmp, &parent->childs, my_list) {
        if (handle == node->handle) {
            if (!list_empty(&node->childs)) {
                egress_tm_node *node_child, *tmp_child;

                egress_tm_warn("node(%lld) has childs=%d\n", node->handle, node->num_of_childs);
                list_for_each_entry_safe(node_child, tmp_child, &node->childs, my_list) {
                    enet_delete_child_node(port, node, node_child->handle);
                }
            }
            if (port->tc_remove_egress_tm)
                port->tc_remove_egress_tm(port, node);

            if (node->type == EGRESS_TM_ETS_PARENT) {
                egress_tm_node *par_entry;

                /* Check if ETS is first or secondary level queues */
                par_entry = enet_get_up_level_ets(node);
                if (par_entry) {
                    par_entry->parent->ets.num_subsidiaries--;
                    par_entry->queue.has_subsidiaries = false;
                    if (par_entry->state == EGRESS_NODE_CONF)
                        par_entry->state = EGRESS_NODE_CHANGED;
                }
            }


            list_del(&node->my_list);
            hash_del(&node->hnode);
            kfree(node);
            parent->num_of_childs--;
            return 0;
        }
    }

    egress_tm_info("Cannot find and delete handle %lld\n", handle);
    return 0;
}

static int enet_create_root_node(enetx_port_t *port, u64 handle, egress_tm_node_type type)
{
    if (port->root_node) {
        egress_tm_err("Root node already exist\n");
        return -EINVAL;
    }

    port->root_node = kzalloc(sizeof(egress_tm_node), GFP_KERNEL);
    if (!port->root_node)
        return -ENOMEM;
    
    port->root_node->parentid = TC_H_ROOT;
    port->root_node->handle = handle;
    port->root_node->type = type;
    port->root_node->state = EGRESS_NODE_NEW;
    INIT_LIST_HEAD(&port->root_node->childs);
    hash_add(port->nodes_hash_table, &port->root_node->hnode, port->root_node->handle);

    return 0;
}

static int enet_delete_root_node(enetx_port_t *port)
{
    if (!port->root_node) {
        egress_tm_err("Root node of TM wasn't configured\n");
        return -EINVAL;           
    }

    if (!list_empty(&port->root_node->childs)) {
        egress_tm_node *node, *tmp;

        egress_tm_warn("Root node has childs=%d\n", port->root_node->num_of_childs);
        egress_tm_warn("Please remove them!!!\n");
        list_for_each_entry_safe(node, tmp, &port->root_node->childs, my_list) {
           enet_delete_child_node(port, port->root_node, node->handle);
        }
    }

    if (port->tc_remove_egress_tm)
        port->tc_remove_egress_tm(port, port->root_node);

    hash_del(&port->root_node->hnode);
    kfree(port->root_node);
    port->root_node = NULL;

    return 0;
}

static int enet_port_get_speed_and_type(struct net_device *dev, u32 *speed, u32 *type)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    phy_dev_t *phy_dev = port->p.phy;
    u32 supported = 0;

    if (!phy_dev) {
        return -EINVAL; 
    }

    phy_dev = cascade_phy_get_last_active(phy_dev);
    /* Fetch upstream speed */
    cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &supported);

    *speed = (supported&PHY_CAP_10000)? SPEED_10000 :
        (supported&PHY_CAP_5000)? SPEED_5000 :
        (supported&PHY_CAP_2500)? SPEED_2500 :
        (supported&(PHY_CAP_1000_FULL|PHY_CAP_1000_HALF))? SPEED_1000 :
        (supported&(PHY_CAP_100_FULL|PHY_CAP_100_HALF))? SPEED_100 :
        (supported&(PHY_CAP_10_FULL|PHY_CAP_10_HALF))? SPEED_10 : SPEED_UNKNOWN;

    if (port->port_type == PORT_TYPE_RUNNER_GPON) {
        *type = EGRESS_DEV_GPON;
    } else if (port->port_type == PORT_TYPE_RUNNER_EPON) {
        *type = EGRESS_DEV_EPON;
    } else if (port->port_type == PORT_TYPE_RUNNER_PORT) {
        *type = EGRESS_DEV_ETH;
    } else if (port->port_type == PORT_TYPE_RUNNER_SW) {
        *type = EGRESS_DEV_SVCQ;
    } else {
        return -EINVAL; 
    }

    return 0; 
}

static u32 enet_port_def_queue_size(struct net_device *dev, u8 prio)
{
    struct egress_tm_speed_dict *node;
    u32 speed, type;
    u64 key;

    if (enet_port_get_speed_and_type(dev, &speed, &type))
        goto err;

    if (type == EGRESS_DEV_SVCQ)
        return EGRESS_TM_DF_QUEUE_THRESHOLD;

    key = EGRESS_DEF_SPEED_KEY(type, speed, prio);
    hash_for_each_possible(def_speed_hash_table, node, hnode, key) {
        if (EGRESS_DEF_SPEED_KEY(node->type, node->speed, node->prio) == key) {
            return node->queue_size;
        }
    }

    /* Get 0 priority if not set */
    key = EGRESS_DEF_SPEED_KEY(type, speed, 0);
    hash_for_each_possible(def_speed_hash_table, node, hnode, key) {
        if (EGRESS_DEF_SPEED_KEY(node->type, node->speed, node->prio) == key) {
            return node->queue_size;
        }
    }

err:
    egress_tm_err("Port %s queue size not found, rollback to default\n", dev->name);
    return EGRESS_TM_DF_QUEUE_THRESHOLD;
}

int enet_setup_pon(struct net_device *dev, struct tc_pon_qopt_offload *qopt)
{
#if !defined(BCM6885) /* CRDP-TODO */
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *entry;
    u64 handle;
    int ret;

    if (port->port_type != PORT_TYPE_RUNNER_GPON && port->port_type != PORT_TYPE_RUNNER_EPON) {
        egress_tm_err("%s not PON port\n", dev->name);
        return -EINVAL;
    }

    if (qopt->parent == TC_H_ROOT)
        handle = EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_QDISC_ID);
    else
        handle = EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_CLASS_ID);

    if (qopt->command == TC_PON_REPLACE) {
        if (qopt->parent == TC_H_ROOT)
            return enet_create_root_node(port, handle, EGRESS_TM_PON);

        ret = enet_create_child_node(port, &entry, port->root_node, handle, EGRESS_TM_TCONT);
        if (ret)
            return ret;

        entry->ponc.id = qopt->handle & EGRESS_TM_TCONT_ID_MASK;
        entry->ponc.control = qopt->control;
        if (entry->ponc.control)
            entry->ponc.id -= EGRESS_TM_CONTROL_LLID_OFFS;
    } else {
        if (qopt->parent == TC_H_ROOT)
            enet_delete_root_node(port);
        else
            enet_delete_child_node(port, port->root_node, handle);
    }
#endif
    return 0;
}

static int enet_ets_replace(struct net_device *dev, struct tc_ets_qopt_offload *qopt, u64 parent_handle, u64 handle, u64 child_handle)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *par_entry, *entry = NULL;
    int i, ret, sp_queues = 0, qid_offset = 0, num_of_queues_orig;
    bool weight_scale_down = false;
    int q_sum = 0;

    if (qopt->parent == TC_H_ROOT) {
        if (!(port->root_node && port->root_node->type == EGRESS_TM_ETS_PARENT)) {
            ret = enet_create_root_node(port, handle, EGRESS_TM_ETS_PARENT);
            if (ret)
                return ret;
        } else if (port->root_node->type != EGRESS_TM_ETS_PARENT) {
            return -EINVAL;
        }
        entry = port->root_node;
    } else {
        /* Parent of not root ETS is always CLASS */
        hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
            if (par_entry->handle == parent_handle) {
                enet_create_child_node(port, &entry, par_entry, handle, EGRESS_TM_ETS_PARENT);
                goto match_found;
            }
        }
    }
match_found:
    if (!entry) {
        egress_tm_err("Root node of TM not found\n");
        return -EINVAL;       
    }

    /* Check if ETS is first or secondary level queues */
    par_entry = enet_get_up_level_ets(entry);
    if (par_entry) {
        if (par_entry->parent->ets.level > 0) {
            egress_tm_err("Third level queues not supported\n");
            return -EINVAL;
        }
        entry->ets.level = par_entry->parent->ets.level + 1;
        par_entry->parent->ets.num_subsidiaries++;
        par_entry->queue.has_subsidiaries = true;
        if (par_entry->state == EGRESS_NODE_CONF)
            par_entry->state = EGRESS_NODE_CHANGED;
    } else {
        entry->ets.level = 0;
    }

    if (entry->ets.level)
        qid_offset = EGRESS_TM_QUEUE_OFFSET(qopt->handle);

    /* Create nodes for queues */
    par_entry = entry;
    num_of_queues_orig = par_entry->num_of_childs;

	for (i = 0; i < qopt->replace_params.bands; i++) {
        if (qopt->replace_params.quanta[i] >= 100)
            weight_scale_down = true;
        q_sum += qopt->replace_params.quanta[i];
    }

    for (i = 0; i < qopt->replace_params.bands; i++) {
        /* Child handle in ETS parent handle + qid */
        ret = enet_create_child_node(port, &entry, par_entry, child_handle + i, EGRESS_TM_ETS_CHILD);
        if (ret == EEXIST && entry && entry->state == EGRESS_NODE_CONF) {
            if ((entry->queue.qid != i) || (entry->queue.weights != qopt->replace_params.weights[i]))
                entry->state = EGRESS_NODE_CHANGED;
        } else if (ret && ret != EEXIST) {
            return ret;
        }
        entry->queue.qid = i + qid_offset;
        entry->queue.priority = i;
        entry->queue.reserved_packet_buffers = EGRESS_TM_DF_RES_BUFFERS;
        /* TODO: use priority from TC ETS command */
        if (ret != EEXIST)
            entry->queue.size = enet_port_def_queue_size(dev, i);
        entry->queue.priomap = qopt->replace_params.priomap[i];
        entry->queue.quanta = qopt->replace_params.quanta[i];
        if (!entry->queue.quanta) {
            sp_queues++;
            continue;
        }

        entry->queue.weights = !weight_scale_down ? qopt->replace_params.quanta[i] : ((qopt->replace_params.quanta[i] * (int)100) / q_sum);
        /* minimum value for weights is 1 */
        entry->queue.weights = entry->queue.weights ? entry->queue.weights : 1;
    }

    for (i = qopt->replace_params.bands; i < num_of_queues_orig; i++) {
        /* Child handle in ETS parent handle + qid */
        enet_delete_child_node(port, par_entry, child_handle + i);
    }
    par_entry->ets.num_queues = qopt->replace_params.bands;
    if (sp_queues == qopt->replace_params.bands)
        par_entry->ets.tm_sched_mode = EGRESS_TM_SP;
    else if (!sp_queues)
        par_entry->ets.tm_sched_mode = EGRESS_TM_WRR;
    else
        par_entry->ets.tm_sched_mode = EGRESS_TM_SP_WRR;
    par_entry->ets.num_sp_queues = sp_queues;

    return 0;
}

static int enet_ets_destroy(struct net_device *dev, struct tc_ets_qopt_offload *qopt, u64 parent_handle, u64 handle, u64 child_handle)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *par_entry, *entry = NULL;
    int i;

    hash_for_each_possible(port->nodes_hash_table, entry, hnode, handle) {
        if (entry->handle == handle) {
            /* Child handle in ETS parent handle + qid */
            for (i = 0; i < entry->ets.num_queues; i++)
                enet_delete_child_node(port, entry, child_handle + i);
            break;
        }
    }

    if (qopt->parent == TC_H_ROOT)
        return enet_delete_root_node(port);

    hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
        if (par_entry->handle == parent_handle) {
            return enet_delete_child_node(port, par_entry, handle);
        }
    }
    egress_tm_warn("Cannot find and delete handle %d\n", qopt->handle);

    return 0;
}

int enet_setup_ets(struct net_device *dev, struct tc_ets_qopt_offload *qopt)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *entry = NULL;
    u64 parent_handle, handle, child_handle;

    /* Only replace/remove supported */
    if (qopt->command != TC_ETS_REPLACE && qopt->command != TC_ETS_DESTROY && qopt->command != TC_ETS_STATS &&
        qopt->command != TC_ETS_CLASS_STATS)
        return 0;

    parent_handle = EGRESS_TM_HANDLE(qopt->parent, EGRESS_TM_CLASS_ID);
    handle = EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_QDISC_ID);
    child_handle = EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_CLASS_ID);

    if (qopt->command == TC_ETS_REPLACE) {
        return enet_ets_replace(dev, qopt, parent_handle, handle, child_handle);
    } else if (qopt->command == TC_ETS_DESTROY) {
        enet_ets_destroy(dev, qopt, parent_handle, handle, child_handle);
    } else {
        u64 ets_handle;

        /* find ETS parrent node handle */
        if (qopt->command == TC_ETS_CLASS_STATS) {
            handle = EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_CLASS_ID);
            ets_handle = EGRESS_TM_HANDLE(qopt->parent, EGRESS_TM_QDISC_ID);
        } else {
            ets_handle = handle;
        }

        hash_for_each_possible(port->nodes_hash_table, entry, hnode, ets_handle) {
            if (entry->handle == ets_handle) {
                if (port->tc_ets_update_stat) {
                    port->tc_ets_update_stat(port, entry);
                }
                /* Check if queue statistic */
                if (qopt->command == TC_ETS_CLASS_STATS) {
                    egress_tm_node *child;

                    list_for_each_entry(child, &entry->childs, my_list)
                    {
                        if (child->handle == handle) {
                            qopt->stats.bstats->packets = child->queue.sent.packets;
                            qopt->stats.bstats->bytes = child->queue.sent.bytes;
                            qopt->stats.qstats->drops = child->queue.dropped.packets;
                            qopt->stats.qstats->drops_bytes = child->queue.dropped.bytes;
                            return 0;
                        }
                    }
                    break;
                }
                qopt->stats.bstats->packets = entry->ets.sent.packets;
                qopt->stats.bstats->bytes = entry->ets.sent.bytes;
                qopt->stats.qstats->drops = entry->ets.dropped.packets;
                qopt->stats.qstats->drops_bytes = entry->ets.dropped.bytes;

                return 0;
            }
        }
        egress_tm_warn("Cannot find handle %d, statistic fetch failed\n", qopt->handle);
    }

    return 0;
}

int enet_setup_htb(struct net_device *dev, struct tc_htb_qopt_offload *qopt)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *par_entry, *entry = NULL;
    egress_tm_node_type type;
    u64 parent_handle, handle;

    if (qopt->command != TC_HTB_CREATE && qopt->command != TC_HTB_DESTROY && qopt->command != TC_HTB_LEAF_ALLOC_QUEUE && qopt->command != TC_HTB_LEAF_DEL &&
        qopt->command != TC_HTB_NODE_MODIFY)
        return 0;

    if (qopt->command == TC_HTB_LEAF_ALLOC_QUEUE || qopt->command == TC_HTB_NODE_MODIFY || qopt->command == TC_HTB_LEAF_DEL) {
        type = EGRESS_TM_HTB_CHILD;
        parent_handle = EGRESS_TM_HANDLE(qopt->parent_classid, EGRESS_TM_QDISC_ID);
        handle = EGRESS_TM_HANDLE(qopt->classid, EGRESS_TM_CLASS_ID);
    } else {
        type = EGRESS_TM_HTB_PARENT;
        parent_handle = EGRESS_TM_HANDLE(qopt->parent_classid, EGRESS_TM_CLASS_ID);
        handle = EGRESS_TM_HANDLE(qopt->classid, EGRESS_TM_QDISC_ID);
    }

    if (qopt->command != TC_HTB_DESTROY && qopt->command != TC_HTB_LEAF_DEL) {
        if (qopt->parent_classid == TC_H_ROOT) {
            if (qopt->command == TC_HTB_LEAF_ALLOC_QUEUE)
                return -EINVAL;
            enet_create_root_node(port, handle, EGRESS_TM_HTB_PARENT);
        } else {
            if (qopt->command != TC_HTB_NODE_MODIFY) {
                hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
                    if (par_entry->handle == parent_handle) {
                        if (par_entry->num_of_childs && qopt->command != TC_HTB_NODE_MODIFY) {
                            egress_tm_err("dev=%s Only one HTB child supported\n", dev->name);
                            return -EOPNOTSUPP;
                        }
                        enet_create_child_node(port, &entry, par_entry, handle, type);
                    }
                }
            } else {
                hash_for_each_possible(port->nodes_hash_table, entry, hnode, handle) {
                    if (entry->handle == handle) {
                        if (entry->state != EGRESS_NODE_NEW)
                            entry->state = EGRESS_NODE_CHANGED;
                        goto match_found;
                    }
                }
                egress_tm_err("Handle %d not found\n", qopt->classid);
                return -EINVAL;            
            }
match_found:
            if (!entry) {
                egress_tm_err("Handle %d not found\n", qopt->classid);
                return -ENOMEM;
            }
            if (qopt->command == TC_HTB_LEAF_ALLOC_QUEUE || qopt->command == TC_HTB_NODE_MODIFY) {
                entry->htb.rate = qopt->rate;
                if (qopt->ceil < qopt->rate)
                    entry->htb.ceil = qopt->rate;
                else
                    entry->htb.ceil = qopt->ceil;
                
                if (qopt->command == TC_HTB_LEAF_ALLOC_QUEUE)
                    entry->htb.index = -1;
            }
        }
    } else {
        if (qopt->parent_classid == TC_H_ROOT)
            return enet_delete_root_node(port);

        hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
            if (par_entry->handle == parent_handle) {
                return enet_delete_child_node(port, par_entry, handle);
            }
        }
        egress_tm_warn("Cannot find and delete handle %d\n", qopt->classid);
    }

    return 0;
}

static egress_tm_node *enet_drop_algo2ets(egress_tm_node *entry)
{
    egress_tm_node *par_entry = entry->parent; /* parent CLASS */

    /* Check if RED parent is ETS(queue) */
    if (par_entry->type == EGRESS_TM_ETS_CHILD)
        return par_entry;

    /* Check if RED grandparent is ETS(queue) */
    par_entry = par_entry->parent; /* parent Qdisc */
    par_entry = par_entry->parent; /* parent ETS CLASS? */
    if (par_entry->type == EGRESS_TM_ETS_CHILD)
        return par_entry;    

    /* not supported topology */
    return NULL;
}

int enet_setup_red(struct net_device *dev, struct tc_red_qopt_offload *qopt)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *par_entry, *entry = NULL, *ets_node;
    u64 parent_handle, handle;
    int ret;

    /* Only replace/remove supported */
    if (qopt->command != TC_RED_REPLACE && qopt->command != TC_RED_DESTROY)
        return 0;

    if (qopt->handle == TC_H_ROOT) {
        egress_tm_err("RED cannot be root\n");
        return -EINVAL;
    }

    parent_handle = EGRESS_TM_HANDLE(qopt->parent, EGRESS_TM_CLASS_ID);
    handle = EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_QDISC_ID);

    if (qopt->command == TC_RED_REPLACE) {
        /* Parent of RED is always CLASS */
        hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
            if (par_entry->handle == parent_handle) {
                ret = enet_create_child_node(port, &entry, par_entry, handle, EGRESS_TM_RED);
                goto match_found;
            }
        }
match_found:
        if (!entry) {
            egress_tm_err("Parent of node not found\n");
            return -EINVAL;       
        }

        ets_node = enet_drop_algo2ets(entry);
        if (!ets_node) {
            enet_delete_child_node(port, par_entry, handle);
            egress_tm_err("RED parent or grandparent should be ETS CLASS\n");
            return -EINVAL; 
        }
        if (ets_node->state == EGRESS_NODE_CONF)
            ets_node->state = EGRESS_NODE_CHANGED;

        if (ret == EEXIST && entry->state == EGRESS_NODE_CONF) {
            if (entry->red.low_class.max != qopt->set.max || entry->red.low_class.min != qopt->set.min || entry->red.low_class.drop_prob == qopt->set.probability)
                entry->state = EGRESS_NODE_CHANGED;
        } else if (ret && ret != EEXIST) {
            return ret;
        }

        entry->handle = handle;
        entry->parentid = parent_handle;
        entry->red.low_class.max = qopt->set.max;
        entry->red.low_class.min = qopt->set.min;
        entry->red.low_class.drop_prob = qopt->set.probability;
    } else {
        hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
            if (par_entry->handle == parent_handle) {
                return enet_delete_child_node(port, par_entry, handle);
            }
        }
        egress_tm_warn("Cannot find and delete handle %d\n", qopt->handle);
    }

    return 0;
}

static bool enet_gred_changed(egress_tm_node *entry, struct tc_gred_qopt_offload_params set)
{
    if (entry->red.low_class.max != set.tab[0].max || entry->red.low_class.min != set.tab[0].min ||
        entry->red.low_class.drop_prob == set.tab[0].probability ||
        entry->red.high_class.max != set.tab[1].max || entry->red.high_class.min != set.tab[1].min ||
        entry->red.high_class.drop_prob == set.tab[1].probability) 
    {
        return true;
    }

    return false;
}

int enet_setup_gred(struct net_device *dev, struct tc_gred_qopt_offload *qopt)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *par_entry, *entry = NULL, *ets_node;
    u64 parent_handle, handle;
    int ret;

    /* Only replace/remove supported */
    if (qopt->command != TC_GRED_REPLACE && qopt->command != TC_GRED_DESTROY)
        return 0;

    if (qopt->handle == TC_H_ROOT) {
        egress_tm_err("GRED cannot be root\n");
        return -EINVAL;
    }

    parent_handle = EGRESS_TM_HANDLE(qopt->parent, EGRESS_TM_CLASS_ID);
    handle = EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_QDISC_ID);

    if (qopt->command == TC_GRED_REPLACE) {
        if (qopt->set.dp_cnt != EGRESS_TM_WRED_CLASSES) {
            egress_tm_err("GRED support only %d vqs\n", EGRESS_TM_WRED_CLASSES);
            return -EINVAL;
        }

        /* Parent of GRED is always CLASS */
        hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
            if (par_entry->handle == parent_handle) {
                ret = enet_create_child_node(port, &entry, par_entry, handle, EGRESS_TM_GRED);
                goto match_found;
            }
        }
match_found:
        if (!entry) {
            egress_tm_err("Parent of node not found\n");
            return -EINVAL;       
        }

        ets_node = enet_drop_algo2ets(entry);
        if (!ets_node) {
            enet_delete_child_node(port, par_entry, handle);
            egress_tm_err("GRED parent or grandparent should be ETS CLASS\n");
            return -EINVAL; 
        }
        if (ets_node->state == EGRESS_NODE_CONF)
            ets_node->state = EGRESS_NODE_CHANGED;

        if (ret == EEXIST && entry->state == EGRESS_NODE_CONF) {
            if (enet_gred_changed(entry, qopt->set))
                entry->state = EGRESS_NODE_CHANGED;
        } else if (ret && ret != EEXIST) {
            return ret;
        }

        entry->handle = handle;
        entry->parentid = parent_handle;
        entry->red.low_class.max =  qopt->set.tab[0].max;
        entry->red.low_class.min = qopt->set.tab[0].min;
        entry->red.low_class.drop_prob = EGRESS_TM_FLOAT2PERC(qopt->set.tab[0].probability);
        entry->red.high_class.max =  qopt->set.tab[1].max;
        entry->red.high_class.min = qopt->set.tab[1].min;
        entry->red.high_class.drop_prob = EGRESS_TM_FLOAT2PERC(qopt->set.tab[1].probability);
    } else {
        hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
            if (par_entry->handle == parent_handle) {
                return enet_delete_child_node(port, par_entry, handle);
            }
        }
        egress_tm_warn("Cannot find and delete handle %d\n", qopt->handle);
    }

    return 0;
}

int enet_setup_pie(struct net_device *dev, struct tc_pie_qopt_offload *qopt)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *par_entry, *entry = NULL, *ets_node;
    u64 parent_handle, handle;
    int ret;

    /* Only replace/remove supported */
    if (qopt->command != TC_PIE_REPLACE && qopt->command != TC_PIE_DESTROY)
        return 0;

    if (qopt->handle == TC_H_ROOT) {
        egress_tm_err("PIE cannot be root\n");
        return -EINVAL;
    }

    parent_handle = EGRESS_TM_HANDLE(qopt->parent, EGRESS_TM_CLASS_ID);
    handle = EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_QDISC_ID);

    if (qopt->command == TC_PIE_REPLACE) {
        /* Parent of PIE is always CLASS */
        hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
            if (par_entry->handle == parent_handle) {
                ret = enet_create_child_node(port, &entry, par_entry, handle, EGRESS_TM_PIE);
                goto match_found;
            }
        }
match_found:
        if (!entry) {
            egress_tm_err("Parent of node not found\n");
            return -EINVAL;       
        }

        ets_node = enet_drop_algo2ets(entry);
        if (!ets_node) {
            enet_delete_child_node(port, par_entry, handle);
            egress_tm_err("GRED parent or grandparent should be ETS CLASS\n");
            return -EINVAL; 
        }
        if (ets_node->state == EGRESS_NODE_CONF)
            ets_node->state = EGRESS_NODE_CHANGED;

        if (ret == EEXIST && entry->state == EGRESS_NODE_CONF) {
            return 0;
        } else if (ret && ret != EEXIST) {
            return ret;
        }

        entry->handle = handle;
        entry->parentid = parent_handle;
    } else {
        hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, parent_handle) {
            if (par_entry->handle == parent_handle) {
                return enet_delete_child_node(port, par_entry, handle);
            }
        }
        egress_tm_warn("Cannot find and delete handle %d\n", qopt->handle);
    }

    return 0;
}

#define L4S_LQ_WEIGHT   15  /* L4S protocol constants */
#define L4S_CQ_WEIGHT   1
#define L4S_NUM_QUEUES  2
#define L4S_CQ_OFFSET   (1000)

int enet_setup_dualpi2(struct net_device *dev, struct tc_dualp2_qopt_offload *qopt)
{
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    egress_tm_node *par_entry, *entry;
    struct tc_ets_qopt_offload qopt_est;
    u64 parent_handle, handle, child_handle;
    int i;

    /* Only replace/remove supported */
    if (qopt->command != TC_DUALPI2_REPLACE && qopt->command != TC_DUALPI2_DESTROY)
        return 0;

    if (qopt->handle == TC_H_ROOT) {
        egress_tm_err("DUALPI2 cannot be root\n");
        return -EINVAL;
    }

    qopt_est.handle = 0;
    qopt_est.parent = qopt->parent;
    parent_handle = EGRESS_TM_HANDLE(qopt->parent, EGRESS_TM_CLASS_ID);
    child_handle = EGRESS_TM_SUBID(EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_CLASS_ID), 1);

    if (qopt->command == TC_DUALPI2_REPLACE) {
        qopt_est.replace_params.bands = L4S_NUM_QUEUES;
        qopt_est.replace_params.quanta[0] = L4S_CQ_WEIGHT;
        qopt_est.replace_params.quanta[1] = L4S_LQ_WEIGHT;
        qopt_est.replace_params.weights[0] = L4S_CQ_WEIGHT;
        qopt_est.replace_params.weights[1] = L4S_LQ_WEIGHT;

        handle = EGRESS_TM_SUBID(EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_QDISC_ID), 1);
        enet_ets_replace(dev, &qopt_est, parent_handle, handle, child_handle);

        for (i = 0; i < L4S_NUM_QUEUES; i++) {
            entry = NULL;
            hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, child_handle + i) {
                if (par_entry->handle == (child_handle + i)) {
                    egress_tm_node *queue_parent;

                    handle = EGRESS_TM_SUBID(EGRESS_TM_HANDLE(qopt->handle + i, EGRESS_TM_QDISC_ID), 2);
                    /* set correct queue ID and drop algo */
                    if (i == 0) {
                        enet_create_child_node(port, &entry, par_entry, handle, EGRESS_TM_PIE);
                        queue_parent =  enet_get_up_level_ets(entry->parent->parent);
                        par_entry->queue.qid = queue_parent->queue.qid;
                    } else {
                        enet_create_child_node(port, &entry, par_entry, handle, EGRESS_TM_DUALQ_LAQM);
                        queue_parent =  enet_get_up_level_ets(entry->parent->parent);
                        par_entry->queue.qid = queue_parent->queue.qid + L4S_CQ_OFFSET;
                    }
                    if (par_entry->parent->type == EGRESS_TM_ETS_PARENT)
                        par_entry->parent->ets.type = EGRESS_TM_TYPE_DUALQ_PI2;
                    else
                        egress_tm_warn("Wrong DUALPI2 queue parent\n");
                    goto match_found;
                }
            }
match_found:
            if (!entry) {
                egress_tm_err("Parent of node not found\n");
                return -EINVAL;       
            }

            entry->handle = handle;
            entry->parentid = parent_handle;
        }
    } else {
        for (i = 0; i < L4S_NUM_QUEUES; i++) {
            hash_for_each_possible(port->nodes_hash_table, par_entry, hnode, child_handle + i) {
                if (par_entry->handle == (child_handle + i)) {
                    handle = EGRESS_TM_SUBID(EGRESS_TM_HANDLE(qopt->handle + i, EGRESS_TM_QDISC_ID), 2);
                    enet_delete_child_node(port, par_entry, handle);
                }
            }
        }
        handle = EGRESS_TM_SUBID(EGRESS_TM_HANDLE(qopt->handle, EGRESS_TM_QDISC_ID), 1);
        enet_ets_destroy(dev, &qopt_est, parent_handle, handle, child_handle);
    }

    return 0;
}
