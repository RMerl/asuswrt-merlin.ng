// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <blk.h>
#include <bootdev.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <time.h>
#include <dm/device-internal.h>
#include <linux/compat.h>
#include "nvme.h"

#define NVME_Q_DEPTH		2
#define NVME_AQ_DEPTH		2
#define NVME_SQ_SIZE(depth)	(depth * sizeof(struct nvme_command))
#define NVME_CQ_SIZE(depth)	(depth * sizeof(struct nvme_completion))
#define NVME_CQ_ALLOCATION	ALIGN(NVME_CQ_SIZE(NVME_Q_DEPTH), \
				      ARCH_DMA_MINALIGN)
#define ADMIN_TIMEOUT		60
#define IO_TIMEOUT		30
#define MAX_PRP_POOL		512

static int nvme_wait_csts(struct nvme_dev *dev, u32 mask, u32 val)
{
	int timeout;
	ulong start;

	/* Timeout field in the CAP register is in 500 millisecond units */
	timeout = NVME_CAP_TIMEOUT(dev->cap) * 500;

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if ((readl(&dev->bar->csts) & mask) == val)
			return 0;
	}

	return -ETIME;
}

static int nvme_setup_prps(struct nvme_dev *dev, u64 *prp2,
			   int total_len, u64 dma_addr)
{
	u32 page_size = dev->page_size;
	int offset = dma_addr & (page_size - 1);
	u64 *prp_pool;
	int length = total_len;
	int i, nprps;
	u32 prps_per_page = page_size >> 3;
	u32 num_pages;

	length -= (page_size - offset);

	if (length <= 0) {
		*prp2 = 0;
		return 0;
	}

	if (length)
		dma_addr += (page_size - offset);

	if (length <= page_size) {
		*prp2 = dma_addr;
		return 0;
	}

	nprps = DIV_ROUND_UP(length, page_size);
	num_pages = DIV_ROUND_UP(nprps - 1, prps_per_page - 1);

	if (nprps > dev->prp_entry_num) {
		free(dev->prp_pool);
		/*
		 * Always increase in increments of pages.  It doesn't waste
		 * much memory and reduces the number of allocations.
		 */
		dev->prp_pool = memalign(page_size, num_pages * page_size);
		if (!dev->prp_pool) {
			printf("Error: malloc prp_pool fail\n");
			return -ENOMEM;
		}
		dev->prp_entry_num = num_pages * (prps_per_page - 1) + 1;
	}

	prp_pool = dev->prp_pool;
	i = 0;
	while (nprps) {
		if ((i == (prps_per_page - 1)) && nprps > 1) {
			*(prp_pool + i) = cpu_to_le64((ulong)prp_pool +
					page_size);
			i = 0;
			prp_pool += page_size;
		}
		*(prp_pool + i++) = cpu_to_le64(dma_addr);
		dma_addr += page_size;
		nprps--;
	}
	*prp2 = (ulong)dev->prp_pool;

	flush_dcache_range((ulong)dev->prp_pool, (ulong)dev->prp_pool +
			   num_pages * page_size);

	return 0;
}

static __le16 nvme_get_cmd_id(void)
{
	static unsigned short cmdid;

	return cpu_to_le16((cmdid < USHRT_MAX) ? cmdid++ : 0);
}

static u16 nvme_read_completion_status(struct nvme_queue *nvmeq, u16 index)
{
	/*
	 * Single CQ entries are always smaller than a cache line, so we
	 * can't invalidate them individually. However CQ entries are
	 * read only by the CPU, so it's safe to always invalidate all of them,
	 * as the cache line should never become dirty.
	 */
	ulong start = (ulong)&nvmeq->cqes[0];
	ulong stop = start + NVME_CQ_ALLOCATION;

	invalidate_dcache_range(start, stop);

	return readw(&(nvmeq->cqes[index].status));
}

