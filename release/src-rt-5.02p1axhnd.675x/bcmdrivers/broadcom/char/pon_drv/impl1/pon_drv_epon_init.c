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
 * pon_epon_init.c -- Bcm Pon driver: init sequence for all E-STD PON modes:
 * EPON_1_1, EPON_2_1, AE_2_2, EPON_10_10, EPON_10_1
 *
 * Description:
 *     Built from initialization script of VLSI team
 *
 * Authors: Fuguo Xu, Akiva Sadovski,  Vitaly Zborovski
 *
 * $Revision: 2.0 $
 *
 * 2017.July: updated by VZ
 ****************************************************************************/

#include <linux/delay.h>
#include <linux/kernel.h>
#include "pon_drv.h"
#include "pmc_drv.h"
#include "ru_types.h"
#include <bcmsfp_i2c.h>
#include "pon_drv_serdes_util.h"
#include "ru.h"
#include "early_txen_ag.h"
#include "wan_drv.h"
#include "pon_drv_gpon_init.h"

#undef MISC  // to allow access by name to MISC registers in Gearbox


////////////////////////////////////////////////////////////////////////////////////////////////////
// Definition of various SerDes modes configurations
///////////////////////////////////////////////////////////////////////////////////////////////////
pon_params_t EPON_1_1_params =
{
        .mode             = EPON_1_1  ,
        .std              = STD_E     ,
        .refclk           = LCREF     ,  // LCREF = 50MHz ref.clock
        .pll_use          = ONE_PLL   ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x0       ,  // 000: 1 to 1
};

pon_params_t EPON_2_1_params =
{
        .mode             = EPON_2_1  ,
        .std              = STD_E     ,
        .refclk           = LCREF     ,  // LCREF = 50MHz ref.clock
        .pll_use          = ONE_PLL   ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x2       ,  // 010: 2 to 1
};

pon_params_t EPON_10_1_params =
{
        .mode             = EPON_10_1 ,
        .std              = STD_E     ,
        .refclk           = LCREF     ,  // LCREF = 50MHz ref.clock
        .pll_use          = TWO_PLLS  ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x5       ,  // 101: 8.25 to 1
};

pon_params_t EPON_10_10_params =
{
        .mode             = EPON_10_10,
        .std              = STD_E     ,
        .refclk           = LCREF     ,  // LCREF = 50MHz ref.clock
        .pll_use          = ONE_PLL   ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x0       ,  // 000: 1 to 1
};

pon_params_t AE_2_2_params =
{
        .mode             = AE_2_2    ,
        .std              = STD_E     ,
        .refclk           = LCREF     ,  // LCREF = 50MHz ref.clock
        .pll_use          = ONE_PLL   ,
        .clk90_offset     = 32        ,
        .p1_offset        = 0         ,
        .rx_tx_rate_ratio = 0x0       ,  // 000: 1 to 1
};


