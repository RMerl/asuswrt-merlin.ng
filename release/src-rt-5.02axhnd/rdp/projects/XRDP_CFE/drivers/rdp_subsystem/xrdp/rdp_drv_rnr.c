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
#include "XRDP_AG.h"
#include "fw_binary_auto.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_map_auto.h"
#include "rdd_data_structures_auto.h"
#if !defined(RDP_SIM)
#include "bcm_map_part.h"
#include "bcm_ubus4.h"
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
        rc = ag_drv_rnr_regs_cfg_ddr_cfg_set(rnr_idx, rnr_dma_cfg->ddr.dma_base, rnr_dma_cfg->ddr.dma_buf_size, rnr_dma_cfg->ddr.dma_static_offset);
        /* each quad should go to a different UBUS slave in PSRAM */
#if defined(BCM6858)
        rc = rc ? rc : ag_drv_rnr_regs_cfg_psram_cfg_set(rnr_idx, (rnr_dma_cfg->psram.dma_base + (rnr_idx/4)), rnr_dma_cfg->psram.dma_buf_size, rnr_dma_cfg->psram.dma_static_offset);
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
    RDD_TRACE_EVENT_DTS *p = ((RDD_TRACE_EVENT_DTS *)RDD_RUNNER_PROFILING_TRACE_BUFFER_PTR(core_id));
    MEMSET(p, 0xFF, sizeof(RDD_RUNNER_PROFILING_TRACE_BUFFER_DTS));
}

void drv_rnr_profiling_core_init(uint8_t core_id)
{
    uint16_t trace_base_addr;
    uint16_t trace_buff_len;

    rnr_regs_trace_config core_cfg = {0};
    core_cfg.trace_disable_idle_in = 0;
    core_cfg.idle_counter_source_sel = RNR_PROFILING_IDLE_CYCLES_COUNTER;
    ag_drv_rnr_regs_trace_config_set(core_id, &core_cfg);

    /* we need trace base address in bytes, rdd has it in 64-bit regs */
    trace_base_addr = RDD_RUNNER_PROFILING_TRACE_BUFFER_ADDRESS_ARR[core_id] >> 3;
    trace_buff_len = (sizeof(RDD_RUNNER_PROFILING_TRACE_BUFFER_DTS)) >> 3;
    ag_drv_rnr_regs_cfg_profiling_cfg_0_set(core_id, trace_base_addr, (trace_base_addr + trace_buff_len - 1));
    drv_rnr_profiling_clear_trace(core_id);
}

