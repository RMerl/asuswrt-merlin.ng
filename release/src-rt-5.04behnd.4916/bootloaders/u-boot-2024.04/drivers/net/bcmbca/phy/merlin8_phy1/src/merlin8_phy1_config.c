// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/***********************************************************************************
 ***********************************************************************************
 *  File Name     :  merlin8_phy1_config.c                                           *
 *  Created On    :  03 Nov 2015                                                   *
 *  Created By    :  Brent Roberts                                                 *
 *  Description   :  API config functions Serdes IPs                               *
 *  Revision      :   *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/


#include "merlin8_phy1_config.h"
#include "merlin8_phy1_access.h"
#include "merlin8_phy1_common.h"
#include "merlin8_phy1_functions.h"
#include "merlin8_phy1_internal.h"
#include "merlin8_phy1_internal_error.h"
#include "merlin8_phy1_select_defns.h"
#include "merlin8_phy1_enum.h"
#include "merlin8_phy1_dependencies.h"
#include "merlin8_phy1_version.h"
#include "merlin8_phy1_diag.h"

/** @file
 *
 */




/* User specific implementation for creating syncronization object, used for critical section access in initialization routine below */
USR_CREATE_LOCK;

#ifndef SMALL_FOOTPRINT
/*******************************/
/*  Stop/Resume RX Adaptation  */
/*******************************/

err_code_t merlin8_phy1_stop_rx_adaptation(srds_access_t *sa__, uint8_t enable) {
  if (enable) {
      err_code_t err_code;
      err_code = merlin8_phy1_pmd_uc_control(sa__, CMD_UC_CTRL_STOP_GRACEFULLY,GRACEFUL_STOP_TIME);
      if(err_code) {
          uint8_t temp=0;
          USR_PRINTF(("Warning Graceful stop request returned error %d; Requesting a forceful stop\n",err_code));
          temp = merlin8_phy1_INTERNAL_stop_micro(sa__,0,&err_code);
          /* return value from merlin8_phy1_INTERNAL_stop_micro - immediate stop is not required */
          /* void casting to avoid compiler warning. */
          UNUSED(temp);
      }
      return(err_code);
  }
  else {
    return(merlin8_phy1_pmd_uc_control(sa__, CMD_UC_CTRL_RESUME,50));
  }
}

err_code_t merlin8_phy1_request_stop_rx_adaptation(srds_access_t *sa__) {

  return(merlin8_phy1_pmd_uc_control_return_immediate(sa__, CMD_UC_CTRL_STOP_GRACEFULLY));
}

/**********************/
/*  uCode CRC Verify  */
/**********************/

