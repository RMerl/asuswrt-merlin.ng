/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */
/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

#ifndef _PMC_ADDR_H
#define _PMC_ADDR_H

#define PMB_BUS_MAX		2
#define PMB_BUS_ID_SHIFT	12

#define PMB_BUS_PERIPH		0
#define PMB_ADDR_PERIPH		(0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH	4

#define PMB_BUS_CHIP_CLKRST	0
#define PMB_ADDR_CHIP_CLKRST	(1 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST	0

#define PMB_BUS_PVTMON		1
#define PMB_ADDR_PVTMON		(3 | PMB_BUS_PVTMON << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PVTMON	0

#define PMB_BUS_CRYPTO		1
#define PMB_ADDR_CRYPTO		(4 | PMB_BUS_CRYPTO << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CRYPTO	0

#define PMB_BUS_USB30_2X	0
#define PMB_ADDR_USB30_2X	(5 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X	4

#define PMB_BUS_PCIE1		1
#define PMB_ADDR_PCIE1		(6 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1		4

// referring to PCIEG3
#define PMB_BUS_PCIE3		1
#define PMB_ADDR_PCIE3		(7 | PMB_BUS_PCIE3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE3		1

#define PMB_BUS_MEMC		1
#define PMB_ADDR_MEMC		(8 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC		1

#define PMB_BUS_XRDP		1
#define PMB_ADDR_XRDP		(9 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP		1

#define PMB_BUS_PCIE2		1
#define PMB_ADDR_PCIE2		(11 | PMB_BUS_PCIE2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE2		1

#define PMB_BUS_PCIE0		0
#define PMB_ADDR_PCIE0		(12 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0		1

#define PMB_BUS_ETH		1
#define PMB_ADDR_ETH		(13 | PMB_BUS_ETH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ETH		1

#define PMB_BUS_MPM		1
#define PMB_ADDR_MPM		(14 | PMB_BUS_MPM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MPM		1

#define PMB_BUS_XRDPPLL		0
#define PMB_ADDR_XRDPPLL	(15 | PMB_BUS_XRDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDPPLL	0

#define PMB_BUS_PERIPH_ARS	0
#define PMB_ADDR_PERIPH_ARS	(16 | PMB_BUS_PERIPH_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH_ARS	0

#define PMB_BUS_PCIE0_UBUS_ARS	0
#define PMB_ADDR_PCIE0_UBUS_ARS	(17 | PMB_BUS_PCIE0_UBUS_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0_UBUS_ARS 0

#define PMB_BUS_USB30_2X_ARS	0
#define PMB_ADDR_USB30_2X_ARS	(18 | PMB_BUS_USB30_2X_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X_ARS	0

#define PMB_BUS_SYS_ARS		0
#define PMB_ADDR_SYS_ARS	(19 | PMB_BUS_SYS_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYS_ARS	0

#define PMB_BUS_CRYPTO2_ARS	1
#define PMB_ADDR_CRYPTO2_ARS	(20 | PMB_BUS_CRYPTO2_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CRYPTO2_ARS	0

#define PMB_BUS_XRDP_ARS	1
#define PMB_ADDR_XRDP_ARS	(21 | PMB_BUS_XRDP_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_ARS	0

#define PMB_BUS_MPM_ARS		1
#define PMB_ADDR_MPM_ARS	(22 | PMB_BUS_MPM_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MPM_ARS	0

#define PMB_BUS_MEMC_ARS	1
#define PMB_ADDR_MEMC_ARS	(23 | PMB_BUS_MEMC_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC_ARS	0

#define PMB_BUS_ETH_ARS		1
#define PMB_ADDR_ETH_ARS	(24 | PMB_BUS_ETH_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ETH_ARS	0

#define PMB_BUS_PCIE1_UBUS_ARS	1
#define PMB_ADDR_PCIE1_UBUS_ARS	(25 | PMB_BUS_PCIE1_UBUS_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1_UBUS_ARS	0

#define PMB_BUS_PCIE3_ARS	1
#define PMB_ADDR_PCIE3_ARS	(26 | PMB_BUS_PCIE3_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE3_ARS	0

#define PMB_BUS_MERLIN0_UBUS_ARS	1
#define PMB_ADDR_MERLIN0_UBUS_ARS	(27 | PMB_BUS_MERLIN0_UBUS_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MERLIN0_UBUS_ARS	0

