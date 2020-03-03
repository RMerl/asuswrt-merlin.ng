/*
 * OMAP mailbox driver
 *
 * Copyright (C) 2006-2009 Nokia Corporation. All rights reserved.
 * Copyright (C) 2013-2014 Texas Instruments Inc.
 *
 * Contact: Hiroshi DOYU <Hiroshi.DOYU@nokia.com>
 *          Suman Anna <s-anna@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/platform_data/mailbox-omap.h>
#include <linux/omap-mailbox.h>
#include <linux/mailbox_controller.h>
#include <linux/mailbox_client.h>

#define MAILBOX_REVISION		0x000
#define MAILBOX_MESSAGE(m)		(0x040 + 4 * (m))
#define MAILBOX_FIFOSTATUS(m)		(0x080 + 4 * (m))
#define MAILBOX_MSGSTATUS(m)		(0x0c0 + 4 * (m))

#define OMAP2_MAILBOX_IRQSTATUS(u)	(0x100 + 8 * (u))
#define OMAP2_MAILBOX_IRQENABLE(u)	(0x104 + 8 * (u))

#define OMAP4_MAILBOX_IRQSTATUS(u)	(0x104 + 0x10 * (u))
#define OMAP4_MAILBOX_IRQENABLE(u)	(0x108 + 0x10 * (u))
#define OMAP4_MAILBOX_IRQENABLE_CLR(u)	(0x10c + 0x10 * (u))

#define MAILBOX_IRQSTATUS(type, u)	(type ? OMAP4_MAILBOX_IRQSTATUS(u) : \
						OMAP2_MAILBOX_IRQSTATUS(u))
#define MAILBOX_IRQENABLE(type, u)	(type ? OMAP4_MAILBOX_IRQENABLE(u) : \
						OMAP2_MAILBOX_IRQENABLE(u))
#define MAILBOX_IRQDISABLE(type, u)	(type ? OMAP4_MAILBOX_IRQENABLE_CLR(u) \
						: OMAP2_MAILBOX_IRQENABLE(u))

#define MAILBOX_IRQ_NEWMSG(m)		(1 << (2 * (m)))
#define MAILBOX_IRQ_NOTFULL(m)		(1 << (2 * (m) + 1))

#define MBOX_REG_SIZE			0x120

#define OMAP4_MBOX_REG_SIZE		0x130

#define MBOX_NR_REGS			(MBOX_REG_SIZE / sizeof(u32))
#define OMAP4_MBOX_NR_REGS		(OMAP4_MBOX_REG_SIZE / sizeof(u32))

struct omap_mbox_fifo {
	unsigned long msg;
	unsigned long fifo_stat;
	unsigned long msg_stat;
	unsigned long irqenable;
	unsigned long irqstatus;
	unsigned long irqdisable;
	u32 intr_bit;
};

struct omap_mbox_queue {
	spinlock_t		lock;
	struct kfifo		fifo;
	struct work_struct	work;
	struct omap_mbox	*mbox;
	bool full;
};

struct omap_mbox_device {
	struct device *dev;
	struct mutex cfg_lock;
	void __iomem *mbox_base;
	u32 num_users;
	u32 num_fifos;
	struct omap_mbox **mboxes;
	struct mbox_controller controller;
	struct list_head elem;
};

struct omap_mbox_fifo_info {
	int tx_id;
	int tx_usr;
	int tx_irq;

	int rx_id;
	int rx_usr;
	int rx_irq;

	const char *name;
};

struct omap_mbox {
	const char		*name;
	int			irq;
	struct omap_mbox_queue	*rxq;
	struct device		*dev;
	struct omap_mbox_device *parent;
	struct omap_mbox_fifo	tx_fifo;
	struct omap_mbox_fifo	rx_fifo;
	u32			ctx[OMAP4_MBOX_NR_REGS];
	u32			intr_type;
	struct mbox_chan	*chan;
};

/* global variables for the mailbox devices */
static DEFINE_MUTEX(omap_mbox_devices_lock);
static LIST_HEAD(omap_mbox_devices);

static unsigned int mbox_kfifo_size = CONFIG_OMAP_MBOX_KFIFO_SIZE;
module_param(mbox_kfifo_size, uint, S_IRUGO);
MODULE_PARM_DESC(mbox_kfifo_size, "Size of omap's mailbox kfifo (bytes)");

