/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

#ifndef _BL_LILAC_DRV_RUNNER_RDD_DEFS_H
#define _BL_LILAC_DRV_RUNNER_RDD_DEFS_H

#include "rdpa_types.h"
#include "rdpa_ip_class_basic.h"
#include "rdpa_cpu_basic.h"
#include "rdp_drv_bpm.h"

/* Runner Device Driver Errors */
typedef enum
{
    BL_LILAC_RDD_OK                                                          = 0,
    BL_LILAC_RDD_ERROR_MALLOC_FAILED                                         = 10,
    BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID                                = 11,
    BL_LILAC_RDD_ERROR_ILLEGAL_EMAC_ID                                       = 12,
    BL_LILAC_RDD_ERROR_ILLEGAL_QUEUE_ID                                      = 13,
    BL_LILAC_RDD_ERROR_ILLEGAL_WAN_CHANNEL_ID                                = 14,
    BL_LILAC_RDD_ERROR_ILLEGAL_RATE_CONTROLLER_ID                            = 15,
    BL_LILAC_RDD_ERROR_ILLEGAL_RATE_SHAPER_ID                                = 16,
    BL_LILAC_RDD_ERROR_ILLEGAL_POLICER_ID                                    = 17,
    BL_LILAC_RDD_ERROR_ILLEGAL_DIRECTION                                     = 18,
    BL_LILAC_RDD_ERROR_RATE_CONTROLLERS_POOL_OVERFLOW                        = 50,
    BL_LILAC_RDD_ERROR_RATE_CONTROLLER_NOT_CONFIGURED                        = 52,
    BL_LILAC_RDD_ERROR_RATE_SHAPER_NOT_CONFIGURED                            = 55,
    BL_LILAC_RDD_ERROR_GPON_TX_QUEUES_POOL_OVERFLOW                          = 70,
    BL_LILAC_RDD_ERROR_GPON_TX_QUEUE_NOT_CONFIGURED                          = 72,
    BL_LILAC_RDD_ERROR_GPON_TX_QUEUE_EMPTY                                   = 73,
    BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_ILLEGAL                                  = 100,
    BL_LILAC_RDD_ERROR_CPU_RX_REASON_ILLEGAL                                 = 101,
    BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY                                    = 102,
    BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_INVALID                                  = 103,
    BL_LILAC_RDD_ERROR_CPU_TX_QUEUE_FULL                                     = 120,
    BL_LILAC_RDD_ERROR_CPU_TX_NOT_ALLOWED                                    = 122,
    BL_LILAC_RDD_ERROR_PCI_TX_QUEUE_EMPTY                                    = 130,
    BL_LILAC_RDD_ERROR_PCI_QUEUE_THRESHOLD_TOO_SMALL                         = 131,
    BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY                                      = 150,
    BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY                                   = 151,
    BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY                                      = 152,
    BL_LILAC_RDD_ERROR_LOOKUP_ENTRY_EXISTS                                   = 153,
    BL_LILAC_RDD_ERROR_HASH_TABLE_NO_EMPTY_ENTRY                             = 156,
    BL_LILAC_RDD_ERROR_HASH_TABLE_NO_MATCHING_KEY                            = 157,
    BL_LILAC_RDD_ERROR_ILLEGAL_MAC_ENTRY_ID                                  = 160,
    BL_LILAC_RDD_ERROR_GET_MAC_ENTRY                                         = 161,
    BL_LILAC_RDD_ERROR_MAC_ENTRY_EXISTS                                      = 162,
    BL_LILAC_RDD_ERROR_MAC_ENTRY_DOESNT_EXIST                                = 163,
    BL_LILAC_RDD_ERROR_ILLEGAL_ARP_ENTRY_ID                                  = 170,
    BL_LILAC_RDD_ERROR_ILLEGAL_IPTV_ENTRY_ID                                 = 175,
    BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_FULL                       = 180,
    BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_ILEGGAL_GROUP_SORT         = 181,
    BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_NOT_EXIST                  = 182,
    BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_KEY_TOO_LONG               = 183,
    BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_LONG_TABLE_FULL            = 184,
    BL_LILAC_RDD_ERROR_CAM_LOOKUP_TABLE_FULL                                 = 200,
    BL_LILAC_RDD_ERROR_CAM_LOOKUP_TABLE_EMPTY                                = 201,
    BL_LILAC_RDD_ERROR_CAM_LOOKUP_FAILED                                     = 202,
    BL_LILAC_RDD_ERROR_CAM_INSERTION_FAILED                                  = 203,
    BL_LILAC_RDD_ERROR_BPM_ALLOC_FAIL                                        = 230,
    BL_LILAC_RDD_ERROR_BPM_FREE_FAIL                                         = 231,
    BL_LILAC_RDD_ERROR_ILLEGAL_PBITS                                         = 300,
    BL_LILAC_RDD_ERROR_ILLEGAL_TRAFFIC_CLASS                                 = 301,
    BL_LILAC_RDD_ERROR_ILLEGAL_WAN_MAPPING_TABLE_INDEX                       = 302,
    BL_LILAC_RDD_ERROR_ILLEGAL_PBITS_TO_WAN_FLOW_MAPPING_TABLE               = 303,
    BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY                                     = 350,
    BL_LILAC_RDD_ERROR_CONTEXT_ENTRY_INVALID                                 = 351,
    BL_LILAC_RDD_ERROR_ILLEGAL_SUBNET_ID                                     = 360,
    BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_EXISTS                               = 400,
    BL_LILAC_RDD_ERROR_IPTV_FORWARDING_TABLE_FULL                            = 401,
    BL_LILAC_RDD_ERROR_IPTV_SRC_IP_COUNTER_NOT_ZERO                          = 402,
    BL_LILAC_RDD_ERROR_IPTV_WITH_SRC_IP_ANY_EXISTS                           = 403,
    BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_NOT_EXISTS                           = 404,
    BL_LILAC_RDD_ERROR_ILLEGAL_IPTV_TABLE_CACHE_SIZE                         = 405,
    BL_LILAC_RDD_ERROR_IPTV_SRC_IP_TABLE_FULL                                = 406,
    BL_LILAC_RDD_ERROR_IPTV_CONTEXT_TABLES_TABLE_FULL                        = 407,
    BL_LILAC_RDD_ERROR_DDR_CONTEXT_TABLE_TABLE_FULL                          = 408,
    BL_LILAC_RDD_ERROR_TIMER_TASK_TABLE_FULL                                 = 440,
    BL_LILAC_RDD_ERROR_TIMER_TASK_PERIOD                                     = 441,
    BL_LILAC_RDD_ERROR_NO_FREE_SKB                                           = 450,
    BL_LILAC_RDD_ERROR_1588_TX                                               = 451,
    BL_LILAC_RDD_ERROR_NO_FREE_GSODESC                                       = 452,
    BL_LILAC_RDD_ERROR_SPDSVC_RESOURCE_BUSY                                  = 453,
    BL_LILAC_RDD_ERROR_INGRESS_RATE_LIMITER_BUDGET_TOO_LARGE                 = 460,
    BL_LILAC_RDD_ERROR_INGRESS_RATE_LIMITER_FLOW_CONTROL_THRESHOLD_TOO_LARGE = 461,
    BL_LILAC_RDD_ERROR_GPON_SNIFFER_NULL_PD_PTR                              = 500,
    BL_LILAC_RDD_ERROR_SMC_PPS_SEND_LEN_LESS_THEN_3                          = 600,
    BL_LILAC_RDD_ERROR_SMC_INVALID_SEND_LENGTH                               = 601,
    BL_LILAC_RDD_ERROR_SMC_INVALID_RECEIVE_LENGTH                            = 602,
    BL_LILAC_RDD_ERROR_MTU_INVALID_LENGTH                                    = 603,
}
BL_LILAC_RDD_ERROR_DTE;


typedef enum
{
    BL_LILAC_RDD_WAN_BRIDGE_PORT            = 0,
    BL_LILAC_RDD_LAN0_BRIDGE_PORT           = 1,
    BL_LILAC_RDD_LAN1_BRIDGE_PORT           = 2,
    BL_LILAC_RDD_LAN2_BRIDGE_PORT           = 3,
    BL_LILAC_RDD_LAN3_BRIDGE_PORT           = 4,
    BL_LILAC_RDD_LAN4_BRIDGE_PORT           = 5,
    BL_LILAC_RDD_WAN_ROUTER_PORT            = 6,
    BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT       = 7,
    BL_LILAC_RDD_PCI_BRIDGE_PORT            = 8,
    BL_LILAC_RDD_G9991_BRIDGE_PORT          = 8,
    BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT        = 9,
    BL_LILAC_RDD_CPU_BRIDGE_PORT            = 12,
    BL_LILAC_RDD_WAN_QUASI_BRIDGE_PORT      = 15,
    BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT = 0x10,
    BL_LILAC_RDD_MULTICAST_LAN1_BRIDGE_PORT = 0x20,
    BL_LILAC_RDD_MULTICAST_LAN2_BRIDGE_PORT = 0x40,
    BL_LILAC_RDD_MULTICAST_LAN3_BRIDGE_PORT = 0x80,
    BL_LILAC_RDD_MULTICAST_LAN4_BRIDGE_PORT = 0x100,
    BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT  = 0x200,
}
BL_LILAC_RDD_BRIDGE_PORT_DTE;


