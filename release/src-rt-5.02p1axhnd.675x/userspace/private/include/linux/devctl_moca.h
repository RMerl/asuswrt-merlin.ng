/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef __DEVCTL_MOCA_H__
#define __DEVCTL_MOCA_H__


/*!\file devctl_moca.h
 * \brief Header file for the user mode MOCA device control library API.
 *  This is in the devCtl library.
 *
 * These API are called by user applications to perform MOCA driver operations.
 * These API make Linux ioctl calls to MOCA driver. 
 *
 */

#include "cms.h"


/***************************************************************************
 * Constant Definitions
 ***************************************************************************/

#define MOCA_TRC_LEVEL_NONE      0x00000000  /**< Permanent traces only */
#define MOCA_TRC_LEVEL_FN_ENTRY  0x00000001  /**< Fn Entry points, Args, if any */
#define MOCA_TRC_LEVEL_FN_EXIT   0x00000002  /**< Fn Exit points, Args, if any */
#define MOCA_TRC_LEVEL_INFO      0x00000004  /**< During the course of flow execution */
#define MOCA_TRC_LEVEL_DBG       0x00000008  /**< Debug information */
#define MOCA_TRC_LEVEL_ERR       0x00000010  /**< Erroneous Situations */
#define MOCA_TRC_LEVEL_WARN      0x00000020  /**< Warning messages */
#define MOCA_TRC_LEVEL_VERBOSE   0x00000040  /**< Verbose messages */
#define MOCA_DUMP_HOST_CORE      0x00010000  /**< Host-Core message exchanges */
#define MOCA_TRC_LEVEL_TRAP      0x00020000  /**< Trap message information */
#define MOCA_TRC_LEVEL_CORE      0x00040000  /**< Core Traces */
#define MOCA_TRC_TIMESTAMPS      0x00080000  /**< Show timestamps */
#define MOCA_TRC_LEVEL_ALL      (MOCA_TRC_LEVEL_FN_ENTRY | \
                                 MOCA_TRC_LEVEL_FN_EXIT  | \
                                 MOCA_TRC_LEVEL_INFO     | \
                                 MOCA_TRC_LEVEL_DBG      | \
                                 MOCA_TRC_LEVEL_ERR      | \
                                 MOCA_TRC_LEVEL_WARN     | \
                                 MOCA_TRC_LEVEL_VERBOSE  | \
                                 MOCA_DUMP_HOST_CORE     | \
                                 MOCA_TRC_LEVEL_TRAP     | \
                                 MOCA_TRC_LEVEL_CORE     | \
                                 MOCA_TRC_TIMESTAMPS)
#define MOCA_TRC_LEVEL_MAX      (MOCA_TRC_LEVEL_CORE|MOCA_TRC_LEVEL_ALL)


/* MoCA Sizing values */
#define MoCA_MIN_PASSWORD_LEN          12
#define MoCA_MAX_PASSWORD_LEN          17
#define MoCA_MAX_KEY_LEN               8
#define MoCA_MAX_PQOS_ENTRIES          16
#define MoCA_MAX_QOS_ENTRIES           16 /* Max Ethernet Prio ?? */
#define MoCA_MAX_NODES                 16
#define MoCA_MAX_NODES_1_0             8
#define MoCA_MAX_UC_FWD_ENTRIES        128
#define MoCA_MAX_MC_FWD_ENTRIES        63
#define MoCA_NUM_MEMBERS_FOR_NON_BCAST 4
#define MoCA_NUM_MEMBERS_PER_MC_GROUP  MoCA_MAX_NODES
#define MoCA_MAX_SUB_CARRIERS          256
#define MoCA_MAX_SRC_ADDR_ENTRIES      128

#define MAX_BITS_PER_SUB_CARRIER       4
#define BITS_PER_BYTE                  8
#define BYTES_PER_WORD                 4

typedef UINT8  MAC_ADDRESS[6];

/* MoCA NV Parms values & definitions */
typedef enum {
   MoCA_AUTO_NEGOTIATE_FOR_NC = 0x0,      /* Default */
   MoCA_ALWAYS_NC,
   MoCA_NEVER_NC,
   MoCA_DEF_NC_MODE = MoCA_AUTO_NEGOTIATE_FOR_NC
} MoCA_NC_MODE;

#define MoCA_AUTO_NW_SCAN_DISABLED     0  /* Default */
#define MoCA_AUTO_NW_SCAN_ENABLED      1
#define MoCA_AUTO_NW_SCAN_DISABLED_NOTABOO 2
#define MoCA_DEF_AUTO_NW_SEARCH        MoCA_AUTO_NW_SCAN_ENABLED

#define MoCA_PSS_IE_DISABLE            0
#define MoCA_PSS_IE_ENABLE             1

#define MoCA_PRIVACY_DISABLED          0  /* Default */
#define MoCA_PRIVACY_ENABLED           1
#define MoCA_DEF_PRIVACY               MoCA_PRIVACY_DISABLED

#define MoCA_TPC_DISABLED              0  /* Default */
#define MoCA_TPC_ENABLED               1
#define MoCA_DEF_TPC                   MoCA_TPC_ENABLED

#define MoCA_NODE_TYPE_INTERMEDIATE    0
#define MoCA_NODE_TYPE_TERMINAL        1

#define MoCA_NORMAL_OPERATION               0  /* Default */
#define MoCA_CONTINUOUS_TX_PROBE_I          1
#define MoCA_CONTINUOUS_RX                  2  /* ICAP.102 mode */
#define MoCA_EXTERNAL_CONTROL               3
#define MoCA_CONTINUOUS_RX_LO_ON            4  /* ICAP.105 mode */
#define MoCA_CONTINUOUS_TX_CW               5
#define MoCA_CONTINUOUS_TX_TONE             6
#define MoCA_CONTINUOUS_TX_TONE_SC          7
#define MoCA_CONTINUOUS_TX_DUAL_TONE_SC     8
#define MoCA_CONTINUOUS_TX_BAND             9
#define MoCA_DEF_CONST_TX_MODE              MoCA_NORMAL_OPERATION

#define MoCA_CONTINUOUS_TX_BAND_MIN         1
#define MoCA_CONTINUOUS_TX_BAND_MAX         256
#define MoCA_CONTINUOUS_TX_BAND_ARRAY_SIZE  8

#define MoCA_MIN_TX_POWER_BEACONS      -31
#define MoCA_MAX_TX_POWER_BEACONS      3
#define MoCA_DEF_TX_POWER_BEACONS      3

#define MoCA_MIN_TX_POWER_PACKETS      -31
#define MoCA_MAX_TX_POWER_PACKETS      3
#define MoCA_DEF_TX_POWER_PACKETS      3

#define MoCA_DEF_RESERVED_INIT_0       575
#define MoCA_DEF_BEACON_CHANNEL        575

#define MoCA_DEF_FREQ_SHIFT_MODE       MoCA_FREQ_SHIFT_MODE_OFF
#define MoCA_FREQ_SHIFT_MODE_OFF       0
#define MoCA_FREQ_SHIFT_MODE_PLUS      1
#define MoCA_FREQ_SHIFT_MODE_MINUS     2
#define MoCA_MIN_FREQ_SHIFT_MODE       0
#define MoCA_MAX_FREQ_SHIFT_MODE       2


#define MoCA_BO_MODE_SLOW              1
#define MoCA_BO_MODE_FAST              0
#define MoCA_DEF_BO_MODE               MoCA_BO_MODE_FAST

#define MoCA_EGR_MC_FILTER_ENABLED                1
#define MoCA_EGR_MC_FILTER_DISABLED               0
#define MoCA_DEF_EGR_MC_FILTER_EN                 MoCA_EGR_MC_FILTER_DISABLED

#define MoCA_DEF_LOW_PRI_Q_NUM                    2

#define MoCA_CONTINUOUS_RX_MODE_ATTN_MAX 63
#define MoCA_CONTINUOUS_RX_MODE_ATTN_MIN -1
#define MoCA_DEF_RX_MODE_ATTN             0


#define MoCA_RF_TYPE_D_BAND            0  /** High RF */
#define MoCA_RF_TYPE_E_BAND            1  /** MidRF Sub-band low */
#define MoCA_RF_TYPE_F_BAND            2  /** MidRF Sub-band high */
#define MoCA_RF_TYPE_C4_BAND           3  /** WAN frequency 1000 MHz */

#define MoCA_DEF_MID_FREQ_MHZ          575
#define MoCA_DEF_WAN_FREQ_MHZ          1000
#define MoCA_DEF_LAN_FREQ_MHZ          1150
#define MoCA_DEF_FREQ_MHZ              MoCA_DEF_LAN_FREQ_MHZ
#define MoCA_NULL_FREQ_MHZ             0
#define MoCA_FREQ_UNSET                1
#define MoCA_MIN_FREQ_MHZ              25  /* To support Mid RF range */
#define MoCA_FREQ_INC_MHZ              25
#define MoCA_MAX_FREQ_MHZ              1825
#define MoCA_DBAND_FREQ_MASK           0x0000FF00 /**  
                                                   Bit   Center Freq (MHz)
                                                   20   500Mhz (M1)
                                                   19   525Mhz (M2)
                                                   18   550Mhz (M3)
                                                   17   575Mhz (M4)
                                                   16   600Mhz (M5)
                                                   15   1150Mhz (D1)
                                                   14   1200Mhz (D2)
                                                   13   1250Mhz (D3)
                                                   12   1300Mhz (D4)
                                                   11   1350Mhz (D5)
                                                   10   1400Mhz (D6)
                                                   9   1450Mhz (D7)
                                                   8   1500Mhz (D8)
                                                   7-0   Reserved for future  high bands */

#define MoCA_DEF_USER_PASSWORD         "99999999988888888"
#define MoCA_DEF_PASSWORD_SIZE         MoCA_MAX_PASSWORD_LEN

#define MoCA_MCAST_NORMAL_MODE         0
#define MoCA_MCAST_BCAST_MODE          1
#define MoCA_DEF_MCAST_MODE            MoCA_MCAST_BCAST_MODE

#define MoCA_NORMAL_MODE               0
#define MoCA_LAB_MODE                  1
#define MoCA_DEF_MODE                  MoCA_NORMAL_MODE

#define MoCA_MAX_TABOO_FIXED_CHANNELS        24
#define MoCA_VALID_TABOO_FIXED_CHAN_MASK     0x00FFFFFF
#define MoCA_VALID_TABOO_LEFT_MASK           0x00FFFFFF
#define MoCA_VALID_TABOO_RIGHT_MASK          0xFFFFFF00

#define MoCA_DEF_TABOO_FIXED_START_CHANNEL      0
#define MoCA_DEF_TABOO_FIXED_CHANNEL_MASK       0xF80000
#define MoCA_DEF_TABOO_LEFT_MASK                0x00FFFFFF
#define MoCA_DEF_TABOO_RIGHT_MASK               0xFFFFFF00


#define MoCA_MIN_PAD_POWER             -15
#define MoCA_MAX_PAD_POWER             -5

#define MoCA_MIN_BEACON_PWR_REDUCTION  0
#define MoCA_MAX_BEACON_PWR_REDUCTION  30

#ifdef DSL_MOCA 
#define MoCA_DEF_PAD_POWER             -10
#else
#define MoCA_DEF_PAD_POWER             -9
#endif


#define MoCA_VERSION_10                0x10
#define MoCA_VERSION_11                0x11
#define MoCA_DEF_OPERATING_VERSION     MoCA_VERSION_11

#define MoCA_NO_PREFERED_NC_MODE       0
#define MoCA_PREFERED_NC_MODE          1
#define MoCA_DEF_PREFERED_NC_MODE      MoCA_NO_PREFERED_NC_MODE

#define MoCA_LED_MODE_NW_SEARCH                 0
#define MoCA_LED_MODE_LINK_ON_OFF               1
#define MoCA_LED_MODE_BLINK_LOW_PHY_TRAFFIC     2
#define MoCA_LED_MODE_BLINK_LOW_PHY_NO_TRAFFIC  3
#define MoCA_DEF_LED_MODE              MoCA_LED_MODE_NW_SEARCH

#define MoCA_LOOPBACK_DISABLED         0    /* Default */
#define MoCA_LOOPBACK_ENABLED          1
#define MoCA_DEF_LOOPBACK_EN           MoCA_LOOPBACK_DISABLED

#define MoCA_MIN_FRAME_SIZE            2048
#define MoCA_MAX_FRAME_SIZE            9216
#define MoCA_DEF_FRAME_SIZE            6148

#define MoCA_MIN_TRANSMIT_TIME         300
#define MoCA_MAX_TRANSMIT_TIME         1000
#define MoCA_DEF_TRANSMIT_TIME         400

