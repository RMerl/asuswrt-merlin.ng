/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
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

/*
 * $Id$
 *
 * @brief Valuepair functions that are radiusd-specific and as such do not
 * 	  belong in the library.
 * @file main/valuepair.c
 *
 * @ingroup AVP
 *
 * @copyright 2000,2006  The FreeRADIUS server project
 * @copyright 2000  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include <ctype.h>

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

const FR_NAME_NUMBER vpt_types[] = {
	{"unknown",		VPT_TYPE_UNKNOWN },
	{"literal",		VPT_TYPE_LITERAL },
	{"expanded",		VPT_TYPE_XLAT },
	{"attribute ref",	VPT_TYPE_ATTR },
	{"list",		VPT_TYPE_LIST },
	{"exec",		VPT_TYPE_EXEC },
	{"value-pair-data",	VPT_TYPE_DATA }
};

struct cmp {
	DICT_ATTR const *da;
	DICT_ATTR const *from;
	bool	first_only;
	void *instance; /* module instance */
	RAD_COMPARE_FUNC compare;
	struct cmp *next;
};
static struct cmp *cmp;

/** Compares check and vp by value.
 *
 * Does not call any per-attribute comparison function, but does honour
 * check.operator. Basically does "vp.value check.op check.value".
 *
 * @param request Current request.
 * @param check rvalue, and operator.
 * @param vp lvalue.
 * @return 0 if check and vp are equal, -1 if vp value is less than check value, 1 is vp value is more than check
 *	value, -2 on error.
 */
int radius_compare_vps(REQUEST *request, VALUE_PAIR *check, VALUE_PAIR *vp)
{
	int ret = 0;

	/*
	 *      Check for =* and !* and return appropriately
	 */
	if (check->op == T_OP_CMP_TRUE)  return 0;
	if (check->op == T_OP_CMP_FALSE) return 1;

#ifdef HAVE_REGEX_H
	if (check->op == T_OP_REG_EQ) {
		int compare;
		regex_t reg;
		char value[1024];
		regmatch_t rxmatch[REQUEST_MAX_REGEX + 1];

		vp_prints_value(value, sizeof(value), vp, -1);

		/*
		 *	Include substring matches.
		 */
		compare = regcomp(&reg, check->vp_strvalue, REG_EXTENDED);
		if (compare != 0) {
			char buffer[256];
			regerror(compare, &reg, buffer, sizeof(buffer));

			RDEBUG("Invalid regular expression %s: %s", check->vp_strvalue, buffer);
			return -2;
		}
		compare = regexec(&reg, value, REQUEST_MAX_REGEX + 1, rxmatch, 0);
		regfree(&reg);
		rad_regcapture(request, compare, value, rxmatch);

		ret = (compare == 0) ? 0 : -1;
		goto finish;
	}

	if (check->op == T_OP_REG_NE) {
		int compare;
		regex_t reg;
		char value[1024];
		regmatch_t rxmatch[REQUEST_MAX_REGEX + 1];

		vp_prints_value(value, sizeof(value), vp, -1);

		/*
		 *	Include substring matches.
		 */
		compare = regcomp(&reg, check->vp_strvalue, REG_EXTENDED);
		if (compare != 0) {
			char buffer[256];
			regerror(compare, &reg, buffer, sizeof(buffer));

			RDEBUG("Invalid regular expression %s: %s", check->vp_strvalue, buffer);
			return -2;
		}
		compare = regexec(&reg, value,  REQUEST_MAX_REGEX + 1, rxmatch, 0);
		regfree(&reg);

		ret = (compare != 0) ? 0 : -1;
	}
#endif

	/*
	 *	Attributes must be of the same type.
	 *
	 *	FIXME: deal with type mismatch properly if one side contain
	 *	ABINARY, OCTETS or STRING by converting the other side to
	 *	a string
	 *
	 */
	if (vp->da->type != check->da->type) return -1;

	/*
	 *	Tagged attributes are equal if and only if both the
	 *	tag AND value match.
	 */
	if (check->da->flags.has_tag) {
		ret = ((int) vp->tag) - ((int) check->tag);
		goto finish;
	}

	/*
	 *	Not a regular expression, compare the types.
	 */
	switch(check->da->type) {
#ifdef WITH_ASCEND_BINARY
		/*
		 *	Ascend binary attributes can be treated
		 *	as opaque objects, I guess...
		 */
		case PW_TYPE_ABINARY:
#endif
		case PW_TYPE_OCTETS:
			if (vp->length != check->length) {
				ret = 1; /* NOT equal */
				break;
			}
			ret = memcmp(vp->vp_strvalue, check->vp_strvalue,
				     vp->length);
			break;

		case PW_TYPE_STRING:
			ret = strcmp(vp->vp_strvalue,
				     check->vp_strvalue);
			break;

		case PW_TYPE_BYTE:
		case PW_TYPE_SHORT:
		case PW_TYPE_INTEGER:
			ret = vp->vp_integer - check->vp_integer;
			break;

		case PW_TYPE_INTEGER64:
			/*
			 *	Don't want integer overflow!
			 */
			if (vp->vp_integer64 < check->vp_integer64) {
				ret = -1;
			} else if (vp->vp_integer64 > check->vp_integer64) {
				ret = +1;
			} else {
				ret = 0;
			}
			break;

		case PW_TYPE_SIGNED:
			if (vp->vp_signed < check->vp_signed) {
				ret = -1;
			} else if (vp->vp_signed > check->vp_signed) {
				ret = +1;
			} else {
				ret = 0;
			}
			break;

		case PW_TYPE_DATE:
			ret = vp->vp_date - check->vp_date;
			break;

		case PW_TYPE_IPADDR:
			ret = ntohl(vp->vp_ipaddr) - ntohl(check->vp_ipaddr);
			break;

		case PW_TYPE_IPV6ADDR:
			ret = memcmp(&vp->vp_ipv6addr, &check->vp_ipv6addr,
				     sizeof(vp->vp_ipv6addr));
			break;

		case PW_TYPE_IPV6PREFIX:
			ret = memcmp(&vp->vp_ipv6prefix, &check->vp_ipv6prefix,
				     sizeof(vp->vp_ipv6prefix));
			break;

		case PW_TYPE_IFID:
			ret = memcmp(&vp->vp_ifid, &check->vp_ifid,
				     sizeof(vp->vp_ifid));
			break;

		default:
			break;
	}

	finish:
	if (ret > 0) {
		return 1;
	}
	if (ret < 0) {
		return -1;
	}
	return 0;
}


