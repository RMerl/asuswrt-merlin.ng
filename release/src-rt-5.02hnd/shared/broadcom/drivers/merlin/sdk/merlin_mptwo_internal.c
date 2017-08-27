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

/** @file merlin_mptwo_internal.c
 * Implementation of API functions
 */

#include "merlin_mptwo_internal.h"

SDK_STATIC uint32_t _mult_with_overflow_check(uint32_t a, uint32_t b, uint8_t *of) {
    uint16_t c,d;
    uint32_t r,s;
    if (a > b) return _mult_with_overflow_check(b, a, of);
    *of = 0;
    c = b >> 16;
    d = UINT16_MAX & b;
    r = a * c;
    s = a * d;
    if (r > UINT16_MAX) *of = 1;
    r <<= 16;
    return (s + r);
}

SDK_STATIC char* _e2s_err_code(err_code_t err_code)
{
        switch(err_code){
        case ERR_CODE_NONE: return "ERR_CODE_NONE";
        case ERR_CODE_INVALID_RAM_ADDR: return "ERR_CODE_INVALID_RAM_ADDR";
        case ERR_CODE_SERDES_DELAY: return "ERR_CODE_SERDES_DELAY";
        case ERR_CODE_POLLING_TIMEOUT: return "ERR_CODE_POLLING_TIMEOUT";
        case ERR_CODE_CFG_PATT_INVALID_PATTERN: return "ERR_CODE_CFG_PATT_INVALID_PATTERN";
        case ERR_CODE_CFG_PATT_INVALID_PATT_LENGTH: return "ERR_CODE_CFG_PATT_INVALID_PATT_LENGTH";
        case ERR_CODE_CFG_PATT_LEN_MISMATCH: return "ERR_CODE_CFG_PATT_LEN_MISMATCH";
        case ERR_CODE_CFG_PATT_PATTERN_BIGGER_THAN_MAXLEN: return "ERR_CODE_CFG_PATT_PATTERN_BIGGER_THAN_MAXLEN";
        case ERR_CODE_CFG_PATT_INVALID_HEX: return "ERR_CODE_CFG_PATT_INVALID_HEX";
        case ERR_CODE_CFG_PATT_INVALID_BIN2HEX: return "ERR_CODE_CFG_PATT_INVALID_BIN2HEX";
        case ERR_CODE_CFG_PATT_INVALID_SEQ_WRITE: return "ERR_CODE_CFG_PATT_INVALID_SEQ_WRITE";
        case ERR_CODE_PATT_GEN_INVALID_MODE_SEL: return "ERR_CODE_PATT_GEN_INVALID_MODE_SEL";
        case ERR_CODE_INVALID_UCODE_LEN: return "ERR_CODE_INVALID_UCODE_LEN";
        case ERR_CODE_MICRO_INIT_NOT_DONE: return "ERR_CODE_MICRO_INIT_NOT_DONE";
        case ERR_CODE_UCODE_LOAD_FAIL: return "ERR_CODE_UCODE_LOAD_FAIL";
        case ERR_CODE_UCODE_VERIFY_FAIL: return "ERR_CODE_UCODE_VERIFY_FAIL";
        case ERR_CODE_INVALID_TEMP_IDX: return "ERR_CODE_INVALID_TEMP_IDX";
        case ERR_CODE_INVALID_PLL_CFG: return "ERR_CODE_INVALID_PLL_CFG";
        case ERR_CODE_TX_HPF_INVALID: return "ERR_CODE_TX_HPF_INVALID";
        case ERR_CODE_VGA_INVALID: return "ERR_CODE_VGA_INVALID";
        case ERR_CODE_PF_INVALID: return "ERR_CODE_PF_INVALID";
        case ERR_CODE_TX_AMP_CTRL_INVALID: return "ERR_CODE_TX_AMP_CTRL_INVALID";
        case ERR_CODE_INVALID_EVENT_LOG_WRITE: return "ERR_CODE_INVALID_EVENT_LOG_WRITE";
        case ERR_CODE_INVALID_EVENT_LOG_READ: return "ERR_CODE_INVALID_EVENT_LOG_READ";
        case ERR_CODE_UC_CMD_RETURN_ERROR: return "ERR_CODE_UC_CMD_RETURN_ERROR";
        case ERR_CODE_DATA_NOTAVAIL: return "ERR_CODE_DATA_NOTAVAIL";
        case ERR_CODE_BAD_PTR_OR_INVALID_INPUT: return "ERR_CODE_BAD_PTR_OR_INVALID_INPUT";
        case ERR_CODE_UC_NOT_STOPPED: return "ERR_CODE_UC_NOT_STOPPED";
        case ERR_CODE_UC_CRC_NOT_MATCH: return "ERR_CODE_UC_CRC_NOT_MATCH";
        case ERR_CODE_CORE_DP_NOT_RESET: return "ERR_CODE_CORE_DP_NOT_RESET";
        case ERR_CODE_LANE_DP_NOT_RESET: return "ERR_CODE_LANE_DP_NOT_RESET";
        case ERR_CODE_CONFLICTING_PARAMETERS: return "ERR_CODE_CONFLICTING_PARAMETERS";
        case ERR_CODE_BAD_LANE_COUNT: return "ERR_CODE_BAD_LANE_COUNT";
        case ERR_CODE_BAD_LANE: return "ERR_CODE_BAD_LANE";
        case ERR_CODE_UC_ACTIVE: return "ERR_CODE_UC_ACTIVE";
        default:{
            switch(err_code>>8){
            case 1: return "ERR_CODE_TXFIR";
            case 2: return "ERR_CODE_DFE_TAP";
            case 3: return "ERR_CODE_DIAG";
            default: return "Invalid error code";
            }
        }
        }
}

SDK_STATIC err_code_t _print_err_msg(uint16_t err_code, const char *s, uint32_t n)
{
    if (err_code != 0) {
        USR_PRINTF(("\n%s:%d: ERROR: SerDes err_code = %s\n", s, n, _e2s_err_code(err_code)));
    }
    return err_code;
}


/* Formula for PVTMON: T = 410.04-0.48705*reg10bit (from PVTMON Analog Module Specification v5.0 section 6.2) */
#define _bin_to_degC(bin) ((int16_t)(410 + ((2212 - 1995*(int32_t)bin)>>12)))

/* Store a cached AFE version for re-use */
static err_code_t _get_afe_hw_version(merlin_access_t *ma, uint8_t *afe_hw_version) {
  static uint8_t _cached_afe_hw_version = 255;

  if (!afe_hw_version)
    return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  if (_cached_afe_hw_version == 255)
    ESTM(_cached_afe_hw_version = rdcv_afe_hardware_version());
  *afe_hw_version = _cached_afe_hw_version;
  return(ERR_CODE_NONE);
}



SDK_STATIC err_code_t _get_p1_threshold(merlin_access_t *ma, int8_t *val) {
    ESTM(*val = -rd_p1_eyediag_bin());
 return (ERR_CODE_NONE);
}


/* Setup the P1 slicer vertical level  */
SDK_STATIC err_code_t _set_p1_threshold(merlin_access_t *ma, int8_t threshold) {

  EFUN(_check_uc_lane_stopped(ma));                     /* make sure uC is stopped to avoid race conditions */

  EFUN(wr_dfe_vga_write_tapsel(0xd));                 /* Configure dfe_vga_write_tapsel to p1_eyediag mode  */
  /* invert polarity for MERLIN, JIRA CRMERLIN_PMD-159 */
  EFUN(wr_dfe_vga_write_val((-threshold)<<3));        /* dfe_vga_write_val[8:3] are used to drive the analog control port. */
  EFUN(wr_dfe_vga_write_en(0x1));                     /* Enable f/w to write the tap values */
  return (ERR_CODE_NONE);
}

SDK_STATIC err_code_t _move_clkp1_offset(merlin_access_t *ma, int8_t delta) {
  int8_t cnt;

  EFUN(_check_uc_lane_stopped(ma));                     /* make sure uC is stopped to avoid race conditions */

  EFUN(wr_rx_pi_slicers_en(0x2));                     /* Select p1 slicer to adjust */
  EFUN(wr_rx_pi_phase_step_dir(delta>0));             /* 1 for positive step   */
  EFUN(wr_rx_pi_phase_step_cnt(1));                   /* Step set to 1 */
  for (cnt=0; cnt < _abs(delta); cnt++) {
    EFUN(wr_rx_pi_manual_strobe(1));                  /* Increments/Decrements by 1 every strobe */
  }
  return(ERR_CODE_NONE);
}