static struct omap_mbox *mbox_chan_to_omap_mbox(struct mbox_chan *chan)
{
	if (!chan || !chan->con_priv)
		return NULL;

	return (struct omap_mbox *)chan->con_priv;
}

static inline
unsigned int mbox_read_reg(struct omap_mbox_device *mdev, size_t ofs)
{
	return __raw_readl(mdev->mbox_base + ofs);
}

static inline
void mbox_write_reg(struct omap_mbox_device *mdev, u32 val, size_t ofs)
{
	__raw_writel(val, mdev->mbox_base + ofs);
}

/* Mailbox FIFO handle functions */
static mbox_msg_t mbox_fifo_read(struct omap_mbox *mbox)
{
	struct omap_mbox_fifo *fifo = &mbox->rx_fifo;
	return (mbox_msg_t) mbox_read_reg(mbox->parent, fifo->msg);
}

static void mbox_fifo_write(struct omap_mbox *mbox, mbox_msg_t msg)
{
	struct omap_mbox_fifo *fifo = &mbox->tx_fifo;
	mbox_write_reg(mbox->parent, msg, fifo->msg);
}

static int mbox_fifo_empty(struct omap_mbox *mbox)
{
	struct omap_mbox_fifo *fifo = &mbox->rx_fifo;
	return (mbox_read_reg(mbox->parent, fifo->msg_stat) == 0);
}

static int mbox_fifo_full(struct omap_mbox *mbox)
{
	struct omap_mbox_fifo *fifo = &mbox->tx_fifo;
	return mbox_read_reg(mbox->parent, fifo->fifo_stat);
}

/* Mailbox IRQ handle functions */
static void ack_mbox_irq(struct omap_mbox *mbox, omap_mbox_irq_t irq)
{
	struct omap_mbox_fifo *fifo = (irq == IRQ_TX) ?
				&mbox->tx_fifo : &mbox->rx_fifo;
	u32 bit = fifo->intr_bit;
	u32 irqstatus = fifo->irqstatus;

	mbox_write_reg(mbox->parent, bit, irqstatus);

	/* Flush posted write for irq status to avoid spurious interrupts */
	mbox_read_reg(mbox->parent, irqstatus);
}

static int is_mbox_irq(struct omap_mbox *mbox, omap_mbox_irq_t irq)
{
	struct omap_mbox_fifo *fifo = (irq == IRQ_TX) ?
				&mbox->tx_fifo : &mbox->rx_fifo;
	u32 bit = fifo->intr_bit;
	u32 irqenable = fifo->irqenable;
	u32 irqstatus = fifo->irqstatus;

	u32 enable = mbox_read_reg(mbox->parent, irqenable);
	u32 status = mbox_read_reg(mbox->parent, irqstatus);

	return (int)(enable & status & bit);
}

void omap_mbox_save_ctx(struct mbox_chan *chan)
{
	int i;
	int nr_regs;
	struct omap_mbox *mbox = mbox_chan_to_omap_mbox(chan);

	if (WARN_ON(!mbox))
		return;

	if (mbox->intr_type)
		nr_regs = OMAP4_MBOX_NR_REGS;
	else
		nr_regs = MBOX_NR_REGS;
	for (i = 0; i < nr_regs; i++) {
		mbox->ctx[i] = mbox_read_reg(mbox->parent, i * sizeof(u32));

		dev_dbg(mbox->dev, "%s: [%02x] %08x\n", __func__,
			i, mbox->ctx[i]);
	}
}
EXPORT_SYMBOL(omap_mbox_save_ctx);

void omap_mbox_restore_ctx(struct mbox_chan *chan)
{
	int i;
	int nr_regs;
	struct omap_mbox *mbox = mbox_chan_to_omap_mbox(chan);

	if (WARN_ON(!mbox))
		return;

	if (mbox->intr_type)
		nr_regs = OMAP4_MBOX_NR_REGS;
	else
		nr_regs = MBOX_NR_REGS;
	for (i = 0; i < nr_regs; i++) {
		mbox_write_reg(mbox->parent, mbox->ctx[i], i * sizeof(u32));
		dev_dbg(mbox->dev, "%s: [%02x] %08x\n", __func__,
			i, mbox->ctx[i]);
	}
}
EXPORT_SYMBOL(omap_mbox_restore_ctx);

