/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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

/***************************************************************************
 * File Name  : bcmxtmcfg.h
 *
 * Description: This file contains the definitions, structures and function
 *              prototypes for the Broadcom Asynchronous/Packet Transfer Mode
 *              (XTM) Configuration driver.
 ***************************************************************************/

#if !defined(_BCMXTMCFG_H_)
#define _BCMXTMCFG_H_

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************************************************
 * API Version Definitions
 ***************************************************************************/

#define  BCM_XTM_API_VERSION(a,b) (((a) << 16) + (b))

#define  BCM_XTM_API_MAJ_VERSION        2
#define  BCM_XTM_API_MIN_VERSION        6

/***************************************************************************
 * Constant Definitions
 ***************************************************************************/

/* Miscellaneous */
#define MAX_PHY_PORTS                   4
#define MAX_SUB_PRIORITIES              8
#define MAX_PTM_PRIORITIES              2
#define MAX_BOND_GROUPS                 1
#define MAX_BOND_PORTS                  2
#define MAX_RECEIVE_QUEUES              2
#define MAX_TRANSMIT_QUEUES             16
#define NETWORK_DEVICE_NAME_SIZE        16

#define MAX_ATM_TRANSMIT_QUEUES         16
#define MAX_PTM_TRANSMIT_QUEUES         8

/* Values for ulPortId and ulPortMask fields. */
#define PORT_PHY0_FAST                  0x01
#define PORT_PHY0_INTERLEAVED           0x02
#define PORT_PHY1_FAST                  0x04
#define PORT_PHY1_INTERLEAVED           0x08
#define PORT_PHY_INVALID                0xFF

#define PORT_PHY0_PATH0                 0x01
#define PORT_PHY0_PATH1                 0x02
#define PORT_PHY1_PATH0                 0x04
#define PORT_PHY1_PATH1                 0x08

#define LOG_PORTID_0                    0x00
#define LOG_PORTID_1                    0x01
#define LOG_PORTID_2                    0x02
#define LOG_PORTID_3                    0x03

/* For bonding, we work on absolutte port id assignments. It goes as follows.
 * PTM bonding, port ids 0 & 1 will be bonded. Single latency only. No dual
 * latency support.
 * ATM bonding, port ids 0 & 1 will be bonded for single latency.
 * ATM bonding, port ids 0&2, 1&3 will be bonded in dual latency mode.
 */
#define PHY_PORTID_0                    0x00
#define PHY_PORTID_1                    0x01
#define PHY_PORTID_2                    0x02
#define PHY_PORTID_3                    0x03

/* Conversions between port and port id. */
#define PORT_TO_PORTID(PORT)            (UINT32) (1 << (PORT))
#define PORTID_TO_PORT(PORTID)          (UINT32)                           \
    (((PORTID) == PORT_PHY0_FAST)        ? PHY_PORTID_0 :                  \
     ((PORTID) == PORT_PHY0_INTERLEAVED) ? PHY_PORTID_1 :                  \
     ((PORTID) == PORT_PHY1_FAST)        ? PHY_PORTID_2 :                  \
     ((PORTID) == PORT_PHY1_INTERLEAVED) ? PHY_PORTID_3 : MAX_PHY_PORTS)

#define PORTMAP_TO_LOGICALPORT(PORTMAP)  (UINT32)                                              \
    (((PORTMAP) == PORT_PHY0_FAST)        ? LOG_PORTID_0 :                                     \
     ((PORTMAP) == PORT_PHY0_INTERLEAVED) ? LOG_PORTID_0 :                                     \
     ((PORTMAP) == (PORT_PHY0_INTERLEAVED | PORT_PHY0_FAST)) ? LOG_PORTID_0 :                  \
     ((PORTMAP) == PORT_PHY1_FAST)        ? LOG_PORTID_1 :                                     \
     ((PORTMAP) == PORT_PHY1_INTERLEAVED) ? LOG_PORTID_1 :                                     \
     ((PORTMAP) == (PORT_PHY1_INTERLEAVED | PORT_PHY1_FAST)) ? LOG_PORTID_1: MAX_PHY_PORTS)

