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
 * MDIO driver for BCM47189
 */

#include "os_dep.h"
#include "bcm_map_part.h"
#include "bcm_misc_hw_init.h"
#include "mdio_drv_impl3.h"

#define MDIO_POLL_PERIOD        10

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


int32_t mdio_read_c22_register(uint8_t core_num, uint32_t phy_addr, uint32_t reg_addr, uint16_t *data_read)
{
    uint32_t reg_read;
    int countdown = 100;
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(core_num);

    /* Issue the read command*/
    misc_regs->phycontrol = (misc_regs->phycontrol & ~PC_EPA_MASK) | phy_addr;
    misc_regs->phyaccess = PA_START | (phy_addr << PA_ADDR_SHIFT) |
                           (reg_addr << PA_REG_SHIFT);

    /* Wait for it to complete */
    while ((misc_regs->phyaccess & PA_START) && (countdown-- > 0))
        udelay(MDIO_POLL_PERIOD);

    reg_read = misc_regs->phyaccess;
    if (reg_read & PA_START) {
        printk("Error: mii_read_ext did not complete\n");
        reg_read = 0xffffffff;
    }

    *data_read = reg_read & PA_DATA_MASK;

    return MDIO_OK;
}

int32_t mdio_write_c22_register(uint8_t core_num, uint32_t phy_addr, uint32_t reg_addr, uint16_t data_write)
{
    uint16 countdown = 100;
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(core_num);

    misc_regs->phycontrol = (misc_regs->phycontrol & ~PC_EPA_MASK)
                                  | phy_addr;

    /* Clear mdioint bit of intstatus */
    misc_regs->intstatus = I_MDIO;
    if ((misc_regs->intstatus & I_MDIO) == 0) {
        /* Issue the write command */
        misc_regs->phyaccess = PA_START | PA_WRITE
                               | (phy_addr << PA_ADDR_SHIFT)
                               | (reg_addr << PA_REG_SHIFT) | data_write;

        /* Wait for it to complete */
        while ((misc_regs->phyaccess & PA_START) && (countdown-- > 0))
            udelay(MDIO_POLL_PERIOD);
    }
    return 0;
}

int32_t mdio_read_c45_register(uint8_t core_num, uint32_t port_addr, uint32_t dev_addr, uint16_t dev_offset, uint16_t *data_read)
{
    return 0;
}

int32_t mdio_write_c45_register(uint8_t core_num, uint32_t port_addr, uint32_t dev_addr,uint16_t dev_offset, uint16_t data_write)
{
    return 0;
}