static void serdes_init (pon_params_t *pon_params)
{
    uint32_t rd_data = 0x0;
    uint32_t wr_data = 0x0;

    uint16_t rd_serdes   = 0x0;
    uint16_t wr_serdes   = 0x0;
    uint16_t wr_mask     = 0x0;

    uint8_t mode            = pon_params->mode;
    uint8_t clk90_offset    = pon_params->clk90_offset;
    uint8_t p1_offset       = pon_params->p1_offset;
    uint8_t pll_use         = pon_params->pll_use;
    uint8_t pll_refclk      = pon_params->refclk;
    uint8_t std             = pon_params->std;

    int rx_pll_sel = 0;
    uint16_t tx_pll_id = serdes_PLL_0;
    uint16_t rx_pll_id = serdes_PLL_0;

    uint16_t tx_pll_vco_div2      = 0x0;
    uint16_t rx_pll_vco_div2      = 0x0;
    uint16_t tx_pll_vco_div4      = 0x0;
    uint16_t rx_pll_vco_div4      = 0x0;
    uint16_t tx_pll_force_kvh_bw  = 0x0;
    uint16_t rx_pll_force_kvh_bw  = 0x0;
    uint16_t tx_pll_kvh_force     = 0x0;
    uint16_t rx_pll_kvh_force     = 0x0;
    uint16_t tx_pll_2rx_bw        = 0x0;
    uint16_t rx_pll_2rx_bw        = 0x0;
    uint16_t tx_pll_fracn_sel     = 0x0;
    uint16_t rx_pll_fracn_sel     = 0x0;
    uint16_t tx_pll_ditheren      = 0x0;
    uint16_t rx_pll_ditheren      = 0x0;
    uint32_t tx_pll_fracn_div     = 0x0;
    uint32_t rx_pll_fracn_div     = 0x0;
    uint32_t tx_pll_fracn_ndiv    = 0x0;
    uint16_t rx_pll_fracn_ndiv    = 0x0;
    uint16_t tx_pll_mode          = 0x0;
    uint16_t rx_pll_mode          = 0x0;
    uint16_t tx_pon_mac_ctrl      = 0x0;
    uint16_t tx_sync_e_ctrl       = 0x0;
    uint16_t rx_pon_mac_ctrl      = 0x0;
    uint16_t rx_tx_rate_ratio     = 0x0;
    uint16_t rx_osr_mode          = 0x0;
    uint16_t tx_osr_mode          = 0x0;

    switch (mode)
    {
        case AE_10_10:
        case EPON_10_10:
        {
            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x0;
            rx_pll_vco_div4     = 0x0;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_kvh_force    = 0x1;
            rx_pll_kvh_force    = 0x1;
            tx_pll_2rx_bw       = 0x0;
            rx_pll_2rx_bw       = 0x0;
            tx_pll_fracn_sel    = 0x1;
            rx_pll_fracn_sel    = 0x1;
            tx_pll_ditheren     = 0x1;
            rx_pll_ditheren     = 0x1;
            rx_pll_fracn_div    = 0x10000;
            rx_pll_fracn_ndiv   = 0x0ce;
            tx_pll_mode         = 0x2;
            rx_pll_mode         = 0x2;
            tx_pon_mac_ctrl     = 0x7;
            rx_pon_mac_ctrl     = 0x7;
            tx_sync_e_ctrl      = 0x7;
            rx_tx_rate_ratio    = 0x0;
            rx_osr_mode         = 0x0;
            tx_osr_mode         = 0x0;

            //   LOAD from FLASH : PLL Fractional settings ! ONLY for 10_10G EPON/AE modes.
            if ((pll_use == ONE_PLL) && (pll_refclk == LCREF) && (tx_pll_fracn_sel==1)) // 10G mode & PLL ref.clk=50MHz
            {
                // Get PLL configuration (Integer & Fractional params) from FLASH
                if (get_pll_fracn_from_flash(&tx_pll_fracn_div, &tx_pll_fracn_ndiv) || (tx_pll_fracn_div > E_FRACN_HIGH) || (tx_pll_fracn_div < E_FRACN_LOW))
                {
                    tx_pll_fracn_div    = 0x10000;
                    tx_pll_fracn_ndiv   = 0x0ce;
                }
            }
            break;
        }
        case AE_10_1:
        case EPON_10_1:
        {
            rx_pll_sel = 1;
            rx_pll_id = serdes_PLL_1;

            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x0;
            rx_pll_vco_div4     = 0x0;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_kvh_force    = 0x1;
            rx_pll_kvh_force    = 0x1;
            tx_pll_2rx_bw       = 0x0;
            rx_pll_2rx_bw       = 0x0;
            tx_pll_fracn_sel    = 0x1;
            rx_pll_fracn_sel    = 0x1;
            tx_pll_ditheren     = 0x1;
            rx_pll_ditheren     = 0x1;
            tx_pll_fracn_div    = 0x00000;
            rx_pll_fracn_div    = 0x10000;
            tx_pll_fracn_ndiv   = 0x0c8;
            rx_pll_fracn_ndiv   = 0x0ce;
            tx_pll_mode         = 0x5;
            rx_pll_mode         = 0x5;
            tx_pon_mac_ctrl     = 0x0;
            tx_sync_e_ctrl      = 0x0;
            rx_pon_mac_ctrl     = 0x7;
            rx_tx_rate_ratio    = 0x5;
            rx_osr_mode         = 0x0;
            tx_osr_mode         = 0x7;

            break;
        }
        case AE_2_1:
        case EPON_2_1:
        {
            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x0;
            rx_pll_vco_div4     = 0x0;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_2rx_bw       = 0x0;
            rx_pll_2rx_bw       = 0x0;
            tx_pll_fracn_sel    = 0x1;
            rx_pll_fracn_sel    = 0x1;
            tx_pll_ditheren     = 0x1;
            rx_pll_ditheren     = 0x1;
            tx_pll_fracn_div    = 0x00000;
            rx_pll_fracn_div    = 0x00000;
            tx_pll_fracn_ndiv   = 0x0c8;
            rx_pll_fracn_ndiv   = 0x0c8;
            tx_pll_mode         = 0x5;
            rx_pll_mode         = 0x5;
            tx_pon_mac_ctrl     = 0x0;
            tx_sync_e_ctrl      = 0x0;
            rx_pon_mac_ctrl     = 0x0;
            rx_tx_rate_ratio    = 0x2;
            rx_osr_mode         = 0x4;
            tx_osr_mode         = 0x7;

            break;
        }
        case EPON_2_2:
        case AE_2_2:
        {
            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x0;
            rx_pll_vco_div4     = 0x0;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_2rx_bw       = 0x0;
            rx_pll_2rx_bw       = 0x0;
            tx_pll_fracn_sel    = 0x1;
            rx_pll_fracn_sel    = 0x1;
            tx_pll_ditheren     = 0x1;
            rx_pll_ditheren     = 0x1;
            tx_pll_fracn_div    = 0x00000;
            rx_pll_fracn_div    = 0x00000;
            tx_pll_fracn_ndiv   = 0x0fa;
            rx_pll_fracn_ndiv   = 0x0fa;
            tx_pll_mode         = 0x5;
            rx_pll_mode         = 0x5;
            tx_pon_mac_ctrl     = 0x5;
            tx_sync_e_ctrl      = 0x0;
            rx_pon_mac_ctrl     = 0x5;
            rx_tx_rate_ratio    = 0x0;
            rx_osr_mode         = 0x4;
            tx_osr_mode         = 0x4;

            break;
        }
        case AE_1_1:
        case EPON_1_1:
        {
            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x0;
            rx_pll_vco_div4     = 0x0;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_2rx_bw       = 0x0;
            rx_pll_2rx_bw       = 0x0;
            tx_pll_fracn_sel    = 0x1;
            rx_pll_fracn_sel    = 0x1;
            tx_pll_ditheren     = 0x1;
            rx_pll_ditheren     = 0x1;
            tx_pll_fracn_div    = 0x00000;
            rx_pll_fracn_div    = 0x00000;
            tx_pll_fracn_ndiv   = 0x0c8;
            rx_pll_fracn_ndiv   = 0x0c8;
            tx_pll_mode         = 0x5;
            rx_pll_mode         = 0x5;
            tx_pon_mac_ctrl     = 0x0;
            tx_sync_e_ctrl      = 0x0;
            rx_pon_mac_ctrl     = 0x0;
            rx_tx_rate_ratio    = 0x0;
            rx_osr_mode         = 0x7;
            tx_osr_mode         = 0x7;

            break;
        }

        default:
            break;
    }


    // ##########################################################
    //  Gearbox Initialization
    // ##########################################################

    /*  # RESCAL Init */
    sgb_rescal_init();


    // # ##########################################################
    // #        Powerup SERDES, bring WAN_TOP out of reset
    // # ##########################################################

    // #(1) Assert POR reset by forcing pmd_por_h_rstb pin to 1'b0
    // ## WAN_TOP_MISC_2 {0x80144048}
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_POR_H_RSTB),  0);


    // WAN_TOP_SCRATCH   //
