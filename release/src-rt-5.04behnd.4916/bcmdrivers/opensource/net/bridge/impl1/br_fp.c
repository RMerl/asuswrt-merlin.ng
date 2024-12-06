/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
*
*    Copyright (c) 2012 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
* :>
*/

#include <linux/version.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/if_vlan.h>
#include <linux/if_bridge.h>
#include <asm/cpu.h>
#include <asm/uaccess.h>
#include <asm/string.h>
#include "bcm_OS_Deps.h"
#include <linux/br_fp.h>
#include "br_private.h"
#include "rdpa_api.h"
#include "rdpa_mw_blog_parse.h"
#include "bcmenet_common.h"
#include <bdmf_dev.h>
#include "br_sdev.h"
#include "rdpa_drv.h"

static bdmf_fastlock bridge_lock;
static bdmf_object_handle system_obj = NULL;

static int br_fp_rdpa_hook(int cmd, void *param1, void *param2);

static int is_wlan_accl_enabled(void)
{
    bdmf_object_handle cpu_obj;

    if (!rdpa_cpu_get(rdpa_cpu_wlan0, &cpu_obj))
    {
        bdmf_put(cpu_obj);
        return 1;
    }
    return 0;
}

static rdpa_bridge_type rdpa_bridge_type_get(bdmf_object_handle bridge_object)
{
    rdpa_bridge_cfg_t br_cfg;

    rdpa_bridge_config_get(bridge_object, &br_cfg);

    return br_cfg.type;
}

static void br_fp_rdpa_fdb_key_set(uint8_t *mac_addr, uint16_t vid, rdpa_fdb_key_t *key, bdmf_object_handle br_obj)
{
    memset(key, 0, sizeof(*key));
    memcpy(&key->mac, mac_addr, sizeof(key->mac));
    if (rdpa_bridge_type_get(br_obj) == rdpa_bridge_802_1q)
        key->vid = vid;
}

bdmf_error_t rdpa_add_or_modify_mac_tbl_entry(struct net_bridge_port *br_port, uint8_t *mac_addr, uint16_t vid, int is_static, int is_add)
{
    bdmf_object_handle bridge;
    rdpa_fdb_key_t key;
    rdpa_fdb_data_t data;
    bdmf_object_handle port_object;
    rdpa_port_type port_type;
    struct net_device *root_dev;
    bdmf_error_t rc;

    root_dev = netdev_path_get_root(br_port->dev);
    port_object = rdpa_mw_get_port_object_by_dev(root_dev);

    if (!port_object)
    {
       	BDMF_TRACE_RET(0, "%s MAC entry: no matching rdpa port. src dev name[%s] mac[%pM]",
            is_add ? "ADD" : "MOD", root_dev->name, mac_addr);
    }

    bridge = (bdmf_object_handle)bridge_fp_data_obj_get(br_port->br->dev);
    if (!bridge)
        return BDMF_ERR_NOENT;

    br_fp_rdpa_fdb_key_set(mac_addr, vid, &key, bridge);

    bdmf_fastlock_lock(&bridge_lock);

    rc = rdpa_bridge_mac_get(bridge, &key, &data);
    if (is_add && !rc)
    {
        /*when using ingress classifier we have a usecase which there is an extra entry in RDPA which will not be deleted
         *thus we have to add it anyway with the real bridge port*/
        rdpa_port_type_get(data.port_obj, &port_type);

        if (port_type != rdpa_port_cpu)
        {
            rc = BDMF_ERR_ALREADY; /* The MAC entry already exists for this bridge. */
            goto exit;
        }
    }

    data.port_obj = port_object;
    data.sa_action = rdpa_forward_action_forward;

    if ((port_type == rdpa_port_wlan) && !is_wlan_accl_enabled())
        data.da_action = rdpa_forward_action_host;
    else
        data.da_action = rdpa_forward_action_forward;

    rc = rdpa_bridge_mac_set(bridge, &key, &data);
    if (rc && (rc != BDMF_ERR_NOT_LINKED))
    {
        BDMF_TRACE_INFO("Failed to add mac entry, src dev name[%s] mac[%pM] (rc=%d)\n", br_port->dev->name,
            mac_addr, rc);
    }

    if ((rc == BDMF_ERR_INTERNAL) && !is_static)
        bridge_mac_entry_discard_counter_inc(br_port->br->dev);

exit:
    bdmf_fastlock_unlock(&bridge_lock);
    return rc;    
}

