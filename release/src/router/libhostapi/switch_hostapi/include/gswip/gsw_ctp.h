/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _GSW_CTP_H_
#define _GSW_CTP_H_

#include "gsw_types.h"

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/** \brief CTP Port configuration mask.
    Used by \ref GSW_CTP_portConfig_t. */
typedef enum {
	/** Mask for \ref GSW_CTP_portConfig_t::nBridgePortId */
	GSW_CTP_PORT_CONFIG_MASK_BRIDGE_PORT_ID = 0x00000001,
	/** Mask for \ref GSW_CTP_portConfig_t::bForcedTrafficClass and
	    \ref GSW_CTP_portConfig_t::nDefaultTrafficClass */
	GSW_CTP_PORT_CONFIG_MASK_FORCE_TRAFFIC_CLASS = 0x00000002,
	/** Mask for \ref GSW_CTP_portConfig_t::bIngressExtendedVlanEnable and
	    \ref GSW_CTP_portConfig_t::nIngressExtendedVlanBlockId */
	GSW_CTP_PORT_CONFIG_MASK_INGRESS_VLAN = 0x00000004,
	/** Mask for \ref GSW_CTP_portConfig_t::bIngressExtendedVlanIgmpEnable and
	    \ref GSW_CTP_portConfig_t::nIngressExtendedVlanBlockIdIgmp */
	GSW_CTP_PORT_CONFIG_MASK_INGRESS_VLAN_IGMP = 0x00000008,
	/** Mask for \ref GSW_CTP_portConfig_t::bEgressExtendedVlanEnable and
	    \ref GSW_CTP_portConfig_t::nEgressExtendedVlanBlockId */
	GSW_CTP_PORT_CONFIG_MASK_EGRESS_VLAN = 0x00000010,
	/** Mask for \ref GSW_CTP_portConfig_t::bEgressExtendedVlanIgmpEnable and
	    \ref GSW_CTP_portConfig_t::nEgressExtendedVlanBlockIdIgmp */
	GSW_CTP_PORT_CONFIG_MASK_EGRESS_VLAN_IGMP = 0x00000020,
	/** Mask for \ref GSW_CTP_portConfig_t::bIngressNto1VlanEnable */
	GSW_CTP_PORT_CONFIG_MASK_INRESS_NTO1_VLAN = 0x00000040,
	/** Mask for \ref GSW_CTP_portConfig_t::bEgressNto1VlanEnable */
	GSW_CTP_PORT_CONFIG_MASK_EGRESS_NTO1_VLAN = 0x00000080,
	/** Mask for \ref GSW_CTP_portConfig_t::eIngressMarkingMode */
	GSW_CTP_PORT_CONFIG_INGRESS_MARKING = 0x00000100,
	/** Mask for \ref GSW_CTP_portConfig_t::eEgressMarkingMode */
	GSW_CTP_PORT_CONFIG_EGRESS_MARKING = 0x00000200,
	/** Mask for \ref GSW_CTP_portConfig_t::bEgressMarkingOverrideEnable and
	    \ref GSW_CTP_portConfig_t::eEgressMarkingModeOverride */
	GSW_CTP_PORT_CONFIG_EGRESS_MARKING_OVERRIDE = 0x00000400,
	/** Mask for \ref GSW_CTP_portConfig_t::eEgressRemarkingMode */
	GSW_CTP_PORT_CONFIG_EGRESS_REMARKING = 0x00000800,
	/** Mask for \ref GSW_CTP_portConfig_t::bIngressMeteringEnable and
	    \ref GSW_CTP_portConfig_t::nIngressTrafficMeterId */
	GSW_CTP_PORT_CONFIG_INGRESS_METER = 0x00001000,
	/** Mask for \ref GSW_CTP_portConfig_t::bEgressMeteringEnable and
	    \ref GSW_CTP_portConfig_t::nEgressTrafficMeterId */
	GSW_CTP_PORT_CONFIG_EGRESS_METER = 0x00002000,
	/** Mask for \ref GSW_CTP_portConfig_t::bBridgingBypass,
	    \ref GSW_CTP_portConfig_t::nDestLogicalPortId,
	    \ref GSW_CTP_portConfig_t::bPmapperEnable,
	    \ref GSW_CTP_portConfig_t::nDestSubIfIdGroup,
	    \ref GSW_CTP_portConfig_t::ePmapperMappingMode
	    \ref GSW_CTP_portConfig_t::bPmapperIdValid and
	    \ref GSW_CTP_portConfig_t::sPmapper */
	GSW_CTP_PORT_CONFIG_BRIDGING_BYPASS = 0x00004000,
	/** Mask for \ref GSW_CTP_portConfig_t::nFirstFlowEntryIndex and
	    \ref GSW_CTP_portConfig_t::nNumberOfFlowEntries */
	GSW_CTP_PORT_CONFIG_FLOW_ENTRY = 0x00008000,
	/** Mask for \ref GSW_CTP_portConfig_t::bIngressLoopbackEnable,
	    \ref GSW_CTP_portConfig_t::bIngressDaSaSwapEnable,
	    \ref GSW_CTP_portConfig_t::bEgressLoopbackEnable,
	    \ref GSW_CTP_portConfig_t::bEgressDaSaSwapEnable,
	    \ref GSW_CTP_portConfig_t::bIngressMirrorEnable and
	    \ref GSW_CTP_portConfig_t::bEgressMirrorEnable */
	GSW_CTP_PORT_CONFIG_LOOPBACK_AND_MIRROR = 0x00010000,

	/** Enable all fields */
	GSW_CTP_PORT_CONFIG_MASK_ALL = 0x7FFFFFFF,
	/** Bypass any check for debug purpose */
	GSW_CTP_PORT_CONFIG_MASK_FORCE = 0x80000000
} GSW_CtpPortConfigMask_t;