static void _omap_mbox_enable_irq(struct omap_mbox *mbox, omap_mbox_irq_t irq)
{
	u32 l;
	struct omap_mbox_fifo *fifo = (irq == IRQ_TX) ?
				&mbox->tx_fifo : &mbox->rx_fifo;
	u32 bit = fifo->intr_bit;
	u32 irqenable = fifo->irqenable;

	l = mbox_read_reg(mbox->parent, irqenable);
	l |= bit;
	mbox_write_reg(mbox->parent, l, irqenable);
}

static void _omap_mbox_disable_irq(struct omap_mbox *mbox, omap_mbox_irq_t irq)
{
	struct omap_mbox_fifo *fifo = (irq == IRQ_TX) ?
				&mbox->tx_fifo : &mbox->rx_fifo;
	u32 bit = fifo->intr_bit;
	u32 irqdisable = fifo->irqdisable;

	/*
	 * Read and update the interrupt configuration register for pre-OMAP4.
	 * OMAP4 and later SoCs have a dedicated interrupt disabling register.
	 */
	if (!mbox->intr_type)
		bit = mbox_read_reg(mbox->parent, irqdisable) & ~bit;

	mbox_write_reg(mbox->parent, bit, irqdisable);
}

void omap_mbox_enable_irq(struct mbox_chan *chan, omap_mbox_irq_t irq)
{
	struct omap_mbox *mbox = mbox_chan_to_omap_mbox(chan);

	if (WARN_ON(!mbox))
		return;

	_omap_mbox_enable_irq(mbox, irq);
}
EXPORT_SYMBOL(omap_mbox_enable_irq);

void omap_mbox_disable_irq(struct mbox_chan *chan, omap_mbox_irq_t irq)
{
	struct omap_mbox *mbox = mbox_chan_to_omap_mbox(chan);

	if (WARN_ON(!mbox))
		return;

	_omap_mbox_disable_irq(mbox, irq);
}
EXPORT_SYMBOL(omap_mbox_disable_irq);

/*
 * Message receiver(workqueue)
 */
static void mbox_rx_work(struct work_struct *work)
{
	struct omap_mbox_queue *mq =
			container_of(work, struct omap_mbox_queue, work);
	mbox_msg_t msg;
	int len;

	while (kfifo_len(&mq->fifo) >= sizeof(msg)) {
		len = kfifo_out(&mq->fifo, (unsigned char *)&msg, sizeof(msg));
		WARN_ON(len != sizeof(msg));

		mbox_chan_received_data(mq->mbox->chan, (void *)msg);
		spin_lock_irq(&mq->lock);
		if (mq->full) {
			mq->full = false;
			_omap_mbox_enable_irq(mq->mbox, IRQ_RX);
		}
		spin_unlock_irq(&mq->lock);
	}
}

/*
 * Mailbox interrupt handler
 */
static void __mbox_tx_interrupt(struct omap_mbox *mbox)
{
	_omap_mbox_disable_irq(mbox, IRQ_TX);
	ack_mbox_irq(mbox, IRQ_TX);
	mbox_chan_txdone(mbox->chan, 0);
}

static void __mbox_rx_interrupt(struct omap_mbox *mbox)
{
	struct omap_mbox_queue *mq = mbox->rxq;
	mbox_msg_t msg;
	int len;

	while (!mbox_fifo_empty(mbox)) {
		if (unlikely(kfifo_avail(&mq->fifo) < sizeof(msg))) {
			_omap_mbox_disable_irq(mbox, IRQ_RX);
			mq->full = true;
			goto nomem;
		}

		msg = mbox_fifo_read(mbox);

		len = kfifo_in(&mq->fifo, (unsigned char *)&msg, sizeof(msg));
		WARN_ON(len != sizeof(msg));
	}

	/* no more messages in the fifo. clear IRQ source. */
	ack_mbox_irq(mbox, IRQ_RX);
nomem:
	schedule_work(&mbox->rxq->work);
}

static irqreturn_t mbox_interrupt(int irq, void *p)
{
	struct omap_mbox *mbox = p;

	if (is_mbox_irq(mbox, IRQ_TX))
		__mbox_tx_interrupt(mbox);

	if (is_mbox_irq(mbox, IRQ_RX))
		__mbox_rx_interrupt(mbox);

	return IRQ_HANDLED;
}

