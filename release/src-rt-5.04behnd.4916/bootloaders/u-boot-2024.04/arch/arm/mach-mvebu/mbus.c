// SPDX-License-Identifier: GPL-2.0
/*
 * Address map functions for Marvell EBU SoCs (Kirkwood, Armada
 * 370/XP, Dove, Orion5x and MV78xx0)
 *
 * Ported from the Barebox version to U-Boot by:
 * Stefan Roese <sr@denx.de>
 *
 * The Barebox version is:
 * Sebastian Hesselbarth <sebastian.hesselbarth@gmail.com>
 *
 * based on mbus driver from Linux
 *   (C) Copyright 2008 Marvell Semiconductor
 *
 * The Marvell EBU SoCs have a configurable physical address space:
 * the physical address at which certain devices (PCIe, NOR, NAND,
 * etc.) sit can be configured. The configuration takes place through
 * two sets of registers:
 *
 * - One to configure the access of the CPU to the devices. Depending
 *   on the families, there are between 8 and 20 configurable windows,
 *   each can be use to create a physical memory window that maps to a
 *   specific device. Devices are identified by a tuple (target,
 *   attribute).
 *
 * - One to configure the access to the CPU to the SDRAM. There are
 *   either 2 (for Dove) or 4 (for other families) windows to map the
 *   SDRAM into the physical address space.
 *
 * This driver:
 *
 * - Reads out the SDRAM address decoding windows at initialization
 *   time, and fills the mbus_dram_info structure with these
 *   informations. The exported function mv_mbus_dram_info() allow
 *   device drivers to get those informations related to the SDRAM
 *   address decoding windows. This is because devices also have their
 *   own windows (configured through registers that are part of each
 *   device register space), and therefore the drivers for Marvell
 *   devices have to configure those device -> SDRAM windows to ensure
 *   that DMA works properly.
 *
 * - Provides an API for platform code or device drivers to
 *   dynamically add or remove address decoding windows for the CPU ->
 *   device accesses. This API is mvebu_mbus_add_window_by_id(),
 *   mvebu_mbus_add_window_remap_by_id() and
 *   mvebu_mbus_del_window().
 */

#include <common.h>
#include <malloc.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/log2.h>
#include <linux/mbus.h>

/* DDR target is the same on all platforms */
#define TARGET_DDR		0

/* CPU Address Decode Windows registers */
#define WIN_CTRL_OFF		0x0000
#define   WIN_CTRL_ENABLE       BIT(0)
#define   WIN_CTRL_TGT_MASK     0xf0
#define   WIN_CTRL_TGT_SHIFT    4
#define   WIN_CTRL_ATTR_MASK    0xff00
#define   WIN_CTRL_ATTR_SHIFT   8
#define   WIN_CTRL_SIZE_MASK    0xffff0000
#define   WIN_CTRL_SIZE_SHIFT   16
#define WIN_BASE_OFF		0x0004
#define   WIN_BASE_LOW          0xffff0000
#define   WIN_BASE_HIGH         0xf
#define WIN_REMAP_LO_OFF	0x0008
#define   WIN_REMAP_LOW         0xffff0000
#define WIN_REMAP_HI_OFF	0x000c

#define ATTR_HW_COHERENCY	(0x1 << 4)

#define DDR_BASE_CS_OFF(n)	(0x0000 + ((n) << 3))
#define  DDR_BASE_CS_HIGH_MASK  0xf
#define  DDR_BASE_CS_LOW_MASK   0xff000000
#define DDR_SIZE_CS_OFF(n)	(0x0004 + ((n) << 3))
#define  DDR_SIZE_ENABLED       BIT(0)
#define  DDR_SIZE_CS_MASK       0x1c
#define  DDR_SIZE_CS_SHIFT      2
#define  DDR_SIZE_MASK          0xff000000

#define DOVE_DDR_BASE_CS_OFF(n) ((n) << 4)

static struct mbus_dram_target_info mbus_dram_info
	__section(".data");

#if defined(CONFIG_ARCH_MVEBU)
	#define MVEBU_MBUS_NUM_WINS 20
	#define MVEBU_MBUS_NUM_REMAPPABLE_WINS 8
	#define MVEBU_MBUS_WIN_CFG_OFFSET(win) armada_370_xp_mbus_win_offset(win)
