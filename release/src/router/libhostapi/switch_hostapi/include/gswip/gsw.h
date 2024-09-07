/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _MXL_GSW_H_
#define _MXL_GSW_H_

#include "gsw_types.h"
#include "gsw_rmon.h"

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

/** \mainpage GSW APIs
    \section intro_sec Introduction

    This document describes the entire interface for accessing and configuring the different services of the Ethernet
	Switch module. The prefix GSW (Gigabit Switch) is used for all data structures and APIs pertaining to GSWIP
	Subsystem.

	Main focus of GSW APIs are as follows:

	*	- Ethernet Bridging Functions
	*	- VLAN Functions
	*	- Multicast Functions
	*	- Operation, Administration, and Management Functions
	*	- Quality of Service Functions
			- Traffic Class Classification Functions
			- Metering Functions
			- Shaping Functions
			- Weighted Fair Queuing
			- Color Marking/Remarking
	*	- Pseudo-MAC Functions
	*	- Debug Statistics Functions
	*	- Classification Engine (PCE) Functions
*/

/** @{*/
/** \defgroup GSW_API GSWIP Functional APIs
    \brief This chapter describes the entire interface for accessing and configuring the different services of the Ethernet Switch module.
*/
/**@}*/

/** \addtogroup GSW_API
 *  @{
 */

/** \defgroup GSW_ETHERNET_BRIDGING Ethernet bridging Functions
	\brief Group of functional APIs for Ethernet bridging (or switching) is the basic task of the device. It provides individual configurations per port and standard global switch features.
*/

/** \defgroup GSW_VLAN VLAN Functions
	\brief Group of functional APIs for VLAN bridging functionality. This includes support for Customer VLAN Tags (C-VLAN) and also Service VLAN Tags (S-VLAN).
*/

/** \defgroup GSW_MULTICAST Multicast Functions
	\brief Group of functional APIs for IGMP/MLD snooping configuration and support for IGMPv1/v2/v3 and MLDv1/v2.
*/

/** \defgroup GSW_OAM Operation, Administration, and Management Functions
	\brief Group of functions that are provided to perform OAM functions on Switch.
*/

/** \defgroup GSW_QoS_SVC Quality of Service Functions
	\brief Group of macros for Quality of Service (QoS) components.
*/

/** \defgroup GSW_RMON RMON Counters Functions
	\brief Group of macros for Remote-Monitoring (RMON) counters.
*/

/** \defgroup GSW_PMAC PMAC Operational Functions
	\brief Group of functions for PMAC Operations.
*/

/** \defgroup GSW_PCE PCE Rule Operational Functions
	\brief Group of functions for GSW PCE (Packet Classification Engine) Rule Operations.
*/

/** \defgroup GSW_8021X_GROUP 802.1x PORT Operational Functions
	\brief Group of functions for GSW STP 802.1x Port Operation.
*/

/** \defgroup GSW_DEBUG Debug Functions
	\brief Group of functions for GSW Debug Interfaces/Operations.
*/

/** @cond DOC_ENABLE_PBB */
/** \defgroup GSW_PBB Mac-in-Mac (PBB) Configuration Functions
	\brief Group of functions for GSW Mac-in-Mac (PBB) Operation.
*/
/** @endcond DOC_ENABLE_PBB */

/**@}*/ /* GSW_API */


/** \ingroup GSW_ETHERNET_BRIDGING
 *  \brief GSWIP specific error code base
 */
#define GSW_ERROR_BASE	1024

/** \ingroup GSW_ETHERNET_BRIDGING
 *  \brief Enumeration for function status return. The upper four bits are reserved for error classification
 */
typedef enum {
	/** Correct or Expected Status */
	GSW_statusOk	= 0,
	/** Generic or unknown error occurred */
	GSW_statusErr	= -1,
	/** Invalid function parameter */
	GSW_statusParam	= -(GSW_ERROR_BASE + 2),
	/** No space left in VLAN table */
	GSW_statusVLAN_Space	= -(GSW_ERROR_BASE + 3),
	/** Requested VLAN ID not found in table */
	GSW_statusVLAN_ID	= -(GSW_ERROR_BASE + 4),
	/** Invalid ioctl */
	GSW_statusInvalIoctl	= -(GSW_ERROR_BASE + 5),
	/** Operation not supported by hardware */
	GSW_statusNoSupport	= -(GSW_ERROR_BASE + 6),
	/** Timeout */
	GSW_statusTimeout	= -(GSW_ERROR_BASE + 7),
	/** At least one value is out of range */
	GSW_statusValueRange	= -(GSW_ERROR_BASE + 8),
	/** The PortId/QueueId/MeterId/etc. is not available in this hardware or the
	    selected feature is not available on this port */
	GSW_statusPortInvalid	= -(GSW_ERROR_BASE + 9),
	/** The interrupt is not available in this hardware */
	GSW_statusIRQ_Invalid	= -(GSW_ERROR_BASE + 10),
	/** The MAC table is full, an entry could not be added */
	GSW_statusMAC_TableFull	= -(GSW_ERROR_BASE + 11),
	/** Locking failed - SWAPI is busy */
	GSW_statusLock_Failed	=  -(GSW_ERROR_BASE + 12),
	/** Multicast Forwarding table entry not found */
	GSW_statusEntryNotFound = -(GSW_ERROR_BASE + 13),
} GSW_return_t;


/** \addtogroup GSW_8021X_GROUP
 *  @{
 */

/** \brief Describes the 802.1x port state. */
typedef enum {
	/** Receive and transmit direction are authorized. The port is allowed to
	    transmit and receive all packets and the address learning process is
	    also allowed. */
	GSW_8021X_PORT_STATE_AUTHORIZED = 0,
	/** Receive and transmit direction are unauthorized. All the packets
	    except EAPOL are not allowed to transmit and receive. The address learning
	    process is disabled. */
	GSW_8021X_PORT_STATE_UNAUTHORIZED = 1,
	/** Receive direction is authorized, transmit direction is unauthorized.
	    The port is allowed to receive all packets. Packet transmission to this
	    port is not allowed. The address learning process is also allowed. */
	GSW_8021X_PORT_STATE_RX_AUTHORIZED = 2,
	/** Transmit direction is authorized, receive direction is unauthorized.
	    The port is allowed to transmit all packets. Packet reception on this
	    port is not allowed. The address learning process is disabled. */
	GSW_8021X_PORT_STATE_TX_AUTHORIZED = 3
} GSW_8021X_portState_t;

/** @}*/ /* GSW_8021X_GROUP */


/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/** \brief Invalid Value.
    Used mainly during resource allocation to indicate that resource is not
    allocated. */
#define INVALID_HANDLE	(~0)

/** \brief MAC Table Entry to be read.
    Used by \ref GSW_MAC_TableEntryRead. */
typedef struct {
	/** Restart the get operation from the beginning of the table. Otherwise
	    return the next table entry (next to the entry that was returned
	    during the previous get operation). This boolean parameter is set by the
	    calling application. */
	gsw_bool_t bInitial;
	/** Indicates that the read operation got all last valid entries of the
	    table. This boolean parameter is set by the switch API
	    when the Switch API is called after the last valid one was returned already. */
	gsw_bool_t bLast;
	/** Get the MAC table entry belonging to the given Filtering Identifier
	    (not supported by all switches). */
	u16 nFId;
	/** Ethernet Port number (zero-based counting) in GSWIP-2.1/2.2/3.0. From
	    GSWIP-3.1, this field is Bridge Port ID. The valid range is hardware
	    dependent.

	    \remarks
	    In GSWIP-2.1/2.2/3.0, this field is used as portmap field, when the MSB
	    bit is set. In portmap mode, every value bit represents an Ethernet port.
	    LSB represents Port 0 with incrementing counting.
	    The (MSB - 1) bit represent the last port.
	    The macro \ref GSW_PORTMAP_FLAG_SET allows to set the MSB bit,
	    marking it as portmap variable.
	    Checking the portmap flag can be done by
	    using the \ref GSW_PORTMAP_FLAG_GET macro.
	    From GSWIP3.1, if MSB is set, other bits in this field are ignored.
	    array \ref GSW_MAC_tableRead_t::nPortMap is used for bit map. */
	u32 nPortId;
	/** Bridge Port Map - to support GSWIP-3.1, following field is added
	    for port map in static entry. It's valid only when MSB of
	    \ref GSW_MAC_tableRead_t::nPortId is set. Each bit stands for 1 bridge
	    port. */
	u16 nPortMap[8];	/* max can be 16 */
	/** Aging Time, given in multiples of 1 second in a range from 1 s to 1,000,000 s.
	    The value read back in a GET command might differ slightly from the value
	    given in the SET command due to limited hardware timing resolution.
	    Filled out by the switch API implementation. */
	int nAgeTimer;
	/** STAG VLAN Id. Only applicable in case SVLAN support is enabled on the device. */
	u16 nSVLAN_Id;
	/** Static Entry (value will be aged out after 'nAgeTimer' if the entry
	    is not set to static). */
	gsw_bool_t bStaticEntry;
	/** Sub-Interface Identifier Destination (supported in GSWIP-3.0/3.1 only). */
	u16 nSubIfId;
	/** MAC Address. Filled out by the switch API implementation. */
	u8 nMAC[GSW_MAC_ADDR_LEN];
	/** Source/Destination MAC address filtering flag (GSWIP-3.1 only)
	    Value 0 - not filter, 1 - source address filter,
	    2 - destination address filter, 3 - both source and destination filter.

	    \remarks
	    Please refer to "GSWIP Hardware Architecture Spec" chapter 3.4.4.6
	    "Source MAC Address Filtering and Destination MAC Address Filtering"
	    for more detail. */
	u8 nFilterFlag;
	/** Packet is marked as IGMP controlled if destination MAC address matches
	    MAC in this entry. (GSWIP-3.1 only) */
	gsw_bool_t bIgmpControlled;

	/** Changed
	0: the entry is not changed
	1: the entry is changed and not accessed yet */

	gsw_bool_t bEntryChanged;

	/** Associated Mac address -(GSWIP-3.2)*/
	u8 nAssociatedMAC[GSW_MAC_ADDR_LEN];
	/** MAC Table Hit Status Update */
	gsw_bool_t hitstatus;
	/** TCI for (GSWIP-3.2) B-Step
	    Bit [0:11] - VLAN ID
	    Bit [12] - VLAN CFI/DEI
	    Bit [13:15] - VLAN PRI */
	u16 nTci;
	/** From which port, this MAC address is first time learned.
	 *  This is used for loop detection. */
	u16 nFirstBridgePortId;
} GSW_MAC_tableRead_t;

/** \brief Search for a MAC address entry in the address table.
    Used by \ref GSW_MAC_TableEntryQuery. */
typedef struct  {
	/** MAC Address. This parameter needs to be provided for the search operation.
	    This is an input parameter. */
	u8 nMAC[GSW_MAC_ADDR_LEN];
	/** Get the MAC table entry belonging to the given Filtering Identifier
	    (not supported by all switches).
	    This is an input parameter. */
	u16 nFId;
	/** MAC Address Found. Switch API sets this boolean variable in case
	    the requested MAC address 'nMAC' is found inside the address table,
	    otherwise it is set to FALSE.
	    This is an output parameter. */
	gsw_bool_t bFound;
	/** Ethernet Port number (zero-based counting) in GSWIP-2.1/2.2/3.0. From
	    GSWIP-3.1, this field is Bridge port ID. The valid range is hardware
	    dependent.

	    \remarks
	    In GSWIP-2.1/2.2/3.0, this field is used as portmap field, when the MSB
	    bit is set. In portmap mode, every value bit represents an Ethernet port.
	    LSB represents Port 0 with incrementing counting.
	    The (MSB - 1) bit represent the last port.
	    The macro \ref GSW_PORTMAP_FLAG_SET allows to set the MSB bit,
	    marking it as portmap variable.
	    Checking the portmap flag can be done by
	    using the \ref GSW_PORTMAP_FLAG_GET macro.
	    From GSWIP3.1, if MSB is set, other bits in this field are ignored.
	    array \ref GSW_MAC_tableRead_t::nPortMap is used for bit map. */
	u32 nPortId;
	/** Bridge Port Map - to support GSWIP-3.1, following field is added
	    for port map in static entry. It's valid only when MSB of
	    \ref GSW_MAC_tableRead_t::nPortId is set. Each bit stands for 1 bridge
	    port. */
	u16 nPortMap[8];	/* max can be 16 */
	/** Sub-Interface Identifier Destination (supported in GSWIP-3.0/3.1 only). */
	u16 nSubIfId;
	/** Aging Time, given in multiples of 1 second in a range from 1 s to 1,000,000 s.
	    The value read back in a GET command might differ slightly from the value
	    given in the SET command due to limited hardware timing resolution.
	    Filled out by the switch API implementation.
	    This is an output parameter. */
	int nAgeTimer;
	/** STAG VLAN Id. Only applicable in case SVLAN support is enabled on the device. */
	u16 nSVLAN_Id;
	/** Static Entry (value will be aged out after 'nAgeTimer' if the entry
	    is not set to static).
	    This is an output parameter. */
	gsw_bool_t bStaticEntry;
	/** Source/Destination MAC address filtering flag (GSWIP-3.1 only)
	    Value 0 - not filter, 1 - source address filter,
	    2 - destination address filter, 3 - both source and destination filter.

	    \remarks
	    Please refer to "GSWIP Hardware Architecture Spec" chapter 3.4.4.6
	    "Source MAC Address Filtering and Destination MAC Address Filtering"
	    for more detail. */
	u8 nFilterFlag;
	/** Packet is marked as IGMP controlled if destination MAC address matches
	    MAC in this entry. (GSWIP-3.1 only) */
	gsw_bool_t bIgmpControlled;
	/** Changed
	0: the entry is not changed
	1: the entry is changed and not accessed yet */
	gsw_bool_t bEntryChanged;
	/** Associated Mac address -(GSWIP-3.2)*/
	u8 nAssociatedMAC[GSW_MAC_ADDR_LEN];

	/** MAC Table Hit Status Update */
	gsw_bool_t hitstatus;
	/** TCI for (GSWIP-3.2) B-Step
	    Bit [0:11] - VLAN ID
	    Bit [12] - VLAN CFI/DEI
	    Bit [13:15] - VLAN PRI */
	u16 nTci;
	/** first bridge port ID (supported in GSWIP-3.3only) */
	u16 nFirstBridgePortId;
} GSW_MAC_tableQuery_t;

/** \brief MAC Table Entry to be added.
    Used by \ref GSW_MAC_TableEntryAdd. */
typedef struct {
	/** Filtering Identifier (FID) (not supported by all switches) */
	u16 nFId;
	/** Ethernet Port number (zero-based counting) in GSWIP-2.1/2.2/3.0. From
	    GSWIP-3.1, this field is Bridge Port ID. The valid range is hardware
	    dependent.

	    \remarks
	    In GSWIP-2.1/2.2/3.0, this field is used as portmap field, when the MSB
	    bit is set. In portmap mode, every value bit represents an Ethernet port.
	    LSB represents Port 0 with incrementing counting.
	    The (MSB - 1) bit represent the last port.
	    The macro \ref GSW_PORTMAP_FLAG_SET allows to set the MSB bit,
	    marking it as portmap variable.
	    Checking the portmap flag can be done by
	    using the \ref GSW_PORTMAP_FLAG_GET macro.
	    From GSWIP3.1, if MSB is set, other bits in this field are ignored.
	    array \ref GSW_MAC_tableRead_t::nPortMap is used for bit map. */
	u32 nPortId;
	/** Bridge Port Map - to support GSWIP-3.1, following field is added
	    for port map in static entry. It's valid only when MSB of
	    \ref GSW_MAC_tableRead_t::nPortId is set. Each bit stands for 1 bridge
	    port. */
	u16 nPortMap[8];	/* max can be 16 */
	/** Sub-Interface Identifier Destination (supported in GSWIP-3.0/3.1 only).

	    \remarks
	    In GSWIP-3.1, this field is sub interface ID for WLAN logical port. For
	    Other types, either outer VLAN ID if Nto1Vlan enabled or 0. */
	u16 nSubIfId;
	/** Aging Time, given in multiples of 1 second in a range
	    from 1 s to 1,000,000 s.
	    The configured value might be rounded that it fits to the given hardware platform. */
	int nAgeTimer;
	/** STAG VLAN Id. Only applicable in case SVLAN support is enabled on the device. */
	u16 nSVLAN_Id;
	/** Static Entry (value will be aged out if the entry is not set to static). The
	    switch API implementation uses the maximum age timer in case the entry
	    is not static. */
	gsw_bool_t bStaticEntry;
	/** Egress queue traffic class.
	    The queue index starts counting from zero.   */
	u8 nTrafficClass;
	/** MAC Address to add to the table. */
	u8 nMAC[GSW_MAC_ADDR_LEN];
	/** Source/Destination MAC address filtering flag (GSWIP-3.1 only)
	    Value 0 - not filter, 1 - source address filter,
	    2 - destination address filter, 3 - both source and destination filter.

	    \remarks
	    Please refer to "GSWIP Hardware Architecture Spec" chapter 3.4.4.6
	    "Source MAC Address Filtering and Destination MAC Address Filtering"
	    for more detail. */
	u8 nFilterFlag;
	/** Packet is marked as IGMP controlled if destination MAC address matches
	    MAC in this entry. (GSWIP-3.1 only) */
	gsw_bool_t bIgmpControlled;

	/** Associated Mac address -(GSWIP-3.2)*/
	u8 nAssociatedMAC[GSW_MAC_ADDR_LEN];

	/** TCI for (GSWIP-3.2) B-Step
	    Bit [0:11] - VLAN ID
	    Bit [12] - VLAN CFI/DEI
	    Bit [13:15] - VLAN PRI */
	u16 nTci;
} GSW_MAC_tableAdd_t;

/** \brief MAC Table Entry to be removed.
    Used by \ref GSW_MAC_TableEntryRemove. */
typedef struct {
	/** Filtering Identifier (FID) (not supported by all switches) */
	u16 nFId;
	/** MAC Address to be removed from the table. */
	u8 nMAC[GSW_MAC_ADDR_LEN];
	/** Source/Destination MAC address filtering flag (GSWIP-3.1 only)
	    Value 0 - not filter, 1 - source address filter,
	    2 - destination address filter, 3 - both source and destination filter.

	    \remarks
	    Please refer to "GSWIP Hardware Architecture Spec" chapter 3.4.4.6
	    "Source MAC Address Filtering and Destination MAC Address Filtering"
	    for more detail. */
	u8 nFilterFlag;
	/** TCI for (GSWIP-3.2) B-Step
	    Bit [0:11] - VLAN ID
	    Bit [12] - VLAN CFI/DEI
	    Bit [13:15] - VLAN PRI */
	u16 nTci;
} GSW_MAC_tableRemove_t;

/** \brief MAC Table Clear Type
    Used by \ref GSW_MAC_tableClearCond_t */
typedef enum {
	/** Clear dynamic entries based on Physical Port */
	GSW_MAC_CLEAR_PHY_PORT = 0,
	/** Clear all dynamic entries */
	GSW_MAC_CLEAR_DYNAMIC = 1,
} GSW_MacClearType_t;

