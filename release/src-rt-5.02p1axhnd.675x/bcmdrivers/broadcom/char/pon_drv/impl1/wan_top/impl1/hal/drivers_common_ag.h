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

#ifndef _BCM6858_DRIVERS_COMMON_AG_H_
#define _BCM6858_DRIVERS_COMMON_AG_H_


#include <bdmf_data_types.h>
#include "bdmf_shell.h"
#include "bdmf_interface.h"
#include <bdmf_session.h>
#include "ru.h"
#include "WAN_TOP_AG.h"


#define _1BITS_MAX_VAL_ (1U<<1)
#define _2BITS_MAX_VAL_ (1U<<2)
#define _3BITS_MAX_VAL_ (1U<<3)
#define _4BITS_MAX_VAL_ (1U<<4)
#define _5BITS_MAX_VAL_ (1U<<5)
#define _6BITS_MAX_VAL_ (1U<<6)
#define _7BITS_MAX_VAL_ (1U<<7)
#define _9BITS_MAX_VAL_ (1U<<9)
#define _10BITS_MAX_VAL_ (1U<<10)
#define _11BITS_MAX_VAL_ (1U<<11)
#define _12BITS_MAX_VAL_ (1U<<12)
#define _13BITS_MAX_VAL_ (1U<<13)
#define _14BITS_MAX_VAL_ (1U<<14)
#define _15BITS_MAX_VAL_ (1U<<15)
#define _17BITS_MAX_VAL_ (1U<<17)
#define _18BITS_MAX_VAL_ (1U<<18)
#define _19BITS_MAX_VAL_ (1U<<19)
#define _20BITS_MAX_VAL_ (1U<<20)
#define _21BITS_MAX_VAL_ (1U<<21)
#define _22BITS_MAX_VAL_ (1U<<22)
#define _23BITS_MAX_VAL_ (1U<<23)
#define _24BITS_MAX_VAL_ (1U<<24)
#define _25BITS_MAX_VAL_ (1U<<25)
#define _26BITS_MAX_VAL_ (1U<<26)
#define _27BITS_MAX_VAL_ (1U<<27)
#define _28BITS_MAX_VAL_ (1U<<28)
#define _29BITS_MAX_VAL_ (1U<<29)
#define _30BITS_MAX_VAL_ (1U<<30)
#define _31BITS_MAX_VAL_ (1U<<31)
#define _32BITS_MAX_VAL_ (1U<<32)
#define _33BITS_MAX_VAL_ (1U<<33)
#define _34BITS_MAX_VAL_ (1U<<34)
#define _35BITS_MAX_VAL_ (1U<<35)
#define _36BITS_MAX_VAL_ (1U<<36)
#define _37BITS_MAX_VAL_ (1U<<37)
#define _38BITS_MAX_VAL_ (1U<<38)
#define _39BITS_MAX_VAL_ (1U<<39)
#define _40BITS_MAX_VAL_ (1U<<40)
#define _41BITS_MAX_VAL_ (1U<<41)
#define _42BITS_MAX_VAL_ (1U<<42)
#define _43BITS_MAX_VAL_ (1U<<43)
#define _44BITS_MAX_VAL_ (1U<<44)
#define _45BITS_MAX_VAL_ (1U<<45)
#define _46BITS_MAX_VAL_ (1U<<46)
#define _47BITS_MAX_VAL_ (1U<<47)
#define _48BITS_MAX_VAL_ (1U<<48)
#define _49BITS_MAX_VAL_ (1U<<49)
#define _50BITS_MAX_VAL_ (1U<<50)
#define _51BITS_MAX_VAL_ (1U<<51)
#define _52BITS_MAX_VAL_ (1U<<52)
#define _53BITS_MAX_VAL_ (1U<<53)
#define _54BITS_MAX_VAL_ (1U<<54)
#define _55BITS_MAX_VAL_ (1U<<55)
#define _56BITS_MAX_VAL_ (1U<<56)
#define _57BITS_MAX_VAL_ (1U<<57)
#define _58BITS_MAX_VAL_ (1U<<58)
#define _59BITS_MAX_VAL_ (1U<<59)
#define _60BITS_MAX_VAL_ (1U<<60)
#define _61BITS_MAX_VAL_ (1U<<61)
#define _62BITS_MAX_VAL_ (1U<<62)
#define _63BITS_MAX_VAL_ (1U<<63)


typedef enum
{
    bdmf_test_method_low,
    bdmf_test_method_mid,
    bdmf_test_method_high,
}
bdmf_test_method;

uint32_t gtmv(bdmf_test_method method, uint8_t bits);

#if 0
static inline void ag_ru_block_addr_print(int block_idx)
{
	int addr_idx;
	for (addr_idx = 0; addr_idx < WAN_TOP_BLOCKS[block_idx]->addr_count; addr_idx++)
	{
		bdmf_trace("block %s[%d] address = 0x%08x\n", WAN_TOP_BLOCKS[block_idx]->name, addr_idx,
			WAN_TOP_BLOCKS[block_idx]->addr[addr_idx]);
	}
}

static inline void ag_ru_blocks_data_print()
{
	int block_idx;
	for (block_idx = 0; block_idx < RU_BLK_COUNT; block_idx++)
	{
		ag_ru_block_addr_print(block_idx);
	}
}
#endif
#endif