/**
 * nvme_submit_cmd() - copy a command into a queue and ring the doorbell
 *
 * @nvmeq:	The queue to use
 * @cmd:	The command to send
 */
static void nvme_submit_cmd(struct nvme_queue *nvmeq, struct nvme_command *cmd)
{
	struct nvme_ops *ops;
	u16 tail = nvmeq->sq_tail;

	memcpy(&nvmeq->sq_cmds[tail], cmd, sizeof(*cmd));
	flush_dcache_range((ulong)&nvmeq->sq_cmds[tail],
			   (ulong)&nvmeq->sq_cmds[tail] + sizeof(*cmd));

	ops = (struct nvme_ops *)nvmeq->dev->udev->driver->ops;
	if (ops && ops->submit_cmd) {
		ops->submit_cmd(nvmeq, cmd);
		return;
	}

	if (++tail == nvmeq->q_depth)
		tail = 0;
	writel(tail, nvmeq->q_db);
	nvmeq->sq_tail = tail;
}

static int nvme_submit_sync_cmd(struct nvme_queue *nvmeq,
				struct nvme_command *cmd,
				u32 *result, unsigned timeout)
{
	struct nvme_ops *ops;
	u16 head = nvmeq->cq_head;
	u16 phase = nvmeq->cq_phase;
	u16 status;
	ulong start_time;
	ulong timeout_us = timeout * 100000;

	cmd->common.command_id = nvme_get_cmd_id();
	nvme_submit_cmd(nvmeq, cmd);

	start_time = timer_get_us();

	for (;;) {
		status = nvme_read_completion_status(nvmeq, head);
		if ((status & 0x01) == phase)
			break;
		if (timeout_us > 0 && (timer_get_us() - start_time)
		    >= timeout_us)
			return -ETIMEDOUT;
	}

	ops = (struct nvme_ops *)nvmeq->dev->udev->driver->ops;
	if (ops && ops->complete_cmd)
		ops->complete_cmd(nvmeq, cmd);

	status >>= 1;
	if (status) {
		printf("ERROR: status = %x, phase = %d, head = %d\n",
		       status, phase, head);
		status = 0;
		if (++head == nvmeq->q_depth) {
			head = 0;
			phase = !phase;
		}
		writel(head, nvmeq->q_db + nvmeq->dev->db_stride);
		nvmeq->cq_head = head;
		nvmeq->cq_phase = phase;

		return -EIO;
	}

	if (result)
		*result = readl(&(nvmeq->cqes[head].result));

	if (++head == nvmeq->q_depth) {
		head = 0;
		phase = !phase;
	}
	writel(head, nvmeq->q_db + nvmeq->dev->db_stride);
	nvmeq->cq_head = head;
	nvmeq->cq_phase = phase;

	return status;
}

static int nvme_submit_admin_cmd(struct nvme_dev *dev, struct nvme_command *cmd,
				 u32 *result)
{
	return nvme_submit_sync_cmd(dev->queues[NVME_ADMIN_Q], cmd,
				    result, ADMIN_TIMEOUT);
}

static struct nvme_queue *nvme_alloc_queue(struct nvme_dev *dev,
					   int qid, int depth)
{
	struct nvme_ops *ops;
	struct nvme_queue *nvmeq = malloc(sizeof(*nvmeq));
	if (!nvmeq)
		return NULL;
	memset(nvmeq, 0, sizeof(*nvmeq));

	nvmeq->cqes = (void *)memalign(4096, NVME_CQ_ALLOCATION);
	if (!nvmeq->cqes)
		goto free_nvmeq;
	memset((void *)nvmeq->cqes, 0, NVME_CQ_SIZE(depth));

	nvmeq->sq_cmds = (void *)memalign(4096, NVME_SQ_SIZE(depth));
	if (!nvmeq->sq_cmds)
		goto free_queue;
	memset((void *)nvmeq->sq_cmds, 0, NVME_SQ_SIZE(depth));

	nvmeq->dev = dev;

	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	nvmeq->q_depth = depth;
	nvmeq->qid = qid;
	dev->queue_count++;
	dev->queues[qid] = nvmeq;

	ops = (struct nvme_ops *)dev->udev->driver->ops;
	if (ops && ops->setup_queue)
		ops->setup_queue(nvmeq);

	return nvmeq;

 free_queue:
	free((void *)nvmeq->cqes);
 free_nvmeq:
	free(nvmeq);

	return NULL;
}

