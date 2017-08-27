/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_cache.c
 * @brief Cache values and merge them back into future requests.
 *
 * @copyright 2012-2013  The FreeRADIUS server project
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/heap.h>
#include <freeradius-devel/rad_assert.h>

#define PW_CACHE_TTL		1140
#define PW_CACHE_STATUS_ONLY	1141
#define PW_CACHE_MERGE		1142
#define PW_CACHE_ENTRY_HITS	1143

/*
 *	Define a structure for our module configuration.
 *
 *	These variables do not need to be in a structure, but it's
 *	a lot cleaner to do so, and a pointer to the structure can
 *	be used as the instance handle.
 */
typedef struct rlm_cache_t {
	char const		*xlat_name;
	char			*key;
	int			ttl;
	int			max_entries;
	int			epoch;
	int			stats;
	CONF_SECTION		*cs;
	rbtree_t		*cache;
	fr_heap_t		*heap;

	value_pair_map_t	*maps;	//!< Attribute map applied to users
					//!< and profiles.
#ifdef HAVE_PTHREAD_H
	pthread_mutex_t	cache_mutex;
#endif
} rlm_cache_t;

typedef struct rlm_cache_entry_t {
	char const	*key;
	int		offset;
	long long int	hits;
	time_t		created;
	time_t		expires;
	VALUE_PAIR	*control;
	VALUE_PAIR	*packet;
	VALUE_PAIR	*reply;
} rlm_cache_entry_t;

#ifdef HAVE_PTHREAD_H
#define PTHREAD_MUTEX_LOCK pthread_mutex_lock
#define PTHREAD_MUTEX_UNLOCK pthread_mutex_unlock
#else
#define PTHREAD_MUTEX_LOCK(_x)
#define PTHREAD_MUTEX_UNLOCK(_x)
#endif

#define MAX_ATTRMAP	128

/*
 *	Compare two entries by key.  There may only be one entry with
 *	the same key.
 */
static int cache_entry_cmp(void const *one, void const *two)
{
	rlm_cache_entry_t const *a = one;
	rlm_cache_entry_t const *b = two;

	return strcmp(a->key, b->key);
}

static void cache_entry_free(void *data)
{
	rlm_cache_entry_t *c = data;

	pairfree(&c->control);
	pairfree(&c->packet);
	pairfree(&c->reply);

	talloc_free(c);
}

/*
 *	Compare two entries by expiry time.  There may be multiple
 *	entries with the same expiry time.
 */
static int cache_heap_cmp(void const *one, void const *two)
{
	rlm_cache_entry_t const *a = one;
	rlm_cache_entry_t const *b = two;

	if (a->expires < b->expires) return -1;
	if (a->expires > b->expires) return +1;

	return 0;
}

/*
 *	Merge a cached entry into a REQUEST.
 */
static void cache_merge(rlm_cache_t *inst, REQUEST *request,
			rlm_cache_entry_t *c)
{
	VALUE_PAIR *vp;

	rad_assert(request != NULL);
	rad_assert(c != NULL);

	vp = pairfind(request->config_items, PW_CACHE_MERGE, 0, TAG_ANY);
	if (vp && (vp->vp_integer == 0)) {
		RDEBUG2("Told not to merge entry into request");
		return;
	}

	if (c->control) {
		RDEBUG2("Merging cached control list:");
		rdebug_pair_list(2, request, c->control);

		pairadd(&request->config_items, paircopy(request, c->control));
	}

	if (c->packet && request->packet) {
		RDEBUG2("Merging cached request list:");
		rdebug_pair_list(2, request, c->packet);

		pairadd(&request->packet->vps,
			paircopy(request->packet, c->packet));
	}

	if (c->reply && request->reply) {
		RDEBUG2("Merging cached reply list:");
		rdebug_pair_list(2, request, c->reply);

		pairadd(&request->reply->vps,
			paircopy(request->reply, c->reply));
	}

	if (inst->stats) {
		vp = paircreate(request->packet, PW_CACHE_ENTRY_HITS, 0);
		rad_assert(vp != NULL);

		vp->vp_integer = c->hits;

		pairadd(&request->packet->vps, vp);
	}
}


