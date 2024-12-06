/*
 *  Copyright: (c) 2020 Broadcom.
 *  All rights reserved.
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
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

#ifndef XFLOW_MACSEC_ESW_DEFS_H
#define XFLOW_MACSEC_ESW_DEFS_H

/*******************************************************************************
 * Function declarations
 */

/* Locks definitions. */
#define XFLOW_MACSEC_LOCK(unit) \
    sal_mutex_take(SOC_CONTROL(unit)->xflow_macsec_lock, sal_mutex_FOREVER)

#define XFLOW_MACSEC_UNLOCK(unit) \
    sal_mutex_give(SOC_CONTROL(unit)->xflow_macsec_lock)

extern int _soc_xflow_macsec_init(int unit);
extern int _bcm_xflow_macsec_init(int unit);
extern int _bcm_xflow_macsec_deinit(int unit);

/* Secure Channel */
extern int
xflow_macsec_secure_chan_create(int unit, uint32 flags,
            xflow_macsec_secure_chan_info_t *cfg,
            int priority,
            xflow_macsec_secure_chan_id_t *chan_id);
extern int
xflow_macsec_secure_chan_set(int unit, uint32 flags,
            xflow_macsec_secure_chan_id_t chan_id,
            xflow_macsec_secure_chan_info_t *chan_info,
            int priority);
extern int
xflow_macsec_secure_chan_get(int unit,
            xflow_macsec_secure_chan_id_t chan_id,
            xflow_macsec_secure_chan_info_t *chan_info,
            int *priority);
extern int
xflow_macsec_secure_chan_destroy(int unit,
            xflow_macsec_secure_chan_id_t chan_id);
extern int
xflow_macsec_secure_chan_enable_set(int unit,
            xflow_macsec_secure_chan_id_t chan_id,
            int enable);
extern int
xflow_macsec_secure_chan_enable_get(int unit,
            xflow_macsec_secure_chan_id_t chan_id,
            int *enable);

/* Secure Assoc */
extern int
xflow_macsec_secure_assoc_create(int unit, uint32 flags,
            xflow_macsec_secure_chan_id_t chan_id,
            xflow_macsec_secure_assoc_info_t *assoc_info,
            xflow_macsec_secure_assoc_id_t *assoc_id);
extern int
xflow_macsec_secure_assoc_set(int unit,
            xflow_macsec_secure_assoc_id_t assoc_id,
            xflow_macsec_secure_assoc_info_t *assoc_info);
extern int
xflow_macsec_secure_assoc_get(int unit,
            xflow_macsec_secure_assoc_id_t assoc_id,
            xflow_macsec_secure_assoc_info_t *assoc_info,
            xflow_macsec_secure_chan_id_t *chan_id);
extern int
xflow_macsec_secure_assoc_destroy(int unit,
            xflow_macsec_secure_assoc_id_t assoc_id);

/* Decrypt Policy */
extern int
xflow_macsec_policy_create (int unit,
            uint32 flags,
            xflow_macsec_policy_info_t *policy_info,
            xflow_macsec_policy_id_t *policy_id);
extern int
xflow_macsec_policy_set (int unit,
            xflow_macsec_policy_id_t policy_id,
            xflow_macsec_policy_info_t *policy_info);
extern int
xflow_macsec_policy_get (int unit,
            xflow_macsec_policy_id_t policy_id,
            xflow_macsec_policy_info_t *policy_info);
extern int
xflow_macsec_policy_destroy (int unit,
            xflow_macsec_policy_id_t policy_id);


/* Decrypt Flows */
extern int
xflow_macsec_flow_create (int unit, uint32 flags,
            xflow_macsec_flow_info_t *flow_info,
            int priority,
            xflow_macsec_flow_id_t *flow_id);
extern int
xflow_macsec_flow_set (int unit, xflow_macsec_flow_id_t flow_id,
            xflow_macsec_flow_info_t *flow_info,
            int priority);
extern int
xflow_macsec_flow_get (int unit, xflow_macsec_flow_id_t flow_id,
            xflow_macsec_flow_info_t *flow_info,
            int *priority);
extern int
xflow_macsec_flow_destroy (int unit, xflow_macsec_flow_id_t flow_id);
extern int
xflow_macsec_flow_enable_set (int unit, xflow_macsec_flow_id_t flow_id, int enable);
extern int
xflow_macsec_flow_enable_get (int unit, xflow_macsec_flow_id_t flow_id, int *enable);


/* Subport Number */
extern int
xflow_macsec_subport_id_get (int unit, xflow_macsec_id_t id,
            xflow_macsec_subport_id_t *macsec_subport_id);

/* Xflow macsec control */
extern int
xflow_macsec_control_set(int unit, uint32 flags,
            xflow_macsec_control_t type, uint64 value);
extern int
xflow_macsec_control_get(int unit, uint32 flags,
            xflow_macsec_control_t type, uint64 *value);

/* Xflow macsec Stats */
extern int
xflow_macsec_stat_get(int unit, uint32 flags,
                      xflow_macsec_id_t id,
                      xflow_macsec_stat_type_t stat_type,
                      uint64 *value);
