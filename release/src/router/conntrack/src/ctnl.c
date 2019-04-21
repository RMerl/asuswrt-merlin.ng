/*
 * (C) 2006-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
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
 *
 * Part of this code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include "conntrackd.h"
#include "netlink.h"
#include "filter.h"
#include "log.h"
#include "alarm.h"
#include "fds.h"
#include "traffic_stats.h"
#include "process.h"
#include "origin.h"
#include "date.h"
#include "internal.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

void ctnl_kill(void)
{
	if (!(CONFIG(flags) & CTD_POLL))
		nfct_close(STATE(event));

	nfct_close(STATE(resync));
	nfct_close(STATE(get));
	origin_unregister(STATE(flush));
	nfct_close(STATE(flush));

	if (STATE(us_filter))
		ct_filter_destroy(STATE(us_filter));
	STATE(mode)->kill();

	if (STATE(mode)->internal->flags & INTERNAL_F_POPULATE) {
		nfct_close(STATE(dump));
	}
}

static void local_flush_master(void)
{
	STATE(stats).nl_kernel_table_flush++;
	dlog(LOG_NOTICE, "flushing kernel conntrack table");

	/* fork a child process that performs the flush operation,
	 * meanwhile the parent process handles events. */
	if (fork_process_new(CTD_PROC_FLUSH, CTD_PROC_F_EXCL,
			     NULL, NULL) == 0) {
		nl_flush_conntrack_table_selective();
		exit(EXIT_SUCCESS);
	}
}

static void local_resync_master(void)
{
	if (STATE(mode)->internal->flags & INTERNAL_F_POPULATE) {
		STATE(stats).nl_kernel_table_resync++;
		dlog(LOG_NOTICE, "resync with master conntrack table");
		nl_dump_conntrack_table(STATE(dump));
	} else {
		dlog(LOG_NOTICE, "resync is unsupported in this mode");
	}
}

static void local_exp_flush_master(void)
{
	if (!(CONFIG(flags) & CTD_EXPECT))
		return;

	STATE(stats).nl_kernel_table_flush++;
	dlog(LOG_NOTICE, "flushing kernel expect table");

	/* fork a child process that performs the flush operation,
	 * meanwhile the parent process handles events. */
	if (fork_process_new(CTD_PROC_FLUSH, CTD_PROC_F_EXCL,
			     NULL, NULL) == 0) {
		nl_flush_expect_table(STATE(flush));
		exit(EXIT_SUCCESS);
	}
}

static void local_exp_resync_master(void)
{
	if (!(CONFIG(flags) & CTD_EXPECT))
		return;

	if (STATE(mode)->internal->flags & INTERNAL_F_POPULATE) {
		STATE(stats).nl_kernel_table_resync++;
		dlog(LOG_NOTICE, "resync with master expect table");
		nl_dump_expect_table(STATE(dump));
	} else {
		dlog(LOG_NOTICE, "resync is unsupported in this mode");
	}
}

int ctnl_local(int fd, int type, void *data)
{
	int ret = LOCAL_RET_OK;

	switch(type) {
	case CT_FLUSH_MASTER:
		local_flush_master();
		break;
	case CT_RESYNC_MASTER:
		local_resync_master();
		break;
	case EXP_FLUSH_MASTER:
		local_exp_flush_master();
		break;
	case EXP_RESYNC_MASTER:
		local_exp_resync_master();
		break;
	case ALL_FLUSH_MASTER:
		local_flush_master();
		local_exp_flush_master();
		break;
	case ALL_RESYNC_MASTER:
		local_resync_master();
		local_exp_resync_master();
		break;
	}

	ret = STATE(mode)->local(fd, type, data);
	if (ret == LOCAL_RET_ERROR) {
		STATE(stats).local_unknown_request++;
		return LOCAL_RET_ERROR;
	}
	return ret;
}

static void do_overrun_resync_alarm(struct alarm_block *a, void *data)
{
	nl_send_resync(STATE(resync));
	STATE(stats).nl_kernel_table_resync++;
}

