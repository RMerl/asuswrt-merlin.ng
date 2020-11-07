/*
  <:copyright-BRCM:2019:proprietary:standard

  Copyright (c) 2019 Broadcom 
  All Rights Reserved

  This program is the proprietary software of Broadcom and/or its
  licensors, and may only be used, duplicated, modified or distributed pursuant
  to the terms and conditions of a separate, written license agreement executed
  between you and Broadcom (an "Authorized License").  Except as set forth in
  an Authorized License, Broadcom grants no license (express or implied), right
  to use, or waiver of any kind with respect to the Software, and Broadcom
  expressly reserves all rights in and to the Software and all intellectual
  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

  Except as expressly set forth in the Authorized License,

  1. This program, including its structure, sequence and organization,
  constitutes the valuable trade secrets of Broadcom, and you shall use
  all reasonable efforts to protect the confidentiality thereof, and to
  use this information only in connection with your use of Broadcom
  integrated circuit products.

  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
  ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
  FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
  COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
  PERFORMANCE OF THE SOFTWARE.

  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
  ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
  WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
  IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
  OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
  SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
  SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
  LIMITED REMEDY.
  :> 
*/

/*
*******************************************************************************
*
* File Name  : sysport_wol.c
*
* Description: This file implements WOL / ACPI feature for Archer.
*              (note WOL is specific to BCM47622 for now)
*
*******************************************************************************
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>

#include "bcm_map_part.h"
#include "bcm_intr.h"

#include "sysport_rsb.h"
#include "sysport_classifier.h"
#include "archer.h"
#include "archer_driver.h"
#include "sysport_driver.h"

#if defined(CONFIG_BCM947622)
typedef struct {
    struct net_device   *dev;
    int intf_index;
} sysport_dev_dma_intf_t;

static sysport_dev_dma_intf_t sysport_dev_dma_intf_g[BCM_ENET_SYSPORT_BLOG_CHNL_MAX];

#define ARCHER_SYSPORT_INTF     2 //BCM47622 has 2 systemport

volatile sysport *sysport_return_regp (int intf_index)
{
    volatile sysport *sysport_p;

    switch (intf_index)
    {
        case 0:
            sysport_p = SYSPORT(0);
            break;
        case 1:
            sysport_p = SYSPORT(1);
            break;
        default:
            __logError ("intf_index %d not supported\n", intf_index);
            sysport_p = NULL;
            break;
    }
    return sysport_p;
}

/*
*******************************************************************************
* Function   : sysport_device_to_dma_intf
* Description: return sysport DMA interface based on network device name
*******************************************************************************
*/
static int sysport_device_to_dma_intf (char *intf_name, int *intf_index)
{
    int i, ret = -1;
    struct net_device *dev;

    /* look for the name of the root device */
    dev = dev_get_by_name(&init_net, intf_name);
    if (!dev)
    {
        __logError("invalid device requested %s\n", intf_name);
        return -1;
    }
    while(1)
    {
        if(netdev_path_is_root(dev))
            break;

        dev_put(dev);
        dev = netdev_path_next_dev(dev);
        dev_hold(dev);
    }
    dev_put(dev);

    /* root device found, look for the systemport interface */
    for (i=0; i < BCM_ENET_SYSPORT_BLOG_CHNL_MAX; i++)
    {
        if (dev->name == sysport_dev_dma_intf_g[i].dev->name)
        {
            *intf_index = sysport_dev_dma_intf_g[i].intf_index;
            ret = 0;

            __logDebug("DMA interface found, root device %s dma index %d\n", dev->name, sysport_dev_dma_intf_g[i].intf_index);
            break;
        }
    }
    return ret;
}

/*
*******************************************************************************
* Function   : sysport_wol_dev_mapping
* Description: keep track of network device and sysport interface
*******************************************************************************
*/
void sysport_wol_intf_dev_mapping (int port_idx, bcmSysport_BlogChnl_t *blog_chnl)
{
    sysport_dev_dma_intf_g[port_idx].dev = blog_chnl->dev;
    sysport_dev_dma_intf_g[port_idx].intf_index = blog_chnl->sysport;
}

