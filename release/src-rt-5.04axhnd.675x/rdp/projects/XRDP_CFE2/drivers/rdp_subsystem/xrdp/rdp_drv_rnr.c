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

#include "rdp_subsystem_common.h"
#include "rdp_drv_rnr.h"
#include "rdp_common.h"
#include "rdd_fw_defs.h"
#include "XRDP_AG.h"
#include "fw_binary_auto.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_map_auto.h"
#include "rdd_data_structures_auto.h"
#if defined(__UBOOT__)
#include "bcm_ubus4.h"
#include <linux/compat.h>
#else
#if !defined(RDP_SIM)
#include "bcm_map_part.h"
#include "bcm_ubus4.h"
#endif
#endif
#include "rdd_defs.h"
#include "xrdp_drv_ubus_slv_ag.h"


#define RNR_NOP_OPCODE  0xFC000000

static int _num_of_cores = GROUPED_EN_SEGMENTS_NUM;

uintptr_t rdp_runner_core_addr[GROUPED_EN_SEGMENTS_NUM];


int drv_rnr_dma_cfg(rnr_dma_regs_cfg_t *rnr_dma_cfg)
{
    uint8_t rnr_idx;
    int rc = BDMF_ERR_OK;

    for (rnr_idx = 0; rnr_idx < _num_of_cores; rnr_idx++)
    {

#if CHIP_VER < RDP_GEN_60
        rc = ag_drv_rnr_regs_cfg_ddr_cfg_set(rnr_idx, rnr_dma_cfg->ddr.dma_base, rnr_dma_cfg->ddr.dma_buf_size, rnr_dma_cfg->ddr.dma_static_offset);
#else
        rc = ag_drv_rnr_regs_cfg_ddr_cfg_set(rnr_idx, rnr_dma_cfg->ddr.dma_base, rnr_dma_cfg->ddr.dma_buf_size, 1, rnr_dma_cfg->ddr.dma_static_offset);
#endif
        /* each quad should go to a different UBUS slave in PSRAM */
#if defined(BCM6858)
        rc = rc ? rc : ag_drv_rnr_regs_cfg_psram_cfg_set(rnr_idx, (rnr_dma_cfg->psram.dma_base + (rnr_idx/4)), rnr_dma_cfg->psram.dma_buf_size, rnr_dma_cfg->psram.dma_static_offset);
#elif CHIP_VER >= RDP_GEN_60
        rc = rc ? rc : ag_drv_rnr_regs_cfg_psram_cfg_set(rnr_idx, rnr_dma_cfg->psram.dma_base, rnr_dma_cfg->psram.dma_buf_size, 0, rnr_dma_cfg->psram.dma_static_offset);
#else
        rc = rc ? rc : ag_drv_rnr_regs_cfg_psram_cfg_set(rnr_idx, rnr_dma_cfg->psram.dma_base, rnr_dma_cfg->psram.dma_buf_size, rnr_dma_cfg->psram.dma_static_offset);
#endif
	}
    return rc;
}

void drv_rnr_cores_addr_init(void)
{
    uint32_t i;

    for (i = 0; i < _num_of_cores; i++)
    {
        rdp_runner_core_addr[i] = RU_BLK(RNR_MEM).addr[i] + RU_REG_OFFSET(RNR_MEM, HIGH);
        ACCESS_LOG(ACCESS_LOG_OP_SET_CORE_ADDR, rdp_runner_core_addr[i], i, 0);
    }
}

inline void drv_rnr_profiling_clear_trace(uint8_t core_id)
{

}

void drv_rnr_profiling_core_init(uint8_t core_id)
{

}

void drv_rnr_quad_profiling_quad_init(rnr_quad_id_e quad_id)
{

}