#define MoCA_MIN_BW_ALARM_THRESHOLD_DISABLED    0
#define MoCA_MIN_BW_ALARM_THRESHOLD             50
#define MoCA_DEF_BW_ALARM_THRESHOLD             100
#define MoCA_MAX_BW_ALARM_THRESHOLD             300

#define MoCA_CONTINUOUS_IE_RR_INSERT_OFF        0
#define MoCA_CONTINUOUS_IE_RR_INSERT_ON         1
#define MoCA_DEF_CONTINUOUS_IE_RR_INSERT        MoCA_CONTINUOUS_IE_RR_INSERT_OFF

#define MoCA_CONTINUOUS_IE_MAP_INSERT_OFF       0
#define MoCA_CONTINUOUS_IE_MAP_INSERT_ON        1
#define MoCA_DEF_CONTINUOUS_IE_MAP_INSERT       MoCA_CONTINUOUS_IE_MAP_INSERT_OFF

#define MoCA_MIN_PKT_AGGR              1
#define MoCA_MAX_PKT_AGGR              10
#define MoCA_DEF_PKT_AGGR              10

#define MoCA_MAX_CONSTELLATION_ALL_NODES        0xFFFFFFFF
#define MoCA_DEF_CONSTELLATION_NODE             0
typedef enum _Constellation {
   MIN_CONSTELLATION = 0,
   BPSK,
   QPSK,
   QAM8,
   QAM16,
   QAM32,
   QAM64,
   QAM128,
   QAM256,
   QAM512,
   QAM1024,
   MAX_CONSTELLATION
} MaxConstellation;
#define MoCA_DEF_CONSTELLATION_INFO             QAM256

#define MoCA_MIN_PMK_EXCHANGE_INTERVAL          1
#define MoCA_MAX_PMK_EXCHANGE_INTERVAL          12
#define MoCA_DEF_PMK_EXCHANGE_INTERVAL          11

#define MoCA_MIN_TEK_EXCHANGE_INTERVAL          1
#define MoCA_MAX_TEK_EXCHANGE_INTERVAL          10
#define MoCA_DEF_TEK_EXCHANGE_INTERVAL          9

#define MoCA_MIN_USER_SNR_MARGIN                -3   /** in dBs */
#define MoCA_DEF_USER_SNR_MARGIN                0
#define MoCA_DEF_USER_SNR_MARGIN_HIRF           0
#define MoCA_MAX_USER_SNR_MARGIN                25

#define MoCA_MIN_USER_SNR_MARGIN_OFFSET         -3   /** in dBs */
#define MoCA_DEF_USER_SNR_MARGIN_OFFSET         0
#define MoCA_MAX_USER_SNR_MARGIN_OFFSET         25

#define MoCA_QAM256_CAPABILITY_ENABLED         1
#define MoCA_QAM256_CAPABILITY_DISABLED        0



#define MoCA_MIN_SAPM_TABLE            0   /** in dBs */
#define MoCA_MAX_SAPM_TABLE            100

#define MoCA_MIN_RLAPM_TABLE           0   /** in dBs */
#define MoCA_MAX_RLAPM_TABLE           30


#define MoCA_MIN_SAPM_EN               0
#define MoCA_MAX_SAPM_EN               1
#define MoCA_DEF_SAPM_EN               0

#define MoCA_MIN_ARPL_TH               -65
#define MoCA_MAX_ARPL_TH               0
#define MoCA_DEF_ARPL_TH               -50

#define MoCA_MIN_RLAPM_EN              0
#define MoCA_MAX_RLAPM_EN              1
#define MoCA_DEF_RLAPM_EN              0



#define MoCA_PRIO_ALLOC_RESV_HIGH      300
#define MoCA_PRIO_ALLOC_RESV_MED       300
#define MoCA_PRIO_ALLOC_RESV_LOW       300
#define MoCA_PRIO_ALLOC_LIMIT_HIGH     300
#define MoCA_PRIO_ALLOC_LIMIT_MED      300
#define MoCA_PRIO_ALLOC_LIMIT_LOW      300

#define MoCA_DEF_PRIO_RESV_HIGH        9
#define MoCA_DEF_PRIO_RESV_MED         64
#define MoCA_DEF_PRIO_RESV_LOW         64
#define MoCA_DEF_PRIO_RESV_HIGH_7408   1
#define MoCA_DEF_PRIO_RESV_MED_7408    1
#define MoCA_DEF_PRIO_RESV_LOW_7408    10

#define MoCA_DEF_PRIO_LIMIT_HIGH       300
#define MoCA_DEF_PRIO_LIMIT_MED        300
#define MoCA_DEF_PRIO_LIMIT_LOW        300

#define MoCA_MIN_TARGET_PHY_RATE_QAM128         0
#define MoCA_DEF_TARGET_PHY_RATE_QAM128         235
#define MoCA_MAX_TARGET_PHY_RATE_QAM128         500

#define MoCA_MIN_TARGET_PHY_RATE_QAM256         0
#define MoCA_DEF_TARGET_PHY_RATE_QAM256         275
#define MoCA_MAX_TARGET_PHY_RATE_QAM256         500

#define MoCA_MIN_TARGET_PHY_RATE_TURBO          0
#define MoCA_DEF_TARGET_PHY_RATE_TURBO          380
#define MoCA_MAX_TARGET_PHY_RATE_TURBO          2500

#define MoCA_MIN_SELECTIVE_RR                   0
#define MoCA_MAX_SELECTIVE_RR                   255

#define MoCA_MIN_TARGET_PHY_RATE_TURBO_PLUS     0
#define MoCA_DEF_TARGET_PHY_RATE_TURBO_PLUS     500
#define MoCA_MAX_TARGET_PHY_RATE_TURBO_PLUS     2500


#define MoCA_CORE_TRACE_CONTROL_DISABLE         0
#define MoCA_CORE_TRACE_CONTROL_ENABLE          1
#define MoCA_DEF_CORE_TRACE_CONTROL_ENABLE      MoCA_CORE_TRACE_CONTROL_DISABLE

#define MoCA_IQ_BURST_TYPE_BEACON               1
#define MoCA_IQ_BURST_TYPE_MAP                  6
#define MoCA_IQ_BURST_TYPE_UNICAST              7
#define MoCA_IQ_BURST_TYPE_BROADCAST            8

#define MoCA_IQ_ACMT_SYM_NUM_LAST_SYM           1
#define MoCA_IQ_ACMT_SYM_NUM_SYM_BEFORE_LAST    2

#define MOCA_MAX_EGR_MC_FILTERS                 32

#define MOCA_UPDATE_FLASH_INIT                  0
#define MOCA_UPDATE_FLASH_CONFIG                1
#define MOCA_UPDATE_FLASH_TRACE                 2

#define MOCA_DEF_MIN_AGGR_WAIT_TIME             0

/* Return status values. */
typedef enum BcmMoCAStatus
{
   MoCASTS_SUCCESS = 0,
   MoCASTS_STANDALONE_MODE,
   MoCASTS_INIT_FAILED,
   MoCASTS_ERROR,
   MoCASTS_LOAD_ERROR,
   MoCASTS_STATE_ERROR,
   MoCASTS_PARAMETER_ERROR,
   MoCASTS_ALLOC_ERROR,
   MoCASTS_RESOURCE_ERROR,
   MoCASTS_IN_USE,
   MoCASTS_NOT_FOUND,
   MoCASTS_NOT_SUPPORTED,
   MoCASTS_HOST_REQ_TIMEOUT,
   MoCASTS_CORE_RESP_TIMEOUT,
   MoCASTS_GET_NEXT,
   MoCASTS_NOT_EXIST
} BCMMoCA_STATUS;

/* status definitions */

#define MoCA_QAM_256_SUPPORT_ON         1

#define MoCA_LINK_DOWN                  0
#define MoCA_LINK_UP                    1

#define MoCA_OPER_STATUS_ENABLED        0
#define MoCA_OPER_STATUS_HW_ERROR       1

#define MoCA_NODE_INACTIVE              0
#define MoCA_NODE_ACTIVE                1

#define MoCA_NODE_IS_NOT_NC             0
#define MoCA_NODE_IS_NC                 1

typedef enum _MoCA_CALLBACK_EVENT {
   MoCA_CALLBACK_EVENT_LINK_STATUS = 0,
   MoCA_CALLBACK_EVENT_LOF,
   MoCA_MAX_CALLBACK_EVENTS
} MoCA_CALLBACK_EVENT;

typedef struct _MoCA_CALLBACK_DATA {
   union {
     UINT32     linkStatus;
     UINT32     lof;
   } data;   
} MoCA_CALLBACK_DATA;

typedef struct _MoCANvParams {
   UINT32        lastOperFreq;
} MoCA_NV_PARAMS, *PMoCA_NV_PARAMS;

typedef struct _MoCA_InitOptions {
      UINT32     constTxSubCarrier1;
      UINT32     constTxSubCarrier2;
      UINT32     constTxNoiseBand[MoCA_CONTINUOUS_TX_BAND_ARRAY_SIZE];
      UINT32     dontStartMoca; /** default 0. setting to 1 causes moca not to run */
} MoCA_INIT_OPTIONS;

/* Masks defined here are used in selecting the required parameter from Mgmt
 * application.
 */
#define MoCA_MAX_INIT_PARAMS                  45
#define MoCA_INIT_PARAM_ALL_MASK              0x1FFFFFFFFFFFLL
typedef struct MoCAInitialization
{
#define MoCA_INIT_PARAM_NC_MODE_MASK                  0x00000001LL
   MoCA_NC_MODE   ncMode;                /* Network Control Mode */
#define MoCA_INIT_PARAM_AUTO_NETWORK_SEARCH_EN_MASK   0x00000002LL
   UINT32         autoNetworkSearchEn;
#define MoCA_INIT_PARAM_PRIVACY_MASK                  0x00000004LL
   UINT32         privacyEn;
#define MoCA_INIT_PARAM_TX_PWR_CONTROL_EN_MASK        0x00000008LL
   UINT32         txPwrControlEn;          /* Transmit Power Control */
#define MoCA_INIT_PARAM_CONST_TRANSMIT_MODE_MASK      0x00000010LL
   UINT32         constTransmitMode;
#define MoCA_INIT_PARAM_NV_PARAMS_LOF_MASK            0x00000020LL
   MoCA_NV_PARAMS nvParams;
#define MoCA_INIT_PARAM_MAX_TX_POWER_BEACONS_MASK     0x00000040LL
   signed int     maxTxPowerBeacons;
#define MoCA_INIT_PARAM_PASSWORD_SIZE_MASK            0x00000080LL
   UINT32         passwordSize;
#define MoCA_INIT_PARAM_PASSWORD_MASK                 0x00000100LL
   UINT8          password [MoCA_MAX_PASSWORD_LEN];
#define MoCA_INIT_PARAM_MCAST_MODE_MASK               0x00000200LL
   UINT32         mcastMode;
#define MoCA_INIT_PARAM_LAB_MODE_MASK                 0x00000400LL
   UINT32         labMode;
#define MoCA_INIT_PARAM_TABOO_FIXED_MASK_START_MASK   0x00000800LL
   UINT32         tabooFixedMaskStart;
#define MoCA_INIT_PARAM_TABOO_FIXED_CHANNEL_MASK_MASK 0x00001000LL
   UINT32         tabooFixedChannelMask;
#define MoCA_INIT_PARAM_TABOO_LEFT_MASK_MASK          0x00002000LL
   UINT32         tabooLeftMask;
#define MoCA_INIT_PARAM_TABOO_RIGHT_MASK_MASK         0x00004000LL
   UINT32         tabooRightMask;
#define MoCA_INIT_PARAM_PAD_POWER_MASK                0x00008000LL
   signed int     padPower;
#define MoCA_INIT_PARAM_OPERATING_VERSION_MASK        0x00010000LL
   signed int     operatingVersion;
#define MoCA_INIT_PARAM_PREFERED_NC_MASK              0x00020000LL
   UINT32         preferedNC;
#define MoCA_INIT_PARAM_LED_MODE_MASK                 0x00040000LL
   UINT32         ledMode;
#define MoCA_INIT_PARAM_MAX_TX_POWER_PACKETS_MASK     0x00080000LL
   signed int     maxTxPowerPackets;
#define MoCA_INIT_PARAM_MoCA_LOOPBACK_EN_MASK         0x00100000LL
   UINT32         mocaLoopbackEn; 
#define MoCA_INIT_PARAM_BO_MODE_MASK                  0x00200000LL
   UINT32         boMode; 
#define MoCA_INIT_PARAM_RF_TYPE_MASK                  0x00400000LL
   UINT32         rfType; 
#define MoCA_INIT_PARAM_TERMINAL_INTERMEDIATE_TYPE_MASK 0x00800000LL
   UINT32         terminalIntermediateType; 
#define MoCA_INIT_PARAM_RESERVED_INIT_0_MASK          0x01000000LL  // unused, no mmp res1
   UINT32         res0;
#define MoCA_INIT_PARAM_BEACON_CHANNEL_MASK           0x02000000LL
   UINT32         beaconChannel;
#define MoCA_INIT_PARAM_MR_NON_DEF_SEQ_NUM_MASK       0x04000000LL
   UINT32         mrNonDefSeqNum; 
#define MoCA_INIT_PARAM_LOW_PRI_Q_NUM_MASK            0x08000000LL
   UINT32         lowPriQNum;
#define MoCA_INIT_PARAM_EGR_MC_FILTER_EN_MASK         0x10000000LL
   UINT32         egrMcFilterEn;
#define MoCA_INIT_PARAM_QAM256_CAPABILITY_MASK        0x20000000LL
   UINT32         qam256Capability; 
#define MoCA_INIT_PARAM_CONTINUOUS_RX_MODE_ATTN_MASK  0x40000000LL
   signed int     continuousRxModeAttn;
#define MoCA_INIT_PARAM_RESERVED_INIT_1_MASK          0x80000000LL
   UINT32         reservedInit1; 
#define MoCA_INIT_PARAM_RESERVED_INIT_2_MASK         0x100000000LL
   UINT32         reservedInit2; 
#define MoCA_INIT_PARAM_RESERVED_INIT_3_MASK         0x200000000LL
   UINT32         reservedInit3; 
#define MoCA_INIT_PARAM_RESERVED_INIT_4_MASK         0x400000000LL
   UINT32         reservedInit4; 
#define MoCA_INIT_PARAM_RESERVED_INIT_5_MASK         0x800000000LL
   UINT32         reservedInit5;
#define MoCA_INIT_PARAM_OPTIONS_MASK                0x1000000000LL
   MoCA_INIT_OPTIONS initOptions;
#define MoCA_INIT_PARAM_FREQ_MASK_MASK              0x2000000000LL
   UINT32         freqMask;
#define MoCA_INIT_PARAM_OTF_EN_MASK                 0x4000000000LL
   UINT32         otfEn;
#define MoCA_INIT_PARAM_TURBO_EN_MASK               0x8000000000LL
   UINT32         turboEn;
#define MoCA_INIT_PARAM_BEACON_PWR_REDUCTION_MASK  0x10000000000LL
   UINT32         beaconPwrReduction;
#define MoCA_INIT_PARAM_BEACON_PWR_REDUCTION_EN_MASK 0x20000000000LL
   UINT32         beaconPwrReductionEn;
#define MoCA_INIT_PARAM_FLOW_CONTROL_EN_MASK       0x40000000000LL
   UINT32         flowControlEn;
#define MoCA_INIT_PARAM_MTM_EN_MASK                0x80000000000LL
   UINT32         mtmEn;
#define MoCA_INIT_PARAM_PNS_FREQ_MASK_MASK        0x100000000000LL
   UINT32         pnsFreqMask;

} MoCA_INITIALIZATION_PARMS, *PMoCA_INITIALIZATION_PARMS;


