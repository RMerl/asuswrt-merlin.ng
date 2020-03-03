/*
 * Xilinx Video IP Composite Device
 *
 * Copyright (C) 2013-2015 Ideas on Board
 * Copyright (C) 2013-2015 Xilinx, Inc.
 *
 * Contacts: Hyun Kwon <hyun.kwon@xilinx.com>
 *           Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_graph.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <media/v4l2-async.h>
#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-of.h>

#include "xilinx-dma.h"
#include "xilinx-vipp.h"

#define XVIPP_DMA_S2MM				0
#define XVIPP_DMA_MM2S				1

/**
 * struct xvip_graph_entity - Entity in the video graph
 * @list: list entry in a graph entities list
 * @node: the entity's DT node
 * @entity: media entity, from the corresponding V4L2 subdev
 * @asd: subdev asynchronous registration information
 * @subdev: V4L2 subdev
 */
struct xvip_graph_entity {
	struct list_head list;
	struct device_node *node;
	struct media_entity *entity;

	struct v4l2_async_subdev asd;
	struct v4l2_subdev *subdev;
};

/* -----------------------------------------------------------------------------
 * Graph Management
 */

static struct xvip_graph_entity *
xvip_graph_find_entity(struct xvip_composite_device *xdev,
		       const struct device_node *node)
{
	struct xvip_graph_entity *entity;

	list_for_each_entry(entity, &xdev->entities, list) {
		if (entity->node == node)
			return entity;
	}

	return NULL;
}

static int xvip_graph_build_one(struct xvip_composite_device *xdev,
				struct xvip_graph_entity *entity)
{
	u32 link_flags = MEDIA_LNK_FL_ENABLED;
	struct media_entity *local = entity->entity;
	struct media_entity *remote;
	struct media_pad *local_pad;
	struct media_pad *remote_pad;
	struct xvip_graph_entity *ent;
	struct v4l2_of_link link;
	struct device_node *ep = NULL;
	struct device_node *next;
	int ret = 0;

	dev_dbg(xdev->dev, "creating links for entity %s\n", local->name);

	while (1) {
		/* Get the next endpoint and parse its link. */
		next = of_graph_get_next_endpoint(entity->node, ep);
		if (next == NULL)
			break;

		of_node_put(ep);
		ep = next;

		dev_dbg(xdev->dev, "processing endpoint %s\n", ep->full_name);

		ret = v4l2_of_parse_link(ep, &link);
		if (ret < 0) {
			dev_err(xdev->dev, "failed to parse link for %s\n",
				ep->full_name);
			continue;
		}

		/* Skip sink ports, they will be processed from the other end of
		 * the link.
		 */
		if (link.local_port >= local->num_pads) {
			dev_err(xdev->dev, "invalid port number %u on %s\n",
				link.local_port, link.local_node->full_name);
			v4l2_of_put_link(&link);
			ret = -EINVAL;
			break;
		}

		local_pad = &local->pads[link.local_port];

		if (local_pad->flags & MEDIA_PAD_FL_SINK) {
			dev_dbg(xdev->dev, "skipping sink port %s:%u\n",
				link.local_node->full_name, link.local_port);
			v4l2_of_put_link(&link);
			continue;
		}

		/* Skip DMA engines, they will be processed separately. */
		if (link.remote_node == xdev->dev->of_node) {
			dev_dbg(xdev->dev, "skipping DMA port %s:%u\n",
				link.local_node->full_name, link.local_port);
			v4l2_of_put_link(&link);
			continue;
		}

		/* Find the remote entity. */
		ent = xvip_graph_find_entity(xdev, link.remote_node);
		if (ent == NULL) {
			dev_err(xdev->dev, "no entity found for %s\n",
				link.remote_node->full_name);
			v4l2_of_put_link(&link);
			ret = -ENODEV;
			break;
		}

		remote = ent->entity;

		if (link.remote_port >= remote->num_pads) {
			dev_err(xdev->dev, "invalid port number %u on %s\n",
				link.remote_port, link.remote_node->full_name);
			v4l2_of_put_link(&link);
			ret = -EINVAL;
			break;
		}

		remote_pad = &remote->pads[link.remote_port];

		v4l2_of_put_link(&link);

		/* Create the media link. */
		dev_dbg(xdev->dev, "creating %s:%u -> %s:%u link\n",
			local->name, local_pad->index,
			remote->name, remote_pad->index);

		ret = media_entity_create_link(local, local_pad->index,
					       remote, remote_pad->index,
					       link_flags);
		if (ret < 0) {
			dev_err(xdev->dev,
				"failed to create %s:%u -> %s:%u link\n",
				local->name, local_pad->index,
				remote->name, remote_pad->index);
			break;
		}
	}

