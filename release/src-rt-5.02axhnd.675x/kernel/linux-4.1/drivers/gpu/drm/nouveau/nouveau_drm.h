#ifndef __NOUVEAU_DRMCLI_H__
#define __NOUVEAU_DRMCLI_H__

#define DRIVER_AUTHOR		"Nouveau Project"
#define DRIVER_EMAIL		"nouveau@lists.freedesktop.org"

#define DRIVER_NAME		"nouveau"
#define DRIVER_DESC		"nVidia Riva/TNT/GeForce/Quadro/Tesla"
#define DRIVER_DATE		"20120801"

#define DRIVER_MAJOR		1
#define DRIVER_MINOR		2
#define DRIVER_PATCHLEVEL	2

/*
 * 1.1.1:
 * 	- added support for tiled system memory buffer objects
 *      - added support for NOUVEAU_GETPARAM_GRAPH_UNITS on [nvc0,nve0].
 *      - added support for compressed memory storage types on [nvc0,nve0].
 *      - added support for software methods 0x600,0x644,0x6ac on nvc0
 *        to control registers on the MPs to enable performance counters,
 *        and to control the warp error enable mask (OpenGL requires out of
 *        bounds access to local memory to be silently ignored / return 0).
 * 1.1.2:
 *      - fixes multiple bugs in flip completion events and timestamping
 * 1.2.0:
 * 	- object api exposed to userspace
 * 	- fermi,kepler,maxwell zbc
 * 1.2.1:
 *      - allow concurrent access to bo's mapped read/write.
 * 1.2.2:
 *      - add NOUVEAU_GEM_DOMAIN_COHERENT flag
 */

#include <nvif/client.h>
#include <nvif/device.h>

#include <drmP.h>

#include <drm/ttm/ttm_bo_api.h>
#include <drm/ttm/ttm_bo_driver.h>
#include <drm/ttm/ttm_placement.h>
#include <drm/ttm/ttm_memory.h>
#include <drm/ttm/ttm_module.h>
#include <drm/ttm/ttm_page_alloc.h>

#include "uapi/drm/nouveau_drm.h"

struct nouveau_channel;
struct platform_device;

#define DRM_FILE_PAGE_OFFSET (0x100000000ULL >> PAGE_SHIFT)

#include "nouveau_fence.h"
#include "nouveau_bios.h"

struct nouveau_drm_tile {
	struct nouveau_fence *fence;
	bool used;
};

enum nouveau_drm_object_route {
	NVDRM_OBJECT_NVIF = 0,
	NVDRM_OBJECT_USIF,
	NVDRM_OBJECT_ABI16,
};

enum nouveau_drm_notify_route {
	NVDRM_NOTIFY_NVIF = 0,
	NVDRM_NOTIFY_USIF
};

enum nouveau_drm_handle {
	NVDRM_CLIENT  = 0xffffffff,
	NVDRM_DEVICE  = 0xdddddddd,
	NVDRM_CONTROL = 0xdddddddc,
	NVDRM_DISPLAY = 0xd1500000,
	NVDRM_PUSH    = 0xbbbb0000, /* |= client chid */
	NVDRM_CHAN    = 0xcccc0000, /* |= client chid */
	NVDRM_NVSW    = 0x55550000,
};

struct nouveau_cli {
	struct nvif_client base;
	struct nvkm_vm *vm; /*XXX*/
	struct list_head head;
	struct mutex mutex;
	void *abi16;
	struct list_head objects;
	struct list_head notifys;
};

static inline struct nouveau_cli *
nouveau_cli(struct drm_file *fpriv)
{
	return fpriv ? fpriv->driver_priv : NULL;
}

#include <nvif/object.h>
#include <nvif/device.h>

extern int nouveau_runtime_pm;

struct nouveau_drm {
	struct nouveau_cli client;
	struct drm_device *dev;

	struct nvif_device device;
	struct list_head clients;

	struct {
		enum {
			UNKNOWN = 0,
			DISABLE = 1,
			ENABLED = 2
		} stat;
		u32 base;
		u32 size;
	} agp;

	/* TTM interface support */
	struct {
		struct drm_global_reference mem_global_ref;
		struct ttm_bo_global_ref bo_global_ref;
		struct ttm_bo_device bdev;
		atomic_t validate_sequence;
		int (*move)(struct nouveau_channel *,
			    struct ttm_buffer_object *,
			    struct ttm_mem_reg *, struct ttm_mem_reg *);
		struct nouveau_channel *chan;
		struct nvif_object copy;
		int mtrr;
	} ttm;

	/* GEM interface support */
	struct {
		u64 vram_available;
		u64 gart_available;
	} gem;

	/* synchronisation */
	void *fence;

	/* context for accelerated drm-internal operations */
	struct nouveau_channel *cechan;
	struct nouveau_channel *channel;
	struct nvkm_gpuobj *notify;
	struct nouveau_fbdev *fbcon;
	struct nvif_object nvsw;
	struct nvif_object ntfy;

	/* nv10-nv40 tiling regions */
	struct {
		struct nouveau_drm_tile reg[15];
		spinlock_t lock;
	} tile;

	/* modesetting */
	struct nvbios vbios;
	struct nouveau_display *display;
	struct backlight_device *backlight;

	/* power management */
	struct nouveau_hwmon *hwmon;
	struct nouveau_sysfs *sysfs;

	/* display power reference */
	bool have_disp_power_ref;

	struct dev_pm_domain vga_pm_domain;
	struct pci_dev *hdmi_device;
};

static inline struct nouveau_drm *
nouveau_drm(struct drm_device *dev)
{
	return dev->dev_private;
}

int nouveau_pmops_suspend(struct device *);
int nouveau_pmops_resume(struct device *);

#define nouveau_platform_device_create(p, u)                                   \
	nouveau_platform_device_create_(p, sizeof(**u), (void **)u)
struct drm_device *
nouveau_platform_device_create_(struct platform_device *pdev,
				int size, void **pobject);
void nouveau_drm_device_remove(struct drm_device *dev);

#define NV_PRINTK(l,c,f,a...) do {                                             \
	struct nouveau_cli *_cli = (c);                                        \
	nv_##l(_cli->base.base.priv, f, ##a);                                  \
} while(0)
#define NV_FATAL(drm,f,a...) NV_PRINTK(fatal, &(drm)->client, f, ##a)
#define NV_ERROR(drm,f,a...) NV_PRINTK(error, &(drm)->client, f, ##a)
#define NV_WARN(drm,f,a...) NV_PRINTK(warn, &(drm)->client, f, ##a)
#define NV_INFO(drm,f,a...) NV_PRINTK(info, &(drm)->client, f, ##a)
#define NV_DEBUG(drm,f,a...) NV_PRINTK(debug, &(drm)->client, f, ##a)

extern int nouveau_modeset;

#endif
