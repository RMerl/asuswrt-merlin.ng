/*
    Copyright 2000-2012 Broadcom Corporation

    <:label-BRCM:2012:DUAL/GPL:standard
    
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
