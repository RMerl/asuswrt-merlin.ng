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
#include "acpi.h"

#include <core/client.h>
#include <core/option.h>
#include <core/notify.h>
#include <core/parent.h>
#include <subdev/bios.h>
#include <subdev/fb.h>
#include <subdev/instmem.h>

#include <nvif/class.h>
#include <nvif/unpack.h>

static DEFINE_MUTEX(nv_devices_mutex);
static LIST_HEAD(nv_devices);

struct nvkm_device *
nvkm_device_find(u64 name)
{
	struct nvkm_device *device, *match = NULL;
	mutex_lock(&nv_devices_mutex);
	list_for_each_entry(device, &nv_devices, head) {
		if (device->handle == name) {
			match = device;
			break;
		}
	}
	mutex_unlock(&nv_devices_mutex);
	return match;
}

int
nvkm_device_list(u64 *name, int size)
{
	struct nvkm_device *device;
	int nr = 0;
	mutex_lock(&nv_devices_mutex);
	list_for_each_entry(device, &nv_devices, head) {
		if (nr++ < size)
			name[nr - 1] = device->handle;
	}
	mutex_unlock(&nv_devices_mutex);
	return nr;
}

/******************************************************************************
 * nvkm_devobj (0x0080): class implementation
 *****************************************************************************/

struct nvkm_devobj {
	struct nvkm_parent base;
	struct nvkm_object *subdev[NVDEV_SUBDEV_NR];
};

static int
nvkm_devobj_info(struct nvkm_object *object, void *data, u32 size)
{
	struct nvkm_device *device = nv_device(object);
	struct nvkm_fb *pfb = nvkm_fb(device);
	struct nvkm_instmem *imem = nvkm_instmem(device);
	union {
		struct nv_device_info_v0 v0;
	} *args = data;
	int ret;

	nv_ioctl(object, "device info size %d\n", size);
	if (nvif_unpack(args->v0, 0, 0, false)) {
		nv_ioctl(object, "device info vers %d\n", args->v0.version);
	} else
		return ret;

	switch (device->chipset) {
	case 0x01a:
	case 0x01f:
	case 0x04c:
	case 0x04e:
	case 0x063:
	case 0x067:
	case 0x068:
	case 0x0aa:
	case 0x0ac:
	case 0x0af:
		args->v0.platform = NV_DEVICE_INFO_V0_IGP;
		break;
	default:
		if (device->pdev) {
			if (pci_find_capability(device->pdev, PCI_CAP_ID_AGP))
				args->v0.platform = NV_DEVICE_INFO_V0_AGP;
			else
			if (pci_is_pcie(device->pdev))
				args->v0.platform = NV_DEVICE_INFO_V0_PCIE;
			else
				args->v0.platform = NV_DEVICE_INFO_V0_PCI;
		} else {
			args->v0.platform = NV_DEVICE_INFO_V0_SOC;
		}
		break;
	}

	switch (device->card_type) {
	case NV_04: args->v0.family = NV_DEVICE_INFO_V0_TNT; break;
	case NV_10:
	case NV_11: args->v0.family = NV_DEVICE_INFO_V0_CELSIUS; break;
	case NV_20: args->v0.family = NV_DEVICE_INFO_V0_KELVIN; break;
	case NV_30: args->v0.family = NV_DEVICE_INFO_V0_RANKINE; break;
	case NV_40: args->v0.family = NV_DEVICE_INFO_V0_CURIE; break;
	case NV_50: args->v0.family = NV_DEVICE_INFO_V0_TESLA; break;
	case NV_C0: args->v0.family = NV_DEVICE_INFO_V0_FERMI; break;
	case NV_E0: args->v0.family = NV_DEVICE_INFO_V0_KEPLER; break;
	case GM100: args->v0.family = NV_DEVICE_INFO_V0_MAXWELL; break;
	default:
		args->v0.family = 0;
		break;
	}

	args->v0.chipset  = device->chipset;
	args->v0.revision = device->chiprev;
	if (pfb && pfb->ram)
		args->v0.ram_size = args->v0.ram_user = pfb->ram->size;
	else
		args->v0.ram_size = args->v0.ram_user = 0;
	if (imem && args->v0.ram_size > 0)
		args->v0.ram_user = args->v0.ram_user - imem->reserved;

	return 0;
}

