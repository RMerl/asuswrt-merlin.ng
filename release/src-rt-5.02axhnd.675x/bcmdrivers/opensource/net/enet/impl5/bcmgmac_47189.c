/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
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


//**************************************************************************
// File Name  : bcmgmac.c
//
// Description: This is Linux network driver for Broadcom GMAC controller
//
//**************************************************************************

#define VERSION     "0.1"
#define VER_STR     "v" VERSION

#define _BCMENET_LOCAL_

#include <linux/types.h>
#include <linux/bcm_log_mod.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>
#include "bcm_misc_hw_init.h"

#include "bcmgmacctl.h"
#include "bcmmii.h"
#include "bcm_gpio.h"
#include "bcm_bbsi.h"
#include "bcmgmac_47189.h"
#include "ethsw_phy.h"
#include <bcm_pkt_lengths.h>


#define GMAC_RESET_DELAY        2

/* PMU clock/power control */
#define PMUCTL_ENAB                     (MISC->capabilities & CC_CAP_PMU)

/* 53537 series moved switch_type and gmac_if_type to CC4 [15:14] and [13:12] */
#define PMU_CC4_IF_TYPE_MASK            0x00003000
#define PMU_CC4_IF_TYPE_RMII            0x00000000
#define PMU_CC4_IF_TYPE_MII             0x00001000
#define PMU_CC4_IF_TYPE_RGMII           0x00002000

#define PMU_CC4_SW_TYPE_MASK            0x0000c000
#define PMU_CC4_SW_TYPE_EPHY            0x00000000
#define PMU_CC4_SW_TYPE_EPHYMII         0x00004000
#define PMU_CC4_SW_TYPE_EPHYRMII        0x00008000

/* PMU chip control4 register */
#define PMU_CHIPCTL4                    4
#define PMU_CC4_SW_TYPE_RGMII           0x0000c000

/*
 * Spin at most 'us' microseconds while 'exp' is true.
 * Caller should explicitly test 'exp' when this completes
 * and take appropriate error action if 'exp' is still true.
 */
#define SPINWAIT_POLL_PERIOD	10

#define SPINWAIT(exp, us) { \
        uint32 countdown = (us) + (SPINWAIT_POLL_PERIOD - 1); \
        while ((exp) && (countdown >= SPINWAIT_POLL_PERIOD)) { \
                udelay(SPINWAIT_POLL_PERIOD); \
                countdown -= SPINWAIT_POLL_PERIOD; \
        } \
}

static unsigned int gmac_port_map = 0;

/* Helper functions */
static inline volatile EnetCoreMisc* gmac_misc_regs(int ethcore)
{
    if (ethcore == 0) {
        return ENET_CORE0_MISC;
    } else if (ethcore == 1) {
        return ENET_CORE1_MISC;
    } else {
        printk("Fatal error: Ethernet core %d doesn't exist\n", ethcore);
        /* Loop here forever */
        while (1) ;
    }

    /* Never reached */
    return 0;
}

inline volatile EnetCoreMib* gmac_mib_regs(int ethcore)
{
    if (ethcore == 0) {
        return ENET_CORE0_MIB;
    } else if (ethcore == 1) {
        return ENET_CORE1_MIB;
    } else {
        printk("Fatal error: Ethernet core %d doesn't exist\n", ethcore);
        /* Loop here forever */
        while (1) ;
    }

    /* Never reached */
    return 0;
}

static inline volatile EnetCoreUnimac* gmac_unimac_regs(int ethcore)
{
    if (ethcore == 0) {
        return ENET_CORE0_UNIMAC;
    } else if (ethcore == 1) {
        return ENET_CORE1_UNIMAC;
    } else {
        printk("Fatal error: Ethernet core %d doesn't exist\n", ethcore);
        /* Loop here forever */
        while (1) ;
    }

    /* Never reached */
    return 0;
}

static void ether_config_gphy(int phy_id)
{
    uint16_t data;
    /* disable RGMII RXD to RXC Skew mode,shadow register */
    data = 0xf0e7;
    ethsw_phy_wreg(phy_id, 0x18, &data);
    /* bypass (GTXCLK) delay */
    data = 0x8c00;
    ethsw_phy_wreg(phy_id, 0x1c, &data);
}

