/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * Based on the original work in Linux by
 * Copyright (c) 2006  SUSE Linux Products GmbH
 * Copyright (c) 2006  Tejun Heo <teheo@suse.de>
 * Copyright 2019 Google LLC
 */

#ifndef _DM_DEVRES_H
#define _DM_DEVRES_H

#include <linux/compat.h>

struct udevice;

/* device resource management */
typedef void (*dr_release_t)(struct udevice *dev, void *res);
typedef int (*dr_match_t)(struct udevice *dev, void *res, void *match_data);

/**
 * struct devres_stats - Information about devres allocations for a device
 *
 * @allocs: Number of allocations
 * @total_size: Total size of allocations in bytes
 */
struct devres_stats {
	int allocs;
	int total_size;
};

#if CONFIG_IS_ENABLED(DEVRES)

#ifdef CONFIG_DEBUG_DEVRES
void *__devres_alloc(dr_release_t release, size_t size, gfp_t gfp,
		     const char *name);
#define _devres_alloc(release, size, gfp) \
	__devres_alloc(release, size, gfp, #release)
#else
void *_devres_alloc(dr_release_t release, size_t size, gfp_t gfp);
#endif

/**
 * devres_alloc() - Allocate device resource data
 * @release: Release function devres will be associated with
 * @size: Allocation size
 * @gfp: Allocation flags
 *
 * Allocate devres of @size bytes.  The allocated area is associated
 * with @release.  The returned pointer can be passed to
 * other devres_*() functions.
 *
 * Return:
 * Pointer to allocated devres on success, NULL on failure.
 */
#define devres_alloc(release, size, gfp) \
	_devres_alloc(release, size, (gfp) | __GFP_ZERO)

/**
 * devres_free() - Free device resource data
 * @res: Pointer to devres data to free
 *
 * Free devres created with devres_alloc().
 */
void devres_free(void *res);

/**
 * devres_add() - Register device resource
 * @dev: Device to add resource to
 * @res: Resource to register
 *
 * Register devres @res to @dev.  @res should have been allocated
 * using devres_alloc().  On driver detach, the associated release
 * function will be invoked and devres will be freed automatically.
 */
void devres_add(struct udevice *dev, void *res);

/**
 * devres_find() - Find device resource
 * @dev: Device to lookup resource from
 * @release: Look for resources associated with this release function
 * @match: Match function (optional)
 * @match_data: Data for the match function
 *
 * Find the latest devres of @dev which is associated with @release
 * and for which @match returns 1.  If @match is NULL, it's considered
 * to match all.
 *
 * Return: pointer to found devres, NULL if not found.
 */
void *devres_find(struct udevice *dev, dr_release_t release,
		  dr_match_t match, void *match_data);

/**
 * devres_get() - Find devres, if non-existent, add one atomically
 * @dev: Device to lookup or add devres for
 * @new_res: Pointer to new initialized devres to add if not found
 * @match: Match function (optional)
 * @match_data: Data for the match function
 *
 * Find the latest devres of @dev which has the same release function
 * as @new_res and for which @match return 1.  If found, @new_res is
 * freed; otherwise, @new_res is added atomically.
 *
 * Return: pointer to found or added devres.
 */
void *devres_get(struct udevice *dev, void *new_res,
		 dr_match_t match, void *match_data);

/**
 * devres_remove() - Find a device resource and remove it
 * @dev: Device to find resource from
 * @release: Look for resources associated with this release function
 * @match: Match function (optional)
 * @match_data: Data for the match function
 *
 * Find the latest devres of @dev associated with @release and for
 * which @match returns 1.  If @match is NULL, it's considered to
 * match all.  If found, the resource is removed atomically and
 * returned.
 *
 * Return: pointer to removed devres on success, NULL if not found.
 */
void *devres_remove(struct udevice *dev, dr_release_t release,
		    dr_match_t match, void *match_data);

/**
 * devres_destroy() - Find a device resource and destroy it
 * @dev: Device to find resource from
 * @release: Look for resources associated with this release function
 * @match: Match function (optional)
 * @match_data: Data for the match function
 *
 * Find the latest devres of @dev associated with @release and for
 * which @match returns 1.  If @match is NULL, it's considered to
 * match all.  If found, the resource is removed atomically and freed.
 *
 * Note that the release function for the resource will not be called,
 * only the devres-allocated data will be freed.  The caller becomes
 * responsible for freeing any other data.
 *
 * Return: 0 if devres is found and freed, -ENOENT if not found.
 */
int devres_destroy(struct udevice *dev, dr_release_t release,
		   dr_match_t match, void *match_data);

/**
 * devres_release() - Find a device resource and destroy it, calling release
 * @dev: Device to find resource from
 * @release: Look for resources associated with this release function
 * @match: Match function (optional)
 * @match_data: Data for the match function
 *
 * Find the latest devres of @dev associated with @release and for
 * which @match returns 1.  If @match is NULL, it's considered to
 * match all.  If found, the resource is removed atomically, the
 * release function called and the resource freed.
 *
 * Return: 0 if devres is found and freed, -ENOENT if not found.
 */
int devres_release(struct udevice *dev, dr_release_t release,
		   dr_match_t match, void *match_data);

/* managed devm_k.alloc/kfree for device drivers */
/**
 * devm_kmalloc() - Resource-managed kmalloc
 * @dev: Device to allocate memory for
 * @size: Allocation size
 * @gfp: Allocation gfp flags
 *
 * Managed kmalloc.  Memory allocated with this function is
 * automatically freed on driver detach.  Like all other devres
 * resources, guaranteed alignment is unsigned long long.
 *
 * Return: pointer to allocated memory on success, NULL on failure.
 */
void *devm_kmalloc(struct udevice *dev, size_t size, gfp_t gfp);
static inline void *devm_kzalloc(struct udevice *dev, size_t size, gfp_t gfp)
{
	return devm_kmalloc(dev, size, gfp | __GFP_ZERO);
}

static inline void *devm_kmalloc_array(struct udevice *dev,
				       size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > SIZE_MAX / size)
		return NULL;
	return devm_kmalloc(dev, n * size, flags);
}