/* Values for XTM_INITIALIZATION_PARMS ulPortConfig. */
#define PC_INTERNAL_EXTERNAL_MASK       0x03
#define PC_ALL_INTERNAL                 0x00
#define PC_ALL_EXTERNAL                 0x01
#define PC_INTERNAL_EXTERNAL            0x02
#define PC_NEG_EDGE                     0x04

/* Values for XTM_INITIALIZATION_PARMS sBondConfig */
#define BC_PTM_BONDING_ENABLE           0x01
#define BC_PTM_BONDING_DISABLE          0x00

#define BC_ATM_BONDING_ENABLE           0x01
#define BC_ATM_BONDING_DISABLE          0x00

#define BC_DUAL_LATENCY_ENABLE          0x01
#define BC_DUAL_LATENCY_DISABLE         0x00

#define BC_ATM_AUTO_SENSE_ENABLE        0x01
#define BC_ATM_AUTO_SENSE_DISABLE       0x00

#define BC_BOND_PROTO_NONE              0x00
#define BC_BOND_PROTO_G994_AGGR         0x01
#define BC_BOND_PROTO_ASM               0x02
#define BC_BOND_PROTO_BACP              0x03

#define BONDING_INVALID_GROUP_ID        0xFFFFFFFF

#define DATA_STATUS_DISABLED    0x0
#define DATA_STATUS_ENABLED     0x1
#define DATA_STATUS_RESET       0x2

/* G.998.1 message types */
#define ATMBOND_ASM_MESSAGE_TYPE_12BITSID    0
#define ATMBOND_ASM_MESSAGE_TYPE_8BITSID     1
#define ATMBOND_ASM_MESSAGE_TYPE_NOSID       2

/* Values for XTM_TRAFFIC_DESCR_PARM_ENTRY ulTrafficDescrType. */
#define TDT_ATM_NO_TRAFFIC_DESCRIPTOR   1
#define TDT_ATM_NO_CLP_NO_SCR           2
#define TDT_ATM_NO_CLP_SCR              5
#define TDT_ATM_CLP_NO_TAGGING_MCR      8
#define TDT_ATM_CLP_TRANSPARENT_NO_SCR  9
#define TDT_ATM_CLP_TRANSPARENT_SCR     10
#define TDT_ATM_NO_CLP_TAGGING_NO_SCR   11
#define TDT_ATM_PTM_MAX_BIT_RATE_SCR    16

/* Values for XTM_TRAFFIC_DESCR_PARM_ENTRY ulServiceCategory. */
#define SC_OTHER                        1
#define SC_CBR                          2
#define SC_RT_VBR                       3
#define SC_NRT_VBR                      4
#define SC_UBR                          6
#define SC_MBR                          7

/*Values for XTM_INTERFACE_CFG ulIfAdminStatus and XTM_CONN_CFG ulAdminStatus */
#define ADMSTS_UP                       1
#define ADMSTS_DOWN                     2

/* Values for XTM_INTERFACE_CFG ulIfOperStatus and XTM_CONN_CFG ulOperStatus. */
#define OPRSTS_UP                       1
#define OPRSTS_DOWN                     2

/* Values for XTM_INTERFACE_LINK_INFO ulLinkState. */
#define LINK_UP                         1
#define LINK_DOWN                       2
#define LINK_START_TEQ_DATA             3
#define LINK_STOP_TEQ_DATA              4
#define LINK_DS_DOWN                    5
#define LINK_TRAINING1                  6   /* DSL Training G992/G993 Started */
#define LINK_TRAINING2                  7   /* DSL Training G992/G993 Channel Analysis */


/* Values for XTM_ADDR ulTrafficType, XTM_INTERFACE_CFG usIfTrafficType and
 * XTM_INTERFACE_LINK_INFO ulLinkTrafficType.
 */
#define TRAFFIC_TYPE_NOT_CONNECTED      0

#define TRAFFIC_TYPE_ATM_MASK				 0x1
#define TRAFFIC_TYPE_ATM                0x1   	/* Odd types for ATM... */
#define TRAFFIC_TYPE_ATM_BONDED         0x3
#define TRAFFIC_TYPE_ATM_TEQ            0x5

