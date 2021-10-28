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

#ifdef BCM6858
#ifdef USE_BDMF_SHELL

struct bdmfmon_enum_val bbh_id_enum_table[] = {
    {"EMAC_0", BBH_ID_XLMAC0_0_10G},
    {"EMAC_1", BBH_ID_XLMAC0_1_2p5G},
    {"EMAC_2", BBH_ID_XLMAC0_2_1G},
    {"EMAC_3", BBH_ID_XLMAC0_3_1G},
    {"EMAC_4", BBH_ID_XLMAC1_0_RGMII},
    {"EMAC_5", BBH_ID_XLMAC1_1_RGMII},
    {"EMAC_6", BBH_ID_XLMAC1_2_RGMII},
    {"EMAC_7", BBH_ID_XLMAC1_3_RGMII},
    {"WAN",    BBH_ID_PON},
    {NULL, 0},
};

struct bdmfmon_enum_val bbh_id_tx_enum_table[] = {
    {"EMAC_0", BBH_ID_XLMAC0_0_10G},
    {"EMAC_1", BBH_ID_XLMAC0_1_2p5G},
    {"EMAC_2", BBH_ID_XLMAC0_2_1G},
    {"EMAC_3", BBH_ID_XLMAC0_3_1G},
    {"EMAC_4", BBH_ID_XLMAC1_0_RGMII},
    {"EMAC_5", BBH_ID_XLMAC1_1_RGMII},
    {"EMAC_6", BBH_ID_XLMAC1_2_RGMII},
    {"EMAC_7", BBH_ID_XLMAC1_3_RGMII},
    {"WAN",    BBH_ID_PON},
    {NULL, 0},
};

struct bdmfmon_enum_val rnr_id_enum_table[] = {
    {"RNR_0", RNR_CORE0_ID},
    {"RNR_1", RNR_CORE1_ID},
    {"RNR_2", RNR_CORE2_ID},
    {"RNR_3", RNR_CORE3_ID},
    {"RNR_4", RNR_CORE4_ID},
    {"RNR_5", RNR_CORE5_ID},
    {"RNR_6", RNR_CORE6_ID},
    {"RNR_7", RNR_CORE7_ID},
    {"RNR_8", RNR_CORE8_ID},
    {"RNR_9", RNR_CORE9_ID},
    {"RNR_10", RNR_CORE10_ID},
    {"RNR_11", RNR_CORE11_ID},
    {"RNR_12", RNR_CORE12_ID},
    {"RNR_13", RNR_CORE13_ID},
    {"RNR_14", RNR_CORE14_ID},
    {"RNR_15", RNR_CORE15_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val quad_idx_enum_table[] = {
    {"RNR_QUAD_0", RNR_QUAD0_ID},
    {"RNR_QUAD_1", RNR_QUAD1_ID},
    {"RNR_QUAD_2", RNR_QUAD2_ID},
    {"RNR_QUAD_3", RNR_QUAD3_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val dma_id_enum_table[] = {
    {"DMA_0", DMA0_ID},
    {"DMA_1", DMA1_ID},
    {"SDMA_0", SDMA0_ID},
    {"SDMA_1", SDMA1_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val bacif_id_enum_table[] = {
    {"BACIF_0", RNR_QUAD0_ID},
    {"BACIF_1", RNR_QUAD1_ID},
    {"BACIF_2", RNR_QUAD2_ID},
    {"BACIF_3", RNR_QUAD3_ID},
};

#endif

int reg_id[32] = {[8] = 0, [9] = 1, [10] = 2, [11] = 4, [12] = 5, [13] = 6, [14] = 8, [15] = 9, [16] = 10, [17] = 12, [18] = 13, [19] = 14, [20] = 16, [21] = 17, [22] = 18,
    [23] = 20, [24] = 21, [25] = 22, [26] = 24, [27] = 25, [28] = 26, [29] = 28, [30] = 29, [31] = 30};
#else
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
    {"SDMA_0", SDMA0_ID},
    {NULL, 0},
};
#endif
int reg_id[32]={[8]=0, [9]=1, [10]=2, [11]=3, [12]=4, [13]=5, [14]=6, [15]=7, [16]=8, [17]=9, [18]=10, [19]=11, [20]=12, [21]=13, [22]=14,
    [23]=15, [24]=16, [25]=17, [26]=18, [27]=19, [28]=20, [29]=21, [30]=22, [31]=23};
#endif

