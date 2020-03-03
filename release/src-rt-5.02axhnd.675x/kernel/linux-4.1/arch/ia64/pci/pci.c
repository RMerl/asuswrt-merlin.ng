/*
 * pci.c - Low-Level PCI Access in IA-64
 *
 * Derived from bios32.c of i386 tree.
 *
 * (c) Copyright 2002, 2005 Hewlett-Packard Development Company, L.P.
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 *	Bjorn Helgaas <bjorn.helgaas@hp.com>
 * Copyright (C) 2004 Silicon Graphics, Inc.
 *
 * Note: Above list of copyright holders is incomplete...
 */

#include <linux/acpi.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/pci-acpi.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/bootmem.h>
#include <linux/export.h>

#include <asm/machvec.h>
#include <asm/page.h>
#include <asm/io.h>
#include <asm/sal.h>
#include <asm/smp.h>
#include <asm/irq.h>
#include <asm/hw_irq.h>

/*
 * Low-level SAL-based PCI configuration access functions. Note that SAL
 * calls are already serialized (via sal_lock), so we don't need another
 * synchronization mechanism here.
 */

#define PCI_SAL_ADDRESS(seg, bus, devfn, reg)		\
	(((u64) seg << 24) | (bus << 16) | (devfn << 8) | (reg))

/* SAL 3.2 adds support for extended config space. */

#define PCI_SAL_EXT_ADDRESS(seg, bus, devfn, reg)	\
	(((u64) seg << 28) | (bus << 20) | (devfn << 12) | (reg))

int raw_pci_read(unsigned int seg, unsigned int bus, unsigned int devfn,
	      int reg, int len, u32 *value)
{
	u64 addr, data = 0;
	int mode, result;

	if (!value || (seg > 65535) || (bus > 255) || (devfn > 255) || (reg > 4095))
		return -EINVAL;

	if ((seg | reg) <= 255) {
		addr = PCI_SAL_ADDRESS(seg, bus, devfn, reg);
		mode = 0;
	} else if (sal_revision >= SAL_VERSION_CODE(3,2)) {
		addr = PCI_SAL_EXT_ADDRESS(seg, bus, devfn, reg);
		mode = 1;
	} else {
		return -EINVAL;
	}

	result = ia64_sal_pci_config_read(addr, mode, len, &data);
	if (result != 0)
		return -EINVAL;

	*value = (u32) data;
	return 0;
}

int raw_pci_write(unsigned int seg, unsigned int bus, unsigned int devfn,
	       int reg, int len, u32 value)
{
	u64 addr;
	int mode, result;

	if ((seg > 65535) || (bus > 255) || (devfn > 255) || (reg > 4095))
		return -EINVAL;

	if ((seg | reg) <= 255) {
		addr = PCI_SAL_ADDRESS(seg, bus, devfn, reg);
		mode = 0;
	} else if (sal_revision >= SAL_VERSION_CODE(3,2)) {
		addr = PCI_SAL_EXT_ADDRESS(seg, bus, devfn, reg);
		mode = 1;
	} else {
		return -EINVAL;
	}
	result = ia64_sal_pci_config_write(addr, mode, len, value);
	if (result != 0)
		return -EINVAL;
	return 0;
}

static int pci_read(struct pci_bus *bus, unsigned int devfn, int where,
							int size, u32 *value)
{
	return raw_pci_read(pci_domain_nr(bus), bus->number,
				 devfn, where, size, value);
}

static int pci_write(struct pci_bus *bus, unsigned int devfn, int where,
							int size, u32 value)
{
	return raw_pci_write(pci_domain_nr(bus), bus->number,
				  devfn, where, size, value);
}

struct pci_ops pci_root_ops = {
	.read = pci_read,
	.write = pci_write,
};

/* Called by ACPI when it finds a new root bus.  */

static struct pci_controller *alloc_pci_controller(int seg)
{
	struct pci_controller *controller;

	controller = kzalloc(sizeof(*controller), GFP_KERNEL);
	if (!controller)
		return NULL;

