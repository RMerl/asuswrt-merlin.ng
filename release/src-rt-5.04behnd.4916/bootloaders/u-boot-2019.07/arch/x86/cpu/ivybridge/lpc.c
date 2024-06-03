// SPDX-License-Identifier: GPL-2.0
/*
 * From coreboot southbridge/intel/bd82x6x/lpc.c
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <rtc.h>
#include <pci.h>
#include <asm/intel_regs.h>
#include <asm/interrupt.h>
#include <asm/io.h>
#include <asm/ioapic.h>
#include <asm/lpc_common.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>

DECLARE_GLOBAL_DATA_PTR;

#define NMI_OFF				0

#define ENABLE_ACPI_MODE_IN_COREBOOT	0
#define TEST_SMM_FLASH_LOCKDOWN		0

static int pch_enable_apic(struct udevice *pch)
{
	u32 reg32;
	int i;

	/* Enable ACPI I/O and power management. Set SCI IRQ to IRQ9 */
	dm_pci_write_config8(pch, ACPI_CNTL, 0x80);

	writel(0, IO_APIC_INDEX);
	writel(1 << 25, IO_APIC_DATA);

	/* affirm full set of redirection table entries ("write once") */
	writel(1, IO_APIC_INDEX);
	reg32 = readl(IO_APIC_DATA);
	writel(1, IO_APIC_INDEX);
	writel(reg32, IO_APIC_DATA);

	writel(0, IO_APIC_INDEX);
	reg32 = readl(IO_APIC_DATA);
	debug("PCH APIC ID = %x\n", (reg32 >> 24) & 0x0f);
	if (reg32 != (1 << 25)) {
		printf("APIC Error - cannot write to registers\n");
		return -EPERM;
	}

	debug("Dumping IOAPIC registers\n");
	for (i = 0;  i < 3; i++) {
		writel(i, IO_APIC_INDEX);
		debug("  reg 0x%04x:", i);
		reg32 = readl(IO_APIC_DATA);
		debug(" 0x%08x\n", reg32);
	}

	/* Select Boot Configuration register. */
	writel(3, IO_APIC_INDEX);

	/* Use Processor System Bus to deliver interrupts. */
	writel(1, IO_APIC_DATA);

	return 0;
}

static void pch_enable_serial_irqs(struct udevice *pch)
{
	u32 value;

	/* Set packet length and toggle silent mode bit for one frame. */
	value = (1 << 7) | (1 << 6) | ((21 - 17) << 2) | (0 << 0);
#ifdef CONFIG_SERIRQ_CONTINUOUS_MODE
	dm_pci_write_config8(pch, SERIRQ_CNTL, value);
#else
	dm_pci_write_config8(pch, SERIRQ_CNTL, value | (1 << 6));
#endif
}

static int pch_pirq_init(struct udevice *pch)
{
	uint8_t route[8], *ptr;

	if (fdtdec_get_byte_array(gd->fdt_blob, dev_of_offset(pch),
				  "intel,pirq-routing", route, sizeof(route)))
		return -EINVAL;
	ptr = route;
	dm_pci_write_config8(pch, PIRQA_ROUT, *ptr++);
	dm_pci_write_config8(pch, PIRQB_ROUT, *ptr++);
	dm_pci_write_config8(pch, PIRQC_ROUT, *ptr++);
	dm_pci_write_config8(pch, PIRQD_ROUT, *ptr++);

	dm_pci_write_config8(pch, PIRQE_ROUT, *ptr++);
	dm_pci_write_config8(pch, PIRQF_ROUT, *ptr++);
	dm_pci_write_config8(pch, PIRQG_ROUT, *ptr++);
	dm_pci_write_config8(pch, PIRQH_ROUT, *ptr++);

	/*
	 * TODO(sjg@chromium.org): U-Boot does not set up the interrupts
	 * here. It's unclear if it is needed
	 */
	return 0;
}

static int pch_gpi_routing(struct udevice *pch)
{
	u8 route[16];
	u32 reg;
	int gpi;

	if (fdtdec_get_byte_array(gd->fdt_blob, dev_of_offset(pch),
				  "intel,gpi-routing", route, sizeof(route)))
		return -EINVAL;

	for (reg = 0, gpi = 0; gpi < ARRAY_SIZE(route); gpi++)
		reg |= route[gpi] << (gpi * 2);

	dm_pci_write_config32(pch, 0xb8, reg);

	return 0;
}

