/*
 * Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "gf100.h"

struct gk20a_fb_priv {
	struct nvkm_fb base;
};

static int
gk20a_fb_init(struct nvkm_object *object)
{
	struct gk20a_fb_priv *priv = (void *)object;
	int ret;

	ret = nvkm_fb_init(&priv->base);
	if (ret)
		return ret;

	nv_mask(priv, 0x100c80, 0x00000001, 0x00000000); /* 128KiB lpg */
	return 0;
}

static int
gk20a_fb_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
	      struct nvkm_oclass *oclass, void *data, u32 size,
	      struct nvkm_object **pobject)
{
	struct gk20a_fb_priv *priv;
	int ret;

	ret = nvkm_fb_create(parent, engine, oclass, &priv);
	*pobject = nv_object(priv);
	if (ret)
		return ret;

	return 0;
}

struct nvkm_oclass *
gk20a_fb_oclass = &(struct nvkm_fb_impl) {
	.base.handle = NV_SUBDEV(FB, 0xea),
	.base.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = gk20a_fb_ctor,
		.dtor = _nvkm_fb_dtor,
		.init = gk20a_fb_init,
		.fini = _nvkm_fb_fini,
	},
	.memtype = gf100_fb_memtype_valid,
}.base;