#if defined (CONFIG_BCM96858) || defined (CONFIG_BCM96856) || defined(CONFIG_BCM963158)
    wr_data = 0x01234567;
    RU_REG_WRITE(0, TOP_SCRATCH, ATCH, wr_data);
    RU_REG_READ (0, TOP_SCRATCH, ATCH, rd_data);
    __logDebug("\n WAN_TOP_SCRATCH = 0x%08x", rd_data);

    wr_data = 0x89abcdef;
    RU_REG_WRITE(0, TOP_SCRATCH, ATCH, wr_data);
    RU_REG_READ (0, TOP_SCRATCH, ATCH, rd_data);
    __logDebug("\n WAN_TOP_SCRATCH = 0x%08x", rd_data);
#endif

    // Clear Gearbox registers
    wr_data = 0x0;
    RU_REG_WRITE(0, MISC,       0,        wr_data);
    RU_REG_WRITE(0, MISC,       1,        wr_data);
    RU_REG_WRITE(0, MISC,       2,        wr_data);
    RU_REG_WRITE(0, MISC,       3,        wr_data);
    RU_REG_WRITE(0, WAN_SERDES, PLL_CTL,  wr_data);
    RU_REG_WRITE(0, WAN_SERDES, PRAM_CTL, wr_data);

    wan_serdes_temp_init();

    // #(2) set PLL Ref Clock input
    set_pll_refclk(pon_params->refclk);  // LCREF =50MHz or PADREF =155.52MHz

    // #(3) De-assert POR reset {0x80144048}
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_POR_H_RSTB      ),  1); // release PMD reset
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_CORE_1_DP_H_RSTB),  1); // release DP reset
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_CORE_0_DP_H_RSTB),  1); // release DP reset
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_LN_H_RSTB       ),  1); // release Lane reset
    RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_LN_DP_H_RSTB    ),  1); // release Lane reset
    RU_REG_READ   (0, MISC, 2, rd_data);
    __logDebug("\n WAN_TOP_MISC_2 = 0x%08x", rd_data);

    /* MISC_1 = 0 */
    RU_FIELD_WRITE(0, MISC, 1, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_CORE_1_MODE),  0);
    RU_FIELD_WRITE(0, MISC, 1, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_CORE_0_MODE),  0);
    RU_REG_READ(0, MISC, 1, rd_data);
    __logDebug("\n WAN_TOP_MISC_1 = 0x%08x", rd_data);

    /* MISC_3 : Set Gearbox to "NGPON" mode  (LASER_MODE / LASER_OE / INTERFACE_SELECT)  */
    RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_OE),  0); //  Disable Laser(BE) during init

    if (std == STD_E)
    {
        RU_FIELD_WRITE(0, MISC, 2, CFGACTIVEETHERNET2P5, 1); //  cfgActiveEthernet2P5.set(1); // Always use the divideBy1 clock//#endif

        /* set Oversample modes */
        RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_RX_OSR_MODE),  rx_osr_mode); // release PMD reset
        RU_FIELD_WRITE(0, MISC, 2, CHOP(CR_XGWAN_TOP_WAN_MISC_, PMD_TX_OSR_MODE),  tx_osr_mode); // release PMD reset
        RU_REG_READ(0, MISC, 2, rd_data);
        __logDebug("\n WAN_TOP_MISC_2 = 0x%08x", rd_data);

#if defined (CONFIG_BCM96858)
        if ((mode == EPON_2_2) || (mode == AE_2_2))
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_, EPON_GBOX_AE_2P5_FULL_RATE_MODE),  1);  //1: Explicit 2.5G Full Rate Serdes Mode.  0: all other modes.

        if ((mode == EPON_2_1) || (mode == AE_2_1))
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_, EPON_GBOX_PON_RX_WIDTH_MODE), 1);
        else
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_, EPON_GBOX_PON_RX_WIDTH_MODE), 0);

#elif (defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)|| defined(CONFIG_BCM96878) || defined(CONFIG_BCM963158))
        if ((mode == EPON_2_2) || (mode == AE_2_2))
