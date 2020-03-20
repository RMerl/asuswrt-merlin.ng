/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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
/***************************************************************************
 * File Name  : sputest.c
 *
 * Description: This file contains SPU test code
 *
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
#include "bcm_intr.h"
#include "bcmspudrv.h"
#include "../spudrv.h"
#include "spudrv_data.h"
#include <board.h>
#include <linux/hw_random.h>
#include "../spu.h"

#if defined(CONFIG_BCM_SPU_TEST)

/* Prototypes. */
static int encrypt_decrypt_verify (uint16 test_pkt_id, uint16 rx_index);
unsigned long spu_get_cycle_count(void);
void spu_assign_output_desc (unsigned char *buf, uint16 len);
extern void spu_dump_array(char *msg, unsigned char *buf, uint16 len);

/*
 * Performance variables
 */
extern unsigned long start;
extern unsigned long end;
extern unsigned long proc_time;
int xmit_pkt_len = 0;
int recv_pkt_len = 0;

/* DMA buffers */
unsigned char *tx_data = NULL;
unsigned char *rx_data = NULL;
unsigned char *tx_hdr = NULL;
extern unsigned long spu_cycle_per_us;

/* Device control structure */
extern pspu_dev_ctrl_t pdev_ctrl;

int num_tests_passed = 0;
int num_tests_failed = 0;


/***************************************************************************
 * Function Name: ipsec_setup_tx_rx
 * Description  : Setup Tx and Rx buffers for test.
 * Returns      : rx_index
 ***************************************************************************/
static int ipsec_setup_tx_rx (uint32 test_pkt_id, int *done)
{
    int i = 0;
    unsigned char *p;
    unsigned char *p8;
    unsigned char *palign;
    unsigned char *ptx;
    unsigned char *ptx_tmpl;
    unsigned char *tx_pkt;
    uint16 rx_index;
    uint16 tx_pkt_size = tx_pkt_len[test_pkt_id];
    uint16 rx_pkt_size = rx_pkt_len[test_pkt_id];
    unsigned long irq_flags;

    /*
     * Setup the Rx Buffer first
     */
    if ((p = kmalloc ((rx_pkt_size + BUF_ALIGN), GFP_KERNEL)) == NULL)
    {
	printk (KERN_ERR "IPSEC SPU: Error no memory for Rx buffer\n");
	*done = 0;
	return -ENOMEM;
    }

    rx_data = p;
    rx_index = pdev_ctrl->rx_tail;
    memset (p, 0, rx_pkt_size);
    cache_flush_len(p, rx_pkt_size + BUF_ALIGN);
    p8 = (unsigned char *) IPSEC_SPU_ALIGN (p, BUF_ALIGN);

    spin_lock_irqsave (&pdev_ctrl->spin_lock, irq_flags);
    pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].address = (uint32) VIRT_TO_PHYS (p8);
    pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].length = rx_pkt_size;
    recv_pkt_len = rx_pkt_size;

    if (pdev_ctrl->rx_tail == (NR_RX_BDS - 1))
    {
	pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].status = DMA_OWN | DMA_WRAP;
#ifdef SPU_DEBUG
	printk (KERN_ERR "IPSEC SPU: Rx BD %p addr %lx len %x sts %x\n",
	      &pdev_ctrl->rx_bds[pdev_ctrl->rx_tail],
	      pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].address,
	      pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].length,
	      pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].status);
#endif
	pdev_ctrl->rx_tail = 0;
    }
    else
    {
	pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].status = DMA_OWN;
#ifdef SPU_DEBUG
	printk (KERN_ERR "IPSEC SPU: ** Rx BD %p addr %lx len %x sts %x\n",
	      &pdev_ctrl->rx_bds[pdev_ctrl->rx_tail],
	      pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].address,
	      pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].length,
	      pdev_ctrl->rx_bds[pdev_ctrl->rx_tail].status);
#endif
	pdev_ctrl->rx_tail++;
    }
    spin_unlock_irqrestore(&pdev_ctrl->spin_lock, irq_flags);

    /*
     * Now setup the Tx buffer.
     */
    if ((tx_pkt = kmalloc (tx_pkt_size + BUF_ALIGN, GFP_KERNEL)) == NULL)
    {
	printk (KERN_ERR "IPSEC SPU: Error INPUT PKT OUT OF MEMORY\n");
	*done = 0;
	return -ENOMEM;
    }

    tx_data = tx_pkt;
    p8 = (unsigned char *) IPSEC_SPU_ALIGN (tx_pkt, BUF_ALIGN);
    p = (uint8 *) CACHE_TO_NONCACHE ((uint32) p8);
    memset (p, 0, tx_pkt_size);
    ptx_tmpl = (unsigned char *) tx_test_pkts[test_pkt_id];
    ptx = (unsigned char *) p;

