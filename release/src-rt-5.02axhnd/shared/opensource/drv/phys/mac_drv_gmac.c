/*
   <:copyright-BRCM:2016:DUAL/GPL:standard

      Copyright (c) 2016 Broadcom
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


/*
 * GMAC driver for BCM47189
 */

#include "mac_drv.h"
#include <board.h>
#include "bcm_map_part.h"
#include "bcm_misc_hw_init.h"

/* Only necessary for ether_gphy_reset */
#include <linux/delay.h>
#include "bcm_gpio.h"


#define GMAC_RESET_DELAY        2
#define MDIO_POLL_PERIOD        10

/* The maximum packet length */
#define	ETHER_MAX_LEN   1518
/* The number of bytes in an ethernet (MAC) address */
#define	ETHER_ADDR_LEN  6
/* The number of bytes in the type field */
#define	ETHER_TYPE_LEN  2
/* The length of the combined header*/
#define	ETHER_HDR_LEN   (ETHER_ADDR_LEN * 2 + ETHER_TYPE_LEN)



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

static inline volatile EnetCoreMib* gmac_mib_regs(int ethcore)
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


static int ether_gphy_reset(int dmaPort)
{
    uint16_t reset;

    /* set gpio pad to floating state */
    GPIO->gpiopullup = 0;
    GPIO->gpiopulldown = 0;

    /* reset the external phy */
    if (BpGetPhyResetGpio(0, dmaPort, &reset) != BP_SUCCESS)
    {
        printk(KERN_EMERG "Phy reset gpio not found\n");
        /* put the core back into reset */
        /*
        if (softc->dmaPort == 0) {
            ethercore_disable(ENET_CORE0_WRAP);
        } else if (softc->dmaPort == 1) {
            ethercore_disable(ENET_CORE1_WRAP);
        }
        */
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


/*
 * Ethernet core/dma registers are not mapped into the address space
 * until the Eth core is reset.
 *
 * By default, only the ChipCommon registers are mapped (0x18000000). In WCC
 * the SoC cores, their identifiers and addresses are read programmatically from
 * the chip enumeration ROM.
 *
 * Core reset is done by writing the appropriate registers in the core
 * wrapper.
 *
 * Instead of populating a core list at runtime by walking the eROM, I defined
 * the necessary info from the available cores statically in the 47189 register
 * map so we can access the wrapper registers the same way we access any regular
 * register.
 */
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

void ethercore_disable(volatile Aidmp *wrap)
{
    /* if core is already in reset, just return */
    if (wrap->resetctrl & AIRC_RESET)
      return;

    /* ensure there are no pending backplane operations */
    SPINWAIT(wrap->resetstatus, 300);

    /* if pending backplane ops still, try waiting longer */
    if (wrap->resetstatus) {
        /* 300usecs was sufficient to allow backplane ops to clear for big hammer */
        /* during driver load we may need more time */
        SPINWAIT(wrap->resetstatus, 10000);
        /* if still pending ops, continue on and try disable anyway */
        /* this is in big hammer path, so don't call wl_reinit in this case... */
    }

    /* Put core into reset state */
    wrap->resetctrl = AIRC_RESET;
    udelay(1);

    wrap->ioctrl = 0;
    udelay(10);
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
    /* Disable promiscuous mode */
    unimac_promisc(unimac_regs, 0);

    /* Enable the mac transmit and receive paths now */
    udelay(2);
    cmdcfg &= ~CC_SR;
    cmdcfg |= (CC_RE | CC_TE);

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
}




/********** MAC API **********/

static int port_gmac_stats_clear(mac_dev_t *mac_dev);


static int port_gmac_init(mac_dev_t *mac_dev)
{
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(mac_dev->mac_id);
    volatile EnetCoreUnimac *unimac_regs = gmac_unimac_regs(mac_dev->mac_id);

    ether_gphy_reset(mac_dev->mac_id);
    /* enable one rx interrupt per received frame */
    misc_regs->intrecvlazy = 1 << IRL_FC_SHIFT;

    /* Set the MAC address */
    //unimac_regs->macaddrhigh = htonl(*(uint32 *)&hwaddr[0]);
    //unimac_regs->macaddrlow = htons(*(uint32 *)&hwaddr[4]);

    /* set max frame lengths - account for possible vlan tag */
    unimac_regs->rxmaxlength = ETHER_MAX_LEN + 32;

    /* Clear interrupts, don't enable yet */
    misc_regs->intmask = 0;
    misc_regs->intstatus = DEF_INTMASK;

    /* Turn ON the GMAC */
    gmac_enable(mac_dev->mac_id);

    return 0;
}

static int port_gmac_enable(mac_dev_t *mac_dev)
{
    /* Clear MIB counters */
    port_gmac_stats_clear(mac_dev);

    return 0;
}

static int port_gmac_disable(mac_dev_t *mac_dev)
{
    return 0;
}

static int port_gmac_cfg_get(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    return 0;
}

static int port_gmac_cfg_set(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    return 0;
}

static int port_gmac_pause_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    return 0;
}

static int port_gmac_pause_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    return 0;
}