#if defined(CONFIG_BCM963158)
            RU_FIELD_WRITE(0, MISC, 0, CHOP(CR_XGWAN_TOP_WAN_MISC_, EPON_GBOX_AE_2P5_FULL_RATE_MODE),  1);  //1: Explicit 2.5G Full Rate Serdes Mode.  0: all other modes.
#else
            RU_FIELD_WRITE(0, MISC, 0, CHOP(CR_XGWAN_TOP_WAN_MISC_, EPON_AE_2P5_FULL_RATE_MODE),  1);  //1: Explicit 2.5G Full Rate Serdes Mode.  0: all other modes.
#endif

        if ((mode == EPON_2_1) || (mode == AE_2_1))
            RU_FIELD_WRITE(0, MISC, 0, CHOP(CR_XGWAN_TOP_WAN_MISC_, EPON_GBOX_PON_RX_WIDTH_MODE), 1);
        else
            RU_FIELD_WRITE(0, MISC, 0, CHOP(CR_XGWAN_TOP_WAN_MISC_, EPON_GBOX_PON_RX_WIDTH_MODE), 0);
#endif

#if (defined (CONFIG_BCM96858) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878))
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE          ),  0); // laser_mode =0= EPON
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT),  0); // wan_interface_select = 0= EPON
#if (defined (CONFIG_BCM96858))
        if ((mode == EPON_10_10) || (mode == AE_10_10))
        {
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE          ),  1); // laser_mode= 1 = 10G-EPON
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT),  1);  // wan_interface_select = 1 = 10G-EPON
        }
#elif (defined(CONFIG_BCM96856))
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT_HS),  0); // wan_interface_select_hs = 0= EPON
        if ((mode == EPON_10_10) || (mode == AE_10_10))
        {
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE             ),  1); // laser_mode= 1 = 10G-EPON
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT_HS),  1); // wan_interface_select_hs = 1= 10G-EPON
        }
#endif
#endif

#if defined(CONFIG_BCM963158) //  ??? check / modify for 63158 different reg names ???
        RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE              ),  0); // laser_mode= 0 = EPON

        //LASER_MODE  0: EPON;1: 10G EPON; 2: GPON; 3: NGPON; 4 (or higher) : Disable laser
        //WAN_INTERFACE_SELECT @63158 = //0: 100M AE; 1: 1G AE; 2: 2.5G AE; 3: 10G AE; 4: GPON; 5: 1G EPON; 6: 10G EPON;7: NGPON
        if (mode == EPON_10_10)
        {
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE           ),  1); 
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ),  6);
        }
        else if (mode == AE_10_10)
        {
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE           ),  1);         
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ),  3);
        }
        else if (mode == EPON_10_1)
        {
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE           ),  0);        
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ),  5);
        }
        else if (mode == AE_10_1)
        {
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_MODE           ),  0);  
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ),  1);
        }
        else if ((mode == EPON_2_2) || (mode == AE_2_2))
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ),  2);
        else if ( (mode == EPON_1_1) )
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ),  5);
        else if ( (mode == AE_1_1) )
            RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, WAN_INTERFACE_SELECT ),  1);
#endif     // CONFIG_BCM963158
    }


// EPON_10G_GEARBOX
#if defined (CONFIG_BCM96858)
#define TEN_G_GEARBOX_PREFFIX_AG  TEN_G_GEARBOX
#define TEN_G_GEARBOX_SUFFIX_AG GEARBOX
#elif defined(CONFIG_BCM96856) || defined(CONFIG_BCM963158)
#define TEN_G_GEARBOX_PREFFIX_AG WAN_EPON
#define TEN_G_GEARBOX_SUFFIX_AG 10G_GEARBOX
#endif
#if defined (CONFIG_BCM96858) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM963158)
    if ((mode == EPON_10_10) || (mode == EPON_10_1) || (mode == AE_10_10) || (mode == AE_10_1))
    {
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF   , 0);
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD, 0);
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN , 0);
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_RX_DATA_END   , 0);
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_CLK_EN        , 1);
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN  , 1);
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN  , 1);
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN  , 1);
        RU_FIELD_WRITE(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN  , 1);

        RU_REG_READ(0, TEN_G_GEARBOX_PREFFIX_AG, TEN_G_GEARBOX_SUFFIX_AG, rd_data);
        __logDebug("\n EPON_10G_GEARBOX = 0x%08x", rd_data);
    }
