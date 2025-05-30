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

/*******************************************************************************************
 *******************************************************************************************
 *  File Name     :  merlin16_tx_analog_functions.h                                        *
 *  Created On    :  04/20/2015                                                            *
 *  Created By    :  Kiran Divakar                                                         *
 *  Description   :  Functions to Configure Merlin16 TXFIR                                 *
 *  Revision      :   *
 *                                                                                         *
 *******************************************************************************************
 *******************************************************************************************/

/** @file merlin16_tx_analog_functions.h
 * Functions to configure Merlin16 TX
 */

#ifndef MERLI16_TX_ANALOG_FUNCTIONS_H
#define MERLI16_TX_ANALOG_FUNCTIONS_H

#ifndef EXCLUDE_STD_HEADERS
#include <stdint.h>
#endif

#include "merlin16_shasta_dependencies.h"

/* Structs required for M16 TXFIR AFE slicer controls */

/** Post2 Slicer Register Field Struct */
struct p2sr_regfield_st {
  uint16_t post2_1x_0         ;
  uint16_t dc_level_post2_1x_0;
  uint16_t post2_1x_1         ;
  uint16_t dc_level_post2_1x_1;
  uint16_t post2_2x_0         ;
  uint16_t dc_level_post2_2x_0;
  uint16_t post2_2x_1         ;
  uint16_t dc_level_post2_2x_1;
  uint16_t reserved0          ;
};

/** Post1 Slicer Register Field Struct */
struct p1sr_regfield_st {
  uint16_t post1_1x_0            ;
  uint16_t dc_level_post1_1x_0   ;
  uint16_t post1_1x_1            ;
  uint16_t dc_level_post1_1x_1   ;
  uint16_t post1_2x_0            ;
  uint16_t dc_level_post1_2x_0   ;
  uint16_t post1_2x_1            ;
  uint16_t dc_level_post1_2x_1   ;
  uint16_t post1_2x_2            ;
  uint16_t dc_level_post1_2x_2   ;
  uint16_t reserved0             ;    
  uint16_t pre_post1pre_1x_0     ;
  uint16_t post1_post1pre_1x_0   ;
  uint16_t dc_level_post1pre_1x_0;
  uint16_t reserved1             ;
};

/** Post1Pre Slicer Register Field Struct */
struct presr_regfield_st {
  uint16_t pre_post1pre_1x_1     ;
  uint16_t post1_post1pre_1x_1   ;
  uint16_t dc_level_post1pre_1x_1;
  uint16_t pre_post1pre_2x_0     ;
  uint16_t post1_post1pre_2x_0   ;
  uint16_t dc_level_post1pre_2x_0;
  uint16_t pre_post1pre_2x_1     ;
  uint16_t post1_post1pre_2x_1   ;
  uint16_t dc_level_post1pre_2x_1;
  uint16_t pre_post1pre_2x_2     ;
  uint16_t post1_post1pre_2x_2   ;
  uint16_t dc_level_post1pre_2x_2;
  uint16_t pre_post1pre_2x_3     ;
  uint16_t post1_post1pre_2x_3   ;
  uint16_t dc_level_post1pre_2x_3;
  uint16_t reserved0             ;
};

/** Main Slicer Register Field Struct */
struct mssr_regfield_st {
  uint16_t dc_level_0p5x     ;
  uint16_t dc_level_main_1x  ;
  uint16_t dc_level_main_2x_0;
  uint16_t dc_level_main_2x_1;
  uint16_t reserved0         ;
};

/** Post2 Slicer Register Struct */
struct  p2sr_reg_st {
  struct p2sr_regfield_st field;
  uint16_t word;
};

/** Post1 Slicer Register Struct */
struct  p1sr_reg_st {
  struct p1sr_regfield_st field;
  uint16_t word;
};

/** Post1Pre Slicer Register Struct */
struct  presr_reg_st {
  struct presr_regfield_st field;
  uint16_t word;
};

/** Main Slicer Register Struct */
struct  mssr_reg_st {
  struct mssr_regfield_st field;
  uint16_t word;
};

/** Converts the input TXFIR tap values to the required analog tap slicer settings
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param pre      TXFIR pre   tap value (0..10)  in 1/60 Vpp
 * @param main     TXFIR main  tap value (1..60)  in 1/60 Vpp
 * @param post1    TXFIR post1 tap value (0..24)  in 1/60 Vpp
 * @param post2    TXFIR post2 tap value (0..6)   in 1/60 Vpp
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
static err_code_t _txfir_tap_mapping(srds_access_t *sa__, int8_t pre, int8_t main, int8_t post1, int8_t post2);

/* Update Merlin16 TXFIR Slicer Structs */
static err_code_t _update_p1sr_reg_word(struct p1sr_reg_st *st);
static err_code_t _update_p2sr_reg_word(struct p2sr_reg_st *st);
static err_code_t _update_mssr_reg_word(struct mssr_reg_st *st);
static err_code_t _update_presr_reg_word(struct presr_reg_st *st);

/** Write to Analog Slicer Control Registers */ 
static err_code_t _write_txfir_slicer_controls(srds_access_t *sa__,
                                               struct p1sr_reg_st p1_st, struct p2sr_reg_st p2_st,
                                               struct mssr_reg_st ms_st, struct presr_reg_st pre_st);
#endif
