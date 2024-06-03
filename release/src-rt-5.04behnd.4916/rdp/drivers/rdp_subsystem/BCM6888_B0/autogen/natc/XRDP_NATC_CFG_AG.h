/*
   Copyright (c) 2015 Broadcom
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


#ifndef _XRDP_NATC_CFG_AG_H_
#define _XRDP_NATC_CFG_AG_H_

#include "ru_types.h"

#define NATC_KEY_MASK_KEY_MASK_FIELD_MASK 0xFFFFFFFF
#define NATC_KEY_MASK_KEY_MASK_FIELD_WIDTH 32
#define NATC_KEY_MASK_KEY_MASK_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_KEY_MASK_KEY_MASK_FIELD;
#endif
extern const ru_reg_rec NATC_KEY_MASK_REG;
#define NATC_KEY_MASK_REG_OFFSET 0x000003B0
#define NATC_KEY_MASK_REG_RAM_CNT 8

extern const ru_block_rec NATC_KEY_BLOCK;

#define NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK 0x00000007
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH 3
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_ZEROS_FIELD;
#endif
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_MASK 0xFFFFFFF8
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH 29
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT 3
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_BAR_FIELD;
#endif
extern const ru_reg_rec NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_REG;
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET 0x000002D0

#define NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_MASK 0x000000FF
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH 8
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_BAR_FIELD;
#endif
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK 0xFFFFFF00
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH 24
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT 8
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_ZEROS_FIELD;
#endif
extern const ru_reg_rec NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_REG;
#define NATC_TBL_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET 0x000002D4

#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_MASK 0x00000007
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_WIDTH 3
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_ZEROS_FIELD;
#endif
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_MASK 0xFFFFFFF8
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_WIDTH 29
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD_SHIFT 3
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_BAR_FIELD;
#endif
extern const ru_reg_rec NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_REG;
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET 0x000002D8

#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_MASK 0x000000FF
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_WIDTH 8
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_BAR_FIELD;
#endif
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_MASK 0xFFFFFF00
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_WIDTH 24
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD_SHIFT 8
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_ZEROS_FIELD;
#endif
extern const ru_reg_rec NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_REG;
#define NATC_TBL_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET 0x000002DC

extern const ru_block_rec NATC_TBL_BLOCK;

#endif