static void do_polling_alarm(struct alarm_block *a, void *data)
{
	if (STATE(mode)->internal->ct.purge)
		STATE(mode)->internal->ct.purge();

	if (STATE(mode)->internal->exp.purge)
		STATE(mode)->internal->exp.purge();

	nl_send_resync(STATE(resync));
	if (CONFIG(flags) & CTD_EXPECT)
		nl_send_expect_resync(STATE(resync));

	add_alarm(&STATE(polling_alarm), CONFIG(poll_kernel_secs), 0);
}

static int event_handler(const struct nlmsghdr *nlh,
			 enum nf_conntrack_msg_type type,
			 struct nf_conntrack *ct,
			 void *data)
{
	int origin_type;

	STATE(stats).nl_events_received++;

	/* skip user-space filtering if already do it in the kernel */
	if (ct_filter_conntrack(ct, !CONFIG(filter_from_kernelspace))) {
		STATE(stats).nl_events_filtered++;
		goto out;
	}

	origin_type = origin_find(nlh);

	switch(type) {
	case NFCT_T_NEW:
		STATE(mode)->internal->ct.new(ct, origin_type);
		break;
	case NFCT_T_UPDATE:
		STATE(mode)->internal->ct.upd(ct, origin_type);
		break;
	case NFCT_T_DESTROY:
		if (STATE(mode)->internal->ct.del(ct, origin_type))
			update_traffic_stats(ct);
		break;
	default:
		STATE(stats).nl_events_unknown_type++;
		break;
	}

out:
	/* we reset the iteration limiter in the main select loop. */
	if (STATE(event_iterations_limit)-- <= 0)
		return NFCT_CB_STOP;
	else
		return NFCT_CB_CONTINUE;
}

static int exp_event_handler(const struct nlmsghdr *nlh,
			     enum nf_conntrack_msg_type type,
			     struct nf_expect *exp,
			     void *data)
{
	int origin_type;
	const struct nf_conntrack *master =
		nfexp_get_attr(exp, ATTR_EXP_MASTER);

	STATE(stats).nl_events_received++;

	if (!exp_filter_find(STATE(exp_filter), exp)) {
		STATE(stats).nl_events_filtered++;
		goto out;
	}
	if (ct_filter_master(master))
		return NFCT_CB_CONTINUE;

	origin_type = origin_find(nlh);

	switch(type) {
	case NFCT_T_NEW:
		STATE(mode)->internal->exp.new(exp, origin_type);
		break;
	case NFCT_T_UPDATE:
		STATE(mode)->internal->exp.upd(exp, origin_type);
		break;
	case NFCT_T_DESTROY:
		STATE(mode)->internal->exp.del(exp, origin_type);
		break;
	default:
		STATE(stats).nl_events_unknown_type++;
		break;
	}

out:
	/* we reset the iteration limiter in the main select loop. */
	if (STATE(event_iterations_limit)-- <= 0)
		return NFCT_CB_STOP;
	else
		return NFCT_CB_CONTINUE;
}

static int dump_handler(enum nf_conntrack_msg_type type,
			struct nf_conntrack *ct,
			void *data)
{
	if (ct_filter_conntrack(ct, 1))
		return NFCT_CB_CONTINUE;

	switch(type) {
	case NFCT_T_UPDATE:
		STATE(mode)->internal->ct.populate(ct);
		break;
	default:
		STATE(stats).nl_dump_unknown_type++;
		break;
	}
	return NFCT_CB_CONTINUE;
}

static int exp_dump_handler(enum nf_conntrack_msg_type type,
			    struct nf_expect *exp, void *data)
{
	const struct nf_conntrack *master =
		nfexp_get_attr(exp, ATTR_EXP_MASTER);

	if (!exp_filter_find(STATE(exp_filter), exp))
		return NFCT_CB_CONTINUE;

	if (ct_filter_master(master))
		return NFCT_CB_CONTINUE;

	switch(type) {
	case NFCT_T_UPDATE:
		STATE(mode)->internal->exp.populate(exp);
		break;
	default:
		STATE(stats).nl_dump_unknown_type++;
		break;
	}
	return NFCT_CB_CONTINUE;
}

