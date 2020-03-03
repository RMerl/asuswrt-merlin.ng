/*
 * Copyright 2013 Red Hat Inc.
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
#include "priv.h"

static int
nv20_ram_create(struct nvkm_object *parent, struct nvkm_object *engine,
		struct nvkm_oclass *oclass, void *data, u32 size,
		struct nvkm_object **pobject)
{
	struct nvkm_fb *pfb = nvkm_fb(parent);
	struct nvkm_ram *ram;
	u32 pbus1218 = nv_rd32(pfb, 0x001218);
	int ret;

	ret = nvkm_ram_create(parent, engine, oclass, &ram);
	*pobject = nv_object(ram);
	if (ret)
		return ret;

	switch (pbus1218 & 0x00000300) {
	case 0x00000000: ram->type = NV_MEM_TYPE_SDRAM; break;
	case 0x00000100: ram->type = NV_MEM_TYPE_DDR1; break;
	case 0x00000200: ram->type = NV_MEM_TYPE_GDDR3; break;
	case 0x00000300: ram->type = NV_MEM_TYPE_GDDR2; break;
	}
	ram->size  = (nv_rd32(pfb, 0x10020c) & 0xff000000);
	ram->parts = (nv_rd32(pfb, 0x100200) & 0x00000003) + 1;
	ram->tags  = nv_rd32(pfb, 0x100320);
	return 0;
}

struct nvkm_oclass
nv20_ram_oclass = {
	.handle = 0,
	.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = nv20_ram_create,
		.dtor = _nvkm_ram_dtor,
		.init = _nvkm_ram_init,
		.fini = _nvkm_ram_fini,
	}
};
