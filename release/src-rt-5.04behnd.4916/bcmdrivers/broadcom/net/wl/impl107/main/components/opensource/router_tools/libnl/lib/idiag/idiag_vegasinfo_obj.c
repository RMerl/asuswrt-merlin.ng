/*
 * lib/idiag/idiagnl_vegasinfo_obj.c Inet Diag TCP Vegas Info Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#include <netlink-private/netlink.h>
#include <netlink/idiag/vegasinfo.h>

/**
 * @ingroup idiag
 * @defgroup idiagnl_vegasinfo Inet Diag TCP Vegas Info
 *
 * @details
 * @idiagnl_doc{idiagnl_vegasinfo, Inet Diag TCP Vegas Info Documentation}
 * @{
 */
struct idiagnl_vegasinfo *idiagnl_vegasinfo_alloc(void)
{
	return (struct idiagnl_vegasinfo *) nl_object_alloc(&idiagnl_vegasinfo_obj_ops);
}

void idiagnl_vegasinfo_get(struct idiagnl_vegasinfo *vinfo)
{
	nl_object_get((struct nl_object *) vinfo);
}

void idiagnl_vegasinfo_put(struct idiagnl_vegasinfo *vinfo)
{
	nl_object_put((struct nl_object *) vinfo);
}

/**
 * @name Attributes
 * @{
 */
uint32_t idiagnl_vegasinfo_get_enabled(const struct idiagnl_vegasinfo *vinfo)
{
	return vinfo->tcpv_enabled;
}

void idiagnl_vegasinfo_set_enabled(struct idiagnl_vegasinfo *vinfo, uint32_t
		enabled)
{
	vinfo->tcpv_enabled = enabled;
}

uint32_t idiagnl_vegasinfo_get_rttcnt(const struct idiagnl_vegasinfo *vinfo)
{
	return vinfo->tcpv_rttcnt;
}

void idiagnl_vegasinfo_set_rttcnt(struct idiagnl_vegasinfo *vinfo, uint32_t
		rttcnt)
{
	vinfo->tcpv_rttcnt = rttcnt;
}

uint32_t idiagnl_vegasinfo_get_rtt(const struct idiagnl_vegasinfo *vinfo)
{
	return vinfo->tcpv_rtt;
}

void idiagnl_vegasinfo_set_rtt(struct idiagnl_vegasinfo *vinfo, uint32_t rtt)
{
	vinfo->tcpv_rtt = rtt;
}

uint32_t idiagnl_vegasinfo_get_minrtt(const struct idiagnl_vegasinfo *vinfo)
{
	return vinfo->tcpv_minrtt;
}

void idiagnl_vegasinfo_set_minrtt(struct idiagnl_vegasinfo *vinfo, uint32_t
		minrtt)
{
	vinfo->tcpv_minrtt = minrtt;
}
/** @} */

static int idiagnl_vegasinfo_clone(struct nl_object *_dst,
                                   struct nl_object *_src)
{
	struct idiagnl_vegasinfo *dst = (struct idiagnl_vegasinfo *) _dst;
	struct idiagnl_vegasinfo *src = (struct idiagnl_vegasinfo *) _src;

	memcpy(dst, src, sizeof(struct idiagnl_vegasinfo));

	return 0;
}

/** @cond SKIP */
struct nl_object_ops idiagnl_vegasinfo_obj_ops = {
	.oo_name	= "idiag/idiag_vegasinfo",
	.oo_size	= sizeof(struct idiagnl_vegasinfo),
	.oo_clone	= idiagnl_vegasinfo_clone,
};
/** @endcond */
/** @} */
