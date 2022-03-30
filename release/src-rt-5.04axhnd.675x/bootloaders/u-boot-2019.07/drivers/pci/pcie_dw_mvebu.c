// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Marvell International Ltd.
 *
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * Based on:
 *   - drivers/pci/pcie_imx.c
 *   - drivers/pci/pci_mvebu.c
 *   - drivers/pci/pcie_xilinx.c
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/* PCI Config space registers */
#define PCIE_CONFIG_BAR0		0x10
#define PCIE_LINK_STATUS_REG		0x80
#define PCIE_LINK_STATUS_SPEED_OFF	16
#define PCIE_LINK_STATUS_SPEED_MASK	(0xf << PCIE_LINK_STATUS_SPEED_OFF)
#define PCIE_LINK_STATUS_WIDTH_OFF	20
#define PCIE_LINK_STATUS_WIDTH_MASK	(0xf << PCIE_LINK_STATUS_WIDTH_OFF)

/* Resizable bar capability registers */
#define RESIZABLE_BAR_CAP		0x250
#define RESIZABLE_BAR_CTL0		0x254
#define RESIZABLE_BAR_CTL1		0x258

/* iATU registers */
#define PCIE_ATU_VIEWPORT		0x900
#define PCIE_ATU_REGION_INBOUND		(0x1 << 31)
#define PCIE_ATU_REGION_OUTBOUND	(0x0 << 31)
#define PCIE_ATU_REGION_INDEX1		(0x1 << 0)
#define PCIE_ATU_REGION_INDEX0		(0x0 << 0)
#define PCIE_ATU_CR1			0x904
#define PCIE_ATU_TYPE_MEM		(0x0 << 0)
#define PCIE_ATU_TYPE_IO		(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		(0x5 << 0)
#define PCIE_ATU_CR2			0x908
#define PCIE_ATU_ENABLE			(0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE	(0x1 << 30)
#define PCIE_ATU_LOWER_BASE		0x90C
#define PCIE_ATU_UPPER_BASE		0x910
#define PCIE_ATU_LIMIT			0x914
#define PCIE_ATU_LOWER_TARGET		0x918
#define PCIE_ATU_BUS(x)			(((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			(((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		(((x) & 0x7) << 16)
#define PCIE_ATU_UPPER_TARGET		0x91C

#define PCIE_LINK_CAPABILITY		0x7C
#define PCIE_LINK_CTL_2			0xA0
#define TARGET_LINK_SPEED_MASK		0xF
#define LINK_SPEED_GEN_1		0x1
#define LINK_SPEED_GEN_2		0x2
#define LINK_SPEED_GEN_3		0x3

#define PCIE_GEN3_RELATED		0x890
#define GEN3_EQU_DISABLE		(1 << 16)
#define GEN3_ZRXDC_NON_COMP		(1 << 0)

#define PCIE_GEN3_EQU_CTRL		0x8A8
#define GEN3_EQU_EVAL_2MS_DISABLE	(1 << 5)

#define PCIE_ROOT_COMPLEX_MODE_MASK	(0xF << 4)

#define PCIE_LINK_UP_TIMEOUT_MS		100

#define PCIE_GLOBAL_CONTROL		0x8000
#define PCIE_APP_LTSSM_EN		(1 << 2)
#define PCIE_DEVICE_TYPE_OFFSET		(4)
#define PCIE_DEVICE_TYPE_MASK		(0xF)
#define PCIE_DEVICE_TYPE_EP		(0x0) /* Endpoint */
#define PCIE_DEVICE_TYPE_LEP		(0x1) /* Legacy endpoint */
#define PCIE_DEVICE_TYPE_RC		(0x4) /* Root complex */

#define PCIE_GLOBAL_STATUS		0x8008
#define PCIE_GLB_STS_RDLH_LINK_UP	(1 << 1)
#define PCIE_GLB_STS_PHY_LINK_UP	(1 << 9)

#define PCIE_ARCACHE_TRC		0x8050
#define PCIE_AWCACHE_TRC		0x8054
#define ARCACHE_SHAREABLE_CACHEABLE	0x3511
#define AWCACHE_SHAREABLE_CACHEABLE	0x5311

