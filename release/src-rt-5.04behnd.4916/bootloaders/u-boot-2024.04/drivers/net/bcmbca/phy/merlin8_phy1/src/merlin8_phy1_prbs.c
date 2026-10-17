// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/***********************************************************************************
 ***********************************************************************************
 *  File Name     :  merlin8_phy1_prbs.c                                             *
 *  Created On    :  04 Nov 2015                                                   *
 *  Created By    :  Brent Roberts                                                 *
 *  Description   :  Serdes PRBS test APIs for Serdes IPs                          *
 *  Revision      :     *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/

#ifndef EXCLUDE_STD_HEADERS
#include <stdio.h>
#include <math.h>
#endif

#include "merlin8_phy1_prbs.h"
#include "merlin8_phy1_common.h"
#include "merlin8_phy1_config.h"
#include "merlin8_phy1_dependencies.h"
#include "merlin8_phy1_functions.h"
#include "merlin8_phy1_internal.h"
#include "merlin8_phy1_internal_error.h"
#include "merlin8_phy1_select_defns.h"
#include "merlin8_phy1_access.h"


/*! @file
 *
 */

#ifndef SMALL_FOOTPRINT


/******************************/
/*  TX Pattern Generator APIs */
/******************************/

/* Configure shared TX Pattern (Return Val = 0:PASS, 1-6:FAIL (reports 6 possible error scenarios if failed)) */
err_code_t merlin8_phy1_config_shared_tx_pattern(srds_access_t *sa__, uint8_t patt_length_bits, const char pattern[]) {
    INIT_SRDS_ERR_CODE

  char       patt_final[246] = "";
  char       patt_mod[246]   = "", bin[5] = "";
  size_t     final_count = 0, mod_count = 0;
  int32_t    bytes_written = 0;
  uint8_t    str_len, i, k, j = 0;
  uint8_t    offset_len, actual_patt_len = 0, hex = 0;
  uint8_t    zero_pad_len    = 0; /* suppress warnings, changed by merlin8_phy1_INTERNAL_calc_patt_gen_mode_sel() */
  uint16_t   patt_gen_wr_val = 0;
  uint8_t    mode_sel        = 0; /* suppress warnings, changed by merlin8_phy1_INTERNAL_calc_patt_gen_mode_sel() */

  EFUN(merlin8_phy1_INTERNAL_calc_patt_gen_mode_sel(sa__, &mode_sel,&zero_pad_len,patt_length_bits));

  /* Generating the appropriate write value to patt_gen_seq registers */
  str_len = (uint8_t)USR_STRLEN(pattern);

  if ((str_len > 2) && ((USR_STRNCMP(pattern, "0x", 2)) == 0)) {
    /* Hexadecimal Pattern */
    for (i=2; i < str_len; i++) {
      if (pattern[i] != '_') {
        actual_patt_len = (uint8_t)(actual_patt_len + 4);
        if (actual_patt_len > 240) {
          EFUN_PRINTF(("ERROR: Pattern bigger than max pattern length\n"));
          return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_PATTERN_BIGGER_THAN_MAXLEN));
        }
        EFUN(merlin8_phy1_INTERNAL_compute_bin(sa__, pattern[i],bin, sizeof(bin)));
        bytes_written = USR_SNPRINTF(patt_mod + mod_count, sizeof(patt_mod) - mod_count, "%s",  bin);
        mod_count += (size_t)bytes_written;
        if(( bytes_written < 0) || (mod_count >= sizeof(patt_mod))) {
          return ERR_CODE_REACHED_BUF_SIZE_LIMIT;
        }
      }
    }

    offset_len = (uint8_t)(actual_patt_len - patt_length_bits);
    if ((offset_len > 3)  || (actual_patt_len < patt_length_bits)) {
      EFUN_PRINTF(("ERROR: Pattern length provided does not match the hexadecimal pattern provided\n"));
      return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_LEN_MISMATCH));
    }
    else if (offset_len) {
      for (i=0; i < offset_len; i++) {
        if (patt_mod[i] != '0') {
          EFUN_PRINTF(("ERROR: Pattern length provided does not match the hexadecimal pattern provided\n"));
          return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_LEN_MISMATCH));
        }
      }
      for (i=offset_len; i <= actual_patt_len; i++) {
        patt_mod[i - offset_len] = patt_mod[i];
      }
    }
  }
  else {
    /* Binary Pattern */
    for (i=0; i < str_len; i++) {
      if ((pattern[i] == '0') || (pattern[i] == '1')) {
        actual_patt_len++;
        if (actual_patt_len > 240) {
          EFUN_PRINTF(("ERROR: Pattern bigger than max pattern length\n"));
          return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_PATTERN_BIGGER_THAN_MAXLEN));
        }
        bin[0] = pattern[i];
        bin[1] = '\0';
        bytes_written = USR_SNPRINTF(patt_mod + mod_count, sizeof(patt_mod) - mod_count, "%s",  bin);
        mod_count += (size_t)bytes_written;
        if(( bytes_written < 0) || (mod_count >= sizeof(patt_mod))) {
          return ERR_CODE_REACHED_BUF_SIZE_LIMIT;
        }
      }
      else if (pattern[i] != '_') {
        EFUN_PRINTF(("ERROR: Invalid input Pattern\n"));
        return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_INVALID_PATTERN));
      }
    }

    if (actual_patt_len != patt_length_bits) {
      EFUN_PRINTF(("ERROR: Pattern length provided does not match the binary pattern provided\n"));
      return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_LEN_MISMATCH));
    }
  }

  /* Zero padding upper bits and concatenating patt_mod to form patt_final */
  if(zero_pad_len) {
    bytes_written = USR_SNPRINTF(patt_final + final_count, sizeof(patt_final) - final_count, "%0*d", zero_pad_len, 0);
    final_count += (size_t)bytes_written;
    if(( bytes_written < 0) || (final_count >= sizeof(patt_final))) {
      return ERR_CODE_REACHED_BUF_SIZE_LIMIT;
    }
  }
  for (i=zero_pad_len; i + patt_length_bits < 241; i = (uint8_t)(i + patt_length_bits)) {
    bytes_written = USR_SNPRINTF(patt_final + final_count, sizeof(patt_final) - final_count, "%s", patt_mod);
    final_count += (size_t)bytes_written;
    if(( bytes_written < 0) || (final_count >= sizeof(patt_final))) {
      return ERR_CODE_REACHED_BUF_SIZE_LIMIT;
    }
  }

  /* EFUN_PRINTF(("\nFinal Pattern = %s\n\n",patt_final)); */

  for (i=0; i < 15; i++) {

    for (j=0; j < 4; j++) {
      k = (uint8_t)(i*16 + j*4);
      bin[0] = patt_final[k];
      bin[1] = patt_final[k+1];
      bin[2] = patt_final[k+2];
      bin[3] = patt_final[k+3];
      bin[4] = '\0';
      EFUN(merlin8_phy1_INTERNAL_compute_hex(sa__,bin, &hex));
      patt_gen_wr_val = (uint16_t)((patt_gen_wr_val << 4) | hex);
    }
    /* EFUN_PRINTF(("patt_gen_wr_val[%d] = 0x%x\n",(14-i),patt_gen_wr_val)); */

    /* Writing to appropriate patt_gen_seq Registers */
    switch (i) {
      case 0:  EFUN(wrc_patt_gen_seq_14(patt_gen_wr_val));
               break;
      case 1:  EFUN(wrc_patt_gen_seq_13(patt_gen_wr_val));
               break;
      case 2:  EFUN(wrc_patt_gen_seq_12(patt_gen_wr_val));
               break;
      case 3:  EFUN(wrc_patt_gen_seq_11(patt_gen_wr_val));
               break;
      case 4:  EFUN(wrc_patt_gen_seq_10(patt_gen_wr_val));
               break;
      case 5:  EFUN(wrc_patt_gen_seq_9(patt_gen_wr_val));
               break;
      case 6:  EFUN(wrc_patt_gen_seq_8(patt_gen_wr_val));
               break;
      case 7:  EFUN(wrc_patt_gen_seq_7(patt_gen_wr_val));
               break;
      case 8:  EFUN(wrc_patt_gen_seq_6(patt_gen_wr_val));
               break;
      case 9:  EFUN(wrc_patt_gen_seq_5(patt_gen_wr_val));
               break;
      case 10: EFUN(wrc_patt_gen_seq_4(patt_gen_wr_val));
               break;
      case 11: EFUN(wrc_patt_gen_seq_3(patt_gen_wr_val));
               break;
      case 12: EFUN(wrc_patt_gen_seq_2(patt_gen_wr_val));
               break;
      case 13: EFUN(wrc_patt_gen_seq_1(patt_gen_wr_val));
               break;
      case 14: EFUN(wrc_patt_gen_seq_0(patt_gen_wr_val));
               break;
      default: EFUN_PRINTF(("ERROR: Invalid write to patt_gen_seq register\n"));
               return (merlin8_phy1_error(sa__, ERR_CODE_CFG_PATT_INVALID_SEQ_WRITE));
    }
  }

  /* Pattern Generator Mode Select */
  /* EFUN(wr_patt_gen_mode_sel(mode_sel)); */
  /* EFUN_PRINTF(("Pattern gen Mode = %d\n",mode));    */

  /* Enable Fixed pattern Generation */
  /* EFUN(wr_patt_gen_en(0x1)); */
  return(ERR_CODE_NONE);
}


