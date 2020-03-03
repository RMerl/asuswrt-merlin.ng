/*
 * phy-core.c  --  Generic Phy framework.
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Kishon Vijay Abraham I <kishon@ti.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/phy/phy.h>
#include <linux/idr.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>

static struct class *phy_class;
static DEFINE_MUTEX(phy_provider_mutex);
static LIST_HEAD(phy_provider_list);
static LIST_HEAD(phys);
static DEFINE_IDA(phy_ida);

static void devm_phy_release(struct device *dev, void *res)
{
	struct phy *phy = *(struct phy **)res;

	phy_put(phy);
}

static void devm_phy_provider_release(struct device *dev, void *res)
{
	struct phy_provider *phy_provider = *(struct phy_provider **)res;

	of_phy_provider_unregister(phy_provider);
}

static void devm_phy_consume(struct device *dev, void *res)
{
	struct phy *phy = *(struct phy **)res;

	phy_destroy(phy);
}

static int devm_phy_match(struct device *dev, void *res, void *match_data)
{
	struct phy **phy = res;

	return *phy == match_data;
}

/**
 * phy_create_lookup() - allocate and register PHY/device association
 * @phy: the phy of the association
 * @con_id: connection ID string on device
 * @dev_id: the device of the association
 *
 * Creates and registers phy_lookup entry.
 */
