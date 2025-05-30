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

/**************************************************************************************
 **************************************************************************************
 *  File Name     :  merlin16_shortfin_field_access.h                                        *
 *  Created On    :  29/04/2013                                                       *
 *  Created By    :  Kiran Divakar                                                    *
 *  Description   :  Serdes IP Register and Field access APIs                         *
 *  Revision      :   *
 *                                                                                    *
 **************************************************************************************
 **************************************************************************************/

/** @file merlin16_shortfin_field_access.h
 * Registers and field access
 */

#ifndef MERLIN16_SHORTFIN_API_FIELD_ACCESS_H
#define MERLIN16_SHORTFIN_API_FIELD_ACCESS_H

#include "merlin16_shortfin_ipconfig.h"
#include "common/srds_api_err_code.h"
#include "merlin16_shortfin_dependencies.h"

/* Extract a bitfield from a register value. */
#define extract_field(reg_value,msb,lsb) (((uint16_t)(reg_value) & ((1 << ((msb) + 1)) - 1)) >> (lsb))

/** Read a register field as an unsigned value.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param shift_left  Number of bits sign_bit should be moved to the left [shift_left  = (15 - msb)]
 * @param shift_right Number of right shifts to move field to bit0        [shift_right = (15 - msb + shift_right)]
 * @param *val_p 16-bit unsigned value read from the field
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t _merlin16_shortfin_pmd_rdt_field(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, uint16_t *val_p);

/** Read a register field as an signed value.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param shift_left  Number of bits sign_bit should be moved to the left [shift_left  = (15 - msb)]
 * @param shift_right Number of right shifts to move field to bit0        [shift_right = (15 - msb + lsb)]
 * @param *val_p 16-bit signed value read from the field
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t _merlin16_shortfin_pmd_rdt_field_signed(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, int16_t *val_p);


/*-------------------------------*/
/* Byte Write and Read Functions */
/*-------------------------------*/

/** Write a contiguous bit field in a register.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param mask 16-bit mask indicating the position of the field with bits of 1s
 * @param lsb  LSB of the field, the width of the field is implied by mask.
 * @param val  8bit value to write into the field.
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t _merlin16_shortfin_pmd_mwr_reg_byte(srds_access_t *sa__, uint16_t addr, uint16_t mask, uint8_t lsb, uint8_t val);

/** Read a register field as an 8-bit unsigned value.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param shift_left  Number of bits sign_bit should be moved to the left [shift_left  = (15 - msb)]
 * @param shift_right Number of right shifts to move field to bit0        [shift_right = (15 - msb + lsb)]
 * @param *val_p 8-bit unsigned value read from the field
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t _merlin16_shortfin_pmd_rdt_field_byte(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, uint8_t *val_p);

/** Read a register field as an 8-bit signed value.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param shift_left  Number of bits sign_bit should be moved to the left [shift_left  = (15 - msb)]
 * @param shift_right Number of right shifts to move field to bit0        [shift_right = (15 - msb + lsb)]
 * @param *val_p 8-bit signed value read from the field
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t _merlin16_shortfin_pmd_rdt_field_signed_byte(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, int8_t *val_p);

/** Read a register.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param err_code_p error code returned through pointer
 * @return 16-bit value read from the register
 */
uint16_t _merlin16_shortfin_pmd_rde_reg(srds_access_t *sa__, uint16_t addr, err_code_t *err_code_p);

/** Read a register field as an unsigned value.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param shift_left  Number of bits sign_bit should be moved to the left [shift_left  = (15 - msb)]
 * @param shift_right Number of right shifts to move field to bit0        [shift_right = (15 - msb + lsb)]
 * @param err_code_p error code returned through pointer
 * @return 16-bit unsigned value read from the field
 */
uint16_t _merlin16_shortfin_pmd_rde_field(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p);

/** Read a register field as a signed value.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param shift_left  Number of bits sign_bit should be moved to the left [shift_left  = (15 - msb)]
 * @param shift_right Number of right shifts with sign extension          [shift_right = (15 - msb + lsb)]
 * @param err_code_p error code returned through pointer
 * @return 16-bit signed value read from the field
 */
int16_t  _merlin16_shortfin_pmd_rde_field_signed(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p);

/** Read a bit field as an unsigned value.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param shift_left  Number of bits sign_bit should be moved to the left [shift_left  = (15 - msb)]
 * @param shift_right Number of right shifts to move field to bit0        [shift_right = (15 - msb + lsb)]
 * @param err_code_p error code returned through pointer
 * @return 8-bit unsigned value read from the field
 */
uint8_t _merlin16_shortfin_pmd_rde_field_byte(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p);

/** Read a bit field as a signed value.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param shift_left  Number of bits sign bit should be moved to the left [shift_left  = (15 - msb)]
 * @param shift_right Number of right shifts with sign extension          [shift_right = (15 - msb + lsb)]
 * @param err_code_p error code returned through pointer
 * @return 8-bit signed value read from the field
 */
int8_t  _merlin16_shortfin_pmd_rde_field_signed_byte(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p);

/*--------------------------------------------------*/
/* Serdes Access API Dependency "Wrapper" Functions */
/*--------------------------------------------------*/

/** Read a register as an unsigned value.
 * NOTE: This should be used instead of the API Dedendency function merlin16_shortfin_pmd_rdt_reg().
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address.
 * @param *valu16_p 16-bit unsigned value read from the register
 * @return Error Code generated by API (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shortfin_acc_rdt_reg(srds_access_t *sa__, uint16_t addr, uint16_t *valu16_p);

/** Write to a register from the currently selected Serdes IP Lane.
 * NOTE: This should be used instead of the API Dedendency function merlin16_shortfin_pmd_wr_reg().
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit register address of register to be written
 * @param valu16 16-bit unsigned value to be written to the register
 * @return Error code generated by write function (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shortfin_acc_wr_reg(srds_access_t *sa__, uint16_t addr, uint16_t valu16);

/** Masked Register Write to the currently selected Serdes IP core/lane.
 * NOTE: This should be used instead of the API Dedendency function merlin16_shortfin_pmd_mwr_reg().
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param addr 16-bit address of register to be written
 * @param mask 16-bit mask indicating the position of the field with bits of 1s
 * @param lsb  LSB of the field
 * @param valu16  16-bit value to be written
 * @return Error code generated by write function (returns ERR_CODE_NONE if no errors)
 */
err_code_t merlin16_shortfin_acc_mwr_reg(srds_access_t *sa__, uint16_t addr, uint16_t mask, uint8_t lsb, uint16_t valu16);

#endif /* FIELD_ACCESS_H_ */
