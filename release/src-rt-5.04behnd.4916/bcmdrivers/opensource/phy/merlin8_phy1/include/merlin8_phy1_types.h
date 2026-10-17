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


/*****************************************************************************************
 *****************************************************************************************
 *                                                                                       *
 *  Revision      :   *
 *                                                                                       *
 *  Description   :  Common types used by Serdes API functions                           *
 *                                                                                       *
 *****************************************************************************************
 *****************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MERLIN8_PHY1_API_TYPES_H
#define MERLIN8_PHY1_API_TYPES_H

#include "merlin8_phy1_ipconfig.h"
#include "merlin8_phy1_common.h"
#include "merlin8_phy1_select_defns.h"

/*! @file
 *  @brief Common types used by Serdes API functions
 */

/*! @addtogroup APITag
 * @{
 */

/*! @defgroup SerdesAPITypesTag Common Core Types
 * Contains structs and typedefs which are used throughout
 * Serdes APIs.
 */

/*! @addtogroup SerdesAPITypesTag
 * @{
 */

/*! Used for error reporting APIs
 *
 */
typedef struct merlin8_phy1_triage_info_st {
   uint32_t     api_ver;
   uint32_t     ucode_ver;
   uint8_t      core;
   uint8_t      lane;

   err_code_t   error;
   uint16_t     line;

   uint16_t     stop_status;
   uint16_t     sw_exception;
   uint16_t     hw_exception;
   uint16_t     stack_overflow;
   uint16_t     overflow_lane_id;
   uint16_t     cmd_info;

   uint8_t      pmd_lock;
   uint8_t      pmd_lock_chg;
   uint8_t      sig_det;
   uint8_t      sig_det_chg;
   uint16_t     dsc_one_hot[2];
} merlin8_phy1_triage_info;


#define SIZE_OF_DBGFB_STATS_STRUCT 296  /* Change the size if merlin8_phy1_dbgfb_stats_st size changes */
/*! Used for maintaining correctness of SIZE_OF_DBGFB_STATS_STRUCT
 *
 */
typedef struct {
    uint8_t dbgfb_tmp[sizeof(merlin8_phy1_dbgfb_stats_st)==SIZE_OF_DBGFB_STATS_STRUCT ? 1: -1];
} merlin8_phy1_dbgfb_stats_st_size_check;

/*! Used for debug feedback APIs
 *
 */
typedef struct merlin8_phy1_dbgfb_cfg_s {
    uint8_t y;
    uint8_t x;
    uint32_t time_in_us;
    int32_t data1;
    int32_t data2;
    int32_t data3;
} merlin8_phy1_dbgfb_cfg_st;

#define DBGFB_CFG_ST_INIT {0,0,0,0,0,0}

/*----------------------------------------*/
/*  Lane/Core structs (without bitfields) */
/*----------------------------------------*/

/*! Lane Config Variable Structure in Microcode
 *
 */
struct merlin8_phy1_uc_lane_config_field_st {
  uint8_t  lane_cfg_from_pcs ;
  uint8_t  an_enabled        ;
  uint8_t  dfe_on            ;
  uint8_t  force_brdfe_on    ;
  uint8_t  media_type        ;
  uint8_t  unreliable_los    ;
  uint8_t  scrambling_dis    ;
  uint8_t  cl72_auto_polarity_en ;
  uint8_t  cl72_restart_timeout_en;
  uint16_t reserved;
  uint16_t afe_char_mode_enable;
  uint16_t channel_est_mode_enable;
};
#define UC_LANE_CONFIG_FIELD_INIT {0,0,0,0,0,0,0,0,0,0,0,0}

/*! Core Config Variable Structure in Microcode
 *
 */
struct merlin8_phy1_uc_core_config_field_st {
  uint8_t  core_cfg_from_pcs ;
  uint8_t  vco_rate          ;
  uint8_t  an_los_workaround ;
  uint8_t  reserved1         ;
  uint8_t  reserved2         ;
};
#define UC_CORE_CONFIG_FIELD_INIT {0,0,0,0,0}

/*! Lane Config Struct
 *
 */
