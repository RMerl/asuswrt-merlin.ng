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

#include "cache.h"
#include "hash.h"
#include "log.h"
#include "conntrackd.h"
#include "netlink.h"
#include "event.h"
#include "jhash.h"
#include "network.h"

#include <errno.h>
#include <string.h>
#include <time.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static uint32_t
cache_hash4_ct(const struct nf_conntrack *ct, const struct hashtable *table)
{
	uint32_t a[4] = {
		[0]	= nfct_get_attr_u32(ct, ATTR_IPV4_SRC),
		[1]	= nfct_get_attr_u32(ct, ATTR_IPV4_DST),
		[2]	= nfct_get_attr_u8(ct, ATTR_L3PROTO) << 16 |
			  nfct_get_attr_u8(ct, ATTR_L4PROTO),
		[3]	= nfct_get_attr_u16(ct, ATTR_PORT_SRC) << 16 |
			  nfct_get_attr_u16(ct, ATTR_PORT_DST),
	};

	/*
	 * Instead of returning hash % table->hashsize (implying a divide)
	 * we return the high 32 bits of the (hash * table->hashsize) that will
	 * give results between [0 and hashsize-1] and same hash distribution,
	 * but using a multiply, less expensive than a divide. See:
	 * http://www.mail-archive.com/netdev@vger.kernel.org/msg56623.html
	 */
	return ((uint64_t)jhash2(a, 4, 0) * table->hashsize) >> 32;
}

static uint32_t
cache_hash6_ct(const struct nf_conntrack *ct, const struct hashtable *table)
{
	uint32_t a[10];

	memcpy(&a[0], nfct_get_attr(ct, ATTR_IPV6_SRC), sizeof(uint32_t)*4);
	memcpy(&a[4], nfct_get_attr(ct, ATTR_IPV6_DST), sizeof(uint32_t)*4);
	a[8] = nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO) << 16 |
	       nfct_get_attr_u8(ct, ATTR_ORIG_L4PROTO);
	a[9] = nfct_get_attr_u16(ct, ATTR_ORIG_PORT_SRC) << 16 |
	       nfct_get_attr_u16(ct, ATTR_ORIG_PORT_DST);

	return ((uint64_t)jhash2(a, 10, 0) * table->hashsize) >> 32;
}

static uint32_t
cache_ct_hash(const void *data, const struct hashtable *table)
{
	int ret = 0;
	const struct nf_conntrack *ct = data;

	switch(nfct_get_attr_u8(ct, ATTR_L3PROTO)) {
		case AF_INET:
			ret = cache_hash4_ct(ct, table);
			break;
		case AF_INET6:
			ret = cache_hash6_ct(ct, table);
			break;
		default:
			dlog(LOG_ERR, "unknown layer 3 proto in hash");
			break;
	}
	return ret;
}

/* master conntrack of expectations have no ID */
static inline int
cache_ct_cmp_id(const struct nf_conntrack *ct1, const struct nf_conntrack *ct2)
{
	return nfct_attr_is_set(ct2, ATTR_ID) ?
	       nfct_get_attr_u32(ct1, ATTR_ID) == nfct_get_attr_u32(ct2, ATTR_ID) : 1;
}

static int cache_ct_cmp(const void *data1, const void *data2)
{
	const struct cache_object *obj = data1;
	const struct nf_conntrack *ct = data2;

	return nfct_cmp(obj->ptr, ct, NFCT_CMP_ORIG) &&
	       cache_ct_cmp_id(obj->ptr, ct);
}

static void *cache_ct_alloc(void)
{
	return nfct_new();
}

static void cache_ct_free(void *ptr)
{
	nfct_destroy(ptr);
}

static void cache_ct_copy(void *dst, void *src, unsigned int flags)
{
	nfct_copy(dst, src, flags);
}

