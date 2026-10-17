// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Xilinx Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <dma.h>
#include <dma-uclass.h>
#include <errno.h>
#include <dm/device_compat.h>

/**
 * struct zynqmp_dpdma_priv - Private structure
 * @dev: Device uclass for video_ops
 */
struct zynqmp_dpdma_priv {
	struct udevice *dev;
};

static int zynqmp_dpdma_probe(struct udevice *dev)
{
	/* Only placeholder for power domain driver */
	return 0;
}

static const struct dma_ops zynqmp_dpdma_ops = {
};

static const struct udevice_id zynqmp_dpdma_ids[] = {
	{ .compatible = "xlnx,zynqmp-dpdma" },
	{ }
};

U_BOOT_DRIVER(zynqmp_dpdma) = {
	.name = "zynqmp_dpdma",
	.id = UCLASS_DMA,
	.of_match = zynqmp_dpdma_ids,
	.ops = &zynqmp_dpdma_ops,
	.probe = zynqmp_dpdma_probe,
	.priv_auto = sizeof(struct zynqmp_dpdma_priv),
};
