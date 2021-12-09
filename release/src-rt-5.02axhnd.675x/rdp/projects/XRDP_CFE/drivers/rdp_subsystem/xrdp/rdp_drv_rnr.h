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

#ifndef DRV_RNR_H_INCLUDED
#define DRV_RNR_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "rdd_runner_proj_defs.h"
#include "xrdp_drv_rnr_quad_ag.h"
#include "xrdp_drv_rnr_inst_ag.h"
#include "xrdp_drv_rnr_mem_ag.h"
#include "xrdp_drv_rnr_pred_ag.h"
#include "xrdp_drv_rnr_cntxt_ag.h"
#include "xrdp_drv_rnr_regs_ag.h"
#include "rdpa_types.h"

typedef enum
{
    DRV_RNR_2SP_14RR = 0,
    DRV_RNR_4SP_12RR = 1,
    DRV_RNR_8SP_8RR = 2,
    DRV_RNR_16RR = 3,
    DRV_RNR_16SP = 4,
} drv_rnr_sch_mode_t;

typedef enum
{
    DRV_PARSER_QTAG_PROFILE_0 = 0,
    DRV_PARSER_QTAG_PROFILE_1 = 1,
    DRV_PARSER_QTAG_PROFILE_2 = 2,
    DRV_PARSER_NUM_OF_QTAG_PROFILE = 3,
} drv_parser_qtag_profile_t;

typedef enum
{
    PROP_TAG_SIZE_0 = 0,
    PROP_TAG_SIZE_2 = 2,
    PROP_TAG_SIZE_4 = 4,
    PROP_TAG_SIZE_6 = 6,
    PROP_TAG_SIZE_8 = 8,
} drv_rnr_prop_tag_size_t;

typedef enum
{
    RNR_PROFILING_IDLE_CYCLES_COUNTER,
    RNR_PROFILING_IDLE_PWRSAVE_MODE_COUNTER,
} drv_rnr_profiling_idle_counter_t;

typedef enum
{
	BRIDGE_DA_MAC_FILTER_GROUP = 0,
	ROUTER_DA_MAC_FILTER_GROUP = 1,
	DA_MAC_FILTER_GROUP_SIZE   = 2,
} drv_rnr_da_flt_grp_t;

/* qtag mask */
#define DRV_PARSER_OUTER_QTAG_USER_OUTER_BIT        0
#define DRV_PARSER_OUTER_QTAG_8100_OUTER_BIT        0
#define DRV_PARSER_OUTER_QTAG_88A8_OUTER_BIT        3
#define DRV_PARSER_OUTER_QTAG_9100_OUTER_BIT        6
#define DRV_PARSER_OUTER_QTAG_9200_OUTER_BIT        9
#define DRV_PARSER_OUTER_QTAG_USER_INNER_BIT        1
#define DRV_PARSER_OUTER_QTAG_8100_INNER_BIT        1
#define DRV_PARSER_OUTER_QTAG_88A8_INNER_BIT        4
#define DRV_PARSER_OUTER_QTAG_9100_INNER_BIT        7
#define DRV_PARSER_OUTER_QTAG_9200_INNER_BIT        10
#define DRV_PARSER_OUTER_QTAG_USER_3RD_BIT          2
#define DRV_PARSER_OUTER_QTAG_8100_3RD_BIT          2
#define DRV_PARSER_OUTER_QTAG_88A8_3RD_BIT          5
#define DRV_PARSER_OUTER_QTAG_9100_3RD_BIT          8
#define DRV_PARSER_OUTER_QTAG_9200_3RD_BIT          11
#define MAX_NUM_OF_PROFILES                         3
#define NUM_OF_RNR_QUAD                             NUM_OF_RNR_QUADS

#define PARSER_ENG_TRIPLE_TAG_DETECTION_BIT_SHIFT   8
/* number of bytes in MAC address */
#define PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS       6
#define RNR_PROFILING_DEFAULT_COUNTER_LSB           0
#define RNR_PROFILING_DEFAULT_CYCLE_NUM             0xFFFF /* maximum possible */

