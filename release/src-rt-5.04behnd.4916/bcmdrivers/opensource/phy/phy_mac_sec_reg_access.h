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

#ifndef PHY_MAC_SEC_REG_ACCESS_H_
#define PHY_MAC_SEC_REG_ACCESS_H_

static inline void macsec_EIP160_EIP_REV_RD(secy_device_access_t *secy_device_access,
                                            uint8_t *  EipNumber,
                                            uint8_t *  ComplmtEipNumber,
                                            uint8_t *  HWPatchLevel,
                                            uint8_t *  MinHWRevision,
                                            uint8_t *  MajHWRevision)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_VERSION);

    *MajHWRevision    = (uint8_t)((RegVal >> 24) & MASK_4_BITS);
    *MinHWRevision    = (uint8_t)((RegVal >> 20) & MASK_4_BITS);
    *HWPatchLevel     = (uint8_t)((RegVal >> 16) & MASK_4_BITS);
    *ComplmtEipNumber = (uint8_t)((RegVal >> 8)  & MASK_8_BITS);
    *EipNumber        = (uint8_t)((RegVal)       & MASK_8_BITS);
}

static inline void macsec_EIP160_CONFIG_RD(secy_device_access_t *secy_device_access,
                                           uint16_t *  SA_Count,
                                           uint16_t *  SC_Count,
                                           uint16_t *  vPort_Count,
                                           uint16_t *  Rule_Count,
                                           uint8_t *  fMatchSCI)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_CONFIG);

    *SA_Count     = (uint16_t)(RegVal & MASK_8_BITS);
    *fMatchSCI    = ((RegVal & BIT_16) != 0);

    /* See ClearTAG spec for calculations about this */
    *SC_Count     = (uint16_t)(*SA_Count / 2);
    *vPort_Count  = (uint16_t)(*SA_Count / 2);
    *Rule_Count   = (uint16_t)(*SC_Count * 2);
}

static inline void macsec_EIP160_CONFIG2_RD(secy_device_access_t *secy_device_access,
                                            uint16_t *  SA_Counters,
                                            uint16_t *  VLAN_Counters,
                                            uint16_t *  Global_Counters,
                                            uint8_t *  fIngressOnly,
                                            uint8_t *  fEgressOnly)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_CONFIG2);

    *SA_Counters     = (uint16_t)((RegVal >> 0) & MASK_4_BITS);
    *VLAN_Counters   = (uint16_t)((RegVal >> 4) & MASK_4_BITS);
    *Global_Counters = (uint16_t)((RegVal >> 8) & MASK_6_BITS);
    *fIngressOnly    = ((RegVal & BIT_24) != 0);
    *fEgressOnly     = ((RegVal & BIT_25) != 0);
}


static inline void macsec_EIP217_EIP_REV_RD(secy_device_access_t *secy_device_access,
                                            uint8_t *  EipNumber,
                                            uint8_t *  ComplmtEipNumber,
                                            uint8_t *  HWPatchLevel,
                                            uint8_t *  MinHWRevision,
                                            uint8_t *  MajHWRevision)
{
    uint32_t RegVal;

    RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_TCAM_STAT_CTRL_OFFS + EIP217_REG_VERSION);

    *MajHWRevision    = (uint8_t)((RegVal >> 24) & MASK_4_BITS);
    *MinHWRevision    = (uint8_t)((RegVal >> 20) & MASK_4_BITS);
    *HWPatchLevel     = (uint8_t)((RegVal >> 16) & MASK_4_BITS);
    *ComplmtEipNumber = (uint8_t)((RegVal >>  8) & MASK_8_BITS);
    *EipNumber        = (uint8_t)((RegVal)       & MASK_8_BITS);
}



static inline void macsec_EIP217_OPTIONS_RD(secy_device_access_t *secy_device_access,
                                            uint8_t *  CountersCount_p,
                                            uint8_t *  ByteCountersCount_p,
                                            uint8_t *  GlobalPktCountersCount_p,
                                            uint8_t *  GlobalByteCountersCount_p,
                                            uint8_t *  ChannelPktCountersCount_p,
                                            uint8_t *  ChannelByteCountersCount_p,
                                            uint8_t *  ChannelCount_p)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_TCAM_STAT_CTRL_OFFS + EIP217_REG_OPTIONS);

    *ChannelCount_p             = (uint8_t)((RegVal >> 24) & MASK_5_BITS);
    *ChannelByteCountersCount_p = (uint8_t)((RegVal >> 21) & MASK_3_BITS);
    *ChannelPktCountersCount_p  = (uint8_t)((RegVal >> 16) & MASK_5_BITS);
    *GlobalByteCountersCount_p  = (uint8_t)((RegVal >> 13) & MASK_3_BITS);
    *GlobalPktCountersCount_p   = (uint8_t)((RegVal >> 8)  & MASK_5_BITS);
    *ByteCountersCount_p        = (uint8_t)((RegVal >> 5)  & MASK_3_BITS);
    *CountersCount_p            = (uint8_t)((RegVal)       & MASK_5_BITS);
}


