// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx Zynq MPSoC Mailbox driver
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.
 */

#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/system.h>
#include <dm.h>
#include <mailbox-uclass.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <linux/arm-smccc.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <wait_bit.h>
#include <zynqmp_firmware.h>

/* IPI bitmasks, register base */
/* TODO: move reg base to DT */
#define IPI_BIT_MASK_PMU0     0x10000
#define IPI_INT_REG_BASE_APU  0xFF300000

/* IPI agent ID any */
#define IPI_ID_ANY 0xFFUL

/* indicate if ZynqMP IPI mailbox driver uses SMC calls or HVC calls */
#define USE_SMC 0

/* Default IPI SMC function IDs */
#define SMC_IPI_MAILBOX_OPEN		0x82001000U
#define SMC_IPI_MAILBOX_RELEASE		0x82001001U
#define SMC_IPI_MAILBOX_STATUS_ENQUIRY	0x82001002U
#define SMC_IPI_MAILBOX_NOTIFY		0x82001003U
#define SMC_IPI_MAILBOX_ACK		0x82001004U
#define SMC_IPI_MAILBOX_ENABLE_IRQ	0x82001005U
#define SMC_IPI_MAILBOX_DISABLE_IRQ	0x82001006U

/* IPI SMC Macros */

/*
 * Flag to indicate if notification interrupt
 * to be disabled.
 */
#define IPI_SMC_ENQUIRY_DIRQ_MASK	BIT(0)

/*
 * Flag to indicate if notification interrupt
 * to be enabled.
 */
#define IPI_SMC_ACK_EIRQ_MASK		BIT(0)

/* IPI mailbox status */
#define IPI_MB_STATUS_IDLE		0
#define IPI_MB_STATUS_SEND_PENDING	1
#define IPI_MB_STATUS_RECV_PENDING	2

#define IPI_MB_CHNL_TX	0 /* IPI mailbox TX channel */
#define IPI_MB_CHNL_RX	1 /* IPI mailbox RX channel */

struct ipi_int_regs {
	u32 trig; /* 0x0  */
	u32 obs;  /* 0x4  */
	u32 dummy0;
	u32 dummy1;
	u32 isr;  /* 0x10  */
	u32 imr;  /* 0x14  */
	u32 ier;  /* 0x18 */
	u32 idr;  /* 0x1C */
};

#define ipi_int_apu ((struct ipi_int_regs *)IPI_INT_REG_BASE_APU)

struct zynqmp_ipi {
	void __iomem *local_req_regs;
	void __iomem *local_res_regs;
	void __iomem *remote_req_regs;
	void __iomem *remote_res_regs;
	u32 remote_id;
	u32 local_id;
	bool el3_supported;
};

static int zynqmp_ipi_fw_call(struct zynqmp_ipi *ipi_mbox,
			      unsigned long a0, unsigned long a3)
{
	struct arm_smccc_res res = {0};
	unsigned long a1, a2;

	a1 = ipi_mbox->local_id;
	a2 = ipi_mbox->remote_id;
	arm_smccc_smc(a0, a1, a2, a3, 0, 0, 0, 0, &res);

	return (int)res.a0;
}

static int zynqmp_ipi_send(struct mbox_chan *chan, const void *data)
{
	const struct zynqmp_ipi_msg *msg = (struct zynqmp_ipi_msg *)data;
	struct zynqmp_ipi *zynqmp = dev_get_priv(chan->dev);
	u32 ret;
	u32 *mbx = (u32 *)zynqmp->local_req_regs;

	for (size_t i = 0; i < msg->len; i++)
		writel(msg->buf[i], &mbx[i]);

	/* Use SMC calls for Exception Level less than 3 where TF-A is available */
	if (!IS_ENABLED(CONFIG_SPL_BUILD) && current_el() < 3) {
		ret = zynqmp_ipi_fw_call(zynqmp, SMC_IPI_MAILBOX_NOTIFY, 0);

		debug("%s, send %ld bytes\n", __func__, msg->len);

		return ret;
	}

	/* Return if EL3 is not supported */
	if (!zynqmp->el3_supported) {
		dev_err(chan->dev, "mailbox in EL3 only supported for zynqmp");
		return -EOPNOTSUPP;
	}

	/* Write trigger interrupt */
	writel(IPI_BIT_MASK_PMU0, &ipi_int_apu->trig);

	/* Wait until observation bit is cleared */
	ret = wait_for_bit_le32(&ipi_int_apu->obs, IPI_BIT_MASK_PMU0, false,
				1000, false);

	debug("%s, send %ld bytes\n", __func__, msg->len);
	return ret;
};