typedef enum
{
    BL_LILAC_RDD_WAN_PHYSICAL_PORT_FIBER = 0,
    BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH4  = 1,
    BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH0  = 3,
    BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH1 = 5,
    BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH2 = 6,
    BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH3 = 7,
}
BL_LILAC_RDD_WAN_PHYSICAL_PORT_DTE;

typedef enum
{
    rdd_wan_gpon  = 0,
    rdd_wan_epon  = 1,
    rdd_wan_ae    = 2,
}
rdd_wan_mode_t;

typedef enum
{
    BL_LILAC_RDD_EMAC_ID_START = 0,
#ifdef G9991
    BL_LILAC_RDD_EMAC_ID_0   = 0,
    BL_LILAC_RDD_EMAC_ID_1   = 1,
    BL_LILAC_RDD_EMAC_ID_2   = 2,
    BL_LILAC_RDD_EMAC_ID_3   = 3,
    BL_LILAC_RDD_EMAC_ID_4   = 4,
    BL_LILAC_RDD_EMAC_ID_5   = 5,
    BL_LILAC_RDD_EMAC_ID_6   = 6,
    BL_LILAC_RDD_EMAC_ID_7   = 7,
    BL_LILAC_RDD_EMAC_ID_8   = 8,
    BL_LILAC_RDD_EMAC_ID_9   = 9,
    BL_LILAC_RDD_EMAC_ID_10   = 10,
    BL_LILAC_RDD_EMAC_ID_11   = 11,
    BL_LILAC_RDD_EMAC_ID_12   = 12,
    BL_LILAC_RDD_EMAC_ID_13   = 13,
    BL_LILAC_RDD_EMAC_ID_14   = 14,
    BL_LILAC_RDD_EMAC_ID_15   = 15,
    BL_LILAC_RDD_EMAC_ID_16   = 16,
    BL_LILAC_RDD_EMAC_ID_17   = 17,
    BL_LILAC_RDD_EMAC_ID_18   = 18,
    BL_LILAC_RDD_EMAC_ID_19   = 19,
    BL_LILAC_RDD_EMAC_ID_20   = 20,
    BL_LILAC_RDD_EMAC_ID_21   = 21,
    BL_LILAC_RDD_EMAC_ID_22   = 22,
    BL_LILAC_RDD_EMAC_ID_23   = 23,
    BL_LILAC_RDD_EMAC_ID_24   = 24,
    BL_LILAC_RDD_EMAC_ID_25   = 25,
    BL_LILAC_RDD_EMAC_ID_26   = 26,
    BL_LILAC_RDD_EMAC_ID_27   = 27,
    BL_LILAC_RDD_EMAC_ID_28   = 28,
    BL_LILAC_RDD_EMAC_ID_29   = 29,
#else
    BL_LILAC_RDD_EMAC_ID_PCI  = 0,
    BL_LILAC_RDD_EMAC_ID_0    = 1,
    BL_LILAC_RDD_EMAC_ID_1    = 2,
    BL_LILAC_RDD_EMAC_ID_2    = 3,
    BL_LILAC_RDD_EMAC_ID_3    = 4,
    BL_LILAC_RDD_EMAC_ID_4    = 5,
#endif
    BL_LILAC_RDD_EMAC_ID_COUNT   ,
}
BL_LILAC_RDD_EMAC_ID_DTE;

static inline int rdd_emac_prt_to_vector(BL_LILAC_RDD_EMAC_ID_DTE emac, bdmf_boolean is_iptv)
{
#ifndef G9991
    if(is_iptv)
        return emac == BL_LILAC_RDD_EMAC_ID_PCI ? 1LL << 5 : 1LL << (emac - 1);
#endif
    return 1LL << emac; 
}
#define RDD_EMAC_PORT_TO_VECTOR(rdd_emac_id,is_iptv) rdd_emac_prt_to_vector(rdd_emac_id, is_iptv)

typedef enum
{
    RDD_WAN_CHANNEL_UNASSIGNED = -1,
    RDD_WAN_CHANNEL_0  = 0,
    RDD_WAN_CHANNEL_1  = 1,
    RDD_WAN_CHANNEL_2  = 2,
    RDD_WAN_CHANNEL_3  = 3,
    RDD_WAN_CHANNEL_4  = 4,
    RDD_WAN_CHANNEL_5  = 5,
    RDD_WAN_CHANNEL_6  = 6,
    RDD_WAN_CHANNEL_7  = 7,
    RDD_WAN_CHANNEL_8  = 8,
    RDD_WAN_CHANNEL_9  = 9,
    RDD_WAN_CHANNEL_10 = 10,
    RDD_WAN_CHANNEL_11 = 11,
    RDD_WAN_CHANNEL_12 = 12,
    RDD_WAN_CHANNEL_13 = 13,
    RDD_WAN_CHANNEL_14 = 14,
    RDD_WAN_CHANNEL_15 = 15,
    RDD_WAN_CHANNEL_16 = 16,
    RDD_WAN_CHANNEL_17 = 17,
    RDD_WAN_CHANNEL_18 = 18,
    RDD_WAN_CHANNEL_19 = 19,
    RDD_WAN_CHANNEL_20 = 20,
    RDD_WAN_CHANNEL_21 = 21,
    RDD_WAN_CHANNEL_22 = 22,
    RDD_WAN_CHANNEL_23 = 23,
    RDD_WAN_CHANNEL_24 = 24,
    RDD_WAN_CHANNEL_25 = 25,
    RDD_WAN_CHANNEL_26 = 26,
    RDD_WAN_CHANNEL_27 = 27,
    RDD_WAN_CHANNEL_28 = 28,
    RDD_WAN_CHANNEL_29 = 29,
    RDD_WAN_CHANNEL_30 = 30,
    RDD_WAN_CHANNEL_31 = 31,
    RDD_WAN_CHANNEL_32 = 32,
    RDD_WAN_CHANNEL_33 = 33,
    RDD_WAN_CHANNEL_34 = 34,
    RDD_WAN_CHANNEL_35 = 35,
    RDD_WAN_CHANNEL_36 = 36,
    RDD_WAN_CHANNEL_37 = 37,
    RDD_WAN_CHANNEL_38 = 38,
    RDD_WAN_CHANNEL_39 = 39,
}
RDD_WAN_CHANNEL_ID;


