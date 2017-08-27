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
 * @file groups.c
 * @brief LDAP module group functions.
 *
 * @author Arran Cudbard-Bell <a.cudbardb@freeradius.org>
 * @copyright 2013 Network RADIUS SARL <info@networkradius.com>
 * @copyright 2013 The FreeRADIUS Server Project.
 */
#include	<freeradius-devel/rad_assert.h>
#include	<ctype.h>

#include	"ldap.h"

/** Convert multiple group names into a DNs
 *
 * Given an array of group names, builds a filter matching all names, then retrieves all group objects
 * and stores the DN associated with each group object.
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] names to covert to DNs (NULL terminated).
 * @param[out] out Where to write the DNs. DNs must be freed with ldap_memfree(). Will be NULL terminated.
 * @param[in] outlen Size of out.
 * @return One of the RLM_MODULE_* values.
 */
static rlm_rcode_t rlm_ldap_group_name2dn(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
					  char **names, char **out, size_t outlen)
{
	rlm_rcode_t rcode = RLM_MODULE_OK;
	ldap_rcode_t status;
	int ldap_errno;

	unsigned int name_cnt = 0;
	unsigned int entry_cnt;
	char const *attrs[] = { NULL };

	LDAPMessage *result = NULL, *entry;

	char **name = names;
	char **dn = out;
	char buffer[LDAP_MAX_GROUP_NAME_LEN + 1];

	char *filter;

	*dn = NULL;

	if (!*names) {
		return RLM_MODULE_OK;
	}

	if (!inst->groupobj_name_attr) {
		REDEBUG("Told to convert group names to DNs but missing 'group.name_attribute' directive");

		return RLM_MODULE_INVALID;
	}

	RDEBUG("Converting group name(s) to group DN(s)");

	/*
	 *	It'll probably only save a few ms in network latency, but it means we can send a query
	 *	for the entire group list at once.
	 */
	filter = talloc_asprintf(request, "%s%s%s",
				 inst->groupobj_filter ? "(&" : "",
				 inst->groupobj_filter ? inst->groupobj_filter : "",
				 names[0] && names[1] ? "(|" : "");
	while (*name) {
		rlm_ldap_escape_func(request, buffer, sizeof(buffer), *name++, NULL);
		filter = talloc_asprintf_append_buffer(filter, "(%s=%s)", inst->groupobj_name_attr, buffer);

		name_cnt++;
	}
	filter = talloc_asprintf_append_buffer(filter, "%s%s",
					       inst->groupobj_filter ? ")" : "",
					       names[0] && names[1] ? ")" : "");

	status = rlm_ldap_search(inst, request, pconn, inst->groupobj_base_dn, inst->groupobj_scope,
				 filter, attrs, &result);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_NO_RESULT:
			RDEBUG("Tried to resolve group name(s) to DNs but got no results");
			goto finish;
		default:
			rcode = RLM_MODULE_FAIL;
			goto finish;
	}

	entry_cnt = ldap_count_entries((*pconn)->handle, result);
	if (entry_cnt > name_cnt) {
		REDEBUG("Number of DNs exceeds number of names, group and/or dn should be more restrictive");
		rcode = RLM_MODULE_INVALID;

		goto finish;
	}

	if (entry_cnt > (outlen - 1)) {
		REDEBUG("Number of DNs exceeds limit (%zu)", outlen - 1);
		rcode = RLM_MODULE_INVALID;

		goto finish;
	}

	if (entry_cnt < name_cnt) {
		RWDEBUG("Got partial mapping of group names (%i) to DNs (%i), membership information may be incomplete",
			name_cnt, entry_cnt);
	}

	entry = ldap_first_entry((*pconn)->handle, result);
	if (!entry) {
		ldap_get_option((*pconn)->handle, LDAP_OPT_RESULT_CODE, &ldap_errno);
		REDEBUG("Failed retrieving entry: %s", ldap_err2string(ldap_errno));

		rcode = RLM_MODULE_FAIL;
		goto finish;
	}

	do {
		*dn = ldap_get_dn((*pconn)->handle, entry);
		RDEBUG("Got group DN \"%s\"", *dn);
		dn++;
	} while((entry = ldap_next_entry((*pconn)->handle, entry)));

	*dn = NULL;

	finish:
	talloc_free(filter);
	if (result) {
		ldap_msgfree(result);
	}

	/*
	 *	Be nice and cleanup the output array if we error out.
	 */
	if (rcode != RLM_MODULE_OK) {
		dn = out;
		while(*dn) ldap_memfree(*dn++);
		*dn = NULL;
	}

	return rcode;
}

