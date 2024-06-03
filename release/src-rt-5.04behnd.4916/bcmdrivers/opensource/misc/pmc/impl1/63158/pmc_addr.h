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

#define PMB_BUS_MAX              2
#define PMB_BUS_ID_SHIFT         8

#define PMB_BUS_PERIPH           1
#define PMB_ADDR_PERIPH          (3 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH         4

#define PMB_BUS_CHIP_CLKRST      0
#define PMB_ADDR_CHIP_CLKRST     (1 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST    0

#define BPCM_CLKRST_AFE_PWRDWN   0x80000000

#define PMB_BUS_SYSPLL           0
#define PMB_ADDR_SYSPLL          (4 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL         0

#define PMB_BUS_RDPPLL           0
#define PMB_ADDR_RDPPLL          (6 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL         0

#define PMB_BUS_UBUSPLL          0
#define PMB_ADDR_UBUSPLL         (5 | PMB_BUS_UBUSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_UBUSPLL        0

#define PMB_BUS_MEMC             0
#define PMB_ADDR_MEMC            (2 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC           1

#define PMB_BUS_SYNC_PLL         1
#define PMB_ADDR_SYNC_PLL        (7 | PMB_BUS_SYNC_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYNC_PLL       1

#define PMB_BUS_USB30_2X         1
#define PMB_ADDR_USB30_2X        (13 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X       4

#define PMB_BUS_WAN              1
#define PMB_ADDR_WAN             (15 | PMB_BUS_WAN << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WAN            7

#define PMB_BUS_XRDP             1
#define PMB_ADDR_XRDP            (16 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP           3

#define PMB_BUS_PCIE0            0
#define PMB_ADDR_PCIE0           (8 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0          1

#define PMB_BUS_PCIE1            0
#define PMB_ADDR_PCIE1           (9 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1          1

#define PMB_BUS_PCIE2            0
#define PMB_ADDR_PCIE2           (10 | PMB_BUS_PCIE2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE2          1

#define PMB_BUS_PCIE3            1
#define PMB_ADDR_PCIE3           (12 | PMB_BUS_PCIE3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE3          1

#define PMB_BUS_SATA             0
#define PMB_ADDR_SATA            (11 | PMB_BUS_SATA << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SATA           1

#define PMB_BUS_SGMII            1
#define PMB_ADDR_SGMII           (14 | PMB_BUS_SGMII << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SGMII          0

#define PMB_BUS_SWITCH           1
#define PMB_ADDR_SWITCH          (0 | PMB_BUS_SWITCH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SWITCH         4

#define PMB_BUS_XRDP_RC0         1
#define PMB_ADDR_XRDP_RC0        (17 | PMB_BUS_XRDP_RC0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC0       1

#define PMB_BUS_XRDP_RC1         1
#define PMB_ADDR_XRDP_RC1        (18 | PMB_BUS_XRDP_RC1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC1       1

#define PMB_BUS_XRDP_RC2         1
#define PMB_ADDR_XRDP_RC2        (19 | PMB_BUS_XRDP_RC2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC2       1

#define PMB_BUS_XRDP_RC3         1
#define PMB_ADDR_XRDP_RC3        (20 | PMB_BUS_XRDP_RC3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC3       1

#define PMB_BUS_XRDP_RC4         1
#define PMB_ADDR_XRDP_RC4        (21 | PMB_BUS_XRDP_RC4 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC4       1

#define PMB_BUS_XRDP_RC5         1
#define PMB_ADDR_XRDP_RC5        (22 | PMB_BUS_XRDP_RC5 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC5       1

#define PMB_BUS_VDSL3_CORE       0
#define PMB_ADDR_VDSL3_CORE      (23 | PMB_BUS_VDSL3_CORE << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_VDSL3_CORE     1

#define PMB_BUS_VDSL3_MIPS       PMB_BUS_VDSL3_CORE
#define PMB_ADDR_VDSL3_MIPS      PMB_ADDR_VDSL3_CORE
#define PMB_ZONES_VDSL3_MIPS     PMB_ZONES_VDSL3_CORE

#define PMB_BUS_VDSL3_PMD        0
#define PMB_ADDR_VDSL3_PMD       (24 | PMB_BUS_VDSL3_PMD << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_VDSL3_PMD      1

//--------- DGASP related bits/Offsets ------------------------
#define BPCM_PHY_CNTL_OVERRIDE         0x08000000
#define PMB_ADDR_VDSL_DGASP_PMD        PMB_ADDR_VDSL3_PMD
#define BPCM_VDSL_PHY_CTL_REG	       global_control	// Alias for register containing DGASP override inside the VDSL PMD
#define BPCM_VDSL_AFE_CTL_REG	       misc_control	// Alias for register containing DGASP configuration inside the VDSL PMD

#define PMB_BUS_CRYPTO           0
#define PMB_ADDR_CRYPTO          (25 | PMB_BUS_CRYPTO << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CRYPTO         1

#define AFEPLL_PMB_BUS_VDSL3_CORE       0
#define AFEPLL_PMB_ADDR_VDSL3_CORE      (26 | AFEPLL_PMB_BUS_VDSL3_CORE << PMB_BUS_ID_SHIFT)
#define AFEPLL_PMB_ZONES_VDSL3_CORE     0

#define PMB_BUS_ORION_CPU0       0
#define PMB_ADDR_ORION_CPU0      (32 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0     1

#define PMB_BUS_ORION_CPU1       0
#define PMB_ADDR_ORION_CPU1      (33 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1     1

#define PMB_BUS_ORION_CPU2       0
#define PMB_ADDR_ORION_CPU2      (34 | PMB_BUS_ORION_CPU2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU2     1