#define TRAFFIC_TYPE_PTM                0x2	/* Even types for PTM... */
#define TRAFFIC_TYPE_PTM_RAW            0x4
#define TRAFFIC_TYPE_PTM_BONDED         0x6
#define TRAFFIC_TYPE_PTM_TEQ            0x8
#define TRAFFIC_TYPE_TXPAF_PTM_BONDED   0xA

#define TRAFFIC_TYPE_TEQ                (TRAFFIC_TYPE_ATM_TEQ|TRAFFIC_TYPE_PTM_TEQ)

/* Values for XTM_INTERFACE_CFG usIfSupportedTrafficTypes. */
#define SUPPORT_TRAFFIC_TYPE_ATM               (1 << TRAFFIC_TYPE_ATM)
#define SUPPORT_TRAFFIC_TYPE_PTM               (1 << TRAFFIC_TYPE_PTM)
#define SUPPORT_TRAFFIC_TYPE_PTM_RAW           (1 << TRAFFIC_TYPE_PTM_RAW)
#define SUPPORT_TRAFFIC_TYPE_PTM_BONDED        (1 << TRAFFIC_TYPE_PTM_BONDED)
#define SUPPORT_TRAFFIC_TYPE_ATM_BONDED        (1 << TRAFFIC_TYPE_ATM_BONDED)
#define SUPPORT_TRAFFIC_TYPE_TEQ               (1 << TRAFFIC_TYPE_TEQ)
#define SUPPORT_TRAFFIC_TYPE_TXPAF_PTM_BONDED  (1 << TRAFFIC_TYPE_TXPAF_PTM_BONDED)

/* Values for PTM_ADDR ulPtmPriority. */
#define PTM_PRI_LOW                     0x01
#define PTM_PRI_HIGH                    0x02

/* Values for XTM_TRANSMIT_QUEUE_PARMS ulWeightAlg. */
#define WA_DISABLED                     0 /* disabled */
#define WA_CWRR                         1 /* cell weighted round robin */
#define WA_PWRR                         2 /* packet weighted round robin */
#define WA_WFQ                          3 /* weighted fair queuing */

/* Values for XTM_TRANSMIT_QUEUE_PARMS ucDropAlg. */
#define WA_DT                           0 /* Drop Tail */
#define WA_RED                          1 /* Random Early Detection */
#define WA_WRED                         2 /* Weighted RED */ 

/* Values for XTM_CONN_CFG ulAtmAalType. */
#define AAL_TRANSPARENT                 1
#define AAL_0_PACKET                    2
#define AAL_0_CELL                      3
#define AAL_5                           7

/* Values for XTM_CONN_CFG header types. */
#define HT_TYPE_LLC_SNAP_ETHERNET        0x01 /* AA AA 03 00 80 C2 00 07 00 00 */
#define HT_TYPE_LLC_SNAP_ROUTE_IP        0x02 /* AA AA 03 00 00 00 08 00 */
#define HT_TYPE_LLC_ENCAPS_PPP           0x03 /* FE FE 03 CF */
#define HT_TYPE_VC_MUX_ETHERNET          0x04 /* 00 00 */
#define HT_TYPE_VC_MUX_IPOA              0x05 /* */
#define HT_TYPE_VC_MUX_PPPOA             0x06 /* */
#define HT_TYPE_PTM                      0x07 /* */

/* Values for XTM_CONN_CFG header lengths. */
#define HT_LEN_LLC_SNAP_ETHERNET         10
#define HT_LEN_LLC_SNAP_ROUTE_IP         8
#define HT_LEN_LLC_ENCAPS_PPP            4
#define HT_LEN_VC_MUX_ETHERNET           2
#define HT_LEN_VC_MUX_PPPOA              0
#define HT_LEN_VC_MUX_IPOA               0
#define HT_LEN_PTM                       0

/* Values for XTM_CONN_CFG ulHeaderType. */
#define HT_LLC_SNAP_ETHERNET             \
    (((UINT32) HT_TYPE_LLC_SNAP_ETHERNET << 16) | HT_LEN_LLC_SNAP_ETHERNET)