#define LINK_SPEED_GEN_1                0x1
#define LINK_SPEED_GEN_2                0x2
#define LINK_SPEED_GEN_3                0x3

/**
 * struct pcie_dw_mvebu - MVEBU DW PCIe controller state
 *
 * @ctrl_base: The base address of the register space
 * @cfg_base: The base address of the configuration space
 * @cfg_size: The size of the configuration space which is needed
 *            as it gets written into the PCIE_ATU_LIMIT register
 * @first_busno: This driver supports multiple PCIe controllers.
 *               first_busno stores the bus number of the PCIe root-port
 *               number which may vary depending on the PCIe setup
 *               (PEX switches etc).
 */
struct pcie_dw_mvebu {
	void *ctrl_base;
	void *cfg_base;
	fdt_size_t cfg_size;
	int first_busno;

	/* IO and MEM PCI regions */
	struct pci_region io;
	struct pci_region mem;
};

static int pcie_dw_get_link_speed(const void *regs_base)
{
	return (readl(regs_base + PCIE_LINK_STATUS_REG) &
		PCIE_LINK_STATUS_SPEED_MASK) >> PCIE_LINK_STATUS_SPEED_OFF;
}

static int pcie_dw_get_link_width(const void *regs_base)
{
	return (readl(regs_base + PCIE_LINK_STATUS_REG) &
		PCIE_LINK_STATUS_WIDTH_MASK) >> PCIE_LINK_STATUS_WIDTH_OFF;
}

/**
 * pcie_dw_prog_outbound_atu() - Configure ATU for outbound accesses
 *
 * @pcie: Pointer to the PCI controller state
 * @index: ATU region index
 * @type: ATU accsess type
 * @cpu_addr: the physical address for the translation entry
 * @pci_addr: the pcie bus address for the translation entry
 * @size: the size of the translation entry
 */
static void pcie_dw_prog_outbound_atu(struct pcie_dw_mvebu *pcie, int index,
				      int type, u64 cpu_addr, u64 pci_addr,
				      u32 size)
{
	writel(PCIE_ATU_REGION_OUTBOUND | index,
	       pcie->ctrl_base + PCIE_ATU_VIEWPORT);
	writel(lower_32_bits(cpu_addr), pcie->ctrl_base + PCIE_ATU_LOWER_BASE);
	writel(upper_32_bits(cpu_addr), pcie->ctrl_base + PCIE_ATU_UPPER_BASE);
	writel(lower_32_bits(cpu_addr + size - 1),
	       pcie->ctrl_base + PCIE_ATU_LIMIT);
	writel(lower_32_bits(pci_addr),
	       pcie->ctrl_base + PCIE_ATU_LOWER_TARGET);
	writel(upper_32_bits(pci_addr),
	       pcie->ctrl_base + PCIE_ATU_UPPER_TARGET);
	writel(type, pcie->ctrl_base + PCIE_ATU_CR1);
	writel(PCIE_ATU_ENABLE, pcie->ctrl_base + PCIE_ATU_CR2);
}

/**
 * set_cfg_address() - Configure the PCIe controller config space access
 *
 * @pcie: Pointer to the PCI controller state
 * @d: PCI device to access
 * @where: Offset in the configuration space
 *
 * Configures the PCIe controller to access the configuration space of
 * a specific PCIe device and returns the address to use for this
 * access.
 *
 * Return: Address that can be used to access the configation space
 *         of the requested device / offset
 */
static uintptr_t set_cfg_address(struct pcie_dw_mvebu *pcie,
				 pci_dev_t d, uint where)
{
	uintptr_t va_address;
	u32 atu_type;

	/*
	 * Region #0 is used for Outbound CFG space access.
	 * Direction = Outbound
	 * Region Index = 0
	 */

	if (PCI_BUS(d) == (pcie->first_busno + 1))
		/* For local bus, change TLP Type field to 4. */
		atu_type = PCIE_ATU_TYPE_CFG0;
	else
		/* Otherwise, change TLP Type field to 5. */
		atu_type = PCIE_ATU_TYPE_CFG1;

	if (PCI_BUS(d) == pcie->first_busno) {
		/* Accessing root port configuration space. */
		va_address = (uintptr_t)pcie->ctrl_base;
	} else {
		d = PCI_MASK_BUS(d) | (PCI_BUS(d) - pcie->first_busno);
		pcie_dw_prog_outbound_atu(pcie, PCIE_ATU_REGION_INDEX0,
					  atu_type, (u64)pcie->cfg_base,
					  d << 8, pcie->cfg_size);
		va_address = (uintptr_t)pcie->cfg_base;
	}

	va_address += where & ~0x3;

	return va_address;
}

