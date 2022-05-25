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
#include "phy_drv_merlin16.h"
#include "merlin16_shortfin_debug_functions.h"
#include "merlin16_shortfin_access.h"
#include "merlin16_shortfin_common.h"
#include "merlin16_shortfin_config.h"
#include "merlin16_shortfin_functions.h"
#include "merlin16_shortfin_internal.h"
#include "merlin16_shortfin_internal_error.h"
#include "merlin16_shortfin_prbs.h"
#include "merlin16_shortfin_select_defns.h"
#include "merlin16_shortfin_reg_dump.h"


#if 0
static err_code_t _merlin16_shortfin_reg_print_no_buf(srds_access_t *sa__, uint8_t reg_section);
static err_code_t _merlin16_shortfin_reg_print_with_buf(srds_access_t *sa__, uint8_t reg_section, char reg_buffer[][250], uint8_t *buf_index);
static err_code_t _merlin16_shortfin_reg_print(srds_access_t *sa__, uint8_t reg_section, char reg_buffer[][250], uint8_t *buf_index);


/*************************/
/*  Stop/Resume uC Lane  */
/*************************/

err_code_t merlin16_shortfin_stop_uc_lane(srds_access_t *sa__, uint8_t enable) {

    return(merlin16_shortfin_stop_rx_adaptation(sa__,enable));
}