typedef enum
{
    BL_LILAC_RDD_RATE_CONTROLLER_UNASSIGNED = -1,
    BL_LILAC_RDD_RATE_CONTROLLER_0  = 0,
    BL_LILAC_RDD_RATE_CONTROLLER_1  = 1,
    BL_LILAC_RDD_RATE_CONTROLLER_2  = 2,
    BL_LILAC_RDD_RATE_CONTROLLER_3  = 3,
    BL_LILAC_RDD_RATE_CONTROLLER_4  = 4,
    BL_LILAC_RDD_RATE_CONTROLLER_5  = 5,
    BL_LILAC_RDD_RATE_CONTROLLER_6  = 6,
    BL_LILAC_RDD_RATE_CONTROLLER_7  = 7,
    BL_LILAC_RDD_RATE_CONTROLLER_8  = 8,
    BL_LILAC_RDD_RATE_CONTROLLER_9  = 9,
    BL_LILAC_RDD_RATE_CONTROLLER_10 = 10,
    BL_LILAC_RDD_RATE_CONTROLLER_11 = 11,
    BL_LILAC_RDD_RATE_CONTROLLER_12 = 12,
    BL_LILAC_RDD_RATE_CONTROLLER_13 = 13,
    BL_LILAC_RDD_RATE_CONTROLLER_14 = 14,
    BL_LILAC_RDD_RATE_CONTROLLER_15 = 15,
    BL_LILAC_RDD_RATE_CONTROLLER_16 = 16,
    BL_LILAC_RDD_RATE_CONTROLLER_17 = 17,
    BL_LILAC_RDD_RATE_CONTROLLER_18 = 18,
    BL_LILAC_RDD_RATE_CONTROLLER_19 = 19,
    BL_LILAC_RDD_RATE_CONTROLLER_20 = 20,
    BL_LILAC_RDD_RATE_CONTROLLER_21 = 21,
    BL_LILAC_RDD_RATE_CONTROLLER_22 = 22,
    BL_LILAC_RDD_RATE_CONTROLLER_23 = 23,
    BL_LILAC_RDD_RATE_CONTROLLER_24 = 24,
    BL_LILAC_RDD_RATE_CONTROLLER_25 = 25,
    BL_LILAC_RDD_RATE_CONTROLLER_26 = 26,
    BL_LILAC_RDD_RATE_CONTROLLER_27 = 27,
    BL_LILAC_RDD_RATE_CONTROLLER_28 = 28,
    BL_LILAC_RDD_RATE_CONTROLLER_29 = 29,
    BL_LILAC_RDD_RATE_CONTROLLER_30 = 30,
    BL_LILAC_RDD_RATE_CONTROLLER_31 = 31,
    BL_LILAC_RDD_RATE_CONTROLLER_32 = 32,
    BL_LILAC_RDD_RATE_CONTROLLER_33 = 33,
    BL_LILAC_RDD_RATE_CONTROLLER_34 = 34,
    BL_LILAC_RDD_RATE_CONTROLLER_35 = 35,
    BL_LILAC_RDD_RATE_CONTROLLER_36 = 36,
    BL_LILAC_RDD_RATE_CONTROLLER_37 = 37,
    BL_LILAC_RDD_RATE_CONTROLLER_38 = 38,
    BL_LILAC_RDD_RATE_CONTROLLER_39 = 39,
    BL_LILAC_RDD_RATE_CONTROLLER_40 = 40,
    BL_LILAC_RDD_RATE_CONTROLLER_41 = 41,
    BL_LILAC_RDD_RATE_CONTROLLER_42 = 42,
    BL_LILAC_RDD_RATE_CONTROLLER_43 = 43,
    BL_LILAC_RDD_RATE_CONTROLLER_44 = 44,
    BL_LILAC_RDD_RATE_CONTROLLER_45 = 45,
    BL_LILAC_RDD_RATE_CONTROLLER_46 = 46,
    BL_LILAC_RDD_RATE_CONTROLLER_47 = 47,
    BL_LILAC_RDD_RATE_CONTROLLER_48 = 48,
    BL_LILAC_RDD_RATE_CONTROLLER_49 = 49,
    BL_LILAC_RDD_RATE_CONTROLLER_50 = 50,
    BL_LILAC_RDD_RATE_CONTROLLER_51 = 51,
    BL_LILAC_RDD_RATE_CONTROLLER_52 = 52,
    BL_LILAC_RDD_RATE_CONTROLLER_53 = 53,
    BL_LILAC_RDD_RATE_CONTROLLER_54 = 54,
    BL_LILAC_RDD_RATE_CONTROLLER_55 = 55,
    BL_LILAC_RDD_RATE_CONTROLLER_56 = 56,
    BL_LILAC_RDD_RATE_CONTROLLER_57 = 57,
    BL_LILAC_RDD_RATE_CONTROLLER_58 = 58,
    BL_LILAC_RDD_RATE_CONTROLLER_59 = 59,
    BL_LILAC_RDD_RATE_CONTROLLER_60 = 60,
    BL_LILAC_RDD_RATE_CONTROLLER_61 = 61,
    BL_LILAC_RDD_RATE_CONTROLLER_62 = 62,
    BL_LILAC_RDD_RATE_CONTROLLER_63 = 63,
    BL_LILAC_RDD_RATE_CONTROLLER_64 = 64,
    BL_LILAC_RDD_RATE_CONTROLLER_65 = 65,
    BL_LILAC_RDD_RATE_CONTROLLER_66 = 66,
    BL_LILAC_RDD_RATE_CONTROLLER_67 = 67,
    BL_LILAC_RDD_RATE_CONTROLLER_68 = 68,
    BL_LILAC_RDD_RATE_CONTROLLER_69 = 69,
    BL_LILAC_RDD_RATE_CONTROLLER_70 = 70,
    BL_LILAC_RDD_RATE_CONTROLLER_71 = 71,
    BL_LILAC_RDD_RATE_CONTROLLER_72 = 72,
    BL_LILAC_RDD_RATE_CONTROLLER_73 = 73,
    BL_LILAC_RDD_RATE_CONTROLLER_74 = 74,
    BL_LILAC_RDD_RATE_CONTROLLER_75 = 75,
    BL_LILAC_RDD_RATE_CONTROLLER_76 = 76,
    BL_LILAC_RDD_RATE_CONTROLLER_77 = 77,
    BL_LILAC_RDD_RATE_CONTROLLER_78 = 78,
    BL_LILAC_RDD_RATE_CONTROLLER_79 = 79,
    BL_LILAC_RDD_RATE_CONTROLLER_80 = 80,
    BL_LILAC_RDD_RATE_CONTROLLER_81 = 81,
    BL_LILAC_RDD_RATE_CONTROLLER_82 = 82,
    BL_LILAC_RDD_RATE_CONTROLLER_83 = 83,
    BL_LILAC_RDD_RATE_CONTROLLER_84 = 84,
    BL_LILAC_RDD_RATE_CONTROLLER_85 = 85,
    BL_LILAC_RDD_RATE_CONTROLLER_86 = 86,
    BL_LILAC_RDD_RATE_CONTROLLER_87 = 87,
    BL_LILAC_RDD_RATE_CONTROLLER_88 = 88,
    BL_LILAC_RDD_RATE_CONTROLLER_89 = 89,
    BL_LILAC_RDD_RATE_CONTROLLER_90 = 90,
    BL_LILAC_RDD_RATE_CONTROLLER_91 = 91,
    BL_LILAC_RDD_RATE_CONTROLLER_92 = 92,
    BL_LILAC_RDD_RATE_CONTROLLER_93 = 93,
    BL_LILAC_RDD_RATE_CONTROLLER_94 = 94,
    BL_LILAC_RDD_RATE_CONTROLLER_95 = 95,
    BL_LILAC_RDD_RATE_CONTROLLER_96 = 96,
    BL_LILAC_RDD_RATE_CONTROLLER_97 = 97,
    BL_LILAC_RDD_RATE_CONTROLLER_98 = 98,
    BL_LILAC_RDD_RATE_CONTROLLER_99 = 99,
    BL_LILAC_RDD_RATE_CONTROLLER_100 = 100,
    BL_LILAC_RDD_RATE_CONTROLLER_101 = 101,
    BL_LILAC_RDD_RATE_CONTROLLER_102 = 102,
    BL_LILAC_RDD_RATE_CONTROLLER_103 = 103,
    BL_LILAC_RDD_RATE_CONTROLLER_104 = 104,
    BL_LILAC_RDD_RATE_CONTROLLER_105 = 105,
    BL_LILAC_RDD_RATE_CONTROLLER_106 = 106,
    BL_LILAC_RDD_RATE_CONTROLLER_107 = 107,
    BL_LILAC_RDD_RATE_CONTROLLER_108 = 108,
    BL_LILAC_RDD_RATE_CONTROLLER_109 = 109,
    BL_LILAC_RDD_RATE_CONTROLLER_110 = 110,
    BL_LILAC_RDD_RATE_CONTROLLER_111 = 111,
    BL_LILAC_RDD_RATE_CONTROLLER_112 = 112,
    BL_LILAC_RDD_RATE_CONTROLLER_113 = 113,
    BL_LILAC_RDD_RATE_CONTROLLER_114 = 114,
    BL_LILAC_RDD_RATE_CONTROLLER_115 = 115,
    BL_LILAC_RDD_RATE_CONTROLLER_116 = 116,
    BL_LILAC_RDD_RATE_CONTROLLER_117 = 117,
    BL_LILAC_RDD_RATE_CONTROLLER_118 = 118,
    BL_LILAC_RDD_RATE_CONTROLLER_119 = 119,
    BL_LILAC_RDD_RATE_CONTROLLER_120 = 120,
    BL_LILAC_RDD_RATE_CONTROLLER_121 = 121,
    BL_LILAC_RDD_RATE_CONTROLLER_122 = 122,
    BL_LILAC_RDD_RATE_CONTROLLER_123 = 123,
    BL_LILAC_RDD_RATE_CONTROLLER_124 = 124,
    BL_LILAC_RDD_RATE_CONTROLLER_125 = 125,
    BL_LILAC_RDD_RATE_CONTROLLER_126 = 126,
    BL_LILAC_RDD_RATE_CONTROLLER_127 = 127,
}
BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE;


typedef enum
{
    BL_LILAC_RDD_QUEUE_0 = 0,
    BL_LILAC_RDD_QUEUE_1 = 1,
    BL_LILAC_RDD_QUEUE_2 = 2,
    BL_LILAC_RDD_QUEUE_3 = 3,
    BL_LILAC_RDD_QUEUE_4 = 4,
    BL_LILAC_RDD_QUEUE_5 = 5,
    BL_LILAC_RDD_QUEUE_6 = 6,
    BL_LILAC_RDD_QUEUE_7 = 7,
#ifndef G9991
    BL_LILAC_RDD_QUEUE_LAST = 7,
#else
    BL_LILAC_RDD_QUEUE_LAST = 3,
#endif
}
BL_LILAC_RDD_QUEUE_ID_DTE;


typedef enum
{
    RDD_WAN_CHANNEL_SCHEDULE_PRIORITY     = 0,
    RDD_WAN_CHANNEL_SCHEDULE_RATE_CONTROL = 1,
}
RDD_WAN_CHANNEL_SCHEDULE;


typedef enum
{
    BL_LILAC_RDD_CPU_RX_QUEUE_0 = 0,
    BL_LILAC_RDD_CPU_RX_QUEUE_1 = 1,
    BL_LILAC_RDD_CPU_RX_QUEUE_2 = 2,
    BL_LILAC_RDD_CPU_RX_QUEUE_3 = 3,
    BL_LILAC_RDD_CPU_RX_QUEUE_4 = 4,
    BL_LILAC_RDD_CPU_RX_QUEUE_5 = 5,
    BL_LILAC_RDD_CPU_RX_QUEUE_6 = 6,
    BL_LILAC_RDD_CPU_RX_QUEUE_7 = 7,
}
BL_LILAC_RDD_CPU_RX_QUEUE_DTE;