/** Compare check and vp. May call the attribute comparison function.
 *
 * Unlike radius_compare_vps() this function will call any attribute-specific
 * comparison functions registered.
 *
 * @param request Current request.
 * @param req list pairs.
 * @param check item to compare.
 * @param check_pairs list.
 * @param reply_pairs list.
 * @return 0 if check and vp are equal, -1 if vp value is less than check value, 1 is vp value is more than check
 *	value.
 */
int radius_callback_compare(REQUEST *request, VALUE_PAIR *req,
			    VALUE_PAIR *check, VALUE_PAIR *check_pairs,
			    VALUE_PAIR **reply_pairs)
{
	struct cmp *c;

	/*
	 *      Check for =* and !* and return appropriately
	 */
	if (check->op == T_OP_CMP_TRUE)  return 0;
	if (check->op == T_OP_CMP_FALSE) return 1;

	/*
	 *	See if there is a special compare function.
	 *
	 *	FIXME: use new RB-Tree code.
	 */
	for (c = cmp; c; c = c->next) {
		if (c->da == check->da) {
			return (c->compare)(c->instance, request, req, check,
				check_pairs, reply_pairs);
		}
	}

	if (!req) return -1; /* doesn't exist, don't compare it */

	return radius_compare_vps(request, check, req);
}


/** Find a comparison function for two attributes.
 *
 * @todo this should probably take DA's.
 * @param da to find comparison function for.
 * @return true if a comparison function was found, else false.
 */
int radius_find_compare(DICT_ATTR const *da)
{
	struct cmp *c;

	for (c = cmp; c; c = c->next) {
		if (c->da == da) {
			return true;
		}
	}

	return false;
}


/** See what attribute we want to compare with.
 *
 * @param da to find comparison function for.
 * @param from reference to compare with
 * @return true if the comparison callback require a matching attribue in the request, else false.
 */
static bool otherattr(DICT_ATTR const *da, DICT_ATTR const **from)
{
	struct cmp *c;

	for (c = cmp; c; c = c->next) {
		if (c->da == da) {
			*from = c->from;
			return c->first_only;
		}
	}

	*from = da;
	return false;
}

/** Register a function as compare function.
 *
 * @param da to register comparison function for.
 * @param from the attribute we want to compare with. Normally this is the same as attribute.
 *  If null call the comparison function on every attributes in the request if first_only is false
 * @param first_only will decide if we loop over the request attributes or stop on the first one
 * @param func comparison function
 * @param instance argument to comparison function
 * @return 0
 */
int paircompare_register(DICT_ATTR const *da, DICT_ATTR const *from,
			 bool first_only, RAD_COMPARE_FUNC func, void *instance)
{
	struct cmp *c;

	rad_assert(da != NULL);

	paircompare_unregister(da, func);

	c = rad_malloc(sizeof(struct cmp));

	c->compare   = func;
	c->da = da;
	c->from = from;
	c->first_only = first_only;
	c->instance  = instance;
	c->next      = cmp;
	cmp = c;

	return 0;
}

/** Unregister comparison function for an attribute
 *
 * @param da dict reference to unregister for.
 * @param func comparison function to remove.
 */
void paircompare_unregister(DICT_ATTR const *da, RAD_COMPARE_FUNC func)
{
	struct cmp *c, *last;

	last = NULL;
	for (c = cmp; c; c = c->next) {
		if (c->da == da && c->compare == func) {
			break;
		}
		last = c;
	}

	if (c == NULL) return;

	if (last != NULL) {
		last->next = c->next;
	} else {
		cmp = c->next;
	}

	free(c);
}

