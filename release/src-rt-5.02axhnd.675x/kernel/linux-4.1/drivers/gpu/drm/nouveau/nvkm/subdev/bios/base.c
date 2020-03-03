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
#include "priv.h"

#include <subdev/bios.h>
#include <subdev/bios/bmp.h>
#include <subdev/bios/bit.h>

u8
nvbios_checksum(const u8 *data, int size)
{
	u8 sum = 0;
	while (size--)
		sum += *data++;
	return sum;
}

u16
nvbios_findstr(const u8 *data, int size, const char *str, int len)
{
	int i, j;

	for (i = 0; i <= (size - len); i++) {
		for (j = 0; j < len; j++)
			if ((char)data[i + j] != str[j])
				break;
		if (j == len)
			return i;
	}

	return 0;
}

int
nvbios_extend(struct nvkm_bios *bios, u32 length)
{
	if (bios->size < length) {
		u8 *prev = bios->data;
		if (!(bios->data = kmalloc(length, GFP_KERNEL))) {
			bios->data = prev;
			return -ENOMEM;
		}
		memcpy(bios->data, prev, bios->size);
		bios->size = length;
		kfree(prev);
		return 1;
	}
	return 0;
}

static u8
nvkm_bios_rd08(struct nvkm_object *object, u64 addr)
{
	struct nvkm_bios *bios = (void *)object;
	return bios->data[addr];
}

static u16
nvkm_bios_rd16(struct nvkm_object *object, u64 addr)
{
	struct nvkm_bios *bios = (void *)object;
	return get_unaligned_le16(&bios->data[addr]);
}

static u32
nvkm_bios_rd32(struct nvkm_object *object, u64 addr)
{
	struct nvkm_bios *bios = (void *)object;
	return get_unaligned_le32(&bios->data[addr]);
}

static void
nvkm_bios_wr08(struct nvkm_object *object, u64 addr, u8 data)
{
	struct nvkm_bios *bios = (void *)object;
	bios->data[addr] = data;
}

static void
nvkm_bios_wr16(struct nvkm_object *object, u64 addr, u16 data)
{
	struct nvkm_bios *bios = (void *)object;
	put_unaligned_le16(data, &bios->data[addr]);
}

static void
nvkm_bios_wr32(struct nvkm_object *object, u64 addr, u32 data)
{
	struct nvkm_bios *bios = (void *)object;
	put_unaligned_le32(data, &bios->data[addr]);
}

static int
nvkm_bios_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
	       struct nvkm_oclass *oclass, void *data, u32 size,
	       struct nvkm_object **pobject)
{
	struct nvkm_bios *bios;
	struct bit_entry bit_i;
	int ret;

	ret = nvkm_subdev_create(parent, engine, oclass, 0,
				 "VBIOS", "bios", &bios);
	*pobject = nv_object(bios);
	if (ret)
		return ret;

	ret = nvbios_shadow(bios);
	if (ret)
		return ret;

	/* detect type of vbios we're dealing with */
	bios->bmp_offset = nvbios_findstr(bios->data, bios->size,
					  "\xff\x7f""NV\0", 5);
	if (bios->bmp_offset) {
		nv_info(bios, "BMP version %x.%x\n",
			bmp_version(bios) >> 8,
			bmp_version(bios) & 0xff);
	}

	bios->bit_offset = nvbios_findstr(bios->data, bios->size,
					  "\xff\xb8""BIT", 5);
	if (bios->bit_offset)
		nv_info(bios, "BIT signature found\n");

	/* determine the vbios version number */
	if (!bit_entry(bios, 'i', &bit_i) && bit_i.length >= 4) {
		bios->version.major = nv_ro08(bios, bit_i.offset + 3);
		bios->version.chip  = nv_ro08(bios, bit_i.offset + 2);
		bios->version.minor = nv_ro08(bios, bit_i.offset + 1);
		bios->version.micro = nv_ro08(bios, bit_i.offset + 0);
		bios->version.patch = nv_ro08(bios, bit_i.offset + 4);
	} else
	if (bmp_version(bios)) {
		bios->version.major = nv_ro08(bios, bios->bmp_offset + 13);
		bios->version.chip  = nv_ro08(bios, bios->bmp_offset + 12);
		bios->version.minor = nv_ro08(bios, bios->bmp_offset + 11);
		bios->version.micro = nv_ro08(bios, bios->bmp_offset + 10);
	}

	nv_info(bios, "version %02x.%02x.%02x.%02x.%02x\n",
		bios->version.major, bios->version.chip,
		bios->version.minor, bios->version.micro, bios->version.patch);

	return 0;
}

static void
nvkm_bios_dtor(struct nvkm_object *object)
{
	struct nvkm_bios *bios = (void *)object;
	kfree(bios->data);
	nvkm_subdev_destroy(&bios->base);
}

static int
nvkm_bios_init(struct nvkm_object *object)
{
	struct nvkm_bios *bios = (void *)object;
	return nvkm_subdev_init(&bios->base);
}

static int
nvkm_bios_fini(struct nvkm_object *object, bool suspend)
{
	struct nvkm_bios *bios = (void *)object;
	return nvkm_subdev_fini(&bios->base, suspend);
}

struct nvkm_oclass
nvkm_bios_oclass = {
	.handle = NV_SUBDEV(VBIOS, 0x00),
	.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = nvkm_bios_ctor,
		.dtor = nvkm_bios_dtor,
		.init = nvkm_bios_init,
		.fini = nvkm_bios_fini,
		.rd08 = nvkm_bios_rd08,
		.rd16 = nvkm_bios_rd16,
		.rd32 = nvkm_bios_rd32,
		.wr08 = nvkm_bios_wr08,
		.wr16 = nvkm_bios_wr16,
		.wr32 = nvkm_bios_wr32,
	},
};
