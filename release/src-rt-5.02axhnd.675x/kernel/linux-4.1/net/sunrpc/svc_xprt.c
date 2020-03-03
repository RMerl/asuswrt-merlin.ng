/*
 * linux/net/sunrpc/svc_xprt.c
 *
 * Author: Tom Tucker <tom@opengridcomputing.com>
 */

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <net/sock.h>
#include <linux/sunrpc/stats.h>
#include <linux/sunrpc/svc_xprt.h>
#include <linux/sunrpc/svcsock.h>
#include <linux/sunrpc/xprt.h>
#include <linux/module.h>
#include <trace/events/sunrpc.h>

#define RPCDBG_FACILITY	RPCDBG_SVCXPRT

static struct svc_deferred_req *svc_deferred_dequeue(struct svc_xprt *xprt);
static int svc_deferred_recv(struct svc_rqst *rqstp);
static struct cache_deferred_req *svc_defer(struct cache_req *req);
static void svc_age_temp_xprts(unsigned long closure);
static void svc_delete_xprt(struct svc_xprt *xprt);
static void svc_xprt_do_enqueue(struct svc_xprt *xprt);

/* apparently the "standard" is that clients close
 * idle connections after 5 minutes, servers after
 * 6 minutes
 *   http://www.connectathon.org/talks96/nfstcp.pdf
 */
static int svc_conn_age_period = 6*60;

/* List of registered transport classes */
static DEFINE_SPINLOCK(svc_xprt_class_lock);
static LIST_HEAD(svc_xprt_class_list);

/* SMP locking strategy:
 *
 *	svc_pool->sp_lock protects most of the fields of that pool.
 *	svc_serv->sv_lock protects sv_tempsocks, sv_permsocks, sv_tmpcnt.
 *	when both need to be taken (rare), svc_serv->sv_lock is first.
 *	The "service mutex" protects svc_serv->sv_nrthread.
 *	svc_sock->sk_lock protects the svc_sock->sk_deferred list
 *             and the ->sk_info_authunix cache.
 *
 *	The XPT_BUSY bit in xprt->xpt_flags prevents a transport being
 *	enqueued multiply. During normal transport processing this bit
 *	is set by svc_xprt_enqueue and cleared by svc_xprt_received.
 *	Providers should not manipulate this bit directly.
 *
 *	Some flags can be set to certain values at any time
 *	providing that certain rules are followed:
 *
 *	XPT_CONN, XPT_DATA:
 *		- Can be set or cleared at any time.
 *		- After a set, svc_xprt_enqueue must be called to enqueue
 *		  the transport for processing.
 *		- After a clear, the transport must be read/accepted.
 *		  If this succeeds, it must be set again.
 *	XPT_CLOSE:
 *		- Can set at any time. It is never cleared.
 *      XPT_DEAD:
 *		- Can only be set while XPT_BUSY is held which ensures
 *		  that no other thread will be using the transport or will
 *		  try to set XPT_DEAD.
 */
int svc_reg_xprt_class(struct svc_xprt_class *xcl)
{
	struct svc_xprt_class *cl;
	int res = -EEXIST;

	dprintk("svc: Adding svc transport class '%s'\n", xcl->xcl_name);

	INIT_LIST_HEAD(&xcl->xcl_list);
	spin_lock(&svc_xprt_class_lock);
	/* Make sure there isn't already a class with the same name */
	list_for_each_entry(cl, &svc_xprt_class_list, xcl_list) {
		if (strcmp(xcl->xcl_name, cl->xcl_name) == 0)
			goto out;
	}
	list_add_tail(&xcl->xcl_list, &svc_xprt_class_list);
	res = 0;
out:
	spin_unlock(&svc_xprt_class_lock);
	return res;
}
EXPORT_SYMBOL_GPL(svc_reg_xprt_class);

void svc_unreg_xprt_class(struct svc_xprt_class *xcl)
{
	dprintk("svc: Removing svc transport class '%s'\n", xcl->xcl_name);
	spin_lock(&svc_xprt_class_lock);
	list_del_init(&xcl->xcl_list);
	spin_unlock(&svc_xprt_class_lock);
}
EXPORT_SYMBOL_GPL(svc_unreg_xprt_class);

/*
 * Format the transport list for printing
 */
int svc_print_xprts(char *buf, int maxlen)
{
	struct svc_xprt_class *xcl;
	char tmpstr[80];
	int len = 0;
	buf[0] = '\0';

	spin_lock(&svc_xprt_class_lock);
	list_for_each_entry(xcl, &svc_xprt_class_list, xcl_list) {
		int slen;

		sprintf(tmpstr, "%s %d\n", xcl->xcl_name, xcl->xcl_max_payload);
		slen = strlen(tmpstr);
		if (len + slen > maxlen)
			break;
		len += slen;
		strcat(buf, tmpstr);
	}
	spin_unlock(&svc_xprt_class_lock);

	return len;
}

static void svc_xprt_free(struct kref *kref)
{
	struct svc_xprt *xprt =
		container_of(kref, struct svc_xprt, xpt_ref);
	struct module *owner = xprt->xpt_class->xcl_owner;
	if (test_bit(XPT_CACHE_AUTH, &xprt->xpt_flags))
		svcauth_unix_info_release(xprt);
	put_net(xprt->xpt_net);
	/* See comment on corresponding get in xs_setup_bc_tcp(): */
	if (xprt->xpt_bc_xprt)
		xprt_put(xprt->xpt_bc_xprt);
	xprt->xpt_ops->xpo_free(xprt);
	module_put(owner);
}

void svc_xprt_put(struct svc_xprt *xprt)
{
	kref_put(&xprt->xpt_ref, svc_xprt_free);
}
EXPORT_SYMBOL_GPL(svc_xprt_put);

/*
 * Called by transport drivers to initialize the transport independent
 * portion of the transport instance.
 */
