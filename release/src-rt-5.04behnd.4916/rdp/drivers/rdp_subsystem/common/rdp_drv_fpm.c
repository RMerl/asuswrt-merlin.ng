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

#ifndef RUNNER_MPM_SUPPORT

#include "rdp_drv_xpm.h"
#include "ru.h"
#include "rdp_drv_fpm.h"
#include "data_path_init.h"

#include "bdmf_dev.h"
#include "xrdp_autogens.h"

extern int invalid_free_token_valid;
extern int invalid_mcast_token_valid;

#ifndef RDP_ARCH_QEMU_SIM
static void reset_fpm_statistics(void)
{
    uint32_t tmp32 = 0xFFFFFFFF;

    RU_REG_WRITE(0, FPM, POOL1_STAT1, tmp32);
    RU_REG_WRITE(0, FPM, POOL1_STAT2, tmp32);
    RU_REG_WRITE(0, FPM, POOL1_STAT3, tmp32);
    RU_REG_WRITE(0, FPM, POOL1_STAT4, tmp32);
    RU_REG_WRITE(0, FPM, POOL1_STAT5, tmp32);
    RU_REG_WRITE(0, FPM, POOL1_STAT6, tmp32);
    RU_REG_WRITE(0, FPM, POOL1_STAT7, tmp32);
    RU_REG_WRITE(0, FPM, POOL1_STAT8, tmp32);
    RU_REG_WRITE(0, FPM, POOL1_INTR_STS, tmp32);
}
#endif

/* 
   Configure FPM hardware to ignore FPM buffers beyond available fpm_pool_memory_size 
   algorytem : 
   1. calculated the number of buffers that can be used in the avilable memory space
   2. allocate all buffers in FPM
   3. release only buffers in the supported range calculated in 1.
   4. in addition , there are max of N prefetch buffers, that why we start free only 
   from this number, to make sure we alloc (and free) all the tokents beetween N and the supported size
   we added calculation that verify it (and return fail in case the calculation is failed)
*/
static int drv_fpm_reduce_fpm_tokens(void *virt_base, uintptr_t phys_base, uint32_t fpm_pool_memory_size, uint32_t fpm_buf_size)
{
    int rc = 0;
#ifndef CONFIG_BRCM_QEMU_NO_INDIRECT
    uint32_t i;
    uint32_t fpm_buff_num;
    uint32_t calculated_tokens = 0;
    uint32_t actual_calculated_tokens = 0;
#endif

    configured_total_fpm_tokens = fpm_pool_memory_size / fpm_buf_size;

    configured_total_fpm_tokens = (configured_total_fpm_tokens < TOTAL_FPM_TOKENS) ? configured_total_fpm_tokens : TOTAL_FPM_TOKENS;

    /* Make sure configured_total_fpm_tokens is a power of 2 value.
       It should always be like that due to the limitation on FPM pool size
       configuration to be one of 16/32/64/128/256MB values and it has not
       been tested for non power of 2 values. So just a precaution.
    */
    if (!IS_POWER_OF_2_INTEGER(configured_total_fpm_tokens))
    {
        BDMF_TRACE_RET(BDMF_ERR_INTR, "ERROR: Configured Number of FPM tokens 0x%X is not a power of 2 value\n",
                            configured_total_fpm_tokens);
    }

    drv_xpm_common_init(virt_base, phys_base, fpm_buf_size, configured_total_fpm_tokens);

#ifndef CONFIG_BRCM_QEMU_NO_INDIRECT
    if(configured_total_fpm_tokens < TOTAL_FPM_TOKENS)
    { 
        /* Allocate all available FPM tokens from pool 3 */
        do
        {
            rc = drv_fpm_alloc_buffer(1, &fpm_buff_num);
            fpm_buff_num = fpm_buff_num & FPM_INDX_MASK;

            if ((rc == 0) && (FPM_MAX_PREFETCH_TOKEN <= fpm_buff_num) && (fpm_buff_num < configured_total_fpm_tokens))
                actual_calculated_tokens += fpm_buff_num;

#if defined(__KERNEL__)
            udelay(10); /* avoid multiple accesses from CPU to FPM without a delay */
#endif
        }while (rc == 0);

        if (rc != BDMF_ERR_NOMEM)
        {
           BDMF_TRACE_RET(rc, "Failed to release entire FPM\n"); 
        }

        /*here rc should be BDMF_ERR_NOMEM which is ok */
        rc = 0;
        /* Release FPM tokens referring to available FPM pool memory. */
        for (i = FPM_MAX_PREFETCH_TOKEN; i < configured_total_fpm_tokens; ++i)
        {
            calculated_tokens +=i;
#if defined(__KERNEL__)
            udelay(10); /* avoid multiple accesses from CPU to FPM without a delay */
#endif

            rc = drv_fpm_free_buffer(1, i | FPM_MIN_POOL_SIZE << FPM_POOL_ID_SHIFT, NULL);
            if (rc)
            {
                BDMF_TRACE_RET(rc, "Failed to release FPM buffer %d pool %d\n", i, FPM_MIN_POOL_SIZE);
            }
        }

        if (actual_calculated_tokens != calculated_tokens) 
        {
            rc = BDMF_ERR_INTERNAL;
            BDMF_TRACE_RET(rc, "error in fpm token init actual_calculated_tokens=0x%x, calculated_tokens=0x%x\n", actual_calculated_tokens, calculated_tokens);
        }

        /* Reset FPM HW statistics. */
        reset_fpm_statistics();
    }
#endif

    return rc;
}

