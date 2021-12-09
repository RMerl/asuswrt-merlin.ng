/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

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

#ifndef PHY_MAC_SEC_H_
#define PHY_MAC_SEC_H_

#include "phy_drv.h"
#include "phy_mac_sec_defs.h"
#include "phy_macsec_common.h"
#include "phy_macsec_api.h"

#define SHA_BLK_SIZE_IN_BYTE 32

typedef enum
{
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG
} phy_macsec_log_level_t;

/** SecY role: either egress only or ingress only */
typedef enum
{
    SECY_ROLE_EGRESS,             
    SECY_ROLE_INGRESS             
} secy_role_t;


typedef struct
{
    /** Reference handle to an SA */
    void * p;
} secy_SAHandle_t;

typedef struct
{
    /** Reference handle to an Rule */
    void * p;
} secy_RuleHandle_t;

typedef struct
{
    /** Reference handle to an vPort */
    void * p;
} secy_vPortHandle_t;

typedef enum
{
    MACSEC_PORT_TXMAC,              /**< \n */
    MACSEC_PORT_REDIRECTFIFO,       /**< \n */
    MACSEC_PORT_CAPTUREFIFO,        /**< \n */
    MACSEC_PORT_TXMACCAPTURE        /**< \n */
} macsec_port_type_t;

/**----------------------------------------------------------------------------
 * macsec_drop_type_t
 *
 * SA drop type:\n
 *   0 = bypass with CRC corruption signaling,\n
 *   1 = bypass with bad packet indicator,\n
 *   2 = internal drop by crypto-core (packet is not seen outside),\n
 *   3 = do not drop (for debugging only).
 */
typedef enum
{
    MACSEC_SA_DROP_CRC_ERROR,               /**< \n */
    MACSEC_SA_DROP_PKT_ERROR,               /**< \n */
    MACSEC_SA_DROP_INTERNAL,                /**< \n */
    MACSEC_SA_DROP_NONE                     /**< \n */
} macsec_drop_type_t;

/** Packet types */
typedef enum
{
    MACSEC_RULE_PKT_TYPE_OTHER  = 0        /**< untagged, VLAN, etc\n */
} macsec_rule_packet_type_t;

/** Frame types */
typedef enum
{
    MACSEC_RULE_FRAME_TYPE_OTHER  = 0,      /**< Ethernet II\n */
    MACSEC_RULE_FRAME_TYPE_ETH    = 1,      /**< \n */
    MACSEC_RULE_FRAME_TYPE_LLC    = 2,      /**< \n */
    MACSEC_RULE_FRAME_TYPE_SNAP   = 3       /**< \n */
} macsec_rule_frame_type_t;

/** Specify direction: egress (encrypt) or ingress (decrypt) */
typedef enum 
{
    MACSEC_SAB_DIRECTION_EGRESS, /**< \n */
    MACSEC_SAB_DIRECTION_INGRESS /**< \n */
} macsec_sa_builder_direction_t;


/** Operation type */
typedef enum 
{
    MACSEC_SAB_OP_MACSEC,            /**< MACsec operation (default) */
    MACSEC_SAB_OP_ENCAUTH_AES_GCM,   /**< Test operation for authenticate-encrypt. */
    MACSEC_SAB_OP_ENC_AES_CTR       /**< Test operation for encryption. */
} macsec_sa_builder_operation_t;

/*----------------------------------------------------------------------------
 * List_Element_t
 *
 * The element data structure.
 *
 */
typedef struct
{
    /* Pointer to a data object associated with this element, */
    /* can be filled in by the API user */
    void * DataObject_p;

    /* Data used internally by the API implementation only, */
    /* may not be written by the API user */
    void * Internal[LIST_INTERNAL_DATA_SIZE];

} List_Element_t;

/*----------------------------------------------------------------------------
 * List_Status_t
 *
 * Return values for all the API functions.
 */
typedef enum
{
    LIST_STATUS_OK,
    LIST_ERROR_BAD_ARGUMENT,
    LIST_ERROR_INTERNAL
} List_Status_t;

/* vPort, SC to SA mapping type */
typedef enum
{
    /* SA has been detached from its SC/vPort, either by direct
       replacement or by chaining; or no SA record is used (bypass or drop).*/
    SECY_SA_MAP_DETACHED,
    /* Egress mapping, SC mapped to single SA*/
    SECY_SA_MAP_EGRESS,
    /* Egress mapping, SC mapped to single SA, for CRYPT_AUTH */
    SECY_SA_MAP_EGRESS_CRYPT_AUTH,
    /* Ingress mapping, RxCAM used, SC mapped to up to four SAs */
    SECY_SA_MAP_INGRESS,
    /* Ingress Crypt Auth, RxCAM usd, same SA is mapped four times to SC */
    SECY_SA_MAP_INGRESS_CRYPT_AUTH,
} secy_sa_type_t;

typedef enum 
{
    macsecPortLocDC = 0, /**< Loc/side don't care */
    macsecPortLocLine, /**< Loc/line side */
    macsecPortLocSys, /**< Loc/system side */
    macsecPortCount
} macsec_port_loc_t;