/* can be called under RCU lock or RTNL lock from switchdev notifier
 * In case of FDB modification it called under bridge hashlock lock */
static bdmf_error_t br_fp_add_or_modify_mac_tbl_entry(struct net_bridge_fdb_entry *fdb, int is_add)
{
    bdmf_error_t rc = -1;
    struct net_bridge_port *port;

    rcu_read_lock();
    port = br_port_get_rcu(fdb->dst->dev);
    if (port)
        rc = rdpa_add_or_modify_mac_tbl_entry(port, (uint8_t *)GET_FDB_ADDR(fdb).addr, GET_FDB_VLAN(fdb), fdb->is_static, is_add);
    rcu_read_unlock();
    return rc;
}

bdmf_error_t rdpa_remove_mac_tbl_entry(struct net_bridge_port *br_port, uint8_t *mac_addr, uint16_t vid, int remove_dups)
{    
    bdmf_object_handle bridge;
    rdpa_fdb_key_t key = {};
    bdmf_object_handle port_object;
    struct net_device *root_dev;
    bdmf_error_t rc;

    root_dev = netdev_path_get_root(br_port->dev);

    port_object = rdpa_mw_get_port_object_by_dev(root_dev);

    if (!port_object)
        BDMF_TRACE_RET(0, "REM MAC entry: no matching rdpa port. src dev name[%s] mac[%pM]", root_dev->name, mac_addr);

    bridge = (bdmf_object_handle)bridge_fp_data_obj_get(br_port->br->dev);
    if (!bridge)
        return BDMF_ERR_NOENT;

    br_fp_rdpa_fdb_key_set(mac_addr, vid, &key, bridge);
    rc = rdpa_bridge_mac_set(bridge, &key, NULL);
    /* RDPA return code BDMF_ERR_NOENT means the MAC target to be deleted here is NOT in Runner bridge.
       Hence not an error in br_fp level. Here are some cases:
       Case 1: Linux Birdge has larger FDB size than Runner bridge;
       Case 2: The MAC has never been learned in this Runner bridge, eg. HGU bridge WAN case.*/
    if ((rc) && (rc != BDMF_ERR_NOENT))
        BDMF_TRACE_RET(rc, "Failed to remove mac entry, src dev name[%s] mac[%pM]\n", br_port->dev->name, mac_addr);

    return 0;
}

static int br_fp_remove_mac_tbl_entry(struct net_bridge_fdb_entry *fdb, int remove_dups)
{
    struct net_bridge_port *port;
    int rc = -1;

    rcu_read_lock();
    port = br_port_get_rcu(fdb->dst->dev);
    if (port)
        rc = rdpa_remove_mac_tbl_entry(port, GET_FDB_ADDR(fdb).addr, GET_FDB_VLAN(fdb), remove_dups);
    rcu_read_unlock();

    return rc;
}

static int br_fp_ageing_mac_tbl_entry(struct net_bridge_fdb_entry *fdb, int *age_valid)
{
    bdmf_error_t rc;
    bdmf_boolean _age_valid = 0;
    bdmf_object_handle bridge;
    rdpa_fdb_key_t key = {};

#if defined(CONFIG_NET_SWITCHDEV)
    if (!runner_switchdev_learning_enabled(fdb->dst->dev))
        return 0;
#endif

    bridge = (bdmf_object_handle)bridge_fp_data_obj_get(fdb->dst->br->dev);
    if (!bridge)
        return BDMF_ERR_NOENT;

    br_fp_rdpa_fdb_key_set(GET_FDB_ADDR(fdb).addr, GET_FDB_VLAN(fdb), &key, bridge);

    rc = rdpa_bridge_mac_status_get(bridge, &key, &_age_valid);
    if (rc)
    {
        if (rc != BDMF_ERR_NOENT)
    	    BDMF_TRACE_ERR("Aging check failed(%d). src dev name[%s] mac[%pM]\n", rc, fdb->dst->dev->name, GET_FDB_ADDR(fdb).addr);
        _age_valid = 0; /* return status, which will cause deletion of fdb entry in kernel bridge */
    }

    *age_valid = _age_valid;

    return 0;
}

