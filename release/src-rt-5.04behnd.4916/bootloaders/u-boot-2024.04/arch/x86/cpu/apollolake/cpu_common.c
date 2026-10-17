// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/cpu_common.h>
#include <asm/io.h>
#include <asm/msr.h>
#include <asm/pci.h>
#include <asm/arch/cpu.h>
#include <asm/arch/iomap.h>
#include <asm/arch/uart.h>
#include <power/acpi_pmc.h>

/* Define this here to avoid referencing any drivers for the debug UART 1 */
#define PCH_DEV_P2SB	PCI_BDF(0, 0x0d, 0)

void cpu_flush_l1d_to_l2(void)
{
	struct msr_t msr;

	msr = msr_read(MSR_POWER_MISC);
	msr.lo |= FLUSH_DL1_L2;
	msr_write(MSR_POWER_MISC, msr);
}

void enable_pm_timer_emulation(const struct udevice *pmc)
{
	struct acpi_pmc_upriv *upriv = dev_get_uclass_priv(pmc);
	msr_t msr;

	/*
	 * The derived frequency is calculated as follows:
	 *    (CTC_FREQ * msr[63:32]) >> 32 = target frequency.
	 *
	 * Back-solve the multiplier so the 3.579545MHz ACPI timer frequency is
	 * used.
	 */
	msr.hi = (3579545ULL << 32) / CTC_FREQ;

	/* Set PM1 timer IO port and enable */
	msr.lo = EMULATE_PM_TMR_EN | (upriv->acpi_base + R_ACPI_PM1_TMR);
	debug("PM timer %x %x\n", msr.hi, msr.lo);
	msr_write(MSR_EMULATE_PM_TIMER, msr);
}

static void pch_uart_init(void)
{
	/*
	 * Set up the pinmux so that the UART rx/tx signals are connected
	 * outside the SoC.
	 *
	 * There are about 500 lines of code required to program the GPIO
	 * configuration for the UARTs. But it boils down to four writes, and
	 * for the debug UART we want the minimum possible amount of code before
	 * the UART is running. So just add the magic writes here. See
	 * apl_hostbridge_early_init_pinctrl() for the full horror.
	 */
	if (PCI_FUNC(PCH_DEV_UART) == 1) {
		writel(0x40000402, 0xd0c50650);
		writel(0x3c47, 0xd0c50654);
		writel(0x40000400, 0xd0c50658);
		writel(0x3c48, 0xd0c5065c);
	} else { /* UART2 */
		writel(0x40000402, 0xd0c50670);
		writel(0x3c4b, 0xd0c50674);
		writel(0x40000400, 0xd0c50678);
		writel(0x3c4c, 0xd0c5067c);
	}

#ifdef CONFIG_DEBUG_UART
	apl_uart_init(PCH_DEV_UART, CONFIG_VAL(DEBUG_UART_BASE));
#endif
}

static void p2sb_enable_bar(ulong bar)
{
	/* Enable PCR Base address in PCH */
	pci_x86_write_config(PCH_DEV_P2SB, PCI_BASE_ADDRESS_0, bar,
			     PCI_SIZE_32);
	pci_x86_write_config(PCH_DEV_P2SB, PCI_BASE_ADDRESS_1, 0, PCI_SIZE_32);

	/* Enable P2SB MSE */
	pci_x86_write_config(PCH_DEV_P2SB, PCI_COMMAND,
			     PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY,
			     PCI_SIZE_8);
}

/*
 * board_debug_uart_init() - Init the debug UART ready for use
 *
 * This is the minimum init needed to get the UART running. It avoids any
 * drivers or complex code, so that the UART is running as soon as possible.
 */
void board_debug_uart_init(void)
{
	p2sb_enable_bar(IOMAP_P2SB_BAR);
	pch_uart_init();
}
