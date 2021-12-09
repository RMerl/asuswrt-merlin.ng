/*
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
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

// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg
#include "boardparms.h"
#include "bcm_map_part.h"
#include "bcm_misc_hw_init.h"

#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "cfe_timer.h"
#include "bcm_map.h"
#define printk  printf
#define udelay  cfe_usleep
#else // Linux
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "board.h"
#include "rdpa_types.h"
#include "lport_defs.h"
#include "bcm_intr.h"
#endif
#include "bcm_ubus4.h"
#if defined(CONFIG_BCM963158) || defined(_BCM963158_) 
#include "pmc_drv.h"
#endif


#if defined(__KERNEL__) && defined(CONFIG_BCM_XRDP)
struct device *rdp_dummy_dev = NULL;
EXPORT_SYMBOL(rdp_dummy_dev);
#endif


#if defined(CONFIG_BCM963158) || defined(_BCM963158_) 
static void bcm_set_vreg_sync(void)
{
#ifdef _CFE_
    /* Only to access the following register, we need to power ON the WAN block */
    PowerOnDevice(PMB_ADDR_WAN);
#endif
    /*
        hardware team suggested the following change so VREG_SYNC works correctly 
        When gpio_19 is configured to output vreg_sync, the 1.0V analog rail is shut off.
        It drops to 0.2V and is probably sustained at that level rather than 0V because power
        is flowing from the digital domain into the analog rail.  
        That's a dangerous situation, so don't let it persist for too long on the board or it will damage the chip.
    */
    WAN_TOP->WAN_VOLTAGE_REGULATOR_DIVIDER = (WAN_TOP->WAN_VOLTAGE_REGULATOR_DIVIDER)&~(VREG_CFG_VREG_CLK_BYPASS_MASK);
    WAN_TOP->WAN_VOLTAGE_REGULATOR_DIVIDER = (WAN_TOP->WAN_VOLTAGE_REGULATOR_DIVIDER)&~(VREG_CFG_VREG_CLK_SRC_MASK);
    WAN_TOP->WAN_VOLTAGE_REGULATOR_DIVIDER = ((WAN_TOP->WAN_VOLTAGE_REGULATOR_DIVIDER)&~(VREG_CFG_VREG_DIV_MASK))|0x53;
}
static void bcm_set_power_sync_gpio(void)
{
#ifndef _CFE_
    unsigned short value;
    //we just have to make sure VregSync is defined in the board param
    // before driving the Power Sync gpio
    if(BpGetVregSyncGpio(&value) == BP_SUCCESS && BpGetPwrSyncGpio(&value) == BP_SUCCESS)
    {
         kerSysSetGpio(value, kGpioActive);
    }
#endif
    return;
}


#endif

#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158)
static void bcm_misc_hw_rcal(void)
{
    /* start the resistor calibrator by setting RSTB and then clearing the PWRDN bit */
#if defined(CONFIG_BCM94908)
    TOPCTRL->RescalIPCtrl |= RESCAL_RSTB;
    udelay(10);
    TOPCTRL->RescalIPCtrl &= ~RESCAL_PWRDN;
    udelay(10);
#endif
#if defined(CONFIG_BCM963158)
    WAN_TOP->WAN_TOP_RESCAL_CFG |= RESCAL_RSTB;
    udelay(10);
    WAN_TOP->WAN_TOP_RESCAL_CFG &= ~RESCAL_PWRDN;
    while(!(WAN_TOP->WAN_TOP_RESCAL_STATUS_0&RESCAL_DONE));
#endif

}
#endif

#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
void bcm_set_padctrl(unsigned int pin_num, unsigned int pad_ctrl)
{
    unsigned int tp_blk_data_lsb;

    //printk("set pad ctrl %d to %d\n",pin_num, pad_ctrl);
    tp_blk_data_lsb= 0;
    tp_blk_data_lsb |= pin_num;
    tp_blk_data_lsb |= pad_ctrl;
    GPIO->TestPortBlockDataMSB = 0;
    GPIO->TestPortBlockDataLSB = tp_blk_data_lsb;
    GPIO->TestPortCmd = LOAD_PAD_CTRL_CMD;
}

