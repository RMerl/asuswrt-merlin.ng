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
#include "merlin16_shortfin_field_access.h"
#include "merlin16_shortfin_functions.h"
#include "merlin16_shortfin_internal.h"
#include "merlin16_shortfin_internal_error.h"

/* If SERDES_EVAL is defined, then is_ate_log_enabled() is queried to *\
\* know whether to log ATE.  merlin16_shortfin_access.h provides that function.  */

err_code_t _merlin16_shortfin_pmd_rdt_field(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, uint16_t *val_p) {

  EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr,val_p));
  *val_p = (uint16_t)(*val_p << shift_left);                   /* Move the MSB to the left most      [shift_left  = (15-msb)]     */
  *val_p = (uint16_t)(*val_p >> shift_right);                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return(ERR_CODE_NONE);
}

err_code_t _merlin16_shortfin_pmd_rdt_field_signed(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, int16_t *val_p) {

  EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr,(uint16_t *)val_p));
  *val_p = (int16_t)(*val_p << shift_left);                   /* Move the sign bit to the left most [shift_left  = (15-msb)]     */
  *val_p = (int16_t)(*val_p >> shift_right);                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return(ERR_CODE_NONE);
}

/*-------------------------------*/
/* Byte Write and Read Functions */
/*-------------------------------*/

err_code_t _merlin16_shortfin_pmd_mwr_reg_byte(srds_access_t *sa__, uint16_t addr, uint16_t mask, uint8_t lsb, uint8_t val) {


  EFUN(merlin16_shortfin_pmd_mwr_reg(sa__, addr, mask, lsb, (uint16_t) val));

  return(ERR_CODE_NONE);
}

err_code_t _merlin16_shortfin_pmd_rdt_field_byte(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, uint8_t *val8_p) {

  uint16_t val = 0;

  EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr,&val));

  val = (uint16_t)(val << shift_left);                   /* Move the MSB to the left most      [shift_left  = (15-msb)]     */
  val = (uint16_t)(val >> shift_right);                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  *val8_p = (uint8_t) val;

  return(ERR_CODE_NONE);
}

  err_code_t _merlin16_shortfin_pmd_rdt_field_signed_byte(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, int8_t *val8_p) {

  int16_t val = 0;

  EFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr,(uint16_t *) &val));

  val = (int16_t)(val << shift_left);                   /* Move the sign bit to the left most [shift_left  = (15-msb)]     */
  val = (int16_t)(val >> shift_right);                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  *val8_p = (int8_t) val;
  return(ERR_CODE_NONE);
}


/*-------------------------------*/
/* EVAL specific functions  */
/*-------------------------------*/

uint16_t _merlin16_shortfin_pmd_rde_reg(srds_access_t *sa__, uint16_t addr, err_code_t *err_code_p)
{
  uint16_t data = 0;

  EPFUN(merlin16_shortfin_pmd_rdt_reg(sa__, addr, &data));

    return data;
}

uint16_t _merlin16_shortfin_pmd_rde_field(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
  uint16_t data;

  EPSTM(data = _merlin16_shortfin_pmd_rde_reg(sa__, addr, err_code_p));

  data = (uint16_t)(data << shift_left);                 /* Move the sign bit to the left most [shift_left  = (15-msb)] */
  data = (uint16_t)(data >> shift_right);                /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return data;
}

int16_t _merlin16_shortfin_pmd_rde_field_signed(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
  int16_t  data;

  EPSTM(data = (int16_t) _merlin16_shortfin_pmd_rde_reg(sa__, addr, err_code_p));  /* convert it to signed */

  data = (int16_t)(data << shift_left);             /* Move the sign bit to the left most    [shift_left  = (15-msb)] */
  data = (int16_t)(data >> shift_right);            /* Move to the right with sign extension [shift_right = (15-msb+lsb)] */

  return data;
}

uint8_t _merlin16_shortfin_pmd_rde_field_byte(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p) {
    return ((uint8_t) _merlin16_shortfin_pmd_rde_field(sa__, addr, shift_left, shift_right, err_code_p));
}

int8_t _merlin16_shortfin_pmd_rde_field_signed_byte(srds_access_t *sa__, uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p) {
    return ((int8_t) _merlin16_shortfin_pmd_rde_field_signed(sa__, addr, shift_left, shift_right, err_code_p));
}
