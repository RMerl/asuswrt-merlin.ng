/*
    <:copyright-BRCM:2014:DUAL/GPL:standard
    
       Copyright (c) 2014 Broadcom 
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

#ifndef _RDD_RUNNER_PROJ_DEFS_H
#define _RDD_RUNNER_PROJ_DEFS_H

#define NUM_OF_GLOBAL_REGS                                      0
#define NUM_OF_LOCAL_REGS                                       32
#define NUM_OF_MAIN_RUNNER_THREADS                              16

#define RDD_WAN0_VPORT       RDD_VPORT_ID_0
#define RDD_LAN0_VPORT       RDD_VPORT_ID_1
#define RDD_LAN1_VPORT       RDD_VPORT_ID_2
#define RDD_LAN2_VPORT       RDD_VPORT_ID_3
#define RDD_LAN3_VPORT       RDD_VPORT_ID_4
#define RDD_LAN4_VPORT       RDD_VPORT_ID_5
#define RDD_LAN5_VPORT       RDD_VPORT_ID_6
#define RDD_LAN6_VPORT       RDD_VPORT_ID_7
#define RDD_LAN_VPORT_LAST   RDD_LAN6_VPORT

#define RDD_CPU0_VPORT       (RDD_LAN_VPORT_LAST + 1)
#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT

#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT
#define RDD_CPU1_VPORT       (RDD_CPU_VPORT_FIRST + 1)
#define RDD_CPU2_VPORT       (RDD_CPU_VPORT_FIRST + 2)
#define RDD_CPU3_VPORT       (RDD_CPU_VPORT_FIRST + 3)
#define RDD_CPU4_VPORT       (RDD_CPU_VPORT_FIRST + 4)
#define RDD_WLAN0_VPORT      RDD_CPU4_VPORT
#define RDD_CPU5_VPORT       (RDD_CPU_VPORT_FIRST + 5)
#define RDD_WLAN1_VPORT      RDD_CPU5_VPORT
#define RDD_CPU6_VPORT       (RDD_CPU_VPORT_FIRST + 6)
#define RDD_WLAN2_VPORT      RDD_CPU6_VPORT
#define RDD_CPU_VPORT_FLOOD  (RDD_CPU_VPORT_FIRST + 7)
#define RDD_CPU_VPORT_LAST   RDD_CPU_VPORT_FLOOD


#endif