#if defined(CONFIG_BCM96846) || defined(_BCM96846_)
static void bcm_misc_hw_xmii_pads_init(void)
{
    int i, n, errcnt;
    int pin_num;
    uint32_t tp_data;
    uint32_t pad_ctrl;
    unsigned short Function[BP_PINMUX_MAX];
    unsigned int Muxinfo[BP_PINMUX_MAX];
    const ETHERNET_MAC_INFO *Enet;

    if ((Enet = BpGetEthernetMacInfoArrayPtr()) == NULL)
        return;

    if (!((Enet->sw.port_map & (1 << 4)) && ((Enet->sw.phy_id[4] & MAC_IFACE) == MAC_IF_RGMII)))
        return;

    pad_ctrl = GPIO->PadCtrl;
    pad_ctrl |= (1 << 8); /* rgmii_0_pad_modehv = 1 */
    GPIO->PadCtrl = pad_ctrl;

    if (BpGetIfacePinmux (BP_PINMUX_FNTYPE_xMII | 4, BP_PINMUX_MAX,  &n, &errcnt, Function, Muxinfo) != BP_SUCCESS)
        return;

    for (i = 0; i < n ;i++)
    {
        if ((Muxinfo[i] & BP_PINMUX_OP_MASK) != BP_PINMUX_PADCTL)
            continue;

        pin_num = Muxinfo[i] & BP_PINMUX_PIN_MASK;
        tp_data = 0;
        tp_data |= (3 << 12); /* 8mA */
        tp_data |= (((pin_num >= 48) ? 1 : 0) << 16);

        bcm_set_padctrl((Muxinfo[i] & BP_PINMUX_PIN_MASK), tp_data);
    }
}
#else

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
static int bcm_misc_hw_xmii_pads_setting(unsigned int phyid, volatile uint32_t* padctrl)
{
    uint32_t rgmii_ctrl = 0;
    uint32_t tp_data = 0;
    int i, n, errcnt;
    unsigned short Function[BP_PINMUX_MAX];
    unsigned int Muxinfo[BP_PINMUX_MAX];

    switch (phyid&MAC_IFACE)
    {
        case MAC_IF_RGMII_1P8V :
            tp_data = PAD_SLEW_RATE_CTRL;  //rgmii_pad_amp_en
            break;
        case MAC_IF_RGMII_2P5V :
            rgmii_ctrl = RGMII_PAD_MODEHV;
            break;
        case MAC_IF_RGMII_3P3V :
            tp_data = PAD_HYST_ENABLE; //rgmii_pad_sel_gmii
            rgmii_ctrl = RGMII_PAD_MODEHV;
            break;
        default:
            printk("unsupported rgmii interface for phy id 0x%x!!!\n", phyid);
            return -1;
     }
     /* drive strength selection */
     tp_data |= 0x6<<PAD_DRIVE_STRENGTH_SHIFT;
     /* enable rgmii pad */ 
     rgmii_ctrl |= RGMII_PAD_ENABLE;
     TOPCTRL->RGMIICtrl = rgmii_ctrl;

     /* rgmii pin pad control using pinmux tp interface */
     if (BP_SUCCESS == BpGetIfacePinmux (BP_PINMUX_FNTYPE_xMII | 11, BP_PINMUX_MAX,  &n, &errcnt, Function, Muxinfo)) {
         for (i = n-1 ; 0 <= i ; i--) {
             if( BP_PINMUX_PADCTL == (Muxinfo[i] & BP_PINMUX_OP_MASK) ) {
                 bcm_set_padctrl((Muxinfo[i] & BP_PINMUX_PIN_MASK), tp_data);
             }
         }
     }

     return 0;
}
#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
static int bcm_misc_hw_xmii_pads_setting(unsigned int phyid, volatile uint32_t* padctrl)
{
    uint32_t value = 0;
    switch (phyid&MAC_IFACE)
    {
        case MAC_IF_RGMII_1P8V :
            value |= MISC_XMII_PAD_AMP_EN;
            break;
        case MAC_IF_RGMII_2P5V :
        case MAC_IF_RGMII_3P3V :
            value |= MISC_XMII_PAD_MODEHV;
            break;
        default:
            printk("unsupported rgmii interface for phy id 0x%x!!!\n", phyid);
            return -1;
     }
     /* drive strength selection */
     value |= 0x6<<MISC_XMII_PAD_DRIVE_STRENGTH_SHIFT;
     *padctrl = value;

     return 0;
}
#endif

