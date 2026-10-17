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


/***********************************************************************************
 ***********************************************************************************
 *  File Name     :  merlin8_phy1_internal.c                                         *
 *  Created On    :  13/02/2014                                                    *
 *  Created By    :  Justin Gaither                                                *
 *  Description   :  Internal APIs for Serdes IPs                                  *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/

#ifndef EXCLUDE_STD_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#endif

#include "merlin8_phy1_internal.h"
#include "merlin8_phy1_internal_error.h"
#include "merlin8_phy1_access.h"
#include "merlin8_phy1_common.h"
#include "merlin8_phy1_config.h"
#include "merlin8_phy1_functions.h"
#include "merlin8_phy1_select_defns.h"
#include "merlin8_phy1_debug_functions.h"
#include "merlin8_phy1_decode_print.h"
#include "merlin8_phy1_diag.h"


/*! @file
 *
 */



#ifndef SMALL_FOOTPRINT
#ifndef PCIEG3_V1
#ifndef UINT16_MAX
#define UINT16_MAX 0xFFFF
#endif
uint32_t merlin8_phy1_INTERNAL_mult_with_overflow_check(uint32_t a, uint32_t b, uint8_t *of) {
    uint16_t c,d;
    uint32_t r,s;
    if (a > b) return merlin8_phy1_INTERNAL_mult_with_overflow_check(b, a, of);
    *of = 0;
    c = (uint16_t)(b >> 16);
    d = UINT16_MAX & b;
    r = a * c;
    s = a * d;
    if (r > UINT16_MAX) *of = 1;
    r <<= 16;
    return (s + r);
}
#endif /* MERLIN7_PCIEG3 */
#endif /* SMALL_FOOTPRINT */

#if !defined(SERDES_EXTERNAL_INFO_TABLE_EN)
static srds_info_t merlin8_phy1_info;
#endif


srds_info_t *merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr(srds_access_t *sa__) {
#if defined(SERDES_EXTERNAL_INFO_TABLE_EN)
    return merlin8_phy1_acc_get_info_table_address(sa__);
#else
    UNUSED(sa__);
    return &merlin8_phy1_info;
#endif
}

srds_info_t *merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(srds_access_t *sa__) {
    err_code_t err_code = ERR_CODE_NONE;
    srds_info_t * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr(sa__);
    if (merlin8_phy1_info_ptr->signature != SRDS_INFO_SIGNATURE) {
        err_code = merlin8_phy1_init_merlin8_phy1_info(sa__);
    }
    if (err_code != ERR_CODE_NONE) {
        EFUN_PRINTF(("ERROR: Serdes Info pointer not initialized correctly\n"));
        return (srds_info_t *)NULL;
    }
    return merlin8_phy1_info_ptr;
}

err_code_t merlin8_phy1_INTERNAL_match_ucode_from_info(srds_access_t *sa__, srds_info_t const *merlin8_phy1_info_ptr) {
    INIT_SRDS_ERR_CODE
    uint16_t ucode_version_major;
    uint8_t ucode_version_minor;
    uint32_t ucode_version;

    if (merlin8_phy1_info_ptr == NULL) {
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    ESTM(ucode_version_major = rdcv_common_ucode_version());
    ESTM(ucode_version_minor = rdcv_common_ucode_minor_version());
    ucode_version = (uint32_t)((ucode_version_major << 8) | ucode_version_minor);

    if (ucode_version == merlin8_phy1_info_ptr->ucode_version) {
        return(ERR_CODE_NONE);
    } else {
        EFUN_PRINTF(("ERROR:  ucode version of the current thread not matching with stored merlin8_phy1_info->ucode_version, Expected 0x%08X, but received 0x%08X.\n",
                    merlin8_phy1_info_ptr->ucode_version, ucode_version));
        return(ERR_CODE_UCODE_VERIFY_FAIL);
    }
}

/* This function is getting deprecated.
   Please use merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check and merlin8_phy1_INTERNAL_match_ucode_from_info instead */
err_code_t merlin8_phy1_INTERNAL_verify_merlin8_phy1_info(srds_access_t *sa__, srds_info_t const *merlin8_phy1_info_ptr) {
    err_code_t err_code = ERR_CODE_NONE;

    if (merlin8_phy1_info_ptr == NULL) {
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    if (merlin8_phy1_info_ptr->signature != SRDS_INFO_SIGNATURE) {
        EFUN_PRINTF(("ERROR:  Mismatch in merlin8_phy1_info signature.  Expected 0x%08X, but received 0x%08X.\n",
                    SRDS_INFO_SIGNATURE, merlin8_phy1_info_ptr->signature));
        return (merlin8_phy1_error(sa__, ERR_CODE_INFO_TABLE_ERROR));
    }
    err_code = merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr);

    if (err_code != ERR_CODE_NONE) {
        /* ucode version mismatch */
        EFUN_PRINTF(("ERROR:  merlin8_phy1_INTERNAL_match_ucode_from_info returned %d\n",err_code));
        return(ERR_CODE_MICRO_INIT_NOT_DONE);
    }
    return (ERR_CODE_NONE);
}

#ifndef SMALL_FOOTPRINT
/* Store a cached AFE version for re-use */
err_code_t merlin8_phy1_INTERNAL_get_afe_hw_version(srds_access_t *sa__, uint8_t *afe_hw_version) {
  INIT_SRDS_ERR_CODE
  static uint8_t _cached_afe_hw_version = 255;

  if (!afe_hw_version)
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  if (_cached_afe_hw_version == 255)
    ESTM(_cached_afe_hw_version = rdcv_afe_hardware_version());
  *afe_hw_version = _cached_afe_hw_version;
  return(ERR_CODE_NONE);
}

/* Timestamp check to see if heartbeat timer is programmed correctly for the COMCLK frequency it is running at */
err_code_t merlin8_phy1_INTERNAL_test_uc_timestamp_with_print_options(srds_access_t *sa__, uint8_t console_print_options) {
#ifdef SERDES_API_FLOATING_POINT
    INIT_SRDS_ERR_CODE
    uint16_t time = 0;
    uint16_t time1 = 0;
    uint16_t time2 = 0;
    int        tm0;
    USR_DOUBLE resolution, tmElapsed;

    const USR_DOUBLE max_resolution=11.0;
    const USR_DOUBLE min_resolution=9.0;

    tm0 = 0;
    EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_UC_DBG, CMD_UC_DBG_TIMESTAMP, 100));
    ESTM(time1=rd_uc_dsc_data());
    do {
      EFUN(USR_DELAY_MS(10));
      EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_UC_DBG, CMD_UC_DBG_TIMESTAMP, 100));
      ESTM(time2=rd_uc_dsc_data());
      time=(uint16_t)(time2-time1);
      if (0 == time)
        break;
      tm0 += 10;
    } while (time < 40000);
    tmElapsed = tm0 * 1000;
    if(time == 0) {
        EFUN_PRINTF(("%s ERROR : Diag  timestamp fail, start = 0x%04x, end = 0x%04x\n", API_FUNCTION_NAME, time1, time2));
        return (ERR_CODE_DIAG_TIMESTAMP_FAIL);
    }
    resolution=tmElapsed/(double)time;
    EFUN_PRINTF(("UcTimer: hostTime=%lf ucTime=%d Resolution: %.1fus/count\n", tmElapsed, time, resolution));
    if((resolution<min_resolution)||(resolution>max_resolution)){
        if (console_print_options) {
            EFUN_PRINTF(("\nERROR : Lane %i:\tuC timestamp: %i\t\tResolution: %.1fus/count. Passing resolution limits:\tMax: %.1fus/count\tMin: %.1fus/count\n",merlin8_phy1_acc_get_lane(sa__),time,resolution,max_resolution,min_resolution));
        }
        return(ERR_CODE_DIAG_TIMESTAMP_FAIL);
    }
    else {
        if (console_print_options) {
            EFUN_PRINTF(("\nPassed timestamp check : Lane %i:\tuC timestamp: %i\t\tResolution: %.1fus/count. Passing resolution limits:\tMax: %.1fus/count\tMin: %.1fus/count\n",merlin8_phy1_acc_get_lane(sa__),time,resolution,max_resolution,min_resolution));
        }
   }
#else
    USR_PRINTF(("%s : This function needs SERDES_API_FLOATING_POINT define to operate \n", API_FUNCTION_NAME));
