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

#ifndef _XRDP_DRV_NATC_DDR_CFG_AG_H_
#define _XRDP_DRV_NATC_DDR_CFG_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* ddr_size_tbl0:  - Number of entries in DDR table 0Value of 6 or above is invalidTo compute the */
/*                 actual size of the table, add DDR_BINS_PER_BUCKET fieldto the table size selec */
/*                tion;For instance, if DDR_BINS_PER_BUCKET is 3 (4 bins per bucket)and DDR_size  */
/*                is 3 (64k entries), the actual size of the table in DDR is(64*1024+3) multiply  */
/*                by total length (TOTAL_LEN) of key and context in bytesExtra 3 entries are used */
/*                 to store collided entries of the last entryvalue 256k 5256k entriesvalue 128k  */
/*                4128k entriesvalue 64k 364k entriesvalue 32k 232k entriesvalue 16k 116k entries */
/*                value 8k 08k entriesdefault 0h                                                  */
/* ddr_size_tbl1:  - Number of entries in DDR table 10=8k; 1=16k; 2=32k; 3=64k; 4=128k; 5=256k; 6 */
/*                =invalid; 7=invalidSee description of DDR_SIZE_TBL0                             */
/* ddr_size_tbl2:  - Number of entries in DDR table 2See description of DDR_SIZE_TBL00=8k; 1=16k; */
/*                 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid                             */
/* ddr_size_tbl3:  - Number of entries in DDR table 3See description of DDR_SIZE_TBL00=8k; 1=16k; */
/*                 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid                             */
/* ddr_size_tbl4:  - Number of entries in DDR table 4See description of DDR_SIZE_TBL00=8k; 1=16k; */
/*                 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid                             */
/* ddr_size_tbl5:  - Number of entries in DDR table 5See description of DDR_SIZE_TBL00=8k; 1=16k; */
/*                 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid                             */
/* ddr_size_tbl6:  - Number of entries in DDR table 6See description of DDR_SIZE_TBL00=8k; 1=16k; */
/*                 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid                             */
/* ddr_size_tbl7:  - Number of entries in DDR table 7See description of DDR_SIZE_TBL00=8k; 1=16k; */
/*                 2=32k; 3=64k; 4=128k; 5=256k; 6=invalid; 7=invalid                             */
/**************************************************************************************************/
typedef struct
{
    uint8_t ddr_size_tbl0;
    uint8_t ddr_size_tbl1;
    uint8_t ddr_size_tbl2;
    uint8_t ddr_size_tbl3;
    uint8_t ddr_size_tbl4;
    uint8_t ddr_size_tbl5;
    uint8_t ddr_size_tbl6;
    uint8_t ddr_size_tbl7;
} natc_ddr_cfg_natc_ddr_size;


/**************************************************************************************************/
/* total_len_tbl7:  - Length of the lookup key plus context (including 8-byte counters) in DDR ta */
/*                 ble 7See description of TOTAL_LEN_TBL0.                                        */
/* total_len_tbl6:  - Length of the lookup key plus context (including 8-byte counters) in DDR ta */
/*                 ble 6See description of TOTAL_LEN_TBL0.                                        */
/* total_len_tbl5:  - Length of the lookup key plus context (including 8-byte counters) in DDR ta */
/*                 ble 5See description of TOTAL_LEN_TBL0.                                        */
/* total_len_tbl4:  - Length of the lookup key plus context (including 8-byte counters) in DDR ta */
/*                 ble 4See description of TOTAL_LEN_TBL0.                                        */
/* total_len_tbl3:  - Length of the lookup key plus context (including 8-byte counters) in DDR ta */
/*                 ble 3See description of TOTAL_LEN_TBL0.                                        */
/* total_len_tbl2:  - Length of the lookup key plus context (including 8-byte counters) in DDR ta */
/*                 ble 2See description of TOTAL_LEN_TBL0.                                        */
/* total_len_tbl1:  - Length of the lookup key plus context (including 8-byte counters) in DDR ta */
/*                 ble 1See description of TOTAL_LEN_TBL0.                                        */
/* total_len_tbl0:  - Length of the lookup key plus context (including 8-byte counters) in DDR ta */
/*                 ble 0The context length (including 8-byte counters) is calculated by TOTAL_LEN */
/*                  minus KEY_LENThe maximum value should not exceed hardware capability.For inst */
/*                 ance, most projects have max of 80-bytes and BCM63158 has max value of 144-byt */
/*                 e.0h: 48-byte1h: 64-byte2h: 80-byte3h: 96-byte4h: 112-byte5h: 128-byte6h: 144- */
/*                 byte7h: 160-byte                                                               */
/**************************************************************************************************/
typedef struct
{
    uint8_t total_len_tbl7;
    uint8_t total_len_tbl6;
    uint8_t total_len_tbl5;
    uint8_t total_len_tbl4;
    uint8_t total_len_tbl3;
    uint8_t total_len_tbl2;
    uint8_t total_len_tbl1;
    uint8_t total_len_tbl0;
} natc_ddr_cfg_total_len;