/** \brief MAC Table Clear based on given condition.
    Used by \ref GSW_MAC_TableClearCond. */
typedef struct {
	/** MAC table clear type \ref GSW_MacClearType_t */
	u8 eType;
	union {
		/** Physical port id (0~16) if \ref eType is
		    ref GSW_MAC_CLEAR_PHY_PORT. */
		u8 nPortId;
	};
} GSW_MAC_tableClearCond_t;

/** \brief MAC Address Filter Type.
    Used by \ref GSW_MACFILTER_default_t */
typedef enum {
	/** Source MAC Address Filter */
	GSW_MACFILTERTYPE_SRC = 0,
	/** Destination MAC Address Filter */
	GSW_MACFILTERTYPE_DEST = 1
} GSW_MacFilterType_t;

/** \brief Default MAC Address Filter.
    Used by \ref GSW_DefaultMacFilterSet and \ref GSW_DefaultMacFilterGet */
typedef struct {
	/** MAC Filter Type */
	GSW_MacFilterType_t eType;

	/** Destination bridge port map. For GSWIP-3.1 only.

	    \remarks
	    Each bit stands for 1 bridge port. For PRX300 (GSWIP-3.1 integrated),
	    only index 0-7 is valid. */
	u16 nPortmap[8];	/* max can be 16 */
} GSW_MACFILTER_default_t;

/** \brief Detect MAC violation for input map of bridge ports.
 *  Used by \ref GSW_MAC_TableLoopDetect
 */
typedef struct {
	/** Input bridge port map to check MAC violation */
	u32 bp_map_in[128 / 32];
	/** Output bridge port map where the MAC violation is found */
	u32 bp_map_out[128 / 32];
} GSW_MAC_tableLoopDetect_t;

/** \brief Packet forwarding.
    Used by \ref GSW_STP_BPDU_Rule_t and \ref GSW_multicastSnoopCfg_t. */
typedef enum {
	/** Default; portmap is determined by the forwarding classification. */
	GSW_PORT_FORWARD_DEFAULT = 0,
	/** Discard; discard packets. */
	GSW_PORT_FORWARD_DISCARD = 1,
	/** Forward to the CPU port. This requires that the CPU port is previously
	    set by calling \ref GSW_CPU_PortCfgSet. */
	GSW_PORT_FORWARD_CPU = 2,
	/** Forward to a port, selected by the parameter 'nForwardPortId'.
	    Please note that this feature is not supported by all
	    hardware platforms. */
	GSW_PORT_FORWARD_PORT = 3
} GSW_portForward_t;

/** \brief Spanning Tree Protocol port states.
    Used by \ref GSW_STP_portCfg_t. */
typedef enum {
	/** Forwarding state. The port is allowed to transmit and receive
	    all packets. Address Learning is allowed. */
	GSW_STP_PORT_STATE_FORWARD = 0,
	/** Disabled/Discarding state. The port entity will not transmit
	    and receive any packets. Learning is disabled in this state. */
	GSW_STP_PORT_STATE_DISABLE = 1,
	/** Learning state. The port entity will only transmit and receive
	    Spanning Tree Protocol packets (BPDU). All other packets are discarded.
	    MAC table address learning is enabled for all good frames. */
	GSW_STP_PORT_STATE_LEARNING = 2,
	/** Blocking/Listening. Only the Spanning Tree Protocol packets will
	    be received and transmitted. All other packets are discarded by
	    the port entity. MAC table address learning is disabled in this
	    state. */
	GSW_STP_PORT_STATE_BLOCKING = 3
} GSW_STP_PortState_t;

/** \brief Configures the Spanning Tree Protocol state of an Ethernet port.
    Used by \ref GSW_STP_PortCfgSet
    and \ref GSW_STP_PortCfgGet. */
typedef struct {
	/** Ethernet Port number (zero-based counting) in GSWIP-2.1/2.2/3.0. From
	    GSWIP-3.1, this field is Bridge Port ID. The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. */
	u16 nPortId;
	/** Filtering Identifier (FID) (not supported by all switches).
	    The FID allows to keep multiple STP states per physical Ethernet port.
	    Multiple CTAG VLAN groups could be a assigned to one FID and therefore
	    share the same STP port state. Switch API ignores the FID value
	    in case the switch device does not support it or switch CTAG VLAN
	    awareness is disabled. */
	u16 nFId;
	/** Spanning Tree Protocol state of the port. */
	GSW_STP_PortState_t ePortState;
} GSW_STP_portCfg_t;

/** \brief Spanning tree packet detection and forwarding.
    Used by \ref GSW_STP_BPDU_RuleSet
    and \ref GSW_STP_BPDU_RuleGet. */
typedef struct {
	/** Filter spanning tree packets and forward them, discard them or
	    disable the filter. */
	GSW_portForward_t eForwardPort;
	/** Target (bridge) port for forwarded packets; only used if selected by
	    'eForwardPort'. Forwarding is done
	    if 'eForwardPort = GSW_PORT_FORWARD_PORT'. */
	u8 nForwardPortId;
} GSW_STP_BPDU_Rule_t;

/** \brief  SSB Arbitration memory mode */
typedef enum {
	/* GSWIP will be 10 ports mode */
	GSW_10_PORT_MODE = 0,
	/* GSWIP will be 16 ports mode */
	GSW_16_PORT_MODE = 1,
} GSW_SSB_Arb_Mode_t;

/** \brief Bridge Port Allocation.
    Used by \ref GSW_BridgePortAlloc and \ref GSW_BridgePortFree. */
typedef struct {
	/** If \ref GSW_BridgePortAlloc is successful, a valid ID will be returned
	    in this field. Otherwise, \ref INVALID_HANDLE is returned in this field.
	    For \ref GSW_BridgePortFree, this field should be valid ID returned by
	    \ref GSW_BridgePortAlloc. ID 0 is special for CPU port in PRX300
	    by mapping to CTP 0 (Logical Port 0 with Sub-interface ID 0), and
	    pre-alloced during initialization. */
	u16 nBridgePortId;
} GSW_BRIDGE_portAlloc_t;

/** \brief Color Remarking Mode
    Used by \ref GSW_CTP_portConfig_t. */
typedef enum {
	/** values from last process stage */
	GSW_REMARKING_NONE = 0,
	/** DEI mark mode */
	GSW_REMARKING_DEI = 2,
	/** PCP 8P0D mark mode */
	GSW_REMARKING_PCP_8P0D = 3,
	/** PCP 7P1D mark mode */
	GSW_REMARKING_PCP_7P1D = 4,
	/** PCP 6P2D mark mode */
	GSW_REMARKING_PCP_6P2D = 5,
	/** PCP 5P3D mark mode */
	GSW_REMARKING_PCP_5P3D = 6,
	/** DSCP AF class */
	GSW_REMARKING_DSCP_AF = 7
} GSW_ColorRemarkingMode_t;

/** \brief Meters for various egress traffic type.
    Used by \ref GSW_BRIDGE_portConfig_t. */
typedef enum {
	/** Index of broadcast traffic meter */
	GSW_BRIDGE_PORT_EGRESS_METER_BROADCAST = 0,
	/** Index of known multicast traffic meter */
	GSW_BRIDGE_PORT_EGRESS_METER_MULTICAST = 1,
	/** Index of unknown multicast IP traffic meter */
	GSW_BRIDGE_PORT_EGRESS_METER_UNKNOWN_MC_IP = 2,
	/** Index of unknown multicast non-IP traffic meter */
	GSW_BRIDGE_PORT_EGRESS_METER_UNKNOWN_MC_NON_IP = 3,
	/** Index of unknown unicast traffic meter */
	GSW_BRIDGE_PORT_EGRESS_METER_UNKNOWN_UC = 4,
	/** Index of traffic meter for other types */
	GSW_BRIDGE_PORT_EGRESS_METER_OTHERS = 5,
	/** Number of index */
	GSW_BRIDGE_PORT_EGRESS_METER_MAX = 6
} GSW_BridgePortEgressMeter_t;

/** \brief P-mapper Mapping Mode
    Used by \ref GSW_CTP_portConfig_t. */
typedef enum {
	/** Use PCP for VLAN tagged packets to derive sub interface ID group.

	    \remarks
	    P-mapper table entry 1-8. */
	GSW_PMAPPER_MAPPING_PCP = 0,
	/** Use LAG Index for Pmapper access (regardless of IP and VLAN packet)to
	    derive sub interface ID group.

	    \remarks
	    P-mapper table entry 9-72. */
	GSW_PMAPPER_MAPPING_LAG = 1,
	/** Use DSCP for VLAN tagged IP packets to derive sub interface ID group.

	    \remarks
	    P-mapper table entry 9-72. */
	GSW_PMAPPER_MAPPING_DSCP = 2,
} GSW_PmapperMappingMode_t;

/** \brief P-mapper Configuration
    Used by \ref GSW_CTP_portConfig_t, GSW_BRIDGE_portConfig_t.
    In case of LAG, it is user's responsibility to provide the mapped entries
    in given P-mapper table. In other modes the entries are auto mapped from
    input packet. */
typedef struct {
	/** Index of P-mapper <0-31>. */
	u16 nPmapperId;

	/** Sub interface ID group.

	    \remarks
	    Entry 0 is for non-IP and non-VLAN tagged packets. Entries 1-8 are PCP
	    mapping entries for VLAN tagged packets with \ref GSW_PMAPPER_MAPPING_PCP
	    selected. Entries 9-72 are DSCP or LAG mapping entries for IP packets without
	    VLAN tag or VLAN tagged packets with \ref GSW_PMAPPER_MAPPING_DSCP or
	    GSW_PMAPPER_MAPPING_LAG selected. When LAG is selected this 8bit field is
	    decoded as Destination sub-interface ID group field bits 3:0, Destination
	    logical port ID field bits 7:4 */
	u8 nDestSubIfIdGroup[73];
} GSW_PMAPPER_t;

/** \brief Bridge Port configuration mask.
    Used by \ref GSW_BRIDGE_portConfig_t. */
typedef enum {
	/** Mask for \ref GSW_BRIDGE_portConfig_t::nBridgeId */
	GSW_BRIDGE_PORT_CONFIG_MASK_BRIDGE_ID = 0x00000001,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bIngressExtendedVlanEnable and
	    \ref GSW_BRIDGE_portConfig_t::nIngressExtendedVlanBlockId */
	GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_VLAN = 0x00000002,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bEgressExtendedVlanEnable and
	    \ref GSW_BRIDGE_portConfig_t::nEgressExtendedVlanBlockId */
	GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_VLAN = 0x00000004,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::eIngressMarkingMode */
	GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_MARKING = 0x00000008,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::eEgressRemarkingMode */
	GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_REMARKING = 0x00000010,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bIngressMeteringEnable and
	    \ref GSW_BRIDGE_portConfig_t::nIngressTrafficMeterId */
	GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_METER = 0x00000020,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bEgressSubMeteringEnable and
	    \ref GSW_BRIDGE_portConfig_t::nEgressTrafficSubMeterId */
	GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_SUB_METER = 0x00000040,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::nDestLogicalPortId,
	    \ref GSW_BRIDGE_portConfig_t::bPmapperEnable,
	    \ref GSW_BRIDGE_portConfig_t::nDestSubIfIdGroup,
	    \ref GSW_BRIDGE_portConfig_t::ePmapperMappingMode,
	    \ref GSW_BRIDGE_portConfig_t::bPmapperIdValid and
	    \ref GSW_BRIDGE_portConfig_t::sPmapper. */
	GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_CTP_MAPPING = 0x00000080,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::nBridgePortMap */
	GSW_BRIDGE_PORT_CONFIG_MASK_BRIDGE_PORT_MAP = 0x00000100,

	/** Mask for \ref GSW_BRIDGE_portConfig_t::bMcDestIpLookupDisable. */
	GSW_BRIDGE_PORT_CONFIG_MASK_MC_DEST_IP_LOOKUP = 0x00000200,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bMcSrcIpLookupEnable. */
	GSW_BRIDGE_PORT_CONFIG_MASK_MC_SRC_IP_LOOKUP = 0x00000400,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bDestMacLookupDisable. */
	GSW_BRIDGE_PORT_CONFIG_MASK_MC_DEST_MAC_LOOKUP = 0x00000800,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bSrcMacLearningDisable. */
	GSW_BRIDGE_PORT_CONFIG_MASK_MC_SRC_MAC_LEARNING = 0x00001000,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bMacSpoofingDetectEnable. */
	GSW_BRIDGE_PORT_CONFIG_MASK_MAC_SPOOFING = 0x00002000,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bPortLockEnable. */
	GSW_BRIDGE_PORT_CONFIG_MASK_PORT_LOCK = 0x00004000,

	/** Mask for \ref GSW_BRIDGE_portConfig_t::bMacLearningLimitEnable and
	    \ref GSW_BRIDGE_portConfig_t::nMacLearningLimit. */
	GSW_BRIDGE_PORT_CONFIG_MASK_MAC_LEARNING_LIMIT = 0x00008000,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::nMacLearningCount */
	GSW_BRIDGE_PORT_CONFIG_MASK_MAC_LEARNED_COUNT = 0x00010000,

	/** Mask for \ref GSW_BRIDGE_portConfig_t::bIngressVlanFilterEnable and
	    \ref GSW_BRIDGE_portConfig_t::nIngressVlanFilterBlockId. */
	GSW_BRIDGE_PORT_CONFIG_MASK_INGRESS_VLAN_FILTER = 0x00020000,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bBypassEgressVlanFilter1,
	    \ref GSW_BRIDGE_portConfig_t::bEgressVlanFilter1Enable
	    and \ref GSW_BRIDGE_portConfig_t::nEgressVlanFilter1BlockId. */
	GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_VLAN_FILTER1 = 0x00040000,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bEgressVlanFilter2Enable and
	    \ref GSW_BRIDGE_portConfig_t::nEgressVlanFilter2BlockId. */
	GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_VLAN_FILTER2 = 0x00080000,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bVlanTagSelection,
	    \ref GSW_BRIDGE_portConfig_t::bVlanSrcMacPriorityEnable,
	    \ref GSW_BRIDGE_portConfig_t::bVlanSrcMacDEIEnable,
	    \ref GSW_BRIDGE_portConfig_t::bVlanSrcMacVidEnable,
	    \ref GSW_BRIDGE_portConfig_t::bVlanDstMacPriorityEnable,
	    \ref GSW_BRIDGE_portConfig_t::bVlanDstMacDEIEnable,
	    \ref GSW_BRIDGE_portConfig_t::bVlanDstMacVidEnable */
	GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MAC_LEARNING = 0x00100000,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::bVlanMulticastPriorityEnable,
	    \ref GSW_BRIDGE_portConfig_t::bVlanMulticastDEIEnable,
	    \ref GSW_BRIDGE_portConfig_t::bVlanMulticastVidEnable */
	GSW_BRIDGE_PORT_CONFIG_MASK_VLAN_BASED_MULTICAST_LOOKUP = 0x00200000,
	/** Mask for \ref GSW_BRIDGE_portConfig_t::nLoopViolationCount */
	GSW_BRIDGE_PORT_CONFIG_MASK_LOOP_VIOLATION_COUNTER = 0x00400000,
	/** Enable all */
	GSW_BRIDGE_PORT_CONFIG_MASK_ALL = 0x7FFFFFFF,
	/** Bypass any check for debug purpose */
	GSW_BRIDGE_PORT_CONFIG_MASK_FORCE = 0x80000000
} GSW_BridgePortConfigMask_t;

/** \brief Color Marking Mode
    Used by \ref GSW_CTP_portConfig_t. */
typedef enum {
	/** mark packets (except critical) to green */
	GSW_MARKING_ALL_GREEN = 0,
	/** do not change color and priority */
	GSW_MARKING_INTERNAL_MARKING = 1,
	/** DEI mark mode */
	GSW_MARKING_DEI = 2,
	/** PCP 8P0D mark mode */
	GSW_MARKING_PCP_8P0D = 3,
	/** PCP 7P1D mark mode */
	GSW_MARKING_PCP_7P1D = 4,
	/** PCP 6P2D mark mode */
	GSW_MARKING_PCP_6P2D = 5,
	/** PCP 5P3D mark mode */
	GSW_MARKING_PCP_5P3D = 6,
	/** DSCP AF class */
	GSW_MARKING_DSCP_AF = 7
} GSW_ColorMarkingMode_t;

/** \brief Bridge configuration mask.
    Used by \ref GSW_BRIDGE_config_t. */
typedef enum {
	/** Mask for \ref GSW_BRIDGE_config_t::bMacLearningLimitEnable
	    and \ref GSW_BRIDGE_config_t::nMacLearningLimit. */
	GSW_BRIDGE_CONFIG_MASK_MAC_LEARNING_LIMIT = 0x00000001,
	/** Mask for \ref GSW_BRIDGE_config_t::nMacLearningCount */
	GSW_BRIDGE_CONFIG_MASK_MAC_LEARNED_COUNT = 0x00000002,
	/** Mask for \ref GSW_BRIDGE_config_t::nLearningDiscardEvent */
	GSW_BRIDGE_CONFIG_MASK_MAC_DISCARD_COUNT = 0x00000004,
	/** Mask for \ref GSW_BRIDGE_config_t::bSubMeteringEnable and
	    \ref GSW_BRIDGE_config_t::nTrafficSubMeterId */
	GSW_BRIDGE_CONFIG_MASK_SUB_METER = 0x00000008,
	/** Mask for \ref GSW_BRIDGE_config_t::eForwardBroadcast,
	    \ref GSW_BRIDGE_config_t::eForwardUnknownMulticastIp,
	    \ref GSW_BRIDGE_config_t::eForwardUnknownMulticastNonIp,
	    and \ref GSW_BRIDGE_config_t::eForwardUnknownUnicast. */
	GSW_BRIDGE_CONFIG_MASK_FORWARDING_MODE = 0x00000010,

	/** Enable all */
	GSW_BRIDGE_CONFIG_MASK_ALL = 0x7FFFFFFF,
	/** Bypass any check for debug purpose */
	GSW_BRIDGE_CONFIG_MASK_FORCE = 0x80000000
} GSW_BridgeConfigMask_t;

/** \brief Bridge forwarding type of packet.
    Used by \ref GSW_BRIDGE_portConfig_t. */
typedef enum {
	/** Packet is flooded to port members of ingress bridge port */
	GSW_BRIDGE_FORWARD_FLOOD = 0,
	/** Packet is dscarded */
	GSW_BRIDGE_FORWARD_DISCARD = 1,
	/** Packet is forwarded to logical port 0 CTP port 0 bridge port 0 */
	GSW_BRIDGE_FORWARD_CPU = 2
} GSW_BridgeForwardMode_t;


/** \brief Bridge Port Configuration.
    Used by \ref GSW_BridgePortConfigSet and \ref GSW_BridgePortConfigGet. */
