// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/******************************************************************************
 ******************************************************************************
 *  Revision      :   *
 *                                                                            *
 *  Description   :  Enum types used by Serdes API functions                  *
 *                                                                            *
 ******************************************************************************
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MERLIN8_PHY1_API_ENUM_H
#define MERLIN8_PHY1_API_ENUM_H

#include "merlin8_phy1_ipconfig.h"
#ifndef EXCLUDE_STD_HEADERS
#include <stdint.h>
#endif

/*! @file
 *  @brief Enum types used by Serdes API functions
 */

/*! @addtogroup APITag
 * @{
 */

/*! @defgroup SerdesAPIEnumTag Serdes API Enums
 * Enums used throughout Serdes APIs.
 */

/*! @addtogroup SerdesAPIEnumTag
 * @{
 */


/*--------------------------*/
/** Serdes RX PAM Mode Enum */
/*--------------------------*/
enum merlin8_phy1_rx_pam_mode_enum {
    NRZ,
    PAM4_NR,
    PAM4_ER
};

enum merlin8_phy1_cdr_enum {
    OS,
    BR
};

/*! Reference clock frequency enums
 *
 */
enum merlin8_phy1_pll_refclk_enum {
    MERLIN8_PHY1_PLL_REFCLK_UNKNOWN = 0, /*!< Refclk value to be determined by API. */
    MERLIN8_PHY1_PLL_REFCLK_400MHZ         = (int)0x00100190, /*!< 400 MHz         */
    MERLIN8_PHY1_PLL_REFCLK_50MHZ          = (int)0x00100032, /*!< 50 MHz          */
    MERLIN8_PHY1_PLL_REFCLK_125MHZ         = (int)0x0010007D, /*!< 125 MHz         */
    MERLIN8_PHY1_PLL_REFCLK_156P25MHZ      = (int)0x00400271, /*!< 156.25 MHz      */
    MERLIN8_PHY1_PLL_REFCLK_161P1328125MHZ = (int)0x08005091  /*!< 161.1328125 MHz */
    };

/** @brief Enumeration DIV values used by PLL configuration APIs.
 */
