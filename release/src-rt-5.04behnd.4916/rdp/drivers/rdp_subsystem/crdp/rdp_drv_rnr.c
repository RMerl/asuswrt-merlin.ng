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
#include "data_path_init.h"
#include "rdp_platform.h"

#include "rdp_drv_bkpt.h"

#include "rdd_data_structures_auto.h"
#if !defined(RDP_SIM)
#include "bcm_ubus4.h"
#endif
#include "rdd_defs.h"
#include "xrdp_autogens.h"
#include "rdp_drv_parser.h"

extern uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx);
extern uint8_t valid_rnr_core[NUM_OF_TM_RUNNERS + NUM_OF_PROCESSING_RUNNERS];

extern int runner_core_size[NUM_OF_TM_RUNNERS + NUM_OF_PROCESSING_RUNNERS];

#define RNR_NOP_OPCODE  0xFC000000

static int _num_of_cores = NUM_OF_TM_RUNNERS + NUM_OF_PROCESSING_RUNNERS;

/* keep power save status when run profile */
bdmf_boolean g_old_auto_gate ;

uintptr_t rdp_runner_core_addr[NUM_OF_TM_RUNNERS + NUM_OF_PROCESSING_RUNNERS];

uint16_t drv_rnr_dynamic_code_start_address_get(void)
{
    return image_0_dyn_code_func;
}

uint16_t drv_rnr_dynamic_code_end_address_get(void)
{
    return 0x1FFF; /* last address, dynamic code is at the end*/
}

#if 0 /* TODO - in next phase */
static bdmf_error_t _rdp_rnr_parser_qtag_cfg(drv_parser_qtag_profile_t profile, ag_rdp_tpid_index tpid_type, uint8_t position, bdmf_boolean enable)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t qtag_nest_0 = 0, qtag_nest_1 = 0, rnr_cmn_index;
    uint16_t hard_nest = 0;

    for (rnr_cmn_index = 0; rnr_cmn_index < RU_BLK(CRDP_RCG_RNR_CMN).addr_count; rnr_cmn_index++)
    {
        rc = qtag_hard_nest_get_func[profile](rnr_cmn_index, (void *) &hard_nest);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed qtag_hard_nest_get_func. Err: %d\n", rc);  

        rc = qtag_nest_get_func[profile](rnr_cmn_index, &qtag_nest_0, &qtag_nest_1);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed qtag_nest_get_func. Err: %d\n", rc);  

        if (tpid_type == ag_RDP_TPID_INDEX_USER_1)
        {
            MS_SET_BIT_I(qtag_nest_0, tpid_position_bit[position][ag_RDP_TPID_INDEX_USER_1], enable);
        }
        else if (tpid_type == ag_RDP_TPID_INDEX_USER_2)
        {
            MS_SET_BIT_I(qtag_nest_1, tpid_position_bit[position][ag_RDP_TPID_INDEX_USER_2], enable);
        }
        else 
        {
            MS_SET_BIT_I(hard_nest, tpid_position_bit[position][tpid_type], enable);
        }

        rc = qtag_hard_nest_set_func[profile](rnr_cmn_index, (crdp_rcg_rnr_cmn_parser_qtag_hard_nest_prof0 *) &hard_nest);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed qtag_hard_nest_set_func. Err: %d\n", rc);  

        rc = qtag_nest_set_func[profile](rnr_cmn_index, qtag_nest_0, qtag_nest_1);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed qtag_nest_set_func. Err: %d\n", rc);  
    }
    return rc;
}

bdmf_error_t rdp_rnr_parser_tpid_set(ag_rdp_tpid_index tpid_type, ag_tpid_enable tpid_param, uint16_t tpid_user_def_1, uint16_t tpid_user_def_2)
{   
    bdmf_error_t rc = BDMF_ERR_OK;
    uint16_t profile_id, rnr_cmn_idx;
    uint16_t ethertype_0[NUM_OF_PROCESSING_RNR_CMN];
    uint16_t ethertype_1[NUM_OF_PROCESSING_RNR_CMN];

    if ( (tpid_type == ag_RDP_TPID_INDEX_USER_1) || (tpid_type == ag_RDP_TPID_INDEX_USER_2))
    {
        for (rnr_cmn_idx = 0; !rc && rnr_cmn_idx < NUM_OF_PROCESSING_RNR_CMN; rnr_cmn_idx++)
            rc = ag_drv_crdp_rcg_rnr_cmn_parser_core_configuration_qtag_ethtype_get(rnr_cmn_idx, &ethertype_0[rnr_cmn_idx], &ethertype_1[rnr_cmn_idx]);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed to get user-defined TPIDs Detect values. Err: %d\n", rc);  

        if (tpid_type == ag_RDP_TPID_INDEX_USER_1)
        {
            for (rnr_cmn_idx = 0; rnr_cmn_idx < NUM_OF_PROCESSING_RNR_CMN; rnr_cmn_idx++)
                ethertype_0[rnr_cmn_idx] = tpid_user_def_1;
        }
        else
        {
            for (rnr_cmn_idx = 0; rnr_cmn_idx < NUM_OF_PROCESSING_RNR_CMN; rnr_cmn_idx++)
                ethertype_1[rnr_cmn_idx] = tpid_user_def_2;
        }
        for (rnr_cmn_idx = 0; !rc && rnr_cmn_idx < NUM_OF_PROCESSING_RNR_CMN; rnr_cmn_idx++)
            rc = ag_drv_crdp_rcg_rnr_cmn_parser_core_configuration_qtag_ethtype_set(rnr_cmn_idx, ethertype_0[rnr_cmn_idx], ethertype_1[rnr_cmn_idx]);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed to set user-defined TPIDs Detect values. Err: %d\n", rc);  
    }

    for ( profile_id = 0; profile_id < MAX_NUM_OF_PROFILES; profile_id++)
    {
        rc = _rdp_rnr_parser_qtag_cfg(profile_id, tpid_type, DRV_PARSER_TPID_OUTER, tpid_param.otag_en);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed _rdp_rnr_parser_qtag_cfg otag_en. Err: %d\n", rc);

        rc = _rdp_rnr_parser_qtag_cfg(profile_id, tpid_type, DRV_PARSER_TPID_INNER, tpid_param.itag_en);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed _rdp_rnr_parser_qtag_cfg itag_en. Err: %d\n", rc);  

        rc = _rdp_rnr_parser_qtag_cfg(profile_id, tpid_type, DRV_PARSER_TPID_THIRD, tpid_param.third_tag_en);
        BDMF_TRACE_RET_IF_COND(rc , rc, "Failed _rdp_rnr_parser_qtag_cfg thirdtag_en. Err: %d\n", rc);  
    }
    return rc;
}
#endif

void memcpyl_prediction(void *__to, void *__from, unsigned int __n)
{
   uint8_t *src = (uint8_t *)__from;
    uint8_t *dst = (uint8_t *)__to;

    int i;
    for (i = 0; i < (__n / 2); i++, src += 2, dst += 4)
    {
        MWRITE_32(dst, (unsigned int)(*(volatile unsigned short *)src));
    }
}