void svc_xprt_init(struct net *net, struct svc_xprt_class *xcl,
		   struct svc_xprt *xprt, struct svc_serv *serv)
{
	memset(xprt, 0, sizeof(*xprt));
	xprt->xpt_class = xcl;
	xprt->xpt_ops = xcl->xcl_ops;
	kref_init(&xprt->xpt_ref);
	xprt->xpt_server = serv;
	INIT_LIST_HEAD(&xprt->xpt_list);
	INIT_LIST_HEAD(&xprt->xpt_ready);
	INIT_LIST_HEAD(&xprt->xpt_deferred);
	INIT_LIST_HEAD(&xprt->xpt_users);
	mutex_init(&xprt->xpt_mutex);
	spin_lock_init(&xprt->xpt_lock);
	set_bit(XPT_BUSY, &xprt->xpt_flags);
	rpc_init_wait_queue(&xprt->xpt_bc_pending, "xpt_bc_pending");
	xprt->xpt_net = get_net(net);
}
EXPORT_SYMBOL_GPL(svc_xprt_init);

static struct svc_xprt *__svc_xpo_create(struct svc_xprt_class *xcl,
					 struct svc_serv *serv,
					 struct net *net,
					 const int family,
					 const unsigned short port,
					 int flags)
{
	struct sockaddr_in sin = {
		.sin_family		= AF_INET,
		.sin_addr.s_addr	= htonl(INADDR_ANY),
		.sin_port		= htons(port),
	};
#if IS_ENABLED(CONFIG_IPV6)
	struct sockaddr_in6 sin6 = {
		.sin6_family		= AF_INET6,
		.sin6_addr		= IN6ADDR_ANY_INIT,
		.sin6_port		= htons(port),
	};
#endif
	struct sockaddr *sap;
	size_t len;

	switch (family) {
	case PF_INET:
		sap = (struct sockaddr *)&sin;
		len = sizeof(sin);
		break;
#if IS_ENABLED(CONFIG_IPV6)
	case PF_INET6:
		sap = (struct sockaddr *)&sin6;
		len = sizeof(sin6);
		break;
#endif
	default:
		return ERR_PTR(-EAFNOSUPPORT);
	}

	return xcl->xcl_ops->xpo_create(serv, net, sap, len, flags);
}

/*
 * svc_xprt_received conditionally queues the transport for processing
 * by another thread. The caller must hold the XPT_BUSY bit and must
 * not thereafter touch transport data.
 *
 * Note: XPT_DATA only gets cleared when a read-attempt finds no (or
 * insufficient) data.
 */
static void svc_xprt_received(struct svc_xprt *xprt)
{
	if (!test_bit(XPT_BUSY, &xprt->xpt_flags)) {
		WARN_ONCE(1, "xprt=0x%p already busy!", xprt);
		return;
	}

	/* As soon as we clear busy, the xprt could be closed and
	 * 'put', so we need a reference to call svc_xprt_do_enqueue with:
	 */
	svc_xprt_get(xprt);
	smp_mb__before_atomic();
	clear_bit(XPT_BUSY, &xprt->xpt_flags);
	svc_xprt_do_enqueue(xprt);
	svc_xprt_put(xprt);
}

void svc_add_new_perm_xprt(struct svc_serv *serv, struct svc_xprt *new)
{
	clear_bit(XPT_TEMP, &new->xpt_flags);
	spin_lock_bh(&serv->sv_lock);
	list_add(&new->xpt_list, &serv->sv_permsocks);
	spin_unlock_bh(&serv->sv_lock);
	svc_xprt_received(new);
}

int svc_create_xprt(struct svc_serv *serv, const char *xprt_name,
		    struct net *net, const int family,
		    const unsigned short port, int flags)
{
	struct svc_xprt_class *xcl;

	dprintk("svc: creating transport %s[%d]\n", xprt_name, port);
	spin_lock(&svc_xprt_class_lock);
	list_for_each_entry(xcl, &svc_xprt_class_list, xcl_list) {
		struct svc_xprt *newxprt;
		unsigned short newport;

		if (strcmp(xprt_name, xcl->xcl_name))
			continue;

		if (!try_module_get(xcl->xcl_owner))
			goto err;

		spin_unlock(&svc_xprt_class_lock);
		newxprt = __svc_xpo_create(xcl, serv, net, family, port, flags);
		if (IS_ERR(newxprt)) {
			module_put(xcl->xcl_owner);
			return PTR_ERR(newxprt);
		}
		svc_add_new_perm_xprt(serv, newxprt);
		newport = svc_xprt_local_port(newxprt);
		return newport;
	}
 err:
	spin_unlock(&svc_xprt_class_lock);
	dprintk("svc: transport %s not found\n", xprt_name);

	/* This errno is exposed to user space.  Provide a reasonable
	 * perror msg for a bad transport. */
	return -EPROTONOSUPPORT;
}
EXPORT_SYMBOL_GPL(svc_create_xprt);

/*
 * Copy the local and remote xprt addresses to the rqstp structure
 */
void svc_xprt_copy_addrs(struct svc_rqst *rqstp, struct svc_xprt *xprt)
{
	memcpy(&rqstp->rq_addr, &xprt->xpt_remote, xprt->xpt_remotelen);
	rqstp->rq_addrlen = xprt->xpt_remotelen;

	/*
	 * Destination address in request is needed for binding the
	 * source address in RPC replies/callbacks later.
	 */
	memcpy(&rqstp->rq_daddr, &xprt->xpt_local, xprt->xpt_locallen);
	rqstp->rq_daddrlen = xprt->xpt_locallen;
}
EXPORT_SYMBOL_GPL(svc_xprt_copy_addrs);

/**
 * svc_print_addr - Format rq_addr field for printing
 * @rqstp: svc_rqst struct containing address to print
 * @buf: target buffer for formatted address
 * @len: length of target buffer
 *
 */
char *svc_print_addr(struct svc_rqst *rqstp, char *buf, size_t len)
{
	return __svc_print_addr(svc_addr(rqstp), buf, len);
}
EXPORT_SYMBOL_GPL(svc_print_addr);

static bool svc_xprt_has_something_to_do(struct svc_xprt *xprt)
{
	if (xprt->xpt_flags & ((1<<XPT_CONN)|(1<<XPT_CLOSE)))
		return true;
	if (xprt->xpt_flags & ((1<<XPT_DATA)|(1<<XPT_DEFERRED)))
		return xprt->xpt_ops->xpo_has_wspace(xprt);
	return false;
}

