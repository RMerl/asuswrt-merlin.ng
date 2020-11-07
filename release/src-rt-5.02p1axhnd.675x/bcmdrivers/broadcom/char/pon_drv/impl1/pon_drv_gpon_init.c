/*
  <:copyright-BRCM:2017:proprietary:standard
  
     Copyright (c) 2017 Broadcom 
     All Rights Reserved
  
   This program is the proprietary software of Broadcom and/or its
   licensors, and may only be used, duplicated, modified or distributed pursuant
   to the terms and conditions of a separate, written license agreement executed
   between you and Broadcom (an "Authorized License").  Except as set forth in
   an Authorized License, Broadcom grants no license (express or implied), right
   to use, or waiver of any kind with respect to the Software, and Broadcom
   expressly reserves all rights in and to the Software and all intellectual
   property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
   NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
   BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
   Except as expressly set forth in the Authorized License,
  
   1. This program, including its structure, sequence and organization,
      constitutes the valuable trade secrets of Broadcom, and you shall use
      all reasonable efforts to protect the confidentiality thereof, and to
      use this information only in connection with your use of Broadcom
      integrated circuit products.
  
   2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
      AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
      WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
      RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
      ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
      FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
      COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
      TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
      PERFORMANCE OF THE SOFTWARE.
  
   3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
      ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
      INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
      WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
      IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
      OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
      SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
      SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
      LIMITED REMEDY.
  :>
*/

/****************************************************************************
 *
 * pon_drv_gpon_init.c -- Bcm Pon driver: init sequence for all G-STD PON modes:
 * GPON, XGPON, XGS, NGPON2
 *
 * Description:
 *      Built from initialization script of VLSI team
 *
 * Authors: Fuguo Xu, Akiva Sadovski, Vitaly Zborovski
 *
 * $Revision: 2.0 $
 *
 * 2017.July: updated by VZ
 ****************************************************************************/

#include <linux/delay.h>
#include <linux/kernel.h>

#include "pon_drv.h"
#include "gpon_ag.h"
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
#include "ngpon_gearbox_ag.h"
#endif
#include "ru_types.h"
#include <bcmsfp_i2c.h>
#include "pon_drv_serdes_util.h"
#include "ru.h"

#include "early_txen_ag.h"
#include "wan_drv.h"
#include "pon_drv_gpon_init.h"
#include "shared_utils.h"

#undef MISC  // to allow access by name to MISC registers in Gearbox

// Definition of various SerDes modes configurations
pon_params_t GPON_2_1_params =
{
        .mode             = GPON_2_1  ,
        .std              = STD_G     ,
        .refclk           = LCREF     ,  // LCREF = 50MHz ref.clock
        .pll_use          = ONE_PLL   ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x0       , // GPON_OS. RX-TX rate= 000: 1 to 1
};

pon_params_t XGPON_10_2_params =
{
        .mode             = XGPON_10_2,
        .std              = STD_G     ,
        .refclk           = LCREF     ,  // LCREF = 50MHz ref.clock
        .pll_use          = TWO_PLLS  ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x3       , // XGPON_OS. RX-TX rate= 011: 4 to 1
};

pon_params_t XGPON_10_2_SubRate_params =
{
        .mode             = XGPON_10_2,
        .std              = STD_G__SubRate  ,
        .refclk           = LCREF     ,  // LCREF = 50MHz ref.clock
        .pll_use          = TWO_PLLS  ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x3       , // XGPON_OS. RX-TX rate= 011: 4 to 1
};

pon_params_t NGPON_10_10_params =
{
        .mode             = NGPON_10_10,
        .std              = STD_G     ,
        .refclk           = LCREF     ,  // can be changed by 'pll155=1'
        .pll_use          = ONE_PLL   ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x0       , //NGPON full rate. RX-TX rate= 000: 1 to 1
};

#if (defined(CONFIG_BCM96856) && (defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE))) || defined(CONFIG_BCM963158)
static int chip_rev = 0;
#endif

// WAN Gearbox Initialization:
static void wan_init(pon_params_t *pon_params)
{
    uint32_t rd_data = 0x0;
    uint32_t wr_data = 0x0;
    uint8_t  mode         = pon_params->mode;
    uint8_t  pll_refclk   = pon_params->refclk;
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE) || defined(CONFIG_BCM963158)
    uint8_t  std          = pon_params->std;
#endif

    /*  # RESCAL Init */
    sgb_rescal_init();

    // # ##########################################################
    // #        Powerup SERDES, bring WAN_TOP out of reset
    // # ##########################################################

    /* GEARBOX RX/TX FIFO configuration */
    if (mode == GPON_2_1)
    {
        RU_FIELD_WRITE(0, GPON, GEARBOX_0, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, FIFO_CFG_0_TXLBE_BIT_ORDER), 1);
        RU_FIELD_WRITE(0, GPON, GEARBOX_0, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, FIFO_CFG_0_RX_16BIT_ORDER),  1);
        RU_FIELD_WRITE(0, GPON, GEARBOX_0, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, FIFO_CFG_0_TX_8BIT_ORDER),   0);
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined (CONFIG_BCM96878)
        RU_FIELD_WRITE(0, GPON, GEARBOX_0, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, FIFO_CFG_0_TX_20BIT_ORDER),  1);
#else
        RU_FIELD_WRITE(0, GPON, GEARBOX_0, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, FIFO_CFG_0_TX_16BIT_ORDER),  1);