static struct omap_mbox_queue *mbox_queue_alloc(struct omap_mbox *mbox,
					void (*work)(struct work_struct *))
{
	struct omap_mbox_queue *mq;

	if (!work)
		return NULL;

	mq = kzalloc(sizeof(struct omap_mbox_queue), GFP_KERNEL);
	if (!mq)
		return NULL;

	spin_lock_init(&mq->lock);

	if (kfifo_alloc(&mq->fifo, mbox_kfifo_size, GFP_KERNEL))
		goto error;

	INIT_WORK(&mq->work, work);
	return mq;

error:
	kfree(mq);
	return NULL;
}

static void mbox_queue_free(struct omap_mbox_queue *q)
{
	kfifo_free(&q->fifo);
	kfree(q);
}

static int omap_mbox_startup(struct omap_mbox *mbox)
{
	int ret = 0;
	struct omap_mbox_queue *mq;

	mq = mbox_queue_alloc(mbox, mbox_rx_work);
	if (!mq)
		return -ENOMEM;
	mbox->rxq = mq;
	mq->mbox = mbox;

	ret = request_irq(mbox->irq, mbox_interrupt, IRQF_SHARED,
			  mbox->name, mbox);
	if (unlikely(ret)) {
		pr_err("failed to register mailbox interrupt:%d\n", ret);
		goto fail_request_irq;
	}

	_omap_mbox_enable_irq(mbox, IRQ_RX);

	return 0;

fail_request_irq:
	mbox_queue_free(mbox->rxq);
	return ret;
}

static void omap_mbox_fini(struct omap_mbox *mbox)
{
	_omap_mbox_disable_irq(mbox, IRQ_RX);
	free_irq(mbox->irq, mbox);
	flush_work(&mbox->rxq->work);
	mbox_queue_free(mbox->rxq);
}

static struct omap_mbox *omap_mbox_device_find(struct omap_mbox_device *mdev,
					       const char *mbox_name)
{
	struct omap_mbox *_mbox, *mbox = NULL;
	struct omap_mbox **mboxes = mdev->mboxes;
	int i;

	if (!mboxes)
		return NULL;

	for (i = 0; (_mbox = mboxes[i]); i++) {
		if (!strcmp(_mbox->name, mbox_name)) {
			mbox = _mbox;
			break;
		}
	}
	return mbox;
}

struct mbox_chan *omap_mbox_request_channel(struct mbox_client *cl,
					    const char *chan_name)
{
	struct device *dev = cl->dev;
	struct omap_mbox *mbox = NULL;
	struct omap_mbox_device *mdev;
	struct mbox_chan *chan;
	unsigned long flags;
	int ret;

	if (!dev)
		return ERR_PTR(-ENODEV);

	if (dev->of_node) {
		pr_err("%s: please use mbox_request_channel(), this API is supported only for OMAP non-DT usage\n",
		       __func__);
		return ERR_PTR(-ENODEV);
	}

	mutex_lock(&omap_mbox_devices_lock);
	list_for_each_entry(mdev, &omap_mbox_devices, elem) {
		mbox = omap_mbox_device_find(mdev, chan_name);
		if (mbox)
			break;
	}
	mutex_unlock(&omap_mbox_devices_lock);

	if (!mbox || !mbox->chan)
		return ERR_PTR(-ENOENT);

	chan = mbox->chan;
	spin_lock_irqsave(&chan->lock, flags);
	chan->msg_free = 0;
	chan->msg_count = 0;
	chan->active_req = NULL;
	chan->cl = cl;
	init_completion(&chan->tx_complete);
	spin_unlock_irqrestore(&chan->lock, flags);

	ret = chan->mbox->ops->startup(chan);
	if (ret) {
		pr_err("Unable to startup the chan (%d)\n", ret);
		mbox_free_channel(chan);
		chan = ERR_PTR(ret);
	}

	return chan;
}
EXPORT_SYMBOL(omap_mbox_request_channel);

static struct class omap_mbox_class = { .name = "mbox", };

