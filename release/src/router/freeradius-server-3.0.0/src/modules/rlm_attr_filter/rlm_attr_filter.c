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
 * @file rlm_attr_filter.c
 * @brief Filter the contents of a list, allowing only certain attributes.
 *
 * @copyright (C) 2001,2006 The FreeRADIUS server project
 * @copyright (C) 2001 Chris Parker <cparker@starnetusa.net>
 */
RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/modules.h>
#include	<freeradius-devel/rad_assert.h>

#include	<sys/stat.h>

#include	<ctype.h>
#include	<fcntl.h>
#include	<limits.h>


/*
 *	Define a structure with the module configuration, so it can
 *	be used as the instance handle.
 */
typedef struct rlm_attr_filter {
	char		*filename;
	char		*key;
	int		relaxed;
	PAIR_LIST	*attrs;
} rlm_attr_filter_t;

static const CONF_PARSER module_config[] = {
	{ "attrsfile",     PW_TYPE_FILE_INPUT | PW_TYPE_DEPRECATED,
	  offsetof(rlm_attr_filter_t, filename), NULL, NULL},
	{ "filename",     PW_TYPE_FILE_INPUT | PW_TYPE_REQUIRED,
	  offsetof(rlm_attr_filter_t, filename), NULL, NULL},
	{ "key",     PW_TYPE_STRING_PTR,
	  offsetof(rlm_attr_filter_t, key), NULL, "%{Realm}" },
	{ "relaxed",    PW_TYPE_BOOLEAN,
	  offsetof(rlm_attr_filter_t, relaxed), NULL, "no" },
	{ NULL, -1, 0, NULL, NULL }
};

static void check_pair(VALUE_PAIR *check_item, VALUE_PAIR *reply_item,
		      int *pass, int *fail)
{
	int compare;

	if (check_item->op == T_OP_SET) return;

	compare = paircmp(check_item, reply_item);
	if (compare == 1) {
		++*(pass);
	} else {
		++*(fail);
	}

	return;
}


static int attr_filter_getfile(TALLOC_CTX *ctx, char const *filename, PAIR_LIST **pair_list)
{
	vp_cursor_t cursor;
	int rcode;
	PAIR_LIST *attrs = NULL;
	PAIR_LIST *entry;
	VALUE_PAIR *vp;

	rcode = pairlist_read(ctx, filename, &attrs, 1);
	if (rcode < 0) {
		return -1;
	}

	/*
	 * Walk through the 'attrs' file list.
	 */

	entry = attrs;
	while (entry) {
		entry->check = entry->reply;
		entry->reply = NULL;

		for (vp = paircursor(&cursor, &entry->check);
		     vp;
		     vp = pairnext(&cursor)) {
		    /*
		     * If it's NOT a vendor attribute,
		     * and it's NOT a wire protocol
		     * and we ignore Fall-Through,
		     * then bitch about it, giving a good warning message.
		     */
		     if ((vp->da->vendor == 0) &&
			 (vp->da->attr > 0xff) &&
			 (vp->da->attr > 1000)) {
			WDEBUG("[%s]:%d Check item \"%s\"\n\tfound in filter list for realm \"%s\".\n",
			       filename, entry->lineno, vp->da->name, entry->name);
		    }
		}

		entry = entry->next;
	}

	*pair_list = attrs;
	return 0;
}


/*
 *	(Re-)read the "attrs" file into memory.
 */
static int mod_instantiate(UNUSED CONF_SECTION *conf, void *instance)
{
	rlm_attr_filter_t *inst = instance;
	int rcode;

	rcode = attr_filter_getfile(inst, inst->filename, &inst->attrs);
	if (rcode != 0) {
		ERROR("Errors reading %s", inst->filename);

		return -1;
	}

	return 0;
}


/*
 *	Common attr_filter checks
 */