SDK_STATIC int16_t _ladder_setting_to_mV(merlin_access_t *ma, int8_t ctrl, uint8_t range_250) {
    uint8_t absv;                                     /* Absolute value of ctrl */
    int16_t nlmv;                                     /* Non-linear value */

    /* Get absolute value */
    absv = _abs(ctrl);

    {
    int16_t nlv;                                      /* Non-linear value */
    uint8_t api_afe_hw_version = 0;
    EFUN(_get_afe_hw_version(ma,&api_afe_hw_version));
    if (api_afe_hw_version == 0) {
      /* G28 A0, KOS, KOI A0 version */
      /* Convert to linear scale from non-linear*/
      /* First 25 steps are 1x, next 5 steps are 3x, last step is 2x */
      nlv = (absv + 2*(absv-25)*(absv>25) - (absv==31));

      /* Convert from non-linear scale to mV */
      if (range_250) {
         /* 250mV range, 6mV units */
         nlmv = nlv*6;
       } else {
        /* 150mV range, 6 *0.6 = 3.6 mV runits */
        /* Multiply by 0.6 and add 0.5 for rounding */
        nlmv = (nlv*6*6+5)/10;
      }
    } else {
        /* G28 B0 onwards */
        /* Convert to linear scale from non-linear*/
        /* First 24 steps are 6mV, last 7 steps are 16mV   */
        nlmv = 6*absv + (absv-24)*(absv>24)*10;

        if (!range_250)
            /* 150mV range, 6 *0.6 = 3.6 mV runits */
            /* Multiply by 0.6 and add 0.5 for rounding */
            nlmv = (nlmv*6+5)/10;
    }
    }
    /* Add sign and return */
    return( (ctrl>=0) ? nlmv : -nlmv);
}


