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
#include "bcm_map.h"
#define printk  printf
#else // Linux
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <bcm_map_part.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "board.h"
#include "rdpa_types.h"
#endif


#if defined(__KERNEL__) &&  defined(CONFIG_BCM96858)
struct device *rdp_dummy_dev = NULL;
EXPORT_SYMBOL(rdp_dummy_dev);
#endif

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
static void bcm_set_padctrl(unsigned int pin_num, unsigned int pad_ctrl)
{
    unsigned int tp_blk_data_lsb;
    //printk("set pad ctrl %d to %d\n",pin_num, pad_ctrl);
    tp_blk_data_lsb= 0;
    tp_blk_data_lsb |= pin_num;
    tp_blk_data_lsb |= (pad_ctrl << PAD_CTRL_SHIFT);
    GPIO->TestPortBlockDataMSB = 0;
    GPIO->TestPortBlockDataLSB = tp_blk_data_lsb;
    GPIO->TestPortCmd = LOAD_PAD_CTRL_CMD;
}

static void bcm_misc_hw_xmii_pads_init(void)
{
    const ETHERNET_MAC_INFO *Enet;
    int i,j;
    int u, found = 0;
    uint32_t rgmii_ctrl = 0;
    uint32_t tp_data = 0;
    int n, errcnt;
    unsigned short Function[BP_PINMUX_MAX];
    unsigned int Muxinfo[BP_PINMUX_MAX];

    // Check for network ports requiring MAC interfaces to be active
    if ( (Enet = BpGetEthernetMacInfoArrayPtr()) != NULL)
    {
        for (i = 0 ; i < BP_MAX_ENET_MACS ; i++) {
            for (j = 0; j < BP_MAX_CROSSBAR_EXT_PORTS ; j++) {
                u = BP_CROSSBAR_PORT_TO_PHY_PORT(j);
                /* 4908 only has one xGMII port, phy port 11, on the crossbar */
                if (Enet[i].sw.crossbar[j].switch_port != BP_CROSSBAR_NOT_DEFINED && u == 11)
                {
                    switch (Enet[i].sw.crossbar[j].phy_id & MAC_IFACE)
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
                    }
                    /* drive strength selection */
                    tp_data |= 0x6<<PAD_DRIVE_STRENGTH_SHIFT;
                    /* enable rgmii pad */ 
                    rgmii_ctrl |= RGMII_PAD_ENABLE;
                    found = 1;
                    break;
                }
            }
        }
    }

    if( found ) {
   
        TOPCTRL->RGMIICtrl = rgmii_ctrl;

        /* rgmii pin pad control using pinmux tp interface */
        if (BP_SUCCESS == BpGetIfacePinmux (BP_PINMUX_FNTYPE_xMII | u, BP_PINMUX_MAX,  &n, &errcnt, Function, Muxinfo)) {
            for (i = n-1 ; 0 <= i ; i--) {
                if( BP_PINMUX_PADCTL == (Muxinfo[i] & BP_PINMUX_OP_MASK) )
                    bcm_set_padctrl((Muxinfo[i] & BP_PINMUX_PIN_MASK), tp_data>>PAD_CTRL_SHIFT);
            }
        }
    }
    
    return;
}
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
    int i = 0, rc = 0;
    void* token = NULL;
    uint16_t extintr, gpio;

    for(;;)
    {
        rc = BpGetExtIntrNumGpio(i, &token, &extintr, &gpio);
        if( rc == BP_MAX_ITEM_EXCEEDED )
                break;
        else if( rc == BP_SUCCESS )
        {
            if( gpio != BP_NOT_DEFINED )
                bcm_misc_hw_set_intr_mux(extintr, gpio);
            else
                printk("Error no gpio number defined for external interrupt %d!\n", extintr);
        }
        else 
        {
            token = NULL;
            i++;
        }
    }

    return;
}