extern int
xflow_macsec_stat_set(int unit, uint32 flags,
                      xflow_macsec_id_t id,
                      xflow_macsec_stat_type_t stat_type,
                      uint64 value);
extern int
xflow_macsec_stat_multi_get(int unit, uint32 flags,
                            xflow_macsec_id_t id,
                            uint32 num_stats,
                            xflow_macsec_stat_type_t  *stat_type_array,
                            uint64 *value_array);
extern int
xflow_macsec_stat_multi_set(int unit, uint32 flags,
                            xflow_macsec_id_t id, uint32 num_stats,
                            xflow_macsec_stat_type_t  *stat_type_array,
                            uint64 *value_array);

/* TPID array configuration. */
extern int
xflow_macsec_vlan_tpid_array_set(int unit, xflow_macsec_vlan_tpid_t *vlan_tpid);
extern int
xflow_macsec_vlan_tpid_array_get(int unit, xflow_macsec_vlan_tpid_t *vlan_tpid);
extern int
xflow_macsec_vlan_tpid_array_index_get (int unit, uint32 vlan_tpid, uint8 *tpid_index_sel);


/* XFLOW MACSEC MTU */
extern int
xflow_macsec_mtu_set (int unit, int flags, uint32 mtu, xflow_macsec_mtu_t *mtu_sel);
extern int
xflow_macsec_mtu_get (int unit, int flags, xflow_macsec_mtu_t mtu_sel, uint32 *mtu);


/* XFLOW MACSEC ETHERTYPE */
extern int
xflow_macsec_sectag_etype_set (int unit, int flags, uint32 sectag_etype,
                               xflow_macsec_sectag_ethertype_t *sectag_etype_sel);
extern int
xflow_macsec_sectag_etype_get (int unit,
                               xflow_macsec_sectag_ethertype_t sectag_etype_sel,
                               uint32 *sectag_etype);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int
xflow_macsec_wb_sync(int unit);
#endif

/* xflow_macsec_chan_traverse_cb */
typedef int (*xflow_macsec_chan_traverse_cb)(
             int unit,
             xflow_macsec_secure_chan_info_t *chan_info,
             xflow_macsec_secure_chan_id_t chan_id,
             void *user_data);

/* Traverse secure channels */
extern int xflow_macsec_secure_chan_info_traverse(
            int unit,
            uint32 flags,
            xflow_macsec_chan_traverse_cb callback,
            void *user_data);

/* bcm_xflow_macsec_secure_assoc_traverse_cb */
typedef int (*xflow_macsec_secure_assoc_traverse_cb)(
             int unit,
             xflow_macsec_secure_assoc_info_t *assoc,
             xflow_macsec_secure_chan_id_t chan_id,
             xflow_macsec_secure_assoc_id_t assoc_id,
             void *user_data);

/* Traverse Secure Associations */
extern int xflow_macsec_secure_assoc_traverse(
            int unit,
            xflow_macsec_secure_chan_id_t chan_id,
            xflow_macsec_secure_assoc_traverse_cb callback,
            void *user_data);

/* Station MAC ADR set */
extern int xflow_macsec_station_mac_addr_set(int unit, bcm_mac_t mac_addr);

/* Station MAC ADR get */
extern int xflow_macsec_station_mac_addr_get(int unit, bcm_mac_t mac_addr);


/* Event handling */

/*
 * Callback function to notify Xflow Macsec events. The flags specify whether the
 * callback is for encrypt or decrypt. The index id should be typecasted based on
 * the event.
 */
typedef int (*xflow_macsec_event_cb)(
            int unit, uint32 flags,
            xflow_macsec_instance_id_t instance_id,
            xflow_macsec_event_t event,
            xflow_macsec_id_t id,
            void *user_data);

/*
 * Xflow Macsec callback registration API. The callback function will be invoked
 * when an event occurs.
 */
extern int
xflow_macsec_event_register(int unit, xflow_macsec_event_cb cb, void *user_data);

/*
 * Xflow Macsec callback deregistration API.
 */
int
xflow_macsec_event_deregister(int unit, xflow_macsec_event_cb cb);


/* Xflow Macsec Port API */
/* Not applicable to Inline Xflow Macsec.*/
extern int
xflow_macsec_port_info_set(int unit, bcm_port_t port,
                           xflow_macsec_port_info_t *port_info);
/* Not applicable to Inline Xflow Macsec.*/
extern int
xflow_macsec_port_info_get(int unit, bcm_port_t port,
                           xflow_macsec_port_info_t *port_info);

/* Xflow Macsec interrupt handling */
extern void
xflow_macsec_process_ecc_intr(int unit, void *ecc_data);
extern void
xflow_macsec_process_functional_intr (int unit, uint64 rval64);

/* Xflow Macsec counters API */
extern void
_xflow_macsec_counters_collect(int unit);

#ifdef BCM_HURRICANE4_SUPPORT
extern int
_soc_xflow_macsec_modid_base_set(int unit, uint32 base);
#endif

