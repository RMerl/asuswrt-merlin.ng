/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Further modified by Broadcom Corp, Jan 25 2017
 */

#ifndef __CSS_SCMI_PRIVATE_H__
#define __CSS_SCMI_PRIVATE_H__

/* SCMI Protocols */
#define SCMI_PROTOCOL_BASE	0x10
#define	SCMI_PROTOCOL_POWER	0x11
#define	SCMI_PROTOCOL_SYSTEM	0x12
#define	SCMI_PROTOCOL_PERF	0x13
#define	SCMI_PROTOCOL_CLOCK	0x14
#define	SCMI_PROTOCOL_SENSOR	0x15
#define	SCMI_PROTOCOL_RESET	0x16
#define	SCMI_PROTOCOL_VOLTAGE	0x17
#define	SCMI_PROTOCOL_BRCM	0x80
#define	SCMI_PROTOCOL_VPUCOM	0x82

/* SCMI Message IDs common to all Protocols */
#define PROTOCOL_VERSION		0x0
#define PROTOCOL_ATTRIBUTES		0x1
#define PROTOCOL_MESSAGE_ATTRIBUTES	0x2

/* SCMI Base Protocol Message IDs */
#define BASE_DISCOVER_VENDOR		0x3
#define BASE_DISCOVER_SUB_VENDOR	0x4
#define BASE_DISCOVER_IMPLEMENT_VERSION	0x5
#define BASE_DISCOVER_LIST_PROTOCOLS	0x6
#define BASE_DISCOVER_AGENT		0x7
#define BASE_NOTIFY_ERRORS		0x8

/* SCMI Perf attritbutes */
#define PERF_ATTR_PWR_IN_MW		(1 << 16)

/* SCMI Perf domain attritbutes */
#define PERF_DOM_ATTR_SET_LIMIT		(1 << 31)
#define PERF_DOM_ATTR_SET_PERF		(1 << 30)
#define PERF_DOM_ATTR_LIMIT_NOTIFY	(1 << 29)
#define PERF_DOM_ATTR_CHANGE_NOTIFY	(1 << 28)

/* Indivdual command constants */
#define CLOCK_CONFIG_SET		7 /* Clock Proto Cmd */
#define PERF_LEVEL_SET			7 /* Perf Proto Cmd */
#define PERF_LEVEL_GET			8 /* Perf Proto Cmd */

#endif	/* __CSS_SCMI_PRIVATE_H__ */