static int cache_ct_dump_step(void *data1, void *n)
{
	char buf[1024];
	int size;
	struct __dump_container *container = data1;
	struct cache_object *obj = n;
	char *data = obj->data;
	unsigned i;

	/*
	 * XXX: Do not dump the entries that are scheduled to expire.
	 * 	These entries talk about already destroyed connections
	 * 	that we keep for some time just in case that we have to
	 * 	resent some lost messages. We do not show them to the
	 * 	user as he may think that the firewall replicas are not
	 * 	in sync. The branch below is a hack as it is quite
	 * 	specific and it breaks conntrackd modularity. Probably
	 * 	there's a nicer way to do this but until I come up with it...
	 */
	if (CONFIG(flags) & CTD_SYNC_FTFW && obj->status == C_OBJ_DEAD)
		return 0;

	/* do not show cached timeout, this may confuse users */
	if (nfct_attr_is_set(obj->ptr, ATTR_TIMEOUT))
		nfct_attr_unset(obj->ptr, ATTR_TIMEOUT);

	memset(buf, 0, sizeof(buf));
	size = nfct_snprintf(buf, 
			     sizeof(buf), 
			     obj->ptr,
			     NFCT_T_UNKNOWN, 
			     container->type,
			     0);

	for (i = 0; i < obj->cache->num_features; i++) {
		if (obj->cache->features[i]->dump) {
			size += obj->cache->features[i]->dump(obj, 
							      data, 
							      buf+size,
							      container->type);
			data += obj->cache->features[i]->size;
		}
	}
	if (container->type != NFCT_O_XML) {
		long tm = time(NULL);
		size += sprintf(buf+size, " [active since %lds]",
				tm - obj->lifetime);
	}
	size += sprintf(buf+size, "\n");
	if (send(container->fd, buf, size, 0) == -1) {
		if (errno != EPIPE)
			return -1;
	}

	return 0;
}

static void
cache_ct_commit_step(struct __commit_container *tmp, struct cache_object *obj)
{
	int ret, retry = 1, timeout;
	struct nf_conntrack *ct = obj->ptr;

	if (CONFIG(commit_timeout)) {
		timeout = CONFIG(commit_timeout);
	} else {
		timeout = time(NULL) - obj->lastupdate;
		if (timeout < 0) {
			/* XXX: Arbitrarily set the timer to one minute, how
			 * can this happen? For example, an adjustment due to
			 * daylight-saving. Probably other situations can
			 * trigger this. */
			timeout = 60;
		}
		/* calculate an estimation of the current timeout */
		timeout = nfct_get_attr_u32(ct, ATTR_TIMEOUT) - timeout;
		if (timeout < 0) {
			timeout = 60;
		}
	}

retry:
	if (nl_create_conntrack(tmp->h, ct, timeout) == -1) {
		if (errno == EEXIST && retry == 1) {
			ret = nl_destroy_conntrack(tmp->h, ct);
			if (ret == 0 || (ret == -1 && errno == ENOENT)) {
				if (retry) {
					retry = 0;
					goto retry;
				}
			}
			dlog(LOG_ERR, "commit-destroy: %s", strerror(errno));
			dlog_ct(STATE(log), ct, NFCT_O_PLAIN);
			tmp->c->stats.commit_fail++;
		} else {
			dlog(LOG_ERR, "commit-create: %s", strerror(errno));
			dlog_ct(STATE(log), ct, NFCT_O_PLAIN);
			tmp->c->stats.commit_fail++;
		}
	} else {
		tmp->c->stats.commit_ok++;
	}
}

static int cache_ct_commit_related(void *data, void *n)
{
	struct cache_object *obj = n;

	if (ct_is_related(obj->ptr))
		cache_ct_commit_step(data, obj);

	/* keep iterating even if we have found errors */
	return 0;
}

static int cache_ct_commit_master(void *data, void *n)
{
	struct cache_object *obj = n;

	if (ct_is_related(obj->ptr))
		return 0;

	cache_ct_commit_step(data, obj);
	return 0;
}