static inline void macsec_EIP217_OPTIONS2_RD(secy_device_access_t *secy_device_access,
                                             uint16_t *  Counters_p,
                                             uint8_t *  DW_BitCount_p)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_TCAM_STAT_CTRL_OFFS + EIP217_REG_OPTIONS2);

    *DW_BitCount_p  = (uint8_t)((RegVal >> 16) & MASK_8_BITS);
    *Counters_p     = (uint16_t)((RegVal)      & MASK_14_BITS);
}

static inline void macsec_EIP62_CTX_CTRL_WR(secy_device_access_t *secy_device_access,
                                            uint8_t ContextWord32Count,
                                            uint8_t fPktNumThrMode,
                                            uint16_t EtherType)
{
    uint32_t RegVal;

    RegVal  = (((uint32_t)EtherType) & MASK_16_BITS) << 16;
    RegVal |= ((uint32_t)ContextWord32Count) & MASK_8_BITS;

    RegVal |= BIT_9; /* Control mode to fetch context, always 1 */

    if(fPktNumThrMode)
        RegVal |= BIT_10;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP62_REG_CTX_CTRL, RegVal);
}


static inline void macsec_EIP62_CTX_UPD_CTRL_WR(secy_device_access_t *secy_device_access, uint8_t BlockContextUpdate)
{
    uint32_t RegVal = ((uint32_t)BlockContextUpdate) & MASK_2_BITS;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP62_REG_CTX_UPD_CTRL, RegVal);
}


static inline void macsec_EIP160_SAF_CTRL_WR(secy_device_access_t *secy_device_access,
                                              uint8_t fEnable,
                                              uint8_t fDropClass,
                                              uint8_t fDropPP,
                                              uint8_t fDropSecFail,
                                              uint8_t fDropMTU,
                                              uint8_t fDropMACErr,
                                              uint8_t fDropMACCRC)
{
    uint32_t RegVal = EIP160_REG_SAF_CTRL_DEFAULT;

    if(fEnable)
        RegVal |= BIT_0;

    if(fDropClass)
        RegVal |= BIT_1;

    if(fDropPP)
        RegVal |= BIT_2;

    if(fDropSecFail)
        RegVal |= BIT_3;

    if(fDropMTU)
        RegVal |= BIT_4;

    if(fDropMACErr)
        RegVal |= BIT_5;

    if(fDropMACCRC)
        RegVal |= BIT_6;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAF_CTRL, RegVal);
}


static inline void macsec_EIP160_SAF_THRESHOLD_WR(secy_device_access_t *secy_device_access,
                                                   uint16_t LowWatermark,
                                                   uint16_t HighWatermark)
{
    uint32_t RegVal = 0;

    RegVal |= (LowWatermark  & MASK_16_BITS);
    RegVal |= (HighWatermark & MASK_16_BITS) << 16;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAF_THRESHOLD, RegVal);
}


static inline void macsec_EIP160_SAM_NM_FLOW_NCP_WR(secy_device_access_t *secy_device_access,
                                                     uint32_t FlowTypeBits,
                                                     uint32_t DestPortBits,
                                                     uint32_t DropNonReservedBits,
                                                     uint32_t DropActionBits)
{
    uint32_t RegVal = 0;
    uint32_t ONE_BIT_MASK = BIT_0 | BIT_8 | BIT_16 | BIT_24;
    uint32_t TWO_BIT_MASK = ONE_BIT_MASK |
                            BIT_1 | BIT_9 | BIT_17 | BIT_25;

    RegVal |= (uint32_t)(FlowTypeBits & ONE_BIT_MASK);

    RegVal |= (uint32_t)((DestPortBits & TWO_BIT_MASK) << 2);

    RegVal |= (uint32_t)((DropNonReservedBits & ONE_BIT_MASK) << 4);

    RegVal |= (uint32_t)((DropActionBits & TWO_BIT_MASK) << 6);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_NM_FLOW_NCP, RegVal);
}


