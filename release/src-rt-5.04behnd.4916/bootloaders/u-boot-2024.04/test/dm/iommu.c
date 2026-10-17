// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <iommu.h>
#include <malloc.h>
#include <test/test.h>
#include <test/ut.h>
#include <asm/io.h>

static int dm_test_iommu(struct unit_test_state *uts)
{
	struct udevice *dev;
	dma_addr_t dva;
	void *addr;

	ut_assertok(uclass_find_device(UCLASS_IOMMU, 0, &dev));
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));

	/* Probing USB probes the IOMMU through the "iommus" property */
	ut_assertok(uclass_probe_all(UCLASS_USB));
	ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);

	addr = malloc(256);
	ut_assert(addr);

	ut_assertok(uclass_find_device(UCLASS_USB, 0, &dev));
	dva = dev_iommu_dma_map(dev, addr, 256);
	ut_assert(dva >= 0x89abc000 && dva < 0x89ac00000);

	dev_iommu_dma_unmap(dev, dva, 256);

	free(addr);

	return 0;
}
DM_TEST(dm_test_iommu, UT_TESTF_SCAN_FDT);

static int dm_test_iommu_noiommu(struct unit_test_state *uts)
{
	struct udevice *dev;
	dma_addr_t dva;
	void *addr;

	ut_assertok(uclass_find_device(UCLASS_IOMMU, 0, &dev));
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));

	/* Probing ethernet should not probe the IOMMU */
	ut_assertok(uclass_probe_all(UCLASS_ETH));
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));

	addr = malloc(256);
	ut_assert(addr);

	ut_assertok(uclass_find_device(UCLASS_ETH, 0, &dev));
	dva = dev_iommu_dma_map(dev, addr, 256);
	ut_assert(dva == virt_to_phys(addr));

	dev_iommu_dma_unmap(dev, dva, 256);

	free(addr);

	return 0;
}
DM_TEST(dm_test_iommu_noiommu, UT_TESTF_SCAN_FDT);

static int dm_test_iommu_pci(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_find_device(UCLASS_IOMMU, 0, &dev));
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));

	/* Probing P2SB probes the IOMMU through the "iommu-map" property */
	ut_assertok(uclass_probe_all(UCLASS_P2SB));
	ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);

	return 0;
}
DM_TEST(dm_test_iommu_pci, UT_TESTF_SCAN_FDT);

static int dm_test_iommu_pci_noiommu(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_find_device(UCLASS_IOMMU, 0, &dev));
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));

	/* Probing PMC should not probe the IOMMU */
	ut_assertok(uclass_probe_all(UCLASS_ACPI_PMC));
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));

	return 0;
}
DM_TEST(dm_test_iommu_pci_noiommu, UT_TESTF_SCAN_FDT);
