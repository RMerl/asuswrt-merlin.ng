/*
 * Header file for DHD daemon to handle timeouts
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_daemon.h 671442 2016-11-22 05:16:18Z $
 */

#ifndef __BCM_DHDD_H__
#define __BCM_DHDD_H__

/**
 * To maintain compatabily when dhd driver and dhd daemon is taken from different branches,
 * make sure to keep this file same across dhd driver branch and dhd apps branch.
 * TODO: Make this file as shared between apps and dhd.ko
 */

#define BCM_TO_MAGIC 0x600DB055
#define NO_TRAP 0
#define DO_TRAP	1

#define BCM_NL_USER 31

typedef enum notify_dhd_daemon_reason {
	REASON_COMMAND_TO,
	REASON_OQS_TO,
	REASON_SCAN_TO,
	REASON_JOIN_TO,
	REASON_DAEMON_STARTED,
	REASON_DEVICE_TX_STUCK_WARNING,
	REASON_DEVICE_TX_STUCK,
	REASON_UNKOWN
} notify_dhd_daemon_reason_t;

typedef struct bcm_to_info {
	int magic;
	int reason;
	int trap;
} bcm_to_info_t;

#endif /* __BCM_DHDD_H__ */