static int local_switching_set(struct net_device *dev)
{
    bdmf_object_handle bridge_object = (bdmf_object_handle)bridge_fp_data_obj_get(dev);
    return rdpa_bridge_local_switch_enable_set(bridge_object, !bridge_local_switching_disable_get(dev));
}

static int bridge_create(struct net_bridge *br, rdpa_bridge_type type, bdmf_boolean auto_aggregate)
{
    bdmf_object_handle br_obj = NULL;
    int ret;
    BDMF_MATTR_ALLOC(bridge_attr, rdpa_bridge_drv());
    rdpa_bridge_cfg_t rdpa_bridge_cfg = {
        .type = type,
        .learning_mode = rdpa_bridge_learn_svl,
        .auto_forward = 1,
        .auto_aggregate = auto_aggregate,
    };

    rdpa_bridge_config_set(bridge_attr, &rdpa_bridge_cfg);
    ret = bdmf_new_and_set(rdpa_bridge_drv(), NULL, bridge_attr, &br_obj);
    BDMF_MATTR_FREE(bridge_attr);
    if (ret)
        return -1;

    bridge_fp_data_obj_set(br->dev, br_obj);
    bridge_fp_data_hook_set(br->dev, br_fp_rdpa_hook);

    bridge_mac_entry_discard_counter_set(br->dev, 0);

    local_switching_set(br->dev);

    return 0;
}

static int br_fp_rdpa_bridge_add(struct net_bridge *br)
{
    static int is_first = 1;
    rdpa_bridge_type type = rdpa_bridge_802_1d;
    bdmf_boolean auto_aggregate = 0;
    int rc = 0;

    /* 1st bridge is always of type 802.1d */
    if (is_first)
    {
        is_first = 0;
    }
#if defined(CONFIG_ONU_TYPE_HGU)
    else
    {
        type = rdpa_bridge_802_1q;
    }
#endif

    rc = bridge_create(br, type, auto_aggregate);
    if (rc < 0)
        BDMF_TRACE_ERR("Failed to add RDPA bridge object for %s, error %d\n", br->dev->name, rc);

#if defined(CONFIG_BCM_KF_NETDEV_EXT)
    /* For locally terminated traffic, "link" rdpa_port_cpu object to bridge devices to open connections */
    bcm_netdev_ext_field_set(br->dev, bcm_netdev_cb_fn, bcm_netdev_def_cpu_port_obj_get);
#endif

    return rc;
}

static int br_fp_rdpa_bridge_del(struct net_bridge *br)
{
    if (!bridge_fp_data_obj_get(br->dev))
        return 0;

    bdmf_destroy((bdmf_object_handle)bridge_fp_data_obj_get(br->dev));
    bridge_fp_data_obj_set(br->dev, NULL);
    bridge_fp_data_hook_set(br->dev, NULL);
    return 0;
}

static int is_vlan_vid_enabled(bdmf_object_handle vlan_object)
{
    int i;
    bdmf_boolean vid_enabled = 0;

    for (i = 0; i < 4096 && !vid_enabled ; i++)
    {
        if (rdpa_vlan_vid_enable_get(vlan_object, i, &vid_enabled))
            vid_enabled = 0;
    }

    return vid_enabled;
}