static inline void macsec_EIP160_SAM_NM_FLOW_CP_WR(secy_device_access_t *secy_device_access,
                                                    uint32_t FlowTypeBits,
                                                    uint32_t DestPortBits,
                                                    uint32_t DropNonReservedBits,
                                                    uint32_t DropActionBits)
{
    uint32_t RegVal = 0;
    uint32_t ONE_BIT_MASK = BIT_0 | BIT_8 | BIT_16 | BIT_24;
    uint32_t TWO_BIT_MASK = ONE_BIT_MASK |
                            BIT_1 | BIT_9 | BIT_17 | BIT_25;

    RegVal |= (uint32_t)(FlowTypeBits & ONE_BIT_MASK);

    RegVal |= (uint32_t)((DestPortBits & TWO_BIT_MASK) << 2);

    RegVal |= (uint32_t)((DropNonReservedBits & ONE_BIT_MASK) << 4);

    RegVal |= (uint32_t)((DropActionBits & TWO_BIT_MASK) << 6);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_NM_FLOW_CP, RegVal);
}


static inline void macsec_EIP160_COUNT_CONTROL_RD(secy_device_access_t *secy_device_access,
                                                  uint16_t offset,
                                                  uint8_t *  fResetAll,
                                                  uint8_t *  fSaturateCtrs,
                                                  uint8_t *  fAutoCtrReset,
                                                  uint8_t *  fResetSummary)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, offset + EIP217_REG_COUNT_CONTROL);

    *fResetAll     = ((RegVal & BIT_0) != 0);
    *fSaturateCtrs = ((RegVal & BIT_2) != 0);
    *fAutoCtrReset = ((RegVal & BIT_3) != 0);
    *fResetSummary = ((RegVal & BIT_4) != 0);
}



static inline void macsec_EIP62_EIP_REV_RD(secy_device_access_t *secy_device_access,
                                           uint8_t *  EipNumber,
                                           uint8_t *  ComplmtEipNumber,
                                           uint8_t *  HWPatchLevel,
                                           uint8_t *  MinHWRevision,
                                           uint8_t *  MajHWRevision)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP62_REG_VERSION);

    *MajHWRevision    = (uint8_t)((RegVal >> 24) & MASK_4_BITS);
    *MinHWRevision    = (uint8_t)((RegVal >> 20) & MASK_4_BITS);
    *HWPatchLevel     = (uint8_t)((RegVal >> 16) & MASK_4_BITS);
    *ComplmtEipNumber = (uint8_t)((RegVal >>  8) & MASK_8_BITS);
    *EipNumber        = (uint8_t)((RegVal)       & MASK_8_BITS);
}


static inline void macsec_EIP62_TYPE_RD(secy_device_access_t *secy_device_access,
                                        uint8_t *  fFPGASolution,
                                        uint8_t *  fGFSboxes,
                                        uint8_t *  fLookupSboxes,
                                        uint8_t *  fMACsecAESOnly,
                                        uint8_t *  fAESPresent,
                                        uint8_t *  fAESFb,
                                        uint8_t *  AESSpeed,
                                        uint8_t *  KeyLengths,
                                        uint8_t *  EopParamBits,
                                        uint8_t *  fIPSec,
                                        uint8_t *  fHdrExtention,
                                        uint8_t *  fSecTagOffset,
                                        uint8_t *  fGHASHPresent,
                                        uint8_t *  fOneCycleCore)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP62_REG_TYPE);

    *fOneCycleCore  = ((RegVal & BIT_31) != 0);
    *fGHASHPresent  = ((RegVal & BIT_30) != 0);
    *fSecTagOffset  = ((RegVal & BIT_27) != 0);
    *fHdrExtention  = ((RegVal & BIT_26) != 0);
    *fIPSec         = ((RegVal & BIT_25) != 0);
    *EopParamBits   = (uint8_t)((RegVal >> 20) & MASK_3_BITS);
    *KeyLengths     = (uint8_t)((RegVal >> 18) & MASK_2_BITS);
    *AESSpeed      = (uint8_t)((RegVal >> 14) & MASK_4_BITS);
    *fAESFb         = ((RegVal & BIT_13) != 0);
    *fAESPresent    = ((RegVal & BIT_12) != 0);
    *fMACsecAESOnly = ((RegVal & BIT_11) != 0);
    *fLookupSboxes  = ((RegVal & BIT_10) != 0);
    *fGFSboxes      = ((RegVal & BIT_9) != 0);
    *fFPGASolution  = ((RegVal & BIT_8) != 0);
}

static inline void macsec_EIP160_FORCE_CLOCK_WR(secy_device_access_t *secy_device_access, uint32_t offset,  uint32_t ClockMask)
{
    uint32_t RegVal = 0;

    RegVal |= (ClockMask & 0x0033);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, offset, RegVal);
}

/*----------------------------------------------------------------------------
 * macsec_EIP160_SecY_SecY_Stat_E_Clear
 */