/** \brief CTP Port Configuration.
    Used by \ref GSW_CtpPortConfigSet and \ref GSW_CtpPortConfigGet. */
typedef struct {
	/** Logical Port Id. The valid range is hardware dependent.
	    If \ref GSW_CTP_portConfig_t::eMask has
	    \ref GSW_CtpPortConfigMask_t::GSW_CTP_PORT_CONFIG_MASK_FORCE, this field
	    is ignored. */
	u8 nLogicalPortId;

	/** Sub interface ID group. The valid range is hardware/protocol dependent.

	    \remarks
	    Sub interface ID group is defined for each of \ref GSW_LogicalPortMode_t.
	    For both \ref GSW_LOGICAL_PORT_8BIT_WLAN and
	    \ref GSW_LOGICAL_PORT_9BIT_WLAN, this field is VAP.
	    For \ref GSW_LOGICAL_PORT_GPON, this field is GEM index.
	    For \ref GSW_LOGICAL_PORT_EPON, this field is stream index.
	    For \ref GSW_LOGICAL_PORT_GINT, this field is LLID.
	    For others, this field is 0.
	    If \ref GSW_CTP_portConfig_t::eMask has
	    \ref GSW_CtpPortConfigMask_t::GSW_CTP_PORT_CONFIG_MASK_FORCE, this field
	    is absolute index of CTP in hardware for debug purpose, bypassing
	    any check. */
	u16 nSubIfIdGroup;

	/** Mask for updating/retrieving fields. */
	GSW_CtpPortConfigMask_t eMask;

	/** Ingress Bridge Port ID to which this CTP port is associated for ingress
	    traffic. */
	u16 nBridgePortId;

	/** Default traffic class can not be overridden by other rules (except
	    traffic flow table and special tag) in processing stages. */
	gsw_bool_t bForcedTrafficClass;
	/** Default traffic class associated with all ingress traffic from this CTP
	    Port. */
	u8 nDefaultTrafficClass;

	/** Enable Extended VLAN processing for ingress non-IGMP traffic. */
	gsw_bool_t bIngressExtendedVlanEnable;
	/** Extended VLAN block allocated for ingress non-IGMP traffic. It defines
	    extended VLAN process for ingress non-IGMP traffic. Valid when
	    bIngressExtendedVlanEnable is TRUE. */
	u16 nIngressExtendedVlanBlockId;
	/** Extended VLAN block size for ingress non-IGMP traffic. This is optional.
	    If it is 0, the block size of nIngressExtendedVlanBlockId will be used.
	    Otherwise, this field will be used. */
	u16 nIngressExtendedVlanBlockSize;
	/** Enable extended VLAN processing for ingress IGMP traffic. */
	gsw_bool_t bIngressExtendedVlanIgmpEnable;
	/** Extended VLAN block allocated for ingress IGMP traffic. It defines
	    extended VLAN process for ingress IGMP traffic. Valid when
	    bIngressExtendedVlanIgmpEnable is TRUE. */
	u16 nIngressExtendedVlanBlockIdIgmp;
	/** Extended VLAN block size for ingress IGMP traffic. This is optional.
	    If it is 0, the block size of nIngressExtendedVlanBlockIdIgmp will be
	    used. Otherwise, this field will be used. */
	u16 nIngressExtendedVlanBlockSizeIgmp;

	/** Enable extended VLAN processing for egress non-IGMP traffic. */
	gsw_bool_t bEgressExtendedVlanEnable;
	/** Extended VLAN block allocated for egress non-IGMP traffic. It defines
	    extended VLAN process for egress non-IGMP traffic. Valid when
	    bEgressExtendedVlanEnable is TRUE. */
	u16 nEgressExtendedVlanBlockId;
	/** Extended VLAN block size for egress non-IGMP traffic. This is optional.
	    If it is 0, the block size of nEgressExtendedVlanBlockId will be used.
	    Otherwise, this field will be used. */
	u16 nEgressExtendedVlanBlockSize;
	/** Enable extended VLAN processing for egress IGMP traffic. */
	gsw_bool_t bEgressExtendedVlanIgmpEnable;
	/** Extended VLAN block allocated for egress IGMP traffic. It defines
	    extended VLAN process for egress IGMP traffic. Valid when
	    bEgressExtendedVlanIgmpEnable is TRUE. */
	u16 nEgressExtendedVlanBlockIdIgmp;
	/** Extended VLAN block size for egress IGMP traffic. This is optional.
	    If it is 0, the block size of nEgressExtendedVlanBlockIdIgmp will be
	    used. Otherwise, this field will be used. */
	u16 nEgressExtendedVlanBlockSizeIgmp;

	/** For WLAN type logical port, this should be FALSE. For other types, if
	     enabled and ingress packet is VLAN tagged, outer VLAN ID is used for
	    "nSubIfId" field in MAC table, otherwise, 0 is used for "nSubIfId". */
	gsw_bool_t bIngressNto1VlanEnable;
	/** For WLAN type logical port, this should be FALSE. For other types, if
	     enabled and egress packet is known unicast, outer VLAN ID is from
	     "nSubIfId" field in MAC table. */
	gsw_bool_t bEgressNto1VlanEnable;

	/** Ingress color marking mode for ingress traffic. */
	GSW_ColorMarkingMode_t eIngressMarkingMode;
	/** Egress color marking mode for ingress traffic at egress priority queue
	    color marking stage */
	GSW_ColorMarkingMode_t eEgressMarkingMode;
	/** Egress color marking mode override color marking mode from last stage. */
	gsw_bool_t bEgressMarkingOverrideEnable;
	/** Egress color marking mode for egress traffic. Valid only when
	    bEgressMarkingOverride is TRUE. */
	GSW_ColorMarkingMode_t eEgressMarkingModeOverride;

	/** Color remarking for egress traffic. */
	GSW_ColorRemarkingMode_t eEgressRemarkingMode;

	/** Traffic metering on ingress traffic applies. */
	gsw_bool_t bIngressMeteringEnable;
	/** Meter for ingress CTP process.

	    \remarks
	    Meter should be allocated with \ref GSW_QOS_MeterAlloc before CTP
	    port configuration. If this CTP port is re-set, the last used meter
	    should be released. */
	u16 nIngressTrafficMeterId;
	/** Traffic metering on egress traffic applies. */
	gsw_bool_t bEgressMeteringEnable;
	/** Meter for egress CTP process.

	    \remarks
	    Meter should be allocated with \ref GSW_QOS_MeterAlloc before CTP
	    port configuration. If this CTP port is re-set, the last used meter
	    should be released. */
	u16 nEgressTrafficMeterId;

	/** Ingress traffic bypass bridging/multicast processing. Following
	    parameters are used to determine destination. Traffic flow table is not
	    bypassed. */
	gsw_bool_t bBridgingBypass;
	/** When bBridgingBypass is TRUE, this field defines destination logical
	    port. */
	u8 nDestLogicalPortId;
	/** When bBridgingBypass is TRUE, this field indicates whether to use
	    \ref GSW_CTP_portConfig_t::nDestSubIfIdGroup or
	    \ref GSW_CTP_portConfig_t::ePmapperMappingMode/
	    \ref GSW_CTP_portConfig_t::sPmapper. */
	gsw_bool_t bPmapperEnable;
	/** When bBridgingBypass is TRUE and bPmapperEnable is FALSE, this field
	    defines destination sub interface ID group. */
	u16 nDestSubIfIdGroup;
	/** When bBridgingBypass is TRUE and bPmapperEnable is TRUE, this field
	    selects either DSCP or PCP to derive sub interface ID. */
	GSW_PmapperMappingMode_t ePmapperMappingMode;
	/** When bPmapperEnable is TRUE, P-mapper is used. This field determines
	    whether sPmapper.nPmapperId is valid. If this field is TRUE, the
	    P-mapper is re-used and no allocation of new P-mapper or value
	    change in the P-mapper. If this field is FALSE, allocation is
	    taken care in the API implementation. */
	gsw_bool_t bPmapperIdValid;
	/** When bBridgingBypass is TRUE and bPmapperEnable is TRUE, P-mapper is
	    used. If bPmapperIdValid is FALSE, API implementation need take care
	    of P-mapper allocation, and maintain the reference counter of
	    P-mapper used multiple times. If bPmapperIdValid is TRUE, only
	    sPmapper.nPmapperId is used to associate the P-mapper, and there is
	    no allocation of new P-mapper or value change in the P-mapper. */
	GSW_PMAPPER_t sPmapper;

	/** First traffic flow table entry is associated to this CTP port. Ingress
	    traffic from this CTP port will go through traffic flow table search
	    starting from nFirstFlowEntryIndex. Should be times of 4. */
	u16 nFirstFlowEntryIndex;
	/** Number of traffic flow table entries are associated to this CTP port.
	    Ingress traffic from this CTP port will go through PCE rules search
	    ending at (nFirstFlowEntryIndex+nNumberOfFlowEntries)-1. Should
	    be times of 4. */
	u16 nNumberOfFlowEntries;

	/** Ingress traffic from this CTP port will be redirected to ingress
	    logical port of this CTP port with source sub interface ID used as
	    destination sub interface ID. Following processing except traffic
	    flow table search is bypassed if loopback enabled. */
	gsw_bool_t bIngressLoopbackEnable;
	/** Destination/Source MAC address of ingress traffic is swapped before
	    transmitted (not swapped during PCE processing stages). If destination
	    is multicast, there is no swap, but source MAC address is replaced
	    with global configurable value. */
	gsw_bool_t bIngressDaSaSwapEnable;
	/** Egress traffic to this CTP port will be redirected to ingress logical
	    port with same sub interface ID as ingress. */
	gsw_bool_t bEgressLoopbackEnable;
	/** Destination/Source MAC address of egress traffic is swapped before
	    transmitted. */
	gsw_bool_t bEgressDaSaSwapEnable;

	/** If enabled, ingress traffic is mirrored to the monitoring port.

	    \remarks
	    This should be used exclusive with bIngressLoopbackEnable. */
	gsw_bool_t bIngressMirrorEnable;
	/** If enabled, egress traffic is mirrored to the monitoring port.

	    \remarks
	    This should be used exclusive with bEgressLoopbackEnable. */
	gsw_bool_t bEgressMirrorEnable;
} GSW_CTP_portConfig_t;