static int get_handler(enum nf_conntrack_msg_type type,
		       struct nf_conntrack *ct,
		       void *data)
{
	if (ct_filter_conntrack(ct, 1))
		return NFCT_CB_CONTINUE;

	STATE(get_retval) = 1;
	return NFCT_CB_CONTINUE;
}

static int exp_get_handler(enum nf_conntrack_msg_type type,
			   struct nf_expect *exp, void *data)
{
	const struct nf_conntrack *master =
		nfexp_get_attr(exp, ATTR_EXP_MASTER);

	if (!exp_filter_find(STATE(exp_filter), exp))
		return NFCT_CB_CONTINUE;

	if (ct_filter_master(master))
		return NFCT_CB_CONTINUE;

	STATE(get_retval) = 1;
	return NFCT_CB_CONTINUE;
}

/* we have received an event from ctnetlink */
static void event_cb(void *data)
{
	int ret;

	ret = nfct_catch(STATE(event));
	/* reset event iteration limit counter */
	STATE(event_iterations_limit) = CONFIG(event_iterations_limit);
	if (ret == -1) {
		switch(errno) {
		case ENOBUFS:
			/* We have hit ENOBUFS, it's likely that we are
			 * losing events. Two possible situations may
			 * trigger this error:
			 *
			 * 1) The netlink receiver buffer is too small:
			 *    increasing the netlink buffer size should
			 *    be enough. However, some event messages
			 *    got lost. We have to resync ourselves
			 *    with the kernel table conntrack table to
			 *    resolve the inconsistency.
			 *
			 * 2) The receiver is too slow to process the
			 *    netlink messages so that the queue gets
			 *    full quickly. This generally happens
			 *    if the system is under heavy workload
			 *    (busy CPU). In this case, increasing the
			 *    size of the netlink receiver buffer
			 *    would not help anymore since we would
			 *    be delaying the overrun. Moreover, we
			 *    should avoid resynchronizations. We
			 *    should do our best here and keep
			 *    replicating as much states as possible.
			 *    If workload lowers at some point,
			 *    we resync ourselves.
			 */
			nl_resize_socket_buffer(STATE(event));
			if (CONFIG(nl_overrun_resync) > 0 &&
			    STATE(mode)->internal->flags & INTERNAL_F_RESYNC) {
				add_alarm(&STATE(resync_alarm),
					  CONFIG(nl_overrun_resync),0);
			}
			STATE(stats).nl_catch_event_failed++;
			STATE(stats).nl_overrun++;
			break;
		case ENOENT:
			/*
			 * We received a message from another
			 * netfilter subsystem that we are not
			 * interested in. Just ignore it.
			 */
		break;
		case EAGAIN:
			/* No more events to receive, try later. */
			break;
		default:
			STATE(stats).nl_catch_event_failed++;
			break;
		}
	}
}

/* we previously requested a resync due to buffer overrun. */
static void resync_cb(void *data)
{
	nfct_catch(STATE(resync));
	if (STATE(mode)->internal->ct.purge)
		STATE(mode)->internal->ct.purge();
}

static void poll_cb(void *data)
{
	nfct_catch(STATE(resync));
}

