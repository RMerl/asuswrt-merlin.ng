// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <asm/arch/boot.h>
#include <asm/arch/eth.h>
#include <asm/arch/gx.h>
#include <asm/arch/mem.h>
#include <asm/arch/meson-vpu.h>
#include <asm/io.h>
#include <asm/armv8/mmu.h>
#include <linux/sizes.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

int meson_get_boot_device(void)
{
	return readl(GX_AO_SEC_GP_CFG0) & GX_AO_BOOT_DEVICE;
}

/* Configure the reserved memory zones exported by the secure registers
 * into EFI and DTB reserved memory entries.
 */
void meson_init_reserved_memory(void *fdt)
{
	u64 bl31_size, bl31_start;
	u64 bl32_size, bl32_start;
	u32 reg;

	/*
	 * Get ARM Trusted Firmware reserved memory zones in :
	 * - AO_SEC_GP_CFG3: bl32 & bl31 size in KiB, can be 0
	 * - AO_SEC_GP_CFG5: bl31 physical start address, can be NULL
	 * - AO_SEC_GP_CFG4: bl32 physical start address, can be NULL
	 */
	reg = readl(GX_AO_SEC_GP_CFG3);

	bl31_size = ((reg & GX_AO_BL31_RSVMEM_SIZE_MASK)
			>> GX_AO_BL31_RSVMEM_SIZE_SHIFT) * SZ_1K;
	bl32_size = (reg & GX_AO_BL32_RSVMEM_SIZE_MASK) * SZ_1K;

	bl31_start = readl(GX_AO_SEC_GP_CFG5);
	bl32_start = readl(GX_AO_SEC_GP_CFG4);

	/*
	 * Early Meson GX Firmware revisions did not provide the reserved
	 * memory zones in the registers, keep fixed memory zone handling.
	 */
	if (IS_ENABLED(CONFIG_MESON_GX) &&
	    !reg && !bl31_start && !bl32_start) {
		bl31_start = 0x10000000;
		bl31_size = 0x200000;
	}

	/* Add first 16MiB reserved zone */
	meson_board_add_reserved_memory(fdt, 0, GX_FIRMWARE_MEM_SIZE);

	/* Add BL31 reserved zone */
	if (bl31_start && bl31_size)
		meson_board_add_reserved_memory(fdt, bl31_start, bl31_size);

	/* Add BL32 reserved zone */
	if (bl32_start && bl32_size)
		meson_board_add_reserved_memory(fdt, bl32_start, bl32_size);

#if defined(CONFIG_VIDEO_MESON)
	meson_vpu_rsv_fb(fdt);
#endif
}

phys_size_t get_effective_memsize(void)
{
	/* Size is reported in MiB, convert it in bytes */
	return ((readl(GX_AO_SEC_GP_CFG0) & GX_AO_MEM_SIZE_MASK)
			>> GX_AO_MEM_SIZE_SHIFT) * SZ_1M;
}

static struct mm_region gx_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xc0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xc0000000UL,
		.phys = 0xc0000000UL,
		.size = 0x30000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = gx_mem_map;

/* Configure the Ethernet MAC with the requested interface mode
 * with some optional flags.
 */
void meson_eth_init(phy_interface_t mode, unsigned int flags)
{
	switch (mode) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		/* Set RGMII mode */
		setbits_le32(GX_ETH_REG_0, GX_ETH_REG_0_PHY_INTF |
			     GX_ETH_REG_0_TX_PHASE(1) |
			     GX_ETH_REG_0_TX_RATIO(4) |
			     GX_ETH_REG_0_PHY_CLK_EN |
			     GX_ETH_REG_0_CLK_EN);

		/* Reset to external PHY */
		if(!IS_ENABLED(CONFIG_MESON_GXBB))
			writel(0x2009087f, GX_ETH_REG_3);

		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(GX_ETH_REG_0, GX_ETH_REG_0_INVERT_RMII_CLK |
					 GX_ETH_REG_0_CLK_EN);

		/* Use GXL RMII Internal PHY (also on GXM) */
		if (!IS_ENABLED(CONFIG_MESON_GXBB)) {
			if ((flags & MESON_USE_INTERNAL_RMII_PHY)) {
				writel(0x10110181, GX_ETH_REG_2);
				writel(0xe40908ff, GX_ETH_REG_3);
			} else
				writel(0x2009087f, GX_ETH_REG_3);
		}

		break;

	default:
		printf("Invalid Ethernet interface mode\n");
		return;
	}

	/* Enable power gate */
	clrbits_le32(GX_MEM_PD_REG_0, GX_MEM_PD_REG_0_ETH_MASK);
}
