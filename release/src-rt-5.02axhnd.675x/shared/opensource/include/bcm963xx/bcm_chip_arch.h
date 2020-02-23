/*
    Copyright 2000-2012 Broadcom Corporation

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


