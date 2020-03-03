/*
 * Copyright (C) 2010 Francisco Jerez.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "nv04.h"
#include "fbmem.h"

#include <subdev/bios.h>
#include <subdev/bios/init.h>

static void
nv10_devinit_meminit(struct nvkm_devinit *devinit)
{
	struct nv04_devinit_priv *priv = (void *)devinit;
	static const int mem_width[] = { 0x10, 0x00, 0x20 };
	int mem_width_count;
	uint32_t patt = 0xdeadbeef;
	struct io_mapping *fb;
	int i, j, k;

	if (nv_device(priv)->card_type >= NV_11 &&
	    nv_device(priv)->chipset >= 0x17)
		mem_width_count = 3;
	else
		mem_width_count = 2;

	/* Map the framebuffer aperture */
	fb = fbmem_init(nv_device(priv));
	if (!fb) {
		nv_error(priv, "failed to map fb\n");
		return;
	}

	nv_wr32(priv, NV10_PFB_REFCTRL, NV10_PFB_REFCTRL_VALID_1);

	/* Probe memory bus width */
	for (i = 0; i < mem_width_count; i++) {
		nv_mask(priv, NV04_PFB_CFG0, 0x30, mem_width[i]);

		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++)
				fbmem_poke(fb, 0x1c, 0);

			fbmem_poke(fb, 0x1c, patt);
			fbmem_poke(fb, 0x3c, 0);

			if (fbmem_peek(fb, 0x1c) == patt)
				goto mem_width_found;
		}
	}

mem_width_found:
	patt <<= 1;

	/* Probe amount of installed memory */
	for (i = 0; i < 4; i++) {
		int off = nv_rd32(priv, 0x10020c) - 0x100000;

		fbmem_poke(fb, off, patt);
		fbmem_poke(fb, 0, 0);

		fbmem_peek(fb, 0);
		fbmem_peek(fb, 0);
		fbmem_peek(fb, 0);
		fbmem_peek(fb, 0);

		if (fbmem_peek(fb, off) == patt)
			goto amount_found;
	}

	/* IC missing - disable the upper half memory space. */
	nv_mask(priv, NV04_PFB_CFG0, 0x1000, 0);

amount_found:
	fbmem_fini(fb);
}

struct nvkm_oclass *
nv10_devinit_oclass = &(struct nvkm_devinit_impl) {
	.base.handle = NV_SUBDEV(DEVINIT, 0x10),
	.base.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = nv04_devinit_ctor,
		.dtor = nv04_devinit_dtor,
		.init = nv04_devinit_init,
		.fini = nv04_devinit_fini,
	},
	.meminit = nv10_devinit_meminit,
	.pll_set = nv04_devinit_pll_set,
	.post = nvbios_init,
}.base;
