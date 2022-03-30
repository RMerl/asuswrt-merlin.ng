// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/ioapic.h>
#include <asm/irq.h>
#include <asm/mrccache.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/arch/device.h>
#include <asm/arch/msg_port.h>
#include <asm/arch/quark.h>

static void quark_setup_mtrr(void)
{
	u32 base, mask;
	int i;

	disable_caches();

	/* mark the VGA RAM area as uncacheable */
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_FIX_16K_A0000,
		       MTRR_FIX_TYPE(MTRR_TYPE_UNCACHEABLE));
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_FIX_16K_B0000,
		       MTRR_FIX_TYPE(MTRR_TYPE_UNCACHEABLE));

	/* mark other fixed range areas as cacheable */
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_FIX_64K_00000,
		       MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_FIX_64K_40000,
		       MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_FIX_16K_80000,
		       MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_FIX_16K_90000,
		       MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	for (i = MTRR_FIX_4K_C0000; i <= MTRR_FIX_4K_FC000; i++)
		msg_port_write(MSG_PORT_HOST_BRIDGE, i,
			       MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));

	/* variable range MTRR#0: ROM area */
	mask = ~(CONFIG_SYS_MONITOR_LEN - 1);
	base = CONFIG_SYS_TEXT_BASE & mask;
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_VAR_PHYBASE(MTRR_VAR_ROM),
		       base | MTRR_TYPE_WRBACK);
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_VAR_PHYMASK(MTRR_VAR_ROM),
		       mask | MTRR_PHYS_MASK_VALID);

	/* variable range MTRR#1: eSRAM area */
	mask = ~(ESRAM_SIZE - 1);
	base = CONFIG_ESRAM_BASE & mask;
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_VAR_PHYBASE(MTRR_VAR_ESRAM),
		       base | MTRR_TYPE_WRBACK);
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_VAR_PHYMASK(MTRR_VAR_ESRAM),
		       mask | MTRR_PHYS_MASK_VALID);

	/* enable both variable and fixed range MTRRs */
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_DEF_TYPE,
		       MTRR_DEF_TYPE_EN | MTRR_DEF_TYPE_FIX_EN);

	enable_caches();
}

static void quark_setup_bars(void)
{
	/* GPIO - D31:F0:R44h */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_GBA,
				   CONFIG_GPIO_BASE | IO_BAR_EN);

	/* ACPI PM1 Block - D31:F0:R48h */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_PM1BLK,
				   CONFIG_ACPI_PM1_BASE | IO_BAR_EN);

	/* GPE0 - D31:F0:R4Ch */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_GPE0BLK,
				   CONFIG_ACPI_GPE0_BASE | IO_BAR_EN);

	/* WDT - D31:F0:R84h */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_WDTBA,
				   CONFIG_WDT_BASE | IO_BAR_EN);

	/* RCBA - D31:F0:RF0h */
	qrk_pci_write_config_dword(QUARK_LEGACY_BRIDGE, LB_RCBA,
				   CONFIG_RCBA_BASE | MEM_BAR_EN);

	/* ACPI P Block - Msg Port 04:R70h */
	msg_port_write(MSG_PORT_RMU, PBLK_BA,
		       CONFIG_ACPI_PBLK_BASE | IO_BAR_EN);

	/* SPI DMA - Msg Port 04:R7Ah */
	msg_port_write(MSG_PORT_RMU, SPI_DMA_BA,
		       CONFIG_SPI_DMA_BASE | IO_BAR_EN);

	/* PCIe ECAM */
	msg_port_write(MSG_PORT_MEM_ARBITER, AEC_CTRL,
		       CONFIG_PCIE_ECAM_BASE | MEM_BAR_EN);
	msg_port_write(MSG_PORT_HOST_BRIDGE, HEC_REG,
		       CONFIG_PCIE_ECAM_BASE | MEM_BAR_EN);
}

static void quark_pcie_early_init(void)
{
	/*
	 * Step1: Assert PCIe signal PERST#
	 *
	 * The CPU interface to the PERST# signal is platform dependent.
	 * Call the board-specific codes to perform this task.
	 */
	board_assert_perst();

	/* Step2: PHY common lane reset */
	msg_port_alt_setbits(MSG_PORT_SOC_UNIT, PCIE_CFG, PCIE_PHY_LANE_RST);
	/* wait 1 ms for PHY common lane reset */
	mdelay(1);

	/* Step3: PHY sideband interface reset and controller main reset */
	msg_port_alt_setbits(MSG_PORT_SOC_UNIT, PCIE_CFG,
			     PCIE_PHY_SB_RST | PCIE_CTLR_MAIN_RST);
	/* wait 80ms for PLL to lock */
	mdelay(80);

	/* Step4: Controller sideband interface reset */
	msg_port_alt_setbits(MSG_PORT_SOC_UNIT, PCIE_CFG, PCIE_CTLR_SB_RST);
	/* wait 20ms for controller sideband interface reset */
	mdelay(20);

	/* Step5: De-assert PERST# */
	board_deassert_perst();

	/* Step6: Controller primary interface reset */
	msg_port_alt_setbits(MSG_PORT_SOC_UNIT, PCIE_CFG, PCIE_CTLR_PRI_RST);

	/* Mixer Load Lane 0 */
	msg_port_io_clrbits(MSG_PORT_PCIE_AFE, PCIE_RXPICTRL0_L0,
			    (1 << 6) | (1 << 7));

	/* Mixer Load Lane 1 */
	msg_port_io_clrbits(MSG_PORT_PCIE_AFE, PCIE_RXPICTRL0_L1,
			    (1 << 6) | (1 << 7));
}

