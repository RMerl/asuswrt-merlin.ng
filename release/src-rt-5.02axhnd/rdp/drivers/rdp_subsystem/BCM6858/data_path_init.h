/*
* <:copyright-BRCM:2015:proprietary:standard
*
*    Copyright (c) 2015 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
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
#include "rdd_natc.h"

/* default value for DISPATCHER */
#define DISP_REOR_WAN_BBH_VIQ_EN(viq_en)    ((viq_en | (1 << BBH_ID_PON)) | (1 << DISP_REOR_VIQ_BBH_WAN_EXCL))

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
#define FPM_XON_THRESHOLD                   (TOTAL_FPM_TOKENS - TOTAL_DYNAMIC_FPM)
#define FPM_XOFF_THRESHOLD                  (7*FPM_XON_THRESHOLD/8)

/* default value for BBH-RX */
#define SOP_OFFSET 18
#ifdef G9991
#define G9991_US_SOP_OFFSET 124
#endif
#define MIN_ETH_PKT_SIZE  64
#define MIN_WAN_PKT_SIZE  64
#define MIN_OMCI_PKT_SIZE 14
#define BBH_RX_FLOWS_32_255_GROUP_DIVIDER 255
#define G9991_DFC_PATTERN_MSB  (0x01 << 16)
#define G9991_DFC_PATTERN_LSB  0x0
#define G9991_DFC_MASK_MSB     0xffff0000
#define G9991_DFC_MASK_LSB     0x0
#define EPON_CONTROL_PATTERN_MSB        0
#define EPON_CONTROL_PATTERN_LSB        0x88080000
#define EPON_CONTROL_PATTERN_MASK_MSB   0
#define EPON_CONTROL_PATTERN_MASK_LSB   0xfffe0000

/* Common to both BBH-RX and SDMA */
#define BBH_RX_TOTAL_BUFFER_NUM  32
#define BBH_TX_TOTAL_BUFFER_NUM  64
#define NUM_OF_DMA 2
#define NUM_OF_DMA_SDMA 4
#define RNR0 0
#define RNR1 1
#define MAX_OTF_READ_REQUEST_DEFAULT_DMA0  12
#define MAX_OTF_READ_REQUEST_DEFAULT_DMA1  16
#define EPON_URGENT_REQUEST_DISABLE  0
#define EPON_URGENT_REQUEST_ENABLE   1

/* BBH TX thresholds */
#ifdef BCM6858
#define BBH_TX_XFI_LAN_DDR_THRESHOLD 0x15F
#define BBH_TX_XFI_LAN_SRAM_THRESHOLD 0x9F
#define BBH_TX_DATA_XFI_FIFO_SRAM_SIZE 0xBF
#define BBH_TX_DATA_XFI_FIFO_DDR_SIZE  0x17F
#define BBH_TX_XFI_DATA_FIFO_SRAM_BASE 0x180
#else
#define BBH_TX_XFI_LAN_DDR_THRESHOLD 0xAA
#define BBH_TX_XFI_LAN_SRAM_THRESHOLD 0xAA
#define BBH_TX_DATA_XFI_FIFO_SRAM_SIZE 0xBE
#define BBH_TX_DATA_XFI_FIFO_DDR_SIZE  0xBE
#define BBH_TX_XFI_DATA_FIFO_SRAM_BASE 0xBF
#endif
#define BBH_TX_LAN_DDR_THRESHOLD 0x100
#define BBH_TX_LAN_SRAM_THRESHOLD 0x49
#define BBH_TX_DATA_FIFO_SRAM_SIZE 0x5E
#define BBH_TX_DATA_FIFO_DDR_SIZE  0x11F
#define BBH_TX_DATA_FIFO_SRAM_BASE 0x120

/* RNR */
#define RNR_QUAD_DMA_ARB_CONJEST_THRESHOLD  2

/* profiles */
#define TOTAL_BUFFERS_WEIGHT 32

#define UBUS_SLV_VPB_BASE_ADDR 0x82d00000
#define UBUS_SLV_APB_BASE_ADDR 0x82e00000
#define UBUS_SLV_MASK 0xffe00000
#define XRDP_UBUS_SLAVE_IDX 0x14

#define UBUS_MSTR_ID_0        0
#define UBUS_MSTR_ID_1        1
#define UBUS_MSTR_CMD_SPCAE   2
#define UBUS_MSTR_DATA_SPCAE  2

/* QM */
#ifdef G9991
#define US_DDR_COPY_ENABLE    1
#define DS_DDR_COPY_ENABLE    1
#define DS_AGG_DISABLE        1
#else
#define US_DDR_COPY_ENABLE    1
#define DS_DDR_COPY_ENABLE    0
#define DS_AGG_DISABLE        0
#endif

#define QM_COPY_DEC_PROFILE_LOW                0
#define QM_COPY_DEC_PROFILE_HIGH               1

#define QM_COPY_DEC_MAX_QUEUE_OCCUPANCY        10240


/* DQM cannot host exactly 16K PDs, b/c some of the space is used for it's internal logic ('next-pointers').
 * So actual max size PDs is 16256. In addition, 192 PD entries required by QM copy machine before they are actually
 * pushed to DQM, leave us with max PDs = 16064. To avoid some corner cases, we also reserve few more. */

#define DEF_QM_PD_CONGESTION_CONTROL_THRESHOLD 64000

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

#define NATC_TABLE_BASE_SIZE_SIZE      8192

/* functions */
uint32_t data_path_init_fiber(int wan_bbh);
uint32_t data_path_init_gbe(rdpa_emac wan_emac);
uint32_t fpm_get_dqm_extra_fpm_tokens(void);

int set_ug_thresholds(uint8_t ug_idx, uint32_t fpm_max_thr, uint32_t reserved_packet_buffer);

#ifdef __cplusplus
}
#endif
#endif