typedef enum
{
    BL_LILAC_RDD_PCI_TX_QUEUE_0 = 8,
    BL_LILAC_RDD_PCI_TX_QUEUE_1 = 9,
    BL_LILAC_RDD_PCI_TX_QUEUE_2 = 10,
    BL_LILAC_RDD_PCI_TX_QUEUE_3 = 11,
}
BL_LILAC_RDD_PCI_TX_QUEUE_DTE;


typedef enum
{
    BL_LILAC_RDD_CPU_METER_0       = 0,
    BL_LILAC_RDD_CPU_METER_1       = 1,
    BL_LILAC_RDD_CPU_METER_2       = 2,
    BL_LILAC_RDD_CPU_METER_3       = 3,
    BL_LILAC_RDD_CPU_METER_4       = 4,
    BL_LILAC_RDD_CPU_METER_5       = 5,
    BL_LILAC_RDD_CPU_METER_6       = 6,
    BL_LILAC_RDD_CPU_METER_7       = 7,
    BL_LILAC_RDD_CPU_METER_8       = 8,
    BL_LILAC_RDD_CPU_METER_9       = 9,
    BL_LILAC_RDD_CPU_METER_10      = 10,
    BL_LILAC_RDD_CPU_METER_11      = 11,
    BL_LILAC_RDD_CPU_METER_12      = 12,
    BL_LILAC_RDD_CPU_METER_13      = 13,
    BL_LILAC_RDD_CPU_METER_14      = 14,
    BL_LILAC_RDD_CPU_METER_15      = 15,
    BL_LILAC_RDD_CPU_METER_DISABLE = 16,
}
BL_LILAC_RDD_CPU_METER_DTE;


typedef enum
{
    BL_LILAC_RDD_STATIC_MAC_ADDRESS = 0,
    BL_LILAC_RDD_BRIDGE_MAC_ADDRESS = 1,
}
BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE;


typedef enum
{
    BL_LILAC_RDD_DISABLED = 0,
    BL_LILAC_RDD_ENABLED  = 1,
}
BL_LILAC_RDD_CONTROL_DTE;


typedef enum
{
    BL_LILAC_RDD_PPPOE_DISABLED = 0,
    BL_LILAC_RDD_PPPOE_ENABLED  = 1,
}
BL_LILAC_RDD_PPPOE_ENABLE_DTE;


typedef enum
{
    BL_LILAC_RDD_DUAL_STACK_LITE_DISABLED = 0,
    BL_LILAC_RDD_DUAL_STACK_LITE_ENABLED  = 1,
}
BL_LILAC_RDD_DUAL_STACK_LITE_ENABLE_DTE;


typedef enum
{
    BL_LILAC_RDD_IPV6_DISABLED = 0,
    BL_LILAC_RDD_IPV6_ENABLED  = 1,
}
BL_LILAC_RDD_IPV6_ENABLE_DTE;


typedef enum
{
    BL_LILAC_RDD_FORWARD_DISABLE = 0,
    BL_LILAC_RDD_FORWARD_ENABLE  = 1,
}
BL_LILAC_RDD_FORWARD_MATRIX_DTE;


typedef enum
{
    BL_LILAC_RDD_VLAN_SWITCHING_DISABLE = 0,
    BL_LILAC_RDD_VLAN_SWITCHING_ENABLE  = 1,
}
BL_LILAC_RDD_VLAN_SWITCHING_CONFIG_DTE;


typedef enum
{
    BL_LILAC_RDD_VLAN_BINDING_DISABLE = 0,
    BL_LILAC_RDD_VLAN_BINDING_ENABLE  = 1,
}
BL_LILAC_RDD_VLAN_BINDING_CONFIG_DTE;


typedef enum
{
    BL_LILAC_RDD_AGGREGATION_MODE_DISABLED = 0,
    BL_LILAC_RDD_AGGREGATION_MODE_ENABLED  = 1,
}
BL_LILAC_RDD_AGGREGATION_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_PCI  = 0x1,
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN0 = 0x2,
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN1 = 0x4,
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN2 = 0x8,
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN3 = 0x10,
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN4 = 0x20,
}
BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE;


typedef enum
{
    BL_LILAC_RDD_SUBNET_CLASSIFY_ETHERNET_FLOW = 0,
    BL_LILAC_RDD_SUBNET_CLASSIFY_MAC_FILTER    = 1,
}
BL_LILAC_RDD_SUBNET_CLASSIFY_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_SUBNET_FLOW_CACHE  = 0,
    BL_LILAC_RDD_SUBNET_BRIDGE      = 1,
    BL_LILAC_RDD_SUBNET_BRIDGE_IPTV = 2,
    BL_LILAC_RDD_SUBNET_LAN         = 2,
}
BL_LILAC_RDD_SUBNET_ID_DTE;


typedef enum
{
    BL_LILAC_RDD_DS_RATE_CONTROL_DISABLE = 0,
    BL_LILAC_RDD_DS_RATE_CONTROL_ENABLE  = 1,
}
BL_LILAC_RDD_DS_RATE_CONTROL_MODE_DTE;

#ifndef G9991
typedef enum
{
    RDD_RATE_LIMITER_EMAC_0                 = 0,
    RDD_RATE_LIMITER_EMAC_1                 = 1,
    RDD_RATE_LIMITER_EMAC_2                 = 2,
    RDD_RATE_LIMITER_EMAC_3                 = 3,
    RDD_RATE_LIMITER_EMAC_4                 = 4,
    RDD_RATE_LIMITER_EMAC_5                 = 5,
    RDD_RATE_LIMITER_EMAC_LAST              = 5,
    RDD_RATE_LIMITER_SERVICE_QUEUE_0        = 6,
    RDD_RATE_LIMITER_SERVICE_QUEUE_1        = 7,
    RDD_RATE_LIMITER_SERVICE_QUEUE_2        = 8,
    RDD_RATE_LIMITER_SERVICE_QUEUE_3        = 9,
    RDD_RATE_LIMITER_SERVICE_QUEUE_4        = 10,
    RDD_RATE_LIMITER_SERVICE_QUEUE_5        = 11,
    RDD_RATE_LIMITER_SERVICE_QUEUE_6        = 12,
    RDD_RATE_LIMITER_SERVICE_QUEUE_7        = 13,
    RDD_RATE_LIMITER_SERVICE_QUEUE_OVERALL  = 14,
    RDD_RATE_LIMITER_IDLE                   = 15,
}
RDD_RATE_LIMITER_ID_DTE;
#else
typedef enum
{
    RDD_RATE_LIMITER_EMAC_0     = 0,
    RDD_RATE_LIMITER_EMAC_1     = 1,
    RDD_RATE_LIMITER_EMAC_2     = 2,
    RDD_RATE_LIMITER_EMAC_3     = 3,
    RDD_RATE_LIMITER_EMAC_4     = 4,
    RDD_RATE_LIMITER_EMAC_5     = 5,
    RDD_RATE_LIMITER_EMAC_6     = 6,
    RDD_RATE_LIMITER_EMAC_7     = 7,
    RDD_RATE_LIMITER_EMAC_8     = 8,
    RDD_RATE_LIMITER_EMAC_9     = 9,
    RDD_RATE_LIMITER_EMAC_10    = 10,
    RDD_RATE_LIMITER_EMAC_11    = 11,
    RDD_RATE_LIMITER_EMAC_12    = 12,
    RDD_RATE_LIMITER_EMAC_13    = 13,
    RDD_RATE_LIMITER_EMAC_14    = 14,
    RDD_RATE_LIMITER_EMAC_15    = 15,
    RDD_RATE_LIMITER_EMAC_16    = 16,
    RDD_RATE_LIMITER_EMAC_17    = 17,
    RDD_RATE_LIMITER_EMAC_18    = 18,
    RDD_RATE_LIMITER_EMAC_19    = 19,
    RDD_RATE_LIMITER_EMAC_20    = 20,
    RDD_RATE_LIMITER_EMAC_21    = 21,
    RDD_RATE_LIMITER_EMAC_22    = 22,
    RDD_RATE_LIMITER_EMAC_23    = 23,
    RDD_RATE_LIMITER_EMAC_LAST  = 23,
    RDD_RATE_LIMITER_IDLE       = 24,
}
RDD_RATE_LIMITER_ID_DTE;
#endif

typedef enum
{
    BL_LILAC_RDD_POLICER_0        = 0,
    BL_LILAC_RDD_POLICER_1        = 1,
    BL_LILAC_RDD_POLICER_2        = 2,
    BL_LILAC_RDD_POLICER_3        = 3,
    BL_LILAC_RDD_POLICER_4        = 4,
    BL_LILAC_RDD_POLICER_5        = 5,
    BL_LILAC_RDD_POLICER_6        = 6,
    BL_LILAC_RDD_POLICER_7        = 7,
    BL_LILAC_RDD_POLICER_8        = 8,
    BL_LILAC_RDD_POLICER_9        = 9,
    BL_LILAC_RDD_POLICER_10       = 10,
    BL_LILAC_RDD_POLICER_11       = 11,
    BL_LILAC_RDD_POLICER_12       = 12,
    BL_LILAC_RDD_POLICER_13       = 13,
    BL_LILAC_RDD_POLICER_14       = 14,
    BL_LILAC_RDD_POLICER_15       = 15,
    BL_LILAC_RDD_POLICER_DISABLED = 16,
}
BL_LILAC_RDD_POLICER_ID_DTE;


