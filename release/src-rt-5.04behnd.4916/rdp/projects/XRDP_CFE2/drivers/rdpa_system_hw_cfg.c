/*
   Copyright (c) 2014 Broadcom
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
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
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdp_common.h"
#include "data_path_init.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_platform.h"
#include "rdp_drv_fpm.h"
#include "bcm_hwdefs.h"
#include "ru_types.h"
#include "board.h"
#include "bcm_misc_hw_init.h"
#include "rdp_drv_rnr.h"
#include "bcm_OS_Deps.h"
extern const ru_block_rec *RU_ALL_BLOCKS[];
extern unsigned long BBH_TX_ADDRS[];
extern unsigned long BBH_RX_ADDRS[];
#if defined(CONFIG_BCM963158)
extern unsigned long XLIF_RX_IF_ADDRS[];
extern unsigned long XLIF_RX_FLOW_CONTROL_ADDRS[];
extern unsigned long XLIF_TX_IF_ADDRS[];
extern unsigned long XLIF_TX_FLOW_CONTROL_ADDRS[];
extern unsigned long DEBUG_BUS_ADDRS[];
extern unsigned long XLIF_EEE_ADDRS[];
extern unsigned long XLIF_Q_OFF_ADDRS[];
#endif

dpi_params_t dpi_params = {};

extern int bcm_misc_xfi_port_get(void);

#include <linux/of.h>
#include <linux/of_address.h>
#include "ru_remap.h"
extern unsigned long bcm_io_block_address[];
uintptr_t xrdp_phys_base = 0;
uintptr_t xrdp_virt_base = 0;

uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx)
{
    return ru_block->addr[addr_idx] - (xrdp_virt_base - xrdp_phys_base);
}

void remap_ru_block_single_instance(ru_block_rec *ru_block, uintptr_t virt_base,  uint32_t phys_base)
{
    uintptr_t ptr;
    uint32_t addr_itr;
    ptr = virt_base - phys_base;

    for (addr_itr = 0; addr_itr < ru_block->addr_count; addr_itr++)
    {
        ru_block->addr[addr_itr] += ptr;
    }
}

void remap_ru_blocks(void)
{
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880)
    bbh_id_e bbh_idx;
    uintptr_t wan_bbh_virt_base;
    uintptr_t wan_bbh_phys_base;
    uint32_t wan_bbh_len;
#endif

    int i;
    uint32_t xrdp_phys_len;

#ifdef CONFIG_BCM963158
    uintptr_t xlif_virt_base;
    uintptr_t xlif_phys_base;
    uint32_t xlif_len;
#endif
    
    if (scan_reg_by_name("brcm,rdpa", &xrdp_phys_base, &xrdp_phys_len, 0))
    {
        bdmf_trace("%s: Failed to find reg in device tree", __FUNCTION__);
        return;
    }
    xrdp_virt_base = (uintptr_t)bdmf_ioremap((uint32_t)xrdp_phys_base, xrdp_phys_len);
    bdmf_trace("XRDP virt address : ox%px\n", (void *)xrdp_virt_base);

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880)
    if (scan_reg_by_name("brcm,rdpa", &wan_bbh_phys_base, &wan_bbh_len, 1))
    {
        bdmf_trace("%s: Failed to find reg in device tree", __FUNCTION__);
        return;
    }
    wan_bbh_virt_base = (uintptr_t)bdmf_ioremap(wan_bbh_phys_base, wan_bbh_len);
#endif
   
#ifdef CONFIG_BCM963158

    if (scan_reg_by_name("brcm,rdpa", &xlif_phys_base, &xlif_len, 2))
    {
        bdmf_trace("%s: Failed to find reg in device tree", __FUNCTION__);
        return;
    }

    xlif_virt_base = (uintptr_t)bdmf_ioremap(xlif_phys_base, xlif_len);
#endif

    for (i = 0; RU_ALL_BLOCKS[i]; i++)
    {
#if defined(CONFIG_BCM963158)
        /* Skip the WAN blocks */
        if (RU_ALL_BLOCKS[i]->addr == BBH_TX_ADDRS                ||
            RU_ALL_BLOCKS[i]->addr == BBH_RX_ADDRS                ||
            RU_ALL_BLOCKS[i]->addr == XLIF_RX_IF_ADDRS            ||
            RU_ALL_BLOCKS[i]->addr == XLIF_RX_FLOW_CONTROL_ADDRS  ||
            RU_ALL_BLOCKS[i]->addr == XLIF_TX_IF_ADDRS            ||
            RU_ALL_BLOCKS[i]->addr == XLIF_TX_FLOW_CONTROL_ADDRS  ||
            RU_ALL_BLOCKS[i]->addr == DEBUG_BUS_ADDRS             ||
            RU_ALL_BLOCKS[i]->addr == XLIF_EEE_ADDRS              ||
            RU_ALL_BLOCKS[i]->addr == XLIF_Q_OFF_ADDRS)
            continue;
