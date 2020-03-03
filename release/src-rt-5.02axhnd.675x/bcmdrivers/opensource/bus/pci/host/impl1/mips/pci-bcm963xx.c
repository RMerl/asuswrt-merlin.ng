#if defined(CONFIG_BCM_KF_PCI_FIXUP)
/*
<:copyright-BRCM:2012:GPL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <bcm_map_part.h>
#include <bcmpci.h>
#include <boardparms.h>
#include <board.h>
#if defined(CONFIG_BCM963381) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
#include <pmc_pcie.h>
#include <pmc_drv.h>
#endif
#include <shared_utils.h>  

#include <pcie_common.h>
#include <pcie-bcm963xx.h>

extern struct pci_ops bcm63xx_pci_ops;
static struct resource bcm_pci_io_resource = {
    .name   = "bcm63xx pci IO space",
    .start  = BCM_PCI_IO_BASE,
    .end    = BCM_PCI_IO_BASE + BCM_PCI_IO_SIZE - 1,
    .flags  = IORESOURCE_IO
};

static struct resource bcm_pci_mem_resource = {
    .name   = "bcm63xx pci memory space",
    .start  = BCM_PCI_MEM_BASE,
    .end    = BCM_PCI_MEM_BASE + BCM_PCI_MEM_SIZE - 1,
    .flags  = IORESOURCE_MEM
};

struct pci_controller bcm63xx_controller = {
    .pci_ops	= &bcm63xx_pci_ops,
    .io_resource	= &bcm_pci_io_resource,
    .mem_resource	= &bcm_pci_mem_resource,
};

#if defined(PCIEH)
extern struct pci_ops bcm63xx_pcie_ops;
static struct resource bcm_pcie_io_resource = {
    .name   = "bcm63xx pcie null io space",
    .start  = 0,
    .end    = 0,
    .flags  = 0
};

static struct resource bcm_pcie_mem_resource = {
    .name   = "bcm63xx pcie memory space",
    .start  = BCM_PCIE_MEM1_BASE,
    .end    = BCM_PCIE_MEM1_BASE + BCM_PCIE_MEM1_SIZE - 1,
    .flags  = IORESOURCE_MEM
};

struct pci_controller bcm63xx_pcie_controller = {
    .pci_ops	= &bcm63xx_pcie_ops,
    .io_resource	= &bcm_pcie_io_resource,
    .mem_resource	= &bcm_pcie_mem_resource,
};

#if defined(PCIEH_1)
static struct resource bcm_pcie1_mem_resource = {
    .name   = "bcm63xx pcie memory space",
    .start  = BCM_PCIE_MEM2_BASE,
    .end    = BCM_PCIE_MEM2_BASE + BCM_PCIE_MEM2_SIZE - 1,
    .flags  = IORESOURCE_MEM
};

struct pci_controller bcm63xx_pcie1_controller = {
    .pci_ops	= &bcm63xx_pcie_ops,
    .io_resource	= &bcm_pcie_io_resource,
    .mem_resource	= &bcm_pcie1_mem_resource,
};
#endif
#endif

#if defined(CONFIG_BCM963268)
extern unsigned int pci63xx_wlan_soft_config_space[WLAN_ONCHIP_DEV_NUM][WLAN_ONCHIP_PCI_HDR_DW_LEN];
static int __init bcm63xx_pci_swhdr_patch(void)
{
    /* modify sw pci hdr for different board for onchip wlan */
    int i;
    for (i = 0; i <WLAN_ONCHIP_DEV_NUM; i++) {       
        BpUpdateWirelessPciConfig(pci63xx_wlan_soft_config_space[i][0],pci63xx_wlan_soft_config_space[i],WLAN_ONCHIP_PCI_HDR_DW_LEN);
    }
    return 0;
}
#endif

