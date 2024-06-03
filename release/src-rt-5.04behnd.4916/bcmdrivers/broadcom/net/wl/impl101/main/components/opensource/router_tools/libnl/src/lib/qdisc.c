/*
 * src/lib/qdisc.c     CLI QDisc Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cli
 * @defgroup cli_qdisc Queueing Disciplines
 * @{
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/qdisc.h>
#include <netlink/route/class.h>

struct rtnl_qdisc *nl_cli_qdisc_alloc(void)
{
	struct rtnl_qdisc *qdisc;

	if (!(qdisc = rtnl_qdisc_alloc()))
		nl_cli_fatal(ENOMEM, "Unable to allocate qdisc object");

	return qdisc;
}

/** @} */
