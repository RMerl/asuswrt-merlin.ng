/*
 * Copyright (C) 2012 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/shmem_fs.h>
#include <drm/drmP.h>
#include "armada_drm.h"
#include "armada_gem.h"
#include <drm/armada_drm.h>
#include "armada_ioctlP.h"

static int armada_gem_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct armada_gem_object *obj = drm_to_armada_gem(vma->vm_private_data);
	unsigned long addr = (unsigned long)vmf->virtual_address;
	unsigned long pfn = obj->phys_addr >> PAGE_SHIFT;
	int ret;

	pfn += (addr - vma->vm_start) >> PAGE_SHIFT;
	ret = vm_insert_pfn(vma, addr, pfn);

	switch (ret) {
	case 0:
	case -EBUSY:
		return VM_FAULT_NOPAGE;
	case -ENOMEM:
		return VM_FAULT_OOM;
	default:
		return VM_FAULT_SIGBUS;
	}
}

const struct vm_operations_struct armada_gem_vm_ops = {
	.fault	= armada_gem_vm_fault,
	.open	= drm_gem_vm_open,
	.close	= drm_gem_vm_close,
};

static size_t roundup_gem_size(size_t size)
{
	return roundup(size, PAGE_SIZE);
}

/* dev->struct_mutex is held here */
void armada_gem_free_object(struct drm_gem_object *obj)
{
	struct armada_gem_object *dobj = drm_to_armada_gem(obj);

	DRM_DEBUG_DRIVER("release obj %p\n", dobj);

	drm_gem_free_mmap_offset(&dobj->obj);

	if (dobj->page) {
		/* page backed memory */
		unsigned int order = get_order(dobj->obj.size);
		__free_pages(dobj->page, order);
	} else if (dobj->linear) {
		/* linear backed memory */
		drm_mm_remove_node(dobj->linear);
		kfree(dobj->linear);
		if (dobj->addr)
			iounmap(dobj->addr);
	}

	if (dobj->obj.import_attach) {
		/* We only ever display imported data */
		dma_buf_unmap_attachment(dobj->obj.import_attach, dobj->sgt,
					 DMA_TO_DEVICE);
		drm_prime_gem_destroy(&dobj->obj, NULL);
	}

	drm_gem_object_release(&dobj->obj);

	kfree(dobj);
}

int
armada_gem_linear_back(struct drm_device *dev, struct armada_gem_object *obj)
{
	struct armada_private *priv = dev->dev_private;
	size_t size = obj->obj.size;

	if (obj->page || obj->linear)
		return 0;

	/*
	 * If it is a small allocation (typically cursor, which will
	 * be 32x64 or 64x32 ARGB pixels) try to get it from the system.
	 * Framebuffers will never be this small (our minimum size for
	 * framebuffers is larger than this anyway.)  Such objects are
	 * only accessed by the CPU so we don't need any special handing
	 * here.
	 */
	if (size <= 8192) {
		unsigned int order = get_order(size);
		struct page *p = alloc_pages(GFP_KERNEL, order);

		if (p) {
			obj->addr = page_address(p);
			obj->phys_addr = page_to_phys(p);
			obj->page = p;

			memset(obj->addr, 0, PAGE_ALIGN(size));
		}
	}

	/*
	 * We could grab something from CMA if it's enabled, but that
	 * involves building in a problem:
	 *
	 * CMA's interface uses dma_alloc_coherent(), which provides us
	 * with an CPU virtual address and a device address.
	 *
	 * The CPU virtual address may be either an address in the kernel
	 * direct mapped region (for example, as it would be on x86) or
	 * it may be remapped into another part of kernel memory space
	 * (eg, as it would be on ARM.)  This means virt_to_phys() on the
	 * returned virtual address is invalid depending on the architecture
	 * implementation.
	 *
	 * The device address may also not be a physical address; it may
	 * be that there is some kind of remapping between the device and
	 * system RAM, which makes the use of the device address also
	 * unsafe to re-use as a physical address.
	 *
	 * This makes DRM usage of dma_alloc_coherent() in a generic way
	 * at best very questionable and unsafe.
	 */

	/* Otherwise, grab it from our linear allocation */
	if (!obj->page) {
		struct drm_mm_node *node;
		unsigned align = min_t(unsigned, size, SZ_2M);
		void __iomem *ptr;
		int ret;

		node = kzalloc(sizeof(*node), GFP_KERNEL);
		if (!node)
			return -ENOSPC;

		mutex_lock(&dev->struct_mutex);
		ret = drm_mm_insert_node(&priv->linear, node, size, align,
					 DRM_MM_SEARCH_DEFAULT);
		mutex_unlock(&dev->struct_mutex);
		if (ret) {
			kfree(node);
			return ret;
		}

		obj->linear = node;

		/* Ensure that the memory we're returning is cleared. */
		ptr = ioremap_wc(obj->linear->start, size);
		if (!ptr) {
			mutex_lock(&dev->struct_mutex);
			drm_mm_remove_node(obj->linear);
			mutex_unlock(&dev->struct_mutex);
			kfree(obj->linear);
			obj->linear = NULL;
			return -ENOMEM;
		}

		memset_io(ptr, 0, size);
		iounmap(ptr);

		obj->phys_addr = obj->linear->start;
		obj->dev_addr = obj->linear->start;
	}

	DRM_DEBUG_DRIVER("obj %p phys %#llx dev %#llx\n", obj,
			 (unsigned long long)obj->phys_addr,
			 (unsigned long long)obj->dev_addr);

	return 0;
}

