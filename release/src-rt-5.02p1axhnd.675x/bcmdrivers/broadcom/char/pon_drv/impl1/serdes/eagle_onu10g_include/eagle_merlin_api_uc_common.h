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


/** @file eagle_merlin_api_uc_common.h
 * Defines and Enumerations shared by Eagle & Merlin IP Specific API and Microcode
 */

#ifndef EAGLE_MERLIN_API_UC_COMMON_H
#define EAGLE_MERLIN_API_UC_COMMON_H

/* Add Eagle/Merlin/OLT/ONU specific items below this */

/** Translate between a VCO frequency in MHz and the vco_rate that is **\
*** found in the Core Config Variable Structure using the formula:    ***
***                                                                   ***
***     vco_rate = (frequency_in_ghz * 4.0) - 22.0                    ***
***                                                                   ***
*** Both functions round to the nearest resulting value.  This        ***
*** provides the highest accuracy possible, and ensures that:         ***
***                                                                   ***
***     vco_rate == MHZ_TO_VCO_RATE(VCO_RATE_TO_MHZ(vco_rate))        ***
***                                                                   ***
*** In the microcode, this should only be called with a numeric       ***
*** literal parameter.                                                ***
\**                                                                   **/
#define MHZ_TO_VCO_RATE(mhz) ((uint8_t)((((uint16_t)(mhz)) / 250) - 22))
#define VCO_RATE_TO_MHZ(vco_rate) ((((uint16_t)(vco_rate) + 22) * 1000) >> 2)

/* Please note that when adding entries here you should update the #defines in the eagle_onu10g_common.h */

/** OSR_MODES Enum */
enum eagle_onu10g_osr_mode_enum {
  EAGLE_ONU10G_OSX1    = 0,
  EAGLE_ONU10G_OSX2    = 1,
  EAGLE_ONU10G_OSX3    = 2,
  EAGLE_ONU10G_OSX3P3  = 3,
  EAGLE_ONU10G_OSX4    = 4,
  EAGLE_ONU10G_OSX5    = 5,
  EAGLE_ONU10G_OSX7P5  = 6,
  EAGLE_ONU10G_OSX8    = 7,
  EAGLE_ONU10G_OSX8P25 = 8,
  EAGLE_ONU10G_OSX10   = 9
};


/** VCO_RATE Enum */
enum eagle_onu10g_vco_rate_enum {
  EAGLE_ONU10G_VCO_5P5G   = MHZ_TO_VCO_RATE( 5500),
  EAGLE_ONU10G_VCO_5P75G  = MHZ_TO_VCO_RATE( 5750),
  EAGLE_ONU10G_VCO_6G     = MHZ_TO_VCO_RATE( 6000),
  EAGLE_ONU10G_VCO_6P25G  = MHZ_TO_VCO_RATE( 6250),
  EAGLE_ONU10G_VCO_6P5G   = MHZ_TO_VCO_RATE( 6500),
  EAGLE_ONU10G_VCO_6P75G  = MHZ_TO_VCO_RATE( 6750),
  EAGLE_ONU10G_VCO_7G     = MHZ_TO_VCO_RATE( 7000),
  EAGLE_ONU10G_VCO_7P25G  = MHZ_TO_VCO_RATE( 7250),
  EAGLE_ONU10G_VCO_7P5G   = MHZ_TO_VCO_RATE( 7500),
  EAGLE_ONU10G_VCO_7P75G  = MHZ_TO_VCO_RATE( 7750),
  EAGLE_ONU10G_VCO_8G     = MHZ_TO_VCO_RATE( 8000),
  EAGLE_ONU10G_VCO_8P25G  = MHZ_TO_VCO_RATE( 8250),
  EAGLE_ONU10G_VCO_8P5G   = MHZ_TO_VCO_RATE( 8500),
  EAGLE_ONU10G_VCO_8P75G  = MHZ_TO_VCO_RATE( 8750),
  EAGLE_ONU10G_VCO_9G     = MHZ_TO_VCO_RATE( 9000),
  EAGLE_ONU10G_VCO_9P25G  = MHZ_TO_VCO_RATE( 9250),
  EAGLE_ONU10G_VCO_9P5G   = MHZ_TO_VCO_RATE( 9500),
  EAGLE_ONU10G_VCO_9P75G  = MHZ_TO_VCO_RATE( 9750),
  EAGLE_ONU10G_VCO_10G    = MHZ_TO_VCO_RATE(10000),
  EAGLE_ONU10G_VCO_10P25G = MHZ_TO_VCO_RATE(10250),
  EAGLE_ONU10G_VCO_10P5G  = MHZ_TO_VCO_RATE(10500),
  EAGLE_ONU10G_VCO_10P75G = MHZ_TO_VCO_RATE(10750),
  EAGLE_ONU10G_VCO_11G    = MHZ_TO_VCO_RATE(11000),
  EAGLE_ONU10G_VCO_11P25G = MHZ_TO_VCO_RATE(11250),
  EAGLE_ONU10G_VCO_11P5G  = MHZ_TO_VCO_RATE(11500),
  EAGLE_ONU10G_VCO_11P75G = MHZ_TO_VCO_RATE(11750),
  EAGLE_ONU10G_VCO_12G    = MHZ_TO_VCO_RATE(12000),
  EAGLE_ONU10G_VCO_12P25G = MHZ_TO_VCO_RATE(12250),
  EAGLE_ONU10G_VCO_12P5G  = MHZ_TO_VCO_RATE(12500),
  EAGLE_ONU10G_VCO_12P75G = MHZ_TO_VCO_RATE(12750),
  EAGLE_ONU10G_VCO_13G    = MHZ_TO_VCO_RATE(13000),
  EAGLE_ONU10G_VCO_13P25G = MHZ_TO_VCO_RATE(13250)
};

#endif
