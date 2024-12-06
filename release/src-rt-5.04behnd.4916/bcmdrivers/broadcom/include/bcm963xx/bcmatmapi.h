/*
    Copyright 2007-2015 Broadcom Corporation

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

//**************************************************************************
// File Name  : BcmAtmApi.h
//
// Description: This file contains the definitions, structures and function
//              prototypes for the Broadcom Asynchronous Transfer Mode (ATM)
//              Application Program Interface (API).
//
// Updates    : 09/15/2000  lat.  Created.
//**************************************************************************

#if !defined(_BCMATMAPI_H_)
#define _BCMATMAPI_H_

//define CONFIG_ATM_EOP_MONITORING

#if defined(__cplusplus)
extern "C" {
#endif

//**************************************************************************
// Constant Definitions
//**************************************************************************

#if defined(CONFIG_MIPS_BRCM)
#define ATM_CACHE_SMARTFLUSH
#endif

/* Values for traffic type */
#define ATM_TRAFFIC 0
#define PTM_TRAFFIC 1

// ATM physical port constants.
#define PHY_NUM_PORTS                       4
#define PHY_0                               0
#define PHY_1                               1
#define PHY_2                               2 // [BCM635x Only]
#define PHY_3                               3 // [BCM635x Only]

// Used for backwards compatibility.
#define PHY_UTOPIA0                         0
#define PHY_UTOPIA1                         1
#define PHY_UTOPIA2                         2
#define PHY_UTOPIA3_TC_LOOPBACK             3

// Values for ATM_PORT_CFG ucPortType.
#define PT_DISABLED                         0
#define PT_UTOPIA                           1
#define PT_LOOPBACK                         2
#define PT_TC                               3 // [BCM635x Only]
#define PT_ADSL_INTERLEAVED                 4 // [BCM6345 Only]
#define PT_ADSL_FAST                        5 // [BCM6345 Only]

// Values for ATM_PORT_CFG ucPortFlags.
#define PF_UTOPIA_LEVEL_2                   0x01
#define PF_UTOPIA_NEG_EDGE_SEL              0x02

// Wildcard definitions.
#define ALL_INTERFACES                      0xffffffff
#define ANY_PRIORITY                        0xff

// Values for priority packet groups.
#define NUM_PRI_PKT_GROUPS                  16
#define NUM_ENTRIES_PER_PRI_PKT_GROUP       6

// Values for ATM_TRAFFIC_DESCR_PARM_ENTRY ulTrafficDescrType.
#define TDT_ATM_NO_TRAFFIC_DESCRIPTOR       1
#define TDT_ATM_NO_CLP_NO_SCR               2
#define TDT_ATM_CLP_NO_TAGGING_NO_SCR       3
#define TDT_ATM_CLP_TAGGING_NO_SCR          4
#define TDT_ATM_NO_CLP_SCR                  5
#define TDT_ATM_CLP_NO_TAGGING_SCR          6 // [BCM635x Only]
#define TDT_ATM_CLP_TAGGING_SCR             7 // [BCM635x Only]
#define TDT_ATM_CLP_TRANSPARENT_NO_SCR      9
#define TDT_ATM_CLP_TRANSPARENT_SCR         10
#define TDT_ATM_NO_CLP_TAGGING_NO_SCR       11
#define TDT_ATM_NO_CLP_NO_SCR_CDVT          12
#define TDT_ATM_NO_CLP_SCR_CDVT             13
#define TDT_ATM_CLP_NO_TAGGING_SCR_CDVT     14 // [BCM635x Only]
#define TDT_ATM_CLP_TAGGING_SCR_CDVT        15 // [BCM635x Only]

// Values for ATM_TRAFFIC_DESCR_PARM_ENTRY ulTrafficDescrRowStatus.
#define TDRS_ACTIVE                         1
#define TDRS_NOT_IN_SERVICE                 2

// Values for ATM_TRAFFIC_DESCR_PARM_ENTRY ulServiceCategory.
#define SC_OTHER                            1
#define SC_CBR                              2
#define SC_RT_VBR                           3
#define SC_NRT_VBR                          4
#define SC_UBR                              6

// Values for ATM_INTERFACE_CFG ulIfAdminStatus and ATM_VCC_CFG
// ulAtmVclAdminStatus.
#define ADMSTS_UP                           1
#define ADMSTS_DOWN                         2
#define ADMSTS_TESTING                      3

// Values for ATM_INTERFACE_CFG ulIfOperStatus and ATM_VCC_CFG
// ulAtmVclOperStatus.
#define OPRSTS_UP                           1
#define OPRSTS_DOWN                         2
#define OPRSTS_UNKNOWN                      3

// Values for ATM_INTERFACE_LINK_INFO ulLinkState.
#define LINK_UP                             1
#define LINK_DOWN                           2

