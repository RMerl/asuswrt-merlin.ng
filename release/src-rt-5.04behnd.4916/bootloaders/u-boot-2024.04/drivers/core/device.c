// SPDX-License-Identifier: GPL-2.0+
/*
 * Device manager
 *
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <event.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <clk.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <malloc.h>
#include <asm/cache.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <dm/pinctrl.h>
#include <dm/platdata.h>
#include <dm/read.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <iommu.h>
#include <linux/err.h>
#include <linux/list.h>
#include <power-domain.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

static int device_bind_common(struct udevice *parent, const struct driver *drv,
			      const char *name, void *plat,
			      ulong driver_data, ofnode node,
			      uint of_plat_size, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int size, ret = 0;
	bool auto_seq = true;
	void *ptr;

	if (CONFIG_IS_ENABLED(OF_PLATDATA_NO_BIND))
		return -ENOSYS;

	if (devp)
		*devp = NULL;
	if (!name)
		return -EINVAL;

	ret = uclass_get(drv->id, &uc);
	if (ret) {
		debug("Missing uclass for driver %s\n", drv->name);
		return ret;
	}

	dev = calloc(1, sizeof(struct udevice));
	if (!dev)
		return -ENOMEM;

	INIT_LIST_HEAD(&dev->sibling_node);
	INIT_LIST_HEAD(&dev->child_head);
	INIT_LIST_HEAD(&dev->uclass_node);
#if CONFIG_IS_ENABLED(DEVRES)
	INIT_LIST_HEAD(&dev->devres_head);
#endif
	dev_set_plat(dev, plat);
	dev->driver_data = driver_data;
	dev->name = name;
	dev_set_ofnode(dev, node);
	dev->parent = parent;
	dev->driver = drv;
	dev->uclass = uc;

	dev->seq_ = -1;
	if (CONFIG_IS_ENABLED(DM_SEQ_ALIAS) &&
	    (uc->uc_drv->flags & DM_UC_FLAG_SEQ_ALIAS)) {
		/*
		 * Some devices, such as a SPI bus, I2C bus and serial ports
		 * are numbered using aliases.
		 */
		if (CONFIG_IS_ENABLED(OF_CONTROL) &&
		    !CONFIG_IS_ENABLED(OF_PLATDATA)) {
			if (uc->uc_drv->name && ofnode_valid(node)) {
				if (!dev_read_alias_seq(dev, &dev->seq_)) {
					auto_seq = false;
					log_debug("   - seq=%d\n", dev->seq_);
					}
			}
		}
	}
	if (auto_seq && !(uc->uc_drv->flags & DM_UC_FLAG_NO_AUTO_SEQ))
		dev->seq_ = uclass_find_next_free_seq(uc);

	/* Check if we need to allocate plat */
	if (drv->plat_auto) {
		bool alloc = !plat;

		/*
		 * For of-platdata, we try use the existing data, but if
		 * plat_auto is larger, we must allocate a new space
		 */
		if (CONFIG_IS_ENABLED(OF_PLATDATA)) {
			if (of_plat_size)
				dev_or_flags(dev, DM_FLAG_OF_PLATDATA);
			if (of_plat_size < drv->plat_auto)
				alloc = true;
		}
		if (alloc) {
			dev_or_flags(dev, DM_FLAG_ALLOC_PDATA);
			ptr = calloc(1, drv->plat_auto);
			if (!ptr) {
				ret = -ENOMEM;
				goto fail_alloc1;
			}

			/*
			 * For of-platdata, copy the old plat into the new
			 * space
			 */
			if (CONFIG_IS_ENABLED(OF_PLATDATA) && plat)
				memcpy(ptr, plat, of_plat_size);
			dev_set_plat(dev, ptr);
		}
	}

	size = uc->uc_drv->per_device_plat_auto;
	if (size) {
		dev_or_flags(dev, DM_FLAG_ALLOC_UCLASS_PDATA);
		ptr = calloc(1, size);
		if (!ptr) {
			ret = -ENOMEM;
			goto fail_alloc2;
		}
		dev_set_uclass_plat(dev, ptr);
	}

	if (parent) {
		size = parent->driver->per_child_plat_auto;
		if (!size)
			size = parent->uclass->uc_drv->per_child_plat_auto;
		if (size) {
			dev_or_flags(dev, DM_FLAG_ALLOC_PARENT_PDATA);
			ptr = calloc(1, size);
			if (!ptr) {
				ret = -ENOMEM;
				goto fail_alloc3;
			}
			dev_set_parent_plat(dev, ptr);
		}
		/* put dev into parent's successor list */
		list_add_tail(&dev->sibling_node, &parent->child_head);
	}

	ret = uclass_bind_device(dev);
	if (ret)
		goto fail_uclass_bind;

	/* if we fail to bind we remove device from successors and free it */
	if (drv->bind) {
		ret = drv->bind(dev);
		if (ret)
			goto fail_bind;
	}
	if (parent && parent->driver->child_post_bind) {
		ret = parent->driver->child_post_bind(dev);
		if (ret)
			goto fail_child_post_bind;
	}
	if (uc->uc_drv->post_bind) {
		ret = uc->uc_drv->post_bind(dev);
		if (ret)
			goto fail_uclass_post_bind;
	}

	if (parent)
		pr_debug("Bound device %s to %s\n", dev->name, parent->name);
	if (devp)
		*devp = dev;

	dev_or_flags(dev, DM_FLAG_BOUND);

	return 0;

