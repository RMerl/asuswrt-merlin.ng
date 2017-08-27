/*
 * acct.c	Accounting routines.
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
 * Copyright 2000  Miquel van Smoorenburg <miquels@cistron.nl>
 * Copyright 2000  Alan DeKok <aland@ox.org>
 * Copyright 2000  Alan Curry <pacman@world.std.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#ifdef WITH_ACCOUNTING
/*
 *	rad_accounting: call modules.
 *
 *	The return value of this function isn't actually used right now, so
 *	it's not entirely clear if it is returning the right things. --Pac.
 */
int rad_accounting(REQUEST *request)
{
	int result = RLM_MODULE_OK;


#ifdef WITH_PROXY
#define WAS_PROXIED (request->proxy)
#else
#define WAS_PROXIED (0)
#endif

	/*
	 *	Run the modules only once, before proxying.
	 */
	if (!WAS_PROXIED) {
		VALUE_PAIR	*vp;
		int		acct_type = 0;

		result = module_preacct(request);
		switch (result) {
			/*
			 *	The module has a number of OK return codes.
			 */
			case RLM_MODULE_NOOP:
			case RLM_MODULE_OK:
			case RLM_MODULE_UPDATED:
				break;
			/*
			 *	The module handled the request, stop here.
			 */
			case RLM_MODULE_HANDLED:
				return result;
			/*
			 *	The module failed, or said the request is
			 *	invalid, therefore we stop here.
			 */
			case RLM_MODULE_FAIL:
			case RLM_MODULE_INVALID:
			case RLM_MODULE_NOTFOUND:
			case RLM_MODULE_REJECT:
			case RLM_MODULE_USERLOCK:
			default:
				return result;
		}

		/*
		 *	Do the data storage before proxying. This is to ensure
		 *	that we log the packet, even if the proxy never does.
		 */
		vp = pairfind(request->config_items, PW_ACCT_TYPE, 0, TAG_ANY);
		if (vp) {
			acct_type = vp->vp_integer;
			DEBUG2("  Found Acct-Type %s",
			       dict_valnamebyattr(PW_ACCT_TYPE, 0, acct_type));
		}
		result = process_accounting(acct_type, request);
		switch (result) {
			/*
			 *	In case the accounting module returns FAIL,
			 *	it's still useful to send the data to the
			 *	proxy.
			 */
			case RLM_MODULE_FAIL:
			case RLM_MODULE_NOOP:
			case RLM_MODULE_OK:
			case RLM_MODULE_UPDATED:
				break;
			/*
			 *	The module handled the request, don't reply.
			 */
			case RLM_MODULE_HANDLED:
				return result;
			/*
			 *	Neither proxy, nor reply to invalid requests.
			 */
			case RLM_MODULE_INVALID:
			case RLM_MODULE_NOTFOUND:
			case RLM_MODULE_REJECT:
			case RLM_MODULE_USERLOCK:
			default:
				return result;
		}

		/*
		 *	Maybe one of the preacct modules has decided
		 *	that a proxy should be used.
		 */
		if ((vp = pairfind(request->config_items, PW_PROXY_TO_REALM, 0, TAG_ANY))) {
			REALM *realm;

			/*
			 *	Check whether Proxy-To-Realm is
			 *	a LOCAL realm.
			 */
			realm = realm_find2(vp->vp_strvalue);
			if (realm && !realm->acct_pool) {
				DEBUG("rad_accounting: Cancelling proxy to realm %s, as it is a LOCAL realm.", realm->name);
				pairdelete(&request->config_items, PW_PROXY_TO_REALM, 0, TAG_ANY);
			} else {
				/*
				 *	Don't reply to the NAS now because
				 *	we have to send the proxied packet
				 *	before that.
				 */
				return result;
			}
		}
	}

	/*
	 *	We get here IF we're not proxying, OR if we've
	 *	received the accounting reply from the end server,
	 *	THEN we can reply to the NAS.
	 *      If the accounting module returns NOOP, the data
	 *      storage did not succeed, so radiusd should not send
	 *      Accounting-Response.
	 */
	switch (result) {
		/*
		 *	Send back an ACK to the NAS.
		 */
		case RLM_MODULE_OK:
		case RLM_MODULE_UPDATED:
			request->reply->code = PW_ACCOUNTING_RESPONSE;
			break;

		/*
		 *	Failed to log or to proxy the accounting data,
		 *	therefore don't reply to the NAS.
		 */
		case RLM_MODULE_FAIL:
		case RLM_MODULE_INVALID:
		case RLM_MODULE_NOOP:
		case RLM_MODULE_NOTFOUND:
		case RLM_MODULE_REJECT:
		case RLM_MODULE_USERLOCK:
		default:
			break;
	}
	return result;
}
#endif
