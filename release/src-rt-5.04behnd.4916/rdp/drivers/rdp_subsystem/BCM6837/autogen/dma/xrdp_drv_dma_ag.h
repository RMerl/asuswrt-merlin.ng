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

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

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

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} dma_config_clk_gate_cntrl;

typedef struct
{
    uint8_t ddr;
    uint8_t sram;
    bdmf_boolean ddr_set;
    bdmf_boolean sram_set;
    uint16_t dspace_full_thrsh;
    uint16_t hspace_full_thrsh;
    bdmf_boolean ddrch;
} dma_config_ubus_credits;


/**********************************************************************************************************************
 * nempty: 
 *     indication of the queue state
 * urgnt: 
 *     indication whether the queue is in urgent state or not
 * sel_src: 
 *     the next peripheral to be served by the dma
 * address: 
 *     address within the ram
 * datacs: 
 *     chip select for write data ram
 * cdcs: 
 *     chip select for chunk descriptors ram
 * rrcs: 
 *     chip select for read requests ram
 * valid: 
 *     indirect read request is valid
 * ready: 
 *     read data ready
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_info_set(uint8_t dma_id, const dma_debug_info *debug_info);
bdmf_error_t ag_drv_dma_debug_info_get(uint8_t dma_id, dma_debug_info *debug_info);

/**********************************************************************************************************************
 * numofbuff: 
 *     the number of 128bytes buffers allocated to the peripheral.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_num_of_writes_set(uint8_t dma_id, uint8_t emac_index, uint8_t numofbuff);
bdmf_error_t ag_drv_dma_config_num_of_writes_get(uint8_t dma_id, uint8_t emac_index, uint8_t *numofbuff);

/**********************************************************************************************************************
 * rr_num: 
 *     number of read requests
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_num_of_reads_set(uint8_t dma_id, uint8_t emac_index, uint8_t rr_num);
bdmf_error_t ag_drv_dma_config_num_of_reads_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rr_num);

/**********************************************************************************************************************
 * into_u: 
 *     moving into urgent threshold
 * out_of_u: 
 *     moving out ot urgent threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_u_thresh_set(uint8_t dma_id, uint8_t emac_index, uint8_t into_u, uint8_t out_of_u);
bdmf_error_t ag_drv_dma_config_u_thresh_get(uint8_t dma_id, uint8_t emac_index, uint8_t *into_u, uint8_t *out_of_u);

/**********************************************************************************************************************
 * rxpri: 
 *     priority of rx side (upload) of the peripheral
 * txpri: 
 *     priority of tx side (download) of the peripheral
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_pri_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxpri, uint8_t txpri);
bdmf_error_t ag_drv_dma_config_pri_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxpri, uint8_t *txpri);

/**********************************************************************************************************************
 * rxweight: 
 *     weight of rx side (upload) of the peripheral
 * txweight: 
 *     weight of tx side (download) of the peripheral
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_weight_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxweight, uint8_t txweight);
bdmf_error_t ag_drv_dma_config_weight_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxweight, uint8_t *txweight);

/**********************************************************************************************************************
 * rxsource: 
 *     bb source of rx side (upload) of the peripheral
 * txsource: 
 *     bb source of tx side (download) of the peripheral
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_periph_source_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxsource, uint8_t txsource);
bdmf_error_t ag_drv_dma_config_periph_source_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxsource, uint8_t *txsource);

/**********************************************************************************************************************
 * rxtmem: 
 *     target memory of rx side (upload) of the peripheral
 * txtmem: 
 *     target memory of tx side (download) of the peripheral
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_target_mem_set(uint8_t dma_id, uint8_t emac_index, bdmf_boolean rxtmem, bdmf_boolean txtmem);
bdmf_error_t ag_drv_dma_config_target_mem_get(uint8_t dma_id, uint8_t emac_index, bdmf_boolean *rxtmem, bdmf_boolean *txtmem);

/**********************************************************************************************************************
 * rstvec: 
 *     vector in which each bit represents a peripheral.
 *     LSB represent RX peripherals and MSB represent TX peripherals.
 *     When asserted, the relevant FIFOS of the selected peripheral will be reset to zero
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_ptrrst_set(uint8_t dma_id, uint16_t rstvec);
bdmf_error_t ag_drv_dma_config_ptrrst_get(uint8_t dma_id, uint16_t *rstvec);

/**********************************************************************************************************************
 * dest: 
 *     destination ID
 * route: 
 *     the route to be used (override the default route)
 * ovrd: 
 *     override enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_bbrouteovrd_set(uint8_t dma_id, uint8_t dest, uint16_t route, bdmf_boolean ovrd);
bdmf_error_t ag_drv_dma_config_bbrouteovrd_get(uint8_t dma_id, uint8_t *dest, uint16_t *route, bdmf_boolean *ovrd);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 * keep_alive_intrvl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_clk_gate_cntrl_set(uint8_t dma_id, const dma_config_clk_gate_cntrl *config_clk_gate_cntrl);
bdmf_error_t ag_drv_dma_config_clk_gate_cntrl_get(uint8_t dma_id, dma_config_clk_gate_cntrl *config_clk_gate_cntrl);

/**********************************************************************************************************************
 * ddr: 
 *     DDR destination port-id
 * sram: 
 *     SRAM destination port id
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_ubus_dpids_set(uint8_t dma_id, uint8_t ddr, uint8_t sram);
bdmf_error_t ag_drv_dma_config_ubus_dpids_get(uint8_t dma_id, uint8_t *ddr, uint8_t *sram);

/**********************************************************************************************************************
 * max_ddr: 
 *     max on the fly bytes for DDR
 * max_sram: 
 *     max on the fly bytes for SRAM
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_max_otf_set(uint8_t dma_id, uint16_t max_ddr, uint16_t max_sram);
bdmf_error_t ag_drv_dma_config_max_otf_get(uint8_t dma_id, uint16_t *max_ddr, uint16_t *max_sram);

/**********************************************************************************************************************
 * ddr: 
 *     DDR_credits
 * sram: 
 *     SRAM_credits
 * ddr_set: 
 *     set the DMA DDR credits counter to the number in DDR_credits field
 * sram_set: 
 *     set the DMA SRAM credits counter to the number in SRAM_credits field
 * dspace_full_thrsh: 
 *     in ccb mode, when not working with credits, this register determines the minimal number of dspaces to be
 *     considered not full full
 * hspace_full_thrsh: 
 *     in ccb mode, when not working with credits, this register determines the minimal number of hspaces to be
 *     considered not full full
 * ddrch: 
 *     determines the UBUS channel in which the DDR traffic is using in ccb mode
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_ubus_credits_set(uint8_t dma_id, const dma_config_ubus_credits *config_ubus_credits);
bdmf_error_t ag_drv_dma_config_ubus_credits_get(uint8_t dma_id, dma_config_ubus_credits *config_ubus_credits);

/**********************************************************************************************************************
 * base: 
 *     base address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_psram_base_set(uint8_t dma_id, uint32_t base);
bdmf_error_t ag_drv_dma_config_psram_base_get(uint8_t dma_id, uint32_t *base);

/**********************************************************************************************************************
 * base: 
 *     base address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_ddr_base_set(uint8_t dma_id, uint32_t base);
bdmf_error_t ag_drv_dma_config_ddr_base_get(uint8_t dma_id, uint32_t *base);

/**********************************************************************************************************************
 * pair256: 
 *     when asserted the DMA will try to pair 2 read commands of the same client to the DDR. The commands will be
 *     issued back to back if they are to the DDR and if the first is not eop.
 * r: 
 *     default
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_gen_cfg_set(uint8_t dma_id, bdmf_boolean pair256, uint32_t r);
bdmf_error_t ag_drv_dma_config_gen_cfg_get(uint8_t dma_id, bdmf_boolean *pair256, uint32_t *r);

/**********************************************************************************************************************
 * max_ddr: 
 *     max on the fly req for DDR
 * max_sram: 
 *     max on the fly req for SRAM
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_config_max_otf_req_set(uint8_t dma_id, uint8_t max_ddr, uint8_t max_sram);
bdmf_error_t ag_drv_dma_config_max_otf_req_get(uint8_t dma_id, uint8_t *max_ddr, uint8_t *max_sram);

/**********************************************************************************************************************
 * req_cnt: 
 *     the number of pending write requests
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_req_cnt_rx_get(uint8_t dma_id, uint8_t emac_index, uint8_t *req_cnt);

/**********************************************************************************************************************
 * req_cnt: 
 *     the number of pending read requests
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_req_cnt_tx_get(uint8_t dma_id, uint8_t emac_index, uint8_t *req_cnt);

/**********************************************************************************************************************
 * req_cnt: 
 *     the number of pending write requests
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_req_cnt_rx_acc_get(uint8_t dma_id, uint8_t emac_index, uint32_t *req_cnt);

/**********************************************************************************************************************
 * req_cnt: 
 *     the number of pending write requests
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_req_cnt_tx_acc_get(uint8_t dma_id, uint8_t emac_index, uint32_t *req_cnt);

/**********************************************************************************************************************
 * ddr: 
 *     credits
 * sram: 
 *     credits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_ubuscrdt_get(uint8_t dma_id, uint8_t *ddr, uint8_t *sram);

/**********************************************************************************************************************
 * ddr: 
 *     on the fly bytes
 * sram: 
 *     on the fly bytes
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_ubusbytes_get(uint8_t dma_id, uint16_t *ddr, uint16_t *sram);

/**********************************************************************************************************************
 * otf: 
 *     reads on the fly
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_on_the_fly_get(uint8_t dma_id, uint8_t *otf);

/**********************************************************************************************************************
 * dbgsel: 
 *     select
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_dbg_sel_set(uint8_t dma_id, uint8_t dbgsel);
bdmf_error_t ag_drv_dma_debug_dbg_sel_get(uint8_t dma_id, uint8_t *dbgsel);

/**********************************************************************************************************************
 * dbg: 
 *     debug
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_debugout_get(uint8_t dma_id, uint32_t *dbg);

/**********************************************************************************************************************
 * ddr: 
 *     ddr requests
 * sram: 
 *     SRAM requests
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dma_debug_ubusreq_get(uint8_t dma_id, uint8_t *ddr, uint8_t *sram);

/**********************************************************************************************************************
 * data: 
 *     read data from ram
 **********************************************************************************************************************/
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
    cli_dma_config_gen_cfg,
    cli_dma_config_max_otf_req,
    cli_dma_debug_req_cnt_rx,
    cli_dma_debug_req_cnt_tx,
    cli_dma_debug_req_cnt_rx_acc,
    cli_dma_debug_req_cnt_tx_acc,
    cli_dma_debug_ubuscrdt,
    cli_dma_debug_ubusbytes,
    cli_dma_debug_on_the_fly,
    cli_dma_debug_dbg_sel,
    cli_dma_debug_debugout,
    cli_dma_debug_ubusreq,
    cli_dma_debug_rddata,
};

int bcm_dma_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_dma_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
