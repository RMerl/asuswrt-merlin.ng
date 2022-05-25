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

#ifndef MERLIN16_API_UC_COMMON_H
#define MERLIN16_API_UC_COMMON_H

/* Add Merlin16 specific items below this */ 

/* Please note that when adding entries here you should update the #defines in the merlin16_shortfin_common.h */

/** OSR_MODES Enum */
enum merlin16_shortfin_osr_mode_enum {
  MERLIN16_SHORTFIN_OSX1    = 0,
  MERLIN16_SHORTFIN_OSX2    = 1,
  MERLIN16_SHORTFIN_OSX3    = 2,
  MERLIN16_SHORTFIN_OSX3P3  = 3,
  MERLIN16_SHORTFIN_OSX4    = 4,
  MERLIN16_SHORTFIN_OSX5    = 5,
  MERLIN16_SHORTFIN_OSX7P5  = 6,
  MERLIN16_SHORTFIN_OSX8    = 7,
  MERLIN16_SHORTFIN_OSX8P25 = 8,
  MERLIN16_SHORTFIN_OSX10   = 9
};


/* The following functions translate between a VCO frequency in MHz and the
 * vco_rate that is found in the Core Config Variable Structure using the
 * formula:
 *
 *     vco_rate = (frequency_in_ghz * 4.0) - 22.0
 *
 * Both functions round to the nearest resulting value.  This
 * provides the highest accuracy possible, and ensures that:
 *
 *     vco_rate == MHZ_TO_VCO_RATE(VCO_RATE_TO_MHZ(vco_rate))
 *
 * In the microcode, these functions should only be called with a numeric
 * literal parameter.
 */
#define MHZ_TO_VCO_RATE(mhz) ((uint8_t)((((uint16_t)(mhz) + 125) / 250) - 22))
#define VCO_RATE_TO_MHZ(vco_rate) (((uint16_t)(vco_rate) + 22) * 250)


/* BOUNDARIES FOR FIR TAP VALUES
 *
 * Sources:  M16_TXFIR document (Section 1.1.2)
 *
 *   The following constraints are enforced by CL72 SW    
 *     pre + main + post1 + post2   <= TXFIR_SUM_LIMIT
 *     main - (pre + post1 + post2) >= TXFIR_V2_LIMIT
 *     pre + post                   <= TXFIR_PRE_POST_SUM_LIMIT
 *     main                         >= TXFIR_MAIN_CL72_MIN
 */
#define TXFIR_PRE_MIN   ( 0)
#define TXFIR_PRE_MAX   (10)
#define TXFIR_MAIN_MIN  ( 0)
#define TXFIR_MAIN_MAX  (60)
#define TXFIR_POST1_MIN ( 0)
#define TXFIR_POST1_MAX (24)
#define TXFIR_POST2_MIN ( 0)
#define TXFIR_POST2_MAX ( 6)

#define TXFIR_SUM_LIMIT (60)
#define TXFIR_V2_LIMIT  ( 6)
#define TXFIR_PRE_POST_SUM_LIMIT (24)
#define TXFIR_MAIN_CL72_MIN      (33)

/** INIT values for FIR taps */
#define TXFIR_PRE_INIT   ( 4)
#define TXFIR_MAIN_INIT  (40)
#define TXFIR_POST1_INIT (16)

/** Minimum and maximum values for rx_vga_ctrl_val */
/*  These are hardware limits for VGA. The tuning limits are defined separately in ucode */
#define RX_VGA_CTRL_VAL_MIN (0)
#define RX_VGA_CTRL_VAL_MAX (31)

/* BEGIN_GENERATED_TEMPERATURE_CODE */
/*
 * The formula for PVTMON is:
 *
 *     T = 434.10000 - 0.54619 * reg10bit
 */
#define _bin_to_degC_double(bin_) (434.10000 - (0.54619 * (USR_DOUBLE)(bin_)))


/* Identify the temperature from the PVTMON output. */
#define _bin_to_degC(bin_) (((((int32_t)(  910373683L) +           \
                               ((int32_t)(bin_) * (   -1145443L))) \
                              >> 20) + 1) >> 1)

/* Identify the PVTMON output corresponding to the temperature. */
#define _degC_to_bin(degc_) (((((int32_t)(  416692764L) +           \
                                ((int32_t)(degc_) * (    -959900L))) \
                               >> 18) + 1) >> 1)
/* END_GENERATED_TEMPERATURE_CODE */

#endif   