static err_code_t _compute_bin(char var, char bin[]) {
    if(!bin) {
        return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

  switch (var) {
    case '0':  ENULL_STRCPY(bin,"0000");
               break;
    case '1':  ENULL_STRCPY(bin,"0001");
               break;
    case '2':  ENULL_STRCPY(bin,"0010");
               break;
    case '3':  ENULL_STRCPY(bin,"0011");
               break;
    case '4':  ENULL_STRCPY(bin,"0100");
               break;
    case '5':  ENULL_STRCPY(bin,"0101");
               break;
    case '6':  ENULL_STRCPY(bin,"0110");
               break;
    case '7':  ENULL_STRCPY(bin,"0111");
               break;
    case '8':  ENULL_STRCPY(bin,"1000");
               break;
    case '9':  ENULL_STRCPY(bin,"1001");
               break;
    case 'a':
    case 'A':  ENULL_STRCPY(bin,"1010");
               break;
    case 'b':
    case 'B':  ENULL_STRCPY(bin,"1011");
               break;
    case 'c':
    case 'C':  ENULL_STRCPY(bin,"1100");
               break;
    case 'd':
    case 'D':  ENULL_STRCPY(bin,"1101");
               break;
    case 'e':
    case 'E':  ENULL_STRCPY(bin,"1110");
               break;
    case 'f':
    case 'F':  ENULL_STRCPY(bin,"1111");
               break;
    case '_':  ENULL_STRCPY(bin,"");
               break;
    default :  ENULL_STRCPY(bin,"");
               EFUN_PRINTF(("ERROR: Invalid Hexadecimal Pattern\n"));
               return (_error(ERR_CODE_CFG_PATT_INVALID_HEX));
               break;
  }
  return (ERR_CODE_NONE);
}


static err_code_t _compute_hex(char bin[],uint8_t *hex) {
    if(!hex) {
        return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }


  if (!USR_STRCMP(bin,"0000")) {
    *hex = 0x0;
  }
  else if (!USR_STRCMP(bin,"0001")) {
    *hex = 0x1;
  }
  else if (!USR_STRCMP(bin,"0010")) {
    *hex = 0x2;
  }
  else if (!USR_STRCMP(bin,"0011")) {
    *hex = 0x3;
  }
  else if (!USR_STRCMP(bin,"0100")) {
    *hex = 0x4;
  }
  else if (!USR_STRCMP(bin,"0101")) {
    *hex = 0x5;
  }
  else if (!USR_STRCMP(bin,"0110")) {
    *hex = 0x6;
  }
  else if (!USR_STRCMP(bin,"0111")) {
    *hex = 0x7;
  }
  else if (!USR_STRCMP(bin,"1000")) {
    *hex = 0x8;
  }
  else if (!USR_STRCMP(bin,"1001")) {
    *hex = 0x9;
  }
  else if (!USR_STRCMP(bin,"1010")) {
    *hex = 0xA;
  }
  else if (!USR_STRCMP(bin,"1011")) {
    *hex = 0xB;
  }
  else if (!USR_STRCMP(bin,"1100")) {
    *hex = 0xC;
  }
  else if (!USR_STRCMP(bin,"1101")) {
    *hex = 0xD;
  }
  else if (!USR_STRCMP(bin,"1110")) {
    *hex = 0xE;
  }
  else if (!USR_STRCMP(bin,"1111")) {
    *hex = 0xF;
  }
  else {
    EFUN_PRINTF(("ERROR: Invalid Binary to Hex Conversion\n"));
    *hex = 0x0;
        return (_error(ERR_CODE_CFG_PATT_INVALID_BIN2HEX));

  }
  return (ERR_CODE_NONE);
}

/* Repeater Only APIs (Not applicable to PMD) */

/* Setup the Ultra low latency clk and datapath for TX [Return Val = Error Code (0 = PASS)] */
static err_code_t _ull_tx_mode_setup(merlin_access_t *ma, uint8_t enable) {
    EFUN(wr_ams_tx_fifo_phsdetect_mode(enable));
    EFUN(wr_ams_tx_lowlatency_en(enable));                   /* Enable FIFO phase detector output */

    EFUN(wr_tx_pi_hs_fifo_phserr_invert(0x0));
    EFUN(wr_tx_pi_ext_phase_bwsel_integ(0x5));            /* 0 to 5 */
    EFUN(wr_tx_pi_ext_ctrl_en(enable));                      /* Turn on jitter filter's phase detector input path  */

    EFUN(merlin_mptwo_delay_us(1024));                          /* Wait for phase detector path to settle */

    EFUN(wr_tx_pi_second_order_loop_en(enable));             /* Turn on 2nd order loop in jitter filter */

    EFUN(wr_ams_tx_ll_fifo_ctrl(0x6));

    EFUN(wr_afe_tx_fifo_resetb(enable));                     /* Release user reset to FIFO */
                                                          /* When the correct event occurs, reset is released to AFE's actual FIFO */
    EFUN(merlin_mptwo_poll_st_afe_tx_fifo_resetb_equals_1(ma,1));  /* Check if FIFO reset is reported to be released within 1ms time interval */

    EFUN(wr_ams_tx_ll_selpath_tx(enable));                   /* Switch mux and enable traffic to flow through */
    return (ERR_CODE_NONE);
}
/* Setup the Ultra low latency for RX [Return Val = Error Code (0 = PASS)] */
static err_code_t _ull_rx_mode_setup(merlin_access_t *ma, uint8_t enable) {
    EFUN(wr_ams_rx_ll_en(enable));                           /* Enable RX ULL datapath */
    return (ERR_CODE_NONE);
}




/********************************************************/
/*  Global RAM access through Micro Register Interface  */
/********************************************************/
/* Micro Global RAM Byte Read */
static uint8_t _merlin_mptwo_rdb_uc_var(merlin_access_t *ma, err_code_t *err_code_p, uint16_t addr) {
    uint8_t rddata;

    if(!err_code_p) {
        return(0);
    }

    EPSTM(rddata = merlin_mptwo_rdb_uc_ram(ma, err_code_p, addr)); /* Use Micro register interface for reading RAM */

    return (rddata);
}

/* Micro Global RAM Word Read */
static uint16_t _merlin_mptwo_rdw_uc_var(merlin_access_t *ma, err_code_t *err_code_p, uint16_t addr) {
  uint16_t rddata;

  if(!err_code_p) {
      return(0);
  }

  if (addr%2 != 0) {                                                                /* Validate even address */
      *err_code_p = ERR_CODE_INVALID_RAM_ADDR;
      return (0);
  }
#ifdef ATE_LOG
  EFUN_PRINTF(("// ATE_LOG IGNORE FOLLOWING READ ACCESS!\n"));
#endif
  EPSTM(rddata = merlin_mptwo_rdw_uc_ram(ma, err_code_p, (addr))); /* Use Micro register interface for reading RAM */

  return (rddata);
}

/* Micro Global RAM Byte Write  */
static err_code_t _merlin_mptwo_wrb_uc_var(merlin_access_t *ma, uint16_t addr, uint8_t wr_val) {

    return (merlin_mptwo_wrb_uc_ram(ma, addr, wr_val));                                /* Use Micro register interface for writing RAM */
}
/* Micro Global RAM Word Write  */
static err_code_t _merlin_mptwo_wrw_uc_var(merlin_access_t *ma, uint16_t addr, uint16_t wr_val) {
    if (addr%2 != 0) {                                                                /* Validate even address */
        return (_error(ERR_CODE_INVALID_RAM_ADDR));
    }
    return (merlin_mptwo_wrw_uc_ram(ma, addr, wr_val));                                 /* Use Micro register interface for writing RAM */
}


static err_code_t _display_event(uint8_t event_id,
                          uint8_t entry_len,
                          uint8_t prev_cursor,
                          uint8_t curr_cursor,
                          uint8_t post_cursor,
                          uint8_t *supp_info) {
    char *s1;
    switch (event_id) {                                                       /* decode event code */
    case EVENT_CODE_ENTRY_TO_DSC_RESET:
        EFUN_PRINTF(("  Entry to DSC reset"));
        break;
    case EVENT_CODE_RELEASE_USER_RESET:
        EFUN_PRINTF(("  Release user reset"));
        break;
    case EVENT_CODE_EXIT_FROM_DSC_RESET:
        EFUN_PRINTF(("  Exit from DSC reset"));
        break;
    case EVENT_CODE_ENTRY_TO_CORE_RESET:
        EFUN_PRINTF(("  Entry to core reset"));
        break;
    case EVENT_CODE_RELEASE_USER_CORE_RESET:
        EFUN_PRINTF(("  Release user core reset"));
        break;
    case EVENT_CODE_ACTIVE_RESTART_CONDITION:
        EFUN_PRINTF(("  Active restart condition"));
        break;
    case EVENT_CODE_EXIT_FROM_RESTART:
        EFUN_PRINTF(("  Exit from restart"));
        break;
    case EVENT_CODE_WRITE_TR_COARSE_LOCK:
        EFUN_PRINTF(("  Write timing recovery coarse lock"));
        break;
    case EVENT_CODE_CL72_READY_FOR_COMMAND:
        EFUN_PRINTF(("  General event - %d",EVENT_CODE_CL72_READY_FOR_COMMAND));
        break;
    case EVENT_CODE_EACH_WRITE_TO_CL72_TX_CHANGE_REQUEST:
        EFUN_PRINTF(("  General event - %d",EVENT_CODE_EACH_WRITE_TO_CL72_TX_CHANGE_REQUEST));
        break;
    case EVENT_CODE_FRAME_LOCK:
        EFUN_PRINTF(("  CL72 Frame locked"));
        break;
    case EVENT_CODE_LOCAL_RX_TRAINED:
        EFUN_PRINTF(("  CL72 Local Rx trained"));
        break;
    case EVENT_CODE_DSC_LOCK:
        EFUN_PRINTF(("  DSC lock"));
        break;
    case EVENT_CODE_FIRST_RX_PMD_LOCK:
        EFUN_PRINTF(("  Rx PMD lock"));
        break;
    case EVENT_CODE_PMD_RESTART_FROM_CL72_CMD_INTF_TIMEOUT:
        EFUN_PRINTF(("  PMD restart due to CL72 ready for command timeout"));
        break;
    case EVENT_CODE_LP_RX_READY:
        EFUN_PRINTF(("  CL72 Remote receiver ready"));
        break;
    case EVENT_CODE_STOP_EVENT_LOG:
        EFUN_PRINTF(("  Start reading event log"));
        break;
    case EVENT_CODE_GENERAL_EVENT_0:
        EFUN_PRINTF(("  General event 0, (0x%x%x)",post_cursor,prev_cursor));
        break;
    case EVENT_CODE_GENERAL_EVENT_1:
        EFUN_PRINTF(("  General event 1, (0x%x%x)",post_cursor,prev_cursor));
        break;
    case EVENT_CODE_GENERAL_EVENT_2:
        EFUN_PRINTF(("  General event 2, (0x%x%x)",post_cursor,prev_cursor));
        break;
    case EVENT_CODE_ERROR_EVENT:
        s1 = _error_val_2_str(post_cursor);
        EFUN_PRINTF(("  UC error event: %s", s1));
        break;
    case EVENT_CODE_NUM_TIMESTAMP_WRAPAROUND_MAXOUT:
        EFUN_PRINTF(("  Reset number of timestamp wraparounds"));
        break;
    case EVENT_CODE_RESTART_PMD_ON_CDR_LOCK_LOST:
        EFUN_PRINTF(("  Restart Rx PMD on CDR lock lost"));
        break;
    case EVENT_CODE_RESTART_PMD_ON_CLOSE_EYE:
        EFUN_PRINTF(("  Restart Rx PMD because of closed eye"));
        break;
    case EVENT_CODE_RESTART_PMD_ON_DFE_TAP_CONFIG:
        EFUN_PRINTF(("  Restart Rx PMD on maxed out DFE tap magnitude"));
        break;
    case EVENT_CODE_SM_STATUS_RESTART:
        EFUN_PRINTF(("  Check DSC SM status restart reg value"));
        break;
    case EVENT_CODE_CORE_PROGRAMMING:
        EFUN_PRINTF(("  Program core config value"));
        break;
    case EVENT_CODE_LANE_PROGRAMMING:
        EFUN_PRINTF(("  Program lane config value"));
        break;
    case EVENT_CODE_CL72_AUTO_POLARITY_CHANGE:
        EFUN_PRINTF(("  CL72 Auto Polarity Change"));
        break;
    case EVENT_CODE_RESTART_FROM_CL72_MAX_TIMEOUT:
        EFUN_PRINTF(("  Restart Rx PMD due to F-CL72 training timeout"));
        break;
    case EVENT_CODE_CL72_LOCAL_TX_CHANGED:
        EFUN_PRINTF(("  General event - %d",EVENT_CODE_CL72_LOCAL_TX_CHANGED));
        break;
    case EVENT_CODE_FIRST_WRITE_TO_CL72_TX_CHANGE_REQUEST:
        EFUN_PRINTF(("  First write to LP Cl72 transmit change request"));
        break;
    case EVENT_CODE_FRAME_UNLOCK:
        EFUN_PRINTF(("  General event - %d",EVENT_CODE_FRAME_UNLOCK));
        break;
    case EVENT_CODE_ENTRY_TO_CORE_PLL1_RESET:
        EFUN_PRINTF(("  Entry to core PLL1 reset"));
        break;
    case EVENT_CODE_RELEASE_USER_CORE_PLL1_RESET:
        EFUN_PRINTF(("  Release user core PLL1 reset"));
        break;

    default:
        EFUN_PRINTF(("  UNRECOGNIZED EVENT CODE (0x%x) !!!",event_id));
        break;
    }

    if (entry_len == 4) {
        EFUN_PRINTF(("\n"));
    }
    else {
        int ii=0;
        EFUN_PRINTF((", SUP_INFO={"));
        supp_info += (entry_len-5);
        for (ii=0; ii<entry_len-4; ii++) {
            if (ii != 0) {
                EFUN_PRINTF((", "));
            }
            EFUN_PRINTF(("0x%x",*supp_info));
            supp_info--;
        }
        EFUN_PRINTF(("}\n"));
        if ((event_id >= EVENT_CODE_MAX) && (event_id < EVENT_CODE_TIMESTAMP_WRAP_AROUND)) {
            /* newline for unrecognized event */
            EFUN_PRINTF(("\n"));
        }
    }

    return(ERR_CODE_NONE);
}



static char* _error_val_2_str(uint8_t val) {
        switch (val) {
    case INVALID_CORE_TEMP_IDX:
        return ("INVALID CORE TEMP INDEX");
        case INVALID_OTP_CONFIG:
                return ("INVALID OTP CONFIGURATION");
        case DSC_CONFIG_INVALID_REENTRY_ID:
                return ("DSC CONFIG INVALID REENTRY");
        case INVALID_REENTRY_ID:
                return ("INVALID REENTRY");
        case GENERIC_UC_ERROR:
                return ("GENERIC UC ERROR");
        default:
                return ("UNDEFINED");
        }
}

#ifdef TO_FLOATS
/* convert uint32_t to float8 */
static float8_t _int32_to_float8(uint32_t input) {
        int8_t cnt;
        uint8_t output;

        if(input == 0) {
                return(0);
        } else if(input == 1) {
                return(0xe0);
        } else {
                cnt = 0;
                input = input & 0x7FFFFFFF; /* get rid of MSB which may be lock indicator */
                do {
                        input = input << 1;
                        cnt++;
                } while ((input & 0x80000000) == 0);

                output = (uint8_t)((input & 0x70000000)>>23)+(31 - cnt%32);
                        return(output);
        }
}
#endif

/* convert float8 to uint32_t */
static uint32_t _float8_to_int32(float8_t input) {
        uint32_t x;
        if(input == 0) return(0);
        x = (input>>5) + 8;
        if((input & 0x1F) < 3) {
                return(x>>(3-(input & 0x1f)));
        }
        return(x<<((input & 0x1F)-3));
}

/* convert float12 to uint32 */
static uint32_t _float12_to_uint32(uint8_t byte, uint8_t multi) {

        return(((uint32_t)byte)<<multi);
}

#ifdef TO_FLOATS
/* convert uint32 to float12 */
static uint8_t _uint32_to_float12(uint32_t input, uint8_t *multi) {
        int8_t cnt;
        uint8_t output;
        if(!multi) {
                return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
        }

        if((input == 0) || (!multi)) {
                *multi = 0;
                return(0);
        } else {
                cnt = 0;
                if(input > 0x007FFFFF) input = 0x007FFFFF; /* limit to 23bits so multi is 4 bits */
                do {
                        input = input << 1;
                        cnt++;
                } while ((input & 0x80000000) == 0);

                *multi = (31 - (cnt%32));
                if(*multi < 8) {
                        output = (uint8_t)((input & 0xFF000000)>>(24 + (7-*multi)));
                        *multi = 0;
                } else {
                        output = (uint8_t)((input & 0xFF000000)>>24);
                        *multi = *multi - 7;
                }
                return(output);
        }
}
#endif

SDK_STATIC err_code_t _set_rx_pf_main(merlin_access_t *ma, uint8_t val) {
        if (val > 15) {
                return (_error(ERR_CODE_PF_INVALID));
        }
        EFUN(wr_pf_ctrl(val));
        return(ERR_CODE_NONE);
}

static err_code_t _get_rx_pf_main(merlin_access_t *ma, int8_t *val) {
                ESTM(*val = (int8_t)rd_pf_ctrl());
 return (ERR_CODE_NONE);
}

SDK_STATIC err_code_t _set_rx_pf2(merlin_access_t *ma, uint8_t val) {
        if (val > 7) {
                return (_error(ERR_CODE_PF_INVALID));
        }
        EFUN(wr_pf2_lowp_ctrl(val));
        return(ERR_CODE_NONE);
}

static err_code_t _get_rx_pf2(merlin_access_t *ma, int8_t *val) {
                ESTM(*val = (int8_t)rd_pf2_lowp_ctrl());
 return (ERR_CODE_NONE);
}

SDK_STATIC err_code_t _set_rx_vga(merlin_access_t *ma, uint8_t val) {

    EFUN(_check_uc_lane_stopped(ma));                     /* make sure uC is stopped to avoid race conditions */

        if (val > 45) {
                return (_error(ERR_CODE_VGA_INVALID));
        }
        EFUN(wr_dfe_vga_write_tapsel(0));                   /* Configure dfe_vga_write_tapsel to VGA */
        EFUN(wr_dfe_vga_write_val(val<<3));                 /* dfe_vga_write_val[8:3] are used to drive the analog control port */
        EFUN(wr_dfe_vga_write_en(1));
        return(ERR_CODE_NONE);
}

static err_code_t _get_rx_vga(merlin_access_t *ma, int8_t *val) {
                ESTM(*val = (int8_t)rd_vga_bin());
 return (ERR_CODE_NONE);
}


SDK_STATIC err_code_t _set_rx_dfe1(merlin_access_t *ma, int8_t val) {

    EFUN(_check_uc_lane_stopped(ma));                     /* make sure uC is stopped to avoid race conditions */

    {
        int8_t tap_eo;

        if (val > 63) {
            return (_error(ERR_CODE_DFE1_INVALID));
        }
        /* Compute tap1 even/odd component */
        tap_eo = 0;                                         /* Not supporting dfe_dcd values */
        EFUN(wr_dfe_vga_write_tapsel(2));                   /* Write tap1_odd */
        EFUN(wr_dfe_vga_write_val(tap_eo));
        EFUN(wr_dfe_vga_write_en(1));
        EFUN(wr_dfe_vga_write_tapsel(3));                   /* Write tap1_even */
        EFUN(wr_dfe_vga_write_val(tap_eo));
        EFUN(wr_dfe_vga_write_en(1));
        EFUN(wr_dfe_vga_write_tapsel(1));
        EFUN(wr_dfe_vga_write_val(val-tap_eo));           /* Write the common tap */
        EFUN(wr_dfe_vga_write_en(1));
    }
        return(ERR_CODE_NONE);
}
SDK_STATIC err_code_t _get_rx_dfe1(merlin_access_t *ma, int8_t *val) {
        ESTM(*val = (int8_t)(rd_dfe_1_e() + rd_dfe_1_cmn()));
 return (ERR_CODE_NONE);
}


SDK_STATIC err_code_t _set_rx_dfe2(merlin_access_t *ma, int8_t val) {
    int8_t tap_eo;

    if ((val > 31) || (val < -31)) {
        return (_error(ERR_CODE_DFE2_INVALID));
    }

    EFUN(_check_uc_lane_stopped(ma));                     /* make sure uC is stopped to avoid race conditions */

    /* Compute tap2 odd/even component */
    tap_eo = 0;                                         /* Not supporting dfe_dcd values */
    EFUN(wr_dfe_vga_write_tapsel(5));                   /* Write tap2_odd */
    EFUN(wr_dfe_vga_write_val(tap_eo));
    EFUN(wr_dfe_vga_write_en(1));
    EFUN(wr_dfe_vga_write_tapsel(6));                   /* Write tap2_even */
    EFUN(wr_dfe_vga_write_val(tap_eo));
    EFUN(wr_dfe_vga_write_en(1));
    EFUN(wr_dfe_vga_write_tapsel(4));
    EFUN(wr_dfe_vga_write_val(_abs(val)-tap_eo));      /* Write the common tap */
    EFUN(wr_dfe_vga_write_en(1));
    EFUN(wr_dfe_vga_write_tapsel(10));                  /* Write tap2_even_sign */
    EFUN(wr_dfe_vga_write_val(val<0?1:0));
    EFUN(wr_dfe_vga_write_en(1));
    EFUN(wr_dfe_vga_write_tapsel(11));                  /* Write tap2_odd_sign */
    EFUN(wr_dfe_vga_write_val(val<0?1:0));
    EFUN(wr_dfe_vga_write_en(1));
    return(ERR_CODE_NONE);
}

SDK_STATIC err_code_t _get_rx_dfe2(merlin_access_t *ma, int8_t *val) {
    ESTM(*val = rd_dfe_2_se() ? -(rd_dfe_2_e()+rd_dfe_2_cmn()) : (rd_dfe_2_e()+rd_dfe_2_cmn()));
 return (ERR_CODE_NONE);
}

static err_code_t _set_rx_dfe3(merlin_access_t *ma, int8_t val) {
    if ((val > 31) || (val < -31)) {
        return (_error(ERR_CODE_DFE3_INVALID));
    }

    EFUN(_check_uc_lane_stopped(ma));                     /* make sure uC is stopped to avoid race conditions */

    EFUN(wr_dfe_vga_write_tapsel(7));                   /* Write tap3 */
    EFUN(wr_dfe_vga_write_val(val));
    EFUN(wr_dfe_vga_write_en(1));
    return(ERR_CODE_NONE);
}

static err_code_t _get_rx_dfe3(merlin_access_t *ma, int8_t *val) {
    ESTM(*val = rd_dfe_3_cmn());
 return (ERR_CODE_NONE);
}

static err_code_t _set_rx_dfe4(merlin_access_t *ma, int8_t val) {
    if ((val > 15) || (val < -15)) {
            return (_error(ERR_CODE_DFE4_INVALID));
    }

    EFUN(_check_uc_lane_stopped(ma));                     /* make sure uC is stopped to avoid race conditions */

    EFUN(wr_dfe_vga_write_tapsel(8));                   /* Write tap4 */
    EFUN(wr_dfe_vga_write_val(val));
    EFUN(wr_dfe_vga_write_en(1));
    return(ERR_CODE_NONE);
}

static err_code_t _get_rx_dfe4(merlin_access_t *ma, int8_t *val) {
    ESTM(*val = rd_dfe_4_cmn());
    return (ERR_CODE_NONE);
}

static err_code_t _set_rx_dfe5(merlin_access_t *ma, int8_t val) {
    if ((val > 15) || (val < -15)) {
        return (_error(ERR_CODE_DFE5_INVALID));
    }

    EFUN(_check_uc_lane_stopped(ma));                     /* make sure uC is stopped to avoid race conditions */

    EFUN(wr_dfe_vga_write_tapsel(9));                   /* Write tap5 */
    EFUN(wr_dfe_vga_write_val(val));
    EFUN(wr_dfe_vga_write_en(1));
    return(ERR_CODE_NONE);
}

static err_code_t _get_rx_dfe5(merlin_access_t *ma, int8_t *val) {
        ESTM(*val = rd_dfe_5_cmn());
 return (ERR_CODE_NONE);
}





static err_code_t _get_tx_pre(merlin_access_t *ma, int8_t *val) {
    ESTM(*val = rd_txfir_pre_after_ovr());
    return (ERR_CODE_NONE);
}


static err_code_t _get_tx_main(merlin_access_t *ma, int8_t *val) {
    ESTM(*val = rd_txfir_main_after_ovr());
    return (ERR_CODE_NONE);
}


static err_code_t _get_tx_post1(merlin_access_t *ma, int8_t *val) {
    ESTM(*val = rd_txfir_post_after_ovr());
    return (ERR_CODE_NONE);
}


static err_code_t _get_tx_post2(merlin_access_t *ma, int8_t *val) {
    ESTM(*val = rd_txfir_post2_adjusted());
    return (ERR_CODE_NONE);
}


static err_code_t _merlin_mptwo_core_clkgate(merlin_access_t *ma, uint8_t enable) {

    if (enable) {
    }
    else {
    }
    return (ERR_CODE_NONE);
}

static err_code_t _merlin_mptwo_lane_clkgate(merlin_access_t *ma, uint8_t enable) {

    if (enable) {
        /* Use frc/frc_val to force all RX and TX clk_vld signals to 0 */
        EFUN(wr_pmd_rx_clk_vld_frc_val(0x0));
        EFUN(wr_pmd_rx_clk_vld_frc(0x1));
        EFUN(wr_pmd_tx_clk_vld_frc_val(0x0));
        EFUN(wr_pmd_tx_clk_vld_frc(0x1));

        /* Use frc/frc_val to force clkgate */
        EFUN(wr_ln_rx_s_clkgate_frc_val(0x1));
        EFUN(wr_ln_rx_s_clkgate_frc(0x1));
        EFUN(wr_ln_tx_s_clkgate_frc_val(0x1));
        EFUN(wr_ln_tx_s_clkgate_frc(0x1));

    }
    else {
        EFUN(wr_pmd_rx_clk_vld_frc_val(0x0));
        EFUN(wr_pmd_rx_clk_vld_frc(0x0));
        EFUN(wr_pmd_tx_clk_vld_frc_val(0x0));
        EFUN(wr_pmd_tx_clk_vld_frc(0x1));

        EFUN(wr_ln_rx_s_clkgate_frc_val(0x0));
        EFUN(wr_ln_rx_s_clkgate_frc(0x0));
        EFUN(wr_ln_tx_s_clkgate_frc_val(0x0));
        EFUN(wr_ln_tx_s_clkgate_frc(0x0));
    }
    return (ERR_CODE_NONE);
}

SDK_STATIC uint16_t _eye_to_mUI(uint8_t var)
{
    /* var is in units of 1/512 th UI, so need to multiply by 1000/512 */
    return ((uint16_t)var)*125/64;
}


SDK_STATIC uint16_t _eye_to_mV(merlin_access_t *ma, uint8_t var, uint8_t ladder_range)
{
    /* find nearest two vertical levels */
    uint16_t vl = _ladder_setting_to_mV(ma,var/8, ladder_range);
    uint16_t vu = _ladder_setting_to_mV(ma,_min(31,var/8+1), ladder_range);
    return (vl + (vu-vl)*(var&7)/8);
}

SDK_STATIC err_code_t _merlin_mptwo_get_osr_mode(merlin_access_t *ma, merlin_mptwo_osr_mode_st *imode) {
    merlin_mptwo_osr_mode_st mode;

     ENULL_MEMSET(&mode, 0, sizeof(merlin_mptwo_osr_mode_st));

    if(!imode)
        return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));

  ESTM(mode.tx_rx = rd_osr_mode());
  mode.tx = mode.tx_rx;mode.rx = mode.tx_rx;
  *imode = mode;
  return (ERR_CODE_NONE);

}