struct  merlin8_phy1_uc_lane_config_st {
  struct merlin8_phy1_uc_lane_config_field_st field;
  uint16_t word;
};
#define UC_LANE_CONFIG_INIT {UC_LANE_CONFIG_FIELD_INIT, 0}

/*! Core Config Struct
 *
 */
struct  merlin8_phy1_uc_core_config_st {
  struct merlin8_phy1_uc_core_config_field_st field;
  uint16_t word;
  int32_t vco_rate_in_Mhz; /*!< if > 0 then will get converted and replace field.vco_rate when update is called */
};
#define UC_CORE_CONFIG_INIT {UC_CORE_CONFIG_FIELD_INIT, 0, 0}

/*! Lane User Control Function Structure in Microcode
 *
 */
struct merlin8_phy1_usr_ctrl_func_st {
  uint8_t pf_adaptation            ; /*!< Peaking Filter Adaptation */
  uint8_t dc_adaptation            ; /*!< DC Offset Adaptation */
  uint8_t vga_adaptation           ; /*!< Variable Gain Amplifier adaptation */
  uint8_t slicer_offset_tuning     ; /*!< Slicer Offset tuning */
  uint8_t clk90_offset_adaptation  ; /*!< Clk90 Offset adaptation */
  uint8_t p1_level_tuning          ; /*!< P1 Level tuning */
  uint8_t eye_adaptation           ; /*!< Eye adaptation */
  uint8_t all_adaptation           ; /*!< All adaptation */
};

/*! Lane User DFE Control Function Structure in Microcode
 *
 */
struct merlin8_phy1_usr_ctrl_dfe_func_st {
  uint8_t dfe_tap1_adaptation      ; /*!< DFE tap 1 adaptation */
  uint8_t dfe_tap2_adaptation      ; /*!< DFE tap 2 adaptation */
  uint8_t dfe_tap3_adaptation      ; /*!< DFE tap 3 adaptation */
  uint8_t dfe_tap4_adaptation      ; /*!< DFE tap 4 adaptation */
  uint8_t dfe_tap5_adaptation      ; /*!< DFE tap 5 adaptation */
  uint8_t dfe_tap1_dcd             ; /*!< DFE tap 1 Duty Cycle Distortion */
  uint8_t dfe_tap2_dcd             ; /*!< DFE tap 2 Duty Cycle Distortion */
};

/*! Lane User Control Disable Function Struct
 *
 */
struct merlin8_phy1_usr_ctrl_disable_functions_st {
  struct merlin8_phy1_usr_ctrl_func_st field;
  uint8_t  byte;
};

/*! Lane User Control Disable DFE Function Struct
 *
 */
struct merlin8_phy1_usr_ctrl_disable_dfe_functions_st {
  struct merlin8_phy1_usr_ctrl_dfe_func_st field;
  uint8_t  byte;
};

/*! Used in BER APIs
 *
 */
struct merlin8_phy1_ber_data_st {
    uint64_t num_errs;
    uint64_t num_bits;
    uint8_t lcklost;
    uint8_t prbs_chk_en;
    uint8_t cdrlcklost;
    uint8_t prbs_lck_state;
};


/*! Used for checking platform-specific padding
 *
 */
struct merlin8_phy1_check_platform_info_st {
    char c;
    uint32_t d;
};

#define BER_DATA_ST_INIT {0, 0, 0, 0, 0, 0}

/*! @def NUM_ILV
 * The number of interleaves per lane
 */
#define NUM_ILV 2

#define DATA_ARR_SIZE (6)
#define PHASE_ARR_SIZE (3)

#define DFE_ARR_SIZE (26)

#ifndef PCIEG3_V1
/*! Merlin7 Lane status Struct
 *  Please ensure that decreasing order of size is maintained while adding new elements to this structure
 */
