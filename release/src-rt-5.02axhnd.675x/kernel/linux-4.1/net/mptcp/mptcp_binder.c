#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
#include <linux/module.h>

#include <net/mptcp.h>
#include <net/mptcp_v4.h>

#include <linux/route.h>
#include <linux/inet.h>
#include <linux/mroute.h>
#include <linux/spinlock_types.h>
#include <net/inet_ecn.h>
#include <net/route.h>
#include <net/xfrm.h>
#include <net/compat.h>
#include <linux/slab.h>

#define MPTCP_GW_MAX_LISTS	10
#define MPTCP_GW_LIST_MAX_LEN	6
#define MPTCP_GW_SYSCTL_MAX_LEN	(15 * MPTCP_GW_LIST_MAX_LEN *	\
							MPTCP_GW_MAX_LISTS)

struct mptcp_gw_list {
	struct in_addr list[MPTCP_GW_MAX_LISTS][MPTCP_GW_LIST_MAX_LEN];
	u8 len[MPTCP_GW_MAX_LISTS];
};

struct binder_priv {
	/* Worker struct for subflow establishment */
	struct work_struct subflow_work;

	struct mptcp_cb *mpcb;

	/* Prevent multiple sub-sockets concurrently iterating over sockets */
	spinlock_t *flow_lock;
};

static struct mptcp_gw_list *mptcp_gws;
static rwlock_t mptcp_gws_lock;

static int mptcp_binder_ndiffports __read_mostly = 1;

static char sysctl_mptcp_binder_gateways[MPTCP_GW_SYSCTL_MAX_LEN] __read_mostly;

static int mptcp_get_avail_list_ipv4(struct sock *sk)
{
	int i, j, list_taken, opt_ret, opt_len;
	unsigned char *opt_ptr, *opt_end_ptr, opt[MAX_IPOPTLEN];

	for (i = 0; i < MPTCP_GW_MAX_LISTS; ++i) {
		if (mptcp_gws->len[i] == 0)
			goto error;

		mptcp_debug("mptcp_get_avail_list_ipv4: List %i\n", i);
		list_taken = 0;

		/* Loop through all sub-sockets in this connection */
		mptcp_for_each_sk(tcp_sk(sk)->mpcb, sk) {
			mptcp_debug("mptcp_get_avail_list_ipv4: Next sock\n");

			/* Reset length and options buffer, then retrieve
			 * from socket
			 */
			opt_len = MAX_IPOPTLEN;
			memset(opt, 0, MAX_IPOPTLEN);
			opt_ret = ip_getsockopt(sk, IPPROTO_IP,
				IP_OPTIONS, opt, &opt_len);
			if (opt_ret < 0) {
				mptcp_debug(KERN_ERR "%s: MPTCP subsocket getsockopt() IP_OPTIONS failed, error %d\n",
					    __func__, opt_ret);
				goto error;
			}

			/* If socket has no options, it has no stake in this list */
			if (opt_len <= 0)
				continue;

			/* Iterate options buffer */
			for (opt_ptr = &opt[0]; opt_ptr < &opt[opt_len]; opt_ptr++) {
				if (*opt_ptr == IPOPT_LSRR) {
					mptcp_debug("mptcp_get_avail_list_ipv4: LSRR options found\n");
					goto sock_lsrr;
				}
			}
			continue;

sock_lsrr:
			/* Pointer to the 2nd to last address */
			opt_end_ptr = opt_ptr+(*(opt_ptr+1))-4;

			/* Addresses start 3 bytes after type offset */
			opt_ptr += 3;
			j = 0;

			/* Different length lists cannot be the same */
			if ((opt_end_ptr-opt_ptr)/4 != mptcp_gws->len[i])
				continue;

			/* Iterate if we are still inside options list
			 * and sysctl list
			 */
			while (opt_ptr < opt_end_ptr && j < mptcp_gws->len[i]) {
				/* If there is a different address, this list must
				 * not be set on this socket
				 */
				if (memcmp(&mptcp_gws->list[i][j], opt_ptr, 4))
					break;

				/* Jump 4 bytes to next address */
				opt_ptr += 4;
				j++;
			}

			/* Reached the end without a differing address, lists
			 * are therefore identical.
			 */
			if (j == mptcp_gws->len[i]) {
				mptcp_debug("mptcp_get_avail_list_ipv4: List already used\n");
				list_taken = 1;
				break;
			}
		}

		/* Free list found if not taken by a socket */
		if (!list_taken) {
			mptcp_debug("mptcp_get_avail_list_ipv4: List free\n");
			break;
		}
	}

	if (i >= MPTCP_GW_MAX_LISTS)
		goto error;

	return i;
error:
	return -1;
}