void drv_fpm_select_pools(int *pool_size, uint32_t pool_set)
{
    pool_size[0] = pool_set & 0xff;
    pool_size[1] = (pool_set >> 8) & 0xff;
    pool_size[2] = (pool_set >> 16) & 0xff;;
    pool_size[3] = (pool_set >> 24) & 0xff;;
}


int drv_fpm_pool0_encode(int tokens)
{
    /* POOL_0_SIZE   0: 6 tkns, 1: 7 tkns, 2: 8 tkns(default), 3: 20 tkns,*/
    int encode = 2;
    switch (tokens)
    {
        case 20: encode = 3; break;
        case 8:  encode = 2; break;
        case 7:  encode = 1; break;
        case 6:  encode = 0; break;
        default:
            BDMF_TRACE_ERR("\n=== %s:%d_Error: unsupported number of tokens %d\n", __FUNCTION__, __LINE__, tokens);
            break;
    }

    return encode;
}

int drv_fpm_pool1_encode(int tokens)
{
    /* POOL_0_SIZE   0: 4 tkns(default), 1: 5 tkns, 2: 6 tkns, 3: 8 tkns,*/

    int encode = 0;
    switch (tokens)
    {
        case 8: encode = 3; break;
        case 6: encode = 2; break;
        case 5: encode = 1; break;
        case 4: encode = 0; break;
        default:
            BDMF_TRACE_ERR("\n=== %s:%d_Error: unsupported number of tokens %d\n", __FUNCTION__, __LINE__, tokens);
            break;
    }

    return encode;
}

int drv_fpm_init(dpi_params_t *p_dpi_cfg)
{
    int rc;
    int pool_size[4];

    rc = drv_fpm_reduce_fpm_tokens(p_dpi_cfg->rdp_ddr_pkt_base_virt, p_dpi_cfg->rdp_ddr_pkt_base_phys, p_dpi_cfg->fpm_pool_memory_size, p_dpi_cfg->fpm_buf_size);

    if (!rc)
    {
        drv_fpm_select_pools(pool_size, p_dpi_cfg->fpm_pool_size_tokens_set);
        drv_xpm_common_update_pool_size(pool_size);

        update_rdp_fpm_resources(p_dpi_cfg->fpm_pool_memory_size, p_dpi_cfg->fpm_buf_size,
                                 TOTAL_FPM_TOKENS, configured_total_fpm_tokens, pool_size);
    }

    return rc;
}