static int zynqmp_ipi_recv(struct mbox_chan *chan, void *data)
{
	struct zynqmp_ipi_msg *msg = (struct zynqmp_ipi_msg *)data;
	struct zynqmp_ipi *zynqmp = dev_get_priv(chan->dev);
	u32 *mbx = (u32 *)zynqmp->local_res_regs;
	int ret = 0;

	/*
	 * PMU Firmware does not trigger IPI interrupt for API call responses so
	 * there is no need to check ISR flags for EL3.
	 */
	for (size_t i = 0; i < msg->len; i++)
		msg->buf[i] = readl(&mbx[i]);

	/* Ack to remote if EL is not 3 */
	if (!IS_ENABLED(CONFIG_SPL_BUILD) && current_el() < 3) {
		ret = zynqmp_ipi_fw_call(zynqmp, SMC_IPI_MAILBOX_ACK,
					 IPI_SMC_ACK_EIRQ_MASK);
	}

	debug("%s, recv %ld bytes\n", __func__, msg->len);
	return ret;
};

static int zynqmp_ipi_dest_probe(struct udevice *dev)
{
	struct zynqmp_ipi *zynqmp = dev_get_priv(dev);
	struct resource res;
	ofnode node;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	node = dev_ofnode(dev);

	if (IS_ENABLED(CONFIG_SPL_BUILD) || of_machine_is_compatible("xlnx,zynqmp"))
		zynqmp->el3_supported = true;

	ret = dev_read_u32(dev->parent, "xlnx,ipi-id", &zynqmp->local_id);
	if (ret) {
		dev_err(dev, "can't get local ipi id\n");
		return ret;
	}

	ret = ofnode_read_u32(node, "xlnx,ipi-id", &zynqmp->remote_id);
	if (ret) {
		dev_err(dev, "can't get remote ipi id\n");
		return ret;
	}

	if (ofnode_read_resource_byname(node, "local_request_region", &res)) {
		dev_err(dev, "No reg property for local_request_region\n");
		return -EINVAL;
	};
	zynqmp->local_req_regs = devm_ioremap(dev, res.start,
					      (res.start - res.end));
	if (!zynqmp->local_req_regs)
		return -EINVAL;

	if (ofnode_read_resource_byname(node, "local_response_region", &res)) {
		dev_err(dev, "No reg property for local_response_region\n");
		return -EINVAL;
	};
	zynqmp->local_res_regs = devm_ioremap(dev, res.start,
					      (res.start - res.end));
	if (!zynqmp->local_res_regs)
		return -EINVAL;

	if (ofnode_read_resource_byname(node, "remote_request_region", &res)) {
		dev_err(dev, "No reg property for remote_request_region\n");
		return -EINVAL;
	};
	zynqmp->remote_req_regs = devm_ioremap(dev, res.start,
					       (res.start - res.end));
	if (!zynqmp->remote_req_regs)
		return -EINVAL;

	if (ofnode_read_resource_byname(node, "remote_response_region", &res)) {
		dev_err(dev, "No reg property for remote_response_region\n");
		return -EINVAL;
	};
	zynqmp->remote_res_regs = devm_ioremap(dev, res.start,
					       (res.start - res.end));
	if (!zynqmp->remote_res_regs)
		return -EINVAL;

	return 0;
};

static int zynqmp_ipi_probe(struct udevice *dev)
{
	struct udevice *cdev;
	ofnode cnode;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	dev_for_each_subnode(cnode, dev) {
		ret = device_bind_driver_to_node(dev, "zynqmp_ipi_dest",
						 ofnode_get_name(cnode),
						 cnode, &cdev);
		if (ret)
			return ret;
	}

	return 0;
};

struct mbox_ops zynqmp_ipi_dest_mbox_ops = {
	.send = zynqmp_ipi_send,
	.recv = zynqmp_ipi_recv,
};

static const struct udevice_id zynqmp_ipi_dest_ids[] = {
	{ .compatible = "xlnx,zynqmp-ipi-dest-mailbox" },
	{ }
};

U_BOOT_DRIVER(zynqmp_ipi_dest) = {
	.name = "zynqmp_ipi_dest",
	.id = UCLASS_MAILBOX,
	.of_match = zynqmp_ipi_dest_ids,
	.probe = zynqmp_ipi_dest_probe,
	.priv_auto = sizeof(struct zynqmp_ipi),
	.ops = &zynqmp_ipi_dest_mbox_ops,
};

static const struct udevice_id zynqmp_ipi_ids[] = {
	{ .compatible = "xlnx,zynqmp-ipi-mailbox" },
	{ }
};

U_BOOT_DRIVER(zynqmp_ipi) = {
	.name = "zynqmp_ipi",
	.id = UCLASS_NOP,
	.of_match = zynqmp_ipi_ids,
	.probe = zynqmp_ipi_probe,
	.flags = DM_FLAG_PROBE_AFTER_BIND,
};