/* The list of addresses is parsed each time a new connection is opened,
 *  to make sure it's up to date. In case of error, all the lists are
 *  marked as unavailable and the subflow's fingerprint is set to 0.
 */
static void mptcp_v4_add_lsrr(struct sock *sk, struct in_addr addr)
{
	int i, j, ret;
	unsigned char opt[MAX_IPOPTLEN] = {0};
	struct tcp_sock *tp = tcp_sk(sk);
	struct binder_priv *fmp = (struct binder_priv *)&tp->mpcb->mptcp_pm[0];

	/* Read lock: multiple sockets can read LSRR addresses at the same
	 * time, but writes are done in mutual exclusion.
	 * Spin lock: must search for free list for one socket at a time, or
	 * multiple sockets could take the same list.
	 */
	read_lock(&mptcp_gws_lock);
	spin_lock(fmp->flow_lock);

	i = mptcp_get_avail_list_ipv4(sk);

	/* Execution enters here only if a free path is found.
	 */
	if (i >= 0) {
		opt[0] = IPOPT_NOP;
		opt[1] = IPOPT_LSRR;
		opt[2] = sizeof(mptcp_gws->list[i][0].s_addr) *
				(mptcp_gws->len[i] + 1) + 3;
		opt[3] = IPOPT_MINOFF;
		for (j = 0; j < mptcp_gws->len[i]; ++j)
			memcpy(opt + 4 +
				(j * sizeof(mptcp_gws->list[i][0].s_addr)),
				&mptcp_gws->list[i][j].s_addr,
				sizeof(mptcp_gws->list[i][0].s_addr));
		/* Final destination must be part of IP_OPTIONS parameter. */
		memcpy(opt + 4 + (j * sizeof(addr.s_addr)), &addr.s_addr,
		       sizeof(addr.s_addr));

		/* setsockopt must be inside the lock, otherwise another
		 * subflow could fail to see that we have taken a list.
		 */
		ret = ip_setsockopt(sk, IPPROTO_IP, IP_OPTIONS, opt,
				4 + sizeof(mptcp_gws->list[i][0].s_addr)
				* (mptcp_gws->len[i] + 1));

		if (ret < 0) {
			mptcp_debug(KERN_ERR "%s: MPTCP subsock setsockopt() IP_OPTIONS failed, error %d\n",
				    __func__, ret);
		}
	}

	spin_unlock(fmp->flow_lock);
	read_unlock(&mptcp_gws_lock);

	return;
}

/* Parses gateways string for a list of paths to different
 * gateways, and stores them for use with the Loose Source Routing (LSRR)
 * socket option. Each list must have "," separated addresses, and the lists
 * themselves must be separated by "-". Returns -1 in case one or more of the
 * addresses is not a valid ipv4/6 address.
 */