bdmf_error_t drv_fpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num)
{
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */

#ifndef RDP_SIM
    bdmf_boolean token_valid = 0;
    bdmf_boolean ddr_pool = 0;
    uint32_t fpm_pool = 0;
    uint32_t token_idx = 0;
    uint16_t token_size;
    bdmf_error_t rc = 0;

    if (packet_len <= (xpm_common_cfg.buf_size * xpm_common_cfg.pool_size[3]))
    {
        rc = ag_drv_fpm_pool4_alloc_dealloc_get(&token_valid, &ddr_pool, &token_idx, &token_size);
        fpm_pool = 3;
    }
    else if (packet_len <= (xpm_common_cfg.buf_size * xpm_common_cfg.pool_size[2]))
    {
        rc = ag_drv_fpm_pool3_alloc_dealloc_get(&token_valid, &ddr_pool, &token_idx, &token_size);
        fpm_pool = 2;
    }
    else if (packet_len <= (xpm_common_cfg.buf_size * xpm_common_cfg.pool_size[1]))
    {
        rc = ag_drv_fpm_pool2_alloc_dealloc_get(&token_valid, &ddr_pool, &token_idx, &token_size);
        fpm_pool = 1;
    }
    else if (packet_len <= (xpm_common_cfg.buf_size * xpm_common_cfg.pool_size[0]))
    {
        rc = ag_drv_fpm_pool1_alloc_dealloc_get(&token_valid, &ddr_pool, &token_idx, &token_size);
        fpm_pool = 0;
    }

    if (!token_valid)
        return BDMF_ERR_NOMEM;

    *buff_num = (fpm_pool << FPM_POOL_ID_SHIFT) | token_idx;

    return rc;
#else
    /* TODO: support non standard multipliers in Simulator */
    return rdp_cpu_fpm_alloc(packet_len, buff_num, xpm_common_cfg.buf_size);
#endif
#else
	return 0;
	/* 
		use with new APIS (and parameters 
		ag_drv_fpm_pool1_alloc_dealloc_1_set(uint16_t token_size, uint32_t token_index, bdmf_boolean ddr, bdmf_boolean token_valid) 
	*/
#endif
}

#if defined(CONFIG_CPU_RX_FROM_XPM)
extern bdmf_attr_enum_table_t bufmng_enum_table; 
void drv_fpm_print_non_empty_token_info(bdmf_session_handle session)
{
    ddr_token_info_t *info;
    uint32_t token_idx;
    uint32_t valid_entries = 0;
    uint32_t total_fpms = 0;

    bdmf_session_print(session, "========== FPM DATA BASE VALID ENTRIES: =======\n");
    for (token_idx = 0; token_idx < xpm_common_cfg.num_of_token; token_idx++)
    {
        info = drv_xpm_ddr_token_info_get(token_idx);
        if (info->is_xpm_valid)
        {
            bdmf_session_print(session, "token_idx %d:, \tinfo->pool_id: %d\t bufmng_cnt_id %s\n", token_idx, info->pool_id, bdmf_attr_get_enum_text_hlp(&bufmng_enum_table, info->bufmng_cnt_id));
            total_fpms += xpm_common_cfg.pool_size[info->pool_id];
            valid_entries += 1;
        }
    }
    bdmf_session_print(session, "========== FPM DATA BASE VALID DONE =======\n");
    bdmf_session_print(session, "valid_entries: %d\t, total_fpms: %d\n", valid_entries, total_fpms);
}
#endif