	controller->segment = seg;
	return controller;
}

struct pci_root_info {
	struct acpi_device *bridge;
	struct pci_controller *controller;
	struct list_head resources;
	struct resource *res;
	resource_size_t *res_offset;
	unsigned int res_num;
	struct list_head io_resources;
	char *name;
};

static unsigned int
new_space (u64 phys_base, int sparse)
{
	u64 mmio_base;
	int i;

	if (phys_base == 0)
		return 0;	/* legacy I/O port space */

	mmio_base = (u64) ioremap(phys_base, 0);
	for (i = 0; i < num_io_spaces; i++)
		if (io_space[i].mmio_base == mmio_base &&
		    io_space[i].sparse == sparse)
			return i;

	if (num_io_spaces == MAX_IO_SPACES) {
		pr_err("PCI: Too many IO port spaces "
			"(MAX_IO_SPACES=%lu)\n", MAX_IO_SPACES);
		return ~0;
	}

	i = num_io_spaces++;
	io_space[i].mmio_base = mmio_base;
	io_space[i].sparse = sparse;

	return i;
}

static u64 add_io_space(struct pci_root_info *info,
			struct acpi_resource_address64 *addr)
{
	struct iospace_resource *iospace;
	struct resource *resource;
	char *name;
	unsigned long base, min, max, base_port;
	unsigned int sparse = 0, space_nr, len;

	len = strlen(info->name) + 32;
	iospace = kzalloc(sizeof(*iospace) + len, GFP_KERNEL);
	if (!iospace) {
		dev_err(&info->bridge->dev,
				"PCI: No memory for %s I/O port space\n",
				info->name);
		goto out;
	}

	name = (char *)(iospace + 1);

	min = addr->address.minimum;
	max = min + addr->address.address_length - 1;
	if (addr->info.io.translation_type == ACPI_SPARSE_TRANSLATION)
		sparse = 1;

	space_nr = new_space(addr->address.translation_offset, sparse);
	if (space_nr == ~0)
		goto free_resource;

	base = __pa(io_space[space_nr].mmio_base);
	base_port = IO_SPACE_BASE(space_nr);
	snprintf(name, len, "%s I/O Ports %08lx-%08lx", info->name,
		base_port + min, base_port + max);

	/*
	 * The SDM guarantees the legacy 0-64K space is sparse, but if the
	 * mapping is done by the processor (not the bridge), ACPI may not
	 * mark it as sparse.
	 */
	if (space_nr == 0)
		sparse = 1;

	resource = &iospace->res;
	resource->name  = name;
	resource->flags = IORESOURCE_MEM;
	resource->start = base + (sparse ? IO_SPACE_SPARSE_ENCODING(min) : min);
	resource->end   = base + (sparse ? IO_SPACE_SPARSE_ENCODING(max) : max);
	if (insert_resource(&iomem_resource, resource)) {
		dev_err(&info->bridge->dev,
				"can't allocate host bridge io space resource  %pR\n",
				resource);
		goto free_resource;
	}

	list_add_tail(&iospace->list, &info->io_resources);
	return base_port;

free_resource:
	kfree(iospace);
out:
	return ~0;
}

static acpi_status resource_to_window(struct acpi_resource *resource,
				      struct acpi_resource_address64 *addr)
{
	acpi_status status;

	/*
	 * We're only interested in _CRS descriptors that are
	 *	- address space descriptors for memory or I/O space
	 *	- non-zero size
	 */
	status = acpi_resource_to_address64(resource, addr);
	if (ACPI_SUCCESS(status) &&
	    (addr->resource_type == ACPI_MEMORY_RANGE ||
	     addr->resource_type == ACPI_IO_RANGE) &&
	    addr->address.address_length)
		return AE_OK;

	return AE_ERROR;
}

static acpi_status count_window(struct acpi_resource *resource, void *data)
{
	unsigned int *windows = (unsigned int *) data;
	struct acpi_resource_address64 addr;
	acpi_status status;

	status = resource_to_window(resource, &addr);
	if (ACPI_SUCCESS(status))
		(*windows)++;

	return AE_OK;
}