#endif

    /* OSR Gearbox configuration */
    RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXLBE_SER_ORDER)      , 0);
    RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXLBE_SER_INIT_VAL)   , 0);
    RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXLBE_SER_EN)         , 0);
    RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, TXFIFO_RD_LEGACY_MODE), 0);
    RU_FIELD_WRITE(0, TOP_OSR, CONTROL, CHOP(CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_, CFG_GPON_RX_CLK)      , 2);


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

    // PLL0 Configurations (if pll_use = ONE_PLL, than PLL0 is used by both RX & TX)
    //#=================================================================================================
    // AMS_COM_PLL_CONTROL_1  [0xd0b1]
    wr_serdes = (((tx_pll_vco_div4 << 7) & (0x0001 << 7)) | ((tx_pll_vco_div2 << 6) & (0x0001 << 6)));
    wr_mask   = ((                         (0x0001 << 7)) | (                         (0x0001 << 6)));
    writePONSerdesReg((DEVID_1 | tx_pll_id), AMS_COM_PLL_CONTROL_1, wr_serdes, wr_mask);

    // TX AMS_COM_PLL_CONTROL_4 [0xd0b1]
    wr_serdes = (((tx_pll_force_kvh_bw << 14) & (0x0001 << 14)) | ((tx_pll_kvh_force << 12) & (0x0003 << 12)) | ((tx_pll_2rx_bw << 8) & (0x0003 << 8)));
    wr_mask   = ((                              (0x0001 << 14)) | (                           (0x0003 << 12)) | (                       (0x0003 << 8)));
    writePONSerdesReg((DEVID_1 | tx_pll_id), AMS_COM_PLL_CONTROL_4, wr_serdes, (wr_mask));

    // TX AMS_COM_PLL_CONTROL_7 [0xd0b7]
    wr_serdes = (((tx_pll_fracn_div & 0xffff) << 0) & (0xffff << 0));
    wr_mask   = (                                     (0xffff << 0));
    writePONSerdesReg((DEVID_1 | tx_pll_id), AMS_COM_PLL_CONTROL_7, wr_serdes, (wr_mask));

    // TX AMS_COM_PLL_CONTROL_8 [0xd0b8]
    wr_serdes = (((tx_pll_fracn_sel << 15) & (0x0001 << 15)) | ((tx_pll_ditheren << 14) & (0x0001 << 14)) | ((tx_pll_fracn_ndiv << 4) & (0x03ff << 4)) | ((((tx_pll_fracn_div & 0x30000)>> 16)  << 0) & (0x0003 << 0)));
    wr_mask   = ((                           (0x0001 << 15)) | (                          (0x0001 << 14)) | (                           (0x03ff << 4)) | (                                              (0x0003 << 0)));
    writePONSerdesReg((DEVID_1 | tx_pll_id), AMS_COM_PLL_CONTROL_8, wr_serdes, wr_mask);

    // TX PLL_CAL_COM_CTL_7  [0xd127]
    wr_serdes = ((tx_pll_mode << 0) & (0x000f << 0));
    wr_mask   = (                     (0x000f << 0));
    writePONSerdesReg((DEVID_1 | tx_pll_id), PLL_CAL_COM_CTL_7, wr_serdes, wr_mask);



    if (pll_use == TWO_PLLS)
    {
        // PLL1 Configurations : (RX use PLL1 only if pll_use=TWO_PLLS)
        //#=================================================================================================
        // RX AMS_COM_PLL_CONTROL_1
        wr_serdes = (((rx_pll_vco_div4 << 7) & (0x0001 << 7)) | ((rx_pll_vco_div2 << 6) & (0x0001 << 6)));
        wr_mask   = ((                         (0x0001 << 7)) | (                         (0x0001 << 6)));
        writePONSerdesReg((DEVID_1 | rx_pll_id), AMS_COM_PLL_CONTROL_1, wr_serdes, wr_mask);

        // RX AMS_COM_PLL_CONTROL_4
        wr_serdes = (((rx_pll_force_kvh_bw << 14) & (0x0001 << 14)) | ((rx_pll_kvh_force << 12) & (0x0003 << 12)) | ((rx_pll_2rx_bw << 8) & (0x0003 << 8)));
        wr_mask   = ((                              (0x0001 << 14)) | (                           (0x0003 << 12)) | (                       (0x0003 << 8)));
        writePONSerdesReg((DEVID_1 | rx_pll_id), AMS_COM_PLL_CONTROL_4, wr_serdes, wr_mask);

        // RX AMS_COM_PLL_CONTROL_7
        wr_serdes = (((rx_pll_fracn_div & 0xffff) << 0) & (0xffff << 0));
        wr_mask   = (                                     (0xffff << 0));
        writePONSerdesReg((DEVID_1 | rx_pll_id), AMS_COM_PLL_CONTROL_7, wr_serdes, wr_mask);

        // RX AMS_COM_PLL_CONTROL_8
        wr_serdes = (((rx_pll_fracn_sel << 15) & (0x0001 << 15)) | ((rx_pll_ditheren << 14) & (0x0001 << 14)) | ((rx_pll_fracn_ndiv << 4) & (0x03ff << 4)) | ((((rx_pll_fracn_div & 0x30000) >> 16) << 0) & (0x0003 << 0)));
        wr_mask   = ((                           (0x0001 << 15)) | (                          (0x0001 << 14)) | (                           (0x03ff << 4)) | (                                              (0x0003 << 0)));
        writePONSerdesReg((DEVID_1 | rx_pll_id), AMS_COM_PLL_CONTROL_8, wr_serdes, wr_mask);

        // RX PLL_CAL_COM_CTL_7
        wr_serdes = ((rx_pll_mode << 0) & (0x000f << 0));
        wr_mask   = (                     (0x000f << 0));
        writePONSerdesReg((DEVID_1 | rx_pll_id), PLL_CAL_COM_CTL_7, wr_serdes, wr_mask);
    }


    //#===================================================================================================

    /* CKRST_CTRL_PLL_SELECT_CONTROL (0xd08d)   */
    // # [0] tx_pll_select : 0= Select PLL0 as clock for TX lane
    // # [1] rx_pll_select : 0= Select PLL0 or 1= PLL1  as clock for RX lane
    if (pll_use == TWO_PLLS)
        writePONSerdesReg(0x0800, CKRST_CTRL_PLL_SELECT_CONTROL, 0x0002, 0x0003); // PLL#1  as clock for RX lane
    else // pll== ONE_PLL
        writePONSerdesReg(0x0800, CKRST_CTRL_PLL_SELECT_CONTROL, 0x0000, 0x0003); // PLL#0 as clock for RX lane


    //(*) PLL charge pump settings
    //#===============================================================
    writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x000a, 0x001e); // pll_iqp [04:01] = 0x5 = default

    if ((pll_refclk == LCREF) && ((mode == EPON_10_10) || (mode == EPON_10_1) || (mode == AE_10_10) || (mode == AE_10_1))) // 10G mode & PLL ref.clk=50MHz
    {
#if defined (CONFIG_BCM96858) || defined (CONFIG_BCM963158) || defined (CONFIG_BCM96856)
        //  0xD0B2[bit0]=en_HRz<1> =1,  0xD0B0[bit11]=en_HRz<0>=1  -->>> en_HRz = 6[kOhm]
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x0001, 0x0001);
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_0, 0x0800, 0x0800);

        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_2, 0x0001, 0x0001);
        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_0, 0x0800, 0x0800);

        // set PLL-Current-ChargePump=ipq[bit4:1]= 0x0 -->>> 50[uA]
        writePONSerdesReg(0x0800, AMS_COM_PLL_CONTROL_2, 0x0000, 0x001e);
        writePONSerdesReg(0x0900, AMS_COM_PLL_CONTROL_2, 0x0000, 0x001e);
