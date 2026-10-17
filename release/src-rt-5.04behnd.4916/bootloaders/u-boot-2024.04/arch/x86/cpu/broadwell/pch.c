// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pch.h>
#include <asm/cpu.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/i8259.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/ioapic.h>
#include <asm/lpc_common.h>
#include <asm/pch_common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/iomap.h>
#include <asm/arch/pch.h>
#include <asm/arch/pm.h>
#include <asm/arch/rcb.h>
#include <asm/arch/serialio.h>
#include <asm/arch/spi.h>
#include <dm/uclass-internal.h>
#include <linux/delay.h>

#define BIOS_CTRL	0xdc

bool cpu_is_ult(void)
{
	u32 fm = cpu_get_family_model();

	return fm == BROADWELL_FAMILY_ULT || fm == HASWELL_FAMILY_ULT;
}

static int broadwell_pch_early_init(struct udevice *dev)
{
	struct gpio_desc desc;
	struct udevice *bus;
	pci_dev_t bdf;
	int ret;

	dm_pci_write_config32(dev, PCH_RCBA, RCB_BASE_ADDRESS | 1);

	dm_pci_write_config32(dev, PMBASE, ACPI_BASE_ADDRESS | 1);
	dm_pci_write_config8(dev, ACPI_CNTL, ACPI_EN);
	dm_pci_write_config32(dev, GPIO_BASE, GPIO_BASE_ADDRESS | 1);
	dm_pci_write_config8(dev, GPIO_CNTL, GPIO_EN);

	/* Enable IOAPIC */
	writew(0x1000, RCB_REG(OIC));
	/* Read back for posted write */
	readw(RCB_REG(OIC));

	/* Set HPET address and enable it */
	clrsetbits_le32(RCB_REG(HPTC), 3, 1 << 7);
	/* Read back for posted write */
	readl(RCB_REG(HPTC));
	/* Enable HPET to start counter */
	setbits_le32(HPET_BASE_ADDRESS + 0x10, 1 << 0);

	setbits_le32(RCB_REG(GCS), 1 << 5);

	/*
	 * Enable PP3300_AUTOBAHN_EN after initial GPIO setup
	 * to prevent possible brownout. This will cause the GPIOs to be set
	 * up if it has not been done already.
	 */
	ret = gpio_request_by_name(dev, "power-enable-gpio", 0, &desc,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret)
		return ret;

	/* 8.14 Additional PCI Express Programming Steps, step #1 */
	bdf = PCI_BDF(0, 0x1c, 0);
	bus = pci_get_controller(dev);
	pci_bus_clrset_config32(bus, bdf, 0xf4, 0x60, 0);
	pci_bus_clrset_config32(bus, bdf, 0xf4, 0x80, 0x80);
	pci_bus_clrset_config32(bus, bdf, 0xe2, 0x30, 0x30);

	return 0;
}

static void pch_misc_init(struct udevice *dev)
{
	/* Setup SLP signal assertion, SLP_S4=4s, SLP_S3=50ms */
	dm_pci_clrset_config8(dev, GEN_PMCON_3, 3 << 4 | 1 << 10,
			      1 << 3 | 1 << 11 | 1 << 12);
	/* Prepare sleep mode */
	clrsetio_32(ACPI_BASE_ADDRESS + PM1_CNT, SLP_TYP, SCI_EN);

	/* Setup NMI on errors, disable SERR */
	clrsetio_8(0x61, 0xf0, 1 << 2);
	/* Disable NMI sources */
	setio_8(0x70, 1 << 7);
	/* Indicate DRAM init done for MRC */
	dm_pci_clrset_config8(dev, GEN_PMCON_2, 0, 1 << 7);

	/* Clear status bits to prevent unexpected wake */
	setbits_le32(RCB_REG(0x3310), 0x0000002f);
	clrsetbits_le32(RCB_REG(0x3f02), 0x0000000f, 0);
	/* Enable PCIe Relaxed Order */
	setbits_le32(RCB_REG(0x2314), 1 << 31 | 1 << 7);
	setbits_le32(RCB_REG(0x1114), 1 << 15 | 1 << 14);
	/* Setup SERIRQ, enable continuous mode */
	dm_pci_clrset_config8(dev, SERIRQ_CNTL, 0, 1 << 7 | 1 << 6);
};

