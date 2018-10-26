/*
   Copyright (c) 2015 Broadcom
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

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD =
{
    "SDMABBID",
#if RU_INCLUDE_DESC
    "SDMA_BB_ID",
    "SDMA BB ID. This ID defines the BB ID of the SDMA that the BBH communicates with.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD =
{
    "DISPBBID",
#if RU_INCLUDE_DESC
    "DISP_BB_ID",
    "Dispatcher BB ID. This ID defines the BB ID of the Dispatcher that the BBH communicates with.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD =
{
    "SBPMBBID",
#if RU_INCLUDE_DESC
    "SBPM_BB_ID",
    "SBPM BB ID. This ID defines the BB ID of the SBPM that the BBH communicates with.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED2_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED2_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD =
{
    "NORMALVIQ",
#if RU_INCLUDE_DESC
    "VIQ_NORMAL",
    "Defines the Dispatchers Virtual Ingress Queue for normal packets",
#endif
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD =
{
    "EXCLVIQ",
#if RU_INCLUDE_DESC
    "VIQ_EXCL",
    "Defines the Dispatchers Virtual Ingress Queue for exclusive packets",
#endif
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD =
{
    "PATTERNDATALSB",
#if RU_INCLUDE_DESC
    "Pattern_Data_LSB",
    "Pattern Data[31:0]",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD =
{
    "PATTERNDATAMSB",
#if RU_INCLUDE_DESC
    "Pattern_Data_MSB",
    "Pattern Data[63:32]",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD =
{
    "PATTERNMASKLSB",
#if RU_INCLUDE_DESC
    "Pattern_Mask_LSB",
    "Pattern mask[31:0]",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD =
{
    "PATTERNMASKMSB",
#if RU_INCLUDE_DESC
    "Pattern_Mask_MSB",
    "Pattern Mask[63:32]",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD =
{
    "PLOAMEN",
#if RU_INCLUDE_DESC
    "PLOAM_EN",
    "Direct this packet type to Exclusive VIQ in the Dispatcher",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD =
{
    "PRI3EN",
#if RU_INCLUDE_DESC
    "Priority3_EN",
    "Direct this packet type to Exclusive VIQ in the Dispatcher",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD =
{
    "PAUSEEN",
#if RU_INCLUDE_DESC
    "Pause_EN",
    "Direct this packet type to Exclusive VIQ in the Dispatcher",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD =
{
    "PFCEN",
#if RU_INCLUDE_DESC
    "PFC_EN",
    "Direct this packet type to Exclusive VIQ in the Dispatcher",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD =
{
    "CTRLEN",
#if RU_INCLUDE_DESC
    "Control_EN",
    "Direct this packet type to Exclusive VIQ in the Dispatcher",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD =
{
    "MULTEN",
#if RU_INCLUDE_DESC
    "Multicast_EN",
    "Direct this packet type to Exclusive VIQ in the Dispatcher",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD =
{
    "OAMEN",
#if RU_INCLUDE_DESC
    "OAM_EN",
    "Direct this packet type to Exclusive VIQ in the Dispatcher",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD =
{
    "PATTENOFFSET",
#if RU_INCLUDE_DESC
    "Pattern_recognition_offset",
    "Defines the pattern recognition offset within the packet. Offset is 8 bytes resolution",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD =
{
    "PATTERNEN",
#if RU_INCLUDE_DESC
    "Pattern_recognition_en",
    "Must be enabled if pattern recognition is used",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED2_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED2_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD =
{
    "EXCEN",
#if RU_INCLUDE_DESC
    "Exclusive_viq_en",
    "Must be enabled if Exclusive VIQ is used",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD =
{
    "DISNORMALCHECK",
#if RU_INCLUDE_DESC
    "Disable_normal_check",
    "If asserted, Exclusive queue is determined only according to the information from the MAC (EPON OAM or GPON PLOAM/OMCI)"
    "No support for pattern match or multicast identification in this mode",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED3
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED3_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED3_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD =
{
    "DATABASE",
#if RU_INCLUDE_DESC
    "Data_base_address",
    "The Data FIFO base address within the SDMA address space."
    "The address is in chunk resolution (128 bytes)."
    "The value should be identical to the relevant configuration in the SDMA.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD =
{
    "DESCBASE",
#if RU_INCLUDE_DESC
    "Descriptor_base_address",
    "The Descriptor FIFO base address within the SDMA address space."
    "The address is in chunk descriptor resolution (8 bytes)."
    "The value  should be identical to the relevant configuration in the SDMA.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD =
{
    "NUMOFCD",
#if RU_INCLUDE_DESC
    "Number_of_Chunk-Descriptors",
    "Defines the size of the Chunk descripors FIFO in the DMA.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD =
{
    "EXCLTH",
#if RU_INCLUDE_DESC
    "Exclusive_threshold",
    "This field defines the number of occupied write chunks for dropping normal or high priority packets.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD =
{
    "COHERENCYEN",
#if RU_INCLUDE_DESC
    "Coherency1_en",
    "BBH has two methods to keep coherency:"
    "1. Write reply for last chunk only"
    "2. Write reply for each chunk"
    ""
    "1 - enables the first method"
    "0 - enables the second method",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED2_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED2_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD =
{
    "MINPKT0",
#if RU_INCLUDE_DESC
    "Minimum_Packet_0",
    "Packets shorter than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD =
{
    "MINPKT1",
#if RU_INCLUDE_DESC
    "Minimum_Packet_1",
    "Packets shorter than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD =
{
    "MINPKT2",
#if RU_INCLUDE_DESC
    "Minimum_Packet_2",
    "Packets shorter than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD =
{
    "MINPKT3",
#if RU_INCLUDE_DESC
    "Minimum_Packet_3",
    "Packets shorter than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD =
{
    "MAXPKT0",
#if RU_INCLUDE_DESC
    "Maximum_Packet_0",
    "Packets longer than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD =
{
    "MAXPKT1",
#if RU_INCLUDE_DESC
    "Maximum_Packet_1",
    "Packets longer than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD =
{
    "MAXPKT2",
#if RU_INCLUDE_DESC
    "Maximum_Packet_2",
    "Packets longer than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD =
{
    "MAXPKT3",
#if RU_INCLUDE_DESC
    "Maximum_Packet_3",
    "Packets longer than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD =
{
    "SOPOFFSET",
#if RU_INCLUDE_DESC
    "SOP_offset",
    "The SOP offset in bytes."
    "Allowed values: 0-127."
    "This value should match the relevant configuration in the Runner block.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD =
{
    "TIMER",
#if RU_INCLUDE_DESC
    "Timer",
    "Timer value before de-asserting the flow control indication."
    "The duration of the time is determined according to the BBH clock frequency.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD =
{
    "DISPDROPDIS",
#if RU_INCLUDE_DESC
    "Dispatcher_drop_disable",
    "Disable dropping packets due to no space in the Dispatcher.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD =
{
    "SDMADROPDIS",
#if RU_INCLUDE_DESC
    "SMDA_drop_disable",
    "Disable dropping packets due to no space in the SDMA.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD =
{
    "SBPMDROPDIS",
#if RU_INCLUDE_DESC
    "SBPM_drop_disable",
    "Disable dropping packets due to no space in the SBPM.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD =
{
    "FCFORCE",
#if RU_INCLUDE_DESC
    "Flow_control_force",
    "Asserting this bit will force a flow control indication towards the MAC",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD =
{
    "CRCOMITDIS",
#if RU_INCLUDE_DESC
    "CRC_omit_disable",
    "Disable CRC omitting.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD =
{
    "PKTEN",
#if RU_INCLUDE_DESC
    "Packet_enable",
    "When de-asserted, the BBH will not read new fragment/packet from the MAC."
    "The BBH will Gracefully enable/disable (on fragment boundary for N/X/GPON/2 and on packet boundary for the rest)",
#endif
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD =
{
    "SBPMEN",
#if RU_INCLUDE_DESC
    "SBPM_enable",
    "When de-asserted, the BBH will not pre-fetch SBPM buffers",
#endif
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_ENABLE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "Enable",
    "Enable G999.1",
#endif
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD =
{
    "BYTES4_7ENABLE",
#if RU_INCLUDE_DESC
    "Byte4_7Enable",
    "Enable G999.1 transfer of bytes 4-7 instead of bytes 0-3",
#endif
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_G9991EN_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_G9991EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD =
{
    "FLOWTH",
#if RU_INCLUDE_DESC
    "Flow_threshold",
    "According to this threshold:"
    "Flows 32 - th will have set 0 configurations."
    "Flows (th+1) - 255 will have set 1 configurations.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD =
{
    "MINPKTSEL0",
#if RU_INCLUDE_DESC
    "Minimum_packet_size_select_0",
    "Set 0 of the general configuration."
    "Selects between 4 global minimum packet size.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD =
{
    "MAXPKTSEL0",
#if RU_INCLUDE_DESC
    "Maximum_packet_size_select_0",
    "Set 0 of the general configuration."
    "Selects between 4 global maximum packet size.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD =
{
    "MINPKTSEL1",
#if RU_INCLUDE_DESC
    "Minimum_packet_size_select_1",
    "Set 1 of the general configuration."
    "Selects between 4 global minimum packet size.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD =
{
    "MAXPKTSEL1",
#if RU_INCLUDE_DESC
    "Maximum_packet_size_select_1",
    "Set 1 of the general configuration."
    "Selects between 4 global maximum packet size.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD =
{
    "MINPKTSEL",
#if RU_INCLUDE_DESC
    "Mimimum_packet_size_select",
    "Selects one of the 4 global configurations for minimum packet size."
    "Bits {2n, 2n+1} refers to flow n.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD =
{
    "MINPKTSEL",
#if RU_INCLUDE_DESC
    "Mimimum_packet_size_select",
    "Selects one of the 4 global configurations for minimum packet size."
    "Bits {2n, 2n+1} refers to flow n+16.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD =
{
    "MAXPKTSEL",
#if RU_INCLUDE_DESC
    "Maximum_packet_size_select",
    "Selects one of the 4 global configurations for maximum packet size."
    "Bits {2n, 2n+1} refers to flow n.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD =
{
    "MAXPKTSEL",
#if RU_INCLUDE_DESC
    "Maximum_packet_size_select",
    "Selects one of the 4 global configurations for maximum packet size."
    "Bits {2n, 2n+1} refers to flow n+16.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD =
{
    "MACMODE",
#if RU_INCLUDE_DESC
    "Mac_mode",
    "Relevant for PON BBH only."
    "Distinguish between GPON (GPON, XGPON, NGPON2) to EPON (EPON, 10GEPON):"
    "0: N/X/GPON/2"
    "1: 10G/EPON",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD =
{
    "GPONMODE",
#if RU_INCLUDE_DESC
    "GPON_mode",
    "Relevant for GPON BBH only."
    "Distinguish between GPON and XGPON (XGPON, NGPON2):"
    "0: GPON"
    "1: N/X/GPON/2",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD =
{
    "MACVDSL",
#if RU_INCLUDE_DESC
    "Mac_is_VDSL",
    "Relevant for PON BBH only."
    "Distinguish between GPON (GPON, XGPON, NGPON2) to EPON (EPON, 10GEPON):"
    "0: N/X/GPON/2"
    "1: 10G/EPON",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_MACMODE_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD =
{
    "MAXREQ",
#if RU_INCLUDE_DESC
    "MAXREQ",
    "Configure max on the fly requests to SBPM",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD =
{
    "INBUFRST",
#if RU_INCLUDE_DESC
    "Input_buf_reset_command",
    "Writing 1 to this register will reset the input buffer."
    "For a reset operation the SW should assert and then de-assert this bit.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD =
{
    "BURSTBUFRST",
#if RU_INCLUDE_DESC
    "Burst_buf_reset_command",
    "Writing 1 to this register will reset the Burst buffer."
    "For a reset operation the SW should assert and then de-assert this bit.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD =
{
    "INGRESSCNTXT",
#if RU_INCLUDE_DESC
    "Ingress_context_reset_command",
    "Writing 1 to this register will reset the ingress context."
    "For a reset operation the SW should assert and then de-assert this bit.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD =
{
    "CMDFIFORST",
#if RU_INCLUDE_DESC
    "CMD_FIFO_reset_command",
    "Writing 1 to this register will reset the IH buffer enable."
    "For a reset operation the SW should assert and then de-assert this bit.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD =
{
    "SBPMFIFORST",
#if RU_INCLUDE_DESC
    "SBPM_FIFO_reset_command",
    "Writing 1 to this register will reset the SBPM FIFO."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD =
{
    "COHERENCYFIFORST",
#if RU_INCLUDE_DESC
    "Coherency_FIFO_reset_command",
    "Writing 1 to this register will reset the coherency FIFO."
    "For a reset operation the SW should assert and then de-assert this bit.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD =
{
    "CNTXTRST",
#if RU_INCLUDE_DESC
    "Context_reset_command",
    "Writing 1 to this register will reset the reassembly context table."
    "The reset is done immediately. Reading this register will always return 0.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD =
{
    "SDMARST",
#if RU_INCLUDE_DESC
    "SDMA_write_pointer_reset_command",
    "Writing 1 to this register will reset the SDMA write pointer."
    "For a reset operation the SW should assert and then de-assert this bit.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD =
{
    "RXDBGSEL",
#if RU_INCLUDE_DESC
    "RX_debug_select",
    "Selects one out of 10 possible debug vectors",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD =
{
    "ID_2OVERWR",
#if RU_INCLUDE_DESC
    "dest_id_to_overwr",
    "This field contains the users BB id for override",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD =
{
    "OVERWR_RA",
#if RU_INCLUDE_DESC
    "overwr_route_addr",
    "The new RA",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD =
{
    "OVERWR_EN",
#if RU_INCLUDE_DESC
    "overwr_route_addr_en",
    "the overwr mechanism will be used only if this bit is active (1).",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED2_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED2_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "FLOWID",
    "Non Ethernet flow ID",
#endif
    BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "Non_Ethernet_dis",
    "When asserted, CRC errors will not be counted for that flow.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_NONETH_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_NONETH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_NONETH_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_INPKT_INPKT
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD =
{
    "INPKT",
#if RU_INCLUDE_DESC
    "Incoming_packets",
    "This counter counts the number of incoming good packets.",
#endif
    BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD =
{
    "INPLOAM",
#if RU_INCLUDE_DESC
    "Incoming_PLOAM",
    "This counter counts the number of incoming PLOAMs.",
#endif
    BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "PM_counter_value",
    "PM counter value.",
#endif
    BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_PM_COUNTERS_RUNTERROR_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_PM_COUNTERS_RUNTERROR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_PM_COUNTERS_RUNTERROR_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_PM_COUNTERS_RUNTERROR_RESERVED0_FIELD_WIDTH,
    BBH_RX_PM_COUNTERS_RUNTERROR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0LSB_INREASS
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD =
{
    "INREASS",
#if RU_INCLUDE_DESC
    "In_reassembly",
    "In reassembly."
    "Not relevant for Ethernet.",
#endif
    BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0LSB_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0LSB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX0LSB_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0LSB_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0LSB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0LSB_FLOWID
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "Flow_ID",
    "Flow ID",
#endif
    BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD =
{
    "CUROFFSET",
#if RU_INCLUDE_DESC
    "Current_offset",
    "Current offset",
#endif
    BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0LSB_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0LSB_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX0LSB_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0LSB_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0LSB_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0MSB_CURBN
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD =
{
    "CURBN",
#if RU_INCLUDE_DESC
    "Current_BN",
    "Current BN",
#endif
    BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0MSB_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0MSB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX0MSB_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0MSB_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0MSB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD =
{
    "FIRSTBN",
#if RU_INCLUDE_DESC
    "First_BN",
    "First BN",
#endif
    BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0MSB_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0MSB_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX0MSB_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0MSB_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0MSB_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1LSB_INREASS
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD =
{
    "INREASS",
#if RU_INCLUDE_DESC
    "In_reassembly",
    "In reassembly."
    "Not relevant for Ethernet.",
#endif
    BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1LSB_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1LSB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX1LSB_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1LSB_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1LSB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1LSB_FLOWID
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "Flow_ID",
    "Flow ID",
#endif
    BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD =
{
    "CUROFFSET",
#if RU_INCLUDE_DESC
    "Current_offset",
    "Current offset",
#endif
    BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1LSB_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1LSB_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX1LSB_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1LSB_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1LSB_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1MSB_CURBN
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD =
{
    "CURBN",
#if RU_INCLUDE_DESC
    "Current_BN",
    "Current BN",
#endif
    BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1MSB_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1MSB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX1MSB_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1MSB_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1MSB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD =
{
    "FIRSTBN",
#if RU_INCLUDE_DESC
    "First_BN",
    "First BN",
#endif
    BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1MSB_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1MSB_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX1MSB_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1MSB_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1MSB_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD =
{
    "INREASS",
#if RU_INCLUDE_DESC
    "In_reassembly",
    "In reassembly."
    "Not relevant for Ethernet.",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0INGRESS_SOP
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD =
{
    "SOP",
#if RU_INCLUDE_DESC
    "SOP",
    "SOP",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD =
{
    "PRIORITY",
#if RU_INCLUDE_DESC
    "Priority",
    "Priority",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "Flow_ID",
    "Flow ID",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD =
{
    "CUROFFSET",
#if RU_INCLUDE_DESC
    "Current_offset",
    "Current offset",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED2_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED2_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD =
{
    "INREASS",
#if RU_INCLUDE_DESC
    "In_reassembly",
    "In reassembly."
    "Not relevant for Ethernet.",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1INGRESS_SOP
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD =
{
    "SOP",
#if RU_INCLUDE_DESC
    "SOP",
    "SOP",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD =
{
    "PRIORITY",
#if RU_INCLUDE_DESC
    "Priority",
    "Priority",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "Flow_ID",
    "Flow ID",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD =
{
    "CUROFFSET",
#if RU_INCLUDE_DESC
    "Current_offset",
    "Current offset",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED2
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED2_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED2_FIELD_WIDTH,
    BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_IBUW_UW
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_IBUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "Used_words",
    "Used words",
#endif
    BBH_RX_DEBUG_IBUW_UW_FIELD_MASK,
    0,
    BBH_RX_DEBUG_IBUW_UW_FIELD_WIDTH,
    BBH_RX_DEBUG_IBUW_UW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_IBUW_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_IBUW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_IBUW_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_IBUW_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_IBUW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_BBUW_UW
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_BBUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "Used_words",
    "Used words",
#endif
    BBH_RX_DEBUG_BBUW_UW_FIELD_MASK,
    0,
    BBH_RX_DEBUG_BBUW_UW_FIELD_WIDTH,
    BBH_RX_DEBUG_BBUW_UW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_BBUW_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_BBUW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_BBUW_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_BBUW_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_BBUW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CFUW_UW
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CFUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "Used_words",
    "Used words",
#endif
    BBH_RX_DEBUG_CFUW_UW_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CFUW_UW_FIELD_WIDTH,
    BBH_RX_DEBUG_CFUW_UW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CFUW_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CFUW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CFUW_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CFUW_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CFUW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_ACKCNT_SDMA
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_ACKCNT_SDMA_FIELD =
{
    "SDMA",
#if RU_INCLUDE_DESC
    "SDMA",
    "SDMA ACK counter",
#endif
    BBH_RX_DEBUG_ACKCNT_SDMA_FIELD_MASK,
    0,
    BBH_RX_DEBUG_ACKCNT_SDMA_FIELD_WIDTH,
    BBH_RX_DEBUG_ACKCNT_SDMA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_ACKCNT_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_ACKCNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_ACKCNT_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_ACKCNT_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_ACKCNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_ACKCNT_CONNECT
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD =
{
    "CONNECT",
#if RU_INCLUDE_DESC
    "Connect",
    "Connect ACK counter",
#endif
    BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD_MASK,
    0,
    BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD_WIDTH,
    BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_ACKCNT_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_ACKCNT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_ACKCNT_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_ACKCNT_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_ACKCNT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_COHERENCYCNT_NORMAL
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD =
{
    "NORMAL",
#if RU_INCLUDE_DESC
    "Normal",
    "Normal",
#endif
    BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD_MASK,
    0,
    BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD_WIDTH,
    BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_COHERENCYCNT_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_COHERENCYCNT_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_COHERENCYCNT_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_COHERENCYCNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD =
{
    "EXCLUSIVE",
#if RU_INCLUDE_DESC
    "Exclusive",
    "Exclusive",
#endif
    BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD_MASK,
    0,
    BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD_WIDTH,
    BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_COHERENCYCNT_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_COHERENCYCNT_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_COHERENCYCNT_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_COHERENCYCNT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_DBGVEC_DBGVEC
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD =
{
    "DBGVEC",
#if RU_INCLUDE_DESC
    "Debug_vector",
    "selected debug vector",
#endif
    BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_MASK,
    0,
    BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_WIDTH,
    BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_DBGVEC_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_DBGVEC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_DBGVEC_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_DBGVEC_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_DBGVEC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_UFUW_UW
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_UFUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "Used_words",
    "Used words",
#endif
    BBH_RX_DEBUG_UFUW_UW_FIELD_MASK,
    0,
    BBH_RX_DEBUG_UFUW_UW_FIELD_WIDTH,
    BBH_RX_DEBUG_UFUW_UW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_UFUW_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_UFUW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_UFUW_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_UFUW_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_UFUW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CREDITCNT_NORMAL
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD =
{
    "NORMAL",
#if RU_INCLUDE_DESC
    "Normal",
    "Normal",
#endif
    BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD_WIDTH,
    BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CREDITCNT_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CREDITCNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CREDITCNT_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CREDITCNT_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CREDITCNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD =
{
    "EXCLUSIVE",
#if RU_INCLUDE_DESC
    "Exclusive",
    "Exclusive",
#endif
    BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD_WIDTH,
    BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CREDITCNT_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CREDITCNT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CREDITCNT_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CREDITCNT_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_CREDITCNT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SDMACNT_UCD
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SDMACNT_UCD_FIELD =
{
    "UCD",
#if RU_INCLUDE_DESC
    "Used_CD",
    "Used CDs",
#endif
    BBH_RX_DEBUG_SDMACNT_UCD_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SDMACNT_UCD_FIELD_WIDTH,
    BBH_RX_DEBUG_SDMACNT_UCD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SDMACNT_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SDMACNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_SDMACNT_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SDMACNT_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_SDMACNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CMFUW_UW
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CMFUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "Used_words",
    "Used words",
#endif
    BBH_RX_DEBUG_CMFUW_UW_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CMFUW_UW_FIELD_WIDTH,
    BBH_RX_DEBUG_CMFUW_UW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CMFUW_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CMFUW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_CMFUW_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CMFUW_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_CMFUW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SBNFIFO_BNENTRY
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD =
{
    "BNENTRY",
#if RU_INCLUDE_DESC
    "BN_entry",
    "BN",
#endif
    BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD_WIDTH,
    BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SBNFIFO_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_SBNFIFO_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SBNFIFO_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_SBNFIFO_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SBNFIFO_VALID
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "VALID",
    "SBN is Valid",
#endif
    BBH_RX_DEBUG_SBNFIFO_VALID_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SBNFIFO_VALID_FIELD_WIDTH,
    BBH_RX_DEBUG_SBNFIFO_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SBNFIFO_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_SBNFIFO_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SBNFIFO_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_SBNFIFO_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_CMDFIFO_CMDENTRY
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD =
{
    "CMDENTRY",
#if RU_INCLUDE_DESC
    "CMD_entry",
    "CMD",
#endif
    BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD_MASK,
    0,
    BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD_WIDTH,
    BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD =
{
    "BNENTRY",
#if RU_INCLUDE_DESC
    "BN_entry",
    "BN",
#endif
    BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD_WIDTH,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "VALID",
    "SBN is Valid",
#endif
    BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD_WIDTH,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_COHERENCYCNT2_CDSENT
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD =
{
    "CDSENT",
#if RU_INCLUDE_DESC
    "CD_sent",
    "CD sent",
#endif
    BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD_MASK,
    0,
    BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD_WIDTH,
    BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_COHERENCYCNT2_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_COHERENCYCNT2_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_COHERENCYCNT2_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_COHERENCYCNT2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD =
{
    "ACKRECEIVED",
#if RU_INCLUDE_DESC
    "ACK_received",
    "EOP ACK received",
#endif
    BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD_MASK,
    0,
    BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD_WIDTH,
    BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_COHERENCYCNT2_RESERVED1
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_COHERENCYCNT2_RESERVED1_FIELD_MASK,
    0,
    BBH_RX_DEBUG_COHERENCYCNT2_RESERVED1_FIELD_WIDTH,
    BBH_RX_DEBUG_COHERENCYCNT2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD =
{
    "DISPSTATUS",
#if RU_INCLUDE_DESC
    "DISP_STATUS",
    "Dispatcher drop due to coherency FIFO full."
    "Writing 1 to this bit clears it"
    "",
#endif
    BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD_MASK,
    0,
    BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD_WIDTH,
    BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD =
{
    "SDMASTATUS",
#if RU_INCLUDE_DESC
    "SDMA_STATUS",
    "SDMA drop due to coherency method 2 counters over 63 (dec)."
    "Writing 1 to this bit clears it",
#endif
    BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD_MASK,
    0,
    BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD_WIDTH,
    BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: BBH_RX_DEBUG_DROPSTATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec BBH_RX_DEBUG_DROPSTATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    BBH_RX_DEBUG_DROPSTATUS_RESERVED0_FIELD_MASK,
    0,
    BBH_RX_DEBUG_DROPSTATUS_RESERVED0_FIELD_WIDTH,
    BBH_RX_DEBUG_DROPSTATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_BBCFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_BBCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG = 
{
    "GENERAL_CONFIGURATION_BBCFG",
#if RU_INCLUDE_DESC
    "BROADBUS_CONFIGURATION Register",
    "Each BBH unit has its own position on the BB tree. The BB defines the Route address for the specific unit.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG_OFFSET,
    0,
    0,
    621,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG = 
{
    "GENERAL_CONFIGURATION_DISPVIQ",
#if RU_INCLUDE_DESC
    "DISPATCHER_FLOW Register",
    "For every reassembled packet in the PSRAM the BBH writes a packet descriptor (PD) into the Dispatcher. The PDs are arranged using a link list in the Dispatcher. The Dispatcher has 32 virtual queues (ingress queues) and the BBH may be assigned to each of the 32 virtual queues of the Dispatcher"
    "This register defines virtual queue for normal and exclusive packets.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG_OFFSET,
    0,
    0,
    622,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG = 
{
    "GENERAL_CONFIGURATION_PATTERNDATALSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_DATA_LSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG_OFFSET,
    0,
    0,
    623,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG = 
{
    "GENERAL_CONFIGURATION_PATTERNDATAMSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_DATA_MSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG_OFFSET,
    0,
    0,
    624,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG = 
{
    "GENERAL_CONFIGURATION_PATTERNMASKLSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_MASK_LSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG_OFFSET,
    0,
    0,
    625,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG = 
{
    "GENERAL_CONFIGURATION_PATTERNMASKMSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_MASK_MSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG_OFFSET,
    0,
    0,
    626,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED2_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG = 
{
    "GENERAL_CONFIGURATION_EXCLQCFG",
#if RU_INCLUDE_DESC
    "EXCLUSIVE_QUEUE_CFG Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to special packet types (e.g. pause)."
    "This register enables this function",
#endif
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG_OFFSET,
    0,
    0,
    627,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_SDMAADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG = 
{
    "GENERAL_CONFIGURATION_SDMAADDR",
#if RU_INCLUDE_DESC
    "SDMA_ADDRESS_CONFIGURATION Register",
    "The BBH reassembles the incoming data in the SRAM. The Data is written into the SRAM using the SDMA. The data is organized in a configurable number of chunks of 128 bytes."
    "The BBH arranges the written data in the SDMA in these chunks. It arranges the data in a predefined address space in the SDMA memory and manages the chunks in a cyclic FIFO style."
    "For every write chunk the BBH writes a write descriptor. The write descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style as well."
    "This register defines the Data and descriptors base addresses.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG_OFFSET,
    0,
    0,
    628,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_SDMACFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SDMACFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG = 
{
    "GENERAL_CONFIGURATION_SDMACFG",
#if RU_INCLUDE_DESC
    "SDMA_CONFIGURATION Register",
    "The BBH reassembles the incoming data in the SRAM. The Data is written into the SRAM using the SDMA. The data is organized in a configurable number of chunks of 128 bytes."
    "The BBH arranges the written data in the SDMA in these chunks. It arranges the data in a predefined address space in the SDMA memory and manages the chunks in a cyclic FIFO style."
    "For every write chunk the BBH writes a write descriptor. The write descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style as well."
    ""
    "The BBH handles the congestion over the SDMA write chunks according to 2 priorities (low + high, exclusive). This field defines the number of occupied write chunks for dropping normal or high priority packets. If the number of occupied chunk is lower than this threshold, then all packets are passed. If the number of occupied chunk is equal or higher than this threshold, then only exclusive priority packets are passed."
    ""
    "This register defines the Data and descriptors FIFO sizes and the exclusive threshold.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG_OFFSET,
    0,
    0,
    629,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MINPKT0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MINPKT0_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG = 
{
    "GENERAL_CONFIGURATION_MINPKT0",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SIZE Register",
    "There are 4 global configuration for Minimum packet size. Each flow can get one out of these 4 global configurations."
    "Packets shorter than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG_OFFSET,
    0,
    0,
    630,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MAXPKT0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG = 
{
    "GENERAL_CONFIGURATION_MAXPKT0",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SIZE_0 Register",
    "There are 4 global configuration for Maximum packet size. Each flow can get one out of these 4 global configurations."
    "Packets longer than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG_OFFSET,
    0,
    0,
    631,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MAXPKT1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG = 
{
    "GENERAL_CONFIGURATION_MAXPKT1",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SIZE_1 Register",
    "There are 4 global configuration for Maximum packet size. Each flow can get one out of these 4 global configurations."
    "Packets longer than this threshold will be discarded.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG_OFFSET,
    0,
    0,
    632,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG = 
{
    "GENERAL_CONFIGURATION_SOPOFFSET",
#if RU_INCLUDE_DESC
    "SOP_OFFSET Register",
    "The BBH writes the packets into the PSRAM. The start of data offset is configurable. This register defines the SOP (start of packet) offset.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG_OFFSET,
    0,
    0,
    633,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG = 
{
    "GENERAL_CONFIGURATION_FLOWCTRL",
#if RU_INCLUDE_DESC
    "FLOW_CONTROL_CONFIGURATION Register",
    "The BBH manages a flow control indication towards the Ethernet MAC according to BB messages from the FW."
    "Each FW command will assert the flow control indication towards the Ethernet MAC and will trigger a timer. When the timer expires, the BBH will de-assert the flow control indication."
    "This register also disable BBH packet drop due to no space in the SDMA, SBPM or Dispatcher.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG_OFFSET,
    0,
    0,
    634,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG = 
{
    "GENERAL_CONFIGURATION_CRCOMITDIS",
#if RU_INCLUDE_DESC
    "CRC_OMIT_DISABLE Register",
    "The BBH omits the 4 CRC bytes of the packet for all packets except PLOAMs and OMCI (marked as exclusive priority)."
    "The configuration will disable this functionality.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG_OFFSET,
    0,
    0,
    635,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_ENABLE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_ENABLE_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG = 
{
    "GENERAL_CONFIGURATION_ENABLE",
#if RU_INCLUDE_DESC
    "BBH_ENABLE Register",
    "Controls the BBH enable configuration",
#endif
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG_OFFSET,
    0,
    0,
    636,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_G9991EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_G9991EN_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_G9991EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG = 
{
    "GENERAL_CONFIGURATION_G9991EN",
#if RU_INCLUDE_DESC
    "G999_1_ENABLE Register",
    "When asserted, G999.1 fragments are received by the BBH."
    "The BBH will pass the G999.1 header in the PD instead of the 1588 time-stamp.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG_OFFSET,
    0,
    0,
    637,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG = 
{
    "GENERAL_CONFIGURATION_PERFLOWTH",
#if RU_INCLUDE_DESC
    "PER_FLOW_THRESHOLD Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines X.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG_OFFSET,
    0,
    0,
    638,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG = 
{
    "GENERAL_CONFIGURATION_PERFLOWSETS",
#if RU_INCLUDE_DESC
    "PER_FLOW_SETS Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the configurations sets.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG_OFFSET,
    0,
    0,
    639,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG = 
{
    "GENERAL_CONFIGURATION_MINPKTSEL0",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SELECT_0 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the minimum packet size for flows 0-15.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG_OFFSET,
    0,
    0,
    640,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG = 
{
    "GENERAL_CONFIGURATION_MINPKTSEL1",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SELECT_1 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the minimum packet size for flows 16-31.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG_OFFSET,
    0,
    0,
    641,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG = 
{
    "GENERAL_CONFIGURATION_MAXPKTSEL0",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SELECT_0 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the maximum packet size for flows 0-15.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG_OFFSET,
    0,
    0,
    642,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG = 
{
    "GENERAL_CONFIGURATION_MAXPKTSEL1",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SELECT_1 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow."
    "Flows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations."
    "X is configurable."
    "This register defines the maximum packet size for flows 16-31.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG_OFFSET,
    0,
    0,
    643,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_MACMODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MACMODE_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG = 
{
    "GENERAL_CONFIGURATION_MACMODE",
#if RU_INCLUDE_DESC
    "MAC_MODE Register",
    "When the BBH functions as a PON BBH, this bit selects between N/X/GPON/2 and 10G/EPON functionality",
#endif
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG_OFFSET,
    0,
    0,
    644,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_SBPMCFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG = 
{
    "GENERAL_CONFIGURATION_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "Configure max on the fly requests to SBPM",
#endif
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG_OFFSET,
    0,
    0,
    645,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG = 
{
    "GENERAL_CONFIGURATION_RXRSTRST",
#if RU_INCLUDE_DESC
    "RX_RESET_COMMAND Register",
    "This register enable reset of internal units (for WA perposes).",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG_OFFSET,
    0,
    0,
    646,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG = 
{
    "GENERAL_CONFIGURATION_RXDBGSEL",
#if RU_INCLUDE_DESC
    "RX_DEBUG_SELECT Register",
    "Selects one out of 10 possible debug vectors",
#endif
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG_OFFSET,
    0,
    0,
    647,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG = 
{
    "GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER",
#if RU_INCLUDE_DESC
    "BBH_RX_RADDR_DECODER Register",
    "This register enables changing the route address for a specified BB ID",
#endif
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG_OFFSET,
    0,
    0,
    648,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_NONETH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_NONETH_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_NONETH_REG = 
{
    "GENERAL_CONFIGURATION_NONETH",
#if RU_INCLUDE_DESC
    "NON_ETHERNET_FLOW Register",
    "There an option to disable CRC error counting for this flow.",
#endif
    BBH_RX_GENERAL_CONFIGURATION_NONETH_REG_OFFSET,
    0,
    0,
    649,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG = 
{
    "GENERAL_CONFIGURATION_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    650,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_INPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_INPKT_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_INPKT_REG = 
{
    "PM_COUNTERS_INPKT",
#if RU_INCLUDE_DESC
    "INCOMING_PACKETS Register",
    "This counter counts the number of incoming good packets."
    "It counts the packets from all flows together."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_INPKT_REG_OFFSET,
    0,
    0,
    651,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_INPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_THIRDFLOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_THIRDFLOW_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_THIRDFLOW_REG = 
{
    "PM_COUNTERS_THIRDFLOW",
#if RU_INCLUDE_DESC
    "THIRD_FLOW_ERROR Register",
    "This counter counts the packets drop due to Third flow error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_THIRDFLOW_REG_OFFSET,
    0,
    0,
    652,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_THIRDFLOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_SOPASOP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_SOPASOP_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_SOPASOP_REG = 
{
    "PM_COUNTERS_SOPASOP",
#if RU_INCLUDE_DESC
    "SOP_AFTER_SOP_ERROR Register",
    "This counter counts the packets drop due to SOP after SOP error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_SOPASOP_REG_OFFSET,
    0,
    0,
    653,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_SOPASOP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_TOOSHORT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_TOOSHORT_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_TOOSHORT_REG = 
{
    "PM_COUNTERS_TOOSHORT",
#if RU_INCLUDE_DESC
    "TOO_SHORT_ERROR Register",
    "This counter counts the packets drop due to Too short error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_TOOSHORT_REG_OFFSET,
    0,
    0,
    654,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_TOOSHORT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_TOOLONG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_TOOLONG_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_TOOLONG_REG = 
{
    "PM_COUNTERS_TOOLONG",
#if RU_INCLUDE_DESC
    "TOO_LONG_ERROR Register",
    "This counter counts the packets drop due to Too long error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_TOOLONG_REG_OFFSET,
    0,
    0,
    655,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_TOOLONG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_CRCERROR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_CRCERROR_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_CRCERROR_REG = 
{
    "PM_COUNTERS_CRCERROR",
#if RU_INCLUDE_DESC
    "CRC_ERROR Register",
    "This counter counts the packets drop due to CRC error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_CRCERROR_REG_OFFSET,
    0,
    0,
    656,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_CRCERROR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_ENCRYPTERROR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_ENCRYPTERROR_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG = 
{
    "PM_COUNTERS_ENCRYPTERROR",
#if RU_INCLUDE_DESC
    "ENCRYPTION_ERROR Register",
    "This counter counts the packets drop due to XGPON encryption error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG_OFFSET,
    0,
    0,
    657,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_ENCRYPTERROR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_DISPCONG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_DISPCONG_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_DISPCONG_REG = 
{
    "PM_COUNTERS_DISPCONG",
#if RU_INCLUDE_DESC
    "DISPATCHER_CONGESTION_ERROR Register",
    "This counter counts the packets drop due to Dispatcher congestion error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_DISPCONG_REG_OFFSET,
    0,
    0,
    658,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_DISPCONG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_NOSBPMSBN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_NOSBPMSBN_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_NOSBPMSBN_REG = 
{
    "PM_COUNTERS_NOSBPMSBN",
#if RU_INCLUDE_DESC
    "NO_SBPM_SBN_ERROR Register",
    "This counter counts the packets drop due to NO SBPM SBN error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_NOSBPMSBN_REG_OFFSET,
    0,
    0,
    659,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_NOSBPMSBN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_NOSDMACD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_NOSDMACD_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_NOSDMACD_REG = 
{
    "PM_COUNTERS_NOSDMACD",
#if RU_INCLUDE_DESC
    "NO_SDMA_CD_ERROR Register",
    "This counter counts the packets drop due to No SDMA CD error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_NOSDMACD_REG_OFFSET,
    0,
    0,
    660,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_NOSDMACD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_INPLOAM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_INPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_INPLOAM_REG = 
{
    "PM_COUNTERS_INPLOAM",
#if RU_INCLUDE_DESC
    "INCOMING_PLOAM Register",
    "This counter counts the number of incoming good PLOAMs."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_INPLOAM_REG_OFFSET,
    0,
    0,
    661,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_INPLOAM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_CRCERRORPLOAM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_CRCERRORPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG = 
{
    "PM_COUNTERS_CRCERRORPLOAM",
#if RU_INCLUDE_DESC
    "CRC_PLOAM_ERROR Register",
    "This counter counts the PLOAMs drop due to CRC error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG_OFFSET,
    0,
    0,
    662,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_CRCERRORPLOAM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_DISPCONGPLOAM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_DISPCONGPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG = 
{
    "PM_COUNTERS_DISPCONGPLOAM",
#if RU_INCLUDE_DESC
    "DISPATCHER_CONGESTION_PLOAM_ERROR Register",
    "This counter counts the packets drop due to Dispatcher congestion error for PLOAM."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG_OFFSET,
    0,
    0,
    663,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_DISPCONGPLOAM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG = 
{
    "PM_COUNTERS_NOSBPMSBNPLOAM",
#if RU_INCLUDE_DESC
    "NO_SBPM_SBN_PLOAM_ERROR Register",
    "This counter counts the PLOAMs drop due to No SBPM SBN error."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG_OFFSET,
    0,
    0,
    664,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_NOSDMACDPLOAM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG = 
{
    "PM_COUNTERS_NOSDMACDPLOAM",
#if RU_INCLUDE_DESC
    "NO_SDMA_CD_PLOAM_ERROR Register",
    "This counter counts the packets drop due to No SDMA CD error for PLOAMs."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG_OFFSET,
    0,
    0,
    665,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_EPONTYPERROR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_EPONTYPERROR_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_EPONTYPERROR_REG = 
{
    "PM_COUNTERS_EPONTYPERROR",
#if RU_INCLUDE_DESC
    "EPON_TYPE_ERROR Register",
    "This counter counts the events of EPON type sequence which is wrong, meaning no sop after header, or sop/header in the middle of packet (before eop)."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_EPONTYPERROR_REG_OFFSET,
    0,
    0,
    666,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_EPONTYPERROR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_PM_COUNTERS_RUNTERROR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_RUNTERROR_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD,
    &BBH_RX_PM_COUNTERS_RUNTERROR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_PM_COUNTERS_RUNTERROR_REG = 
{
    "PM_COUNTERS_RUNTERROR",
#if RU_INCLUDE_DESC
    "RUNT_ERROR Register",
    "This counter counts the number of RUNT packets received from the XLMAC."
    "This counter is cleared when read and freezes when reaches the maximum value.",
#endif
    BBH_RX_PM_COUNTERS_RUNTERROR_REG_OFFSET,
    0,
    0,
    667,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_PM_COUNTERS_RUNTERROR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX0LSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX0LSB_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD,
    &BBH_RX_DEBUG_CNTXTX0LSB_RESERVED0_FIELD,
    &BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD,
    &BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD,
    &BBH_RX_DEBUG_CNTXTX0LSB_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CNTXTX0LSB_REG = 
{
    "DEBUG_CNTXTX0LSB",
#if RU_INCLUDE_DESC
    "CONTEXT_0_LSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX0LSB_REG_OFFSET,
    0,
    0,
    668,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_RX_DEBUG_CNTXTX0LSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX0MSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX0MSB_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD,
    &BBH_RX_DEBUG_CNTXTX0MSB_RESERVED0_FIELD,
    &BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD,
    &BBH_RX_DEBUG_CNTXTX0MSB_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CNTXTX0MSB_REG = 
{
    "DEBUG_CNTXTX0MSB",
#if RU_INCLUDE_DESC
    "CONTEXT_0_MSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX0MSB_REG_OFFSET,
    0,
    0,
    669,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_CNTXTX0MSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX1LSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX1LSB_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD,
    &BBH_RX_DEBUG_CNTXTX1LSB_RESERVED0_FIELD,
    &BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD,
    &BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD,
    &BBH_RX_DEBUG_CNTXTX1LSB_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CNTXTX1LSB_REG = 
{
    "DEBUG_CNTXTX1LSB",
#if RU_INCLUDE_DESC
    "CONTEXT_1_LSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX1LSB_REG_OFFSET,
    0,
    0,
    670,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_RX_DEBUG_CNTXTX1LSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX1MSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX1MSB_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD,
    &BBH_RX_DEBUG_CNTXTX1MSB_RESERVED0_FIELD,
    &BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD,
    &BBH_RX_DEBUG_CNTXTX1MSB_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CNTXTX1MSB_REG = 
{
    "DEBUG_CNTXTX1MSB",
#if RU_INCLUDE_DESC
    "CONTEXT_1_MSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX1MSB_REG_OFFSET,
    0,
    0,
    671,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_CNTXTX1MSB_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX0INGRESS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX0INGRESS_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED0_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED1_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CNTXTX0INGRESS_REG = 
{
    "DEBUG_CNTXTX0INGRESS",
#if RU_INCLUDE_DESC
    "INGRESS_CONTEXT_0 Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX0INGRESS_REG_OFFSET,
    0,
    0,
    672,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_RX_DEBUG_CNTXTX0INGRESS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CNTXTX1INGRESS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX1INGRESS_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED0_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED1_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CNTXTX1INGRESS_REG = 
{
    "DEBUG_CNTXTX1INGRESS",
#if RU_INCLUDE_DESC
    "INGRESS_CONTEXT_1 Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.",
#endif
    BBH_RX_DEBUG_CNTXTX1INGRESS_REG_OFFSET,
    0,
    0,
    673,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    BBH_RX_DEBUG_CNTXTX1INGRESS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_IBUW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_IBUW_FIELDS[] =
{
    &BBH_RX_DEBUG_IBUW_UW_FIELD,
    &BBH_RX_DEBUG_IBUW_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_IBUW_REG = 
{
    "DEBUG_IBUW",
#if RU_INCLUDE_DESC
    "INPUT_BUF_USED_WORDS Register",
    "Input buf used words",
#endif
    BBH_RX_DEBUG_IBUW_REG_OFFSET,
    0,
    0,
    674,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_IBUW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_BBUW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_BBUW_FIELDS[] =
{
    &BBH_RX_DEBUG_BBUW_UW_FIELD,
    &BBH_RX_DEBUG_BBUW_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_BBUW_REG = 
{
    "DEBUG_BBUW",
#if RU_INCLUDE_DESC
    "BURST_BUF_USED_WORDS Register",
    "Burst buf used words",
#endif
    BBH_RX_DEBUG_BBUW_REG_OFFSET,
    0,
    0,
    675,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_BBUW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CFUW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CFUW_FIELDS[] =
{
    &BBH_RX_DEBUG_CFUW_UW_FIELD,
    &BBH_RX_DEBUG_CFUW_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CFUW_REG = 
{
    "DEBUG_CFUW",
#if RU_INCLUDE_DESC
    "COHERENCY_FIFO_USED_WORDS Register",
    "Coherency FIFO used words",
#endif
    BBH_RX_DEBUG_CFUW_REG_OFFSET,
    0,
    0,
    676,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_CFUW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_ACKCNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_ACKCNT_FIELDS[] =
{
    &BBH_RX_DEBUG_ACKCNT_SDMA_FIELD,
    &BBH_RX_DEBUG_ACKCNT_RESERVED0_FIELD,
    &BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD,
    &BBH_RX_DEBUG_ACKCNT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_ACKCNT_REG = 
{
    "DEBUG_ACKCNT",
#if RU_INCLUDE_DESC
    "ACK_COUNTERS Register",
    "The register reflects 2 ACK counters:"
    "SDMA"
    "CONNECT",
#endif
    BBH_RX_DEBUG_ACKCNT_REG_OFFSET,
    0,
    0,
    677,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_ACKCNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_COHERENCYCNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_COHERENCYCNT_FIELDS[] =
{
    &BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD,
    &BBH_RX_DEBUG_COHERENCYCNT_RESERVED0_FIELD,
    &BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD,
    &BBH_RX_DEBUG_COHERENCYCNT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_COHERENCYCNT_REG = 
{
    "DEBUG_COHERENCYCNT",
#if RU_INCLUDE_DESC
    "COHERENCY_COUNTERS Register",
    "The register 2 pending coherency counters:"
    "Normal"
    "Exclusive",
#endif
    BBH_RX_DEBUG_COHERENCYCNT_REG_OFFSET,
    0,
    0,
    678,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_COHERENCYCNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_DBGVEC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_DBGVEC_FIELDS[] =
{
    &BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD,
    &BBH_RX_DEBUG_DBGVEC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_DBGVEC_REG = 
{
    "DEBUG_DBGVEC",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR Register",
    "selected debug vector",
#endif
    BBH_RX_DEBUG_DBGVEC_REG_OFFSET,
    0,
    0,
    679,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_DBGVEC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_UFUW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_UFUW_FIELDS[] =
{
    &BBH_RX_DEBUG_UFUW_UW_FIELD,
    &BBH_RX_DEBUG_UFUW_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_UFUW_REG = 
{
    "DEBUG_UFUW",
#if RU_INCLUDE_DESC
    "UPLOAD_FIFO_USED_WORDS Register",
    "Upload FIFO used words",
#endif
    BBH_RX_DEBUG_UFUW_REG_OFFSET,
    0,
    0,
    680,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_UFUW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CREDITCNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CREDITCNT_FIELDS[] =
{
    &BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD,
    &BBH_RX_DEBUG_CREDITCNT_RESERVED0_FIELD,
    &BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD,
    &BBH_RX_DEBUG_CREDITCNT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CREDITCNT_REG = 
{
    "DEBUG_CREDITCNT",
#if RU_INCLUDE_DESC
    "CREDIT_COUNTERS Register",
    "This register holds 2 credit counters:"
    "Normal"
    "Exclusive",
#endif
    BBH_RX_DEBUG_CREDITCNT_REG_OFFSET,
    0,
    0,
    681,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_CREDITCNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_SDMACNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SDMACNT_FIELDS[] =
{
    &BBH_RX_DEBUG_SDMACNT_UCD_FIELD,
    &BBH_RX_DEBUG_SDMACNT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_SDMACNT_REG = 
{
    "DEBUG_SDMACNT",
#if RU_INCLUDE_DESC
    "USED_SDMA_CD_CNT Register",
    "Number of used SDMA CDs",
#endif
    BBH_RX_DEBUG_SDMACNT_REG_OFFSET,
    0,
    0,
    682,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_SDMACNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CMFUW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CMFUW_FIELDS[] =
{
    &BBH_RX_DEBUG_CMFUW_UW_FIELD,
    &BBH_RX_DEBUG_CMFUW_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CMFUW_REG = 
{
    "DEBUG_CMFUW",
#if RU_INCLUDE_DESC
    "CMD_FIFO_USED_WORDS Register",
    "CMD FIFO used words",
#endif
    BBH_RX_DEBUG_CMFUW_REG_OFFSET,
    0,
    0,
    683,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_CMFUW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_SBNFIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SBNFIFO_FIELDS[] =
{
    &BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD,
    &BBH_RX_DEBUG_SBNFIFO_RESERVED0_FIELD,
    &BBH_RX_DEBUG_SBNFIFO_VALID_FIELD,
    &BBH_RX_DEBUG_SBNFIFO_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_SBNFIFO_REG = 
{
    "DEBUG_SBNFIFO",
#if RU_INCLUDE_DESC
    "SRAM_BN_FIFO %i Register",
    "The BBH RX hold a FIFO with 16 BN.",
#endif
    BBH_RX_DEBUG_SBNFIFO_REG_OFFSET,
    BBH_RX_DEBUG_SBNFIFO_REG_RAM_CNT,
    4,
    684,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_SBNFIFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_CMDFIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CMDFIFO_FIELDS[] =
{
    &BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_CMDFIFO_REG = 
{
    "DEBUG_CMDFIFO",
#if RU_INCLUDE_DESC
    "CMD_FIFO %i Register",
    "The BBH RX hold a FIFO with 8 command.",
#endif
    BBH_RX_DEBUG_CMDFIFO_REG_OFFSET,
    BBH_RX_DEBUG_CMDFIFO_REG_RAM_CNT,
    4,
    685,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_CMDFIFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_SBNRECYCLEFIFO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SBNRECYCLEFIFO_FIELDS[] =
{
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD,
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED0_FIELD,
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD,
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_REG = 
{
    "DEBUG_SBNRECYCLEFIFO",
#if RU_INCLUDE_DESC
    "SRAM_BN_RECYCLE_FIFO %i Register",
    "The BBH RX hold a recycle FIFO with up to 2 BN.",
#endif
    BBH_RX_DEBUG_SBNRECYCLEFIFO_REG_OFFSET,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_REG_RAM_CNT,
    4,
    686,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_COHERENCYCNT2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_COHERENCYCNT2_FIELDS[] =
{
    &BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD,
    &BBH_RX_DEBUG_COHERENCYCNT2_RESERVED0_FIELD,
    &BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD,
    &BBH_RX_DEBUG_COHERENCYCNT2_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_COHERENCYCNT2_REG = 
{
    "DEBUG_COHERENCYCNT2",
#if RU_INCLUDE_DESC
    "COHERENCY_COUNTERS_METHOD2 Register",
    "Read of 4 coherency counters:"
    "CD CMD sent (1 per flow)"
    "EOP ACK received (1 per flow)",
#endif
    BBH_RX_DEBUG_COHERENCYCNT2_REG_OFFSET,
    0,
    0,
    687,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_COHERENCYCNT2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: BBH_RX_DEBUG_DROPSTATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_DROPSTATUS_FIELDS[] =
{
    &BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD,
    &BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD,
    &BBH_RX_DEBUG_DROPSTATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec BBH_RX_DEBUG_DROPSTATUS_REG = 
{
    "DEBUG_DROPSTATUS",
#if RU_INCLUDE_DESC
    "SPECIAL_DROP_STATUS Register",
    "Information of the following:"
    "- Dispatcher drop due to coherency FIFO full"
    "- SDMA drop due to coherency method 2 counters over 63 (dec)"
    "",
#endif
    BBH_RX_DEBUG_DROPSTATUS_REG_OFFSET,
    0,
    0,
    688,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_DEBUG_DROPSTATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: BBH_RX
 ******************************************************************************/
