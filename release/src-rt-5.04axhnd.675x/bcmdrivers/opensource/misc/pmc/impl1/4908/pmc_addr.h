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

#define PMB_BUS_PERIPH			0
#define PMB_ADDR_PERIPH			(0 | PMB_BUS_PERIPH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PERIPH		4

#define PMB_BUS_CRYPTO                  0
#define PMB_ADDR_CRYPTO                 (1 | PMB_BUS_CRYPTO << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CRYPTO                1

#define PMB_BUS_PCIE2			0
#define PMB_ADDR_PCIE2			(2 | PMB_BUS_PCIE2 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE2			1

#define PMB_BUS_RDP			0
#define PMB_ADDR_RDP			(3 | PMB_BUS_RDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDP			2

#define PMB_BUS_FPM			0
#define PMB_ADDR_FPM			(4 | PMB_BUS_RDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_FPM			1

#define PMB_BUS_DQM			0
#define PMB_ADDR_DQM			(5 | PMB_BUS_RDP << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_DQM			1

#define PMB_BUS_URB			0
#define PMB_ADDR_URB			(6 | PMB_BUS_URB << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_URB			1

#define PMB_BUS_MEMC			0
#define PMB_ADDR_MEMC			(7 | PMB_BUS_MEMC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_MEMC			1

#define PMB_BUS_RDPPLL			0
#define PMB_ADDR_RDPPLL			(8 | PMB_BUS_RDPPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_RDPPLL		0

#define PMB_BUS_BIU_PLL			0
#define PMB_ADDR_BIU_PLL     	(9 | PMB_BUS_BIU_PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_BIU_PLL		0

#define PMB_BUS_SWITCH			1
#define PMB_ADDR_SWITCH			(10 | PMB_BUS_SWITCH << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SWITCH		3

#define PMB_BUS_PCM			1
#define PMB_ADDR_PCM			(11 | PMB_BUS_PCM << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCM			2
enum {
	PCM_Zone_Main,
	PCM_Zone_PCM,
};
//--------- SOFT Reset bits for PCM ------------------------
#define   BPCM_PCM_SRESET_HARDRST_N   0x00000004
#define   BPCM_PCM_SRESET_PCM_N       0x00000002
#define   BPCM_PCM_SRESET_200_N       0x00000001

#define PMB_BUS_SGMII  			1
#define PMB_ADDR_SGMII			(12 | PMB_BUS_SGMII << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SGMII			0

#define PMB_BUS_CHIP_CLKRST		1
#define PMB_ADDR_CHIP_CLKRST		(13 | PMB_BUS_CHIP_CLKRST << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_CHIP_CLKRST		0

#define PMB_BUS_PCIE0			1
#define PMB_ADDR_PCIE0			(14 | PMB_BUS_PCIE0 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE0			1

#define PMB_BUS_PCIE1			1
#define PMB_ADDR_PCIE1			(15 | PMB_BUS_PCIE1 << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_PCIE1			1

#define PMB_BUS_SATA			1
#define PMB_ADDR_SATA			(16 | PMB_BUS_SATA << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SATA			1

#define PMB_BUS_USB				1
#define PMB_ADDR_USB30_2X		(17 | PMB_BUS_USB << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_USB			4

#define PMB_BUS_SYSPLL			1
#define PMB_ADDR_SYSPLL			(18 | PMB_BUS_SYSPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SYSPLL		0

#define PMB_BUS_SWTPLL			1
#define PMB_ADDR_SWTPLL			(19 | PMB_BUS_SWTPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_SWTPLL		0

#define PMB_BUS_I2SPLL			1
#define PMB_ADDR_I2SPLL			(20 | PMB_BUS_I2SPLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_I2SPLL		0

#define PMB_BUS_GMAC			1
#define PMB_ADDR_GMAC			(21 | PMB_BUS_GMAC << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_GMAC			1

#include "pmc_sysfs.h"
static const struct bpcm_device bpcm_devs[] = {
        /* name                dev                         zones                      */
        { "b53pll",            PMB_ADDR_BIU_PLL,           PMB_ZONES_BIU_PLL           },
        { "chip_clkrst",       PMB_ADDR_CHIP_CLKRST,       PMB_ZONES_CHIP_CLKRST       },
        { "crypto",            PMB_ADDR_CRYPTO,            PMB_ZONES_CRYPTO            },
        { "dqm",               PMB_ADDR_DQM,               PMB_ZONES_DQM               },
        { "fpm",               PMB_ADDR_FPM,               PMB_ZONES_FPM               },
        { "gmac",              PMB_ADDR_GMAC,              PMB_ZONES_GMAC              },
        { "i2spll",            PMB_ADDR_I2SPLL,            PMB_ZONES_I2SPLL            },
        { "memc",              PMB_ADDR_MEMC,              PMB_ZONES_MEMC              },
        { "pcie0",             PMB_ADDR_PCIE0,             PMB_ZONES_PCIE0             },
        { "pcie1",             PMB_ADDR_PCIE1,             PMB_ZONES_PCIE1             },
        { "pcie2",             PMB_ADDR_PCIE2,             PMB_ZONES_PCIE2             },
        { "pcm",               PMB_ADDR_PCM,               PMB_ZONES_PCM               },
        { "periph",            PMB_ADDR_PERIPH,            PMB_ZONES_PERIPH            },
        { "rdp",               PMB_ADDR_RDP,               PMB_ZONES_RDP               },
        { "rdppll",            PMB_ADDR_RDPPLL,            PMB_ZONES_RDPPLL            },
        { "sata",              PMB_ADDR_SATA,              PMB_ZONES_SATA              },
        { "sgmii",             PMB_ADDR_SGMII,             PMB_ZONES_SGMII             },
        { "switch",            PMB_ADDR_SWITCH,            PMB_ZONES_SWITCH            },
        { "swtpll",            PMB_ADDR_SWTPLL,            PMB_ZONES_SWTPLL            },
        { "syspll",            PMB_ADDR_SYSPLL,            PMB_ZONES_SYSPLL            },
        { "urb",               PMB_ADDR_URB,               PMB_ZONES_URB               },
        { "usb",               PMB_ADDR_USB30_2X,          PMB_ZONES_USB               },
};
#endif
