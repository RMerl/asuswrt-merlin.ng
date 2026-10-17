// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020-2021 NXP
 */

#include <net/dsa.h>
#include <dm/test.h>
#include <test/ut.h>
#include <net.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

/* This test exercises the major dsa.h API functions, after making sure
 * that the DSA ports and the master Eth are correctly probed.
 */
static int dm_test_dsa_probe(struct unit_test_state *uts)
{
	struct udevice *dev_dsa, *dev_port, *dev_master;
	struct dsa_pdata *dsa_pdata;
	enum uclass_id id;

	id = uclass_get_by_name("dsa");
	ut_assert(id == UCLASS_DSA);

	ut_assertok(uclass_find_device_by_name(UCLASS_DSA, "dsa-test",
					       &dev_dsa));
	ut_assertok(uclass_find_device_by_name(UCLASS_ETH, "dsa-test-eth",
					       &dev_master));
	ut_assertok(device_probe(dev_master));

	ut_assertok(uclass_find_device_by_name(UCLASS_ETH, "dsa-test@0",
					       &dev_port));
	ut_assertok(device_probe(dev_port));

	ut_assertok(uclass_find_device_by_name(UCLASS_ETH, "dsa-test@1",
					       &dev_port));
	ut_assertok(device_probe(dev_port));

	/* exercise DSA API */
	dsa_pdata = dev_get_uclass_plat(dev_dsa);
	ut_assertnonnull(dsa_pdata);
	/* includes CPU port */
	ut_assert(dsa_pdata->num_ports == 3);

	ut_assertok(uclass_find_device_by_name(UCLASS_ETH, "lan0",
					       &dev_port));
	ut_assertok(device_probe(dev_port));

	ut_assertok(uclass_find_device_by_name(UCLASS_ETH, "lan1",
					       &dev_port));
	ut_assertok(device_probe(dev_port));

	dev_master = dsa_get_master(dev_dsa);
	ut_assertnonnull(dev_master);
	ut_asserteq_str("dsa-test-eth", dev_master->name);

	return 0;
}

DM_TEST(dm_test_dsa_probe, UT_TESTF_SCAN_FDT);

/* This test sends ping requests with the local address through each DSA port
 * via the sandbox DSA master Eth.
 */
static int dm_test_dsa(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.2.3.5");

	env_set("ethact", "eth2");
	ut_assertok(net_loop(PING));

	env_set("ethact", "lan0");
	ut_assertok(net_loop(PING));
	env_set("ethact", "lan1");
	ut_assertok(net_loop(PING));

	env_set("ethact", "");

	return 0;
}

DM_TEST(dm_test_dsa, UT_TESTF_SCAN_FDT);