/* Enable/Disable Shared TX pattern generator */
err_code_t merlin8_phy1_tx_shared_patt_gen_en(srds_access_t *sa__, uint8_t enable, uint8_t patt_length_bits) {
    INIT_SRDS_ERR_CODE
  uint8_t zero_pad_len = 0; /* suppress warnings, changed by merlin8_phy1_INTERNAL_calc_patt_gen_mode_sel() */
  uint8_t mode_sel     = 0; /* suppress warnings, changed by merlin8_phy1_INTERNAL_calc_patt_gen_mode_sel() */

  EFUN(merlin8_phy1_INTERNAL_calc_patt_gen_mode_sel(sa__, &mode_sel,&zero_pad_len,patt_length_bits));

  if (enable) {
    if ((mode_sel < 1) || (mode_sel > 6)) {
      return (merlin8_phy1_error(sa__, ERR_CODE_PATT_GEN_INVALID_MODE_SEL));
    }
    mode_sel = (uint8_t)(12 - mode_sel);
    EFUN(wr_patt_gen_start_pos(mode_sel));            /* Start position for pattern */
    EFUN(wr_patt_gen_stop_pos(0x0));                  /* Stop position for pattern */
    EFUN(wr_patt_gen_en(0x1));                        /* Enable Fixed pattern Generation  */
  }
  else {
    EFUN(wr_patt_gen_en(0x0));                        /* Disable Fixed pattern Generation  */
  }
  return(ERR_CODE_NONE);
}


