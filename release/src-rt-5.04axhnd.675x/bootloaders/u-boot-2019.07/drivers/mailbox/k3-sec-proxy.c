// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 Secure proxy Driver
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/soc/ti/k3-sec-proxy.h>
#include <dm.h>
#include <mailbox-uclass.h>

DECLARE_GLOBAL_DATA_PTR;

/* SEC PROXY RT THREAD STATUS */
#define RT_THREAD_STATUS			0x0
#define RT_THREAD_THRESHOLD			0x4
#define RT_THREAD_STATUS_ERROR_SHIFT		31
#define RT_THREAD_STATUS_ERROR_MASK		BIT(31)
#define RT_THREAD_STATUS_CUR_CNT_SHIFT		0
#define RT_THREAD_STATUS_CUR_CNT_MASK		GENMASK(7, 0)

/* SEC PROXY SCFG THREAD CTRL */
#define SCFG_THREAD_CTRL			0x1000
#define SCFG_THREAD_CTRL_DIR_SHIFT		31
#define SCFG_THREAD_CTRL_DIR_MASK		BIT(31)

#define SEC_PROXY_THREAD(base, x)		((base) + (0x1000 * (x)))
#define THREAD_IS_RX				1
#define THREAD_IS_TX				0

/**
 * struct k3_sec_proxy_desc - Description of secure proxy integration.
 * @thread_count:	Number of Threads.
 * @max_msg_size:	Message size in bytes.
 * @data_start_offset:	Offset of the First data register of the thread
 * @data_end_offset:	Offset of the Last data register of the thread
 * @valid_threads:	List of Valid threads that the processor can access
 * @num_valid_threads:	Number of valid threads.
 */
struct k3_sec_proxy_desc {
	u16 thread_count;
	u16 max_msg_size;
	u16 data_start_offset;
	u16 data_end_offset;
	const u32 *valid_threads;
	u32 num_valid_threads;
};

/**
 * struct k3_sec_proxy_thread - Description of a secure proxy Thread
 * @id:		Thread ID
 * @data:	Thread Data path region for target
 * @scfg:	Secure Config Region for Thread
 * @rt:		RealTime Region for Thread
 * @rx_buf:	Receive buffer data, max message size.
 */
struct k3_sec_proxy_thread {
	u32 id;
	void __iomem *data;
	void __iomem *scfg;
	void __iomem *rt;
	u32 *rx_buf;
};

/**
 * struct k3_sec_proxy_mbox - Description of a Secure Proxy Instance
 * @chan:		Mailbox Channel
 * @desc:		Description of the SoC integration
 * @chans:		Array for valid thread instances
 * @target_data:	Secure Proxy region for Target Data
 * @scfg:		Secure Proxy Region for Secure configuration.
 * @rt:			Secure proxy Region for Real Time Region.
 */
struct k3_sec_proxy_mbox {
	struct mbox_chan chan;
	struct k3_sec_proxy_desc *desc;
	struct k3_sec_proxy_thread *chans;
	phys_addr_t target_data;
	phys_addr_t scfg;
	phys_addr_t rt;
};

static inline u32 sp_readl(void __iomem *addr, unsigned int offset)
{
	return readl(addr + offset);
}

static inline void sp_writel(void __iomem *addr, unsigned int offset, u32 data)
{
	writel(data, addr + offset);
}

/**
 * k3_sec_proxy_of_xlate() - Translation of phandle to channel
 * @chan:	Mailbox channel
 * @args:	Phandle Pointer
 *
 * Translates the phandle args and fills up the Mailbox channel from client.
 * Return: 0 if all goes good, else return corresponding error message.
 */
static int k3_sec_proxy_of_xlate(struct mbox_chan *chan,
				 struct ofnode_phandle_args *args)
{
	struct k3_sec_proxy_mbox *spm = dev_get_priv(chan->dev);
	int ind, i;

	debug("%s(chan=%p)\n", __func__, chan);

	if (args->args_count != 1) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}
	ind = args->args[0];

	for (i = 0; i < spm->desc->num_valid_threads; i++)
		if (spm->chans[i].id == ind) {
			chan->id = ind;
			chan->con_priv = &spm->chans[i];
			return 0;
		}

	dev_err(chan->dev, "%s: Invalid Thread ID %d\n", __func__, ind);
	return -ENOENT;
}

/**
 * k3_sec_proxy_request() - Request for mailbox channel
 * @chan:	Channel Pointer
 */
static int k3_sec_proxy_request(struct mbox_chan *chan)
{
	debug("%s(chan=%p)\n", __func__, chan);

	return 0;
}

/**
 * k3_sec_proxy_free() - Free the mailbox channel
 * @chan:	Channel Pointer
 */
static int k3_sec_proxy_free(struct mbox_chan *chan)
{
	debug("%s(chan=%p)\n", __func__, chan);

	return 0;
}