static rlm_rcode_t attr_filter_common(void *instance, REQUEST *request, RADIUS_PACKET *packet)
{
	rlm_attr_filter_t *inst = instance;
	VALUE_PAIR	*vp;
	vp_cursor_t	input, check, out;
	VALUE_PAIR	*input_item, *check_item, *output;
	PAIR_LIST	*pl;
	int		found = 0;
	int		pass, fail = 0;
	char const	*keyname = NULL;
	char		buffer[256];

	if (!packet) return RLM_MODULE_NOOP;

	if (!inst->key) {
		VALUE_PAIR	*namepair;

		namepair = pairfind(request->packet->vps, PW_REALM, 0, TAG_ANY);
		if (!namepair) {
			return (RLM_MODULE_NOOP);
		}
		keyname = namepair->vp_strvalue;
	} else {
		int len;

		len = radius_xlat(buffer, sizeof(buffer), request, inst->key, NULL, NULL);
		if (len < 0) {
			return RLM_MODULE_FAIL;
		}
		if (len == 0) {
			return RLM_MODULE_NOOP;
		}
		keyname = buffer;
	}

	/*
	 *	Head of the output list
	 */
	output = NULL;
	paircursor(&out, &output);

	/*
	 *      Find the attr_filter profile entry for the entry.
	 */
	for (pl = inst->attrs; pl; pl = pl->next) {
		int fall_through = 0;
		int relax_filter = inst->relaxed;

		/*
		 *  If the current entry is NOT a default,
		 *  AND the realm does NOT match the current entry,
		 *  then skip to the next entry.
		 */
		if ((strcmp(pl->name, "DEFAULT") != 0) &&
		    (strcmp(keyname, pl->name) != 0))  {
		    continue;
		}

		RDEBUG2("Matched entry %s at line %d", pl->name, pl->lineno);
		found = 1;

		for (check_item = paircursor(&check, &pl->check);
		     check_item;
		     check_item = pairnext(&check)) {
			if (!check_item->da->vendor &&
			    (check_item->da->attr == PW_FALL_THROUGH) &&
				(check_item->vp_integer == 1)) {
				fall_through = 1;
				continue;
			}
			else if (!check_item->da->vendor &&
				 check_item->da->attr == PW_RELAX_FILTER) {
				relax_filter = check_item->vp_integer;
				continue;
			}

			/*
			 *    If it is a SET operator, add the attribute to
			 *    the output list without checking it.
			 */
			if (check_item->op == T_OP_SET ) {
				vp = paircopyvp(packet, check_item);
				if (!vp) {
					goto error;
				}
				radius_xlat_do(request, vp);
				pairinsert(&out, vp);
			}
		}

		/*
		 *	Iterate through the input items, comparing
		 *	each item to every rule, then moving it to the
		 *	output list only if it matches all rules
		 *	for that attribute.  IE, Idle-Timeout is moved
		 *	only if it matches all rules that describe an
		 *	Idle-Timeout.
		 */
		for (input_item = paircursor(&input, &packet->vps);
		     input_item;
		     input_item = pairnext(&input)) {
			/* reset the pass,fail vars for each reply item */
			pass = fail = 0;

			/*
			 *	reset the check_item pointer to
			 *	beginning of the list
			 */
			for (check_item = pairfirst(&check);
			     check_item;
			     check_item = pairnext(&check)) {
				/*
				 *	Vendor-Specific is special, and
				 *	matches any VSA if the comparison
				 *	is always true.
				 */
				if ((check_item->da->attr == PW_VENDOR_SPECIFIC) && (input_item->da->vendor != 0) &&
				    (check_item->op == T_OP_CMP_TRUE)) {
					pass++;
					continue;
				}

				if (input_item->da->attr == check_item->da->attr) {
					check_pair(check_item, input_item, &pass, &fail);
				}
			}

			/*
			 *  Only move attribute if it passed all rules,
			 *  or if the config says we should copy unmatched
			 *  attributes ('relaxed' mode).
			 */
			if (fail == 0 && (pass > 0 || relax_filter)) {
				if (!pass) {
					RDEBUG3("Attribute (%s) allowed by relaxed mode", input_item->da->name);
				}
				vp = paircopyvp(packet, input_item);
				if (!vp) {
					goto error;
				}
				pairinsert(&out, vp);
			}
		}

		/* If we shouldn't fall through, break */
		if (!fall_through) {
			break;
		}
	}

	/*
	 *	No entry matched.  We didn't do anything.
	 */
	if (!found) {
		rad_assert(!output);
		return RLM_MODULE_NOOP;
	}

	/*
	 *	Replace the existing request list with our filtered one
	 */
	pairfree(&packet->vps);
	packet->vps = output;

	if (request->packet->code == PW_AUTHENTICATION_REQUEST) {
		request->username = pairfind(request->packet->vps, PW_STRIPPED_USER_NAME, 0, TAG_ANY);
		if (!request->username)
			request->username = pairfind(request->packet->vps, PW_USER_NAME, 0, TAG_ANY);
		request->password = pairfind(request->packet->vps, PW_USER_PASSWORD, 0, TAG_ANY);
	}

	return RLM_MODULE_UPDATED;

	error:
	pairfree(&output);
	return RLM_MODULE_FAIL;
}

#define RLM_AF_FUNC(_x, _y) static rlm_rcode_t mod_##_x(void *instance, REQUEST *request) \
			{ \
				return attr_filter_common(instance, request, request->_y); \
			}

RLM_AF_FUNC(authorize, packet)
RLM_AF_FUNC(post_auth, reply)

RLM_AF_FUNC(preacct, packet)
RLM_AF_FUNC(accounting, reply)

#ifdef WITH_PROXY
RLM_AF_FUNC(pre_proxy, proxy)
RLM_AF_FUNC(post_proxy, proxy_reply)
#endif

#ifdef WITH_COA
RLM_AF_FUNC(recv_coa, packet)
RLM_AF_FUNC(send_coa, reply)
#endif

/* globally exported name */
module_t rlm_attr_filter = {
	RLM_MODULE_INIT,
	"attr_filter",
	RLM_TYPE_CHECK_CONFIG_SAFE | RLM_TYPE_HUP_SAFE,   	/* type */
	sizeof(rlm_attr_filter_t),
	module_config,
	mod_instantiate,	/* instantiation */
	NULL,			/* detach */
	{
		NULL,			/* authentication */
		mod_authorize,	/* authorization */
		mod_preacct,	/* pre-acct */
		mod_accounting,	/* accounting */
		NULL,			/* checksimul */
#ifdef WITH_PROXY
		mod_pre_proxy,	/* pre-proxy */
		mod_post_proxy,	/* post-proxy */
#else
		NULL, NULL,
#endif
		mod_post_auth	/* post-auth */
#ifdef WITH_COA
		,
		mod_recv_coa,
		mod_send_coa
#endif
	},
};