/**************************************/
/*  PRBS Generator/Checker Functions  */
/**************************************/

/* Configure PRBS Generator */
err_code_t merlin8_phy1_config_tx_prbs(srds_access_t *sa__, enum srds_prbs_polynomial_enum prbs_poly_mode, uint8_t prbs_inv) {
    INIT_SRDS_ERR_CODE
    if(prbs_poly_mode >= PRBS_49 && prbs_poly_mode <= PRBS_AUTO_DETECT) {
      return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
#ifdef PCIEG3_V1
  else if(prbs_poly_mode >= PCS_PRBS_7) {
    uint16_t val = 0;
    uint8_t lane;

    ESTM(lane = merlin8_phy1_acc_get_lane(sa__));
    EFUN(merlin8_phy1_acc_rdt_reg(sa__,PCS_PRBS_ORDER_ADDR, &val));

    switch(prbs_poly_mode) {
      case PCS_PRBS_7:
          val &= (uint16_t)(~(0x3 << (lane * 2)));  /* clear bits 0 and 1 of lane */
      break;
      case PCS_PRBS_15:
          val |= (uint16_t)(0x1 << (lane * 2));               /* set bit 0 of lane */
          val &= (uint16_t)~(1 << ((lane * 2) + 1));          /* clear bit 1 of lane */
      break;
      case PCS_PRBS_23:
          val |= (uint16_t)(0x1 << ((lane * 2) + 1));         /* clear bit 0 of lane */
          val &= (uint16_t)~(1 << (lane * 2));                /* set bit 1 of lane */
      break;
      case PCS_PRBS_31:
          val |= (uint16_t)((0x3 << (lane * 2)));  /* set bits 0 and 1 of lane */
      break;
      default:
          return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_ORDER_ADDR, val));

    EFUN(merlin8_phy1_acc_rdt_reg(sa__,PCS_PRBS_INVERSE_ADDR, &val));
    if(prbs_inv) {
      val |= (uint16_t)(0x1 << lane);
    }
    else {
      val &= (uint16_t)(~(0x1 << lane));
    }

    EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_INVERSE_ADDR, val));
  }
  else