static bdmf_object_handle get_link_object(bdmf_object_handle bridge_object, struct net_device *dev)
{
    bdmf_object_handle link_object = NULL, port_object = NULL, vlan_object = NULL;
    const bool is_q = rdpa_bridge_type_get(bridge_object) == rdpa_bridge_802_1q;

    if (is_q && is_netdev_vlan(dev) && !rdpa_vlan_get(dev->name, &vlan_object) && is_vlan_vid_enabled(vlan_object))
    {
        link_object = vlan_object;
    }
    else if (is_q && is_netdev_vfrwd(dev)) {
        BDMF_MATTR_ALLOC(vlan_attr, rdpa_vlan_drv());
        bdmf_object_handle po;

        if (!rdpa_vlan_get(dev->name, &link_object))
            goto out;

        po = rdpa_mw_get_port_object_by_dev(netdev_path_get_root(dev));
        if (!po)
            goto out;

        rdpa_vlan_name_set(vlan_attr, dev->name);
        if (bdmf_new_and_set(rdpa_vlan_drv(), po, vlan_attr, &link_object))
            BDMF_TRACE_ERR("Can't create link object for %s\n", dev->name);
out:
        BDMF_MATTR_FREE(vlan_attr);
    }
    else /* XXX: Should fail here if it did not find a vlan object, however some usecases still try to
            link a non-vlan interface to a 802.1q bridge (in HGU mode+interface grouping)
            if (rdpa_bridge_type_get(bridge_object) == rdpa_bridge_802_1d) */
    {
        link_object = port_object = rdpa_mw_get_port_object_by_dev(netdev_path_get_root(dev));
    }

    if (vlan_object)
        bdmf_put(vlan_object);

    return link_object;
}

static int get_link_usage(bdmf_object_handle bridge_object, const char *object_name, struct net_bridge *br)
{
    struct net_bridge_port *br_port;
    int count = 0;

    list_for_each_entry(br_port, &br->port_list, list)
    {
        bdmf_object_handle link_object;

        link_object = get_link_object(bridge_object, br_port->dev);

        if (!link_object || link_object->state != bdmf_state_active)
            continue;

        if (!strcmp(bdmf_object_name(link_object), object_name))
            count++;
    }

    return count;
}

/* May return 0 even with link_object NULL, if no RDPA device mapping found */
static int get_rdpa_objects(struct net_bridge *br, struct net_device *dev,
    bdmf_object_handle *bridge_object, bdmf_object_handle *link_object)
{
    *bridge_object = (bdmf_object_handle)bridge_fp_data_obj_get(br->dev);
    if (!*bridge_object)
    {
        BDMF_TRACE_ERR("Can't find RDPA bridge object for %s\n", br->dev->name);
        return -1;
    }

    if ((*link_object = get_link_object(*bridge_object, dev)))
        return 0;

    /* Silently ignore non-RPDA */
    BDMF_TRACE_DBG("Can't find RDPA link object for %s\n", br->dev->name);
    return 0;
}

static int br_fp_del_port(struct net_bridge *br, struct net_device *dev)
{
    bdmf_object_handle bridge_object = NULL, link_object = NULL;
    struct bdmf_link *plink;
    int ret = -1;

    rcu_read_lock();

    if ((ret = get_rdpa_objects(br, dev, &bridge_object, &link_object)) || !link_object)
        goto Exit;

    if (get_link_usage(bridge_object, bdmf_object_name(link_object), br) > 1)
        goto Exit;

    if (!bdmf_is_linked(link_object, bridge_object, &plink))
        goto Exit;

    if ((ret = bdmf_unlink(link_object, bridge_object)))
    {
        BDMF_TRACE_ERR("Failed to unlink RDPA objects %s - %s, error %s (%d)\n",
            bdmf_object_name(link_object), bdmf_object_name(bridge_object),
            bdmf_strerror(ret), ret);
    }

Exit:
    rcu_read_unlock();

    return ret;
}

/* XRDP supports rdpa port per radio; should skip linking operation if radio is linked to another bridge */
static int wlan_radio_link_object_linked_to_any_bridge(bdmf_object_handle link_object)
{
#if defined(CONFIG_BCM_PON_XRDP)
    bdmf_link_handle link;
    rdpa_port_type port_type;

    if (link_object->drv != rdpa_port_drv())
        return 0;

    rdpa_port_type_get(link_object, &port_type);

    if (port_type == rdpa_port_wlan)
        return 0;

    while ((link = bdmf_get_next_us_link(link_object, NULL)))
    {
        bdmf_object_handle obj = bdmf_us_link_to_object(link);

        if (obj->drv == rdpa_bridge_drv())
            return 1;
    }
#endif
    return 0;
}