#endif
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880)
        /* Skip the WAN blocks */
        if (RU_ALL_BLOCKS[i]->addr == BBH_TX_ADDRS || 
            RU_ALL_BLOCKS[i]->addr == BBH_RX_ADDRS)
            continue;
#endif
        
        remap_ru_block_single_instance((ru_block_rec *)RU_ALL_BLOCKS[i], xrdp_virt_base, xrdp_phys_base);
    }
#ifdef CONFIG_BCM96858
    /* go over BBH Tx and BBH Rx and map only XRDP instances */
    for (bbh_idx = BBH_ID_FIRST; bbh_idx < BBH_ID_PON; bbh_idx++)
    {
        BBH_RX_ADDRS[bbh_idx] += xrdp_virt_base - xrdp_phys_base;
        BBH_TX_ADDRS[bbh_idx] += xrdp_virt_base - xrdp_phys_base;
    }
    /* remap BBHWAN */
    BBH_RX_ADDRS[BBH_ID_PON] += wan_bbh_virt_base - wan_bbh_phys_base;
    BBH_TX_ADDRS[BBH_ID_PON] += wan_bbh_virt_base - wan_bbh_phys_base;
#endif
#ifdef CONFIG_BCM963158
    /* go over BBH Tx and BBH Rx and map only XRDP instances */
    for (bbh_idx = BBH_ID_0; bbh_idx <= BBH_ID_2; bbh_idx++)
    {
        BBH_RX_ADDRS[bbh_idx] += xrdp_virt_base - xrdp_phys_base; 
    }
    BBH_TX_ADDRS[BBH_TX_ID_0] += xrdp_virt_base - xrdp_phys_base; 

    /* remap BBHWAN */
    for (bbh_idx = BBH_ID_PON; bbh_idx <= BBH_ID_DSL; bbh_idx++)
    {
        BBH_RX_ADDRS[bbh_idx] += wan_bbh_virt_base - wan_bbh_phys_base;
    }
    for (bbh_idx = BBH_TX_ID_PON; bbh_idx <= BBH_TX_ID_DSL; bbh_idx++)
    {
        BBH_TX_ADDRS[bbh_idx] += wan_bbh_virt_base - wan_bbh_phys_base;
    }

    {
        xlif_id_e xlif_idx;
        /* Map XLIF */
        for (xlif_idx = XLIF_ID_CHANNEL_FIRST; xlif_idx < XLIF_ID_CHANNEL_NUM; xlif_idx++)
        {
            XLIF_RX_IF_ADDRS[xlif_idx]            += xlif_virt_base - xlif_phys_base;
            XLIF_RX_FLOW_CONTROL_ADDRS[xlif_idx]  += xlif_virt_base - xlif_phys_base;
            XLIF_TX_IF_ADDRS[xlif_idx]            += xlif_virt_base - xlif_phys_base;
            XLIF_TX_FLOW_CONTROL_ADDRS[xlif_idx]  += xlif_virt_base - xlif_phys_base;
            DEBUG_BUS_ADDRS[xlif_idx]             += xlif_virt_base - xlif_phys_base;
            XLIF_EEE_ADDRS[xlif_idx]              += xlif_virt_base - xlif_phys_base;
            XLIF_Q_OFF_ADDRS[xlif_idx]            += xlif_virt_base - xlif_phys_base;
        }
    }
#endif

# defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880)
    for (bbh_idx = BBH_ID_0; bbh_idx <= BBH_ID_15; bbh_idx++)
    {
        BBH_RX_ADDRS[bbh_idx] += xrdp_virt_base - xrdp_phys_base; 
    }
    for (bbh_idx = BBH_ID_QM_COPY; bbh_idx <= BBH_TX_ID_LAN2; bbh_idx++)
    {
        BBH_TX_ADDRS[bbh_idx] += xrdp_virt_base - xrdp_phys_base; 
    }
    
    /* remap BBHWAN */
    BBH_RX_ADDRS[BBH_ID_PON] += wan_bbh_virt_base - wan_bbh_phys_base;
    BBH_TX_ADDRS[BBH_TX_ID_PON] += wan_bbh_virt_base - wan_bbh_phys_base;

}


extern unsigned long get_rdp_freq(unsigned int *rdp_freq);

