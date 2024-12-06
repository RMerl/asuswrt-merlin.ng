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

#define PMB_BUS_PERIPH           0
#define PMB_ADDR_PERIPH          (0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH         3

#define PMB_BUS_CHIP_CLKRST      0
#define PMB_ADDR_CHIP_CLKRST     (1 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST    0

#define PMB_BUS_SYSPLL           0
#define PMB_ADDR_SYSPLL          (2 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL         0

#define PMB_BUS_RDPPLL           0
#define PMB_ADDR_RDPPLL          (3 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL         0

#define PMB_BUS_UNIPLL           0
#define PMB_ADDR_UNIPLL          (5 | PMB_BUS_UNIPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_UNIPLL         0

#define PMB_BUS_CRYPTO           1
#define PMB_ADDR_CRYPTO          (6 | PMB_BUS_CRYPTO << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CRYPTO         0

#define PMB_BUS_APM              0
#define PMB_ADDR_APM             (7 | PMB_BUS_APM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_APM            2

#define PMB_BUS_MEMC             0
#define PMB_ADDR_MEMC            (8 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC           1

#define PMB_BUS_LPORT            1
#define PMB_ADDR_LPORT           (9 | PMB_BUS_LPORT << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_LPORT          3

#define PMB_BUS_USB30_2X         1
#define PMB_ADDR_USB30_2X        (10 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X       4

#define PMB_BUS_WAN              1
#define PMB_ADDR_WAN             (11 | PMB_BUS_WAN << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_WAN            7

#define PMB_BUS_XRDP              1
#define PMB_ADDR_XRDP             (12 | PMB_BUS_XRDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP            3

#define PMB_BUS_XRDP_QM           1
#define PMB_ADDR_XRDP_QM          (13 | PMB_BUS_XRDP_QM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_QM         1

#define PMB_BUS_XRDP_RC_QUAD0     1
#define PMB_ADDR_XRDP_RC_QUAD0    (14 | PMB_BUS_XRDP_RC_QUAD0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC_QUAD0   1

#define PMB_BUS_XRDP_RC_QUAD1     1
#define PMB_ADDR_XRDP_RC_QUAD1    (15 | PMB_BUS_XRDP_RC_QUAD1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC_QUAD1   1

#define PMB_BUS_XRDP_RC_QUAD2     1
#define PMB_ADDR_XRDP_RC_QUAD2    (16 | PMB_BUS_XRDP_RC_QUAD2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC_QUAD2   1

#define PMB_BUS_XRDP_RC_QUAD3     1
#define PMB_ADDR_XRDP_RC_QUAD3    (17 | PMB_BUS_XRDP_RC_QUAD3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_XRDP_RC_QUAD3   1

#define PMB_BUS_PCIE0              1
#define PMB_ADDR_PCIE0             (18 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0            1

#define PMB_BUS_PCIE1              1
#define PMB_ADDR_PCIE1             (19 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1            1

#define PMB_BUS_SATA               1
#define PMB_ADDR_SATA             (20 | PMB_BUS_SATA << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SATA             1

#define PMB_BUS_PCIE_UBUS          1
#define PMB_ADDR_PCIE_UBUS         (21 | PMB_BUS_PCIE_UBUS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE_UBUS        1

#define PMB_BUS_ORION_CPU0         0
#define PMB_ADDR_ORION_CPU0        (24 | PMB_BUS_ORION_CPU0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU0       1

#define PMB_BUS_ORION_CPU1         0
#define PMB_ADDR_ORION_CPU1        (25 | PMB_BUS_ORION_CPU1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU1       1

#define PMB_BUS_ORION_CPU2         0
#define PMB_ADDR_ORION_CPU2        (26 | PMB_BUS_ORION_CPU2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU2       1

#define PMB_BUS_ORION_CPU3         0
#define PMB_ADDR_ORION_CPU3        (27 | PMB_BUS_ORION_CPU3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_CPU3       1

#define PMB_BUS_ORION_NONCPU       0
#define PMB_ADDR_ORION_NONCPU      (28 | PMB_BUS_ORION_NONCPU << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_NONCPU     1

#define PMB_BUS_ORION_ARS          0
#define PMB_ADDR_ORION_ARS         (29 | PMB_BUS_ORION_ARS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_ORION_ARS        1

#define PMB_BUS_BIU_PLL            0
#define PMB_ADDR_BIU_PLL           (30 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL          1	// FIXMET

#define PMB_BUS_BIU_BPCM           0
#define PMB_ADDR_BIU_BPCM          (31 | PMB_BUS_BIU_BPCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_BPCM         1

#define PMB_BUS_PCM                0

#define PMB_ADDR_PCM               (0 | PMB_BUS_PCM << PMB_BUS_ID_SHIFT)

#define PMB_ZONES_PCM              2
enum {
	PCM_Zone_Main,
	PCM_Zone_PCM = 3,
};
//--------- SOFT Reset bits for PCM ------------------------
#define   BPCM_PCM_SRESET_HARDRST_N   0x00000004

