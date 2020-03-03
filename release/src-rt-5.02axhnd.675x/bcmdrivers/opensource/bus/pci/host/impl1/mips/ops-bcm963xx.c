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
#include <asm/addrspace.h>

#include <bcm_intr.h>
#include <bcm_map_part.h>
#include <bcmpci.h>
#include <boardparms.h>
#include <board.h>

#include <linux/delay.h>


#if 0
#define DPRINT(x...)        printk(x)
#else
#undef DPRINT
#define DPRINT(x...)
#endif

#if defined(CONFIG_USB)
static int pci63xx_int_usb_read(unsigned int devfn, int where, u32 * value, int size);
static int pci63xx_int_usb_write(unsigned int devfn, int where, u32 * value, int size);

static bool usb_mem_size_rd = FALSE;
static uint32 usb_mem_base = 0;
static uint32 usb_cfg_space_cmd_reg = 0;

static int pci63xx_int_usb20_read(unsigned int devfn, int where, u32 * value, int size);
static int pci63xx_int_usb20_write(unsigned int devfn, int where, u32 * value, int size);

static bool usb20_mem_size_rd = FALSE;
static uint32 usb20_mem_base = 0;
static uint32 usb20_cfg_space_cmd_reg = 0;
#endif /* CONFIG_USB */

#if defined(CONFIG_BCM963268)
static int pci63xx_wlan_pci_read(unsigned int devfn, int where, u32 * value, int size);
static int pci63xx_wlan_pci_write(unsigned int devfn, int where, u32 val, int size);
unsigned int pci63xx_wlan_soft_config_space[WLAN_ONCHIP_DEV_NUM][WLAN_ONCHIP_PCI_HDR_DW_LEN] = {
	{WLAN_ONCHIP_PCI_ID, 0x00000006, 0x02800000, 0x00000010,
	 WLAN_CHIPC_BASE,    0x00000000, 0x00000000, 0x00000000,
	 0x00000000,  	     0x00000000, 0x00000000, 0x051314e4,
	 0x00000000,  	     0x00000040, 0x00000000, 0x0000010f,
	 0xce035801,         0x00004008, 0x0080d005, 0x00000000,
	 0x00000000,         0x00000000, 0x00784809, 0x00000010,
	 0x00000000,         0x00000000, 0x00000000, 0x00000000,
	 0x00000000,         0x00000000, 0x00000000, 0x00000000,
	 0x18001000,         0x00000000, 0xffffffff, 0x00000003,
	 0x00000000,         0x00000100, 0x00000000, 0x00000000,
	 0x00000000,         0x00000000, 0x00010000, 0x18101000,
	 0x00000000,         0x00000000, 0x00000000, 0x00000000,
	 0x00000000,         0x00000000, 0x00000000, 0x00000000,
	 0x00010010,         0x00288fa0, 0x00190100, 0x00176c11,
	 0x30110040,         0x00000000, 0x00000000, 0x00000000,
	 0x00000000,         0x00000000, 0x00000000, 0x00000000,
	 },
};                                      
#endif

#if defined(CONFIG_USB)
/* --------------------------------------------------------------------------
    Name: pci63xx_int_usb_write
Abstract: PCI Config write on internal device(s)
 -------------------------------------------------------------------------- */