static int ether_gphy_reset(int dmaPort)
{
    uint16_t reset;

    /* set gpio pad to floating state */
    GPIO->gpiopullup = 0;
    GPIO->gpiopulldown = 0;

    /* reset the external phy */
    if (BpGetPhyResetGpio(0, dmaPort, &reset) != BP_SUCCESS ||
        reset == BP_GPIO_NONE)
    {
        printk(KERN_EMERG "Phy reset gpio not found\n");
        return -1;
    }

    /* keep RESET low for 2 us */
    bcm_gpio_set_data(reset, 0);
    bcm_gpio_set_dir(reset, GPIO_OUT);
    udelay(2);

    /* keep RESET high for at least 2 us */
    bcm_gpio_set_data(reset, 1);
    udelay(2);

    return 0;
}

void ethercore_enable(volatile Aidmp *wrap)
{
    int loop_counter = 10;

    /* Put core into reset state */
    wrap->resetctrl = AIRC_RESET;
    udelay(1000);

    /* Ensure there are no pending backplane operations */
    SPINWAIT(wrap->resetstatus, 300);

    wrap->ioctrl = SICF_FGC | SICF_CLOCK_EN;

    /* Ensure there are no pending backplane operations */
    SPINWAIT(wrap->resetstatus, 300);

    while (wrap->resetctrl != 0 && --loop_counter) {
        SPINWAIT(wrap->resetstatus, 300);
        /* Take core out of reset */
        wrap->resetctrl = 0;
        SPINWAIT(wrap->resetstatus, 300);
    }

    wrap->ioctrl = SICF_CLOCK_EN;
    udelay(1000);
}

/*
 * Configures the Ethernet core clock (from PMU)
 */
static void ethercore_clock_init(int ethcore)
{
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(ethcore);

    /* set gmac into loopback mode to ensure no rx traffic */
    //gmac_macloopback(TRUE);
    //udelay(1);

    /* ethernet clock is generated by the PMU */
    misc_regs->clk_ctl_st |= CS_ER;
    SPINWAIT((misc_regs->clk_ctl_st & CS_ES) != CS_ES, 1000);

    /* configure gmac and switch data for PMU */
    if (PMUCTL_ENAB) {
        PMU->chipcontrol_addr = PMU_CHIPCTL4;
        PMU->chipcontrol_data &= ~(PMU_CC4_IF_TYPE_MASK | PMU_CC4_SW_TYPE_MASK);
        PMU->chipcontrol_data |= PMU_CC4_IF_TYPE_RGMII | PMU_CC4_SW_TYPE_RGMII;
    }

    /* set phy control: set smi_master to drive mdc_clk */
    misc_regs->phycontrol |= PC_MTE;

    /* Read the devstatus to figure out the configuration mode of
     * the interface. Set the speed to 100 if the switch interface
     * is mii/rmii. We know that we have rgmii, just maintained for
     * completeness.
     */
    /* NOT REALLY NECESSARY, REMOVE */
    //gmac_miiconfig();
}

static void unimac_init_reset(volatile EnetCoreUnimac *unimac_regs)
{
    /* put mac in software reset */
    unimac_regs->cmdcfg |= CC_SR;
    udelay(GMAC_RESET_DELAY);
}



static void unimac_clear_reset(volatile EnetCoreUnimac *unimac_regs)
{
    /* bring mac out of software reset */
    unimac_regs->cmdcfg &= ~CC_SR;
    udelay(GMAC_RESET_DELAY);
}

static void unimac_flowcontrol(volatile EnetCoreUnimac *unimac_regs,
                             bool tx_flowctrl, bool rx_flowctrl)
{
    uint32 cmdcfg;

    cmdcfg = unimac_regs->cmdcfg;

    /* put the mac in reset */
    unimac_init_reset(unimac_regs);

    /* to enable tx flow control clear the rx pause ignore bit */
    if (tx_flowctrl)
        cmdcfg &= ~CC_RPI;
    else
        cmdcfg |= CC_RPI;

    /* to enable rx flow control clear the tx pause transmit ignore bit */
    if (rx_flowctrl)
        cmdcfg &= ~CC_TPI;
    else
        cmdcfg |= CC_TPI;

    unimac_regs->cmdcfg = cmdcfg;

    /* bring mac out of reset */
    unimac_clear_reset(unimac_regs);
}


