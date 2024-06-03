/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef PMC_ADDR_63158_H__
#define PMC_ADDR_63158_H__

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
#define BPCM_VDSL_PHY_CTL_REG	       global_control  // Alias for register containing DGASP override inside the VDSL PMD
#define BPCM_VDSL_AFE_CTL_REG	       misc_control    // Alias for register containing DGASP configuration inside the VDSL PMD

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
	PCM_Zone_PCM=3
};
//--------- SOFT Reset bits for PCM ------------------------
#define   BPCM_PCM_SRESET_PCM_N       0x00000040

#endif
