/*
 * <:copyright-BRCM:2014:DUAL/GPL:standard
 * 
 *    Copyright (c) 2014 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

/* These functions are meant to be used from Linux only. They access
 * hardware registers in the PLCPHY region, which aren't directly
 * addressable and thus require explicit memory mappings. */
#ifndef _CFE_

#include "boardparms.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <bcm_map_part.h>

/*
 * acb_write_wait_end
 *
 * Writes an AFE register
 *
 * Parameters:
 *   addr:  Register address
 *   dataw: Value to write
 *
 * Returns: 0 if write was OK
 *          1 if write failed
 *
 * NOTE: This function performs an ioremap_nocache to access PLCPHY
 * adresses
 */
static int acb_write_wait_end(unsigned int addr, unsigned char dataw)
{
    int i = 0;
    int timeout = 1000;
    int retval = 0;

    /* PLCPHY.PLCPHY_REGISTERS.QBUS2ACB_BRIDGE.ACB_INSTRUCTION */
    unsigned int *acb_instruction = ioremap_nocache(0x80b00004, 4);
    /* PLCPHY.PLCPHY_REGISTERS.QBUS2ACB_BRIDGE.ACB_STATUS */
    unsigned int *acb_status = ioremap_nocache(0x80b00008, 4);

    *acb_instruction = ((addr << 16) | dataw) & ~(1 << 31);

    while ((*acb_status >> 30) & 1) {
        udelay(1000);
        if (i++ > timeout) {
          printk("acb_read_wait_end timed out\n");
          retval = 1;
          break;
        }
    }

    if (*acb_status & (1 << 31 | 1 << 29 | 1 << 28)) {
        printk("ERROR: acb_write_wait_end status=0x%x\n",
                              (unsigned int)*acb_status);
        retval = 1;
    }
    iounmap(acb_instruction);
    iounmap(acb_status);
    return retval;
}

#if defined(CONFIG_PCI)
/*
 * plcphy_pcie_afe_init
 *
 * Initializes AFE clocks for PCIe.
 *
 * NOTE: Only for PCI builds
 */
static void plcphy_pcie_afe_init(void)
{
    unsigned int afe_pplo_lock;

    /* PLCPHY.PLCPHY_REGISTERS.PLCPHY_SYS_CTRL.AFE_STATUS_REG */
    unsigned int *afe_status = ioremap_nocache(0x80300074, 4);
    /* PLCPHY.PLCPHY_REGISTERS.QBUS2ACB_BRIDGE.ACB_CONFIG. */
    unsigned int *acb_config = ioremap_nocache(0x80b00000, 4);
    unsigned int *rom_id = ioremap_nocache(PLC_ROM_CHECK_ADDR, 4);

    *acb_config = 0xa;
    /* PLL Configuration, (to enable 25MHz XTAL, 4.8GHz DCO,
     * 2.4GHz PI clock, 100MHz PCIe, 400MHz PI update clock) */
    acb_write_wait_end(0x500, 0x02); acb_write_wait_end(0x501, 0x04);
    acb_write_wait_end(0x502, 0x7F); acb_write_wait_end(0x503, 0x02);
    acb_write_wait_end(0x504, 0x7d); acb_write_wait_end(0x505, 0xb1);
    acb_write_wait_end(0x506, 0x07); acb_write_wait_end(0x593, 0x06);
    acb_write_wait_end(0x592, 0x00); acb_write_wait_end(0x591, 0x00);
    acb_write_wait_end(0x590, 0x00); acb_write_wait_end(0x58c, 0x10);
    acb_write_wait_end(0x58b, 0x10); acb_write_wait_end(0x58a, 0x10);
    acb_write_wait_end(0x589, 0x06); acb_write_wait_end(0x588, 0x18);
    acb_write_wait_end(0x587, 0x06); acb_write_wait_end(0x586, 0x04);
    acb_write_wait_end(0x585, 0x15);

    if (rom_id[0] == PLC_ROM_ID_A0)
    {
        acb_write_wait_end(0x584, 0x8B);
    }
    else
    {
        acb_write_wait_end(0x584, 0xCB);
    }

    acb_write_wait_end(0x583, 0x31); acb_write_wait_end(0x582, 0x88);
    acb_write_wait_end(0x581, 0x00); acb_write_wait_end(0x5C6, 0x08);

    msleep(35);

    afe_pplo_lock = *afe_status;
    if(!(afe_pplo_lock & 0x100))
    {
      printk("ERROR: 100 MHz source PLL is not locked!\n");
    }

    if (rom_id[0] == PLC_ROM_ID_A0)
    {
        acb_write_wait_end(0x5C3, 0x10);
        acb_write_wait_end(0x543, 0x10);
    }
    else
    {
        acb_write_wait_end(0x5C3, 0x18);
        acb_write_wait_end(0x543, 0x10);
    }
    acb_write_wait_end(0x5c4, 0x30); acb_write_wait_end(0x544, 0x0c);

    iounmap(afe_status);
    iounmap(acb_config);
    iounmap(rom_id);
}

/*
 * stop_100mhz_afe_clock
 *
 * Stops the 100 MHz AFE clock to reduce AFE_PI current in case no PCIe
 * card is detected.
 *
 * NOTE: Only for PCI builds
 */
void stop_100mhz_afe_clock(void)
{
    acb_write_wait_end(0x58b, 0);
}
#endif

int bcm_misc_hw_init(void)
{
#if defined(CONFIG_PCI)
    plcphy_pcie_afe_init();
#endif

    return 0;
}

core_initcall(bcm_misc_hw_init);
#if defined(CONFIG_PCI)
EXPORT_SYMBOL(stop_100mhz_afe_clock);
#endif

#endif /* ifndef _CFE_ */
