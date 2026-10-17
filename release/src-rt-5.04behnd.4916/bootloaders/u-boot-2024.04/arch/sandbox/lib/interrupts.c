// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <efi_loader.h>
#include <irq_func.h>
#include <os.h>
#include <asm/global_data.h>
#include <asm-generic/signal.h>
#include <asm/u-boot-sandbox.h>

DECLARE_GLOBAL_DATA_PTR;

int interrupt_init(void)
{
	return 0;
}

void enable_interrupts(void)
{
	return;
}
int disable_interrupts(void)
{
	return 0;
}

void os_signal_action(int sig, unsigned long pc)
{
	efi_restore_gd();

	switch (sig) {
	case SIGILL:
		printf("\nIllegal instruction\n");
		break;
	case SIGBUS:
		printf("\nBus error\n");
		break;
	case SIGSEGV:
		printf("\nSegmentation violation\n");
		break;
	default:
		break;
	}
	printf("pc = 0x%lx, ", pc);
	printf("pc_reloc = 0x%lx\n\n", pc - gd->reloc_off);
	efi_print_image_infos((void *)pc);

	if (IS_ENABLED(CONFIG_SANDBOX_CRASH_RESET)) {
		printf("resetting ...\n\n");
		sandbox_reset();
	} else {
		sandbox_exit();
	}
}
