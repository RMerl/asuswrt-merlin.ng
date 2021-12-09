#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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
 ****************************************************************************
 * File Name  : bcm_sata.c
 *
 * Description: This file contains the initilzation and registration routines
 * to enable sata controller on bcm63xxx boards. 
 *
 *
 ***************************************************************************/

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/bug.h>
#include <linux/ahci_platform.h>

#include <bcm_intr.h>
#include <bcm_map_part.h>
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM94908)   
#include <bcm_otp.h>
#endif
#include <pmc_sata.h>
#include <board.h>

/* macros to read write, to registers, with memory barriers to avoid reordering */
#define BDEV_RD(x)      (*((volatile unsigned *)(x))); mb()
#define BDEV_WR(x, y)   do { *((volatile unsigned *)(x)) = (y); mb(); } while (0)

/*TODO move these reg definitions to map_part.h */

#define SATA_HBA_BASE_ADDR  ((unsigned long)SATA_BASE) 

#define SATA_TOP_CTRL      (SATA_HBA_BASE_ADDR+0x0040)
#define SATA_PORT0_PCB     (SATA_HBA_BASE_ADDR+0x0100)
#define SATA_AHCI_BASE     (SATA_HBA_BASE_ADDR+0x2000) 
#define SATA_AHCI_GHC      (SATA_HBA_BASE_ADDR+0x2000) 
#define SATA_AHCI_PORT0_S1 (SATA_HBA_BASE_ADDR+0x2100)

#define SATA_AHCI_GHC_PHYS (SATA_PHYS_BASE+0x2000) 

#define SATA_MEM_SIZE		0x00002000

/* SATA_TOP_CTRL regsiters */
#define SATA_TOP_CTRL_BUS_CTRL      (SATA_TOP_CTRL+0x04)

/* SATA_PORT0_AHCI_S1 registers */
#define SATA_PORT0_AHCI_S1_PXIS     (SATA_AHCI_PORT0_S1+0x10)
#define SATA_PORT0_AHCI_S1_PXIE     (SATA_AHCI_PORT0_S1+0x14)
#define SATA_PORT0_AHCI_S1_PXCMD    (SATA_AHCI_PORT0_S1+0x18)

/* GHC regs */
#define GHC_HBA_CAP                 (SATA_AHCI_GHC+0x00) /* host capabilities */
#define GHC_GLOBAL_HBA_CONTROL      (SATA_AHCI_GHC+0x04) /* global host control */
#define GHC_INTERRUPT_STATUS        (SATA_AHCI_GHC+0x08) /* interrupt status */
#define GHC_PORTS_IMPLEMENTED       (SATA_AHCI_GHC+0x0c) /* bitmap of implemented ports */
#define GHC_HOST_VERSION            (SATA_AHCI_GHC+0x10) /* AHCI spec. version compliancy */

/* Phy reg */
#define PORT0_SATA3_PCB_REG0        (SATA_PORT0_PCB+0x0200)
#define PORT0_SATA3_PCB_REG1        (SATA_PORT0_PCB+0x0204)
#define PORT0_SATA3_PCB_REG2        (SATA_PORT0_PCB+0x0208)
#define PORT0_SATA3_PCB_REG3        (SATA_PORT0_PCB+0x020c)
#define PORT0_SATA3_PCB_REG4        (SATA_PORT0_PCB+0x0210)
#define PORT0_SATA3_PCB_REG5        (SATA_PORT0_PCB+0x0214)
#define PORT0_SATA3_PCB_REG6        (SATA_PORT0_PCB+0x0218)
#define PORT0_SATA3_PCB_REG7        (SATA_PORT0_PCB+0x021c)
#define PORT0_SATA3_PCB_REG8        (SATA_PORT0_PCB+0x0220)
#define PORT0_SATA3_PCB_BLOCK_ADDR  (SATA_PORT0_PCB+0x023C)

#define PCB_REG(x) (unsigned long)(PORT0_SATA3_PCB_REG0 + x*4)

#define SATA3_TXPMD_REG_BANK    0x01a0

static void write_2_pcb_block(unsigned long reg_addr, unsigned value, unsigned pcb_block)
{
    BDEV_WR(PORT0_SATA3_PCB_BLOCK_ADDR, pcb_block);
    BDEV_WR(reg_addr, value);
}

static unsigned read_from_pcb_block(unsigned long reg_addr, unsigned pcb_block)
{
    unsigned int value;
    BDEV_WR(PORT0_SATA3_PCB_BLOCK_ADDR, pcb_block);
    value = BDEV_RD(reg_addr);
    return value;
}