fail_uclass_post_bind:
	/* There is no child unbind() method, so no clean-up required */
fail_child_post_bind:
	if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
		if (drv->unbind && drv->unbind(dev)) {
			dm_warn("unbind() method failed on dev '%s' on error path\n",
				dev->name);
		}
	}

fail_bind:
	if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
		if (uclass_unbind_device(dev)) {
			dm_warn("Failed to unbind dev '%s' on error path\n",
				dev->name);
		}
	}
fail_uclass_bind:
	if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
		list_del(&dev->sibling_node);
		if (dev_get_flags(dev) & DM_FLAG_ALLOC_PARENT_PDATA) {
			free(dev_get_parent_plat(dev));
			dev_set_parent_plat(dev, NULL);
		}
	}
fail_alloc3:
	if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
		if (dev_get_flags(dev) & DM_FLAG_ALLOC_UCLASS_PDATA) {
			free(dev_get_uclass_plat(dev));
			dev_set_uclass_plat(dev, NULL);
		}
	}
fail_alloc2:
	if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
		if (dev_get_flags(dev) & DM_FLAG_ALLOC_PDATA) {
			free(dev_get_plat(dev));
			dev_set_plat(dev, NULL);
		}
	}
fail_alloc1:
	devres_release_all(dev);

	free(dev);

	return ret;
}

int device_bind_with_driver_data(struct udevice *parent,
				 const struct driver *drv, const char *name,
				 ulong driver_data, ofnode node,
				 struct udevice **devp)
{
	return device_bind_common(parent, drv, name, NULL, driver_data, node,
				  0, devp);
}

int device_bind(struct udevice *parent, const struct driver *drv,
		const char *name, void *plat, ofnode node,
		struct udevice **devp)
{
	return device_bind_common(parent, drv, name, plat, 0, node, 0,
				  devp);
}

int device_bind_by_name(struct udevice *parent, bool pre_reloc_only,
			const struct driver_info *info, struct udevice **devp)
{
	struct driver *drv;
	uint plat_size = 0;
	int ret;

	drv = lists_driver_lookup_name(info->name);
	if (!drv)
		return -ENOENT;
	if (pre_reloc_only && !(drv->flags & DM_FLAG_PRE_RELOC))
		return -EPERM;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	plat_size = info->plat_size;
#endif
	ret = device_bind_common(parent, drv, info->name, (void *)info->plat, 0,
				 ofnode_null(), plat_size, devp);
	if (ret)
		return ret;

	return ret;
}

