/*
 Copyright 2004-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard    
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2, as published by
 the Free Software Foundation (the "GPL").
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 
 A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
 
 :>
*/
#ifndef _ETHSW_DMA_H_
#define _ETHSW_DMA_H_

/****************************************************************************
    Prototypes
****************************************************************************/

#define NULL_FUNC_RET_INT (0)
int ethsw_phy_pll_up(int ephy_and_gphy);
uint32 ethsw_ephy_auto_power_down_wakeup(void);
uint32 ethsw_ephy_auto_power_down_sleep(void);
int ethsw_reset_ports(struct net_device *dev);
int ethsw_enable_sar_port(void);
int ethsw_disable_sar_port(void);
int ethsw_save_port_state(void);
int ethsw_restore_port_state(void);

void ethsw_configure_ports(int port_map, int *pphy_id);


void bcmeapi_ethsw_init_config(void);
void ethsw_port_mirror_get(int *enable, int *mirror_port, unsigned int *ing_pmap,
                           unsigned int *eg_pmap, unsigned int *blk_no_mrr,
                           int *tx_port, int *rx_port);
void ethsw_port_mirror_set(int enable, int mirror_port, unsigned int ing_pmap, 
                           unsigned int eg_pmap, unsigned int blk_no_mrr,
                           int tx_port, int rx_port);

#endif /* _ETHSW_LEGACY_H_ */
