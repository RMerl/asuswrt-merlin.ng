// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated - http://www.ti.com/
 */
#define pr_fmt(fmt) "%s: " fmt, __func__
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <remoteproc.h>

/**
 * enum sandbox_state - different device states
 * @sb_booted:	Entry condition, just booted
 * @sb_init:	Initialized (basic environment is ready)
 * @sb_reset:	Held in reset (accessible, but not running)
 * @sb_loaded:	Loaded with image (but not running)
 * @sb_running:	Processor is running
 */
enum sandbox_state {
	sb_booted,
	sb_init,
	sb_reset,
	sb_loaded,
	sb_running
};

/**
 * struct sandbox_test_devdata - private data per device
 * @current_state:	device current state
 */
struct sandbox_test_devdata {
	enum sandbox_state current_state;
};

/**
 * sandbox_dev_move_to_state() - statemachine for our dummy device
 * @dev:	device to switch state
 * @next_state:	next proposed state
 *
 * This tries to follow the following statemachine:
 *           Entry
 *            |
 *            v
 *         +-------+
 *     +---+ init  |
 *     |   |       | <---------------------+
 *     |   +-------+                       |
 *     |                                   |
 *     |                                   |
 *     |   +--------+                      |
 * Load|   |  reset |                      |
 *     |   |        | <----------+         |
 *     |   +--------+            |         |
 *     |        |Load            |         |
 *     |        |                |         |
 *     |   +----v----+   reset   |         |
 *     +-> |         |    (opt)  |         |
 *         |  Loaded +-----------+         |
 *         |         |                     |
 *         +----+----+                     |
 *              | Start                    |
 *          +---v-----+        (opt)       |
 *       +->| Running |        Stop        |
 * Ping  +- |         +--------------------+
 * (opt)    +---------+
 *
 * (is_running does not change state)
 *
 * Return: 0 when valid state transition is seen, else returns -EINVAL
 */
static int sandbox_dev_move_to_state(struct udevice *dev,
				     enum sandbox_state next_state)
{
	struct sandbox_test_devdata *ddata = dev_get_priv(dev);

	/* No state transition is OK */
	if (ddata->current_state == next_state)
		return 0;

	debug("current_state=%d, next_state=%d\n", ddata->current_state,
	      next_state);
	switch (ddata->current_state) {
	case sb_booted:
		if (next_state == sb_init)
			goto ok_state;
		break;

	case sb_init:
		if (next_state == sb_reset || next_state == sb_loaded)
			goto ok_state;
		break;

	case sb_reset:
		if (next_state == sb_loaded || next_state == sb_init)
			goto ok_state;
		break;

	case sb_loaded:
		if (next_state == sb_reset || next_state == sb_init ||
		    next_state == sb_running)
			goto ok_state;
		break;

	case sb_running:
		if (next_state == sb_reset || next_state == sb_init)
			goto ok_state;
		break;
	};
	return -EINVAL;

ok_state:
	ddata->current_state = next_state;
	return 0;
}

