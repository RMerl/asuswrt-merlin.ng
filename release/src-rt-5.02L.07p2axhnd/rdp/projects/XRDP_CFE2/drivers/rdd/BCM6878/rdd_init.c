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

* :>
*/

#include "rdd.h"
#include "rdd_common.h"
#include "rdd_defs.h"
#include "rdd_init.h"

#include "XRDP_AG.h"
#include "rdd_map_auto.h"
#include "rdd_common.h"
#include "rdp_common.h"
#include "rdd_runner_proj_defs.h"
#include "xrdp_drv_rnr_regs_ag.h"

extern int reg_id[32];
extern RDD_FPM_GLOBAL_CFG_DTS g_fpm_hw_cfg;

int rdd_init(void)
{
    return 0;
}

void rdd_exit(void)
{
}

#ifndef RDP_SIM
void rdp_rnr_write_context(void *__to, void *__from, unsigned int __n);
#endif

static void rdd_global_registers_init(uint32_t core_index, uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][NUM_OF_LOCAL_REGS], uint32_t last_thread)
{
     uint32_t i;

    /********** Reserved global registers **********/
    /* R6 - ingress qos don't drop counter in processing cores */

    /********** common to all runners **********/
    /* in this project we don't have really global, so will set all registers that should be global for all threads */
    for (i = 0; i <= last_thread; ++i)
    {
        local_regs[i][1] = 1; /* CONST_1 is 1 */
    }
}

static void image_0_context_set(uint32_t core_index)
{
    static uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][NUM_OF_LOCAL_REGS];
    uint32_t *sram_context;

    rdd_global_registers_init(core_index, local_regs, core_index);
    sram_context = (uint32_t *)RUNNER_CORE_CONTEXT_ADDRESS(core_index);
#ifndef XRDP_EMULATION
    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
     MREAD_BLK_32(local_regs, sram_context, sizeof(local_regs));
#endif


    /* UPDATE_FIFO_READ: thread 1 */
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[0]]  = ADDRESS_OF(image_0, direct_processing_wakeup_request);
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[2]] = IMAGE_0_RX_FLOW_TABLE_ADDRESS << 16 | IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER;
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[10]] = BB_ID_DISPATCHER_REORDER;
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[11]] = 0;
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[16]] =IMAGE_0_DIRECT_PROCESSING_PD_TABLE_ADDRESS;

     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[12]] = IMAGE_0_CPU_RING_DESCRIPTORS_TABLE_ADDRESS;
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[17]] = IMAGE_0_SRAM_PD_FIFO_ADDRESS;

     /* CPU_TX: thread 1 */
     local_regs[IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER][reg_id[0]] = ADDRESS_OF(image_0, cpu_tx_wakeup_request);
     local_regs[IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER][reg_id[8]] = IMAGE_0_BBH_TX_RING_TABLE_ADDRESS;
     local_regs[IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER][reg_id[9]] = IMAGE_0_BBH_TX_BB_DESTINATION_TABLE_ADDRESS;

#if defined(RDP_SIM) || defined(XRDP_EMULATION) 
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32(sram_context, local_regs, sizeof(local_regs));
#else
     rdp_rnr_write_context(sram_context, local_regs, sizeof(local_regs));
#endif
}

static void rdd_local_registers_init(void)
{
    image_0_context_set(0);

}

int rdd_data_structures_init(rdd_init_params_t *init_params)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    rdd_local_registers_init();


    rdd_fpm_pool_number_mapping_cfg(512);
    /* write global FPM congifuration */
    RDD_FPM_GLOBAL_CFG_FPM_BASE_LOW_WRITE_G(g_fpm_hw_cfg.fpm_base_low,RDD_FPM_GLOBAL_CFG_ADDRESS_ARR,0);
    RDD_FPM_GLOBAL_CFG_FPM_BASE_HIGH_WRITE_G(g_fpm_hw_cfg.fpm_base_high,RDD_FPM_GLOBAL_CFG_ADDRESS_ARR,0);
    RDD_FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_WRITE_G(g_fpm_hw_cfg.fpm_token_size_asr_8,RDD_FPM_GLOBAL_CFG_ADDRESS_ARR,0);


    return rc;
}