err_code_t merlin8_phy1_ucode_crc_verify(srds_access_t *sa__, uint32_t ucode_len, uint16_t expected_crc_value) {
    INIT_SRDS_ERR_CODE
    uint16_t calc_crc;

    EFUN(merlin8_phy1_pmd_uc_cmd_with_data(sa__, CMD_CALC_CRC,0,(uint16_t)ucode_len,200));
    ESTM(calc_crc = rd_uc_dsc_data());

    if(calc_crc != expected_crc_value) {
        EFUN_PRINTF(("Microcode CRC did not match expected=%04x : calculated=%04x\n",expected_crc_value, calc_crc));
        return(merlin8_phy1_error(sa__, ERR_CODE_UC_CRC_NOT_MATCH));
    }

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_start_ucode_crc_calc(srds_access_t *sa__, uint16_t ucode_len) {
    INIT_SRDS_ERR_CODE
    /* Invoke Calculate CRC command and return control immediately */
    EFUN(merlin8_phy1_pmd_uc_cmd_with_data_return_immediate(sa__, CMD_CALC_CRC, 0, ucode_len));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_check_ucode_crc(srds_access_t *sa__, uint16_t expected_crc_value, uint32_t timeout_ms) {
    INIT_SRDS_ERR_CODE
    uint16_t calc_crc;
    err_code_t err_code;

    /* Poll for uc_dsc_ready_for_cmd = 1 to indicate merlin8_phy1 ready for command */
    err_code = merlin8_phy1_INTERNAL_poll_uc_dsc_ready_for_cmd_equals_1(sa__, timeout_ms, CMD_CALC_CRC);
    if (err_code) {
      EFUN_PRINTF(("ERROR : DSC ready for command timed out. Previous uC command not finished yet\n"));
      return (err_code);
    }

    ESTM(calc_crc = rd_uc_dsc_data());
    if(calc_crc != expected_crc_value) {
        EFUN_PRINTF(("UC CRC did not match expected=%04x : calculated=%04x\n",expected_crc_value, calc_crc));
        return(merlin8_phy1_error(sa__, ERR_CODE_UC_CRC_NOT_MATCH));
    }

    return(ERR_CODE_NONE);
}
#endif /* SMALL_FOOTPRINT */

#ifndef PCIEG3_V1
/**************************************************************/
/*  APIs to Write Lane Config and User variables into uC RAM  */
/**************************************************************/

err_code_t merlin8_phy1_set_uc_lane_cfg(srds_access_t *sa__, struct merlin8_phy1_uc_lane_config_st struct_val) {
    INIT_SRDS_ERR_CODE
  uint8_t reset_state;
  ESTM(reset_state = rd_rx_lane_dp_reset_state());
  if(reset_state < 7) {
      EFUN_PRINTF(("ERROR: merlin8_phy1_set_uc_lane_cfg(..) called without ln_dp_s_rstb=0\n"));
      return (merlin8_phy1_error(sa__, ERR_CODE_LANE_DP_NOT_RESET));
  }
  EFUN(merlin8_phy1_INTERNAL_update_uc_lane_config_word(&struct_val));
  return(wrv_config_word(struct_val.word));
}


/******************************************************************/
/*  APIs to Read Core/Lane Config and User variables from uC RAM  */
/******************************************************************/

err_code_t merlin8_phy1_get_uc_core_config(srds_access_t *sa__, struct merlin8_phy1_uc_core_config_st *get_val) {
    INIT_SRDS_ERR_CODE
    uint8_t pll_orig;
    ESTM(pll_orig = merlin8_phy1_acc_get_pll_idx(sa__));
    EFUN(merlin8_phy1_acc_set_pll_idx(sa__, 0));
    /* TODO: convert to fail-safe form where default return is error case */
    if(!get_val) {
        return merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
        ESTM(get_val->word = rdcv_config_word());
    EFUN(merlin8_phy1_acc_set_pll_idx(sa__, pll_orig));
    EFUN(merlin8_phy1_INTERNAL_update_uc_core_config_st(get_val));
    return ERR_CODE_NONE;
}
#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_get_uc_lane_cfg(srds_access_t *sa__, struct merlin8_phy1_uc_lane_config_st *get_val) {
    INIT_SRDS_ERR_CODE

  if(!get_val) {
     return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }
  ESTM(get_val->word = rdv_config_word());
  EFUN(merlin8_phy1_INTERNAL_update_uc_lane_config_st(get_val));
  return (ERR_CODE_NONE);
}
#endif /* SMALL_FOOTPRINT */
#endif /* MERLIN7_PCIEG3 */

/*---------------------------------*/
/*  APIs to Reset Lane to Default  */
/*---------------------------------*/

err_code_t merlin8_phy1_reset_tx_lane_to_default(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    EFUN(wr_tx_ln_dp_s_rstb(0x0));
    EFUN(USR_DELAY_US(1));
    EFUN(wr_tx_ln_s_rstb(0x0));
    EFUN(USR_DELAY_US(1));
    EFUN(wr_tx_ln_s_rstb(0x1));
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_reset_rx_lane_to_default(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    EFUN(wr_rx_ln_dp_s_rstb(0x0));
    EFUN(USR_DELAY_US(1));
    EFUN(wr_rx_ln_s_rstb(0x0));
    EFUN(USR_DELAY_US(1));
    EFUN(wr_rx_ln_s_rstb(0x1));
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_reset_lane_to_default(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    EFUN(wr_ln_dp_s_rstb(0x0));
    EFUN(USR_DELAY_US(1));
    EFUN(wr_ln_s_rstb(0x0));
    EFUN(USR_DELAY_US(1));
    EFUN(wr_ln_s_rstb(0x1));
    return (ERR_CODE_NONE);
}

/*--------------------------------------------*/
/*  APIs to Enable or Disable datapath reset  */
/*--------------------------------------------*/


err_code_t merlin8_phy1_tx_dp_reset(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE
    if (enable) {
        EFUN(wr_tx_ln_dp_s_rstb(0x0));
    } else {
        EFUN(wr_tx_ln_dp_s_rstb(0x1));
        EFUN(wr_tx_uc_ack_lane_dp_reset(0x1));
        EFUN(wr_tx_uc_ack_lane_cfg_done(0x1));
#ifndef PCIEG3_V1
        {
            uint16_t i, tx_lane_dp_reset_state;
            i = 10;
            tx_lane_dp_reset_state = 0x7;
            /* read tx_lane_dp_reset_state and handle DCC enable once released */
            while(--i) {
                ESTM(tx_lane_dp_reset_state = rd_tx_lane_dp_reset_state());
                if(!tx_lane_dp_reset_state)
                    break;
            }
            EFUN(merlin8_phy1_apply_dcc_en_cfg(sa__));
        }
#endif
    }
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_rx_dp_reset(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE
    if (enable) {
        EFUN(wr_rx_ln_dp_s_rstb(0x0));
    } else {
        EFUN(wr_rx_ln_dp_s_rstb(0x1));
    }
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_core_dp_reset(srds_access_t *sa__, uint8_t enable){
    INIT_SRDS_ERR_CODE
    if (enable) {
        EFUN(wrc_core_dp_s_rstb(0x0));
    } else {
        EFUN(wrc_core_dp_s_rstb(0x1));
    }
    return ERR_CODE_NONE;
}
err_code_t merlin8_phy1_ln_dp_reset(srds_access_t *sa__, uint8_t enable){
    INIT_SRDS_ERR_CODE
    if (enable) {
        EFUN(wr_ln_dp_s_rstb(0x0));
    } else {
        EFUN(wr_ln_dp_s_rstb(0x1));
#ifndef PCIEG3_V1
        {
            uint16_t i, tx_lane_dp_reset_state;
            i = 10;
            tx_lane_dp_reset_state = 0x7;
            /* read tx_lane_dp_reset_state and handle DCC enable once released */
            while(--i) {
                ESTM(tx_lane_dp_reset_state = rd_tx_lane_dp_reset_state());
                if(!tx_lane_dp_reset_state)
                    break;
            }
            EFUN(merlin8_phy1_apply_dcc_en_cfg(sa__));
        }
#endif
    }
    return ERR_CODE_NONE;
}

/*-----------------------------------------*/
/*  APIs for setting and getting OSR mode  */
/*-----------------------------------------*/

err_code_t merlin8_phy1_set_use_osr_mode_pins_only(srds_access_t *sa__, uint8_t use_pins_only) {
    INIT_SRDS_ERR_CODE

    EFUN(merlin8_phy1_set_use_tx_osr_mode_pins_only(sa__, use_pins_only));
    EFUN(merlin8_phy1_set_use_rx_osr_mode_pins_only(sa__, use_pins_only));

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_use_osr_mode_pins_only(srds_access_t *sa__, uint8_t *tx_use_pins_only, uint8_t *rx_use_pins_only) {
    INIT_SRDS_ERR_CODE

    if(tx_use_pins_only == NULL || rx_use_pins_only == NULL) {
        return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
    }

    EFUN(merlin8_phy1_get_use_tx_osr_mode_pins_only(sa__, tx_use_pins_only));
    EFUN(merlin8_phy1_get_use_rx_osr_mode_pins_only(sa__, rx_use_pins_only));

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_set_use_rx_osr_mode_pins_only(srds_access_t *sa__, uint8_t rx_use_pins_only) {
    INIT_SRDS_ERR_CODE
    uint8_t reset_state;
#ifdef SERDES_TX_RX_SEPARATE
    ESTM(reset_state = rd_rx_lane_dp_reset_state());
#else
    ESTM(reset_state = rd_lane_dp_reset_state());
#endif
    if(reset_state < 7) {
        EFUN_PRINTF(("ERROR: merlin8_phy1_set_use_rx_osr_mode_pins_only(..) called without ln_dp_s_rstb=0\n"));
        return (merlin8_phy1_error(sa__, ERR_CODE_LANE_DP_NOT_RESET));
    }
    if(rx_use_pins_only) {
        EFUN(wr_rx_osr_mode_frc(0x0));
        EFUN(wr_rx_osr_mode_frc_val(MERLIN8_PHY1_OSR_UNINITIALIZED));
    }

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_use_rx_osr_mode_pins_only(srds_access_t *sa__, uint8_t *rx_use_pins_only) {
    INIT_SRDS_ERR_CODE

    if(rx_use_pins_only == NULL) {
        return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
    }
    ESTM(*rx_use_pins_only = (rd_rx_osr_mode_frc() == 0));

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_set_use_tx_osr_mode_pins_only(srds_access_t *sa__, uint8_t tx_use_pins_only) {
    INIT_SRDS_ERR_CODE
    uint8_t reset_state;
#ifdef SERDES_TX_RX_SEPARATE
    ESTM(reset_state = rd_tx_lane_dp_reset_state());
#else
    ESTM(reset_state = rd_lane_dp_reset_state());
#endif
    if(reset_state < 7) {
        EFUN_PRINTF(("ERROR: merlin8_phy1_set_use_tx_osr_mode_pins_only(..) called without ln_dp_s_rstb=0\n"));
        return (merlin8_phy1_error(sa__, ERR_CODE_LANE_DP_NOT_RESET));
    }
    if(tx_use_pins_only) {
        EFUN(wr_tx_osr_mode_frc(0x0));
        EFUN(wr_tx_osr_mode_frc_val(MERLIN8_PHY1_OSR_UNINITIALIZED));
    }

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_use_tx_osr_mode_pins_only(srds_access_t *sa__, uint8_t *tx_use_pins_only) {
    INIT_SRDS_ERR_CODE

    if(tx_use_pins_only == NULL) {
        return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
    }
    ESTM(*tx_use_pins_only = (rd_tx_osr_mode_frc() == 0));

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_set_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum osr_mode) {
    INIT_SRDS_ERR_CODE

    EFUN(merlin8_phy1_set_tx_osr_mode(sa__, osr_mode));
    EFUN(merlin8_phy1_set_rx_osr_mode(sa__, osr_mode));

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum *tx_osr_mode,  enum merlin8_phy1_osr_mode_enum *rx_osr_mode) {
    INIT_SRDS_ERR_CODE

    if(tx_osr_mode == NULL || rx_osr_mode == NULL) {
        return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
    }

    EFUN(merlin8_phy1_get_tx_osr_mode(sa__, tx_osr_mode));
    EFUN(merlin8_phy1_get_rx_osr_mode(sa__, rx_osr_mode));

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_set_rx_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum rx_osr_mode) {
    INIT_SRDS_ERR_CODE
    uint8_t reset_state;
#ifdef SERDES_TX_RX_SEPARATE
    ESTM(reset_state = rd_rx_lane_dp_reset_state());
#else
    ESTM(reset_state = rd_lane_dp_reset_state());
#endif
    if(reset_state < 7) {
        EFUN_PRINTF(("ERROR: merlin8_phy1_set_rx_osr_mode(..) called without ln_dp_s_rstb=0\n"));
        return (merlin8_phy1_error(sa__, ERR_CODE_LANE_DP_NOT_RESET));
    }
    EFUN(wr_rx_osr_mode_frc_val((uint8_t)rx_osr_mode));
    EFUN(wr_rx_osr_mode_frc(0x1));
    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_rx_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum *rx_osr_mode) {
    INIT_SRDS_ERR_CODE
    uint8_t rx_osr_ctrl;

    if(rx_osr_mode == NULL) {
        return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
    }

    ESTM(rx_osr_ctrl = rd_rx_osr_mode_frc());
    if(rx_osr_ctrl) {
        ESTM(*rx_osr_mode = (enum merlin8_phy1_osr_mode_enum)(rd_rx_osr_mode_frc_val()));
    }
    else {
        ESTM(*rx_osr_mode = (enum merlin8_phy1_osr_mode_enum)rd_rx_osr_mode_pin());
    }

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_set_tx_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum tx_osr_mode) {
    INIT_SRDS_ERR_CODE
    uint8_t reset_state;
#ifdef SERDES_TX_RX_SEPARATE
    ESTM(reset_state = rd_tx_lane_dp_reset_state());
#else
    ESTM(reset_state = rd_lane_dp_reset_state());
#endif
    if(reset_state < 7) {
        EFUN_PRINTF(("ERROR: merlin8_phy1_set_tx_osr_mode(..) called without ln_dp_s_rstb=0\n"));
        return (merlin8_phy1_error(sa__, ERR_CODE_LANE_DP_NOT_RESET));
    }
    EFUN(wr_tx_osr_mode_frc_val((uint8_t)tx_osr_mode));
    EFUN(wr_tx_osr_mode_frc(0x1));

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_tx_osr_mode(srds_access_t *sa__, enum merlin8_phy1_osr_mode_enum *tx_osr_mode) {
    INIT_SRDS_ERR_CODE
    uint8_t tx_osr_ctrl;

    if(tx_osr_mode == NULL) {
        return ERR_CODE_BAD_PTR_OR_INVALID_INPUT;
    }

    ESTM(tx_osr_ctrl = rd_tx_osr_mode_frc());
    if(tx_osr_ctrl) {
        ESTM(*tx_osr_mode = (enum merlin8_phy1_osr_mode_enum)(rd_tx_osr_mode_frc_val()));
    }
    else {
        ESTM(*tx_osr_mode = (enum merlin8_phy1_osr_mode_enum)rd_tx_osr_mode_pin());
    }

    return(ERR_CODE_NONE);
}

err_code_t merlin8_phy1_ucode_configure_data_ram(srds_access_t *sa__, char const * ucode_image) {
    INIT_SRDS_ERR_CODE
    srds_info_t * merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr(sa__);
    uint8_t  ramCfg; // Configure number of kbytes of data ram
    uint8_t  ramWr = 1; // Enable ram code write restrictions
    int stackTop = ((int *) ucode_image)[0], info_table_base = ((int *) ucode_image)[4];

    if (0 == info_table_base)
        info_table_base = INFO_TABLE_RAM_BASE;
    merlin8_phy1_info_ptr->core_var_ram_base = (uint32_t) ((int *) ucode_image)[(info_table_base + INFO_TABLE_OFFS_CORE_MEM_PTR) / ((int) sizeof(int))];
    /* DATA/CODE RAM allocation with single code/data address space support */
    // This assumes that stack and data are in the same memory area and that if msnibble of the data is not 2 it is 0.
    if (0 == (stackTop & 0x20000000)) /* single code/data address space */
    {
      ramCfg = 0;
      ramWr = 0; // Disable ram code write restrictions to allow code and data to coexist.

    }
    else
    {
      // This assumes that the stack top is located within 1 kbytes of the top of the data ram
      #define LOG2_1K 10
      // compute ceil(stackTop/1024)
      ramCfg = (uint8_t) (((stackTop & 0xFFFF) + (1<<LOG2_1K)-1) >> LOG2_1K);
      EFUN(reg_wrc_MICRO_C_COM_RAM_CONTROL0((uint16_t) (0x8001 | (ramCfg << 8)))); /* DATA/CODE RAM allocation */
    }

    EFUN(wrc_micro_dr_size(ramCfg));
    EFUN(wrc_micro_ignore_m0_code_writes(ramWr));
    EFUN(wrc_micro_dr_looktab_en(ramWr));

    merlin8_phy1_info_ptr->hw_exception_mem_ram_base = (uint32_t) ((int *) ucode_image)[(info_table_base + INFO_TABLE_OFFS_HW_EXCEPTION_MEM_PTR) / ((int) sizeof(uint32_t))];
    return(ERR_CODE_NONE);
}

/********************************/
/*  Loading ucode through PRAM  */
/********************************/
#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_ucode_pram_load(srds_access_t *sa__, char const * ucode_image, uint32_t ucode_len) {
    INIT_SRDS_ERR_CODE
    uint32_t *ucode_ptr = (uint32_t *) ucode_image;

    if(!ucode_image) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
    EFUN(merlin8_phy1_ucode_configure_data_ram(sa__, ucode_image));

    EFUN(merlin8_phy1_ucode_load_init(sa__, 1));
    EFUN(merlin8_phy1_acc_wr_pram(sa__, 1, 0));  // PRAM interface init

    /* Wait 8 pram clocks */
    EFUN(USR_DELAY_US(1));

    /* write ucode into pram */
    while (ucode_len > 3) {
        EFUN(merlin8_phy1_acc_wr_pram(sa__, 0, *ucode_ptr++));
        ucode_len -= 4;
    }
    /* remainder of 32 bits */
    if (ucode_len != 0)
      EFUN(merlin8_phy1_acc_wr_pram(sa__, 0, *ucode_ptr & (0xFFFFFFFF << ((4-ucode_len) << 3))));
    /* Wait 8 pram clocks */
    EFUN(USR_DELAY_US(1));

    EFUN(merlin8_phy1_ucode_load_close(sa__, 1));
    EFUN(merlin8_phy1_acc_wr_pram(sa__, 2, 0));  // PRAM interface done
    return(ERR_CODE_NONE);
}

#ifndef PCIEG3_V1
/************************************/
/*  Accessing core_config_from_pcs  */
/************************************/

err_code_t merlin8_phy1_set_core_config_from_pcs(srds_access_t *sa__, uint8_t core_cfg_from_pcs) {
    INIT_SRDS_ERR_CODE
    struct merlin8_phy1_uc_core_config_st core_config = UC_CORE_CONFIG_INIT;
    EFUN(merlin8_phy1_get_uc_core_config(sa__, &core_config));
    core_config.field.core_cfg_from_pcs = core_cfg_from_pcs;
    EFUN(merlin8_phy1_INTERNAL_set_uc_core_config(sa__, core_config));
    return (ERR_CODE_NONE);
}

#endif /* !MERLIN7_PCIEG3 */

/************************/
/*  Serdes API Version  */
/************************/

err_code_t merlin8_phy1_version(srds_access_t *sa__, uint32_t *api_version) {
    if(!api_version) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
    *api_version = API_VERSION;
    return (ERR_CODE_NONE);
}
#endif /* SMALL_FOOTPRINT */

/*****************/
/*  PMD_RX_LOCK  */
/*****************/

err_code_t merlin8_phy1_pmd_lock_status(srds_access_t *sa__, uint8_t *pmd_rx_lock) {
    INIT_SRDS_ERR_CODE
    if(!pmd_rx_lock) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
    ESTM(*pmd_rx_lock = rd_pmd_rx_lock());
    return (ERR_CODE_NONE);
}

/**********************************/
/*  Serdes TX disable/RX Restart  */
/**********************************/

err_code_t merlin8_phy1_tx_disable(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE

    if (enable) {
        EFUN(wr_sdk_tx_disable(0x1));
    }
    else {
        EFUN(wr_sdk_tx_disable(0x0));
    }
    return(ERR_CODE_NONE);
}

#ifndef PCIEG3_V1
err_code_t merlin8_phy1_rx_restart(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE

    EFUN(wr_rx_restart_pmd_hold(enable));
    return(ERR_CODE_NONE);
}
#endif

#if !defined(SMALL_FOOTPRINT)
/******************************************************/
/*  Single function to set/get all RX AFE parameters  */
/******************************************************/

err_code_t merlin8_phy1_write_rx_afe(srds_access_t *sa__, enum srds_rx_afe_settings_enum param, int8_t val) {
  /* Assumes the micro is not actively tuning */

    switch(param) {
    case RX_AFE_PF:
        return(merlin8_phy1_INTERNAL_set_rx_pf_main(sa__, (uint8_t)val));

    case RX_AFE_PF2:
        return(merlin8_phy1_INTERNAL_set_rx_pf2(sa__, (uint8_t)val));

    case RX_AFE_VGA:
      return(merlin8_phy1_INTERNAL_set_rx_vga(sa__, (uint8_t)val));


    case RX_AFE_DFE1:
        return(merlin8_phy1_INTERNAL_set_rx_dfe1(sa__, val));

    case RX_AFE_DFE2:
        return(merlin8_phy1_INTERNAL_set_rx_dfe2(sa__, val));

    case RX_AFE_DFE3:
        return(merlin8_phy1_INTERNAL_set_rx_dfe3(sa__, val));

    case RX_AFE_DFE4:
        return(merlin8_phy1_INTERNAL_set_rx_dfe4(sa__, val));

    case RX_AFE_DFE5:
        return(merlin8_phy1_INTERNAL_set_rx_dfe5(sa__, val));

    default:
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
}
#endif /* !SMALL_FOOTPRINT */

#ifndef SMALL_FOOTPRINT
err_code_t merlin8_phy1_read_rx_afe(srds_access_t *sa__, enum srds_rx_afe_settings_enum param, int8_t *val) {
    INIT_SRDS_ERR_CODE
    /* Assumes the micro is not actively tuning */

    uint8_t rd_val;

    if(!val) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    switch(param) {
    case RX_AFE_PF:
        EFUN(merlin8_phy1_INTERNAL_get_rx_pf_main(sa__, &rd_val));
        *val = (int8_t)rd_val;
        break;
    case RX_AFE_PF2:
        EFUN(merlin8_phy1_INTERNAL_get_rx_pf2(sa__, &rd_val));
        *val = (int8_t)rd_val;
        break;
    case RX_AFE_VGA:
        EFUN(merlin8_phy1_INTERNAL_get_rx_vga(sa__, &rd_val));
        *val = (int8_t)rd_val;
      break;
    case RX_AFE_DFE1:
        EFUN(merlin8_phy1_INTERNAL_get_rx_dfe1(sa__, val));
        break;
    case RX_AFE_DFE2:
        EFUN(merlin8_phy1_INTERNAL_get_rx_dfe2(sa__, val));
        break;
    case RX_AFE_DFE3:
        EFUN(merlin8_phy1_INTERNAL_get_rx_dfe3(sa__, val));
        break;
    case RX_AFE_DFE4:
        EFUN(merlin8_phy1_INTERNAL_get_rx_dfe4(sa__, val));
        break;
    case RX_AFE_DFE5:
        EFUN(merlin8_phy1_INTERNAL_get_rx_dfe5(sa__, val));
        break;
    default:
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    UNUSED(rd_val);
    return(ERR_CODE_NONE);
}
#endif /* SMALL_FOOTPRINT */


/**********************************************/
/*  Loopback and Ultra-Low Latency Functions  */
/**********************************************/
#if !defined(PCIEG3_V1)
/* Locks TX_PI to Loop timing */
err_code_t merlin8_phy1_loop_timing(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE

    if (enable) {
        EFUN(wr_tx_pi_en(0x1));                  /* TX_PI enable: 0 = disabled, 1 = enabled */
        EFUN(wr_tx_pi_jitter_filter_en(0x1));    /* Jitter filter enable to lock freq: 0 = diabled, 1 = enabled */
        EFUN(USR_DELAY_US(25));                  /* Wait for tclk to lock to CDR */
    }
    else {
        EFUN(wr_tx_pi_jitter_filter_en(0x0));    /* Jitter filter enable to lock freq: 0 = diabled, 1 = enabled */
        EFUN(wr_tx_pi_en(0x0));                  /* TX_PI enable: 0 = diabled, 1 = enabled */
    }
    return (ERR_CODE_NONE);
}
#endif

#ifndef PCIEG3_V1
/* Setup Remote Loopback mode  */
err_code_t merlin8_phy1_rmt_lpbk(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE
    if (enable) {
        EFUN(merlin8_phy1_loop_timing(sa__, enable));
        EFUN(wr_tx_pi_ext_ctrl_en(0x1));  /* PD path enable: 0 = diabled, 1 = enabled */
        EFUN(wr_rmt_lpbk_en(0x1));        /* Remote Loopback Enable: 0 = diabled, 1 = enable  */
        EFUN(USR_DELAY_US(50));        /* Wait for rclk and tclk phase lock before expecing good data from rmt loopback */
    } else {                              /* Might require longer wait time for smaller values of tx_pi_ext_phase_bwsel_integ */
        EFUN(wr_rmt_lpbk_en(0x0));        /* Remote Loopback Enable: 0 = diabled, 1 = enable  */
        EFUN(wr_tx_pi_ext_ctrl_en(0x0));  /* PD path enable: 0 = diabled, 1 = enabled */
        EFUN(merlin8_phy1_loop_timing(sa__, enable));
    }
    return (ERR_CODE_NONE);
}
#endif /* MERLIN7_PCIEG3 */


/********************************/
/*  TX_PI Fixed Frequency Mode  */
/********************************/
#ifndef SMALL_FOOTPRINT
/* TX_PI Frequency Override (Fixed Frequency Mode) */
err_code_t merlin8_phy1_tx_pi_freq_override(srds_access_t *sa__, uint8_t enable, int16_t freq_override_val) {
    INIT_SRDS_ERR_CODE
    if (enable) {
        EFUN(wr_tx_pi_en(0x1));                              /* TX_PI enable :            0 = disabled, 1 = enabled */
        EFUN(wr_tx_pi_freq_override_val((uint16_t)freq_override_val)); /* Fixed Freq Override Value (+/-8192) */
        EFUN(wr_tx_pi_freq_override_en(0x1));                /* Fixed freq mode enable:   0 = disabled, 1 = enabled */
    }
    else {
        EFUN(wr_tx_pi_freq_override_en(0x0));                /* Fixed freq mode enable:   0 = disabled, 1 = enabled */
        EFUN(wr_tx_pi_freq_override_val(0));                 /* Fixed Freq Override Value to 0 */
        EFUN(wr_tx_pi_en(0x0));                              /* TX_PI enable :            0 = disabled, 1 = enabled */
    }
  return (ERR_CODE_NONE);
}
#endif /* SMALL_FOOTPRINT */

/*********************************/
/*  uc_active and uc_reset APIs  */
/*********************************/
err_code_t merlin8_phy1_get_micro_num_uc_cores(srds_access_t *sa__, uint8_t *num_micros) {
    UNUSED(sa__);
    *num_micros = 1;
    return (ERR_CODE_NONE);
}

/* Toggle core reset */
err_code_t merlin8_phy1_core_reset(srds_access_t *sa__, ucode_info_t ucode_info) {
    INIT_SRDS_ERR_CODE
    uint8_t i, num_lanes;
    uint8_t orig_pll = merlin8_phy1_acc_get_pll_idx(sa__);
    uint8_t orig_lane = merlin8_phy1_acc_get_lane(sa__);
    EFUN(merlin8_phy1_uc_reset(sa__, 1, ucode_info));   /* Assert uC reset */
    EFUN(wrc_mdio_brcst_port_addr(0x1f));         /* mdio_brcst_port_addr is supposed to be reset to default 0x1f, but can't in fact. So has to be set explicitly.*/
    ESTM(num_lanes = rdc_revid_multiplicity());
    for(i = 0; i < num_lanes; i++) {
        EFUN(merlin8_phy1_acc_set_lane(sa__, i));
        EFUN(merlin8_phy1_ln_dp_reset(sa__, 1));
    }
    EFUN(USR_DELAY_US(1));
    for (i = 0; i < NUM_PLLS; i++) {
        EFUN(merlin8_phy1_acc_set_pll_idx(sa__, i));
        EFUN(merlin8_phy1_core_dp_reset(sa__, 1));
    }
    EFUN(USR_DELAY_US(1));
    EFUN(wrc_core_s_rstb(0x0));                   /* Assert core reset */
    EFUN(wrc_core_s_rstb(0x1));                   /* Deassert core reset */
    EFUN(merlin8_phy1_acc_set_lane(sa__, orig_lane));
    EFUN(merlin8_phy1_acc_set_pll_idx(sa__, orig_pll));
    return (ERR_CODE_NONE);
}

/* Enable or Disable the uC reset; Dummy function to maintain compatibility with BHK16 APIs */
err_code_t merlin8_phy1_uc_reset_with_info(srds_access_t *sa__, uint8_t enable, ucode_info_t ucode_info) {
    return merlin8_phy1_uc_reset(sa__, enable, ucode_info);
}

/* Enable or Disable the uC reset */
err_code_t merlin8_phy1_uc_reset(srds_access_t *sa__, uint8_t enable, ucode_info_t ucode_info) {
    INIT_SRDS_ERR_CODE
  if (enable) {
    /* Assert micro reset and reset all micro registers (all non-status registers written to default value) */
    EFUN(wrc_uc_active(0));
    {
        uint8_t micro_orig, num_micros, micro_idx;
        ESTM(micro_orig = merlin8_phy1_acc_get_micro_idx(sa__));
        EFUN(merlin8_phy1_get_micro_num_uc_cores(sa__, &num_micros));
        for (micro_idx = 0; micro_idx < num_micros; micro_idx++) {
            EFUN(merlin8_phy1_acc_set_micro_idx(sa__, micro_idx));
            EFUN(wrc_micro_core_rstb(0));
            EFUN(wrc_micro_core_clk_en(0));               /* Disable clocks to micro cores */
        }
        EFUN(merlin8_phy1_acc_set_micro_idx(sa__, micro_orig));
    }

    EFUN(wrc_micro_master_clk_en(0x0));                   /* Disable clock to microcontroller subsystem */
    /* Reset all Merlin7 Micro registers */
// FIXME NOW! replace with register named macros
#if 0
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD200, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD201, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD202, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD204, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD205, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD206, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD207, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD208, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD209, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD20A, 0x0000)); /* THIS IS A READ ONLY REGISTER! */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD20B, 0x0000)); /* THIS IS A READ ONLY REGISTER! */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD20C, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD20D, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD20E, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD211, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD212, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD213, 0x0000)); /* this is a read only register! */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD214, 0x0000)); /* this is a read only register */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD215, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD216, 0x0007));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD217, 0x0000)); /* read only */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD218, 0x0000)); /* read only */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD219, 0x0000)); /* read only */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD21A, 0x0000)); /* read only */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD21B, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD220, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD221, 0x0000));
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD224, 0x0000));

    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD225, 0x8301)); /* DATA/CODE RAM allocation */
