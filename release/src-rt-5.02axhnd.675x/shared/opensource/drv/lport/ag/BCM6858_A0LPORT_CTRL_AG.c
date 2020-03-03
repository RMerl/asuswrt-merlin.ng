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
 * Field: LPORT_CTRL_LPORT_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_CTRL_LPORT_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_TIMEOUT_RST_DISABLE
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_TIMEOUT_RST_DISABLE_FIELD =
{
    "TIMEOUT_RST_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, LPORT internal register bus bridges are not automatically reseted/reinitalized when the UBUS slave port times out.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_TIMEOUT_RST_DISABLE_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_TIMEOUT_RST_DISABLE_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_TIMEOUT_RST_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_P4_MODE
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_P4_MODE_FIELD =
{
    "P4_MODE",
#if RU_INCLUDE_DESC
    "",
    "P4 Mode:\n"
    "0 : P4 operates in GMII mode.\n"
    "1 : P4 operates in XGMII mode.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_P4_MODE_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_P4_MODE_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_P4_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_P0_MODE
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_P0_MODE_FIELD =
{
    "P0_MODE",
#if RU_INCLUDE_DESC
    "",
    "P0 Mode:\n"
    "0 : P0 operates in GMII mode.\n"
    "1 : P0 operates in XGMII mode.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_P0_MODE_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_P0_MODE_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_P0_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_7
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_7_FIELD =
{
    "GPORT_SEL_7",
#if RU_INCLUDE_DESC
    "",
    "Port 7 PHY Select (Gigabit Mode):\n"
    "0 : P7 is unconnected.\n"
    "1 : SERDES0 is connected to P7.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_7_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_7_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_6
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_6_FIELD =
{
    "GPORT_SEL_6",
#if RU_INCLUDE_DESC
    "",
    "Port 6 PHY Select (Gigabit Mode):\n"
    "0 : RGMII2 is connected to P6.\n"
    "1 : SERDES3 is connected to P6.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_6_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_6_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_5
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_5_FIELD =
{
    "GPORT_SEL_5",
#if RU_INCLUDE_DESC
    "",
    "Port 5 PHY Select (Gigabit Mode):\n"
    "0 : RGMII1 is connected to P5.\n"
    "1 : SERDES2 is connected to P5.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_5_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_5_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_4
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_4_FIELD =
{
    "GPORT_SEL_4",
#if RU_INCLUDE_DESC
    "",
    "Port 4 PHY Select (Gigabit Mode):\n"
    "0 : RGMII0 is connected to P4.\n"
    "1 : SERDES1 is connected to P4.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_4_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_4_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_3
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_3_FIELD =
{
    "GPORT_SEL_3",
#if RU_INCLUDE_DESC
    "",
    "Port 3 PHY Select (Gigabit Mode):\n"
    "0 : EGPHY4 is connected to P3.\n"
    "1 : SERDES3 is connected to P3.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_3_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_3_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_2
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_2_FIELD =
{
    "GPORT_SEL_2",
#if RU_INCLUDE_DESC
    "",
    "Port 2 PHY Select (Gigabit Mode):\n"
    "0 : EGPHY3 is connected to P2.\n"
    "1 : SERDES2 is connected to P2.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_2_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_2_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_1
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_1_FIELD =
{
    "GPORT_SEL_1",
#if RU_INCLUDE_DESC
    "",
    "Port 1 PHY Select (Gigabit Mode):\n"
    "0 : EGPHY2 is connected to P1.\n"
    "1 : SERDES1 is connected to P1.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_1_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_1_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_0
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_0_FIELD =
{
    "GPORT_SEL_0",
#if RU_INCLUDE_DESC
    "",
    "Port 0 PHY Select (Gigabit Mode):\n"
    "0 : EGPHY1 is connected to P0.\n"
    "1 : SERDES0 is connected to P0.\n",
#endif
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_0_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_0_FIELD_WIDTH,
    LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_REVISION_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_REVISION_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_CTRL_LPORT_REVISION_RESERVED0_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_REVISION_RESERVED0_FIELD_WIDTH,
    LPORT_CTRL_LPORT_REVISION_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LPORT_REVISION_LPORT_REV
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LPORT_REVISION_LPORT_REV_FIELD =
{
    "LPORT_REV",
#if RU_INCLUDE_DESC
    "",
    "LPORT revision.",
#endif
    LPORT_CTRL_LPORT_REVISION_LPORT_REV_FIELD_MASK,
    0,
    LPORT_CTRL_LPORT_REVISION_LPORT_REV_FIELD_WIDTH,
    LPORT_CTRL_LPORT_REVISION_LPORT_REV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_REVISION_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_REVISION_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_CTRL_QEGPHY_REVISION_RESERVED0_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_REVISION_RESERVED0_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_REVISION_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_REVISION_QUAD_PHY_REV
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_REVISION_QUAD_PHY_REV_FIELD =
{
    "QUAD_PHY_REV",
#if RU_INCLUDE_DESC
    "",
    "Quad EGPHY Revision Control Register.",
#endif
    LPORT_CTRL_QEGPHY_REVISION_QUAD_PHY_REV_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_REVISION_QUAD_PHY_REV_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_REVISION_QUAD_PHY_REV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_TEST_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_TEST_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_CTRL_QEGPHY_TEST_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_REFCLK_SEL
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_REFCLK_SEL_FIELD =
{
    "PLL_REFCLK_SEL",
#if RU_INCLUDE_DESC
    "",
    "These bits are used to select the reference clock source to EGPHY:\n"
    "00 : i_pll_refclk[0].\n"
    "01 : i_pll_refclk[1].\n"
    "10 : i_pll_refclk[2].\n"
    "11 : TVCO.\n"
    "Note: Do note change these bits from their default value before consulting with Broadcom.",
#endif
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_REFCLK_SEL_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_REFCLK_SEL_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_REFCLK_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_SEL_DIV5
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_SEL_DIV5_FIELD =
{
    "PLL_SEL_DIV5",
#if RU_INCLUDE_DESC
    "",
    "These bits are used to select the frequency of the reference clock source to EGPHY:\n"
    "00 : 25MHz.\n"
    "01 : 54MHz.\n"
    "10 : 50MHz.\n"
    "11 : reserved.\n"
    "Note: Do note change these bits from their default value before consulting with Broadcom.",
#endif
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_SEL_DIV5_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_SEL_DIV5_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_SEL_DIV5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_CLK125_250_SEL
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_CLK125_250_SEL_FIELD =
{
    "PLL_CLK125_250_SEL",
#if RU_INCLUDE_DESC
    "",
    "This bit is used to select the EGPHY PLL output clock frequency:\n"
    "0 : 125MHz.\n"
    "1 : 250MHz.\n"
    "Note: Do note change this bit from its default value before consulting with Broadcom.",
#endif
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_CLK125_250_SEL_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_CLK125_250_SEL_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_CLK125_250_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_TEST_CNTRL_PHY_TEST_EN
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_TEST_CNTRL_PHY_TEST_EN_FIELD =
{
    "PHY_TEST_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables EGPHY test mode within chip testmux:\n"
    "0 : EGPHY is not in test mode.\n"
    "1 : EGPHY is in test mode.\n"
    "Note:This bit is combined with PHY_SEL.",
#endif
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PHY_TEST_EN_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PHY_TEST_EN_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_PHY_TEST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_PHY_PHYAD
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_PHY_PHYAD_FIELD =
{
    "PHY_PHYAD",
#if RU_INCLUDE_DESC
    "",
    "Quad EGPHY base PHY address. "
    "EGPHYs within quad EGPHY are addressed as base address + offset where offset=0,1,2,3.",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_PHY_PHYAD_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_PHY_PHYAD_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_PHY_PHYAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_RESERVED1_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_RESERVED1_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_PHY_RESET
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_PHY_RESET_FIELD =
{
    "PHY_RESET",
#if RU_INCLUDE_DESC
    "",
    "Quad EGPHY system reset. Must be held high for at leaset 60 us while system clock is running. "
    "After reset de-assertion no EGPHY activity should occur for 20 us.",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_PHY_RESET_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_PHY_RESET_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_PHY_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_CK25_EN
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_CK25_EN_FIELD =
{
    "CK25_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable 25 MHz clock to quad EGPHY. "
    "This bit should be cleared in quad EGPHY power down mode only when MDIO access is not required.",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_CK25_EN_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_CK25_EN_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_CK25_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_IDDQ_GLOBAL_PWR
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_IDDQ_GLOBAL_PWR_FIELD =
{
    "IDDQ_GLOBAL_PWR",
#if RU_INCLUDE_DESC
    "",
    "Enables isolation cells for quad EGPHY power gating.",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_IDDQ_GLOBAL_PWR_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_IDDQ_GLOBAL_PWR_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_IDDQ_GLOBAL_PWR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_FORCE_DLL_EN
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_FORCE_DLL_EN_FIELD =
{
    "FORCE_DLL_EN",
#if RU_INCLUDE_DESC
    "",
    "Force DLL on.",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_FORCE_DLL_EN_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_FORCE_DLL_EN_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_FORCE_DLL_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_EXT_PWR_DOWN
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_EXT_PWR_DOWN_FIELD =
{
    "EXT_PWR_DOWN",
#if RU_INCLUDE_DESC
    "",
    "When any of bits is set, corresponding EGPHY AFE is powered down. "
    "When 4'b1111 and force_dll_en=0, quad EGPHY DLL is also powered down. "
    "Requires SW reset to bring EGPHY back from power down.",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_EXT_PWR_DOWN_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_EXT_PWR_DOWN_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_EXT_PWR_DOWN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_CNTRL_IDDQ_BIAS
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_CNTRL_IDDQ_BIAS_FIELD =
{
    "IDDQ_BIAS",
#if RU_INCLUDE_DESC
    "",
    "Power down BIAS. When 1'b1. the internal bias is put into iddq mode. "
    "The energy_det output is not valid when this input is set. "
    "Requires HW reset(see bit 8 of this register) to bring EGPHY back from power down.",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_IDDQ_BIAS_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_CNTRL_IDDQ_BIAS_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_CNTRL_IDDQ_BIAS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_CTRL_QEGPHY_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_STATUS_GPHY_TEST_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_STATUS_GPHY_TEST_STATUS_FIELD =
{
    "GPHY_TEST_STATUS",
#if RU_INCLUDE_DESC
    "",
    "EGPHY test status. When 1'b1 indicates that EGPHY is in test mode.",
#endif
    LPORT_CTRL_QEGPHY_STATUS_GPHY_TEST_STATUS_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_STATUS_GPHY_TEST_STATUS_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_STATUS_GPHY_TEST_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_STATUS_RECOVERED_CLK_LOCK
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_STATUS_RECOVERED_CLK_LOCK_FIELD =
{
    "RECOVERED_CLK_LOCK",
#if RU_INCLUDE_DESC
    "",
    "When 1'b1 indicates that recovered clock is locked.",
#endif
    LPORT_CTRL_QEGPHY_STATUS_RECOVERED_CLK_LOCK_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_STATUS_RECOVERED_CLK_LOCK_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_STATUS_RECOVERED_CLK_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_STATUS_PLL_LOCK
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_STATUS_PLL_LOCK_FIELD =
{
    "PLL_LOCK",
#if RU_INCLUDE_DESC
    "",
    "When 1'b1 indicates that Quad EGPHY DLL is locked.",
#endif
    LPORT_CTRL_QEGPHY_STATUS_PLL_LOCK_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_STATUS_PLL_LOCK_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_STATUS_PLL_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_APD
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_APD_FIELD =
{
    "ENERGY_DET_APD",
#if RU_INCLUDE_DESC
    "",
    "Filtered Energy Detect in Auto-Power Down mode.",
#endif
    LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_APD_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_APD_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_APD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_MASKED
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_MASKED_FIELD =
{
    "ENERGY_DET_MASKED",
#if RU_INCLUDE_DESC
    "",
    "Filtered Energy Detect.",
#endif
    LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_MASKED_FIELD_MASK,
    0,
    LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_MASKED_FIELD_WIDTH,
    LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_MASKED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_ON_TIME
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD =
{
    "LED_ON_TIME",
#if RU_INCLUDE_DESC
    "",
    "Led ON time. Expressed in 50us units.",
#endif
    LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_MASK,
    0,
    LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_WIDTH,
    LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_OFF_TIME
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD =
{
    "LED_OFF_TIME",
#if RU_INCLUDE_DESC
    "",
    "Led OFF time. Expressed in 50us  units.",
#endif
    LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_MASK,
    0,
    LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_WIDTH,
    LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LED_SERIAL_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LED_SERIAL_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_CTRL_LED_SERIAL_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_CTRL_LED_SERIAL_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_CTRL_LED_SERIAL_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LED_SERIAL_CNTRL_SMODE
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LED_SERIAL_CNTRL_SMODE_FIELD =
{
    "SMODE",
#if RU_INCLUDE_DESC
    "",
    "Indicates number of LED signals per port that are shifted out:\n"
    "11 : 4 LEDs per port mode (SPDLNK_LED[2:0] and ACT_LED).\n"
    "10 : 3 LEDs per port mode (SPDLNK_LED[2:0]).\n"
    "01 : 3 LEDs per port mode (SPDLNK_LED[1:0] and ACT_LED).\n"
    "00 : 2 LEDs per port mode (SPDLNK_LED[1:0])",
#endif
    LPORT_CTRL_LED_SERIAL_CNTRL_SMODE_FIELD_MASK,
    0,
    LPORT_CTRL_LED_SERIAL_CNTRL_SMODE_FIELD_WIDTH,
    LPORT_CTRL_LED_SERIAL_CNTRL_SMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD =
{
    "SLED_CLK_FREQUENCY",
#if RU_INCLUDE_DESC
    "",
    "Indicates SLED_CLK frequency.\n"
    "0 : SLED_CLK is 6.25Mhz.\n"
    "1 : SLED_CLK is 3.125Mhz.",
#endif
    LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD_MASK,
    0,
    LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD_WIDTH,
    LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_POL
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD =
{
    "SLED_CLK_POL",
#if RU_INCLUDE_DESC
    "",
    "When this bit is 1'b1 serial LED clock(SCLK) polarity is inveretd. "
    "Used with shift registers that trigger on the falling edge.",
#endif
    LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD_MASK,
    0,
    LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD_WIDTH,
    LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LED_SERIAL_CNTRL_REFRESH_PERIOD
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD =
{
    "REFRESH_PERIOD",
#if RU_INCLUDE_DESC
    "",
    "Serial LED refresh period. "
    "Expressed in 5ms units. Value of 0 means 32x5ms period.",
#endif
    LPORT_CTRL_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD_MASK,
    0,
    LPORT_CTRL_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD_WIDTH,
    LPORT_CTRL_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_CTRL_LED_SERIAL_CNTRL_PORT_EN
 ******************************************************************************/
const ru_field_rec LPORT_CTRL_LED_SERIAL_CNTRL_PORT_EN_FIELD =
{
    "PORT_EN",
#if RU_INCLUDE_DESC
    "",
    "When the corresponding bit is set, port LEDs are shifted out. "
    "When all bits are cleared, serial LED interface is disabled.",
#endif
    LPORT_CTRL_LED_SERIAL_CNTRL_PORT_EN_FIELD_MASK,
    0,
    LPORT_CTRL_LED_SERIAL_CNTRL_PORT_EN_FIELD_WIDTH,
    LPORT_CTRL_LED_SERIAL_CNTRL_PORT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: LPORT_CTRL_LPORT_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_CTRL_LPORT_CNTRL_FIELDS[] =
{
    &LPORT_CTRL_LPORT_CNTRL_RESERVED0_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_TIMEOUT_RST_DISABLE_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_P4_MODE_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_P0_MODE_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_7_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_6_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_5_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_4_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_3_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_2_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_1_FIELD,
    &LPORT_CTRL_LPORT_CNTRL_GPORT_SEL_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_CTRL_LPORT_CNTRL_REG = 
{
    "LPORT_CNTRL",
#if RU_INCLUDE_DESC
    "LPORT Control Register",
    "",
#endif
    LPORT_CTRL_LPORT_CNTRL_REG_OFFSET,
    0,
    0,
    205,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    LPORT_CTRL_LPORT_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_CTRL_LPORT_REVISION
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_CTRL_LPORT_REVISION_FIELDS[] =
{
    &LPORT_CTRL_LPORT_REVISION_RESERVED0_FIELD,
    &LPORT_CTRL_LPORT_REVISION_LPORT_REV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_CTRL_LPORT_REVISION_REG = 
{
    "LPORT_REVISION",
#if RU_INCLUDE_DESC
    "LPORT Revision Control Register",
    "",
#endif
    LPORT_CTRL_LPORT_REVISION_REG_OFFSET,
    0,
    0,
    206,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_CTRL_LPORT_REVISION_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_CTRL_QEGPHY_REVISION
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_CTRL_QEGPHY_REVISION_FIELDS[] =
{
    &LPORT_CTRL_QEGPHY_REVISION_RESERVED0_FIELD,
    &LPORT_CTRL_QEGPHY_REVISION_QUAD_PHY_REV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_CTRL_QEGPHY_REVISION_REG = 
{
    "QEGPHY_REVISION",
#if RU_INCLUDE_DESC
    "QEGPHY Revision Control Register",
    "",
#endif
    LPORT_CTRL_QEGPHY_REVISION_REG_OFFSET,
    0,
    0,
    207,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_CTRL_QEGPHY_REVISION_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_CTRL_QEGPHY_TEST_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_CTRL_QEGPHY_TEST_CNTRL_FIELDS[] =
{
    &LPORT_CTRL_QEGPHY_TEST_CNTRL_RESERVED0_FIELD,
    &LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_REFCLK_SEL_FIELD,
    &LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_SEL_DIV5_FIELD,
    &LPORT_CTRL_QEGPHY_TEST_CNTRL_PLL_CLK125_250_SEL_FIELD,
    &LPORT_CTRL_QEGPHY_TEST_CNTRL_PHY_TEST_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_CTRL_QEGPHY_TEST_CNTRL_REG = 
{
    "QEGPHY_TEST_CNTRL",
#if RU_INCLUDE_DESC
    "Quad GPHY Test Control Register",
    "",
#endif
    LPORT_CTRL_QEGPHY_TEST_CNTRL_REG_OFFSET,
    0,
    0,
    208,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_CTRL_QEGPHY_TEST_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_CTRL_QEGPHY_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_CTRL_QEGPHY_CNTRL_FIELDS[] =
{
    &LPORT_CTRL_QEGPHY_CNTRL_RESERVED0_FIELD,
    &LPORT_CTRL_QEGPHY_CNTRL_PHY_PHYAD_FIELD,
    &LPORT_CTRL_QEGPHY_CNTRL_RESERVED1_FIELD,
    &LPORT_CTRL_QEGPHY_CNTRL_PHY_RESET_FIELD,
    &LPORT_CTRL_QEGPHY_CNTRL_CK25_EN_FIELD,
    &LPORT_CTRL_QEGPHY_CNTRL_IDDQ_GLOBAL_PWR_FIELD,
    &LPORT_CTRL_QEGPHY_CNTRL_FORCE_DLL_EN_FIELD,
    &LPORT_CTRL_QEGPHY_CNTRL_EXT_PWR_DOWN_FIELD,
    &LPORT_CTRL_QEGPHY_CNTRL_IDDQ_BIAS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_CTRL_QEGPHY_CNTRL_REG = 
{
    "QEGPHY_CNTRL",
#if RU_INCLUDE_DESC
    "Quad GPHY Control Register",
    "",
#endif
    LPORT_CTRL_QEGPHY_CNTRL_REG_OFFSET,
    0,
    0,
    209,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_CTRL_QEGPHY_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_CTRL_QEGPHY_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_CTRL_QEGPHY_STATUS_FIELDS[] =
{
    &LPORT_CTRL_QEGPHY_STATUS_RESERVED0_FIELD,
    &LPORT_CTRL_QEGPHY_STATUS_GPHY_TEST_STATUS_FIELD,
    &LPORT_CTRL_QEGPHY_STATUS_RECOVERED_CLK_LOCK_FIELD,
    &LPORT_CTRL_QEGPHY_STATUS_PLL_LOCK_FIELD,
    &LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_APD_FIELD,
    &LPORT_CTRL_QEGPHY_STATUS_ENERGY_DET_MASKED_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_CTRL_QEGPHY_STATUS_REG = 
{
    "QEGPHY_STATUS",
#if RU_INCLUDE_DESC
    "Quad GPHY Status Register",
    "",
#endif
    LPORT_CTRL_QEGPHY_STATUS_REG_OFFSET,
    0,
    0,
    210,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LPORT_CTRL_QEGPHY_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_CTRL_LED_BLINK_RATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_CTRL_LED_BLINK_RATE_CNTRL_FIELDS[] =
{
    &LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD,
    &LPORT_CTRL_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_CTRL_LED_BLINK_RATE_CNTRL_REG = 
{
    "LED_BLINK_RATE_CNTRL",
#if RU_INCLUDE_DESC
    "Aggregate LED Blink Rate Control Register",
    "",
#endif
    LPORT_CTRL_LED_BLINK_RATE_CNTRL_REG_OFFSET,
    0,
    0,
    211,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_CTRL_LED_BLINK_RATE_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_CTRL_LED_SERIAL_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_CTRL_LED_SERIAL_CNTRL_FIELDS[] =
{
    &LPORT_CTRL_LED_SERIAL_CNTRL_RESERVED0_FIELD,
    &LPORT_CTRL_LED_SERIAL_CNTRL_SMODE_FIELD,
    &LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD,
    &LPORT_CTRL_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD,
    &LPORT_CTRL_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD,
    &LPORT_CTRL_LED_SERIAL_CNTRL_PORT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_CTRL_LED_SERIAL_CNTRL_REG = 
{
    "LED_SERIAL_CNTRL",
#if RU_INCLUDE_DESC
    "LED Serial Control Register",
    "",
#endif
    LPORT_CTRL_LED_SERIAL_CNTRL_REG_OFFSET,
    0,
    0,
    212,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LPORT_CTRL_LED_SERIAL_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: LPORT_CTRL
 ******************************************************************************/
static const ru_reg_rec *LPORT_CTRL_REGS[] =
{
    &LPORT_CTRL_LPORT_CNTRL_REG,
    &LPORT_CTRL_LPORT_REVISION_REG,
    &LPORT_CTRL_QEGPHY_REVISION_REG,
    &LPORT_CTRL_QEGPHY_TEST_CNTRL_REG,
    &LPORT_CTRL_QEGPHY_CNTRL_REG,
    &LPORT_CTRL_QEGPHY_STATUS_REG,
    &LPORT_CTRL_LED_BLINK_RATE_CNTRL_REG,
    &LPORT_CTRL_LED_SERIAL_CNTRL_REG,
};

unsigned long LPORT_CTRL_ADDRS[] =
{
    0x8013c000,
};

const ru_block_rec LPORT_CTRL_BLOCK = 
{
    "LPORT_CTRL",
    LPORT_CTRL_ADDRS,
    1,
    8,
    LPORT_CTRL_REGS
};

/* End of file BCM6858_A0LPORT_CTRL.c */
