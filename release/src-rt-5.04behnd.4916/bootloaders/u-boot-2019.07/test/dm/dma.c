// SPDX-License-Identifier: GPL-2.0+
/*
 * Direct Memory Access U-Class tests
 *
 * Copyright (C) 2018 Texas Instruments Incorporated <www.ti.com>
 * Grygorii Strashko <grygorii.strashko@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <dma.h>
#include <test/ut.h>

static int dm_test_dma_m2m(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct dma dma_m2m;
	u8 src_buf[512];
	u8 dst_buf[512];
	size_t len = 512;
	int i;

	ut_assertok(uclass_get_device_by_name(UCLASS_DMA, "dma", &dev));
	ut_assertok(dma_get_by_name(dev, "m2m", &dma_m2m));

	memset(dst_buf, 0, len);
	for (i = 0; i < len; i++)
		src_buf[i] = i;

	ut_assertok(dma_memcpy(dst_buf, src_buf, len));

	ut_assertok(memcmp(src_buf, dst_buf, len));
	return 0;
}
DM_TEST(dm_test_dma_m2m, DM_TESTF_SCAN_FDT);

static int dm_test_dma(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct dma dma_tx, dma_rx;
	u8 src_buf[512];
	u8 dst_buf[512];
	void *dst_ptr;
	size_t len = 512;
	u32 meta1, meta2;
	int i;

	ut_assertok(uclass_get_device_by_name(UCLASS_DMA, "dma", &dev));

	ut_assertok(dma_get_by_name(dev, "tx0", &dma_tx));
	ut_assertok(dma_get_by_name(dev, "rx0", &dma_rx));

	ut_assertok(dma_enable(&dma_tx));
	ut_assertok(dma_enable(&dma_rx));

	memset(dst_buf, 0, len);
	for (i = 0; i < len; i++)
		src_buf[i] = i;
	meta1 = 0xADADDEAD;
	meta2 = 0;
	dst_ptr = &dst_buf;

	ut_assertok(dma_send(&dma_tx, src_buf, len, &meta1));

	ut_asserteq(len, dma_receive(&dma_rx, &dst_ptr, &meta2));
	ut_asserteq(0xADADDEAD, meta2);

	ut_assertok(dma_disable(&dma_tx));
	ut_assertok(dma_disable(&dma_rx));

	ut_assertok(dma_free(&dma_tx));
	ut_assertok(dma_free(&dma_rx));
	ut_assertok(memcmp(src_buf, dst_buf, len));

	return 0;
}
DM_TEST(dm_test_dma, DM_TESTF_SCAN_FDT);

static int dm_test_dma_rx(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct dma dma_tx, dma_rx;
	u8 src_buf[512];
	u8 dst_buf[512];
	void *dst_ptr;
	size_t len = 512;
	u32 meta1, meta2;
	int i;

	ut_assertok(uclass_get_device_by_name(UCLASS_DMA, "dma", &dev));

	ut_assertok(dma_get_by_name(dev, "tx0", &dma_tx));
	ut_assertok(dma_get_by_name(dev, "rx0", &dma_rx));

	ut_assertok(dma_enable(&dma_tx));
	ut_assertok(dma_enable(&dma_rx));

	memset(dst_buf, 0, len);
	for (i = 0; i < len; i++)
		src_buf[i] = i;
	meta1 = 0xADADDEAD;
	meta2 = 0;
	dst_ptr = NULL;

	ut_assertok(dma_prepare_rcv_buf(&dma_tx, dst_buf, len));

	ut_assertok(dma_send(&dma_tx, src_buf, len, &meta1));

	ut_asserteq(len, dma_receive(&dma_rx, &dst_ptr, &meta2));
	ut_asserteq(0xADADDEAD, meta2);
	ut_asserteq_ptr(dst_buf, dst_ptr);

	ut_assertok(dma_disable(&dma_tx));
	ut_assertok(dma_disable(&dma_rx));

	ut_assertok(dma_free(&dma_tx));
	ut_assertok(dma_free(&dma_rx));
	ut_assertok(memcmp(src_buf, dst_buf, len));

	return 0;
}
DM_TEST(dm_test_dma_rx, DM_TESTF_SCAN_FDT);