err_code_t merlin16_shortfin_stop_uc_lane_status(srds_access_t *sa__, uint8_t *uc_lane_stopped) {

  if(!uc_lane_stopped) {
      return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(*uc_lane_stopped = rdv_usr_sts_micro_stopped());

  return (ERR_CODE_NONE);
}

/*******************************************************************/
/*  APIs to Write Core/Lane Config and User variables into uC RAM  */
/*******************************************************************/

err_code_t merlin16_shortfin_set_usr_ctrl_core_event_log_level(srds_access_t *sa__, uint8_t core_event_log_level) {
  return(wrcv_usr_ctrl_core_event_log_level(core_event_log_level));
}

err_code_t merlin16_shortfin_set_usr_ctrl_lane_event_log_level(srds_access_t *sa__, uint8_t lane_event_log_level) {
  return(wrv_usr_ctrl_lane_event_log_level(lane_event_log_level));
}

err_code_t merlin16_shortfin_set_usr_ctrl_disable_startup(srds_access_t *sa__, struct merlin16_shortfin_usr_ctrl_disable_functions_st set_val) {
  EFUN(merlin16_shortfin_INTERNAL_update_usr_ctrl_disable_functions_byte(&set_val));
  return(wrv_usr_ctrl_disable_startup_functions_byte(set_val.byte));
}

err_code_t merlin16_shortfin_set_usr_ctrl_disable_startup_dfe(srds_access_t *sa__, struct merlin16_shortfin_usr_ctrl_disable_dfe_functions_st set_val) {
  EFUN(merlin16_shortfin_INTERNAL_update_usr_ctrl_disable_dfe_functions_byte(&set_val));
  return(wrv_usr_ctrl_disable_startup_dfe_functions_byte(set_val.byte));
}

err_code_t merlin16_shortfin_set_usr_ctrl_disable_steady_state(srds_access_t *sa__, struct merlin16_shortfin_usr_ctrl_disable_functions_st set_val) {
  EFUN(merlin16_shortfin_INTERNAL_check_uc_lane_stopped(sa__));  /* make sure uC is stopped to avoid race conditions */
  EFUN(merlin16_shortfin_INTERNAL_update_usr_ctrl_disable_functions_byte(&set_val));
  return(wrv_usr_ctrl_disable_steady_state_functions_byte(set_val.byte));
}

err_code_t merlin16_shortfin_set_usr_ctrl_disable_steady_state_dfe(srds_access_t *sa__, struct merlin16_shortfin_usr_ctrl_disable_dfe_functions_st set_val) {
  EFUN(merlin16_shortfin_INTERNAL_update_usr_ctrl_disable_dfe_functions_byte(&set_val));
  return(wrv_usr_ctrl_disable_steady_state_dfe_functions_byte(set_val.byte));
}

/******************************************************************/
/*  APIs to Read Core/Lane Config and User variables from uC RAM  */
/******************************************************************/

err_code_t merlin16_shortfin_get_usr_ctrl_core_event_log_level(srds_access_t *sa__, uint8_t *core_event_log_level) {

  if(!core_event_log_level) {
     return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(*core_event_log_level = rdcv_usr_ctrl_core_event_log_level());

  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_get_usr_ctrl_lane_event_log_level(srds_access_t *sa__, uint8_t *lane_event_log_level) {

  if(!lane_event_log_level) {
     return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(*lane_event_log_level = rdv_usr_ctrl_lane_event_log_level());
  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_get_usr_ctrl_disable_startup(srds_access_t *sa__, struct merlin16_shortfin_usr_ctrl_disable_functions_st *get_val) {

  if(!get_val) {
     return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(get_val->byte = rdv_usr_ctrl_disable_startup_functions_byte());
  EFUN(merlin16_shortfin_INTERNAL_update_usr_ctrl_disable_functions_st(get_val));
  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_get_usr_ctrl_disable_startup_dfe(srds_access_t *sa__, struct merlin16_shortfin_usr_ctrl_disable_dfe_functions_st *get_val) {

  if(!get_val) {
     return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(get_val->byte = rdv_usr_ctrl_disable_startup_dfe_functions_byte());
  EFUN(merlin16_shortfin_INTERNAL_update_usr_ctrl_disable_dfe_functions_st(get_val));
  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_get_usr_ctrl_disable_steady_state(srds_access_t *sa__, struct merlin16_shortfin_usr_ctrl_disable_functions_st *get_val) {

  if(!get_val) {
     return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(get_val->byte = rdv_usr_ctrl_disable_steady_state_functions_byte());
  EFUN(merlin16_shortfin_INTERNAL_update_usr_ctrl_disable_functions_st(get_val));
  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_get_usr_ctrl_disable_steady_state_dfe(srds_access_t *sa__, struct merlin16_shortfin_usr_ctrl_disable_dfe_functions_st *get_val) {

  if(!get_val) {
     return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }

  ESTM(get_val->byte = rdv_usr_ctrl_disable_steady_state_dfe_functions_byte());
  EFUN(merlin16_shortfin_INTERNAL_update_usr_ctrl_disable_dfe_functions_st(get_val));
  return (ERR_CODE_NONE);
}

/********************************************************************************/
/* Helper print function for Serdes Register/Variable Dump using no buffer      */
/********************************************************************************/
static err_code_t _merlin16_shortfin_reg_print_no_buf(srds_access_t *sa__, uint8_t reg_section)
{
    uint16_t addr, rddata = 0;

    for (addr = merlin16_shortfin_reg_dump_arr[reg_section][0]; addr < merlin16_shortfin_reg_dump_arr[reg_section][1]; addr++) 
    {
        if (!(addr & 0x000F))  
        {
            EFUN_PRINTF(("\n%04x ",addr));
        }
        EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr,&rddata));
        EFUN_PRINTF(("%04x ",rddata));
    }
    return (ERR_CODE_NONE);
}
/********************************************************************************/
/* Helper print function for Serdes Register/Variable Dump using a buffer       */
/********************************************************************************/
static err_code_t _merlin16_shortfin_reg_print_with_buf(srds_access_t *sa__, uint8_t reg_section, char reg_buffer[][250], uint8_t *buf_index)
{
    uint16_t addr, rddata = 0;
	uint8_t first = 0;
    char temp_buf[6];
	
	if(reg_section == 0) first = 1;

    for (addr = merlin16_shortfin_reg_dump_arr[reg_section][0]; addr < merlin16_shortfin_reg_dump_arr[reg_section][1]; addr++) 
    {
        if (!(addr & 0x000F))  
        {
            if(first) first = 0;
            else (*buf_index)++; /* don't increment buf_index for first section */
            USR_SPRINTF(temp_buf,"%04x ", addr);
            USR_STRCPY(reg_buffer[*buf_index], temp_buf);
        }
        EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr,&rddata));
        USR_SPRINTF(temp_buf,"%04x ", rddata);
        USR_STRNCAT(reg_buffer[*buf_index], temp_buf, 6);
    }
    return (ERR_CODE_NONE);
}
/********************************************************************************/
/* Helper print function for Serdes Register/Variable Dump                      */
/********************************************************************************/
static err_code_t _merlin16_shortfin_reg_print(srds_access_t *sa__, uint8_t reg_section, char reg_buffer[][250], uint8_t *buf_index)
{
    if(reg_buffer == NULL)
    {
         EFUN(_merlin16_shortfin_reg_print_no_buf(sa__, reg_section));
    }
    else
    {
        EFUN(_merlin16_shortfin_reg_print_with_buf(sa__, reg_section, reg_buffer, buf_index));
    }
    return (ERR_CODE_NONE);
}
err_code_t merlin16_shortfin_reg_dump(srds_access_t *sa__, uint8_t core, uint8_t lane, char reg_buffer[][250])
{    
    uint8_t reg_section = 0;
    uint8_t buf_index = 0;

    if (reg_buffer == NULL) 
    {
        EFUN_PRINTF(("\n****  SERDES REGISTER CORE %d LANE %d DUMP    ****", core, lane));
    }
    else
    {
        EFUN_PRINTF(("\n****  SERDES REGISTER CORE %d LANE %d DECODED    ****\n", core, lane));
    }
    
    while(reg_section < MERLIN16_SHORTFIN_REG_DUMP_SECTIONS) 
    {
        if(merlin16_shortfin_reg_dump_arr[reg_section][0] == MERLIN16_SHORTFIN_REG_PLL_START)
        {
                while( reg_section < MERLIN16_SHORTFIN_REG_DUMP_SECTIONS && merlin16_shortfin_reg_dump_arr[reg_section][0] != MERLIN16_SHORTFIN_REG_UC_START)
                {
                    EFUN(_merlin16_shortfin_reg_print(sa__, reg_section, reg_buffer, &buf_index));
                    reg_section++;
                }
        }
        else if (merlin16_shortfin_reg_dump_arr[reg_section][0] == MERLIN16_SHORTFIN_REG_UC_START)
        {
#ifdef SERDES_MULTI_MICROS
            uint8_t micro_orig, micro_idx, num_micros;
            uint8_t uc_start_section = reg_section;

            ESTM(num_micros = rdc_micro_num_uc_cores());
            ESTM(micro_orig = merlin16_shortfin_get_micro_idx(sa__));
            for(micro_idx = 0; micro_idx < num_micros; micro_idx++)
            {
                reg_section = uc_start_section;
                EFUN(merlin16_shortfin_set_micro_idx(sa__,micro_idx));
#endif
                while(reg_section < MERLIN16_SHORTFIN_REG_DUMP_SECTIONS)
                {
                    EFUN(_merlin16_shortfin_reg_print(sa__, reg_section, reg_buffer, &buf_index));
                    reg_section++;
                }
#ifdef SERDES_MULTI_MICROS
            }
            EFUN(merlin16_shortfin_set_micro_idx(sa__,micro_orig));
#endif
        }
        else
        {
            EFUN(_merlin16_shortfin_reg_print(sa__, reg_section, reg_buffer, &buf_index));
            reg_section++;
        }
    } 

    if (reg_buffer != NULL)
    {
        reg_buffer[++buf_index][0] = 0xA;
        reg_buffer[++buf_index][0] = 0;
    }

    return (ERR_CODE_NONE);
}


err_code_t merlin16_shortfin_uc_core_var_dump(srds_access_t *sa__, uint8_t core, char ram_buffer[][250]) {
  uint8_t  addr, rddata, core_var_ram_size;

    core_var_ram_size = 0xFF;
    
  if (ram_buffer == NULL) {
      EFUN_PRINTF(("\n**** SERDES UC CORE %d RAM VARIABLE DUMP ****", core));
      for (addr = 0x0; addr < core_var_ram_size; addr++) {
          if (!(addr % 26))  {
              EFUN_PRINTF(("\n%04x ",addr));
          }
          ESTM_PRINTF(("%02x ", merlin16_shortfin_rdbc_uc_var(sa__, __ERR, addr)));
      }
  }
  else {
      uint8_t index=0;
      char tempBuf[6];

      EFUN_PRINTF(("\n**** SERDES UC CORE %d RAM VARIABLE DECODED ****\n", core));
      for (addr = 0x0; addr < core_var_ram_size; addr++) {
          if (!(addr % 26))  {
             if (addr != 0) {
                index++;
             }
             USR_SPRINTF(tempBuf,"%04x ", addr);
             USR_STRCPY(ram_buffer[index], tempBuf);
          }
          ESTM(rddata = merlin16_shortfin_rdbc_uc_var(sa__, __ERR, addr));
          USR_SPRINTF(tempBuf,"%02x ", rddata);
          USR_STRNCAT(ram_buffer[index], tempBuf, 6);
      }
      ram_buffer[index+1][0] = 0xA;
      ram_buffer[index+2][0] = 0;
  }
  return (ERR_CODE_NONE);
}


err_code_t merlin16_shortfin_uc_lane_var_dump(srds_access_t *sa__, uint8_t core, uint8_t lane, char ram_buffer[][250]) {
  uint8_t     rx_lock, uc_stopped = 0;
  uint16_t    addr, lane_var_ram_size;

  lane_var_ram_size = LANE_VAR_RAM_SIZE;

  ESTM(rx_lock = rd_pmd_rx_lock());

  {
      err_code_t err_code=ERR_CODE_NONE;
      uc_stopped = merlin16_shortfin_INTERNAL_stop_micro(sa__,rx_lock,&err_code);
      if(err_code) USR_PRINTF(("Unable to stop microcontroller,  following data is suspect\n"));
  }

  if (ram_buffer == NULL) {
      EFUN_PRINTF(("\n**** SERDES UC CORE %d LANE %d RAM VARIABLE DUMP ****", core,lane));
      for (addr = 0x0; addr < lane_var_ram_size; addr++) {
         if (!(addr % 26))  {
            EFUN_PRINTF(("\n%04x ",addr));
         }
         ESTM_PRINTF(("%02x ", merlin16_shortfin_rdbl_uc_var(sa__, __ERR, addr)));
      }
  }
  else {
      uint8_t rddata, index = 0;
      char tempBuf[6];
      EFUN_PRINTF(("\n**** SERDES UC CORE %d LANE %d RAM VARIABLE DECODED ****\n", core,lane));

      for (addr = 0x0; addr < lane_var_ram_size; addr++) {
         if (!(addr % 26))  {
            if (addr != 0) {
               index++;
            }
            USR_SPRINTF(tempBuf,"%04x ", addr);
            USR_STRCPY(ram_buffer[index], tempBuf);
         }
         ESTM(rddata = merlin16_shortfin_rdbl_uc_var(sa__, __ERR, addr));
         USR_SPRINTF(tempBuf,"%02x ", rddata);
         USR_STRNCAT(ram_buffer[index], tempBuf, 6);
      }
      ram_buffer[index+1][0] = 0xA;
      ram_buffer[index+2][0] = 0;
  }

  if (rx_lock == 1) {
      if (!uc_stopped) {
          EFUN(merlin16_shortfin_stop_rx_adaptation(sa__, 0));
      }
  } else {
      EFUN(merlin16_shortfin_stop_rx_adaptation(sa__, 0));
  }

  return (ERR_CODE_NONE);
}

/***************************************/
/*  API Function to Read Event Logger  */
/***************************************/

err_code_t merlin16_shortfin_read_event_log(srds_access_t *sa__) {
    merlin16_shortfin_INTERNAL_event_log_dump_state_t state;
    uint8_t micro_num = 0;
    {
        state.index = 0;
        state.line_start_index = 0;
        EFUN(merlin16_shortfin_INTERNAL_read_event_log_with_callback(sa__, micro_num, 0, &state, merlin16_shortfin_INTERNAL_event_log_dump_callback));
        EFUN(merlin16_shortfin_INTERNAL_event_log_dump_callback(&state, 0, 0));
    }
    return(ERR_CODE_NONE);
}

/**********************************************/
/*  Loopback and Ultra-Low Latency Functions  */
/**********************************************/

/* Enable/Disable Digital Loopback */
err_code_t merlin16_shortfin_dig_lpbk(srds_access_t *sa__, uint8_t enable) {
    /* setting/clearing prbs_chk_en_auto_mode while coming out of/going in to dig lpbk */
    EFUN(wr_prbs_chk_en_auto_mode(!enable));
    EFUN(wr_dig_lpbk_en(enable));                         /* 0 = disabled, 1 = enabled */

    /* Disabling Link Training as it's an invalid mode in digital loopback */
    if(enable) {
        uint8_t link_training_enable = 0;
        ESTM(link_training_enable = rd_cl72_ieee_training_enable());
        if(link_training_enable) {
            EFUN_PRINTF(("Warning: Core: %d, Lane %d: Link Training mode is on in digital loopback.\n", merlin16_shortfin_get_core(sa__), merlin16_shortfin_get_lane(sa__)));
        }
    }
  return (ERR_CODE_NONE);
}


/**********************************/
/*  TX_PI Jitter Generation APIs  */
/**********************************/

/* TX_PI Sinusoidal or Spread-Spectrum (SSC) Jitter Generation  */
err_code_t merlin16_shortfin_tx_pi_jitt_gen(srds_access_t *sa__, uint8_t enable, int16_t freq_override_val, enum srds_tx_pi_freq_jit_gen_enum jit_type, uint8_t tx_pi_jit_freq_idx, uint8_t tx_pi_jit_amp) {
    /* Added a limiting for the jitter amplitude index, per freq_idx */
    uint8_t max_amp_idx_r20_os1[] = {37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 42, 48, 56, 33, 39, 47, 58, 37, 48, 33, 47, 37, 33, 37, 37};

    /* Irrespective of the osr_mode, txpi runs @ os1. Thus the max amp idx values remain the same. */
    if (jit_type == TX_PI_SJ) {
        if (tx_pi_jit_amp > max_amp_idx_r20_os1[tx_pi_jit_freq_idx]) {
            tx_pi_jit_amp = max_amp_idx_r20_os1[tx_pi_jit_freq_idx];
        }
    }

    EFUN(merlin16_shortfin_tx_pi_freq_override(sa__, enable, freq_override_val));

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

err_code_t merlin16_shortfin_isolate_ctrl_pins(srds_access_t *sa__, uint8_t enable) {
    uint8_t lane, lane_orig, num_lanes;
        EFUN(merlin16_shortfin_isolate_core_ctrl_pins(sa__, enable));

    ESTM(lane_orig = merlin16_shortfin_get_lane(sa__));
    /* read num lanes per core directly from register */
    ESTM(num_lanes = rdc_revid_multiplicity());
    for(lane = 0; lane < num_lanes; lane++) {
        EFUN(merlin16_shortfin_set_lane(sa__, lane));
        EFUN(merlin16_shortfin_isolate_lane_ctrl_pins(sa__, enable));
    }
    EFUN(merlin16_shortfin_set_lane(sa__, lane_orig));

  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_isolate_lane_ctrl_pins(srds_access_t *sa__, uint8_t enable) {

  if (enable) {
    EFUN(wr_pmd_ln_tx_h_pwrdn_pkill(0x1));
    EFUN(wr_pmd_ln_rx_h_pwrdn_pkill(0x1));
    EFUN(wr_pmd_ln_dp_h_rstb_pkill(0x1));
    EFUN(wr_pmd_ln_h_rstb_pkill(0x1));
    EFUN(wr_pmd_tx_disable_pkill(0x1));
  }
  else {
    EFUN(wr_pmd_ln_tx_h_pwrdn_pkill(0x0));
    EFUN(wr_pmd_ln_rx_h_pwrdn_pkill(0x0));
    EFUN(wr_pmd_ln_dp_h_rstb_pkill(0x0));
    EFUN(wr_pmd_ln_h_rstb_pkill(0x0));
    EFUN(wr_pmd_tx_disable_pkill(0x0));
  }
  return (ERR_CODE_NONE);
}

err_code_t merlin16_shortfin_isolate_core_ctrl_pins(srds_access_t *sa__, uint8_t enable) {

  if (enable) {
    EFUN(wrc_pmd_core_dp_h_rstb_pkill(0x1));
  }
  else {
    EFUN(wrc_pmd_core_dp_h_rstb_pkill(0x0));
  }
  return (ERR_CODE_NONE);
}
#endif 

void merlin16_shortfin_INTERNAL_print_triage_info(srds_access_t *sa__, err_code_t err_code, uint8_t print_header, uint8_t print_data, uint16_t line)
{
return;
#if 0
/*  Note: No EFUNs or ESTMs should be used in this function as this print routine is called by _error() handler. */
    merlin16_shortfin_triage_info info;
    err_code_t error;
    uint8_t error_seen = 0;
    err_code_t __err;
    uint16_t   ucode_version_major;
    uint8_t    ucode_version_minor;
    UNUSED(error);
    memset(&info, 0, sizeof(merlin16_shortfin_triage_info));
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
    error = merlin16_shortfin_version(sa__, &info.api_ver);
    if (error) {
        /* Unable to read api version */
        /*USR_PRINTF(("Unable to capture api version and so setting invalid value in the print\n"));*/
        info.api_ver = 0xFFFFFFFF;
        error_seen = 1;
    }
    CHECK_ERR(ucode_version_major = rdcv_common_ucode_version());
    CHECK_ERR(ucode_version_minor = rdcv_common_ucode_minor_version());
    info.ucode_ver = (uint32_t)((ucode_version_major << 8) | ucode_version_minor);
    CHECK_ERR(info.stop_status = rdv_usr_sts_micro_stopped());
        /* Collect exception and overflow information */
    CHECK_ERR(info.stack_overflow = rdcv_status_byte());
    if(merlin16_shortfin_INTERNAL_sigdet_status(sa__, &info.sig_det, &info.sig_det_chg)) {
        error_seen = 1;
    }
    if (merlin16_shortfin_INTERNAL_pmd_lock_status(sa__, &info.pmd_lock, &info.pmd_lock_chg)) {
        error_seen = 1;
    }

    CHECK_ERR(info.dsc_one_hot[0] = rd_dsc_state_one_hot());
    CHECK_ERR(info.dsc_one_hot[1] = rd_dsc_state_one_hot());
    CHECK_ERR(info.cmd_info = reg_rd_DSC_A_DSC_UC_CTRL());
    info.core = merlin16_shortfin_get_core(sa__);
    info.lane = merlin16_shortfin_get_lane(sa__);
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
                        merlin16_shortfin_INTERNAL_e2s_err_code(info.error) ));
        } else {
            USR_PRINTF(("%4d, %4d,  %X_%X,    %X_%X, %s\n",
                        info.lane,
                        info.core,
                        info.api_ver>>8,
                        info.api_ver & 0xFF,
                        info.ucode_ver >> 8,
                        info.ucode_ver & 0xFF,
                        merlin16_shortfin_INTERNAL_e2s_err_code(info.error) ));
        }
    }
    if (error_seen) {
        USR_PRINTF(("WARNING: There were some errors seen while collecting triage info and so the debug data above may not be all accurate\n"));
    }
    return;
#endif
}


