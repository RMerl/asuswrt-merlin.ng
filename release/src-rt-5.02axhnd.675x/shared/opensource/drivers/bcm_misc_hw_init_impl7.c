/*
 * <:copyright-BRCM:2016:DUAL/GPL:standard
 * 
 *    Copyright (c) 2016 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

#include "boardparms.h"
#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map.h"
#endif
#include "bcm_map_part.h"
#ifndef _CFE_
#include <linux/module.h>
#endif

/*
 * Do timer clock (using ILP clock) calibration for BCM53573
 */
static void bcm_misc_timerclk_calibration(void)
{
        unsigned int val1, val2, val_sum = 0, val_num = 0, loop_num = 0;
        unsigned int timer_clk;

	/* Configure ALP period, 0x199 = 16384/40 for using 40KHz crystal */
	PMU->slowclkperiod=0x10199;
	PMU->ilp_period=0x10000;

        /* Enable XtalCntrEanble bit, bit[31] of PMU_XtalFreqRatio register */
        PMU->pmu_xtalfreq = 0x80000000;
        val1 = PMU->pmu_xtalfreq & 0x1fff;

        /*
         * Get some valid values of the field AlpPer4Ilp of the above register, and
         * average it as timer clock.
         */
        while (val_num < 20) {
                /* Check next valid value */
                val2 = PMU->pmu_xtalfreq & 0x1fff;
                if (val1 == val2) {
                        if (++loop_num > 5000) {
                                val_sum += val2;
                                val_num++;
                                break;
                        }
                        continue;
                }
                val1 = val2;
                val_sum += val1;
                val_num++;
                loop_num = 0;
        }

        /* Disable XtalCntrEanble bit, bit[31] of PMU_XtalFreqRatio register */
        PMU->pmu_xtalfreq = 0x0;

        val_sum /= val_num;
        timer_clk = (ALP_CLOCK_47189 * 4) / val_sum;
        asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (timer_clk));
return;

}


/* PMU chip control7 register */
#define PMU_CHIPCTL7                            7
#define PMU_CC7_ENABLE_L2REFCLKPAD_PWRDWN       (1 << 25)
#define PMU_CC7_ENABLE_MDIO_RESET_WAR           (1 << 27)
/* 53537 series have gmca1 gmac_if_type in cc7 [7:6](defalut 0b01) */
#define PMU_CC7_IF_TYPE_MASK                    0x000000c0
#define PMU_CC7_IF_TYPE_RMII                    0x00000000
#define PMU_CC7_IF_TYPE_MII                     0x00000040
#define PMU_CC7_IF_TYPE_RGMII                   0x00000080

int bcm_misc_hw_init(void)
{
    /* Enable all GPIOs in 47189/53573. Not all GPIOs are availabe for the chip
     * core after reset. This makes them available.
     *
     * These register writes are done through the GCI Indirect Address
     * register
     */
    GCI->gci_indirect_addr = 0;
    GCI->gci_chipctrl = 0x11111111;
    GCI->gci_indirect_addr = 1;
    GCI->gci_chipctrl = 0x11111111;
    GCI->gci_indirect_addr = 2;
    GCI->gci_chipctrl = 0x11111111;
    GCI->gci_indirect_addr = 3;
    GCI->gci_chipctrl = 0x11111111;

    bcm_misc_timerclk_calibration();

    /* Set GMAC1 to RGMII mode */
    PMU->chipcontrol_addr = PMU_CHIPCTL7;
    PMU->chipcontrol_data &= ~PMU_CC7_IF_TYPE_MASK;
    PMU->chipcontrol_data |= PMU_CC7_IF_TYPE_RGMII;

    /* Set the GMAC1 IRQ ID to OOB #6 (IRQ 38) */
    ENET_CORE1_WRAP->oobselouta30 = 0x86;
    return 0;
}

uint32 pmu_clk(uint8 mdiv_shift)
{
    uint32 xtal_freq = ALP_CLOCK_47189;
    uint32 fvco;
    uint32 mdiv;
    uint32 pdiv;
    uint32 ndiv_nfrac;
    uint32 pll_ctrl_20;
    uint64 res;

    /* Read mdiv, pdiv from pllcontrol[13] */
    PMU->pllcontrol_addr = 13;
    mdiv = (PMU->pllcontrol_data >> mdiv_shift) & 0xff;
    pdiv = (PMU->pllcontrol_data >> PMU_PLL_CTRL_P2DIV_SHIFT) & 0x7;

    /* Read ndiv[29:20], ndiv_frac[19:0] from pllcontrol[14] */
    PMU->pllcontrol_addr = 14;
    ndiv_nfrac = PMU->pllcontrol_data & 0x3ffffff;

    /* Read pll_ctrl_20 from pllcontrol[15] */
    PMU->pllcontrol_addr = 15;
    pll_ctrl_20 = 1 << ((PMU->pllcontrol_data >> 20) & 0x1);

    res = (uint64)ndiv_nfrac * (uint64)xtal_freq * (uint64)pll_ctrl_20;
    fvco = (uint64)res >> 20;

    return (fvco / pdiv / mdiv);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmu_clk);
#endif