static void unimac_promisc(volatile EnetCoreUnimac *unimac_regs, int mode)
{
    uint32 cmdcfg;

    cmdcfg = unimac_regs->cmdcfg;

    /* put the mac in reset */
    unimac_init_reset(unimac_regs);

    /* enable or disable promiscuous mode */
    if (mode)
        cmdcfg |= CC_PROM;
    else
        cmdcfg &= ~CC_PROM;

    unimac_regs->cmdcfg = cmdcfg;

    /* bring mac out of reset */
    unimac_clear_reset(unimac_regs);
}

void gmac_reset_mib(int ethcore)
{
    volatile EnetCoreMib *mib_regs = gmac_mib_regs(ethcore);
    volatile uint32_t *p;

    /*
     * mib_regs->tx_good_octets is the first counter, mib_regs->rx_uni_pkts is
     * the last one and the register space between them is filled with the rest
     * of the 32-bit counters.
     * Watch out for the 32-bit gap after mib_regs->tx_q3_octets_high!
     */
    for (p = &mib_regs->tx_good_octets; p <= &mib_regs->rx_uni_pkts; p++) {
        *p = 0;
        if (p == &mib_regs->tx_q3_octets_high) {
            /* Skip a hole in the register space to avoid a bus error */
            p++;
        }
    }
}