bdmf_error_t drv_fpm_free_buffer(uint32_t packet_len, uint32_t buff_num, void *info_in)
{
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */
    ddr_token_info_t *info;
    ddr_token_info_t info_bckup;
    bdmf_boolean token_valid = 1;
    bdmf_boolean ddr_pool = 0;
    uint32_t token_idx = buff_num & FPM_INDX_MASK;
    bdmf_error_t rc = 0;
    uint32_t calc_packet_len;

#if CHIP_VER >= RDP_GEN_61
    calc_packet_len = 0;
#else
    calc_packet_len = packet_len;
#endif
   
    if (!info_in)
        info = drv_xpm_ddr_token_info_get(token_idx);
    else
        info = (ddr_token_info_t *)info_in;

    info_bckup = *info;
    drv_xpm_ddr_token_info_clear(token_idx);

#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
    if (info_bckup.bufmng_cnt_id != BDMF_INDEX_UNASSIGNED)
        drv_xpm_buffer_update_cpu_free_cnt(token_idx, info_bckup.bufmng_cnt_id, info_bckup.pool_id);
#endif
    if (info_bckup.is_xpm_valid)
    {
        switch (info_bckup.pool_id)
        {
            case 0:
                rc = ag_drv_fpm_pool1_alloc_dealloc_set(token_valid, ddr_pool, token_idx, calc_packet_len);
                break;
            case 1:
                rc = ag_drv_fpm_pool2_alloc_dealloc_set(token_valid, ddr_pool, token_idx, calc_packet_len);
                break;
            case 2:
                rc = ag_drv_fpm_pool3_alloc_dealloc_set(token_valid, ddr_pool, token_idx, calc_packet_len);
                break;
            case 3:
                rc = ag_drv_fpm_pool4_alloc_dealloc_set(token_valid, ddr_pool, token_idx, calc_packet_len);
                break;
        }
    }
    else
    {
        if (packet_len <= (xpm_common_cfg.buf_size * xpm_common_cfg.pool_size[3]))
            rc = ag_drv_fpm_pool4_alloc_dealloc_set(token_valid, ddr_pool, token_idx, calc_packet_len);
        else if (packet_len <= (xpm_common_cfg.buf_size * xpm_common_cfg.pool_size[2]))
            rc = ag_drv_fpm_pool3_alloc_dealloc_set(token_valid, ddr_pool, token_idx, calc_packet_len);
        else if (packet_len <= (xpm_common_cfg.buf_size * xpm_common_cfg.pool_size[1]))
            rc = ag_drv_fpm_pool2_alloc_dealloc_set(token_valid, ddr_pool, token_idx, calc_packet_len);
        else if (packet_len <= (xpm_common_cfg.buf_size * xpm_common_cfg.pool_size[0]))
            rc = ag_drv_fpm_pool1_alloc_dealloc_set(token_valid, ddr_pool, token_idx, calc_packet_len);
    }

    return rc;
#else
    return 0;
#endif
}

bdmf_error_t drv_fpm_check_xoff(uint32_t num_of_token)
{
    bdmf_error_t rc = 0;
#ifndef RDP_SIM
    fpm_pool_stat fpm_stat;
    uint16_t xon_thr, xoff_thr;

    rc = ag_drv_fpm_pool_stat_get(&fpm_stat);
#if CHIP_VER >= RDP_GEN_62
    rc = rc ? rc : ag_drv_fpm_pool1_xon_xoff_cfg_get(&xoff_thr, &xon_thr);
#else
    rc = rc ? rc : ag_drv_fpm_pool1_xon_xoff_cfg_get(&xon_thr, &xoff_thr);
#endif

    if (rc || ((fpm_stat.num_of_tokens_available - num_of_token) <= xoff_thr))
        return BDMF_ERR_NORES;

#endif
    return rc;
}

#ifdef DQM_MINI_FPM_MODE
void drv_fpm_data_path_mini_fpm_init(uint32_t fpm_mini_tokens)
{
    bdmf_boolean fpmini_init;

    /* Initialize minifpm */
    ag_drv_fpm_fpmini_fpmini_lvl_0_reg_l0_set(fpm_mini_tokens);

    ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_set(1);
    /* coverity[dead_error_condition:SUPPRESS] */
    FPM_INIT_LOOP
    {
        /* coverity[dead_error_line:SUPPRESS] */
        ag_drv_fpm_fpmini_fpmini_cfg0_l2_init_get(&fpmini_init);
    }
    ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_set(1);
    /* coverity[dead_error_condition:SUPPRESS] */
    FPM_INIT_LOOP
    {
        /* coverity[dead_error_line:SUPPRESS] */
        ag_drv_fpm_fpmini_fpmcast_cfg0_mc_init_get(&fpmini_init);
    }
}
#endif

#if CHIP_VER < RDP_GEN_60
static uint32_t g_extra_dqm_tokens = 0;

uint32_t drv_fpm_dqm_extra_fpm_tokens_get(void)
{
    return g_extra_dqm_tokens;
}

