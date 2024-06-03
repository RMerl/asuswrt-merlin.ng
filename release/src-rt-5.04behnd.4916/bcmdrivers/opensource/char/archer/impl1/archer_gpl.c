/*
   <:copyright-BRCM:2021:DUAL/GPL:standard
   
      Copyright (c) 2021 Broadcom 
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
*
* File Name  : archer_gpl.c
*
* Description: Archer GPL Driver
*
*******************************************************************************
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/bcm_log.h>
#include <linux/kthread.h>
#include <linux/nbuff.h>
#include <linux/blog.h>
#include <linux/bcm_skb_defines.h>
#include <net/ipv6.h>
#include "shared_utils.h"
#include <bcm_OS_Deps.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0))
#include <linux/dma-map-ops.h>
#endif

#include "archer_gpl.h"
#include "crossbow_gpl.h"

#define ARCHER_GPL_NAME_SIZE  32

#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_ARCHER, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_ARCHER, fmt, ##arg)

#define __debug(fmt, arg...)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
            bcm_print(fmt, ##arg); )

#define ARCHER_GPL_SYSPORT_INTF_MAX   2

typedef struct {
    void __iomem *virt;
    union {
        archer_phys_addr_t phys;
        uint64_t phys_u64;
    };
} archer_gpl_block_t;

typedef struct {
    archer_gpl_block_t block;
    int irq[ARCHER_GPL_SYSPORT_IRQ_MAX];
} archer_gpl_sysport_intf_t;

typedef struct {
    struct platform_device *pdev;
    archer_gpl_sysport_intf_t intf[ARCHER_GPL_SYSPORT_INTF_MAX];
    int intf_count;
} archer_gpl_sysport_t;

typedef struct {
    void __iomem *virt;
    int irq[ARCHER_GPL_SAR_IUDMA_IRQ_MAX];
} archer_gpl_sar_iudma_t;

#if defined(CONFIG_BCM_ARCHER_GSO)
typedef struct {
    struct net_device *dev;
    struct net_device *skb_dev_orig;
    pNBuff_t *pNBuff_list;
    int nbuff_count;
    int nbuff_max;
} archer_driver_gso_t;
#endif

typedef struct {
    archer_gpl_sysport_t sysport;
    archer_gpl_sar_iudma_t sar_iudma;
    archer_gpl_crossbow_t crossbow;
#if defined(CONFIG_BCM_ARCHER_GSO)
    archer_driver_gso_t gso;
#endif
    struct device *dummy_dev;
} archer_gpl_t;

static archer_gpl_t archer_gpl_g;

static struct device *archer_dummy_dev_g;

/*******************************************************************************
 *
 * Memory Allocation
 *
 *******************************************************************************/

static int __init archer_dummy_device_alloc(void)
{
    if(archer_dummy_dev_g == NULL)
    {
        archer_dummy_dev_g = kzalloc(sizeof(struct device), GFP_KERNEL);

        if(archer_dummy_dev_g == NULL)
        {
            __logError("Could not allocate dummy_dev");

            return -1;
        }

#ifdef CONFIG_BCM_GLB_COHERENCY
        arch_setup_dma_ops(archer_dummy_dev_g, 0, 0, NULL, true);
#else
        arch_setup_dma_ops(archer_dummy_dev_g, 0, 0, NULL, false);
#endif
#if defined(CC_ARCHER_PHYS_ADDR_40BIT)
        dma_coerce_mask_and_coherent(archer_dummy_dev_g, DMA_BIT_MASK(40));
#else
        dma_coerce_mask_and_coherent(archer_dummy_dev_g, DMA_BIT_MASK(32));
#endif
    }

    return 0;
}

static void __exit archer_dummy_device_free(void)
{
    if(archer_dummy_dev_g)
    {
        kfree(archer_dummy_dev_g);
    }
}

volatile void *archer_coherent_mem_alloc(int size, archer_phys_addr_t *phys_addr_p)
{
    dma_addr_t dma_addr;

    volatile void *p = dma_alloc_coherent(archer_dummy_dev_g,
                                          size, &dma_addr, GFP_KERNEL);

    if(dma_addr & (L1_CACHE_BYTES-1))
    {
        __logError("Not Cache Aligned\n");

        dma_free_coherent(archer_dummy_dev_g, size, (void *)p, dma_addr);

        return NULL;
    }

    *phys_addr_p = (archer_phys_addr_t)dma_addr;

    return p; /* return host address */
}
EXPORT_SYMBOL(archer_coherent_mem_alloc);