/* Masks defined here are used in selecting the required parameter from Mgmt
 * application.
 */
typedef struct   _PrioAllocations {
   UINT32   resvHigh;
   UINT32   resvMed;
   UINT32   resvLow;
   UINT32   limitHigh;
   UINT32   limitMed;
   UINT32   limitLow;
} MoCAPrioAllocations;

typedef struct _MCAddrFilter
{
    UINT32 EntryId;
    UINT32 Valid;
    UINT32 AddrHi;
    UINT32 AddrLo;    
} MoCAMCAddrFilter;

typedef struct _IqDiagramSet {
   UINT32   nodeId;     /**< 0-15 */
   UINT32   burstType;  /**< 1 = Beacon, 6 = MAP, 7 = Unicast, 8 = Broadcast, 0,2,3,4,5 = Reserved */
   UINT32   acmtSymNum; /**< 1 = last sym, 2 = sym before last, 3..10 = prev sym */
} MoCAIqDiagramSet;

typedef struct _LabRegMem {
   UINT32   input;
   UINT32   len;
   UINT32   value[48];
} MoCALabRegMem;

typedef struct _LabCallFunc {
   UINT32   funcAddr;
   UINT32   param1;
   UINT32   param2;
   UINT32   param3;
   UINT32   numParams;
} MoCALabCallFunc;

typedef struct _LabTPCAP {
   UINT32   enable;
   UINT32   type;
} MoCALabTPCAP;

#define MoCA_CFG_MAX_PARAMS                     64
#define MoCA_CFG_PARAM_ALL_MASK               0x03F007FFFFF7FFFELL

#define MoCA_CFG_NON_LAB_PARAM_ALL_MASK            0x7FFFFB7FFEELL    /* Due to issue with OutOfOrderLmo and
                                                                 core trace control */

#define MoCA_MAX_SNR_TBL_INDEX                  11
#define MoCA_MAX_SAPM_TBL_INDEX                 112
#define MoCA_MAX_RLAPM_TBL_INDEX                66

typedef struct _MoCAConfigParams {
#define MoCA_CFG_PARAM_MAX_FRAME_SIZE_MASK            0x00000002LL
   UINT32         maxFrameSize;
#define MoCA_CFG_PARAM_MAX_TRANS_TIME_MASK            0x00000004LL
   UINT32         maxTransmitTime;
#define MoCA_CFG_PARAM_MIN_BW_ALARM_THRE_MASK         0x00000008LL
   UINT32         minBwAlarmThreshold;
#define MoCA_CFG_PARAM_OUT_OF_ORDER_LMO_MASK          0x00000010LL
   UINT32         outOfOrderLmo;
#define MoCA_CFG_PARAM_RES_1_MASK                     0x00000020LL
   UINT32               res1 ;
#define MoCA_CFG_PARAM_RES_2_MASK                     0x00000040LL
   UINT32               res2 ;
#define MoCA_CFG_PARAM_RES_3_MASK                     0x00000080LL
   UINT32               res3 ;
#define MoCA_CFG_PARAM_CONT_IE_RR_INS_MASK            0x00000100LL
   UINT32         continuousIERRInsert;
#define MoCA_CFG_PARAM_CONT_IE_MAP_INS_MASK           0x00000200LL
   UINT32         continuousIEMapInsert;
#define MoCA_CFG_PARAM_MAX_PKT_AGGR_MASK              0x00000400LL
   UINT32         maxPktAggr;
#define MoCA_CFG_PARAM_MAX_CONSTELLATION_MASK         0x00000800LL
   UINT32         constellation[MoCA_MAX_NODES];
#define MoCA_CFG_PARAM_RESV3_MASK                     0x00001000LL
   UINT32         resv3;
#define MoCA_CFG_PARAM_PMK_EXCHG_INTVL_MASK           0x00002000LL
   UINT32         pmkExchangeInterval;
#define MoCA_CFG_PARAM_TEK_EXCHG_INTVL_MASK           0x00004000LL
   UINT32         tekExchangeInterval;
#define MoCA_CFG_PARAM_PRIO_ALLOCATIONS_MASK          0x00008000LL
   MoCAPrioAllocations   prioAllocation;
#define MoCA_CFG_PARAM_SNR_MARGIN_MASK                0x00010000LL
   signed int     snrMargin;
#define MoCA_CFG_PARAM_MIN_MAP_CYCLE_MASK             0x00020000LL
   UINT32         minMapCycle;
#define MoCA_CFG_PARAM_MAX_MAP_CYCLE_MASK             0x00040000LL
   UINT32         maxMapCycle;
#define MoCA_CFG_PARAM_TARGET_PHY_RATE_QAM128_MASK    0x00100000LL
   UINT32         targetPhyRateQAM128;
#define MoCA_CFG_PARAM_TARGET_PHY_RATE_QAM256_MASK    0x00200000LL
   UINT32         targetPhyRateQAM256;
#define MoCA_CFG_PARAM_CORE_TRACE_CONTROL_MASK        0x00400000LL
   UINT32         coreTraceControl;
#define MoCA_CFG_PARAM_SNR_MARGIN_OFFSET_MASK         0x00800000LL
   signed int     snrMarginOffset [MoCA_MAX_SNR_TBL_INDEX];
#define MoCA_CFG_PARAM_SAPM_EN_MASK                   0x01000000LL
    UINT32        sapmEn;
#define MoCA_CFG_PARAM_ARPL_TH_MASK                   0x02000000LL
    signed int    arplTh;
#define MoCA_CFG_PARAM_SAPM_TABLE_MASK                0x04000000LL
    struct _sapmTable {
      UINT8          sapmTableLo[MoCA_MAX_SAPM_TBL_INDEX];
      UINT8          sapmTableHi[MoCA_MAX_SAPM_TBL_INDEX];
    } sapmTable;
#define MoCA_CFG_PARAM_RLAPM_EN_MASK                  0x08000000LL
     UINT32       rlapmEn;
#define MoCA_CFG_PARAM_RLAPM_TABLE_MASK               0x10000000LL
     UINT8        rlapmTable[68];
#define MoCA_CFG_PARAM_PSS_EN_MASK                    0x20000000LL
      UINT32      pssEn;
#define MoCA_CFG_PARAM_FREQ_SHIFT_MASK                0x40000000LL
      UINT32      freqShiftMode;
#define MoCA_CFG_PARAM_EGR_MC_ADDR_FILTER_MASK        0x80000000LL
      MoCAMCAddrFilter     mcAddrFilter[MOCA_MAX_EGR_MC_FILTERS];
#define MoCA_CFG_PARAM_RX_TX_PACKETS_PER_QM_MASK     0x100000000LL
      UINT32      rxTxPacketsPerQM;
#define MoCA_CFG_PARAM_EXTRA_RX_PACKETS_PER_QM_MASK  0x200000000LL
      UINT32      extraRxPacketsPerQM;
#define MoCA_CFG_PARAM_RX_POWER_TUNING_MASK          0x400000000LL
      SINT32      rxPowerTuning;
#define MoCA_CFG_PARAM_EN_CAPABLE_MASK               0x800000000LL
      UINT32      enCapable;         
#define MoCA_CFG_PARAM_MIN_AGGR_WAIT_TIME_MASK      0x1000000000LL
      UINT32      minAggrWaitTime;         
#define MoCA_CFG_PARAM_DIPLEXER_MASK                0x2000000000LL
      SINT32      diplexer;
#define MoCA_CFG_PARAM_EN_MAX_RATE_IN_MAX_BO_MASK   0x4000000000LL
      UINT32      enMaxRateInMaxBo;
#define MoCA_CFG_PARAM_TARGET_PHY_RATE_TURBO_MASK   0x8000000000LL
      UINT32      targetPhyRateTurbo;
#define MoCA_CFG_PARAM_TARGET_PHY_RATE_TURBO_PLUS_MASK   0x10000000000LL
      UINT32      targetPhyRateTurboPlus;
#define MoCA_CFG_PARAM_NBAS_CAPPING_EN_MASK        0x20000000000LL
      UINT32      nbasCappingEn;
#define MoCA_CFG_PARAM_SELECTIVE_RR_MASK           0x40000000000LL
         UINT32      selectiveRR;


   /* All these lab configurations should appear in the end of the
    * structure, Important restriction to keep in mind.
    */
#define MoCA_CFG_PARAM_LAB_START_MASK                 MoCA_CFG_PARAM_LAB_PILOTS_MASK
#define MoCA_CFG_PARAM_LAB_PILOTS_MASK               0x0010000000000000LL
   UINT8             pilots [8];
#define MoCA_CFG_PARAM_IQ_DIAGRAM_SET_MASK           0x0020000000000000LL
   MoCAIqDiagramSet  iq;
#define MoCA_CFG_PARAM_SNR_GRAPH_SET_MASK            0x0040000000000000LL
   UINT32            MoCASnrGraphSetNodeId;
#define MoCA_CFG_PARAM_LAB_REG_MEM_MASK              0x0080000000000000LL
   MoCALabRegMem     RegMem;
#define MoCA_CFG_PARAM_LAB_CALL_FUNC_MASK            0x0100000000000000LL
   MoCALabCallFunc   callFunc;
#define MoCA_CFG_PARAM_LAB_TPCAP_MASK                0x0200000000000000LL
   MoCALabTPCAP      labTPCAP;

   /* Following are the translated values from User Anytime parameter to
    * MoCA core anytime parameter.
    * User Management application does not need to be bothered with this. This
    * is used by the MoCA control Library.
    */
   struct _snrMarginTable {
     signed char         mgnTable [12];
   } __attribute__((packed,aligned(4))) snrMarginTable;
} MoCA_CONFIG_PARAMS, *PMoCA_CONFIG_PARAMS;