#endif
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_num_bits_per_ms(srds_access_t *sa__, uint8_t select_rx, uint32_t *num_bits_per_ms) {
    INIT_SRDS_ERR_CODE
    uint8_t osr_mode = 0;
    uint8_t pll_select = 0, prev_pll = 0;
    struct merlin8_phy1_uc_core_config_st core_config = UC_CORE_CONFIG_INIT;
    merlin8_phy1_osr_mode_st osr_mode_st;
    ENULL_MEMSET(&osr_mode_st, 0, sizeof(merlin8_phy1_osr_mode_st));

    {
        EFUN(merlin8_phy1_INTERNAL_get_osr_mode(sa__, &osr_mode_st));
        if (select_rx) {
            osr_mode = osr_mode_st.rx;
        } else {
            osr_mode = osr_mode_st.tx;
        }
    }


    ESTM(prev_pll = merlin8_phy1_acc_get_pll_idx(sa__));
    pll_select = 0;
    EFUN(merlin8_phy1_acc_set_pll_idx(sa__,pll_select));

#ifndef PCIEG3_V1
        EFUN(merlin8_phy1_get_uc_core_config(sa__, &core_config));
#else
        {
            int32_t rate_sel;
            ESTM(rate_sel = rd_rate_sel());
            switch (rate_sel) {
            case 0: core_config.vco_rate_in_Mhz = 2500; break;
            case 1: core_config.vco_rate_in_Mhz = 5000; break;
            case 2: core_config.vco_rate_in_Mhz = 8000; break;
            default: return ERR_CODE_CONFLICTING_PARAMETERS;
            }
        }
#endif
        *num_bits_per_ms = (uint32_t)((((uint64_t)core_config.vco_rate_in_Mhz * 100000UL) / merlin8_phy1_osr_mode_enum_to_int_x1000(osr_mode))*10);

    EFUN(merlin8_phy1_acc_set_pll_idx(sa__,prev_pll));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_display_BER(srds_access_t *sa__, uint16_t time_ms) {
    INIT_SRDS_ERR_CODE
    char string[SRDS_MAX_BER_STR_LEN]="\0";
      EFUN(merlin8_phy1_INTERNAL_get_BER_string(sa__,time_ms,string, sizeof(string)));
      USR_PRINTF(("%s",string));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_BER_string(srds_access_t *sa__, uint16_t time_ms, char *string, uint8_t string_size) {
    INIT_SRDS_ERR_CODE
    char string2[4];
    char string_extra[3] = "";
    uint8_t chk_en = 0;
    struct merlin8_phy1_ber_data_st ber_data_local = BER_DATA_ST_INIT;
    err_code_t err = ERR_CODE_NONE;

    if(string == NULL) {
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    ESTM(chk_en = rd_prbs_chk_en());
    if(chk_en == 0) {
        USR_SNPRINTF(string, (size_t)(string_size), " !chk_en ");
        return(ERR_CODE_NONE);
    }
    {
        enum merlin8_phy1_prbs_chk_timer_selection_enum timer_sel = USE_HW_TIMERS;

        err = merlin8_phy1_INTERNAL_get_BER_data(sa__, time_ms, &ber_data_local, timer_sel);
    }

    if(err == ERR_CODE_RX_CLKGATE_FRC_ON) {
        USR_SNPRINTF(string, (size_t)(string_size), "clk_gated");
        return(ERR_CODE_NONE);
    }
    else if(err == ERR_CODE_NO_PMD_RX_LOCK) {
        USR_SNPRINTF(string, (size_t)(string_size), "!pmd_lock");
        return(ERR_CODE_NONE);
    }
    else if(err == ERR_CODE_PRBS_CHK_HW_TIMERS_NOT_EXPIRED) {
        USR_SNPRINTF(string, (size_t)(string_size), "!time_exp");
        return(ERR_CODE_NONE);
    }
    else if(err != ERR_CODE_NONE) {
        return merlin8_phy1_error(sa__, err);
    }

    if((ber_data_local.num_errs < 3) && (ber_data_local.lcklost == 0)) {   /* lcklost = 0 */
        USR_SNPRINTF(string2, sizeof(string2), " <");
        ber_data_local.num_errs = 3;
    } else {
        USR_SNPRINTF(string2, sizeof(string2), "  ");
    }
    if(ber_data_local.cdrlcklost) {
        USR_SNPRINTF(string_extra, sizeof(string_extra), "*");
    }
    if(ber_data_local.lcklost == 1) {    /* lcklost = 1 */
        {
            USR_SNPRINTF(string, (size_t)(string_size), "  !Lock  ");
        }
    }
    else {                    /* lcklost = 0 */
        uint16_t x=0,y=0,z=0,srds_div;

        if(ber_data_local.num_errs < ber_data_local.num_bits) {
            while(1) {
                srds_div = (uint16_t)(((ber_data_local.num_errs<<1) + ber_data_local.num_bits)/(ber_data_local.num_bits<<1));
                if(srds_div>=10) break;
                ber_data_local.num_errs = ber_data_local.num_errs*10;
                z++;
            }
            if(srds_div>=100) {
                srds_div = srds_div/10;
                /* coverity[overflow_const] */
                z--;
            }
            x=srds_div/10;
            y = (uint16_t)(srds_div - 10*x);
            /* coverity[overflow_const] */
            z--;
            USR_SNPRINTF(string, (size_t)(string_size), "%s%d.%1de-%02d%s",string2,x,y,z,string_extra);
        }

    }
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_BER_data(srds_access_t *sa__, uint16_t time_ms, struct merlin8_phy1_ber_data_st *ber_data, enum merlin8_phy1_prbs_chk_timer_selection_enum timer_sel) {
    INIT_SRDS_ERR_CODE
    uint8_t  lcklost = 0;
    uint32_t err_cnt= 0;

    {
        uint8_t clk_gate = 0;
        UNUSED(timer_sel);
        ESTM(ber_data->prbs_chk_en = rd_prbs_chk_en());
        ESTM(clk_gate = rd_ln_rx_s_clkgate_frc_on());
        if(ber_data->prbs_chk_en == 0) {
            return(ERR_CODE_PRBS_CHK_DISABLED);
        }
        else if (clk_gate == 1) {
            return(ERR_CODE_RX_CLKGATE_FRC_ON);
        }
        EFUN(merlin8_phy1_prbs_err_count_state(sa__, &err_cnt, &lcklost)); /* clear error counters */
        EFUN(USR_DELAY_MS(time_ms));
        EFUN(merlin8_phy1_prbs_err_count_state(sa__, &err_cnt, &lcklost));
        ber_data->lcklost = lcklost;
        if(ber_data->lcklost == 0) {
            uint32_t num_bits_per_ms=0;
            EFUN(merlin8_phy1_INTERNAL_get_num_bits_per_ms(sa__, 1, &num_bits_per_ms));
            ber_data->num_bits = (uint64_t)num_bits_per_ms*(uint64_t)time_ms;
            ber_data->num_errs = err_cnt;
            ESTM(lcklost = rd_prbs_chk_lock_lost_lh()); /* Clear sticky bit */
            UNUSED(lcklost);
        }

    }

    return(ERR_CODE_NONE);
}

/* Check CDR lost lock */
err_code_t merlin8_phy1_INTERNAL_prbs_chk_cdr_lock_lost(srds_access_t *sa__, uint8_t *cdrlcklost) {
    INIT_SRDS_ERR_CODE
    uint32_t err_cnt, num_bits_per_ms = 0, num_bits;
    uint8_t lcklost;
    uint8_t time_ms = 1;
    if(!cdrlcklost) {
        return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    EFUN(merlin8_phy1_prbs_err_count_state(sa__, &err_cnt,&lcklost)); /* clear error counters */
    EFUN(USR_DELAY_MS(time_ms));
    EFUN(merlin8_phy1_prbs_err_count_state(sa__, &err_cnt,&lcklost));
    EFUN(merlin8_phy1_INTERNAL_get_num_bits_per_ms(sa__, 1, &num_bits_per_ms));
    num_bits = num_bits_per_ms*time_ms;

    /* check for BER>0.25 */
    *cdrlcklost = lcklost || (err_cnt>(num_bits>>2));

    return ERR_CODE_NONE;
}



err_code_t merlin8_phy1_INTERNAL_get_p1_threshold(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
    if (val == NULL) {
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
#ifdef USE_P1_VREF_SIGN_INV_HW_FIX
    // Don't need to invert sign when HW fix is enabled
    ESTM(*val = (int8_t)(rd_p1_eyediag_bin()));
#else
    ESTM(*val = (int8_t)(-rd_p1_eyediag_bin()));
#endif
    return (ERR_CODE_NONE);
}



err_code_t merlin8_phy1_INTERNAL_ladder_setting_to_mV(srds_access_t *sa__, int16_t ctrl, uint8_t range_250, afe_override_slicer_sel_t slicer_sel, int16_t *nlmv_val) {
    uint16_t absv;                                    /* Absolute value of ctrl */
    int16_t nlmv=0;                                     /* Non-linear value */
    UNUSED(sa__);
    UNUSED(slicer_sel);
    if (nlmv_val == NULL) {
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }

    /* Get absolute value */
    absv = (uint16_t)(SRDS_ABS(ctrl));

    {
        if (range_250) {
            /* 28nm range is 250mV, but 16nm range is 200mV */
            /* 200mV range, 6.5519mV linear units */
            nlmv = (int16_t)((absv*190+29/2)/29);
        }
        else {
            /* 120mV range, 3.8854mV linear units */
            nlmv = (int16_t)((absv*136+35/2)/35);
        }
    }
    /* Add sign and return */
    *nlmv_val=(int16_t)((ctrl>=0) ? nlmv : -nlmv);

    return ERR_CODE_NONE;
}


err_code_t merlin8_phy1_INTERNAL_compute_bin(srds_access_t *sa__, char var, char bin[], size_t bin_size) {
    if(!bin) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

  switch (var) {
    case '0':  USR_SNPRINTF(bin, bin_size, "0000");
               break;
    case '1':  USR_SNPRINTF(bin, bin_size, "0001");
               break;
    case '2':  USR_SNPRINTF(bin, bin_size, "0010");
               break;
    case '3':  USR_SNPRINTF(bin, bin_size, "0011");
               break;
    case '4':  USR_SNPRINTF(bin, bin_size, "0100");
               break;
    case '5':  USR_SNPRINTF(bin, bin_size, "0101");
               break;
    case '6':  USR_SNPRINTF(bin, bin_size, "0110");
               break;
    case '7':  USR_SNPRINTF(bin, bin_size, "0111");
               break;
    case '8':  USR_SNPRINTF(bin, bin_size, "1000");
               break;
    case '9':  USR_SNPRINTF(bin, bin_size, "1001");
               break;
    case 'a':
    case 'A':  USR_SNPRINTF(bin,bin_size,"1010");
               break;
    case 'b':
    case 'B':  USR_SNPRINTF(bin,bin_size,"1011");
               break;
    case 'c':
    case 'C':  USR_SNPRINTF(bin,bin_size,"1100");
               break;
    case 'd':
    case 'D':  USR_SNPRINTF(bin,bin_size,"1101");
               break;
    case 'e':
    case 'E':  USR_SNPRINTF(bin,bin_size,"1110");
               break;
    case 'f':
    case 'F':  USR_SNPRINTF(bin,bin_size,"1111");
               break;
    case '_': bin[0] = '\0';
              break;
    default : bin[0] = '\0';
               USR_PRINTF(("ERROR: Invalid Hexadecimal Pattern\n"));
               return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_INVALID_HEX));
  }
  return (ERR_CODE_NONE);
}


err_code_t merlin8_phy1_INTERNAL_compute_hex(srds_access_t *sa__, char bin[],uint8_t *hex) {
  if(!hex || !bin) {
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
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
    return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_INVALID_BIN2HEX));
  }
  return (ERR_CODE_NONE);
}

#define MICRO_STOP_BITS  (0x7F)


uint8_t merlin8_phy1_INTERNAL_stop_micro(srds_access_t *sa__, uint8_t graceful, err_code_t *err_code_p) {
   INIT_SRDS_ERR_CODE
   uint8_t stop_state = 0;

   if(!err_code_p) {
       return(0xFF);
   }

   /* Log current micro stop status */
   EPSTM2((stop_state = rdv_usr_sts_micro_stopped()),0xFF);

   /* Stop micro only if micro is not stopped currently */
   if (!(stop_state & MICRO_STOP_BITS)) {
       if (graceful) {
           EPFUN2((merlin8_phy1_stop_rx_adaptation(sa__, 1)),0xFF);
       }
       else {
           EPFUN2((merlin8_phy1_pmd_uc_control(sa__, CMD_UC_CTRL_STOP_IMMEDIATE,GRACEFUL_STOP_TIME)),0xFF);
       }
   }

   /* Return the previous micro stop status */
   return(stop_state);
}




/********************************************************/
/*  Global RAM access through Micro Register Interface  */
/********************************************************/
/* Micro Global RAM Byte Read */
uint8_t merlin8_phy1_INTERNAL_rdb_uc_var(srds_access_t *sa__, err_code_t *err_code_p, uint32_t addr) {
    INIT_SRDS_ERR_CODE
    uint8_t rddata;

    if(!err_code_p) {
        return(0);
    }
    EPSTM(rddata = merlin8_phy1_rdb_uc_ram(sa__, err_code_p, addr)); /* Use Micro register interface for reading RAM */
    return (rddata);
}

/* Micro Global RAM Word Read */
uint16_t merlin8_phy1_INTERNAL_rdw_uc_var(srds_access_t *sa__, err_code_t *err_code_p, uint32_t addr) {
  uint16_t rddata;
  INIT_SRDS_ERR_CODE

  if(!err_code_p) {
      return(0);
  }
  if (addr%2 != 0) {                                         /* Validate even address */
      *err_code_p = ERR_CODE_INVALID_RAM_ADDR;
      USR_PRINTF(("Error incorrect addr x%04x\n",addr));
      return (0);
  }
  EPSTM(rddata = merlin8_phy1_rdw_uc_ram(sa__, err_code_p, addr)); /* Use Micro register interface for reading RAM */
  return (rddata);
}

/* Micro Global RAM Byte Write  */
err_code_t merlin8_phy1_INTERNAL_wrb_uc_var(srds_access_t *sa__, uint32_t addr, uint8_t wr_val) {

    return (merlin8_phy1_wrb_uc_ram(sa__, addr, wr_val));          /* Use Micro register interface for writing RAM */
}

/* Micro Global RAM Word Write  */
err_code_t merlin8_phy1_INTERNAL_wrw_uc_var(srds_access_t *sa__, uint32_t addr, uint16_t wr_val) {
    if (addr%2 != 0) {                                       /* Validate even address */
      USR_PRINTF(("Error incorrect addr x%04x\n",addr));
        return (merlin8_phy1_error(sa__, ERR_CODE_INVALID_RAM_ADDR));
    }
    return (merlin8_phy1_wrw_uc_ram(sa__, addr, wr_val));          /* Use Micro register interface for writing RAM */
}


/***********************/
/*  Event Log Display  */
/***********************/
err_code_t merlin8_phy1_INTERNAL_event_log_dump_callback(srds_access_t *sa__, void *arg, uint8_t byte_count, uint16_t data) {
    merlin8_phy1_INTERNAL_event_log_dump_state_t * const state_ptr = (merlin8_phy1_INTERNAL_event_log_dump_state_t *)arg;
    const uint8_t bytes_per_row=16;

    /* when detecting more than 256 bytes of consequtive 0 in data,
     * terminate the printing to the log
     */
    if (state_ptr->zero_cnt == 0xffff) {
        return (ERR_CODE_NONE);
    }
    if (data == 0) {
        state_ptr->zero_cnt++;
    }
    else {
        state_ptr->zero_cnt = 0;
    }
    if (byte_count == 0) {
        if (state_ptr->line_start_index != state_ptr->index) {
            EFUN_PRINTF(("%*s    %d\n", 4*(bytes_per_row - state_ptr->index + state_ptr->line_start_index), "", state_ptr->line_start_index));
        }
        return (ERR_CODE_NONE);
    }
    if (byte_count == 1) {
        /* There is a trailing byte in the event log.
         * The simplest way to handle it is to print out a whole word, but mask
         * the invalid upper byte.
         */
        data &= 0xFF;
    }
    EFUN_PRINTF(("  0x%04x", ((data & 0xFF) << 8) | (data >> 8)));
    state_ptr->index = (uint16_t)(state_ptr->index + 2);
    if (state_ptr->index % bytes_per_row == 0) {
        EFUN_PRINTF(("    %d\n", state_ptr->line_start_index));
        state_ptr->line_start_index = state_ptr->index;
        /* 128 words/256 bytes is the largest event data section.
         * 0xffff is used as a flag to terminate the printing to the logfile
         */
        if (state_ptr->zero_cnt > 128) {
            state_ptr->zero_cnt = 0xffff;
        }
    }

    return(ERR_CODE_NONE);
}



err_code_t merlin8_phy1_INTERNAL_read_event_log_with_callback(srds_access_t *sa__,
                                                        uint8_t micro_num,
                                                        uint8_t bypass_micro,
                                                        void *arg,
                                                        err_code_t (*callback)(srds_access_t *, void *, uint8_t, uint16_t)) {
    INIT_SRDS_ERR_CODE

    srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
    uint16_t rd_idx, rd_idx0 = 0;
    int      rd_size;
    uint8_t current_lane;


    EFUN(merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr));

    if (micro_num >= merlin8_phy1_info_ptr->micro_count) {
        return (merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
    /* Read Current lane so that it can be restored at the end of function */
    current_lane = merlin8_phy1_acc_get_lane(sa__);
    if (0 == (bypass_micro & 2)) {
        EFUN_PRINTF(("\n\n********************************************\n"));
        EFUN_PRINTF((    "**** SERDES UC TRACE MEMORY DUMP ***********\n"));
        EFUN_PRINTF((    "********************************************\n"));
    }
    rd_size = (int) merlin8_phy1_info_ptr->trace_mem_ram_size;
    if (bypass_micro & 1) {
        ESTM(rd_idx = rdcv_trace_mem_wr_idx());
        rd_idx0 = rd_idx;
        if (merlin8_phy1_info_ptr->trace_memory_concurrent_read) {
            uint16_t rd_end;
            ESTM(rd_end = rdcv_trace_mem_rd_idx());
            if (rd_end < merlin8_phy1_info_ptr->trace_mem_ram_size) {
              rd_size = (int) rd_end - (int) rd_idx;
              if (0 == rd_size)
                  return(ERR_CODE_NONE);
              if (rd_size < 0)
                  rd_size = ((int) merlin8_phy1_info_ptr->trace_mem_ram_size) + rd_size;
            }
            /* supporting only trace_memory_descending_writes */
            ++rd_idx;
            if (rd_idx >= merlin8_phy1_info_ptr->trace_mem_ram_size)
                rd_idx = 0;
        }
        else if (merlin8_phy1_info_ptr->trace_memory_descending_writes) {
            ++rd_idx;
            if (rd_idx >= merlin8_phy1_info_ptr->trace_mem_ram_size) {
                rd_idx = 0;
            }
        } else {
            if (rd_idx == 0) {
                rd_idx = (uint16_t)merlin8_phy1_info_ptr->trace_mem_ram_size;
            }
            --rd_idx;
        }
    } else {
        /* Start Read to stop uC logging and read the word at last event written by uC */
        EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_EVENT_LOG_READ, CMD_EVENT_LOG_READ_START, GRACEFUL_STOP_TIME));
        ESTM(rd_idx = rdcv_trace_mem_rd_idx());
    }

    if (0 == (bypass_micro & 2)) {
        EFUN_PRINTF(( "\n  DEBUG INFO: trace memory read index = 0x%04x\n", rd_idx));
        EFUN_PRINTF(("  DEBUG INFO: trace memory size = 0x%04x\n", merlin8_phy1_info_ptr->trace_mem_ram_size));
        EFUN_PRINTF(("  DEBUG INFO: bypass_micro = 0x%04x\n\n", bypass_micro));
    }

    if (merlin8_phy1_info_ptr->trace_memory_descending_writes) {
        /* Micro writes trace memory using descending addresses.
         * So read using ascending addresses using block read
         */

        EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                  merlin8_phy1_info_ptr->trace_mem_ram_base + (uint32_t)(merlin8_phy1_info_ptr->grp_ram_size*micro_num),
                                                  (uint16_t)merlin8_phy1_info_ptr->trace_mem_ram_size,
                                                  rd_idx,
                                                  (uint16_t) rd_size,
                                                  arg,
                                                  callback));
    } else {
        /* Micro writes trace memory using descending addresses.
         * So read using ascending addresses using block read
         */
        EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram_descending(sa__,
                                                             merlin8_phy1_info_ptr->trace_mem_ram_base + (uint32_t)(merlin8_phy1_info_ptr->grp_ram_size*micro_num),
                                                             (uint16_t)merlin8_phy1_info_ptr->trace_mem_ram_size,
                                                             rd_idx,
                                                             (uint16_t)merlin8_phy1_info_ptr->trace_mem_ram_size,
                                                             arg,
                                                             callback));
    }


    if (0 == (bypass_micro & 1)) {
        /* Read Done to resume logging  */
        EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_EVENT_LOG_READ, CMD_EVENT_LOG_READ_DONE, 50));
    }
    else if (merlin8_phy1_info_ptr->trace_memory_concurrent_read) {
        /* Update read pointer */
        EFUN(reg_wr_DSC_A_DSC_SCRATCH(rd_idx0));
        EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_EVENT_LOG_READ, CMD_EVENT_LOG_SET_READ_IDX, 50));
    }
    EFUN(merlin8_phy1_acc_set_lane(sa__,current_lane));
    return(ERR_CODE_NONE);
}  /* merlin8_phy1_INTERNAL_read_event_log_with_callback() */


