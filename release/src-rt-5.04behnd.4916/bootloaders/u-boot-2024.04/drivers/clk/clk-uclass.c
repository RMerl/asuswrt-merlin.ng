// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016, NVIDIA CORPORATION.
 * Copyright (c) 2018, Theobroma Systems Design und Consulting GmbH
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/devres.h>
#include <dm/read.h>
#include <linux/bug.h>
#include <linux/clk-provider.h>
#include <linux/err.h>

static inline const struct clk_ops *clk_dev_ops(struct udevice *dev)
{
	return (const struct clk_ops *)dev->driver->ops;
}

struct clk *dev_get_clk_ptr(struct udevice *dev)
{
	return (struct clk *)dev_get_uclass_priv(dev);
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
int clk_get_by_phandle(struct udevice *dev, const struct phandle_1_arg *cells,
		       struct clk *clk)
{
	int ret;

	ret = device_get_by_ofplat_idx(cells->idx, &clk->dev);
	if (ret)
		return ret;
	clk->id = cells->arg[0];

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(OF_REAL)
static int clk_of_xlate_default(struct clk *clk,
				struct ofnode_phandle_args *args)
{
	debug("%s(clk=%p)\n", __func__, clk);

	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = args->args[0];
	else
		clk->id = 0;

	clk->data = 0;

	return 0;
}

static int clk_get_by_index_tail(int ret, ofnode node,
				 struct ofnode_phandle_args *args,
				 const char *list_name, int index,
				 struct clk *clk)
{
	struct udevice *dev_clk;
	const struct clk_ops *ops;

	assert(clk);
	clk->dev = NULL;
	if (ret)
		goto err;

	ret = uclass_get_device_by_ofnode(UCLASS_CLK, args->node, &dev_clk);
	if (ret) {
		debug("%s: uclass_get_device_by_of_offset failed: err=%d\n",
		      __func__, ret);
		return log_msg_ret("get", ret);
	}

	clk->dev = dev_clk;

	ops = clk_dev_ops(dev_clk);

	if (ops->of_xlate)
		ret = ops->of_xlate(clk, args);
	else
		ret = clk_of_xlate_default(clk, args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		return log_msg_ret("xlate", ret);
	}

	return clk_request(dev_clk, clk);
err:
	debug("%s: Node '%s', property '%s', failed to request CLK index %d: %d\n",
	      __func__, ofnode_get_name(node), list_name, index, ret);

	return log_msg_ret("prop", ret);
}

static int clk_get_by_indexed_prop(struct udevice *dev, const char *prop_name,
				   int index, struct clk *clk)
{
	int ret;
	struct ofnode_phandle_args args;

	debug("%s(dev=%p, index=%d, clk=%p)\n", __func__, dev, index, clk);

	assert(clk);
	clk->dev = NULL;

	ret = dev_read_phandle_with_args(dev, prop_name, "#clock-cells", 0,
					 index, &args);
	if (ret) {
		debug("%s: fdtdec_parse_phandle_with_args failed: err=%d\n",
		      __func__, ret);
		return log_ret(ret);
	}


	return clk_get_by_index_tail(ret, dev_ofnode(dev), &args, "clocks",
				     index, clk);
}

int clk_get_by_index(struct udevice *dev, int index, struct clk *clk)
{
	return clk_get_by_index_nodev(dev_ofnode(dev), index, clk);
}

int clk_get_by_index_nodev(ofnode node, int index, struct clk *clk)
{
	struct ofnode_phandle_args args;
	int ret;

	ret = ofnode_parse_phandle_with_args(node, "clocks", "#clock-cells", 0,
					     index, &args);

	return clk_get_by_index_tail(ret, node, &args, "clocks",
				     index, clk);
}

int clk_get_bulk(struct udevice *dev, struct clk_bulk *bulk)
{
	int i, ret, err, count;

	bulk->count = 0;

	count = dev_count_phandle_with_args(dev, "clocks", "#clock-cells", 0);
	if (count < 1)
		return count;

	bulk->clks = devm_kcalloc(dev, count, sizeof(struct clk), GFP_KERNEL);
	if (!bulk->clks)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		ret = clk_get_by_index(dev, i, &bulk->clks[i]);
		if (ret < 0)
			goto bulk_get_err;

		++bulk->count;
	}

