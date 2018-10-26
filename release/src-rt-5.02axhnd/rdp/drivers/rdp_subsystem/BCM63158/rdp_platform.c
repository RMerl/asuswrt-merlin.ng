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

/* TBD BCM63158 figure out correct values for all the definitions. */
struct bdmfmon_enum_val bacif_id_enum_table[] = {
    {"BACIF_0", BBH_ID_0},
    {"BACIF_1", BBH_ID_1},
    {"BACIF_2", BBH_ID_2},
};

struct bdmfmon_enum_val umac_misc_id_enum_table[] = {
    {"UNIMAC_0", BBH_ID_0},
    {"UNIMAC_1", BBH_ID_1},
    {"UNIMAC_2", BBH_ID_2},
    {"UNIMAC_3", BBH_ID_AE10},
    {"UNIMAC_4", BBH_ID_AE2P5},
    {NULL, 0},
};

struct bdmfmon_enum_val bbh_id_enum_table[] = {
    {"EMAC_0", BBH_ID_0},
    {"EMAC_1", BBH_ID_1},
    {"EMAC_2", BBH_ID_2},
    {"WAN_1",  BBH_ID_PON},
    {"WAN_2",  BBH_ID_AE10},
    {"WAN_3",  BBH_ID_AE2P5},
    {"WAN_4",  BBH_ID_DSL},
    {NULL, 0},
};

struct bdmfmon_enum_val bbh_id_tx_enum_table[] = {
    {"LAN",    BBH_TX_ID_0},
    {"WAN_1",  BBH_TX_ID_PON},
    {"WAN_2",  BBH_TX_ID_AE10},
    {"WAN_3",  BBH_TX_ID_AE2P5},
    {"WAN_4",  BBH_TX_ID_DSL},
    {NULL, 0},
};

struct bdmfmon_enum_val rnr_id_enum_table[] = {
    {"RNR_0", RNR_CORE0_ID},
    {"RNR_1", RNR_CORE1_ID},
    {"RNR_2", RNR_CORE2_ID},
    {"RNR_3", RNR_CORE3_ID},
    {"RNR_4", RNR_CORE4_ID},
    {"RNR_5", RNR_CORE5_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val quad_idx_enum_table[] = {
    {"RNR_QUAD_0", RNR_QUAD0_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val dma_id_enum_table[] = {
    {"DMA_0", DMA0_ID},
    {"SDMA_0", SDMA0_ID},
    {"SDMA_1", SDMA1_ID},
    {NULL, 0},
};

#endif

int reg_id[32]={[8]=0, [9]=1, [10]=2, [11]=3, [12]=4, [13]=5, [14]=6, [15]=7, [16]=8, [17]=9, [18]=10, [19]=11, [20]=12, [21]=13, [22]=14,
    [23]=15, [24]=16, [25]=17, [26]=18, [27]=19, [28]=20, [29]=21, [30]=22, [31]=23};