struct merlin8_phy1_detailed_lane_status_st {
    /* =========== 32-bit variables start =========== */
    /** PRBS Error Count */
    uint32_t prbs_chk_errcnt;
    /* =========== 32-bit variables end ============= */
    /* =========== 16-bit variables start =========== */
    /** One Hot DSC State */
    uint16_t dsc_sm[2];
    /** Horizontal left eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
    uint16_t heye_left;
    /** Horizontal right eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
    uint16_t heye_right;
    /** Vertical upper eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
    uint16_t veye_upper;
    /** Vertical lower eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV */
    uint16_t veye_lower;
    /** Link time in milliseconds */
    uint16_t link_time;
    /** Frequency offset of local reference clock with respect to RX data in ppm */
    int16_t ppm;
    /** Estimate for Main tap */
    int16_t main_tap_est;
    /** RX PI counter value (Phase Error) for data slicer */
    uint16_t rx_pi_cnt_perr;
    /** uC lane configuration */
    uint16_t lane_config_word;
    /** Vertical threshold voltage of p1 slicer (mV) */
    int16_t p1_lvl;
    /** Vertical threshold voltage of m1 slicer (mV) [Used to read out 'channel loss hint' in PAM4 mode] */
    int16_t m1_lvl;
    /** Frequency offset of local reference clock with respect to TX data in ppm */
    int16_t tx_ppm;
    /* =========== 16-bit variables end ============ */
    /* =========== 8-bit variables start =========== */
    /** Order of PRBS Checker */
    uint8_t prbs_chk_order;
    /** Temperature Index */
    uint8_t  temp_idx;
    /** TX driver to disable high voltage control */
    uint8_t  ams_tx_drv_hv_disable;
    /** Analog Resistor Calibration value */
    uint8_t  ams_tx_ana_rescal;
    /* Analog Amplitude control */
    uint8_t  amp_ctrl;
    /* No of TXFIR Taps Enabled */
    uint8_t num_txfir_taps;
    /** TX equalization FIR pre tap weight */
    uint8_t  pre_tap;
    /** TX equalization FIR main tap weight */
    uint8_t  main_tap;
    /** TX equalization FIR post1 tap weight */
    int8_t   post1_tap;
    /** TX equalization FIR post2 tap weight */
    int8_t   post2_tap;
    /** TX equalization FIR post3 tap weight */
    int8_t   post3_tap;
    /** TX equalization FIR rpara*/
    uint8_t  rpara;
    /** Signal Detect */
    uint8_t  sigdet;
    /** Signal Detect Change */
    uint8_t  sigdet_chg;
    /** PMD RX Lock */
    uint8_t  pmd_lock;
    /** PMD RX Lock Change */
    uint8_t  pmd_lock_chg;
    /** Restart Count */
    uint8_t restart_count;
    /** Reset Count */
    uint8_t reset_count;
    /**  PMD Lock Count */
    uint8_t pmd_lock_count;
    /** Variable Gain Amplifier settings */
    int8_t  vga;
    /** Main Peaking Filter setting */
    uint8_t  pf;
    /** Low Frequency Peaking Filter settings */
    uint8_t  pf2;
    /** Main Frequency Peaking Filter reg settings */
    uint8_t  pf_reg[3];
    /** Data Slicer Threshold */
    int8_t   data_thresh;
    /** Phase Slicer Threshold */
    int8_t   phase_thresh;
    /** LMS Slicer Threshold */
    int8_t   lms_thresh;
    /** Horizontal offset between data and dataq */
    uint8_t  ddq_hoffset;
    /** Horizontal offset between phase and phaseq */
    uint8_t  ppq_hoffset;
    /** Horizontal offset between lms and lmsq */
    uint8_t  llq_hoffset;
    /** Horizontal offset between data and phase */
    uint8_t  dp_hoffset;
    /** Horizontal offset between data and lms */
    uint8_t  dl_hoffset;
    /** DC offset DAC control value */
    int8_t   dc_offset;
    /** Slicer calibration control codes (datap) */
    int8_t   thctrl_dp[4];
    /** Slicer calibration control codes (datan) */
    int8_t   thctrl_dn[4];
    /** Slicer calibration control codes (phasep) */
    int8_t   thctrl_zp[4];
    /** Slicer calibration control codes (phasen) */
    int8_t   thctrl_zn[4];
    /** Slicer calibration control codes (lms) */
    int8_t   thctrl_l[4];
    /** PRBS Checker Enable */
    uint8_t  prbs_chk_en;
    /** PRBS Checker Invert */
    uint8_t  prbs_chk_inv;
    /** PRBS Checker Lock */
    uint8_t  prbs_chk_lock;
    /** PRBS Checker Lock Lost */
    uint8_t  prbs_chk_lock_lost;
    /** uC stopped state **/
    uint8_t stop_state;
    /** Delay of zero crossing slicer, m1, wrt to data in PI codes */
    int8_t clk90;
    /** Delay of diagnostic/lms slicer, p1, wrt to data in PI codes */
    int8_t clkp1;
    /** Peaking Filter Main Settings */
    int8_t pf_main;
    /** Peaking Filter Hiz mode enable */
    uint8_t pf_hiz;
    /** Low Frequency Peaking filter control */
    int8_t pf2_ctrl;
    /** P1 eyediag status */
    int8_t p1_lvl_ctrl;
    /** DFE tap 1 value */
    int8_t dfe1;
    /** DFE tap 2 value */
    int8_t dfe2;
    /** DFE tap 3 value */
    int8_t dfe3;
    /** DFE tap 4 value */
    int8_t dfe4;
    /** DFE tap 5 value */
    int8_t dfe5;
    /** DFE tap 6 value */
    int8_t dfe6;
    /** DFE tap 1 Duty Cycle Distortion */
    int8_t dfe1_dcd;
    /** DFE tap 2 Duty Cycle Distortion */
    int8_t dfe2_dcd;
    /** Slicer calibration control codes (p1 even) */
    int8_t pe;
    /** Slicer calibration control codes (data even) */
    int8_t ze;
    /** Slicer calibration control codes (m1 even) */
    int8_t me;
    /** Slicer calibration control codes (p1 odd) */
    int8_t po;
    /** Slicer calibration control codes (data odd) */
    int8_t zo;
    /** Slicer calibration control codes (m1 odd) */
    int8_t mo;
    /** uC lane status */
    uint8_t ucv_status;
    /** TX equalization FIR pre tap weight */
    int8_t txfir_pre;
    /** TX equalization FIR main tap weight */
    int8_t txfir_main;
    /** TX equalization FIR post1 tap weight */
    int8_t txfir_post1;
    /** TX equalization FIR post2 tap weight */
    int8_t txfir_post2;
    /** Baud Rate Phase Detector enable */
    uint8_t br_pd_en;
    /** lane_reset_state **/
    uint8_t reset_state;
    /** lane_tx_reset_state **/
    uint8_t tx_reset_state;
    /** Sigdet offset correction - positive **/
    uint8_t soc_pos;
    /** Sigdet offset correction - negative **/
    uint8_t soc_neg;
    /** BER Error Rate string **/
    char ber_string[10];
    /** Big endian */
    uint8_t big_endian;
    /** Dummy 1 */
    uint8_t dummy1;
    /* Padding to complete 4-byte boundary */
    /* uint8_t padding[1]; */
    /* =========== 8-bit variables end ============== */
};  /* struct merlin8_phy1_detailed_lane_status_st  for MERLIN7 */
/* Change the size if merlin8_phy1_detailed_lane_status_st size changes */
#define SIZE_OF_EXTENDED_LANE_STATE (136 + 0)

#endif  /* if MERLIN7 elif BLACKHAWK7 elif OSPREY7 elif OSPREYLITE7 */


#ifndef PCIEG3_V1
#ifndef SMALL_FOOTPRINT
/*! Used for maintaining correctness of SIZE_OF_EXTENDED_LANE_STATE
 *
 */
typedef struct {
    uint8_t extended_lane_st_tmp[sizeof(struct merlin8_phy1_detailed_lane_status_st)==SIZE_OF_EXTENDED_LANE_STATE ? 1: -1];
} merlin8_phy1_detailed_lane_status_st_size_check;

#endif /* !SMALL_FOOTPRINT */
#endif

/****************************************************************************
 * @name Diagnostic Sampling
 *
 ****************************************************************************/
/***/

#ifdef STANDALONE_EVENT
#define DIAG_MAX_SAMPLES (64)
#else
/**
 * Diagnostic sample set size, MERLIN7 variant.
 * Applies to collections of BER measurements, eye margins, etc.
 */
#define DIAG_MAX_SAMPLES (32)
#endif



/*! @} SerdesAPITypesTag */
/*! @} APITag */
#endif
#ifdef __cplusplus
}
#endif