int device_reparent(struct udevice *dev, struct udevice *new_parent)
{
	struct udevice *pos, *n;

	assert(dev);
	assert(new_parent);

	device_foreach_child_safe(pos, n, dev->parent) {
		if (pos->driver != dev->driver)
			continue;

		list_del(&dev->sibling_node);
		list_add_tail(&dev->sibling_node, &new_parent->child_head);
		dev->parent = new_parent;

		break;
	}

	return 0;
}

static void *alloc_priv(int size, uint flags)
{
	void *priv;

	if (flags & DM_FLAG_ALLOC_PRIV_DMA) {
		size = ROUND(size, ARCH_DMA_MINALIGN);
		priv = memalign(ARCH_DMA_MINALIGN, size);
		if (priv) {
			memset(priv, '\0', size);

			/*
			 * Ensure that the zero bytes are flushed to memory.
			 * This prevents problems if the driver uses this as
			 * both an input and an output buffer:
			 *
			 * 1. Zeroes written to buffer (here) and sit in the
			 *	cache
			 * 2. Driver issues a read command to DMA
			 * 3. CPU runs out of cache space and evicts some cache
			 *	data in the buffer, writing zeroes to RAM from
			 *	the memset() above
			 * 4. DMA completes
			 * 5. Buffer now has some DMA data and some zeroes
			 * 6. Data being read is now incorrect
			 *
			 * To prevent this, ensure that the cache is clean
			 * within this range at the start. The driver can then
			 * use normal flush-after-write, invalidate-before-read
			 * procedures.
			 */
			flush_dcache_range((ulong)priv, (ulong)priv + size);
		}
	} else {
		priv = calloc(1, size);
	}

	return priv;
}

/**
 * device_alloc_priv() - Allocate priv/plat data required by the device
 *
 * @dev: Device to process
 * Return: 0 if OK, -ENOMEM if out of memory
 */
static int device_alloc_priv(struct udevice *dev)
{
	const struct driver *drv;
	void *ptr;
	int size;

	drv = dev->driver;
	assert(drv);

	/* Allocate private data if requested and not reentered */
	if (drv->priv_auto && !dev_get_priv(dev)) {
		ptr = alloc_priv(drv->priv_auto, drv->flags);
		if (!ptr)
			return -ENOMEM;
		dev_set_priv(dev, ptr);
	}

	/* Allocate private data if requested and not reentered */
	size = dev->uclass->uc_drv->per_device_auto;
	if (size && !dev_get_uclass_priv(dev)) {
		ptr = alloc_priv(size, dev->uclass->uc_drv->flags);
		if (!ptr)
			return -ENOMEM;
		dev_set_uclass_priv(dev, ptr);
	}

	/* Allocate parent data for this child */
	if (dev->parent) {
		size = dev->parent->driver->per_child_auto;
		if (!size)
			size = dev->parent->uclass->uc_drv->per_child_auto;
		if (size && !dev_get_parent_priv(dev)) {
			ptr = alloc_priv(size, drv->flags);
			if (!ptr)
				return -ENOMEM;
			dev_set_parent_priv(dev, ptr);
		}
	}

	return 0;
}

int device_of_to_plat(struct udevice *dev)
{
	const struct driver *drv;
	int ret;

	if (!dev)
		return -EINVAL;

	if (dev_get_flags(dev) & DM_FLAG_PLATDATA_VALID)
		return 0;

	/*
	 * This is not needed if binding is disabled, since data is allocated
	 * at build time.
	 */
	if (!CONFIG_IS_ENABLED(OF_PLATDATA_NO_BIND)) {
		/* Ensure all parents have ofdata */
		if (dev->parent) {
			ret = device_of_to_plat(dev->parent);
			if (ret)
				goto fail;

			/*
			 * The device might have already been probed during
			 * the call to device_probe() on its parent device
			 * (e.g. PCI bridge devices). Test the flags again
			 * so that we don't mess up the device.
			 */
			if (dev_get_flags(dev) & DM_FLAG_PLATDATA_VALID)
				return 0;
		}

		ret = device_alloc_priv(dev);
		if (ret)
			goto fail;
	}
	drv = dev->driver;
	assert(drv);

	if (drv->of_to_plat &&
	    (CONFIG_IS_ENABLED(OF_PLATDATA) || dev_has_ofnode(dev))) {
		ret = drv->of_to_plat(dev);
		if (ret)
			goto fail;
	}

	dev_or_flags(dev, DM_FLAG_PLATDATA_VALID);

	return 0;
fail:
	device_free(dev);

	return ret;
}

