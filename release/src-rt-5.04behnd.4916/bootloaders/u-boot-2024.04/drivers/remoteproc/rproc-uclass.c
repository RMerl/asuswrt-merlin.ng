// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated - https://www.ti.com/
 */

#define LOG_CATEGORY UCLASS_REMOTEPROC

#define pr_fmt(fmt) "%s: " fmt, __func__
#include <common.h>
#include <elf.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <virtio_ring.h>
#include <remoteproc.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <dm.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <linux/compat.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

struct resource_table {
	u32 ver;
	u32 num;
	u32 reserved[2];
	u32 offset[0];
} __packed;

typedef int (*handle_resource_t) (struct udevice *, void *, int offset, int avail);

static struct resource_table *rsc_table;

/**
 * for_each_remoteproc_device() - iterate through the list of rproc devices
 * @fn: check function to call per match, if this function returns fail,
 *	iteration is aborted with the resultant error value
 * @skip_dev:	Device to skip calling the callback about.
 * @data:	Data to pass to the callback function
 *
 * Return: 0 if none of the callback returned a non 0 result, else returns the
 * result from the callback function
 */
static int for_each_remoteproc_device(int (*fn) (struct udevice *dev,
					struct dm_rproc_uclass_pdata *uc_pdata,
					const void *data),
				      struct udevice *skip_dev,
				      const void *data)
{
	struct udevice *dev;
	struct dm_rproc_uclass_pdata *uc_pdata;
	int ret;

	for (ret = uclass_find_first_device(UCLASS_REMOTEPROC, &dev); dev;
	     ret = uclass_find_next_device(&dev)) {
		if (ret || dev == skip_dev)
			continue;
		uc_pdata = dev_get_uclass_plat(dev);
		ret = fn(dev, uc_pdata, data);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * _rproc_name_is_unique() - iteration helper to check if rproc name is unique
 * @dev:	device that we are checking name for
 * @uc_pdata:	uclass platform data
 * @data:	compare data (this is the name we want to ensure is unique)
 *
 * Return: 0 is there is no match(is unique); if there is a match(we dont
 * have a unique name), return -EINVAL.
 */
static int _rproc_name_is_unique(struct udevice *dev,
				 struct dm_rproc_uclass_pdata *uc_pdata,
				 const void *data)
{
	const char *check_name = data;

	/* devices not yet populated with data - so skip them */
	if (!uc_pdata->name || !check_name)
		return 0;

	/* Return 0 to search further if we dont match */
	if (strlen(uc_pdata->name) != strlen(check_name))
		return 0;

	if (!strcmp(uc_pdata->name, check_name))
		return -EINVAL;

	return 0;
}

/**
 * rproc_name_is_unique() - Check if the rproc name is unique
 * @check_dev:	Device we are attempting to ensure is unique
 * @check_name:	Name we are trying to ensure is unique.
 *
 * Return: true if we have a unique name, false if name is not unique.
 */
static bool rproc_name_is_unique(struct udevice *check_dev,
				 const char *check_name)
{
	int ret;

	ret = for_each_remoteproc_device(_rproc_name_is_unique,
					 check_dev, check_name);
	return ret ? false : true;
}

/**
 * rproc_pre_probe() - Pre probe accessor for the uclass
 * @dev:	device for which we are preprobing
 *
 * Parses and fills up the uclass pdata for use as needed by core and
 * remote proc drivers.
 *
 * Return: 0 if all wernt ok, else appropriate error value.
 */
static int rproc_pre_probe(struct udevice *dev)
{
	struct dm_rproc_uclass_pdata *uc_pdata;
	const struct dm_rproc_ops *ops;

	uc_pdata = dev_get_uclass_plat(dev);

	/* See if we need to populate via fdt */

	if (!dev_get_plat(dev)) {
#if CONFIG_IS_ENABLED(OF_CONTROL)
		bool tmp;
		debug("'%s': using fdt\n", dev->name);
		uc_pdata->name = dev_read_string(dev, "remoteproc-name");

		/* Default is internal memory mapped */
		uc_pdata->mem_type = RPROC_INTERNAL_MEMORY_MAPPED;
		tmp = dev_read_bool(dev, "remoteproc-internal-memory-mapped");
		if (tmp)
			uc_pdata->mem_type = RPROC_INTERNAL_MEMORY_MAPPED;
#else
		/* Nothing much we can do about this, can we? */
		return -EINVAL;
#endif

	} else {
		struct dm_rproc_uclass_pdata *pdata = dev_get_plat(dev);

		debug("'%s': using legacy data\n", dev->name);
		if (pdata->name)
			uc_pdata->name = pdata->name;
		uc_pdata->mem_type = pdata->mem_type;
		uc_pdata->driver_plat_data = pdata->driver_plat_data;
	}

	/* Else try using device Name */
	if (!uc_pdata->name)
		uc_pdata->name = dev->name;
	if (!uc_pdata->name) {
		debug("Unnamed device!");
		return -EINVAL;
	}

	if (!rproc_name_is_unique(dev, uc_pdata->name)) {
		debug("%s duplicate name '%s'\n", dev->name, uc_pdata->name);
		return -EINVAL;
	}

	ops = rproc_get_ops(dev);
	if (!ops) {
		debug("%s driver has no ops?\n", dev->name);
		return -EINVAL;
	}

	if (!ops->load || !ops->start) {
		debug("%s driver has missing mandatory ops?\n", dev->name);
		return -EINVAL;
	}

	return 0;
}

/**
 * rproc_post_probe() - post probe accessor for the uclass
 * @dev:	deivce we finished probing
 *
 * initiate init function after the probe is completed. This allows
 * the remote processor drivers to split up the initializations between
 * probe and init as needed.
 *
 * Return: if the remote proc driver has a init routine, invokes it and
 * hands over the return value. overall, 0 if all went well, else appropriate
 * error value.
 */
static int rproc_post_probe(struct udevice *dev)
{
	const struct dm_rproc_ops *ops;

	ops = rproc_get_ops(dev);
	if (!ops) {
		debug("%s driver has no ops?\n", dev->name);
		return -EINVAL;
	}

	if (ops->init)
		return ops->init(dev);

	return 0;
}

/**
 * rproc_add_res() - After parsing the resource table add the mappings
 * @dev:	device we finished probing
 * @mapping: rproc_mem_entry for the resource
 *
 * Return: if the remote proc driver has a add_res routine, invokes it and
 * hands over the return value. overall, 0 if all went well, else appropriate
 * error value.
 */
static int rproc_add_res(struct udevice *dev, struct rproc_mem_entry *mapping)
{
	const struct dm_rproc_ops *ops = rproc_get_ops(dev);

	if (!ops->add_res)
		return -ENOSYS;

	return ops->add_res(dev, mapping);
}

/**
 * rproc_alloc_mem() - After parsing the resource table allocat mem
 * @dev:	device we finished probing
 * @len: rproc_mem_entry for the resource
 * @align: alignment for the resource
 *
 * Return: if the remote proc driver has a add_res routine, invokes it and
 * hands over the return value. overall, 0 if all went well, else appropriate
 * error value.
 */
static void *rproc_alloc_mem(struct udevice *dev, unsigned long len,
			     unsigned long align)
{
	const struct dm_rproc_ops *ops;

	ops = rproc_get_ops(dev);
	if (!ops) {
		debug("%s driver has no ops?\n", dev->name);
		return NULL;
	}

	if (ops->alloc_mem)
		return ops->alloc_mem(dev, len, align);

	return NULL;
}

/**
 * rproc_config_pagetable() - Configure page table for remote processor
 * @dev:	device we finished probing
 * @virt: Virtual address of the resource
 * @phys: Physical address the resource
 * @len: length the resource
 *
 * Return: if the remote proc driver has a add_res routine, invokes it and
 * hands over the return value. overall, 0 if all went well, else appropriate
 * error value.
 */
static int rproc_config_pagetable(struct udevice *dev, unsigned int virt,
				  unsigned int phys, unsigned int len)
{
	const struct dm_rproc_ops *ops;

	ops = rproc_get_ops(dev);
	if (!ops) {
		debug("%s driver has no ops?\n", dev->name);
		return -EINVAL;
	}

	if (ops->config_pagetable)
		return ops->config_pagetable(dev, virt, phys, len);

	return 0;
}

UCLASS_DRIVER(rproc) = {
	.id = UCLASS_REMOTEPROC,
	.name = "remoteproc",
	.flags = DM_UC_FLAG_SEQ_ALIAS,
	.pre_probe = rproc_pre_probe,
	.post_probe = rproc_post_probe,
	.per_device_plat_auto	= sizeof(struct dm_rproc_uclass_pdata),
};

/* Remoteproc subsystem access functions */
/**
 * _rproc_probe_dev() - iteration helper to probe a rproc device
 * @dev:	device to probe
 * @uc_pdata:	uclass data allocated for the device
 * @data:	unused
 *
 * Return: 0 if all ok, else appropriate error value.
 */
static int _rproc_probe_dev(struct udevice *dev,
			    struct dm_rproc_uclass_pdata *uc_pdata,
			    const void *data)
{
	int ret;

	ret = device_probe(dev);

	if (ret)
		debug("%s: Failed to initialize - %d\n", dev->name, ret);
	return ret;
}

/**
 * _rproc_dev_is_probed() - check if the device has been probed
 * @dev:	device to check
 * @uc_pdata:	unused
 * @data:	unused
 *
 * Return: -EAGAIN if not probed else return 0
 */
static int _rproc_dev_is_probed(struct udevice *dev,
			    struct dm_rproc_uclass_pdata *uc_pdata,
			    const void *data)
{
	if (dev_get_flags(dev) & DM_FLAG_ACTIVATED)
		return 0;

	return -EAGAIN;
}

bool rproc_is_initialized(void)
{
	int ret = for_each_remoteproc_device(_rproc_dev_is_probed, NULL, NULL);
	return ret ? false : true;
}

int rproc_init(void)
{
	int ret;

	if (rproc_is_initialized()) {
		debug("Already initialized\n");
		return -EINVAL;
	}

	ret = for_each_remoteproc_device(_rproc_probe_dev, NULL, NULL);
	return ret;
}

int rproc_dev_init(int id)
{
	struct udevice *dev = NULL;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_REMOTEPROC, id, &dev);
	if (ret) {
		debug("Unknown remote processor id '%d' requested(%d)\n",
		      id, ret);
		return ret;
	}

	ret = device_probe(dev);
	if (ret)
		debug("%s: Failed to initialize - %d\n", dev->name, ret);

	return ret;
}

int rproc_load(int id, ulong addr, ulong size)
{
	struct udevice *dev = NULL;
	struct dm_rproc_uclass_pdata *uc_pdata;
	const struct dm_rproc_ops *ops;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_REMOTEPROC, id, &dev);
	if (ret) {
		debug("Unknown remote processor id '%d' requested(%d)\n",
		      id, ret);
		return ret;
	}

	uc_pdata = dev_get_uclass_plat(dev);

	ops = rproc_get_ops(dev);
	if (!ops) {
		debug("%s driver has no ops?\n", dev->name);
		return -EINVAL;
	}

	debug("Loading to '%s' from address 0x%08lX size of %lu bytes\n",
	      uc_pdata->name, addr, size);
	if (ops->load)
		return ops->load(dev, addr, size);

	debug("%s: data corruption?? mandatory function is missing!\n",
	      dev->name);

	return -EINVAL;
};