int phy_create_lookup(struct phy *phy, const char *con_id, const char *dev_id)
{
	struct phy_lookup *pl;

	if (!phy || !dev_id || !con_id)
		return -EINVAL;

	pl = kzalloc(sizeof(*pl), GFP_KERNEL);
	if (!pl)
		return -ENOMEM;

	pl->dev_id = dev_id;
	pl->con_id = con_id;
	pl->phy = phy;

	mutex_lock(&phy_provider_mutex);
	list_add_tail(&pl->node, &phys);
	mutex_unlock(&phy_provider_mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(phy_create_lookup);

/**
 * phy_remove_lookup() - find and remove PHY/device association
 * @phy: the phy of the association
 * @con_id: connection ID string on device
 * @dev_id: the device of the association
 *
 * Finds and unregisters phy_lookup entry that was created with
 * phy_create_lookup().
 */
void phy_remove_lookup(struct phy *phy, const char *con_id, const char *dev_id)
{
	struct phy_lookup *pl;

	if (!phy || !dev_id || !con_id)
		return;

	mutex_lock(&phy_provider_mutex);
	list_for_each_entry(pl, &phys, node)
		if (pl->phy == phy && !strcmp(pl->dev_id, dev_id) &&
		    !strcmp(pl->con_id, con_id)) {
			list_del(&pl->node);
			kfree(pl);
			break;
		}
	mutex_unlock(&phy_provider_mutex);
}
EXPORT_SYMBOL_GPL(phy_remove_lookup);

static struct phy *phy_find(struct device *dev, const char *con_id)
{
	const char *dev_id = dev_name(dev);
	struct phy_lookup *p, *pl = NULL;

	mutex_lock(&phy_provider_mutex);
	list_for_each_entry(p, &phys, node)
		if (!strcmp(p->dev_id, dev_id) && !strcmp(p->con_id, con_id)) {
			pl = p;
			break;
		}
	mutex_unlock(&phy_provider_mutex);

	return pl ? pl->phy : ERR_PTR(-ENODEV);
}

static struct phy_provider *of_phy_provider_lookup(struct device_node *node)
{
	struct phy_provider *phy_provider;
	struct device_node *child;

	list_for_each_entry(phy_provider, &phy_provider_list, list) {
		if (phy_provider->dev->of_node == node)
			return phy_provider;

		for_each_child_of_node(phy_provider->dev->of_node, child)
			if (child == node)
				return phy_provider;
	}

	return ERR_PTR(-EPROBE_DEFER);
}

int phy_pm_runtime_get(struct phy *phy)
{
	int ret;

	if (!pm_runtime_enabled(&phy->dev))
		return -ENOTSUPP;

	ret = pm_runtime_get(&phy->dev);
	if (ret < 0 && ret != -EINPROGRESS)
		pm_runtime_put_noidle(&phy->dev);

	return ret;
}
EXPORT_SYMBOL_GPL(phy_pm_runtime_get);

int phy_pm_runtime_get_sync(struct phy *phy)
{
	int ret;

	if (!pm_runtime_enabled(&phy->dev))
		return -ENOTSUPP;

	ret = pm_runtime_get_sync(&phy->dev);
	if (ret < 0)
		pm_runtime_put_sync(&phy->dev);

	return ret;
}
EXPORT_SYMBOL_GPL(phy_pm_runtime_get_sync);

int phy_pm_runtime_put(struct phy *phy)
{
	if (!pm_runtime_enabled(&phy->dev))
		return -ENOTSUPP;

	return pm_runtime_put(&phy->dev);
}
EXPORT_SYMBOL_GPL(phy_pm_runtime_put);

int phy_pm_runtime_put_sync(struct phy *phy)
{
	if (!pm_runtime_enabled(&phy->dev))
		return -ENOTSUPP;

	return pm_runtime_put_sync(&phy->dev);
}
EXPORT_SYMBOL_GPL(phy_pm_runtime_put_sync);

void phy_pm_runtime_allow(struct phy *phy)
{
	if (!pm_runtime_enabled(&phy->dev))
		return;

	pm_runtime_allow(&phy->dev);
}
EXPORT_SYMBOL_GPL(phy_pm_runtime_allow);

void phy_pm_runtime_forbid(struct phy *phy)
{
	if (!pm_runtime_enabled(&phy->dev))
		return;

	pm_runtime_forbid(&phy->dev);
}
EXPORT_SYMBOL_GPL(phy_pm_runtime_forbid);

int phy_init(struct phy *phy)
{
	int ret;

	if (!phy)
		return 0;

	ret = phy_pm_runtime_get_sync(phy);
	if (ret < 0 && ret != -ENOTSUPP)
		return ret;
	ret = 0; /* Override possible ret == -ENOTSUPP */

	mutex_lock(&phy->mutex);
	if (phy->init_count == 0 && phy->ops->init) {
		ret = phy->ops->init(phy);
		if (ret < 0) {
			dev_err(&phy->dev, "phy init failed --> %d\n", ret);
			goto out;
		}
	}
	++phy->init_count;

out:
	mutex_unlock(&phy->mutex);
	phy_pm_runtime_put(phy);
	return ret;
}
EXPORT_SYMBOL_GPL(phy_init);

int phy_exit(struct phy *phy)
{
	int ret;

	if (!phy)
		return 0;

	ret = phy_pm_runtime_get_sync(phy);
	if (ret < 0 && ret != -ENOTSUPP)
		return ret;
	ret = 0; /* Override possible ret == -ENOTSUPP */

	mutex_lock(&phy->mutex);
	if (phy->init_count == 1 && phy->ops->exit) {
		ret = phy->ops->exit(phy);
		if (ret < 0) {
			dev_err(&phy->dev, "phy exit failed --> %d\n", ret);
			goto out;
		}
	}
	--phy->init_count;

out:
	mutex_unlock(&phy->mutex);
	phy_pm_runtime_put(phy);
	return ret;
}
EXPORT_SYMBOL_GPL(phy_exit);

int phy_power_on(struct phy *phy)
{
	int ret = 0;

	if (!phy)
		goto out;

	if (phy->pwr) {
		ret = regulator_enable(phy->pwr);
		if (ret)
			goto out;
	}

	ret = phy_pm_runtime_get_sync(phy);
	if (ret < 0 && ret != -ENOTSUPP)
		goto err_pm_sync;

	ret = 0; /* Override possible ret == -ENOTSUPP */

	mutex_lock(&phy->mutex);
	if (phy->power_count == 0 && phy->ops->power_on) {
		ret = phy->ops->power_on(phy);
		if (ret < 0) {
			dev_err(&phy->dev, "phy poweron failed --> %d\n", ret);
			goto err_pwr_on;
		}
	}
	++phy->power_count;
	mutex_unlock(&phy->mutex);
	return 0;

err_pwr_on:
	mutex_unlock(&phy->mutex);
	phy_pm_runtime_put_sync(phy);
err_pm_sync:
	if (phy->pwr)
		regulator_disable(phy->pwr);
out:
	return ret;
}
EXPORT_SYMBOL_GPL(phy_power_on);

int phy_power_off(struct phy *phy)
{
	int ret;

	if (!phy)
		return 0;

	mutex_lock(&phy->mutex);
	if (phy->power_count == 1 && phy->ops->power_off) {
		ret =  phy->ops->power_off(phy);
		if (ret < 0) {
			dev_err(&phy->dev, "phy poweroff failed --> %d\n", ret);
			mutex_unlock(&phy->mutex);
			return ret;
		}
	}
	--phy->power_count;
	mutex_unlock(&phy->mutex);
	phy_pm_runtime_put(phy);

	if (phy->pwr)
		regulator_disable(phy->pwr);

	return 0;
}
EXPORT_SYMBOL_GPL(phy_power_off);

/**
 * _of_phy_get() - lookup and obtain a reference to a phy by phandle
 * @np: device_node for which to get the phy
 * @index: the index of the phy
 *
 * Returns the phy associated with the given phandle value,
 * after getting a refcount to it or -ENODEV if there is no such phy or
 * -EPROBE_DEFER if there is a phandle to the phy, but the device is
 * not yet loaded. This function uses of_xlate call back function provided
 * while registering the phy_provider to find the phy instance.
 */
static struct phy *_of_phy_get(struct device_node *np, int index)
{
	int ret;
	struct phy_provider *phy_provider;
	struct phy *phy = NULL;
	struct of_phandle_args args;

	ret = of_parse_phandle_with_args(np, "phys", "#phy-cells",
		index, &args);
	if (ret)
		return ERR_PTR(-ENODEV);

	mutex_lock(&phy_provider_mutex);
	phy_provider = of_phy_provider_lookup(args.np);
	if (IS_ERR(phy_provider) || !try_module_get(phy_provider->owner)) {
		phy = ERR_PTR(-EPROBE_DEFER);
		goto err0;
	}

	phy = phy_provider->of_xlate(phy_provider->dev, &args);
	module_put(phy_provider->owner);

err0:
	mutex_unlock(&phy_provider_mutex);
	of_node_put(args.np);

	return phy;
}

/**
 * of_phy_get() - lookup and obtain a reference to a phy using a device_node.
 * @np: device_node for which to get the phy
 * @con_id: name of the phy from device's point of view
 *
 * Returns the phy driver, after getting a refcount to it; or
 * -ENODEV if there is no such phy. The caller is responsible for
 * calling phy_put() to release that count.
 */
struct phy *of_phy_get(struct device_node *np, const char *con_id)
{
	struct phy *phy = NULL;
	int index = 0;

	if (con_id)
		index = of_property_match_string(np, "phy-names", con_id);

	phy = _of_phy_get(np, index);
	if (IS_ERR(phy))
		return phy;

	if (!try_module_get(phy->ops->owner))
		return ERR_PTR(-EPROBE_DEFER);

	get_device(&phy->dev);

	return phy;
}
EXPORT_SYMBOL_GPL(of_phy_get);

/**
 * phy_put() - release the PHY
 * @phy: the phy returned by phy_get()
 *
 * Releases a refcount the caller received from phy_get().
 */
void phy_put(struct phy *phy)
{
	if (!phy || IS_ERR(phy))
		return;

	module_put(phy->ops->owner);
	put_device(&phy->dev);
}
EXPORT_SYMBOL_GPL(phy_put);

/**
 * devm_phy_put() - release the PHY
 * @dev: device that wants to release this phy
 * @phy: the phy returned by devm_phy_get()
 *
 * destroys the devres associated with this phy and invokes phy_put
 * to release the phy.
 */
void devm_phy_put(struct device *dev, struct phy *phy)
{
	int r;

	if (!phy)
		return;

	r = devres_destroy(dev, devm_phy_release, devm_phy_match, phy);
	dev_WARN_ONCE(dev, r, "couldn't find PHY resource\n");
}
EXPORT_SYMBOL_GPL(devm_phy_put);

/**
 * of_phy_simple_xlate() - returns the phy instance from phy provider
 * @dev: the PHY provider device
 * @args: of_phandle_args (not used here)
 *
 * Intended to be used by phy provider for the common case where #phy-cells is
 * 0. For other cases where #phy-cells is greater than '0', the phy provider
 * should provide a custom of_xlate function that reads the *args* and returns
 * the appropriate phy.
 */
struct phy *of_phy_simple_xlate(struct device *dev, struct of_phandle_args
	*args)
{
	struct phy *phy;
	struct class_dev_iter iter;

	class_dev_iter_init(&iter, phy_class, NULL, NULL);
	while ((dev = class_dev_iter_next(&iter))) {
		phy = to_phy(dev);
		if (args->np != phy->dev.of_node)
			continue;

		class_dev_iter_exit(&iter);
		return phy;
	}

	class_dev_iter_exit(&iter);
	return ERR_PTR(-ENODEV);
}
EXPORT_SYMBOL_GPL(of_phy_simple_xlate);

/**
 * phy_get() - lookup and obtain a reference to a phy.
 * @dev: device that requests this phy
 * @string: the phy name as given in the dt data or the name of the controller
 * port for non-dt case
 *
 * Returns the phy driver, after getting a refcount to it; or
 * -ENODEV if there is no such phy.  The caller is responsible for
 * calling phy_put() to release that count.
 */
struct phy *phy_get(struct device *dev, const char *string)
{
	int index = 0;
	struct phy *phy;

	if (string == NULL) {
		dev_WARN(dev, "missing string\n");
		return ERR_PTR(-EINVAL);
	}

	if (dev->of_node) {
		index = of_property_match_string(dev->of_node, "phy-names",
			string);
		phy = _of_phy_get(dev->of_node, index);
	} else {
		phy = phy_find(dev, string);
	}
	if (IS_ERR(phy))
		return phy;

	if (!try_module_get(phy->ops->owner))
		return ERR_PTR(-EPROBE_DEFER);

	get_device(&phy->dev);

	return phy;
}
EXPORT_SYMBOL_GPL(phy_get);

/**
 * phy_optional_get() - lookup and obtain a reference to an optional phy.
 * @dev: device that requests this phy
 * @string: the phy name as given in the dt data or the name of the controller
 * port for non-dt case
 *
 * Returns the phy driver, after getting a refcount to it; or
 * NULL if there is no such phy.  The caller is responsible for
 * calling phy_put() to release that count.
 */
struct phy *phy_optional_get(struct device *dev, const char *string)
{
	struct phy *phy = phy_get(dev, string);

	if (IS_ERR(phy) && (PTR_ERR(phy) == -ENODEV))
		phy = NULL;

	return phy;
}
EXPORT_SYMBOL_GPL(phy_optional_get);

/**
 * devm_phy_get() - lookup and obtain a reference to a phy.
 * @dev: device that requests this phy
 * @string: the phy name as given in the dt data or phy device name
 * for non-dt case
 *
 * Gets the phy using phy_get(), and associates a device with it using
 * devres. On driver detach, release function is invoked on the devres data,
 * then, devres data is freed.
 */
struct phy *devm_phy_get(struct device *dev, const char *string)
{
	struct phy **ptr, *phy;

	ptr = devres_alloc(devm_phy_release, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	phy = phy_get(dev, string);
	if (!IS_ERR(phy)) {
		*ptr = phy;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return phy;
}
EXPORT_SYMBOL_GPL(devm_phy_get);

/**
 * devm_phy_optional_get() - lookup and obtain a reference to an optional phy.
 * @dev: device that requests this phy
 * @string: the phy name as given in the dt data or phy device name
 * for non-dt case
 *
 * Gets the phy using phy_get(), and associates a device with it using
 * devres. On driver detach, release function is invoked on the devres
 * data, then, devres data is freed. This differs to devm_phy_get() in
 * that if the phy does not exist, it is not considered an error and
 * -ENODEV will not be returned. Instead the NULL phy is returned,
 * which can be passed to all other phy consumer calls.
 */
struct phy *devm_phy_optional_get(struct device *dev, const char *string)
{
	struct phy *phy = devm_phy_get(dev, string);

	if (IS_ERR(phy) && (PTR_ERR(phy) == -ENODEV))
		phy = NULL;

	return phy;
}
EXPORT_SYMBOL_GPL(devm_phy_optional_get);

/**
 * devm_of_phy_get() - lookup and obtain a reference to a phy.
 * @dev: device that requests this phy
 * @np: node containing the phy
 * @con_id: name of the phy from device's point of view
 *
 * Gets the phy using of_phy_get(), and associates a device with it using
 * devres. On driver detach, release function is invoked on the devres data,
 * then, devres data is freed.
 */
struct phy *devm_of_phy_get(struct device *dev, struct device_node *np,
			    const char *con_id)
{
	struct phy **ptr, *phy;

	ptr = devres_alloc(devm_phy_release, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	phy = of_phy_get(np, con_id);
	if (!IS_ERR(phy)) {
		*ptr = phy;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return phy;
}
EXPORT_SYMBOL_GPL(devm_of_phy_get);

/**
 * phy_create() - create a new phy
 * @dev: device that is creating the new phy
 * @node: device node of the phy
 * @ops: function pointers for performing phy operations
 *
 * Called to create a phy using phy framework.
 */
struct phy *phy_create(struct device *dev, struct device_node *node,
		       const struct phy_ops *ops)
{
	int ret;
	int id;
	struct phy *phy;

	if (WARN_ON(!dev))
		return ERR_PTR(-EINVAL);

	phy = kzalloc(sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return ERR_PTR(-ENOMEM);

	id = ida_simple_get(&phy_ida, 0, 0, GFP_KERNEL);
	if (id < 0) {
		dev_err(dev, "unable to get id\n");
		ret = id;
		goto free_phy;
	}

	/* phy-supply */
	phy->pwr = regulator_get_optional(dev, "phy");
	if (IS_ERR(phy->pwr)) {
		if (PTR_ERR(phy->pwr) == -EPROBE_DEFER) {
			ret = -EPROBE_DEFER;
			goto free_ida;
		}
		phy->pwr = NULL;
	}

	device_initialize(&phy->dev);
	mutex_init(&phy->mutex);

	phy->dev.class = phy_class;
	phy->dev.parent = dev;
	phy->dev.of_node = node ?: dev->of_node;
	phy->id = id;
	phy->ops = ops;

	ret = dev_set_name(&phy->dev, "phy-%s.%d", dev_name(dev), id);
	if (ret)
		goto put_dev;

	ret = device_add(&phy->dev);
	if (ret)
		goto put_dev;

	if (pm_runtime_enabled(dev)) {
		pm_runtime_enable(&phy->dev);
		pm_runtime_no_callbacks(&phy->dev);
	}

	return phy;

put_dev:
	put_device(&phy->dev);  /* calls phy_release() which frees resources */
	return ERR_PTR(ret);

free_ida:
	ida_simple_remove(&phy_ida, phy->id);

free_phy:
	kfree(phy);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(phy_create);

/**
 * devm_phy_create() - create a new phy
 * @dev: device that is creating the new phy
 * @node: device node of the phy
 * @ops: function pointers for performing phy operations
 *
 * Creates a new PHY device adding it to the PHY class.
 * While at that, it also associates the device with the phy using devres.
 * On driver detach, release function is invoked on the devres data,
 * then, devres data is freed.
 */
struct phy *devm_phy_create(struct device *dev, struct device_node *node,
			    const struct phy_ops *ops)
{
	struct phy **ptr, *phy;

	ptr = devres_alloc(devm_phy_consume, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	phy = phy_create(dev, node, ops);
	if (!IS_ERR(phy)) {
		*ptr = phy;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return phy;
}
EXPORT_SYMBOL_GPL(devm_phy_create);

/**
 * phy_destroy() - destroy the phy
 * @phy: the phy to be destroyed
 *
 * Called to destroy the phy.
 */
void phy_destroy(struct phy *phy)
{
	pm_runtime_disable(&phy->dev);
	device_unregister(&phy->dev);
}
EXPORT_SYMBOL_GPL(phy_destroy);

/**
 * devm_phy_destroy() - destroy the PHY
 * @dev: device that wants to release this phy
 * @phy: the phy returned by devm_phy_get()
 *
 * destroys the devres associated with this phy and invokes phy_destroy
 * to destroy the phy.
 */
void devm_phy_destroy(struct device *dev, struct phy *phy)
{
	int r;

	r = devres_destroy(dev, devm_phy_consume, devm_phy_match, phy);
	dev_WARN_ONCE(dev, r, "couldn't find PHY resource\n");
}
EXPORT_SYMBOL_GPL(devm_phy_destroy);

/**
 * __of_phy_provider_register() - create/register phy provider with the framework
 * @dev: struct device of the phy provider
 * @owner: the module owner containing of_xlate
 * @of_xlate: function pointer to obtain phy instance from phy provider
 *
 * Creates struct phy_provider from dev and of_xlate function pointer.
 * This is used in the case of dt boot for finding the phy instance from
 * phy provider.
 */
struct phy_provider *__of_phy_provider_register(struct device *dev,
	struct module *owner, struct phy * (*of_xlate)(struct device *dev,
	struct of_phandle_args *args))
{
	struct phy_provider *phy_provider;

	phy_provider = kzalloc(sizeof(*phy_provider), GFP_KERNEL);
	if (!phy_provider)
		return ERR_PTR(-ENOMEM);

	phy_provider->dev = dev;
	phy_provider->owner = owner;
	phy_provider->of_xlate = of_xlate;

	mutex_lock(&phy_provider_mutex);
	list_add_tail(&phy_provider->list, &phy_provider_list);
	mutex_unlock(&phy_provider_mutex);

	return phy_provider;
}
EXPORT_SYMBOL_GPL(__of_phy_provider_register);

/**
 * __devm_of_phy_provider_register() - create/register phy provider with the
 * framework
 * @dev: struct device of the phy provider
 * @owner: the module owner containing of_xlate
 * @of_xlate: function pointer to obtain phy instance from phy provider
 *
 * Creates struct phy_provider from dev and of_xlate function pointer.
 * This is used in the case of dt boot for finding the phy instance from
 * phy provider. While at that, it also associates the device with the
 * phy provider using devres. On driver detach, release function is invoked
 * on the devres data, then, devres data is freed.
 */
struct phy_provider *__devm_of_phy_provider_register(struct device *dev,
	struct module *owner, struct phy * (*of_xlate)(struct device *dev,
	struct of_phandle_args *args))
{
	struct phy_provider **ptr, *phy_provider;

	ptr = devres_alloc(devm_phy_provider_release, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	phy_provider = __of_phy_provider_register(dev, owner, of_xlate);
	if (!IS_ERR(phy_provider)) {
		*ptr = phy_provider;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return phy_provider;
}
EXPORT_SYMBOL_GPL(__devm_of_phy_provider_register);

/**
 * of_phy_provider_unregister() - unregister phy provider from the framework
 * @phy_provider: phy provider returned by of_phy_provider_register()
 *
 * Removes the phy_provider created using of_phy_provider_register().
 */
void of_phy_provider_unregister(struct phy_provider *phy_provider)
{
	if (IS_ERR(phy_provider))
		return;

	mutex_lock(&phy_provider_mutex);
	list_del(&phy_provider->list);
	kfree(phy_provider);
	mutex_unlock(&phy_provider_mutex);
}
EXPORT_SYMBOL_GPL(of_phy_provider_unregister);

/**
 * devm_of_phy_provider_unregister() - remove phy provider from the framework
 * @dev: struct device of the phy provider
 *
 * destroys the devres associated with this phy provider and invokes
 * of_phy_provider_unregister to unregister the phy provider.
 */
void devm_of_phy_provider_unregister(struct device *dev,
	struct phy_provider *phy_provider) {
	int r;

	r = devres_destroy(dev, devm_phy_provider_release, devm_phy_match,
		phy_provider);
	dev_WARN_ONCE(dev, r, "couldn't find PHY provider device resource\n");
}
EXPORT_SYMBOL_GPL(devm_of_phy_provider_unregister);

/**
 * phy_release() - release the phy
 * @dev: the dev member within phy
 *
 * When the last reference to the device is removed, it is called
 * from the embedded kobject as release method.
 */
static void phy_release(struct device *dev)
{
	struct phy *phy;

	phy = to_phy(dev);
	dev_vdbg(dev, "releasing '%s'\n", dev_name(dev));
	regulator_put(phy->pwr);
	ida_simple_remove(&phy_ida, phy->id);
	kfree(phy);
}

static int __init phy_core_init(void)
{
	phy_class = class_create(THIS_MODULE, "phy");
	if (IS_ERR(phy_class)) {
		pr_err("failed to create phy class --> %ld\n",
			PTR_ERR(phy_class));
		return PTR_ERR(phy_class);
	}

	phy_class->dev_release = phy_release;

	return 0;
}
module_init(phy_core_init);

static void __exit phy_core_exit(void)
{
	class_destroy(phy_class);
}
module_exit(phy_core_exit);

MODULE_DESCRIPTION("Generic PHY Framework");
MODULE_AUTHOR("Kishon Vijay Abraham I <kishon@ti.com>");
MODULE_LICENSE("GPL v2");