#if defined(PCIEH)
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM960333) || defined(CONFIG_BCM96838) || defined(PCIE3_CORE)
/* 
  Function pcie_mdio_read (phyad, regad)

   Parameters:
     phyad ... MDIO PHY address (typically 0!)
     regad ... Register address in range 0-0x1f

   Description:
     Perform PCIE MDIO read on specified PHY (typically 0), and Register.
     Access is through an indirect command/status mechanism, and timeout
     is possible. If command is not immediately complete, which would
     be typically the case, one more attempt is made after a 1ms delay.

   Return: 16-bit data item or 0xdead on MDIO timeout
*/
uint16 bcm63xx_pcie_mdio_read (int port, uint16 phyad, uint16 regad) 
{
    int timeout;
    uint32 data;
    uint16 retval;
    volatile PcieBlk1000Regs *RcDLReg;
#if defined(PCIEH_1)
    RcDLReg = port ? PCIEH_1_BLK_1000_REGS : PCIEH_BLK_1000_REGS;
#else 
    RcDLReg = PCIEH_BLK_1000_REGS; 
#endif
    /* Bit-20=1 to initiate READ, bits 19:16 is the phyad, bits 4:0 is the regad */
    data = 0x100000;
    data = data |((phyad & 0xf)<<16);
    data = data |(regad & 0x1F);

    RcDLReg->mdioAddr = data;
    /* critical delay */
    udelay(1000);

    timeout = 2;
    while (timeout-- > 0) {
        data = RcDLReg->mdioRdData;
        /* Bit-31=1 is DONE */
        if (data & 0x80000000)
            break;
        timeout = timeout - 1;
        udelay(1000);
    }

    if (timeout == 0) {
        retval = 0xdead;
    }else 
        /* Bits 15:0 is read data*/
        retval = (data&0xffff);

    return retval;
}

/* 
 Function pcie_mdio_write (phyad, regad, wrdata)

   Parameters:
     phyad ... MDIO PHY address (typically 0!)
     regad  ... Register address in range 0-0x1f
     wrdata ... 16-bit write data

   Description:
     Perform PCIE MDIO write on specified PHY (typically 0), and Register.
     Access is through an indirect command/status mechanism, and timeout
     is possible. If command is not immediately complete, which would
     be typically the case, one more attempt is made after a 1ms delay.

   Return: 1 on success, 0 on timeout
*/
int bcm63xx_pcie_mdio_write (int port, uint16 phyad, uint16 regad, uint16 wrdata)
{
    int timeout;
    uint32 data;
    volatile PcieBlk1000Regs *RcDLReg;
#if defined(PCIEH_1)
    RcDLReg = port ? PCIEH_1_BLK_1000_REGS : PCIEH_BLK_1000_REGS;
#else 
    RcDLReg = PCIEH_BLK_1000_REGS; 
#endif

    /* bits 19:16 is the phyad, bits 4:0 is the regad */
    data = ((phyad & 0xf) << 16);
    data = data | (regad & 0x1F);

    RcDLReg->mdioAddr = data;
    udelay(1000);

    /* Bit-31=1 to initial the WRITE, bits 15:0 is the write data */
    data = 0x80000000;
    data = data | (wrdata & 0xFFFF);

    RcDLReg->mdioWrData = data;
    udelay(1000);

    /* Bit-31=0 when DONE */
    timeout = 2;
    while (timeout-- > 0) {

        data = RcDLReg->mdioWrData;

        /* CTRL1 Bit-31=1 is DONE */
        if ((data & 0x80000000) == 0 )
            break;

        timeout = timeout - 1;
        udelay(1000);
    }

    if (timeout == 0){
        return 0;
    } else 
        return 1;
}

#if defined(PCIE3_CORE)
struct bcm963xx_pcie_hcd bcm63xx_pcie_hcd_cb[NUM_CORE] = {0};

static void bcm63xx_pcie_hcd_init(int port)
{
	struct bcm963xx_pcie_hcd *pdrv;

	pdrv = &bcm63xx_pcie_hcd_cb[port];

	pdrv->core_id = port;
	bcm963xx_pcie_init_hc_cfg(pdrv);

	return;
}
static void bcm63xx_pcie_core_set_speed(int port)
{
	volatile PcieBlk428Regs *blk428Regs;
	volatile PcieRegs *pcieRegs;
	u8 speed;

	speed = bcm63xx_pcie_hcd_cb[port].hc_cfg.speed;

	if (speed) {
#if defined(PCIEH_1)
	    blk428Regs = port ? PCIEH_1_BLK_428_REGS : PCIEH_BLK_428_REGS;
	    pcieRegs = port ? PCIEH_1_REGS : PCIEH_REGS;
#else
	    blk428Regs = PCIEH_BLK_428_REGS;
	    pcieRegs = PCIEH_REGS;
#endif

	    blk428Regs->linkCapability &= ~PCIE_IP_BLK428_LINK_CAPBILITY_MAX_LINK_SPEED_MASK;
	    blk428Regs->linkCapability |= speed;

	    pcieRegs->linkControl2 &= ~PCIE_IP_BLK428_LINK_CAPBILITY_MAX_LINK_SPEED_MASK;
	    pcieRegs->linkControl2  |= speed;
	    mdelay(10);
	    printk("PCIE port %d  speed set to %d\n", port, speed);
	}
}

