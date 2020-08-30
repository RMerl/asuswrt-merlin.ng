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

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_drv_fpm.h"
#include "rdp_subsystem_common.h"
#include "rdp_common.h"
#include "XRDP_AG.h"
#include "data_path_init.h"
#ifndef RDP_SIM
#include "bcm_mm.h"
#endif

#undef FLUSH_RANGE
#undef INV_RANGE

#ifdef _CFE_
extern void _cfe_flushcache ( int, uint8_t *, uint8_t * );
#define FLUSH_RANGE(s,l)                    _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))
#define FLUSH_FPM_RANGE(s,l)                _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))
#define INV_RANGE(s,l)                      _cfe_flushcache(CFE_CACHE_INVAL_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))
#else
#define FLUSH_RANGE(s,l)                    bdmf_dcache_flush(s,l);
#define FLUSH_FPM_RANGE(s,l)                bdmf_fpm_dcache_flush(s,l);
#define INV_RANGE(s,l)                      bdmf_dcache_inv(s,l);
#endif

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

typedef struct {
    void *virt_base;
    unsigned int buf_size;
} fpm_cfg_t;

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Global variables definitions                                               */
/*                                                                            */
/******************************************************************************/

fpm_cfg_t fpm_cfg;

void drv_fpm_init(void *virt_base, unsigned int fpm_buf_size)
{
    fpm_cfg.virt_base = virt_base;
    fpm_cfg.buf_size = fpm_buf_size;

    bdmf_trace("FPM driver: base 0x%p; buffer size %d\n", virt_base, fpm_buf_size);
}

bdmf_error_t drv_fpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num)
{
#ifndef RDP_SIM
    bdmf_boolean token_valid = 0;
    bdmf_boolean ddr_pool = 0;
    uint32_t fpm_pool = 0;
    uint32_t token_index = 0;
    uint16_t token_size;
    bdmf_error_t rc = 0;

    if (packet_len <= fpm_cfg.buf_size)
    {
        rc = ag_drv_fpm_pool4_alloc_dealloc_get(&token_valid, &ddr_pool, &token_index, &token_size);
        fpm_pool = 3;
    }
    else if (packet_len <= (fpm_cfg.buf_size<<1))
    {
        rc = ag_drv_fpm_pool3_alloc_dealloc_get(&token_valid, &ddr_pool, &token_index, &token_size);
        fpm_pool = 2;
    }
    else if (packet_len <= (fpm_cfg.buf_size<<2))
    {
        rc = ag_drv_fpm_pool2_alloc_dealloc_get(&token_valid, &ddr_pool, &token_index, &token_size);
        fpm_pool = 1;
    }
    else if (packet_len <= (fpm_cfg.buf_size<<3))
    {
        rc = ag_drv_fpm_pool1_alloc_dealloc_get(&token_valid, &ddr_pool, &token_index, &token_size);
        fpm_pool = 0;
    }

    if (!token_valid)
        return BDMF_ERR_NOMEM;

#if defined (BCM6858)
    *buff_num = (ddr_pool << 16) | token_index;
#else
    *buff_num = (fpm_pool << 16) | token_index;
#endif

    return rc;
#else    
    return rdp_cpu_fpm_alloc(packet_len, buff_num);
#endif
}
EXPORT_SYMBOL(drv_fpm_alloc_buffer);

bdmf_error_t drv_fpm_free_buffer(uint32_t packet_len, uint32_t buff_num)
{
#ifndef RDP_SIM  
    bdmf_boolean token_valid = 1;
    bdmf_boolean ddr_pool = 0;
    uint32_t token_index = buff_num & 0xFFFF;
    bdmf_error_t rc = 0;

    if (packet_len <= fpm_cfg.buf_size)
        rc = ag_drv_fpm_pool4_alloc_dealloc_set(token_valid, ddr_pool, token_index, packet_len);
    else if (packet_len <= (fpm_cfg.buf_size<<1))
        rc = ag_drv_fpm_pool3_alloc_dealloc_set(token_valid, ddr_pool, token_index, packet_len);
    else if (packet_len <= (fpm_cfg.buf_size<<2))
        rc = ag_drv_fpm_pool2_alloc_dealloc_set(token_valid, ddr_pool, token_index, packet_len);
    else if (packet_len <= (fpm_cfg.buf_size<<3))
        rc = ag_drv_fpm_pool1_alloc_dealloc_set(token_valid, ddr_pool, token_index, packet_len);

    return rc;
#else
    rdp_cpu_fpm_free(buff_num);   
    return 0;
#endif    
}
EXPORT_SYMBOL(drv_fpm_free_buffer);

void *drv_fpm_buffer_get_address(uint32_t fpm_bn)
{
    return (void *)((uint8_t *)fpm_cfg.virt_base + fpm_bn * fpm_cfg.buf_size);
}
EXPORT_SYMBOL(drv_fpm_buffer_get_address);

