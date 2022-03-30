// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Google, Inc
 */

#include <common.h>
#include <bios_emul.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <pci_rom.h>
#include <vbe.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>
#include <asm/arch/sandybridge.h>

DECLARE_GLOBAL_DATA_PTR;

struct gt_powermeter {
	u16 reg;
	u32 value;
};

/* These are magic values - unfortunately the meaning is unknown */
static const struct gt_powermeter snb_pm_gt1[] = {
	{ 0xa200, 0xcc000000 },
	{ 0xa204, 0x07000040 },
	{ 0xa208, 0x0000fe00 },
	{ 0xa20c, 0x00000000 },
	{ 0xa210, 0x17000000 },
	{ 0xa214, 0x00000021 },
	{ 0xa218, 0x0817fe19 },
	{ 0xa21c, 0x00000000 },
	{ 0xa220, 0x00000000 },
	{ 0xa224, 0xcc000000 },
	{ 0xa228, 0x07000040 },
	{ 0xa22c, 0x0000fe00 },
	{ 0xa230, 0x00000000 },
	{ 0xa234, 0x17000000 },
	{ 0xa238, 0x00000021 },
	{ 0xa23c, 0x0817fe19 },
	{ 0xa240, 0x00000000 },
	{ 0xa244, 0x00000000 },
	{ 0xa248, 0x8000421e },
	{ 0 }
};

static const struct gt_powermeter snb_pm_gt2[] = {
	{ 0xa200, 0x330000a6 },
	{ 0xa204, 0x402d0031 },
	{ 0xa208, 0x00165f83 },
	{ 0xa20c, 0xf1000000 },
	{ 0xa210, 0x00000000 },
	{ 0xa214, 0x00160016 },
	{ 0xa218, 0x002a002b },
	{ 0xa21c, 0x00000000 },
	{ 0xa220, 0x00000000 },
	{ 0xa224, 0x330000a6 },
	{ 0xa228, 0x402d0031 },
	{ 0xa22c, 0x00165f83 },
	{ 0xa230, 0xf1000000 },
	{ 0xa234, 0x00000000 },
	{ 0xa238, 0x00160016 },
	{ 0xa23c, 0x002a002b },
	{ 0xa240, 0x00000000 },
	{ 0xa244, 0x00000000 },
	{ 0xa248, 0x8000421e },
	{ 0 }
};

static const struct gt_powermeter ivb_pm_gt1[] = {
	{ 0xa800, 0x00000000 },
	{ 0xa804, 0x00021c00 },
	{ 0xa808, 0x00000403 },
	{ 0xa80c, 0x02001700 },
	{ 0xa810, 0x05000200 },
	{ 0xa814, 0x00000000 },
	{ 0xa818, 0x00690500 },
	{ 0xa81c, 0x0000007f },
	{ 0xa820, 0x01002501 },
	{ 0xa824, 0x00000300 },
	{ 0xa828, 0x01000331 },
	{ 0xa82c, 0x0000000c },
	{ 0xa830, 0x00010016 },
	{ 0xa834, 0x01100101 },
	{ 0xa838, 0x00010103 },
	{ 0xa83c, 0x00041300 },
	{ 0xa840, 0x00000b30 },
	{ 0xa844, 0x00000000 },
	{ 0xa848, 0x7f000000 },
	{ 0xa84c, 0x05000008 },
	{ 0xa850, 0x00000001 },
	{ 0xa854, 0x00000004 },
	{ 0xa858, 0x00000007 },
	{ 0xa85c, 0x00000000 },
	{ 0xa860, 0x00010000 },
	{ 0xa248, 0x0000221e },
	{ 0xa900, 0x00000000 },
	{ 0xa904, 0x00001c00 },
	{ 0xa908, 0x00000000 },
	{ 0xa90c, 0x06000000 },
	{ 0xa910, 0x09000200 },
	{ 0xa914, 0x00000000 },
	{ 0xa918, 0x00590000 },
	{ 0xa91c, 0x00000000 },
	{ 0xa920, 0x04002501 },
	{ 0xa924, 0x00000100 },
	{ 0xa928, 0x03000410 },
	{ 0xa92c, 0x00000000 },
	{ 0xa930, 0x00020000 },
	{ 0xa934, 0x02070106 },
	{ 0xa938, 0x00010100 },
	{ 0xa93c, 0x00401c00 },
	{ 0xa940, 0x00000000 },
	{ 0xa944, 0x00000000 },
	{ 0xa948, 0x10000e00 },
	{ 0xa94c, 0x02000004 },
	{ 0xa950, 0x00000001 },
	{ 0xa954, 0x00000004 },
	{ 0xa960, 0x00060000 },
	{ 0xaa3c, 0x00001c00 },
	{ 0xaa54, 0x00000004 },
	{ 0xaa60, 0x00060000 },
	{ 0 }
};

