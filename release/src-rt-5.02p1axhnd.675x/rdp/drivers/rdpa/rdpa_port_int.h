/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#ifndef _RDPA_PORT_INT_H_
#define _RDPA_PORT_INT_H_

#include "bdmf_dev.h"
#include "bdmf_errno.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdpa_policer_ex.h"
#ifndef XRDP
#include "rdp_drv_ih.h"
#include "rdp_drv_bbh.h"
#else
#include "rdp_drv_bbh_tx.h"
#include "rdp_drv_bbh_rx.h"
#include "rdp_drv_rnr.h"
#endif
#include "rdd.h"
#include "rdd_data_structures_auto.h"

extern const bdmf_attr_enum_table_t rdpa_physical_port_enum_table;

#define RDPA_PORT_MAX_MAC  8 /**< Max number of PORT MAC addresses */

#define EXCEPTION_RX_MIRROR (1 << 0)
#define EXCEPTION_PORT_FW_LOOPBACK (1 << 1)
#define EXCEPTION_PORT_WAN_GBE (1 << 2)
#ifdef G9991
#define EXCEPTION_PORT_SYSTEM (1 << 3)
#endif
#define EXCEPTION_PORT_FLOW_CTRL (1 << 4)
#define EXCEPTION_PORT_DBG_EN (1 << 5)
#define EXCEPTION_PORT_RATE_LIMIT_ALL_TRAFFIC (1 << 6)
/* port object private data */
typedef struct
{
    rdpa_if index; /**< port index */
    rdpa_wan_type wan_type; /**< WAN type */
    rdpa_speed_type speed; /**< Ethernet speed for epon mode, must be set after /ref ae_enable */
    rdpa_port_dp_cfg_t cfg; /**< Logical configuration */
    rdpa_port_tm_cfg_t tm_cfg; /**< TM configuration */
    rdpa_ic_result_t default_cfg; /**< Default configuration */
    bdmf_boolean default_cfg_exist; /**< flag for post_init*/
    rdpa_port_sa_limit_t sa_limit; /**< SA limit configuration */
    rdpa_port_flow_ctrl_t flow_ctrl; /**< Ingress emac flow control */
    rdpa_port_ingress_rate_limit_t ingress_rate_limit; /**< Ingress rate limit */
    bdmf_index def_flow_index;  /**index of def flow context configuration */
    rdpa_port_mirror_cfg_t mirror_cfg; /**< Port mirroring configuration */
    rdpa_stat_tx_rx_t host_stat;  /**< Host statistics shadow */
    rdpa_port_vlan_isolation_t vlan_isolation;  /**<Port VLAN isolation control */
    bdmf_boolean transparent;  /**<Port  transparent control */
    int channel;        /* port channel (RDD) */
    rdpa_port_loopback_t  loopback_cfg; /* port loopbacks configuration */
    uint32_t mtu_size;  /*port mtu size*/
    bdmf_object_handle cpu_obj; /**< CPU object for exception/forwarding-through-cpu packets */
    bdmf_index cpu_meter; /**< Index of per-port CPU meter. If set, supersedes per-reason meter configuration */
    uint32_t exception_flags;
    bdmf_object_handle bridge_obj; /**< Bridge object that is linked to port */
    rdpa_filter_ctrl_t ingress_filters[RDPA_FILTERS_QUANT]; /** Ingress filters per port */
    uint8_t ingress_filters_profile; /**< Profile ID for internal usage, \XRDP_LIMITED */
    uint32_t proto_filters; /**< Map of rdpa_proto_filtres_t */
    int has_emac; /* True if port should have emac */
    bdmf_boolean enable; /* enable or disable port */
    rdpa_discard_prty pbit_to_dp_map[8]; /* Pbit to drop precedence configuration map */
    uint32_t pkt_size_stat_en; /**< enable debug statistics packets number per packet size \XRDP_LIMITED */
    bdmf_mac_t mac;
    int mac_addr_idx;
    uint32_t options; /**< Reserved flags \XRDP_LIMITED */
    bdmf_handle def_vlan_obj;  /* pointer to default vlan if linked to port, optional */
} port_drv_priv_t;

typedef enum
{
    TX_MIRRORING,
    RX_MIRRORING
} port_mirror_type_t;

