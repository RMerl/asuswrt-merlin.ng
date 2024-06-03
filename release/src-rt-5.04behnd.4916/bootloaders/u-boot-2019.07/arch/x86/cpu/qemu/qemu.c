// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <pci.h>
#include <qfw.h>
#include <asm/irq.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/arch/device.h>
#include <asm/arch/qemu.h>

static bool i440fx;

#ifdef CONFIG_QFW

/* on x86, the qfw registers are all IO ports */
#define FW_CONTROL_PORT	0x510
#define FW_DATA_PORT		0x511
#define FW_DMA_PORT_LOW	0x514
#define FW_DMA_PORT_HIGH	0x518

static void qemu_x86_fwcfg_read_entry_pio(uint16_t entry,
		uint32_t size, void *address)
{
	uint32_t i = 0;
	uint8_t *data = address;

	/*
	 * writting FW_CFG_INVALID will cause read operation to resume at
	 * last offset, otherwise read will start at offset 0
	 *
	 * Note: on platform where the control register is IO port, the
	 * endianness is little endian.
	 */
	if (entry != FW_CFG_INVALID)
		outw(cpu_to_le16(entry), FW_CONTROL_PORT);

	/* the endianness of data register is string-preserving */
	while (size--)
		data[i++] = inb(FW_DATA_PORT);
}

static void qemu_x86_fwcfg_read_entry_dma(struct fw_cfg_dma_access *dma)
{
	/* the DMA address register is big endian */
	outl(cpu_to_be32((uintptr_t)dma), FW_DMA_PORT_HIGH);

	while (be32_to_cpu(dma->control) & ~FW_CFG_DMA_ERROR)
		__asm__ __volatile__ ("pause");
}

static struct fw_cfg_arch_ops fwcfg_x86_ops = {
	.arch_read_pio = qemu_x86_fwcfg_read_entry_pio,
	.arch_read_dma = qemu_x86_fwcfg_read_entry_dma
};
#endif

static void enable_pm_piix(void)
{
	u8 en;
	u16 cmd;

	/* Set the PM I/O base */
	pci_write_config32(PIIX_PM, PMBA, CONFIG_ACPI_PM1_BASE | 1);

	/* Enable access to the PM I/O space */
	pci_read_config16(PIIX_PM, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_IO;
	pci_write_config16(PIIX_PM, PCI_COMMAND, cmd);

	/* PM I/O Space Enable (PMIOSE) */
	pci_read_config8(PIIX_PM, PMREGMISC, &en);
	en |= PMIOSE;
	pci_write_config8(PIIX_PM, PMREGMISC, en);
}

static void enable_pm_ich9(void)
{
	/* Set the PM I/O base */
	pci_write_config32(ICH9_PM, PMBA, CONFIG_ACPI_PM1_BASE | 1);
}

static void qemu_chipset_init(void)
{
	u16 device, xbcs;
	int pam, i;

	/*
	 * i440FX and Q35 chipset have different PAM register offset, but with
	 * the same bitfield layout. Here we determine the offset based on its
	 * PCI device ID.
	 */
	pci_read_config16(PCI_BDF(0, 0, 0), PCI_DEVICE_ID, &device);
	i440fx = (device == PCI_DEVICE_ID_INTEL_82441);
	pam = i440fx ? I440FX_PAM : Q35_PAM;

	/*
	 * Initialize Programmable Attribute Map (PAM) Registers
	 *
	 * Configure legacy segments C/D/E/F to system RAM
	 */
	for (i = 0; i < PAM_NUM; i++)
		pci_write_config8(PCI_BDF(0, 0, 0), pam + i, PAM_RW);

	if (i440fx) {
		/*
		 * Enable legacy IDE I/O ports decode
		 *
		 * Note: QEMU always decode legacy IDE I/O port on PIIX chipset.
		 * However Linux ata_piix driver does sanity check on these two
		 * registers to see whether legacy ports decode is turned on.
		 * This is to make Linux ata_piix driver happy.
		 */
		pci_write_config16(PIIX_IDE, IDE0_TIM, IDE_DECODE_EN);
		pci_write_config16(PIIX_IDE, IDE1_TIM, IDE_DECODE_EN);

		/* Enable I/O APIC */
		pci_read_config16(PIIX_ISA, XBCS, &xbcs);
		xbcs |= APIC_EN;
		pci_write_config16(PIIX_ISA, XBCS, xbcs);

		enable_pm_piix();
	} else {
		/* Configure PCIe ECAM base address */
		pci_write_config32(PCI_BDF(0, 0, 0), PCIEX_BAR,
				   CONFIG_PCIE_ECAM_BASE | BAR_EN);

		enable_pm_ich9();
	}

#ifdef CONFIG_QFW
	qemu_fwcfg_init(&fwcfg_x86_ops);
#endif
}

#if !CONFIG_IS_ENABLED(SPL_X86_32BIT_INIT)
int arch_cpu_init(void)
{
	post_code(POST_CPU_INIT);

	return x86_cpu_init_f();
}

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	post_code(POST_CPU_INFO);
	return default_print_cpuinfo();
}
#endif

int arch_early_init_r(void)
{
	qemu_chipset_init();

	return 0;
}

#ifdef CONFIG_GENERATE_MP_TABLE
int mp_determine_pci_dstirq(int bus, int dev, int func, int pirq)
{
	u8 irq;

	if (i440fx) {
		/*
		 * Not like most x86 platforms, the PIRQ[A-D] on PIIX3 are not
		 * connected to I/O APIC INTPIN#16-19. Instead they are routed
		 * to an irq number controled by the PIRQ routing register.
		 */
		pci_read_config8(PCI_BDF(bus, dev, func),
				 PCI_INTERRUPT_LINE, &irq);
	} else {
		/*
		 * ICH9's PIRQ[A-H] are not consecutive numbers from 0 to 7.
		 * PIRQ[A-D] still maps to [0-3] but PIRQ[E-H] maps to [8-11].
		 */
		irq = pirq < 8 ? pirq + 16 : pirq + 12;
	}

	return irq;
}
#endif
