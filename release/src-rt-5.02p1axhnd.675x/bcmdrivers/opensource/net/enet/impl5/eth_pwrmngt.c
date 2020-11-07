/*
 Copyright 2007-2013 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2013:DUAL/GPL:standard
 
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
#define _BCMENET_LOCAL_


#if defined(CONFIG_BCM963381)
#define CONFIG_BCM_EPHY_EEE
#endif

#include <bcm_map_part.h>
#include "bcmenet.h"
#include "bcmmii.h"
#include "ethsw.h"
#include "bcmsw.h"
#include "ethsw_phy.h"

#include "eth_pwrmngt.h"

extern struct semaphore bcm_ethlock_switch_config;
extern BcmEnet_devctrl *pVnetDev0_g;

static uint32 ephy_forced_pwr_down_status = 0;
extern atomic_t phy_write_ref_cnt;
extern atomic_t phy_read_ref_cnt;

#define CORE_SHD1C_05 0x15
#define CORE_SHD1C_0A 0x1a

#if defined(CONFIG_BCM963268)
#define NUM_INT_EPHYS 3
#define NUM_INT_GPHYS 1
#define EPHY_BASE_PHYID 1
#define GPHY_BASE_PHYID (EPHY_BASE_PHYID + NUM_INT_EPHYS)
#elif defined(CONFIG_BCM963381)
#define NUM_INT_EPHYS 4
#define NUM_INT_GPHYS 0
#define EPHY_BASE_PHYID (MISC_REG->EphyPhyAd & EPHY_PHYAD_MASK)
#elif defined(CONFIG_BCM947189)
#define NUM_INT_EPHYS 0
#define NUM_INT_GPHYS 0
#else
#define NUM_INT_EPHYS 4
#define NUM_INT_GPHYS 0
#define EPHY_BASE_PHYID 1
#endif
#define NUM_INT_PHYS (NUM_INT_EPHYS + NUM_INT_GPHYS)


/*
 * Ethernet Auto-Power-Down Features
 */

#if defined(CONFIG_BCM_ETH_PWRSAVE)
static unsigned int  eth_auto_power_down_enabled = 1;
#else
static unsigned int  eth_auto_power_down_enabled = 0;
#endif

#if defined(CONFIG_BCM_ETH_PWRSAVE)
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
extern spinlock_t bcmsw_pll_control_lock;
#endif
#if !defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
static uint32 ephy_pwr_down_status = 0;
#endif
#endif

/* Deep Green Mode enabled flag currently tied to the WebGUI's "Advanced Setup -> Power Management -> Ethernet Auto Power Down & Sleep" checkbox */
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
static int deep_green_mode_enabled = 1;      //Keep track of whether Deep Green Mode feature is enabled/disabled
static int deep_green_mode_activated = 0;    //Keep track of whether Deep Green Mode feature is activated/deactivated (DGM is activated when the feature is enabled and all ports are disconnected)
#endif


int ethsw_shutdown_unused_macs(BcmEnet_devctrl *pDevCtrl)
{
    return 0;
}


int ethsw_shutdown_unused_phys(void)
{
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    unsigned int portmap, i;
    unsigned int count = 0;

    portmap = pVnetDev0->EnetInfo[0].sw.port_map;

    for (i = 0; i < NUM_INT_EPHYS; i++) {
        if ((portmap & (1U<<i)) == 0) {
            /* Shut down unused EPHY ports completely */
#if defined(CONFIG_BCM963381)
            MISC_REG->EphyPwrMgnt |= EPHY_PWR_DOWN_ALL_0 >> (i*EPHY_PWR_DOWN_SHIFT_FACTOR);
#elif defined (EPHY_PWR_DOWN_ALL_1)
            GPIO->RoboswEphyCtrl |= EPHY_PWR_DOWN_ALL_1 << (i*EPHY_PWR_DOWN_SHIFT_FACTOR);
#endif
            count ++;
        }
    }
    if (count == NUM_INT_EPHYS) {
#if defined(CONFIG_BCM963381)
        MISC_REG->EphyPwrMgnt |= EPHY_PWR_DOWN_BIAS;
#elif defined(EPHY_PWR_DOWN_BIAS)
        GPIO->RoboswEphyCtrl |= EPHY_PWR_DOWN_BIAS;
#endif
    }
#endif
    return 0;
}

#if (CONFIG_BCM_EXT_SWITCH_TYPE == 53115)
static int extsw_bus_contention(int operation, int retries)
{
   static int contention_req = 0;
   uint8 val;
   int i;

   if (operation) {
      if (!contention_req) {
         val=2;
         extsw_wreg_wrap(PAGE_CONTROL, 0xa0, &val , 1);
         contention_req = 1;
      }

      for (i=0;i<retries;++i) {
         extsw_rreg_wrap(PAGE_CONTROL, 0xa0, &val , 1);
         if (val == 3) {
            return 1;
         }
      }
   } else {
      contention_req = 0;
      val=0;
      extsw_wreg_wrap(PAGE_CONTROL, 0xa0, &val , 1);
      return 1;
   }

   return 0;
}

/* This code keeps the APD compatibility bit set (register 1c, shadow 0xa, bit 8)
   when the internal 8051 on the 53125 chip clears it. If the 8051 is disabled,
   then this code does not need to run. */