#ifdef SPU_DEBUG
    printk (KERN_ERR "IPSEC SPU: tx_data %p p %p\n", ptx, p);
#endif

    while (i < tx_pkt_size)
    {
	*ptx = *ptx_tmpl;
	ptx++;
	ptx_tmpl++;
	i++;
    }

    palign = (unsigned char *) IPSEC_SPU_ALIGN (tx_pkt, BUF_ALIGN);

#ifdef SPU_DEBUG
    printk (KERN_ERR
	"IPSEC SPU: Setting Up Tx BD tx_pkt %p p %p phy addr 0x%lx\n", ptx,
	  p, (uint32) VIRT_TO_PHYS (palign));
#endif

    spin_lock_irqsave(&pdev_ctrl->spin_lock, irq_flags);
    pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].address =
                                        (uint32) VIRT_TO_PHYS (palign);
    pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].length = tx_pkt_size;
    xmit_pkt_len = tx_pkt_size;
    if (pdev_ctrl->tx_tail == (NR_XMIT_BDS - 1))
    {
	pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].status =
                                 DMA_OWN | DMA_SOP | DMA_EOP | DMA_WRAP;
#ifdef SPU_DEBUG
	printk (KERN_ERR "IPSEC SPU: Tx BD %p addr %lx len %x sts %x\n",
	      &pdev_ctrl->tx_bds[pdev_ctrl->tx_tail],
	      pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].address,
	      pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].length,
	      pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].status);
#endif
	pdev_ctrl->tx_tail = 0;
	pdev_ctrl->tx_free_bds = NR_XMIT_BDS;
    }
    else
    {
	pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].status =
                                         DMA_OWN | DMA_SOP | DMA_EOP;
#ifdef SPU_DEBUG
	printk (KERN_ERR "IPSEC SPU: ** Tx BD %p addr %lx len %x sts %x\n",
	      &pdev_ctrl->tx_bds[pdev_ctrl->tx_tail],
	      pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].address,
	      pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].length,
	      pdev_ctrl->tx_bds[pdev_ctrl->tx_tail].status);
#endif
	pdev_ctrl->tx_free_bds--;
	pdev_ctrl->tx_tail++;
    }
    spin_unlock_irqrestore(&pdev_ctrl->spin_lock, irq_flags);
    return rx_index;
}

/***************************************************************************
 * Function Name: encrypt_decrypt_verify
 * Description  : Verify the result of encryption or decryption operation.
 * Returns      : 0 success
 ***************************************************************************/
