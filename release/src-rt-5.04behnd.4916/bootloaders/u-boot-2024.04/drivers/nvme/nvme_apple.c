// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>
#include <mailbox.h>
#include <mapmem.h>
#include "nvme.h"
#include <reset.h>

#include <asm/io.h>
#include <asm/arch/rtkit.h>
#include <asm/arch/sart.h>
#include <linux/iopoll.h>

/* ASC registers */
#define REG_CPU_CTRL		0x0044
#define  REG_CPU_CTRL_RUN	BIT(4)

/* Apple NVMe registers */
#define ANS_MAX_PEND_CMDS_CTRL	0x01210
#define  ANS_MAX_QUEUE_DEPTH	64
#define ANS_BOOT_STATUS		0x01300
#define  ANS_BOOT_STATUS_OK	0xde71ce55
#define ANS_MODESEL		0x01304
#define ANS_UNKNOWN_CTRL	0x24008
#define  ANS_PRP_NULL_CHECK	(1 << 11)
#define ANS_LINEAR_SQ_CTRL	0x24908
#define  ANS_LINEAR_SQ_CTRL_EN	(1 << 0)
#define ANS_ASQ_DB		0x2490c
#define ANS_IOSQ_DB		0x24910
#define ANS_NVMMU_NUM		0x28100
#define ANS_NVMMU_BASE_ASQ	0x28108
#define ANS_NVMMU_BASE_IOSQ	0x28110
#define ANS_NVMMU_TCB_INVAL	0x28118
#define ANS_NVMMU_TCB_STAT	0x28120

#define ANS_NVMMU_TCB_SIZE	0x4000
#define ANS_NVMMU_TCB_PITCH	0x80

/*
 * The Apple NVMe controller includes an IOMMU known as NVMMU.  The
 * NVMMU is programmed through an array of TCBs. These TCBs are paired
 * with the corresponding slot in the submission queues and need to be
 * configured with the command details before a command is allowed to
 * execute. This is necessary even for commands that don't do DMA.
 */
struct ans_nvmmu_tcb {
	u8 opcode;
	u8 flags;
	u8 slot;
	u8 pad0;
	u32 prpl_len;
	u8 pad1[16];
	u64 prp1;
	u64 prp2;
};

#define ANS_NVMMU_TCB_WRITE	BIT(0)
#define ANS_NVMMU_TCB_READ	BIT(1)

struct apple_nvme_priv {
	struct nvme_dev ndev;
	void *base;		/* NVMe registers */
	void *asc;		/* ASC registers */
	struct reset_ctl_bulk resets; /* ASC reset */
	struct mbox_chan chan;
	struct apple_sart *sart;
	struct apple_rtkit *rtk;
	struct ans_nvmmu_tcb *tcbs[NVME_Q_NUM]; /* Submission queue TCBs */
	u32 __iomem *q_db[NVME_Q_NUM]; /* Submission queue doorbell */
};

static int apple_nvme_setup_queue(struct nvme_queue *nvmeq)
{
	struct apple_nvme_priv *priv =
		container_of(nvmeq->dev, struct apple_nvme_priv, ndev);
	struct nvme_dev *dev = nvmeq->dev;

	switch (nvmeq->qid) {
	case NVME_ADMIN_Q:
	case NVME_IO_Q:
		break;
	default:
		return -EINVAL;
	}

	priv->tcbs[nvmeq->qid] = (void *)memalign(4096, ANS_NVMMU_TCB_SIZE);
	memset((void *)priv->tcbs[nvmeq->qid], 0, ANS_NVMMU_TCB_SIZE);

	switch (nvmeq->qid) {
	case NVME_ADMIN_Q:
		priv->q_db[nvmeq->qid] =
			((void __iomem *)dev->bar) + ANS_ASQ_DB;
		nvme_writeq((ulong)priv->tcbs[nvmeq->qid],
			    ((void __iomem *)dev->bar) + ANS_NVMMU_BASE_ASQ);
		break;
	case NVME_IO_Q:
		priv->q_db[nvmeq->qid] =
			((void __iomem *)dev->bar) + ANS_IOSQ_DB;
		nvme_writeq((ulong)priv->tcbs[nvmeq->qid],
			    ((void __iomem *)dev->bar) + ANS_NVMMU_BASE_IOSQ);
		break;
	}

	return 0;
}

static void apple_nvme_submit_cmd(struct nvme_queue *nvmeq,
				  struct nvme_command *cmd)
{
	struct apple_nvme_priv *priv =
		container_of(nvmeq->dev, struct apple_nvme_priv, ndev);
	struct ans_nvmmu_tcb *tcb;
	u16 tail = nvmeq->sq_tail;

	tcb = ((void *)priv->tcbs[nvmeq->qid]) + tail * ANS_NVMMU_TCB_PITCH;
	memset(tcb, 0, sizeof(*tcb));
	tcb->opcode = cmd->common.opcode;
	tcb->flags = ANS_NVMMU_TCB_WRITE | ANS_NVMMU_TCB_READ;
	tcb->slot = tail;
	tcb->prpl_len = cmd->rw.length;
	tcb->prp1 = cmd->common.prp1;
	tcb->prp2 = cmd->common.prp2;

	writel(tail, priv->q_db[nvmeq->qid]);
}

static void apple_nvme_complete_cmd(struct nvme_queue *nvmeq,
				    struct nvme_command *cmd)
{
	struct apple_nvme_priv *priv =
		container_of(nvmeq->dev, struct apple_nvme_priv, ndev);
	struct ans_nvmmu_tcb *tcb;
	u16 tail = nvmeq->sq_tail;

