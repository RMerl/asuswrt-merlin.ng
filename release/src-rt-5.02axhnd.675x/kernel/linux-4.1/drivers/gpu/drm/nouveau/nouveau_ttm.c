/*
 * Copyright (c) 2007-2008 Tungsten Graphics, Inc., Cedar Park, TX., USA,
 * All Rights Reserved.
 * Copyright (c) 2009 VMware, Inc., Palo Alto, CA., USA,
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nouveau_drm.h"
#include "nouveau_ttm.h"
#include "nouveau_gem.h"

#include "drm_legacy.h"
static int
nouveau_vram_manager_init(struct ttm_mem_type_manager *man, unsigned long psize)
{
	struct nouveau_drm *drm = nouveau_bdev(man->bdev);
	struct nvkm_fb *pfb = nvxx_fb(&drm->device);
	man->priv = pfb;
	return 0;
}

static int
nouveau_vram_manager_fini(struct ttm_mem_type_manager *man)
{
	man->priv = NULL;
	return 0;
}

static inline void
nvkm_mem_node_cleanup(struct nvkm_mem *node)
{
	if (node->vma[0].node) {
		nvkm_vm_unmap(&node->vma[0]);
		nvkm_vm_put(&node->vma[0]);
	}

	if (node->vma[1].node) {
		nvkm_vm_unmap(&node->vma[1]);
		nvkm_vm_put(&node->vma[1]);
	}
}

static void
nouveau_vram_manager_del(struct ttm_mem_type_manager *man,
			 struct ttm_mem_reg *mem)
{
	struct nouveau_drm *drm = nouveau_bdev(man->bdev);
	struct nvkm_fb *pfb = nvxx_fb(&drm->device);
	nvkm_mem_node_cleanup(mem->mm_node);
	pfb->ram->put(pfb, (struct nvkm_mem **)&mem->mm_node);
}

static int
nouveau_vram_manager_new(struct ttm_mem_type_manager *man,
			 struct ttm_buffer_object *bo,
			 const struct ttm_place *place,
			 struct ttm_mem_reg *mem)
{
	struct nouveau_drm *drm = nouveau_bdev(man->bdev);
	struct nvkm_fb *pfb = nvxx_fb(&drm->device);
	struct nouveau_bo *nvbo = nouveau_bo(bo);
	struct nvkm_mem *node;
	u32 size_nc = 0;
	int ret;

	if (drm->device.info.ram_size == 0)
		return -ENOMEM;

	if (nvbo->tile_flags & NOUVEAU_GEM_TILE_NONCONTIG)
		size_nc = 1 << nvbo->page_shift;

	ret = pfb->ram->get(pfb, mem->num_pages << PAGE_SHIFT,
			   mem->page_alignment << PAGE_SHIFT, size_nc,
			   (nvbo->tile_flags >> 8) & 0x3ff, &node);
	if (ret) {
		mem->mm_node = NULL;
		return (ret == -ENOSPC) ? 0 : ret;
	}

	node->page_shift = nvbo->page_shift;

	mem->mm_node = node;
	mem->start   = node->offset >> PAGE_SHIFT;
	return 0;
}

static void
nouveau_vram_manager_debug(struct ttm_mem_type_manager *man, const char *prefix)
{
	struct nvkm_fb *pfb = man->priv;
	struct nvkm_mm *mm = &pfb->vram;
	struct nvkm_mm_node *r;
	u32 total = 0, free = 0;

	mutex_lock(&nv_subdev(pfb)->mutex);
	list_for_each_entry(r, &mm->nodes, nl_entry) {
		printk(KERN_DEBUG "%s %d: 0x%010llx 0x%010llx\n",
		       prefix, r->type, ((u64)r->offset << 12),
		       (((u64)r->offset + r->length) << 12));

		total += r->length;
		if (!r->type)
			free += r->length;
	}
	mutex_unlock(&nv_subdev(pfb)->mutex);

	printk(KERN_DEBUG "%s  total: 0x%010llx free: 0x%010llx\n",
	       prefix, (u64)total << 12, (u64)free << 12);
	printk(KERN_DEBUG "%s  block: 0x%08x\n",
	       prefix, mm->block_size << 12);
}

const struct ttm_mem_type_manager_func nouveau_vram_manager = {
	nouveau_vram_manager_init,
	nouveau_vram_manager_fini,
	nouveau_vram_manager_new,
	nouveau_vram_manager_del,
	nouveau_vram_manager_debug
};

static int
nouveau_gart_manager_init(struct ttm_mem_type_manager *man, unsigned long psize)
{
	return 0;
}

static int
nouveau_gart_manager_fini(struct ttm_mem_type_manager *man)
{
	return 0;
}

static void
nouveau_gart_manager_del(struct ttm_mem_type_manager *man,
			 struct ttm_mem_reg *mem)
{
	nvkm_mem_node_cleanup(mem->mm_node);
	kfree(mem->mm_node);
	mem->mm_node = NULL;
}

static int
nouveau_gart_manager_new(struct ttm_mem_type_manager *man,
			 struct ttm_buffer_object *bo,
			 const struct ttm_place *place,
			 struct ttm_mem_reg *mem)
{
	struct nouveau_drm *drm = nouveau_bdev(bo->bdev);
	struct nouveau_bo *nvbo = nouveau_bo(bo);
	struct nvkm_mem *node;

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return -ENOMEM;

	node->page_shift = 12;

	switch (drm->device.info.family) {
	case NV_DEVICE_INFO_V0_TESLA:
		if (drm->device.info.chipset != 0x50)
			node->memtype = (nvbo->tile_flags & 0x7f00) >> 8;
		break;
	case NV_DEVICE_INFO_V0_FERMI:
	case NV_DEVICE_INFO_V0_KEPLER:
		node->memtype = (nvbo->tile_flags & 0xff00) >> 8;
		break;
	default:
		break;
	}

	mem->mm_node = node;
	mem->start   = 0;
	return 0;
}

static void
nouveau_gart_manager_debug(struct ttm_mem_type_manager *man, const char *prefix)
{
}

const struct ttm_mem_type_manager_func nouveau_gart_manager = {
	nouveau_gart_manager_init,
	nouveau_gart_manager_fini,
	nouveau_gart_manager_new,
	nouveau_gart_manager_del,
	nouveau_gart_manager_debug
};

/*XXX*/
#include <subdev/mmu/nv04.h>
static int
nv04_gart_manager_init(struct ttm_mem_type_manager *man, unsigned long psize)
{
	struct nouveau_drm *drm = nouveau_bdev(man->bdev);
	struct nvkm_mmu *mmu = nvxx_mmu(&drm->device);
	struct nv04_mmu_priv *priv = (void *)mmu;
	struct nvkm_vm *vm = NULL;
	nvkm_vm_ref(priv->vm, &vm, NULL);
	man->priv = vm;
	return 0;
}