/*
*******************************************************************************
* Function   : sysport_wol_mpd_cfg
* Description: configure magic packet system port interface would wake up to
*******************************************************************************
*/
void sysport_wol_mpd_cfg (archer_mpd_cfg_t *mpd_cfg)
{
    uint32_t mac_h, mac_l;
    volatile sysport *sysport_p;
    unsigned char *mac_addr;
    int intf_index;

    if (sysport_device_to_dma_intf(mpd_cfg->intf_name, &intf_index) == 0)
    {
        if (mpd_cfg->mode == ARCHER_MPD_INTF)
        {
            struct net_device *dev;

            dev = dev_get_by_name (&init_net, mpd_cfg->intf_name);
            mac_addr = dev->dev_addr;
        }
        else
        {
            mac_addr = mpd_cfg->mac_addr;
        }

        sysport_p = sysport_return_regp(intf_index);

        /* configure the mac address for MPD */
        mac_h = (uint32_t)mac_addr[0] << 24 | (uint32_t)mac_addr[1] << 16 |
            (uint32_t)mac_addr[2] << 8  | mac_addr[3];
        mac_l = (uint32_t)mac_addr[4] << 8  | mac_addr[5];

        sysport_p->SYSTEMPORT_UNIMAC.SYSTEMPORT_UMAC_MAC0 = mac_h;
        sysport_p->SYSTEMPORT_UNIMAC.SYSTEMPORT_UMAC_MAC1 = mac_l;
    }
}

/*
*******************************************************************************
* Function   : sysport_wol_enter
* Description: set specific system port to WOL mode 
this function is called assuming MPD MAC addresses has been programmed
*******************************************************************************
*/
void sysport_wol_enter(char *dev_name)
{
    uint32_t v32;
    int intf_index;
    volatile sysport *sysport_p;
    
    if (sysport_device_to_dma_intf(dev_name, &intf_index) == 0)
    {
        sysport_p = sysport_return_regp(intf_index);

        // enable both ARP and MPD
        v32 = sysport_p->SYSTEMPORT_RXCHK.SYSTEMPORT_RXCHK_CONTROL;
        v32 |= SYSPORT_RXCHK_CONTROL_ARP_INTR_EN_M;
        sysport_p->SYSTEMPORT_RXCHK.SYSTEMPORT_RXCHK_CONTROL = v32;

        v32 = sysport_p->SYSTEMPORT_MPD.SYSTEMPORT_MPD_CTRL;
        v32 |= SYSPORT_MPD_CTRL_MPD_EN;
        sysport_p->SYSTEMPORT_MPD.SYSTEMPORT_MPD_CTRL = v32;
    }
}

/*
*******************************************************************************
* WOL Proc entries implementation Function 
*******************************************************************************
*/
#define PROC_DIR        "driver/archer"
#define WOL_PROC_FILE   "/wol_status"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *wol_proc_entry;

static ssize_t archer_wol_status_procfs(struct file *file, char __user *page,
                                        size_t len, loff_t *offset)
{
    int i, bytes = 0;
    uint32_t v32;
    volatile sysport *sysport_p;

    if (*offset == 0)
    {
        bytes += sprintf (page, "number of sysport interfaces: %d\n", ARCHER_SYSPORT_INTF);

        for (i=0; i < ARCHER_SYSPORT_INTF; i++)
        {
            sysport_p = sysport_return_regp(i);

            v32 = sysport_p->SYSTEMPORT_RBUF.SYSTEMPORT_RBUF_RBUF_STATUS;

            bytes += sprintf (page+bytes, "interface %d in %s mode\n", i, 
                              (v32 & SYSPORT_RBUF_STATUS_WOL_M)? "WOL" : "Active");
        }
        *offset += bytes;
    }

    return bytes;
}

static struct file_operations archer_wol_interface_status_proc = {
    .read = archer_wol_status_procfs,
};

