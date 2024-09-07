/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _GSW_PMAC_H_
#define _GSW_PMAC_H_

#include "gsw_types.h"

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

/** \addtogroup GSW_PMAC
 *  @{
 */

/** \brief Configure the Backpressure mapping for egress Queues Congestion or ingress (receiving)  ports to DMA channel.
    Used by \ref GSW_PMAC_BM_CfgSet and \ref GSW_PMAC_BM_CfgGet. */
typedef struct {
	/** PMAC Interface ID */
	u8 nPmacId;
	/**  Tx DMA Channel Identifier which receives sideband backpressure signal (0..15) */
	u8 nTxDmaChanId;
	/** Transmit Queues Selection Mask which will generate backpressure - (Configurable upto 32 Egress Queues) */
	u32 txQMask;
	/** Receive (Ingress) ports selection congestion Mask which will generate backpressure - (Configurable upto 16 ports) */
	u32 rxPortMask;
} GSW_PMAC_BM_Cfg_t;

/** \brief Short Length Received Frame Check Type for PMAC.
    Used by PMAC structure \ref GSW_PMAC_Glbl_Cfg_t. */
typedef enum {
	/** Short frame length check is disabled. */
	GSW_PMAC_SHORT_LEN_DIS = 0,
	/** Short frame length check is enabled without considering VLAN Tags. */
	GSW_PMAC_SHORT_LEN_ENA_UNTAG = 1,
	/** Short frame length check is enabled including VLAN Tags. */
	GSW_PMAC_SHORT_LEN_ENA_TAG = 2,
	/** Reserved - Currently unused */
	GSW_PMAC_SHORT_LEN_RESERVED = 3
} GSW_PMAC_Short_Frame_Chk_t;

/** \brief Egress PMAC Config Table Selector */
typedef enum {
	/** Use value of \ref GSW_PMAC_Glbl_Cfg_t::bProcFlagsEgCfgEna */
	GSW_PMAC_PROC_FLAGS_NONE = 0,
	/** Use traffic class for egress config table addressing */
	GSW_PMAC_PROC_FLAGS_TC = 1,
	/** Use flags (MPE1, MPE2, DEC, ENC) for egress config table addressing */
	GSW_PMAC_PROC_FLAGS_FLAG = 2,
	/** Use reduced traffic class (saturated to 3) and flags (MPE1, MPE2) for
	    egress config table addressing */
	GSW_PMAC_PROC_FLAGS_MIX = 3
} GSW_PMAC_Proc_Flags_Eg_Cfg_t;

/** \brief Configure the global settings of PMAC for GSWIP-3.x. This includes settings such as Jumbo frame, Checksum handling,
    Padding and Engress PMAC Selector Config.
    Used by \ref GSW_PMAC_GLBL_CfgSet and \ref GSW_PMAC_GLBL_CfgGet. */
