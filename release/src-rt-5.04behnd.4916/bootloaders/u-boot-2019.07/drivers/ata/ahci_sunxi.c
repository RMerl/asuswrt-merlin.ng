#include <common.h>
#include <ahci.h>
#include <dm.h>
#include <scsi.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/gpio.h>

#define AHCI_PHYCS0R 0x00c0
#define AHCI_PHYCS1R 0x00c4
#define AHCI_PHYCS2R 0x00c8
#define AHCI_RWCR    0x00fc

/* This magic PHY initialisation was taken from the Allwinner releases
 * and Linux driver, but is completely undocumented.
 */
static int sunxi_ahci_phy_init(u8 *reg_base)
{
	u32 reg_val;
	int timeout;

	writel(0, reg_base + AHCI_RWCR);
	mdelay(5);

	setbits_le32(reg_base + AHCI_PHYCS1R, 0x1 << 19);
	clrsetbits_le32(reg_base + AHCI_PHYCS0R,
			(0x7 << 24),
			(0x5 << 24) | (0x1 << 23) | (0x1 << 18));
	clrsetbits_le32(reg_base + AHCI_PHYCS1R,
			(0x3 << 16) | (0x1f << 8) | (0x3 << 6),
			(0x2 << 16) | (0x6 << 8) | (0x2 << 6));
	setbits_le32(reg_base + AHCI_PHYCS1R, (0x1 << 28) | (0x1 << 15));
	clrbits_le32(reg_base + AHCI_PHYCS1R, (0x1 << 19));
	clrsetbits_le32(reg_base + AHCI_PHYCS0R, (0x7 << 20), (0x3 << 20));
	clrsetbits_le32(reg_base + AHCI_PHYCS2R, (0x1f << 5), (0x19 << 5));
	mdelay(5);

	setbits_le32(reg_base + AHCI_PHYCS0R, (0x1 << 19));

	timeout = 250; /* Power up takes approx 50 us */
	for (;;) {
		reg_val = readl(reg_base + AHCI_PHYCS0R) & (0x7 << 28);
		if (reg_val == (0x2 << 28))
			break;
		if (--timeout == 0) {
			printf("AHCI PHY power up failed.\n");
			return -EIO;
		}
		udelay(1);
	};

	setbits_le32(reg_base + AHCI_PHYCS2R, (0x1 << 24));

	timeout = 100; /* Calibration takes approx 10 us */
	for (;;) {
		reg_val = readl(reg_base + AHCI_PHYCS2R) & (0x1 << 24);
		if (reg_val == 0x0)
			break;
		if (--timeout == 0) {
			printf("AHCI PHY calibration failed.\n");
			return -EIO;
		}
		udelay(1);
	}

	mdelay(15);

	writel(0x7, reg_base + AHCI_RWCR);

	return 0;
}

static int sunxi_sata_probe(struct udevice *dev)
{
	ulong base;
	u8 *reg;
	int ret;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE) {
		debug("%s: Failed to find address (err=%d\n)", __func__, ret);
		return -EINVAL;
	}
	reg = (u8 *)base;
	ret = sunxi_ahci_phy_init(reg);
	if (ret) {
		debug("%s: Failed to init phy (err=%d\n)", __func__, ret);
		return ret;
	}
	ret = ahci_probe_scsi(dev, base);
	if (ret) {
		debug("%s: Failed to probe (err=%d\n)", __func__, ret);
		return ret;
	}

	return 0;
}

static int sunxi_sata_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;
	int ret;

	ret = ahci_bind_scsi(dev, &scsi_dev);
	if (ret) {
		debug("%s: Failed to bind (err=%d\n)", __func__, ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id sunxi_ahci_ids[] = {
	{ .compatible = "allwinner,sun4i-a10-ahci" },
	{ .compatible = "allwinner,sun8i-r40-ahci" },
	{ }
};

U_BOOT_DRIVER(ahci_sunxi_drv) = {
	.name		= "ahci_sunxi",
	.id		= UCLASS_AHCI,
	.of_match	= sunxi_ahci_ids,
	.bind		= sunxi_sata_bind,
	.probe		= sunxi_sata_probe,
};