/** Unregister comparison function for a module
 *
 *  All paircompare() functions for this module will be unregistered.
 *
 * @param instance the module instance
 */
void paircompare_unregister_instance(void *instance)
{
	struct cmp *c, **tail;

	tail = &cmp;
	while ((c = *tail) != NULL) {
		if (c->instance == instance) {
			*tail = c->next;
			free(c);
			continue;
		}

		tail = &(c->next);
	}
}

/** Compare two pair lists except for the password information.
 *
 * For every element in "check" at least one matching copy must be present
 * in "reply".
 *
 * @param[in] request Current request.
 * @param[in] req_list request valuepairs.
 * @param[in] check Check/control valuepairs.
 * @param[in,out] rep_list Reply value pairs.
 *
 * @return 0 on match.
 */
int paircompare(REQUEST *request, VALUE_PAIR *req_list, VALUE_PAIR *check,
		VALUE_PAIR **rep_list)
{
	vp_cursor_t cursor;
	VALUE_PAIR *check_item;
	VALUE_PAIR *auth_item;
	DICT_ATTR const *from;

	int result = 0;
	int compare;
	bool first_only;

	for (check_item = paircursor(&cursor, &check);
	     check_item;
	     check_item = pairnext(&cursor)) {
		/*
		 *	If the user is setting a configuration value,
		 *	then don't bother comparing it to any attributes
		 *	sent to us by the user.  It ALWAYS matches.
		 */
		if ((check_item->op == T_OP_SET) ||
		    (check_item->op == T_OP_ADD)) {
			continue;
		}

		if (!check_item->da->vendor) switch (check_item->da->attr) {
			/*
			 *	Attributes we skip during comparison.
			 *	These are "server" check items.
			 */
			case PW_CRYPT_PASSWORD:
			case PW_AUTH_TYPE:
			case PW_AUTZ_TYPE:
			case PW_ACCT_TYPE:
			case PW_SESSION_TYPE:
			case PW_STRIP_USER_NAME:
				continue;
				break;

			/*
			 *	IF the password attribute exists, THEN
			 *	we can do comparisons against it.  If not,
			 *	then the request did NOT contain a
			 *	User-Password attribute, so we CANNOT do
			 *	comparisons against it.
			 *
			 *	This hack makes CHAP-Password work..
			 */
			case PW_USER_PASSWORD:
				if (check_item->op == T_OP_CMP_EQ) {
					WDEBUG("Found User-Password == \"...\".");
					WDEBUG("Are you sure you don't mean Cleartext-Password?");
					WDEBUG("See \"man rlm_pap\" for more information.");
				}
				if (pairfind(req_list, PW_USER_PASSWORD, 0, TAG_ANY) == NULL) {
					continue;
				}
				break;
		}

		/*
		 *	See if this item is present in the request.
		 */
		first_only = otherattr(check_item->da, &from);

		auth_item = req_list;
	try_again:
		if (!first_only) {
			while (auth_item != NULL) {
				if ((auth_item->da == from) || (!from)) {
					break;
				}
				auth_item = auth_item->next;
			}
		}

		/*
		 *	Not found, it's not a match.
		 */
		if (auth_item == NULL) {
			/*
			 *	Didn't find it.  If we were *trying*
			 *	to not find it, then we succeeded.
			 */
			if (check_item->op == T_OP_CMP_FALSE) {
				continue;
			} else {
				return -1;
			}
		}

		/*
		 *	Else we found it, but we were trying to not
		 *	find it, so we failed.
		 */
		if (check_item->op == T_OP_CMP_FALSE) {
			return -1;
		}


		/*
		 *	We've got to xlat the string before doing
		 *	the comparison.
		 */
		radius_xlat_do(request, check_item);

		/*
		 *	OK it is present now compare them.
		 */
		compare = radius_callback_compare(request, auth_item,
						  check_item, check, rep_list);

		switch (check_item->op) {
			case T_OP_EQ:
			default:
				INFO("Invalid operator for item %s: "
				       "reverting to '=='", check_item->da->name);
				/* FALL-THROUGH */
			case T_OP_CMP_TRUE:
			case T_OP_CMP_FALSE:
			case T_OP_CMP_EQ:
				if (compare != 0) result = -1;
				break;

			case T_OP_NE:
				if (compare == 0) result = -1;
				break;

			case T_OP_LT:
				if (compare >= 0) result = -1;
				break;

			case T_OP_GT:
				if (compare <= 0) result = -1;
				break;

			case T_OP_LE:
				if (compare > 0) result = -1;
				break;

			case T_OP_GE:
				if (compare < 0) result = -1;
				break;

#ifdef HAVE_REGEX_H
			case T_OP_REG_EQ:
			case T_OP_REG_NE:
				if (compare != 0) result = -1;
				break;
#endif
		} /* switch over the operator of the check item */

		/*
		 *	This attribute didn't match, but maybe there's
		 *	another of the same attribute, which DOES match.
		 */
		if ((result != 0) && (!first_only)) {
			auth_item = auth_item->next;
			result = 0;
			goto try_again;
		}

	} /* for every entry in the check item list */

	return result;
}