static int omap_mbox_register(struct omap_mbox_device *mdev)
{
	int ret;
	int i;
	struct omap_mbox **mboxes;

	if (!mdev || !mdev->mboxes)
		return -EINVAL;

	mboxes = mdev->mboxes;
	for (i = 0; mboxes[i]; i++) {
		struct omap_mbox *mbox = mboxes[i];
		mbox->dev = device_create(&omap_mbox_class, mdev->dev,
					0, mbox, "%s", mbox->name);
		if (IS_ERR(mbox->dev)) {
			ret = PTR_ERR(mbox->dev);
			goto err_out;
		}
	}

	mutex_lock(&omap_mbox_devices_lock);
	list_add(&mdev->elem, &omap_mbox_devices);
	mutex_unlock(&omap_mbox_devices_lock);

	ret = mbox_controller_register(&mdev->controller);

err_out:
	if (ret) {
		while (i--)
			device_unregister(mboxes[i]->dev);
	}
	return ret;
}

static int omap_mbox_unregister(struct omap_mbox_device *mdev)
{
	int i;
	struct omap_mbox **mboxes;

	if (!mdev || !mdev->mboxes)
		return -EINVAL;

	mutex_lock(&omap_mbox_devices_lock);
	list_del(&mdev->elem);
	mutex_unlock(&omap_mbox_devices_lock);

	mbox_controller_unregister(&mdev->controller);

	mboxes = mdev->mboxes;
	for (i = 0; mboxes[i]; i++)
		device_unregister(mboxes[i]->dev);
	return 0;
}

static int omap_mbox_chan_startup(struct mbox_chan *chan)
{
	struct omap_mbox *mbox = mbox_chan_to_omap_mbox(chan);
	struct omap_mbox_device *mdev = mbox->parent;
	int ret = 0;

	mutex_lock(&mdev->cfg_lock);
	pm_runtime_get_sync(mdev->dev);
	ret = omap_mbox_startup(mbox);
	if (ret)
		pm_runtime_put_sync(mdev->dev);
	mutex_unlock(&mdev->cfg_lock);
	return ret;
}

static void omap_mbox_chan_shutdown(struct mbox_chan *chan)
{
	struct omap_mbox *mbox = mbox_chan_to_omap_mbox(chan);
	struct omap_mbox_device *mdev = mbox->parent;

	mutex_lock(&mdev->cfg_lock);
	omap_mbox_fini(mbox);
	pm_runtime_put_sync(mdev->dev);
	mutex_unlock(&mdev->cfg_lock);
}

static int omap_mbox_chan_send_data(struct mbox_chan *chan, void *data)
{
	struct omap_mbox *mbox = mbox_chan_to_omap_mbox(chan);
	int ret = -EBUSY;

	if (!mbox)
		return -EINVAL;

	if (!mbox_fifo_full(mbox)) {
		mbox_fifo_write(mbox, (mbox_msg_t)data);
		ret = 0;
	}

	/* always enable the interrupt */
	_omap_mbox_enable_irq(mbox, IRQ_TX);
	return ret;
}

static struct mbox_chan_ops omap_mbox_chan_ops = {
	.startup        = omap_mbox_chan_startup,
	.send_data      = omap_mbox_chan_send_data,
	.shutdown       = omap_mbox_chan_shutdown,
};

static const struct of_device_id omap_mailbox_of_match[] = {
	{
		.compatible	= "ti,omap2-mailbox",
		.data		= (void *)MBOX_INTR_CFG_TYPE1,
	},
	{
		.compatible	= "ti,omap3-mailbox",
		.data		= (void *)MBOX_INTR_CFG_TYPE1,
	},
	{
		.compatible	= "ti,omap4-mailbox",
		.data		= (void *)MBOX_INTR_CFG_TYPE2,
	},
	{
		/* end */
	},
};
MODULE_DEVICE_TABLE(of, omap_mailbox_of_match);

static struct mbox_chan *omap_mbox_of_xlate(struct mbox_controller *controller,
					    const struct of_phandle_args *sp)
{
	phandle phandle = sp->args[0];
	struct device_node *node;
	struct omap_mbox_device *mdev;
	struct omap_mbox *mbox;

	mdev = container_of(controller, struct omap_mbox_device, controller);
	if (WARN_ON(!mdev))
		return NULL;

	node = of_find_node_by_phandle(phandle);
	if (!node) {
		pr_err("%s: could not find node phandle 0x%x\n",
		       __func__, phandle);
		return NULL;
	}

	mbox = omap_mbox_device_find(mdev, node->name);
	of_node_put(node);
	return mbox ? mbox->chan : NULL;
}

