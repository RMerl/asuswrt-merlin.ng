/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _GSW_RMON_H_
#define _GSW_RMON_H_

#include "gsw_types.h"

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

/** \ingroup GSW_ETHERNET_BRIDGING
 *  \brief Port Type.
 *         Used by \ref GSW_portCfg_t. */
typedef enum {
	/** Logical Port */
	GSW_LOGICAL_PORT = 0,
	/** Physical Port
	    Applicable only for GSWIP-3.1/3.2 */
	GSW_PHYSICAL_PORT = 1,
	/** Connectivity Termination Port (CTP)
	    Applicable only for GSWIP-3.1/3.2 */
	GSW_CTP_PORT = 2,
	/** Bridge Port
	    Applicable only for GSWIP-3.1/3.2 */
	GSW_BRIDGE_PORT = 3
} GSW_portType_t;

/** \addtogroup GSW_RMON
 *  @{
 */

/** \brief RMON Counters Type enumeration.
    Used by \ref GSW_RMON_clear_t and \ref GSW_RMON_mode_t. */
typedef enum {
	/** All RMON Types Counters */
	GSW_RMON_ALL_TYPE = 0,
	/** All PMAC RMON Counters */
	GSW_RMON_PMAC_TYPE = 1,
	/** Port based RMON Counters */
	GSW_RMON_PORT_TYPE = 2,
	/** Meter based RMON Counters */
	GSW_RMON_METER_TYPE = 3,
	/** Interface based RMON Counters */
	GSW_RMON_IF_TYPE = 4,
	/** Route based RMON Counters */
	GSW_RMON_ROUTE_TYPE = 5,
	/** Redirected Traffic based RMON Counters */
	GSW_RMON_REDIRECT_TYPE	= 6,
	/** Bridge Port based RMON Counters */
	GSW_RMON_BRIDGE_TYPE	= 7,
	/** CTP Port based RMON Counters */
	GSW_RMON_CTP_TYPE	= 8,
} GSW_RMON_type_t;

/** \brief RMON Counters Data Structure for clearance of values.
    Used by \ref GSW_RMON_Clear. */
typedef struct {
	/** RMON Counters Type */
	GSW_RMON_type_t eRmonType;
	/** RMON Counters Identifier - Meter, Port, If, Route, etc. */
	u8 nRmonId;
} GSW_RMON_clear_t;

/**Defined as per RMON counter table structure
  Applicable only for GSWIP 3.1*/
typedef enum {
	GSW_RMON_CTP_PORT_RX = 0,
	GSW_RMON_CTP_PORT_TX = 1,
	GSW_RMON_BRIDGE_PORT_RX = 2,
	GSW_RMON_BRIDGE_PORT_TX = 3,
	GSW_RMON_CTP_PORT_PCE_BYPASS = 4,
	GSW_RMON_TFLOW_RX = 5,
	GSW_RMON_TFLOW_TX = 6,
	GSW_RMON_QMAP = 0x0E,
	GSW_RMON_METER = 0x19,
	GSW_RMON_PMAC = 0x1C,
} GSW_RMON_portType_t;

/** TFLOW counter mode type */
typedef enum {
	/** Global mode */
	GSW_TFLOW_CMODE_GLOBAL = 0,
	/** Logical port mode */
	GSW_TFLOW_CMODE_LOGICAL = 1,
	/** CTP port mode */
	GSW_TFLOW_CMODE_CTP = 2,
	/** Bridge port mode */
	GSW_TFLOW_CMODE_BRIDGE = 3,
} GSW_TflowCmodeType_t;

/** TFLOW counter type */
typedef enum {
	/** Set all Rx/Tx/PCE-Bp-Tx registers to same value */
	GSW_TFLOW_COUNTER_ALL = 0, //Default for 'set' function.
	/** SEt PCE Rx register config only */
	GSW_TFLOW_COUNTER_PCE_Rx = 1, //Default for 'get' function.
	/** SEt PCE Tx register config only */
	GSW_TFLOW_COUNTER_PCE_Tx = 2,
	/** SEt PCE-Bypass Tx register config only */
	GSW_TFLOW_COUNTER_PCE_BP_Tx = 3,
} GSW_TflowCountConfType_t;