static int br_fp_add_port(struct net_bridge *br, struct net_device *dev)
{
    bdmf_object_handle bridge_object, link_object = NULL;
    struct bdmf_link *plink;
    int ret = -1;

    rcu_read_lock();

    if ((ret = get_rdpa_objects(br, dev, &bridge_object, &link_object)) || !link_object)
        goto Exit;

    if (bdmf_is_linked(link_object, bridge_object, &plink))
        goto Exit;

    if (wlan_radio_link_object_linked_to_any_bridge(link_object))
    {
        ret = 0;
        goto Exit;
    }

    if ((ret = bdmf_link(link_object, bridge_object, NULL)))
    {
        BDMF_TRACE_ERR("Failed to link RDPA objects %s - %s, error %s (%d)\n",
            bdmf_object_name(link_object), bdmf_object_name(bridge_object),
            bdmf_strerror(ret), ret);
    }

Exit:
    rcu_read_unlock();

    return ret;
}

static int is_rdpa_bridge_supported_device(struct net_device *root_dev)
{
#if defined(CONFIG_ONU_TYPE_SFU) 
    int ret;
    rdpa_system_init_cfg_t init_cfg;

    rdpa_system_init_cfg_get(system_obj, &init_cfg);

    if (init_cfg.operation_mode == rdpa_method_fc)
    {
        /* Bridging accelerated by ip/l2_class flows in when fc is enabled in SFU */
        /* XXX: For NLDP, should we always return 0 here??? */
        if (is_netdev_wan(root_dev))
            ret = 0;
        else
            ret = 1;
    }
    else
    {
        ret = 1;
    }

    return ret;
#else
    /* Bridging accelerated by ip/l2_class flows in HGU */
    if (is_netdev_wan(root_dev))
        return 0;
    else
        return 1;
#endif
}

static int br_fp_bridge_type_get(struct net_device *dev)
{
    bdmf_object_handle br_obj = (bdmf_object_handle)bridge_fp_data_obj_get(dev);
    rdpa_bridge_cfg_t cfg;

    rdpa_bridge_config_get(br_obj, &cfg);

    return cfg.type;
}

static int br_fp_bridge_type_set(struct net_device *dev, rdpa_bridge_type type)
{
    struct net_bridge *br = (struct net_bridge *)netdev_priv(dev);
    bdmf_object_handle br_obj = (bdmf_object_handle)bridge_fp_data_obj_get(dev);
    bdmf_index index = BDMF_INDEX_UNASSIGNED;
    rdpa_fdb_key_t key;
    rdpa_bridge_cfg_t rdpa_bridge_cfg_old;
    int rc;
    struct net_bridge_port *p;

    memcpy(&key, &index, sizeof(index));

    spin_lock_bh(&br->lock);
    rc = rdpa_bridge_mac_get_next(br_obj, &key);
    if (rc != BDMF_ERR_NO_MORE || br->dev->flags & IFF_UP)
    {
        printk("br_fp: Interface must be down and with no MAC learnt in order to change type (rc=%d)\n", rc);
        rc = -1;
        goto out;
    }

    list_for_each_entry(p, &br->port_list, list)
    {
        printk("br_fp: Interface must not have enslaved ports (rc=%d)\n", rc);
        rc = -1;
        goto out;
    }

    rdpa_bridge_config_get(br_obj, &rdpa_bridge_cfg_old);
    if ((rc = bdmf_destroy(br_obj)))
        goto out;

    if ((rc = bridge_create(br, type, rdpa_bridge_cfg_old.auto_aggregate)))
        goto out;

out:
    spin_unlock_bh(&br->lock);

    return rc;
}