static acpi_status add_window(struct acpi_resource *res, void *data)
{
	struct pci_root_info *info = data;
	struct resource *resource;
	struct acpi_resource_address64 addr;
	acpi_status status;
	unsigned long flags, offset = 0;
	struct resource *root;

	/* Return AE_OK for non-window resources to keep scanning for more */
	status = resource_to_window(res, &addr);
	if (!ACPI_SUCCESS(status))
		return AE_OK;

	if (addr.resource_type == ACPI_MEMORY_RANGE) {
		flags = IORESOURCE_MEM;
		root = &iomem_resource;
		offset = addr.address.translation_offset;
	} else if (addr.resource_type == ACPI_IO_RANGE) {
		flags = IORESOURCE_IO;
		root = &ioport_resource;
		offset = add_io_space(info, &addr);
		if (offset == ~0)
			return AE_OK;
	} else
		return AE_OK;

	resource = &info->res[info->res_num];
	resource->name = info->name;
	resource->flags = flags;
	resource->start = addr.address.minimum + offset;
	resource->end = resource->start + addr.address.address_length - 1;
	info->res_offset[info->res_num] = offset;

	if (insert_resource(root, resource)) {
		dev_err(&info->bridge->dev,
			"can't allocate host bridge window %pR\n",
			resource);
	} else {
		if (offset)
			dev_info(&info->bridge->dev, "host bridge window %pR "
				 "(PCI address [%#llx-%#llx])\n",
				 resource,
				 resource->start - offset,
				 resource->end - offset);
		else
			dev_info(&info->bridge->dev,
				 "host bridge window %pR\n", resource);
	}
	/* HP's firmware has a hack to work around a Windows bug.
	 * Ignore these tiny memory ranges */
	if (!((resource->flags & IORESOURCE_MEM) &&
	      (resource->end - resource->start < 16)))
		pci_add_resource_offset(&info->resources, resource,
					info->res_offset[info->res_num]);

	info->res_num++;
	return AE_OK;
}

static void free_pci_root_info_res(struct pci_root_info *info)
{
	struct iospace_resource *iospace, *tmp;

	list_for_each_entry_safe(iospace, tmp, &info->io_resources, list)
		kfree(iospace);

	kfree(info->name);
	kfree(info->res);
	info->res = NULL;
	kfree(info->res_offset);
	info->res_offset = NULL;
	info->res_num = 0;
	kfree(info->controller);
	info->controller = NULL;
}

static void __release_pci_root_info(struct pci_root_info *info)
{
	int i;
	struct resource *res;
	struct iospace_resource *iospace;

	list_for_each_entry(iospace, &info->io_resources, list)
		release_resource(&iospace->res);

	for (i = 0; i < info->res_num; i++) {
		res = &info->res[i];

		if (!res->parent)
			continue;

		if (!(res->flags & (IORESOURCE_MEM | IORESOURCE_IO)))
			continue;

		release_resource(res);
	}

	free_pci_root_info_res(info);
	kfree(info);
}

static void release_pci_root_info(struct pci_host_bridge *bridge)
{
	struct pci_root_info *info = bridge->release_data;

	__release_pci_root_info(info);
}

static int
probe_pci_root_info(struct pci_root_info *info, struct acpi_device *device,
		int busnum, int domain)
{
	char *name;

	name = kmalloc(16, GFP_KERNEL);
	if (!name)
		return -ENOMEM;

	sprintf(name, "PCI Bus %04x:%02x", domain, busnum);
	info->bridge = device;
	info->name = name;

	acpi_walk_resources(device->handle, METHOD_NAME__CRS, count_window,
			&info->res_num);
	if (info->res_num) {
		info->res =
			kzalloc_node(sizeof(*info->res) * info->res_num,
				     GFP_KERNEL, info->controller->node);
		if (!info->res) {
			kfree(name);
			return -ENOMEM;
		}

		info->res_offset =
			kzalloc_node(sizeof(*info->res_offset) * info->res_num,
					GFP_KERNEL, info->controller->node);
		if (!info->res_offset) {
			kfree(name);
			kfree(info->res);
			info->res = NULL;
			return -ENOMEM;
		}

		info->res_num = 0;
		acpi_walk_resources(device->handle, METHOD_NAME__CRS,
			add_window, info);
	} else
		kfree(name);

	return 0;
}

