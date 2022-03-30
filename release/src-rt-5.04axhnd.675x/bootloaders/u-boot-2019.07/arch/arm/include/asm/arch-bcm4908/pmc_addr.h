/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef PMC_ADDR_4908_H__
#define PMC_ADDR_4908_H__

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

#define PMB_BUS_B53PLL			0
#define PMB_ADDR_B53PLL			(9 | PMB_BUS_B53PLL << PMB_BUS_ID_SHIFT)
#define PMB_ZONES_B53PLL		0

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

#endif