void archer_coherent_mem_free(int size, archer_phys_addr_t phys_addr, volatile void *p)
{
    dma_addr_t dma_addr = (dma_addr_t)phys_addr;

    dma_free_coherent(archer_dummy_dev_g, size, (void *)p, dma_addr);
}
EXPORT_SYMBOL(archer_coherent_mem_free);

/*******************************************************************************
 *
 * Archer GSO
 *
 *******************************************************************************/

#if defined(CONFIG_BCM_ARCHER_GSO)
static int archer_dev_open(struct net_device *dev)
{
    bcm_print("Open %s Netdevice\n", dev->name);

    return 0;
}

static int archer_dev_change_mtu(struct net_device *dev, int new_mtu)
{
    if(new_mtu < ETH_ZLEN || new_mtu > ENET_MAX_MTU_PAYLOAD_SIZE)
    {
        return -EINVAL;
    }

    dev->mtu = new_mtu;

    return 0;
}

static netdev_tx_t archer_dev_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    archer_driver_gso_t *gso_p = &archer_gpl_g.gso;

    if(likely(gso_p->nbuff_count < gso_p->nbuff_max))
    {
        dev->stats.tx_packets++;
        dev->stats.tx_bytes += skb->len;

        gso_p->pNBuff_list[gso_p->nbuff_count++] = SKBUFF_2_PNBUFF(skb);
    }
    else
    {
        dev->stats.tx_dropped++;

#if defined(CONFIG_BCM_GLB_COHERENCY)
        nbuff_free(SKBUFF_2_PNBUFF(skb));
#else
        nbuff_flushfree(SKBUFF_2_PNBUFF(skb));
#endif
    }

//    bcm_print("%s,%d: count %d, max %d\n", __FUNCTION__, __LINE__, gso_p->nbuff_count, gso_p->nbuff_max);

    return NETDEV_TX_OK;
}

void archer_gso(pNBuff_t pNBuff, int nbuff_max, pNBuff_t *pNBuff_list, int *nbuff_count_p)
{
    archer_driver_gso_t *gso_p = &archer_gpl_g.gso;

    if(IS_SKBUFF_PTR(pNBuff))
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

        gso_p->skb_dev_orig = skb->dev;
        gso_p->pNBuff_list = pNBuff_list;
        gso_p->nbuff_count = 0;
        gso_p->nbuff_max = nbuff_max;

        gso_p->dev->stats.rx_packets++;
        gso_p->dev->stats.rx_bytes += skb->len;

        // Send SKB through the Linux stack and let it do GSO
        skb->dev = gso_p->dev;

        dev_queue_xmit(skb);

//        bcm_print("%s,%d: count %d\n", __FUNCTION__, __LINE__, gso_p->nbuff_count);

        *nbuff_count_p = gso_p->nbuff_count;

        // All non-GSO packets transmitted to the dummy archer interface should be dropped
        gso_p->nbuff_count = 0;
        gso_p->nbuff_max = 0;
    }
    else
    {
        pNBuff_list[0] = pNBuff;

        *nbuff_count_p = 1;
    }
}
EXPORT_SYMBOL(archer_gso);

typedef struct {
    int rx_count;
} archer_netdevice_t;

static const struct net_device_ops archer_netdev_ops_g =
{
    .ndo_open = archer_dev_open,
    .ndo_start_xmit = archer_dev_start_xmit,
    .ndo_change_mtu = archer_dev_change_mtu
};

static struct net_device * __init archer_create_netdevice(void)
{
    struct net_device *dev;
    int ret;

    dev = alloc_netdev(sizeof(archer_netdevice_t), "archer",
                       NET_NAME_UNKNOWN, ether_setup);
    if(!dev)
    {
        __logError("Failed to allocate Archer netdev\n");

        return NULL;
    }

#if defined(CONFIG_BCM_KF_NETDEV_EXT)
    /* Mark device as BCM */
    netdev_bcm_dev_set(dev);
#endif

    dev->watchdog_timeo = 2 * HZ;

    netif_carrier_off(dev);
    netif_stop_queue(dev);

    dev->netdev_ops = &archer_netdev_ops_g;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
    dev->needs_free_netdev = false;
    dev->priv_destructor = free_netdev;
    dev->priv_flags |= IFF_NO_QUEUE;
#else
    dev->destructor = free_netdev;
#endif

    dev->tx_queue_len = 0;

    rtnl_lock();