/**
 * pcie_dw_addr_valid() - Check for valid bus address
 *
 * @d: The PCI device to access
 * @first_busno: Bus number of the PCIe controller root complex
 *
 * Return 1 (true) if the PCI device can be accessed by this controller.
 *
 * Return: 1 on valid, 0 on invalid
 */
static int pcie_dw_addr_valid(pci_dev_t d, int first_busno)
{
	if ((PCI_BUS(d) == first_busno) && (PCI_DEV(d) > 0))
		return 0;
	if ((PCI_BUS(d) == first_busno + 1) && (PCI_DEV(d) > 0))
		return 0;

	return 1;
}

/**
 * pcie_dw_mvebu_read_config() - Read from configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 *
 * Return: 0 on success
 */
static int pcie_dw_mvebu_read_config(struct udevice *bus, pci_dev_t bdf,
				     uint offset, ulong *valuep,
				     enum pci_size_t size)
{
	struct pcie_dw_mvebu *pcie = dev_get_priv(bus);
	uintptr_t va_address;
	ulong value;

	debug("PCIE CFG read:  (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!pcie_dw_addr_valid(bdf, pcie->first_busno)) {
		debug("- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	va_address = set_cfg_address(pcie, bdf, offset);

	value = readl(va_address);

	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);
	*valuep = pci_conv_32_to_size(value, offset, size);

	pcie_dw_prog_outbound_atu(pcie, PCIE_ATU_REGION_INDEX0,
				  PCIE_ATU_TYPE_IO, pcie->io.phys_start,
				  pcie->io.bus_start, pcie->io.size);

	return 0;
}

/**
 * pcie_dw_mvebu_write_config() - Write to configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 *
 * Return: 0 on success
 */
static int pcie_dw_mvebu_write_config(struct udevice *bus, pci_dev_t bdf,
				      uint offset, ulong value,
				      enum pci_size_t size)
{
	struct pcie_dw_mvebu *pcie = dev_get_priv(bus);
	uintptr_t va_address;
	ulong old;