static int mptcp_parse_gateway_ipv4(char *gateways)
{
	int i, j, k, ret;
	char *tmp_string = NULL;
	struct in_addr tmp_addr;

	tmp_string = kzalloc(16, GFP_KERNEL);
	if (tmp_string == NULL)
		return -ENOMEM;

	write_lock(&mptcp_gws_lock);

	memset(mptcp_gws, 0, sizeof(struct mptcp_gw_list));

	/* A TMP string is used since inet_pton needs a null terminated string
	 * but we do not want to modify the sysctl for obvious reasons.
	 * i will iterate over the SYSCTL string, j will iterate over the
	 * temporary string where each IP is copied into, k will iterate over
	 * the IPs in each list.
	 */
	for (i = j = k = 0;
			i < MPTCP_GW_SYSCTL_MAX_LEN && k < MPTCP_GW_MAX_LISTS;
			++i) {
		if (gateways[i] == '-' || gateways[i] == ',' || gateways[i] == '\0') {
			/* If the temp IP is empty and the current list is
			 *  empty, we are done.
			 */
			if (j == 0 && mptcp_gws->len[k] == 0)
				break;

			/* Terminate the temp IP string, then if it is
			 * non-empty parse the IP and copy it.
			 */
			tmp_string[j] = '\0';
			if (j > 0) {
				mptcp_debug("mptcp_parse_gateway_list tmp: %s i: %d\n", tmp_string, i);

				ret = in4_pton(tmp_string, strlen(tmp_string),
						(u8 *)&tmp_addr.s_addr, '\0',
						NULL);

				if (ret) {
					mptcp_debug("mptcp_parse_gateway_list ret: %d s_addr: %pI4\n",
						    ret,
						    &tmp_addr.s_addr);
					memcpy(&mptcp_gws->list[k][mptcp_gws->len[k]].s_addr,
					       &tmp_addr.s_addr,
					       sizeof(tmp_addr.s_addr));
					mptcp_gws->len[k]++;
					j = 0;
					tmp_string[j] = '\0';
					/* Since we can't impose a limit to
					 * what the user can input, make sure
					 * there are not too many IPs in the
					 * SYSCTL string.
					 */
					if (mptcp_gws->len[k] > MPTCP_GW_LIST_MAX_LEN) {
						mptcp_debug("mptcp_parse_gateway_list too many members in list %i: max %i\n",
							    k,
							    MPTCP_GW_LIST_MAX_LEN);
						goto error;
					}
				} else {
					goto error;
				}
			}

			if (gateways[i] == '-' || gateways[i] == '\0')
				++k;
		} else {
			tmp_string[j] = gateways[i];
			++j;
		}
	}

	/* Number of flows is number of gateway lists plus master flow */
	mptcp_binder_ndiffports = k+1;

	write_unlock(&mptcp_gws_lock);
	kfree(tmp_string);

	return 0;

error:
	memset(mptcp_gws, 0, sizeof(struct mptcp_gw_list));
	memset(gateways, 0, sizeof(char) * MPTCP_GW_SYSCTL_MAX_LEN);
	write_unlock(&mptcp_gws_lock);
	kfree(tmp_string);
	return -1;
}

/**
 * Create all new subflows, by doing calls to mptcp_initX_subsockets
 *
 * This function uses a goto next_subflow, to allow releasing the lock between
 * new subflows and giving other processes a chance to do some work on the
 * socket and potentially finishing the communication.
 **/
static void create_subflow_worker(struct work_struct *work)
{
	const struct binder_priv *pm_priv = container_of(work,
						     struct binder_priv,
						     subflow_work);
	struct mptcp_cb *mpcb = pm_priv->mpcb;
	struct sock *meta_sk = mpcb->meta_sk;
	int iter = 0;

next_subflow:
	if (iter) {
		release_sock(meta_sk);
		mutex_unlock(&mpcb->mpcb_mutex);

		cond_resched();
	}
	mutex_lock(&mpcb->mpcb_mutex);
	lock_sock_nested(meta_sk, SINGLE_DEPTH_NESTING);

	iter++;

	if (sock_flag(meta_sk, SOCK_DEAD))
		goto exit;

	if (mpcb->master_sk &&
	    !tcp_sk(mpcb->master_sk)->mptcp->fully_established)
		goto exit;

	if (mptcp_binder_ndiffports > iter &&
	    mptcp_binder_ndiffports > mpcb->cnt_subflows) {
		struct mptcp_loc4 loc;
		struct mptcp_rem4 rem;

		loc.addr.s_addr = inet_sk(meta_sk)->inet_saddr;
		loc.loc4_id = 0;
		loc.low_prio = 0;

		rem.addr.s_addr = inet_sk(meta_sk)->inet_daddr;
		rem.port = inet_sk(meta_sk)->inet_dport;
		rem.rem4_id = 0; /* Default 0 */

		mptcp_init4_subsockets(meta_sk, &loc, &rem);

		goto next_subflow;
	}

exit:
	release_sock(meta_sk);
	mutex_unlock(&mpcb->mpcb_mutex);
	sock_put(meta_sk);
}

