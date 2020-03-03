/*
 * Copyright(c) 2004 - 2009 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called COPYING.
 */
#ifndef IOATDMA_H
#define IOATDMA_H

#include <linux/dmaengine.h>
#include "hw.h"
#include "registers.h"
#include <linux/init.h>
#include <linux/dmapool.h>
#include <linux/cache.h>
#include <linux/pci_ids.h>
#include <net/tcp.h>

#define IOAT_DMA_VERSION  "4.00"

#define IOAT_LOW_COMPLETION_MASK	0xffffffc0
#define IOAT_DMA_DCA_ANY_CPU		~0

#define to_ioatdma_device(dev) container_of(dev, struct ioatdma_device, common)
#define to_ioat_desc(lh) container_of(lh, struct ioat_desc_sw, node)
#define tx_to_ioat_desc(tx) container_of(tx, struct ioat_desc_sw, txd)
#define to_dev(ioat_chan) (&(ioat_chan)->device->pdev->dev)
#define to_pdev(ioat_chan) ((ioat_chan)->device->pdev)

#define chan_num(ch) ((int)((ch)->reg_base - (ch)->device->reg_base) / 0x80)

/*
 * workaround for IOAT ver.3.0 null descriptor issue
 * (channel returns error when size is 0)
 */
#define NULL_DESC_BUFFER_SIZE 1

enum ioat_irq_mode {
	IOAT_NOIRQ = 0,
	IOAT_MSIX,
	IOAT_MSI,
	IOAT_INTX
};

/**
 * struct ioatdma_device - internal representation of a IOAT device
 * @pdev: PCI-Express device
 * @reg_base: MMIO register space base address
 * @dma_pool: for allocating DMA descriptors
 * @common: embedded struct dma_device
 * @version: version of ioatdma device
 * @msix_entries: irq handlers
 * @idx: per channel data
 * @dca: direct cache access context
 * @intr_quirk: interrupt setup quirk (for ioat_v1 devices)
 * @enumerate_channels: hw version specific channel enumeration
 * @reset_hw: hw version specific channel (re)initialization
 * @cleanup_fn: select between the v2 and v3 cleanup routines
 * @timer_fn: select between the v2 and v3 timer watchdog routines
 * @self_test: hardware version specific self test for each supported op type
 *
 * Note: the v3 cleanup routine supports raid operations
 */
struct ioatdma_device {
	struct pci_dev *pdev;
	void __iomem *reg_base;
	struct pci_pool *dma_pool;
	struct pci_pool *completion_pool;
#define MAX_SED_POOLS	5
	struct dma_pool *sed_hw_pool[MAX_SED_POOLS];
	struct dma_device common;
	u8 version;
	struct msix_entry msix_entries[4];
	struct ioat_chan_common *idx[4];
	struct dca_provider *dca;
	enum ioat_irq_mode irq_mode;
	u32 cap;
	void (*intr_quirk)(struct ioatdma_device *device);
	int (*enumerate_channels)(struct ioatdma_device *device);
	int (*reset_hw)(struct ioat_chan_common *chan);
	void (*cleanup_fn)(unsigned long data);
	void (*timer_fn)(unsigned long data);
	int (*self_test)(struct ioatdma_device *device);
};

struct ioat_chan_common {
	struct dma_chan common;
	void __iomem *reg_base;
	dma_addr_t last_completion;
	spinlock_t cleanup_lock;
	unsigned long state;
	#define IOAT_COMPLETION_PENDING 0
	#define IOAT_COMPLETION_ACK 1
	#define IOAT_RESET_PENDING 2
	#define IOAT_KOBJ_INIT_FAIL 3
	#define IOAT_RESHAPE_PENDING 4
	#define IOAT_RUN 5
	#define IOAT_CHAN_ACTIVE 6
	struct timer_list timer;
	#define COMPLETION_TIMEOUT msecs_to_jiffies(100)
	#define IDLE_TIMEOUT msecs_to_jiffies(2000)
	#define RESET_DELAY msecs_to_jiffies(100)
	struct ioatdma_device *device;
	dma_addr_t completion_dma;
	u64 *completion;
	struct tasklet_struct cleanup_task;
	struct kobject kobj;
};

