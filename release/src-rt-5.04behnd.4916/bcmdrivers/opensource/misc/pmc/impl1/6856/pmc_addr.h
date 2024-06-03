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

/* TODO: FIXME Verify the correctness of the bpcms*/

#define PMB_BUS_MAX              2
#define PMB_BUS_ID_SHIFT         12

#define PMB_BUS_PERIPH           0
#define PMB_ADDR_PERIPH          (0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH         4

#define PMB_BUS_MEMC             0
#define PMB_ADDR_MEMC            (1 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC           1

#define PMB_BUS_PVTMON           0
#define PMB_ADDR_PVTMON          (2 | PMB_BUS_PVTMON << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PVTMON         0

#define PMB_BUS_CHIP_CLKRST      0
#define PMB_ADDR_CHIP_CLKRST     (3 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST    0

#define PMB_BUS_USB30_2X         0
#define PMB_ADDR_USB30_2X        (4 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X       4

#define PMB_BUS_SYSPLL           0
#define PMB_ADDR_SYSPLL          (5 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL         0

#define PMB_BUS_RDPPLL           0
#define PMB_ADDR_RDPPLL          (6 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL         0

#define PMB_BUS_PCIE0            1
#define PMB_ADDR_PCIE0           (7 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0          1

#define PMB_BUS_PCIE1            1
#define PMB_ADDR_PCIE1           (8 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1          1

#define PMB_BUS_PCIE2            1
#define PMB_ADDR_PCIE2           (9 | PMB_BUS_PCIE2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE2          1

#define PMB_BUS_XRDP             1
#define PMB_ADDR_XRDP            (10 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP           3

#define PMB_BUS_XRDP_RC0         1
#define PMB_ADDR_XRDP_RC0        (11 | PMB_BUS_XRDP_RC0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC0       1

#define PMB_BUS_XRDP_RC1         1
#define PMB_ADDR_XRDP_RC1        (12 | PMB_BUS_XRDP_RC1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC1       1

#define PMB_BUS_XRDP_RC2         1
#define PMB_ADDR_XRDP_RC2        (13 | PMB_BUS_XRDP_RC2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC2       1

#define PMB_BUS_XRDP_RC3         1
#define PMB_ADDR_XRDP_RC3        (14 | PMB_BUS_XRDP_RC3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC3       1

#define PMB_BUS_XRDP_RC4         1
#define PMB_ADDR_XRDP_RC4        (15 | PMB_BUS_XRDP_RC4 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC4       1

#define PMB_BUS_XRDP_RC5         1
#define PMB_ADDR_XRDP_RC5        (16 | PMB_BUS_XRDP_RC5 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC5       1

#define PMB_BUS_XRDP_RC6         1
#define PMB_ADDR_XRDP_RC6        (17 | PMB_BUS_XRDP_RC6 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC6       1

#define PMB_BUS_XRDP_RC7         1
#define PMB_ADDR_XRDP_RC7        (18 | PMB_BUS_XRDP_RC7 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC7       1

#define PMB_BUS_WAN              1
#define PMB_ADDR_WAN             (19 | PMB_BUS_WAN << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WAN            6

#define PMB_BUS_ORION_CPU0         0
#define PMB_ADDR_ORION_CPU0        (32 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0       1

#define PMB_BUS_ORION_CPU1         0
#define PMB_ADDR_ORION_CPU1        (33 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1       1

#define PMB_BUS_ORION_NONCPU       0
#define PMB_ADDR_ORION_NONCPU      (36 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU     1

#define PMB_BUS_BIU_PLL            0
#define PMB_ADDR_BIU_PLL           (38 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL          1

#define PMB_BUS_BIU_BPCM           0
#define PMB_ADDR_BIU_BPCM          (39 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM         1

#include "pmc_sysfs.h" 
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones           ucbid           */
        { "periph",      PMB_ADDR_PERIPH,      PMB_ZONES_PERIPH,        0                     },
        { "chip_clkrst", PMB_ADDR_CHIP_CLKRST, PMB_ZONES_CHIP_CLKRST,   0                     },
        { "syspll",      PMB_ADDR_SYSPLL,      PMB_ZONES_SYSPLL,        0                     },
        { "rdppll",      PMB_ADDR_RDPPLL,      PMB_ZONES_RDPPLL,        0                     },
        { "memc",        PMB_ADDR_MEMC,        PMB_ZONES_MEMC,          UCB_NODE_ID_SLV_MEMC  },
        { "usb30",       PMB_ADDR_USB30_2X,    PMB_ZONES_USB30_2X,      UCB_NODE_ID_MST_USB | 
                                                                        UCB_HAS_MST_SLV       },
        { "wan",         PMB_ADDR_WAN,         PMB_ZONES_WAN,           0                     },
        { "xrdp",        PMB_ADDR_XRDP,        PMB_ZONES_XRDP,          0                     },
        { "xrdp_rc0",    PMB_ADDR_XRDP_RC0,    PMB_ZONES_XRDP_RC0,      0                     },
        { "xrdp_rc1",    PMB_ADDR_XRDP_RC1,    PMB_ZONES_XRDP_RC1,      0                     },
        { "xrdp_rc2",    PMB_ADDR_XRDP_RC2,    PMB_ZONES_XRDP_RC2,      0                     },
        { "xrdp_rc3",    PMB_ADDR_XRDP_RC3,    PMB_ZONES_XRDP_RC3,      0                     },
        { "xrdp_rc4",    PMB_ADDR_XRDP_RC4,    PMB_ZONES_XRDP_RC4,      0                     },
        { "xrdp_rc5",    PMB_ADDR_XRDP_RC5,    PMB_ZONES_XRDP_RC5,      0                     },
        { "xrdp_rc6",    PMB_ADDR_XRDP_RC6,    PMB_ZONES_XRDP_RC6,      0                     },
        { "xrdp_rc7",    PMB_ADDR_XRDP_RC7,    PMB_ZONES_XRDP_RC7,      0                     },
        { "pcie0",       PMB_ADDR_PCIE0,       PMB_ZONES_PCIE0,         0                     },
        { "pcie1",       PMB_ADDR_PCIE1,       PMB_ZONES_PCIE1,         0                     },
        { "pcie2",       PMB_ADDR_PCIE2,       PMB_ZONES_PCIE2,         0                     },
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
    {"xrdp_rc6", PMB_ADDR_XRDP_RC6, 0xffffffff},
    {"xrdp_rc7", PMB_ADDR_XRDP_RC7, 0xffffffff},
    {"wan",      PMB_ADDR_WAN,      PMB_NO_RESET}
};

#endif