void drv_fpm_copy_from_host_buffer(void *data, uint32_t fpm_bn, uint32_t packet_len, uint16_t offset)
{
    void *fpm_buffer_ptr;

    fpm_buffer_ptr = ((uint8_t *)drv_fpm_buffer_get_address(fpm_bn)) + offset;
#ifndef XRDP_EMULATION 	
    MWRITE_BLK_8(fpm_buffer_ptr, data, packet_len);
#else
    MWRITE_BLK_8((uint32_t)fpm_buffer_ptr, (uint8_t *)data, packet_len);
#endif

#ifndef RDP_SIM 
    /* Be sure that all memory access done before flush */
    __asm__ __volatile__ ("dsb    sy");
  
    FLUSH_FPM_RANGE((unsigned long)fpm_buffer_ptr, packet_len);
#endif
}

void lookup_bbh_tx_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsize, uint8_t *bbh_tx_bufsize)
{
#ifndef _CFE_
    switch(*fpm_bufsize)
    {
    case FPM_BUF_SIZE_256:
        *bbh_tx_bufsize = BUF_256;
        break;
    case FPM_BUF_SIZE_512:
        *bbh_tx_bufsize = BUF_512;
        break;
    case FPM_BUF_SIZE_1K:
        *bbh_tx_bufsize = BUF_1K;
        break;
    case FPM_BUF_SIZE_2K:
        *bbh_tx_bufsize = BUF_2K;
        break;
    default:
        *bbh_tx_bufsize = BUF_512;
    }
#else
    *bbh_tx_bufsize = BUF_512;
#endif
    return;
}

void lookup_dma_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsz, uint8_t* dma_bufsz)
{
    switch (*fpm_bufsz)
    {
    case FPM_BUF_SIZE_256:
        *dma_bufsz = DMA_BUFSIZE_256;
        break;
    case FPM_BUF_SIZE_1K:
        *dma_bufsz = DMA_BUFSIZE_1024;
        break;
    case FPM_BUF_SIZE_2K:
        *dma_bufsz = DMA_BUFSIZE_2048;
        break;
    default:
        *dma_bufsz = DMA_BUFSIZE_512;
    }
}

#ifdef USE_BDMF_SHELL

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

int drv_fpm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t fpm_debug[] = {cli_fpm_pool_stat, cli_fpm_pool1_count, cli_fpm_pool1_intr_sts, cli_fpm_fpm_bb_dbg_cfg};

    /* get fpm debug information */
    bdmf_session_print(session, "\nFPM debug:\n");
    HAL_CLI_PRINT_LIST(session, fpm, fpm_debug);

    return drv_fpm_cli_sanity_check(session, parm, n_parms);
}

int drv_fpm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    fpm_pool_stat pool_stat = {};
    int rc;

    rc = ag_drv_fpm_pool_stat_get(&pool_stat);

    if (!rc)
    {
        if (pool_stat.ovrfl)
            bdmf_session_print(session, "\nFPM: overflow\n");
        if (pool_stat.undrfl)
            bdmf_session_print(session, "\nFPM: underflow\n");
        if (pool_stat.invalid_free_token_valid)
            bdmf_session_print(session, "\nFPM: invalid token freed\n");
        if (pool_stat.invalid_mcast_token_valid)
            bdmf_session_print(session, "\nFPM: invalid mcast token freed\n");
    }

    return rc;
}

int drv_fpm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t pool_cfg[] = {cli_fpm_pool1_en, cli_fpm_pool_cfg, cli_fpm_timer, cli_fpm_fpm_bb_cfg, 
                                  cli_fpm_pool1_xon_xoff_cfg, cli_fpm_fpm_not_empty_cfg, cli_fpm_pool_search,
                                  cli_fpm_pool2_search2, cli_fpm_pool3_search3, cli_fpm_pool4_search4,
                                  cli_fpm_ddr0_weight, cli_fpm_ddr1_weight};

    /* get fpm pool cfg */
    bdmf_session_print(session, "\nFPM configurations:\n");
    HAL_CLI_PRINT_LIST(session, fpm, pool_cfg);

    return 0;
}

static int drv_fpm_dump(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t fpm_bn = parm[0].value.unumber;
    uint16_t len = parm[1].value.unumber;
    void *fpm_buffer_ptr;

    if (!len)
        len = fpm_cfg.buf_size;
    fpm_buffer_ptr = (void *)((uint8_t *)fpm_cfg.virt_base + fpm_bn * fpm_cfg.buf_size);
    bdmf_session_print(session, "\nDump FPM #%d (%x), ptr %p:\n", fpm_bn, fpm_bn, fpm_buffer_ptr);

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