struct ioat_sysfs_entry {
	struct attribute attr;
	ssize_t (*show)(struct dma_chan *, char *);
};

/**
 * struct ioat_dma_chan - internal representation of a DMA channel
 */
struct ioat_dma_chan {
	struct ioat_chan_common base;

	size_t xfercap;	/* XFERCAP register value expanded out */

	spinlock_t desc_lock;
	struct list_head free_desc;
	struct list_head used_desc;

	int pending;
	u16 desccount;
	u16 active;
};

/**
 * struct ioat_sed_ent - wrapper around super extended hardware descriptor
 * @hw: hardware SED
 * @sed_dma: dma address for the SED
 * @list: list member
 * @parent: point to the dma descriptor that's the parent
 */
struct ioat_sed_ent {
	struct ioat_sed_raw_descriptor *hw;
	dma_addr_t dma;
	struct ioat_ring_ent *parent;
	unsigned int hw_pool;
};

static inline struct ioat_chan_common *to_chan_common(struct dma_chan *c)
{
	return container_of(c, struct ioat_chan_common, common);
}

static inline struct ioat_dma_chan *to_ioat_chan(struct dma_chan *c)
{
	struct ioat_chan_common *chan = to_chan_common(c);

	return container_of(chan, struct ioat_dma_chan, base);
}

/* wrapper around hardware descriptor format + additional software fields */

/**
 * struct ioat_desc_sw - wrapper around hardware descriptor
 * @hw: hardware DMA descriptor (for memcpy)
 * @node: this descriptor will either be on the free list,
 *     or attached to a transaction list (tx_list)
 * @txd: the generic software descriptor for all engines
 * @id: identifier for debug
 */
struct ioat_desc_sw {
	struct ioat_dma_descriptor *hw;
	struct list_head node;
	size_t len;
	struct list_head tx_list;
	struct dma_async_tx_descriptor txd;
	#ifdef DEBUG
	int id;
	#endif
};

#ifdef DEBUG
#define set_desc_id(desc, i) ((desc)->id = (i))
#define desc_id(desc) ((desc)->id)
#else
#define set_desc_id(desc, i)
#define desc_id(desc) (0)
#endif

static inline void
__dump_desc_dbg(struct ioat_chan_common *chan, struct ioat_dma_descriptor *hw,
		struct dma_async_tx_descriptor *tx, int id)
{
	struct device *dev = to_dev(chan);

	dev_dbg(dev, "desc[%d]: (%#llx->%#llx) cookie: %d flags: %#x"
		" ctl: %#10.8x (op: %#x int_en: %d compl: %d)\n", id,
		(unsigned long long) tx->phys,
		(unsigned long long) hw->next, tx->cookie, tx->flags,
		hw->ctl, hw->ctl_f.op, hw->ctl_f.int_en, hw->ctl_f.compl_write);
}

#define dump_desc_dbg(c, d) \
	({ if (d) __dump_desc_dbg(&c->base, d->hw, &d->txd, desc_id(d)); 0; })

static inline struct ioat_chan_common *
ioat_chan_by_index(struct ioatdma_device *device, int index)
{
	return device->idx[index];
}

static inline u64 ioat_chansts_32(struct ioat_chan_common *chan)
{
	u8 ver = chan->device->version;
	u64 status;
	u32 status_lo;

	/* We need to read the low address first as this causes the
	 * chipset to latch the upper bits for the subsequent read
	 */
	status_lo = readl(chan->reg_base + IOAT_CHANSTS_OFFSET_LOW(ver));
	status = readl(chan->reg_base + IOAT_CHANSTS_OFFSET_HIGH(ver));
	status <<= 32;
	status |= status_lo;

	return status;
}

#if BITS_PER_LONG == 64

static inline u64 ioat_chansts(struct ioat_chan_common *chan)
{
	u8 ver = chan->device->version;
	u64 status;

	 /* With IOAT v3.3 the status register is 64bit.  */
	if (ver >= IOAT_VER_3_3)
		status = readq(chan->reg_base + IOAT_CHANSTS_OFFSET(ver));
	else
		status = ioat_chansts_32(chan);

	return status;
}

