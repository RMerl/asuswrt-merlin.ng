/*
    Copyright 2004-2013 Broadcom Corporation

    <:label-BRCM:2013:DUAL/GPL:standard
    
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

#ifndef _ETHSW_PWRMNGT_H_
#define _ETHSW_PWRMNGT_H_

int    ethsw_shutdown_unused_phys(void);
int    ethsw_shutdown_unused_macs(BcmEnet_devctrl *pVnetDev0);
void   ethsw_setup_hw_apd(unsigned int enable);
int    ethsw_phy_pll_up(int ephy_and_gphy);
uint32 ethsw_ephy_auto_power_down_wakeup(void);
uint32 ethsw_ephy_auto_power_down_sleep(void);
void   ethsw_switch_manage_port_power_mode(int portnumber, int power_mode);
int    ethsw_switch_get_port_power_mode(int portnumber);

void   extsw_apd_set_compatibility_mode(void);
void   ethsw_deep_green_mode_handler(int linkState);
void ethsw_force_mac_up(int port);

int BcmPwrMngtGetEthAutoPwrDwn(void);
void BcmPwrMngtSetEthAutoPwrDwn(unsigned int enable, int linkState);
void BcmPwrMngtSetEnergyEfficientEthernetEn(unsigned int enable);
int BcmPwrMngtGetEnergyEfficientEthernetEn(void);

int  BcmPwrMngtGetDeepGreenMode(int mode);
void BcmPwrMngtSetDeepGreenMode(unsigned int enable, int linkState);

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
void extsw_eee_init(void);
void ethsw_eee_init(void);
void ethsw_eee_port_enable(int log_port, int enable, int linkstate);
void ethsw_eee_unimac_init(int phys_port);
void ethsw_eee_init_hw(void);
void ethsw_eee_process_delayed_enable_requests(void);
int ethsw_eee_get_by_log_port(int log_port, unsigned int *enable);
void ethsw_eee_set_by_log_port(int log_port, unsigned int enable);
int ethsw_eee_resolution_get_by_log_port(int log_port, unsigned int *enable);
#else
#define extsw_eee_init()
#define ethsw_eee_init()
#define ethsw_eee_port_enable(l,e,s)
#define ethsw_eee_unimac_init(p)
#define ethsw_eee_init_hw()
#define ethsw_eee_process_delayed_enable_requests()
#endif

#endif