	of_node_put(ep);
	return ret;
}

static struct xvip_dma *
xvip_graph_find_dma(struct xvip_composite_device *xdev, unsigned int port)
{
	struct xvip_dma *dma;

	list_for_each_entry(dma, &xdev->dmas, list) {
		if (dma->port == port)
			return dma;
	}

	return NULL;
}

static int xvip_graph_build_dma(struct xvip_composite_device *xdev)
{
	u32 link_flags = MEDIA_LNK_FL_ENABLED;
	struct device_node *node = xdev->dev->of_node;
	struct media_entity *source;
	struct media_entity *sink;
	struct media_pad *source_pad;
	struct media_pad *sink_pad;
	struct xvip_graph_entity *ent;
	struct v4l2_of_link link;
	struct device_node *ep = NULL;
	struct device_node *next;
	struct xvip_dma *dma;
	int ret = 0;

	dev_dbg(xdev->dev, "creating links for DMA engines\n");

	while (1) {
		/* Get the next endpoint and parse its link. */
		next = of_graph_get_next_endpoint(node, ep);
		if (next == NULL)
			break;

		of_node_put(ep);
		ep = next;

		dev_dbg(xdev->dev, "processing endpoint %s\n", ep->full_name);

		ret = v4l2_of_parse_link(ep, &link);
		if (ret < 0) {
			dev_err(xdev->dev, "failed to parse link for %s\n",
				ep->full_name);
			continue;
		}

		/* Find the DMA engine. */
		dma = xvip_graph_find_dma(xdev, link.local_port);
		if (dma == NULL) {
			dev_err(xdev->dev, "no DMA engine found for port %u\n",
				link.local_port);
			v4l2_of_put_link(&link);
			ret = -EINVAL;
			break;
		}

		dev_dbg(xdev->dev, "creating link for DMA engine %s\n",
			dma->video.name);

		/* Find the remote entity. */
		ent = xvip_graph_find_entity(xdev, link.remote_node);
		if (ent == NULL) {
			dev_err(xdev->dev, "no entity found for %s\n",
				link.remote_node->full_name);
			v4l2_of_put_link(&link);
			ret = -ENODEV;
			break;
		}

		if (link.remote_port >= ent->entity->num_pads) {
			dev_err(xdev->dev, "invalid port number %u on %s\n",
				link.remote_port, link.remote_node->full_name);
			v4l2_of_put_link(&link);
			ret = -EINVAL;
			break;
		}

		if (dma->pad.flags & MEDIA_PAD_FL_SOURCE) {
			source = &dma->video.entity;
			source_pad = &dma->pad;
			sink = ent->entity;
			sink_pad = &sink->pads[link.remote_port];
		} else {
			source = ent->entity;
			source_pad = &source->pads[link.remote_port];
			sink = &dma->video.entity;
			sink_pad = &dma->pad;
		}

		v4l2_of_put_link(&link);

		/* Create the media link. */
		dev_dbg(xdev->dev, "creating %s:%u -> %s:%u link\n",
			source->name, source_pad->index,
			sink->name, sink_pad->index);

		ret = media_entity_create_link(source, source_pad->index,
					       sink, sink_pad->index,
					       link_flags);
		if (ret < 0) {
			dev_err(xdev->dev,
				"failed to create %s:%u -> %s:%u link\n",
				source->name, source_pad->index,
				sink->name, sink_pad->index);
			break;
		}
	}

	of_node_put(ep);
	return ret;
}

