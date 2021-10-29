/*
    Copyright 2000-2012 Broadcom Corporation

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

/**************************************************************************
 * File Name  : boardparms.h
 *
 * Description: This file contains definitions and function prototypes for
 *              the BCM63xx board parameter access functions.
 *
 * Updates    : 07/14/2003  Created.
 ***************************************************************************/

#if !defined(_BCM_CHIP_ARCH_H)
#define _BCM_CHIP_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif

extern u32 chip_arch_wan_only_portmap[];
extern u32 chip_arch_wan_pref_portmap[];
extern u32 chip_arch_lan_only_portmap[];

extern u32 chip_arch_all_portmap[];
extern u32 chip_arch_mgmt_portmap[];

#ifdef __cplusplus
}
#endif

#endif /* _BCM_CHIP_ARCH_H */