void gmac_dump_mib(int ethcore, int type)
{
    volatile EnetCoreMib *mib_regs = gmac_mib_regs(ethcore);

    printk("\n");

    printk("tx_good_pkts:                 %10u\n", mib_regs->tx_good_pkts);
    printk("tx_broadcast_pkts:            %10u\n", mib_regs->tx_broadcast_pkts);
    printk("tx_multicast_pkts:            %10u\n", mib_regs->tx_multicast_pkts);

    if(type) {
        printk("tx_good_octets:               %10u\n", mib_regs->tx_good_octets);
        printk("tx_good_octets_high:          %10u\n", mib_regs->tx_good_octets_high);
        printk("tx_octets:                    %10u\n", mib_regs->tx_octets);
        printk("tx_octets_high:               %10u\n", mib_regs->tx_octets_high);
        printk("tx_pkts:                      %10u\n", mib_regs->tx_pkts);
        printk("tx_len_64:                    %10u\n", mib_regs->tx_len_64);
        printk("tx_len_65_to_127:             %10u\n", mib_regs->tx_len_65_to_127);
        printk("tx_len_128_to_255:            %10u\n", mib_regs->tx_len_128_to_255);
        printk("tx_len_256_to_511:            %10u\n", mib_regs->tx_len_256_to_511);
        printk("tx_len_512_to_1023:           %10u\n", mib_regs->tx_len_512_to_1023);
        printk("tx_len_1024_to_1522:          %10u\n", mib_regs->tx_len_1024_to_1522);
        printk("tx_len_1523_to_2047:          %10u\n", mib_regs->tx_len_1523_to_2047);
        printk("tx_len_2048_to_4095:          %10u\n", mib_regs->tx_len_2048_to_4095);
        printk("tx_len_4095_to_8191:          %10u\n", mib_regs->tx_len_4095_to_8191);
        printk("tx_len_8192_to_max:           %10u\n", mib_regs->tx_len_8192_to_max);
        printk("tx_jabber_pkts:               %10u\n", mib_regs->tx_jabber_pkts);
        printk("tx_oversize_pkts:             %10u\n", mib_regs->tx_oversize_pkts);
        printk("tx_fragment_pkts:             %10u\n", mib_regs->tx_fragment_pkts);
        printk("tx_underruns:                 %10u\n", mib_regs->tx_underruns);
        printk("tx_total_cols:                %10u\n", mib_regs->tx_total_cols);
        printk("tx_single_cols:               %10u\n", mib_regs->tx_single_cols);
        printk("tx_multiple_cols:             %10u\n", mib_regs->tx_multiple_cols);
        printk("tx_excessive_cols:            %10u\n", mib_regs->tx_excessive_cols);
        printk("tx_late_cols:                 %10u\n", mib_regs->tx_late_cols);
        printk("tx_defered:                   %10u\n", mib_regs->tx_defered);
        printk("tx_carrier_lost:              %10u\n", mib_regs->tx_carrier_lost);
        printk("tx_pause_pkts:                %10u\n", mib_regs->tx_pause_pkts);
        printk("tx_uni_pkts:                  %10u\n", mib_regs->tx_uni_pkts);
        printk("tx_q0_pkts:                   %10u\n", mib_regs->tx_q0_pkts);
        printk("tx_q0_octets:                 %10u\n", mib_regs->tx_q0_octets);
        printk("tx_q0_octets_high:            %10u\n", mib_regs->tx_q0_octets_high);
        printk("tx_q1_pkts:                   %10u\n", mib_regs->tx_q1_pkts);
        printk("tx_q1_octets:                 %10u\n", mib_regs->tx_q1_octets);
        printk("tx_q1_octets_high:            %10u\n", mib_regs->tx_q1_octets_high);
        printk("tx_q2_pkts:                   %10u\n", mib_regs->tx_q2_pkts);
        printk("tx_q2_octets:                 %10u\n", mib_regs->tx_q2_octets);
        printk("tx_q2_octets_high:            %10u\n", mib_regs->tx_q2_octets_high);
        printk("tx_q3_pkts:                   %10u\n", mib_regs->tx_q3_pkts);
        printk("tx_q3_octets:                 %10u\n", mib_regs->tx_q3_octets);        
	    printk("tx_q3_octets_high:            %10u\n", mib_regs->tx_q3_octets_high);
    }

    // Rx
    printk("rx_good_pkts:                 %10u\n", mib_regs->rx_good_pkts);
    printk("rx_broadcast_pkts:            %10u\n", mib_regs->rx_broadcast_pkts);
    printk("rx_multicast_pkts:            %10u\n", mib_regs->rx_multicast_pkts);

    if(type) {
        printk("rx_good_octets:               %10u\n", mib_regs->rx_good_octets);
        printk("rx_good_octets_high:          %10u\n", mib_regs->rx_good_octets_high);
        printk("rx_octets:                    %10u\n", mib_regs->rx_octets);
        printk("rx_octets_high:               %10u\n", mib_regs->rx_octets_high);
        printk("rx_pkts:                      %10u\n", mib_regs->rx_pkts);
        printk("rx_len_64:                    %10u\n", mib_regs->rx_len_64);
        printk("rx_len_65_to_127:             %10u\n", mib_regs->rx_len_65_to_127);
        printk("rx_len_128_to_255:            %10u\n", mib_regs->rx_len_128_to_255);
        printk("rx_len_256_to_511:            %10u\n", mib_regs->rx_len_256_to_511);
        printk("rx_len_512_to_1023:           %10u\n", mib_regs->rx_len_512_to_1023);
        printk("rx_len_1024_to_1522:          %10u\n", mib_regs->rx_len_1024_to_1522);
        printk("rx_len_1523_to_2047:          %10u\n", mib_regs->rx_len_1523_to_2047);
        printk("rx_len_2048_to_4095:          %10u\n", mib_regs->rx_len_2048_to_4095);
        printk("rx_len_4095_to_8191:          %10u\n", mib_regs->rx_len_4095_to_8191);
        printk("rx_len_8192_to_max:           %10u\n", mib_regs->rx_len_8192_to_max);
        printk("rx_jabber_pkts:               %10u\n", mib_regs->rx_jabber_pkts);
        printk("rx_oversize_pkts:             %10u\n", mib_regs->rx_oversize_pkts);
        printk("rx_fragment_pkts:             %10u\n", mib_regs->rx_fragment_pkts);
        printk("rx_missed_pkts:               %10u\n", mib_regs->rx_missed_pkts);
        printk("rx_crc_align_errs:            %10u\n", mib_regs->rx_crc_align_errs);
        printk("rx_undersize:                 %10u\n", mib_regs->rx_undersize);
        printk("rx_crc_errs:                  %10u\n", mib_regs->rx_crc_errs);
        printk("rx_align_errs:                %10u\n", mib_regs->rx_align_errs);
        printk("rx_symbol_errs:               %10u\n", mib_regs->rx_symbol_errs);
        printk("rx_pause_pkts:                %10u\n", mib_regs->rx_pause_pkts);
        printk("rx_nonpause_pkts:             %10u\n", mib_regs->rx_nonpause_pkts);
        printk("rx_sachanges:                 %10u\n", mib_regs->rx_sachanges);
        printk("rx_uni_pkts:                  %10u\n", mib_regs->rx_uni_pkts);
    }
}