static int pch_power_options(struct udevice *pch)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(pch);
	u8 reg8;
	u16 reg16, pmbase;
	u32 reg32;
	const char *state;
	int pwr_on;
	int nmi_option;
	int ret;

	/*
	 * Which state do we want to goto after g3 (power restored)?
	 * 0 == S0 Full On
	 * 1 == S5 Soft Off
	 *
	 * If the option is not existent (Laptops), use Kconfig setting.
	 * TODO(sjg@chromium.org): Make this configurable
	 */
	pwr_on = MAINBOARD_POWER_ON;

	dm_pci_read_config16(pch, GEN_PMCON_3, &reg16);
	reg16 &= 0xfffe;
	switch (pwr_on) {
	case MAINBOARD_POWER_OFF:
		reg16 |= 1;
		state = "off";
		break;
	case MAINBOARD_POWER_ON:
		reg16 &= ~1;
		state = "on";
		break;
	case MAINBOARD_POWER_KEEP:
		reg16 &= ~1;
		state = "state keep";
		break;
	default:
		state = "undefined";
	}

	reg16 &= ~(3 << 4);	/* SLP_S4# Assertion Stretch 4s */
	reg16 |= (1 << 3);	/* SLP_S4# Assertion Stretch Enable */

	reg16 &= ~(1 << 10);
	reg16 |= (1 << 11);	/* SLP_S3# Min Assertion Width 50ms */

	reg16 |= (1 << 12);	/* Disable SLP stretch after SUS well */

	dm_pci_write_config16(pch, GEN_PMCON_3, reg16);
	debug("Set power %s after power failure.\n", state);

	/* Set up NMI on errors. */
	reg8 = inb(0x61);
	reg8 &= 0x0f;		/* Higher Nibble must be 0 */
	reg8 &= ~(1 << 3);	/* IOCHK# NMI Enable */
	reg8 |= (1 << 2); /* PCI SERR# Disable for now */
	outb(reg8, 0x61);

	reg8 = inb(0x70);
	/* TODO(sjg@chromium.org): Make this configurable */
	nmi_option = NMI_OFF;
	if (nmi_option) {
		debug("NMI sources enabled.\n");
		reg8 &= ~(1 << 7);	/* Set NMI. */
	} else {
		debug("NMI sources disabled.\n");
		/* Can't mask NMI from PCI-E and NMI_NOW */
		reg8 |= (1 << 7);
	}
	outb(reg8, 0x70);

	/* Enable CPU_SLP# and Intel Speedstep, set SMI# rate down */
	dm_pci_read_config16(pch, GEN_PMCON_1, &reg16);
	reg16 &= ~(3 << 0);	/* SMI# rate 1 minute */
	reg16 &= ~(1 << 10);	/* Disable BIOS_PCI_EXP_EN for native PME */
#if DEBUG_PERIODIC_SMIS
	/* Set DEBUG_PERIODIC_SMIS in pch.h to debug using periodic SMIs */
	reg16 |= (3 << 0);	/* Periodic SMI every 8s */
#endif
	dm_pci_write_config16(pch, GEN_PMCON_1, reg16);

	/* Set the board's GPI routing. */
	ret = pch_gpi_routing(pch);
	if (ret)
		return ret;

	dm_pci_read_config16(pch, 0x40, &pmbase);
	pmbase &= 0xfffe;

	writel(fdtdec_get_int(blob, node, "intel,gpe0-enable", 0),
	       (ulong)pmbase + GPE0_EN);
	writew(fdtdec_get_int(blob, node, "intel,alt-gp-smi-enable", 0),
	       (ulong)pmbase + ALT_GP_SMI_EN);

	/* Set up power management block and determine sleep mode */
	reg32 = inl(pmbase + 0x04); /* PM1_CNT */
	reg32 &= ~(7 << 10);	/* SLP_TYP */
	reg32 |= (1 << 0);	/* SCI_EN */
	outl(reg32, pmbase + 0x04);

	/* Clear magic status bits to prevent unexpected wake */
	setbits_le32(RCB_REG(0x3310), (1 << 4) | (1 << 5) | (1 << 0));
	clrbits_le32(RCB_REG(0x3f02), 0xf);

	return 0;
}

