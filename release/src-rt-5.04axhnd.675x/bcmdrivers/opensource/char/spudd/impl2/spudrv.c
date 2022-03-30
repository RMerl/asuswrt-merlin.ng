/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
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
/***************************************************************************
 * File Name  : spudrv.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the IPSec SPU.
 *
 * Updates    : 11/16/2007  Pavan Kumar.  Created.
 * Updates    : 10/10/2008  Bhaskara Peela.  
 *
 * NOTE: Rx -- From SPU to Host memory
 *       Tx -- From Host memory to SPU
 ***************************************************************************/


/* Includes. */
#include <linux/version.h>
#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <linux/if_arp.h>
#include <asm/uaccess.h>
#include "bcm_map_part.h"
#include "bcm_mm.h"
#include "bcm_intr.h"
#include "bcmspudrv.h"
#include "spudrv.h"
#include <board.h>
#include <linux/hw_random.h>
#include "spu.h"
#include <linux/nbuff.h>

#ifndef IPSEC_SPU_IFNAME
#define IPSEC_SPU_IFNAME             "spu"
#endif
#define DESC_ALIGN                   16
#define BUF_ALIGN                    4
#define IPSEC_SPU_MSEC_PER_TICK      (1000/HZ)
#define IPSEC_SPU_ALIGN(addr, bound) (((UINT32) addr + bound - 1) & ~(bound - 1))
/* Macros to round down and up, an address to a cachealigned address */
#define ROUNDDN(addr, align)  ( (addr) & ~((align) - 1) )
#define ROUNDUP(addr, align)  ( ((addr) + (align) - 1) & ~((align) - 1) )

extern int bcm_crypto_dev_disable(void);
#ifdef CONFIG_BCM_SPU_TEST
extern int spu_finish_processing (uint8_t *addr, int test_mode);
#else
extern int spu_finish_processing (uint8_t *addr);
#endif
extern int bcm_crypto_dev_init (void);
extern void spu_perform_test(uint32 tx_pkt_id, uint32 num_pkts);

/* typedefs. */
typedef void (*IPSec_SPU_FN_IOCTL) (unsigned long arg);

/* Prototypes. */
static int __init spu_init (void);
static void spu_cleanup (void);
static int spu_open (struct inode *inode, struct file *filp);
static int spu_ioctl (struct inode *inode, struct file *flip,
                      unsigned int command, unsigned long arg);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static long spu_ioctl_unlocked(struct file *filep, unsigned int cmd, 
                               unsigned long arg);
#endif
static int spu_release (struct inode *inode, struct file *filp);
static int spu_init_dev (void);
static void spu_init_dma (void);
static pspu_dev_ctrl_t spu_alloc_dev (void);
static irqreturn_t spu_isr (int irq, void *, struct pt_regs *regs);
static void do_spudd_initialize (unsigned long arg);
static void do_spudd_uninitialize (unsigned long arg);
static void do_spu_test(unsigned long arg);
static void do_spu_show (unsigned long arg);
unsigned long spu_get_cycle_count(void);
static void spu_reclaim_tx_descriptors(int numbds);

/*
 * Performance variables
 */
unsigned long start = 0;
unsigned long end = 0;
unsigned long proc_time = 0;

/* DMA buffers */
unsigned long spu_cycle_per_us = 0;

/* device structure */
struct spu_device *spudevice = NULL;
/* Device control structure */
pspu_dev_ctrl_t pdev_ctrl = NULL;

#ifdef SPU_DEBUG_PKT
void spu_dump_array(char *msg, unsigned char *buf, uint16 len)
{
    int i;
    unsigned char *ptmp;
    printk ("%s Buf Addr %p Len %d ****\n", msg, buf, len);
    ptmp = buf;
    for (i = 0; i < len; i++)
    {
       printk ("%02x ", *(ptmp + i));
       if (!((i + 1) % 16))
           printk ("\n");
    }
    printk ("\n");

    return;
}
#endif

/*
 * process completed requests for channels that have done status
 */