static uint32_t drv_fpm_dqm_extra_fpm_tokens_set(uint32_t fpm_buf_size)
{
    switch(fpm_buf_size)
    {
        case FPM_BUF_SIZE_256:
        case FPM_BUF_SIZE_512:
            g_extra_dqm_tokens = FPM_EXTRA_TOKENS_FOR_DQM;
            break;
        case FPM_BUF_SIZE_1K:
        case FPM_BUF_SIZE_2K:
            g_extra_dqm_tokens = 0;
            break;
        default:
            return BDMF_ERR_PARM; /* Invalid token size */
    }
    return 0;
}

int drv_fpm_data_path_init(dpi_params_t *p_dpi_cfg, fpm_pool_cfg *pool_cfg, int extra_dqm_tokens_set)
{
    int rc;
    bdmf_boolean reset_req = 1;
    fpm_pool2_intr_sts interrupt_status = FPM_INTERRUPT_STATUS;
    fpm_pool2_intr_msk interrupt_mask = FPM_INTERRUPT_MASK;
    uint16_t timeout = 0;

    if (extra_dqm_tokens_set)
        drv_fpm_dqm_extra_fpm_tokens_set(p_dpi_cfg->fpm_buf_size);

    rc = ag_drv_fpm_pool1_xon_xoff_cfg_set(FPM_XON_THRESHOLD + g_extra_dqm_tokens,
        FPM_XOFF_THRESHOLD + g_extra_dqm_tokens);
    rc = rc ? rc : ag_drv_fpm_init_mem_set(1);

    /* polling until reset is finished */
    while (!rc && reset_req && timeout <= FPM_INIT_TIMEOUT)
    {
        rc = ag_drv_fpm_init_mem_get(&reset_req);
        xrdp_usleep(FPM_POLL_SLEEP_TIME);
        timeout++;
    }
    if (timeout == FPM_INIT_TIMEOUT)
        rc = BDMF_ERR_INTERNAL;

    /* enable the fpm pool */
    rc = rc ? rc : ag_drv_fpm_pool1_en_set(1);
    if (pool_cfg)
        rc = rc ? rc : ag_drv_fpm_pool_cfg_set(pool_cfg);

    /* fpm configurations */
    /* fpm interrupts */
    rc = rc ? rc : ag_drv_fpm_pool1_intr_sts_set(&interrupt_status);
    rc = rc ? rc : ag_drv_fpm_pool1_intr_msk_set(&interrupt_mask);

    /* FPM Driver initialization + Token adjustment */
    rc = rc ? rc : drv_fpm_init(p_dpi_cfg);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize fpm driver\n");

    return rc;
}
#else /* #if CHIP_VER < RDP_GEN_60 */
    /* TODO */
#endif

#ifdef USE_BDMF_SHELL
/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

int drv_fpm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
#if (CHIP_VER < RDP_GEN_100) 
    static uint32_t fpm_debug[] = {cli_fpm_pool_stat, cli_fpm_pool1_count, cli_fpm_pool1_intr_sts, cli_fpm_fpm_bb_dbg_cfg, cli_fpm_ddr0_weight, cli_fpm_pool_cfg, cli_fpm_pool1_xon_xoff_cfg, cli_fpm_fpm_not_empty_cfg};
#else
    static uint32_t fpm_debug[] = {cli_fpm_pool_stat, cli_fpm_stat5, cli_fpm_xon_xoff_cfg, cli_fpm_not_empty_cfg};
#endif    
    /* get fpm debug information */
    bdmf_session_print(session, "\nFPM debug:\n");
    HAL_CLI_PRINT_LIST(session, fpm, fpm_debug);

    return drv_fpm_cli_sanity_check(session, parm, n_parms);
}