static int nvme_delete_queue(struct nvme_dev *dev, u8 opcode, u16 id)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.delete_queue.opcode = opcode;
	c.delete_queue.qid = cpu_to_le16(id);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int nvme_delete_sq(struct nvme_dev *dev, u16 sqid)
{
	return nvme_delete_queue(dev, nvme_admin_delete_sq, sqid);
}

static int nvme_delete_cq(struct nvme_dev *dev, u16 cqid)
{
	return nvme_delete_queue(dev, nvme_admin_delete_cq, cqid);
}

static int nvme_enable_ctrl(struct nvme_dev *dev)
{
	dev->ctrl_config &= ~NVME_CC_SHN_MASK;
	dev->ctrl_config |= NVME_CC_ENABLE;
	writel(dev->ctrl_config, &dev->bar->cc);

	return nvme_wait_csts(dev, NVME_CSTS_RDY, NVME_CSTS_RDY);
}

static int nvme_disable_ctrl(struct nvme_dev *dev)
{
	dev->ctrl_config &= ~NVME_CC_SHN_MASK;
	dev->ctrl_config &= ~NVME_CC_ENABLE;
	writel(dev->ctrl_config, &dev->bar->cc);

	return nvme_wait_csts(dev, NVME_CSTS_RDY, 0);
}

static int nvme_shutdown_ctrl(struct nvme_dev *dev)
{
	dev->ctrl_config &= ~NVME_CC_SHN_MASK;
	dev->ctrl_config |= NVME_CC_SHN_NORMAL;
	writel(dev->ctrl_config, &dev->bar->cc);

	return nvme_wait_csts(dev, NVME_CSTS_SHST_MASK, NVME_CSTS_SHST_CMPLT);
}

static void nvme_free_queue(struct nvme_queue *nvmeq)
{
	free((void *)nvmeq->cqes);
	free(nvmeq->sq_cmds);
	free(nvmeq);
}

static void nvme_free_queues(struct nvme_dev *dev, int lowest)
{
	int i;

	for (i = dev->queue_count - 1; i >= lowest; i--) {
		struct nvme_queue *nvmeq = dev->queues[i];
		dev->queue_count--;
		dev->queues[i] = NULL;
		nvme_free_queue(nvmeq);
	}
}

static void nvme_init_queue(struct nvme_queue *nvmeq, u16 qid)
{
	struct nvme_dev *dev = nvmeq->dev;

	nvmeq->sq_tail = 0;
	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	memset((void *)nvmeq->cqes, 0, NVME_CQ_SIZE(nvmeq->q_depth));
	flush_dcache_range((ulong)nvmeq->cqes,
			   (ulong)nvmeq->cqes + NVME_CQ_ALLOCATION);
	dev->online_queues++;
}

