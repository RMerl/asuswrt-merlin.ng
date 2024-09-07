/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef MXL_GSW_FLOW_H_
#define MXL_GSW_FLOW_H_

#include "gsw_types.h"
#include "gsw_flow_index.h"

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

/** \addtogroup GSW_DEBUG
 *  @{
 */

/** \brief Register access parameter to directly read or write switch
    internal registers.
    Used by \ref GSW_RegisterSet and \ref GSW_RegisterGet. */
typedef struct {
	/** Register Address Offset for read or write access. */
	u16 nRegAddr;
	/** Value to write to or read from 'nRegAddr'. */
	u16 nData;
} GSW_register_t;

/** \brief Register access parameter to directly modify internal registers.
    Used by \ref GSW_RegisterMod. */
typedef struct {
	/** Register Address Offset for modifiation. */
	u16 nRegAddr;
	/** Value to write to 'nRegAddr'. */
	u16 nData;
	/** Mask of bits to be modified. 1 to modify, 0 to ignore. */
	u16 nMask;
} GSW_register_mod_t;

/** @}*/ /* GSW_DEBUG */


/** \addtogroup GSW_PCE
 *  @{
 */

/** \brief Traffic Flow Table Mangaement.
 *   Used by \ref GSW_PCE_rule_t.
 */
typedef enum {
	/** PCE Rule Region common for all CTP */
	GSW_PCE_RULE_COMMMON = 0,
	/** PCE Rule Region for specific CTP */
	GSW_PCE_RULE_CTP = 1,
	/** PCE Rule Debug (HW direct mapping) */
	GSW_PCE_RULE_DEBUG = 2
} GSW_PCE_RuleRegion_t;

/** \brief Select Mode of Sub-Interface ID Field.
    Used by \ref GSW_PCE_pattern_t. */
typedef enum {
	/** Sub Interface ID group as defined by GSWIP-3.1. */
	GSW_PCE_SUBIFID_TYPE_GROUP = 0,
	/** Bridge Port ID as defined by GSWIP-3.1. */
	GSW_PCE_SUBIFID_TYPE_BRIDGEPORT = 1
} GSW_PCE_SUBIFID_TYPE_t;

/** \brief Rule selection for IPv4/IPv6.
    Used by \ref GSW_PCE_pattern_t. */
typedef enum {
	/** Rule Pattern for IP selection disabled. */
	GSW_PCE_IP_DISABLED	= 0,
	/** Rule Pattern for IPv4. */
	GSW_PCE_IP_V4	= 1,
	/** Rule Pattern for IPv6. */
	GSW_PCE_IP_V6	= 2
} GSW_PCE_IP_t;

/** \brief Packet Classification Engine Pattern Configuration.
     GSWIP-3.0 has additional patterns such as Inner IP, Inner DSCP, Inner Protocol, Exclude Mode etc.
    Used by \ref GSW_PCE_rule_t. */