#endif
        RU_REG_READ   (0, GPON, GEARBOX_0, rd_data);
        __logDebug("\n GPON_GEARBOX_0 = 0x%08x", rd_data);

        /* GPON_GEARBOX_2.CONFIG_BURST_DELAY_CYC = 0x3  [ln:192 @ GponEnvChip.cc ]   */
        RU_FIELD_WRITE(0, GPON, GEARBOX_2, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, CONFIG_BURST_DELAY_CYC),  1);
#if defined (CONFIG_BCM96858)
        RU_FIELD_WRITE(0, GPON, GEARBOX_2, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, TX_VLD_DELAY_CYC),  0);
#endif
        RU_REG_READ   (0, GPON, GEARBOX_2, rd_data);
        __logDebug("\n GPON_GEARBOX_2 = 0x%08x", rd_data);
    }
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
    else if ((mode == XGPON_10_2) || (mode == NGPON_10_10))
    {
        RU_FIELD_WRITE(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCHUNT    , 0);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXOUTDATAFLIP, 0);
        RU_REG_READ   (0, NGPON_GEARBOX, RX_CTL_0, rd_data);
        __logDebug("\n NGPON_GEARBOX_RX_CTL_0 = 0x%08x", rd_data);

        // NGPON_GEARBOX_TX_CTL
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXEN, 1);
        RU_REG_READ   (0, NGPON_GEARBOX, TX_CTL, rd_data);
        __logDebug("\n NGPON_GEARBOX_TX_CTL = 0x%08x", rd_data);
    }
#endif

    /* OSR Gearbox configuration */
    if (mode == GPON_2_1)
    {   // GP_OVERSAMPLE mode :  WAN_TOP_OSR_CONTROL = 0x9
        RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, CFG_GPON_RX_CLK)      , 1);
        RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXFIFO_RD_LEGACY_MODE), 0);
        RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXLBE_SER_EN)         , 1);
    }
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE) || defined(CONFIG_BCM963158)
    else if (mode == XGPON_10_2)
    {
        RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, CFG_GPON_RX_CLK)       , 2);
        if (std == STD_G__SubRate) //Should be for XGPON1 only
            RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXFIFO_RD_LEGACY_MODE), 0); // must set to 1 for XGpon & NGPON2 OSR modes.
        else
            RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXFIFO_RD_LEGACY_MODE), 1); // must set to 1 for XGpon & NGPON2 OSR modes.
        RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXLBE_SER_EN)          , 0);
    }
    else if (mode == NGPON_10_10)
    {
        // XGP_OVERSAMPLE mode: WAN_TOP_OSR_CONTROL = 0xD
        RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, CFG_GPON_RX_CLK)       , 2);
#if defined(CONFIG_BCM96856) || defined(CONFIG_BCM963158)
        if (chip_rev <= 0xB0)
            RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXFIFO_RD_LEGACY_MODE), 1); // must set to 1 for XGpon & NGPON2 OSR modes.
        else
            RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXFIFO_RD_LEGACY_MODE), 0); // must set to 1 for XGpon & NGPON2 OSR modes.
#else
        RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXFIFO_RD_LEGACY_MODE) , 1); // must set to 1 for XGpon & NGPON2 OSR modes.
#endif
        RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXLBE_SER_EN)          , 0);
    }
#endif

    RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXLBE_SER_INIT_VAL), 0);
    RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXLBE_SER_ORDER)   , 0);
    RU_REG_READ   (0, TOP_OSR, CONTROL, rd_data);
    __logDebug("\n WAN_TOP_OSR_CONTROL = 0x%08x", rd_data);

    // ##########################################################
    //            Powerup SERDES, bring out of reset
    // ##########################################################

    // #(1) Assert POR reset by forcing pmd_por_h_rstb pin to 1'b0
    // ## WAN_TOP_MISC_2 {0x80144048}
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_POR_H_RSTB),  0);

    //  Gearbox clock dividers for mac_tx/rx_clock & Oversample modes for rx/tx
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_RX_OSR_MODE),  0);
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_TX_MODE    ),  0);
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_TX_OSR_MODE),  0);
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_TX_DISABLE ),  0);
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_RX_MODE    ),  0);
    if (mode == GPON_2_1)
    {
        RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_RX_OSR_MODE),  4);  // OSX4
        RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_TX_OSR_MODE),  7);  // OSX8
    }
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
    else if (mode == XGPON_10_2)
    {
        RU_FIELD_WRITE(0, MISC, 2, CFGNGPONRXCLK, 2);
        RU_FIELD_WRITE(0, MISC, 2, CFGNGPONTXCLK, 1);
    }
    else if (mode == NGPON_10_10)
    {
        RU_FIELD_WRITE(0, MISC, 2, CFGNGPONRXCLK, 2);
        RU_FIELD_WRITE(0, MISC, 2, CFGNGPONTXCLK, 2);
    }