/*
 * Completely internal helper enums..
 * Keeping this isolated helps this code evolve independent of other
 * parts..
 */
enum rproc_ops {
	RPROC_START,
	RPROC_STOP,
	RPROC_RESET,
	RPROC_PING,
	RPROC_RUNNING,
};

/**
 * _rproc_ops_wrapper() - wrapper for invoking remote proc driver callback
 * @id:		id of the remote processor
 * @op:		one of rproc_ops that indicate what operation to invoke
 *
 * Most of the checks and verification for remoteproc operations are more
 * or less same for almost all operations. This allows us to put a wrapper
 * and use the common checks to allow the driver to function appropriately.
 *
 * Return: 0 if all ok, else appropriate error value.
 */
static int _rproc_ops_wrapper(int id, enum rproc_ops op)
{
	struct udevice *dev = NULL;
	struct dm_rproc_uclass_pdata *uc_pdata;
	const struct dm_rproc_ops *ops;
	int (*fn)(struct udevice *dev);
	bool mandatory = false;
	char *op_str;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_REMOTEPROC, id, &dev);
	if (ret) {
		debug("Unknown remote processor id '%d' requested(%d)\n",
		      id, ret);
		return ret;
	}

	uc_pdata = dev_get_uclass_plat(dev);

	ops = rproc_get_ops(dev);
	if (!ops) {
		debug("%s driver has no ops?\n", dev->name);
		return -EINVAL;
	}
	switch (op) {
	case RPROC_START:
		fn = ops->start;
		mandatory = true;
		op_str = "Starting";
		break;
	case RPROC_STOP:
		fn = ops->stop;
		op_str = "Stopping";
		break;
	case RPROC_RESET:
		fn = ops->reset;
		op_str = "Resetting";
		break;
	case RPROC_RUNNING:
		fn = ops->is_running;
		op_str = "Checking if running:";
		break;
	case RPROC_PING:
		fn = ops->ping;
		op_str = "Pinging";
		break;
	default:
		debug("what is '%d' operation??\n", op);
		return -EINVAL;
	}

	debug("%s %s...\n", op_str, uc_pdata->name);
	if (fn)
		return fn(dev);

	if (mandatory)
		debug("%s: data corruption?? mandatory function is missing!\n",
		      dev->name);

	return -ENOSYS;
}