#endif
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
    //  # rx_osr_mode_frc     = 1
    //  # rx_osr_mode_frc_val = mode dependent
    //  # tx_osr_mode_frc     = 1
    //  # tx_osr_mode_frc_val = mode dependent
    wr_serdes = (((tx_osr_mode << 4) | (0x0001 << 15)) | (rx_osr_mode | (0x0001 << 14)));
    writePONSerdesReg(0x0800, CKRST_CTRL_OSR_MODE_CONTROL, wr_serdes, 0xffff);

    /* #(*) TX Phase Interpolator Control 0 (0xD070)    */
    writePONSerdesReg((DEVID_1 | LANE_BRDCST), TX_PI_LBE_tx_pi_control_0, 0x2003, 0xffff);

    /// #(6) CDR Control
    // *********************************************************************************
    /* Configure DSC_A_cdr_control_0 */
    writePONSerdesReg((DEVID_1 | LANE_BRDCST), DSC_A_cdr_control_0, 0x0005, 0x7ff7); // cdr_freq_en=1, cdr_integ_sat_sel=0, cdr_freq_override_en=0, cdr_phase_sat_ctrl=1

    /* Configure DSC_A_cdr_control_2 */
    if ((mode == EPON_10_10) || (mode == EPON_10_1) || (mode == AE_10_1) || (mode == AE_10_10))
        writePONSerdesReg((DEVID_1 | LANE_BRDCST), DSC_A_cdr_control_2, 0x0030, 0x1ff3); // osx2p_pherr_gain=0, phase_err_offset_mult_2=0, pattern_sel=3
    else
        writePONSerdesReg((DEVID_1 | LANE_BRDCST), DSC_A_cdr_control_2, 0x00f0, 0x1ff3); // osx2p_pherr_gain=0, phase_err_offset_mult_2=0, pattern_sel=15

    /*Configure DSC_B_dsc_sm_ctrl_7 */
    writePONSerdesReg((DEVID_1 | LANE_BRDCST), DSC_B_dsc_sm_ctrl_7, 0x0000, 0xffff); // cdr_bwsel_integ_acqcdr=0, cdr_bwsel_integ_norm=0, cdr_bwsel_prop_acqcdr=0, cdr_bwsel_prop_norm=0

    /* Configure DSC_A_cdr_control_1 */
    writePONSerdesReg((DEVID_1 | LANE_BRDCST), DSC_A_cdr_control_1, 0x0690, 0xffff); // cdr_freq_override_val=0x690

    /*Configure DSC_B_dsc_sm_ctrl_8 */
    writePONSerdesReg((DEVID_1 | LANE_BRDCST), DSC_B_dsc_sm_ctrl_8, 0x0010, 0xcfff); // phase_err_offset=0, phase_err_offset_en=0


    // (*)  Analog = VGA, and PF programming
    // *********************************************************************************
    writePONSerdesReg(0x0800, DSC_E_dsc_e_pf_ctrl, 0x0007, 0x000F); // _set_rx_pf_main(7);
    writePONSerdesReg(0x0800, DSC_E_dsc_e_pf2_lowp_ctrl, 0x0003, 0x0007); // _set_rx_pf2(3);
    writePONSerdesReg(0x0800, DSC_C_dfe_vga_override, 0x0000, 0x3E00); // _set_rx_vga(32);
    writePONSerdesReg(0x0800, DSC_C_dfe_vga_override, 0x0100, 0x01FF);
    writePONSerdesReg(0x0800, DSC_C_dfe_vga_override, 0x8000, 0x8000);


    // (#11) RX-&-TX PON_MAC_CLK Division Control and SYNC_E_CLK
    // *********************************************************************************
    // TX AMS_TX_TX_CONTROL_1  [ 0xD0A1 ]
    wr_serdes = (((tx_pon_mac_ctrl << 4) & (0x0007 << 4)) | ((tx_sync_e_ctrl << 1) & (0x0007 << 1)));
    wr_mask   = ((                         (0x0007 << 4)) | (                        (0x0007 << 1)));
    writePONSerdesReg((DEVID_1 | LANE_BRDCST), AMS_TX_TX_CONTROL_1, wr_serdes, wr_mask);

    // TX AMS_RX_RX_CONTROL_2 [ 0xD092 ]
    wr_serdes = (((rx_pon_mac_ctrl << 0) & (0x0007 << 0)));
    wr_mask   = ((                         (0x0007 << 0)));
    writePONSerdesReg((DEVID_1 | LANE_BRDCST), AMS_RX_RX_CONTROL_2, wr_serdes, wr_mask);


    // (*)   Enable PLL's.  De-assert core_dp_s_rstb --> will start the PLL calibration.
    // *********************************************************************************
    writePONSerdesReg(0x0800, CORE_PLL_COM_TOP_USER_CONTROL, 0x2000, 0x6000);   /* # [13] core_dp_s_rstb = 1 */
    if (pll_use == TWO_PLLS)
        writePONSerdesReg(0x0900, CORE_PLL_COM_TOP_USER_CONTROL, 0x2000, 0x6000);

    mdelay(3);  // wait for PLL Lock

    // (*) Verify Both PLL's are Locked
    rd_serdes = readPONSerdesReg(0x0800, PLL_CAL_COM_CTL_STATUS_0);
    __logDebug("\nPLL0_Lock=%x. ", (rd_serdes & 0x0100) >> 8);
    if (pll_use == TWO_PLLS)
    {
        rd_serdes = readPONSerdesReg(0x0900, PLL_CAL_COM_CTL_STATUS_0);
        __logDebug("  PLL1_Lock=%x. ", (rd_serdes & 0x0100) >> 8);
    }

    udelay(100);

    /* #(11) De-assert ln_dp_s_rstb */
    /* ln_dp_s_rstb = 1 = serdes data-path out of reset  (0xd081)  */
    writePONSerdesReg(0x0800, CKRST_CTRL_LANE_CLK_RESET_N_POWERDOWN_CONTROL, 0x0002, 0x0002); /* # bit[2] =ln_dp_s_rstb =1 */


    /* pmi_lp_write((DEVID_1 | LANE_BRDCST), AMS_TX_TX_CONTROL_0, 0x0000, (0x00c0))*/
    writePONSerdesReg(0x0800, AMS_TX_TX_CONTROL_0, 0x0000, 0x00c0);

    /* Ignore SigDetect:  0xd010[09] = ignore_sigdet =1 */
    writePONSerdesReg(0x0800, DSC_B_dsc_sm_ctrl_0, 0x0200, 0x0200);