void drv_rnr_quad_profiling_quad_init(rnr_quad_id_e quad_id)
{
    uint8_t core;
    rnr_quad_general_config_profiling_config quad_cfg = {0};

    quad_cfg.counter_lsb_sel = RNR_PROFILING_DEFAULT_COUNTER_LSB;
    ag_drv_rnr_quad_general_config_profiling_config_set(quad_id, &quad_cfg);
    for (core = quad_id * NUM_OF_CORES_IN_QUAD; core < (quad_id + 1) * NUM_OF_CORES_IN_QUAD; core++)
        drv_rnr_profiling_core_init(core);
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

void drv_rnr_set_sch_cfg(void)
{
#ifndef _CFE_REDUCED_XRDP_
    uint8_t i;

    /* for all processing cores use rr */
    for (i = 0; i < NUM_OF_RUNNER_CORES; ++i)
    {
        if (IS_PROCESSING_RUNNER_IMAGE(i))
            ag_drv_rnr_regs_cfg_sch_cfg_set(i, DRV_RNR_16RR);
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

bdmf_error_t drv_rnr_quad_parser_da_filter_valid_cfg(rnr_quad_id_e quad_id, uint8_t filter_index, uint8_t enable)
{
    bdmf_error_t rc = 0, rc1;
    rnr_quad_da_filter_valid da_filter;

    if (filter_index >= DRV_PARSER_DA_FILTER_NUM)
    {
        bdmf_trace("Invalid filter index %d\n", filter_index);
        return BDMF_ERR_PARM;
    }
    rc1 = ag_drv_rnr_quad_da_filter_valid_get(quad_id, &da_filter);
    if (rc1)
        bdmf_trace("Failed to get da_filter valid bit\n");
    rc |= rc1;

    if (rc)
        return rc; /*to protect memory access in case da_filter not set*/

    *((uint8_t *)(&da_filter) + filter_index) = enable;

    rc1 = ag_drv_rnr_quad_da_filter_valid_set(quad_id, &da_filter);
    if (rc1)
        bdmf_trace("Failed to set da_filter valid bit\n");
    rc |= rc1;

    rc1 = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(quad_id, &da_filter);
    if (rc1)
        bdmf_trace("Failed to get da_filter valid bit\n");
    rc |= rc1;

    *((uint8_t *)(&da_filter) + filter_index) = enable;

    rc1 = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(quad_id, &da_filter);
    if (rc1)
        bdmf_trace("Failed to set da_filter valid bit\n");
    rc |= rc1;

    rc1 = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(quad_id, &da_filter);
    if (rc1)
        bdmf_trace("Failed to get da_filter valid bit\n");
    rc |= rc1;

    *((uint8_t *)(&da_filter) + filter_index) = enable;

    rc1 = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(quad_id, &da_filter);
    if (rc1)
        bdmf_trace("Failed to set da_filter valid bit\n");
    rc |= rc1;
    return rc;
}

static bdmf_error_t _drv_rnr_quad_parser_da_filter_without_mask_set(rnr_quad_id_e quad_id,
    uint8_t filter_index, uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS])
{
    bdmf_error_t rc = BDMF_ERR_OK;
    rnr_quad_parser_da_filter mac_filter;

    /* no mask */
    mac_filter.da_filt_mask_msb = 0xffff;
    mac_filter.da_filt_mask_l = 0xffffffff;
    /* copy mac address */
    parser_mac_address_array_to_hw_format(mac_address, &mac_filter.da_filt_lsb, &mac_filter.da_filt_msb);

    /* We assumed here that DRV_PARSER_DA_FILTER_NUM=9 */
    switch (filter_index)
    {
    case 0:
        rc = ag_drv_rnr_quad_parser_da_filter_set(quad_id, &mac_filter);
        break;
    case 1:
        rc = ag_drv_rnr_quad_parser_da_filter1_set(quad_id, &mac_filter);
        break;
    case 2:
        rc = ag_drv_rnr_quad_parser_da_filter2_set(quad_id, mac_filter.da_filt_msb, mac_filter.da_filt_lsb);
        break;
    case 3:
        rc = ag_drv_rnr_quad_parser_da_filter3_set(quad_id, mac_filter.da_filt_msb, mac_filter.da_filt_lsb);
        break;
    case 4:
        rc = ag_drv_rnr_quad_parser_da_filter4_set(quad_id, mac_filter.da_filt_msb, mac_filter.da_filt_lsb);
        break;
    case 5:
        rc = ag_drv_rnr_quad_parser_da_filter5_set(quad_id, mac_filter.da_filt_msb, mac_filter.da_filt_lsb);
        break;
    case 6:
        rc = ag_drv_rnr_quad_parser_da_filter6_set(quad_id, mac_filter.da_filt_msb, mac_filter.da_filt_lsb);
        break;
    case 7:
        rc = ag_drv_rnr_quad_parser_da_filter7_set(quad_id, mac_filter.da_filt_msb, mac_filter.da_filt_lsb);
        break;
    case 8:
        rc = ag_drv_rnr_quad_parser_da_filter8_set(quad_id, mac_filter.da_filt_msb, mac_filter.da_filt_lsb);
        break;
    default:
    	bdmf_trace("switch Invalid filter index %d\n", filter_index);
    	rc = BDMF_ERR_PARM;
    }
    return rc;
}

typedef struct {
    uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS];
    uint16_t num_of_users;
} drv_rnr_lan_mac_data_t;

bdmf_error_t drv_rnr_quad_parser_da_filter_without_mask_set(uint8_t *mac_address, bdmf_boolean add)
{
    /* Masked MACs use indexes 0<->(DRV_PARSER_MASKED_DA_FILTER_NUM-1). 
       Unmasked MACs use indexes DRV_PARSER_MASKED_DA_FILTER_NUM<->(DRV_PARSER_DA_FILTER_NUM-1) */
    static drv_rnr_lan_mac_data_t lan_mac_list[DRV_PARSER_DA_FILTER_NUM] = {};
    uint8_t quad_id, filter_index, i;
    bdmf_error_t rc = BDMF_ERR_OK;
    rnr_quad_da_filter_valid da_filt_valid;
    
    if (add)
    {
        /* find available HW filter index. All RNR_QUADs have the same configuration therefore read from quad 0 */
        ag_drv_rnr_quad_da_filter_valid_get(0, &da_filt_valid);
    
        /* Find an existing (with same mac address) or new location in lan_mac list */
        filter_index = DRV_PARSER_DA_FILTER_NUM;
        for (i = DRV_PARSER_MASKED_DA_FILTER_NUM; i < DRV_PARSER_DA_FILTER_NUM; i++)
        {
            /* If table index is not used by any bridge, save it as a potential space */
            if (!(*(((bdmf_boolean *)(&da_filt_valid.da_filt0_valid)) + i)))
            {
                lan_mac_list[i].num_of_users = 0; /* Here we deal with reset. If HW marks invalid, reset */ 
                filter_index = i;
            }
            
            /* If new MAC address match an existing address, use its index */
            if (!memcmp(lan_mac_list[i].mac_address, mac_address, PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS))
            {
                filter_index = i;
                break;
            }
        }
        /* No place in list. Issue an error */
        if (filter_index == DRV_PARSER_DA_FILTER_NUM)
            return BDMF_ERR_NORES;
        
        /* Update driver and save MAC only if this is a new MAC */
        if (!lan_mac_list[filter_index].num_of_users)
        {
            for (quad_id = 0; !rc && quad_id < NUM_OF_RNR_QUAD; quad_id++)
            {
                rc = drv_rnr_quad_parser_da_filter_valid_cfg(quad_id, filter_index, 1);
                rc = rc ? rc : _drv_rnr_quad_parser_da_filter_without_mask_set(quad_id, filter_index, mac_address);
            }
            /* If anything has failed invalid relevant filters and return error */
            if (rc)
            {
                for (quad_id = 0; !rc && quad_id < NUM_OF_RNR_QUAD; quad_id++)
                    drv_rnr_quad_parser_da_filter_valid_cfg(quad_id, filter_index, 0);

                return rc;
            }
            memcpy(lan_mac_list[filter_index].mac_address, mac_address, PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS);
        }
        
        lan_mac_list[filter_index].num_of_users++;     
    }
    else
    {
        /* Find the relevant index in lan_mac list if */
        for (i = DRV_PARSER_MASKED_DA_FILTER_NUM; i < DRV_PARSER_DA_FILTER_NUM; i++)
        {
            /* If Bridge lan_mac address match an existing address, and there are bridges registered to it, delete it */
            if ((!memcmp(lan_mac_list[i].mac_address, mac_address, PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS)) 
                && lan_mac_list[i].num_of_users)
            {
                lan_mac_list[i].num_of_users--;
                /* If no more bridges are registered on this lan mac, delete from parser */
                if (lan_mac_list[i].num_of_users == 0)
                {
                    for (quad_id = 0; !rc && quad_id < NUM_OF_RNR_QUAD; quad_id++)
                    {
                        rc = drv_rnr_quad_parser_da_filter_valid_cfg(quad_id, i, 0);
                    }
                }
                break;
            }
        }
        if (i == DRV_PARSER_DA_FILTER_NUM)
        {
            return BDMF_ERR_NORES;
        }
    }

    return rc;
}

bdmf_error_t drv_rnr_quad_parser_configure_outer_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean outer_en, rdpa_tpid_detect_t etype)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t qtag_nest_0 = 0, qtag_nest_1 = 0;
    uint16_t hard_nest = 0;

    if (profile == DRV_PARSER_QTAG_PROFILE_0)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }

    if (etype == rdpa_tpid_detect_udef_1)
    {
        MS_SET_BIT_I(qtag_nest_0, DRV_PARSER_OUTER_QTAG_USER_OUTER_BIT, outer_en);
    }
    else if (etype == rdpa_tpid_detect_udef_2)
    {
        MS_SET_BIT_I(qtag_nest_1, DRV_PARSER_OUTER_QTAG_USER_OUTER_BIT, outer_en);
    }
    else if (etype == rdpa_tpid_detect_0x8100)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_8100_OUTER_BIT, outer_en);
    }
    else if (etype == rdpa_tpid_detect_0x88A8)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_88A8_OUTER_BIT, outer_en);
    }
    else if (etype == rdpa_tpid_detect_0x9100)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_9100_OUTER_BIT, outer_en);
    }
    else if (etype == rdpa_tpid_detect_0x9200)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_9200_OUTER_BIT, outer_en);
    }

    if (profile == DRV_PARSER_QTAG_PROFILE_0)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    return rc;
}

