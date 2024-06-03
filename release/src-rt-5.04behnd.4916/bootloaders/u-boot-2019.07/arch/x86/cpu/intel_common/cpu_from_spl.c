// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/cpu_common.h>
#include <asm/intel_regs.h>
#include <asm/lapic.h>
#include <asm/lpc_common.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/post.h>
#include <asm/microcode.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_cpu_init(void)
{
	int ret;

	ret = x86_cpu_reinit_f();

	return ret;
}