	debug("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);

	if (!pcie_dw_addr_valid(bdf, pcie->first_busno)) {
		debug("- out of range\n");
		return 0;
	}

	va_address = set_cfg_address(pcie, bdf, offset);

	old = readl(va_address);
	value = pci_conv_size_to_32(old, value, offset, size);
	writel(value, va_address);

	pcie_dw_prog_outbound_atu(pcie, PCIE_ATU_REGION_INDEX0,
				  PCIE_ATU_TYPE_IO, pcie->io.phys_start,
				  pcie->io.bus_start, pcie->io.size);

	return 0;
}

/**
 * pcie_dw_configure() - Configure link capabilities and speed
 *
 * @regs_base: A pointer to the PCIe controller registers
 * @cap_speed: The capabilities and speed to configure
 *
 * Configure the link capabilities and speed in the PCIe root complex.
 */
static void pcie_dw_configure(const void *regs_base, u32 cap_speed)
{
	/*
	 * TODO (shadi@marvell.com, sr@denx.de):
	 * Need to read the serdes speed from the dts and according to it
	 * configure the PCIe gen
	 */

	/* Set link to GEN 3 */
	clrsetbits_le32(regs_base + PCIE_LINK_CTL_2,
			TARGET_LINK_SPEED_MASK, cap_speed);
	clrsetbits_le32(regs_base + PCIE_LINK_CAPABILITY,
			TARGET_LINK_SPEED_MASK, cap_speed);
	setbits_le32(regs_base + PCIE_GEN3_EQU_CTRL, GEN3_EQU_EVAL_2MS_DISABLE);
}

/**
 * is_link_up() - Return the link state
 *
 * @regs_base: A pointer to the PCIe controller registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link
 */
static int is_link_up(const void *regs_base)
{
	u32 mask = PCIE_GLB_STS_RDLH_LINK_UP | PCIE_GLB_STS_PHY_LINK_UP;
	u32 reg;

	reg = readl(regs_base + PCIE_GLOBAL_STATUS);
	if ((reg & mask) == mask)
		return 1;

	return 0;
}

/**
 * wait_link_up() - Wait for the link to come up
 *
 * @regs_base: A pointer to the PCIe controller registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link (timeout)
 */
static int wait_link_up(const void *regs_base)
{
	unsigned long timeout;

	timeout = get_timer(0) + PCIE_LINK_UP_TIMEOUT_MS;
	while (!is_link_up(regs_base)) {
		if (get_timer(0) > timeout)
			return 0;
	};

	return 1;
}

/**
 * pcie_dw_mvebu_pcie_link_up() - Configure the PCIe root port
 *
 * @regs_base: A pointer to the PCIe controller registers
 * @cap_speed: The capabilities and speed to configure
 *
 * Configure the PCIe controller root complex depending on the
 * requested link capabilities and speed.
 *
 * Return: 1 (true) for active line and 0 (false) for no link
 */
static int pcie_dw_mvebu_pcie_link_up(const void *regs_base, u32 cap_speed)
{
	if (!is_link_up(regs_base)) {
		/* Disable LTSSM state machine to enable configuration */
		clrbits_le32(regs_base + PCIE_GLOBAL_CONTROL,
			     PCIE_APP_LTSSM_EN);
	}

	clrsetbits_le32(regs_base + PCIE_GLOBAL_CONTROL,
			PCIE_DEVICE_TYPE_MASK << PCIE_DEVICE_TYPE_OFFSET,
			PCIE_DEVICE_TYPE_RC << PCIE_DEVICE_TYPE_OFFSET);

	/* Set the PCIe master AXI attributes */
	writel(ARCACHE_SHAREABLE_CACHEABLE, regs_base + PCIE_ARCACHE_TRC);
	writel(AWCACHE_SHAREABLE_CACHEABLE, regs_base + PCIE_AWCACHE_TRC);

	/* DW pre link configurations */
	pcie_dw_configure(regs_base, cap_speed);

	if (!is_link_up(regs_base)) {
		/* Configuration done. Start LTSSM */
		setbits_le32(regs_base + PCIE_GLOBAL_CONTROL,
			     PCIE_APP_LTSSM_EN);
	}

	/* Check that link was established */
	if (!wait_link_up(regs_base))
		return 0;

	/*
	 * Link can be established in Gen 1. still need to wait
	 * till MAC nagaotiation is completed
	 */
	udelay(100);

	return 1;
}

/**
 * pcie_dw_set_host_bars() - Configure the host BARs
 *
 * @regs_base: A pointer to the PCIe controller registers
 *
 * Configure the host BARs of the PCIe controller root port so that
 * PCI(e) devices may access the system memory.
 */
static void pcie_dw_set_host_bars(const void *regs_base)
{
	u32 size = gd->ram_size;
	u64 max_size;
	u32 reg;
	u32 bar0;

	/* Verify the maximal BAR size */
	reg = readl(regs_base + RESIZABLE_BAR_CAP);
	max_size = 1ULL << (5 + (reg + (1 << 4)));

	if (size > max_size) {
		size = max_size;
		printf("Warning: PCIe BARs can't map all DRAM space\n");
	}

	/* Set the BAR base and size towards DDR */
	bar0 = CONFIG_SYS_SDRAM_BASE & ~0xf;
	bar0 |= PCI_BASE_ADDRESS_MEM_TYPE_32;
	writel(CONFIG_SYS_SDRAM_BASE, regs_base + PCIE_CONFIG_BAR0);

	reg = ((size >> 20) - 1) << 12;
	writel(size, regs_base + RESIZABLE_BAR_CTL0);
}

/**
 * pcie_dw_mvebu_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_dw_mvebu_probe(struct udevice *dev)
{
	struct pcie_dw_mvebu *pcie = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
#ifdef CONFIG_DM_GPIO
	struct gpio_desc reset_gpio;

	gpio_request_by_name(dev, "marvell,reset-gpio", 0, &reset_gpio,
			     GPIOD_IS_OUT);
	/*
	 * Issue reset to add-in card trough the dedicated GPIO.
	 * Some boards are connecting the card reset pin to common system
	 * reset wire and others are using separate GPIO port.
	 * In the last case we have to release a reset of the addon card
	 * using this GPIO.
	 */
	if (dm_gpio_is_valid(&reset_gpio)) {
		dm_gpio_set_value(&reset_gpio, 1); /* assert */
		mdelay(200);
		dm_gpio_set_value(&reset_gpio, 0); /* de-assert */
		mdelay(200);
	}
#else
	debug("PCIE Reset on GPIO support is missing\n");
#endif /* CONFIG_DM_GPIO */