bdmf_error_t drv_rnr_quad_parser_configure_inner_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean inner_en, rdpa_tpid_detect_t etype)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t qtag_nest_0 = 0, qtag_nest_1 = 0;
    uint16_t hard_nest = 0;

    if (profile == DRV_PARSER_QTAG_PROFILE_0)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }

    if (etype == rdpa_tpid_detect_udef_1)
    {
        MS_SET_BIT_I(qtag_nest_0, DRV_PARSER_OUTER_QTAG_USER_INNER_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_udef_2)
    {
        MS_SET_BIT_I(qtag_nest_1, DRV_PARSER_OUTER_QTAG_USER_INNER_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_0x8100)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_8100_INNER_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_0x88A8)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_88A8_INNER_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_0x9100)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_9100_INNER_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_0x9200)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_9200_INNER_BIT, inner_en);
    }

    if (profile == DRV_PARSER_QTAG_PROFILE_0)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    return rc;
}

bdmf_error_t drv_rnr_quad_parser_configure_3rd_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean inner_en, rdpa_tpid_detect_t etype)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t qtag_nest_0 = 0, qtag_nest_1 = 0;
    uint16_t hard_nest = 0;

    if (profile == DRV_PARSER_QTAG_PROFILE_0)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(quad_id, &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_get(quad_id, &qtag_nest_0, &qtag_nest_1);
    }

    if (etype == rdpa_tpid_detect_udef_1)
    {
        MS_SET_BIT_I(qtag_nest_0, DRV_PARSER_OUTER_QTAG_USER_3RD_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_udef_2)
    {
        MS_SET_BIT_I(qtag_nest_1, DRV_PARSER_OUTER_QTAG_USER_3RD_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_0x8100)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_8100_3RD_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_0x88A8)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_88A8_3RD_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_0x9100)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_9100_3RD_BIT, inner_en);
    }
    else if (etype == rdpa_tpid_detect_0x9200)
    {
        MS_SET_BIT_I(hard_nest, DRV_PARSER_OUTER_QTAG_9200_3RD_BIT, inner_en);
    }

    if (profile == DRV_PARSER_QTAG_PROFILE_0)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(quad_id, hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_set(quad_id, qtag_nest_0, qtag_nest_1);
    }
    return rc;
}

