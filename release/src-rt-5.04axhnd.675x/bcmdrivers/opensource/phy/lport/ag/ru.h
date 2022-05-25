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
#ifndef _RU_H_
#define _RU_H_
/**
 * \brief Register tracking utilities
 *
 * The register track module provides functionality to comprehensively debug
 * register level transactions.  It is a slow interface that should be used in
 * parallel to standard direct read/write transactions.  The module can parse
 * registers into easy read field format.  Registers may be looked up by name
 * or address.
 *
 */
#include "ru_config.h"
#include "ru_types.h"
#include "ru_chip.h"

/**
 * \brief Write a register
 *
 * This function writes a value to a register.  If debugging is enabled the
 * written value may be logged.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param val Value to write
 *
 * \return
 * 0 if successful
 */
#define RU_REG_WRITE(i,b,r,v) WRITE_32(RU_BLK(b).addr[i]+RU_REG_OFFSET(b,r),v)


/**
 * \brief Read a register
 *
 * This function reads a register.  If debugging is enabled the value read may
 * be logged.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param val Read back register value
 *
 * \return
 * 0 if successful
 */
extern
uint32_t ru_reg_read(ru_block_inst blk_inst,
                const ru_block_rec *blk,
                const ru_reg_rec *reg);
#define RU_REG_READ(i,b,r,reg) READ_32(RU_BLK(b).addr[i]+RU_REG_OFFSET(b,r),reg)


/**
 * \brief Set a register field
 *
 * This function sets a field in a register.  The value if applied to the
 * supplied old register value and returned.  If debbuging is enabled an
 * assertion will be raised if the field is out of range with range being
 * determined by the width of the field. If the field is out of range it will
 * not be set and the original old value will be returned.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param fld Register field record
 * \param reg_val Old register value to write in to
 * \param fld_val Field value to write
 *
 * \return
 * val with new field value applied
 */
extern
uint32_t ru_field_set(ru_block_inst blk_inst,
                      const ru_block_rec *blk,
                      const ru_reg_rec *reg,
                      const ru_field_rec *fld,
                      uint32_t reg_val,
                      uint32_t fld_val);
#define RU_FIELD_SET(i,b,r,f,rv,fv)                                         \
    (((rv) & ~RU_FLD_MASK(b,r,f)) | (fv << RU_FLD_SHIFT(b,r,f)))


/**
 * \brief Get a register field
 *
 * This function gets a field in a register.  The value returned is justified to
 * bit [0]. This function does not support any debug features.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param fld Register field record
 * \param reg_val Old register value to read from
 *
 * \return
 * Bit [0] justified field value
 */
#define RU_FIELD_GET(i,b,r,f,rv)                                            \
    (((rv) & RU_FLD_MASK(b,r,f)) >> RU_FLD_SHIFT(b,r,f))


#endif /* End of file _RU_H_ */