static const struct gt_powermeter ivb_pm_gt2_17w[] = {
	{ 0xa800, 0x20000000 },
	{ 0xa804, 0x000e3800 },
	{ 0xa808, 0x00000806 },
	{ 0xa80c, 0x0c002f00 },
	{ 0xa810, 0x0c000800 },
	{ 0xa814, 0x00000000 },
	{ 0xa818, 0x00d20d00 },
	{ 0xa81c, 0x000000ff },
	{ 0xa820, 0x03004b02 },
	{ 0xa824, 0x00000600 },
	{ 0xa828, 0x07000773 },
	{ 0xa82c, 0x00000000 },
	{ 0xa830, 0x00020032 },
	{ 0xa834, 0x1520040d },
	{ 0xa838, 0x00020105 },
	{ 0xa83c, 0x00083700 },
	{ 0xa840, 0x000016ff },
	{ 0xa844, 0x00000000 },
	{ 0xa848, 0xff000000 },
	{ 0xa84c, 0x0a000010 },
	{ 0xa850, 0x00000002 },
	{ 0xa854, 0x00000008 },
	{ 0xa858, 0x0000000f },
	{ 0xa85c, 0x00000000 },
	{ 0xa860, 0x00020000 },
	{ 0xa248, 0x0000221e },
	{ 0xa900, 0x00000000 },
	{ 0xa904, 0x00003800 },
	{ 0xa908, 0x00000000 },
	{ 0xa90c, 0x0c000000 },
	{ 0xa910, 0x12000800 },
	{ 0xa914, 0x00000000 },
	{ 0xa918, 0x00b20000 },
	{ 0xa91c, 0x00000000 },
	{ 0xa920, 0x08004b02 },
	{ 0xa924, 0x00000300 },
	{ 0xa928, 0x01000820 },
	{ 0xa92c, 0x00000000 },
	{ 0xa930, 0x00030000 },
	{ 0xa934, 0x15150406 },
	{ 0xa938, 0x00020300 },
	{ 0xa93c, 0x00903900 },
	{ 0xa940, 0x00000000 },
	{ 0xa944, 0x00000000 },
	{ 0xa948, 0x20001b00 },
	{ 0xa94c, 0x0a000010 },
	{ 0xa950, 0x00000000 },
	{ 0xa954, 0x00000008 },
	{ 0xa960, 0x00110000 },
	{ 0xaa3c, 0x00003900 },
	{ 0xaa54, 0x00000008 },
	{ 0xaa60, 0x00110000 },
	{ 0 }
};