// Values for ulAalType.
#define AAL_2                               0 // [BCM635x Only]
#define AAL_TRANSPARENT                     1
#define AAL_0_PACKET                        2
#define AAL_0_CELL_CRC                      3
#define AAL_5                               7

// Values for ATM_VCC_CFG ulAtmVccEncapsType.
#define ET_VC_MULTIPLEX_ROUTED_PROTOCOL     1
#define ET_VC_MULTIPLEX_BRG_PROTOCOL_8023   2
#define ET_VC_MULTIPLEX_BRG_PROTOCOL_8025   3
#define ET_VC_MULTIPLEX_BRG_PROTOCOL_8026   4
#define ET_VC_MULTIPLEX_LAN_EMULATION_8023  5
#define ET_VC_MULTIPLEX_LAN_EMULATION_8025  6
#define ET_LLC_ENCAPSULATION                7
#define ET_MULTI_PROTOCOL_FRAME_RELAY_SSCS  8
#define ET_OTHER                            9
#define ET_UNKNOWN                          10

// [BCM635x Only] Values for ATM_AAL2_VCC_CFG ucAal2CpsOptimisation.
#define OPT_SNG_PKT_PER_PDU_NO_OVERLAP      1
#define OPT_MULT_PKTS_PER_PDU_OVERLAP       2

// [BCM635x Only] Values for ATM_INTERFACE_STATS ulTcAlarmState.
#define TCALM_NO_ALARM                      1
#define TCALM_LCD_FAILURE                   2

// Values for ATM_NOTIFY_PARMS ulNotificationType.
#define ATM_NOTIFY_INTERFACE_CHANGE         1

// Values for AN_INTF_CHANGE_PARMS ulInterfaceState.
#define ATM_INTERFACE_UP                    1
#define ATM_INTERFACE_DOWN                  2

// Values for AN_VCC_CHANGE_PARMS ulInterfaceState.
#define ATM_VCC_UP                          1
#define ATM_VCC_DOWN                        2

// Values for ATM_VCC_ATTACH_PARMS ulFlags.
#define AVAP_ALLOW_OAM_F5_SEGMENT_CELLS     0x0001
#define AVAP_ALLOW_OAM_F5_END_TO_END_CELLS  0x0002
#define AVAP_ALLOW_RM_CELLS                 0x0004
#define AVAP_ALLOW_OAM_F4_SEGMENT_CELLS     0x0008
#define AVAP_ALLOW_OAM_F4_END_TO_END_CELLS  0x0010
#define AVAP_ALLOW_CELLS_WITH_ERRORS        0x0020
#define AVAP_ADD_AAL0_CRC10                 0x0040
#define AVAP_DSP                            0x8000 // [BCM635x Only]

// [BCM635x Only] Values for ATM_VCC_AAL2_CHANNEL_ID_PARMS ucVoiceRouting.
#define VOICE_ROUTE_MIPS                    0
#define VOICE_ROUTE_DSP                     2

// [BCM635x Only] Values for ATM_VCC_AAL2_CHANNEL_ID_PARMS ucFlags.
#define CID_USE_FRAME_MODE                  0x01

// Values for ATM_VCC_DATA_PARMS ucCircuitType.
#define CT_TRANSPARENT                      0x01
#define CT_AAL0_PACKET                      0x02
#define CT_AAL0_CELL_CRC                    0x03
#define CT_OAM_F5_SEGMENT                   0x04
#define CT_OAM_F5_END_TO_END                0x05
#define CT_RM                               0x06
#define CT_AAL5                             0x07
#define CT_ANY_AAL2_MASK                    0x08 // [BCM635x Only]
#define CT_AAL2_ALARM                       0x08 // [BCM635x Only]
#define CT_AAL2_TYPE_3                      0x09 // [BCM635x Only]
#define CT_AAL2_TYPE_1                      0x0A // [BCM635x Only]
#define CT_AAL2_FRAME                       0x0B // [BCM635x Only]
#define CT_OAM_TRANSPARENT                  0x10
#define CT_OAM_F4_ANY                       0x20
#define CT_BONDING_ASM                      0x40 // [BCM635x Only - Managed by SW completely]

// OAM F4 VCI values.
#define VCI_OAM_F4_SEGMENT                  3
#define VCI_OAM_F4_END_TO_END               4
#define VCI_RM                              6

// Values for ATM_VCC_DATA_PARMS ucFlags.
#define ATMDATA_CI                          0x04
#define ATMDATA_CLP                         0x08
#define ATMDATA_FROM_XMIT_CLEAN             0x10

// [BCM635x Only] DSP specific values.
#define DSP_VCID                            31

// ATM cell layer interface name
#define ATM_CELL_LAYER_IFNAME               "atm0"

// AAL5 CPCS layer interface name
#define AAL5_CPCS_LAYER_IFNAME              "cpcs0"

//**************************************************************************
// Type Definitions
//**************************************************************************