static void spu_done (unsigned long data)
{
    unsigned long irq_flags;
    uint32 address;
    int numbds = 0;

    spin_lock_irqsave (&pdev_ctrl->spin_lock, irq_flags);
    while( pdev_ctrl->rx_free_bds < NR_RX_BDS )
    {
        if (pdev_ctrl->rx_bds[pdev_ctrl->rx_head].status & DMA_OWN)
        {
            break;
        }

        if(pdev_ctrl->rx_bds[pdev_ctrl->rx_head].status & DMA_SOP)
        {
            address = pdev_ctrl->rx_bds[pdev_ctrl->rx_head].address;
        }
        else
        {
            address = 0;
        }

        pdev_ctrl->rx_bds[pdev_ctrl->rx_head].address = 0;
        pdev_ctrl->rx_bds[pdev_ctrl->rx_head].length = 0;
        pdev_ctrl->rx_bds[pdev_ctrl->rx_head].status = 0;
        pdev_ctrl->rx_head++;
        pdev_ctrl->rx_free_bds++;
        if ( pdev_ctrl->rx_head == NR_RX_BDS )
        {
            pdev_ctrl->rx_head = 0;
        }
        spin_unlock_irqrestore (&pdev_ctrl->spin_lock, irq_flags);
        if ( address )
        {
#ifdef CONFIG_BCM_SPU_TEST
            numbds = spu_finish_processing((uint8_t *)address, pdev_ctrl->test_mode);
#else
            numbds = spu_finish_processing((uint8_t *)address);
#endif
        }
        spin_lock_irqsave (&pdev_ctrl->spin_lock, irq_flags);
        spu_reclaim_tx_descriptors( numbds );
    }
    spin_unlock_irqrestore (&pdev_ctrl->spin_lock, irq_flags);
} /* spu_done */

static void spu_reclaim_tx_descriptors( int numbds )
{
    int cnt = 0;
    while ((pdev_ctrl->tx_free_bds < NR_XMIT_BDS) && (cnt < numbds) )
    {
        /* if DMA_OWN bit is set there are no more
           descriptors to reclaim */
        if (pdev_ctrl->tx_bds[pdev_ctrl->tx_head].status & DMA_OWN)
        {
            break;
        }

        pdev_ctrl->tx_head++;
        if (NR_XMIT_BDS == pdev_ctrl->tx_head)
        {
            pdev_ctrl->tx_head = 0;
        }

        pdev_ctrl->tx_free_bds++;
        cnt++;
    }
}

void spu_assign_output_desc (unsigned char *buf, uint16 len, uint16 flags)
{
    SPU_TRACE (("IPSEC SPU: buf %p len %d\n", buf, len));

    /*
    * Invalidate the cache.
    */
    cache_flush_len(buf, len);

    pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].address = (uint32) VIRT_TO_PHYS (buf);
    pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].length = len;
    SPU_TRACE (("IPSEC SPU: Indx %d BD Addr %p Buf Addr %lx Len %x\n",
                 pdev_ctrl->rx_tail,
                 &(pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].address),
                 pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].address,
                 pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].length));

    if (pdev_ctrl->rx_tail == (NR_RX_BDS - 1)) {
        flags |= DMA_WRAP;
    }
    pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].status = flags;
    SPU_TRACE(("IPSEC SPU: ** Rx BD %p addr %lx len %x sts %x tail %d\n",
               &pdev_ctrl->rx_bds[pdev_ctrl->rx_tail],
               pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].address,
               pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].length,
               pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].status,
               pdev_ctrl->rx_tail));
    pdev_ctrl->rx_tail++;
    pdev_ctrl->rx_free_bds--;
    if ( pdev_ctrl->rx_tail == NR_RX_BDS ) {
        pdev_ctrl->rx_tail = 0;
    }
} /* spu_assign_output_desc */

