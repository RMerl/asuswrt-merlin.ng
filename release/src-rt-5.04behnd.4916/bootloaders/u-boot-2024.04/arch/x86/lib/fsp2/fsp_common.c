// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <init.h>
#include <asm/fsp/fsp_support.h>

void board_final_cleanup(void)
{
	u32 status;

	/* TODO(sjg@chromium.org): This causes Linux to crash */
	return;

	/* call into FspNotify */
	debug("Calling into FSP (notify phase INIT_PHASE_END_FIRMWARE): ");
	status = fsp_notify(NULL, INIT_PHASE_END_FIRMWARE);
	if (status)
		debug("fail, error code %x\n", status);
	else
		debug("OK\n");
}