static void svc_xprt_do_enqueue(struct svc_xprt *xprt)
{
	struct svc_pool *pool;
	struct svc_rqst	*rqstp = NULL;
	int cpu;
	bool queued = false;

	if (!svc_xprt_has_something_to_do(xprt))
		goto out;

	/* Mark transport as busy. It will remain in this state until
	 * the provider calls svc_xprt_received. We update XPT_BUSY
	 * atomically because it also guards against trying to enqueue
	 * the transport twice.
	 */
	if (test_and_set_bit(XPT_BUSY, &xprt->xpt_flags)) {
		/* Don't enqueue transport while already enqueued */
		dprintk("svc: transport %p busy, not enqueued\n", xprt);
		goto out;
	}

	cpu = get_cpu();
	pool = svc_pool_for_cpu(xprt->xpt_server, cpu);

	atomic_long_inc(&pool->sp_stats.packets);

redo_search:
	/* find a thread for this xprt */
	rcu_read_lock();
	list_for_each_entry_rcu(rqstp, &pool->sp_all_threads, rq_all) {
		/* Do a lockless check first */
		if (test_bit(RQ_BUSY, &rqstp->rq_flags))
			continue;

		/*
		 * Once the xprt has been queued, it can only be dequeued by
		 * the task that intends to service it. All we can do at that
		 * point is to try to wake this thread back up so that it can
		 * do so.
		 */
		if (!queued) {
			spin_lock_bh(&rqstp->rq_lock);
			if (test_and_set_bit(RQ_BUSY, &rqstp->rq_flags)) {
				/* already busy, move on... */
				spin_unlock_bh(&rqstp->rq_lock);
				continue;
			}

			/* this one will do */
			rqstp->rq_xprt = xprt;
			svc_xprt_get(xprt);
			spin_unlock_bh(&rqstp->rq_lock);
		}
		rcu_read_unlock();

		atomic_long_inc(&pool->sp_stats.threads_woken);
		wake_up_process(rqstp->rq_task);
		put_cpu();
		goto out;
	}
	rcu_read_unlock();

	/*
	 * We didn't find an idle thread to use, so we need to queue the xprt.
	 * Do so and then search again. If we find one, we can't hook this one
	 * up to it directly but we can wake the thread up in the hopes that it
	 * will pick it up once it searches for a xprt to service.
	 */
	if (!queued) {
		queued = true;
		dprintk("svc: transport %p put into queue\n", xprt);
		spin_lock_bh(&pool->sp_lock);
		list_add_tail(&xprt->xpt_ready, &pool->sp_sockets);
		pool->sp_stats.sockets_queued++;
		spin_unlock_bh(&pool->sp_lock);
		goto redo_search;
	}
	rqstp = NULL;
	put_cpu();
out:
	trace_svc_xprt_do_enqueue(xprt, rqstp);
}

/*
 * Queue up a transport with data pending. If there are idle nfsd
 * processes, wake 'em up.
 *
 */
void svc_xprt_enqueue(struct svc_xprt *xprt)
{
	if (test_bit(XPT_BUSY, &xprt->xpt_flags))
		return;
	svc_xprt_do_enqueue(xprt);
}
EXPORT_SYMBOL_GPL(svc_xprt_enqueue);

/*
 * Dequeue the first transport, if there is one.
 */
static struct svc_xprt *svc_xprt_dequeue(struct svc_pool *pool)
{
	struct svc_xprt	*xprt = NULL;

	if (list_empty(&pool->sp_sockets))
		goto out;

	spin_lock_bh(&pool->sp_lock);
	if (likely(!list_empty(&pool->sp_sockets))) {
		xprt = list_first_entry(&pool->sp_sockets,
					struct svc_xprt, xpt_ready);
		list_del_init(&xprt->xpt_ready);
		svc_xprt_get(xprt);

		dprintk("svc: transport %p dequeued, inuse=%d\n",
			xprt, atomic_read(&xprt->xpt_ref.refcount));
	}
	spin_unlock_bh(&pool->sp_lock);
out:
	trace_svc_xprt_dequeue(xprt);
	return xprt;
}

/**
 * svc_reserve - change the space reserved for the reply to a request.
 * @rqstp:  The request in question
 * @space: new max space to reserve
 *
 * Each request reserves some space on the output queue of the transport
 * to make sure the reply fits.  This function reduces that reserved
 * space to be the amount of space used already, plus @space.
 *
 */
void svc_reserve(struct svc_rqst *rqstp, int space)
{
	space += rqstp->rq_res.head[0].iov_len;

	if (space < rqstp->rq_reserved) {
		struct svc_xprt *xprt = rqstp->rq_xprt;
		atomic_sub((rqstp->rq_reserved - space), &xprt->xpt_reserved);
		rqstp->rq_reserved = space;

		if (xprt->xpt_ops->xpo_adjust_wspace)
			xprt->xpt_ops->xpo_adjust_wspace(xprt);
		svc_xprt_enqueue(xprt);
	}
}
EXPORT_SYMBOL_GPL(svc_reserve);

static void svc_xprt_release(struct svc_rqst *rqstp)
{
	struct svc_xprt	*xprt = rqstp->rq_xprt;

	rqstp->rq_xprt->xpt_ops->xpo_release_rqst(rqstp);

	kfree(rqstp->rq_deferred);
	rqstp->rq_deferred = NULL;

	svc_free_res_pages(rqstp);
	rqstp->rq_res.page_len = 0;
	rqstp->rq_res.page_base = 0;

	/* Reset response buffer and release
	 * the reservation.
	 * But first, check that enough space was reserved
	 * for the reply, otherwise we have a bug!
	 */
	if ((rqstp->rq_res.len) >  rqstp->rq_reserved)
		printk(KERN_ERR "RPC request reserved %d but used %d\n",
		       rqstp->rq_reserved,
		       rqstp->rq_res.len);

	rqstp->rq_res.head[0].iov_len = 0;
	svc_reserve(rqstp, 0);
	rqstp->rq_xprt = NULL;

	svc_xprt_put(xprt);
}