typedef struct {
	/** PCE Rule Index (Upto 512 rules supported in GSWIP-3.0, whereas 64 rules supported in GSWIP-2.x) */
	u16	nIndex;
	/** Destination IP Nibble Mask.
	    1 bit represents 1 nibble mask of the 'nDstIP' field.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u32	nDstIP_Mask;
	/** Inner Destination IP Nibble Mask - for GSWIP-3.0 only.
	    1 bit represents 1 nibble mask of the 'nInnerDstIP' field.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u32	nInnerDstIP_Mask;
	/** Source IP Nibble Mask (Outer for GSWIP-3.0).
	    1 bit represents 1 nibble mask of the 'nSrcIP' field.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u32	nSrcIP_Mask;
	/** Inner Src IP Nibble Mask - for GSWIP-3.0 only.
	    1 bit represents 1 nibble mask of the 'nInnerSrcIP' field.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u32	nInnerSrcIP_Mask;
	/** Incoming Sub-Interface ID value - used for GSWIP-3.0 only */
	u16	nSubIfId;
	/** Packet length in bytes */
	u16		nPktLng;
	/** Packet length Range (from nPktLng to nPktLngRange) */
	u16		nPktLngRange;
	/** Destination MAC address nibble mask.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u16	nMAC_DstMask;
	/** Source MAC address nibble mask.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u16	nMAC_SrcMask;
	/** MSB Application field.
	    The first 2 bytes of the packet content following the IP header
	    for TCP/UDP packets (source port field), or the first 2 bytes of packet content
	    following the Ethertype for non-IP packets. Any part of this
	    content can be masked-out by a programmable bit
	    mask 'nAppMaskRangeMSB'. */
	u16	nAppDataMSB;
	/** MSB Application mask/range. When used as a range parameter,
	    1 bit represents 1 nibble mask of the 'nAppDataMSB' field.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u16	nAppMaskRangeMSB;
	/** Ethertype */
	u16	nEtherType;
	/** Ethertype Mask.
	    1 bit represents 1 nibble mask of the 'nEtherType' field.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u16	nEtherTypeMask;
	/** PPPoE Session Id */
	u16	nSessionId;
	/** PPP Protocol Value  - used for GSWIP-3.0 only*/
	u16	nPPP_Protocol;
	/** PPP protocol Bit Mask (Positional bit 1 signifies masking of corresponding bit value in nPPP_Protocol) - for GSWIP-3.0 only. */
	u16	nPPP_ProtocolMask;
	/** VLAN ID (CVID) */
	u16	nVid;
	/** STAG VLAN ID */
	u16	nSLAN_Vid;
	/** Flexible Field 4 value. 16 bit value for pattern match**/
	u16 nFlexibleField4_Value;
	/** Flexible Field 4 mask or range value.If bFlexibleField4_MaskEnable is 1 then
		 this 16 bit feid will be Mask or it will be Range**/
	u16 nFlexibleField4_MaskOrRange;
	/** Flexible Field 3 value. 16 bit value for pattern match**/
	u16 nFlexibleField3_Value;
	/** Flexible Field 3 mask or range value.If bFlexibleField4_MaskEnable is 1 then
	  this 16 bit feid will be Mask or it will be Range**/
	u16 nFlexibleField3_MaskOrRange;
	/** Flexible Field 1 value. 16 bit value for pattern match**/
	u16 nFlexibleField1_Value;
	/** Flexible Field 1 mask or range value.If bFlexibleField4_MaskEnable is 1 then
		  this 16 bit feid will be Mask or it will be Range**/
	u16 nFlexibleField1_MaskOrRange;
	/** MSB Application Data Exclude - for GSWIP-3.0 only */
	/** VLAN ID Range for outer VLAN tag. Used for GSWIP-3.1 only. */
	u16 nOuterVidRange;
	/** Payload-1 Value (16-bits) - for GSWIP-3.0 PAE only */
	u16	nPayload1;
	/** Payload-1 Bit mask - for GSWIP-3.0 PAE only */
	u16	nPayload1_Mask;
	/** Payload-2 Value (16-bits) - for GSWIP-3.0 PAE only */
	u16	nPayload2;
	/** Payload-2 Bit mask - for GSWIP-3.0 PAE only */
	u16	nPayload2_Mask;
	/** Parser Flag LSW Value - each bit indicates specific parsed result */
	u16	nParserFlagLSB;
	/** Corresponding LSW Parser Flag Mask - when the bit is set to 1 corresponding flag gets masked out (ignored). */
	u16	nParserFlagLSB_Mask;
	/** Parser Flag MSW Value - each bit indicates specific parsed result */
	u16	nParserFlagMSB;
	/** Corresponding Parser Flag MSW Mask - when the bit is set to 1 corresponding flag gets masked out (ignored). */
	u16	nParserFlagMSB_Mask;
	/** Parser Flag LSW Value - each bit indicates specific parsed result */
	u16	nParserFlag1LSB;
	/** Corresponding LSW Parser Flag Mask - when the bit is set to 1 corresponding flag gets masked out (ignored). */
	u16	nParserFlag1LSB_Mask;
	/** Parser Flag MSW Value - each bit indicates specific parsed result */
	u16	nParserFlag1MSB;
	/** Corresponding Parser Flag MSW Mask - when the bit is set to 1 corresponding flag gets masked out (ignored). */
	u16	nParserFlag1MSB_Mask;
	/** LSB Application field.
	    The following 2 bytes of the packet behind the 'nAppDataMSB' field.
	    This is the destination port field for TCP/UDP packets,
	    or byte 3 and byte 4 of the packet content following the Ethertype
	    for non-IP packets. Any part of this content can be masked-out
	    by a programmable bit mask 'nAppMaskRangeLSB'. */
	u16	nAppDataLSB;
	/** LSB Application mask/range. When used as a range parameter,
	    1 bit represents 1 nibble mask of the 'nAppDataLSB' field.
	    Please clear the bits of the nibbles that are not marked out and set all other bits.
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u16	nAppMaskRangeLSB;
	/** Inserted packet by CPU to data path. For GSWIP-3.1 only */
	u16 nInsertionFlag;
	/** VLAN ID Range (CVID). Gets used as mask to nVid in case bVidRange_Select is set to 0 */
	u16	nVidRange;
	/** Flexible Field 2 mask or range value.If bFlexibleField4_MaskEnable is 1 then
	 * this 16 bit feid will be Mask or it will be Range**/
	u16 nFlexibleField2_MaskOrRange;
	/** Flexible Field 2 value. 16 bit value for pattern match**/
	u16 nFlexibleField2_Value;
	/** Port ID value of incoming packets used for classification */
	u8	nPortId;
	/** DSCP value (Outer for GSWIP-3.0) */
	u8		nDSCP;
	/** Inner DSCP value  for GSWIP-3.0 only */
	u8		nInnerDSCP;
	/** CTAG (i.e Inner Tag) VLAN DEI (bit 3) and PCP (bit 2-0) value */
	u8		nPCP;
	/** STAG (i.e Outer Tag) VLAN DEI (bit 3) and PCP (bit 2-0) value */
	u8		nSTAG_PCP_DEI;
	/** Destination MAC address */
	u8		nMAC_Dst[6];
	/** Source MAC address */
	u8	nMAC_Src[6];
	/** IP protocol Value */
	u8	nProtocol;
	/** IP protocol Mask.
	    1 bit represents 1 nibble mask of the 'nProtocol' field.
	    Please clear the bits of the nibbles that are not marked out and set all other bits i.e. a set bit 1 indicates that bit is masked out (not compared).
	    The LSB bit represents the lowest data nibble, the next bit the next nibble,
	    and so on. */
	u8	nProtocolMask;
	/** Inner IP protocol Value - for GSWIP-3.0 only. */
	u8	nInnerProtocol;
	/** Inner IP protocol Bit Mask - for GSWIP-3.0 only. */
	u8	nInnerProtocolMask;
	/** Flexible Field 4 parser out put index 0-127**/
	u8 nFlexibleField4_ParserIndex;
	/** Flexible Field 3 parser out put index 0-127**/
	u8 nFlexibleField3_ParserIndex;
	/** Flexible Field 1 parser out put index 0-127**/
	u8 nFlexibleField1_ParserIndex;
	/** Flexible Field 2 parser out put index 0-127**/
	u8 nFlexibleField2_ParserIndex;
	/** Index is used (enabled) or set to unused (disabled) */
	gsw_bool_t	bEnable;
	/** Port ID used  for ingress packet classification */
	gsw_bool_t	bPortIdEnable;
	/** Exclude Port Id Value - When set exclusion of specified nPortId takes effect.  Available for GSWIP-3.0 only */
	gsw_bool_t	bPortId_Exclude;
	/** Select mode of sub-interface ID field */
	GSW_PCE_SUBIFID_TYPE_t eSubIfIdType;
	/** Incoming Sub-Interface ID Enable - used for GSWIP-3.0 only */
	gsw_bool_t	bSubIfIdEnable;
	/** Exclude of specified Sub-Interface Id value in nSubIfId - used for GSWIP-3.0 only */
	gsw_bool_t	bSubIfId_Exclude;
	/** DSCP value used (Outer for GSWIP-3.0) */
	gsw_bool_t	bDSCP_Enable;
	/** Exclude (Outer) DSCP value used for GSWIP-3.0 only */
	gsw_bool_t	bDSCP_Exclude;
	/** Inner DSCP value used for GSWIP-3.0 only */
	gsw_bool_t	bInner_DSCP_Enable;
	/** Exclude of Inner DSCP (nInnerDSCP) value used for GSWIP-3.0 only */
	gsw_bool_t	bInnerDSCP_Exclude;
	/** CTAG VLAN PCP n DEI value used */
	gsw_bool_t	bPCP_Enable;
	/** Exclude CTAG PCP & DEI value used for GSWIP-3.0 only */
	gsw_bool_t	bCTAG_PCP_DEI_Exclude;
	/** STAG VLAN PCP/DEI value used */
	gsw_bool_t	bSTAG_PCP_DEI_Enable;
	/** Exclude STAG PCP & DEI value used for GSWIP-3.0 only */
	gsw_bool_t	bSTAG_PCP_DEI_Exclude;
	/** Packet length used for classification */
	gsw_bool_t	bPktLngEnable;
	/** Exclude of Packet Length or range value used for GSWIP-3.0 only */
	gsw_bool_t	bPktLng_Exclude;
	/** Destination MAC address used */
	gsw_bool_t	bMAC_DstEnable;
	/** Exclude Destination MAC Address used for GSWIP-3.0 only */
	gsw_bool_t	bDstMAC_Exclude;
	/** Source MAC address used */
	gsw_bool_t	bMAC_SrcEnable;
	/** Exclude Source MAC Address used for GSWIP-3.0 only */
	gsw_bool_t	bSrcMAC_Exclude;
	/** MSB Application field used */
	gsw_bool_t	bAppDataMSB_Enable;
	/** MSB Application mask/range selection.
	    If set to LTQ_TRUE, the field 'nAppMaskRangeMSB' is used as a
	    range parameter, otherwise it is used as a nibble mask field. */
	gsw_bool_t	bAppMaskRangeMSB_Select;
	/** MSB Application exclude mode */
	gsw_bool_t	bAppMSB_Exclude;
	/** LSB Application used */
	gsw_bool_t	bAppDataLSB_Enable;
	/** LSB Application mask/range selection.
	    If set to LTQ_TRUE, the field 'nAppMaskRangeLSB' is used as
	    a range parameter, otherwise it is used as a nibble mask field. */
	gsw_bool_t	bAppMaskRangeLSB_Select;
	/** LSB Application Data Exclude - for GSWIP-3.0 only */
	gsw_bool_t	bAppLSB_Exclude;
	/** Destination IP Selection (Outer for GSWIP-3.0). */
	GSW_PCE_IP_t	eDstIP_Select;
	/** Destination IP (Outer for GSWIP-3.0) */
	GSW_IP_t	nDstIP;
	/** Exclude Destination IP Value - used for GSWIP-3.0 only */
	gsw_bool_t	bDstIP_Exclude;
	/** Inner Destination IP Selection - for GSWIP-3.0 only. */
	GSW_PCE_IP_t	eInnerDstIP_Select;
	/** Inner Destination IP  - for GSWIP-3.0 only.*/
	GSW_IP_t	nInnerDstIP;
	/** Exclude Inner Destination IP Value - used for GSWIP-3.0 only */
	gsw_bool_t	bInnerDstIP_Exclude;
	/** Source IP Selection (Outer for GSWIP-3.0). */
	GSW_PCE_IP_t	eSrcIP_Select;
	/** Source IP  (Outer for GSWIP-3.0) */
	GSW_IP_t	nSrcIP;
	/** Exclude Source IP Value - used for GSWIP-3.0 only */
	gsw_bool_t	bSrcIP_Exclude;
	/** Inner Source IP Selection - for GSWIP-3.0 only. */
	GSW_PCE_IP_t	eInnerSrcIP_Select;
	/** Inner Source IP  - for GSWIP-3.0 only*/
	GSW_IP_t	nInnerSrcIP;
	/** Exclude Inner Source IP Value - used for GSWIP-3.0 only */
	gsw_bool_t	bInnerSrcIP_Exclude;
	/** Ethertype used. */
	gsw_bool_t	bEtherTypeEnable;
	/** Exclude for Ether Type Value - used for GSWIP-3.0 only. */
	gsw_bool_t	bEtherType_Exclude;
	/** IP protocol used */
	gsw_bool_t	bProtocolEnable;
	/** Exclude for IP Protocol Value - used for GSWIP-3.0 only. */
	gsw_bool_t	bProtocol_Exclude;
	/** Inner IP protocol used - for GSWIP-3.0 only. */
	gsw_bool_t	bInnerProtocolEnable;
	/** Exclude for Inner IP Protocol Value - used for GSWIP-3.0 only. */
	gsw_bool_t	bInnerProtocol_Exclude;
	/** PPPoE used. */
	gsw_bool_t	bSessionIdEnable;
	/** Exclude for PPPoE Session Value - used for GSWIP-3.0 only. */
	gsw_bool_t	bSessionId_Exclude;
	/** PPP Protocol used - used for GSWIP-3.0 only */
	gsw_bool_t	bPPP_ProtocolEnable;
	/** Exclude for PPP Protocol Value - used for GSWIP-3.0 only. */
	gsw_bool_t	bPPP_Protocol_Exclude;
	/** VLAN ID (CVID) used.
	    \remarks
	    CVID is inner VLAN as defined in GSWIP-3.1 */
	gsw_bool_t	bVid;
	/** VID mask/range selection.
	    If set to 1, the field 'nVidRange' is used as
	    a range parameter, otherwise it is used as a mask field.
	    \remarks
	    This must be range in GSWIP-3.1 */
	gsw_bool_t	bVidRange_Select;
	/** Exclude for VLAN Id (CVLAN) - used for GSWIP-3.0 only. */
	gsw_bool_t	bVid_Exclude;
	/** If this field is TRUE, use original VLAN ID as key even it's modified in
	    any stage before flow table process. Used for GSWIP-3.1 only. */
	gsw_bool_t bVid_Original;
	/** STAG VLAN ID used.
	    \remarks
	    SLAN is outer VLAN as defined GSWIP-3.1 */
	gsw_bool_t	bSLAN_Vid;
	/** Exclude for SVLAN Id (SVLAN) - used for GSWIP-3.0 only. */
	gsw_bool_t	bSLANVid_Exclude;
	/** VID mask/range selection.
	    If set to 1, the field 'nVidRange' is used as
	    a range parameter, otherwise it is used as a mask field.
	    \remarks
	    This must be range in GSWIP-3.1 */
	gsw_bool_t	bSVidRange_Select;
	/** If this field is TRUE, use original VLAN ID as key even it's modified in
	    any stage before flow table process. Used for GSWIP-3.1 only. */
	gsw_bool_t bOuterVid_Original;
	/** Payload-1 used - for GSWIP-3.0 PAE only */
	gsw_bool_t	bPayload1_SrcEnable;
	/** Payload1 mask/range selection.
	    If set to LTQ_TRUE, the field 'nPayload1' is used as
	    a range parameter, otherwise it is used as a bit mask field. */
	gsw_bool_t	bPayload1MaskRange_Select;
	/** Exclude Payload-1 used for GSWIP-3.0 PAE only */
	gsw_bool_t	bPayload1_Exclude;
	/** Payload-2 used - for GSWIP-3.0 PAE only */
	gsw_bool_t	bPayload2_SrcEnable;
	/** Payload2 mask/range selection.
	    If set to LTQ_TRUE, the field 'nPayload2' is used as
	    a range parameter, otherwise it is used as a bit mask field. */
	gsw_bool_t	bPayload2MaskRange_Select;
	/** Exclude Payload-2 used for GSWIP-3.0 PAE only */
	gsw_bool_t	bPayload2_Exclude;
	/** Parser Flag LSW (Bit position 15 to 0) is used - for GSWIP 3.0 only */
	gsw_bool_t	bParserFlagLSB_Enable;
	/** Exclude for Parser Flag LSW specified in nParserFlagLSB */
	gsw_bool_t	bParserFlagLSB_Exclude;
	/** Parser Flag MSW (Bit 31 to 16) is used - for GSWIP 3.0 only */
	gsw_bool_t	bParserFlagMSB_Enable;
	/** Exclude for Parser Flag MSW specified in nParserFlagMSB */
	gsw_bool_t	bParserFlagMSB_Exclude;
	/** Parser Flag LSW (Bit position 47 to 32) is used - for GSWIP 3.1 only */
	gsw_bool_t	bParserFlag1LSB_Enable;
	/** Exclude for Parser Flag LSW specified in nParserFlagLSB */
	gsw_bool_t	bParserFlag1LSB_Exclude;
	/** Parser Flag MSW (Bit 63 to 48) is used - for GSWIP 3.1 only */
	gsw_bool_t	bParserFlag1MSB_Enable;
	/** Exclude for Parser Flag MSW specified in nParserFlagMSB */
	gsw_bool_t	bParserFlag1MSB_Exclude;
	/** nInsertionFlag is used. For GSWIP-3.1 only */
	gsw_bool_t bInsertionFlag_Enable;
	/** Flexible Field 4: 1 enable and 0 disable */
	gsw_bool_t bFlexibleField4Enable;
	/** Flexible Field 4 exclude mode 1 enable and 0 disable */
	gsw_bool_t bFlexibleField4_ExcludeEnable;
	/** Flexible Field 4 parser mask or range selction - 0 mask/1 range */
	gsw_bool_t bFlexibleField4_RangeEnable;
	/** Flexible Field 3: 1 enable and 0 disable */
	gsw_bool_t bFlexibleField3Enable;
	/** Flexible Field 3 exclude mode 1 enable and 0 disable */
	gsw_bool_t bFlexibleField3_ExcludeEnable;
	/** Flexible Field 3 parser mask or range selction - 0 mask/1 range */
	gsw_bool_t bFlexibleField3_RangeEnable;
	/** Flexible Field 2: 1 enable and 0 disable */
	gsw_bool_t bFlexibleField2Enable;
	/** Flexible Field 2 exclude mode 1 enable and 0 disable */
	gsw_bool_t bFlexibleField2_ExcludeEnable;
	/** Flexible Field 2 parser mask or range selction - 0 mask/1 range */
	gsw_bool_t bFlexibleField2_RangeEnable;
	/** Flexible Field 1: 1 enable and 0 disable */
	gsw_bool_t bFlexibleField1Enable;
	/** Flexible Field 1 exclude mode 1 enable and 0 disable */
	gsw_bool_t bFlexibleField1_ExcludeEnable;
	/** Flexible Field 1 parser mask or range selction - 0 mask/1 range */
	gsw_bool_t bFlexibleField1_RangeEnable;
} GSW_PCE_pattern_t;