/**
 * sandbox_testproc_probe() - basic probe function
 * @dev:	test proc device that is being probed.
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int sandbox_testproc_probe(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	struct sandbox_test_devdata *ddata;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);
	ddata = dev_get_priv(dev);
	if (!ddata) {
		debug("%s: platform private data missing\n", uc_pdata->name);
		return -EINVAL;
	}
	ret = sandbox_dev_move_to_state(dev, sb_booted);
	debug("%s: called(%d)\n", uc_pdata->name, ret);

	return ret;
}

/**
 * sandbox_testproc_init() - Simple initialization function
 * @dev:	device to operate upon
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int sandbox_testproc_init(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);

	ret = sandbox_dev_move_to_state(dev, sb_init);

	debug("%s: called(%d)\n", uc_pdata->name, ret);
	if (ret)
		debug("%s init failed\n", uc_pdata->name);

	return ret;
}

/**
 * sandbox_testproc_reset() - Reset the remote processor
 * @dev:	device to operate upon
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int sandbox_testproc_reset(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);

	ret = sandbox_dev_move_to_state(dev, sb_reset);

	debug("%s: called(%d)\n", uc_pdata->name, ret);

	if (ret)
		debug("%s reset failed\n", uc_pdata->name);
	return ret;
}

/**
 * sandbox_testproc_load() - (replace: short desc)
 * @dev:	device to operate upon
 * @addr:	Address of the binary image to load
 * @size:	Size (in bytes) of the binary image to load
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int sandbox_testproc_load(struct udevice *dev, ulong addr, ulong size)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);

	ret = sandbox_dev_move_to_state(dev, sb_loaded);

	debug("%s: called(%d) Loading to %08lX %lu size\n",
	      uc_pdata->name, ret, addr, size);

	if (ret)
		debug("%s load failed\n", uc_pdata->name);
	return ret;
}

/**
 * sandbox_testproc_start() - Start the remote processor
 * @dev:	device to operate upon
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int sandbox_testproc_start(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);

	ret = sandbox_dev_move_to_state(dev, sb_running);

	debug("%s: called(%d)\n", uc_pdata->name, ret);

	if (ret)
		debug("%s start failed\n", uc_pdata->name);
	return ret;
}

/**
 * sandbox_testproc_stop() - Stop the remote processor
 * @dev:	device to operate upon
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int sandbox_testproc_stop(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);

	ret = sandbox_dev_move_to_state(dev, sb_init);

	debug("%s: called(%d)\n", uc_pdata->name, ret);

	if (ret)
		debug("%s stop failed\n", uc_pdata->name);
	return ret;
}

/**
 * sandbox_testproc_is_running() - Check if remote processor is running
 * @dev:	device to operate upon
 *
 * Return: 0 if running, 1 if not running
 */
static int sandbox_testproc_is_running(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	struct sandbox_test_devdata *ddata;
	int ret = 1;

	uc_pdata = dev_get_uclass_platdata(dev);
	ddata = dev_get_priv(dev);

	if (ddata->current_state == sb_running)
		ret = 0;
	debug("%s: called(%d)\n", uc_pdata->name, ret);

	return ret;
}

/**
 * sandbox_testproc_ping() - Try pinging remote processor
 * @dev:	device to operate upon
 *
 * Return: 0 if running, -EINVAL if not running
 */
static int sandbox_testproc_ping(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	struct sandbox_test_devdata *ddata;
	int ret;

	uc_pdata = dev_get_uclass_platdata(dev);
	ddata = dev_get_priv(dev);

	if (ddata->current_state == sb_running)
		ret = 0;
	else
		ret = -EINVAL;

	debug("%s: called(%d)\n", uc_pdata->name, ret);
	if (ret)
		debug("%s: No response.(Not started?)\n", uc_pdata->name);

	return ret;
}

static const struct dm_rproc_ops sandbox_testproc_ops = {
	.init = sandbox_testproc_init,
	.reset = sandbox_testproc_reset,
	.load = sandbox_testproc_load,
	.start = sandbox_testproc_start,
	.stop = sandbox_testproc_stop,
	.is_running = sandbox_testproc_is_running,
	.ping = sandbox_testproc_ping,
};

static const struct udevice_id sandbox_ids[] = {
	{.compatible = "sandbox,test-processor"},
	{}
};

U_BOOT_DRIVER(sandbox_testproc) = {
	.name = "sandbox_test_proc",
	.of_match = sandbox_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &sandbox_testproc_ops,
	.probe = sandbox_testproc_probe,
	.priv_auto_alloc_size = sizeof(struct sandbox_test_devdata),
};

/* TODO(nm@ti.com): Remove this along with non-DT support */
static struct dm_rproc_uclass_pdata proc_3_test = {
	.name = "proc_3_legacy",
	.mem_type = RPROC_INTERNAL_MEMORY_MAPPED,
};

U_BOOT_DEVICE(proc_3_demo) = {
	.name = "sandbox_test_proc",
	.platdata = &proc_3_test,
};
