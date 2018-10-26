/******************************************************************************
 *
 *  Copyright (c) 2009  Broadcom Corporation
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
/***************************************************************************
 *
 *     Copyright (c) 2008-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 *  Description: Public interfaces for libmoca
 *
 ************************************************************/

#ifndef _MOCALIB_H_
#define _MOCALIB_H_

#include <stdint.h>

#if defined(WIN32)
#define inline __inline
#define __attribute__(x)
#endif


#if defined(STANDALONE)
#define GCAP_GEN FLASH
#else
#define GCAP_GEN 
#endif

#if defined(STANDALONE)
#include "standalone.h"

#define MOCALIB_CLI_HELP              FLASH
#define MOCALIB_CLI_SET               FLASH
#define MOCALIB_CLI_GET               FLASH
#define MOCALIB_CLI_DO                FLASH
#define MOCALIB_CLI_PRINT             FLASH
#define MOCALIB_CLI                   FLASH
#define MOCALIB_GEN_SET_FUNCTION      FLASH
#define MOCALIB_GEN_GET_FUNCTION      FLASH
#define MOCALIB_GEN_RANGE_FUNCTION    FLASH
#define MOCALIB_GEN_SWAP_FUNCTION     FLASH
#define MOCALIB_GEN_REGISTER_FUNCTION FLASH
#define MOCALIB_GEN_NVRAM_FUNCTION    FLASH
#define MOCALIB_GEN_MISC_FUNCTION     FLASH
#define MOCALIB_GEN_DO_FUNCTION       FLASH
#define MOCAD_S                       FLASH // slow mocad
//#define MOCA_DATA2                    DATA2
#define MOCA_DATA2                   
#else
#define MOCALIB_CLI_HELP
#define MOCALIB_CLI_SET
#define MOCALIB_CLI_GET
#define MOCALIB_CLI_DO
#define MOCALIB_CLI_PRINT
#define MOCALIB_CLI
#define MOCALIB_GEN_SET_FUNCTION
#define MOCALIB_GEN_GET_FUNCTION
#define MOCALIB_GEN_RANGE_FUNCTION
#define MOCALIB_GEN_SWAP_FUNCTION
#define MOCALIB_GEN_REGISTER_FUNCTION
#define MOCALIB_GEN_NVRAM_FUNCTION
#define MOCALIB_GEN_MISC_FUNCTION
#define MOCALIB_GEN_DO_FUNCTION
#define MOCAD_S
#define MOCA_DATA2 
#endif