/* converts mac address from array to the format used by HW */
void parser_mac_address_array_to_hw_format(uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS],
    uint32_t *address_4_ls_bytes, uint16_t *addres_2_ms_bytes)
{
    *address_4_ls_bytes = (mac_address[2] << 24) |
                          (mac_address[3] << 16) |
                          (mac_address[4] << 8) |
                          (mac_address[5]);

    *addres_2_ms_bytes =  (mac_address[0] << 8) |
                          (mac_address[1]);
}


/* converts mac address from the format used by HW to array */
void parser_mac_address_hw_format_to_array(uint32_t address_4_ls_bytes,
    uint16_t addres_2_ms_bytes, uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS])
{
    int byte_index;

    /* handle 4 LS bytes */
    for (byte_index = PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS - 1; byte_index > 1; --byte_index)
    {
        /* take the LS-byte only */
        mac_address[byte_index] = address_4_ls_bytes & 0xFF;
        /* one byte right shift */
        address_4_ls_bytes >>= 8;
    }

    /* handle 2 MS bytes */
    for (byte_index = 1; byte_index >= 0; --byte_index)
    {
        /* take the LS-byte only */
        mac_address[byte_index] = addres_2_ms_bytes & 0xFF;
        /* one byte right shift */
        addres_2_ms_bytes >>= 8;
    }
}
#if !defined(RDP_SIM)
/* use this function to map XRDP ubus to the coherency window *
 * although it is not described in the RDB, the window registers start at offset 0x10
 * each window is 3 words
 */
