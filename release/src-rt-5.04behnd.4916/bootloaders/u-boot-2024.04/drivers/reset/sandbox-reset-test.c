// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/reset.h>
#include <linux/err.h>

struct sandbox_reset_test {
	struct reset_ctl ctl;
	struct reset_ctl_bulk bulk;

	struct reset_ctl *ctlp;
	struct reset_ctl_bulk *bulkp;
};

int sandbox_reset_test_get(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	sbrt->ctlp = &sbrt->ctl;
	return reset_get_by_name(dev, "test", &sbrt->ctl);
}

int sandbox_reset_test_get_devm(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);
	struct reset_ctl *r;

	r = devm_reset_control_get(dev, "not-a-valid-reset-ctl");
	if (!IS_ERR(r))
		return -EINVAL;

	r = devm_reset_control_get_optional(dev, "not-a-valid-reset-ctl");
	if (r)
		return -EINVAL;

	sbrt->ctlp = devm_reset_control_get(dev, "test");
	if (IS_ERR(sbrt->ctlp))
		return PTR_ERR(sbrt->ctlp);

	return 0;
}

int sandbox_reset_test_get_bulk(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	sbrt->bulkp = &sbrt->bulk;
	return reset_get_bulk(dev, &sbrt->bulk);
}

int sandbox_reset_test_get_bulk_devm(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);
	struct reset_ctl_bulk *r;

	r = devm_reset_bulk_get_optional(dev);
	if (IS_ERR(r))
		return PTR_ERR(r);

	sbrt->bulkp = r;
	return 0;
}

int sandbox_reset_test_assert(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_assert(sbrt->ctlp);
}

int sandbox_reset_test_assert_bulk(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_assert_bulk(sbrt->bulkp);
}

int sandbox_reset_test_deassert(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_deassert(sbrt->ctlp);
}

int sandbox_reset_test_deassert_bulk(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_deassert_bulk(sbrt->bulkp);
}

int sandbox_reset_test_free(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_free(sbrt->ctlp);
}

int sandbox_reset_test_release_bulk(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_release_bulk(sbrt->bulkp);
}

static const struct udevice_id sandbox_reset_test_ids[] = {
	{ .compatible = "sandbox,reset-ctl-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_reset_test) = {
	.name = "sandbox_reset_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_reset_test_ids,
	.priv_auto	= sizeof(struct sandbox_reset_test),
};