static int nvme_configure_admin_queue(struct nvme_dev *dev)
{
	int result;
	u32 aqa;
	u64 cap = dev->cap;
	struct nvme_queue *nvmeq;
	/* most architectures use 4KB as the page size */
	unsigned page_shift = 12;
	unsigned dev_page_min = NVME_CAP_MPSMIN(cap) + 12;
	unsigned dev_page_max = NVME_CAP_MPSMAX(cap) + 12;

	if (page_shift < dev_page_min) {
		debug("Device minimum page size (%u) too large for host (%u)\n",
		      1 << dev_page_min, 1 << page_shift);
		return -ENODEV;
	}

	if (page_shift > dev_page_max) {
		debug("Device maximum page size (%u) smaller than host (%u)\n",
		      1 << dev_page_max, 1 << page_shift);
		page_shift = dev_page_max;
	}

	result = nvme_disable_ctrl(dev);
	if (result < 0)
		return result;

	nvmeq = dev->queues[NVME_ADMIN_Q];
	if (!nvmeq) {
		nvmeq = nvme_alloc_queue(dev, 0, NVME_AQ_DEPTH);
		if (!nvmeq)
			return -ENOMEM;
	}

	aqa = nvmeq->q_depth - 1;
	aqa |= aqa << 16;

	dev->page_size = 1 << page_shift;

	dev->ctrl_config = NVME_CC_CSS_NVM;
	dev->ctrl_config |= (page_shift - 12) << NVME_CC_MPS_SHIFT;
	dev->ctrl_config |= NVME_CC_ARB_RR | NVME_CC_SHN_NONE;
	dev->ctrl_config |= NVME_CC_IOSQES | NVME_CC_IOCQES;

	writel(aqa, &dev->bar->aqa);
	nvme_writeq((ulong)nvmeq->sq_cmds, &dev->bar->asq);
	nvme_writeq((ulong)nvmeq->cqes, &dev->bar->acq);

	result = nvme_enable_ctrl(dev);
	if (result)
		goto free_nvmeq;

	nvmeq->cq_vector = 0;

	nvme_init_queue(dev->queues[NVME_ADMIN_Q], 0);

	return result;

 free_nvmeq:
	nvme_free_queues(dev, 0);

	return result;
}

static int nvme_alloc_cq(struct nvme_dev *dev, u16 qid,
			    struct nvme_queue *nvmeq)
{
	struct nvme_command c;
	int flags = NVME_QUEUE_PHYS_CONTIG | NVME_CQ_IRQ_ENABLED;

	memset(&c, 0, sizeof(c));
	c.create_cq.opcode = nvme_admin_create_cq;
	c.create_cq.prp1 = cpu_to_le64((ulong)nvmeq->cqes);
	c.create_cq.cqid = cpu_to_le16(qid);
	c.create_cq.qsize = cpu_to_le16(nvmeq->q_depth - 1);
	c.create_cq.cq_flags = cpu_to_le16(flags);
	c.create_cq.irq_vector = cpu_to_le16(nvmeq->cq_vector);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int nvme_alloc_sq(struct nvme_dev *dev, u16 qid,
			    struct nvme_queue *nvmeq)
{
	struct nvme_command c;
	int flags = NVME_QUEUE_PHYS_CONTIG | NVME_SQ_PRIO_MEDIUM;