#define NUM_FUNC_CALL_OPTIONS 3
typedef struct {
   UINT32 Address;
   UINT32 param1;
   UINT32 param2;
   UINT32 param3;
   UINT32 numParams;
} MoCA_FUNC_CALL_PARAMS, *PMoCA_FUNC_CALL_PARAMS;


typedef struct {
   UINT32 enable;
   UINT32 type;
} MoCA_TPCAP_PARAMS, *PMoCA_TPCAP_PARAMS;


#define MoCA_LOW_PRIO            0
#define MoCA_MED_PRIO            1
#define MoCA_HIGH_PRIO           2

#define MoCA_ETH_MIN_PRIO        0
#define MoCA_ETH_LOW_PRIO_MAX    3
#define MoCA_ETH_MED_PRIO_MAX    5
#define MoCA_ETH_HIGH_PRIO_MAX   7
#define MoCA_ETH_MAX_PRIO        7
#define MoCA_ETH_NUM_PRIO        8

/* MoCA standard recommendation.
 * Eth Prio 0,1,2,3 maps to MoCA_LOW_PRIO.
 * Eth Prio 4,5 maps to MoCA_MED_PRIO.
 * Eth Prio 6,7 maps to MoCA_HIGH_PRIO.
 */
typedef struct _MoCAQoSParamsEntry {
   UINT32        tciPri;
   UINT32        MoCAPri;
} MoCA_QOS_PARAMS_ENTRY, *PMoCA_QOS_PARAMS_ENTRY;

typedef struct _MoCAUCFwdEntry {
   UINT32        nodeMacAddr[2];
   UINT32        ucDestNodeId;
} MoCA_UC_FWD_ENTRY, *PMoCA_UC_FWD_ENTRY;

typedef struct _NodeMacAddr {
   UINT32        macAddr[2];
} sNodeMacAddr;

typedef struct _MoCAMCFwdEntry {
   UINT32        numOfMembers;   /**< number of node mac addresses */

   UINT32        mcMacAddr[2];
   sNodeMacAddr  nodeAddr [MoCA_NUM_MEMBERS_PER_MC_GROUP];

   UINT32        destNodeId;      /**< Read-only, For write accesses, it is dont care */
} MoCA_MC_FWD_ENTRY, *PMoCA_MC_FWD_ENTRY;

typedef struct _MoCASrcAddrEntry {
   UINT32        nodeSrcMacAddr[2];  /**< Node address sourced from this device */
   UINT32        selfNodeId;         /**< Self Node id */
} MoCA_SRC_ADDR_ENTRY, *PMoCA_SRC_ADDR_ENTRY;

/** MoCA LED Status. This is a bit field which indicates the current MoCA LED
 *  status. The MSB that is set indicates what the LED is currently doing. Any other
 *  bits that are set indicate what the LED will do next. For example if the LED
 *  status has bits MoCA_LED_STATUS_FAST_BLINK and MoCA_LED_STATUS_ON bits set, the
 *  LED is currently blinking quickly (i.e. for traffic) and will be lit once the 
 *  fast blinking stops.
 */
typedef enum _MoCALedStatusType {
   MoCA_LED_STATUS_OFF              = 0x0,      /**< LED is off */
   MoCA_LED_STATUS_ON               = (1 << 0), /**< LED is on */
   MoCA_LED_STATUS_SLOW_BLINK       = (1 << 1), /**< LED is blinking slowly */
   MoCA_LED_STATUS_FAST_BLINK       = (1 << 2), /**< LED is blinking quickly */
} MoCA_LED_STATUS_TYPE;


typedef struct _MoCAStatus {
   /* General status */
   struct _generalStatus {
     UINT32       vendorId;            /**< BRCM vendor ID */
     UINT32       hwVersion;           /**< chip type of device */
     UINT32       swVersion;           /**< MoCA firmware version */ 
     UINT32       selfMoCAVersion;     /**< MoCA version 1.0 or 1.1 support */
     UINT32       networkVersionNumber;/**< Current MoCA beacon version of network (1.0 or 1.1) - valid only if linkStatus is UP */
     UINT32       qam256Support;       /**< Indicates whether or not qam256 is supported by this device */
     UINT32       operStatus;          /**< Indicates whether this MoCA interface is operational or not */
     UINT32       linkStatus;          /**< Indicates if MoCA network is up or down */
     UINT32       connectedNodes;      /**< Number of nodes present on MoCA network */
     UINT32       nodeId;              /**< Node ID of this device on MoCA network */
     UINT32       ncNodeId;            /**< Node ID of network controller on MoCA network */
     UINT32       backupNcId;          /**< Node ID of backup network controller on MoCA network */
     UINT32       rfChannel;           /**< Operating frequency of MoCA network in MHz */
     UINT32       bwStatus;            /**< Bits 0-1: Node ID 0, Bits 2-3: Node ID 1 ... Bits 30-31: Node ID 15
                                             0 = Unusable channel
                                             1 = Good BW
                                             2 = Low BW, according to MIN_BW_ALARM_THRESHOLD
                                             3 = No information */
     UINT32       nodesUsableBitmask;  /**< Indicate the numbers of 1's in the GCD_BITMASK field  reported in Type I Probe Reports. 
                                            This value corresponds to the number of nodes that this node communicates to in the MoCA network. 
                                            This value may be smaller than the number of nodes reported by the NC node. */
     UINT32       networkTabooMask;    /**< Taboo mask from MoCA network beacon */
     UINT32       networkTabooStart;   /**< Taboo start field from MoCA network beacon */
     UINT32       txGcdPowerReduction; /**< The Transmit Power Control back-off used for broadcast transmissions from this node */
     UINT32       pqosEgressNumFlows;  /**< Number of PQoS Flows in which this node is an Egress node */
     UINT32       ledStatus;           /**< Current MoCA LED Setting, see MoCA_LED_STATUS_TYPE */
   } generalStatus;

   /* Extended Status */
   struct _extendedStatus {
      // keep these keys at the top of the struct
     UINT8        pmkEvenKey [MoCA_MAX_KEY_LEN];   /**< Current PMK even key */
     UINT8        pmkOddKey [MoCA_MAX_KEY_LEN];    /**< Current PMK odd key */
     UINT8        tekEvenKey [MoCA_MAX_KEY_LEN];   /**< Current TEK even key */
     UINT8        tekOddKey [MoCA_MAX_KEY_LEN];    /**< Current TEK odd key */

      // these are determined locally (not from firmware)
     UINT32       lastTekExchange; /**< Time in seconds starting at 0 from system boot when last TEK exchange occurred */
     UINT32       lastTekInterval; /**< Duration of last TEK validity in seconds */     
     UINT32       tekEvenOdd;      /**< Indicates whether current TEK is even or odd, 0=even, 1=odd */
     UINT32       lastPmkExchange; /**< Time in seconds starting at 0 from system boot when last PMK exchange occurred */
     UINT32       lastPmkInterval; /**< Duration of last PMK validity in seconds */     
     UINT32       pmkEvenOdd;      /**< Indicates whether current PMK is even or odd, 0=even, 1=odd */
   } extendedStatus;

   /* Misc Status */
   struct _miscStatus {
     MAC_ADDRESS  macAddr;        /**< The MAC address of this MoCA interface */
     UINT32       isNC;           /**< Indicates whether or not this node is the network controller */
     UINT32       driverUpTime;   /**< Time In Secs. that MoCA driver has been running */
     UINT32       MoCAUpTime;     /**< Time In Secs. that MoCA core has been running since last reset */
     UINT32       linkUpTime;     /**< Time In Secs. that current MoCA link has been established 
                                       (if link is down, indicates the time that previous link was up for). */
     UINT32       linkResetCount; /**< Not used */
     UINT32       lmoInfoAvailable;/**< Not used */
   } miscStatus;

} MoCA_STATUS, *PMoCA_STATUS;

/* Modes of statistics retrieval. INTERNAL is meant for manipulation of
 * 64-bit octet counters.
 */
#define MoCA_INTERNAL        1
#define MoCA_EXTERNAL        2

#define MoCA_NUM_AGGR_PKT_COUNTS 10
typedef struct _MoCAStatistics {
   /* General Stats */
   struct _generalStats {
     UINT32       inUcPkts;         /**< Number of unicast packets sent from this node into the MoCA network */
     UINT32       inDiscardPktsEcl; /**< Number of packets to be sent into the MoCA network that were dropped at ECL layer */
     UINT32       inDiscardPktsMac; /**< Number of packets to be sent into the MoCA network that were dropped at MAC layer */
     UINT32       inUnKnownPkts;    /**< Number of packets sent into the MoCA network destined to an unknown node */
     UINT32       inMcPkts;         /**< Number of multicast packets sent from this node into the MoCA network */
     UINT32       inBcPkts;         /**< Number of broadcast packets sent from this node into the MoCA network */
     UINT32       inOctets_low;     /**< Count of octets sent from this node. Lower 32-bits. Upper 32-bits in inOctets_hi */
     UINT32       outUcPkts;        /**< Number of unicast packets received by this node out from the MoCA network */
     UINT32       outDiscardPkts;   /**< Number of packets received by this node out from the MoCA network in error (i.e. CRC) */
     UINT32       outBcPkts;        /**< Number of broadcast packets received by this node out from the MoCA network */
     UINT32       outOctets_low;    /**< Count of octets received by this node. Lower 32-bits. Upper 32-bits in outOctets_hi */
     UINT32       ncHandOffs;       /**< Count of the number of NC hand-offs that have occurred in the network */
     UINT32       ncBackups;        /**< Count of the number of NC back-ups in the network */
     UINT32       aggrPktStatsTx [MoCA_NUM_AGGR_PKT_COUNTS]; /**< Count of number of times packets were sent per aggregation number. 
                                                                  aggrPktStatsTx[0]=count of times packets sent with no aggregation
                                                                  aggrPktStatsTx[1]=count of times packets sent 2 at a time, etc. */
     UINT32       aggrPktStatsRx [MoCA_NUM_AGGR_PKT_COUNTS]; /**< Count of number of times packets received per aggregation number. 
                                                                  aggrPktStatsRx[0]=count of times packets received with no aggregation
                                                                  aggrPktStatsRx[1]=count of times packets received 2 at a time, etc. */
     UINT32       receivedDataFiltered; /**< Count of number of packets filtered due to MC EGR filtering */
   } generalStats;

   /* 64Bit Stats */
   struct _BitStats64 {
     UINT32       inOctets_hi;      /**< Count of octets sent from this node. Upper 32-bits. */
     UINT32       outOctets_hi;     /**< Count of octets received by this node. Upper 32-bits. */
   } BitStats64;

   /* Extended Stats */
   struct _extendedStats {
     UINT32       rxMapPkts;        /**< MAP packets received from MoCA network */
     UINT32       rxRRPkts;         /**< Reservation requests received from MoCA network */
     UINT32       rxBeacons;        /**< Beacons received from MoCA network */
     UINT32       rxCtrlPkts;       /**< Link control packets received from MoCA network */
     UINT32       txBeacons;        /**< Number of beacons transmitted to the MoCA network */
     UINT32       txMaps;           /**< Number of maps transmitted to the MoCA network */
     UINT32       txLinkCtrlPkts;   /**< Number of link control packets transmitted to the MoCA network */
     UINT32       txRRs;            /**< Number of reservation requests transmitted to the MoCA network */
     UINT32       resyncAttempts;   /**< Number entrances into re-sync mode */
     UINT32       gMiiTxBufFull;    /**< Number times interrupt occurred due to lack of buffers in TX queues */
     UINT32       MoCARxBufFull;    /**< Number times interrupt occurred due to lack of buffers in RX queues */
     UINT32       thisHandOffs;     /**< Number times this node became NC through hand-off */
     UINT32       thisBackups;      /**< Number times this node became NC through back-up */
     UINT32       fcCounter[3];     /**< Number of times an FC level was set */
     UINT32       txProtocolIe;     /**< Number of protocol IEs transmitted to the MoCA network */
     UINT32       rxProtocolIe;     /**< Number of protocol IEs received from the MoCA network */
     UINT32       txTimeIe;         /**< Number of time IEs transmitted to the MoCA network */
     UINT32       rxTimeIe;         /**< Number of time IEs received from the MoCA network */
     UINT32       rxLcAdmReqCrcErr; /**< Number of Admission Requests received with CRC errors. If a node has mis-matched
                                         privacy settings, its admission requests will be counted in this field. */

      // dummy value for backwards compatibility with cms on 6816
      UINT32      rxDataCrc;
   } extendedStats;

} MoCA_STATISTICS, *PMoCA_STATISTICS;