/* SA descriptor */
typedef union
{
    struct 
    {
        uint32_t SAIndex;
        uint32_t SCIndex;
        uint32_t SAMFlowCtrlIndex;
        uint32_t AN;
        secy_sa_type_t MapType;
        uint32_t Magic;
    } InUse;

    List_Element_t free;
} secy_sa_descriptor_t;

/* SC descriptor */
typedef struct
{
    secy_SAHandle_t SAHandle[4];
    uint32_t SCIndex;
    uint8_t SCI[8];
    secy_sa_type_t MapType;
    uint32_t    Magic;
    List_Element_t OnList; /* Either free list of per-vPort list. */
} secy_sc_descriptor_t;

/* vPort Descriptor */
typedef struct
{
    struct 
    {
        uint32_t vPortId;
        uint32_t SACount; /* Number of active SA records. */
        void *SCList; /* plp_longfin_List of inbound SC's associated with this vPort. */
        /* Number of rules associated with this vPortId */
        uint32_t BoundRulesCount;
        uint32_t Magic;
    } InUse;
    List_Element_t free;
} secy_vport_descriptor_t;

/* Rule descriptor */
typedef union
{
    struct 
    {
        uint32_t RuleId;
        /* Pointer to the matching vPort for this rule */
        secy_vport_descriptor_t * vPortDscr_p;
        uint32_t Magic;
    } InUse;
    List_Element_t free;
} secy_rule_descriptor_t;

typedef struct 
{
    uint32_t            base_addr;
    phy_dev_t           *phy_dev;
    //uint32_t            SyncCount;
    uint32_t            MaxSACount;
    uint32_t            MaxSCCount;
    uint32_t            MaxvPortCount;
    uint32_t            MaxRuleCount;
    uint32_t            MaxSACounters;
    uint32_t            MaxGlobalCounters;
    uint32_t            MaxVLANCounters;
    uint8_t             fMatchSCI;
} secy_device_access_t;

/* XtSecY device data structure */
typedef struct
{
    void * SAFL_p; /* SA Free plp_longfin_List instance pointer */
    secy_sa_descriptor_t * SADscr_p;
    void * SCFL_p; /* Secure Channel Free plp_longfin_List instance pointer */
    secy_sc_descriptor_t *SCDscr_p;
    void * vPortFreeList_p;
    secy_vport_descriptor_t *vPortDscr_p;
    uint8_t *vPortListHeads_p;
    void * RuleFreeList_p;
    secy_rule_descriptor_t *RuleDscr_p;
    uint32_t SACount;
    uint32_t SCCount;
    uint32_t vPortCount;
    uint32_t RuleCount;
    uint32_t SACounters;
    uint32_t VLANCounters;
    uint32_t GlobalCounters;
    secy_role_t Role;
    secy_device_access_t secy_device_access;
    uint8_t fInitialized;
    secy_vPortHandle_t  vport_handlers[VPORT_NUM_MAX];
    secy_RuleHandle_t   rule_handlers[SA_NUM_MAX];
    secy_SAHandle_t     sa_handlers[RULE_NUM_MAX];
} secy_device_t;

/* Validate frame values, */
typedef enum
{
    macsec_VALIDATE_FRAME_DISABLED,
    macsec_VALIDATE_FRAME_CHECK,
    macsec_VALIDATE_FRAME_STRICT
} macsec_ValidateFrame_t;

/* Generic EIP HW version */
typedef struct
{
    /* The basic EIP number. */
    uint8_t EipNumber;

    /* The complement of the basic EIP number. */
    uint8_t ComplmtEipNumber;

    /* Hardware Patch Level. */
    uint8_t HWPatchLevel;

    /* Minor Hardware revision. */
    uint8_t MinHWRevision;

    /* Major Hardware revision. */
    uint8_t MajHWRevision;
} macsec_Version_t;

/* Configuration options of EIP-160 HW */
typedef struct
{

    /* Number of supported SA Counters */
    uint16_t    SA_Counters;

    /* Number of supported Vlan counters */
    uint16_t    VLAN_Counters;

    /* Number of Global Counters */
    uint16_t     Global_Counters;

    /* Number of SA's */
    uint16_t    SA_Count;

    /* Number of SA's */
    uint16_t    SC_Count;

    /* Number of vPorts */
    uint16_t    vPort_Count;

    /* Number of SA's */
    uint16_t    Rule_Count;

    /* Boolean for Matching SCI */
    uint8_t        fMatchSCI;

    /* Specialize the engine for ingress functionality and counters only, */
    /* true for -i, false for -e, false for -ie. */
    uint8_t        fIngressOnly;

    /* Specialize the engine for egress functionality and counters only, */
    /* false for -i, true for -e, false for -ie. */
    uint8_t        fEgressOnly;
} macsec_EIP160_Options_t;