static inline void macsec_EIP160_SecY_SecY_Stat_E_Clear(secy_device_access_t *secy_device_access,  uint32_t SecY_Index)
{
    /* Egress SecY statistics */
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_TRANSFORM_ERROR_PKTS), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_TRANSFORM_ERROR_PKTS), EIP217_REG_COUNTER_HI_DEFAULT);


    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_E_OUT_PKTS_CTRL), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_E_OUT_PKTS_CTRL), EIP217_REG_COUNTER_HI_DEFAULT);


    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_E_OUT_PKTS_UNTAGGED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_E_OUT_PKTS_UNTAGGED), EIP217_REG_COUNTER_HI_DEFAULT);
}


/*----------------------------------------------------------------------------
 * macsec_EIP160_SecY_SecY_Stat_I_Clear
 */
static inline void macsec_EIP160_SecY_SecY_Stat_I_Clear(secy_device_access_t *secy_device_access,  uint32_t SecY_Index)
{
    /* Ingress SecY statistics */
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_TRANSFORM_ERROR_PKTS), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_TRANSFORM_ERROR_PKTS), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_I_IN_PKTS_CTRL), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_I_IN_PKTS_CTRL), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_I_IN_PKTS_UNTAGGED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_I_IN_PKTS_UNTAGGED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_I_IN_PKTS_NOTAG), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_I_IN_PKTS_NOTAG), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_I_IN_PKTS_BADTAG), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_I_IN_PKTS_BADTAG), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_I_IN_PKTS_NOSCI), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_I_IN_PKTS_NOSCI), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_I_IN_PKTS_UNKNOWNSCI), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_I_IN_PKTS_UNKNOWNSCI), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_LO(EIP160_SECY_STAT_I_IN_PKTS_TAGGEDCTRL), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XTSECY_STATISTICS(SecY_Index) + EIP217_REG_COUNTER_HI(EIP160_SECY_STAT_I_IN_PKTS_TAGGEDCTRL), EIP217_REG_COUNTER_HI_DEFAULT);
}


static inline void macsec_EIP160_SecY_Ifc_Stat_E_Clear(secy_device_access_t *secy_device_access,  uint32_t IfcIndex)
{
    /* Egress IFC statistics */
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_OCTETS_COMMON), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_OCTETS_COMMON), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_OCTETS_UNCONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_OCTETS_UNCONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_OCTETS_CONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_OCTETS_CONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_UCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_UCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);
    
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_MCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_MCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);
    
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_BCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_BCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_UCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_UCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_MCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_MCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_E_OUT_BCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_E_OUT_BCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);
}


/*----------------------------------------------------------------------------
 * macsec_EIP160_SecY_Ifc_Stat_I_Clear
 */
static inline void macsec_EIP160_SecY_Ifc_Stat_I_Clear(secy_device_access_t *secy_device_access,  uint32_t IfcIndex)
{
    /* Ingress IFC statistics */
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_I_IN_OCTETS_UNCONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_I_IN_OCTETS_UNCONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_I_IN_OCTETS_CONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_I_IN_OCTETS_CONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_I_IN_UCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_I_IN_UCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_I_IN_MCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_I_IN_MCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_I_IN_BCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_I_IN_BCAST_PKTS_UNCONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_I_IN_UCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_I_IN_UCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_I_IN_MCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_I_IN_MCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_LO(EIP160_IFC_STAT_I_IN_BCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_IFC_STATISTICS(IfcIndex) + EIP217_REG_COUNTER_HI(EIP160_IFC_STAT_I_IN_BCAST_PKTS_CONTROLLED), EIP217_REG_COUNTER_HI_DEFAULT);
}

static inline void macsec_EIP160_XFORM_REC_RD(secy_device_access_t *secy_device_access, uint32_t XformIndex, uint32_t WordOffsetInXform, int32_t WordCount, uint32_t * MemoryDst_p)
{
    phy_macsec_read_array(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XFORM_REC(XformIndex) + (WordOffsetInXform * EIP160_REG_OFFS), MemoryDst_p, WordCount);
}

static inline void macsec_EIP160_XFORM_REC_WR(secy_device_access_t *secy_device_access, uint32_t XformIndex, uint32_t * MemorySrc_p, int32_t Count)
{
    int i;

    phy_macsec_write_array(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XFORM_REC(XformIndex), MemorySrc_p, Count);

    /* Write the rest of the transform record with zero. */
    for (i = Count; i < EIP160_XFORM_REC_WORD_COUNT; i++)
    {
        phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XFORM_REC(XformIndex) + (i * EIP160_REG_OFFS), 0);
    }
}

static inline void macsec_EIP160_XFORM_REC_WORD_WR(secy_device_access_t *secy_device_access, uint32_t XformIndex, uint32_t WordOffsetInXform, uint32_t DataWord)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_XFORM_REC(XformIndex) + (WordOffsetInXform * EIP160_REG_OFFS), DataWord);
}

