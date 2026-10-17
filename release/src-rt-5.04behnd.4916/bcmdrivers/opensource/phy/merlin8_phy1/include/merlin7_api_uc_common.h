/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

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


/** @file merlin7_api_uc_common.h
 * Defines and Enumerations shared by Merlin7 IP Specific API and Microcode
 */

#ifndef MERLIN7_API_UC_COMMON_H
#define MERLIN7_API_UC_COMMON_H

/* Add Merlin7 specific items below this */ 
#define MERLIN8_PHY1_LANE_TIMER_SHIFT (3)
/* Please note that when adding entries here you should update the #defines in the merlin8_phy1_common.h */

#ifdef PCIEG3_V1

/** DSC_STATES Enum **/
enum merlin8_phy1_dsc_state_enum {
  DSC_STATE_UNKNOWN        = 255,
  DSC_STATE_INIT           = 0,
  DSC_STATE_PAUSE          = 1,
  DSC_STATE_SLICER_OFFSET  = 2,
  DSC_STATE_ACQ_CDR        = 3,
  DSC_STATE_CDR_SETTLE     = 4,
  DSC_STATE_HW_TUNE        = 5,
  DSC_STATE_UC_TUNE        = 6,
  DSC_STATE_MEASURE        = 7,
  DSC_STATE_DONE           = 8,
  DSC_STATE_FREEZE         = 9,
  DSC_STATE_FACQ_WAIT      = 10,
  DSC_STATE_L0sL1_ACQ_CDR  = 11,
  DSC_STATE_L0sL1_CDR_STTL = 12,
  DSC_STATE_L0sL1_HW_TUNE  = 13,
  DSC_STATE_FACQ_DONE      = 14,
  DSC_STATE_DC_OFFSET      = 15,
  DSC_STATE_ADJ_PI         = 16
};

/** PCIE_TUNING_STATES Enum **/
enum pcie_tuning_stage_enum {
  TUNING_STAGE_LINKUP      = 0,
  TUNING_STAGE_EVAL        = 1,
  TUNING_STAGE_PHASE2      = 2,
  TUNING_EQ_EVAL_DONE      = 3,
  TUNING_EQ_REQ_DONE       = 4, 
  TUNING_STAGE_VGA         = 5
};

#else /* else !MERLIN7_ PCIEG3 */

/** DSC_STATES Enum **/
/** These state values must match the dsc_state bitfield in the register map. **/
/** If these values change, then consider changing the test functions below.  **/
enum srds_dsc_state_enum {
  DSC_STATE_UNKNOWN      = 255,
  DSC_STATE_RESET        = 0,
  DSC_STATE_RESTART      = 1,
  DSC_STATE_CONFIG       = 2,
  DSC_STATE_WAIT_FOR_SIG = 3,
  DSC_STATE_ACQ_CDR      = 4,
  DSC_STATE_CDR_SETTLE   = 5,
  DSC_STATE_HW_TUNE      = 6,
  DSC_STATE_UC_TUNE      = 7,
  DSC_STATE_MEASURE      = 8,
  DSC_STATE_DONE         = 9
};

#endif /* end MERLIN7_ PCIEG3 */

#ifdef PCIEG3_V1

/* Merlin specific DSC states */
#define DSC_STATE_SLICER_CAL 17

/** Is the state between DSC_STATE_RESET and DSC_STATE_WAIT_FOR_SIG? 
 ** For Merlin, DSC_STATE_SLICER_CAL is in that sequence.            */
#define is_dsc_state_at_or_before_wait_for_sig(state) ((state <= DSC_STATE_WAIT_FOR_SIG) \
                                                       || (state == DSC_STATE_SLICER_CAL))
#else  /* !PCIEG3 */

/** Is the state between DSC_STATE_RESET and DSC_STATE_WAIT_FOR_SIG? **/
#define is_dsc_state_at_or_before_wait_for_sig(state) (state <= DSC_STATE_WAIT_FOR_SIG)

#endif  /* end PCIEG3 */


#define EYE_SCAN_NRZ_VERTICAL_IDX_MAX   (31)
#define EYE_SCAN_NRZ_VERTICAL_STEP       (1)

/** OSR_MODES Enum */
/*  NOTE:  If adding or removing ANY of these you MUST change the initializer below. */
enum merlin8_phy1_osr_mode_enum {
  MERLIN8_PHY1_OSX1    = 0,
  MERLIN8_PHY1_OSX2    = 1,
  MERLIN8_PHY1_OSX3    = 2,
  MERLIN8_PHY1_OSX3P3  = 3,
  MERLIN8_PHY1_OSX4    = 4,
  MERLIN8_PHY1_OSX5    = 5,
  MERLIN8_PHY1_OSX7P5  = 6,
  MERLIN8_PHY1_OSX8    = 7,
  MERLIN8_PHY1_OSX8P25 = 8,
  MERLIN8_PHY1_OSX10   = 9,
  MERLIN8_PHY1_OSR_UNINITIALIZED = 0x3f
};

/* UNIQUIFY_PUBLIC_END  - Marker used by API Uniquify script */

/*This is an initializer to sort the OSR modes in order so that EVAL can walk*/
/*through them proper and choose them in order. */
#define INIT_OSR_MODE_ENUM {MERLIN8_PHY1_OSX1, MERLIN8_PHY1_OSX2, MERLIN8_PHY1_OSX3, \
                            MERLIN8_PHY1_OSX3P3, MERLIN8_PHY1_OSX4, MERLIN8_PHY1_OSX5, \
                            MERLIN8_PHY1_OSX7P5, MERLIN8_PHY1_OSX8, MERLIN8_PHY1_OSX8P25, \
                            MERLIN8_PHY1_OSX10, 0xff}

#define INIT_OSR_MODE_INT {1000, 2000, 3000, 3300, 4000, 5000, 7500, 8000, 8250, 10000}

#ifdef PCIEG3_V1
/** LANE_RATE Enum **/
enum merlin8_phy1_rate_enum {
  MERLIN8_PHY1_GEN1 = 0,
  MERLIN8_PHY1_GEN2 = 1,
  MERLIN8_PHY1_GEN3 = 2
};
#endif

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

/*
 * Peaking Filter Boundaries
 */
#define PF_MIN_VALUE          ( 0)
#define PF_MAX_VALUE          (15)

/** Minimum and maximum values for rx_vga_ctrl_val */
/*  These are hardware limits for VGA. The tuning limits are defined separately in ucode */
#define RX_VGA_CTRL_VAL_MIN (0)
#define RX_VGA_CTRL_VAL_MAX (31)

/*******************************************************************************
 * PVTMON code generated by script: common/tools/gen_die_temp_idx_constants.py *
 *******************************************************************************/
/* BEGIN_GENERATED_TEMPERATURE_CODE */

/*
 * The formula for PVTMON is:
 *
 *     T = 356.01000 - 0.23751 * reg_bin
 */
#define _bin_to_degC_double(bin_) (356.01000 - (0.23751 * (USR_DOUBLE)(bin_)))


/* Identify the temperature from the PVTMON output. */
#define _bin_to_degC(bin_) (((((int32_t)(  746607084L) +           \
                               ((int32_t)(bin_) * (    -498095L))) \
                              >> 20) + 1) >> 1)

/* Identify the PVTMON output corresponding to the temperature. */
#define _degC_to_bin(degc_) (((((int32_t)(  196467276L) +           \
                                ((int32_t)(degc_) * (    -551859L))) \
                               >> 16) + 1) >> 1)

/* END_GENERATED_TEMPERATURE_CODE */
#endif   