static int xvip_graph_notify_complete(struct v4l2_async_notifier *notifier)
{
	struct xvip_composite_device *xdev =
		container_of(notifier, struct xvip_composite_device, notifier);
	struct xvip_graph_entity *entity;
	int ret;

	dev_dbg(xdev->dev, "notify complete, all subdevs registered\n");

	/* Create links for every entity. */
	list_for_each_entry(entity, &xdev->entities, list) {
		ret = xvip_graph_build_one(xdev, entity);
		if (ret < 0)
			return ret;
	}

	/* Create links for DMA channels. */
	ret = xvip_graph_build_dma(xdev);
	if (ret < 0)
		return ret;

	ret = v4l2_device_register_subdev_nodes(&xdev->v4l2_dev);
	if (ret < 0)
		dev_err(xdev->dev, "failed to register subdev nodes\n");

	return ret;
}

static int xvip_graph_notify_bound(struct v4l2_async_notifier *notifier,
				   struct v4l2_subdev *subdev,
				   struct v4l2_async_subdev *asd)
{
	struct xvip_composite_device *xdev =
		container_of(notifier, struct xvip_composite_device, notifier);
	struct xvip_graph_entity *entity;

	/* Locate the entity corresponding to the bound subdev and store the
	 * subdev pointer.
	 */
	list_for_each_entry(entity, &xdev->entities, list) {
		if (entity->node != subdev->dev->of_node)
			continue;

		if (entity->subdev) {
			dev_err(xdev->dev, "duplicate subdev for node %s\n",
				entity->node->full_name);
			return -EINVAL;
		}

		dev_dbg(xdev->dev, "subdev %s bound\n", subdev->name);
		entity->entity = &subdev->entity;
		entity->subdev = subdev;
		return 0;
	}

	dev_err(xdev->dev, "no entity for subdev %s\n", subdev->name);
	return -EINVAL;
}

static int xvip_graph_parse_one(struct xvip_composite_device *xdev,
				struct device_node *node)
{
	struct xvip_graph_entity *entity;
	struct device_node *remote;
	struct device_node *ep = NULL;
	struct device_node *next;
	int ret = 0;

	dev_dbg(xdev->dev, "parsing node %s\n", node->full_name);

	while (1) {
		next = of_graph_get_next_endpoint(node, ep);
		if (next == NULL)
			break;

		of_node_put(ep);
		ep = next;

		dev_dbg(xdev->dev, "handling endpoint %s\n", ep->full_name);

		remote = of_graph_get_remote_port_parent(ep);
		if (remote == NULL) {
			ret = -EINVAL;
			break;
		}

		/* Skip entities that we have already processed. */
		if (remote == xdev->dev->of_node ||
		    xvip_graph_find_entity(xdev, remote)) {
			of_node_put(remote);
			continue;
		}

		entity = devm_kzalloc(xdev->dev, sizeof(*entity), GFP_KERNEL);
		if (entity == NULL) {
			of_node_put(remote);
			ret = -ENOMEM;
			break;
		}

		entity->node = remote;
		entity->asd.match_type = V4L2_ASYNC_MATCH_OF;
		entity->asd.match.of.node = remote;
		list_add_tail(&entity->list, &xdev->entities);
		xdev->num_subdevs++;
	}

	of_node_put(ep);
	return ret;
}