/** TFLOW CTP counter LSB bits */
typedef enum {
	/* Num of valid bits  */
	/** 0 valid bits  */
	GSW_TCM_CTP_VAL_BITS_0 = 0,
	/** 1 valid bits  */
	GSW_TCM_CTP_VAL_BITS_1 = 1,
	/** 2 valid bits  */
	GSW_TCM_CTP_VAL_BITS_2 = 2,
	/** 3 valid bits  */
	GSW_TCM_CTP_VAL_BITS_3 = 3,
	/** 4 valid bits  */
	GSW_TCM_CTP_VAL_BITS_4 = 4,
	/** 5 valid bits  */
	GSW_TCM_CTP_VAL_BITS_5 = 5,
	/** 6 valid bits  */
	GSW_TCM_CTP_VAL_BITS_6 = 6,
} GSW_TflowCtpValBits_t;

/** TFLOW bridge port counter LSB bits */
typedef enum {
	/* Num of valid bits  */
	/** 2 valid bits  */
	GSW_TCM_BRP_VAL_BITS_2 = 2,
	/** 3 valid bits  */
	GSW_TCM_BRP_VAL_BITS_3 = 3,
	/** 4 valid bits  */
	GSW_TCM_BRP_VAL_BITS_4 = 4,
	/** 5 valid bits  */
	GSW_TCM_BRP_VAL_BITS_5 = 5,
	/** 6 valid bits  */
	GSW_TCM_BRP_VAL_BITS_6 = 6,
} GSW_TflowBrpValBits_t;

/**
 \brief RMON Counters for individual Port.
 This structure contains the RMON counters of an Ethernet Switch Port.
    Used by \ref GSW_RMON_Port_Get. */
