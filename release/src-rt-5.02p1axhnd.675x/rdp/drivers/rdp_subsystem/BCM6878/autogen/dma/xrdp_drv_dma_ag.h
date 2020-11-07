/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef _XRDP_DRV_DMA_AG_H_
#define _XRDP_DRV_DMA_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* nempty: not_empty_indications - indication of the queue state                                  */
/* urgnt: urgent - indication whether the queue is in urgent state or not                         */
/* sel_src: selected_source - the next peripheral to be served by the dma                         */
/* address: address - address within the ram                                                      */
/* datacs: data_ram_cs - chip select for write data ram                                           */
/* cdcs: cd_ram_cs - chip select for chunk descriptors ram                                        */
/* rrcs: rr_ram_cd - chip select for read requests ram                                            */
/* valid: valid - indirect read request is valid                                                  */
/* ready: ready - read data ready                                                                 */
/**************************************************************************************************/
typedef struct
{
    uint16_t nempty;
    uint16_t urgnt;
    uint8_t sel_src;
    uint16_t address;
    bdmf_boolean datacs;
    bdmf_boolean cdcs;
    bdmf_boolean rrcs;
    bdmf_boolean valid;
    bdmf_boolean ready;
} dma_debug_info;


/**************************************************************************************************/
/* bypass_clk_gate: BYPASS_CLOCK_GATE - If set to 1b1 will disable the clock gate logic such to a */
/*                  lways enable the clock                                                        */
/* timer_val: TIMER_VALUE - For how long should the clock stay active once all conditions for clo */
/*            ck disable are met.                                                                 */
/* keep_alive_en: KEEP_ALIVE_ENABLE - Enables the keep alive logic which will periodically enable */
/*                 the clock to assure that no deadlock of clock being removed completely will oc */
/*                cur                                                                             */
/* keep_alive_intrvl: KEEP_ALIVE_INTERVAL - If the KEEP alive option is enabled the field will de */
/*                    termine for how many cycles should the clock be active                      */
/* keep_alive_cyc: KEEP_ALIVE_CYCLE - If the KEEP alive option is enabled this field will determi */
/*                 ne for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTE */
/*                 RVAL)So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} dma_config_clk_gate_cntrl;

bdmf_error_t ag_drv_dma_debug_info_set(uint8_t dma_id, const dma_debug_info *debug_info);
bdmf_error_t ag_drv_dma_debug_info_get(uint8_t dma_id, dma_debug_info *debug_info);
bdmf_error_t ag_drv_dma_config_num_of_writes_set(uint8_t dma_id, uint8_t emac_index, uint8_t numofbuff);
bdmf_error_t ag_drv_dma_config_num_of_writes_get(uint8_t dma_id, uint8_t emac_index, uint8_t *numofbuff);
bdmf_error_t ag_drv_dma_config_num_of_reads_set(uint8_t dma_id, uint8_t emac_index, uint8_t rr_num);
bdmf_error_t ag_drv_dma_config_num_of_reads_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rr_num);
bdmf_error_t ag_drv_dma_config_u_thresh_set(uint8_t dma_id, uint8_t emac_index, uint8_t into_u, uint8_t out_of_u);
bdmf_error_t ag_drv_dma_config_u_thresh_get(uint8_t dma_id, uint8_t emac_index, uint8_t *into_u, uint8_t *out_of_u);
bdmf_error_t ag_drv_dma_config_pri_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxpri, uint8_t txpri);
bdmf_error_t ag_drv_dma_config_pri_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxpri, uint8_t *txpri);
bdmf_error_t ag_drv_dma_config_weight_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxweight, uint8_t txweight);
bdmf_error_t ag_drv_dma_config_weight_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxweight, uint8_t *txweight);
bdmf_error_t ag_drv_dma_config_periph_source_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxsource, uint8_t txsource);
bdmf_error_t ag_drv_dma_config_periph_source_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxsource, uint8_t *txsource);
bdmf_error_t ag_drv_dma_config_target_mem_set(uint8_t dma_id, uint8_t word_index, bdmf_boolean rxtmem, bdmf_boolean txtmem);
bdmf_error_t ag_drv_dma_config_target_mem_get(uint8_t dma_id, uint8_t word_index, bdmf_boolean *rxtmem, bdmf_boolean *txtmem);
bdmf_error_t ag_drv_dma_config_ptrrst_set(uint8_t dma_id, uint16_t rstvec);
bdmf_error_t ag_drv_dma_config_ptrrst_get(uint8_t dma_id, uint16_t *rstvec);
bdmf_error_t ag_drv_dma_config_bbrouteovrd_set(uint8_t dma_id, uint8_t dest, uint16_t route, bdmf_boolean ovrd);
bdmf_error_t ag_drv_dma_config_bbrouteovrd_get(uint8_t dma_id, uint8_t *dest, uint16_t *route, bdmf_boolean *ovrd);
bdmf_error_t ag_drv_dma_config_clk_gate_cntrl_set(uint8_t dma_id, const dma_config_clk_gate_cntrl *config_clk_gate_cntrl);
bdmf_error_t ag_drv_dma_config_clk_gate_cntrl_get(uint8_t dma_id, dma_config_clk_gate_cntrl *config_clk_gate_cntrl);
bdmf_error_t ag_drv_dma_config_ubus_dpids_set(uint8_t dma_id, uint8_t ddr, uint8_t sram);
bdmf_error_t ag_drv_dma_config_ubus_dpids_get(uint8_t dma_id, uint8_t *ddr, uint8_t *sram);
bdmf_error_t ag_drv_dma_config_max_otf_set(uint8_t dma_id, uint16_t max_ddr, uint16_t max_sram);
bdmf_error_t ag_drv_dma_config_max_otf_get(uint8_t dma_id, uint16_t *max_ddr, uint16_t *max_sram);
bdmf_error_t ag_drv_dma_config_ubus_credits_set(uint8_t dma_id, uint8_t ddr, uint8_t sram, bdmf_boolean ddr_set, bdmf_boolean sram_set);
bdmf_error_t ag_drv_dma_config_ubus_credits_get(uint8_t dma_id, uint8_t *ddr, uint8_t *sram, bdmf_boolean *ddr_set, bdmf_boolean *sram_set);
bdmf_error_t ag_drv_dma_config_psram_base_set(uint8_t dma_id, uint32_t base);
bdmf_error_t ag_drv_dma_config_psram_base_get(uint8_t dma_id, uint32_t *base);
bdmf_error_t ag_drv_dma_config_ddr_base_set(uint8_t dma_id, uint32_t base);
bdmf_error_t ag_drv_dma_config_ddr_base_get(uint8_t dma_id, uint32_t *base);
bdmf_error_t ag_drv_dma_debug_req_cnt_rx_get(uint8_t dma_id, uint8_t emac_index, uint8_t *req_cnt);
bdmf_error_t ag_drv_dma_debug_req_cnt_tx_get(uint8_t dma_id, uint8_t emac_index, uint8_t *req_cnt);
bdmf_error_t ag_drv_dma_debug_req_cnt_rx_acc_get(uint8_t dma_id, uint8_t emac_index, uint32_t *req_cnt);
bdmf_error_t ag_drv_dma_debug_req_cnt_tx_acc_get(uint8_t dma_id, uint8_t emac_index, uint32_t *req_cnt);
bdmf_error_t ag_drv_dma_debug_ubuscrdt_get(uint8_t dma_id, uint8_t *ddr, uint8_t *sram);
bdmf_error_t ag_drv_dma_debug_ubusbytes_get(uint8_t dma_id, uint16_t *ddr, uint16_t *sram);
bdmf_error_t ag_drv_dma_debug_on_the_fly_get(uint8_t dma_id, uint8_t *otf);
bdmf_error_t ag_drv_dma_debug_dbg_sel_set(uint8_t dma_id, uint8_t dbgsel);
bdmf_error_t ag_drv_dma_debug_dbg_sel_get(uint8_t dma_id, uint8_t *dbgsel);
bdmf_error_t ag_drv_dma_debug_debugout_get(uint8_t dma_id, uint32_t *dbg);
bdmf_error_t ag_drv_dma_debug_rddata_get(uint8_t dma_id, uint8_t word_index, uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_dma_debug_info,
    cli_dma_config_num_of_writes,
    cli_dma_config_num_of_reads,
    cli_dma_config_u_thresh,
    cli_dma_config_pri,
    cli_dma_config_weight,
    cli_dma_config_periph_source,
    cli_dma_config_target_mem,
    cli_dma_config_ptrrst,
    cli_dma_config_bbrouteovrd,
    cli_dma_config_clk_gate_cntrl,
    cli_dma_config_ubus_dpids,
    cli_dma_config_max_otf,
    cli_dma_config_ubus_credits,
    cli_dma_config_psram_base,
    cli_dma_config_ddr_base,
    cli_dma_debug_req_cnt_rx,
    cli_dma_debug_req_cnt_tx,
    cli_dma_debug_req_cnt_rx_acc,
    cli_dma_debug_req_cnt_tx_acc,
    cli_dma_debug_ubuscrdt,
    cli_dma_debug_ubusbytes,
    cli_dma_debug_on_the_fly,
    cli_dma_debug_dbg_sel,
    cli_dma_debug_debugout,
    cli_dma_debug_rddata,
};

int bcm_dma_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_dma_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