typedef struct
{
    uint16_t    vid;
    uint16_t    wlan_mcast_index;
    uint32_t    egress_port_vector;
    uint8_t     ingress_classification_context;
    bdmf_mac_t  mac_addr;
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_IPTV_L2_ENTRY_DTE;



#define BL_LILAC_RDD_IPV4_ADDRESS_BYTE_SIZE             ( 4 )
#define BL_LILAC_RDD_IPV6_ADDRESS_BYTE_SIZE             ( 16 )
#define RDD_FLOW_CACHE_L2_HEADER_BYTE_SIZE     ( 32 )

typedef struct
{
    uint16_t   vid;
    uint16_t   wlan_mcast_index;
    uint32_t   egress_port_vector;
    uint8_t    ingress_classification_context;
    bdmf_ip_t  dst_ip;
    bdmf_ip_t  src_ip;
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_IPTV_L3_ENTRY_DTE;


typedef union
{
    BL_LILAC_RDD_IPTV_L2_ENTRY_DTE  l2_entry_fields;
    BL_LILAC_RDD_IPTV_L3_ENTRY_DTE  l3_entry_fields;
}
RDD_IPTV_ENTRY_UNION;


typedef enum
{
    BL_LILAC_RDD_FILTER_DISABLE = 0,
    BL_LILAC_RDD_FILTER_ENABLE  = 1,
}
BL_LILAC_RDD_FILTER_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_RATE_LIMITER_DISABLE = 0,
    BL_LILAC_RDD_RATE_LIMITER_ENABLE  = 1,
}
BL_LILAC_RDD_RATE_LIMITER_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_RATE_LIMITER_LOW  = 0,
    BL_LILAC_RDD_RATE_LIMITER_HIGH = 1,
}
BL_LILAC_RDD_RATE_LIMITER_PRIORITY_DTE;


typedef enum
{
    BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP = 1,
    BL_LILAC_RDD_FILTER_ACTION_DROP     = 2,
}
BL_LILAC_RDD_FILTER_ACTION_DTE;


typedef enum
{
    BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0      = 2,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_1      = 3,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_2      = 4,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_3      = 5,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_PPPOE_D     = 6,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_PPPOE_S     = 7,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_ARP         = 8,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_1588        = 9,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1X      = 10,
    BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1AG_CFM = 11,
}
BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE;


typedef enum
{
    BL_LILAC_RDD_MAC_LOOKUP_DISABLE = 0,
    BL_LILAC_RDD_MAC_LOOKUP_ENABLE  = 1,
}
BL_LILAC_RDD_MAC_LOOKUP_DTE;


typedef enum
{
    BL_LILAC_RDD_FLOW_BASED_FORWARDING_DISABLED = 0,
    BL_LILAC_RDD_FLOW_BASED_FORWARDING_ENABLED  = 1,
}
BL_LILAC_RDD_FLOW_BASED_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_MAC_FWD_ACTION_FORWARD    = 0,
    BL_LILAC_RDD_MAC_FWD_ACTION_DROP       = 1,
    BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP0  = 2,
    BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP1  = 3,
    BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP2  = 4,
    BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP3  = 5,
    BL_LILAC_RDD_MAC_FWD_ACTION_RATE_LIMIT = 6,
}
BL_LILAC_RDD_MAC_FWD_ACTION_DTE;


typedef enum
{
    BL_LILAC_RDD_ACL_LAYER3_FILTER_DISABLE                      = 0,
    BL_LILAC_RDD_ACL_LAYER3_FILTER_SRC_IP_INCLUSIVE             = 2,
    BL_LILAC_RDD_ACL_LAYER3_FILTER_SRC_IP_EXCLUSIVE             = 3,
    BL_LILAC_RDD_ACL_LAYER3_FILTER_SRC_MAC_SRC_IP_INCLUSIVE     = 4,
    BL_LILAC_RDD_ACL_LAYER3_FILTER_SRC_MAC_SRC_IP_EXCLUSIVE     = 5,
    BL_LILAC_RDD_ACL_LAYER3_FILTER_SRC_MAC_VID_SRC_IP_INCLUSIVE = 8,
    BL_LILAC_RDD_ACL_LAYER3_FILTER_SRC_MAC_VID_SRC_IP_EXCLUSIVE = 9,
}
BL_LILAC_RDD_ACL_LAYER3_FILTER_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_CRC_CALC_DISABLE = 0,
    BL_LILAC_RDD_CRC_CALC_ENABLE  = 1,
}
BL_LILAC_RDD_TX_CRC_CALC_DTE;


typedef enum
{
    rdd_vlan_command_transparent                         = 0,
    rdd_vlan_command_add_tag                             = 1,
    rdd_vlan_command_remove_tag                          = 2,
    rdd_vlan_command_replace_tag                         = 3,
    rdd_vlan_command_add_two_tags                        = 4,
    rdd_vlan_command_remove_two_tags                     = 5,
    rdd_vlan_command_add_outer_tag_replace_inner_tag     = 6,
    rdd_vlan_command_remove_outer_tag_replace_inner_tag  = 7,
    rdd_vlan_command_add_tag_always                      = 8,
    rdd_vlan_command_remove_tag_always                   = 9,
    rdd_vlan_command_replace_outer_tag_replace_inner_tag = 10,
    rdd_vlan_command_remove_outer_tag_copy               = 11,
    rdd_vlan_command_add_3rd_tag                         = 12,
    rdd_max_vlan_command                                 = 13,
}
rdd_bridge_vlan_command;


typedef enum
{
    rdd_pbits_command_transparent = 0,
    rdd_pbits_command_copy        = 1,
    rdd_pbits_command_configured  = 2,
    rdd_pbits_command_remap       = 3,
    rdd_max_pbits_command         = 4,
}
rdd_bridge_pbits_command;


typedef enum
{
    rdd_tpid_id_0   = 0,
    rdd_tpid_id_1   = 1,
    rdd_tpid_id_2   = 2,
    rdd_tpid_id_3   = 3,
    rdd_tpid_id_4   = 4,
    rdd_tpid_id_5   = 5,
    rdd_tpid_id_6   = 6,
    rdd_tpid_id_7   = 7,
}
rdd_tpid_id;


typedef enum
{
    BL_LILAC_RDD_UNKNOWN_MAC_CMD_FORWARD  = 1,
    BL_LILAC_RDD_UNKNOWN_MAC_CMD_CPU_TRAP = 2,
    BL_LILAC_RDD_UNKNOWN_MAC_CMD_DROP     = 4,
    BL_LILAC_RDD_UNKNOWN_MAC_CMD_FLOOD    = 8,
}
BL_LILAC_RDD_UNKNOWN_MAC_COMMAND_DTE;


typedef enum
{
    BL_LILAC_RDD_MAC_TABLE_SIZE_32   = 0,
    BL_LILAC_RDD_MAC_TABLE_SIZE_64   = 1,
    BL_LILAC_RDD_MAC_TABLE_SIZE_128  = 2,
    BL_LILAC_RDD_MAC_TABLE_SIZE_256  = 3,
    BL_LILAC_RDD_MAC_TABLE_SIZE_512  = 4,
    BL_LILAC_RDD_MAC_TABLE_SIZE_1024 = 5,
    BL_LILAC_RDD_MAC_TABLE_SIZE_2048 = 6,
    BL_LILAC_RDD_MAC_TABLE_SIZE_4096 = 7,
}
BL_LILAC_RDD_MAC_TABLE_SIZE_DTE;


typedef enum
{
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_1    = 0,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_2    = 1,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_4    = 2,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_8    = 3,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_16   = 4,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_32   = 5,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_64   = 6,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_128  = 7,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_256  = 8,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_512  = 9,
    BL_LILAC_RDD_MAC_TABLE_MAX_HOP_1024 = 10,
}
BL_LILAC_RDD_MAC_TABLE_MAX_HOP_DTE;


typedef enum
{
    BL_LILAC_RDD_IGMP_FILTER_NUMBER        = 0,
    BL_LILAC_RDD_ICMPV6_FILTER_NUMBER      = 1,
    BL_LILAC_RDD_USER_0_FILTER_NUMBER      = 2,
    BL_LILAC_RDD_USER_1_FILTER_NUMBER      = 3,
    BL_LILAC_RDD_USER_2_FILTER_NUMBER      = 4,
    BL_LILAC_RDD_USER_3_FILTER_NUMBER      = 5,
    BL_LILAC_RDD_PPPOE_D_FILTER_NUMBER     = 6,
    BL_LILAC_RDD_PPPOE_S_FILTER_NUMBER     = 7,
    BL_LILAC_RDD_ARP_FILTER_NUMBER         = 8,
    BL_LILAC_RDD_1588_FILTER_NUMBER        = 9,
    BL_LILAC_RDD_802_1X_FILTER_NUMBER      = 10,
    BL_LILAC_RDD_802_1AG_CFM_FILTER_NUMBER = 11,
    BL_LILAC_RDD_BROADCAST_FILTER_NUMBER   = 12,
    BL_LILAC_RDD_MULTICAST_FILTER_NUMBER   = 13,
    BL_LILAC_RDD_INGRESS_FILTERS_NUMBER    = 14,
}
BL_LILAC_RDD_INGRESS_FILTER_DTE;