/*
 * Some svc_serv's will have occasional work to do, even when a xprt is not
 * waiting to be serviced. This function is there to "kick" a task in one of
 * those services so that it can wake up and do that work. Note that we only
 * bother with pool 0 as we don't need to wake up more than one thread for
 * this purpose.
 */
void svc_wake_up(struct svc_serv *serv)
{
	struct svc_rqst	*rqstp;
	struct svc_pool *pool;

	pool = &serv->sv_pools[0];

	rcu_read_lock();
	list_for_each_entry_rcu(rqstp, &pool->sp_all_threads, rq_all) {
		/* skip any that aren't queued */
		if (test_bit(RQ_BUSY, &rqstp->rq_flags))
			continue;
		rcu_read_unlock();
		dprintk("svc: daemon %p woken up.\n", rqstp);
		wake_up_process(rqstp->rq_task);
		trace_svc_wake_up(rqstp->rq_task->pid);
		return;
	}
	rcu_read_unlock();

	/* No free entries available */
	set_bit(SP_TASK_PENDING, &pool->sp_flags);
	smp_wmb();
	trace_svc_wake_up(0);
}
EXPORT_SYMBOL_GPL(svc_wake_up);

int svc_port_is_privileged(struct sockaddr *sin)
{
	switch (sin->sa_family) {
	case AF_INET:
		return ntohs(((struct sockaddr_in *)sin)->sin_port)
			< PROT_SOCK;
	case AF_INET6:
		return ntohs(((struct sockaddr_in6 *)sin)->sin6_port)
			< PROT_SOCK;
	default:
		return 0;
	}
}

/*
 * Make sure that we don't have too many active connections. If we have,
 * something must be dropped. It's not clear what will happen if we allow
 * "too many" connections, but when dealing with network-facing software,
 * we have to code defensively. Here we do that by imposing hard limits.
 *
 * There's no point in trying to do random drop here for DoS
 * prevention. The NFS clients does 1 reconnect in 15 seconds. An
 * attacker can easily beat that.
 *
 * The only somewhat efficient mechanism would be if drop old
 * connections from the same IP first. But right now we don't even
 * record the client IP in svc_sock.
 *
 * single-threaded services that expect a lot of clients will probably
 * need to set sv_maxconn to override the default value which is based
 * on the number of threads
 */
static void svc_check_conn_limits(struct svc_serv *serv)
{
	unsigned int limit = serv->sv_maxconn ? serv->sv_maxconn :
				(serv->sv_nrthreads+3) * 20;

	if (serv->sv_tmpcnt > limit) {
		struct svc_xprt *xprt = NULL;
		spin_lock_bh(&serv->sv_lock);
		if (!list_empty(&serv->sv_tempsocks)) {
			/* Try to help the admin */
			net_notice_ratelimited("%s: too many open connections, consider increasing the %s\n",
					       serv->sv_name, serv->sv_maxconn ?
					       "max number of connections" :
					       "number of threads");
			/*
			 * Always select the oldest connection. It's not fair,
			 * but so is life
			 */
			xprt = list_entry(serv->sv_tempsocks.prev,
					  struct svc_xprt,
					  xpt_list);
			set_bit(XPT_CLOSE, &xprt->xpt_flags);
			svc_xprt_get(xprt);
		}
		spin_unlock_bh(&serv->sv_lock);

		if (xprt) {
			svc_xprt_enqueue(xprt);
			svc_xprt_put(xprt);
		}
	}
}

static int svc_alloc_arg(struct svc_rqst *rqstp)
{
	struct svc_serv *serv = rqstp->rq_server;
	struct xdr_buf *arg;
	int pages;
	int i;

	/* now allocate needed pages.  If we get a failure, sleep briefly */
	pages = (serv->sv_max_mesg + PAGE_SIZE) / PAGE_SIZE;
	WARN_ON_ONCE(pages >= RPCSVC_MAXPAGES);
	if (pages >= RPCSVC_MAXPAGES)
		/* use as many pages as possible */
		pages = RPCSVC_MAXPAGES - 1;
	for (i = 0; i < pages ; i++)
		while (rqstp->rq_pages[i] == NULL) {
			struct page *p = alloc_page(GFP_KERNEL);
			if (!p) {
				set_current_state(TASK_INTERRUPTIBLE);
				if (signalled() || kthread_should_stop()) {
					set_current_state(TASK_RUNNING);
					return -EINTR;
				}
				schedule_timeout(msecs_to_jiffies(500));
			}
			rqstp->rq_pages[i] = p;
		}
	rqstp->rq_page_end = &rqstp->rq_pages[i];
	rqstp->rq_pages[i++] = NULL; /* this might be seen in nfs_read_actor */

	/* Make arg->head point to first page and arg->pages point to rest */
	arg = &rqstp->rq_arg;
	arg->head[0].iov_base = page_address(rqstp->rq_pages[0]);
	arg->head[0].iov_len = PAGE_SIZE;
	arg->pages = rqstp->rq_pages + 1;
	arg->page_base = 0;
	/* save at least one page for response */
	arg->page_len = (pages-2)*PAGE_SIZE;
	arg->len = (pages-1)*PAGE_SIZE;
	arg->tail[0].iov_len = 0;
	return 0;
}

static bool
rqst_should_sleep(struct svc_rqst *rqstp)
{
	struct svc_pool		*pool = rqstp->rq_pool;

	/* did someone call svc_wake_up? */
	if (test_and_clear_bit(SP_TASK_PENDING, &pool->sp_flags))
		return false;

	/* was a socket queued? */
	if (!list_empty(&pool->sp_sockets))
		return false;

	/* are we shutting down? */
	if (signalled() || kthread_should_stop())
		return false;

	/* are we freezing? */
	if (freezing(current))
		return false;

	return true;
}

static struct svc_xprt *svc_get_next_xprt(struct svc_rqst *rqstp, long timeout)
{
	struct svc_xprt *xprt;
	struct svc_pool		*pool = rqstp->rq_pool;
	long			time_left = 0;