static inline void macsec_EIP160_RXSC_CAM_SCI_LO_WR(secy_device_access_t *secy_device_access, uint32_t SC_Index, uint32_t Value)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_RXSC_CAM_SCI_LO(SC_Index), Value);
}


static inline void macsec_EIP160_RXSC_CAM_SCI_HI_WR(secy_device_access_t *secy_device_access, uint32_t SC_Index, uint32_t Value)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_RXSC_CAM_SCI_HI(SC_Index), Value);
}


static inline void macsec_EIP160_RXSC_CAM_CTRL_WR(secy_device_access_t *secy_device_access, uint32_t SC_Index, uint32_t Value)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_RXSC_CAM_CTRL(SC_Index), Value);
}


static inline void macsec_EIP160_RXSC_ENTRY_ENABLE_CTRL_WR(secy_device_access_t *secy_device_access, uint32_t SC_IndexSet, uint8_t fSetEnable, uint8_t fSetAll, uint32_t SC_ClearIndex, uint8_t fClearEnable, uint8_t fClearAll)
{
    uint32_t RegVal = EIP160_REG_RXSC_ENTRY_ENABLE_CTRL_DEFAULT;

    if (fSetEnable)
        RegVal |= BIT_14;
    else
        RegVal &= ~BIT_14;

    if (fSetAll)
        RegVal |= BIT_15;
    else
        RegVal &= ~BIT_15;

    if (fClearEnable)
        RegVal |= BIT_30;
    else
        RegVal &= ~BIT_30;

    if (fClearAll)
        RegVal |= BIT_31;
    else
        RegVal &= ~BIT_31;

    RegVal |= (SC_IndexSet   & MASK_8_BITS);
    RegVal |= (SC_ClearIndex & MASK_8_BITS) << 16;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_RXSC_ENTRY_ENABLE_CTRL, RegVal);
}

static inline void macsec_EIP160_SC_SA_MAP1_WR(secy_device_access_t *secy_device_access,
         uint32_t SC_Index,
         uint32_t SA_Index0,
         uint8_t fSA_InUse0,
         uint8_t fSA_IndexUpdate0,
         uint32_t SA_Index1,
         uint8_t fSA_InUse1)
{
    uint32_t RegVal = EIP160_REG_SC_SA_MAP1_DEFAULT;

    if (fSA_IndexUpdate0)
        RegVal |= BIT_13;
    else
        RegVal &= ~BIT_13;

    if (fSA_InUse0)
        RegVal |= BIT_15;
    else
        RegVal &= ~BIT_15;

    if (fSA_InUse1)
        RegVal |= BIT_31;
    else
        RegVal &= ~BIT_31;

    RegVal |= (SA_Index0 & MASK_10_BITS);
    RegVal |= (SA_Index1 & MASK_10_BITS) << 16;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SC_SA_MAP1(SC_Index), RegVal);
}


static inline void macsec_EIP160_SC_SA_MAP1_RD(secy_device_access_t *secy_device_access,
         uint32_t SC_Index,
        uint32_t *SA_Index0,
        uint8_t *fSA_InUse0,
        uint32_t *SA_Index1,
        uint8_t *fSA_InUse1)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SC_SA_MAP1(SC_Index));

    *SA_Index0  = RegVal & MASK_10_BITS;
    *fSA_InUse0 = (RegVal >> 15) & MASK_1_BIT;
    *SA_Index1  = (RegVal >> 16) & MASK_10_BITS;
    *fSA_InUse1 = (RegVal >> 31) & MASK_1_BIT;
}


static inline void macsec_EIP160_SC_SA_MAP2_WR(secy_device_access_t *secy_device_access,
         uint32_t SC_Index,
         uint32_t SA_Index2,
         uint8_t fSA_InUse2,
         uint32_t SA_Index3,
         uint8_t fSA_InUse3)
{
    uint32_t RegVal = EIP160_REG_SC_SA_MAP2_DEFAULT;

    if (fSA_InUse2)
        RegVal |= BIT_15;
    else
        RegVal &= ~BIT_15;

    if (fSA_InUse3)
        RegVal |= BIT_31;
    else
        RegVal &= ~BIT_31;

    RegVal |= (SA_Index2 & MASK_10_BITS);
    RegVal |= (SA_Index3 & MASK_10_BITS) << 16;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SC_SA_MAP2(SC_Index), RegVal);
}


