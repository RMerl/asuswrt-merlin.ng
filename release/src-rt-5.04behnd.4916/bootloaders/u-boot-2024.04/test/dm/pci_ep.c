// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Ramon Fried
 */

#include <common.h>
#include <dm.h>
#include <hexdump.h>
#include <pci_ep.h>
#include <asm/io.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that sandbox PCI EP works correctly */
static int dm_test_pci_ep_base(struct unit_test_state *uts)
{
	struct udevice *bus;
	struct pci_bar tmp_bar;
	struct pci_ep_header tmp_header;
	int i;

	struct pci_ep_header ep_header = {
		.vendorid = 0x1234,
		.deviceid = 0x2020,
		.revid = 1,
		.interrupt_pin = PCI_INTERRUPT_INTA,
	};

	struct pci_bar bar = {
		.phys_addr = 0x80000000,
		.size = 0x100000,
		.barno = BAR_0,
		.flags = PCI_BASE_ADDRESS_MEM_TYPE_64 |
			PCI_BASE_ADDRESS_MEM_PREFETCH,
	};

	ut_assertok(uclass_get_device(UCLASS_PCI_EP, 0, &bus));
	ut_assertnonnull(bus);

	ut_assertok(pci_ep_write_header(bus, 0, &ep_header));
	ut_assertok(pci_ep_read_header(bus, 0, &tmp_header));
	ut_asserteq_mem(&tmp_header, &ep_header, sizeof(ep_header));

	ut_assertok(pci_ep_set_msi(bus, 0, 4));
	ut_asserteq(pci_ep_get_msi(bus, 0), 4);

	ut_assertok(pci_ep_set_msix(bus, 0, 360));
	ut_asserteq(pci_ep_get_msix(bus, 0), 360);

	ut_assertok(pci_ep_set_bar(bus, 0, &bar));

	ut_assertok(pci_ep_read_bar(bus, 0, &tmp_bar, BAR_0));
	ut_asserteq_mem(&tmp_bar, &bar, sizeof(bar));

	for (i = 0; i < 10; i++)
		ut_assertok(pci_ep_raise_irq(bus, 0, 1, PCI_EP_IRQ_LEGACY));

	ut_asserteq(sandbox_get_pci_ep_irq_count(bus), 10);
	return 0;
}

DM_TEST(dm_test_pci_ep_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