	/* rq_xprt should be clear on entry */
	WARN_ON_ONCE(rqstp->rq_xprt);

	/* Normally we will wait up to 5 seconds for any required
	 * cache information to be provided.
	 */
	rqstp->rq_chandle.thread_wait = 5*HZ;

	xprt = svc_xprt_dequeue(pool);
	if (xprt) {
		rqstp->rq_xprt = xprt;

		/* As there is a shortage of threads and this request
		 * had to be queued, don't allow the thread to wait so
		 * long for cache updates.
		 */
		rqstp->rq_chandle.thread_wait = 1*HZ;
		clear_bit(SP_TASK_PENDING, &pool->sp_flags);
		return xprt;
	}

	/*
	 * We have to be able to interrupt this wait
	 * to bring down the daemons ...
	 */
	set_current_state(TASK_INTERRUPTIBLE);
	clear_bit(RQ_BUSY, &rqstp->rq_flags);
	smp_mb();

	if (likely(rqst_should_sleep(rqstp)))
		time_left = schedule_timeout(timeout);
	else
		__set_current_state(TASK_RUNNING);

	try_to_freeze();

	spin_lock_bh(&rqstp->rq_lock);
	set_bit(RQ_BUSY, &rqstp->rq_flags);
	spin_unlock_bh(&rqstp->rq_lock);

	xprt = rqstp->rq_xprt;
	if (xprt != NULL)
		return xprt;

	if (!time_left)
		atomic_long_inc(&pool->sp_stats.threads_timedout);

	if (signalled() || kthread_should_stop())
		return ERR_PTR(-EINTR);
	return ERR_PTR(-EAGAIN);
}

static void svc_add_new_temp_xprt(struct svc_serv *serv, struct svc_xprt *newxpt)
{
	spin_lock_bh(&serv->sv_lock);
	set_bit(XPT_TEMP, &newxpt->xpt_flags);
	list_add(&newxpt->xpt_list, &serv->sv_tempsocks);
	serv->sv_tmpcnt++;
	if (serv->sv_temptimer.function == NULL) {
		/* setup timer to age temp transports */
		setup_timer(&serv->sv_temptimer, svc_age_temp_xprts,
			    (unsigned long)serv);
		mod_timer(&serv->sv_temptimer,
			  jiffies + svc_conn_age_period * HZ);
	}
	spin_unlock_bh(&serv->sv_lock);
	svc_xprt_received(newxpt);
}

static int svc_handle_xprt(struct svc_rqst *rqstp, struct svc_xprt *xprt)
{
	struct svc_serv *serv = rqstp->rq_server;
	int len = 0;

	if (test_bit(XPT_CLOSE, &xprt->xpt_flags)) {
		dprintk("svc_recv: found XPT_CLOSE\n");
		svc_delete_xprt(xprt);
		/* Leave XPT_BUSY set on the dead xprt: */
		goto out;
	}
	if (test_bit(XPT_LISTENER, &xprt->xpt_flags)) {
		struct svc_xprt *newxpt;
		/*
		 * We know this module_get will succeed because the
		 * listener holds a reference too
		 */
		__module_get(xprt->xpt_class->xcl_owner);
		svc_check_conn_limits(xprt->xpt_server);
		newxpt = xprt->xpt_ops->xpo_accept(xprt);
		if (newxpt)
			svc_add_new_temp_xprt(serv, newxpt);
		else
			module_put(xprt->xpt_class->xcl_owner);
	} else {
		/* XPT_DATA|XPT_DEFERRED case: */
		dprintk("svc: server %p, pool %u, transport %p, inuse=%d\n",
			rqstp, rqstp->rq_pool->sp_id, xprt,
			atomic_read(&xprt->xpt_ref.refcount));
		rqstp->rq_deferred = svc_deferred_dequeue(xprt);
		if (rqstp->rq_deferred)
			len = svc_deferred_recv(rqstp);
		else
			len = xprt->xpt_ops->xpo_recvfrom(rqstp);
		dprintk("svc: got len=%d\n", len);
		rqstp->rq_reserved = serv->sv_max_mesg;
		atomic_add(rqstp->rq_reserved, &xprt->xpt_reserved);
	}
	/* clear XPT_BUSY: */
	svc_xprt_received(xprt);
out:
	trace_svc_handle_xprt(xprt, len);
	return len;
}

/*
 * Receive the next request on any transport.  This code is carefully
 * organised not to touch any cachelines in the shared svc_serv
 * structure, only cachelines in the local svc_pool.
 */
int svc_recv(struct svc_rqst *rqstp, long timeout)
{
	struct svc_xprt		*xprt = NULL;
	struct svc_serv		*serv = rqstp->rq_server;
	int			len, err;

	dprintk("svc: server %p waiting for data (to = %ld)\n",
		rqstp, timeout);

	if (rqstp->rq_xprt)
		printk(KERN_ERR
			"svc_recv: service %p, transport not NULL!\n",
			 rqstp);

	err = svc_alloc_arg(rqstp);
	if (err)
		goto out;

	try_to_freeze();
	cond_resched();
	err = -EINTR;
	if (signalled() || kthread_should_stop())
		goto out;

	xprt = svc_get_next_xprt(rqstp, timeout);
	if (IS_ERR(xprt)) {
		err = PTR_ERR(xprt);
		goto out;
	}

	len = svc_handle_xprt(rqstp, xprt);

	/* No data, incomplete (TCP) read, or accept() */
	err = -EAGAIN;
	if (len <= 0)
		goto out_release;

	clear_bit(XPT_OLD, &xprt->xpt_flags);

	if (xprt->xpt_ops->xpo_secure_port(rqstp))
		set_bit(RQ_SECURE, &rqstp->rq_flags);
	else
		clear_bit(RQ_SECURE, &rqstp->rq_flags);
	rqstp->rq_chandle.defer = svc_defer;
	rqstp->rq_xid = svc_getu32(&rqstp->rq_arg.head[0]);

	if (serv->sv_stats)
		serv->sv_stats->netcnt++;
	trace_svc_recv(rqstp, len);
	return len;
out_release:
	rqstp->rq_res.len = 0;
	svc_xprt_release(rqstp);
out:
	trace_svc_recv(rqstp, err);
	return err;
}
EXPORT_SYMBOL_GPL(svc_recv);