#define HT_LLC_SNAP_ROUTE_IP             \
    (((UINT32) HT_TYPE_LLC_SNAP_ROUTE_IP << 16) | HT_LEN_LLC_SNAP_ROUTE_IP)
#define HT_LLC_ENCAPS_PPP                \
    (((UINT32) HT_TYPE_LLC_ENCAPS_PPP << 16) | HT_LEN_LLC_ENCAPS_PPP)
#define HT_VC_MUX_ETHERNET               \
    (((UINT32) HT_TYPE_VC_MUX_ETHERNET << 16) | HT_LEN_VC_MUX_ETHERNET)
#define HT_VC_MUX_IPOA                   \
    (((UINT32) HT_TYPE_VC_MUX_IPOA << 16) | HT_LEN_VC_MUX_IPOA)
#define HT_VC_MUX_PPPOA                  \
    (((UINT32) HT_TYPE_VC_MUX_PPPOA << 16) | HT_LEN_VC_MUX_PPPOA)
#define HT_PTM                           \
    (((UINT32) HT_TYPE_PTM << 16) | HT_LEN_PTM)

#define HT_TYPE(H)                       ((H >> 16) & 0xffff)
#define HT_LEN(H)                        (H & 0xffff)

/* Values for BcmXtm_SendOamCell ucCircuitType. */
#define CTYPE_OAM_F5_SEGMENT            0x00
#define CTYPE_OAM_F5_END_TO_END         0x01
#define CTYPE_OAM_F4_SEGMENT            0x02
#define CTYPE_OAM_F4_END_TO_END         0x03
#define CTYPE_ASM_P0                    0x04
#define CTYPE_ASM_P1                    0x05
#define CTYPE_ASM_P2                    0x06
#define CTYPE_ASM_P3                    0x07

/* Deviation */
#define XTM_DS_MIN_DEVIATION            2
#define XTM_DS_MAX_DEVIATION            128

#define XTM_RX_TEQ_PHY_PORT             PHY_PORTID_3

/***************************************************************************
 * Type Definitions
 ***************************************************************************/

/* Return status values. */
typedef enum BcmXtmStatus
{
    XTMSTS_SUCCESS = 0,
    XTMSTS_ERROR,
    XTMSTS_STATE_ERROR,
    XTMSTS_PARAMETER_ERROR,
    XTMSTS_ALLOC_ERROR,
    XTMSTS_RESOURCE_ERROR,
    XTMSTS_IN_USE,
    XTMSTS_NOT_FOUND,
    XTMSTS_NOT_SUPPORTED,
    XTMSTS_TIMEOUT,
    XTMSTS_PROTO_ERROR
} BCMXTM_STATUS;

#define IDLE_CELL_VPI                 0x00
#define IDLE_CELL_VCI                 0x00

typedef struct AtmAddr
{
    UINT32 ulPortMask;
    UINT16 usVpi;
    UINT16 usVci;
} ATM_ADDR, *PATM_ADDR;

typedef struct PtmAddr
{
    UINT32 ulPortMask;
    UINT32 ulPtmPriority;
} PTM_ADDR, *PPTM_ADDR;

typedef struct GenAddr
{
    UINT32 ulPortMask;
    UINT32 ulPtmPriority;
} GEN_ADDR, *PGEN_ADDR;

typedef struct XtmAddr
{
    UINT32 ulTrafficType;
    union
    {
        ATM_ADDR  Vcc;
        PTM_ADDR  Flow;
        GEN_ADDR  Conn;
    } u;
} XTM_ADDR, *PXTM_ADDR;

typedef union _XtmBondConfig {
   struct _sConfig {
      UINT32 ptmBond       :  1 ;
      UINT32 atmBond       :  1 ;
      UINT32 bondProto     :  1 ;    /* For PTM, BACP (Bonding Aggr Cont Protocol)
                                        For ATM, ASM based (as defined in G998.1) */
      UINT32 dualLat       :  1 ;
		UINT32 autoSenseAtm  :  1 ;    /* Needed to auto sense between ATM bonded/Non-bonded types */
      UINT32 resv          : 27 ;
   } sConfig ;
   UINT32 uConfig ;
} XtmBondConfig ;

