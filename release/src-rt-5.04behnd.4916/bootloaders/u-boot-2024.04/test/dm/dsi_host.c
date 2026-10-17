// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019 STMicroelectronics - All Rights Reserved
 * Author(s): Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <dm.h>
#include <dsi_host.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_dsi_host_phy_init(void *priv_data)
{
	return 0;
}

static void dm_test_dsi_host_phy_post_set_mode(void *priv_data,
					       unsigned long mode_flags)
{
}

static int dm_test_dsi_host_phy_get_lane_mbps(void *priv_data,
					      struct display_timing *timings,
					      u32 lanes,
					      u32 format,
					      unsigned int *lane_mbps)
{
	return 0;
}

static const struct mipi_dsi_phy_ops dm_test_dsi_host_phy_ops = {
	.init = dm_test_dsi_host_phy_init,
	.get_lane_mbps = dm_test_dsi_host_phy_get_lane_mbps,
	.post_set_mode = dm_test_dsi_host_phy_post_set_mode,
};

/* Test that dsi_host driver functions are called */
static int dm_test_dsi_host(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct mipi_dsi_device device;
	struct display_timing timings;
	unsigned int max_data_lanes = 4;

	ut_assertok(uclass_first_device_err(UCLASS_DSI_HOST, &dev));

	ut_assertok(dsi_host_init(dev, &device, &timings, max_data_lanes,
				  &dm_test_dsi_host_phy_ops));

	ut_assertok(dsi_host_enable(dev));

	return 0;
}

DM_TEST(dm_test_dsi_host, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