static int
nvkm_devobj_mthd(struct nvkm_object *object, u32 mthd, void *data, u32 size)
{
	switch (mthd) {
	case NV_DEVICE_V0_INFO:
		return nvkm_devobj_info(object, data, size);
	default:
		break;
	}
	return -EINVAL;
}

static u8
nvkm_devobj_rd08(struct nvkm_object *object, u64 addr)
{
	return nv_rd08(object->engine, addr);
}

static u16
nvkm_devobj_rd16(struct nvkm_object *object, u64 addr)
{
	return nv_rd16(object->engine, addr);
}

static u32
nvkm_devobj_rd32(struct nvkm_object *object, u64 addr)
{
	return nv_rd32(object->engine, addr);
}

static void
nvkm_devobj_wr08(struct nvkm_object *object, u64 addr, u8 data)
{
	nv_wr08(object->engine, addr, data);
}

static void
nvkm_devobj_wr16(struct nvkm_object *object, u64 addr, u16 data)
{
	nv_wr16(object->engine, addr, data);
}

static void
nvkm_devobj_wr32(struct nvkm_object *object, u64 addr, u32 data)
{
	nv_wr32(object->engine, addr, data);
}

static int
nvkm_devobj_map(struct nvkm_object *object, u64 *addr, u32 *size)
{
	struct nvkm_device *device = nv_device(object);
	*addr = nv_device_resource_start(device, 0);
	*size = nv_device_resource_len(device, 0);
	return 0;
}

static const u64 disable_map[] = {
	[NVDEV_SUBDEV_VBIOS]	= NV_DEVICE_V0_DISABLE_VBIOS,
	[NVDEV_SUBDEV_DEVINIT]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_GPIO]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_I2C]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_CLK  ]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_MXM]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_MC]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_BUS]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_TIMER]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_FB]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_LTC]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_IBUS]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_INSTMEM]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_MMU]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_BAR]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_VOLT]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_THERM]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_PMU]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_SUBDEV_FUSE]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_ENGINE_DMAOBJ]	= NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_ENGINE_PM     ]  = NV_DEVICE_V0_DISABLE_CORE,
	[NVDEV_ENGINE_FIFO]	= NV_DEVICE_V0_DISABLE_FIFO,
	[NVDEV_ENGINE_SW]	= NV_DEVICE_V0_DISABLE_FIFO,
	[NVDEV_ENGINE_GR]	= NV_DEVICE_V0_DISABLE_GR,
	[NVDEV_ENGINE_MPEG]	= NV_DEVICE_V0_DISABLE_MPEG,
	[NVDEV_ENGINE_ME]	= NV_DEVICE_V0_DISABLE_ME,
	[NVDEV_ENGINE_VP]	= NV_DEVICE_V0_DISABLE_VP,
	[NVDEV_ENGINE_CIPHER]	= NV_DEVICE_V0_DISABLE_CIPHER,
	[NVDEV_ENGINE_BSP]	= NV_DEVICE_V0_DISABLE_BSP,
	[NVDEV_ENGINE_MSPPP]	= NV_DEVICE_V0_DISABLE_MSPPP,
	[NVDEV_ENGINE_CE0]	= NV_DEVICE_V0_DISABLE_CE0,
	[NVDEV_ENGINE_CE1]	= NV_DEVICE_V0_DISABLE_CE1,
	[NVDEV_ENGINE_CE2]	= NV_DEVICE_V0_DISABLE_CE2,
	[NVDEV_ENGINE_VIC]	= NV_DEVICE_V0_DISABLE_VIC,
	[NVDEV_ENGINE_MSENC]	= NV_DEVICE_V0_DISABLE_MSENC,
	[NVDEV_ENGINE_DISP]	= NV_DEVICE_V0_DISABLE_DISP,
	[NVDEV_ENGINE_MSVLD]	= NV_DEVICE_V0_DISABLE_MSVLD,
	[NVDEV_ENGINE_SEC]	= NV_DEVICE_V0_DISABLE_SEC,
	[NVDEV_SUBDEV_NR]	= 0,
};