#endif
    RU_REG_READ(0, MISC, 2, rd_data);
    __logDebug("\n WAN_TOP_MISC_2 = 0x%08x", rd_data);

    wan_serdes_temp_init();

    // #(2) set PLL Ref Clock input
    set_pll_refclk(pll_refclk);  // LCREF = 50MHz or PADREF = 155.52MHz

    // #(3) De-assert POR reset {0x80144048}
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_POR_H_RSTB      ),  1); // release PMD reset
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_CORE_1_DP_H_RSTB),  1); // release DP reset
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_CORE_0_DP_H_RSTB),  1); // release DP reset
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_LN_H_RSTB       ),  1); // release Lane reset
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_LN_DP_H_RSTB    ),  1); // release Lane reset
    RU_REG_READ   (0, MISC, 2, rd_data);
    __logDebug("\n WAN_TOP_MISC_2 = 0x%08x", rd_data);

    /* MISC_0.REFIN_EN = 1  */
    wr_data = 0x0;
    RU_REG_WRITE  (0, MISC, 0, wr_data); // (Akiva) in BCM68360 -> ONU2G_PHYA [10:06] = 0x2 which can cause issue with LAN interface
    RU_FIELD_WRITE(0, MISC, 0, CHOP(CR_XGWAN_TOP_WAN_MISC_, REFIN_EN),  1);
    RU_REG_READ(0, MISC, 0, rd_data);
    __logDebug("\n WAN_TOP_MISC_0 = 0x%08x", rd_data);

    /* MISC_1 = 0 */
    RU_FIELD_WRITE(0, MISC, 1, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_CORE_1_MODE), 0);
    RU_FIELD_WRITE(0, MISC, 1, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_CORE_0_MODE), 0);
    RU_REG_READ(0, MISC, 1, rd_data);
    __logDebug("\n WAN_TOP_MISC_1 = 0x%08x", rd_data);

    /* MISC_3 : Set Gearbox to "NGPON" mode  (LASER_MODE / LASER_OE / INTERFACE_SELECT)  */
    RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_OE), 0); //  Disable Laser(BE) during init

    if (mode == GPON_2_1)
    {
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE           ), 2); //  BE control by Gpon MAC
#if defined (CONFIG_BCM96858)
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ), 2); //  Gpon mode. Sequoia(6858)
#elif (defined (CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined (CONFIG_BCM96878))
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ), 1); //  Gpon mode. Acacia(68360) has only Epon(0) & Gpon(1) modes.
#elif defined(CONFIG_BCM963158)
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ), 4); /*  Gpon mode. 63158 has lots of AE modes GPON = 4. */
#endif
    }
    else if ((mode == XGPON_10_2) || (mode == NGPON_10_10))
    {
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE             ), 3); //  BE control by NGpon MAC
#if defined (CONFIG_BCM96858)
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT   ), 3); //  NGpon mode
#elif defined(CONFIG_BCM96856)
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT   ), 1); //  XGPON
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT_HS), 1); //
#elif defined(CONFIG_BCM963158)
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT   ), 7); /*  NGPON mode. 63158 has lots of AE modes NGPON = 7. */
#endif
    }

    //EWAKE configuration for PMD in XGPON modes
    if ((mode == XGPON_10_2) && (optics_type == BCM_I2C_PON_OPTICS_TYPE_PMD))
    {
        early_txen_txen txen;

        // WAN_TOP_WRITE_32(0x80144030, 0x032f2f2f);
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass  = 0;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity = 1;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity = 1;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_toff_time = 0x2f;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_setup_time = 0x2f;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_hold_time = 0x2f;
        ag_drv_early_txen_txen_set(&txen);
    } // END if
} // END 'wan_init()' function.


