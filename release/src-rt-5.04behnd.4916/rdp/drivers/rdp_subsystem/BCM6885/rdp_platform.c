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
    {"RNR_16", RNR_CORE16_ID},
    {"RNR_17", RNR_CORE17_ID},
    {"RNR_18", RNR_CORE18_ID},
    {"RNR_19", RNR_CORE19_ID},
    {"RNR_20", RNR_CORE20_ID},
    {"RNR_21", RNR_CORE21_ID},
    {"RNR_22", RNR_CORE22_ID},
    {"RNR_23", RNR_CORE23_ID},
    {"RNR_24", RNR_CORE24_ID},
    {"RNR_25", RNR_CORE25_ID},
    {"RNR_26", RNR_CORE26_ID},
    {"RNR_27", RNR_CORE27_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val quad_idx_enum_table[] = {
    {"RNR_QUAD_0", RNR_QUAD0_ID},
    {NULL, 0},
};
#endif

/*runner size per core*/
int runner_core_size[NUM_OF_TM_RUNNERS + NUM_OF_PROCESSING_RUNNERS]= { /* PP_RNR */
                                    8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,
                                    8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,8*1024,
                                    8*1024,8*1024,8*1024,8*1024,
                                    /* TM_RNR */
                                    8*1024,8*1024,8*1024,8*1024};

int reg_id[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

uint8_t valid_rnr_core[NUM_OF_TM_RUNNERS + NUM_OF_PROCESSING_RUNNERS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};