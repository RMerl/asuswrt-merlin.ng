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


#ifndef _XRDP_RNR_MEM_AG_H_
#define _XRDP_RNR_MEM_AG_H_

#include "ru_types.h"

#define RNR_MEM_HIGH_DATA_MEM_FIELD_MASK 0xFFFFFFFF
#define RNR_MEM_HIGH_DATA_MEM_FIELD_WIDTH 32
#define RNR_MEM_HIGH_DATA_MEM_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec RNR_MEM_HIGH_DATA_MEM_FIELD;
#endif
extern const ru_reg_rec RNR_MEM_HIGH_REG;
#define RNR_MEM_HIGH_REG_OFFSET 0x00000000
#define RNR_MEM_HIGH_REG_RAM_CNT 2048

#define RNR_MEM_LOW_DATA_MEM_FIELD_MASK 0xFFFFFFFF
#define RNR_MEM_LOW_DATA_MEM_FIELD_WIDTH 32
#define RNR_MEM_LOW_DATA_MEM_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec RNR_MEM_LOW_DATA_MEM_FIELD;
#endif
extern const ru_reg_rec RNR_MEM_LOW_REG;
#define RNR_MEM_LOW_REG_OFFSET 0x00000004
#define RNR_MEM_LOW_REG_RAM_CNT 2048

extern const ru_block_rec RNR_MEM_BLOCK;

#endif
