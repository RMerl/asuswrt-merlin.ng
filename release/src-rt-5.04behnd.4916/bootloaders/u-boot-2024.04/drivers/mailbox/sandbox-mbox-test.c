// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <mailbox.h>
#include <malloc.h>
#include <asm/io.h>

struct sandbox_mbox_test {
	struct mbox_chan chan;
};

int sandbox_mbox_test_get(struct udevice *dev)
{
	struct sandbox_mbox_test *sbmt = dev_get_priv(dev);

	return mbox_get_by_name(dev, "test", &sbmt->chan);
}

int sandbox_mbox_test_send(struct udevice *dev, uint32_t msg)
{
	struct sandbox_mbox_test *sbmt = dev_get_priv(dev);

	return mbox_send(&sbmt->chan, &msg);
}

int sandbox_mbox_test_recv(struct udevice *dev, uint32_t *msg)
{
	struct sandbox_mbox_test *sbmt = dev_get_priv(dev);

	return mbox_recv(&sbmt->chan, msg, 100);
}

int sandbox_mbox_test_free(struct udevice *dev)
{
	struct sandbox_mbox_test *sbmt = dev_get_priv(dev);

	return mbox_free(&sbmt->chan);
}

static const struct udevice_id sandbox_mbox_test_ids[] = {
	{ .compatible = "sandbox,mbox-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_mbox_test) = {
	.name = "sandbox_mbox_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_mbox_test_ids,
	.priv_auto	= sizeof(struct sandbox_mbox_test),
};
