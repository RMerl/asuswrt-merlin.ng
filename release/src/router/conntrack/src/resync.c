/*
 * (C) 2006-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011 by Vyatta Inc. <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "conntrackd.h"
#include "network.h"
#include "log.h"
#include "queue_tx.h"
#include "resync.h"
#include "cache.h"

void resync_req(void)
{
	dlog(LOG_NOTICE, "resync requested");
	tx_queue_add_ctlmsg(NET_F_RESYNC, 0, 0);
}

void resync_send(int (*do_cache_to_tx)(void *data1, void *data2))
{
	dlog(LOG_NOTICE, "sending bulk update");
	cache_iterate(STATE(mode)->internal->ct.data,
		      NULL, do_cache_to_tx);
	cache_iterate(STATE(mode)->internal->exp.data,
		      NULL, do_cache_to_tx);
}

void resync_at_startup(void)
{
	if (CONFIG(startup_resync) == 0)
		return;

	resync_req();
}