int sysport_wol_proc_init(void)
{
    int ret = 0;
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir)
    {
        __logError("Could not create Archer WOL PROC directory\n");
        ret = -1;
    }
    wol_proc_entry = proc_create_data (PROC_DIR WOL_PROC_FILE, 
                                       S_IRUGO, NULL, &archer_wol_interface_status_proc, NULL);

    if (!wol_proc_entry)
    {
        __logError("failed to create wol status entry\n");
        ret = -1;
    }
    return ret;
}

static FN_HANDLER_RT sysport_wol_isr(int irq, void *param)
{

    uint32_t v32, rbuf_status;
    int intf = (int)param;
    volatile sysport *sysport_p = sysport_return_regp(intf);
    volatile SYSTEMPORT_X_INTRL2_PHY *phy_intrl2_p = &sysport_p->SYSTEMPORT_1_INTRL2_PHY;
    
    rbuf_status = sysport_p->SYSTEMPORT_RBUF.SYSTEMPORT_RBUF_RBUF_STATUS;

    // disable ACPI (WOL) mode
    v32 = sysport_p->SYSTEMPORT_RBUF.SYSTEMPORT_RBUF_RBUF_CONTROL;
    v32 &= ~SYSPORT_RBUF_CTRL_ACPI_EN_M;
    sysport_p->SYSTEMPORT_RBUF.SYSTEMPORT_RBUF_RBUF_CONTROL = v32;

    // disable MPD
    v32 = sysport_p->SYSTEMPORT_MPD.SYSTEMPORT_MPD_CTRL;
    v32 &= ~(0x1); // MPD disable
    sysport_p->SYSTEMPORT_MPD.SYSTEMPORT_MPD_CTRL = v32;

    // clear ARP detection status
    phy_intrl2_p->SYSTEMPORT_INTRL2_PHY_CPU_CLEAR = 0x10000;
    // disable WOL interrupt on ARP
    v32 = sysport_p->SYSTEMPORT_RXCHK.SYSTEMPORT_RXCHK_CONTROL;
    v32 &= ~SYSPORT_RXCHK_CONTROL_ARP_INTR_EN_M;
    v32 &= ~SYSPORT_RXCHK_CONTROL_BRCM_TAG_MATCH_EN_M;
    sysport_p->SYSTEMPORT_RXCHK.SYSTEMPORT_RXCHK_CONTROL = v32;

    bcm_print("\n* WOL IRQ %d ** rbuf status 0x%x\n", irq, rbuf_status);

    return BCM_IRQ_HANDLED;
}

/*
*******************************************************************************
* WOL initialization 
*******************************************************************************
*/
int sysport_wol_init(void)
{
    int ret;
    volatile sysport *sysport_p = SYSPORT(0);
    volatile SYSTEMPORT_X_INTRL2_PHY *phy_intrl2_p = &sysport_p->SYSTEMPORT_1_INTRL2_PHY;
    int wol_interrupt_id = SYSPORT_WOL_INTERRUPT_ID(0);

    BcmHalMapInterrupt(sysport_wol_isr, 0, wol_interrupt_id);

    bcm_print("Sysport 0 WOL IRQ %d\n", wol_interrupt_id);

    /* Enable RX ARP Interrupts for WOL */
    phy_intrl2_p->SYSTEMPORT_INTRL2_PHY_CPU_MASK_CLEAR = 0x10000;

#if ARCHER_SYSPORT_INTF > 1
    sysport_p = SYSPORT(1);
    phy_intrl2_p = &sysport_p->SYSTEMPORT_1_INTRL2_PHY;
    wol_interrupt_id = SYSPORT_WOL_INTERRUPT_ID(1);

    BcmHalMapInterrupt(sysport_wol_isr, (void *)1, wol_interrupt_id);

    bcm_print("Sysport 1 WOL IRQ %d\n", wol_interrupt_id);

    /* Enable RX ARP Interrupts for WOL */
    phy_intrl2_p->SYSTEMPORT_INTRL2_PHY_CPU_MASK_CLEAR = 0x10000;
#endif

    memset(&sysport_dev_dma_intf_g, 0, sizeof(sysport_dev_dma_intf_g));

    ret = sysport_wol_proc_init();

    return ret;
}

#endif