/** Expands an attribute marked with pairmark_xlat
 *
 * Writes the new value to the vp.
 *
 * @param request Current request.
 * @param vp to expand.
 * @return 0 if successful else -1 (on xlat failure) or -2 (on parse failure).
 *	On failure pair will still no longer be marked for xlat expansion.
 */
int radius_xlat_do(REQUEST *request, VALUE_PAIR *vp)
{
	ssize_t len;

	char buffer[1024];

	if (vp->type != VT_XLAT) return 0;

	vp->type = VT_DATA;

	len = radius_xlat(buffer, sizeof(buffer), request, vp->value.xlat, NULL, NULL);


	rad_const_free(vp->value.xlat);
	vp->value.xlat = NULL;
	if (len < 0) {
		return -1;
	}

	/*
	 *	Parse the string into a new value.
	 */
	if (!pairparsevalue(vp, buffer)){
		return -2;
	}

	return 0;
}

/** Move pairs, replacing/over-writing them, and doing xlat.
 *
 * Move attributes from one list to the other if not already present.
 */
void radius_xlat_move(REQUEST *request, VALUE_PAIR **to, VALUE_PAIR **from)
{
	VALUE_PAIR **tailto, *i, *j, *next;
	VALUE_PAIR *tailfrom = NULL;
	VALUE_PAIR *found;

	/*
	 *	Point "tailto" to the end of the "to" list.
	 */
	tailto = to;
	for (i = *to; i; i = i->next) {
		tailto = &i->next;
	}

	/*
	 *	Loop over the "from" list.
	 */
	for (i = *from; i; i = next) {
		next = i->next;

		/*
		 *	Don't move 'fallthrough' over.
		 */
		if (!i->da->vendor && i->da->attr == PW_FALL_THROUGH) {
			tailfrom = i;
			continue;
		}

		/*
		 *	We've got to xlat the string before moving
		 *	it over.
		 */
		radius_xlat_do(request, i);

		found = pairfind(*to, i->da->attr, i->da->vendor, TAG_ANY);
		switch (i->op) {

			/*
			 *	If a similar attribute is found,
			 *	delete it.
			 */
			case T_OP_SUB:		/* -= */
				if (found) {
					if (!i->vp_strvalue[0] ||
				    	    (strcmp(found->vp_strvalue,
					    	    i->vp_strvalue) == 0)) {
				  		pairdelete(to, found->da->attr,
				  			found->da->vendor,
				  			found->tag);

					/*
					 *	'tailto' may have been
					 *	deleted...
					 */
					tailto = to;
					for (j = *to; j; j = j->next) {
						tailto = &j->next;
					}
				}
			}
			tailfrom = i;
			continue;
			break;

			/*
			 *	Add it, if it's not already there.
			 */
			case T_OP_EQ:		/* = */
				if (found) {
					tailfrom = i;
					continue; /* with the loop */
				}
				break;

			/*
			 *	If a similar attribute is found,
			 *	replace it with the new one.  Otherwise,
			 *	add the new one to the list.
			 */
			case T_OP_SET:		/* := */
				if (found) {
					VALUE_PAIR *vp;

					vp = found->next;
					memcpy(found, i, sizeof(*found));
					found->next = vp;
					tailfrom = i;
					continue;
				}
				break;

			/*
			 *	FIXME: Add support for <=, >=, <, >
			 *
			 *	which will mean (for integers)
			 *	'make the attribute the smaller, etc'
			 */

			/*
			 *  Add the new element to the list, even
			 *  if similar ones already exist.
			 */
			default:
			case T_OP_ADD:		/* += */
				break;
		}

		if (tailfrom) {
			tailfrom->next = next;
		} else {
			*from = next;
		}

		/*
		 *	If ALL of the 'to' attributes have been deleted,
		 *	then ensure that the 'tail' is updated to point
		 *	to the head.
		 */
		if (!*to) {
			tailto = to;
		}
		*tailto = i;
		if (i) {
			i->next = NULL;
			tailto = &i->next;
		}
	} /* loop over the 'from' list */
}

/** Create a VALUE_PAIR and add it to a list of VALUE_PAIR s
 *
 * @note This function ALWAYS returns. If we're OOM, then it causes the
 * @note server to exit, so you don't need to check the return value.
 *
 * @param[in] request Current request.
 * @param[out] vps List to add new VALUE_PAIR to, if NULL will just
 *	return VALUE_PAIR.
 * @param[in] attribute number.
 * @param[in] vendor number.
 * @return a new VLAUE_PAIR or causes server to exit on error.
 */