#ifndef _CFE_
int bcm_misc_xfi_port_get(void)
{
    int iter;
    const ETHERNET_MAC_INFO *emac_info;

    if (!(emac_info = BpGetEthernetMacInfoArrayPtr()))
        return rdpa_emac_none;

    for (iter = 0; iter < BP_MAX_SWITCH_PORTS; iter++)
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

#if defined(__KERNEL__) &&  defined(CONFIG_BCM96858)
static void alloc_rdp_dummy_device(void)
{
    if (rdp_dummy_dev == NULL) {
        rdp_dummy_dev = kzalloc(sizeof(struct device), GFP_ATOMIC);

        /* need to confirm how many bits we support in 6858 runner */
        dma_set_coherent_mask(rdp_dummy_dev, DMA_BIT_MASK(40));
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
#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
    bcm_misc_hw_xmii_pads_init();
#endif
#if defined(CONFIG_BCM96858)
#ifndef _CFE_
    alloc_rdp_dummy_device();
#else
    /*Set UBUS credits for runner quads */
    *(uint32_t*)(0x83480450) = 1;
    *(uint32_t*)(0x83488450) = 1;
    *(uint32_t*)(0x83490450) = 1;
    *(uint32_t*)(0x83498450) = 1;
#endif
    configure_xfi_optic_phy();
#endif
    return 0;
}

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
int ubus_master_decode_wnd_cfg(MST_PORT_NODE node, int win, unsigned int phys_addr, unsigned int size_power_of_2, int port_id)
{
    MstPortNode *master_addr;
    int ret = 0;

    if((win > 3) || (size_power_of_2 > 31) || (phys_addr & ((1<<size_power_of_2)-1)))
        return -1;

    switch(node)
    {
        case MST_PORT_NODE_PCIE0:
            master_addr = (MstPortNode *)MST_PORT_NODE_PCIE0_BASE;
            break;
        case MST_PORT_NODE_PCIE1:
            master_addr = (MstPortNode *)MST_PORT_NODE_PCIE1_BASE;
            break;
        case MST_PORT_NODE_B53:
            master_addr = (MstPortNode *)MST_PORT_NODE_B53_BASE;
            break;
        case MST_PORT_NODE_PCIE2:
            master_addr = (MstPortNode *)MST_PORT_NODE_PCIE2_BASE;
            break;
        case MST_PORT_NODE_SATA:
            master_addr = (MstPortNode *)MST_PORT_NODE_SATA_BASE;
            break;
        case MST_PORT_NODE_USB:
            master_addr = (MstPortNode *)MST_PORT_NODE_USB_BASE;
            break;
        case MST_PORT_NODE_PMC:
            master_addr = (MstPortNode *)MST_PORT_NODE_PMC_BASE;
            break;
        case MST_PORT_NODE_APM:
            master_addr = (MstPortNode *)MST_PORT_NODE_APM_BASE;
            break;
        case MST_PORT_NODE_PER:
            master_addr = (MstPortNode *)MST_PORT_NODE_PER_BASE;
            break;
        case MST_PORT_NODE_DMA0:
            master_addr = (MstPortNode *)MST_PORT_NODE_DMA0_BASE;
            break;
        case MST_PORT_NODE_DMA1:
            master_addr = (MstPortNode *)MST_PORT_NODE_DMA1_BASE;
            break;
        case MST_PORT_NODE_RQ0:
            master_addr = (MstPortNode *)MST_PORT_NODE_RQ0_BASE;
            break;
        case MST_PORT_NODE_RQ1:
            master_addr = (MstPortNode *)MST_PORT_NODE_RQ1_BASE;
            break;
        case MST_PORT_NODE_RQ2:
            master_addr = (MstPortNode *)MST_PORT_NODE_RQ2_BASE;
            break;
        case MST_PORT_NODE_RQ3:
            master_addr = (MstPortNode *)MST_PORT_NODE_RQ3_BASE;
            break;
        case MST_PORT_NODE_NATC:
            master_addr = (MstPortNode *)MST_PORT_NODE_NATC_BASE;
            break;
        case MST_PORT_NODE_DQM:
            master_addr = (MstPortNode *)MST_PORT_NODE_DQM_BASE;
            break;
        case MST_PORT_NODE_QM:
            master_addr = (MstPortNode *)MST_PORT_NODE_QM_BASE;
            break;
        default:
            ret = -1;
    }

    if(!ret)
    {
        if(size_power_of_2)
        {
            // there is a bug in A0 UBUS that requires a shift by one into the decode cfg
            master_addr->decode_cfg.window[win].base_addr = (phys_addr>>8) << (win + 1);
            master_addr->decode_cfg.window[win].remap_addr = (phys_addr>>8) << (win + 1);
            if(port_id == DECODE_CFG_PID_B53)
                master_addr->decode_cfg.window[win].attributes = (DECODE_CFG_CACHE_BITS | DECODE_CFG_ENABLE_ADDR_ONLY | (size_power_of_2 << DECODE_CFG_SIZE_SHIFT) | port_id) << (win + 1);
            else
                master_addr->decode_cfg.window[win].attributes = (DECODE_CFG_ENABLE_ADDR_ONLY | (size_power_of_2 << DECODE_CFG_SIZE_SHIFT) | port_id) << 1;
        }
        else
        {
            master_addr->decode_cfg.window[win].base_addr = 0;
            master_addr->decode_cfg.window[win].remap_addr = 0;
            master_addr->decode_cfg.window[win].attributes = 0;
        }
    }

    return ret;
}

/*this function is used to set UBUS route credits per usub master, should be equivalent configuration at masters*/
int ubus_master_set_token_credits(MST_PORT_NODE node, int token, int credits)
{
    MstPortNode *master_addr = NULL;
    int ret = 0;

    switch(node)
    {
        case MST_PORT_NODE_PCIE0:
        master_addr = (MstPortNode *)MST_PORT_NODE_PCIE0_BASE;
        break;
        case MST_PORT_NODE_PCIE1:
        master_addr = (MstPortNode *)MST_PORT_NODE_PCIE1_BASE;
        break;
        case MST_PORT_NODE_B53:
        master_addr = (MstPortNode *)MST_PORT_NODE_B53_BASE;
        break;
        case MST_PORT_NODE_PCIE2:
        master_addr = (MstPortNode *)MST_PORT_NODE_PCIE2_BASE;
        break;
        case MST_PORT_NODE_SATA:
        master_addr = (MstPortNode *)MST_PORT_NODE_SATA_BASE;
        break;
        case MST_PORT_NODE_USB:
        master_addr = (MstPortNode *)MST_PORT_NODE_USB_BASE;
        break;
        case MST_PORT_NODE_PMC:
        master_addr = (MstPortNode *)MST_PORT_NODE_PMC_BASE;
        break;
        case MST_PORT_NODE_APM:
        master_addr = (MstPortNode *)MST_PORT_NODE_APM_BASE;
        break;
        case MST_PORT_NODE_PER:
        master_addr = (MstPortNode *)MST_PORT_NODE_PER_BASE;
        break;
        case MST_PORT_NODE_DMA0:
        master_addr = (MstPortNode *)MST_PORT_NODE_DMA0_BASE;
        break;
        case MST_PORT_NODE_DMA1:
        master_addr = (MstPortNode *)MST_PORT_NODE_DMA1_BASE;
        break;
        case MST_PORT_NODE_RQ0:
        master_addr = (MstPortNode *)MST_PORT_NODE_RQ0_BASE;
        break;
        case MST_PORT_NODE_RQ1:
        master_addr = (MstPortNode *)MST_PORT_NODE_RQ1_BASE;
        break;
        case MST_PORT_NODE_RQ2:
        master_addr = (MstPortNode *)MST_PORT_NODE_RQ2_BASE;
        break;
        case MST_PORT_NODE_RQ3:
        master_addr = (MstPortNode *)MST_PORT_NODE_RQ3_BASE;
        break;
        case MST_PORT_NODE_NATC:
        master_addr = (MstPortNode *)MST_PORT_NODE_NATC_BASE;
        break;
        case MST_PORT_NODE_DQM:
        master_addr = (MstPortNode *)MST_PORT_NODE_DQM_BASE;
        break;
        case MST_PORT_NODE_QM:
        master_addr = (MstPortNode *)MST_PORT_NODE_QM_BASE;
        break;
        default:
        ret = -1;
    }

    if(!master_addr)
    {
        printk("Node %d master address is zero\n", node);
        return -1;
    }
    if(!ret)
        master_addr->token[token] = credits;

    return ret;
}
#ifndef _CFE_
EXPORT_SYMBOL(ubus_master_decode_wnd_cfg);
EXPORT_SYMBOL(ubus_master_set_token_credits);
#endif
#endif


#ifndef _CFE_
arch_initcall(bcm_misc_hw_init);
#endif

