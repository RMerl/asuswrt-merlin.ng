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

static void rdd_global_registers_init(uint32_t core_index)
{
    static uint32_t global_regs[8] = {};
    uint32_t i;

    /********** common to all runners **********/
    global_regs[1] = 1; /* R1 = 1 */

    for (i = 0; i < 8; ++i)
        RDD_BYTES_4_BITS_WRITE(global_regs[i], (uint8_t *)RDD_RUNNER_GLOBAL_REGISTERS_INIT_PTR(core_index) + (sizeof(RDD_BYTES_4_DTS) * i));
}

static void image_0_context_set(uint32_t core_index)
{
    static uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][NUM_OF_LOCAL_REGS];
    uint32_t *sram_context;

    rdd_global_registers_init(core_index);
    sram_context = (uint32_t *)RUNNER_CORE_CONTEXT_ADDRESS(core_index);

#ifndef XRDP_EMULATION
    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
     MREAD_BLK_32(local_regs, sram_context, sizeof(local_regs));
#endif

     /* CPU_RX : thread 0 */
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[9]] = IMAGE_0_RX_FLOW_TABLE_ADDRESS << 16 | IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER;
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[10]] = BB_ID_DISPATCHER_REORDER;
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[11]] = 0;
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[16]] =
         ADDRESS_OF(image_0, direct_processing_wakeup_request) << 16 | IMAGE_0_DIRECT_PROCESSING_PD_TABLE_ADDRESS;

     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[12]] = IMAGE_0_CPU_RING_DESCRIPTORS_TABLE_ADDRESS;
     local_regs[IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER][reg_id[17]] = IMAGE_0_SRAM_PD_FIFO_ADDRESS;

     /* CPU_TX: thread 1 */
     local_regs[IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, cpu_tx_wakeup_request) << 16;
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

    /*CFE works only with fpm buff size of 512 */
    rdd_fpm_pool_number_mapping_cfg(512);
    /* write global FPM congifuration */
    RDD_FPM_GLOBAL_CFG_FPM_BASE_LOW_WRITE_G(g_fpm_hw_cfg.fpm_base_low,RDD_FPM_GLOBAL_CFG_ADDRESS_ARR,0);
    RDD_FPM_GLOBAL_CFG_FPM_BASE_HIGH_WRITE_G(g_fpm_hw_cfg.fpm_base_high,RDD_FPM_GLOBAL_CFG_ADDRESS_ARR,0);
    RDD_FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_WRITE_G(g_fpm_hw_cfg.fpm_token_size_asr_8,RDD_FPM_GLOBAL_CFG_ADDRESS_ARR,0);

    return rc;
}