#ifdef TO_FLOATS
/* convert uint32_t to float8 */
float8_t merlin8_phy1_INTERNAL_int32_to_float8(uint32_t input) {
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
uint32_t merlin8_phy1_INTERNAL_float8_to_uint32(float8_t input) {
    uint32_t x;
    if(input == 0) return(0);
    x = (uint32_t)((((uint8_t)(input))>>5) + 8);
    if((input & 0x1F) < 3) {
      return(x>>(3-(input & 0x1f)));
    } else if((input & 0x1F) > 3) {
        x=(x<<1)|1;
        return((x<<((input & 0x1F)-3-1)));
    }else {
        return(x<<((input & 0x1F)-3));
    }
}

#ifdef PCIEG3_V1
/* Convert uint8 to 8-bit gray code */
uint8_t merlin8_phy1_INTERNAL_uint8_to_gray(uint8_t input) {
    return input ^ (input >> 1);
}

/* Convert 8-bit gray code to uint8 */
uint8_t merlin8_phy1_INTERNAL_gray_to_uint8(uint8_t input) {
    input = input ^ (input >> 4);
    input = input ^ (input >> 2);
    input = input ^ (input >> 1);
    return input;
}
#endif


/* convert float12 to uint32 */
uint32_t merlin8_phy1_INTERNAL_float12_to_uint32(uint8_t input, uint8_t multi) {

    return(((uint32_t)input)<<multi);
}

#ifdef TO_FLOATS
/* convert uint32 to float12 */
uint8_t merlin8_phy1_INTERNAL_uint32_to_float12(srds_access_t *sa__, uint32_t input, uint8_t *multi) {
    int8_t cnt;
    uint8_t output;
    if(!multi) {
      return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
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
#endif /* !SMALL_FOOTPRINT */

#if !defined(SMALL_FOOTPRINT)
err_code_t merlin8_phy1_INTERNAL_set_rx_pf_main(srds_access_t *sa__, uint8_t val) {
    INIT_SRDS_ERR_CODE
    uint8_t pf_max;
    pf_max = PF_MAX_VALUE;
    if (val > pf_max) {
       return (merlin8_phy1_error(sa__, ERR_CODE_PF_INVALID));
    }
    EFUN(WR_RX_PF_CTRL(val));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_set_rx_pf2(srds_access_t *sa__, uint8_t val) {
    INIT_SRDS_ERR_CODE
    if (val > 7) {
      return (merlin8_phy1_error(sa__, ERR_CODE_PF_INVALID));
    }
    EFUN(WR_RX_PF2_CTRL(val));
    return(ERR_CODE_NONE);
}

#endif /* !SMALL_FOOTPRINT */

#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_INTERNAL_get_rx_pf_main(srds_access_t *sa__, uint8_t *val) {
    INIT_SRDS_ERR_CODE
    ESTM(*val = RD_RX_PF_CTRL());
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_rx_pf2(srds_access_t *sa__, uint8_t *val) {
    INIT_SRDS_ERR_CODE
    ESTM(*val = RD_RX_PF2_CTRL());
    return (ERR_CODE_NONE);
}

#endif /* !SMALL_FOOTPRINT */

#if !defined(SMALL_FOOTPRINT)
err_code_t merlin8_phy1_INTERNAL_set_rx_vga(srds_access_t *sa__, uint8_t val) {
    INIT_SRDS_ERR_CODE

    EFUN(merlin8_phy1_INTERNAL_check_uc_lane_stopped(sa__));  /* make sure uC is stopped to avoid race conditions */

    if (val > RX_VGA_CTRL_VAL_MAX) {
      return (merlin8_phy1_error(sa__, ERR_CODE_VGA_INVALID));
    }
#if defined(RX_VGA_CTRL_VAL_MIN) && (RX_VGA_CTRL_VAL_MIN > 0)
    if (val < RX_VGA_CTRL_VAL_MIN) {
      return (merlin8_phy1_error(sa__, ERR_CODE_VGA_INVALID));
    }
#endif

#ifndef PCIEG3_V1
    EFUN(wr_hwtune_ovr_write_sel(0));                   /* Configure hwtune_ovr_write_sel to VGA */
    EFUN(wr_hwtune_ovr_write_val((uint16_t)(val<<3)));              /* hwtune_ovr_write_val[8:3] are used to drive the analog control port */
    EFUN(wr_hwtune_ovr_write_en(1));
#else
    {
        uint8_t rate_sel;
        ESTM(rate_sel = rd_rate_sel());
        if (rate_sel < MERLIN8_PHY1_GEN3) {
            if (val <= 15) {
                EFUN(wr_rx_vga1_ctrl_G1G2(val));
                EFUN(wr_rx_vga2_ctrl_G1G2(0));
                EFUN(wr_rx_vga3_ctrl_G1G2(0));
            } else if (val <= 30) {
                EFUN(wr_rx_vga1_ctrl_G1G2(15));
                EFUN(wr_rx_vga2_ctrl_G1G2((uint8_t)(val-15)));
                EFUN(wr_rx_vga3_ctrl_G1G2(0));
            }else{
                EFUN(wr_rx_vga1_ctrl_G1G2(15));
                EFUN(wr_rx_vga2_ctrl_G1G2(15));
                EFUN(wr_rx_vga3_ctrl_G1G2((uint8_t)(val-30)));
            }
        } else {
            EFUN(wr_hwtune_ovr_write_sel(0));                   /* Configure hwtune_ovr_write_sel to VGA */
            EFUN(wr_hwtune_ovr_write_val((uint16_t)(val<<3)));              /* hwtune_ovr_write_val[8:3] are used to drive the analog control port */
            EFUN(wr_hwtune_ovr_write_en(1));
        }
    }
#endif /* MERLIN7_PCIEG3 */
    return(ERR_CODE_NONE);
}



#endif /* !SMALL_FOOTPRINT */

#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_INTERNAL_get_rx_vga(srds_access_t *sa__, uint8_t *val) {
    INIT_SRDS_ERR_CODE
#ifndef PCIEG3_V1
    ESTM(*val = rd_vga_bin());
#else
    {
        uint8_t rate_sel;
        ESTM(rate_sel = rd_rate_sel());
        if (rate_sel < MERLIN8_PHY1_GEN3) {
            uint8_t temp = 0;
            ESTM(temp = (uint8_t)(temp + rd_rx_vga1_ctrl_G1G2()));
            ESTM(temp = (uint8_t)(temp + rd_rx_vga2_ctrl_G1G2()));
            ESTM(temp = (uint8_t)(temp + rd_rx_vga3_ctrl_G1G2()));
            *val = temp;
        } else {
            ESTM(*val = rd_vga_bin());
        }
    }
#endif
    return (ERR_CODE_NONE);
}

#endif /* !SMALL_FOOTPRINT */

#if !defined(SMALL_FOOTPRINT)
err_code_t merlin8_phy1_INTERNAL_set_rx_dfe1(srds_access_t *sa__, int8_t val) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_INTERNAL_check_uc_lane_stopped(sa__));  /* make sure uC is stopped to avoid race conditions */

    {
        int8_t tap_eo;

        if (val > 63) {
            return (merlin8_phy1_error(sa__, ERR_CODE_DFE1_INVALID));
        }
        /* Compute tap1 even/odd component */
        tap_eo = 0;                                      /* Not supporting dfe_dcd values */
        EFUN(wr_hwtune_ovr_write_sel(2));                /* Write tap1_odd */
        EFUN(wr_hwtune_ovr_write_val((uint16_t)tap_eo));
        EFUN(wr_hwtune_ovr_write_en(1));
        EFUN(wr_hwtune_ovr_write_sel(3));                /* Write tap1_even */
        EFUN(wr_hwtune_ovr_write_val((uint16_t)tap_eo));
        EFUN(wr_hwtune_ovr_write_en(1));
        EFUN(wr_hwtune_ovr_write_sel(1));
        EFUN(wr_hwtune_ovr_write_val((uint16_t)(val-tap_eo)));       /* Write the common tap */
        EFUN(wr_hwtune_ovr_write_en(1));
    }
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_set_rx_dfe2(srds_access_t *sa__, int8_t val) {
    INIT_SRDS_ERR_CODE
    int8_t tap_eo;

    if ((val > 31) || (val < -31)) {
      return (merlin8_phy1_error(sa__, ERR_CODE_DFE2_INVALID));
    }

    EFUN(merlin8_phy1_INTERNAL_check_uc_lane_stopped(sa__));  /* make sure uC is stopped to avoid race conditions */

    /* Compute tap2 odd/even component */
    tap_eo = 0;                                         /* Not supporting dfe_dcd values */
    EFUN(wr_hwtune_ovr_write_sel(5));                   /* Write tap2_odd */
    EFUN(wr_hwtune_ovr_write_val((uint16_t)tap_eo));
    EFUN(wr_hwtune_ovr_write_en(1));
    EFUN(wr_hwtune_ovr_write_sel(6));                   /* Write tap2_even */
    EFUN(wr_hwtune_ovr_write_val((uint16_t)tap_eo));
    EFUN(wr_hwtune_ovr_write_en(1));
    EFUN(wr_hwtune_ovr_write_sel(4));
    EFUN(wr_hwtune_ovr_write_val((uint16_t)(SRDS_ABS(val)-tap_eo)));/* Write the common tap */
    EFUN(wr_hwtune_ovr_write_en(1));
    EFUN(wr_hwtune_ovr_write_sel(10));                  /* Write tap2_even_sign */
    EFUN(wr_hwtune_ovr_write_val(val<0?1:0));
    EFUN(wr_hwtune_ovr_write_en(1));
    EFUN(wr_hwtune_ovr_write_sel(11));                  /* Write tap2_odd_sign */
    EFUN(wr_hwtune_ovr_write_val(val<0?1:0));
    EFUN(wr_hwtune_ovr_write_en(1));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_set_rx_dfe3(srds_access_t *sa__, int8_t val) {
    INIT_SRDS_ERR_CODE
    if ((val > 31) || (val < -31)) {
      return (merlin8_phy1_error(sa__, ERR_CODE_DFE3_INVALID));
    }

    EFUN(merlin8_phy1_INTERNAL_check_uc_lane_stopped(sa__));  /* make sure uC is stopped to avoid race conditions */

    EFUN(wr_hwtune_ovr_write_sel(7));                   /* Write tap3 */
    EFUN(wr_hwtune_ovr_write_val((uint16_t)val));
    EFUN(wr_hwtune_ovr_write_en(1));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_set_rx_dfe4(srds_access_t *sa__, int8_t val) {
    INIT_SRDS_ERR_CODE
    if ((val > 15) || (val < -15)) {
      return (merlin8_phy1_error(sa__, ERR_CODE_DFE4_INVALID));
    }

    EFUN(merlin8_phy1_INTERNAL_check_uc_lane_stopped(sa__));  /* make sure uC is stopped to avoid race conditions */

    EFUN(wr_hwtune_ovr_write_sel(8));                   /* Write tap4 */
    EFUN(wr_hwtune_ovr_write_val((uint16_t)val));
    EFUN(wr_hwtune_ovr_write_en(1));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_set_rx_dfe5(srds_access_t *sa__, int8_t val) {
    INIT_SRDS_ERR_CODE
    if ((val > 15) || (val < -15)) {
      return (merlin8_phy1_error(sa__, ERR_CODE_DFE5_INVALID));
    }

    EFUN(merlin8_phy1_INTERNAL_check_uc_lane_stopped(sa__));  /* make sure uC is stopped to avoid race conditions */

    EFUN(wr_hwtune_ovr_write_sel(9));                   /* Write tap5 */
    EFUN(wr_hwtune_ovr_write_val((uint16_t)val));
    EFUN(wr_hwtune_ovr_write_en(1));
    return(ERR_CODE_NONE);
}

#endif /* !SMALL_FOOTPRINT */

#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_INTERNAL_get_rx_dfe1(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
    ESTM(*val = (int8_t)(rd_dfe_1_e() + rd_dfe_1_cmn()));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_rx_dfe2(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
    uint8_t dfe_2_se;
    int8_t dfe_2_cmn, dfe_2_e;
    ESTM(dfe_2_se = rd_dfe_2_se());
    ESTM(dfe_2_e = (int8_t)rd_dfe_2_e());
    ESTM(dfe_2_cmn = (int8_t)rd_dfe_2_cmn());
    *val = (int8_t)(dfe_2_se ? (-(dfe_2_e + dfe_2_cmn)) : (dfe_2_e + dfe_2_cmn));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_rx_dfe3(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
    ESTM(*val = rd_dfe_3_cmn());
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_rx_dfe4(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
    ESTM(*val = rd_dfe_4_cmn());
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_rx_dfe5(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
    ESTM(*val = rd_dfe_5_cmn());
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_dfe2_dcd(srds_access_t *sa__, int8_t *dfe2_dcd) {
    INIT_SRDS_ERR_CODE

    int8_t dfe_2_se, dfe_2_e, dfe_2_so, dfe_2_o;
    ESTM(dfe_2_se  = (int8_t)rd_dfe_2_se());
    ESTM(dfe_2_e   = (int8_t)rd_dfe_2_e());
    ESTM(dfe_2_so  = (int8_t)rd_dfe_2_so());
    ESTM(dfe_2_o   = (int8_t)rd_dfe_2_o());

    ESTM(*dfe2_dcd = (int8_t)((dfe_2_se ? -dfe_2_e: dfe_2_e) -(dfe_2_so ? -dfe_2_o: dfe_2_o)));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_eye_scan_setup(srds_access_t *sa__, uint8_t intr_pass_b, int16_t *y_max, int16_t *y_step, uint16_t *usr_diag_status) {
    INIT_SRDS_ERR_CODE
    uint8_t dig_lpbk_en;
    int16_t val_max, val_step, slicer_range_mV;

    /* init value */
    *usr_diag_status = 0;

    ESTM(dig_lpbk_en = rd_dig_lpbk_en());
    if(dig_lpbk_en) {
        EFUN_PRINTF(("Eye scan is not supported while digital loopback is enabled.\n"));
        return (ERR_CODE_INVALID_DIG_LPBK_STATE);
    }
    val_max  = EYE_SCAN_NRZ_VERTICAL_IDX_MAX;
    val_step = EYE_SCAN_NRZ_VERTICAL_STEP;
    EFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, 31, rd_p1_thresh_sel(), INVALID_SLICER, &slicer_range_mV));
    EFUN_PRINTF(("\n\n****  SERDES %sEYE SCAN CORE %d LANE %d [%d, %d]   ****\n",
      intr_pass_b?"INTRUSIVE ":"",merlin8_phy1_acc_get_core(sa__),merlin8_phy1_acc_get_lane(sa__),
      -slicer_range_mV+1, slicer_range_mV));

    /* start horizontal acquisition */
    {
        uint8_t direction = EYE_SCAN_HORIZ;
        err_code_t err_code;

        if(intr_pass_b) {
            EFUN_PRINTF(("ERROR: Intrusive eye scan mode is not supported in this IP\n"));
            return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
        }

        err_code = merlin8_phy1_meas_eye_scan_start(sa__, direction);

        if (err_code) {
            if (err_code == ERR_CODE_DIAG_SCAN_NO_PMD_LOCK) {
                return err_code;
            }
            else {
                EFUN(merlin8_phy1_meas_eye_scan_done(sa__));
            }
        }
    }



    *y_max = val_max;
    *y_step = val_step;

    return ERR_CODE_NONE;
}
#endif /* SMALL_FOOTPRINT */

#ifndef PCIEG3_V1
err_code_t merlin8_phy1_INTERNAL_get_tx_pre(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
  ESTM(*val = (int8_t)rd_cl72_txfir_pre());
  return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_tx_main(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
  ESTM(*val = (int8_t)rd_cl72_txfir_main());
  return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_tx_post1(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
  ESTM(*val = (int8_t)rd_cl72_txfir_post());
  return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_tx_post2(srds_access_t *sa__, int8_t *val) {
    INIT_SRDS_ERR_CODE
  ESTM(*val = (int8_t)rd_txfir_post2());
  return (ERR_CODE_NONE);
}
#endif /* MERLIN7_PCIEG3 */


err_code_t merlin8_phy1_INTERNAL_core_clkgate(srds_access_t *sa__, uint8_t enable) {
    UNUSED(sa__);
    UNUSED(enable);
    return (ERR_CODE_NONE);
}
#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_INTERNAL_lane_clkgate(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE

  if (enable) {
    /* Use frc/frc_val to force all RX and TX clk_vld signals to 0 */
        EFUN(wr_pmd_rx_clk_vld_frc_val(0x0));
        EFUN(wr_pmd_rx_clk_vld_frc(0x1));
        EFUN(wr_pmd_tx_clk_vld_frc_val(0x0));
        EFUN(wr_pmd_tx_clk_vld_frc(0x1));

    /* Use frc_on to force clkgate */
        EFUN(wr_ln_rx_s_clkgate_frc_on(0x1));

        EFUN(wr_ln_tx_s_clkgate_frc_on(0x1));

  }
  else {
        EFUN(wr_pmd_rx_clk_vld_frc_val(0x0));
        EFUN(wr_pmd_rx_clk_vld_frc(0x0));
        EFUN(wr_pmd_tx_clk_vld_frc_val(0x0));
        EFUN(wr_pmd_tx_clk_vld_frc(0x0));

        EFUN(wr_ln_rx_s_clkgate_frc_on(0x0));

        EFUN(wr_ln_tx_s_clkgate_frc_on(0x0));
  }
  return (ERR_CODE_NONE);
}

#ifndef PCIEG3_V1
/*-----------------------------------------------*/
/*  Get dynamic eye margin estimation values     */
/*-----------------------------------------------*/
err_code_t merlin8_phy1_INTERNAL_get_eye_margin_est(srds_access_t *sa__, uint16_t *left_eye_mUI, uint16_t *right_eye_mUI, uint16_t  *upper_eye_mV, uint16_t *lower_eye_mV) {
  INIT_SRDS_ERR_CODE
  uint8_t ladder_range = 0;
  uint8_t eye_val = 0;
  ESTM(ladder_range = rd_p1_thresh_sel());
  ESTM(eye_val = rdv_usr_sts_heye_left());
  ESTM(*left_eye_mUI  = merlin8_phy1_INTERNAL_eye_to_mUI(sa__, eye_val));
  ESTM(eye_val = rdv_usr_sts_heye_right());
  ESTM(*right_eye_mUI = merlin8_phy1_INTERNAL_eye_to_mUI(sa__, eye_val));
  ESTM(eye_val = rdv_usr_sts_veye_upper());
  ESTM(*upper_eye_mV = merlin8_phy1_INTERNAL_eye_to_mV(sa__, eye_val, ladder_range));
  ESTM(eye_val = rdv_usr_sts_veye_lower());
  ESTM(*lower_eye_mV = merlin8_phy1_INTERNAL_eye_to_mV(sa__, eye_val, ladder_range));

  return (ERR_CODE_NONE);
}

uint16_t merlin8_phy1_INTERNAL_eye_to_mUI(srds_access_t *sa__, uint8_t var)
{
    UNUSED(sa__);
    /* var is in units of 1/512 th UI, so need to multiply by 1000/512 */
    return (uint16_t)(((uint16_t)var)*125/64);
}


uint16_t merlin8_phy1_INTERNAL_eye_to_mV(srds_access_t *sa__, uint8_t var, uint8_t ladder_range) {
    INIT_SRDS_ERR_CODE
    uint16_t vl, vu;
    int16_t nlmv_val = 0;
    /* find nearest two vertical levels */
    EFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, (int16_t)var/8, ladder_range, INVALID_SLICER, &nlmv_val));
    vl = (uint16_t)nlmv_val;
    EFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, (int16_t)(SRDS_MIN(31,var/8+1)), ladder_range, INVALID_SLICER, &nlmv_val));
    vu = (uint16_t)nlmv_val;
    return ((uint16_t)(vl + (vu-vl)*(var&7)/8));
}

#endif /* !MERLIN7_PCIEG3*/


err_code_t merlin8_phy1_INTERNAL_get_osr_mode(srds_access_t *sa__, merlin8_phy1_osr_mode_st *imode) {
    INIT_SRDS_ERR_CODE
    merlin8_phy1_osr_mode_st mode;

    ENULL_MEMSET(&mode, 0, sizeof(merlin8_phy1_osr_mode_st));

    if(!imode) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    ESTM(mode.tx = rd_tx_osr_mode());
    ESTM(mode.rx = rd_rx_osr_mode());
    mode.tx_rx = 255;
    *imode = mode;
    return (ERR_CODE_NONE);
}


/** A simple decoding structure
 *
 */
struct simpleDecoder
{
    int32_t  name;
    uint32_t val;
};

uint32_t merlin8_phy1_osr_mode_enum_to_int_x1000(uint8_t osr_mode) {

    switch(osr_mode) {
    case MERLIN8_PHY1_OSX1:      return 1000;
    case MERLIN8_PHY1_OSX2:      return 2000;
    case MERLIN8_PHY1_OSX4:      return 4000;
    case MERLIN8_PHY1_OSX8:      return 8000;
    case MERLIN8_PHY1_OSX3:      return 3000;
    case MERLIN8_PHY1_OSX3P3:    return 3300;
    case MERLIN8_PHY1_OSX5:      return 5000;
    case MERLIN8_PHY1_OSX7P5:    return 7500;
    case MERLIN8_PHY1_OSX8P25:   return 8250;
    case MERLIN8_PHY1_OSX10:     return 10000;
    default:                    return 1000;
    }
}
#ifndef PCIEG3_V1
const char* merlin8_phy1_INTERNAL_get_osr_str(uint8_t osr_val, uint8_t osr_str_length) {
    const char* e2s_long_osr_mode_enum[10] = {
      "x1   ",
      "x2   ",
      "x3   ",
      "x3.3 ",
      "x4   ",
      "x5   ",
      "x7.5 ",
      "x8   ",
      "x8.25",
      "x10  "
    };

    const char* e2s_short_osr_mode_enum[10] = {
      "x1",
      "x2",
      "x3",
      "xT",
      "x4",
      "x5",
      "x7",
      "x8",
      "x9",
      "xA"
    };
    switch(osr_str_length) {
        case (GET_OSR_STR_LONG):
            return ((osr_val < sizeof(e2s_long_osr_mode_enum)/sizeof(e2s_long_osr_mode_enum[0])) ? e2s_long_osr_mode_enum[osr_val] : "ERR  ");
        case (GET_OSR_STR_SHORT):
            return ((osr_val < sizeof(e2s_short_osr_mode_enum)/sizeof(e2s_short_osr_mode_enum[0])) ? e2s_short_osr_mode_enum[osr_val] : "ER");
        default:
            return "ER";
    }
}
#endif /* !MERLIN7_PCIEG3 */
#endif /* SMALL_FOOTPRINT */


err_code_t merlin8_phy1_INTERNAL_sigdet_status(srds_access_t *sa__, uint8_t *sig_det, uint8_t *sig_det_chg) {
    INIT_SRDS_ERR_CODE
#ifndef PCIEG3_V1
    uint16_t rddata;

    ESTM(rddata = reg_rd_SIGDET_SDSTATUS_0());
    *sig_det = ex_SIGDET_SDSTATUS_0__signal_detect(rddata);
    *sig_det_chg = (uint8_t)(ex_SIGDET_SDSTATUS_0__signal_detect_change(rddata));
#else
    ESTM(*sig_det     = ((rd_rx_sigdet() >> 1) & 0x1));
    ESTM(*sig_det_chg = (uint8_t)rd_rg_sigdet_tggl_cnt());
#endif
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_pll_lock_status(srds_access_t *sa__, uint8_t *pll_lock, uint8_t *pll_lock_chg) {
    INIT_SRDS_ERR_CODE
    uint16_t rddata;
    uint8_t pll_fail;
    // FIXME Immediately: what to do for fail indicate and sticky bits????
    ESTM(rddata = reg_rdc_AMSCOM8_C_AMSCOM8_C_PLL_STS1());
    // No definition available to say this is bit 14
    *pll_lock = (uint8_t)((rddata >> 14) & 1);
    pll_fail = 0;
    *pll_lock_chg = (uint8_t)((*pll_lock ^ 0) | (*pll_lock ^ !pll_fail));
    return(ERR_CODE_NONE);
}

#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_INTERNAL_pmd_lock_status(srds_access_t *sa__, uint8_t *pmd_lock, uint8_t *pmd_lock_chg) {
    INIT_SRDS_ERR_CODE
    uint16_t rddata;
    ESTM(rddata = reg_rd_TLB_RX_DBG_PMD_RX_LOCK_STATUS());
    *pmd_lock = ex_TLB_RX_DBG_PMD_RX_LOCK_STATUS__dbg_pmd_rx_lock(rddata);
    *pmd_lock_chg = (uint8_t)(ex_TLB_RX_DBG_PMD_RX_LOCK_STATUS__dbg_pmd_rx_lock_change(rddata));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_get_tx_ppm(srds_access_t *sa__, int16_t *tx_ppm) {
    INIT_SRDS_ERR_CODE
    /* tx_ppm = register/10.486                     */
    /* 3125/32768 = 1/10.486                        */
    /* tx_ppm = register*3125/32768                 */
    /* Suppose you want x/N, but with rounding then */
    /* tx_ppm rounded = (x+N/2)/N;                  */

    ESTM(*tx_ppm = (int16_t)(((rd_tx_pi_integ2_reg() * 3125) + (32768 / 2)) / 32768));
    return ERR_CODE_NONE;
}


err_code_t merlin8_phy1_INTERNAL_get_rx_ppm(srds_access_t *sa__, int16_t *rx_ppm) {
    INIT_SRDS_ERR_CODE
    int16_t divisor;
    divisor = 84;
    ESTM(*rx_ppm = rd_cdr_integ_reg()/divisor);
    return ERR_CODE_NONE;
}


err_code_t merlin8_phy1_INTERNAL_read_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *istate) {
   INIT_SRDS_ERR_CODE
    uint8_t ladder_range = 0;

    if(!istate) {
      return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    EFUN(merlin8_phy1_INTERNAL_pmd_lock_status(sa__,&istate->pmd_lock, &istate->pmd_lock_chg));
    EFUN(merlin8_phy1_INTERNAL_get_stop_state_lane_state(sa__, istate)); /* Dependency on istate->pmd_lock. Updates istate->stop_state AND stop micro as well */
    istate->laneid = merlin8_phy1_acc_get_lane(sa__);

#ifdef PCIEG3_V1
    ESTM(istate->rate_select = (uint8_t)(rd_rate_sel()+1));
    ESTM(istate->dsc_one_hot_0_0 = rd_dsc_state_one_hot_0());
    ESTM(istate->dsc_one_hot_0_1 = rd_dsc_state_one_hot_0());
#endif
    EFUN(merlin8_phy1_INTERNAL_get_osr_mode(sa__, &istate->osr_mode));
#ifndef PCIEG3_V1
    EFUN(merlin8_phy1_INTERNAL_get_osr_str_lane_state(sa__, istate)); /* Dependency on istate->osr_mode */
    EFUN(merlin8_phy1_INTERNAL_get_uc_cfg_lane_state(sa__, istate));
    ESTM(istate->UC_STS = rdv_status_byte());
#endif

#if defined(SERDES_TX_RX_SEPARATE)
    ESTM(istate->rx_rst    = rd_rx_lane_dp_reset_state());
    ESTM(istate->tx_rst    = rd_tx_lane_dp_reset_state());
#else
    ESTM(istate->rx_rst = rd_lane_dp_reset_state());
#endif
    EFUN(merlin8_phy1_INTERNAL_sigdet_status(sa__,&istate->sig_det, &istate->sig_det_chg));
    EFUN(merlin8_phy1_INTERNAL_get_rx_ppm(sa__, &istate->ppm));
    EFUN(merlin8_phy1_INTERNAL_get_clk_lane_state(sa__, istate));
    ESTM(istate->CDR = rd_br_pd_en());
    EFUN(merlin8_phy1_INTERNAL_get_pf_lane_state(sa__, istate));
    EFUN(merlin8_phy1_read_rx_afe(sa__, RX_AFE_VGA, &istate->VGA));

    ESTM(istate->DCO = rd_dc_offset_bin());

    EFUN(merlin8_phy1_INTERNAL_get_TP0_lane_state(sa__, istate)); /* Dependency on istate->rx_pam_mode */
    ESTM(ladder_range = rd_p1_thresh_sel());
    EFUN(merlin8_phy1_INTERNAL_get_p1_threshold(sa__, &istate->p1_lvl_ctrl));
    EFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, istate->p1_lvl_ctrl, ladder_range, INVALID_SLICER, &istate->P1mV));
    EFUN(merlin8_phy1_INTERNAL_get_dfe_lane_state(sa__, istate)); /* Dependency on istate->rx_pam_mode */
    ESTM(istate->pe = rd_p1_offset_evn_bin());
    ESTM(istate->ze = rd_data_offset_evn_bin());
    ESTM(istate->me = rd_m1_offset_evn_bin());

    ESTM(istate->po = rd_p1_offset_odd_bin());
    ESTM(istate->zo = rd_data_offset_odd_bin());
    ESTM(istate->mo = rd_m1_offset_odd_bin());

    EFUN(merlin8_phy1_INTERNAL_get_tx_ppm(sa__, &istate->tx_ppm));

#ifndef PCIEG3_V1
    ESTM(istate->linktrn_en = rd_cl72_ieee_training_enable());
#endif

#ifndef PCIEG3_V1
    EFUN(merlin8_phy1_INTERNAL_get_txfir_lane_state(sa__, istate));
    EFUN(merlin8_phy1_INTERNAL_get_eye_margin_est(sa__, &istate->heye_left, &istate->heye_right, &istate->veye_upper, &istate->veye_lower));
    ESTM(istate->soc_pos = rd_ams_rx_sigdet_offset_correction_pos());
    ESTM(istate->soc_neg = rd_ams_rx_sigdet_offset_correction_neg());
    EFUN(merlin8_phy1_INTERNAL_get_link_time(sa__, &istate->link_time));
#endif /* !defined(MERLIN7_PCIEG3) */
    EFUN(merlin8_phy1_INTERNAL_get_pmd_lock_chg_lane_state(sa__, istate));  /* Depedency on istate->pmd_lock */
    ESTM(istate->tx_pmd_dp_invert = rd_tx_pmd_dp_invert());
    ESTM(istate->rx_pmd_dp_invert = rd_rx_pmd_dp_invert());
    ESTM(istate->rmt_lpbk_en = rd_rmt_lpbk_en());
    ESTM(istate->dig_lpbk_en = rd_dig_lpbk_en());
    ESTM(istate->tx_pi_en = rd_tx_pi_en());

    EFUN(merlin8_phy1_INTERNAL_resume_micro_lane_state(sa__, istate)); /* Dependency on istate->stop_state */

    EFUN(merlin8_phy1_INTERNAL_get_BER_string(sa__, 100, istate->BER, sizeof(istate->BER)));

    return (ERR_CODE_NONE);
}

/*------------------------------------*/
/*  Read Lane state Helper Functions  */
/*------------------------------------*/
err_code_t merlin8_phy1_INTERNAL_resume_micro_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    if (!st->stop_state || (st->stop_state == 0xFF)) {
        /* manually check error code instead of EFUN*/
        err_code_t resume_status = ERR_CODE_NONE;
        resume_status = merlin8_phy1_stop_rx_adaptation(sa__, 0);
        if (resume_status) {
            USR_PRINTF(("WARNING: Resuming micro after lane state capture failed \n"));
        }
    }
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_get_stop_state_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    err_code_t err_code = 0;
    st->stop_state = merlin8_phy1_INTERNAL_stop_micro(sa__,st->pmd_lock,&err_code);

    if(err_code) USR_PRINTF(("WARNING: Unable to stop microcontroller,  following data is suspect\n"));
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_get_pmd_lock_chg_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    INIT_SRDS_ERR_CODE
    uint8_t rx_lock_at_end=0, rx_lock_chg_at_end=0;

    /* read lock status at end and combine them to handle transient cases */
    EFUN(merlin8_phy1_INTERNAL_pmd_lock_status(sa__, &rx_lock_at_end, &rx_lock_chg_at_end));
    if (st->pmd_lock != rx_lock_at_end) {
        USR_PRINTF(("WARNING: Lane %d rx_lock status changed while reading lane state (rx_lock_start=%d, rx_lock_chg_start=%d, rx_lock_at_end=%d, rx_lock_chg_at_end=%d) \n",
                    merlin8_phy1_acc_get_lane(sa__), st->pmd_lock, st->pmd_lock_chg, rx_lock_at_end, rx_lock_chg_at_end));
    }
    st->pmd_lock &= rx_lock_at_end;
    st->pmd_lock_chg |= rx_lock_chg_at_end;
    return ERR_CODE_NONE;
}
#ifndef PCIEG3_V1

err_code_t merlin8_phy1_INTERNAL_get_osr_str_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    const char *tx;
    UNUSED(sa__);

    if(st->osr_mode.tx_rx != 255) {
        tx = merlin8_phy1_INTERNAL_get_osr_str(st->osr_mode.tx_rx, GET_OSR_STR_LONG);
        USR_SNPRINTF(st->tx_osr_mode_str, sizeof(st->tx_osr_mode_str), "%s", tx);
    }
    else {
        const char *rx;
        tx = merlin8_phy1_INTERNAL_get_osr_str(st->osr_mode.tx, GET_OSR_STR_SHORT);
        rx = merlin8_phy1_INTERNAL_get_osr_str(st->osr_mode.rx, GET_OSR_STR_SHORT);
        USR_SNPRINTF(st->tx_osr_mode_str, sizeof(st->tx_osr_mode_str), "%s", tx);
        USR_SNPRINTF(st->rx_osr_mode_str, sizeof(st->rx_osr_mode_str), "%s", rx);
    }
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_get_uc_cfg_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    INIT_SRDS_ERR_CODE
    struct merlin8_phy1_uc_lane_config_st lane_cfg;

    EFUN(merlin8_phy1_get_uc_lane_cfg(sa__, &lane_cfg));
    st->UC_CFG = lane_cfg.word;

    return ERR_CODE_NONE;
}
#endif /* !defined(MERLIN7_PCIEG3) */

err_code_t merlin8_phy1_INTERNAL_get_TP0_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    UNUSED(sa__);
    st->TP0 = 0;
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_get_pf_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    INIT_SRDS_ERR_CODE
#ifdef PCIEG3_V1
    uint8_t rate_sel, pf2_val;

    ESTM(rate_sel = rd_rate_sel());
    if (rate_sel == MERLIN8_PHY1_GEN1) {
        ESTM(pf2_val = merlin8_phy1_INTERNAL_gray_to_uint8(rd_pf2_lowp_ctrl_g1()));
    }
    else if (rate_sel == MERLIN8_PHY1_GEN2) {
        ESTM(pf2_val = merlin8_phy1_INTERNAL_gray_to_uint8(rd_pf2_lowp_ctrl_g2()));
    }
    else {
        ESTM(pf2_val = rd_pf2_lowp_ctrl());
    }
    st->pf2_ctrl = (int8_t)pf2_val;
#else
    EFUN(merlin8_phy1_read_rx_afe(sa__, RX_AFE_PF2, &st->pf2_ctrl));
#endif
    ESTM(st->pf_hiz = rd_pf_hiz());
    EFUN(merlin8_phy1_read_rx_afe(sa__, RX_AFE_PF, &st->pf_main));
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_get_clk_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    INIT_SRDS_ERR_CODE
    ESTM(st->clk90 = rd_cnt_d_minus_m1());
    ESTM(st->clkp1 = rd_cnt_d_minus_p1());
    /* drop the MSB, the result is actually valid modulo 128 */
    /* Also flip the sign to account for d-m1, versus m1-d */
    st->clk90 = (int8_t)(st->clk90<<1);
    st->clk90 = (int8_t)(-(st->clk90>>1));
    st->clkp1 = (int8_t)(st->clkp1<<1);
    st->clkp1 = (int8_t)(-(st->clkp1>>1));
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_get_dfe_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    INIT_SRDS_ERR_CODE
    int8_t dfe_1_e;
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe1(sa__, &st->DFE1));
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe2(sa__, &st->DFE2));
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe3(sa__, &st->DFE3));
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe4(sa__, &st->DFE4));
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe5(sa__, &st->DFE5));
    ESTM(dfe_1_e = (int8_t)rd_dfe_1_e());
    ESTM(st->dfe1_dcd = (int8_t)(dfe_1_e - rd_dfe_1_o()));
    EFUN(merlin8_phy1_INTERNAL_get_dfe2_dcd(sa__, &st->dfe2_dcd));
    return ERR_CODE_NONE;
}
#ifndef PCIEG3_V1