static const struct gt_powermeter ivb_pm_gt2_35w[] = {
	{ 0xa800, 0x00000000 },
	{ 0xa804, 0x00030400 },
	{ 0xa808, 0x00000806 },
	{ 0xa80c, 0x0c002f00 },
	{ 0xa810, 0x0c000300 },
	{ 0xa814, 0x00000000 },
	{ 0xa818, 0x00d20d00 },
	{ 0xa81c, 0x000000ff },
	{ 0xa820, 0x03004b02 },
	{ 0xa824, 0x00000600 },
	{ 0xa828, 0x07000773 },
	{ 0xa82c, 0x00000000 },
	{ 0xa830, 0x00020032 },
	{ 0xa834, 0x1520040d },
	{ 0xa838, 0x00020105 },
	{ 0xa83c, 0x00083700 },
	{ 0xa840, 0x000016ff },
	{ 0xa844, 0x00000000 },
	{ 0xa848, 0xff000000 },
	{ 0xa84c, 0x0a000010 },
	{ 0xa850, 0x00000001 },
	{ 0xa854, 0x00000008 },
	{ 0xa858, 0x00000008 },
	{ 0xa85c, 0x00000000 },
	{ 0xa860, 0x00020000 },
	{ 0xa248, 0x0000221e },
	{ 0xa900, 0x00000000 },
	{ 0xa904, 0x00003800 },
	{ 0xa908, 0x00000000 },
	{ 0xa90c, 0x0c000000 },
	{ 0xa910, 0x12000800 },
	{ 0xa914, 0x00000000 },
	{ 0xa918, 0x00b20000 },
	{ 0xa91c, 0x00000000 },
	{ 0xa920, 0x08004b02 },
	{ 0xa924, 0x00000300 },
	{ 0xa928, 0x01000820 },
	{ 0xa92c, 0x00000000 },
	{ 0xa930, 0x00030000 },
	{ 0xa934, 0x15150406 },
	{ 0xa938, 0x00020300 },
	{ 0xa93c, 0x00903900 },
	{ 0xa940, 0x00000000 },
	{ 0xa944, 0x00000000 },
	{ 0xa948, 0x20001b00 },
	{ 0xa94c, 0x0a000010 },
	{ 0xa950, 0x00000000 },
	{ 0xa954, 0x00000008 },
	{ 0xa960, 0x00110000 },
	{ 0xaa3c, 0x00003900 },
	{ 0xaa54, 0x00000008 },
	{ 0xaa60, 0x00110000 },
	{ 0 }
};

static inline u32 gtt_read(void *bar, u32 reg)
{
	return readl(bar + reg);
}

static inline void gtt_write(void *bar, u32 reg, u32 data)
{
	writel(data, bar + reg);
}

static void gtt_write_powermeter(void *bar, const struct gt_powermeter *pm)
{
	for (; pm && pm->reg; pm++)
		gtt_write(bar, pm->reg, pm->value);
}

#define GTT_RETRY 1000
static int gtt_poll(void *bar, u32 reg, u32 mask, u32 value)
{
	unsigned try = GTT_RETRY;
	u32 data;

	while (try--) {
		data = gtt_read(bar, reg);
		if ((data & mask) == value)
			return 1;
		udelay(10);
	}

	printf("GT init timeout\n");
	return 0;
}

