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

#define PMB_BUS_MAX			2
#define PMB_BUS_ID_SHIFT		8

#define PMB_BUS_APM			1
#define PMB_ADDR_APM			(0 | PMB_BUS_APM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_APM			5

//--------- SOFT Reset bits for APM ------------------------
#define   BPCM_APM_SRESET_HARDRST_N   0x00000040
#define   BPCM_APM_SRESET_AUDIO_N     0x00000020
#define   BPCM_APM_SRESET_PCM_N       0x00000010
#define   BPCM_APM_SRESET_HVGA_N      0x00000008
#define   BPCM_APM_SRESET_HVGB_N      0x00000004
#define   BPCM_APM_SRESET_BMU_N       0x00000002
#define   BPCM_APM_SRESET_200_N       0x00000001

#define PMB_BUS_SWITCH			1
#define PMB_ADDR_SWITCH			(1 | PMB_BUS_SWITCH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SWITCH		3

#define PMB_BUS_CHIP_CLKRST		1
#define PMB_ADDR_CHIP_CLKRST		(2 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST		0

#define PMB_BUS_SATA			0
#define PMB_ADDR_SATA			(3 | PMB_BUS_SATA << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SATA			1

#define PMB_BUS_AIP			0
#define PMB_ADDR_AIP			(4 | PMB_BUS_AIP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_AIP			1

#define PMB_BUS_DECT_UBUS		0
#define PMB_ADDR_DECT_UBUS		(5 | PMB_BUS_DECT_UBUS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_DECT_UBUS		1

#define PMB_BUS_SAR			1
#define PMB_ADDR_SAR			(6 | PMB_BUS_SAR << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SAR			1

#define PMB_BUS_RDP			1
#define PMB_ADDR_RDP			(7 | PMB_BUS_RDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDP			2

#define PMB_BUS_MEMC			0
#define PMB_ADDR_MEMC			(8 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC			1

#define PMB_BUS_PERIPH			0
#define PMB_ADDR_PERIPH			(9 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH		3

#define PMB_BUS_SYSPLL			1
#define PMB_ADDR_SYSPLL			(10 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL		0

#define PMB_BUS_RDPPLL			1
#define PMB_ADDR_RDPPLL			(11 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL		0

#define PMB_BUS_SYSPLL2			0
#define PMB_ADDR_SYSPLL2		(12 | PMB_BUS_SYSPLL2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL2		0

#define PMB_BUS_SYSPLL3			0
#define PMB_ADDR_SYSPLL3		(13 | PMB_BUS_SYSPLL3 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL3		0

#define PMB_BUS_SYSPLL4			0
#define PMB_ADDR_SYSPLL4		(14 | PMB_BUS_SYSPLL4 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL4		0

#define PMB_BUS_PCIE0			0
#define PMB_ADDR_PCIE0			(15 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0			1

#define PMB_BUS_PCIE1			0
#define PMB_ADDR_PCIE1			(16 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1			1

#define PMB_BUS_USB30_2X		1
#define PMB_ADDR_USB30_2X		(17 | PMB_BUS_USB30_2X << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB30_2X		4

#define PMB_BUS_PSAB			0
#define PMB_ADDR_PSAB			(18 | PMB_BUS_PSAB << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PSAB			1	// not shown in spreadsheet

#define PMB_BUS_PSBC			0
#define PMB_ADDR_PSBC			(19 | PMB_BUS_PSBC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PSBC			1	// not shown in spreadsheet

#define PMB_BUS_EGPHY			0
#define PMB_ADDR_EGPHY			(20 | PMB_BUS_EGPHY << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_EGPHY			1	// not shown in spreadsheet

#define PMB_BUS_VDSL3_MIPS		0
#define PMB_ADDR_VDSL3_MIPS		(21 | PMB_BUS_VDSL3_MIPS << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_VDSL3_MIPS		1

#define PMB_BUS_VDSL3_CORE		0
#define PMB_ADDR_VDSL3_CORE		(22 | PMB_BUS_VDSL3_CORE << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_VDSL3_CORE		3

#define AFEPLL_PMB_BUS_VDSL3_CORE	0
#define AFEPLL_PMB_ADDR_VDSL3_CORE	(23 | AFEPLL_PMB_BUS_VDSL3_CORE << PMB_BUS_ID_SHIFT)
#define AFEPLL_PMB_ZONES_VDSL3_CORE	0

#define UBUS_PMB_BUS_VDSL3_CORE		PMB_BUS_VDSL3_CORE
#define UBUS_PMB_ADDR_VDSL3_CORE	(24 | UBUS_PMB_BUS_VDSL3_CORE << PMB_BUS_ID_SHIFT)
#define UBUS_PMB_ZONES_VDSL3_CORE	0

#define UBUS_PMB_BUS_VDSL3_MIPS		PMB_BUS_VDSL3_MIPS
#define UBUS_PMB_ADDR_VDSL3_MIPS	(25 | UBUS_PMB_BUS_VDSL3_MIPS << PMB_BUS_ID_SHIFT)
#define UBUS_PMB_ZONES_VDSL3_MIPS	0

#define UBUS_PMB_BUS_DECT		PMB_BUS_DECT_UBUS
#define UBUS_PMB_ADDR_DECT		(26 | UBUS_PMB_BUS_DECT << PMB_BUS_ID_SHIFT)
#define UBUS_PMB_ZONES_DECT		0