/** Convert a single group name into a DN
 *
 * Unlike the inverse conversion of a name to a DN, most LDAP directories don't allow filtering by DN,
 * so we need to search for each DN individually.
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] dn to resolve.
 * @param[out] out Where to write group name (must be freed with talloc_free).
 * @return One of the RLM_MODULE_* values.
 */
static rlm_rcode_t rlm_ldap_group_dn2name(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
					  char const *dn, char **out)
{
	rlm_rcode_t rcode = RLM_MODULE_OK;
	ldap_rcode_t status;
	int ldap_errno;

	char **vals = NULL;
	char const *attrs[] = { inst->groupobj_name_attr, NULL };
	LDAPMessage *result = NULL, *entry;

	*out = NULL;

	if (!inst->groupobj_name_attr) {
		REDEBUG("Told to convert group DN to name but missing 'group.name_attribute' directive");

		return RLM_MODULE_INVALID;
	}

	RDEBUG("Converting group DN to group Name");

	status = rlm_ldap_search(inst, request, pconn, dn, LDAP_SCOPE_BASE, NULL, attrs,
				 &result);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_NO_RESULT:
			REDEBUG("DN \"%s\" did not resolve to an object", dn);

			return RLM_MODULE_INVALID;
		default:
			return RLM_MODULE_FAIL;
	}

	entry = ldap_first_entry((*pconn)->handle, result);
	if (!entry) {
		ldap_get_option((*pconn)->handle, LDAP_OPT_RESULT_CODE, &ldap_errno);
		REDEBUG("Failed retrieving entry: %s", ldap_err2string(ldap_errno));

		rcode = RLM_MODULE_INVALID;
		goto finish;
	}

	vals = ldap_get_values((*pconn)->handle, entry, inst->groupobj_name_attr);
	if (!vals) {
		REDEBUG("No %s attributes found in object", inst->groupobj_name_attr);

		rcode = RLM_MODULE_INVALID;

		goto finish;
	}

	RDEBUG("Group name is \"%s\"", vals[0]);

	*out = talloc_strdup(request, vals[0]);

	finish:
	if (result) {
		ldap_msgfree(result);
	}

	if (vals) {
		ldap_value_free(vals);
	}

	return rcode;
}

/** Convert group membership information into attributes
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] entry retrieved by rlm_ldap_find_user or rlm_ldap_search.
 * @param[in] attr membership attribute to look for in the entry.
 * @return One of the RLM_MODULE_* values.
 */