#else
#define ioat_chansts ioat_chansts_32
#endif

static inline void ioat_start(struct ioat_chan_common *chan)
{
	u8 ver = chan->device->version;

	writeb(IOAT_CHANCMD_START, chan->reg_base + IOAT_CHANCMD_OFFSET(ver));
}

static inline u64 ioat_chansts_to_addr(u64 status)
{
	return status & IOAT_CHANSTS_COMPLETED_DESCRIPTOR_ADDR;
}

static inline u32 ioat_chanerr(struct ioat_chan_common *chan)
{
	return readl(chan->reg_base + IOAT_CHANERR_OFFSET);
}

static inline void ioat_suspend(struct ioat_chan_common *chan)
{
	u8 ver = chan->device->version;

	writeb(IOAT_CHANCMD_SUSPEND, chan->reg_base + IOAT_CHANCMD_OFFSET(ver));
}

static inline void ioat_reset(struct ioat_chan_common *chan)
{
	u8 ver = chan->device->version;

	writeb(IOAT_CHANCMD_RESET, chan->reg_base + IOAT_CHANCMD_OFFSET(ver));
}

static inline bool ioat_reset_pending(struct ioat_chan_common *chan)
{
	u8 ver = chan->device->version;
	u8 cmd;

	cmd = readb(chan->reg_base + IOAT_CHANCMD_OFFSET(ver));
	return (cmd & IOAT_CHANCMD_RESET) == IOAT_CHANCMD_RESET;
}

static inline void ioat_set_chainaddr(struct ioat_dma_chan *ioat, u64 addr)
{
	struct ioat_chan_common *chan = &ioat->base;

	writel(addr & 0x00000000FFFFFFFF,
	       chan->reg_base + IOAT1_CHAINADDR_OFFSET_LOW);
	writel(addr >> 32,
	       chan->reg_base + IOAT1_CHAINADDR_OFFSET_HIGH);
}

static inline bool is_ioat_active(unsigned long status)
{
	return ((status & IOAT_CHANSTS_STATUS) == IOAT_CHANSTS_ACTIVE);
}

static inline bool is_ioat_idle(unsigned long status)
{
	return ((status & IOAT_CHANSTS_STATUS) == IOAT_CHANSTS_DONE);
}

static inline bool is_ioat_halted(unsigned long status)
{
	return ((status & IOAT_CHANSTS_STATUS) == IOAT_CHANSTS_HALTED);
}

static inline bool is_ioat_suspended(unsigned long status)
{
	return ((status & IOAT_CHANSTS_STATUS) == IOAT_CHANSTS_SUSPENDED);
}

/* channel was fatally programmed */
static inline bool is_ioat_bug(unsigned long err)
{
	return !!err;
}

int ioat_probe(struct ioatdma_device *device);
int ioat_register(struct ioatdma_device *device);
int ioat1_dma_probe(struct ioatdma_device *dev, int dca);
int ioat_dma_self_test(struct ioatdma_device *device);
void ioat_dma_remove(struct ioatdma_device *device);
struct dca_provider *ioat_dca_init(struct pci_dev *pdev, void __iomem *iobase);
dma_addr_t ioat_get_current_completion(struct ioat_chan_common *chan);
void ioat_init_channel(struct ioatdma_device *device,
		       struct ioat_chan_common *chan, int idx);
enum dma_status ioat_dma_tx_status(struct dma_chan *c, dma_cookie_t cookie,
				   struct dma_tx_state *txstate);
bool ioat_cleanup_preamble(struct ioat_chan_common *chan,
			   dma_addr_t *phys_complete);
void ioat_kobject_add(struct ioatdma_device *device, struct kobj_type *type);
void ioat_kobject_del(struct ioatdma_device *device);
int ioat_dma_setup_interrupts(struct ioatdma_device *device);
void ioat_stop(struct ioat_chan_common *chan);
extern const struct sysfs_ops ioat_sysfs_ops;
extern struct ioat_sysfs_entry ioat_version_attr;
extern struct ioat_sysfs_entry ioat_cap_attr;
#endif /* IOATDMA_H */