void rdp_rnr_write_context(void *__to, void *__from, unsigned int __n)
{
    uint8_t *src = (uint8_t *)__from;
    uint8_t *dst = (uint8_t *)__to;
    int i, n = __n / 4;

    for (i = 0; i < n; i++, src += 4, dst += 4)
    {
#if  !defined (RDP_SIM)
        if (*(volatile unsigned int *)src != 0)
#endif
            MWRITE_32(dst, *(volatile unsigned int *)src);
    }
}

int drv_rnr_get_inst_word_size(int core)
{
    return runner_core_size[core];
}

extern bdmf_phys_addr_t ddr_token_info_phy_addr;
extern FPM_GLOBAL_CFG_STRUCT g_fpm_hw_cfg;
int drv_rnr_dma_cfg(dpi_params_t *dpi_params)
{
    uint8_t rnr_idx, rnr_cmn_index;
    uint32_t addr_hi, addr_lo;
    bdmf_phys_addr_t fpm_base_phys_addr;
    rnr_dma_regs_cfg_t rnr_dma_cfg = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_phys_addr_t psram_dma_base_phys_addr;

    /* Global FPM configuration, part of RNR DMA configuration should be invoked after fpm_init */
#if CHIP_VER >= RDP_GEN_60
#if defined(BCM_DSL_XRDP) || defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, ddr_token_info_phy_addr);
    g_fpm_hw_cfg.ddr_token_info_low = addr_lo;
    g_fpm_hw_cfg.ddr_token_info_high = addr_hi;
#endif
#endif

    fpm_base_phys_addr = dpi_params->rdp_ddr_pkt_base_phys;
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);

    g_fpm_hw_cfg.fpm_base_low = addr_lo;
    g_fpm_hw_cfg.fpm_base_high = addr_hi;

    rnr_dma_cfg.ddr.dma_base = (addr_hi << 12) | (addr_lo >> 20);

    lookup_dma_bufsz_by_fpm_bufsz(&dpi_params->fpm_buf_size, &rnr_dma_cfg.ddr.dma_buf_size);

    g_fpm_hw_cfg.fpm_token_size = dpi_params->fpm_buf_size;

    /* RNR DMA configuration */
    rnr_dma_cfg.ddr.dma_static_offset = 0;
    psram_dma_base_phys_addr = xrdp_virt2phys(&RU_BLK(PSRAM_MEM), 1) + RU_REG_OFFSET(PSRAM_MEM, IPSRAM_MEMORY_DATA);
    rnr_dma_cfg.psram.dma_base = psram_dma_base_phys_addr >> 20;
    rnr_dma_cfg.psram.dma_buf_size = DMA_BUFSIZE_128;
    rnr_dma_cfg.psram.dma_static_offset = 0;

    for (rnr_idx = 0; rnr_idx < RU_BLK(ALL_RNR_REGS).addr_count; rnr_idx++)
    {
        rc = ag_drv_all_rnr_regs_cfg_ddr_cfg_set(rnr_idx, rnr_dma_cfg.ddr.dma_base, rnr_dma_cfg.ddr.dma_buf_size, 0, rnr_dma_cfg.ddr.dma_static_offset);
        rc = rc ? rc : ag_drv_all_rnr_regs_cfg_psram_cfg_set(rnr_idx, rnr_dma_cfg.psram.dma_buf_size, 0, rnr_dma_cfg.psram.dma_static_offset);
    }

    for (rnr_cmn_index = 0; rnr_cmn_index < RU_BLK(CRDP_RCG_RNR_CMN).addr_count; rnr_cmn_index++)
    {
        ag_drv_all_rnr_cmn_general_config_dma_psram_base_0_set(rnr_cmn_index, RU_BLK(PSRAM_MEM).addr[0] >> 20);
        ag_drv_all_rnr_cmn_general_config_dma_psram_base_1_set(rnr_cmn_index, RU_BLK(PSRAM_MEM).addr[1] >> 20);
        ag_drv_all_rnr_cmn_general_config_dma_psram_base_2_set(rnr_cmn_index, RU_BLK(PSRAM_MEM).addr[2] >> 20);

    }
    return rc;
}

void drv_rnr_configure_scheduler(void)
{
    uint8_t rnr_idx;

    for (rnr_idx = 0; rnr_idx < RU_BLK(CRDP_RCG_RNR_REGS).addr_count; rnr_idx++)
    {
        ag_drv_crdp_rcg_rnr_regs_cfg_sch_cfg_set(rnr_idx, DRV_RNR_8SP_8RR);
    }
}

bdmf_error_t drv_rnr_bkpt_init(void)
{
    uint16_t handlr_addr, pc_val;
    bdmf_error_t rc = BDMF_ERR_OK;
    int rnr_cmn_idx;

    for (rnr_cmn_idx = 0; !rc && rnr_cmn_idx < ALL_RNR_CMN_BLOCK.addr_count; rnr_cmn_idx++)
    {
        rc = ag_drv_all_rnr_cmn_general_config_bkpt_gen_cfg_get(rnr_cmn_idx, &handlr_addr, &pc_val);

#ifndef PC_ADDRESS_INST_IND
        handlr_addr = image_0_debug_routine_external >> 2;
#else
        handlr_addr = image_0_debug_routine_external;
#endif
        rc = rc ? rc : ag_drv_all_rnr_cmn_general_config_bkpt_gen_cfg_set(rnr_cmn_idx, handlr_addr, pc_val);
    }
    return rc;
}


bdmf_error_t drv_rnr_power_save_init(void)
{
    uint8_t rnr_cmn_index;
    bdmf_error_t rc = 0;

    all_rnr_cmn_general_config_powersave_config general_config_powersave_config = {};
    for (rnr_cmn_index= 0; rnr_cmn_index < ALL_RNR_CMN_BLOCK.addr_count; rnr_cmn_index++)
    {
        rc = ag_drv_all_rnr_cmn_general_config_powersave_config_get(rnr_cmn_index, &general_config_powersave_config);
        general_config_powersave_config.time_counter = 2;
        general_config_powersave_config.enable_powersave_core_0 = 1;
        general_config_powersave_config.enable_powersave_core_1 = 1;
        general_config_powersave_config.enable_powersave_core_2 = 1;
        general_config_powersave_config.enable_powersave_core_3 = 1;
        general_config_powersave_config.enable_powersave_core_4 = 1;
        general_config_powersave_config.enable_powersave_core_5 = 1;
        general_config_powersave_config.enable_powersave_core_6 = 1;
        general_config_powersave_config.enable_powersave_core_7 = 1;
        general_config_powersave_config.enable_powersave_core_8 = 1;
        general_config_powersave_config.enable_powersave_core_9 = 1;
        general_config_powersave_config.enable_powersave_core_10 = 1;
        general_config_powersave_config.enable_powersave_core_11 = 1;
        general_config_powersave_config.enable_powersave_core_12 = 1;
        general_config_powersave_config.enable_powersave_core_13 = 1;
        general_config_powersave_config.enable_powersave_core_14 = 1;
        general_config_powersave_config.enable_powersave_core_15 = 1;
        general_config_powersave_config.enable_cpu_if_clk_gating = 0;
        general_config_powersave_config.enable_common_reg_clk_gating = 0;
        general_config_powersave_config.enable_ec_blocks_clk_gating = 0;
        rc = rc ? rc: ag_drv_all_rnr_cmn_general_config_powersave_config_set(rnr_cmn_index, &general_config_powersave_config);
        BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: drv_rnr_power_save_init. rnr_cmn_index: %d\n", rnr_cmn_index);
    }
    return BDMF_ERR_OK;
}