static inline void *devm_kcalloc(struct udevice *dev,
				 size_t n, size_t size, gfp_t flags)
{
	return devm_kmalloc_array(dev, n, size, flags | __GFP_ZERO);
}

/**
 * devm_kfree() - Resource-managed kfree
 * @dev: Device this memory belongs to
 * @ptr: Memory to free
 *
 * Free memory allocated with devm_kmalloc().
 */
void devm_kfree(struct udevice *dev, void *ptr);

/* Get basic stats on allocations */
void devres_get_stats(const struct udevice *dev, struct devres_stats *stats);

#else /* ! DEVRES */

static inline void *devres_alloc(dr_release_t release, size_t size, gfp_t gfp)
{
	return kzalloc(size, gfp);
}

static inline void devres_free(void *res)
{
	kfree(res);
}

static inline void devres_add(struct udevice *dev, void *res)
{
}

static inline void *devres_find(struct udevice *dev, dr_release_t release,
				dr_match_t match, void *match_data)
{
	return NULL;
}

static inline void *devres_get(struct udevice *dev, void *new_res,
			       dr_match_t match, void *match_data)
{
	return NULL;
}

static inline void *devres_remove(struct udevice *dev, dr_release_t release,
				  dr_match_t match, void *match_data)
{
	return NULL;
}

static inline int devres_destroy(struct udevice *dev, dr_release_t release,
				 dr_match_t match, void *match_data)
{
	return 0;
}

static inline int devres_release(struct udevice *dev, dr_release_t release,
				 dr_match_t match, void *match_data)
{
	return 0;
}

static inline void *devm_kmalloc(struct udevice *dev, size_t size, gfp_t gfp)
{
	return kmalloc(size, gfp);
}

static inline void *devm_kzalloc(struct udevice *dev, size_t size, gfp_t gfp)
{
	return kzalloc(size, gfp);
}

static inline void *devm_kmalloc_array(struct udevice *dev,
				       size_t n, size_t size, gfp_t flags)
{
	/* TODO: add kmalloc_array() to linux/compat.h */
	if (size != 0 && n > SIZE_MAX / size)
		return NULL;
	return kmalloc(n * size, flags);
}

static inline void *devm_kcalloc(struct udevice *dev,
				 size_t n, size_t size, gfp_t flags)
{
	/* TODO: add kcalloc() to linux/compat.h */
	return kmalloc(n * size, flags | __GFP_ZERO);
}

static inline void devm_kfree(struct udevice *dev, void *ptr)
{
	kfree(ptr);
}

static inline void devres_get_stats(const struct udevice *dev,
				    struct devres_stats *stats)
{
}

#endif /* DEVRES */
#endif /* _DM_DEVRES_H */