void spu_assign_input_desc (unsigned char *buf, uint16 len, uint16 flags)
{
    SPU_TRACE(("IPSEC SPU: Setting Up Tx BD index %d "
                     "phy addr 0x%lx non-cache 0x%x flags %x\n",
                      pdev_ctrl->tx_tail, (uint32) VIRT_TO_PHYS (buf),
                      CACHE_TO_NONCACHE (buf), flags));
    cache_flush_len(buf, len + BUF_ALIGN);
    pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].address = (uint32) VIRT_TO_PHYS (buf);
    pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].length = len;

    if (pdev_ctrl->tx_tail == (NR_XMIT_BDS - 1)) {
        flags |= DMA_WRAP;
    }
    pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].status = flags;
    SPU_TRACE(("IPSEC SPU: Assigned Tx BD index %d %p addr %lx len %x sts %x\n",
               pdev_ctrl->tx_tail,
               &pdev_ctrl->tx_bds[pdev_ctrl->tx_tail],
               pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].address,
               pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].length,
               pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].status));
    pdev_ctrl->tx_tail++;
    pdev_ctrl->tx_free_bds--;
    if ( pdev_ctrl->tx_tail == NR_XMIT_BDS ) {
        pdev_ctrl->tx_tail = 0;
    }
} /* spu_assign_input_desc */

/***************************************************************************
 * Function Name: spu_init
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 ***************************************************************************/
static int __init spu_init (void)
{
    int err;
    dev_t dev = 0;
    dev_t devno;

    SPU_TRACE (("IPSEC SPU: spu_init entry\n"));

#ifdef MISC_IDDQ_CTRL_IPSEC
    MISC->miscIddqCtrl &= ~MISC_IDDQ_CTRL_IPSEC;
#endif

#ifdef IPSEC_CLK_EN
    PERF->blkEnables |= IPSEC_CLK_EN;
#endif

#ifdef SOFT_RST_IPSEC
    PERF->softResetB &= ~SOFT_RST_IPSEC;
    udelay(100);
    PERF->softResetB |= SOFT_RST_IPSEC;
#endif

    spudevice = kzalloc(sizeof(struct spu_device), GFP_KERNEL);
    if (NULL == spudevice) {
        printk("bcm_spu_init: Insufficient memory for context data\n");
        err = -ENOMEM;
        goto fail;
    }

    /* Get a range of minor numbers (starting with 0) to work with */
    err = alloc_chrdev_region(&dev, 0, SPU_NUM_DEVICES, SPU_DEVICE_NAME);
    if (err < 0) {
        printk(KERN_WARNING "bcm_spu_init: alloc_chrdev_region() failed\n");
        return err;
    }
    spudevice->major = MAJOR(dev);

    /* create device class */
    spudevice->spu_class = class_create(THIS_MODULE, SPU_DEVICE_NAME);
    if (IS_ERR(spudevice->spu_class)) {
        err = PTR_ERR(spudevice->spu_class);
        goto fail;
    }

    devno = MKDEV(spudevice->major, 0);
    spudevice->spu_file_ops.unlocked_ioctl = spu_ioctl_unlocked;
#if defined(CONFIG_COMPAT)
    spudevice->spu_file_ops.compat_ioctl = spu_ioctl_unlocked,
#endif
    spudevice->spu_file_ops.open = spu_open;
    spudevice->spu_file_ops.release = spu_release;

    memset(&spudevice->cdev, 0, sizeof(spudevice->cdev));
    cdev_init(&spudevice->cdev, &spudevice->spu_file_ops);
    spudevice->cdev.owner = THIS_MODULE;

    err = cdev_add(&spudevice->cdev, devno, 1);
    if (err)
    {
        printk(KERN_WARNING "bcm_spu_init: Error %d while trying to add %s%d",
               err, SPU_DEVICE_NAME, 0);
        goto fail;
    }
    
    spudevice->device = device_create(spudevice->spu_class, NULL, 
                                      devno, NULL, SPU_DEVICE_NAME "%d", 0);
    if (IS_ERR(spudevice->device))
    {
        err = PTR_ERR(spudevice->device);
        printk(KERN_WARNING "bcm_spu_init: Error %d while trying to create %s%d",
                    err, SPU_DEVICE_NAME, 0);
        cdev_del(&spudevice->cdev);
        goto fail;
    }

    return 0;

fail:
    spu_cleanup();
    return err;
    
} /* spu_init */