rlm_rcode_t rlm_ldap_cacheable_userobj(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
				       LDAPMessage *entry, char const *attr)
{
	rlm_rcode_t rcode = RLM_MODULE_OK;

	char **vals;

	char *group_name[LDAP_MAX_CACHEABLE + 1];
	char **name_p = group_name;

	char *group_dn[LDAP_MAX_CACHEABLE + 1];
	char **dn_p;

	char *name;

	int is_dn, i;

	rad_assert(entry);
	rad_assert(attr);

	/*
	 *	Parse the membership information we got in the initial user query.
	 */
	vals = ldap_get_values((*pconn)->handle, entry, attr);
	if (!vals) {
		RDEBUG2("No cacheable group memberships found in user object");

		return RLM_MODULE_OK;
	}

	for (i = 0; (vals[i] != NULL) && (i < LDAP_MAX_CACHEABLE); i++) {
		is_dn = rlm_ldap_is_dn(vals[i]);

		if (inst->cacheable_group_dn) {
			/*
			 *	The easy case, were caching DNs and we got a DN.
			 */
			if (is_dn) {
				pairmake(request, &request->config_items, inst->group_da->name, vals[i], T_OP_ADD);
				RDEBUG("Added %s with value \"%s\" to control list", inst->group_da->name, vals[i]);

			/*
			 *	We were told to cache DNs but we got a name, we now need to resolve
			 *	this to a DN. Store all the group names in an array so we can do one query.
			 */
			} else {
				*name_p++ = vals[i];
			}
		}

		if (inst->cacheable_group_name) {
			/*
			 *	The easy case, were caching names and we got a name.
			 */
			if (!is_dn) {
				pairmake(request, &request->config_items, inst->group_da->name, vals[i], T_OP_ADD);
				RDEBUG("Added %s with value \"%s\" to control list", inst->group_da->name, vals[i]);
			/*
			 *	We were told to cache names but we got a DN, we now need to resolve
			 *	this to a name.
			 *	Only Active Directory supports filtering on DN, so we have to search
			 *	for each individual group.
			 */
			} else {
				rcode = rlm_ldap_group_dn2name(inst, request, pconn, vals[i], &name);
				if (rcode != RLM_MODULE_OK) {
					ldap_value_free(vals);

					return rcode;
				}

				pairmake(request, &request->config_items, inst->group_da->name, name, T_OP_ADD);
				RDEBUG("Added %s with value \"%s\" to control list", inst->group_da->name, name);
				talloc_free(name);
			}
		}
	}
	*name_p = NULL;

	rcode = rlm_ldap_group_name2dn(inst, request, pconn, group_name, group_dn, sizeof(group_dn));

	ldap_value_free(vals);

	if (rcode != RLM_MODULE_OK) {
		return rcode;
	}

	dn_p = group_dn;
	while(*dn_p) {
		pairmake(request, &request->config_items, inst->group_da->name, *dn_p, T_OP_ADD);
		RDEBUG("Added %s with value \"%s\" to control list", inst->group_da->name, *dn_p);
		ldap_memfree(*dn_p);

		dn_p++;
	}

	return rcode;
}

/** Convert group membership information into attributes
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @return One of the RLM_MODULE_* values.
 */
rlm_rcode_t rlm_ldap_cacheable_groupobj(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn)
{
	rlm_rcode_t rcode = RLM_MODULE_OK;
	ldap_rcode_t status;
	int ldap_errno;

	char **vals;

	LDAPMessage *result = NULL;
	LDAPMessage *entry;

	char base_dn[LDAP_MAX_DN_STR_LEN];

	char const *filters[] = { inst->groupobj_filter, inst->groupobj_membership_filter };
	char filter[LDAP_MAX_FILTER_STR_LEN + 1];

	char const *attrs[] = { inst->groupobj_name_attr, NULL };

	char *dn;

	rad_assert(inst->groupobj_base_dn);

	if (!inst->groupobj_membership_filter) {
		RDEBUG2("Skipping caching group objects as directive 'group.membership_filter' is not set");

		return RLM_MODULE_OK;
	}

	if (rlm_ldap_xlat_filter(request,
				 filters, sizeof(filters) / sizeof(*filters),
				 filter, sizeof(filter)) < 0) {
		return RLM_MODULE_INVALID;
	}

	if (radius_xlat(base_dn, sizeof(base_dn), request, inst->groupobj_base_dn, rlm_ldap_escape_func, NULL) < 0) {
		REDEBUG("Failed creating base_dn");

		return RLM_MODULE_INVALID;
	}

	status = rlm_ldap_search(inst, request, pconn, base_dn, inst->groupobj_scope, filter, attrs, &result);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_NO_RESULT:
			RDEBUG2("No cacheable group memberships found in group objects");
		default:
			goto finish;
	}

	entry = ldap_first_entry((*pconn)->handle, result);
	if (!entry) {
		ldap_get_option((*pconn)->handle, LDAP_OPT_RESULT_CODE, &ldap_errno);
		REDEBUG("Failed retrieving entry: %s", ldap_err2string(ldap_errno));

		goto finish;
	}

	do {
		if (inst->cacheable_group_dn) {
			dn = ldap_get_dn((*pconn)->handle, entry);
			pairmake(request, &request->config_items, inst->group_da->name, dn, T_OP_ADD);
			RDEBUG("Added %s with value \"%s\" to control list", inst->group_da->name, dn);
			ldap_memfree(dn);
		}

		if (inst->cacheable_group_name) {
			vals = ldap_get_values((*pconn)->handle, entry, inst->groupobj_name_attr);
			if (!vals) {
				continue;
			}

			pairmake(request, &request->config_items, inst->group_da->name, *vals, T_OP_ADD);
			RDEBUG("Added %s with value \"%s\" to control list", inst->group_da->name, *vals);

			ldap_value_free(vals);
		}
	} while((entry = ldap_next_entry((*pconn)->handle, entry)));

	finish:
	if (result) {
		ldap_msgfree(result);
	}

	return rcode;
}