static inline void macsec_EIP160_SC_SA_MAP2_RD(secy_device_access_t *secy_device_access,
         uint32_t SC_Index,
        uint32_t *SA_Index2,
        uint8_t *fSA_InUse2,
        uint32_t *SA_Index3,
        uint8_t *fSA_InUse3)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SC_SA_MAP2(SC_Index));

    *SA_Index2  = RegVal & MASK_10_BITS;
    *fSA_InUse2 = (RegVal >> 15) & MASK_1_BIT;
    *SA_Index3  = (RegVal >> 16) & MASK_10_BITS;
    *fSA_InUse3 = (RegVal >> 31) & MASK_1_BIT;
}

static inline void macsec_EIP160_SAM_FLOW_CTRL1_INGRESS_WR(secy_device_access_t *secy_device_access,
         uint32_t Index,
         uint8_t DestPort,
         uint8_t fDropNonRedir,
         uint8_t fFlowCryptAuth,
         uint8_t DropAction,
         uint8_t CaptReason,
         uint8_t fReplayProtect,
         uint8_t fAllowTaggedData,
         uint8_t fAllowUntaggedData,
         uint8_t ValidateFramesTagged,
         uint8_t ValidateFramesUntagged,
         uint8_t fEoMPLSCtrlWord,
         uint8_t fEoMPLSSubport,
         uint8_t fRetainICV,
         uint8_t fRetainSecTAG)
{
    uint32_t RegVal = 0x2;              /* Ingress */

    /* General MACsec SecY part */
    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fDropNonRedir)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    RegVal |= (uint32_t)((((uint32_t)CaptReason) & MASK_4_BITS) << 8);

    /* Specific MACsec SecY part */
    if(fReplayProtect)
        RegVal |= BIT_16;

    if(fAllowTaggedData)
        RegVal |= BIT_17;

    if(fAllowUntaggedData)
        RegVal |= BIT_18;

    RegVal |= (uint32_t)((((uint32_t)ValidateFramesTagged) &
                                                        MASK_2_BITS) << 19);

    if(ValidateFramesUntagged)
        RegVal |= BIT_21;

    if(fEoMPLSCtrlWord)
        RegVal |= BIT_22;

    if(fEoMPLSSubport)
        RegVal |= BIT_23;

    if(fRetainICV)
        RegVal |= BIT_24;

    if(fRetainSecTAG)
        RegVal |= BIT_25;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_FLOW_CTRL1(Index), RegVal);
}


static inline void macsec_EIP160_SAM_FLOW_CTRL1_EGRESS_WR(secy_device_access_t *secy_device_access,
         uint32_t Index,
         uint8_t DestPort,
         uint8_t fDropNonRedir,
         uint8_t fFlowCryptAuth,
         uint8_t DropAction,
         uint8_t CaptReason,
         uint8_t fProtectFrames,
         uint8_t fConfProtect,
         uint8_t fIncludeSCI,
         uint8_t fUseES,
         uint8_t fUseSCB,
         uint8_t fAllowDataPkts,
         uint8_t fEoMPLSCtrlWord,
         uint8_t fEoMPLSSubport,
         uint8_t fSLPadStripEnb,
         uint8_t fECFromSTVlan)
{
    uint32_t RegVal = 0x3;              /* Egress */

    /* General MACsec SecY part */
    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fDropNonRedir)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    RegVal |= (uint32_t)((((uint32_t)CaptReason) & MASK_4_BITS) << 8);

    /* Specific MACsec SecY part */
    if(fProtectFrames)
        RegVal |= BIT_16;

    if(fConfProtect)
        RegVal |= BIT_17;

    if(fIncludeSCI)
        RegVal |= BIT_18;

    if(fUseES)
        RegVal |= BIT_19;

    if(fUseSCB)
        RegVal |= BIT_20;

    if(fAllowDataPkts)
        RegVal |= BIT_21;

    if(fEoMPLSCtrlWord)
        RegVal |= BIT_22;

    if(fEoMPLSSubport)
        RegVal |= BIT_23;

    if(fSLPadStripEnb)
        RegVal |= BIT_28;

    if(fECFromSTVlan)
        RegVal |= BIT_29;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_FLOW_CTRL1(Index), RegVal);
}


static inline void macsec_EIP160_SAM_FLOW_CTRL1_CRYPTAUTH_WR(secy_device_access_t *secy_device_access,
         uint32_t Index,
         uint8_t DestPort,
         uint8_t fDropNonRedir,
         uint8_t fFlowCryptAuth,
         uint8_t DropAction,
         uint8_t CaptReason,
         uint8_t fConfProtect,
         uint8_t fIcvAppend,
         uint8_t IV,
         uint8_t fIcvVerify)
{
    uint32_t RegVal = 0;

    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fDropNonRedir)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    RegVal |= (uint32_t)((((uint32_t)CaptReason) & MASK_4_BITS) << 8);

    if(fConfProtect)
        RegVal |= BIT_17;

    if(fIcvAppend)
        RegVal |= BIT_18;

    RegVal |= (uint32_t)((((uint32_t)IV) & MASK_2_BITS) << 19);

    if(fIcvVerify)
        RegVal |= BIT_21;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_FLOW_CTRL1(Index), RegVal);
}