VALUE_PAIR *radius_paircreate(REQUEST *request, VALUE_PAIR **vps,
			      unsigned int attribute, unsigned int vendor)
{
	VALUE_PAIR *vp;

	/*
	 *	FIXME: the context should ideally be the packet...
	 */
	vp = paircreate(request, attribute, vendor);
	if (!vp) {
		ERROR("No memory!");
		rad_assert("No memory" == NULL);
		fr_exit_now(1);
	}

	if (vps) pairadd(vps, vp);

	return vp;
}

/** Print a single valuepair to stderr or error log.
 *
 * @param[in] vp list to print.
 */
void debug_pair(VALUE_PAIR *vp)
{
	if (!vp || !debug_flag || !fr_log_fp) return;

	vp_print(fr_log_fp, vp);
}

/** Print a list of valuepairs to stderr or error log.
 *
 * @param[in] vp to print.
 */
void debug_pair_list(VALUE_PAIR *vp)
{
	vp_cursor_t cursor;
	if (!vp || !debug_flag || !fr_log_fp) return;

	for (vp = paircursor(&cursor, &vp);
	     vp;
	     vp = pairnext(&cursor)) {
		/*
		 *	Take this opportunity to verify all the VALUE_PAIRs are still valid.
		 */
		if (!talloc_get_type(vp, VALUE_PAIR)) {
			ERROR("Expected VALUE_PAIR pointer got \"%s\"", talloc_get_name(vp));

			log_talloc_report(vp);
			rad_assert(0);
		}

		vp_print(fr_log_fp, vp);
	}
	fflush(fr_log_fp);
}

/** Print a list of valuepairs to the request list.
 *
 * @param[in] level Debug level (1-4).
 * @param[in] request to read logging params from.
 * @param[in] vp to print.
 */
void rdebug_pair_list(int level, REQUEST *request, VALUE_PAIR *vp)
{
	vp_cursor_t cursor;
	char buffer[256];
	if (!vp || !request || !request->radlog) return;

	for (vp = paircursor(&cursor, &vp);
	     vp;
	     vp = pairnext(&cursor)) {
		/*
		 *	Take this opportunity to verify all the VALUE_PAIRs are still valid.
		 */
		if (!talloc_get_type(vp, VALUE_PAIR)) {
			REDEBUG("Expected VALUE_PAIR pointer got \"%s\"", talloc_get_name(vp));

			log_talloc_report(vp);
			rad_assert(0);
		}

		vp_prints(buffer, sizeof(buffer), vp);

		request->radlog(L_DBG, level, request, "\t%s", buffer);
	}
}

/** Resolve attribute pair_lists_t value to an attribute list.
 *
 * The value returned is a pointer to the pointer of the HEAD of the list
 * in the REQUEST. If the head of the list changes, the pointer will still
 * be valid.
 *
 * @param[in] request containing the target lists.
 * @param[in] list pair_list_t value to resolve to VALUE_PAIR list.
 *	Will be NULL if list name couldn't be resolved.
 */
VALUE_PAIR **radius_list(REQUEST *request, pair_lists_t list)
{
	if (!request) return NULL;

	switch (list) {
		case PAIR_LIST_UNKNOWN:
		default:
			break;

		case PAIR_LIST_REQUEST:
			return &request->packet->vps;

		case PAIR_LIST_REPLY:
			return &request->reply->vps;

		case PAIR_LIST_CONTROL:
			return &request->config_items;

#ifdef WITH_PROXY
		case PAIR_LIST_PROXY_REQUEST:
			if (!request->proxy) break;
			return &request->proxy->vps;

		case PAIR_LIST_PROXY_REPLY:
			if (!request->proxy) break;
			return &request->proxy_reply->vps;
#endif
#ifdef WITH_COA
		case PAIR_LIST_COA:
			if (request->coa &&
			    (request->coa->proxy->code == PW_COA_REQUEST)) {
				return &request->coa->proxy->vps;
			}
			break;

		case PAIR_LIST_COA_REPLY:
			if (request->coa && /* match reply with request */
			    (request->coa->proxy->code == PW_COA_REQUEST) &&
			    request->coa->proxy_reply) {
				return &request->coa->proxy_reply->vps;
			}
			break;

		case PAIR_LIST_DM:
			if (request->coa &&
			    (request->coa->proxy->code == PW_DISCONNECT_REQUEST)) {
				return &request->coa->proxy->vps;
			}
			break;

		case PAIR_LIST_DM_REPLY:
			if (request->coa && /* match reply with request */
			    (request->coa->proxy->code == PW_DISCONNECT_REQUEST) &&
			    request->coa->proxy_reply) {
			   	return &request->coa->proxy->vps;
			}
			break;
#endif
	}

	RWDEBUG2("List \"%s\" is not available",
		fr_int2str(pair_lists, list, "<INVALID>"));

	return NULL;
}

/** Convert value_pair_map_t to VALUE_PAIR(s) and add them to a REQUEST.
 *
 * Takes a single value_pair_map_t, resolves request and list identifiers
 * to pointers in the current request, then attempts to retrieve module
 * specific value(s) using callback, and adds the resulting values to the
 * correct request/list.
 *
 * @param request The current request.
 * @param map specifying destination attribute and location and src identifier.
 * @param func to retrieve module specific values and convert them to
 *	VALUE_PAIRS.
 * @param ctx to be passed to func.
 * @param src name to be used in debugging if different from map value.
 * @return -1 if the operation failed, -2 in the source attribute wasn't valid, 0 on success.
 */