static unsigned int bcm_misc_hw_get_cb_xmii_phyid(const ETHERNET_MAC_INFO *Enet, int phyport)
{
    unsigned int phyid0, phyid1, phyid;
    int cbport;

    phyid0 = phyid1 = 0;
    cbport = BP_PHY_PORT_TO_CROSSBAR_PORT(phyport);

    /* check if crossbar rgmii is on runner switch */
    if (Enet[0].sw.crossbar[cbport].switch_port != BP_CROSSBAR_NOT_DEFINED)
            phyid0 = Enet[0].sw.crossbar[cbport].phy_id;

    /* check if crossbar rgmii is on SF2 switch */
    if (Enet[1].sw.crossbar[cbport].switch_port != BP_CROSSBAR_NOT_DEFINED)
            phyid1 = Enet[1].sw.crossbar[cbport].phy_id;

    if( phyid0 == phyid1 )
        phyid = phyid0;
    else 
    {
        if( phyid0 == 0 )
            phyid = phyid1;
        else if( phyid1 == 0 )
            phyid = phyid0;
        else
        {
            printk("inconsistent crossbar xMII phyid 0x%x as WAN port, 0x%x as LAN port! FIX board parameter!\n", 
                phyid0, phyid1);
            phyid = 0;
        }
    }

    return phyid;
}
static void bcm_misc_hw_xmii_pads_init(void)
{
    const ETHERNET_MAC_INFO *Enet;
    volatile uint32_t* padctrl = NULL;
    unsigned phyid = 0;

    // Check for network ports requiring MAC interfaces to be active
    if ( (Enet = BpGetEthernetMacInfoArrayPtr()) != NULL) {
#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
        if( Enet[1].sw.port_map&0x8 && IsRGMII(Enet[1].sw.phy_id[3]) ) {
#if (CONFIG_BRCM_CHIP_REV==0x63158A0)
            padctrl = &MISC->miscxMIIPadCtrl[2];
#else
            padctrl = &TOPCTRL->xMIIPadCtrl[2];
#endif
            bcm_misc_hw_xmii_pads_setting(Enet[1].sw.phy_id[3], padctrl);
        }

        phyid = bcm_misc_hw_get_cb_xmii_phyid(Enet, 12);
        if (phyid) {
#if (CONFIG_BRCM_CHIP_REV==0x63158A0)
            padctrl = &MISC->miscxMIIPadCtrl[0];
#else
            padctrl = &TOPCTRL->xMIIPadCtrl[0];
#endif
            bcm_misc_hw_xmii_pads_setting(phyid, padctrl);
        }
#endif

        phyid = bcm_misc_hw_get_cb_xmii_phyid(Enet, 11);
        if (phyid) {
#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
#if (CONFIG_BRCM_CHIP_REV==0x63158A0)
            padctrl = &MISC->miscxMIIPadCtrl[1];
#else
            padctrl = &TOPCTRL->xMIIPadCtrl[1];
#endif
#else
            padctrl = &MISC->miscxMIIPadCtrl[1];
#endif
            bcm_misc_hw_xmii_pads_setting(phyid, padctrl);
        }
    }

    return;
}
#endif
#endif