// Return status values
typedef enum BcmAtmStatus
{
    STS_SUCCESS = 0,
    STS_ERROR,
    STS_STATE_ERROR,
    STS_PARAMETER_ERROR,
    STS_ALLOC_ERROR,
    STS_RESOURCE_ERROR,
    STS_IN_USE,
    STS_VCC_DOWN,
    STS_INTERFACE_DOWN,
    STS_LINK_DOWN,
    STS_NOT_FOUND,
    STS_NOT_SUPPORTED,
    STS_VCAM_MULT_MATCH_ERROR,
    STS_CCAM_MULT_MATCH_ERROR,
    STS_PKTERR_INVALID_VPI_VCI,
    STS_PKTERR_PORT_NOT_ENABLED,
    STS_PKTERR_HEC_ERROR,
    STS_PKTERR_PTI_ERROR,
    STS_PKTERR_RECEIVED_IDLE_CELL,
    STS_PKTERR_CIRCUIT_TYPE_ERROR,
    STS_PKTERR_OAM_RM_CRC_ERROR,
    STS_PKTERR_GFC_ERROR,
    STS_PKTERR_AAL5_AAL0_CRC_ERROR,
    STS_PKTERR_AAL5_AAL0_SHORT_PKT_ERROR,
    STS_PKTERR_AAL5_AAL0_LENGTH_ERROR,
    STS_PKTERR_AAL5_AAL0_BIG_PKT_ERROR,
    STS_PKTERR_AAL5_AAL0_SAR_TIMEOUT_ERROR,
    STS_PKTERR_AAL2F_HEC_ERROR,
    STS_PKTERR_AAL2F_SEQ_NUM_ERROR,
    STS_PKTERR_AAL2F_PARITY_ERROR,
    STS_PKTERR_AAL2F_CRC_ERROR,
    STS_PKTERR_AAL2F_CAM_ERROR,
    STS_PKTERR_AAL2F_BIG_PKT_ERROR,
    STS_PKTERR_AAL2F_RAS_TIMEOUT_ERROR,
    STS_PKTERR_AAL2F_SHORT_PKT_ERROR,
    STS_PKTERR_AAL2F_LENGTH_MISMATCH_ERROR,
    STS_PKTERR_AAL2V_HEC_ERROR,
    STS_PKTERR_AAL2V_SEQ_NUM_ERROR,
    STS_PKTERR_AAL2V_PARITY_ERROR,
    STS_PKTERR_AAL2V_CRC_ERROR,
    STS_PKTERR_AAL2V_CAM_ERROR,
    STS_PKTERR_AAL2V_OSF_MISMATCH_ERROR,
    STS_PKTERR_AAL2V_OSF_ERROR,
    STS_PKTERR_AAL2V_HEC_OVERLAP_ERROR,
    STS_PKTERR_AAL2V_BIG_PKT_ERROR,
    STS_PKTERR_AAL2V_RAS_ERROR,
    STS_PKTERR_AAL2V_UUI_ERROR
} BCMATM_STATUS;


// ATM_VCC_ADDR identifies a Virtual Channel Connection (VCC).
typedef struct AtmVccAddr
{
    UINT32 ulInterfaceId;
    UINT16 usVpi;
    UINT16 usVci;
} ATM_VCC_ADDR, *PATM_VCC_ADDR;


// ATM_PORT_CFG contains ATM physical port configuration parameters.
typedef struct AtmPortCfg
{
    UINT32 ulInterfaceId;
    UINT8 ucPortType;
    UINT8 ucPortFlags;
    UINT8 ucReserved[2];
} ATM_PORT_CFG, *PATM_PORT_CFG;

/* note the buffer will have the following pattern:

   uint8 *ptr to next

*/

// ATM_INITIALIZATION_PARMS contains ATM API module initialization parameters.
#define ID_ATM_INITIALIZATION_PARMS         2
typedef struct AtmInitialization
{
    UINT32 ulStructureId;
    UINT32 ulThreadPriority;
    UINT16 usFreeCellQSize;
    UINT16 usFreePktQSize;
    UINT16 usFreePktQBufferSize;
    UINT16 usFreePktQBufferOffset; // offset into buffer to start receiving data
    UINT16 usReceiveCellQSize;
    UINT16 usReceivePktQSize;
    UINT8  ucTransmitFifoPriority; // [BCM635x Only]
    UINT8  ucPrioritizeReceivePkts;
    UINT16 usAal5CpcsMaxSduLength;
    UINT16 usAal2SscsMaxSsarSduLength; // [BCM635x Only]
    ATM_PORT_CFG PortCfg[PHY_NUM_PORTS];
} ATM_INITIALIZATION_PARMS, *PATM_INITIALIZATION_PARMS;


// ATM_PRIORITY_PACKET_ENTRY contains fields for identifying a received packet.
#define ID_ATM_PRIORITY_PACKET_ENTRY        1
typedef struct AtmPriorityPacketEntry
{
    UINT32 ulStructureId;
    UINT32 ulPacketOffset;
    UINT16 usPacketValue;
    UINT16 usValueMask;
} ATM_PRIORITY_PACKET_ENTRY, *PATM_PRIORITY_PACKET_ENTRY;


