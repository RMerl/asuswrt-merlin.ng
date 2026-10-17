// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/types.h>
#include <dm.h>

/* System Bus Controller registers */
#define UNIPHIER_SBC_BASE	0x100	/* base address of bank0 space */
#define    UNIPHIER_SBC_BASE_BE		BIT(0)	/* bank_enable */
#define UNIPHIER_SBC_CTRL0	0x200	/* timing parameter 0 of bank0 */
#define UNIPHIER_SBC_CTRL1	0x204	/* timing parameter 1 of bank0 */
#define UNIPHIER_SBC_CTRL2	0x208	/* timing parameter 2 of bank0 */
#define UNIPHIER_SBC_CTRL3	0x20c	/* timing parameter 3 of bank0 */
#define UNIPHIER_SBC_CTRL4	0x300	/* timing parameter 4 of bank0 */

#define UNIPHIER_SBC_STRIDE	0x10	/* register stride to next bank */

#if 1
/* slower but LED works */
#define SBCTRL0_VALUE	0x55450000
#define SBCTRL1_VALUE	0x07168d00
#define SBCTRL2_VALUE	0x34000009
#define SBCTRL4_VALUE	0x02110110

#else
/* faster but LED does not work */
#define SBCTRL0_VALUE	0x55450000
#define SBCTRL1_VALUE	0x06057700
/* NOR flash needs more wait counts than SRAM */
#define SBCTRL2_VALUE	0x34000009
#define SBCTRL4_VALUE	0x02110210
#endif

void uniphier_system_bus_set_reg(void __iomem *membase)
{
	void __iomem *bank0_base = membase;
	void __iomem *bank1_base = membase + UNIPHIER_SBC_STRIDE;

	/*
	 * Only CS1 is connected to support card.
	 * BKSZ[1:0] should be set to "01".
	 */
	writel(SBCTRL0_VALUE, bank1_base + UNIPHIER_SBC_CTRL0);
	writel(SBCTRL1_VALUE, bank1_base + UNIPHIER_SBC_CTRL1);
	writel(SBCTRL2_VALUE, bank1_base + UNIPHIER_SBC_CTRL2);
	writel(SBCTRL4_VALUE, bank1_base + UNIPHIER_SBC_CTRL4);

	if (readl(bank1_base + UNIPHIER_SBC_BASE) & UNIPHIER_SBC_BASE_BE) {
		/*
		 * Boot Swap On: boot from external NOR/SRAM
		 * 0x42000000-0x43ffffff is a mirror of 0x40000000-0x41ffffff.
		 *
		 * 0x40000000-0x41efffff, 0x42000000-0x43efffff: memory bank
		 * 0x41f00000-0x41ffffff, 0x43f00000-0x43ffffff: peripherals
		 */
		writel(0x0000bc01, bank0_base + UNIPHIER_SBC_BASE);
	} else {
		/*
		 * Boot Swap Off: boot from mask ROM
		 * 0x40000000-0x41ffffff: mask ROM
		 * 0x42000000-0x43efffff: memory bank (31MB)
		 * 0x43f00000-0x43ffffff: peripherals (1MB)
		 */
		writel(0x0000be01, bank0_base + UNIPHIER_SBC_BASE); /* dummy */
		writel(0x0200be01, bank0_base + UNIPHIER_SBC_BASE);
	}
}

static int uniphier_system_bus_probe(struct udevice *dev)
{
	fdt_addr_t base;
	void __iomem *membase;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	membase = devm_ioremap(dev, base, SZ_1K);
	if (!membase)
		return -ENOMEM;

	uniphier_system_bus_set_reg(membase);

	return 0;
}

static const struct udevice_id uniphier_system_bus_match[] = {
	{ .compatible = "socionext,uniphier-system-bus" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_system_bus_driver) = {
	.name	= "uniphier-system-bus",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = uniphier_system_bus_match,
	.probe = uniphier_system_bus_probe,
};