	return 0;

bulk_get_err:
	err = clk_release_all(bulk->clks, bulk->count);
	if (err)
		debug("%s: could release all clocks for %p\n",
		      __func__, dev);

	return ret;
}

static struct clk *clk_set_default_get_by_id(struct clk *clk)
{
	struct clk *c = clk;

	if (CONFIG_IS_ENABLED(CLK_CCF)) {
		int ret = clk_get_by_id(clk->id, &c);

		if (ret) {
			debug("%s(): could not get parent clock pointer, id %lu\n",
			      __func__, clk->id);
			ERR_PTR(ret);
		}
	}

	return c;
}

static int clk_set_default_parents(struct udevice *dev,
				   enum clk_defaults_stage stage)
{
	struct clk clk, parent_clk, *c, *p;
	int index;
	int num_parents;
	int ret;

	num_parents = dev_count_phandle_with_args(dev, "assigned-clock-parents",
						  "#clock-cells", 0);
	if (num_parents < 0) {
		debug("%s: could not read assigned-clock-parents for %p\n",
		      __func__, dev);
		return 0;
	}

	for (index = 0; index < num_parents; index++) {
		ret = clk_get_by_indexed_prop(dev, "assigned-clock-parents",
					      index, &parent_clk);
		/* If -ENOENT, this is a no-op entry */
		if (ret == -ENOENT)
			continue;

		if (ret) {
			debug("%s: could not get parent clock %d for %s\n",
			      __func__, index, dev_read_name(dev));
			return ret;
		}

		p = clk_set_default_get_by_id(&parent_clk);
		if (IS_ERR(p))
			return PTR_ERR(p);

		ret = clk_get_by_indexed_prop(dev, "assigned-clocks",
					      index, &clk);
		/*
		 * If the clock provider is not ready yet, let it handle
		 * the re-programming later.
		 */
		if (ret == -EPROBE_DEFER) {
			ret = 0;
			continue;
		}

		if (ret) {
			debug("%s: could not get assigned clock %d for %s\n",
			      __func__, index, dev_read_name(dev));
			return ret;
		}

		/* This is clk provider device trying to reparent itself
		 * It cannot be done right now but need to wait after the
		 * device is probed
		 */
		if (stage == CLK_DEFAULTS_PRE && clk.dev == dev)
			continue;

		if (stage != CLK_DEFAULTS_PRE && clk.dev != dev)
			/* do not setup twice the parent clocks */
			continue;

		c = clk_set_default_get_by_id(&clk);
		if (IS_ERR(c))
			return PTR_ERR(c);

		ret = clk_set_parent(c, p);
		/*
		 * Not all drivers may support clock-reparenting (as of now).
		 * Ignore errors due to this.
		 */
		if (ret == -ENOSYS)
			continue;

		if (ret < 0) {
			debug("%s: failed to reparent clock %d for %s\n",
			      __func__, index, dev_read_name(dev));
			return ret;
		}
	}

	return 0;
}

static int clk_set_default_rates(struct udevice *dev,
				 enum clk_defaults_stage stage)
{
	struct clk clk, *c;
	int index;
	int num_rates;
	int size;
	int ret = 0;
	u32 *rates = NULL;

	size = dev_read_size(dev, "assigned-clock-rates");
	if (size < 0)
		return 0;

	num_rates = size / sizeof(u32);
	rates = calloc(num_rates, sizeof(u32));
	if (!rates)
		return -ENOMEM;

	ret = dev_read_u32_array(dev, "assigned-clock-rates", rates, num_rates);
	if (ret)
		goto fail;