// ATM_TRAFFIC_DESCR_PARM_ENTRY contains the fields needed to create a Traffic
// Descriptor Table parameter entry.
#define ID_ATM_TRAFFIC_DESCR_PARM_ENTRY     1
typedef struct AtmTrafficDescrParmEntry
{
    UINT32 ulStructureId;
    UINT32 ulTrafficDescrIndex;
    UINT32 ulTrafficDescrType;
    UINT32 ulTrafficDescrParm1;
    UINT32 ulTrafficDescrParm2;
    UINT32 ulTrafficDescrParm3;
    UINT32 ulTrafficDescrParm4;
    UINT32 ulTrafficDescrParm5;
    UINT32 ulTrafficDescrRowStatus;
    UINT32 ulServiceCategory;
    UINT32 ulTrafficFrameDiscard;
} ATM_TRAFFIC_DESCR_PARM_ENTRY, *PATM_TRAFFIC_DESCR_PARM_ENTRY;


// ATM_INTERFACE_CFG contains configuration fields for an ATM interface.
#define ID_ATM_INTERFACE_CFG                4
typedef struct AtmInterfaceCfg
{
    UINT32 ulStructureId;
    UINT32 ulAtmInterfaceMaxVccs;
    UINT32 ulAtmInterfaceConfVccs;
    UINT32 ulAtmInterfaceMaxActiveVpiBits;
    UINT32 ulAtmInterfaceMaxActiveVciBits;
    UINT32 ulAtmInterfaceCurrentMaxVpiBits;
    UINT32 ulAtmInterfaceCurrentMaxVciBits;
    UINT32 ulIfAdminStatus;
    UINT32 ulIfOperStatus;                  // read-only
    UINT32 ulSendNullCells;
    UINT32 ulTcScramble;
    UINT32 ulPortType;
    UINT32 ulIfLastChange;
    UINT32 ulPortFlags;
} ATM_INTERFACE_CFG, *PATM_INTERFACE_CFG;


// ATM_VCC_TRANSMIT_QUEUE_PARMS contains fields for configuring an transmit
// queue.
#define ID_ATM_VCC_TRANSMIT_QUEUE_PARMS     1
typedef struct AtmVccTransmitQueueParms
{
    UINT32 ulStructureId;
    UINT32 ulSize;
    UINT32 ulPriority;
    UINT32 ulReserved;
} ATM_VCC_TRANSMIT_QUEUE_PARMS, *PATM_VCC_TRANSMIT_QUEUE_PARMS;


// ATM_AAL5_VCC_CFG contains configuration fields for an ATM AAL5 Virtual
// Channel Connection (VCC).
typedef struct AtmAal5VccCfg
{
    UINT32 ulAtmVccEncapsType;
    UINT32 ulAtmVccCpcsAcceptCorruptedPdus;
} ATM_AAL5_VCC_CFG, *PATM_AAL5_VCC_CFG;


// [BCM635x Only] ATM_AAL2_VCC_CFG contains configuration fields for an ATM
// AAL2 Virtual Channel Connection (VCC).
typedef struct AtmAal2VccCfg
{
    UINT8 ucAal2CpsMaxMultiplexedChannels;
    UINT8 ucAal2CpsMaxSduLength;
    UINT8 ucAal2CpsCidLowerLimit;
    UINT8 ucAal2CpsCidUpperLimit;
    UINT8 ucAal2CpsOptimisation;
    UINT8 ucReserved[3];
} ATM_AAL2_VCC_CFG, *PATM_AAL2_VCC_CFG;


// ATM_AAL0_VCC_CFG contains configuration fields for an ATM AAL0 Virtual
// Channel Connection (VCC).
typedef struct AtmAal0VccCfg
{
    UINT8 ucReserved;
    // Reserved for future use.
} ATM_AAL0_VCC_CFG, *PATM_AAL0_VCC_CFG;


// ATM_VCC_CFG contains configuration fields for an ATM Virtual Channel
// Connection (VCC).
#define ID_ATM_VCC_CFG                      3
#define TX_Q_PARM_SIZE                      8
typedef struct AtmVccCfg
{
    UINT32 ulStructureId;
    UINT32 ulAalType;
    UINT32 ulAtmVclAdminStatus;
    UINT32 ulAtmVclOperStatus;
    UINT32 ulAtmVclLastChange;
    UINT32 ulAtmVclReceiveTrafficDescrIndex;
    UINT32 ulAtmVclTransmitTrafficDescrIndex;
    UINT32 ulTransmitQParmsSize;
    UINT32 ulPriorityPacketGroupNumbers[NUM_PRI_PKT_GROUPS];
    ATM_VCC_TRANSMIT_QUEUE_PARMS TransmitQParms[TX_Q_PARM_SIZE];
    union
    {
        ATM_AAL5_VCC_CFG Aal5Cfg;
        ATM_AAL2_VCC_CFG Aal2Cfg; // [BCM635x Only]
        ATM_AAL0_VCC_CFG Aal0Cfg;
    } u;
} ATM_VCC_CFG, *PATM_VCC_CFG;


