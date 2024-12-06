/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

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

/**
 * $Id: //xpon_co_tools/cur/hal_generator/libru/ru_types.h#11 $
 * $Date: 2020/03/30 $
 */

#ifndef _RU_TYPES_H_
#define _RU_TYPES_H_

/**
 * \brief Register tracking type definitions
 *
 * The register track module provides functionality to comprehensively debug
 * register level transactions.  It is a slow interface that should be used in
 * parallel to standard direct read/write transactions.  The module can parse
 * registers into easy read field format.  Registers may be looked up by name
 * or address.
 *
 */
#include "ru_config.h"

typedef enum
{
    ru_access_read      = 0x01,         /*< Read only */
    ru_access_write     = 0x02,         /*< Write only */
    ru_access_rw        = 0x03          /*< Read/write */
} ru_access;

/* Constants that indicate "this field/register isn't supported for this chip revision". */
#define RU_NO_FIELD 0xFFFFFFFF
#define RU_NO_REG 0xFFFFFFFF

typedef struct
{
    const char *name;                   /*< Name of field from reg spec */
#if RU_INCLUDE_DESC
    const char *title;                  /*< Short title of the field */
    const char *desc;                   /*< Detail description */
#endif
    uint32_t mask[RU_CHIP_REV_COUNT];   /*< Field bit mask (value can differ between chip revs) */
    uint32_t align;                     /*< Unknown, used by register macro */
    uint32_t bits[RU_CHIP_REV_COUNT];   /*< Field bit width (value can differ between chip revs) */
    uint32_t shift[RU_CHIP_REV_COUNT];  /*< Field bit offset (value can differ between chip revs) */
    uint32_t defval;                    /*< Field default value */
#if RU_INCLUDE_ACCESS
    ru_access access;                   /*< Field read/write access */
#endif
} ru_field_rec;                         /*< Field info record */

typedef struct
{
    const char *name;                   /*< Name of register from reg spec */
#if RU_INCLUDE_DESC
    const char *title;                  /*< Short title of the register */
    const char *desc;                   /*< Detail description */
#endif
    uint32_t addr[RU_CHIP_REV_COUNT];   /*< Block relative register address (value can differ between chip revs) */
    uint32_t ram_count;                 /*< RAM addresses, 0 for std register */
    uint32_t offset;                    /*< Offset of next index in RAM types */
    uint32_t log_idx;                   /*< Register ID for debug logging */
#if RU_INCLUDE_ACCESS
    ru_access access;                   /*< Register read/write access */
#endif
#if RU_INCLUDE_FIELD_DB
    uint32_t field_count;               /*< Number of fields, private */
    const ru_field_rec **fields;        /*< All fields for register, private */
#endif
} ru_reg_rec;                           /*< Register info record */

typedef struct
{
    const char *name;                   /*< Name of the block */
    unsigned long *addr;                /*< Block base addresses */
    uint32_t addr_count;                /*< Number of block instances */
    uint32_t reg_count;                 /*< Number of registers, private */
    const ru_reg_rec **regs;            /*< All registers for block, private */
} ru_block_rec;                         /*< Info for a block instance */

typedef enum
{
    ru_log_none     = 0x00,             /*< Do not log */
    ru_log_read     = 0x01,             /*< Log read access only */
    ru_log_write    = 0x02,             /*< Log write access only */
    ru_log_both     = 0x03              /*< Log both read and write */
} ru_log_type;                          /*< Access logging type */

typedef enum
{
    ru_op_equal,                        /*< (Register & Mask) == Value */
    ru_op_not_equal,                    /*< (Register & Mask) != Value */
    ru_op_greater_than,                 /*< (Register & Mask) > Value */
    ru_op_less_than                     /*< (Register & Mask) < Value */
} ru_log_op;                            /*< Logging comparator operation */

typedef struct
{
    const ru_block_rec *block;
    uint32_t      ind;
} ru_sorted_block;

typedef struct
{
    uint32_t      n;
    ru_sorted_block **blocks;
} ru_sorted_blocks;

#define RU_BLK(b) b##_BLOCK
#define RU_CHIP_BLK(b,c) b##_##c##_BLOCK
#define RU_REG(b,r) b##_##r##_REG
#define RU_CHIP_REG_OFFSET(b,c,r) b##_##c##_##r##_##c##_REG_OFFSET
#define RU_REG_RAM_CNT(b,r) b##_##r##_REG_RAM_CNT
#define RU_FLD(b,r,f) b##_##r##_##f##_FIELD

typedef uint8_t ru_block_inst;          /*< Multiple block instance index */
typedef uint32_t ru_ram_addr;           /*< Index for RAM mapped registers */

#endif /* End of file ru_types.h */