int rdp_get_init_params(void)
{
    uint32_t rnr_size, fpm_pool_size;
    int rc = 0;

    /* fetch the reserved-memory from device tree */
    if (BcmMemReserveGetByName(FPMPOOL_BASE_ADDR_STR, &dpi_params.rdp_ddr_pkt_base_virt, &fpm_pool_size))
    {
        BDMF_TRACE_RET(BDMF_ERR_NOMEM, "Failed to get fpm_pool base address");
    }
    
     
    if (BcmMemReserveGetByName(RNRTBLS_BASE_ADDR_STR, &dpi_params.rdp_ddr_rnr_tables_base_virt, &rnr_size))
    {
        BDMF_TRACE_RET(BDMF_ERR_NOMEM, "Failed to get runner_tables base address");
    }

    /* remap all RU blocks to virtual space */
    remap_ru_blocks();

    if ((FPM_BUF_SIZE_256*FPM_BUF_MAX_BASE_BUFFS) >= RDPA_MTU)
        dpi_params.fpm_buf_size = FPM_BUF_SIZE_256;
    else if ((FPM_BUF_SIZE_512*FPM_BUF_MAX_BASE_BUFFS) >= RDPA_MTU)
        dpi_params.fpm_buf_size = FPM_BUF_SIZE_512;
    else if ((FPM_BUF_SIZE_1K*FPM_BUF_MAX_BASE_BUFFS) >= RDPA_MTU)
        dpi_params.fpm_buf_size = FPM_BUF_SIZE_1K;
    else
        dpi_params.fpm_buf_size = FPM_BUF_SIZE_2K;

    /* non jumbo packets, increase buffer size if more fpm memory available */
    if  (dpi_params.fpm_buf_size == FPM_BUF_SIZE_256)   
    {
        /* for >128M we can allow to increase basic token to 1K */
        if ((fpm_pool_size >> 20) >= 128)
        {
            dpi_params.fpm_buf_size = FPM_BUF_SIZE_2K;
        }
        /* for >64M we can allow to increase basic token to 1K */
        else if ((fpm_pool_size >> 20) >= 64)
        {
            dpi_params.fpm_buf_size = FPM_BUF_SIZE_1K;
        }
        else if ((fpm_pool_size >> 20) >= 32)
        {
            dpi_params.fpm_buf_size = FPM_BUF_SIZE_512;
        }   
    }

    bdmf_trace("fpm base =0x%pK fpm poolsize %d, MTU: %d, token: %d rnr_tables=0x%pK size=%d\n", dpi_params.rdp_ddr_pkt_base_virt, fpm_pool_size, RDPA_MTU,
        dpi_params.fpm_buf_size, dpi_params.rdp_ddr_rnr_tables_base_virt, rnr_size);
    if (fpm_pool_size < dpi_params.fpm_buf_size * TOTAL_FPM_TOKENS)
        BDMF_TRACE_INFO("Number of FPM tokens will be set according to fpm_buf_size %d (0x%X) and fpm pool size %d (0x%X)\n",
           dpi_params.fpm_buf_size, dpi_params.fpm_buf_size, fpm_pool_size, fpm_pool_size);

    rc = get_rdp_freq(&dpi_params.runner_freq);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_NODEV, "Failed to get runner freq %d\n", rc);
    }

    return rc;
}
#endif

extern bbh_id_e rdpa_emac_to_bbh_id_e[rdpa_emac__num_of];
int system_data_path_init(void)
{
    const rdpa_system_init_cfg_t *system_init = _rdpa_system_init_cfg_get();
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();
    int rc;

    rc = rdp_get_init_params();
    if (rc)
    {
        BDMF_TRACE_RET(rc, "Failed to rdp_get_init_params");
    }

    /* TODO: add wan initialization using auto sensing */
    dpi_params.bbh_id_gbe_wan = system_init->gbe_wan_emac != rdpa_emac_none ?
        rdpa_emac_to_bbh_id_e[system_init->gbe_wan_emac] : BBH_ID_NULL;
    dpi_params.max_pkt_size = system_cfg->max_pkt_size;
    dpi_params.headroom_size = system_cfg->headroom_size;
    dpi_params.bcmsw_tag = system_init->runner_ext_sw_cfg.type;

    rc = data_path_init(&dpi_params);
    if (rc)
        BDMF_TRACE_RET(-1, "Failed to data path init\n");
    /*increase UBUS credits for QUAD0 towards MEMC where all CPU Rx/Tx processing is done*/
    ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(0, UBUS_PORT_ID_DDR, 8);
/* XXX: need to implement 6836 */
    return 0;
}

int system_data_path_init_fiber(rdpa_wan_type wan_type)
{
    switch (wan_type)
    {
    case rdpa_wan_gpon:
        data_path_init_fiber(MAC_TYPE_GPON);
        break;
    case rdpa_wan_epon:
        data_path_init_fiber(MAC_TYPE_EPON);
        break;
    case rdpa_wan_xgpon:
        data_path_init_fiber(MAC_TYPE_XGPON);
        break;
    case rdpa_wan_xepon:
        data_path_init_fiber(MAC_TYPE_XEPON);
        break;
    default:
        bdmf_trace("RDPA wan type is not supported");
        return -1;
    }

    return 0;
}

#if defined(BCM63158)
int system_data_path_init_dsl(void)
{
    return data_path_init_dsl(MAC_TYPE_DSL); 
}
#endif

rdpa_bpm_buffer_size_t rdpa_bpm_buffer_size_get(void)
{
    return dpi_params.fpm_buf_size;
}

