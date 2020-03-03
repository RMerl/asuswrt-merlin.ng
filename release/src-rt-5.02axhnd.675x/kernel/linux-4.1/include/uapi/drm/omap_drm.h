/*
 * include/uapi/drm/omap_drm.h
 *
 * Copyright (C) 2011 Texas Instruments
 * Author: Rob Clark <rob@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __OMAP_DRM_H__
#define __OMAP_DRM_H__

#include <drm/drm.h>

/* Please note that modifications to all structs defined here are
 * subject to backwards-compatibility constraints.
 */

#define OMAP_PARAM_CHIPSET_ID	1	/* ie. 0x3430, 0x4430, etc */

struct drm_omap_param {
	uint64_t param;			/* in */
	uint64_t value;			/* in (set_param), out (get_param) */
};

#define OMAP_BO_SCANOUT		0x00000001	/* scanout capable (phys contiguous) */
#define OMAP_BO_CACHE_MASK	0x00000006	/* cache type mask, see cache modes */
#define OMAP_BO_TILED_MASK	0x00000f00	/* tiled mapping mask, see tiled modes */

/* cache modes */
#define OMAP_BO_CACHED		0x00000000	/* default */
#define OMAP_BO_WC		0x00000002	/* write-combine */
#define OMAP_BO_UNCACHED	0x00000004	/* strongly-ordered (uncached) */

/* tiled modes */
#define OMAP_BO_TILED_8		0x00000100
#define OMAP_BO_TILED_16	0x00000200
#define OMAP_BO_TILED_32	0x00000300
#define OMAP_BO_TILED		(OMAP_BO_TILED_8 | OMAP_BO_TILED_16 | OMAP_BO_TILED_32)

union omap_gem_size {
	uint32_t bytes;		/* (for non-tiled formats) */
	struct {
		uint16_t width;
		uint16_t height;
	} tiled;		/* (for tiled formats) */
};

struct drm_omap_gem_new {
	union omap_gem_size size;	/* in */
	uint32_t flags;			/* in */
	uint32_t handle;		/* out */
	uint32_t __pad;
};

/* mask of operations: */
enum omap_gem_op {
	OMAP_GEM_READ = 0x01,
	OMAP_GEM_WRITE = 0x02,
};

struct drm_omap_gem_cpu_prep {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t op;			/* mask of omap_gem_op (in) */
};

struct drm_omap_gem_cpu_fini {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t op;			/* mask of omap_gem_op (in) */
	/* TODO maybe here we pass down info about what regions are touched
	 * by sw so we can be clever about cache ops?  For now a placeholder,
	 * set to zero and we just do full buffer flush..
	 */
	uint32_t nregions;
	uint32_t __pad;
};

struct drm_omap_gem_info {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t pad;
	uint64_t offset;		/* mmap offset (out) */
	/* note: in case of tiled buffers, the user virtual size can be
	 * different from the physical size (ie. how many pages are needed
	 * to back the object) which is returned in DRM_IOCTL_GEM_OPEN..
	 * This size here is the one that should be used if you want to
	 * mmap() the buffer:
	 */
	uint32_t size;			/* virtual size for mmap'ing (out) */
	uint32_t __pad;
};

#define DRM_OMAP_GET_PARAM		0x00
#define DRM_OMAP_SET_PARAM		0x01
/* placeholder for plugin-api
#define DRM_OMAP_GET_BASE		0x02
*/
#define DRM_OMAP_GEM_NEW		0x03
#define DRM_OMAP_GEM_CPU_PREP		0x04
#define DRM_OMAP_GEM_CPU_FINI		0x05
#define DRM_OMAP_GEM_INFO		0x06
#define DRM_OMAP_NUM_IOCTLS		0x07

#define DRM_IOCTL_OMAP_GET_PARAM	DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GET_PARAM, struct drm_omap_param)
#define DRM_IOCTL_OMAP_SET_PARAM	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_SET_PARAM, struct drm_omap_param)
/* placeholder for plugin-api
#define DRM_IOCTL_OMAP_GET_BASE		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GET_BASE, struct drm_omap_get_base)
*/
#define DRM_IOCTL_OMAP_GEM_NEW		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GEM_NEW, struct drm_omap_gem_new)
#define DRM_IOCTL_OMAP_GEM_CPU_PREP	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_GEM_CPU_PREP, struct drm_omap_gem_cpu_prep)
#define DRM_IOCTL_OMAP_GEM_CPU_FINI	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_GEM_CPU_FINI, struct drm_omap_gem_cpu_fini)
#define DRM_IOCTL_OMAP_GEM_INFO		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GEM_INFO, struct drm_omap_gem_info)

#endif /* __OMAP_DRM_H__ */