struct pci_bus *pci_acpi_scan_root(struct acpi_pci_root *root)
{
	struct acpi_device *device = root->device;
	int domain = root->segment;
	int bus = root->secondary.start;
	struct pci_controller *controller;
	struct pci_root_info *info = NULL;
	int busnum = root->secondary.start;
	struct pci_bus *pbus;
	int ret;

	controller = alloc_pci_controller(domain);
	if (!controller)
		return NULL;

	controller->companion = device;
	controller->node = acpi_get_node(device->handle);

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		dev_err(&device->dev,
				"pci_bus %04x:%02x: ignored (out of memory)\n",
				domain, busnum);
		kfree(controller);
		return NULL;
	}

	info->controller = controller;
	INIT_LIST_HEAD(&info->io_resources);
	INIT_LIST_HEAD(&info->resources);

	ret = probe_pci_root_info(info, device, busnum, domain);
	if (ret) {
		kfree(info->controller);
		kfree(info);
		return NULL;
	}
	/* insert busn resource at first */
	pci_add_resource(&info->resources, &root->secondary);
	/*
	 * See arch/x86/pci/acpi.c.
	 * The desired pci bus might already be scanned in a quirk. We
	 * should handle the case here, but it appears that IA64 hasn't
	 * such quirk. So we just ignore the case now.
	 */
	pbus = pci_create_root_bus(NULL, bus, &pci_root_ops, controller,
				   &info->resources);
	if (!pbus) {
		pci_free_resource_list(&info->resources);
		__release_pci_root_info(info);
		return NULL;
	}

	pci_set_host_bridge_release(to_pci_host_bridge(pbus->bridge),
			release_pci_root_info, info);
	pci_scan_child_bus(pbus);
	return pbus;
}

int pcibios_root_bridge_prepare(struct pci_host_bridge *bridge)
{
	/*
	 * We pass NULL as parent to pci_create_root_bus(), so if it is not NULL
	 * here, pci_create_root_bus() has been called by someone else and
	 * sysdata is likely to be different from what we expect.  Let it go in
	 * that case.
	 */
	if (!bridge->dev.parent) {
		struct pci_controller *controller = bridge->bus->sysdata;
		ACPI_COMPANION_SET(&bridge->dev, controller->companion);
	}
	return 0;
}

void pcibios_fixup_device_resources(struct pci_dev *dev)
{
	int idx;

	if (!dev->bus)
		return;

	for (idx = 0; idx < PCI_BRIDGE_RESOURCES; idx++) {
		struct resource *r = &dev->resource[idx];

		if (!r->flags || r->parent || !r->start)
			continue;

		pci_claim_resource(dev, idx);
	}
}
EXPORT_SYMBOL_GPL(pcibios_fixup_device_resources);

static void pcibios_fixup_bridge_resources(struct pci_dev *dev)
{
	int idx;

	if (!dev->bus)
		return;

	for (idx = PCI_BRIDGE_RESOURCES; idx < PCI_NUM_RESOURCES; idx++) {
		struct resource *r = &dev->resource[idx];

		if (!r->flags || r->parent || !r->start)
			continue;

		pci_claim_bridge_resource(dev, idx);
	}
}

/*
 *  Called after each bus is probed, but before its children are examined.
 */
void pcibios_fixup_bus(struct pci_bus *b)
{
	struct pci_dev *dev;

	if (b->self) {
		pci_read_bridge_bases(b);
		pcibios_fixup_bridge_resources(b->self);
	}
	list_for_each_entry(dev, &b->devices, bus_list)
		pcibios_fixup_device_resources(dev);
	platform_pci_fixup_bus(b);
}