int  drv_rnr_quad_ubus_decode_wnd_cfg(uint32_t win, uint32_t phys_addr, uint32_t size_power_of_2, int port_id, uint32_t cache_bit_en)
{
    int quad;
    uint32_t reg_val;
    uint32_t reg_idx = (win * 3) + 0x4;

    if((win > 3) || (size_power_of_2 > 31) || (phys_addr & ((1<<size_power_of_2)-1)))
        return -1;

    for (quad = 0; quad < NUM_OF_RNR_QUAD; quad++)
    {
        reg_val = (phys_addr>>8);
        ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(quad, reg_idx, reg_val);

        /* write remap address */
        ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(quad, reg_idx + 1, reg_val);

        /*write attributes, if going to bridge, update cache */
         if( (port_id == UBUS_PORT_ID_BIU) && (cache_bit_en))
            reg_val = (DECODE_CFG_CACHE_BITS | DECODE_CFG_ENABLE_ADDR_ONLY | (size_power_of_2 << DECODE_CFG_SIZE_SHIFT) | port_id) ;
        else
            reg_val = (DECODE_CFG_ENABLE_ADDR_ONLY | (size_power_of_2 << DECODE_CFG_SIZE_SHIFT) | port_id) ;
        ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(quad, reg_idx + 2, reg_val);
    }

    return 0;
}

#endif