err_code_t merlin8_phy1_INTERNAL_get_txfir_lane_state(srds_access_t *sa__, merlin8_phy1_lane_state_st_t *st) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_INTERNAL_get_tx_pre(sa__, &st->txfir_pre));
    EFUN(merlin8_phy1_INTERNAL_get_tx_main(sa__, &st->txfir_main));
    EFUN(merlin8_phy1_INTERNAL_get_tx_post1(sa__, &st->txfir_post1));
    EFUN(merlin8_phy1_INTERNAL_get_tx_post2(sa__, &st->txfir_post2));
    return ERR_CODE_NONE;
}
#endif /* !defined(MERLIN7_PCIEG3) */



err_code_t merlin8_phy1_INTERNAL_get_link_time(srds_access_t *sa__, uint32_t *link_time) {
    INIT_SRDS_ERR_CODE
    uint32_t usr_sts_link_time = 0;
    if(link_time == NULL) {
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    ESTM(usr_sts_link_time = rdv_usr_sts_link_time());
    *link_time = (usr_sts_link_time<<MERLIN8_PHY1_LANE_TIMER_SHIFT)/10;
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_display_lane_state_no_newline(srds_access_t *sa__) {
  INIT_SRDS_ERR_CODE
  err_code_t Exit_Status = ERR_CODE_NONE;
  merlin8_phy1_lane_state_st_t *state;


  state = (merlin8_phy1_lane_state_st_t *)USR_CALLOC(1, sizeof(merlin8_phy1_lane_state_st_t));
  if(!state) {
      return (ERR_CODE_MEM_ALLOC_FAIL);
  }
  CFUN(merlin8_phy1_INTERNAL_read_lane_state(sa__, state));

  EFUN_PRINTF(("%2d ", state->laneid));

#ifdef PCIEG3_V1
  EFUN_PRINTF((" %s%s%02x ", (state->tx_pmd_dp_invert ? "-" : "+"), (state->rx_pmd_dp_invert ? "-" : "+"), state->stop_state));
  EFUN_PRINTF((" GEN%1d", state->rate_select));
#else

  if(state->osr_mode.tx_rx != 255) {
      EFUN_PRINTF(("(%s%s%2s%s, 0x%04x, 0x%02x ,", (state->tx_pmd_dp_invert ? "-" : "+"), (state->rx_pmd_dp_invert ? "-" : "+"),

                                                    ((state->CDR) ? "BR" : "OS"),

                                                    state->tx_osr_mode_str, state->UC_CFG,state->UC_STS));
  } else {
      EFUN_PRINTF(("(%s%s%2s%s:%s, 0x%04x, 0x%02x,  ", (state->tx_pmd_dp_invert ? "-" : "+"), (state->rx_pmd_dp_invert ? "-" : "+"), (state->CDR ? "BR" : "OS"),
                                                        state->tx_osr_mode_str, state->rx_osr_mode_str, state->UC_CFG, state->UC_STS));

  }
#if defined(SERDES_TX_RX_SEPARATE)
  EFUN_PRINTF((" %01x,%01x, %02x )",state->tx_rst,state->rx_rst,state->stop_state));
#else
  EFUN_PRINTF((" %01x,  %02x )",state->rx_rst,state->stop_state));
#endif
#endif /* MERLIN7_PCIEG3 */
#ifndef PCIEG3_V1
  EFUN_PRINTF((" %1d%s", state->sig_det, state->sig_det_chg ? "*" : " "));
#else
  EFUN_PRINTF((" %04X,%04X", state->dsc_one_hot_0_0, state->dsc_one_hot_0_1));
  EFUN_PRINTF(("  %1d,%3u ", state->sig_det, state->sig_det_chg));
#endif
  EFUN_PRINTF((" %1d%s", state->pmd_lock, state->pmd_lock_chg ? "* " : "  "));
  EFUN_PRINTF(("%4d ", state->ppm));

  EFUN_PRINTF(("  %3d", state->clk90));
  EFUN_PRINTF(("   %3d ", state->clkp1));
  EFUN_PRINTF(("   %2d,%1d   ", state->pf_main, state->pf2_ctrl));

  EFUN_PRINTF(("%2d ", state->VGA));

  EFUN_PRINTF(("%4d ", state->DCO));
  EFUN_PRINTF(("%4d ", state->P1mV));

  EFUN_PRINTF((" (%3d,%3d,%3d,%3d,%3d,%3d,%3d) ", state->DFE1, state->DFE2, state->DFE3, state->DFE4, state->DFE5, state->dfe1_dcd, state->dfe2_dcd));
  EFUN_PRINTF(("(%3d,%3d,%3d,%3d,%3d,%3d)", state->ze, state->zo, state->pe, state->po, state->me, state->mo));
  EFUN_PRINTF((" %4d%c ", state->tx_ppm, state->tx_pi_en ? '*' : ' '));
#ifndef PCIEG3_V1
  if(state->rmt_lpbk_en){
      EFUN_PRINTF(("   R"));
  }else if(state->dig_lpbk_en){
      EFUN_PRINTF(("   L"));
  }else if(state->linktrn_en){
      EFUN_PRINTF(("   T"));
  }else{
      EFUN_PRINTF(("    "));
  }
  EFUN_PRINTF(("(%2d,%2d,%2d,%2d)  ", state->txfir_pre, state->txfir_main, state->txfir_post1, state->txfir_post2));
#endif
#ifndef PCIEG3_V1
  EFUN_PRINTF((" (%3d,%3d,%3d,%3d) ", state->heye_left, state->heye_right, state->veye_upper, state->veye_lower));
  EFUN_PRINTF((" %2d,%2d  ", state->soc_pos, state->soc_neg));
  /* Check to see if link_time is max value after 80us to 0.1msec unit conversion */
  if (state->link_time == ((0xFFFFL<<MERLIN8_PHY1_LANE_TIMER_SHIFT)/10)) {
      EFUN_PRINTF((" >%4d.%01d", state->link_time/10, state->link_time%10));
      EFUN_PRINTF(("  "));
  } else {
      EFUN_PRINTF((" %4d.%01d", state->link_time/10, state->link_time%10));
      EFUN_PRINTF(("   "));
  }

  EFUN_PRINTF(("%s",state->BER));

#endif /* MERLIN7_PCIEG3 */

Exit:
  USR_FREE(state);
  if(Exit_Status != ERR_CODE_NONE) {
      return merlin8_phy1_error_report(sa__, Exit_Status);
  }

  return (ERR_CODE_NONE);
}  /* merlin8_phy1_INTERNAL_display_lane_state_no_newline(srds_access_t *sa__) */



err_code_t merlin8_phy1_INTERNAL_read_core_state(srds_access_t *sa__, merlin8_phy1_core_state_st *istate) {
  INIT_SRDS_ERR_CODE
  merlin8_phy1_core_state_st state;
  struct merlin8_phy1_uc_core_config_st core_cfg = UC_CORE_CONFIG_INIT;

  ENULL_MEMSET(&state, 0, sizeof(merlin8_phy1_core_state_st));

  if(!istate)
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));

#ifndef PCIEG3_V1
  EFUN(merlin8_phy1_get_uc_core_config(sa__, &core_cfg));
#endif

  /* pllpon_mux is read in case the rescal value is overriden. */
  ESTM(state.rescal = rdc_ams_pll_pllpon_mux());

  ESTM(state.core_reset           = rdc_core_dp_reset_state());
  ESTM(state.uc_active            = rdc_uc_active());
  ESTM(state.comclk_mhz           = rdc_heartbeat_count_1us());
  state.pll_pwrdn            = 0; // Fake value
  ESTM(state.ucode_version        = rdcv_common_ucode_version());
  ESTM(state.ucode_minor_version  = rdcv_common_ucode_minor_version());
  EFUN(merlin8_phy1_version(sa__, &state.api_version));
  ESTM(state.afe_hardware_version = rdcv_afe_hardware_version());
  ESTM(state.temp_idx             = rdcv_temp_idx());
  EFUN(merlin8_phy1_read_die_temperature(sa__, &state.die_temp));
  {
    ESTM(state.avg_tmon             = (int16_t)(_bin_to_degC(rdcv_avg_tmon_reg13bit()>>3)));
  }
  state.vco_rate_mhz              = (uint16_t)core_cfg.vco_rate_in_Mhz;
#ifndef PCIEG3_V1
  EFUN(merlin8_phy1_INTERNAL_read_pll_div(sa__, &state.pll_div));
#endif
  EFUN(merlin8_phy1_INTERNAL_pll_lock_status(sa__, &state.pll_lock, &state.pll_lock_chg));
  state.analog_vco_range = 0x3f; // Fake value
  ESTM(state.core_status          = rdcv_status_byte());
#ifdef PCIEG3_V1
  ESTM(state.rate_select = (uint8_t)(rd_rate_sel()+1));
#endif
  *istate = state;
  return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_display_core_state_no_newline(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
  merlin8_phy1_core_state_st state;
    ENULL_MEMSET(&state     , 0, sizeof(state     ));
    {
        uint8_t pll_orig;
        ESTM(pll_orig = merlin8_phy1_acc_get_pll_idx(sa__));
        EFUN(merlin8_phy1_acc_set_pll_idx(sa__, 0));
        EFUN(merlin8_phy1_INTERNAL_read_core_state(sa__, &state));
        EFUN(merlin8_phy1_acc_set_pll_idx(sa__, pll_orig));
    }

#ifndef PCIEG3_V1
    if ((state.avg_tmon<-50)||(state.avg_tmon>135)) {
      EFUN_PRINTF(("\n*** WARNING: Core die temperature (LIVE_TEMP) out of bounds -50C to 130C\n"));
    }
#endif
    if ((state.rescal < RESCAL_MIN) || (state.rescal > RESCAL_MAX)) {
      EFUN_PRINTF(("\n*** WARNING: RESCAL value is out of bounds %d to %d\n",RESCAL_MIN, RESCAL_MAX));
    }

    EFUN_PRINTF((" %02d "              ,  merlin8_phy1_acc_get_core(sa__)));
#ifndef PCIEG3_V1
    EFUN_PRINTF(("  %x,%02x  "         ,  state.core_reset, state.core_status));
#else
    EFUN_PRINTF(("  %02x  "            ,  state.core_status));
#endif
    EFUN_PRINTF(("    %1d     "        ,  state.pll_pwrdn));
#ifndef PCIEG3_V1
    EFUN_PRINTF(("   %1d    "          ,  state.uc_active));
#endif
    EFUN_PRINTF((" %3d.%02dMHz"        , (state.comclk_mhz/4),((state.comclk_mhz%4)*25)));    /* comclk in Mhz = heartbeat_count_1us / 4 */
    EFUN_PRINTF((" %d_%d_%d"           ,  state.ucode_version >> 8, state.ucode_minor_version, (state.ucode_version >> 4) & 0xF));
    if (state.ucode_version & 0xF)
      EFUN_PRINTF(("_rc%d "            ,  state.ucode_version & 0xF));
    else
      EFUN_PRINTF(("    "));
    EFUN_PRINTF(("%d_%d_%d"            ,  state.api_version >> 24, (state.api_version >> 16) & 0xFF, (state.api_version >> 8) & 0xFF));
    if (state.api_version & 0xFF)
      EFUN_PRINTF(("_rc%d "            ,  state.api_version & 0xFF));
    else
      EFUN_PRINTF(("    "));
    EFUN_PRINTF(("    0x%02X   "       ,  state.afe_hardware_version));
    EFUN_PRINTF(("   %3dC   "          ,  state.die_temp));
#ifndef PCIEG3_V1
    EFUN_PRINTF(("   (%02d)%3dC "      ,  state.temp_idx, state.avg_tmon));
#endif
    EFUN_PRINTF(("   0x%02x  "         ,  state.rescal));
#ifndef PCIEG3_V1
    EFUN_PRINTF(("  %2d.%03dGHz "      ,  state.vco_rate_mhz/1000, state.vco_rate_mhz % 1000));
#else
    EFUN_PRINTF(("  GEN%d "            ,  state.rate_select));
#endif
    EFUN_PRINTF(("    %03d       "     ,  state.analog_vco_range));
#ifndef PCIEG3_V1
    EFUN(merlin8_phy1_INTERNAL_display_pll_to_divider(sa__, state.pll_div));
#endif
    EFUN_PRINTF(("     %01d%s  "       ,  state.pll_lock, state.pll_lock_chg ? "*" : " "));
    return (ERR_CODE_NONE);
  }
#endif /* SMALL_FOOTPRINT */


#ifndef PCIEG3_V1
/* returns 000111 (7 = 8-1), for n = 3  */
#define BFMASK(n) ((1<<((n)))-1)

err_code_t merlin8_phy1_INTERNAL_update_uc_lane_config_st(struct  merlin8_phy1_uc_lane_config_st *st) {
  uint16_t in = st->word;
  st->field.lane_cfg_from_pcs = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.an_enabled = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.dfe_on = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.force_brdfe_on = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.media_type = in & BFMASK(2); in = (uint16_t)(in >> 2);
  st->field.unreliable_los = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.scrambling_dis = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.cl72_auto_polarity_en = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.cl72_restart_timeout_en = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.reserved = in & BFMASK(4); in >>= 4;
  st->field.afe_char_mode_enable = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.channel_est_mode_enable = in & BFMASK(1);
  return(ERR_CODE_NONE);
}
#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_INTERNAL_update_usr_ctrl_disable_functions_st(struct merlin8_phy1_usr_ctrl_disable_functions_st *st) {
  uint8_t in = st->byte;
  st->field.pf_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.dc_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.vga_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.slicer_offset_tuning = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.clk90_offset_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.p1_level_tuning = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.eye_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.all_adaptation = in & BFMASK(1);
  return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_update_usr_ctrl_disable_dfe_functions_st(struct merlin8_phy1_usr_ctrl_disable_dfe_functions_st *st) {
  uint8_t in = st->byte;
  st->field.dfe_tap1_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.dfe_tap2_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.dfe_tap3_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.dfe_tap4_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.dfe_tap5_adaptation = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.dfe_tap1_dcd = in & BFMASK(1); in = (uint8_t)(in >> 1);
  st->field.dfe_tap2_dcd = in & BFMASK(1);
  return ERR_CODE_NONE;
}
#endif /* SMALL_FOOTPRINT */
/* word to fields, for display */
err_code_t merlin8_phy1_INTERNAL_update_uc_core_config_st(struct merlin8_phy1_uc_core_config_st *st) {
  uint16_t in = st->word;
  st->field.core_cfg_from_pcs = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.vco_rate = in & BFMASK(5); in = (uint16_t)(in >> 5);
  st->field.an_los_workaround = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.reserved1 = in & BFMASK(1); in = (uint16_t)(in >> 1);
  st->field.reserved2 = (uint8_t)(in & BFMASK(8));
  st->vco_rate_in_Mhz = VCO_RATE_TO_MHZ(st->field.vco_rate);
  return ERR_CODE_NONE;
}

/* fields to word, to write into uC RAM */
err_code_t merlin8_phy1_INTERNAL_update_uc_core_config_word(struct merlin8_phy1_uc_core_config_st *st) {
  uint16_t in = 0;
  in = (uint16_t)(in << 8); in |= 0/*st->field.reserved2*/ & BFMASK(8);
  in = (uint16_t)(in << 1); in |= 0/*st->field.reserved1*/ & BFMASK(1);
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.an_los_workaround & BFMASK(1)));
  in = (uint16_t)(in << 5); in = (uint16_t)(in | (st->field.vco_rate & BFMASK(5)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.core_cfg_from_pcs & BFMASK(1)));
  st->word = in;
  return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_update_uc_lane_config_word(struct merlin8_phy1_uc_lane_config_st *st) {
  uint16_t in = 0;
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.channel_est_mode_enable & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.afe_char_mode_enable & BFMASK(1)));
  in = (uint16_t)(in << 4); in = (uint16_t)(in | (st->field.reserved & BFMASK(4)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.cl72_restart_timeout_en & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | ( st->field.cl72_auto_polarity_en & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | ( st->field.scrambling_dis & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | ( st->field.unreliable_los & BFMASK(1)));
  in = (uint16_t)(in << 2); in = (uint16_t)(in | ( st->field.media_type & BFMASK(2)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | ( st->field.force_brdfe_on & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dfe_on & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.an_enabled & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.lane_cfg_from_pcs & BFMASK(1)));
  st->word = in;
  return ERR_CODE_NONE;
}


#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_INTERNAL_update_usr_ctrl_disable_functions_byte(struct merlin8_phy1_usr_ctrl_disable_functions_st *st) {
  uint8_t in = 0;
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.all_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.eye_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.p1_level_tuning & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.clk90_offset_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.slicer_offset_tuning & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.vga_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.dc_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.pf_adaptation & BFMASK(1)));
  st->byte = in;
  return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_update_usr_ctrl_disable_dfe_functions_byte(struct  merlin8_phy1_usr_ctrl_disable_dfe_functions_st *st) {
  uint8_t in = 0;
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.dfe_tap2_dcd & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.dfe_tap1_dcd & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.dfe_tap5_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.dfe_tap4_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.dfe_tap3_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.dfe_tap2_adaptation & BFMASK(1)));
  in = (uint8_t)(in << 1); in = (uint8_t)(in | (st->field.dfe_tap1_adaptation & BFMASK(1)));
  st->byte = in;
  return ERR_CODE_NONE;
}
#endif /*SMALL_FOOTPRINT */