static void serdes_init (pon_params_t *pon_params)
{
    uint32_t rd_data     = 0;
    uint16_t rd_serdes   = 0;

    uint8_t mode         = pon_params-> mode;
    uint8_t clk90_offset = pon_params-> clk90_offset;
    uint8_t p1_offset    = pon_params-> p1_offset;
    uint8_t pll_use      = pon_params-> pll_use;
    uint8_t pll_refclk   = pon_params->refclk;
    uint16_t rx_tx_rate_ratio = pon_params-> rx_tx_rate_ratio;
    uint8_t  std          = pon_params->std;
    uint32_t flash_fracn_div, flash_fracn_ndiv;

    // ##########################################################
    //  SerDes Initialization
    // ##########################################################

    /* Power-UP PLL's   { #(*) AMS_COM_PLL_INTCTRL [0xd0b9]: ams_pll_pwrdn[bit5] = 0 } */
    // if (ONE_PLL) -> TX-&-RX use PLL0 .
    // if (TWO_PLL) -> TX=PLL0 & RX=PLL1
    writePONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_0), AMS_COM_PLL_INTCTRL, 0x0000, 0x0004); // PLL0: Power-Up. ams_pll_pwrdn[bit5] = 0
    if (pll_use == TWO_PLLS)
        writePONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_1), AMS_COM_PLL_INTCTRL, 0x0000, 0x0004); // PLL1: Power-Up. ams_pll_pwrdn[bit5] = 0
    else
        writePONSerdesReg((DEVID_1 | LANE_0 | serdes_PLL_1), AMS_COM_PLL_INTCTRL, 0x0004, 0x0004); // PLL1: Power-Off. ams_pll_pwrdn[bit5] = 1

    // ##########################################################
    //  SerDes PLL Settings   (based on "ONU10G PLL programing for data rate" doc)
    // ##########################################################
    if (mode == GPON_2_1)
    {   //(*) GPON OS mode
        //////////////////////////////////////////////////////////////////////////////////////
        // GPON, TX rate = 1.244G, RX rate = 2.488G
        // ONE common PLL,   fracN pll for both TX and RX, VCO = 9.953G
        //  OS4 for RX and OS8 for TX;    // address 0xD080 = 16'hc074
        // N div = 199.066
        // fracn_ndiv_int[9:0] = 0xC7    --> 0xd0b8 [13:04] = ams_pll_fracn_ndiv_int
        // fracn_div[17:0]     = 0x432C --> 0xd0b8 [01:00] = ams_pll_fracn_div_h
        //                                  & 0xd0b7[15:00] = ams_pll_fracn_div_l
        // vcodiv4 =  0 (fullrate)       --> 0xd0b1 [07:07] = ams_pll_vco_div4 = 0 (Full Rate)
        //
        // kvh_force_val = 01            --> 0xd0b4 [13:12] = ams_pll_kvh_force
        //     // Mario.C: for KVH, please continue to use kvh_force_val='01'.
        //     //          Changing this to kvh_force_val='10' changes the Kvco gain and the VCO tuning range
        //     //          and may result in the extreme VDD/Temp ranges where you run out of VCO tuning range.
        //////////////////////////////////////////////////////////////////////////////////////

        //#(*)AMS_COM_PLL_CONTROL_1 [0xd0b1]
        //# [7]   = ams_pll_vco_div4 = 0
        //# [6]   = ams_pll_vco_div2 = 0
        //# [5:4] = ams_pll_fp3_ctrl = 3
        //# [3]   = ams_pll_fp3_rh   = 1
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_1, 0x0038, 0x00f8);
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x0001, 0x0001);
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x5077, 0xffff);
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_6, 0x0001, 0x0021);
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_7, 0x432c, 0xffff);
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x8c70, 0xffff); // [14]pll_ditheren = 0
    }
    //#===================================================================================================
    else if (mode == XGPON_10_2)
    {   //(*) XGPON OS mode w PLL Ref-Clock=50MHz.
        //////////////////////////////////////////////////////////////////////////////////////
        // XGPON/NGPON2(10/2.5), TX rate = 2.488G, RX rate = 9.95328G
        //  2 PLLs, fracN pll for both TX and RX, VCO = 9.953G
        // OS1 for RX and OS4 for TX;    // address 0xD080 = 16'hc040
        // N div = 199.066
        // fracn_ndiv_int[9:0] = 0xC7    --> 0xd0b8 [13:04] = ams_pll_fracn_ndiv_int
        // fracn_div[17:0]     = 0x432C --> 0xd0b8 [01:00] = ams_pll_fracn_div_h
        //                                  & 0xd0b7[15:00] = ams_pll_fracn_div_l
        // vcodiv4 =  0 (fullrate)       --> 0xd0b1 [07:07] = ams_pll_vco_div4 = 0 (Full Rate)
        // kvh_force_val = 01            --> 0xd0b4 [13:12] = ams_pll_kvh_force
        //////////////////////////////////////////////////////////////////////////////////////

        // #(*)AMS_COM_PLL_CONTROL_1 [0xd0b1]
        // # [7]   = ams_pll_vco_div4 = 0
        // # [6]   = ams_pll_vco_div2 = 0
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_1, 0x0000, 0x00c0);
        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_1, 0x0000, 0x00c0);

        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x5077, 0xffff);  // pll_kvh_force = '01'
        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_4, 0x5077, 0xffff);

        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_7, 0x432c, 0xffff);
        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_7, 0x432c, 0xffff);

        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x8c70, 0xffff);  // [14]pll_ditheren = 0
        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_8, 0x8c70, 0xffff);

        if (std == STD_G__SubRate)
        {   // SubRate mode
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_1, 0x0080, 0x00c0);  // # PLL0 set to SubRate -> pll_vco_div4 [bit7]=1
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x4377, 0xffff);  // # vco_div4 = 1 [SubRate]--> pll_kvh_force = '00', pll_2rx_clkbw = '11'
        }

    }
    //#===================================================================================================
    else if (mode == NGPON_10_10)
    {   //(*) NGPON2(10/10) Full-Rate, TX rate = 9.95328G, RX rate = 9.95328G
        //////////////////////////////////////////////////////////////////////////////////////
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_1, 0x0000, 0x00c0);  // pll_vco_div4[7]= pll_vco_div2[6] = 0
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x1000, 0x3000);  // [13:12] = ams_pll_kvh_force = '01'
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x0000, 0x0300);  // [09:08] = pll_2rx_clkbw     = '00'
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_6, 0x0000, 0x0001);  // Mario.C: Do not use 0xD0B6[bit0]=test_fracn_en. It is only a test mode.

        if (pll_refclk== PADREF)
        {
            //(*)PLL Ref-Clock= PADREF = 155.52MHz -->  Common PLL=Full-Rate, Integer Mode   (Ndiv = 64)
            //////////////////////////////////////////////////////////////////////////////////////
            // N div = 64                      --> 0xd127 [03:00] = pll_mode = 4'b1001
            // fracn_ndiv_int[9:0] = 0x40
            // fracn_div[17:0] = 0x0
            // vcodiv4 = 0
            //
            // pll_force_kvh_bw =  0           --> 0xd0b4 [14]    = force_kvh_bw = 0
            // pll_kvh_force    = '01'         --> 0xd0b4 [13:12] = ams_pll_kvh_force
            // pll_2rx_clkbw    = '00'         --> 0xd0b4 [9:8]   = 2'b00
            //////////////////////////////////////////////////////////////////////////////////////
            writePONSerdesReg(0x0800, 0xd127, 0x0009, 0x000f); // pll_mode[3:0] = 0x9 --> N-Div = 64
            // writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x0000, 0x4000); // default: Integer(fullrate)   --> bit[14]force_kvh_bw=0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x4000, 0x4000); // (*) Improved jitter: FraqN(fullrate)--> bit[14]force_kvh_bw=1
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x2000, 0x3000); // (*) Improved jitter:  pll_kvh_force = 2
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_7, 0x0000, 0xffff); // pll_fracn_div_l[15:00] = lower 16 bits of fracn_div[17:16]=0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x0000, 0x0003); // pll_fracn_div_h[01:00] = upper 2MSB bits of fracn_div[17:16]=0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x0008, 0x0008); // pll_fracn_bypass[3] = 0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x0400, 0x3ff0); // pll_fracn_ndiv_int[13:04] = 0x40
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x0000, 0x4000); // pll_ditheren [14]   = 0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x0000, 0x8000); // pll_fracn_sel[15]   = 0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_1, 0x0000, 0x0030); // pll_fp3_ctrl [05:04]= 0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_1, 0x0000, 0x0008); // pll_fp3_rh   [03]   = 0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x0000, 0x0001);
        }
        else
        {
            //(*) PLL Ref-Clock = LCREF = 50MHz -->  Common PLL=Full-Rate, FraqN mode
            //#===============================================================
            /// common PLL, fracN mode, VCO =  9.952G
            // N div = 199.066
            // fracn_ndiv_int[9:0] = 0xC7    --> 0xd0b8 [13:04] = ams_pll_fracn_ndiv_int
            // fracn_div[17:0]     = 0x432C  --> 0xd0b8 [01:00] = ams_pll_fracn_div_h
            //                                  & 0xd0b7[15:00] = ams_pll_fracn_div_l
            // vcodiv4 =  0 (fullrate)       --> 0xd0b1 [07:07] = ams_pll_vco_div4 = 0 (Full Rate)
            //
            // pll_ force_kvh_bw = 1           --> 0xd0b4 [14]    = force_kvh_bw = 1
            // pll_kvh_force     = '01'        --> 0xd0b4 [13:12] = ams_pll_kvh_force
            // pll_2rx_clkbw     = '00'        --> 0xd0b4 [9:8]   = 2'b00
            //////////////////////////////////////////////////////////////////////////////////////
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x4000, 0x4000); // FraqN(fullrate)--> bit[14]force_kvh_bw=1
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_4, 0x2000, 0x3000); // (*) Improved jitter:  pll_kvh_force = 2
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x0000, 0x0008); // pll_fracn_bypass[3] = 0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x0000, 0x0004);
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x0000, 0x4000); // pll_ditheren [14]   = 0
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, 0x8000, 0x8000); // pll_fracn_sel[15]   = 1
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_1, 0x0030, 0x0030); // pll_fp3_ctrl [05:04]= 3
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_1, 0x0008, 0x0008); // pll_fp3_rh   [03]   = 1
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x0001, 0x0001);

            // Get PLL configuration (Integer & Fractional params) from FLASH.  If fails set default
            if (get_pll_fracn_from_flash(&flash_fracn_div, &flash_fracn_ndiv) || (flash_fracn_div > G_FRACN_HIGH) || (flash_fracn_div < G_FRACN_LOW))
            {
                flash_fracn_div = 0x432C;
                flash_fracn_ndiv = 0xC7;
            }

            // set Integer & FracN values into PLL registers
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_7, flash_fracn_div & 0xffff,                 0xffff); // pll_fracn_div_l[15:00]
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, (flash_fracn_div >> 16) & 0x0003,         0x0003); // pll_fracn_div_h[01:00]
            writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_8, (flash_fracn_ndiv << 4) & 0x3ff0,         0x3ff0); // pll_fracn_ndiv_int[13:04]
        }
    }

    //#===================================================================================================

    /* CKRST_CTRL_PLL_SELECT_CONTROL (0xd08d) */
    // # [0] tx_pll_select : 0= Select PLL0 as clock for TX lane
    // # [1] rx_pll_select : 0= Select PLL0 or 1= PLL1  as clock for RX lane
    if (pll_use == TWO_PLLS)
        writePONSerdesReg(0x0800, CKRST_CTRL_PLL_SELECT_CONTROL, 0x0002, 0x0003); // PLL#1  as clock for RX lane
    else // pll== ONE_PLL
        writePONSerdesReg(0x0800, CKRST_CTRL_PLL_SELECT_CONTROL, 0x0000, 0x0003); // PLL#0 as clock for RX lane


    //(*) PLL charge pump settings
    //#===============================================================

    writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x000a, 0x001e); // pll_iqp [04:01] = 0x5 = default

    if ((mode == XGPON_10_2) || (mode == NGPON_10_10))
    {
        //  0xD0B2[bit0]=en_HRz<1> =1,  0xD0B0[bit11]=en_HRz<0>=1  -->>> en_HRz = 6[kOhm]
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x0001, 0x0001);
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_0, 0x0800, 0x0800);

        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_2, 0x0001, 0x0001);
        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_0, 0x0800, 0x0800);

        // set PLL-Current-ChargePump=ipq[bit4:1]= 0x0 -->>> 50[uA]
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x0000, 0x001e);
        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_2, 0x0000, 0x001e);
    }
    //#===============================================================


    //*  #Looptiming Control
    //#(*) DSC_F_ONU10G_looptiming_ctrl_0.
    // RX line rate to TX line rate ratio
    // 000: 1 to 1   001: 1.25 to 1  010: 2 to 1
    // 011: 4 to 1   100: 5 to 1     101: 8.25 to 1
    writePONSerdesReg(0x0800, DSC_F_ONU10G_looptiming_ctrl_0, rx_tx_rate_ratio, 0x0007);


    //#(5.a)  RX=OS4 and TX=OS8 // address 0xD080 = 16'hc074
    //  #     OSR Mode Value: 0= OSx1, 1=OSx2, 4=OSX4, 7=OSX8, A =OSx16 (revB0)
    //  # rx_osr_mode_frc    = 1
    //  # rx_osr_mode_frc_val = mode dependent
    //  # tx_osr_mode_frc     = 1
    //  # tx_osr_mode_frc_val = mode dependent
    if (mode ==GPON_2_1)
        writePONSerdesReg(0x0800, CKRST_CTRL_OSR_MODE_CONTROL, 0xC074, 0xffff); //tx_osr_mode = 7, rx_osr_mode = 4
    else if (mode ==XGPON_10_2)
    {
        if (std == STD_G__SubRate)
            writePONSerdesReg(0x0800, CKRST_CTRL_OSR_MODE_CONTROL, 0x0000, 0xffff); //No OSR modes.
        else //STD_G
            writePONSerdesReg(0x0800, CKRST_CTRL_OSR_MODE_CONTROL, 0xC040, 0xffff); //tx_osr_mode = 4, rx_osr_mode = 0
    }
    else if (mode == NGPON_10_10)
        writePONSerdesReg(0x0800, CKRST_CTRL_OSR_MODE_CONTROL, 0xC000, 0xffff); //tx_osr_mode = 0, rx_osr_mode = 0, TX=RX=Full-Rate


    /* #(*) TX Phase Interpolator Control 0 (0xD070)  */
    writePONSerdesReg(0x0800, TX_PI_LBE_tx_pi_control_0, 0x5003, 0xffff);


    /// #(6) CDR Control
    // fieldSet("cdr_freq_en",             1);
    // fieldSet("osx2p_pherr_gain",        0);
    // fieldSet("cdr_integ_sat_sel",       0);
    // fieldSet("cdr_bwsel_integ_acqcdr",  0);
    // fieldSet("cdr_bwsel_integ_norm",    0);
    // fieldSet("cdr_bwsel_prop_acqcdr",   0);
    // fieldSet("cdr_bwsel_prop_norm",     0);
    // fieldSet("cdr_freq_override_en",    0);
    // fieldSet("cdr_freq_override_val",   690);
    // fieldSet("phase_err_offset",        0);
    // fieldSet("phase_err_offset_en",     0);
    // fieldSet("cdr_phase_sat_ctrl",      1);
    // fieldSet("phase_err_offset_mult_2", 0);
    // fieldSet("pattern_sel",             f); //# For OSx3 and higher, this feature is not supported, leave as reset value of 0xF.

    writePONSerdesReg(0x0800, DSC_A_cdr_control_0, 0x0004, 0x0004);
    writePONSerdesReg(0x0800, DSC_A_cdr_control_2, 0x0000, 0x0300);
    writePONSerdesReg(0x0800, DSC_A_cdr_control_0, 0x0000, 0x0040);
    writePONSerdesReg(0x0800, DSC_B_dsc_sm_ctrl_7, 0x0000, 0x000F);
    writePONSerdesReg(0x0800, DSC_B_dsc_sm_ctrl_7, 0x0000, 0x0F00);
    writePONSerdesReg(0x0800, DSC_B_dsc_sm_ctrl_7, 0x0000, 0x3000);
    writePONSerdesReg(0x0800, DSC_B_dsc_sm_ctrl_7, 0x0000, 0xC000);
    writePONSerdesReg(0x0800, DSC_A_cdr_control_0, 0x0000, 0x0080);
    writePONSerdesReg(0x0800, DSC_A_cdr_control_1, 0x02B2, 0xFFFF);  // 0x2b2=690[dec]
    writePONSerdesReg(0x0800, DSC_B_dsc_sm_ctrl_8, 0x0000, 0x000F);
    writePONSerdesReg(0x0800, DSC_B_dsc_sm_ctrl_8, 0x0000, 0x0300);
    writePONSerdesReg(0x0800, DSC_A_cdr_control_0, 0x0001, 0x0001);
    writePONSerdesReg(0x0800, DSC_A_cdr_control_2, 0x0000, 0x0002);
    writePONSerdesReg(0x0800, DSC_A_cdr_control_2, 0x00f0, 0x00F0); // fieldSet("pattern_sel", f);  //prev.value = 0x3


    // (*)  Analog = VGA, and PF programming
    writePONSerdesReg(0x0800, DSC_E_dsc_e_pf_ctrl, 0x0007, 0x000F); // _set_rx_pf_main(7);
    writePONSerdesReg(0x0800, DSC_E_dsc_e_pf2_lowp_ctrl, 0x0003, 0x0007); // _set_rx_pf2(3);
    writePONSerdesReg(0x0800, DSC_C_dfe_vga_override, 0x0000, 0x3E00); // _set_rx_vga(32);
    writePONSerdesReg(0x0800, DSC_C_dfe_vga_override, 0x0100, 0x01FF);
    writePONSerdesReg(0x0800, DSC_C_dfe_vga_override, 0x8000, 0x8000);

    // (*) LBE setup
    //*********************************************************************************
    // SubRate TX :  tx_lbe4_0[bit8]=1 & ams_tx_txclk4_en=1 --->> enable 4bit LBE mode
    // OverSample :  tx_lbe4_0[bit8]=0 & ams_tx_txclk4_en=0 --->> enable 1bit LBE mode.

    if (std == STD_G__SubRate) // SubRate
        writePONSerdesReg(0x0800, TX_PI_LBE_tx_lbe_control_0, 0x0100, 0x0100); // tx_lbe4_0[bit8]=1
    else // OverSample
        writePONSerdesReg(0x0800, TX_PI_LBE_tx_lbe_control_0, 0x0000, 0x0100); // tx_lbe4_0[bit8]=0

    // (#11) RX-&-TX PON_MAC_CLK Division Control and SYNC_E_CLK
    // *********************************************************************************
    if (mode ==GPON_2_1)
    {
#if defined (CONFIG_BCM96858)
        writePONSerdesReg(0x0800, TX_PI_LBE_tx_lbe_control_0, 0x0010, 0x01FF); /* tx_lbe4_0[bit8]=0  lbe_delay_ctrl[7:5]=0  data_delay_ctrl[4:0]=0x10 */
#endif
        /* ams_tx_tx_pon_mac_ctrl=6 ams_tx_tx_sync_e_ctrl=6  ams_tx_tx_wclk4_en=1 */
        /* ams_rx_rx_pon_mac_ctrl=4 */
        writePONSerdesReg(0x0800, AMS_TX_TX_CONTROL_1, 0x006c, 0x007f);
        writePONSerdesReg(0x0800, AMS_RX_RX_CONTROL_2, 0x0004, 0x0007);
    }
    else if (mode ==XGPON_10_2)
    {
        if (std == STD_G)
            //(*) TX : LBE & MAC clocks control  (OverSample mode)
            writePONSerdesReg(0x0800, AMS_TX_TX_CONTROL_1, 0x0026, 0x007f);   // bit[0]   = ams_tx_tx_wclk4_en = 0
        else //std == STD_G__SubRate
            //(*) TX : LBE & MAC clocks control  (OverSample mode)
            writePONSerdesReg(0x0800, AMS_TX_TX_CONTROL_1, 0x0027, 0x007f);   // bit[0]   = ams_tx_tx_wclk4_en = 1

        // bit[6:4] = ams_tx_tx_pon_mac_ctrl = 2 = tx_pon_mac clock division control
        // (*) RX : MAC clock control
        writePONSerdesReg(0x0800, AMS_RX_RX_CONTROL_2, 0xA804, 0xFFFF);// bit[2:0] = ams_rx_rx_pon_mac_ctrl = 4 = rx_pon_mac clock division control
    }
    else if (mode == NGPON_10_10)
    {
        //(*) TX : LBE & MAC clocks control  (OverSample mode)
        writePONSerdesReg(0x0800, AMS_TX_TX_CONTROL_1, 0x0000, 0x0001); // bit[0]   = ams_tx_tx_wclk4_en = 0
        writePONSerdesReg(0x0800, AMS_TX_TX_CONTROL_1, 0x000c, 0x000e); // bit[3:1] = tx_sync_e_ctrl = 6
        writePONSerdesReg(0x0800, AMS_TX_TX_CONTROL_1, 0x0060, 0x0070); // bit[6:4] = ams_tx_tx_pon_mac_ctrl = 6 = tx_pon_mac clock division control
        // (*) RX : MAC clock control
        writePONSerdesReg(0x0800, AMS_RX_RX_CONTROL_2, 0xab04, 0xffff); // bit[2:0] = ams_rx_rx_pon_mac_ctrl = 4 = rx_pon_mac clock division control
    }


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // (*)   De-assert core_dp_s_rstb --> will start the PLL calibration.
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    writePONSerdesReg(0x0800, CORE_PLL_COM_TOP_USER_CONTROL, 0x2000, 0x2000);   /* # [13] core_dp_s_rstb = 1 */
    if (pll_use == TWO_PLLS)
        writePONSerdesReg(0x0900, CORE_PLL_COM_TOP_USER_CONTROL, 0x2000, 0x2000);

    mdelay(3);  // wait for PLL Lock

    // (*) Verify Both PLL's are Locked
    rd_serdes = readPONSerdesReg(0x0800, PLL_CAL_COM_CTL_STATUS_0);
    __logDebug("\nPLL0_Lock=%x. ", ((rd_serdes & 0x0100) >>8));
    if (pll_use == TWO_PLLS)
    {
        rd_serdes = readPONSerdesReg(0x0900, PLL_CAL_COM_CTL_STATUS_0);
        __logDebug("  PLL1_Lock=%x. ", ((rd_serdes & 0x0100) >>8));
    }

    udelay(100);

    /* #(11) De-assert ln_dp_s_rstb  */
    /* ln_dp_s_rstb = 1 = serdes data-path out of reset  (0xd081)  */
    writePONSerdesReg(0x0800, CKRST_CTRL_LANE_CLK_RESET_N_POWERDOWN_CONTROL, 0x0002, 0x0002); /* # bit[2] =ln_dp_s_rstb =1 */

    /* pmi_lp_write(DEVID_1, LANE_BRDCST, _AMS_TX_TX_CONTROL_0, 0x0000, ~(0x00c0))*/
    writePONSerdesReg(0x0800, AMS_TX_TX_CONTROL_0, 0x0000, 0xffff);

    /* Ignore SigDetect:  0xd010[09] = ignore_sigdet =1 */
    writePONSerdesReg(0x0800, DSC_B_dsc_sm_ctrl_0, 0x0200, 0x0200);

    ///////////////////////////////////////////////
    // (*)   PLL PPM Adjustment for RX=10G modes
    // ///////////////////////////////////////////
