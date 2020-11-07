/*
 <:copyright-BRCM:2009:DUAL/GPL:standard
 
    Copyright (c) 2009 Broadcom 
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
 *******************************************************************************
 * File Name  : bcmPktDma.c
 *
 * Description: This file contains the Packet DMA initialization API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/skbuff.h>

#include "bcmtypes.h"
#include <bcm_map_part.h>
#include "fap_task.h"
#include "fap_packet.h"
#include "bcmPktDma.h"
#include "bcmPktDmaHooks.h"

bcmPktDma_hostHooks_t bcmPktDma_hostHooks_g;
static RecycleFuncP   bcmPktDma_enet_recycle_hook = NULL;
static RecycleFuncP   bcmPktDma_xtm_fkb_recycle_hook = NULL;
static RecycleFuncP   bcmPktDma_xtm_skb_recycle_hook = NULL;
#if defined(CONFIG_BCM_FAP_LAYER2)
bcmPktDma_arlNotifyHandlerFuncP bcmPktDma_arlNotifyHandlerFuncP_g = NULL;
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#if (MAX_SWITCH_PORTS != 8)
#error "PKTDMA_ETH_PORT_TO_IUDMA does not match MAX_SWITCH_PORTS"
#endif
static uint8 g_Eth_rx_port_to_iudma[MAX_SWITCH_PORTS] =
{
    PKTDMA_ETH_PORT_TO_IUDMA  /* alls ports default to the US iuDMA channel */
};

void mapEthPortToRxIudma(uint8 port, uint8 iudma)
{
    if((port < MAX_SWITCH_PORTS) && (iudma < ENET_RX_CHANNELS_MAX))
    {
        g_Eth_rx_port_to_iudma[port] = iudma;
    }
    else
    {
        printk("%s : Invalid Argument: port <%d>, channel <%d>\n",
               __FUNCTION__, port, iudma);
    }
}

int getEthRxIudmaFromPort(int port)
{
    if(port >= MAX_TOTAL_SWITCH_PORTS)
    {
        printk("%s : Invalid Argument: port <%d>\n", __FUNCTION__, port);
        return PKTDMA_ETH_US_IUDMA;
    }

    if (IsExternalSwitchPort(port))
    {
        port = BpGetPortConnectedToExtSwitch();
    }
    else
    {
        port = LOGICAL_PORT_TO_PHYSICAL_PORT(port);
    }

    if(port < MAX_SWITCH_PORTS)
    {
        return g_Eth_rx_port_to_iudma[port];
    }
    else
    {
        printk("%s : Invalid Argument: port <%d>\n", __FUNCTION__, port);

        return PKTDMA_ETH_US_IUDMA;
    }
}
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */

static void initHostHooks(void)
{
    memset(&bcmPktDma_hostHooks_g, 0, sizeof(bcmPktDma_hostHooks_t));
}

int bcmPktDma_bind(bcmPktDma_hostHooks_t *hostHooks)
{
    if(hostHooks->xmit2Fap == NULL ||
       /* FAP PSM Memory Allocation added Apr 2010 */
       hostHooks->psmAlloc == NULL ||
       hostHooks->dqmXmitMsgHost == NULL ||
       hostHooks->dqmRecvMsgHost == NULL ||
       hostHooks->dqmEnableHost == NULL ||
       hostHooks->dqmEnableNotEmptyIrq == NULL
#if defined(CC_FAP4KE_TM)
        || hostHooks->tmMasterConfig == NULL ||
       hostHooks->tmPortConfig == NULL ||
       hostHooks->tmSetPortMode == NULL ||
       hostHooks->tmGetPortMode == NULL ||
       hostHooks->tmPortType == NULL ||
       hostHooks->tmPortEnable == NULL ||
       hostHooks->tmPauseEnable == NULL ||
       hostHooks->tmApply == NULL ||
       hostHooks->tmCheckSetHighPrio == NULL ||
       hostHooks->tmXtmCheckHighPrio == NULL
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
       || hostHooks->tmXtmQueueDropAlgConfig == NULL
#endif
#endif
       )
    {
        return FAP_ERROR;
    }

    bcmPktDma_hostHooks_g = *hostHooks;

    printk("%s: FAP Driver binding successfull\n", __FUNCTION__);

    return FAP_SUCCESS;
}

void bcmPktDma_unbind(void)
{
    initHostHooks();
}

/* Add code for buffer quick free between enet and xtm - June 2010 */
void bcmPktDma_set_enet_recycle(RecycleFuncP enetRecycle)
{
    bcmPktDma_enet_recycle_hook = enetRecycle;
}

RecycleFuncP bcmPktDma_get_enet_recycle(void)
{
    return(bcmPktDma_enet_recycle_hook);
}

void bcmPktDma_set_xtm_recycle(RecycleFuncP fkbRecycleFunc, RecycleFuncP skbRecycleFunc)
{
    bcmPktDma_xtm_fkb_recycle_hook = fkbRecycleFunc;
    bcmPktDma_xtm_skb_recycle_hook = skbRecycleFunc;
}

RecycleFuncP bcmPktDma_get_xtm_fkb_recycle(void)
{
    return(bcmPktDma_xtm_fkb_recycle_hook);
}

RecycleFuncP bcmPktDma_get_xtm_skb_recycle(void)
{
    return(bcmPktDma_xtm_skb_recycle_hook);
}

#if defined(CONFIG_BCM_FAP_LAYER2)
void bcmPktDma_registerArlNotifyHandler(bcmPktDma_arlNotifyHandlerFuncP arlNotifyHandlerFuncP)
{
    bcmPktDma_arlNotifyHandlerFuncP_g = arlNotifyHandlerFuncP;
}

void bcmPktDma_unregisterArlNotifyHandler(void)
{
    bcmPktDma_arlNotifyHandlerFuncP_g = NULL;
}
#endif /* CONFIG_BCM_FAP_LAYER2 */

int __init bcmPktDma_init(void)
{
    printk("%s: Broadcom Packet DMA Library initialized\n", __FUNCTION__);

    initHostHooks();

    return 0;
}

void __exit bcmPktDma_exit(void)
{
    printk("Broadcom Packet DMA Library exited");
}

module_init(bcmPktDma_init);
module_exit(bcmPktDma_exit);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
EXPORT_SYMBOL(mapEthPortToRxIudma);
EXPORT_SYMBOL(getEthRxIudmaFromPort);
#endif
EXPORT_SYMBOL(bcmPktDma_hostHooks_g);
EXPORT_SYMBOL(bcmPktDma_bind);
EXPORT_SYMBOL(bcmPktDma_unbind);
EXPORT_SYMBOL(bcmPktDma_set_enet_recycle);
EXPORT_SYMBOL(bcmPktDma_get_enet_recycle);
EXPORT_SYMBOL(bcmPktDma_set_xtm_recycle);
EXPORT_SYMBOL(bcmPktDma_get_xtm_fkb_recycle);
EXPORT_SYMBOL(bcmPktDma_get_xtm_skb_recycle);
#if defined(CONFIG_BCM_FAP_LAYER2)
EXPORT_SYMBOL(bcmPktDma_arlNotifyHandlerFuncP_g);
EXPORT_SYMBOL(bcmPktDma_registerArlNotifyHandler);
EXPORT_SYMBOL(bcmPktDma_unregisterArlNotifyHandler);
#endif /* CONFIG_BCM_FAP_LAYER2 */