/* Data path init helpers */
int drv_rnr_frequency_set(uint16_t freq)
{
    uint8_t rnr_idx;
    int rc = BDMF_ERR_OK;

    for (rnr_idx = 0; rnr_idx < RU_BLK(ALL_RNR_REGS).addr_count; rnr_idx++)
    {
        rc = ag_drv_all_rnr_regs_rnr_freq_set(rnr_idx, (freq-1));
        if (rc)
            return rc;
    }
    return BDMF_ERR_OK;
}

typedef struct
{
    all_rnr_cmn_parser_core_configuration_key_cfg       parser_key_cfg;
    all_rnr_cmn_parser_core_configuration_qtag_nest     parser_core_configuration_qtag_nest;
    uint8_t                                             tcp_flags_filt;
    uint32_t                                            exception_en;
    uint16_t                                            hard_nest_profile;
    uint32_t                                            code;
    bdmf_boolean                                        en_rfc1042;
    bdmf_boolean                                        en_8021q;
    bdmf_boolean                                        disable_l2tp_source_port_check;
}parser_cfg_t;

static int _drv_parser_init(parser_cfg_t *cfg)
{
    int rc = BDMF_ERR_OK;
    uint8_t i;
    uint32_t key_build_cfg_addr;
    all_rnr_cmn_parser_snap_conf parser_snap_conf;

    for (i = 0; !rc && i < RU_BLK(CRDP_RCG_RNR_CMN).addr_count; i++)
    {
        /* parser configuration */
        rc = ag_drv_all_rnr_cmn_parser_core_configuration_parser_misc_cfg_set(i, cfg->exception_en, 0, cfg->disable_l2tp_source_port_check, cfg->tcp_flags_filt);

        rc = rc ? rc : ag_drv_all_rnr_cmn_parser_core_configuration_ppp_ip_prot_code_set(i, PARSER_PPP_PROTOCOL_CODE_0_IPV4, PARSER_PPP_PROTOCOL_CODE_1_IPV6);
        rc = rc ? rc : ag_drv_all_rnr_cmn_parser_core_configuration_eng_set(i, PARSER_ENG_CFG_DEFAULT);
        rc = rc ? rc : ag_drv_all_rnr_cmn_parser_core_configuration_dos_attack_set(i, PARSER_DOS_REASON_MASK_BITS);
        rc = rc ? rc : ag_drv_all_rnr_cmn_parser_core_configuration_icmp_max_size_set(i, PARSER_MAX_ICMPV4_SIZE, PARSER_MAX_ICMPV6_SIZE);
        /* support VXLAN*/
        rc = rc ? rc : ag_drv_all_rnr_cmn_parser_core_configuration_vxlan_cfg_set(i, 0, 1);

        /* parse key default configuration */
        rc = rc ? rc : ag_drv_all_rnr_cmn_parser_core_configuration_key_cfg_set(i, &cfg->parser_key_cfg);

        /* key build cfg : dummy at this stage. */
        key_build_cfg_addr = RDD_PARSER_KEY_BUILD_TABLE_ADDRESS_ARR[i*16];
        if (key_build_cfg_addr != INVALID_TABLE_ADDRESS)
        {
            ag_drv_all_rnr_cmn_parser_core_configuration_key_build_cfg_0_set(i, key_build_cfg_addr, sizeof(PARSER_KEY_BUILD_ENTRY_STRUCT)/sizeof(uint64_t));
            ag_drv_all_rnr_cmn_parser_core_configuration_key_build_cfg_1_set(i, key_build_cfg_addr + sizeof(PARSER_KEY_BUILD_ENTRY_STRUCT), sizeof(PARSER_KEY_BUILD_ENTRY_STRUCT)/sizeof(uint64_t));
            ag_drv_all_rnr_cmn_parser_core_configuration_key_build_cfg_2_set(i, key_build_cfg_addr + 2*sizeof(PARSER_KEY_BUILD_ENTRY_STRUCT), sizeof(PARSER_KEY_BUILD_ENTRY_STRUCT)/sizeof(uint64_t));
            ag_drv_all_rnr_cmn_parser_core_configuration_key_build_cfg_3_set(i, key_build_cfg_addr + 3*sizeof(PARSER_KEY_BUILD_ENTRY_STRUCT), sizeof(PARSER_KEY_BUILD_ENTRY_STRUCT)/sizeof(uint64_t));
        }

        rdp_drv_parser_cfg_key_build_table(0);

        rc = rc ? rc : ag_drv_all_rnr_cmn_parser_core_configuration_qtag_nest_set(i, &cfg->parser_core_configuration_qtag_nest);

        ag_drv_all_rnr_cmn_parser_core_configuration_qtag_hard_nest_0_set(i, cfg->hard_nest_profile);
        ag_drv_all_rnr_cmn_parser_core_configuration_qtag_hard_nest_1_set(i, cfg->hard_nest_profile);
        ag_drv_all_rnr_cmn_parser_core_configuration_qtag_hard_nest_2_set(i, cfg->hard_nest_profile);

        /* LLC/SNAP configuration */
        parser_snap_conf.code = cfg->code;
        parser_snap_conf.en_rfc1042 = cfg->en_rfc1042;
        parser_snap_conf.en_8021q = cfg->en_8021q;
        rc = rc ? rc : ag_drv_all_rnr_cmn_parser_snap_conf_set(i, &parser_snap_conf);
    }

    if (rc)
      	BDMF_TRACE_ERR("Failed to initialize parser driver\n");
        
    return rc;
}