static int omap_mbox_probe(struct platform_device *pdev)
{
	struct resource *mem;
	int ret;
	struct mbox_chan *chnls;
	struct omap_mbox **list, *mbox, *mboxblk;
	struct omap_mbox_pdata *pdata = pdev->dev.platform_data;
	struct omap_mbox_dev_info *info = NULL;
	struct omap_mbox_fifo_info *finfo, *finfoblk;
	struct omap_mbox_device *mdev;
	struct omap_mbox_fifo *fifo;
	struct device_node *node = pdev->dev.of_node;
	struct device_node *child;
	const struct of_device_id *match;
	u32 intr_type, info_count;
	u32 num_users, num_fifos;
	u32 tmp[3];
	u32 l;
	int i;

	if (!node && (!pdata || !pdata->info_cnt || !pdata->info)) {
		pr_err("%s: platform not supported\n", __func__);
		return -ENODEV;
	}

	if (node) {
		match = of_match_device(omap_mailbox_of_match, &pdev->dev);
		if (!match)
			return -ENODEV;
		intr_type = (u32)match->data;

		if (of_property_read_u32(node, "ti,mbox-num-users",
					 &num_users))
			return -ENODEV;

		if (of_property_read_u32(node, "ti,mbox-num-fifos",
					 &num_fifos))
			return -ENODEV;

		info_count = of_get_available_child_count(node);
		if (!info_count) {
			dev_err(&pdev->dev, "no available mbox devices found\n");
			return -ENODEV;
		}
	} else { /* non-DT device creation */
		info_count = pdata->info_cnt;
		info = pdata->info;
		intr_type = pdata->intr_type;
		num_users = pdata->num_users;
		num_fifos = pdata->num_fifos;
	}

	finfoblk = devm_kzalloc(&pdev->dev, info_count * sizeof(*finfoblk),
				GFP_KERNEL);
	if (!finfoblk)
		return -ENOMEM;

	finfo = finfoblk;
	child = NULL;
	for (i = 0; i < info_count; i++, finfo++) {
		if (node) {
			child = of_get_next_available_child(node, child);
			ret = of_property_read_u32_array(child, "ti,mbox-tx",
							 tmp, ARRAY_SIZE(tmp));
			if (ret)
				return ret;
			finfo->tx_id = tmp[0];
			finfo->tx_irq = tmp[1];
			finfo->tx_usr = tmp[2];

			ret = of_property_read_u32_array(child, "ti,mbox-rx",
							 tmp, ARRAY_SIZE(tmp));
			if (ret)
				return ret;
			finfo->rx_id = tmp[0];
			finfo->rx_irq = tmp[1];
			finfo->rx_usr = tmp[2];

			finfo->name = child->name;
		} else {
			finfo->tx_id = info->tx_id;
			finfo->rx_id = info->rx_id;
			finfo->tx_usr = info->usr_id;
			finfo->tx_irq = info->irq_id;
			finfo->rx_usr = info->usr_id;
			finfo->rx_irq = info->irq_id;
			finfo->name = info->name;
			info++;
		}
		if (finfo->tx_id >= num_fifos || finfo->rx_id >= num_fifos ||
		    finfo->tx_usr >= num_users || finfo->rx_usr >= num_users)
			return -EINVAL;
	}

	mdev = devm_kzalloc(&pdev->dev, sizeof(*mdev), GFP_KERNEL);
	if (!mdev)
		return -ENOMEM;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mdev->mbox_base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(mdev->mbox_base))
		return PTR_ERR(mdev->mbox_base);

	/* allocate one extra for marking end of list */
	list = devm_kzalloc(&pdev->dev, (info_count + 1) * sizeof(*list),
			    GFP_KERNEL);
	if (!list)
		return -ENOMEM;

	chnls = devm_kzalloc(&pdev->dev, (info_count + 1) * sizeof(*chnls),
			     GFP_KERNEL);
	if (!chnls)
		return -ENOMEM;

	mboxblk = devm_kzalloc(&pdev->dev, info_count * sizeof(*mbox),
			       GFP_KERNEL);
	if (!mboxblk)
		return -ENOMEM;

	mbox = mboxblk;
	finfo = finfoblk;
	for (i = 0; i < info_count; i++, finfo++) {
		fifo = &mbox->tx_fifo;
		fifo->msg = MAILBOX_MESSAGE(finfo->tx_id);
		fifo->fifo_stat = MAILBOX_FIFOSTATUS(finfo->tx_id);
		fifo->intr_bit = MAILBOX_IRQ_NOTFULL(finfo->tx_id);
		fifo->irqenable = MAILBOX_IRQENABLE(intr_type, finfo->tx_usr);
		fifo->irqstatus = MAILBOX_IRQSTATUS(intr_type, finfo->tx_usr);
		fifo->irqdisable = MAILBOX_IRQDISABLE(intr_type, finfo->tx_usr);

		fifo = &mbox->rx_fifo;
		fifo->msg = MAILBOX_MESSAGE(finfo->rx_id);
		fifo->msg_stat =  MAILBOX_MSGSTATUS(finfo->rx_id);
		fifo->intr_bit = MAILBOX_IRQ_NEWMSG(finfo->rx_id);
		fifo->irqenable = MAILBOX_IRQENABLE(intr_type, finfo->rx_usr);
		fifo->irqstatus = MAILBOX_IRQSTATUS(intr_type, finfo->rx_usr);
		fifo->irqdisable = MAILBOX_IRQDISABLE(intr_type, finfo->rx_usr);

		mbox->intr_type = intr_type;

		mbox->parent = mdev;
		mbox->name = finfo->name;
		mbox->irq = platform_get_irq(pdev, finfo->tx_irq);
		if (mbox->irq < 0)
			return mbox->irq;
		mbox->chan = &chnls[i];
		chnls[i].con_priv = mbox;
		list[i] = mbox++;
	}

	mutex_init(&mdev->cfg_lock);
	mdev->dev = &pdev->dev;
	mdev->num_users = num_users;
	mdev->num_fifos = num_fifos;
	mdev->mboxes = list;

	/* OMAP does not have a Tx-Done IRQ, but rather a Tx-Ready IRQ */
	mdev->controller.txdone_irq = true;
	mdev->controller.dev = mdev->dev;
	mdev->controller.ops = &omap_mbox_chan_ops;
	mdev->controller.chans = chnls;
	mdev->controller.num_chans = info_count;
	mdev->controller.of_xlate = omap_mbox_of_xlate;
	ret = omap_mbox_register(mdev);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, mdev);
	pm_runtime_enable(mdev->dev);

	ret = pm_runtime_get_sync(mdev->dev);
	if (ret < 0) {
		pm_runtime_put_noidle(mdev->dev);
		goto unregister;
	}

	/*
	 * just print the raw revision register, the format is not
	 * uniform across all SoCs
	 */
	l = mbox_read_reg(mdev, MAILBOX_REVISION);
	dev_info(mdev->dev, "omap mailbox rev 0x%x\n", l);

	ret = pm_runtime_put_sync(mdev->dev);
	if (ret < 0)
		goto unregister;

	devm_kfree(&pdev->dev, finfoblk);
	return 0;

