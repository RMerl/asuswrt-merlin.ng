/*
 Copyright 2007-2010 Broadcom Corp. All Rights Reserved.

<:label-BRCM:2012:GPL/GPL:standard

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
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <board.h>
#include "boardparms.h"
#include <bcm_map_part.h>
#include "bcmenet.h"
#include "bcmPktDma.h"


#if defined(CONFIG_BCM947189)
static int proc_get_dma_summary(struct seq_file *seq, void *offset)
{
  BcmEnet_devctrl *p = seq->private;
  BcmPktDma_EthTxDma *tx = p->txdma[0];
  BcmEnet_RxDma *rx = p->rxdma[0];

  seq_printf(seq, "== dma controller registers ==\n");

  seq_printf(seq, "ch:  control:ptr:addrlow:addrHigh:status1:status2\n");

  seq_printf(seq, "rx:%08x:%08x:%08x:%08x:%08x:%08x\n",
        rx->pktDmaRxInfo.rxDma->control,
        rx->pktDmaRxInfo.rxDma->ptr,
        rx->pktDmaRxInfo.rxDma->addrlow,
        rx->pktDmaRxInfo.rxDma->addrhigh,
        rx->pktDmaRxInfo.rxDma->status0,
        rx->pktDmaRxInfo.rxDma->status1
        );

  seq_printf(seq, "tx:%08x:%08x:%08x:%08x:%08x:%08x\n",
        tx->txDma->control,
        tx->txDma->ptr,
        tx->txDma->addrlow,
        tx->txDma->addrhigh,
        tx->txDma->status0,
        tx->txDma->status1
        );

  seq_printf(
        seq, "== rxbd:  total %d, avail %d, head %d, tail %d  bd_base 0x%08x ==\n",
        rx->pktDmaRxInfo.numRxBds,
        rx->pktDmaRxInfo.rxAssignedBds,
        rx->pktDmaRxInfo.rxHeadIndex,
        rx->pktDmaRxInfo.rxTailIndex,
        (unsigned int)rx->pktDmaRxInfo.rxBdsBase
        );

  seq_printf(
        seq, "== txbd:  total %d, avail %d, head %d, tail %d  bd_base 0x%08x ==\n",
        tx->numTxBds,
        tx->txFreeBds,
        tx->txHeadIndex,
        tx->txTailIndex,
        (unsigned int)tx->txBdsBase
        );

  return 0;
}

static int dma_summary_open(struct inode *inode, struct file *file)
{
   return single_open(file, proc_get_dma_summary, PDE_DATA(inode));
}


static const struct file_operations dma_summary_fops = {
	.open		= dma_summary_open,
	.read		= seq_read,
	.llseek	= seq_lseek,
	.release	= seq_release,
};


int bcmenet_del_proc_files(struct net_device *dev)
{
  char tmp[32];

  sprintf(tmp, "driver/%s/dma_summary", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s", dev->name);
  remove_proc_entry(tmp, NULL);
  return 0;
}

int bcmenet_add_proc_files(struct net_device *dev)
{
  char tmp[32];
  struct proc_dir_entry *parentdir;
  BcmEnet_devctrl *p = netdev_priv(dev);

  sprintf(tmp, "driver/%s", dev->name);
  parentdir = proc_mkdir (tmp, NULL);

  proc_create_data("dma_summary", S_IRUGO, parentdir, &dma_summary_fops, p);

  return 0;
}
#else
#define ENET_POS_TO_DMA_CHANNEL(pos)              (((pos) & 0xffff0000) >> 16)
#define ENET_POS_TO_SLOT(pos)                     ((pos) & 0xffff)
#define ENET_DMA_CHANNEL_SLOT_TO_POS(chan, slot)  ((chan) << 16 | (slot))

#define ENET_GET_NUMRXBDS(r)        ((r)->pktDmaRxInfo.numRxBds)
#define ENET_GET_RXASSIGNEDBDS(r)   ((r)->pktDmaRxInfo.rxAssignedBds)
#define ENET_GET_RXHEADINDEX(r)     ((r)->pktDmaRxInfo.rxHeadIndex)
#define ENET_GET_RXTAILINDEX(r)     ((r)->pktDmaRxInfo.rxTailIndex)
#define ENET_GET_RXBDS_LENGTH(r, i) ((r)->pktDmaRxInfo.rxBds[(i)].length)
#define ENET_GET_RXBDS_STATUS(r, i) ((r)->pktDmaRxInfo.rxBds[(i)].status)
#define ENET_GET_RXBDS_ADDR(r, i)   ((r)->pktDmaRxInfo.rxBds[(i)].address)

/* TODO: only 1 DMA channel is supported for now */
static int enet_max_dma_channels=1;