typedef struct {
	/** Bridge Port ID allocated by \ref GSW_BridgePortAlloc.

	    \remarks
	    If \ref GSW_BRIDGE_portConfig_t::eMask has
	    \ref GSW_BridgePortConfigMask_t::GSW_BRIDGE_PORT_CONFIG_MASK_FORCE, this
	    field is absolute index of Bridge Port in hardware for debug purpose,
	    bypassing any check. */
	u16 nBridgePortId;

	/** Mask for updating/retrieving fields. */
	GSW_BridgePortConfigMask_t eMask;

	/** Bridge ID (FID) to which this bridge port is associated. A default
	    bridge (ID 0) should be always available. */
	u16 nBridgeId;

	/** Enable extended VLAN processing for ingress traffic. */
	gsw_bool_t bIngressExtendedVlanEnable;
	/** Extended VLAN block allocated for ingress traffic. It defines
	    extended VLAN process for ingress traffic. Valid when
	    bIngressExtendedVlanEnable is TRUE. */
	u16 nIngressExtendedVlanBlockId;
	/** Extended VLAN block size for ingress traffic. This is optional.
	    If it is 0, the block size of nIngressExtendedVlanBlockId will be used.
	    Otherwise, this field will be used. */
	u16 nIngressExtendedVlanBlockSize;

	/** Enable extended VLAN processing enabled for egress traffic. */
	gsw_bool_t bEgressExtendedVlanEnable;
	/** Extended VLAN block allocated for egress traffic. It defines
	    extended VLAN process for egress traffic. Valid when
	    bEgressExtendedVlanEnable is TRUE. */
	u16 nEgressExtendedVlanBlockId;
	/** Extended VLAN block size for egress traffic. This is optional.
	    If it is 0, the block size of nEgressExtendedVlanBlockId will be used.
	    Otherwise, this field will be used. */
	u16 nEgressExtendedVlanBlockSize;

	/** Ingress color marking mode for ingress traffic. */
	GSW_ColorMarkingMode_t eIngressMarkingMode;

	/** Color remarking for egress traffic. */
	GSW_ColorRemarkingMode_t eEgressRemarkingMode;

	/** Traffic metering on ingress traffic applies. */
	gsw_bool_t bIngressMeteringEnable;
	/** Meter for ingress Bridge Port process.

	    \remarks
	    Meter should be allocated with \ref GSW_QOS_MeterAlloc before Bridge
	    port configuration. If this Bridge port is re-set, the last used meter
	    should be released. */
	u16 nIngressTrafficMeterId;

	/** Traffic metering on various types of egress traffic (such as broadcast,
	    multicast, unknown unicast, etc) applies. */
	gsw_bool_t bEgressSubMeteringEnable[GSW_BRIDGE_PORT_EGRESS_METER_MAX];
	/** Meter for egress Bridge Port process with specific type (such as
	    broadcast, multicast, unknown unicast, etc). Need pre-allocated for each
	    type. */
	u16 nEgressTrafficSubMeterId[GSW_BRIDGE_PORT_EGRESS_METER_MAX];

	/** This field defines destination logical port. */
	u8 nDestLogicalPortId;
	/** This field indicates whether to enable P-mapper. */
	gsw_bool_t bPmapperEnable;
	/** When bPmapperEnable is FALSE, this field defines destination sub
	    interface ID group. */
	u16 nDestSubIfIdGroup;
	/** When bPmapperEnable is TRUE, this field selects either DSCP or PCP to
	    derive sub interface ID. */
	GSW_PmapperMappingMode_t ePmapperMappingMode;
	/** When bPmapperEnable is TRUE, P-mapper is used. This field determines
	    whether sPmapper.nPmapperId is valid. If this field is TRUE, the
	    P-mapper is re-used and no allocation of new P-mapper or value
	    change in the P-mapper. If this field is FALSE, allocation is
	    taken care in the API implementation. */
	gsw_bool_t bPmapperIdValid;
	/** When bPmapperEnable is TRUE, P-mapper is used. if bPmapperIdValid is
	    FALSE, API implementation need take care of P-mapper allocation,
	    and maintain the reference counter of P-mapper used multiple times.
	    If bPmapperIdValid is TRUE, only sPmapper.nPmapperId is used to
	    associate the P-mapper, and there is no allocation of new P-mapper
	    or value change in the P-mapper. */
	GSW_PMAPPER_t sPmapper;

	/** Port map define broadcast domain.

	    \remarks
	    Each bit is one bridge port. Bridge port ID is index * 16 + bit offset.
	    For example, bit 1 of nBridgePortMap[1] is bridge port ID 17. */
	u16 nBridgePortMap[8];	/* max can be 16 */

	/** Multicast IP table is searched if this field is FALSE and traffic is IP
	    multicast. */
	gsw_bool_t bMcDestIpLookupDisable;
	/** Multicast IP table is searched if this field is TRUE and traffic is IP
	    multicast. */
	gsw_bool_t bMcSrcIpLookupEnable;

	/** Default is FALSE. Packet is treated as "unknown" if it's not
	    broadcast/multicast packet. */
	gsw_bool_t bDestMacLookupDisable;

	/** Default is FALSE. Source MAC address is learned. */
	gsw_bool_t bSrcMacLearningDisable;

	/** If this field is TRUE and MAC address which is already learned in another
	    bridge port appears on this bridge port, port locking violation is
	    detected. */
	gsw_bool_t bMacSpoofingDetectEnable;

	/** If this field is TRUE and MAC address which is already learned in this
	    bridge port appears on another bridge port, port locking violation is
	    detected. */
	gsw_bool_t bPortLockEnable;

	/** Enable MAC learning limitation. */
	gsw_bool_t bMacLearningLimitEnable;
	/** Max number of MAC can be learned from this bridge port. */
	u16 nMacLearningLimit;

	/** Get number of Loop violation counter from this bridge port. */
	u16 nLoopViolationCount;

	/** Get number of MAC address learned from this bridge port. */
	u16 nMacLearningCount;

	/** Enable ingress VLAN filter */
	gsw_bool_t bIngressVlanFilterEnable;
	/** VLAN filter block of ingress traffic if
	    \ref GSW_BRIDGE_portConfig_t::bIngressVlanFilterEnable is TRUE. */
	u16 nIngressVlanFilterBlockId;
	/** VLAN filter block size. This is optional.
	    If it is 0, the block size of nIngressVlanFilterBlockId will be used.
	    Otherwise, this field will be used. */
	u16 nIngressVlanFilterBlockSize;
	/** For ingress traffic, bypass VLAN filter 1 at egress bridge port
	    processing. */
	gsw_bool_t bBypassEgressVlanFilter1;
	/** Enable egress VLAN filter 1 */
	gsw_bool_t bEgressVlanFilter1Enable;
	/** VLAN filter block 1 of egress traffic if
	    \ref GSW_BRIDGE_portConfig_t::bEgressVlanFilter1Enable is TRUE. */
	u16 nEgressVlanFilter1BlockId;
	/** VLAN filter block 1 size. This is optional.
	    If it is 0, the block size of nEgressVlanFilter1BlockId will be used.
	    Otherwise, this field will be used. */
	u16 nEgressVlanFilter1BlockSize;
	/** Enable egress VLAN filter 2 */
	gsw_bool_t bEgressVlanFilter2Enable;
	/** VLAN filter block 2 of egress traffic if
	    \ref GSW_BRIDGE_portConfig_t::bEgressVlanFilter2Enable is TRUE. */
	u16 nEgressVlanFilter2BlockId;
	/** VLAN filter block 2 size. This is optional.
	    If it is 0, the block size of nEgressVlanFilter2BlockId will be used.
	    Otherwise, this field will be used. */
	u16 nEgressVlanFilter2BlockSize;

	/** 0 - Intermediate outer VLAN
	    tag is used for MAC address/multicast
	    learning, lookup and filtering.
	    1 - Original outer VLAN tag is used
	    for MAC address/multicast learning, lookup
	    and filtering. */
	gsw_bool_t bVlanTagSelection;
	/** 0 - Disable, VLAN Priority field is not used
	    and value 0 is used for source MAC address
	    learning and filtering.
	    1 - Enable, VLAN Priority field is used for
	    source MAC address learning and filtering. */
	gsw_bool_t bVlanSrcMacPriorityEnable;
	/** 0 - Disable, VLAN DEI/CFI field is not used
	    and value 0 is used for source MAC address
	    learning and filtering.
	    1 -	 Enable, VLAN DEI/CFI field is used for
	    source MAC address learning and filtering */
	gsw_bool_t bVlanSrcMacDEIEnable;
	/** 0 - Disable, VLAN ID field is not used and
	    value 0 is used for source MAC address
	    learning and filtering
	    1 - Enable, VLAN ID field is used for source
	    MAC address learning and filtering. */
	gsw_bool_t bVlanSrcMacVidEnable;
	/** 0 - Disable, VLAN Priority field is not used
	    and value 0 is used for destination MAC
	    address look up and filtering.
	    1 - Enable, VLAN Priority field is used for
	    destination MAC address look up and
	    filtering */
	gsw_bool_t bVlanDstMacPriorityEnable;
	/** 0 - Disable, VLAN CFI/DEI field is not used
	    and value 0 is used for destination MAC
	    address lookup and filtering.
	    1 - Enable, VLAN CFI/DEI field is used for
	    destination MAC address look up and
	    filtering. */
	gsw_bool_t bVlanDstMacDEIEnable;
	/** 0 - Disable, VLAN ID field is not used and
	    value 0 is used for destination MAC address
	    look up and filtering.
	    1 - Enable, VLAN ID field is destination for
	    destination MAC address look up and
	    filtering. */
	gsw_bool_t bVlanDstMacVidEnable;

	/** 0 - Disable, VLAN Priority field is not used
	    and value 0 is used for IP multicast lookup.
	    1 - Enable, VLAN Priority field is used for IP
	    multicast lookup. */
	gsw_bool_t bVlanMulticastPriorityEnable;
	/** 0 - Disable, VLAN CFI/DEI field is not used
	    and value 0 is used for IP multicast lookup.
	    1 - Enable, VLAN CFI/DEI field is used for IP
	    multicast lookup. */
	gsw_bool_t bVlanMulticastDEIEnable;
	/** 0 - Disable, VLAN ID field is not used and
	    value 0 is used for IP multicast lookup.
	    1 - Enable, VLAN ID field is destination for IP
	    multicast lookup. */
	gsw_bool_t bVlanMulticastVidEnable;
} GSW_BRIDGE_portConfig_t;

/** \brief Bridge Port Loop Violation Counter Read and Clear.
    Used by \ref GSW_BridgePortLoopRead. */
typedef struct {
	/** Bridge Port ID allocated by \ref GSW_BridgePortAlloc. */
	u16 nBridgePortId;

	/** Loop violation counter saturated at 255 */
	u16 nLoopViolationCount;
} GSW_BRIDGE_portLoopRead_t;

/** \brief Read Hit Status.
    Used to read Multicast/Mac hit stats. */
typedef struct {
	/** Mac/multicast Table Id */
	u8 table_id;
	/** Table Index to read */
	u16 nIndex;
	/** Hit status */
	u8 hitStatus;
	/** Entry Validity, if 1 entry is valid */
	u8 nValid;
} GSW_HitStatusRead_t;

/** \brief Miscellaneous configuration of logical port
 *  Used by \ref GSW_MiscPortCfgGet to get or \ref GSW_MiscPortCfgSet to
 *  configure miscellaneous configurations such as 4-TPID support
 *  on logical port (0~15).
 */
typedef struct {
	/** reserved for future extension */
	u32 mask;
	/** logical port id
	 *  value 0~15 is logical port
	 *  value 255 is special value to apply configuration on all
	 *  logical ports and only used by \ref GSW_MiscPortCfgSet
	 */
	u8 port;
	/** flag to indicate enable of 4-TPID mode
	 *  1 - 4-TPID mode (metering function in Extended VLAN is disabled)
	 *  0 - 2-TPID mode
	 *  in \ref GSW_MiscPortCfgGet, this field is output
	 *  in \ref GSW_MiscPortCfgSet,
	 *    if port == 255, this is input only
	 *    otherwise, this is input as well as output
	 *    the original value before modification is returned
	 */
	unsigned char TPID4: 1;
} GSW_MiscPortCfg_t;

/** \brief VLAN Filter TCI Mask.
    Used by \ref GSW_VLANFILTER_config_t */
typedef enum {
	GSW_VLAN_FILTER_TCI_MASK_VID = 0,
	GSW_VLAN_FILTER_TCI_MASK_PCP = 1,
	GSW_VLAN_FILTER_TCI_MASK_TCI = 2
} GSW_VlanFilterTciMask_t;


/** \brief Bridge Allocation.
    Used by \ref GSW_BridgeAlloc and \ref GSW_BridgeFree. */
typedef struct {
	/** If \ref GSW_BridgeAlloc is successful, a valid ID will be returned
	    in this field. Otherwise, \ref INVALID_HANDLE is returned in this field.
	    For \ref GSW_BridgeFree, this field should be valid ID returned by
	    \ref GSW_BridgeAlloc. ID 0 is special Bridge created during
	    initialization. */
	u16 nBridgeId;
} GSW_BRIDGE_alloc_t;

/** \brief Bridge Configuration.
    Used by \ref GSW_BridgeConfigSet and \ref GSW_BridgeConfigGet. */
typedef struct {
	/** Bridge ID (FID) allocated by \ref GSW_BridgeAlloc.

	    \remarks
	    If \ref GSW_BRIDGE_config_t::eMask has
	    \ref GSW_BridgeConfigMask_t::GSW_BRIDGE_CONFIG_MASK_FORCE, this field is
	    absolute index of Bridge (FID) in hardware for debug purpose, bypassing
	    any check. */
	u16 nBridgeId;

	/** Mask for updating/retrieving fields. */
	GSW_BridgeConfigMask_t eMask;

	/** Enable MAC learning limitation. */
	gsw_bool_t bMacLearningLimitEnable;
	/** Max number of MAC can be learned in this bridge (all bridge ports). */
	u16 nMacLearningLimit;

	/** Get number of MAC address learned from this bridge port. */
	u16 nMacLearningCount;

	/** Number of learning discard event due to hardware resource not available.

	    \remarks
	    This is discard event due to either MAC table full or Hash collision.
	    Discard due to nMacLearningCount reached is not counted in this field. */
	u32 nLearningDiscardEvent;

	/** Traffic metering on type of traffic (such as broadcast, multicast,
	    unknown unicast, etc) applies. */
	gsw_bool_t bSubMeteringEnable[GSW_BRIDGE_PORT_EGRESS_METER_MAX];
	/** Meter for bridge process with specific type (such as broadcast,
	    multicast, unknown unicast, etc). Need pre-allocated for each type. */
	u16 nTrafficSubMeterId[GSW_BRIDGE_PORT_EGRESS_METER_MAX];

	/** Forwarding mode of broadcast traffic. */
	GSW_BridgeForwardMode_t eForwardBroadcast;
	/** Forwarding mode of unknown multicast IP traffic. */
	GSW_BridgeForwardMode_t eForwardUnknownMulticastIp;
	/** Forwarding mode of unknown multicast non-IP traffic. */
	GSW_BridgeForwardMode_t eForwardUnknownMulticastNonIp;
	/** Forwarding mode of unknown unicast traffic. */
	GSW_BridgeForwardMode_t eForwardUnknownUnicast;
} GSW_BRIDGE_config_t;

/** \brief Ethernet port speed mode.
    For certain generations of GSWIP, a port might support only a subset of the possible settings.
    Used by \ref GSW_portLinkCfg_t. */
typedef enum {
	/** 10 Mbit/s */
	GSW_PORT_SPEED_10,
	/** 100 Mbit/s */
	GSW_PORT_SPEED_100,
	/** 200 Mbit/s */
	GSW_PORT_SPEED_200,
	/** 1000 Mbit/s */
	GSW_PORT_SPEED_1000,
	/** 2.5 Gbit/s */
	GSW_PORT_SPEED_2500,
	/** 5 Gbit/s */
	GSW_PORT_SPEED_5000,
	/** 10 Gbit/s */
	GSW_PORT_SPEED_10000,
	/** Auto speed for XGMAC */
	GSW_PORT_SPEED_AUTO,
} GSW_portSpeed_t;

/** \brief Ethernet port duplex status.
    Used by \ref GSW_portLinkCfg_t. */
typedef enum {
	/** Port operates in full-duplex mode */
	GSW_DUPLEX_FULL	= 0,
	/** Port operates in half-duplex mode */
	GSW_DUPLEX_HALF	= 1,
	/** Port operates in Auto mode */
	GSW_DUPLEX_AUTO	= 2,
} GSW_portDuplex_t;

/** \brief Force the MAC and PHY link modus.
    Used by \ref GSW_portLinkCfg_t. */
typedef enum {
	/** Link up. Any connected LED
	    still behaves based on the real PHY status. */
	GSW_PORT_LINK_UP	= 0,
	/** Link down. */
	GSW_PORT_LINK_DOWN	= 1,
	/** Link Auto. */
	GSW_PORT_LINK_AUTO	= 2,
} GSW_portLink_t;

/** \brief Ethernet port interface mode.
    A port might support only a subset of the possible settings.
    Used by \ref GSW_portLinkCfg_t. */
typedef enum {
	/** Normal PHY interface (twisted pair), use the internal MII Interface. */
	GSW_PORT_HW_MII	= 0,
	/** Reduced MII interface in normal mode. */
	GSW_PORT_HW_RMII	= 1,
	/** GMII or MII, depending upon the speed. */
	GSW_PORT_HW_GMII	= 2,
	/** RGMII mode. */
	GSW_PORT_HW_RGMII = 3,
	/** XGMII mode. */
	GSW_PORT_HW_XGMII = 4,
} GSW_MII_Mode_t;

/** \brief Ethernet port configuration for PHY or MAC mode.
    Used by \ref GSW_portLinkCfg_t. */
typedef enum {
	/** MAC Mode. The Ethernet port is configured to work in MAC mode. */
	GSW_PORT_MAC	= 0,
	/** PHY Mode. The Ethernet port is configured to work in PHY mode. */
	GSW_PORT_PHY	= 1
} GSW_MII_Type_t;

/** \brief Ethernet port clock source configuration.
    Used by \ref GSW_portLinkCfg_t. */
typedef enum {
	/** Clock Mode not applicable. */
	GSW_PORT_CLK_NA	= 0,
	/** Clock Master Mode. The port is configured to provide the clock as output signal. */
	GSW_PORT_CLK_MASTER	= 1,
	/** Clock Slave Mode. The port is configured to use the input clock signal. */
	GSW_PORT_CLK_SLAVE	= 2
} GSW_clkMode_t;