typedef struct {
	/** Port Type. This gives information which type of port to get RMON.
	    nPortId should be based on this field.
	    This is new in GSWIP-3.1. For GSWIP-2.1/2.2/3.0, this field is always
	    ZERO (GSW_LOGICAL_PORT). */
	GSW_portType_t ePortType;
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. This parameter specifies for which MAC port the RMON
	    counter is read. It has to be set by the application before
	    calling \ref GSW_RMON_Port_Get. */
	u16	nPortId;
	/** Sub interface ID group. The valid range is hardware/protocol dependent.

	    \remarks
	    This field is valid when \ref GSW_RMON_Port_cnt_t::ePortType is
	    \ref GSW_portType_t::GSW_CTP_PORT.
	    Sub interface ID group is defined for each of \ref GSW_LogicalPortMode_t.
	    For both \ref GSW_LOGICAL_PORT_8BIT_WLAN and
	    \ref GSW_LOGICAL_PORT_9BIT_WLAN, this field is VAP.
	    For \ref GSW_LOGICAL_PORT_GPON, this field is GEM index.
	    For \ref GSW_LOGICAL_PORT_EPON, this field is stream index.
	    For \ref GSW_LOGICAL_PORT_GINT, this field is LLID.
	    For others, this field is 0. */
	u16 nSubIfIdGroup;
	/** Separate set of CTP Tx counters when PCE is bypassed. GSWIP-3.1 only.*/
	gsw_bool_t bPceBypass;
	/*Applicable only for GSWIP 3.1*/
	/** Discarded at Extended VLAN Operation Packet Count. GSWIP-3.1 only. */
	u32	nRxExtendedVlanDiscardPkts;
	/** Discarded MTU Exceeded Packet Count. GSWIP-3.1 only. */
	u32	nMtuExceedDiscardPkts;
	/** Tx Undersize (<64) Packet Count. GSWIP-3.1 only. */
	u32 nTxUnderSizeGoodPkts;
	/** Tx Oversize (>1518) Packet Count. GSWIP-3.1 only. */
	u32 nTxOversizeGoodPkts;
	/** Receive Packet Count (only packets that are accepted and not discarded). */
	u32	nRxGoodPkts;
	/** Receive Unicast Packet Count. */
	u32	nRxUnicastPkts;
	/** Receive Broadcast Packet Count. */
	u32	nRxBroadcastPkts;
	/** Receive Multicast Packet Count. */
	u32	nRxMulticastPkts;
	/** Receive FCS Error Packet Count. */
	u32	nRxFCSErrorPkts;
	/** Receive Undersize Good Packet Count. */
	u32	nRxUnderSizeGoodPkts;
	/** Receive Oversize Good Packet Count. */
	u32	nRxOversizeGoodPkts;
	/** Receive Undersize Error Packet Count. */
	u32	nRxUnderSizeErrorPkts;
	/** Receive Good Pause Packet Count. */
	u32	nRxGoodPausePkts;
	/** Receive Oversize Error Packet Count. */
	u32	nRxOversizeErrorPkts;
	/** Receive Align Error Packet Count. */
	u32	nRxAlignErrorPkts;
	/** Filtered Packet Count. */
	u32	nRxFilteredPkts;
	/** Receive Size 64 Bytes Packet Count. */
	u32	nRx64BytePkts;
	/** Receive Size 65-127 Bytes Packet Count. */
	u32	nRx127BytePkts;
	/** Receive Size 128-255 Bytes Packet Count. */
	u32	nRx255BytePkts;
	/** Receive Size 256-511 Bytes Packet Count. */
	u32	nRx511BytePkts;
	/** Receive Size 512-1023 Bytes Packet Count. */
	u32	nRx1023BytePkts;
	/** Receive Size 1024-1522 Bytes (or more, if configured) Packet Count. */
	u32	nRxMaxBytePkts;
	/** Overall Transmit Good Packets Count. */
	u32	nTxGoodPkts;
	/** Transmit Unicast Packet Count. */
	u32	nTxUnicastPkts;
	/** Transmit Broadcast Packet Count. */
	u32	nTxBroadcastPkts;
	/** Transmit Multicast Packet Count. */
	u32	nTxMulticastPkts;
	/** Transmit Single Collision Count. */
	u32	nTxSingleCollCount;
	/** Transmit Multiple Collision Count. */
	u32	nTxMultCollCount;
	/** Transmit Late Collision Count. */
	u32	nTxLateCollCount;
	/** Transmit Excessive Collision Count. */
	u32	nTxExcessCollCount;
	/** Transmit Collision Count. */
	u32	nTxCollCount;
	/** Transmit Pause Packet Count. */
	u32	nTxPauseCount;
	/** Transmit Size 64 Bytes Packet Count. */
	u32	nTx64BytePkts;
	/** Transmit Size 65-127 Bytes Packet Count. */
	u32	nTx127BytePkts;
	/** Transmit Size 128-255 Bytes Packet Count. */
	u32	nTx255BytePkts;
	/** Transmit Size 256-511 Bytes Packet Count. */
	u32	nTx511BytePkts;
	/** Transmit Size 512-1023 Bytes Packet Count. */
	u32	nTx1023BytePkts;
	/** Transmit Size 1024-1522 Bytes (or more, if configured) Packet Count. */
	u32	nTxMaxBytePkts;
	/** Transmit Drop Packet Count. */
	u32	nTxDroppedPkts;
	/** Transmit Dropped Packet Count, based on Congestion Management. */
	u32	nTxAcmDroppedPkts;
	/** Receive Dropped Packet Count. */
	u32	nRxDroppedPkts;
	/** Receive Good Byte Count (64 bit). */
	u64	nRxGoodBytes;
	/** Receive Bad Byte Count (64 bit). */
	u64	nRxBadBytes;
	/** Transmit Good Byte Count (64 bit). */
	u64	nTxGoodBytes;
} GSW_RMON_Port_cnt_t;

/** \brief RMON Counters Mode Enumeration.
    This enumeration defines Counters mode - Packets based or Bytes based counting.
    Metering and Routing Sessions RMON counting support either Byte based or packets based only. */
typedef enum {
	/** Packet based RMON Counters */
	GSW_RMON_COUNT_PKTS	= 0,
	/** Bytes based RMON Counters */
	GSW_RMON_COUNT_BYTES	= 1,
	/**  number of dropped frames, supported only for interface cunters */
	GSW_RMON_DROP_COUNT	= 2,
} GSW_RMON_CountMode_t;

/** \brief RMON Counters Mode for different Elements.
    This structure takes RMON Counter Element Name and mode config */
typedef struct {
	/** RMON Counters Type */
	GSW_RMON_type_t eRmonType;
	/** RMON Counters Mode */
	GSW_RMON_CountMode_t eCountMode;
} GSW_RMON_mode_t;

/**
 \brief RMON Counters for Meter - Type (GSWIP-3.0 only).
 This structure contains the RMON counters of one Meter Instance.
    Used by \ref GSW_RMON_Meter_Get. */