/* Reads the stats from GMAC Regs */
#ifdef REPORT_HARDWARE_STATS
void gmac_hw_stats( int port,  struct rtnl_link_stats64 *stats)
{
    volatile EnetCoreMib *e = gmac_mib_regs(port);

    if ((gmac_port_map & (1 << port)))
    {
        stats->rx_packets = e->rx_pkts;
        stats->rx_bytes =  e->rx_octets;
        stats->multicast = e->rx_multicast_pkts;
        stats->rx_broadcast_packets = e->rx_broadcast_pkts;
        stats->rx_dropped = (e->rx_pkts - e->rx_good_pkts);
        stats->rx_errors =  
            (e->rx_crc_errs + e->rx_align_errs + e->rx_symbol_errs);
			
        stats->tx_packets = e->tx_pkts;
        stats->tx_bytes = e->tx_octets;
        stats->tx_multicast_packets = e->tx_multicast_pkts;
        stats->tx_broadcast_packets = e->tx_broadcast_pkts;
        stats->tx_dropped = (e->tx_pkts - e->tx_good_pkts);
    }
}
#endif

static void gmac_enable(int ethcore)
{
    uint32 cmdcfg, rxqctl, bp_clk, mdp, mode;
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(ethcore);
    volatile EnetCoreUnimac *unimac_regs = gmac_unimac_regs(ethcore);

    cmdcfg = unimac_regs->cmdcfg;

    /* put mac in reset */
    unimac_init_reset(unimac_regs);

    /* initialize default config */
    cmdcfg = unimac_regs->cmdcfg;

    cmdcfg &= ~(CC_TE | CC_RE | CC_RPI | CC_TAI | CC_HD | CC_ML |
                CC_CFE | CC_RL | CC_RED | CC_PE | CC_TPI | CC_PAD_EN | CC_PF);
    cmdcfg |= (CC_PROM | CC_NLC | CC_CFE | CC_TPI | CC_AT);

    unimac_regs->cmdcfg = cmdcfg;

    /* bring mac out of reset */
    unimac_clear_reset(unimac_regs);

    /* Other default configurations */
    /* Enable RX and TX flow control */
    unimac_flowcontrol(unimac_regs, 1, 1);
    /* Enable promiscuous mode */
    unimac_promisc(unimac_regs, 1);

    /* Enable the mac transmit and receive paths now */
    udelay(2);
    cmdcfg = unimac_regs->cmdcfg;
    cmdcfg &= ~CC_SR;
    //cmdcfg |= (CC_RE | CC_TE); move to gmac_set_active

    /* assert rx_ena and tx_ena when out of reset to enable the mac */
    unimac_regs->cmdcfg = cmdcfg;

    /* not force ht when gmac is in rev mii mode (we have rgmii mode) */
    mode = ((misc_regs->devstatus & DS_MM_MASK) >> DS_MM_SHIFT);
    if (mode != 0)
        /* request ht clock */
        misc_regs->clk_ctl_st |= CS_FH;

    /* Adjust RGMII TX delay time to meet the standard hold time limitation */
    misc_regs->devcontrol |= (0x3 << DC_TDS_SHIFT);

    /* init the mac data period. the value is set according to expr
     * ((128ns / bp_clk) - 3). */
    rxqctl = misc_regs->rxqctl;
    rxqctl &= ~RC_MDP_MASK;

    bp_clk = pmu_clk(PMU_PLL_CTRL_M3DIV_SHIFT) / 1000000;
    mdp = ((bp_clk * 128) / 1000) - 3;
    misc_regs->rxqctl = rxqctl | (mdp << RC_MDP_SHIFT);

    gmac_reset_mib(ethcore);
}