#undef BFMASK
#endif /* MERLIN7_PCIEG3 */

err_code_t merlin8_phy1_INTERNAL_check_uc_lane_stopped(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE

  uint8_t is_micro_stopped;
  ESTM(is_micro_stopped = rdv_usr_sts_micro_stopped() || (rd_rx_lane_dp_reset_state() > 0));
  if (!is_micro_stopped) {
      return(merlin8_phy1_error(sa__, ERR_CODE_UC_NOT_STOPPED));
  } else {
      return(ERR_CODE_NONE);
  }
}
#ifndef SMALL_FOOTPRINT

err_code_t merlin8_phy1_INTERNAL_calc_patt_gen_mode_sel(srds_access_t *sa__, uint8_t *mode_sel, uint8_t *zero_pad_len, uint8_t patt_length) {

  if(!mode_sel || !zero_pad_len || !patt_length) {
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
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
    return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_INVALID_PATT_LENGTH));
  }
  return(ERR_CODE_NONE);
}
#endif /*SMALL_FOOTPRINT */

#ifndef PCIEG3_V1
/*-----------------------------------------*/
/*  Write Core Config variables to uC RAM  */
/*-----------------------------------------*/

err_code_t merlin8_phy1_INTERNAL_set_uc_core_config(srds_access_t *sa__, struct merlin8_phy1_uc_core_config_st struct_val) {
    INIT_SRDS_ERR_CODE
    uint8_t pll_orig;
    ESTM(pll_orig = merlin8_phy1_acc_get_pll_idx(sa__));
    EFUN(merlin8_phy1_acc_set_pll_idx(sa__, 0));
    {   uint8_t reset_state;
        ESTM(reset_state = rdc_core_dp_reset_state());
        if(reset_state < 7) {
            EFUN_PRINTF(("ERROR: merlin8_phy1_INTERNAL_set_uc_core_config(..) called without core_dp_s_rstb=0\n"));
            return (merlin8_phy1_error(sa__, ERR_CODE_CORE_DP_NOT_RESET));
        }
    }
    if(struct_val.vco_rate_in_Mhz > 0) {
        struct_val.field.vco_rate = MHZ_TO_VCO_RATE(struct_val.vco_rate_in_Mhz);
    }
    EFUN(merlin8_phy1_INTERNAL_update_uc_core_config_word(&struct_val));
        EFUN(wrcv_config_word(struct_val.word));
    EFUN(merlin8_phy1_acc_set_pll_idx(sa__, pll_orig));
    return (ERR_CODE_NONE);
}

/*---------------------*/
/*  PLL Configuration  */
/*---------------------*/

/** Set refclk pre-divide, adjust div to match */
err_code_t merlin8_phy1_INTERNAL_set_pll_pdiv(srds_access_t *sa__, enum merlin8_phy1_pll_div_enum *div_, uint32_t frac_width)
{
  INIT_SRDS_ERR_CODE

  /* Keep PLL divider in range by using predivider */
  if (SRDS_INTERNAL_GET_PLL_DIV_INTEGER(*div_) > 125)
  {
    /* LSbit of integer part of divider becomes MSbit of fractional part of divider */
    uint32_t temp_bit = (SRDS_INTERNAL_GET_PLL_DIV_INTEGER(*div_) & 1) << (frac_width-1);
    EFUN(wrc_ams_pll_pdiv(0x0)); /* Multiply refclk by 2 */
    *div_ = (enum merlin8_phy1_pll_div_enum) SRDS_INTERNAL_COMPOSE_PLL_DIV((SRDS_INTERNAL_GET_PLL_DIV_INTEGER(*div_) >> 1),(SRDS_INTERNAL_GET_PLL_DIV_FRACTION_NUM(*div_,frac_width) >> 1) | temp_bit, frac_width );
  }
  else
  {
    EFUN(wrc_ams_pll_pdiv(0x1)); /* Multiply refclk by 1, no change id div_ */
  }

  return (ERR_CODE_NONE);
}



/** Extract the refclk frequency in Hz, based on a merlin8_phy1_pll_refclk_enum value. */
err_code_t merlin8_phy1_INTERNAL_get_refclk_in_hz(srds_access_t *sa__, enum merlin8_phy1_pll_refclk_enum refclk, uint32_t *refclk_in_hz) {
    switch (refclk) {
        case MERLIN8_PHY1_PLL_REFCLK_50MHZ:          *refclk_in_hz =  50000000; break;
        case MERLIN8_PHY1_PLL_REFCLK_125MHZ:         *refclk_in_hz = 125000000; break;
        case MERLIN8_PHY1_PLL_REFCLK_156P25MHZ:      *refclk_in_hz = 156250000; break;
        case MERLIN8_PHY1_PLL_REFCLK_161P1328125MHZ: *refclk_in_hz = 161132813; break;
        case MERLIN8_PHY1_PLL_REFCLK_400MHZ:         *refclk_in_hz = 400000000; break;
        default:
            EFUN_PRINTF(("ERROR: Unknown refclk frequency:  0x%08X\n", (uint32_t)refclk));
            *refclk_in_hz = 0xFFFFFFFF;
            return (merlin8_phy1_error(sa__, ERR_CODE_REFCLK_FREQUENCY_INVALID));
    }
    return (ERR_CODE_NONE);
}

/** Identify the ratio:
 *
 *     (numerator / denominator) = (1000 / divisor)
 *
 * such that this has as little rounding error as possible:
 *
 *     refclk_freq_hz = numerator * round(vco_freq_khz / denominator)
 *
 * This will yield the most accurate refclk_freq_hz.
 * Common values of vco_freq_khz are considered in this.
 */
static uint32_t _steins_gcd(uint32_t u, uint32_t v) {
    /* simple cases (termination) */
    if (u == v)
        return u;

    if (u == 0)
        return v;

    if (v == 0)
        return u;

    /* look for factors of 2 */
    if (~u & 1) {                                       /* u is even */
        if (v & 1)                                      /* v is odd */
            return _steins_gcd(u >> 1, v);
        else                                            /* both u and v are even */
            return _steins_gcd(u >> 1, v >> 1) << 1;
    }

    if (~v & 1)                                         /* u is odd, v is even */
        return _steins_gcd(u, v >> 1);

    /* reduce larger argument */
    if (u > v)
        return _steins_gcd((u - v) >> 1, v);

    return _steins_gcd((v - u) >> 1, u);
}

static err_code_t _merlin8_phy1_minimize_divisor_ratio(uint32_t md, uint32_t *simple_numerator, uint32_t *simple_denominator) {
    /* continued fraction coefficients */
    uint32_t a, h[3] = { 0, 1, 0 }, k[3] = { 1, 0, 0 };
    uint32_t x, d, n;
    uint8_t i;

    n = *simple_numerator;
    d = *simple_denominator;

    /* continued fraction and check denominator each step */
    for (i = 0; i < 64; i++) {

        a = (n ? d / n : 0);
        if (i && !a) break;

        /* coverity[divide_by_zero] */
        x = d; d = n; n = x % n;

        x = a;
        if (k[1] * a + k[0] >= md) {
            x = (md - k[0]) / k[1];
            if (x * 2 >= a || k[1] >= md)
                i = 65;
            else
                break;
        }

        h[2] = x * h[1] + h[0]; h[0] = h[1]; h[1] = h[2];
        k[2] = x * k[1] + k[0]; k[0] = k[1]; k[1] = k[2];
    }
    *simple_denominator = k[1];
    *simple_numerator   = h[1];
    return (ERR_CODE_NONE);
}