#define UBUS_PMB_BUS_ARM		PMB_BUS_AIP
#define UBUS_PMB_ADDR_ARM		(27 | UBUS_PMB_BUS_ARM << PMB_BUS_ID_SHIFT)
#define UBUS_PMB_ZONES_ARM		0

#define UBUS_PMB_BUS_DAP		PMB_BUS_AIP
#define UBUS_PMB_ADDR_DAP		(28 | UBUS_PMB_BUS_DAP << PMB_BUS_ID_SHIFT)
#define UBUS_PMB_ZONES_DAP		0

#define UBUS_CFG0_PMB_BUS_SAR		PMB_BUS_SAR
#define UBUS_CFG0_PMB_ADDR_SAR		(29 | UBUS_CFG0_PMB_BUS_SAR << PMB_BUS_ID_SHIFT)
#define UBUS_CFG0_PMB_ZONES_SAR		0

#define UBUS_CFG1_PMB_BUS_SAR		PMB_BUS_SAR
#define UBUS_CFG1_PMB_ADDR_SAR		(30 | UBUS_CFG1_PMB_BUS_SAR << PMB_BUS_ID_SHIFT)
#define UBUS_CFG1_PMB_ZONES_SAR		0

#define UBUS_CFG_PMB_BUS_DBR		PMB_BUS_RDP
#define UBUS_CFG_PMB_ADDR_DBR		(31 | UBUS_CFG_PMB_BUS_DBR << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_DBR		0

#define UBUS_CFG_PMB_BUS_RABR		PMB_BUS_RDP
#define UBUS_CFG_PMB_ADDR_RABR		(32 | UBUS_CFG_PMB_BUS_RABR << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_RABR		0

#define UBUS_CFG_PMB_BUS_RBBR		PMB_BUS_RDP
#define UBUS_CFG_PMB_ADDR_RBBR		(33 | UBUS_CFG_PMB_BUS_RBBR << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_RBBR		0

#define UBUS_CFG_PMB_BUS_APM 		PMB_BUS_APM
#define UBUS_CFG_PMB_ADDR_APM 		(34 | UBUS_CFG_PMB_BUS_APM << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_APM 		0

#define UBUS_CFG_PMB_BUS_PCIE0 		PMB_BUS_PCIE0
#define UBUS_CFG_PMB_ADDR_PCIE0 	(35 | UBUS_CFG_PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_PCIE0 	0

#define UBUS_CFG_PMB_BUS_PCIE1 		PMB_BUS_PCIE1
#define UBUS_CFG_PMB_ADDR_PCIE1 	(36 | UBUS_CFG_PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_PCIE1 	0

#define UBUS_CFG_PMB_BUS_USBH 		PMB_BUS_USB30_2X
#define UBUS_CFG_PMB_ADDR_USBH 		(37 | UBUS_CFG_PMB_BUS_USBH << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_USBH 	0

#define UBUS_CFG_PMB_BUS_USBD 		PMB_BUS_USB30_2X
#define UBUS_CFG_PMB_ADDR_USBD 		(38 | UBUS_CFG_PMB_BUS_USBD << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_USBD 	0

#define UBUS_CFG_PMB_BUS_SWITCH		PMB_BUS_SWITCH
#define UBUS_CFG_PMB_ADDR_SWITCH	(39 | UBUS_CFG_PMB_BUS_SWITCH << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_SWITCH	0

#define UBUS_CFG_PMB_BUS_PERIPH		PMB_BUS_PERIPH
#define UBUS_CFG_PMB_ADDR_PERIPH	(40 | UBUS_CFG_PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_PERIPH	0

#define UBUS_CFG_PMB_BUS_SATA  		PMB_BUS_SATA
#define UBUS_CFG_PMB_ADDR_SATA  	(41 | UBUS_CFG_PMB_BUS_SATA << PMB_BUS_ID_SHIFT)
#define UBUS_CFG_PMB_ZONES_SATA  	0

/* define Zone enum for each block here */
enum {
	APM_Zone_Main,
	APM_Zone_Audio,
	APM_Zone_PCM,
	APM_Zone_HVG,
	APM_Zone_BMU,
};

#include "pmc_sysfs.h"
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
        { "apm",               PMB_ADDR_APM,               PMB_ZONES_APM               },
        { "switch",            PMB_ADDR_SWITCH,            PMB_ZONES_SWITCH            },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "sata",              PMB_ADDR_SATA,              PMB_ZONES_SATA              },
        { "aip",               PMB_ADDR_AIP,               PMB_ZONES_AIP               },
        { "dect_ubus",         PMB_ADDR_DECT_UBUS,         PMB_ZONES_DECT_UBUS         },
        { "sar",               PMB_ADDR_SAR,               PMB_ZONES_SAR               },
        { "rdp",               PMB_ADDR_RDP,               PMB_ZONES_RDP               },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL            },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL            },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1             },
        { "usb30_2x",          PMB_ADDR_USB30_2X,          PMB_ZONES_USB30_2X          },
        { "vdsl3_mips",        PMB_ADDR_VDSL3_MIPS,        PMB_ZONES_VDSL3_MIPS        },
        { "vdsl3_core",        PMB_ADDR_VDSL3_CORE,        PMB_ZONES_VDSL3_CORE        },
        { "vdsl3_afepll",      AFEPLL_PMB_ADDR_VDSL3_CORE, AFEPLL_PMB_ZONES_VDSL3_CORE },
};
#endif