// ATM_INTF_ATM_STATS contains statistics for the ATM layer of an interface.
typedef struct AtmIntfAtmStats
{
    UINT32 ulIfInOctets;
    UINT32 ulIfOutOctets;
    UINT32 ulIfInErrors;
    UINT32 ulIfInUnknownProtos;
    UINT32 ulIfOutErrors;

    // The following fields are added together to calculate ulIfInErrors.
    UINT32 ulIfInHecErrors;

    // The following fields are added together to calculate ulIfInUnknownProtos.
    UINT32 ulIfInInvalidVpiVciErrors;
    UINT32 ulIfInPortNotEnabledErrors;
    UINT32 ulIfInPtiErrors;
    UINT32 ulIfInIdleCells;
    UINT32 ulIfInCircuitTypeErrors;
    UINT32 ulIfInOamRmCrcErrors;
    UINT32 ulIfInGfcErrors;
} ATM_INTF_ATM_STATS, *PATM_INTF_ATM_STATS;


// [BCM635x Only] ATM_INTF_ATM_STATS contains statistics for the TC layer.
typedef struct AtmIntfTcStats
{
    UINT32 ulTcInDataCells;
    UINT32 ulTcInTotalCells;
    UINT32 ulTcInHecErrors;
    UINT32 ulTcInOcdEvents;
    UINT32 ulTcAlarmState;
} ATM_INTF_TC_STATS, *PATM_INTF_TC_STATS;


// ATM_INTF_AAL5_AAL0_STATS contains statistics for all AAL5/AAL0 VCCs on an
// ATM interface.
typedef struct AtmIntfAal5Aal0Stats
{
    UINT32 ulIfInOctets;
    UINT32 ulIfOutOctets;
    UINT32 ulIfInUcastPkts;
    UINT32 ulIfOutUcastPkts;
    UINT32 ulIfInErrors;
    UINT32 ulIfOutErrors;
    UINT32 ulIfInDiscards;
    UINT32 ulIfOutDiscards;
    UINT32 ulIfInPriPkts;
} ATM_INTF_AAL5_AAL0_STATS, *PATM_INTF_AAL5_AAL0_STATS;


// [BCM635x Only] ATM_INTF_AAL2_STATS contains statistics for all AAL2 VCCs
// on an ATM interface.
typedef struct AtmIntfAal2Stats
{
    UINT32 ulIfInOctets;
    UINT32 ulIfOutOctets;
    UINT32 ulIfInUcastPkts;
    UINT32 ulIfOutUcastPkts;
    UINT32 ulIfInErrors;
    UINT32 ulIfOutErrors;
    UINT32 ulIfInDiscards;
    UINT32 ulIfOutDiscards;
} ATM_INTF_AAL2_STATS, *PATM_INTF_AAL2_STATS;


// ATM_INTERFACE_STATS contains statistics for an ATM interface.
#define ID_ATM_INTERFACE_STATS              2
typedef struct AtmInterfaceStats
{
    UINT32 ulStructureId;
    ATM_INTF_ATM_STATS       AtmIntfStats;
    ATM_INTF_TC_STATS        TcIntfStats;   // [BCM635x Only]
    ATM_INTF_AAL5_AAL0_STATS Aal5IntfStats;
    ATM_INTF_AAL2_STATS      Aal2IntfStats; // [BCM635x Only]
    ATM_INTF_AAL5_AAL0_STATS Aal0IntfStats;
} ATM_INTERFACE_STATS, *PATM_INTERFACE_STATS;


// ATM_VCC_AAL5_STATS contains statistics for an AAL5 VCC.
typedef struct AtmVccAal5Stats
{
    UINT32 ulAal5VccCrcErrors;
    UINT32 ulAal5VccSarTimeOuts;
    UINT32 ulAal5VccOverSizedSdus;
    UINT32 ulAal5VccShortPacketErrors;
    UINT32 ulAal5VccLengthErrors;
} ATM_VCC_AAL5_STATS, *PATM_VCC_AAL5_STATS;


