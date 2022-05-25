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

#ifndef _DATA_PATH_INIT_
#define _DATA_PATH_INIT_

#ifdef __cplusplus 
extern "C" {
#endif

/* includes */
#include "rdpa_types.h"
#include "access_macros.h"
#include "packing.h"
#include "rdp_drv_bbh_tx.h"
#include "rdp_drv_bbh_rx.h"
#include "data_path_init_basic.h"

/* default value for DISPATCHER */
#define DISP_REOR_XRDP_VIQ_EN(viq_num)      ((((1 << viq_num) - 1) & ~(1 << BBH_ID_PON)) & ~(1 << (BBH_ID_PON + BBH_ID_NUM)))
#define DISP_REOR_WAN_BBH_VIQ_EN(viq_en)    ((viq_en | (1 << BBH_ID_PON)) | (1 << (BBH_ID_PON + BBH_ID_NUM)))

/* default values for FPM */
#define FPM_POOL_BASE_ADDRESS               0
#define FPM_CLEAR_IRQ_STATUS                0xffffffff
#define FPM_DDR_0_POOL_ID                   0
#define FPM_ALLOC_WEIGHT                    1
#define FPM_FREE_WEIGHT                     1
#define FPM_POLL_SLEEP_TIME                 200
#ifdef RDP_SIM
#define FPM_INIT_TIMEOUT                    0x0000
#else
#define FPM_INIT_TIMEOUT                    0xf000
#endif
#define FPM_INTERRUPT_STATUS                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
#define FPM_INTERRUPT_MASK                  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
#define FPM_XON_THRESHOLD                   512
#define FPM_XOFF_THRESHOLD                  576

#define FPM_RES_XEPON                       0
#define FPM_RES_WLAN                        1

#define NUM_OF_FPM_UG                       4
#define TOTAL_FPM_TOKENS                    ((64*1024) - 1)

#define DS_FPM_UG_DEFAULT                   (20*1024)
#define US_FPM_UG_DEFAULT                   (40*1024)
#define WLAN_FPM_UG_DEFAULT                 (0*1024)

#define DS_FPM_UG_NO_XEPON                  (8*1024)
#define US_FPM_UG_NO_XEPON                  (24*1024)
#define WLAN_FPM_UG_NO_XEPON                (30*1024)

#define DS_FPM_UG_XEPON                     (8*1024)
#define US_FPM_UG_XEPON                     (38*1024)
#define WLAN_FPM_UG_XEPON                   (16*1024)


/* default value for BBH-RX */
#define SOP_OFFSET 0
#define MIN_ETH_PKT_SIZE  64
#define MIN_WAN_PKT_SIZE  64
#define MIN_OMCI_PKT_SIZE 16
#define BBH_RX_FLOWS_32_255_GROUP_DIVIDER 255
#define EPON_CONTROL_PATTERN_MSB        0
#define EPON_CONTROL_PATTERN_LSB        0x88080000
#define EPON_CONTROL_PATTERN_MASK_MSB   0
#define EPON_CONTROL_PATTERN_MASK_LSB   0xfffe0000

/* Common to both BBH-RX and SDMA */
#define BBH_RX_TOTAL_BUFFER_NUM  32
#define BBH_TX_TOTAL_BUFFER_NUM  64
#define NUM_OF_DMA 1
#define NUM_OF_DMA_SDMA 1
#define NUM_OF_UNIMAC 5

#define RNR0 0
#define RNR1 1
#define MAX_OTF_READ_REQUEST_DEFAULT  16
#define DMA_PSRAM_BYTES_OTF  0xffff
#define DMA_DDR_BYTES_OTF  1500 
#define PERIPHERALS_PER_DMA 8
#define PSRAM1_BASE_OFFSET 0x822000

/* BBH TX thresholds */
#define BBH_TX_LAN_DDR_THRESHOLD 0x100
#define BBH_TX_LAN_SRAM_THRESHOLD 0x49
#define BBH_TX_DATA_FIFO_SRAM_SIZE 0x5E
#define BBH_TX_DATA_FIFO_DDR_SIZE  0x11F
#define BBH_TX_DATA_FIFO_SRAM_BASE 0x120
#define BBH_TX_NUM_OF_LAN_QUEUES 8
#define BBH_TX_NUM_OF_LAN_QUEUES_PAIRS  (BBH_TX_NUM_OF_LAN_QUEUES / 2)

/* RNR */
#define RNR_QUAD_DMA_ARB_CONJEST_THRESHOLD  2

/* profiles */
#define TOTAL_BUFFERS_WEIGHT 32

#define UBUS_SLV_VPB_BASE_ADDR 0x82700000
#define UBUS_SLV_APB_BASE_ADDR 0x82900000
#define UBUS_SLV_VPB_BASE_MASK 0x82900000
#define UBUS_SLV_APB_BASE_MASK 0x82A00000
#define UBUS_SLV_DEVICE_0_BASE_ADDR 0x82a00000
#define UBUS_SLV_DEVICE_0_BASE_MASK 0x82c00000
#define UBUS_SLV_DEVICE_1_BASE_ADDR 0x82c00000
#define UBUS_SLV_DEVICE_1_BASE_MASK 0x82c80000
#define UBUS_SLV_DEVICE_2_BASE_ADDR 0x82c80000
#define UBUS_SLV_DEVICE_2_BASE_MASK 0x82d00000

#define XRDP_UBUS_SLAVE_IDX 0x14

#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window0_31_0  0x83020a10
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window0_63_32 0x83020a14
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window0_95_64 0x83020a18
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window1_31_0  0x83020a1c
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window1_63_32 0x83020a20
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window1_95_64 0x83020a24

#define UBUS_MSTR_ID_0        0
#define UBUS_MSTR_ID_1        1
#define UBUS_MSTR_ID_2        2
#define UBUS_MSTR_CMD_SPCAE   2
#define UBUS_MSTR_DATA_SPCAE  2

/* QM */
#define US_DDR_COPY_ENABLE    1
#define DS_DDR_COPY_ENABLE    0
#define DS_AGG_DISABLE        0

#define QM_WRED_PROFILE_CPU_RX                 13
/* DQM cannot host exactly 64K PDs, b/c some of the space is usef for it's internal logic ('next-pointers').
 * So actual max size PDs is 65024. In addition, 192 PD entries required by QM copy machine before they are actually
 * pushed to DQM, leave us with max PDs = 64000. To avoid some corner cases, we also reserve few more. */
#define DEF_QM_PD_CONGESTION_CONTROL_THRESHOLD 64000

#define QM_COPY_DEC_PROFILE_LOW                0
#define QM_COPY_DEC_PROFILE_HIGH               1

#define QM_COPY_DEC_MAX_QUEUE_OCCUPANCY        10240

#define QM_BBHTX_REQ_ADDR 5
#define QM_BBHTX_REQ_OTF 16


// dma0,1
#define DMA_DDR_CREDITS_0   10
#define DMA_PSRAM_CREDITS_0 12

//  dma copy
#define DMA_DDR_CREDITS_1   1
#define DMA_PSRAM_CREDITS_1 12

typedef struct
{
    bbh_rx_sdma_chunks_cfg_t bbh_rx_sdma_chunks_config[BBH_ID_NUM];
} bbh_rx_sdma_profile_t;

typedef struct
{
    bbh_tx_bbh_dma_cfg bbh_tx_dma_cfg[BBH_ID_NUM];
} bbh_tx_dma_profile_t;

typedef struct
{
    bbh_tx_bbh_sdma_cfg bbh_tx_sdma_cfg[BBH_ID_NUM];
} bbh_tx_sdma_profile_t;

typedef struct
{
    bbh_tx_bbh_ddr_cfg bbh_tx_ddr_cfg[BBH_ID_NUM];
} bbh_tx_ddr_profile_t;

typedef enum natc_tbl_idx_e
{
    tuple_lkp_ds_tbl = 0,
    tuple_lkp_us_tbl = 1,
    natc_tbls_num,
} natc_tbl_idx_t;

#define NATC_TABLE_BASE_SIZE_SIZE      8192

/* functions */
uint32_t data_path_init_fiber(int wan_bbh);
uint32_t data_path_init_gbe(rdpa_emac wan_emac);
int set_fpm_budget(int resource_num, int add);

#if CHIP_VER == RDP_GEN_60
/* TBD63146 - RU/HAL does not generate this structure and corresponding get/set functions on BCM63146_A0
 *            but does generate on the BCM63158_B0 after moving the Type_RCQ_CORE_REGS_RCQ_CFG_GEN_CFG
 *            definition from BCM63146_A0 to BCM63158_B0.
 */
/**************************************************************************************************/
/* disable_dma_old_flow_control: DISABLE_DMA_OLD_FLOW_CONTROL - Disable DMA old flow control. Whe */
/*                               n set to 1, DMA will not check read FIFO occupancy when issuing  */
/*                               READ requests, relying instead on DMA backpressure mechanism vs  */
/*                               read dispatcher block.                                           */
/* test_fit_fail: TEST_FIT_FAIL - set to 1 to test fit fail interrupt.                            */
/* zero_data_mem: ZERO_DATA_MEM - Trigger self-zeroing mechanism for data memory.                 */
/* zero_context_mem: ZERO_CONTEXT_MEM - Trigger self-zeroing mechanism for context memory.        */
/* zero_data_mem_done: ZERO_DATA_MEM_DONE - Goes high when zeroing is done. Reset to 0 when confi */
/*                     g ZERO_DATA_MEM is set to 0                                                */
/* zero_context_mem_done: ZERO_CONTEXT_MEM_DONE - Goes high when zeroing is done. Reset to 0 when */
/*                         config ZERO_CONTEXT_MEM is set to 0                                    */
/* enable_breakpoint_on_fit_fail: ENABLE_BREAKPOINT_ON_FIT_FAIL - When set to 1, Runner will brea */
/*                                k on fit_fail                                                   */
/* gdma_desc_offset: gdma_desc_offset - Configure descriptor offset for GATHER DMA command        */
/* bbtx_tcam_dest_sel: BBTX_TCAM_DEST_SEL - Select destination TCAM for Runner                    */
/* bbtx_hash_dest_sel: BBTX_HASH_DEST_SEL - Select destination HASH for Runner                    */
/* bbtx_natc_dest_sel: BBTX_NATC_DEST_SEL - Select destination NATC for Runner                    */
/* bbtx_cnpl_dest_sel: BBTX_CNPL_DEST_SEL - Select destination CNPL for Runner                    */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean disable_dma_old_flow_control;
    bdmf_boolean test_fit_fail;
    bdmf_boolean zero_data_mem;
    bdmf_boolean zero_context_mem;
    bdmf_boolean zero_data_mem_done;
    bdmf_boolean zero_context_mem_done;
    bdmf_boolean enable_breakpoint_on_fit_fail;
    uint8_t gdma_desc_offset;
    bdmf_boolean bbtx_tcam_dest_sel;
    bdmf_boolean bbtx_hash_dest_sel;
    bdmf_boolean bbtx_natc_dest_sel;
    bdmf_boolean bbtx_cnpl_dest_sel;
} rnr_regs_cfg_gen_cfg;

bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_set(uint8_t rnr_id, const rnr_regs_cfg_gen_cfg *cfg_gen_cfg);
bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_get(uint8_t rnr_id, rnr_regs_cfg_gen_cfg *cfg_gen_cfg);
#endif /* CHIP_VER == RDP_GEN_60 */

#ifdef __cplusplus 
}
#endif
#endif