void extsw_apd_set_compatibility_mode(void)
{
   uint16 v16;
   int i;
   static int port = 0;
   static int contention_req = 0;

   if (!KNOWN_SWITCH(pVnetDev0_g->extSwitch->switch_id))
      return;

   down(&bcm_ethlock_switch_config);
   /* Only check ports that don't have a link */
   if (!(pVnetDev0_g->linkState & (1 << port)) || contention_req) {

      /* Bus contention with 8051 */
      contention_req = 1;
      if (!extsw_bus_contention(1, 3)) {
         /* We'll get it next time around */
         up(&bcm_ethlock_switch_config);
         return;
      }

      /* Check if one of the ports needs to set APD compatibility */
      v16 = MII_1C_AUTO_POWER_DOWN_SV;
      ethsw_phyport_wreg(PHYSICAL_PORT_TO_LOGICAL_PORT(port, 1), MII_REGISTER_1C, &v16);
      ethsw_phyport_rreg(PHYSICAL_PORT_TO_LOGICAL_PORT(port, 1), MII_REGISTER_1C, &v16);

      /* If one of the ports needs to be set, process all the ports */
      if (v16 == 0x2821) {
         for (i=0;i<5;i++) {
            /* Don't need to process ports that have a link */
            if (pVnetDev0_g->linkState & (1 << i))
               continue;

            /* Only change the register if it is not correctly set */
            v16 = MII_1C_AUTO_POWER_DOWN_SV;
            ethsw_phyport_wreg(PHYSICAL_PORT_TO_LOGICAL_PORT(i, 1), MII_REGISTER_1C, &v16);
            ethsw_phyport_rreg(PHYSICAL_PORT_TO_LOGICAL_PORT(i, 1), MII_REGISTER_1C, &v16);
            if (v16 == 0x2821) {
               /* Change it to 0xa921 */
               v16 = MII_1C_WRITE_ENABLE | MII_1C_AUTO_POWER_DOWN_SV | MII_1C_WAKEUP_TIMER_SEL_84
                   | MII_1C_AUTO_POWER_DOWN | MII_1C_APD_COMPATIBILITY;
               ethsw_phyport_wreg(PHYSICAL_PORT_TO_LOGICAL_PORT(i, 1), MII_REGISTER_1C, &v16);
            }
         }
      }

      /* Release bus to 8051 */
      contention_req = 0;
      extsw_bus_contention(0, 0);
   }

   if (++port > 4) {
      port = 0;
   }
   up(&bcm_ethlock_switch_config);
}
#endif

#if defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
void ethsw_ephy_enable_apd(int phy_id, int enable)
{
    uint16 v16 = 0x7001;

    if (enable) {
        v16 |= MII_1C_AUTO_POWER_DOWN;
    }
    ethsw_ephy_shadow_write(phy_id, 1, 0x1b, &v16); 
}

void ethsw_gphy_enable_apd(int phy_id, int apd_enable, int dllapd_enable)
{
    uint16 v16;

    v16 = MII_1C_WRITE_ENABLE | MII_1C_AUTO_POWER_DOWN_SV |
          MII_1C_WAKEUP_TIMER_SEL_84 | MII_1C_APD_COMPATIBILITY;
    if (apd_enable) {
        v16 |= MII_1C_AUTO_POWER_DOWN;
    }
    ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);

    // Also enable DLL APD if requested, no need to support disabling
    // since this setting is dependent on APD enable/disable
    if (dllapd_enable) {
        v16 = MII_1C_RESERVED_CTRL3_SV;
        ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
        ethsw_phy_rreg(phy_id, MII_REGISTER_1C, &v16);
        v16 |= MII_1C_WRITE_ENABLE | MII_1C_CLK125_OUTPUT_DIS;
        v16 &= ~MII_1C_AUTO_PWRDN_DLL_DIS; 
        ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
    }
}

void ethsw_setup_hw_apd(unsigned int enable)
{
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    unsigned int phy_id, i;
    int unit, phys_port;
    uint16 v16;

    down(&bcm_ethlock_switch_config);

    /* For each configured external PHY, enable/disable APD */
    for (i = 0; i < MAX_TOTAL_SWITCH_PORTS; i++) {
        unit = LOGICAL_PORT_TO_UNIT_NUMBER(i);
        phys_port = LOGICAL_PORT_TO_PHYSICAL_PORT(i);
        /* Check if the port is in the portmap or not */
        if ((pVnetDev0->EnetInfo[unit].sw.port_map & (1U<<phys_port)) != 0) {
            /* Check if the port is connected to a PHY or not */
            phy_id = pVnetDev0->EnetInfo[unit].sw.phy_id[phys_port];
            /* If a Phy is connected, and is external, set APD */
            if(IsPhyConnected(phy_id) && IsExtPhyId(phy_id) && !IsSerdes(phy_id)) {
                /* Read MII Phy Identifier, process the AC201 differently */
                ethsw_phy_rreg(phy_id, MII_PHYSID2, &v16);
                if ((v16 & BCM_PHYID_M) == (BCMAC201_PHYID2 & BCM_PHYID_M)) {
                    ethsw_ephy_enable_apd(phy_id, enable);
                } else if ((v16 & BCM_PHYID_M) != (BCM54610_PHYID2 & BCM_PHYID_M)) {
                    /* Other PHYs */
                    ethsw_gphy_enable_apd(phy_id, enable, 1);
                }
            }
        }
    }

    /* For each internal PHY (including those not in boardparms), enable/disable APD */
#if NUM_INT_EPHYS > 0
    /* EPHYs */
    for (i = EPHY_BASE_PHYID; i < EPHY_BASE_PHYID+NUM_INT_EPHYS; i++) {
        ethsw_ephy_enable_apd(i, enable);
    }
#endif

#if NUM_INT_GPHYS > 0
    /* GPHYs */
    for (i = GPHY_BASE_PHYID; i < GPHY_BASE_PHYID+NUM_INT_GPHYS; i++) {
        ethsw_gphy_enable_apd(i, enable, 1);
    }
#endif


    up(&bcm_ethlock_switch_config);
    printk("Ethernet Auto Power Down and Sleep: %s\n", enable?"Enabled":"Disabled");
}
#endif

void BcmPwrMngtSetEthAutoPwrDwn(unsigned int enable, int linkState)
{
   if (eth_auto_power_down_enabled != enable) {
      eth_auto_power_down_enabled = enable;
#if defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
      ethsw_setup_hw_apd(enable);
#else
      printk("Ethernet Auto Power Down and Sleep changed to %s\n", enable?"enabled":"disabled");
#endif
   }
}

