/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __NVME_H__
#define __NVME_H__

struct nvme_dev;

/**
 * nvme_identify - identify controller or namespace capabilities and status
 *
 * This issues an identify command to the NVMe controller to return a data
 * buffer that describes the controller or namespace capabilities and status.
 *
 * @dev:	NVMe controller device
 * @nsid:	0 for controller, namespace id for namespace to identify
 * @cns:	1 for controller, 0 for namespace
 * @dma_addr:	dma buffer address to store the identify result
 * @return:	0 on success, -ETIMEDOUT on command execution timeout,
 *		-EIO on command execution fails
 */
int nvme_identify(struct nvme_dev *dev, unsigned nsid,
		  unsigned cns, dma_addr_t dma_addr);

/**
 * nvme_get_features - retrieve the attributes of the feature specified
 *
 * This retrieves the attributes of the feature specified.
 *
 * @dev:	NVMe controller device
 * @fid:	feature id to provide data
 * @nsid:	namespace id the command applies to
 * @dma_addr:	data structure used as part of the specified feature
 * @result:	command-specific result in the completion queue entry
 * @return:	0 on success, -ETIMEDOUT on command execution timeout,
 *		-EIO on command execution fails
 */
int nvme_get_features(struct nvme_dev *dev, unsigned fid, unsigned nsid,
		      dma_addr_t dma_addr, u32 *result);

/**
 * nvme_set_features - specify the attributes of the feature indicated
 *
 * This specifies the attributes of the feature indicated.
 *
 * @dev:	NVMe controller device
 * @fid:	feature id to provide data
 * @dword11:	command-specific input parameter
 * @dma_addr:	data structure used as part of the specified feature
 * @result:	command-specific result in the completion queue entry
 * @return:	0 on success, -ETIMEDOUT on command execution timeout,
 *		-EIO on command execution fails
 */
int nvme_set_features(struct nvme_dev *dev, unsigned fid, unsigned dword11,
		      dma_addr_t dma_addr, u32 *result);

/**
 * nvme_scan_namespace - scan all namespaces attached to NVMe controllers
 *
 * This probes all registered NVMe uclass device drivers in the system,
 * and tries to find all namespaces attached to the NVMe controllers.
 *
 * @return:	0 on success, -ve on error
 */
int nvme_scan_namespace(void);

/**
 * nvme_print_info - print detailed NVMe controller and namespace information
 *
 * This prints out detailed human readable NVMe controller and namespace
 * information which is very useful for debugging.
 *
 * @udev:	NVMe controller device
 * @return:	0 on success, -EIO if NVMe identify command fails
 */
int nvme_print_info(struct udevice *udev);

/**
 * nvme_get_namespace_id - return namespace identifier
 *
 * This returns the namespace identifier.
 *
 * @udev:	NVMe controller device
 * @ns_id:	Place where to put the name space identifier
 * @eui64:	Place where to put the IEEE Extended Unique Identifier
 * @return:	0 on success, -ve on error
 */
int nvme_get_namespace_id(struct udevice *udev, u32 *ns_id, u8 *eui64);

#endif /* __NVME_H__ */