static int port_gmac_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    volatile EnetCoreMib *mib_regs = gmac_mib_regs(mac_dev->mac_id);

    mac_stats->rx_byte = ((uint64_t)mib_regs->rx_good_octets_high << 32) | mib_regs->rx_good_octets;
    mac_stats->rx_packet = mib_regs->rx_good_pkts;
    mac_stats->rx_broadcast_packet = mib_regs->rx_broadcast_pkts;
    mac_stats->rx_multicast_packet = mib_regs->rx_multicast_pkts;
    mac_stats->tx_byte = ((uint64_t)mib_regs->tx_good_octets_high << 32) | mib_regs->tx_good_octets;
    mac_stats->tx_packet = mib_regs->tx_good_pkts;
    mac_stats->tx_broadcast_packet = mib_regs->tx_broadcast_pkts;
    mac_stats->tx_multicast_packet = mib_regs->tx_multicast_pkts;

    return 0;
}

static int port_gmac_stats_clear(mac_dev_t *mac_dev)
{
    volatile EnetCoreMib *mib_regs = gmac_mib_regs(mac_dev->mac_id);
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

    return 0;
}

static int port_gmac_mtu_set(mac_dev_t *mac_dev, int mtu)
{
    return 0;
}

static int port_gmac_eee_set(mac_dev_t *mac_dev, int enable)
{
    return 0;
}

static int port_gmac_dev_add(mac_dev_t *mac_dev)
{
    volatile Aidmp *wrap;

    if (mac_dev->mac_id == 0) {
        wrap = ENET_CORE0_WRAP;
    } else if (mac_dev->mac_id == 1) {
        wrap = ENET_CORE1_WRAP;
    } else {
        printk("Invalid Ethernet core in mac_id (%d)\n", mac_dev->mac_id);
        return 1;
    }

    /*
     * Enable the Ethernet core in order to have access to the Ethernet core
     * registers.
     */
    ethercore_enable(wrap);
    ethercore_clock_init(mac_dev->mac_id);
    return 0;
}

static int port_gmac_dev_del(mac_dev_t *mac_dev)
{
    return 0;
}

static int port_gmac_drv_init(mac_drv_t *mac_drv)
{
    mac_drv->initialized = 1;
    return 0;
}


mac_drv_t mac_drv_gmac =
{
    .mac_type = MAC_TYPE_GMAC,
    .name = "GMAC",
    .init = port_gmac_init,
    .enable = port_gmac_enable,
    .disable = port_gmac_disable,
    .cfg_get = port_gmac_cfg_get,
    .cfg_set = port_gmac_cfg_set,
    .pause_get = port_gmac_pause_get,
    .pause_set = port_gmac_pause_set,
    .stats_get = port_gmac_stats_get,
    .stats_clear = port_gmac_stats_clear,
    .mtu_set = port_gmac_mtu_set,
    .eee_set = port_gmac_eee_set,
    .dev_add = port_gmac_dev_add,
    .dev_del = port_gmac_dev_del,
    .drv_init = port_gmac_drv_init,
};
