// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 Advanced Micro Devices, Inc
 * Michal Simek <michal.simek@amd.com>
 *
 * (C) Copyright 2007 Michal Simek
 * Michal SIMEK <monstr@monstr.eu>
 */

#include <common.h>
#include <dm.h>
#include <timer.h>
#include <regmap.h>
#include <dm/device_compat.h>

#define TIMER_ENABLE_ALL    0x400 /* ENALL */
#define TIMER_PWM           0x200 /* PWMA0 */
#define TIMER_INTERRUPT     0x100 /* T0INT */
#define TIMER_ENABLE        0x080 /* ENT0 */
#define TIMER_ENABLE_INTR   0x040 /* ENIT0 */
#define TIMER_RESET         0x020 /* LOAD0 */
#define TIMER_RELOAD        0x010 /* ARHT0 */
#define TIMER_EXT_CAPTURE   0x008 /* CAPT0 */
#define TIMER_EXT_COMPARE   0x004 /* GENT0 */
#define TIMER_DOWN_COUNT    0x002 /* UDT0 */
#define TIMER_CAPTURE_MODE  0x001 /* MDT0 */

#define TIMER_CONTROL_OFFSET	0
#define TIMER_LOADREG_OFFSET	4
#define TIMER_COUNTER_OFFSET	8

struct xilinx_timer_priv {
	struct regmap *regs;
};

static u64 xilinx_timer_get_count(struct udevice *dev)
{
	struct xilinx_timer_priv *priv = dev_get_priv(dev);
	u32 value;

	regmap_read(priv->regs, TIMER_COUNTER_OFFSET, &value);

	return timer_conv_64(value);
}

static int xilinx_timer_probe(struct udevice *dev)
{
	struct xilinx_timer_priv *priv = dev_get_priv(dev);
	int ret;

	/* uc_priv->clock_rate has already clock rate */
	ret = regmap_init_mem(dev_ofnode(dev), &priv->regs);
	if (ret) {
		dev_dbg(dev, "failed to get regbase of timer\n");
		return ret;
	}

	regmap_write(priv->regs, TIMER_LOADREG_OFFSET, 0);
	regmap_write(priv->regs, TIMER_CONTROL_OFFSET, TIMER_RESET);
	regmap_write(priv->regs, TIMER_CONTROL_OFFSET,
		     TIMER_ENABLE | TIMER_RELOAD);

	return 0;
}

static const struct timer_ops xilinx_timer_ops = {
	.get_count = xilinx_timer_get_count,
};

static const struct udevice_id xilinx_timer_ids[] = {
	{ .compatible = "xlnx,xps-timer-1.00.a" },
	{}
};

U_BOOT_DRIVER(xilinx_timer) = {
	.name = "xilinx_timer",
	.id = UCLASS_TIMER,
	.of_match = xilinx_timer_ids,
	.priv_auto = sizeof(struct xilinx_timer_priv),
	.probe = xilinx_timer_probe,
	.ops = &xilinx_timer_ops,
};
