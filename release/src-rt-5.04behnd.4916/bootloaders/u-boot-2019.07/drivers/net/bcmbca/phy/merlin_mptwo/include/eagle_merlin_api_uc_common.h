// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
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

/* Please note that when adding entries here you should update the #defines in the merlin_mptwo_common.h */

/** OSR_MODES Enum */
enum merlin_mptwo_osr_mode_enum {
  MERLIN_MPTWO_OSX1    = 0,
  MERLIN_MPTWO_OSX2    = 1,
  MERLIN_MPTWO_OSX3    = 2,
  MERLIN_MPTWO_OSX3P3  = 3,
  MERLIN_MPTWO_OSX4    = 4,
  MERLIN_MPTWO_OSX5    = 5,
  MERLIN_MPTWO_OSX7P5  = 6,
  MERLIN_MPTWO_OSX8    = 7,
  MERLIN_MPTWO_OSX8P25 = 8,
  MERLIN_MPTWO_OSX10   = 9
};


/** VCO_RATE Enum */
enum merlin_mptwo_vco_rate_enum {
  MERLIN_MPTWO_VCO_5P5G   = MHZ_TO_VCO_RATE( 5500),
  MERLIN_MPTWO_VCO_5P75G  = MHZ_TO_VCO_RATE( 5750),
  MERLIN_MPTWO_VCO_6G     = MHZ_TO_VCO_RATE( 6000),
  MERLIN_MPTWO_VCO_6P25G  = MHZ_TO_VCO_RATE( 6250),
  MERLIN_MPTWO_VCO_6P5G   = MHZ_TO_VCO_RATE( 6500),
  MERLIN_MPTWO_VCO_6P75G  = MHZ_TO_VCO_RATE( 6750),
  MERLIN_MPTWO_VCO_7G     = MHZ_TO_VCO_RATE( 7000),
  MERLIN_MPTWO_VCO_7P25G  = MHZ_TO_VCO_RATE( 7250),
  MERLIN_MPTWO_VCO_7P5G   = MHZ_TO_VCO_RATE( 7500),
  MERLIN_MPTWO_VCO_7P75G  = MHZ_TO_VCO_RATE( 7750),
  MERLIN_MPTWO_VCO_8G     = MHZ_TO_VCO_RATE( 8000),
  MERLIN_MPTWO_VCO_8P25G  = MHZ_TO_VCO_RATE( 8250),
  MERLIN_MPTWO_VCO_8P5G   = MHZ_TO_VCO_RATE( 8500),
  MERLIN_MPTWO_VCO_8P75G  = MHZ_TO_VCO_RATE( 8750),
  MERLIN_MPTWO_VCO_9G     = MHZ_TO_VCO_RATE( 9000),
  MERLIN_MPTWO_VCO_9P25G  = MHZ_TO_VCO_RATE( 9250),
  MERLIN_MPTWO_VCO_9P5G   = MHZ_TO_VCO_RATE( 9500),
  MERLIN_MPTWO_VCO_9P75G  = MHZ_TO_VCO_RATE( 9750),
  MERLIN_MPTWO_VCO_10G    = MHZ_TO_VCO_RATE(10000),
  MERLIN_MPTWO_VCO_10P25G = MHZ_TO_VCO_RATE(10250),
  MERLIN_MPTWO_VCO_10P5G  = MHZ_TO_VCO_RATE(10500),
  MERLIN_MPTWO_VCO_10P75G = MHZ_TO_VCO_RATE(10750),
  MERLIN_MPTWO_VCO_11G    = MHZ_TO_VCO_RATE(11000),
  MERLIN_MPTWO_VCO_11P25G = MHZ_TO_VCO_RATE(11250),
  MERLIN_MPTWO_VCO_11P5G  = MHZ_TO_VCO_RATE(11500),
  MERLIN_MPTWO_VCO_11P75G = MHZ_TO_VCO_RATE(11750),
  MERLIN_MPTWO_VCO_12G    = MHZ_TO_VCO_RATE(12000),
  MERLIN_MPTWO_VCO_12P25G = MHZ_TO_VCO_RATE(12250),
  MERLIN_MPTWO_VCO_12P5G  = MHZ_TO_VCO_RATE(12500),
  MERLIN_MPTWO_VCO_12P75G = MHZ_TO_VCO_RATE(12750),
  MERLIN_MPTWO_VCO_13G    = MHZ_TO_VCO_RATE(13000),
  MERLIN_MPTWO_VCO_13P25G = MHZ_TO_VCO_RATE(13250)
};

#endif