#define   BPCM_PCM_SRESET_PCM_N       0x00000040

#define   BPCM_PCM_SRESET_BUS_N       0x00000001

#include "pmc_sysfs.h" 
static const struct bpcm_device bpcm_devs[] = {
        /* name            dev                     zones                    ucbid                  */
        { "periph",        PMB_ADDR_PERIPH,        PMB_ZONES_PERIPH,        0                       },
        { "chip_clkrst",   PMB_ADDR_CHIP_CLKRST,   PMB_ZONES_CHIP_CLKRST,   0                       },
        { "syspll",        PMB_ADDR_SYSPLL,        PMB_ZONES_SYSPLL,        0                       },
        { "rdppll",        PMB_ADDR_RDPPLL,        PMB_ZONES_RDPPLL,        0                       },
        { "unipll",        PMB_ADDR_UNIPLL,        PMB_ZONES_UNIPLL,        0                       },
        { "lport",         PMB_ADDR_LPORT,         PMB_ZONES_LPORT,         UCB_NODE_ID_SLV_LPORT   },
        { "usb30",         PMB_ADDR_USB30_2X,      PMB_ZONES_USB30_2X,      UCB_NODE_ID_MST_USB |  
                                                                            UCB_HAS_MST_SLV         },
        { "wan",           PMB_ADDR_WAN,           PMB_ZONES_WAN,           UCB_NODE_ID_SLV_WAN     },
        { "xrdp",          PMB_ADDR_XRDP,          PMB_ZONES_XRDP,          0                       },
        { "xrdp_qm",       PMB_ADDR_XRDP_QM,       PMB_ZONES_XRDP_QM,       0                       },
        { "xrdp_rc_quad0", PMB_ADDR_XRDP_RC_QUAD0, PMB_ZONES_XRDP_RC_QUAD0, 0                       },
        { "xrdp_rc_quad1", PMB_ADDR_XRDP_RC_QUAD1, PMB_ZONES_XRDP_RC_QUAD1, 0                       },
        { "xrdp_rc_quad2", PMB_ADDR_XRDP_RC_QUAD2, PMB_ZONES_XRDP_RC_QUAD2, 0                       },
        { "xrdp_rc_quad3", PMB_ADDR_XRDP_RC_QUAD3, PMB_ZONES_XRDP_RC_QUAD3, 0                       },
        { "pcie0",         PMB_ADDR_PCIE0,         PMB_ZONES_PCIE0,         UCB_NODE_ID_MST_PCIE0 | 
                                                                            UCB_HAS_MST_SLV         },
        { "sata",          PMB_ADDR_SATA,          PMB_ZONES_SATA,          UCB_NODE_ID_MST_SATA | 
                                                                            UCB_HAS_MST_SLV         },
        { "pcie_ubus",     PMB_ADDR_PCIE_UBUS,     PMB_ZONES_PCIE_UBUS,     UCB_NODE_ID_MST_PCIE2 | 
                                                                            UCB_HAS_MST_SLV         },
        { "orion_cpu0",    PMB_ADDR_ORION_CPU0,    PMB_ZONES_ORION_CPU0,    0                       },
        { "orion_cpu1",    PMB_ADDR_ORION_CPU1,    PMB_ZONES_ORION_CPU1,    0                       },
        { "orion_cpu2",    PMB_ADDR_ORION_CPU2,    PMB_ZONES_ORION_CPU2,    0                       },
        { "orion_cpu3",    PMB_ADDR_ORION_CPU3,    PMB_ZONES_ORION_CPU3,    0                       },
        { "orion_noncpu",  PMB_ADDR_ORION_NONCPU,  PMB_ZONES_ORION_NONCPU,  0                       },
        { "orion_ars",     PMB_ADDR_ORION_ARS ,    PMB_ZONES_ORION_ARS,     0                       },
        { "biupll",        PMB_ADDR_BIU_PLL,       PMB_ZONES_BIU_PLL,       0                       },
        { "biu",           PMB_ADDR_BIU_BPCM,      PMB_ZONES_BIU_BPCM,      0                       },
};

static const unsigned int cpu_pmb[] = {
	PMB_ADDR_ORION_CPU0, PMB_ADDR_ORION_CPU1,
	PMB_ADDR_ORION_CPU2, PMB_ADDR_ORION_CPU3,
};

static const pmb_init_t xrdp_pmb[] = {
    /* name           dev                    reset value  */
    {"xrdp",          PMB_ADDR_XRDP,          0xff},
    {"xrdp_qm",       PMB_ADDR_XRDP_QM,       0xff}, 
    {"xrdp_rc_quad0", PMB_ADDR_XRDP_RC_QUAD0, 0xff},
    {"xrdp_rc_quad1", PMB_ADDR_XRDP_RC_QUAD1, 0xff},
    {"xrdp_rc_quad2", PMB_ADDR_XRDP_RC_QUAD2, 0xff},
    {"xrdp_rc_quad3", PMB_ADDR_XRDP_RC_QUAD3, 0xff}
};

#endif