	for (index = 0; index < num_rates; index++) {
		/* If 0 is passed, this is a no-op */
		if (!rates[index])
			continue;

		ret = clk_get_by_indexed_prop(dev, "assigned-clocks",
					      index, &clk);
		/*
		 * If the clock provider is not ready yet, let it handle
		 * the re-programming later.
		 */
		if (ret == -EPROBE_DEFER) {
			ret = 0;
			continue;
		}

		if (ret) {
			dev_dbg(dev,
				"could not get assigned clock %d (err = %d)\n",
				index, ret);
			/* Skip if it is empty */
			if (ret == -ENOENT) {
				ret = 0;
				continue;
			}

			return ret;
		}

		/* This is clk provider device trying to program itself
		 * It cannot be done right now but need to wait after the
		 * device is probed
		 */
		if (stage == CLK_DEFAULTS_PRE && clk.dev == dev)
			continue;

		if (stage != CLK_DEFAULTS_PRE && clk.dev != dev)
			/* do not setup twice the parent clocks */
			continue;

		c = clk_set_default_get_by_id(&clk);
		if (IS_ERR(c))
			return PTR_ERR(c);

		ret = clk_set_rate(c, rates[index]);

		if (ret < 0) {
			dev_warn(dev,
				 "failed to set rate on clock index %d (%ld) (error = %d)\n",
				 index, clk.id, ret);
			break;
		}
	}

fail:
	free(rates);
	return ret;
}

int clk_set_defaults(struct udevice *dev, enum clk_defaults_stage stage)
{
	int ret;

	if (!dev_has_ofnode(dev))
		return 0;

	/*
	 * To avoid setting defaults twice, don't set them before relocation.
	 * However, still set them for SPL. And still set them if explicitly
	 * asked.
	 */
	if (!(IS_ENABLED(CONFIG_SPL_BUILD) || (gd->flags & GD_FLG_RELOC)))
		if (stage != CLK_DEFAULTS_POST_FORCE)
			return 0;

	debug("%s(%s)\n", __func__, dev_read_name(dev));

	ret = clk_set_default_parents(dev, stage);
	if (ret)
		return ret;

	ret = clk_set_default_rates(dev, stage);
	if (ret < 0)
		return ret;

	return 0;
}

int clk_get_by_name(struct udevice *dev, const char *name, struct clk *clk)
{
	return clk_get_by_name_nodev(dev_ofnode(dev), name, clk);
}
#endif /* OF_REAL */

int clk_get_by_name_nodev(ofnode node, const char *name, struct clk *clk)
{
	int index = 0;

	debug("%s(node=%p, name=%s, clk=%p)\n", __func__,
		ofnode_get_name(node), name, clk);
	clk->dev = NULL;

	if (name) {
		index = ofnode_stringlist_search(node, "clock-names", name);
		if (index < 0) {
			debug("fdt_stringlist_search() failed: %d\n", index);
			return index;
		}
	}

	return clk_get_by_index_nodev(node, index, clk);
}

int clk_release_all(struct clk *clk, unsigned int count)
{
	unsigned int i;
	int ret;

	for (i = 0; i < count; i++) {
		debug("%s(clk[%u]=%p)\n", __func__, i, &clk[i]);

		/* check if clock has been previously requested */
		if (!clk[i].dev)
			continue;

		ret = clk_disable(&clk[i]);
		if (ret && ret != -ENOSYS)
			return ret;
	}

	return 0;
}

int clk_request(struct udevice *dev, struct clk *clk)
{
	const struct clk_ops *ops;

	debug("%s(dev=%p, clk=%p)\n", __func__, dev, clk);
	if (!clk)
		return 0;
	ops = clk_dev_ops(dev);

	clk->dev = dev;

	if (!ops->request)
		return 0;

	return ops->request(clk);
}

ulong clk_get_rate(struct clk *clk)
{
	const struct clk_ops *ops;

	debug("%s(clk=%p)\n", __func__, clk);
	if (!clk_valid(clk))
		return 0;
	ops = clk_dev_ops(clk->dev);

	if (!ops->get_rate)
		return -ENOSYS;

	return ops->get_rate(clk);
}

struct clk *clk_get_parent(struct clk *clk)
{
	struct udevice *pdev;
	struct clk *pclk;

	debug("%s(clk=%p)\n", __func__, clk);
	if (!clk_valid(clk))
		return NULL;