int BcmPwrMngtGetEthAutoPwrDwn(void)
{
   return (eth_auto_power_down_enabled);
}

/* Delay in miliseconds after enabling the EPHY PLL before reading the different EPHY status      */
/* The PLL requires 400 uSec to stabilize, but Energy detection on the ports requires more time. */
/* Normally, Energy detection works when PLL is down, but a long delay (minutes) is present     */
/* for ports that are in Auto Power Down mode. 40 mSec is chosen because this is the delay      */
/* that allows EPHY to send two link pulses (or series of pulses) at 16 mSec interval                  */
#define PHY_PLL_ENABLE_DELAY 1
#define PHY_PORT_ENABLE_DELAY 40

/* Describe internal PHYs */
#if defined(CONFIG_BCM_ETH_PWRSAVE)
#if defined(CONFIG_BCM963268)
static uint64 ephy_energy_det[NUM_INT_PHYS] = {1<<(INTERRUPT_ID_EPHY_ENERGY_0-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_EPHY_ENERGY_1-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_EPHY_ENERGY_2-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_GPHY_ENERGY_0-INTERNAL_ISR_TABLE_OFFSET)};
static uint32 gphy_pwr_dwn[NUM_INT_GPHYS] =   {GPHY_LOW_PWR};
#define ROBOSWGPHYCTRL RoboswGphyCtrl
#endif

int ethsw_phy_pll_up(int ephy_and_gphy)
{
#if defined(CONFIG_BCM963268)
    int ephy_status_changed = 0;

#if NUM_INT_GPHYS > 0
    if (ephy_and_gphy)
    {
        int i;
        uint32 roboswGphyCtrl = GPIO->ROBOSWGPHYCTRL;

        /* Bring up internal GPHY PLLs if they are down */
        for (i = 0; i < NUM_INT_GPHYS; i++)
        {
            if ((roboswGphyCtrl & gphy_pwr_dwn[i]) && !(ephy_forced_pwr_down_status & (1<<PHYSICAL_PORT_TO_LOGICAL_PORT(i+NUM_INT_EPHYS, 0))))
            {
                roboswGphyCtrl &= ~gphy_pwr_dwn[i];
                ephy_status_changed = 1;
            }
        }
        if (ephy_status_changed) {
            GPIO->ROBOSWGPHYCTRL = roboswGphyCtrl;
        }
    }
#endif

    /* This is a safety measure in case one tries to access the EPHY */
    /* while the PLL/RoboSw is powered down */
#if defined(ROBOSW250_CLK_EN)
    if (!(PERF->blkEnables & ROBOSW250_CLK_EN))
    {
        /* Enable robosw clock */
#if defined(ROBOSW025_CLK_EN)
        PERF->blkEnables |= ROBOSW250_CLK_EN | ROBOSW025_CLK_EN;
#else
        PERF->blkEnables |= ROBOSW250_CLK_EN;
#endif
        ephy_status_changed = 1;
    }
#endif

    if (ephy_status_changed) {
        if (irqs_disabled() || (preempt_count() != 0)) {
            mdelay(PHY_PLL_ENABLE_DELAY);
        } else {
            msleep(PHY_PLL_ENABLE_DELAY);
        }
        return (msecs_to_jiffies(PHY_PLL_ENABLE_DELAY));
    }
#endif
    return 0;
}

uint32 ethsw_ephy_auto_power_down_wakeup(void)
{
#if defined(CONFIG_BCM963268)
#if !defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
    int phy_id;
    int ephy_sleep_delay = 0;
    int ephy_status_changed = 0;
    int i;
    uint16 v16;
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);

    /* Ensure that only this thread accesses PHY registers in this interval */
    down(&bcm_ethlock_switch_config);

    /* Make sure EPHY PLL is up */
    ephy_sleep_delay = ethsw_phy_pll_up(1);


    /* Make sure all PHY Ports are up */
    for (i = 0; i < NUM_INT_PHYS; i++)
    {
        if (ephy_pwr_down_status & (1<<i) && !(ephy_forced_pwr_down_status & (1<<PHYSICAL_PORT_TO_LOGICAL_PORT(i, 0))))
        {
#if NUM_INT_GPHYS > 0
            if (i >= NUM_INT_EPHYS)
            {
                // This GPHY port was down
                // Toggle pwr down bit, register 0, bit 11
                // if it was not already down, otherwise leave it down
                phy_id = priv->EnetInfo[0].sw.phy_id[i];
                ethsw_phy_rreg(phy_id, MII_CONTROL, &v16);
                if (!(v16 & MII_CONTROL_POWER_DOWN)) {
                    v16 |= MII_CONTROL_POWER_DOWN;
                    ethsw_phy_wreg(phy_id, MII_CONTROL, &v16);
                    v16 &= ~MII_CONTROL_POWER_DOWN;
                    ethsw_phy_wreg(phy_id, MII_CONTROL, &v16);
                    ephy_status_changed = 1;
                    ephy_sleep_delay += 3;
                }
                ephy_pwr_down_status &= ~(1<<i);
            }
#endif
        }
    }

    up(&bcm_ethlock_switch_config);

    if (ephy_status_changed)
    {
        /* Allow the ports to be enabled and transmitting link pulses */
        msleep(PHY_PORT_ENABLE_DELAY);
        ephy_sleep_delay += msecs_to_jiffies(PHY_PORT_ENABLE_DELAY);
    }

    return ephy_sleep_delay;
#else
    return ethsw_phy_pll_up(1);
#endif
#else
    return 0;
#endif
}