/***************************************************************************
 * Function Name: spu_cleanup
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void spu_cleanup (void)
{
   SPU_TRACE (("IPSEC SPU: spu_cleanup\n"));
   if ( spudevice )
   {
      if ( !IS_ERR(spudevice->device) )
      {
         device_destroy(spudevice->spu_class, MKDEV(spudevice->major, 0));
         cdev_del(&spudevice->cdev);
      }
      
      if (!IS_ERR(spudevice->spu_class))
      {
         class_destroy(spudevice->spu_class);
      }

      if ( spudevice->major )
      {
         unregister_chrdev_region(MKDEV(spudevice->major, 0), SPU_NUM_DEVICES);
      }
   
      kfree(spudevice);
      spudevice = NULL;
   }

   return;
} /* spu_cleanup */

/***************************************************************************
 * Function Name: spu_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int spu_open (struct inode *inode, struct file *filp)
{

    SPU_TRACE (("IPSEC SPU: spu_open entry\n"));
    return (0);

} /* spu_open */

/***************************************************************************
 * Function Name: spu_alloc_dev
 * Description  : Called to allocate device control data structure
 * Returns      : Pointer to spu_dev_ctrl_t structure.
 ***************************************************************************/
static pspu_dev_ctrl_t spu_alloc_dev (void)
{
    int alloc_size;
    void *p;

    /*
     * Align it to 32 bits boundary
     */
    alloc_size = (sizeof (struct spu_dev_ctrl_s) + DEV_ALIGN_CONST)
                                                 & ~(DEV_ALIGN_CONST);
    p = kmalloc (alloc_size, GFP_KERNEL);
    if (!p)
    {
        printk (KERN_ERR "alloc_dev: Unable to allocate device.\n");
        return NULL;
    }

    memset (p, 0, alloc_size);

    return ((pspu_dev_ctrl_t) p);
} /* spu_alloc_dev */

/***************************************************************************
 * Function Name: spu_init_dev
 * Description  : Called to initialize device control data structure
 * Returns      : 0 - success.
 ***************************************************************************/
static int spu_init_dev (void)
{
    uint32 size;

    SPU_TRACE (("IPSEc SPU: Device structre initialization %p\n", pdev_ctrl));

    if (!pdev_ctrl)
    {
        return -1;
    }

    /*
     * Device initialization
     */
    pdev_ctrl->dma_ctrl = (DmaRegs *) (IPSEC_DMA_BASE);
    pdev_ctrl->rx_irq = INTERRUPT_ID_IPSEC_DMA_0;

    pdev_ctrl->rx_dma = &pdev_ctrl->dma_ctrl->chcfg[0];
    pdev_ctrl->tx_dma = &pdev_ctrl->dma_ctrl->chcfg[1];

    /*
     * Disable interrupts until we setup DMA.
     */
    BcmHalInterruptDisable (pdev_ctrl->rx_irq);

    /*
     * Register the interrupt service handler
     */
    BcmHalMapInterrupt ((FN_HANDLER)spu_isr, 
                        (void*) pdev_ctrl, 
                        pdev_ctrl->rx_irq);

    /* allocate TX descriptors */
    size = (NR_XMIT_BDS * sizeof(DmaDesc)) + DESC_ALIGN;
    if (!(pdev_ctrl->txBdsBase = kmalloc(size, GFP_KERNEL))) {
        printk ("IPSEC SPU: No memory for Tx BDs\n");
        return -ENOMEM;
    }
    memset(pdev_ctrl->txBdsBase, 0, size);
    cache_flush_len(pdev_ctrl->txBdsBase, size);
    pdev_ctrl->tx_bds = (DmaDesc *)IPSEC_SPU_ALIGN(CACHE_TO_NONCACHE(pdev_ctrl->txBdsBase), DESC_ALIGN);

    /* allocate RX descriptors */
    size = (NR_RX_BDS * sizeof(DmaDesc)) + DESC_ALIGN;
    if (!(pdev_ctrl->rxBdsBase = kmalloc(size, GFP_KERNEL))) {
        printk ("IPSEC SPU: No memory for Rx BDs\n");
        return -ENOMEM;
    }
    memset(pdev_ctrl->rxBdsBase, 0, size);
    cache_flush_len(pdev_ctrl->rxBdsBase, size);
    pdev_ctrl->rx_bds = (DmaDesc *)IPSEC_SPU_ALIGN(CACHE_TO_NONCACHE(pdev_ctrl->rxBdsBase), DESC_ALIGN);

    SPU_TRACE(("IPSEC SPU: Tx BD Ring %p Rx BD Ring %p\n",
                      pdev_ctrl->tx_bds, pdev_ctrl->rx_bds));

    pdev_ctrl->rx_free_bds = NR_RX_BDS;
    pdev_ctrl->rx_tail = 0;
    pdev_ctrl->rx_head = 0;

    pdev_ctrl->tx_free_bds = NR_XMIT_BDS;
    pdev_ctrl->tx_tail = 0;
    pdev_ctrl->tx_head = 0;

#ifdef CONFIG_BCM_SPU_TEST
    pdev_ctrl->test_mode = 0;
#endif

    return 0;
} /* spu_init_dev */