	pdev = dev_get_parent(clk->dev);
	if (!pdev)
		return ERR_PTR(-ENODEV);
	pclk = dev_get_clk_ptr(pdev);
	if (!pclk)
		return ERR_PTR(-ENODEV);

	return pclk;
}

ulong clk_get_parent_rate(struct clk *clk)
{
	const struct clk_ops *ops;
	struct clk *pclk;

	debug("%s(clk=%p)\n", __func__, clk);
	if (!clk_valid(clk))
		return 0;

	pclk = clk_get_parent(clk);
	if (IS_ERR(pclk))
		return -ENODEV;

	ops = clk_dev_ops(pclk->dev);
	if (!ops->get_rate)
		return -ENOSYS;

	/* Read the 'rate' if not already set or if proper flag set*/
	if (!pclk->rate || pclk->flags & CLK_GET_RATE_NOCACHE)
		pclk->rate = clk_get_rate(pclk);

	return pclk->rate;
}

ulong clk_round_rate(struct clk *clk, ulong rate)
{
	const struct clk_ops *ops;

	debug("%s(clk=%p, rate=%lu)\n", __func__, clk, rate);
	if (!clk_valid(clk))
		return 0;

	ops = clk_dev_ops(clk->dev);
	if (!ops->round_rate)
		return -ENOSYS;

	return ops->round_rate(clk, rate);
}

static void clk_get_priv(struct clk *clk, struct clk **clkp)
{
	*clkp = clk;

	/* get private clock struct associated to the provided clock */
	if (CONFIG_IS_ENABLED(CLK_CCF)) {
		/* Take id 0 as a non-valid clk, such as dummy */
		if (clk->id)
			clk_get_by_id(clk->id, clkp);
	}
}

/* clean cache, called with private clock struct */
static void clk_clean_rate_cache(struct clk *clk)
{
	struct udevice *child_dev;
	struct clk *clkp;

	if (!clk)
		return;

	clk->rate = 0;

	list_for_each_entry(child_dev, &clk->dev->child_head, sibling_node) {
		clkp = dev_get_clk_ptr(child_dev);
		clk_clean_rate_cache(clkp);
	}
}

ulong clk_set_rate(struct clk *clk, ulong rate)
{
	const struct clk_ops *ops;
	struct clk *clkp;

	debug("%s(clk=%p, rate=%lu)\n", __func__, clk, rate);
	if (!clk_valid(clk))
		return 0;
	ops = clk_dev_ops(clk->dev);

	if (!ops->set_rate)
		return -ENOSYS;

	/* get private clock struct used for cache */
	clk_get_priv(clk, &clkp);
	/* Clean up cached rates for us and all child clocks */
	clk_clean_rate_cache(clkp);

	return ops->set_rate(clk, rate);
}

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	const struct clk_ops *ops;
	int ret;

	debug("%s(clk=%p, parent=%p)\n", __func__, clk, parent);
	if (!clk_valid(clk))
		return 0;
	ops = clk_dev_ops(clk->dev);

	if (!ops->set_parent)
		return -ENOSYS;

	ret = ops->set_parent(clk, parent);
	if (ret)
		return ret;

	if (CONFIG_IS_ENABLED(CLK_CCF))
		ret = device_reparent(clk->dev, parent->dev);

	return ret;
}

int clk_enable(struct clk *clk)
{
	const struct clk_ops *ops;
	struct clk *clkp = NULL;
	int ret;

	debug("%s(clk=%p)\n", __func__, clk);
	if (!clk_valid(clk))
		return 0;
	ops = clk_dev_ops(clk->dev);

	if (CONFIG_IS_ENABLED(CLK_CCF)) {
		/* Take id 0 as a non-valid clk, such as dummy */
		if (clk->id && !clk_get_by_id(clk->id, &clkp)) {
			ops = clk_dev_ops(clkp->dev);
			if (clkp->enable_count) {
				clkp->enable_count++;
				return 0;
			}
			if (clkp->dev->parent &&
			    device_get_uclass_id(clkp->dev->parent) == UCLASS_CLK) {
				ret = clk_enable(dev_get_clk_ptr(clkp->dev->parent));
				if (ret) {
					printf("Enable %s failed\n",
					       clkp->dev->parent->name);
					return ret;
				}
			}
		}

		if (ops->enable) {
			ret = ops->enable(clkp ? clkp : clk);
			if (ret) {
				printf("Enable %s failed\n", clk->dev->name);
				return ret;
			}
		}
		if (clkp)
			clkp->enable_count++;
	} else {
		if (!ops->enable)
			return -ENOSYS;
		return ops->enable(clk);
	}

	return 0;
}

