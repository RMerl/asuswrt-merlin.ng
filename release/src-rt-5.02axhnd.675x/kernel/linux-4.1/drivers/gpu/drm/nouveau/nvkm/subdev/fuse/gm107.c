/*
 * Copyright 2014 Martin Peres
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
 * Authors: Martin Peres
 */
#include "priv.h"

struct gm107_fuse_priv {
	struct nvkm_fuse base;
};

static u32
gm107_fuse_rd32(struct nvkm_object *object, u64 addr)
{
	struct gf100_fuse_priv *priv = (void *)object;
	return nv_rd32(priv, 0x21100 + addr);
}


static int
gm107_fuse_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
		struct nvkm_oclass *oclass, void *data, u32 size,
		struct nvkm_object **pobject)
{
	struct gm107_fuse_priv *priv;
	int ret;

	ret = nvkm_fuse_create(parent, engine, oclass, &priv);
	*pobject = nv_object(priv);

	return ret;
}

struct nvkm_oclass
gm107_fuse_oclass = {
	.handle = NV_SUBDEV(FUSE, 0x117),
	.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = gm107_fuse_ctor,
		.dtor = _nvkm_fuse_dtor,
		.init = _nvkm_fuse_init,
		.fini = _nvkm_fuse_fini,
		.rd32 = gm107_fuse_rd32,
	},
};