uint32 ethsw_ephy_auto_power_down_sleep(void)
{
#if defined(CONFIG_BCM963268)
    int i;
    int ephy_sleep_delay = 0;
    uint64 irqStatus = PERF->IrqControl[0].IrqStatus;
    static uint64 prevIrqStatus[4] = {0};
    static int prevIrqStatusIndex = 0;
    uint64 lastIrqStatus = 0;
    int ephy_has_energy = 0;
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    int map = priv->EnetInfo[0].sw.port_map;
    int phy_id;
    uint16 v16;

    prevIrqStatus[prevIrqStatusIndex] = irqStatus;
    prevIrqStatusIndex++;
    prevIrqStatusIndex &= 0x3;
    for (i=0;i<4;i++) {
        lastIrqStatus |= prevIrqStatus[i];
    }

    if (!eth_auto_power_down_enabled)
    {
        return ephy_sleep_delay;
    }

    /* Ensure that only this thread accesses PHY registers in this interval */
    down(&bcm_ethlock_switch_config);

    /* Turn off EPHY Ports that have no energy */
    for (i = 0; i < NUM_INT_PHYS; i++)
    {
        if (map & (1<<i))
        {
            /* Verify if the link is down, don't want to force down while the link is still up */
            phy_id = priv->EnetInfo[0].sw.phy_id[i];
            ethsw_phy_rreg(phy_id, 0x1, &v16);
            if (!(lastIrqStatus & ephy_energy_det[i]) || (!(v16&0x4) && (ephy_forced_pwr_down_status & (1<<PHYSICAL_PORT_TO_LOGICAL_PORT(i, 0)))))
            {
#if !defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE) && NUM_INT_GPHYS > 0
                if (i >= NUM_INT_EPHYS)
                {
                    /* This GPHY port has no energy, bring it down */
                    GPIO->ROBOSWGPHYCTRL |= gphy_pwr_dwn[i-NUM_INT_EPHYS];
                    ephy_pwr_down_status |= (1<<i);
                }
#endif
            }
            else
            {
                ephy_has_energy = 1;
            }
        }
    }

    if (priv->extSwitch->brcm_tag_type != BRCM_TYPE2) {
        /* If no energy was found on any PHY and no other switch port is linked, bring down PLL to save power */
        if (!ephy_has_energy && !priv->linkState)
        {
            unsigned long flags;
            spin_lock_irqsave(&bcmsw_pll_control_lock, flags);
#if defined(ROBOSW025_CLK_EN)
            PERF->blkEnables &= ~(ROBOSW250_CLK_EN | ROBOSW025_CLK_EN);
#elif defined(ROBOSW250_CLK_EN)
            // The following line of code is found to cause system freeze on some products
            // It needs to be commented out until a different solution is found for those who
            // need the extra power savings of about 150 mWatt at AC level
            // PERF->blkEnables &= ~ROBOSW250_CLK_EN;
#endif
            spin_unlock_irqrestore(&bcmsw_pll_control_lock, flags);
        }
    }

    up(&bcm_ethlock_switch_config);

    return ephy_sleep_delay;
#else
    return 0;
#endif
}
#endif

static void ethsw_switch_manage_ext_phy_power_mode(int portnumber, int power_mode)
{
    uint16 reg = 0; /* MII reg 0x00 */
    uint16 v16 = 0;

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
    if (IsExternalSwitchPort(portnumber)) {
        /*
         * use BCM specific MII Reg 16 instead of standard MII_BMCR to
         * force link down/up, as on 53125 switch powering down a PHY
         * has impact on adjacent PHYs.
         */
        BCM_ENET_DEBUG("portnumber=%d power_mode=%d\n", portnumber, power_mode);
        if (!power_mode)
            v16 = 0x2000;
        ethsw_phyport_wreg(portnumber, 16, &v16);
        return;
    }
#endif

    ethsw_phyport_rreg(portnumber, reg, &v16);
    v16 |= 0x0200; /* Auto negotiation restart */

    if(!power_mode) {
        /* MII Power Down enable, forces link down */
        v16 |= 0x0800;
        ethsw_phyport_wreg(portnumber, reg, &v16);
        ephy_forced_pwr_down_status |= (1<<portnumber);
    }
    else {
        /* MII Power Down disable */
        v16 &= ~0x0800;
        ethsw_phyport_wreg(portnumber, reg, &v16);
        ephy_forced_pwr_down_status &= ~(1<<portnumber);
    }
    return; 
}

void ethsw_switch_manage_port_power_mode(int portnumber, int power_mode)
{
   int phy_id = enet_logport_to_phyid(portnumber);
#if NUM_INT_GPHYS > 0
   int phys_port = LOGICAL_PORT_TO_PHYSICAL_PORT(portnumber);
#endif

   down(&bcm_ethlock_switch_config);

   /* external ports, GPHYs and external PHYs */
   if (IsExternalSwitchPort(portnumber) ||
#if NUM_INT_GPHYS > 0
       ((phys_port >= NUM_INT_EPHYS) && (phys_port < NUM_INT_PHYS)) ||
#endif
       (IsExtPhyId(phy_id)))
   {
       if (power_mode) {
          ethsw_switch_manage_ext_phy_power_mode(portnumber, power_mode);
       }
       else {
          uint16 v16, u16;
          // Turn off DLL APD while powering down, as it interferes with power down
          v16 = MII_1C_RESERVED_CTRL3_SV;
          ethsw_phyport_wreg(portnumber, MII_REGISTER_1C, &v16);
          ethsw_phyport_rreg(portnumber, MII_REGISTER_1C, &v16);
          v16 |= MII_1C_WRITE_ENABLE;
          u16 = v16;
          v16 |= MII_1C_AUTO_PWRDN_DLL_DIS; 
          ethsw_phyport_wreg(portnumber, MII_REGISTER_1C, &v16);

          // Now power down the port
          ethsw_switch_manage_ext_phy_power_mode(portnumber, power_mode);
          // Give time for GPHY to detect power down
          up(&bcm_ethlock_switch_config);
          msleep(1000);
          down(&bcm_ethlock_switch_config);

          // Set DLL APD to its previous setting
          ethsw_phyport_wreg(portnumber, MII_REGISTER_1C, &u16);
       }       
   }
#if NUM_INT_EPHYS > 0
   else if (portnumber < NUM_INT_EPHYS) {  /* EPHYs */
       uint16 v16 = 0;
       /* When the link is being brought up or down, the link status interrupt may occur
          before this command is completed, where the PHY is configured in shadow mode.
          We need to prevent this by disabling EPHY interrupts. The problem does not exist
          for GPHY because the link status changes in a single command.
       */
       atomic_inc(&phy_write_ref_cnt);
       atomic_inc(&phy_read_ref_cnt);
       BcmHalInterruptDisable(INTERRUPT_ID_EPHY);

       if(!power_mode) {
          /* Bring it down */
          v16 = 0x008B;
          ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
          v16 = 0x0700; /* tx pwr down */
          ethsw_phy_wreg(phy_id, 0x10, &v16);
          msleep(1); /* Without this, the speed LED on 63168 stays on */
          v16 = 0x7008; /* rx pwr down & link status disable */
          ethsw_phy_wreg(phy_id, 0x14, &v16);
          v16 = 0x000B;
          ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
          ephy_forced_pwr_down_status |= (1<<portnumber);
       }
       else {
          /* Bring it up */
          v16 = 0x008B;
          ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
          v16 = 0x0400;
          ethsw_phy_wreg(phy_id, 0x10, &v16);
          v16 = 0x0008;
          ethsw_phy_wreg(phy_id, 0x14, &v16);
          v16 = 0x000B;
          ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
          /* Restart Autoneg in case the cable was unplugged or plugged while down */
          ethsw_phy_rreg(phy_id, MII_CONTROL, &v16);
          v16 |= MII_CONTROL_RESTART_AUTONEG;
          ethsw_phy_wreg(phy_id, MII_CONTROL, &v16);
          ephy_forced_pwr_down_status &= ~(1<<portnumber);
       }

       atomic_dec(&phy_write_ref_cnt);
       atomic_dec(&phy_read_ref_cnt);
       BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
   }
#endif

   up(&bcm_ethlock_switch_config);
}
EXPORT_SYMBOL(ethsw_switch_manage_port_power_mode);