static int gma_pm_init_pre_vbios(void *gtt_bar, int rev)
{
	u32 reg32;

	debug("GT Power Management Init, silicon = %#x\n", rev);

	if (rev < IVB_STEP_C0) {
		/* 1: Enable force wake */
		gtt_write(gtt_bar, 0xa18c, 0x00000001);
		gtt_poll(gtt_bar, 0x130090, (1 << 0), (1 << 0));
	} else {
		gtt_write(gtt_bar, 0xa180, 1 << 5);
		gtt_write(gtt_bar, 0xa188, 0xffff0001);
		gtt_poll(gtt_bar, 0x130040, (1 << 0), (1 << 0));
	}

	if ((rev & BASE_REV_MASK) == BASE_REV_SNB) {
		/* 1d: Set GTT+0x42004 [15:14]=11 (SnB C1+) */
		reg32 = gtt_read(gtt_bar, 0x42004);
		reg32 |= (1 << 14) | (1 << 15);
		gtt_write(gtt_bar, 0x42004, reg32);
	}

	if (rev >= IVB_STEP_A0) {
		/* Display Reset Acknowledge Settings */
		reg32 = gtt_read(gtt_bar, 0x45010);
		reg32 |= (1 << 1) | (1 << 0);
		gtt_write(gtt_bar, 0x45010, reg32);
	}

	/* 2: Get GT SKU from GTT+0x911c[13] */
	reg32 = gtt_read(gtt_bar, 0x911c);
	if ((rev & BASE_REV_MASK) == BASE_REV_SNB) {
		if (reg32 & (1 << 13)) {
			debug("SNB GT1 Power Meter Weights\n");
			gtt_write_powermeter(gtt_bar, snb_pm_gt1);
		} else {
			debug("SNB GT2 Power Meter Weights\n");
			gtt_write_powermeter(gtt_bar, snb_pm_gt2);
		}
	} else {
		u32 unit = readl(MCHBAR_REG(0x5938)) & 0xf;

		if (reg32 & (1 << 13)) {
			/* GT1 SKU */
			debug("IVB GT1 Power Meter Weights\n");
			gtt_write_powermeter(gtt_bar, ivb_pm_gt1);
		} else {
			/* GT2 SKU */
			u32 tdp = readl(MCHBAR_REG(0x5930)) & 0x7fff;
			tdp /= (1 << unit);

			if (tdp <= 17) {
				/* <=17W ULV */
				debug("IVB GT2 17W Power Meter Weights\n");
				gtt_write_powermeter(gtt_bar, ivb_pm_gt2_17w);
			} else if ((tdp >= 25) && (tdp <= 35)) {
				/* 25W-35W */
				debug("IVB GT2 25W-35W Power Meter Weights\n");
				gtt_write_powermeter(gtt_bar, ivb_pm_gt2_35w);
			} else {
				/* All others */
				debug("IVB GT2 35W Power Meter Weights\n");
				gtt_write_powermeter(gtt_bar, ivb_pm_gt2_35w);
			}
		}
	}

	/* 3: Gear ratio map */
	gtt_write(gtt_bar, 0xa004, 0x00000010);

	/* 4: GFXPAUSE */
	gtt_write(gtt_bar, 0xa000, 0x00070020);

	/* 5: Dynamic EU trip control */
	gtt_write(gtt_bar, 0xa080, 0x00000004);

	/* 6: ECO bits */
	reg32 = gtt_read(gtt_bar, 0xa180);
	reg32 |= (1 << 26) | (1 << 31);
	/* (bit 20=1 for SNB step D1+ / IVB A0+) */
	if (rev >= SNB_STEP_D1)
		reg32 |= (1 << 20);
	gtt_write(gtt_bar, 0xa180, reg32);

	/* 6a: for SnB step D2+ only */
	if (((rev & BASE_REV_MASK) == BASE_REV_SNB) &&
	    (rev >= SNB_STEP_D2)) {
		reg32 = gtt_read(gtt_bar, 0x9400);
		reg32 |= (1 << 7);
		gtt_write(gtt_bar, 0x9400, reg32);

		reg32 = gtt_read(gtt_bar, 0x941c);
		reg32 &= 0xf;
		reg32 |= (1 << 1);
		gtt_write(gtt_bar, 0x941c, reg32);
		gtt_poll(gtt_bar, 0x941c, (1 << 1), (0 << 1));
	}

	if ((rev & BASE_REV_MASK) == BASE_REV_IVB) {
		reg32 = gtt_read(gtt_bar, 0x907c);
		reg32 |= (1 << 16);
		gtt_write(gtt_bar, 0x907c, reg32);

		/* 6b: Clocking reset controls */
		gtt_write(gtt_bar, 0x9424, 0x00000001);
	} else {
		/* 6b: Clocking reset controls */
		gtt_write(gtt_bar, 0x9424, 0x00000000);
	}

	/* 7 */
	if (gtt_poll(gtt_bar, 0x138124, (1 << 31), (0 << 31))) {
		gtt_write(gtt_bar, 0x138128, 0x00000029); /* Mailbox Data */
		/* Mailbox Cmd for RC6 VID */
		gtt_write(gtt_bar, 0x138124, 0x80000004);
		if (gtt_poll(gtt_bar, 0x138124, (1 << 31), (0 << 31)))
			gtt_write(gtt_bar, 0x138124, 0x8000000a);
		gtt_poll(gtt_bar, 0x138124, (1 << 31), (0 << 31));
	}

	/* 8 */
	gtt_write(gtt_bar, 0xa090, 0x00000000); /* RC Control */
	gtt_write(gtt_bar, 0xa098, 0x03e80000); /* RC1e Wake Rate Limit */
	gtt_write(gtt_bar, 0xa09c, 0x0028001e); /* RC6/6p Wake Rate Limit */
	gtt_write(gtt_bar, 0xa0a0, 0x0000001e); /* RC6pp Wake Rate Limit */
	gtt_write(gtt_bar, 0xa0a8, 0x0001e848); /* RC Evaluation Interval */
	gtt_write(gtt_bar, 0xa0ac, 0x00000019); /* RC Idle Hysteresis */

	/* 9 */
	gtt_write(gtt_bar, 0x2054, 0x0000000a); /* Render Idle Max Count */
	gtt_write(gtt_bar, 0x12054, 0x0000000a); /* Video Idle Max Count */
	gtt_write(gtt_bar, 0x22054, 0x0000000a); /* Blitter Idle Max Count */

	/* 10 */
	gtt_write(gtt_bar, 0xa0b0, 0x00000000); /* Unblock Ack to Busy */
	gtt_write(gtt_bar, 0xa0b4, 0x000003e8); /* RC1e Threshold */
	gtt_write(gtt_bar, 0xa0b8, 0x0000c350); /* RC6 Threshold */
	gtt_write(gtt_bar, 0xa0bc, 0x000186a0); /* RC6p Threshold */
	gtt_write(gtt_bar, 0xa0c0, 0x0000fa00); /* RC6pp Threshold */

	/* 11 */
	gtt_write(gtt_bar, 0xa010, 0x000f4240); /* RP Down Timeout */
	gtt_write(gtt_bar, 0xa014, 0x12060000); /* RP Interrupt Limits */
	gtt_write(gtt_bar, 0xa02c, 0x00015f90); /* RP Up Threshold */
	gtt_write(gtt_bar, 0xa030, 0x000186a0); /* RP Down Threshold */
	gtt_write(gtt_bar, 0xa068, 0x000186a0); /* RP Up EI */
	gtt_write(gtt_bar, 0xa06c, 0x000493e0); /* RP Down EI */
	gtt_write(gtt_bar, 0xa070, 0x0000000a); /* RP Idle Hysteresis */

	/* 11a: Enable Render Standby (RC6) */
	if ((rev & BASE_REV_MASK) == BASE_REV_IVB) {
		/*
		 * IvyBridge should also support DeepRenderStandby.
		 *
		 * Unfortunately it does not work reliably on all SKUs so
		 * disable it here and it can be enabled by the kernel.
		 */
		gtt_write(gtt_bar, 0xa090, 0x88040000); /* HW RC Control */
	} else {
		gtt_write(gtt_bar, 0xa090, 0x88040000); /* HW RC Control */
	}

	/* 12: Normal Frequency Request */
	/* RPNFREQ_VAL comes from MCHBAR 0x5998 23:16 (8 bits!? use 7) */
	reg32 = readl(MCHBAR_REG(0x5998));
	reg32 >>= 16;
	reg32 &= 0xef;
	reg32 <<= 25;
	gtt_write(gtt_bar, 0xa008, reg32);

	/* 13: RP Control */
	gtt_write(gtt_bar, 0xa024, 0x00000592);

	/* 14: Enable PM Interrupts */
	gtt_write(gtt_bar, 0x4402c, 0x03000076);

	/* Clear 0x6c024 [8:6] */
	reg32 = gtt_read(gtt_bar, 0x6c024);
	reg32 &= ~0x000001c0;
	gtt_write(gtt_bar, 0x6c024, reg32);

	return 0;
}