#ifdef PLL_PPM_ADJ
    if (mode == NGPON_10_10)
    {
        mdelay(100);
        pll_ppm_adj_FracN_10G(mode, PPM_TARGET, TRUE);
    }
#endif

    /*  Adjust RX PI intial location  */
    rx_pi_spacing(clk90_offset, p1_offset);

    mdelay(1);

    // (12) Verify RX_DSC_Lock and DSC_State=Done
    rd_serdes = readPONSerdesReg(0x0800, DSC_B_dsc_sm_status_dsc_lock);  // RX_PMD_LOCK[0xD01A], bit[0] = 1
    __logDebug("  RX_DSC_LOCK =0x%08x. ", rd_serdes);

    rd_serdes = readPONSerdesReg(0x0800, DSC_B_dsc_sm_status_dsc_state); // # Get DSC_STATE [0xD0E1] bits[15:11] = 9 = Done
    __logDebug("  DSC_State=0x%08x. ", (rd_serdes >>11));

    // ####### SerDes Initialization =  Done    ###############################
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // ###############  Final Init Settings   ###############################

    //  TX FIR and Amplitude -  per board type
    if ((mode ==XGPON_10_2) && (optics_type == BCM_I2C_PON_OPTICS_TYPE_PMD))
    {
        //For 6858XPMD board (19/9/16 - simple optimization without enabling DFE)
        writePONSerdesReg(0x0800, DSC_E_dsc_e_ctrl, 0x0080, 0x0080);
        writePONSerdesReg(0x0800, DSC_E_dsc_e_pf_ctrl, 0x0008, 0x000F);
        writePONSerdesReg(0x0800, DSC_E_dsc_e_pf2_lowp_ctrl, 0x0000, 0x0007);
    }

    /* Disable/Bypass Signal-Detect */
    rx_sigdet_dis();

    /* Clear RX/TX Gearbox FIFO pointers*/
    if (mode == GPON_2_1)
    {
        /* GPON_GEARBOX_2.CONFIG_BURST_DELAY_CYC = 0x3  [ln:192 @ GponEnvChip.cc ]   */
        RU_FIELD_WRITE(0, GPON, GEARBOX_2, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, FIFO_CFG_1_TX_WR_POINTER),  0x00);
        RU_FIELD_WRITE(0, GPON, GEARBOX_2, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, FIFO_CFG_1_TX_RD_POINTER),  0x10);

        RU_FIELD_WRITE(0, GPON, GEARBOX_0, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, SW_RESET_TXFIFO_RESET),   1);
        udelay(10);
        RU_FIELD_WRITE(0, GPON, GEARBOX_0, CHOP(CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_, SW_RESET_TXFIFO_RESET),   0);
    }