int ethsw_switch_get_port_power_mode(int portnumber)
{
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
    if (IsExternalSwitchPort(portnumber)) {
        uint16 v16 = 0;
        ethsw_phyport_rreg(portnumber, 16, &v16);
        return v16 & 0x2000;
    }
#endif
   return (ephy_forced_pwr_down_status & (1<<portnumber));
}
EXPORT_SYMBOL(ethsw_switch_get_port_power_mode);



/*
 * Ethernet Deep Green Mode Features
 */
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)


void ethsw_deep_green_mode_activate(void)
{
}

void ethsw_deep_green_mode_deactivate(void)
{
}

void ethsw_deep_green_mode_handler(int linkState)
{
}

int BcmPwrMngtGetDeepGreenMode(int mode) {
    if (!mode) {
        return (deep_green_mode_enabled);
    } else {
        return (deep_green_mode_activated);
    }
}

void BcmPwrMngtSetDeepGreenMode(unsigned int enable, int linkState) {
    if (deep_green_mode_enabled != enable) {
        deep_green_mode_enabled = enable;
        ethsw_deep_green_mode_handler(linkState);
        printk("Deep Green Mode feature changed to %s (DGM status: %s)\n", enable?"enabled":"disabled", deep_green_mode_activated?"activated":"deactivated");
    }
}
#endif  /* defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE) */

void ethsw_force_mac_up(int port)
{
}


/*
 * Energy Efficient Ethernet Features
 */

// Note that if you want to force all interfaces to run with EEE disabled
// It is necessary to keep the CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET set,
// but change pm_eee_enabled to 0 by defining CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET_DISABLED
// at compile time or by modifying the code below.
// This is because some PHYs start with EEE enabled by default.
#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET_DISABLED)
static unsigned int pm_eee_enabled = 0;
#else
static unsigned int pm_eee_enabled = 1;
#endif

static void ethsw_eee_set_all(unsigned int enable);

void BcmPwrMngtSetEnergyEfficientEthernetEn(unsigned int enable)
{
    pm_eee_enabled = enable;
    ethsw_eee_set_all(enable);

    printk("Energy Efficient Ethernet changed to %s\n", enable?"enabled":"disabled");
}

int BcmPwrMngtGetEnergyEfficientEthernetEn(void)
{
    return pm_eee_enabled;
}

#if defined(CONFIG_BCM_EPHY_EEE)
static int ethsw_eee_ephy_get(int phy_id, unsigned int *enable)
{
    uint16 data16;

    if (IsExtPhyId(phy_id))
        return -1;

    /* Check if local EEE is set */
    ethsw_ephy_shadow_read(phy_id, 3, 0x03, &data16);
    *enable = data16 & (1 << 1);

    return 0;
}

static int ethsw_eee_resolution_ephy_get(int phy_id, unsigned int *enable)
{
    uint16 data16;

    if (IsExtPhyId(phy_id))
        return -1;

    /* Check if AN EEE Resolution is set */
    ethsw_ephy_shadow_read(phy_id, 3, 0x000b, &data16);
    *enable = data16 & 0x100;

    return 0;
}

static void ethsw_eee_ephy_set(int phy_id, unsigned int enable)
{
   uint16 data16;

   if (!IsExtPhyId(phy_id)) {
      if (enable) {
         ethsw_ephy_write_bank3_reg(phy_id, 0x3, 0x0002);
         ethsw_ephy_write_bank3_reg(phy_id, 0x6, 0x4400);
         ethsw_ephy_write_bank3_reg(phy_id, 0xe, 0x0050);
         ethsw_ephy_write_bank3_reg(phy_id, 0xb, 0x0003);
         data16 = 0x3200;
         ethsw_phy_wreg(phy_id, 0x00, &data16);
      } else {
         ethsw_ephy_write_bank3_reg(phy_id, 0x3, 0x0000);
         ethsw_ephy_write_bank3_reg(phy_id, 0xb, 0x0000);
         data16 = 0x3200;
         ethsw_phy_wreg(phy_id, 0x00, &data16);
      }
   }
}
#endif