#endif
  {
#ifndef PCIEG3_V1
    if(prbs_poly_mode >= PCS_PRBS_7) {
      return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
#endif
    {
      uint8_t poly_mode_lsb;

      poly_mode_lsb = prbs_poly_mode & 0x7;

      EFUN(wr_prbs_gen_mode_sel((uint8_t)poly_mode_lsb));   /* PRBS Generator mode sel */
    }
    EFUN(wr_prbs_gen_inv(prbs_inv));                      /* PRBS Invert Enable/Disable */
    /* To enable PRBS Generator */
    /* EFUN(wr_prbs_gen_en(0x1)); */
  }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_tx_prbs_config(srds_access_t *sa__, enum srds_prbs_polynomial_enum *prbs_poly_mode, uint8_t *prbs_inv) {
    INIT_SRDS_ERR_CODE
#ifdef PCIEG3_V1
  uint8_t prbs_en = 0;

  EFUN(merlin8_phy1_get_tx_prbs_en(sa__, &prbs_en));
  if(prbs_en == 0x02) {
    uint16_t val = 0;
    uint8_t lane;

    ESTM(lane = merlin8_phy1_acc_get_lane(sa__));
    EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_ORDER_ADDR, &val));

    val = (val >> (2 * lane)) & 0x3;

    switch(val) {
      case 0x0:
        *prbs_poly_mode = PCS_PRBS_7;;
      break;
      case 0x01:
        *prbs_poly_mode = PCS_PRBS_15;
      break;
      case 0x02:
        *prbs_poly_mode = PCS_PRBS_23;
      break;
      case 0x03:
        *prbs_poly_mode = PCS_PRBS_31;
      break;
    }
    EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_INVERSE_ADDR, &val));
    *prbs_inv = ((val & (0x1 << lane)) ? 1 : 0);
  }
  else
#endif
  {
    uint8_t val;

    ESTM(val = rd_prbs_gen_mode_sel());                   /* PRBS Generator mode sel */
    *prbs_poly_mode = (enum srds_prbs_polynomial_enum)val;
    ESTM(val = rd_prbs_gen_inv());                        /* PRBS Invert Enable/Disable */
    *prbs_inv = val;
  }
  return (ERR_CODE_NONE);
}

/* PRBS Generator Enable/Disable */
err_code_t merlin8_phy1_tx_prbs_en(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE
#ifdef PCIEG3_V1
    if((enable & 0x3) == 0x3) {
        return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    else {
        uint16_t val = 0;
        uint8_t lane;

        ESTM(lane = merlin8_phy1_acc_get_lane(sa__));
        EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_ENABLE_ADDR, &val));

        if (enable & 0x2) {
            EFUN(wr_prbs_gen_en(0x0));                                     /* Disable PMD PRBS Generator */
            val |= (uint16_t)(0x1 << lane);
            EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_ENABLE_ADDR, val));      /* Enable PCS PRBS Generator */
        }
        else if(enable & 0x1) {
            val &= (uint16_t)~(0x1 << lane);
            EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_ENABLE_ADDR, val));      /* Disable PCS PRBS */
            EFUN(wr_prbs_gen_en(0x1));                                     /* Enable PMD PRBS Generator */
        }
        else {
            val &= (uint16_t)~(0x1 << lane);
            EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_ENABLE_ADDR, val));      /* Disable PCS PRBS */
            EFUN(wr_prbs_gen_en(0x0));                                     /* Disable PMD PRBS Generator */
        }
    }
#else
    if (enable) {
        EFUN(wr_prbs_gen_en(0x1));                          /* Enable PRBS Generator */
    }
    else {
        EFUN(wr_prbs_gen_en(0x0));                          /* Disable PRBS Generator */
    }
#endif
    return (ERR_CODE_NONE);
}
/* Get PRBS Generator Enable/Disable */
err_code_t merlin8_phy1_get_tx_prbs_en(srds_access_t *sa__, uint8_t *enable) {
    INIT_SRDS_ERR_CODE
    if(!enable) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
#ifdef PCIEG3_V1
    ESTM(*enable = rd_prbs_gen_en());
    if(*enable == 0) {
        uint16_t val = 0;
        uint8_t lane;

        ESTM(lane = merlin8_phy1_acc_get_lane(sa__));
        EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_ENABLE_ADDR, &val));
        *enable = ((val & (0x1 << lane)) ? 0x2 : 0x0);
    }

#else
    ESTM(*enable = rd_prbs_gen_en());
#endif
    return (ERR_CODE_NONE);
}

/* PRBS 1-bit error injection */
err_code_t merlin8_phy1_tx_prbs_err_inject(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE
#ifdef PCIEG3_V1
  uint8_t pcs_enable = 0;
  EFUN(merlin8_phy1_get_tx_prbs_en(sa__, &pcs_enable));
  if(pcs_enable == 0x2) {
    UNUSED(enable);
    EFUN_PRINTF(("WARNING: TX PRBS ERR INJECT is not supported for PCS PRBS.\n"));
  }
  else
#endif
  {
  /* PRBS Error Insert.
     0 to 1 transition on this signal will insert single bit error in the MSB bit of the data bus.
     Reset value is 0x0.
  */
    if(enable) {
      EFUN(wr_prbs_gen_err_ins(0x1));
    }
    EFUN(wr_prbs_gen_err_ins(0));
  }
  return (ERR_CODE_NONE);
}