/** Query the LDAP directory to check if a group object includes a user object as a member
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] check vp containing the group value (name or dn).
 * @return One of the RLM_MODULE_* values.
 */
rlm_rcode_t rlm_ldap_check_groupobj_dynamic(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
					    VALUE_PAIR *check)

{
	ldap_rcode_t	status;

	char		base_dn[LDAP_MAX_DN_STR_LEN + 1];
	char 		filter[LDAP_MAX_FILTER_STR_LEN + 1];
	char const	*dn = base_dn;

	char const     	*name = check->vp_strvalue;

	rad_assert(inst->groupobj_base_dn);

	RDEBUG2("Checking for user in group objects");

	if (rlm_ldap_is_dn(name)) {
		char const *filters[] = { inst->groupobj_filter, inst->groupobj_membership_filter };

		if (rlm_ldap_xlat_filter(request,
					 filters, sizeof(filters) / sizeof(*filters),
					 filter, sizeof(filter)) < 0) {
			return RLM_MODULE_INVALID;
		}

		dn = name;
	} else {
		char name_filter[LDAP_MAX_FILTER_STR_LEN];
		char const *filters[] = { name_filter, inst->groupobj_filter, inst->groupobj_membership_filter };

		if (!inst->groupobj_name_attr) {
			REDEBUG("Told to search for group by name, but missing 'group.name_attribute' "
				"directive");

			return RLM_MODULE_INVALID;
		}

		snprintf(name_filter, sizeof(name_filter), "(%s=%s)", inst->groupobj_name_attr, name);
		if (rlm_ldap_xlat_filter(request,
					 filters, sizeof(filters) / sizeof(*filters),
					 filter, sizeof(filter)) < 0) {
			return RLM_MODULE_INVALID;
		}

		/*
		 *	rlm_ldap_find_user does this, too.  Oh well.
		 */
		if (radius_xlat(base_dn, sizeof(base_dn), request, inst->groupobj_base_dn,
				rlm_ldap_escape_func, NULL) < 0) {
			REDEBUG("Failed creating base_dn");

			return RLM_MODULE_INVALID;
		}
	}

	status = rlm_ldap_search(inst, request, pconn, dn, inst->groupobj_scope, filter, NULL, NULL);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			RDEBUG("User found in group object");

			break;
		case LDAP_PROC_NO_RESULT:
			RDEBUG("Search returned not found");

			return RLM_MODULE_NOTFOUND;
		default:
			return RLM_MODULE_FAIL;
	}

	return RLM_MODULE_OK;
}

/** Query the LDAP directory to check if a user object is a member of a group
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] dn of user object.
 * @param[in] check vp containing the group value (name or dn).
 * @return One of the RLM_MODULE_* values.
 */