static int encrypt_decrypt_verify (uint16 test_pkt_id, uint16 rx_index)
{
    int ret = 1;
    //unsigned char *p;
    unsigned char *ptx = NULL;
    unsigned char *prx = NULL;
    unsigned char *ptmpl = NULL;
    unsigned char *rxdata = NULL;
    //unsigned char *tx_data = tx_test_pkts[test_pkt_id];
    unsigned char *rx_tmpl = rx_templates[test_pkt_id];
    uint16 rx_len = pdev_ctrl->rx_bds[rx_index].length;
#ifdef SPU_DEBUG
    int i = 0;
    uint16 tx_len = tx_pkt_len[test_pkt_id];
#endif
    uint16 dma_status = pdev_ctrl->rx_bds[rx_index].status;

    if (dma_status & DMA_OWN)
    {
	printk (KERN_ERR "IPSEC SPU: Nothing to process\n");
	goto clean_up;
	return ret;
    }

    rxdata = (unsigned char *) pdev_ctrl->rx_bds[rx_index].address;
    prx = (unsigned char *) IPSEC_SPU_ALIGN (rxdata, BUF_ALIGN);

#ifdef SPU_DEBUG
    printk (KERN_ERR "IPSEC SPU: Rx Buffer pres addr %p rx addr %p\n",
	  prx, rxdata);
#endif

    prx = (uint8 *) CACHE_TO_NONCACHE (prx);

#ifdef SPU_DEBUG
    printk (KERN_ERR "IPSEC SPU: Rx Buffer pres addr %p rx addr %p\n",
	  prx, rxdata);
#endif

    ptmpl = (unsigned char *) IPSEC_SPU_ALIGN (rx_tmpl, BUF_ALIGN);

    ptx = (unsigned char *) IPSEC_SPU_ALIGN (tx_data, BUF_ALIGN);
    ptx = (uint8 *) CACHE_TO_NONCACHE (ptx);

    if (memcmp (prx, rx_tmpl, rx_len) == 0)
    {
        num_tests_passed++;
        printk (KERN_ERR "IPSEC SPU: Packet [%d] Test Passed Tx Len %d "
	      "Rx Len %d Time %lx\n", test_pkt_id, xmit_pkt_len,
	      recv_pkt_len, proc_time);
    }
    else
    {
        num_tests_failed++;
        printk (KERN_ERR "IPSEC SPU: Packet [%d] Test Failed Tx Len %d "
	      "Rx Len %d Time %lx\n", test_pkt_id, xmit_pkt_len,
	      recv_pkt_len, proc_time);

#ifdef SPU_DEBUG
	for (i = 0; i < tx_len; i += 4)
	{
	    printk ("Tx Pkt %p 0x%02x%02x%02x%02x\n",
		  (ptx + i),
		  *(ptx + i), *(ptx + i + 1), *(ptx + i + 2), *(ptx + i + 3));

	}

	for (i = 0; i < rx_len; i += 4)
	{
	    printk
	    ("Rx Pkt %p 0x%02x%02x%02x%02x \t Rx Exp %p 0x%02x%02x%02x%02x\n",
	     (prx + i), *(prx + i), *(prx + i + 1), *(prx + i + 2),
	     *(prx + i + 3), (ptmpl + i), *(ptmpl + i), *(ptmpl + i + 1),
	     *(ptmpl + i + 2), *(ptmpl + i + 3));
	}
#endif

    }

    ret = 0;

clean_up:

    /*
     * Clean up the buffers allocated for tx and rx
     * before leaving.
     */
    kfree (rx_data);
    kfree (tx_data);
    kfree (tx_hdr);

    return ret;
}

static int dump_output (uint16 rx_index)
{
    int ret = 1;
#ifdef SPU_DEBUG
    int i = 0;
#endif
    unsigned char *rxdata = NULL;
    unsigned char *prx;
    uint16 dma_status = pdev_ctrl->rx_bds[rx_index].status;

    if (dma_status & DMA_OWN)
    {
	printk (KERN_ERR "IPSEC SPU: Nothing to process\n");
	return ret;
    }

    rxdata = (unsigned char *) pdev_ctrl->rx_bds[rx_index].address;
    //prx = (unsigned char *)IPSEC_SPU_ALIGN(rxdata, BUF_ALIGN);
    prx = (uint8 *) CACHE_TO_NONCACHE (rxdata);

#ifdef SPU_DEBUG
    printk ("BD len %d status %0x address %lx\n",
	  pdev_ctrl->rx_bds[rx_index].length,
	  pdev_ctrl->rx_bds[rx_index].status,
	  pdev_ctrl->rx_bds[rx_index].address);

    printk ("********** Received Data **********\n");

    for (i = 0; i < pdev_ctrl->rx_bds[rx_index].length; i++)
    {
	printk ("%02x ", *(prx + i));
	if (!((i + 1) % 4))
	{
	    printk ("\n");
	}
    }
#endif

    return 0;
}

/***************************************************************************
 * Function Name: spu_perform_test
 * Description  : Tests for the hardware to do encrypt
 *                or decrypt operation 
 * Returns      : N/A
 ***************************************************************************/