#define PMB_BUS_ORION_CPU3       0
#define PMB_ADDR_ORION_CPU3      (35 | PMB_BUS_ORION_CPU3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU3     1

#define PMB_BUS_ORION_NONCPU     0
#define PMB_ADDR_ORION_NONCPU    (36 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU   1

#define PMB_BUS_ORION_ARS        0
#define PMB_ADDR_ORION_ARS       (37 | PMB_BUS_ORION_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_ARS      0

#define PMB_BUS_BIU_PLL          0
#define PMB_ADDR_BIU_PLL         (38 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL        0

#define PMB_BUS_BIU_BPCM         0
#define PMB_ADDR_BIU_BPCM        (39 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM       1

#define PMB_BUS_ORION_C0_ARS     0
#define PMB_ADDR_ORION_C0_ARS    (45 | PMB_BUS_ORION_C0_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_C0_ARS   0

#define PMB_BUS_PCM              1
#define PMB_ADDR_PCM             (3 | PMB_BUS_PCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCM            4

enum {
	PCM_Zone_Main,
	PCM_Zone_PCM = 3
};
//--------- SOFT Reset bits for PCM ------------------------
#define   BPCM_PCM_SRESET_PCM_N       0x00000040

#include "pmc_sysfs.h" 
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
        { "pcm",               PMB_ADDR_PCM,               PMB_ZONES_PCM               },
        { "switch",            PMB_ADDR_SWITCH,            PMB_ZONES_SWITCH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "sata",              PMB_ADDR_SATA,              PMB_ZONES_SATA              },
        { "orion_cpu0",        PMB_ADDR_ORION_CPU0,        PMB_ZONES_ORION_CPU0        },
        { "orion_cpu1",        PMB_ADDR_ORION_CPU1,        PMB_ZONES_ORION_CPU1        },
        { "orion_cpu2",        PMB_ADDR_ORION_CPU2,        PMB_ZONES_ORION_CPU2        },
        { "orion_cpu3",        PMB_ADDR_ORION_CPU3,        PMB_ZONES_ORION_CPU3        },
        { "orion_noncpu",      PMB_ADDR_ORION_NONCPU,      PMB_ZONES_ORION_NONCPU      },
        { "orion_ars",         PMB_ADDR_ORION_ARS,         PMB_ZONES_ORION_ARS         },
        { "orion_c0_ars",      PMB_ADDR_ORION_C0_ARS,      PMB_ADDR_ORION_C0_ARS       },
        { "biu_pll",           PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL           },
        { "sync_pll",          PMB_ADDR_SYNC_PLL,          PMB_ZONES_SYNC_PLL          },
        { "biu",               PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM          },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL            },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL            },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1             },
        { "pcie2",             PMB_ADDR_PCIE2,             PMB_ZONES_PCIE2             },
        { "pcie3",             PMB_ADDR_PCIE3,             PMB_ZONES_PCIE3             },
        { "wan",               PMB_ADDR_WAN,               PMB_ZONES_WAN               },
        { "xrdp",              PMB_ADDR_XRDP,              PMB_ZONES_XRDP              },
        { "xrdp_rc0",          PMB_ADDR_XRDP_RC0,          PMB_ZONES_XRDP_RC0          },
        { "xrdp_rc1",          PMB_ADDR_XRDP_RC1,          PMB_ZONES_XRDP_RC1          },
        { "xrdp_rc2",          PMB_ADDR_XRDP_RC2,          PMB_ZONES_XRDP_RC2          },
        { "xrdp_rc3",          PMB_ADDR_XRDP_RC3,          PMB_ZONES_XRDP_RC3          },
        { "xrdp_rc4",          PMB_ADDR_XRDP_RC4,          PMB_ZONES_XRDP_RC4          },
        { "xrdp_rc5",          PMB_ADDR_XRDP_RC5,          PMB_ZONES_XRDP_RC5          },
        { "sgmii",             PMB_ADDR_SGMII,             PMB_ZONES_SGMII             },
        { "crypto",            PMB_ADDR_CRYPTO,            PMB_ZONES_CRYPTO            },
        { "usb30_2x",          PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X          },
        { "vdsl3_pmd",         PMB_ADDR_VDSL3_PMD,         PMB_ZONES_VDSL3_PMD         },
        { "vdsl3_mips",        PMB_ADDR_VDSL3_MIPS,        PMB_ZONES_VDSL3_MIPS        },
        { "vdsl3_core",        PMB_ADDR_VDSL3_CORE,        PMB_ZONES_VDSL3_CORE        },
};

static const unsigned int cpu_pmb[] = {
	PMB_ADDR_ORION_CPU0, PMB_ADDR_ORION_CPU1,
	PMB_ADDR_ORION_CPU2, PMB_ADDR_ORION_CPU3,
};

static const pmb_init_t xrdp_pmb[] = {
    /* name      dev                reset value  */
    {"xrdp",     PMB_ADDR_XRDP,     0xfffffff8},
    {"xrdp_rc0", PMB_ADDR_XRDP_RC0, 0xffffffff},
    {"xrdp_rc1", PMB_ADDR_XRDP_RC1, 0xffffffff},
    {"xrdp_rc2", PMB_ADDR_XRDP_RC2, 0xffffffff},
    {"xrdp_rc3", PMB_ADDR_XRDP_RC3, 0xffffffff},
    {"xrdp_rc4", PMB_ADDR_XRDP_RC4, 0xffffffff},
    {"xrdp_rc5", PMB_ADDR_XRDP_RC5, 0xffffffff},
    {"wan",      PMB_ADDR_WAN,      PMB_NO_RESET}
};

#endif
