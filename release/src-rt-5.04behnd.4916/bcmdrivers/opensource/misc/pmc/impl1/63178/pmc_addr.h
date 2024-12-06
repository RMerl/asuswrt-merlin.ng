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

#define PMB_BUS_ID_SHIFT         12

#define PMB_BUS_PERIPH           0
#define PMB_ADDR_PERIPH          (0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH         4

#define PMB_BUS_CHIP_CLKRST      0
#define PMB_ADDR_CHIP_CLKRST     (1 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST    0

#define BPCM_CLKRST_AFE_PWRDWN   0x20000000

#define PMB_BUS_AFEPLL           0
#define PMB_ADDR_AFEPLL          (2 | PMB_BUS_AFEPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_AFEPLL         0

#define AFEPLL_PMB_BUS_VDSL3_CORE       PMB_BUS_AFEPLL
#define AFEPLL_PMB_ADDR_VDSL3_CORE      PMB_ADDR_AFEPLL
#define AFEPLL_PMB_ZONES_VDSL3_CORE     PMB_ZONES_AFEPLL

#define PMB_BUS_PVTMON           0
#define PMB_ADDR_PVTMON          (3 | PMB_BUS_PVTMON << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PVTMON         0

#define PMB_BUS_SWITCH           0
#define PMB_ADDR_SWITCH          (4 | PMB_BUS_SWITCH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SWITCH         4

#define PMB_BUS_USB30_2X         0
#define PMB_ADDR_USB30_2X        (5 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X       4

#define PMB_BUS_PCIE0            1
#define PMB_ADDR_PCIE0           (6 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0          1

#define PMB_BUS_VDSL3_CORE       1
#define PMB_ADDR_VDSL3_CORE      (7 | PMB_BUS_VDSL3_CORE << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_VDSL3_CORE     2

#define PMB_BUS_VDSL3_MIPS       PMB_BUS_VDSL3_CORE
#define PMB_ADDR_VDSL3_MIPS      PMB_ADDR_VDSL3_CORE
#define PMB_ZONES_VDSL3_MIPS     PMB_ZONES_VDSL3_CORE

//--------- DGASP related bits/Offsets ------------------------
#define BPCM_PHY_CNTL_OVERRIDE         0x08000000
#define PMB_ADDR_VDSL_DGASP_PMD        PMB_ADDR_VDSL3_CORE
#define BPCM_VDSL_PHY_CTL_REG	       vdsl_phy_ctl	// Alias for register containing DGASP override inside the VDSL PMD
#define BPCM_VDSL_AFE_CTL_REG	       vdsl_afe_ctl	// Alias for register containing DGASP configuration inside the VDSL PMD

#define PMB_BUS_MEMC             1
#define PMB_ADDR_MEMC            (8 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC           1

#define PMB_BUS_WLAN0_PHY1       0
#define PMB_ADDR_WLAN0_PHY1      (9 | PMB_BUS_WLAN0_PHY1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WLAN0_PHY1     1

#define PMB_BUS_WLAN0_PHY2       0
#define PMB_ADDR_WLAN0_PHY2      (10 | PMB_BUS_WLAN0_PHY2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WLAN0_PHY2     1

#define PMB_BUS_WLAN0            0
#define PMB_ADDR_WLAN0           (11 | PMB_BUS_WLAN0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WLAN0          1

#define PMB_BUS_ORION_CPU0       1
#define PMB_ADDR_ORION_CPU0      (32 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0     1

#define PMB_BUS_ORION_CPU1       1
#define PMB_ADDR_ORION_CPU1      (33 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1     1

#define PMB_BUS_ORION_CPU2       1
#define PMB_ADDR_ORION_CPU2      (34 | PMB_BUS_ORION_CPU2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU2     1

#define PMB_BUS_ORION_NONCPU     1
#define PMB_ADDR_ORION_NONCPU    (36 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU   1

#define PMB_BUS_BIU_PLL          1
#define PMB_ADDR_BIU_PLL         (38 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL        0

#define PMB_BUS_BIU_BPCM         1
#define PMB_ADDR_BIU_BPCM        (39 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM       1

#define PMB_BUS_PCM              PMB_BUS_PERIPH
#define PMB_ADDR_PCM             (0 | PMB_BUS_PCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCM            4

enum {
	PCM_Zone_Main,
	PCM_Zone_PCM = 3
};
//--------- SOFT Reset bits for PCM ------------------------
#define   BPCM_PCM_SRESET_PCM_N       0x00000040

#define RCAL_0P25UM_HORZ          0
#define RCAL_0P25UM_VERT          1
#define RCAL_0P5UM_HORZ           2
#define RCAL_0P5UM_VERT           3
#define RCAL_1UM_HORZ             4
#define RCAL_1UM_VERT             5
#define PMMISC_RMON_EXT_REG       ((RCAL_1UM_VERT + 1)/2)
#define PMMISC_RMON_VALID_MASK    (0x1<<16)

#include "pmc_sysfs.h" 
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "afe_pll",           PMB_ADDR_AFEPLL,            PMB_ZONES_AFEPLL            },
        { "pvtmon",            PMB_ADDR_PVTMON,            PMB_ZONES_PVTMON            },
        { "switch",            PMB_ADDR_SWITCH,            PMB_ZONES_SWITCH            },
        { "usb30_2x",          PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X          },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "vdsl3_core",        PMB_ADDR_VDSL3_CORE,        PMB_ZONES_VDSL3_CORE        },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "wlan0",             PMB_ADDR_WLAN0,             PMB_ZONES_WLAN0             },
        { "wlan0_phy1",        PMB_ADDR_WLAN0_PHY1,        PMB_ZONES_WLAN0_PHY1        },
        { "wlan0_phy2",        PMB_ADDR_WLAN0_PHY2,        PMB_ZONES_WLAN0_PHY2        },
        { "orion_cpu0",        PMB_ADDR_ORION_CPU0,        PMB_ZONES_ORION_CPU0        },
        { "orion_cpu1",        PMB_ADDR_ORION_CPU1,        PMB_ZONES_ORION_CPU1        },
        { "orion_cpu2",        PMB_ADDR_ORION_CPU2,        PMB_ZONES_ORION_CPU2        },
        { "orion_noncpu",      PMB_ADDR_ORION_NONCPU,      PMB_ZONES_ORION_NONCPU      },
        { "biu_pll",           PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL           },
        { "biu",               PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM          },
        { "pcm",               PMB_ADDR_PCM,               PMB_ZONES_PCM               },
};
#endif