    ret = register_netdevice(dev);
    if(ret)
    {
        __logError("Failed to register Archer netdev\n");

        rtnl_unlock();

        free_netdev(dev);

        return NULL;
    }
    else
    {
        __logDebug("Registered Archer netdev\n");
    }

    archer_dev_change_mtu(dev, BCM_ENET_DEFAULT_MTU_SIZE);

    netif_start_queue(dev);
    netif_carrier_on(dev);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0))
    ret = dev_open(dev, NULL);
#else
    ret = dev_open(dev);
#endif

    rtnl_unlock();

    return dev;
}

static void __exit archer_remove_netdevice(struct net_device *dev)
{
    __logDebug("Unregister %s netdev\n", dev->name);
    
    rtnl_lock();

    unregister_netdevice(dev);

    rtnl_unlock();
}
#endif /* CONFIG_BCM_ARCHER_GSO */

/*******************************************************************************
 *
 * Ethernet Driver Binding
 *
 *******************************************************************************/

int archer_gpl_cpu_affinity_set(struct task_struct *task_p)
{
    int retry = 10;
    int ret;

    do {
        ret = set_cpus_allowed_ptr(task_p, cpumask_of(CONFIG_BCM_ARCHER_CPU_AFFINITY));
        udelay(500);
    } while(ret && --retry);

    if(ret)
    {
        __logError("Could not set Archer CPU Affinity = %d, ret %d",
                   CONFIG_BCM_ARCHER_CPU_AFFINITY, ret);
        BUG();
    }
    else
    {
        bcm_print("Archer CPU Affinity = %d\n", CONFIG_BCM_ARCHER_CPU_AFFINITY);
    }

    return ret;
}

#if !defined(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD)

int archer_gpl_enet_bind(archer_host_hooks_t *hooks_p)
{
    bcmFun_t *archer_host_bind = bcmFun_get(BCM_FUN_ID_ARCHER_HOST_BIND);
    int ret;

    if(!archer_host_bind)
    {
        __logError("Archer binding is not available");

        return -1;
    }

    hooks_p->archer_task_p = NULL;

    ret = archer_host_bind(hooks_p);
    if(ret)
    {
        __logError("Could not archer_host_bind");

        return ret;
    }

    BCM_ASSERT(hooks_p->archer_task_p);

    return archer_gpl_cpu_affinity_set(hooks_p->archer_task_p);
}
EXPORT_SYMBOL(archer_gpl_enet_bind);

#endif /* !CONFIG_BCM_CROSSBOW_FULL_OFFLOAD */

/*******************************************************************************
 *
 * Interrupts
 *
 *******************************************************************************/

#define ARCHER_GPL_INTR_NAME_SIZE  32

const char *archer_gpl_irq_type_name_g[ARCHER_GPL_IRQ_TYPE_MAX] =
{
    "_rxq",
    "_txq",
    "_wol",
    "_usr",
    "",
    "_socket",
    "_mailbox"
};

static int archer_gpl_request_irq(irq_handler_t handler, void *param, int irq,
                                  archer_gpl_irq_type_t type, int type_index,
                                  char *block_name)
{
    unsigned long irq_flags = 0;
    char *irq_name;
    int ret;

    irq_name = kmalloc(ARCHER_GPL_INTR_NAME_SIZE, GFP_KERNEL);

    if(irq_name == NULL)
    {
        __logError("kmalloc(%d, GFP_KERNEL) failed\n", ARCHER_GPL_INTR_NAME_SIZE);

        return -1;
    }

    snprintf(irq_name, ARCHER_GPL_INTR_NAME_SIZE, "%s%s[%d]:%d",
             block_name, archer_gpl_irq_type_name_g[type], type_index, irq);

    ret = request_irq(irq, handler, irq_flags, irq_name, param);
    if(ret)
    {
        __logError("request_irq failed for irq %d (%s), ret %d",
                   irq, irq_name, ret);

        kfree(irq_name);
    }

    return ret;
}