static void pch_enable_ioapic(void)
{
	u32 reg32;

	/* Make sure this is a unique ID within system */
	io_apic_set_id(0x04);

	/* affirm full set of redirection table entries ("write once") */
	reg32 = io_apic_read(0x01);

	/* PCH-LP has 39 redirection entries */
	reg32 &= ~0x00ff0000;
	reg32 |= 0x00270000;

	io_apic_write(0x01, reg32);

	/*
	 * Select Boot Configuration register (0x03) and
	 * use Processor System Bus (0x01) to deliver interrupts.
	 */
	io_apic_write(0x03, 0x01);
}

/* Enable all requested GPE */
void enable_all_gpe(u32 set1, u32 set2, u32 set3, u32 set4)
{
	outl(set1, ACPI_BASE_ADDRESS + GPE0_EN(GPE_31_0));
	outl(set2, ACPI_BASE_ADDRESS + GPE0_EN(GPE_63_32));
	outl(set3, ACPI_BASE_ADDRESS + GPE0_EN(GPE_94_64));
	outl(set4, ACPI_BASE_ADDRESS + GPE0_EN(GPE_STD));
}

/*
 * Enable GPIO SMI events - it would be good to put this in the GPIO driver
 * but it would need a new driver operation.
 */
int enable_alt_smi(struct udevice *pch, u32 mask)
{
	struct pch_lp_gpio_regs *regs;
	u32 gpiobase;
	int ret;

	ret = pch_get_gpio_base(pch, &gpiobase);
	if (ret) {
		debug("%s: invalid GPIOBASE address (%08x)\n", __func__,
		      gpiobase);
		return -EINVAL;
	}

	regs = (struct pch_lp_gpio_regs *)gpiobase;
	setio_32(regs->alt_gpi_smi_en, mask);

	return 0;
}

static int pch_power_options(struct udevice *dev)
{
	int pwr_on_after_power_fail = MAINBOARD_POWER_OFF;
	const char *state;
	u32 enable[4];
	u16 reg16;
	int ret;

	dm_pci_read_config16(dev, GEN_PMCON_3, &reg16);
	reg16 &= 0xfffe;
	switch (pwr_on_after_power_fail) {
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
	dm_pci_write_config16(dev, GEN_PMCON_3, reg16);
	debug("Set power %s after power failure.\n", state);

	/* GPE setup based on device tree configuration */
	ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(dev),
				   "intel,gpe0-en", enable, ARRAY_SIZE(enable));
	if (ret)
		return -EINVAL;
	enable_all_gpe(enable[0], enable[1], enable[2], enable[3]);

	/* SMI setup based on device tree configuration */
	enable_alt_smi(dev, fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					   "intel,alt-gp-smi-enable", 0));

	return 0;
}

/* Magic register settings for power management */
static void pch_pm_init_magic(struct udevice *dev)
{
	dm_pci_write_config8(dev, 0xa9, 0x46);
	clrbits_le32(RCB_REG(0x232c), 1),
	setbits_le32(RCB_REG(0x1100), 0x0000c13f);
	clrsetbits_le32(RCB_REG(0x2320), 0x60, 0x10);
	writel(0x00012fff, RCB_REG(0x3314));
	clrsetbits_le32(RCB_REG(0x3318), 0x000f0330, 0x0dcf0400);
	writel(0x04000000, RCB_REG(0x3324));
	writel(0x00041400, RCB_REG(0x3368));
	writel(0x3f8ddbff, RCB_REG(0x3388));
	writel(0x00007001, RCB_REG(0x33ac));
	writel(0x00181900, RCB_REG(0x33b0));
	writel(0x00060A00, RCB_REG(0x33c0));
	writel(0x06200840, RCB_REG(0x33d0));
	writel(0x01010101, RCB_REG(0x3a28));
	writel(0x040c0404, RCB_REG(0x3a2c));
	writel(0x9000000a, RCB_REG(0x3a9c));
	writel(0x03808033, RCB_REG(0x2b1c));
	writel(0x80000009, RCB_REG(0x2b34));
	writel(0x022ddfff, RCB_REG(0x3348));
	writel(0x00000001, RCB_REG(0x334c));
	writel(0x0001c000, RCB_REG(0x3358));
	writel(0x3f8ddbff, RCB_REG(0x3380));
	writel(0x0001c7e1, RCB_REG(0x3384));
	writel(0x0001c7e1, RCB_REG(0x338c));
	writel(0x0001c000, RCB_REG(0x3398));
	writel(0x00181900, RCB_REG(0x33a8));
	writel(0x00080000, RCB_REG(0x33dc));
	writel(0x00000001, RCB_REG(0x33e0));
	writel(0x0000040c, RCB_REG(0x3a20));
	writel(0x01010101, RCB_REG(0x3a24));
	writel(0x01010101, RCB_REG(0x3a30));
	dm_pci_clrset_config32(dev, 0xac, 0x00200000, 0);
	setbits_le32(RCB_REG(0x0410), 0x00000003);
	setbits_le32(RCB_REG(0x2618), 0x08000000);
	setbits_le32(RCB_REG(0x2300), 0x00000002);
	setbits_le32(RCB_REG(0x2600), 0x00000008);
	writel(0x00007001, RCB_REG(0x33b4));
	writel(0x022ddfff, RCB_REG(0x3350));
	writel(0x00000001, RCB_REG(0x3354));
	/* Power Optimizer */
	setbits_le32(RCB_REG(0x33d4), 0x08000000);
	/*
	 * This stops the LCD from turning on:
	 * setbits_le32(RCB_REG(0x33c8), 0x08000080);
	 */
	writel(0x0000883c, RCB_REG(0x2b10));
	writel(0x1e0a4616, RCB_REG(0x2b14));
	writel(0x40000005, RCB_REG(0x2b24));
	writel(0x0005db01, RCB_REG(0x2b20));
	writel(0x05145005, RCB_REG(0x3a80));
	writel(0x00001005, RCB_REG(0x3a84));
	setbits_le32(RCB_REG(0x33d4), 0x2fff2fb1);
	setbits_le32(RCB_REG(0x33c8), 0x00008000);
};

