/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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
#ifndef _RDP_DQM_WKUP_H
#define _RDP_DQM_WKUP_H

#include "rdp_map.h"

enum
{
    RUNNER_CORE_A =0,
    RUNNER_CORE_B
}runner_core;

/* Bit struct in little endian */
#pragma pack(push,1)
typedef struct
{
    uint32_t runner_task          :6 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t runner_sel           :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t wake_on_not_empty    :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t wake_on_watermark    :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t wake_on_timer_expire :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t reserved:22;
} rnr_wakeup_msg_cfg;
#pragma pack(pop)

#define RNR_WKUP_CPUC_RNR_ADDR_0_OFFSET (0x180)
#define RNR_WKUP_CPUC_RNR_ADDR_1_OFFSET (0x184)

#endif