static void quark_usb_early_init(void)
{
	/* The sequence below comes from Quark firmware writer guide */

	msg_port_alt_clrsetbits(MSG_PORT_USB_AFE, USB2_GLOBAL_PORT,
				1 << 1, (1 << 6) | (1 << 7));

	msg_port_alt_clrsetbits(MSG_PORT_USB_AFE, USB2_COMPBG,
				(1 << 8) | (1 << 9), (1 << 7) | (1 << 10));

	msg_port_alt_setbits(MSG_PORT_USB_AFE, USB2_PLL2, 1 << 29);

	msg_port_alt_setbits(MSG_PORT_USB_AFE, USB2_PLL1, 1 << 1);

	msg_port_alt_clrsetbits(MSG_PORT_USB_AFE, USB2_PLL1,
				(1 << 3) | (1 << 4) | (1 << 5), 1 << 6);

	msg_port_alt_clrbits(MSG_PORT_USB_AFE, USB2_PLL2, 1 << 29);

	msg_port_alt_setbits(MSG_PORT_USB_AFE, USB2_PLL2, 1 << 24);
}

static void quark_thermal_early_init(void)
{
	/* The sequence below comes from Quark firmware writer guide */

	/* thermal sensor mode config */
	msg_port_alt_clrsetbits(MSG_PORT_SOC_UNIT, TS_CFG1,
				(1 << 3) | (1 << 4) | (1 << 5), 1 << 5);
	msg_port_alt_clrsetbits(MSG_PORT_SOC_UNIT, TS_CFG1,
				(1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) |
				(1 << 12), 1 << 9);
	msg_port_alt_setbits(MSG_PORT_SOC_UNIT, TS_CFG1, 1 << 14);
	msg_port_alt_clrbits(MSG_PORT_SOC_UNIT, TS_CFG1, 1 << 17);
	msg_port_alt_clrbits(MSG_PORT_SOC_UNIT, TS_CFG1, 1 << 18);
	msg_port_alt_clrsetbits(MSG_PORT_SOC_UNIT, TS_CFG2, 0xffff, 0x011f);
	msg_port_alt_clrsetbits(MSG_PORT_SOC_UNIT, TS_CFG3, 0xff, 0x17);
	msg_port_alt_clrsetbits(MSG_PORT_SOC_UNIT, TS_CFG3,
				(1 << 8) | (1 << 9), 1 << 8);
	msg_port_alt_clrbits(MSG_PORT_SOC_UNIT, TS_CFG3, 0xff000000);
	msg_port_alt_clrsetbits(MSG_PORT_SOC_UNIT, TS_CFG4,
				0x7ff800, 0xc8 << 11);

	/* thermal monitor catastrophic trip set point (105 celsius) */
	msg_port_clrsetbits(MSG_PORT_RMU, TS_TRIP, 0xff, 155);

	/* thermal monitor catastrophic trip clear point (0 celsius) */
	msg_port_clrsetbits(MSG_PORT_RMU, TS_TRIP, 0xff0000, 50 << 16);

	/* take thermal sensor out of reset */
	msg_port_alt_clrbits(MSG_PORT_SOC_UNIT, TS_CFG4, 1 << 0);

	/* enable thermal monitor */
	msg_port_setbits(MSG_PORT_RMU, TS_MODE, 1 << 15);

	/* lock all thermal configuration */
	msg_port_setbits(MSG_PORT_RMU, RMU_CTRL, (1 << 5) | (1 << 6));
}

static void quark_enable_legacy_seg(void)
{
	msg_port_setbits(MSG_PORT_HOST_BRIDGE, HMISC2,
			 HMISC2_SEGE | HMISC2_SEGF | HMISC2_SEGAB);
}

int arch_cpu_init(void)
{
	int ret;

	post_code(POST_CPU_INIT);

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	/*
	 * Quark SoC does not support MSR MTRRs. Fixed and variable range MTRRs
	 * are accessed indirectly via the message port and not the traditional
	 * MSR mechanism. Only UC, WT and WB cache types are supported.
	 */
	quark_setup_mtrr();

	/*
	 * Quark SoC has some non-standard BARs (excluding PCI standard BARs)
	 * which need be initialized with suggested values
	 */
	quark_setup_bars();

	/* Initialize USB2 PHY */
	quark_usb_early_init();

	/* Initialize thermal sensor */
	quark_thermal_early_init();

	/* Turn on legacy segments (A/B/E/F) decode to system RAM */
	quark_enable_legacy_seg();

	return 0;
}