typedef struct {
	/** PMAC Interface Id */
	u8 nPmacId;
	/**  Automatic Padding Settings - Disabled (Default), to enable set it true. */
	gsw_bool_t bAPadEna;
	/**  Global Padding Settings - Disabled (Default), to enable set it true. */
	gsw_bool_t bPadEna;
	/**  VLAN Padding Setting - Disabled (Default), to enable set it true - applicable when bPadEna is set. */
	gsw_bool_t bVPadEna;
	/**  Stacked VLAN Padding Setting - Disabled (Default), to enable set it true - applicable when bPadEna is set. */
	gsw_bool_t bSVPadEna;
	/**  Packet carry FCS after PMAC process - Disabled (Default), to enable set it true. */
	gsw_bool_t bRxFCSDis;
	/**  Transmit FCS Regeneration Setting - Disabled (Default), to enable set it true. */
	gsw_bool_t bTxFCSDis;
	/**   IP and Transport (TCP/UDP) Headers Checksum Generation Control - Enabled (Default), to disable set it true. */
	gsw_bool_t bIPTransChkRegDis;
	/**   IP and Transport (TCP/UDP) Headers Checksum Verification Control - Enabled (Default), to disable set it true. */
	gsw_bool_t bIPTransChkVerDis;
	/**   To enable receipt of Jumbo frames - Disabled (Default - 1518 bytes normal frames without VLAN tags), to enable Jumbo set it true. */
	gsw_bool_t bJumboEna;
	/**   Maximum length of Jumbo frames in terms of bytes (Bits 13:0). The maximum handled in Switch is 9990 bytes.  */
	u16 nMaxJumboLen;
	/**   Threshold length for Jumbo frames qualification in terms of bytes (Bits 13:0).  */
	u16 nJumboThreshLen;
	/**   Long frame length check - Enabled (Default), to disable set it true.  */
	gsw_bool_t bLongFrmChkDis;
	/**   Short frame length check Type - default (Enabled for 64 bytes without considering VLAN).  */
	GSW_PMAC_Short_Frame_Chk_t eShortFrmChkType;
	/** GSWIP3.0 specific - Egress PMAC Config Table Selector - TrafficClass or Processing Flags (MPE1, MPE22, DEC, ENC based).
	    The default setting is Traffic Class based selector for Egress PMAC. */
	gsw_bool_t	bProcFlagsEgCfgEna;
	/** GSWIP3.1 specific - Egress PMAC Config Table Selector
	    If this field is not \ref GSW_PMAC_PROC_FLAGS_NONE, it will override
	    bProcFlagsEgCfgEna. */
	GSW_PMAC_Proc_Flags_Eg_Cfg_t eProcFlagsEgCfg;
	/** GSWIP3.1 specific - frame size threshold for buffer selection.
	    Value in this array should be in ascending order. */
	u32 nBslThreshold[3];
} GSW_PMAC_Glbl_Cfg_t;

/** \brief PMAC Ingress Configuration Source
    Source of the corresponding field. */
typedef enum {
	/** Field is from DMA descriptor */
	GSW_PMAC_IG_CFG_SRC_DMA_DESC = 0,
	/** Field is from default PMAC header */
	GSW_PMAC_IG_CFG_SRC_DEF_PMAC = 1,
	/** Field is from PMAC header of packet */
	GSW_PMAC_IG_CFG_SRC_PMAC = 2,
} GSW_PMAC_Ig_Cfg_Src_t;

/** \brief Configure the PMAC Ingress Configuration on a given Tx DMA channel to PMAC. (Upto 16 entries).
    This Ingress PMAC table is addressed through Trasnmit DMA Channel Identifier.
    Used by \ref GSW_PMAC_IG_CfgSet and \ref GSW_PMAC_IG_CfgGet. */
typedef struct {
	/** PMAC Interface Id */
	u8 nPmacId;
	/**  Tx DMA Channel Identifier (0..16) - Index of Ingress PMAC Config Table */
	u8 nTxDmaChanId;
	/** Error set packets to be discarded (True) or not (False) */
	gsw_bool_t bErrPktsDisc;
	/** Port Map info from default PMAC header (True) or incoming PMAC header (False) */
	gsw_bool_t bPmapDefault;
	/** Port Map Enable info from default PMAC header (True) or incoming PMAC header (False) */
	gsw_bool_t bPmapEna;
	/** Class Info from default PMAC header (True) or incoming PMAC header (False) */
	gsw_bool_t bClassDefault;
	/** Class Enable info from default PMAC header (True) or incoming PMAC header (False) */
	gsw_bool_t bClassEna;
	/** Sub_Interface Id Info from ingress PMAC header (GSW_PMAC_IG_CFG_SRC_PMAC),
	    default PMAC header (GSW_PMAC_IG_CFG_SRC_DEF_PMAC), or source sub-If in
	    packet descriptor (GSW_PMAC_IG_CFG_SRC_DMA_DESC) */
	GSW_PMAC_Ig_Cfg_Src_t eSubId;
	/**  Source Port Id from default PMAC header (True) or incoming PMAC header (False) */
	gsw_bool_t bSpIdDefault;
	/** Packet PMAC header is present (True) or not (False) */
	gsw_bool_t bPmacPresent;
	/** Default PMAC header - 8 Bytes Configuration  - Ingress PMAC Header Format */
	u8 defPmacHdr[8];
} GSW_PMAC_Ig_Cfg_t;