void *
armada_gem_map_object(struct drm_device *dev, struct armada_gem_object *dobj)
{
	/* only linear objects need to be ioremap'd */
	if (!dobj->addr && dobj->linear)
		dobj->addr = ioremap_wc(dobj->phys_addr, dobj->obj.size);
	return dobj->addr;
}

struct armada_gem_object *
armada_gem_alloc_private_object(struct drm_device *dev, size_t size)
{
	struct armada_gem_object *obj;

	size = roundup_gem_size(size);

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return NULL;

	drm_gem_private_object_init(dev, &obj->obj, size);
	obj->dev_addr = DMA_ERROR_CODE;

	DRM_DEBUG_DRIVER("alloc private obj %p size %zu\n", obj, size);

	return obj;
}

struct armada_gem_object *armada_gem_alloc_object(struct drm_device *dev,
	size_t size)
{
	struct armada_gem_object *obj;
	struct address_space *mapping;

	size = roundup_gem_size(size);

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return NULL;

	if (drm_gem_object_init(dev, &obj->obj, size)) {
		kfree(obj);
		return NULL;
	}

	obj->dev_addr = DMA_ERROR_CODE;

	mapping = file_inode(obj->obj.filp)->i_mapping;
	mapping_set_gfp_mask(mapping, GFP_HIGHUSER | __GFP_RECLAIMABLE);

	DRM_DEBUG_DRIVER("alloc obj %p size %zu\n", obj, size);

	return obj;
}

/* Dumb alloc support */
int armada_gem_dumb_create(struct drm_file *file, struct drm_device *dev,
	struct drm_mode_create_dumb *args)
{
	struct armada_gem_object *dobj;
	u32 handle;
	size_t size;
	int ret;

	args->pitch = armada_pitch(args->width, args->bpp);
	args->size = size = args->pitch * args->height;

	dobj = armada_gem_alloc_private_object(dev, size);
	if (dobj == NULL)
		return -ENOMEM;

	ret = armada_gem_linear_back(dev, dobj);
	if (ret)
		goto err;

	ret = drm_gem_handle_create(file, &dobj->obj, &handle);
	if (ret)
		goto err;

	args->handle = handle;

	/* drop reference from allocate - handle holds it now */
	DRM_DEBUG_DRIVER("obj %p size %zu handle %#x\n", dobj, size, handle);
 err:
	drm_gem_object_unreference_unlocked(&dobj->obj);
	return ret;
}

int armada_gem_dumb_map_offset(struct drm_file *file, struct drm_device *dev,
	uint32_t handle, uint64_t *offset)
{
	struct armada_gem_object *obj;
	int ret = 0;

	mutex_lock(&dev->struct_mutex);
	obj = armada_gem_object_lookup(dev, file, handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object\n");
		ret = -EINVAL;
		goto err_unlock;
	}

	/* Don't allow imported objects to be mapped */
	if (obj->obj.import_attach) {
		ret = -EINVAL;
		goto err_unlock;
	}

	ret = drm_gem_create_mmap_offset(&obj->obj);
	if (ret == 0) {
		*offset = drm_vma_node_offset_addr(&obj->obj.vma_node);
		DRM_DEBUG_DRIVER("handle %#x offset %llx\n", handle, *offset);
	}

	drm_gem_object_unreference(&obj->obj);
 err_unlock:
	mutex_unlock(&dev->struct_mutex);

	return ret;
}

int armada_gem_dumb_destroy(struct drm_file *file, struct drm_device *dev,
	uint32_t handle)
{
	return drm_gem_handle_delete(file, handle);
}

/* Private driver gem ioctls */
int armada_gem_create_ioctl(struct drm_device *dev, void *data,
	struct drm_file *file)
{
	struct drm_armada_gem_create *args = data;
	struct armada_gem_object *dobj;
	size_t size;
	u32 handle;
	int ret;