int archer_gpl_enet_config(bcmSysport_Config_t *sysport_p)
{
    bcmFun_t *enet_sysport_config = bcmFun_get(BCM_FUN_ID_ENET_SYSPORT_CONFIG);
    archer_gpl_enet_config_t config;
    int intr;
    int ret;

    if(!enet_sysport_config)
    {
        __logError("Sysport Configuration is not available\n");

        return -1;
    }

    config.sysport_p = sysport_p;

    memset(config.intr, 0, sizeof(archer_gpl_intr_t) * ARCHER_GPL_INTR_MAX);

    ret = enet_sysport_config(&config);
    if(ret)
    {
        __logError("Could not enet_sysport_config");

        return ret;
    }

    for(intr=0; intr<ARCHER_GPL_INTR_MAX; ++intr)
    {
        archer_gpl_intr_t *intr_p = &config.intr[intr];

        if(intr_p->isr)
        {
            char *block_name;
            int irq;

            if(ARCHER_GPL_IRQ_TYPE_DSL == intr_p->type)
            {
                if(intr_p->intc >= ARCHER_GPL_SAR_IUDMA_IRQ_MAX)
                {
                    __logError("SAR INTR[%u]: Invalid intc %d (max %u)",
                               intr, intr_p->intc, ARCHER_GPL_SAR_IUDMA_IRQ_MAX);
                    return -1;
                }

                irq = archer_gpl_g.sar_iudma.irq[intr_p->intc];

                block_name = "sar_iudma";
            }
            else if(intr_p->intf_index >= archer_gpl_g.sysport.intf_count)
            {
                __logError("SYSPORT INTR[%u]: Invalid intf_index %d (max %u)",
                           intr, intr_p->intf_index, archer_gpl_g.sysport.intf_count);
                return -1;
            }
            else if(intr_p->intc >= ARCHER_GPL_SYSPORT_IRQ_MAX)
            {
                __logError("SYSPORT INTR[%u]: Invalid intc %d (max %u)",
                           intr, intr_p->intc, ARCHER_GPL_SYSPORT_IRQ_MAX);
                return -1;
            }
            else
            {
                archer_gpl_sysport_intf_t *intf_p =
                    &archer_gpl_g.sysport.intf[intr_p->intf_index];
                int irq_index = sysport_intc_to_irq_index(intr_p->intc);

                irq = intf_p->irq[irq_index];

#if defined(CONFIG_BCM_CROSSBOW)
                block_name = "crossbow";
#else
                block_name = (!intr_p->intf_index) ? "sysport" : "sysport1";
#endif
            }

            ret = archer_gpl_request_irq(intr_p->isr, intr_p->param, irq,
                                         intr_p->type, intr_p->type_index,
                                         block_name);
            if(ret)
            {
                return ret;
            }

            if(intr_p->cpu_id != ARCHER_GPL_CPU_ID_INVALID)
            {
                if(bcm_set_affinity(irq, intr_p->cpu_id, 0))
                {
                    __logError("Could not bcm_set_affinity");

                    return -1;
                }
            }

            bcm_print("Archer INTR[%u]: irq %u, cpu_id %d, %s%s (0x%px)\n",
                      intr, irq, intr_p->cpu_id, block_name,
                      archer_gpl_irq_type_name_g[intr_p->type], intr_p->isr);
        }
    }

    return 0;
}
EXPORT_SYMBOL(archer_gpl_enet_config);

#if defined(CONFIG_BCM_CROSSBOW)

volatile SYSTEMPORT_INTRL2 *
archer_gpl_intc_to_intrl2(volatile sysport *sysport_p, int intc)
{
    int intrl2 = SYSPORT_INTC_TO_INTRL2(intc);
    int intr = SYSPORT_INTC_TO_INTRL2_INTR(intc);

    return &sysport_p->SYSTEMPORT_INTRL2_DUAL[intrl2].intr[intr];
}

volatile SYSTEMPORT_INTC *
archer_gpl_intc_to_intrl2_intc(volatile sysport *sysport_p, int intc)
{
    int intrl2 = SYSPORT_INTC_TO_INTRL2(intc);

    return &sysport_p->SYSTEMPORT_INTC[intrl2];
}

#else /* !CONFIG_BCM_CROSSBOW */

volatile SYSTEMPORT_INTRL2 *
archer_gpl_intc_to_intrl2(volatile sysport *sysport_p, int intc)
{
    return &sysport_p->SYSTEMPORT_INTRL2[intc];
}

volatile SYSTEMPORT_INTC *
archer_gpl_intc_to_intrl2_intc(volatile sysport *sysport_p, int intc)
{
    return &sysport_p->SYSTEMPORT_INTC[intc];
}

#endif /* CONFIG_BCM_CROSSBOW */

EXPORT_SYMBOL(archer_gpl_intc_to_intrl2);
EXPORT_SYMBOL(archer_gpl_intc_to_intrl2_intc);