/* Configuration options of EIP-160 HW */
typedef struct
{
    /* false if this is not a core optimized for FPGA implementation */
    uint8_t        fFPGASolution;

    /* false if "Galois Field" S-boxes are not used in the AES cores */
    uint8_t        fGFSboxes;

    /* true if "Lookup table" S-boxes are used in the AES cores */
    uint8_t        fLookupSboxes;

    /* true when this core only includes AES as crypto-algorithm */
    uint8_t        fMACsecAESOnly;

    /* AES cores are available */
    uint8_t        fAESPresent;

    /* false when no feedback modes are available for the AES cores */
    uint8_t        fAESFb;

    /* The number of AES engines operating in parallel: */
    /* 1 for an "s" configuration, */
    /* 4 for an "a" configuration, */
    /* 7 for a "b" configuration and */
    /* 14 for "c" configuration */
    /* 15 for "d" configuration */
    uint8_t     AESSpeed;

    /* 0 only AES-128 is supported */
    /* 1 only AES-192 is supported (a, b, c configurations) */
    /* 2 only AES-256 is supported */
    uint8_t     KeyLengths;

    /* Number of "parameter" bits passed through the EIP-62 pipeline */
    /* with the end-of-packet, 2 here */
    uint8_t     EopParamBits;

    /* true when IPSec is supported */
    uint8_t        fIPSec;

    /* true when HW support for header-offset */
    uint8_t        fHdrExtension;

    /* true when HW support for tag by-passing */
    uint8_t        fSecTAGOffset;

    /* true when GHASH core is available */
    uint8_t        fGHASHPresent;

    /* Speed indicator for GHASH core: */
    /* false when */
    /*     2-cycle core ("a" configuration) or 3-cycle core "s" configuration) */
    /* true when 1-cycle core ("b", "c" and "d" configurations). */
    uint8_t        fOneCycleCore;
} macsec_EIP62_Options_t;

/* Configuration options of EIP-217 device */
typedef struct
{
    /* Number of supported TCAM hit counters */
    uint16_t    TCAMHitCounters_Count;

    /* TCAM hit counters width (in bits) */
    uint8_t     TCAMHitCountersWidth_BitCount;

    /* Number of supported TCAM hit packet counters */
    uint8_t     TCAMHitPktCounters_Count;

    /* Number of supported TCAM hit octet counters */
    uint8_t     TCAMHitByteCounters_Count;

    /* Number of supported global TCAM hit packet counters */
    uint8_t     TCAMHitPktCountersGlobal_Count;

    /* Number of supported global TCAM hit octet counters */
    uint8_t     TCAMHitByteCountersGlobal_Count;

} macsec_TCAM_Options_t;

/* Capabilities structure for EIP-160 HW */
typedef struct
{
    /* EIP-160 */
    macsec_EIP160_Options_t    EIP160_Options;
    macsec_Version_t           EIP160_Version;

    /* Processing Engine */
    macsec_EIP62_Options_t     EIP62_Options;
    macsec_Version_t           EIP62_Version;

    /* EIP-217 TCAM */
    macsec_TCAM_Options_t      TCAM_Options;
    macsec_Version_t           TCAM_Version;

} macsec_Capabilities_t;

/* Types of ports */
typedef enum
{
    macsec_PORT_TXMAC,
    macsec_PORT_REDIRECTFIFO,
    macsec_PORT_CAPTUREFIFO,
    macsec_PORT_TXMACCAPTURE
} macsec_PortType_t;

/*----------------------------------------------------------------------------
 * macsec_SecY_DropType_t
 *
 * SA drop type:
 *      bypass with CRC error,
 *      bypass with packet error,
 *      internal drop by crypto device.
 *
 */
typedef enum
{
    macsec_SECY_SA_DROP_CRC_ERROR,
    macsec_SECY_SA_DROP_PKT_ERROR,
    macsec_SECY_SA_DROP_INTERNAL,
    macsec_SECY_SA_DO_NOT_DROP          /* For debug purposes only */
} macsec_SecY_SA_DropType_t;

/* MACsec SecTAG parsing rule, */
/* This rule classifies each packet into one of four categories, */
/* see also the macsec_Rules_SA_NonMatch_t data structure: */
/* 1) Untagged, the packet has no MACsec tag, */
/*    i.e. the EtherType differs from 0x88E5. */
/* 2) Bad tag, the packet has an invalid MACsec tag */
/* 3) KaY tag, the packet has a KaY tag. These packets are generated and/or */
/*    handled by application software and no MACsec processing is performed */
/*    for them by the Classification device except for straight bypass. */
/* 4) Tagged, the packet has a valid MACsec tag that is not KaY. */
typedef struct
{
    /* Compare EtherType in packet against EtherType value. */
    /* false - all packets are treated as MACsec */
    /*         (no packets are classified as untagged). */
    /* default is true */
    uint8_t fCompEType;

    /* true - check V bit to be 0 */
    /* default is true */
    uint8_t fCheckV;

    /* true - check if this is a KaY packet (C and E bits) */
    /* default is true */
    uint8_t fCheckKay;

    /* true - check illegal C and E bits combination (C=1 and E=0) */
    /* default is true */
    uint8_t fCheckCE;

    /* true - check illegal SC/ES/SCB bits combinations */
    /* default is true */
    uint8_t fCheckSC;

    /* true - check if SL (Short Length) field value is out of range */
    /* default is true */
    uint8_t fCheckSL;

    /* true - check PN (Packet Number) is non-zero */
    /* default is true */
    uint8_t fCheckPN;

    /* true - extended SL (Short length) check enabled. */
    uint8_t fCheckSLExt;

    /* EtherType value used for MACsec tag type comparison, */
    /* default is 0x88E5 */
    uint16_t EtherType;

} macsec_Rule_SecTAG_t;