/* this is common for pktdma and non-pktdma */
static void *rxbd_seq_start(struct seq_file *f, loff_t *pos)
{
	BcmEnet_RxDma *rxdma;
	uint32_t val = (uint32_t) *pos;
	uint16_t dma_channel;
	uint16_t slot;

	dma_channel = ENET_POS_TO_DMA_CHANNEL(val);
	slot = ENET_POS_TO_SLOT(val);

	rxdma = ((BcmEnet_devctrl *)f->private)->rxdma[dma_channel];

	/* check if there is more data to return */

	if (dma_channel < enet_max_dma_channels && slot < ENET_GET_NUMRXBDS(rxdma)) {
		return pos;
	} else {
		return NULL;
	}
}

static void *rxbd_seq_next(struct seq_file *f, void *v, loff_t *pos)
{
	BcmEnet_RxDma *rxdma;
	uint32_t val = (uint32_t) *pos;
	uint16_t dma_channel;
	uint16_t slot;

	dma_channel = ENET_POS_TO_DMA_CHANNEL(val);
	slot = ENET_POS_TO_SLOT(val);

	rxdma = ((BcmEnet_devctrl *)f->private)->rxdma[dma_channel];

	/* move to the next slot, if no more slots, move to next DMA channel */
	slot++;
	if (slot > ENET_GET_NUMRXBDS(rxdma)) {
		dma_channel++;
		if (dma_channel >= enet_max_dma_channels) {
			return NULL;
		}
		slot = 0;
	}

	val = ENET_DMA_CHANNEL_SLOT_TO_POS(dma_channel, slot);
	*pos = val;
	return pos;
}

static int rxbd_seq_show(struct seq_file *f, void *v)
{
  BcmEnet_devctrl *enetdev = (BcmEnet_devctrl *) f->private;
  BcmEnet_RxDma *rxdma;
  uint32_t pos = *(loff_t *)v;
  uint16_t dma_channel;
  uint16_t slot;

  dma_channel = ENET_POS_TO_DMA_CHANNEL(pos);
  slot = ENET_POS_TO_SLOT(pos);
  rxdma =  enetdev->rxdma[dma_channel];

  if (slot == 0) {
    seq_printf(f,
        "rxbd: total %d, avail %d, head %d, tail %d\n",
        ENET_GET_NUMRXBDS(rxdma),
        ENET_GET_RXASSIGNEDBDS(rxdma),
        ENET_GET_RXHEADINDEX(rxdma),
        ENET_GET_RXHEADINDEX(rxdma)
        );

    seq_printf(f, "idx:len :stat:address\n");
  }

  seq_printf(f,
          "%03d:%04x:%04x:%08x\n",
          slot,
          ENET_GET_RXBDS_LENGTH(rxdma, slot),
          ENET_GET_RXBDS_STATUS(rxdma, slot),
          ENET_GET_RXBDS_ADDR(rxdma, slot)
          );

  return 0;
}

static void rxbd_seq_stop(struct seq_file *f, void *v)
{
	/* Nothing to do */
}

static const struct seq_operations rxbd_seq_ops = {
	.start = rxbd_seq_start,
	.next  = rxbd_seq_next,
	.stop  = rxbd_seq_stop,
	.show  = rxbd_seq_show
};

static int rxbd_open(struct inode *inode, struct file *file)
{
   struct seq_file *seq;
   int rc;

   rc = seq_open(file, &rxbd_seq_ops);
   if (rc)
           return rc;

   seq = file->private_data;
   seq->private = PDE_DATA(inode);
   return 0;
}

static const struct file_operations rxbd_fops = {
	.open		= rxbd_open,
	.read		= seq_read,
	.llseek	= seq_lseek,
	.release	= seq_release,
};

static int proc_get_txbd0(struct seq_file *seq, void *offset)
{
  int n, i;
  int nr_tx_bds; 
  
/* TBD: for each channel */
  BcmPktDma_EthTxDma *p = ((BcmEnet_devctrl *)seq->private)->txdma[0];
  nr_tx_bds = bcmPktDma_EthGetTxBds( p, 0 );
  
  n = 0;
 
  seq_printf(
        seq, "== txbd (%d/2): total %d, avail %d, head %d, tail %d ==\n",
        (n == 0)? 1: 2,
        nr_tx_bds,
        p->txFreeBds,
        p->txHeadIndex,
        p->txTailIndex
        );

  seq_printf(seq, "idx: len:stat: address\n");

  for (i = n; i < n + nr_tx_bds / 2; i++)
  {
    seq_printf(
          seq, "%03d:%04x:%04x:%08x\n",
          i,
          p->txBds[i].length,
          p->txBds[i].status,
          p->txBds[i].address
          );
  }

  seq_printf(seq, "\n");
  return 0;
}