unregister:
	pm_runtime_disable(mdev->dev);
	omap_mbox_unregister(mdev);
	return ret;
}

static int omap_mbox_remove(struct platform_device *pdev)
{
	struct omap_mbox_device *mdev = platform_get_drvdata(pdev);

	pm_runtime_disable(mdev->dev);
	omap_mbox_unregister(mdev);

	return 0;
}

static struct platform_driver omap_mbox_driver = {
	.probe	= omap_mbox_probe,
	.remove	= omap_mbox_remove,
	.driver	= {
		.name = "omap-mailbox",
		.of_match_table = of_match_ptr(omap_mailbox_of_match),
	},
};

static int __init omap_mbox_init(void)
{
	int err;

	err = class_register(&omap_mbox_class);
	if (err)
		return err;

	/* kfifo size sanity check: alignment and minimal size */
	mbox_kfifo_size = ALIGN(mbox_kfifo_size, sizeof(mbox_msg_t));
	mbox_kfifo_size = max_t(unsigned int, mbox_kfifo_size,
							sizeof(mbox_msg_t));

	return platform_driver_register(&omap_mbox_driver);
}
subsys_initcall(omap_mbox_init);

static void __exit omap_mbox_exit(void)
{
	platform_driver_unregister(&omap_mbox_driver);
	class_unregister(&omap_mbox_class);
}
module_exit(omap_mbox_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("omap mailbox: interrupt driven messaging");
MODULE_AUTHOR("Toshihiro Kobayashi");
MODULE_AUTHOR("Hiroshi DOYU");