static void bcm_misc_hw_set_intr_mux(uint16_t extintr, uint16_t gpio)
{
    int sel0, sel1, intnum;
    uint32_t mask, value;

    gpio = gpio&BP_GPIO_NUM_MASK;

    sel0 = gpio % EXT_IRQ_SLOT_SIZE; // select one gpio pin in the slot
    sel1 = gpio / EXT_IRQ_SLOT_SIZE; // select the slot

    intnum = extintr - BP_EXT_INTR_0;

    mask = ~(EXT_IRQ_MUX_SEL0_MASK<<(EXT_IRQ_MUX_SEL0_SHIFT*intnum));
    value = sel0<<(EXT_IRQ_MUX_SEL0_SHIFT*intnum);
    PERF->ExtIrqMuxSel0 &= mask;
    PERF->ExtIrqMuxSel0 |= value;

    mask = ~(EXT_IRQ_MUX_SEL1_MASK<<(EXT_IRQ_MUX_SEL1_SHIFT*intnum));
    value = sel1<<(EXT_IRQ_MUX_SEL1_SHIFT*intnum);
    PERF->ExtIrqMuxSel1 &= mask;
    PERF->ExtIrqMuxSel1 |= value;

    return;
}

void bcm_misc_hw_intr_mux_init(void)
{
    int i = 0, rc = 0, first = 1;
    void* token = NULL;
    uint16_t extintr, gpio;

    /* set the mux register to invalid selection to avoid default
       select of gpio 0 */
    PERF->ExtIrqMuxSel0 = 0xffffffff;
    PERF->ExtIrqMuxSel1 = 0xffffffff;

    for(;;)
    {
        rc = BpGetExtIntrNumGpio(i, &token, &extintr, &gpio);
        if( rc == BP_MAX_ITEM_EXCEEDED )
                break;
        else if( rc == BP_SUCCESS )
        {
            if( gpio != BP_NOT_DEFINED )
            {
                if( BpCheckExtIntr(extintr, gpio, first) == BP_SUCCESS )
                    bcm_misc_hw_set_intr_mux(extintr, gpio);
                first = 0;
            }
            else
                printk("Error no gpio number defined for external interrupt %d!\n", extintr);
        }
        else 
        {
            token = NULL;
            i++;
        }
    }

   /* clear the intr status caused by above mux setting */
    PERF->ExtIrqCtrl |= 0xff;
    PERF->ExtIrqCtrl &= ~0xff;
#if !defined(CONFIG_BCM94908) && !defined(_BCM94908_) 
    PERF->ExtIrqClear |= 0xff;
#endif

    return;
}

#ifndef _CFE_
int bcm_misc_xfi_port_get(void)
{
    int iter;
    const ETHERNET_MAC_INFO *emac_info;

    if (!(emac_info = BpGetEthernetMacInfoArrayPtr()))
        return rdpa_emac_none;

    for (iter = 0; iter < LPORT_NUM_OF_PORTS && iter < BP_MAX_SWITCH_PORTS; iter++)
    {
        if ((emac_info->sw.port_map & (1 << iter)) &&
            ((emac_info->sw.phy_id[iter] & MAC_IFACE) == MAC_IF_XFI))
        {
            return (rdpa_emac)(rdpa_emac0 + iter);
        }
    }

    return rdpa_emac_none;
}
EXPORT_SYMBOL(bcm_misc_xfi_port_get);

int bcm_misc_g9991_debug_port_get(void)
{
    int iter;
    const ETHERNET_MAC_INFO *emac_info;

    if (!(emac_info = BpGetEthernetMacInfoArrayPtr()))
        return -1;

    for (iter = 0; iter < BP_MAX_SWITCH_PORTS; iter++)
    {
        if (emac_info->sw.port_map & (1 << iter) &&
            emac_info->sw.port_flags[iter] & PORT_FLAG_MGMT)
        {
            return (rdpa_emac)(rdpa_emac0 + iter);
        }
    }

    return -1;
}
EXPORT_SYMBOL(bcm_misc_g9991_debug_port_get);

uint32_t bcm_misc_g9991_phys_port_vec_get(void)
{
    uint32_t vec = 0;
    int iter;
    const ETHERNET_MAC_INFO *emac_info;

    if (!(emac_info = BpGetEthernetMacInfoArrayPtr()))
        return 0;

    for (iter = 0; iter < BP_MAX_SWITCH_PORTS; iter++)
    {
        if (emac_info->sw.port_map & (1 << iter) &&
            emac_info->sw.port_flags[iter] & PORT_FLAG_ATTACHED)
        {
            vec |= (1 << iter);
        }
    }

    return vec;
}
EXPORT_SYMBOL(bcm_misc_g9991_phys_port_vec_get);

