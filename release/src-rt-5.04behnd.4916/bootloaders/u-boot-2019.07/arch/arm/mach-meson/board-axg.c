// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <asm/arch/boot.h>
#include <asm/arch/eth.h>
#include <asm/arch/axg.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/armv8/mmu.h>
#include <linux/sizes.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

int meson_get_boot_device(void)
{
	return readl(AXG_AO_SEC_GP_CFG0) & AXG_AO_BOOT_DEVICE;
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
	reg = readl(AXG_AO_SEC_GP_CFG3);

	bl31_size = ((reg & AXG_AO_BL31_RSVMEM_SIZE_MASK)
			>> AXG_AO_BL31_RSVMEM_SIZE_SHIFT) * SZ_1K;
	bl32_size = (reg & AXG_AO_BL32_RSVMEM_SIZE_MASK) * SZ_1K;

	bl31_start = readl(AXG_AO_SEC_GP_CFG5);
	bl32_start = readl(AXG_AO_SEC_GP_CFG4);

	/* Add BL31 reserved zone */
	if (bl31_start && bl31_size)
		meson_board_add_reserved_memory(fdt, bl31_start, bl31_size);

	/* Add BL32 reserved zone */
	if (bl32_start && bl32_size)
		meson_board_add_reserved_memory(fdt, bl32_start, bl32_size);
}

phys_size_t get_effective_memsize(void)
{
	/* Size is reported in MiB, convert it in bytes */
	return ((readl(AXG_AO_SEC_GP_CFG0) & AXG_AO_MEM_SIZE_MASK)
			>> AXG_AO_MEM_SIZE_SHIFT) * SZ_1M;
}

static struct mm_region axg_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = axg_mem_map;

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
		setbits_le32(AXG_ETH_REG_0, AXG_ETH_REG_0_PHY_INTF_RGMII |
			     AXG_ETH_REG_0_TX_PHASE(1) |
			     AXG_ETH_REG_0_TX_RATIO(4) |
			     AXG_ETH_REG_0_PHY_CLK_EN |
			     AXG_ETH_REG_0_CLK_EN);
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(AXG_ETH_REG_0, AXG_ETH_REG_0_PHY_INTF_RMII |
					AXG_ETH_REG_0_INVERT_RMII_CLK |
					AXG_ETH_REG_0_CLK_EN);
		break;

	default:
		printf("Invalid Ethernet interface mode\n");
		return;
	}

	/* Enable power gate */
	clrbits_le32(AXG_MEM_PD_REG_0, AXG_MEM_PD_REG_0_ETH_MASK);
}