/* gets bit #i from a given number */
#define MS_GET_BIT_I(number, i)   (((1 << (i)) & (number)) >> (i))
/* sets bit #i of a given number to a given value */
#define MS_SET_BIT_I(number, i, bit_value)   ((number) &= (~(1 << (i))), (number) |= ((bit_value) << (i)))

#define QUAD_TO_PROFILING_CORE_ID(quad_id) (quad_id * 4)

#define drv_rnr_quad_parser_da_filter_set(num, a, b, c) ag_drv_rnr_quad_parser_da_filter## #num ##_set(a,b,c)

typedef struct
{
    uint32_t dma_base;
    uint8_t dma_buf_size;
    uint8_t dma_static_offset;
} rnr_dma_cfg_t;

typedef struct
{
    rnr_dma_cfg_t psram;
    rnr_dma_cfg_t ddr;
} rnr_dma_regs_cfg_t;

extern uintptr_t rdp_runner_core_addr[];

int drv_rnr_dma_cfg(rnr_dma_regs_cfg_t *rnr_dma_cfg);
void drv_rnr_cores_addr_init(void);
void drv_rnr_mem_init(void);
void drv_rnr_load_microcode(void);
void drv_rnr_load_prediction(void);
void drv_rnr_set_sch_cfg(void);
void rdp_rnr_write_context(void *__to, void *__from, unsigned int __n);
bdmf_error_t drv_rnr_quad_parser_configure_inner_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean inner_en, rdpa_tpid_detect_t etype);
bdmf_error_t drv_rnr_quad_parser_configure_outer_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean outer_en, rdpa_tpid_detect_t etype);
bdmf_error_t drv_rnr_quad_parser_configure_3rd_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean inner_en, rdpa_tpid_detect_t etype);
void parser_mac_address_array_to_hw_format(uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS],
    uint32_t *address_4_ls_bytes, uint16_t *addres_2_ms_bytes);
void parser_mac_address_hw_format_to_array(uint32_t address_4_ls_bytes,
    uint16_t addres_2_ms_bytes, uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS]);
bdmf_error_t drv_rnr_quad_parser_da_filter_without_mask_set(uint8_t *mac_address, bdmf_boolean add);
bdmf_error_t drv_rnr_quad_parser_da_filter_valid_cfg(rnr_quad_id_e quad_id, uint8_t filter_index, uint8_t enable);
void drv_rnr_profiling_clear_trace(uint8_t core_id);
void drv_rnr_quad_profiling_quad_init(rnr_quad_id_e quad_id);
#if !defined(RDP_SIM)
int  drv_rnr_quad_ubus_decode_wnd_cfg(uint32_t win, uint32_t phys_addr, uint32_t size_power_of_2, int port_id, unsigned int cache_bit_en);
#endif
int drv_rnr_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val);
#ifdef USE_BDMF_SHELL
int drv_rnr_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_quad_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_parser_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_rnr_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_rnr_cli_config_trace(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_rnr_cli_init(bdmfmon_handle_t driver_dir);
void drv_rnr_cli_exit(bdmfmon_handle_t driver_dir);
#endif

void drv_rnr_num_of_cores_set(int num_of_cores);

struct xrdp_rnr_profiling_cfg {
    char enable_quads[NUM_OF_RNR_QUAD];
    uint32_t num_cycles;
};

struct xrdp_rnr_profiling_res {
    uint32_t idle_cnts[NUM_OF_RUNNER_CORES];
    uint32_t total_cnt;
    char profiling_on;
};

int xrdp_rnr_profiling_set_config(struct xrdp_rnr_profiling_cfg *cfg);
int xrdp_rnr_profiling_get_result(struct xrdp_rnr_profiling_res *res);

#ifdef __cplusplus
}
#endif

#endif