#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
    if ((mode == XGPON_10_2) || (mode == NGPON_10_10))
    {
#if defined(CONFIG_BCM963158)
        /*  WAN_TOP_RESET (PCS RESET)  */
        RU_FIELD_WRITE(0, TOP_RESET,  T, CFG_PCS_RESET_N, 0);
        RU_FIELD_WRITE(0, TOP_RESET,  T, CFG_PCS_RESET_N, 1);
        RU_FIELD_WRITE(0, TOP_RESET,  T, CFG_PCS_RESET_N, 0);
        RU_FIELD_WRITE(0, TOP_RESET,  T, CFG_PCS_RESET_N, 1);
#endif

        /* #RX FIFO Control */
        RU_FIELD_WRITE(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFORDPTR  , 8);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFOPTRLD  , 1);

        RU_FIELD_WRITE(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAFLIP, 1);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMODE       , 1);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXEN         , 1);

        /* (*) NGPON_GEARBOX_RX : Load FIFO Pointers  [//cfNGponGboxRxFifoPtrLd[4]:1-->0 ] */
        RU_FIELD_WRITE(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFOPTRLD  , 0);

        /* #TX FIFO Control */
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFODATARDPTR, 8);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDFLIP   , 1);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAFLIP  , 1);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDPTRLD , 1);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOPTRLD    , 1);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDOFF   , 0);

        /* (*) NGPON_GEARBOX_TX : Load FIFO Pointers */
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDPTRLD , 0);
        RU_FIELD_WRITE(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOPTRLD    , 0);
    }
