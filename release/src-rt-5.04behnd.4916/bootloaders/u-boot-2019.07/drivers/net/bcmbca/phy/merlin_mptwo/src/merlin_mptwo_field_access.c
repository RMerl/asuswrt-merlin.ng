// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/** @file merlin_mptwo_field_access.c
 * Registers and field access
 */

#include "merlin_mptwo_field_access.h"
#include "merlin_mptwo_dependencies.h"
#include "merlin_mptwo_ipconfig.h"
#include "merlin_mptwo_internal.h"

err_code_t _merlin_mptwo_pmd_rdt_field(uint16_t addr, uint8_t shift_left, uint8_t shift_right, uint16_t *val_p) {
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%04x : MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif

  EFUN(merlin_mptwo_pmd_rdt_reg(addr,val_p));
  *val_p <<= shift_left;                   /* Move the MSB to the left most      [shift_left  = (15-msb)]     */
  *val_p >>= shift_right;                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return(ERR_CODE_NONE);
}

err_code_t _merlin_mptwo_pmd_rdt_field_signed(uint16_t addr, uint8_t shift_left, uint8_t shift_right, int16_t *val_p) {
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%04x : Signed MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif

  EFUN(merlin_mptwo_pmd_rdt_reg(addr,(uint16_t *)val_p));
  *val_p <<= shift_left;                   /* Move the sign bit to the left most [shift_left  = (15-msb)]     */
  *val_p >>= shift_right;                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return(ERR_CODE_NONE);
}

/*-------------------------------*/
/* Byte Write and Read Functions */
/*-------------------------------*/

err_code_t _merlin_mptwo_pmd_mwr_reg_byte(uint16_t addr, uint16_t mask, uint8_t lsb, uint8_t val) {

  EFUN(merlin_mptwo_pmd_mwr_reg(addr, mask, lsb, (uint16_t) val));

  return(ERR_CODE_NONE);
}

  err_code_t _merlin_mptwo_pmd_rdt_field_byte(uint16_t addr, uint8_t shift_left, uint8_t shift_right, uint8_t *val8_p) {

  uint16_t val;
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%04x : MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif

  EFUN(merlin_mptwo_pmd_rdt_reg(addr,&val));
  val <<= shift_left;                   /* Move the MSB to the left most      [shift_left  = (15-msb)]     */
  val >>= shift_right;                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  *val8_p = (uint8_t) val;

  return(ERR_CODE_NONE);
}

err_code_t _merlin_mptwo_pmd_rdt_field_signed_byte(uint16_t addr, uint8_t shift_left, uint8_t shift_right, int8_t *val8_p) {

  int16_t val;
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%04x : Signed MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif

  EFUN(merlin_mptwo_pmd_rdt_reg(addr,(uint16_t *) &val));
  val <<= shift_left;                   /* Move the sign bit to the left most [shift_left  = (15-msb)]     */
  val >>= shift_right;                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  *val8_p = (int8_t) val;

  return(ERR_CODE_NONE);
}

/*-------------------------------*/
/* MDIO specific Misc functions  */
/*-------------------------------*/



/*--------------------------*/
/* EVAL specific functions  */
/*--------------------------*/

uint16_t _merlin_mptwo_pmd_rde_reg(uint16_t addr, err_code_t *err_code_p)
{
  uint16_t data;
  EPFUN(merlin_mptwo_pmd_rdt_reg(addr, &data));
  return data;
}

uint16_t _merlin_mptwo_pmd_rde_field(uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
  uint16_t data;
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%04x : MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif
  EPSTM(data = _merlin_mptwo_pmd_rde_reg(addr, err_code_p));

  data <<= shift_left;                 /* Move the sign bit to the left most [shift_left  = (15-msb)] */
  data >>= shift_right;                /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return data;
}

int16_t _merlin_mptwo_pmd_rde_field_signed(uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
  int16_t  data;
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%04x : Signed MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif
  EPSTM(data = (int16_t) _merlin_mptwo_pmd_rde_reg(addr, err_code_p));  /* convert it to signed */

  data <<= shift_left;             /* Move the sign bit to the left most    [shift_left  = (15-msb)] */
  data >>= shift_right;            /* Move to the right with sign extension [shift_right = (15-msb+lsb)] */

  return data;
}

uint8_t _merlin_mptwo_pmd_rde_field_byte(uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p) {
   return ((uint8_t) _merlin_mptwo_pmd_rde_field(addr, shift_left, shift_right, err_code_p));
}

int8_t _merlin_mptwo_pmd_rde_field_signed_byte(uint16_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p) {
  return ((int8_t) _merlin_mptwo_pmd_rde_field_signed(addr, shift_left, shift_right, err_code_p));
}



/*--------------------------*/
/* 32-bit-address Functions */
/*--------------------------*/