void drv_rnr_mem_init(void)
{
    uint8_t rnr_idx;
    uint32_t mem_byte_num, mem_cntxt_byte_num;

    mem_byte_num = 2 * (RU_REG_RAM_CNT(RNR_MEM, HIGH) + 1) * sizeof(uint32_t);
    mem_cntxt_byte_num = (RU_REG_RAM_CNT(RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);

    for (rnr_idx = 0; rnr_idx < _num_of_cores; rnr_idx++)
    {
#ifndef XRDP_EMULATION
        /* reset SRAM memory */
        MEMSET((uint32_t *)DEVICE_ADDRESS(RU_BLK(RNR_MEM).addr[rnr_idx] + RU_REG_OFFSET(RNR_MEM, HIGH)), 0, mem_byte_num);

        /* reset SRAM context memory */
        MEMSET((uint32_t *)DEVICE_ADDRESS(RU_BLK(RNR_CNTXT).addr[rnr_idx] + RU_REG_OFFSET(RNR_CNTXT, MEM_ENTRY)), 0, mem_cntxt_byte_num);
#endif
    }
}

void drv_rnr_load_microcode(void)
{
    uint8_t rnr_idx;

#if defined(XRDP_EMULATION) || defined (RDP_SIM)
	uint32_t mem_inst_byte_num = (RU_REG_RAM_CNT(RNR_INST, MEM_ENTRY) + 1) * sizeof(uint32_t);

    for (rnr_idx = 0; rnr_idx < _num_of_cores; rnr_idx++)
    {
         MWRITE_BLK_32(DEVICE_ADDRESS(RU_BLK(RNR_INST).addr[rnr_idx] + RU_REG_OFFSET(RNR_INST, MEM_ENTRY)),
            (uint8_t *)fw_inst_binaries[rnr_idx], mem_inst_byte_num);
    }
#else
    uint32_t inst;
    uint32_t mem_inst_word_num = (RU_REG_RAM_CNT(RNR_INST, MEM_ENTRY) + 1) ;
#ifdef CONFIG_BRCM_IKOS
    uint8_t nops;
#endif
    for (rnr_idx = 0; rnr_idx < _num_of_cores; rnr_idx++)
    {
#ifdef CONFIG_BRCM_IKOS
    	nops = 0;
#endif
#if defined(PHYS_ADDR_64BIT) && !defined(CONFIG_GPL_RDP_GEN)
        for (inst = 0; inst < mem_inst_word_num; inst += 2)
        {
            MWRITE_I_64(DEVICE_ADDRESS(RU_BLK(RNR_INST).addr[rnr_idx]), inst >> 1,
                *(uint64_t*)&fw_inst_binaries[rnr_idx][inst]);
        }
#else
        /* Pre-init instruction memory with NOPs */
        MEMSET_32(DEVICE_ADDRESS(RU_BLK(RNR_INST).addr[rnr_idx]), RNR_NOP_OPCODE, mem_inst_word_num);

        for (inst = 0; inst < mem_inst_word_num; inst ++)
        {
#ifdef CONFIG_BRCM_IKOS
            if (nops > 25)
                break;
            if (fw_inst_binaries[rnr_idx][inst+1] == 0xFC000000)
            {
                if (fw_inst_binaries[rnr_idx][inst] == 0xFC000000)
                    nops +=2;
                else
                    nops = 1;
            }
            else
            {
                nops = 0;
            }
#endif
            if (*(uint32_t*)&fw_inst_binaries[rnr_idx][inst] != RNR_NOP_OPCODE)
            {
                MWRITE_I_32(DEVICE_ADDRESS(RU_BLK(RNR_INST).addr[rnr_idx]), inst,
                    *(uint32_t*)&fw_inst_binaries[rnr_idx][inst]);
            }
        }
#endif
    }
#endif
}

static void memcpyl_prediction(void *__to, void *__from, unsigned int __n)
{
    uint8_t *src = (uint8_t *)__from;
    uint8_t *dst = (uint8_t *)__to;

    int i;
    for (i = 0; i < (__n / 2); i++, src += 2, dst += 4)
        MWRITE_32(dst, (unsigned int)(*(volatile unsigned short *)src));
}

void drv_rnr_load_prediction(void)
{
    uint8_t rnr_idx;
    uint32_t mem_pred_byte_num = (RU_REG_RAM_CNT(RNR_PRED, MEM_ENTRY) + 1) * sizeof(uint16_t);

    for (rnr_idx = 0; rnr_idx < _num_of_cores; rnr_idx++)
    {
#ifndef XRDP_EMULATION
        memcpyl_prediction((void *)(DEVICE_ADDRESS(RU_BLK(RNR_PRED).addr[rnr_idx] + RU_REG_OFFSET(RNR_PRED, MEM_ENTRY))), fw_pred_binaries[rnr_idx], mem_pred_byte_num);
#else
        MWRITE_BLK_16(DEVICE_ADDRESS(RU_BLK(RNR_PRED).addr[rnr_idx] + RU_REG_OFFSET(RNR_PRED, MEM_ENTRY)), fw_pred_binaries[rnr_idx], mem_pred_byte_num);
#endif
    }
}

void rdp_rnr_write_context(void *__to, void *__from, unsigned int __n)
{
    uint8_t *src = (uint8_t *)__from;
    uint8_t *dst = (uint8_t *)__to;
    int i, n = __n / 4;

    for (i = 0; i < n; i++, src += 4, dst += 4)
    {
/* In 68460: and 63158, context memory has logical size 96x128 */
#if defined(BCM6858) || defined(BCM6836)
        if ((i & 0x3) == 3)
            continue;
#endif
        MWRITE_32(dst, *(volatile unsigned int *)src);
    }
}

int drv_rnr_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val)
{

     rnr_quad_general_config_powersave_config rnr_ctrl;
     uint8_t block_id = 0;

     for (block_id = 0; block_id < RU_BLK(RNR_QUAD).addr_count; block_id++) {
         ag_drv_rnr_quad_general_config_powersave_config_get(block_id, &rnr_ctrl);
         rnr_ctrl.enable_powersave_core_0 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_1 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_2 = auto_gate ? 1 : 0;
#if !defined(BCM6846) && !defined(BCM6878) && !defined(BCM63146) && !defined(BCM4912)
         rnr_ctrl.enable_powersave_core_3 = auto_gate ? 1 : 0;
#ifndef BCM6858
         rnr_ctrl.enable_powersave_core_4 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_5 = auto_gate ? 1 : 0;
#endif
#endif
         rnr_ctrl.time_counter = timer_val;
         ag_drv_rnr_quad_general_config_powersave_config_set(block_id, &rnr_ctrl);
     }

     return 0;
}

void drv_rnr_num_of_cores_set(int num_of_cores)
{
    _num_of_cores = num_of_cores;
}