static int pci63xx_int_usb_write(unsigned int devfn, int where, u32 * value, int size)
{
    switch (size) {
        case 1:
            DPRINT("W => Slot: %d Where: %2X Len: %d Data: %02X\n",
                PCI_SLOT(devfn), where, size, *value);
            break;
        case 2:
            DPRINT("W => Slot: %d Where: %2X Len: %d Data: %04X\n",
                PCI_SLOT(devfn), where, size, *value);
            switch (where) {
                case PCI_COMMAND:
                    usb_cfg_space_cmd_reg = *value;
                    break;
                default:
                    break;
            }
            break;
        case 4:
            DPRINT("W => Slot: %d Where: %2X Len: %d Data: %08X\n",
                PCI_SLOT(devfn), where, size, *value);
            switch (where) {
                case PCI_BASE_ADDRESS_0:
                    if (*value == 0xffffffff) {
                        usb_mem_size_rd = TRUE;
                    } else {
                        usb_mem_base = *value;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return PCIBIOS_SUCCESSFUL;
}

/* --------------------------------------------------------------------------
    Name: pci63xx_int_usb_read
Abstract: PCI Config read on internal device(s)
 -------------------------------------------------------------------------- */
static int pci63xx_int_usb_read(unsigned int devfn, int where, u32 * value, int size)
{
    uint32 retValue = 0xFFFFFFFF;

    // For now, this is specific to the USB Host controller. We can
    // make it more general if we have to...
    // Emulate PCI Config accesses
    switch (where) {
        case PCI_VENDOR_ID:
        case PCI_DEVICE_ID:
            retValue = PCI_VENDOR_ID_BROADCOM | 0x63000000;
            break;
        case PCI_COMMAND:
        case PCI_STATUS:
            retValue = (0x0006 << 16) | usb_cfg_space_cmd_reg;
            break;
        case PCI_CLASS_REVISION:
        case PCI_CLASS_DEVICE:
            retValue = (PCI_CLASS_SERIAL_USB << 16) | (0x10 << 8) | 0x01;
            break;
        case PCI_BASE_ADDRESS_0:
            if (usb_mem_size_rd) {
                retValue = USB_BAR0_MEM_SIZE;
            } else {
                if (usb_mem_base != 0)
                    retValue = usb_mem_base;
                else
                    retValue = USB_OHCI_BASE;
            }
            usb_mem_size_rd = FALSE;
            break;
        case PCI_CACHE_LINE_SIZE:
            retValue = L1_CACHE_BYTES/4;
            break;
        case PCI_LATENCY_TIMER:
            retValue = 0;
            break;
        case PCI_HEADER_TYPE:
            retValue = PCI_HEADER_TYPE_NORMAL;
            break;
        case PCI_SUBSYSTEM_VENDOR_ID:
            retValue = PCI_VENDOR_ID_BROADCOM;
            break;
        case PCI_SUBSYSTEM_ID:
            retValue = 0x6300;
            break;
        case PCI_INTERRUPT_LINE:
            retValue = INTERRUPT_ID_USBH;
            break;
        default:
            break;
    }

    switch (size) {
        case 1:
            *value = (retValue >> ((where & 3) << 3)) & 0xff;
            DPRINT("R <= Slot: %d Where: %2X Len: %d Data: %02X\n",
                PCI_SLOT(devfn), where, size, *value);
            break;
        case 2:
            *value = (retValue >> ((where & 3) << 3)) & 0xffff;
            DPRINT("R <= Slot: %d Where: %2X Len: %d Data: %04X\n",
                PCI_SLOT(devfn), where, size, *value);
            break;
        case 4:
            *value = retValue;
            DPRINT("R <= Slot: %d Where: %2X Len: %d Data: %08X\n",
                PCI_SLOT(devfn), where, size, *value);
            break;
        default:
            break;
    }
    return PCIBIOS_SUCCESSFUL;
}

/* --------------------------------------------------------------------------
    Name: pci63xx_int_usb20_write
Abstract: PCI Config write on internal device(s)
 -------------------------------------------------------------------------- */
static int pci63xx_int_usb20_write(unsigned int devfn, int where, u32 * value, int size)
{
    switch (size) {
        case 1:
            DPRINT("W => Slot: %d Where: %2X Len: %d Data: %02X\n",
                PCI_SLOT(devfn), where, size, *value);
            break;
        case 2:
            DPRINT("W => Slot: %d Where: %2X Len: %d Data: %04X\n",
                PCI_SLOT(devfn), where, size, *value);
            switch (where) {
                case PCI_COMMAND:
                    usb20_cfg_space_cmd_reg = *value;
                    break;
                default:
                    break;
            }
            break;
        case 4:
            DPRINT("W => Slot: %d Where: %2X Len: %d Data: %08X\n",
                PCI_SLOT(devfn), where, size, *value);
            switch (where) {
                case PCI_BASE_ADDRESS_0:
                    if (*value == 0xffffffff) {
                        usb20_mem_size_rd = TRUE;
                    } else {
                        usb20_mem_base = *value;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return PCIBIOS_SUCCESSFUL;
}

/* --------------------------------------------------------------------------
    Name: pci63xx_int_usb20_read
Abstract: PCI Config read on internal device(s)
 -------------------------------------------------------------------------- */
static int pci63xx_int_usb20_read(unsigned int devfn, int where, u32 * value, int size)
{
    uint32 retValue = 0xFFFFFFFF;

    // For now, this is specific to the USB Host controller. We can
    // make it more general if we have to...
    // Emulate PCI Config accesses
    switch (where) {
        case PCI_VENDOR_ID:
        case PCI_DEVICE_ID:
            retValue = PCI_VENDOR_ID_BROADCOM | 0x63000000;
            break;
        case PCI_COMMAND:
        case PCI_STATUS:
            retValue = (0x0006 << 16) | usb20_cfg_space_cmd_reg;
            break;
        case PCI_CLASS_REVISION:
        case PCI_CLASS_DEVICE:
            retValue = (PCI_CLASS_SERIAL_USB << 16) | (0x20 << 8) | 0x01;
            break;
        case PCI_BASE_ADDRESS_0:
            if (usb20_mem_size_rd) {
                retValue = USB_BAR0_MEM_SIZE;
            } else {
                if (usb20_mem_base != 0)
                    retValue = usb20_mem_base;
                else
                    retValue = USB_EHCI_BASE;
            }
            usb20_mem_size_rd = FALSE;
            break;
        case PCI_CACHE_LINE_SIZE:
            retValue = L1_CACHE_BYTES/4;
            break;
        case PCI_LATENCY_TIMER:
            retValue = 0;
            break;
        case PCI_HEADER_TYPE:
            retValue = PCI_HEADER_TYPE_NORMAL;
            break;
        case PCI_SUBSYSTEM_VENDOR_ID:
            retValue = PCI_VENDOR_ID_BROADCOM;
            break;
        case PCI_SUBSYSTEM_ID:
            retValue = 0x6300;
            break;
        case PCI_INTERRUPT_LINE:
            retValue = INTERRUPT_ID_USBH20;
            break;
        default:
            break;
    }

    switch (size) {
        case 1:
            *value = (retValue >> ((where & 3) << 3)) & 0xff;
            DPRINT("R <= Slot: %d Where: %2X Len: %d Data: %02X\n",
                PCI_SLOT(devfn), where, size, *value);
            break;
        case 2:
            *value = (retValue >> ((where & 3) << 3)) & 0xffff;
            DPRINT("R <= Slot: %d Where: %2X Len: %d Data: %04X\n",
                PCI_SLOT(devfn), where, size, *value);
            break;
        case 4:
            *value = retValue;
            DPRINT("R <= Slot: %d Where: %2X Len: %d Data: %08X\n",
                PCI_SLOT(devfn), where, size, *value);
            break;
        default:
            break;
    }

    return PCIBIOS_SUCCESSFUL;
}
#endif

#if defined(CONFIG_BCM963268)
/* --------------------------------------------------------------------------
    Name: pci63xx_wlan_pci_write
    Abstract: PCI Config write on internal device(s)
    
    extra read/write to take care / pass through 
    BASE ADDR 0, where=16 , R/W
    INTERRUPT PIN, where=60, R/W
    COMMAND, where=4, R/W
    LATENCY TIMER, where=13
 -------------------------------------------------------------------------- */
static int pci63xx_wlan_pci_write(unsigned int devfn, int where, u32 val, int size)
{
    uint32 data;
        
    if(where >= 256) {
    	return PCIBIOS_BAD_REGISTER_NUMBER;    
    }
    data = pci63xx_wlan_soft_config_space[PCI_SLOT(devfn)-WLAN_ONCHIP_DEV_SLOT][where/4];

    switch(size) {
        case 1:
            data = (data & ~(0xff << ((where & 3) << 3))) |
                (val << ((where & 3) << 3));
            break;
        case 2:
            data = (data & ~(0xffff << ((where & 3) << 3))) |
                (val << ((where & 3) << 3));
            break;
        case 4:
            data = val;
            break;
        default:
            break;
    }
    pci63xx_wlan_soft_config_space[PCI_SLOT(devfn)-WLAN_ONCHIP_DEV_SLOT][where/4] = data;

    return PCIBIOS_SUCCESSFUL;
}

/* --------------------------------------------------------------------------
    Name: pci63xx_wlan_pci_read
Abstract: PCI Config read on internal device(s)
 -------------------------------------------------------------------------- */
static int pci63xx_wlan_pci_read(unsigned int devfn, int where, u32 * val, int size)
{
    uint32 data;    
    
    if(where >= 256) {
    	data = 0xffffffff;
    }	
    else	
    	data = pci63xx_wlan_soft_config_space[PCI_SLOT(devfn)-WLAN_ONCHIP_DEV_SLOT][where/4];

    switch(size) {
        case 1:
            *val = (data >> ((where & 3) << 3)) & 0xff;
            break;
        case 2:
            *val = (data >> ((where & 3) << 3)) & 0xffff;
            break;
        case 4:
            *val = data;
             /* Special case for reading PCI device range */
            if ((where >= PCI_BASE_ADDRESS_0) && (where <= PCI_BASE_ADDRESS_5)) {
                if (data == 0xffffffff) {
                	if (where == PCI_BASE_ADDRESS_0)
                        *val = 0xffffe000;/* PCI_SIZE_8K */
                	else 
                        *val = 0xffffffff;
                }
            }
            if(where == PCI_ROM_ADDRESS)
                *val = 0xffffffff;
            break;
        default:
            break;
    }
    return PCIBIOS_SUCCESSFUL;
}
#endif

#if defined(CONFIG_USB)
static int isUsbHostPresent(void)
{
	if(kerSysGetUsbHostPortEnable(0) || kerSysGetUsbHostPortEnable(1))
		return 1;
	else
		return 0;
}
#endif

int bcm63xx_pcibios_read(struct pci_bus *bus, unsigned int devfn,
    int where, int size, u32 * val)
{
    *val = 0xffffffff;

#if defined(CONFIG_USB)
    if(isUsbHostPresent())
    {
        if (PCI_SLOT(devfn) == USB_HOST_SLOT)
            return pci63xx_int_usb_read(devfn, where, val, size);
        if (PCI_SLOT(devfn) == USB20_HOST_SLOT)
            return pci63xx_int_usb20_read(devfn, where, val, size);
    }
#endif

#if defined(CONFIG_BCM963268)
    if (PCI_SLOT(devfn) == WLAN_ONCHIP_DEV_SLOT)
       	return pci63xx_wlan_pci_read(devfn, where, val, size);
#endif


    return PCIBIOS_SUCCESSFUL;
}

int bcm63xx_pcibios_write(struct pci_bus *bus, unsigned int devfn,
    int where, int size, u32 val)
{

#if defined(CONFIG_USB)
    if(isUsbHostPresent())
    {
        if (PCI_SLOT(devfn) == USB_HOST_SLOT)
            return pci63xx_int_usb_write(devfn, where, &val, size);
        if (PCI_SLOT(devfn) == USB20_HOST_SLOT)
            return pci63xx_int_usb20_write(devfn, where, &val, size);
    }
#endif

#if defined(CONFIG_BCM963268)
    if (PCI_SLOT(devfn) == WLAN_ONCHIP_DEV_SLOT)
       	return pci63xx_wlan_pci_write(devfn, where, val, size);
#endif

    return PCIBIOS_SUCCESSFUL;
}

struct pci_ops bcm63xx_pci_ops = {
    .read   = bcm63xx_pcibios_read,
    .write  = bcm63xx_pcibios_write
};


#if defined(PCIEH)
/* supported external PCIE devices */
enum pcie_device_supported {UNSPECIFIED, SWITCH_8232104c};
#define PCIID_SWITCH_8232104c  0x8232104c
/* supported devices */
enum pcie_device_supported first_device_detected = UNSPECIFIED;

/* check supported devices, setup config properly before calling */
static u32 bcm63xx_pcie_detect_first_device(void)
{	
    u32 devid;
	
    devid = *(u32*)(((u8*)PCIEH)+PCIEH_DEV_OFFSET);

    switch(devid) {
        case PCIID_SWITCH_8232104c:
            first_device_detected = SWITCH_8232104c;
            break;
        default:
            first_device_detected = UNSPECIFIED;
    }

    return devid;
}

static inline u32 CFGOFFSET(u32 bus, u32 devfn, u32 where)
{
    if(bus == BCM_BUS_PCIE_ROOT ) {
        /* access offset 0 */
        return where;
#if defined(PCIEH_1)
     } else if(bus == BCM_BUS_PCIE1_ROOT){
        /* access offset 0 */
        return where;
#endif
    } else {
        /* access offset */
        return PCIEH_DEV_OFFSET|where;
    }
}

#define CFGADDR(bus,devfn,where)   CFGOFFSET((bus)->number,(devfn),where)

static inline u32 READCFG32(u32 bus, u32 addr)
{
#if defined(PCIEH_1)
    if(bus >= BCM_BUS_PCIE1_ROOT) {
        DPRINT("pcie_reading addr(0x%x) = 0x%x\n", (unsigned int)(((u8*)PCIEH_1) + (addr&~3)),*(u32*)(((u8*)PCIEH_1) + (addr&~3)));
        return *(u32 *)(((u8*)PCIEH_1) + (addr&~3));
    } else 
#endif
    {
        DPRINT("pcie_reading addr(0x%x) = 0x%x\n", (unsigned int)(((u8*)PCIEH) + (addr&~3)),*(u32*)(((u8*)PCIEH) + (addr&~3)));
        return *(u32 *)(((u8*)PCIEH) + (addr&~3));
    }
}

static inline void WRITECFG32(u32 bus, u32 addr, u32 data)
{
#if defined(PCIEH_1)
    if(bus >= BCM_BUS_PCIE1_ROOT) {
        *(u32 *)(((u8*)PCIEH_1) + (addr & ~3)) = data;
    } else 
#endif
    {
    DPRINT("pcie_writing addr(0x%x) = 0x%x\n", (unsigned int)(((u8*)PCIEH) + (addr & ~3)), data);
    *(u32 *)(((u8*)PCIEH) + (addr & ~3)) = data;
    }
}

static void pci63xx_pcie_config_select_by_numbers(u32 bus_no, u32 dev_no, u32 func_no)
{
    /* set device bus/func/func */
#if defined(UBUS2_PCIE)
#if defined(PCIEH_1)
   if(bus_no >= BCM_BUS_PCIE1_ROOT) {
        //PCIEH_1_MISC_REGS->misc_ctrl |= PCIE_MISC_CTRL_CFG_READ_UR_MODE;
        PCIEH_1_PCIE_EXT_CFG_REGS->index = ((bus_no<<PCIE_EXT_CFG_BUS_NUM_SHIFT)|(dev_no <<PCIE_EXT_CFG_DEV_NUM_SHIFT)|(func_no<<PCIE_EXT_CFG_FUNC_NUM_SHIFT));
        DPRINT("PCIEH_PCIE_EXT_CFG_REGS->index(%d/%d/%d) = 0x%x\n", bus_no, dev_no, func_no, (unsigned int) PCIEH_1_PCIE_EXT_CFG_REGS->index );   	
   } else
#endif
    {
    /* disable data bus error for enumeration */
        //PCIEH_MISC_REGS->misc_ctrl |= PCIE_MISC_CTRL_CFG_READ_UR_MODE;
        PCIEH_PCIE_EXT_CFG_REGS->index = ((bus_no<<PCIE_EXT_CFG_BUS_NUM_SHIFT)|(dev_no <<PCIE_EXT_CFG_DEV_NUM_SHIFT)|(func_no<<PCIE_EXT_CFG_FUNC_NUM_SHIFT));
        DPRINT("PCIEH_PCIE_EXT_CFG_REGS->index(%d/%d/%d) = 0x%x\n", bus_no, dev_no, func_no, (unsigned int) PCIEH_PCIE_EXT_CFG_REGS->index );
    }
#else    
    PCIEH_BRIDGE_REGS->bridgeOptReg2 &= ~(PCIE_BRIDGE_OPT_REG2_cfg_type1_bus_no_MASK |
    		PCIE_BRIDGE_OPT_REG2_cfg_type1_dev_no_MASK|PCIE_BRIDGE_OPT_REG2_cfg_type1_func_no_MASK);
     
    PCIEH_BRIDGE_REGS->bridgeOptReg2 |= ((bus_no<<PCIE_BRIDGE_OPT_REG2_cfg_type1_bus_no_SHIFT) |
    		(dev_no<<PCIE_BRIDGE_OPT_REG2_cfg_type1_dev_no_SHIFT) |
    		(func_no<<PCIE_BRIDGE_OPT_REG2_cfg_type1_func_no_SHIFT) |
        PCIE_BRIDGE_OPT_REG2_cfg_type1_bd_sel_MASK );
    DPRINT("PCIEH_BRIDGE_REGS->bridgeOptReg2 = 0x%x\n", (unsigned int) PCIEH_BRIDGE_REGS->bridgeOptReg2 );
#endif

}
#if !defined(PCIEH_1)
/* this is the topology of deviceid/vendorid 0x823210
  (US, bus N)
    |
    |--- (DS #0, bus N+1) --- Dev #0 (bus N+2)
    |--- (DS #1, bus N+1) --- Dev #1 (bus N+3)
    |--- (DS #2, bus N+1) --- Dev #2 (bus N+4)
*/
static int pci63xx_pcie_can_access_switch8232104c_by_numbers(u32 bus_no, u32 dev_no, u32 func_no)
{

  /* disable data bus error for enumeration */
#if defined(UBUS2_PCIE)
    PCIEH_MISC_REGS->misc_ctrl |= PCIE_MISC_CTRL_CFG_READ_UR_MODE;
#else
    PCIEH_BRIDGE_REGS->bridgeOptReg2 |= (PCIE_BRIDGE_OPT_REG2_dis_pcie_abort_MASK );
#endif
																					
    if ((bus_no == BCM_BUS_PCIE_DEVICE + 1) && (dev_no <= 2)) {
        return TRUE;
    } else if ((bus_no >= BCM_BUS_PCIE_DEVICE + 2 ) && (bus_no <= BCM_BUS_PCIE_DEVICE + 4)) {
    /*support single function device*/
    return (dev_no == 0); 
    }
    return FALSE;	
}
#endif

static int pci63xx_pcie_can_access_by_numbers(u32 bus_no, u32 dev_no, u32 func_no)
{
		
    /* select device */
    pci63xx_pcie_config_select_by_numbers(bus_no, dev_no, func_no);

#if defined(PCIEH_1)
    if (bus_no == BCM_BUS_PCIE1_ROOT) {
        /* bridge */
        return (dev_no == 0); /*otherwise will loop for the rest of the device*/
    } else if (bus_no == BCM_BUS_PCIE1_DEVICE) {
        /* upstream port or end device */
        /* check link up*/
        if(!(PCIEH_1_BLK_1000_REGS->dlStatus&PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_MASK)) {
            return 0;
        }
       	return (dev_no == 0); /*otherwise will loop for the rest of the device*/
     } else
#endif

    if (bus_no == BCM_BUS_PCIE_ROOT ) {
        /* bridge */
        return (dev_no == 0); /*otherwise will loop for the rest of the device*/
    } else if (bus_no == BCM_BUS_PCIE_DEVICE) {
        /* upstream port or end device */
        /* check link up*/
        if(!(PCIEH_BLK_1000_REGS->dlStatus&PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_MASK)) {
            return 0;
        }
        bcm63xx_pcie_detect_first_device();

       	return (dev_no == 0); /*otherwise will loop for the rest of the device*/
#if !defined(PCIEH_1)
		/* no support on external bridge */
    } else { 	
        if(first_device_detected == SWITCH_8232104c)
            return pci63xx_pcie_can_access_switch8232104c_by_numbers(bus_no, dev_no, func_no);
#endif
    }
    return 0;
	
}

static int pci63xx_pcie_can_access(struct pci_bus *bus, int devfn)
{
    return pci63xx_pcie_can_access_by_numbers(bus->number,PCI_SLOT(devfn), PCI_FUNC(devfn));     
}

static int bcm63xx_pciebios_read(struct pci_bus *bus, unsigned int devfn,
    int where, int size, u32 * val)
{
    u32 data = 0;
    DPRINT("pcie_read, bus=%d, devfn=%d, where=%d, size=%d, val=0x%x\n", bus->number, devfn, where, size, *val);
    if ((size == 2) && (where & 1)) {
        return PCIBIOS_BAD_REGISTER_NUMBER;
    }
    else if ((size == 4) && (where & 3)) {
        return PCIBIOS_BAD_REGISTER_NUMBER;
    }

    if (pci63xx_pcie_can_access(bus, devfn)) {
        data = READCFG32((bus)->number,CFGADDR(bus, devfn, where));
        if (data == 0xdeaddead) {
            data = 0xffffffff;
            return PCIBIOS_DEVICE_NOT_FOUND;
        }
    }            
    else {
        data = 0xffffffff;
        return PCIBIOS_DEVICE_NOT_FOUND;
    }
       
    if (size == 1)
        *val = (data >> ((where & 3) << 3)) & 0xff;
    else if (size == 2)
        *val = (data >> ((where & 3) << 3)) & 0xffff;
    else
        *val = data;

    return PCIBIOS_SUCCESSFUL;

}

static int bcm63xx_pciebios_write(struct pci_bus *bus, unsigned int devfn,
    int where, int size, u32 val)
{
    u32 cfgaddr = CFGADDR(bus, devfn, where);
    u32 data = 0;

    DPRINT("pcie_write, bus=%d, devfn=%d, where=%d, size=%d, val=0x%x\n", bus->number, devfn, where, size, val);

    if ((size == 2) && (where & 1))
        return PCIBIOS_BAD_REGISTER_NUMBER;
    else if ((size == 4) && (where & 3))
        return PCIBIOS_BAD_REGISTER_NUMBER;

    if (!pci63xx_pcie_can_access(bus, devfn))
        return PCIBIOS_DEVICE_NOT_FOUND;

    data = READCFG32((bus)->number,cfgaddr);

    if (size == 1)
        data = (data & ~(0xff << ((where & 3) << 3))) |
            (val << ((where & 3) << 3));
    else if (size == 2)
        data = (data & ~(0xffff << ((where & 3) << 3))) |
            (val << ((where & 3) << 3));
    else
        data = val;

    WRITECFG32((bus)->number, cfgaddr, data);

    return PCIBIOS_SUCCESSFUL;

}

struct pci_ops bcm63xx_pcie_ops = {
    .read   = bcm63xx_pciebios_read,
    .write  = bcm63xx_pciebios_write
};
#endif
#endif