/*******************************************************************************
 *
 * System Port Resources
 *
 *******************************************************************************/

#define ARCHER_GPL_SYSPORT_RES_ERROR  -ENODEV

static int archer_gpl_sysport_resource(archer_gpl_block_t *block_p, char *block_name, int all_error)
{
    struct device *dev = &archer_gpl_g.sysport.pdev->dev;
    struct resource *res;

    res = platform_get_resource_byname(archer_gpl_g.sysport.pdev,
                                       IORESOURCE_MEM, block_name);
    if(!res)
    {
        if(all_error)
        {
            dev_err(dev, "Could not find %s\n", block_name);
        }

        return ARCHER_GPL_SYSPORT_RES_ERROR;
    }

    block_p->virt = devm_ioremap(dev, res->start, resource_size(res));

    if(IS_ERR_OR_NULL(block_p->virt))
    {
        dev_err(dev, "Could not map %s\n", block_name);

        block_p->virt = NULL;

        return -ENXIO;
    }

    block_p->phys = res->start;

    dev_info(dev, "%s (0x%px, 0x%llx): %pr\n", block_name, block_p->virt, block_p->phys_u64, res);

    return 0;
}

static int archer_gpl_sysport_probe(struct platform_device *pdev)
{
    archer_gpl_sysport_intf_t *intf_p;
    char name[ARCHER_GPL_NAME_SIZE];
    int i;
    int ret;

    archer_gpl_g.sysport.pdev = pdev;

    // SYSPORT 0

    intf_p = &archer_gpl_g.sysport.intf[0];

    if((ret = archer_gpl_sysport_resource(&intf_p->block, "sysport-base", 1)))
    {
        return ret;
    }

    archer_gpl_g.sysport.intf_count = 1;

    for(i=0; i<ARCHER_GPL_SYSPORT_IRQ_MAX; ++i)
    {
        snprintf(name, ARCHER_GPL_NAME_SIZE, "sysport-irq-%u", i);

        intf_p->irq[i] = platform_get_irq_byname(pdev, name);
        if(intf_p->irq[i] > 0)
        {
            bcm_print("SYSPORT[0] IRQ[%u]: %d\n", i, intf_p->irq[i]);
        }
        else
        {
            __logError("IRQ[%d] %s failed\n", i, name);

            return -EINVAL;
        }
    }

    // SYSPORT 1

    intf_p = &archer_gpl_g.sysport.intf[1];

    ret = archer_gpl_sysport_resource(&intf_p->block, "sysport1-base", 0);
    if(!ret)
    {
        archer_gpl_g.sysport.intf_count = 2;

        for(i=0; i<ARCHER_GPL_SYSPORT_IRQ_MAX; ++i)
        {
            snprintf(name, ARCHER_GPL_NAME_SIZE, "sysport1-irq-%u", i);

            intf_p->irq[i] = platform_get_irq_byname(pdev, name);
            if(intf_p->irq[i] > 0)
            {
                bcm_print("SYSPORT[1] IRQ[%u]: %d\n", i, intf_p->irq[i]);
            }
            else
            {
                __logError("Could not platform_get_irq_byname");

                return -EINVAL;
            }
        }
    }
    else if(ret != ARCHER_GPL_SYSPORT_RES_ERROR)
    {
        return ret;
    }

    bcm_print("Found %d System Port(s)\n", archer_gpl_g.sysport.intf_count);

    return 0;
}

static const struct of_device_id archer_gpl_sysport_of_match[] = {
    { .compatible = "brcm,sysport-blk", .data = NULL, },
    {},
};
MODULE_DEVICE_TABLE(of, archer_gpl_sysport_of_match);

static struct platform_driver archer_gpl_sysport_driver = {
    .probe = archer_gpl_sysport_probe,
    .driver = {
        .name = "bcm-archer-gpl",
        .of_match_table = archer_gpl_sysport_of_match,
    },
};

static int __init archer_gpl_sysport_init(void)
{
    return platform_driver_register(&archer_gpl_sysport_driver);
}

int archer_gpl_sysport_get(int intf_index, volatile sysport **sysport_p,
                           archer_phys_addr_t *phys_addr_p)
{
    archer_gpl_sysport_intf_t *intf_p = &archer_gpl_g.sysport.intf[intf_index];

    if(intf_index >= archer_gpl_g.sysport.intf_count)
    {
        __logError("Invalid intf_index %d (max %u)",
                   intf_index, archer_gpl_g.sysport.intf_count);

        return -1;
    }

    *sysport_p = intf_p->block.virt;
    *phys_addr_p = intf_p->block.phys;

    return 0;
}
EXPORT_SYMBOL(archer_gpl_sysport_get);