	memset(&c, 0, sizeof(c));
	c.create_sq.opcode = nvme_admin_create_sq;
	c.create_sq.prp1 = cpu_to_le64((ulong)nvmeq->sq_cmds);
	c.create_sq.sqid = cpu_to_le16(qid);
	c.create_sq.qsize = cpu_to_le16(nvmeq->q_depth - 1);
	c.create_sq.sq_flags = cpu_to_le16(flags);
	c.create_sq.cqid = cpu_to_le16(qid);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

int nvme_identify(struct nvme_dev *dev, unsigned nsid,
		  unsigned cns, dma_addr_t dma_addr)
{
	struct nvme_command c;
	u32 page_size = dev->page_size;
	int offset = dma_addr & (page_size - 1);
	int length = sizeof(struct nvme_id_ctrl);
	int ret;

	memset(&c, 0, sizeof(c));
	c.identify.opcode = nvme_admin_identify;
	c.identify.nsid = cpu_to_le32(nsid);
	c.identify.prp1 = cpu_to_le64(dma_addr);

	length -= (page_size - offset);
	if (length <= 0) {
		c.identify.prp2 = 0;
	} else {
		dma_addr += (page_size - offset);
		c.identify.prp2 = cpu_to_le64(dma_addr);
	}

	c.identify.cns = cpu_to_le32(cns);

	invalidate_dcache_range(dma_addr,
				dma_addr + sizeof(struct nvme_id_ctrl));

	ret = nvme_submit_admin_cmd(dev, &c, NULL);
	if (!ret)
		invalidate_dcache_range(dma_addr,
					dma_addr + sizeof(struct nvme_id_ctrl));

	return ret;
}

int nvme_get_features(struct nvme_dev *dev, unsigned fid, unsigned nsid,
		      dma_addr_t dma_addr, u32 *result)
{
	struct nvme_command c;
	int ret;

	memset(&c, 0, sizeof(c));
	c.features.opcode = nvme_admin_get_features;
	c.features.nsid = cpu_to_le32(nsid);
	c.features.prp1 = cpu_to_le64(dma_addr);
	c.features.fid = cpu_to_le32(fid);

	ret = nvme_submit_admin_cmd(dev, &c, result);

	/*
	 * TODO: Add some cache invalidation when a DMA buffer is involved
	 * in the request, here and before the command gets submitted. The
	 * buffer size varies by feature, also some features use a different
	 * field in the command packet to hold the buffer address.
	 * Section 5.21.1 (Set Features command) in the NVMe specification
	 * details the buffer requirements for each feature.
	 *
	 * At the moment there is no user of this function.
	 */

	return ret;
}

int nvme_set_features(struct nvme_dev *dev, unsigned fid, unsigned dword11,
		      dma_addr_t dma_addr, u32 *result)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.features.opcode = nvme_admin_set_features;
	c.features.prp1 = cpu_to_le64(dma_addr);
	c.features.fid = cpu_to_le32(fid);
	c.features.dword11 = cpu_to_le32(dword11);

	/*
	 * TODO: Add a cache clean (aka flush) operation when a DMA buffer is
	 * involved in the request. The buffer size varies by feature, also
	 * some features use a different field in the command packet to hold
	 * the buffer address. Section 5.21.1 (Set Features command) in the
	 * NVMe specification details the buffer requirements for each
	 * feature.
	 * At the moment the only user of this function is not using
	 * any DMA buffer at all.
	 */