static void ethsw_eee_phy_expansion_wreg(int phy_id, uint16 reg, uint16 value)
{
   uint16 v16, apdv16;
   int apd_disabled = 0;

   /* Disable APD if it was set */
   apdv16 = MII_1C_AUTO_POWER_DOWN_SV;
   ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &apdv16);
   ethsw_phy_rreg(phy_id, MII_REGISTER_1C, &apdv16);
   if (apdv16 & MII_1C_AUTO_POWER_DOWN) {
      apdv16 &= ~MII_1C_AUTO_POWER_DOWN;
      ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &apdv16);
      apd_disabled = 1;
   }

   v16 = 0x0C00;
   ethsw_phy_wreg(phy_id, MII_REGISTER_18, &v16);

   // Expansion write
   ethsw_phy_wreg(phy_id, 0x17, &reg);
   ethsw_phy_wreg(phy_id, 0x15, &value);

   v16 = 0x0400;
   ethsw_phy_wreg(phy_id, MII_REGISTER_18, &v16);

   /* Re-enable APD if it was disabled by this code */
   if (apd_disabled) {
      apdv16 |= MII_1C_AUTO_POWER_DOWN;
      ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &apdv16);
   }
}


static void ethsw_eee_extra_phy_init(int phy_id)
{
   uint16 data16;

   if (IsPhyConnected(phy_id) && IsExtPhyId(phy_id) && !IsSerdes(phy_id)) {
      ethsw_phy_rreg(phy_id, MII_PHYSID1, &data16);
      if (data16 == BCM50210_PHYID1) {
         ethsw_phy_rreg(phy_id, MII_PHYSID2, &data16);
         if ((data16 & BCM_PHYID_M) == (BCM50210_PHYID2 & BCM_PHYID_M)) {
            // 50210E & 50212E: Extra EEE configuration
            ethsw_eee_phy_expansion_wreg(phy_id, 0x0021, 0x470f);

            /* Restart autoneg */
            ethsw_phy_rreg(phy_id, MII_CONTROL, &data16);
            data16 |= MII_CONTROL_RESTART_AUTONEG;
            ethsw_phy_wreg(phy_id, MII_CONTROL, &data16);
         }
      }
   }
}

static void ethsw_eee_compatibility_set(int log_port, int enable)
{
   uint16 v16, apdv16;
   uint16 r18s0;
   int apd_disabled = 0;

   /* Disable APD if it was set */
   apdv16 = MII_1C_AUTO_POWER_DOWN_SV;
   ethsw_phyport_wreg(log_port, MII_REGISTER_1C, &apdv16);
   ethsw_phyport_rreg(log_port, MII_REGISTER_1C, &apdv16);
   if (apdv16 & MII_1C_AUTO_POWER_DOWN) {
      apdv16 &= ~MII_1C_AUTO_POWER_DOWN;
	  apdv16 |= MII_1C_WRITE_ENABLE;
      ethsw_phyport_wreg(log_port, MII_REGISTER_1C, &apdv16);
      apd_disabled = 1;
   }

   /* Write a sequence specific for these GPHYs */
   v16 = 0x0007;
   ethsw_phyport_wreg(log_port, 0x18, &v16);
   ethsw_phyport_rreg(log_port, 0x18, &r18s0);

   v16 = r18s0 | 0x0800;
   ethsw_phyport_wreg(log_port, 0x18, &v16);
   v16 = 0x001A;
   ethsw_phyport_wreg(log_port, 0x17, &v16);
   if (enable) {
      v16 = 0x0003;
   } else {
      v16 = 0x0007;
   }
   ethsw_phyport_wreg(log_port, 0x15, &v16);
   v16 = r18s0 & ~0x0800;
   ethsw_phyport_wreg(log_port, 0x18, &v16);

   /* Re-enable APD if it was disabled by this code */
   if (apd_disabled) {
      apdv16 |= MII_1C_AUTO_POWER_DOWN;
      ethsw_phyport_wreg(log_port, MII_REGISTER_1C, &apdv16);
   }
}

void ethsw_eee_init_hw(void)
{
#if defined(GPHY_EEE_1000BASE_T_DEF)
    {
        uint16 v16;
        /* Configure EEE delays. In the bootloader, we already initialized EEE
           on the GPHY before it was taken out of reset, on 6318, nothing is needed in bootloader */
        v16 = 0x3F;
        ethsw_wreg(PAGE_CONTROL, REG_EEE_TW_SYS_TX_100, (uint8_t *)&v16, 2);
        v16 = 0x23;
        ethsw_wreg(PAGE_CONTROL, REG_EEE_TW_SYS_TX_1000, (uint8_t *)&v16, 2);
#if defined(CONFIG_BCM_GMAC)
        if (gmac_is_gmac_supported())
        {
            volatile GmacEEE_t *gmacEEEp = GMAC_EEE;
            // Reset EEE controller in GMAC
            gmacEEEp->eeeCtrl.softReset = 1;
            gmacEEEp->eeeCtrl.softReset = 0;
            // following are the default values(upon reset). If need to change, this 
            // is the place.
            gmacEEEp->eeeTx100WakeTime = 0x3F;
            gmacEEEp->eeeT1000WakeTime = 0x23;
            gmacEEEp->eeeLPIWaitTime   = 0xf424;
        }
#endif
    }
#elif defined(CONFIG_BCM_EPHY_EEE)
    {
        uint16 v16;
        /* Configure EEE delays */
        v16 = 0x3F;
        ethsw_wreg(PAGE_CONTROL, REG_EEE_TW_SYS_TX_100, (uint8_t *)&v16, 2);
        v16 = 0x23;
        ethsw_wreg(PAGE_CONTROL, REG_EEE_TW_SYS_TX_1000, (uint8_t *)&v16, 2);
    }
#endif
}