/** \brief Ethernet port link, speed status and flow control status.
    Used by \ref GSW_PortLinkCfgGet and \ref GSW_PortLinkCfgSet. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. */
	u16	nPortId;
	/** Force Port Duplex Mode.

	    - 0: Negotiate Duplex Mode. Auto-negotiation mode. Negotiated
	      duplex mode given in 'eDuplex'
	      during GSW_PortLinkCfgGet calls.
	    - 1: Force Duplex Mode. Force duplex mode in 'eDuplex'.
	*/
	gsw_bool_t	bDuplexForce;
	/** Port Duplex Status. */
	GSW_portDuplex_t	eDuplex;
	/** Force Link Speed.

	    - 0: Negotiate Link Speed. Negotiated speed given in
	      'eSpeed' during GSW_PortLinkCfgGet calls.
	    - 1: Force Link Speed. Forced speed mode in 'eSpeed'.
	*/
	gsw_bool_t	bSpeedForce;
	/** Ethernet port link up/down and speed status. */
	GSW_portSpeed_t	eSpeed;
	/** Force Link.

	     - 0: Auto-negotiate Link. Current link status is given in
	       'eLink' during GSW_PortLinkCfgGet calls.
	     - 1: Force Duplex Mode. Force duplex mode in 'eLink'.
	 */
	gsw_bool_t	bLinkForce;
	/** Link Status. Read out the current link status.
	    Note that the link could be forced by setting 'bLinkForce'. */
	GSW_portLink_t	eLink;
	/** Selected interface mode (MII/RMII/RGMII/GMII/XGMII). */
	GSW_MII_Mode_t	eMII_Mode;
	/** Select MAC or PHY mode (PHY = Reverse xMII). */
	GSW_MII_Type_t	eMII_Type;
	/** Interface Clock mode (used for RMII mode). */
	GSW_clkMode_t	eClkMode;
	/** 'Low Power Idle' Support for 'Energy Efficient Ethernet'.
	    Only enable this feature in case the attached PHY also supports it. */
	gsw_bool_t	bLPI;
} GSW_portLinkCfg_t;

/** \brief Port Enable Type Selection.
    Used by \ref GSW_portCfg_t. */
typedef enum {
	/** The port is disabled in both directions. */
	GSW_PORT_DISABLE	= 0,
	/** The port is enabled in both directions (ingress and egress). */
	GSW_PORT_ENABLE_RXTX	= 1,
	/** The port is enabled in the receive (ingress) direction only. */
	GSW_PORT_ENABLE_RX	= 2,
	/** The port is enabled in the transmit (egress) direction only. */
	GSW_PORT_ENABLE_TX	= 3
} GSW_portEnable_t;

/** \brief Ethernet flow control status.
    Used by \ref GSW_portCfg_t. */
typedef enum {
	/** Automatic flow control mode selection through auto-negotiation. */
	GSW_FLOW_AUTO	= 0,
	/** Receive flow control only */
	GSW_FLOW_RX	= 1,
	/** Transmit flow control only */
	GSW_FLOW_TX	= 2,
	/** Receive and Transmit flow control */
	GSW_FLOW_RXTX	= 3,
	/** No flow control */
	GSW_FLOW_OFF	= 4
} GSW_portFlow_t;


/** \brief Port Mirror Options.
    Used by \ref GSW_portCfg_t. */
typedef enum {
	/** Mirror Feature is disabled. Normal port usage. */
	GSW_PORT_MONITOR_NONE	= 0,
	/** Port Ingress packets are mirrored to the monitor port. */
	GSW_PORT_MONITOR_RX	= 1,
	/** Port Egress packets are mirrored to the monitor port. */
	GSW_PORT_MONITOR_TX	= 2,
	/** Port Ingress and Egress packets are mirrored to the monitor port. */
	GSW_PORT_MONITOR_RXTX	= 3,
	/** Packet mirroring of 'unknown VLAN violation' frames. */
	GSW_PORT_MONITOR_VLAN_UNKNOWN          = 4,
	/** Packet mirroring of 'VLAN ingress or egress membership violation' frames. */
	GSW_PORT_MONITOR_VLAN_MEMBERSHIP       = 16,
	/** Packet mirroring of 'port state violation' frames. */
	GSW_PORT_MONITOR_PORT_STATE	= 32,
	/** Packet mirroring of 'MAC learning limit violation' frames. */
	GSW_PORT_MONITOR_LEARNING_LIMIT        = 64,
	/** Packet mirroring of 'port lock violation' frames. */
	GSW_PORT_MONITOR_PORT_LOCK	= 128
} GSW_portMonitor_t;

/** \brief Interface RMON Counter Mode - (FID, SUBID or FLOWID) Config - GSWIP-3.0 only.
    Used by \ref GSW_portCfg_t. */
typedef enum {
	/** FID based Interface RMON counters Usage */
	GSW_IF_RMON_FID	= 0,
	/** Sub-Interface Id based Interface RMON counters Usage */
	GSW_IF_RMON_SUBID	= 1,
	/** Flow Id (LSB bits 3 to 0) based Interface RMON counters Usage */
	GSW_IF_RMON_FLOWID_LSB	= 2,
	/** Flow Id (MSB bits 7 to 4) based Interface RMON counters Usage */
	GSW_IF_RMON_FLOWID_MSB	= 3
} GSW_If_RMON_Mode_t;

/** \brief Port Configuration Parameters.
    Used by \ref GSW_PortCfgGet and \ref GSW_PortCfgSet. */
typedef struct {
	/** Port Type. This gives information which type of port is configured.
	    nPortId should be based on this field. */
	GSW_portType_t ePortType;

	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. */
	u16	nPortId;
	/** Enable Port (ingress only, egress only, both directions, or disabled).
	    This parameter is used for Spanning Tree Protocol and 802.1X applications. */
	GSW_portEnable_t	eEnable;
	/** Drop unknown unicast packets.
	    Do not send out unknown unicast packets on this port,
	    if the boolean parameter is enabled. By default packets of this type
	    are forwarded to this port. */
	gsw_bool_t	bUnicastUnknownDrop;
	/** Drop unknown multicast packets.
	    Do not send out unknown multicast packets on this port,
	    if boolean parameter is enabled. By default packets of this type
	    are forwarded to this port.
	    Some platforms also drop broadcast packets. */
	gsw_bool_t	bMulticastUnknownDrop;
	/** Drop reserved packet types
	    (destination address from '01 80 C2 00 00 00' to
	    '01 80 C2 00 00 2F') received on this port. */
	gsw_bool_t	bReservedPacketDrop;
	/** Drop Broadcast packets received on this port. By default packets of this
	  type are forwarded to this port. */
	gsw_bool_t	bBroadcastDrop;
	/** Enables MAC address table aging.
	    The MAC table entries learned on this port are removed after the
	    aging time has expired.
	    The aging time is a global parameter, common to all ports. */
	gsw_bool_t	bAging;
	/** MAC address table learning on the port specified by 'nPortId'.
	    By default this parameter is always enabled. */
	gsw_bool_t	bLearning;
	/** Automatic MAC address table learning locking on the port specified
	    by 'nPortId'.
	    This parameter is only taken into account when 'bLearning' is enabled. */
	gsw_bool_t	bLearningMAC_PortLock;
	/** Automatic MAC address table learning limitation on this port.
	    The learning functionality is disabled when the limit value is zero.
	    The value 0xFFFF to allow unlimited learned address.
	    This parameter is only taken into account when 'bLearning' is enabled. */
	u16 nLearningLimit;
	/** MAC spoofing detection. Identifies ingress packets that carry
	    a MAC source address which was previously learned on a different ingress
	    port (learned by MAC bridging table). This also applies to static added
	    entries. Those violated packets could be accepted or discarded,
	    depending on the global switch configuration 'bMAC_SpoofingAction'.
	    This parameter is only taken into account when 'bLearning' is enabled. */
	gsw_bool_t	bMAC_SpoofingDetection;
	/** Port Flow Control Status. Enables the flow control function. */
	GSW_portFlow_t	eFlowCtrl;
	/** Port monitor feature. Allows forwarding of egress and/or ingress
	    packets to the monitor port. If enabled, the monitor port gets
	    a copy of the selected packet type. */
	GSW_portMonitor_t	ePortMonitor;
	/** Assign Interface RMON Counters for this Port - GSWIP-3.0 */
	gsw_bool_t	bIfCounters;
	/** Interface RMON Counters Start Index - GSWIP-3.0.
	    Value of (-1) denotes unassigned Interface Counters.
	    Valid range : 0-255 available to be shared amongst ports in desired way*/
	int nIfCountStartIdx;
	/** Interface RMON Counters Mode - GSWIP-3.0 */
	GSW_If_RMON_Mode_t	eIfRMONmode;
} GSW_portCfg_t;

/** @}*/ /* GSW_ETHERNET_BRIDGING */

/** \addtogroup GSW_QoS_SVC
 *  @{
 */

/** \brief Meter Type - srTCM or trTCM. Defines the Metering algorithm Type.
    Used by \ref GSW_QoS_meterCfg_t. */
typedef enum {
	/** srTCM Meter Type - single rate 3 color mode */
	GSW_QOS_Meter_srTCM	= 0,
	/** trTCM Meter Type - 2 rate 3 color mode */
	GSW_QOS_Meter_trTCM	= 1,
} GSW_QoS_Meter_Type;

/** \brief DSCP Drop Precedence to color code assignment.
    Used by \ref GSW_QoS_DSCP_DropPrecedenceCfg_t. */
typedef enum {
	/** Critical Packet. Metering never changes the drop precedence of these packets. */
	GSW_DROP_PRECEDENCE_CRITICAL           = 0,
	/** Green Drop Precedence Packet. Packet is marked with a 'low' drop precedence. */
	GSW_DROP_PRECEDENCE_GREEN = 1,
	/** Yellow Drop Precedence Packet. Packet is marked with a 'middle' drop precedence. */
	GSW_DROP_PRECEDENCE_YELLOW	= 2,
	/** Red Drop Precedence Packet. Packet is marked with a 'high' drop precedence. */
	GSW_DROP_PRECEDENCE_RED = 3
} GSW_QoS_DropPrecedence_t;


/** \brief Selection of the traffic class field.
    Used by \ref GSW_QoS_portCfg_t.
    The port default traffic class is assigned in case non of the
    configured protocol code points given by the packet. */
typedef enum {
	/** No traffic class assignment based on DSCP or PCP */
	GSW_QOS_CLASS_SELECT_NO = 0,
	/** Traffic class assignment based on DSCP. PCP information is ignored.
	    The Port Class is used in case DSCP is not available in the packet. */
	GSW_QOS_CLASS_SELECT_DSCP = 1,
	/** Traffic class assignment based on PCP. DSCP information is ignored.
	    The Port Class is used in case PCP is not available in the packet. */
	GSW_QOS_CLASS_SELECT_PCP	= 2,
	/** Traffic class assignment based on DSCP. Make the assignment based on
	    PCP in case the DSCP information is not available in the packet header.
	    The Port Class is used in case both are not available in the packet. */
	GSW_QOS_CLASS_SELECT_DSCP_PCP          = 3,
	/** CTAG VLAN PCP, IP DSCP. Traffic class assignment based
	    on CTAG VLAN PCP, alternative use DSCP based assignment. */
	GSW_QOS_CLASS_SELECT_PCP_DSCP          = 4,
	/** STAG VLAN PCP. Traffic class assignment based
	    on STAG VLAN PCP. */
	GSW_QOS_CLASS_SELECT_SPCP	= 5,
	/** STAG VLAN PCP, IP DSCP. Traffic class assignment based
	    on STAG VLAN PCP, alternative use DSCP based assignment. */
	GSW_QOS_CLASS_SELECT_SPCP_DSCP         = 6,
	/** IP DSCP, STAG VLAN PCP. Traffic class assignment based
	    on DSCP, alternative use STAG VLAN PCP based assignment. */
	GSW_QOS_CLASS_SELECT_DSCP_SPCP         = 7,
	/** STAG VLAN PCP, CTAG VLAN PCP. Traffic class assignment based
	    on STAG VLAN PCP, alternative use CTAG VLAN PCP based assignment. */
	GSW_QOS_CLASS_SELECT_SPCP_PCP          = 8,
	/** STAG VLAN PCP, CTAG VLAN PCP, IP DSCP. Traffic class assignment
	    based on STAG VLAN PCP, alternative use CTAG VLAN PCP based
	    assignment, alternative use DSCP based assignment. */
	GSW_QOS_CLASS_SELECT_SPCP_PCP_DSCP     = 9,
	/** IP DSCP, STAG VLAN PCP, CTAG VLAN PCP. Traffic class assignment
	    based on DSCP, alternative use STAG VLAN PCP based
	    assignment, alternative use CTAG VLAN PCP based assignment. */
	GSW_QOS_CLASS_SELECT_DSCP_SPCP_PCP     = 10
} GSW_QoS_ClassSelect_t;


/** \brief Configures the parameters of a rate meter instance.
    Used by \ref GSW_QOS_MeterAlloc, \ref GSW_QOS_MeterFree,
    \ref GSW_QoS_MeterCfgSet and \ref GSW_QoS_MeterCfgGet. */
typedef struct {
	/** Enable/Disable the meter shaper. */
	gsw_bool_t	bEnable;
	/** Meter index (zero-based counting).

	    \remarks
	    For \ref GSW_QOS_MeterFree, this is the only input and other fields are
	    ignored. For \ref GSW_QOS_MeterAlloc, this is output when allocation
	    is successful. For \ref GSW_QoS_MeterCfgSet and
	    \ref GSW_QoS_MeterCfgGet, this is input to indicate meter to
	    configure/get-configuration. */
	u16	nMeterId;
	/** Meter Name string for easy reference (Id to Name Mapping) - TBD*/
	char	cMeterName[32];
	/** Meter Algorithm Type */
	GSW_QoS_Meter_Type eMtrType;
	/** Committed Burst Size (CBS [Bytes])

	    \remarks
	    Range 64B~4GB. Only most 10 significant bits are effective.
	    Value is rounded up to HW boundary.
	    If value is less than 64, default value 32KB is used. */
	u32	nCbs;
	/** reserve for backward compatibility */
	u32	res1;
	/** Excess Burst Size (EBS [Bytes]).

	    \remarks
	    Range 64B~4GB. Only most 10 significant bits are effective.
	    Value is rounded up to HW boundary.
	    If value is less than 64, default value 32KB is used. */
	u32	nEbs;
	/** reserve for backward compatibility */
	u32	res2;
	/** Committed Information Rate (CIR)

	    \remarks
	    CIR in [kbit/s] if \ref GSW_QoS_meterCfg_t::bPktMode is FALSE,
	    or in [packet/s] if \ref GSW_QoS_meterCfg_t::bPktMode is TRUE. */
	u32	nRate;
	/** Peak Information Rate (PIR) - applicable for trTCM only

	    \remarks
	    PIR in [kbit/s] if \ref GSW_QoS_meterCfg_t::bPktMode is FALSE,
	    or in [packet/s] if \ref GSW_QoS_meterCfg_t::bPktMode is TRUE. */
	u32	nPiRate;
	/** Peak Burst Size (PBS [Bytes]) - applicable for trTCM only */
	/* u32	nPbs; */
	/** Meter colour mode **/
	u8 nColourBlindMode;
	/** Enable/Disable Packet Mode. 0- Byte, 1 - Pkt */
	u8 bPktMode;
	/** Enable/Disable local overhead for metering rate calculation. */
	gsw_bool_t bLocalOverhd;
	/** Local overhead for metering rate calculation when
	    \ref GSW_QoS_meterCfg_t::bLocalOverhd is TRUE. */
	u16 nLocaloverhd;
} GSW_QoS_meterCfg_t;

/** \brief DSCP mapping table.
    Used by \ref GSW_QoS_DSCP_ClassSet
    and \ref GSW_QoS_DSCP_ClassGet. */
typedef struct {
	/** Traffic class associated with a particular DSCP value.
	    DSCP is the index to an array of resulting traffic class values.
	    The index starts counting from zero. */
	u8	nTrafficClass[64];
} GSW_QoS_DSCP_ClassCfg_t;

/** \brief DSCP to Drop Precedence assignment table configuration.
    Used by \ref GSW_QoS_DSCP_DropPrecedenceCfgSet
    and \ref GSW_QoS_DSCP_DropPrecedenceCfgGet. */
typedef struct {
	/** DSCP to drop precedence assignment. Every array entry represents the
	    drop precedence for one of the 64 existing DSCP values.
	    DSCP is the index to an array of resulting drop precedence values.
	    The index starts counting from zero.
	    Value refers to \ref GSW_QoS_DropPrecedence_t. */
	u8 nDSCP_DropPrecedence[64];
} GSW_QoS_DSCP_DropPrecedenceCfg_t;

/** \brief Ingress DSCP remarking attribute. This attribute defines on the
    ingress port packets how these will be remarked on the egress port.
    A packet is only remarked in case its ingress and its egress port
    have remarking enabled.
    Used by \ref GSW_QoS_portRemarkingCfg_t. */
typedef enum {
	/** No DSCP Remarking. No remarking is done on the egress port. */
	GSW_DSCP_REMARK_DISABLE = 0,
	/** TC DSCP 6-Bit Remarking. The complete DSCP remarking is done based
	    on the traffic class. The traffic class to DSCP value mapping is
	    given in a device global table. */
	GSW_DSCP_REMARK_TC6 = 1,
	/** TC DSCP 3-Bit Remarking. The upper 3-Bits of the DSCP field are
	    remarked based on the traffic class. The traffic class to DSCP value
	    mapping is given in a device global table. */
	GSW_DSCP_REMARK_TC3 = 2,
	/** Drop Precedence Remarking. The Drop Precedence is remarked on the
	    egress side. */
	GSW_DSCP_REMARK_DP3 = 3,
	/** TC Drop Precedence Remarking. The Drop Precedence is remarked on the
	    egress side and the upper 3-Bits of the DSCP field are
	    remarked based on the traffic class. The traffic class to DSCP value
	    mapping is given in a device global table. */
	GSW_DSCP_REMARK_DP3_TC3 = 4
} GSW_Qos_ingressRemarking_t;

/** \brief Port Remarking Configuration. Ingress and Egress remarking options for
    dedicated packet fields DSCP, CTAG VLAN PCP, STAG VLAN PCP
    and STAG VLAN DEI.
    Remarking is done either on the used traffic class or the
    drop precedence.
    Packet field specific remarking only applies on a packet if
    enabled on ingress and egress port.
    Used by \ref GSW_QoS_PortRemarkingCfgSet
    and \ref GSW_QoS_PortRemarkingCfgGet. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. */
	u16 nPortId;
	/** Ingress DSCP Remarking. Specifies on ingress side how a packet should
	    be remarked. This DSCP remarking only works in case remarking is
	    enabled on the egress port.
	    This configuration requires that remarking is also enabled on the
	    egress port. DSCP remarking enable on either ingress or egress port
	    side does not perform any remark operation. */
	GSW_Qos_ingressRemarking_t	eDSCP_IngressRemarkingEnable;
	/** Egress DSCP Remarking. Applies remarking on egress packets in a
	    fashion as specified on the ingress port. This ingress port remarking
	    is configured by the parameter 'eDSCP_IngressRemarking'.
	    This configuration requires that remarking is also enabled on the
	    ingress port. DSCP remarking enable on either ingress or egress port
	    side does not perform any remark operation. */
	gsw_bool_t bDSCP_EgressRemarkingEnable;
	/** Ingress PCP Remarking. Applies remarking to all port ingress packets.
	    This configuration requires that remarking is also enabled on the
	    egress port. PCP remarking enable on either ingress or egress port
	    side does not perform any remark operation. */
	gsw_bool_t bPCP_IngressRemarkingEnable;
	/** Egress PCP Remarking. Applies remarking for all port egress packets.
	    This configuration requires that remarking is also enabled on the
	    ingress port. PCP remarking enable on either ingress or egress port
	    side does not perform any remark operation. */
	gsw_bool_t bPCP_EgressRemarkingEnable;
	/** Ingress STAG VLAN PCP Remarking */
	gsw_bool_t bSTAG_PCP_IngressRemarkingEnable;
	/** Ingress STAG VLAN DEI Remarking */
	gsw_bool_t bSTAG_DEI_IngressRemarkingEnable;
	/** Egress STAG VLAN PCP & DEI Remarking */
	gsw_bool_t bSTAG_PCP_DEI_EgressRemarkingEnable;
} GSW_QoS_portRemarkingCfg_t;