typedef struct XtmInitialization
{
    UINT32        ulReceiveQueueSizes[MAX_RECEIVE_QUEUES];
    UINT32        ulPortConfig;
    XtmBondConfig bondConfig ;
} XTM_INITIALIZATION_PARMS, *PXTM_INITIALIZATION_PARMS;




typedef struct XtmConfiguration
{
    /* Set flags to indicate which parameters are being configured */
    struct _sParamsSelected {
#define XTM_CONFIGURATION_PARM_SET  1   /* Set a parameter */
#define XTM_CONFIGURATION_PARM_DUMP 2   /* Print existing parameter value to console */

        UINT32  trafficParam;      /* Indicates timeout for bonding indication configured */
        UINT32  singleLineParam;   /* Indicates single line activity configured */
    } sParamsSelected;

    /* The actual parameters to set */
    UINT32        ulBondingTrafficTimeoutSeconds;
    UINT32        ulSingleLineTimeoutSeconds;
} XTM_CONFIGURATION_PARMS, *PXTM_CONFIGURATION_PARMS;

#define PORT_Q_SHAPING_OFF   0
#define PORT_Q_SHAPING_ON    1

typedef struct XtmInterfaceCfg
{
    UINT32 ulIfAdminStatus;           /* UTOPIA enable/disable */
    UINT32 ulIfOperStatus;            /* read only */
    UINT32 ulIfLastChange;            /* read only */
    UINT16 usIfSupportedTrafficTypes; /* read only */
    UINT16 usIfTrafficType;           /* read only */
    UINT32 ulAtmInterfaceConfVccs;    /* read only */
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    UINT32 ulPortShaping;             /* Port Shaping enable/disable */
    UINT32 ulShapeRate;               /* Port Shaping */
    UINT16 usMbs;                     /* Port Shaping */
#endif
} XTM_INTERFACE_CFG, *PXTM_INTERFACE_CFG;

/* XTM_THRESHOLD contains the fields needed to config the link mode
 * thresholds.
 */
typedef struct XtmThreshold
{
    /* Set flags to indicate which parameters are being configured */
    struct _sParams {
#define XTM_THRESHOLD_PARM_SET  1   /* Set a parameter */
#define XTM_THRESHOLD_PARM_GET  2   /* Print existing parameter value to console */

        UINT32  adslParam;      /* Indicates threshold for ADSL */
        UINT32  vdslParam;      /* Indicates threshold for VDSL */
        UINT32  vdslRtxParam;   /* Indicates threshold for VDSL GINP */
        UINT32  gfastParam;     /* Indicates threshold for GFAST */
    } sParams;

    /* The actual parameters to operate */
    UINT32        adslThreshold;
    UINT32        vdslThreshold;
    UINT32        vdslRtxThreshold;
    UINT32        gfastThreshold;
} XTM_THRESHOLD_PARMS, *PXTM_THRESHOLD_PARMS;


/* XTM_TRAFFIC_DESCR_PARM_ENTRY contains the fields needed to create a Traffic
 * Descriptor Table parameter entry.
 */
typedef struct XtmTrafficDescrParmEntry
{
    UINT32 ulTrafficDescrIndex;
    UINT32 ulTrafficDescrType;
    UINT32 ulPcr;
    UINT32 ulScr;
    UINT32 ulMbs;
    UINT32 ulMcr;
    UINT32 ulServiceCategory;
} XTM_TRAFFIC_DESCR_PARM_ENTRY, *PXTM_TRAFFIC_DESCR_PARM_ENTRY;