#if defined(CONFIG_BCM_EXT_SWITCH)
void extsw_eee_init(void)
{
   uint32 v32;
   int i;

   down(&bcm_ethlock_switch_config);
   /* EEE requires initialization on 53125 */
   if (KNOWN_SWITCH(pVnetDev0_g->extSwitch->switch_id)) {
      uint16 v16;
      extsw_bus_contention(1,5);

      extsw_rreg_wrap(PAGE_EEE, REG_EEE_LPI_SYM_TX_DISABLE, (uint8 *)&v16, 2);
      v16 |= 0x0f; // disable port 0 to 3
      extsw_wreg_wrap(PAGE_EEE, REG_EEE_LPI_SYM_TX_DISABLE, (uint8 *)&v16, 2);
        
      /* Change default settings */
      for (i=0; i<6; ++i) {
         /* Change the Giga EEE Sleep Timer default value from 4 uS to 400 uS */
         v32 = 0x00000190;
         extsw_wreg_wrap(PAGE_EEE, REG_EEE_SLEEP_TIMER_G+(i*4), (uint8 *)&v32, 4);
         /* Change the 100 Mbps EEE Sleep Timer default value from 40 uS to 4000 uS */
         v32 = 0x00000FA0;
         extsw_wreg_wrap(PAGE_EEE, REG_EEE_SLEEP_TIMER_FE+(i*4), (uint8 *)&v32, 4);

         /* Set the initial compatibility. This is also required in mdk, if mdk is used */
         ethsw_eee_compatibility_set(PHYSICAL_PORT_TO_LOGICAL_PORT(i, 1), 0);
      }

      /*
       * Give more time for the PHY to recover from the Low Power mode before
       * the MAC communicates with the PHY.
       */
      v32 = 0x000f4240; 
      extsw_wreg_wrap(PAGE_EEE, 0xe8, (uint8 *)&v32, 4);

      extsw_bus_contention(0,0);
   }
   up(&bcm_ethlock_switch_config);
}
#endif

void ethsw_eee_port_enable(int log_port, int enable, int linkstate)
{
   int phys_port = LOGICAL_PORT_TO_PHYSICAL_PORT(log_port);
   int unit = LOGICAL_PORT_TO_UNIT_NUMBER(log_port);
   int phy_id = enet_logport_to_phyid(log_port);
   uint16 data16;

   if ((unit == 0) && IsPhyConnected(phy_id) && IsExtPhyId(phy_id) && !IsSerdes(phy_id)) {
      // Internal switch with an external PHY
      // Only apply these settings to the 50612, rev 1 and 2.
      ethsw_phy_rreg(phy_id, MII_PHYSID1, &data16);
      if (data16 == 0x0362) {
         ethsw_phy_rreg(phy_id, MII_PHYSID2, &data16);
         if ((data16 == 0x5e61) || (data16 == 0x5e62) || (data16 == 0x5e6a) || (data16 == 0x5f6a) ||
             (data16 == 0x5e6e) || (data16 == 0x5e66)) {
            ethsw_eee_compatibility_set(log_port, linkstate);
         }
      }
   }

#if defined(GPHY_EEE_1000BASE_T_DEF)
   if (unit == 0) {
      /* Integrated switch */
      /* Ensure that EEE was enabled in bootloader */
      #define EEE_BITS (GPHY_LPI_FEATURE_EN_DEF_MASK | \
         GPHY_EEE_1000BASE_T_DEF | GPHY_EEE_100BASE_TX_DEF | \
         GPHY_EEE_PCS_1000BASE_T_DEF | GPHY_EEE_PCS_100BASE_TX_DEF)

      if ( (GPIO->RoboswGphyCtrl & EEE_BITS) == EEE_BITS ) {
         /* Only the GPHY port supports EEE on 63268 */
         if ((phys_port >= NUM_INT_EPHYS) && (phys_port < NUM_INT_PHYS)) {
            uint16 v16 = 0;

            ethsw_eee_compatibility_set(log_port, linkstate);
            if (enable) {
               /* Check if 100Base-T Bit[1] or 1000Base-T EEE Bit[2] was advertised by the partner reg 7.61 */
               /* This step is not essential since the PHY does this already */
               ethsw_phyport_c45_rreg(log_port, 7, 61, &data16);
               if (data16 & 0x6) {
                  v16 = REG_EEE_CTRL_ENABLE;
               }
            }
#if defined(CONFIG_BCM_GMAC)
            BCM_ENET_DEBUG("%s: Checking port %d req %d v16 %d d16 0x%x\n",
                           __FUNCTION__,  phys_port, enable, v16, data16);
            if (IsGmacPort(log_port) && gmac_info_pg->active && (gmac_info_pg->link_speed == 1000)) { 
                volatile GmacEEE_t *gmacEEEp = GMAC_EEE;
                gmacEEEp->eeeCtrl.enable = v16? 1: 0;
                gmacEEEp->eeeCtrl.linkUp = v16? 1: 0;
            }
            else
#endif
            {
                ethsw_wreg(PAGE_CONTROL, REG_EEE_CTRL + (phys_port << 1), (uint8_t *)&v16, 2);
            }
         }
      }
   } 
#endif
#if defined(CONFIG_BCM_EPHY_EEE)
   if (unit == 0) {
      /* Integrated switch */
      if (phys_port < NUM_INT_PHYS) {
         uint16 v16 = 0;

         if (enable) {
            /* Check if AN EEE Resolution is set */
            ethsw_ephy_shadow_read(phy_id, 3, 0x000b, &data16);
            if (data16 & 0x100) {
               v16 = REG_EEE_CTRL_ENABLE;
            }
         }
         ethsw_wreg(PAGE_CONTROL, REG_EEE_CTRL + (phys_port << 1), (uint8_t *)&v16, 2);
      }
   } 