int ctnl_init(void)
{
	if (CONFIG(flags) & CTD_STATS_MODE)
		STATE(mode) = &stats_mode;
	else if (CONFIG(flags) & CTD_SYNC_MODE)
		STATE(mode) = &sync_mode;
	else {
		dlog(LOG_WARNING, "No running mode specified. "
		     "Defaulting to statistics mode.");
		CONFIG(flags) |= CTD_STATS_MODE;
		STATE(mode) = &stats_mode;
	}

	/* Initialization */
	if (STATE(mode)->init() == -1) {
		dlog(LOG_ERR, "initialization failed");
		return -1;
	}

	/* resynchronize (like 'dump' socket) but it also purges old entries */
	STATE(resync) = nfct_open(CONFIG(netlink).subsys_id, 0);
	if (STATE(resync)== NULL) {
		dlog(LOG_ERR, "can't open netlink handler: %s",
		     strerror(errno));
		dlog(LOG_ERR, "no ctnetlink kernel support?");
		return -1;
	}
	nfct_callback_register(STATE(resync),
			       NFCT_T_ALL,
			       STATE(mode)->internal->ct.resync,
			       NULL);
	if (CONFIG(flags) & CTD_POLL) {
		register_fd(nfct_fd(STATE(resync)), poll_cb,
				NULL, STATE(fds));
	} else {
		register_fd(nfct_fd(STATE(resync)), resync_cb,
				NULL, STATE(fds));
	}
	fcntl(nfct_fd(STATE(resync)), F_SETFL, O_NONBLOCK);

	if (STATE(mode)->internal->flags & INTERNAL_F_POPULATE) {
		STATE(dump) = nfct_open(CONFIG(netlink).subsys_id, 0);
		if (STATE(dump) == NULL) {
			dlog(LOG_ERR, "can't open netlink handler: %s",
			     strerror(errno));
			dlog(LOG_ERR, "no ctnetlink kernel support?");
			return -1;
		}
		nfct_callback_register(STATE(dump), NFCT_T_ALL,
				       dump_handler, NULL);

		if (CONFIG(flags) & CTD_EXPECT) {
			nfexp_callback_register(STATE(dump), NFCT_T_ALL,
						exp_dump_handler, NULL);
		}

		if (nl_dump_conntrack_table(STATE(dump)) == -1) {
			dlog(LOG_ERR, "can't get kernel conntrack table");
			return -1;
		}

		if (CONFIG(flags) & CTD_EXPECT) {
			if (nl_dump_expect_table(STATE(dump)) == -1) {
				dlog(LOG_ERR, "can't get kernel "
					      "expect table");
				return -1;
			}
		}
	}

	STATE(get) = nfct_open(CONFIG(netlink).subsys_id, 0);
	if (STATE(get) == NULL) {
		dlog(LOG_ERR, "can't open netlink handler: %s",
		     strerror(errno));
		dlog(LOG_ERR, "no ctnetlink kernel support?");
		return -1;
	}
	nfct_callback_register(STATE(get), NFCT_T_ALL, get_handler, NULL);

	if (CONFIG(flags) & CTD_EXPECT) {
		nfexp_callback_register(STATE(get), NFCT_T_ALL,
					exp_get_handler, NULL);
	}

	STATE(flush) = nfct_open(CONFIG(netlink).subsys_id, 0);
	if (STATE(flush) == NULL) {
		dlog(LOG_ERR, "cannot open flusher handler");
		return -1;
	}
	/* register this handler as the origin of a flush operation */
	origin_register(STATE(flush), CTD_ORIGIN_FLUSH);

	if (CONFIG(flags) & CTD_POLL) {
		init_alarm(&STATE(polling_alarm), NULL, do_polling_alarm);
		add_alarm(&STATE(polling_alarm), CONFIG(poll_kernel_secs), 0);
		dlog(LOG_NOTICE, "running in polling mode");
	} else {
		init_alarm(&STATE(resync_alarm), NULL, do_overrun_resync_alarm);
		/*
		 * The last nfct handler that we register is the event handler.
		 * The reason to do this is that we may receive events while
		 * populating the internal cache. Thus, we hit ENOBUFS
		 * prematurely. However, if we open the event handler before
		 * populating the internal cache, we may still lose events
		 * that have occured during the population.
		 */
		STATE(event) = nl_init_event_handler();
		if (STATE(event) == NULL) {
			dlog(LOG_ERR, "can't open netlink handler: %s",
			     strerror(errno));
			dlog(LOG_ERR, "no ctnetlink kernel support?");
			return -1;
		}
		nfct_callback_register2(STATE(event), NFCT_T_ALL,
				        event_handler, NULL);

		if (CONFIG(flags) & CTD_EXPECT) {
			nfexp_callback_register2(STATE(event), NFCT_T_ALL,
						 exp_event_handler, NULL);
		}
		register_fd(nfct_fd(STATE(event)), event_cb, NULL, STATE(fds));
	}

	return 0;
}