static err_code_t _merlin8_phy1_get_divisor_ratio(srds_access_t *sa__, enum merlin8_phy1_pll_div_enum srds_div, uint16_t *simple_numerator, uint16_t *simple_denominator){
    /*
       From the documentation above:
       (numerator / denominator) = (1000 / divisor)
       divisor = div_int + div_decimal where div_decimal = (UPPER_FIVE_HEX_DIGITS_OF_ENUM)/(0x100000).
       This is the same as: (numerator / denominator) =  (1000)(0x100000) / ((div_int)*(0x100000) + UPPER_FIVE_HEX_DIGITS_OF_ENUM)
       numerator = 1000(0x100000) and denominator= div_int*0x100000 + UPPER_FIVE_HEX_DIGITS_OF_ENUM
       If we simplify this fraction, we should end up with the same values as the table above
    */

    uint32_t lowest_numerator, lowest_denominator, gcd;

    if (srds_div == MERLIN8_PHY1_PLL_DIV_UNKNOWN){
        EFUN_PRINTF(("ERROR: Unknown divider value:  0x%08X\n", (uint32_t)srds_div));
        *simple_numerator = 0;
        *simple_denominator = 0;
        return (merlin8_phy1_error(sa__, ERR_CODE_PLL_DIV_INVALID));
    }

    lowest_numerator = (1000)*(0x100000);
    lowest_denominator = ((uint32_t)srds_div & 0xFFF)*0x100000 + (((uint32_t)srds_div & 0xFFFFC000)>>12);
    gcd = _steins_gcd(lowest_numerator, lowest_denominator);

    lowest_numerator    = lowest_numerator / gcd;
    lowest_denominator  = lowest_denominator / gcd;

    if (lowest_numerator >= 0x10000){
        _merlin8_phy1_minimize_divisor_ratio(0x10000,&lowest_numerator, &lowest_denominator);
        *simple_numerator   = (uint16_t)lowest_denominator;
        *simple_denominator = (uint16_t)lowest_numerator;
        return (ERR_CODE_NONE);
    }
    *simple_numerator   = (uint16_t)lowest_numerator;
    *simple_denominator = (uint16_t)lowest_denominator;

    return (ERR_CODE_NONE);
}


#ifndef SMALL_FOOTPRINT
/** Get the Refclk frequency in Hz, based on the merlin8_phy1_pll_div_enum value and VCO frequency. */
static err_code_t _merlin8_phy1_get_refclk_from_div_vco(srds_access_t *sa__, uint32_t *refclk_freq_hz, enum merlin8_phy1_pll_div_enum srds_div, uint32_t vco_freq_khz, enum merlin8_phy1_pll_option_enum pll_option) {
    INIT_SRDS_ERR_CODE
    uint16_t numerator, denominator;
    EFUN(_merlin8_phy1_get_divisor_ratio(sa__, srds_div, &numerator, &denominator));
    if (denominator == 0){
        EFUN_PRINTF(("ERROR: Divider returned denominator 0:  0x%08X\n", (uint32_t)srds_div));
        return (merlin8_phy1_error(sa__, ERR_CODE_PLL_DIV_INVALID));
    }
    *refclk_freq_hz = ((vco_freq_khz+(denominator>>1)) / denominator) * numerator;
    if (pll_option == MERLIN8_PHY1_PLL_OPTION_REFCLK_DOUBLER_EN) *refclk_freq_hz /= 2;
    if (pll_option == MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV2_EN)    *refclk_freq_hz *= 2;
    if (pll_option == MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV4_EN)    *refclk_freq_hz *= 4;
    return (ERR_CODE_NONE);
}
#endif /*SMALL_FOOTPRINT */

/** Get the VCO frequency in kHz, based on the reference clock frequency and merlin8_phy1_pll_div_enum value. */
err_code_t merlin8_phy1_INTERNAL_get_vco_from_refclk_div(srds_access_t *sa__, uint32_t refclk_freq_hz, enum merlin8_phy1_pll_div_enum srds_div, uint32_t *vco_freq_khz, enum merlin8_phy1_pll_option_enum pll_option) {
    INIT_SRDS_ERR_CODE
    uint16_t numerator, denominator;
    EFUN(_merlin8_phy1_get_divisor_ratio(sa__, srds_div, &numerator, &denominator));
    if (pll_option == MERLIN8_PHY1_PLL_OPTION_REFCLK_DOUBLER_EN) refclk_freq_hz *= 2;
    if (pll_option == MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV2_EN)    refclk_freq_hz /= 2;
    if (pll_option == MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV4_EN)    refclk_freq_hz /= 4;
    if (numerator == 0){
        EFUN_PRINTF(("ERROR: Divider returned numerator 0:  0x%08X\n", (uint32_t)srds_div));
        return (merlin8_phy1_error(sa__, ERR_CODE_PLL_DIV_INVALID));
    }
    *vco_freq_khz =(uint32_t) (((uint64_t)refclk_freq_hz * denominator + (numerator >> 1)) / numerator);
    return (ERR_CODE_NONE);
}

/* Boundaries for allowed VCO frequency */
#    define SERDES_VCO_FREQ_KHZ_MIN ( 8500000)
#    define SERDES_VCO_FREQ_KHZ_MAX (12500000)

/* Allowed tolerance in resultant VCO frequency when auto-determining divider value */
#    define SERDES_VCO_FREQ_KHZ_TOLERANCE (4000)

#ifndef SMALL_FOOTPRINT
/* The allowed PLL divider values */
static const enum merlin8_phy1_pll_div_enum _merlin8_phy1_div_candidates[] = {
    MERLIN8_PHY1_PLL_DIV_52P751515,
    MERLIN8_PHY1_PLL_DIV_54P4,
    MERLIN8_PHY1_PLL_DIV_58P181818,
    MERLIN8_PHY1_PLL_DIV_60,
    MERLIN8_PHY1_PLL_DIV_62P060606,
    MERLIN8_PHY1_PLL_DIV_64,
    MERLIN8_PHY1_PLL_DIV_66,
    MERLIN8_PHY1_PLL_DIV_66P460703,
    MERLIN8_PHY1_PLL_DIV_66P743079,
    MERLIN8_PHY1_PLL_DIV_67P878788,
    MERLIN8_PHY1_PLL_DIV_68,
    MERLIN8_PHY1_PLL_DIV_68P537598,
    MERLIN8_PHY1_PLL_DIV_68P570764,
    MERLIN8_PHY1_PLL_DIV_68P828796,
    MERLIN8_PHY1_PLL_DIV_68P856242,
    MERLIN8_PHY1_PLL_DIV_69P152458,
    MERLIN8_PHY1_PLL_DIV_69P389964,
    MERLIN8_PHY1_PLL_DIV_69P818182,
    MERLIN8_PHY1_PLL_DIV_70,
    MERLIN8_PHY1_PLL_DIV_70P713596,
    MERLIN8_PHY1_PLL_DIV_71P008,
    MERLIN8_PHY1_PLL_DIV_71P112952,
    MERLIN8_PHY1_PLL_DIV_71P31347,
    MERLIN8_PHY1_PLL_DIV_71P5584,
    MERLIN8_PHY1_PLL_DIV_72,
    MERLIN8_PHY1_PLL_DIV_73P335232,
    MERLIN8_PHY1_PLL_DIV_73P6,
    MERLIN8_PHY1_PLL_DIV_75,
    MERLIN8_PHY1_PLL_DIV_80,
    MERLIN8_PHY1_PLL_DIV_82P5,
    MERLIN8_PHY1_PLL_DIV_85P671997,
    MERLIN8_PHY1_PLL_DIV_86P036,
    MERLIN8_PHY1_PLL_DIV_87P5,
    MERLIN8_PHY1_PLL_DIV_88P392,
    MERLIN8_PHY1_PLL_DIV_88P76,
    MERLIN8_PHY1_PLL_DIV_89P141838,
    MERLIN8_PHY1_PLL_DIV_89P447998,
    MERLIN8_PHY1_PLL_DIV_90,
    MERLIN8_PHY1_PLL_DIV_91P669037,
    MERLIN8_PHY1_PLL_DIV_92,
    MERLIN8_PHY1_PLL_DIV_100,
    MERLIN8_PHY1_PLL_DIV_125,
    MERLIN8_PHY1_PLL_DIV_170,
    MERLIN8_PHY1_PLL_DIV_187P5,
    MERLIN8_PHY1_PLL_DIV_200,
    MERLIN8_PHY1_PLL_DIV_206P25
};

static const uint8_t _merlin8_phy1_div_candidates_count = sizeof(_merlin8_phy1_div_candidates) / sizeof(_merlin8_phy1_div_candidates[0]);