#endif
#if defined(CONFIG_BCM_EXT_SWITCH)
   if (unit == 1) {
      /* External switch GPHYs */
      if (KNOWN_SWITCH(pVnetDev0_g->extSwitch->switch_id)) {
         uint16 v16;
         static int eee_strap = -1;

         extsw_bus_contention(1,5);

         /* Determine if eee strap is enabled (works when 8051 is disabled) */
         /* 8051 overwrites the strap setting with 0 at boot time, and overwrites it with 0x1f */
         /* the first time a link comes up, this is why we read the strap setting here */
         extsw_rreg_wrap(PAGE_EEE, REG_EEE_EN_CTRL, (uint8 *)&v16, 2);

         if ((eee_strap < 0) && linkstate) {
            eee_strap = v16;
            v16 = 0; /* Start with EEE disabled on all ports */
            extsw_wreg_wrap(PAGE_EEE, REG_EEE_EN_CTRL, (uint8 *)&v16, 2);
         }

         if (enable) {
            if (eee_strap > 0) {
               /* Check if 100Base-T Bit[1] or 1000Base-T EEE Bit[2] was advertised by the partner reg 7.61 */
               /* This step is not essential since the PHY does this already */
               ethsw_phyport_c45_rreg(log_port, 7, 61, &data16);
               if (data16 & 0x6) {
                  v16 |= (1<<phys_port);
               }
            }
         } else {
            v16 &= ~(1<<phys_port);
         }
         extsw_wreg_wrap(PAGE_EEE, REG_EEE_EN_CTRL, (uint8 *)&v16, 2);
         if (linkstate || pm_eee_enabled) {
            ethsw_eee_compatibility_set(log_port, linkstate);
         }

         extsw_bus_contention(0,0);
      }
   }
#endif
}


int ethsw_eee_get_by_log_port(int log_port, unsigned int *enable)
{
    uint16 val16;
    int phy_id, ret;
   
    val16 = 0;
    phy_id = enet_logport_to_phyid(log_port);
    ret = 0;

    down(&bcm_ethlock_switch_config);

#if defined(CONFIG_BCM_EPHY_EEE)
    ret = ethsw_eee_ephy_get(phy_id, enable);
#else
    ethsw_phyport_c45_rreg(log_port, 7, 60, &val16);
    *enable = (val16 & (1 << 1) || val16 & (1 << 2));
#endif

    up(&bcm_ethlock_switch_config);

    return ret;
}

int ethsw_eee_resolution_get_by_log_port(int log_port, unsigned int *enable)
{
    uint16 val16;
    int phy_id, ret;
   
    val16 = 0;
    phy_id = enet_logport_to_phyid(log_port);
    ret = 0;

    down(&bcm_ethlock_switch_config);

#if defined(CONFIG_BCM_EPHY_EEE)
    ret = ethsw_eee_resolution_ephy_get(phy_id, enable);
#else
    ethsw_phyport_c45_rreg(log_port, 7, 0x803e, &val16);
    *enable = (val16 & (1 << 1) || val16 & (1 << 2));
#endif

    up(&bcm_ethlock_switch_config);

    return ret;
}

static void __ethsw_eee_set_by_log_port(int log_port, unsigned int enable)
{
   uint16 val16;
   int phy_id = enet_logport_to_phyid(log_port);

   ethsw_eee_extra_phy_init(phy_id);

#if defined(CONFIG_BCM_EPHY_EEE)
   ethsw_eee_ephy_set(phy_id, enable);
#else
   val16 = enable?6:0;
   ethsw_phyport_c45_wreg(log_port, 7, 60, &val16);
#endif

   /* Restart autoneg - Required on every PHY to activate the EEE settings */
   ethsw_phyport_rreg(log_port, MII_CONTROL, &val16);
   val16 |= MII_CONTROL_RESTART_AUTONEG;
   ethsw_phyport_wreg(log_port, MII_CONTROL, &val16);
}

void ethsw_eee_set_by_log_port(int log_port, unsigned int enable)
{
    down(&bcm_ethlock_switch_config);
    __ethsw_eee_set_by_log_port(log_port, enable);
    up(&bcm_ethlock_switch_config);
}

static void __ethsw_eee_set_all(unsigned int enable)
{
   int i;
   int phys_port;
   int unit;

   /* Enable/disable EEE in the PHYs */
   for ( i = LOGICAL_PORT_START; i <= LOGICAL_PORT_END; i++) {
      phys_port = LOGICAL_PORT_TO_PHYSICAL_PORT(i);
      unit = LOGICAL_PORT_TO_UNIT_NUMBER(i);

      if (pVnetDev0_g->EnetInfo[unit].sw.port_map & (1 << phys_port)) {
         __ethsw_eee_set_by_log_port(i, enable);
      }
   }
}

static void ethsw_eee_set_all(unsigned int enable)
{
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);

    down(&bcm_ethlock_switch_config);
    __ethsw_eee_set_all(enable);

    /* Clear the global variable since autoneg causes the interfaces to relink */
    priv->eee_enable_request_flag[0] = 0;
    priv->eee_enable_request_flag[1] = 0;
    up(&bcm_ethlock_switch_config);
}

void ethsw_eee_init(void)
{
   ethsw_eee_set_all(pm_eee_enabled);
   printk("Energy Efficient Ethernet: %s\n", pm_eee_enabled?"Enabled":"Disabled");
}

void ethsw_eee_process_delayed_enable_requests(void)
{
   struct net_device *dev = vnet_dev[0];
   BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
   int i;

   /* Process enable requests that have been here for more than 1 second */
   if (priv->eee_enable_request_flag[1]) {
     down(&bcm_ethlock_switch_config);
     for ( i = LOGICAL_PORT_START; i <= LOGICAL_PORT_END; i++) {
         if (priv->eee_enable_request_flag[1] & (1<<i)) {
            ethsw_eee_port_enable(i, 1, priv->linkState & (1 << i));
         }
      }
      up(&bcm_ethlock_switch_config);
   }

   /* Now delay recent requests by one polling interval (1 second) */
   priv->eee_enable_request_flag[1] = priv->eee_enable_request_flag[0];
   priv->eee_enable_request_flag[0] = 0;
}
#endif