static inline void bcm63xx_pcie_phy_enable_ssc(int port, bool enable)
{
	bcm963xx_pcie_gen2_phy_enable_ssc(&bcm63xx_pcie_hcd_cb[port], enable);
	return;
}

static inline void bcm63xx_pcie_phy_config_ssc(int port)
{
	return bcm963xx_pcie_gen2_phy_config_ssc(&bcm63xx_pcie_hcd_cb[port]);
}
#endif

inline uint16 bcm963xx_pcie_mdio_read (struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad) {
	return bcm63xx_pcie_mdio_read(pdrv->core_id, phyad, regad);
}
inline int bcm963xx_pcie_mdio_write (struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad, uint16 wrdata) {
	return bcm63xx_pcie_mdio_write(pdrv->core_id, phyad, regad, wrdata);
}



static void bcm63xx_pcie_phy_mode_config(int port)
{
    uint16 data;

#if defined(CONFIG_BCM963268)
   /*
    * PCIe Serdes register at block 820, register 18, bit 3:0 from 7 to F. Help reduce EMI spur.
    */
    bcm63xx_pcie_mdio_write(port, 1, 0x1f , 0x8200); 
    data = bcm63xx_pcie_mdio_read (port, 1, 0x18);
    data = ((data&0xfff0) | 0xf);
    bcm63xx_pcie_mdio_write(port, 1, 0x18, data);
#endif

#if defined(CONFIG_BCM960333) || defined(CONFIG_BCM96838)
   /*
    * PCIe Serdes register at block 808, register 1a, bit 11=1, 16-bit default 0x0283, new value 0x0a83.
    * Help reduce SerDes Tx jitter
    */
    bcm63xx_pcie_mdio_write(port, 0, 0x1f , 0x8080); 
    data = bcm63xx_pcie_mdio_read (port, 0, 0x1a);
    data = ((data&0xffff) | 0x800);
    bcm63xx_pcie_mdio_write(port, 0, 0x1a, data);
    
   /*
    * Signal detect level at block 840, register 1D, bits[5:3], default 0xf000, new value 0xf008.
    * Help to have enough margin
    */
    bcm63xx_pcie_mdio_write(port, 0, 0x1f , 0x8400); 
    data = bcm63xx_pcie_mdio_read (port, 0, 0x1d);
    data = ((data&0xffc7) | 0x8);
    bcm63xx_pcie_mdio_write(port, 0, 0x1d, data);
#endif

#if defined(RCAL_1UM_VERT)
	/*
	 * Rcal Calibration Timers
	 *   Block 0x1000, Register 1, bit 4(enable), and 3:0 (value)
	 */
    {
        int val = 0;
        uint16 data = 0;
        if(GetRCalSetting(RCAL_1UM_VERT, &val)== kPMC_NO_ERROR) {
            printk("bcm63xx_pcie: setting resistor calibration value to 0x%x\n", val);
            bcm63xx_pcie_mdio_write(port, 0, 0x1f , 0x1000);
            data = bcm63xx_pcie_mdio_read (port, 0, 1);
            data = ((data & 0xffe0) | (val & 0xf) | (1 << 4)); /*enable*/
            bcm63xx_pcie_mdio_write(port, 0, 1, data);
        }
    }
#endif
#if defined(PCIE3_CORE) /* CONFIG_BCM963381, CONFIG_BCM96848 */
    //printk("chipid:0x%x , chiprev:0x%x \n", kerSysGetChipId(), (UtilGetChipRev()));
    {
        printk("bcm63xx_pcie: applying serdes parameters\n");
        /*
         * VCO Calibration Timers
         * Workaround: 
         * Block 0x3000, Register 0xB = 0x40
         * Block 0x3000, Register 0xD = 7
         * Notes: 
         * -Fixed in 63148A0, 63381B0, 63138B0, 6848 but ok to write anyway
         */ 
        bcm63xx_pcie_mdio_write(port, 0, 0x1f, 0x3000);
        data = bcm63xx_pcie_mdio_read (port, 0, 0x1f);  /* just to exericise the read */
        bcm63xx_pcie_mdio_write(port, 0, 0xB, 0x40);
        bcm63xx_pcie_mdio_write(port, 0, 0xD, 7);
        
        /*	
         * Reference clock output level
         * Workaround:
         * Block 0x2200, Register 3 = 0xaba4
         * Note: 
         * -Fixed in 63148A0, 63381B0, 63138B0, 6848 but ok to write anyway
         */
        bcm63xx_pcie_mdio_write(port, 0, 0x1f, 0x2200);
        bcm63xx_pcie_mdio_write(port, 0, 3, 0xaba4);
        
        /* 
         * Tx Pre-emphasis
         * Workaround:
         * Block 0x4000, Register 0 = 0x1d20  // Gen1
         * Block 0x4000, Register 1 = 0x12cd  // Gen1
         * Block 0x4000, Register 3 = 0x0016  // Gen1, Gen2
         * Block 0x4000, Register 4 = 0x5920  // Gen2
         * Block 0x4000, Register 5 = 0x13cd  // Gen2
         * Notes: 
         * -Fixed in 63148A0, 63381B0, 63138B0, 6848 but ok to write anyway
         */
        bcm63xx_pcie_mdio_write(port, 0, 0x1f, 0x4000);
        bcm63xx_pcie_mdio_write(port, 0, 0, 0x1D20); 
        bcm63xx_pcie_mdio_write(port, 0, 1, 0x12CD);
        bcm63xx_pcie_mdio_write(port, 0, 3, 0x0016);
        bcm63xx_pcie_mdio_write(port, 0, 4, 0x5920);
        bcm63xx_pcie_mdio_write(port, 0, 5, 0x13CD);
        
        /*
         * Rx Signal Detect
         * Workaround:
         * Block 0x6000, Register 5 = 0x2c0d 
         * Notes:
         * -Fixed in 63148A0, 63381B0, 63138B0, 6848 but ok to write anyway
         */
        bcm63xx_pcie_mdio_write(port, 0, 0x1f, 0x6000);
        bcm63xx_pcie_mdio_write(port, 0, 0x5, 0x2C0D);	
        
        /*
         * Rx Jitter Tolerance
         * Workaround:
         * Block 0x7300, Register 3 = 0x190  // Gen1
         * Block 0x7300, Register 9 = 0x194  // Gen2
         * Notes:
         * -Gen1 setting 63148A0, 63381B0, 63138B0, 6848 but ok to write anyway
         * -Gen2 setting only in latest SerDes RTL  / future tapeouts
         */
        bcm63xx_pcie_mdio_write(port, 0, 0x1f, 0x7300);
        bcm63xx_pcie_mdio_write(port, 0, 3, 0x190);
        bcm63xx_pcie_mdio_write(port, 0, 9, 0x194);
        
        /* 
         * Gen2 Rx Equalizer
         * Workaround:
         * Block 0x6000 Register 7 = 0xf0c8  // Gen2
         * Notes:
         * -New setting only in latest SerDes RTL / future tapeouts
         */
        bcm63xx_pcie_mdio_write(port, 0, 0x1f, 0x6000);
        bcm63xx_pcie_mdio_write(port, 0, 7, 0xf0c8);
        
	/* Disable SSC, will be enabled after reset if link is up (enable= FALSE)*/
	bcm63xx_pcie_phy_config_ssc(port);
	bcm63xx_pcie_phy_enable_ssc(port, FALSE);
        
        /*
         * EP Mode PLL Bandwidth and Peaking
         * Workaround:
         * Block 0x2100, Register 0 = 0x5174
         * Block 0x2100, Register 4 = 0x6023
         * Notes:
         * -Only needed for EP mode, but ok to write in RC mode too
         * -New setting only in latest SerDes RTL / future tapeouts
         */
        bcm63xx_pcie_mdio_write(port, 0, 0x1f, 0x2100);
        bcm63xx_pcie_mdio_write(port, 0, 0, 0x5174);
        bcm63xx_pcie_mdio_write(port, 0, 4, 0x6023);
    }
#endif

    return;
}
#endif 