/**
 * k3_sec_proxy_verify_thread() - Verify thread status before
 *				  sending/receiving data.
 * @spt:	pointer to secure proxy thread description
 * @dir:	Direction of the thread
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static inline int k3_sec_proxy_verify_thread(struct k3_sec_proxy_thread *spt,
					     u8 dir)
{
	/* Check for any errors already available */
	if (sp_readl(spt->rt, RT_THREAD_STATUS) &
	    RT_THREAD_STATUS_ERROR_MASK) {
		printf("%s: Thread %d is corrupted, cannot send data.\n",
		       __func__, spt->id);
		return -EINVAL;
	}

	/* Make sure thread is configured for right direction */
	if ((sp_readl(spt->scfg, SCFG_THREAD_CTRL)
	    & SCFG_THREAD_CTRL_DIR_MASK) >> SCFG_THREAD_CTRL_DIR_SHIFT != dir) {
		if (dir)
			printf("%s: Trying to receive data on tx Thread %d\n",
			       __func__, spt->id);
		else
			printf("%s: Trying to send data on rx Thread %d\n",
			       __func__, spt->id);
		return -EINVAL;
	}

	/* Check the message queue before sending/receiving data */
	if (!(sp_readl(spt->rt, RT_THREAD_STATUS) &
	      RT_THREAD_STATUS_CUR_CNT_MASK))
		return -ENODATA;

	return 0;
}

/**
 * k3_sec_proxy_send() - Send data via mailbox channel
 * @chan:	Channel Pointer
 * @data:	Pointer to k3_sec_proxy_msg
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_sec_proxy_send(struct mbox_chan *chan, const void *data)
{
	const struct k3_sec_proxy_msg *msg = (struct k3_sec_proxy_msg *)data;
	struct k3_sec_proxy_mbox *spm = dev_get_priv(chan->dev);
	struct k3_sec_proxy_thread *spt = chan->con_priv;
	int num_words, trail_bytes, ret;
	void __iomem *data_reg;
	u32 *word_data;

	debug("%s(chan=%p, data=%p)\n", __func__, chan, data);

	ret = k3_sec_proxy_verify_thread(spt, THREAD_IS_TX);
	if (ret) {
		dev_err(dev, "%s: Thread%d verification failed. ret = %d\n",
			__func__, spt->id, ret);
		return ret;
	}

	/* Check the message size. */
	if (msg->len > spm->desc->max_msg_size) {
		printf("%s: Thread %ld message length %zu > max msg size %d\n",
		       __func__, chan->id, msg->len, spm->desc->max_msg_size);
		return -EINVAL;
	}

	/* Send the message */
	data_reg = spt->data + spm->desc->data_start_offset;
	for (num_words = msg->len / sizeof(u32), word_data = (u32 *)msg->buf;
	     num_words;
	     num_words--, data_reg += sizeof(u32), word_data++)
		writel(*word_data, data_reg);

	trail_bytes = msg->len % sizeof(u32);
	if (trail_bytes) {
		u32 data_trail = *word_data;

		/* Ensure all unused data is 0 */
		data_trail &= 0xFFFFFFFF >> (8 * (sizeof(u32) - trail_bytes));
		writel(data_trail, data_reg);
		data_reg++;
	}

	/*
	 * 'data_reg' indicates next register to write. If we did not already
	 * write on tx complete reg(last reg), we must do so for transmit
	 */
	if (data_reg <= (spt->data + spm->desc->data_end_offset))
		sp_writel(spt->data, spm->desc->data_end_offset, 0);

	debug("%s: Message successfully sent on thread %ld\n",
	      __func__, chan->id);

	return 0;
}

/**
 * k3_sec_proxy_recv() - Receive data via mailbox channel
 * @chan:	Channel Pointer
 * @data:	Pointer to k3_sec_proxy_msg
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_sec_proxy_recv(struct mbox_chan *chan, void *data)
{
	struct k3_sec_proxy_mbox *spm = dev_get_priv(chan->dev);
	struct k3_sec_proxy_thread *spt = chan->con_priv;
	struct k3_sec_proxy_msg *msg = data;
	void __iomem *data_reg;
	int num_words, ret;
	u32 *word_data;

	debug("%s(chan=%p, data=%p)\n", __func__, chan, data);

	ret = k3_sec_proxy_verify_thread(spt, THREAD_IS_RX);
	if (ret)
		return ret;

	msg->len = spm->desc->max_msg_size;
	msg->buf = spt->rx_buf;
	data_reg = spt->data + spm->desc->data_start_offset;
	word_data = spt->rx_buf;
	for (num_words = spm->desc->max_msg_size / sizeof(u32);
	     num_words;
	     num_words--, data_reg += sizeof(u32), word_data++)
		*word_data = readl(data_reg);

	debug("%s: Message successfully received from thread %ld\n",
	      __func__, chan->id);

	return 0;
}

struct mbox_ops k3_sec_proxy_mbox_ops = {
	.of_xlate = k3_sec_proxy_of_xlate,
	.request = k3_sec_proxy_request,
	.free = k3_sec_proxy_free,
	.send = k3_sec_proxy_send,
	.recv = k3_sec_proxy_recv,
};

/**
 * k3_sec_proxy_of_to_priv() - generate private data from device tree
 * @dev:	corresponding k3 secure proxy device
 * @spm:	pointer to driver specific private data
 *
 * Return: 0 if all went ok, else corresponding error message.
 */