/** \brief Traffic Class Action Selector.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disabled. Traffic class action is disabled. */
	GSW_PCE_ACTION_TRAFFIC_CLASS_DISABLE	= 0,
	/** Regular Class. Traffic class action is enabled and the CoS
	    classification traffic class is used. */
	GSW_PCE_ACTION_TRAFFIC_CLASS_REGULAR	= 1,
	/** Alternative Class. Traffic class action is enabled and the
	    class of the 'nTrafficClassAlter' field is used. */
	GSW_PCE_ACTION_TRAFFIC_CLASS_ALTERNATIVE	= 2,
} GSW_PCE_ActionTrafficClass_t;

/** \brief IGMP Snooping Control.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disabled. IGMP Snooping is disabled. */
	GSW_PCE_ACTION_IGMP_SNOOP_DISABLE	= 0,
	/** Default. Regular Packet. No IGMP Snooping action required. */
	GSW_PCE_ACTION_IGMP_SNOOP_REGULAR	= 1,
	/** IGMP Report/Join Message. */
	GSW_PCE_ACTION_IGMP_SNOOP_REPORT	= 2,
	/** IGMP Leave Message. */
	GSW_PCE_ACTION_IGMP_SNOOP_LEAVE	= 3,
	/**  Router Solicitation/Advertisement message. */
	GSW_PCE_ACTION_IGMP_SNOOP_AD	= 4,
	/** IGMP Query Message. */
	GSW_PCE_ACTION_IGMP_SNOOP_QUERY	= 5,
	/** IGMP Group Specific Query Message. */
	GSW_PCE_ACTION_IGMP_SNOOP_QUERY_GROUP	= 6,
	/** IGMP General Query message without Router Solicitation. */
	GSW_PCE_ACTION_IGMP_SNOOP_QUERY_NO_ROUTER = 7
} GSW_PCE_ActionIGMP_Snoop_t;

