/*
 * lib/idiag/idiagnl_meminfo_obj.c Inet Diag Meminfo Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#include <netlink-private/netlink.h>
#include <netlink/idiag/meminfo.h>

/**
 * @ingroup idiag
 * @defgroup idiagnl_meminfo Inet Diag Memory Info
 *
 * @details
 * @idiagnl_doc{idiagnl_meminfo, Inet Diag Memory Info Documentation}
 * @{
 */
struct idiagnl_meminfo *idiagnl_meminfo_alloc(void)
{
	return (struct idiagnl_meminfo *) nl_object_alloc(&idiagnl_meminfo_obj_ops);
}

void idiagnl_meminfo_get(struct idiagnl_meminfo *minfo)
{
	nl_object_get((struct nl_object *) minfo);
}

void idiagnl_meminfo_put(struct idiagnl_meminfo *minfo)
{
	nl_object_put((struct nl_object *) minfo);
}

/**
 * @name Attributes
 * @{
 */
uint32_t idiagnl_meminfo_get_rmem(const struct idiagnl_meminfo *minfo)
{
	return minfo->idiag_rmem;
}

void idiagnl_meminfo_set_rmem(struct idiagnl_meminfo *minfo, uint32_t rmem)
{
	minfo->idiag_rmem = rmem;
}

uint32_t idiagnl_meminfo_get_wmem(const struct idiagnl_meminfo *minfo)
{
	return minfo->idiag_wmem;
}

void idiagnl_meminfo_set_wmem(struct idiagnl_meminfo *minfo, uint32_t wmem)
{
	minfo->idiag_wmem = wmem;
}

uint32_t idiagnl_meminfo_get_fmem(const struct idiagnl_meminfo *minfo)
{
	return minfo->idiag_fmem;
}

void idiagnl_meminfo_set_fmem(struct idiagnl_meminfo *minfo, uint32_t fmem)
{
	minfo->idiag_fmem = fmem;
}

uint32_t idiagnl_meminfo_get_tmem(const struct idiagnl_meminfo *minfo)
{
	return minfo->idiag_tmem;
}

void idiagnl_meminfo_set_tmem(struct idiagnl_meminfo *minfo, uint32_t tmem)
{
	minfo->idiag_tmem = tmem;
}
/** @} */

static int idiagnl_meminfo_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct idiagnl_meminfo *dst = (struct idiagnl_meminfo *) _dst;
	struct idiagnl_meminfo *src = (struct idiagnl_meminfo *) _src;

	memcpy(dst, src, sizeof(struct idiagnl_meminfo));

	return 0;
}

/** @cond SKIP */
struct nl_object_ops idiagnl_meminfo_obj_ops = {
	.oo_name	= "idiag/idiag_meminfo",
	.oo_size	= sizeof(struct idiagnl_meminfo),
	.oo_clone	= idiagnl_meminfo_clone,
};
/** @endcond */
/** @} */