void pcibios_add_bus(struct pci_bus *bus)
{
	acpi_pci_add_bus(bus);
}

void pcibios_remove_bus(struct pci_bus *bus)
{
	acpi_pci_remove_bus(bus);
}

void pcibios_set_master (struct pci_dev *dev)
{
	/* No special bus mastering setup handling */
}

int
pcibios_enable_device (struct pci_dev *dev, int mask)
{
	int ret;

	ret = pci_enable_resources(dev, mask);
	if (ret < 0)
		return ret;

	if (!dev->msi_enabled)
		return acpi_pci_irq_enable(dev);
	return 0;
}

void
pcibios_disable_device (struct pci_dev *dev)
{
	BUG_ON(atomic_read(&dev->enable_cnt));
	if (!dev->msi_enabled)
		acpi_pci_irq_disable(dev);
}

resource_size_t
pcibios_align_resource (void *data, const struct resource *res,
		        resource_size_t size, resource_size_t align)
{
	return res->start;
}

int
pci_mmap_page_range (struct pci_dev *dev, struct vm_area_struct *vma,
		     enum pci_mmap_state mmap_state, int write_combine)
{
	unsigned long size = vma->vm_end - vma->vm_start;
	pgprot_t prot;

	/*
	 * I/O space cannot be accessed via normal processor loads and
	 * stores on this platform.
	 */
	if (mmap_state == pci_mmap_io)
		/*
		 * XXX we could relax this for I/O spaces for which ACPI
		 * indicates that the space is 1-to-1 mapped.  But at the
		 * moment, we don't support multiple PCI address spaces and
		 * the legacy I/O space is not 1-to-1 mapped, so this is moot.
		 */
		return -EINVAL;

	if (!valid_mmap_phys_addr_range(vma->vm_pgoff, size))
		return -EINVAL;

	prot = phys_mem_access_prot(NULL, vma->vm_pgoff, size,
				    vma->vm_page_prot);

	/*
	 * If the user requested WC, the kernel uses UC or WC for this region,
	 * and the chipset supports WC, we can use WC. Otherwise, we have to
	 * use the same attribute the kernel uses.
	 */
	if (write_combine &&
	    ((pgprot_val(prot) & _PAGE_MA_MASK) == _PAGE_MA_UC ||
	     (pgprot_val(prot) & _PAGE_MA_MASK) == _PAGE_MA_WC) &&
	    efi_range_is_wc(vma->vm_start, vma->vm_end - vma->vm_start))
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	else
		vma->vm_page_prot = prot;

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			     vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

/**
 * ia64_pci_get_legacy_mem - generic legacy mem routine
 * @bus: bus to get legacy memory base address for
 *
 * Find the base of legacy memory for @bus.  This is typically the first
 * megabyte of bus address space for @bus or is simply 0 on platforms whose
 * chipsets support legacy I/O and memory routing.  Returns the base address
 * or an error pointer if an error occurred.
 *
 * This is the ia64 generic version of this routine.  Other platforms
 * are free to override it with a machine vector.
 */
char *ia64_pci_get_legacy_mem(struct pci_bus *bus)
{
	return (char *)__IA64_UNCACHED_OFFSET;
}

/**
 * pci_mmap_legacy_page_range - map legacy memory space to userland
 * @bus: bus whose legacy space we're mapping
 * @vma: vma passed in by mmap
 *
 * Map legacy memory space for this device back to userspace using a machine
 * vector to get the base address.
 */
int
pci_mmap_legacy_page_range(struct pci_bus *bus, struct vm_area_struct *vma,
			   enum pci_mmap_state mmap_state)
{
	unsigned long size = vma->vm_end - vma->vm_start;
	pgprot_t prot;
	char *addr;

	/* We only support mmap'ing of legacy memory space */
	if (mmap_state != pci_mmap_mem)
		return -ENOSYS;

