/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
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
#include "access_macros.h"
#include "packing.h"
#include "rdp_drv_bbh.h"

/* BBH route address towards DMA */
#define BBH_ROUTE_ADDRESS_DMA_EMAC0 0xC
#define BBH_ROUTE_ADDRESS_DMA_EMAC1 0xD
#define BBH_ROUTE_ADDRESS_DMA_EMAC2 0xE
#define BBH_ROUTE_ADDRESS_DMA_WAN   0xF

/* BBH route address towards BPM/FPM */
#define BBH_ROUTE_ADDRESS_BPM_EMAC0 0x14
#define BBH_ROUTE_ADDRESS_BPM_EMAC1 0x15
#define BBH_ROUTE_ADDRESS_BPM_EMAC2 0x16
#define BBH_ROUTE_ADDRESS_BPM_WAN   0x17

/* BBH route address towards SDMA */
#define BBH_ROUTE_ADDRESS_SDMA_EMAC0 0x1C
#define BBH_ROUTE_ADDRESS_SDMA_EMAC1 0x1D
#define BBH_ROUTE_ADDRESS_SDMA_EMAC2 0x1E
#define BBH_ROUTE_ADDRESS_SDMA_WAN   0x1F

/* BBH route address towards SBPM */
#define BBH_ROUTE_ADDRESS_SBPM_EMAC0 0x34
#define BBH_ROUTE_ADDRESS_SBPM_EMAC1 0x35
#define BBH_ROUTE_ADDRESS_SBPM_EMAC2 0x36
#define BBH_ROUTE_ADDRESS_SBPM_WAN 0x37

/* BBH route address towards RUNNER0 */
#define BBH_ROUTE_ADDRESS_RUNNER0_EMAC0 0x0
#define BBH_ROUTE_ADDRESS_RUNNER0_EMAC1 0x1
#define BBH_ROUTE_ADDRESS_RUNNER0_EMAC2 0x2
#define BBH_ROUTE_ADDRESS_RUNNER0_WAN   0x3

/* BBH route address towards RUNNER1 */
#define BBH_ROUTE_ADDRESS_RUNNER1_EMAC0 0x8
#define BBH_ROUTE_ADDRESS_RUNNER1_EMAC1 0x9
#define BBH_ROUTE_ADDRESS_RUNNER1_EMAC2 0xA
#define BBH_ROUTE_ADDRESS_RUNNER1_WAN   0xB

/* BBH route address towards IH */
#define BBH_ROUTE_ADDRESS_IH_EMAC0 0x18
#define BBH_ROUTE_ADDRESS_IH_EMAC1 0x19
#define BBH_ROUTE_ADDRESS_IH_EMAC2 0x1A
#define BBH_ROUTE_ADDRESS_IH_WAN 0x13

/* BBH DMA/SDMA read request base address */
#define BBH_DMA_FIFO_ADDRESS_BASE_EMAC0 0x0
#define BBH_DMA_FIFO_ADDRESS_BASE_EMAC1 0x8
#define BBH_DMA_FIFO_ADDRESS_BASE_EMAC2 0x10
#define BBH_DMA_FIFO_ADDRESS_BASE_WAN 0x28


#define IH_HEADER_LENGTH_MIN                       0x20
/* high congestion threshold of runner A (DS) */
#define IH_DS_RUNNER_A_HIGH_CONGESTION_THRESHOLD      32
/* exclusive congestion threshold of runner A (DS) */
#define IH_DS_RUNNER_A_EXCLUSIVE_CONGESTION_THRESHOLD 32
#define IH_PARSER_EXCEPTION_STATUS_BITS   0x47
#define IH_PARSER_AH_DETECTION            0x18000
/* PPP protocol code for IPv4 is configured at index 0 */
#define IH_PARSER_PPP_PROTOCOL_CODE_0_IPV4   0x21
/* PPP protocol code for IPv6 is configured at index 1 */
#define IH_PARSER_PPP_PROTOCOL_CODE_1_IPV6   0x57
#define IH_DA_FILTER_IPTV_IPV4   0
#define IH_DA_FILTER_IPTV_IPV6   1
#define TCP_CTRL_FLAG_RST        0x04
#define TCP_CTRL_FLAG_SYN        0x02
#define TCP_CTRL_FLAG_FIN        0x01
#define IH_IP_L4_FILTER_USER_DEFINED_0    0
#define IH_IP_L4_FILTER_USER_DEFINED_1    1
#define IH_IP_L4_FILTER_USER_DEFINED_2    2
#define IH_IP_L4_FILTER_USER_DEFINED_3    3