SDK_STATIC err_code_t _merlin_mptwo_read_lane_state(merlin_access_t *ma, merlin_mptwo_lane_state_st *istate) {

  merlin_mptwo_lane_state_st state;
  uint8_t ladder_range = 0;

  ENULL_MEMSET(&state, 0, sizeof(merlin_mptwo_lane_state_st));

  if(!istate)
          return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));

  EFUN(_merlin_mptwo_pmd_lock_status(ma,&state.rx_lock, &state.rx_lock_chg));

  if (state.rx_lock == 1) {
      ESTM(state.stop_state = rdv_usr_sts_micro_stopped());
      if (!state.stop_state) {
          EFUN(merlin_mptwo_stop_rx_adaptation(ma,1));
      }
  } else {
      EFUN(merlin_mptwo_pmd_uc_control(ma,CMD_UC_CTRL_STOP_IMMEDIATE,200));
  }

  {  merlin_mptwo_osr_mode_st osr_mode;
  ENULL_MEMSET(&osr_mode, 0, sizeof(merlin_mptwo_osr_mode_st));
  EFUN(_merlin_mptwo_get_osr_mode(ma,&osr_mode));
  state.osr_mode = osr_mode;
  }
  ESTM(state.ucv_config = rdv_config_word());
  ESTM(state.reset_state = rd_lane_dp_reset_state());
  ESTM(state.tx_pll_select = 255);
  ESTM(state.rx_pll_select = 255);
  EFUN(_merlin_mptwo_sigdet_status(ma, &state.sig_det, &state.sig_det_chg));
  ESTM(state.rx_ppm = rd_cdr_integ_reg()/84);
  EFUN(merlin_mptwo_get_clk90_offset(ma,&state.clk90));
  ESTM(state.clkp1 = rd_cnt_d_minus_p1());
  ESTM(state.br_pd_en = rd_br_pd_en());
  /* drop the MSB, the result is actually valid modulo 128 */
  /* Also flip the sign to account for d-m1, versus m1-d */
  state.clkp1 = state.clkp1<<1;
  state.clkp1 = -(state.clkp1>>1);

  EFUN(_get_rx_pf_main(ma,&state.pf_main));
  ESTM(state.pf_hiz = rd_pf_hiz());
  ESTM(state.pf2_ctrl = rd_pf2_lowp_ctrl());
  EFUN(_get_rx_vga(ma,&state.vga));
  ESTM(state.dc_offset = rd_dc_offset());
  ESTM(ladder_range = rd_p1_thresh_sel());
  EFUN(_get_p1_threshold(ma,&state.p1_lvl_ctrl));
  state.p1_lvl = _ladder_setting_to_mV(ma,state.p1_lvl_ctrl, ladder_range);
  state.m1_lvl = 0;

  ESTM(state.pf_bst = rd_ams_rx_pkbst());

  EFUN(_get_rx_dfe1(ma,&state.dfe1));
  EFUN(_get_rx_dfe2(ma,&state.dfe2));
  EFUN(_get_rx_dfe3(ma,&state.dfe3));
  EFUN(_get_rx_dfe4(ma,&state.dfe4));
  EFUN(_get_rx_dfe5(ma,&state.dfe5));

  ESTM(state.dfe1_dcd = rd_dfe_1_e()-rd_dfe_1_o());
  ESTM(state.dfe2_dcd = (rd_dfe_2_se() ? -rd_dfe_2_e(): rd_dfe_2_e()) -(rd_dfe_2_so() ? -rd_dfe_2_o(): rd_dfe_2_o()));

  ESTM(state.pe = rd_dfe_offset_adj_p1_even());
  ESTM(state.ze = rd_dfe_offset_adj_data_even());
  ESTM(state.me = rd_dfe_offset_adj_m1_even());

  ESTM(state.po = rd_dfe_offset_adj_p1_odd());
  ESTM(state.zo = rd_dfe_offset_adj_data_odd());
  ESTM(state.mo = rd_dfe_offset_adj_m1_odd());

  /* tx_ppm = register/10.84 */
  ESTM(state.tx_ppm = (int16_t)(((int32_t)(rd_tx_pi_integ2_reg()))*3125/32768));

  EFUN(_get_tx_pre(ma,&state.txfir_pre));
  EFUN(_get_tx_main(ma,&state.txfir_main));
  EFUN(_get_tx_post1(ma,&state.txfir_post1));
  EFUN(_get_tx_post2(ma,&state.txfir_post2));
  ESTM(state.heye_left = _eye_to_mUI(rdv_usr_sts_heye_left()));
  ESTM(state.heye_right = _eye_to_mUI(rdv_usr_sts_heye_right()));
  ESTM(state.veye_upper = _eye_to_mV(ma,rdv_usr_sts_veye_upper(), ladder_range));
  ESTM(state.veye_lower = _eye_to_mV(ma,rdv_usr_sts_veye_lower(), ladder_range));
  ESTM(state.link_time = (((uint32_t)rdv_usr_sts_link_time())*8)/10);     /* convert from 80us to 0.1 ms units */

  if (state.rx_lock == 1) {
      if (!state.stop_state) {
          EFUN(merlin_mptwo_stop_rx_adaptation(ma,0));
      }
  } else {
      EFUN(merlin_mptwo_stop_rx_adaptation(ma,0));
  }

  *istate = state;
  return (ERR_CODE_NONE);
}

