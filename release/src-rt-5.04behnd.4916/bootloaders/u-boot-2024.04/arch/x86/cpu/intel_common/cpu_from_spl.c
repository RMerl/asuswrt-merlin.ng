// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <handoff.h>
#include <init.h>
#include <asm/cpu_common.h>
#include <asm/global_data.h>
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

#if CONFIG_IS_ENABLED(HANDOFF) && IS_ENABLED(CONFIG_USE_HOB)
	struct spl_handoff *ho = gd->spl_handoff;

	gd->arch.hob_list = ho->arch.hob_list;
#endif
	ret = x86_cpu_reinit_f();

	return ret;
}
