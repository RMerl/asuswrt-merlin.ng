// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
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
	list_for_each_entry(uc, &gd->uclass_root, sibling_node) {
		if (uc->uc_drv->id == key)
			return uc;
	}

	return NULL;
}

/**
 * uclass_add() - Create new uclass in list
 * @id: Id number to create
 * @ucp: Returns pointer to uclass, or NULL on error
 * @return 0 on success, -ve on error
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
	if (uc_drv->priv_auto_alloc_size) {
		uc->priv = calloc(1, uc_drv->priv_auto_alloc_size);
		if (!uc->priv) {
			ret = -ENOMEM;
			goto fail_mem;
		}
	}
	uc->uc_drv = uc_drv;
	INIT_LIST_HEAD(&uc->sibling_node);
	INIT_LIST_HEAD(&uc->dev_head);
	list_add(&uc->sibling_node, &DM_UCLASS_ROOT_NON_CONST);

	if (uc_drv->init) {
		ret = uc_drv->init(uc);
		if (ret)
			goto fail;
	}

	*ucp = uc;

	return 0;
fail:
	if (uc_drv->priv_auto_alloc_size) {
		free(uc->priv);
		uc->priv = NULL;
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
		ret = device_remove(dev, DM_REMOVE_NORMAL);
		if (ret)
			return ret;
		ret = device_unbind(dev);
		if (ret)
			return ret;
	}

	uc_drv = uc->uc_drv;
	if (uc_drv->destroy)
		uc_drv->destroy(uc);
	list_del(&uc->sibling_node);
	if (uc_drv->priv_auto_alloc_size)
		free(uc->priv);
	free(uc);

	return 0;
}

int uclass_get(enum uclass_id id, struct uclass **ucp)
{
	struct uclass *uc;

	*ucp = NULL;
	uc = uclass_find(id);
	if (!uc)
		return uclass_add(id, ucp);
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

enum uclass_id uclass_get_by_name(const char *name)
{
	int i;

	for (i = 0; i < UCLASS_COUNT; i++) {
		struct uclass_driver *uc_drv = lists_uclass_lookup(i);

		if (uc_drv && !strcmp(uc_drv->name, name))
			return i;
	}

	return UCLASS_INVALID;
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
		return -ENODEV;

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

int uclass_find_device_by_name(enum uclass_id id, const char *name,
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
		if (!strncmp(dev->name, name, strlen(name))) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

#if !CONFIG_IS_ENABLED(OF_CONTROL) || CONFIG_IS_ENABLED(OF_PLATDATA)
int uclass_find_next_free_req_seq(enum uclass_id id)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;
	int max = -1;

	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	list_for_each_entry(dev, &uc->dev_head, uclass_node) {
		if ((dev->req_seq != -1) && (dev->req_seq > max))
			max = dev->req_seq;
	}

	if (max == -1)
		return 0;

	return max + 1;
}
#endif

int uclass_find_device_by_seq(enum uclass_id id, int seq_or_req_seq,
			      bool find_req_seq, struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	*devp = NULL;
	debug("%s: %d %d\n", __func__, find_req_seq, seq_or_req_seq);
	if (seq_or_req_seq == -1)
		return -ENODEV;
	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		debug("   - %d %d '%s'\n", dev->req_seq, dev->seq, dev->name);
		if ((find_req_seq ? dev->req_seq : dev->seq) ==
				seq_or_req_seq) {
			*devp = dev;
			debug("   - found\n");
			return 0;
		}
	}
	debug("   - not found\n");

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

#if CONFIG_IS_ENABLED(OF_CONTROL)
int uclass_find_device_by_phandle(enum uclass_id id, struct udevice *parent,
				  const char *name, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int find_phandle;
	int ret;

	*devp = NULL;
	find_phandle = dev_read_u32_default(parent, name, -1);
	if (find_phandle <= 0)
		return -ENOENT;
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
	ret = uclass_find_device_by_seq(id, seq, false, &dev);
	if (ret == -ENODEV) {
		/*
		 * We didn't find it in probed devices. See if there is one
		 * that will request this seq if probed.
		 */
		ret = uclass_find_device_by_seq(id, seq, true, &dev);
	}
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

#if CONFIG_IS_ENABLED(OF_CONTROL)
int uclass_get_device_by_phandle_id(enum uclass_id id, uint phandle_id,
				    struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	*devp = NULL;
	ret = uclass_get(id, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		uint phandle;

		phandle = dev_read_phandle(dev);

		if (phandle == phandle_id) {
			*devp = dev;
			return uclass_get_device_tail(dev, ret, devp);
		}
	}

	return -ENODEV;
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

int uclass_first_device(enum uclass_id id, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = uclass_find_first_device(id, &dev);
	if (!dev)
		return 0;
	return uclass_get_device_tail(dev, ret, devp);
}

int uclass_first_device_err(enum uclass_id id, struct udevice **devp)
{
	int ret;

	ret = uclass_first_device(id, devp);
	if (ret)
		return ret;
	else if (!*devp)
		return -ENODEV;

	return 0;
}

int uclass_next_device(struct udevice **devp)
{
	struct udevice *dev = *devp;
	int ret;

	*devp = NULL;
	ret = uclass_find_next_device(&dev);
	if (!dev)
		return 0;
	return uclass_get_device_tail(dev, ret, devp);
}

int uclass_next_device_err(struct udevice **devp)
{
	int ret;

	ret = uclass_next_device(devp);
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
int uclass_unbind_device(struct udevice *dev)
{
	struct uclass *uc;
	int ret;

	uc = dev->uclass;
	if (uc->uc_drv->pre_unbind) {
		ret = uc->uc_drv->pre_unbind(dev);
		if (ret)
			return ret;
	}

	list_del(&dev->uclass_node);
	return 0;
}
#endif

int uclass_resolve_seq(struct udevice *dev)
{
	struct udevice *dup;
	int seq;
	int ret;

	assert(dev->seq == -1);
	ret = uclass_find_device_by_seq(dev->uclass->uc_drv->id, dev->req_seq,
					false, &dup);
	if (!ret) {
		dm_warn("Device '%s': seq %d is in use by '%s'\n",
			dev->name, dev->req_seq, dup->name);
	} else if (ret == -ENODEV) {
		/* Our requested sequence number is available */
		if (dev->req_seq != -1)
			return dev->req_seq;
	} else {
		return ret;
	}

	for (seq = 0; seq < DM_MAX_SEQ; seq++) {
		ret = uclass_find_device_by_seq(dev->uclass->uc_drv->id, seq,
						false, &dup);
		if (ret == -ENODEV)
			break;
		if (ret)
			return ret;
	}
	return seq;
}

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
	if (uc_drv->child_pre_probe)
		return uc_drv->child_pre_probe(dev);

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
	if (uc_drv->post_probe)
		return uc_drv->post_probe(dev);

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

UCLASS_DRIVER(nop) = {
	.id		= UCLASS_NOP,
	.name		= "nop",
};