/** \brief MAC Address Learning control.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Learning is based on the forwarding decision. If the packet is discarded,
	    the address is not learned. If the packet is forwarded to any egress port,
	    the address is learned. */
	GSW_PCE_ACTION_LEARNING_DISABLE	= 0,
	/** Reserved */
	GSW_PCE_ACTION_LEARNING_REGULAR	= 1,
	/** Force No Learning. The address is not learned; forwarding decision
	    ignored. */
	GSW_PCE_ACTION_LEARNING_FORCE_NOT = 2,
	/** Force Learning. The address is learned, the forwarding decision ignored.
	    Note: The MAC Learning Control signals delivered to Port-Map filtering
	    and combined with Final Forwarding Decision. The result is used as a
	    feedback for MAC Address learning in the Bridging Table. */
	GSW_PCE_ACTION_LEARNING_FORCE	= 3
} GSW_PCE_ActionLearning_t;

/** \brief Interrupt Control Action Selector.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disabled. Interrupt Control Action is disabled for this rule. */
	GSW_PCE_ACTION_IRQ_DISABLE	= 0,
	/** Regular Packet. The Interrupt Control Action is enabled, the packet is
	    treated as a regular packet and no interrupt event is generated. */
	GSW_PCE_ACTION_IRQ_REGULAR	= 1,
	/** Interrupt Event. The Interrupt Control Action is enabled and an
	    interrupt event is generated. */
	GSW_PCE_ACTION_IRQ_EVENT	= 2
} GSW_PCE_ActionIrq_t;

/** \brief Cross State Action Selector.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disable. The Cross State Action is disabled. */
	GSW_PCE_ACTION_CROSS_STATE_DISABLE	= 0,
	/** Regular Packet. The Cross State Action is enabled and the packet is
	    treated as a non-Cross-State packet (regular packet). Therefore it does
	    not ignore Port-State filtering rules. */
	GSW_PCE_ACTION_CROSS_STATE_REGULAR	= 1,
	/** Cross-State packet. The Cross State Action is enabled and the packet is
	    treated as a Cross-State packet. It ignores the Port-State
	    filtering rules. */
	GSW_PCE_ACTION_CROSS_STATE_CROSS	= 2
} GSW_PCE_ActionCrossState_t;

/** \brief Critical Frame Action Selector.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disable. The Critical Frame Action is disabled. */
	GSW_PCE_ACTION_CRITICAL_FRAME_DISABLE	= 0,
	/** Regular Packet. The Critical Frame Action is enabled and the packet is
	    treated as a non-Critical Frame. */
	GSW_PCE_ACTION_CRITICAL_FRAME_REGULAR	= 1,
	/** Critical Packet. The Critical Frame Action is enabled and the packet is
	    treated as a Critical Frame. */
	GSW_PCE_ACTION_CRITICAL_FRAME_CRITICAL	= 2
} GSW_PCE_ActionCriticalFrame_t;

/** \brief Color Frame Action Selector.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disable. No color frame action. */
	GSW_PCE_ACTION_COLOR_FRAME_DISABLE = 0,
	/** Do not change color. */
	GSW_PCE_ACTION_COLOR_FRAME_NO_CHANGE = 1,
	/** Idendity packet as critical which bypass active congestion
	    management (ACM). */
	GSW_PCE_ACTION_COLOR_FRAME_CRITICAL = 2,
	/** Change to green color. */
	GSW_PCE_ACTION_COLOR_FRAME_GREEN = 3,
	/** Change to yellow color. */
	GSW_PCE_ACTION_COLOR_FRAME_YELLOW = 4,
	/** Change to red color. */
	GSW_PCE_ACTION_COLOR_FRAME_RED = 5
} GSW_PCE_ActionColorFrame_t;