static int k3_sec_proxy_of_to_priv(struct udevice *dev,
				   struct k3_sec_proxy_mbox *spm)
{
	const void *blob = gd->fdt_blob;

	if (!blob) {
		debug("'%s' no dt?\n", dev->name);
		return -ENODEV;
	}

	spm->target_data = devfdt_get_addr_name(dev, "target_data");
	if (spm->target_data == FDT_ADDR_T_NONE) {
		dev_err(dev, "No reg property for target data base\n");
		return -EINVAL;
	}

	spm->scfg = devfdt_get_addr_name(dev, "scfg");
	if (spm->rt == FDT_ADDR_T_NONE) {
		dev_err(dev, "No reg property for Secure Cfg base\n");
		return -EINVAL;
	}

	spm->rt = devfdt_get_addr_name(dev, "rt");
	if (spm->rt == FDT_ADDR_T_NONE) {
		dev_err(dev, "No reg property for Real Time Cfg base\n");
		return -EINVAL;
	}

	return 0;
}

/**
 * k3_sec_proxy_thread_setup - Initialize the parameters for all valid threads
 * @spm:	Mailbox instance for which threads needs to be initialized
 *
 * Return: 0 if all went ok, else corresponding error message
 */
static int k3_sec_proxy_thread_setup(struct k3_sec_proxy_mbox *spm)
{
	struct k3_sec_proxy_thread *spt;
	int i, ind;

	for (i = 0; i < spm->desc->num_valid_threads; i++) {
		spt = &spm->chans[i];
		ind = spm->desc->valid_threads[i];
		spt->id = ind;
		spt->data = (void *)SEC_PROXY_THREAD(spm->target_data, ind);
		spt->scfg = (void *)SEC_PROXY_THREAD(spm->scfg, ind);
		spt->rt = (void *)SEC_PROXY_THREAD(spm->rt, ind);
		spt->rx_buf = calloc(1, spm->desc->max_msg_size);
		if (!spt->rx_buf)
			return -ENOMEM;
	}

	return 0;
}

/**
 * k3_sec_proxy_probe() - Basic probe
 * @dev:	corresponding mailbox device
 *
 * Return: 0 if all went ok, else corresponding error message
 */
static int k3_sec_proxy_probe(struct udevice *dev)
{
	struct k3_sec_proxy_mbox *spm = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	ret = k3_sec_proxy_of_to_priv(dev, spm);
	if (ret)
		return ret;

	spm->desc = (void *)dev_get_driver_data(dev);
	spm->chans = calloc(spm->desc->num_valid_threads,
			    sizeof(struct k3_sec_proxy_thread));
	if (!spm->chans)
		return -ENOMEM;

	ret = k3_sec_proxy_thread_setup(spm);
	if (ret) {
		debug("%s: secure proxy thread setup failed\n", __func__);
		return ret;
	}

	return 0;
}

static int k3_sec_proxy_remove(struct udevice *dev)
{
	struct k3_sec_proxy_mbox *spm = dev_get_priv(dev);

	debug("%s(dev=%p)\n", __func__, dev);

	free(spm->chans);

	return 0;
}

/*
 * Thread ID #4: ROM request
 * Thread ID #5: ROM response, SYSFW notify
 * Thread ID #6: SYSFW request response
 * Thread ID #7: SYSFW request high priority
 * Thread ID #8: SYSFW request low priority
 * Thread ID #9: SYSFW notify response
 */
static const u32 am6x_valid_threads[] = { 4, 5, 6, 7, 8, 9, 11, 13 };

static const struct k3_sec_proxy_desc am654_desc = {
	.thread_count = 90,
	.max_msg_size = 60,
	.data_start_offset = 0x4,
	.data_end_offset = 0x3C,
	.valid_threads = am6x_valid_threads,
	.num_valid_threads = ARRAY_SIZE(am6x_valid_threads),
};

static const struct udevice_id k3_sec_proxy_ids[] = {
	{ .compatible = "ti,am654-secure-proxy", .data = (ulong)&am654_desc},
	{ }
};

U_BOOT_DRIVER(k3_sec_proxy) = {
	.name = "k3-secure-proxy",
	.id = UCLASS_MAILBOX,
	.of_match = k3_sec_proxy_ids,
	.probe = k3_sec_proxy_probe,
	.remove = k3_sec_proxy_remove,
	.priv_auto_alloc_size = sizeof(struct k3_sec_proxy_mbox),
	.ops = &k3_sec_proxy_mbox_ops,
};