/*
 *	Find a cached entry.
 */
static rlm_cache_entry_t *cache_find(rlm_cache_t *inst, REQUEST *request,
				     char const *key)
{
	int ttl;
	rlm_cache_entry_t *c, my_c;
	VALUE_PAIR *vp;

	/*
	 *	Look at the expiry heap.
	 */
	c = fr_heap_peek(inst->heap);
	if (!c) {
		rad_assert(rbtree_num_elements(inst->cache) == 0);
		return NULL;
	}

	/*
	 *	If it's time to expire an old entry, do so now.
	 */
	if (c->expires < request->timestamp) {
		fr_heap_extract(inst->heap, c);
		rbtree_deletebydata(inst->cache, c);
	}

	/*
	 *	Is there an entry for this key?
	 */
	my_c.key = key;
	c = rbtree_finddata(inst->cache, &my_c);
	if (!c) return NULL;

	/*
	 *	Yes, but it expired, OR the "forget all" epoch has
	 *	passed.  Delete it, and pretend it doesn't exist.
	 */
	if ((c->expires < request->timestamp) ||
	    (c->created < inst->epoch)) {
	delete:
		RDEBUG("Entry has expired, removing");

		fr_heap_extract(inst->heap, c);
		rbtree_deletebydata(inst->cache, c);

		return NULL;
	}

	RDEBUG("Found entry for \"%s\"", key);

	/*
	 *	Update the expiry time based on the TTL.
	 *	A TTL of 0 means "delete from the cache".
	 */
	vp = pairfind(request->config_items, PW_CACHE_TTL, 0, TAG_ANY);
	if (vp) {
		if (vp->vp_integer == 0) goto delete;

		ttl = vp->vp_integer;
		c->expires = request->timestamp + ttl;
		RDEBUG("Adding %d to the TTL", ttl);
	}
	c->hits++;

	return c;
}


/*
 *	Add an entry to the cache.
 */
