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

#ifndef DRV_BBH_TX_H_INCLUDED
#define DRV_BBH_TX_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "xrdp_drv_bbh_tx_ag.h"

/* DS DEFS */
#define BBH_TX_DS_PD_FIFO_SIZE_0 8
#define BBH_TX_DS_PD_FIFO_SIZE_1 8

/*GPON DEFS*/
/* size of each one of FIFOs 0-7 - value is size + 1*/
#define BBH_TX_GPON_PD_FIFO_SIZE_0_7 11
#define BBH_TX_GPON_PD_FIFO_SIZE_8_15 11
#define BBH_TX_GPON_PD_FIFO_SIZE_16_23 11
#define BBH_TX_GPON_PD_FIFO_SIZE_24_31 11
#define BBH_TX_GPON_PD_FIFO_SIZE_32_39 11

/*EPON DEFS*/
#define BBH_TX_EPON_PD_FIFO_SIZE_0_7 35
/* size of each one of FIFOs 8-15 */
#define BBH_TX_EPON_PD_FIFO_SIZE_8_15 0
#define BBH_TX_EPON_PD_FIFO_SIZE_16_23 0
#define BBH_TX_EPON_PD_FIFO_SIZE_24_31 0
#define BBH_TX_EPON_PD_FIFO_SIZE_32_39 0

/* size of each one of FIFOs 0-7 is 2.5K - value is /8 */
#define BBH_TX_LAN_FE_FIFO_SIZE 319
#define BBH_TX_LAN_FE_FIFO_BASE_0 0
#define BBH_TX_LAN_FE_FIFO_BASE_1 (BBH_TX_LAN_FE_FIFO_BASE_0 + BBH_TX_LAN_FE_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_FIFO_BASE_2 (BBH_TX_LAN_FE_FIFO_BASE_1 + BBH_TX_LAN_FE_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_FIFO_BASE_3 (BBH_TX_LAN_FE_FIFO_BASE_2 + BBH_TX_LAN_FE_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_FIFO_BASE_4 (BBH_TX_LAN_FE_FIFO_BASE_3 + BBH_TX_LAN_FE_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_FIFO_BASE_5 (BBH_TX_LAN_FE_FIFO_BASE_4 + BBH_TX_LAN_FE_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_FIFO_BASE_6 0
#define BBH_TX_LAN_FE_FIFO_BASE_7 0

/* size of each one of FIFOs 0-7 is 40 PDs */
#define BBH_TX_LAN_FE_PD_FIFO_SIZE 39
#define BBH_TX_LAN_FE_PD_FIFO_BASE_0 0
#define BBH_TX_LAN_FE_PD_FIFO_BASE_1 (BBH_TX_LAN_FE_PD_FIFO_BASE_0 + BBH_TX_LAN_FE_PD_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_PD_FIFO_BASE_2 (BBH_TX_LAN_FE_PD_FIFO_BASE_1 + BBH_TX_LAN_FE_PD_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_PD_FIFO_BASE_3 (BBH_TX_LAN_FE_PD_FIFO_BASE_2 + BBH_TX_LAN_FE_PD_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_PD_FIFO_BASE_4 (BBH_TX_LAN_FE_PD_FIFO_BASE_3 + BBH_TX_LAN_FE_PD_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_PD_FIFO_BASE_5 (BBH_TX_LAN_FE_PD_FIFO_BASE_4 + BBH_TX_LAN_FE_PD_FIFO_SIZE + 1)
#define BBH_TX_LAN_FE_PD_FIFO_BASE_6 0
#define BBH_TX_LAN_FE_PD_FIFO_BASE_7 0

/* Maximal PD prefetch byte threshold, in 32 byte resolution */
#define DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE               ( 4095 )

#define TX_QEUEU_PAIRS 20
#define LAN_QEUEU_PAIRS 4
/* TX threshold should be 2K. value is in 8B resolution */
#define LAN_TX_THRESHOLD 256 

typedef struct
{
    bdmf_boolean q0;
    bdmf_boolean q1;
} queue_to_rnr_t;

typedef struct
{
    uint16_t base0;
    uint16_t base1;
} pd_fifo_base_t;

typedef struct
{
    uint16_t size0;
    uint16_t size1;
} pd_fifo_size_t;

typedef struct
{
    uint8_t threshold0;
    uint8_t threshold1;
} pd_wkup_threshold_t;

typedef struct
{
    uint16_t threshold0;
    uint16_t threshold1;
} pd_bytes_threshold_t;

typedef struct
{
    uint8_t  rnr_src_id;
    uint16_t tcont_addr;
    uint16_t skb_addr;
    uint16_t ptr_addr;
    uint8_t  task_number;
} rnr_cfg_t;

typedef struct
{
    queue_to_rnr_t *queue_to_rnr;
    pd_fifo_base_t *pd_fifo_base;
    pd_fifo_size_t *pd_fifo_size;
    pd_fifo_base_t *fe_fifo_base;
    pd_fifo_size_t *fe_fifo_size;
    pd_fifo_base_t *fe_pd_fifo_base;
    pd_fifo_size_t *fe_pd_fifo_size;
    pd_wkup_threshold_t *pd_wkup_threshold;
    pd_bytes_threshold_t *pd_bytes_threshold;
    bdmf_boolean pd_bytes_threshold_en;
    uint8_t pd_empty_threshold;
} pd_queue_cfg_t;

typedef struct
{
    bdmf_boolean stop_len_err;
    bdmf_boolean cmp_width;
} epon_cfg_t;


typedef struct
{
    uint8_t mac_type;
    bbh_tx_cfg_src_id src_id;
    bbh_tx_bbh_dma_cfg *dma_cfg;
    bbh_tx_bbh_sdma_cfg *sdma_cfg;
    bbh_tx_bbh_ddr_cfg *ddr_cfg;
    bbh_tx_common_configurations_ddrtmbasel base_addr_low;
    bbh_tx_common_configurations_ddrtmbaseh base_addr_high;
    rnr_cfg_t rnr_cfg[2];
    rnr_cfg_t sts_rnr_cfg[2];
    rnr_cfg_t msg_rnr_cfg[2];
    pd_queue_cfg_t wan_queue_cfg;
    pd_queue_cfg_t lan_queue_cfg;
} bbh_tx_config;

/******************************************************************************/
/*                                                                            */
/* Functions prototypes                                                       */
/*                                                                            */
/******************************************************************************/
int drv_bbh_tx_configuration_set(uint8_t bbh_id, bbh_tx_config *config);
int drv_bbh_tx_configuration_get(uint8_t bbh_id, bbh_tx_config *config);
int drv_bbh_tx_wan_queue_cfg_set(uint8_t bbh_id, pd_queue_cfg_t *wan_queue_cfg);

#ifdef USE_BDMF_SHELL
int drv_bbh_tx_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_bbh_tx_cli_init(bdmfmon_handle_t driver_dir);
void drv_bbh_tx_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#ifdef __cplusplus
}
#endif

#endif