/*
 * Drop request
 */
void svc_drop(struct svc_rqst *rqstp)
{
	dprintk("svc: xprt %p dropped request\n", rqstp->rq_xprt);
	svc_xprt_release(rqstp);
}
EXPORT_SYMBOL_GPL(svc_drop);

/*
 * Return reply to client.
 */
int svc_send(struct svc_rqst *rqstp)
{
	struct svc_xprt	*xprt;
	int		len = -EFAULT;
	struct xdr_buf	*xb;

	xprt = rqstp->rq_xprt;
	if (!xprt)
		goto out;

	/* release the receive skb before sending the reply */
	rqstp->rq_xprt->xpt_ops->xpo_release_rqst(rqstp);

	/* calculate over-all length */
	xb = &rqstp->rq_res;
	xb->len = xb->head[0].iov_len +
		xb->page_len +
		xb->tail[0].iov_len;

	/* Grab mutex to serialize outgoing data. */
	mutex_lock(&xprt->xpt_mutex);
	if (test_bit(XPT_DEAD, &xprt->xpt_flags)
			|| test_bit(XPT_CLOSE, &xprt->xpt_flags))
		len = -ENOTCONN;
	else
		len = xprt->xpt_ops->xpo_sendto(rqstp);
	mutex_unlock(&xprt->xpt_mutex);
	rpc_wake_up(&xprt->xpt_bc_pending);
	svc_xprt_release(rqstp);

	if (len == -ECONNREFUSED || len == -ENOTCONN || len == -EAGAIN)
		len = 0;
out:
	trace_svc_send(rqstp, len);
	return len;
}

/*
 * Timer function to close old temporary transports, using
 * a mark-and-sweep algorithm.
 */
static void svc_age_temp_xprts(unsigned long closure)
{
	struct svc_serv *serv = (struct svc_serv *)closure;
	struct svc_xprt *xprt;
	struct list_head *le, *next;

	dprintk("svc_age_temp_xprts\n");

	if (!spin_trylock_bh(&serv->sv_lock)) {
		/* busy, try again 1 sec later */
		dprintk("svc_age_temp_xprts: busy\n");
		mod_timer(&serv->sv_temptimer, jiffies + HZ);
		return;
	}

	list_for_each_safe(le, next, &serv->sv_tempsocks) {
		xprt = list_entry(le, struct svc_xprt, xpt_list);

		/* First time through, just mark it OLD. Second time
		 * through, close it. */
		if (!test_and_set_bit(XPT_OLD, &xprt->xpt_flags))
			continue;
		if (atomic_read(&xprt->xpt_ref.refcount) > 1 ||
		    test_bit(XPT_BUSY, &xprt->xpt_flags))
			continue;
		list_del_init(le);
		set_bit(XPT_CLOSE, &xprt->xpt_flags);
		dprintk("queuing xprt %p for closing\n", xprt);

		/* a thread will dequeue and close it soon */
		svc_xprt_enqueue(xprt);
	}
	spin_unlock_bh(&serv->sv_lock);

	mod_timer(&serv->sv_temptimer, jiffies + svc_conn_age_period * HZ);
}

static void call_xpt_users(struct svc_xprt *xprt)
{
	struct svc_xpt_user *u;

	spin_lock(&xprt->xpt_lock);
	while (!list_empty(&xprt->xpt_users)) {
		u = list_first_entry(&xprt->xpt_users, struct svc_xpt_user, list);
		list_del(&u->list);
		u->callback(u);
	}
	spin_unlock(&xprt->xpt_lock);
}

/*
 * Remove a dead transport
 */
static void svc_delete_xprt(struct svc_xprt *xprt)
{
	struct svc_serv	*serv = xprt->xpt_server;
	struct svc_deferred_req *dr;

	/* Only do this once */
	if (test_and_set_bit(XPT_DEAD, &xprt->xpt_flags))
		BUG();

	dprintk("svc: svc_delete_xprt(%p)\n", xprt);
	xprt->xpt_ops->xpo_detach(xprt);

	spin_lock_bh(&serv->sv_lock);
	list_del_init(&xprt->xpt_list);
	WARN_ON_ONCE(!list_empty(&xprt->xpt_ready));
	if (test_bit(XPT_TEMP, &xprt->xpt_flags))
		serv->sv_tmpcnt--;
	spin_unlock_bh(&serv->sv_lock);

	while ((dr = svc_deferred_dequeue(xprt)) != NULL)
		kfree(dr);

	call_xpt_users(xprt);
	svc_xprt_put(xprt);
}

void svc_close_xprt(struct svc_xprt *xprt)
{
	set_bit(XPT_CLOSE, &xprt->xpt_flags);
	if (test_and_set_bit(XPT_BUSY, &xprt->xpt_flags))
		/* someone else will have to effect the close */
		return;
	/*
	 * We expect svc_close_xprt() to work even when no threads are
	 * running (e.g., while configuring the server before starting
	 * any threads), so if the transport isn't busy, we delete
	 * it ourself:
	 */
	svc_delete_xprt(xprt);
}
EXPORT_SYMBOL_GPL(svc_close_xprt);

static int svc_close_list(struct svc_serv *serv, struct list_head *xprt_list, struct net *net)
{
	struct svc_xprt *xprt;
	int ret = 0;

	spin_lock(&serv->sv_lock);
	list_for_each_entry(xprt, xprt_list, xpt_list) {
		if (xprt->xpt_net != net)
			continue;
		ret++;
		set_bit(XPT_CLOSE, &xprt->xpt_flags);
		svc_xprt_enqueue(xprt);
	}
	spin_unlock(&serv->sv_lock);
	return ret;
}

static struct svc_xprt *svc_dequeue_net(struct svc_serv *serv, struct net *net)
{
	struct svc_pool *pool;
	struct svc_xprt *xprt;
	struct svc_xprt *tmp;
	int i;