static int xvip_graph_parse(struct xvip_composite_device *xdev)
{
	struct xvip_graph_entity *entity;
	int ret;

	/*
	 * Walk the links to parse the full graph. Start by parsing the
	 * composite node and then parse entities in turn. The list_for_each
	 * loop will handle entities added at the end of the list while walking
	 * the links.
	 */
	ret = xvip_graph_parse_one(xdev, xdev->dev->of_node);
	if (ret < 0)
		return 0;

	list_for_each_entry(entity, &xdev->entities, list) {
		ret = xvip_graph_parse_one(xdev, entity->node);
		if (ret < 0)
			break;
	}

	return ret;
}

static int xvip_graph_dma_init_one(struct xvip_composite_device *xdev,
				   struct device_node *node)
{
	struct xvip_dma *dma;
	enum v4l2_buf_type type;
	const char *direction;
	unsigned int index;
	int ret;

	ret = of_property_read_string(node, "direction", &direction);
	if (ret < 0)
		return ret;

	if (strcmp(direction, "input") == 0)
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	else if (strcmp(direction, "output") == 0)
		type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	else
		return -EINVAL;

	of_property_read_u32(node, "reg", &index);

	dma = devm_kzalloc(xdev->dev, sizeof(*dma), GFP_KERNEL);
	if (dma == NULL)
		return -ENOMEM;

	ret = xvip_dma_init(xdev, dma, type, index);
	if (ret < 0) {
		dev_err(xdev->dev, "%s initialization failed\n",
			node->full_name);
		return ret;
	}

	list_add_tail(&dma->list, &xdev->dmas);

	xdev->v4l2_caps |= type == V4L2_BUF_TYPE_VIDEO_CAPTURE
			 ? V4L2_CAP_VIDEO_CAPTURE : V4L2_CAP_VIDEO_OUTPUT;

	return 0;
}