#endif

    /* Enable Laser-BE signal: Laser_OE = 1   */
    RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_OE),  1);
    RU_REG_READ   (0, MISC, 3, rd_data);
    __logDebug("\n WAN_TOP_MISC_3 = 0x%08x", rd_data);


} // END of serdes_init() function
//////////////////////////////////////////////////////////////////////////////////////////////////


void pon_drv_gpon_init(serdes_wan_type_t wan_type, int pll50mhz)
{
    pon_params_t *pon_params;

#if defined CONFIG_BCM96856 && (defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE))
    chip_rev = UtilGetChipRev();
#endif
    pon_params = &GPON_2_1_params; /* default */

    switch (wan_type)
    {
        /* *****  G-PON MODES ***** */
        case SERDES_WAN_TYPE_GPON:
            __logInfo("\n\nserdes initialization: GPON Start\n");
            pon_params = &GPON_2_1_params;
            break;
#if defined(CONFIG_BCM_NGPON) || defined(CONFIG_BCM_NGPON_MODULE)
        case SERDES_WAN_TYPE_XGPON_10G_2_5G:   /* XGPON */
        case SERDES_WAN_TYPE_NGPON_10G_2_5G:
            __logInfo("\n\nserdes initialization: XGPON Start\n");
            pon_params = &XGPON_10_2_params;
#if defined CONFIG_BCM96856
            if (chip_rev == 0xB0)
            {
                __logInfo("serdes in Subrate mode\n");
                pon_params = &XGPON_10_2_SubRate_params;
            }
#endif
            break;
        case SERDES_WAN_TYPE_NGPON_10G_10G:   /* NGPON / XGS */
            __logInfo("\n\nserdes initialization: NGPON Start\n");
            if (!pll50mhz)
            {
                NGPON_10_10_params.refclk = PADREF;  /* PADREF = 155.52MHz refclk */
            }
            pon_params = &NGPON_10_10_params;
            break;
#endif

        default:
            __logError("\n Undefined PON type %d.  Using GPON as default\n", wan_type);
            break;
    } // End of switch (wan_type)
    /////////////////////////////////////////////////

    wan_init(pon_params);
    serdes_init(pon_params);

    __logInfo("\nserdes initialization: Done");

} // End of 'pon_init(serdes_wan_type_t wan_serdes_type)'

///////////////////////////////////////////////////////////////////////////////////////////////////