	/*
	 * Avoid attribute aliasing.  See Documentation/ia64/aliasing.txt
	 * for more details.
	 */
	if (!valid_mmap_phys_addr_range(vma->vm_pgoff, size))
		return -EINVAL;
	prot = phys_mem_access_prot(NULL, vma->vm_pgoff, size,
				    vma->vm_page_prot);

	addr = pci_get_legacy_mem(bus);
	if (IS_ERR(addr))
		return PTR_ERR(addr);

	vma->vm_pgoff += (unsigned long)addr >> PAGE_SHIFT;
	vma->vm_page_prot = prot;

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			    size, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

/**
 * ia64_pci_legacy_read - read from legacy I/O space
 * @bus: bus to read
 * @port: legacy port value
 * @val: caller allocated storage for returned value
 * @size: number of bytes to read
 *
 * Simply reads @size bytes from @port and puts the result in @val.
 *
 * Again, this (and the write routine) are generic versions that can be
 * overridden by the platform.  This is necessary on platforms that don't
 * support legacy I/O routing or that hard fail on legacy I/O timeouts.
 */
int ia64_pci_legacy_read(struct pci_bus *bus, u16 port, u32 *val, u8 size)
{
	int ret = size;

	switch (size) {
	case 1:
		*val = inb(port);
		break;
	case 2:
		*val = inw(port);
		break;
	case 4:
		*val = inl(port);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * ia64_pci_legacy_write - perform a legacy I/O write
 * @bus: bus pointer
 * @port: port to write
 * @val: value to write
 * @size: number of bytes to write from @val
 *
 * Simply writes @size bytes of @val to @port.
 */
int ia64_pci_legacy_write(struct pci_bus *bus, u16 port, u32 val, u8 size)
{
	int ret = size;

	switch (size) {
	case 1:
		outb(val, port);
		break;
	case 2:
		outw(val, port);
		break;
	case 4:
		outl(val, port);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * set_pci_cacheline_size - determine cacheline size for PCI devices
 *
 * We want to use the line-size of the outer-most cache.  We assume
 * that this line-size is the same for all CPUs.
 *
 * Code mostly taken from arch/ia64/kernel/palinfo.c:cache_info().
 */
static void __init set_pci_dfl_cacheline_size(void)
{
	unsigned long levels, unique_caches;
	long status;
	pal_cache_config_info_t cci;

	status = ia64_pal_cache_summary(&levels, &unique_caches);
	if (status != 0) {
		pr_err("%s: ia64_pal_cache_summary() failed "
			"(status=%ld)\n", __func__, status);
		return;
	}

	status = ia64_pal_cache_config_info(levels - 1,
				/* cache_type (data_or_unified)= */ 2, &cci);
	if (status != 0) {
		pr_err("%s: ia64_pal_cache_config_info() failed "
			"(status=%ld)\n", __func__, status);
		return;
	}
	pci_dfl_cache_line_size = (1 << cci.pcci_line_size) / 4;
}

u64 ia64_dma_get_required_mask(struct device *dev)
{
	u32 low_totalram = ((max_pfn - 1) << PAGE_SHIFT);
	u32 high_totalram = ((max_pfn - 1) >> (32 - PAGE_SHIFT));
	u64 mask;

	if (!high_totalram) {
		/* convert to mask just covering totalram */
		low_totalram = (1 << (fls(low_totalram) - 1));
		low_totalram += low_totalram - 1;
		mask = low_totalram;
	} else {
		high_totalram = (1 << (fls(high_totalram) - 1));
		high_totalram += high_totalram - 1;
		mask = (((u64)high_totalram) << 32) + 0xffffffff;
	}
	return mask;
}
EXPORT_SYMBOL_GPL(ia64_dma_get_required_mask);

u64 dma_get_required_mask(struct device *dev)
{
	return platform_dma_get_required_mask(dev);
}
EXPORT_SYMBOL_GPL(dma_get_required_mask);

static int __init pcibios_init(void)
{
	set_pci_dfl_cacheline_size();
	return 0;
}

subsys_initcall(pcibios_init);