/** \brief Timestamp Action Selector.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disable. Timestamp Action is disabled for this rule. */
	GSW_PCE_ACTION_TIMESTAMP_DISABLE	= 0,
	/** Regular Packet. The Timestamp Action is enabled for this rule.
	    The packet is treated as a regular packet and no timing information
	    is stored. */
	GSW_PCE_ACTION_TIMESTAMP_REGULAR	= 1,
	/** Receive/Transmit Timing packet. Ingress and Egress Timestamps for
	    this packet should be stored. */
	GSW_PCE_ACTION_TIMESTAMP_STORED	= 2
} GSW_PCE_ActionTimestamp_t;

/** \brief Forwarding Group Action Selector.
    This flow table action and the 'bFlowID_Action' action
    can be used exclusively.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disable. Forwarding Group Action is disabled. */
	GSW_PCE_ACTION_PORTMAP_DISABLE	= 0,
	/** Regular Packet. Forwarding Action enabled. Select Default
	    Port-Map (result of Default Forwarding Classification). */
	GSW_PCE_ACTION_PORTMAP_REGULAR	= 1,
	/** Discard. Discard the packets. */
	GSW_PCE_ACTION_PORTMAP_DISCARD	= 2,
	/** Forward to the CPU port. This requires that the CPU port is previously
	    set by calling \ref GSW_CPU_PortCfgSet. */
	GSW_PCE_ACTION_PORTMAP_CPU	= 3,
	/** Forward to a portmap, selected by the parameter 'nForwardPortMap'.
	    Please note that this feature is not supported by all
	    hardware platforms. */
	GSW_PCE_ACTION_PORTMAP_ALTERNATIVE	= 4,
	/** The packet is treated as Multicast Router
	    Solicitation/Advertisement or Query packet. */
	GSW_PCE_ACTION_PORTMAP_MULTICAST_ROUTER	= 5,
	/** The packet is interpreted as Multicast packet and learned in the
	    multicast group table. */
	GSW_PCE_ACTION_PORTMAP_MULTICAST_HW_TABLE = 6,
	/** The CTAG VLAN portmap classification result is replaced by the
	    portmap parameter 'nForwardPortMap'. All other classification
	    results stay unchanged and will be combined together with
	    the overwritten portmap. */
	GSW_PCE_ACTION_PORTMAP_ALTERNATIVE_VLAN	= 7,
	/** Add STAG VLAN portmap 'nForwardPortMap' to the overall portmap
	    classification result (AND'ed with the portmap). */
	GSW_PCE_ACTION_PORTMAP_ALTERNATIVE_STAG_VLAN	= 8
} GSW_PCE_ActionPortmap_t;

/** \brief Flow Meter Assignment control.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Action Disable. */
	GSW_PCE_ACTION_METER_DISABLE	= 0,
	/** Action Enable.
	    The action is enabled but no dedicated metering instance is assigned by the rule. */
	GSW_PCE_ACTION_METER_REGULAR	= 1,
	/** Action Enable. Assign one meter instance as given in parameter "nMeterId". */
	GSW_PCE_ACTION_METER_1	= 2,
	/** Action Enable. Assign pair of meter instances.
	    These instances are "nMeterId" and the next following meter instance index. */
	GSW_PCE_ACTION_METER_1_2	= 3
} GSW_PCE_ActionMeter_t;

/** \brief VLAN Group Action Selector.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disabled. The VLAN Action is disabled. */
	GSW_PCE_ACTION_VLAN_DISABLE	= 0,
	/** Regular VLAN. VLAN Action enabled. Select Default VLAN ID. */
	GSW_PCE_ACTION_VLAN_REGULAR	= 1,
	/** Alternative VLAN. VLAN Action enabled. Reserved. */
	GSW_PCE_ACTION_VLAN_ALTERNATIVE	= 2
} GSW_PCE_ActionVLAN_t;

/** \brief Cross VLAN Action Selector.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Disabled. The Cross VLAN Action is disabled. */
	GSW_PCE_ACTION_CROSS_VLAN_DISABLE	= 0,
	/** Regular VLAN Packet. Do not ignore VLAN filtering rules. */
	GSW_PCE_ACTION_CROSS_VLAN_REGULAR	= 1,
	/** Cross-VLAN packet. Ignore VLAN filtering  rules.*/
	GSW_PCE_ACTION_CROSS_VLAN_CROSS	= 2
} GSW_PCE_ActionCrossVLAN_t;

/** \brief MPE Processing Path Assignment Selector - used for GSWIP-3.0 only.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Processing Path is not enabled. */
	GSW_PCE_PROCESSING_PATH_UNUSED = 0,
	/** Processing Path-1 is used for MPE-1. */
	GSW_PCE_PROCESSING_PATH_1 = 1,
	/** Processing Path-2 is used for MPE-2. */
	GSW_PCE_PROCESSING_PATH_2 = 2,
	/** Processing Path-1 and -2 are used for MPE-1 & MPE-2. */
	GSW_PCE_PROCESSING_PATH_BOTH = 3,
} GSW_PCE_ProcessingPathAction_t;

/** \brief Port Filter Action-1/2/3/4/5/6 Selector - used for GSWIP-3.0 only.
     This can be used only along with PortMember config.
    Used by \ref GSW_PCE_action_t. */
typedef enum {
	/** Port Filter Action is Unused. */
	GSW_PCE_PORT_FILTER_ACTION_UNUSED = 0,
	/** Port Filter Action Type-1 is used. */
	GSW_PCE_PORT_FILTER_ACTION_1	= 1,
	/** Port Filter Action Type-2 is used. */
	GSW_PCE_PORT_FILTER_ACTION_2	= 2,
	/** Port Filter Action Type-3 is used. */
	GSW_PCE_PORT_FILTER_ACTION_3	= 3,
	/** Port Filter Action Type-4 is used. */
	GSW_PCE_PORT_FILTER_ACTION_4	= 4,
	/** Port Filter Action Type-5 (Unknown Unicast) is used. */
	GSW_PCE_PORT_FILTER_ACTION_5	= 5,
	/** Port Filter Action Type-6 (Unknown Multicast) is used. */
	GSW_PCE_PORT_FILTER_ACTION_6	= 6
} GSW_PCE_PortFilterAction_t;

/** \brief Mac-in-Mac (PBB) I-Header operation mode.
    Used by \ref GSW_PCE_ActionPBB_t. */
typedef enum {
	/** No change in packet. */
	GSW_PCE_I_HEADER_OPERATION_NOCHANGE = 0,
	/** Insert I-Header for Non-PBB packet */
	GSW_PCE_I_HEADER_OPERATION_INSERT = 1,
	/** Remove I-Header for PBB packet
	   (Removes both I-Header and B-Tag)*/
	GSW_PCE_I_HEADER_OPERATION_REMOVE = 2,
	/** Replace the I-Header Fields for PBB packet */
	GSW_PCE_I_HEADER_OPERATION_REPLACE = 3,
} GSW_PCE_IheaderOperationMode;

