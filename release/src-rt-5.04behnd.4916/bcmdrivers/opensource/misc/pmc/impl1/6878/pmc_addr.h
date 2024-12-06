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
#define PMB_BUS_ID_SHIFT         12

#define PMB_BUS_PERIPH           0
#define PMB_ADDR_PERIPH          (0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH         3

#define PMB_BUS_CHIP_CLKRST      1
#define PMB_ADDR_CHIP_CLKRST     (1 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST    0

#define PMB_BUS_SYSPLL          1
#define PMB_ADDR_SYSPLL         (3 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL        0

#define PMB_BUS_PVTMON           1
#define PMB_ADDR_PVTMON          (6 | PMB_BUS_PVTMON << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PVTMON         0

#define PMB_BUS_MEMC             1
#define PMB_ADDR_MEMC            (8 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC           1

#define PMB_BUS_USB20_2X         1
#define PMB_ADDR_USB20_2X        (10 | PMB_BUS_USB20_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB20_2X       4

#define PMB_BUS_WAN              1
#define PMB_ADDR_WAN             (11 | PMB_BUS_WAN << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WAN            3

#define PMB_BUS_XRDP              1
#define PMB_ADDR_XRDP             (12 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP            3

#define PMB_BUS_PCIE0             1
#define PMB_ADDR_PCIE0            (18 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0           1

#define PMB_BUS_WLAN0             0
#define PMB_ADDR_WLAN0            (19 | PMB_BUS_WLAN0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WLAN0           1

#define PMB_BUS_ORION_CPU0       1
#define PMB_ADDR_ORION_CPU0      (32 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0     1

#define PMB_BUS_ORION_CPU1       1
#define PMB_ADDR_ORION_CPU1      (33 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1     1

#define PMB_BUS_ORION_NONCPU     1
#define PMB_ADDR_ORION_NONCPU    (36 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU   1

#define PMB_BUS_BIU_PLL          1
#define PMB_ADDR_BIU_PLL         (38 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL        0

#define PMB_BUS_BIU_BPCM         1
#define PMB_ADDR_BIU_BPCM        (39 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM       1

#include "pmc_sysfs.h" 
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "pvtmon",            PMB_ADDR_PVTMON,            PMB_ZONES_PVTMON            },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "usb20",             PMB_ADDR_USB20_2X,          PMB_ZONES_USB20_2X,         },
        { "wan",               PMB_ADDR_WAN,               PMB_ZONES_WAN,              },
        { "xrdp",              PMB_ADDR_XRDP,              PMB_ZONES_XRDP,             },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "wlan0",             PMB_ADDR_WLAN0,             PMB_ZONES_WLAN0             },
        { "orion_cpu0",        PMB_ADDR_ORION_CPU0,        PMB_ZONES_ORION_CPU0        },
        { "orion_cpu1",        PMB_ADDR_ORION_CPU1,        PMB_ZONES_ORION_CPU1        },
        { "orion_noncpu",      PMB_ADDR_ORION_NONCPU,      PMB_ZONES_ORION_NONCPU      },
        { "biu_pll",           PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL           },
        { "biu",               PMB_ADDR_BIU_BPCM,          PMB_ZONES_BIU_BPCM          },
};

static const pmb_init_t xrdp_pmb[] = {
    /* name      dev                reset value  */
    {"xrdp",     PMB_ADDR_XRDP,     0xfffffff8},
    {"wan",      PMB_ADDR_WAN,      PMB_NO_RESET}
};

#endif
