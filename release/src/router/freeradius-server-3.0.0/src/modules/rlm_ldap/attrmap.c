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
 * @file ldap.c
 * @brief Functions for mapping between LDAP and FreeRADIUS attributes.
 *
 * @author Arran Cudbard-Bell <a.cudbardb@freeradius.org>
 * @copyright 2013 Network RADIUS SARL <info@networkradius.com>
 * @copyright 2013 The FreeRADIUS Server Project.
 */

#include <freeradius-devel/rad_assert.h>
#include "ldap.h"

static int rlm_ldap_map_getvalue(VALUE_PAIR **out, REQUEST *request, value_pair_map_t const *map, void *ctx)
{
	rlm_ldap_result_t *self = ctx;
	VALUE_PAIR *head = NULL, *vp;
	vp_cursor_t cursor;
	int i;

	paircursor(&cursor, &head);

	/*
	 *	Iterate over all the retrieved values,
	 *	don't try and be clever about changing operators
	 *	just use whatever was set in the attribute map.
	 */
	for (i = 0; i < self->count; i++) {
		vp = pairalloc(request, map->dst->da);
		rad_assert(vp);

		if (!pairparsevalue(vp, self->values[i])) {
			RDEBUG("Failed parsing value for \"%s\"", map->dst->da->name);

			talloc_free(vp);
			continue;
		}

		vp->op = map->op;
		pairinsert(&cursor, vp);
	}

	*out = head;

	return 0;
}

int rlm_ldap_map_verify(ldap_instance_t *inst, value_pair_map_t **head)
{
	value_pair_map_t *map;

	if (radius_attrmap(cf_section_sub_find(inst->cs, "update"),
			   head, PAIR_LIST_REPLY,
			   PAIR_LIST_REQUEST, LDAP_MAX_ATTRMAP) < 0) {
		return -1;
	}
	/*
	 *	Attrmap only performs some basic validation checks, we need
	 *	to do rlm_ldap specific checks here.
	 */
	for (map = *head; map != NULL; map = map->next) {
		if (map->dst->type != VPT_TYPE_ATTR) {
			cf_log_err(map->ci, "Left operand must be an attribute ref");

			return -1;
		}

		if (map->src->type == VPT_TYPE_LIST) {
			cf_log_err(map->ci, "Right operand must not be a list");

			return -1;
		}

		if (map->src->type == VPT_TYPE_EXEC) {
			cf_log_err(map->ci, "Exec values are not allowed");

			return -1;
		}

		/*
		 *	Be smart about whether we warn the user about missing passwords.
		 *	If there are no password attributes in the mapping, then the user's either an idiot
		 *	and has no idea what they're doing, or they're authenticating the user using a different
		 *	method.
		 */
		if (!inst->expect_password && map->dst->da && (map->dst->type == VPT_TYPE_ATTR)) {
			switch (map->dst->da->attr) {
			case PW_CLEARTEXT_PASSWORD:
			case PW_NT_PASSWORD:
			case PW_USER_PASSWORD:
			case PW_PASSWORD_WITH_HEADER:
			case PW_CRYPT_PASSWORD:
				/*
				 *	Because you just know someone is going to map NT-Password to the
				 *	request list, and then complain it's not working...
				 */
				if (map->dst->list != PAIR_LIST_CONTROL) {
					LDAP_DBGW("Mapping LDAP (%s) attribute to password \"reference\" attribute "
						  "(%s) in %s list. This is probably *NOT* the correct list, "
						  "you should prepend \"control:\" to \"reference\" attribute "
						  "(control:%s)",
						  map->src->name, map->dst->da->name,
						  fr_int2str(pair_lists, map->dst->list, "<invalid>"),
						  map->dst->da->name);
				}

				inst->expect_password = true;
			default:
				break;
			}
		}

		switch (map->src->type) {
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
				cf_log_err(map->ci, "Operator \"%s\" not allowed for %s values",
					   fr_int2str(fr_tokens, map->op, "<INVALID>"),
					   fr_int2str(vpt_types, map->src->type, "<INVALID>"));
				return -1;
			}
		default:
			break;
		}
	}
	return 0;
}

/** Free attribute map values
 *
 */
void rlm_ldap_map_xlat_free(rlm_ldap_map_xlat_t const *expanded)
{
	value_pair_map_t const *map;
	unsigned int total = 0;

	char const *name;

	for (map = expanded->maps; map != NULL; map = map->next) {
		name = expanded->attrs[total++];
		if (!name) return;

		switch (map->src->type) {
		case VPT_TYPE_XLAT:
		case VPT_TYPE_ATTR:
			rad_const_free(name);
			break;
		default:
			break;
		}
	}
}

/** Expand values in an attribute map where needed
 *
 */