/** \brief Mac-in-Mac (PBB) B-tag operation mode.
    Used by \ref GSW_PCE_ActionPBB_t. */
typedef enum {
	/** No change in B-TAG for PBB packet. */
	GSW_PCE_B_TAG_OPERATION_NOCHANGE = 0,
	/** Insert B-TAG for PBB packet */
	GSW_PCE_B_TAG_OPERATION_INSERT = 1,
	/** Remove B-TAG for PBB packet */
	GSW_PCE_B_TAG_OPERATION_REMOVE = 2,
	/** Replace B-TAG fields for PBB packet */
	GSW_PCE_B_TAG_OPERATION_REPLACE = 3,
} GSW_PCE_BtagOperationMode;

/** \brief Mac-in-Mac (PBB) MAC selection.
    Used by \ref GSW_PCE_ActionPBB_t. */
typedef enum {
	/** Outer Mac address is selected for learning
		non-PBB packet or PBB encapsulation*/
	GSW_PCE_OUTER_MAC_SELECTED = 0,
	/** inner Mac address is selected for learning
		PBB decapsulation*/
	GSW_PCE_INNER_MAC_SELECTED = 1,
} GSW_PCE_MacTableMacinMacSelect;

/** \brief PBB Action.
     Used by \ref GSW_PCE_action_t. */
typedef struct {
	/** Tunnel Template Index for I-Header Known traffic.
		\if DOC_ENABLE_PBB
		This field should be valid ID returned by
		\ref GSW_PBB_TunnelTempate_Alloc and c
		onfigured Using GSW_PBB_TunnelTempate_Config_Set
		\endif
	 */
	u8 nTunnelIdKnownTraffic;
	/** Tunnel Template Index for I-Header UnKnown traffic.
		\if DOC_ENABLE_PBB
		This field should be valid ID returned by
		\ref GSW_PBB_TunnelTempate_Alloc and
		configured Using GSW_PBB_TunnelTempate_Config_Set
		\endif
	 */
	u8 nTunnelIdUnKnownTraffic;
	/** Tunnel Template Index for B-TAG Known traffic.
		\if DOC_ENABLE_PBB
		This field should be valid ID returned by
		\ref GSW_PBB_TunnelTempate_Alloc and
		configured Using GSW_PBB_TunnelTempate_Config_Set
		\endif
	 */
	u8 nProcessIdKnownTraffic;
	/** Tunnel Template Index for B-TAG UnKnown traffic.
		\if DOC_ENABLE_PBB
		This field should be valid ID returned by
		\ref GSW_PBB_TunnelTempate_Alloc and
		Configured Using GSW_PBB_TunnelTempate_Config_Set
		\endif
	 */
	u8 nProcessIdUnKnownTraffic;
	/** I-Header Operation Mode*/
	GSW_PCE_IheaderOperationMode eIheaderOpMode;
	/** B-Tag Operation Mode*/
	GSW_PCE_BtagOperationMode eBtagOpMode;
	/** Action enable I-Header*/
	/**Select Mac Table MacinMac Action */
	GSW_PCE_MacTableMacinMacSelect eMacTableMacinMacSelect;
	/** Enable Mac-in-Mac (PBB) I-Header action */
	gsw_bool_t bIheaderActionEnable;
	/** Enable Tunnel Id for I-Header Known traffic*/
	gsw_bool_t bTunnelIdKnownTrafficEnable;
	/** Enable Tunnel Id for I-Header UnKnown traffic*/
	gsw_bool_t bTunnelIdUnKnownTrafficEnable;
	/** Incase of I-Header operation mode is Insertion
	    and bB_DstMac_FromMacTableEnable is enabled,
	    the B-DA is from Mac table instead of tunnel template*/
	gsw_bool_t bB_DstMac_FromMacTableEnable;
	/** Replace B-SA from tunnel template*/
	gsw_bool_t bReplace_B_SrcMacEnable;
	/** Replace B-DA from tunnel template*/
	gsw_bool_t bReplace_B_DstMacEnable;
	/** Replace I-Tag Res from tunnel template*/
	gsw_bool_t bReplace_I_TAG_ResEnable;
	/** Replace I-Tag UAC from tunnel template*/
	gsw_bool_t bReplace_I_TAG_UacEnable;
	/** Replace I-Tag DEI from tunnel template*/
	gsw_bool_t bReplace_I_TAG_DeiEnable;
	/** Replace I-Tag PCP from tunnel template*/
	gsw_bool_t bReplace_I_TAG_PcpEnable;
	/** Replace I-Tag SID from tunnel template*/
	gsw_bool_t bReplace_I_TAG_SidEnable;
	/** Replace I-Tag TPID from tunnel template*/
	gsw_bool_t bReplace_I_TAG_TpidEnable;
	/** Action enable B-TAG*/
	gsw_bool_t bBtagActionEnable;
	/** Enable Process Id for B-TAG Known traffic*/
	gsw_bool_t bProcessIdKnownTrafficEnable;
	/** Enable Process Id for B-TAG UnKnown traffic*/
	gsw_bool_t bProcessIdUnKnownTrafficEnable;
	/** Replace B-Tag DEI from tunnel template*/
	gsw_bool_t bReplace_B_TAG_DeiEnable;
	/** Replace B-Tag PCP from tunnel template*/
	gsw_bool_t bReplace_B_TAG_PcpEnable;
	/** Replace B-Tag SID from tunnel template*/
	gsw_bool_t bReplace_B_TAG_VidEnable;
	/** Replace B-Tag TPID from tunnel template*/
	gsw_bool_t bReplace_B_TAG_TpidEnable;
	/**Action enable Mac Table MacinMac*/
	gsw_bool_t bMacTableMacinMacActionEnable;
} GSW_PCE_ActionPBB_t;

/** \brief Sub-interface ID Action.
     Used by \ref GSW_PCE_action_t. */
typedef struct {
	/** Destination Sub IF ID Group Field Action Enable*/
	gsw_bool_t bDestSubIFIDActionEnable;
	/** Destination Sub IF ID Group Field Assignment Enable*/
	gsw_bool_t bDestSubIFIDAssignmentEnable;
	/** Destination Sub IF ID Group Field or LAG Index for truncking action*/
	u16 nDestSubIFGrp_Field;
} GSW_PCE_ActionDestSubIF_t;

/** \brief Packet Classification Engine Action Configuration.
    GSWIP-3.0 extension actions are explicitly indicated.
    Used by \ref GSW_PCE_rule_t. */