#if defined(UBUS2_PCIE)
static void __init bcm63xx_pcie_hw_powerup(int port, bool PowerOn)
{
#if defined(PMC_PCIE_H)
    pmc_pcie_power_up(port);
#endif
    return;
}
#endif

static void __init bcm63xx_pcie_pcie_reset(int port, bool PowerOn)
{
#if defined(PCIE3_CORE) /* CONFIG_BCM963381, CONFIG_BCM96848 */
    u32 val = MISC->miscPCIECtrl;
    if(PowerOn) {
        val &= ~(1<<port);
        MISC->miscPCIECtrl = val;
        mdelay(10);
        bcm63xx_pcie_hcd_init(port);
        /* adjust pcie phy */
        bcm63xx_pcie_phy_mode_config(port);
        mdelay(10);
        bcm63xx_pcie_core_set_speed(port);
        val |= (1<<port);
        MISC->miscPCIECtrl = val;
        mdelay(10);
    } else {
        val &= ~(1<<port);
        MISC->miscPCIECtrl = val;
    }
#endif
    /* ubus2 pcie architecture*/
#if defined(UBUS2_PCIE)
    if(port == 0){
#if defined(CONFIG_BCM960333)
        PERF->blkEnables |= PCIE_CLK_EN;

        /*
         * SOFT_RST_PCIE_EXT is the software equivalent of a power-on or push-button reset, clears PCIe sticky bits, 
         * Hard Reset registers, and SerDes MDIO Registers, and because of this is only appropriate to assert in RC mode.
         * SOFT_RST_PCIE_HARD (hard reset) is also available. It is directly equivalent to the device hard reset to PCIe, and should not be required
         */
        PERF->softResetB &= ~(SOFT_RST_PCIE_EXT |SOFT_RST_PCIE|SOFT_RST_PCIE_CORE);
        mdelay(10);		
        PERF->softResetB |= (SOFT_RST_PCIE_EXT);
        mdelay(10);
        PERF->softResetB |= SOFT_RST_PCIE;
        mdelay(10);

#if defined(CONFIG_BCM960333)
        /* adjust pcie phy */
        bcm63xx_pcie_phy_mode_config(port);
#endif
        /* optional serdes initialization de-asserts */
        PCIEH_MISC_HARD_REGS->hard_pcie_hard_debug &= ~PCIE_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ;
        mdelay(10);
        PERF->softResetB |= SOFT_RST_PCIE_CORE;
        mdelay(10);	
#endif
#if defined(CONFIG_BCM96838)
        PERF->pcie_softResetB_lo &= ~(SOFT_RST_PCIE0_CORE);
        /* adjust pcie phy */
        bcm63xx_pcie_phy_mode_config(port);         
        mdelay(10);               
        PERF->pcie_softResetB_lo |= (SOFT_RST_PCIE0_CORE);
        mdelay(10);
#endif        
    }

    if(port == 1){
#if defined(CONFIG_BCM96838)
        PERF->pcie_softResetB_lo &= ~(SOFT_RST_PCIE1_CORE);
        /* adjust pcie phy */
        bcm63xx_pcie_phy_mode_config(port);
        mdelay(10);        
        PERF->pcie_softResetB_lo |= (SOFT_RST_PCIE1_CORE);
        mdelay(10); 
#endif
    }
#else /* ubus1 pcie architecture*/
    PERF->blkEnables |= PCIE_CLK_EN;

    /* pcie serdes enable */

#if defined(CONFIG_BCM963268)
    MISC->miscSerdesCtrl |= (SERDES_PCIE_ENABLE|SERDES_PCIE_EXD_ENABLE);
#endif    

    /* reset pcie and ext device */
    PERF->softResetB &= ~(SOFT_RST_PCIE|SOFT_RST_PCIE_EXT|SOFT_RST_PCIE_CORE);

#if defined(CONFIG_BCM963268)
    PERF->softResetB &= ~SOFT_RST_PCIE_HARD;
    mdelay(10);
   
    PERF->softResetB |= SOFT_RST_PCIE_HARD;
#endif

    mdelay(10);
    
    PERF->softResetB |= (SOFT_RST_PCIE|SOFT_RST_PCIE_CORE);
    mdelay(10);
    
#if defined(CONFIG_BCM963268)
    /* adjust pcie phy */
    bcm63xx_pcie_phy_mode_config(0);
#endif    
    
    PERF->softResetB |= (SOFT_RST_PCIE_EXT);

#endif 	
    /* this is a critical delay */
    mdelay(200);
}

