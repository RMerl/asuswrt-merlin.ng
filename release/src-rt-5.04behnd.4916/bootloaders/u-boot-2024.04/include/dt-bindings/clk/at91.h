/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * This header provides constants for AT91 pmc status.
 * The constants defined in this header are being used in dts and PMC code.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on include/dt-bindings/clock/at91.h on Linux.
 */

#ifndef _DT_BINDINGS_CLK_AT91_H
#define _DT_BINDINGS_CLK_AT91_H

#define PMC_TYPE_CORE		1
#define PMC_TYPE_SYSTEM		2
#define PMC_TYPE_PERIPHERAL	3
#define PMC_TYPE_GCK		4
#define PMC_TYPE_SLOW		5
#define USB_UTMI		6

#define USB_UTMI1		0
#define USB_UTMI2		1
#define USB_UTMI3		2

#endif