/** \brief Describes which priority information of ingress packets is used
    (taken into account) to identify the packet priority and the related egress
    priority queue. For DSCP, the priority to queue assignment is done
    using \ref GSW_QoS_DSCP_ClassSet. For VLAN, the priority to queue
    assignment is done using \ref GSW_QoS_PCP_ClassSet.
    Used by \ref GSW_QoS_PortCfgSet and \ref GSW_QoS_PortCfgGet. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. */
	u16	nPortId;
	/** Select the packet header field on which to base the traffic class assignment. */
	GSW_QoS_ClassSelect_t	eClassMode;
	/** Default port priority in case no other priority
	    (such as VLAN-based PCP or IP-based DSCP) is used. */
	u8	nTrafficClass;
} GSW_QoS_portCfg_t;

/** \brief Traffic class associated with a particular 802.1P (PCP) priority mapping value.
    This table is global for the entire switch device. Priority map entry structure.
    Used by \ref GSW_QoS_PCP_ClassSet
    and \ref GSW_QoS_PCP_ClassGet. */
typedef struct {
	/** Configures the traffic class to PCP (3-bit) mapping.
	    The queue index starts counting from zero. */
	u8 nPCP[16];
} GSW_QoS_ClassPCP_Cfg_t;


/** \brief Traffic class associated with a particular 802.1P (PCP) priority mapping value.
    This table is global for the entire switch device. Priority map entry structure.
    Used by \ref GSW_QoS_PCP_ClassSet
    and \ref GSW_QoS_PCP_ClassGet. */
typedef struct {
	/** Configures the PCP to traffic class mapping.
	    The queue index starts counting from zero. */
	u8	nTrafficClass[16];
} GSW_QoS_PCP_ClassCfg_t;

/** \brief Configure queue specific parameter.
    Used by \ref GSW_QoS_QueueCfgSet and \ref GSW_QoS_QueueCfgGet. */
typedef struct {
	/** QoS queue index (zero-based counting). */
	u8 nQueueId;
	/** Enable/disable this queue. */
	gsw_bool_t bEnable;
	/** Redirect traffic forward port. */
	u8 nPortId;
} GSW_QoS_queueCfg_t;

/** \brief Describes the QoS Queue Mapping Mode. GSWIP-3.1 only.
    Used by \ref GSW_QoS_queuePort_t. */
typedef enum {
	/** This is default mode where the QID is fixed at
	    \ref GSW_QoS_QueuePortSet. */
	GSW_QOS_QMAP_SINGLE_MODE = 0,
	/** This is new mode in GSWIP-3.1. The QID given in
	    \ref GSW_QoS_QueuePortSet is base, and bit 0~3 of sub-interface ID
	    is offset. The final QID is base + SubIfId[0:3]. */
	GSW_QOS_QMAP_SUBIFID_MODE = 1
} GSW_QoS_qMapMode_t;

/** \brief Sets the Queue ID for one traffic class of one port.
    Used by \ref GSW_QoS_QueuePortSet and \ref GSW_QoS_QueuePortGet. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available.
	    This is an input parameter for \ref GSW_QoS_QueuePortGet. */
	u16 nPortId;
	/** Forward CPU (extraction) before external QoS queueing (DownMEP).
	    GSWIP-3.1 only. */
	gsw_bool_t bExtrationEnable;
	/** When \ref GSW_QoS_queuePort_t::bExtrationEnable is FALSE, this field
	    defines Queue Mapping Mode. GSWIP-3.1 only. */
	GSW_QoS_qMapMode_t eQMapMode;
	/** Traffic Class index (zero-based counting).
	    This is an input parameter for \ref GSW_QoS_QueuePortGet. */
	u8 nTrafficClassId;
	/** QoS queue index (zero-based counting).
	    This is an output parameter for \ref GSW_QoS_QueuePortGet. */
	u8 nQueueId;
	/** Queue Redirection bypass Option.
	    If enabled, all packets destined to 'nQueueId' are redirected from the
	    'nPortId' to 'nRedirectPortId'. This is used for 2nd stage of FULL QoS
	    Path, where the packet has completed QoS process at CBM/CQEM and been
	    injected into GSWIP again. */
	gsw_bool_t bRedirectionBypass;
	/** Redirected traffic forward port.
	    All egress packets to 'nPortId' are redirected to "nRedirectPortId".
	    If there is no redirection required, it should be same as "nPortId". */
	u8 nRedirectPortId;

	/** To enable Ingress PCE Bypass. Applicable for GSWIP 3.2 and above.
	    For \ref GSW_QoS_QueuePortGet, set TRUE as input to check whether
	    Ingress PCE Bypass is enabled, and this field is updated as output.
	    For \ref GSW_QoS_QueuePortSet, set FALSE to configure normal
	    path first, then set TRUE to configure Ingress PCE Bypass path
	    (only if application requires). */
	gsw_bool_t bEnableIngressPceBypass;
	/** Internal purpose only - user not allowed to use it.
	    Applicable for GSWIP 3.2 and above. */
	gsw_bool_t bReservedPortMode;
} GSW_QoS_queuePort_t;

/** \brief Select the type of the Egress Queue Scheduler.
    Used by \ref GSW_QoS_schedulerCfg_t. */
typedef enum {
	/** Strict Priority Scheduler (strict high). */
	GSW_QOS_SCHEDULER_STRICT = 0,
	/** Strict Priority Scheduler (strict high).
	    Same as \ref GSW_QOS_SCHEDULER_STRICT. */
	GSW_QOS_SCHEDULER_STRICT_HIGH = GSW_QOS_SCHEDULER_STRICT,
	/** Weighted Fair Queuing Shceduler. */
	GSW_QOS_SCHEDULER_WFQ = 1,
	/** Strict Priority Scheduler (strict low). */
	GSW_QOS_SCHEDULER_STRICT_LOW = 2,
} GSW_QoS_Scheduler_t;

/** \brief Configures the egress queues attached to a single port, and that
    are scheduled to transmit the queued Ethernet packets.
    Used by \ref GSW_QoS_SchedulerCfgSet and \ref GSW_QoS_SchedulerCfgGet. */
typedef struct {
	/** QoS queue index (zero-based counting). */
	u8 nQueueId;
	/** Scheduler Type (Strict Priority/Weighted Fair Queuing).
	    Refers to \ref GSW_QoS_Scheduler_t for detail values. */
	u8 eType;
	/** Weight in Token. Parameter used for WFQ configuration.
	    Sets the weight in token in relation to all remaining
	    queues on this egress port having WFQ configuration.
	    This parameter is only used when 'eType=GSW_QOS_SCHEDULER_WFQ'. */
	u16 nWeight;
} GSW_QoS_schedulerCfg_t;

/** \brief Configures a rate shaper instance with the rate and the burst size.
    Used by \ref GSW_QoS_ShaperCfgSet and \ref GSW_QoS_ShaperCfgGet. */
typedef struct {
	/** Rate shaper index (zero-based counting). */
	u8	nRateShaperId;
	/** Enable/Disable the rate shaper. */
	gsw_bool_t	bEnable;
	/** 802.1Qav credit based shaper mode. This specific shaper
	    algorithm mode is used by the audio/video bridging (AVB)
	    network (according to 802.1Qav). By default, an token
	    based shaper algorithm is used. */
	gsw_bool_t	bAVB;
	/** Committed Burst Size (CBS [bytes])

	    \remarks
	    Range 64B~4GB. Only most 10 significant bits are effective.
	    Value is rounded up to HW boundary.
	    If value is less than 64, default value 32KB is used. */
	u32	nCbs;
	/** Rate [kbit/s] */
	u32	nRate;
} GSW_QoS_ShaperCfg_t;

/** \brief Assign one rate shaper instance to a QoS queue.
    Used by \ref GSW_QoS_ShaperQueueAssign and \ref GSW_QoS_ShaperQueueDeassign. */
typedef struct {
	/** Rate shaper index (zero-based counting). */
	u8	nRateShaperId;
	/** QoS queue index (zero-based counting). */
	u8	nQueueId;
} GSW_QoS_ShaperQueue_t;

/** \brief Retrieve if a rate shaper instance is assigned to a QoS egress queue.
    Used by \ref GSW_QoS_ShaperQueueGet. */
typedef struct {
	/** QoS queue index (zero-based counting).
	    This parameter is the input parameter for the GET function. */
	u8	nQueueId;
	/** Shaper instances (max 2) associated with queue. */
	struct {
		/** Rate shaper instance assigned.
		    If 1, a rate shaper instance is assigned to the queue. Otherwise no shaper instance is assigned. */
		gsw_bool_t	bAssigned;
		/** Rate shaper index (zero-based counting). Only a valid instance is returned in case 'bAssigned == 1'. */
		u8	nRateShaperId;
	} sShaper[2];
} GSW_QoS_ShaperQueueGet_t;

/** \brief Assigns one meter instances for storm control.
    Used by \ref GSW_QoS_StormCfgSet and \ref GSW_QoS_StormCfgGet.
    Not applicable to GSWIP-3.1. */
typedef struct {
	/** Meter index 0 (zero-based counting). */
	u16	nMeterId;
	/** Meter instances used for broadcast traffic. */
	gsw_bool_t	bBroadcast;
	/** Meter instances used for multicast traffic. */
	gsw_bool_t	bMulticast;
	/** Meter instances used for unknown unicast traffic. */
	gsw_bool_t	bUnknownUnicast;
} GSW_QoS_stormCfg_t;

/** \brief Egress Queue Congestion Notification Watermark.
    Used by \ref GSW_QoS_WRED_Cfg_t. */
typedef enum {
	/**
	>= 1/4 of green max water mark assert
	<= 1/4 of green max water mark de assert*/
	GSW_QOS_WRED_WATERMARK_1_4	= 0,
	/**
	>= 1/8 of green max water mark assert
	<= 1/8 of green max water mark de assert*/
	GSW_QOS_WRED_WATERMARK_1_8	= 1,
	/**
	>= 1/12 of green max water mark assert
	<= 1/12 of green max water mark de assert*/
	GSW_QOS_WRED_WATERMARK_1_12	= 2,
	/**
	>= 1/16 of green max water mark assert
	<= 1/16 of green max water mark de assert*/
	GSW_QOS_WRED_WATERMARK_1_16	= 3
} GSW_QoS_WRED_WATERMARK_t;

/** \brief Drop Probability Profile. Defines the drop probability profile.
    Used by \ref GSW_QoS_WRED_Cfg_t. */
typedef enum {
	/** Pmin = 25%, Pmax = 75% (default) */
	GSW_QOS_WRED_PROFILE_P0	= 0,
	/** Pmin = 25%, Pmax = 50% */
	GSW_QOS_WRED_PROFILE_P1	= 1,
	/** Pmin = 50%, Pmax = 50% */
	GSW_QOS_WRED_PROFILE_P2	= 2,
	/** Pmin = 50%, Pmax = 75% */
	GSW_QOS_WRED_PROFILE_P3	= 3
} GSW_QoS_WRED_Profile_t;

/** \brief WRED Cfg Type - Automatic (Adaptive) or Manual.
    Used by \ref GSW_QoS_WRED_Cfg_t. */
typedef enum {
	/** Automatic - Adaptive Watermark Type - GSWIP-3.0/3.1 only*/
	GSW_QOS_WRED_Adaptive	= 0,
	/** Manual Threshold Levels Type */
	GSW_QOS_WRED_Manual	= 1
} GSW_QoS_WRED_Mode_t;

/** \brief WRED Thresholds Mode Type. - GSWIP-3.0/3.1 only
    Used by \ref GSW_QoS_WRED_Cfg_t. */
typedef enum {
	/** Local Thresholds Mode */
	GSW_QOS_WRED_Local_Thresh	= 0,
	/** Global Thresholds Mode */
	GSW_QOS_WRED_Global_Thresh	= 1,
	/** Port queue and Port WRED Thresholds */
	GSW_QOS_WRED_Port_Thresh	= 2,

} GSW_QoS_WRED_ThreshMode_t;

/** \brief Configures the global probability profile of the device.
    The min. and max. threshold values are given in number of packet
    buffer segments and required only in case of Manual Mode.
    The GSWIP-3.0/3.1 supports Auto mode and the threshold values are
    dynamically computed internally by GSWIP.
    The size of a segment can be retrieved using \ref GSW_CapGet.
    Used by \ref GSW_QoS_WredCfgSet and \ref GSW_QoS_WredCfgGet. */
typedef struct {
	/** Egress Queue Congestion Notification Watermark
	   only applicable for GSWIP 3.1*/
	GSW_QoS_WRED_WATERMARK_t eCongestionWatermark;
	/** Drop Probability Profile. */
	GSW_QoS_WRED_Profile_t	eProfile;
	/** Automatic or Manual Mode of Thresholds Config */
	GSW_QoS_WRED_Mode_t eMode;
	/** WRED Threshold Mode Config */
	GSW_QoS_WRED_ThreshMode_t eThreshMode;
	/** WRED Red Threshold Min [number of segments] - Valid for Manual Mode only. */
	u16	nRed_Min;
	/** WRED Red Threshold Max [number of segments] - Valid for Manual Mode only */
	u16	nRed_Max;
	/** WRED Yellow Threshold Min [number of segments] - Valid for Manual Mode only */
	u16	nYellow_Min;
	/** WRED Yellow Threshold Max [number of segments] - Valid for Manual Mode only */
	u16	nYellow_Max;
	/** WRED Green Threshold Min [number of segments] - Valid for Manual Mode only */
	u16	nGreen_Min;
	/** WRED Green Threshold Max [number of segments] - Valid for Manual Mode only */
	u16	nGreen_Max;
} GSW_QoS_WRED_Cfg_t;

/** \brief Configures the WRED threshold level values.
    The min. and max. values are given in number of packet
    buffer segments. The size of a segment can be retrieved using \ref GSW_CapGet.
    Used by \ref GSW_QoS_WredQueueCfgSet and \ref GSW_QoS_WredQueueCfgGet. */
typedef struct {
	/** QoS queue index (zero-based counting). */
	u16	nQueueId;
	/** WRED Red Threshold Min [number of segments]. */
	u16	nRed_Min;
	/** WRED Red Threshold Max [number of segments]. */
	u16	nRed_Max;
	/** WRED Yellow Threshold Min [number of segments]. */
	u16	nYellow_Min;
	/** WRED Yellow Threshold Max [number of segments]. */
	u16	nYellow_Max;
	/** WRED Green Threshold Min [number of segments]. */
	u16	nGreen_Min;
	/** WRED Green Threshold Max [number of segments]. */
	u16	nGreen_Max;
	/** Reserved Buffer Threshold */
	u16	nReserveThreshold;
} GSW_QoS_WRED_QueueCfg_t;

/** \brief Configures the WRED threshold parameter per port.
    The configured thresholds apply to fill level sum
    of all egress queues which are assigned to the egress port.
    The min. and max. values are given in number of packet
    buffer segments. The size of a segment can be retrieved using \ref GSW_CapGet.
    Used by \ref GSW_QoS_WredPortCfgSet and \ref GSW_QoS_WredPortCfgGet. */
typedef struct {
	/** Ethernet Port number (zero-based counting).
	    The valid range is hardware dependent. */
	u16	nPortId;
	/** WRED Red Threshold Min [number of segments]. */
	u16	nRed_Min;
	/** WRED Red Threshold Max [number of segments]. */
	u16	nRed_Max;
	/** WRED Yellow Threshold Min [number of segments]. */
	u16	nYellow_Min;
	/** WRED Yellow Threshold Max [number of segments]. */
	u16	nYellow_Max;
	/** WRED Green Threshold Min [number of segments]. */
	u16	nGreen_Min;
	/** WRED Green Threshold Max [number of segments]. */
	u16	nGreen_Max;
} GSW_QoS_WRED_PortCfg_t;

/** \brief Configures the global buffer flow control threshold for
    conforming and non-conforming packets.
    The min. and max. values are given in number of packet
    buffer segments. The size of a segment can be retrieved using \ref GSW_CapGet.
    Used by \ref GSW_QoS_FlowctrlCfgSet and \ref GSW_QoS_FlowctrlCfgGet. */
typedef struct {
	/** Global Buffer Non Conforming Flow Control Threshold Minimum [number of segments]. */
	u16	nFlowCtrlNonConform_Min;
	/** Global Buffer Non Conforming Flow Control Threshold Maximum [number of segments]. */
	u16	nFlowCtrlNonConform_Max;
	/** Global Buffer Conforming Flow Control Threshold Minimum [number of segments]. */
	u16	nFlowCtrlConform_Min;
	/** Global Buffer Conforming Flow Control Threshold Maximum [number of segments]. */
	u16	nFlowCtrlConform_Max;
} GSW_QoS_FlowCtrlCfg_t;

/** \brief Configures the ingress port flow control threshold for
    used packet segments.
    The min. and max. values are given in number of packet
    buffer segments. The size of a segment can be retrieved using \ref GSW_CapGet.
    Used by \ref GSW_QoS_FlowctrlPortCfgSet and \ref GSW_QoS_FlowctrlPortCfgGet. */
typedef struct {
	/** Ethernet Port number (zero-based counting).
	    The valid range is hardware dependent. */
	u16	nPortId;
	/** Ingress Port occupied Buffer Flow Control Threshold Minimum [number of segments]. */
	u16	nFlowCtrl_Min;
	/** Ingress Port occupied Buffer Flow Control Threshold Maximum [number of segments]. */
	u16	nFlowCtrl_Max;
} GSW_QoS_FlowCtrlPortCfg_t;

/** \brief Reserved egress queue buffer segments.
    Used by \ref GSW_QoS_QueueBufferReserveCfgSet and \ref GSW_QoS_QueueBufferReserveCfgGet. */
typedef struct {
	/** QoS queue index (zero-based counting).
	    This is an input parameter for \ref GSW_QoS_QueueBufferReserveCfgGet. */
	u16	nQueueId;
	/** Reserved Buffer Segment Threshold [number of segments].
	    This is an output parameter for \ref GSW_QoS_QueueBufferReserveCfgGet. */
	u16	nBufferReserved;
} GSW_QoS_QueueBufferReserveCfg_t;

/** \brief Color Marking Table.
    There are standards to define the marking table. User should use
    \ref GSW_QOS_ColorMarkingTableSet to initialize the table before color
    marking happens. \ref GSW_QOS_ColorMarkingTableGet is used to get
    the marking table, mainly for debug purpose. */