static err_code_t _merlin_mptwo_display_lane_state_no_newline(merlin_access_t *ma) {
  uint16_t lane_idx;
  merlin_mptwo_lane_state_st state;

  const char* e2s_osr_mode_enum[10] = {
    "x1       ",
    "x2       ",
    "x3       ",
    "x3.3     ",
    "x4       ",
    "x5       ",
    "x7.5     ",
    "x8       ",
    "x8.25    ",
    "x10      "
  };

  const char* e2s_tx_osr_mode_enum[10] = {
    "x1  ",
    "x2  ",
    "x3  ",
    "x3p ",
    "x4  ",
    "x5  ",
    "x7p ",
    "x8  ",
    "x8p ",
    "x10 "
  };

  ENULL_MEMSET(&state, 0, sizeof(merlin_mptwo_lane_state_st));

  EFUN(_merlin_mptwo_read_lane_state(ma,&state));

  lane_idx = merlin_mptwo_get_lane(ma);
  EFUN_PRINTF(("%2d ", lane_idx));
  if(state.osr_mode.tx_rx != 255) {
      char *s;
      s = (char *)e2s_osr_mode_enum[state.osr_mode.tx_rx];
      EFUN_PRINTF(("(%2s%s, 0x%04x,", (state.br_pd_en) ? "BR" : "OS", s, state.ucv_config));  
  } else {
      char *s;
      char *r;
      s = (char *)e2s_tx_osr_mode_enum[state.osr_mode.tx];
      r = (char *)e2s_tx_osr_mode_enum[state.osr_mode.rx];
      EFUN_PRINTF(("(%2s%s:%s, 0x%04x,", (state.br_pd_en) ? "BR" : "OS", s,r, state.ucv_config));  
  }
  if (state.tx_pll_select != 255)
      EFUN_PRINTF(("   %01x, %01x,%1d,%1d) ",state.reset_state,state.stop_state,state.tx_pll_select,state.rx_pll_select));
  else
      EFUN_PRINTF(("   %01x, %01x) ",state.reset_state,state.stop_state));
  if (state.sig_det_chg) {
    EFUN_PRINTF((" %1d*", state.sig_det));
  }
  else {
    EFUN_PRINTF((" %1d ", state.sig_det));
  }
  if (state.rx_lock_chg) {
    EFUN_PRINTF(("  %1d*", state.rx_lock));
  }
  else {
    EFUN_PRINTF(("  %1d ", state.rx_lock));
  }
  EFUN_PRINTF((" %4d ", state.rx_ppm));
  EFUN_PRINTF(("  %3d   %3d ", state.clk90, state.clkp1));
  EFUN_PRINTF(("  %2d,%1d ", state.pf_main, state.pf2_ctrl));
  EFUN_PRINTF(("   %2d ", state.vga));
  EFUN_PRINTF(("%3d ", state.dc_offset));
  EFUN_PRINTF(("%4d ", state.p1_lvl));
  EFUN_PRINTF(("%3d,%3d,%3d,%3d,%3d,%3d,%3d ", state.dfe1, state.dfe2, state.dfe3, state.dfe4, state.dfe5, state.dfe1_dcd, state.dfe2_dcd));
  EFUN_PRINTF(("%3d,%3d,%3d,%3d,%3d,%3d  ", state.ze, state.zo, state.pe, state.po, state.me, state.mo));
  EFUN_PRINTF((" %4d ", state.tx_ppm));
  EFUN_PRINTF(("   %2d,%2d,%2d,%2d    ", state.txfir_pre, state.txfir_main, state.txfir_post1, state.txfir_post2));
  EFUN_PRINTF((" %3d,%3d,%3d,%3d ", state.heye_left, state.heye_right, state.veye_upper, state.veye_lower));
  EFUN_PRINTF((" %4d.%01d", state.link_time/10, state.link_time%10));
  return (ERR_CODE_NONE);
}