#elif defined(CONFIG_ARCH_KIRKWOOD)
	#define MVEBU_MBUS_NUM_WINS 8
	#define MVEBU_MBUS_NUM_REMAPPABLE_WINS 4
	#define MVEBU_MBUS_WIN_CFG_OFFSET(win) orion5x_mbus_win_offset(win)
#else
	#error "No supported architecture"
#endif

static unsigned int armada_370_xp_mbus_win_offset(int win);
static unsigned int orion5x_mbus_win_offset(int win);

/*
 * Functions to manipulate the address decoding windows
 */

static void mvebu_mbus_read_window(int win, int *enabled, u64 *base,
				   u32 *size, u8 *target, u8 *attr,
				   u64 *remap)
{
	void __iomem *addr = (void __iomem *)MVEBU_CPU_WIN_BASE +
		MVEBU_MBUS_WIN_CFG_OFFSET(win);
	u32 basereg = readl(addr + WIN_BASE_OFF);
	u32 ctrlreg = readl(addr + WIN_CTRL_OFF);

	if (!(ctrlreg & WIN_CTRL_ENABLE)) {
		*enabled = 0;
		return;
	}

	*enabled = 1;
	*base = ((u64)basereg & WIN_BASE_HIGH) << 32;
	*base |= (basereg & WIN_BASE_LOW);
	*size = (ctrlreg | ~WIN_CTRL_SIZE_MASK) + 1;

	if (target)
		*target = (ctrlreg & WIN_CTRL_TGT_MASK) >> WIN_CTRL_TGT_SHIFT;

	if (attr)
		*attr = (ctrlreg & WIN_CTRL_ATTR_MASK) >> WIN_CTRL_ATTR_SHIFT;

	if (remap) {
		if (win < MVEBU_MBUS_NUM_REMAPPABLE_WINS) {
			u32 remap_low = readl(addr + WIN_REMAP_LO_OFF);
			u32 remap_hi  = readl(addr + WIN_REMAP_HI_OFF);
			*remap = ((u64)remap_hi << 32) | remap_low;
		} else {
			*remap = 0;
		}
	}
}

static void mvebu_mbus_disable_window(int win)
{
	void __iomem *addr;

	addr = (void __iomem *)MVEBU_CPU_WIN_BASE + MVEBU_MBUS_WIN_CFG_OFFSET(win);

	writel(0, addr + WIN_BASE_OFF);
	writel(0, addr + WIN_CTRL_OFF);
	if (win < MVEBU_MBUS_NUM_REMAPPABLE_WINS) {
		writel(0, addr + WIN_REMAP_LO_OFF);
		writel(0, addr + WIN_REMAP_HI_OFF);
	}
}

/* Checks whether the given window number is available */
static int mvebu_mbus_window_is_free(const int win)
{
	void __iomem *addr = (void __iomem *)MVEBU_CPU_WIN_BASE +
		MVEBU_MBUS_WIN_CFG_OFFSET(win);
	u32 ctrl = readl(addr + WIN_CTRL_OFF);
	return !(ctrl & WIN_CTRL_ENABLE);
}

/*
 * Checks whether the given (base, base+size) area doesn't overlap an
 * existing region
 */
static int mvebu_mbus_window_conflicts(phys_addr_t base, size_t size,
				       u8 target, u8 attr)
{
	u64 end = (u64)base + size;
	int win;

	for (win = 0; win < MVEBU_MBUS_NUM_WINS; win++) {
		u64 wbase, wend;
		u32 wsize;
		u8 wtarget, wattr;
		int enabled;

		mvebu_mbus_read_window(win,
				       &enabled, &wbase, &wsize,
				       &wtarget, &wattr, NULL);

		if (!enabled)
			continue;

		wend = wbase + wsize;

		/*
		 * Check if the current window overlaps with the
		 * proposed physical range
		 */
		if ((u64)base < wend && end > wbase)
			return 0;

		/*
		 * Check if target/attribute conflicts
		 */
		if (target == wtarget && attr == wattr)
			return 0;
	}

	return 1;
}

static int mvebu_mbus_find_window(phys_addr_t base, size_t size)
{
	int win;

	for (win = 0; win < MVEBU_MBUS_NUM_WINS; win++) {
		u64 wbase;
		u32 wsize;
		int enabled;

		mvebu_mbus_read_window(win,
				       &enabled, &wbase, &wsize,
				       NULL, NULL, NULL);

		if (!enabled)
			continue;

		if (base == wbase && size == wsize)
			return win;
	}

	return -ENODEV;
}