/* Configure PRBS Checker */
err_code_t merlin8_phy1_config_rx_prbs(srds_access_t *sa__, enum srds_prbs_polynomial_enum prbs_poly_mode, enum srds_prbs_checker_mode_enum prbs_checker_mode, uint8_t prbs_inv) {
    INIT_SRDS_ERR_CODE
  if(prbs_poly_mode >= PRBS_49 && prbs_poly_mode <= PRBS_AUTO_DETECT) {
    return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
  }
#ifdef PCIEG3_V1
  else if(prbs_poly_mode >= PCS_PRBS_7) {
    uint16_t val;
    UNUSED(prbs_checker_mode);
    switch(prbs_poly_mode) {
      case PCS_PRBS_7:
      case PCS_PRBS_15:
      case PCS_PRBS_23:
      case PCS_PRBS_31:
        val = 0xE020;
      break;
      default:
        return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_MONITOR_ADDR, val));        /* Set RX status = PRBS monitor on all lanes */
    EFUN(merlin8_phy1_config_tx_prbs(sa__, prbs_poly_mode, prbs_inv));
  }
  else
#endif
  {
    uint8_t dig_lpbk = 0;
#ifndef PCIEG3_V1
  if(prbs_poly_mode >= PCS_PRBS_7) {
    return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
  }
#endif
      {
        uint8_t poly_mode_lsb;

        poly_mode_lsb = prbs_poly_mode & 0x7;
        EFUN(wr_prbs_chk_mode_sel((uint8_t)poly_mode_lsb));  /* PRBS Checker Polynomial mode sel  */
      }
      EFUN(wr_prbs_chk_mode((uint8_t)prbs_checker_mode));  /* PRBS Checker mode sel (PRBS LOCK state machine select) */
      ESTM(dig_lpbk = rd_dig_lpbk_en());
      if(dig_lpbk == 0) {
        /* Only enable auto mode in non-digital loop-back mode */
        EFUN(wr_prbs_chk_en_auto_mode(0x1));             /* PRBS Checker enable control - rx_dsc_lock & prbs_chk_en */
      }
      EFUN(wr_prbs_chk_inv(prbs_inv));                     /* PRBS Invert Enable/Disable */
  }
  /* To enable PRBS Checker */
  /* EFUN(wr_prbs_chk_en(0x1)); */
  return (ERR_CODE_NONE);
}

/* get PRBS Checker */
err_code_t merlin8_phy1_get_rx_prbs_config(srds_access_t *sa__, enum srds_prbs_polynomial_enum *prbs_poly_mode, enum srds_prbs_checker_mode_enum *prbs_checker_mode, uint8_t *prbs_inv) {
    INIT_SRDS_ERR_CODE
#ifdef PCIEG3_V1
    uint8_t prbs_en = 0;
    EFUN(merlin8_phy1_get_rx_prbs_en(sa__, &prbs_en));
    if(prbs_en == 0x2) {
      UNUSED(*prbs_checker_mode);
      EFUN(merlin8_phy1_get_tx_prbs_config(sa__, prbs_poly_mode, prbs_inv));
    }
    else
#endif
    {
    uint8_t val;

      ESTM(val = rd_prbs_chk_mode_sel());                 /* PRBS Checker Polynomial mode sel  */
      *prbs_poly_mode = (enum srds_prbs_polynomial_enum)val;

      ESTM(val = rd_prbs_chk_mode());                     /* PRBS Checker mode sel (PRBS LOCK state machine select) */
      *prbs_checker_mode = (enum srds_prbs_checker_mode_enum)val;
      ESTM(val = rd_prbs_chk_inv());                      /* PRBS Invert Enable/Disable */
      *prbs_inv = val;
  }
  return (ERR_CODE_NONE);
}

/* PRBS Checker Enable/Disable */
err_code_t merlin8_phy1_rx_prbs_en(srds_access_t *sa__, uint8_t enable) {
    INIT_SRDS_ERR_CODE
#ifdef PCIEG3_V1
    if((enable & 0x3) == 0x3){
        return (ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }
    else {
        uint16_t val = 0;
        uint8_t lane;

        ESTM(lane = merlin8_phy1_acc_get_lane(sa__));
        EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_ENABLE_ADDR, &val));

        if (enable & 0x2) {
            EFUN(wr_prbs_chk_en(0x0));                                     /* Disable PRBS Checker */
            val |= (uint16_t)(0x1 << lane);
            EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_ENABLE_ADDR, val));      /* Enable PCS PRBS Checker */
            EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_LOCK_ERR_ADDR, &val));  /* Flush PRBS Checker prior to using */

        }
        else if(enable & 0x1) {
            val &= (uint16_t)~(0x1 << lane);
            EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_ENABLE_ADDR, val));      /* Disable PCS PRBS Checker */
            EFUN(wr_prbs_chk_en(0x1));                                     /* Enable PMD PRBS Checker */
        }
        else {
            val &= (uint16_t)~(0x1 << lane);
            EFUN(merlin8_phy1_acc_wr_reg(sa__, PCS_PRBS_ENABLE_ADDR, val));      /* Disable PCS PRBS */
            EFUN(wr_prbs_chk_en(0x0));                                     /* Disable PMD PRBS Checker */
        }
    }
