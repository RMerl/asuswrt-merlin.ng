// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 * Pratyush Yadav <p.yadav@ti.com>
 */
#include <common.h>
#include <dm.h>
#include <mux.h>
#include <mux-internal.h>
#include <dm/test.h>
#include <test/ut.h>
#include <asm/global_data.h>

struct mux_emul_priv {
	u32 state;
};

static int mux_emul_set(struct mux_control *mux, int state)
{
	struct mux_emul_priv *priv = dev_get_priv(mux->dev);

	priv->state = state;
	return 0;
}

static int mux_emul_probe(struct udevice *dev)
{
	struct mux_chip *mux_chip = dev_get_uclass_priv(dev);
	struct mux_control *mux;
	u32 idle_state;
	int ret;

	ret = mux_alloc_controllers(dev, 1);
	if (ret < 0)
		return ret;

	mux = &mux_chip->mux[0];

	ret = dev_read_u32(dev, "idle-state", &idle_state);
	if (ret)
		return ret;

	mux->idle_state = idle_state;
	mux->states = 0x100000;

	return 0;
}

static const struct mux_control_ops mux_emul_ops = {
	.set = mux_emul_set,
};

static const struct udevice_id mux_emul_of_match[] = {
	{ .compatible = "mux-emul" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(emul_mux) = {
	.name = "mux-emul",
	.id = UCLASS_MUX,
	.of_match = mux_emul_of_match,
	.ops = &mux_emul_ops,
	.probe = mux_emul_probe,
	.priv_auto	= sizeof(struct mux_emul_priv),
};

static int dm_test_mux_emul_default_state(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct mux_control *mux;
	struct mux_emul_priv *priv;

	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "a-test",
					      &dev));
	ut_assertok(mux_control_get(dev, "mux4", &mux));

	priv = dev_get_priv(mux->dev);

	ut_asserteq(0xabcd, priv->state);

	return 0;
}
DM_TEST(dm_test_mux_emul_default_state, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_mux_emul_select_deselect(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct mux_control *mux;
	struct mux_emul_priv *priv;

	gd->flags &= ~(GD_FLG_SILENT | GD_FLG_RECORD);
	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "a-test",
					      &dev));
	ut_assertok(mux_control_get(dev, "mux4", &mux));

	priv = dev_get_priv(mux->dev);

	ut_assertok(mux_control_select(mux, 0x1234));
	ut_asserteq(priv->state, 0x1234);

	ut_assertok(mux_control_deselect(mux));
	ut_asserteq(priv->state, 0xabcd);

	return 0;
}
DM_TEST(dm_test_mux_emul_select_deselect, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
