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
#include "bdmf_data_types.h"
#include "rdp_common.h"

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
#define FPM_INIT_TIMEOUT                    0xf000
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
#define SOP_OFFSET 18
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
#define NUM_OF_UNIMAC 6
#define RNR0 0
#define RNR1 1
#define MAX_OTF_READ_REQUEST_DEFAULT_DMA0  12
#define MAX_OTF_READ_REQUEST_DEFAULT_DMA1  16
#define EPON_URGENT_REQUEST_DISABLE  0
#define EPON_URGENT_REQUEST_ENABLE   1

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



#define XRDP_UBUS_SLAVE_IDX 0x14

#define UBUS_SLV_VPB_START_ADDR 0x82700000
#define UBUS_SLV_VPB_END_ADDRESS 0x82900000
#define UBUS_SLV_APB_START_ADDR 0x82900000
#define UBUS_SLV_APB_END_ADDRESS 0x82A00000

#define UBUS_SLV_DEVICE_0_START_ADDR 0x82a00000
#define UBUS_SLV_DEVICE_0_END_ADDR 0x82c00000
#define UBUS_SLV_DEVICE_1_START_ADDR 0x82C00000
#define UBUS_SLV_DEVICE_1_END_ADDR 0x82C80000
#define UBUS_SLV_DEVICE_2_START_ADDR 0x82C80000
#define UBUS_SLV_DEVICE_2_END_ADDR 0x82D00000

#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window0_31_0  (0x20a10 / 4)
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window0_63_32 (0x20a14 / 4)
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window0_95_64 (0x20a18 / 4)
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window1_31_0  (0x20a1c / 4)
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window1_63_32 (0x20a20 / 4)
#define DECODE_CFG_REG_MST_PORT_NODE_B53_Window1_95_64 (0x20a24 / 4)

#define UBUS_MSTR_ID_0        0
#define UBUS_MSTR_ID_1        1
#define UBUS_MSTR_CMD_SPCAE   2
#define UBUS_MSTR_DATA_SPCAE  2

/* QM */
#define US_DDR_COPY_ENABLE    1
#define DS_DDR_COPY_ENABLE    0
#define DS_AGG_DISABLE        0

/* DQM cannot host exactly 64K PDs, b/c some of the space is usef for it's internal logic ('next-pointers').
 * So actual max size PDs is 65024. In addition, 192 PD entries required by QM copy machine before they are actually
 * pushed to DQM, leave us with max PDs = 64000. To avoid some corner cases, we also reserve few more. */
#define DEF_QM_PD_CONGESTION_CONTROL_THRESHOLD 64000

#define QM_COPY_DEC_PROFILE_LOW                0
#define QM_COPY_DEC_PROFILE_HIGH               1

#define QM_COPY_DEC_MAX_QUEUE_OCCUPANCY        10240

#define QM_BBHTX_REQ_ADDR 0
#define QM_BBHTX_REQ_OTF 16

/* DMA */
#define DMA_DDR_CREDITS   7
#define DMA_PSRAM_CREDITS 8
#define DMA_DDR_BYTES_OTF  1500
#define DMA_PSRAM_BYTES_OTF  0xffff
#define PERIPHERALS_PER_DMA 8
#define PSRAM1_BASE_OFFSET 0x822000
#define PSRAM0_BASE_OFFSET 0x820000
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


/* functions */
int data_path_init(dpi_params_t *dpi_params);
int data_path_init_basic(dpi_params_t *dpi_params);
uint32_t data_path_init_fiber(int wan_bbh);
int set_fpm_budget(int resource_num, int add);

#ifdef __cplusplus 
}
#endif
#endif

