/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Broadcom Ltd.
 */

#ifndef _6888_PMC_ADDR_H
#define _6888_PMC_ADDR_H


#define PMB_BUS_MAX				2
#define PMB_BUS_ID_SHIFT		12

#define PMB_BUS_PERIPH			0
#define PMB_ADDR_PERIPH			(0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH		4

#define PMB_BUS_CHIP_CLKRST		0
#define PMB_ADDR_CHIP_CLKRST	(1 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST	0

#define PMB_BUS_RDPPLL           0
#define PMB_ADDR_RDPPLL          (2 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL         0

#define PMB_BUS_SYSPLL			0
#define PMB_ADDR_SYSPLL			(3 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL		0

#define PMB_BUS_PVTMON			1
#define PMB_ADDR_PVTMON			(5 | PMB_BUS_PVTMON << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PVTMON		0

#define PMB_BUS_MEMC             1
#define PMB_ADDR_MEMC            (7 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC           3

#define PMB_BUS_USB              1
#define PMB_ADDR_USB             (8 | PMB_BUS_USB << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB            4

#define PMB_BUS_XRDP             0
#define PMB_ADDR_XRDP            (9 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP           4

#define PMB_BUS_XRDP_RC          1
#define PMB_ADDR_XRDP_RC         (10 | PMB_BUS_XRDP_RC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC        7

#define PMB_BUS_PCIE2            1
#define PMB_ADDR_PCIE2           (12 | PMB_BUS_PCIE2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE2          1

#define PMB_BUS_PCIE0            1
#define PMB_ADDR_PCIE0           (13 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0          1

#define PMB_BUS_PCIE1            1
#define PMB_ADDR_PCIE1           (14 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1          1

#define PMB_BUS_PCIE3            1
#define PMB_ADDR_PCIE3           (15 | PMB_BUS_PCIE3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE3          1

#define PMB_BUS_CRYPTO           1
#define PMB_ADDR_CRYPTO          (17 | PMB_BUS_CRYPTO << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CRYPTO         1

#define PMB_BUS_WAN              0
#define PMB_ADDR_WAN             (18 | PMB_BUS_WAN << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WAN            7

#define PMB_BUS_ETH              0
#define PMB_ADDR_ETH             (19 | PMB_BUS_ETH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ETH            6

#define PMB_BUS_BIU_PLL          0
#define PMB_ADDR_BIU_PLL         (32 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL        0

#define PMB_BUS_BIU_BPCM         0
#define PMB_ADDR_BIU_BPCM        (33 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM       1

#define PMB_BUS_ORION_CPU0       0
#define PMB_ADDR_ORION_CPU0      (34 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0     1

#define PMB_BUS_ORION_CPU1       0
#define PMB_ADDR_ORION_CPU1      (35 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1     1

#define PMB_BUS_ORION_CPU2       0
#define PMB_ADDR_ORION_CPU2      (36 | PMB_BUS_ORION_CPU2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU2     1

#define PMB_BUS_ORION_CPU3       0
#define PMB_ADDR_ORION_CPU3      (37 | PMB_BUS_ORION_CPU3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU3     1

#define PMB_BUS_ORION_NONCPU     0
#define PMB_ADDR_ORION_NONCPU    (38 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU   1

#endif