static int xvip_graph_dma_init(struct xvip_composite_device *xdev)
{
	struct device_node *ports;
	struct device_node *port;
	int ret;

	ports = of_get_child_by_name(xdev->dev->of_node, "ports");
	if (ports == NULL) {
		dev_err(xdev->dev, "ports node not present\n");
		return -EINVAL;
	}

	for_each_child_of_node(ports, port) {
		ret = xvip_graph_dma_init_one(xdev, port);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static void xvip_graph_cleanup(struct xvip_composite_device *xdev)
{
	struct xvip_graph_entity *entityp;
	struct xvip_graph_entity *entity;
	struct xvip_dma *dmap;
	struct xvip_dma *dma;

	v4l2_async_notifier_unregister(&xdev->notifier);

	list_for_each_entry_safe(entity, entityp, &xdev->entities, list) {
		of_node_put(entity->node);
		list_del(&entity->list);
	}

	list_for_each_entry_safe(dma, dmap, &xdev->dmas, list) {
		xvip_dma_cleanup(dma);
		list_del(&dma->list);
	}
}

static int xvip_graph_init(struct xvip_composite_device *xdev)
{
	struct xvip_graph_entity *entity;
	struct v4l2_async_subdev **subdevs = NULL;
	unsigned int num_subdevs;
	unsigned int i;
	int ret;

	/* Init the DMA channels. */
	ret = xvip_graph_dma_init(xdev);
	if (ret < 0) {
		dev_err(xdev->dev, "DMA initialization failed\n");
		goto done;
	}

	/* Parse the graph to extract a list of subdevice DT nodes. */
	ret = xvip_graph_parse(xdev);
	if (ret < 0) {
		dev_err(xdev->dev, "graph parsing failed\n");
		goto done;
	}

	if (!xdev->num_subdevs) {
		dev_err(xdev->dev, "no subdev found in graph\n");
		goto done;
	}

	/* Register the subdevices notifier. */
	num_subdevs = xdev->num_subdevs;
	subdevs = devm_kzalloc(xdev->dev, sizeof(*subdevs) * num_subdevs,
			       GFP_KERNEL);
	if (subdevs == NULL) {
		ret = -ENOMEM;
		goto done;
	}

	i = 0;
	list_for_each_entry(entity, &xdev->entities, list)
		subdevs[i++] = &entity->asd;

	xdev->notifier.subdevs = subdevs;
	xdev->notifier.num_subdevs = num_subdevs;
	xdev->notifier.bound = xvip_graph_notify_bound;
	xdev->notifier.complete = xvip_graph_notify_complete;

	ret = v4l2_async_notifier_register(&xdev->v4l2_dev, &xdev->notifier);
	if (ret < 0) {
		dev_err(xdev->dev, "notifier registration failed\n");
		goto done;
	}

	ret = 0;

done:
	if (ret < 0)
		xvip_graph_cleanup(xdev);

	return ret;
}

/* -----------------------------------------------------------------------------
 * Media Controller and V4L2
 */

static void xvip_composite_v4l2_cleanup(struct xvip_composite_device *xdev)
{
	v4l2_device_unregister(&xdev->v4l2_dev);
	media_device_unregister(&xdev->media_dev);
}

static int xvip_composite_v4l2_init(struct xvip_composite_device *xdev)
{
	int ret;

	xdev->media_dev.dev = xdev->dev;
	strlcpy(xdev->media_dev.model, "Xilinx Video Composite Device",
		sizeof(xdev->media_dev.model));
	xdev->media_dev.hw_revision = 0;

	ret = media_device_register(&xdev->media_dev);
	if (ret < 0) {
		dev_err(xdev->dev, "media device registration failed (%d)\n",
			ret);
		return ret;
	}

	xdev->v4l2_dev.mdev = &xdev->media_dev;
	ret = v4l2_device_register(xdev->dev, &xdev->v4l2_dev);
	if (ret < 0) {
		dev_err(xdev->dev, "V4L2 device registration failed (%d)\n",
			ret);
		media_device_unregister(&xdev->media_dev);
		return ret;
	}

	return 0;
}

/* -----------------------------------------------------------------------------
 * Platform Device Driver
 */

static int xvip_composite_probe(struct platform_device *pdev)
{
	struct xvip_composite_device *xdev;
	int ret;

	xdev = devm_kzalloc(&pdev->dev, sizeof(*xdev), GFP_KERNEL);
	if (!xdev)
		return -ENOMEM;

	xdev->dev = &pdev->dev;
	INIT_LIST_HEAD(&xdev->entities);
	INIT_LIST_HEAD(&xdev->dmas);

	ret = xvip_composite_v4l2_init(xdev);
	if (ret < 0)
		return ret;

	ret = xvip_graph_init(xdev);
	if (ret < 0)
		goto error;

	platform_set_drvdata(pdev, xdev);

	dev_info(xdev->dev, "device registered\n");

	return 0;

error:
	xvip_composite_v4l2_cleanup(xdev);
	return ret;
}

static int xvip_composite_remove(struct platform_device *pdev)
{
	struct xvip_composite_device *xdev = platform_get_drvdata(pdev);

	xvip_graph_cleanup(xdev);
	xvip_composite_v4l2_cleanup(xdev);

	return 0;
}

static const struct of_device_id xvip_composite_of_id_table[] = {
	{ .compatible = "xlnx,video" },
	{ }
};
MODULE_DEVICE_TABLE(of, xvip_composite_of_id_table);

static struct platform_driver xvip_composite_driver = {
	.driver = {
		.name = "xilinx-video",
		.of_match_table = xvip_composite_of_id_table,
	},
	.probe = xvip_composite_probe,
	.remove = xvip_composite_remove,
};

module_platform_driver(xvip_composite_driver);

MODULE_AUTHOR("Laurent Pinchart <laurent.pinchart@ideasonboard.com>");
MODULE_DESCRIPTION("Xilinx Video IP Composite Driver");
MODULE_LICENSE("GPL v2");
