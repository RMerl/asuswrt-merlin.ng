/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#include "rdp_common.h"
#include "rdd_data_structures_auto.h"

#ifdef USE_BDMF_SHELL

struct bdmfmon_enum_val tbl_idx_enum_table[] = {
    {"TBL_0", NATC_TBL0_ID},
    {"TBL_1", NATC_TBL1_ID},
    {"TBL_2", NATC_TBL2_ID},
    {"TBL_3", NATC_TBL3_ID},
    {"TBL_4", NATC_TBL4_ID},
    {"TBL_5", NATC_TBL5_ID},
    {"TBL_6", NATC_TBL6_ID},
    {"TBL_7", NATC_TBL7_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val eng_idx_enum_table[] = {
    {"ENG_0", NATC_ENG0_ID},
    {"ENG_1", NATC_ENG1_ID},
    {"ENG_2", NATC_ENG2_ID},
    {"ENG_3", NATC_ENG3_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val ubus_mstr_id_enum_table[] = {
    {"UBUS_MSTR_0", UBUS_MSTR0_ID},
    {"UBUS_MSTR_1", UBUS_MSTR1_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val channel_id_enum_table[] = {
    {"CHANNEL_0", CHANNEL0_ID},
    {"CHANNEL_1", CHANNEL1_ID},
    {"CHANNEL_2", CHANNEL2_ID},
    {"CHANNEL_3", CHANNEL3_ID},
    {"CHANNEL_4", CHANNEL4_ID},
    {"CHANNEL_5", CHANNEL5_ID},
    {"CHANNEL_6", CHANNEL6_ID},
    {"CHANNEL_7", CHANNEL7_ID},
    {NULL, 0},
};
#endif
#ifdef XRDP_EMULATION
/* used for ACCESS_MACORS, defined at emulation env. */
write32_p write32;
write16_p write16;
write8_p write8;
read32_p read32;
read16_p read16;
read8_p read8;
#endif