static err_code_t _merlin8_phy1_check_div(srds_access_t *sa__, enum merlin8_phy1_pll_div_enum srds_div) {
    uint8_t i, found = 0;
    for (i=0; i<_merlin8_phy1_div_candidates_count; i++) {
        found = (uint8_t)(found | (srds_div == _merlin8_phy1_div_candidates[i]));
    }
    if (!found) {
        EFUN_PRINTF(("ERROR: Invalid divider value:  0x%08X\n", (uint32_t)srds_div));
        return (merlin8_phy1_error(sa__, ERR_CODE_PLL_DIV_INVALID));
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_check_vco_freq_khz(srds_access_t *sa__, uint32_t vco_freq_khz) {
    uint32_t min_vco_freq = SERDES_VCO_FREQ_KHZ_MIN;
    if (vco_freq_khz < min_vco_freq - SERDES_VCO_FREQ_KHZ_TOLERANCE) {
        EFUN_PRINTF(("ERROR: VCO frequency too low:  %d kHz is lower than minimum (%d kHz)\n", vco_freq_khz, min_vco_freq));
        return (merlin8_phy1_error(sa__, ERR_CODE_VCO_FREQUENCY_INVALID));
    }
    if (vco_freq_khz > SERDES_VCO_FREQ_KHZ_MAX + SERDES_VCO_FREQ_KHZ_TOLERANCE) {
        EFUN_PRINTF(("ERROR: VCO frequency too high:  %d kHz is higher than maximum (%d kHz)\n", vco_freq_khz, SERDES_VCO_FREQ_KHZ_MAX));
        return (merlin8_phy1_error(sa__, ERR_CODE_VCO_FREQUENCY_INVALID));
    }
    return (ERR_CODE_NONE);
}

/** Find the entry out of _merlin8_phy1_div_candidates that is closest to matching refclk_freq_hz and vco_freq_khz. */
static err_code_t _merlin8_phy1_get_div(srds_access_t *sa__, uint32_t refclk_freq_hz, uint32_t vco_freq_khz, enum merlin8_phy1_pll_div_enum *srds_div, enum merlin8_phy1_pll_option_enum pll_option) {
    INIT_SRDS_ERR_CODE
    int32_t vco_freq_khz_min_error = 0x7FFFFFFF;
    uint8_t i;
    for (i=0; i<_merlin8_phy1_div_candidates_count; i++) {
        uint32_t actual_vco_freq_khz = 0;
        int32_t  vco_freq_khz_error;
        EFUN(merlin8_phy1_INTERNAL_get_vco_from_refclk_div(sa__, refclk_freq_hz, _merlin8_phy1_div_candidates[i], &actual_vco_freq_khz, pll_option));
        vco_freq_khz_error = (int32_t)(vco_freq_khz - actual_vco_freq_khz);
        vco_freq_khz_error = SRDS_ABS(vco_freq_khz_error);
        if (vco_freq_khz_min_error > vco_freq_khz_error) {
            vco_freq_khz_min_error = vco_freq_khz_error;
            *srds_div = _merlin8_phy1_div_candidates[i];
        }
    }
    if (vco_freq_khz_min_error == (int32_t)0xFFFFFFFF) {
      return (merlin8_phy1_error(sa__, ERR_CODE_CONFLICTING_PARAMETERS));
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_resolve_pll_parameters(srds_access_t *sa__,
                                                  enum merlin8_phy1_pll_refclk_enum refclk,
                                                  uint32_t *refclk_freq_hz,
                                                  enum merlin8_phy1_pll_div_enum *srds_div,
                                                  uint32_t *vco_freq_khz,
                                                  enum merlin8_phy1_pll_option_enum pll_option) {
    INIT_SRDS_ERR_CODE

    /* Parameter value and consistency checks */
    const uint8_t given_param_count = (uint8_t)(((refclk == MERLIN8_PHY1_PLL_REFCLK_UNKNOWN) ? 0 : 1)
                                       + ((*srds_div == MERLIN8_PHY1_PLL_DIV_UNKNOWN) ? 0 : 1)
                                       + ((*vco_freq_khz == 0) ? 0 : 1));
    const uint32_t original_vco_freq_khz = *vco_freq_khz;
    enum merlin8_phy1_pll_div_enum auto_div = MERLIN8_PHY1_PLL_DIV_UNKNOWN;
    const char*pll_option_e2s[] = {"no", "refclk_x2", "refclk_div2", "refclk_div4"};

    /* Verify that at least two of the three parameters is given. */
    if (given_param_count < 2) {
        return (merlin8_phy1_error(sa__, ERR_CODE_INSUFFICIENT_PARAMETERS));
    }

    /* Skip verification if option is selected. Error if all parameters not given. */
    if ((pll_option & MERLIN8_PHY1_PLL_OPTION_DISABLE_VERIFY) == MERLIN8_PHY1_PLL_OPTION_DISABLE_VERIFY) {
        if (given_param_count < 3) {
            return (merlin8_phy1_error(sa__, ERR_CODE_INSUFFICIENT_PARAMETERS));
        } else {
            /* Calculate "refclk_freq_hz" from the input "refclk" enum (0xNNNMMMMM = Frequency is MMMMM / NNN MHz) */
            *refclk_freq_hz = (uint32_t)((((uint64_t)(refclk & 0xFFFFF)) * 1000000) / ((((uint32_t)refclk) & 0xFFF00000) >> 20));
            return(ERR_CODE_NONE);
        }
    }
    pll_option = (enum merlin8_phy1_pll_option_enum)(pll_option & MERLIN8_PHY1_PLL_OPTION_REFCLK_MASK);

    /* The refclk value is checked in various functions below. */

    /* Verify that the requested div value is allowed. */
    if (*srds_div != MERLIN8_PHY1_PLL_DIV_UNKNOWN) {
        EFUN(_merlin8_phy1_check_div(sa__, *srds_div));
    }

    /* Verify that the requested VCO frequency is allowed. */
    if (*vco_freq_khz != 0) {
        EFUN(merlin8_phy1_INTERNAL_check_vco_freq_khz(sa__,*vco_freq_khz));
    }

    if (refclk == MERLIN8_PHY1_PLL_REFCLK_UNKNOWN) {
        /* Determine refclk from vco frequency and div */
        EFUN(_merlin8_phy1_get_refclk_from_div_vco(sa__, refclk_freq_hz, *srds_div, *vco_freq_khz, pll_option));
    } else {
        EFUN(merlin8_phy1_INTERNAL_get_refclk_in_hz(sa__, refclk, refclk_freq_hz));
    }

    if (*vco_freq_khz == 0) {
        /* Determine VCO frequency from refclk and divider */
        EFUN(merlin8_phy1_INTERNAL_get_vco_from_refclk_div(sa__, *refclk_freq_hz, *srds_div, vco_freq_khz, pll_option));
    }

    /* Determine divider from vco frequency and refclk.
     * This is done even if the div was provided, because if it is,
     * we still want to check whether the parameters are conflicting.
     */
    EFUN(_merlin8_phy1_get_div(sa__, *refclk_freq_hz, *vco_freq_khz, &auto_div, pll_option));
    if (*srds_div == MERLIN8_PHY1_PLL_DIV_UNKNOWN) {
        /* Use the auto-determined divider value, since the divider was not supplied. */
        *srds_div = auto_div;

        /* Determine resultant VCO frequency from refclk and divider */
        EFUN(merlin8_phy1_INTERNAL_get_vco_from_refclk_div(sa__, *refclk_freq_hz, *srds_div, vco_freq_khz, pll_option));
    }

    /* Verify the resultant VCO frequency */
    EFUN(merlin8_phy1_INTERNAL_check_vco_freq_khz(sa__, *vco_freq_khz));

    /* Verify that the requested VCO frequency is delivered. */
    if ((original_vco_freq_khz != 0)
        && ((*vco_freq_khz < original_vco_freq_khz - SERDES_VCO_FREQ_KHZ_TOLERANCE)
            || (*vco_freq_khz > original_vco_freq_khz + SERDES_VCO_FREQ_KHZ_TOLERANCE))) {
        EFUN_PRINTF(("ERROR: Could not achieve requested VCO frequency of %d kHz.\n       Refclk is %d Hz, %s option enabled, and auto-determined divider is 0x%08X, yielding a VCO frequency of %d kHz.\n",
                     original_vco_freq_khz, *refclk_freq_hz, pll_option_e2s[pll_option], *srds_div, *vco_freq_khz));
        return (merlin8_phy1_error(sa__, ERR_CODE_VCO_FREQUENCY_INVALID));
    }

    /* Verify the auto-determined divider value. */
    if (auto_div != *srds_div) {
        EFUN_PRINTF(("ERROR: Conflicting PLL parameters:  refclk is %d Hz, %s option enabled, divider is 0x%08X, and VCO frequency is %d kHz.\n       Expected divider is 0x%08X\n",
                     *refclk_freq_hz, pll_option_e2s[pll_option], *srds_div, *vco_freq_khz, auto_div));
        return (merlin8_phy1_error(sa__, ERR_CODE_CONFLICTING_PARAMETERS));
    }

    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_display_pll_to_divider(srds_access_t *sa__, uint32_t srds_div) {
    /* Adjust these to increase or decrease the number of digits to the right
     * of the decimal point.
     */
    const uint8_t decimal_digits = 4;
    const uint32_t ten_to_the_decimal_digits = 10000; /* 10**decimal_digits */

    /* Not a const, because of carry logic below. */
    uint16_t div_integer = SRDS_INTERNAL_GET_PLL_DIV_INTEGER(srds_div);

    if (SRDS_INTERNAL_IS_PLL_DIV_FRACTIONAL(srds_div)) {
        /* fraction_num will have this many bits (ending at bit 0). */
        const uint8_t  fraction_num_width = 28;
        const uint32_t fraction_num =(uint32_t)(SRDS_INTERNAL_GET_PLL_DIV_FRACTION_NUM(srds_div, fraction_num_width));

        /* Identify the number that, when printed with left-padded zeros,
         * becomes the digits to the right of the decimal point.
         *
         * This value can be obtained by dividing fraction_num by:
         *
         *     (2^fraction_num_width)/(10^decimal_digits)
         */
        const uint32_t divisor = (((1U << fraction_num_width) + (ten_to_the_decimal_digits>>1))
                                  / ten_to_the_decimal_digits);
        uint32_t fraction_as_int = (fraction_num + (divisor>>1)) / divisor;

        /* In case the rounding above caused the fractional portion to overflow
         * (e.g. 4.9999999999 becomes 5.0000), implement the carry into the
         * integer portion.
         */
        if (fraction_as_int >= ten_to_the_decimal_digits) {
            fraction_as_int -= ten_to_the_decimal_digits;
            ++div_integer;
        }

        EFUN_PRINTF(("%3d.%0*d", div_integer, decimal_digits, fraction_as_int));
    } else {
        const uint8_t left_spaces = (decimal_digits + 1) >> 1;
        EFUN_PRINTF(("%*s%3d%*s", left_spaces, "", div_integer, decimal_digits - left_spaces + 1, ""));
    }
    return(ERR_CODE_NONE);
}
#endif /* SMALL_FOOTPRINT */
#endif /* MERLIN7_PCIEG3 */
err_code_t merlin8_phy1_INTERNAL_print_uc_dsc_error(srds_access_t *sa__, enum srds_pmd_uc_cmd_enum cmd) {
    INIT_SRDS_ERR_CODE
    uint32_t supp_info;

    ESTM(supp_info = (rd_uc_dsc_supp_info()) & 0xFF);
    switch (SUPP_INFO_TO_ERROR_CODE(supp_info)) {
      case UC_CMD_ERROR_INVALID_COMMAND:
        ESTM_PRINTF(("ERROR : UC reported invalid command %d.  (other_info = 0x%X)\n",
                     cmd, SUPP_INFO_TO_OTHER_INFO(supp_info)));
        break;
      case UC_CMD_ERROR_BUSY:
        ESTM_PRINTF(("ERROR : UC reported busy for command %d.  (other_info = 0x%X)\n",
                     cmd, SUPP_INFO_TO_OTHER_INFO(supp_info)));
        break;
      case UC_CMD_ERROR_GET_EYE_SAMPLE_ERROR:
        ESTM_PRINTF(("ERROR : UC reported error in getting eye sample.  (command %d, other_info = 0x%X)\n",
                     cmd, SUPP_INFO_TO_OTHER_INFO(supp_info)));
        break;
      case UC_CMD_ERROR_PRBS_NOT_LOCKED:
        ESTM_PRINTF(("ERROR : UC reported PRBS not locked.  (command %d, other_info = 0x%X)\n",
                     cmd, SUPP_INFO_TO_OTHER_INFO(supp_info)));
        break;
      case UC_CMD_ERROR_COMMAND_IN_PROGRESS:
        ESTM_PRINTF(("ERROR : UC reported command already in progress.  (command %d, other_info = 0x%X)\n",
                     cmd, SUPP_INFO_TO_OTHER_INFO(supp_info)));
        break;
      case UC_CMD_ERROR_INVALID_MODE:
        ESTM_PRINTF(("ERROR : UC reported invalid mode for command %d.  (other_info = 0x%X)\n",
                     cmd, SUPP_INFO_TO_OTHER_INFO(supp_info)));
        break;
      default:
        ESTM_PRINTF(("ERROR : UC reported unknown error 0x%X for command %d.  (other_info = 0x%X)\n",
                     SUPP_INFO_TO_ERROR_CODE(supp_info), cmd, SUPP_INFO_TO_OTHER_INFO(supp_info)));
    }
    /* Cleanup cmd register */
    EFUN(reg_wr_DSC_A_DSC_UC_CTRL(0x80));
    EFUN(wr_uc_dsc_data(0));

    return(ERR_CODE_NONE);
}
/******************************************/
/*  Serdes Register field Poll functions  */
/******************************************/

#ifndef CUSTOM_REG_POLLING
#ifndef SMALL_FOOTPRINT
/* poll for microcontroller to populate the dsc_data register */
err_code_t merlin8_phy1_INTERNAL_poll_diag_done(srds_access_t *sa__, uint16_t *status, uint32_t timeout_ms) {
    INIT_SRDS_ERR_CODE
    uint8_t loop;

    if(!status) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    for(loop=0;loop < 100; loop++) {
        ESTM(*status=rdv_usr_diag_status());

        if((*status & 0x8000) > 0) {
            return(ERR_CODE_NONE);
        }
        if(loop>10) {
            EFUN(USR_DELAY_US(10*timeout_ms));
        }
    }
 return(merlin8_phy1_error(sa__, ERR_CODE_DIAG_TIMEOUT));
}
#endif /*SMALL_FOOTPRINT */


/** Poll for field "uc_dsc_ready_for_cmd" = 1 [Return Val => Error_code (0 = Polling Pass)] */
err_code_t merlin8_phy1_INTERNAL_poll_uc_dsc_ready_for_cmd_equals_1(srds_access_t *sa__, uint32_t timeout_ms, enum srds_pmd_uc_cmd_enum cmd)
{
    INIT_SRDS_ERR_CODE
  {
    /* read quickly for 10 tries */
    uint16_t loop = 0;
    uint16_t reset_state;

    for (loop = 0; loop < 100; loop++)
    {
        uint16_t rddata;
        ESTM(rddata = reg_rd_DSC_A_DSC_UC_CTRL());
        if (ex_DSC_A_DSC_UC_CTRL__uc_dsc_ready_for_cmd(rddata)) {
          if (ex_DSC_A_DSC_UC_CTRL__uc_dsc_error_found(rddata)) {
                EFUN(merlin8_phy1_INTERNAL_print_uc_dsc_error(sa__, cmd));
                return(merlin8_phy1_error(sa__, ERR_CODE_UC_CMD_RETURN_ERROR));
            }
            return (ERR_CODE_NONE);
        }
        if(loop>10) {
            EFUN(USR_DELAY_US(10*timeout_ms));
        }
    }
    /* Check if ln_s_rstb is asserted by looking at the reset state, then give a warning and display the commands */
    ESTM(reset_state = rd_lane_dp_reset_state());
    if (reset_state & 0x0007) {
        EFUN_PRINTF(("DSC ready for command is not working; SerDes lane is probably reset!\n"));
        EFUN(reg_wr_DSC_A_DSC_UC_CTRL(0x80)); /* Cleanup after failure */
        return (ERR_CODE_NONE);
    }
    {
        EFUN_PRINTF(("%s ERROR : DSC ready for command is not working, applying workaround and getting debug info !\n", API_FUNCTION_NAME));
        /* print the triage info and reset the cmd interface */
        merlin8_phy1_INTERNAL_print_triage_info(sa__, ERR_CODE_UC_CMD_POLLING_TIMEOUT, 1, 1, (uint16_t)__LINE__);
        /* artifically terminate the command to re-enable the command interface */
        EFUN(wr_uc_dsc_ready_for_cmd(0x1));
    }
    return (ERR_CODE_UC_CMD_POLLING_TIMEOUT);
  }
}


#ifndef SMALL_FOOTPRINT
/* Poll for field "dsc_state" = DSC_STATE_UC_TUNE [Return Val => Error_code (0 = Polling Pass)] */
err_code_t merlin8_phy1_INTERNAL_poll_dsc_state_equals_uc_tune(srds_access_t *sa__, uint32_t timeout_ms) {
    INIT_SRDS_ERR_CODE
    uint16_t loop;
    uint16_t dsc_state;
    /* poll 10 times to avoid longer delays later */
    for (loop = 0; loop < 100; loop++) {
        ESTM(dsc_state = rd_dsc_state());
        if (dsc_state == DSC_STATE_UC_TUNE) {
            return (ERR_CODE_NONE);
        }
        if(loop>10) {
            EFUN(USR_DELAY_US(10*timeout_ms));
        }
    }
    ESTM_PRINTF(("DSC_STATE = %d\n", rd_dsc_state()));
    return (merlin8_phy1_error(sa__, ERR_CODE_POLLING_TIMEOUT));          /* Error Code for polling timeout */
}

#endif /*SMALL_FOOTPRINT */

/* Poll for field "micro_ra_initdone" = 1 [Return Val => Error_code (0 = Polling Pass)] */
err_code_t merlin8_phy1_INTERNAL_poll_micro_ra_initdone(srds_access_t *sa__, uint32_t timeout_ms)
{
    INIT_SRDS_ERR_CODE
    uint16_t loop, max_count;
    uint8_t result;

    max_count = 333;
    for (loop = 0; loop <= max_count; loop++) {
        ESTM(result = rdc_micro_ra_initdone());
        if (result) {
            return (ERR_CODE_NONE);
        }
        EFUN(USR_DELAY_US(3*timeout_ms));
    }
    return (merlin8_phy1_error(sa__, ERR_CODE_POLLING_TIMEOUT));          /* Error Code for polling timeout */
}

#endif /* CUSTOM_REG_POLLING */

uint8_t merlin8_phy1_INTERNAL_is_big_endian(void) {
    uint32_t one_u32 = 0x01000000;
    char * ptr = (char *)(&one_u32);
    const uint8_t big_endian = (ptr[0] == 1);
    return big_endian;
}

err_code_t merlin8_phy1_INTERNAL_rdblk_callback(srds_access_t *sa__, void *arg, uint8_t byte_count, uint16_t data) {
    /* Never call merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check() here */

    merlin8_phy1_INTERNAL_rdblk_callback_arg_t * const cast_arg = (merlin8_phy1_INTERNAL_rdblk_callback_arg_t *)arg;
    UNUSED(sa__);
    *(cast_arg->mem_ptr + get_endian_offset(cast_arg->mem_ptr)) = (uint8_t)(data & 0xFF);
    cast_arg->mem_ptr++;
    if (byte_count == 2) {
        *(cast_arg->mem_ptr + get_endian_offset(cast_arg->mem_ptr)) = (uint8_t)(data >> 8);
        cast_arg->mem_ptr++;
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_ram_dump_callback(srds_access_t *sa__, void *arg, uint8_t byte_count, uint16_t data) {
    merlin8_phy1_INTERNAL_ram_dump_state_t * const state_ptr = (merlin8_phy1_INTERNAL_ram_dump_state_t *)arg;
    const uint8_t bytes_per_row=26;

    if (byte_count == 0) {
        if (state_ptr->line_start_index != state_ptr->index) {
            EFUN_PRINTF(("%*s    %d\n", 4*(bytes_per_row - state_ptr->index + state_ptr->line_start_index), "", state_ptr->line_start_index));
        }
        return (ERR_CODE_NONE);
    }
    if (byte_count == 1) {
#if defined(SMALL_FOOTPRINT)
        /* coverity[assigned_value] */
#endif
        data &= 0xFF;
    }

    if (state_ptr->index % bytes_per_row == 0) {
        state_ptr->line_start_index = state_ptr->index;
        EFUN_PRINTF(("\n%04x ", state_ptr->line_start_index));

    }
    EFUN_PRINTF(("%02x ", (uint8_t)(data & 0xFF)));
    EFUN_PRINTF(("%02x ", (data >> 8)));
    state_ptr->index = (uint16_t)(state_ptr->index + 2);

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_rdblk_ram_callback(srds_access_t *sa__, void *arg, uint8_t byte_count, uint16_t data) {
    INIT_SRDS_ERR_CODE
    srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__); /* Needed for get_endian_offset_info_ptr() */
    merlin8_phy1_INTERNAL_rdblk_ram_arg_t *  cast_arg = (merlin8_phy1_INTERNAL_rdblk_ram_arg_t *)arg;
    merlin8_phy1_INTERNAL_ram_dump_state_t * const state_ptr = (merlin8_phy1_INTERNAL_ram_dump_state_t *)(cast_arg->dump_state_ptr);
    SRDS_INFO_PTR_NULL_CHECK;

    if(cast_arg->ram_buff != NULL) {
        const uint8_t bytes_per_row = 26;
        if (state_ptr->index % bytes_per_row == 0) {
            state_ptr->line_start_index = (state_ptr->index);
            if (state_ptr->index != 0){
                state_ptr->ram_idx++;
                state_ptr->count = 0;
            }

            state_ptr->count += USR_SNPRINTF((cast_arg->ram_buff + get_endian_offset_info_ptr(cast_arg->ram_buff)+(state_ptr->ram_idx * SRDS_DUMP_BUF_SIZE )+ state_ptr->count), (size_t)(SRDS_DUMP_BUF_SIZE - state_ptr->count - 1), "%04x ",(state_ptr->line_start_index) );

        }
        state_ptr->count += USR_SNPRINTF((cast_arg->ram_buff + get_endian_offset_info_ptr(cast_arg->ram_buff)+(state_ptr->ram_idx * SRDS_DUMP_BUF_SIZE )+ state_ptr->count), (size_t)(SRDS_DUMP_BUF_SIZE - state_ptr->count - 1), "%02x ", (uint8_t)(data&0xFF));

        state_ptr->index++;
        if (byte_count == 2) {
            state_ptr->count += USR_SNPRINTF((cast_arg->ram_buff + get_endian_offset_info_ptr(cast_arg->ram_buff)+(state_ptr->ram_idx * SRDS_DUMP_BUF_SIZE)+ state_ptr->count), (size_t)(SRDS_DUMP_BUF_SIZE - state_ptr->count - 1), "%02x ",(uint8_t)(data>>8));
            state_ptr->index++;
        }

        if(state_ptr->index == cast_arg->ram_size){
            state_ptr->count = 0;
            state_ptr->ram_idx++;
            *(cast_arg->ram_buff + get_endian_offset_info_ptr(cast_arg->ram_buff)+(state_ptr->ram_idx * SRDS_DUMP_BUF_SIZE) + state_ptr->count) = 0xA;
            state_ptr->ram_idx++;
            *(cast_arg->ram_buff + get_endian_offset_info_ptr(cast_arg->ram_buff)+(state_ptr->ram_idx * SRDS_DUMP_BUF_SIZE) + state_ptr->count) = 0x0;

        }

    }

    if (cast_arg->ram_buff == NULL) {
        EFUN(merlin8_phy1_INTERNAL_ram_dump_callback(sa__, cast_arg->dump_state_ptr, byte_count,data));
    }


    return (ERR_CODE_NONE);
}

/************************************************************/
/*      Serdes IP RAM access - Lane RAM Variables           */
/*----------------------------------------------------------*/
/*   - through Micro Register Interface for PMD IPs         */
/*   - through Serdes FW DSC Command Interface for Gallardo */
/************************************************************/

/* This function returns the absolute address of lane RAM variables given the offset address and lane */
uint32_t _merlin8_phy1_INTERNAL_get_addr_from_lane(srds_access_t *sa__, uint16_t addr, uint8_t lane) {
    uint32_t lane_var_ram_base=0, lane_var_ram_size=0;
    uint32_t lane_addr_offset = 0;
    uint16_t grp_ram_size=0,lane_count=0;
    srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);

    if (merlin8_phy1_info_ptr != NULL) {
        lane_var_ram_base = merlin8_phy1_info_ptr->lane_var_ram_base;
        lane_var_ram_size = merlin8_phy1_info_ptr->lane_var_ram_size;
        grp_ram_size = merlin8_phy1_info_ptr->grp_ram_size;
        lane_count = merlin8_phy1_info_ptr->lane_count;
        if(!lane_count) {
            return lane_addr_offset; /* To avoid a modulo by 0 exception */
        }
        lane_addr_offset = (uint32_t)(lane_var_ram_base+addr+ ((uint32_t)(lane%lane_count)*lane_var_ram_size) + (uint32_t)(grp_ram_size*merlin8_phy1_INTERNAL_grp_idx_from_lane(lane)));
    }
    return(lane_addr_offset);
}


 /************************************************************/
 /*      Serdes IP RAM access - Core RAM Variables           */
 /*----------------------------------------------------------*/
 /*   - through Micro Register Interface for PMD IPs         */
 /*   - through Serdes FW DSC Command Interface for Gallardo */
 /************************************************************/

 /* This function returns the absolute address of core RAM variables given the offset address and lane */
 uint32_t _merlin8_phy1_INTERNAL_get_addr_from_core(srds_access_t *sa__, uint16_t addr) {
     uint32_t core_var_ram_base=0;
     uint32_t core_addr_offset = 0;
     srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);

     if (merlin8_phy1_info_ptr != NULL) {
         core_var_ram_base = merlin8_phy1_info_ptr->core_var_ram_base;
         core_addr_offset = core_var_ram_base + addr;
     }
     return(core_addr_offset);
}


err_code_t merlin8_phy1_INTERNAL_rdblk_ram_read_callback(srds_access_t * sa__,void *arg,uint8_t byte_count,uint16_t data) {
    INIT_SRDS_ERR_CODE
    srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__); /* Needed for get_endian_offset_info_ptr() */
    merlin8_phy1_INTERNAL_rdblk_ram_arg_t * const cast_arg = (merlin8_phy1_INTERNAL_rdblk_ram_arg_t *)arg;
    SRDS_INFO_PTR_NULL_CHECK;
    if(cast_arg->mem_ptr != NULL) {
        *(cast_arg->mem_ptr + get_endian_offset_info_ptr(cast_arg->mem_ptr)) = (uint8_t)(data & 0xFF);
        cast_arg->mem_ptr++;
        if (byte_count == 2) {
            *(cast_arg->mem_ptr + get_endian_offset_info_ptr(cast_arg->mem_ptr)) = (uint8_t)(data >> 8);
            cast_arg->mem_ptr++;
        }
    }
    if (cast_arg->mem_ptr == NULL) {
        EFUN(merlin8_phy1_INTERNAL_ram_dump_callback(sa__, cast_arg->dump_state_ptr, byte_count,data));
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(srds_access_t *sa__,
                                                uint32_t block_addr,
                                                uint32_t block_size,
                                                uint32_t start_offset,
                                                uint32_t cnt,
                                                void *arg,
                                                err_code_t (*callback)(srds_access_t *, void *, uint8_t, uint16_t)) {
    /* Never call merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check() here */

    INIT_SRDS_ERR_CODE
    err_code_t Exit_Status = ERR_CODE_NONE;
    uint32_t read_val = 0;
    uint8_t defecit = 0;
    uint32_t addr = block_addr + start_offset;
    if (cnt == 0) {
        return (ERR_CODE_NONE);
    }

    /* Check for bad start offset and block size. */
    if (start_offset >= block_size) {
        return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }


    while (cnt > 0) {
        /* Determine how many bytes to read before wrapping back to start of block. */
        uint32_t block_cnt = (uint32_t)(SRDS_MIN(cnt, block_addr + block_size - addr));
        cnt = cnt - block_cnt;

            /* Set up the word reads. */
            CFUN(wrc_micro_autoinc_rdaddr_en(1));
            CFUN(wrc_micro_ra_rddatasize(0x1));                     /* Select 16bit read datasize */
            CFUN(wrc_micro_ra_rdaddr_msw((uint16_t)(addr >> 16)));              /* Upper 16bits of RAM address to be read */
            CFUN(wrc_micro_ra_rdaddr_lsw(addr & 0xFFFE));           /* Lower 16bits of RAM address to be read */

            /* Read the leading byte, if starting at an odd address. */
            if ((addr & 1) == 1) {
                CSTM(read_val |= (uint32_t)((rdc_micro_ra_rddata_lsw() >> 8) << defecit));
                if (defecit == 8) {
                    CFUN(callback(sa__, arg, 2, (uint16_t)read_val));
                    read_val = 0;
                }
                /* We just read a byte.  This toggles the defecit from 0 to 8 or from 8 to 0. */
                defecit ^= 8;
                --block_cnt;
            }

            /* Read the whole words, and call the callback with two bytes at a time. */
            while (block_cnt >= 2) {
                CSTM(read_val |= (uint32_t)(rdc_micro_ra_rddata_lsw() << defecit));

                CFUN(callback(sa__, arg, 2, (uint16_t)read_val));
                read_val >>= 16;
                /* We just read two bytes.  This preserves whatever defecit (8 or 0) is there. */
                block_cnt -= 2;
            }

            /* Read the trailing byte, if leftover after reading whole words. */
            if (block_cnt > 0) {
                CSTM(read_val |= (uint32_t)((rdc_micro_ra_rddata_lsw() & 0xFF) << defecit));
                if (defecit == 8) {
                    CFUN(callback(sa__, arg, 2, (uint16_t)read_val));
                    read_val = 0;
                }
                /* We just read a byte.  This toggles the defecit from 0 to 8 or from 8 to 0. */
                defecit ^= 8;
            }
        addr = block_addr;
    }

    /* If a final byte is left behind, then call the callback with it. */
    if (defecit > 0) {
        CFUN(callback(sa__, arg, 1, (uint16_t)read_val));
    }

Exit:
    return(Exit_Status);
}


#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_INTERNAL_rdblk_uc_generic_ram_descending(srds_access_t *sa__,
                                                           uint32_t block_addr,
                                                           uint32_t block_size,
                                                           uint32_t start_offset,
                                                           uint32_t cnt,
                                                           void *arg,
                                                           err_code_t (*callback)(srds_access_t *, void *, uint8_t, uint16_t)) {
    INIT_SRDS_ERR_CODE
    uint32_t read_val = 0;
    uint8_t defecit = 0;
    uint32_t addr = block_addr + start_offset;
    uint16_t configured_addr_msw = (uint16_t)((addr >> 16) + 1);

    if (cnt == 0) {
        return (ERR_CODE_NONE);
    }

    /* Check for bad start offset and block size. */
    if (start_offset >= block_size) {
        return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }

    EFUN(wrc_micro_autoinc_rdaddr_en(0));
    EFUN(wrc_micro_ra_rddatasize(0x1));                         /* Select 16bit read datasize */

    while (cnt > 0) {
        /* Determine how many bytes to read before wrapping back to end of block. */
        uint32_t block_cnt = (uint32_t)(SRDS_MIN(cnt, start_offset+1));
        cnt = cnt - block_cnt;

        while (block_cnt > 0) {
            const uint16_t addr_msw = (uint16_t)(addr >> 16);
            uint16_t read_val2;
            if (addr_msw != configured_addr_msw) {
                EFUN(wrc_micro_ra_rdaddr_msw(addr_msw));        /* Upper 16bits of RAM address to be read */
                configured_addr_msw = addr_msw;
            }

            EFUN(wrc_micro_ra_rdaddr_lsw(addr & 0xFFFE));       /* Lower 16bits of RAM address to be read */
            ESTM(read_val2 = rdc_micro_ra_rddata_lsw());

            if (((addr & 1) == 1) && (block_cnt >= 2)) {
                /* Reading two bytes.  Since we're reading in descending address order, they
                 * will be reversed before they are sent out. */
                read_val = read_val | (uint32_t)((((read_val2 & 0xFF) << 8) | (read_val2 >> 8)) << defecit);
                EFUN(callback(sa__, arg, 2, (uint16_t)read_val));
                read_val >>= 16;
                /* We just read two bytes.  This preserves whatever defecit (8 or 0) is there. */
                block_cnt -= 2;
                addr -= 2;
            }
            else {
                if ((addr & 1) == 1) {
                    /* Reading upper byte of word. */
                    read_val = read_val | (uint32_t)((read_val2 >> 8) << defecit);
                } else {
                    /* Reading lower byte of word. */
                    read_val = read_val | (uint32_t)((read_val2 & 0xFF) << defecit);
                }
                if (defecit == 8) {
                    EFUN(callback(sa__, arg, 2, (uint16_t)read_val));
                    read_val = 0;
                }
                /* We just read a byte.  This toggles the defecit from 0 to 8 or from 8 to 0. */
                defecit ^= 8;
                --block_cnt;
                --addr;
            }
        }

        addr = block_addr + block_size - 1;
        start_offset = block_size - 1;
    }

    /* If a final byte is left behind, then call the callback with it. */
    if (defecit > 0) {
        EFUN(callback(sa__, arg, 1, (uint16_t)read_val));
    }

    return(ERR_CODE_NONE);
}
#endif /*SMALL_FOOTPRINT */

uint8_t merlin8_phy1_INTERNAL_grp_idx_from_lane(uint8_t lane) {
    UNUSED(lane);
    return(0);
}


err_code_t merlin8_phy1_INTERNAL_display_info_table (srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
    EFUN(merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr));

    EFUN_PRINTF(("\n********** SERDES INFO TABLE DUMP **********\n"));
    EFUN_PRINTF(("signature                 = 0x%X\n", merlin8_phy1_info_ptr->signature));
    EFUN_PRINTF(("diag_mem_ram_base         = 0x%X\n", merlin8_phy1_info_ptr->diag_mem_ram_base));
    EFUN_PRINTF(("diag_mem_ram_size         = 0x%X\n", merlin8_phy1_info_ptr->diag_mem_ram_size));
    EFUN_PRINTF(("core_var_ram_base         = 0x%X\n", merlin8_phy1_info_ptr->core_var_ram_base));
    EFUN_PRINTF(("core_var_ram_size         = 0x%X\n", merlin8_phy1_info_ptr->core_var_ram_size));
    EFUN_PRINTF(("lane_var_ram_base         = 0x%X\n", merlin8_phy1_info_ptr->lane_var_ram_base));
    EFUN_PRINTF(("lane_var_ram_size         = 0x%X\n", merlin8_phy1_info_ptr->lane_var_ram_size));
    EFUN_PRINTF(("trace_mem_ram_base        = 0x%X\n", merlin8_phy1_info_ptr->trace_mem_ram_base));
    EFUN_PRINTF(("trace_mem_ram_size        = 0x%X\n", merlin8_phy1_info_ptr->trace_mem_ram_size));
    EFUN_PRINTF(("micro_var_ram_base        = 0x%X\n", merlin8_phy1_info_ptr->micro_var_ram_base));
    EFUN_PRINTF(("lane_count                = 0x%X\n", merlin8_phy1_info_ptr->lane_count));
    EFUN_PRINTF(("trace_memory_descending_writes = 0x%X\n", merlin8_phy1_info_ptr->trace_memory_descending_writes));
    EFUN_PRINTF(("micro_count               = 0x%X\n", merlin8_phy1_info_ptr->micro_count));
    EFUN_PRINTF(("micro_var_ram_size        = 0x%X\n", merlin8_phy1_info_ptr->micro_var_ram_size));
    EFUN_PRINTF(("grp_ram_size              = 0x%X\n", merlin8_phy1_info_ptr->grp_ram_size));
    EFUN_PRINTF(("ucode_version             = 0x%X\n", merlin8_phy1_info_ptr->ucode_version));
    EFUN_PRINTF(("info_table_version        = 0x%X\n", merlin8_phy1_info_ptr->info_table_version));
    EFUN_PRINTF(("is_big_endian             = 0x%X\n", merlin8_phy1_info_ptr->is_big_endian));

    return (ERR_CODE_NONE);
}

#ifndef SMALL_FOOTPRINT
/****************/
/*  DBstop API  */
/****************/
static err_code_t _print_breakpoint_warning(srds_access_t *sa__) {
    EFUN_PRINTF(("*******************************************************************************\n"));
    EFUN_PRINTF(("* WARNING: SRDS_BREAKPOINT functionality is being used.                       *\n"));
    EFUN_PRINTF(("*                                                                             *\n"));
    EFUN_PRINTF(("*          Use of this feature may affect SERDES performance!                 *\n"));
    EFUN_PRINTF(("*******************************************************************************\n"));
    return (ERR_CODE_NONE);
}
err_code_t merlin8_phy1_INTERNAL_en_breakpoint(srds_access_t *sa__, uint8_t breakpoint) {
    INIT_SRDS_ERR_CODE
    EFUN(_print_breakpoint_warning(sa__));
    EFUN(wrv_usr_dbstop_enable(breakpoint));
    return (ERR_CODE_NONE);
}
err_code_t merlin8_phy1_INTERNAL_goto_breakpoint(srds_access_t *sa__, uint8_t breakpoint) {
    INIT_SRDS_ERR_CODE
    uint8_t already_stopped;

    ESTM(already_stopped = rdv_usr_dbstopped());      /* find out if breakpoint is already reached */
    EFUN(wrv_usr_dbstop_enable(breakpoint));
    if (already_stopped) {WRV_USR_DBSTOPPED_TO_0;}    /* if already stopped, let it continue */
    return (ERR_CODE_NONE);
}
err_code_t merlin8_phy1_INTERNAL_rd_breakpoint(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint8_t val;
    uint8_t core;
    uint8_t lane;
#ifdef SMALL_FOOTPRINT
    UNUSED(core);
    UNUSED(lane);
#endif
    ESTM(core = merlin8_phy1_acc_get_core(sa__));
    ESTM(lane = merlin8_phy1_acc_get_lane(sa__));
    ESTM(val = rdv_usr_dbstopped());
    if(val > 0) {
        EFUN_PRINTF(("Stopped at Breakpoint %d on Core=%d, Lane=%d\n", val, core, lane));
    }
    else {
        EFUN_PRINTF(("Not currently stopped at a breakpoint.\n"));
    }
    return (ERR_CODE_NONE);
}
err_code_t merlin8_phy1_INTERNAL_dis_breakpoint(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    EFUN(wrv_usr_dbstop_enable(0));
    WRV_USR_DBSTOPPED_TO_0;
    return (ERR_CODE_NONE);
}

#endif /* SMALL_FOOTPRINT */



err_code_t merlin8_phy1_INTERNAL_fit_second_order(srds_access_t *sa__, uint8_t n, USR_DOUBLE *x, USR_DOUBLE *y, USR_DOUBLE *a) {
#ifdef SERDES_API_FLOATING_POINT
    uint8_t    i, j, k;
    USR_DOUBLE Ex[5]     = { 0.0 }; /* elements of normal matrix */
    USR_DOUBLE Eb[3][4]  = { {0.0} }; /* augmented matrix */
    USR_DOUBLE Ey[3]     = { 0.0 };
    UNUSED(sa__);

    if ( x == NULL || y == NULL || a == NULL ) {
        return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    if ( n < 2 ) {
        EFUN_PRINTF(("Not enough samples.\n"));
        return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }

    for (i=0; i<=4; i++) {
        Ex[i] = 0;
        for (j=0; j<n; j++) {
            Ex[i] = Ex[i] + pow(x[j], i);
        }
    }

    for (i=0; i<=2; i++) {
        Ey[i] = 0;
        for (j=0; j<n; j++) {
            Ey[i] = Ey[i] + pow(x[j], i) * y[j];
        }
    }

    for (i=0; i<=2; i++) {
        for (j=0; j<=2; j++) {
            Eb[i][j] = Ex[i+j];
        }
        Eb[i][3] = Ey[i];
    }

    /* apply gauss elimination to get the coefficents */
    for (i=0; i<2; i++) {
        /* partial pivoting */
        for (k=(uint8_t)(i+1); k<3; k++) {
            /* swap if diagonal element(absolute) is smaller than any terms below it */
             if (fabs(Eb[i][j]) < fabs(Eb[k][i])) {
                 for (j=0; j<4; j++) {
                     USR_DOUBLE temp;
                     temp = Eb[i][j];
                     Eb[i][j] = Eb[k][j];
                     Eb[k][j] = temp;
                 }
            }
        }
        /* gauss elimination */
        for (k=(uint8_t)(i+1); k<3; k++) {
            USR_DOUBLE term = Eb[k][i]/Eb[i][i];
            for (j=0; j<4; j++) {
                Eb[k][j] = Eb[k][j] - term * Eb[i][j];
            }
        }
    }

    /* back substitution */
    for (i=3; i>0; i--) {
        a[i-1] = Eb[i-1][3];
        for (j=i; j<3; j++) {
            a[i-1] = a[i-1] - Eb[i-1][j] * a[j];
        }
        a[i-1] = a[i-1]/Eb[i-1][i-1];
    }

    return (ERR_CODE_NONE);
#else
    UNUSED(n);
    UNUSED(x);
    UNUSED(y);
    UNUSED(a);
    USR_PRINTF(("%s : This function needs SERDES_API_FLOATING_POINT define to operate \n", API_FUNCTION_NAME));
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
#endif /* SERDES_API_FLOATING_POINT */
}



#if !defined(SMALL_FOOTPRINT)
err_code_t merlin8_phy1_INTERNAL_poll_diag_data(srds_access_t *sa__, const srds_info_t *merlin8_phy1_info_ptr, uint16_t *status,
                                          uint8_t * diag_rd_ptr, uint8_t byte_count, uint32_t timeout_ms)
{
    INIT_SRDS_ERR_CODE
    const uint32_t lane_diag_size = merlin8_phy1_info_ptr->diag_mem_ram_size;
    uint16_t loop;

    if(!status || !diag_rd_ptr) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    *diag_rd_ptr = 0;

    /** If the byte_count is too high, then there might be problems not updating
     *  the read pointer fast enough.
     */
    if (byte_count > (lane_diag_size / 2)) {
        EFUN_PRINTF(("\nERROR : merlin8_phy1_INTERNAL_poll_diag_data() has excessive byte count of %d.\n", byte_count));
        return (merlin8_phy1_error(sa__, ERR_CODE_DIAG_TIMEOUT));
    }

    ESTM(*diag_rd_ptr = rdv_usr_diag_rd_ptr());

    /* Wait until byte_count bytes are available to be read in the diagnostic memory. */
    loop = 0;
    while (1) {
        uint8_t diag_wr_ptr, full_count;

        ESTM(diag_wr_ptr = rdv_usr_diag_wr_ptr());
        if (diag_wr_ptr >= *diag_rd_ptr) {
            full_count = (uint8_t)(diag_wr_ptr - *diag_rd_ptr);
        } else {
            full_count = (uint8_t)((uint16_t)diag_wr_ptr + lane_diag_size - *diag_rd_ptr);
        }
        if (full_count >= byte_count) {
            break;
        }

        ++loop;
        if (loop > 10) {
            EFUN(USR_DELAY_US(10*timeout_ms));
        }
        if (loop > 1000) {
            return(merlin8_phy1_error(sa__, ERR_CODE_DIAG_TIMEOUT));
        }
    }
    ESTM(*status = rdv_usr_diag_status() & 0xFF);
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_INTERNAL_eye_margin_proj_start(srds_access_t *sa__, uint8_t ber_scan_mode, uint8_t timer_control, uint8_t max_error_control, int16_t *offset_start) {
    INIT_SRDS_ERR_CODE
    uint8_t lane = merlin8_phy1_acc_get_lane(sa__);
    /* Below 'DIAG_VERBOSE' level is intended to be modified only within a debug */
    /* session immediately after a breakpoint, and to retain its state only */
    /* through function exit:  therefore it must be 'volatile' to prevent a */
    /* compiler from eliding code conditioned on it, but NOT 'static'.      */

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("lane %d: start begin\n", lane));
    EFUN(merlin8_phy1_start_ber_scan_test(sa__, ber_scan_mode, timer_control, max_error_control));
    ESTM(*offset_start = (int16_t)rd_uc_dsc_data());
    {
        int16_t nlmv_val = 0;
        afe_override_slicer_sel_t slicer = INVALID_SLICER;
        EFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, *offset_start, 0, slicer, &nlmv_val));
        if(DIAG_VERBOSE > 2) EFUN_PRINTF(("lane %d: offset_start = %d:%dmV\n",lane, *offset_start, nlmv_val));
    }
    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("lane %d: start done\n", lane));
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_INTERNAL_eye_margin_proj_end(srds_access_t *sa__, USR_DOUBLE rate, uint8_t ber_scan_mode, uint8_t timer_control, int16_t offset_start) {
    INIT_SRDS_ERR_CODE
    uint32_t errs[DIAG_MAX_SAMPLES];
    uint32_t time[DIAG_MAX_SAMPLES];
    uint8_t i,cnt=0;
    uint16_t sts;
    uint8_t lane = merlin8_phy1_acc_get_lane(sa__);

    /* Below 'DIAG_VERBOSE' level is intended to be modified only within a debug */
    /* session immediately after a breakpoint, and to retain its state only */
    /* through function exit:  therefore it must be 'volatile' to prevent a */
    /* compiler from eliding code conditioned on it, but NOT 'static'.      */

    for(i=0;i<DIAG_MAX_SAMPLES;i++) {
        errs[i]=0;
        time[i]=0;
    }

    EFUN(merlin8_phy1_INTERNAL_poll_diag_done(sa__, &sts,(uint32_t)(timer_control*2000)));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("lane %d: delay done\n", lane));

    EFUN(merlin8_phy1_read_ber_scan_data(sa__, &errs[0], &time[0], &cnt, 2000));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("lane %d: read done cnt=%d\n",lane, cnt));

    EFUN(merlin8_phy1_pmd_uc_cmd(sa__, CMD_CAPTURE_BER_END,0x00,200));

    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("lane %d: end function done\n", lane));
 /* if(cnt == 1) {                                                     */
 /*     EFUN_PRINTF(("Not enough points found to extrapolate BER\n")); */
 /*     return(ERR_CODE_NONE);                                         */
 /* }                                                                  */

    // For unknown reasons, coverity cannot figure out that the called function has range checks.
    EFUN(merlin8_phy1_display_ber_scan_data(sa__, rate, ber_scan_mode, &errs[0], &time[0],(uint8_t)SRDS_ABS(offset_start)));


    if(DIAG_VERBOSE > 2) EFUN_PRINTF(("lane %d: display done\n", lane));

    return ERR_CODE_NONE;
}

#endif /* !defined(SMALL_FOOTPRINT) */
