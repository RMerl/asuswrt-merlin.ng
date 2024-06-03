/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 - 2017 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa6<Q,D,DL,S> module on
 * MBa6 starter kit
 */

#ifndef __CONFIG_TQMA6_MBA6_H
#define __CONFIG_TQMA6_MBA6_H

#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"

#define CONFIG_FEC_MXC_PHYADDR		0x03

#define CONFIG_MXC_UART_BASE		UART2_BASE
#define CONSOLE_DEV		"ttymxc1"

#endif /* __CONFIG_TQMA6_MBA6_H */
