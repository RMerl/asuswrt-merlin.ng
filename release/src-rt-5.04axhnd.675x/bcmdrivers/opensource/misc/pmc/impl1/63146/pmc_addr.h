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

#ifndef PMC_ADDR_H__
#define PMC_ADDR_H__

/* FIXME! only fill those that I found from RTL */
#define PMB_BUS_MAX		2
#define PMB_BUS_ID_SHIFT	12

#define PMB_BUS_PCIE0		0
#define PMB_ADDR_PCIE0		(0 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0		1

#define PMB_BUS_VDSL3_CORE	0
#define PMB_ADDR_VDSL3_CORE	(1 | PMB_BUS_VDSL3_CORE << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_VDSL3_CORE	1

#define PMB_BUS_EGPHY		0
#define PMB_ADDR_EGPHY		(2 | PMB_BUS_EGPHY << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_EGPHY		1	// not shown in spreadsheet

#define PMB_BUS_XRDP		0
#define PMB_ADDR_XRDP		(3 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP		3

#define PMB_BUS_USB30_2X	0
#define PMB_ADDR_USB30_2X	(4 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X	4

#define PMB_BUS_MEMC		0
#define PMB_ADDR_MEMC		(5 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC		1

#define PMB_BUS_PVTMON		0
#define PMB_ADDR_PVTMON		(6 | PMB_BUS_PVTMON << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PVTMON	0

#define PMB_BUS_PCIE1		0
#define PMB_ADDR_PCIE1		(7 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1		1

#define PMB_BUS_PCIE2		0
#define PMB_ADDR_PCIE2		(8 | PMB_BUS_PCIE2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE2		1

#if (CONFIG_BRCM_CHIP_REV != 0x63146A0)
#define PMB_BUS_MPM		0
#define PMB_ADDR_MPM		(9 | PMB_BUS_MPM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MPM		1
#endif

#define PMB_BUS_PERIPH		1
#define PMB_ADDR_PERIPH		(9 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH	4

#define PMB_BUS_VDSL3_PMD	1
#define PMB_ADDR_VDSL3_PMD	(10 | PMB_BUS_VDSL3_PMD << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_VDSL3_PMD	1

//--------- DGASP related bits/Offsets ------------------------
#define BPCM_PHY_CNTL_OVERRIDE		0x00000002	
#define BPCM_PHY_CNTL_AFE_PWRDWN	0x00000001
#define PMB_ADDR_VDSL_DGASP_PMD		PMB_ADDR_VDSL3_PMD
#define BPCM_VDSL_PHY_CTL_REG		vdsl_afe_config1	// Alias for register containing DGASP override inside the VDSL PMD
#define BPCM_VDSL_AFE_CTL_REG		vdsl_afe_config0	// Alias for register containing DGASP configuration inside the VDSL PMD

#define PMB_BUS_AFEPLL		1
#define PMB_ADDR_AFEPLL		(11 | PMB_BUS_AFEPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_AFEPLL	1

#define AFEPLL_PMB_BUS_VDSL3_CORE	PMB_BUS_AFEPLL
#define AFEPLL_PMB_ADDR_VDSL3_CORE	PMB_ADDR_AFEPLL
#define AFEPLL_PMB_ZONES_VDSL3_CORE	PMB_ZONES_AFEPLL

#define PMB_BUS_CHIP_CLKRST	1
#define PMB_ADDR_CHIP_CLKRST	(12 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST	0

#define PMB_BUS_RDPPLL		1
#define PMB_ADDR_RDPPLL		(13 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL	0

#define PMB_BUS_BIU_PLL		1
#define PMB_ADDR_BIU_PLL	(32 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL	0

#define PMB_BUS_BIU_BPCM	1
#define PMB_ADDR_BIU_BPCM	(33 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM	1

#define PMB_BUS_ORION_CPU0	1
#define PMB_ADDR_ORION_CPU0	(34 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0	1

#define PMB_BUS_ORION_CPU1	1
#define PMB_ADDR_ORION_CPU1	(35 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1	1

#define PMB_BUS_ORION_NONCPU	1
#define PMB_ADDR_ORION_NONCPU	(38 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU	1

#define PMB_BUS_ORION_ARS	1
#define PMB_ADDR_ORION_ARS	(39 | PMB_BUS_ORION_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_ARS	0

#include "pmc_sysfs.h" 
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "afe_pll",           PMB_ADDR_AFEPLL,            PMB_ZONES_AFEPLL            },
        { "pvtmon",            PMB_ADDR_PVTMON,            PMB_ZONES_PVTMON            },
        { "ethphy",            PMB_ADDR_EGPHY,             PMB_ZONES_EGPHY             },
        { "usb30_2x",          PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X          },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1             },
        { "pcie2",             PMB_ADDR_PCIE2,             PMB_ZONES_PCIE2             },
        { "xrdp",              PMB_ADDR_XRDP,              PMB_ZONES_XRDP,             },
#if (CONFIG_BRCM_CHIP_REV != 0x63146A0)
        { "mpm",               PMB_ADDR_MPM,               PMB_ZONES_MPM               },
#endif
        { "vdsl3_core",        PMB_ADDR_VDSL3_CORE,        PMB_ZONES_VDSL3_CORE        },
        { "vdsl3_pmd",         PMB_ADDR_VDSL3_PMD,         PMB_ZONES_VDSL3_PMD         },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "orion_cpu0",        PMB_ADDR_ORION_CPU0,        PMB_ZONES_ORION_CPU0        },
        { "orion_cpu1",        PMB_ADDR_ORION_CPU1,        PMB_ZONES_ORION_CPU1        },
        { "orion_noncpu",      PMB_ADDR_ORION_NONCPU,      PMB_ZONES_ORION_NONCPU      },
        { "biu_pll",           PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL           },
        { "biu",               PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM          },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL            },
};

static const pmb_init_t xrdp_pmb[] = {
    /* name      dev                reset value  */
    {"xrdp",     PMB_ADDR_XRDP,     0xfffffff8},
};

#endif
