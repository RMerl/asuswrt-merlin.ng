/*
 <:copyright-BRCM:2011:DUAL/GPL:standard
 
    Copyright (c) 2011 Broadcom 
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
#include "bcm_OS_Deps.h"
#include "bcmtypes.h"
#include "bcmmii.h"
#include "ethsw_phy.h"
#if defined(CONFIG_BCM947189)
#define _BCMENET_LOCAL_
#include "bcmenet.h"
#include "bcmswshared.h"
#else
#include "bcmenet_common.h"
#endif
#include "bcmsw.h"
#include "boardparms.h"
#include "bcmswaccess.h"

#if defined(CONFIG_BCM947189)

extern spinlock_t bcm_ethlock_phy_access;
static DEFINE_MUTEX(bcm_phy_mutex);

void ethsw_ephy_shadow_rw(int phy_id, int bank, uint16 reg, uint16 *data, int write){};
int ethsw_get_emac_index(int phy_id)
{
    ETHERNET_MAC_INFO *EnetInfo = EnetGetEthernetMacInfo();

    if (phy_id == PSEUDO_PHY_ADDR)
    {
        return global.pVnetDev0_g->extSwitch->connected_to_internalPort;
    }

    if (EnetInfo[0].sw.phy_id[0] != 0 && (EnetInfo[0].sw.phy_id[0] & BCM_PHY_ID_M) == phy_id)
        return 0;

    if (EnetInfo[0].sw.phy_id[1] != 0 && (EnetInfo[0].sw.phy_id[1] & BCM_PHY_ID_M) == phy_id)
        return 1;

    /* connect to external switch */
    return global.pVnetDev0_g->extSwitch->connected_to_internalPort;
}

int ethsw_phy_rw_reg32_chip(int phy_id, u32 reg, uint32 *data, int ext_bit, int rd)
{
    uint32 reg_value, read_back; 
    u16 v16;
    unsigned long flags;
    static int needIsrDelay = 0;
    int atomic;
    int ethcore;
    volatile EnetCoreMisc *misc_regs;
    

    phy_id &= BCM_PHY_ID_M;
    ethcore = ethsw_get_emac_index(phy_id);
    if (ethcore == 0)
        misc_regs = ENET_CORE0_MISC;
    else if (ethcore == 1)
        misc_regs = ENET_CORE1_MISC;
    else
    {
        printk("%s: Fatal error: Ethernet core %d doesn't exist(phy_id=%d)\n", __FUNCTION__, ethcore, phy_id);
        return 0;
    }

    v16 = (*data) & 0xffff;
    atomic = preempt_count() || irqs_disabled();

    if (atomic == 0)
    {
        mutex_lock(&bcm_phy_mutex);
    }

    spin_lock_irqsave(&bcm_ethlock_phy_access, flags);

    /* If this is an ISR and a task initiated access then went to sleep, 
        we have to wait here until outstanding access finishes */
    if(atomic && needIsrDelay) 
    {
        udelay(60);
    }

taskRetry:
    reg_value = 0;
    misc_regs->phycontrol = (misc_regs->phycontrol & ~PC_EPA_MASK) | phy_id;
    reg_value = PA_START | (phy_id << PA_ADDR_SHIFT) | (reg << PA_REG_SHIFT) |
                (rd? (0) : (PA_WRITE | v16));
    
    misc_regs->phyaccess = reg_value;

    if(atomic == 0)
    {
        needIsrDelay = 1;
waitFinish:
        spin_unlock_irqrestore(&bcm_ethlock_phy_access, flags);
        msleep(1);
        spin_lock_irqsave(&bcm_ethlock_phy_access, flags);

        /* Now compare the R/W control register contents,
            if it changed, IRS has cut in and changed the register,
            we need to retry the operation. If the register content
            is the same, either no IRS cut in, or even IRS cut in it did the
            same R/W operation, so we can use the result directly in either case. */
        reg_value &= (~PA_START);
        read_back = misc_regs->phyaccess;
        read_back &= (rd? (~PA_DATA_MASK):(0xffffffff));
       
        if (read_back != reg_value)
        {
            if (read_back & PA_START)
                goto waitFinish;
            else
                goto taskRetry;
        }
    }
    else
    {
        udelay(60);
    }

    if (rd)
    {
        reg_value = misc_regs->phyaccess;
        v16 = (u16)(reg_value & PA_DATA_MASK);
        *data = v16;
    }

    needIsrDelay = 0;

    spin_unlock_irqrestore(&bcm_ethlock_phy_access, flags);
    if(atomic == 0)
    {
        mutex_unlock(&bcm_phy_mutex);
    }

    return 0;
}