#define IH_L4_FILTER_DEF         0xff
/* we use one basic class */
#define IH_BASIC_CLASS_INDEX     0
/* IH classifier indices for broadcast and multicast traffic iptv destination */
#define IH_CLASSIFIER_BCAST_IPTV    0
#define IH_CLASSIFIER_IGMP_IPTV     1
#define IH_CLASSIFIER_ICMPV6        2
#define IH_CLASSIFIER_IPTV          (IH_CLASSIFIER_ICMPV6 + 1)

#define MASK_IH_CLASS_KEY_L4     0x3c0
/* IPTV DA filter mask in IH */
#define IPTV_FILTER_MASK_DA      0x3800
#define IPTV_FILTER_MASK_BCAST   0x8000

/* source port address */
#define IH_ETH0_ROUTE_ADDRESS       0x1C
#define IH_ETH1_ROUTE_ADDRESS       0xC
#define IH_ETH2_ROUTE_ADDRESS       0x14
#define IH_WAN_ROUTE_ADDRESS        0x0
#define IH_RUNNER_A_ROUTE_ADDRESS   0x3
#define IH_RUNNER_B_ROUTE_ADDRESS   0x2
/* size of ingress queue of each one of the EMACs which function as LAN */
#define IH_INGRESS_QUEUE_SIZE_LAG_EMACS      2
/* size of ingress queue of the WAN port */
#define IH_INGRESS_QUEUE_SIZE_WAN            4
/* size of ingress queue of each runner */
#define IH_INGRESS_QUEUE_SIZE_RUNNERS        1
/* priority of ingress queue of each one of the EMACs which function as LAN */
#define IH_INGRESS_QUEUE_PRIORITY_LAG_EMACS  1
/* priority of ingress queue of the WAN port */
#define IH_INGRESS_QUEUE_PRIORITY_WAN        2
/* priority of ingress queue of each runner */
#define IH_INGRESS_QUEUE_PRIORITY_RUNNERS    0
/* weight of ingress queue of each one of the EMACs which function as LAN */
#define IH_INGRESS_QUEUE_WEIGHT_LAG_EMACS    1
/* weight of ingress queue of the WAN port */
#define IH_INGRESS_QUEUE_WEIGHT_WAN          1
/* weight of ingress queue of each runner */
#define IH_INGRESS_QUEUE_WEIGHT_RUNNERS      1
/* congestion threshold of ingress queue of each one of the EMACs which function as LAN */
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_LAG_EMACS     31
/* congestion threshold of ingress queue of the WAN port */
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN           31
/* congestion threshold of ingress queue of each runner */
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_RUNNERS       31

/* default value for SBPM */
#define SBPM_DEFAULT_THRESHOLD   800
#define SBPM_DEFAULT_HYSTERESIS  0
#define BPM_DEFAULT_HYSTERESIS   64
#define BPM_DEFAULT_DS_THRESHOLD 7680
#define SBPM_BASE_ADDRESS        0
#define SBPM_LIST_SIZE           0x3FF

/* DMA */
#define BBH_RX_DMA_FIFOS_SIZE_LAG      18
#define BBH_RX_DMA_FIFOS_SIZE_WAN      10
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAG      18
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN     10