typedef struct XtmTransmitQueueParms
{
    UINT32 ulPortId;
    UINT32 ulBondingPortId;         /* read-only. Necessary for PTM bonding and not for
                                       ATM bonding, as for PTM bonding, Flow Buffer/port
                                       intermediate between Tx DMA channels and the Utopia
                                       ports when scheduling, each flow buffer needs to be
                                       configured/port. */
    UINT32 ulPtmPriority;
    UINT32 ulWeightValue;
    UINT8  ucQosQId;
    UINT8  ucWeightAlg;             /* per packet arbitration for a PVC */
    UINT8  ucSubPriority;
    UINT8  ucDropAlg;               /* DT or RED or WRED */
    UINT8  ucLoMinThresh;           /* RED/WRED Low Class min threshold in % of queue size */
    UINT8  ucLoMaxThresh;           /* RED/WRED Low Class max threshold in % of queue size */
    UINT8  ucHiMinThresh;           /* WRED High Class min threshold in % of queue size */
    UINT8  ucHiMaxThresh;           /* WRED High Class max threshold in % of queue size */
    UINT32 ulMinBitRate;            /* 0 indicates no shaping */
    UINT32 ulShapingRate;           /* 0 indicates no shaping */
    UINT16 usShapingBurstSize;
#if defined(CONFIG_BCM963158)
    UINT32 usSize;
#else    
    UINT16 usSize;
#endif    
    UINT32 ulTxQueueIdx;            /* transmit channel index */
} XTM_TRANSMIT_QUEUE_PARMS, *PXTM_TRANSMIT_QUEUE_PARMS;

typedef struct XtmConnArb
{
    UINT32 ulWeightAlg;             /* per cell arbitration among PVCs */
    UINT32 ulWeightValue;
    UINT32 ulSubPriority;
} XTM_CONN_ARB, *PXTM_CONN_ARB;

typedef struct XtmConnCfg
{
    UINT32 ulAtmAalType;
    UINT32 ulAdminStatus;
    UINT32 ulOperStatus;            /* read only */
    UINT32 ulLastChange;            /* read only */

    UINT32 ulTransmitTrafficDescrIndex;
    UINT32 ulHeaderType;
    XTM_CONN_ARB ConnArbs[MAX_PHY_PORTS][MAX_PTM_PRIORITIES];
    UINT32 ulTransmitQParmsSize;
    XTM_TRANSMIT_QUEUE_PARMS TransmitQParms[MAX_TRANSMIT_QUEUES];
} XTM_CONN_CFG, *PXTM_CONN_CFG;

typedef struct XtmErrorStats 
{
	/* NOTE: Some fields not supported in impl1 of driver due to
	   differences in hardware registers.  Those fields are zeroed
	   in that implementation */
    UINT32 ulPafErrs;			/* Not supported in impl1 */
    UINT32 ulPafLostFragments;	/* Not supported in impl1 */
    UINT32 ulOverflowErrorsRx;	/* Not supported in impl1 */
    UINT32 ulFramesDropped;
} XTM_ERROR_STATS, *PXTM_ERROR_STATS;

typedef struct XtmInterfaceStats
{
    UINT32 ulIfInOctets;
    UINT32 ulIfOutOctets;
    UINT32 ulIfInPackets;
    UINT32 ulIfOutPackets;
    UINT32 ulIfInOamRmCells;
    UINT32 ulIfOutOamRmCells;
    UINT32 ulIfInAsmCells;
    UINT32 ulIfOutAsmCells;
    UINT32 ulIfInCellErrors;
    UINT32 ulIfInPacketErrors;
} XTM_INTERFACE_STATS, *PXTM_INTERFACE_STATS;

typedef struct XtmInterfaceLinkInfo
{
    UINT32 ulLinkState;
    UINT32 ulLinkUsRate;
    UINT32 ulLinkDsRate;
    UINT32 ulLinkTrafficType;
} XTM_INTERFACE_LINK_INFO, *PXTM_INTERFACE_LINK_INFO;

typedef struct XtmOamCellInfo
{
    UINT8 ucCircuitType;
    UINT32 ulTimeout;
    UINT32 ulRepetition;
    UINT32 ulSent;
    UINT32 ulReceived;
    UINT32 ulMinRspTime;
    UINT32 ulMaxRspTime;
    UINT32 ulAvgRspTime;
} XTM_OAM_CELL_INFO, *PXTM_OAM_CELL_INFO;