typedef struct {
	/** Mode of color marking. */
	GSW_ColorMarkingMode_t eMode;

	/** If eMode is GSW_REMARKING_DSCP_AF, index stands for 6-bit DSCP value.
	    If eMode is one of GSW_REMARKING_PCP_8P0D, GSW_REMARKING_PCP_7P1D,
	    GSW_REMARKING_PCP_6P2D and GSW_REMARKING_PCP_5P3D, index 0-7 is
	    3-bit PCP value with DEI is 0, and index 8-15 is 3-bit PCP value with
	    DEI is 1. Ignored in other modes. */
	u8 nPriority[64];
	/** If eMode is GSW_REMARKING_DSCP_AF, index stands for 6-bit DSCP value.
	    If eMode is one of GSW_REMARKING_PCP_8P0D, GSW_REMARKING_PCP_7P1D,
	    GSW_REMARKING_PCP_6P2D and GSW_REMARKING_PCP_5P3D, index 0-7 is 3-bit
	    PCP value with DEI is 0, and index 8-15 is 3-bit PCP value with DEI is 1.
	    Ignored in other modes.
	    Value refers to \ref GSW_QoS_DropPrecedence_t. */
	u8 nColor[64];
} GSW_QoS_colorMarkingEntry_t;

/** \brief Color Remarking Table.
    There are standards to define the remarking table. User should use
    \ref GSW_QOS_ColorReMarkingTableSet to initialize the table before color
    remarking happens. \ref GSW_QOS_ColorReMarkingTableGet is used to get
    the remarking table, mainly for debug purpose. */
typedef struct {
	/** Mode of color remarking. */
	GSW_ColorRemarkingMode_t eMode;

	/** Index stands for color and priority. Index 0-7 is green color with
	    priority (traffic class) 0-7. Index 8-15 is yellow color with priority
	    (traffic class) 0-7. Value is DSCP if eMode is GSW_REMARKING_DSCP_AF.
	    Value bit 0 is DEI and bit 1-3 is PCP if eMode is one of
	    GSW_REMARKING_PCP_8P0D, GSW_REMARKING_PCP_7P1D, GSW_REMARKING_PCP_6P2D
	    and GSW_REMARKING_PCP_5P3D. Value is ignored for other mode. */
	u8 nVal[16];
} GSW_QoS_colorRemarkingEntry_t;

/** \brief DSCP to PCP Mapping.
    Used by \ref GSW_QOS_Dscp2PcpTableGet. */
typedef struct {
	/** Index of entry in mapping table. */
	u16 nIndex;

	/** The index of array stands for DSCP value. Each byte of the array is 3-bit
	    PCP value. */
	u8 nMap[64];
} GSW_DSCP2PCP_map_t;

/** \brief Traffic class associated with a particular STAG VLAN 802.1P (PCP) priority and Drop Eligible Indicator (DEI) mapping value.
    This table is global for the entire switch device. Priority map entry structure.
    The table index value is calculated by 'index=PCP + 8*DEI'
    Used by \ref GSW_QoS_SVLAN_PCP_ClassSet and \ref GSW_QoS_SVLAN_PCP_ClassGet. */
typedef struct {
	/** Configures the PCP and DEI to traffic class mapping.
	    The queue index starts counting from zero. */
	u8	nTrafficClass[16];
	/**  Configures the PCP traffic color.
	     Not applicable to GSWIP-3.1. */
	u8	nTrafficColor[16];
	/** PCP Remark disable control.
	     Not applicable to GSWIP-3.1. */
	u8	nPCP_Remark_Enable[16];
	/** DEI Remark disable control.
	    Not applicable to GSWIP-3.1. */
	u8	nDEI_Remark_Enable[16];

} GSW_QoS_SVLAN_PCP_ClassCfg_t;

/** @}*/ /* GSW_QoS_SVC */


/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/** \brief Enumeration used for Switch capability types. GSWIP-3.0 only capabilities are explicitly indicated.
    Used by \ref GSW_cap_t. */
typedef enum {
	/** Number of physical Ethernet ports. */
	GSW_CAP_TYPE_PORT = 0,
	/** Number of virtual Ethernet ports. */
	GSW_CAP_TYPE_VIRTUAL_PORT = 1,
	/** Size of internal packet memory [in Bytes]. */
	GSW_CAP_TYPE_BUFFER_SIZE = 2,
	/** Buffer segment size.
	    Byte size of a segment, used to store received packet data. */
	GSW_CAP_TYPE_SEGMENT_SIZE = 3,
	/** Number of priority queues per device. */
	GSW_CAP_TYPE_PRIORITY_QUEUE = 4,
	/** Number of meter instances. */
	GSW_CAP_TYPE_METER	= 5,
	/** Number of rate shaper instances. */
	GSW_CAP_TYPE_RATE_SHAPER	= 6,
	/** Number of CTAG VLAN groups that can be configured on the switch hardware. */
	GSW_CAP_TYPE_VLAN_GROUP	= 7,
	/** Number of Filtering Identifiers (FIDs) */
	GSW_CAP_TYPE_FID	= 8,
	/** Number of MAC Bridging table entries */
	GSW_CAP_TYPE_MAC_TABLE_SIZE	= 9,
	/** Number of multicast level 3 hardware table entries */
	GSW_CAP_TYPE_MULTICAST_TABLE_SIZE      = 10,
	/** Number of supported PPPoE sessions. */
	GSW_CAP_TYPE_PPPOE_SESSION	= 11,
	/** Number of STAG VLAN groups that can be configured on the switch hardware. */
	GSW_CAP_TYPE_SVLAN_GROUP	= 12,
	/** Number of PMAC Supported in Switch Macro - for GSWIP-3.0 only. */
	GSW_CAP_TYPE_PMAC	= 13,
	/** Number of entries in Payload Table size - for GSWIP-3.0 only. */
	GSW_CAP_TYPE_PAYLOAD	= 14,
	/** Number of RMON Counters Supported - for GSWIP-3.0 only. */
	GSW_CAP_TYPE_IF_RMON	= 15,
	/** Number of Egress VLAN Treatment Entries - for GSWIP-3.0 only. */
	GSW_CAP_TYPE_EGRESS_VLAN = 16,
	/** Number of Routing Source-MAC Entries - for GSWIP-R-3.0 only. */
	GSW_CAP_TYPE_RT_SMAC = 17,
	/** Number of Routing Destination-MAC Entries - for GSWIP-R-3.0 only. */
	GSW_CAP_TYPE_RT_DMAC = 18,
	/** Number of Routing-PPPoE Entries - for GSWIP-R-3.0 only. */
	GSW_CAP_TYPE_RT_PPPoE = 19,
	/** Number of Routing-NAT Entries - for GSWIP-R-3.0 only. */
	GSW_CAP_TYPE_RT_NAT = 20,
	/** Number of MTU Entries - for GSWIP-R-3.0 only. */
	GSW_CAP_TYPE_RT_MTU = 21,
	/** Number of Tunnel Entries - for GSWIP-R-3.0 only. */
	GSW_CAP_TYPE_RT_TUNNEL = 22,
	/** Number of RTP Entries - for GSWIP-R-3.0 only. */
	GSW_CAP_TYPE_RT_RTP = 23,
	/** Number of CTP ports - for GSWIP-3.1 only. */
	GSW_CAP_TYPE_CTP = 24,
	/** Number of bridge ports - for GSWIP-3.1 only. */
	GSW_CAP_TYPE_BRIDGE_PORT = 25,
	/** Number of COMMON PCE Rules. */
	GSW_CAP_TYPE_COMMON_PCE_RULES = 26,
	/** 3.2 Revision (A0 or B0)          */
	GSW_CAP_TYPE_32_VERSION = 27,
	/** Last Capability Index */
	GSW_CAP_TYPE_LAST	= 28
} GSW_capType_t;

/** \brief Maximum String Length for the Capability String. */
#define GSW_CAP_STRING_LEN	128

/** \brief Capability structure.
    Used by \ref GSW_CapGet. */
typedef struct {
	/** Defines the capability type, see \ref GSW_capType_t.*/
	GSW_capType_t	nCapType;
	/** Description of the capability. */
	char cDesc[GSW_CAP_STRING_LEN];
	/** Defines if, what or how many are available. The definition of cap
	depends on the type, see captype. */
	u32 nCap;
} GSW_cap_t;

/** \brief Aging Timer Value.
    Used by \ref GSW_cfg_t. */
typedef enum {
	/** 1 second aging time */
	GSW_AGETIMER_1_SEC	= 1,
	/** 10 seconds aging time */
	GSW_AGETIMER_10_SEC	= 2,
	/** 300 seconds aging time */
	GSW_AGETIMER_300_SEC	= 3,
	/** 1 hour aging time */
	GSW_AGETIMER_1_HOUR	= 4,
	/** 24 hours aging time */
	GSW_AGETIMER_1_DAY	= 5,
	/** Custom aging time in seconds */
	GSW_AGETIMER_CUSTOM  = 6
} GSW_ageTimer_t;

/** @}*/ /* GSW_ETHERNET_BRIDGING */


/** \addtogroup GSW_VLAN
 *  @{
 */

/** \brief Extended 4 TPID selection.
    Used by \ref GSW_EXTENDEDVLAN_filter_t. */
typedef enum {
	/** TPID is FDMA_VTETYPE (0x88A8 by default) */
	GSW_EXTENDEDVLAN_TPID_VTETYPE_1 = 0,
	/** TPID is 0x8100 */
	GSW_EXTENDEDVLAN_TPID_VTETYPE_2 = 1,
	/** TPID is FDMA_VTETYPE2 (0x9100 by default) */
	GSW_EXTENDEDVLAN_TPID_VTETYPE_3 = 2,
	/** TPID is FDMA_VTETYPE3 (0x9200 by default) */
	GSW_EXTENDEDVLAN_TPID_VTETYPE_4 = 3
} GSW_ExtendedVlan_4_Tpid_Mode_t;

/** \brief Extended VLAN Filter TPID Field.
    Used by \ref GSW_EXTENDEDVLAN_filterVLAN_t. */
typedef enum {
	/** Do not filter. */
	GSW_EXTENDEDVLAN_FILTER_TPID_NO_FILTER = 0,
	/** TPID is 0x8100. */
	GSW_EXTENDEDVLAN_FILTER_TPID_8021Q = 1,
	/** TPID is global configured value. */
	GSW_EXTENDEDVLAN_FILTER_TPID_VTETYPE = 2
} GSW_ExtendedVlanFilterTpid_t;

/** \brief Extended VLAN Treatment Set TPID.
   Used by \ref GSW_EXTENDEDVLAN_treatmentVlan_t. */
typedef enum {
	/** TPID is copied from inner VLAN tag of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_INNER_TPID = 0,
	/** TPID is copied from outer VLAN tag of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_OUTER_TPID = 1,
	/** TPID is global configured value. */
	GSW_EXTENDEDVLAN_TREATMENT_VTETYPE = 2,
	/** TPID is 0x8100. */
	GSW_EXTENDEDVLAN_TREATMENT_8021Q = 3
} GSW_ExtendedVlanTreatmentTpid_t;

/** \brief Extended VLAN Filter DEI Field.
    Used by \ref GSW_EXTENDEDVLAN_filterVLAN_t. */
typedef enum {
	/** Do not filter. */
	GSW_EXTENDEDVLAN_FILTER_DEI_NO_FILTER = 0,
	/** DEI is 0. */
	GSW_EXTENDEDVLAN_FILTER_DEI_0 = 1,
	/** DEI is 1. */
	GSW_EXTENDEDVLAN_FILTER_DEI_1 = 2
} GSW_ExtendedVlanFilterDei_t;

/** \brief Extended VLAN Treatment Set DEI.
   Used by \ref GSW_EXTENDEDVLAN_treatmentVlan_t. */
typedef enum {
	/** DEI (if applicable) is copied from inner VLAN tag of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_INNER_DEI = 0,
	/** DEI (if applicable) is copied from outer VLAN tag of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_OUTER_DEI = 1,
	/** DEI is 0. */
	GSW_EXTENDEDVLAN_TREATMENT_DEI_0 = 2,
	/** DEI is 1. */
	GSW_EXTENDEDVLAN_TREATMENT_DEI_1 = 3
} GSW_ExtendedVlanTreatmentDei_t;

/** \brief Extended VLAN Filter Type.
    Used by \ref GSW_EXTENDEDVLAN_filterVLAN_t. */
typedef enum {
	/** There is tag and criteria applies. */
	GSW_EXTENDEDVLAN_FILTER_TYPE_NORMAL = 0,
	/** There is tag but no criteria. */
	GSW_EXTENDEDVLAN_FILTER_TYPE_NO_FILTER = 1,
	/** Default entry if no other rule applies. */
	GSW_EXTENDEDVLAN_FILTER_TYPE_DEFAULT = 2,
	/** There is no tag. */
	GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG = 3,
	/** Block invalid*/
	GSW_EXTENDEDVLAN_BLOCK_INVALID = 4
} GSW_ExtendedVlanFilterType_t;

/** \brief Extended VLAN Filter EtherType.
    Used by \ref GSW_EXTENDEDVLAN_filterVLAN_t. */
typedef enum {
	/** Do not filter. */
	GSW_EXTENDEDVLAN_FILTER_ETHERTYPE_NO_FILTER = 0,
	/** IPoE frame (Ethertyp is 0x0800). */
	GSW_EXTENDEDVLAN_FILTER_ETHERTYPE_IPOE = 1,
	/** PPPoE frame (Ethertyp is 0x8863 or 0x8864). */
	GSW_EXTENDEDVLAN_FILTER_ETHERTYPE_PPPOE = 2,
	/** ARP frame (Ethertyp is 0x0806). */
	GSW_EXTENDEDVLAN_FILTER_ETHERTYPE_ARP = 3,
	/** IPv6 IPoE frame (Ethertyp is 0x86DD). */
	GSW_EXTENDEDVLAN_FILTER_ETHERTYPE_IPV6IPOE = 4,
	/** EAPOL (Ethertyp is 0x888E). */
	GSW_EXTENDEDVLAN_FILTER_ETHERTYPE_EAPOL = 5,
	/** DHCPV4 (UDP DESTINATION PORT 67&68). */
	GSW_EXTENDEDVLAN_FILTER_ETHERTYPE_DHCPV4 = 6,
	/** DHCPV6 (UDP DESTINATION PORT 546&547). */
	GSW_EXTENDEDVLAN_FILTER_ETHERTYPE_DHCPV6 = 7
} GSW_ExtendedVlanFilterEthertype_t;

/** \brief Extended VLAN Treatment Set Priority.
   Used by \ref GSW_EXTENDEDVLAN_treatmentVlan_t. */
