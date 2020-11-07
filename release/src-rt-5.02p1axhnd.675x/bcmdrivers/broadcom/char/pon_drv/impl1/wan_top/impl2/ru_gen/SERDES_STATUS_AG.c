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

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: SERDES_STATUS_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SERDES_STATUS_STATUS_RESERVED0_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_RESERVED0_FIELD_WIDTH,
    SERDES_STATUS_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMD_PLL1_LOCK
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMD_PLL1_LOCK_FIELD =
{
    "PMD_PLL1_LOCK",
#if RU_INCLUDE_DESC
    "",
    "Assertion of this signal indicates that the pll has achieved lock.",
#endif
    SERDES_STATUS_STATUS_PMD_PLL1_LOCK_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMD_PLL1_LOCK_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMD_PLL1_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_O_LASER_BURST_EN
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_O_LASER_BURST_EN_FIELD =
{
    "O_LASER_BURST_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, the SERDES is attempting to enable the laser.  The actual"
    "state of the laser also depends on the laser output enable.",
#endif
    SERDES_STATUS_STATUS_O_LASER_BURST_EN_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_O_LASER_BURST_EN_FIELD_WIDTH,
    SERDES_STATUS_STATUS_O_LASER_BURST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMI_LP_ERROR
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMI_LP_ERROR_FIELD =
{
    "PMI_LP_ERROR",
#if RU_INCLUDE_DESC
    "",
    "Error response from RMIC slave indicating an address error which"
    "means that either the block address does not exist or that the devid"
    "did not match the strap value. The ack signal indicates that the"
    "transaction is complete and the error signal indicates that there"
    "was an address error with this transaction. This signal is asserted"
    "along with the ack signal and should be treated an asynchronous"
    "signal the same way as the ack signal.",
#endif
    SERDES_STATUS_STATUS_PMI_LP_ERROR_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMI_LP_ERROR_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMI_LP_ERROR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMI_LP_ACKNOWLEDGE
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMI_LP_ACKNOWLEDGE_FIELD =
{
    "PMI_LP_ACKNOWLEDGE",
#if RU_INCLUDE_DESC
    "",
    "Ack response back from the RMIC slave indicating that the write or"
    "read transaction is complete. This signal is driven in the registers"
    "blocks clock domain and should be treated as an asynchronous input"
    "by the master.",
#endif
    SERDES_STATUS_STATUS_PMI_LP_ACKNOWLEDGE_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMI_LP_ACKNOWLEDGE_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMI_LP_ACKNOWLEDGE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMD_SIGNAL_DETECT_0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMD_SIGNAL_DETECT_0_FIELD =
{
    "PMD_SIGNAL_DETECT_0",
#if RU_INCLUDE_DESC
    "",
    "Signal detect status from the analog.  This signal is not related to"
    "any interface clock or data validity.",
#endif
    SERDES_STATUS_STATUS_PMD_SIGNAL_DETECT_0_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMD_SIGNAL_DETECT_0_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMD_SIGNAL_DETECT_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMD_ENERGY_DETECT_0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMD_ENERGY_DETECT_0_FIELD =
{
    "PMD_ENERGY_DETECT_0",
#if RU_INCLUDE_DESC
    "",
    "EEE energy detect.",
#endif
    SERDES_STATUS_STATUS_PMD_ENERGY_DETECT_0_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMD_ENERGY_DETECT_0_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMD_ENERGY_DETECT_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMD_RX_LOCK_0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMD_RX_LOCK_0_FIELD =
{
    "PMD_RX_LOCK_0",
#if RU_INCLUDE_DESC
    "",
    "Receive PMD lock.  WHen this signal is low, the receiver is"
    "acquiring lock.  During this period, the phase of the receive clock"
    "and alignment of data are not reliable.",
#endif
    SERDES_STATUS_STATUS_PMD_RX_LOCK_0_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMD_RX_LOCK_0_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMD_RX_LOCK_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMD_TX_CLK_VLD
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMD_TX_CLK_VLD_FIELD =
{
    "PMD_TX_CLK_VLD",
#if RU_INCLUDE_DESC
    "",
    "Transmit clock valid.",
#endif
    SERDES_STATUS_STATUS_PMD_TX_CLK_VLD_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMD_TX_CLK_VLD_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMD_TX_CLK_VLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMD_RX_CLK_VLD_0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMD_RX_CLK_VLD_0_FIELD =
{
    "PMD_RX_CLK_VLD_0",
#if RU_INCLUDE_DESC
    "",
    "Receive clock valid.",
#endif
    SERDES_STATUS_STATUS_PMD_RX_CLK_VLD_0_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMD_RX_CLK_VLD_0_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMD_RX_CLK_VLD_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMD_RX_LOCK_0_INVERT
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMD_RX_LOCK_0_INVERT_FIELD =
{
    "PMD_RX_LOCK_0_INVERT",
#if RU_INCLUDE_DESC
    "",
    "Assertion of this signal indicates that the pll has not achieved"
    "lock.",
#endif
    SERDES_STATUS_STATUS_PMD_RX_LOCK_0_INVERT_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMD_RX_LOCK_0_INVERT_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMD_RX_LOCK_0_INVERT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_STATUS_PMD_PLL0_LOCK
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_STATUS_PMD_PLL0_LOCK_FIELD =
{
    "PMD_PLL0_LOCK",
#if RU_INCLUDE_DESC
    "",
    "Assertion of this signal indicates that the pll has achieved lock.",
#endif
    SERDES_STATUS_STATUS_PMD_PLL0_LOCK_FIELD_MASK,
    0,
    SERDES_STATUS_STATUS_PMD_PLL0_LOCK_FIELD_WIDTH,
    SERDES_STATUS_STATUS_PMD_PLL0_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: SERDES_STATUS_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SERDES_STATUS_STATUS_FIELDS[] =
{
    &SERDES_STATUS_STATUS_RESERVED0_FIELD,
    &SERDES_STATUS_STATUS_PMD_PLL1_LOCK_FIELD,
    &SERDES_STATUS_STATUS_O_LASER_BURST_EN_FIELD,
    &SERDES_STATUS_STATUS_PMI_LP_ERROR_FIELD,
    &SERDES_STATUS_STATUS_PMI_LP_ACKNOWLEDGE_FIELD,
    &SERDES_STATUS_STATUS_PMD_SIGNAL_DETECT_0_FIELD,
    &SERDES_STATUS_STATUS_PMD_ENERGY_DETECT_0_FIELD,
    &SERDES_STATUS_STATUS_PMD_RX_LOCK_0_FIELD,
    &SERDES_STATUS_STATUS_PMD_TX_CLK_VLD_FIELD,
    &SERDES_STATUS_STATUS_PMD_RX_CLK_VLD_0_FIELD,
    &SERDES_STATUS_STATUS_PMD_RX_LOCK_0_INVERT_FIELD,
    &SERDES_STATUS_STATUS_PMD_PLL0_LOCK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SERDES_STATUS_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "WAN_TOP_SERDES_STATUS Register",
    "Register used for various WAN status bits.",
#endif
    SERDES_STATUS_STATUS_REG_OFFSET,
    0,
    0,
    25,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    SERDES_STATUS_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: SERDES_STATUS
 ******************************************************************************/
static const ru_reg_rec *SERDES_STATUS_REGS[] =
{
    &SERDES_STATUS_STATUS_REG,
};

unsigned long SERDES_STATUS_ADDRS[] =
{
    0x80144094,
};

const ru_block_rec SERDES_STATUS_BLOCK = 
{
    "SERDES_STATUS",
    SERDES_STATUS_ADDRS,
    1,
    1,
    SERDES_STATUS_REGS
};

/* End of file SERDES_STATUS.c */