/* Actions per packet that did not match any SA */
typedef struct
{
    /* Flow action type */
    /* true - bypass, */
    /* false - perform drop action, see macsec_SecY_SA_DropType_t */
    uint8_t fBypass;

    /* Packet drop type, see macsec_SecY_SA_DropType_t */
    macsec_SecY_SA_DropType_t DropType;

    /* Ingress only */
    /* Perform drop action if packet is not from the reserved port */
    uint8_t fDropNonReserved;

    macsec_PortType_t DestPort;

    /* 4 bit Capture reason code */
    uint8_t CaptureReason;

} macsec_Rules_NonSA_t;

/* Actions per packet type for packets which did not match any SA, */
/* see macsec_Rule_SecTAG_t rules which are used to classify packets into these */
/* categories */
typedef struct
{
    macsec_Rules_NonSA_t Untagged;
    macsec_Rules_NonSA_t Tagged;
    macsec_Rules_NonSA_t BadTag;
    macsec_Rules_NonSA_t KaY;
} macsec_Rules_SA_NonMatch_t;

/* TCAM initialization data structure */
typedef struct
{
    /* Pointer to memory location where an array of 32-bit words */
    /* is stored. This array will be copied to the TCAM memory during */
    /* the EIP-160 device initialization. */
    uint32_t * InitData_p;

    /* Size of the InitData_p array in 32-bit words */
    uint32_t InitData_WordCount;

    /* 32-bit word offset in TCAM where the InitData_p array must be written */
    uint32_t WordOffset;

} macsec_TCAM_Init_t;

/* Error drop flow settings */
typedef struct
{
    /* 1b = Drop frame on input packet error detect. */
    uint8_t fDropOnPktErr;

    /* 1b = Drop frame on input CRC error detect. */
    uint8_t fDropOnCrcErr;

    /* Defines the way the drop operation for header parser and (short) */
    /* input packet/CRC error frames is performed. */
    macsec_SecY_SA_DropType_t ErrDropAction;

    /* Dest port for header parser and (short) input packet/CRC error frames */
    /* if not internal dropping. */
    macsec_PortType_t ErrDestPort;

    /* Capture reason for header parser and (short) input packet/CRC error */
    /* frames if not internal dropping */
    uint8_t ErrCaptReason;

} macsec_ErrorDrop_Settings_t;

/* The MACSEC_ID_CTRL register is intended for the ingress-only use-case, */
/*    where the Packet Number in the frame can be replaced with a MACsec ID */
/*    when the SC is not found. (in case the SC is found, the MACsec ID */
/*    replacement can be controlled from the attached SA). */
/*    Note that this requires the vPort policy to retain the SecTag. */
typedef struct
{
    /* Enable replacing PN with MACSec ID value in retained SecTag. */
    uint8_t fMACsecIDEnb;

    /* MACSec ID value to replace PN with in retained SecTag if */
    /* SC is not found. */
    uint16_t MACsecID;

} macsec_MACsecID_Ctrl;

/* Store and Forward buffer control */
typedef struct
{
    /* 1b = Enable store and forward mode. */
    /* 0b = Disable store and forward mode. Engine operates in cut-through */
    /* mode and frames cannot be dropped by post-processing */
    /* (i.e. the other bits in this register are ignored). */
    uint8_t fEnable;

    /* Enable dropping on a classification error. */
    uint8_t fDropClass;

    /* Enable dropping on a post-processing error. */
    uint8_t fDropPP;

    /* Enable dropping on a security failure. */
    uint8_t fDropSecFail;

    /* Enable dropping on a MTU overflow detection */
    /* (egress-capable configuration only). */
    uint8_t fDropMTU;

    /* Enable dropping on an input MAC packet error detection. */
    uint8_t fDropMACErr;

    /* Enable dropping on an input MAC CRC error detection. */
    uint8_t fDropMACCRC;

    /* Low watermark value in 128-bit word (16 byte) units. */
    /* When FIFO fill level becomes greater than or equal to this value, */
    /* the saf_lwm output signal is set to 1b. Reset value is 50% */
    /* of the SAF buffer depth. */
    uint16_t LowWatermark;

    /* High watermark value in 128-bit word (16 byte) units. */
    /* When FIFO fill level becomes greater than or equal to this value, */
    /* the saf_hwm output signal is set to 1b. */
    /* Reset value is 75% of the SAF buffer depth. */
    uint16_t HighWatermark;
} macsec_SAF_Ctrl_t;

/* Context update control */
typedef struct
{
    /* 0b = strict comparison */
    /* 1b = greater or equal comparison */
    uint8_t fThresholdMode;

    /* Threshold value used to trigger a context update through the */
    /* context update interface */
    uint8_t UpdateThresholdValue;
} macsec_Ctx_Update_Ctrl_t;

