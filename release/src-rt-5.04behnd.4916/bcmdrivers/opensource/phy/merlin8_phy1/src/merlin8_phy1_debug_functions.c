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
 *  File Name     :  merlin8_phy1_debug_functions.c                                  *
 *  Created On    :  03 Nov 2015                                                   *
 *  Created By    :  Brent Roberts                                                 *
 *  Description   :  Debug APIs for Serdes IPs                                     *
 *  Revision      :                                                            *
 ***********************************************************************************
 ***********************************************************************************/

#include "merlin8_phy1_debug_functions.h"
#include "merlin8_phy1_access.h"
#include "merlin8_phy1_common.h"
#include "merlin8_phy1_config.h"
#include "merlin8_phy1_functions.h"
#include "merlin8_phy1_internal.h"
#include "merlin8_phy1_internal_error.h"
#include "merlin8_phy1_prbs.h"
#include "merlin8_phy1_select_defns.h"
#include "merlin8_phy1_reg_dump.h"


/** @file
 *
 */




#ifndef SMALL_FOOTPRINT
static err_code_t _merlin8_phy1_reg_print_no_buf(srds_access_t *sa__, merlin8_phy1_reg_dump_arr_t *merlin8_phy1_reg_dump_arr, uint8_t reg_section);
static err_code_t _merlin8_phy1_reg_print_with_buf(srds_access_t *sa__, uint16_t start_addr, uint16_t valid_addr, char reg_buffer[SRDS_DUMP_BUF_SIZE]);
static err_code_t _merlin8_phy1_reg_print(srds_access_t *sa__, uint8_t reg_start_index, uint8_t reg_end_index, char reg_buffer[][SRDS_DUMP_BUF_SIZE], uint8_t *buf_index);
#endif /* SMALL_FOOTPRINT */


#ifndef SMALL_FOOTPRINT

/*************************/
/*  Stop/Resume uC Lane  */
/*************************/

err_code_t merlin8_phy1_stop_uc_lane(srds_access_t *sa__, uint8_t enable) {

    return(merlin8_phy1_stop_rx_adaptation(sa__,enable));
}

err_code_t merlin8_phy1_stop_uc_lane_status(srds_access_t *sa__, uint8_t *uc_lane_stopped) {
    INIT_SRDS_ERR_CODE

  if(!uc_lane_stopped) {
      return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(*uc_lane_stopped = rdv_usr_sts_micro_stopped());

  return (ERR_CODE_NONE);
}

/*******************************************************************/
/*  APIs to Write Core/Lane Config and User variables into uC RAM  */
/*******************************************************************/

err_code_t merlin8_phy1_set_usr_ctrl_core_event_log_level(srds_access_t *sa__, uint8_t core_event_log_level) {
    return(wrcv_usr_ctrl_core_event_log_level(core_event_log_level));
}

err_code_t merlin8_phy1_set_usr_ctrl_lane_event_log_level(srds_access_t *sa__, uint8_t lane_event_log_level) {
    return(wrv_usr_ctrl_lane_event_log_level(lane_event_log_level));
}

#ifndef PCIEG3_V1
err_code_t merlin8_phy1_set_usr_ctrl_disable_startup(srds_access_t *sa__, struct merlin8_phy1_usr_ctrl_disable_functions_st set_val) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_INTERNAL_update_usr_ctrl_disable_functions_byte(&set_val));
    return(wrv_usr_ctrl_disable_startup_functions_byte(set_val.byte));
}

err_code_t merlin8_phy1_set_usr_ctrl_disable_startup_dfe(srds_access_t *sa__, struct merlin8_phy1_usr_ctrl_disable_dfe_functions_st set_val) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_INTERNAL_update_usr_ctrl_disable_dfe_functions_byte(&set_val));
    return(wrv_usr_ctrl_disable_startup_dfe_functions_byte(set_val.byte));
}

err_code_t merlin8_phy1_set_usr_ctrl_disable_steady_state(srds_access_t *sa__, struct merlin8_phy1_usr_ctrl_disable_functions_st set_val) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_INTERNAL_check_uc_lane_stopped(sa__));  /* make sure uC is stopped to avoid race conditions */
    EFUN(merlin8_phy1_INTERNAL_update_usr_ctrl_disable_functions_byte(&set_val));
    return(wrv_usr_ctrl_disable_steady_state_functions_byte(set_val.byte));
}

err_code_t merlin8_phy1_set_usr_ctrl_disable_steady_state_dfe(srds_access_t *sa__, struct merlin8_phy1_usr_ctrl_disable_dfe_functions_st set_val) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_INTERNAL_update_usr_ctrl_disable_dfe_functions_byte(&set_val));
    return(wrv_usr_ctrl_disable_steady_state_dfe_functions_byte(set_val.byte));
}
#endif /* ! MERLIN7_PCIEG3 */


/******************************************************************/
/*  APIs to Read Core/Lane Config and User variables from uC RAM  */
/******************************************************************/