// [BCM635x Only] ATM_VCC_AAL2_STATS contains statistics for an AAL2 VCC.
typedef struct AtmVccAal2Stats
{
    UINT32 ulAal2CpsInPkts;
    UINT32 ulAal2CpsOutPkts;
    UINT32 ulAal2CpsParityErrors;
    UINT32 ulAal2CpsSeqNumErrors;
    UINT32 ulAal2CpsOsfMismatchErrors;
    UINT32 ulAal2CpsOsfErrors;
    UINT32 ulAal2CpsHecOverlapErrors;
    UINT32 ulAal2CpsHecErrors;
    UINT32 ulAal2CpsOversizedSduErrors;
    UINT32 ulAal2CpsReassemblyErrors;
    UINT32 ulAal2CpsUuiErrors;
    UINT32 ulAal2CpsCidErrors;
    UINT32 ulAal2SscsOversizedSssarSduErrors;
    UINT32 ulAal2SscsSssarRasTimerExipiryErrors;
    UINT32 ulAal2SscsUndersizedSstedPduErrors;
    UINT32 ulAal2SscsSstedPduLengthMismatchErrors;
    UINT32 ulAal2SscsSstedCrcMismatchErrors;
} ATM_VCC_AAL2_STATS, *PATM_VCC_AAL2_STATS;


// ATM_VCC_AAL0PKT_STATS contains statistics for an AAL0 Packet VCC.
typedef struct AtmVccAal0PktStats
{
    UINT32 ulAal0VccSarTimeOuts;
    UINT32 ulAal0VccOverSizedSdus;
} ATM_VCC_AAL0PKT_STATS, *PATM_VCC_AAL0PKT_STATS;


// ATM_VCC_AAL0CELL_STATS contains statistics for an AAL0 Cell with CRC VCC.
typedef struct AtmVccAal0CellStats
{
    UINT32 ulAal0VccCrcErrors;
} ATM_VCC_AAL0CELL_STATS, *PATM_VCC_AAL0CELL_STATS;


// ATM_VCC_STATS contains statistics for a VCC.
#define ID_ATM_VCC_STATS                    1
typedef struct AtmVccStatistics
{
    UINT32 ulStructureId;
    UINT32 ulAalType;
    union
    {
        ATM_VCC_AAL5_STATS AtmVccAal5Stats;
        ATM_VCC_AAL2_STATS AtmVccAal2Stats; // [BCM635x Only]
        ATM_VCC_AAL0PKT_STATS AtmVccAal0PktStats;
        ATM_VCC_AAL0CELL_STATS AtmVccAal0CellStats;
    } u;
} ATM_VCC_STATS, *PATM_VCC_STATS;


// ATM_INTERFACE_LINK_INFO contains fields for the physical link that the
// ATM interface is using.
#define ID_ATM_INTERFACE_LINK_INFO          1
typedef struct AtmInterfaceLinkInfo
{
    UINT32 ulStructureId;
    UINT32 ulLinkState;
    UINT32 ulLineRate;
    UINT32 ulReserved[2];
} ATM_INTERFACE_LINK_INFO, *PATM_INTERFACE_LINK_INFO;


// AN_INTF_CHANGE_PARMS contains notification fields that passed to an
// application callback function when the ATM interface goes up or down.
#define ID_AN_INTF_CHANGE_PARMS             1
typedef struct AnIntfChangeParms
{
    UINT32 ulInterfaceId;
    UINT32 ulInterfaceState;
    UINT32 ulInterfaceLineRate;
} AN_INTF_CHANGE_PARMS, *PAN_INTF_CHANGE_PARMS;


// ATM_NOTIFY_PARMS contains notification fields that passed to an application
// callback function when an ATM notification event occurs.
typedef struct AtmNotifyParms
{
    UINT32 ulNotifyType;
    union
    {
        AN_INTF_CHANGE_PARMS IntfChangeParms;

        // Other fields and structures that are specific
        // to the type of notification are declared here.
    } u;
} ATM_NOTIFY_PARMS, *PATM_NOTIFY_PARMS;

typedef void (*FN_NOTIFY_CB) (PATM_NOTIFY_PARMS pNotifyParms);


// ATM_VCC_ATTACH_PARMS contains fields for attaching to a VCC.  It is used
// by all BcmAtm_Attach... functions.
struct AtmVccDataParms;
typedef void (*FN_RECEIVE_CB) (UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    struct AtmVccDataParms *pDataParms, UINT32 ulParmReceiveData);

#define ID_ATM_VCC_ATTACH_PARMS             1
typedef struct AtmAttachParms
{
    UINT32 ulStructureId;
    UINT32 ulFlags;
    FN_RECEIVE_CB pFnReceiveDataCb;
    UINT32 ulParmReceiveData;
    ATM_VCC_TRANSMIT_QUEUE_PARMS *pTransmitQParms;
    UINT32 ulTransmitQParmsSize;
    UINT32 ulHandle;
    UINT32 ulReserved;
} ATM_VCC_ATTACH_PARMS, *PATM_VCC_ATTACH_PARMS;


// [BCM635x Only] ATM_VCC_AAL2_CHANNEL_ID_PARMS contains fields for
// configuring an transmit queue.
#define ID_ATM_VCC_AAL2_CHANNEL_ID_PARMS    1
typedef struct AtmVccAal2ChannelIdParms
{
    UINT32 ulStructureId;
    UINT8 ucChannelId;
    UINT8 ucVoiceRouting;
    UINT8 ucFlags;
    UINT8 ucReserved[5];
} ATM_VCC_AAL2_CHANNEL_ID_PARMS, *PATM_VCC_AAL2_CHANNEL_ID_PARMS;