int drv_parser_init(void)
{
    parser_cfg_t cfg;
    cfg.disable_l2tp_source_port_check = 0;     // TBD
    cfg.tcp_flags_filt = PARSER_TCP_CTL_FLAGS;
    cfg.exception_en = PARSER_EXCP_STATUS_BITS;

    /* vlan configuration, turn on all 3 tpid */
    cfg.parser_core_configuration_qtag_nest.qtag_nest_0_profile_0 = 0x7;
    cfg.parser_core_configuration_qtag_nest.qtag_nest_0_profile_1 = 0x7;
    cfg.parser_core_configuration_qtag_nest.qtag_nest_0_profile_2 = 0x7;
    cfg.parser_core_configuration_qtag_nest.qtag_nest_1_profile_0 = 0x7;
    cfg.parser_core_configuration_qtag_nest.qtag_nest_1_profile_1 = 0x7;
    cfg.parser_core_configuration_qtag_nest.qtag_nest_1_profile_2 = 0x7;

    cfg.parser_core_configuration_qtag_nest.max_num_of_vlans = 0x7;

    /* Hard nest profile:
        bit 2-0:  Enable 8100 as VLAN for outer, 2nd, and inner VLANs (inner is bit 2). 
        bit 5-3:  Enable 88a8 as VLAN for outer, 2nd, and inner VLANs. 
        bit 8-6:  Enable 9100 as VLAN for outer, 2nd, and inner VLANs. 
        bit 11-9: Enable 9200 as VLAN for outer, 2nd, and inner VLANs.
    */
    cfg.hard_nest_profile = 0xfff;

    /* LLC/SNAP configuration */
    cfg.code = 0;
    cfg.en_rfc1042 = 1;
    cfg.en_8021q = 1;

    cfg.parser_key_cfg.l2_exclude_smac = 0;
    cfg.parser_key_cfg.tcp_pure_ack_mask = 0; 
    cfg.parser_key_cfg.incude_dei_in_vlans_crc = 0; 
    cfg.parser_key_cfg.key_size = 0; 
    cfg.parser_key_cfg.max_num_of_vlans_in_crc = 0; 
    cfg.parser_key_cfg.l3_tcp_pure_ack_mask = 0; 

    return _drv_parser_init(&cfg);
}

void drv_rnr_cores_addr_init(const ru_block_rec *RNR_BLOCK, uint32_t ru_get_offset_rnr_mem)
{
    uint32_t rnr_idx;

    for (rnr_idx = 0; rnr_idx < RNR_BLOCK->addr_count; rnr_idx++)
    {
        rdp_runner_core_addr[rnr_idx] = RNR_BLOCK->addr[rnr_idx] + ru_get_offset_rnr_mem;
        ACCESS_LOG(ACCESS_LOG_OP_SET_CORE_ADDR, rdp_runner_core_addr[rnr_idx], rnr_idx, 0);
    }
}

