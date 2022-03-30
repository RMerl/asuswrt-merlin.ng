/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; 
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef BR_FP_H
#define BR_FP_H

#include <linux/device.h>
#include <linux/module.h>

#define BR_FP_FDB_ADD 1
#define BR_FP_FDB_REMOVE 2
#define BR_FP_FDB_MODIFY 3
#define BR_FP_FDB_CHECK_AGE 4
#define BR_FP_PORT_ADD 5
#define BR_FP_PORT_REMOVE 6
#define BR_FP_LOCAL_SWITCHING_DISABLE 7
#define BR_FP_BRIDGE_TYPE 8

#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
struct br_fp_data
{
    int (*rdpa_hook)(int cmd, void *in, void *out);
    void *rdpa_obj;
};
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */

struct bcm_br_ext
{
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
    struct  br_fp_data br_fp_data;
#define bridge_fp_data_hook_get(_br_dev)        (bcm_netdev_ext_field_get((_br_dev), bcm_br_ext.br_fp_data.rdpa_hook))
#define bridge_fp_data_hook_set(_br_dev, _hook) bcm_netdev_ext_field_set((_br_dev), bcm_br_ext.br_fp_data.rdpa_hook, (_hook))
#define bridge_fp_data_obj_get(_br_dev)         (bcm_netdev_ext_field_get((_br_dev), bcm_br_ext.br_fp_data.rdpa_obj))
#define bridge_fp_data_obj_set(_br_dev, _obj)   bcm_netdev_ext_field_set((_br_dev), bcm_br_ext.br_fp_data.rdpa_obj, (_obj))

#define br_fp_hook(_br_dev, _cmd, _arg1, _arg2) (bridge_fp_data_hook_get((_br_dev)) ? bridge_fp_data_hook_get((_br_dev))((_cmd), (_arg1), (_arg2)) : 0)
#endif

#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
    int     local_switching_disable;
#define bridge_local_switching_disable_get(_br_dev)       (bcm_netdev_ext_field_get((_br_dev), bcm_br_ext.local_switching_disable))
#define bridge_local_switching_disable_set(_br_dev, _val) bcm_netdev_ext_field_set((_br_dev), bcm_br_ext.local_switching_disable, (_val))
#else
#define bridge_local_switching_disable_get(_br_dev)       ((void)(_br_dev), 0)
#define bridge_local_switching_disable_set(_br_dev, _val) ((void)(_br_dev), (void)(_val), 0)
#endif

#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
    u32     mac_entry_discard_counter;
#define bridge_mac_entry_discard_counter_get(_br_dev) (bcm_netdev_ext_field_get((_br_dev), bcm_br_ext.mac_entry_discard_counter))
#define bridge_mac_entry_discard_counter_set(_br_dev, _val) bcm_netdev_ext_field_set((_br_dev), bcm_br_ext.mac_entry_discard_counter, (_val))
#define bridge_mac_entry_discard_counter_inc(_br_dev) ((bcm_netdev_ext_field_get((_br_dev), bcm_br_ext.mac_entry_discard_counter)++))
#else
#define bridge_mac_entry_discard_counter_get(_br_dev) ((void)(br_dev), 0)
#define bridge_mac_entry_discard_counter_set(_br_dev, _val) do {} while(0)
#define bridge_mac_entry_discard_counter_inc(_br_dev) do {} while(0)
#endif
};

#endif /* BR_FP_H */