int rproc_start(int id)
{
	return _rproc_ops_wrapper(id, RPROC_START);
};

int rproc_stop(int id)
{
	return _rproc_ops_wrapper(id, RPROC_STOP);
};

int rproc_reset(int id)
{
	return _rproc_ops_wrapper(id, RPROC_RESET);
};

int rproc_ping(int id)
{
	return _rproc_ops_wrapper(id, RPROC_PING);
};

int rproc_is_running(int id)
{
	return _rproc_ops_wrapper(id, RPROC_RUNNING);
};


static int handle_trace(struct udevice *dev, struct fw_rsc_trace *rsc,
			int offset, int avail)
{
	if (sizeof(*rsc) > avail) {
		debug("trace rsc is truncated\n");
		return -EINVAL;
	}

	/*
	 * make sure reserved bytes are zeroes
	 */
	if (rsc->reserved) {
		debug("trace rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	debug("trace rsc: da 0x%x, len 0x%x\n", rsc->da, rsc->len);

	return 0;
}

static int handle_devmem(struct udevice *dev, struct fw_rsc_devmem *rsc,
			 int offset, int avail)
{
	struct rproc_mem_entry *mapping;

	if (sizeof(*rsc) > avail) {
		debug("devmem rsc is truncated\n");
		return -EINVAL;
	}

	/*
	 * make sure reserved bytes are zeroes
	 */
	if (rsc->reserved) {
		debug("devmem rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	debug("devmem rsc: pa 0x%x, da 0x%x, len 0x%x\n",
	      rsc->pa, rsc->da, rsc->len);

	rproc_config_pagetable(dev, rsc->da, rsc->pa, rsc->len);

	mapping = kzalloc(sizeof(*mapping), GFP_KERNEL);
	if (!mapping)
		return -ENOMEM;

	/*
	 * We'll need this info later when we'll want to unmap everything
	 * (e.g. on shutdown).
	 *
	 * We can't trust the remote processor not to change the resource
	 * table, so we must maintain this info independently.
	 */
	mapping->dma = rsc->pa;
	mapping->da = rsc->da;
	mapping->len = rsc->len;
	rproc_add_res(dev, mapping);

	debug("mapped devmem pa 0x%x, da 0x%x, len 0x%x\n",
	      rsc->pa, rsc->da, rsc->len);

	return 0;
}

static int handle_carveout(struct udevice *dev, struct fw_rsc_carveout *rsc,
			   int offset, int avail)
{
	struct rproc_mem_entry *mapping;

	if (sizeof(*rsc) > avail) {
		debug("carveout rsc is truncated\n");
		return -EINVAL;
	}

	/*
	 * make sure reserved bytes are zeroes
	 */
	if (rsc->reserved) {
		debug("carveout rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	debug("carveout rsc: da %x, pa %x, len %x, flags %x\n",
	      rsc->da, rsc->pa, rsc->len, rsc->flags);

	rsc->pa = (uintptr_t)rproc_alloc_mem(dev, rsc->len, 8);
	if (!rsc->pa) {
		debug
		    ("failed to allocate carveout rsc: da %x, pa %x, len %x, flags %x\n",
		     rsc->da, rsc->pa, rsc->len, rsc->flags);
		return -ENOMEM;
	}
	rproc_config_pagetable(dev, rsc->da, rsc->pa, rsc->len);

	/*
	 * Ok, this is non-standard.
	 *
	 * Sometimes we can't rely on the generic iommu-based DMA API
	 * to dynamically allocate the device address and then set the IOMMU
	 * tables accordingly, because some remote processors might
	 * _require_ us to use hard coded device addresses that their
	 * firmware was compiled with.
	 *
	 * In this case, we must use the IOMMU API directly and map
	 * the memory to the device address as expected by the remote
	 * processor.
	 *
	 * Obviously such remote processor devices should not be configured
	 * to use the iommu-based DMA API: we expect 'dma' to contain the
	 * physical address in this case.
	 */
	mapping = kzalloc(sizeof(*mapping), GFP_KERNEL);
	if (!mapping)
		return -ENOMEM;

	/*
	 * We'll need this info later when we'll want to unmap
	 * everything (e.g. on shutdown).
	 *
	 * We can't trust the remote processor not to change the
	 * resource table, so we must maintain this info independently.
	 */
	mapping->dma = rsc->pa;
	mapping->da = rsc->da;
	mapping->len = rsc->len;
	rproc_add_res(dev, mapping);

	debug("carveout mapped 0x%x to 0x%x\n", rsc->da, rsc->pa);

	return 0;
}

#define RPROC_PAGE_SHIFT 12
#define RPROC_PAGE_SIZE  BIT(RPROC_PAGE_SHIFT)
#define RPROC_PAGE_ALIGN(x) (((x) + (RPROC_PAGE_SIZE - 1)) & ~(RPROC_PAGE_SIZE - 1))

static int alloc_vring(struct udevice *dev, struct fw_rsc_vdev *rsc, int i)
{
	struct fw_rsc_vdev_vring *vring = &rsc->vring[i];
	int size;
	int order;
	void *pa;

	debug("vdev rsc: vring%d: da %x, qsz %d, align %d\n",
	      i, vring->da, vring->num, vring->align);

	/*
	 * verify queue size and vring alignment are sane
	 */
	if (!vring->num || !vring->align) {
		debug("invalid qsz (%d) or alignment (%d)\n", vring->num,
		      vring->align);
		return -EINVAL;
	}

	/*
	 * actual size of vring (in bytes)
	 */
	size = RPROC_PAGE_ALIGN(vring_size(vring->num, vring->align));
	order = vring->align >> RPROC_PAGE_SHIFT;

	pa = rproc_alloc_mem(dev, size, order);
	if (!pa) {
		debug("failed to allocate vring rsc\n");
		return -ENOMEM;
	}
	debug("alloc_mem(%#x, %d): %p\n", size, order, pa);
	vring->da = (uintptr_t)pa;

	return 0;
}

static int handle_vdev(struct udevice *dev, struct fw_rsc_vdev *rsc,
		       int offset, int avail)
{
	int i, ret;
	void *pa;

	/*
	 * make sure resource isn't truncated
	 */
	if (sizeof(*rsc) + rsc->num_of_vrings * sizeof(struct fw_rsc_vdev_vring)
	    + rsc->config_len > avail) {
		debug("vdev rsc is truncated\n");
		return -EINVAL;
	}

	/*
	 * make sure reserved bytes are zeroes
	 */
	if (rsc->reserved[0] || rsc->reserved[1]) {
		debug("vdev rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	debug("vdev rsc: id %d, dfeatures %x, cfg len %d, %d vrings\n",
	      rsc->id, rsc->dfeatures, rsc->config_len, rsc->num_of_vrings);

	/*
	 * we currently support only two vrings per rvdev
	 */
	if (rsc->num_of_vrings > 2) {
		debug("too many vrings: %d\n", rsc->num_of_vrings);
		return -EINVAL;
	}

	/*
	 * allocate the vrings
	 */
	for (i = 0; i < rsc->num_of_vrings; i++) {
		ret = alloc_vring(dev, rsc, i);
		if (ret)
			goto alloc_error;
	}

	pa = rproc_alloc_mem(dev, RPMSG_TOTAL_BUF_SPACE, 6);
	if (!pa) {
		debug("failed to allocate vdev rsc\n");
		return -ENOMEM;
	}
	debug("vring buffer alloc_mem(%#x, 6): %p\n", RPMSG_TOTAL_BUF_SPACE,
	      pa);

	return 0;

 alloc_error:
	return ret;
}

/*
 * A lookup table for resource handlers. The indices are defined in
 * enum fw_resource_type.
 */
static handle_resource_t loading_handlers[RSC_LAST] = {
	[RSC_CARVEOUT] = (handle_resource_t)handle_carveout,
	[RSC_DEVMEM] = (handle_resource_t)handle_devmem,
	[RSC_TRACE] = (handle_resource_t)handle_trace,
	[RSC_VDEV] = (handle_resource_t)handle_vdev,
};

/*
 * handle firmware resource entries before booting the remote processor
 */
static int handle_resources(struct udevice *dev, int len,
			    handle_resource_t handlers[RSC_LAST])
{
	handle_resource_t handler;
	int ret = 0, i;

	for (i = 0; i < rsc_table->num; i++) {
		int offset = rsc_table->offset[i];
		struct fw_rsc_hdr *hdr = (void *)rsc_table + offset;
		int avail = len - offset - sizeof(*hdr);
		void *rsc = (void *)hdr + sizeof(*hdr);

		/*
		 * make sure table isn't truncated
		 */
		if (avail < 0) {
			debug("rsc table is truncated\n");
			return -EINVAL;
		}

		debug("rsc: type %d\n", hdr->type);

		if (hdr->type >= RSC_LAST) {
			debug("unsupported resource %d\n", hdr->type);
			continue;
		}

		handler = handlers[hdr->type];
		if (!handler)
			continue;

		ret = handler(dev, rsc, offset + sizeof(*hdr), avail);
		if (ret)
			break;
	}

	return ret;
}

static int
handle_intmem_to_l3_mapping(struct udevice *dev,
			    struct rproc_intmem_to_l3_mapping *l3_mapping)
{
	u32 i = 0;

	for (i = 0; i < l3_mapping->num_entries; i++) {
		struct l3_map *curr_map = &l3_mapping->mappings[i];
		struct rproc_mem_entry *mapping;

		mapping = kzalloc(sizeof(*mapping), GFP_KERNEL);
		if (!mapping)
			return -ENOMEM;

		mapping->dma = curr_map->l3_addr;
		mapping->da = curr_map->priv_addr;
		mapping->len = curr_map->len;
		rproc_add_res(dev, mapping);
	}

	return 0;
}

static Elf32_Shdr *rproc_find_table(unsigned int addr)
{
	Elf32_Ehdr *ehdr;	/* Elf header structure pointer */
	Elf32_Shdr *shdr;	/* Section header structure pointer */
	Elf32_Shdr sectionheader;
	int i;
	u8 *elf_data;
	char *name_table;
	struct resource_table *ptable;

	ehdr = (Elf32_Ehdr *)(uintptr_t)addr;
	elf_data = (u8 *)ehdr;
	shdr = (Elf32_Shdr *)(elf_data + ehdr->e_shoff);
	memcpy(&sectionheader, &shdr[ehdr->e_shstrndx], sizeof(sectionheader));
	name_table = (char *)(elf_data + sectionheader.sh_offset);

	for (i = 0; i < ehdr->e_shnum; i++, shdr++) {
		memcpy(&sectionheader, shdr, sizeof(sectionheader));
		u32 size = sectionheader.sh_size;
		u32 offset = sectionheader.sh_offset;

		if (strcmp
		    (name_table + sectionheader.sh_name, ".resource_table"))
			continue;

		ptable = (struct resource_table *)(elf_data + offset);

		/*
		 * make sure table has at least the header
		 */
		if (sizeof(struct resource_table) > size) {
			debug("header-less resource table\n");
			return NULL;
		}

		/*
		 * we don't support any version beyond the first
		 */
		if (ptable->ver != 1) {
			debug("unsupported fw ver: %d\n", ptable->ver);
			return NULL;
		}

		/*
		 * make sure reserved bytes are zeroes
		 */
		if (ptable->reserved[0] || ptable->reserved[1]) {
			debug("non zero reserved bytes\n");
			return NULL;
		}

		/*
		 * make sure the offsets array isn't truncated
		 */
		if (ptable->num * sizeof(ptable->offset[0]) +
		    sizeof(struct resource_table) > size) {
			debug("resource table incomplete\n");
			return NULL;
		}

		return shdr;
	}

	return NULL;
}

struct resource_table *rproc_find_resource_table(struct udevice *dev,
						 unsigned int addr,
						 int *tablesz)
{
	Elf32_Shdr *shdr;
	Elf32_Shdr sectionheader;
	struct resource_table *ptable;
	u8 *elf_data = (u8 *)(uintptr_t)addr;

	shdr = rproc_find_table(addr);
	if (!shdr) {
		debug("%s: failed to get resource section header\n", __func__);
		return NULL;
	}

	memcpy(&sectionheader, shdr, sizeof(sectionheader));
	ptable = (struct resource_table *)(elf_data + sectionheader.sh_offset);
	if (tablesz)
		*tablesz = sectionheader.sh_size;

	return ptable;
}

unsigned long rproc_parse_resource_table(struct udevice *dev, struct rproc *cfg)
{
	struct resource_table *ptable = NULL;
	int tablesz;
	int ret;
	unsigned long addr;

	addr = cfg->load_addr;

	ptable = rproc_find_resource_table(dev, addr, &tablesz);
	if (!ptable) {
		debug("%s : failed to find resource table\n", __func__);
		return 0;
	}

	debug("%s : found resource table\n", __func__);
	rsc_table = kzalloc(tablesz, GFP_KERNEL);
	if (!rsc_table) {
		debug("resource table alloc failed!\n");
		return 0;
	}

	/*
	 * Copy the resource table into a local buffer before handling the
	 * resource table.
	 */
	memcpy(rsc_table, ptable, tablesz);
	if (cfg->intmem_to_l3_mapping)
		handle_intmem_to_l3_mapping(dev, cfg->intmem_to_l3_mapping);
	ret = handle_resources(dev, tablesz, loading_handlers);
	if (ret) {
		debug("handle_resources failed: %d\n", ret);
		return 0;
	}

	/*
	 * Instead of trying to mimic the kernel flow of copying the
	 * processed resource table into its post ELF load location in DDR
	 * copying it into its original location.
	 */
	memcpy(ptable, rsc_table, tablesz);
	free(rsc_table);
	rsc_table = NULL;

	return 1;
}
