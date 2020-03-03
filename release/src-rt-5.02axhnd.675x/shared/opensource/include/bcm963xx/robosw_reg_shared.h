/*
    Copyright 2000-2012 Broadcom Corporation

    <:label-BRCM:2012:DUAL/GPL:standard
    
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

#ifndef __ROBOSW_REG_SHARED_H
#define __ROBOSW_REG_SHARED_H

void bcm_ethsw_init(void);
void bcm_ethsw_open(void);
void bcm_ethsw_close(void);
void robosw_configure_ports(void);

/* Both internal and external switch have max 8 port;
 * this assumption is followed several places */
#define MAX_SWITCH_PORTS    8

#if defined(CONFIG_BCM_EXT_SWITCH)
#define MAX_TOTAL_SWITCH_PORTS (MAX_SWITCH_PORTS + MAX_SWITCH_PORTS)
#else
#define MAX_TOTAL_SWITCH_PORTS MAX_SWITCH_PORTS
#endif

#endif