/**
 * device_get_dma_constraints() - Populate device's DMA constraints
 *
 * Gets a device's DMA constraints from firmware. This information is later
 * used by drivers to translate physcal addresses to the device's bus address
 * space. For now only device-tree is supported.
 *
 * @dev: Pointer to target device
 * Return: 0 if OK or if no DMA constraints were found, error otherwise
 */
static int device_get_dma_constraints(struct udevice *dev)
{
	struct udevice *parent = dev->parent;
	phys_addr_t cpu = 0;
	dma_addr_t bus = 0;
	u64 size = 0;
	int ret;

	if (!CONFIG_IS_ENABLED(DM_DMA) || !parent || !dev_has_ofnode(parent))
		return 0;

	/*
	 * We start parsing for dma-ranges from the device's bus node. This is
	 * specially important on nested buses.
	 */
	ret = dev_get_dma_range(parent, &cpu, &bus, &size);
	/* Don't return an error if no 'dma-ranges' were found */
	if (ret && ret != -ENOENT) {
		dm_warn("%s: failed to get DMA range, %d\n", dev->name, ret);
		return ret;
	}

	dev_set_dma_offset(dev, cpu - bus);

	return 0;
}

int device_probe(struct udevice *dev)
{
	const struct driver *drv;
	int ret;

	if (!dev)
		return -EINVAL;

	if (dev_get_flags(dev) & DM_FLAG_ACTIVATED)
		return 0;

	ret = device_notify(dev, EVT_DM_PRE_PROBE);
	if (ret)
		return ret;

	drv = dev->driver;
	assert(drv);

	ret = device_of_to_plat(dev);
	if (ret)
		goto fail;

	/* Ensure all parents are probed */
	if (dev->parent) {
		ret = device_probe(dev->parent);
		if (ret)
			goto fail;

		/*
		 * The device might have already been probed during
		 * the call to device_probe() on its parent device
		 * (e.g. PCI bridge devices). Test the flags again
		 * so that we don't mess up the device.
		 */
		if (dev_get_flags(dev) & DM_FLAG_ACTIVATED)
			return 0;
	}

	dev_or_flags(dev, DM_FLAG_ACTIVATED);

	if (CONFIG_IS_ENABLED(POWER_DOMAIN) && dev->parent &&
	    (device_get_uclass_id(dev) != UCLASS_POWER_DOMAIN) &&
	    !(drv->flags & DM_FLAG_DEFAULT_PD_CTRL_OFF)) {
		ret = dev_power_domain_on(dev);
		if (ret)
			goto fail;
	}

	/*
	 * Process pinctrl for everything except the root device, and
	 * continue regardless of the result of pinctrl. Don't process pinctrl
	 * settings for pinctrl devices since the device may not yet be
	 * probed.
	 *
	 * This call can produce some non-intuitive results. For example, on an
	 * x86 device where dev is the main PCI bus, the pinctrl device may be
	 * child or grandchild of that bus, meaning that the child will be
	 * probed here. If the child happens to be the P2SB and the pinctrl
	 * device is a child of that, then both the pinctrl and P2SB will be
	 * probed by this call. This works because the DM_FLAG_ACTIVATED flag
	 * is set just above. However, the PCI bus' probe() method and
	 * associated uclass methods have not yet been called.
	 */
	if (dev->parent && device_get_uclass_id(dev) != UCLASS_PINCTRL) {
		ret = pinctrl_select_state(dev, "default");
		if (ret && ret != -ENOSYS)
			log_debug("Device '%s' failed to configure default pinctrl: %d (%s)\n",
				  dev->name, ret, errno_str(ret));
	}

	if (CONFIG_IS_ENABLED(IOMMU) && dev->parent &&
	    (device_get_uclass_id(dev) != UCLASS_IOMMU)) {
		ret = dev_iommu_enable(dev);
		if (ret)
			goto fail;
	}

	ret = device_get_dma_constraints(dev);
	if (ret)
		goto fail;

	ret = uclass_pre_probe_device(dev);
	if (ret)
		goto fail;

	if (dev->parent && dev->parent->driver->child_pre_probe) {
		ret = dev->parent->driver->child_pre_probe(dev);
		if (ret)
			goto fail;
	}

	/* Only handle devices that have a valid ofnode */
	if (dev_has_ofnode(dev)) {
		/*
		 * Process 'assigned-{clocks/clock-parents/clock-rates}'
		 * properties
		 */
		ret = clk_set_defaults(dev, CLK_DEFAULTS_PRE);
		if (ret)
			goto fail;
	}

	if (drv->probe) {
		ret = drv->probe(dev);
		if (ret)
			goto fail;
	}

	ret = uclass_post_probe_device(dev);
	if (ret)
		goto fail_uclass;

	if (dev->parent && device_get_uclass_id(dev) == UCLASS_PINCTRL) {
		ret = pinctrl_select_state(dev, "default");
		if (ret && ret != -ENOSYS)
			log_debug("Device '%s' failed to configure default pinctrl: %d (%s)\n",
				  dev->name, ret, errno_str(ret));
	}

	ret = device_notify(dev, EVT_DM_POST_PROBE);
	if (ret)
		goto fail_event;

	return 0;
fail_event:
fail_uclass:
	if (device_remove(dev, DM_REMOVE_NORMAL)) {
		dm_warn("%s: Device '%s' failed to remove on error path\n",
			__func__, dev->name);
	}
fail:
	dev_bic_flags(dev, DM_FLAG_ACTIVATED);

	device_free(dev);

	return ret;
}