rlm_rcode_t rlm_ldap_check_userobj_dynamic(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
					   char const *dn, VALUE_PAIR *check)
{
	rlm_rcode_t	rcode = RLM_MODULE_NOTFOUND, ret;
	ldap_rcode_t	status;
	int		name_is_dn = false, value_is_dn = false;

	LDAPMessage     *result = NULL;
	LDAPMessage     *entry = NULL;
	char		**vals = NULL;

	char const     	*name = check->vp_strvalue;

	char const	*attrs[] = { inst->userobj_membership_attr, NULL };
	int		i, count, ldap_errno;

	RDEBUG2("Checking user object membership (%s) attributes", inst->userobj_membership_attr);

	status = rlm_ldap_search(inst, request, pconn, dn, LDAP_SCOPE_BASE, NULL, attrs, &result);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_NO_RESULT:
			RDEBUG("Can't check membership attributes, user object not found");

			rcode = RLM_MODULE_NOTFOUND;

			/* FALL-THROUGH */
		default:
			goto finish;
	}

	entry = ldap_first_entry((*pconn)->handle, result);
	if (!entry) {
		ldap_get_option((*pconn)->handle, LDAP_OPT_RESULT_CODE, &ldap_errno);
		REDEBUG("Failed retrieving entry: %s", ldap_err2string(ldap_errno));

		rcode = RLM_MODULE_FAIL;

		goto finish;
	}

	vals = ldap_get_values((*pconn)->handle, entry, inst->userobj_membership_attr);
	if (!vals) {
		RDEBUG("No group membership attribute(s) found in user object");

		goto finish;
	}

	/*
	 *	Loop over the list of groups the user is a member of,
	 *	looking for a match.
	 */
	name_is_dn = rlm_ldap_is_dn(name);
	count = ldap_count_values(vals);
	for (i = 0; i < count; i++) {
		value_is_dn = rlm_ldap_is_dn(vals[i]);

		RDEBUG2("Processing group membership value \"%s\"", vals[i]);

		/*
		 *	Both literal group names, do case sensitive comparison
		 */
		if (!name_is_dn && !value_is_dn) {
			if (strcmp(vals[i], name) == 0){
				RDEBUG("User found. Comparison between membership: name, check: name]");
				rcode = RLM_MODULE_OK;

				goto finish;
			}

			continue;
		}

		/*
		 *	Both DNs, do case insensitive comparison
		 */
		if (name_is_dn && value_is_dn) {
			if (strcasecmp(vals[i], name) == 0){
				RDEBUG("User found. Comparison between membership: dn, check: dn");
				rcode = RLM_MODULE_OK;

				goto finish;
			}

			continue;
		}

		/*
		 *	If the value is not a DN, and the name we were given is a dn
		 *	convert the value to a DN and do a comparison.
		 */
		if (!value_is_dn && name_is_dn) {
			char *resolved;
			int eq;

			ret = rlm_ldap_group_dn2name(inst, request, pconn, name, &resolved);
			if (ret != RLM_MODULE_OK) {
				rcode = ret;
				goto finish;
			}

			eq = strcmp(vals[i], resolved);
			talloc_free(resolved);
			if (eq == 0){
				RDEBUG("User found. Comparison between membership: name, check: name "
				       "(resolved from DN)");
				rcode = RLM_MODULE_OK;

				goto finish;
			}

			continue;
		}

		/*
		 *	We have a value which is a DN, and a check item which specifies the name of a group,
		 *	convert the value to a name so we can do a comparison.
		 */
		if (value_is_dn && !name_is_dn) {
			char *resolved;
			int eq;

			ret = rlm_ldap_group_dn2name(inst, request, pconn, vals[i], &resolved);
			if (ret != RLM_MODULE_OK) {
				rcode = ret;
				goto finish;
			}

			eq = strcmp(resolved, name);
			talloc_free(resolved);
			if (eq == 0){
				RDEBUG("User found. Comparison between membership: name (resolved from DN), "
				       "check: name");
				rcode = RLM_MODULE_OK;

				goto finish;
			}

			continue;
		}

		rad_assert(0);
	}

	finish:

	if (vals) {
		ldap_value_free(vals);
	}

	if (result) {
		ldap_msgfree(result);
	}

	return rcode;
}

/** Check group membership attributes to see if a user is a member.
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in] check vp containing the group value (name or dn).
 *
 * @return One of the RLM_MODULE_* values.
 */
rlm_rcode_t rlm_ldap_check_cached(ldap_instance_t const *inst, REQUEST *request, VALUE_PAIR *check)
{
	VALUE_PAIR	*vp;
	int		ret;
	vp_cursor_t	cursor;

	paircursor(&cursor, &request->config_items);
	vp = pairfindnext(&cursor, inst->group_da->attr, inst->group_da->vendor, TAG_ANY);
	if (!vp) {
		return RLM_MODULE_INVALID;
	}

	for (; vp; vp = pairfindnext(&cursor, inst->group_da->attr, inst->group_da->vendor, TAG_ANY)) {
		ret = radius_compare_vps(request, check, vp);
		if (ret == 0) {
			RDEBUG2("User found. Matched cached membership");
			return RLM_MODULE_OK;
		}

		if (ret < -1) {
			return RLM_MODULE_FAIL;
		}
	}

	RDEBUG2("Membership not found");
	return RLM_MODULE_NOTFOUND;
}