static const ru_reg_rec *BBH_RX_REGS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG,
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG,
    &BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG,
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_REG,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG,
    &BBH_RX_PM_COUNTERS_INPKT_REG,
    &BBH_RX_PM_COUNTERS_THIRDFLOW_REG,
    &BBH_RX_PM_COUNTERS_SOPASOP_REG,
    &BBH_RX_PM_COUNTERS_TOOSHORT_REG,
    &BBH_RX_PM_COUNTERS_TOOLONG_REG,
    &BBH_RX_PM_COUNTERS_CRCERROR_REG,
    &BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG,
    &BBH_RX_PM_COUNTERS_DISPCONG_REG,
    &BBH_RX_PM_COUNTERS_NOSBPMSBN_REG,
    &BBH_RX_PM_COUNTERS_NOSDMACD_REG,
    &BBH_RX_PM_COUNTERS_INPLOAM_REG,
    &BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG,
    &BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG,
    &BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG,
    &BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG,
    &BBH_RX_PM_COUNTERS_EPONTYPERROR_REG,
    &BBH_RX_PM_COUNTERS_RUNTERROR_REG,
    &BBH_RX_DEBUG_CNTXTX0LSB_REG,
    &BBH_RX_DEBUG_CNTXTX0MSB_REG,
    &BBH_RX_DEBUG_CNTXTX1LSB_REG,
    &BBH_RX_DEBUG_CNTXTX1MSB_REG,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_REG,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_REG,
    &BBH_RX_DEBUG_IBUW_REG,
    &BBH_RX_DEBUG_BBUW_REG,
    &BBH_RX_DEBUG_CFUW_REG,
    &BBH_RX_DEBUG_ACKCNT_REG,
    &BBH_RX_DEBUG_COHERENCYCNT_REG,
    &BBH_RX_DEBUG_DBGVEC_REG,
    &BBH_RX_DEBUG_UFUW_REG,
    &BBH_RX_DEBUG_CREDITCNT_REG,
    &BBH_RX_DEBUG_SDMACNT_REG,
    &BBH_RX_DEBUG_CMFUW_REG,
    &BBH_RX_DEBUG_SBNFIFO_REG,
    &BBH_RX_DEBUG_CMDFIFO_REG,
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_REG,
    &BBH_RX_DEBUG_COHERENCYCNT2_REG,
    &BBH_RX_DEBUG_DROPSTATUS_REG,
};

unsigned long BBH_RX_ADDRS[] =
{
    0x82d94000,
    0x82d94400,
    0x82d94800,
    0x82d94c00,
    0x82d95000,
    0x82d95400,
    0x82d95800,
};

const ru_block_rec BBH_RX_BLOCK = 
{
    "BBH_RX",
    BBH_RX_ADDRS,
    7,
    68,
    BBH_RX_REGS
};

/* End of file XRDP_BBH_RX.c */
