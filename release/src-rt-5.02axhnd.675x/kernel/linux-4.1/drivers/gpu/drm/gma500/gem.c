/*
 *  psb GEM interface
 *
 * Copyright (c) 2011, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors: Alan Cox
 *
 * TODO:
 *	-	we need to work out if the MMU is relevant (eg for
 *		accelerated operations on a GEM object)
 */

#include <drm/drmP.h>
#include <drm/drm.h>
#include <drm/gma_drm.h>
#include <drm/drm_vma_manager.h>
#include "psb_drv.h"

void psb_gem_free_object(struct drm_gem_object *obj)
{
	struct gtt_range *gtt = container_of(obj, struct gtt_range, gem);

	/* Remove the list map if one is present */
	drm_gem_free_mmap_offset(obj);
	drm_gem_object_release(obj);

	/* This must occur last as it frees up the memory of the GEM object */
	psb_gtt_free_range(obj->dev, gtt);
}

int psb_gem_get_aperture(struct drm_device *dev, void *data,
				struct drm_file *file)
{
	return -EINVAL;
}

/**
 *	psb_gem_dumb_map_gtt	-	buffer mapping for dumb interface
 *	@file: our drm client file
 *	@dev: drm device
 *	@handle: GEM handle to the object (from dumb_create)
 *
 *	Do the necessary setup to allow the mapping of the frame buffer
 *	into user memory. We don't have to do much here at the moment.
 */
int psb_gem_dumb_map_gtt(struct drm_file *file, struct drm_device *dev,
			 uint32_t handle, uint64_t *offset)
{
	int ret = 0;
	struct drm_gem_object *obj;

	mutex_lock(&dev->struct_mutex);

	/* GEM does all our handle to object mapping */
	obj = drm_gem_object_lookup(dev, file, handle);
	if (obj == NULL) {
		ret = -ENOENT;
		goto unlock;
	}
	/* What validation is needed here ? */

	/* Make it mmapable */
	ret = drm_gem_create_mmap_offset(obj);
	if (ret)
		goto out;
	*offset = drm_vma_node_offset_addr(&obj->vma_node);
out:
	drm_gem_object_unreference(obj);
unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

/**
 *	psb_gem_create		-	create a mappable object
 *	@file: the DRM file of the client
 *	@dev: our device
 *	@size: the size requested
 *	@handlep: returned handle (opaque number)
 *
 *	Create a GEM object, fill in the boilerplate and attach a handle to
 *	it so that userspace can speak about it. This does the core work
 *	for the various methods that do/will create GEM objects for things
 */
int psb_gem_create(struct drm_file *file, struct drm_device *dev, u64 size,
		   u32 *handlep, int stolen, u32 align)
{
	struct gtt_range *r;
	int ret;
	u32 handle;

	size = roundup(size, PAGE_SIZE);

	/* Allocate our object - for now a direct gtt range which is not
	   stolen memory backed */
	r = psb_gtt_alloc_range(dev, size, "gem", 0, PAGE_SIZE);
	if (r == NULL) {
		dev_err(dev->dev, "no memory for %lld byte GEM object\n", size);
		return -ENOSPC;
	}
	/* Initialize the extra goodies GEM needs to do all the hard work */
	if (drm_gem_object_init(dev, &r->gem, size) != 0) {
		psb_gtt_free_range(dev, r);
		/* GEM doesn't give an error code so use -ENOMEM */
		dev_err(dev->dev, "GEM init failed for %lld\n", size);
		return -ENOMEM;
	}
	/* Limit the object to 32bit mappings */
	mapping_set_gfp_mask(r->gem.filp->f_mapping, GFP_KERNEL | __GFP_DMA32);
	/* Give the object a handle so we can carry it more easily */
	ret = drm_gem_handle_create(file, &r->gem, &handle);
	if (ret) {
		dev_err(dev->dev, "GEM handle failed for %p, %lld\n",
							&r->gem, size);
		drm_gem_object_release(&r->gem);
		psb_gtt_free_range(dev, r);
		return ret;
	}
	/* We have the initial and handle reference but need only one now */
	drm_gem_object_unreference_unlocked(&r->gem);
	*handlep = handle;
	return 0;
}

/**
 *	psb_gem_dumb_create	-	create a dumb buffer
 *	@drm_file: our client file
 *	@dev: our device
 *	@args: the requested arguments copied from userspace
 *
 *	Allocate a buffer suitable for use for a frame buffer of the
 *	form described by user space. Give userspace a handle by which
 *	to reference it.
 */
int psb_gem_dumb_create(struct drm_file *file, struct drm_device *dev,
			struct drm_mode_create_dumb *args)
{
	args->pitch = ALIGN(args->width * ((args->bpp + 7) / 8), 64);
	args->size = args->pitch * args->height;
	return psb_gem_create(file, dev, args->size, &args->handle, 0,
			      PAGE_SIZE);
}

/**
 *	psb_gem_fault		-	pagefault handler for GEM objects
 *	@vma: the VMA of the GEM object
 *	@vmf: fault detail
 *
 *	Invoked when a fault occurs on an mmap of a GEM managed area. GEM
 *	does most of the work for us including the actual map/unmap calls
 *	but we need to do the actual page work.
 *
 *	This code eventually needs to handle faulting objects in and out
 *	of the GTT and repacking it when we run out of space. We can put
 *	that off for now and for our simple uses
 *
 *	The VMA was set up by GEM. In doing so it also ensured that the
 *	vma->vm_private_data points to the GEM object that is backing this
 *	mapping.
 */
int psb_gem_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct drm_gem_object *obj;
	struct gtt_range *r;
	int ret;
	unsigned long pfn;
	pgoff_t page_offset;
	struct drm_device *dev;
	struct drm_psb_private *dev_priv;

	obj = vma->vm_private_data;	/* GEM object */
	dev = obj->dev;
	dev_priv = dev->dev_private;

	r = container_of(obj, struct gtt_range, gem);	/* Get the gtt range */

	/* Make sure we don't parallel update on a fault, nor move or remove
	   something from beneath our feet */
	mutex_lock(&dev->struct_mutex);

	/* For now the mmap pins the object and it stays pinned. As things
	   stand that will do us no harm */
	if (r->mmapping == 0) {
		ret = psb_gtt_pin(r);
		if (ret < 0) {
			dev_err(dev->dev, "gma500: pin failed: %d\n", ret);
			goto fail;
		}
		r->mmapping = 1;
	}

	/* Page relative to the VMA start - we must calculate this ourselves
	   because vmf->pgoff is the fake GEM offset */
	page_offset = ((unsigned long) vmf->virtual_address - vma->vm_start)
				>> PAGE_SHIFT;

	/* CPU view of the page, don't go via the GART for CPU writes */
	if (r->stolen)
		pfn = (dev_priv->stolen_base + r->offset) >> PAGE_SHIFT;
	else
		pfn = page_to_pfn(r->pages[page_offset]);
	ret = vm_insert_pfn(vma, (unsigned long)vmf->virtual_address, pfn);

fail:
	mutex_unlock(&dev->struct_mutex);
	switch (ret) {
	case 0:
	case -ERESTARTSYS:
	case -EINTR:
		return VM_FAULT_NOPAGE;
	case -ENOMEM:
		return VM_FAULT_OOM;
	default:
		return VM_FAULT_SIGBUS;
	}
}