#else
    if (enable) {
        EFUN(wr_prbs_chk_burst_err_cnt_en(0x1));            /* Must be enabled before prbs_chk_en; Only enabled when NOT using HW_TIMERS */
        EFUN(wr_prbs_chk_en(0x1));                          /* Enable PRBS Checker */
    }
    else {
        EFUN(wr_prbs_chk_en(0x0));                          /* Disable PRBS Checker */
        EFUN(wr_prbs_chk_burst_err_cnt_en(0x0));
    }
#endif
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_get_rx_prbs_en(srds_access_t *sa__, uint8_t *enable) {
    INIT_SRDS_ERR_CODE
    if(!enable) {
        return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }
#ifdef PCIEG3_V1
    ESTM(*enable = rd_prbs_chk_en());
    if(*enable == 0) {
        uint16_t val = 0;
        uint8_t lane;

        ESTM(lane = merlin8_phy1_acc_get_lane(sa__));
        EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_ENABLE_ADDR, &val));
        *enable = ((val & (0x1 << lane)) ? 0x2 : 0x0);
    }
#else
    ESTM(*enable = rd_prbs_chk_en());
#endif /* MERLIN7_PCIEG3 */
    return (ERR_CODE_NONE);
}


/* PRBS Checker Lock State */
err_code_t merlin8_phy1_prbs_chk_lock_state(srds_access_t *sa__, uint8_t *chk_lock) {
    INIT_SRDS_ERR_CODE
#ifdef PCIEG3_V1
  uint8_t pmd_prbs_chk_en = 0;
#endif
  if(!chk_lock) {
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }
#ifdef PCIEG3_V1
  ESTM(pmd_prbs_chk_en = rd_prbs_chk_en());
  if(pmd_prbs_chk_en == 0) { /* if PMD prbs is not enabled, use PCS prbs */
    uint16_t val = 0;
    EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_LOCK_ERR_ADDR, &val));
    *chk_lock = (uint8_t)((val & 0x8000) >> 15);
  }
  else
#endif
  {
    ESTM(*chk_lock = rd_prbs_chk_lock());                  /* PRBS Checker Lock Indication 1 = Locked, 0 = Out of Lock */
  }
  return (ERR_CODE_NONE);
}

/* PRBS Error Count and Lock Lost (bit 31 in lock lost) */
err_code_t merlin8_phy1_prbs_err_count_ll(srds_access_t *sa__, uint32_t *prbs_err_cnt) {
    INIT_SRDS_ERR_CODE
  uint16_t rddata;

  if(!prbs_err_cnt) {
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }
#ifdef PCIEG3_V1
  ESTM(rddata = rd_prbs_chk_en());
  if(rddata == 0) { /* if PMD prbs is not enabled, use PCS prbs */
    EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_LOCK_ERR_ADDR, &rddata));
    *prbs_err_cnt = rddata & 0x3FFF;
    if(rddata & 0x8000) {
      *prbs_err_cnt |= ((uint32_t)(rddata & 0x4000) << 17); /* Extracting lock lost sticky from bit 14 and moving it to bit 31 */
    }
    else {
      *prbs_err_cnt |= (0x1U << 31);
    }
  }
  else
#endif
  {
    ESTM(rddata = reg_rd_TLB_RX_PRBS_CHK_ERR_CNT_MSB_STATUS());
    *prbs_err_cnt = ((uint32_t) rddata)<<16;
    ESTM(*prbs_err_cnt = (*prbs_err_cnt | rd_prbs_chk_err_cnt_lsb()));
  }
  return (ERR_CODE_NONE);
}