extern bdmf_fastlock port_fastlock;
extern struct bdmf_object *port_objects[];

static inline int _rdpa_port_get_locked(rdpa_if index,
    port_drv_priv_t **port)
{
    struct bdmf_object *mo;
    if ((unsigned)index >= rdpa_if__number_of)
        return -1;
    bdmf_fastlock_lock(&port_fastlock);
    mo = port_objects[index];
    if (!mo)
    {
        bdmf_fastlock_unlock(&port_fastlock);
        return -1;
    }
    *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    return 0;
}

static inline int _rdpa_port_channel(rdpa_if index)
{
    port_drv_priv_t *port;
    int channel, rc;

    rc = _rdpa_port_get_locked(index, &port);
    if (rc)
        return -1;
    channel = port->channel;
    bdmf_fastlock_unlock(&port_fastlock);
    return channel;
}

static inline int _rdpa_port_channel_no_lock(rdpa_if index)
{
    struct bdmf_object *mo;
    port_drv_priv_t *port;

    if ((unsigned)index >= rdpa_if__number_of)
        return -1;
    mo = port_objects[index];
    if (!mo)
        return -1;
    port = (port_drv_priv_t *)bdmf_obj_data(mo);
    return port->channel;
}

static inline bdmf_object_handle _rdpa_port_get_linked_bridge(rdpa_if index)
{
    struct bdmf_object *mo;
    port_drv_priv_t *port;

    if ((unsigned)index >= rdpa_if__number_of)
        return NULL;
    mo = port_objects[index];
    if (!mo || mo->state == bdmf_state_deleted)
        return NULL;
    port = (port_drv_priv_t *)bdmf_obj_data(mo);
    return port->bridge_obj;
}

int rdpa_update_da_sa_searches(rdpa_if port, bdmf_boolean dal);
#ifndef XRDP
void update_port_tag_size(rdpa_emac emac, DRV_IH_PROPRIETARY_TAG_SIZE new_tag_size);
#else
void update_port_tag_size(rdpa_emac emac, drv_rnr_prop_tag_size_t new_tag_size);
void update_broadcom_tag_size(void);
int port_def_flow_cntr_add(port_drv_priv_t *port);
void port_flow_add(rdpa_if if_vport);
void port_flow_del(port_drv_priv_t *port);
int rdpa_if_rdd_vport_to_rdpa_is_set(rdpa_if port);
#endif
int rdpa_if_to_rdpa_physical_port(rdpa_if port, rdpa_physical_port *physical_port);
int port_attr_mtu_size_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int port_attr_mtu_size_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);

bdmf_error_t rdpa_port_tm_discard_prty_cfg_ex(struct bdmf_object *mo, rdpa_port_tm_cfg_t *tm_cfg);

int port_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);
int port_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int port_attr_pkt_size_stat_en_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);

int port_attr_debug_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);
int port_attr_debug_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int port_attr_pkt_size_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);
int port_attr_pkt_size_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);

int port_attr_uninit_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, 
    uint32_t size);
    
int port_ls_fc_cfg_ex(struct bdmf_object *mo, rdpa_port_dp_cfg_t *cfg);
int port_mirror_cfg_ex(struct bdmf_object *mo, port_drv_priv_t *port, rdpa_port_mirror_cfg_t *mirror_cfg);
int port_attr_loopback_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);
int port_attr_options_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);
int port_vlan_isolation_cfg_ex(struct bdmf_object *mo,
    rdpa_port_vlan_isolation_t *vlan_isolation_cfg, bdmf_boolean is_active);
int port_flow_control_cfg_ex(port_drv_priv_t *port, rdpa_port_flow_ctrl_t *flow_ctrl);
int port_ingress_rate_limit_cfg_ex(port_drv_priv_t *port, rdpa_port_ingress_rate_limit_t *ingress_rate_limit);
bdmf_error_t common_timer_init(void);
int port_set_bbh_timer_clock_ex(port_drv_priv_t *port);
int port_post_init_ex(struct bdmf_object *mo);
int port_attr_wan_type_write_ex(port_drv_priv_t *port, rdpa_wan_type wan_type);
int port_attr_cpu_obj_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int port_attr_cpu_meter_write_ex(struct bdmf_object *mo, rdpa_traffic_dir dir, rdpa_if intf, bdmf_index meter_idx);
int port_attr_mac_write_ex(struct bdmf_object *mo, bdmf_mac_t *mac);