static int br_fp_rdpa_hook(int cmd, void *param1, void *param2)
{
    struct net_device *dev = param1;
    rdpa_bridge_type type;
    int rc = -1;

    switch (cmd)
    {
    case BR_FP_LOCAL_SWITCHING_DISABLE:
        rc = local_switching_set(dev);
        break;
    case BR_FP_BRIDGE_TYPE_SET:
        type = (rdpa_bridge_type)*(unsigned long *)param2;
        rc = br_fp_bridge_type_set(dev, type);
        break;
    case BR_FP_BRIDGE_TYPE_GET:
        type = br_fp_bridge_type_get(dev) ;
        *(unsigned long *)param2 = type;
        rc = 0;
        break;
    case BR_FP_FDB_CHECK_AGE:
        {
            struct net_bridge_fdb_entry *fdb = param1;
            int *age_valid = param2;
            rc = br_fp_ageing_mac_tbl_entry(fdb, age_valid);
        }
    }
    return rc;
}

static inline void handle_fdb_entry(struct net_bridge_fdb_entry *f, int add)
{
    if (f->is_local)
        return;

    if (add)
        br_fp_add_or_modify_mac_tbl_entry(f, 1);
    else
    {
        /* always remove, even if exists in more then 1 bridge */
        br_fp_remove_mac_tbl_entry(f, 1);
    }
}

static void do_fdb_entries(struct net_device *brdev, int add)
{
    struct hlist_node *h;
    struct net_bridge_fdb_entry *f;
    struct net_bridge *br;

    br = netdev_priv(brdev);

    rcu_read_lock();
    HLIST_FOR_EACH_ENTRY_RCU(f, h, &br->fdb_list, fdb_node)
        handle_fdb_entry(f, add);
    rcu_read_unlock();
}

#define BR_MACS_NUM 9
typedef struct br_mac {
    struct net_device *br_dev;
    bdmf_index mac_idx;
} br_mac_t;
static br_mac_t br_macs[BR_MACS_NUM];

/* this function is registered to device events. we are interested in mac change of the bridge*/
static int br_fp_notifier_call(struct notifier_block *nb, unsigned long event, void *ptr)
{
    struct net_device *dev = NETDEV_NOTIFIER_GET_DEV(ptr);
    struct net_bridge *br;

    switch (event)
    {
    /* NETDEV_CHANGEADDR is not enough - a device may be granted a MAC address
     * on its way up without moving through NETDEV_CHANGEADDR. */
    case NETDEV_REGISTER:
    case NETDEV_UNREGISTER:
        {
            if (!(dev->priv_flags & IFF_EBRIDGE))
                break;

            br = (struct net_bridge *)netdev_priv(dev);
            if (event == NETDEV_REGISTER)
                return br_fp_rdpa_bridge_add(br);
            else
                return br_fp_rdpa_bridge_del(br);
        }
        break;
    case NETDEV_UP:
    case NETDEV_DOWN:
    case NETDEV_CHANGEADDR:
        if (dev->priv_flags & IFF_EBRIDGE)
        {
            int rc, i, empty_i;

            if (!is_rdpa_bridge_supported_device(dev))
                break;

            if (event == NETDEV_UP)
            {
                if (bdmf_mac_is_zero((bdmf_mac_t *)dev->dev_addr))
                {
                    /* Skip adding zero mac address */
                    return 0;
                }
                for (i = 0, empty_i = BR_MACS_NUM; i < BR_MACS_NUM; i++)
                {
                    if (br_macs[i].br_dev == dev)
                    {
                        /* un-expected double-call for NETDEV_UP, ignore */
                        return 0;
                    }
                    if (!br_macs[i].br_dev && empty_i == BR_MACS_NUM)
                        empty_i = i;
                }
                if (empty_i == BR_MACS_NUM)
                {
                    printk("No empty slot to add a MAC address, dev %s, mac %pM\n", dev->name, dev->dev_addr);
                    return 0;
                }
                rc = rdpa_system_host_mac_address_table_add(system_obj, &br_macs[empty_i].mac_idx,
                    (bdmf_mac_t *)dev->dev_addr);
                if (rc < 0)
                {
                    printk("unable to add LAN MAC address %pM, dev %s, rc %d\n", dev->dev_addr, dev->name, rc);
                    return 0;
                }
                br_macs[empty_i].br_dev = dev;
            }
            else /* NETDEV_DOWN || NETDEV_CHANGEADDR */
            {
                for (i = 0, empty_i = BR_MACS_NUM; i < BR_MACS_NUM; i++)
                {
                    if (br_macs[i].br_dev == dev)
                        break;
                    if (!br_macs[i].br_dev && empty_i == BR_MACS_NUM)
                        empty_i = i;
                }
                if (i == BR_MACS_NUM)
                {
                    /* Mac doesn't exist; thus can happen if same mac was already added for another bridge device and
                     * now it's changed. Use empty if found */
                    i = empty_i;
                }
                else
                {
                    rc = rdpa_system_host_mac_address_table_delete(system_obj, br_macs[i].mac_idx);
                    if (rc < 0)
                    {
                        printk("unable to delete LAN MAC address %pM, dev %s, rc %d\n", br_macs[i].br_dev->dev_addr,
                            br_macs[i].br_dev->name, rc);
                        return 0;
                    }
                }
                if (event == NETDEV_CHANGEADDR && !bdmf_mac_is_zero((bdmf_mac_t *)dev->dev_addr)
                    && i != BR_MACS_NUM)
                {
                    rc = rdpa_system_host_mac_address_table_add(system_obj, &br_macs[i].mac_idx,
                        (bdmf_mac_t *)dev->dev_addr);
                    if (rc < 0)
                    {
                        printk("unable to add LAN MAC address %pM, dev %s, rc %d\n", dev->dev_addr, dev->name, rc);
                        br_macs[i].br_dev = NULL;
                        return 0;
                    }
                    br_macs[i].br_dev = dev;
                }
                else
                {
                    /* Deleted, free the slot */
                    br_macs[i].br_dev = NULL;
                }
            }
        }
        break;
    case NETDEV_CHANGEUPPER:
        {
            struct netdev_notifier_changeupper_info *info = ptr;

            if (!is_rdpa_bridge_supported_device(dev) || !netif_is_bridge_master(info->upper_dev))
                break;

            br = (struct net_bridge *)netdev_priv(info->upper_dev);

            if (info->linking)
                br_fp_add_port(br, dev);
            else
                br_fp_del_port(br, dev);
        }
        break;
    default:
        break;
    }

    return 0;
}