int clk_enable_bulk(struct clk_bulk *bulk)
{
	int i, ret;

	for (i = 0; i < bulk->count; i++) {
		ret = clk_enable(&bulk->clks[i]);
		if (ret < 0 && ret != -ENOSYS)
			return ret;
	}

	return 0;
}

int clk_disable(struct clk *clk)
{
	const struct clk_ops *ops;
	struct clk *clkp = NULL;
	int ret;

	debug("%s(clk=%p)\n", __func__, clk);
	if (!clk_valid(clk))
		return 0;
	ops = clk_dev_ops(clk->dev);

	if (CONFIG_IS_ENABLED(CLK_CCF)) {
		if (clk->id && !clk_get_by_id(clk->id, &clkp)) {
			ops = clk_dev_ops(clkp->dev);
			if (clkp->flags & CLK_IS_CRITICAL)
				return 0;

			if (clkp->enable_count == 0) {
				printf("clk %s already disabled\n",
				       clkp->dev->name);
				return 0;
			}

			if (--clkp->enable_count > 0)
				return 0;
		}

		if (ops->disable) {
			ret = ops->disable(clkp ? clkp : clk);
			if (ret)
				return ret;
		}

		if (clkp && clkp->dev->parent &&
		    device_get_uclass_id(clkp->dev->parent) == UCLASS_CLK) {
			ret = clk_disable(dev_get_clk_ptr(clkp->dev->parent));
			if (ret) {
				printf("Disable %s failed\n",
				       clkp->dev->parent->name);
				return ret;
			}
		}
	} else {
		if (!ops->disable)
			return -ENOSYS;

		return ops->disable(clk);
	}

	return 0;
}

int clk_disable_bulk(struct clk_bulk *bulk)
{
	int i, ret;

	for (i = 0; i < bulk->count; i++) {
		ret = clk_disable(&bulk->clks[i]);
		if (ret < 0 && ret != -ENOSYS)
			return ret;
	}

	return 0;
}

int clk_get_by_id(ulong id, struct clk **clkp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_CLK, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		struct clk *clk = dev_get_clk_ptr(dev);

		if (clk && clk->id == id) {
			*clkp = clk;
			return 0;
		}
	}

	return -ENOENT;
}

bool clk_is_match(const struct clk *p, const struct clk *q)
{
	/* trivial case: identical struct clk's or both NULL */
	if (p == q)
		return true;

	/* trivial case #2: on the clk pointer is NULL */
	if (!p || !q)
		return false;

	/* same device, id and data */
	if (p->dev == q->dev && p->id == q->id && p->data == q->data)
		return true;

	return false;
}

struct clk *devm_clk_get(struct udevice *dev, const char *id)
{
	int rc;
	struct clk *clk;

	clk = devm_kzalloc(dev, sizeof(*clk), GFP_KERNEL);
	if (unlikely(!clk))
		return ERR_PTR(-ENOMEM);

	rc = clk_get_by_name(dev, id, clk);
	if (rc)
		return ERR_PTR(rc);

	return clk;
}

int clk_uclass_post_probe(struct udevice *dev)
{
	/*
	 * when a clock provider is probed. Call clk_set_defaults()
	 * also after the device is probed. This takes care of cases
	 * where the DT is used to setup default parents and rates
	 * using assigned-clocks
	 */
	clk_set_defaults(dev, CLK_DEFAULTS_POST);

	return 0;
}

UCLASS_DRIVER(clk) = {
	.id		= UCLASS_CLK,
	.name		= "clk",
	.post_probe	= clk_uclass_post_probe,
};