int drv_rnr_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val)
{

     rnr_quad_general_config_powersave_config rnr_ctrl;
     uint8_t block_id = 0;

     for (block_id = 0; block_id < RU_BLK(RNR_QUAD).addr_count; block_id++) {
         ag_drv_rnr_quad_general_config_powersave_config_get(block_id, &rnr_ctrl);
         rnr_ctrl.enable_powersave_core_0 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_1 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_2 = auto_gate ? 1 : 0;
#ifndef BCM6846
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

#ifdef USE_BDMF_SHELL

static bdmfmon_handle_t rnr_dir;

void drv_rnr_cli_config_trace(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rnr_regs_trace_config trace_cfg = {0};
    ag_drv_rnr_regs_trace_config_get(parm[0].value.unumber, &trace_cfg);
    trace_cfg.trace_disable_wakeup_log = parm[1].value.unumber;
    trace_cfg.trace_mode = parm[2].value.unumber;
    trace_cfg.trace_task = parm[3].value.unumber;
    ag_drv_rnr_regs_trace_config_set(parm[0].value.unumber, &trace_cfg);
}

static int drv_rnr_cli_get_core_stats(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmfmon_cmd_parm_t cmd_parm[2];
    bdmf_error_t rc;

    cmd_parm[1] = parm[0];
    cmd_parm[0].value.unumber = cli_rnr_regs_reset_trace_fifo;

    bdmf_session_print(session, "\nProfiling status:\n");
    rc = bcm_rnr_regs_cli_get(session, cmd_parm, 1);

    cmd_parm[0].value.unumber = cli_rnr_regs_cfg_pc_sts;
    bdmf_session_print(session, "\nCurrent PC:\n");
    rc = rc ? rc : bcm_rnr_regs_cli_get(session, cmd_parm, 1);
    return rc;
}

int drv_rnr_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t rnr_id = parm[0].value.unumber;

    static uint32_t cli_rnr_cfg[] = {cli_rnr_regs_rnr_freq, cli_rnr_regs_cfg_ddr_cfg, cli_rnr_regs_cfg_psram_cfg,
        cli_rnr_regs_cfg_sch_cfg};

    bdmf_session_print(session, "RNR %d configuration:\n\r", rnr_id);
    bdmf_session_print(session, "=====================\n\r");

    HAL_CLI_IDX_PRINT_LIST(session, rnr_regs, cli_rnr_cfg, rnr_id);

    return 0;
}

int drv_quad_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t quad_idx = parm[0].value.unumber;

    static uint32_t cli_quad_cfg[] = {cli_rnr_quad_general_config_psram0_base, cli_rnr_quad_general_config_psram1_base,
        cli_rnr_quad_general_config_psram2_base, cli_rnr_quad_general_config_psram3_base, cli_rnr_quad_general_config_ddr0_base,
        cli_rnr_quad_general_config_ddr1_base, cli_rnr_quad_general_config_psram0_mask, cli_rnr_quad_general_config_psram1_mask,
        cli_rnr_quad_general_config_psram2_mask, cli_rnr_quad_general_config_psram3_mask,cli_rnr_quad_general_config_ddr0_mask,
        cli_rnr_quad_general_config_ddr1_mask, cli_rnr_quad_general_config_powersave_config,
        cli_rnr_quad_general_config_powersave_status};

    bdmf_session_print(session, "Quad %d configuration:\n\r", quad_idx);
    bdmf_session_print(session, "======================\n\r");

    HAL_CLI_IDX_PRINT_LIST(session, rnr_quad, cli_quad_cfg, quad_idx);

    return 0;
}

int drv_parser_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t quad_idx = parm[0].value.unumber;
    static uint32_t cli_quad_parser_cfg[] = {cli_rnr_quad_parser_vid0, cli_rnr_quad_parser_vid1, cli_rnr_quad_parser_vid2,
        cli_rnr_quad_parser_vid3, cli_rnr_quad_parser_vid4, cli_rnr_quad_parser_vid5, cli_rnr_quad_parser_vid6,
        cli_rnr_quad_parser_vid7, cli_rnr_quad_parser_ip0, cli_rnr_quad_parser_ip1,
        cli_rnr_quad_exception_bits,cli_rnr_quad_tcp_flags, cli_rnr_quad_parser_ip_protocol0,
        cli_rnr_quad_parser_ip_protocol1, cli_rnr_quad_parser_ip_protocol2, cli_rnr_quad_parser_ip_protocol3,
        cli_rnr_quad_parser_da_filter, cli_rnr_quad_parser_da_filter1, cli_rnr_quad_parser_da_filter2,
        cli_rnr_quad_parser_da_filter3, cli_rnr_quad_parser_da_filter4, cli_rnr_quad_parser_da_filter5,
        cli_rnr_quad_parser_da_filter6, cli_rnr_quad_parser_da_filter7, cli_rnr_quad_parser_da_filter8,
        cli_rnr_quad_profile_us, cli_rnr_quad_parser_snap_conf, cli_rnr_quad_parser_ipv6_filter};

    bdmf_session_print(session, "Parser Quad %d configuration:\n\r", quad_idx);
    bdmf_session_print(session, "=============================\n\r");

    HAL_CLI_IDX_PRINT_LIST(session, rnr_quad, cli_quad_parser_cfg, quad_idx);

    return 0;
}

