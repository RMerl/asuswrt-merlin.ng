/*
 Copyright 2004-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard    
 
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