#if defined(UBUS2_PCIE)
#ifndef PCIEH_0_CPU_INTR1_REGS
#define PCIEH_0_CPU_INTR1_REGS        PCIEH_CPU_INTR1_REGS
#endif
#ifndef PCIEH_0_REGS
#define PCIEH_0_REGS                  PCIEH_REGS
#endif
#ifndef PCIEH_0_MISC_REGS
#define PCIEH_0_MISC_REGS             PCIEH_MISC_REGS
#endif
#ifndef PCIEH_0_RC_CFG_VENDOR_REGS
#define PCIEH_0_RC_CFG_VENDOR_REGS    PCIEH_RC_CFG_VENDOR_REGS
#endif
#ifndef PCIEH_0_PCIE_EXT_CFG_REGS
#define PCIEH_0_PCIE_EXT_CFG_REGS     PCIEH_PCIE_EXT_CFG_REGS
#endif
#ifndef PCIEH_0_BLK_428_REGS
#define PCIEH_0_BLK_428_REGS          PCIEH_BLK_428_REGS
#endif
#ifndef PCIEH_0_BLK_404_REGS
#define PCIEH_0_BLK_404_REGS          PCIEH_BLK_404_REGS
#endif
#ifndef BCM_BUS_PCIE_0_DEVICE
#define BCM_BUS_PCIE_0_DEVICE         BCM_BUS_PCIE_DEVICE
#endif
#ifndef BCM_PCIE_0_MEM_BASE
#define BCM_PCIE_0_MEM_BASE           BCM_PCIE_MEM1_BASE
#endif
#ifndef BCM_PCIE_0_MEM_SIZE
#define BCM_PCIE_0_MEM_SIZE           BCM_PCIE_MEM1_SIZE
#endif