/**************************************************************************************************/
/* debug_sel:  - Debug bus select.2'b00: prb_nat_control.2'b01: prb_cmd_control.2'b10: prb_wb_con */
/*            trol.2'b11: prb_ddr_control.                                                        */
/* apb_state:  - APB to RBUS bridge state machine.2'b00: APB_ST_IDLE.2'b01: APB_ST_RW.2'b10: AOB_ */
/*            ST_END.                                                                             */
/* ddr_req_state:  - DDR request state machine.2'b00: DDR_REQ_ST_IDLE.2'b01: DDR_REQ_ST_WRITE_HEA */
/*                DER.2'b10: DDR_REQ_ST_WRITE_HEADER_DELAY.                                       */
/* ddr_rep_state:  - DDR reply state machine.3'b000: DDR_REP_ST_IDLE.3'b001: DDR_REP_ST_READ_DATA */
/*                .3'b010: DDR_REP_ST_READ_RESULT.3'b011: DDR_REP_ST_READ_WAIT.3'b100: DDR_REP_ST */
/*                _EVICT_WR_NON_CACHEABLE.                                                        */
/* runner_cmd_state:  - Runner command state machine.1'b0: RUNNER_CMD_ST_IDLE.1'b1: RUNNER_CMD_ST */
/*                   _WRITE_RUNNER_FIFO.                                                          */
/* wb_state:  - Write-back state machine.1'b0: WB_ST_IDLE.1'b1: WB_ST_WRITE_BACIF.                */
/* nat_state:  - Nat state machine.15'b000000000000000: NAT_ST_IDLE.15'b000000000000001: NAT_ST_I */
/*            DLE_WRITE_SMEM.15'b000000000000010: NAT_ST_IDLE_DDR_PENDING.15'b000000000000100: NA */
/*            T_ST_HASH.15'b000000000001000: NAT_ST_NAT_MEM_READ_REQ.15'b000000000010000: NAT_ST_ */
/*            NAT_MEM_WRITE_REQ.15'b000000000100000: NAT_ST_READ_SMEM.15'b000000001000000: NAT_ST */
/*            _UPDATE_DDR.15'b000000010000000: NAT_ST_IDLE_BLOCKING_PENDING.15'b000000100000000:  */
/*            NAT_ST_EVICT_WAIT.15'b000001000000000: NAT_ST_CHECK_NON_CACHEABLE.15'b0000100000000 */
/*            00: NAT_ST_WAIT.15'b000100000000000: NAT_ST_WAIT_NATC_MEM_REQ_DONE.15'b001000000000 */
/*            000: NAT_ST_CACHE_FLUSH.15'b010000000000000: NAT_ST_DDR_MISS_0.15'b100000000000000: */
/*             NAT_ST_DDR_MISS_1.                                                                 */
/**************************************************************************************************/
typedef struct
{
    uint8_t debug_sel;
    uint8_t apb_state;
    uint8_t ddr_req_state;
    uint8_t ddr_rep_state;
    bdmf_boolean runner_cmd_state;
    bdmf_boolean wb_state;
    uint16_t nat_state;
} natc_ddr_cfg_sm_status;

bdmf_error_t ag_drv_natc_ddr_cfg_natc_ddr_size_set(const natc_ddr_cfg_natc_ddr_size *natc_ddr_size);
bdmf_error_t ag_drv_natc_ddr_cfg_natc_ddr_size_get(natc_ddr_cfg_natc_ddr_size *natc_ddr_size);
bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(uint8_t ddr_bins_per_bucket_tbl3, uint8_t ddr_bins_per_bucket_tbl2, uint8_t ddr_bins_per_bucket_tbl1, uint8_t ddr_bins_per_bucket_tbl0);
bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_get(uint8_t *ddr_bins_per_bucket_tbl3, uint8_t *ddr_bins_per_bucket_tbl2, uint8_t *ddr_bins_per_bucket_tbl1, uint8_t *ddr_bins_per_bucket_tbl0);
bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(uint8_t ddr_bins_per_bucket_tbl7, uint8_t ddr_bins_per_bucket_tbl6, uint8_t ddr_bins_per_bucket_tbl5, uint8_t ddr_bins_per_bucket_tbl4);
bdmf_error_t ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_get(uint8_t *ddr_bins_per_bucket_tbl7, uint8_t *ddr_bins_per_bucket_tbl6, uint8_t *ddr_bins_per_bucket_tbl5, uint8_t *ddr_bins_per_bucket_tbl4);
bdmf_error_t ag_drv_natc_ddr_cfg_total_len_set(const natc_ddr_cfg_total_len *total_len);
bdmf_error_t ag_drv_natc_ddr_cfg_total_len_get(natc_ddr_cfg_total_len *total_len);
bdmf_error_t ag_drv_natc_ddr_cfg_sm_status_set(const natc_ddr_cfg_sm_status *sm_status);
bdmf_error_t ag_drv_natc_ddr_cfg_sm_status_get(natc_ddr_cfg_sm_status *sm_status);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_ddr_cfg_natc_ddr_size,
    cli_natc_ddr_cfg_ddr_bins_per_bucket_0,
    cli_natc_ddr_cfg_ddr_bins_per_bucket_1,
    cli_natc_ddr_cfg_total_len,
    cli_natc_ddr_cfg_sm_status,
};

int bcm_natc_ddr_cfg_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_ddr_cfg_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