// ATM_BUFFER contains fields for passing data to, and receive data from, the
// ATM API.
typedef struct AtmBuffer
{
    struct AtmBuffer *pNextAtmBuf;
    UINT8 *pDataBuf;
    UINT32 ulDataLen;
    UINT16 usDataOffset;
    UINT16 usReserved;
    UINT32 ulReserved;
} ATM_BUFFER, *PATM_BUFFER;


// ATM_VCC_DATA_PARMS contains fields for sending or receiving data on a VCC.
// It is used by all BcmAtm_Send... and receive functions.
typedef void (*FN_FREE_DATA_PARMS) (struct AtmVccDataParms *pDataParms);

#define ID_ATM_VCC_DATA_PARMS               2
typedef struct AtmVccDataParms
{
    UINT32 ulStructureId;
    UINT8 ucCircuitType;
    UINT8 ucAal2ChannelId; // [BCM635x Only]
    UINT8 ucUuData8;
    UINT8 ucUuData5;
    UINT8 ucFlags;
    UINT8 ucSendPriority;
    UINT8 ucReserved[2];
    BCMATM_STATUS baReceiveStatus;
    PATM_BUFFER pAtmBuffer;
    FN_FREE_DATA_PARMS pFnFreeDataParms;
    UINT32 ulParmFreeDataParms;
    struct AtmVccDataParms *pApplicationLink;
    UINT32 ulApplicationDefined[2];
} ATM_VCC_DATA_PARMS, *PATM_VCC_DATA_PARMS;

// Flag bits for ATM_DMA_BD ulFlags_NextRxBd
#define BD_FLAG_EOP                 0x80000000 // End Of Packet
#define BD_FLAG_CLP                 0x40000000 // Cell Loss Priority
#define BD_FLAG_CI                  0x20000000 // Congestion Indicator
#define BD_FLAG_NEG                 0x10000000 // Negative length

// BD bit twiddling.
#define BD_CT_SHIFT                 27
#define BD_FLAGS_SHIFT              27
#define BD_ADDR_MASK                0x07ffffff
#define BD_SET_ADDR(F,V)            F = ((F & ~BD_ADDR_MASK) | \
                                        NONCACHE_TO_PHYS((UINT32) (V)))
#define BD_SET_CT(F,V)              F = ((F & BD_ADDR_MASK) | \
                                        ((UINT32) (V) << BD_CT_SHIFT))
#define BD_GET_CADDR(T,F) \
    ((F) & BD_ADDR_MASK) ? T (PHYS_TO_CACHE((F) & BD_ADDR_MASK)) : NULL
#define BD_GET_NCADDR(T,F) \
    ((F) & BD_ADDR_MASK) ? T (PHYS_TO_NONCACHE((F) & BD_ADDR_MASK)) : NULL
#define BD_SET_RX_RBL(BD,V) \
    BD->ucRxAalErrors_RblHigh = (BD->ucRxAalErrors_RblHigh & 0x8e) | ((V)>>3); \
    BD->ucRxRblLow  = (BD->ucRxRblLow  & 0x1f) | (((V) & 0x07) << 5)