#if defined(PCIEH_1)
#ifndef BCM_BUS_PCIE_1_DEVICE
#define BCM_BUS_PCIE_1_DEVICE         BCM_BUS_PCIE1_DEVICE
#endif
#ifndef BCM_PCIE_1_MEM_BASE
#define BCM_PCIE_1_MEM_BASE           BCM_PCIE_MEM2_BASE
#endif
#ifndef BCM_PCIE_1_MEM_SIZE
#define BCM_PCIE_1_MEM_SIZE           BCM_PCIE_MEM2_SIZE
#endif
#endif /* PCIEH_1 */

#define BCM63XX_PCIE_UBUS2_INIT_PORT(X) {            \
        PCIEH_##X##_CPU_INTR1_REGS->maskClear = (    \
            PCIE_CPU_INTR1_PCIE_INTD_CPU_INTR |      \
            PCIE_CPU_INTR1_PCIE_INTC_CPU_INTR |      \
            PCIE_CPU_INTR1_PCIE_INTB_CPU_INTR |      \
            PCIE_CPU_INTR1_PCIE_INTA_CPU_INTR );     \
       /* setup outgoing mem resource window */      \
        PCIEH_##X##_MISC_REGS->cpu_2_pcie_mem_win0_base_limit = (((BCM_PCIE_##X##_MEM_BASE+BCM_PCIE_##X##_MEM_SIZE-1)&PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK) \
                                                                 |((BCM_PCIE_##X##_MEM_BASE>>PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT)<<PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT)); \
        \
        PCIEH_##X##_MISC_REGS->cpu_2_pcie_mem_win0_lo |= (BCM_PCIE_##X##_MEM_BASE&PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK); \
        \
        /* setup incoming DDR memory BAR(1) */        \
        PCIEH_##X##_MISC_REGS->rc_bar1_config_lo = ((DDR_UBUS_ADDRESS_BASE&PCIE_MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK) \
                                                    | PCIE_MISC_RC_BAR_CONFIG_LO_SIZE_256MB); \
        \
        PCIEH_##X##_MISC_REGS->ubus_bar1_config_remap = PCIE_MISC_UBUS_BAR_CONFIG_ACCESS_EN; \
        \
        /* set device bus/func/func */ \
        PCIEH_##X##_PCIE_EXT_CFG_REGS->index = (BCM_BUS_PCIE_##X##_DEVICE<<PCIE_EXT_CFG_BUS_NUM_SHIFT); \
        \
        /* setup class code, as bridge */ \
        PCIEH_##X##_BLK_428_REGS->idVal3 &= ~PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_MASK; \
        PCIEH_##X##_BLK_428_REGS->idVal3 |= (PCI_CLASS_BRIDGE_PCI << 8);             \
        /* disable bar0 size */ \
        PCIEH_##X##_BLK_404_REGS->config2 &= ~PCIE_IP_BLK404_CONFIG_2_BAR1_SIZE_MASK; \
}