	if (args->size == 0)
		return -ENOMEM;

	size = args->size;

	dobj = armada_gem_alloc_object(dev, size);
	if (dobj == NULL)
		return -ENOMEM;

	ret = drm_gem_handle_create(file, &dobj->obj, &handle);
	if (ret)
		goto err;

	args->handle = handle;

	/* drop reference from allocate - handle holds it now */
	DRM_DEBUG_DRIVER("obj %p size %zu handle %#x\n", dobj, size, handle);
 err:
	drm_gem_object_unreference_unlocked(&dobj->obj);
	return ret;
}

/* Map a shmem-backed object into process memory space */
int armada_gem_mmap_ioctl(struct drm_device *dev, void *data,
	struct drm_file *file)
{
	struct drm_armada_gem_mmap *args = data;
	struct armada_gem_object *dobj;
	unsigned long addr;

	dobj = armada_gem_object_lookup(dev, file, args->handle);
	if (dobj == NULL)
		return -ENOENT;

	if (!dobj->obj.filp) {
		drm_gem_object_unreference(&dobj->obj);
		return -EINVAL;
	}

	addr = vm_mmap(dobj->obj.filp, 0, args->size, PROT_READ | PROT_WRITE,
		       MAP_SHARED, args->offset);
	drm_gem_object_unreference(&dobj->obj);
	if (IS_ERR_VALUE(addr))
		return addr;

	args->addr = addr;

	return 0;
}

int armada_gem_pwrite_ioctl(struct drm_device *dev, void *data,
	struct drm_file *file)
{
	struct drm_armada_gem_pwrite *args = data;
	struct armada_gem_object *dobj;
	char __user *ptr;
	int ret;

	DRM_DEBUG_DRIVER("handle %u off %u size %u ptr 0x%llx\n",
		args->handle, args->offset, args->size, args->ptr);

	if (args->size == 0)
		return 0;

	ptr = (char __user *)(uintptr_t)args->ptr;

	if (!access_ok(VERIFY_READ, ptr, args->size))
		return -EFAULT;

	ret = fault_in_multipages_readable(ptr, args->size);
	if (ret)
		return ret;

	dobj = armada_gem_object_lookup(dev, file, args->handle);
	if (dobj == NULL)
		return -ENOENT;

	/* Must be a kernel-mapped object */
	if (!dobj->addr)
		return -EINVAL;

	if (args->offset > dobj->obj.size ||
	    args->size > dobj->obj.size - args->offset) {
		DRM_ERROR("invalid size: object size %u\n", dobj->obj.size);
		ret = -EINVAL;
		goto unref;
	}

	if (copy_from_user(dobj->addr + args->offset, ptr, args->size)) {
		ret = -EFAULT;
	} else if (dobj->update) {
		dobj->update(dobj->update_data);
		ret = 0;
	}

 unref:
	drm_gem_object_unreference_unlocked(&dobj->obj);
	return ret;
}

/* Prime support */
struct sg_table *
armada_gem_prime_map_dma_buf(struct dma_buf_attachment *attach,
	enum dma_data_direction dir)
{
	struct drm_gem_object *obj = attach->dmabuf->priv;
	struct armada_gem_object *dobj = drm_to_armada_gem(obj);
	struct scatterlist *sg;
	struct sg_table *sgt;
	int i, num;

	sgt = kmalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return NULL;

	if (dobj->obj.filp) {
		struct address_space *mapping;
		int count;

		count = dobj->obj.size / PAGE_SIZE;
		if (sg_alloc_table(sgt, count, GFP_KERNEL))
			goto free_sgt;

		mapping = file_inode(dobj->obj.filp)->i_mapping;

		for_each_sg(sgt->sgl, sg, count, i) {
			struct page *page;

			page = shmem_read_mapping_page(mapping, i);
			if (IS_ERR(page)) {
				num = i;
				goto release;
			}

			sg_set_page(sg, page, PAGE_SIZE, 0);
		}

		if (dma_map_sg(attach->dev, sgt->sgl, sgt->nents, dir) == 0) {
			num = sgt->nents;
			goto release;
		}
	} else if (dobj->page) {
		/* Single contiguous page */
		if (sg_alloc_table(sgt, 1, GFP_KERNEL))
			goto free_sgt;

		sg_set_page(sgt->sgl, dobj->page, dobj->obj.size, 0);

		if (dma_map_sg(attach->dev, sgt->sgl, sgt->nents, dir) == 0)
			goto free_table;
	} else if (dobj->linear) {
		/* Single contiguous physical region - no struct page */
		if (sg_alloc_table(sgt, 1, GFP_KERNEL))
			goto free_sgt;
		sg_dma_address(sgt->sgl) = dobj->dev_addr;
		sg_dma_len(sgt->sgl) = dobj->obj.size;
	} else {
		goto free_sgt;
	}
	return sgt;