err_code_t merlin8_phy1_get_usr_ctrl_core_event_log_level(srds_access_t *sa__, uint8_t *core_event_log_level) {
    INIT_SRDS_ERR_CODE
    if(!core_event_log_level) {
       return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
    ESTM(*core_event_log_level = rdcv_usr_ctrl_core_event_log_level());
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_usr_ctrl_lane_event_log_level(srds_access_t *sa__, uint8_t *lane_event_log_level) {
    INIT_SRDS_ERR_CODE
    if(!lane_event_log_level) {
       return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
    ESTM(*lane_event_log_level = rdv_usr_ctrl_lane_event_log_level());
    return (ERR_CODE_NONE);
}

#ifndef PCIEG3_V1
err_code_t merlin8_phy1_get_usr_ctrl_disable_startup(srds_access_t *sa__, struct merlin8_phy1_usr_ctrl_disable_functions_st *get_val) {
    INIT_SRDS_ERR_CODE

    if(!get_val) {
       return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    ESTM(get_val->byte = rdv_usr_ctrl_disable_startup_functions_byte());
    EFUN(merlin8_phy1_INTERNAL_update_usr_ctrl_disable_functions_st(get_val));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_usr_ctrl_disable_startup_dfe(srds_access_t *sa__, struct merlin8_phy1_usr_ctrl_disable_dfe_functions_st *get_val) {
    INIT_SRDS_ERR_CODE

    if(!get_val) {
       return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    ESTM(get_val->byte = rdv_usr_ctrl_disable_startup_dfe_functions_byte());
    EFUN(merlin8_phy1_INTERNAL_update_usr_ctrl_disable_dfe_functions_st(get_val));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_usr_ctrl_disable_steady_state(srds_access_t *sa__, struct merlin8_phy1_usr_ctrl_disable_functions_st *get_val) {
    INIT_SRDS_ERR_CODE

    if(!get_val) {
       return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    ESTM(get_val->byte = rdv_usr_ctrl_disable_steady_state_functions_byte());
    EFUN(merlin8_phy1_INTERNAL_update_usr_ctrl_disable_functions_st(get_val));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_usr_ctrl_disable_steady_state_dfe(srds_access_t *sa__, struct merlin8_phy1_usr_ctrl_disable_dfe_functions_st *get_val) {
    INIT_SRDS_ERR_CODE

    if(!get_val) {
       return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    ESTM(get_val->byte = rdv_usr_ctrl_disable_steady_state_dfe_functions_byte());
    EFUN(merlin8_phy1_INTERNAL_update_usr_ctrl_disable_dfe_functions_st(get_val));
    return (ERR_CODE_NONE);
}
#endif /* ! MERLIN7_PCIEG3 */


/********************************************************************************/
/* Helper print function for Serdes Register/Variable Dump using no buffer      */
/********************************************************************************/
static err_code_t _merlin8_phy1_reg_print_no_buf(srds_access_t *sa__, merlin8_phy1_reg_dump_arr_t *merlin8_phy1_reg_dump_arr, uint8_t reg_section) {
    INIT_SRDS_ERR_CODE
    uint16_t offset = 0, rddata = 0;
    uint16_t start_addr = merlin8_phy1_reg_dump_arr[reg_section][0];
    uint16_t valid_addr = merlin8_phy1_reg_dump_arr[reg_section][1];

    EFUN_PRINTF(("\n%04x ", start_addr));
    for (offset = 0; offset < 0x10; offset++) {
        if(valid_addr & (0x1 << offset)) {
            EFUN(merlin8_phy1_acc_rdt_reg(sa__, (uint16_t)(start_addr + offset), &rddata));
        }
        else {
            rddata = 0x0;
        }
        EFUN_PRINTF(("%04x ",rddata));
    }
    return (ERR_CODE_NONE);
}

/********************************************************************************/
/* Helper print function for Serdes Register/Variable Dump using a buffer       */
/********************************************************************************/
static err_code_t _merlin8_phy1_reg_print_with_buf(srds_access_t *sa__, uint16_t start_addr, uint16_t valid_addr, char reg_buffer[SRDS_DUMP_BUF_SIZE]) {
    INIT_SRDS_ERR_CODE
    int32_t count = 0;
    uint16_t offset = 0, rddata = 0;

    count += USR_SNPRINTF(reg_buffer + count, (size_t)(SRDS_DUMP_BUF_SIZE - count),"%04x ", start_addr);

    for (offset = 0; offset < 0x10; offset++) {
        if(valid_addr & (0x1 << offset)) {
            EFUN(merlin8_phy1_acc_rdt_reg(sa__, (uint16_t)(start_addr + offset), &rddata));
        }
        else {
            rddata = 0x0;
        }
        count += USR_SNPRINTF(reg_buffer + count, (size_t)(SRDS_DUMP_BUF_SIZE - count), "%04x ", rddata);
    }
    return (ERR_CODE_NONE);
}

/********************************************************************************/
/* Helper print function for Serdes Register/Variable Dump                      */
/********************************************************************************/
static err_code_t _merlin8_phy1_reg_print(srds_access_t *sa__, uint8_t reg_start_index, uint8_t reg_end_index, char reg_buffer[][SRDS_DUMP_BUF_SIZE], uint8_t *buf_index) {
    INIT_SRDS_ERR_CODE
    uint8_t reg_section;
    merlin8_phy1_reg_dump_arr_t *merlin8_phy1_reg_dump_arr = merlin8_phy1_get_reg_dump_arr();
    if(reg_buffer == NULL) {
        for(reg_section = reg_start_index; reg_section < reg_end_index; reg_section++) {
            EFUN(_merlin8_phy1_reg_print_no_buf(sa__, merlin8_phy1_reg_dump_arr,reg_section));
        }
    }
    else {
        for(reg_section = reg_start_index; reg_section < reg_end_index; reg_section++) {
            EFUN(_merlin8_phy1_reg_print_with_buf(sa__, merlin8_phy1_reg_dump_arr[reg_section][0], merlin8_phy1_reg_dump_arr[reg_section][1], reg_buffer[*buf_index]));
            (*buf_index)++;
        }
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_reg_dump(srds_access_t *sa__, uint8_t core, uint8_t lane, char reg_buffer[][SRDS_DUMP_BUF_SIZE]) {
    INIT_SRDS_ERR_CODE
    UNUSED(core);
    UNUSED(lane);
    EFUN(merlin8_phy1_reg_select_dump(sa__, SRDS_LANE_AND_CORE_REG, reg_buffer));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_reg_select_dump(srds_access_t *sa__, uint8_t reg_dump_control, char reg_buffer[][SRDS_DUMP_BUF_SIZE]) {
    INIT_SRDS_ERR_CODE
    uint8_t core = merlin8_phy1_acc_get_core(sa__);
    uint8_t lane = merlin8_phy1_acc_get_lane(sa__);
    uint8_t buf_index = 0;
    uint8_t pll_orig, pll_idx;
    uint8_t micro_orig, micro_idx, num_micros;

    if (reg_buffer == NULL) {
        if(reg_dump_control == SRDS_CORE_REG_ONLY) {
            EFUN_PRINTF(("\n****  SERDES REGISTER CORE %d DUMP    ****", core));
        }
        else {
            EFUN_PRINTF(("\n****  SERDES REGISTER CORE %d LANE %d DUMP    ****", core, lane));
        }
    }
    else {
        if(reg_dump_control == SRDS_CORE_REG_ONLY) {
            EFUN_PRINTF(("\n****  SERDES REGISTER CORE %d DECODED    ****\n", core));
        }
        else {
            EFUN_PRINTF(("\n****  SERDES REGISTER CORE %d LANE %d DECODED    ****\n", core, lane));
        }
    }

    if (reg_dump_control & SRDS_LANE_REG_ONLY) {
        /* Lane Registers */
        EFUN(_merlin8_phy1_reg_print(sa__, 0, MERLIN8_PHY1_REG_CORE_START_INDEX, reg_buffer, &buf_index));
    }
    if (reg_dump_control & SRDS_CORE_REG_ONLY) {
        /* Core Registers */
        EFUN(_merlin8_phy1_reg_print(sa__, MERLIN8_PHY1_REG_CORE_START_INDEX, MERLIN8_PHY1_REG_PLL_START_INDEX, reg_buffer, &buf_index));

        /* Core PLL Registers */
        ESTM(pll_orig = merlin8_phy1_acc_get_pll_idx(sa__));
        for (pll_idx = 0; pll_idx < NUM_PLLS; pll_idx++) {
            EFUN(merlin8_phy1_acc_set_pll_idx(sa__,pll_idx));
            EFUN(_merlin8_phy1_reg_print(sa__, MERLIN8_PHY1_REG_PLL_START_INDEX, MERLIN8_PHY1_REG_UC_START_INDEX, reg_buffer, &buf_index));
        }
        EFUN(merlin8_phy1_acc_set_pll_idx(sa__,pll_orig));

        /* Core Micro Registers */
        ESTM(micro_orig = merlin8_phy1_acc_get_micro_idx(sa__));
        EFUN(merlin8_phy1_get_micro_num_uc_cores(sa__, &num_micros));
        for(micro_idx = 0; micro_idx < num_micros; micro_idx++) {
            EFUN(merlin8_phy1_acc_set_micro_idx(sa__,micro_idx));
            EFUN(_merlin8_phy1_reg_print(sa__, MERLIN8_PHY1_REG_UC_START_INDEX, MERLIN8_PHY1_REG_DUMP_SECTIONS, reg_buffer, &buf_index));
        }
        EFUN(merlin8_phy1_acc_set_micro_idx(sa__,micro_orig));
    }

    if (reg_buffer != NULL) {
        reg_buffer[buf_index++][0] = 0xA;
        reg_buffer[buf_index][0] = 0;
    }

    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_uc_core_var_blk_dump(srds_access_t *sa__,uint8_t core, uint16_t data_buff_size,uint8_t *data_buffer) {
      INIT_SRDS_ERR_CODE

      srds_info_t * merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
      merlin8_phy1_INTERNAL_rdblk_ram_arg_t rdblk_state;
      merlin8_phy1_INTERNAL_ram_dump_state_t state;
      uint32_t core_var_ram_size = 0;
      EFUN(merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr));
      core_var_ram_size = (uint32_t)merlin8_phy1_info_ptr->core_var_ram_size;
      state.index = 0;
      state.line_start_index = 0;
      state.ram_idx = 0;
      state.count = 0;
      rdblk_state.mem_ptr = data_buffer;
      rdblk_state.dump_state_ptr = &state;
      if(data_buffer == NULL) {
      EFUN_PRINTF(("\n**** SERDES UC CORE %d RAM VARIABLE DUMP ****", core));
      } else {
      EFUN_PRINTF(("\n***** SERDES UC CORE %d RAM VARIABLE DECODED ****\n", core));
      }
      if(data_buffer != NULL) {
      if(data_buff_size != core_var_ram_size) {
         EFUN_PRINTF(("***ERROR: UC CORE VAR BLOCK DUMP BUFFER SIZE IS INSUFFICIENT\n"));
         return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
      }
      }
      EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
                                                      merlin8_phy1_info_ptr->core_var_ram_base,
                                                      merlin8_phy1_info_ptr->core_var_ram_size,
                                                      0,
                                                      merlin8_phy1_info_ptr->core_var_ram_size,
                                                      &rdblk_state,
                                                      merlin8_phy1_INTERNAL_rdblk_ram_read_callback));


      return (ERR_CODE_NONE);

 }



err_code_t merlin8_phy1_uc_lane_var_blk_dump(srds_access_t *sa__, uint8_t core, uint8_t lane, uint32_t data_buff_size,uint8_t *data_buffer) {
    INIT_SRDS_ERR_CODE
    uint8_t     rx_lock, uc_stopped = 0;
    uint32_t    lane_var_ram_size;
    srds_info_t const * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
    merlin8_phy1_INTERNAL_rdblk_ram_arg_t rdblk_state;
    merlin8_phy1_INTERNAL_ram_dump_state_t state;
    uint32_t lane_addr_offset = 0;

    EFUN(merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr));
    lane_var_ram_size = (uint16_t)merlin8_phy1_info_ptr->lane_var_ram_size;

    EFUN(merlin8_phy1_pmd_lock_status(sa__, &rx_lock));

    {
        err_code_t err_code=ERR_CODE_NONE;
        uc_stopped = merlin8_phy1_INTERNAL_stop_micro(sa__,rx_lock,&err_code);
        if(err_code) USR_PRINTF(("Unable to stop microcontroller,  following data is suspect\n"));
    }

    state.index = 0;
    state.line_start_index = 0;
    state.count = 0;
    state.ram_idx = 0;
    rdblk_state.mem_ptr = data_buffer;
    rdblk_state.ram_size = lane_var_ram_size;
    rdblk_state.dump_state_ptr = &state;

    if(data_buffer != NULL) {
        if(data_buff_size != lane_var_ram_size) {
            EFUN_PRINTF(("\n***ERROR: UC LANE VAR BLOCK DUMP BUFFER SIZE IS INSUFFICIENT"));
            return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
        }
    }


    if(data_buffer == NULL) {
        EFUN_PRINTF(("\n**** SERDES UC CORE %d LANE %d RAM VARIABLE DUMP ****", core,lane));
    } else {
        EFUN_PRINTF(("\n***** SERDES UC CORE %d LANE %d RAM VARIABLE DECODED ****\n", core,lane));
    }


    lane = merlin8_phy1_acc_get_physical_lane(sa__);
    lane_addr_offset = _merlin8_phy1_INTERNAL_get_addr_from_lane(sa__, 0 ,lane);


    EFUN(merlin8_phy1_INTERNAL_rdblk_uc_generic_ram(sa__,
            lane_addr_offset,
            merlin8_phy1_info_ptr->lane_var_ram_size,
            0,
            merlin8_phy1_info_ptr->lane_var_ram_size,
            &rdblk_state,
            merlin8_phy1_INTERNAL_rdblk_ram_read_callback));

    if (rx_lock == 1) {
        if (!uc_stopped) {
            EFUN(merlin8_phy1_stop_rx_adaptation(sa__, 0));
        }
    } else {
        EFUN(merlin8_phy1_stop_rx_adaptation(sa__, 0));
    }

    return (ERR_CODE_NONE);
}

/***************************************/
/*  API Function to Read Event Logger  */
/***************************************/

err_code_t merlin8_phy1_read_micro_event_log(srds_access_t *sa__, uint8_t micro_idx) {
    INIT_SRDS_ERR_CODE
    merlin8_phy1_INTERNAL_event_log_dump_state_t state;
    srds_info_t * merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
    uint8_t micro_num = 0;
    uint8_t micro_start = 0;
    uint8_t micro_end = 0;
    EFUN(merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr));

    if (micro_idx < merlin8_phy1_info_ptr->micro_count) {
        micro_start = micro_idx;
        micro_end = micro_start + 1;
    }
    else {
        micro_start = 0;
        micro_end = merlin8_phy1_info_ptr->micro_count;
    }


    for (micro_num = micro_start; micro_num < micro_end; ++micro_num)
    {
        state.index = 0;
        state.line_start_index = 0;
        state.zero_cnt = 0;

        EFUN(merlin8_phy1_INTERNAL_read_event_log_with_callback(sa__, micro_num, 0, &state, merlin8_phy1_INTERNAL_event_log_dump_callback));
        EFUN(merlin8_phy1_INTERNAL_event_log_dump_callback(sa__, &state, 0, 0));
    }
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_read_event_log(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE

    /* Print CORE for the standalone diag decoder usage with Version 2 event code in customer logs */
    EFUN_PRINTF(("\n**** Starting Event Log Decode for CORE: %d ****\n", merlin8_phy1_acc_get_core(sa__)));

    EFUN(merlin8_phy1_read_micro_event_log(sa__, 0xff)); /* 0xff: all micros */

    return(ERR_CODE_NONE);
}

/**********************************************/
/*  Loopback and Ultra-Low Latency Functions  */
/**********************************************/


/* Enable/Disable Digital Loopback */
err_code_t merlin8_phy1_dig_lpbk(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE
    /* setting/clearing prbs_chk_en_auto_mode while coming out of/going in to dig lpbk */
    EFUN(wr_prbs_chk_en_auto_mode(!enable));
    EFUN(wr_dig_lpbk_en(enable));                         /* 0 = disabled, 1 = enabled */
#ifndef PCIEG3_V1
    /* Link Training enabled is an invalid mode in digital loopback */
    if(enable) {
        uint8_t link_training_enable = 0;
        ESTM(link_training_enable = rd_cl72_ieee_training_enable());
        if(link_training_enable) {
            EFUN_PRINTF(("Warning: Core: %d, Lane %d: Link Training mode is on in digital loopback.\n", merlin8_phy1_acc_get_core(sa__), merlin8_phy1_acc_get_lane(sa__)));
        }
    }
#endif

  return (ERR_CODE_NONE);
}


/**********************************/
/*  TX_PI Jitter Generation APIs  */
/**********************************/

/* TX_PI Sinusoidal or Spread-Spectrum (SSC) Jitter Generation  */
err_code_t merlin8_phy1_tx_pi_jitt_gen(srds_access_t *sa__, uint8_t enable, int16_t freq_override_val, enum srds_tx_pi_freq_jit_gen_enum jit_type, uint8_t tx_pi_jit_freq_idx, uint8_t tx_pi_jit_amp) {
    INIT_SRDS_ERR_CODE
    /* Added a limiting for the jitter amplitude index, per freq_idx */
    uint8_t max_amp_idx_r20_os1[] = {37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 48, 33, 47, 37, 33, 37, 37};

    /* Irrespective of the osr_mode, txpi runs @ os1. Thus the max amp idx values remain the same. */
    if (jit_type == TX_PI_SJ) {
        if (tx_pi_jit_amp > max_amp_idx_r20_os1[tx_pi_jit_freq_idx]) {
            tx_pi_jit_amp = max_amp_idx_r20_os1[tx_pi_jit_freq_idx];
        }
    }

    EFUN(merlin8_phy1_tx_pi_freq_override(sa__, enable, freq_override_val));

    if (enable) {
        EFUN(wr_tx_pi_jit_freq_idx(tx_pi_jit_freq_idx));
        EFUN(wr_tx_pi_jit_amp(tx_pi_jit_amp));

        if (jit_type == TX_PI_SSC_HIGH_FREQ) {
            EFUN(wr_tx_pi_jit_ssc_freq_mode(0x1));        /* SSC_FREQ_MODE:             0 = 6G SSC mode, 1 = 10G SSC mode */
            EFUN(wr_tx_pi_ssc_gen_en(0x1));               /* SSC jitter enable:         0 = disabled,    1 = enabled */
        }
        else if (jit_type == TX_PI_SSC_LOW_FREQ) {
            EFUN(wr_tx_pi_jit_ssc_freq_mode(0x0));        /* SSC_FREQ_MODE:             0 = 6G SSC mode, 1 = 10G SSC mode */
            EFUN(wr_tx_pi_ssc_gen_en(0x1));               /* SSC jitter enable:         0 = disabled,    1 = enabled */
        }
        else if (jit_type == TX_PI_SJ) {
            EFUN(wr_tx_pi_sj_gen_en(0x1));                /* Sinusoidal jitter enable:  0 = disabled,    1 = enabled */
        }
    }
    else {
        EFUN(wr_tx_pi_ssc_gen_en(0x0));                   /* SSC jitter enable:         0 = disabled,    1 = enabled */
        EFUN(wr_tx_pi_sj_gen_en(0x0));                    /* Sinusoidal jitter enable:  0 = disabled,    1 = enabled */
    }
  return (ERR_CODE_NONE);
}


/*******************************/
/*  Isolate Serdes Input Pins  */
/*******************************/

err_code_t merlin8_phy1_isolate_ctrl_pins(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE
    uint8_t lane, lane_orig, num_lanes;
    uint8_t pll, pll_orig;
    ESTM(pll_orig = merlin8_phy1_acc_get_pll_idx(sa__));

    for(pll = 0; pll < NUM_PLLS; pll++) {
        EFUN(merlin8_phy1_acc_set_pll_idx(sa__, pll));
        EFUN(merlin8_phy1_isolate_core_ctrl_pins(sa__, enable));
    }
    EFUN(merlin8_phy1_acc_set_pll_idx(sa__, pll_orig));

    ESTM(lane_orig = merlin8_phy1_acc_get_lane(sa__));
    /* read num lanes per core directly from register */
    ESTM(num_lanes = rdc_revid_multiplicity());
    for(lane = 0; lane < num_lanes; lane++) {
        EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
        EFUN(merlin8_phy1_isolate_lane_ctrl_pins(sa__, enable));
    }
    EFUN(merlin8_phy1_acc_set_lane(sa__, lane_orig));

  return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_isolate_lane_ctrl_pins(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE

    EFUN(merlin8_phy1_isolate_lane_ctrl_tx_pins(sa__, enable));
    EFUN(merlin8_phy1_isolate_lane_ctrl_rx_pins(sa__, enable));
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_isolate_lane_ctrl_tx_pins(srds_access_t *sa__, uint8_t enable){

    INIT_SRDS_ERR_CODE

    if (enable) {
        EFUN(wr_pmd_ln_tx_h_pwrdn_pkill(0x1));
        EFUN(wr_pmd_ln_dp_h_rstb_pkill(0x1));
        EFUN(wr_pmd_ln_h_rstb_pkill(0x1));
        EFUN(wr_pmd_tx_disable_pkill(0x1));
    }
    else {
        EFUN(wr_pmd_ln_tx_h_pwrdn_pkill(0x0));
        EFUN(wr_pmd_ln_dp_h_rstb_pkill(0x0));
        EFUN(wr_pmd_ln_h_rstb_pkill(0x0));
        EFUN(wr_pmd_tx_disable_pkill(0x0));
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_isolate_lane_ctrl_rx_pins(srds_access_t *sa__, uint8_t enable){

    INIT_SRDS_ERR_CODE

    if (enable) {
      EFUN(wr_pmd_ln_rx_h_pwrdn_pkill(0x1));
      EFUN(wr_pmd_ln_dp_h_rstb_pkill(0x1));
      EFUN(wr_pmd_ln_h_rstb_pkill(0x1));

    }
    else {
      EFUN(wr_pmd_ln_rx_h_pwrdn_pkill(0x0));
      EFUN(wr_pmd_ln_dp_h_rstb_pkill(0x0));
      EFUN(wr_pmd_ln_h_rstb_pkill(0x0));

    }
    return (ERR_CODE_NONE);
}


err_code_t merlin8_phy1_isolate_core_ctrl_pins(srds_access_t *sa__, uint8_t enable) {
  INIT_SRDS_ERR_CODE

  if (enable) {
    EFUN(wrc_pmd_core_dp_h_rstb_pkill(0x1));
  }
  else {
    EFUN(wrc_pmd_core_dp_h_rstb_pkill(0x0));
  }
  return (ERR_CODE_NONE);
}




// This condition makes the PCIEG3_V1 conditions inside this function and several that follow redundant
#ifndef PCIEG3_V1
err_code_t merlin8_phy1_log_full_pmd_state_noPRBS(srds_access_t *sa__, struct merlin8_phy1_detailed_lane_status_st *lane_st) {
    INIT_SRDS_ERR_CODE
    uint8_t  tmp1;
    srds_info_t * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr_with_check(sa__);
    EFUN(merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr));

    if(!lane_st)
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));

    EFUN(merlin8_phy1_INTERNAL_pmd_lock_status(sa__,&lane_st->pmd_lock, &lane_st->pmd_lock_chg));
    {
        err_code_t err_code=ERR_CODE_NONE;
        lane_st->stop_state = merlin8_phy1_INTERNAL_stop_micro(sa__,lane_st->pmd_lock,&err_code);
        if(err_code) USR_PRINTF(("Unable to stop microcontroller,  following data is suspect\n"));
    }

    ESTM(lane_st->reset_state = rd_lane_dp_reset_state());
    /*populate the big endian info */
    lane_st->big_endian = merlin8_phy1_info_ptr->is_big_endian;
#ifndef PCIEG3_V1
    ESTM(lane_st->dummy1 = rdv_usr_dbstopped());
#endif

#ifndef PCIEG3_V1
    {
        struct merlin8_phy1_uc_lane_config_st lane_cfg;
        EFUN(merlin8_phy1_get_uc_lane_cfg(sa__, &lane_cfg));
        lane_st->lane_config_word = lane_cfg.word;
    }
    ESTM(lane_st->ucv_status = rdv_status_byte());
    ESTM(lane_st->reset_state    = rd_rx_lane_dp_reset_state());
    ESTM(lane_st->tx_reset_state = rd_tx_lane_dp_reset_state());
#endif
#ifndef PCIEG3_V1
    ESTM(lane_st->clk90 = rd_cnt_d_minus_m1());
    ESTM(lane_st->clkp1 = rd_cnt_d_minus_p1());
    /* drop the MSB, the result is actually valid modulo 128 */
    /* Also flip the sign to account for d-m1, versus m1-d */
    lane_st->clk90 = (int8_t)(lane_st->clk90<<1);
    lane_st->clk90 = (int8_t)(-(lane_st->clk90>>1));
    lane_st->clkp1 = (int8_t)(lane_st->clkp1<<1);
    lane_st->clkp1 = (int8_t)(-(lane_st->clkp1>>1));
    ESTM(lane_st->pf_hiz = rd_pf_hiz());
    EFUN(merlin8_phy1_read_rx_afe(sa__, RX_AFE_PF2, &lane_st->pf2_ctrl));
    EFUN(merlin8_phy1_read_rx_afe(sa__, RX_AFE_VGA, &lane_st->vga));
    ESTM(lane_st->dc_offset = rd_dc_offset_bin());
    {
        uint8_t ladder_range = 0;
        ESTM(ladder_range = rd_p1_thresh_sel());
        EFUN(merlin8_phy1_INTERNAL_get_p1_threshold(sa__, &lane_st->p1_lvl_ctrl));
        EFUN(merlin8_phy1_INTERNAL_ladder_setting_to_mV(sa__, lane_st->p1_lvl_ctrl, ladder_range, INVALID_SLICER, &lane_st->p1_lvl));
    }
    lane_st->m1_lvl = 0;


    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe1(sa__, &lane_st->dfe1));
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe2(sa__, &lane_st->dfe2));
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe3(sa__, &lane_st->dfe3));
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe4(sa__, &lane_st->dfe4));
    EFUN(merlin8_phy1_INTERNAL_get_rx_dfe5(sa__, &lane_st->dfe5));
    {
        int8_t dfe_1_e;
        ESTM(dfe_1_e = (int8_t)rd_dfe_1_e());
        ESTM(lane_st->dfe1_dcd = (int8_t)(dfe_1_e - rd_dfe_1_o()));
    }
    EFUN(merlin8_phy1_INTERNAL_get_dfe2_dcd(sa__, &lane_st->dfe2_dcd));
    ESTM(lane_st->pe = rd_p1_offset_evn_bin());
    ESTM(lane_st->ze = rd_data_offset_evn_bin());
    ESTM(lane_st->me = rd_m1_offset_evn_bin());

    ESTM(lane_st->po = rd_p1_offset_odd_bin());
    ESTM(lane_st->zo = rd_data_offset_odd_bin());
    ESTM(lane_st->mo = rd_m1_offset_odd_bin());

    EFUN(merlin8_phy1_INTERNAL_get_tx_ppm(sa__, &lane_st->tx_ppm));

    EFUN(merlin8_phy1_INTERNAL_get_tx_pre(sa__,   &lane_st->txfir_pre));
    EFUN(merlin8_phy1_INTERNAL_get_tx_main(sa__,  &lane_st->txfir_main));
    EFUN(merlin8_phy1_INTERNAL_get_tx_post1(sa__, &lane_st->txfir_post1));
    EFUN(merlin8_phy1_INTERNAL_get_tx_post2(sa__, &lane_st->txfir_post2));

    ESTM(lane_st->soc_pos = rd_ams_rx_sigdet_offset_correction_pos());
    ESTM(lane_st->soc_neg = rd_ams_rx_sigdet_offset_correction_neg());
#endif  /* if defined(MERLIN7) && !defined(MERLIN7_PCIEG3) */

    ESTM(lane_st->restart_count = rdv_usr_sts_restart_counter());
    ESTM(lane_st->reset_count = rdv_usr_sts_reset_counter());
    ESTM(lane_st->pmd_lock_count = rdv_usr_sts_pmd_lock_counter());

    ESTM(lane_st->temp_idx = rdcv_temp_idx());

    EFUN(merlin8_phy1_INTERNAL_sigdet_status(sa__, &lane_st->sigdet, &lane_st->sigdet_chg));
    ESTM(lane_st->dsc_sm[0] = rd_dsc_state_one_hot());
    ESTM(lane_st->dsc_sm[1] = rd_dsc_state_one_hot());
    ESTM(lane_st->ppm = rd_cdr_integ_reg());
    EFUN(merlin8_phy1_INTERNAL_get_rx_vga(sa__, &tmp1));
#ifndef PCIEG3_V1
    lane_st->vga = (int8_t)tmp1;
#else
    lane_st->vga = tmp1;
#endif
    EFUN(merlin8_phy1_INTERNAL_get_rx_pf_main(sa__, &tmp1)); lane_st->pf = tmp1;
    EFUN(merlin8_phy1_INTERNAL_get_rx_pf2(sa__, &tmp1)); lane_st->pf2 = tmp1;



    EFUN(merlin8_phy1_INTERNAL_get_eye_margin_est(sa__, &lane_st->heye_left, &lane_st->heye_right, &lane_st->veye_upper, &lane_st->veye_lower));

    {
        uint32_t link_time32b = 0;
        EFUN(merlin8_phy1_INTERNAL_get_link_time(sa__, &link_time32b));
        lane_st->link_time = (uint16_t)link_time32b;
    }

    if (lane_st->pmd_lock == 1) {
      if (!lane_st->stop_state) {
        EFUN(merlin8_phy1_stop_rx_adaptation(sa__, 0));
      }
    } else {
        EFUN(merlin8_phy1_stop_rx_adaptation(sa__, 0));
    }

    return(ERR_CODE_NONE);
}  /* err_code_t merlin8_phy1_log_full_pmd_state_noPRBS(srds_access_t *sa__, struct merlin8_phy1_detailed_lane_status_st *lane_st) */

err_code_t merlin8_phy1_log_full_pmd_state(srds_access_t *sa__, struct merlin8_phy1_detailed_lane_status_st *lane_st) {
    INIT_SRDS_ERR_CODE

    if(!lane_st) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    ENULL_MEMSET(lane_st, 0, sizeof(struct merlin8_phy1_detailed_lane_status_st));
  {
    enum srds_prbs_polynomial_enum prbs_poly_mode = PRBS_7;
    enum srds_prbs_checker_mode_enum prbs_checker_mode = PRBS_SELF_SYNC_HYSTERESIS;

    EFUN(merlin8_phy1_log_full_pmd_state_noPRBS(sa__, lane_st));
    ESTM(lane_st->prbs_chk_en = rd_prbs_chk_en());
    EFUN(merlin8_phy1_get_rx_prbs_config(sa__, &prbs_poly_mode, &prbs_checker_mode, &lane_st->prbs_chk_inv));
    lane_st->prbs_chk_order = (uint8_t)prbs_poly_mode;
    EFUN(merlin8_phy1_prbs_chk_lock_state(sa__, &lane_st->prbs_chk_lock));
    EFUN(merlin8_phy1_prbs_err_count_state(sa__, &lane_st->prbs_chk_errcnt, &lane_st->prbs_chk_lock_lost));
    EFUN(merlin8_phy1_INTERNAL_get_BER_string(sa__,100,&lane_st->ber_string[0], sizeof(lane_st->ber_string)));
  }
    return(ERR_CODE_NONE);
}
err_code_t merlin8_phy1_disp_full_pmd_state (srds_access_t *sa__, struct merlin8_phy1_detailed_lane_status_st const * const lane_st, uint8_t num_lanes){
    INIT_SRDS_ERR_CODE

    const uint8_t num_bytes_each_line = 26;
    uint32_t i;
    uint32_t size_of_lane_st = 0;
    const uint8_t big_endian = merlin8_phy1_INTERNAL_is_big_endian();

    if(lane_st == NULL) {
        return (merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    if((num_lanes != 1) && (num_lanes != 2) && (num_lanes != 4) && (num_lanes != 8)) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT)); /* Number of lanes has to be 1, 2, 4, or 8 */
    }

      size_of_lane_st = sizeof(struct merlin8_phy1_detailed_lane_status_st);

      EFUN_PRINTF(("\n**** SERDES EXTENDED LANE DIAG DATA CORE %d ALL LANES DUMP ****", merlin8_phy1_acc_get_core(sa__)));
      EFUN_PRINTF(("\n0000 "));
      ESTM_PRINTF(("%02x ", num_lanes));
      ESTM_PRINTF(("%02x ", big_endian));
      ESTM_PRINTF(("%02x ", (uint8_t)sizeof(struct merlin8_phy1_check_platform_info_st)));
      /* For some customer builds, ESTM_PRINTF is an empty macro.
         In such cases, some compilers might generates an error of "unused variable" for big_endian.
         The void usage of big_endian down below - solves this compiler error */
      UNUSED(big_endian);
      for (i = 3; i < num_lanes*size_of_lane_st+3; i++) {
        if (!(i % num_bytes_each_line))  {
          EFUN_PRINTF(("\n%04x ", i));
        }
        ESTM_PRINTF(("%02x ", *(((uint8_t*)lane_st)+i-3)));
      }

      EFUN_PRINTF(("\n**** END OF DATA DUMP ****\n"));

    EFUN_PRINTF(("\n========== End of SERDES EXTENDED LANE DIAG DATA ==================\n"));
/*#endif*/
    return (ERR_CODE_NONE);

}
#endif /* !MERLIN7_PCIEG3 */
#endif /* ! SMALL_FOOTPRINT */

/*
 * for backtrace() support
 */


void merlin8_phy1_INTERNAL_print_triage_info(srds_access_t *sa__, err_code_t err_code, uint8_t print_header, uint8_t print_data, uint16_t line)
{
    /*  Note: No EFUNs or ESTMs should be used in this function as this print routine is called by _error() handler. */
#if defined(SMALL_FOOTPRINT)
    return;
#else
    merlin8_phy1_triage_info info;
    INIT_SRDS_ERR_CODE
    uint16_t   ucode_version_major;
    uint8_t    ucode_version_minor, error_seen = 0;

    if (ERR_CODE_SRDS_REG_ACCESS_FAIL == err_code) {  /* Early return to prevent error handling recursion on Access Errors! */
        return;
    }

    USR_MEMSET(&info, 0, sizeof(struct merlin8_phy1_triage_info_st));
    info.error = err_code;
    info.line = line;

    if (print_header) {
        USR_PRINTF(("Triage Info Below:\n"));
        if ((err_code == ERR_CODE_UC_CMD_POLLING_TIMEOUT) || (err_code == ERR_CODE_UC_NOT_STOPPED)) {
            USR_PRINTF(("Lane, Core,  API_VER, UCODE_VER, micro_stop_status, exception(sw,hw), stack_ovflw, cmd_info, pmd_lock, sigdet, dsc_one_hot(0,1), Error\n"));
        } else {
            USR_PRINTF(("Lane, Core,  API_VER, UCODE_VER, Error\n"));
        }
    }

    if (merlin8_phy1_version(sa__, &info.api_ver)) {  /* Unable to read api version */
        info.api_ver = 0xFFFFFFFF;
        error_seen = 1;
    }
    CHECK_ERR(ucode_version_major = rdcv_common_ucode_version());
    CHECK_ERR(ucode_version_minor = rdcv_common_ucode_minor_version());
    info.ucode_ver = (uint32_t)((ucode_version_major << 8) | ucode_version_minor);
    CHECK_ERR(info.stop_status = rdv_usr_sts_micro_stopped());

    /* Collect exception and overflow information */
    CHECK_ERR(info.stack_overflow = rdcv_status_byte());
    if(merlin8_phy1_INTERNAL_sigdet_status(sa__, &info.sig_det, &info.sig_det_chg)) {
        error_seen = 1;
    }
    if (merlin8_phy1_pmd_lock_status(sa__, &info.pmd_lock)) {
        error_seen = 1;
    }

#ifndef PCIEG3_V1
    CHECK_ERR(info.dsc_one_hot[0] = rd_dsc_state_one_hot());
    CHECK_ERR(info.dsc_one_hot[1] = rd_dsc_state_one_hot());
#else
    CHECK_ERR(info.dsc_one_hot[0] = rd_dsc_state_one_hot_0());
    CHECK_ERR(info.dsc_one_hot[1] = rd_dsc_state_one_hot_0());
#endif
    CHECK_ERR(info.cmd_info = reg_rd_DSC_A_DSC_UC_CTRL());
    info.core = merlin8_phy1_acc_get_core(sa__);
    info.lane = merlin8_phy1_acc_get_lane(sa__);
    if (print_data) {
        if ((err_code == ERR_CODE_UC_CMD_POLLING_TIMEOUT) || (err_code == ERR_CODE_UC_NOT_STOPPED)) {
            USR_PRINTF(("%4d, %4d,  %X_%X,    %X_%X, %17d, %7d,%d       , %11d,   0x%04x, %8d, %6d,    0x%x,0x%x   , %s\n",
                        info.lane,
                        info.core,
                        info.api_ver>>8,
                        info.api_ver & 0xFF,
                        info.ucode_ver >> 8,
                        info.ucode_ver & 0xFF,
                        info.stop_status,
                        info.sw_exception,
                        info.hw_exception,
                        info.stack_overflow,
                        info.cmd_info,
                        info.pmd_lock,
                        info.sig_det,
                        info.dsc_one_hot[0],
                        info.dsc_one_hot[1],
                        merlin8_phy1_INTERNAL_e2s_err_code(info.error) ));
        } else {
            USR_PRINTF(("%4d, %4d,  %X_%X,     %X_%X, %s\n",
                        info.lane,
                        info.core,
                        info.api_ver>>8,
                        info.api_ver & 0xFF,
                        info.ucode_ver >> 8,
                        info.ucode_ver & 0xFF,
                        merlin8_phy1_INTERNAL_e2s_err_code(info.error) ));
        }
    }
    if (error_seen) {
        USR_PRINTF(("WARNING: There were some errors seen while collecting triage info and so the debug data above may not be all accurate\n"));
    }
    return;
#endif /* SMALL_FOOTPRINT */
}


