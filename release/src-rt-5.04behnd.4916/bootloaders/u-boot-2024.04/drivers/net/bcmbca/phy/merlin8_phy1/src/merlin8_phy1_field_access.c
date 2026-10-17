// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/**************************************************************************************
 **************************************************************************************
 *  File Name     :  merlin8_phy1_field_access.c                                        *
 *  Created On    :  29/04/2013                                                       *
 *  Created By    :  Kiran Divakar                                                    *
 *  Description   :  APIs to access Serdes IP Registers and Reg fields                *
 *  Revision      :                                                               *
 **************************************************************************************
 **************************************************************************************/


#include "merlin8_phy1_field_access.h"
#include "merlin8_phy1_functions.h"
#include "merlin8_phy1_internal.h"
#include "merlin8_phy1_internal_error.h"
#include "merlin8_phy1_dependencies.h"


/** @file
 *
 */

/* Function prototype for local function merlin8_phy1_INTERNAL_print_err_msg_dependency().
 *   Print errors for calls to API Dedendency functions which return error codes other then ERR_CODE_NONE.
 *   Always returns known API Serdes Access violation error code ERR_CODE_SRDS_REG_ACCESS_FAIL.
 */
err_code_t merlin8_phy1_INTERNAL_print_err_msg_dependency(srds_access_t *sa__, int err_code, const char *filename, const char *func_name,
                                                    const char *dep_func_name, uint32_t addr32, uint32_t valu32);


/*-------------------------------*/
/* Byte Write and Read Functions */
/*-------------------------------*/

err_code_t merlin8_phy1_acc_mwr_reg_u8(srds_access_t *sa__, uint16_t addr, uint16_t mask, uint8_t lsb, uint8_t valu8)
{
    return merlin8_phy1_acc_mwr_reg(sa__, addr, mask, lsb, (uint16_t) valu8);
}

#ifdef PCIEG3_V1
/* Read modify write field update, required for "ex" register access */
err_code_t merlin8_phy1_acc_rmw_reg_u8(srds_access_t *sa__, uint16_t addr, uint16_t mask, uint8_t lsb, uint8_t valu8)
{
    return merlin8_phy1_acc_rmw_reg(sa__, addr, mask, lsb, (uint16_t) valu8);
}
#endif

uint16_t merlin8_phy1_acc_rde_reg(srds_access_t *sa__, uint16_t addr, err_code_t *err_code_p)
{
    uint16_t valu16 = 0;

        if (ERR_CODE_NONE != (*err_code_p = merlin8_phy1_pmd_rdt_reg(sa__, addr, &valu16))) {
            *err_code_p = merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, *err_code_p, __FILE__, API_FUNCTION_NAME, "_pmd_rdt_reg", addr, valu16);
        }
    return valu16;
}

uint16_t merlin8_phy1_acc_rde_field_u16(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
    uint16_t valu16 = 0;
        *err_code_p = merlin8_phy1_pmd_rdt_reg(sa__, addr, &valu16);
        if (ERR_CODE_NONE != *err_code_p) {
            *err_code_p = merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, *err_code_p, __FILE__, API_FUNCTION_NAME, "_pmd_rdt_reg", addr, valu16);
        }
    valu16 = (uint16_t)(valu16 << shift_left);          /* Move the sign bit to the left most [shift_left  = (15-msb)] */
    valu16 = (uint16_t)(valu16 >> shift_right);         /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */
    return valu16;
}

int16_t merlin8_phy1_acc_rde_field_s16(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
    int16_t vals16 = 0;
        *err_code_p = merlin8_phy1_pmd_rdt_reg(sa__, addr, (uint16_t *) &vals16);
    if (ERR_CODE_NONE != *err_code_p) {
        *err_code_p = merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, *err_code_p, __FILE__, API_FUNCTION_NAME, "_pmd_rdt_reg", addr, (uint32_t)vals16);
    }
    vals16 = (int16_t)(vals16 << shift_left);               /* Move the sign bit to the left most    [shift_left  = (15-msb)] */
    vals16 = (int16_t)(vals16 >> shift_right);              /* Move to the right with sign extension [shift_right = (15-msb+lsb)] */
    return vals16;
}

