/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
       All Rights Reserved
    
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