static void pch_rtc_init(struct udevice *pch)
{
	int rtc_failed;
	u8 reg8;

	dm_pci_read_config8(pch, GEN_PMCON_3, &reg8);
	rtc_failed = reg8 & RTC_BATTERY_DEAD;
	if (rtc_failed) {
		reg8 &= ~RTC_BATTERY_DEAD;
		dm_pci_write_config8(pch, GEN_PMCON_3, reg8);
	}
	debug("rtc_failed = 0x%x\n", rtc_failed);

	/* TODO: Handle power failure */
	if (rtc_failed)
		printf("RTC power failed\n");
}

/* CougarPoint PCH Power Management init */
static void cpt_pm_init(struct udevice *pch)
{
	debug("CougarPoint PM init\n");
	dm_pci_write_config8(pch, 0xa9, 0x47);
	setbits_le32(RCB_REG(0x2238), (1 << 6) | (1 << 0));

	setbits_le32(RCB_REG(0x228c), 1 << 0);
	setbits_le32(RCB_REG(0x1100), (1 << 13) | (1 << 14));
	setbits_le32(RCB_REG(0x0900), 1 << 14);
	writel(0xc0388400, RCB_REG(0x2304));
	setbits_le32(RCB_REG(0x2314), (1 << 5) | (1 << 18));
	setbits_le32(RCB_REG(0x2320), (1 << 15) | (1 << 1));
	clrsetbits_le32(RCB_REG(0x3314), ~0x1f, 0xf);
	writel(0x050f0000, RCB_REG(0x3318));
	writel(0x04000000, RCB_REG(0x3324));
	setbits_le32(RCB_REG(0x3340), 0xfffff);
	setbits_le32(RCB_REG(0x3344), 1 << 1);

	writel(0x0001c000, RCB_REG(0x3360));
	writel(0x00061100, RCB_REG(0x3368));
	writel(0x7f8fdfff, RCB_REG(0x3378));
	writel(0x000003fc, RCB_REG(0x337c));
	writel(0x00001000, RCB_REG(0x3388));
	writel(0x0001c000, RCB_REG(0x3390));
	writel(0x00000800, RCB_REG(0x33a0));
	writel(0x00001000, RCB_REG(0x33b0));
	writel(0x00093900, RCB_REG(0x33c0));
	writel(0x24653002, RCB_REG(0x33cc));
	writel(0x062108fe, RCB_REG(0x33d0));
	clrsetbits_le32(RCB_REG(0x33d4), 0x0fff0fff, 0x00670060);
	writel(0x01010000, RCB_REG(0x3a28));
	writel(0x01010404, RCB_REG(0x3a2c));
	writel(0x01041041, RCB_REG(0x3a80));
	clrsetbits_le32(RCB_REG(0x3a84), 0x0000ffff, 0x00001001);
	setbits_le32(RCB_REG(0x3a84), 1 << 24); /* SATA 2/3 disabled */
	setbits_le32(RCB_REG(0x3a88), 1 << 0);  /* SATA 4/5 disabled */
	writel(0x00000001, RCB_REG(0x3a6c));
	clrsetbits_le32(RCB_REG(0x2344), ~0x00ffff00, 0xff00000c);
	clrsetbits_le32(RCB_REG(0x80c), 0xff << 20, 0x11 << 20);
	writel(0, RCB_REG(0x33c8));
	setbits_le32(RCB_REG(0x21b0), 0xf);
}