int drv_rnr_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t rnr_id;
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_boolean dma_illegal_status;

    for (rnr_id = 0; rnr_id < _num_of_cores; rnr_id++)
    {
        rc = ag_drv_rnr_regs_dma_illegal_get(rnr_id, &dma_illegal_status);
        if (!rc)
        {
            if (dma_illegal_status)
                bdmf_session_print(session, "rnr core : %d ; dma_illegal_status: %d\n\r", rnr_id, dma_illegal_status);
        }
    }
    return 0;
}

void drv_rnr_cli_init(bdmfmon_handle_t driver_dir)
{
    rnr_dir = ag_drv_rnr_regs_cli_init(driver_dir);
    ag_drv_rnr_quad_cli_init(rnr_dir);

    rdp_drv_bkpt_cli_init(rnr_dir);
    BDMFMON_MAKE_CMD(rnr_dir, "ct", "config tracer per core", (bdmfmon_cmd_cb_t)drv_rnr_cli_config_trace,
        BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("disable_wakeup_log", "do not log scheduler events", BDMFMON_PARM_UNUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("trace_mode", "0: all tasks, 1: single task mode", BDMFMON_PARM_UNUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE_DEFVAL("task_id", "task_id for single task mode", BDMFMON_PARM_UNUMBER, 0, 0, 15, 0),
        BDMFMON_PARM_LIST_TERMINATOR);

    BDMFMON_MAKE_CMD(rnr_dir, "sts", "get core profiling status and counters", (bdmfmon_cmd_cb_t)drv_rnr_cli_get_core_stats,
        BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
        BDMFMON_PARM_LIST_TERMINATOR);

    BDMFMON_MAKE_CMD(rnr_dir, "grc", "get rnr configuration", (bdmfmon_cmd_cb_t)drv_rnr_cli_config_get,
        BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0));
    BDMFMON_MAKE_CMD(rnr_dir, "gqc", "get quad configuration", drv_quad_cli_config_get,
        BDMFMON_MAKE_PARM_ENUM("quad_id", "quad_id", quad_idx_enum_table, 0));
    BDMFMON_MAKE_CMD(rnr_dir, "gpg", "get parser configuration", drv_parser_cli_config_get,
        BDMFMON_MAKE_PARM_ENUM("quad_id", "quad_id", quad_idx_enum_table, 0));
    BDMFMON_MAKE_CMD_NOPARM(rnr_dir, "cs", "check sanity registers", drv_rnr_cli_sanity_get);
}

void drv_rnr_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (rnr_dir)
    {
        bdmfmon_token_destroy(rnr_dir);
        rnr_dir = NULL;
    }
}

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/
/*
    cfg_get
    ag:
        pf_cfg_get (0_31 & group0_1)
        error_pm_counters
        pm_counters
*/

#endif /* USE_BDMF_SHELL */

void drv_rnr_num_of_cores_set(int num_of_cores)
{
    _num_of_cores = num_of_cores;
}