/* Device settings */
typedef struct
{
    /* Device operation mode */
    secy_role_t Mode;

    /* When true the statistics counters are automatically reset */
    /* on a read operation */
    uint8_t fAutoStatCntrsReset;

    /* This mask specifies which SA-related counter increments are regarded a */
    /* security fail event - bit [0] is for the first 64-bits counter of an */
    /* SA-related counter set. */
    /* Note: Actual width depends on the number of SA counters implemented. */
    uint16_t SA_SecFail_Mask;

    /* This mask specifies which SecY counter increments are regarded a */
    /* security fail event - bit [0] is for the first 64-bits global counter. */
    /* Note: Actual width depends on the number of SecY counters implemented. */
    uint32_t SecY_SecFail_Mask;

    /* This mask specifies which global counter increments are regarded a */
    /* security fail event - bit [0] is for the first 64-bits global counter. */
    /* Note: Actual width depends on the number of global counters implemented. */
    uint32_t Global_SecFail_Mask;

    /* The fixed packet latency, if set to 0 then no fixed latency will be used */
    /* Latency + 4 = engine clock cycles */
    /* NOTE: do not set this value above 26 when setting the fStaticBypass. */
    uint16_t Latency;

    /* Number of words to keep in buffer for dynamic latency control */
    uint8_t DynamicLatencyWords;

    /* Enable dynamic latency control. */
    uint8_t fDynamicLatency;

    /* Disable the MACsec crypto-core (EIP-62), */
    /* send the packets around it to minimize latency */
    uint8_t fStaticBypass;

    /* Outbound sequence number threshold value (one global for all SA's) */
    /* When non-0 the device will generate an interrupt to indicate */
    /* the threshold event which can be used to start the re-keying procedure. */
    /* If set to zero then only the sequence number roll-over interrupt */
    /* will be generated. */
    uint32_t SeqNrThreshold;

    /* Outbound sequence number threshold value for 64-bit packet numbering */
    uint32_t SeqNrThreshold64Lo;
    uint32_t SeqNrThreshold64Hi;

    /* Threshold for the frame counters */
    uint32_t SACountFrameThrLo;
    uint32_t SACountFrameThrHi;
    uint32_t SecYCountFrameThrLo;
    uint32_t SecYCountFrameThrHi;
    uint32_t IFCCountFrameThrLo;
    uint32_t IFCCountFrameThrHi;
    uint32_t RxCAMCountFrameThrLo;
    uint32_t RxCAMCountFrameThrHi;
    uint32_t TCAMCountFrameThrLo;
    uint32_t TCAMCountFrameThrHi;

    /* Threshold for the octet counters */
    uint32_t SACountOctetThrLo;
    uint32_t SACountOctetThrHi;
    uint32_t IFCCountOctetThrLo;
    uint32_t IFCCountOctetThrHi;

    /* Prescale value for the time stamp tick counter */
    uint32_t GLTimePrescale;
    /* Time tick value used for timestamping the SAs */
    uint32_t GLTimeTick;

    /* MACsec ID Control */
    macsec_MACsecID_Ctrl MACsecIDCtrl;

    /* Initial processing rules for default/bypass flows (all vPorts) */
    macsec_Rules_SA_NonMatch_t NonControlDropBypass;
    macsec_Rules_SA_NonMatch_t ControlDropBypass;

    /* Error Drop flow settings */
    macsec_ErrorDrop_Settings_t ErrorDropFlow;

    /* Store and Forward buffer control */
    macsec_SAF_Ctrl_t SAFControl;

    /* When true device will be initialized for low-latency bypass mode */
    uint8_t fBypassNoclass;

    /* Context update control */
    macsec_Ctx_Update_Ctrl_t UpdateCtrl;

    /* Counter increment enable control */
    uint8_t CountIncDisCtrl;

    /* TCAM initialization data */
    macsec_TCAM_Init_t TCAMInit;

    uint32_t CPMatchEnableMask;

} macsec_settings_t;

typedef enum
{
    SAB_DIRECTION_EGRESS, /**< \n */
    SAB_DIRECTION_INGRESS /**< \n */
} SABuilder_Direction_t;

typedef enum
{
    SAB_OP_MACSEC,            /**< MACsec operation (default) */
    SAB_OP_ENCAUTH_AES_GCM,   /**< Test operation for authenticate-encrypt. */
    SAB_OP_ENC_AES_CTR,       /**< Test operation for encryption. */
} SABuilder_Operation_t;

/* Data structure to represent offsets of various fields.  If an
   offset is zero, the corresponding field is not present. */
typedef struct {
    uint8_t KeyOffs;
    uint8_t HKeyOffs;
    uint8_t SeqNumOffs;
    uint8_t MaskOffs;
    uint8_t CtxSaltOffs;
    uint8_t IVOffs;
    uint8_t UpdateCtrlOffs;
    uint8_t MTUOffs;
    uint8_t MaxOffs;
} SABuilder_Offsets_t;