static int mvebu_mbus_setup_window(int win, phys_addr_t base, size_t size,
				   phys_addr_t remap, u8 target,
				   u8 attr)
{
	void __iomem *addr = (void __iomem *)MVEBU_CPU_WIN_BASE +
		MVEBU_MBUS_WIN_CFG_OFFSET(win);
	u32 ctrl, remap_addr;

	ctrl = ((size - 1) & WIN_CTRL_SIZE_MASK) |
		(attr << WIN_CTRL_ATTR_SHIFT)    |
		(target << WIN_CTRL_TGT_SHIFT)   |
		WIN_CTRL_ENABLE;

	writel(base & WIN_BASE_LOW, addr + WIN_BASE_OFF);
	writel(ctrl, addr + WIN_CTRL_OFF);
	if (win < MVEBU_MBUS_NUM_REMAPPABLE_WINS) {
		if (remap == MVEBU_MBUS_NO_REMAP)
			remap_addr = base;
		else
			remap_addr = remap;
		writel(remap_addr & WIN_REMAP_LOW, addr + WIN_REMAP_LO_OFF);
		writel(0, addr + WIN_REMAP_HI_OFF);
	}

	return 0;
}

static int mvebu_mbus_alloc_window(phys_addr_t base, size_t size,
				   phys_addr_t remap, u8 target,
				   u8 attr)
{
	int win;

	if (remap == MVEBU_MBUS_NO_REMAP) {
		for (win = MVEBU_MBUS_NUM_REMAPPABLE_WINS;
		     win < MVEBU_MBUS_NUM_WINS; win++)
			if (mvebu_mbus_window_is_free(win))
				return mvebu_mbus_setup_window(win, base,
							       size, remap,
							       target, attr);
	}


	for (win = 0; win < MVEBU_MBUS_NUM_WINS; win++)
		if (mvebu_mbus_window_is_free(win))
			return mvebu_mbus_setup_window(win, base, size,
						       remap, target, attr);

	return -ENOMEM;
}

/*
 * SoC-specific functions and definitions
 */

static unsigned int __maybe_unused armada_370_xp_mbus_win_offset(int win)
{
	/* The register layout is a bit annoying and the below code
	 * tries to cope with it.
	 * - At offset 0x0, there are the registers for the first 8
	 *   windows, with 4 registers of 32 bits per window (ctrl,
	 *   base, remap low, remap high)
	 * - Then at offset 0x80, there is a hole of 0x10 bytes for
	 *   the internal registers base address and internal units
	 *   sync barrier register.
	 * - Then at offset 0x90, there the registers for 12
	 *   windows, with only 2 registers of 32 bits per window
	 *   (ctrl, base).
	 */
	if (win < 8)
		return win << 4;
	else
		return 0x90 + ((win - 8) << 3);
}

static unsigned int __maybe_unused orion5x_mbus_win_offset(int win)
{
	return win << 4;
}

static void mvebu_mbus_default_setup_cpu_target(void)
{
	int i;
	int cs;

	mbus_dram_info.mbus_dram_target_id = TARGET_DDR;

	for (i = 0, cs = 0; i < 4; i++) {
		u32 base = readl((void __iomem *)MVEBU_SDRAM_BASE + DDR_BASE_CS_OFF(i));
		u32 size = readl((void __iomem *)MVEBU_SDRAM_BASE + DDR_SIZE_CS_OFF(i));

		/*
		 * We only take care of entries for which the chip
		 * select is enabled, and that don't have high base
		 * address bits set (devices can only access the first
		 * 32 bits of the memory).
		 */
		if ((size & DDR_SIZE_ENABLED) &&
		    !(base & DDR_BASE_CS_HIGH_MASK)) {
			struct mbus_dram_window *w;

			w = &mbus_dram_info.cs[cs++];
			w->cs_index = i;
			w->mbus_attr = 0xf & ~(1 << i);
			w->base = base & DDR_BASE_CS_LOW_MASK;
			w->size = (size | ~DDR_SIZE_MASK) + 1;
		}
	}
	mbus_dram_info.num_cs = cs;

#if defined(CONFIG_ARMADA_MSYS)
	/* Disable MBUS Err Prop - in order to avoid data aborts */
	clrbits_le32((void __iomem *)MVEBU_CPU_WIN_BASE + 0x200, BIT(8));
#endif
}