static int gma_pm_init_post_vbios(struct udevice *dev, int rev, void *gtt_bar)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	u32 reg32, cycle_delay;

	debug("GT Power Management Init (post VBIOS)\n");

	/* 15: Deassert Force Wake */
	if (rev < IVB_STEP_C0) {
		gtt_write(gtt_bar, 0xa18c, gtt_read(gtt_bar, 0xa18c) & ~1);
		gtt_poll(gtt_bar, 0x130090, (1 << 0), (0 << 0));
	} else {
		gtt_write(gtt_bar, 0xa188, 0x1fffe);
		if (gtt_poll(gtt_bar, 0x130040, (1 << 0), (0 << 0))) {
			gtt_write(gtt_bar, 0xa188,
				  gtt_read(gtt_bar, 0xa188) | 1);
		}
	}

	/* 16: SW RC Control */
	gtt_write(gtt_bar, 0xa094, 0x00060000);

	/* Setup Digital Port Hotplug */
	reg32 = gtt_read(gtt_bar, 0xc4030);
	if (!reg32) {
		u32 dp_hotplug[3];

		if (fdtdec_get_int_array(blob, node, "intel,dp_hotplug",
					 dp_hotplug, ARRAY_SIZE(dp_hotplug)))
			return -EINVAL;

		reg32 = (dp_hotplug[0] & 0x7) << 2;
		reg32 |= (dp_hotplug[0] & 0x7) << 10;
		reg32 |= (dp_hotplug[0] & 0x7) << 18;
		gtt_write(gtt_bar, 0xc4030, reg32);
	}

	/* Setup Panel Power On Delays */
	reg32 = gtt_read(gtt_bar, 0xc7208);
	if (!reg32) {
		reg32 = (unsigned)fdtdec_get_int(blob, node,
						 "panel-port-select", 0) << 30;
		reg32 |= fdtdec_get_int(blob, node, "panel-power-up-delay", 0)
				<< 16;
		reg32 |= fdtdec_get_int(blob, node,
					"panel-power-backlight-on-delay", 0);
		gtt_write(gtt_bar, 0xc7208, reg32);
	}

	/* Setup Panel Power Off Delays */
	reg32 = gtt_read(gtt_bar, 0xc720c);
	if (!reg32) {
		reg32 = fdtdec_get_int(blob, node, "panel-power-down-delay", 0)
				<< 16;
		reg32 |= fdtdec_get_int(blob, node,
					"panel-power-backlight-off-delay", 0);
		gtt_write(gtt_bar, 0xc720c, reg32);
	}

	/* Setup Panel Power Cycle Delay */
	cycle_delay = fdtdec_get_int(blob, node,
				     "intel,panel-power-cycle-delay", 0);
	if (cycle_delay) {
		reg32 = gtt_read(gtt_bar, 0xc7210);
		reg32 &= ~0xff;
		reg32 |= cycle_delay;
		gtt_write(gtt_bar, 0xc7210, reg32);
	}

	/* Enable Backlight if needed */
	reg32 = fdtdec_get_int(blob, node, "intel,cpu-backlight", 0);
	if (reg32) {
		gtt_write(gtt_bar, 0x48250, (1 << 31));
		gtt_write(gtt_bar, 0x48254, reg32);
	}
	reg32 = fdtdec_get_int(blob, node, "intel,pch-backlight", 0);
	if (reg32) {
		gtt_write(gtt_bar, 0xc8250, (1 << 31));
		gtt_write(gtt_bar, 0xc8254, reg32);
	}

	return 0;
}