static int proc_get_txbd1(struct seq_file *seq, void *offset) 
{
  int n, i;
  int nr_tx_bds; 

/* TBD: for each channel */
  BcmPktDma_EthTxDma *p = ((BcmEnet_devctrl *)seq->private)->txdma[0];
  nr_tx_bds = bcmPktDma_EthGetTxBds( p, 0 );

  n = nr_tx_bds / 2;
 
  seq_printf(
        seq, "== txbd (%d/2): total %d, avail %d, head %d, tail %d ==\n",
        (n == 0)? 1: 2,
        nr_tx_bds,
        p->txFreeBds,
        p->txHeadIndex,
        p->txTailIndex
        );

  seq_printf(seq, "idx: len:stat: address\n");

  for (i = n; i < n + nr_tx_bds / 2; i++)
  {
     seq_printf(
          seq, "%03d:%04x:%04x:%08x\n",
          i,
          p->txBds[i].length,
          p->txBds[i].status,
          p->txBds[i].address
          );
  }

  seq_printf(seq, "\n");
  return 0;
}

extern volatile IrqControl_t * brcm_irq_ctrl[NR_CPUS];

static int proc_get_dma_summary(struct seq_file *seq, void *offset)
{
  BcmEnet_devctrl *p = seq->private;
  BcmPktDma_EthTxDma *tx = p->txdma[0];
  BcmEnet_RxDma *rx = p->rxdma[0];

  DmaStateRam *rx_sram = (DmaStateRam *)&p->dmaCtrl->stram.s[p->unit * 2];
  DmaStateRam *tx_sram = (DmaStateRam *)&p->dmaCtrl->stram.s[p->unit * 2 + 1];

  seq_printf(seq, "== dma controller registers ==\n");

  seq_printf(seq, "controller config: %08x\n", p->dmaCtrl->controller_cfg);

  seq_printf(seq, "ch:  config:int stat:int mask\n");

  seq_printf(seq, "rx:%08x:%08x:%08x\n",
        rx->pktDmaRxInfo.rxDma->cfg,
        rx->pktDmaRxInfo.rxDma->intStat,
        rx->pktDmaRxInfo.rxDma->intMask
        );

  seq_printf(seq, "tx:%08x:%08x:%08x\n\n",
        tx->txDma->cfg,
        tx->txDma->intStat,
        tx->txDma->intMask
        );

  seq_printf(seq, "== sram contents ==\n");

  seq_printf(seq, "ch: bd base:  status:current bd content\n");

  seq_printf(seq, "rx:%08x:%08x:%08x:%08x\n",
        rx_sram->baseDescPtr,
        rx_sram->state_data,
        rx_sram->desc_len_status,
        rx_sram->desc_base_bufptr
        );

  seq_printf(seq, "tx:%08x:%08x:%08x:%08x\n\n",
        tx_sram->baseDescPtr,
        tx_sram->state_data,
        tx_sram->desc_len_status,
        tx_sram->desc_base_bufptr
        );

  seq_printf(seq, "== MIPS and MISC registers ==\n");
  seq_printf(seq, "CP0 cause:        %08lx\n",   read_c0_cause() & CAUSEF_IP);
  seq_printf(seq, "CP0 status:       %08x\n",    read_c0_status());
  //   seq_printf(m, "PERF->IrqMask:    %08lx\n",   PERF->IrqMask);
  //   seq_printf(m, "PERF->IrqStatus:  %08lx\n\n", PERF->IrqStatus);
 
  return 0;
}


static int txbd0_open(struct inode *inode, struct file *file)
{
   return single_open(file, proc_get_txbd0, PDE_DATA(inode));
}

static int txbd1_open(struct inode *inode, struct file *file)
{
   return single_open(file, proc_get_txbd1, PDE_DATA(inode));
}

static int dma_summary_open(struct inode *inode, struct file *file)
{
   return single_open(file, proc_get_dma_summary, PDE_DATA(inode));
}

static const struct file_operations txbd0_fops = {
	.open		= txbd0_open,
	.read		= seq_read,
	.llseek	= seq_lseek,
	.release	= seq_release,
};

static const struct file_operations txbd1_fops = {
	.open		= txbd1_open,
	.read		= seq_read,
	.llseek	= seq_lseek,
	.release	= seq_release,
};

static const struct file_operations dma_summary_fops = {
	.open		= dma_summary_open,
	.read		= seq_read,
	.llseek	= seq_lseek,
	.release	= seq_release,
};

int bcmenet_del_proc_files(struct net_device *dev)
{
  char tmp[32];

  sprintf(tmp, "driver/%s/rxbd", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s/txbd0", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s/txbd1", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s/dma_summary", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s", dev->name);
  remove_proc_entry(tmp, NULL);
  return 0;
}

int bcmenet_add_proc_files(struct net_device *dev)
{
  char tmp[32];
  struct proc_dir_entry *parentdir;
  BcmEnet_devctrl *p = netdev_priv(dev);

  sprintf(tmp, "driver/%s", dev->name);
  parentdir = proc_mkdir (tmp, NULL);

  proc_create_data("rxbd", 0, parentdir, &rxbd_fops, p);
  proc_create_data("txbd0", S_IRUGO, parentdir, &txbd0_fops, p);
  proc_create_data("txbd1", S_IRUGO, parentdir, &txbd1_fops, p);
  proc_create_data("dma_summary", S_IRUGO, parentdir, &dma_summary_fops, p);

  return 0;
}
#endif

/* End of file */