static struct notifier_block nb =
{
    .notifier_call = br_fp_notifier_call,
    .priority = 0,
};

static int br_fp_init(void)
{
    struct net_device *dev;
    struct net_bridge *br;
    struct net_bridge_port *p;
    int rc = 0;

    printk("Bridge fastpath module\n");

    if (rdpa_system_get(&system_obj))
    {
        printk("Failed to get RDPA System object\n");
        return -1;
    }

    for_each_netdev(&init_net, dev)
    {
        if (dev->priv_flags & IFF_EBRIDGE)
        {
            br = netdev_priv(dev);
            rc = br_fp_rdpa_bridge_add(br);
            if (rc)
                return -1;

            list_for_each_entry(p, &br->port_list, list) {
                br_fp_add_port(br, p->dev);
            }

            do_fdb_entries(dev, 1);
        }
    }

    if (register_netdevice_notifier(&nb))
    {
        printk(KERN_ERR "register_netdevice_notifier() failed(%d)", 0);
        return -1;
    }
#if defined(CONFIG_NET_SWITCHDEV)
    runner_switchdev_init();
#endif

    memset(br_macs, 0, sizeof(br_macs));

    return 0;
}

static void br_fp_cleanup(void)
{
    struct net_device *dev;

#if defined(CONFIG_NET_SWITCHDEV)
    runner_switchdev_cleanup();
#endif
 
    /* Below unregister_netdevice_notifier() removes interfaces, however because bridge might be
       called first, proper MAC cleanup is required because rdpa bridge object does not clear MACs */
    for_each_netdev(&init_net, dev)
    {
        if (dev->priv_flags & IFF_EBRIDGE)
            do_fdb_entries(dev, 0);
    }

    unregister_netdevice_notifier(&nb);
    bdmf_put(system_obj);
}

MODULE_LICENSE("GPL");
module_init(br_fp_init);
module_exit(br_fp_cleanup);