typedef struct _MoCANodeExtendedStatus {
   UINT32        nBas;              /**< Number of bits per ACMT symbol */
   UINT32        preambleType;
   UINT32        cp;                /**< Cyclic prefix length */
   UINT32        txPower;           /**< in dBm */
   SINT32        rxGain;            /**< in dBm */
   UINT32        bitLoading [MoCA_MAX_SUB_CARRIERS * MAX_BITS_PER_SUB_CARRIER/(BITS_PER_BYTE * BYTES_PER_WORD)]; /**< bits per sub-carrier, 4-bits per sub-carrier */
   UINT32        avgSnr;            /**< in dB */
   UINT32        turbo;
} MoCA_NODE_EXTENDED_STATUS;

typedef struct _MoCANodeRates {
   UINT32        txUcPhyRate;
   UINT32        rxUcPhyRate;
   UINT32        rxBcPhyRate;
   UINT32        rxMapPhyRate;
} MoCA_NODE_RATES;

typedef struct _MoCANodeCommonRates {
   UINT32        txBcPhyRate;
   UINT32        txMapPhyRate;
} MoCA_NODE_COMMON_RATES;

typedef struct _MoCANodeStatusEntry {
   /* Basic Status */
   UINT32                     nodeId;
   UINT32                     eui[2];  /* MAC Address of node */
   signed int                 freqOffset;
   UINT32                     otherNodeUcPwrBackOff;
   UINT32                     protocolSupport;

   /* Extended Status */
   MoCA_NODE_EXTENDED_STATUS  txUc;

   MoCA_NODE_EXTENDED_STATUS  rxUc;
   MoCA_NODE_EXTENDED_STATUS  rxBc;
   MoCA_NODE_EXTENDED_STATUS  rxMap;
   MoCA_NODE_RATES            maxPhyRates;
} MoCA_NODE_STATUS_ENTRY, *PMoCA_NODE_STATUS_ENTRY;

typedef struct _MoCANodeCommonStatusEntry {
   MoCA_NODE_EXTENDED_STATUS  txBc;
   MoCA_NODE_EXTENDED_STATUS  txMap;
   MoCA_NODE_COMMON_RATES     maxCommonPhyRates;
} MoCA_NODE_COMMON_STATUS_ENTRY, *PMoCA_NODE_COMMON_STATUS_ENTRY;

typedef struct _MoCANodeStatisticsEntry {
   UINT32        nodeId;
/* txPkts needs to be the first field, ordering cannot change, as some generalization
 * is done in the code based on this */
   UINT32        txPkts;            /**< number of packets sent to nodeId */
   UINT32        rxPkts;            /**< number of received from nodeId */
   UINT32        rxCwUnError;       /**< number of unerrored codewords received from nodeId */
   UINT32        rxCwCorrected;     /**< number of errored and corrected codewords received from nodeId */
   UINT32        rxCwUncorrected;   /**< number of errored and uncorrected codewords received from nodeId */
   UINT32        rxNoSync;          /**< number of received bursts with a "no sync" error from nodeId */
   UINT32        rxNoEnergy;        /**< number of received bursts with a "no energy" error from nodeId */
} MoCA_NODE_STATISTICS_ENTRY, *PMoCA_NODE_STATISTICS_ENTRY;

typedef struct _MoCANodeStatisticsExtEntry {
   UINT32        nodeId;
   UINT32        rxUcCrcError;
   UINT32        rxUcTimeoutError;
   UINT32        rxBcCrcError;
   UINT32        rxBcTimeoutError;
   UINT32        rxMapCrcError;
   UINT32        rxMapTimeoutError;
   UINT32        rxBeaconCrcError;
   UINT32        rxBeaconTimeoutError;
   UINT32        rxRrCrcError;
   UINT32        rxRrTimeoutError;
   UINT32        rxLcCrcError;
   UINT32        rxLcTimeoutError;
   UINT32        rxP1Error;
   UINT32        rxP2Error;
   UINT32        rxP3Error;
   UINT32        rxP1GcdError;
} MoCA_NODE_STATISTICS_EXT_ENTRY, *PMoCA_NODE_STATISTICS_EXT_ENTRY;



/* Trace Configuration Parameters. */
typedef struct _MoCATraceParams {
   UINT32        traceLevel;      /**< Mask Value of the trace configuration level */
   UINT32        bRestoreDefault; /**< Force default trace level */
} MoCA_TRACE_PARAMS, *PMoCA_TRACE_PARAMS;


/** Version Information */
typedef struct _MoCAVersion {
   UINT32        coreHwVersion;     /**< Chip ID of device */
   UINT32        coreSwVersion;     /**< MoCA Core software version. Bits 31-28=major version, bits 27-24=minor version, bits 23-0=build number */
   UINT32        nwVersion;         /**< MoCA network version number. 0x10 = 1.0 MoCA version, 0x11 = 1.1 MoCA version */
   UINT32        selfVersion;       /**< MoCA version supported by this interface */
   UINT16        drvMnVersion;      /**< MoCA driver software minor version number */
   UINT16        drvMjVersion;      /**< MoCA driver software major version number */
   UINT32        drvBuildVersion;   /**< MoCA driver software build version number */
} MoCA_VERSION, *PMoCA_VERSION;

/* Asynchronous Notification Information.
 */

typedef struct _MoCAAsyncNotifyParams {
   signed int        userMoCADesc;
} MoCA_ASYNC_NOTIFY_PARAMS, *PMoCA_ASYNC_NOTIFY_PARAMS;

/** Asynchronous Information. All the traps that are sent to user are
 * detailed here.
 */
typedef enum _MoCAAsyncType {
   MOCA_EVENTS_LINK_STATE           = 0x1,   /**< Signals a MoCA link transition */
   MOCA_EVENTS_ADMISSION_FAILED     = 0x2,   /**< Reported by the NN to indicate that its admission to the network failed because the PHY Rate between this NN node and the NC was below the NC's Min Bandwidth Threshold */
   MOCA_EVENTS_LIMITED_BW           = 0x3,   /**< Indicate that the PHY link rate to one of the nodes of the network is below the Min Bandwidth Threshold specified into the General Parameters Table */
   MOCA_EVENTS_ERROR                = 0x4,   /**< Indicates that an error occurred */
   MOCA_EVENTS_LMO_INFO             = 0x5,   /**< Report parameters about the recent LMO cycle. */
   MOCA_EVENTS_KEY_CHANGED          = 0x6,   /**< Report the testbed clock timestamp when each TEK and PMK key refresh takes place. */
   MOCA_EVENTS_TOPOLOGY_CHANGED     = 0x7,   /**< Report if a node joined/dropped from the network. */
   MOCA_EVENTS_MOCA_VERSION_CHANGED = 0x8,   /**< Reporting a change in the Beacon MOCA_VERSION field. */
   MOCA_MAX_EVENTS
} MoCA_ASYNC_TYPE;

typedef struct _MoCAAsyncInfo {
   MoCA_ASYNC_TYPE   type;
   union _info {
     UINT32 u32Val;
     struct _lmoInfo {
       UINT32 nodeId;
       UINT32 durationSec;
       UINT32 isLmoSuccess;
     } lmoInfo;
     struct _keyChange {
       UINT32 tekPmk;
       UINT32 evenOdd;
     } keyChanged;
     struct _topologyChange {
       UINT32 nodeId;
       UINT32 joinedDropped;
     } topologyChanged;
     struct _versionChange {
       UINT32 newVersion;
     } versionChange;
   } uInfo;
} MoCA_ASYNC_INFO, *PMoCA_ASYNC_INFO;

#define MoCA_ALL_SUBCARRIER       MoCA_MAX_SUB_CARRIERS+1
#define MoCA_CARRIER_IQ_SIZE      4     /* 4 bytes */

typedef struct _MoCAIQData {
   UINT8      *pData;
   UINT32      size;
   UINT32      subCarrier;
} MoCA_IQ_DATA, *PMoCA_IQ_DATA;

#define MoCA_MAX_PROBES          8
#define MoCA_ALL_PROBES          MoCA_MAX_PROBES+1
#define MoCA_PROBE_POINTS        256
#define MoCA_POINT_SIZE          4     /* 4 bytes */

typedef struct _MoCASNRData {
   UINT8      *pData;
   UINT32      size;
   UINT32      probe;
} MoCA_SNR_DATA, *PMoCA_SNR_DATA;

typedef struct _MoCACIRData {
   UINT8      *pData;
   UINT32      size;
   UINT32      nodeId;
} MoCA_CIR_DATA, *PMoCA_CIR_DATA;

typedef struct _MoCA_FMR_PARAMS {
   UINT32   address[2];
} MoCA_FMR_PARAMS, *PMoCA_FMR_PARAMS;


#define MoCA_MAX_PQOS_LISTENERS        4
#define MoCA_PQOS_PEAK_RATE_DEFAULT    1000
#define MoCA_PQOS_PACKET_SIZE_DEFAULT  800
#define MoCA_PQOS_BURST_COUNT_DEFAULT  2
#define MoCA_PQOS_LEASE_TIME_DEFAULT   0
#define MoCA_PQOS_VLAN_ID_DEFAULT      0xFFF
#define MoCA_PQOS_VLAN_ID_MAX          0xFFF
#define MoCA_PQOS_VLAN_PRIO_DEFAULT    0xFF
#define MoCA_PQOS_VLAN_PRIO_MAX        7

typedef struct _MoCA_CREATE_PQOS_PARAMS {
   MAC_ADDRESS   packet_da;
   MAC_ADDRESS   ingress_node;
   MAC_ADDRESS   egress_node;
   MAC_ADDRESS   talker;
   MAC_ADDRESS   listeners[MoCA_MAX_PQOS_LISTENERS];
   UINT32        peak_rate;
   UINT32        packet_size;
   UINT32        burst_count;
   int           lease_time;
   UINT32        flow_tag;
   UINT32        vlan_id;
   UINT32        vlan_prio;
   MAC_ADDRESS   flow_id;
} MoCA_CREATE_PQOS_PARAMS, *PMoCA_CREATE_PQOS_PARAMS;

typedef struct _MoCA_UPDATE_PQOS_PARAMS {
   MAC_ADDRESS   flow_id;
   UINT32        peak_rate;
   UINT32        packet_size;
   UINT32        burst_count;
   int           lease_time;
   UINT32        flow_tag;
   MAC_ADDRESS   packet_da;
} MoCA_UPDATE_PQOS_PARAMS, *PMoCA_UPDATE_PQOS_PARAMS;

typedef struct _MoCA_DELETE_PQOS_PARAMS {
   MAC_ADDRESS   flow_id;
} MoCA_DELETE_PQOS_PARAMS, *PMoCA_DELETE_PQOS_PARAMS;

typedef struct _MoCA_LIST_PQOS_PARAMS {
   MAC_ADDRESS   ingress_mac;
   UINT32        node_id;      /**< To use node_id, set ingress_mac to 0 */
} MoCA_LIST_PQOS_PARAMS, *PMoCA_LIST_PQOS_PARAMS;

typedef struct _MoCA_QUERY_PQOS_PARAMS {
   MAC_ADDRESS   flow_id;
} MoCA_QUERY_PQOS_PARAMS, *PMoCA_QUERY_PQOS_PARAMS;

typedef struct _MoCA_MR_PARAMS {
   UINT32        reset_status;
   UINT32        reset_timer;
   UINT32        non_def_seq_num;
} MoCA_MR_PARAMS, *PMoCA_MR_PARAMS;


/** Opens handle to MoCA driver
 *
 * This function returns a handle to the MoCA driver associated with
 * the given 'ifname'. The handle should then be used as an input
 * parameter to MoCA APIs that require a handle. When finished with
 * the handle, call MoCACtl_Close to free memory.
 *
 * @param ifname (IN) - A pointer to the MoCA interface name string, 
 *                      i.e. "moca0" or "moca1".
 *
 * @return MoCA handle or NULL if unsuccessful.
 */
void * MoCACtl_Open(char * ifname);


/** Closes handle to MoCA driver
 *
 * This function closes the handle to a MoCA driver and frees 
 * associated memory.
 *
 * @param handle (IN) - handle returned by previous MoCACtl_Open() call.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 */
CmsRet MoCACtl_Close(void * handle);


/** Initializes MOCA driver
 *
 * This function is called to initialize the MOCA driver with init time
 * configuration parameters and optional any time configuration 
 * parameters. This function also starts the MoCA interface. Parameter
 * range checking is done on 'pInitParms' and 'pCfgParams'. An error 
 * will be returned if the supplied parameters are out of the valid range.
 *
 * @param pInitParms (IN) - A pointer to MOCA_INITIALIZATION_PARMS.
 * @param pCfgParams (IN) - A pointer to MoCA_CONFIG_PARAMS. This is an
 *                          optional parameter. Not used if set to NULL
 *                          or if configMask set to 0.
 * @param configMask (IN) - Bit mask of pCfgParms to be applied during 
 *                          initialization. This is an optional parameter.
 *                          Not used if set to 0 or if pCfgParms set to NULL.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INVALID_ARGUMENTS - parameter range checking failed
 *         CMSRET_INVALID_PARAM_VALUE - MoCA password invalid
 */