/***************************************************************************
 * Function Name: spu_init_dma
 * Description  : Called to initialize IPSec DMA
 * Returns      : N/A.
 ***************************************************************************/
static void spu_init_dma (void)
{

    SPU_TRACE (("IPSEC SPU: Initializing DMA %p\n", pdev_ctrl));

    if (!pdev_ctrl)
    {
        return;
    }

    /*
     * clear State RAM
     */
    memset ((char *) &pdev_ctrl->dma_ctrl->stram.s[0],
                                          0x00, sizeof (DmaStateRam) * 2);
    SPU_TRACE(("IPSEC SPU: dma_ctrl %p  Tx dma %p\n",
                          pdev_ctrl->dma_ctrl,  pdev_ctrl->tx_dma));

    /* setup Tx dma register */
    pdev_ctrl->tx_dma->cfg = 0;
    pdev_ctrl->tx_dma->maxBurst = DMA_MAX_BURST_LENGTH;
    pdev_ctrl->tx_dma->intMask = 0; /* mask all ints */

    SPU_TRACE(("IPSEC SPU: Tx BDs %p\n", pdev_ctrl->tx_bds));

    pdev_ctrl->dma_ctrl->stram.s[1].baseDescPtr = 
                                      (uint32) VIRT_TO_PHYS (pdev_ctrl->tx_bds);

    SPU_TRACE(("IPSEC SPU: Tx DMA initialized baseDescPtr %lx Rx dma %p\n",
               pdev_ctrl->dma_ctrl->stram.s[1].baseDescPtr, pdev_ctrl->rx_dma));

    /* setup Rx dma register */
    pdev_ctrl->rx_dma->cfg = 0;
    pdev_ctrl->rx_dma->maxBurst = DMA_MAX_BURST_LENGTH;
    pdev_ctrl->rx_dma->intMask = 0; /* mask all ints */

    /* clr any pending interrupts on channel */
    pdev_ctrl->rx_dma->intStat = DMA_DONE;

    /* set to interrupt on packet complete and no descriptor available */
    pdev_ctrl->rx_dma->intMask = DMA_DONE;

    SPU_TRACE(("IPSEC SPU: Rx BDs %p\n", pdev_ctrl->rx_bds));

    pdev_ctrl->dma_ctrl->stram.s[0].baseDescPtr =
                                      (uint32) VIRT_TO_PHYS (pdev_ctrl->rx_bds);

    SPU_TRACE(("IPSEC SPU: Rx DMA initialized baseDescPtr %lx\n",
                                   pdev_ctrl->dma_ctrl->stram.s[0].baseDescPtr));

    /*
     * Enable DMA controller
     */
    pdev_ctrl->dma_ctrl->controller_cfg |= DMA_MASTER_EN;
    
}/* spu_init_dma */