int gmac_init(void)
{
    volatile Aidmp *wrap;
    volatile EnetCoreMisc *misc_regs;
    volatile EnetCoreUnimac *unimac_regs;
    ETHERNET_MAC_INFO EnetInfo[BP_MAX_ENET_MACS];
    int index;

    if(BpGetEthernetMacInfo(EnetInfo, BP_MAX_ENET_MACS) != BP_SUCCESS)
    {
        printk(KERN_DEBUG " board id not set\n");
        return -1;
    }

    gmac_port_map = EnetInfo[0].sw.port_map;

    for (index=0; index < BP_MAX_ENET_MACS; index++)
    {
	    /* ethcore-0 must be enabled */
        if ((EnetInfo[0].sw.port_map & (1 << index)) == 0 && index == 0)
        {
            ethercore_enable(ENET_CORE0_WRAP);
            ethercore_clock_init(index);
            continue;
        }

        if (index == 0)
            wrap = ENET_CORE0_WRAP;
        else 
            wrap = ENET_CORE1_WRAP;

        /*
         * Enable the Ethernet core in order to have access to the Ethernet core
         * registers.
         */
        ethercore_enable(wrap);
        ethercore_clock_init(index);

        misc_regs = gmac_misc_regs(index);
        unimac_regs = gmac_unimac_regs(index);
        ether_gphy_reset(index);
        if (EnetInfo[0].sw.phyconn[index] == PHY_CONN_TYPE_EXT_PHY)
            ether_config_gphy(EnetInfo[0].sw.phy_id[index]);
        /* enable one rx interrupt per received frame */
        misc_regs->intrecvlazy = 1 << IRL_FC_SHIFT;

        /* Set the MAC address */
        //unimac_regs->macaddrhigh = htonl(*(uint32 *)&hwaddr[0]);
        //unimac_regs->macaddrlow = htons(*(uint32 *)&hwaddr[4]);

        /* set max frame lengths - account for possible vlan tag */
        unimac_regs->rxmaxlength = BCM_MAX_PKT_LEN;

        /* Clear interrupts, don't enable yet */
        misc_regs->intmask = 0;
        misc_regs->intstatus = DEF_INTMASK;

        /* Turn ON the GMAC */
        gmac_enable(index);
    }
    return 0;
}

/* sets the GMAC to be active, and ROBO port to be inactive */
//fixme
int gmac_set_active( void )
{
    ETHERNET_MAC_INFO EnetInfo[BP_MAX_ENET_MACS];
    int index;
    uint32 cmdcfg;
    volatile EnetCoreUnimac *unimac_regs;

    if(BpGetEthernetMacInfo(EnetInfo, BP_MAX_ENET_MACS) == BP_SUCCESS)
    {
        for (index=0; index < BP_MAX_ENET_MACS; index++)
        {
            if ((EnetInfo[0].sw.port_map & (1 << index)) == 0)
                continue;

            unimac_regs = gmac_unimac_regs(index);

            cmdcfg = unimac_regs->cmdcfg;
            /* Enable GMAC Tx and Rx */
            printk("Enable MAC core %d Rx & Tx (set bitMask 0x03)\n", index); 
            cmdcfg |= (CC_RE | CC_TE);
            unimac_regs->cmdcfg = cmdcfg;
        }
    }

    return 0;
}

void gmac_link_status_changed(int ethcore, int link_status, int speed, int duplex)
{
    uint32 cmdcfg;
    volatile EnetCoreUnimac *unimac_regs = gmac_unimac_regs(ethcore);

    if (link_status) 
    {
        cmdcfg = unimac_regs->cmdcfg;

        /* put mac in reset */
        unimac_init_reset(unimac_regs);
        /* clear speed and duplex */
        cmdcfg &= ~(CC_ES_MASK | CC_HD);
        /* set speed */
        if (speed == 1000)
            cmdcfg |= (0x2 << CC_ES_SHIFT);
        else if (speed == 100)
            cmdcfg |= (0x1 << CC_ES_SHIFT);
        /* set duplex */
        if (duplex == 0)
            cmdcfg |= CC_HD;

        unimac_regs->cmdcfg = cmdcfg;

        /* bring mac out of reset */
        unimac_clear_reset(unimac_regs);
    }
}

static void MoCA_reset(uint32_t gpio)
{
    /* using GPIO to reset moca device */
    bcm_gpio_set_dir(gpio, GPIO_OUT);

    /* Keep RESET high for 50ms */
    bcm_gpio_set_data(gpio, 1);
    mdelay(50);

    /* Keep RESET low for 300ms */
    bcm_gpio_set_data(gpio, 0);
    mdelay(300);

    /* Keep RESET high for 300ms */
    bcm_gpio_set_data(gpio, 1);
    mdelay(300);
}

