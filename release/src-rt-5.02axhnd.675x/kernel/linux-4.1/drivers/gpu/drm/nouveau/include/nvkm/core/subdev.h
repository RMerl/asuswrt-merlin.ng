#ifndef __NVKM_SUBDEV_H__
#define __NVKM_SUBDEV_H__
#include <core/object.h>
#include <core/devidx.h>

#define NV_SUBDEV_(sub,var) (NV_SUBDEV_CLASS | ((var) << 8) | (sub))
#define NV_SUBDEV(name,var)  NV_SUBDEV_(NVDEV_SUBDEV_##name, (var))

struct nvkm_subdev {
	struct nvkm_object object;
	struct mutex mutex;
	const char *name;
	void __iomem *mmio;
	u32 debug;
	u32 unit;

	void (*intr)(struct nvkm_subdev *);
};

static inline struct nvkm_subdev *
nv_subdev(void *obj)
{
#if CONFIG_NOUVEAU_DEBUG >= NV_DBG_PARANOIA
	if (unlikely(!nv_iclass(obj, NV_SUBDEV_CLASS)))
		nv_assert("BAD CAST -> NvSubDev, %08x", nv_hclass(obj));
#endif
	return obj;
}

static inline int
nv_subidx(struct nvkm_subdev *subdev)
{
	return nv_hclass(subdev) & 0xff;
}

struct nvkm_subdev *nvkm_subdev(void *obj, int idx);

#define nvkm_subdev_create(p,e,o,v,s,f,d)                                   \
	nvkm_subdev_create_((p), (e), (o), (v), (s), (f),                   \
			       sizeof(**d),(void **)d)

int  nvkm_subdev_create_(struct nvkm_object *, struct nvkm_object *,
			    struct nvkm_oclass *, u32 pclass,
			    const char *sname, const char *fname,
			    int size, void **);
void nvkm_subdev_destroy(struct nvkm_subdev *);
int  nvkm_subdev_init(struct nvkm_subdev *);
int  nvkm_subdev_fini(struct nvkm_subdev *, bool suspend);
void nvkm_subdev_reset(struct nvkm_object *);

void _nvkm_subdev_dtor(struct nvkm_object *);
int  _nvkm_subdev_init(struct nvkm_object *);
int  _nvkm_subdev_fini(struct nvkm_object *, bool suspend);

#define s_printk(s,l,f,a...) do {                                              \
	if ((s)->debug >= OS_DBG_##l) {                                        \
		nv_printk((s)->base.parent, (s)->name, l, f, ##a);             \
	}                                                                      \
} while(0)

static inline u8
nv_rd08(void *obj, u32 addr)
{
	struct nvkm_subdev *subdev = nv_subdev(obj);
	u8 data = ioread8(subdev->mmio + addr);
	nv_spam(subdev, "nv_rd08 0x%06x 0x%02x\n", addr, data);
	return data;
}

static inline u16
nv_rd16(void *obj, u32 addr)
{
	struct nvkm_subdev *subdev = nv_subdev(obj);
	u16 data = ioread16_native(subdev->mmio + addr);
	nv_spam(subdev, "nv_rd16 0x%06x 0x%04x\n", addr, data);
	return data;
}

static inline u32
nv_rd32(void *obj, u32 addr)
{
	struct nvkm_subdev *subdev = nv_subdev(obj);
	u32 data = ioread32_native(subdev->mmio + addr);
	nv_spam(subdev, "nv_rd32 0x%06x 0x%08x\n", addr, data);
	return data;
}

static inline void
nv_wr08(void *obj, u32 addr, u8 data)
{
	struct nvkm_subdev *subdev = nv_subdev(obj);
	nv_spam(subdev, "nv_wr08 0x%06x 0x%02x\n", addr, data);
	iowrite8(data, subdev->mmio + addr);
}

static inline void
nv_wr16(void *obj, u32 addr, u16 data)
{
	struct nvkm_subdev *subdev = nv_subdev(obj);
	nv_spam(subdev, "nv_wr16 0x%06x 0x%04x\n", addr, data);
	iowrite16_native(data, subdev->mmio + addr);
}

static inline void
nv_wr32(void *obj, u32 addr, u32 data)
{
	struct nvkm_subdev *subdev = nv_subdev(obj);
	nv_spam(subdev, "nv_wr32 0x%06x 0x%08x\n", addr, data);
	iowrite32_native(data, subdev->mmio + addr);
}

static inline u32
nv_mask(void *obj, u32 addr, u32 mask, u32 data)
{
	u32 temp = nv_rd32(obj, addr);
	nv_wr32(obj, addr, (temp & ~mask) | data);
	return temp;
}
#endif