static inline void macsec_EIP160_SAM_FLOW_CTRL1_BYPASS_WR(secy_device_access_t *secy_device_access,
         uint32_t Index,
         uint8_t DestPort,
         uint8_t fDropNonRedir,
         uint8_t fFlowCryptAuth,
         uint8_t DropAction,
         uint8_t CaptReason)
{
    uint32_t RegVal = 0;

    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fDropNonRedir)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    RegVal |= (uint32_t)((((uint32_t)CaptReason) & MASK_4_BITS) << 8);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_FLOW_CTRL1(Index), RegVal);
}


static inline void macsec_EIP160_SAM_FLOW_CTRL1_DROP_WR(secy_device_access_t *secy_device_access,
         uint32_t Index,
         uint8_t DestPort,
         uint8_t fDropNonRedir,
         uint8_t fFlowCryptAuth,
         uint8_t DropAction,
         uint8_t CaptReason)
{
    uint32_t RegVal = 1;

    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fDropNonRedir)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    RegVal |= (uint32_t)((((uint32_t)CaptReason) & MASK_4_BITS) << 8);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_FLOW_CTRL1(Index), RegVal);
}


static inline void macsec_EIP160_SAM_FLOW_CTRL2_WR(secy_device_access_t *secy_device_access,
         uint32_t Index,
         uint8_t ConfidentialityOffset,
         uint8_t PreSecTagAuthStart,
         uint8_t PreSecTagAuthLen,
         uint8_t SectagOffset)
{
    uint32_t RegVal = 0;

    RegVal |= (PreSecTagAuthStart    & MASK_6_BITS);
    RegVal |= (PreSecTagAuthLen      & MASK_6_BITS) << 8;
    RegVal |= (SectagOffset          & MASK_6_BITS) << 16;
    RegVal |= (ConfidentialityOffset & MASK_8_BITS) << 24;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_FLOW_CTRL2(Index), RegVal);
}

static inline void macsec_EIP160_COUNTER_64_DEFAULT_WR(secy_device_access_t *secy_device_access, uint32_t Offset, uint32_t Index)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, Offset + EIP217_REG_COUNTER_LO(Index), EIP217_REG_COUNTER_LO_DEFAULT);
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, Offset + EIP217_REG_COUNTER_HI(Index), EIP217_REG_COUNTER_HI_DEFAULT);
}

static inline void macsec_EIP160_COUNTER_64_RD(secy_device_access_t *secy_device_access, uint32_t Offset, uint32_t Index, uint32_t *CountLo_p, uint32_t *CountHi_p)
{
    *CountLo_p = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, Offset + EIP217_REG_COUNTER_LO(Index));
    *CountHi_p = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, Offset + EIP217_REG_COUNTER_HI(Index));
}

static inline void macsec_EIP160_TCAM_CTRL_WR(secy_device_access_t *secy_device_access,
        uint16_t Offset,
         uint8_t NumTags,
         uint8_t PacketType,
         uint8_t STVLAN,
         uint8_t FrameType,
         uint8_t MACsecTagged,
         uint8_t FromRedirect)
{
    uint32_t RegVal = EIP160_REG_TCAM_CTRL_DEFAULT;

    RegVal |= (FromRedirect << 14);
    RegVal |= (MACsecTagged << 13);
    RegVal |= ((FrameType  & MASK_2_BITS) << 11);
    RegVal |= (STVLAN       << 10);
    RegVal |= ((PacketType & MASK_2_BITS) << 8);
    RegVal |= (NumTags     & MASK_7_BITS);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, Offset, RegVal);
}

static inline void macsec_EIP160_WriteArray(secy_device_access_t *secy_device_access,
         uint32_t Offset,
         uint32_t *MemorySrc_p,
         int32_t Count)
{
    phy_macsec_write_array(secy_device_access->phy_dev, secy_device_access->base_addr, Offset, MemorySrc_p, Count);
}

static inline void macsec_EIP160_TCAM_POLICY_WR(secy_device_access_t *secy_device_access,
         uint32_t Index,
         uint16_t vPortIndex,
         uint8_t Priority,
         uint8_t fDrop,
         uint8_t fControlPkt)
{
    uint32_t RegVal = EIP160_REG_TCAM_POLICY_DEFAULT;

    if(fControlPkt)
        RegVal |= BIT_31;

    if(fDrop)
        RegVal |= BIT_30;

    RegVal |= ((Priority  & MASK_3_BITS)   << 27);
    RegVal |= (vPortIndex & MASK_16_BITS);

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_TCAM_POLICY(Index), RegVal);
}