/** \brief Logical port mode.
    Used by \ref GSW_CTP_portAssignment_t. */
typedef enum {
	/** WLAN with 8-bit station ID */
	GSW_LOGICAL_PORT_8BIT_WLAN = 0,
	/** WLAN with 9-bit station ID */
	GSW_LOGICAL_PORT_9BIT_WLAN = 1,
	/** GPON OMCI context */
	GSW_LOGICAL_PORT_GPON = 2,
	/** EPON context */
	GSW_LOGICAL_PORT_EPON = 3,
	/** G.INT context */
	GSW_LOGICAL_PORT_GINT = 4,
	/** Others (sub interface ID is 0 by default) */
	GSW_LOGICAL_PORT_OTHER = 0xFF,
} GSW_LogicalPortMode_t;

/** \brief CTP Port Assignment/association with logical port.
    Used by \ref GSW_CTP_PortAssignmentAlloc, \ref GSW_CTP_PortAssignmentSet
    and \ref GSW_CTP_PortAssignmentGet. */
typedef struct {
	/** Logical Port Id. The valid range is hardware dependent. */
	u8 nLogicalPortId;

	/** First CTP Port ID mapped to above logical port ID.

	    \remarks
	    For \ref GSW_CTP_PortAssignmentAlloc, this is output when CTP
	    allocation is successful. For other APIs, this is input. */
	u16 nFirstCtpPortId;
	/** Total number of CTP Ports mapped above logical port ID. */
	u16 nNumberOfCtpPort;

	/** Logical port mode to define sub interface ID format. */
	GSW_LogicalPortMode_t eMode;

	/** Bridge ID (FID)

	    \remarks
	    For \ref GSW_CTP_PortAssignmentAlloc, this is input. Each CTP allocated
	    is mapped to Bridge Port given by this field. The Bridge Port will be
	    configured to use first CTP
	    (\ref GSW_CTP_portAssignment_t::nFirstCtpPortId) as egress CTP.
	    For other APIs, this is ignored. */
	u16 nBridgePortId;
} GSW_CTP_portAssignment_t;

/** @}*/ /* GSW_ETHERNET_BRIDGING */

#pragma scalar_storage_order default
#pragma pack(pop)

#endif /*_GSW_CTP_H */