#define BBH_RX_SDMA_FIFOS_SIZE_LAG     6
#define BBH_RX_SDMA_FIFOS_SIZE_WAN     5

#define BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_LAG     5
#define BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_WAN     4

#define BBH_RX_DMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS  64
#define BBH_RX_SDMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS 32

/* multicast header size */
#define BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT 32

#define MIN_ETH_PKT_SIZE_LAN 64
#define MIN_ETH_PKT_SIZE_WAN 64

/* PD FIFO size of EMAC, when MDU mode is disabled */
#define BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_DISABLED 8

/* PD FIFO size of EMAC, when MDU mode is enabled */
#define BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_ENABLED  4

#define PSRAM_HEADROOM_SIZE   40
#define RDD_CPU_TX_ABS_FIFO_SIZE        LILAC_RDD_CPU_TX_SKB_LIMIT_MAX

#define BB_COLOR(clr_code)
#define _BBh_   BB_COLOR("\e[0;36;44m")     /* Highlight color */
#define _BBr_   BB_COLOR("\e[0;31m")        /* Error     color */
#define _BBg_   BB_COLOR("\e[0;32m")        /* Debug     color */
#define _BBn_   BB_COLOR("\e[0m")           /* Reset     color */
#define _BBb_   BB_COLOR("\e[0;34m")        /* Bold      color */
#define _BBnl_  _BBn_ "\n"                  /* Reset with newline */
/******************************************************************************/
/* There are 8 IH ingress queues. This enumeration defines, for each ingress  */
/* queue, which physical source port it belongs to.                           */
/******************************************************************************/
typedef enum
{
    IH_INGRESS_QUEUE_0_ETH0,
    IH_INGRESS_QUEUE_1_ETH1,
    IH_INGRESS_QUEUE_2_ETH2,
    IH_INGRESS_QUEUE_3_NOT_USED,
    IH_INGRESS_QUEUE_4_NOT_USED,
    IH_INGRESS_QUEUE_5_WAN,
    IH_INGRESS_QUEUE_6_RUNNER_A,
    IH_INGRESS_QUEUE_7_RUNNER_B,

    NUMBER_OF_IH_INGRESS_QUEUES
}
IH_INGRESS_QUEUE_INDEX ;

/******************************************************************************/
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    BB_MODULE_DMA,
    BB_MODULE_SDMA,

    BB_MODULE_NUM
} E_BB_MODULE;

typedef enum
{
    DMA_PERIPHERAL_EMAC_0,
    DMA_PERIPHERAL_EMAC_1,
    DMA_PERIPHERAL_EMAC_2,
    DMA_PERIPHERAL_EMAC_3_NOT_USED,
    DMA_PERIPHERAL_EMAC_4_NOT_USED,
    DMA_PERIPHERAL_WAN,

    DMA_NUMBER_OF_PERIPHERALS
} E_DMA_PERIPHERAL;

typedef enum
{
    bcm_tag_opcode0,
    bcm_tag_opcode1
} bcm_tag_type;

typedef struct
{
    uint32_t mtu_size;
    uint32_t headroom_size;
    /* buffer memory is the memory reserved for FPM buffer pool
     * flow memory is the memory reserved for Runner Tables + Multicast */
    uint8_t *rdp_ddr_bm_base;            /* buffer memory virtual base address */
    uint8_t *rdp_ddr_bm_phys;            /* buffer memory physical base address */
    uint32_t runner_ddr_bm_size;         /* buffer memory size */
    uint8_t *rdp_ddr_fm_base;            /* flow memory virtual base address */
    uint8_t *rdp_ddr_fm_phys;            /* flow memory physical base address */
    uint32_t runner_ddr_fm_size;         /* flow memory size */
    DRV_BBH_FPM_BUFF_SIZE fpm_buff_size_set;
    uint32_t runner_freq;                    /* rdp block clock */
} data_path_init_params;

int data_path_init(data_path_init_params *dpi_params);
int data_path_init_sim(data_path_init_params *dpi_params);

#endif