int drv_fpm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO implemnt sanity with new registers*/
    fpm_pool_stat pool_stat = {};
    int rc;

    rc = ag_drv_fpm_pool_stat_get(&pool_stat);

    if (!rc)
    {
        if (pool_stat.ovrfl)
        {
            bdmf_session_print(session, "\nError:FPM: overflow\n");
            LOG_EVENT(system_set, 0, ev_fpm_overflow, 0);
        }
        if (pool_stat.undrfl)
        {
            bdmf_session_print(session, "\nError:FPM: underflow\n");
            LOG_EVENT(system_set, 0, ev_fpm_underflow, 0);
        }

        if (invalid_free_token_valid)
        {
            bdmf_session_print(session, "\nError:FPM: invalid token freed\n");
            LOG_EVENT(system_set, 0, ev_fpm_invalid_token_freed, 0);
        }
        if (invalid_mcast_token_valid)
        {
            bdmf_session_print(session, "\nError:FPM: invalid mcast token freed\n");
            LOG_EVENT(system_set, 0, ev_fpm_invalid_mcast_token, 0);
        }
    }

    return rc;
#else
    return 0;
#endif
}

int drv_fpm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO implemnt sanity with new registers*/
#if CHIP_VER >= RDP_GEN_60 || defined(BCM63158)
    static uint32_t pool_cfg[] = {cli_fpm_pool1_en, cli_fpm_pool_cfg, cli_fpm_timer, cli_fpm_fpm_bb_cfg,
                                  cli_fpm_pool1_xon_xoff_cfg, cli_fpm_fpm_not_empty_cfg,
                                  cli_fpm_ddr0_weight, cli_fpm_ddr1_weight};
#else
    static uint32_t pool_cfg[] = {cli_fpm_pool1_en, cli_fpm_pool_cfg, cli_fpm_timer, cli_fpm_fpm_bb_cfg,
                                  cli_fpm_pool1_xon_xoff_cfg, cli_fpm_fpm_not_empty_cfg, cli_fpm_pool_search,
                                  cli_fpm_pool2_search2, cli_fpm_pool3_search3, cli_fpm_pool4_search4,
                                  cli_fpm_ddr0_weight, cli_fpm_ddr1_weight};
#endif

    /* get fpm pool cfg */
    bdmf_session_print(session, "\nFPM configurations:\n");
    HAL_CLI_PRINT_LIST(session, fpm, pool_cfg);
#endif
    return 0;
}

static int drv_fpm_dump(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t fpm_bn = parm[0].value.unumber;
    uint16_t len = parm[1].value.unumber;
    void *fpm_buffer_ptr;

    if (!len)
        len = xpm_common_cfg.buf_size;
    fpm_buffer_ptr = (void *)((uint8_t *)xpm_common_cfg.virt_base + fpm_bn * xpm_common_cfg.buf_size);
    bdmf_session_print(session, "\nDump FPM #%d (%x), ptr %px:\n", fpm_bn, fpm_bn, fpm_buffer_ptr);

    bdmf_session_hexdump(session, fpm_buffer_ptr, 0, len);
    return 0;
}

static bdmfmon_handle_t fpm_dir;

void drv_fpm_cli_init(bdmfmon_handle_t driver_dir)
{
    fpm_dir = ag_drv_fpm_cli_init(driver_dir);

    BDMFMON_MAKE_CMD_NOPARM(fpm_dir, "debug_get", "get debug information", (bdmfmon_cmd_cb_t)drv_fpm_cli_debug_get);
    BDMFMON_MAKE_CMD_NOPARM(fpm_dir, "sanity", "sanity check", (bdmfmon_cmd_cb_t)drv_fpm_cli_sanity_check);
    BDMFMON_MAKE_CMD_NOPARM(fpm_dir, "cfg_get", "fpm configuration", (bdmfmon_cmd_cb_t)drv_fpm_cli_config_get);
    BDMFMON_MAKE_CMD(fpm_dir, "dump_fpm", "dump fpm", (bdmfmon_cmd_cb_t)drv_fpm_dump,
        BDMFMON_MAKE_PARM("fpm_bn", "FPM buffer number", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("len", "Length to dump, 0 for default fpm size", BDMFMON_PARM_NUMBER, 0));

#if CHIP_VER >= RDP_GEN_62
    ag_drv_fpm_fpmini_cli_init(fpm_dir);
#endif
}


void drv_fpm_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (fpm_dir)
    {
        bdmfmon_token_destroy(fpm_dir);
        fpm_dir = NULL;
    }
}

#endif /* USE_BDMF_SHELL */

#endif /* RUNNER_MPM_SUPPORT */