typedef enum
{
    RDD_LAYER4_FILTER_ERROR             = 0,
    RDD_LAYER4_FILTER_EXCEPTION         = 1,
    RDD_LAYER4_FILTER_IP_FIRST_FRAGMENT = 2,
    RDD_LAYER4_FILTER_IP_FRAGMENT       = 3,
    RDD_LAYER4_FILTER_GRE               = 4,
    RDD_LAYER4_FILTER_LAYER3_IPV4       = 5,
    RDD_LAYER4_FILTER_LAYER3_IPV6       = 6,
    RDD_LAYER4_FILTER_ICMP              = 7,
    RDD_LAYER4_FILTER_ESP               = 8,
    RDD_LAYER4_FILTER_AH                = 9,
    RDD_LAYER4_FILTER_IPV6              = 10,
    RDD_LAYER4_FILTER_USER_DEFINED_0    = 11,
    RDD_LAYER4_FILTER_USER_DEFINED_1    = 12,
    RDD_LAYER4_FILTER_USER_DEFINED_2    = 13,
    RDD_LAYER4_FILTER_USER_DEFINED_3    = 14,
    RDD_LAYER4_FILTER_UNKNOWN           = 15,
}
RDD_LAYER4_FILTER_INDEX;


typedef enum
{
    BL_LILAC_RDD_LAYER4_FILTER_FORWARD  = 0,
    BL_LILAC_RDD_LAYER4_FILTER_CPU_TRAP = 1,
    BL_LILAC_RDD_LAYER4_FILTER_DROP     = 2,
}
BL_LILAC_RDD_LAYER4_FILTER_ACTION_DTE;


typedef enum
{
    BL_LILAC_RDD_FIREWALL_PROTOCOL_TYPE_TCP = 0,
    BL_LILAC_RDD_FIREWALL_PROTOCOL_TYPE_UDP = 1,
}
BL_LILAC_RDD_FIREWALL_PROTOCOL_TYPE_DTE;


typedef enum
{
    BL_LILAC_RDD_FLOW_CACHE_PBIT_ACTION_DSCP_COPY  = 0,
    BL_LILAC_RDD_FLOW_CACHE_PBIT_ACTION_OUTER_COPY = 1,
    BL_LILAC_RDD_FLOW_CACHE_PBIT_ACTION_INNER_COPY = 2,
}
BL_LILAC_RDD_FLOW_CACHE_PBIT_ACTION_DTE;


typedef enum
{
    RDD_FLOW_CACHE_FORWARD_ACTION_CPU  = 0,
    RDD_FLOW_CACHE_FORWARD_ACTION_DROP = 1,
}
RDD_FLOW_CACHE_FORWARD_ACTION;


typedef enum
{
    BL_LILAC_RDD_ACL_LAYER2_ACTION_DENY   = 0,
    BL_LILAC_RDD_ACL_LAYER2_ACTION_ACCEPT = 1,
}
BL_LILAC_RDD_ACL_LAYER2_ACTION_DTE;


typedef enum
{
    BL_LILAC_RDD_1588_MODE_DISABLE = 0,
    BL_LILAC_RDD_1588_MODE_ENABLE  = 1,
}
BL_LILAC_RDD_1588_MODE_DTE;


typedef enum
{
    RDD_1588_TX_THREAD_RETURN_NO_RESULT = 0,
    RDD_1588_TX_THREAD_RETURN_SUCCESS   = 1,
    RDD_1588_TX_THREAD_RETURN_FAIL      = 2,
}
RDD_1588_TX_THREAD_RESULT_DTE;


typedef enum
{
    BL_LILAC_RDD_INTER_LAN_SCHEDULING_MODE_NORMAL          = 0,
    BL_LILAC_RDD_INTER_LAN_SCHEDULING_MODE_STRICT_PRIORITY = 1,
    BL_LILAC_RDD_INTER_LAN_SCHEDULING_MODE_ROUND_ROBIN     = 2,
}
BL_LILAC_RDD_INTER_LAN_SCHEDULING_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_UPSTREAM_PADDING_DISABLE = 0,
    BL_LILAC_RDD_UPSTREAM_PADDING_ENABLE  = 1,
}
BL_LILAC_RDD_UPSTREAM_PADDING_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_NO_WAIT = 0,
    BL_LILAC_RDD_WAIT    = 1,
}
BL_LILAC_RDD_CPU_WAIT_DTE;


typedef enum
{
    RDD_US_PEAK_SCHEDULING_MODE_ROUND_ROBIN     = 0,
    RDD_US_PEAK_SCHEDULING_MODE_STRICT_PRIORITY = 1,
}
RDD_US_PEAK_SCHEDULING_MODE;


typedef enum
{
    rdd_queue_profile_0         = 0,
    rdd_queue_profile_1         = 1,
    rdd_queue_profile_2         = 2,
    rdd_queue_profile_3         = 3,
    rdd_queue_profile_4         = 4,
    rdd_queue_profile_5         = 5,
    rdd_queue_profile_6         = 6,
    rdd_queue_profile_7         = 7,
    rdd_queue_profile_disabled  = 8,
} rdd_queue_profile;