int runner_reserved_memory_get(uint8_t **bm_base_addr,
                               uint8_t **bm_base_addr_phys,
                               unsigned int *bm_size,
                               uint8_t **fm_base_addr,
                               uint8_t **fm_base_addr_phys,
                               unsigned int *fm_size)
{
    int rc;

    rc = BcmMemReserveGetByName(BUFFER_MEMORY_BASE_ADDR_STR,
                                (void **)bm_base_addr, bm_size);
    if (unlikely(rc)) {
        printk("%s %s Failed to get buffer memory, rc(%d)\n",
               __FILE__, __func__, rc);
        return rc;
    }

    rc = BcmMemReserveGetByName(FLOW_MEMORY_BASE_ADDR_STR,
                                (void **)fm_base_addr, fm_size);
    if (unlikely(rc)) {
        printk("Failed to get valid flow memory, rc = %d\n", rc);
        return rc;
    }

    memset(*bm_base_addr, 0x00, *bm_size);
    memset(*fm_base_addr, 0x00, *fm_size);

    *bm_base_addr_phys = (uint8_t *)virt_to_phys(*bm_base_addr);

    printk("bm_base_addr 0x%p, size %u, bm_base_addr_phys 0x%p\n",
           *bm_base_addr, *bm_size, *bm_base_addr_phys);

    *fm_base_addr_phys = (uint8_t *)virt_to_phys(*fm_base_addr);

    printk("fm_base_addr 0x%p, size %u, fm_base_addr_phys 0x%p\n",
           *fm_base_addr, *fm_size, *fm_base_addr_phys);

    *bm_size = *bm_size >> 20;	/* convert from Byte to MB */
    *fm_size = *fm_size >> 20;	/* convert from Byte to MB */

    return rc;
}
EXPORT_SYMBOL(runner_reserved_memory_get);
#endif

int rdp_shut_down(void)
{
    /*put all RDP modules in reset state*/
    // TBD. pmcPutAllRdpModulesInReset();
    return 0;
}
#ifndef _CFE_
EXPORT_SYMBOL(rdp_shut_down);
#endif

#if defined(__KERNEL__) && defined(CONFIG_BCM_XRDP)
static void alloc_rdp_dummy_device(void)
{
    if (rdp_dummy_dev == NULL) {
        rdp_dummy_dev = kzalloc(sizeof(struct device), GFP_ATOMIC);

#if defined(CONFIG_BCM96858)
        /* need to confirm how many bits we support in 6858 runner */
        dma_set_coherent_mask(rdp_dummy_dev, DMA_BIT_MASK(40));
#else
        dma_set_coherent_mask(rdp_dummy_dev, DMA_BIT_MASK(32));
#endif
#ifdef CONFIG_BCM_GLB_COHERENCY
        arch_setup_dma_ops(rdp_dummy_dev, NULL, 0, NULL, true);
#endif
    }
}
#endif
#if defined(CONFIG_BCM96858)
extern void bcm_gpio_set_data(unsigned int, unsigned int);
extern void bcm_gpio_set_dir(unsigned int gpio_num, unsigned int dir);

static void configure_xfi_optic_phy(void)
{
    bcm_gpio_set_dir(52, 1);
    bcm_gpio_set_data(52, 0);
}
#endif


int bcm_misc_hw_init(void)
{
    bcm_misc_hw_intr_mux_init();
#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined(CONFIG_BCM963158) || defined(_BCM963158_) 
    bcm_misc_hw_xmii_pads_init();
#endif
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158)
    bcm_misc_hw_rcal();
#endif
#if !defined( _CFE_) && (defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856))
    alloc_rdp_dummy_device();
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
    configure_xfi_optic_phy();
#endif

#if !defined(_BCM94908_) && !defined(CONFIG_BCM94908) && !defined(_BCM963158_) 
    bcm_ubus_config();
#endif

#if defined(CONFIG_BCM963158) || defined(_BCM963158_) 
    bcm_set_vreg_sync();
    bcm_set_power_sync_gpio();
#endif
    return 0;
}

#ifndef _CFE_
arch_initcall(bcm_misc_hw_init);
#endif