typedef struct	{
	/** Signed time compensation value for OAM delay measurement.
	  * The valid values are -4,294,967,295 ~ 4,294,967,295.
	  */
	long long nTimeComp;
	/** Extended VLAN block allocated for traffic match this flow entry. Valid
	    when bExtendedVlanEnable is TRUE. Only FIRST VLAN operation in this block
	    is used for flow process. */
	u16 nExtendedVlanBlockId;
	/** Insertion/Extraction Point */
	u8 nInsExtPoint;
	/** PTP Sequence ID for PTP application */
	u8 nPtpSeqId;
	/** Target portmap for forwarded packets, only used if selected by
	    'ePortMapAction'. Forwarding is done
	    if 'ePortMapAction = GSW_PCE_ACTION_PORTMAP_ALTERNATIVE'.
	    Every bit in the portmap represents one port (port 0 = LSB bit). */
	/** If both bNoPktUpdate and bAppendToPkt are false, this is byte offset (2~255) to insert counter or timestamp. */
	u16 nPktUpdateOffset;
	/** Traffic flow counter ID used for counter insertion in OAM loss measurement.
	  * This is least significant bits of traffic flow counter ID.
	  * The most significant bits depends on traffic flow counter mode.
	  * For example, if it's global mode, this is full traffic flow counter ID.
	  * If it's logical port mode, MSB is ingress logical port ID and LSB is 5 bits of this parameter.
	  * If it's CTP or BP (bridge port) mode, it depends on configuration.
	  */
	u16 nOamFlowId;
	/** Record ID is information used by extraction (bExtractEnable is set)
	  * and/or OAM process (bOamEnable is set). For GSWIP-3.1 and above.
	  * Refer to GSWIP-3.1 Hardware Architecture Spec (HAS) for more detail.
	  * This is deperated and should be set to ZERO.
	  **/
	u16 nRecordId;

	/** Target portmap for forwarded packets, only used if selected by
	    'ePortMapAction'. Forwarding is done
	    if 'ePortMapAction = GSW_PCE_ACTION_PORTMAP_ALTERNATIVE'.
	    Every bit in the portmap represents one bridge port (port 0 = LSB bit). */
	u16 nForwardPortMap[8];	/* max can be 16 */
	/** Counter ID (The index starts counting from zero). */
	u16	nRMON_Id;
	/** Alternative STAG VLAN Id */
	u16	nSVLAN_Id;
	/** Flow ID */
	u16	nFlowID;
	/** Routing Extension Id Value - for GSWIP-3.0 only. (8-bits range) */
	u16	nRoutExtId;
	/** Alternative Traffic class - used when eTrafficClassAction is set to 2. */
	u8	nTrafficClassAlternate;
	/** Meter ID */
	u8	nMeterId;
	/** Alternative CTAG VLAN Id */
	u8	nVLAN_Id;
	/** Alternative FID. Valid when bFidEnable is TRUE. */
	u8	nFId;
	/** Action "Traffic Class" Group.
	    Traffic class action enable */
	GSW_PCE_ActionTrafficClass_t	eTrafficClassAction;

	/** Action "IGMP Snooping" Group.
	    IGMP Snooping control and enable. Please note that the 'nPortMapAction'
	    configuration is ignored in case the IGMP snooping is enabled.
	    Here, on read operations,
	    'nPortMapAction = GSW_PCE_ACTION_PORTMAP_DISABLE' is returned. */
	GSW_PCE_ActionIGMP_Snoop_t	eSnoopingTypeAction;

	/** Action "Learning" Group.
	    Learning action control and enable */
	GSW_PCE_ActionLearning_t	eLearningAction;

	/** Action "Interrupt" Group.
	    Interrupt action generate and enable */
	GSW_PCE_ActionIrq_t	eIrqAction;

	/** Action "Cross State" Group.
	    Cross state action control and enable */
	GSW_PCE_ActionCrossState_t	eCrossStateAction;

	/** Action "Critical Frames" Group.
	    Critical Frame action control and enable */
	GSW_PCE_ActionCriticalFrame_t	eCritFrameAction;

	/** Action "Color Frames" Group.
	    This is replacement of eCritFrameAction in GSWIP-3.1. */
	GSW_PCE_ActionColorFrame_t eColorFrameAction;

	/** Action "Timestamp" Group. Time stamp action control and enable */
	GSW_PCE_ActionTimestamp_t	eTimestampAction;

	/** Action "Forwarding" Group.
	    Port map action enable. This port forwarding configuration is ignored
	    in case the action "IGMP Snooping" is enabled via the
	    parameter 'nSnoopingTypeAction'. */
	GSW_PCE_ActionPortmap_t	ePortMapAction;
	/** Action "Meter" Group. Meter action control and enable.
	    If metering action enabled, specified metering instance number
	    overrules any other metering assignment.
	    Up to two metering instances can be applied to a single packet. */
	GSW_PCE_ActionMeter_t	eMeterAction;
	/** Action "CTAG VLAN" Group. VLAN action enable */
	GSW_PCE_ActionVLAN_t	eVLAN_Action;
	/** Action "STAG VLAN" Group. VLAN action enable */
	GSW_PCE_ActionVLAN_t	eSVLAN_Action;
	/** Action "Cross VLAN" Group. Cross VLAN action enable */
	GSW_PCE_ActionCrossVLAN_t	eVLAN_CrossAction;
	/** Assignment of flow to MPE Processing Path-1 or -2 or both - for GSWIP-3.0 only. */
	GSW_PCE_ProcessingPathAction_t eProcessPath_Action;
	/** Port Filter Action Config for this flow - for GSWIP-3.0 only. */
	GSW_PCE_PortFilterAction_t	ePortFilterType_Action;
	/** Mac-in-Mac (PBB) Action Config */
	GSW_PCE_ActionPBB_t sPBB_Action;
	/** Sub-interface-ID Assignment Action */
	GSW_PCE_ActionDestSubIF_t sDestSubIF_Action;
	/** Action "Remarking" Group. Remarking action enable. Reserved in
	    GSWIP-3.1. */
	gsw_bool_t	bRemarkAction;
	/** CTAG VLAN PCP remarking enable. Reserved in GSWIP-3.1.
	    Remarking enabling means that remarking is possible in case
	    the port configuration or metering enables remarking on that
	    packet. Disabling remarking means that it is forced to
	    not remarking this packet, independent of any port remarking of
	    metering configuration. */
	gsw_bool_t	bRemarkPCP;
	/** STAG VLAN PCP remarking enable. Reserved in GSWIP-3.1.
	    Remarking enabling means that remarking is possible in case
	    the port configuration or metering enables remarking on that
	    packet. Disabling remarking means that it is forced to
	    not remarking this packet, independent of any port remarking of
	    metering configuration. */
	gsw_bool_t	bRemarkSTAG_PCP;
	/** STAG VLAN DEI remarking enable. Reserved in GSWIP-3.1.
	    Remarking enabling means that remarking is possible in case
	    the port configuration or metering enables remarking on that
	    packet. Disabling remarking means that it is forced to
	    not remarking this packet, independent of any port remarking of
	    metering configuration. */
	gsw_bool_t	bRemarkSTAG_DEI;
	/** DSCP remarking enable. Reserved in GSWIP-3.1.
	    Remarking enabling means that remarking is possible in case
	    the port configuration or metering enables remarking on that
	    packet. Disabling remarking means that it is forced to
	    not remarking this packet, independent of any port remarking of
	    metering configuration. */
	gsw_bool_t	bRemarkDSCP;
	/** Class remarking enable. Reserved in GSWIP-3.1.
	    Remarking enabling means that remarking is possible in case
	    the port configuration or metering enables remarking on that
	    packet. Disabling remarking means that it is forced to
	    not remarking this packet, independent of any port remarking of
	    metering configuration. */
	gsw_bool_t	bRemarkClass;

	/** Action "RMON" Group. RMON action enable. Reserved in GSWIP-3.1. */
	gsw_bool_t	bRMON_Action;
	/** Enable alternative FID */
	gsw_bool_t bFidEnable;
	/** Enable extended VLAN operation for traffic match this flow entry. */
	gsw_bool_t bExtendedVlanEnable;
	/**  CVLAN Ignore control */
	gsw_bool_t	bCVLAN_Ignore_Control;
	/** Port BitMap Mux control */
	gsw_bool_t	bPortBitMapMuxControl;
	/** Trunking action enable  */
	gsw_bool_t	bPortTrunkAction;
	/**  Port Link Selection control */
	gsw_bool_t	bPortLinkSelection; //NA for F48X. Can remove?
	/** Action "Flow ID".
	 The Switch supports enhancing the egress packets by a device specific
	 special tag header. This header contains detailed switch classification
	 results. One header file is a 'Flow ID', which can be explicitly set as
	 flow table action when hitting a table rule.
	 If selected, the Flow ID is given by the parameter 'nFlowID'. */
	gsw_bool_t	bFlowID_Action;
	/** Routing Extension Id Action Selector - for GSWIP-3.0 only.
	  When enabled, it expects a valid nRoutExtId value to be supplied. */
	gsw_bool_t	bRoutExtId_Action;
	/** Routing Destination Port Mask Comparison - for GSWIP-3.0 only. If not enabled this field is not considered for routing session pattern lookup.*/
	gsw_bool_t	bRtDstPortMaskCmp_Action;
	/** Routing Source Port Mask Comparison - for GSWIP-3.0 only. If not enabled, this field is not considered for routing session pattern lookup.*/
	gsw_bool_t	bRtSrcPortMaskCmp_Action;
	/** Routing Destination IP Address Mask Comparison - for GSWIP-3.0 only. If not enabled, this field is not considered for routing session pattern lookup.*/
	gsw_bool_t	bRtDstIpMaskCmp_Action;
	/** Routing Source IP Address Mask Comparison - for GSWIP-3.0 only. If not enabled, this field is not considered for routing session pattern lookup.*/
	gsw_bool_t	bRtSrcIpMaskCmp_Action;
	/** Selector of IP in Tunneled IP header (Outer or Inner) - for GSWIP-3.0 only. */
	gsw_bool_t	bRtInnerIPasKey_Action;
	/** Routing Acceleration Enable Action - for GSWIP-3.0 only. This variable decides whether to accelerate the Routing session or not */
	gsw_bool_t	bRtAccelEna_Action;
	/** Routing Control Enable Action - for GSWIP-3.0 only. This variable is selector of Routing Accelerate action*/
	gsw_bool_t	bRtCtrlEna_Action;
	/** Enable Extraction. For GSWIP-3.1 only.
	    Packet is identified to be extracted at extraction point defined by
	    nRecordId. */
	gsw_bool_t bExtractEnable;
	/** Enable OAM. For GSWIP-3.1 only.
	    Packet is identified for OAM process. */
	gsw_bool_t bOamEnable;
	/** Update packet after QoS queue (in PCE Bypass path). */
	gsw_bool_t bPceBypassPath;
	/** This is Tx flow counter, otherwise Rx flow counter is used in OAM loss measurement. */
	gsw_bool_t bTxFlowCnt;
	/** Timestamp format. */
	enum {
		GSW_PCE_DIGITAL_10B = 0,
		GSW_PCE_BINARY_10B = 1,
		GSW_PCE_DIGITAL_8B = 2,
		GSW_PCE_BINARY_8B = 3
	} eTimeFormat;
	/** Do not update packet */
	gsw_bool_t bNoPktUpdate;
	/** If bNoPktUpdate is false, this flag indicates to append counter or timestamp to end of packet. */
	gsw_bool_t bAppendToPkt;
	/** Configure PBB action. For GSWIP-3.2 only*/
	gsw_bool_t	bPBB_Action_Enable;
	/** For Enabling Dest SubIF ID Group field in TFLOW.Valid only For GSWIP-3.2 only*/
	gsw_bool_t	bDestSubIf_Action_Enable;
} GSW_PCE_action_t;