#ifdef __cplusplus
extern "C" {
#endif

void *moca_open(char *ifname);
void moca_close(void *vctx);
int moca_init_evt(void *vctx);
int moca_event_loop(void *vctx);
int moca_event_wait_timeout(void *vctx, uint32_t timeout_sec);
void moca_cancel_event_loop(void *vctx);
int moca_start_event_loop(void * ctx);
int moca_wait_for_event(void * vctx, uint32_t timeout_s, uint32_t * test);
const char *moca_l2_error_name(uint32_t l2_error);

const char *moca_ie_name(uint32_t ie_type);
void moca_get_version(uint32_t *moca, uint32_t *major, uint32_t *minor, uint32_t *patch);


unsigned long moca_phy_rate(unsigned long nbas, unsigned long cp, unsigned long turbo, unsigned long moca_version);
uint16_t moca_password_hash (char * password);
const char * moca_decision_string(uint32_t value);
uint32_t moca_count_bits(uint32_t val);
int moca_print_help( void);
int moca_print_pvt_help( void);
int moca_set_persistent(void *ctx);

// A definition to let users know the moca_event_loop_timeout() function exists
#define MOCA_EVENT_WAIT_TIMEOUT_FN (1) 

/* Timeout for all mocalib requests */
#define MoCALIB_TIMEOUT          10

/* MOCA API Error codes */
#define MOCA_API_SUCCESS         ( 0)
#define MOCA_API_ERROR           (-1)
#define MOCA_API_TIMEOUT         (-2)
#define MOCA_API_CANT_CONNECT    (-3)
#define MOCA_API_SEND_FAILED     (-4)
#define MOCA_API_READ_FAILED     (-5)
#define MOCA_API_SHORT_READ      (-6)
#define MOCA_API_LONG_READ       (-7)
#define MOCA_API_INVALID_ACK     (-8)
#define MOCA_API_INVALID_SIZE    (-9)
#define MOCA_API_INVALID_TYPE    (-10)
#define MOCA_API_INVALID_THREAD  (-11)

#define MOCA_LINK_DOWN  0
#define MOCA_LINK_UP    1

#define MOCA_TX_BC		0x10
#define MOCA_TX_MAP		0x11

#define MOCA_MAX_NODES                  16
#define MOCA_MAX_NODES_1_0              8
#define MOCA_MAX_PQOS_LIST_FLOW_IDS     32
#define MOCA_MAX_PQOS_ENTRIES           16
#define MOCA_MAX_EGR_MC_FILTERS         32
#define MOCA_MAX_ECL_MCFILTERS          48

#define MOCA_MAX_SUB_CARRIERS_1_1       256
#define MOCA_MAX_SUB_CARRIERS           512

#define MOCA_L2_SUCCESS                 0x0
#define MOCA_L2_TRANSACTION_FAILED      0x1
#define MOCA_L2_L2ME_NO_SUPPORT         0x2
#define MOCA_L2_PQOS_INGR_NOT_FOUND     0x4
#define MOCA_L2_PQOS_EGR_NOT_FOUND      0x8
#define MOCA_L2_TRANSACTION_CANT_START  0x10

#define MOCA_PQOS_DECISION_SUCCESS                      1
#define MOCA_PQOS_DECISION_FLOW_EXISTS                  3
#define MOCA_PQOS_DECISION_INSUF_INGR_BW                4
#define MOCA_PQOS_DECISION_INSUF_EGR_BW                 5
#define MOCA_PQOS_DECISION_TOO_MANY_FLOWS               6 
#define MOCA_PQOS_DECISION_INVALID_TSPEC                8
#define MOCA_PQOS_DECISION_INVALID_DA                   9
#define MOCA_PQOS_DECISION_LEASE_EXPIRED                10
#define MOCA_PQOS_DECISION_INVALID_BURST_SIZE           11
#define MOCA_PQOS_DECISION_RETRY_NOTOK                  12
#define MOCA_PQOS_DECISION_INGRESS_TOO_MANY_MCAST_FLOWS 13
#define MOCA_PQOS_DECISION_INGRESS_DUPLICATE_FLOW       14
#define MOCA_PQOS_DECISION_INGRESS_M1_NOTOK             15
#define MOCA_PQOS_DECISION_FLOW_NOT_FOUND               16
#define MOCA_PQOS_DECISION_INSUF_AGGR_STPS              17
#define MOCA_PQOS_DECISION_INSUF_AGGR_TXPS              18
#define MOCA_PQOS_DECISION_UNUSABLE_LINK                19


#define MOCA_PQOS_INGR_CLASS_RULE_DA_VLAN_4_5   0
#define MOCA_PQOS_INGR_CLASS_RULE_DA_ONLY       4
#define MOCA_PQOS_INGR_CLASS_RULE_DA_DSCP       5
#define MOCA_PQOS_INGR_CLASS_RULE_DA_VLAN       6
#define MOCA_PQOS_INGR_CLASS_RULE_DA_VLAN_DSCP  7


#define MOCA_BANDWIDTH_50MHZ                    0
#define MOCA_BANDWIDTH_100MHZ                   1


#define MOCA_CHIP_11      0x1100   // 60/65nm chips
#define MOCA_CHIP_11_LITE 0x1101   // 7408 with smaller buffers
#define MOCA_CHIP_11_PLUS 0x1102   // 40nm chips
#define MOCA_CHIP_20      0x2000   // for compatibility
#define MOCA_CHIP_20_1    0x2001
#define MOCA_CHIP_20_2    0x2002
#define MOCA_CHIP_20_3    0x2003
#define MOCA_CHIP_20_4    0x2004
#define MOCA_CHIP_20_5    0x2005

#define MOCA_EXT_STATUS_PROFILE_RX_UCAST    0
#define MOCA_EXT_STATUS_PROFILE_RX_BCAST    1
#define MOCA_EXT_STATUS_PROFILE_RX_MAP      2
#define MOCA_EXT_STATUS_PROFILE_TX_UCAST    3
#define MOCA_EXT_STATUS_PROFILE_TX_BCAST    4
#define MOCA_EXT_STATUS_PROFILE_TX_MAP      5
#define MOCA_EXT_STATUS_PROFILE_RX_UC_VLPER 6
#define MOCA_EXT_STATUS_PROFILE_RX_UC_NPER  7
#define MOCA_EXT_STATUS_PROFILE_RX_BC_VLPER 8
#define MOCA_EXT_STATUS_PROFILE_RX_BC_NPER  9
#define MOCA_EXT_STATUS_PROFILE_RX_MAP_20   10
#define MOCA_EXT_STATUS_PROFILE_RX_OFDMA    11
#define MOCA_EXT_STATUS_PROFILE_TX_UC_VLPER 12
#define MOCA_EXT_STATUS_PROFILE_TX_UC_NPER  13
#define MOCA_EXT_STATUS_PROFILE_TX_BC_VLPER 14
#define MOCA_EXT_STATUS_PROFILE_TX_BC_NPER  15
#define MOCA_EXT_STATUS_PROFILE_TX_MAP_20   16
#define MOCA_EXT_STATUS_PROFILE_TX_OFDMA    17
#define MOCA_EXT_STATUS_PROFILE_RX_UC_NPER_SEC_CH    18
#define MOCA_EXT_STATUS_PROFILE_RX_UC_VLPER_SEC_CH   19
#define MOCA_EXT_STATUS_PROFILE_TX_UC_NPER_SEC_CH    20
#define MOCA_EXT_STATUS_PROFILE_TX_UC_VLPER_SEC_CH   21

#define MOCA_PROFILE_IS_ON_PRIMARY_CH(profile)      ((profile) <= MOCA_EXT_STATUS_PROFILE_TX_OFDMA)
#define MOCA_PROFILE_IS_ON_SECONDARY_CH(profile)    ((profile) >  MOCA_EXT_STATUS_PROFILE_TX_OFDMA)


#define MoCA_VERSION_1_0                 0x10
#define MoCA_VERSION_1_1                 0x11
#define MoCA_VERSION_2_0                 0x20

#define MOCA_RF_BAND_D_LOW               0
#define MOCA_RF_BAND_D_HIGH              1
#define MOCA_RF_BAND_EX_D                2
#define MOCA_RF_BAND_E                   3
#define MOCA_RF_BAND_F                   4
#define MOCA_RF_BAND_C4                  5
#define MOCA_RF_BAND_H                   6
#define MOCA_RF_BAND_GENERIC             7

#define MOCA_SINGLE_CH_OP_NETWORK_SEARCH 0
#define MOCA_SINGLE_CH_OP_SINGLE         1

#define MOCA_PER_MODE_NPER               0
#define MOCA_PER_MODE_VLPER              1

#define MOCA_KEY_CHANGED_TEK             0
#define MOCA_KEY_CHANGED_PMK             1
#define MOCA_KEY_CHANGED_ATEK            2
#define MOCA_KEY_CHANGED_APMK            3


#define MOCA_KEY_CHANGED_ATEK_EVEN_ODD_BIT   (1 << 0)
#define MOCA_KEY_CHANGED_APMK_EVEN_ODD_BIT   (1 << 1)

#define MOCA_PROBE_REQUEST_TYPE_LOOPBACK 0
#define MOCA_PROBE_REQUEST_TYPE_SILENT   1

#define MOCA_UPDATE_FLASH_INIT           0

#define MOCA_DISPLAY_MAC(mac)  (unsigned int)((mac).addr[0]), \
                               (unsigned int)((mac).addr[1]), \
                               (unsigned int)((mac).addr[2]), \
                               (unsigned int)((mac).addr[3]), \
                               (unsigned int)((mac).addr[4]), \
                               (unsigned int)((mac).addr[5])

#define MOCA_DONT_START_MOCA_OFF         0  // MoCA core will start normally
#define MOCA_DONT_START_MOCA_ON          1  // MoCA core will not be started
#define MOCA_DONT_START_MOCA_WAIT        2  // MoCA core will only be started after receiving the 'start' operation

typedef struct 
{
   uint8_t addr[6];
} macaddr_t;

static inline void moca_mac_to_u32(uint32_t *hi, uint32_t *lo,
   const uint8_t *mac)
{
   *hi = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | (mac[3] << 0);
   *lo = (mac[4] << 24) | (mac[5] << 16);
}

static inline void moca_u32_to_mac(uint8_t *mac, uint32_t hi, uint32_t lo)
{
   mac[0] = (hi >> 24) & 0xff;
   mac[1] = (hi >> 16) & 0xff;
   mac[2] = (hi >>  8) & 0xff;
   mac[3] = (hi >>  0) & 0xff;
   mac[4] = (lo >> 24) & 0xff;
   mac[5] = (lo >> 16) & 0xff;
}

void mocacli_to_bits(char *str, uint32_t * out, uint32_t size);

/* GENERATED API BELOW THIS LINE - DO NOT EDIT */

#define MOCA_PREFERRED_NC_DEF                      0
#define MOCA_PREFERRED_NC_BAND_E_DEF               1
#define MOCA_PREFERRED_NC_BAND_F_DEF               1

#define MOCA_SINGLE_CHANNEL_OPERATION_DEF          0
#define MOCA_SINGLE_CHANNEL_OPERATION_BAND_GENERIC_DEF  1

#define MOCA_CONTINUOUS_POWER_TX_MODE_DEF          0

#define MOCA_CONTINUOUS_RX_MODE_ATTN_DEF           0
#define MOCA_CONTINUOUS_RX_MODE_ATTN_MIN           -1
#define MOCA_CONTINUOUS_RX_MODE_ATTN_MAX           63

#define MOCA_LOF_DEF                               0
#define MOCA_LOF_BAND_EX_D_DEF                     1150
#define MOCA_LOF_BAND_D_LOW_DEF                    1150
#define MOCA_LOF_BAND_GENERIC_DEF                  1150
#define MOCA_LOF_BAND_D_HIGH_DEF                   1400
#define MOCA_LOF_BAND_E_DEF                        575
#define MOCA_LOF_BAND_F_DEF                        800
#define MOCA_LOF_BAND_C4_DEF                       1000
#define MOCA_LOF_BAND_H_DEF                        1000

#define MOCA_NO_IFG6_DEF                           0
#define MOCA_NO_IFG6_MIN                           0
#define MOCA_NO_IFG6_MAX                           1

#define MOCA_BONDING_DEF                           0
#define MOCA_BONDING_BAND_D_LOW_DEF                0
#define MOCA_BONDING_3390B0_DEF                    1
#define MOCA_BONDING_BONDING_SUPPORTED_DEF         1
#define MOCA_BONDING_MIN                           0
#define MOCA_BONDING_3390B0_MIN                    1
#define MOCA_BONDING_MAX                           0
#define MOCA_BONDING_BAND_D_LOW_MAX                0
#define MOCA_BONDING_BONDING_SUPPORTED_MAX         1

#define MOCA_LISTENING_FREQ_MASK_DEF               0xFFFFFFFF

#define MOCA_LISTENING_DURATION_DEF                1050
#define MOCA_LISTENING_DURATION_MIN                100

#define MOCA_LIMIT_TRAFFIC_DEF                     0
#define MOCA_LIMIT_TRAFFIC_MIN                     0
#define MOCA_LIMIT_TRAFFIC_MAX                     2

#define MOCA_REMOTE_MAN_DEF                        0
#define MOCA_REMOTE_MAN_STANDALONE_6802B0_DEF      1
#define MOCA_REMOTE_MAN_STANDALONE_6802C0_DEF      2
#define MOCA_REMOTE_MAN_MIN                        0
#define MOCA_REMOTE_MAN_MAX                        2

#define MOCA_C4_MOCA20_EN_DEF                      0
#define MOCA_C4_MOCA20_EN_MIN                      0
#define MOCA_C4_MOCA20_EN_MAX                      1

#define MOCA_POWER_SAVE_MECHANISM_DIS_DEF          0
#define MOCA_POWER_SAVE_MECHANISM_DIS_MIN          0
#define MOCA_POWER_SAVE_MECHANISM_DIS_MAX          1

#define MOCA_PSM_CONFIG_DEF                        7
#define MOCA_PSM_CONFIG_7425_DEF                   6
#define MOCA_PSM_CONFIG_28NM_DEF                   3
#define MOCA_PSM_CONFIG_MIN                        0
#define MOCA_PSM_CONFIG_7425_MIN                   2
#define MOCA_PSM_CONFIG_MAX                        7
#define MOCA_PSM_CONFIG_28NM_MAX                   3

#define MOCA_USE_EXT_DATA_MEM_DEF                  0
#define MOCA_USE_EXT_DATA_MEM_MIN                  0
#define MOCA_USE_EXT_DATA_MEM_MAX                  0
#define MOCA_USE_EXT_DATA_MEM_BONDING_SUPPORTED_MAX  1
#define MOCA_USE_EXT_DATA_MEM_6803C0_MAX           1

#define MOCA_AIF_MODE_DEF                          0x9
#define MOCA_AIF_MODE_MIN                          0
#define MOCA_AIF_MODE_MAX                          0x7FF

#define MOCA_PROP_BONDING_COMPATIBILITY_MODE_DEF   0
#define MOCA_PROP_BONDING_COMPATIBILITY_MODE_BONDING_SUPPORTED_DEF  0
#define MOCA_PROP_BONDING_COMPATIBILITY_MODE_MIN   0
#define MOCA_PROP_BONDING_COMPATIBILITY_MODE_MAX   0
#define MOCA_PROP_BONDING_COMPATIBILITY_MODE_BONDING_SUPPORTED_MAX  2

#define MOCA_RDEG_3450_DEF                         0xE
#define MOCA_RDEG_3450_MIN                         0
#define MOCA_RDEG_3450_MAX                         0xF

#define MOCA_PHY_CLOCK_DEF                         0
#define MOCA_PHY_CLOCK_3390B0_DEF                  0
#define MOCA_PHY_CLOCK_MIN                         0
#define MOCA_PHY_CLOCK_MAX                         0
#define MOCA_PHY_CLOCK_3390B0_MAX                  1000

#define MOCA_MAX_TX_POWER_TUNE_OFFSET_DEF          0
#define MOCA_MAX_TX_POWER_TUNE_OFFSET_MIN          0
#define MOCA_MAX_TX_POWER_TUNE_OFFSET_MAX          56

#define MOCA_MAX_TX_POWER_TUNE_SEC_CH_OFFSET_DEF   0
#define MOCA_MAX_TX_POWER_TUNE_SEC_CH_OFFSET_MIN   0
#define MOCA_MAX_TX_POWER_TUNE_SEC_CH_OFFSET_MAX   56

#define MOCA_RX_POWER_TUNE_OFFSET_DEF              0
#define MOCA_RX_POWER_TUNE_OFFSET_MIN              -120
#define MOCA_RX_POWER_TUNE_OFFSET_MAX              120

#define MOCA_IMPEDANCE_MODE_BONDING_DEF            0
#define MOCA_IMPEDANCE_MODE_BONDING_BONDING_SUPPORTED_DEF  0x3C3
#define MOCA_IMPEDANCE_MODE_BONDING_MIN            0
#define MOCA_IMPEDANCE_MODE_BONDING_MAX            0
#define MOCA_IMPEDANCE_MODE_BONDING_BONDING_SUPPORTED_MAX  0xFFF

#define MOCA_REWORK_6802_DEF                       0
#define MOCA_REWORK_6802_MIN                       0
#define MOCA_REWORK_6802_BONDING_SUPPORTED_MAX     1
#define MOCA_REWORK_6802_MAX                       0

#define MOCA_DRV_INFO_DEF                          0

#define MOCA_EN_CAPABLE_DEF                        1

#define MOCA_LOF_UPDATE_DEF                        1
#define MOCA_LOF_UPDATE_MIN                        0
#define MOCA_LOF_UPDATE_MAX                        1

#define MOCA_PRIMARY_CH_OFFSET_DEF                 1

#define MOCA_ASSERTTEXT_DEF                        0

#define MOCA_WDOG_ENABLE_DEF                       1

#define MOCA_MR_SEQ_NUM_DEF                        0
#define MOCA_MR_SEQ_NUM_MIN                        0
#define MOCA_MR_SEQ_NUM_MAX                        0xFFFF

#define MOCA_SECONDARY_CH_OFFSET_DEF               1
#define MOCA_SECONDARY_CH_OFFSET_BAND_GENERIC_DEF  125

#define MOCA_COF_DEF                               0

#define MOCA_AMP_TYPE_DEF                          1

#define MOCA_TPC_EN_DEF                            1
#define MOCA_TPC_EN_BAND_E_DEF                     0
#define MOCA_TPC_EN_BAND_F_DEF                     0
#define MOCA_TPC_EN_BAND_H_DEF                     0

#define MOCA_MAX_TX_POWER_DEF                      3
#define MOCA_MAX_TX_POWER_MIN                      -31
#define MOCA_MAX_TX_POWER_MAX                      3

#define MOCA_BEACON_PWR_REDUCTION_DEF              0
#define MOCA_BEACON_PWR_REDUCTION_MIN              0
#define MOCA_BEACON_PWR_REDUCTION_MAX              5
#define MOCA_BEACON_PWR_REDUCTION_BAND_E_MAX       0
#define MOCA_BEACON_PWR_REDUCTION_BAND_F_MAX       0

#define MOCA_BEACON_PWR_REDUCTION_EN_6816_DEF      0
#define MOCA_BEACON_PWR_REDUCTION_EN_BAND_E_DEF    0
#define MOCA_BEACON_PWR_REDUCTION_EN_BAND_F_DEF    0
#define MOCA_BEACON_PWR_REDUCTION_EN_7xxx_DEF      1
#define MOCA_BEACON_PWR_REDUCTION_EN_MIN           0
#define MOCA_BEACON_PWR_REDUCTION_EN_MAX           1
#define MOCA_BEACON_PWR_REDUCTION_EN_BAND_E_MAX    0
#define MOCA_BEACON_PWR_REDUCTION_EN_BAND_F_MAX    0

#define MOCA_BO_MODE_DEF                           0

#define MOCA_QAM256_CAPABILITY_DEF                 1

#define MOCA_OTF_EN_DEF                            0

#define MOCA_STAR_TOPOLOGY_EN_DEF                  0
#define MOCA_STAR_TOPOLOGY_EN_MIN                  0
#define MOCA_STAR_TOPOLOGY_EN_MAX                  1

#define MOCA_OFDMA_EN_DEF                          1
#define MOCA_OFDMA_EN_MIN                          0
#define MOCA_OFDMA_EN_MAX                          1

#define MOCA_MIN_BW_ALARM_THRESHOLD_DEF            100
#define MOCA_MIN_BW_ALARM_THRESHOLD_MIN            50
#define MOCA_MIN_BW_ALARM_THRESHOLD_MAX            3200

#define MOCA_EN_MAX_RATE_IN_MAX_BO_DEF             0

#define MOCA_TARGET_PHY_RATE_QAM128_DEF            245
#define MOCA_TARGET_PHY_RATE_QAM128_MAX            500

#define MOCA_TARGET_PHY_RATE_QAM256_DEF            275
#define MOCA_TARGET_PHY_RATE_QAM256_MAX            500

#define MOCA_SAPM_EN_DEF                           0

#define MOCA_ARPL_TH_50_DEF                        -50
#define MOCA_ARPL_TH_50_MIN                        -65
#define MOCA_ARPL_TH_50_MAX                        0

#define MOCA_RLAPM_EN_DEF                          0
#define MOCA_RLAPM_EN_BAND_E_DEF                   1
#define MOCA_RLAPM_EN_BAND_F_DEF                   1

#define MOCA_FREQ_SHIFT_DEF                        0

#define MOCA_MAX_PHY_RATE_DEF                      670
#define MOCA_MAX_PHY_RATE_7425_DEF                 630

#define MOCA_BANDWIDTH_DEF                         0
#define MOCA_BANDWIDTH_MIN                         0
#define MOCA_BANDWIDTH_MAX                         1

#define MOCA_ARPL_TH_100_DEF                       -50
#define MOCA_ARPL_TH_100_MIN                       -65
#define MOCA_ARPL_TH_100_MAX                       0

#define MOCA_ADC_MODE_DEF                          0

#define MOCA_MAX_PHY_RATE_TURBO_DEF                670
#define MOCA_MAX_PHY_RATE_TURBO_7425_DEF           680

#define MOCA_MAX_PHY_RATE_50M_DEF                  300

#define MOCA_MAX_CONSTELLATION_ALL_DEF             10
#define MOCA_MAX_CONSTELLATION_ALL_MIN             1
#define MOCA_MAX_CONSTELLATION_ALL_MAX             10

#define MOCA_MAX_CONSTELLATION_NODE_ID_MIN         0
#define MOCA_MAX_CONSTELLATION_NODE_ID_MAX         15

#define MOCA_MAX_CONSTELLATION_P2P_LIMIT_50_DEF    10
#define MOCA_MAX_CONSTELLATION_P2P_LIMIT_50_MIN    1
#define MOCA_MAX_CONSTELLATION_P2P_LIMIT_50_MAX    10

#define MOCA_MAX_CONSTELLATION_GCD_LIMIT_50_DEF    10
#define MOCA_MAX_CONSTELLATION_GCD_LIMIT_50_MIN    1
#define MOCA_MAX_CONSTELLATION_GCD_LIMIT_50_MAX    10

#define MOCA_MAX_CONSTELLATION_P2P_LIMIT_100_DEF   10
#define MOCA_MAX_CONSTELLATION_P2P_LIMIT_100_MIN   1
#define MOCA_MAX_CONSTELLATION_P2P_LIMIT_100_MAX   10

#define MOCA_MAX_CONSTELLATION_GCD_LIMIT_100_DEF   10
#define MOCA_MAX_CONSTELLATION_GCD_LIMIT_100_MIN   1
#define MOCA_MAX_CONSTELLATION_GCD_LIMIT_100_MAX   10

#define MOCA_MAX_CONSTELLATION_MIN                 0
#define MOCA_MAX_CONSTELLATION_MAX                 15

#define MOCA_RLAPM_TABLE_50_RLAPMTABLE_DEF         0
#define MOCA_RLAPM_TABLE_50_RLAPMTABLE_MIN         0
#define MOCA_RLAPM_TABLE_50_RLAPMTABLE_MAX         60

#define MOCA_PHY_STATUS_MIN                        0
#define MOCA_PHY_STATUS_MAX                        35

#define MOCA_RLAPM_TABLE_100_RLAPMTABLE_DEF        0
#define MOCA_RLAPM_TABLE_100_RLAPMTABLE_MIN        0
#define MOCA_RLAPM_TABLE_100_RLAPMTABLE_MAX        60

#define MOCA_NV_CAL_ENABLE_DEF                     0
#define MOCA_NV_CAL_ENABLE_MIN                     0
#define MOCA_NV_CAL_ENABLE_MAX                     1

#define MOCA_RLAPM_CAP_50_DEF                      0

#define MOCA_SNR_MARGIN_RS_BASE_MARGIN_DEF         0
#define MOCA_SNR_MARGIN_RS_BASE_MARGIN_MIN         -768
#define MOCA_SNR_MARGIN_RS_BASE_MARGIN_MAX         6400

#define MOCA_SNR_MARGIN_RS_OFFSETS_DEF             0
#define MOCA_SNR_MARGIN_RS_OFFSETS_MIN             -768
#define MOCA_SNR_MARGIN_RS_OFFSETS_MAX             6400

#define MOCA_SNR_MARGIN_LDPC_BASE_MARGIN_DEF       0x0
#define MOCA_SNR_MARGIN_LDPC_BASE_MARGIN_MIN       -768
#define MOCA_SNR_MARGIN_LDPC_BASE_MARGIN_MAX       6400

#define MOCA_SNR_MARGIN_LDPC_OFFSETS_DEF           0
#define MOCA_SNR_MARGIN_LDPC_OFFSETS_MIN           -768
#define MOCA_SNR_MARGIN_LDPC_OFFSETS_MAX           6400

#define MOCA_SNR_MARGIN_LDPC_SEC_CH_BASE_MARGIN_DEF  0x100
#define MOCA_SNR_MARGIN_LDPC_SEC_CH_BASE_MARGIN_MIN  -768
#define MOCA_SNR_MARGIN_LDPC_SEC_CH_BASE_MARGIN_MAX  6400

#define MOCA_SNR_MARGIN_LDPC_SEC_CH_OFFSETS_DEF    0
#define MOCA_SNR_MARGIN_LDPC_SEC_CH_OFFSETS_MIN    -768
#define MOCA_SNR_MARGIN_LDPC_SEC_CH_OFFSETS_MAX    6400

#define MOCA_SNR_MARGIN_LDPC_PRE5_BASE_MARGIN_DEF  0
#define MOCA_SNR_MARGIN_LDPC_PRE5_BASE_MARGIN_MIN  -768
#define MOCA_SNR_MARGIN_LDPC_PRE5_BASE_MARGIN_MAX  6400

#define MOCA_SNR_MARGIN_LDPC_PRE5_OFFSETS_DEF      0
#define MOCA_SNR_MARGIN_LDPC_PRE5_OFFSETS_MIN      -768
#define MOCA_SNR_MARGIN_LDPC_PRE5_OFFSETS_MAX      6400

#define MOCA_SNR_MARGIN_OFDMA_BASE_MARGIN_DEF      0
#define MOCA_SNR_MARGIN_OFDMA_BASE_MARGIN_MIN      -768
#define MOCA_SNR_MARGIN_OFDMA_BASE_MARGIN_MAX      6400

#define MOCA_SNR_MARGIN_OFDMA_OFFSETS_DEF          0
#define MOCA_SNR_MARGIN_OFDMA_OFFSETS_MIN          -768
#define MOCA_SNR_MARGIN_OFDMA_OFFSETS_MAX          6400

#define MOCA_RLAPM_CAP_100_DEF                     0

#define MOCA_SAPM_TABLE_50_VAL_DEF                 0
#define MOCA_SAPM_TABLE_50_VAL_MIN                 0
#define MOCA_SAPM_TABLE_50_VAL_MAX                 120

#define MOCA_SAPM_TABLE_100_VAL_DEF                0
#define MOCA_SAPM_TABLE_100_VAL_MIN                0
#define MOCA_SAPM_TABLE_100_VAL_MAX                120

#define MOCA_SAPM_TABLE_SEC_VAL_DEF                0
#define MOCA_SAPM_TABLE_SEC_VAL_MIN                0
#define MOCA_SAPM_TABLE_SEC_VAL_MAX                120

#define MOCA_AMP_REG_VALUE_DEF                     0

#define MOCA_SNR_MARGIN_LDPC_PRI_CH_BASE_MARGIN_DEF  0x100
#define MOCA_SNR_MARGIN_LDPC_PRI_CH_BASE_MARGIN_MIN  -768
#define MOCA_SNR_MARGIN_LDPC_PRI_CH_BASE_MARGIN_MAX  6400

#define MOCA_SNR_MARGIN_LDPC_PRI_CH_OFFSETS_DEF    0
#define MOCA_SNR_MARGIN_LDPC_PRI_CH_OFFSETS_MIN    -768
#define MOCA_SNR_MARGIN_LDPC_PRI_CH_OFFSETS_MAX    6400

#define MOCA_SNR_MARGIN_PRE5_PRI_CH_BASE_MARGIN_DEF  0x100
#define MOCA_SNR_MARGIN_PRE5_PRI_CH_BASE_MARGIN_MIN  -768
#define MOCA_SNR_MARGIN_PRE5_PRI_CH_BASE_MARGIN_MAX  6400

#define MOCA_SNR_MARGIN_PRE5_PRI_CH_OFFSETS_DEF    0
#define MOCA_SNR_MARGIN_PRE5_PRI_CH_OFFSETS_MIN    -768
#define MOCA_SNR_MARGIN_PRE5_PRI_CH_OFFSETS_MAX    6400

#define MOCA_SNR_MARGIN_PRE5_SEC_CH_BASE_MARGIN_DEF  0x100
#define MOCA_SNR_MARGIN_PRE5_SEC_CH_BASE_MARGIN_MIN  -768
#define MOCA_SNR_MARGIN_PRE5_SEC_CH_BASE_MARGIN_MAX  6400

#define MOCA_SNR_MARGIN_PRE5_SEC_CH_OFFSETS_DEF    0
#define MOCA_SNR_MARGIN_PRE5_SEC_CH_OFFSETS_MIN    -768
#define MOCA_SNR_MARGIN_PRE5_SEC_CH_OFFSETS_MAX    6400

#define MOCA_MAX_FRAME_SIZE_DEF                    32768
#define MOCA_MAX_FRAME_SIZE_MIN                    2048
#define MOCA_MAX_FRAME_SIZE_MAX                    32768

#define MOCA_MIN_AGGR_WAITING_TIME_DEF             0

#define MOCA_SELECTIVE_RR_DEF                      3

#define MOCA_MAX_TRANSMIT_TIME_DEF                 400
#define MOCA_MAX_TRANSMIT_TIME_MIN                 300
#define MOCA_MAX_TRANSMIT_TIME_MAX                 1000

#define MOCA_MAX_PKT_AGGR_DEF                      20
#define MOCA_MAX_PKT_AGGR_MIN                      1
#define MOCA_MAX_PKT_AGGR_MAX                      20

#define MOCA_RTR_CONFIG_LOW_DEF                    0
#define MOCA_RTR_CONFIG_LOW_MIN                    0
#define MOCA_RTR_CONFIG_LOW_MAX                    3

#define MOCA_RTR_CONFIG_MED_DEF                    0
#define MOCA_RTR_CONFIG_MED_MIN                    0
#define MOCA_RTR_CONFIG_MED_MAX                    3

#define MOCA_RTR_CONFIG_HIGH_DEF                   0
#define MOCA_RTR_CONFIG_HIGH_MIN                   0
#define MOCA_RTR_CONFIG_HIGH_MAX                   3

#define MOCA_RTR_CONFIG_BG_DEF                     0
#define MOCA_RTR_CONFIG_BG_MIN                     0
#define MOCA_RTR_CONFIG_BG_MAX                     3

#define MOCA_TLP_MODE_DEF                          1
#define MOCA_TLP_MODE_MIN                          1
#define MOCA_TLP_MODE_MAX                          2

#define MOCA_MAX_PKT_AGGR_BONDING_DEF              27
#define MOCA_MAX_PKT_AGGR_BONDING_MIN              1
#define MOCA_MAX_PKT_AGGR_BONDING_MAX              30

#define MOCA_MULTICAST_MODE_DEF                    1

#define MOCA_EGR_MC_FILTER_EN_DEF                  0

#define MOCA_FC_MODE_FC_CAPABLE_CHIP_DEF           0
#define MOCA_FC_MODE_DEF                           1
#define MOCA_FC_MODE_MIN                           0
#define MOCA_FC_MODE_MAX                           1

#define MOCA_PQOS_MAX_PACKET_SIZE_DEF              1518
#define MOCA_PQOS_MAX_PACKET_SIZE_MIN              64

#define MOCA_PER_MODE_DEF                          0
#define MOCA_PER_MODE_BAND_E_DEF                   1
#define MOCA_PER_MODE_BAND_H_DEF                   1
#define MOCA_PER_MODE_MIN                          0
#define MOCA_PER_MODE_MAX                          1

#define MOCA_POLICING_EN_DEF                       0
#define MOCA_POLICING_EN_MIN                       0
#define MOCA_POLICING_EN_MAX                       1

#define MOCA_PQOS_EGRESS_NUMFLOWS_MIN              0
#define MOCA_PQOS_EGRESS_NUMFLOWS_MAX              12

#define MOCA_ORR_EN_DEF                            0
#define MOCA_ORR_EN_MIN                            0
#define MOCA_ORR_EN_MAX                            1

#define MOCA_BRCMTAG_ENABLE_DEF                    0
#define MOCA_BRCMTAG_ENABLE_MIN                    0
#define MOCA_BRCMTAG_ENABLE_MAX                    1

#define MOCA_UNKNOWN_RATELIMIT_EN_DEF              1
#define MOCA_UNKNOWN_RATELIMIT_EN_MIN              0
#define MOCA_UNKNOWN_RATELIMIT_EN_MAX              1

#define MOCA_EGR_MC_ADDR_FILTER_ENTRYID_MIN        0
#define MOCA_EGR_MC_ADDR_FILTER_ENTRYID_MAX        31

#define MOCA_EGR_MC_ADDR_FILTER_MIN                0
#define MOCA_EGR_MC_ADDR_FILTER_MAX                31

#define MOCA_EGR_MC_ADDR_FILTER_ENTRYID_MIN        0
#define MOCA_EGR_MC_ADDR_FILTER_ENTRYID_MAX        31

#define MOCA_UC_FWD_MOCA_DEST_NODE_ID_MIN          0
#define MOCA_UC_FWD_MOCA_DEST_NODE_ID_MAX          15

#define MOCA_MC_FWD_DEST_NODE_ID_MIN               0
#define MOCA_MC_FWD_DEST_NODE_ID_MAX               15

#define MOCA_MAC_AGING_UC_FWD_AGE_DEF              300

#define MOCA_MAC_AGING_MC_FWD_AGE_DEF              0xFFFF

#define MOCA_MAC_AGING_SRC_ADDR_AGE_DEF            300

#define MOCA_LOOPBACK_EN_DEF                       0

#define MOCA_MCFILTER_ENABLE_DEF                   0

#define MOCA_PAUSE_FC_EN_DEF                       0
#define MOCA_PAUSE_FC_EN_STANDALONE_DEF            1
#define MOCA_PAUSE_FC_EN_MIN                       0
#define MOCA_PAUSE_FC_EN_MAX                       1

#define MOCA_STAG_PRIORITY_ENABLE_DEF              0
#define MOCA_STAG_PRIORITY_ENABLE_MIN              0
#define MOCA_STAG_PRIORITY_ENABLE_MAX              1

#define MOCA_STAG_PRIORITY_TAG_MASK_DEF            0
#define MOCA_STAG_PRIORITY_TAG_MASK_MIN            0
#define MOCA_STAG_PRIORITY_TAG_MASK_MAX            0xFFFF

#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_0_DEF     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_0_MIN     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_0_MAX     4

#define MOCA_STAG_PRIORITY_TAG_PRIORITY_0_DEF      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_0_MIN      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_0_MAX      4

#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_1_DEF     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_1_MIN     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_1_MAX     4

#define MOCA_STAG_PRIORITY_TAG_PRIORITY_1_DEF      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_1_MIN      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_1_MAX      4

#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_2_DEF     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_2_MIN     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_2_MAX     4

#define MOCA_STAG_PRIORITY_TAG_PRIORITY_2_DEF      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_2_MIN      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_2_MAX      4

#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_3_DEF     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_3_MIN     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_3_MAX     4

#define MOCA_STAG_PRIORITY_TAG_PRIORITY_3_DEF      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_3_MIN      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_3_MAX      4

#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_4_DEF     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_4_MIN     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_4_MAX     4

#define MOCA_STAG_PRIORITY_TAG_PRIORITY_4_DEF      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_4_MIN      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_4_MAX      4

#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_5_DEF     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_5_MIN     0

#define MOCA_STAG_PRIORITY_TAG_PRIORITY_5_DEF      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_5_MIN      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_5_MAX      4

#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_6_DEF     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_6_MIN     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_6_MAX     4

#define MOCA_STAG_PRIORITY_TAG_PRIORITY_6_DEF      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_6_MIN      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_6_MAX      4

#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_7_DEF     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_7_MIN     0
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_7_MAX     4

#define MOCA_STAG_PRIORITY_TAG_PRIORITY_7_DEF      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_7_MIN      0
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_7_MAX      4

#define MOCA_STAG_REMOVAL_ENABLE_DEF               0
#define MOCA_STAG_REMOVAL_ENABLE_MIN               0
#define MOCA_STAG_REMOVAL_ENABLE_MAX               1

#define MOCA_STAG_REMOVAL_VALID_0_DEF              0
#define MOCA_STAG_REMOVAL_VALID_0_MIN              0
#define MOCA_STAG_REMOVAL_VALID_0_MAX              1

#define MOCA_STAG_REMOVAL_VALUE_0_DEF              0

#define MOCA_STAG_REMOVAL_MASK_0_DEF               0xffffffff

#define MOCA_STAG_REMOVAL_VALID_1_DEF              0
#define MOCA_STAG_REMOVAL_VALID_1_MIN              0
#define MOCA_STAG_REMOVAL_VALID_1_MAX              1

#define MOCA_STAG_REMOVAL_VALUE_1_DEF              0

#define MOCA_STAG_REMOVAL_MASK_1_DEF               0xffffffff

#define MOCA_STAG_REMOVAL_VALID_2_DEF              0
#define MOCA_STAG_REMOVAL_VALID_2_MIN              0
#define MOCA_STAG_REMOVAL_VALID_2_MAX              1

#define MOCA_STAG_REMOVAL_VALUE_2_DEF              0

#define MOCA_STAG_REMOVAL_MASK_2_DEF               0xffffffff

#define MOCA_STAG_REMOVAL_VALID_3_DEF              0
#define MOCA_STAG_REMOVAL_VALID_3_MIN              0
#define MOCA_STAG_REMOVAL_VALID_3_MAX              1

#define MOCA_STAG_REMOVAL_VALUE_3_DEF              0

#define MOCA_STAG_REMOVAL_MASK_3_DEF               0xffffffff

#define MOCA_SET_PQOS_CREATE_FLOW_FLOW_ID_DEF(x)   (x).addr[0] = 0x01; (x).addr[1] = 0x00; (x).addr[2] = 0x5e; (x).addr[3] = 0x00; (x).addr[4] = 0x01; (x).addr[5] = 0x00;

#define MOCA_SET_PQOS_CREATE_FLOW_PACKET_DA_DEF(x)  (x).addr[0] = 0x01; (x).addr[1] = 0x00; (x).addr[2] = 0x5e; (x).addr[3] = 0x00; (x).addr[4] = 0x01; (x).addr[5] = 0x00;

#define MOCA_PQOS_CREATE_FLOW_PACKET_SIZE_DEF      800
#define MOCA_PQOS_CREATE_FLOW_PACKET_SIZE_MIN      59

#define MOCA_PQOS_CREATE_FLOW_FLOW_TAG_DEF         0

#define MOCA_PQOS_CREATE_FLOW_PEAK_DATA_RATE_DEF   1000
#define MOCA_PQOS_CREATE_FLOW_PEAK_DATA_RATE_MIN   1
#define MOCA_PQOS_CREATE_FLOW_PEAK_DATA_RATE_MAX   0xFFFFFF

#define MOCA_PQOS_CREATE_FLOW_LEASE_TIME_DEF       0

#define MOCA_PQOS_CREATE_FLOW_BURST_SIZE_DEF       2
#define MOCA_PQOS_CREATE_FLOW_BURST_SIZE_MIN       0
#define MOCA_PQOS_CREATE_FLOW_BURST_SIZE_MAX       10

#define MOCA_PQOS_CREATE_FLOW_VLAN_ID_DEF          0xFFFFFFFF

#define MOCA_PQOS_CREATE_FLOW_MAX_LATENCY_DEF      0
#define MOCA_PQOS_CREATE_FLOW_MAX_LATENCY_MIN      0
#define MOCA_PQOS_CREATE_FLOW_MAX_LATENCY_MAX      255

#define MOCA_PQOS_CREATE_FLOW_SHORT_TERM_AVG_RATIO_DEF  255
#define MOCA_PQOS_CREATE_FLOW_SHORT_TERM_AVG_RATIO_MIN  0
#define MOCA_PQOS_CREATE_FLOW_SHORT_TERM_AVG_RATIO_MAX  255

#define MOCA_PQOS_CREATE_FLOW_MAX_RETRY_DEF        0
#define MOCA_PQOS_CREATE_FLOW_MAX_RETRY_MIN        0
#define MOCA_PQOS_CREATE_FLOW_MAX_RETRY_MAX        3

#define MOCA_PQOS_CREATE_FLOW_FLOW_PER_DEF         0
#define MOCA_PQOS_CREATE_FLOW_FLOW_PER_MIN         0
#define MOCA_PQOS_CREATE_FLOW_FLOW_PER_MAX         1

#define MOCA_PQOS_CREATE_FLOW_IN_ORDER_DELIVERY_DEF  0
#define MOCA_PQOS_CREATE_FLOW_IN_ORDER_DELIVERY_MIN  0
#define MOCA_PQOS_CREATE_FLOW_IN_ORDER_DELIVERY_MAX  2

#define MOCA_PQOS_CREATE_FLOW_TRAFFIC_PROTOCOL_DEF  0
#define MOCA_PQOS_CREATE_FLOW_TRAFFIC_PROTOCOL_MIN  0
#define MOCA_PQOS_CREATE_FLOW_TRAFFIC_PROTOCOL_MAX  5

#define MOCA_PQOS_CREATE_FLOW_INGR_CLASS_RULE_DEF  0

#define MOCA_PQOS_CREATE_FLOW_VLAN_TAG_DEF         5
#define MOCA_PQOS_CREATE_FLOW_VLAN_TAG_MIN         0
#define MOCA_PQOS_CREATE_FLOW_VLAN_TAG_MAX         7

#define MOCA_PQOS_CREATE_FLOW_DSCP_MOCA_DEF        0
#define MOCA_PQOS_CREATE_FLOW_DSCP_MOCA_MIN        0
#define MOCA_PQOS_CREATE_FLOW_DSCP_MOCA_MAX        7

#define MOCA_SET_PQOS_UPDATE_FLOW_FLOW_ID_DEF(x)   (x).addr[0] = 0x01; (x).addr[1] = 0x00; (x).addr[2] = 0x5e; (x).addr[3] = 0x00; (x).addr[4] = 0x00; (x).addr[5] = 0x00;

#define MOCA_PQOS_UPDATE_FLOW_RESERVED_DEF         0

#define MOCA_PQOS_UPDATE_FLOW_PACKET_SIZE_MIN      59

#define MOCA_PQOS_UPDATE_FLOW_PEAK_DATA_RATE_MIN   1
#define MOCA_PQOS_UPDATE_FLOW_PEAK_DATA_RATE_MAX   0xFFFFFE

#define MOCA_PQOS_UPDATE_FLOW_BURST_SIZE_MIN       1
#define MOCA_PQOS_UPDATE_FLOW_BURST_SIZE_MAX       9

#define MOCA_PQOS_UPDATE_FLOW_MAX_LATENCY_DEF      0
#define MOCA_PQOS_UPDATE_FLOW_MAX_LATENCY_MIN      0
#define MOCA_PQOS_UPDATE_FLOW_MAX_LATENCY_MAX      255

#define MOCA_PQOS_UPDATE_FLOW_SHORT_TERM_AVG_RATIO_DEF  255
#define MOCA_PQOS_UPDATE_FLOW_SHORT_TERM_AVG_RATIO_MIN  0
#define MOCA_PQOS_UPDATE_FLOW_SHORT_TERM_AVG_RATIO_MAX  255

#define MOCA_PQOS_UPDATE_FLOW_MAX_RETRY_DEF        0
#define MOCA_PQOS_UPDATE_FLOW_MAX_RETRY_MIN        0
#define MOCA_PQOS_UPDATE_FLOW_MAX_RETRY_MAX        3

#define MOCA_PQOS_UPDATE_FLOW_FLOW_PER_DEF         0
#define MOCA_PQOS_UPDATE_FLOW_FLOW_PER_MIN         0
#define MOCA_PQOS_UPDATE_FLOW_FLOW_PER_MAX         1

#define MOCA_PQOS_UPDATE_FLOW_IN_ORDER_DELIVERY_DEF  0
#define MOCA_PQOS_UPDATE_FLOW_IN_ORDER_DELIVERY_MIN  0
#define MOCA_PQOS_UPDATE_FLOW_IN_ORDER_DELIVERY_MAX  2

#define MOCA_PQOS_UPDATE_FLOW_TRAFFIC_PROTOCOL_DEF  0
#define MOCA_PQOS_UPDATE_FLOW_TRAFFIC_PROTOCOL_MIN  0
#define MOCA_PQOS_UPDATE_FLOW_TRAFFIC_PROTOCOL_MAX  5

#define MOCA_SET_PQOS_DELETE_FLOW_DEF(x)           (x).addr[0] = 0x01; (x).addr[1] = 0x00; (x).addr[2] = 0x5e; (x).addr[3] = 0x00; (x).addr[4] = 0x00; (x).addr[5] = 0x00;

#define MOCA_SET_PQOS_LIST_INGR_NODE_MAC_DEF(x)    (x).addr[0] = 0x00; (x).addr[1] = 0x00; (x).addr[2] = 0x00; (x).addr[3] = 0x00; (x).addr[4] = 0x00; (x).addr[5] = 0x00;

#define MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF         32
#define MOCA_PQOS_LIST_FLOW_MAX_RETURN_MIN         0
#define MOCA_PQOS_LIST_FLOW_MAX_RETURN_MAX         32

#define MOCA_PQOS_LIST_FLOW_START_INDEX_DEF        0

#define MOCA_SET_PQOS_QUERY_DEF(x)                 (x).addr[0] = 0x01; (x).addr[1] = 0x00; (x).addr[2] = 0x5e; (x).addr[3] = 0x00; (x).addr[4] = 0x01; (x).addr[5] = 0x00;

#define MOCA_PQOS_STATUS_DEF                       0

#define MOCA_HOST_QOS_FC_CAPABLE_CHIP_DEF          1
#define MOCA_HOST_QOS_DEF                          0

#define MOCA_TABOO_CHANNELS_TABOO_FIXED_MASK_START_DEF  0

#define MOCA_TABOO_CHANNELS_TABOO_FIXED_CHANNEL_MASK_DEF  0x0

#define MOCA_TABOO_CHANNELS_TABOO_LEFT_MASK_DEF    0x00ffffff

#define MOCA_TABOO_CHANNELS_TABOO_RIGHT_MASK_DEF   0xffffff00

#define MOCA_GEN_NODE_STATUS_NODE_TX_BACKOFF_MIN   0
#define MOCA_GEN_NODE_STATUS_NODE_TX_BACKOFF_MAX   35

#define MOCA_GEN_NODE_STATUS_MIN                   0
#define MOCA_GEN_NODE_STATUS_MAX                   15

#define MOCA_GEN_NODE_EXT_STATUS_NBAS_MIN          224
#define MOCA_GEN_NODE_EXT_STATUS_NBAS_MAX          4480

#define MOCA_GEN_NODE_EXT_STATUS_PREAMBLE_TYPE_MIN  0
#define MOCA_GEN_NODE_EXT_STATUS_PREAMBLE_TYPE_MAX  10

#define MOCA_GEN_NODE_EXT_STATUS_CP_MIN            10
#define MOCA_GEN_NODE_EXT_STATUS_CP_MAX            128

#define MOCA_GEN_NODE_EXT_STATUS_AGC_ADDRESS_MIN   0
#define MOCA_GEN_NODE_EXT_STATUS_AGC_ADDRESS_MAX   127

#define MOCA_GEN_NODE_EXT_STATUS_INDEX_MIN         0
#define MOCA_GEN_NODE_EXT_STATUS_INDEX_MAX         15

#define MOCA_GEN_NODE_EXT_STATUS_PROFILE_TYPE_MIN  0
#define MOCA_GEN_NODE_EXT_STATUS_PROFILE_TYPE_MAX  21

#define MOCA_NODE_STATS_INDEX_MIN                  0
#define MOCA_NODE_STATS_INDEX_MAX                  15

#define MOCA_NODE_STATS_RESET_STATS_DEF            0

#define MOCA_NODE_STATS_EXT_INDEX_MIN              0
#define MOCA_NODE_STATS_EXT_INDEX_MAX              15

#define MOCA_NODE_STATS_EXT_RESET_STATS_DEF        0

#define MOCA_NETWORK_STATUS_NODE_ID_MIN            0
#define MOCA_NETWORK_STATUS_NODE_ID_MAX            15

#define MOCA_NETWORK_STATUS_NC_NODE_ID_MIN         0
#define MOCA_NETWORK_STATUS_NC_NODE_ID_MAX         15

#define MOCA_NETWORK_STATUS_BACKUP_NC_ID_MIN       0
#define MOCA_NETWORK_STATUS_BACKUP_NC_ID_MAX       15

#define MOCA_OOO_LMO_MIN                           0
#define MOCA_OOO_LMO_MAX                           15

#define MOCA_START_ULMO_REPORT_TYPE_MIN            0
#define MOCA_START_ULMO_REPORT_TYPE_MAX            2

#define MOCA_START_ULMO_NODE_ID_MIN                0
#define MOCA_START_ULMO_NODE_ID_MAX                0x3F

#define MOCA_START_ULMO_OFDMA_NODE_MASK_MIN        0
#define MOCA_START_ULMO_OFDMA_NODE_MASK_MAX        0xFFFF

#define MOCA_RXD_LMO_REQUEST_NODE_ID_MIN           0
#define MOCA_RXD_LMO_REQUEST_NODE_ID_MAX           15

#define MOCA_RXD_LMO_REQUEST_PROBE_ID_MIN          1
#define MOCA_RXD_LMO_REQUEST_PROBE_ID_MAX          2

#define MOCA_RXD_LMO_REQUEST_CHANNEL_ID_DEF        0
#define MOCA_RXD_LMO_REQUEST_CHANNEL_ID_MIN        0
#define MOCA_RXD_LMO_REQUEST_CHANNEL_ID_MAX        2

#define MOCA_ACA_SRC_NODE_MIN                      0
#define MOCA_ACA_SRC_NODE_MAX                      15

#define MOCA_ACA_DEST_NODEMASK_MIN                 1
#define MOCA_ACA_DEST_NODEMASK_MAX                 0xFFFF

#define MOCA_ACA_TYPE_DEF                          1
#define MOCA_ACA_TYPE_MIN                          1
#define MOCA_ACA_TYPE_MAX                          2

#define MOCA_ACA_NUM_PROBES_MAX                    8

#define MOCA_FMR_INIT_DEF                          0

#define MOCA_MOCA_RESET_RESET_TIMER_DEF            1
#define MOCA_MOCA_RESET_RESET_TIMER_MAX            255

#define MOCA_MOCA_RESET_NON_DEF_SEQ_NUM_DEF        0x10000
#define MOCA_MOCA_RESET_NON_DEF_SEQ_NUM_MIN        0
#define MOCA_MOCA_RESET_NON_DEF_SEQ_NUM_MAX        0x10000

#define MOCA_DD_INIT_DEF                           0xFFFF

#define MOCA_FMR_20_DEF                            0

#define MOCA_HOSTLESS_MODE_DEF                     0

#define MOCA_WAKEUP_NODE_DEF                       0
#define MOCA_WAKEUP_NODE_MIN                       0
#define MOCA_WAKEUP_NODE_MAX                       15

#define MOCA_IF_ACCESS_EN_DEF                      0
#define MOCA_IF_ACCESS_EN_MIN                      0
#define MOCA_IF_ACCESS_EN_MAX                      1

#define MOCA_LED_MODE_DEF                          0
#define MOCA_LED_MODE_MIN                          0
#define MOCA_LED_MODE_MAX                          2
#define MOCA_LED_MODE_STANDALONE_MAX               1

#define MOCA_GEN_STATS_DEF                         0

#define MOCA_INTERFACE_STATUS_RF_CHANNEL_MIN       20
#define MOCA_INTERFACE_STATUS_RF_CHANNEL_MAX       73

#define MOCA_M1_TX_POWER_VARIATION_DEF             0
#define MOCA_M1_TX_POWER_VARIATION_MIN             0
#define MOCA_M1_TX_POWER_VARIATION_MAX             6

#define MOCA_NC_LISTENING_INTERVAL_DEF             10
#define MOCA_NC_LISTENING_INTERVAL_MIN             1
#define MOCA_NC_LISTENING_INTERVAL_MAX             10

#define MOCA_NC_HEARTBEAT_INTERVAL_DEF             10
#define MOCA_NC_HEARTBEAT_INTERVAL_MIN             1
#define MOCA_NC_HEARTBEAT_INTERVAL_MAX             255

#define MOCA_WOM_MAGIC_ENABLE_DEF                  0
#define MOCA_WOM_MAGIC_ENABLE_MIN                  0
#define MOCA_WOM_MAGIC_ENABLE_MAX                  1

#define MOCA_PM_RESTORE_ON_LINK_DOWN_DEF           0
#define MOCA_PM_RESTORE_ON_LINK_DOWN_MIN           0
#define MOCA_PM_RESTORE_ON_LINK_DOWN_MAX           1

#define MOCA_POWER_STATE_MIN                       0
#define MOCA_POWER_STATE_MAX                       3

#define MOCA_FILTER_M2_DATA_WAKEUP_DEF             0

#define MOCA_WOM_PATTERN_INDEX_MIN                 0
#define MOCA_WOM_PATTERN_INDEX_MAX                 4

#define MOCA_WOM_PATTERN_MASK_DEF                  0xFF

#define MOCA_WOM_IP_INDEX_MIN                      0
#define MOCA_WOM_IP_INDEX_MAX                      4

#define MOCA_STANDBY_POWER_STATE_DEF               2
#define MOCA_STANDBY_POWER_STATE_MIN               0
#define MOCA_STANDBY_POWER_STATE_MAX               3

#define MOCA_WOM_MODE_DEF                          0
#define MOCA_WOM_MODE_SWITCH_DEF                   2
#define MOCA_WOM_MODE_MIN                          0
#define MOCA_WOM_MODE_MAX                          2

#define MOCA_POWER_STATE_RSP_MIN                   0
#define MOCA_POWER_STATE_RSP_MAX                   1

#define MOCA_POWER_STATE_EVENT_MIN                 0
#define MOCA_POWER_STATE_EVENT_MAX                 6

#define MOCA_PS_CMD_MIN                            0
#define MOCA_PS_CMD_MAX                            3

#define MOCA_PRIVACY_EN_DEF                        0

#define MOCA_PMK_EXCHANGE_INTERVAL_DEF             39600000
#define MOCA_PMK_EXCHANGE_INTERVAL_MIN             20000

#define MOCA_TEK_EXCHANGE_INTERVAL_DEF             540000
#define MOCA_TEK_EXCHANGE_INTERVAL_MIN             20000

#define MOCA_AES_EXCHANGE_INTERVAL_DEF             25200000
#define MOCA_AES_EXCHANGE_INTERVAL_MIN             20000

#define MOCA_PASSWORD_PASSWORD_DEF                 0

#define MOCA_MTM_EN_DEF                            0

#define MOCA_CIR_PRINTS_DEF                        0

#define MOCA_SNR_PRINTS_DEF                        0

#define MOCA_SIGMA2_PRINTS_DEF                     0

#define MOCA_BAD_PROBE_PRINTS_DEF                  0

#define MOCA_CONST_TX_PARAMS_CONST_TX_SUBMODE_DEF  1
#define MOCA_CONST_TX_PARAMS_CONST_TX_SUBMODE_MIN  0
#define MOCA_CONST_TX_PARAMS_CONST_TX_SUBMODE_MAX  3

#define MOCA_GMII_TRAP_HEADER_ETH_TYPE_DEF         0x800

#define MOCA_GMII_TRAP_HEADER_DSCP_ECN_DEF         0

#define MOCA_GMII_TRAP_HEADER_ID_DEF               0

#define MOCA_GMII_TRAP_HEADER_FLAGS_FRAGOFFS_DEF   0

#define MOCA_GMII_TRAP_HEADER_TTL_DEF              32

#define MOCA_GMII_TRAP_HEADER_PROT_DEF             17

#define MOCA_MOCA_CORE_TRACE_ENABLE_DEF            0

#define MOCA_DONT_START_MOCA_DEF                   0

#define MOCA_LAB_MODE_DEF                          0

#define MOCA_NC_MODE_DEF                           0

#define MOCA_RX_TX_PACKETS_PER_QM_DEF              18

#define MOCA_EXTRA_RX_PACKETS_PER_QM_DEF           6

#define MOCA_TARGET_PHY_RATE_20_DEF                630

#define MOCA_TARGET_PHY_RATE_20_TURBO_DEF          670

#define MOCA_TURBO_EN_DEF                          0

#define MOCA_TARGET_PHY_RATE_20_TURBO_VLPER_DEF    650

#define MOCA_TARGET_PHY_RATE_20_SEC_CH_DEF         630

#define MOCA_TARGET_PHY_RATE_20_TURBO_SEC_CH_DEF   670

#define MOCA_TARGET_PHY_RATE_20_TURBO_VLPER_SEC_CH_DEF  650

#define MOCA_CAP_PHY_RATE_EN_DEF                   0

#define MOCA_CAP_TARGET_PHY_RATE_DEF               670

#define MOCA_CAP_SNR_BASE_MARGIN_DEF               400

#define MOCA_LAB_IQ_DIAGRAM_SET_NODEID_MIN         0
#define MOCA_LAB_IQ_DIAGRAM_SET_NODEID_MAX         15

#define MOCA_FORCE_HANDOFF_NEXTBACKUP_DEF          0

#define MOCA_MPS_EN_DEF                            1
#define MOCA_MPS_EN_MIN                            0
#define MOCA_MPS_EN_MAX                            1

#define MOCA_MPS_PRIVACY_RECEIVE_DEF               1
#define MOCA_MPS_PRIVACY_RECEIVE_MIN               0
#define MOCA_MPS_PRIVACY_RECEIVE_MAX               1

#define MOCA_MPS_PRIVACY_DOWN_DEF                  0
#define MOCA_MPS_PRIVACY_DOWN_MIN                  0
#define MOCA_MPS_PRIVACY_DOWN_MAX                  1

#define MOCA_MPS_WALK_TIME_DEF                     120
#define MOCA_MPS_WALK_TIME_MIN                     12
#define MOCA_MPS_WALK_TIME_MAX                     1200

#define MOCA_MPS_UNPAIRED_TIME_DEF                 300
#define MOCA_MPS_UNPAIRED_TIME_MIN                 120
#define MOCA_MPS_UNPAIRED_TIME_MAX                 7200

#define MOCA_MPS_STATE_DEF                         0

#define MOCA_PRIVACY_DEFAULTS_DEF                  0
#define MOCA_PRIVACY_DEFAULTS_MIN                  0
#define MOCA_PRIVACY_DEFAULTS_MAX                  1


/* Default flags for use when calling moca_restore_defaults() */
#define MOCA_7425B0_FLAG             (1 << 0)
#define MOCA_7435B0_FLAG             (1 << 1)
#define MOCA_6802C0_FLAG             (1 << 2)
#define MOCA_6803C0_FLAG             (1 << 3)
#define MOCA_7428B0_FLAG             (1 << 4)
#define MOCA_28NM_FLAG               (1 << 5)
#define MOCA_7445D0_FLAG             (1 << 6)
#define MOCA_74371B0_FLAG            (1 << 7)
#define MOCA_BAND_E_FLAG             (1 << 8)
#define MOCA_BAND_F_FLAG             (1 << 9)
#define MOCA_BAND_GENERIC_FLAG       (1 << 10)
#define MOCA_BAND_EX_D_FLAG          (1 << 11)
#define MOCA_BAND_D_LOW_FLAG         (1 << 12)
#define MOCA_BAND_D_HIGH_FLAG        (1 << 13)
#define MOCA_BAND_C4_FLAG            (1 << 14)
#define MOCA_BAND_H_FLAG             (1 << 15)
#define MOCA_3390B0_FLAG             (1 << 16)
#define MOCA_BONDING_SUPPORTED_FLAG  (1 << 17)
#define MOCA_STANDALONE_FLAG         (1 << 18)
#define MOCA_6802B0_FLAG             (1 << 19)
#define MOCA_7425_FLAG               (1 << 20)
#define MOCA_6816_FLAG               (1 << 21)
#define MOCA_7xxx_FLAG               (1 << 22)
#define MOCA_FC_CAPABLE_CHIP_FLAG    (1 << 23)
#define MOCA_7408_FLAG               (1 << 24)
#define MOCA_SWITCH_FLAG             (1 << 25)


#define MOCA_CONTINUOUS_RX_MODE_ATTN_ERR                               -1000
#define MOCA_NO_IFG6_ERR                                               -1003
#define MOCA_BONDING_ERR                                               -1005
#define MOCA_LISTENING_DURATION_ERR                                    -1006
#define MOCA_LIMIT_TRAFFIC_ERR                                         -1007
#define MOCA_REMOTE_MAN_ERR                                            -1008
#define MOCA_C4_MOCA20_EN_ERR                                          -1009
#define MOCA_POWER_SAVE_MECHANISM_DIS_ERR                              -1010
#define MOCA_PSM_CONFIG_ERR                                            -1011
#define MOCA_USE_EXT_DATA_MEM_ERR                                      -1012
#define MOCA_AIF_MODE_ERR                                              -1013
#define MOCA_PROP_BONDING_COMPATIBILITY_MODE_ERR                       -1015
#define MOCA_RDEG_3450_ERR                                             -1016
#define MOCA_PHY_CLOCK_ERR                                             -1017
#define MOCA_MAX_TX_POWER_TUNE_OFFSET_ERR                              -1019
#define MOCA_MAX_TX_POWER_TUNE_SEC_CH_OFFSET_ERR                       -1020
#define MOCA_RX_POWER_TUNE_OFFSET_ERR                                  -1021
#define MOCA_IMPEDANCE_MODE_BONDING_ERR                                -1022
#define MOCA_REWORK_6802_ERR                                           -1023
#define MOCA_LOF_UPDATE_ERR                                            -1025
#define MOCA_PRIMARY_CH_OFFSET_ERR                                     -1026
#define MOCA_MR_SEQ_NUM_ERR                                            -1027
#define MOCA_SECONDARY_CH_OFFSET_ERR                                   -1028
#define MOCA_MAX_TX_POWER_ERR                                          -1029
#define MOCA_BEACON_PWR_REDUCTION_ERR                                  -1030
#define MOCA_BEACON_PWR_REDUCTION_EN_ERR                               -1031
#define MOCA_STAR_TOPOLOGY_EN_ERR                                      -1032
#define MOCA_OFDMA_EN_ERR                                              -1033
#define MOCA_MIN_BW_ALARM_THRESHOLD_ERR                                -1034
#define MOCA_TARGET_PHY_RATE_QAM128_ERR                                -1035
#define MOCA_TARGET_PHY_RATE_QAM256_ERR                                -1036
#define MOCA_ARPL_TH_50_ERR                                            -1037
#define MOCA_BANDWIDTH_ERR                                             -1038
#define MOCA_ARPL_TH_100_ERR                                           -1039
#define MOCA_MAX_CONSTELLATION_ALL_ERR                                 -1043
#define MOCA_MAX_CONSTELLATION_NODE_ID_ERR                             -1044
#define MOCA_MAX_CONSTELLATION_P2P_LIMIT_50_ERR                        -1045
#define MOCA_MAX_CONSTELLATION_GCD_LIMIT_50_ERR                        -1046
#define MOCA_MAX_CONSTELLATION_P2P_LIMIT_100_ERR                       -1047
#define MOCA_MAX_CONSTELLATION_GCD_LIMIT_100_ERR                       -1048
#define MOCA_RLAPM_TABLE_50_RLAPMTABLE_ERR                             -1052
#define MOCA_RLAPM_TABLE_100_RLAPMTABLE_ERR                            -1059
#define MOCA_NV_CAL_ENABLE_ERR                                         -1066
#define MOCA_SNR_MARGIN_RS_BASE_MARGIN_ERR                             -1067
#define MOCA_SNR_MARGIN_RS_OFFSETS_ERR                                 -1068
#define MOCA_SNR_MARGIN_LDPC_BASE_MARGIN_ERR                           -1069
#define MOCA_SNR_MARGIN_LDPC_OFFSETS_ERR                               -1070
#define MOCA_SNR_MARGIN_LDPC_SEC_CH_BASE_MARGIN_ERR                    -1071
#define MOCA_SNR_MARGIN_LDPC_SEC_CH_OFFSETS_ERR                        -1072
#define MOCA_SNR_MARGIN_LDPC_PRE5_BASE_MARGIN_ERR                      -1073
#define MOCA_SNR_MARGIN_LDPC_PRE5_OFFSETS_ERR                          -1074
#define MOCA_SNR_MARGIN_OFDMA_BASE_MARGIN_ERR                          -1075
#define MOCA_SNR_MARGIN_OFDMA_OFFSETS_ERR                              -1076
#define MOCA_SAPM_TABLE_50_VAL_ERR                                     -1077
#define MOCA_SAPM_TABLE_100_VAL_ERR                                    -1078
#define MOCA_SAPM_TABLE_SEC_VAL_ERR                                    -1079
#define MOCA_SNR_MARGIN_LDPC_PRI_CH_BASE_MARGIN_ERR                    -1080
#define MOCA_SNR_MARGIN_LDPC_PRI_CH_OFFSETS_ERR                        -1081
#define MOCA_SNR_MARGIN_PRE5_PRI_CH_BASE_MARGIN_ERR                    -1082
#define MOCA_SNR_MARGIN_PRE5_PRI_CH_OFFSETS_ERR                        -1083
#define MOCA_SNR_MARGIN_PRE5_SEC_CH_BASE_MARGIN_ERR                    -1084
#define MOCA_SNR_MARGIN_PRE5_SEC_CH_OFFSETS_ERR                        -1085
#define MOCA_MAX_FRAME_SIZE_ERR                                        -1086
#define MOCA_MAX_TRANSMIT_TIME_ERR                                     -1088
#define MOCA_MAX_PKT_AGGR_ERR                                          -1089
#define MOCA_RTR_CONFIG_LOW_ERR                                        -1092
#define MOCA_RTR_CONFIG_MED_ERR                                        -1093
#define MOCA_RTR_CONFIG_HIGH_ERR                                       -1094
#define MOCA_RTR_CONFIG_BG_ERR                                         -1095
#define MOCA_TLP_MODE_ERR                                              -1096
#define MOCA_MAX_PKT_AGGR_BONDING_ERR                                  -1097
#define MOCA_FC_MODE_ERR                                               -1098
#define MOCA_PQOS_MAX_PACKET_SIZE_ERR                                  -1099
#define MOCA_PER_MODE_ERR                                              -1100
#define MOCA_POLICING_EN_ERR                                           -1101
#define MOCA_ORR_EN_ERR                                                -1102
#define MOCA_BRCMTAG_ENABLE_ERR                                        -1103
#define MOCA_UNKNOWN_RATELIMIT_EN_ERR                                  -1104
#define MOCA_EGR_MC_ADDR_FILTER_ENTRYID_ERR                            -1113
#define MOCA_PAUSE_FC_EN_ERR                                           -1114
#define MOCA_STAG_PRIORITY_ENABLE_ERR                                  -1115
#define MOCA_STAG_PRIORITY_TAG_MASK_ERR                                -1116
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_0_ERR                         -1117
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_0_ERR                          -1118
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_1_ERR                         -1119
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_1_ERR                          -1120
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_2_ERR                         -1121
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_2_ERR                          -1122
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_3_ERR                         -1123
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_3_ERR                          -1124
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_4_ERR                         -1125
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_4_ERR                          -1126
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_5_ERR                          -1127
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_6_ERR                         -1128
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_6_ERR                          -1129
#define MOCA_STAG_PRIORITY_MOCA_PRIORITY_7_ERR                         -1130
#define MOCA_STAG_PRIORITY_TAG_PRIORITY_7_ERR                          -1131
#define MOCA_STAG_REMOVAL_ENABLE_ERR                                   -1132
#define MOCA_STAG_REMOVAL_VALID_0_ERR                                  -1133
#define MOCA_STAG_REMOVAL_VALID_1_ERR                                  -1134
#define MOCA_STAG_REMOVAL_VALID_2_ERR                                  -1135
#define MOCA_STAG_REMOVAL_VALID_3_ERR                                  -1136
#define MOCA_PQOS_CREATE_FLOW_PACKET_SIZE_ERR                          -1137
#define MOCA_PQOS_CREATE_FLOW_PEAK_DATA_RATE_ERR                       -1138
#define MOCA_PQOS_CREATE_FLOW_BURST_SIZE_ERR                           -1139
#define MOCA_PQOS_CREATE_FLOW_MAX_LATENCY_ERR                          -1140
#define MOCA_PQOS_CREATE_FLOW_SHORT_TERM_AVG_RATIO_ERR                 -1141
#define MOCA_PQOS_CREATE_FLOW_MAX_RETRY_ERR                            -1142
#define MOCA_PQOS_CREATE_FLOW_FLOW_PER_ERR                             -1143
#define MOCA_PQOS_CREATE_FLOW_IN_ORDER_DELIVERY_ERR                    -1144
#define MOCA_PQOS_CREATE_FLOW_TRAFFIC_PROTOCOL_ERR                     -1145
#define MOCA_PQOS_CREATE_FLOW_INGR_CLASS_RULE_ERR                      -1146
#define MOCA_PQOS_CREATE_FLOW_VLAN_TAG_ERR                             -1147
#define MOCA_PQOS_CREATE_FLOW_DSCP_MOCA_ERR                            -1148
#define MOCA_PQOS_UPDATE_FLOW_PACKET_SIZE_ERR                          -1149
#define MOCA_PQOS_UPDATE_FLOW_PEAK_DATA_RATE_ERR                       -1150
#define MOCA_PQOS_UPDATE_FLOW_BURST_SIZE_ERR                           -1151
#define MOCA_PQOS_UPDATE_FLOW_MAX_LATENCY_ERR                          -1152
#define MOCA_PQOS_UPDATE_FLOW_SHORT_TERM_AVG_RATIO_ERR                 -1153
#define MOCA_PQOS_UPDATE_FLOW_MAX_RETRY_ERR                            -1154
#define MOCA_PQOS_UPDATE_FLOW_FLOW_PER_ERR                             -1155
#define MOCA_PQOS_UPDATE_FLOW_IN_ORDER_DELIVERY_ERR                    -1156
#define MOCA_PQOS_UPDATE_FLOW_TRAFFIC_PROTOCOL_ERR                     -1157
#define MOCA_PQOS_LIST_FLOW_MAX_RETURN_ERR                             -1158
#define MOCA_OOO_LMO_ERR                                               -1159
#define MOCA_START_ULMO_REPORT_TYPE_ERR                                -1160
#define MOCA_START_ULMO_NODE_ID_ERR                                    -1161
#define MOCA_START_ULMO_OFDMA_NODE_MASK_ERR                            -1162
#define MOCA_RXD_LMO_REQUEST_NODE_ID_ERR                               -1163
#define MOCA_RXD_LMO_REQUEST_PROBE_ID_ERR                              -1164
#define MOCA_RXD_LMO_REQUEST_CHANNEL_ID_ERR                            -1165
#define MOCA_ACA_SRC_NODE_ERR                                          -1170
#define MOCA_ACA_DEST_NODEMASK_ERR                                     -1171
#define MOCA_ACA_TYPE_ERR                                              -1172
#define MOCA_ACA_NUM_PROBES_ERR                                        -1173
#define MOCA_MOCA_RESET_RESET_TIMER_ERR                                -1174
#define MOCA_MOCA_RESET_NON_DEF_SEQ_NUM_ERR                            -1175
#define MOCA_WAKEUP_NODE_ERR                                           -1176
#define MOCA_IF_ACCESS_EN_ERR                                          -1178
#define MOCA_LED_MODE_ERR                                              -1179
#define MOCA_M1_TX_POWER_VARIATION_ERR                                 -1180
#define MOCA_NC_LISTENING_INTERVAL_ERR                                 -1181
#define MOCA_NC_HEARTBEAT_INTERVAL_ERR                                 -1182
#define MOCA_WOM_MAGIC_ENABLE_ERR                                      -1184
#define MOCA_PM_RESTORE_ON_LINK_DOWN_ERR                               -1185
#define MOCA_WOM_PATTERN_INDEX_ERR                                     -1187
#define MOCA_WOM_IP_INDEX_ERR                                          -1188
#define MOCA_STANDBY_POWER_STATE_ERR                                   -1189
#define MOCA_WOM_MODE_ERR                                              -1190
#define MOCA_PS_CMD_ERR                                                -1191
#define MOCA_PMK_EXCHANGE_INTERVAL_ERR                                 -1192
#define MOCA_TEK_EXCHANGE_INTERVAL_ERR                                 -1193
#define MOCA_AES_EXCHANGE_INTERVAL_ERR                                 -1194
#define MOCA_CONST_TX_PARAMS_CONST_TX_SUBMODE_ERR                      -1196
#define MOCA_LAB_IQ_DIAGRAM_SET_NODEID_ERR                             -1198
#define MOCA_MPS_EN_ERR                                                -1199
#define MOCA_MPS_PRIVACY_RECEIVE_ERR                                   -1200
#define MOCA_MPS_PRIVACY_DOWN_ERR                                      -1201
#define MOCA_MPS_WALK_TIME_ERR                                         -1202
#define MOCA_MPS_UNPAIRED_TIME_ERR                                     -1203
#define MOCA_PRIVACY_DEFAULTS_ERR                                      -1204


struct moca_mac_addr {
   macaddr_t               val;
} __attribute__((packed,aligned(4)));

struct moca_node_status {
   uint32_t                vendor_id;
   uint32_t                moca_hw_version;
   uint32_t                moca_sw_version_major;
   uint32_t                moca_sw_version_minor;
   uint32_t                moca_sw_version_rev;
   uint32_t                self_moca_version;
   uint32_t                qam_256_support;
} __attribute__((packed,aligned(4)));

struct moca_fw_version {
   uint32_t                version_moca;
   uint32_t                version_major;
   uint32_t                version_minor;
   uint32_t                version_patch;
} __attribute__((packed,aligned(4)));

struct moca_max_tx_power_tune {
   int8_t                  offset[86];
   uint16_t                padding;
} __attribute__((packed,aligned(4)));

struct moca_max_tx_power_tune_sec_ch {
   int8_t                  offset[86];
   uint16_t                padding;
} __attribute__((packed,aligned(4)));

struct moca_rx_power_tune {
   int8_t                  offset[86];
   uint16_t                padding;
} __attribute__((packed,aligned(4)));

struct moca_mocad_forwarding_rx_ack {
   uint32_t                offset;
   uint32_t                size;
} __attribute__((packed,aligned(4)));

struct moca_mocad_forwarding_tx_alloc {
   uint32_t                offset;
   uint32_t                size;
   uint32_t                count;
} __attribute__((packed,aligned(4)));

struct moca_mocad_forwarding_tx_send {
   uint32_t                offset;
   uint32_t                size;
   uint32_t                dest_if;
} __attribute__((packed,aligned(4)));

struct moca_core_ready {
   uint8_t                 chip_type;
   uint8_t                 phy_freq_mhz;
   uint8_t                 compatibility;
   uint8_t                 reserved;
   uint32_t                syncVersion;
} __attribute__((packed,aligned(4)));

struct moca_mocad_forwarding_rx_packet {
   uint32_t                offset;
   uint32_t                length;
} __attribute__((packed,aligned(4)));

struct moca_drv_info {
   uint32_t                version;
   uint32_t                build_number;
   uint32_t                hw_rev;
   uint32_t                uptime;
   uint32_t                link_uptime;
   uint32_t                core_uptime;
   char                    ifname[16];
   char                    devname[64];
   uint32_t                rf_band;
   uint32_t                chip_id;
   uint32_t                reset_count;
   uint32_t                link_up_count;
   uint32_t                link_down_count;
   uint32_t                topology_change_count;
   uint32_t                assert_count;
   int32_t                 last_assert_num;
   uint32_t                wdog_count;
   uint32_t                restart_history;
} __attribute__((packed,aligned(4)));

struct moca_mocad_version {
   uint32_t                mocad_version_moca;
   uint32_t                mocad_version_major;
   uint32_t                mocad_version_minor;
   uint32_t                mocad_version_patch;
} __attribute__((packed,aligned(4)));

struct moca_max_constellation {
   uint32_t                node_id;
   uint32_t                p2p_limit_50;
   uint32_t                gcd_limit_50;
   uint32_t                p2p_limit_100;
   uint32_t                gcd_limit_100;
} __attribute__((packed,aligned(4)));

struct moca_rlapm_table_50 {
   uint8_t                 rlapmtable[66];
   uint8_t                 reserved_0;
   uint8_t                 reserved_1;
} __attribute__((packed,aligned(4)));

struct moca_rlapm_table_100 {
   uint8_t                 rlapmtable[66];
   uint8_t                 reserved_0;
   uint8_t                 reserved_1;
} __attribute__((packed,aligned(4)));

struct moca_rx_gain_params {
   uint32_t                lna_ctrl_reg;
   uint32_t                is3451;
} __attribute__((packed,aligned(4)));

struct moca_tx_power_params {
   uint32_t                channelMode;
   uint32_t                channel;
   uint32_t                user_reduce_power;
   uint32_t                channel_reduce_tune;
   uint32_t                tx_digital_gain;
   uint32_t                pad_ctrl_deg;
   uint32_t                pa_ctrl_reg;
   uint32_t                is3451;
   uint32_t                table_max_index;
   uint16_t                tx_table[62];
} __attribute__((packed,aligned(4)));

struct moca_tx_power_params_in {
   uint32_t                channelMode;
   uint32_t                txTableIndex;
} __attribute__((packed,aligned(4)));

struct moca_snr_margin_rs {
   int32_t                 base_margin;
   int16_t                 offsets[10];
} __attribute__((packed,aligned(4)));

struct moca_snr_margin_ldpc {
   int32_t                 base_margin;
   int16_t                 offsets[10];
} __attribute__((packed,aligned(4)));

struct moca_snr_margin_ldpc_sec_ch {
   int32_t                 base_margin;
   int16_t                 offsets[10];
} __attribute__((packed,aligned(4)));

struct moca_snr_margin_ldpc_pre5 {
   int32_t                 base_margin;
   int16_t                 offsets[10];
} __attribute__((packed,aligned(4)));

struct moca_snr_margin_ofdma {
   int32_t                 base_margin;
   int16_t                 offsets[10];
} __attribute__((packed,aligned(4)));

struct moca_sapm_table_50 {
   uint8_t                 val[256];
} __attribute__((packed,aligned(4)));

struct moca_sapm_table_100 {
   uint8_t                 val[512];
} __attribute__((packed,aligned(4)));

struct moca_sapm_table_sec {
   uint8_t                 val[512];
} __attribute__((packed,aligned(4)));

struct moca_amp_reg {
   uint32_t                addr;
   uint32_t                value;
} __attribute__((packed,aligned(4)));

struct moca_snr_margin_ldpc_pri_ch {
   int32_t                 base_margin;
   int16_t                 offsets[10];
} __attribute__((packed,aligned(4)));

struct moca_snr_margin_pre5_pri_ch {
   int32_t                 base_margin;
   int16_t                 offsets[10];
} __attribute__((packed,aligned(4)));

struct moca_snr_margin_pre5_sec_ch {
   int32_t                 base_margin;
   int16_t                 offsets[10];
} __attribute__((packed,aligned(4)));

struct moca_rtr_config {
   uint8_t                 low;
   uint8_t                 med;
   uint8_t                 high;
   uint8_t                 bg;
} __attribute__((packed,aligned(4)));

struct moca_egr_mc_addr_filter {
   uint32_t                entryid;
   uint32_t                valid;
   macaddr_t               addr;
   uint16_t                reserved_0;
} __attribute__((packed,aligned(4)));

struct moca_egr_mc_addr_filter_set {
   uint32_t                entryid;
   uint32_t                valid;
   macaddr_t               addr;
} __attribute__((packed,aligned(4)));

struct moca_uc_fwd {
   macaddr_t               mac_addr;
   uint16_t                moca_dest_node_id;
} __attribute__((packed,aligned(4)));

struct moca_mc_fwd {
   macaddr_t               multicast_mac_addr;
   uint16_t                reserved_0;
   uint32_t                dest_node_id;
} __attribute__((packed,aligned(4)));

struct moca_mc_fwd_set {
   macaddr_t               multicast_mac_addr;
   macaddr_t               dest_mac_addr1;
   macaddr_t               dest_mac_addr2;
   macaddr_t               dest_mac_addr3;
   macaddr_t               dest_mac_addr4;
} __attribute__((packed,aligned(4)));

struct moca_src_addr {
   macaddr_t               mac_addr;
   uint16_t                moca_node_id;
} __attribute__((packed,aligned(4)));

struct moca_mac_aging {
   uint16_t                uc_fwd_age;
   uint16_t                mc_fwd_age;
   uint16_t                src_addr_age;
} __attribute__((packed,aligned(4)));

struct moca_mcfilter_addentry {
   macaddr_t               addr;
} __attribute__((packed,aligned(4)));

struct moca_mcfilter_delentry {
   macaddr_t               addr;
} __attribute__((packed,aligned(4)));

struct moca_stag_priority {
   uint32_t                enable;
   uint32_t                tag_mask;
   uint32_t                moca_priority_0;
   uint32_t                tag_priority_0;
   uint32_t                moca_priority_1;
   uint32_t                tag_priority_1;
   uint32_t                moca_priority_2;
   uint32_t                tag_priority_2;
   uint32_t                moca_priority_3;
   uint32_t                tag_priority_3;
   uint32_t                moca_priority_4;
   uint32_t                tag_priority_4;
   uint32_t                moca_priority_5;
   uint32_t                tag_priority_5;
   uint32_t                moca_priority_6;
   uint32_t                tag_priority_6;
   uint32_t                moca_priority_7;
   uint32_t                tag_priority_7;
} __attribute__((packed,aligned(4)));

struct moca_stag_removal {
   uint32_t                enable;
   uint32_t                valid_0;
   uint32_t                value_0;
   uint32_t                mask_0;
   uint32_t                valid_1;
   uint32_t                value_1;
   uint32_t                mask_1;
   uint32_t                valid_2;
   uint32_t                value_2;
   uint32_t                mask_2;
   uint32_t                valid_3;
   uint32_t                value_3;
   uint32_t                mask_3;
} __attribute__((packed,aligned(4)));

struct moca_pqos_maintenance_complete {
   uint32_t                iocovercommit;
   uint32_t                allocatedstps;
   uint32_t                allocatedtxps;
} __attribute__((packed,aligned(4)));

struct moca_pqos_create_flow_out {
   macaddr_t               flow_id;
   macaddr_t               flowda;
   uint32_t                response_code;
   uint32_t                decision;
   uint32_t                flow_tag;
   uint32_t                peak_data_rate;
   uint32_t                packet_size;
   uint32_t                burst_size;
   uint32_t                lease_time;
   uint32_t                total_stps;
   uint32_t                total_txps;
   uint32_t                flow_stps;
   uint32_t                flow_txps;
   uint32_t                dest_flow_id;
   uint32_t                maximum_latency;
   uint32_t                short_term_avg_ratio;
   uint32_t                max_number_retry;
   uint32_t                flow_per;
   uint32_t                in_order_delivery;
   uint32_t                ingr_class_rule;
   uint32_t                traffic_protocol;
   uint32_t                vlan_tag;
   uint32_t                dscp_moca;
   uint32_t                max_short_term_avg_ratio;
   uint32_t                bw_limit_info;
} __attribute__((packed,aligned(4)));

struct moca_pqos_create_flow_in {
   macaddr_t               ingress_node;
   macaddr_t               egress_node;
   macaddr_t               flow_id;
   macaddr_t               packet_da;
   uint32_t                packet_size;
   uint32_t                flow_tag;
   uint32_t                peak_data_rate;
   uint32_t                lease_time;
   uint32_t                burst_size;
   uint32_t                vlan_id;
   uint32_t                max_latency;
   uint32_t                short_term_avg_ratio;
   uint32_t                max_retry;
   uint32_t                flow_per;
   uint32_t                in_order_delivery;
   uint32_t                traffic_protocol;
   uint32_t                ingr_class_rule;
   uint32_t                vlan_tag;
   uint32_t                dscp_moca;
} __attribute__((packed,aligned(4)));

struct moca_pqos_update_flow_out {
   macaddr_t               flowid;
   macaddr_t               flowda;
   uint32_t                response_code;
   uint32_t                decision;
   uint32_t                flow_tag;
   uint32_t                peak_data_rate;
   uint32_t                packet_size;
   uint32_t                burst_size;
   uint32_t                lease_time;
   uint32_t                total_stps;
   uint32_t                total_txps;
   uint32_t                flow_stps;
   uint32_t                flow_txps;
   uint32_t                maximum_latency;
   uint32_t                short_term_avg_ratio;
   uint32_t                max_number_retry;
   uint32_t                flow_per;
   uint32_t                in_order_delivery;
   uint32_t                ingr_class_rule;
   uint32_t                traffic_protocol;
   uint32_t                vlan_tag;
   uint32_t                dscp_moca;
   uint32_t                max_short_term_avg_ratio;
   uint32_t                bw_limit_info;
} __attribute__((packed,aligned(4)));

struct moca_pqos_update_flow_in {
   macaddr_t               flow_id;
   macaddr_t               ingress_mac;
   macaddr_t               egress_mac;
   uint16_t                reserved;
   uint32_t                packet_size;
   uint32_t                flow_tag;
   uint32_t                peak_data_rate;
   uint32_t                lease_time;
   uint32_t                burst_size;
   uint32_t                max_latency;
   uint32_t                short_term_avg_ratio;
   uint32_t                max_retry;
   uint32_t                flow_per;
   uint32_t                in_order_delivery;
   uint32_t                traffic_protocol;
} __attribute__((packed,aligned(4)));

struct moca_pqos_delete_flow_out {
   macaddr_t               flowid;
   uint32_t                response_code;
} __attribute__((packed,aligned(4)));

struct moca_pqos_list_out {
   uint32_t                response_code;
   uint32_t                flow_update_count;
   uint32_t                total_flow_id_count;
   uint32_t                num_ret_flow_ids;
   macaddr_t               flowid[32];
} __attribute__((packed,aligned(4)));

struct moca_pqos_list_in {
   uint32_t                ingr_node_id;
   macaddr_t               ingr_node_mac;
   uint16_t                flow_max_return;
   uint32_t                flow_start_index;
} __attribute__((packed,aligned(4)));

struct moca_pqos_query_out {
   uint32_t                response_code;
   macaddr_t               flow_id;
   macaddr_t               packet_da;
   macaddr_t               ingress_node;
   macaddr_t               egress_node;
   uint32_t                packet_size;
   uint32_t                flow_tag;
   uint32_t                peak_data_rate;
   uint32_t                burst_size;
   uint32_t                lease_time;
   uint32_t                lease_time_left;
   uint32_t                max_latency;
   uint32_t                short_term_avg_ratio;
   uint32_t                ingr_class_rule;
   uint32_t                vlan_tag;
   uint32_t                dscp_moca;
   uint32_t                dest_flow_id;
   uint32_t                max_retry;
   uint32_t                flow_per;
   uint32_t                in_order_delivery;
   uint32_t                traffic_protocol;
} __attribute__((packed,aligned(4)));

struct moca_pqos_status_out {
   uint32_t                total_stps;
   uint32_t                total_txps;
   uint32_t                flow_stps;
   uint32_t                flow_txps;
} __attribute__((packed,aligned(4)));

struct moca_mcfilter_table {
   macaddr_t               addr[48];
} __attribute__((packed,aligned(4)));

struct moca_taboo_channels {
   uint32_t                taboo_fixed_mask_start;
   uint32_t                taboo_fixed_channel_mask;
   uint32_t                taboo_left_mask;
   uint32_t                taboo_right_mask;
} __attribute__((packed,aligned(4)));

struct moca_gen_node_status {
   macaddr_t               eui;
   uint16_t                reserved_0;
   int32_t                 freq_offset;
   uint32_t                node_tx_backoff;
   uint32_t                protocol_support;
   uint32_t                active_moca_version;
   uint32_t                max_ingress_pqos;
   uint32_t                max_egress_pqos;
   uint32_t                ae_number;
   uint32_t                max_aggr_pdus;
   uint32_t                max_aggr_kb;
} __attribute__((packed,aligned(4)));

struct moca_gen_node_ext_status {
   uint32_t                nbas;
   uint32_t                preamble_type;
   uint32_t                cp;
   uint32_t                tx_power;
   int32_t                 rx_power;
   int32_t                 agc_address;
   uint32_t                bit_loading[64];
   uint32_t                avg_snr;
   uint32_t                phy_rate;
   uint32_t                turbo_status;
   int32_t                 tx_backoff;
   int32_t                 rx_backoff;
} __attribute__((packed,aligned(4)));

struct moca_gen_node_ext_status_in {
   uint32_t                index;
   uint32_t                profile_type;
} __attribute__((packed,aligned(4)));

struct moca_node_stats {
   uint32_t                tx_packets;
   uint32_t                rx_packets;
   uint32_t                primary_ch_rx_cw_unerror;
   uint32_t                primary_ch_rx_cw_corrected;
   uint32_t                primary_ch_rx_cw_uncorrected;
   uint32_t                primary_ch_rx_no_sync;
   uint32_t                secondary_ch_rx_cw_unerror;
   uint32_t                secondary_ch_rx_cw_corrected;
   uint32_t                secondary_ch_rx_cw_uncorrected;
   uint32_t                secondary_ch_rx_no_sync;
} __attribute__((packed,aligned(4)));

struct moca_node_stats_in {
   uint32_t                index;
   uint32_t                reset_stats;
} __attribute__((packed,aligned(4)));

struct moca_node_stats_ext {
   uint16_t                rx_uc_crc_error;
   uint16_t                rx_uc_crc_error_sec_ch;
   uint16_t                rx_uc_timeout_error;
   uint16_t                rx_uc_timeout_error_sec_ch;
   uint16_t                rx_bc_crc_error;
   uint16_t                rx_bc_timeout_error;
   uint16_t                rx_map_crc_error;
   uint16_t                rx_map_timeout_error;
   uint16_t                rx_beacon_crc_error;
   uint16_t                rx_beacon_timeout_error;
   uint16_t                rx_rr_crc_error;
   uint16_t                rx_ofdma_rr_crc_error;
   uint16_t                rx_rr_timeout_error;
   uint16_t                rx_lc_uc_crc_error;
   uint16_t                rx_lc_bc_crc_error;
   uint16_t                rx_lc_uc_timeout_error;
   uint16_t                rx_lc_bc_timeout_error;
   uint16_t                rx_probe1_error;
   uint16_t                rx_probe1_error_sec_ch;
   uint16_t                rx_probe2_error;
   uint16_t                rx_probe3_error;
   uint16_t                rx_probe1_gcd_error;
   uint16_t                rx_plp_crc_error;
   uint16_t                rx_plp_timeout_error;
   uint16_t                rx_broken_packet_error;
   uint16_t                rx_broken_packet_error_sec_ch;
   uint16_t                rx_acf_crc_error;
   uint16_t                reserved_0;
} __attribute__((packed,aligned(4)));

struct moca_node_stats_ext_in {
   uint32_t                index;
   uint32_t                reset_stats;
} __attribute__((packed,aligned(4)));

struct moca_network_status {
   uint32_t                network_moca_version;
   uint32_t                connected_nodes;
   uint32_t                node_id;
   uint32_t                nc_node_id;
   uint32_t                backup_nc_id;
   uint32_t                bw_status;
   uint32_t                nodes_usable_bitmask;
   uint32_t                network_taboo_mask;
   uint32_t                network_taboo_start;
   uint32_t                bonded_nodes_bitmask;
} __attribute__((packed,aligned(4)));

struct moca_start_ulmo {
   uint32_t                report_type;
   uint32_t                node_id;
   uint32_t                ofdma_node_mask;
   uint32_t                subcarrier[16];
} __attribute__((packed,aligned(4)));

struct moca_rxd_lmo_request {
   uint32_t                node_id;
   uint32_t                probe_id;
   uint32_t                channel_id;
} __attribute__((packed,aligned(4)));

struct moca_ofdma_definition_table {
   uint32_t                ofdmaDefTabNum;
   uint32_t                subchannelDefId[4];
   uint32_t                node_bitmask[4];
   uint32_t                subchannel_NBAS[4];
} __attribute__((packed,aligned(4)));

struct moca_ofdma_assignment_table {
   uint32_t                ofdmaDefTabNum;
   uint32_t                ofdmaFrameId[4];
   uint32_t                subchannelDefId[4];
   uint32_t                num_Subchannels[4];
   uint32_t                node_bitmask[4];
   uint32_t                cp_length[4];
} __attribute__((packed,aligned(4)));

struct moca_adm_stats {
   uint16_t                started;
   uint16_t                succeeded;
   uint16_t                admission_failed;
   uint16_t                no_response;
   uint16_t                channel_unusable;
   uint16_t                t2_timeout;
   uint16_t                priv_full_blacklist;
   uint16_t                admission_failed_nc;
   uint16_t                resync_loss;
   uint16_t                nc_started_nn;
   uint16_t                nc_succeeded_nn;
   uint16_t                nc_dropped_en;
} __attribute__((packed,aligned(4)));

struct moca_lmo_info {
   uint32_t                lmo_node_id;
   uint32_t                lmo_initial_ls;
   uint32_t                lmo_duration_dsec;
   uint32_t                lmo_anb;
   uint32_t                is_lmo_success;
} __attribute__((packed,aligned(4)));

struct moca_moca_reset_request {
   uint32_t                cause;
   uint32_t                mr_seq_num;
} __attribute__((packed,aligned(4)));

struct moca_aca_out {
   uint32_t                aca_status;
   uint32_t                aca_type;
   uint32_t                tx_status;
   uint32_t                rx_status;
   int32_t                 total_power;
   int32_t                 relative_power;
   uint32_t                num_elements;
   uint8_t                 power_profile[512];
} __attribute__((packed,aligned(4)));

struct moca_aca_in {
   uint32_t                src_node;
   uint32_t                dest_nodemask;
   uint32_t                type;
   uint32_t                channel;
   uint32_t                num_probes;
} __attribute__((packed,aligned(4)));

struct moca_fmr_init_out {
   uint32_t                responsecode;
   uint32_t                responded_node_0;
   uint16_t                fmrinfo_node_0[16];
   uint32_t                responded_node_1;
   uint16_t                fmrinfo_node_1[16];
   uint32_t                responded_node_2;
   uint16_t                fmrinfo_node_2[16];
   uint32_t                responded_node_3;
   uint16_t                fmrinfo_node_3[16];
   uint32_t                responded_node_4;
   uint16_t                fmrinfo_node_4[16];
   uint32_t                responded_node_5;
   uint16_t                fmrinfo_node_5[16];
   uint32_t                responded_node_6;
   uint16_t                fmrinfo_node_6[16];
   uint32_t                responded_node_7;
   uint16_t                fmrinfo_node_7[16];
   uint32_t                responded_node_8;
   uint16_t                fmrinfo_node_8[16];
   uint32_t                responded_node_9;
   uint16_t                fmrinfo_node_9[16];
   uint32_t                responded_node_10;
   uint16_t                fmrinfo_node_10[16];
   uint32_t                responded_node_11;
   uint16_t                fmrinfo_node_11[16];
   uint32_t                responded_node_12;
   uint16_t                fmrinfo_node_12[16];
   uint32_t                responded_node_13;
   uint16_t                fmrinfo_node_13[16];
   uint32_t                responded_node_14;
   uint16_t                fmrinfo_node_14[16];
   uint32_t                responded_node_15;
   uint16_t                fmrinfo_node_15[16];
} __attribute__((packed,aligned(4)));

struct moca_moca_reset_out {
   uint32_t                response_code;
   uint32_t                reset_status;
   uint32_t                non_def_seq_num;
   uint8_t                 n00ResetStatus;
   uint8_t                 n00RspCode;
   uint8_t                 n01ResetStatus;
   uint8_t                 n01RspCode;
   uint8_t                 n02ResetStatus;
   uint8_t                 n02RspCode;
   uint8_t                 n03ResetStatus;
   uint8_t                 n03RspCode;
   uint8_t                 n04ResetStatus;
   uint8_t                 n04RspCode;
   uint8_t                 n05ResetStatus;
   uint8_t                 n05RspCode;
   uint8_t                 n06ResetStatus;
   uint8_t                 n06RspCode;
   uint8_t                 n07ResetStatus;
   uint8_t                 n07RspCode;
   uint8_t                 n08ResetStatus;
   uint8_t                 n08RspCode;
   uint8_t                 n09ResetStatus;
   uint8_t                 n09RspCode;
   uint8_t                 n10ResetStatus;
   uint8_t                 n10RspCode;
   uint8_t                 n11ResetStatus;
   uint8_t                 n11RspCode;
   uint8_t                 n12ResetStatus;
   uint8_t                 n12RspCode;
   uint8_t                 n13ResetStatus;
   uint8_t                 n13RspCode;
   uint8_t                 n14ResetStatus;
   uint8_t                 n14RspCode;
   uint8_t                 n15ResetStatus;
   uint8_t                 n15RspCode;
} __attribute__((packed,aligned(4)));

struct moca_moca_reset_in {
   uint32_t                node_mask;
   uint32_t                reset_timer;
   uint32_t                non_def_seq_num;
} __attribute__((packed,aligned(4)));

struct moca_dd_init_out {
   uint32_t                responsecode;
   uint32_t                responded_nodemask;
   uint32_t                ingress_pqos_flows[16];
   uint32_t                egress_pqos_flows[16];
   uint32_t                aggr_pdus[16];
   uint32_t                aggr_size[16];
   uint32_t                ae_number[16];
   uint32_t                aggr_pdus_bonded[16];
   uint32_t                aggr_size_bonded[16];
} __attribute__((packed,aligned(4)));

struct moca_fmr_20_out {
   uint32_t                responsecode;
   uint8_t                 node0_gap_nper[16];
   uint8_t                 node0_gap_vlper[16];
   uint16_t                node0_ofdmb_nper[16];
   uint16_t                node0_ofdmb_vlper[16];
   uint16_t                node0_ofdmb_gcd;
   uint8_t                 node0_gap_gcd;
   uint8_t                 node0_ofdma_def_tab_num;
   uint32_t                node0_ofdma_tab_node_bitmask[4];
   uint8_t                 node0_ofdma_tab_num_subchan[4];
   uint8_t                 node0_ofdma_tab_gap[4];
   uint16_t                node0_ofdma_tab_bps[4];
   uint8_t                 node1_gap_nper[16];
   uint8_t                 node1_gap_vlper[16];
   uint16_t                node1_ofdmb_nper[16];
   uint16_t                node1_ofdmb_vlper[16];
   uint16_t                node1_ofdmb_gcd;
   uint8_t                 node1_gap_gcd;
   uint8_t                 node1_ofdma_def_tab_num;
   uint32_t                node1_ofdma_tab_node_bitmask[4];
   uint8_t                 node1_ofdma_tab_num_subchan[4];
   uint8_t                 node1_ofdma_tab_gap[4];
   uint16_t                node1_ofdma_tab_bps[4];
   uint8_t                 node2_gap_nper[16];
   uint8_t                 node2_gap_vlper[16];
   uint16_t                node2_ofdmb_nper[16];
   uint16_t                node2_ofdmb_vlper[16];
   uint16_t                node2_ofdmb_gcd;
   uint8_t                 node2_gap_gcd;
   uint8_t                 node2_ofdma_def_tab_num;
   uint32_t                node2_ofdma_tab_node_bitmask[4];
   uint8_t                 node2_ofdma_tab_num_subchan[4];
   uint8_t                 node2_ofdma_tab_gap[4];
   uint16_t                node2_ofdma_tab_bps[4];
   uint8_t                 node3_gap_nper[16];
   uint8_t                 node3_gap_vlper[16];
   uint16_t                node3_ofdmb_nper[16];
   uint16_t                node3_ofdmb_vlper[16];
   uint16_t                node3_ofdmb_gcd;
   uint8_t                 node3_gap_gcd;
   uint8_t                 node3_ofdma_def_tab_num;
   uint32_t                node3_ofdma_tab_node_bitmask[4];
   uint8_t                 node3_ofdma_tab_num_subchan[4];
   uint8_t                 node3_ofdma_tab_gap[4];
   uint16_t                node3_ofdma_tab_bps[4];
   uint8_t                 node4_gap_nper[16];
   uint8_t                 node4_gap_vlper[16];
   uint16_t                node4_ofdmb_nper[16];
   uint16_t                node4_ofdmb_vlper[16];
   uint16_t                node4_ofdmb_gcd;
   uint8_t                 node4_gap_gcd;
   uint8_t                 node4_ofdma_def_tab_num;
   uint32_t                node4_ofdma_tab_node_bitmask[4];
   uint8_t                 node4_ofdma_tab_num_subchan[4];
   uint8_t                 node4_ofdma_tab_gap[4];
   uint16_t                node4_ofdma_tab_bps[4];
   uint8_t                 node5_gap_nper[16];
   uint8_t                 node5_gap_vlper[16];
   uint16_t                node5_ofdmb_nper[16];
   uint16_t                node5_ofdmb_vlper[16];
   uint16_t                node5_ofdmb_gcd;
   uint8_t                 node5_gap_gcd;
   uint8_t                 node5_ofdma_def_tab_num;
   uint32_t                node5_ofdma_tab_node_bitmask[4];
   uint8_t                 node5_ofdma_tab_num_subchan[4];
   uint8_t                 node5_ofdma_tab_gap[4];
   uint16_t                node5_ofdma_tab_bps[4];
   uint8_t                 node6_gap_nper[16];
   uint8_t                 node6_gap_vlper[16];
   uint16_t                node6_ofdmb_nper[16];
   uint16_t                node6_ofdmb_vlper[16];
   uint16_t                node6_ofdmb_gcd;
   uint8_t                 node6_gap_gcd;
   uint8_t                 node6_ofdma_def_tab_num;
   uint32_t                node6_ofdma_tab_node_bitmask[4];
   uint8_t                 node6_ofdma_tab_num_subchan[4];
   uint8_t                 node6_ofdma_tab_gap[4];
   uint16_t                node6_ofdma_tab_bps[4];
   uint8_t                 node7_gap_nper[16];
   uint8_t                 node7_gap_vlper[16];
   uint16_t                node7_ofdmb_nper[16];
   uint16_t                node7_ofdmb_vlper[16];
   uint16_t                node7_ofdmb_gcd;
   uint8_t                 node7_gap_gcd;
   uint8_t                 node7_ofdma_def_tab_num;
   uint32_t                node7_ofdma_tab_node_bitmask[4];
   uint8_t                 node7_ofdma_tab_num_subchan[4];
   uint8_t                 node7_ofdma_tab_gap[4];
   uint16_t                node7_ofdma_tab_bps[4];
   uint8_t                 node8_gap_nper[16];
   uint8_t                 node8_gap_vlper[16];
   uint16_t                node8_ofdmb_nper[16];
   uint16_t                node8_ofdmb_vlper[16];
   uint16_t                node8_ofdmb_gcd;
   uint8_t                 node8_gap_gcd;
   uint8_t                 node8_ofdma_def_tab_num;
   uint32_t                node8_ofdma_tab_node_bitmask[4];
   uint8_t                 node8_ofdma_tab_num_subchan[4];
   uint8_t                 node8_ofdma_tab_gap[4];
   uint16_t                node8_ofdma_tab_bps[4];
   uint8_t                 node9_gap_nper[16];
   uint8_t                 node9_gap_vlper[16];
   uint16_t                node9_ofdmb_nper[16];
   uint16_t                node9_ofdmb_vlper[16];
   uint16_t                node9_ofdmb_gcd;
   uint8_t                 node9_gap_gcd;
   uint8_t                 node9_ofdma_def_tab_num;
   uint32_t                node9_ofdma_tab_node_bitmask[4];
   uint8_t                 node9_ofdma_tab_num_subchan[4];
   uint8_t                 node9_ofdma_tab_gap[4];
   uint16_t                node9_ofdma_tab_bps[4];
   uint8_t                 node10_gap_nper[16];
   uint8_t                 node10_gap_vlper[16];
   uint16_t                node10_ofdmb_nper[16];
   uint16_t                node10_ofdmb_vlper[16];
   uint16_t                node10_ofdmb_gcd;
   uint8_t                 node10_gap_gcd;
   uint8_t                 node10_ofdma_def_tab_num;
   uint32_t                node10_ofdma_tab_node_bitmask[4];
   uint8_t                 node10_ofdma_tab_num_subchan[4];
   uint8_t                 node10_ofdma_tab_gap[4];
   uint16_t                node10_ofdma_tab_bps[4];
   uint8_t                 node11_gap_nper[16];
   uint8_t                 node11_gap_vlper[16];
   uint16_t                node11_ofdmb_nper[16];
   uint16_t                node11_ofdmb_vlper[16];
   uint16_t                node11_ofdmb_gcd;
   uint8_t                 node11_gap_gcd;
   uint8_t                 node11_ofdma_def_tab_num;
   uint32_t                node11_ofdma_tab_node_bitmask[4];
   uint8_t                 node11_ofdma_tab_num_subchan[4];
   uint8_t                 node11_ofdma_tab_gap[4];
   uint16_t                node11_ofdma_tab_bps[4];
   uint8_t                 node12_gap_nper[16];
   uint8_t                 node12_gap_vlper[16];
   uint16_t                node12_ofdmb_nper[16];
   uint16_t                node12_ofdmb_vlper[16];
   uint16_t                node12_ofdmb_gcd;
   uint8_t                 node12_gap_gcd;
   uint8_t                 node12_ofdma_def_tab_num;
   uint32_t                node12_ofdma_tab_node_bitmask[4];
   uint8_t                 node12_ofdma_tab_num_subchan[4];
   uint8_t                 node12_ofdma_tab_gap[4];
   uint16_t                node12_ofdma_tab_bps[4];
   uint8_t                 node13_gap_nper[16];
   uint8_t                 node13_gap_vlper[16];
   uint16_t                node13_ofdmb_nper[16];
   uint16_t                node13_ofdmb_vlper[16];
   uint16_t                node13_ofdmb_gcd;
   uint8_t                 node13_gap_gcd;
   uint8_t                 node13_ofdma_def_tab_num;
   uint32_t                node13_ofdma_tab_node_bitmask[4];
   uint8_t                 node13_ofdma_tab_num_subchan[4];
   uint8_t                 node13_ofdma_tab_gap[4];
   uint16_t                node13_ofdma_tab_bps[4];
   uint8_t                 node14_gap_nper[16];
   uint8_t                 node14_gap_vlper[16];
   uint16_t                node14_ofdmb_nper[16];
   uint16_t                node14_ofdmb_vlper[16];
   uint16_t                node14_ofdmb_gcd;
   uint8_t                 node14_gap_gcd;
   uint8_t                 node14_ofdma_def_tab_num;
   uint32_t                node14_ofdma_tab_node_bitmask[4];
   uint8_t                 node14_ofdma_tab_num_subchan[4];
   uint8_t                 node14_ofdma_tab_gap[4];
   uint16_t                node14_ofdma_tab_bps[4];
   uint8_t                 node15_gap_nper[16];
   uint8_t                 node15_gap_vlper[16];
   uint16_t                node15_ofdmb_nper[16];
   uint16_t                node15_ofdmb_vlper[16];
   uint16_t                node15_ofdmb_gcd;
   uint8_t                 node15_gap_gcd;
   uint8_t                 node15_ofdma_def_tab_num;
   uint32_t                node15_ofdma_tab_node_bitmask[4];
   uint8_t                 node15_ofdma_tab_num_subchan[4];
   uint8_t                 node15_ofdma_tab_gap[4];
   uint16_t                node15_ofdma_tab_bps[4];
} __attribute__((packed,aligned(4)));

struct moca_error_stats {
   uint32_t                rx_uc_crc_error;
   uint32_t                rx_uc_crc_error_sec_ch;
   uint32_t                rx_uc_timeout_error;
   uint32_t                rx_uc_timeout_error_sec_ch;
   uint32_t                rx_bc_crc_error;
   uint32_t                rx_bc_timeout_error;
   uint32_t                rx_map_crc_error;
   uint32_t                rx_map_timeout_error;
   uint32_t                rx_beacon_crc_error;
   uint32_t                rx_beacon_timeout_error;
   uint32_t                rx_rr_crc_error;
   uint32_t                rx_ofdma_rr_crc_error;
   uint32_t                rx_rr_timeout_error;
   uint32_t                rx_lc_uc_crc_error;
   uint32_t                rx_lc_bc_crc_error;
   uint32_t                rx_lc_uc_timeout_error;
   uint32_t                rx_lc_bc_timeout_error;
   uint32_t                rx_probe1_error;
   uint32_t                rx_probe1_error_sec_ch;
   uint32_t                rx_probe2_error;
   uint32_t                rx_probe3_error;
   uint32_t                rx_probe1_gcd_error;
   uint32_t                rx_plp_crc_error;
   uint32_t                rx_plp_timeout_error;
   uint32_t                rx_acf_crc_error;
} __attribute__((packed,aligned(4)));

struct moca_last_mr_events {
   int32_t                 last_cause;
   uint32_t                last_seq_num;
   int32_t                 last_mr_result;
} __attribute__((packed,aligned(4)));

struct moca_gen_stats {
   uint32_t                ecl_tx_total_pkts;
   uint32_t                ecl_tx_ucast_pkts;
   uint32_t                ecl_tx_bcast_pkts;
   uint32_t                ecl_tx_mcast_pkts;
   uint32_t                ecl_tx_ucast_unknown;
   uint32_t                ecl_tx_mcast_unknown;
   uint32_t                ecl_tx_ucast_drops;
   uint32_t                ecl_tx_mcast_drops;
   uint64_t                ecl_tx_total_bytes;
   uint32_t                ecl_tx_buff_drop_pkts;
   uint32_t                ecl_tx_error_drop_pkts;
   uint32_t                ecl_rx_total_pkts;
   uint32_t                ecl_rx_ucast_pkts;
   uint32_t                ecl_rx_bcast_pkts;
   uint32_t                ecl_rx_mcast_pkts;
   uint32_t                ecl_rx_ucast_drops;
   uint32_t                ecl_rx_mcast_filter_pkts;
   uint64_t                ecl_rx_total_bytes;
   uint32_t                ecl_fc_bg;
   uint32_t                ecl_fc_low;
   uint32_t                ecl_fc_medium;
   uint32_t                ecl_fc_high;
   uint32_t                ecl_fc_pqos;
   uint32_t                ecl_fc_bp_all;
   uint32_t                mac_tx_low_drop_pkts;
   uint32_t                mac_rx_buff_drop_pkts;
   uint32_t                mac_channel_usable_drop;
   uint32_t                mac_remove_node_drop;
   uint32_t                mac_loopback_pkts;
   uint32_t                mac_loopback_drop_pkts;
   uint32_t                aggr_pkt_stats_rx_max;
   uint32_t                aggr_pkt_stats_rx_count;
   uint32_t                aggr_pkt_stats_tx[30];
   uint32_t                link_down_count;
   uint32_t                link_up_count;
   uint32_t                nc_handoff_counter;
   uint32_t                nc_backup_counter;
   uint32_t                resync_attempts_to_network;
   uint32_t                tx_beacons;
   uint32_t                tx_map_packets;
   uint32_t                tx_rr_packets;
   uint32_t                tx_ofdma_rr_packets;
   uint32_t                tx_control_uc_packets;
   uint32_t                tx_control_bc_packets;
   uint32_t                tx_protocol_ie;
   uint32_t                rx_beacons;
   uint32_t                rx_map_packets;
   uint32_t                rx_rr_packets;
   uint32_t                rx_ofdma_rr_packets;
   uint32_t                rx_control_uc_packets;
   uint32_t                rx_control_bc_packets;
   uint32_t                rx_protocol_ie;
   uint32_t                mac_frag_mpdu_tx;
   uint32_t                mac_frag_mpdu_rx;
   uint32_t                mac_pqos_policing_tx;
   uint32_t                mac_pqos_policing_drop;
   uint32_t                nc_became_nc_counter;
   uint32_t                nc_became_backup_nc_counter;
   uint32_t                rx_buffer_full_counter;
} __attribute__((packed,aligned(4)));

struct moca_interface_status {
   uint32_t                link_status;
   uint32_t                rf_channel;
   uint32_t                primary_channel;
   uint32_t                secondary_channel;
} __attribute__((packed,aligned(4)));

struct moca_if_access_table {
   macaddr_t               mac_addr[16];
} __attribute__((packed,aligned(4)));

struct moca_ext_octet_count {
   uint32_t                in_octets_hi;
   uint32_t                in_octets_lo;
   uint32_t                out_octets_hi;
   uint32_t                out_octets_lo;
} __attribute__((packed,aligned(4)));

struct moca_node_power_state {
   uint32_t                state;
   uint32_t                pwr;
} __attribute__((packed,aligned(4)));

struct moca_wom_pattern {
   uint8_t                 bytes[16];
   uint8_t                 mask[16];
} __attribute__((packed,aligned(4)));

struct moca_wom_pattern_set {
   uint32_t                index;
   uint8_t                 bytes[16];
   uint8_t                 mask[16];
} __attribute__((packed,aligned(4)));

struct moca_wom_ip {
   uint32_t                index;
   uint32_t                ipaddr;
} __attribute__((packed,aligned(4)));

struct moca_wom_magic_mac {
   macaddr_t               val;
} __attribute__((packed,aligned(4)));

struct moca_mmk_key {
   uint32_t                mmk_key_hi;
   uint32_t                mmk_key_lo;
} __attribute__((packed,aligned(4)));

struct moca_pmk_initial_key {
   uint32_t                pmk_initial_key_hi;
   uint32_t                pmk_initial_key_lo;
} __attribute__((packed,aligned(4)));

struct moca_aes_mm_key {
   uint32_t                val[4];
} __attribute__((packed,aligned(4)));

struct moca_aes_pm_key {
   uint32_t                val[4];
} __attribute__((packed,aligned(4)));

struct moca_current_keys {
   uint32_t                pmk_even_key[2];
   uint32_t                pmk_odd_key[2];
   uint32_t                tek_even_key[2];
   uint32_t                tek_odd_key[2];
   uint32_t                aes_pmk_even_key[4];
   uint32_t                aes_pmk_odd_key[4];
   uint32_t                aes_tek_even_key[4];
   uint32_t                aes_tek_odd_key[4];
} __attribute__((packed,aligned(4)));

struct moca_permanent_salt {
   uint32_t                aes_salt[3];
} __attribute__((packed,aligned(4)));

struct moca_aes_pmk_initial_key {
   uint32_t                val[4];
} __attribute__((packed,aligned(4)));

struct moca_key_changed {
   uint32_t                key_type;
   uint32_t                even_odd;
} __attribute__((packed,aligned(4)));

struct moca_key_times {
   uint32_t                tek_time;
   uint32_t                tek_last_interval;
   uint32_t                tek_even_odd;
   uint32_t                pmk_time;
   uint32_t                pmk_last_interval;
   uint32_t                pmk_even_odd;
   uint32_t                aes_tek_time;
   uint32_t                aes_tek_last_interval;
   uint32_t                aes_tek_even_odd;
   uint32_t                aes_pmk_time;
   uint32_t                aes_pmk_last_interval;
   uint32_t                aes_pmk_even_odd;
} __attribute__((packed,aligned(4)));

struct moca_password {
   char                    password[32];
} __attribute__((packed,aligned(4)));

struct moca_const_tx_params {
   uint32_t                const_tx_submode;
   uint32_t                const_tx_sc1;
   uint32_t                const_tx_sc2;
   uint32_t                const_tx_band[16];
} __attribute__((packed,aligned(4)));

struct moca_gmii_trap_header {
   uint8_t                 dest_mac[6];
   uint8_t                 source_mac[6];
   uint8_t                 dscp_ecn;
   uint16_t                id;
   uint8_t                 ttl;
   uint8_t                 prot;
   uint16_t                ip_checksum;
   uint8_t                 src_ip_addr[4];
   uint8_t                 dst_ip_addr[4];
   uint16_t                src_port;
   uint16_t                dst_port;
} __attribute__((packed,aligned(4)));

struct moca_error {
   uint32_t                string_id;
   uint32_t                num_params;
   uint32_t                err_id;
} __attribute__((packed,aligned(4)));

struct moca_error_lookup {
   uint32_t                string_id;
   uint32_t                num_params;
   uint32_t                err_id;
} __attribute__((packed,aligned(4)));

struct moca_error_to_mask {
   int32_t                 error1;
   int32_t                 error2;
   int32_t                 error3;
} __attribute__((packed,aligned(4)));

struct moca_fw_file {
   uint8_t                 fw_file[128];
} __attribute__((packed,aligned(4)));

struct moca_mocad_printf_out {
   int8_t                  msg[240];
} __attribute__((packed,aligned(4)));

struct moca_lab_pilots {
   uint8_t                 pilots[8];
} __attribute__((packed,aligned(4)));

struct moca_lab_iq_diagram_set {
   uint32_t                nodeid;
   uint32_t                bursttype;
   uint32_t                acmtsymnum;
} __attribute__((packed,aligned(4)));

struct moca_lab_register {
   uint32_t                address;
   uint32_t                len;
   uint32_t                value[48];
} __attribute__((packed,aligned(4)));

struct moca_lab_tpcap {
   uint32_t                enable;
   uint32_t                type;
   uint32_t                type_2;
   uint32_t                stopBurstType;
} __attribute__((packed,aligned(4)));

struct moca_host_pool {
   uint32_t                isr_start;
   uint32_t                isr_end;
   uint32_t                isr_head;
   uint32_t                isr_head_msg;
   uint32_t                isr_head_len;
   uint32_t                isr_tail;
   uint32_t                isr_tail_msg;
   uint32_t                isr_tail_len;
   uint32_t                task_start;
   uint32_t                task_end;
   uint32_t                task_head;
   uint32_t                task_head_msg;
   uint32_t                task_head_len;
   uint32_t                task_tail;
   uint32_t                task_tail_msg;
   uint32_t                task_tail_len;
} __attribute__((packed,aligned(4)));

struct moca_force_handoff {
   uint32_t                nextNc;
   uint32_t                nextBackup;
} __attribute__((packed,aligned(4)));

struct moca_assert {
   uint32_t                error_code;
   uint32_t                return_address;
   uint32_t                cpu_id;
   uint32_t                flags;
   uint32_t                dma_desc_start_addr;
   uint32_t                dma_desc_end_addr;
   uint32_t                llm_queue_table_size;
   uint32_t                llm_queue_table_addr;
   uint32_t                rtt_buffer_ptr_0;
   uint32_t                rtt_buffer_ptr_1;
   uint32_t                string_id;
   uint32_t                num_params;
   uint32_t                macClock;
   uint32_t                args[40];
} __attribute__((packed,aligned(4)));

struct moca_snr_data {
   uint32_t                data[2048];
} __attribute__((packed,aligned(4)));

struct moca_iq_data {
   uint8_t                 data[1024];
} __attribute__((packed,aligned(4)));

struct moca_mps_init_scan_payload {
   uint32_t                channel;
   uint32_t                nc_moca_version;
   uint32_t                mps_code;
   uint32_t                mps_parameters;
   char                    network_name[16];
} __attribute__((packed,aligned(4)));

struct moca_mps_request_mpskey {
   uint32_t                is_nn;
   uint8_t                 public_key[32];
   uint8_t                 nn_guid[8];
} __attribute__((packed,aligned(4)));

MOCALIB_GEN_SWAP_FUNCTION void moca_max_tx_power_tune_swap(struct moca_max_tx_power_tune * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_max_tx_power_tune_sec_ch_swap(struct moca_max_tx_power_tune_sec_ch * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_rx_power_tune_swap(struct moca_rx_power_tune * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_core_ready_swap(struct moca_core_ready * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_drv_info_swap(struct moca_drv_info * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_tx_power_params_swap(struct moca_tx_power_params * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_tx_power_params_in_swap(struct moca_tx_power_params_in * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_snr_margin_rs_swap(struct moca_snr_margin_rs * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_snr_margin_ldpc_swap(struct moca_snr_margin_ldpc * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_snr_margin_ldpc_sec_ch_swap(struct moca_snr_margin_ldpc_sec_ch * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_snr_margin_ldpc_pre5_swap(struct moca_snr_margin_ldpc_pre5 * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_snr_margin_ofdma_swap(struct moca_snr_margin_ofdma * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_snr_margin_ldpc_pri_ch_swap(struct moca_snr_margin_ldpc_pri_ch * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_snr_margin_pre5_pri_ch_swap(struct moca_snr_margin_pre5_pri_ch * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_snr_margin_pre5_sec_ch_swap(struct moca_snr_margin_pre5_sec_ch * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_egr_mc_addr_filter_swap(struct moca_egr_mc_addr_filter * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_egr_mc_addr_filter_set_swap(struct moca_egr_mc_addr_filter_set * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_uc_fwd_swap(struct moca_uc_fwd * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_mc_fwd_swap(struct moca_mc_fwd * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_src_addr_swap(struct moca_src_addr * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_mac_aging_swap(struct moca_mac_aging * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_pqos_create_flow_out_swap(struct moca_pqos_create_flow_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_pqos_create_flow_in_swap(struct moca_pqos_create_flow_in * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_pqos_update_flow_out_swap(struct moca_pqos_update_flow_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_pqos_update_flow_in_swap(struct moca_pqos_update_flow_in * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_pqos_delete_flow_out_swap(struct moca_pqos_delete_flow_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_pqos_list_out_swap(struct moca_pqos_list_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_pqos_list_in_swap(struct moca_pqos_list_in * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_pqos_query_out_swap(struct moca_pqos_query_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_gen_node_status_swap(struct moca_gen_node_status * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_node_stats_ext_swap(struct moca_node_stats_ext * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_node_stats_ext_in_swap(struct moca_node_stats_ext_in * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_adm_stats_swap(struct moca_adm_stats * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_aca_out_swap(struct moca_aca_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_aca_in_swap(struct moca_aca_in * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_fmr_init_out_swap(struct moca_fmr_init_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_moca_reset_out_swap(struct moca_moca_reset_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_moca_reset_in_swap(struct moca_moca_reset_in * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_fmr_20_out_swap(struct moca_fmr_20_out * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_gen_stats_swap(struct moca_gen_stats * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_wom_pattern_set_swap(struct moca_wom_pattern_set * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_gmii_trap_header_swap(struct moca_gmii_trap_header * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_mps_init_scan_payload_swap(struct moca_mps_init_scan_payload * x);
MOCALIB_GEN_SWAP_FUNCTION void moca_mps_request_mpskey_swap(struct moca_mps_request_mpskey * x);

MOCALIB_GEN_SET_FUNCTION void moca_set_preferred_nc_defaults(uint32_t *preferred_nc, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_single_channel_operation_defaults(uint32_t *single_channel_operation, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_continuous_power_tx_mode_defaults(uint32_t *continuous_power_tx_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_continuous_rx_mode_attn_defaults(int32_t *continuous_rx_mode_attn, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_lof_defaults(uint32_t *lof, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_no_ifg6_defaults(uint32_t *no_ifg6, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_bonding_defaults(uint32_t *bonding, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_listening_freq_mask_defaults(uint32_t *listening_freq_mask, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_listening_duration_defaults(uint32_t *listening_duration, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_limit_traffic_defaults(uint32_t *limit_traffic, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_remote_man_defaults(uint32_t *remote_man, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_c4_moca20_en_defaults(uint32_t *c4_moca20_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_power_save_mechanism_dis_defaults(uint32_t *power_save_mechanism_dis, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_psm_config_defaults(uint32_t *psm_config, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_use_ext_data_mem_defaults(uint32_t *use_ext_data_mem, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_aif_mode_defaults(uint32_t *aif_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_prop_bonding_compatibility_mode_defaults(uint32_t *prop_bonding_compatibility_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rdeg_3450_defaults(uint32_t *rdeg_3450, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_phy_clock_defaults(uint32_t *phy_clock, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mac_addr_defaults(struct moca_mac_addr * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_beacon_channel_set_defaults(uint32_t *beacon_channel_set, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_tx_power_tune_defaults(struct moca_max_tx_power_tune * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_tx_power_tune_sec_ch_defaults(struct moca_max_tx_power_tune_sec_ch * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rx_power_tune_defaults(struct moca_rx_power_tune * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_impedance_mode_bonding_defaults(uint32_t *impedance_mode_bonding, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rework_6802_defaults(uint32_t *rework_6802, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_miscval_defaults(uint32_t *miscval, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_en_capable_defaults(uint32_t *en_capable, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_lof_update_defaults(uint32_t *lof_update, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_primary_ch_offset_defaults(int32_t *primary_ch_offset, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_assertText_defaults(uint32_t *assertText, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_wdog_enable_defaults(uint32_t *wdog_enable, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_miscval2_defaults(uint32_t *miscval2, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_secondary_ch_offset_defaults(int32_t *secondary_ch_offset, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_amp_type_defaults(uint32_t *amp_type, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_tpc_en_defaults(uint32_t *tpc_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_tx_power_defaults(int32_t *max_tx_power, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_beacon_pwr_reduction_defaults(uint32_t *beacon_pwr_reduction, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_beacon_pwr_reduction_en_defaults(uint32_t *beacon_pwr_reduction_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_bo_mode_defaults(uint32_t *bo_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_qam256_capability_defaults(uint32_t *qam256_capability, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_otf_en_defaults(uint32_t *otf_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_star_topology_en_defaults(uint32_t *star_topology_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_ofdma_en_defaults(uint32_t *ofdma_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_min_bw_alarm_threshold_defaults(uint32_t *min_bw_alarm_threshold, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_en_max_rate_in_max_bo_defaults(uint32_t *en_max_rate_in_max_bo, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_target_phy_rate_qam128_defaults(uint32_t *target_phy_rate_qam128, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_target_phy_rate_qam256_defaults(uint32_t *target_phy_rate_qam256, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_sapm_en_defaults(uint32_t *sapm_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_arpl_th_50_defaults(int32_t *arpl_th_50, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rlapm_en_defaults(uint32_t *rlapm_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_freq_shift_defaults(uint32_t *freq_shift, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_phy_rate_defaults(uint32_t *max_phy_rate, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_bandwidth_defaults(uint32_t *bandwidth, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_arpl_th_100_defaults(int32_t *arpl_th_100, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_adc_mode_defaults(uint32_t *adc_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_phy_rate_turbo_defaults(uint32_t *max_phy_rate_turbo, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_phy_rate_50M_defaults(uint32_t *max_phy_rate_50M, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_constellation_all_defaults(uint32_t *max_constellation_all, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_constellation_defaults(struct moca_max_constellation * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rlapm_table_50_defaults(struct moca_rlapm_table_50 * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rlapm_table_100_defaults(struct moca_rlapm_table_100 * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_nv_cal_enable_defaults(uint32_t *nv_cal_enable, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rlapm_cap_50_defaults(uint32_t *rlapm_cap_50, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_margin_rs_defaults(struct moca_snr_margin_rs * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_margin_ldpc_defaults(struct moca_snr_margin_ldpc * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_margin_ldpc_sec_ch_defaults(struct moca_snr_margin_ldpc_sec_ch * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_margin_ldpc_pre5_defaults(struct moca_snr_margin_ldpc_pre5 * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_margin_ofdma_defaults(struct moca_snr_margin_ofdma * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rlapm_cap_100_defaults(uint32_t *rlapm_cap_100, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_sapm_table_50_defaults(struct moca_sapm_table_50 * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_sapm_table_100_defaults(struct moca_sapm_table_100 * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_sapm_table_sec_defaults(struct moca_sapm_table_sec * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_margin_ldpc_pri_ch_defaults(struct moca_snr_margin_ldpc_pri_ch * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_margin_pre5_pri_ch_defaults(struct moca_snr_margin_pre5_pri_ch * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_margin_pre5_sec_ch_defaults(struct moca_snr_margin_pre5_sec_ch * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_frame_size_defaults(uint32_t *max_frame_size, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_min_aggr_waiting_time_defaults(uint32_t *min_aggr_waiting_time, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_selective_rr_defaults(uint32_t *selective_rr, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_transmit_time_defaults(uint32_t *max_transmit_time, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_pkt_aggr_defaults(uint32_t *max_pkt_aggr, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rtr_config_defaults(struct moca_rtr_config * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_tlp_mode_defaults(uint32_t *tlp_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_max_pkt_aggr_bonding_defaults(uint32_t *max_pkt_aggr_bonding, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_multicast_mode_defaults(uint32_t *multicast_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_egr_mc_filter_en_defaults(uint32_t *egr_mc_filter_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_fc_mode_defaults(uint32_t *fc_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_pqos_max_packet_size_defaults(uint32_t *pqos_max_packet_size, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_per_mode_defaults(uint32_t *per_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_policing_en_defaults(uint32_t *policing_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_orr_en_defaults(uint32_t *orr_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_brcmtag_enable_defaults(uint32_t *brcmtag_enable, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_unknown_ratelimit_en_defaults(uint32_t *unknown_ratelimit_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_egr_mc_addr_filter_defaults(struct moca_egr_mc_addr_filter * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mac_aging_defaults(struct moca_mac_aging * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_loopback_en_defaults(uint32_t *loopback_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mcfilter_enable_defaults(uint32_t *mcfilter_enable, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_pause_fc_en_defaults(uint32_t *pause_fc_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_stag_priority_defaults(struct moca_stag_priority * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_stag_removal_defaults(struct moca_stag_removal * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_host_qos_defaults(uint32_t *host_qos, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_taboo_channels_defaults(struct moca_taboo_channels * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_ooo_lmo_defaults(uint32_t *ooo_lmo, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_start_ulmo_defaults(struct moca_start_ulmo * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rf_band_defaults(uint32_t *rf_band, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_if_access_en_defaults(uint32_t *if_access_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_led_mode_defaults(uint32_t *led_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_if_access_table_defaults(struct moca_if_access_table * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_m1_tx_power_variation_defaults(uint32_t *m1_tx_power_variation, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_nc_listening_interval_defaults(uint32_t *nc_listening_interval, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_nc_heartbeat_interval_defaults(uint32_t *nc_heartbeat_interval, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_wom_magic_enable_defaults(uint32_t *wom_magic_enable, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_pm_restore_on_link_down_defaults(uint32_t *pm_restore_on_link_down, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_standby_power_state_defaults(uint32_t *standby_power_state, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_wom_mode_defaults(uint32_t *wom_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_privacy_en_defaults(uint32_t *privacy_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_pmk_exchange_interval_defaults(uint32_t *pmk_exchange_interval, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_tek_exchange_interval_defaults(uint32_t *tek_exchange_interval, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_aes_exchange_interval_defaults(uint32_t *aes_exchange_interval, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_aes_pm_key_defaults(struct moca_aes_pm_key * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_password_defaults(struct moca_password * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mtm_en_defaults(uint32_t *mtm_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_cir_prints_defaults(uint32_t *cir_prints, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_snr_prints_defaults(uint32_t *snr_prints, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_sigma2_prints_defaults(uint32_t *sigma2_prints, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_bad_probe_prints_defaults(uint32_t *bad_probe_prints, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_const_tx_params_defaults(struct moca_const_tx_params * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_moca_core_trace_enable_defaults(uint32_t *moca_core_trace_enable, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_dont_start_moca_defaults(uint32_t *dont_start_moca, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_lab_mode_defaults(uint32_t *lab_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_nc_mode_defaults(uint32_t *nc_mode, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_rx_tx_packets_per_qm_defaults(uint32_t *rx_tx_packets_per_qm, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_extra_rx_packets_per_qm_defaults(uint32_t *extra_rx_packets_per_qm, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_target_phy_rate_20_defaults(uint32_t *target_phy_rate_20, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_target_phy_rate_20_turbo_defaults(uint32_t *target_phy_rate_20_turbo, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_turbo_en_defaults(uint32_t *turbo_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_target_phy_rate_20_turbo_vlper_defaults(uint32_t *target_phy_rate_20_turbo_vlper, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_target_phy_rate_20_sec_ch_defaults(uint32_t *target_phy_rate_20_sec_ch, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_target_phy_rate_20_turbo_sec_ch_defaults(uint32_t *target_phy_rate_20_turbo_sec_ch, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_target_phy_rate_20_turbo_vlper_sec_ch_defaults(uint32_t *target_phy_rate_20_turbo_vlper_sec_ch, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_cap_phy_rate_en_defaults(uint32_t *cap_phy_rate_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_cap_target_phy_rate_defaults(uint32_t *cap_target_phy_rate, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_cap_snr_base_margin_defaults(uint32_t *cap_snr_base_margin, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_lab_iq_diagram_set_defaults(struct moca_lab_iq_diagram_set * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_lab_tpcap_defaults(struct moca_lab_tpcap * in, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mps_en_defaults(uint32_t *mps_en, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mps_privacy_receive_defaults(uint32_t *mps_privacy_receive, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mps_privacy_down_defaults(uint32_t *mps_privacy_down, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mps_walk_time_defaults(uint32_t *mps_walk_time, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_mps_unpaired_time_defaults(uint32_t *mps_unpaired_time, uint32_t flags);
MOCALIB_GEN_SET_FUNCTION void moca_set_privacy_defaults_defaults(uint32_t *privacy_defaults, uint32_t flags);

MOCALIB_GEN_RANGE_FUNCTION int moca_continuous_rx_mode_attn_check(int32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_no_ifg6_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_bonding_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_listening_duration_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_limit_traffic_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_remote_man_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_c4_moca20_en_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_power_save_mechanism_dis_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_psm_config_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_use_ext_data_mem_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_aif_mode_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_prop_bonding_compatibility_mode_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_rdeg_3450_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_phy_clock_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_tx_power_tune_check(struct moca_max_tx_power_tune * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_tx_power_tune_sec_ch_check(struct moca_max_tx_power_tune_sec_ch * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_rx_power_tune_check(struct moca_rx_power_tune * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_impedance_mode_bonding_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_rework_6802_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_lof_update_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_primary_ch_offset_check(int32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_mr_seq_num_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_secondary_ch_offset_check(int32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_tx_power_check(int32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_beacon_pwr_reduction_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_beacon_pwr_reduction_en_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_star_topology_en_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_ofdma_en_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_min_bw_alarm_threshold_check(uint32_t mbps, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_target_phy_rate_qam128_check(uint32_t mbps, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_target_phy_rate_qam256_check(uint32_t mbps, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_arpl_th_50_check(int32_t arpl, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_bandwidth_check(uint32_t bandwidth, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_arpl_th_100_check(int32_t arpl, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_constellation_all_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_constellation_check(struct moca_max_constellation * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_rlapm_table_50_check(struct moca_rlapm_table_50 * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_rlapm_table_100_check(struct moca_rlapm_table_100 * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_nv_cal_enable_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_snr_margin_rs_check(struct moca_snr_margin_rs * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_snr_margin_ldpc_check(struct moca_snr_margin_ldpc * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_snr_margin_ldpc_sec_ch_check(struct moca_snr_margin_ldpc_sec_ch * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_snr_margin_ldpc_pre5_check(struct moca_snr_margin_ldpc_pre5 * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_snr_margin_ofdma_check(struct moca_snr_margin_ofdma * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_sapm_table_50_check(uint8_t * val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_sapm_table_100_check(uint8_t * val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_sapm_table_sec_check(uint8_t * val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_snr_margin_ldpc_pri_ch_check(struct moca_snr_margin_ldpc_pri_ch * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_snr_margin_pre5_pri_ch_check(struct moca_snr_margin_pre5_pri_ch * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_snr_margin_pre5_sec_ch_check(struct moca_snr_margin_pre5_sec_ch * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_frame_size_check(uint32_t bytes, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_transmit_time_check(uint32_t usec, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_pkt_aggr_check(uint32_t pkts, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_rtr_config_check(struct moca_rtr_config * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_tlp_mode_check(uint32_t mode, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_max_pkt_aggr_bonding_check(uint32_t pkts, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_fc_mode_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_pqos_max_packet_size_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_per_mode_check(uint32_t mode, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_policing_en_check(uint32_t enable, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_orr_en_check(uint32_t enable, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_brcmtag_enable_check(uint32_t enable, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_unknown_ratelimit_en_check(uint32_t enable, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_egr_mc_addr_filter_check(struct moca_egr_mc_addr_filter_set * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_pause_fc_en_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_stag_priority_check(struct moca_stag_priority * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_stag_removal_check(struct moca_stag_removal * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_pqos_create_flow_check(struct moca_pqos_create_flow_in * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_pqos_update_flow_check(struct moca_pqos_update_flow_in * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_pqos_list_check(struct moca_pqos_list_in * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_ooo_lmo_check(uint32_t node_id, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_start_ulmo_check(struct moca_start_ulmo * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_rxd_lmo_request_check(struct moca_rxd_lmo_request * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_aca_check(struct moca_aca_in * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_moca_reset_check(struct moca_moca_reset_in * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_wakeup_node_check(uint32_t node, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_if_access_en_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_led_mode_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_m1_tx_power_variation_check(uint32_t state, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_nc_listening_interval_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_nc_heartbeat_interval_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_wom_magic_enable_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_pm_restore_on_link_down_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_wom_pattern_check(struct moca_wom_pattern_set * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_wom_ip_check(struct moca_wom_ip * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_standby_power_state_check(uint32_t state, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_wom_mode_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_ps_cmd_check(uint32_t new_state, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_pmk_exchange_interval_check(uint32_t msec, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_tek_exchange_interval_check(uint32_t msec, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_aes_exchange_interval_check(uint32_t msec, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_const_tx_params_check(struct moca_const_tx_params * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_lab_iq_diagram_set_check(struct moca_lab_iq_diagram_set * in, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_mps_en_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_mps_privacy_receive_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_mps_privacy_down_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_mps_walk_time_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_mps_unpaired_time_check(uint32_t val, uint32_t flags);
MOCALIB_GEN_RANGE_FUNCTION int moca_privacy_defaults_check(uint32_t val, uint32_t flags);

MOCALIB_GEN_GET_FUNCTION int moca_get_preferred_nc(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_preferred_nc(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_single_channel_operation(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_single_channel_operation(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_continuous_power_tx_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_continuous_power_tx_mode(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_continuous_rx_mode_attn(void *vctx, int32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_continuous_rx_mode_attn(void *vctx, int32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_lof(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_lof(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_no_ifg6(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_no_ifg6(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_bonding(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_bonding(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_listening_freq_mask(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_listening_freq_mask(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_listening_duration(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_listening_duration(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_limit_traffic(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_limit_traffic(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_remote_man(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_remote_man(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_c4_moca20_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_c4_moca20_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_power_save_mechanism_dis(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_power_save_mechanism_dis(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_psm_config(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_psm_config(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_use_ext_data_mem(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_use_ext_data_mem(void *vctx, uint32_t val);

MOCALIB_GEN_SET_FUNCTION int moca_set_aif_mode(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_aif_mode(void *vctx, uint32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_prop_bonding_compatibility_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_prop_bonding_compatibility_mode(void *vctx, uint32_t val);

MOCALIB_GEN_SET_FUNCTION int moca_set_rdeg_3450(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_rdeg_3450(void *vctx, uint32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_phy_clock(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_phy_clock(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_mac_addr(void *vctx, struct moca_mac_addr *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_mac_addr(void *vctx, struct moca_mac_addr *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_node_status(void *vctx, struct moca_node_status *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_beacon_channel_set(void *vctx, uint32_t channel);

MOCALIB_GEN_GET_FUNCTION int moca_get_fw_version(void *vctx, struct moca_fw_version *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_tx_power_tune(void *vctx, struct moca_max_tx_power_tune *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_tx_power_tune(void *vctx, struct moca_max_tx_power_tune *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_tx_power_tune_sec_ch(void *vctx, struct moca_max_tx_power_tune_sec_ch *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_tx_power_tune_sec_ch(void *vctx, struct moca_max_tx_power_tune_sec_ch *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_rx_power_tune(void *vctx, struct moca_rx_power_tune *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_rx_power_tune(void *vctx, struct moca_rx_power_tune *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_mocad_forwarding_rx_mac(void *vctx, macaddr_t * mac_addr);

MOCALIB_GEN_SET_FUNCTION int moca_set_mocad_forwarding_rx_ack(void *vctx, const struct moca_mocad_forwarding_rx_ack *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_mocad_forwarding_tx_alloc(void *vctx, struct moca_mocad_forwarding_tx_alloc *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_mocad_forwarding_tx_send(void *vctx, const struct moca_mocad_forwarding_tx_send *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_impedance_mode_bonding(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_impedance_mode_bonding(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_rework_6802(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_rework_6802(void *vctx, uint32_t val);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_core_ready_cb(void *vctx, void (*callback)(void *userarg, struct moca_core_ready *out), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_power_up_status_cb(void *vctx, void (*callback)(void *userarg, uint32_t status), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_new_lof_cb(void *vctx, void (*callback)(void *userarg, uint32_t lof), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_admission_completed_cb(void *vctx, void (*callback)(void *userarg, uint32_t lof), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_tpcap_done_cb(void *vctx, void (*callback)(void *userarg), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mocad_forwarding_rx_packet_cb(void *vctx, void (*callback)(void *userarg, struct moca_mocad_forwarding_rx_packet *out), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mocad_forwarding_tx_ack_cb(void *vctx, void (*callback)(void *userarg, uint32_t offset), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_pr_degradation_cb(void *vctx, void (*callback)(void *userarg), void *userarg);

MOCALIB_GEN_SET_FUNCTION int moca_set_start(void *vctx);

MOCALIB_GEN_SET_FUNCTION int moca_set_stop(void *vctx);

MOCALIB_GEN_GET_FUNCTION int moca_get_drv_info(void *vctx, uint32_t reset_stats, struct moca_drv_info *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_miscval(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_miscval(void *vctx, uint32_t *val);

MOCALIB_GEN_SET_FUNCTION int moca_set_en_capable(void *vctx, uint32_t enable);
MOCALIB_GEN_GET_FUNCTION int moca_get_en_capable(void *vctx, uint32_t *enable);

MOCALIB_GEN_SET_FUNCTION int moca_set_restore_defaults(void *vctx);

MOCALIB_GEN_GET_FUNCTION int moca_get_mocad_version(void *vctx, struct moca_mocad_version *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_restart(void *vctx);

MOCALIB_GEN_GET_FUNCTION int moca_get_lof_update(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_lof_update(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_primary_ch_offset(void *vctx, int32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_primary_ch_offset(void *vctx, int32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_assertText(void *vctx, uint32_t *assertText);
MOCALIB_GEN_SET_FUNCTION int moca_set_assertText(void *vctx, uint32_t assertText);

MOCALIB_GEN_GET_FUNCTION int moca_get_wdog_enable(void *vctx, uint32_t *enable);
MOCALIB_GEN_SET_FUNCTION int moca_set_wdog_enable(void *vctx, uint32_t enable);

MOCALIB_GEN_SET_FUNCTION int moca_set_miscval2(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_miscval2(void *vctx, uint32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_mr_seq_num(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_mr_seq_num(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_secondary_ch_offset(void *vctx, int32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_secondary_ch_offset(void *vctx, int32_t val);

MOCALIB_GEN_SET_FUNCTION int moca_set_cof(void *vctx, uint32_t val);

MOCALIB_GEN_SET_FUNCTION int moca_set_amp_type(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_amp_type(void *vctx, uint32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_tpc_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_tpc_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_tx_power(void *vctx, int32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_tx_power(void *vctx, int32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_beacon_pwr_reduction(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_beacon_pwr_reduction(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_beacon_pwr_reduction_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_beacon_pwr_reduction_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_bo_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_bo_mode(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_qam256_capability(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_qam256_capability(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_otf_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_otf_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_star_topology_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_star_topology_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_ofdma_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_ofdma_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_min_bw_alarm_threshold(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_min_bw_alarm_threshold(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_en_max_rate_in_max_bo(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_en_max_rate_in_max_bo(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_target_phy_rate_qam128(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_target_phy_rate_qam128(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_target_phy_rate_qam256(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_target_phy_rate_qam256(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_sapm_en(void *vctx, uint32_t *bool_val);
MOCALIB_GEN_SET_FUNCTION int moca_set_sapm_en(void *vctx, uint32_t bool_val);

MOCALIB_GEN_GET_FUNCTION int moca_get_arpl_th_50(void *vctx, int32_t *arpl);
MOCALIB_GEN_SET_FUNCTION int moca_set_arpl_th_50(void *vctx, int32_t arpl);

MOCALIB_GEN_GET_FUNCTION int moca_get_rlapm_en(void *vctx, uint32_t *bool_val);
MOCALIB_GEN_SET_FUNCTION int moca_set_rlapm_en(void *vctx, uint32_t bool_val);

MOCALIB_GEN_GET_FUNCTION int moca_get_freq_shift(void *vctx, uint32_t *direction);
MOCALIB_GEN_SET_FUNCTION int moca_set_freq_shift(void *vctx, uint32_t direction);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_phy_rate(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_phy_rate(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_bandwidth(void *vctx, uint32_t *bandwidth);
MOCALIB_GEN_SET_FUNCTION int moca_set_bandwidth(void *vctx, uint32_t bandwidth);

MOCALIB_GEN_GET_FUNCTION int moca_get_arpl_th_100(void *vctx, int32_t *arpl);
MOCALIB_GEN_SET_FUNCTION int moca_set_arpl_th_100(void *vctx, int32_t arpl);

MOCALIB_GEN_GET_FUNCTION int moca_get_adc_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_adc_mode(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_phy_rate_turbo(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_phy_rate_turbo(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_phy_rate_50M(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_phy_rate_50M(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_constellation_all(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_constellation_all(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_constellation(void *vctx, uint32_t node_id, struct moca_max_constellation *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_constellation(void *vctx, const struct moca_max_constellation *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_rlapm_table_50(void *vctx, struct moca_rlapm_table_50 *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_rlapm_table_50(void *vctx, struct moca_rlapm_table_50 *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_phy_status(void *vctx, uint32_t *tx_gcd_power_reduction);

MOCALIB_GEN_GET_FUNCTION int moca_get_rlapm_table_100(void *vctx, struct moca_rlapm_table_100 *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_rlapm_table_100(void *vctx, struct moca_rlapm_table_100 *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_rx_gain_params(void *vctx, uint32_t table_index, struct moca_rx_gain_params *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_tx_power_params(void *vctx, struct moca_tx_power_params_in *in, struct moca_tx_power_params *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_nv_cal_enable(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_nv_cal_enable(void *vctx, uint32_t *val);

MOCALIB_GEN_SET_FUNCTION int moca_set_rlapm_cap_50(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_rlapm_cap_50(void *vctx, uint32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_margin_rs(void *vctx, struct moca_snr_margin_rs *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_margin_rs(void *vctx, struct moca_snr_margin_rs *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_margin_ldpc(void *vctx, struct moca_snr_margin_ldpc *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_margin_ldpc(void *vctx, struct moca_snr_margin_ldpc *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_margin_ldpc_sec_ch(void *vctx, struct moca_snr_margin_ldpc_sec_ch *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_margin_ldpc_sec_ch(void *vctx, struct moca_snr_margin_ldpc_sec_ch *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_margin_ldpc_pre5(void *vctx, struct moca_snr_margin_ldpc_pre5 *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_margin_ldpc_pre5(void *vctx, struct moca_snr_margin_ldpc_pre5 *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_margin_ofdma(void *vctx, struct moca_snr_margin_ofdma *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_margin_ofdma(void *vctx, struct moca_snr_margin_ofdma *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_rlapm_cap_100(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_rlapm_cap_100(void *vctx, uint32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_sapm_table_50(void *vctx, struct moca_sapm_table_50 *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_sapm_table_50(void *vctx, struct moca_sapm_table_50 *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_sapm_table_100(void *vctx, struct moca_sapm_table_100 *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_sapm_table_100(void *vctx, struct moca_sapm_table_100 *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_nv_cal_clear(void *vctx);

MOCALIB_GEN_GET_FUNCTION int moca_get_sapm_table_sec(void *vctx, struct moca_sapm_table_sec *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_sapm_table_sec(void *vctx, struct moca_sapm_table_sec *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_amp_reg(void *vctx, uint32_t addr, struct moca_amp_reg *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_amp_reg(void *vctx, const struct moca_amp_reg *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_margin_ldpc_pri_ch(void *vctx, struct moca_snr_margin_ldpc_pri_ch *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_margin_ldpc_pri_ch(void *vctx, struct moca_snr_margin_ldpc_pri_ch *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_margin_pre5_pri_ch(void *vctx, struct moca_snr_margin_pre5_pri_ch *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_margin_pre5_pri_ch(void *vctx, struct moca_snr_margin_pre5_pri_ch *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_margin_pre5_sec_ch(void *vctx, struct moca_snr_margin_pre5_sec_ch *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_margin_pre5_sec_ch(void *vctx, struct moca_snr_margin_pre5_sec_ch *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_frame_size(void *vctx, uint32_t *bytes);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_frame_size(void *vctx, uint32_t bytes);

MOCALIB_GEN_GET_FUNCTION int moca_get_min_aggr_waiting_time(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_min_aggr_waiting_time(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_selective_rr(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_selective_rr(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_transmit_time(void *vctx, uint32_t *usec);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_transmit_time(void *vctx, uint32_t usec);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_pkt_aggr(void *vctx, uint32_t *pkts);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_pkt_aggr(void *vctx, uint32_t pkts);

MOCALIB_GEN_GET_FUNCTION int moca_get_rtr_config(void *vctx, struct moca_rtr_config *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_rtr_config(void *vctx, struct moca_rtr_config *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_tlp_mode(void *vctx, uint32_t *mode);
MOCALIB_GEN_SET_FUNCTION int moca_set_tlp_mode(void *vctx, uint32_t mode);

MOCALIB_GEN_GET_FUNCTION int moca_get_max_pkt_aggr_bonding(void *vctx, uint32_t *pkts);
MOCALIB_GEN_SET_FUNCTION int moca_set_max_pkt_aggr_bonding(void *vctx, uint32_t pkts);

MOCALIB_GEN_GET_FUNCTION int moca_get_multicast_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_multicast_mode(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_egr_mc_filter_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_egr_mc_filter_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_fc_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_fc_mode(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_pqos_max_packet_size(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_pqos_max_packet_size(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_per_mode(void *vctx, uint32_t *mode);
MOCALIB_GEN_SET_FUNCTION int moca_set_per_mode(void *vctx, uint32_t mode);

MOCALIB_GEN_GET_FUNCTION int moca_get_policing_en(void *vctx, uint32_t *enable);
MOCALIB_GEN_SET_FUNCTION int moca_set_policing_en(void *vctx, uint32_t enable);

MOCALIB_GEN_GET_FUNCTION int moca_get_pqos_egress_numflows(void *vctx, uint32_t *pqos_egress_numflows);

MOCALIB_GEN_GET_FUNCTION int moca_get_orr_en(void *vctx, uint32_t *enable);
MOCALIB_GEN_SET_FUNCTION int moca_set_orr_en(void *vctx, uint32_t enable);

MOCALIB_GEN_GET_FUNCTION int moca_get_brcmtag_enable(void *vctx, uint32_t *enable);
MOCALIB_GEN_SET_FUNCTION int moca_set_brcmtag_enable(void *vctx, uint32_t enable);

MOCALIB_GEN_GET_FUNCTION int moca_get_unknown_ratelimit_en(void *vctx, uint32_t *enable);
MOCALIB_GEN_SET_FUNCTION int moca_set_unknown_ratelimit_en(void *vctx, uint32_t enable);

MOCALIB_GEN_GET_FUNCTION int moca_get_egr_mc_addr_filter(void *vctx, uint32_t entryid, struct moca_egr_mc_addr_filter *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_egr_mc_addr_filter(void *vctx, struct moca_egr_mc_addr_filter_set *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_pqos_maintenance_start(void *vctx);

MOCALIB_GEN_GET_FUNCTION int moca_get_uc_fwd(void *vctx, struct moca_uc_fwd *out, int max_out_len);

MOCALIB_GEN_GET_FUNCTION int moca_get_mc_fwd(void *vctx, struct moca_mc_fwd *out, int max_out_len);
MOCALIB_GEN_SET_FUNCTION int moca_set_mc_fwd(void *vctx, struct moca_mc_fwd_set *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_src_addr(void *vctx, struct moca_src_addr *out, int max_out_len);

MOCALIB_GEN_GET_FUNCTION int moca_get_mac_aging(void *vctx, struct moca_mac_aging *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_mac_aging(void *vctx, struct moca_mac_aging *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_loopback_en(void *vctx, uint32_t *en);
MOCALIB_GEN_SET_FUNCTION int moca_set_loopback_en(void *vctx, uint32_t en);

MOCALIB_GEN_GET_FUNCTION int moca_get_mcfilter_enable(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_mcfilter_enable(void *vctx, uint32_t val);

MOCALIB_GEN_SET_FUNCTION int moca_set_mcfilter_addentry(void *vctx, struct moca_mcfilter_addentry *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_mcfilter_delentry(void *vctx, struct moca_mcfilter_delentry *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_pause_fc_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_pause_fc_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_stag_priority(void *vctx, struct moca_stag_priority *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_stag_priority(void *vctx, const struct moca_stag_priority *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_stag_removal(void *vctx, struct moca_stag_removal *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_stag_removal(void *vctx, const struct moca_stag_removal *in);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_ucfwd_update_cb(void *vctx, void (*callback)(void *userarg), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_pqos_maintenance_complete_cb(void *vctx, void (*callback)(void *userarg, struct moca_pqos_maintenance_complete *out), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_pqos_create_flow_cb(void *vctx, void (*callback)(void *userarg, struct moca_pqos_create_flow_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_pqos_create_flow(void *vctx, struct moca_pqos_create_flow_in *in, struct moca_pqos_create_flow_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_pqos_update_flow_cb(void *vctx, void (*callback)(void *userarg, struct moca_pqos_update_flow_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_pqos_update_flow(void *vctx, struct moca_pqos_update_flow_in *in, struct moca_pqos_update_flow_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_pqos_delete_flow_cb(void *vctx, void (*callback)(void *userarg, struct moca_pqos_delete_flow_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_pqos_delete_flow(void *vctx, macaddr_t flow_id, struct moca_pqos_delete_flow_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_pqos_list_cb(void *vctx, void (*callback)(void *userarg, struct moca_pqos_list_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_pqos_list(void *vctx, struct moca_pqos_list_in *in, struct moca_pqos_list_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_pqos_query_cb(void *vctx, void (*callback)(void *userarg, struct moca_pqos_query_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_pqos_query(void *vctx, macaddr_t flow_id, struct moca_pqos_query_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_pqos_status_cb(void *vctx, void (*callback)(void *userarg, struct moca_pqos_status_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_pqos_status(void *vctx, uint32_t unused, struct moca_pqos_status_out *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_mcfilter_clear_table(void *vctx);

MOCALIB_GEN_GET_FUNCTION int moca_get_mcfilter_table(void *vctx, struct moca_mcfilter_table *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_host_qos(void *vctx, uint32_t *enable);
MOCALIB_GEN_SET_FUNCTION int moca_set_host_qos(void *vctx, uint32_t enable);

MOCALIB_GEN_GET_FUNCTION int moca_get_network_state(void *vctx, uint32_t *state);

MOCALIB_GEN_GET_FUNCTION int moca_get_taboo_channels(void *vctx, struct moca_taboo_channels *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_taboo_channels(void *vctx, const struct moca_taboo_channels *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_gen_node_status(void *vctx, uint32_t index, struct moca_gen_node_status *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_gen_node_ext_status(void *vctx, struct moca_gen_node_ext_status_in *in, struct moca_gen_node_ext_status *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_node_stats(void *vctx, struct moca_node_stats_in *in, struct moca_node_stats *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_node_stats_ext(void *vctx, struct moca_node_stats_ext_in *in, struct moca_node_stats_ext *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_network_status(void *vctx, struct moca_network_status *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_ooo_lmo(void *vctx, uint32_t node_id);

MOCALIB_GEN_GET_FUNCTION int moca_get_start_ulmo(void *vctx, struct moca_start_ulmo *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_start_ulmo(void *vctx, const struct moca_start_ulmo *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_rxd_lmo_request(void *vctx, const struct moca_rxd_lmo_request *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_ofdma_definition_table(void *vctx, struct moca_ofdma_definition_table *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_ofdma_assignment_table(void *vctx, struct moca_ofdma_assignment_table *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_adm_stats(void *vctx, struct moca_adm_stats *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_admission_status_cb(void *vctx, void (*callback)(void *userarg, uint32_t status), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_limited_bw_cb(void *vctx, void (*callback)(void *userarg, uint32_t bw_status), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_lmo_info_cb(void *vctx, void (*callback)(void *userarg, struct moca_lmo_info *out), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_topology_changed_cb(void *vctx, void (*callback)(void *userarg, uint32_t nodemask), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_moca_version_changed_cb(void *vctx, void (*callback)(void *userarg, uint32_t new_version), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_moca_reset_request_cb(void *vctx, void (*callback)(void *userarg, struct moca_moca_reset_request *out), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_nc_id_changed_cb(void *vctx, void (*callback)(void *userarg, uint32_t new_nc_id), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mr_event_cb(void *vctx, void (*callback)(void *userarg, uint32_t status), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_aca_cb(void *vctx, void (*callback)(void *userarg, struct moca_aca_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_aca(void *vctx, struct moca_aca_in *in, struct moca_aca_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_fmr_init_cb(void *vctx, void (*callback)(void *userarg, struct moca_fmr_init_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_fmr_init(void *vctx, uint32_t node_mask, struct moca_fmr_init_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_moca_reset_cb(void *vctx, void (*callback)(void *userarg, struct moca_moca_reset_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_moca_reset(void *vctx, struct moca_moca_reset_in *in, struct moca_moca_reset_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_dd_init_cb(void *vctx, void (*callback)(void *userarg, struct moca_dd_init_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_dd_init(void *vctx, uint32_t node_mask, struct moca_dd_init_out *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_fmr_20_cb(void *vctx, void (*callback)(void *userarg, struct moca_fmr_20_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_fmr_20(void *vctx, uint32_t node_mask, struct moca_fmr_20_out *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_error_stats(void *vctx, struct moca_error_stats *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_hostless_mode_cb(void *vctx, void (*callback)(void *userarg, uint32_t status), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_hostless_mode(void *vctx, uint32_t enable, uint32_t *status);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_wakeup_node_cb(void *vctx, void (*callback)(void *userarg, uint32_t status), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_wakeup_node(void *vctx, uint32_t node, uint32_t *status);

MOCALIB_GEN_GET_FUNCTION int moca_get_last_mr_events(void *vctx, struct moca_last_mr_events *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_rf_band(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_rf_band(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_if_access_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_if_access_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_led_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_led_mode(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_gen_stats(void *vctx, uint32_t reset_stats, struct moca_gen_stats *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_interface_status(void *vctx, struct moca_interface_status *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_if_access_table(void *vctx, struct moca_if_access_table *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_if_access_table(void *vctx, struct moca_if_access_table *in);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_link_up_state_cb(void *vctx, void (*callback)(void *userarg, uint32_t status), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_new_rf_band_cb(void *vctx, void (*callback)(void *userarg, uint32_t rf_band), void *userarg);

MOCALIB_GEN_GET_FUNCTION int moca_get_ext_octet_count(void *vctx, struct moca_ext_octet_count *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_reset_stats(void *vctx);

MOCALIB_GEN_GET_FUNCTION int moca_get_m1_tx_power_variation(void *vctx, uint32_t *state);
MOCALIB_GEN_SET_FUNCTION int moca_set_m1_tx_power_variation(void *vctx, uint32_t state);

MOCALIB_GEN_GET_FUNCTION int moca_get_nc_listening_interval(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_nc_listening_interval(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_nc_heartbeat_interval(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_nc_heartbeat_interval(void *vctx, uint32_t val);

MOCALIB_GEN_SET_FUNCTION int moca_set_wom_magic_enable(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_wom_magic_enable(void *vctx, uint32_t *val);

MOCALIB_GEN_SET_FUNCTION int moca_set_pm_restore_on_link_down(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_pm_restore_on_link_down(void *vctx, uint32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_power_state(void *vctx, uint32_t *state);

MOCALIB_GEN_GET_FUNCTION int moca_get_hostless_mode_request(void *vctx, uint32_t *enable);
MOCALIB_GEN_SET_FUNCTION int moca_set_hostless_mode_request(void *vctx, uint32_t enable);

MOCALIB_GEN_SET_FUNCTION int moca_set_wakeup_node_request(void *vctx, uint32_t node);

MOCALIB_GEN_GET_FUNCTION int moca_get_node_power_state(void *vctx, uint32_t node, struct moca_node_power_state *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_filter_m2_data_wakeUp(void *vctx, uint32_t *mode);
MOCALIB_GEN_SET_FUNCTION int moca_set_filter_m2_data_wakeUp(void *vctx, uint32_t mode);

MOCALIB_GEN_GET_FUNCTION int moca_get_wom_pattern(void *vctx, struct moca_wom_pattern *out, int max_out_len);
MOCALIB_GEN_SET_FUNCTION int moca_set_wom_pattern(void *vctx, struct moca_wom_pattern_set *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_wom_ip(void *vctx, uint32_t *out, int max_out_len);
MOCALIB_GEN_SET_FUNCTION int moca_set_wom_ip(void *vctx, const struct moca_wom_ip *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_wom_magic_mac(void *vctx, struct moca_wom_magic_mac *in);
MOCALIB_GEN_GET_FUNCTION int moca_get_wom_magic_mac(void *vctx, struct moca_wom_magic_mac *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_standby_power_state(void *vctx, uint32_t *state);
MOCALIB_GEN_SET_FUNCTION int moca_set_standby_power_state(void *vctx, uint32_t state);

MOCALIB_GEN_SET_FUNCTION int moca_set_wom_mode(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_wom_mode(void *vctx, uint32_t *val);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_power_state_rsp_cb(void *vctx, void (*callback)(void *userarg, uint32_t rsp_code), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_power_state_event_cb(void *vctx, void (*callback)(void *userarg, uint32_t event_code), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_power_state_cap_cb(void *vctx, void (*callback)(void *userarg, uint32_t power_modes), void *userarg);

MOCALIB_GEN_SET_FUNCTION int moca_set_wol(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_wol(void *vctx, uint32_t *val);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_ps_cmd_cb(void *vctx, void (*callback)(void *userarg, uint32_t rsp_code), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_ps_cmd(void *vctx, uint32_t new_state, uint32_t *rsp_code);

MOCALIB_GEN_GET_FUNCTION int moca_get_power_state_capabilities(void *vctx, uint32_t *power_modes);

MOCALIB_GEN_GET_FUNCTION int moca_get_last_ps_event_code(void *vctx, int32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_privacy_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_privacy_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_pmk_exchange_interval(void *vctx, uint32_t *msec);
MOCALIB_GEN_SET_FUNCTION int moca_set_pmk_exchange_interval(void *vctx, uint32_t msec);

MOCALIB_GEN_GET_FUNCTION int moca_get_tek_exchange_interval(void *vctx, uint32_t *msec);
MOCALIB_GEN_SET_FUNCTION int moca_set_tek_exchange_interval(void *vctx, uint32_t msec);

MOCALIB_GEN_GET_FUNCTION int moca_get_aes_exchange_interval(void *vctx, uint32_t *msec);
MOCALIB_GEN_SET_FUNCTION int moca_set_aes_exchange_interval(void *vctx, uint32_t msec);

MOCALIB_GEN_GET_FUNCTION int moca_get_mmk_key(void *vctx, struct moca_mmk_key *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_pmk_initial_key(void *vctx, struct moca_pmk_initial_key *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_aes_mm_key(void *vctx, struct moca_aes_mm_key *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_aes_mm_key(void *vctx, const struct moca_aes_mm_key *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_aes_pm_key(void *vctx, struct moca_aes_pm_key *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_aes_pm_key(void *vctx, const struct moca_aes_pm_key *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_current_keys(void *vctx, struct moca_current_keys *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_permanent_salt(void *vctx, struct moca_permanent_salt *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_aes_pmk_initial_key(void *vctx, struct moca_aes_pmk_initial_key *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_aes_pmk_initial_key(void *vctx, const struct moca_aes_pmk_initial_key *in);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_key_changed_cb(void *vctx, void (*callback)(void *userarg, struct moca_key_changed *out), void *userarg);

MOCALIB_GEN_GET_FUNCTION int moca_get_key_times(void *vctx, struct moca_key_times *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_password(void *vctx, struct moca_password *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_password(void *vctx, struct moca_password *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_mtm_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_mtm_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_cir_prints(void *vctx, uint32_t *bool_val);
MOCALIB_GEN_SET_FUNCTION int moca_set_cir_prints(void *vctx, uint32_t bool_val);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_prints(void *vctx, uint32_t *bool_val);
MOCALIB_GEN_SET_FUNCTION int moca_set_snr_prints(void *vctx, uint32_t bool_val);

MOCALIB_GEN_GET_FUNCTION int moca_get_sigma2_prints(void *vctx, uint32_t *bool_val);
MOCALIB_GEN_SET_FUNCTION int moca_set_sigma2_prints(void *vctx, uint32_t bool_val);

MOCALIB_GEN_GET_FUNCTION int moca_get_bad_probe_prints(void *vctx, uint32_t *bool_val);
MOCALIB_GEN_SET_FUNCTION int moca_set_bad_probe_prints(void *vctx, uint32_t bool_val);

MOCALIB_GEN_GET_FUNCTION int moca_get_const_tx_params(void *vctx, struct moca_const_tx_params *out);
MOCALIB_GEN_SET_FUNCTION int moca_set_const_tx_params(void *vctx, const struct moca_const_tx_params *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_gmii_trap_header(void *vctx, struct moca_gmii_trap_header *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_led_status(void *vctx, uint32_t *led_status);

MOCALIB_GEN_GET_FUNCTION int moca_get_moca_core_trace_enable(void *vctx, uint32_t *bool_val);
MOCALIB_GEN_SET_FUNCTION int moca_set_moca_core_trace_enable(void *vctx, uint32_t bool_val);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_error_cb(void *vctx, void (*callback)(void *userarg, struct moca_error *out), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_error_lookup_cb(void *vctx, void (*callback)(void *userarg, struct moca_error_lookup *out), void *userarg);

MOCALIB_GEN_SET_FUNCTION int moca_set_error_to_mask(void *vctx, const struct moca_error_to_mask *in);
MOCALIB_GEN_GET_FUNCTION int moca_get_error_to_mask(void *vctx, struct moca_error_to_mask *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_fw_file(void *vctx, struct moca_fw_file *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_verbose(void *vctx, uint32_t *level);
MOCALIB_GEN_SET_FUNCTION int moca_set_verbose(void *vctx, uint32_t level);

MOCALIB_GEN_SET_FUNCTION int moca_set_dont_start_moca(void *vctx, uint32_t dont_start_moca);
MOCALIB_GEN_GET_FUNCTION int moca_get_dont_start_moca(void *vctx, uint32_t *dont_start_moca);

MOCALIB_GEN_SET_FUNCTION int moca_set_no_rtt(void *vctx);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mocad_printf_cb(void *vctx, void (*callback)(void *userarg, struct moca_mocad_printf_out *out), void *userarg);
MOCALIB_GEN_DO_FUNCTION int moca_do_mocad_printf(void *vctx, struct moca_mocad_printf_out *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_lab_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_lab_mode(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_nc_mode(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_nc_mode(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_rx_tx_packets_per_qm(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_rx_tx_packets_per_qm(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_extra_rx_packets_per_qm(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_extra_rx_packets_per_qm(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_target_phy_rate_20(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_target_phy_rate_20(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_target_phy_rate_20_turbo(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_target_phy_rate_20_turbo(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_turbo_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_turbo_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_target_phy_rate_20_turbo_vlper(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_target_phy_rate_20_turbo_vlper(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_target_phy_rate_20_sec_ch(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_target_phy_rate_20_sec_ch(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_target_phy_rate_20_turbo_sec_ch(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_target_phy_rate_20_turbo_sec_ch(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_target_phy_rate_20_turbo_vlper_sec_ch(void *vctx, uint32_t *mbps);
MOCALIB_GEN_SET_FUNCTION int moca_set_target_phy_rate_20_turbo_vlper_sec_ch(void *vctx, uint32_t mbps);

MOCALIB_GEN_GET_FUNCTION int moca_get_cap_phy_rate_en(void *vctx, uint32_t *bool_val);
MOCALIB_GEN_SET_FUNCTION int moca_set_cap_phy_rate_en(void *vctx, uint32_t bool_val);

MOCALIB_GEN_GET_FUNCTION int moca_get_cap_target_phy_rate(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_cap_target_phy_rate(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_cap_snr_base_margin(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_cap_snr_base_margin(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_lab_pilots(void *vctx, struct moca_lab_pilots *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_lab_iq_diagram_set(void *vctx, const struct moca_lab_iq_diagram_set *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_lab_register(void *vctx, uint32_t address, uint32_t *data);
MOCALIB_GEN_SET_FUNCTION int moca_set_lab_register(void *vctx, const struct moca_lab_register *in);

MOCALIB_GEN_SET_FUNCTION int moca_set_lab_tpcap(void *vctx, const struct moca_lab_tpcap *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_host_pool(void *vctx, struct moca_host_pool *out);

MOCALIB_GEN_SET_FUNCTION int moca_set_force_handoff(void *vctx, const struct moca_force_handoff *in);

MOCALIB_GEN_GET_FUNCTION int moca_get_tpcap_capture_time(void *vctx, uint32_t *last_time_tpcap_cap);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_assert_cb(void *vctx, void (*callback)(void *userarg, struct moca_assert *out), void *userarg);

MOCALIB_GEN_SET_FUNCTION int moca_set_message(void *vctx, uint32_t id);

MOCALIB_GEN_GET_FUNCTION int moca_get_snr_data(void *vctx, struct moca_snr_data *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_iq_data(void *vctx, struct moca_iq_data *out);

MOCALIB_GEN_GET_FUNCTION int moca_get_mps_en(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_mps_en(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_mps_privacy_receive(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_mps_privacy_receive(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_mps_privacy_down(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_mps_privacy_down(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_mps_walk_time(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_mps_walk_time(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_mps_unpaired_time(void *vctx, uint32_t *val);
MOCALIB_GEN_SET_FUNCTION int moca_set_mps_unpaired_time(void *vctx, uint32_t val);

MOCALIB_GEN_GET_FUNCTION int moca_get_mps_state(void *vctx, uint32_t *val);

MOCALIB_GEN_GET_FUNCTION int moca_get_mps_init_scan_payload(void *vctx, struct moca_mps_init_scan_payload *out);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mps_privacy_changed_cb(void *vctx, void (*callback)(void *userarg), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mps_trigger_cb(void *vctx, void (*callback)(void *userarg), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mps_pair_fail_cb(void *vctx, void (*callback)(void *userarg), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_init_scan_rec_cb(void *vctx, void (*callback)(void *userarg), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mps_request_mpskey_cb(void *vctx, void (*callback)(void *userarg, struct moca_mps_request_mpskey *out), void *userarg);

MOCALIB_GEN_REGISTER_FUNCTION void moca_register_mps_admission_nochange_cb(void *vctx, void (*callback)(void *userarg), void *userarg);

MOCALIB_GEN_SET_FUNCTION int moca_set_mps_button_press(void *vctx);

MOCALIB_GEN_SET_FUNCTION int moca_set_mps_reset(void *vctx);

MOCALIB_GEN_SET_FUNCTION int moca_set_privacy_defaults(void *vctx, uint32_t val);
MOCALIB_GEN_GET_FUNCTION int moca_get_privacy_defaults(void *vctx, uint32_t *val);
MOCALIB_GEN_NVRAM_FUNCTION void moca_write_nvram( void * handle, char *c, uint32_t max_len);


/* GENERATED API ABOVE THIS LINE - DO NOT EDIT */

#ifdef __cplusplus
}
#endif

#endif /* ! _MOCALIB_H_ */