static int pch_type(struct udevice *dev)
{
	u16 type;

	dm_pci_read_config16(dev, PCI_DEVICE_ID, &type);

	return type;
}

/* Return 1 if PCH type is WildcatPoint */
static int pch_is_wpt(struct udevice *dev)
{
	return ((pch_type(dev) & 0xfff0) == 0x9cc0) ? 1 : 0;
}

/* Return 1 if PCH type is WildcatPoint ULX */
static int pch_is_wpt_ulx(struct udevice *dev)
{
	u16 lpcid = pch_type(dev);

	switch (lpcid) {
	case PCH_WPT_BDW_Y_SAMPLE:
	case PCH_WPT_BDW_Y_PREMIUM:
	case PCH_WPT_BDW_Y_BASE:
		return 1;
	}

	return 0;
}

static u32 pch_read_soft_strap(int id)
{
	clrbits_le32(SPI_REG(SPIBAR_FDOC), 0x00007ffc);
	setbits_le32(SPI_REG(SPIBAR_FDOC), 0x00004000 | id * 4);

	return readl(SPI_REG(SPIBAR_FDOD));
}

static void pch_enable_mphy(struct udevice *dev)
{
	u32 data_and = 0xffffffff;
	u32 data_or = (1 << 14) | (1 << 13) | (1 << 12);

	data_or |= (1 << 0);
	if (pch_is_wpt(dev)) {
		data_and &= ~((1 << 7) | (1 << 6) | (1 << 3));
		data_or |= (1 << 5) | (1 << 4);

		if (pch_is_wpt_ulx(dev)) {
			/* Check if SATA and USB3 MPHY are enabled */
			u32 strap19 = pch_read_soft_strap(19);
			strap19 &= ((1 << 31) | (1 << 30));
			strap19 >>= 30;
			if (strap19 == 3) {
				data_or |= (1 << 3);
				debug("Enable ULX MPHY PG control in single domain\n");
			} else if (strap19 == 0) {
				debug("Enable ULX MPHY PG control in split domains\n");
			} else {
				debug("Invalid PCH Soft Strap 19 configuration\n");
			}
		} else {
			data_or |= (1 << 3);
		}
	}

	pch_iobp_update(0xCF000000, data_and, data_or);
}

static void pch_init_deep_sx(bool deep_sx_enable_ac, bool deep_sx_enable_dc)
{
	if (deep_sx_enable_ac) {
		setbits_le32(RCB_REG(DEEP_S3_POL), DEEP_S3_EN_AC);
		setbits_le32(RCB_REG(DEEP_S5_POL), DEEP_S5_EN_AC);
	}

	if (deep_sx_enable_dc) {
		setbits_le32(RCB_REG(DEEP_S3_POL), DEEP_S3_EN_DC);
		setbits_le32(RCB_REG(DEEP_S5_POL), DEEP_S5_EN_DC);
	}

	if (deep_sx_enable_ac || deep_sx_enable_dc) {
		setbits_le32(RCB_REG(DEEP_SX_CONFIG),
			     DEEP_SX_WAKE_PIN_EN | DEEP_SX_GP27_PIN_EN);
	}
}