/** Input parameters for the SA Builder */
typedef struct
{
    /** flags, either 0 or the bitwise or of one or more of the SAB_MACSEC_FLAG values*/
    uint32_t flags;
    /** Direction, egress or ingress */
    SABuilder_Direction_t direction;
    /** Operation type */
    SABuilder_Operation_t operation;
    /** AN inserted in SecTAG (egress). */
    uint8_t AN;
    /** MACsec Key. */
    uint8_t *Key_p;
    /** Size of the MACsec key in bytes. */
    uint32_t KeyByteCount;
    /** authentication key, derived from MACsec key. */
    uint8_t *HKey_p;
    /** 12-byte salt (64-bit sequence numbers). */
    uint8_t *Salt_p;
    /** 4-byte SSCI value (64-bit sequence numbers).*/
    uint8_t *SSCI_p;
    /** 8-byte SCI.*/
    uint8_t *SCI_p;
    /** sequence number.*/
    uint32_t SeqNumLo;
    /** High part of sequence number (64-bit sequence numbers)*/
    uint32_t SeqNumHi;
    /** Size of the replay window (ingress).*/
    uint32_t WindowSize;
    /** digest length for ENCAUTH operation only.*/
    uint32_t ICVByteCount;
} SABuilder_Params_t;

typedef struct
{
    /** Set to true if the SA must be updated. */
    uint8_t fUpdateEnable;
    /** SA Index field is a valid SA. */
    uint8_t fSAIndexValid;
    /** True if SA expired IRQ is to be generated.*/
    uint8_t fExpiredIRQ;
    /** Transfer timestamp to next SA. */
    uint8_t fUpdateTime;
    /** AN of an ingress SA.*/
    uint8_t AN;
    /** SA index of the next chained SA (egress).*/
    uint32_t SAIndex;
    /** SC index where SA is stored.*/
    uint32_t SCIndex;
} SABuilder_UpdCtrl_Params_t;


/** SA parameters for Egress action type */
typedef struct
{
    /** true - SA is in use, packets classified for it can be transformed\n
        false - SA not in use, packets classified for it can not be
                transformed */
    uint8_t fsa_inuse;

    /** The number of bytes (in the range of 0-127) that are authenticated\n
        but not encrypted following the SecTAG in the encrypted packet.\n
        Values 65-127 are reserved in HW < 4.0 and should not be used there.*/
    uint8_t confidentiality_offset;

    /** true - enable frame protection,\n
        false - bypass frame through device */
    uint8_t fprotect_frames;

    /** true - inserts explicit SCI in the packet,\n
        false - use implicit SCI (not transferred) */
    uint8_t finclude_sci;

    /** true - enable ES bit in the generated SecTAG\n
        false - disable ES bit in the generated SecTAG */
    uint8_t fuse_es;

    /** true - enable SCB bit in the generated SecTAG\n
        false - disable SCB bit in the generated SecTAG */
    uint8_t fuse_scb;

    /** true - enable confidentiality protection\n
        false - disable confidentiality protection */
    uint8_t fconf_protect;

    /** true - allow data (non-control) packets.\n
        false - drop data packets.*/
    uint8_t fallow_data_pkts;

    /** true - EoMPLS packet contains a 4-byte control word.\n
        false - EoMPLS packet does not contain a 4-byte control word.*/
    uint8_t fEoMPLS_ctrl_word;

    /** true - (Eo)MPLS packets are assumed to be EoMPLS packets.\n
        false - (Eo)MPLS packets are assumed to be MPLS packets. */
    uint8_t fEoMPLS_subport;

    /** true - Use value of SL field to strip Ethernet padding.\n
        false - Do not use value of SL field to strip Ethernet padding. */
    uint8_t fsl_pad_strip_enb;

    /** true - take e & c bits from ST-VLAN tag.\n
        false - e & c bits are calculated. */
    uint8_t fec_from_st_vlan;

    /** Specifies number of bytes from the start of the frame
        to be bypassed without MACsec protection */
    uint8_t pre_sectag_auth_start;

    /** Specifies number of bytes to be authenticated in the pre-SecTAG area.*/
    uint8_t pre_sectag_auth_length;

    /** Offset of the SecTag */
    uint8_t sectag_offset;

} macsec_sa_e_t;