static int
nv04_gart_manager_fini(struct ttm_mem_type_manager *man)
{
	struct nvkm_vm *vm = man->priv;
	nvkm_vm_ref(NULL, &vm, NULL);
	man->priv = NULL;
	return 0;
}

static void
nv04_gart_manager_del(struct ttm_mem_type_manager *man, struct ttm_mem_reg *mem)
{
	struct nvkm_mem *node = mem->mm_node;
	if (node->vma[0].node)
		nvkm_vm_put(&node->vma[0]);
	kfree(mem->mm_node);
	mem->mm_node = NULL;
}

static int
nv04_gart_manager_new(struct ttm_mem_type_manager *man,
		      struct ttm_buffer_object *bo,
		      const struct ttm_place *place,
		      struct ttm_mem_reg *mem)
{
	struct nvkm_mem *node;
	int ret;

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return -ENOMEM;

	node->page_shift = 12;

	ret = nvkm_vm_get(man->priv, mem->num_pages << 12, node->page_shift,
			  NV_MEM_ACCESS_RW, &node->vma[0]);
	if (ret) {
		kfree(node);
		return ret;
	}

	mem->mm_node = node;
	mem->start   = node->vma[0].offset >> PAGE_SHIFT;
	return 0;
}

static void
nv04_gart_manager_debug(struct ttm_mem_type_manager *man, const char *prefix)
{
}

const struct ttm_mem_type_manager_func nv04_gart_manager = {
	nv04_gart_manager_init,
	nv04_gart_manager_fini,
	nv04_gart_manager_new,
	nv04_gart_manager_del,
	nv04_gart_manager_debug
};

int
nouveau_ttm_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct drm_file *file_priv = filp->private_data;
	struct nouveau_drm *drm = nouveau_drm(file_priv->minor->dev);

	if (unlikely(vma->vm_pgoff < DRM_FILE_PAGE_OFFSET))
		return drm_legacy_mmap(filp, vma);

	return ttm_bo_mmap(filp, vma, &drm->ttm.bdev);
}

static int
nouveau_ttm_mem_global_init(struct drm_global_reference *ref)
{
	return ttm_mem_global_init(ref->object);
}

static void
nouveau_ttm_mem_global_release(struct drm_global_reference *ref)
{
	ttm_mem_global_release(ref->object);
}

