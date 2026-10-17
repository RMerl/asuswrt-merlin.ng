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


#ifndef _XRDP_NATC_INDIR_AG_H_
#define _XRDP_NATC_INDIR_AG_H_

#include "ru_types.h"

#define NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_MASK 0x000003FF
#define NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_WIDTH 10
#define NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD;
#endif
#define NATC_INDIR_ADDR_REG_W_R_FIELD_MASK 0x00000400
#define NATC_INDIR_ADDR_REG_W_R_FIELD_WIDTH 1
#define NATC_INDIR_ADDR_REG_W_R_FIELD_SHIFT 10
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_INDIR_ADDR_REG_W_R_FIELD;
#endif
extern const ru_reg_rec NATC_INDIR_ADDR_REG_REG;
#define NATC_INDIR_ADDR_REG_REG_OFFSET 0x00000000

#define NATC_INDIR_DATA_REG_DATA_FIELD_MASK 0xFFFFFFFF
#define NATC_INDIR_DATA_REG_DATA_FIELD_WIDTH 32
#define NATC_INDIR_DATA_REG_DATA_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_INDIR_DATA_REG_DATA_FIELD;
#endif
extern const ru_reg_rec NATC_INDIR_DATA_REG_REG;
#define NATC_INDIR_DATA_REG_REG_OFFSET 0x00000010
#define NATC_INDIR_DATA_REG_REG_RAM_CNT 41

#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_MASK 0x0000007F
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_WIDTH 7
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD;
#endif
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_MASK 0x00000400
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_WIDTH 1
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_SHIFT 10
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD;
#endif
extern const ru_reg_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_REG;
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_REG_OFFSET 0x000000C0

#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_MASK 0xFFFFFFFF
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_WIDTH 32
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD;
#endif
extern const ru_reg_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_REG;
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_REG_OFFSET 0x000000D0
#define NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_REG_RAM_CNT 2

extern const ru_block_rec NATC_INDIR_BLOCK;

#endif