typedef struct _XtmPortInfo {
   UINT32 ulInterfaceId ;
   UINT32 linkState ;
   UINT32 usRate ;          /* in bps */
   UINT32 dsRate ;          /* in bps */
   UINT32 usDelay ;         /* in milli sec */
   UINT32 dsBondingDelay ;  /* in milli sec */
   UINT32 rcvdGrpId;        /* Bonding Group Id */ 
   int    localTxLineSt ;   /* local TxLink Status */
   int    localRxLineSt ;   /* local RxLink Status */
} XTM_PORT_INFO, *PXTM_PORT_INFO ;

typedef struct XtmBondGroupInfo {
    UINT32        ulGroupId ;
    XTM_PORT_INFO portInfo [MAX_BOND_PORTS] ;
    UINT32        aggrUSRate ;
    UINT32        aggrDSRate ;
    UINT32        diffUSDelay ;
    UINT32        dataStatus ;
} XTM_BOND_GROUP_INFO, *PXTM_BOND_GROUP_INFO ;

typedef struct XtmBondInfo {
    UINT8               u8MajorVersion ;
    UINT8               u8MinorVersion ;
    UINT16              u8BuildVersion ;
    UINT32              ulTrafficType ;
    UINT32              ulBondProto ;
    UINT32              ulNumGroups ;
    UINT32              ulTxPafEnabled;
    XTM_BOND_GROUP_INFO grpInfo [MAX_BOND_GROUPS] ;
} XTM_BOND_INFO, *PXTM_BOND_INFO ;


/***************************************************************************
 * Function Prototypes
 ***************************************************************************/

#ifndef FAP_4KE

BCMXTM_STATUS BcmXtm_Initialize( PXTM_INITIALIZATION_PARMS pInitParms );
#if defined(CONFIG_BCM_55153_DPU)
BCMXTM_STATUS BcmXtm_Initialize_DPU( PXTM_INITIALIZATION_PARMS pInitParms );
#endif   //CONFIG_BCM_55153_DPU
BCMXTM_STATUS BcmXtm_Uninitialize( void );
BCMXTM_STATUS BcmXtm_Configure( PXTM_CONFIGURATION_PARMS pConfigParms );
BCMXTM_STATUS BcmXtm_ManageThreshold( PXTM_THRESHOLD_PARMS pThresholdParms );
BCMXTM_STATUS BcmXtm_GetTrafficDescrTable( PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescTable, UINT32 *pulTrafficDescrTableSize );
BCMXTM_STATUS BcmXtm_SetTrafficDescrTable( PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescTable, UINT32  ulTrafficDescrTableSize );
BCMXTM_STATUS BcmXtm_GetInterfaceCfg( UINT32 ulPortId, PXTM_INTERFACE_CFG
    pInterfaceCfg );
BCMXTM_STATUS BcmXtm_SetInterfaceCfg( UINT32 ulPortId, PXTM_INTERFACE_CFG
    pInterfaceCfg );
BCMXTM_STATUS BcmXtm_GetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg );
BCMXTM_STATUS BcmXtm_SetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg );
BCMXTM_STATUS BcmXtm_GetConnAddrs( PXTM_ADDR pConnAddrs, UINT32 *pulNumConns );
BCMXTM_STATUS BcmXtm_GetInterfaceStatistics( UINT32 ulPortId,
    PXTM_INTERFACE_STATS pStatistics, UINT32 ulReset );
BCMXTM_STATUS BcmXtm_SetInterfaceLinkInfo( UINT32 ulPortId,
    PXTM_INTERFACE_LINK_INFO pLinkInfo );
BCMXTM_STATUS BcmXtm_SendOamCell( PXTM_ADDR pConnAddr,
    PXTM_OAM_CELL_INFO pOamCellInfo);
BCMXTM_STATUS BcmXtm_CreateNetworkDevice( PXTM_ADDR pConnAddr,
    char *pszNetworkDeviceName );
BCMXTM_STATUS BcmXtm_DeleteNetworkDevice( PXTM_ADDR pConnAddr );
BCMXTM_STATUS BcmXtm_GetBondingInfo ( PXTM_BOND_INFO pBondingInfo) ;
BCMXTM_STATUS BcmXtm_ReInitialize( void );
BCMXTM_STATUS BcmXtm_SetDsPtmBondingDeviation ( UINT32 ulDeviation ) ;
BCMXTM_STATUS BcmXtm_GetErrorStatistics( PXTM_ERROR_STATS pStatistics );