/*
 * Function     - xflow_macsec_mac_addr_control_set
 * Description  - Set various mac address parameters for Xflow Macsec.
 * flags        - specifies if the API is called for encrypt or decrypt.
 * type         - type of parameter to be configured.
 * mac_addr     - mac address.
 * value        - Additional argument. Used only when specified
 *                in the type description.
 */
extern int
xflow_macsec_mac_addr_control_set(int unit, uint32 flags,
                                  xflow_macsec_instance_id_t instance_id,
                                  xflow_macsec_mac_addr_control_t control_type,
                                  xflow_macsec_mac_addr_info_t *control_info);

/*
 * Function     - xflow_macsec_mac_addr_control_get
 * Description  - Set various mac address parameters for Xflow Macsec.
 * flags        - specifies if the API is called for encrypt or decrypt.
 * type         - type of parameter to be configured.
 * mac_addr     - mac address.
 * value        - Additional argument. Used only when specified
 *                in the type description.
 */
int
xflow_macsec_mac_addr_control_get(int unit, uint32 flags,
                                  xflow_macsec_instance_id_t instance_id,
                                  xflow_macsec_mac_addr_control_t control_type,
                                  xflow_macsec_mac_addr_info_t *control_info);


#if 0
/*
 * Function    - bcm_xflow_macsec_port_control_set
 * Description - Set the per port parameters for Xflow Macsec.
 *               This API should be called only when traffic is absent.
 * Flags       - specifies if the API is called for encrypt or decrypt.
 * Port        - logical port gport or physical port ID.
 */
int xflow_macsec_port_control_set(int unit, uint32 flags, bcm_gport_t gport,
                              xflow_macsec_port_control_t type,
                              xflow_macsec_port_info_t *port_info);

/*
 * Function     - bcm_xflow_macsec_port_control_get
 * Description  - Get the per port parameters for Xflow Macsec.
 * Flags        - specifies if the API is called for encrypt or decrypt.
 * Port         - logical port gport or physical port ID.
 */
int xflow_macsec_port_control_get(int unit, uint32 flags, bcm_gport_t gport,
                              xflow_macsec_port_control_t type,
                              xflow_macsec_port_info_t *port_info);
#endif
/*
 * Function - bcm_xflow_macsec_decrypt_flow_deafult_policy_get
 * Description - Get the default Policy ID if flow lookup results in a
 *               TCAM miss. Valid only if
 *               xflow_macsec_decrypt_flow_default_policy_enable config
 *               property is enabled. This API is valid only for the
 *               Inline Xflow Macsec architecture.
 * policy_id - The decrypt policy ID assigned when a Flow lookup results in a TCAM miss.
 */
extern int
xflow_macsec_flow_default_policy_get (int unit,
                          xflow_macsec_policy_id_t *policy_id);

/*
 * Function - xflow_macsec_instance_pbmp_map_get
 * Description - Get all the instances along with the member port bitmap.
 *          If input parameter instance_max = 0, return in the output parameter
 *          instance_count will be the total number of instances.
 * unit - (IN) Unit number
 * instance_max - (IN) Maximum number of entries in the array provided.
 * instance_pbmp_map - (OUT) Array of instance to pbmp map.
 * instance_count - (OUT) Actual number of entries valid in the array.
 */
extern int
xflow_macsec_instance_pbmp_map_get (int unit, int instance_max,
    xflow_macsec_instance_pbmp_t *instance_pbmp_map,
    int *instance_count);

/*
 * Function - xflow_macsec_port_rsvd_secure_chan_get
 * Description -
 *      Get the reserved secure chan ID for the port configured in port mode.
 *      Valid only if config bcm_xflow_macsec_encrypt_phy_port_based_macsec
 *      is set for the port.
 * flags - (IN) Specify if it is encrypt or decrypt. Valid only for encrypt.
 * port - (IN) Logical port number
 * chan_id - (OUT) The reserved secure channel ID.
 */
int
xflow_macsec_port_rsvd_secure_chan_get (int unit, uint32 flags,
                            bcm_gport_t port,
                            xflow_macsec_secure_chan_id_t *chan_id);
/*
 * Function - xflow_macsec_process_intr
 * Description -
 *      Process Macsec interrupts.
 * unit - (IN) Unit
 */
void
xflow_macsec_process_intr (int unit);

/*
 * Function - xflow_macsec_port_map_info_get
 * Description -
 *      Get the Macsec block port map information given a logical port.
 *      Valid only for Inline Xflow Macsec.
 * flags - (IN) Macsec flags
 * port - (IN) Logical port number
 * port_map_info - (OUT) Port map information
 */
int
xflow_macsec_port_map_info_get(int unit, uint32 flags,
                               bcm_gport_t port,
                               xflow_macsec_port_map_info_t *port_map_info);
/*
 * Function - xflow_macsec_handle_info_get
 * Description -
 *      Get the Macsec handle information given a Macsec ID.
 * id - (IN) Macsec ID
 * handle_info - (OUT) Macsec handle information
 */

int
xflow_macsec_handle_info_get(int unit,
                             xflow_macsec_id_t id,
                             xflow_macsec_handle_info_t *handle_info);

#endif /* XFLOW_MACSEC_ESW_DEFS_H */