/* Power Management init */
static void pch_pm_init(struct udevice *dev)
{
	debug("PCH PM init\n");

	pch_init_deep_sx(false, false);
	pch_enable_mphy(dev);
	pch_pm_init_magic(dev);

	if (pch_is_wpt(dev)) {
		setbits_le32(RCB_REG(0x33e0), 1 << 4 | 1 << 1);
		setbits_le32(RCB_REG(0x2b1c), 1 << 22 | 1 << 14 | 1 << 13);
		writel(0x16bf0002, RCB_REG(0x33e4));
		setbits_le32(RCB_REG(0x33e4), 0x1);
	}

	pch_iobp_update(0xCA000000, ~0UL, 0x00000009);

	/* Set RCBA 0x2b1c[29]=1 if DSP disabled */
	if (readl(RCB_REG(FD)) & PCH_DISABLE_ADSPD)
		setbits_le32(RCB_REG(0x2b1c), 1 << 29);
}

static void pch_cg_init(struct udevice *dev)
{
	struct udevice *bus = pci_get_controller(dev);
	u32 reg32;
	u16 reg16;
	ulong val;

	/* DMI */
	setbits_le32(RCB_REG(0x2234), 0xf);

	dm_pci_read_config16(dev, GEN_PMCON_1, &reg16);
	reg16 &= ~(1 << 10); /* Disable BIOS_PCI_EXP_EN for native PME */
	if (pch_is_wpt(dev))
		reg16 &= ~(1 << 11);
	else
		reg16 |= 1 << 11;
	reg16 |= 1 << 5 | 1 << 6 | 1 << 7 | 1 << 12;
	reg16 |= 1 << 2; /* PCI CLKRUN# Enable */
	dm_pci_write_config16(dev, GEN_PMCON_1, reg16);

	/*
	 * RCBA + 0x2614[27:25,14:13,10,8] = 101,11,1,1
	 * RCBA + 0x2614[23:16] = 0x20
	 * RCBA + 0x2614[30:28] = 0x0
	 * RCBA + 0x2614[26] = 1 (IF 0:2.0@0x08 >= 0x0b)
	 */
	clrsetbits_le32(RCB_REG(0x2614), 0x64ff0000, 0x0a206500);

	/* Check for 0:2.0@0x08 >= 0x0b */
	pci_bus_read_config(bus, PCI_BDF(0, 0x2, 0), 0x8, &val, PCI_SIZE_8);
	if (pch_is_wpt(dev) || val >= 0x0b)
		setbits_le32(RCB_REG(0x2614), 1 << 26);

	setbits_le32(RCB_REG(0x900), 0x0000031f);

	reg32 = readl(RCB_REG(CG));
	if (readl(RCB_REG(0x3454)) & (1 << 4))
		reg32 &= ~(1 << 29); /* LPC Dynamic */
	else
		reg32 |= (1 << 29); /* LPC Dynamic */
	reg32 |= 1 << 31; /* LP LPC */
	reg32 |= 1 << 30; /* LP BLA */
	if (readl(RCB_REG(0x3454)) & (1 << 4))
		reg32 &= ~(1 << 29);
	else
		reg32 |= 1 << 29;
	reg32 |= 1 << 28; /* GPIO Dynamic */
	reg32 |= 1 << 27; /* HPET Dynamic */
	reg32 |= 1 << 26; /* Generic Platform Event Clock */
	if (readl(RCB_REG(BUC)) & PCH_DISABLE_GBE)
		reg32 |= 1 << 23; /* GbE Static */
	if (readl(RCB_REG(FD)) & PCH_DISABLE_HD_AUDIO)
		reg32 |= 1 << 21; /* HDA Static */
	reg32 |= 1 << 22; /* HDA Dynamic */
	writel(reg32, RCB_REG(CG));

	/* PCH-LP LPC */
	if (pch_is_wpt(dev))
		clrsetbits_le32(RCB_REG(0x3434), 0x1f, 0x17);
	else
		setbits_le32(RCB_REG(0x3434), 0x7);

	/* SPI */
	setbits_le32(RCB_REG(0x38c0), 0x3c07);

	pch_iobp_update(0xCE00C000, ~1UL, 0x00000000);
}