	tcb = ((void *)priv->tcbs[nvmeq->qid]) + tail * ANS_NVMMU_TCB_PITCH;
	memset(tcb, 0, sizeof(*tcb));
	writel(tail, ((void __iomem *)nvmeq->dev->bar) + ANS_NVMMU_TCB_INVAL);
	readl(((void __iomem *)nvmeq->dev->bar) + ANS_NVMMU_TCB_STAT);

	if (++tail == nvmeq->q_depth)
		tail = 0;
	nvmeq->sq_tail = tail;
}

static int nvme_shmem_setup(void *cookie, struct apple_rtkit_buffer *buf)
{
	struct apple_nvme_priv *priv = (struct apple_nvme_priv *)cookie;

	if (!buf || buf->dva || !buf->size)
		return -1;

	buf->buffer = memalign(SZ_16K, ALIGN(buf->size, SZ_16K));
	if (!buf->buffer)
		return -ENOMEM;

	if (!sart_add_allowed_region(priv->sart, buf->buffer, buf->size)) {
		free(buf->buffer);
		buf->buffer = NULL;
		buf->size = 0;
		return -1;
	}

	buf->dva = (u64)buf->buffer;

	return 0;
}

static void nvme_shmem_destroy(void *cookie, struct apple_rtkit_buffer *buf)
{
	struct apple_nvme_priv *priv = (struct apple_nvme_priv *)cookie;

	if (!buf)
		return;

	if (buf->buffer) {
		sart_remove_allowed_region(priv->sart, buf->buffer, buf->size);
		free(buf->buffer);
		buf->buffer = NULL;
		buf->size = 0;
		buf->dva = 0;
	}
}

static int apple_nvme_probe(struct udevice *dev)
{
	struct apple_nvme_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	ofnode of_sart;
	u32 ctrl, stat, phandle;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	addr = dev_read_addr_index(dev, 1);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	priv->asc = map_sysmem(addr, 0);

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret < 0)
		return ret;

	ret = mbox_get_by_index(dev, 0, &priv->chan);
	if (ret < 0)
		return ret;

	ret = dev_read_u32(dev, "apple,sart", &phandle);
	if (ret < 0)
		return ret;

	of_sart = ofnode_get_by_phandle(phandle);
	priv->sart = sart_init(of_sart);
	if (!priv->sart)
		return -EINVAL;

	ctrl = readl(priv->asc + REG_CPU_CTRL);
	writel(ctrl | REG_CPU_CTRL_RUN, priv->asc + REG_CPU_CTRL);

	priv->rtk = apple_rtkit_init(&priv->chan, priv, nvme_shmem_setup, nvme_shmem_destroy);
	if (!priv->rtk)
		return -ENOMEM;

	ret = apple_rtkit_boot(priv->rtk);
	if (ret < 0) {
		printf("%s: NVMe apple_rtkit_boot returned: %d\n", __func__, ret);
		return ret;
	}

	ret = readl_poll_sleep_timeout(priv->base + ANS_BOOT_STATUS, stat,
				       (stat == ANS_BOOT_STATUS_OK), 100,
				       500000);
	if (ret < 0) {
		printf("%s: NVMe firmware didn't boot\n", __func__);
		return -ETIMEDOUT;
	}

	writel(ANS_LINEAR_SQ_CTRL_EN, priv->base + ANS_LINEAR_SQ_CTRL);
	writel(((ANS_MAX_QUEUE_DEPTH << 16) | ANS_MAX_QUEUE_DEPTH),
	       priv->base + ANS_MAX_PEND_CMDS_CTRL);

	writel(readl(priv->base + ANS_UNKNOWN_CTRL) & ~ANS_PRP_NULL_CHECK,
	       priv->base + ANS_UNKNOWN_CTRL);

	strcpy(priv->ndev.vendor, "Apple");

	writel((ANS_NVMMU_TCB_SIZE / ANS_NVMMU_TCB_PITCH) - 1,
	       priv->base + ANS_NVMMU_NUM);
	writel(0, priv->base + ANS_MODESEL);

	priv->ndev.bar = priv->base;
	return nvme_init(dev);
}

static int apple_nvme_remove(struct udevice *dev)
{
	struct apple_nvme_priv *priv = dev_get_priv(dev);
	u32 ctrl;

	nvme_shutdown(dev);

	apple_rtkit_shutdown(priv->rtk, APPLE_RTKIT_PWR_STATE_SLEEP);

	ctrl = readl(priv->asc + REG_CPU_CTRL);
	writel(ctrl & ~REG_CPU_CTRL_RUN, priv->asc + REG_CPU_CTRL);

	apple_rtkit_free(priv->rtk);
	priv->rtk = NULL;

	sart_free(priv->sart);
	priv->sart = NULL;

	reset_assert_bulk(&priv->resets);
	reset_deassert_bulk(&priv->resets);

	return 0;
}

static const struct nvme_ops apple_nvme_ops = {
	.setup_queue = apple_nvme_setup_queue,
	.submit_cmd = apple_nvme_submit_cmd,
	.complete_cmd = apple_nvme_complete_cmd,
};

static const struct udevice_id apple_nvme_ids[] = {
	{ .compatible = "apple,nvme-ans2" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(apple_nvme) = {
	.name = "apple_nvme",
	.id = UCLASS_NVME,
	.of_match = apple_nvme_ids,
	.priv_auto = sizeof(struct apple_nvme_priv),
	.probe = apple_nvme_probe,
	.remove = apple_nvme_remove,
	.ops = &apple_nvme_ops,
	.flags = DM_FLAG_OS_PREPARE,
};