static void
nvkm_devobj_dtor(struct nvkm_object *object)
{
	struct nvkm_devobj *devobj = (void *)object;
	int i;

	for (i = NVDEV_SUBDEV_NR - 1; i >= 0; i--)
		nvkm_object_ref(NULL, &devobj->subdev[i]);

	nvkm_parent_destroy(&devobj->base);
}

static struct nvkm_oclass
nvkm_devobj_oclass_super = {
	.handle = NV_DEVICE,
	.ofuncs = &(struct nvkm_ofuncs) {
		.dtor = nvkm_devobj_dtor,
		.init = _nvkm_parent_init,
		.fini = _nvkm_parent_fini,
		.mthd = nvkm_devobj_mthd,
		.map  = nvkm_devobj_map,
		.rd08 = nvkm_devobj_rd08,
		.rd16 = nvkm_devobj_rd16,
		.rd32 = nvkm_devobj_rd32,
		.wr08 = nvkm_devobj_wr08,
		.wr16 = nvkm_devobj_wr16,
		.wr32 = nvkm_devobj_wr32,
	}
};

static int
nvkm_devobj_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
		 struct nvkm_oclass *oclass, void *data, u32 size,
		 struct nvkm_object **pobject)
{
	union {
		struct nv_device_v0 v0;
	} *args = data;
	struct nvkm_client *client = nv_client(parent);
	struct nvkm_device *device;
	struct nvkm_devobj *devobj;
	u32 boot0, strap;
	u64 disable, mmio_base, mmio_size;
	void __iomem *map;
	int ret, i, c;

	nv_ioctl(parent, "create device size %d\n", size);
	if (nvif_unpack(args->v0, 0, 0, false)) {
		nv_ioctl(parent, "create device v%d device %016llx "
				 "disable %016llx debug0 %016llx\n",
			 args->v0.version, args->v0.device,
			 args->v0.disable, args->v0.debug0);
	} else
		return ret;

	/* give priviledged clients register access */
	if (client->super)
		oclass = &nvkm_devobj_oclass_super;

	/* find the device subdev that matches what the client requested */
	device = nv_device(client->device);
	if (args->v0.device != ~0) {
		device = nvkm_device_find(args->v0.device);
		if (!device)
			return -ENODEV;
	}

	ret = nvkm_parent_create(parent, nv_object(device), oclass, 0,
				 nvkm_control_oclass,
				 (1ULL << NVDEV_ENGINE_DMAOBJ) |
				 (1ULL << NVDEV_ENGINE_FIFO) |
				 (1ULL << NVDEV_ENGINE_DISP) |
				 (1ULL << NVDEV_ENGINE_PM), &devobj);
	*pobject = nv_object(devobj);
	if (ret)
		return ret;

	mmio_base = nv_device_resource_start(device, 0);
	mmio_size = nv_device_resource_len(device, 0);

	/* translate api disable mask into internal mapping */
	disable = args->v0.debug0;
	for (i = 0; i < NVDEV_SUBDEV_NR; i++) {
		if (args->v0.disable & disable_map[i])
			disable |= (1ULL << i);
	}