#define BD_GET_RX_RBL(BD) \
    (((BD->ucRxAalErrors_RblHigh & 0x01) << 3) | ((BD->ucRxRblLow & 0xe0) >> 5)

#define BD_TX_PORT_ID_MASK          0x30
#define BD_TX_PORT_ID_SHIFT         4

#define BD_RX_AAL_ERROR_MASK        (0xf0 & ~RXAAL5AAL0_LENGTH_ERROR)

// ATM Error Indicators.
#define RXATM_PORT_NOT_ENABLED      0x80
#define RXATM_HEC_ERROR             0x40
#define RXATM_PTI_ERROR             0x20
#define RXATM_RECEIVED_IDLE_CELL    0x10
#define RXATM_INVALID_VPI_VCI       0x08
#define RXATM_NOT_USED              0x04
#define RXATM_OAM_RM_CRC_ERROR      0x02
#define RXATM_GFC_ERROR             0x01

// AAL5 and AAL0 Error Indicators.
#define RXAAL5AAL0_CRC_ERROR        0x80
#define RXAAL5AAL0_SHORT_PKT_ERROR  0x40
#define RXAAL5AAL0_LENGTH_ERROR     0x20
#define RXAAL5AAL0_BIG_PKT_ERROR    0x10

// ATM DMA Buffer Descriptor
typedef struct ATM_DMA_BD
{
    UINT32 ulCt_BufPtr;
    UINT32 ulFlags_NextRxBd;
    UINT8 ucUui8;
    UINT8 ucRxPortId_Vcid;
    UINT16 usLength;
    union
    {
        UINT8 ucTxPortId_Gfc;
        UINT8 ucRxAtmErrors;
    };
    UINT8 ucRxAalErrors_RblHigh;
    union
    {
        UINT8 ucRxRblLow;
        UINT8 ucFreeRbl;
    };
    UINT8 ucReserved;
} ATM_DMA_BD ;

//**************************************************************************
// Function Prototypes
//**************************************************************************

BCMATM_STATUS BcmAtm_Initialize( PATM_INITIALIZATION_PARMS pInitValues );
BCMATM_STATUS BcmAtm_Uninitialize( void );
BCMATM_STATUS BcmAtm_GetInterfaceId( UINT8 ucPhyPort, UINT32 *pulInterfaceId );
BCMATM_STATUS BcmAtm_GetPriorityPacketGroup(UINT32 ulGroupNumber,
    PATM_PRIORITY_PACKET_ENTRY pPriorityPackets, UINT32 *pulPriorityPacketsSize);
BCMATM_STATUS BcmAtm_SetPriorityPacketGroup(UINT32 ulGroupNumber,
    PATM_PRIORITY_PACKET_ENTRY pPriorityPackets, UINT32 ulPriorityPacketsSize);
BCMATM_STATUS BcmAtm_GetTrafficDescrTableSize(UINT32 *pulTrafficDescrTableSize);
BCMATM_STATUS BcmAtm_GetTrafficDescrTable( PATM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescTable, UINT32 ulTrafficDescrTableSize );
BCMATM_STATUS BcmAtm_SetTrafficDescrTable( PATM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescTable, UINT32  ulTrafficDescrTableSize );
BCMATM_STATUS BcmAtm_GetInterfaceCfg( UINT32 ulInterfaceId, PATM_INTERFACE_CFG
    pInterfaceCfg );
BCMATM_STATUS BcmAtm_SetInterfaceCfg( UINT32 ulInterfaceId, PATM_INTERFACE_CFG
    pInterfaceCfg );
BCMATM_STATUS BcmAtm_GetVccCfg( PATM_VCC_ADDR pVccAddr, PATM_VCC_CFG pVccCfg );
BCMATM_STATUS BcmAtm_SetVccCfg( PATM_VCC_ADDR pVccAddr, PATM_VCC_CFG pVccCfg );
BCMATM_STATUS BcmAtm_GetVccAddrs( UINT32 ulInterfaceId, PATM_VCC_ADDR pVccAddrs,
    UINT32 ulNumVccs, UINT32 *pulNumReturned );
BCMATM_STATUS BcmAtm_GetInterfaceStatistics( UINT32 ulInterfaceId,
    PATM_INTERFACE_STATS pStatistics, UINT32 ulReset );
BCMATM_STATUS BcmAtm_GetVccStatistics( PATM_VCC_ADDR pVccAddr, PATM_VCC_STATS
    pVccStatistics, UINT32 ulReset );
BCMATM_STATUS BcmAtm_SetInterfaceLinkInfo( UINT32 ulInterfaceId,
    PATM_INTERFACE_LINK_INFO pInterfaceCfg );
BCMATM_STATUS BcmAtm_SetNotifyCallback( FN_NOTIFY_CB pFnNotifyCb );
BCMATM_STATUS BcmAtm_ResetNotifyCallback( FN_NOTIFY_CB pFnNotifyCb );
BCMATM_STATUS BcmAtm_AttachVcc( PATM_VCC_ADDR pVccAddr, PATM_VCC_ATTACH_PARMS
    pAttachParms );
BCMATM_STATUS BcmAtm_AttachMgmtCells( UINT32 ulInterfaceId,
    PATM_VCC_ATTACH_PARMS pAttachParms );
BCMATM_STATUS BcmAtm_AttachTransparent( UINT32 ulInterfaceId,
    PATM_VCC_ATTACH_PARMS pAttachParms );
BCMATM_STATUS BcmAtm_Detach( UINT32 ulHandle );
BCMATM_STATUS BcmAtm_SetAal2ChannelIds( UINT32 ulHandle,
    PATM_VCC_AAL2_CHANNEL_ID_PARMS pChannelIdParms, UINT32
    ulNumChannelIdParms ); // [BCM635x Only]
BCMATM_STATUS BcmAtm_SendVccData( UINT32 ulHandle,
    PATM_VCC_DATA_PARMS pDataParms );
BCMATM_STATUS BcmAtm_SendMgmtData( UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDataParms );
BCMATM_STATUS BcmAtm_SendTransparentData( UINT32 ulHandle, UINT32 ulInterfaceId,
    PATM_VCC_DATA_PARMS pDataParms );
#if defined(__cplusplus)
}
#endif

#define S32MAX            0x7FFFFFFF
#define GET_U32_DIFF(a, b)  (((a) - (b)) & S32MAX)
 
#endif // _BCMATMAPI_H_