enum merlin8_phy1_pll_div_enum {
    MERLIN8_PHY1_PLL_DIV_UNKNOWN = 0, /*!< Divide value to be determined by API. */
    MERLIN8_PHY1_PLL_DIV_25P78125   = (int)0xC8000019, /*!< Divide by 25.78125   */
    MERLIN8_PHY1_PLL_DIV_31P25      = (int)0x4000001F, /*!< Divide by 31.25      */
    MERLIN8_PHY1_PLL_DIV_51P5625    = (int)0x90000033, /*!< Divide by 51.5625    */
    MERLIN8_PHY1_PLL_DIV_62P5       = (int)0x8000003E, /*!< Divide by 62.5       */
    MERLIN8_PHY1_PLL_DIV_103P125    = (int)0x20000067, /*!< Divide by 103.125    */
    MERLIN8_PHY1_PLL_DIV_125        = (int)0x0000007D, /*!< Divide by 125        */
    MERLIN8_PHY1_PLL_DIV_52P751515  = (int)0xC0634034, /*!< Divide by 52.751515  */
    MERLIN8_PHY1_PLL_DIV_54P4       = (int)0x66668036, /*!< Divide by 54.4       */
    MERLIN8_PHY1_PLL_DIV_58P181818  = (int)0x2E8BC03A, /*!< Divide by 58.181818  */
    MERLIN8_PHY1_PLL_DIV_60         = (int)0x0000003C, /*!< Divide by 60         */
    MERLIN8_PHY1_PLL_DIV_62P060606  = (int)0x0F84003E, /*!< Divide by 62.060606  */
    MERLIN8_PHY1_PLL_DIV_64         = (int)0x00000040, /*!< Divide by 64         */
    MERLIN8_PHY1_PLL_DIV_66         = (int)0x00000042, /*!< Divide by 66         */
    MERLIN8_PHY1_PLL_DIV_66P460703  = (int)0x75F0C042, /*!< Divide by 66.460703  */
    MERLIN8_PHY1_PLL_DIV_66P743079  = (int)0xBE3A8042, /*!< Divide by 66.743079  */
    MERLIN8_PHY1_PLL_DIV_67P878788  = (int)0xE0F84043, /*!< Divide by 67.878788  */
    MERLIN8_PHY1_PLL_DIV_68         = (int)0x00000044, /*!< Divide by 68         */
    MERLIN8_PHY1_PLL_DIV_68P537598  = (int)0x89A00044, /*!< Divide by 68.537598  */
    MERLIN8_PHY1_PLL_DIV_68P570764  = (int)0x921D8044, /*!< Divide by 68.570764  */
    MERLIN8_PHY1_PLL_DIV_68P828796  = (int)0xD42C0044, /*!< Divide by 68.828796  */
    MERLIN8_PHY1_PLL_DIV_68P856242  = (int)0xDB32C044, /*!< Divide by 68.856242  */
    MERLIN8_PHY1_PLL_DIV_69P152458  = (int)0x27078045, /*!< Divide by 69.152458  */
    MERLIN8_PHY1_PLL_DIV_69P389964  = (int)0x63D4C045, /*!< Divide by 69.389964  */
    MERLIN8_PHY1_PLL_DIV_69P818182  = (int)0xD1748045, /*!< Divide by 69.818182  */
    MERLIN8_PHY1_PLL_DIV_70         = (int)0x00000046, /*!< Divide by 70         */
    MERLIN8_PHY1_PLL_DIV_70P713596  = (int)0xB6AE4046, /*!< Divide by 70.713596  */
    MERLIN8_PHY1_PLL_DIV_71P008     = (int)0x020C4047, /*!< Divide by 71.008     */
    MERLIN8_PHY1_PLL_DIV_71P112952  = (int)0x1CEA8047, /*!< Divide by 71.P112952 */
    MERLIN8_PHY1_PLL_DIV_71P31347   = (int)0x503F8047, /*!< Divide by 71.31347   */
    MERLIN8_PHY1_PLL_DIV_71P5584    = (int)0x8EF34047, /*!< Divide by 71.5584    */
    MERLIN8_PHY1_PLL_DIV_72         = (int)0x00000048, /*!< Divide by 72         */
    MERLIN8_PHY1_PLL_DIV_73P335232  = (int)0x55D1C049, /*!< Divide by 73.335232  */
    MERLIN8_PHY1_PLL_DIV_73P6       = (int)0x9999C049, /*!< Divide by 73.6       */
    MERLIN8_PHY1_PLL_DIV_75         = (int)0x0000004B, /*!< Divide by 75         */
    MERLIN8_PHY1_PLL_DIV_80         = (int)0x00000050, /*!< Divide by 80         */
    MERLIN8_PHY1_PLL_DIV_82P5       = (int)0x80000052, /*!< Divide by 82.5       */
    MERLIN8_PHY1_PLL_DIV_85P671997  = (int)0xAC080055, /*!< Divide by 85.671997  */
    MERLIN8_PHY1_PLL_DIV_86P036     = (int)0x09374056, /*!< Divide by 86.036     */
    MERLIN8_PHY1_PLL_DIV_87P5       = (int)0x80000057, /*!< Divide by 87.5       */
    MERLIN8_PHY1_PLL_DIV_88P392     = (int)0x645A4058, /*!< Divide by 88.392     */
    MERLIN8_PHY1_PLL_DIV_88P76      = (int)0xC28F8058, /*!< Divide by 88.76      */
    MERLIN8_PHY1_PLL_DIV_89P141838  = (int)0x244F8059, /*!< Divide by 89.141838  */
    MERLIN8_PHY1_PLL_DIV_89P447998  = (int)0x72B00059, /*!< Divide by 89.447998  */
    MERLIN8_PHY1_PLL_DIV_90         = (int)0x0000005A, /*!< Divide by 90         */
    MERLIN8_PHY1_PLL_DIV_91P669037  = (int)0xAB46005B, /*!< Divide by 91.669037  */
    MERLIN8_PHY1_PLL_DIV_92         = (int)0x0000005C, /*!< Divide by 92         */
    MERLIN8_PHY1_PLL_DIV_100        = (int)0x00000064, /*!< Divide by 100        */
    MERLIN8_PHY1_PLL_DIV_170        = (int)0x000000AA, /*!< Divide by 170        */
    MERLIN8_PHY1_PLL_DIV_187P5      = (int)0x800000BB, /*!< Divide by 187.5      */
    MERLIN8_PHY1_PLL_DIV_200        = (int)0x000000C8, /*!< Divide by 200        */
    MERLIN8_PHY1_PLL_DIV_206P25     = (int)0x400000CE  /*!< Divide by 206.25     */
};

/** TX AFE Settings Enum */
enum merlin8_phy1_tx_afe_settings_enum {
    MERLIN8_PHY1_TX_AFE_PRE,
    MERLIN8_PHY1_TX_AFE_MAIN,
    MERLIN8_PHY1_TX_AFE_POST1,
    MERLIN8_PHY1_TX_AFE_POST2,
    MERLIN8_PHY1_TX_AFE_POST3,
    MERLIN8_PHY1_TX_AFE_RPARA
};


#ifndef PCIEG3_V1
/*! @def MERLIN8_PHY1_PLL_OPTION_REFCLK_MASK
 * Mask used for REFCLK options within enum merlin8_phy1_pll_option_enum.
 * All REFCLK options should fall within this mask's range.
 */
#define MERLIN8_PHY1_PLL_OPTION_REFCLK_MASK (0xF)

/** PLL Configuration Options Enum used for configure_pll APIs
 *  If using REFCLK options, only one REFCLK option may be used at a time.
 */
enum merlin8_phy1_pll_option_enum {
    MERLIN8_PHY1_PLL_OPTION_NONE              =  0,
    MERLIN8_PHY1_PLL_OPTION_REFCLK_DOUBLER_EN =  1,
    MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV2_EN    =  2,
    MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV4_EN    =  3,
    MERLIN8_PHY1_PLL_OPTION_POWERDOWN         =  1<<4,
    MERLIN8_PHY1_PLL_OPTION_DISABLE_VERIFY    =  1<<5
};
#endif /* MERLIN7_PCIEG3 */



/** PRBS Error Analyzer Aggregate Mode */
enum srds_prbs_error_analyzer_aggregate_mode_enum {
    SRDS_CUSTOM
};






/*! @} SerdesAPIEnumTag */
/*! @} APITag */
#endif /*#define MERLIN8_PHY1_API_ENUM_H*/
#ifdef __cplusplus
}
#endif