/** \brief Configure the PMAC Egress Configuration. (Upto 1024 entries)
    This Egress PMAC table is addressed through combination of following fields (Bit0 - Bit 9).
     nDestPortId (Bits 0-3) + Combination of [bMpe1Flag (Bit 4) + bMpe2Flag (Bit 5) + bEncFlag (Bit 6) + bDecFlag (Bit 7) ] or TrafficClass Value (Bits 4-7) + nFlowIdMSB (Bits 8-9).
    The bits 4-7 of index option is either based upon TC (default) or combination of Processing flags is decided through bProcFlagsEgPMACEna.
    It is expected to pass the correct value in bProcFlagsSelect same as global bProcFlagsEgPMACEna;
    Used by \ref GSW_PMAC_EG_CfgSet and \ref GSW_PMAC_EG_CfgGet. */
typedef struct {
	/** PMAC Interface ID */
	u8 nPmacId;
	/**  Destination Port Identifier (0..15) - Part of Table Index (Bits 0-3)*/
	u8 nDestPortId;
	/** Traffic Class value [Lower 4 -bits (LSB-0, 1, 2, 3)]. - Part of Table Index Bits 4-7.
	    This value is considered, only when bProcFlagsSelect is not set */
	u8 nTrafficClass;
	/**  MPE-1 Flag value - Part of Table Index Bit 4. Valid only when bProcFlagsSelect is set. */
	gsw_bool_t bMpe1Flag;
	/**  MPE-2 Flag value - Part of Table Index Bit 5. Valid only when bProcFlagsSelect is set. */
	gsw_bool_t bMpe2Flag;
	/**  Cryptography Decryption Action Flag value - Part of Table Index Bit 6. Valid only, when bProcFlagsSelect is set. */
	gsw_bool_t bDecFlag;
	/**  Cryptography Encryption Action Flag value - Part of Table Index Bit 7. Valid only, when bProcFlagsSelect is set. */
	gsw_bool_t bEncFlag;
	/**  Flow-ID MSB (2-bits) value -  valid range (0..2). - Part of Table Index Bits 8-9. */
	u8 nFlowIDMsb;
	/** Selector for Processing Flags (MPE1, MPE2, DEC & ENC bits). If enabled, then the combination of flags bDecFlag, bEncFlag, bMpe1Flag and  bMpe2Flag are considered as index instead of nTrafficClass. For using these combination flags, turn ON this boolean selector.
	TC or combination processing flag is decided at global level through bProcFlagsEgPMACEna.
	It is expected that user always passes correct value based upon bProcFlagsEgMPACEna. If mismatch found with global PMAC mode, SWAPI will return error code.

	    \remarks
	    In GSWIP-3.1, this is ignored and driver will determine automatically by
	    reading register.
	*/
	gsw_bool_t bProcFlagsSelect;
	/**  Receive DMA Channel Identifier (0..15) */
	u8 nRxDmaChanId;
	/** To remove L2 header & additional bytes (True) or Not (False) */
	gsw_bool_t bRemL2Hdr;
	/** No. of bytes to be removed after Layer-2 Header, valid when bRemL2Hdr is set */
	u8 numBytesRem;
	/** Packet egressing will have FCS (True) or Not (False) */
	gsw_bool_t bFcsEna;
	/** Packet egressing will have PMAC (True) or Not (False) */
	gsw_bool_t bPmacEna;
	/** Enable redirection flag. GSWIP-3.1 only.
	    Overwritten by bRes1DW0Enable and nRes1DW0. */
	gsw_bool_t bRedirEnable;
	/** Allow (False) or not allow (True) segmentation during buffer selection.
	    GSWIP-3.1 only. Overwritten by bResDW1Enable and nResDW1. */
	gsw_bool_t bBslSegmentDisable;
	/** Traffic class used for buffer selection. GSWIP-3.1 only.
	    Overwritten by bResDW1Enable and nResDW1. */
	u8 nBslTrafficClass;
	/** If false, nResDW1 is ignored. */
	gsw_bool_t bResDW1Enable;
	/** 4-bits Reserved Field in DMA Descriptor - DW1 (bit 7 to 4) - for any future/custom usage. (Valid range : 0-15) */
	u8 nResDW1;
	/** If false, nRes1DW0 is ignored. */
	gsw_bool_t bRes1DW0Enable;
	/** 3-bits Reserved Field in DMA Descriptor - DW0 (bit 31 to 29) - for any future/custom usage. (Valid range : 0-7) */
	u8 nRes1DW0;
	/** If false, nRes2DW0 is ignored. */
	gsw_bool_t bRes2DW0Enable;
	/** 2-bits Reserved Field in DMA Descriptor - DW0 (bit 14 to 13) - for any future/custom usage. (Valid range : 0-2) */
	u8 nRes2DW0;
	/** Selector for TrafficClass bits. If enabled, then the flags
	bDecFlag, bEncFlag, bMpe1Flag and  bMpe2Flag are not used instead nTrafficClass parameter is used. For using these flags turn off this boolean */
	gsw_bool_t bTCEnable;
} GSW_PMAC_Eg_Cfg_t;