#define XTM_USE_DSL_MIB       /* needed for dsl line monitoring */

#define XTM_USE_DSL_WAN_NOTIFY

#define XTM_SUPPORT_DSL_SRA

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#define XTM_USE_DSL_SYSCTL    /* needed for XTM traffic/mode sensing functionality */

/* Support for non bonding connection on port 1*/
#define XTM_SUPPORT_PORT1_NONBONDCONN
#endif

#endif   /* FAP_4KE */

#define XTMRT_PTM_BOND_FRAG_HDR_SIZE         2
#if 1
#define XTMRT_PTM_BOND_MAX_FRAG_PER_PKT      8
#else
/* This is only to test 63268 D0 additional capability, which is not used currently. */
#define XTMRT_PTM_BOND_MAX_FRAG_PER_PKT      24 
#endif

#define XTM_PTM_BOND_HEADER_LEN             (XTMRT_PTM_BOND_MAX_FRAG_PER_PKT*XTMRT_PTM_BOND_FRAG_HDR_SIZE)

#define XTM_MODE_NITRO_DISABLE                0x0
#define XTM_MODE_NITRO_ENABLE                 0x1

/* Defaults per mode. Mode here implies the type of DSL modes that have impact
** in the */

#define XTM_LINK_MODE_UNKNOWN                 0x0
#define XTM_LINK_MODE_ADSL                    0x1
#define XTM_LINK_MODE_VDSL                    0x2
#define XTM_LINK_MODE_VDSL_RTX                0x4
#define XTM_LINK_MODE_GFAST                   0x8

#define XTM_UNKNOWN_MODE_QUEUE_DROP_THRESHOLD   0    /* Unknown Mode               */
#define XTM_XDSL_MODE_MIN_THRESHOLD            32    /* Min Threshold              */
#define XTM_XDSL_MODE_MAX_THRESHOLD         32768    /* Max Threshold              */
#define XTM_ADSL_QUEUE_DROP_THRESHOLD          64    /* ADSL ATM,PTM, PTM_BONDED   */
#define XTM_VDSL_QUEUE_DROP_THRESHOLD         128    /* VDSL PTM, PTM_BONDED       */
#define XTM_VDSL_RTX_QUEUE_DROP_THRESHOLD     512    /* VDSL PTM, PTM_BONDED, ReTx */
#define XTM_GFAST_QUEUE_DROP_THRESHOLD       1024    /* GFAST PTM, PTM_BONDED      */

#define XTMCFG_QUEUE_DROP_THRESHOLD          XTM_VDSL_RTX_QUEUE_DROP_THRESHOLD /* For Non RDP/XRDP platforms. For RDP/XRDP platforms, the above feature of
                                                                                  varying thresholds based on mode is active */
/* in ms */

/*
In rtx mode (G.inp and G.fast) you don't have fixed delay between the lines, so maxBondlingDelay from DSL MIB is useless. 
Any line can fall behind when rtx start. Unfortunately rtx maxDelay is not reported by the PHY so we'll have to go with max delay allowed by the standard which for 
  G.inp   is 64ms   (but per SJC team, with SRA the delay will be twice as much so  128ms)
  G.fast     16ms
 
To give extra margin let's use
  G.inp       160ms   0x16E 
  G.fast       48ms    0x6E
 
and that's for both lines.
*/

#define XTM_UNKNOWN_MODE_BONDING_DELAY          0    /* Unknown Mode */
#define XTM_ADSL_MODE_BONDING_DELAY            10    /* ADSL ATM_BONDED, PTM_BONDED */
#define XTM_VDSL_MODE_BONDING_DELAY            16    /* PTM_BONDED */
#define XTM_VDSL_RTX_BONDING_DELAY             160   /* PTM_BONDED, ReTx */
#define XTM_GFAST_BONDING_DELAY                48    /* GFAST PTM_BONDED */

#if defined(__cplusplus)
}
#endif

#endif /* _BCMXTMCFG_H_ */

