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
 * Field: XPORT_WOL_MPD_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_WOL_MPD_CONFIG_RESERVED0_FIELD_MASK,
    0,
    XPORT_WOL_MPD_CONFIG_RESERVED0_FIELD_WIDTH,
    XPORT_WOL_MPD_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_CONFIG_PSW_EN
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_CONFIG_PSW_EN_FIELD =
{
    "PSW_EN",
#if RU_INCLUDE_DESC
    "",
    "1=> enable Magic Packet password check."
    "This password is an optional 6-byte sequence that follows the last MAC_DA in the Magic Packet sequence, and is specified in registers %%rref PSW_HI%% and %%rref PSW_LOW%%.",
#endif
    XPORT_WOL_MPD_CONFIG_PSW_EN_FIELD_MASK,
    0,
    XPORT_WOL_MPD_CONFIG_PSW_EN_FIELD_WIDTH,
    XPORT_WOL_MPD_CONFIG_PSW_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_CONFIG_MSEQ_LEN
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_CONFIG_MSEQ_LEN_FIELD =
{
    "MSEQ_LEN",
#if RU_INCLUDE_DESC
    "",
    "Number of repetitions of MAC Destination Address that must be detected within Magic Packet.",
#endif
    XPORT_WOL_MPD_CONFIG_MSEQ_LEN_FIELD_MASK,
    0,
    XPORT_WOL_MPD_CONFIG_MSEQ_LEN_FIELD_WIDTH,
    XPORT_WOL_MPD_CONFIG_MSEQ_LEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_WOL_MPD_CONTROL_RESERVED0_FIELD_MASK,
    0,
    XPORT_WOL_MPD_CONTROL_RESERVED0_FIELD_WIDTH,
    XPORT_WOL_MPD_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_CONTROL_MPD_EN
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_CONTROL_MPD_EN_FIELD =
{
    "MPD_EN",
#if RU_INCLUDE_DESC
    "",
    "1=> enable Magic Packet detection"
    "0=> disable Magic Packet detection, clear Magic Packet detection status.",
#endif
    XPORT_WOL_MPD_CONTROL_MPD_EN_FIELD_MASK,
    0,
    XPORT_WOL_MPD_CONTROL_MPD_EN_FIELD_WIDTH,
    XPORT_WOL_MPD_CONTROL_MPD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_WOL_MPD_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_WOL_MPD_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_WOL_MPD_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_STATUS_MP_DETECTED
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_STATUS_MP_DETECTED_FIELD =
{
    "MP_DETECTED",
#if RU_INCLUDE_DESC
    "",
    "Indicates that a Magic Packet has been detected."
    "Magic Packet detection status is cleared when %%fref CONTROL.MPD_EN%% is 0.",
#endif
    XPORT_WOL_MPD_STATUS_MP_DETECTED_FIELD_MASK,
    0,
    XPORT_WOL_MPD_STATUS_MP_DETECTED_FIELD_WIDTH,
    XPORT_WOL_MPD_STATUS_MP_DETECTED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_MAC_DA_31_0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_MAC_DA_31_0_FIELD =
{
    "MAC_DA_31_0",
#if RU_INCLUDE_DESC
    "",
    "Last (rightmost) four bytes of the MAC Destination Address repeated in the Magic Sequence."
    "Bits [31:24] correspond to the third Magic Sequence MAC Destination Address byte received from the wire."
    "Bits [23:16] correspond to the fourth Magic Sequence MAC Destination Address byte received from the wire."
    "Bits [15:8] correspond to the fifth Magic Sequence MAC Destination Address byte received from the wire."
    "Bits [7:0] correspond to the 6th Magic Sequence MAC Destination Address byte received from the wire.",
#endif
    XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_MAC_DA_31_0_FIELD_MASK,
    0,
    XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_MAC_DA_31_0_FIELD_WIDTH,
    XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_MAC_DA_31_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_MSEQ_MAC_DA_HI_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_MSEQ_MAC_DA_HI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_WOL_MPD_MSEQ_MAC_DA_HI_RESERVED0_FIELD_MASK,
    0,
    XPORT_WOL_MPD_MSEQ_MAC_DA_HI_RESERVED0_FIELD_WIDTH,
    XPORT_WOL_MPD_MSEQ_MAC_DA_HI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_MSEQ_MAC_DA_HI_MAC_DA_47_32
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_MSEQ_MAC_DA_HI_MAC_DA_47_32_FIELD =
{
    "MAC_DA_47_32",
#if RU_INCLUDE_DESC
    "",
    "First (leftomost) two bytes of the MAC Destination Address repeated in the Magic Sequence."
    "Bits [47:40] correspond to the first Magic Sequence MAC Destination Address byte received from the wire."
    "Bits [39:32] correspond to the second Magic Sequence MAC Destination Address byte received from the wire.",
#endif
    XPORT_WOL_MPD_MSEQ_MAC_DA_HI_MAC_DA_47_32_FIELD_MASK,
    0,
    XPORT_WOL_MPD_MSEQ_MAC_DA_HI_MAC_DA_47_32_FIELD_WIDTH,
    XPORT_WOL_MPD_MSEQ_MAC_DA_HI_MAC_DA_47_32_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_PSW_LOW_PSW_31_0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_PSW_LOW_PSW_31_0_FIELD =
{
    "PSW_31_0",
#if RU_INCLUDE_DESC
    "",
    "Last (rightmost) four bytes of the 6-byte Magic Packet password."
    "Bits [31:24] correspond to the third password byte received from the wire."
    "Bits [23:16] correspond to the fourth password byte received from the wire."
    "Bits [15:8] correspond to the fifth password byte received from the wire."
    "Bits [7:0] correspond to the last (6th) password byte received from the wire."
    "This setting is used only when %%fref CONFIG.PSW_EN%% is set to 1.",
#endif
    XPORT_WOL_MPD_PSW_LOW_PSW_31_0_FIELD_MASK,
    0,
    XPORT_WOL_MPD_PSW_LOW_PSW_31_0_FIELD_WIDTH,
    XPORT_WOL_MPD_PSW_LOW_PSW_31_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_PSW_HI_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_PSW_HI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_WOL_MPD_PSW_HI_RESERVED0_FIELD_MASK,
    0,
    XPORT_WOL_MPD_PSW_HI_RESERVED0_FIELD_WIDTH,
    XPORT_WOL_MPD_PSW_HI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_MPD_PSW_HI_PSW_47_32
 ******************************************************************************/
const ru_field_rec XPORT_WOL_MPD_PSW_HI_PSW_47_32_FIELD =
{
    "PSW_47_32",
#if RU_INCLUDE_DESC
    "",
    "First (leftmost) two bytes of the 6-byte Magic Packet password."
    "Bits [47:40] correspond to the first password byte received from the wire."
    "Bits [39:32] correspond to the second password byte received from the wire."
    "This setting is used only when %%fref CONFIG.PSW_EN%% is set to 1.",
#endif
    XPORT_WOL_MPD_PSW_HI_PSW_47_32_FIELD_MASK,
    0,
    XPORT_WOL_MPD_PSW_HI_PSW_47_32_FIELD_WIDTH,
    XPORT_WOL_MPD_PSW_HI_PSW_47_32_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_WOL_MPD_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_MPD_CONFIG_FIELDS[] =
{
    &XPORT_WOL_MPD_CONFIG_RESERVED0_FIELD,
    &XPORT_WOL_MPD_CONFIG_PSW_EN_FIELD,
    &XPORT_WOL_MPD_CONFIG_MSEQ_LEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_MPD_CONFIG_REG = 
{
    "CONFIG",
#if RU_INCLUDE_DESC
    "MPD Configuration Register",
    "",
#endif
    XPORT_WOL_MPD_CONFIG_REG_OFFSET,
    0,
    0,
    286,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPORT_WOL_MPD_CONFIG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_MPD_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_MPD_CONTROL_FIELDS[] =
{
    &XPORT_WOL_MPD_CONTROL_RESERVED0_FIELD,
    &XPORT_WOL_MPD_CONTROL_MPD_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_MPD_CONTROL_REG = 
{
    "CONTROL",
#if RU_INCLUDE_DESC
    "MPD Control Register",
    "",
#endif
    XPORT_WOL_MPD_CONTROL_REG_OFFSET,
    0,
    0,
    287,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_WOL_MPD_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_MPD_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_MPD_STATUS_FIELDS[] =
{
    &XPORT_WOL_MPD_STATUS_RESERVED0_FIELD,
    &XPORT_WOL_MPD_STATUS_MP_DETECTED_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_MPD_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "MPD Status Register",
    "",
#endif
    XPORT_WOL_MPD_STATUS_REG_OFFSET,
    0,
    0,
    288,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_WOL_MPD_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_MPD_MSEQ_MAC_DA_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_FIELDS[] =
{
    &XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_MAC_DA_31_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_REG = 
{
    "MSEQ_MAC_DA_LOW",
#if RU_INCLUDE_DESC
    "MPD Destination MAC Address Low Register",
    "",
#endif
    XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_REG_OFFSET,
    0,
    0,
    289,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_MPD_MSEQ_MAC_DA_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_MPD_MSEQ_MAC_DA_HI_FIELDS[] =
{
    &XPORT_WOL_MPD_MSEQ_MAC_DA_HI_RESERVED0_FIELD,
    &XPORT_WOL_MPD_MSEQ_MAC_DA_HI_MAC_DA_47_32_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_MPD_MSEQ_MAC_DA_HI_REG = 
{
    "MSEQ_MAC_DA_HI",
#if RU_INCLUDE_DESC
    "MPD Destination MAC Address High Register",
    "",
#endif
    XPORT_WOL_MPD_MSEQ_MAC_DA_HI_REG_OFFSET,
    0,
    0,
    290,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_WOL_MPD_MSEQ_MAC_DA_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_MPD_PSW_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_MPD_PSW_LOW_FIELDS[] =
{
    &XPORT_WOL_MPD_PSW_LOW_PSW_31_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_MPD_PSW_LOW_REG = 
{
    "PSW_LOW",
#if RU_INCLUDE_DESC
    "MPD Password Low Register",
    "",
#endif
    XPORT_WOL_MPD_PSW_LOW_REG_OFFSET,
    0,
    0,
    291,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_WOL_MPD_PSW_LOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_MPD_PSW_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_MPD_PSW_HI_FIELDS[] =
{
    &XPORT_WOL_MPD_PSW_HI_RESERVED0_FIELD,
    &XPORT_WOL_MPD_PSW_HI_PSW_47_32_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_MPD_PSW_HI_REG = 
{
    "PSW_HI",
#if RU_INCLUDE_DESC
    "MPD Password High Register",
    "",
#endif
    XPORT_WOL_MPD_PSW_HI_REG_OFFSET,
    0,
    0,
    292,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_WOL_MPD_PSW_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_WOL_MPD
 ******************************************************************************/
static const ru_reg_rec *XPORT_WOL_MPD_REGS[] =
{
    &XPORT_WOL_MPD_CONFIG_REG,
    &XPORT_WOL_MPD_CONTROL_REG,
    &XPORT_WOL_MPD_STATUS_REG,
    &XPORT_WOL_MPD_MSEQ_MAC_DA_LOW_REG,
    &XPORT_WOL_MPD_MSEQ_MAC_DA_HI_REG,
    &XPORT_WOL_MPD_PSW_LOW_REG,
    &XPORT_WOL_MPD_PSW_HI_REG,
};

unsigned long XPORT_WOL_MPD_ADDRS[] =
{
    0x837f3500,
    0x837f7500,
};

const ru_block_rec XPORT_WOL_MPD_BLOCK = 
{
    "XPORT_WOL_MPD",
    XPORT_WOL_MPD_ADDRS,
    2,
    7,
    XPORT_WOL_MPD_REGS
};

/* End of file XPORT_WOL_MPD.c */