static __init void GetFreqLock( void )
{
    uint32_t regData;
    int i = 10;

   printk("writing PORT0_SATA3_PCB_BLOCK_ADDR\n");
    
    write_2_pcb_block(PORT0_SATA3_PCB_REG7, 0x873, 0x60);

    write_2_pcb_block(PORT0_SATA3_PCB_REG6, 0xc000, 0x60);

    write_2_pcb_block(PORT0_SATA3_PCB_REG1, 0x3089, 0x50);
   udelay(100);
    write_2_pcb_block(PORT0_SATA3_PCB_REG1, 0x3088, 0x50);
   udelay(1000);
   //// Done with PLL ratio change and re-tunning

    write_2_pcb_block(PORT0_SATA3_PCB_REG2, 0x3000, 0xE0);
    write_2_pcb_block(PORT0_SATA3_PCB_REG6, 0x3000, 0xE0);

   udelay(1000);
    write_2_pcb_block(PORT0_SATA3_PCB_REG3, 0x32, 0x50);

    write_2_pcb_block(PORT0_SATA3_PCB_REG4, 0xA, 0x50);

    write_2_pcb_block(PORT0_SATA3_PCB_REG6, 0x64, 0x50);
   
   udelay(1000);
   BDEV_WR(PORT0_SATA3_PCB_BLOCK_ADDR, 0x00);
    wmb();
   
   regData = BDEV_RD(PORT0_SATA3_PCB_REG1);

   while (i && ((regData & 0x1000) == 0))
   {
      regData = BDEV_RD(PORT0_SATA3_PCB_REG1);
      udelay(1000);
      i--;
   }
   printk("INFO: PLL lock for port0 detected %0x...\n", regData);
}

static __init void sata_sim_init(void)
{
	BDEV_WR(GHC_GLOBAL_HBA_CONTROL, 0x80000001);
    mdelay(1);
	BDEV_WR(GHC_GLOBAL_HBA_CONTROL, 0x80000000);
    mdelay(10);

	BDEV_WR(SATA_PORT0_AHCI_S1_PXIS, 0x7fffffff);
	BDEV_WR(GHC_INTERRUPT_STATUS, 0x7fffffff);
	BDEV_WR(SATA_PORT0_AHCI_S1_PXIE, 0x7fffffff);

	BDEV_WR(SATA_PORT0_AHCI_S1_PXCMD, 0x00000010);
    /* setup endianess */
	BDEV_WR(SATA_TOP_CTRL_BUS_CTRL, 0x00000000);
}

static void bcm_dev_release(struct device *dev)
{
    put_device(dev->parent);
}

static u64 bcm_ahci_dmamask = DMA_BIT_MASK(32);
static struct platform_device *bcm_ahci_pdev; 

static __init struct platform_device *bcm_add_sata_pdev(int id,
                        uint32_t mem_base, uint32_t mem_size, int irq,
                        const char *devname, void *private_data)
{
    struct resource res[2];
    struct platform_device *pdev;

    memset(&res, 0, sizeof(res));
    res[0].start = mem_base;
    res[0].end   = mem_base + (mem_size -1);
    res[0].flags = IORESOURCE_MEM;

    res[1].flags = IORESOURCE_IRQ;
    res[1].start = res[1].end = irq;

    pdev = platform_device_alloc(devname, id);
    if(!pdev)
    {
        printk(KERN_ERR "Error Failed to allocate platform device for devname=%s id=%d\n",
                devname, id);
        return NULL;
    }

    platform_device_add_resources(pdev, res, 2);

    pdev->dev.dma_mask = (u64 *)&bcm_ahci_dmamask;
    pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
#if defined(CONFIG_BCM963158) && defined(CONFIG_BCM_GLB_COHERENCY)
    pdev->dev.archdata.dma_coherent = 1; 
#endif

    pdev->dev.release = bcm_dev_release;

    if(private_data)
    {
        pdev->dev.platform_data = private_data;
    }


    if(platform_device_add(pdev))
    {
        printk(KERN_ERR "Error Failed to add platform device for devname=%s id=%d\n",
                devname, id);
        return NULL;
    }

    return pdev;
}


static __init int bcm_add_sata(void)
{
    if( !kerSysGetSataPortEnable() )
    { 
        printk("++++ No SATA Hardware Detected\n");
        return -1;
    }
    else
    {
        printk("++++ Powering up SATA block\n");

        pmc_sata_power_up();
        mdelay(1);

        GetFreqLock();
        mdelay(1);

        sata_sim_init();
        mdelay(1);

        /*enable SSC */
        {
            int rvalue;

            rvalue = read_from_pcb_block(PCB_REG(1), SATA3_TXPMD_REG_BANK);
            rvalue |= 0x3;
            write_2_pcb_block( PCB_REG(1), rvalue, SATA3_TXPMD_REG_BANK);
        }

        bcm_ahci_pdev = bcm_add_sata_pdev(0, SATA_AHCI_GHC_PHYS,
            SATA_MEM_SIZE, INTERRUPT_ID_SATAC, "ahci", NULL);

        return 0;
    }
}

#if defined CONFIG_SATA_AHCI_MODULE
static void bcm_mod_cleanup(void)
{
    if(bcm_ahci_pdev)
        platform_device_del(bcm_ahci_pdev);
    pmc_sata_power_down();
    mdelay(1);
}

module_init(bcm_add_sata);
module_exit(bcm_mod_cleanup);

MODULE_LICENSE("GPL");
#else
arch_initcall(bcm_add_sata);
#endif

MODULE_LICENSE("GPL");
#endif