/** SA parameters for Ingress action type */
typedef struct
{
    /** true - SA is in use, packets classified for it can be transformed\n
        false - SA not in use, packets classified for it can not be
                transformed */
    uint8_t fsa_inuse;

    /** The number of bytes (in the range of 0-127) that are authenticated\n
        but not encrypted following the SecTAG in the encrypted packet.\n
        Values 65-127 are reserved in hardware < 4.0 and should not be
        used there. */
    uint8_t confidentiality_offset;

    /** true - enable replay protection\n
        false - disable replay protection */
    uint8_t freplay_protect;

    /** MACsec frame validation level (tagged). */
    macsec_validate_frames_t validate_frames_tagged;

    /** SCI to which ingress SA applies (8 bytes). */
    uint8_t *sci_p;

    /** Association number to which ingress SA applies. */
    uint8_t an;

    /** true - allow tagged packets.\n
        false - drop tagged packets.*/
    uint8_t fallow_tagged;

    /** true - allow untagged packets.\n
        false - drop untagged packets. */
    uint8_t fallow_untagged;

    /** true - enable validate untagged packets.\n
        false - disable validate untagged packets.*/
    uint8_t fvalidate_untagged;

    /** true is EOMPLS */
    uint8_t fEOMPLS;

    /** pre-Sectag Auth start  */
    uint8_t pre_sectag_auth_start;

    /** pre-Sectag Auth length */
    uint8_t pre_sectag_auth_length;

    /** Sectag offset */
    uint8_t sectag_offset;

    /** true - EoMPLS packet contains a 4-byte control word.\n
        false - EoMPLS packet does not contain a 4-byte control word.*/
    uint8_t fEoMPLS_ctrl_word;

    /** true - (Eo)MPLS packets are assumed to be EoMPLS packets.\n
        false - (Eo)MPLS packets are assumed to be MPLS packets. */
    uint8_t fEoMPLS_subport;

    /** true is Mac in Mac */
    uint8_t fmac_icmac;

    /** For situations when RxSC is not found or SAinUse=0 with validation
        level that allows packet to be sent to the Controlled port with the
        SecTAG/ICV removed, this flag represents a policy to allow SecTAG
        retaining.\n
        true - SecTAG is retained. */
    uint8_t fretain_sectag;

    /** true - ICV is retained (allowed only when fRetainSecTAG is true). */
    uint8_t fretain_icv;

} macsec_sa_i_t;

/** SA parameters for Bypass/Drop action type */
typedef struct
{
    /** true - enable statistics counting for the associated SA\n
       false - disable statistics counting for the associated SA */
    uint8_t fsa_inuse;

} macsec_sa_bd_t;

/** SA parameters for Crypt-Authenticate action type */
typedef struct
{
    /** true - message has length 0\n
        false - message has length > 0 */
    uint8_t fzero_length_message;

    /** The number of bytes (in the range of 0-255) that are authenticated but
        not encrypted (AAD length). */
    uint8_t confidentiality_offset;

    /** IV loading mode:\n
        0: The IV is fully loaded via the transform record.\n
        1: The full IV is loaded via the input frame. This IV is located in
           front of the frame and is considered to be part of the bypass data,
           however it is not part to the result frame.\n
        2: The full IV is loaded via the input frame. This IV is located at the
           end of the bypass data and is considered to be part of the bypass
           data, and it also part to the result frame.\n
        3: The first three IV words are loaded via the input frame, the counter
           value of the IV is set to one. The three IV words are located in
           front of the frame and are considered to be part of the bypass data,
           however it is not part to the result frame. */
    uint8_t iv_mode;

    /** true - append the calculated ICV\n
        false - don't append the calculated ICV */
    uint8_t ficv_append;

    /** true - enable ICV verification\n
        false - disable ICV verification */
    uint8_t ficv_verify;

    /** true - enable confidentiality protection (AES-GCM/CTR operation)\n
        false - disable confidentiality protection (AES-GMAC operation) */
    uint8_t fconf_protect;

} macsec_sa_ca_t;

typedef union
{
    macsec_sa_e_t egress;
    macsec_sa_i_t ingress;
    macsec_sa_bd_t bypass_drop;
    macsec_sa_ca_t crypt_auth;
} macsec_sa_action_t;

/** vPort matching rule Key/Mask data structure */
typedef struct
{
    /** rule Packet type */
    macsec_rule_packet_type_t packet_type;

    /** Bit 0 = 1 : No (ST)VLAN tags\n
        Bit 1 = 1 : 1 (ST)VLAN tag\n
        Bit 2 = 1 : 2 VLAN tags\n
        Bit 3 = 1 : 3 VLAN tags\n
        Bit 4 = 1 : 4 VLAN tags\n
        Bit 5 = 1 : 5 Reserved\n
        Bit 6 = 1 : >4 VLAN tags */
    uint8_t num_tags; /**< bit mask, only 7 bits [6:0] are used,
                     see above how */

    /** true is STVLAN enabled */
    uint8_t st_vlan;

    /** rule for frame type */
    macsec_rule_frame_type_t frame_type;

    /** true is MaCsec tagged */
    uint8_t macsec_tagged;
    /** true is coming from Redirect */
    uint8_t from_Redirect;

} macsec_rule_key_mask_t;

typedef struct
{
    /** Size of the transform record (TransformRecord_p) associated with SA
       in 32-bit words */
    uint32_t sa_word_count;

    /** Pointer to the transform record data */
    uint32_t transform_record[24];

    /** SA parameters */
    macsec_sa_action_t params;

    /** SA action type, see macsec_sa_sction_type_t */
    macsec_sa_sction_type_t action_type;

    /** SA drop type, see macsec_drop_type_t */
    macsec_drop_type_t drop_type;

    /** Destination port */
    macsec_port_type_t dest_port;

    /** 4-bit capture reason code */
    uint8_t capture_reason;

} macsec_sa_t;