int xrdp_rnr_profiling_set_config(struct xrdp_rnr_profiling_cfg *cfg)
{
    int quad, core, rc = BDMF_ERR_OK;
    rnr_quad_general_config_profiling_config quad_cfg = {};

    /* reset profiling by passing profiling_start = 0 and counter_enable = 0 to vpb_bridge */
    rc = rc ? rc : ag_drv_ubus_slv_profiling_cycle_num_set(cfg->num_cycles);
    rc = rc ? rc : ag_drv_ubus_slv_profiling_cfg_set(0, 0, 0, 0);

    for (quad = 0; (quad < NUM_OF_RNR_QUAD) && (!rc); quad++)
    {
        rc = ag_drv_rnr_quad_general_config_profiling_config_get(quad, &quad_cfg);
        if (rc)
            return rc;
        for (core = quad * NUM_OF_CORES_IN_QUAD; core < (quad + 1) * NUM_OF_CORES_IN_QUAD; core++)
        {
            /* clear trace-fifo-overrun bit and clear trace fifo in general - workaround */
            rc = rc ? rc : ag_drv_rnr_regs_reset_trace_fifo_set(core, 1);
            rc = rc ? rc : ag_drv_rnr_regs_clear_trace_fifo_overrun_set(core, 1);
            rc = rc ? rc : ag_drv_rnr_regs_reset_trace_fifo_set(core, 0);
            rc = rc ? rc : ag_drv_rnr_regs_clear_trace_fifo_overrun_set(core, 0);

            /* clear trace buffer contents (fill trace buffer with 0xFF) */
            drv_rnr_profiling_clear_trace(core);
        }
            /* enable/disable trace collection for this core according to quad parameter */
#if defined(BCM6858)
        quad_cfg.enable_trace_core_0 = cfg->enable_quads[quad];
        quad_cfg.enable_trace_core_1 = cfg->enable_quads[quad];
        quad_cfg.enable_trace_core_2 = cfg->enable_quads[quad];
        quad_cfg.enable_trace_core_3 = cfg->enable_quads[quad];
#else
        quad_cfg.enable_trace_core_0 = 1;
        quad_cfg.enable_trace_core_1 = 1;
        quad_cfg.enable_trace_core_2 = 1;
        quad_cfg.enable_trace_core_3 = 1;
        quad_cfg.enable_trace_core_4 = 1;
        quad_cfg.enable_trace_core_5 = 1;
#endif
        rc = ag_drv_rnr_quad_general_config_profiling_config_set(quad, &quad_cfg);
    }

    rc = rc ? rc : ag_drv_ubus_slv_profiling_cycle_num_set(cfg->num_cycles);
    rc = rc ? rc : ag_drv_ubus_slv_profiling_cfg_set(1, 1, 0, 0);
    return rc;
}
EXPORT_SYMBOL(xrdp_rnr_profiling_set_config);

int xrdp_rnr_profiling_get_result(struct xrdp_rnr_profiling_res *res)
{
    int core, rc = BDMF_ERR_OK;
    bdmf_boolean profiling_on = 0;
#ifndef BCM6858
    rnr_regs_rnr_core_cntrs cntrs;
#endif
    uint32_t num_cycles;

    /* verify that profiling is not running right now */
#ifdef RDP_SIM
    num_cycles = 0xFFFFFFFF;
#else
    rc = ag_drv_ubus_slv_profiling_status_get(&profiling_on, &num_cycles);
#endif
    if (rc) return rc;
    res->profiling_on = profiling_on;
    if (!profiling_on && num_cycles)
    {
        res->total_cnt = (uint32_t)(num_cycles * RNR_FREQ_IN_MHZ / UBUS_SLV_FREQ_IN_MHZ);
        for (core = 0; core < NUM_OF_RUNNER_CORES; core += (NUM_OF_RUNNER_CORES / NUM_OF_RNR_WITH_PROFILING))
        {
#ifndef BCM6858
            rc = rc ? rc : ag_drv_rnr_regs_rnr_core_cntrs_get(core, &cntrs);
            res->idle_cnts[core] = cntrs.idle_cnt; /* idle counter frequency is twice the frequency of the vpb-bridge counter */
#else
            ag_drv_rnr_regs_cfg_idle_cnt1_get(core, &res->idle_cnts[core]);
#endif
        }
    }

    return rc;
}
EXPORT_SYMBOL(xrdp_rnr_profiling_get_result);