void spu_perform_test(uint32 tx_pkt_id, uint32 num_pkts)
{
    int ntries = 0;
#ifdef DISABLE_ENGINES
    unsigned long aes_enable;
#endif
    uint16 rx_index;
    int done = 1;

    printk (KERN_ERR "IPSEC SPU: do_encrypt\n");

#if AES_MD5
    printk (KERN_ERR "IPSEC SPU: AES MD5 TEST\n");
#endif

#if AES_SHA1
    printk (KERN_ERR "IPSEC SPU: AES SHA1 TEST\n");
#endif

#if DES_MD5
    printk (KERN_ERR "IPSEC SPU: DES MD5 TEST\n");
#endif

#if DES_SHA1
    printk (KERN_ERR "IPSEC SPU: DES SHA1 TEST\n");
#endif

#if DES3_MD5
    printk (KERN_ERR "IPSEC SPU: DES3 MD5 TEST\n");
#endif

#if DES3_SHA1
    printk (KERN_ERR "IPSEC SPU: DES3 SHA1 TEST\n");
#endif

    if (tx_pkt_id > MAX_PKT_ID || num_pkts > MAX_PKTS_PER_TEST)
    {
	return;
    }

#ifdef DISABLE_ENGINES
    printk (KERN_ERR "IPSEC SPU: Disabling AES, DES, HASH engines 0x%08x\n",
	  SPU_CTRL);
    aes_enable = SPU_CTRL;
    /* Disable AES, DES/3DES and HASH */
    //aes_enable = (1 << 8) | (1 << 9) | (1 << 10);
    /* Disable DES/3DES and HASH */
    aes_enable = (1 << 9) | (1 << 10);
    SPU_CTRL = aes_enable;
    printk (KERN_ERR "IPSEC SPU: AES, DES, HASH Disabled 0x%08x\n", SPU_CTRL);
#endif

    num_tests_failed = num_tests_passed = 0;

    do
    {
	/*
	 * Setup the rx and tx buffer.
	 */
	rx_index = ipsec_setup_tx_rx (tx_pkt_id, &done);

	/*
	 * Enable the DMA.
	 */

//retry:
	pdev_ctrl->rx_dma->cfg |= DMA_ENABLE;
	pdev_ctrl->tx_dma->cfg |= DMA_ENABLE;
	BcmHalInterruptEnable (pdev_ctrl->rx_irq);

#ifdef SPU_DEBUG
	printk (KERN_ERR
	      "IPSEC SPU: Done Enabling Tx %p %lx and Rx %p %lx DMA\n",
	      &(pdev_ctrl->tx_dma->cfg), pdev_ctrl->tx_dma->cfg,
	      &(pdev_ctrl->rx_dma->cfg), pdev_ctrl->rx_dma->cfg);

	printk (KERN_ERR "IPSEC SPU: Tx Chn BaseDesc %lx st data %lx "
	      "len status %lx bufptr %lx\n",
	      pdev_ctrl->dma_ctrl->stram.s[1].baseDescPtr,
	      pdev_ctrl->dma_ctrl->stram.s[1].state_data,
	      pdev_ctrl->dma_ctrl->stram.s[1].desc_len_status,
	      pdev_ctrl->dma_ctrl->stram.s[1].desc_base_bufptr);
	printk (KERN_ERR "IPSEC SPU: Rx Chn BaseDesc %lx st data %lx "
	      "len status %lx bufptr %lx\n",
	      pdev_ctrl->dma_ctrl->stram.s[0].baseDescPtr,
	      pdev_ctrl->dma_ctrl->stram.s[0].state_data,
	      pdev_ctrl->dma_ctrl->stram.s[0].desc_len_status,
	      pdev_ctrl->dma_ctrl->stram.s[0].desc_base_bufptr);
	printk (KERN_ERR "IPSEC SPU: DMA CTRL Global Interrupt Status %lx "
	      "Mask %lx\n",
	      pdev_ctrl->dma_ctrl->ctrl_global_interrupt_status,
	      pdev_ctrl->dma_ctrl->ctrl_global_interrupt_mask);
#endif

	start = spu_get_cycle_count();

	/*
	 * Unblocked so check the Rx with the template and
	 * print the verdict. But first compute the elapsed
	 * time in usecs.
	 */
	if (end < start)
	{
	    proc_time = (0xFFFFFFFF - start) + end;
	}
	else
	{
	    proc_time = end - start;
	}

	proc_time = proc_time / spu_cycle_per_us;
	encrypt_decrypt_verify(tx_pkt_id, rx_index);
	dump_output (rx_index);
	start = end = proc_time = 0;
	xmit_pkt_len = recv_pkt_len = 0;
	ntries = 0;
	num_pkts--;
	tx_pkt_id++;

    } while (done && num_pkts);

    printk("Num Tests %d Failed %d Passed %d\n", (num_tests_passed + num_tests_failed), num_tests_failed, num_tests_passed);
}
#endif /* CONFIG_BCM_SPU_TEST */

MODULE_LICENSE("GPL");
