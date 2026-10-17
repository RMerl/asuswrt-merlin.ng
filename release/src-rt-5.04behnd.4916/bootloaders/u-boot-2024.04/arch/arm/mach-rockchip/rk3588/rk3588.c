// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#include <common.h>
#include <spl.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/ioc_rk3588.h>

#define FIREWALL_DDR_BASE		0xfe030000
#define FW_DDR_MST5_REG			0x54
#define FW_DDR_MST13_REG		0x74
#define FW_DDR_MST21_REG		0x94
#define FW_DDR_MST26_REG		0xa8
#define FW_DDR_MST27_REG		0xac
#define FIREWALL_SYSMEM_BASE		0xfe038000
#define FW_SYSM_MST5_REG		0x54
#define FW_SYSM_MST13_REG		0x74
#define FW_SYSM_MST21_REG		0x94
#define FW_SYSM_MST26_REG		0xa8
#define FW_SYSM_MST27_REG		0xac

#define PMU1_IOC_BASE			0xfd5f0000
#define PMU2_IOC_BASE			0xfd5f4000

#define BUS_IOC_BASE			0xfd5f8000
#define BUS_IOC_GPIO2A_IOMUX_SEL_L	0x40
#define BUS_IOC_GPIO2B_IOMUX_SEL_L	0x48
#define BUS_IOC_GPIO2D_IOMUX_SEL_L	0x58
#define BUS_IOC_GPIO2D_IOMUX_SEL_H	0x5c
#define BUS_IOC_GPIO3A_IOMUX_SEL_L	0x60

/**
 * Boot-device identifiers used by the BROM on RK3588 when device is booted
 * from SPI flash. IOMUX used for SPI flash affect the value used by the BROM
 * and not the type of SPI flash used.
 */
enum {
	BROM_BOOTSOURCE_FSPI_M0 = 3,
	BROM_BOOTSOURCE_FSPI_M1 = 4,
	BROM_BOOTSOURCE_FSPI_M2 = 6,
};

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@fe2e0000",
	[BROM_BOOTSOURCE_FSPI_M0] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_FSPI_M1] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_FSPI_M2] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_SD] = "/mmc@fe2c0000",
};

static struct mm_region rk3588_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xf0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},  {
		.virt = 0x900000000,
		.phys = 0x900000000,
		.size = 0x150000000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},  {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3588_mem_map;

/* GPIO0B_IOMUX_SEL_H */
enum {
	GPIO0B5_SHIFT		= 4,
	GPIO0B5_MASK		= GENMASK(7, 4),
	GPIO0B5_REFER		= 8,
	GPIO0B5_UART2_TX_M0	= 10,

	GPIO0B6_SHIFT		= 8,
	GPIO0B6_MASK		= GENMASK(11, 8),
	GPIO0B6_REFER		= 8,
	GPIO0B6_UART2_RX_M0	= 10,
};

void board_debug_uart_init(void)
{
	__maybe_unused static struct rk3588_bus_ioc * const bus_ioc = (void *)BUS_IOC_BASE;
	static struct rk3588_pmu2_ioc * const pmu2_ioc = (void *)PMU2_IOC_BASE;

	/* Refer to BUS_IOC */
	rk_clrsetreg(&pmu2_ioc->gpio0b_iomux_sel_h,
		     GPIO0B6_MASK | GPIO0B5_MASK,
		     GPIO0B6_REFER << GPIO0B6_SHIFT |
		     GPIO0B5_REFER << GPIO0B5_SHIFT);

	/* UART2_M0 Switch iomux */
	rk_clrsetreg(&bus_ioc->gpio0b_iomux_sel_h,
		     GPIO0B6_MASK | GPIO0B5_MASK,
		     GPIO0B6_UART2_RX_M0 << GPIO0B6_SHIFT |
		     GPIO0B5_UART2_TX_M0 << GPIO0B5_SHIFT);
}

#ifdef CONFIG_SPL_BUILD
void rockchip_stimer_init(void)
{
	/* If Timer already enabled, don't re-init it */
	u32 reg = readl(CONFIG_ROCKCHIP_STIMER_BASE + 0x4);

	if (reg & 0x1)
		return;

	asm volatile("msr CNTFRQ_EL0, %0" : : "r" (CONFIG_COUNTER_FREQUENCY));
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x14);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x18);
	writel(0x1, CONFIG_ROCKCHIP_STIMER_BASE + 0x4);
}
#endif

#ifndef CONFIG_TPL_BUILD
int arch_cpu_init(void)
{
#ifdef CONFIG_SPL_BUILD
	int secure_reg;

	/* Set the SDMMC eMMC crypto_ns FSPI access secure area */
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST5_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST5_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST13_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST13_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST21_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST21_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST26_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST26_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST27_REG);
	secure_reg &= 0xffff0000;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST27_REG);

	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST5_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST5_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST13_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST13_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST21_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST21_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST26_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST26_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST27_REG);
	secure_reg &= 0xffff0000;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST27_REG);
#endif

	return 0;
}
#endif