/*
 * Public API of the driver
 */
const struct mbus_dram_target_info *mvebu_mbus_dram_info(void)
{
	return &mbus_dram_info;
}

int mvebu_mbus_add_window_remap_by_id(unsigned int target,
				      unsigned int attribute,
				      phys_addr_t base, size_t size,
				      phys_addr_t remap)
{
	if (!mvebu_mbus_window_conflicts(base, size, target, attribute)) {
		printf("Cannot add window '%x:%x', conflicts with another window\n",
		       target, attribute);
		return -EINVAL;
	}

	return mvebu_mbus_alloc_window(base, size, remap, target, attribute);
}

int mvebu_mbus_add_window_by_id(unsigned int target, unsigned int attribute,
				phys_addr_t base, size_t size)
{
	return mvebu_mbus_add_window_remap_by_id(target, attribute, base,
						 size, MVEBU_MBUS_NO_REMAP);
}

int mvebu_mbus_del_window(phys_addr_t base, size_t size)
{
	int win;

	win = mvebu_mbus_find_window(base, size);
	if (win < 0)
		return win;

	mvebu_mbus_disable_window(win);
	return 0;
}

#ifndef CONFIG_ARCH_KIRKWOOD
static void mvebu_mbus_get_lowest_base(phys_addr_t *base)
{
	int win;
	*base = 0xffffffff;

	for (win = 0; win < MVEBU_MBUS_NUM_WINS; win++) {
		u64 wbase;
		u32 wsize;
		u8 wtarget, wattr;
		int enabled;

		mvebu_mbus_read_window(win,
				       &enabled, &wbase, &wsize,
				       &wtarget, &wattr, NULL);

		if (!enabled)
			continue;

		if (wbase < *base)
			*base = wbase;
	}
}

static void mvebu_config_mbus_bridge(void)
{
	phys_addr_t base;
	u32 val;
	u32 size;

	/* Set MBUS bridge base/ctrl */
	mvebu_mbus_get_lowest_base(&base);

	size = 0xffffffff - base + 1;
	if (!is_power_of_2(size)) {
		/* Round up to next power of 2 */
		size = 1 << (ffs(base) + 1);
		base = 0xffffffff - size + 1;
	}

	/* Now write base and size */
	writel(base, MBUS_BRIDGE_WIN_BASE_REG);
	/* Align window size to 64KiB */
	val = (size / (64 << 10)) - 1;
	writel((val << 16) | 0x1, MBUS_BRIDGE_WIN_CTRL_REG);
}
#endif

int mbus_dt_setup_win(u32 base, u32 size, u8 target, u8 attr)
{
	if (!mvebu_mbus_window_conflicts(base, size, target, attr)) {
		printf("Cannot add window '%04x:%04x', conflicts with another window\n",
		       target, attr);
		return -EBUSY;
	}

	/*
	 * In U-Boot we first try to add the mbus window to the remap windows.
	 * If this fails, lets try to add the windows to the non-remap windows.
	 */
	if (mvebu_mbus_alloc_window(base, size, base, target, attr)) {
		if (mvebu_mbus_alloc_window(base, size,
					    MVEBU_MBUS_NO_REMAP, target, attr))
			return -ENOMEM;
	}

#ifndef CONFIG_ARCH_KIRKWOOD
	/*
	 * Re-configure the mbus bridge registers each time this function
	 * is called. Since it may get called from the board code in
	 * later boot stages as well.
	 */
	mvebu_config_mbus_bridge();
#endif

	return 0;
}

int mvebu_mbus_probe(const struct mbus_win windows[], int count)
{
	int win;
	int ret;
	int i;

	for (win = 0; win < MVEBU_MBUS_NUM_WINS; win++)
		mvebu_mbus_disable_window(win);

	mvebu_mbus_default_setup_cpu_target();

	/* Setup statically declared windows in the DT */
	for (i = 0; i < count; i++) {
		u32 base, size;
		u8 target, attr;

		target = windows[i].target;
		attr = windows[i].attr;
		base = windows[i].base;
		size = windows[i].size;
		ret = mbus_dt_setup_win(base, size, target, attr);
		if (ret < 0)
			return ret;
	}

	return 0;
}