#else

    EFUN(reg_wrc_MICRO_A_COM_CLOCK_CONTROL0(0)            );
    EFUN(reg_wrc_MICRO_A_COM_RESET_CONTROL0(0)            );
    EFUN(reg_wrc_MICRO_A_COM_AHB_CONTROL0(0)              );
    EFUN(reg_wrc_MICRO_A_COM_AHB_WRADDR_LSW(0)            );
    EFUN(reg_wrc_MICRO_A_COM_AHB_WRADDR_MSW(0)            );
    EFUN(reg_wrc_MICRO_A_COM_AHB_WRDATA_LSW(0)            ); /* Write causes memory modification */
    EFUN(reg_wrc_MICRO_A_COM_AHB_WRDATA_MSW(0)            ); /* Write causes memory modification */
    EFUN(reg_wrc_MICRO_A_COM_AHB_RDADDR_LSW(0)            );
    EFUN(reg_wrc_MICRO_A_COM_AHB_RDADDR_MSW(0)            );
    EFUN(reg_wrc_MICRO_A_COM_AHB_RDDATA_LSW(0)            ); /* READ ONLY REGISTER */
    EFUN(reg_wrc_MICRO_A_COM_AHB_RDDATA_MSW(0)            ); /* READ ONLY REGISTER */
    EFUN(reg_wrc_MICRO_A_COM_PRAMIF_CONTROL0(0)           );
    EFUN(reg_wrc_MICRO_A_COM_PRAMIF_AHB_WRADDR_LSW(0)     );
    EFUN(reg_wrc_MICRO_A_COM_PRAMIF_AHB_WRADDR_MSW(0)     );
    EFUN(reg_wrc_MICRO_B_COM_RMI_TO_MICRO_MBOX0(0)        );
    EFUN(reg_wrc_MICRO_B_COM_RMI_TO_MICRO_MBOX1(0)        );
    EFUN(reg_wrc_MICRO_B_COM_MICRO_TO_RMI_MBOX0(0)        ); /* READ ONLY REGISTER */
    EFUN(reg_wrc_MICRO_B_COM_MICRO_TO_RMI_MBOX1(0)        ); /* READ ONLY REGISTER */
    EFUN(reg_wrc_MICRO_B_COM_RMI_MBOX_CONTROL0(0)         );
    EFUN(reg_wrc_MICRO_B_COM_RMI_AHB_CONTROL1(0x0007)     );
    EFUN(reg_wrc_MICRO_B_COM_RMI_AHB_STATUS1(0)           ); /* READ ONLY REGISTER */
    EFUN(reg_wrc_MICRO_B_COM_RMI_RA_AUTOINC_NXT_WRADDR_LSW(0) ); /* READ ONLY REGISTER */
    EFUN(reg_wrc_MICRO_B_COM_RMI_RA_AUTOINC_NXT_RDADDR_LSW(0) ); /* READ ONLY REGISTER */
    EFUN(reg_wrc_MICRO_B_COM_RMI_PR_AUTOINC_NXT_WRADDR_LSW(0) ); /* READ ONLY REGISTER */
    EFUN(reg_wrc_MICRO_B_COM_RMI_PVT_CONTROL0(0)          );