int radius_map2request(REQUEST *request, value_pair_map_t const *map,
		       UNUSED char const *src, radius_tmpl_getvalue_t func, void *ctx)
{
	int rcode;
	vp_cursor_t cursor;
	VALUE_PAIR **list, *vp, *head = NULL;
	char buffer[1024];

	if (radius_request(&request, map->dst->request) < 0) {
		REDEBUG("Mapping \"%s\" -> \"%s\" invalid in this context", map->src->name, map->dst->name);

		return -2;
	}

	list = radius_list(request, map->dst->list);
	if (!list) {
		REDEBUG("Mapping \"%s\" -> \"%s\" invalid in this context", map->src->name, map->dst->name);

		return -2;
	}


	/*
	 *	The callback should either return -1 to signify operations error, -2 when it can't find the
	 *	attribute or list being referenced, or 0 to signify success.
	 *	It may return "sucess", but still have no VPs to work with.
	 *	Only if it returned an error code should it not write anything to the head pointer.
	 */
	rcode = func(&head, request, map, ctx);
	if (rcode < 0) {
		rad_assert(!head);

		return rcode;
	}

	if (!head) return 0;

	VERIFY_VP(head);

	if (debug_flag) for (vp = paircursor(&cursor, &head); vp; vp = pairnext(&cursor)) {
		char *value;

		switch (map->src->type) {
			/*
			 *	Just print the value being assigned
			 */
			default:

			case VPT_TYPE_LITERAL:
				vp_prints_value(buffer, sizeof(buffer), vp, '\'');
				value = buffer;
				break;
			case VPT_TYPE_XLAT:
				vp_prints_value(buffer, sizeof(buffer), vp, '"');
				value = buffer;
				break;
			case VPT_TYPE_DATA:
				vp_prints_value(buffer, sizeof(buffer), vp, 0);
				value = buffer;
				break;
			/*
			 *	Just printing the value doesn't make sense, but we still
			 *	want to know what it was...
			 */
			case VPT_TYPE_LIST:
				vp_prints_value(buffer, sizeof(buffer), vp, '\'');
				value = talloc_asprintf(request, "&%s%s -> %s", map->src->name, vp->da->name, buffer);
				break;
			case VPT_TYPE_ATTR:
				vp_prints_value(buffer, sizeof(buffer), vp, '\'');
				value = talloc_asprintf(request, "&%s -> %s", map->src->name, buffer);
				break;
		}


		RDEBUG("\t\t%s %s %s", map->dst->name, fr_int2str(fr_tokens, vp->op, "<INVALID>"), value);

		if (value != buffer) talloc_free(value);
	}

	/*
	 *	Use pairmove so the operator is respected
	 */
	radius_pairmove(request, list, head);
	return 0;
}

/** Process map which has exec as a src
 *
 * Evaluate maps which specify exec as a src. This may be used by various sorts of update sections, and so
 * has been broken out into it's own function.
 *
 * @param[out] out Where to write the VALUE_PAIR(s).
 * @param[in] request structure (used only for talloc).
 * @param[in] map the map. The LHS (dst) must be VPT_TYPE_ATTR or VPT_TYPE_LIST. The RHS (src) must be VPT_TYPE_EXEC.
 * @return -1 on failure, 0 on success.
 */
int radius_mapexec(VALUE_PAIR **out, REQUEST *request, value_pair_map_t const *map)
{
	int result;
	char *expanded = NULL;
	char answer[1024];
	VALUE_PAIR **input_pairs = NULL;
	VALUE_PAIR **output_pairs = NULL;

	*out = NULL;

	rad_assert(map->src->type == VPT_TYPE_EXEC);
	rad_assert((map->dst->type == VPT_TYPE_ATTR) || (map->dst->type == VPT_TYPE_LIST));

	/*
	 *	We always put the request pairs into the environment
	 */
	input_pairs = radius_list(request, PAIR_LIST_REQUEST);

	/*
	 *	Automagically switch output type depending on our destination
	 *	If dst is a list, then we create attributes from the output of the program
	 *	if dst is an attribute, then we create an attribute of that type and then
	 *	call pairparsevalue on the output of the script.
	 */
	out[0] = '\0';
	result = radius_exec_program(request, map->src->name, true, true,
				     answer, sizeof(answer),
				     input_pairs ? *input_pairs : NULL,
				     (map->dst->type == VPT_TYPE_LIST) ? output_pairs : NULL);
	talloc_free(expanded);
	if (result != 0) {
		REDEBUG("%s", answer);
		talloc_free(output_pairs);
		return -1;
	}

	switch (map->dst->type) {
	case VPT_TYPE_LIST:
		if (!output_pairs) {
			return -2;
		}
		*out = *output_pairs;

		return 0;
	case VPT_TYPE_ATTR:
		{
			VALUE_PAIR *vp;

			vp = pairalloc(request, map->dst->da);
			if (!vp) return -1;
			vp->op = map->op;
			if (!pairparsevalue(vp, answer)) {
				pairfree(&vp);
				return -2;
			}
			*out = vp;

			return 0;
		}
	default:
		rad_assert(0);
	}

	return -1;
}