SDK_STATIC err_code_t _merlin_mptwo_read_core_state(merlin_access_t *ma, merlin_mptwo_core_state_st *istate) {
  merlin_mptwo_core_state_st state;
  struct merlin_mptwo_uc_core_config_st core_cfg;

  ENULL_MEMSET(&state, 0, sizeof(merlin_mptwo_core_state_st));
  ENULL_MEMSET(&core_cfg, 0, sizeof(struct merlin_mptwo_uc_core_config_st));

  if (!istate)
    return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));

  EFUN(merlin_mptwo_get_uc_core_config(ma,&core_cfg));
  ESTM(state.rescal               = rd_ams_tx_rescal());
  ESTM(state.core_reset           = rdc_core_dp_reset_state());
  ESTM(state.pll_pwrdn            = rdc_afe_s_pll_pwrdn());
  ESTM(state.uc_active            = rdc_uc_active());
  ESTM(state.comclk_mhz           = rdc_heartbeat_count_1us());
  ESTM(state.ucode_version        = rdcv_common_ucode_version());
  ESTM(state.ucode_minor_version  = rdcv_common_ucode_minor_version());
  ESTM(state.afe_hardware_version = rdcv_afe_hardware_version());
  ESTM(state.temp_idx             = rdcv_temp_idx());
  {  int16_t                           die_temp = 0;
     EFUN(merlin_mptwo_read_die_temperature(ma,&die_temp));
     state.die_temp               =    die_temp;
  }
  ESTM(state.avg_tmon             = rdcv_avg_tmon_reg13bit());
  state.vco_rate_mhz            = (uint16_t)core_cfg.vco_rate_in_Mhz;
  ESTM(state.analog_vco_range     = rdc_pll_range());
  ESTM(state.pll_div              = rdc_ams_pll_div());
  EFUN(_merlin_mptwo_pll_lock_status(ma,&state.pll_lock, &state.pll_lock_chg));
  ESTM(state.core_status          = rdcv_status_byte());

  *istate = state;
  return (ERR_CODE_NONE);
}

static err_code_t _merlin_mptwo_display_core_state_no_newline(merlin_access_t *ma) {
    merlin_mptwo_core_state_st state;
    ENULL_MEMSET(&state     , 0, sizeof(state     ));
        EFUN(_merlin_mptwo_read_core_state(ma,&state));
    EFUN_PRINTF(("%02d  "              ,  merlin_mptwo_get_core(ma)));
    EFUN_PRINTF((" %x "                 ,  state.core_reset));
    EFUN_PRINTF(("  %02x  "           ,  state.core_status));
    EFUN_PRINTF(("    %1d     "        ,  state.pll_pwrdn));
    EFUN_PRINTF(("   %1d    "          ,  state.uc_active));
    EFUN_PRINTF((" %3d.%02dMHz"         , (state.comclk_mhz/4),((state.comclk_mhz%4)*25)));    /* comclk in Mhz = heartbeat_count_1us / 4 */
    EFUN_PRINTF(("   %4X_%02X "        ,  state.ucode_version, state.ucode_minor_version));
    EFUN_PRINTF(("    0x%02x   "       ,  state.afe_hardware_version));
    EFUN_PRINTF(("   %3dC   "          ,  state.die_temp));
    EFUN_PRINTF(("   (%02d)%3dC "      ,  state.temp_idx, _bin_to_degC((state.avg_tmon>>3))));
    EFUN_PRINTF(("   0x%02x  "         ,  state.rescal));
    EFUN_PRINTF(("  %2d.%2dGHz ",state.vco_rate_mhz/1000, state.vco_rate_mhz % 1000));
    EFUN_PRINTF(("    %03d       "     ,  state.analog_vco_range));
    EFUN_PRINTF(("(%02d)"              ,  state.pll_div));
    EFUN(_merlin_mptwo_display_pll_to_divider(ma,state.pll_div));
    if (state.pll_lock_chg) {
      EFUN_PRINTF(("   %01d*  "        ,  state.pll_lock));
    }
    else {
      EFUN_PRINTF(("   %01d   "        ,  state.pll_lock));
    }

    return (ERR_CODE_NONE);
}