	return nvme_submit_admin_cmd(dev, &c, result);
}

static int nvme_create_queue(struct nvme_queue *nvmeq, int qid)
{
	struct nvme_dev *dev = nvmeq->dev;
	int result;

	nvmeq->cq_vector = qid - 1;
	result = nvme_alloc_cq(dev, qid, nvmeq);
	if (result < 0)
		goto release_cq;

	result = nvme_alloc_sq(dev, qid, nvmeq);
	if (result < 0)
		goto release_sq;

	nvme_init_queue(nvmeq, qid);

	return result;

 release_sq:
	nvme_delete_sq(dev, qid);
 release_cq:
	nvme_delete_cq(dev, qid);

	return result;
}

static int nvme_set_queue_count(struct nvme_dev *dev, int count)
{
	int status;
	u32 result;
	u32 q_count = (count - 1) | ((count - 1) << 16);

	status = nvme_set_features(dev, NVME_FEAT_NUM_QUEUES,
			q_count, 0, &result);

	if (status < 0)
		return status;
	if (status > 1)
		return 0;

	return min(result & 0xffff, result >> 16) + 1;
}

static int nvme_create_io_queues(struct nvme_dev *dev)
{
	unsigned int i;
	int ret;

	for (i = dev->queue_count; i <= dev->max_qid; i++)
		if (!nvme_alloc_queue(dev, i, dev->q_depth))
			return log_msg_ret("all", -ENOMEM);

	for (i = dev->online_queues; i <= dev->queue_count - 1; i++) {
		ret = nvme_create_queue(dev->queues[i], i);
		if (ret)
			return log_msg_ret("cre", ret);
	}

	return 0;
}

static int nvme_setup_io_queues(struct nvme_dev *dev)
{
	int nr_io_queues;
	int result;

	nr_io_queues = 1;
	result = nvme_set_queue_count(dev, nr_io_queues);
	if (result <= 0) {
		log_debug("Cannot set queue count (err=%dE)\n", result);
		return result;
	}

	dev->max_qid = nr_io_queues;

	/* Free previously allocated queues */
	nvme_free_queues(dev, nr_io_queues + 1);
	result = nvme_create_io_queues(dev);
	if (result)
		return result;

	return 0;
}

static int nvme_get_info_from_identify(struct nvme_dev *dev)
{
	struct nvme_id_ctrl *ctrl;
	int ret;
	int shift = NVME_CAP_MPSMIN(dev->cap) + 12;

	ctrl = memalign(dev->page_size, sizeof(struct nvme_id_ctrl));
	if (!ctrl)
		return -ENOMEM;

	ret = nvme_identify(dev, 0, 1, (dma_addr_t)(long)ctrl);
	if (ret) {
		free(ctrl);
		return -EIO;
	}

	dev->nn = le32_to_cpu(ctrl->nn);
	dev->vwc = ctrl->vwc;
	memcpy(dev->serial, ctrl->sn, sizeof(ctrl->sn));
	memcpy(dev->model, ctrl->mn, sizeof(ctrl->mn));
	memcpy(dev->firmware_rev, ctrl->fr, sizeof(ctrl->fr));
	if (ctrl->mdts)
		dev->max_transfer_shift = (ctrl->mdts + shift);
	else {
		/*
		 * Maximum Data Transfer Size (MDTS) field indicates the maximum
		 * data transfer size between the host and the controller. The
		 * host should not submit a command that exceeds this transfer
		 * size. The value is in units of the minimum memory page size
		 * and is reported as a power of two (2^n).
		 *
		 * The spec also says: a value of 0h indicates no restrictions
		 * on transfer size. But in nvme_blk_read/write() below we have
		 * the following algorithm for maximum number of logic blocks
		 * per transfer:
		 *
		 * u16 lbas = 1 << (dev->max_transfer_shift - ns->lba_shift);
		 *
		 * In order for lbas not to overflow, the maximum number is 15
		 * which means dev->max_transfer_shift = 15 + 9 (ns->lba_shift).
		 * Let's use 20 which provides 1MB size.
		 */
		dev->max_transfer_shift = 20;
	}

	free(ctrl);
	return 0;
}

int nvme_get_namespace_id(struct udevice *udev, u32 *ns_id, u8 *eui64)
{
	struct nvme_ns *ns = dev_get_priv(udev);

	if (ns_id)
		*ns_id = ns->ns_id;
	if (eui64)
		memcpy(eui64, ns->eui64, sizeof(ns->eui64));

	return 0;
}

int nvme_scan_namespace(void)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	ret = uclass_get(UCLASS_NVME, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		ret = device_probe(dev);
		if (ret) {
			log_err("Failed to probe '%s': err=%dE\n", dev->name,
				ret);
			/* Bail if we ran out of memory, else keep trying */
			if (ret != -EBUSY)
				return ret;
		}
	}

	return 0;
}

