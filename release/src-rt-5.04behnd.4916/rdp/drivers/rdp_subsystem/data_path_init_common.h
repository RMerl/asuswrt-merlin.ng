/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

#ifndef __DATA_PATH_INIT_COMMON__
#define __DATA_PATH_INIT_COMMON__

#include "bdmf_data_types.h"
#include "rdp_drv_dis_reor.h"

#if CHIP_VER >= RDP_GEN_62
#include "xrdp_drv_xumac_rdp_ag.h"

/*UBUS_SLV changed to UBUS_RESP */
#define ag_drv_ubus_slv_rnr_intr_ctrl_ier_get ag_drv_ubus_resp_rnr_intr_ctrl_ier_get
#define ag_drv_ubus_slv_rnr_intr_ctrl_ier_set ag_drv_ubus_resp_rnr_intr_ctrl_ier_set
#define ag_drv_ubus_slv_profiling_status_get ag_drv_ubus_resp_profiling_status_get
#define ag_drv_ubus_slv_profiling_cycle_num_set ag_drv_ubus_resp_profiling_cycle_num_set
#define ag_drv_ubus_slv_profiling_cfg_set ag_drv_ubus_resp_profiling_cfg_set
#define ag_drv_ubus_slv_rnr_intr_ctrl_isr_set ag_drv_ubus_resp_rnr_intr_ctrl_isr_set
#define ag_drv_ubus_slv_rnr_intr_ctrl_isr_get ag_drv_ubus_resp_rnr_intr_ctrl_isr_get
#define UBUS_SLV_BLOCK UBUS_RESP_BLOCK
#define UBUS_SLV_RNR_INTR_CTRL_ITR_REG_OFFSET UBUS_RESP_RNR_INTR_CTRL_ITR_REG_OFFSET
#define UBUS_SLV_RNR_INTR_CTRL_ITR_REG UBUS_RESP_RNR_INTR_CTRL_ITR_REG

/*unimac changed to xumac */
typedef xumac_rdp_command_config unimac_rdp_command_config;
#define ag_drv_unimac_rdp_command_config_get ag_drv_xumac_rdp_command_config_get
#define ag_drv_unimac_rdp_command_config_set ag_drv_xumac_rdp_command_config_set

#endif

#if (defined(CONFIG_BRCM_QEMU) && (defined(BCM4912) || defined(BCM6813) || defined(BCM63146)))
    extern volatile uint32_t * dpi_psimtime;
    #define xrdp_usleep(_a) { uint32_t ctime = *dpi_psimtime + 1 + _a; \
                              while (ctime > *dpi_psimtime) {} }
    #define DPI_TRACE(fmt, args...) bdmf_trace("%s#%d  Time: %dus - " fmt, __FUNCTION__, __LINE__, *dpi_psimtime, args)
#else
    #define xrdp_usleep(_a) bdmf_usleep(_a)
    #define xrdp_msleep(_a) bdmf_msleep(_a)
    #define DPI_TRACE(fmt, args...)
#endif

#define xrdp_memset memset
#define xrdp_memcpy memcpy
#define xrdp_alloc(_a) bdmf_alloc(_a)

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

/* includes */
#include "bdmf_data_types.h"
#include "rdp_common.h"
#include "rdp_platform.h"
#include "rdd_init.h"

/* functions */
int data_path_init(dpi_params_t *dpi_params);
int data_path_init_basic(dpi_params_t *dpi_params);

void dispatcher_reorder_viq_init(dsptchr_config *cfg, dsptchr_cngs_params *ingress_congs_init,
    dsptchr_cngs_params *egress_congs_init, uint8_t bb_id, uint32_t target_address,
    bdmf_boolean dest, bdmf_boolean delayed, uint8_t viq_num, uint8_t guaranteed_limit,
	uint16_t common_max_limit, bdmf_boolean is_bbh_queue);

/* BBH RX helpers */
int bbh_rx_cfg(bbh_id_e bbh_id, ports_profile_t *port_profiles);
typedef int (*bbh_rx_init_skip_check_cb_t)(bbh_id_e bbh_id);
int bbh_rx_init(bbh_id_e bbh_id_first, bbh_id_e bbh_id_last_plus_one, bbh_rx_init_skip_check_cb_t skip_bbh_rx_init_cb,
    ports_profile_t *port_profiles);

/* NATC helpers */
uint32_t data_path_init_natc_tbl_en_mask_get(void);
int data_path_natc_init(dpi_params_t *p_dpi_cfg);

typedef struct {
    int is_basic;

    /* rdd data structure params */
    uint32_t iptv_hw_fpm_addr_offset;
    uint32_t hnsize0;
    uint32_t hnsize1;

    /* parser configuration params */
    int parser_has_prop_tag;

    /* runner core params */
    uint16_t rnr_freq;

    /* psram */
    bdmf_phys_addr_t psram_dma_base_phys_addr;
} runner_common_init_cfg_t;

int data_path_runner_common_init(runner_common_init_cfg_t *cfg, dpi_params_t *p_dpi_cfg);

/* call SMC to turn on rnr cores*/
void data_path_init_pwr_up(void);

uint32_t runner_enable(void);

#endif
