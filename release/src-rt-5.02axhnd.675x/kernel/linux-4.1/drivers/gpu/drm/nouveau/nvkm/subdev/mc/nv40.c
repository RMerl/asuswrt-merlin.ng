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

void
nv40_mc_msi_rearm(struct nvkm_mc *pmc)
{
	struct nv04_mc_priv *priv = (void *)pmc;
	nv_wr08(priv, 0x088068, 0xff);
}

struct nvkm_oclass *
nv40_mc_oclass = &(struct nvkm_mc_oclass) {
	.base.handle = NV_SUBDEV(MC, 0x40),
	.base.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = nv04_mc_ctor,
		.dtor = _nvkm_mc_dtor,
		.init = nv04_mc_init,
		.fini = _nvkm_mc_fini,
	},
	.intr = nv04_mc_intr,
	.msi_rearm = nv40_mc_msi_rearm,
}.base;