int rlm_ldap_map_xlat(REQUEST *request, value_pair_map_t const *maps, rlm_ldap_map_xlat_t *expanded)
{
	value_pair_map_t const *map;
	unsigned int total = 0;
	size_t len;

	VALUE_PAIR *found, **from = NULL;
	REQUEST *context;

	for (map = maps; map != NULL; map = map->next) {
		switch (map->src->type) {
		case VPT_TYPE_XLAT:
			{
				char *exp = NULL;

				len = radius_xlat(exp, 0, request, map->src->name, NULL, NULL);
				if (len <= 0) {
					RDEBUG("Expansion of LDAP attribute \"%s\" failed", map->src->name);

					goto error;
				}

				expanded->attrs[total++] = exp;
				break;
			}

		case VPT_TYPE_ATTR:
			context = request;

			if (radius_request(&context, map->src->request) == 0) {
				from = radius_list(context, map->src->list);
			}
			if (!from) continue;

			found = pairfind(*from, map->src->da->attr, map->src->da->vendor, TAG_ANY);
			if (!found) continue;

			expanded->attrs[total++] = talloc_strdup(request, found->vp_strvalue);
			break;

		case VPT_TYPE_LITERAL:
			expanded->attrs[total++] = map->src->name;
			break;
		default:
			rad_assert(0);
		error:
			expanded->attrs[total] = NULL;

			rlm_ldap_map_xlat_free(expanded);

			return -1;
		}

	}

	rad_assert(total < LDAP_MAX_ATTRMAP);

	expanded->attrs[total] = NULL;
	expanded->count = total;
	expanded->maps = maps;

	return 0;
}


/** Convert attribute map into valuepairs
 *
 * Use the attribute map built earlier to convert LDAP values into valuepairs and insert them into whichever
 * list they need to go into.
 *
 * This is *NOT* atomic, but there's no condition for which we should error out...
 */
void rlm_ldap_map_do(UNUSED const ldap_instance_t *inst, REQUEST *request, LDAP *handle,
		     rlm_ldap_map_xlat_t const *expanded, LDAPMessage *entry)
{
	value_pair_map_t const 	*map;
	unsigned int		total = 0;

	rlm_ldap_result_t	result;
	char const		*name;


	for (map = expanded->maps; map != NULL; map = map->next) {
		name = expanded->attrs[total++];

		result.values = ldap_get_values(handle, entry, name);
		if (!result.values) {
			RDEBUG2("Attribute \"%s\" not found in LDAP object", name);

			goto next;
		}

		/*
		 *	Find out how many values there are for the
		 *	attribute and extract all of them.
		 */
		result.count = ldap_count_values(result.values);

		/*
		 *	If something bad happened, just skip, this is probably
		 *	a case of the dst being incorrect for the current
		 *	request context
		 */
		if (radius_map2request(request, map, name, rlm_ldap_map_getvalue, &result) == -1) {
			return;	/* Fail */
		}

		next:

		ldap_value_free(result.values);
	}

	/*
	 *	Retrieve any valuepair attributes from the result, these are generic values specifying
	 *	a radius list, operator and value.
	 */
	if (inst->valuepair_attr) {
		char 		**values;
		int		count, i;

	 	values = ldap_get_values(handle, entry, inst->valuepair_attr);
		count = ldap_count_values(values);

		for (i = 0; i < count; i++) {
			if (radius_str2vp(request, values[i], REQUEST_CURRENT, PAIR_LIST_REPLY) < 0) {
				RWDEBUG("Failed parsing '%s' value \"%s\" as valuepair, skipping...",
					inst->valuepair_attr, values[i]);
			}
		}

		ldap_value_free(values);
	}
}

/** Search for and apply an LDAP profile
 *
 * LDAP profiles are mapped using the same attribute map as user objects, they're used to add common sets of attributes
 * to the request.
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] dn of profile object to apply.
 * @param[in] expanded Structure containing a list of xlat expanded attribute names and mapping information.
 * @return One of the RLM_MODULE_* values.
 */
rlm_rcode_t rlm_ldap_map_profile(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
			    	 char const *dn, rlm_ldap_map_xlat_t const *expanded)
{
	rlm_rcode_t	rcode = RLM_MODULE_OK;
	ldap_rcode_t	status;
	LDAPMessage	*result = NULL, *entry = NULL;
	int		ldap_errno;
	LDAP		*handle = (*pconn)->handle;
	char		filter[LDAP_MAX_FILTER_STR_LEN];

	rad_assert(inst->profile_filter); 	/* We always have a default filter set */

	if (!dn || !*dn) {
		return RLM_MODULE_OK;
	}

	if (radius_xlat(filter, sizeof(filter), request, inst->profile_filter, rlm_ldap_escape_func, NULL) < 0) {
		REDEBUG("Failed creating profile filter");

		return RLM_MODULE_INVALID;
	}

	status = rlm_ldap_search(inst, request, pconn, dn, LDAP_SCOPE_BASE, filter, expanded->attrs, &result);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_NO_RESULT:
			RDEBUG("Profile object \"%s\" not found", dn);
			return RLM_MODULE_NOTFOUND;
		default:
			return RLM_MODULE_FAIL;
	}

	rad_assert(*pconn);
	rad_assert(result);

	entry = ldap_first_entry(handle, result);
	if (!entry) {
		ldap_get_option(handle, LDAP_OPT_RESULT_CODE, &ldap_errno);
		REDEBUG("Failed retrieving entry: %s", ldap_err2string(ldap_errno));

		rcode = RLM_MODULE_NOTFOUND;

	 	goto free_result;
	}

	rlm_ldap_map_do(inst, request, handle, expanded, entry);

free_result:
	ldap_msgfree(result);

	return rcode;
}