/* PRBS Error Count State  */
err_code_t merlin8_phy1_prbs_err_count_state(srds_access_t *sa__, uint32_t *prbs_err_cnt, uint8_t *lock_lost) {
    INIT_SRDS_ERR_CODE
#ifdef PCIEG3_V1
  uint16_t val;
#endif
  uint8_t dig_lpbk_enable = 0;
  uint8_t link_training_enable = 0;
  if(!prbs_err_cnt || !lock_lost) {
    return(merlin8_phy1_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }
#ifdef PCIEG3_V1
  ESTM(val = rd_prbs_chk_en());
  if(val == 0) { /* if PMD prbs is not enabled, use PCS prbs */
    EFUN(merlin8_phy1_acc_rdt_reg(sa__, PCS_PRBS_LOCK_ERR_ADDR, &val));
    *prbs_err_cnt = val & 0x3FFF;
    if(val & 0x8000) {
      *lock_lost = (uint8_t)((val & 0x4000) >> 14);
    }
    else {
      *lock_lost = 1;
    }
  }
  else
#endif
  {
    EFUN(merlin8_phy1_prbs_err_count_ll(sa__, prbs_err_cnt));
    {
      *lock_lost = (uint8_t)(*prbs_err_cnt >> 31);
    }
    *prbs_err_cnt = (*prbs_err_cnt & 0x7FFFFFFF);

    /* Check if Digital Loopback and LinkTrainig both are enabled */
    ESTM(dig_lpbk_enable = rd_dig_lpbk_en());
#ifndef PCIEG3_V1
    ESTM(link_training_enable = rd_cl72_ieee_training_enable());
#endif

    if(dig_lpbk_enable && link_training_enable) {
      EFUN_PRINTF(("WARNING: PRBS Check Lock - Digital Loopback and Link Training both are enabled\n"));
    }
  }
  return (ERR_CODE_NONE);
}


err_code_t merlin8_phy1_display_detailed_prbs_state_hdr(srds_access_t *sa__) {
    EFUN_PRINTF(("\nPRBS DETAILED DISPLAY :\n"));

/* Comment out display of PRBS burst error if !MERLIN7 */
    EFUN_PRINTF((" LN TX-Mode TX-PRBS-Inv TX-PMD-Inv RX-Mode RX-PRBS-Inv RX-PMD-Inv Lck LL PRBS-Err-Cnt Burst-Err"));
    EFUN_PRINTF(("     BER\n"));
    return (ERR_CODE_NONE);
}

const char* merlin8_phy1_get_e2s_prbs_mode_enum(enum srds_prbs_polynomial_enum prbs_poly_mode) {
    const char* merlin8_phy1_e2s_prbs_mode_enum[8] = {
    " PRBS_7",
    " PRBS_9",
    "PRBS_11",
    "PRBS_15",
    "PRBS_23",
    "PRBS_31",
    "PRBS_58",
    "  ERR  "
    };
    if(prbs_poly_mode >= sizeof(merlin8_phy1_e2s_prbs_mode_enum)/sizeof(merlin8_phy1_e2s_prbs_mode_enum[0])) {
        return "UNKNOWN";
    }
    return merlin8_phy1_e2s_prbs_mode_enum[prbs_poly_mode];
}
#ifdef PCIEG3_V1
const char* merlin8_phy1_get_e2s_pcs_prbs_mode_enum(enum srds_prbs_polynomial_enum prbs_poly_mode) {
    const char* merlin8_phy1_e2s_pcs_prbs_mode_enum[4] = {
    "PCS_PRBS_7",
    "PCS_PRBS_15",
    "PCS_PRBS_23",
    "PCS_PRBS_31"
    };
    if(prbs_poly_mode >= sizeof(merlin8_phy1_e2s_pcs_prbs_mode_enum)/sizeof(merlin8_phy1_e2s_pcs_prbs_mode_enum[0])) {
        return "  UNKNOWN  ";
    }
    return merlin8_phy1_e2s_pcs_prbs_mode_enum[prbs_poly_mode];
}
#endif

err_code_t merlin8_phy1_display_all_detailed_prbs_state(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint8_t lane = 0, num_lanes = 0;
    uint8_t lane_orig = merlin8_phy1_acc_get_lane(sa__);

    ESTM(num_lanes = rdc_revid_multiplicity());
    EFUN(merlin8_phy1_display_detailed_prbs_state_hdr(sa__));
    for(lane = 0; lane < num_lanes; lane++) {
        EFUN(merlin8_phy1_acc_set_lane(sa__, lane));
        EFUN(merlin8_phy1_display_detailed_prbs_state(sa__));
    }
    EFUN(merlin8_phy1_acc_set_lane(sa__, lane_orig));

    return ERR_CODE_NONE;
}

err_code_t merlin8_phy1_display_detailed_prbs_state(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint32_t err_cnt = 0;
    uint8_t lock_lost = 0;
    uint8_t enabled;

    ESTM_PRINTF(("  %d ",merlin8_phy1_acc_get_lane(sa__)));

    ESTM(enabled = rd_prbs_gen_en());
    if(enabled) {
        enum srds_prbs_polynomial_enum prbs_poly_mode = PRBS_7;
        uint8_t prbs_inv = 0;
        const char *prbs_string;
        EFUN(merlin8_phy1_get_tx_prbs_config(sa__, &prbs_poly_mode, &prbs_inv));
        if(prbs_poly_mode == PRBS_UNKNOWN) {
            prbs_string = "UNKNOWN";
        }
#ifdef PCIEG3_V1
        else if(prbs_poly_mode >= PCS_PRBS_7) {
            prbs_string = merlin8_phy1_get_e2s_pcs_prbs_mode_enum((enum srds_prbs_polynomial_enum)(prbs_poly_mode - PCS_PRBS_7));
        }
#endif
        else {
            prbs_string =  merlin8_phy1_get_e2s_prbs_mode_enum(prbs_poly_mode);
        }
        ESTM_PRINTF(("%s",prbs_string));
        ESTM_PRINTF(("      %1d     ",prbs_inv));
    } else {
        EFUN_PRINTF(("  OFF  "));
        ESTM_PRINTF(("      -     "));
    }
    ESTM_PRINTF(("     %1d     ",rd_tx_pmd_dp_invert()));

    ESTM(enabled = rd_prbs_chk_en());
    if(enabled) {
        enum srds_prbs_polynomial_enum prbs_poly_mode = PRBS_7;
        enum srds_prbs_checker_mode_enum prbs_checker_mode;
        uint8_t prbs_inv = 0;
        const char *prbs_string;
        EFUN(merlin8_phy1_get_rx_prbs_config(sa__, &prbs_poly_mode, &prbs_checker_mode, &prbs_inv));
        if(prbs_poly_mode == PRBS_UNKNOWN) {
            prbs_string = "UNKNOWN";
        }
#ifdef PCIEG3_V1
        else if(prbs_poly_mode >= PCS_PRBS_7) {
            prbs_string = merlin8_phy1_get_e2s_pcs_prbs_mode_enum((enum srds_prbs_polynomial_enum)(prbs_poly_mode - PCS_PRBS_7));
        }
#endif
        else {
            prbs_string =  merlin8_phy1_get_e2s_prbs_mode_enum(prbs_poly_mode);
        }
        ESTM_PRINTF((" %s",prbs_string));
        ESTM_PRINTF(("      %1d     ",prbs_inv));
    } else {
        EFUN_PRINTF(("   OFF  "));
        ESTM_PRINTF(("      -     "));
    }

    ESTM_PRINTF(("     %1d     ",rd_rx_pmd_dp_invert()));
    ESTM_PRINTF(("  %d ",rd_prbs_chk_lock()));
    EFUN(merlin8_phy1_prbs_err_count_state(sa__,&err_cnt,&lock_lost));
    EFUN_PRINTF(("  %d  %010d ",lock_lost,err_cnt));
    ESTM_PRINTF(("    %4d  ",rd_prbs_chk_burst_err_cnt()));
    EFUN(merlin8_phy1_INTERNAL_display_BER(sa__,100));
    EFUN_PRINTF(("\n"));

    return (ERR_CODE_NONE);
}



#endif /* SMALL_FOOTPRINT */


#ifdef PCIEG3_V1
/* In Merlin7, one can use the PCS' PRBS generator and monitor it likewise */

err_code_t merlin8_phy1_pcs_prbs23_enable(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE

    /* # set gen3 speed */
    /*EFUN(merlin8_phy1_acc_wr_reg(sa__, 0x1301, 0x0006));*/

    /* # Disable TX rcvr detect req from MAC */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0x5001, 0x8000));

    /* # Disable TX eidle from MAC */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0x1300, 0x2080));

    /* # Disable 8b10b & verify */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0x1402, 0x0000));

    EFUN(merlin8_phy1_config_tx_prbs(sa__, PCS_PRBS_23, 0));
    EFUN(merlin8_phy1_config_rx_prbs(sa__, PCS_PRBS_23, PRBS_INITIAL_SEED_HYSTERESIS, 0));
    EFUN(merlin8_phy1_tx_prbs_en(sa__, 0x2));
    EFUN(merlin8_phy1_rx_prbs_en(sa__, 0x2));

    /* # Force sigdet=ON ([14]=ovrd_select, [13]=ovrd_value */
    EFUN(merlin8_phy1_acc_wr_reg(sa__, 0xD1A3, 0x64FF));

    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_pcs_prbs_monitor_status(srds_access_t *sa__) {
    INIT_SRDS_ERR_CODE
    uint32_t prbs_status = 0;

    /*# PRBS status reg is clear-on-read */
    EFUN(merlin8_phy1_prbs_err_count_ll(sa__, &prbs_status));
    EFUN_PRINTF(("\n  MER7 PCS PRBS status for Lane %d = 0x%08x\n\n", merlin8_phy1_acc_get_lane(sa__), prbs_status));
    return (ERR_CODE_NONE);
}
#endif /* MERLIN7_PCIEG3 */