/***************************************************************************
 * Function Name: spu_deinit_dev
 * Description  : Called to initialize device control data structure
 * Returns      : 0 - success.
 ***************************************************************************/
static int spu_deinit_dev (void)
{
    SPU_TRACE (("IPSEc SPU: spu_deinit_dev %p\n", pdev_ctrl));
    if (!pdev_ctrl)
    {
        return -1;
    }

    /* Unregister the interrupt service handler */
    free_irq(pdev_ctrl->rx_irq, pdev_ctrl);

    /* free the RX and TX BDS */
    kfree((const void *)(pdev_ctrl->txBdsBase));
    kfree((const void *)(pdev_ctrl->rxBdsBase));

    return 0;
} /* spu_init_dev */

/***************************************************************************
 * Function Name: spu_deinit_dma
 * Description  : Called to initialize IPSec DMA
 * Returns      : N/A.
 ***************************************************************************/
static void spu_deinit_dma (void)
{
    SPU_TRACE (("IPSEC SPU: Disabling DMA %p\n", pdev_ctrl));
    if (!pdev_ctrl)
    {
        return;
    }

    /* Disable DMA controller */
    pdev_ctrl->dma_ctrl->controller_cfg &= ~DMA_MASTER_EN;    

    SPU_TRACE(("IPSEC SPU: clear TX DMA\n"));

    /* setup Tx dma register */
    pdev_ctrl->tx_dma->cfg = 0;
    pdev_ctrl->tx_dma->maxBurst = DMA_MAX_BURST_LENGTH;
    pdev_ctrl->tx_dma->intMask = 0;   /* mask all ints */

    /* clear base descriptor */
    pdev_ctrl->dma_ctrl->stram.s[1].baseDescPtr = 0x00000000;

    SPU_TRACE(("IPSEC SPU: clear RX DMA\n"))

    /* setup Rx dma register */
    pdev_ctrl->rx_dma->cfg = 0;
    pdev_ctrl->rx_dma->maxBurst = DMA_MAX_BURST_LENGTH;
    pdev_ctrl->rx_dma->intMask = 0;   /* mask all ints */

     pdev_ctrl->dma_ctrl->stram.s[0].baseDescPtr = 0x00000000;

    /* clear State RAM */
    memset ((char *)&pdev_ctrl->dma_ctrl->stram.s[0],
            0x00, sizeof (DmaStateRam) * 2);

}/* spu_deinit_dma */


/***************************************************************************
 * Function Name: spu_ioctl
 * Description  : Entry point for application commands.
 * Returns      : 0 - success.
 ***************************************************************************/