int arch_cpu_init_dm(void)
{
	/*
	 * Initialize PCIe controller
	 *
	 * Quark SoC holds the PCIe controller in reset following a power on.
	 * U-Boot needs to release the PCIe controller from reset. The PCIe
	 * controller (D23:F0/F1) will not be visible in PCI configuration
	 * space and any access to its PCI configuration registers will cause
	 * system hang while it is held in reset.
	 */
	quark_pcie_early_init();

	return 0;
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

static void quark_pcie_init(void)
{
	u32 val;

	/* PCIe upstream non-posted & posted request size */
	qrk_pci_write_config_dword(QUARK_PCIE0, PCIE_RP_CCFG,
				   CCFG_UPRS | CCFG_UNRS);
	qrk_pci_write_config_dword(QUARK_PCIE1, PCIE_RP_CCFG,
				   CCFG_UPRS | CCFG_UNRS);

	/* PCIe packet fast transmit mode (IPF) */
	qrk_pci_write_config_dword(QUARK_PCIE0, PCIE_RP_MPC2, MPC2_IPF);
	qrk_pci_write_config_dword(QUARK_PCIE1, PCIE_RP_MPC2, MPC2_IPF);

	/* PCIe message bus idle counter (SBIC) */
	qrk_pci_read_config_dword(QUARK_PCIE0, PCIE_RP_MBC, &val);
	val |= MBC_SBIC;
	qrk_pci_write_config_dword(QUARK_PCIE0, PCIE_RP_MBC, val);
	qrk_pci_read_config_dword(QUARK_PCIE1, PCIE_RP_MBC, &val);
	val |= MBC_SBIC;
	qrk_pci_write_config_dword(QUARK_PCIE1, PCIE_RP_MBC, val);
}

static void quark_usb_init(void)
{
	u32 bar;

	/* Change USB EHCI packet buffer OUT/IN threshold */
	qrk_pci_read_config_dword(QUARK_USB_EHCI, PCI_BASE_ADDRESS_0, &bar);
	writel((0x7f << 16) | 0x7f, bar + EHCI_INSNREG01);

	/* Disable USB device interrupts */
	qrk_pci_read_config_dword(QUARK_USB_DEVICE, PCI_BASE_ADDRESS_0, &bar);
	writel(0x7f, bar + USBD_INT_MASK);
	writel((0xf << 16) | 0xf, bar + USBD_EP_INT_MASK);
	writel((0xf << 16) | 0xf, bar + USBD_EP_INT_STS);
}

static void quark_irq_init(void)
{
	struct quark_rcba *rcba;
	u32 base;

	qrk_pci_read_config_dword(QUARK_LEGACY_BRIDGE, LB_RCBA, &base);
	base &= ~MEM_BAR_EN;
	rcba = (struct quark_rcba *)base;

	/*
	 * Route Quark PCI device interrupt pin to PIRQ
	 *
	 * Route device#23's INTA/B/C/D to PIRQA/B/C/D
	 * Route device#20,21's INTA/B/C/D to PIRQE/F/G/H
	 */
	writew(PIRQC, &rcba->rmu_ir);
	writew(PIRQA | (PIRQB << 4) | (PIRQC << 8) | (PIRQD << 12),
	       &rcba->d23_ir);
	writew(PIRQD, &rcba->core_ir);
	writew(PIRQE | (PIRQF << 4) | (PIRQG << 8) | (PIRQH << 12),
	       &rcba->d20d21_ir);
}

int arch_early_init_r(void)
{
	quark_pcie_init();

	quark_usb_init();

	quark_irq_init();

	return 0;
}

int arch_misc_init(void)
{
#ifdef CONFIG_ENABLE_MRC_CACHE
	/*
	 * We intend not to check any return value here, as even MRC cache
	 * is not saved successfully, it is not a severe error that will
	 * prevent system from continuing to boot.
	 */
	mrccache_save();
#endif

	/* Assign a unique I/O APIC ID */
	io_apic_set_id(1);

	return 0;
}

void board_final_cleanup(void)
{
	struct quark_rcba *rcba;
	u32 base, val;

	qrk_pci_read_config_dword(QUARK_LEGACY_BRIDGE, LB_RCBA, &base);
	base &= ~MEM_BAR_EN;
	rcba = (struct quark_rcba *)base;

	/* Initialize 'Component ID' to zero */
	val = readl(&rcba->esd);
	val &= ~0xff0000;
	writel(val, &rcba->esd);

	/* Lock HMBOUND for security */
	msg_port_setbits(MSG_PORT_HOST_BRIDGE, HM_BOUND, HM_BOUND_LOCK);

	return;
}