int rdpa_cfg_sa_da_lookup(port_drv_priv_t *port, rdpa_port_dp_cfg_t *cfg, bdmf_boolean old_sa_action,
    bdmf_boolean is_active);
int port_update_all_ports_set(bdmf_object_handle mo, int is_add);

int rdpa_cfg_sa_da_lookup_ex(port_drv_priv_t *port, rdpa_port_dp_cfg_t *cfg);
int port_attr_pfc_tx_enable_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);
int port_attr_pfc_tx_enable_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);
int port_attr_pfc_tx_timer_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);
int port_attr_pfc_tx_timer_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);

/* rdpa_if <-> rdpa_emac */
rdpa_if rdpa_port_emac_to_rdpa_if(rdpa_emac emac);
rdpa_emac rdpa_port_rdpa_if_to_emac(rdpa_if port);

/* rdpa_if <-> rdd_vport_id_t */
rdd_vport_id_t rdpa_port_rdpa_if_to_vport(rdpa_if port);
rdpa_if rdpa_port_vport_to_rdpa_if(rdd_vport_id_t rdd_vport);
void rdpa_port_rdpa_if_to_vport_set(rdpa_if port, rdd_vport_id_t rdd_vport, bdmf_boolean set);
#ifndef XRDP
rdpa_if rdd_vport_to_rdpa_if(rdd_vport_id_t vport, uint8_t wifi_ssid);
rdd_vport_id_t rdpa_if_to_rdd_vport(rdpa_if port, uint8_t *rdd_wifi_ssid);
#else
int update_port_bridge_and_vlan_lookup_method_ex(rdpa_if port);
rdd_vport_id_t rdpa_if_to_rdd_vport(rdpa_if port, rdpa_wan_type wan_type);
#endif

#ifndef XRDP
uint32_t rdpa_port_ports2rdd_ssid_vector(rdpa_ports ports);
rdpa_ports rdpa_port_ssid_vector2rdpa_ports(uint32_t ssid_vector);
#endif

int rdpa_port_lag_link_ex(port_drv_priv_t *lag_port);
void rdpa_port_lag_unlink_ex(port_drv_priv_t *lag_port);
int rdpa_port_bond_link_ex(rdpa_physical_port physical_port);
int rdpa_port_bond_unlink_ex(rdpa_physical_port physical_port);
int rdpa_port_cfg_min_packet_size_get_ex(port_drv_priv_t *port, uint8_t *min_packet_size);
int rdpa_port_cfg_min_packet_size_set_ex(port_drv_priv_t *port, uint8_t min_packet_size);


int rdpa_port_get_egress_tm_channel_from_port_ex(rdpa_wan_type wan_type, rdpa_if port, int *channel_id);

#ifdef XRDP
uint32_t rdpa_port_rx_flow_src_port_get(rdpa_if port, int set_lan_indication);
uint32_t disabled_proto_mask_get(uint32_t proto_filters_mask);
#endif

int port_attr_ingress_filter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int port_attr_proto_filters_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);

int _rdpa_port_set_linked_bridge(rdpa_if index, bdmf_object_handle bridge_obj);
int _rdpa_port_set_linked_bridge_ex(struct bdmf_object *mo, bdmf_object_handle bridge_obj);

int mac_lkp_cfg_validate_ex(rdpa_mac_lookup_cfg_t *mac_lkp_cfg, port_drv_priv_t *port, int ls_fc_enable);
void port_destroy_ex(struct bdmf_object *mo);

int port_attr_pbit_to_dp_map_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);

rdpa_port_sa_limit_t *_rdpa_bridge_port_get_fdb_limit(rdpa_if index);
int _rdpa_bridge_port_sa_limit_enable(rdpa_if index);
int _rdpa_bridge_port_inc_dec_num_sa(rdpa_if index, bdmf_boolean is_inc);
int _rdpa_bridge_port_update_sa_miss_action(rdpa_if index, rdpa_forward_action sal_miss_action);
int rdpa_port_reconfig_rx_mirroring_ex(port_drv_priv_t *port);
#endif


