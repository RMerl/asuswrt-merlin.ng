// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#define LOG_CATEGORY LOGC_DM

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

struct uclass *uclass_find(enum uclass_id key)
{
	struct uclass *uc;

	if (!gd->dm_root)
		return NULL;
	/*
	 * TODO(sjg@chromium.org): Optimise this, perhaps moving the found
	 * node to the start of the list, or creating a linear array mapping
	 * id to node.
	 */
	list_for_each_entry(uc, gd->uclass_root, sibling_node) {
		if (uc->uc_drv->id == key)
			return uc;
	}

	return NULL;
}

/**
 * uclass_add() - Create new uclass in list
 * @id: Id number to create
 * @ucp: Returns pointer to uclass, or NULL on error
 * Return: 0 on success, -ve on error
 *
 * The new uclass is added to the list. There must be only one uclass for
 * each id.
 */
static int uclass_add(enum uclass_id id, struct uclass **ucp)
{
	struct uclass_driver *uc_drv;
	struct uclass *uc;
	int ret;

	*ucp = NULL;
	uc_drv = lists_uclass_lookup(id);
	if (!uc_drv) {
		debug("Cannot find uclass for id %d: please add the UCLASS_DRIVER() declaration for this UCLASS_... id\n",
		      id);
		/*
		 * Use a strange error to make this case easier to find. When
		 * a uclass is not available it can prevent driver model from
		 * starting up and this failure is otherwise hard to debug.
		 */
		return -EPFNOSUPPORT;
	}
	uc = calloc(1, sizeof(*uc));
	if (!uc)
		return -ENOMEM;
	if (uc_drv->priv_auto) {
		void *ptr;

		ptr = calloc(1, uc_drv->priv_auto);
		if (!ptr) {
			ret = -ENOMEM;
			goto fail_mem;
		}
		uclass_set_priv(uc, ptr);
	}
	uc->uc_drv = uc_drv;
	INIT_LIST_HEAD(&uc->sibling_node);
	INIT_LIST_HEAD(&uc->dev_head);
	list_add(&uc->sibling_node, DM_UCLASS_ROOT_NON_CONST);

	if (uc_drv->init) {
		ret = uc_drv->init(uc);
		if (ret)
			goto fail;
	}

	*ucp = uc;

	return 0;
fail:
	if (uc_drv->priv_auto) {
		free(uclass_get_priv(uc));
		uclass_set_priv(uc, NULL);
	}
	list_del(&uc->sibling_node);
fail_mem:
	free(uc);

	return ret;
}

int uclass_destroy(struct uclass *uc)
{
	struct uclass_driver *uc_drv;
	struct udevice *dev;
	int ret;

	/*
	 * We cannot use list_for_each_entry_safe() here. If a device in this
	 * uclass has a child device also in this uclass, it will be also be
	 * unbound (by the recursion in the call to device_unbind() below).
	 * We can loop until the list is empty.
	 */
	while (!list_empty(&uc->dev_head)) {
		dev = list_first_entry(&uc->dev_head, struct udevice,
				       uclass_node);
		ret = device_remove(dev, DM_REMOVE_NORMAL | DM_REMOVE_NO_PD);
		if (ret)
			return log_msg_ret("remove", ret);
		ret = device_unbind(dev);
		if (ret)
			return log_msg_ret("unbind", ret);
	}

	uc_drv = uc->uc_drv;
	if (uc_drv->destroy)
		uc_drv->destroy(uc);
	list_del(&uc->sibling_node);
	if (uc_drv->priv_auto)
		free(uclass_get_priv(uc));
	free(uc);

	return 0;
}

int uclass_get(enum uclass_id id, struct uclass **ucp)
{
	struct uclass *uc;

	/* Immediately fail if driver model is not set up */
	if (!gd->uclass_root)
		return -EDEADLK;
	*ucp = NULL;
	uc = uclass_find(id);
	if (!uc) {
		if (CONFIG_IS_ENABLED(OF_PLATDATA_INST))
			return -ENOENT;
		return uclass_add(id, ucp);
	}
	*ucp = uc;

	return 0;
}

const char *uclass_get_name(enum uclass_id id)
{
	struct uclass *uc;

	if (uclass_get(id, &uc))
		return NULL;
	return uc->uc_drv->name;
}

void *uclass_get_priv(const struct uclass *uc)
{
	return uc->priv_;
}