/** \brief PMAC Counters available for specified DMA Channel.
    Used by \ref GSW_PMAC_CountGet. */
typedef struct {
	/** PMAC Interface ID Applicable only for GSWIP 3.1 */
	u8 nPmacId;
	/**  Transmit DMA Channel Identifier (0..15) for GSWIP3.0  (0..16) for GSWIP3.1 Source PortId for Egress Counters (0..15) for GSWIP3.1 - Index */
	u8 nTxDmaChanId;
	/** Ingress Total discarded packets counter (32-bits) */
	u32 nDiscPktsCount;
	/** Ingress Total discarded bytes counter (32-bits) */
	u32 nDiscBytesCount;
	/** Egress Total TCP/UDP/IP checksum error-ed packets counter (32-bits) */
	u32 nChkSumErrPktsCount;
	/** Egress Total TCP/UDP/IP checksum error-ed bytes counter (32-bits) */
	u32 nChkSumErrBytesCount;
	/** Total Ingress Packet Count in Applicable only for GSWIP 3.1 (32-bits) */
	u32 nIngressPktsCount;
	/** Total Ingress Bytes Count in Applicable only for GSWIP 3.1 (32-bits) */
	u32 nIngressBytesCount;
	/** Total Engress Packet Count in Applicable only for GSWIP 3.1 (32-bits) */
	u32 nEgressPktsCount;
	/** Total Engress Bytes Count in Applicable only for GSWIP 3.1 (32-bits) */
	u32 nEgressBytesCount;
	/** Ingress header Packet Count Applicable only for GSWIP 3.2 (32-bits) */
	u32 nIngressHdrPktsCount;
	/** Ingress header Byte Count Applicable only for GSWIP 3.2 (32-bits) */
	u32 nIngressHdrBytesCount;
	/** Egress header Packet Count Applicable only for GSWIP 3.2 (32-bits) */
	u32 nEgressHdrPktsCount;
	/** Egress header Byte Count Applicable only for GSWIP 3.2 (32-bits) */
	u32 nEgressHdrBytesCount;
	/** Egress header Discard Packet Count Applicable only for GSWIP 3.2 (32-bits) */
	u32 nEgressHdrDiscPktsCount;
	/** Egress header Discard Byte Count Applicable only for GSWIP 3.2 (32-bits) */
	u32 nEgressHdrDiscBytesCount;
} GSW_PMAC_Cnt_t;

/** @}*/ /* GSW_PMAC */

#pragma scalar_storage_order default
#pragma pack(pop)

#endif /*_GSW_PMAC_H_ */