static int cache_ct_commit(struct cache *c, struct nfct_handle *h, int clientfd)
{
	unsigned int commit_ok, commit_fail;
	struct __commit_container tmp = {
		.h = h,
		.c = c,
	};
	struct timeval commit_stop, res;

	/* we already have one commit in progress, skip this. The clientfd
	 * descriptor has to be closed by the caller. */
	if (clientfd && STATE_SYNC(commit).clientfd != -1)
		return -1;

	switch(STATE_SYNC(commit).state) {
	case COMMIT_STATE_INACTIVE:
		gettimeofday(&STATE_SYNC(commit).stats.start, NULL);
		STATE_SYNC(commit).stats.ok = c->stats.commit_ok;
		STATE_SYNC(commit).stats.fail = c->stats.commit_fail;
		STATE_SYNC(commit).clientfd = clientfd;
	case COMMIT_STATE_MASTER:
		STATE_SYNC(commit).current =
			hashtable_iterate_limit(c->h, &tmp,
						STATE_SYNC(commit).current,
						CONFIG(general).commit_steps,
						cache_ct_commit_master);
		if (STATE_SYNC(commit).current < CONFIG(hashsize)) {
			STATE_SYNC(commit).state = COMMIT_STATE_MASTER;
			/* give it another step as soon as possible */
			write_evfd(STATE_SYNC(commit).evfd);
			return 1;
		}
		STATE_SYNC(commit).current = 0;
		STATE_SYNC(commit).state = COMMIT_STATE_RELATED;
	case COMMIT_STATE_RELATED:
		STATE_SYNC(commit).current =
			hashtable_iterate_limit(c->h, &tmp,
						STATE_SYNC(commit).current,
						CONFIG(general).commit_steps,
						cache_ct_commit_related);
		if (STATE_SYNC(commit).current < CONFIG(hashsize)) {
			STATE_SYNC(commit).state = COMMIT_STATE_RELATED;
			/* give it another step as soon as possible */
			write_evfd(STATE_SYNC(commit).evfd);
			return 1;
		}
		/* calculate the time that commit has taken */
		gettimeofday(&commit_stop, NULL);
		timersub(&commit_stop, &STATE_SYNC(commit).stats.start, &res);

		/* calculate new entries committed */
		commit_ok = c->stats.commit_ok - STATE_SYNC(commit).stats.ok;
		commit_fail = 
			c->stats.commit_fail - STATE_SYNC(commit).stats.fail;

		/* log results */
		dlog(LOG_NOTICE, "Committed %u new entries", commit_ok);

		if (commit_fail)
			dlog(LOG_NOTICE, "%u entries can't be "
					 "committed", commit_fail);

		dlog(LOG_NOTICE, "commit has taken %lu.%06lu seconds", 
				res.tv_sec, res.tv_usec);

		/* prepare the state machine for new commits */
		STATE_SYNC(commit).current = 0;
		STATE_SYNC(commit).state = COMMIT_STATE_INACTIVE;

		return 0;
	}
	return 1;
}

static struct nethdr *
cache_ct_build_msg(const struct cache_object *obj, int type)
{
	return BUILD_NETMSG_FROM_CT(obj->ptr, type);
}

/* template to cache conntracks coming from the kernel. */
struct cache_ops cache_sync_internal_ct_ops = {
	.hash		= cache_ct_hash,
	.cmp		= cache_ct_cmp,
	.alloc		= cache_ct_alloc,
	.free		= cache_ct_free,
	.copy		= cache_ct_copy,
	.dump_step	= cache_ct_dump_step,
	.commit		= NULL,
	.build_msg	= cache_ct_build_msg,
};

/* template to cache conntracks coming from the network. */
struct cache_ops cache_sync_external_ct_ops = {
	.hash		= cache_ct_hash,
	.cmp		= cache_ct_cmp,
	.alloc		= cache_ct_alloc,
	.free		= cache_ct_free,
	.copy		= cache_ct_copy,
	.dump_step	= cache_ct_dump_step,
	.commit		= cache_ct_commit,
	.build_msg	= NULL,
};

/* template to cache conntracks for the statistics mode. */
struct cache_ops cache_stats_ct_ops = {
	.hash		= cache_ct_hash,
	.cmp		= cache_ct_cmp,
	.alloc		= cache_ct_alloc,
	.free		= cache_ct_free,
	.copy		= cache_ct_copy,
	.dump_step	= cache_ct_dump_step,
	.commit		= NULL,
	.build_msg	= NULL,
};