static rlm_cache_entry_t *cache_add(rlm_cache_t *inst, REQUEST *request,
				    char const *key)
{
	int ttl;
	VALUE_PAIR *vp, *found, **to_req, **to_cache, **from;
	DICT_ATTR const *da;

	int merge = true;
	REQUEST *context;

	value_pair_map_t const *map;

	rlm_cache_entry_t *c;
	char buffer[1024];

	if (rbtree_num_elements(inst->cache) >= inst->max_entries) {
		RDEBUG("Cache is full: %d entries", inst->max_entries);
		return NULL;
	}

	/*
	 *	TTL of 0 means "don't cache this entry"
	 */
	vp = pairfind(request->config_items, PW_CACHE_TTL, 0, TAG_ANY);
	if (vp && (vp->vp_integer == 0)) return NULL;

	c = talloc_zero(inst, rlm_cache_entry_t);
	c->key = talloc_strdup(c, key);
	c->created = c->expires = request->timestamp;

	/*
	 *	Use per-entry TTL, or globally defined one.
	 */
	if (vp) {
		ttl = vp->vp_integer;
	} else {
		ttl = inst->ttl;
	}
	c->expires += ttl;

	RDEBUG("Creating entry for \"%s\"", key);

	/*
	 *	Check to see if we need to merge the entry into the request
	 */
	vp = pairfind(request->config_items, PW_CACHE_MERGE, 0, TAG_ANY);
	if (vp && (vp->vp_integer == 0)) {
		merge = false;
		RDEBUG2("Told not to merge new entry into request");
	}

	for (map = inst->maps; map != NULL; map = map->next) {
		rad_assert(map->dst && map->src);

		/*
		 *	Specifying inner/outer request doesn't work here
		 *	but there's no easy fix...
		 */
		switch (map->dst->list) {
		case PAIR_LIST_REQUEST:
			to_cache = &c->packet;
			break;

		case PAIR_LIST_REPLY:
			to_cache = &c->reply;
			break;

		case PAIR_LIST_CONTROL:
			to_cache = &c->control;
			break;

		default:
			rad_assert(0);
			return NULL;
		}

		/*
		 *	Resolve the destination in the current request.
		 *	We need to add the to_cache there too if any of these
		 *	are.
		 *	true :
		 *	  - Map specifies an xlat'd string.
		 *	  - Map specifies a literal string.
		 *	  - Map src and dst lists differ.
		 *	  - Map src and dst attributes differ
		 */
		to_req = NULL;
		if (merge && ( !map->src->da ||
		    (map->src->list != map->dst->list) ||
		    (map->src->da != map->dst->da))) {
			context = request;
			/*
			 *	It's ok if the list isn't valid here...
			 *	It might be valid later when we merge
			 *	the cache entry.
			 */
			if (radius_request(&context, map->dst->request) == 0) {
				to_req = radius_list(context, map->dst->list);
			}
		}

		/*
		 *	We infer that src was an attribute ref from the fact
		 *	it contains a da.
		 */
		RDEBUG4(":: dst is \"%s\" src is \"%s\"",
			fr_int2str(vpt_types, map->dst->type, "<INVALID>"),
			fr_int2str(vpt_types, map->src->type, "<INVALID>"));

		switch (map->src->type) {
		case VPT_TYPE_ATTR:
			{
				vp_cursor_t cursor;

				from = NULL;
				da = map->src->da;
				context = request;
				if (radius_request(&context, map->src->request) == 0) {
					from = radius_list(context, map->src->list);
				}

				/*
				 *	Can't add the attribute if the list isn't
				 *	valid.
				 */
				if (!from) continue;

				paircursor(&cursor, from);
				found = pairfindnext(&cursor, da->attr, da->vendor, TAG_ANY);
				if (!found) {
					RWDEBUG("\"%s\" not found, skipping",
					       map->src->name);
					continue;
				}

				RDEBUG("\t%s %s %s", map->dst->name,
				       fr_int2str(fr_tokens, map->op, "<INVALID>"),
				       map->src->name);

				switch (map->op) {
				case T_OP_SET:
				case T_OP_EQ:
				case T_OP_SUB:
					vp = map->dst->type == VPT_TYPE_LIST ?
						paircopyvp(c, found) :
						paircopyvpdata(c, map->dst->da, found);

					if (!vp) continue;

					pairadd(to_cache, vp);

					if (to_req) {
						vp = paircopyvp(request, vp);
						radius_pairmove(request, to_req, vp);
					}

					break;
				case T_OP_ADD:
					do {
						vp = map->dst->type == VPT_TYPE_LIST ?
							paircopyvp(c, found) :
							paircopyvpdata(c, map->dst->da, found);
						if (!vp) continue;

						vp->op = map->op;
						pairadd(to_cache, vp);

						if (to_req) {
							vp = paircopyvp(request, vp);
							radius_pairmove(request, to_req, vp);

						}
					} while ((found = pairfindnext(&cursor, da->attr, da->vendor, TAG_ANY)));
					break;

				default:
					rad_assert(0);
					return NULL;
				}
				break;
			}
		case VPT_TYPE_LIST:
			{
				vp_cursor_t in, out;
				VALUE_PAIR *i;

				rad_assert(map->src->type == VPT_TYPE_LIST);

				from = NULL;
				context = request;
				if (radius_request(&context, map->src->request) == 0) {
					from = radius_list(context, map->src->list);
				}
				if (!from) continue;

				found = NULL;
				paircursor(&out, &found);
				for (i = paircursor(&in, from);
				     i != NULL;
				     i = pairnext(&in)) {
				     	/*
				     	 *	Prevent cache control attributes being added to the cache.
				     	 */
				     	switch (i->da->attr) {
					case PW_CACHE_TTL:
					case PW_CACHE_STATUS_ONLY:
					case PW_CACHE_MERGE:
					case PW_CACHE_ENTRY_HITS:
						RDEBUG("\tskipping %s", i->da->name);
						continue;
					default:
						break;
				     	}

				     	vp = paircopyvp(c, i);
				     	if (!vp) {
				     		pairfree(&found);
				     		return NULL;
				     	}
					RDEBUG("\t%s %s %s (%s)", map->dst->name,
					       fr_int2str(fr_tokens, map->op, "<INVALID>"),
					       map->src->name, vp->da->name);
					vp->op = map->op;
					pairinsert(&out, vp);
				}

				pairadd(to_cache, found);
				if (to_req) {
					vp = paircopy(request, found);
					radius_pairmove(request, to_req, vp);
				}

				break;
			}
		/*
		 *	It was most likely a double quoted string that now
		 *	needs to be expanded.
		 */
		case VPT_TYPE_XLAT:
			if (radius_xlat(buffer, sizeof(buffer), request, map->src->name, NULL, NULL) <= 0) {
				continue;
			}

			RDEBUG("\t%s %s \"%s\"", map->dst->name,
			       fr_int2str(fr_tokens, map->op, "<INVALID>"),
			       buffer);

			vp = pairalloc(NULL, map->dst->da);
			if (!vp) continue;

			vp->op = map->op;
			if (!pairparsevalue(vp, buffer)) {
				pairfree(&vp);
				continue;
			}

			pairadd(to_cache, vp);

			if (to_req) {
				vp = paircopyvp(request, vp);
				radius_pairmove(request, to_req, vp);
			}

			break;
		/*
		 *	Literal string.
		 */
		case VPT_TYPE_LITERAL:
			RDEBUG("\t%s %s '%s'", map->dst->name,
			       fr_int2str(fr_tokens, map->op, "<INVALID>"),
			       map->src->name);

			vp = pairalloc(NULL, map->dst->da);
			if (!vp) continue;

			vp->op = map->op;
			if (!pairparsevalue(vp, map->src->name)) {
				pairfree(&vp);
				continue;
			}

			pairadd(to_cache, vp);

			if (to_req) {
				vp = paircopyvp(request, vp);
				radius_pairmove(request, to_req, vp);
			}

			break;

		default:
			rad_assert(0);
			return NULL;
		}
	}

	if (!rbtree_insert(inst->cache, c)) {
		REDEBUG("FAILED adding entry for key %s", key);
		cache_entry_free(c);
		return NULL;
	}

	if (!fr_heap_insert(inst->heap, c)) {
		REDEBUG("FAILED adding entry for key %s", key);
		rbtree_deletebydata(inst->cache, c);
		return NULL;
	}

	RDEBUG("Inserted entry, TTL %d seconds", ttl);

	return c;
}