err_code_t _merlin_mptwo_pmd_rdt_field_addr32(uint32_t addr, uint8_t shift_left, uint8_t shift_right, uint16_t *val_p) {
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%08lx : MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif

  EFUN(merlin_mptwo_pmd_rdt_reg_addr32(addr,val_p));
  *val_p <<= shift_left;                   /* Move the MSB to the left most      [shift_left  = (15-msb)]     */
  *val_p >>= shift_right;                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return(ERR_CODE_NONE);
}

err_code_t _merlin_mptwo_pmd_rdt_field_signed_addr32(uint32_t addr, uint8_t shift_left, uint8_t shift_right, int16_t *val_p) {
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%08lx : Signed MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif

  EFUN(merlin_mptwo_pmd_rdt_reg_addr32(addr,(uint16_t *)val_p));
  *val_p <<= shift_left;                   /* Move the sign bit to the left most [shift_left  = (15-msb)]     */
  *val_p >>= shift_right;                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return(ERR_CODE_NONE);
}

/*-------------------------------*/
/* Byte Write and Read Functions */
/*-------------------------------*/

err_code_t _merlin_mptwo_pmd_mwr_reg_byte_addr32(uint32_t addr, uint16_t mask, uint8_t lsb, uint8_t val) {

  EFUN(merlin_mptwo_pmd_mwr_reg_addr32(addr, mask, lsb, (uint16_t) val));

  return(ERR_CODE_NONE);
}

err_code_t _merlin_mptwo_pmd_rdt_field_byte_addr32(uint32_t addr, uint8_t shift_left, uint8_t shift_right, uint8_t *val8_p) {

  uint16_t val;
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%08lx : MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif

  EFUN(merlin_mptwo_pmd_rdt_reg_addr32(addr,&val));
  val <<= shift_left;                   /* Move the MSB to the left most      [shift_left  = (15-msb)]     */
  val >>= shift_right;                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  *val8_p = (uint8_t) val;

  return(ERR_CODE_NONE);
}

err_code_t _merlin_mptwo_pmd_rdt_field_signed_byte_addr32(uint32_t addr, uint8_t shift_left, uint8_t shift_right, int8_t *val8_p) {

  int16_t val;
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%08lx : Signed MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif

  EFUN(merlin_mptwo_pmd_rdt_reg_addr32(addr,(uint16_t *) &val));
  val <<= shift_left;                   /* Move the sign bit to the left most [shift_left  = (15-msb)]     */
  val >>= shift_right;                  /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  *val8_p = (int8_t) val;

  return(ERR_CODE_NONE);
}

/*-------------------------*/
/* EVAL specific functions */
/*-------------------------*/

uint16_t _merlin_mptwo_pmd_rde_reg_addr32(uint32_t addr, err_code_t *err_code_p)
{
  uint16_t data;
  EPFUN(merlin_mptwo_pmd_rdt_reg_addr32(addr, &data));
  return data;
}

uint16_t _merlin_mptwo_pmd_rde_field_addr32(uint32_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
  uint16_t data;
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%08lx : MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif
  EPSTM(data = _merlin_mptwo_pmd_rde_reg_addr32(addr, err_code_p));

  data <<= shift_left;                 /* Move the sign bit to the left most [shift_left  = (15-msb)] */
  data >>= shift_right;                /* Right shift entire field to bit 0  [shift_right = (15-msb+lsb)] */

  return data;
}

int16_t _merlin_mptwo_pmd_rde_field_signed_addr32(uint32_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p)
{
  int16_t  data;
#ifdef ATE_LOG
    EFUN_PRINTF(("// ATE_LOG Addr x%08lx : Signed MASK x%04x : Right Shift %d\n",addr,(((uint16_t)(0xFFFF<<shift_left))>>shift_right)<<(shift_right-shift_left),shift_right-shift_left));
#endif
  EPSTM(data = (int16_t) _merlin_mptwo_pmd_rde_reg_addr32(addr, err_code_p));  /* convert it to signed */

  data <<= shift_left;             /* Move the sign bit to the left most    [shift_left  = (15-msb)] */
  data >>= shift_right;            /* Move to the right with sign extension [shift_right = (15-msb+lsb)] */

  return data;
}

uint8_t _merlin_mptwo_pmd_rde_field_byte_addr32(uint32_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p) {
   return ((uint8_t) _merlin_mptwo_pmd_rde_field_addr32(addr, shift_left, shift_right, err_code_p));
}

int8_t _merlin_mptwo_pmd_rde_field_signed_byte_addr32(uint32_t addr, uint8_t shift_left, uint8_t shift_right, err_code_t *err_code_p) {
  return ((int8_t) _merlin_mptwo_pmd_rde_field_signed_addr32(addr, shift_left, shift_right, err_code_p));
}
