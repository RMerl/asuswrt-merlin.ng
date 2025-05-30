/*
   <:copyright-BRCM:2018:DUAL/GPL:standard

      Copyright (c) 2018 Broadcom
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
 *  Created on: May/2018
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef _SF2_COMMON_H_
#define _SF2_COMMON_H_
#include "sw_common.h"
#include <linux/version.h>

int port_sf2_sw_init(enetx_port_t *self);
int port_sf2_sw_uninit(enetx_port_t *self);
int port_sf2_sw_hw_sw_state_set(enetx_port_t *sw, unsigned long state);
int port_sf2_sw_hw_sw_state_get(enetx_port_t *sw);
int port_sf2_sw_config_trunk(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add);
int port_sf2_sw_update_pbvlan(enetx_port_t *sw, unsigned int pmap);

int port_sf2_port_init(enetx_port_t *self);
int port_sf2_tx_pkt_mod(enetx_port_t *port, pNBuff_t *pNBuff, uint8_t **data, uint32_t *len, unsigned int port_map);
int port_sf2_rx_pkt_mod(enetx_port_t *port, struct sk_buff *skb);
uint32_t port_sf2_tx_q_remap(enetx_port_t *port, uint32_t txq);
uint16_t port_sf2_tx_lb_imp(enetx_port_t *port, uint16_t port_id, void* pHdr);
int port_sf2_mib_dump_us(enetx_port_t *self, void *e); // add by Andrew
void port_sf2_generic_open(enetx_port_t *self);
void extlh_mac2mac_port_handle(enetx_port_t *self);

#if defined(CONFIG_NET_SWITCHDEV)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
int sf2_switchdev_port_attr_get(struct net_device *dev, struct switchdev_attr *attr);
int sf2_switchdev_port_attr_set(struct net_device *dev, const struct switchdev_attr *attr, struct switchdev_trans *trans);
#endif
int sf2_switchdev_init(void);
#endif

int enetxapi_post_sf2_config(void);

#endif