void *dev_get_plat(const struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dm_priv_to_rw(dev->plat_);
}

void *dev_get_parent_plat(const struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dm_priv_to_rw(dev->parent_plat_);
}

void *dev_get_uclass_plat(const struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dm_priv_to_rw(dev->uclass_plat_);
}

void *dev_get_priv(const struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dm_priv_to_rw(dev->priv_);
}

/* notrace is needed as this is called by timer_get_rate() */
notrace void *dev_get_uclass_priv(const struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dm_priv_to_rw(dev->uclass_priv_);
}

void *dev_get_parent_priv(const struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dm_priv_to_rw(dev->parent_priv_);
}

void *dev_get_attach_ptr(const struct udevice *dev, enum dm_tag_t tag)
{
	switch (tag) {
	case DM_TAG_PLAT:
		return dev_get_plat(dev);
	case DM_TAG_PARENT_PLAT:
		return dev_get_parent_plat(dev);
	case DM_TAG_UC_PLAT:
		return dev_get_uclass_plat(dev);
	case DM_TAG_PRIV:
		return dev_get_priv(dev);
	case DM_TAG_PARENT_PRIV:
		return dev_get_parent_priv(dev);
	case DM_TAG_UC_PRIV:
		return dev_get_uclass_priv(dev);
	default:
		return NULL;
	}
}

int dev_get_attach_size(const struct udevice *dev, enum dm_tag_t tag)
{
	const struct udevice *parent = dev_get_parent(dev);
	const struct uclass *uc = dev->uclass;
	const struct uclass_driver *uc_drv = uc->uc_drv;
	const struct driver *parent_drv = NULL;
	int size = 0;

	if (parent)
		parent_drv = parent->driver;

	switch (tag) {
	case DM_TAG_PLAT:
		size = dev->driver->plat_auto;
		break;
	case DM_TAG_PARENT_PLAT:
		if (parent) {
			size = parent_drv->per_child_plat_auto;
			if (!size)
				size = parent->uclass->uc_drv->per_child_plat_auto;
		}
		break;
	case DM_TAG_UC_PLAT:
		size = uc_drv->per_device_plat_auto;
		break;
	case DM_TAG_PRIV:
		size = dev->driver->priv_auto;
		break;
	case DM_TAG_PARENT_PRIV:
		if (parent) {
			size = parent_drv->per_child_auto;
			if (!size)
				size = parent->uclass->uc_drv->per_child_auto;
		}
		break;
	case DM_TAG_UC_PRIV:
		size = uc_drv->per_device_auto;
		break;
	default:
		break;
	}

	return size;
}