/** \brief Parameter to add/read a rule to/from the packet classification engine.
    Used by \ref GSW_PceRuleWrite and \ref GSW_PceRuleRead. */
typedef struct {
	/** Logical Port Id. The valid range is hardware dependent. */
	u8 logicalportid;
	/** Sub interface ID group,
	 *The valid range is hardware/protocol dependent.
	 */
	u16 subifidgroup;
	/** PCE TABLE Region */
	GSW_PCE_RuleRegion_t	region;

	/** PCE Rule Pattern Part. */
	GSW_PCE_pattern_t	pattern;
	/** PCE Rule Action Part. */
	GSW_PCE_action_t	action;
} GSW_PCE_rule_t;

/** \brief TRAFFIC FLOW TABLE  Allocation.
 *	Used by \ref GSW_PceRuleAlloc, \ref GSW_PceRuleFree and
 *	\ref GSW_PceRuleBlockSize.
 */
typedef struct {
	/** Number of traffic flow table entries are
	 * associated to CTP port.Ingress traffic from this CTP
	 *	port will go through PCE rules search ending at
	 *	(nFirstFlowEntryIndex+nNumberOfFlowEntries)-1. Should
	 *	be times of 4. Proper value should be given
	 *	for \ref GSW_PceRuleAlloc.
	 *	This field is output for \ref GSW_PceRuleFree and
	 *	\ref GSW_PceRuleBlockSize.
	 */
	u16 num_of_rules;
	/** If \ref GSW_PceRuleAlloc is successful, a valid ID will be returned
	 *  in this field. Otherwise, \ref INVALID_HANDLE is
	 *	returned in this field.
	 *  For \ref GSW_PceRuleFree, this field should be valid ID returned by
	 *  \ref GSW_PceRuleAlloc.
	 */
	u16 blockid;
} GSW_PCE_rule_alloc_t;

/** \brief Parameter to delete a rule from the packet classification engine.
    Used by \ref GSW_PceRuleDelete. */
typedef struct {
	/** Logical Port Id. The valid range is hardware dependent. */
	u8 logicalportid;
	/** Sub interface ID group,
	 *The valid range is hardware/protocol dependent.
	 */
	u16 subifidgroup;
	/** PCE TABLE Region */
	GSW_PCE_RuleRegion_t	region;

	/** Rule Index in the PCE Table. */
	u16 nIndex;
} GSW_PCE_ruleEntry_t;

/** \brief Parameter to move PCE rule to new entry.
 *  It copy the PCE rule from current entry to new entry and then delete it
 *  from current entry. Ths is used by \ref GSW_PceRuleMove. */
typedef struct {
	/** current PCE rule entry. */
	GSW_PCE_ruleEntry_t cur;
	/** new PCE rule entry */
	GSW_PCE_ruleEntry_t new;
} GSW_PCE_rule_move_t;

/** @}*/ /* GSW_PCE */

#pragma scalar_storage_order default
#pragma pack(pop)

#endif /* MXL_GSW_FLOW_H_ */
