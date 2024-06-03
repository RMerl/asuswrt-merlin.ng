// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/** @file srds_api_enum.h
 * Enum types used by Serdes API functions
   This file contains Enum's which are common to all serdes cores
 */

#ifndef SRDS_API_ENUM_H
#define SRDS_API_ENUM_H

/** PRBS Polynomial Enum */
enum srds_prbs_polynomial_enum {
  PRBS_7  = 0,
  PRBS_9  = 1,
  PRBS_11 = 2,
  PRBS_15 = 3,
  PRBS_23 = 4,
  PRBS_31 = 5,
  PRBS_58 = 6
};

#define srds_return_prbs_order(val) ((val==0)?7:  \
                                (val==1)?9:  \
                                (val==2)?11: \
                                (val==3)?15: \
                                (val==4)?23: \
                                (val==5)?31: \
                                (val==6)?58:0)

/** PRBS Checker Mode Enum */
enum srds_prbs_checker_mode_enum {
  PRBS_SELF_SYNC_HYSTERESIS       = 0,
  PRBS_INITIAL_SEED_HYSTERESIS    = 1,
  PRBS_INITIAL_SEED_NO_HYSTERESIS = 2
};


/** TX_PI Fixed Frequency and Jitter Generation Modes Enum */
enum srds_tx_pi_freq_jit_gen_enum {
  TX_PI_FIXED_FREQ      = 0,
  TX_PI_SSC_LOW_FREQ    = 1,
  TX_PI_SSC_HIGH_FREQ   = 2,
  TX_PI_SJ              = 3
};

/** RX AFE Settings Enum */
enum srds_rx_afe_settings_enum {
    RX_AFE_PF,
    RX_AFE_PF2,
    RX_AFE_VGA,
    RX_AFE_DFE1,
    RX_AFE_DFE2,
    RX_AFE_DFE3,
    RX_AFE_DFE4,
    RX_AFE_DFE5,
    RX_AFE_DFE6,
    RX_AFE_DFE7,
    RX_AFE_DFE8,
    RX_AFE_DFE9,
    RX_AFE_DFE10,
    RX_AFE_DFE11,
    RX_AFE_DFE12,
    RX_AFE_DFE13,
    RX_AFE_DFE14
};

/** TX AFE Settings Enum */
enum srds_tx_afe_settings_enum {
    TX_AFE_PRE,
    TX_AFE_MAIN,
    TX_AFE_POST1,
    TX_AFE_POST2,
    TX_AFE_POST3,
    TX_AFE_AMP,
    TX_AFE_DRIVERMODE
};

/** AFE Drivermode Enum */
enum srds_afe_drivermode_enum {
    DM_DEFAULT = 0,
    DM_HALF_AMPLITUDE_HI_IMPED,
    DM_HALF_AMPLITUDE,
    DM_NOT_SUPPORTED
};

/** Repeater Mode Enum */
enum srds_rptr_mode_enum {
    DATA_IN_SIDE,
    DIG_LPBK_SIDE,
    RMT_LPBK_SIDE
};

/** Core Powerdown Modes Enum */
enum srds_core_pwrdn_mode_enum {
    PWR_ON,
    PWRDN,
    PWRDN_DEEP,
    PWRDN_TX,
    PWRDN_RX
};

/** Evenet Log Display Mode Enum */
enum srds_event_log_display_mode_enum {
    EVENT_LOG_HEX,
    EVENT_LOG_DECODED,
    EVENT_LOG_HEX_AND_DECODED,
    EVENT_LOG_MODE_MAX
};

/** Eye Scan Direction Enum */
enum srds_eye_scan_dir_enum {
      EYE_SCAN_HORIZ = 0,
      EYE_SCAN_VERTICAL = 1
};


/* Used by #serdes_display_diag_data to select diag_level             */
/* example:                                                           */
/*  diag_level = SRDS_DIAG_LANE | SRDS_DIAG_CORE | SRDS_DIAG_EVENT    */
enum srds_diag_level_enum {
    SRDS_DIAG_LANE = 1,
    SRDS_DIAG_CORE = 1<<1,
    SRDS_DIAG_EVENT = 1<<2,
    SRDS_DIAG_EYE = 1<<3,
    SRDS_DIAG_REG_CORE = 1<<4,
    SRDS_DIAG_REG_LANE = 1<<5,
    SRDS_DIAG_UC_CORE = 1<<6,
    SRDS_DIAG_UC_LANE = 1<<7,
    SRDS_DIAG_LANE_DEBUG = 1<<8,
    SRDS_DIAG_BER_VERT = 1<<9,
    SRDS_DIAG_BER_HORZ = 1<<10
};

/* Used by #serdes_diag_access to select type of access */
enum srds_diag_access_enum {
    SRDS_REG_READ,
    SRDS_REG_RMW,
    SRDS_CORE_RAM_READ_BYTE,
    SRDS_CORE_RAM_RMW_BYTE,
    SRDS_CORE_RAM_READ_WORD,
    SRDS_CORE_RAM_RMW_WORD,
    SRDS_LANE_RAM_READ_BYTE,
    SRDS_LANE_RAM_RMW_BYTE,
    SRDS_LANE_RAM_READ_WORD,
    SRDS_LANE_RAM_RMW_WORD,
    SRDS_GLOB_RAM_READ_BYTE,
    SRDS_GLOB_RAM_RMW_BYTE,
    SRDS_GLOB_RAM_READ_WORD,
    SRDS_GLOB_RAM_RMW_WORD,
    SRDS_UC_CMD,
    SRDS_EN_BREAKPOINT,
    SRDS_GOTO_BREAKPOINT,
    SRDS_RD_BREAKPOINT,
    SRDS_DIS_BREAKPOINT,
    SRDS_BER_PROJ_DATA
};

#endif
