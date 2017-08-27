/*
 * paircmp.c	Valuepair functions for various attributes
 *
 * Version:	$Id$
 *
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
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 * Copyright 2000  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>
#include "rlm_expr.h"

/*
 *	Compare a Connect-Info and a Connect-Rate
 */
static int connectcmp(UNUSED void *instance,
		      REQUEST *req UNUSED,
		      VALUE_PAIR *request,
		      VALUE_PAIR *check,
		      UNUSED VALUE_PAIR *check_pairs,
		      UNUSED VALUE_PAIR **reply_pairs)
{
	int rate;

	rate = atoi(request->vp_strvalue);
	return rate - check->vp_integer;
}

/*
 *	Compare prefix/suffix.
 *
 *	If they compare:
 *	- if PW_STRIP_USER_NAME is present in check_pairs,
 *	  strip the username of prefix/suffix.
 *	- if PW_STRIP_USER_NAME is not present in check_pairs,
 *	  add a PW_STRIPPED_USER_NAME to the request.
 */
static int presufcmp(UNUSED void *instance,
		     REQUEST *req,
		     VALUE_PAIR *request,
		     VALUE_PAIR *check,
		     VALUE_PAIR *check_pairs,
		     UNUSED VALUE_PAIR **reply_pairs)
{
	VALUE_PAIR *vp;
	char const *name;
	char rest[MAX_STRING_LEN];
	int len, namelen;
	int ret = -1;

	if (!request) {
		return -1;
	}

	VERIFY_VP(request);
	VERIFY_VP(check);
	rad_assert(request->da->type == PW_TYPE_STRING);

	name = request->vp_strvalue;

#if 0 /* DEBUG */
	printf("Comparing %s and %s, check->attr is %d\n", name, check->vp_strvalue, check->attribute);
#endif

	len = strlen(check->vp_strvalue);
	if (check->da->vendor == 0) switch (check->da->attr) {
		case PW_PREFIX:
			ret = strncmp(name, check->vp_strvalue, len);
			if (ret == 0)
				strlcpy(rest, name + len, sizeof(rest));
			break;
		case PW_SUFFIX:
			namelen = strlen(name);
			if (namelen < len)
				break;
			ret = strcmp(name + namelen - len,
				     check->vp_strvalue);
			if (ret == 0) {
				strlcpy(rest, name, namelen - len + 1);
			}
			break;
	}
	if (ret != 0) {
		return ret;
	}

	/*
	 *	If Strip-User-Name == No, then don't do any more.
	 */
	vp = pairfind(check_pairs, PW_STRIP_USER_NAME, 0, TAG_ANY);
	if (vp && !vp->vp_integer) return ret;

	/*
	 *	See where to put the stripped user name.
	 */
	vp = pairfind(check_pairs, PW_STRIPPED_USER_NAME, 0, TAG_ANY);
	if (!vp) {
		/*
		 *	If "request" is NULL, then the memory will be
		 *	lost!
		 */
		vp = radius_paircreate(req, &request, PW_STRIPPED_USER_NAME, 0);
		if (!vp) return ret;
		req->username = vp;
	}

	pairstrcpy(vp, rest);

	return ret;
}


/*
 *	Compare the request packet type.
 */
static int packetcmp(UNUSED void *instance,
		     REQUEST *request,
		     UNUSED VALUE_PAIR *req,
		     VALUE_PAIR *check,
		     UNUSED VALUE_PAIR *check_pairs,
		     UNUSED VALUE_PAIR **reply_pairs)
{
	if (request->packet->code == check->vp_integer) {
		return 0;
	}

	return 1;
}

/*
 *	Compare the response packet type.
 */
static int responsecmp(UNUSED void *instance,
		       REQUEST *request,
		       UNUSED VALUE_PAIR *req,
		       VALUE_PAIR *check,
		       UNUSED VALUE_PAIR *check_pairs,
		       UNUSED VALUE_PAIR **reply_pairs)
{
	if (request->reply->code == check->vp_integer) {
		return 0;
	}

	return 1;
}

/*
 *	Generic comparisons, via xlat.
 */
static int genericcmp(UNUSED void *instance,
		      REQUEST *request,
		      UNUSED VALUE_PAIR *req,
		      VALUE_PAIR *check,
		      UNUSED VALUE_PAIR *check_pairs,
		      UNUSED VALUE_PAIR **reply_pairs)
{
	if ((check->op != T_OP_REG_EQ) &&
	    (check->op != T_OP_REG_NE)) {
		int rcode;
		char name[1024];
		char value[1024];
		VALUE_PAIR *vp;

		snprintf(name, sizeof(name), "%%{%s}", check->da->name);

		if (radius_xlat(value, sizeof(value), request, name, NULL, NULL) < 0) {
			return 0;
		}
		vp = pairmake(req, NULL, check->da->name, value, check->op);

		/*
		 *	Paircmp returns 0 for failed comparison,
		 *	1 for succeeded.
		 */
		rcode = paircmp(check, vp);

		/*
		 *	We're being called from radius_callback_compare,
		 *	which wants 0 for success, and 1 for fail (sigh)
		 *
		 *	We should really fix the API so that it is
		 *	consistent.  i.e. the comparison callbacks should
		 *	return ONLY the resut of comparing A to B.
		 *	The radius_callback_cmp function should then
		 *	take care of using the operator to see if the
		 *	condition (A OP B) is true or not.
		 *
		 *	This would also allow "<", etc. to work in the
		 *	callback functions...
		 *
		 *	See rlm_ldap, ...groupcmp() for something that
		 *	returns 0 for matched, and 1 for didn't match.
		 */
		rcode = !rcode;
		pairfree(&vp);

		return rcode;
	}

	/*
	 *	Will do the xlat for us
	 */
	return radius_compare_vps(request, check, NULL);
}

static int generic_attrs[] = {
	PW_CLIENT_IP_ADDRESS,
	PW_PACKET_SRC_IP_ADDRESS,
	PW_PACKET_DST_IP_ADDRESS,
	PW_PACKET_SRC_PORT,
	PW_PACKET_DST_PORT,
	PW_REQUEST_PROCESSING_STAGE,
	PW_PACKET_SRC_IPV6_ADDRESS,
	PW_PACKET_DST_IPV6_ADDRESS,
	PW_VIRTUAL_SERVER,
	0
};

/*
 *	Register server-builtin special attributes.
 */
void pair_builtincompare_add(void *instance)
{
	int i;

	paircompare_register(dict_attrbyvalue(PW_PREFIX, 0), dict_attrbyvalue(PW_USER_NAME, 0), false, presufcmp, instance);
	paircompare_register(dict_attrbyvalue(PW_SUFFIX, 0), dict_attrbyvalue(PW_USER_NAME, 0), false, presufcmp, instance);
	paircompare_register(dict_attrbyvalue(PW_CONNECT_RATE, 0), dict_attrbyvalue(PW_CONNECT_INFO, 0),
				false, connectcmp, instance);
	paircompare_register(dict_attrbyvalue(PW_PACKET_TYPE, 0), NULL, true, packetcmp, instance);
	paircompare_register(dict_attrbyvalue(PW_RESPONSE_PACKET_TYPE, 0), NULL, true, responsecmp, instance);

	for (i = 0; generic_attrs[i] != 0; i++) {
		paircompare_register(dict_attrbyvalue(generic_attrs[i], 0), NULL, true, genericcmp, instance);
	}
}