static int spu_ioctl (struct inode *inode, struct file *filp,
                      unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    unsigned int cmdnr = _IOC_NR (cmd);

    IPSec_SPU_FN_IOCTL  IoctlFuncs[] = { do_spudd_initialize,
                                         do_spudd_uninitialize,
                                         do_spu_show,
                                         do_spu_test };

    if (cmdnr < MAX_SPUDDDRV_IOCTL_COMMANDS)
    {
        SPU_TRACE (("IPSEC SPU: Ioctl cmd %u\n", cmdnr));
        (*IoctlFuncs[cmdnr]) (arg);
    }
    else
        ret = -EINVAL;

    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(spuIoctlMutex);

static long spu_ioctl_unlocked(struct file *filep, unsigned int cmd, 
                               unsigned long arg)
{
    struct inode *inode;
    long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    inode = filep->f_dentry->d_inode;
#else
   inode = file_inode(filep);
#endif

    mutex_lock(&spuIoctlMutex);
    rt = spu_ioctl(inode, filep, cmd, arg);
    mutex_unlock(&spuIoctlMutex);

    return rt;
}
#endif


/***************************************************************************
 * Function Name: spu_release
 * Description  : Called when an application closes this device.
 * Returns      : 0 - success.
 ***************************************************************************/
static int spu_release (struct inode *inode, struct file *filp)
{
  return (0);
} /* spu_release */

/***************************************************************************
 * Function Name: spu_isr
 * Description  : Interrupt service routine.
 * Returns      : 0 - success.
 ***************************************************************************/
static irqreturn_t spu_isr (int irq, void *dev_id, struct pt_regs *regs)
{
    pspu_dev_ctrl_t pdev_ctrl = dev_id;
    uint32 intStat = pdev_ctrl->rx_dma->intStat;

    SPU_TRACE(("ISR %lx\n", pdev_ctrl->rx_dma->intStat));

    if ((intStat & DMA_DONE) != 0)
    {
        pdev_ctrl->rx_dma->intStat = DMA_DONE;

        /* task will unmask done interrupts at exit */
        tasklet_schedule (&pdev_ctrl->task);
    }

    return IRQ_HANDLED;
} /* spu_isr */

unsigned long spu_get_cycle_count(void)
{

    unsigned long cnt;
    __asm volatile ("mfc0 %0, $9":"=d" (cnt));

    return (cnt);
} /* spu_get_cycle_count */

void spu_get_time(unsigned long *osTime)
{
    *osTime = jiffies;
    return;
} /* spu_get_time */

/***************************************************************************
 * Function Name: spu_calibrate_cycles_per_us
 * Description  : Return clock cycles per microseconds.
 * Returns      : N/A
 ***************************************************************************/
ulong spu_calibrate_cycles_per_us (void)
{
    unsigned long tick0, tick1;
    unsigned long cnt;

    if (spu_cycle_per_us != 0)
        return spu_cycle_per_us;

    spu_get_time(&tick1);

    do
    {
        spu_get_time(&tick0);
    }
    while (tick0 == tick1);

    cnt = spu_get_cycle_count();

    do
    {
        spu_get_time(&tick1);
        tick1 = (tick1 - tick0) * IPSEC_SPU_MSEC_PER_TICK;
    }
    while (tick1 < 60);

    cnt = spu_get_cycle_count() - cnt;
    spu_cycle_per_us = cnt / tick1;
    return (spu_cycle_per_us / 1000);
} /* spu_calibrate_cycles_per_us */


/***************************************************************************
 * Function Name: do_spudd_initialize
 * Description  : Initialize the IPSEC SPU and associated IUDMA.
 * Returns      : N/A
 ***************************************************************************/
static void do_spudd_initialize (unsigned long arg)
{
    SPUDDDRV_INITIALIZE      KArg;

    SPU_TRACE (("IPSEC SPU: do_spudd_initialize\n"));

    KArg.bvStatus = SPUSTS_SUCCESS;
    do
    {
       if (NULL != pdev_ctrl)
           break;

       SPU_TRACE (("IPSEC SPU: Allocating device control data\n"));

       /*
        * Allocate device control structure.
        */
       if ((pdev_ctrl = spu_alloc_dev ()) == NULL)
       {
           printk (KERN_ERR "IPSEC SPU: No memory for device control\n");
           KArg.bvStatus = SPUSTS_MEMERR;
           break;
       }

       SPU_TRACE (("IPSEC SPU: Initializing device control data\n"));

       /*
        * Initialize the device control data structure.
        */
       if (spu_init_dev ())
       {
           printk ((KERN_ERR "IPSEC SPU: device initialization error!\n"));
           KArg.bvStatus = SPUSTS_ERROR;
           break;
       }

       SPU_TRACE (("IPSEC SPU: Initializing DMA\n"));

       /*
        * Initialize DMA
        */
       spu_init_dma ();

       spin_lock_init(&pdev_ctrl->spin_lock);

       /*
        * Calibrate clock cycles per us.
        */
       spu_cycle_per_us = spu_calibrate_cycles_per_us ();

       SPU_TRACE (("Calibration done CPU cycles per us %lx\n", 
                                             spu_cycle_per_us));
       /*
        * Initialize tasklet
        */
       tasklet_init (&pdev_ctrl->task, spu_done, (unsigned long) NULL);

       SPU_TRACE (("IPSEC SPU: SPU initialization successful\n"));

       bcm_crypto_dev_init();
    } while ( 0 );

    if ( SPUSTS_SUCCESS != KArg.bvStatus )
    {
        spu_cleanup();
    }

    put_user (KArg.bvStatus, &((SPUDDDRV_INITIALIZE *)arg)->bvStatus);

    return;
} /* do_spudd_initialize */

/***************************************************************************
 * Function Name: do_spudd_uninitialize
 * Description  : shutdown crypto interface.
 * Returns      : N/A
 ***************************************************************************/
static void do_spudd_uninitialize (unsigned long arg)
{
    unsigned long irq_flags;
    int err;
    SPUDDDRV_INITIALIZE KArg;

    KArg.bvStatus = SPUSTS_SUCCESS;
    do
    {
       SPU_TRACE (("IPSEC SPU: do_spudd_uninitialize\n"));
       if(NULL == pdev_ctrl)
           break;

       /* unregister algorithms from linux, after this there 
          shouldn't be any requests coming in */
       err = bcm_crypto_dev_disable();
       if ( err )
       {
           /* there are still references held to the SPU so abort */
           KArg.bvStatus = SPUSTS_ERROR;
           break;
       }

       /* wait for HW to finish */
       spin_lock_irqsave(&pdev_ctrl->spin_lock, irq_flags);
       while( pdev_ctrl->rx_free_bds < NR_RX_BDS )
       {
           spin_unlock_irqrestore (&pdev_ctrl->spin_lock, irq_flags);
           msleep(1);
           spin_lock_irqsave(&pdev_ctrl->spin_lock, irq_flags);
       }
       spin_unlock_irqrestore (&pdev_ctrl->spin_lock, irq_flags);

       tasklet_kill(&pdev_ctrl->task);

       /* disable DMA controller */
       spu_deinit_dma();

       /* release dev structure data */
       spu_deinit_dev();

       kfree(pdev_ctrl);
       pdev_ctrl = NULL;
    } while ( 0 );

    return;
} /* do_spudd_uninitialize */


/***************************************************************************
 * Function Name: do_spu_test
 * Description  : perform spu tests
 *                
 * Returns      : N/A
 ***************************************************************************/
static void do_spu_test(unsigned long arg)
{
#ifdef CONFIG_BCM_SPU_TEST
    SPUDDDRV_TEST KArg;
#endif /* CONFIG_BCM_SPU_TEST */

    SPU_TRACE (("IPSEC SPU: do_spu_test\n"));

    if(!pdev_ctrl)
        return;

#ifdef CONFIG_BCM_SPU_TEST
    if (copy_from_user (&KArg, (void *) arg, sizeof (KArg)) == 0)
    {
        pdev_ctrl->test_mode = 1;
        spu_perform_test(KArg.testParams.pktId, KArg.testParams.numPkts);
        pdev_ctrl->test_mode = 0;
    }
#endif

    return;
} /* do_spu_test */

/***************************************************************************
 * Function Name: do_spu_show
 * Description  : show statistics
 *                
 * Returns      : N/A
 ***************************************************************************/
static void do_spu_show (unsigned long arg)
{
    SPUDDDRV_SPU_SHOW KArg;

    SPU_TRACE (("IPSEC SPU: do_spu_show\n"));

    memset(&KArg, 0, sizeof(KArg));
    if(pdev_ctrl)
    {
        KArg.bvStatus = SPUSTS_SUCCESS;
        memcpy(&KArg.stats, &pdev_ctrl->stats, sizeof(SPU_STAT_PARMS));
    }
    else
    {
        KArg.bvStatus = SPUSTS_ERROR;
    }
    
    if ( 0 != copy_to_user((void *)arg, &KArg, sizeof(SPUDDDRV_SPU_SHOW)) )
    {
        SPU_TRACE(("do_spu_show - error copying data to user\n"));
    }
    return;
} /* do_spu_show */

module_init (spu_init);
module_exit (spu_cleanup);
EXPORT_SYMBOL (spu_get_cycle_count);
MODULE_LICENSE ("GPL");
