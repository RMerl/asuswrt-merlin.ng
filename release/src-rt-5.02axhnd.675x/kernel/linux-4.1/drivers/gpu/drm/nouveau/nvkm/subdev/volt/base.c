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
#include <subdev/volt.h>
#include <subdev/bios.h>
#include <subdev/bios/vmap.h>
#include <subdev/bios/volt.h>

static int
nvkm_volt_get(struct nvkm_volt *volt)
{
	if (volt->vid_get) {
		int ret = volt->vid_get(volt), i;
		if (ret >= 0) {
			for (i = 0; i < volt->vid_nr; i++) {
				if (volt->vid[i].vid == ret)
					return volt->vid[i].uv;
			}
			ret = -EINVAL;
		}
		return ret;
	}
	return -ENODEV;
}

static int
nvkm_volt_set(struct nvkm_volt *volt, u32 uv)
{
	if (volt->vid_set) {
		int i, ret = -EINVAL;
		for (i = 0; i < volt->vid_nr; i++) {
			if (volt->vid[i].uv == uv) {
				ret = volt->vid_set(volt, volt->vid[i].vid);
				nv_debug(volt, "set %duv: %d\n", uv, ret);
				break;
			}
		}
		return ret;
	}
	return -ENODEV;
}

static int
nvkm_volt_map(struct nvkm_volt *volt, u8 id)
{
	struct nvkm_bios *bios = nvkm_bios(volt);
	struct nvbios_vmap_entry info;
	u8  ver, len;
	u16 vmap;

	vmap = nvbios_vmap_entry_parse(bios, id, &ver, &len, &info);
	if (vmap) {
		if (info.link != 0xff) {
			int ret = nvkm_volt_map(volt, info.link);
			if (ret < 0)
				return ret;
			info.min += ret;
		}
		return info.min;
	}

	return id ? id * 10000 : -ENODEV;
}

static int
nvkm_volt_set_id(struct nvkm_volt *volt, u8 id, int condition)
{
	int ret = nvkm_volt_map(volt, id);
	if (ret >= 0) {
		int prev = nvkm_volt_get(volt);
		if (!condition || prev < 0 ||
		    (condition < 0 && ret < prev) ||
		    (condition > 0 && ret > prev)) {
			ret = nvkm_volt_set(volt, ret);
		} else {
			ret = 0;
		}
	}
	return ret;
}

static void
nvkm_volt_parse_bios(struct nvkm_bios *bios, struct nvkm_volt *volt)
{
	struct nvbios_volt_entry ivid;
	struct nvbios_volt info;
	u8  ver, hdr, cnt, len;
	u16 data;
	int i;

	data = nvbios_volt_parse(bios, &ver, &hdr, &cnt, &len, &info);
	if (data && info.vidmask && info.base && info.step) {
		for (i = 0; i < info.vidmask + 1; i++) {
			if (info.base >= info.min &&
				info.base <= info.max) {
				volt->vid[volt->vid_nr].uv = info.base;
				volt->vid[volt->vid_nr].vid = i;
				volt->vid_nr++;
			}
			info.base += info.step;
		}
		volt->vid_mask = info.vidmask;
	} else if (data && info.vidmask) {
		for (i = 0; i < cnt; i++) {
			data = nvbios_volt_entry_parse(bios, i, &ver, &hdr,
						       &ivid);
			if (data) {
				volt->vid[volt->vid_nr].uv = ivid.voltage;
				volt->vid[volt->vid_nr].vid = ivid.vid;
				volt->vid_nr++;
			}
		}
		volt->vid_mask = info.vidmask;
	}
}

int
_nvkm_volt_init(struct nvkm_object *object)
{
	struct nvkm_volt *volt = (void *)object;
	int ret;

	ret = nvkm_subdev_init(&volt->base);
	if (ret)
		return ret;

	ret = volt->get(volt);
	if (ret < 0) {
		if (ret != -ENODEV)
			nv_debug(volt, "current voltage unknown\n");
		return 0;
	}

	nv_info(volt, "GPU voltage: %duv\n", ret);
	return 0;
}

void
_nvkm_volt_dtor(struct nvkm_object *object)
{
	struct nvkm_volt *volt = (void *)object;
	nvkm_subdev_destroy(&volt->base);
}

int
nvkm_volt_create_(struct nvkm_object *parent, struct nvkm_object *engine,
		  struct nvkm_oclass *oclass, int length, void **pobject)
{
	struct nvkm_bios *bios = nvkm_bios(parent);
	struct nvkm_volt *volt;
	int ret, i;

	ret = nvkm_subdev_create_(parent, engine, oclass, 0, "VOLT",
				  "voltage", length, pobject);
	volt = *pobject;
	if (ret)
		return ret;

	volt->get = nvkm_volt_get;
	volt->set = nvkm_volt_set;
	volt->set_id = nvkm_volt_set_id;

	/* Assuming the non-bios device should build the voltage table later */
	if (bios)
		nvkm_volt_parse_bios(bios, volt);

	if (volt->vid_nr) {
		for (i = 0; i < volt->vid_nr; i++) {
			nv_debug(volt, "VID %02x: %duv\n",
				 volt->vid[i].vid, volt->vid[i].uv);
		}

		/*XXX: this is an assumption.. there probably exists boards
		 * out there with i2c-connected voltage controllers too..
		 */
		ret = nvkm_voltgpio_init(volt);
		if (ret == 0) {
			volt->vid_get = nvkm_voltgpio_get;
			volt->vid_set = nvkm_voltgpio_set;
		}
	}

	return ret;
}