/*
 * Some vga option roms are used for several chipsets but they only have one
 * PCI ID in their header. If we encounter such an option rom, we need to do
 * the mapping ourselves.
 */

uint32_t board_map_oprom_vendev(uint32_t vendev)
{
	switch (vendev) {
	case 0x80860102:		/* GT1 Desktop */
	case 0x8086010a:		/* GT1 Server */
	case 0x80860112:		/* GT2 Desktop */
	case 0x80860116:		/* GT2 Mobile */
	case 0x80860122:		/* GT2 Desktop >=1.3GHz */
	case 0x80860126:		/* GT2 Mobile >=1.3GHz */
	case 0x80860156:                /* IVB */
	case 0x80860166:                /* IVB */
		return 0x80860106;	/* GT1 Mobile */
	}

	return vendev;
}

static int int15_handler(void)
{
	int res = 0;

	debug("%s: INT15 function %04x!\n", __func__, M.x86.R_AX);

	switch (M.x86.R_AX) {
	case 0x5f34:
		/*
		 * Set Panel Fitting Hook:
		 *  bit 2 = Graphics Stretching
		 *  bit 1 = Text Stretching
		 *  bit 0 = Centering (do not set with bit1 or bit2)
		 *  0     = video bios default
		 */
		M.x86.R_AX = 0x005f;
		M.x86.R_CL = 0x00; /* Use video bios default */
		res = 1;
		break;
	case 0x5f35:
		/*
		 * Boot Display Device Hook:
		 *  bit 0 = CRT
		 *  bit 1 = TV (eDP)
		 *  bit 2 = EFP
		 *  bit 3 = LFP
		 *  bit 4 = CRT2
		 *  bit 5 = TV2 (eDP)
		 *  bit 6 = EFP2
		 *  bit 7 = LFP2
		 */
		M.x86.R_AX = 0x005f;
		M.x86.R_CX = 0x0000; /* Use video bios default */
		res = 1;
		break;
	case 0x5f51:
		/*
		 * Hook to select active LFP configuration:
		 *  00h = No LVDS, VBIOS does not enable LVDS
		 *  01h = Int-LVDS, LFP driven by integrated LVDS decoder
		 *  02h = SVDO-LVDS, LFP driven by SVDO decoder
		 *  03h = eDP, LFP Driven by Int-DisplayPort encoder
		 */
		M.x86.R_AX = 0x005f;
		M.x86.R_CX = 0x0003; /* eDP */
		res = 1;
		break;
	case 0x5f70:
		switch (M.x86.R_CH) {
		case 0:
			/* Get Mux */
			M.x86.R_AX = 0x005f;
			M.x86.R_CX = 0x0000;
			res = 1;
			break;
		case 1:
			/* Set Mux */
			M.x86.R_AX = 0x005f;
			M.x86.R_CX = 0x0000;
			res = 1;
			break;
		case 2:
			/* Get SG/Non-SG mode */
			M.x86.R_AX = 0x005f;
			M.x86.R_CX = 0x0000;
			res = 1;
			break;
		default:
			/* Interrupt was not handled */
			debug("Unknown INT15 5f70 function: 0x%02x\n",
			      M.x86.R_CH);
			break;
		}
		break;
	case 0x5fac:
		res = 1;
		break;
	default:
		debug("Unknown INT15 function %04x!\n", M.x86.R_AX);
		break;
	}
	return res;
}