	pcie->first_busno = dev->seq;

	/* Don't register host if link is down */
	if (!pcie_dw_mvebu_pcie_link_up(pcie->ctrl_base, LINK_SPEED_GEN_3)) {
		printf("PCIE-%d: Link down\n", dev->seq);
	} else {
		printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n", dev->seq,
		       pcie_dw_get_link_speed(pcie->ctrl_base),
		       pcie_dw_get_link_width(pcie->ctrl_base),
		       hose->first_busno);
	}

	/* Store the IO and MEM windows settings for future use by the ATU */
	pcie->io.phys_start = hose->regions[0].phys_start; /* IO base */
	pcie->io.bus_start  = hose->regions[0].bus_start;  /* IO_bus_addr */
	pcie->io.size	    = hose->regions[0].size;	   /* IO size */

	pcie->mem.phys_start = hose->regions[1].phys_start; /* MEM base */
	pcie->mem.bus_start  = hose->regions[1].bus_start;  /* MEM_bus_addr */
	pcie->mem.size	     = hose->regions[1].size;	    /* MEM size */

	pcie_dw_prog_outbound_atu(pcie, PCIE_ATU_REGION_INDEX1,
				  PCIE_ATU_TYPE_MEM, pcie->mem.phys_start,
				  pcie->mem.bus_start, pcie->mem.size);

	/* Set the CLASS_REV of RC CFG header to PCI_CLASS_BRIDGE_PCI */
	clrsetbits_le32(pcie->ctrl_base + PCI_CLASS_REVISION,
			0xffff << 16, PCI_CLASS_BRIDGE_PCI << 16);

	pcie_dw_set_host_bars(pcie->ctrl_base);

	return 0;
}

/**
 * pcie_dw_mvebu_ofdata_to_platdata() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_dw_mvebu_ofdata_to_platdata(struct udevice *dev)
{
	struct pcie_dw_mvebu *pcie = dev_get_priv(dev);

	/* Get the controller base address */
	pcie->ctrl_base = (void *)devfdt_get_addr_index(dev, 0);
	if ((fdt_addr_t)pcie->ctrl_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the config space base address and size */
	pcie->cfg_base = (void *)devfdt_get_addr_size_index(dev, 1,
							 &pcie->cfg_size);
	if ((fdt_addr_t)pcie->cfg_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops pcie_dw_mvebu_ops = {
	.read_config	= pcie_dw_mvebu_read_config,
	.write_config	= pcie_dw_mvebu_write_config,
};

static const struct udevice_id pcie_dw_mvebu_ids[] = {
	{ .compatible = "marvell,armada8k-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_dw_mvebu) = {
	.name			= "pcie_dw_mvebu",
	.id			= UCLASS_PCI,
	.of_match		= pcie_dw_mvebu_ids,
	.ops			= &pcie_dw_mvebu_ops,
	.ofdata_to_platdata	= pcie_dw_mvebu_ofdata_to_platdata,
	.probe			= pcie_dw_mvebu_probe,
	.priv_auto_alloc_size	= sizeof(struct pcie_dw_mvebu),
};