#if 0 /* Registers not modified by original code */
    EFUN(reg_wrc_MICRO_B_COM_MICRO_CORE_CFG_FWAPI_DATA0(0));
    EFUN(reg_wrc_MICRO_B_COM_MICRO_CORE_CFG_FWAPI_DATA1(0));
#endif
    EFUN(reg_wrc_MICRO_C_COM_CODE_RAM_ECCCONTROL0(0)      );
    EFUN(reg_wrc_MICRO_C_COM_CODE_RAM_ECCCONTRO1(0)       );
    EFUN(reg_wrc_MICRO_C_COM_CODE_RAM_TESTIFCONTROL0(0)   );
    EFUN(reg_wrc_MICRO_C_COM_RAM_CONTROL0(0x8301)         );
#endif
    UNUSED(ucode_info);


  }
  else {
    /* De-assert micro reset - Start executing code */
    EFUN(wrc_micro_master_clk_en (0x1));                  /* Enable clock to microcontroller subsystem */
    EFUN(wrc_micro_master_rstb   (0x1));                  /* De-assert reset to microcontroller sybsystem */
    // This is tested to work OK for MERLIN8_PCIEG3 A0
    EFUN(wrc_micro_ra_init(0x2));                         /* Write command for data RAM initialization */
    EFUN(merlin8_phy1_INTERNAL_poll_micro_ra_initdone(sa__, 5));/* Poll status of data RAM initialization */
    EFUN(wrc_micro_ra_init(0x0));                         /* Clear command for data RAM initialization */
    {
        uint8_t micro_orig, num_micros = 0;
        int8_t micro_idx;
        ESTM(micro_orig = merlin8_phy1_acc_get_micro_idx(sa__));
        EFUN(merlin8_phy1_get_micro_num_uc_cores(sa__, &num_micros));
        for (micro_idx = (int8_t)(num_micros-1); micro_idx >= 0; micro_idx--) {
            EFUN(merlin8_phy1_acc_set_micro_idx(sa__, (uint8_t)micro_idx));
            EFUN(wrc_micro_core_clk_en(0x1));          /* Enable clock to micro core */
            EFUN(wrc_micro_core_rstb(0x1));
        }
        EFUN(merlin8_phy1_acc_set_micro_idx(sa__, micro_orig));
    }
    UNUSED(ucode_info);
  }
  return (ERR_CODE_NONE);
}