typedef struct
{
    uint16_t  min_threshold;
    uint16_t  max_threshold;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PRIORITY_CLASS_THRESHOLDS;


typedef struct
{
    RDD_PRIORITY_CLASS_THRESHOLDS  high_priority_class;
    RDD_PRIORITY_CLASS_THRESHOLDS  low_priority_class;
    uint32_t                       max_drop_probability;
    bdmf_boolean                   us_flow_control_mode;                         
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_QUEUE_PROFILE;


typedef enum
{
    BL_LILAC_RDD_FLOW_PM_COUNTERS_DS   = 1,
    BL_LILAC_RDD_FLOW_PM_COUNTERS_US   = 2,
    BL_LILAC_RDD_FLOW_PM_COUNTERS_BOTH = 3,
}
BL_LILAC_RDD_FLOW_PM_COUNTERS_TYPE_DTE;


typedef struct
{
    uint32_t  good_rx_packet;
    uint32_t  good_rx_bytes;
    uint32_t  good_tx_packet;
    uint32_t  good_tx_bytes;
    uint16_t  error_rx_packets_discard;
    uint16_t  error_tx_packets_discard;
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE;


typedef struct
{
    uint32_t  good_tx_packet;
    uint16_t  error_tx_packets_discard;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SERVICE_QUEUE_PM_COUNTERS_DTE;


typedef struct
{
    uint32_t  rx_valid;
    uint32_t  tx_valid;
    uint16_t  error_rx_bpm_congestion;
    uint16_t  bridge_filtered_packets;
    uint16_t  bridge_tx_packets_discard;
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE;

#ifdef G9991
typedef struct
{
    uint16_t  dfc_frame_error_counter;
    uint16_t  dfc_frame_counter;
    uint16_t  data_frame_error_counter;
    uint16_t  illegal_sid_error_counter;
    uint16_t  length_error_counter;
    uint16_t  reassembly_error_counter;
    uint16_t  bbh_error_counter;
    uint16_t  consequent_drop;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_G9991_PM_COUNTERS_DTE;

typedef struct
{
    uint32_t  rx_packets;
	uint32_t  rx_mcast_packets;
	uint32_t  rx_bcast_packets;
	uint32_t  rx_bytes;
    uint32_t  tx_packets;
	uint32_t  tx_mcast_packets;
	uint32_t  tx_bcast_packets;
	uint32_t  tx_bytes;
}
RDD_G9991_PM_FLOW_COUNTERS_DTE;
#endif

typedef struct
{
    uint32_t  good_rx_packet;
    uint32_t  good_tx_packet;
    uint32_t  good_rx_bytes;
    uint32_t  good_tx_bytes;
    uint16_t  rx_dropped_packet;
    uint16_t  tx_dropped_packet;
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_SUBNET_PM_COUNTERS_DTE;


typedef struct
{
    uint16_t  invalid_layer2_protocol_drop;
    uint16_t  firewall_drop;
    uint16_t  acl_oui_drop;
    uint16_t  acl_l2_drop;
    uint16_t  acl_l3_drop;
    uint16_t  dst_mac_non_router_drop;
    uint16_t  eth_flow_action_drop;
    uint16_t  sa_lookup_failure_drop;
    uint16_t  da_lookup_failure_drop;
    uint16_t  sa_action_drop;
    uint16_t  da_action_drop;
    uint16_t  forwarding_matrix_disabled_drop;
    uint16_t  connection_action_drop;
    uint16_t  iptv_layer3_drop;
    uint16_t  local_switching_congestion;
    uint16_t  vlan_switching_drop;
    uint16_t  downstream_policers_drop;
    uint16_t  layer4_filters_drop[ 16 ];
    uint16_t  ingress_filters_drop[ BL_LILAC_RDD_INGRESS_FILTERS_NUMBER ];
    uint16_t  ip_validation_filter_drop[ 2 ];
    uint16_t  emac_loopback_drop;
    uint16_t  tpid_detect_drop;
    uint16_t  dual_stack_lite_congestion_drop;
    uint16_t  invalid_subnet_ip_drop;
    uint16_t  us_ddr_queue_drop;
    uint16_t  ds_parallel_processing_no_avialable_slave;
    uint16_t  ds_parallel_processing_reorder_slaves;
    uint16_t  absolute_address_list_overflow_drop;
    uint16_t  dhd_ih_congestion_drop;
    uint16_t  dhd_malloc_failed_drop;
    uint16_t  wlan_mcast_copy_failed_drop;
    uint16_t  wlan_mcast_overflow_drop;
    uint16_t  wlan_mcast_drop;
	uint16_t  sbpm_alloc_reply_nack;
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_VARIOUS_COUNTERS_DTE;


typedef enum
{
    INVALID_LAYER2_PROTOCOL_DROP_COUNTER_MASK    = 0x1,
    FIREWALL_DROP_COUNTER_MASK                   = 0x2,
    ACL_OUI_DROP_COUNTER_MASK                    = 0x4,
    ACL_L2_DROP_COUNTER_MASK                     = 0x8,
    ACL_L3_DROP_COUNTER_MASK                     = 0x10,
    DST_MAC_NON_ROUTER_DROP_COUNTER_MASK         = 0x20,
    ETHERNET_FLOW_ACTION_DROP_COUNTER_MASK       = 0x40,
    SA_LOOKUP_FAILURE_DROP_COUNTER_MASK          = 0x80,
    DA_LOOKUP_FAILURE_DROP_COUNTER_MASK          = 0x100,
    SA_ACTION_DROP_COUNTER_MASK                  = 0x200,
    DA_ACTION_DROP_COUNTER_MASK                  = 0x400,
    FORWARDING_MATRIX_DISABLED_DROP_COUNTER_MASK = 0x800,
    CONNECTION_ACTION_DROP_COUNTER_MASK          = 0x1000,
    IPTV_LAYER3_DROP_COUNTER_MASK                = 0x2000,
    LOCAL_SWITCHING_CONGESTION_COUNTER_MASK      = 0x4000,
    VLAN_SWITCHING_DROP_COUNTER_MASK             = 0x8000,
    DOWNSTREAM_POLICERS_DROP_COUNTER_MASK        = 0x10000,
    LAYER4_FILTERS_DROP_COUNTER_MASK             = 0x20000,
    INGRESS_FILTERS_DROP_COUNTER_MASK            = 0x40000,
    IP_VALIDATION_FILTER_DROP_COUNTER_MASK       = 0x80000,
    EMAC_LOOPBACK_DROP_COUNTER_MASK              = 0x100000,
    TPID_DETECT_DROP_COUNTER_MASK                = 0x200000,
    DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_MASK = 0x400000,
    INVALID_SUBNET_IP_DROP_COUNTER_MASK          = 0x800000,
    EPON_DDR_QUEUEU_DROP_COUNTER_MASK            = 0x800000,
    ABSOLUTE_ADDRESS_LIST_OVERFLOW_MASK          = 0x1000000,
    DHD_IH_CONGESTION_MASK                       = 0x2000000,
    DHD_MALLOC_FAILED_MASK                       = 0x4000000,
    WLAN_MCAST_COPY_FAILED_MASK                  = 0x8000000,
    WLAN_MCAST_OVERFLOW_MASK                     = 0x10000000,
    WLAN_MCAST_DROP_COUNTER_MASK                 = 0x20000000,
}
RDD_VARIOUS_COUNTERS_MASK;

typedef enum
{
    RDD_CHIP_REVISION_A0 = 0,
    RDD_CHIP_REVISION_B0 = 1,
}
BL_LILAC_RDD_CHIP_REVISION_DTE;

typedef struct
{
    uint8_t                             *ddr_pool_ptr;       /* virtual address */
    uint32_t                            ddr_pool_ptr_phys;   /* physical address */
    uint8_t                             *extra_ddr_pool_ptr; /* virtual address */
    uint32_t                            extra_ddr_pool_ptr_phys; /* physical address */
    uint8_t                             *ddr_runner_tables_ptr;  /* virtual address */
    uint32_t                            ddr_runner_tables_ptr_phys; /* physical address */
    BL_LILAC_RDD_MAC_TABLE_SIZE_DTE     mac_table_size;
    BL_LILAC_RDD_MAC_TABLE_SIZE_DTE     iptv_table_size;
    BL_LILAC_RDD_WAN_PHYSICAL_PORT_DTE  wan_physical_port;
    uint32_t                            ddr_headroom_size;
    int16_t                             broadcom_switch_mode;
    BL_LILAC_RDD_BRIDGE_PORT_DTE        broadcom_switch_physical_port;
    uint32_t                            bridge_flow_cache_mode;
    uint16_t                            epon_post_scheduling_queue_size;
    BL_LILAC_RDD_CHIP_REVISION_DTE      chip_revision;
    uint16_t                            cpu_tx_abs_packet_limit;
    BL_LILAC_RDD_BRIDGE_PORT_DTE        lan_direct_to_cpu_port;
    bdmf_boolean                        us_ddr_queue_enable;
    rdpa_bpm_buffer_size_t              bpm_buffer_size;
}
RDD_INIT_PARAMS;


typedef struct
{
    uint32_t  rate;
    uint32_t  limit;
}
RDD_RATE_LIMIT_PARAMS;


typedef struct
{
    uint32_t               sustain_budget;
    RDD_RATE_LIMIT_PARAMS  peak_budget;
    uint32_t               peak_weight;
}
RDD_RATE_CONTROLLER_PARAMS;


typedef struct
{
    BL_LILAC_RDD_SUBNET_ID_DTE               subnet_id;
    BL_LILAC_RDD_FIREWALL_PROTOCOL_TYPE_DTE  protocol;
    uint32_t                                 dst_port;
    uint32_t                                 dst_port_last;
    bdmf_ip_t                                src_ip;
    uint32_t                                 src_ip_mask;
    bdmf_ip_t                                dst_ip;
    uint32_t                                 check_mask_src_ip;
    uint32_t                                 check_src_ip;
    uint32_t                                 check_dst_ip;
    uint32_t                                 next_rule_index;
}
RDD_FIREWALL_RULE_PARAMS;


typedef struct
{
    bdmf_mac_t                         mac_addr;
    uint16_t                           vlan_id;
    BL_LILAC_RDD_BRIDGE_PORT_DTE       bridge_port;
    BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE    entry_type;
    BL_LILAC_RDD_AGGREGATION_MODE_DTE  aggregation_mode;
    uint8_t                            extension_entry;
    BL_LILAC_RDD_MAC_FWD_ACTION_DTE    sa_action;
    BL_LILAC_RDD_MAC_FWD_ACTION_DTE    da_action;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_PARAMS;


typedef struct
{
    uint16_t                             vid;
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  isolation_mode_port_vector;
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  aggregation_mode_port_vector;
    uint16_t                             aggregation_vid_index;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAN_VID_PARAMS;


typedef struct
{
    uint8_t                       *packet_ptr;
    uint32_t                      packet_size;
    BL_LILAC_RDD_BRIDGE_PORT_DTE  src_bridge_port;
    uint8_t                       wifi_src_ssid_index;
    uint16_t                      wifi_dst_ssid_vector;
    uint16_t                      buffer_number;
}
RDD_PCI_TX_PARAMS;


typedef struct
{
    uint8_t                         *packet_ptr;
    uint32_t                        packet_size;
    BL_LILAC_RDD_BRIDGE_PORT_DTE    src_bridge_port;
    uint32_t                        flow_id;
    rdpa_cpu_reason                 reason;
    uint16_t                        buffer_number;
}
RDD_CPU_RX_PARAMS;


typedef enum
{
    BL_LILAC_RDD_UPSTREAM_FLOW_CLASSIFY_VID                = 0,
    BL_LILAC_RDD_UPSTREAM_FLOW_CLASSIFY_VID_PBITS          = 0,
    BL_LILAC_RDD_UPSTREAM_FLOW_CLASSIFY_VID_SRC_PORT       = 1,
    BL_LILAC_RDD_UPSTREAM_FLOW_CLASSIFY_VID_PBITS_SRC_PORT = 2,
}
BL_LILAC_RDD_UPSTREAM_FLOW_CLASSIFY_MODE_DTE;


typedef enum
{
    BL_LILAC_RDD_DOWNSTREAM_FLOW_CLASSIFY_WAN_FLOW = 0,
    BL_LILAC_RDD_DOWNSTREAM_FLOW_CLASSIFY_PACKET   = 1,
}
BL_LILAC_RDD_DOWNSTREAM_FLOW_CLASSIFY_MODE_DTE;

typedef struct
{
	uint32_t                                vlan_command_id;
    rdd_bridge_vlan_command                 vlan_command;
    rdd_bridge_pbits_command                pbits_command;
    uint16_t                                outer_vid;
    uint16_t                                inner_vid;
    uint8_t                                 outer_pbits;
    uint8_t                                 inner_pbits;
	bdmf_boolean                            outer_tpid_overwrite_enable;
	bdmf_boolean                            inner_tpid_overwrite_enable;
    rdd_tpid_id                             outer_tpid_id; 
    rdd_tpid_id                             inner_tpid_id; 
}
rdd_vlan_command_params;

typedef enum
{
    rdd_dei_command_transparent                         = 0,
    rdd_dei_command_clear                               = 1,
    rdd_dei_command_set                                 = 2,
}
rdd_dei_command;

/** Ingress classification lookup mode */
typedef enum
{
    rdd_ingress_classification_lookup_mode_ih        = 0,
    rdd_ingress_classification_lookup_mode_optimized = 1,
    rdd_ingress_classification_lookup_mode_short     = 2,
    rdd_ingress_classification_lookup_mode_long      = 3,
} rdd_ingress_classification_lookup_mode;

#ifndef RDD_BASIC
typedef struct
{
    rdpa_qos_method               qos_method; /**< QoS classification method flow / pbit */
    uint8_t                       wan_flow;  /**< WAN flow : Gem Flow or LLID */
    rdpa_forward_action           action; /**< forward/drop/cpu */
    bdmf_index                    policer; /**< Policer ID */
    rdpa_forwarding_mode          forw_mode;  /** < flow/pkt based */
    bdmf_boolean                  opbit_remark; /**< enable outer pbit remark */
    rdpa_pbit                     opbit_val;	/**< outer pbit remark value */
    bdmf_boolean                  ipbit_remark; /**< enable inner pbit remark */
    rdpa_pbit                     ipbit_val; /**< inner pbit remark value */
    bdmf_boolean                  dscp_remark; /**< enable dscp remark */
    rdpa_dscp                     dscp_val; /**< dscp remark value */
    uint8_t                       ecn_val;
    BL_LILAC_RDD_BRIDGE_PORT_DTE  egress_port;
    uint8_t                       wifi_ssid;
    BL_LILAC_RDD_SUBNET_ID_DTE    subnet_id;
    bdmf_index                    rate_shaper;
    uint8_t                       rate_controller_id;
    uint8_t                       priority;
    uint8_t                       wan_flow_mapping_table;
    rdpa_qos_method               wan_flow_mapping_mode;
    bdmf_boolean                  service_queue_mode;
    uint8_t                       service_queue;
    bdmf_boolean                  qos_rule_wan_flow_overrun; /**< enable overrun wan flow value by qos rule   */
    rdd_dei_command               dei_command;
    bdmf_boolean                  cpu_mirroring;
    bdmf_boolean                  ic_ip_flow;
    uint8_t                       trap_reason;
    union {
        struct { 
            uint8_t  eth0_vlan_command;
            uint8_t  eth1_vlan_command;
            uint8_t  eth2_vlan_command;
            uint8_t  eth3_vlan_command;
            uint8_t  eth4_vlan_command;
#ifndef G9991
            uint8_t  pci_vlan_command;
#else
            uint8_t  eth5_vlan_command;
            uint8_t  eth6_vlan_command;
            uint8_t  eth7_vlan_command;
            uint8_t  eth8_vlan_command;
            uint8_t  eth9_vlan_command;
            uint8_t  eth10_vlan_command;
            uint8_t  eth11_vlan_command;
            uint8_t  eth12_vlan_command;
            uint8_t  eth13_vlan_command;
            uint8_t  eth14_vlan_command;
            uint8_t  eth15_vlan_command;
            uint8_t  eth16_vlan_command;
            uint8_t  eth17_vlan_command;
            uint8_t  eth18_vlan_command;
            uint8_t  eth19_vlan_command;
            uint8_t  eth20_vlan_command;
            uint8_t  eth21_vlan_command;
            uint8_t  eth22_vlan_command;
            uint8_t  eth23_vlan_command;
#endif
        } ds_vlan_command;
        uint8_t  us_vlan_command;
    } vlan_command_id;
} rdd_ingress_classification_context_t;

typedef struct
{
    rdpa_traffic_dir conn_dir;
    bdmf_index conn_index;

    rdpa_fc_action_vec_t actions_vector;
    RDD_FLOW_CACHE_FORWARD_ACTION fwd_action; /* In use when forward action action is turned on in action vector */
    rdpa_cpu_reason trap_reason; /* CPU trap reason in case forwarding action is ::rdpa_forward_action_host
                                    and ::rdpa_fc_action_forward is set. */
    bdmf_boolean service_queue_enabled;
    bdmf_index service_queue_id;
    rdpa_qos_method	qos_method;

    bdmf_ip_family ip_version;
    uint16_t nat_port;
    bdmf_ip_t nat_ip;
    uint8_t ds_lite_hdr_index;
    uint8_t tunnel_index;

    uint8_t ovid_offset;
    uint8_t	opbit_action;
    uint8_t ipbit_action;
    rdpa_dscp dscp_value; /* DSCP value if ::rdpa_fc_action_dscp_remark is set. */
    uint8_t	ecn_value;
    uint8_t policer_id;

    uint8_t	egress_port;

    /* For WAN/LAN egress, mutual exclusive with wl_metadata */
    uint8_t	traffic_class;
    uint8_t	wan_flow_index;
    uint8_t rate_controller;

    /* For WiFi usage */
    uint8_t	wifi_ssid;
    union {
        uint32_t wl_metadata;                  /**< WL metadata */
        rdpa_wfd_t wfd;
        rdpa_rnr_t rnr;
    };

    int8_t l2_hdr_offset;
    uint8_t l2_hdr_size;
    uint8_t l2_hdr_number_of_tags;
    uint8_t l2_header[RDD_FLOW_CACHE_L2_HEADER_BYTE_SIZE];

    /* Statistics (read-only) */
    rdpa_stat_t valid_cnt;

    uint8_t drop_eligibility;              /* drop_eligibility bit 0: 0=non drop eligible class high, 1=drop eligible low, bit 1: enabler */ 
} rdd_fc_context_t;

#endif

/** Full flow cache corner case acceleration modes */
typedef enum
{
    rdd_full_fc_acceleration_non_ip       = 0,
    rdd_full_fc_acceleration_multicast_ip = 1,
} rdd_full_fc_acceleration_mode;

typedef enum
{
    rdd_single_priority_mode = 0,
    rdd_double_priority_mode = 1,
} rdd_policer_mode;



/** CPU TX redesign **/
typedef enum
{
    rdd_cpu_tx_queue_table_fast_a = 0,
    rdd_cpu_tx_queue_table_fast_b = 1,
    rdd_cpu_tx_queue_table_pico_a = 2,
    rdd_cpu_tx_queue_table_pico_b = 3,
} rdd_cpu_tx_queue_table;


typedef enum
{
    rdd_cpu_tx_mode_full         = 0,
    rdd_cpu_tx_mode_interworking = 1,
    rdd_cpu_tx_mode_egress_enq   = 2,
} rdd_cpu_tx_mode;


typedef enum
{
    rdd_host_buffer   = 0,
    rdd_runner_buffer = 1,
} rdd_buffer_type;


typedef struct
{
    rdpa_traffic_dir  traffic_dir;
    rdd_cpu_tx_mode   mode;
    rdd_buffer_type   buffer_type;

    uint8_t   wifi_ssid;
    uint16_t  wan_flow;
    rdpa_discard_prty  drop_precedence;

    union 
    {
        struct 
        {
            BL_LILAC_RDD_EMAC_ID_DTE   emac_id;
            BL_LILAC_RDD_QUEUE_ID_DTE  queue_id;
            bdmf_boolean               en_1588;
        } ds;

        struct
        {
            uint32_t                      wan_channel;
            uint32_t                      rate_controller;
            uint32_t                      queue;
            BL_LILAC_RDD_BRIDGE_PORT_DTE  src_bridge_port;
            bdmf_boolean                  is_spdsrvc;
        } us;
    } direction;
} rdd_cpu_tx_args_t;


typedef void ( * BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_DTE )( bdmf_fastlock * );
typedef void ( * BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_DTE )( bdmf_fastlock * );
typedef void ( * BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_IRQ_DTE )( bdmf_fastlock *, unsigned long * );
typedef void ( * BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_IRQ_DTE )( bdmf_fastlock *, unsigned long );


#if defined(USE_BDMF_SHELL)
extern uint32_t g_rdd_trace;

#define RDD_TRACE(fmt, args...) \
    do { \
        if (g_rdd_trace) \
            bdmf_trace("TRACE: %s#%d : " fmt, __FUNCTION__, __LINE__, ## args); \
    } while(0)

#if defined(LINUX_KERNEL) || defined(__KERNEL__)
#define RDD_BTRACE_FMT "TRACE: %s#%d (<-- %pS): "
#else
#define RDD_BTRACE_FMT "TRACE: %s#%d (<-- %p): "
#endif

#define RDD_BTRACE(fmt, args...) \
    do { \
        void *ra1 = __builtin_return_address(0); \
        if (g_rdd_trace) \
            bdmf_trace(RDD_BTRACE_FMT fmt, __FUNCTION__, __LINE__, (void *)ra1, ## args); \
    } while(0)

#define RDD_BTRACE_BUF(title, buf, len) \
    do { \
        void *ra1 = __builtin_return_address(0); \
        if (g_rdd_trace) { \
            bdmf_session_print(NULL, RDD_BTRACE_FMT "*** %s (Length %d) :\n", __FUNCTION__, __LINE__, (void *)ra1, \
                title, len); \
            bdmf_session_hexdump(NULL, buf, 0, len); \
        } \
    } while(0)

#else
#define RDD_TRACE(fmt, args...) do { } while (0)
#define RDD_BTRACE(fmt, args...) do { } while (0)
#define RDD_BTRACE_BUF(title, buf, len) do { } while (0)
#endif

#endif