/** Convert a map to a VALUE_PAIR.
 *
 * @param[out] out Where to write the VALUE_PAIR(s).
 * @param[in] request structure (used only for talloc)
 * @param[in] map the map. The LHS (dst) has to be VPT_TYPE_ATTR or VPT_TYPE_LIST.
 * @param[in] ctx unused
 * @return 0 on success, -1 on failure, -2 on attribute not found/equivalent
 */
int radius_map2vp(VALUE_PAIR **out, REQUEST *request, value_pair_map_t const *map, UNUSED void *ctx)
{
	int rcode = 0;
	VALUE_PAIR *vp = NULL, *found, **from = NULL;
	DICT_ATTR const *da;
	REQUEST *context;
	vp_cursor_t cursor;

	rad_assert(request != NULL);
	rad_assert(map != NULL);

	*out = NULL;

	/*
	 *	Special case for !*, we don't need to parse the value, just allocate an attribute with
	 *	the right operator.
	 */
	if (map->op == T_OP_CMP_FALSE) {
		vp = pairalloc(request, map->dst->da);
		if (!vp) return -1;
		vp->op = map->op;
		*out = vp;

		return 0;
	}

	/*
	 *	List to list found, this is a special case because we don't need
	 *	to allocate any attributes, just found the current list, and change
	 *	the op.
	 */
	if ((map->dst->type == VPT_TYPE_LIST) && (map->src->type == VPT_TYPE_LIST)) {
		from = radius_list(request, map->src->list);
		if (!from) return -2;

		found = paircopy(request, *from);
		/*
		 *	List to list copy is invalid if the src list has no attributes.
		 */
		if (!found) return -2;

		for (vp = paircursor(&cursor, &found);
		     vp;
		     vp = pairnext(&cursor)) {
		 	vp->op = T_OP_ADD;
		}

		*out = found;

		return 0;
	}

	/*
	 *	Deal with all non-list founding operations.
	 */
	da = map->dst->da ? map->dst->da : map->src->da;

	switch (map->src->type) {
	case VPT_TYPE_XLAT:
	case VPT_TYPE_LITERAL:
	case VPT_TYPE_DATA:
		vp = pairalloc(request, da);
		if (!vp) return -1;
		vp->op = map->op;
		break;
	default:
		break;
	}


	/*
	 *	And parse the RHS
	 */
	switch (map->src->type) {
	case VPT_TYPE_XLAT:
		rad_assert(map->dst->da);	/* Need to know where were going to write the new attribute */
		/*
		 *	Don't call unnecessary expansions
		 */
		if (strchr(map->src->name, '%') != NULL) {
			ssize_t slen;
			char *str = NULL;

			slen = radius_axlat(&str, request, map->src->name, NULL, NULL);
			if (slen < 0) {
				rcode = slen;
				goto error;
			}
			rcode = pairparsevalue(vp, str);
			talloc_free(str);
			if (!rcode) {
				pairfree(&vp);
				rcode = -1;
				goto error;
			}

			break;
		}
		/* FALL-THROUGH */

	case VPT_TYPE_LITERAL:
		if (!pairparsevalue(vp, map->src->name)) {
			rcode = -2;
			goto error;
		}
		break;

	case VPT_TYPE_ATTR:
		rad_assert(!map->dst->da ||
			   (map->src->da->type == map->dst->da->type) ||
			   (map->src->da->type == PW_TYPE_OCTETS) ||
			   (map->dst->da->type == PW_TYPE_OCTETS));
		context = request;

		if (radius_request(&context, map->src->request) == 0) {
			from = radius_list(context, map->src->list);
		}

		/*
		 *	Can't add the attribute if the list isn't
		 *	valid.
		 */
		if (!from) {
			rcode = -2;
			goto error;
		}

		/*
		 *	Special case, destination is a list, found all instance of an attribute.
		 */
		if (map->dst->type == VPT_TYPE_LIST) {
			found = paircopy2(request, *from, map->src->da->attr, map->src->da->vendor, TAG_ANY);
			if (!found) {
				REDEBUG("Attribute \"%s\" not found in request", map->src->name);
				rcode = -2;
				goto error;
			}

			for (vp = paircursor(&cursor, &found);
			     vp;
			     vp = pairnext(&cursor)) {
				vp->op = T_OP_ADD;
			}

			*out = found;
			return 0;
		}

		/*
		 *	FIXME: allow tag references?
		 */
		found = pairfind(*from, map->src->da->attr, map->src->da->vendor, TAG_ANY);
		if (!found) {
			REDEBUG("Attribute \"%s\" not found in request", map->src->name);
			rcode = -2;
			goto error;
		}

		/*
		 *	Copy the data over verbatim, assuming it's
		 *	actually data.
		 */
//		rad_assert(found->type == VT_DATA);
		vp = paircopyvpdata(request, da, found);
		if (!vp) {
			return -1;
		}
		vp->op = map->op;

		break;

	case VPT_TYPE_DATA:
		rad_assert(map->src->da->type == map->dst->da->type);
		memcpy(&vp->data, map->src->vpd, sizeof(vp->data));
		vp->length = map->src->length;
		break;

	/*
	 *	This essentially does the same as rlm_exec xlat, except it's non-configurable.
	 *	It's only really here as a convenience for people who expect the contents of
	 *	backticks to be executed in a shell.
	 *
	 *	exec string is xlat expanded and arguments are shell escaped.
	 */
	case VPT_TYPE_EXEC:
		return radius_mapexec(out, request, map);
	default:
		rad_assert(0);	/* Should of been caught at parse time */
	error:
		pairfree(&vp);
		return rcode;
	}

	*out = vp;
	return 0;
}