#ifdef PLL_PPM_ADJ
    // (*)   PLL PPM Adjustment for RX=10G modes
    if ((mode == EPON_10_10) || (mode == AE_10_10))
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
    __logDebug("  DSC_State=0x%08x. ", (rd_serdes >> 11));

    // ####### SerDes Initialization =  Done    ###############################

#if defined(CONFIG_BCM963158)
    // Additions for BCM63158  ###
    /* AE_GEARBOX_CONTROL_0 */
    //  Bcm963158.Bcm63158.WAN_TOP.WAN_TOP_AE_GEARBOX_CONTROL_0.cr_wan_top_ae_gearbox_tx_fifo_offset        = 0
    //  Bcm963158.Bcm63158.WAN_TOP.WAN_TOP_AE_GEARBOX_CONTROL_0.cr_wan_top_ae_gearbox_tx_fifo_offset_ld     = 0
    //  Bcm963158.Bcm63158.WAN_TOP.WAN_TOP_AE_GEARBOX_CONTROL_0.cr_wan_top_ae_gearbox_full_rate_serdes_mode = 1  // 10G =  Full Rate
    //  Bcm963158.Bcm63158.WAN_TOP.WAN_TOP_AE_GEARBOX_CONTROL_0.cr_wan_top_ae_gearbox_width_mode            = 1  // 10G rate = 20b mode
    RU_FIELD_WRITE(0, AE_GEARBOX_CONTROL_0, _0, CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET       , 0);
    RU_FIELD_WRITE(0, AE_GEARBOX_CONTROL_0, _0, CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_LD    , 0);
    if ((mode == EPON_10_10) || (mode == AE_10_10))
    {
        RU_FIELD_WRITE(0, AE_GEARBOX_CONTROL_0, _0, CR_WAN_TOP_AE_GEARBOX_FULL_RATE_SERDES_MODE, 1);
        RU_FIELD_WRITE(0, AE_GEARBOX_CONTROL_0, _0, CR_WAN_TOP_AE_GEARBOX_WIDTH_MODE           , 1);
    }

    /*  WAN_TOP_RESET (PCS RESET)  */
    RU_FIELD_WRITE(0, TOP_RESET,  T, CFG_PCS_RESET_N, 0);
    RU_FIELD_WRITE(0, TOP_RESET,  T, CFG_PCS_RESET_N, 1);
    RU_FIELD_WRITE(0, TOP_RESET,  T, CFG_PCS_RESET_N, 0);
    RU_FIELD_WRITE(0, TOP_RESET,  T, CFG_PCS_RESET_N, 1);

    /* PCS Programing for 10G and other modes*/
    __logDebug("\nBEGIN Programing PCS registers ");

    if ((mode == EPON_10_10) || (mode == AE_10_10))
    {
        /* XGXSBLK0_XGXSCTRL (0x8000) */
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), XGXSBLK0_XGXSCTRL,  0x260f, 0xffff);

        /* XGXSBLK1_LANECTRL0  (0x8015)*/
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), XGXSBLK1_LANECTRL0,  0x1011, 0xffff);

        /* SerdesDigital_misc1 (0x8308)*/
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), SerdesDigital_misc1,  0x6015, 0xffff);

        /* Digital5_parDetINDControl1 (0x8347)*/
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), Digital5_parDetINDControl1,  0x5015, 0xffff);

        /* Digital5_parDetINDControl2 (0x8348) */
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), Digital5_parDetINDControl2,  0x0008, 0xffff);

        /* Digital5_Misc7 (0x8349) */
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), Digital5_Misc7,  0x0008, 0xffff);

        /* Digital5_Misc6 (0x8345) */
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), Digital5_Misc6,  0x2a00, 0xffff);

        /* If  Dn scrambler is enabled - Digital4_Misc3 (0x833C)*/
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), Digital4_Misc3,  0x8188, 0xffff);

        /* Digital4_Misc4 (0x833D) */
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), Digital4_Misc4,  0x6000, 0xffff);

        /* upstream scrambler enable - tx66_Control (0x83B0) */
        PCSwriteSerdes((PCS_ID | LANE_BRDCST), tx66_Control,  0x4041, 0xffff);

        __logDebug("\nEND Programing PCS registers for 10G ");
    }