/** vPort matching rule policy */
typedef struct
{
    /** vPort handle obtained via macsec_vPort_Add(pa) function */
    uint32_t vport_id;

    /** Priority value that is used to resolve multiple rule matches.
        When multiple rules are hit by a packet simultaneously, the rule with
        the higher priority value will be returned. If multiple rules with
        an identical priority value are hit, the rule with the lowest
        rule index is used. */
    uint8_t priority;

    /** true : drop the packet */
    uint8_t fdrop;

    /** true : process the packet as control packet */
    uint8_t fcontrol_packet;

} macsec_rule_policy_t;


/** vPort matching rule data structure */
typedef struct
{
    /** Sets matching values as specified in macsec_rule_key_mask_t */
    macsec_rule_key_mask_t key;

    /** Mask for matching values, can be used to mask out
        irrelevant Key bits */
    macsec_rule_key_mask_t mask;

    /** Data[0] : MAC Destination Address least significant bytes (3..0)\n
        Data[1] : MAC Destination Address most significant bytes (5, 4)\n
        Data[1] : MAC Source Address least significant bytes (1, 0)\n
        Data[2] : MAC Source Address most significant bytes (5..2)\n
        Data[3] : Frame data (ether_type, VLAN tag, MPLS label, etc..)\n
        Data[4] : Frame data (ether_type, VLAN tag, MPLS label, etc..)\n
        See TCAM packet data fields description in the
        EIP-160 Programmer Manual */
    uint32_t data[MACSEC_RULE_NON_CTRL_WORD_COUNT];

    /** Mask for data values, can be used to mask out irrelevant Data bits */
    uint32_t data_mask[MACSEC_RULE_NON_CTRL_WORD_COUNT];

    /** rule policy */
    macsec_rule_policy_t policy;

} macsec_rule_t;


typedef struct 
{
    uint32_t flags;
    macsec_sa_builder_direction_t direction;
    macsec_sa_builder_operation_t operation;
    uint8_t an;
    uint8_t *key_p;
    uint32_t key_byte_count;
    uint8_t *h_key_p;
    uint8_t *salt_p;
    uint8_t *ssci_p;
    uint8_t *sci_p;
    uint32_t seq_num_lo;
    uint32_t seq_num_hi;
    uint32_t window_size;
    uint32_t icv_byte_count;
} macsec_sa_builder_params_t;




typedef struct
{
    /** Reference handle to an vPort */
    void * p;
} macsec_vport_handle_t;

typedef struct
{
    uint8_t             initialized;
    uint8_t             enabled;
    secy_device_t       secy_devices[2]; /* device 0 is egress, device 1 is ingress */
} phy_macsec_dev;

/**---------------------------------------------------------------------------
 * @typedef SABuilder_AESCallback_t
 *
 * Callback function (provided by application to encrypt a single
 * 16-byte block with AES.
 *
 * @param [in[ In_p
 *     input data
 *
 * @param [out] Out_p
 *     output data
 *
 * @param [in] Key_p
 *     AES key
 *
 * @param [in] KeyByteCount
 *     Size of the key in bytes.
 *
 * @return value
 *     None
 */
typedef void (*SABuilder_AESCallback_t)(const uint8_t * const In_p, uint8_t * const Out_p, const uint8_t * const Key_p, const uint32_t KeyByteCount);

static inline uint8_t macsec_SAHandle_IsSame(const secy_SAHandle_t * const Handle1_p, const secy_SAHandle_t * const Handle2_p)
{
    if (memcmp(Handle1_p, Handle2_p, sizeof(secy_SAHandle_t)) != 0)
    {
        return 0;
    }

    return 1;
}

void phy_macsec_read_array(phy_dev_t *phy_dev, uint16_t base_addr, uint16_t offset_addr, uint32_t * MemoryDst_p, int32_t Count);
void phy_macsec_write_array(phy_dev_t *phy_dev, uint16_t base_addr, uint16_t offset_addr, uint32_t * MemoryDst_p, int32_t Count);
void _phy_macsec_write(phy_dev_t *phy_dev, uint16_t base_addr, uint16_t offset_addr, uint32_t val);
uint32_t _phy_macsec_read(phy_dev_t *phy_dev, uint16_t base_addr, uint16_t offset_addr);


static int log_level = LOG_INFO;

#define phy_macsec_log(level, fmt, arg...) ({ if (level <= log_level) (level == LOG_DEBUG ? printk("[%s:%d] " fmt, __func__, __LINE__, ##arg) : printk("MACSEC: " fmt, ##arg));})

#define phy_macsec_write(phy_dev, base_addr, offset_addr, val) ({\
                        _phy_macsec_write(phy_dev, base_addr, offset_addr, val); \
                        phy_macsec_log(LOG_DEBUG, "phy_macsec_write: addr=0x%04x%04x, val=0x%08x, read_val=0x%08x\n", base_addr, offset_addr, val, _phy_macsec_read(phy_dev, base_addr, offset_addr)); \
                    })
#define phy_macsec_read(phy_dev, base_addr, offset_addr) ({\
                        uint32_t val = _phy_macsec_read(phy_dev, base_addr, offset_addr); \
                        phy_macsec_log(LOG_DEBUG, "phy_macsec_read: addr=0x%04x%04x, val=0x%08x\n", base_addr, offset_addr, val); \
                        val; \
                    })

#endif