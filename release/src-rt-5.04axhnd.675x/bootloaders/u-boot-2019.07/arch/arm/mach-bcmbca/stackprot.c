/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long __stack_chk_guard = 0xdeadbeef;

void __stack_chk_fail(void)
{
	panic("Stack smashing detected in function: %p relocated from %p",
		__builtin_return_address(0),
		__builtin_return_address(0) - gd->reloc_off);
}
