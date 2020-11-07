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

#ifndef _XRDP_DRIVERS_COMMON_AG_H_
#define _XRDP_DRIVERS_COMMON_AG_H_


#include <bdmf_data_types.h>
#include "bdmf_shell.h"
#include "bdmf_interface.h"
#include <bdmf_session.h>
#include "ru.h"


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
#define _32BITS_MAX_VAL_ (0xFFFFFFFF)


typedef enum
{
    bdmf_test_method_low,
    bdmf_test_method_mid,
    bdmf_test_method_high,
}
bdmf_test_method;

uint32_t gtmv(bdmf_test_method method, uint8_t bits);


static inline void ag_ru_block_addr_print(int block_idx)
{
	int addr_idx;
	for (addr_idx = 0; addr_idx < RU_ALL_BLOCKS[block_idx]->addr_count; addr_idx++)
	{
		bdmf_trace("block %s[%d] address = 0x%lx\n", RU_ALL_BLOCKS[block_idx]->name, addr_idx,
			RU_ALL_BLOCKS[block_idx]->addr[addr_idx]);
	}
}

static inline void ag_ru_blocks_data_print(void)
{
	int block_idx;
	for (block_idx = 0; block_idx < RU_BLK_COUNT; block_idx++)
	{
		ag_ru_block_addr_print(block_idx);
	}
}
#endif