static int device_get_device_tail(struct udevice *dev, int ret,
				  struct udevice **devp)
{
	if (ret)
		return ret;

	ret = device_probe(dev);
	if (ret)
		return ret;

	*devp = dev;

	return 0;
}

#if CONFIG_IS_ENABLED(OF_REAL)
/**
 * device_find_by_ofnode() - Return device associated with given ofnode
 *
 * The returned device is *not* activated.
 *
 * @node: The ofnode for which a associated device should be looked up
 * @devp: Pointer to structure to hold the found device
 * Return: 0 if OK, -ve on error
 */
static int device_find_by_ofnode(ofnode node, struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	list_for_each_entry(uc, gd->uclass_root, sibling_node) {
		ret = uclass_find_device_by_ofnode(uc->uc_drv->id, node,
						   &dev);
		if (!ret || dev) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}
#endif

int device_get_child(const struct udevice *parent, int index,
		     struct udevice **devp)
{
	struct udevice *dev;

	device_foreach_child(dev, parent) {
		if (!index--)
			return device_get_device_tail(dev, 0, devp);
	}

	return -ENODEV;
}

int device_get_child_count(const struct udevice *parent)
{
	struct udevice *dev;
	int count = 0;

	device_foreach_child(dev, parent)
		count++;

	return count;
}

int device_get_decendent_count(const struct udevice *parent)
{
	const struct udevice *dev;
	int count = 1;

	device_foreach_child(dev, parent)
		count += device_get_decendent_count(dev);

	return count;
}

int device_find_child_by_seq(const struct udevice *parent, int seq,
			     struct udevice **devp)
{
	struct udevice *dev;

	*devp = NULL;

	device_foreach_child(dev, parent) {
		if (dev->seq_ == seq) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int device_get_child_by_seq(const struct udevice *parent, int seq,
			    struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = device_find_child_by_seq(parent, seq, &dev);

	return device_get_device_tail(dev, ret, devp);
}

int device_find_child_by_of_offset(const struct udevice *parent, int of_offset,
				   struct udevice **devp)
{
	struct udevice *dev;

	*devp = NULL;

	device_foreach_child(dev, parent) {
		if (dev_of_offset(dev) == of_offset) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int device_get_child_by_of_offset(const struct udevice *parent, int node,
				  struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = device_find_child_by_of_offset(parent, node, &dev);
	return device_get_device_tail(dev, ret, devp);
}

static struct udevice *_device_find_global_by_ofnode(struct udevice *parent,
						     ofnode ofnode)
{
	struct udevice *dev, *found;

	if (ofnode_equal(dev_ofnode(parent), ofnode))
		return parent;

	device_foreach_child(dev, parent) {
		found = _device_find_global_by_ofnode(dev, ofnode);
		if (found)
			return found;
	}

	return NULL;
}

int device_find_global_by_ofnode(ofnode ofnode, struct udevice **devp)
{
	*devp = _device_find_global_by_ofnode(gd->dm_root, ofnode);

	return *devp ? 0 : -ENOENT;
}

int device_get_global_by_ofnode(ofnode ofnode, struct udevice **devp)
{
	struct udevice *dev;

	dev = _device_find_global_by_ofnode(gd->dm_root, ofnode);
	return device_get_device_tail(dev, dev ? 0 : -ENOENT, devp);
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
int device_get_by_ofplat_idx(uint idx, struct udevice **devp)
{
	struct udevice *dev;

	if (CONFIG_IS_ENABLED(OF_PLATDATA_INST)) {
		struct udevice *base = ll_entry_start(struct udevice, udevice);

		dev = base + idx;
	} else {
		struct driver_rt *drt = gd_dm_driver_rt() + idx;

		dev = drt->dev;
	}
	*devp = NULL;

	return device_get_device_tail(dev, dev ? 0 : -ENOENT, devp);
}
#endif

int device_find_first_child(const struct udevice *parent, struct udevice **devp)
{
	if (list_empty(&parent->child_head)) {
		*devp = NULL;
	} else {
		*devp = list_first_entry(&parent->child_head, struct udevice,
					 sibling_node);
	}

	return 0;
}

int device_find_next_child(struct udevice **devp)
{
	struct udevice *dev = *devp;
	struct udevice *parent = dev->parent;

	if (list_is_last(&dev->sibling_node, &parent->child_head)) {
		*devp = NULL;
	} else {
		*devp = list_entry(dev->sibling_node.next, struct udevice,
				   sibling_node);
	}

	return 0;
}

int device_find_first_inactive_child(const struct udevice *parent,
				     enum uclass_id uclass_id,
				     struct udevice **devp)
{
	struct udevice *dev;

	*devp = NULL;
	device_foreach_child(dev, parent) {
		if (!device_active(dev) &&
		    device_get_uclass_id(dev) == uclass_id) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int device_find_first_child_by_uclass(const struct udevice *parent,
				      enum uclass_id uclass_id,
				      struct udevice **devp)
{
	struct udevice *dev;

	*devp = NULL;
	device_foreach_child(dev, parent) {
		if (device_get_uclass_id(dev) == uclass_id) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int device_find_child_by_namelen(const struct udevice *parent, const char *name,
				 int len, struct udevice **devp)
{
	struct udevice *dev;

	*devp = NULL;

	device_foreach_child(dev, parent) {
		if (!strncmp(dev->name, name, len) &&
		    strlen(dev->name) == len) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int device_find_child_by_name(const struct udevice *parent, const char *name,
			      struct udevice **devp)
{
	return device_find_child_by_namelen(parent, name, strlen(name), devp);
}

int device_first_child_err(struct udevice *parent, struct udevice **devp)
{
	struct udevice *dev;

	device_find_first_child(parent, &dev);
	if (!dev)
		return -ENODEV;

	return device_get_device_tail(dev, 0, devp);
}

int device_next_child_err(struct udevice **devp)
{
	struct udevice *dev = *devp;

	device_find_next_child(&dev);
	if (!dev)
		return -ENODEV;

	return device_get_device_tail(dev, 0, devp);
}

int device_first_child_ofdata_err(struct udevice *parent, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	device_find_first_child(parent, &dev);
	if (!dev)
		return -ENODEV;

	ret = device_of_to_plat(dev);
	if (ret)
		return ret;

	*devp = dev;

	return 0;
}

int device_next_child_ofdata_err(struct udevice **devp)
{
	struct udevice *dev = *devp;
	int ret;

	device_find_next_child(&dev);
	if (!dev)
		return -ENODEV;

	ret = device_of_to_plat(dev);
	if (ret)
		return ret;

	*devp = dev;

	return 0;
}

struct udevice *dev_get_parent(const struct udevice *child)
{
	return child->parent;
}

ulong dev_get_driver_data(const struct udevice *dev)
{
	return dev->driver_data;
}

const void *dev_get_driver_ops(const struct udevice *dev)
{
	if (!dev || !dev->driver->ops)
		return NULL;

	return dev->driver->ops;
}

enum uclass_id device_get_uclass_id(const struct udevice *dev)
{
	return dev->uclass->uc_drv->id;
}

const char *dev_get_uclass_name(const struct udevice *dev)
{
	if (!dev)
		return NULL;

	return dev->uclass->uc_drv->name;
}

bool device_has_children(const struct udevice *dev)
{
	return !list_empty(&dev->child_head);
}

bool device_has_active_children(const struct udevice *dev)
{
	struct udevice *child;

	for (device_find_first_child(dev, &child);
	     child;
	     device_find_next_child(&child)) {
		if (device_active(child))
			return true;
	}

	return false;
}

bool device_is_last_sibling(const struct udevice *dev)
{
	struct udevice *parent = dev->parent;

	if (!parent)
		return false;
	return list_is_last(&dev->sibling_node, &parent->child_head);
}

void device_set_name_alloced(struct udevice *dev)
{
	dev_or_flags(dev, DM_FLAG_NAME_ALLOCED);
}

int device_set_name(struct udevice *dev, const char *name)
{
	name = strdup(name);
	if (!name)
		return -ENOMEM;
	dev->name = name;
	device_set_name_alloced(dev);

	return 0;
}

void dev_set_priv(struct udevice *dev, void *priv)
{
	dev->priv_ = priv;
}

void dev_set_parent_priv(struct udevice *dev, void *parent_priv)
{
	dev->parent_priv_ = parent_priv;
}

void dev_set_uclass_priv(struct udevice *dev, void *uclass_priv)
{
	dev->uclass_priv_ = uclass_priv;
}

void dev_set_plat(struct udevice *dev, void *plat)
{
	dev->plat_ = plat;
}

void dev_set_parent_plat(struct udevice *dev, void *parent_plat)
{
	dev->parent_plat_ = parent_plat;
}

void dev_set_uclass_plat(struct udevice *dev, void *uclass_plat)
{
	dev->uclass_plat_ = uclass_plat;
}

#if CONFIG_IS_ENABLED(OF_REAL)
bool device_is_compatible(const struct udevice *dev, const char *compat)
{
	return ofnode_device_is_compatible(dev_ofnode(dev), compat);
}

bool of_machine_is_compatible(const char *compat)
{
	return ofnode_device_is_compatible(ofnode_root(), compat);
}

int dev_disable_by_path(const char *path)
{
	struct uclass *uc;
	ofnode node = ofnode_path(path);
	struct udevice *dev;
	int ret = 1;

	if (!of_live_active())
		return -ENOSYS;

	list_for_each_entry(uc, gd->uclass_root, sibling_node) {
		ret = uclass_find_device_by_ofnode(uc->uc_drv->id, node, &dev);
		if (!ret)
			break;
	}

	if (ret)
		return ret;

	ret = device_remove(dev, DM_REMOVE_NORMAL);
	if (ret)
		return ret;

	ret = device_unbind(dev);
	if (ret)
		return ret;

	return ofnode_set_enabled(node, false);
}

int dev_enable_by_path(const char *path)
{
	ofnode node = ofnode_path(path);
	ofnode pnode = ofnode_get_parent(node);
	struct udevice *parent;
	int ret = 1;

	if (!of_live_active())
		return -ENOSYS;

	ret = device_find_by_ofnode(pnode, &parent);
	if (ret)
		return ret;

	ret = ofnode_set_enabled(node, true);
	if (ret)
		return ret;

	return lists_bind_fdt(parent, node, NULL, NULL, false);
}
#endif

#if CONFIG_IS_ENABLED(OF_PLATDATA_RT)
static struct udevice_rt *dev_get_rt(const struct udevice *dev)
{
	struct udevice *base = ll_entry_start(struct udevice, udevice);
	uint each_size = dm_udevice_size();
	int idx = ((void *)dev - (void *)base) / each_size;

	struct udevice_rt *urt = gd_dm_udevice_rt() + idx;

	return urt;
}

u32 dev_get_flags(const struct udevice *dev)
{
	const struct udevice_rt *urt = dev_get_rt(dev);

	return urt->flags_;
}

void dev_or_flags(const struct udevice *dev, u32 or)
{
	struct udevice_rt *urt = dev_get_rt(dev);

	urt->flags_ |= or;
}

void dev_bic_flags(const struct udevice *dev, u32 bic)
{
	struct udevice_rt *urt = dev_get_rt(dev);

	urt->flags_ &= ~bic;
}
#endif /* OF_PLATDATA_RT */