void uclass_set_priv(struct uclass *uc, void *priv)
{
	uc->priv_ = priv;
}

enum uclass_id uclass_get_by_namelen(const char *name, int len)
{
	int i;

	for (i = 0; i < UCLASS_COUNT; i++) {
		struct uclass_driver *uc_drv = lists_uclass_lookup(i);

		if (uc_drv && !strncmp(uc_drv->name, name, len) &&
		    strlen(uc_drv->name) == len)
			return i;
	}

	return UCLASS_INVALID;
}

enum uclass_id uclass_get_by_name(const char *name)
{
	return uclass_get_by_namelen(name, strlen(name));
}

int dev_get_uclass_index(struct udevice *dev, struct uclass **ucp)
{
	struct udevice *iter;
	struct uclass *uc = dev->uclass;
	int i = 0;

	if (list_empty(&uc->dev_head))
		return -ENODEV;

	uclass_foreach_dev(iter, uc) {
		if (iter == dev) {
			if (ucp)
				*ucp = uc;
			return i;
		}
		i++;
	}

	return -ENODEV;
}

int uclass_find_device(enum uclass_id id, int index, struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = uclass_get(id, &uc);
	if (ret)
		return ret;
	if (list_empty(&uc->dev_head))
		return -ENODEV;

	uclass_foreach_dev(dev, uc) {
		if (!index--) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int uclass_find_first_device(enum uclass_id id, struct udevice **devp)
{
	struct uclass *uc;
	int ret;

	*devp = NULL;
	ret = uclass_get(id, &uc);
	if (ret)
		return ret;
	if (list_empty(&uc->dev_head))
		return 0;

	*devp = list_first_entry(&uc->dev_head, struct udevice, uclass_node);

	return 0;
}

int uclass_find_next_device(struct udevice **devp)
{
	struct udevice *dev = *devp;

	*devp = NULL;
	if (list_is_last(&dev->uclass_node, &dev->uclass->dev_head))
		return 0;

	*devp = list_entry(dev->uclass_node.next, struct udevice, uclass_node);

	return 0;
}

int uclass_find_device_by_namelen(enum uclass_id id, const char *name, int len,
				  struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	*devp = NULL;
	if (!name)
		return -EINVAL;
	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		if (!strncmp(dev->name, name, len) &&
		    strlen(dev->name) == len) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int uclass_find_device_by_name(enum uclass_id id, const char *name,
			       struct udevice **devp)
{
	return uclass_find_device_by_namelen(id, name, strlen(name), devp);
}

int uclass_find_next_free_seq(struct uclass *uc)
{
	struct udevice *dev;
	int max = -1;

	/* If using aliases, start with the highest alias value */
	if (CONFIG_IS_ENABLED(DM_SEQ_ALIAS) &&
	    (uc->uc_drv->flags & DM_UC_FLAG_SEQ_ALIAS))
		max = dev_read_alias_highest_id(uc->uc_drv->name);

	/* Avoid conflict with existing devices */
	list_for_each_entry(dev, &uc->dev_head, uclass_node) {
		if (dev->seq_ > max)
			max = dev->seq_;
	}
	/*
	 * At this point, max will be -1 if there are no existing aliases or
	 * devices
	 */

	return max + 1;
}

int uclass_find_device_by_seq(enum uclass_id id, int seq, struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	*devp = NULL;
	log_debug("%d\n", seq);
	if (seq == -1)
		return -ENODEV;
	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		log_debug("   - %d '%s'\n", dev->seq_, dev->name);
		if (dev->seq_ == seq) {
			*devp = dev;
			log_debug("   - found\n");
			return 0;
		}
	}
	log_debug("   - not found\n");

	return -ENODEV;
}

int uclass_find_device_by_of_offset(enum uclass_id id, int node,
				    struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	*devp = NULL;
	if (node < 0)
		return -ENODEV;
	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		if (dev_of_offset(dev) == node) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int uclass_find_device_by_ofnode(enum uclass_id id, ofnode node,
				 struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	log(LOGC_DM, LOGL_DEBUG, "Looking for %s\n", ofnode_get_name(node));
	*devp = NULL;
	if (!ofnode_valid(node))
		return -ENODEV;
	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		log(LOGC_DM, LOGL_DEBUG_CONTENT, "      - checking %s\n",
		    dev->name);
		if (ofnode_equal(dev_ofnode(dev), node)) {
			*devp = dev;
			goto done;
		}
	}
	ret = -ENODEV;

done:
	log(LOGC_DM, LOGL_DEBUG, "   - result for %s: %s (ret=%d)\n",
	    ofnode_get_name(node), *devp ? (*devp)->name : "(none)", ret);
	return ret;
}

#if CONFIG_IS_ENABLED(OF_REAL)
static int uclass_find_device_by_phandle_id(enum uclass_id id,
					    uint find_phandle,
					    struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		uint phandle;

		phandle = dev_read_phandle(dev);

		if (phandle == find_phandle) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int uclass_find_device_by_phandle(enum uclass_id id, struct udevice *parent,
				  const char *name, struct udevice **devp)
{
	int find_phandle;

	*devp = NULL;
	find_phandle = dev_read_u32_default(parent, name, -1);
	if (find_phandle <= 0)
		return -ENOENT;

	return uclass_find_device_by_phandle_id(id, find_phandle, devp);
}
#endif

int uclass_get_device_by_driver(enum uclass_id id,
				const struct driver *find_drv,
				struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		if (dev->driver == find_drv)
			return uclass_get_device_tail(dev, 0, devp);
	}

	return -ENODEV;
}

int uclass_get_device_tail(struct udevice *dev, int ret, struct udevice **devp)
{
	if (ret)
		return ret;

	assert(dev);
	ret = device_probe(dev);
	if (ret)
		return ret;

	*devp = dev;

	return 0;
}

int uclass_get_device(enum uclass_id id, int index, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = uclass_find_device(id, index, &dev);
	return uclass_get_device_tail(dev, ret, devp);
}

int uclass_get_device_by_name(enum uclass_id id, const char *name,
			      struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = uclass_find_device_by_name(id, name, &dev);
	return uclass_get_device_tail(dev, ret, devp);
}

int uclass_get_device_by_seq(enum uclass_id id, int seq, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = uclass_find_device_by_seq(id, seq, &dev);

	return uclass_get_device_tail(dev, ret, devp);
}

int uclass_get_device_by_of_offset(enum uclass_id id, int node,
				   struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = uclass_find_device_by_of_offset(id, node, &dev);
	return uclass_get_device_tail(dev, ret, devp);
}

int uclass_get_device_by_ofnode(enum uclass_id id, ofnode node,
				struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	log(LOGC_DM, LOGL_DEBUG, "Looking for %s\n", ofnode_get_name(node));
	*devp = NULL;
	ret = uclass_find_device_by_ofnode(id, node, &dev);
	log(LOGC_DM, LOGL_DEBUG, "   - result for %s: %s (ret=%d)\n",
	    ofnode_get_name(node), dev ? dev->name : "(none)", ret);

	return uclass_get_device_tail(dev, ret, devp);
}

#if CONFIG_IS_ENABLED(OF_REAL)
int uclass_get_device_by_of_path(enum uclass_id id, const char *path,
				 struct udevice **devp)
{
	return uclass_get_device_by_ofnode(id, ofnode_path(path), devp);
}

int uclass_get_device_by_phandle_id(enum uclass_id id, uint phandle_id,
				    struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = uclass_find_device_by_phandle_id(id, phandle_id, &dev);
	return uclass_get_device_tail(dev, ret, devp);
}

int uclass_get_device_by_phandle(enum uclass_id id, struct udevice *parent,
				 const char *name, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = uclass_find_device_by_phandle(id, parent, name, &dev);
	return uclass_get_device_tail(dev, ret, devp);
}
#endif

/*
 * Starting from the given device @dev, return pointer to the first device in
 * the uclass that probes successfully in @devp.
 */
static void _uclass_next_device(struct udevice *dev, struct udevice **devp)
{
	for (; dev; uclass_find_next_device(&dev)) {
		if (!device_probe(dev))
			break;
	}
	*devp = dev;
}

void uclass_first_device(enum uclass_id id, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	ret = uclass_find_first_device(id, &dev);
	_uclass_next_device(dev, devp);
}

void uclass_next_device(struct udevice **devp)
{
	struct udevice *dev = *devp;

	uclass_find_next_device(&dev);
	_uclass_next_device(dev, devp);
}

int uclass_first_device_err(enum uclass_id id, struct udevice **devp)
{
	int ret;

	ret = uclass_first_device_check(id, devp);
	if (ret)
		return ret;
	else if (!*devp)
		return -ENODEV;

	return 0;
}

int uclass_next_device_err(struct udevice **devp)
{
	int ret;

	ret = uclass_next_device_check(devp);
	if (ret)
		return ret;
	else if (!*devp)
		return -ENODEV;

	return 0;
}

int uclass_first_device_check(enum uclass_id id, struct udevice **devp)
{
	int ret;

	*devp = NULL;
	ret = uclass_find_first_device(id, devp);
	if (ret)
		return ret;
	if (!*devp)
		return 0;

	return device_probe(*devp);
}

int uclass_next_device_check(struct udevice **devp)
{
	int ret;

	ret = uclass_find_next_device(devp);
	if (ret)
		return ret;
	if (!*devp)
		return 0;

	return device_probe(*devp);
}

int uclass_get_count(void)
{
	const struct uclass *uc;
	int count = 0;

	if (gd->dm_root) {
		list_for_each_entry(uc, gd->uclass_root, sibling_node)
			count++;
	}

	return count;
}

int uclass_first_device_drvdata(enum uclass_id id, ulong driver_data,
				struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(id, dev, uc) {
		if (dev_get_driver_data(dev) == driver_data) {
			*devp = dev;

			return device_probe(dev);
		}
	}

	return -ENODEV;
}

int uclass_bind_device(struct udevice *dev)
{
	struct uclass *uc;
	int ret;

	uc = dev->uclass;
	list_add_tail(&dev->uclass_node, &uc->dev_head);

	if (dev->parent) {
		struct uclass_driver *uc_drv = dev->parent->uclass->uc_drv;

		if (uc_drv->child_post_bind) {
			ret = uc_drv->child_post_bind(dev);
			if (ret)
				goto err;
		}
	}

	return 0;
err:
	/* There is no need to undo the parent's post_bind call */
	list_del(&dev->uclass_node);

	return ret;
}

#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
int uclass_pre_unbind_device(struct udevice *dev)
{
	struct uclass *uc;
	int ret;

	uc = dev->uclass;
	if (uc->uc_drv->pre_unbind) {
		ret = uc->uc_drv->pre_unbind(dev);
		if (ret)
			return ret;
	}

	return 0;
}

int uclass_unbind_device(struct udevice *dev)
{
	list_del(&dev->uclass_node);

	return 0;
}
#endif

int uclass_pre_probe_device(struct udevice *dev)
{
	struct uclass_driver *uc_drv;
	int ret;

	uc_drv = dev->uclass->uc_drv;
	if (uc_drv->pre_probe) {
		ret = uc_drv->pre_probe(dev);
		if (ret)
			return ret;
	}

	if (!dev->parent)
		return 0;
	uc_drv = dev->parent->uclass->uc_drv;
	if (uc_drv->child_pre_probe) {
		ret = uc_drv->child_pre_probe(dev);
		if (ret)
			return ret;
	}

	return 0;
}

int uclass_post_probe_device(struct udevice *dev)
{
	struct uclass_driver *uc_drv;
	int ret;

	if (dev->parent) {
		uc_drv = dev->parent->uclass->uc_drv;
		if (uc_drv->child_post_probe) {
			ret = uc_drv->child_post_probe(dev);
			if (ret)
				return ret;
		}
	}

	uc_drv = dev->uclass->uc_drv;
	if (uc_drv->post_probe) {
		ret = uc_drv->post_probe(dev);
		if (ret)
			return ret;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
int uclass_pre_remove_device(struct udevice *dev)
{
	struct uclass *uc;
	int ret;

	uc = dev->uclass;
	if (uc->uc_drv->pre_remove) {
		ret = uc->uc_drv->pre_remove(dev);
		if (ret)
			return ret;
	}

	return 0;
}
#endif

int uclass_probe_all(enum uclass_id id)
{
	struct udevice *dev;
	int ret, err;

	err = uclass_first_device_check(id, &dev);

	/* Scanning uclass to probe all devices */
	while (dev) {
		ret = uclass_next_device_check(&dev);
		if (ret)
			err = ret;
	}

	return err;
}

int uclass_id_count(enum uclass_id id)
{
	struct udevice *dev;
	struct uclass *uc;
	int count = 0;

	uclass_id_foreach_dev(id, dev, uc)
		count++;

	return count;
}

UCLASS_DRIVER(nop) = {
	.id		= UCLASS_NOP,
	.name		= "nop",
};