/*
  * Program the timeouts
  *   MISC_UBUS_TIMEOUT:                        0x0180_0000 (250 msec, 10ns increments, based on curent PCIE Clock)
  *   RC_CFG_PCIE_DEVICE_STATUS_CONTROL_2:      0x0006      (210ms)
  *
  * Note: PCI structures Endianness is not properly taken care, So need to write deviceStatus2 i.o deviceControl2
  *       Writing deviceStatus2 has no impact as it is RO filed only.
  */
#define BCM63XX_PCIE_CONFIG_TIMEOUTS(X) {                  \
        PCIEH_##X##_MISC_REGS->ubus_timeout = 0x01800000;  \
        PCIEH_##X##_REGS->deviceControl2 = 0x0006;         \
        PCIEH_##X##_REGS->deviceStatus2 = 0x0006;          \
}

#else


/*
  * Program the timeouts
  *   MISC_UBUS_TIMEOUT:                        (default is large 1sec. No need to program)
  *   RC_CFG_PCIE_DEVICE_STATUS_CONTROL_2:      0x0006      (210ms)
  *
  * Note: PCI structures Endianness is not properly taken care, So need to write deviceStatus2 i.o deviceControl2
  *       Writing deviceStatus2 has no impact as it is RO filed only. 
  */
#define BCM63XX_PCIE_CONFIG_TIMEOUTS(X) {            \
        PCIEH_REGS->deviceControl2 = 0x0006;         \
        PCIEH_REGS->deviceStatus2 = 0x0006;          \
}

#endif /* UBUS2_PCIE */

#if defined(PCIE3_CORE)
#ifndef PCIEH_0_MISC_REGS
#define PCIEH_0_MISC_REGS	PCIEH_MISC_REGS
#endif
#define BCM63XX_PCIE_CORE3_INIT_PORT(X) {        \
        PCIEH_##X##_MISC_REGS->misc_ctrl |= (    \
            PCIE_MISC_CTRL_BURST_ALIGN           \
            |PCIE_MISC_CTRL_PCIE_IN_WR_COMBINE   \
            |PCIE_MISC_CTRL_PCIE_RCB_MPS_MODE    \
            |PCIE_MISC_CTRL_PCIE_RCB_64B_MODE);  \
}
#else
#define BCM63XX_PCIE_CORE3_INIT_PORT(X)
#endif /* PCIE3_CORE */
#endif /* PCIEH */