/*
 *	Verify that the cache section makes sense.
 */
static int cache_verify(rlm_cache_t *inst, value_pair_map_t **head)
{
	value_pair_map_t *map;

	if (radius_attrmap(cf_section_sub_find(inst->cs, "update"),
			   head, PAIR_LIST_REQUEST,
			   PAIR_LIST_REQUEST, MAX_ATTRMAP) < 0) {
		return -1;
	}

	if (!*head) {
		cf_log_err_cs(inst->cs,
			   "Cache config must contain an update section, and "
			   "that section must not be empty");

		return -1;
	}

	for (map = *head; map != NULL; map = map->next) {
		if ((map->dst->type != VPT_TYPE_ATTR) &&
		    (map->dst->type != VPT_TYPE_LIST)) {
			cf_log_err(map->ci, "Left operand must be an attribute "
				   "ref or a list");

			return -1;
		}

		switch (map->src->type) {
		case VPT_TYPE_EXEC:
			cf_log_err(map->ci, "Exec values are not allowed");

			return -1;

		/*
		 *	Only =, :=, += and -= operators are supported for
		 *	cache entries.
		 */
		case VPT_TYPE_LITERAL:
		case VPT_TYPE_XLAT:
		case VPT_TYPE_ATTR:
			switch (map->op) {
			case T_OP_SET:
			case T_OP_EQ:
			case T_OP_SUB:
			case T_OP_ADD:
				break;

			default:
				cf_log_err(map->ci, "Operator \"%s\" not "
					   "allowed for %s values",
					   fr_int2str(fr_tokens, map->op,
						      "<INVALID>"),
					   fr_int2str(vpt_types, map->src->type,
						      "<INVALID>"));
				return -1;
			}
		default:
			break;
		}
	}
	return 0;
}

