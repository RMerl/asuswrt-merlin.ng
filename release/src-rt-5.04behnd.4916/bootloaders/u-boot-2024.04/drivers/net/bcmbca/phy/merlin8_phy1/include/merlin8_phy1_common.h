// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/********************************************************************************
 ********************************************************************************
 *                                                                              *
 *  Revision      :   *
 *                                                                              *
 *  Description   :  Defines and Enumerations required by Serdes APIs           *
 *                                                                              *
 ********************************************************************************
 ********************************************************************************/

/** @file merlin8_phy1_common.h
 * Defines and Enumerations shared across MER7/BHK7/OSP7/PER5 APIs but NOT uCode
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef MERLIN8_PHY1_API_COMMON_H
#define MERLIN8_PHY1_API_COMMON_H

#include "merlin8_phy1_ipconfig.h"
#include "merlin8_phy1_usr_includes.h"

/** Macro to determine sign of a value */
#define sign(x) ((x>=0) ? 1 : -1)

#define SRDS_MAX_BER_STR_LEN        (32)
#define SRDS_MAX_OSR_STR_LEN        (16)


/*
 * IP-Specific Iteration Bounds
 */
# define NUM_PLLS (1)


/*------------------------------*/
/** Serdes OSR Mode Structure   */
/*------------------------------*/
typedef struct {
  /** TX OSR Mode */
  uint8_t tx;
  /** RX OSR Mode */
  uint8_t rx;
  /** OSR Mode for TX and RX (used when both TX and RX should have same OSR Mode) */
  uint8_t tx_rx;
} merlin8_phy1_osr_mode_st;

#define srds_Y_width (6)
#define srds_X_width (3)
/*! Debug feedback status
 *
 */
typedef struct merlin8_phy1_dbgfb_stats_s {
    int32_t  data1[srds_X_width][srds_Y_width];
    int32_t  data2[srds_X_width][srds_Y_width];
    int32_t  data3[srds_X_width][srds_Y_width];
    int32_t  data4[srds_X_width][srds_Y_width];
} merlin8_phy1_dbgfb_data_st;


/*! Debug feedback data
 *
 */
typedef struct merlin8_phy1_dbgfb_data_s {
    uint32_t time_in_us;
    merlin8_phy1_dbgfb_data_st data;
    uint8_t pam4es_en;
    uint8_t temp[3];
} merlin8_phy1_dbgfb_stats_st;



#endif
#ifdef __cplusplus
}
#endif