SDK_STATIC err_code_t _merlin_mptwo_pll_lock_status(merlin_access_t *ma, uint8_t *pll_lock, uint8_t *pll_lock_chg) {

    uint16_t rddata;
    EFUN(merlin_mptwo_pmd_rdt_reg(ma,PLL_STATUS_ADDR, &rddata));

    *pll_lock     = ((rddata >> 9) & 0x1);
    *pll_lock_chg = ((rddata >> 8) & 0x1);

    return(ERR_CODE_NONE);
}

SDK_STATIC err_code_t _merlin_mptwo_pmd_lock_status(merlin_access_t *ma, uint8_t *pmd_lock, uint8_t *pmd_lock_chg) {

    uint16_t rddata;
    EFUN(merlin_mptwo_pmd_rdt_reg(ma,PMD_LOCK_STATUS_ADDR, &rddata));

    *pmd_lock     = (rddata & 0x1);
    *pmd_lock_chg = ((rddata >> 1) & 0x1);

    return(ERR_CODE_NONE);
}

SDK_STATIC err_code_t _merlin_mptwo_sigdet_status(merlin_access_t *ma, uint8_t *sig_det, uint8_t *sig_det_chg) {

    uint16_t rddata;
    EFUN(merlin_mptwo_pmd_rdt_reg(ma,SIGDET_STATUS_ADDR, &rddata));

    *sig_det     = (rddata & 0x1);
    *sig_det_chg = ((rddata >> 1) & 0x1);

    return(ERR_CODE_NONE);
}


/* returns 000111 (7 = 8-1), for n = 3  */
#define BFMASK(n) ((1<<((n)))-1)

static err_code_t _update_uc_lane_config_st(struct  merlin_mptwo_uc_lane_config_st *st) {
  uint16_t in = st->word;
  st->field.lane_cfg_from_pcs = in & BFMASK(1); in >>= 1;
  st->field.an_enabled = in & BFMASK(1); in >>= 1;
  st->field.dfe_on = in & BFMASK(1); in >>= 1;
  st->field.force_brdfe_on = in & BFMASK(1); in >>= 1;
  st->field.media_type = in & BFMASK(2); in >>= 2;
  st->field.unreliable_los = in & BFMASK(1); in >>= 1;
  st->field.scrambling_dis = in & BFMASK(1); in >>= 1;
  st->field.cl72_auto_polarity_en = in & BFMASK(1); in >>= 1;
  st->field.cl72_restart_timeout_en = in & BFMASK(1); in >>= 1;
  st->field.reserved = in & BFMASK(6); in >>= 6;
  return(ERR_CODE_NONE);
}

static err_code_t _update_usr_ctrl_disable_functions_st(struct merlin_mptwo_usr_ctrl_disable_functions_st *st) {
  uint8_t in = st->byte;
  st->field.pf_adaptation = in & BFMASK(1); in >>= 1;
  st->field.dc_adaptation = in & BFMASK(1); in >>= 1;
  st->field.vga_adaptation = in & BFMASK(1); in >>= 1;
  st->field.slicer_offset_tuning = in & BFMASK(1); in >>= 1;
  st->field.clk90_offset_adaptation = in & BFMASK(1); in >>= 1;
  st->field.p1_level_tuning = in & BFMASK(1); in >>= 1;
  st->field.eye_adaptation = in & BFMASK(1); in >>= 1;
  st->field.all_adaptation = in & BFMASK(1); in >>= 1;
  return ERR_CODE_NONE;
}


static err_code_t _update_usr_ctrl_disable_dfe_functions_st(struct merlin_mptwo_usr_ctrl_disable_dfe_functions_st *st) {
  uint8_t in = st->byte;
  st->field.dfe_tap1_adaptation = in & BFMASK(1); in >>= 1;
  st->field.dfe_tap2_adaptation = in & BFMASK(1); in >>= 1;
  st->field.dfe_tap3_adaptation = in & BFMASK(1); in >>= 1;
  st->field.dfe_tap4_adaptation = in & BFMASK(1); in >>= 1;
  st->field.dfe_tap5_adaptation = in & BFMASK(1); in >>= 1;
  st->field.dfe_tap1_dcd = in & BFMASK(1); in >>= 1;
  st->field.dfe_tap2_dcd = in & BFMASK(1); in >>= 1;
  return ERR_CODE_NONE;
}

/* word to fields, for display */
static err_code_t _update_uc_core_config_st(struct merlin_mptwo_uc_core_config_st *st) {
  uint16_t in = st->word;
  st->field.core_cfg_from_pcs = in & BFMASK(1); in >>= 1;
  st->field.vco_rate = in & BFMASK(5); in >>= 5;
  st->field.reserved1 = in & BFMASK(2); in >>= 2;
  st->field.reserved2 = in & BFMASK(8); in >>= 8;
  st->vco_rate_in_Mhz = VCO_RATE_TO_MHZ(st->field.vco_rate);
  return ERR_CODE_NONE;
}

/* fields to word, to write into uC RAM */
static err_code_t _update_uc_core_config_word(struct merlin_mptwo_uc_core_config_st *st) {
  uint16_t in = 0;
  in <<= 8; in |= 0/*st->field.reserved2*/ & BFMASK(8);
  in <<= 2; in |= 0/*st->field.reserved1*/ & BFMASK(2);
  in <<= 5; in |= st->field.vco_rate & BFMASK(5);
  in <<= 1; in |= st->field.core_cfg_from_pcs & BFMASK(1);
  st->word = in;
  return ERR_CODE_NONE;
}

static err_code_t _update_uc_lane_config_word(struct merlin_mptwo_uc_lane_config_st *st) {
  uint16_t in = 0;
  in <<= 6; in |= 0 /*st->field.reserved*/ & BFMASK(6);
  in <<= 1; in |= st->field.cl72_restart_timeout_en & BFMASK(1);
  in <<= 1; in |= st->field.cl72_auto_polarity_en & BFMASK(1);
  in <<= 1; in |= st->field.scrambling_dis & BFMASK(1);
  in <<= 1; in |= st->field.unreliable_los & BFMASK(1);
  in <<= 2; in |= st->field.media_type & BFMASK(2);
  in <<= 1; in |= st->field.force_brdfe_on & BFMASK(1);
  in <<= 1; in |= st->field.dfe_on & BFMASK(1);
  in <<= 1; in |= st->field.an_enabled & BFMASK(1);
  in <<= 1; in |= st->field.lane_cfg_from_pcs & BFMASK(1);
  st->word = in;
  return ERR_CODE_NONE;
}

static err_code_t _update_usr_ctrl_disable_functions_byte(struct merlin_mptwo_usr_ctrl_disable_functions_st *st) {
  uint8_t in = 0;
  in <<= 1; in |= st->field.all_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.eye_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.p1_level_tuning & BFMASK(1);
  in <<= 1; in |= st->field.clk90_offset_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.slicer_offset_tuning & BFMASK(1);
  in <<= 1; in |= st->field.vga_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.dc_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.pf_adaptation & BFMASK(1);
  st->byte = in;
  return ERR_CODE_NONE;
}


static err_code_t _update_usr_ctrl_disable_dfe_functions_byte(struct  merlin_mptwo_usr_ctrl_disable_dfe_functions_st *st) {
  uint8_t in = 0;
  in <<= 1; in |= st->field.dfe_tap2_dcd & BFMASK(1);
  in <<= 1; in |= st->field.dfe_tap1_dcd & BFMASK(1);
  in <<= 1; in |= st->field.dfe_tap5_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.dfe_tap4_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.dfe_tap3_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.dfe_tap2_adaptation & BFMASK(1);
  in <<= 1; in |= st->field.dfe_tap1_adaptation & BFMASK(1);
  st->byte = in;
  return ERR_CODE_NONE;
}

#undef BFMASK