/*
 *	Allow single attribute values to be retrieved from the cache.
 */
static ssize_t cache_xlat(void *instance, REQUEST *request,
			  char const *fmt, char *out, size_t freespace)
{
	rlm_cache_entry_t *c;
	rlm_cache_t *inst = instance;
	VALUE_PAIR *vp, *vps;
	pair_lists_t list;
	DICT_ATTR const *target;
	char const *p = fmt;
	int ret = 0;

	list = radius_list_name(&p, PAIR_LIST_REQUEST);

	target = dict_attrbyname(p);
	if (!target) {
		REDEBUG("Unknown attribute \"%s\"", p);
		return -1;
	}

	PTHREAD_MUTEX_LOCK(&inst->cache_mutex);
	c = cache_find(inst, request, fmt);

	if (!c) {
		RDEBUG("No cache entry for key \"%s\"", fmt);
		*out = '\0';
		goto done;
	}

	switch (list) {
	case PAIR_LIST_REQUEST:
		vps = c->packet;
		break;

	case PAIR_LIST_REPLY:
		vps = c->reply;
		break;

	case PAIR_LIST_CONTROL:
		vps = c->control;
		break;

	case PAIR_LIST_UNKNOWN:
		PTHREAD_MUTEX_UNLOCK(&inst->cache_mutex);
		REDEBUG("Unknown list qualifier in \"%s\"", fmt);
		return -1;

	default:
		PTHREAD_MUTEX_UNLOCK(&inst->cache_mutex);
		REDEBUG("Unsupported list \"%s\"",
		        fr_int2str(pair_lists, list, "¿Unknown?"));
		return -1;
	}

	vp = pairfind(vps, target->attr, target->vendor, TAG_ANY);
	if (!vp) {
		RDEBUG("No instance of this attribute has been cached");
		*out = '\0';
		goto done;
	}

	ret = vp_prints_value(out, freespace, vp, 0);
done:
	PTHREAD_MUTEX_UNLOCK(&inst->cache_mutex);

	return ret;
}

/*
 *	A mapping of configuration file names to internal variables.
 *
 *	Note that the string is dynamically allocated, so it MUST
 *	be freed.  When the configuration file parse re-reads the string,
 *	it free's the old one, and strdup's the new one, placing the pointer
 *	to the strdup'd string into 'config.string'.  This gets around
 *	buffer over-flows.
 */
static const CONF_PARSER module_config[] = {
	{ "key",  PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_cache_t, key), NULL, NULL},
	{ "ttl", PW_TYPE_INTEGER,
	  offsetof(rlm_cache_t, ttl), NULL, "500" },
	{ "max_entries", PW_TYPE_INTEGER,
	  offsetof(rlm_cache_t, max_entries), NULL, "16384" },
	{ "epoch", PW_TYPE_INTEGER,
	  offsetof(rlm_cache_t, epoch), NULL, "0" },
	{ "add_stats", PW_TYPE_BOOLEAN,
	  offsetof(rlm_cache_t, stats), NULL, "no" },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};


/*
 *	Only free memory we allocated.  The strings allocated via
 *	cf_section_parse() do not need to be freed.
 */