/** Convert a valuepair string to VALUE_PAIR and insert it into a list
 *
 * Takes a valuepair string with list and request qualifiers, converts it into a VALUE_PAIR
 * and inserts it into the appropriate list.
 *
 * @param request Current request.
 * @param raw string to parse.
 * @param request_def to use if attribute isn't qualified.
 * @param list_def to use if attribute isn't qualified.
 * @return 0 on success, -1 on error.
 */
int radius_str2vp(REQUEST *request, char const *raw, request_refs_t request_def, pair_lists_t list_def)
{
	char const *p;
	size_t len;
	request_refs_t req;
	pair_lists_t list;

	VALUE_PAIR *vp = NULL;
	VALUE_PAIR **vps;

	p = raw;

	req = radius_request_name(&p, request_def);
	len = p - raw;
	if (req == REQUEST_UNKNOWN) {
		REDEBUG("Invalid request qualifier \"%.*s\"", (int) len, raw);

		return -1;
	}
	raw += len;

	list = radius_list_name(&p, list_def);
	if (list == PAIR_LIST_UNKNOWN) {
		len = p - raw;

		REDEBUG("Invalid list qualifier \"%.*s\"", (int) len, raw);

		return -1;
	}
	raw += len;

	if (radius_request(&request, req) < 0) {
		return -1;
	}

	vps = radius_list(request, list);
	if (!vps) {
		return -1;
	}

	if (userparse(request, raw, &vp) == T_OP_INVALID) {
		return -1;
	}

	pairmove(request, vps, &vp);

	return 0;
}


/** Return a VP from a value_pair_tmpl_t
 *
 * @param request current request.
 * @param vpt the value pair template
 * @return NULL if not found, or the VPs.
 */
VALUE_PAIR *radius_vpt_get_vp(REQUEST *request, value_pair_tmpl_t const *vpt)
{
	VALUE_PAIR **vps;

	if (radius_request(&request, vpt->request) < 0) {
		return NULL;
	}

	vps = radius_list(request, vpt->list);
	if (!vps) {
		return NULL;
	}

	switch (vpt->type) {
	/*
	 *	May not may not be found, but it *is* a known name.
	 */
	case VPT_TYPE_ATTR:
		return pairfind(*vps, vpt->da->attr, vpt->da->vendor, TAG_ANY);

	case VPT_TYPE_LIST:
		return *vps;

	default:
		break;
	}

	return NULL;
}



/** Return a VP from the specified request.
 *
 * @param request current request.
 * @param name attribute name including qualifiers.
 * @param vp_p where to write the pointer to the resolved VP.
 *	Will be NULL if the attribute couldn't be resolved.
 * @return -1 if either the attribute or qualifier were invalid, else 0
 */
int radius_get_vp(REQUEST *request, char const *name, VALUE_PAIR **vp_p)
{
	value_pair_tmpl_t vpt;

	*vp_p = NULL;

	if (radius_parse_attr(name, &vpt, REQUEST_CURRENT,
	    PAIR_LIST_REQUEST) < 0) {
		return -1;
	}

	*vp_p = radius_vpt_get_vp(request, &vpt);
	return 0;
}

/** Add a module failure message VALUE_PAIR to the request
 */
void module_failure_msg(REQUEST *request, char const *fmt, ...)
{
	va_list ap;
	char *p;
	VALUE_PAIR *vp;

	if (!fmt || !request->packet) {
		va_start(ap, fmt);
		va_end(ap);
		return;
	}

	va_start(ap, fmt);
	vp = paircreate(request->packet, PW_MODULE_FAILURE_MESSAGE, 0);
	if (!vp) {
		va_end(ap);
		return;
	}

	p = talloc_vasprintf(vp, fmt, ap);

	if (request->module && *request->module) {
		pairsprintf(vp, "%s: %s", request->module, p);
	} else {
		pairsprintf(vp, "%s", p);
	}
	talloc_free(p);
	pairadd(&request->packet->vps, vp);
}
