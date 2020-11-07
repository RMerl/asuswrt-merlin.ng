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

struct bdmfmon_enum_val bacif_id_enum_table[] = {
    {"BACIF_0", BBH_ID_0},
    {"BACIF_1", BBH_ID_1},
    {"BACIF_2", BBH_ID_2},
    {"BACIF_3", BBH_ID_3},
};

struct bdmfmon_enum_val umac_misc_id_enum_table[] = {
    {"UNIMAC_0", BBH_ID_0},
    {"UNIMAC_1", BBH_ID_1},
    {"UNIMAC_2", BBH_ID_2},
    {"UNIMAC_3", BBH_ID_3},
    {"UNIMAC_4", BBH_ID_4},
    {NULL, 0},
};

struct bdmfmon_enum_val bbh_id_enum_table[] = {
    {"EMAC_0", BBH_ID_0},
    {"EMAC_1", BBH_ID_1},
    {"EMAC_2", BBH_ID_2},
    {"EMAC_3", BBH_ID_3},
    {"EMAC_4", BBH_ID_4},
    {"WAN",    BBH_ID_PON},
    {NULL, 0},
};

struct bdmfmon_enum_val bbh_id_tx_enum_table[] = {
    {"LAN", 0},
    {"WAN", 1},
    {NULL, 0},
};

struct bdmfmon_enum_val rnr_id_enum_table[] = {
    {"RNR_0", RNR_CORE0_ID},
    {"RNR_1", RNR_CORE1_ID},
    {"RNR_2", RNR_CORE2_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val quad_idx_enum_table[] = {
    {"RNR_QUAD_0", RNR_QUAD0_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val dma_id_enum_table[] = {
    {"DMA_0", DMA0_ID},
    {NULL, 0},
};

#endif

/*runner size*/
int runner_core_size[runner_image_last + 1]= {5*1024,8*1024,8*1024};


int reg_id[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