/* PantherPoint PCH Power Management init */
static void ppt_pm_init(struct udevice *pch)
{
	debug("PantherPoint PM init\n");
	dm_pci_write_config8(pch, 0xa9, 0x47);
	setbits_le32(RCB_REG(0x2238), 1 << 0);
	setbits_le32(RCB_REG(0x228c), 1 << 0);
	setbits_le16(RCB_REG(0x1100), (1 << 13) | (1 << 14));
	setbits_le16(RCB_REG(0x0900), 1 << 14);
	writel(0xc03b8400, RCB_REG(0x2304));
	setbits_le32(RCB_REG(0x2314), (1 << 5) | (1 << 18));
	setbits_le32(RCB_REG(0x2320), (1 << 15) | (1 << 1));
	clrsetbits_le32(RCB_REG(0x3314), 0x1f, 0xf);
	writel(0x054f0000, RCB_REG(0x3318));
	writel(0x04000000, RCB_REG(0x3324));
	setbits_le32(RCB_REG(0x3340), 0xfffff);
	setbits_le32(RCB_REG(0x3344), (1 << 1) | (1 << 0));
	writel(0x0001c000, RCB_REG(0x3360));
	writel(0x00061100, RCB_REG(0x3368));
	writel(0x7f8fdfff, RCB_REG(0x3378));
	writel(0x000003fd, RCB_REG(0x337c));
	writel(0x00001000, RCB_REG(0x3388));
	writel(0x0001c000, RCB_REG(0x3390));
	writel(0x00000800, RCB_REG(0x33a0));
	writel(0x00001000, RCB_REG(0x33b0));
	writel(0x00093900, RCB_REG(0x33c0));
	writel(0x24653002, RCB_REG(0x33cc));
	writel(0x067388fe, RCB_REG(0x33d0));
	clrsetbits_le32(RCB_REG(0x33d4), 0x0fff0fff, 0x00670060);
	writel(0x01010000, RCB_REG(0x3a28));
	writel(0x01010404, RCB_REG(0x3a2c));
	writel(0x01040000, RCB_REG(0x3a80));
	clrsetbits_le32(RCB_REG(0x3a84), 0x0000ffff, 0x00001001);
	/* SATA 2/3 disabled */
	setbits_le32(RCB_REG(0x3a84), 1 << 24);
	/* SATA 4/5 disabled */
	setbits_le32(RCB_REG(0x3a88), 1 << 0);
	writel(0x00000001, RCB_REG(0x3a6c));
	clrsetbits_le32(RCB_REG(0x2344), 0xff0000ff, 0xff00000c);
	clrsetbits_le32(RCB_REG(0x80c), 0xff << 20, 0x11 << 20);
	setbits_le32(RCB_REG(0x33a4), (1 << 0));
	writel(0, RCB_REG(0x33c8));
	setbits_le32(RCB_REG(0x21b0), 0xf);
}

static void enable_hpet(void)
{
	/* Move HPET to default address 0xfed00000 and enable it */
	clrsetbits_le32(RCB_REG(HPTC), 3 << 0, 1 << 7);
}

static void enable_clock_gating(struct udevice *pch)
{
	u32 reg32;
	u16 reg16;

	setbits_le32(RCB_REG(0x2234), 0xf);

	dm_pci_read_config16(pch, GEN_PMCON_1, &reg16);
	reg16 |= (1 << 2) | (1 << 11);
	dm_pci_write_config16(pch, GEN_PMCON_1, reg16);

	pch_iobp_update(pch, 0xeb007f07, ~0U, 1 << 31);
	pch_iobp_update(pch, 0xeb004000, ~0U, 1 << 7);
	pch_iobp_update(pch, 0xec007f07, ~0U, 1 << 31);
	pch_iobp_update(pch, 0xec004000, ~0U, 1 << 7);

	reg32 = readl(RCB_REG(CG));
	reg32 |= (1 << 31);
	reg32 |= (1 << 29) | (1 << 28);
	reg32 |= (1 << 27) | (1 << 26) | (1 << 25) | (1 << 24);
	reg32 |= (1 << 16);
	reg32 |= (1 << 17);
	reg32 |= (1 << 18);
	reg32 |= (1 << 22);
	reg32 |= (1 << 23);
	reg32 &= ~(1 << 20);
	reg32 |= (1 << 19);
	reg32 |= (1 << 0);
	reg32 |= (0xf << 1);
	writel(reg32, RCB_REG(CG));

	setbits_le32(RCB_REG(0x38c0), 0x7);
	setbits_le32(RCB_REG(0x36d4), 0x6680c004);
	setbits_le32(RCB_REG(0x3564), 0x3);
}

static void pch_disable_smm_only_flashing(struct udevice *pch)
{
	u8 reg8;

	debug("Enabling BIOS updates outside of SMM... ");
	dm_pci_read_config8(pch, 0xdc, &reg8);	/* BIOS_CNTL */
	reg8 &= ~(1 << 5);
	dm_pci_write_config8(pch, 0xdc, reg8);
}