static void
MoCA_init_gphy(struct bbsi_t *bbsi)
{
	/* write 1's except to bit 26 (gphy sw_init) */
	bbsi_write(bbsi, 0x1040431c, 4, 0xFBFFFFFF);
	/* clear bits 2 and 0 */
	bbsi_write(bbsi, 0x10800004, 4, 0x02a4c000);

	/* DELAY 10MS */
	mdelay(10);

	bbsi_write(bbsi,  0x1040431c, 4, 0xFFFFFFFF);
	/* take unimac out of reset */
	bbsi_write(bbsi, 0x10800000, 4, 0);

	/* Pin muxing for rgmii 0 and 1 */
	bbsi_write(bbsi, 0x10404100, 4, 0x11110000);
	bbsi_write(bbsi, 0x10404104, 4, 0x11111111);
	bbsi_write(bbsi, 0x10404108, 4, 0x11111111);
	bbsi_write(bbsi, 0x1040410c, 4, 0x00001111);
	/* Pin mux for MDIO */
	bbsi_write(bbsi, 0x10404118, 4, 0x00001100);

	bbsi_write(bbsi, 0x10800024, 4, 0x0000930d);
	/* enable rgmii 0 */
	bbsi_write(bbsi, 0x1080000c, 4, 0x00000011);
	/* enable rgmii 1 */
	bbsi_write(bbsi, 0x10800018, 4, 0x00000011);

	bbsi_write(bbsi, 0x10800808, 4, 0x010000d8);
	/* port mode for gphy and moca from rgmii */
	bbsi_write(bbsi, 0x10800000, 4, 2);

	/* tx and rx enable (0x3) */
	bbsi_write(bbsi, 0x10800808, 4, 0x0100000b);
	/* Link/ACT LED */
	bbsi_write(bbsi, 0x10800024, 4, 0x0000934d);

	/* Set 6802 mdio slave */
	bbsi_write(bbsi, 0x10800000, 4, 0xE);
	/* enable ID mode on RGMII 1 */
	bbsi_write(bbsi, 0x1080000c, 4, 0x0000001f);
	/* enable ID mode on RGMII 0 */
	bbsi_write(bbsi, 0x10800018, 4, 0x0000001f);
	/* Set rgmii to 2.5V CMOS */
	bbsi_write(bbsi, 0x104040a4, 4, 0x11);
}
 
void MoCA_eth_init()
{
    struct bbsi_t bbsi;
    uint16_t moca_reset;

    if (BpGetSpiClkGpio(&bbsi.spi_clk) != BP_SUCCESS)
    {
        printk("Error, BBSI SPI Clock is not defined!\n");
        return;
    }
    else 
        bbsi.spi_clk &= BP_GPIO_NUM_MASK;

    if (BpGetSpiCsGpio(&bbsi.spi_cs) != BP_SUCCESS)
    {
        printk("Error, BBSI SPI Chip Select is not defined!\n");
        return;
    }
    else 
        bbsi.spi_cs &= BP_GPIO_NUM_MASK;

    if (BpGetSpiMisoGpio(&bbsi.spi_miso) != BP_SUCCESS)
    {
        printk("Error, BBSI SPI MISO is not defined!\n");
        return;
    }
    else 
        bbsi.spi_miso &= BP_GPIO_NUM_MASK;

    if (BpGetSpiMosiGpio(&bbsi.spi_mosi) != BP_SUCCESS)
    {
        printk("Error, BBSI SPI MOSI is not defined!\n");
        return;
    }
    else 
        bbsi.spi_mosi &= BP_GPIO_NUM_MASK;

    if (BpGetMoCAResetGpio(&moca_reset) != BP_SUCCESS)
    {
        printk("Error, MoCA reset gpio is not defined!\n");
        return;
    }
    else 
        moca_reset &= BP_GPIO_NUM_MASK;

    /* init bbsi bus */
    bbsi_init(&bbsi);
    MoCA_reset(moca_reset);
    /* init gphy */
    MoCA_init_gphy(&bbsi);
}

EXPORT_SYMBOL( gmac_init );
EXPORT_SYMBOL( gmac_link_status_changed );
EXPORT_SYMBOL( gmac_set_active );
EXPORT_SYMBOL( MoCA_eth_init );

