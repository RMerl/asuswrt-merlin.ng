// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

*/

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: XPORT_WOL_ARD_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_ARD_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_WOL_ARD_CONFIG_RESERVED0_FIELD_MASK,
    0,
    XPORT_WOL_ARD_CONFIG_RESERVED0_FIELD_WIDTH,
    XPORT_WOL_ARD_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_ARD_CONFIG_HAS_BRCM_TAG
 ******************************************************************************/
const ru_field_rec XPORT_WOL_ARD_CONFIG_HAS_BRCM_TAG_FIELD =
{
    "HAS_BRCM_TAG",
#if RU_INCLUDE_DESC
    "",
    "If received packets contain the implicit 4B BRCM tag, this bit has to be set to 1.",
#endif
    XPORT_WOL_ARD_CONFIG_HAS_BRCM_TAG_FIELD_MASK,
    0,
    XPORT_WOL_ARD_CONFIG_HAS_BRCM_TAG_FIELD_WIDTH,
    XPORT_WOL_ARD_CONFIG_HAS_BRCM_TAG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_ARD_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_ARD_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_WOL_ARD_CONTROL_RESERVED0_FIELD_MASK,
    0,
    XPORT_WOL_ARD_CONTROL_RESERVED0_FIELD_WIDTH,
    XPORT_WOL_ARD_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_ARD_CONTROL_ARD_EN
 ******************************************************************************/
const ru_field_rec XPORT_WOL_ARD_CONTROL_ARD_EN_FIELD =
{
    "ARD_EN",
#if RU_INCLUDE_DESC
    "",
    "1=> enable ARP Request packet detection"
    "0=> disable ARP Request packet detection, clear ARP Request packet detection status (%%fref STATUS.AR_DETECTED%%).",
#endif
    XPORT_WOL_ARD_CONTROL_ARD_EN_FIELD_MASK,
    0,
    XPORT_WOL_ARD_CONTROL_ARD_EN_FIELD_WIDTH,
    XPORT_WOL_ARD_CONTROL_ARD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_ARD_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_WOL_ARD_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_WOL_ARD_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_WOL_ARD_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_WOL_ARD_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_ARD_STATUS_AR_DETECTED
 ******************************************************************************/
const ru_field_rec XPORT_WOL_ARD_STATUS_AR_DETECTED_FIELD =
{
    "AR_DETECTED",
#if RU_INCLUDE_DESC
    "",
    "Indicates that an ARP Request packet has been detected."
    "This status is cleared when %%fref CONTROL.ARD_EN%% is 0.",
#endif
    XPORT_WOL_ARD_STATUS_AR_DETECTED_FIELD_MASK,
    0,
    XPORT_WOL_ARD_STATUS_AR_DETECTED_FIELD_WIDTH,
    XPORT_WOL_ARD_STATUS_AR_DETECTED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE1
 ******************************************************************************/
const ru_field_rec XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE1_FIELD =
{
    "ETHERTYPE1",
#if RU_INCLUDE_DESC
    "",
    "Custom tag Ethertype #1.",
#endif
    XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE1_FIELD_MASK,
    0,
    XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE1_FIELD_WIDTH,
    XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE2
 ******************************************************************************/
const ru_field_rec XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE2_FIELD =
{
    "ETHERTYPE2",
#if RU_INCLUDE_DESC
    "",
    "Custom tag Ethertype #2.",
#endif
    XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE2_FIELD_MASK,
    0,
    XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE2_FIELD_WIDTH,
    XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_WOL_ARD_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_ARD_CONFIG_FIELDS[] =
{
    &XPORT_WOL_ARD_CONFIG_RESERVED0_FIELD,
    &XPORT_WOL_ARD_CONFIG_HAS_BRCM_TAG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_ARD_CONFIG_REG = 
{
    "CONFIG",
#if RU_INCLUDE_DESC
    "ARP Request Packet Detector Configuration Register",
    "",
#endif
    XPORT_WOL_ARD_CONFIG_REG_OFFSET,
    0,
    0,
    293,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_WOL_ARD_CONFIG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_ARD_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_ARD_CONTROL_FIELDS[] =
{
    &XPORT_WOL_ARD_CONTROL_RESERVED0_FIELD,
    &XPORT_WOL_ARD_CONTROL_ARD_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_ARD_CONTROL_REG = 
{
    "CONTROL",
#if RU_INCLUDE_DESC
    "ARP Request Packet Detector Control Register",
    "",
#endif
    XPORT_WOL_ARD_CONTROL_REG_OFFSET,
    0,
    0,
    294,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_WOL_ARD_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_ARD_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_ARD_STATUS_FIELDS[] =
{
    &XPORT_WOL_ARD_STATUS_RESERVED0_FIELD,
    &XPORT_WOL_ARD_STATUS_AR_DETECTED_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_ARD_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "ARP Request Packet Detector Status Register",
    "",
#endif
    XPORT_WOL_ARD_STATUS_REG_OFFSET,
    0,
    0,
    295,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_WOL_ARD_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_WOL_ARD_CUSTOM_TAG_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_WOL_ARD_CUSTOM_TAG_CFG_FIELDS[] =
{
    &XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE1_FIELD,
    &XPORT_WOL_ARD_CUSTOM_TAG_CFG_ETHERTYPE2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_WOL_ARD_CUSTOM_TAG_CFG_REG = 
{
    "CUSTOM_TAG_CFG",
#if RU_INCLUDE_DESC
    "ARP Request Packet Detector Custom Tag Configuration Register",
    "Custom tag Ethertypes are used in addition to two implicit VLAN tag Etherypes (which are 88a8 and 8100).",
#endif
    XPORT_WOL_ARD_CUSTOM_TAG_CFG_REG_OFFSET,
    0,
    0,
    296,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_WOL_ARD_CUSTOM_TAG_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_WOL_ARD
 ******************************************************************************/
static const ru_reg_rec *XPORT_WOL_ARD_REGS[] =
{
    &XPORT_WOL_ARD_CONFIG_REG,
    &XPORT_WOL_ARD_CONTROL_REG,
    &XPORT_WOL_ARD_STATUS_REG,
    &XPORT_WOL_ARD_CUSTOM_TAG_CFG_REG,
};

unsigned long XPORT_WOL_ARD_ADDRS[] =
{
    0x837f3520,
    0x837f7520,
};

const ru_block_rec XPORT_WOL_ARD_BLOCK = 
{
    "XPORT_WOL_ARD",
    XPORT_WOL_ARD_ADDRS,
    2,
    4,
    XPORT_WOL_ARD_REGS
};

/* End of file XPORT_WOL_ARD.c */