static int nvme_blk_probe(struct udevice *udev)
{
	struct nvme_dev *ndev = dev_get_priv(udev->parent);
	struct blk_desc *desc = dev_get_uclass_plat(udev);
	struct nvme_ns *ns = dev_get_priv(udev);
	u8 flbas;
	struct nvme_id_ns *id;

	id = memalign(ndev->page_size, sizeof(struct nvme_id_ns));
	if (!id)
		return -ENOMEM;

	ns->dev = ndev;
	/* extract the namespace id from the block device name */
	ns->ns_id = trailing_strtol(udev->name);
	if (nvme_identify(ndev, ns->ns_id, 0, (dma_addr_t)(long)id)) {
		free(id);
		return -EIO;
	}

	memcpy(&ns->eui64, &id->eui64, sizeof(id->eui64));
	flbas = id->flbas & NVME_NS_FLBAS_LBA_MASK;
	ns->flbas = flbas;
	ns->lba_shift = id->lbaf[flbas].ds;
	list_add(&ns->list, &ndev->namespaces);

	desc->lba = le64_to_cpu(id->nsze);
	desc->log2blksz = ns->lba_shift;
	desc->blksz = 1 << ns->lba_shift;
	desc->bdev = udev;
	memcpy(desc->vendor, ndev->vendor, sizeof(ndev->vendor));
	memcpy(desc->product, ndev->serial, sizeof(ndev->serial));
	memcpy(desc->revision, ndev->firmware_rev, sizeof(ndev->firmware_rev));

	free(id);
	return 0;
}

static ulong nvme_blk_rw(struct udevice *udev, lbaint_t blknr,
			 lbaint_t blkcnt, void *buffer, bool read)
{
	struct nvme_ns *ns = dev_get_priv(udev);
	struct nvme_dev *dev = ns->dev;
	struct nvme_command c;
	struct blk_desc *desc = dev_get_uclass_plat(udev);
	int status;
	u64 prp2;
	u64 total_len = blkcnt << desc->log2blksz;
	u64 temp_len = total_len;
	uintptr_t temp_buffer = (uintptr_t)buffer;

	u64 slba = blknr;
	u16 lbas = 1 << (dev->max_transfer_shift - ns->lba_shift);
	u64 total_lbas = blkcnt;

	flush_dcache_range((unsigned long)buffer,
			   (unsigned long)buffer + total_len);

	c.rw.opcode = read ? nvme_cmd_read : nvme_cmd_write;
	c.rw.flags = 0;
	c.rw.nsid = cpu_to_le32(ns->ns_id);
	c.rw.control = 0;
	c.rw.dsmgmt = 0;
	c.rw.reftag = 0;
	c.rw.apptag = 0;
	c.rw.appmask = 0;
	c.rw.metadata = 0;

	while (total_lbas) {
		if (total_lbas < lbas) {
			lbas = (u16)total_lbas;
			total_lbas = 0;
		} else {
			total_lbas -= lbas;
		}

		if (nvme_setup_prps(dev, &prp2,
				    lbas << ns->lba_shift, temp_buffer))
			return -EIO;
		c.rw.slba = cpu_to_le64(slba);
		slba += lbas;
		c.rw.length = cpu_to_le16(lbas - 1);
		c.rw.prp1 = cpu_to_le64(temp_buffer);
		c.rw.prp2 = cpu_to_le64(prp2);
		status = nvme_submit_sync_cmd(dev->queues[NVME_IO_Q],
				&c, NULL, IO_TIMEOUT);
		if (status)
			break;
		temp_len -= (u32)lbas << ns->lba_shift;
		temp_buffer += lbas << ns->lba_shift;
	}

	if (read)
		invalidate_dcache_range((unsigned long)buffer,
					(unsigned long)buffer + total_len);

	return (total_len - temp_len) >> desc->log2blksz;
}

static ulong nvme_blk_read(struct udevice *udev, lbaint_t blknr,
			   lbaint_t blkcnt, void *buffer)
{
	return nvme_blk_rw(udev, blknr, blkcnt, buffer, true);
}

static ulong nvme_blk_write(struct udevice *udev, lbaint_t blknr,
			    lbaint_t blkcnt, const void *buffer)
{
	return nvme_blk_rw(udev, blknr, blkcnt, (void *)buffer, false);
}

static const struct blk_ops nvme_blk_ops = {
	.read	= nvme_blk_read,
	.write	= nvme_blk_write,
};

U_BOOT_DRIVER(nvme_blk) = {
	.name	= "nvme-blk",
	.id	= UCLASS_BLK,
	.probe	= nvme_blk_probe,
	.ops	= &nvme_blk_ops,
	.priv_auto	= sizeof(struct nvme_ns),
};