static void pch_fixups(struct udevice *pch)
{
	u8 gen_pmcon_2;

	/* Indicate DRAM init done for MRC S3 to know it can resume */
	dm_pci_read_config8(pch, GEN_PMCON_2, &gen_pmcon_2);
	gen_pmcon_2 |= (1 << 7);
	dm_pci_write_config8(pch, GEN_PMCON_2, gen_pmcon_2);

	/* Enable DMI ASPM in the PCH */
	clrbits_le32(RCB_REG(0x2304), 1 << 10);
	setbits_le32(RCB_REG(0x21a4), (1 << 11) | (1 << 10));
	setbits_le32(RCB_REG(0x21a8), 0x3);
}

static void set_spi_speed(void)
{
	u32 fdod;

	/* Observe SPI Descriptor Component Section 0 */
	writel(0x1000, RCB_REG(SPI_DESC_COMP0));

	/* Extract the1 Write/Erase SPI Frequency from descriptor */
	fdod = readl(RCB_REG(SPI_FREQ_WR_ERA));
	fdod >>= 24;
	fdod &= 7;

	/* Set Software Sequence frequency to match */
	clrsetbits_8(RCB_REG(SPI_FREQ_SWSEQ), 7, fdod);
}

static int lpc_init_extra(struct udevice *dev)
{
	struct udevice *pch = dev->parent;

	debug("pch: lpc_init\n");
	dm_pci_write_bar32(pch, 0, 0);
	dm_pci_write_bar32(pch, 1, 0xff800000);
	dm_pci_write_bar32(pch, 2, 0xfec00000);
	dm_pci_write_bar32(pch, 3, 0x800);
	dm_pci_write_bar32(pch, 4, 0x900);

	/* Set the value for PCI command register. */
	dm_pci_write_config16(pch, PCI_COMMAND, 0x000f);

	/* IO APIC initialization. */
	pch_enable_apic(pch);

	pch_enable_serial_irqs(pch);

	/* Setup the PIRQ. */
	pch_pirq_init(pch);

	/* Setup power options. */
	pch_power_options(pch);

	/* Initialize power management */
	switch (pch_silicon_type(pch)) {
	case PCH_TYPE_CPT: /* CougarPoint */
		cpt_pm_init(pch);
		break;
	case PCH_TYPE_PPT: /* PantherPoint */
		ppt_pm_init(pch);
		break;
	default:
		printf("Unknown Chipset: %s\n", pch->name);
		return -ENOSYS;
	}

	/* Initialize the real time clock. */
	pch_rtc_init(pch);

	/* Initialize the High Precision Event Timers, if present. */
	enable_hpet();

	/* Initialize Clock Gating */
	enable_clock_gating(pch);

	pch_disable_smm_only_flashing(pch);

	pch_fixups(pch);

	return 0;
}

static int bd82x6x_lpc_early_init(struct udevice *dev)
{
	set_spi_speed();

	/* Setting up Southbridge. In the northbridge code. */
	debug("Setting up static southbridge registers\n");
	dm_pci_write_config32(dev->parent, PCH_RCBA_BASE,
			      RCB_BASE_ADDRESS | 1);
	dm_pci_write_config32(dev->parent, PMBASE, DEFAULT_PMBASE | 1);

	/* Enable ACPI BAR */
	dm_pci_write_config8(dev->parent, ACPI_CNTL, 0x80);

	debug("Disabling watchdog reboot\n");
	setbits_le32(RCB_REG(GCS), 1 >> 5);	/* No reset */
	outw(1 << 11, DEFAULT_PMBASE | 0x60 | 0x08);	/* halt timer */

	dm_pci_write_config32(dev->parent, GPIO_BASE, DEFAULT_GPIOBASE | 1);
	dm_pci_write_config32(dev->parent, GPIO_CNTL, 0x10);

	return 0;
}

static int bd82x6x_lpc_probe(struct udevice *dev)
{
	int ret;

	if (!(gd->flags & GD_FLG_RELOC)) {
		ret = lpc_common_early_init(dev);
		if (ret) {
			debug("%s: lpc_early_init() failed\n", __func__);
			return ret;
		}

		return bd82x6x_lpc_early_init(dev);
	}

	return lpc_init_extra(dev);
}

static const struct udevice_id bd82x6x_lpc_ids[] = {
	{ .compatible = "intel,bd82x6x-lpc" },
	{ }
};

U_BOOT_DRIVER(bd82x6x_lpc_drv) = {
	.name		= "lpc",
	.id		= UCLASS_LPC,
	.of_match	= bd82x6x_lpc_ids,
	.probe		= bd82x6x_lpc_probe,
};