CmsRet MoCACtl_Initialize( 
   PMoCA_INITIALIZATION_PARMS pInitParms,
   PMoCA_CONFIG_PARAMS pCfgParams, 
   unsigned long long configMask);
CmsRet MoCACtl2_Initialize( 
   void * ctx, 
   PMoCA_INITIALIZATION_PARMS pInitParms,
   PMoCA_CONFIG_PARAMS pCfgParams, 
   unsigned long long configMask);

/** Uninitializes MOCA driver
 *
 * This function is called to uninitialize MOCA driver and free resources
 * allocated during initialization. This function stops the MoCA interface.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_Uninitialize(void);
CmsRet MoCACtl2_Uninitialize(void * ctx);


/** ReInitializes MOCA driver
 *
 * This function is called to change the init time MoCA parameters in one
 * step without having to call MoCACtl_Unitialize and MoCACtl_Initialize
 * seperately. This function stops and starts MoCA and updates initialization
 * paramters accoring to the 'reInitMask'. Configuration parameters are updated
 * according to the 'configMask'. Parameter range checking is done on 
 * 'pReInitParms' and 'pCfgParams'. An error will be returned if the supplied 
 * parameters are out of the valid range.
 *
 * @param pReInitParms (IN) - A pointer to MOCA_INITIALIZATION_PARMS.
 * @param reInitMask (IN)   - Bit mask of pReInitParms to be applied during 
 *                            initialization
 * @param pCfgParams (IN)   - A pointer to MoCA_CONFIG_PARAMS. This is an
 *                            optional parameter. Not used if set to NULL
 *                            or if configMask set to 0.
 * @param configMask (IN)   - Bit mask of pCfgParms to be applied during 
 *                            initialization. This is an optional parameter.
 *                            Not used if set to 0 or if pCfgParms set to NULL.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INVALID_ARGUMENTS - parameter range checking failed
 *         CMSRET_INVALID_PARAM_VALUE - MoCA password invalid
 */
CmsRet MoCACtl_ReInitialize( 
   PMoCA_INITIALIZATION_PARMS pReInitParms, 
   unsigned long long reInitMask,
   PMoCA_CONFIG_PARAMS pCfgParams, 
   unsigned long long configMask);
CmsRet MoCACtl2_ReInitialize( 
   void * ctx, 
   PMoCA_INITIALIZATION_PARMS pReInitParms, 
   unsigned long long reInitMask, 
   PMoCA_CONFIG_PARAMS pCfgParams, 
   unsigned long long configMask);


/** Retrieve current initialization parameters 
 *
 * This function fills the supplied MoCA_INITIALIZATION_PARMS pointer
 * with the initialization parameters that are currently in effect for
 * the MoCA interface. 
 *
 * @param pMoCAInitParms (OUT) - Pointer to MoCA_INITIALIZATION_PARMS
 *                               structure. The memory must be allocated
 *                               by the caller.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_GetInitParms(
   PMoCA_INITIALIZATION_PARMS pMoCAInitParms);
CmsRet MoCACtl2_GetInitParms(
   void * ctx,
   PMoCA_INITIALIZATION_PARMS pMoCAInitParms);


/** Set initialization parameters 
 *
 * This sets the MoCA_INITIALIZATION_PARMS for the MoCA interface.
 * The MoCA interface must restarted (MoCACtl2_ReInitialize) in order 
 * for the settings to take effect. 
 *
 * @param pMoCAInitParms (IN) - Pointer to MoCA_INITIALIZATION_PARMS
 *                              structure.
 * @param initMask (IN) - Bit mask of fields to use from the 
 *                        pMoCAInitParms structure. 
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INVALID_ARGUMENTS - parameter range checking failed
 *         CMSRET_INVALID_PARAM_VALUE - MoCA password invalid
 */
CmsRet MoCACtl2_SetInitParms( 
    void * ctx,
    PMoCA_INITIALIZATION_PARMS pMoCAInitParms, 
    unsigned long long initMask);


/** Retrieve current configuration parameters 
 *
 * This function fills the supplied MoCA_CONFIG_PARAMS pointer
 * with the configuration parameters that are currently in effect for
 * the MoCA interface. 
 *
 * @param pMoCACfgParams (OUT) - Pointer to MoCA_CONFIG_PARAMS structure. 
 *                               The memory must be allocated by the caller.
 * @param configMask (IN) - Bit mask of configuration parameters to be 
 *                          retrieved.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INTERNAL_ERROR - error retrieving information from MoCA core
 */
CmsRet MoCACtl_GetCfg(
   PMoCA_CONFIG_PARAMS pMoCACfgParams, 
   unsigned long long configMask);
CmsRet MoCACtl2_GetCfg(
   void * ctx,
   PMoCA_CONFIG_PARAMS pMoCACfgParams, 
   unsigned long long configMask);


/** Update current configuration parameters 
 *
 * This function configures the MoCA interface with the supplied 
 * MoCA_CONFIG_PARAMS structure according to the supplied bit mask.
 * Parameters are updated in the MoCA interface immediately and the
 * MoCA interface is not reset. Parameter range checking is done on 
 * 'pCfgParams'. An error will be returned if the supplied parameters 
 * are out of the valid range.
 *
 * @param pMoCACfgParams (IN) - Pointer to MoCA_CONFIG_PARAMS structure
 *                              containing settings to be applied.
 * @param configMask (IN) - Bit mask of configuration parameters to be 
 *                          applied.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INVALID_ARGUMENTS - parameter range checking failed
 */
CmsRet MoCACtl_SetCfg(
   PMoCA_CONFIG_PARAMS pMoCACfgParams, 
   unsigned long long configMask);
CmsRet MoCACtl2_SetCfg(
   void * ctx,
   PMoCA_CONFIG_PARAMS pMoCACfgParams, 
   unsigned long long configMask);



/** Retrieve current Unicast forwarding table
 *
 * This function fills the supplied MoCA_UC_FWD_ENTRY pointer
 * with the unicast forwarding table that is currently in effect for the MoCA 
 * interface. The caller must allocate the memory for 'pMoCAUcFwdParamsTable' 
 * and must allocate at least MoCA_MAX_UC_FWD_ENTRIES entries. The unicast
 * forwarding table shows which MAC addresses are associated with which
 * node IDs on the MoCA network. This table is learned automatically by the 
 * MoCA interface.
 *
 * @param pMoCAUcFwdParamsTable (OUT) - Pointer to MoCA_UC_FWD_ENTRY structure. 
 *                               The memory must be allocated by the caller.
 * @param pulUcFwdTblSize (OUT) - Size of returned table in bytes. The value will
 *                              be a whole multiple of sizeof(MoCA_UC_FWD_ENTRY).
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INTERNAL_ERROR - error retrieving information from MoCA core
 */
CmsRet MoCACtl_GetUcFwdTbl(
   PMoCA_UC_FWD_ENTRY pMoCAUcFwdParamsTable,
   UINT32   *pulUcFwdTblSize);
CmsRet MoCACtl2_GetUcFwdTbl(
   void * ctx,
   PMoCA_UC_FWD_ENTRY pMoCAUcFwdParamsTable,
   UINT32   *pulUcFwdTblSize);


/** Retrieve current Multicast forwarding table
 *
 * This function fills the supplied MoCA_MC_FWD_ENTRY pointer
 * with the multicast forwarding table that is currently in effect for the MoCA 
 * interface. The caller must allocate the memory for 'pMoCAMcFwdParamsTable' 
 * and must allocate at least MoCA_MAX_MC_FWD_ENTRIES entries. The multicast
 * forwarding table shows which MAC addresses are associated with which
 * node IDs on the MoCA network. This table is only used when the initialization
 * parameter mcastMode is set to MoCA_MCAST_NORMAL_MODE. 
 *
 * @param pMoCAMcFwdParamsTable (OUT) - Pointer to MoCA_MC_FWD_ENTRY structure. 
 *                               The memory must be allocated by the caller.
 * @param pulMcFwdTblSize (OUT) - Size of returned table in bytes. The value will
 *                              be a whole multiple of sizeof(MoCA_MC_FWD_ENTRY).
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INTERNAL_ERROR - error retrieving information from MoCA core
 */
CmsRet MoCACtl_GetMcFwdTbl(
   PMoCA_MC_FWD_ENTRY pMoCAMcFwdParamsTable,
   UINT32   *pulMcFwdTblSize);
CmsRet MoCACtl2_GetMcFwdTbl(
   void * ctx,
   PMoCA_MC_FWD_ENTRY pMoCAMcFwdParamsTable,
   UINT32   *pulMcFwdTblSize);


/** Add a multicast forwarding table entry for MoCA interface
 *
 * This functions instructs the MoCA core to forward traffic destined 
 * for specific multicast MAC address to be sent to a particular node
 * or nodes on the MoCA network. The multicast forwarding table is only
 * used when the initialization parameter mcastMode is set to 
 * MoCA_MCAST_NORMAL_MODE. If mcastMode is set to MoCA_MCAST_BCAST_MODE 
 * then all multicast traffic is forwarded to all nodes on the MoCA network
 * at full rate. When mcastMode is set to MoCA_MCAST_NORMAL_MODE, multicast
 * traffic destined to MAC addresses that do not have an entry in the table
 * is limited to 15 pps. Traffic destined to MAC addresses with entries in
 * the table is sent at full rate. Up to four destination node MAC 
 * addresses may be specified for a particular multicast table entry. If 
 * more than 4 destination nodes are to be used, the broadcast MAC address
 * FF:FF:FF:FF:FF:FF should be set as nodeAddr[0].
 *
 * @param pMoCAMcFwdEntry (IN) - Pointer to MoCA_MC_FWD_ENTRY structure. 
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INVALID_PARAM_VALUE - if a multicast address is specified
 *                                      as a node MAC address
 *         CMSRET_INTERNAL_ERROR - error setting entry in MoCA core
 */
CmsRet MoCACtl_CreateMcFwdTblGroup(
   PMoCA_MC_FWD_ENTRY pMoCAMcFwdEntry);
CmsRet MoCACtl2_CreateMcFwdTblGroup(
   void * ctx,
   PMoCA_MC_FWD_ENTRY pMoCAMcFwdEntry);


/** Remove a multicast forwarding table entry for MoCA interface
 *
 * This functions instructs the MoCA core to remove a previously added
 * multicast table entry from its multicast forwarding table. The 
 * pMoCAMcFwdEntry structure should contain the same data that was used
 * to create the table entry with MoCACtl_CreateMcFwdTblGroup(). 
 * The multicast forwarding table is only used when the initialization 
 * parameter mcastMode is set to MoCA_MCAST_NORMAL_MODE. 
 *
 * @param pMoCAMcFwdEntry (IN) - Pointer to MoCA_MC_FWD_ENTRY structure. 
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INVALID_PARAM_VALUE - if a multicast address is specified
 *                                      as a node MAC address
 *         CMSRET_INTERNAL_ERROR - error setting entry in MoCA core
 */
CmsRet MoCACtl_DeleteMcFwdTblGroup(
   PMoCA_MC_FWD_ENTRY pMoCAMcFwdEntry);
CmsRet MoCACtl2_DeleteMcFwdTblGroup(
   void * ctx,
   PMoCA_MC_FWD_ENTRY pMoCAMcFwdEntry);


/** Retrieve current source address table
 *
 * This function fills the supplied MoCA_SRC_ADDR_ENTRY pointer
 * with the source address table that is currently in effect for the MoCA 
 * interface. The caller must allocate the memory for 'pMoCASrcAddrParamsTable' 
 * and must allocate at least MoCA_MAX_SRC_ADDR_ENTRIES entries. The source
 * address table shows which MAC addresses the MoCA core has learned as 
 * devices residing its CPE side of the network. 
 *
 * @param pMoCASrcAddrParamsTable (OUT) - Pointer to MoCA_SRC_ADDR_ENTRY structure. 
 *                               The memory must be allocated by the caller.
 * @param pulSrcAddrTblSize (OUT) - Size of returned table in bytes. The value will
 *                              be a whole multiple of sizeof(MoCA_SRC_ADDR_ENTRY).
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INTERNAL_ERROR - error retrieving information from MoCA core
 */
CmsRet MoCACtl_GetSrcAddrTbl(
   PMoCA_SRC_ADDR_ENTRY pMoCASrcAddrParamsTable,
   UINT32   *pulSrcAddrTblSize);