typedef struct {
	/** Meter Instance number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected meter is not
	    available. This parameter specifies for which Meter Id the RMON-1
	    counter is read. It has to be set by the application before
	    calling \ref GSW_RMON_Meter_Get. */
	u8	nMeterId;
	/** Metered Green colored packets or bytes (depending upon mode) count. */
	u32	nGreenCount;
	/** Metered Yellow colored packets or bytes (depending upon mode) count. */
	u32	nYellowCount;
	/** Metered Red colored packets or bytes (depending upon mode) count. */
	u32	nRedCount;
	/** Metered Reserved (Future Use) packets or bytes (depending upon mode) count. */
	u32	nResCount;
} GSW_RMON_Meter_cnt_t;

/** \brief For debugging Purpose only.
    Used for GSWIP 3.1. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. This parameter specifies for which MAC port the RMON
	    counter is read. It has to be set by the application before
	    calling \ref GSW_RMON_Port_Get. */
	u16 nPortId;
	/**Table address selection based on port type
	 \ref GSW_RMON_portType_t**/
	GSW_RMON_portType_t ePortType;
	/** Extended VLAN Discard Packet Count.*/
	u32 nRxExtendedVlanDiscardPkts;
	/** Oversize Discard Packet Count. */
	u32 nMtuExceedDiscardPkts;
	/** Underisze Good Packet Count. */
	u32 nTxUnderSizeGoodPkts;
	/** Oversize Good Packet Count */
	u32 nTxOversizeGoodPkts;
	/** Receive Packet Count (only packets that are accepted and not discarded). */
	u32 nRxGoodPkts;
	/** Receive Unicast Packet Count. */
	u32 nRxUnicastPkts;
	/** Receive Broadcast Packet Count. */
	u32 nRxBroadcastPkts;
	/** Receive Multicast Packet Count. */
	u32 nRxMulticastPkts;
	/** Receive FCS Error Packet Count. */
	u32 nRxFCSErrorPkts;
	/** Receive Undersize Good Packet Count. */
	u32 nRxUnderSizeGoodPkts;
	/** Receive Oversize Good Packet Count. */
	u32 nRxOversizeGoodPkts;
	/** Receive Undersize Error Packet Count. */
	u32 nRxUnderSizeErrorPkts;
	/** Receive Good Pause Packet Count. */
	u32 nRxGoodPausePkts;
	/** Receive Oversize Error Packet Count. */
	u32 nRxOversizeErrorPkts;
	/** Receive Align Error Packet Count. */
	u32 nRxAlignErrorPkts;
	/** Filtered Packet Count (destination portmap is 0). */
	u32 nRxFilteredPkts;
	/** Receive Size 64 Bytes Packet Count. */
	u32 nRx64BytePkts;
	/** Receive Size 65-127 Bytes Packet Count. */
	u32 nRx127BytePkts;
	/** Receive Size 128-255 Bytes Packet Count. */
	u32 nRx255BytePkts;
	/** Receive Size 256-511 Bytes Packet Count. */
	u32 nRx511BytePkts;
	/** Receive Size 512-1023 Bytes Packet Count. */
	u32 nRx1023BytePkts;
	/** Receive Size 1024-1522 Bytes (or more, if configured) Packet Count. */
	u32 nRxMaxBytePkts;
	/** Overall Transmit Good Packets Count. */
	u32 nTxGoodPkts;
	/** Transmit Unicast Packet Count. */
	u32 nTxUnicastPkts;
	/** Transmit Broadcast Packet Count. */
	u32 nTxBroadcastPkts;
	/** Transmit Multicast Packet Count. */
	u32 nTxMulticastPkts;
	/** Transmit Single Collision Count. */
	u32 nTxSingleCollCount;
	/** Transmit Multiple Collision Count. */
	u32 nTxMultCollCount;
	/** Transmit Late Collision Count. */
	u32 nTxLateCollCount;
	/** Transmit Excessive Collision Count. */
	u32 nTxExcessCollCount;
	/** Transmit Collision Count. */
	u32 nTxCollCount;
	/** Transmit Pause Packet Count. */
	u32 nTxPauseCount;
	/** Transmit Size 64 Bytes Packet Count. */
	u32 nTx64BytePkts;
	/** Transmit Size 65-127 Bytes Packet Count. */
	u32 nTx127BytePkts;
	/** Transmit Size 128-255 Bytes Packet Count. */
	u32 nTx255BytePkts;
	/** Transmit Size 256-511 Bytes Packet Count. */
	u32 nTx511BytePkts;
	/** Transmit Size 512-1023 Bytes Packet Count. */
	u32 nTx1023BytePkts;
	/** Transmit Size 1024-1522 Bytes (or more, if configured) Packet Count. */
	u32 nTxMaxBytePkts;
	/** Transmit Drop Packet Count. */
	u32 nTxDroppedPkts;
	/** Transmit Dropped Packet Count, based on Congestion Management. */
	u32 nTxAcmDroppedPkts;
	/** Receive Dropped Packet Count. */
	u32 nRxDroppedPkts;
	/** Receive Good Byte Count (64 bit). */
	u64 nRxGoodBytes;
	/** Receive Bad Byte Count (64 bit). */
	u64 nRxBadBytes;
	/** Transmit Good Byte Count (64 bit). */
	u64 nTxGoodBytes;
	/** Receive Unicast Packet Count for Yellow & Red packet. */
	u32 nRxUnicastPktsYellowRed;
	/** Receive Broadcast Packet Count for Yellow & Red packet. */
	u32 nRxBroadcastPktsYellowRed;
	/** Receive Multicast Packet Count for Yellow & Red packet. */
	u32 nRxMulticastPktsYellowRed;
	/** Receive Good Byte Count (64 bit) for Yellow & Red packet. */
	u64 nRxGoodBytesYellowRed;
	/** Receive Packet Count for Yellow & Red packet.  */
	u32 nRxGoodPktsYellowRed;
	/** Transmit Unicast Packet Count for Yellow & Red packet. */
	u32 nTxUnicastPktsYellowRed;
	/** Transmit Broadcast Packet Count for Yellow & Red packet. */
	u32 nTxBroadcastPktsYellowRed;
	/** Transmit Multicast Packet Count for Yellow & Red packet. */
	u32 nTxMulticastPktsYellowRed;
	/** Transmit Good Byte Count (64 bit) for Yellow & Red packet. */
	u64 nTxGoodBytesYellowRed;
	/** Transmit Packet Count for Yellow & Red packet.  */
	u32 nTxGoodPktsYellowRed;
} GSW_Debug_RMON_Port_cnt_t;