uint8_t merlin8_phy1_acc_rde_field_u8(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
    return ((uint8_t) merlin8_phy1_acc_rde_field_u16(sa__, addr, shift_left, shift_right, err_code_p));
}

int8_t merlin8_phy1_acc_rde_field_s8(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
    return ((int8_t) merlin8_phy1_acc_rde_field_s16(sa__, addr, shift_left, shift_right, err_code_p));
}


/*------------------------------------------------------------------------------------------------*/
/* New "Serdes ACCess" (_acc_) 'base' API dependency wrapper functions.                           */
/* These trap unknown error codes from Dependency functions, and return a known API error code.   */
/* Implemented to correct recursive error handling where global API error values are not allowed. */
/*------------------------------------------------------------------------------------------------*/

err_code_t merlin8_phy1_acc_rdt_reg(srds_access_t *sa__, uint16_t addr, uint16_t *valu16_p)
{
    err_code_t err_code;
        if (ERR_CODE_NONE != (err_code = merlin8_phy1_pmd_rdt_reg(sa__, addr, valu16_p))) {
            return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_pmd_rdt_reg", addr, *valu16_p);
        }
    return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_acc_wr_reg(srds_access_t *sa__, uint16_t addr, uint16_t valu16)
{
    err_code_t err_code;
    if (ERR_CODE_NONE != (err_code = merlin8_phy1_pmd_wr_reg(sa__, addr, valu16))) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_pmd_wr_reg", addr, valu16);
    } else
        return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_acc_mwr_reg(srds_access_t *sa__, uint16_t addr, uint16_t mask, uint8_t lsb, uint16_t valu16)
{
    err_code_t err_code;
    {
      if (ERR_CODE_NONE != (err_code = merlin8_phy1_pmd_mwr_reg(sa__, addr, mask, lsb, valu16))) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_pmd_mwr_reg", addr, valu16);
      } else
        return (ERR_CODE_NONE);
    }
}

#ifdef PCIEG3_V1
/* Read modify write field update, required for "ex" register access */
err_code_t merlin8_phy1_acc_rmw_reg(srds_access_t *sa__, uint16_t addr, uint16_t mask, uint8_t lsb, uint16_t valu16)
{
    err_code_t err_code;
    {

      uint16_t reg_val=0;

      if (ERR_CODE_NONE != (err_code = merlin8_phy1_pmd_rdt_reg(sa__, addr, &reg_val)))
      {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_pmd_rmw_reg", addr, reg_val);
      }

      /* mask is write enable bits */
      valu16 = (uint16_t) ((reg_val & ~mask) | (valu16 << lsb));

      if (ERR_CODE_NONE != (err_code = merlin8_phy1_acc_wr_reg(sa__, addr, valu16)))
      {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_pmd_rmw_reg", addr, valu16);
      } else
        return (ERR_CODE_NONE);
    }
}
#endif



/*---------------------------------------------------------------------------------------------------*/
/* Additional "Serdes ACCess" (_acc_) API dependency wrapper functions for merlin8_phy1_dependencies.c */
/* These trap unknown error codes from Dependency functions, and return a known API error code.      */
/* If the function does not return an error code, the wrapper is here for naming consistency.        */
/*---------------------------------------------------------------------------------------------------*/

err_code_t merlin8_phy1_acc_wr_pram(srds_access_t *sa__, int wrCmd, uint32_t val)
{
    err_code_t err_code;
    if (ERR_CODE_NONE != (err_code = merlin8_phy1_pmd_wr_pram(sa__, wrCmd, val))) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_pmd_wr_pram", 0, val);
    } else
        return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_acc_delay_ns(srds_access_t *sa__, uint16_t delay_ns)
{
    err_code_t err_code = merlin8_phy1_delay_ns(sa__, delay_ns);

    if (ERR_CODE_NONE != err_code) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_delay_ns", 0, delay_ns);
    } else
        return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_acc_delay_us(srds_access_t *sa__, uint32_t delay_us)
{
    err_code_t err_code = merlin8_phy1_delay_us(sa__, delay_us);

    if (ERR_CODE_NONE != err_code) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_delay_us", 0, delay_us);
    } else
        return (ERR_CODE_NONE);
}