typedef enum {
	/** Set priority with given value. */
	GSW_EXTENDEDVLAN_TREATMENT_PRIORITY_VAL = 0,
	/** Prority value is copied from inner VLAN tag of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_INNER_PRORITY = 1,
	/** Prority value is copied from outer VLAN tag of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_OUTER_PRORITY = 2,
	/** Prority value is derived from DSCP field of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_DSCP = 3
} GSW_ExtendedVlanTreatmentPriority_t;

/** \brief Extended VLAN Treatment Set VID.
   Used by \ref GSW_EXTENDEDVLAN_treatmentVlan_t. */
typedef enum {
	/** Set VID with given value. */
	GSW_EXTENDEDVLAN_TREATMENT_VID_VAL = 0,
	/** VID is copied from inner VLAN tag of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_INNER_VID = 1,
	/** VID is copied from outer VLAN tag of received packet. */
	GSW_EXTENDEDVLAN_TREATMENT_OUTER_VID = 2,
} GSW_ExtendedVlanTreatmentVid_t;

/** \brief Extended VLAN Treatment Remove Tag.
    Used by \ref GSW_EXTENDEDVLAN_treatmentVlan_t. */
typedef enum {
	/** Do not remove VLAN tag. */
	GSW_EXTENDEDVLAN_TREATMENT_NOT_REMOVE_TAG = 0,
	/** Remove 1 VLAN tag following DA/SA. */
	GSW_EXTENDEDVLAN_TREATMENT_REMOVE_1_TAG = 1,
	/** Remove 2 VLAN tag following DA/SA. */
	GSW_EXTENDEDVLAN_TREATMENT_REMOVE_2_TAG = 2,
	/** Discard traffic. */
	GSW_EXTENDEDVLAN_TREATMENT_DISCARD = 3,
	/** Discard upstream traffic. */
	GSW_EXTENDEDVLAN_TREATMENT_DISCARD_UPSTREAM = GSW_EXTENDEDVLAN_TREATMENT_DISCARD,
} GSW_ExtendedVlanTreatmentRemoveTag_t;

/** \brief Extended VLAN Filter VLAN Tag.
    Used by \ref GSW_EXTENDEDVLAN_filter_t. */
typedef struct {
	/** Filter Type: normal filter, default rule, or no tag */
	GSW_ExtendedVlanFilterType_t eType;
	/** Enable priority field filtering. */
	gsw_bool_t bPriorityEnable;
	/** Filter priority value if bPriorityEnable is TRUE. */
	u32 nPriorityVal;
	/** Enable VID filtering. */
	gsw_bool_t bVidEnable;
	/** Filter VID if bVidEnable is TRUE. */
	u32 nVidVal;
	/** Mode to filter TPID of VLAN tag. */
	GSW_ExtendedVlanFilterTpid_t eTpid;
	/** Mode to filter DEI of VLAN tag. */
	GSW_ExtendedVlanFilterDei_t eDei;
} GSW_EXTENDEDVLAN_filterVLAN_t;

/** \brief Extended VLAN Treatment VLAN Tag.
    Used by \ref GSW_EXTENDEDVLAN_treatment_t. */
typedef struct {
	/** Select source of priority field of VLAN tag. */
	GSW_ExtendedVlanTreatmentPriority_t ePriorityMode;
	/** If \ref GSW_EXTENDEDVLAN_treatmentVlan_t::ePriorityMode is
	    \ref GSW_EXTENDEDVLAN_TREATMENT_PRIORITY_VAL, use this value for
	    priority field of VLAN tag. */
	u32 ePriorityVal;
	/** Select source of VID field of VLAN tag. */
	GSW_ExtendedVlanTreatmentVid_t eVidMode;
	/** If \ref GSW_EXTENDEDVLAN_treatmentVlan_t::eVidMode is
	    \ref GSW_EXTENDEDVLAN_TREATMENT_VID_VAL, use this value for VID field
	    of VLAN tag. */
	u32 eVidVal;
	/** Select source of TPID field of VLAN tag. */
	GSW_ExtendedVlanTreatmentTpid_t eTpid;
	/** Select source of DEI field of VLAN tag. */
	GSW_ExtendedVlanTreatmentDei_t eDei;
} GSW_EXTENDEDVLAN_treatmentVlan_t;


/** \brief Extended VLAN Filter.
    Used by \ref GSW_EXTENDEDVLAN_config_t. */
typedef struct {
	/** Filter on Original Packet. */
	gsw_bool_t bOriginalPacketFilterMode;
	/** 4 TPID support. */
	GSW_ExtendedVlan_4_Tpid_Mode_t eFilter_4_Tpid_Mode;
	/** Filter for outer VLAN tag. */
	GSW_EXTENDEDVLAN_filterVLAN_t sOuterVlan;
	/** Filter for inner VLAN tag. */
	GSW_EXTENDEDVLAN_filterVLAN_t sInnerVlan;
	/** Filter EtherType. */
	GSW_ExtendedVlanFilterEthertype_t eEtherType;
} GSW_EXTENDEDVLAN_filter_t;


/** \brief Extended VLAN Allocation.
    Used by \ref GSW_ExtendedVlanAlloc and \ref GSW_ExtendedVlanFree. */
typedef struct {
	/** Total number of extended VLAN entries are requested. Proper value should
	    be given for \ref GSW_ExtendedVlanAlloc. This field is ignored for
	    \ref GSW_ExtendedVlanFree. */
	u16 nNumberOfEntries;

	/** If \ref GSW_ExtendedVlanAlloc is successful, a valid ID will be returned
	    in this field. Otherwise, \ref INVALID_HANDLE is returned in this field.
	    For \ref GSW_ExtendedVlanFree, this field should be valid ID returned by
	    \ref GSW_ExtendedVlanAlloc. */
	u16 nExtendedVlanBlockId;
} GSW_EXTENDEDVLAN_alloc_t;

/** \brief Extended VLAN Treatment.
    Used by \ref GSW_EXTENDEDVLAN_config_t. */
typedef struct {
	/** Number of VLAN tag(s) to remove. */
	GSW_ExtendedVlanTreatmentRemoveTag_t eRemoveTag;

	/** 4 TPID support */
	GSW_ExtendedVlan_4_Tpid_Mode_t eTreatment_4_Tpid_Mode;

	/** Enable outer VLAN tag add/modification. */
	gsw_bool_t bAddOuterVlan;
	/** If bAddOuterVlan is TRUE, add or modify outer VLAN tag. */
	GSW_EXTENDEDVLAN_treatmentVlan_t sOuterVlan;

	/** Enable inner VLAN tag add/modification. */
	gsw_bool_t bAddInnerVlan;
	/** If bAddInnerVlan is TRUE, add or modify inner VLAN tag. */
	GSW_EXTENDEDVLAN_treatmentVlan_t sInnerVlan;

	/** Enable re-assignment of bridge port. */
	gsw_bool_t bReassignBridgePort;
	/** If bReassignBridgePort is TRUE, use this value for bridge port. */
	u16 nNewBridgePortId;

	/** Enable new DSCP. */
	gsw_bool_t bNewDscpEnable;
	/** If bNewDscpEnable is TRUE, use this value for DSCP. */
	u16 nNewDscp;

	/** Enable new traffic class. */
	gsw_bool_t bNewTrafficClassEnable;
	/** If bNewTrafficClassEnable is TRUE, use this value for traffic class. */
	u8 nNewTrafficClass;

	/** Enable new meter. */
	gsw_bool_t bNewMeterEnable;
	/** New meter ID.

	    \remarks
	    Meter should be allocated with \ref GSW_QOS_MeterAlloc before extended
	    VLAN treatment is added. If this extended VLAN treatment is deleted,
	    this meter should be released with \ref GSW_QOS_MeterFree. */
	u16 sNewTrafficMeterId;

	/** DSCP to PCP mapping, if
	    \ref GSW_EXTENDEDVLAN_treatmentVlan_t::ePriorityMode in
	    \ref GSW_EXTENDEDVLAN_treatment_t::sOuterVlan or
	    \ref GSW_EXTENDEDVLAN_treatment_t::sInnerVlan is
	    \ref GSW_EXTENDEDVLAN_TREATMENT_DSCP.

	    \remarks
	    The index of array stands for DSCP value. Each byte of the array is 3-bit
	    PCP value. For implementation, if DSCP2PCP is separate hardware table,
	    a resource management mechanism should be implemented. Allocation happens
	    when extended VLAN treatment added, and release happens when the
	    treatment is deleted. For debug, the DSCP2PCP table can be dumped with
	    \ref GSW_QOS_Dscp2PcpTableGet. */
	u8 nDscp2PcpMap[64];

	/** Enable loopback. */
	gsw_bool_t bLoopbackEnable;
	/** Enable destination/source MAC address swap. */
	gsw_bool_t bDaSaSwapEnable;
	/** Enable traffic mirrored to the monitoring port. */
	gsw_bool_t bMirrorEnable;
} GSW_EXTENDEDVLAN_treatment_t;

/** \brief Extended VLAN Configuration.
    Used by \ref GSW_ExtendedVlanSet and \ref GSW_ExtendedVlanGet. */
typedef struct {
	/** This should be valid ID returned by \ref GSW_ExtendedVlanAlloc.
	    If it is \ref INVALID_HANDLE, \ref GSW_EXTENDEDVLAN_config_t::nEntryIndex
	    is absolute index of Extended VLAN entry in hardware for debug purpose,
	    bypassing any check. */
	u16 nExtendedVlanBlockId;

	/** Index of entry, ranges between 0 and
	    \ref GSW_EXTENDEDVLAN_alloc_t::nNumberOfEntries - 1, to
	    set (\ref GSW_ExtendedVlanSet) or get (\ref GSW_ExtendedVlanGet)
	    Extended VLAN Configuration entry. For debug purpose, this field could be
	    absolute index of Entended VLAN entry in hardware, when
	    \ref GSW_EXTENDEDVLAN_config_t::nExtendedVlanBlockId is
	    \ref INVALID_HANDLE. */
	u16 nEntryIndex;

	/** Extended VLAN Filter */
	GSW_EXTENDEDVLAN_filter_t sFilter;
	/** Extended VLAN Treatment */
	GSW_EXTENDEDVLAN_treatment_t sTreatment;
} GSW_EXTENDEDVLAN_config_t;

/** \brief VLAN Filter Allocation.
    Used by \ref GSW_VlanFilterAlloc and \ref GSW_VlanFilterFree. */
typedef struct {
	/** Total number of VLAN Filter entries are requested. Proper value should
	    be given for \ref GSW_VlanFilterAlloc. This field is ignored for
	    \ref GSW_VlanFilterFree. */
	u16 nNumberOfEntries;

	/** If \ref GSW_VlanFilterAlloc is successful, a valid ID will be returned
	    in this field. Otherwise, \ref INVALID_HANDLE is returned in this field.
	    For \ref GSW_ExtendedVlanFree, this field should be valid ID returned by
	    \ref GSW_VlanFilterAlloc. */
	u16 nVlanFilterBlockId;

	/** Discard packet without VLAN tag. */
	gsw_bool_t bDiscardUntagged;
	/** Discard VLAN tagged packet not matching any filter entry. */
	gsw_bool_t bDiscardUnmatchedTagged;
	/** Use default port VLAN ID for VLAN filtering

	    \remarks
	    This field is not available in PRX300. */
	gsw_bool_t bUseDefaultPortVID;
} GSW_VLANFILTER_alloc_t;

/** \brief VLAN Filter.
    Used by \ref GSW_VlanFilterSet and \ref GSW_VlanFilterGet */
typedef struct {
	/** This should be valid ID return by \ref GSW_VlanFilterAlloc.
	    If it is \ref INVALID_HANDLE, \ref GSW_VLANFILTER_config_t::nEntryIndex
	    is absolute index of VLAN Filter entry in hardware for debug purpose,
	    bypassing any check. */
	u16 nVlanFilterBlockId;

	/** Index of entry. ranges between 0 and
	    \ref GSW_VLANFILTER_alloc_t::nNumberOfEntries - 1, to
	    set (\ref GSW_VlanFilterSet) or get (\ref GSW_VlanFilterGet)
	    VLAN FIlter entry. For debug purpose, this field could be absolute index
	    of VLAN Filter entry in hardware, when
	    \ref GSW_VLANFILTER_config_t::nVlanFilterBlockId is
	    \ref INVALID_HANDLE. */
	u16 nEntryIndex;

	/** VLAN TCI filter mask mode.

	    \remarks
	    In GSWIP-3.1, this field of first entry in the block will applies to rest
	    of entries in the same block. */
	GSW_VlanFilterTciMask_t eVlanFilterMask;

	/** This is value for VLAN filtering. It depends on
	    \ref GSW_VLANFILTER_config_t::eVlanFilterMask.
	    For GSW_VLAN_FILTER_TCI_MASK_VID, this is 12-bit VLAN ID.
	    For GSW_VLAN_FILTER_TCI_MASK_PCP, this is 3-bit PCP field of VLAN tag.
	    For GSW_VLAN_FILTER_TCI_MASK_TCI, this is 16-bit TCI of VLAN tag. */
	u32 nVal;
	/** Discard packet if match. */
	gsw_bool_t bDiscardMatched;
} GSW_VLANFILTER_config_t;

/** VLAN Rmon Counters */
typedef enum {
	/** VLAN Rx Counters */
	GSW_VLAN_RMON_RX = 0,
	/** VLAN Tx Counters */
	GSW_VLAN_RMON_TX = 1,
	/** VLAN Tx Counters on PCE Bypass Path */
	GSW_VLAN_RMON_PCE_BYPASS = 2,
} GSW_VlanRMON_Type_t;

/**
 \brief RMON Counters structure for VLAN. */
typedef struct {
	/** VLAN counter index */
	u16 nVlanCounterIndex;
	/** VLAN counter type (Rx, Tx, Tx PCE Bypass) */
	GSW_VlanRMON_Type_t eVlanRmonType;
	/** VLAN byte counter */
	u64 nByteCount;
	/** VLAN total packet counter */
	u32 nTotalPktCount;
	/** VLAN mutlicast packet counter */
	u32 nMulticastPktCount;
	/** VLAN drop packet counter */
	u32 nDropPktCount;
	/** Clear all VLAN counters.
	 *  Used by \ref GSW_Vlan_RMON_Clear.
	 */
	u32 clear_all;
} GSW_VLAN_RMON_cnt_t;

/**
 \brief RMON Counters control structure for VLAN. */
typedef struct {
	/** enable VLAN counters */
	gsw_bool_t bVlanRmonEnable;
	/** count broadcast packet in VLAN multicast packet counter */
	gsw_bool_t bIncludeBroadCastPktCounting;
	/** index of the last valid entry in VLAN mapping table */
	u32 nVlanLastEntry;
} GSW_VLAN_RMON_control_t;

/** \brief VLAN Counter Mapping. */
typedef enum {
	/** VLAN Mapping for Ingress */
	GSW_VLAN_MAPPING_INGRESS = 0,
	/** VLAN Mapping for Egress */
	GSW_VLAN_MAPPING_EGRESS = 1,
	/** VLAN Mapping for Ingress and Egress */
	GSW_VLAN_MAPPING_INGRESS_AND_EGRESS = 2
} GSW_VlanCounterMappingType_t;

/** \brief VLAN Counter Mapping Filter. */
typedef enum {
	/** There is tag and criteria applies. */
	GSW_VLANCOUNTERMAP_FILTER_TYPE_NORMAL = 0,
	/** There is tag but no criteria. */
	GSW_VLANCOUNTERMAP_FILTER_TYPE_NO_FILTER = 1,
	/** Default entry if no other rule applies. */
	GSW_VLANCOUNTERMAP_FILTER_TYPE_DEFAULT = 2,
	/** There is no tag. */
	GSW_VLANCOUNTERMAP_FILTER_TYPE_NO_TAG = 3,
	/** Filter invalid*/
	GSW_VLANCOUNTERMAP_FILTER_INVALID = 4,
} GSW_VlanCounterMapFilterType_t;

/** \brief VLAN Counter Mapping Configuration. */
typedef struct {
	/** Counter Index */
	u8	nCounterIndex;
	/** Ctp Port Id */
	u16 nCtpPortId;
	/** Priority Enable */
	gsw_bool_t bPriorityEnable;
	/** Priority Val */
	u32 nPriorityVal;
	/** VLAN Id Enable */
	gsw_bool_t bVidEnable;
	/** VLAN Id Value */
	u32 nVidVal;
	/** VLAN Tag Selection Value */
	gsw_bool_t bVlanTagSelectionEnable;
	/** VLAN Counter Mapping Type */
	GSW_VlanCounterMappingType_t eVlanCounterMappingType;
	/** VLAN Counter Mapping Filter Type */
	GSW_VlanCounterMapFilterType_t eVlanCounterMappingFilterType;
} GSW_VlanCounterMapping_config_t;

/** @}*/ /* GSW_VLAN */


/** \addtogroup GSW_MULTICAST
 *  @{
 */

/** \brief Add an Ethernet port as router port to the switch hardware multicast table.
    Used by \ref GSW_MulticastRouterPortAdd and \ref GSW_MulticastRouterPortRemove. */
typedef struct {
	/** Bridge Port ID. The valid range is hardware dependent.
	    An error code is delivered if the selected port is not available.

	    \remarks
	    This field is used as portmap field, when the MSB bit is set.
	    In portmap mode, every value bit represents an Ethernet port.
	    LSB represents Port 0 with incrementing counting.
	    The (MSB - 1) bit represent the last port.
	    The macro \ref GSW_PORTMAP_FLAG_SET allows to set the MSB bit,
	    marking it as portmap variable.
	    Checking the portmap flag can be done by
	    using the \ref GSW_PORTMAP_FLAG_GET macro. */
	u16	nPortId;
} GSW_multicastRouter_t;

/** \brief Check if a port has been selected as a router port.
    Used by \ref GSW_MulticastRouterPortRead. Not applicable to GSWIP-3.1. */
typedef struct {
	/** Restart the get operation from the start of the table. Otherwise
	    return the next table entry (next to the entry that was returned
	    during the previous get operation). This parameter is always reset
	    during the read operation. This boolean parameter is set by the
	    calling application. */
	gsw_bool_t	bInitial;
	/** Indicates that the read operation got all last valid entries of the
	    table. This boolean parameter is set by the switch API
	    when the Switch API is called after the last valid one was returned already. */
	gsw_bool_t	bLast;
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. */
	u16	nPortId;
} GSW_multicastRouterRead_t;


/** \brief Configure the IGMP snooping mode.
    Used by \ref GSW_multicastSnoopCfg_t. */
typedef enum {
	/** IGMP management packet snooping and multicast level 3 table learning
	    is disabled. */
	GSW_MULTICAST_SNOOP_MODE_DISABLED = 0,
	/** IGMP management packet snooping is enabled and used for the hardware
	    auto-learning to fill the multicast level 3 table.
	    This is not supported and reserved. */
	GSW_MULTICAST_SNOOP_MODE_AUTOLEARNING	= 1,
	/** IGMP management packet snooping is enabled and forwarded to the
	    configured port. No autolearning of the multicast level 3 table. This
	    table has to be maintained by the management software. */
	GSW_MULTICAST_SNOOP_MODE_FORWARD = 2
} GSW_multicastSnoopMode_t;

/** \brief Configure the IGMP report suppression mode.
    Used by \ref GSW_multicastSnoopCfg_t. */
typedef enum {
	/** Report Suppression and Join Aggregation. */
	GSW_MULTICAST_REPORT_JOIN	= 0,
	/** Report Suppression. No Join Aggregation. */
	GSW_MULTICAST_REPORT	= 1,
	/** Transparent Mode. No Report Suppression and no Join Aggregation. */
	GSW_MULTICAST_TRANSPARENT	= 2
} GSW_multicastReportSuppression_t;

/** \brief Configure the switch multicast configuration.
    Used by \ref GSW_MulticastSnoopCfgSet and \ref GSW_MulticastSnoopCfgGet. */
typedef struct {
	/** Enables and configures the IGMP/MLD snooping feature.
	Select autolearning or management packet forwarding mode.
	Packet forwarding is done to the port selected in 'eForwardPort'. */
	GSW_multicastSnoopMode_t eIGMP_Mode;
	/** Enables snooped IGMP control packets treated as cross-CTAG VLAN packets. This
	parameter is used for hardware auto-learning and snooping packets
	forwarded to a dedicated port. This dedicated port can be selected
	over 'eForwardPort'. */
	gsw_bool_t bCrossVLAN;
	/** Forward snooped packet, only used if forwarded mode
	    is selected by
	    'eIGMP_Mode = GSW_MULTICAST_SNOOP_MODE_SNOOPFORWARD'. */
	GSW_portForward_t eForwardPort;
	/** Target Bridge Port ID for forwarded packets, only used if selected
	    by 'eForwardPort = GSW_PORT_FORWARD_PORT'. */
	u8 nForwardPortId;
	/** Snooping control class of service.
	Snooping control packet can be forwarded to the 'nForwardPortId' when
	selected in 'eIGMP_Mode'. The class of service of this port can be
	selected for the snooped control packets, starting from zero.
	The maximum possible service class depends
	on the hardware platform used. The value
	GSW_TRAFFIC_CLASS_DISABLE disables overwriting the given
	class assignment. */
	u8 nClassOfService;
} GSW_multicastSnoopCfg_t;

/** \brief Defines the multicast group member mode.
    Used by \ref GSW_multicastTable_t and \ref GSW_multicastTableRead_t. */
typedef enum {
	/** Include source IP address membership mode.
	    Only supported for IGMPv3. */
	GSW_IGMP_MEMBER_INCLUDE	= 0,
	/** Exclude source IP address membership mode.
	    Only supported for IGMPv2. */
	GSW_IGMP_MEMBER_EXCLUDE	= 1,
	/** Group source IP address is 'don't care'. This means all source IP
	    addresses (*) are included for the multicast group membership.
	    This is the default mode for IGMPv1 and IGMPv2. */
	GSW_IGMP_MEMBER_DONT_CARE	= 2,

	GSW_IGMP_MEMBER_INVALID,
} GSW_IGMP_MemberMode_t;

/** \brief Add a host as a member to a multicast group.
    Used by \ref GSW_MulticastTableEntryAdd and \ref GSW_MulticastTableEntryRemove. */
typedef struct {
	/** Ethernet Port number (zero-based counting) in GSWIP-2.1/2.2/3.0. From
	    GSWIP-3.1, this field is Bridge Port ID. The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. */
	u32	nPortId;
	/** Sub-Interface Id - valid for GSWIP 3.0/3.1 only */
	u16	nSubIfId;
	/** Select the IP version of the 'uIP_Gda' and 'uIP_Gsa' fields.
	    Both fields support either IPv4 or IPv6. */
	GSW_IP_Select_t	eIPVersion;
	/** Group Destination IP address (GDA). */
	GSW_IP_t	uIP_Gda;
	/** Group Source IP address. Only used in case IGMPv3 support is enabled
	    and 'eModeMember != GSW_IGMP_MEMBER_DONT_CARE'. */
	GSW_IP_t	uIP_Gsa;
	/** FID - valid for GSWIP 3.0 only subject to Global FID for MC is enabled.
	          always valid in GSWIP-3.1. */
	u8 nFID;
	/** Exclude Mode - valid for GSWIP 3.0 only - Includes or Excludes Source IP - uIP_Gsa */
	gsw_bool_t bExclSrcIP;
	/** Group member filter mode.
	    This is valid for GSWIP-3.0/3.1 to replaces bExclSrcIP.
	    This parameter is ignored when deleting a multicast membership table entry.
	    The configurations 'GSW_IGMP_MEMBER_EXCLUDE'
	    and 'GSW_IGMP_MEMBER_INCLUDE' are only supported
	    if IGMPv3 is used. */
	GSW_IGMP_MemberMode_t	eModeMember;
	/** TCI for (GSWIP-3.2) B-Step
	    Bit [0:11] - VLAN ID
	    Bit [12] - VLAN CFI/DEI
	    Bit [13:15] - VLAN PRI */
	u16 nTci;
	/** Dynamic or Static entry, bStatic=1, Static Entry else default Dynamic */
	gsw_bool_t bStatic;
	/** In the case of Multicast Table Search need the Index of HW table returned */
	u16 nIndex;
} GSW_multicastTable_t;


/** \brief Read out the multicast membership table.
    Used by \ref GSW_MulticastTableEntryRead. */
typedef struct {
	/** Restart the get operation from the beginning of the table. Otherwise
	    return the next table entry (next to the entry that was returned
	    during the previous get operation). This parameter is always reset
	    during the read operation. This boolean parameter is set by the
	    calling application. */
	gsw_bool_t	bInitial;
	/** Indicates that the read operation got all last valid entries of the
	    table. This boolean parameter is set by the switch API
	    when the Switch API is called after the last valid one was returned already. */
	gsw_bool_t	bLast;
	/** Ethernet Port number (zero-based counting) in GSWIP-2.1/2.2/3.0. From
	    GSWIP-3.1, this field is Bridge Port ID. The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available.

	    \remarks
	    This field is used as portmap field, when the MSB bit is set.
	    In portmap mode, every value bit represents an Ethernet port.
	    LSB represents Port 0 with incrementing counting.
	    The (MSB - 1) bit represent the last port.
	    The macro \ref GSW_PORTMAP_FLAG_SET allows to set the MSB bit,
	    marking it as portmap variable.
	    Checking the portmap flag can be done by
	    using the \ref GSW_PORTMAP_FLAG_GET macro. */
	u32	nPortId;
	/** Read Index for the Multicast Table Entry, In case need to read based on Index */
	uint16_t nIndex;
	/** Ethernet Port Map - to support GSWIP-3.1, following field is added
	    for port map in static entry. It's valid only when MSB of nPortId is set.
	    Each bit stands for 1 bridge port. */
	u16 nPortMap[8];	/* max can be 16 */
	/** Sub-Interface Id - valid for GSWIP 3.0 only */
	u16	nSubIfId;
	/** Select the IP version of the 'uIP_Gda' and 'uIP_Gsa' fields.
	    Both fields support either IPv4 or IPv6. */
	GSW_IP_Select_t	eIPVersion;
	/** Group Destination IP address (GDA). */
	GSW_IP_t	uIP_Gda;
	/** Group Source IP address. Only used in case IGMPv3 support is enabled. */
	GSW_IP_t	uIP_Gsa;
	/** FID - valid for GSWIP 3.0 only subject to Global FID for MC is enabled */
	u8 nFID;
	/** Exclude Mode - valid for GSWIP 3.0 only - Includes or Excludes Source IP - uIP_Gsa */
	gsw_bool_t bExclSrcIP;
	/** Group member filter mode.
	    This parameter is ignored when deleting a multicast membership table entry.
	    The configurations 'GSW_IGMP_MEMBER_EXCLUDE'
	    and 'GSW_IGMP_MEMBER_INCLUDE' are only supported
	    if IGMPv3 is used. */
	GSW_IGMP_MemberMode_t	eModeMember;
	/** MULTICAST Table Hit Status Update */
	gsw_bool_t hitstatus;
	/** TCI for (GSWIP-3.2) B-Step
	    Bit [0:11] - VLAN ID
	    Bit [12] - VLAN CFI/DEI
	    Bit [13:15] - VLAN PRI */
	u16 nTci;
	/** Dynamic or Static entry, bStatic=1, Static Entry else default Dynamic */
	gsw_bool_t bStatic;
} GSW_multicastTableRead_t;

/** @}*/ /* GSW_MULTICAST */


/** @cond INTERNAL */
/** \brief For debugging Purpose only.
    Used for GSWIP 3.3. */
typedef struct {
	/** Table Index to get status of the Table Index only for GSWIP 3.1 */
	u16 nTableIndex;
	/** Force table Index In USEonly for GSWIP 3.1 */
	u8 nForceSet;
	/** To check dispaly index which are In USE for GSWIP 3.1 */
	gsw_bool_t nCheckIndexInUse;
	/** Vlan Filter or Exvlan BlockID*/
	u16 nblockid;
	/** Vlan Filter debugging usage*/
	u8 nDiscardUntagged;
	/** Vlan Filter debugging usage*/
	u8 nDiscardUnMatchedTagged;
	/** Pmac debugging purpose*/
	u8 nPmacId;
	/** Pmac debugging purpose*/
	u8 nDestPort;
} GSW_debug_t;
/** @endcond */


/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/** \brief Global Switch configuration Attributes.
    Used by \ref GSW_CfgSet and \ref GSW_CfgGet. */
typedef struct {
	/** MAC table aging timer. After this timer expires the MAC table
	    entry is aged out. */
	GSW_ageTimer_t	eMAC_TableAgeTimer;
	/** If eMAC_TableAgeTimer = GSW_AGETIMER_CUSTOM, this variable defines
	    MAC table aging timer in seconds. */
	u32  nAgeTimer;
	/** Maximum Ethernet packet length. */
	u16	nMaxPacketLen;
	/** Automatic MAC address table learning limitation consecutive action.
	    These frame addresses are not learned, but there exists control as to whether
	    the frame is still forwarded or dropped.

	    - False: Drop
	    - True: Forward
	*/
	gsw_bool_t	bLearningLimitAction;
	/** Accept or discard MAC port locking violation packets.
	    MAC spoofing detection features identifies ingress packets that carry
	    a MAC source address which was previously learned on a different
	    ingress port (learned by MAC bridging table). This also applies to
	    static added entries. MAC address port locking is configured on
	    port level by 'bLearningMAC_PortLock'.

	    - False: Drop
	    - True: Forward
	*/
	gsw_bool_t	bMAC_LockingAction;
	/** Accept or discard MAC spoofing and port MAC locking violation packets.
	    MAC spoofing detection features identifies ingress packets that carry
	    a MAC source address which was previously learned on a different
	    ingress port (learned by MAC bridging table). This also applies to
	    static added entries. MAC spoofing detection is enabled on port
	    level by 'bMAC_SpoofingDetection'.

	    - False: Drop
	    - True: Forward
	*/
	gsw_bool_t	bMAC_SpoofingAction;
	/** Pause frame MAC source address mode. If enabled, use the alternative
	    address specified with 'nMAC'. */
	gsw_bool_t	bPauseMAC_ModeSrc;
	/** Pause frame MAC source address. */
	u8	nPauseMAC_Src[GSW_MAC_ADDR_LEN];
} GSW_cfg_t;


/** \brief Special tag Ethertype mode */
typedef enum {
	/** The EtherType field of the Special Tag of egress packets is always set
	    to a prefined value. This same defined value applies for all
	    switch ports. */
	GSW_CPU_ETHTYPE_PREDEFINED	= 0,
	/** The Ethertype field of the Special Tag of egress packets is set to
	    the FlowID parameter, which is a results of the switch flow
	    classification result. The switch flow table rule provides this
	    FlowID as action parameter. */
	GSW_CPU_ETHTYPE_FLOWID	= 1
} GSW_CPU_SpecialTagEthType_t;


/** \brief Parser Flags and Offsets Header settings on CPU Port for GSWIP-3.0.
    Used by \ref GSW_CPU_PortCfg_t. */
typedef enum {
	/** No Parsing Flags or Offsets accompanying to CPU Port for this combination */
	GSW_CPU_PARSER_NIL	= 0,
	/** 8-Bytes Parsing Flags (Bit 63:0) accompanying to CPU Port for this combination */
	GSW_CPU_PARSER_FLAGS	= 1,
	/** 40-Bytes Offsets (Offset-0 to -39) + 8-Bytes Parsing Flags (Bit 63:0) accompanying to CPU Port for this combination */
	GSW_CPU_PARSER_OFFSETS_FLAGS	= 2,
	/** Reserved - for future use */
	GSW_CPU_PARSER_RESERVED = 3
} GSW_CPU_ParserHeaderCfg_t;

/** \brief FCS and Pad Insertion operations for GSWIP 3.1
    Used by \ref GSW_CPU_PortCfgSet and \ref GSW_CPU_PortCfgGet. */
typedef enum {
	/** CRC Pad Insertion Enable */
	GSW_CRC_PAD_INS_EN	= 0,
	/** CRC Insertion Enable Pad Insertion Disable */
	GSW_CRC_EN_PAD_DIS	= 1,
	/** CRC Pad Insertion Disable */
	GSW_CRC_PAD_INS_DIS	= 2
} GSW_FCS_TxOps_t;

/** \brief Defines one port that is directly connected to CPU.
 *  Used by \ref GSW_CPU_PortSet and \ref GSW_CPU_PortGet.
 *  This API does not configure port settings but update global PCE rules
 *  using CPU port.
 */
typedef struct {
	/** CTP set to CPU Port.
	 *  In current design, first 17 CTP are 1-to-1 mapped to physical ports.
	 *  CTP 0 is native CPU (WSP ARC).
	 *  CTP 1~15 is allowed to be external CPU port.
	 *  CTP 16 is not allowed to be CPU port.
	 */
	u8 nPortId;
} GSW_CPU_Port_t;

/** \brief Defines one port that is directly connected to the CPU and its applicable settings.
    Used by \ref GSW_CPU_PortCfgSet and \ref GSW_CPU_PortCfgGet. */
typedef struct {
	/** Ethernet Port number (zero-based counting) set to CPU Port. The valid number is hardware
	    dependent. (E.g. Port number 0 for GSWIP-3.0 or 6 for GSWIP-2.x). An error code is delivered if the selected port is not
	    available. */
	u16	nPortId;
	/** CPU port validity.
	    Set command: set true to define a CPU port, set false to undo the setting.
	    Get command: true if defined as CPU, false if not defined as CPU port. */
	gsw_bool_t	bCPU_PortValid;
	/** Special tag enable in ingress direction. */
	gsw_bool_t	bSpecialTagIngress;
	/** Special tag enable in egress direction. */
	gsw_bool_t	bSpecialTagEgress;
	/** Enable FCS check

	    - false: No check, forward all frames
	    - 1: Check FCS, drop frames with errors
	*/
	gsw_bool_t	bFcsCheck;
	/** Enable FCS generation

	    - false: Forward packets without FCS
	    - 1: Generate FCS for all frames
	*/
	gsw_bool_t	bFcsGenerate;
	/** Special tag Ethertype mode. Not applicable to GSWIP-3.1. */
	GSW_CPU_SpecialTagEthType_t	bSpecialTagEthType;
	/** GSWIP-3.0 specific Parser Header Config for no MPE flags (i.e. MPE1=0, MPE2=0). */
	GSW_CPU_ParserHeaderCfg_t  eNoMPEParserCfg;
	/** GSWIP-3.0 specific Parser Header Config for MPE-1 set flag (i.e. MPE1=1, MPE2=0). */
	GSW_CPU_ParserHeaderCfg_t  eMPE1ParserCfg;
	/** GSWIP-3.0 specific Parser Header Config for MPE-2 set flag (i.e. MPE1=0, MPE2=1). */
	GSW_CPU_ParserHeaderCfg_t  eMPE2ParserCfg;
	/** GSWIP-3.0 specific Parser Header Config for both MPE-1 and MPE-2 set flag (i.e. MPE1=1, MPE2=1). */
	GSW_CPU_ParserHeaderCfg_t  eMPE1MPE2ParserCfg;
	/** GSWIP-3.1 FCS tx Operations. */
	GSW_FCS_TxOps_t bFcsTxOps;
	/** GSWIP-3.2 Time Stamp Field Removal for PTP Packet
	    0 - DIS Removal is disabled
	    1 - EN Removal is enabled
	*/
	gsw_bool_t	bTsPtp;
	/** GSWIP-3.2 Time Stamp Field Removal for Non-PTP Packet
	    0 - DIS Removal is disabled
	    1 - EN Removal is enabled
	*/
	gsw_bool_t	bTsNonptp;
} GSW_CPU_PortCfg_t;

/** \brief Global Ethernet trunking configuration.
    Used by \ref GSW_TrunkingCfgGet
    and \ref GSW_TrunkingCfgSet. */
typedef struct {
	/** IP source address is used by the
	    hash algorithm to calculate the egress trunking port index. */
	gsw_bool_t bIP_Src;
	/** IP destination address is used by the
	    hash algorithm to calculate the egress trunking port index. */
	gsw_bool_t bIP_Dst;
	/** MAC source address is used by the
	    hash algorithm to calculate the egress trunking port index. */
	gsw_bool_t bMAC_Src;
	/** MAC destination address is used by the
	    hash algorithm to calculate the egress trunking port index. */
	gsw_bool_t bMAC_Dst;
	/** TCP/UDP Source Port is used by the
	    hash algorithm to calculate the egress trunking port index. */
	gsw_bool_t bSrc_Port;
	/** TCP/UDP Destination Port is used by the
	    hash algorithm to calculate the egress trunking port index. */
	gsw_bool_t bDst_Port;
} GSW_trunkingCfg_t;

/** @}*/ /* GSW_ETHERNET_BRIDGING */


/** @cond DOC_ENABLE_PBB */
/** \addtogroup GSW_PBB
 *  @{
 */

/** \brief I-TAG header defintion .GSWIP-3.2 only
	Used by \ref GSW_PBB_Tunnel_Template_Config_t*/
typedef struct {
	/**I-TAG TPID -2 bytes field enable*/
	gsw_bool_t bTpidEnable;
	/**I-TAG TPID -2 bytes field*/
	u16 nTpid;

	/**I-TAG PCP -3 Bit field enable*/
	gsw_bool_t bPcpEnable;
	/**I-TAG PCP -3 Bit field*/
	u8 nPcp;

	/**I-TAG DEI -1 Bit field enable*/
	gsw_bool_t bDeiEnable;
	/**I-TAG DEI -1 Bit field*/
	u8 nDei;

	/**I-TAG UAC -1 Bit field enable*/
	gsw_bool_t bUacEnable;
	/**I-TAG UAC -1 Bit field*/
	u8 nUac;

	/**I-TAG RES -3 Bit field enable*/
	gsw_bool_t bResEnable;
	/**I-TAG RES -3 Bit field*/
	u8 nRes;

	/**I-TAG SID -24 Bit field enable*/
	gsw_bool_t bSidEnable;
	/**I-TAG SID -24 Bit field*/
	u32 nSid;
} GSW_I_TAG_Config_t;

/** \brief B-TAG header defintion .GSWIP-3.2 only
	Used by \ref GSW_PBB_Tunnel_Template_Config_t*/
typedef struct {
	/** B-TAG TPID -2 bytes field enable*/
	gsw_bool_t bTpidEnable;
	/** B-TAG TPID -2 bytes field*/
	u16 nTpid;

	/**B-TAG PCP -3 Bit field enable*/
	gsw_bool_t bPcpEnable;
	/**B-TAG PCP -3 Bit field*/
	u8 nPcp;

	/**B-TAG DEI -1 Bit field enable*/
	gsw_bool_t bDeiEnable;
	/**B-TAG DEI -1 Bit field*/
	u8 nDei;

	/**B-TAG VID -12 Bit field enable*/
	gsw_bool_t bVidEnable;
	/**B-TAG VID -12 Bit field*/
	u16 nVid;
} GSW_B_TAG_Config_t;

/** \brief Tunnel Template Configuration.GSWIP-3.2 only
    Used by \ref GSW_PBB_TunnelTempate_Config_Set and \ref GSW_PBB_TunnelTempate_Config_Get
    For \ref GSW_PBB_TunnelTempate_Free, this field should be valid ID returned by
	    \ref GSW_PBB_TunnelTempate_Alloc.*/
typedef struct {
	/** Mac-in-Mac (PBB) template index. */
	u16 nTunnelTemplateId;

	/** I-Header Destination Address enable*/
	gsw_bool_t bIheaderDstMACEnable;
	/** I-Header Destination Address*/
	u8 nIheaderDstMAC[GSW_MAC_ADDR_LEN];

	/** I-Header source Address enable*/
	gsw_bool_t bIheaderSrcMACEnable;
	/** I-Header source Address*/
	u8 nIheaderSrcMAC[GSW_MAC_ADDR_LEN];

	/** I-Tag enable*/
	gsw_bool_t bItagEnable;
	/** I-Tag*/
	GSW_I_TAG_Config_t sItag;

	/** B-Tag enable*/
	gsw_bool_t bBtagEnable;
	/** B-Tag*/
	GSW_B_TAG_Config_t sBtag;
} GSW_PBB_Tunnel_Template_Config_t;

/** @}*/ /* GSW_PBB */
/** @endcond DOC_ENABLE_PBB */


/** \addtogroup GSW_OAM
 *  @{
 */

/** \brief Port monitor configuration.
    Used by \ref GSW_MonitorPortCfgGet and \ref GSW_MonitorPortCfgSet. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	    dependent. An error code is delivered if the selected port is not
	    available. */
	u8	nPortId;
	/** Monitoring Sub-IF id */
	u16	nSubIfId;
	/** Reserved. */
	gsw_bool_t	bMonitorPort;
} GSW_monitorPortCfg_t;