	/* identify the chipset, and determine classes of subdev/engines */
	if (!(args->v0.disable & NV_DEVICE_V0_DISABLE_IDENTIFY) &&
	    !device->card_type) {
		map = ioremap(mmio_base, 0x102000);
		if (map == NULL)
			return -ENOMEM;

		/* switch mmio to cpu's native endianness */
#ifndef __BIG_ENDIAN
		if (ioread32_native(map + 0x000004) != 0x00000000) {
#else
		if (ioread32_native(map + 0x000004) == 0x00000000) {
#endif
			iowrite32_native(0x01000001, map + 0x000004);
			ioread32_native(map);
		}

		/* read boot0 and strapping information */
		boot0 = ioread32_native(map + 0x000000);
		strap = ioread32_native(map + 0x101000);
		iounmap(map);

		/* determine chipset and derive architecture from it */
		if ((boot0 & 0x1f000000) > 0) {
			device->chipset = (boot0 & 0x1ff00000) >> 20;
			device->chiprev = (boot0 & 0x000000ff);
			switch (device->chipset & 0x1f0) {
			case 0x010: {
				if (0x461 & (1 << (device->chipset & 0xf)))
					device->card_type = NV_10;
				else
					device->card_type = NV_11;
				device->chiprev = 0x00;
				break;
			}
			case 0x020: device->card_type = NV_20; break;
			case 0x030: device->card_type = NV_30; break;
			case 0x040:
			case 0x060: device->card_type = NV_40; break;
			case 0x050:
			case 0x080:
			case 0x090:
			case 0x0a0: device->card_type = NV_50; break;
			case 0x0c0:
			case 0x0d0: device->card_type = NV_C0; break;
			case 0x0e0:
			case 0x0f0:
			case 0x100: device->card_type = NV_E0; break;
			case 0x110:
			case 0x120: device->card_type = GM100; break;
			default:
				break;
			}
		} else
		if ((boot0 & 0xff00fff0) == 0x20004000) {
			if (boot0 & 0x00f00000)
				device->chipset = 0x05;
			else
				device->chipset = 0x04;
			device->card_type = NV_04;
		}

		switch (device->card_type) {
		case NV_04: ret = nv04_identify(device); break;
		case NV_10:
		case NV_11: ret = nv10_identify(device); break;
		case NV_20: ret = nv20_identify(device); break;
		case NV_30: ret = nv30_identify(device); break;
		case NV_40: ret = nv40_identify(device); break;
		case NV_50: ret = nv50_identify(device); break;
		case NV_C0: ret = gf100_identify(device); break;
		case NV_E0: ret = gk104_identify(device); break;
		case GM100: ret = gm100_identify(device); break;
		default:
			ret = -EINVAL;
			break;
		}

		if (ret) {
			nv_error(device, "unknown chipset, 0x%08x\n", boot0);
			return ret;
		}

		nv_info(device, "BOOT0  : 0x%08x\n", boot0);
		nv_info(device, "Chipset: %s (NV%02X)\n",
			device->cname, device->chipset);
		nv_info(device, "Family : NV%02X\n", device->card_type);

		/* determine frequency of timing crystal */
		if ( device->card_type <= NV_10 || device->chipset < 0x17 ||
		    (device->chipset >= 0x20 && device->chipset < 0x25))
			strap &= 0x00000040;
		else
			strap &= 0x00400040;

		switch (strap) {
		case 0x00000000: device->crystal = 13500; break;
		case 0x00000040: device->crystal = 14318; break;
		case 0x00400000: device->crystal = 27000; break;
		case 0x00400040: device->crystal = 25000; break;
		}

		nv_debug(device, "crystal freq: %dKHz\n", device->crystal);
	} else
	if ( (args->v0.disable & NV_DEVICE_V0_DISABLE_IDENTIFY)) {
		device->cname = "NULL";
		device->oclass[NVDEV_SUBDEV_VBIOS] = &nvkm_bios_oclass;
	}

	if (!(args->v0.disable & NV_DEVICE_V0_DISABLE_MMIO) &&
	    !nv_subdev(device)->mmio) {
		nv_subdev(device)->mmio  = ioremap(mmio_base, mmio_size);
		if (!nv_subdev(device)->mmio) {
			nv_error(device, "unable to map device registers\n");
			return -ENOMEM;
		}
	}

	/* ensure requested subsystems are available for use */
	for (i = 1, c = 1; i < NVDEV_SUBDEV_NR; i++) {
		if (!(oclass = device->oclass[i]) || (disable & (1ULL << i)))
			continue;

		if (device->subdev[i]) {
			nvkm_object_ref(device->subdev[i], &devobj->subdev[i]);
			continue;
		}

		ret = nvkm_object_ctor(nv_object(device), NULL, oclass,
				       NULL, i, &devobj->subdev[i]);
		if (ret == -ENODEV)
			continue;
		if (ret)
			return ret;

		device->subdev[i] = devobj->subdev[i];

		/* note: can't init *any* subdevs until devinit has been run
		 * due to not knowing exactly what the vbios init tables will
		 * mess with.  devinit also can't be run until all of its
		 * dependencies have been created.
		 *
		 * this code delays init of any subdev until all of devinit's
		 * dependencies have been created, and then initialises each
		 * subdev in turn as they're created.
		 */
		while (i >= NVDEV_SUBDEV_DEVINIT_LAST && c <= i) {
			struct nvkm_object *subdev = devobj->subdev[c++];
			if (subdev && !nv_iclass(subdev, NV_ENGINE_CLASS)) {
				ret = nvkm_object_inc(subdev);
				if (ret)
					return ret;
				atomic_dec(&nv_object(device)->usecount);
			} else
			if (subdev) {
				nvkm_subdev_reset(subdev);
			}
		}
	}

	return 0;
}

static struct nvkm_ofuncs
nvkm_devobj_ofuncs = {
	.ctor = nvkm_devobj_ctor,
	.dtor = nvkm_devobj_dtor,
	.init = _nvkm_parent_init,
	.fini = _nvkm_parent_fini,
	.mthd = nvkm_devobj_mthd,
};

/******************************************************************************
 * nvkm_device: engine functions
 *****************************************************************************/

struct nvkm_device *
nv_device(void *obj)
{
	struct nvkm_object *device = nv_object(obj);
	if (device->engine == NULL) {
		while (device && device->parent)
			device = device->parent;
	} else {
		device = &nv_object(obj)->engine->subdev.object;
		if (device && device->parent)
			device = device->parent;
	}
#if CONFIG_NOUVEAU_DEBUG >= NV_DBG_PARANOIA
	if (unlikely(!device))
		nv_assert("BAD CAST -> NvDevice, 0x%08x\n", nv_hclass(obj));
#endif
	return (void *)device;
}

static struct nvkm_oclass
nvkm_device_sclass[] = {
	{ 0x0080, &nvkm_devobj_ofuncs },
	{}
};

static int
nvkm_device_event_ctor(struct nvkm_object *object, void *data, u32 size,
		       struct nvkm_notify *notify)
{
	if (!WARN_ON(size != 0)) {
		notify->size  = 0;
		notify->types = 1;
		notify->index = 0;
		return 0;
	}
	return -EINVAL;
}

static const struct nvkm_event_func
nvkm_device_event_func = {
	.ctor = nvkm_device_event_ctor,
};

static int
nvkm_device_fini(struct nvkm_object *object, bool suspend)
{
	struct nvkm_device *device = (void *)object;
	struct nvkm_object *subdev;
	int ret, i;

	for (i = NVDEV_SUBDEV_NR - 1; i >= 0; i--) {
		if ((subdev = device->subdev[i])) {
			if (!nv_iclass(subdev, NV_ENGINE_CLASS)) {
				ret = nvkm_object_dec(subdev, suspend);
				if (ret && suspend)
					goto fail;
			}
		}
	}

	ret = nvkm_acpi_fini(device, suspend);
fail:
	for (; ret && i < NVDEV_SUBDEV_NR; i++) {
		if ((subdev = device->subdev[i])) {
			if (!nv_iclass(subdev, NV_ENGINE_CLASS)) {
				ret = nvkm_object_inc(subdev);
				if (ret) {
					/* XXX */
				}
			}
		}
	}

	return ret;
}

static int
nvkm_device_init(struct nvkm_object *object)
{
	struct nvkm_device *device = (void *)object;
	struct nvkm_object *subdev;
	int ret, i = 0;

	ret = nvkm_acpi_init(device);
	if (ret)
		goto fail;

	for (i = 0; i < NVDEV_SUBDEV_NR; i++) {
		if ((subdev = device->subdev[i])) {
			if (!nv_iclass(subdev, NV_ENGINE_CLASS)) {
				ret = nvkm_object_inc(subdev);
				if (ret)
					goto fail;
			} else {
				nvkm_subdev_reset(subdev);
			}
		}
	}

	ret = 0;
fail:
	for (--i; ret && i >= 0; i--) {
		if ((subdev = device->subdev[i])) {
			if (!nv_iclass(subdev, NV_ENGINE_CLASS))
				nvkm_object_dec(subdev, false);
		}
	}

	if (ret)
		nvkm_acpi_fini(device, false);
	return ret;
}

static void
nvkm_device_dtor(struct nvkm_object *object)
{
	struct nvkm_device *device = (void *)object;

	nvkm_event_fini(&device->event);

	mutex_lock(&nv_devices_mutex);
	list_del(&device->head);
	mutex_unlock(&nv_devices_mutex);

	if (nv_subdev(device)->mmio)
		iounmap(nv_subdev(device)->mmio);

	nvkm_engine_destroy(&device->engine);
}

resource_size_t
nv_device_resource_start(struct nvkm_device *device, unsigned int bar)
{
	if (nv_device_is_pci(device)) {
		return pci_resource_start(device->pdev, bar);
	} else {
		struct resource *res;
		res = platform_get_resource(device->platformdev,
					    IORESOURCE_MEM, bar);
		if (!res)
			return 0;
		return res->start;
	}
}

resource_size_t
nv_device_resource_len(struct nvkm_device *device, unsigned int bar)
{
	if (nv_device_is_pci(device)) {
		return pci_resource_len(device->pdev, bar);
	} else {
		struct resource *res;
		res = platform_get_resource(device->platformdev,
					    IORESOURCE_MEM, bar);
		if (!res)
			return 0;
		return resource_size(res);
	}
}

int
nv_device_get_irq(struct nvkm_device *device, bool stall)
{
	if (nv_device_is_pci(device)) {
		return device->pdev->irq;
	} else {
		return platform_get_irq_byname(device->platformdev,
					       stall ? "stall" : "nonstall");
	}
}

static struct nvkm_oclass
nvkm_device_oclass = {
	.handle = NV_ENGINE(DEVICE, 0x00),
	.ofuncs = &(struct nvkm_ofuncs) {
		.dtor = nvkm_device_dtor,
		.init = nvkm_device_init,
		.fini = nvkm_device_fini,
	},
};

int
nvkm_device_create_(void *dev, enum nv_bus_type type, u64 name,
		    const char *sname, const char *cfg, const char *dbg,
		    int length, void **pobject)
{
	struct nvkm_device *device;
	int ret = -EEXIST;

	mutex_lock(&nv_devices_mutex);
	list_for_each_entry(device, &nv_devices, head) {
		if (device->handle == name)
			goto done;
	}

	ret = nvkm_engine_create_(NULL, NULL, &nvkm_device_oclass, true,
				  "DEVICE", "device", length, pobject);
	device = *pobject;
	if (ret)
		goto done;

	switch (type) {
	case NVKM_BUS_PCI:
		device->pdev = dev;
		break;
	case NVKM_BUS_PLATFORM:
		device->platformdev = dev;
		break;
	}
	device->handle = name;
	device->cfgopt = cfg;
	device->dbgopt = dbg;
	device->name = sname;

	nv_subdev(device)->debug = nvkm_dbgopt(device->dbgopt, "DEVICE");
	nv_engine(device)->sclass = nvkm_device_sclass;
	list_add(&device->head, &nv_devices);

	ret = nvkm_event_init(&nvkm_device_event_func, 1, 1, &device->event);
done:
	mutex_unlock(&nv_devices_mutex);
	return ret;
}