inline void drv_rnr_profiling_clear_trace(uint8_t core_id)
{
    TRACE_EVENT_STRUCT *p = ((TRACE_EVENT_STRUCT *)RDD_RUNNER_PROFILING_TRACE_BUFFER_PTR(core_id));
    MEMSET(p, 0xFF, sizeof(RUNNER_PROFILING_TRACE_BUFFER_STRUCT));
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
    trace_buff_len = (sizeof(RUNNER_PROFILING_TRACE_BUFFER_STRUCT)) >> 3;
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
    mem_byte_num = 2 * (RU_REG_RAM_CNT(ALL_RNR_MEM, MEM_ENTRY) + 1) * sizeof(uint32_t);
    mem_cntxt_byte_num = (RU_REG_RAM_CNT(ALL_RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);
    for (rnr_idx = 0; rnr_idx < _num_of_cores; rnr_idx++)
    {
        /* reset SRAM memory */
        MEMSET((uint32_t *)DEVICE_ADDRESS(RU_BLK(ALL_RNR_MEM).addr[rnr_idx] + RU_REG_OFFSET(ALL_RNR_MEM, MEM_ENTRY)), 0, mem_byte_num);
        /* reset SRAM context memory */
        MEMSET((uint32_t *)DEVICE_ADDRESS(RU_BLK(ALL_RNR_CNTXT).addr[rnr_idx] + RU_REG_OFFSET(ALL_RNR_CNTXT, MEM_ENTRY)), 0, mem_cntxt_byte_num);
    }
}

#if ((CHIP_VER >= RDP_GEN_62) &&  (!defined (RDP_SIM)))
static int __drv_rnr_can_skip_instructions_load(uint32_t *cores_done_mask, uint32_t *slaves_for_this_core_mask,
    uint8_t rnr_idx)
{
    uint32_t _cores_done_mask = *cores_done_mask;
    uint32_t _slaves_for_this_core_mask = *slaves_for_this_core_mask;
    int i;

    /* per core check if need to do to other cores as well */
    if (_cores_done_mask & (1 << rnr_idx))
        return 1;

    _cores_done_mask += (1 << rnr_idx);
    _slaves_for_this_core_mask = 0;

    for (i = 0; i < _num_of_cores; i++)
    {
        if (((_cores_done_mask & (1 << i)) == 0) && (rdp_core_to_image_map[rnr_idx] == rdp_core_to_image_map[i]))
        {
            _slaves_for_this_core_mask += (1<<i);
        }
    }
    _cores_done_mask += _slaves_for_this_core_mask;

    if (_slaves_for_this_core_mask != 0)
    {
        ag_drv_rnr_quad_general_config_multi_psel_cfg_set(0, 1 << rnr_idx, _slaves_for_this_core_mask + (1 << rnr_idx));
    }

    *cores_done_mask = _cores_done_mask;
    *slaves_for_this_core_mask = _slaves_for_this_core_mask;

    return 0;
}
#endif

#if defined(RDP_SIM)

void drv_rnr_load_instructions(uint8_t rnr_idx, uint32_t first_instuct_idx, uint32_t num_of_instructs,
    uint32_t *instructs_arr)
{
    MWRITE_BLK_32_SWAP(
        DEVICE_ADDRESS(RU_BLK(RNR_INST).addr[rnr_idx] + RU_REG_OFFSET(RNR_INST, MEM_ENTRY)) +
            first_instuct_idx * sizeof(uint32_t),
        (uint8_t *)instructs_arr,
        num_of_instructs * sizeof(uint32_t));
}

#else /* !defined(RDP_SIM) */

void drv_rnr_load_instructions(uint8_t rnr_idx, uint32_t first_instuct_idx, uint32_t num_of_instructs,
    uint32_t *instructs_arr)
{
    uint32_t inst;

#if defined(CONFIG_BRCM_IKOS) || defined(CONFIG_BRCM_QEMU)
    uint8_t nops = 0;
#endif

#if defined(PHYS_ADDR_64BIT) && !defined(CONFIG_GPL_RDP_GEN)
#if defined(CONFIG_BRCM_QEMU)
    const uint64_t nop_inst = 0xFC000000FC000000ULL;
#endif
    for (inst = 0; inst < num_of_instructs; inst += 2)
    {
        uint64_t instruct = *(uint64_t*)&instructs_arr[inst];
        MWRITE_I_64(DEVICE_ADDRESS(RU_BLK(RNR_INST).addr[rnr_idx]), ((first_instuct_idx + inst) >> 1),
            instruct);

#if defined(CONFIG_BRCM_QEMU)
        if (instruct == nop_inst)
        {
            nops += 2;
            if (nops >= 26)
            {
                do  /* fast forward to first non-nop instruction */
                {
                    inst += 2;
                    if (inst >= num_of_instructs)
                        break;
                    instruct = *(uint64_t*)&instructs_arr[inst];
                }
                while (instruct == nop_inst);

                if (inst < num_of_instructs)
                    inst -= 2; /* reincremented in for loop */
            }
        }
        else
        {
            nops = 0;
        }
#endif
    }

#else /* !(defined(PHYS_ADDR_64BIT) && !defined(CONFIG_GPL_RDP_GEN)) */
#if defined(CONFIG_BRCM_IKOS)
    const uint32_t nop_inst = 0xFC000000;
#endif

#ifndef CONFIG_BRCM_QEMU
    if (first_instuct_idx == 0)
    {
        /* Pre-init instruction memory with NOPs */
        MEMSET_32(DEVICE_ADDRESS(RU_BLK(RNR_INST).addr[rnr_idx]), RNR_NOP_OPCODE, num_of_instructs);
    }
#endif

    for (inst = 0; inst < num_of_instructs; inst ++)
    {
        MWRITE_I_32(DEVICE_ADDRESS(RU_BLK(RNR_INST).addr[rnr_idx]), (first_instuct_idx + inst),
            instructs_arr[inst]);
#ifdef CONFIG_BRCM_IKOS
        if (instructs_arr[inst] == nop_inst)
        {
            nops = +1;
            if (nops > 10)
            {
                do
                {
                    inst++;
                    if (inst >= num_of_instructs)
                        break;
                } while (instructs_arr[inst] == nop_inst);

                if (inst < num_of_instructs)
                  inst--;
            }
        }
        else
        {
            nops = 0;
        }
#endif
    }
#endif
}

#endif /* !defined(RDP_SIM) */

void drv_rnr_load_microcode()
{
    uint8_t rnr_idx;
#if (CHIP_VER >= RDP_GEN_62) && !defined (RDP_SIM) && !defined(RDP_ARCH_QEMU_SIM)
    uint32_t cores_done_mask = 0;
    uint32_t slaves_for_this_core_mask = 0;
#endif

    for (rnr_idx = 0; rnr_idx < _num_of_cores; rnr_idx++)
    {
#if (CHIP_VER >= RDP_GEN_62) && !defined (RDP_SIM) && !defined(RDP_ARCH_QEMU_SIM)
        ag_drv_rnr_quad_general_config_multi_psel_cfg_set(0, 0, 0);

        if (__drv_rnr_can_skip_instructions_load(&cores_done_mask, &slaves_for_this_core_mask, rnr_idx))
            continue;
#endif

        drv_rnr_load_instructions(rnr_idx, 0, drv_rnr_get_inst_word_size(rnr_idx), fw_inst_binaries[rnr_idx]);
    }

#if (CHIP_VER >= RDP_GEN_62) && !defined (RDP_SIM) && !defined(RDP_ARCH_QEMU_SIM)
    ag_drv_rnr_quad_general_config_multi_psel_cfg_set(0, 0, 0);
#endif
}

void drv_rnr_set_sch_cfg(void)
{

    uint8_t i;

    /* for all processing cores use rr */
    for (i = 0; i < NUM_OF_TM_RUNNERS + NUM_OF_PROCESSING_RUNNERS; ++i)
    {
        /* processing is 8SP, 8 WRR, TM is 16 SP*/
        if (IS_PROCESSING_RUNNER_IMAGE(i))
            ag_drv_rnr_regs_cfg_sch_cfg_set(i, DRV_RNR_8SP_8RR);
        else
            ag_drv_rnr_regs_cfg_sch_cfg_set(i, DRV_RNR_16SP);
    }
}

bdmf_error_t drv_rnr_sram_init(void)
{
    uint8_t rnr_idx;
    bdmf_error_t rc = BDMF_ERR_OK;
    all_rnr_regs_cfg_gen_cfg rcg_cfg_gen_cfg = {};

#if defined (RDP_SIM)
    bdmf_boolean wait_for_hw = 0;
#else    
    bdmf_boolean wait_for_hw = 1;
#endif

    uint32_t ru_get_offset_rnr_mem = RU_REG_OFFSET(ALL_RNR_MEM, MEM_ENTRY);

    drv_rnr_cores_addr_init(&ALL_RNR_MEM_BLOCK, ru_get_offset_rnr_mem);

    if (wait_for_hw)
    {
        for (rnr_idx = 0; rnr_idx < ALL_RNR_MEM_BLOCK.addr_count; rnr_idx++)
        {
            if (!valid_rnr_core[rnr_idx])
                continue;
            rc = rc ? rc : ag_drv_all_rnr_regs_cfg_gen_cfg_get(rnr_idx, &rcg_cfg_gen_cfg);
            rcg_cfg_gen_cfg.zero_data_mem = 1;
            rcg_cfg_gen_cfg.zero_context_mem = 1;
            rc = rc ? rc : ag_drv_all_rnr_regs_cfg_gen_cfg_set(rnr_idx, &rcg_cfg_gen_cfg);
            BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: regs_cfg_gen_cfg_get failed. rnr_idx: %d\n", rnr_idx);
        }        
        /* wait for core memory to be zero'd */
        for (rnr_idx = 0; rnr_idx < ALL_RNR_MEM_BLOCK.addr_count; rnr_idx++)
        {
            if (!valid_rnr_core[rnr_idx])
                continue;
            do
            {
                rc = rc ? rc: ag_drv_all_rnr_regs_cfg_gen_cfg_get(rnr_idx, &rcg_cfg_gen_cfg);
                bdmf_usleep(1*BCM_HW_POLL_INTERVAL_US);
            } while (!rcg_cfg_gen_cfg.zero_data_mem_done || !rcg_cfg_gen_cfg.zero_context_mem_done || rc != BDMF_ERR_OK);
            BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: regs_cfg_gen_cfg_get failed. rnr_idx: %d\n", rnr_idx);
        }
    }
    else
    {
        uint32_t mem_byte_num = 2 * (RU_REG_RAM_CNT(ALL_RNR_MEM, MEM_ENTRY) + 1) * sizeof(uint32_t);
        uint32_t mem_cntxt_byte_num = (RU_REG_RAM_CNT(ALL_RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);
        uint32_t ru_get_offset_rnr_cntx = RU_REG_OFFSET(ALL_RNR_CNTXT, MEM_ENTRY);

        for (rnr_idx = 0; rnr_idx < ALL_RNR_MEM_BLOCK.addr_count; rnr_idx++)
        {
            if (!valid_rnr_core[rnr_idx])
                continue;

            /* reset SRAM memory */
            MEMSET((uint32_t *)DEVICE_ADDRESS(ALL_RNR_MEM_BLOCK.addr[rnr_idx] + ru_get_offset_rnr_mem), 0, mem_byte_num);

            /* reset SRAM context memory */
            MEMSET((uint32_t *)DEVICE_ADDRESS(ALL_RNR_CNTXT_BLOCK.addr[rnr_idx] + ru_get_offset_rnr_cntx), 0, mem_cntxt_byte_num);
        }
    }

    return BDMF_ERR_OK;
}


void drv_rnr_load_prediction(void)
{
    uint8_t rnr_idx;
    uint32_t mem_pred_byte_num;
#if ((CHIP_VER >= RDP_GEN_62) &&  (!defined (RDP_SIM)))
    uint32_t cores_done_mask = 0;
    uint32_t slaves_for_this_core_mask = 0;
#endif

    for (rnr_idx = 0; rnr_idx < _num_of_cores; rnr_idx++)
    {
#if ((CHIP_VER >= RDP_GEN_62) &&  (!defined (RDP_SIM)))
        /* per core check if need to do to other cores as well */
        ag_drv_rnr_quad_general_config_multi_psel_cfg_set(0, 0, 0);

        if (__drv_rnr_can_skip_instructions_load(&cores_done_mask, &slaves_for_this_core_mask, rnr_idx))
            continue;
#endif

        mem_pred_byte_num = drv_rnr_get_inst_word_size(rnr_idx) / 8;
        memcpyl_prediction((void *)(DEVICE_ADDRESS(RU_BLK(RNR_PRED).addr[rnr_idx] + RU_REG_OFFSET(RNR_PRED, MEM_ENTRY))), fw_pred_binaries[rnr_idx], mem_pred_byte_num);
    }
#if ((CHIP_VER >= RDP_GEN_62) &&  (!defined (RDP_SIM)))
    ag_drv_rnr_quad_general_config_multi_psel_cfg_set(0, 0, 0);
#endif
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
    uint8_t filter_index, const uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS])
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

bdmf_error_t drv_rnr_quad_parser_da_filter_without_mask_set(const uint8_t *mac_address, bdmf_boolean add)
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
                for (quad_id = 0; quad_id < NUM_OF_RNR_QUAD; quad_id++)
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
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_get(&qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_get(&qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_get(&qtag_nest_0, &qtag_nest_1);
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
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_set(qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_set(qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_set(qtag_nest_0, qtag_nest_1);
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
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_get(&qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_get(&qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_get(&qtag_nest_0, &qtag_nest_1);
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
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_set(qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_set(qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_set(qtag_nest_0, qtag_nest_1);
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
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_get(&qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_get(&qtag_nest_0, &qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc =  ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get((void *) &hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_get(&qtag_nest_0, &qtag_nest_1);
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
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof0_set(qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_1)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof1_set(qtag_nest_0, qtag_nest_1);
    }
    else if (profile == DRV_PARSER_QTAG_PROFILE_2)
    {
        rc = rc ? rc : ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(hard_nest);
        rc = rc ? rc : ag_drv_rnr_quad_parser_qtag_nest_prof2_set(qtag_nest_0, qtag_nest_1);
    }
    return rc;
}

/* converts mac address from array to the format used by HW */
void parser_mac_address_array_to_hw_format(const uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS],
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

bdmf_error_t drv_rnr_quad_parser_pure_ack_set(rnr_quad_id_e quad_id, uint32_t enable)
{
    int rc = BDMF_ERR_OK;
    rnr_quad_parser_core_configuration_key_cfg parser_key_cfg = {};
    rc = ag_drv_rnr_quad_parser_core_configuration_key_cfg_get(quad_id, &parser_key_cfg);
    parser_key_cfg.l3_tcp_pure_ack_mask = enable;
    parser_key_cfg.tcp_pure_ack_mask = enable;
    rc = rc ? rc : ag_drv_rnr_quad_parser_core_configuration_key_cfg_set(quad_id, &parser_key_cfg);
    return rc;
}

bdmf_error_t drv_rnr_quad_parser_prop_cfg (rnr_quad_id_e quad_id)
{
    int rc = BDMF_ERR_OK;
    rnr_quad_parser_core_configuration_prop_tag_cfg prop_cfg = {};

    prop_cfg.size_profile_0     = PROP_TAG_SIZE_0;
    prop_cfg.size_profile_1     = PROP_TAG_SIZE_0;
    prop_cfg.size_profile_2     = PROP_TAG_SIZE_4;
    prop_cfg.pre_da_dprofile_0  = 0;
    prop_cfg.pre_da_dprofile_1  = 0;
    prop_cfg.pre_da_dprofile_2  = 0;

    rc = ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(quad_id, &prop_cfg);

    return rc;
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

    if ((win > 3) || (size_power_of_2 > 31))
        return -1;

    for (quad = 0; quad < NUM_OF_RNR_QUAD; quad++)
    {
        reg_val = (phys_addr>>8);
        ag_drv_all_rnr_cmn_rcq_common_regs_ubus_decode_cfg_0_indirect_address_psram_set(reg_idx, reg_val);

        /* write remap address */
        ag_drv_all_rnr_cmn_rcq_common_regs_ubus_decode_cfg_0_indirect_address_psram_set(reg_idx + 1, reg_val);

        /*write attributes, if going to bridge, update cache */
         if( (port_id == UBUS_PORT_ID_BIU) && (cache_bit_en))
            reg_val = (DECODE_CFG_CACHE_BITS | DECODE_CFG_ENABLE_ADDR_ONLY | (size_power_of_2 << DECODE_CFG_SIZE_SHIFT) | port_id) ;
        else
            reg_val = (DECODE_CFG_ENABLE_ADDR_ONLY | (size_power_of_2 << DECODE_CFG_SIZE_SHIFT) | port_id) ;
        ag_drv_all_rnr_cmn_rcq_common_regs_ubus_decode_cfg_0_indirect_address_psram_set(reg_idx + 2, reg_val);
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
         rnr_ctrl.enable_powersave_core_3 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_4 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_5 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_6 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_7 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_8 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_9 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_10 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_11 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_12 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_13 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_14 = auto_gate ? 1 : 0;
         rnr_ctrl.enable_powersave_core_15 = auto_gate ? 1 : 0;
         rnr_ctrl.time_counter = timer_val;
         ag_drv_rnr_quad_general_config_powersave_config_set(block_id, &rnr_ctrl);
     }

     return 0;
}

/* used for cases we need temporary auto_gate off,
 * it get old configuration and change to new one
 * has assamption all cores has same parameters
 */
int drv_rnr_config_set_get_autogate(bdmf_boolean new_auto_gate, bdmf_boolean *old_auto_gate)
{
	rnr_quad_general_config_powersave_config rnr_ctrl;
	ag_drv_rnr_quad_general_config_powersave_config_get(0, &rnr_ctrl);
	if (old_auto_gate)
	{
		*old_auto_gate = rnr_ctrl.enable_powersave_core_0;
	}
	return drv_rnr_config_clock_autogate(new_auto_gate, rnr_ctrl.time_counter);
}

bdmf_error_t ag_drv_rnr_quad_exception_bits_set(uint8_t quad_idx, uint32_t exception_en)
{
    uint8_t profile_us;
    bdmf_boolean disable_l2tp_source_port_check;
    uint8_t tcp_flags_filt;
    uint32_t exception_en_temp;
    ag_drv_all_rnr_cmn_parser_core_configuration_parser_misc_cfg_get(quad_idx, &exception_en_temp, &profile_us, &disable_l2tp_source_port_check, &tcp_flags_filt);
    return ag_drv_all_rnr_cmn_parser_core_configuration_parser_misc_cfg_set(quad_idx, exception_en, profile_us, disable_l2tp_source_port_check, tcp_flags_filt);
}

bdmf_error_t ag_drv_rnr_quad_exception_bits_get(uint8_t quad_idx, uint32_t *exception_en)
{
    uint8_t profile_us;
    bdmf_boolean disable_l2tp_source_port_check;
    uint8_t tcp_flags_filt;
    return ag_drv_all_rnr_cmn_parser_core_configuration_parser_misc_cfg_get(quad_idx, exception_en, &profile_us, &disable_l2tp_source_port_check, &tcp_flags_filt);
}

#ifdef USE_BDMF_SHELL

static bdmfmon_handle_t rnr_dir;

int drv_rnr_cli_set_parser_dos_attack(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int rc = BDMF_ERR_OK;
    uint8_t i;

    /* configure runner quads */
    for (i = 0; !rc && i < NUM_OF_RNR_QUAD; i++)
    {
        rc = ag_drv_rnr_quad_parser_core_configuration_dos_attack_set(i, parm[0].value.unumber);
    }

    return rc;
}

int drv_rnr_cli_config_trace(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rnr_regs_trace_config trace_cfg = {0};
    ag_drv_rnr_regs_trace_config_get(parm[0].value.unumber, &trace_cfg);
    trace_cfg.trace_disable_wakeup_log = parm[1].value.unumber;
    trace_cfg.trace_mode = parm[2].value.unumber;
    trace_cfg.trace_task = parm[3].value.unumber;

    trace_cfg.counters_selected_task_mode = parm[2].value.unumber;
    trace_cfg.counters_task = parm[3].value.unumber;

    trace_cfg.pc_start = parm[5].value.unumber;

    /* PC profiling is not supported yet - work only in general mode */
    trace_cfg.tracer_enable = 1;
    if (parm[4].value.unumber == 0)
    {
        /*general mode */
        trace_cfg.pc_stop_or_cycle_count = parm[6].value.unumber;
	    trace_cfg.profiling_window_mode = 0;
        trace_cfg.single_mode_start_option = 0;
    }
    else
    {
        trace_cfg.pc_stop_or_cycle_count = parm[6].value.unumber;
        trace_cfg.profiling_window_mode = 1;
        trace_cfg.single_mode_start_option = 0;
        if (parm[4].value.unumber == 1)
        {
            /* PC start and pc end mode */
            trace_cfg.single_mode_stop_option = 0;
        }
        else
        {
            /* PC start and number of cycles to end mode */
            trace_cfg.single_mode_stop_option = 3;
        }
    }

    ag_drv_rnr_regs_trace_config_set(parm[0].value.unumber, &trace_cfg);
    return 0;
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
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */ 
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
#endif
    return 0;
}

int drv_parser_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
#if 0 /* TODO - fix names*/
    uint8_t quad_idx = parm[0].value.unumber;
    static uint32_t cli_quad_parser_cfg[] = {

        cli_rnr_quad_parser_vid0,
        cli_rnr_quad_parser_vid1,
        cli_rnr_quad_parser_vid2,
        cli_rnr_quad_parser_vid3,
        cli_rnr_quad_parser_vid4,
        cli_rnr_quad_parser_vid5,
        cli_rnr_quad_parser_vid6,
        cli_rnr_quad_parser_vid7,
        cli_rnr_quad_parser_ip0,
        cli_rnr_quad_parser_ip1,
        cli_rnr_quad_parser_hardcoded_ethtype_prof0,
        cli_rnr_quad_parser_hardcoded_ethtype_prof1,
        cli_rnr_quad_parser_hardcoded_ethtype_prof2,
        cli_rnr_quad_parser_qtag_nest_prof0,
        cli_rnr_quad_parser_qtag_nest_prof1,
        cli_rnr_quad_parser_qtag_nest_prof2,
        cli_rnr_quad_parser_qtag_nest_max_vlans,
        cli_rnr_quad_parser_ip_protocol0,
        cli_rnr_quad_parser_ip_protocol1,
        cli_rnr_quad_parser_ip_protocol2,
        cli_rnr_quad_parser_ip_protocol3,
        cli_rnr_quad_parser_da_filter,
        cli_rnr_quad_parser_da_filter1,
        cli_rnr_quad_parser_da_filter2,
        cli_rnr_quad_parser_da_filter3,
        cli_rnr_quad_parser_da_filter4,
        cli_rnr_quad_parser_da_filter5,
        cli_rnr_quad_parser_da_filter6,
        cli_rnr_quad_parser_da_filter7,
        cli_rnr_quad_parser_da_filter8,
        cli_rnr_quad_da_filter_valid,
        cli_rnr_quad_exception_bits,
        cli_rnr_quad_tcp_flags,
        cli_rnr_quad_profile_us,
        cli_rnr_quad_disable_l2tp_source_port,
        cli_rnr_quad_parser_snap_conf,
        cli_rnr_quad_parser_ipv6_filter,
        cli_rnr_quad_parser_core_configuration_eng,
        cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code,
        cli_rnr_quad_parser_core_configuration_qtag_ethtype,
        cli_rnr_quad_parser_core_configuration_user_ethtype_0_1,
        cli_rnr_quad_parser_core_configuration_user_ethtype_2_3,
        cli_rnr_quad_parser_core_configuration_user_ethtype_config,
        cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1,
        cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2,
        cli_rnr_quad_parser_core_configuration_gre_protocol_cfg,
        cli_rnr_quad_parser_core_configuration_prop_tag_cfg,
        cli_rnr_quad_parser_core_configuration_dos_attack,
        cli_rnr_quad_parser_core_configuration_icmp_max_size,
        cli_rnr_quad_parser_core_configuration_key_cfg,
    };

    bdmf_session_print(session, "Parser Quad %d configuration:\n\r", quad_idx);
    bdmf_session_print(session, "=============================\n\r");

    HAL_CLI_IDX_PRINT_LIST(session, rnr_quad, cli_quad_parser_cfg, quad_idx);
#endif
    return 0;
}

int drv_rnr_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t rnr_id;
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_boolean dma_illegal_status;
    rnr_regs_cfg_int_ctrl cfg_int_ctrl;
    bdmf_boolean prediction_overrun_status;

    for (rnr_id = 0; rnr_id < _num_of_cores; rnr_id++)
    {
        rc = ag_drv_rnr_regs_dma_illegal_get(rnr_id, &dma_illegal_status);

        /* will ignore this meanwhile due to small prediction memory update FIFO size 4 */
        /* will recheck again in the future products where FIFO size will increased to 8 */
        rc = rc ? rc : ag_drv_rnr_regs_prediction_overrun_get(rnr_id, &prediction_overrun_status);
        rc = rc ? rc :ag_drv_rnr_regs_cfg_int_ctrl_get(rnr_id, &cfg_int_ctrl);

        if (!rc)
        {
            if (dma_illegal_status)
            {
                bdmf_session_print(session, "\nError:rnr core : %d ; dma_illegal_status: %d\n\r", rnr_id, dma_illegal_status);
                LOG_EVENT(system_set, 0, ev_dma_illegal_status, rnr_id, dma_illegal_status);
            }
            if (prediction_overrun_status)
            {
                 bdmf_session_print(session, "\nWarning:rnr core : %d ; prediction_overrun: %d\n\r", rnr_id, prediction_overrun_status);
                 LOG_EVENT(system_set, 0, ev_prediction_overrun, rnr_id, prediction_overrun_status);
            }
#ifndef DCL_SUPPORT
/* TODO - wait for tools suport to be able update prediction with static prediction*/
            if (cfg_int_ctrl.fit_fail_sts)
            {
                bdmf_session_print(session, "\nError:rnr core : %d ; fit_fail_sts: %d\n\r", rnr_id, cfg_int_ctrl.fit_fail_sts);
                LOG_EVENT(system_set, 0, ev_fit_fail_sts, rnr_id, cfg_int_ctrl.fit_fail_sts);
            }
#endif
        }
    }
    return 0;
}

void drv_rnr_cli_init(bdmfmon_handle_t driver_dir)
{
    rnr_dir = ag_drv_all_rnr_regs_cli_init(driver_dir);
    ag_drv_all_rnr_cmn_cli_init(rnr_dir);

    rdp_drv_bkpt_cli_init(rnr_dir);
    BDMFMON_MAKE_CMD(rnr_dir, "ct", "config tracer per core", (bdmfmon_cmd_cb_t)drv_rnr_cli_config_trace,
        BDMFMON_MAKE_PARM_ENUM("rnr_id", "rnr_id", rnr_id_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("disable_wakeup_log", "do not log scheduler events", BDMFMON_PARM_UNUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("trace_mode", "0: all tasks, 1: single task mode", BDMFMON_PARM_UNUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE_DEFVAL("task_id", "task_id for single task mode", BDMFMON_PARM_UNUMBER, 0, 0, 15, 0),
		BDMFMON_MAKE_PARM_RANGE("pc_mode", "0: No active, 1: pc start and pc end, 2:pc start and numbers of cycles to end", BDMFMON_PARM_UNUMBER, 0, 0, 2),
		BDMFMON_MAKE_PARM_RANGE("start_pc", "start pc to record ", BDMFMON_PARM_UNUMBER, 0, 0, 0x0fffffff),
		BDMFMON_MAKE_PARM_RANGE("end_pc_or_num_of_cycles", "last pc or number of cycles", BDMFMON_PARM_UNUMBER, 0, 0, 0x0fffffff),
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
    BDMFMON_MAKE_CMD(rnr_dir, "sda", "set dos attack parser configuration", drv_rnr_cli_set_parser_dos_attack,
        BDMFMON_MAKE_PARM_RANGE("reasons", "bitmask of reasons", BDMFMON_PARM_NUMBER, 0, 0, 0x7fff));
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

    rc = drv_rnr_config_set_get_autogate(0, &g_old_auto_gate);
    if (rc)
        return rc;

    /* reset profiling by passing profiling_start = 0 and counter_enable = 0 to vpb_bridge */
    rc = ag_drv_ubus_slv_profiling_cycle_num_set(0, cfg->num_cycles);
    rc = rc ? rc : ag_drv_ubus_slv_profiling_cfg_set(0, 0, 0, 0, 0);
    if (rc)
        return rc;

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
        quad_cfg.enable_trace_core_0 = cfg->enable_quads[quad];
        quad_cfg.enable_trace_core_1 = cfg->enable_quads[quad];
        quad_cfg.enable_trace_core_2 = cfg->enable_quads[quad];
        quad_cfg.enable_trace_core_3 = cfg->enable_quads[quad];
        quad_cfg.enable_trace_core_0 = 1;
        quad_cfg.enable_trace_core_1 = 1;
        quad_cfg.enable_trace_core_2 = 1;
        quad_cfg.enable_trace_core_3 = 1;
        quad_cfg.enable_trace_core_4 = 1;
        quad_cfg.enable_trace_core_5 = 1;
        quad_cfg.enable_trace_core_6 = 1;
        quad_cfg.enable_trace_core_6 = 1;
        quad_cfg.enable_trace_core_7 = 1;
        quad_cfg.enable_trace_core_6 = 1;
        quad_cfg.enable_trace_core_7 = 1;
        quad_cfg.enable_trace_core_8 = 1;
        quad_cfg.enable_trace_core_9 = 1;
        quad_cfg.enable_trace_core_10 = 1;
        quad_cfg.enable_trace_core_11 = 1;
        quad_cfg.enable_trace_core_12 = 1;
        quad_cfg.enable_trace_core_13 = 1;
        quad_cfg.enable_trace_core_14 = 1;
        quad_cfg.enable_trace_core_15 = 1;

        rc = rc ? rc : ag_drv_rnr_quad_general_config_profiling_config_set(quad, &quad_cfg);
    }
    /* manual mode need to get higher then 0 in num of cycles) */
    if (cfg->manual_stop_mode)
        cfg->num_cycles = 1;
    rc = rc ? rc : ag_drv_ubus_slv_profiling_cycle_num_set(0, cfg->num_cycles);
    rc = rc ? rc : ag_drv_ubus_slv_profiling_cfg_set(0, 1, 1, cfg->manual_stop_mode, 0);

    return rc;
}
EXPORT_SYMBOL(xrdp_rnr_profiling_set_config);

int xrdp_rnr_profiling_stop_manual_mode(void)
{
    return ag_drv_ubus_slv_profiling_cfg_set(0, 1, 1, 0, 1);
}
EXPORT_SYMBOL(xrdp_rnr_profiling_stop_manual_mode);

int xrdp_rnr_profiling_get_result(struct xrdp_rnr_profiling_res *res)
{
    int core, rc = BDMF_ERR_OK;
    bdmf_boolean profiling_on = 0;
    uint32_t num_cycles;

    /* verify that profiling is not running right now */
#ifdef RDP_SIM
    num_cycles = 0xFFFFFFFF;
#else
    rc = ag_drv_ubus_slv_profiling_status_get(&profiling_on, &num_cycles);
    if (rc)
        return rc;
#endif

    res->profiling_on = profiling_on;
    if (!profiling_on && num_cycles)
    {
        rc = drv_rnr_config_set_get_autogate(g_old_auto_gate, NULL);
        res->total_cnt = (uint32_t)(num_cycles * RNR_FREQ_IN_MHZ / UBUS_SLV_FREQ_IN_MHZ);
        for (core = 0; core < NUM_OF_TM_RUNNERS + NUM_OF_PROCESSING_RUNNERS; core++)
        {
            rc = rc ? rc : ag_drv_rnr_regs_rnr_core_cntrs_get(core, &res->cntrs[core]);
        }
    }

    return rc;
}
EXPORT_SYMBOL(xrdp_rnr_profiling_get_result);

