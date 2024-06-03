/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Broadcom Ltd.
 */
/*
<:copyright-BRCM:2022:DUAL/GPL:standard 

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:> 
*/

#ifndef __PMC_ADDR_H__
#define __PMC_ADDR_H__

#define PMB_BUS_MAX		2
#define PMB_BUS_ID_SHIFT	12

#define PMB_BUS_PERIPH		0
#define PMB_ADDR_PERIPH		(0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH	4

#define PMB_BUS_CHIP_CLKRST	0
#define PMB_ADDR_CHIP_CLKRST	(1 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST	0

#define PMB_BUS_PVTMON		0
#define PMB_ADDR_PVTMON		(3 | PMB_BUS_PVTMON << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PVTMON	0

#define PMB_BUS_MEMC		0
#define PMB_ADDR_MEMC		(4 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC		3

#define PMB_BUS_USB			0
#define PMB_ADDR_USB		(5 | PMB_BUS_USB << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB		4

#define PMB_BUS_PCIE0		0
#define PMB_ADDR_PCIE0		(6 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0		1

#define PMB_BUS_PCIE1		0
#define PMB_ADDR_PCIE1		(7 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1		1

#define PMB_BUS_CNP			0
#define PMB_ADDR_CNP		(8 | PMB_BUS_CNP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CNP		4
#define PMB_BUS_SWITCH		PMB_BUS_CNP
#define PMB_ADDR_SWITCH		PMB_ADDR_CNP
#define PMB_ZONES_SWITCH	PMB_ZONES_CNP

#define PMB_BUS_WLAN0		1
#define PMB_ADDR_WLAN0		(9 | PMB_BUS_WLAN0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WLAN0		1

#define PMB_BUS_WLAN1		1
#define PMB_ADDR_WLAN1		(10 | PMB_BUS_WLAN1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WLAN1		1

#define PMB_BUS_CRYPTO		1
#define PMB_ADDR_CRYPTO		(23 | PMB_BUS_CRYPTO << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CRYPTO	1

#define PMB_BUS_ORION_PLL	0
#define PMB_ADDR_ORION_PLL	(32 | PMB_BUS_ORION_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_PLL	0
#define PMB_BUS_BIU_PLL		PMB_BUS_ORION_PLL
#define PMB_ADDR_BIU_PLL	PMB_ADDR_ORION_PLL
#define PMB_ZONES_BIU_PLL	PMB_ZONES_ORION_PLL

#define PMB_BUS_ORION_BPCM	0
#define PMB_ADDR_ORION_BPCM	(33 | PMB_BUS_ORION_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_BPCM	1
#define PMB_BUS_BIU_BPCM	PMB_BUS_ORION_BPCM
#define PMB_ADDR_BIU_BPCM	PMB_ADDR_ORION_BPCM
#define PMB_ZONES_BIU_BPCM	PMB_ZONES_ORION_BPCM

#define PMB_BUS_ORION_CPU0	0
#define PMB_ADDR_ORION_CPU0	(34 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0	1

#define PMB_BUS_ORION_CPU1	0
#define PMB_ADDR_ORION_CPU1	(35 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1	1

#define PMB_BUS_ORION_CPU2	0
#define PMB_ADDR_ORION_CPU2	(36 | PMB_BUS_ORION_CPU2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU2	1

#define PMB_BUS_ORION_CPU3	0
#define PMB_ADDR_ORION_CPU3	(37 | PMB_BUS_ORION_CPU3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU3	1

#define PMB_BUS_ORION_NONCPU	0
#define PMB_ADDR_ORION_NONCPU	(38 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU	1

#include "pmc_sysfs.h"
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "pvtmon",            PMB_ADDR_PVTMON,            PMB_ZONES_PVTMON            },
        { "crypto",            PMB_ADDR_CRYPTO,            PMB_ZONES_CRYPTO            },
        { "usb",               PMB_ADDR_USB,               PMB_ZONES_USB               },
        { "wlan0",             PMB_ADDR_WLAN0,             PMB_ZONES_WLAN0             },
        { "wlan1",             PMB_ADDR_WLAN1,             PMB_ZONES_WLAN1             },
        { "cnp",               PMB_ADDR_CNP,               PMB_ZONES_CNP               },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE0             },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "biu_pll",           PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL           },
        { "biu",               PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM          },
        { "orion_cpu0",        PMB_ADDR_ORION_CPU0,        PMB_ZONES_ORION_CPU0        },
        { "orion_cpu1",        PMB_ADDR_ORION_CPU1,        PMB_ZONES_ORION_CPU1        },
        { "orion_cpu2",        PMB_ADDR_ORION_CPU2,        PMB_ZONES_ORION_CPU2        },
        { "orion_cpu3",        PMB_ADDR_ORION_CPU3,        PMB_ZONES_ORION_CPU3        },
        { "orion_noncpu",      PMB_ADDR_ORION_NONCPU,      PMB_ZONES_ORION_NONCPU      },
};

#endif