/*******************************************************************************
 *
 * DSL SAR Resources
 *
 *******************************************************************************/

static int __init archer_gpl_dsl_init(void)
{
    struct device_node *np;
    char name[ARCHER_GPL_NAME_SIZE];
    int i;

    np = of_find_compatible_node(NULL, NULL, "brcm,xtmcfg");
    if(np)
    {
        archer_gpl_g.sar_iudma.virt = (void *)of_iomap(np, 1);

        for(i=0; i<ARCHER_GPL_SAR_IUDMA_IRQ_MAX; ++i)
        {
            snprintf(name, ARCHER_GPL_NAME_SIZE, "sar-irq-%u", i);

            archer_gpl_g.sar_iudma.irq[i] = of_irq_get_byname(np, name);
            if(archer_gpl_g.sar_iudma.irq[i] > 0)
            {
                bcm_print("SAR IRQ[%u]: %d\n", i, archer_gpl_g.sar_iudma.irq[i]);
            }
            else
            {
                __logError("Could not of_irq_get_byname");

                return -EINVAL;
            }
        }

        of_node_put(np);

        bcm_print("Found DSL SAR (0x%px)\n", archer_gpl_g.sar_iudma.virt);
    }

    return 0;
}

void *archer_gpl_sar_iudma_get(void)
{
    return archer_gpl_g.sar_iudma.virt;
}
EXPORT_SYMBOL(archer_gpl_sar_iudma_get);

/*******************************************************************************
 *
 * Crossbow Resources
 *
 *******************************************************************************/

static int __init archer_gpl_crossbow_init(void)
{
    struct device_node *np;

    np = of_find_compatible_node(NULL, NULL, "brcm,crossbow");
    if(np)
    {
        archer_gpl_g.crossbow.cnp_p = (void *)of_iomap(np, 0);
        archer_gpl_g.crossbow.natc_p = (void *)of_iomap(np, 1);
        archer_gpl_g.crossbow.cm7_p = (void *)of_iomap(np, 2);

        of_node_put(np);

        bcm_print("CROSSBOW: cnp_p 0x%px, natc_p 0x%px, cm7_p 0x%px\n",
                  archer_gpl_g.crossbow.cnp_p, archer_gpl_g.crossbow.natc_p,
                  archer_gpl_g.crossbow.cm7_p);
    }

    return 0;
}

void archer_gpl_crossbow_get(archer_gpl_crossbow_t *crossbow_p)
{
    *crossbow_p = archer_gpl_g.crossbow;
}
EXPORT_SYMBOL(archer_gpl_crossbow_get);

/*******************************************************************************
 *
 * Driver Initialization
 *
 *******************************************************************************/

int __init archer_construct(void)
{
    int ret;

    bcm_print(CLRcb "Broadcom Archer GPL Driver" CLRnl);

    memset(&archer_gpl_g, 0, sizeof(archer_gpl_t));

    ret = archer_gpl_sysport_init();
    if(ret)
    {
        __logError("Could not archer_gpl_sysport_init");

        return ret;
    }

    ret = archer_gpl_dsl_init();
    if(ret)
    {
        __logError("Could not archer_gpl_dsl_init");

        return ret;
    }

    ret = archer_gpl_crossbow_init();
    if(ret)
    {
        __logError("Could not archer_gpl_crossbow_init");

        return ret;
    }

    ret = archer_dummy_device_alloc();
    if(ret)
    {
        return ret;
    }

#if defined(CONFIG_BCM_ARCHER_GSO)
    archer_gpl_g.gso.dev = archer_create_netdevice();
    if(!archer_gpl_g.gso.dev)
    {
        return -1;
    }
#endif

#if defined(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD)
    ret = crossbow_gpl_construct();
    if(ret)
    {
        return ret;
    }
#endif

    return 0;
}

void __exit archer_destruct(void)
{
#if defined(CONFIG_BCM_ARCHER_GSO)
    archer_remove_netdevice(archer_gpl_g.gso.dev);
#endif

    archer_dummy_device_free();
}

module_init(archer_construct);
module_exit(archer_destruct);

MODULE_DESCRIPTION(ARCHER_MODNAME "GPL");
MODULE_VERSION(ARCHER_VERSION);
MODULE_LICENSE("GPL");