int nvme_init(struct udevice *udev)
{
	struct nvme_dev *ndev = dev_get_priv(udev);
	struct nvme_id_ns *id;
	int ret;

	ndev->udev = udev;
	INIT_LIST_HEAD(&ndev->namespaces);
	if (readl(&ndev->bar->csts) == -1) {
		ret = -EBUSY;
		printf("Error: %s: Controller not ready!\n", udev->name);
		goto free_nvme;
	}

	ndev->queues = malloc(NVME_Q_NUM * sizeof(struct nvme_queue *));
	if (!ndev->queues) {
		ret = -ENOMEM;
		printf("Error: %s: Out of memory!\n", udev->name);
		goto free_nvme;
	}
	memset(ndev->queues, 0, NVME_Q_NUM * sizeof(struct nvme_queue *));

	ndev->cap = nvme_readq(&ndev->bar->cap);
	ndev->q_depth = min_t(int, NVME_CAP_MQES(ndev->cap) + 1, NVME_Q_DEPTH);
	ndev->db_stride = 1 << NVME_CAP_STRIDE(ndev->cap);
	ndev->dbs = ((void __iomem *)ndev->bar) + 4096;

	ret = nvme_configure_admin_queue(ndev);
	if (ret) {
		log_debug("Unable to configure admin queue (err=%dE)\n", ret);
		goto free_queue;
	}

	/* Allocate after the page size is known */
	ndev->prp_pool = memalign(ndev->page_size, MAX_PRP_POOL);
	if (!ndev->prp_pool) {
		ret = -ENOMEM;
		printf("Error: %s: Out of memory!\n", udev->name);
		goto free_nvme;
	}
	ndev->prp_entry_num = MAX_PRP_POOL >> 3;

	ret = nvme_setup_io_queues(ndev);
	if (ret) {
		log_debug("Unable to setup I/O queues(err=%dE)\n", ret);
		goto free_queue;
	}

	nvme_get_info_from_identify(ndev);

	/* Create a blk device for each namespace */

	id = memalign(ndev->page_size, sizeof(struct nvme_id_ns));
	if (!id) {
		ret = -ENOMEM;
		goto free_queue;
	}

	for (int i = 1; i <= ndev->nn; i++) {
		struct udevice *ns_udev;
		char name[20];

		memset(id, 0, sizeof(*id));
		if (nvme_identify(ndev, i, 0, (dma_addr_t)(long)id)) {
			ret = -EIO;
			goto free_id;
		}

		/* skip inactive namespace */
		if (!id->nsze)
			continue;

		/*
		 * Encode the namespace id to the device name so that
		 * we can extract it when doing the probe.
		 */
		sprintf(name, "blk#%d", i);

		/* The real blksz and size will be set by nvme_blk_probe() */
		ret = blk_create_devicef(udev, "nvme-blk", name, UCLASS_NVME,
					 -1, DEFAULT_BLKSZ, 0, &ns_udev);
		if (ret)
			goto free_id;

		ret = bootdev_setup_for_sibling_blk(ns_udev, "nvme_bootdev");
		if (ret)
			return log_msg_ret("bootdev", ret);

		ret = blk_probe_or_unbind(ns_udev);
		if (ret)
			goto free_id;
	}

	free(id);
	return 0;

free_id:
	free(id);
free_queue:
	free((void *)ndev->queues);
free_nvme:
	return ret;
}

int nvme_shutdown(struct udevice *udev)
{
	struct nvme_dev *ndev = dev_get_priv(udev);
	int ret;

	ret = nvme_shutdown_ctrl(ndev);
	if (ret < 0) {
		printf("Error: %s: Shutdown timed out!\n", udev->name);
		return ret;
	}

	return nvme_disable_ctrl(ndev);
}