	for (i = 0; i < serv->sv_nrpools; i++) {
		pool = &serv->sv_pools[i];

		spin_lock_bh(&pool->sp_lock);
		list_for_each_entry_safe(xprt, tmp, &pool->sp_sockets, xpt_ready) {
			if (xprt->xpt_net != net)
				continue;
			list_del_init(&xprt->xpt_ready);
			spin_unlock_bh(&pool->sp_lock);
			return xprt;
		}
		spin_unlock_bh(&pool->sp_lock);
	}
	return NULL;
}

static void svc_clean_up_xprts(struct svc_serv *serv, struct net *net)
{
	struct svc_xprt *xprt;

	while ((xprt = svc_dequeue_net(serv, net))) {
		set_bit(XPT_CLOSE, &xprt->xpt_flags);
		svc_delete_xprt(xprt);
	}
}

/*
 * Server threads may still be running (especially in the case where the
 * service is still running in other network namespaces).
 *
 * So we shut down sockets the same way we would on a running server, by
 * setting XPT_CLOSE, enqueuing, and letting a thread pick it up to do
 * the close.  In the case there are no such other threads,
 * threads running, svc_clean_up_xprts() does a simple version of a
 * server's main event loop, and in the case where there are other
 * threads, we may need to wait a little while and then check again to
 * see if they're done.
 */
void svc_close_net(struct svc_serv *serv, struct net *net)
{
	int delay = 0;

	while (svc_close_list(serv, &serv->sv_permsocks, net) +
	       svc_close_list(serv, &serv->sv_tempsocks, net)) {

		svc_clean_up_xprts(serv, net);
		msleep(delay++);
	}
}

/*
 * Handle defer and revisit of requests
 */

static void svc_revisit(struct cache_deferred_req *dreq, int too_many)
{
	struct svc_deferred_req *dr =
		container_of(dreq, struct svc_deferred_req, handle);
	struct svc_xprt *xprt = dr->xprt;

	spin_lock(&xprt->xpt_lock);
	set_bit(XPT_DEFERRED, &xprt->xpt_flags);
	if (too_many || test_bit(XPT_DEAD, &xprt->xpt_flags)) {
		spin_unlock(&xprt->xpt_lock);
		dprintk("revisit canceled\n");
		svc_xprt_put(xprt);
		kfree(dr);
		return;
	}
	dprintk("revisit queued\n");
	dr->xprt = NULL;
	list_add(&dr->handle.recent, &xprt->xpt_deferred);
	spin_unlock(&xprt->xpt_lock);
	svc_xprt_enqueue(xprt);
	svc_xprt_put(xprt);
}

/*
 * Save the request off for later processing. The request buffer looks
 * like this:
 *
 * <xprt-header><rpc-header><rpc-pagelist><rpc-tail>
 *
 * This code can only handle requests that consist of an xprt-header
 * and rpc-header.
 */
static struct cache_deferred_req *svc_defer(struct cache_req *req)
{
	struct svc_rqst *rqstp = container_of(req, struct svc_rqst, rq_chandle);
	struct svc_deferred_req *dr;

	if (rqstp->rq_arg.page_len || !test_bit(RQ_USEDEFERRAL, &rqstp->rq_flags))
		return NULL; /* if more than a page, give up FIXME */
	if (rqstp->rq_deferred) {
		dr = rqstp->rq_deferred;
		rqstp->rq_deferred = NULL;
	} else {
		size_t skip;
		size_t size;
		/* FIXME maybe discard if size too large */
		size = sizeof(struct svc_deferred_req) + rqstp->rq_arg.len;
		dr = kmalloc(size, GFP_KERNEL);
		if (dr == NULL)
			return NULL;

		dr->handle.owner = rqstp->rq_server;
		dr->prot = rqstp->rq_prot;
		memcpy(&dr->addr, &rqstp->rq_addr, rqstp->rq_addrlen);
		dr->addrlen = rqstp->rq_addrlen;
		dr->daddr = rqstp->rq_daddr;
		dr->argslen = rqstp->rq_arg.len >> 2;
		dr->xprt_hlen = rqstp->rq_xprt_hlen;

		/* back up head to the start of the buffer and copy */
		skip = rqstp->rq_arg.len - rqstp->rq_arg.head[0].iov_len;
		memcpy(dr->args, rqstp->rq_arg.head[0].iov_base - skip,
		       dr->argslen << 2);
	}
	svc_xprt_get(rqstp->rq_xprt);
	dr->xprt = rqstp->rq_xprt;
	set_bit(RQ_DROPME, &rqstp->rq_flags);

	dr->handle.revisit = svc_revisit;
	return &dr->handle;
}

/*
 * recv data from a deferred request into an active one
 */
static int svc_deferred_recv(struct svc_rqst *rqstp)
{
	struct svc_deferred_req *dr = rqstp->rq_deferred;

	/* setup iov_base past transport header */
	rqstp->rq_arg.head[0].iov_base = dr->args + (dr->xprt_hlen>>2);
	/* The iov_len does not include the transport header bytes */
	rqstp->rq_arg.head[0].iov_len = (dr->argslen<<2) - dr->xprt_hlen;
	rqstp->rq_arg.page_len = 0;
	/* The rq_arg.len includes the transport header bytes */
	rqstp->rq_arg.len     = dr->argslen<<2;
	rqstp->rq_prot        = dr->prot;
	memcpy(&rqstp->rq_addr, &dr->addr, dr->addrlen);
	rqstp->rq_addrlen     = dr->addrlen;
	/* Save off transport header len in case we get deferred again */
	rqstp->rq_xprt_hlen   = dr->xprt_hlen;
	rqstp->rq_daddr       = dr->daddr;
	rqstp->rq_respages    = rqstp->rq_pages;
	return (dr->argslen<<2) - dr->xprt_hlen;
}


static struct svc_deferred_req *svc_deferred_dequeue(struct svc_xprt *xprt)
{
	struct svc_deferred_req *dr = NULL;