static int mod_detach(void *instance)
{
	rlm_cache_t *inst = instance;

	talloc_free(inst->maps);

	fr_heap_delete(inst->heap);
	rbtree_free(inst->cache);

#ifdef HAVE_PTHREAD_H
	pthread_mutex_destroy(&inst->cache_mutex);
#endif
	return 0;
}


/*
 *	Instantiate the module.
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_cache_t *inst = instance;

	inst->cs = conf;

	inst->xlat_name = cf_section_name2(conf);
	if (!inst->xlat_name) {
		inst->xlat_name = cf_section_name1(conf);
	}

	/*
	 *	Register the cache xlat function
	 */
	xlat_register(inst->xlat_name, cache_xlat, NULL, inst);

	rad_assert(inst->key && *inst->key);

	if (inst->ttl == 0) {
		cf_log_err_cs(conf, "Must set 'ttl' to non-zero");
		return -1;
	}

	if (inst->epoch != 0) {
		cf_log_err_cs(conf, "Must not set 'epoch' in the configuration files");
		return -1;
	}

#ifdef HAVE_PTHREAD_H
	if (pthread_mutex_init(&inst->cache_mutex, NULL) < 0) {
		EDEBUG("Failed initializing mutex: %s",
		       strerror(errno));
		return -1;
	}
#endif

	/*
	 *	The cache.
	 */
	inst->cache = rbtree_create(cache_entry_cmp, cache_entry_free, 0);
	if (!inst->cache) {
		EDEBUG("Failed to create cache");
		return -1;
	}

	/*
	 *	The heap of entries to expire.
	 */
	inst->heap = fr_heap_create(cache_heap_cmp,
				    offsetof(rlm_cache_entry_t, offset));
	if (!inst->heap) {
		EDEBUG("Failed to create heap for the cache");
		return -1;
	}

	/*
	 *	Make sure the users don't screw up too badly.
	 */
	if (cache_verify(inst, &inst->maps) < 0) {
		return -1;
	}

	return 0;
}

/*
 *	Do caching checks.  Since we can update ANY VP list, we do
 *	exactly the same thing for all sections (autz / auth / etc.)
 *
 *	If you want to cache something different in different sections,
 *	configure another cache module.
 */
static rlm_rcode_t cache_it(void *instance, REQUEST *request)
{
	rlm_cache_entry_t *c;
	rlm_cache_t *inst = instance;
	VALUE_PAIR *vp;
	char buffer[1024];
	rlm_rcode_t rcode;

	if (radius_xlat(buffer, sizeof(buffer), request, inst->key, NULL, NULL) < 0) {
		return RLM_MODULE_FAIL;
	}

	PTHREAD_MUTEX_LOCK(&inst->cache_mutex);
	c = cache_find(inst, request, buffer);

	/*
	 *	If yes, only return whether we found a valid cache entry
	 */
	vp = pairfind(request->config_items, PW_CACHE_STATUS_ONLY, 0, TAG_ANY);
	if (vp && vp->vp_integer) {
		rcode = c ? RLM_MODULE_OK:
			    RLM_MODULE_NOTFOUND;
		goto done;
	}

	if (c) {
		cache_merge(inst, request, c);

		rcode = RLM_MODULE_OK;
		goto done;
	}

	c = cache_add(inst, request, buffer);
	if (!c) {
		rcode = RLM_MODULE_NOOP;
		goto done;
	}

	rcode = RLM_MODULE_UPDATED;

done:
	PTHREAD_MUTEX_UNLOCK(&inst->cache_mutex);
	return rcode;
}


/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_cache = {
	RLM_MODULE_INIT,
	"cache",
	0,				/* type */
	sizeof(rlm_cache_t),
	module_config,
	mod_instantiate,		/* instantiation */
	mod_detach,			/* detach */
	{
		NULL,			/* authentication */
		cache_it,		/* authorization */
		cache_it,		/* preaccounting */
		cache_it,		/* accounting */
		NULL,			/* checksimul */
		cache_it,	      	/* pre-proxy */
		cache_it,	       	/* post-proxy */
		cache_it,		/* post-auth */
	},
};