#ifndef PCIEG3_V1
/* Wait for uC to become active */
err_code_t merlin8_phy1_wait_uc_active(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint16_t loop;
    for (loop = 0; loop < 10000; loop++) {
        uint16_t rddata;
        ESTM(rddata = rdc_uc_active());
        if (rddata) {
            return (ERR_CODE_NONE);
        }
        if (loop>10) {
            EFUN(USR_DELAY_US(1));
        }
    }
    return (merlin8_phy1_error(sa__, ERR_CODE_MICRO_INIT_NOT_DONE));
}
#endif /* MERLIN7_PCIEG3 */

static err_code_t _merlin8_phy1_get_silicon_version(srds_access_t *sa__, uint8_t *silicon_version);
static err_code_t _merlin8_phy1_get_silicon_version(srds_access_t *sa__, uint8_t *silicon_version) {
    UNUSED(sa__);
    *silicon_version = 0xFF; /* Only used for UAPI variants */
    return ERR_CODE_NONE;
}

static err_code_t _merlin8_phy1_ucode_core_ip_check(srds_access_t *sa__);
err_code_t _merlin8_phy1_ucode_core_ip_check(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint8_t status_byte;
    /* Check to make sure that ucode IP matches core IP */
    ESTM(status_byte = rdcv_status_byte());
    if (status_byte & 0x20) {
        EFUN_PRINTF(("Microcode IP did not match core! (status_byte = 0x%x) rdc_revid_model 0x%x rdc_revid2 0x%x\n", status_byte,rdc_revid_model() ,rdc_revid2()));
        return ERR_CODE_UCODE_IP_DOES_NOT_MATCH_CORE;
    }
    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_init_merlin8_phy1_info(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    srds_info_t * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr(sa__); /* Never put a check here */
//    uint32_t info_table[INFO_TABLE_END / sizeof(uint32_t)], info_table_base = INFO_TABLE_RAM_BASE;
    uint32_t info_table[INFO_TABLE_END / sizeof(uint32_t)], info_table_base;
    uint32_t info_table_signature;
    uint8_t  info_table_version;
    err_code_t err_code = ERR_CODE_NONE;

    ENULL_MEMSET(info_table, 0, sizeof(info_table));

    USR_ACQUIRE_LOCK;

    if (merlin8_phy1_info_ptr == NULL) {
        EFUN_PRINTF(("ERROR:  Info table pointer is null.\n"));
        RELEASE_LOCK_AND_RETURN (merlin8_phy1_error(sa__, ERR_CODE_INFO_TABLE_ERROR));
    }
    if (merlin8_phy1_info_ptr->signature == SRDS_INFO_SIGNATURE) {
        /*Already initalized and so check for microcode version and exit with proper status*/
        err_code = merlin8_phy1_INTERNAL_match_ucode_from_info(sa__, merlin8_phy1_info_ptr);

        if (err_code == ERR_CODE_NONE) {
            /* ucode version match */
            RELEASE_LOCK_AND_RETURN (ERR_CODE_NONE);
        } else {
            /* ucode version mismatch */
            RELEASE_LOCK_AND_RETURN (merlin8_phy1_error(sa__, ERR_CODE_MICRO_INIT_NOT_DONE));
        }
    }
    /*Never initialized so far, continue with initialization */
    info_table_base = merlin8_phy1_rd_long_uc_ram(sa__, &err_code, 0x10);
    /* Release lock before returning from the call to avoid deadlock */
    if (ERR_CODE_NONE != err_code) {
       RELEASE_LOCK_AND_RETURN(merlin8_phy1_error(sa__, err_code));
    }
    //EFUN_PRINTF(("%s:  info_table_base=0x%X\n", __FUNCTION__, info_table_base));
    if (info_table_base == 0)
       info_table_base = INFO_TABLE_RAM_BASE;

    err_code = merlin8_phy1_rdblk_uc_prog_ram(sa__, (uint8_t *)info_table, info_table_base, INFO_TABLE_END);
    /* Release lock before returning from the call to avoid deadlock */
    if (ERR_CODE_NONE != err_code) {
       RELEASE_LOCK_AND_RETURN(merlin8_phy1_error(sa__, err_code));
    }

    info_table_signature = info_table[INFO_TABLE_OFFS_SIGNATURE / sizeof(uint32_t)];
    info_table_version = (uint8_t)(info_table_signature >> 24);
    if (((info_table_signature & 0x00FFFFFF) != (INFO_TABLE_SIGNATURE & 0x00FFFFFF))
        || (!(   ((info_table_version >= 0x32) && (info_table_version <= 0x39))
              || ((info_table_version >= 0x41) && (info_table_version <= 0x5A))))) {
        RELEASE_LOCK_AND_RETURN (merlin8_phy1_error(sa__, ERR_CODE_INFO_TABLE_ERROR));
    }
    merlin8_phy1_info_ptr->lane_count = info_table[INFO_TABLE_OFFS_OTHER_SIZE / sizeof(uint32_t)] & 0xFF;
    merlin8_phy1_info_ptr->trace_memory_descending_writes = ((info_table[INFO_TABLE_OFFS_OTHER_SIZE / sizeof(uint32_t)] & 0x1000000) != 0);
    merlin8_phy1_info_ptr->trace_memory_concurrent_read = ((info_table[INFO_TABLE_OFFS_OTHER_SIZE / sizeof(uint32_t)] & 0x2000000) != 0);

    merlin8_phy1_info_ptr->core_var_ram_size = (info_table[INFO_TABLE_OFFS_OTHER_SIZE / sizeof(uint32_t)] >> 8) & 0xFF;

    merlin8_phy1_info_ptr->lane_var_ram_size = (info_table[INFO_TABLE_OFFS_TRACE_LANE_MEM_SIZE / sizeof(uint32_t)] >> 16) & 0xFFFF;

    merlin8_phy1_info_ptr->diag_mem_ram_size = (info_table[INFO_TABLE_OFFS_DIAG_LANE_MEM_SIZE / sizeof(uint32_t)] & 0xFFFF);
    merlin8_phy1_info_ptr->diag_mem_ram_base = info_table[INFO_TABLE_OFFS_DIAG_MEM_PTR / sizeof(uint32_t)];

    merlin8_phy1_info_ptr->trace_mem_ram_size = (info_table[INFO_TABLE_OFFS_TRACE_LANE_MEM_SIZE / sizeof(uint32_t)] & 0xFFFF);
    merlin8_phy1_info_ptr->trace_mem_ram_base = info_table[INFO_TABLE_OFFS_TRACE_MEM_PTR / sizeof(uint32_t)];

    merlin8_phy1_info_ptr->core_var_ram_base = info_table[INFO_TABLE_OFFS_CORE_MEM_PTR / sizeof(uint32_t)];

    merlin8_phy1_info_ptr->micro_var_ram_base = info_table[INFO_TABLE_OFFS_MICRO_MEM_PTR / sizeof(uint32_t)];
    merlin8_phy1_info_ptr->micro_var_ram_size = (info_table[INFO_TABLE_OFFS_OTHER_SIZE_2 / sizeof(uint32_t)] >> 4) & 0xFF;

    merlin8_phy1_info_ptr->lane_var_ram_base = info_table[INFO_TABLE_OFFS_LANE_MEM_PTR / sizeof(uint32_t)];
    merlin8_phy1_info_ptr->hw_exception_mem_ram_base = info_table[INFO_TABLE_OFFS_HW_EXCEPTION_MEM_PTR / sizeof(uint32_t)];


    merlin8_phy1_info_ptr->micro_count = info_table[INFO_TABLE_OFFS_OTHER_SIZE_2 / sizeof(uint32_t)] & 0xF;
    merlin8_phy1_info_ptr->grp_ram_size = info_table[INFO_TABLE_OFFS_GROUP_RAM_SIZE / sizeof(uint32_t)] & 0xFFFF;
    merlin8_phy1_info_ptr->ucode_version = info_table[INFO_TABLE_OFFS_UC_VERSION /sizeof(uint32_t)];
    merlin8_phy1_info_ptr->info_table_version = info_table_version;
    merlin8_phy1_info_ptr->is_big_endian = merlin8_phy1_INTERNAL_is_big_endian();
    err_code = _merlin8_phy1_get_silicon_version(sa__, &merlin8_phy1_info_ptr->silicon_version);
    /* Release lock before returning from the call to avoid deadlock */
    if (ERR_CODE_NONE != err_code) {
        RELEASE_LOCK_AND_RETURN (merlin8_phy1_error(sa__, err_code));
    }

    /* The info table signature should be the last info table member which is populated */
    merlin8_phy1_info_ptr->signature = SRDS_INFO_SIGNATURE;

    err_code = _merlin8_phy1_ucode_core_ip_check(sa__);
    if (ERR_CODE_NONE != err_code) {
        RELEASE_LOCK_AND_RETURN (merlin8_phy1_error(sa__, err_code));
    }

    RELEASE_LOCK_AND_RETURN (ERR_CODE_NONE);
}


err_code_t merlin8_phy1_clear_merlin8_phy1_info(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    srds_info_t * const merlin8_phy1_info_ptr = merlin8_phy1_INTERNAL_get_merlin8_phy1_info_ptr(sa__); /* Never put a check here */

    USR_ACQUIRE_LOCK;

    if (merlin8_phy1_info_ptr == NULL) {
        EFUN_PRINTF(("ERROR:  Info table pointer is null.\n"));
        RELEASE_LOCK_AND_RETURN (merlin8_phy1_error(sa__, ERR_CODE_INFO_TABLE_ERROR));
    }

    ENULL_MEMSET(merlin8_phy1_info_ptr, 0, sizeof(srds_info_t));

    RELEASE_LOCK_AND_RETURN (ERR_CODE_NONE);
}

#ifndef SMALL_FOOTPRINT
#ifndef PCIEG3_V1
/*-----------------*/
/*  Configure PLL  */
/*-----------------*/

err_code_t merlin8_phy1_configure_pll_powerdown(srds_access_t *sa__) {
    return merlin8_phy1_INTERNAL_configure_pll(sa__, MERLIN8_PHY1_PLL_REFCLK_UNKNOWN, MERLIN8_PHY1_PLL_DIV_UNKNOWN, 0, MERLIN8_PHY1_PLL_OPTION_POWERDOWN);
}

err_code_t merlin8_phy1_configure_pll_refclk_div(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           enum merlin8_phy1_pll_div_enum srds_div) {
    return merlin8_phy1_INTERNAL_configure_pll(sa__, refclk, srds_div, 0, MERLIN8_PHY1_PLL_OPTION_NONE);
}

err_code_t merlin8_phy1_configure_pll_refclk_vco(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           uint32_t vco_freq_khz) {
    return merlin8_phy1_INTERNAL_configure_pll(sa__, refclk, MERLIN8_PHY1_PLL_DIV_UNKNOWN, vco_freq_khz, MERLIN8_PHY1_PLL_OPTION_NONE);
}

err_code_t merlin8_phy1_configure_pll_div_vco(srds_access_t *sa__,
                                        enum merlin8_phy1_pll_div_enum srds_div,
                                        uint32_t vco_freq_khz) {
    return merlin8_phy1_INTERNAL_configure_pll(sa__, MERLIN8_PHY1_PLL_REFCLK_UNKNOWN, srds_div, vco_freq_khz, MERLIN8_PHY1_PLL_OPTION_NONE);
}

/* following routines divide refclk input by 2 to prevent fracn mode */
err_code_t merlin8_phy1_configure_pll_refclk_div_div2refclk(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           enum merlin8_phy1_pll_div_enum srds_div) {
   return merlin8_phy1_INTERNAL_configure_pll(sa__, refclk, srds_div, 0, MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV2_EN);
}

err_code_t merlin8_phy1_configure_pll_refclk_vco_div2refclk(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           uint32_t vco_freq_khz) {
    return merlin8_phy1_INTERNAL_configure_pll(sa__, refclk, MERLIN8_PHY1_PLL_DIV_UNKNOWN, vco_freq_khz, MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV2_EN);
}

err_code_t merlin8_phy1_configure_pll_div_vco_div2refclk(srds_access_t *sa__,
                                        enum merlin8_phy1_pll_div_enum srds_div,
                                        uint32_t vco_freq_khz) {
    return merlin8_phy1_INTERNAL_configure_pll(sa__, MERLIN8_PHY1_PLL_REFCLK_UNKNOWN, srds_div, vco_freq_khz, MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV2_EN);
}
/* following routines divide refclk input by 2 to prevent fracn mode */
err_code_t merlin8_phy1_configure_pll_refclk_div_div4refclk(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           enum merlin8_phy1_pll_div_enum srds_div) {
   return merlin8_phy1_INTERNAL_configure_pll(sa__, refclk, srds_div, 0, MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV4_EN);
}

err_code_t merlin8_phy1_configure_pll_refclk_vco_div4refclk(srds_access_t *sa__,
                                           enum merlin8_phy1_pll_refclk_enum refclk,
                                           uint32_t vco_freq_khz) {
    return merlin8_phy1_INTERNAL_configure_pll(sa__, refclk, MERLIN8_PHY1_PLL_DIV_UNKNOWN, vco_freq_khz, MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV4_EN);
}

err_code_t merlin8_phy1_configure_pll_div_vco_div4refclk(srds_access_t *sa__,
                                        enum merlin8_phy1_pll_div_enum srds_div,
                                        uint32_t vco_freq_khz) {
    return merlin8_phy1_INTERNAL_configure_pll(sa__, MERLIN8_PHY1_PLL_REFCLK_UNKNOWN, srds_div, vco_freq_khz, MERLIN8_PHY1_PLL_OPTION_REFCLK_DIV4_EN);
}


err_code_t merlin8_phy1_get_vco_from_refclk_div(srds_access_t *sa__, uint32_t refclk_freq_hz, enum merlin8_phy1_pll_div_enum srds_div, uint32_t *vco_freq_khz, enum merlin8_phy1_pll_option_enum pll_option) {
    INIT_SRDS_ERR_CODE
    EFUN(merlin8_phy1_INTERNAL_get_vco_from_refclk_div(sa__, refclk_freq_hz, srds_div, vco_freq_khz, pll_option));
    return (ERR_CODE_NONE);
}
#endif /* MERLIN7_PCIEG3 */

/***********************************************/
/*  Microcode Load into Program RAM Functions  */
/***********************************************/
err_code_t merlin8_phy1_ucode_load_init(srds_access_t *sa__, uint8_t pram) {
    INIT_SRDS_ERR_CODE
    IF_API_ID_CODES_DONT_MATCH_CORE_ID_CODES
       return (ERR_CODE_API_IP_DOES_NOT_MATCH_CORE);
    }
    EFUN(wrc_micro_master_clk_en(0x1));                     /* Enable clock to microcontroller subsystem */
    EFUN(wrc_micro_master_rstb(0x1));                       /* De-assert reset to microcontroller sybsystem */
    EFUN(wrc_micro_master_rstb(0x0));                       /* Assert reset to microcontroller sybsystem - Toggling reset*/
    EFUN(wrc_micro_master_rstb(0x1));                       /* De-assert reset to microcontroller sybsystem */
    {
      EFUN(wrc_micro_ra_init(0x1));                           /* Set initialization command to initialize code RAM */
      EFUN(merlin8_phy1_INTERNAL_poll_micro_ra_initdone(sa__, 5));  /* Poll for micro_ra_initdone = 1 to indicate initialization done */
#ifdef PCIEG3_V1
      EFUN(wrc_micro_ra_init(0x2));                         /* Write command for data RAM initialization */
      EFUN(merlin8_phy1_INTERNAL_poll_micro_ra_initdone(sa__, 5));/* Poll status of data RAM initialization */
#endif
    }

    EFUN(wrc_micro_ra_init(0x0));

    if(pram) {
        EFUN(wrc_micro_pramif_ahb_wraddr_msw(0x0000));
        EFUN(wrc_micro_pramif_ahb_wraddr_lsw(0x0000));
        EFUN(wrc_micro_pram_if_rstb(1));
        EFUN(wrc_micro_pramif_en(1));
#if 0
        EFUN(wrc_micro_autoinc_wraddr_en(0x1));             /* To auto increment RAM write address */
        EFUN(wrc_micro_ra_wrdatasize(0x2));                 /* Select 32bit transfers */
        EFUN(wrc_micro_ra_wraddr_msw(0x0));                 /* Upper 16bits of start address of Program RAM where the ucode is to be loaded */
        EFUN(wrc_micro_ra_wraddr_lsw(0x0));                 /* Lower 16bits of start address of Program RAM where the ucode is to be loaded */
#endif
    }
    else {
        /* Code to Load microcode */
        EFUN(wrc_micro_autoinc_wraddr_en(0x1));             /* To auto increment RAM write address */
        EFUN(wrc_micro_ra_wrdatasize(0x1));                 /* Select 16bit transfers */
        EFUN(wrc_micro_ra_wraddr_msw(0x0));                 /* Upper 16bits of start address of Program RAM where the ucode is to be loaded */
        EFUN(wrc_micro_ra_wraddr_lsw(0x0));                 /* Lower 16bits of start address of Program RAM where the ucode is to be loaded */
    }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_ucode_load_write(srds_access_t *sa__, uint8_t *ucode_image, uint32_t ucode_len) {
    INIT_SRDS_ERR_CODE
    uint32_t   ucode_len_padded, count = 0;
    uint16_t   wrdata_lsw;
    uint8_t    wrdata_lsb;

    if(!ucode_image) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    ucode_len_padded = ((ucode_len + 3) & 0xFFFFFFFC);    /* Aligning ucode size to 4-byte boundary */

    do {                                                  /* ucode_image loaded 16bits at a time */
        wrdata_lsb = (count < ucode_len) ? ucode_image[count] : 0x0; /* wrdata_lsb read from ucode_image; zero padded to 4byte boundary */
        count++;
        wrdata_lsw = (count < ucode_len) ? ucode_image[count] : 0x0; /* wrdata_msb read from ucode_image; zero padded to 4byte boundary */
        count++;
        wrdata_lsw = (uint16_t)((wrdata_lsw << 8) | wrdata_lsb);               /* 16bit wrdata_lsw formed from 8bit msb and lsb values read from ucode_image */
        EFUN(wrc_micro_ra_wrdata_lsw(wrdata_lsw));                   /* Program RAM lower 16bits write data */
    }   while (count < ucode_len_padded);                 /* Loop repeated till entire image loaded (upto the 4byte boundary) */

    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_ucode_load_close(srds_access_t *sa__, uint8_t pram) {
    INIT_SRDS_ERR_CODE

    if(!pram) {
        EFUN(wrc_micro_ra_wrdatasize(0x2));               /* Select 32bit transfers as default */
    }
    else
        EFUN(wrc_micro_pramif_en(0));

#ifdef PCIEG3_V1
    EFUN(wrc_micro_master_rstb(0x0));                       /* Assert reset to microcontroller sybsystem - Toggling reset*/
    EFUN(wrc_micro_master_rstb(0x1));                       /* De-assert reset to microcontroller sybsystem */
    //EFUN(wrc_micro_core_rstb(0x1));
#endif
    {
        uint8_t micro_orig, num_micros = 0, micro_idx;
        ESTM(micro_orig = merlin8_phy1_acc_get_micro_idx(sa__));
        EFUN(merlin8_phy1_get_micro_num_uc_cores(sa__, &num_micros));
        for (micro_idx = 0; micro_idx < num_micros; micro_idx++) {
            EFUN(merlin8_phy1_acc_set_micro_idx(sa__, micro_idx));
#ifdef PCIEG3_V1
            EFUN(wrc_micro_master_clk_en (0x1));          /* Enable clock to microcontroller subsystem */
#endif
            EFUN(wrc_micro_core_clk_en(0x1));             /* Enable clock to micro core */
        }
        EFUN(merlin8_phy1_acc_set_micro_idx(sa__, micro_orig));
    }

    return (ERR_CODE_NONE);                               /* NO Errors while loading microcode (uCode Load PASS) */

}

/* uCode Load through Register (MDIO) Interface [Return Val = Error_Code (0 = PASS)] */
err_code_t merlin8_phy1_ucode_load(srds_access_t *sa__, uint8_t *ucode_image, uint32_t ucode_len) {
    INIT_SRDS_ERR_CODE

    EFUN(merlin8_phy1_ucode_configure_data_ram(sa__, (char const *) ucode_image));
    EFUN(merlin8_phy1_ucode_load_init(sa__, 0));
    EFUN(merlin8_phy1_ucode_load_write(sa__, ucode_image, ucode_len));
    EFUN(merlin8_phy1_ucode_load_close(sa__, 0));

    return (ERR_CODE_NONE);
}


/* Read-back uCode from Program RAM and verify against ucode_image [Return Val = Error_Code (0 = PASS)]  */
err_code_t merlin8_phy1_ucode_load_verify(srds_access_t *sa__, uint8_t *ucode_image, uint32_t ucode_len) {
    INIT_SRDS_ERR_CODE

    uint32_t ucode_len_padded, count = 0;
    uint16_t rddata_lsw;
    uint16_t data_lsw;
    uint8_t  rddata_lsb;

    if(!ucode_image) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    ucode_len_padded = ((ucode_len + 3) & 0xFFFFFFFC);    /* Aligning ucode size to 4-byte boundary */

    EFUN(wrc_micro_autoinc_rdaddr_en(0x1));               /* To auto increment RAM read address */
    EFUN(wrc_micro_ra_rddatasize(0x1));                   /* Select 16bit transfers */
    EFUN(wrc_micro_ra_rdaddr_msw(0x0));                   /* Upper 16bits of start address of Program RAM from where to read ucode */
    EFUN(wrc_micro_ra_rdaddr_lsw(0x0));                   /* Lower 16bits of start address of Program RAM from where to read ucode */

    do {                                                  /* ucode_image read 16bits at a time */
        rddata_lsb = (count < ucode_len) ? ucode_image[count] : 0x0; /* rddata_lsb read from ucode_image; zero padded to 4byte boundary */
        count++;
        rddata_lsw = (count < ucode_len) ? ucode_image[count] : 0x0; /* rddata_msb read from ucode_image; zero padded to 4byte boundary */
        count++;
        rddata_lsw = (uint16_t)((rddata_lsw << 8) | rddata_lsb);               /* 16bit rddata_lsw formed from 8bit msb and lsb values read from ucode_image */
                                                                     /* Compare Program RAM ucode to ucode_image (Read to micro_ra_rddata_lsw reg auto-increments the ram_address) */
        ESTM(data_lsw = rdc_micro_ra_rddata_lsw());
        if (data_lsw != rddata_lsw) {
            EFUN_PRINTF(("Ucode_Load_Verify_FAIL: Addr = 0x%x: Read_data = 0x%x :  Expected_data = 0x%x \n",(count-2),data_lsw,rddata_lsw));
            return (merlin8_phy1_error(sa__, ERR_CODE_UCODE_VERIFY_FAIL));             /* Verify uCode FAIL */
        }

    } while (count < ucode_len_padded);                   /* Loop repeated till entire image loaded (upto the 4byte boundary) */

    EFUN(wrc_micro_ra_rddatasize(0x2));                   /* Select 32bit transfers ad default */
    return (ERR_CODE_NONE);                               /* Verify uCode PASS */
}

/******************************/
/*  API to Align TX clocks    */
/******************************/
err_code_t merlin8_phy1_tx_clock_align_enable(srds_access_t *sa__, uint8_t enable, uint8_t lane_mask) {
    INIT_SRDS_ERR_CODE
    uint8_t num_lanes, current_lane, lane;

    /* read the number of lanes from register */
    ESTM(num_lanes = rdc_revid_multiplicity());

    ESTM(current_lane = merlin8_phy1_acc_get_lane(sa__));

    /* Check if current lane is part of the mask, else return ERROR */
    if (!((lane_mask >> current_lane) & 0x1))
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));

    /* Start out by disabling all the lanes selected in the mask */
    for(lane = 0;lane<num_lanes;lane++) {
        if ((lane_mask >> lane) & 0x1) {
            EFUN(merlin8_phy1_acc_set_lane(sa__,lane));

           EFUN(wr_ams_tx_sel_txmaster(0));         /* Disable any master lanes */
           /* Initial TX lane clock phase alignment. Program following registers only for slave lanes, master lane registers should not be programmed (can be kept in default values). */
           EFUN(wr_tx_pi_pd_bypass_vco(0));         /* for quick phase lock time */
           EFUN(wr_tx_pi_pd_bypass_flt(0));         /* for quick phase lock time */

// FIXME: MERLIN16_SHORTFIN here is a guess
#if  defined(PCIEG3_V1) 
           EFUN(wr_tx_pi_repeater_mode_en(1));      /* selects external PD from afe as input to tx_pi */
#else
           EFUN(wr_tx_pi_hs_fifo_phserr_sel(0));    /* selects PD from afe as input */
#endif
           EFUN(wr_tx_pi_en(0));                    /* Enable TX_PI    */
           EFUN(wr_tx_pi_jitter_filter_en(0));      /* turn off frequency lock for tx_pi as all te tclk clocks are derived from VCO and same frequency */
           EFUN(wr_tx_pi_ext_ctrl_en(0));           /* turn on PD path to TX_PI to lock the clocks */
           EFUN(wr_tx_pi_ext_phase_bwsel_integ(0)); /* 0 to 7: higher value means faster lock time */
           EFUN(wr_tx_pi_pd_bypass_vco(0));         /* disable filter bypass for more accurate lock */
        }
    }

    if (enable)
    {
        /* Enable master lane first so that when slave lanes are enabled, they start aligning to the master */
        EFUN(merlin8_phy1_acc_set_lane(sa__,current_lane)); /* return lane to previous state */
        EFUN(wr_ams_tx_sel_txmaster(1));          /* Select one lane as txmaster. For all other lanes, this should be 0. */
        EFUN(wr_ams_tx_pd_phasedet(0));           /* 1 - TCA PD powerdown, 0 - enable PD  (enable only for the slave lanes) */

        for(lane = 0;lane<num_lanes;lane++) {
            if ((lane_mask >> lane) & 0x1) {
                if(lane == current_lane) {                   /* Current Lane should be the master lane */
                /* Do nothing */
                } else {
                    EFUN(merlin8_phy1_acc_set_lane(sa__,lane));
                    /* Initial TX lane clock phase alignment. Program following registers only for slave lanes, master lane registers should not be programmed (can be kept in default values). */
                    EFUN(wr_tx_pi_pd_bypass_vco(1));         /* for quick phase lock time */
                    EFUN(wr_tx_pi_pd_bypass_flt(1));         /* for quick phase lock time */
// FIXME: MERLIN16_SHORTFIN here is a guess
#if  defined(PCIEG3_V1) 
                    EFUN(wr_tx_pi_repeater_mode_en(1));      /*  selects external PD from afe as input to tx_pi */
#else
                    EFUN(wr_tx_pi_hs_fifo_phserr_sel(1));    /* selects PD from afe as input */
#endif
#ifdef wr_tx_pi_ext_pd_sel
                    EFUN(wr_tx_pi_ext_pd_sel(0));            /*  selects PD path based on tx_pi_repeater_mode_en */
#endif
                    EFUN(wr_tx_pi_en(1));                    /* Enable TX_PI    */
                    EFUN(wr_tx_pi_jitter_filter_en(0));      /* turn off frequency lock for tx_pi as all te tclk clocks are derived from VCO and same frequency */
                    EFUN(wr_tx_pi_ext_ctrl_en(1));           /* turn on PD path to TX_PI to lock the clocks */
                    EFUN(USR_DELAY_US(25));                  /* wait for the slave lane clocks to lock to the master lane clock */
                    EFUN(wr_tx_pi_ext_phase_bwsel_integ(7)); /* 0 to 7: higher value means faster lock time */
                    EFUN(wr_tx_pi_pd_bypass_vco(0));         /* disable filter bypass for more accurate lock */
                    EFUN(wr_tx_pi_pd_bypass_flt(0));         /* disable filter bypass for more accurate lock */
                }
            }
        }

        EFUN(USR_DELAY_US(60));                          /* wait for the slave lane clocks to lock to the master lane clock withing 1/16th of UI. */

        for(lane = 0;lane<num_lanes;lane++) {
            if ((lane_mask >> lane) & 0x1) {
                EFUN(merlin8_phy1_acc_set_lane(sa__,lane));
                EFUN(wr_tx_pi_en(0));                    /* Freeze TX_PI for all enabled lanes after slave lanes have locked   */
            }
        }
    }
    EFUN(merlin8_phy1_acc_set_lane(sa__,current_lane)); /* return lane to previous state */
    EFUN(wr_ams_tx_sel_txmaster(0));          /* Disable any master lanes and move on to next TCA group */
    return(ERR_CODE_NONE);
 }
#endif /* SMALL_FOOTPRINT */