static void sandybridge_setup_graphics(struct udevice *dev,
				       struct udevice *video_dev)
{
	u32 reg32;
	u16 reg16;
	u8 reg8;

	dm_pci_read_config16(video_dev, PCI_DEVICE_ID, &reg16);
	switch (reg16) {
	case 0x0102: /* GT1 Desktop */
	case 0x0106: /* GT1 Mobile */
	case 0x010a: /* GT1 Server */
	case 0x0112: /* GT2 Desktop */
	case 0x0116: /* GT2 Mobile */
	case 0x0122: /* GT2 Desktop >=1.3GHz */
	case 0x0126: /* GT2 Mobile >=1.3GHz */
	case 0x0156: /* IvyBridge */
	case 0x0166: /* IvyBridge */
		break;
	default:
		debug("Graphics not supported by this CPU/chipset\n");
		return;
	}

	debug("Initialising Graphics\n");

	/* Setup IGD memory by setting GGC[7:3] = 1 for 32MB */
	dm_pci_read_config16(dev, GGC, &reg16);
	reg16 &= ~0x00f8;
	reg16 |= 1 << 3;
	/* Program GTT memory by setting GGC[9:8] = 2MB */
	reg16 &= ~0x0300;
	reg16 |= 2 << 8;
	/* Enable VGA decode */
	reg16 &= ~0x0002;
	dm_pci_write_config16(dev, GGC, reg16);