SDK_STATIC err_code_t _trnsum_clear_and_enable(merlin_access_t *ma) {
    /*the trnsum accumulator is cleared on the rising edge of trnsum_en signal */
    /*this function creates a rising edge on the trnsum_en signal */
    EFUN(wr_trnsum_en(0));
    EFUN(wr_trnsum_en(1));
    EFUN(wr_uc_trnsum_en(1));
    return ERR_CODE_NONE;
}

SDK_STATIC err_code_t _check_uc_lane_stopped(merlin_access_t *ma) {

  uint8_t is_micro_stopped;
  ESTM(is_micro_stopped = rdv_usr_sts_micro_stopped());
  if (!is_micro_stopped) {
      return(_error(ERR_CODE_UC_NOT_STOPPED));
  } else {
      return(ERR_CODE_NONE);
  }
}

SDK_STATIC err_code_t _calc_patt_gen_mode_sel(uint8_t *mode_sel, uint8_t *zero_pad_len, uint8_t patt_length) {

  if(!mode_sel) {
          return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }
  if(!zero_pad_len) {
          return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  /* Select the correct Pattern generator Mode depending on Pattern length */
  if (!(140 % patt_length)) {
    *mode_sel = 6;
    *zero_pad_len = 100;
  }
  else if (!(160 % patt_length)) {
    *mode_sel = 5;
    *zero_pad_len = 80;
  }
  else if (!(180 % patt_length)) {
    *mode_sel = 4;
    *zero_pad_len = 60;
  }
  else if (!(200 % patt_length)) {
    *mode_sel = 3;
    *zero_pad_len = 40;
  }
  else if (!(220 % patt_length)) {
    *mode_sel = 2;
    *zero_pad_len = 20;
  }
  else if (!(240 % patt_length)) {
    *mode_sel = 1;
    *zero_pad_len = 0;
  } else {
    EFUN_PRINTF(("ERROR: Unsupported Pattern Length\n"));
    return (_error(ERR_CODE_CFG_PATT_INVALID_PATT_LENGTH));
  }
  return(ERR_CODE_NONE);
}

static err_code_t _merlin_mptwo_display_pll_to_divider(merlin_access_t *ma, uint8_t val) {

    { uint8_t mmd_en;
    ESTM(mmd_en = rdc_ams_pll_mmd_en());
    if(mmd_en) {
        int primary;long int secondary;int frac;
        ESTM(primary = rdc_ams_pll_i_ndiv_int());
        ESTM(secondary = rdc_ams_pll_i_ndiv_frac_h());
        ESTM(secondary = (secondary<<4) | rdc_ams_pll_i_ndiv_frac_l());
        frac = secondary/262; /* 2^18 * 1000 */
        EFUN_PRINTF(("%3d.%03d ",primary,frac));
    } else {
        switch(val) {
        case 0x1c:  EFUN_PRINTF((" 60     "));
            break;
        case 0x04:  EFUN_PRINTF((" 64     "));
            break;
        case 0x14:  EFUN_PRINTF((" 66     "));
            break;
        case 0x0c:  EFUN_PRINTF((" 80     "));
            break;
        case 0x09:  EFUN_PRINTF((" 54.4   "));
            break;
        case 0x1a:  EFUN_PRINTF(("187.5   "));
            break;
        default  :  EFUN_PRINTF((" xxxxxxx "));
            EFUN_PRINTF(("ERROR: Invalid PLL_DIV VALUE\n"));
            return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
        }
    }
    }

    return(ERR_CODE_NONE);

}

static err_code_t _merlin_mptwo_display_ber_scan_data(merlin_access_t *ma, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control) {
    uint8_t i,prbs_byte,prbs_multi,time_byte,time_multi;
    uint16_t sts,dataword;
    uint8_t verbose = 0;
    int16_t offset_start;
    uint8_t cnt;
    uint32_t errors,timer_values;
    int rate,direction;
    uint8_t range250;
    merlin_mptwo_osr_mode_st osr_mode;
    struct merlin_mptwo_uc_core_config_st core_config;

    ENULL_MEMSET(&core_config,0,sizeof(core_config));
    ENULL_MEMSET(&osr_mode,0,sizeof(osr_mode));

    EFUN(merlin_mptwo_get_uc_core_config(ma,&core_config));
    EFUN(_merlin_mptwo_get_osr_mode(ma,&osr_mode));

    if(osr_mode.rx > 2) {
        EFUN_PRINTF(("ERROR DIAG display_ber_data: osr mode too high\n"));
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    rate = core_config.vco_rate_in_Mhz/(1<<osr_mode.rx);
    direction = (ber_scan_mode & DIAG_BER_NEG) ? -1 : 1 ;
    range250 = (ber_scan_mode & DIAG_BER_P1_NARROW) ? 0 : 1;

    EFUN_PRINTF(("\n****  SERDES BER DATA    ****\n"));
    EFUN_PRINTF(("BER MODE = %x %d %d\n",ber_scan_mode,timer_control,max_error_control));
    EFUN_PRINTF(("DATA RATE = %d Mhz\n",rate));
    /* start UC acquisition */
    if(verbose > 2) EFUN_PRINTF(("start begin\n"));
    EFUN(merlin_mptwo_start_ber_scan_test(ma,ber_scan_mode, timer_control, max_error_control));
    ESTM(offset_start = rd_uc_dsc_data());
    if(ber_scan_mode & DIAG_BER_HORZ) {
        EFUN_PRINTF(("STARTING OFFSET = %d : %d mUI\n",offset_start,(offset_start*1000)>>6));
    } else {
        EFUN_PRINTF(("STARTING OFFSET = %d : %d mV\n",offset_start,_ladder_setting_to_mV(ma,(int8_t)offset_start, range250)));
    }
    if(verbose > 2) EFUN_PRINTF(("start done\n"));


    /* This wait is VERY LONG and should be replaced with interupt or something */
    if(verbose > 5) {
        do {
            EFUN(merlin_mptwo_delay_us(2000000));
            ESTM(sts = rdv_usr_diag_status());
            EFUN_PRINTF(("sts=%04x\n",sts));

        } while ((sts & 0x8000) == 0);
    } else {
        EFUN_PRINTF(("Waiting for measurement time approx %d seconds",timer_control+(timer_control>>1)));
        EFUN(merlin_mptwo_poll_diag_done(ma,&sts,timer_control*2000));
    }
    if(verbose > 2) EFUN_PRINTF(("delay done\n"));

    /* Check for completion read ln.diag_status byte?*/
    ESTM(sts = rdv_usr_diag_status());
    if((sts & 0x8000) == 0) {
        return(_error(ERR_CODE_DATA_NOTAVAIL));
    }
    cnt = (sts & 0x00FF)/3;
    for(i=0;i < cnt;i++) {
        /* Read 2 bytes of data */
        EFUN(merlin_mptwo_pmd_uc_cmd(ma,CMD_READ_DIAG_DATA_WORD, 0, 100));
        ESTM(dataword = rd_uc_dsc_data());           /* LSB contains 2 -4bit nibbles */
        time_byte = (uint8_t)(dataword>>8);    /* MSB is time byte */
        prbs_multi = (uint8_t)dataword & 0x0F; /* split nibbles */
        time_multi = (uint8_t)dataword>>4;
        /* Read 1 bytes of data */
        EFUN(merlin_mptwo_pmd_uc_cmd(ma,CMD_READ_DIAG_DATA_BYTE, 0, 100));
        ESTM(prbs_byte = (uint8_t)rd_uc_dsc_data());
        errors = _float12_to_uint32(prbs_byte,prbs_multi); /* convert 12bits to uint32 */
        timer_values = (_float12_to_uint32(time_byte,time_multi)<<3);
        if(verbose < 5) {
            if (!(i % 4))  {
                EFUN_PRINTF(("\n"));
            }
            if(ber_scan_mode & DIAG_BER_HORZ) {
                EFUN_PRINTF(("%d %d %d ",direction*(((_abs(offset_start)-i)*1000)>>6),errors,timer_values));
            } else {
                EFUN_PRINTF(("%d %d %d ",direction*_ladder_setting_to_mV(ma,(int8_t)_abs(offset_start)-i, range250),errors,timer_values));
            }

        } else {
            EFUN_PRINTF(("BER Errors=%d (%02x<<%d): Time=%d (%02x<<%d)\n",errors,prbs_byte,prbs_multi,timer_values,time_byte,time_multi<<3));
        }
        /*if(timer_values == 0 && errors == 0) break;*/
    }
    EFUN_PRINTF(("\n"));
    EFUN(merlin_mptwo_pmd_uc_cmd(ma,CMD_CAPTURE_BER_END,0x00,2000));

  return(ERR_CODE_NONE);
}