int
nouveau_ttm_global_init(struct nouveau_drm *drm)
{
	struct drm_global_reference *global_ref;
	int ret;

	global_ref = &drm->ttm.mem_global_ref;
	global_ref->global_type = DRM_GLOBAL_TTM_MEM;
	global_ref->size = sizeof(struct ttm_mem_global);
	global_ref->init = &nouveau_ttm_mem_global_init;
	global_ref->release = &nouveau_ttm_mem_global_release;

	ret = drm_global_item_ref(global_ref);
	if (unlikely(ret != 0)) {
		DRM_ERROR("Failed setting up TTM memory accounting\n");
		drm->ttm.mem_global_ref.release = NULL;
		return ret;
	}

	drm->ttm.bo_global_ref.mem_glob = global_ref->object;
	global_ref = &drm->ttm.bo_global_ref.ref;
	global_ref->global_type = DRM_GLOBAL_TTM_BO;
	global_ref->size = sizeof(struct ttm_bo_global);
	global_ref->init = &ttm_bo_global_init;
	global_ref->release = &ttm_bo_global_release;

	ret = drm_global_item_ref(global_ref);
	if (unlikely(ret != 0)) {
		DRM_ERROR("Failed setting up TTM BO subsystem\n");
		drm_global_item_unref(&drm->ttm.mem_global_ref);
		drm->ttm.mem_global_ref.release = NULL;
		return ret;
	}

	return 0;
}

void
nouveau_ttm_global_release(struct nouveau_drm *drm)
{
	if (drm->ttm.mem_global_ref.release == NULL)
		return;

	drm_global_item_unref(&drm->ttm.bo_global_ref.ref);
	drm_global_item_unref(&drm->ttm.mem_global_ref);
	drm->ttm.mem_global_ref.release = NULL;
}

int
nouveau_ttm_init(struct nouveau_drm *drm)
{
	struct drm_device *dev = drm->dev;
	u32 bits;
	int ret;

	bits = nvxx_mmu(&drm->device)->dma_bits;
	if (nv_device_is_pci(nvxx_device(&drm->device))) {
		if (drm->agp.stat == ENABLED ||
		     !pci_dma_supported(dev->pdev, DMA_BIT_MASK(bits)))
			bits = 32;

		ret = pci_set_dma_mask(dev->pdev, DMA_BIT_MASK(bits));
		if (ret)
			return ret;

		ret = pci_set_consistent_dma_mask(dev->pdev,
						  DMA_BIT_MASK(bits));
		if (ret)
			pci_set_consistent_dma_mask(dev->pdev,
						    DMA_BIT_MASK(32));
	}

	ret = nouveau_ttm_global_init(drm);
	if (ret)
		return ret;

	ret = ttm_bo_device_init(&drm->ttm.bdev,
				  drm->ttm.bo_global_ref.ref.object,
				  &nouveau_bo_driver,
				  dev->anon_inode->i_mapping,
				  DRM_FILE_PAGE_OFFSET,
				  bits <= 32 ? true : false);
	if (ret) {
		NV_ERROR(drm, "error initialising bo driver, %d\n", ret);
		return ret;
	}

	/* VRAM init */
	drm->gem.vram_available = drm->device.info.ram_user;

	ret = ttm_bo_init_mm(&drm->ttm.bdev, TTM_PL_VRAM,
			      drm->gem.vram_available >> PAGE_SHIFT);
	if (ret) {
		NV_ERROR(drm, "VRAM mm init failed, %d\n", ret);
		return ret;
	}

	drm->ttm.mtrr = arch_phys_wc_add(nv_device_resource_start(nvxx_device(&drm->device), 1),
					 nv_device_resource_len(nvxx_device(&drm->device), 1));

	/* GART init */
	if (drm->agp.stat != ENABLED) {
		drm->gem.gart_available = nvxx_mmu(&drm->device)->limit;
	} else {
		drm->gem.gart_available = drm->agp.size;
	}

	ret = ttm_bo_init_mm(&drm->ttm.bdev, TTM_PL_TT,
			      drm->gem.gart_available >> PAGE_SHIFT);
	if (ret) {
		NV_ERROR(drm, "GART mm init failed, %d\n", ret);
		return ret;
	}

	NV_INFO(drm, "VRAM: %d MiB\n", (u32)(drm->gem.vram_available >> 20));
	NV_INFO(drm, "GART: %d MiB\n", (u32)(drm->gem.gart_available >> 20));
	return 0;
}

void
nouveau_ttm_fini(struct nouveau_drm *drm)
{
	mutex_lock(&drm->dev->struct_mutex);
	ttm_bo_clean_mm(&drm->ttm.bdev, TTM_PL_VRAM);
	ttm_bo_clean_mm(&drm->ttm.bdev, TTM_PL_TT);
	mutex_unlock(&drm->dev->struct_mutex);

	ttm_bo_device_release(&drm->ttm.bdev);

	nouveau_ttm_global_release(drm);

	arch_phys_wc_del(drm->ttm.mtrr);
	drm->ttm.mtrr = 0;
}