#else
extern spinlock_t bcm_ethlock_phy_access;
static DEFINE_MUTEX(bcm_phy_mutex);

/*      
    Note: PHY register access need to wait 50us to complete.
    Hardware does not allow another MDIO R/W to be requested before last one
    finishes, or it will screw up forever. So we need to completely serialize
    all access passing through here.
*/
int ethsw_phy_rw_reg32_chip(int phy_id, u32 reg, uint32 *data, int ext_bit, int rd)
{
    uint32 reg_value, read_back; 
    u16 v16;
    unsigned long flags;
    static int needIsrDelay = 0;
    int atomic;

    phy_id &= BCM_PHY_ID_M;
    v16 = (*data) & 0xffff;
    atomic = preempt_count() || irqs_disabled();

    if (atomic == 0)
    {
        mutex_lock(&bcm_phy_mutex);
    }

    spin_lock_irqsave(&bcm_ethlock_phy_access, flags);

    /* If this is an ISR and a task initiated access then went to sleep, 
        we have to wait here until outstanding access finishes */
    if(atomic && needIsrDelay) 
    {
        udelay(60);
    }

taskRetry:
    reg_value = 0;
    ethsw_wreg(PAGE_CONTROL, REG_MDIO_CTRL_ADDR, (uint8 *)&reg_value, 4);

    reg_value = (ext_bit? REG_MDIO_CTRL_EXT: 0) |
        ((phy_id << REG_MDIO_CTRL_ID_SHIFT) & REG_MDIO_CTRL_ID_MASK) |
        (reg  << REG_MDIO_CTRL_ADDR_SHIFT)|
        (rd? (REG_MDIO_CTRL_READ) : (REG_MDIO_CTRL_WRITE | v16));
    ethsw_wreg(PAGE_CONTROL, REG_MDIO_CTRL_ADDR, (uint8 *)&reg_value, 4);

    if(atomic == 0)
    {
        needIsrDelay = 1;
        spin_unlock_irqrestore(&bcm_ethlock_phy_access, flags);
        msleep(1);
        spin_lock_irqsave(&bcm_ethlock_phy_access, flags);

        /* Now compare the R/W control register contents,
            if it changed, IRS has cut in and changed the register,
            we need to retry the operation. If the register content
            is the same, either no IRS cut in, or even IRS cut in it did the
            same R/W operation, so we can use the result directly in either case. */
        ethsw_rreg(PAGE_CONTROL, REG_MDIO_CTRL_ADDR, (uint8 *)&read_back, 4);
        if (read_back != reg_value)
        {
            goto taskRetry;
        }
    }
    else
    {
        udelay(60);
    }

    if (rd)
    {
        ethsw_rreg(PAGE_CONTROL, REG_MDIO_DATA_ADDR, (uint8 *)&v16, 2);
        *data = v16;
    }

    needIsrDelay = 0;

    spin_unlock_irqrestore(&bcm_ethlock_phy_access, flags);
    if(atomic == 0)
    {
        mutex_unlock(&bcm_phy_mutex);
    }

    return 0;

}

extern spinlock_t bcm_ethlock_phy_shadow;
void ethsw_ephy_shadow_rw(int phy_id, int bank, uint16 reg, uint16 *data, int write)
{
    unsigned long flags;

    spin_lock_irqsave(&bcm_ethlock_phy_shadow, flags);
    _ethsw_ephy_shadow_rw(phy_id, bank, reg, data, write);
    spin_unlock_irqrestore(&bcm_ethlock_phy_shadow, flags);
}
#endif
MODULE_LICENSE("GPL");