static int __init bcm63xx_pci_init(void)
{
    /* adjust global io port range */
    ioport_resource.start = BCM_PCI_IO_BASE;
    ioport_resource.end = BCM_PCI_IO_BASE + BCM_PCI_IO_SIZE-1;

    printk("bcm63xx PCIe HCD\r\n");

#if defined(CONFIG_BCM963268)
    bcm63xx_pci_swhdr_patch();
#endif
    /* bus 0 */
    register_pci_controller(&bcm63xx_controller);

#if defined(PCIEH)
#if defined(UBUS2_PCIE)
/* defined(CONFIG_BCM96838) */
    if(kerSysGetPciePortEnable(0)){
        bcm63xx_pcie_hw_powerup(0, TRUE);
        bcm63xx_pcie_pcie_reset(0, TRUE);
        BCM63XX_PCIE_CORE3_INIT_PORT(0);
        BCM63XX_PCIE_UBUS2_INIT_PORT(0);
#if !defined(CONFIG_CPU_LITTLE_ENDIAN)
        PCIEH_RC_CFG_VENDOR_REGS->specificReg1 = PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BYTE_ALIGN;
#endif
        /* Currently disabled until good timeout values are available from design team
        BCM63XX_PCIE_CONFIG_TIMEOUTS(0);
        */

#if defined(PCIE3_CORE)
	/* If configured, enable PCIe SSC (enable = TRUE) */
	bcm63xx_pcie_phy_enable_ssc(0, TRUE);
#endif

        /*bus 1 and 2 */
        register_pci_controller(&bcm63xx_pcie_controller);
    }
#if defined(PCIEH_1)
    if(kerSysGetPciePortEnable(1)){
        bcm63xx_pcie_hw_powerup(1, TRUE);
        bcm63xx_pcie_pcie_reset(1, TRUE);
        BCM63XX_PCIE_CORE3_INIT_PORT(1);
        BCM63XX_PCIE_UBUS2_INIT_PORT(1);
#if !defined(CONFIG_CPU_LITTLE_ENDIAN)
        PCIEH_1_RC_CFG_VENDOR_REGS->specificReg1 = PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BYTE_ALIGN;
#endif
        /* Currently disabled until good timeout values are available from design team
        BCM63XX_PCIE_CONFIG_TIMEOUTS(1);
        */

#if defined(PCIE3_CORE)
	/* If configured, enable PCIe SSC (enable = TRUE) */
	bcm63xx_pcie_phy_enable_ssc(1, TRUE);
#endif

        /*bus 3 and 4 */
        register_pci_controller(&bcm63xx_pcie1_controller);
    }
#endif

#else	/* UBUS2_PCIE */

    bcm63xx_pcie_pcie_reset(0, TRUE);

#if defined(CONFIG_BCM963268)
PCIEH_BRIDGE_REGS->bridgeOptReg1 |= (PCIE_BRIDGE_OPT_REG1_en_l1_int_status_mask_polarity |
        PCIE_BRIDGE_OPT_REG1_en_pcie_bridge_hole_detection  |
        PCIE_BRIDGE_OPT_REG1_en_rd_reply_be_fix |
        PCIE_BRIDGE_OPT_REG1_enable_rd_be_opt);
    
    PCIEH_BRIDGE_REGS->rcInterruptMask |= (
        PCIE_BRIDGE_INTERRUPT_MASK_int_a_MASK |
        PCIE_BRIDGE_INTERRUPT_MASK_int_b_MASK |
        PCIE_BRIDGE_INTERRUPT_MASK_int_c_MASK |
        PCIE_BRIDGE_INTERRUPT_MASK_int_c_MASK );

    /* enable credit checking and error checking */
    PCIEH_BRIDGE_REGS->bridgeOptReg2 |= ( PCIE_BRIDGE_OPT_REG2_enable_tx_crd_chk_MASK |
                                          PCIE_BRIDGE_OPT_REG2_dis_ubus_ur_decode_MASK );
#endif

#if defined(CONFIG_BCM963268)
    /* setup outgoing window */
    PCIEH_BRIDGE_REGS->Ubus2PcieBar0BaseMask |= ((BCM_PCIE_MEM1_BASE&PCIE_BRIDGE_BAR0_BASE_base_MASK)|
                                                 (((BCM_PCIE_MEM1_BASE+BCM_PCIE_MEM1_SIZE-1) >>PCIE_BRIDGE_BAR0_BASE_base_MASK_SHIFT)
                                                    << PCIE_BRIDGE_BAR0_BASE_mask_MASK_SHIFT)) | PCIE_BRIDGE_BAR0_BASE_swap_enable;
#endif

#if defined(CONFIG_BCM963268)
    /* set device bus/func/func */
    PCIEH_BRIDGE_REGS->bridgeOptReg2 |= ((BCM_BUS_PCIE_DEVICE<<PCIE_BRIDGE_OPT_REG2_cfg_type1_bus_no_SHIFT) |
        PCIE_BRIDGE_OPT_REG2_cfg_type1_bd_sel_MASK );
#endif

    /* setup class code, as bridge */
    PCIEH_BLK_428_REGS->idVal3 &= ~PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_MASK;
    PCIEH_BLK_428_REGS->idVal3 |= (PCI_CLASS_BRIDGE_PCI << 8);
    /* disable bar0 size */
    PCIEH_BLK_404_REGS->config2 &= ~PCIE_IP_BLK404_CONFIG_2_BAR1_SIZE_MASK;

    /* Currently disabled until good timeout values are available from design team
    BCM63XX_PCIE_CONFIG_TIMEOUTS(0);
    */

    /*bus 1 and 2 */
    register_pci_controller(&bcm63xx_pcie_controller);
#endif /* UBUS2_PCIE */
#endif /* PCIEH */

    return 0;
}

arch_initcall(bcm63xx_pci_init);
#endif
