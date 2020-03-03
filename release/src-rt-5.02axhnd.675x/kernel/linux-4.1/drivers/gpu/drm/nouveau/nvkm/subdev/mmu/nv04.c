/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */
#include "nv04.h"

#include <core/device.h>
#include <core/gpuobj.h>

#define NV04_PDMA_SIZE (128 * 1024 * 1024)
#define NV04_PDMA_PAGE (  4 * 1024)

/*******************************************************************************
 * VM map/unmap callbacks
 ******************************************************************************/

static void
nv04_vm_map_sg(struct nvkm_vma *vma, struct nvkm_gpuobj *pgt,
	       struct nvkm_mem *mem, u32 pte, u32 cnt, dma_addr_t *list)
{
	pte = 0x00008 + (pte * 4);
	while (cnt) {
		u32 page = PAGE_SIZE / NV04_PDMA_PAGE;
		u32 phys = (u32)*list++;
		while (cnt && page--) {
			nv_wo32(pgt, pte, phys | 3);
			phys += NV04_PDMA_PAGE;
			pte += 4;
			cnt -= 1;
		}
	}
}

static void
nv04_vm_unmap(struct nvkm_gpuobj *pgt, u32 pte, u32 cnt)
{
	pte = 0x00008 + (pte * 4);
	while (cnt--) {
		nv_wo32(pgt, pte, 0x00000000);
		pte += 4;
	}
}

static void
nv04_vm_flush(struct nvkm_vm *vm)
{
}

/*******************************************************************************
 * VM object
 ******************************************************************************/

int
nv04_vm_create(struct nvkm_mmu *mmu, u64 offset, u64 length, u64 mmstart,
	       struct nvkm_vm **pvm)
{
	return -EINVAL;
}

/*******************************************************************************
 * MMU subdev
 ******************************************************************************/

static int
nv04_mmu_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
	      struct nvkm_oclass *oclass, void *data, u32 size,
	      struct nvkm_object **pobject)
{
	struct nv04_mmu_priv *priv;
	struct nvkm_gpuobj *dma;
	int ret;

	ret = nvkm_mmu_create(parent, engine, oclass, "PCIGART",
			      "pcigart", &priv);
	*pobject = nv_object(priv);
	if (ret)
		return ret;

	priv->base.create = nv04_vm_create;
	priv->base.limit = NV04_PDMA_SIZE;
	priv->base.dma_bits = 32;
	priv->base.pgt_bits = 32 - 12;
	priv->base.spg_shift = 12;
	priv->base.lpg_shift = 12;
	priv->base.map_sg = nv04_vm_map_sg;
	priv->base.unmap = nv04_vm_unmap;
	priv->base.flush = nv04_vm_flush;

	ret = nvkm_vm_create(&priv->base, 0, NV04_PDMA_SIZE, 0, 4096,
			     &priv->vm);
	if (ret)
		return ret;

	ret = nvkm_gpuobj_new(nv_object(priv), NULL,
			      (NV04_PDMA_SIZE / NV04_PDMA_PAGE) * 4 + 8,
			      16, NVOBJ_FLAG_ZERO_ALLOC,
			      &priv->vm->pgt[0].obj[0]);
	dma = priv->vm->pgt[0].obj[0];
	priv->vm->pgt[0].refcount[0] = 1;
	if (ret)
		return ret;

	nv_wo32(dma, 0x00000, 0x0002103d); /* PCI, RW, PT, !LN */
	nv_wo32(dma, 0x00004, NV04_PDMA_SIZE - 1);
	return 0;
}

void
nv04_mmu_dtor(struct nvkm_object *object)
{
	struct nv04_mmu_priv *priv = (void *)object;
	if (priv->vm) {
		nvkm_gpuobj_ref(NULL, &priv->vm->pgt[0].obj[0]);
		nvkm_vm_ref(NULL, &priv->vm, NULL);
	}
	if (priv->nullp) {
		pci_free_consistent(nv_device(priv)->pdev, 16 * 1024,
				    priv->nullp, priv->null);
	}
	nvkm_mmu_destroy(&priv->base);
}

struct nvkm_oclass
nv04_mmu_oclass = {
	.handle = NV_SUBDEV(MMU, 0x04),
	.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = nv04_mmu_ctor,
		.dtor = nv04_mmu_dtor,
		.init = _nvkm_mmu_init,
		.fini = _nvkm_mmu_fini,
	},
};
