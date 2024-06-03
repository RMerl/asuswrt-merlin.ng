// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 */
#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <div64.h>
#include <asm/scu.h>

/* Hardware timeout in seconds */
#define WDT_PRETIMEOUT		15
#define WDT_TIMEOUT_MIN		(1 + WDT_PRETIMEOUT)
#define WDT_TIMEOUT_MAX		170

/*
 * Note, firmware chooses 90 seconds as a default timeout for watchdog on
 * Intel Tangier SoC. It means that without handling it in the running code
 * the reboot will happen.
 */

enum {
	SCU_WATCHDOG_START			= 0,
	SCU_WATCHDOG_STOP			= 1,
	SCU_WATCHDOG_KEEPALIVE			= 2,
	SCU_WATCHDOG_SET_ACTION_ON_TIMEOUT	= 3,
};

static int tangier_wdt_reset(struct udevice *dev)
{
	scu_ipc_simple_command(IPCMSG_WATCHDOG_TIMER, SCU_WATCHDOG_KEEPALIVE);
	return 0;
}

static int tangier_wdt_stop(struct udevice *dev)
{
	return scu_ipc_simple_command(IPCMSG_WATCHDOG_TIMER, SCU_WATCHDOG_STOP);
}

static int tangier_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	u32 timeout_sec;
	int in_size;
	struct ipc_wd_start {
		u32 pretimeout;
		u32 timeout;
	} ipc_wd_start;

	/* Calculate timeout in seconds and restrict to min and max value */
	do_div(timeout_ms, 1000);
	timeout_sec = clamp_t(u32, timeout_ms, WDT_TIMEOUT_MIN, WDT_TIMEOUT_MAX);

	/* Update values in the IPC request */
	ipc_wd_start.pretimeout = timeout_sec - WDT_PRETIMEOUT;
	ipc_wd_start.timeout = timeout_sec;

	/*
	 * SCU expects the input size for watchdog IPC
	 * to be based on 4 bytes
	 */
	in_size = DIV_ROUND_UP(sizeof(ipc_wd_start), 4);

	scu_ipc_command(IPCMSG_WATCHDOG_TIMER, SCU_WATCHDOG_START,
			(u32 *)&ipc_wd_start, in_size, NULL, 0);

	return 0;
}

static const struct wdt_ops tangier_wdt_ops = {
	.reset = tangier_wdt_reset,
	.start = tangier_wdt_start,
	.stop = tangier_wdt_stop,
};

static const struct udevice_id tangier_wdt_ids[] = {
	{ .compatible = "intel,tangier-wdt" },
	{ /* sentinel */ }
};

static int tangier_wdt_probe(struct udevice *dev)
{
	debug("%s: Probing wdt%u\n", __func__, dev->seq);
	return 0;
}

U_BOOT_DRIVER(wdt_tangier) = {
	.name = "wdt_tangier",
	.id = UCLASS_WDT,
	.of_match = tangier_wdt_ids,
	.ops = &tangier_wdt_ops,
	.probe = tangier_wdt_probe,
};