	/* Enable 256MB aperture */
	dm_pci_read_config8(video_dev, MSAC, &reg8);
	reg8 &= ~0x06;
	reg8 |= 0x02;
	dm_pci_write_config8(video_dev, MSAC, reg8);

	/* Erratum workarounds */
	reg32 = readl(MCHBAR_REG(0x5f00));
	reg32 |= (1 << 9) | (1 << 10);
	writel(reg32, MCHBAR_REG(0x5f00));

	/* Enable SA Clock Gating */
	reg32 = readl(MCHBAR_REG(0x5f00));
	writel(reg32 | 1, MCHBAR_REG(0x5f00));

	/* GPU RC6 workaround for sighting 366252 */
	reg32 = readl(MCHBAR_REG(0x5d14));
	reg32 |= (1 << 31);
	writel(reg32, MCHBAR_REG(0x5d14));

	/* VLW */
	reg32 = readl(MCHBAR_REG(0x6120));
	reg32 &= ~(1 << 0);
	writel(reg32, MCHBAR_REG(0x6120));

	reg32 = readl(MCHBAR_REG(0x5418));
	reg32 |= (1 << 4) | (1 << 5);
	writel(reg32, MCHBAR_REG(0x5418));
}

static int gma_func0_init(struct udevice *dev)
{
	struct udevice *nbridge;
	void *gtt_bar;
	ulong base;
	u32 reg32;
	int ret;
	int rev;

	/* Enable PCH Display Port */
	writew(0x0010, RCB_REG(DISPBDF));
	setbits_le32(RCB_REG(FD2), PCH_ENABLE_DBDF);

	ret = uclass_first_device_err(UCLASS_NORTHBRIDGE, &nbridge);
	if (ret)
		return ret;
	rev = bridge_silicon_revision(nbridge);
	sandybridge_setup_graphics(nbridge, dev);

	/* IGD needs to be Bus Master */
	dm_pci_read_config32(dev, PCI_COMMAND, &reg32);
	reg32 |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY | PCI_COMMAND_IO;
	dm_pci_write_config32(dev, PCI_COMMAND, reg32);

	/* Use write-combining for the graphics memory, 256MB */
	base = dm_pci_read_bar32(dev, 2);
	mtrr_add_request(MTRR_TYPE_WRCOMB, base, 256 << 20);
	mtrr_commit(true);

	gtt_bar = (void *)(ulong)dm_pci_read_bar32(dev, 0);
	debug("GT bar %p\n", gtt_bar);
	ret = gma_pm_init_pre_vbios(gtt_bar, rev);
	if (ret)
		return ret;

	return rev;
}

static int bd82x6x_video_probe(struct udevice *dev)
{
	void *gtt_bar;
	int ret, rev;

	rev = gma_func0_init(dev);
	if (rev < 0)
		return rev;
	ret = vbe_setup_video(dev, int15_handler);
	if (ret)
		return ret;

	/* Post VBIOS init */
	gtt_bar = (void *)(ulong)dm_pci_read_bar32(dev, 0);
	ret = gma_pm_init_post_vbios(dev, rev, gtt_bar);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id bd82x6x_video_ids[] = {
	{ .compatible = "intel,gma" },
	{ }
};

U_BOOT_DRIVER(bd82x6x_video) = {
	.name	= "bd82x6x_video",
	.id	= UCLASS_VIDEO,
	.of_match = bd82x6x_video_ids,
	.probe	= bd82x6x_video_probe,
};