err_code_t merlin8_phy1_acc_delay_ms(srds_access_t *sa__, uint32_t delay_ms)
{
    err_code_t err_code = merlin8_phy1_delay_ms(sa__, delay_ms);

    if (ERR_CODE_NONE != err_code) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_delay_ms", 0, delay_ms);
    } else
        return (ERR_CODE_NONE);
}


uint8_t merlin8_phy1_acc_get_core(srds_access_t *sa__)
{
    return merlin8_phy1_get_core(sa__);
}

uint8_t merlin8_phy1_acc_get_lane(srds_access_t *sa__)
{
    const uint8_t num_lanes = 1;
    uint8_t lane = merlin8_phy1_get_lane(sa__);
    if(lane >= num_lanes) {
        USR_PRINTF(("ERROR: merlin8_phy1_get_lane() returned a lane index of %d. Expected range is 0-%d\n", lane, num_lanes-1));
    }
    return lane;
}

    /* merlin8_phy1_acc_get_physical_lane() is defined to be merlin8_phy1_get_lane() */

err_code_t merlin8_phy1_acc_set_lane(srds_access_t *sa__, uint8_t lane_index)
{
    err_code_t err_code = merlin8_phy1_set_lane(sa__, lane_index);

    if (ERR_CODE_NONE != err_code) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_set_lane", 0, lane_index);
    } else
        return (ERR_CODE_NONE);
}

    /* merlin8_phy1_acc_set_physical_lane() is macro defined to be merlin8_phy1_set_lane() */

uint8_t merlin8_phy1_acc_get_pll_idx(srds_access_t *sa__)
{
    return merlin8_phy1_get_pll_idx(sa__);
}

err_code_t merlin8_phy1_acc_set_pll_idx(srds_access_t *sa__, uint8_t pll_index)
{
    err_code_t err_code = merlin8_phy1_set_pll_idx(sa__, pll_index);

    if (ERR_CODE_NONE != err_code) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_set_pll_idx", 0, pll_index);
    } else
        return (ERR_CODE_NONE);
}

uint8_t merlin8_phy1_acc_get_micro_idx(srds_access_t *sa__)
{
    return merlin8_phy1_get_micro_idx(sa__);
}

err_code_t merlin8_phy1_acc_set_micro_idx(srds_access_t *sa__, uint8_t micro_index)
{
    err_code_t err_code = merlin8_phy1_set_micro_idx(sa__, micro_index);

    if (ERR_CODE_NONE != err_code) {
        return merlin8_phy1_INTERNAL_print_err_msg_dependency(sa__, err_code, __FILE__, API_FUNCTION_NAME, "_set_micro_idx", 0, micro_index);
    } else
        return (ERR_CODE_NONE);
}

#if defined(SERDES_EXTERNAL_INFO_TABLE_EN)
srds_info_t *merlin8_phy1_acc_get_info_table_address(srds_access_t *sa__)
{
    return merlin8_phy1_get_info_table_address(sa__);
}
#endif


/*
 * merlin8_phy1_INTERNAL_print_err_msg_dependency() -
 *   Print errors for calls to API Dedendency functions which return error codes other then ERR_CODE_NONE.
 *   Always returns known API Serdes Access violation error code ERR_CODE_SRDS_REG_ACCESS_FAIL.
 */
err_code_t merlin8_phy1_INTERNAL_print_err_msg_dependency(srds_access_t *sa__, int int_err_code, const char *filename, const char *func_name,
                                                    const char *dep_func_name, uint32_t addr, uint32_t valu16)
{
    USR_PRINTF(("ERROR:%s->%s(): API Dependency .. %s() returned error code %d {Addr:0x%X, Data:0x%X}\n",
                filename, func_name, dep_func_name, int_err_code, addr, valu16));
    return (ERR_CODE_SRDS_REG_ACCESS_FAIL);
}