#endif

    // ####### Final Settings, post serdes init.###############################

    /* PMD : 10G_1G_EPON additional configurations if PMD exist.*/
    //#===============================================================
    if ((mode == EPON_10_1) && (optics_type == BCM_I2C_PON_OPTICS_TYPE_PMD))
    {
        //EWAKE configuration for PMD in EPON_10g1g mode
        early_txen_txen txen;

        // WAN_TOP_WRITE_32(0x80144030, 0x032f2f2f);
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass     = 0;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity   = 1;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity  = 1;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_toff_time             = 0x2f;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_setup_time            = 0x2f;
        txen.cr_xgwan_top_wan_misc_early_txen_cfg_hold_time             = 0x2f;
        ag_drv_early_txen_txen_set(&txen);

        //invert data polarity
        writePONSerdesReg(0x800, TLB_TX_tlb_tx_misc_config, 0x0001, 0x0001);

        //For 6858XPMD board (19/9/16 - simple optimization without enabling DFE)
        writePONSerdesReg(0x0800, DSC_E_dsc_e_ctrl, 0x0080, 0x0080);
        writePONSerdesReg(0x0800, DSC_E_dsc_e_pf_ctrl, 0x0008, 0x000F);
        writePONSerdesReg(0x0800, DSC_E_dsc_e_pf2_lowp_ctrl, 0x0000, 0x0007);

    } // END if ((mode == EPON_10_1)&&(optics_type == BCM_I2C_PON_OPTICS_TYPE_PMD))
    //#===============================================================

    /* Disable/Bypass Signal-Detect */
    rx_sigdet_dis();

    /*  Enable Laser-BE signal: Laser_OE = 1   */
    RU_FIELD_WRITE(0, MISC, 3, CHOP(CR_XGWAN_TOP_WAN_MISC_WAN_CFG_, LASER_OE),  1);
    RU_REG_READ   (0, MISC, 3, rd_data);
    __logDebug("\n WAN_TOP_MISC_3 = 0x%08x", rd_data);
} // END of serdes_init() function


void pon_drv_epon_init(serdes_wan_type_t wan_type, int pll50mhz)
{
    pon_params_t *pon_params;

    pon_params = & EPON_1_1_params; //default

    switch (wan_type)
    {
        /* *****  E-PON MODES ***** */
        case SERDES_WAN_TYPE_EPON_1G:
        case SERDES_WAN_TYPE_AE:
            __logInfo("\n\nserdes initialization: EPON_1_1 Start\n");
            pon_params = & EPON_1_1_params;
            break;

        case SERDES_WAN_TYPE_AE_2_5G:
            __logInfo("\n\nserdes initialization: AE_2_2G Start\n");
            pon_params = & AE_2_2_params;
            break;

        case SERDES_WAN_TYPE_EPON_2G:
            __logInfo("\n\nserdes initialization: EPON_2_1 Start\n");
            pon_params = & EPON_2_1_params;
            break;

        case SERDES_WAN_TYPE_EPON_10G_ASYM:
            __logInfo("\n\nserdes initialization: EPON_10_1 Start\n");
            pon_params = & EPON_10_1_params;
            break;

        case SERDES_WAN_TYPE_EPON_10G_SYM:
        case SERDES_WAN_TYPE_AE_10G:
            __logInfo("\n\nserdes initialization: EPON/AE SYM(10G_10G) Start\n");
            if (!pll50mhz)
            {
                EPON_10_10_params.refclk = PADREF;  // PADREF = 156.25MHz refclk
            }

            pon_params = & EPON_10_10_params;
            break;

        default:
            __logError("\n Undefined PON type, Using EPON 1/1 as default\n");
            break;
    } // End of switch (wan_type)

    serdes_init(pon_params);

    __logInfo("\n\nserdes initialization: Done\n");
} // End of 'pon_init(serdes_wan_type_t wan_serdes_type)'


