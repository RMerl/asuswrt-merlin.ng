// SPDX-License-Identifier: GPL-2.0+
/*
 * eFuse driver for Rockchip devices
 *
 * Copyright 2017, Theobroma Systems Design und Consulting GmbH
 * Written by Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#include <common.h>
#include <asm/io.h>
#include <command.h>
#include <display_options.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <misc.h>

#define RK3399_A_SHIFT          16
#define RK3399_A_MASK           0x3ff
#define RK3399_NFUSES           32
#define RK3399_BYTES_PER_FUSE   4
#define RK3399_STROBSFTSEL      BIT(9)
#define RK3399_RSB              BIT(7)
#define RK3399_PD               BIT(5)
#define RK3399_PGENB            BIT(3)
#define RK3399_LOAD             BIT(2)
#define RK3399_STROBE           BIT(1)
#define RK3399_CSB              BIT(0)

struct rockchip_efuse_regs {
	u32 ctrl;      /* 0x00  efuse control register */
	u32 dout;      /* 0x04  efuse data out register */
	u32 rf;        /* 0x08  efuse redundancy bit used register */
	u32 _rsvd0;
	u32 jtag_pass; /* 0x10  JTAG password */
	u32 strobe_finish_ctrl;
		       /* 0x14	efuse strobe finish control register */
};

struct rockchip_efuse_platdata {
	void __iomem *base;
	struct clk *clk;
};

#if defined(DEBUG)
static int dump_efuses(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
{
	/*
	 * N.B.: This function is tailored towards the RK3399 and assumes that
	 *       there's always 32 fuses x 32 bits (i.e. 128 bytes of data) to
	 *       be read.
	 */

	struct udevice *dev;
	u8 fuses[128];
	int ret;

	/* retrieve the device */
	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_GET_DRIVER(rockchip_efuse), &dev);
	if (ret) {
		printf("%s: no misc-device found\n", __func__);
		return 0;
	}

	ret = misc_read(dev, 0, &fuses, sizeof(fuses));
	if (ret < 0) {
		printf("%s: misc_read failed\n", __func__);
		return 0;
	}

	printf("efuse-contents:\n");
	print_buffer(0, fuses, 1, 128, 16);

	return 0;
}

U_BOOT_CMD(
	rk3399_dump_efuses, 1, 1, dump_efuses,
	"Dump the content of the efuses",
	""
);
#endif

static int rockchip_rk3399_efuse_read(struct udevice *dev, int offset,
				      void *buf, int size)
{
	struct rockchip_efuse_platdata *plat = dev_get_platdata(dev);
	struct rockchip_efuse_regs *efuse =
		(struct rockchip_efuse_regs *)plat->base;

	unsigned int addr_start, addr_end, addr_offset;
	u32 out_value;
	u8  bytes[RK3399_NFUSES * RK3399_BYTES_PER_FUSE];
	int i = 0;
	u32 addr;

	addr_start = offset / RK3399_BYTES_PER_FUSE;
	addr_offset = offset % RK3399_BYTES_PER_FUSE;
	addr_end = DIV_ROUND_UP(offset + size, RK3399_BYTES_PER_FUSE);

	/* cap to the size of the efuse block */
	if (addr_end > RK3399_NFUSES)
		addr_end = RK3399_NFUSES;

	writel(RK3399_LOAD | RK3399_PGENB | RK3399_STROBSFTSEL | RK3399_RSB,
	       &efuse->ctrl);
	udelay(1);
	for (addr = addr_start; addr < addr_end; addr++) {
		setbits_le32(&efuse->ctrl,
			     RK3399_STROBE | (addr << RK3399_A_SHIFT));
		udelay(1);
		out_value = readl(&efuse->dout);
		clrbits_le32(&efuse->ctrl, RK3399_STROBE);
		udelay(1);

		memcpy(&bytes[i], &out_value, RK3399_BYTES_PER_FUSE);
		i += RK3399_BYTES_PER_FUSE;
	}

	/* Switch to standby mode */
	writel(RK3399_PD | RK3399_CSB, &efuse->ctrl);

	memcpy(buf, bytes + addr_offset, size);

	return 0;
}

static int rockchip_efuse_read(struct udevice *dev, int offset,
			       void *buf, int size)
{
	return rockchip_rk3399_efuse_read(dev, offset, buf, size);
}

static const struct misc_ops rockchip_efuse_ops = {
	.read = rockchip_efuse_read,
};

static int rockchip_efuse_ofdata_to_platdata(struct udevice *dev)
{
	struct rockchip_efuse_platdata *plat = dev_get_platdata(dev);

	plat->base = dev_read_addr_ptr(dev);
	return 0;
}

static const struct udevice_id rockchip_efuse_ids[] = {
	{ .compatible = "rockchip,rk3399-efuse" },
	{}
};

U_BOOT_DRIVER(rockchip_efuse) = {
	.name = "rockchip_efuse",
	.id = UCLASS_MISC,
	.of_match = rockchip_efuse_ids,
	.ofdata_to_platdata = rockchip_efuse_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct rockchip_efuse_platdata),
	.ops = &rockchip_efuse_ops,
};
