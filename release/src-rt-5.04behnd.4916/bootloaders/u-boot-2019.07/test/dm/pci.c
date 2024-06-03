// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/* Test that sandbox PCI works correctly */
static int dm_test_pci_base(struct unit_test_state *uts)
{
	struct udevice *bus;

	ut_assertok(uclass_get_device(UCLASS_PCI, 0, &bus));

	return 0;
}
DM_TEST(dm_test_pci_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that sandbox PCI bus numbering and device works correctly */
static int dm_test_pci_busdev(struct unit_test_state *uts)
{
	struct udevice *bus;
	struct udevice *swap;
	u16 vendor, device;

	/* Test bus#0 and its devices */
	ut_assertok(uclass_get_device_by_seq(UCLASS_PCI, 0, &bus));

	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(0, 0x00, 0), &swap));
	vendor = 0;
	ut_assertok(dm_pci_read_config16(swap, PCI_VENDOR_ID, &vendor));
	ut_asserteq(SANDBOX_PCI_VENDOR_ID, vendor);
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(0, 0x1f, 0), &swap));
	device = 0;
	ut_assertok(dm_pci_read_config16(swap, PCI_DEVICE_ID, &device));
	ut_asserteq(SANDBOX_PCI_DEVICE_ID, device);

	/* Test bus#1 and its devices */
	ut_assertok(uclass_get_device_by_seq(UCLASS_PCI, 1, &bus));

	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(1, 0x08, 0), &swap));
	vendor = 0;
	ut_assertok(dm_pci_read_config16(swap, PCI_VENDOR_ID, &vendor));
	ut_asserteq(SANDBOX_PCI_VENDOR_ID, vendor);
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(1, 0x0c, 0), &swap));
	device = 0;
	ut_assertok(dm_pci_read_config16(swap, PCI_DEVICE_ID, &device));
	ut_asserteq(SANDBOX_PCI_DEVICE_ID, device);

	return 0;
}
DM_TEST(dm_test_pci_busdev, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can use the swapcase device correctly */
static int dm_test_pci_swapcase(struct unit_test_state *uts)
{
	struct udevice *swap;
	ulong io_addr, mem_addr;
	char *ptr;

	/* Check that asking for the device 0 automatically fires up PCI */
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(0, 0x00, 0), &swap));

	/* First test I/O */
	io_addr = dm_pci_read_bar32(swap, 0);
	outb(2, io_addr);
	ut_asserteq(2, inb(io_addr));

	/*
	 * Now test memory mapping - note we must unmap and remap to cause
	 * the swapcase emulation to see our data and response.
	 */
	mem_addr = dm_pci_read_bar32(swap, 1);
	ptr = map_sysmem(mem_addr, 20);
	strcpy(ptr, "This is a TesT");
	unmap_sysmem(ptr);

	ptr = map_sysmem(mem_addr, 20);
	ut_asserteq_str("tHIS IS A tESt", ptr);
	unmap_sysmem(ptr);

	/* Check that asking for the device 1 automatically fires up PCI */
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(0, 0x1f, 0), &swap));

	/* First test I/O */
	io_addr = dm_pci_read_bar32(swap, 0);
	outb(2, io_addr);
	ut_asserteq(2, inb(io_addr));

	/*
	 * Now test memory mapping - note we must unmap and remap to cause
	 * the swapcase emulation to see our data and response.
	 */
	mem_addr = dm_pci_read_bar32(swap, 1);
	ptr = map_sysmem(mem_addr, 20);
	strcpy(ptr, "This is a TesT");
	unmap_sysmem(ptr);

	ptr = map_sysmem(mem_addr, 20);
	ut_asserteq_str("tHIS IS A tESt", ptr);
	unmap_sysmem(ptr);

	return 0;
}
DM_TEST(dm_test_pci_swapcase, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can dynamically bind the device driver correctly */
static int dm_test_pci_drvdata(struct unit_test_state *uts)
{
	struct udevice *bus, *swap;

	/* Check that asking for the device automatically fires up PCI */
	ut_assertok(uclass_get_device_by_seq(UCLASS_PCI, 1, &bus));

	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(1, 0x08, 0), &swap));
	ut_asserteq(SWAP_CASE_DRV_DATA, swap->driver_data);
	ut_assertok(dev_of_valid(swap));
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(1, 0x0c, 0), &swap));
	ut_asserteq(SWAP_CASE_DRV_DATA, swap->driver_data);
	ut_assertok(dev_of_valid(swap));
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(1, 0x10, 0), &swap));
	ut_asserteq(SWAP_CASE_DRV_DATA, swap->driver_data);
	ut_assertok(!dev_of_valid(swap));

	return 0;
}
DM_TEST(dm_test_pci_drvdata, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that devices on PCI bus#2 can be accessed correctly */
static int dm_test_pci_mixed(struct unit_test_state *uts)
{
	/* PCI bus#2 has both statically and dynamic declared devices */
	struct udevice *bus, *swap;
	u16 vendor, device;
	ulong io_addr, mem_addr;
	char *ptr;

	ut_assertok(uclass_get_device_by_seq(UCLASS_PCI, 2, &bus));

	/* Test the dynamic device */
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(2, 0x08, 0), &swap));
	vendor = 0;
	ut_assertok(dm_pci_read_config16(swap, PCI_VENDOR_ID, &vendor));
	ut_asserteq(SANDBOX_PCI_VENDOR_ID, vendor);

	/* First test I/O */
	io_addr = dm_pci_read_bar32(swap, 0);
	outb(2, io_addr);
	ut_asserteq(2, inb(io_addr));

	/*
	 * Now test memory mapping - note we must unmap and remap to cause
	 * the swapcase emulation to see our data and response.
	 */
	mem_addr = dm_pci_read_bar32(swap, 1);
	ptr = map_sysmem(mem_addr, 30);
	strcpy(ptr, "This is a TesT oN dYNAMIc");
	unmap_sysmem(ptr);

	ptr = map_sysmem(mem_addr, 30);
	ut_asserteq_str("tHIS IS A tESt On DynamiC", ptr);
	unmap_sysmem(ptr);

	/* Test the static device */
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(2, 0x1f, 0), &swap));
	device = 0;
	ut_assertok(dm_pci_read_config16(swap, PCI_DEVICE_ID, &device));
	ut_asserteq(SANDBOX_PCI_DEVICE_ID, device);

	/* First test I/O */
	io_addr = dm_pci_read_bar32(swap, 0);
	outb(2, io_addr);
	ut_asserteq(2, inb(io_addr));

	/*
	 * Now test memory mapping - note we must unmap and remap to cause
	 * the swapcase emulation to see our data and response.
	 */
	mem_addr = dm_pci_read_bar32(swap, 1);
	ptr = map_sysmem(mem_addr, 30);
	strcpy(ptr, "This is a TesT oN sTATIc");
	unmap_sysmem(ptr);

	ptr = map_sysmem(mem_addr, 30);
	ut_asserteq_str("tHIS IS A tESt On StatiC", ptr);
	unmap_sysmem(ptr);

	return 0;
}
DM_TEST(dm_test_pci_mixed, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test looking up PCI capability and extended capability */
static int dm_test_pci_cap(struct unit_test_state *uts)
{
	struct udevice *bus, *swap;
	int cap;

	ut_assertok(uclass_get_device_by_seq(UCLASS_PCI, 0, &bus));
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(0, 0x1f, 0), &swap));

	/* look up PCI_CAP_ID_EXP */
	cap = dm_pci_find_capability(swap, PCI_CAP_ID_EXP);
	ut_asserteq(PCI_CAP_ID_EXP_OFFSET, cap);

	/* look up PCI_CAP_ID_PCIX */
	cap = dm_pci_find_capability(swap, PCI_CAP_ID_PCIX);
	ut_asserteq(0, cap);

	/* look up PCI_CAP_ID_MSIX starting from PCI_CAP_ID_PM_OFFSET */
	cap = dm_pci_find_next_capability(swap, PCI_CAP_ID_PM_OFFSET,
					  PCI_CAP_ID_MSIX);
	ut_asserteq(PCI_CAP_ID_MSIX_OFFSET, cap);

	/* look up PCI_CAP_ID_VNDR starting from PCI_CAP_ID_EXP_OFFSET */
	cap = dm_pci_find_next_capability(swap, PCI_CAP_ID_EXP_OFFSET,
					  PCI_CAP_ID_VNDR);
	ut_asserteq(0, cap);

	ut_assertok(uclass_get_device_by_seq(UCLASS_PCI, 1, &bus));
	ut_assertok(dm_pci_bus_find_bdf(PCI_BDF(1, 0x08, 0), &swap));

	/* look up PCI_EXT_CAP_ID_DSN */
	cap = dm_pci_find_ext_capability(swap, PCI_EXT_CAP_ID_DSN);
	ut_asserteq(PCI_EXT_CAP_ID_DSN_OFFSET, cap);

	/* look up PCI_EXT_CAP_ID_SRIOV */
	cap = dm_pci_find_ext_capability(swap, PCI_EXT_CAP_ID_SRIOV);
	ut_asserteq(0, cap);

	/* look up PCI_EXT_CAP_ID_DSN starting from PCI_EXT_CAP_ID_ERR_OFFSET */
	cap = dm_pci_find_next_ext_capability(swap, PCI_EXT_CAP_ID_ERR_OFFSET,
					      PCI_EXT_CAP_ID_DSN);
	ut_asserteq(PCI_EXT_CAP_ID_DSN_OFFSET, cap);

	/* look up PCI_EXT_CAP_ID_RCRB starting from PCI_EXT_CAP_ID_VC_OFFSET */
	cap = dm_pci_find_next_ext_capability(swap, PCI_EXT_CAP_ID_VC_OFFSET,
					      PCI_EXT_CAP_ID_RCRB);
	ut_asserteq(0, cap);

	return 0;
}
DM_TEST(dm_test_pci_cap, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