/**
   \brief Hardware platform TFLOW counter mode.
   Supported modes include, Global (default), Logical, CTP, Bridge port mode.
   The number of counters that can be assigned varies based these mode type.
    Used by \ref GSW_TflowCountModeSet and GSW_TflowCountModeGet. */
typedef struct {
	/** Counter type. PCE Rx/Tx/Bp-Tx. */
	GSW_TflowCountConfType_t eCountType;
	/** Counter mode. Global/Logical/CTP/BrP. */
	GSW_TflowCmodeType_t eCountMode;
	/** The below params are valid only for CTP/BrP types.
	    A group of ports matching MS (9-n) bits. n is nCtpLsb or nBrpLsb. */
	u16 nPortMsb;
	/** Number of valid bits in CTP port counter mode. */
	GSW_TflowCtpValBits_t nCtpLsb;
	/** Number of valid bits in bridge port counter mode. */
	GSW_TflowBrpValBits_t nBrpLsb;
} GSW_TflowCmodeConf_t;

/**
   \brief Hardware platform extended RMON Counters. GSWIP-3.1 only.
   This structure contains additional RMON counters. These counters can be
   used by the packet classification engine and can be freely assigned to
   dedicated packet rules and flows.
    Used by \ref GSW_RMON_FlowGet. */
typedef struct {
	/** If TRUE, use \ref GSW_RMON_flowGet_t::nIndex to access the Flow Counter,
	    otherwise, use \ref GSW_TflowCountModeGet to determine mode and use
	    \ref GSW_RMON_flowGet_t::nPortId and \ref GSW_RMON_flowGet_t::nFlowId
	    to calculate index of the Flow Counter. */
	gsw_bool_t bIndex;
	/** Absolute index of Flow Counter. */
	u16 nIndex;
	/** Port ID. This could be Logical Port, CTP or Bridge Port. It depends
	    on the mode set by \ref GSW_TflowCountModeSet. */
	u16 nPortId;
	/** \ref GSW_PCE_action_t::nFlowID. The range depends on the mode set
	    by \ref GSW_TflowCountModeSet. */
	u16 nFlowId;

	/** Rx Packet Counter */
	u32 nRxPkts;
	/** Tx Packet Counter (non-PCE-Bypass) */
	u32 nTxPkts;
	/** Tx Packet Counter (PCE-Bypass) */
	u32 nTxPceBypassPkts;
} GSW_RMON_flowGet_t;

/** @}*/ /* GSW_RMON */

#pragma scalar_storage_order default
#pragma pack(pop)

#endif /* _GSW_RMON_H_ */
