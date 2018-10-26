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
 * Field: XLIF_EEE_IND_LPI_RX_DETECT
 ******************************************************************************/
const ru_field_rec XLIF_EEE_IND_LPI_RX_DETECT_FIELD =
{
    "LPI_RX_DETECT",
#if RU_INCLUDE_DESC
    "lpi_rx_detect",
    "lpi_rx_detect",
#endif
    XLIF_EEE_IND_LPI_RX_DETECT_FIELD_MASK,
    0,
    XLIF_EEE_IND_LPI_RX_DETECT_FIELD_WIDTH,
    XLIF_EEE_IND_LPI_RX_DETECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_EEE_IND_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_EEE_IND_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_EEE_IND_RESERVED0_FIELD_MASK,
    0,
    XLIF_EEE_IND_RESERVED0_FIELD_WIDTH,
    XLIF_EEE_IND_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_EEE_IND_LPI_TX_DETECT
 ******************************************************************************/
const ru_field_rec XLIF_EEE_IND_LPI_TX_DETECT_FIELD =
{
    "LPI_TX_DETECT",
#if RU_INCLUDE_DESC
    "lpi_tx_detect",
    "lpi_tx_detect",
#endif
    XLIF_EEE_IND_LPI_TX_DETECT_FIELD_MASK,
    0,
    XLIF_EEE_IND_LPI_TX_DETECT_FIELD_WIDTH,
    XLIF_EEE_IND_LPI_TX_DETECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_EEE_IND_RESERVED1
 ******************************************************************************/
const ru_field_rec XLIF_EEE_IND_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_EEE_IND_RESERVED1_FIELD_MASK,
    0,
    XLIF_EEE_IND_RESERVED1_FIELD_WIDTH,
    XLIF_EEE_IND_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XLIF_EEE_IND
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_EEE_IND_FIELDS[] =
{
    &XLIF_EEE_IND_LPI_RX_DETECT_FIELD,
    &XLIF_EEE_IND_RESERVED0_FIELD,
    &XLIF_EEE_IND_LPI_TX_DETECT_FIELD,
    &XLIF_EEE_IND_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_EEE_IND_REG = 
{
    "IND",
#if RU_INCLUDE_DESC
    "INDICATIONS Register",
    "eee indications from the XLMAC interface",
#endif
    XLIF_EEE_IND_REG_OFFSET,
    0,
    0,
    1136,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XLIF_EEE_IND_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: XLIF_EEE
 ******************************************************************************/
static const ru_reg_rec *XLIF_EEE_REGS[] =
{
    &XLIF_EEE_IND_REG,
};

unsigned long XLIF_EEE_ADDRS[] =
{
    0x80147878,
    0x80147a78,
    0x80147c78,
    0x80147e78,
};

const ru_block_rec XLIF_EEE_BLOCK = 
{
    "XLIF_EEE",
    XLIF_EEE_ADDRS,
    4,
    1,
    XLIF_EEE_REGS
};

/* End of file XRDP_XLIF_EEE.c */