static void systemagent_init(void)
{
	/* Enable Power Aware Interrupt Routing */
	clrsetbits_8(MCHBAR_REG(MCH_PAIR), 0x7, 0x4); /* Fixed Priority */

	/*
	 * Set bits 0+1 of BIOS_RESET_CPL to indicate to the CPU
	 * that BIOS has initialized memory and power management
	 */
	setbits_8(MCHBAR_REG(BIOS_RESET_CPL), 3);
	debug("Set BIOS_RESET_CPL\n");

	/* Configure turbo power limits 1ms after reset complete bit */
	mdelay(1);

	cpu_set_power_limits(28);
}

/* Enable LTR Auto Mode for D21:F1-F6 */
static void serialio_d21_ltr(u32 bar0)
{
	/* 1. Program BAR0 + 808h[2] = 0b */
	clrbits_le32(bar0 + SIO_REG_PPR_GEN, SIO_REG_PPR_GEN_LTR_MODE_MASK);

	/* 2. Program BAR0 + 804h[1:0] = 00b */
	clrbits_le32(bar0 + SIO_REG_PPR_RST, SIO_REG_PPR_RST_ASSERT);

	/* 3. Program BAR0 + 804h[1:0] = 11b */
	setbits_le32(bar0 + SIO_REG_PPR_RST, SIO_REG_PPR_RST_ASSERT);

	/* 4. Program BAR0 + 814h[31:0] = 00000000h */
	writel(0, bar0 + SIO_REG_AUTO_LTR);
}

/* Select I2C voltage of 1.8V or 3.3V */
static void serialio_i2c_voltage_sel(u32 bar0, uint voltage)
{
	clrsetbits_le32(bar0 + SIO_REG_PPR_GEN, SIO_REG_PPR_GEN_VOLTAGE_MASK,
			SIO_REG_PPR_GEN_VOLTAGE(voltage));
}

/* Put Serial IO D21:F0-F6 device into desired mode */
static void serialio_d21_mode(int sio_index, int int_pin, bool acpi_mode)
{
	u32 portctrl = SIO_IOBP_PORTCTRL_PM_CAP_PRSNT;

	/* Snoop select 1 */
	portctrl |= SIO_IOBP_PORTCTRL_SNOOP_SELECT(1);

	/* Set interrupt pin */
	portctrl |= SIO_IOBP_PORTCTRL_INT_PIN(int_pin);

	if (acpi_mode) {
		/* Enable ACPI interrupt mode */
		portctrl |= SIO_IOBP_PORTCTRL_ACPI_IRQ_EN;
	}

	pch_iobp_update(SIO_IOBP_PORTCTRLX(sio_index), 0, portctrl);
}

/* Init sequence to be run once, done as part of D21:F0 (SDMA) init */
static void serialio_init_once(bool acpi_mode)
{
	if (acpi_mode) {
		/* Enable ACPI IRQ for IRQ13, IRQ7, IRQ6, IRQ5 in RCBA */
		setbits_le32(RCB_REG(ACPIIRQEN),
			     1 << 13 | 1 << 7 | 1 << 6 | 1 << 5);
	}

	/* Program IOBP CB000154h[12,9:8,4:0] = 1001100011111b */
	pch_iobp_update(SIO_IOBP_GPIODF, ~0x0000131f, 0x0000131f);

	/* Program IOBP CB000180h[5:0] = 111111b (undefined register) */
	pch_iobp_update(0xcb000180, ~0x0000003f, 0x0000003f);
}

/**
 * pch_serialio_init() - set up serial I/O devices
 *
 * Return: 0 if OK, -ve on error
 */
