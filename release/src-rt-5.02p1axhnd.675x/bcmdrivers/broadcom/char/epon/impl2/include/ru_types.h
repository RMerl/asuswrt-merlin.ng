/*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>*/
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
 #ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
#include "ru_config.h"


typedef enum
{
    ru_access_read      = 0x01,         /*< Read only */
    ru_access_write     = 0x02,         /*< Write only */
    ru_access_rw        = 0x03          /*< Read/write */
} ru_access;

typedef enum
{
	ru_reg_size_8,
	ru_reg_size_16,
	ru_reg_size_32,
	ru_reg_size_64
}ru_reg_size;

typedef struct
{
    const char *name;                   /*< Name of field from reg spec */
#if RU_INCLUDE_DESC
    const char *title;                  /*< Short title of the field */
    const char *desc;                   /*< Detail description */
#endif
    uint32_t mask;                      /*< Field bit mask */
    uint32_t align;                     /*< Unknown, used by register macro */
    uint32_t bits;                      /*< Field bit width */
    uint32_t shift;                     /*< Field bit offset */
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
    unsigned long addr;                      /*< Block relative register address */
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
    ru_reg_size	reg_size;
} ru_reg_rec;                           /*< Register info record */

typedef struct
{
    const char *name;                   /*< Name of the block */
    unsigned long *addr;                     /*< Block base addresses */
    uint8_t addr_count;                 /*< Number of block instances */
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

#define RU_BLK(b) b##_BLOCK
#define RU_REG(b,r) b##_##r##_REG
#define RU_REG_OFFSET(b,r) b##_##r##_REG_OFFSET
#define RU_REG_RAM_CNT(b,r) b##_##r##_REG_RAM_CNT
#define RU_FLD(b,r,f) b##_##r##_##f##_FIELD
#define RU_FLD_MASK(b,r,f) b##_##r##_##f##_FIELD_MASK
#define RU_FLD_SHIFT(b,r,f) b##_##r##_##f##_FIELD_SHIFT

typedef uint8_t ru_block_inst;          /*< Multiple block instance index */
typedef uint32_t ru_ram_addr;           /*< Index for RAM mapped registers */

#endif /* End of file _RU_TYPES_H_ */