CmsRet MoCACtl2_GetSrcAddrTbl(
   void * ctx,
   PMoCA_SRC_ADDR_ENTRY pMoCASrcAddrParamsTable,
   UINT32   *pulSrcAddrTblSize);



/** Retrieve current status information of MoCA interface
 *
 * This function fills the supplied MoCA_STATUS pointer with the current
 * status information for the MoCA interface. See MoCA_STATUS definition
 * for details on each field.
 *
 * @param pMoCAStatus (OUT) - Pointer to MoCA_STATUS structure. 
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
 CmsRet MoCACtl_GetStatus(
   PMoCA_STATUS pMoCAStatus);
CmsRet MoCACtl2_GetStatus(
   void * ctx,
   PMoCA_STATUS pMoCAStatus);


/** Retrieve current statistic information of MoCA interface
 *
 * This function fills the supplied MoCA_STATISTICS pointer with 
 * the current stats information for the MoCA interface. See 
 * MoCA_STATISTICS definition for details on each field. To reset
 * all MoCA statistics for the next read, set ulReset to 1.
 *
 * @param pMoCAStats (OUT) - Pointer to MoCA_STATISTICS structure. 
 * 
 * @param ulReset (IN) - Set to 1 to reset the statistic counters
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_GetStatistics(
   PMoCA_STATISTICS pMoCAStats,
   UINT32 ulReset);
CmsRet MoCACtl2_GetStatistics(
   void * ctx,
   PMoCA_STATISTICS pMoCAStats,
   UINT32 ulReset);



/** Retrieve current node status table
 *
 * This function fills the supplied MoCA_NODE_STATUS_ENTRY pointer
 * and MoCA_NODE_COMMON_STATUS_ENTRY pointer with the status data that 
 * is currently available on the MoCA network. The caller must allocate 
 * the memory for 'pNodeStatusEntry' and 'pNodeCommonStatusEntry'.
 * The caller must allocate at least MOCA_MAX_NODES entries for 
 * 'pNodeStatusEntry'. 'pNodeStatusEntry' will contain status information
 * for each of the nodes on the MoCA network. 'pNodeCommonStatusEntry'
 * contains information common to all nodes on the MoCA network.
 *
 * @param pNodeStatusEntry (OUT) - Pointer to MoCA_NODE_STATUS_ENTRY structure. 
 *                               The memory must be allocated by the caller.
 * @param pNodeCommonStatusEntry (OUT) - Pointer to MoCA_NODE_COMMON_STATUS_ENTRY 
 *                               structure. The memory must be allocated by the caller.
 * @param pulNodeStatusTblSize (OUT) - Size of returned pNodeStatusEntry table in bytes. 
 *                               The value will be a whole multiple of 
 *                               sizeof(MoCA_NODE_STATUS_ENTRY).
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_GetNodeTblStatus(
   PMoCA_NODE_STATUS_ENTRY pNodeStatusEntry,
   PMoCA_NODE_COMMON_STATUS_ENTRY pNodeCommonStatusEntry,
   UINT32 *pulNodeStatusTblSize);
CmsRet MoCACtl2_GetNodeTblStatus(
   void * ctx,
   PMoCA_NODE_STATUS_ENTRY pNodeStatusEntry,
   PMoCA_NODE_COMMON_STATUS_ENTRY pNodeCommonStatusEntry,
   UINT32 *pulNodeStatusTblSize);



/** Retrieve current node statistics table
 *
 * This function fills the supplied MoCA_NODE_STATISTICS_ENTRY pointer
 * with the statistics data accumulated for all active nodes on the 
 * MoCA network. The caller must allocate the memory for 'pNodeStatsEntry' 
 * and must allocate at least MOCA_MAX_NODES entries. To reset all MoCA 
 * statistics for the next read, set ulReset to 1.
 *
 * @param pNodeStatsEntry (OUT) - Pointer to MoCA_NODE_STATISTICS_ENTRY structure. 
 *                               The memory must be allocated by the caller.
 * @param pulNodeStatsTblSize (OUT) - Size of returned pNodeStatsEntry table in bytes. 
 *                               The value will be a whole multiple of 
 *                               sizeof(MoCA_NODE_STATISTICS_ENTRY).
 * @param ulReset (IN) - Set to 1 to reset the statistic counters
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_GetNodeTblStatistics(
   PMoCA_NODE_STATISTICS_ENTRY pNodeStatsEntry,
   UINT32 *pulNodeStatsTblSize,
   UINT32 ulReset);
CmsRet MoCACtl2_GetNodeTblStatistics(
   void * ctx,
   PMoCA_NODE_STATISTICS_ENTRY pNodeStatsEntry,
   UINT32 *pulNodeStatsTblSize,
   UINT32 ulReset);


/** Retrieve current node extended statistics table
 *
 * This function fills the supplied MoCA_NODE_STATISTICS_EXT_ENTRY pointer
 * with the extended statistics data accumulated for all active nodes on the 
 * MoCA network. The caller must allocate the memory for 'pNodeStatsEntry' 
 * and must allocate at least MOCA_MAX_NODES entries. To reset all MoCA 
 * statistics for the next read, set ulReset to 1.
 *
 * @param pNodeStatsEntry (OUT) - Pointer to MoCA_NODE_STATISTICS_EXT_ENTRY structure. 
 *                               The memory must be allocated by the caller.
 * @param pulNodeStatsTblSize (OUT) - Size of returned pNodeStatsEntry table in bytes. 
 *                               The value will be a whole multiple of 
 *                               sizeof(MoCA_NODE_STATISTICS_EXT_ENTRY).
 * @param ulReset (IN) - Set to 1 to reset the statistic counters
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl2_GetNodeTblStatisticsExt(
   void * ctx,
   PMoCA_NODE_STATISTICS_EXT_ENTRY pNodeStatsEntry,
   UINT32 *pulNodeStatsTblSize,
   UINT32 ulReset); 



/** Retrieve current node status for a particular node
 *
 * This function fills the supplied MoCA_NODE_STATUS_ENTRY pointer
 * with status data that is currently available for the specified node. 
 * The caller must allocate the memory for 'pNodeStatusEntry'.
 * The caller must specify the node ID by setting the nodeId
 * field of pNodeStatusEntry.
 *
 * @param pNodeStatusEntry (IN/OUT) - Pointer to MoCA_NODE_STATUS_ENTRY structure. 
 *                               The memory must be allocated by the caller.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_GetNodeStatus(
   PMoCA_NODE_STATUS_ENTRY pNodeStatusEntry);
CmsRet MoCACtl2_GetNodeStatus(
   void * ctx,
   PMoCA_NODE_STATUS_ENTRY pNodeStatusEntry);


/** Retrieve current node statistics for a particular node
 *
 * This function fills the supplied MoCA_NODE_STATISTICS_ENTRY pointer
 * with stats data that is currently available for the specified node. 
 * The caller must allocate the memory for 'pNodeStatsEntry'. 
 * The caller must specify the node ID by setting the nodeId
 * field of pNodeStatsEntry. To reset all MoCA statistics for the next 
 * read, set ulReset to 1.
 *
 * @param pNodeStatsEntry (IN/OUT) - Pointer to MoCA_NODE_STATISTICS_ENTRY structure. 
 *                               The memory must be allocated by the caller.
 * @param ulReset (IN) - Set to 1 to reset the statistic counters
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_GetNodeStatistics(
   PMoCA_NODE_STATISTICS_ENTRY pNodeStatsEntry,
   UINT32 ulReset);
CmsRet MoCACtl2_GetNodeStatistics(
   void * ctx,
   PMoCA_NODE_STATISTICS_ENTRY pNodeStatsEntry,
   UINT32 ulReset);


/** Retrieve current node extended statistics for a particular node
 *
 * This function fills the supplied MoCA_NODE_STATISTICS_EXT_ENTRY pointer
 * with stats data that is currently available for the specified node. 
 * The caller must allocate the memory for 'pNodeStatsEntry'. 
 * The caller must specify the node ID by setting the nodeId
 * field of pNodeStatsEntry. To reset all MoCA statistics for the next 
 * read, set ulReset to 1.
 *
 * @param pNodeStatsEntry (IN/OUT) - Pointer to MoCA_NODE_STATISTICS_EXT_ENTRY 
 *                         structure. The memory must be allocated by the caller.
 * @param ulReset (IN) - Set to 1 to reset the statistic counters
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl2_GetNodeStatisticsExt(
   void * ctx,
   PMoCA_NODE_STATISTICS_EXT_ENTRY pNodeStatsEntry,
   UINT32 ulReset);


/** Retrieve version information for the MoCA interface
 *
 * This function fills the supplied MoCA_VERSION pointer
 * with the version information of the MoCA interface. 
 * The caller must allocate the memory for 'pMoCAVersion'. 
 * This function provides information on the chip version,
 * MoCA core software version, MoCA driver software version,
 * MoCA network version and self MoCA version.
 *
 * @param pMoCAVersion (OUT) - Pointer to MoCA_VERSION structure. 
 *                         The memory must be allocated by the caller.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_GetVersion(
   PMoCA_VERSION pMoCAVersion);
CmsRet MoCACtl2_GetVersion(
   void * ctx,
   PMoCA_VERSION pMoCAVersion);


/** Retrieve trace level information for the MoCA interface
 *
 * This function fills the supplied MoCA_TRACE_PARAMS pointer
 * with the debug tracing configuration of the MoCA interface. 
 * The caller must allocate the memory for 'pMoCATraceParams'. 
 * The 'traceLevel' field will return a bit mask of the trace
 * level settings that are currently enabled. See 
 * MOCA_TRC_LEVEL_NONE and other definitions for bit-level
 * meanings. 
 *
 * @param pMoCATraceParams (OUT) - Pointer to MoCA_TRACE_PARAMS structure. 
 *                         The memory must be allocated by the caller.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 */
CmsRet MoCACtl_GetTraceConfig (
   PMoCA_TRACE_PARAMS pMoCATraceParams);
CmsRet MoCACtl2_GetTraceConfig (
   void * ctx,
   PMoCA_TRACE_PARAMS pMoCATraceParams);



/** Set the trace level for the MoCA interface
 *
 * This function sets the trace level with the supplied 
 * MoCA_TRACE_PARAMS pointer for the MoCA interface. 
 * The 'traceLevel' field is a bit mask of the trace
 * level settings to be enabled. See MOCA_TRC_LEVEL_NONE and 
 * other definitions for bit-level meanings. Some trace levels
 * (ERR, WARN, INFO) can not be disabled. To set the trace level
 * back to its default level set the 'bRestoreDefault' field
 * to 1.
 *
 * @param pMoCATraceParams (IN) - Pointer to MoCA_TRACE_PARAMS structure. 
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INTERNAL_ERROR - unable to set requested level
 */
CmsRet MoCACtl_SetTraceConfig (
   PMoCA_TRACE_PARAMS pMoCATraceParams);
CmsRet MoCACtl2_SetTraceConfig (
   void * ctx,
   PMoCA_TRACE_PARAMS pMoCATraceParams);



/** Retrieve IQ constellation diagram information. 
 *
 * This function fills the MoCA_IQ_DATA pointer with the collected
 * IQ constellation data. This functionality is only available in
 * lab mode (set init time parameter labMode to MoCA_LAB_MODE). The
 * MoCA interface must first be configured with the constellation
 * data desired using MoCACtl_SetCfg(). Set the fields of the 
 * 'MoCAIqDiagramSet' structure and MoCA_CFG_PARAM_IQ_DIAGRAM_SET_MASK
 * bit in the configMask variable. Once those values are set this function
 * can be called to retrieve the most recent IQ data. The returned data
 * in the 'pData' field contains 256 samples of 32 bit data, where 
 * MSB 16 bit is I data, & LSB 16 bit is Q data.
 * 
 * @param pMoCAIQData (OUT) - Pointer to MoCA_IQ_DATA structure. 
 *                     pData field must be allocated by caller (256*4 bytes)
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INTERNAL_ERROR - unable to get data
 */
CmsRet MoCACtl_GetIQData(
   PMoCA_IQ_DATA pMoCAIQData);
CmsRet MoCACtl2_GetIQData(
   void * ctx,
   PMoCA_IQ_DATA pMoCAIQData);