static int pch_serialio_init(void)
{
	struct udevice *dev, *hda;
	bool acpi_mode = true;
	u32 bar0, bar1;
	int ret;

	ret = uclass_find_first_device(UCLASS_I2C, &dev);
	if (ret)
		return ret;
	bar0 = dm_pci_read_bar32(dev, 0);
	if (!bar0)
		return -EINVAL;
	bar1 = dm_pci_read_bar32(dev, 1);
	if (!bar1)
		return -EINVAL;

	serialio_init_once(acpi_mode);
	serialio_d21_mode(SIO_ID_SDMA, SIO_PIN_INTB, acpi_mode);

	serialio_d21_ltr(bar0);
	serialio_i2c_voltage_sel(bar0, 1); /* Select 1.8V always */
	serialio_d21_mode(SIO_ID_I2C0, SIO_PIN_INTC, acpi_mode);
	setbits_le32(bar1 + PCH_PCS, PCH_PCS_PS_D3HOT);

	clrbits_le32(bar1 + PCH_PCS, PCH_PCS_PS_D3HOT);

	setbits_le32(bar0 + SIO_REG_PPR_CLOCK, SIO_REG_PPR_CLOCK_EN);

	/* Manually find the High-definition audio, to turn it off */
	ret = dm_pci_bus_find_bdf(PCI_BDF(0, 0x1b, 0), &hda);
	if (ret)
		return -ENOENT;
	dm_pci_clrset_config8(hda, 0x43, 0, 0x6f);

	/* Route I/O buffers to ADSP function */
	dm_pci_clrset_config8(hda, 0x42, 0, 1 << 7 | 1 << 6);
	log_debug("HDA disabled, I/O buffers routed to ADSP\n");

	return 0;
}

static int broadwell_pch_init(struct udevice *dev)
{
	int ret;

	/* Enable upper 128 bytes of CMOS */
	setbits_le32(RCB_REG(RC), 1 << 2);

	/*
	 * TODO: TCO timer halt - this hangs
	 * setio_16(ACPI_BASE_ADDRESS + TCO1_CNT, TCO_TMR_HLT);
	 */

	/* Disable unused device (always) */
	setbits_le32(RCB_REG(FD), PCH_DISABLE_ALWAYS);

	pch_misc_init(dev);

	/* Interrupt configuration */
	pch_enable_ioapic();

	/* Initialize power management */
	ret = pch_power_options(dev);
	if (ret)
		return ret;
	pch_pm_init(dev);
	pch_cg_init(dev);
	ret = pch_serialio_init();
	if (ret)
		return ret;
	systemagent_init();

	return 0;
}

static int broadwell_pch_probe(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(X86_32BIT_INIT)) {
		if (!(gd->flags & GD_FLG_RELOC))
			return broadwell_pch_early_init(dev);
		else
			return broadwell_pch_init(dev);
	} else if (IS_ENABLED(CONFIG_SPL) && !IS_ENABLED(CONFIG_SPL_BUILD)) {
		return broadwell_pch_init(dev);
	} else {
		return 0;
	}
}

static int broadwell_pch_get_spi_base(struct udevice *dev, ulong *sbasep)
{
	u32 rcba;

	dm_pci_read_config32(dev, PCH_RCBA, &rcba);
	/* Bits 31-14 are the base address, 13-1 are reserved, 0 is enable */
	rcba = rcba & 0xffffc000;
	*sbasep = rcba + 0x3800;

	return 0;
}

static int broadwell_set_spi_protect(struct udevice *dev, bool protect)
{
	return lpc_set_spi_protect(dev, BIOS_CTRL, protect);
}

static int broadwell_get_gpio_base(struct udevice *dev, u32 *gbasep)
{
	dm_pci_read_config32(dev, GPIO_BASE, gbasep);
	*gbasep &= PCI_BASE_ADDRESS_IO_MASK;

	return 0;
}

static int broadwell_ioctl(struct udevice *dev, enum pch_req_t req, void *data,
			   int size)
{
	switch (req) {
	case PCH_REQ_PMBASE_INFO: {
		struct pch_pmbase_info *pm = data;
		int ret;

		/* Find the base address of the powermanagement registers */
		ret = dm_pci_read_config16(dev, 0x40, &pm->base);
		if (ret)
			return ret;
		pm->base &= 0xfffe;
		pm->gpio0_en_ofs = GPE0_EN(0);
		pm->pm1_sts_ofs = PM1_STS;
		pm->pm1_cnt_ofs = PM1_CNT;

		return 0;
	}
	default:
		return -ENOSYS;
	}
}

static const struct pch_ops broadwell_pch_ops = {
	.get_spi_base	= broadwell_pch_get_spi_base,
	.set_spi_protect = broadwell_set_spi_protect,
	.get_gpio_base	= broadwell_get_gpio_base,
	.ioctl		= broadwell_ioctl,
};

static const struct udevice_id broadwell_pch_ids[] = {
	{ .compatible = "intel,broadwell-pch" },
	{ }
};

U_BOOT_DRIVER(broadwell_pch) = {
	.name		= "broadwell_pch",
	.id		= UCLASS_PCH,
	.of_match	= broadwell_pch_ids,
	.probe		= broadwell_pch_probe,
	.ops		= &broadwell_pch_ops,
};