	if (!test_bit(XPT_DEFERRED, &xprt->xpt_flags))
		return NULL;
	spin_lock(&xprt->xpt_lock);
	if (!list_empty(&xprt->xpt_deferred)) {
		dr = list_entry(xprt->xpt_deferred.next,
				struct svc_deferred_req,
				handle.recent);
		list_del_init(&dr->handle.recent);
	} else
		clear_bit(XPT_DEFERRED, &xprt->xpt_flags);
	spin_unlock(&xprt->xpt_lock);
	return dr;
}

/**
 * svc_find_xprt - find an RPC transport instance
 * @serv: pointer to svc_serv to search
 * @xcl_name: C string containing transport's class name
 * @net: owner net pointer
 * @af: Address family of transport's local address
 * @port: transport's IP port number
 *
 * Return the transport instance pointer for the endpoint accepting
 * connections/peer traffic from the specified transport class,
 * address family and port.
 *
 * Specifying 0 for the address family or port is effectively a
 * wild-card, and will result in matching the first transport in the
 * service's list that has a matching class name.
 */
struct svc_xprt *svc_find_xprt(struct svc_serv *serv, const char *xcl_name,
			       struct net *net, const sa_family_t af,
			       const unsigned short port)
{
	struct svc_xprt *xprt;
	struct svc_xprt *found = NULL;

	/* Sanity check the args */
	if (serv == NULL || xcl_name == NULL)
		return found;

	spin_lock_bh(&serv->sv_lock);
	list_for_each_entry(xprt, &serv->sv_permsocks, xpt_list) {
		if (xprt->xpt_net != net)
			continue;
		if (strcmp(xprt->xpt_class->xcl_name, xcl_name))
			continue;
		if (af != AF_UNSPEC && af != xprt->xpt_local.ss_family)
			continue;
		if (port != 0 && port != svc_xprt_local_port(xprt))
			continue;
		found = xprt;
		svc_xprt_get(xprt);
		break;
	}
	spin_unlock_bh(&serv->sv_lock);
	return found;
}
EXPORT_SYMBOL_GPL(svc_find_xprt);

static int svc_one_xprt_name(const struct svc_xprt *xprt,
			     char *pos, int remaining)
{
	int len;

	len = snprintf(pos, remaining, "%s %u\n",
			xprt->xpt_class->xcl_name,
			svc_xprt_local_port(xprt));
	if (len >= remaining)
		return -ENAMETOOLONG;
	return len;
}

/**
 * svc_xprt_names - format a buffer with a list of transport names
 * @serv: pointer to an RPC service
 * @buf: pointer to a buffer to be filled in
 * @buflen: length of buffer to be filled in
 *
 * Fills in @buf with a string containing a list of transport names,
 * each name terminated with '\n'.
 *
 * Returns positive length of the filled-in string on success; otherwise
 * a negative errno value is returned if an error occurs.
 */
int svc_xprt_names(struct svc_serv *serv, char *buf, const int buflen)
{
	struct svc_xprt *xprt;
	int len, totlen;
	char *pos;

	/* Sanity check args */
	if (!serv)
		return 0;

	spin_lock_bh(&serv->sv_lock);

	pos = buf;
	totlen = 0;
	list_for_each_entry(xprt, &serv->sv_permsocks, xpt_list) {
		len = svc_one_xprt_name(xprt, pos, buflen - totlen);
		if (len < 0) {
			*buf = '\0';
			totlen = len;
		}
		if (len <= 0)
			break;

		pos += len;
		totlen += len;
	}

	spin_unlock_bh(&serv->sv_lock);
	return totlen;
}
EXPORT_SYMBOL_GPL(svc_xprt_names);


/*----------------------------------------------------------------------------*/

static void *svc_pool_stats_start(struct seq_file *m, loff_t *pos)
{
	unsigned int pidx = (unsigned int)*pos;
	struct svc_serv *serv = m->private;

	dprintk("svc_pool_stats_start, *pidx=%u\n", pidx);

	if (!pidx)
		return SEQ_START_TOKEN;
	return (pidx > serv->sv_nrpools ? NULL : &serv->sv_pools[pidx-1]);
}

static void *svc_pool_stats_next(struct seq_file *m, void *p, loff_t *pos)
{
	struct svc_pool *pool = p;
	struct svc_serv *serv = m->private;

	dprintk("svc_pool_stats_next, *pos=%llu\n", *pos);

	if (p == SEQ_START_TOKEN) {
		pool = &serv->sv_pools[0];
	} else {
		unsigned int pidx = (pool - &serv->sv_pools[0]);
		if (pidx < serv->sv_nrpools-1)
			pool = &serv->sv_pools[pidx+1];
		else
			pool = NULL;
	}
	++*pos;
	return pool;
}

static void svc_pool_stats_stop(struct seq_file *m, void *p)
{
}

static int svc_pool_stats_show(struct seq_file *m, void *p)
{
	struct svc_pool *pool = p;

	if (p == SEQ_START_TOKEN) {
		seq_puts(m, "# pool packets-arrived sockets-enqueued threads-woken threads-timedout\n");
		return 0;
	}

	seq_printf(m, "%u %lu %lu %lu %lu\n",
		pool->sp_id,
		(unsigned long)atomic_long_read(&pool->sp_stats.packets),
		pool->sp_stats.sockets_queued,
		(unsigned long)atomic_long_read(&pool->sp_stats.threads_woken),
		(unsigned long)atomic_long_read(&pool->sp_stats.threads_timedout));

	return 0;
}

static const struct seq_operations svc_pool_stats_seq_ops = {
	.start	= svc_pool_stats_start,
	.next	= svc_pool_stats_next,
	.stop	= svc_pool_stats_stop,
	.show	= svc_pool_stats_show,
};

int svc_pool_stats_open(struct svc_serv *serv, struct file *file)
{
	int err;

	err = seq_open(file, &svc_pool_stats_seq_ops);
	if (!err)
		((struct seq_file *) file->private_data)->private = serv;
	return err;
}
EXPORT_SYMBOL(svc_pool_stats_open);

/*----------------------------------------------------------------------------*/
