/*
  <:copyright-BRCM:2015:proprietary:standard
  
     Copyright (c) 2015 Broadcom 
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


/** @file
 * Eagle/ONU10G PLL Configuration
 */

#include "eagle_onu10g_enum.h"

err_code_t eagle_onu10g_configure_pll(enum eagle_onu10g_pll_enum pll_cfg) {

    /* KVH    VCO Range     */
    /* 00        N/A        */
    /* 01  10.3125 - 11.25  */
    /* 10   9.375  - 10.0   */
    /* 11   8.0    - 8.4375 */

    /*  Use this to restore defaults if reprogramming the PLL under dp-reset (typically Auto-Neg FW) */
    /*  EFUN(wrc_pll_mode              (   0xA)); */
    /*  EFUN(wrc_ams_pll_fracn_ndiv_int(   0x0)); */
    /*  EFUN(wrc_ams_pll_fracn_div_h   (   0x0)); */
    /*  EFUN(wrc_ams_pll_fracn_div_l   (   0x0)); */
    /*  EFUN(wrc_ams_pll_fracn_bypass  (   0x0)); */
    /*  EFUN(wrc_ams_pll_fracn_divrange(   0x0)); */
    /*  EFUN(wrc_ams_pll_fracn_sel     (   0x0)); */
    /*  EFUN(wrc_ams_pll_ditheren      (   0x0)); */
    /*  EFUN(wrc_ams_pll_force_kvh_bw  (   0x0)); */
    /*  EFUN(wrc_ams_pll_kvh_force     (   0x0)); */
    /*  EFUN(wrc_ams_pll_2rx_clkbw     (   0x0)); */
    /*  EFUN(wrc_ams_pll_vco_div2      (   0x0)); */
    /*  EFUN(wrc_ams_pll_vco_div4      (   0x0)); */
    /*  EFUN(wrc_ams_pll_refclk_doubler(   0x0)); */

    /* Use core_s_rstb to re-initialize all registers to default before calling this function. */
#ifndef ATE_LOG
    uint8_t reset_state;

    ESTM(reset_state = rdc_core_dp_reset_state());
    if(reset_state < 7) {
        EFUN_PRINTF(("ERROR: eagle_onu10g_configure_pll(..) called without core_dp_s_rstb=0\n"));
        return (_error(ERR_CODE_CORE_DP_NOT_RESET));
    }
#endif
    switch (pll_cfg) {
    /******************/
    /*  Integer Mode  */
    /******************/
   
    case EAGLE_ONU10G_pll_div_50x:      /* VCO 2.5G,   REF 50.0 MHz */
	EFUN(wrc_pll_mode              (   0x5));
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x1));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_kvh_force     (   0x2));
        break;

    case EAGLE_ONU10G_pll_div_32x:      /* VCO 2.5G,   REF 78.125 MHz */
	EFUN(wrc_pll_mode              (   0x7));
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x1));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_kvh_force     (   0x2));
        break;
   

    /*****************/
    /*  Frac-N Mode  */
    /*****************/

    case EAGLE_ONU10G_pll_div_199p06x:         /* DIV  199.066, VCO 2.488G,  REF 50.0 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0xC7));
        EFUN(wrc_ams_pll_fracn_div_l   (0x432C));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x0));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x1));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x2));
        break;

    case EAGLE_ONU10G_pll_div_248p83x:         /* DIV  248.832, VCO 3.1104G,  REF 50.0 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0xF8));
        EFUN(wrc_ams_pll_fracn_div_l   (0x53F7));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x3));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x1));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x1));  
        break;

    case EAGLE_ONU10G_pll_div_250p0x:          /* DIV  250.0,   VCO 3.125G,  REF 50.0 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0xFA));
        EFUN(wrc_ams_pll_fracn_div_l   (0x0000));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x0));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x1));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x1));       
        break;

    case EAGLE_ONU10G_pll_div_199p06x_vco9p9:  /* DIV  199.066, VCO 9.9532G,  REF 50.0 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0xC7));
        EFUN(wrc_ams_pll_fracn_div_l   (0x432C));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x0));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x0));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x2));
        break;	
   
    case EAGLE_ONU10G_pll_div_200p0x:          /* DIV  200.0 ,  VCO 10.0G,  REF 50.0 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0xC8));
        EFUN(wrc_ams_pll_fracn_div_l   (0x0000));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x0));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x0));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x2));    
        break;	

    case EAGLE_ONU10G_pll_div_206p25x:         /* DIV  206.25,  VCO 10.3125G,  REF 50.0 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0xCE));
        EFUN(wrc_ams_pll_fracn_div_l   (0x0000));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x1));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x0));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x1));    
        break;
 
    case EAGLE_ONU10G_pll_div_248p83_vco12p4x: /* DIV  248.832, VCO 12.44G,  REF 50.0 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0xF8));
        EFUN(wrc_ams_pll_fracn_div_l   (0x53F7));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x3));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x0));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x1)); 
        break;
     
    case EAGLE_ONU10G_pll_div_64p0x:           /* DIV  64.0 ,  VCO 2.488G,  REF 155.52 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0x40));
        EFUN(wrc_ams_pll_fracn_div_l   (0x0000));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x0));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x1));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x2));    
        break;	
     
    case EAGLE_ONU10G_pll_div_80p0x:           /* DIV  80.0 ,  VCO 3.1104G,  REF 155.52 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0x50));
        EFUN(wrc_ams_pll_fracn_div_l   (0x0000));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x0));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x1));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x1));    
        break;	

    case EAGLE_ONU10G_pll_div_64p0_vco9p9x:    /* DIV  64.0 ,  VCO 9.9532G,  REF 155.52 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0x40));
        EFUN(wrc_ams_pll_fracn_div_l   (0x0000));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x0));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x0));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x2));    
        break;	
     
    case EAGLE_ONU10G_pll_div_80p0_vco12p4x:   /* DIV  80.0 ,  VCO 12.44G,  REF 155.52 MHz */
	EFUN(wrc_pll_mode              (   0x5));      /* keeping pll_mode at default value for fracn mode */
        EFUN(wrc_ams_pll_fracn_ndiv_int(  0x50));
        EFUN(wrc_ams_pll_fracn_div_l   (0x0000));
        EFUN(wrc_ams_pll_fracn_div_h   (   0x0));
        EFUN(wrc_ams_pll_fracn_bypass  (   0x0));
        EFUN(wrc_ams_pll_fracn_divrange(   0x0));
        EFUN(wrc_ams_pll_ditheren      (   0x1));	
	EFUN(wrc_ams_pll_force_kvh_bw  (   0x1));
        EFUN(wrc_ams_pll_vco_div2      (   0x0));
        EFUN(wrc_ams_pll_vco_div4      (   0x0));
	EFUN(wrc_ams_pll_refclk_doubler(   0x0));
        EFUN(wrc_ams_pll_fracn_sel     (   0x1)); 
        EFUN(wrc_ams_pll_kvh_force     (   0x1));    
        break;

    /*******************************/
    /*  Invalid 'pll_cfg' Selector */
    /*******************************/
    default:                     /* Invalid pll_cfg value  */
        return _error(ERR_CODE_INVALID_PLL_CFG);
        break;
    }  /* switch (pll_cfg) */
    return ERR_CODE_NONE;
}   /* eagle_onu10g_configure_pll */	