/** @}*/ /* GSW_OAM */


/** \addtogroup GSW_ETHERNET_BRIDGING
 *  @{
 */

/** \brief Sets the portmap flag of a PortID variable.
    Some Switch API commands allow to use a port index as portmap variable.
    This requires that the MSB bit is set to indicate that this variable
    contains a portmap, instead of a port index.
    In portmap mode, every value bit represents an Ethernet port.
    LSB represents Port 0 with incrementing counting.
    The (MSB - 1) bit represent the last port. */
#define GSW_PORTMAP_FLAG_SET(varType) (1 << ( sizeof(((varType *)0)->nPortId) * 8 - 1))

/** \brief Checks the portmap flag of a PortID variable.
    Some Switch API commands allow to use a port index as portmap variable.
    This requires that the MSB bit is set to indicate that this variable
    contains a portmap, instead of a port index.
    In portmap mode, every value bit represents an Ethernet port.
    LSB represents Port 0 with incrementing counting.
    The (MSB - 1) bit represent the last port. */
#define GSW_PORTMAP_FLAG_GET(varType) (1 << ( sizeof(((varType *)0)->nPortId) * 8 - 1))

/** @}*/ /* GSW_ETHERNET_BRIDGING */


#pragma scalar_storage_order default
#pragma pack(pop)

#include "gsw_ctp.h"
#include "gsw_flow.h"
#include "gsw_pmac.h"
#include "gsw_tbl_rw.h"

#endif /* _MXL_GSW_H_ */