 release:
	for_each_sg(sgt->sgl, sg, num, i)
		page_cache_release(sg_page(sg));
 free_table:
	sg_free_table(sgt);
 free_sgt:
	kfree(sgt);
	return NULL;
}

static void armada_gem_prime_unmap_dma_buf(struct dma_buf_attachment *attach,
	struct sg_table *sgt, enum dma_data_direction dir)
{
	struct drm_gem_object *obj = attach->dmabuf->priv;
	struct armada_gem_object *dobj = drm_to_armada_gem(obj);
	int i;

	if (!dobj->linear)
		dma_unmap_sg(attach->dev, sgt->sgl, sgt->nents, dir);

	if (dobj->obj.filp) {
		struct scatterlist *sg;
		for_each_sg(sgt->sgl, sg, sgt->nents, i)
			page_cache_release(sg_page(sg));
	}

	sg_free_table(sgt);
	kfree(sgt);
}

static void *armada_gem_dmabuf_no_kmap(struct dma_buf *buf, unsigned long n)
{
	return NULL;
}

static void
armada_gem_dmabuf_no_kunmap(struct dma_buf *buf, unsigned long n, void *addr)
{
}

static int
armada_gem_dmabuf_mmap(struct dma_buf *buf, struct vm_area_struct *vma)
{
	return -EINVAL;
}

static const struct dma_buf_ops armada_gem_prime_dmabuf_ops = {
	.map_dma_buf	= armada_gem_prime_map_dma_buf,
	.unmap_dma_buf	= armada_gem_prime_unmap_dma_buf,
	.release	= drm_gem_dmabuf_release,
	.kmap_atomic	= armada_gem_dmabuf_no_kmap,
	.kunmap_atomic	= armada_gem_dmabuf_no_kunmap,
	.kmap		= armada_gem_dmabuf_no_kmap,
	.kunmap		= armada_gem_dmabuf_no_kunmap,
	.mmap		= armada_gem_dmabuf_mmap,
};

struct dma_buf *
armada_gem_prime_export(struct drm_device *dev, struct drm_gem_object *obj,
	int flags)
{
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	exp_info.ops = &armada_gem_prime_dmabuf_ops;
	exp_info.size = obj->size;
	exp_info.flags = O_RDWR;
	exp_info.priv = obj;

	return dma_buf_export(&exp_info);
}

struct drm_gem_object *
armada_gem_prime_import(struct drm_device *dev, struct dma_buf *buf)
{
	struct dma_buf_attachment *attach;
	struct armada_gem_object *dobj;

	if (buf->ops == &armada_gem_prime_dmabuf_ops) {
		struct drm_gem_object *obj = buf->priv;
		if (obj->dev == dev) {
			/*
			 * Importing our own dmabuf(s) increases the
			 * refcount on the gem object itself.
			 */
			drm_gem_object_reference(obj);
			return obj;
		}
	}

	attach = dma_buf_attach(buf, dev->dev);
	if (IS_ERR(attach))
		return ERR_CAST(attach);

	dobj = armada_gem_alloc_private_object(dev, buf->size);
	if (!dobj) {
		dma_buf_detach(buf, attach);
		return ERR_PTR(-ENOMEM);
	}

	dobj->obj.import_attach = attach;
	get_dma_buf(buf);

	/*
	 * Don't call dma_buf_map_attachment() here - it maps the
	 * scatterlist immediately for DMA, and this is not always
	 * an appropriate thing to do.
	 */
	return &dobj->obj;
}

int armada_gem_map_import(struct armada_gem_object *dobj)
{
	int ret;

	dobj->sgt = dma_buf_map_attachment(dobj->obj.import_attach,
					  DMA_TO_DEVICE);
	if (!dobj->sgt) {
		DRM_ERROR("dma_buf_map_attachment() returned NULL\n");
		return -EINVAL;
	}
	if (IS_ERR(dobj->sgt)) {
		ret = PTR_ERR(dobj->sgt);
		dobj->sgt = NULL;
		DRM_ERROR("dma_buf_map_attachment() error: %d\n", ret);
		return ret;
	}
	if (dobj->sgt->nents > 1) {
		DRM_ERROR("dma_buf_map_attachment() returned an (unsupported) scattered list\n");
		return -EINVAL;
	}
	if (sg_dma_len(dobj->sgt->sgl) < dobj->obj.size) {
		DRM_ERROR("dma_buf_map_attachment() returned a small buffer\n");
		return -EINVAL;
	}
	dobj->dev_addr = sg_dma_address(dobj->sgt->sgl);
	return 0;
}