static void binder_new_session(const struct sock *meta_sk)
{
	struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;
	struct binder_priv *fmp = (struct binder_priv *)&mpcb->mptcp_pm[0];
	static DEFINE_SPINLOCK(flow_lock);

#if IS_ENABLED(CONFIG_IPV6)
	if (meta_sk->sk_family == AF_INET6 &&
	    !mptcp_v6_is_v4_mapped(meta_sk)) {
			mptcp_fallback_default(mpcb);
			return;
	}
#endif

	/* Initialize workqueue-struct */
	INIT_WORK(&fmp->subflow_work, create_subflow_worker);
	fmp->mpcb = mpcb;

	fmp->flow_lock = &flow_lock;
}

static void binder_create_subflows(struct sock *meta_sk)
{
	struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;
	struct binder_priv *pm_priv = (struct binder_priv *)&mpcb->mptcp_pm[0];

	if (mpcb->infinite_mapping_snd || mpcb->infinite_mapping_rcv ||
	    mpcb->send_infinite_mapping ||
	    mpcb->server_side || sock_flag(meta_sk, SOCK_DEAD))
		return;

	if (!work_pending(&pm_priv->subflow_work)) {
		sock_hold(meta_sk);
		queue_work(mptcp_wq, &pm_priv->subflow_work);
	}
}

static int binder_get_local_id(sa_family_t family, union inet_addr *addr,
				  struct net *net, bool *low_prio)
{
	return 0;
}

/* Callback functions, executed when syctl mptcp.mptcp_gateways is updated.
 * Inspired from proc_tcp_congestion_control().
 */
static int proc_mptcp_gateways(struct ctl_table *ctl, int write,
			       void __user *buffer, size_t *lenp,
			       loff_t *ppos)
{
	int ret;
	struct ctl_table tbl = {
		.maxlen = MPTCP_GW_SYSCTL_MAX_LEN,
	};

	if (write) {
		tbl.data = kzalloc(MPTCP_GW_SYSCTL_MAX_LEN, GFP_KERNEL);
		if (tbl.data == NULL)
			return -1;
		ret = proc_dostring(&tbl, write, buffer, lenp, ppos);
		if (ret == 0) {
			ret = mptcp_parse_gateway_ipv4(tbl.data);
			memcpy(ctl->data, tbl.data, MPTCP_GW_SYSCTL_MAX_LEN);
		}
		kfree(tbl.data);
	} else {
		ret = proc_dostring(ctl, write, buffer, lenp, ppos);
	}


	return ret;
}

static struct mptcp_pm_ops binder __read_mostly = {
	.new_session = binder_new_session,
	.fully_established = binder_create_subflows,
	.get_local_id = binder_get_local_id,
	.init_subsocket_v4 = mptcp_v4_add_lsrr,
	.name = "binder",
	.owner = THIS_MODULE,
};

static struct ctl_table binder_table[] = {
	{
		.procname = "mptcp_binder_gateways",
		.data = &sysctl_mptcp_binder_gateways,
		.maxlen = sizeof(char) * MPTCP_GW_SYSCTL_MAX_LEN,
		.mode = 0644,
		.proc_handler = &proc_mptcp_gateways
	},
	{ }
};

struct ctl_table_header *mptcp_sysctl_binder;

/* General initialization of MPTCP_PM */
static int __init binder_register(void)
{
	mptcp_gws = kzalloc(sizeof(*mptcp_gws), GFP_KERNEL);
	if (!mptcp_gws)
		return -ENOMEM;

	rwlock_init(&mptcp_gws_lock);

	BUILD_BUG_ON(sizeof(struct binder_priv) > MPTCP_PM_SIZE);

	mptcp_sysctl_binder = register_net_sysctl(&init_net, "net/mptcp",
			binder_table);
	if (!mptcp_sysctl_binder)
		goto sysctl_fail;

	if (mptcp_register_path_manager(&binder))
		goto pm_failed;

	return 0;

pm_failed:
	unregister_net_sysctl_table(mptcp_sysctl_binder);
sysctl_fail:
	kfree(mptcp_gws);

	return -1;
}

static void binder_unregister(void)
{
	mptcp_unregister_path_manager(&binder);
	unregister_net_sysctl_table(mptcp_sysctl_binder);
	kfree(mptcp_gws);
}

module_init(binder_register);
module_exit(binder_unregister);

MODULE_AUTHOR("Luca Boccassi, Duncan Eastoe, Christoph Paasch (ndiffports)");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BINDER MPTCP");
MODULE_VERSION("0.1");
#endif