#define PMB_BUS_MERLIN1_UBUS_ARS	1
#define PMB_ADDR_MERLIN1_UBUS_ARS	(28 | PMB_BUS_MERLIN1_UBUS_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MERLIN1_UBUS_ARS	0

#define PMB_BUS_MERLIN2_UBUS_ARS	1
#define PMB_ADDR_MERLIN2_UBUS_ARS	(29 | PMB_BUS_MERLIN2_UBUS_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MERLIN2_UBUS_ARS	0

#define PMB_BUS_ORION_PLL	1
#define PMB_ADDR_ORION_PLL	(32 | PMB_BUS_ORION_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_PLL	0
#define PMB_BUS_BIU_PLL		PMB_BUS_ORION_PLL
#define PMB_ADDR_BIU_PLL	PMB_ADDR_ORION_PLL
#define PMB_ZONES_BIU_PLL	PMB_ZONES_ORION_PLL

#define PMB_BUS_ORION_BPCM	1
#define PMB_ADDR_ORION_BPCM	(33 | PMB_BUS_ORION_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_BPCM	1
#define PMB_BUS_BIU_BPCM	PMB_BUS_ORION_BPCM
#define PMB_ADDR_BIU_BPCM	PMB_ADDR_ORION_BPCM
#define PMB_ZONES_BIU_BPCM	PMB_ZONES_ORION_BPCM

#define PMB_BUS_ORION_CPU0	1
#define PMB_ADDR_ORION_CPU0	(34 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0	1

#define PMB_BUS_ORION_CPU1	1
#define PMB_ADDR_ORION_CPU1	(35 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1	1

#define PMB_BUS_ORION_CPU2	1
#define PMB_ADDR_ORION_CPU2	(36 | PMB_BUS_ORION_CPU2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU2	1

#define PMB_BUS_ORION_CPU3	1
#define PMB_ADDR_ORION_CPU3	(37 | PMB_BUS_ORION_CPU3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU3	1

#define PMB_BUS_ORION_NONCPU	1
#define PMB_ADDR_ORION_NONCPU	(38 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU	1

#define PMB_BUS_ORION_ARS	1
#define PMB_ADDR_ORION_ARS	(39 | PMB_BUS_ORION_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_ARS	0

#define PMB_BUS_ORION_ACEBIU_ARS	1
#define PMB_ADDR_ORION_ACEBIU_ARS	(40 | PMB_BUS_ORION_ACEBIU_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_ACEBIU_ARS	0

#include "pmc_sysfs.h"
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "pvtmon",            PMB_ADDR_PVTMON,            PMB_ZONES_PVTMON            },
        { "crypto",            PMB_ADDR_CRYPTO,            PMB_ZONES_CRYPTO            },
        { "usb30_2x",          PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X          },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1             },
        { "pcie3",             PMB_ADDR_PCIE3,             PMB_ZONES_PCIE3             },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "xrdp",              PMB_ADDR_XRDP,              PMB_ZONES_XRDP,             },
        { "pcie2",             PMB_ADDR_PCIE2,             PMB_ZONES_PCIE2             },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "eth",               PMB_ADDR_ETH,               PMB_ZONES_ETH               },
        { "mpm",               PMB_ADDR_MPM,               PMB_ZONES_MPM               },
        { "xrdp_pll",          PMB_ADDR_XRDPPLL,           PMB_ZONES_XRDPPLL           },
        { "biu_pll",           PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL           },
        { "biu",               PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM          },
        { "orion_cpu0",        PMB_ADDR_ORION_CPU0,        PMB_ZONES_ORION_CPU0        },
        { "orion_cpu1",        PMB_ADDR_ORION_CPU1,        PMB_ZONES_ORION_CPU1        },
        { "orion_cpu2",        PMB_ADDR_ORION_CPU2,        PMB_ZONES_ORION_CPU2        },
        { "orion_cpu3",        PMB_ADDR_ORION_CPU3,        PMB_ZONES_ORION_CPU3        },
        { "orion_noncpu",      PMB_ADDR_ORION_NONCPU,      PMB_ZONES_ORION_NONCPU      },
};

static const pmb_init_t xrdp_pmb[] = {
    /* name      dev                reset value  */
    {"xrdp",     PMB_ADDR_XRDP,     0xfffffff8},
};


#endif