static inline void macsec_EIP160_TCAM_POLICY_DEFAULT_WR(secy_device_access_t *secy_device_access, uint32_t Index)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_TCAM_POLICY(Index), EIP160_REG_TCAM_POLICY_DEFAULT);
}

static inline void macsec_EIP160_SAM_IN_FLIGHT_WR(secy_device_access_t *secy_device_access, uint8_t fLoadUnsafe)
{
    uint32_t RegVal = EIP160_REG_SAM_IN_FLIGHT_DEFAULT;

    if(fLoadUnsafe)
        RegVal |= BIT_31;
    else
        RegVal &= ~BIT_31;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_IN_FLIGHT, RegVal);
}

static inline void macsec_EIP160_SAM_IN_FLIGHT_RD(secy_device_access_t *secy_device_access, uint8_t *Unsafe, uint8_t *InFlight)
{
    uint32_t RegVal = phy_macsec_read(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_IN_FLIGHT);

    *Unsafe      = (uint8_t)((RegVal >> 0) & MASK_6_BITS);
    *InFlight    = (uint8_t)((RegVal >> 8) & MASK_6_BITS);
}

static inline void macsec_EIP160_SAM_ENTRY_SET_WR(secy_device_access_t *secy_device_access, uint8_t Index, uint32_t Mask)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_ENTRY_SET(Index), Mask);
}

static inline void macsec_EIP160_SAM_ENTRY_CLEAR_WR(secy_device_access_t *secy_device_access, uint8_t Index, uint32_t Mask)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_SAM_ENTRY_CLEAR(Index), Mask);
}

static inline void macsec_EIP160_CP_MATCH_ENABLE_WR(secy_device_access_t *secy_device_access, uint32_t MatchEnable)
{
    uint32_t RegVal = MatchEnable & EIP160_REG_CP_MATCH_ENABLE_MASK;

    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP160_REG_CP_MATCH_ENABLE, RegVal);
}

static inline void macsec_EIP62_SEQ_NR_THRESH_WR(secy_device_access_t *secy_device_access, uint32_t Threshold)
{
    phy_macsec_write(secy_device_access->phy_dev, secy_device_access->base_addr, EIP62_REG_SEQ_NR_THRESH, Threshold);
}

static inline void macsec_xlmac_reg64_write(phy_dev_t *phy_dev, macsec_port_loc_t port_loc, uint32_t addr32, uint64_t data64)
{
    /* XLMAC register address -- sys side : 0xf8000nnn , line side : 0xf8001nnn */
    uint32_t  sys_line_side = (macsecPortLocLine == port_loc) ?  MACSEC_CONF_CTL_LN_SIDE : MACSEC_CONF_CTL_SW_SIDE;

    /* reset Macsec_config_CONTROL */
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_CTLr, 0);
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_DATA_HIr, XLMAC_DATA_64_HI(data64));
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_DATA_LOr, XLMAC_DATA_64_LO(data64));
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_CTLr, MACSEC_CONF_CTL_COMMITf | MACSEC_CONF_CTL_WRITEf | sys_line_side | (addr32 & MACSEC_CONF_CTL_ADDRf));

    msleep(1); 
    /* reset Macsec_config_CONTROL */
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_CTLr, 0);
}

static inline void macsec_xlmac_reg64_read(phy_dev_t *phy_dev, macsec_port_loc_t port_loc, uint32_t addr32, uint64_t *data64)
{
    /* XLMAC register address -- sys side : 0xf8000nnn , line side : 0xf8001nnn */
    uint32_t  sys_line_side = (macsecPortLocLine == port_loc) ?  MACSEC_CONF_CTL_LN_SIDE : MACSEC_CONF_CTL_SW_SIDE;
    uint32_t  data32lo = 0, data32hi = 0;

    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_CTLr, 0);
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_CTLr, MACSEC_CONF_CTL_COMMITf | sys_line_side | (addr32 & MACSEC_CONF_CTL_ADDRf));

    msleep(1);             /* reset Macsec_config_CONTROL */
    phy_macsec_write(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_CTLr, 0);

    data32hi = phy_macsec_read(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_DATA_HIr);
    data32lo = phy_macsec_read(phy_dev, MACSEC_XLMAC_REG_BASE, MACSEC_CONF_DATA_LOr);
    
    XLMAC_DATA_64_SET(*data64, data32hi, data32lo);
}



#endif //PHY_MAC_SEC_REG_ACCESS_H_