/** Retrieve SNR information. 
 *
 * This function fills the MoCA_SNR_DATA pointer with the collected
 * SNR data. This functionality is only available in
 * lab mode (set init time parameter labMode to MoCA_LAB_MODE). The
 * MoCA interface must first be configured with the constellation
 * data desired using MoCACtl_SetCfg(). Set the MoCASnrGraphSetNodeId field
 * and MoCA_CFG_PARAM_SNR_GRAPH_SET_MASK bit in the configMask variable. 
 * Once the node ID is set this function can be called to retrieve 
 * the most recent SNR data. The data is updated following the LMO
 * cycle. 'pData' contains 256 data points (one per sub-carrier) per probes 
 * for 8 probes, 32-bits per point. Invalid sub-carriers contain
 * unknown data. Each data point represents a dB value between 0-40.
 * To convert to dB: -10*log(pData[i]/(2^31))
 * 
 * @param pMoCASNRData (OUT) - Pointer to MoCA_SNR_DATA structure. 
 *                      pData field must be allocated by caller (256*8*4 bytes)
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INTERNAL_ERROR - unable to get data
 */
CmsRet MoCACtl_GetSNRData(
   PMoCA_SNR_DATA pMoCASNRData);
CmsRet MoCACtl2_GetSNRData(
   void * ctx,
   PMoCA_SNR_DATA pMoCASNRData);



/** Retrieve CIR information. 
 *
 * This function fills the MoCA_CIR_DATA pointer with the collected
 * channel impulse response data. This functionality is only available in
 * lab mode (set init time parameter labMode to MoCA_LAB_MODE). The 'nodeId'
 * must be set by the caller. The data is updated after each LMO cycle with
 * the node. There are 256 32-bit data points. MSB 16 = imaginary data, 
 * LSB 16 = real data. 
 * 
 * @param pMoCASNRData (IN/OUT) - Pointer to MoCA_CIR_DATA structure. 
 *                      pData field must be allocated by caller (256*4 bytes)
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_REQUEST_DENIED - unable to open specified MoCA interface
 *         CMSRET_INTERNAL_ERROR - unable to get data
 */
CmsRet MoCACtl_GetCIRData(
   PMoCA_CIR_DATA pMoCACIRData);
CmsRet MoCACtl2_GetCIRData(
   void * ctx,
   PMoCA_CIR_DATA pMoCACIRData);


/**Initiates FMR request
 *
 * This function is called to initiate an FMR request with the MOCA driver.
 * The MoCA core will trigger an FMR trap with the FMR information. This
 * trap must be processed by the calling application. To process the trap
 * the functions from the mocalib library should be used: moca_event_loop() 
 * and moca_register_fmr_response_cb(). See mocactl.c for an example.
 * 
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param params (IN)  - A pointer to MoCA_FMR_PARAMS. Set the address
 *                       field to the mac address of a single node or 
 *                       to broadcast address FF:FF:FF:FF:FF:FF.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_INVALID_ARGUMENTS - supplied MAC address doesn't match a 
 *                                    node on the MoCA network
 *         CMSRET_INTERNAL_ERROR - unable to perform FMR request
 */
CmsRet MoCACtl2_Fmr(
   void * ctx,
   PMoCA_FMR_PARAMS params);


/**Creates a PQoS Flow
 *
 * This function is called to create a PQoS flow with the MOCA driver.
 * The MoCA core will trigger a trap with the PQoS create results. This
 * trap must be processed by the calling application to know if the create
 * succeeded or not. To process the trap the functions from the mocalib 
 * library should be used: moca_event_loop() and 
 * moca_register_pqos_create_response_cb(). See mocactl.c for an example.
 * 
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param params (IN/OUT) - A pointer to MoCA_CREATE_PQOS_PARAMS. The 
 *                          flow_id field will be populated by this 
 *                          function if filled with zeroes on input. 
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_INVALID_ARGUMENTS - supplied ingress MAC address doesn't
 *                                    match a node on the MoCA network
 *         CMSRET_INTERNAL_ERROR - unable to perform PQoS create request 
 */
CmsRet MoCACtl2_CreatePQoSFlow(
   void * ctx,
   PMoCA_CREATE_PQOS_PARAMS params
);


/**Updates a PQoS Flow
 *
 * This function is called to update a PQoS flow with the MOCA driver.
 * The MoCA core will trigger a trap with the PQoS update results. This
 * trap must be processed by the calling application to know if the update
 * succeeded or not. To process the trap the functions from the mocalib 
 * library should be used: moca_event_loop() and 
 * moca_register_pqos_update_response_cb(). See mocactl.c for an example.
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param params (IN)  - A pointer to MoCA_UPDATE_PQOS_PARAMS.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_INTERNAL_ERROR - unable to perform PQoS update request 
 */
CmsRet MoCACtl2_UpdatePQoSFlow(
   void * ctx,
   PMoCA_UPDATE_PQOS_PARAMS params
);

/**Deletes a PQoS Flow
 *
 * This function is called to delete a PQoS flow with the MOCA driver.
 * The MoCA core will trigger a trap with the PQoS delete results. This
 * trap must be processed by the calling application to know if the delete
 * succeeded or not. To process the trap the functions from the mocalib 
 * library should be used: moca_event_loop() and 
 * moca_register_pqos_delete_response_cb(). See mocactl.c for an example. 
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param params (IN)  - A pointer to MoCA_DELETE_PQOS_PARAMS.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_INTERNAL_ERROR - unable to perform PQoS delete request 
 */
CmsRet MoCACtl2_DeletePQoSFlow(
   void * ctx,
   PMoCA_DELETE_PQOS_PARAMS params
);


/**Queries a PQoS Flow
 *
 * This function is called to query an existing PQoS flow via the MOCA driver.
 * The MoCA core will trigger a trap with the PQoS query results. This
 * trap must be processed by the calling application. To process the trap 
 * the functions from the mocalib library should be used: moca_event_loop() and 
 * moca_register_pqos_query_response_cb(). See mocactl.c for an example. 
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param params (IN)  - A pointer to MoCA_QUERY_PQOS_PARAMS.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_INTERNAL_ERROR - unable to perform PQoS query request 
 */
CmsRet MoCACtl2_QueryPQoSFlow(
   void * ctx,
   PMoCA_QUERY_PQOS_PARAMS params
);


/**List PQoS Flow(s)
 *
 * This function is called to trigger a PQoS list command with the 
 * MOCA driver. The MoCA core will trigger a trap with the list results. 
 * This trap must be processed by the calling application. The application
 * must specify the MAC address of the node for which ingress flow 
 * information is desired. To process the trap the functions from the 
 * mocalib library should be used: moca_event_loop() and 
 * moca_register_pqos_list_response_cb(). See mocactl.c for an example. 
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param params (IN)  - A pointer to MoCA_LIST_PQOS_PARAMS.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_INVALID_ARGUMENTS - supplied MAC address doesn't
 *                                    match a node on the MoCA network
 *         CMSRET_INTERNAL_ERROR - unable to perform PQoS create request 
 */
CmsRet MoCACtl2_ListPQoSFlow(
   void * ctx,
   PMoCA_LIST_PQOS_PARAMS params
);


/**Get PQoS Status
 *
 * This function is called to trigger a PQoS status command with the 
 * MOCA driver. The MoCA core will trigger a trap with the status results. 
 * This trap must be processed by the calling application. To process the 
 * trap the functions from the mocalib library should be used: moca_event_loop() 
 * and moca_register_pqos_create_response_cb(). See mocactl.c for an example. 
 * The trap type is a create response trap in this case. The 
 * moca_pqos_create_response fields of interest are:
 *
 * totalstps = Summed up STPS 
 * totaltxps = Summed up TXPS 
 * flowstps  = QOS STPS       
 * flowtxps  = QOS TXPS       
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_INTERNAL_ERROR - unable to perform MR request 
 */
CmsRet MoCACtl2_GetPQoSStatus(
   void * ctx
);


/**Initiates a MR request
 *
 * This function is called to initiate a MoCA reset request.
 * The MoCA core will trigger an MR trap with the MR information.
 * This trap must be processed by the calling application to know if the 
 * MR operation succeeded or not. To process the trap the functions from the 
 * mocalib library should be used: moca_event_loop() and 
 * moca_register_mr_response_cb(). See mocactl.c for an example.
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param params (IN)  - A pointer to MoCA_MR_REQUEST_PARAMS.
 *
 * @return CmsRet enum.
 *         CMSRET_SUCCESS - success
 *         CMSRET_INTERNAL_ERROR - unable to perform MR request 
 */
CmsRet MoCACtl2_Mr(
   void * ctx,
   PMoCA_MR_PARAMS params);


/**Assigns a node to a channel
 *
 * This function is called to initiate an assign channel request.
 * The MoCA core will trigger an assign channel trap with the results information.
 * This trap must be processed by the calling application to know if the 
 * channel assign operation succeeded or not. To process the trap the 
 * functions from the mocalib library should be used: moca_event_loop() and 
 * moca_register_mr_response_cb(). See mocactl.c for an example. 
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param freq (IN)  - frequency to assign
 *         CMSRET_SUCCESS - success
 *         CMSRET_INTERNAL_ERROR - unable to perform assign channel request
 *
 * @return CmsRet enum.
 */
CmsRet MoCACtl2_ChSel(
   void * ctx,
   unsigned int freq);


/**Call MoCA core function
 *
 * This function is only for advanced users with intimate knowledge of 
 * the MoCA firmware.
 *
 * @param ctx (IN) - A pointer to MoCA context from MoCACtl_Open.
 * @param fnCallParams (IN) - A pointer to MoCA_FUNC_CALL_PARAMS structure
 * 
 * @return CmsRet enum.
 */
CmsRet MoCACtl_FunctionCall(
   PMoCA_FUNC_CALL_PARAMS fnCallParams );
CmsRet MoCACtl2_FunctionCall(
   void * ctx,
   PMoCA_FUNC_CALL_PARAMS fnCallParams );



CmsRet MoCACtl2_SetTPCAP(
   void * ctx,
   PMoCA_TPCAP_PARAMS tpcapParams );


CmsRet MoCACtl_RegisterCallback(
   MoCA_CALLBACK_EVENT event, 
   void (*callback)(void *userarg, MoCA_CALLBACK_DATA data), 
   void *userarg);
CmsRet MoCACtl2_RegisterCallback(
   void * ctx,
   MoCA_CALLBACK_EVENT event, 
   void (*callback)(void *userarg, MoCA_CALLBACK_DATA data), 
   void *userarg);

CmsRet MoCACtl_DispatchCallback(
   MoCA_CALLBACK_EVENT event, 
   MoCA_CALLBACK_DATA * pData);
CmsRet MoCACtl2_DispatchCallback(
   void * ctx,
   MoCA_CALLBACK_EVENT event, 
   MoCA_CALLBACK_DATA * pData);

CmsRet MoCACtl2_EventLoop(
   void * ctx);
CmsRet MoCACtl_EventLoop(void);

CmsRet MoCACtl2_CancelEventLoop(
   void * ctx);
CmsRet MoCACtl_CancelEventLoop(void);

CmsRet MoCACtl2_RestoreDefaults(void * ctx);


/**Write Init Parms as a string
 *
 * This function converts the MoCA_INITIALIZATION_PARMS structure into
 * a parameter string that can be used as input to the mocactl application.
 *
 * @param cli (IN/OUT) - A pointer to a char array into which the string 
 *                       will be written. The user must allocate this string.
 *                       The recommended size is 4096 bytes.
 * @param buf (IN) - A pointer to MoCA_INITIALIZATION_PARMS structure
 * 
 * @return char * a pointer to the end of the string written to 'cli'.
 */
char * MoCACtl2_WriteInitParms(char *cli, const void *buf);


/**Write Config Parms as a string
 *
 * This function converts the MoCA_CONFIG_PARAMS structure into
 * a parameter string that can be used as input to the mocactl application.
 *
 * @param cli (IN/OUT) - A pointer to a char array into which the string 
 *                       will be written. The user must allocate this string.
 *                       The recommended size is 4096 bytes.
 * @param buf (IN) - A pointer to MoCA_CONFIG_PARAMS structure
 * 
 * @return char * a pointer to the end of the string written to 'cli'.
 */
char * MoCACtl2_WriteCfgParms(char *cli, const void *buf);


/**Write Trace Parms as a string
 *
 * This function converts the MoCA_TRACE_PARAMS structure into
 * a parameter string that can be used as input to the mocactl application.
 *
 * @param cli (IN/OUT) - A pointer to a char array into which the string 
 *                       will be written. The user must allocate this string.
 *                       The recommended size is 4096 bytes.
 * @param buf (IN) - A pointer to MoCA_TRACE_PARAMS structure
 * 
 * @return char * a pointer to the end of the string written to 'cli'.
 */
char * MoCACtl2_WriteTraceParms(char *cli, MoCA_TRACE_PARAMS *buf);


SINT32 MoCACtl2_GetPersistent(void * ctx, const char *key, void *buf, UINT32 bufLen);

CmsRet MoCACtl2_SetPersistent(void * ctx, const char *key, const void *buf, UINT32 bufLen);

#endif /* __DEVCTL_MOCA_H__ */
