/* Plugin for DynDNS2 API compatible services
 *
 * Supported DDNS providers are:
 *   - DynDNS,
 *   - DNS-O-Matic,
 *   - no-ip,
 *   - 3322,
 *   - Hurricane-Electric (HE)
 *   - Loopia, and
 *   - nsupdate.info
 *   - Google Domains
 *   - SPDYN
 *   - Dynu
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2020  Joachim Nilsson <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "plugin.h"

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t dyndns = {
	.name         = "default@dyndns.org",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "members.dyndns.org",
	.server_url   = "/nic/update"
};

static ddns_system_t dnsomatic = {
	.name         = "default@dnsomatic.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

/* unreliable due some global rate-limiting
	.checkip_name = "myip.dnsomatic.com",
	.checkip_url  = "/",
*/
	.checkip_name = DDNS_MY_IP_SERVER,
	.checkip_url  = DDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DDNS_MY_IP_SSL,

	.server_name  = "updates.dnsomatic.com",
	.server_url   = "/nic/update"
};

/* Support HOSTNAME.selfhost.eu, see issue #215 */
static ddns_system_t selfhost = {
	.name         = "default@selfhost.de",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "carol.selfhost.de",
	.server_url   = "/nic/update"
};

static ddns_system_t no_ip = {
	.name         = "default@no-ip.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "ip1.dynupdate.no-ip.com",
	.checkip_url  = "/",
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "dynupdate.no-ip.com",
	.server_url   = "/nic/update"
};

static ddns_system_t noip = {
	.name         = "default@noip.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "ip1.dynupdate.noip.com",
	.checkip_url  = "/",
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "dynupdate.noip.com",
	.server_url   = "/nic/update"
};

/* http://www.pubyun.com/wiki/%E5%B8%AE%E5%8A%A9:api#%E6%8E%A5%E5%8F%A3%E5%9C%B0%E5%9D%80 */
static ddns_system_t _3322 = {
	.name         = "dyndns@3322.org",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "ip.3322.net",
	.checkip_url  = "/",

	.server_name  = "members.3322.org",
	.server_url   = "/dyndns/update"
};

/* See also tunnelbroker.c for Hurricate Electric's IPv6 service */
static ddns_system_t henet = {
	.name         = "dyndns@he.net",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "checkip.dns.he.net",
	.checkip_url  = "/",

	.server_name  = "dyn.dns.he.net",
	.server_url   = "/nic/update"
};

/* New API, see also the old compat plugin for Hurricane Electric's
 * IPv6 tunnel brokering service in tunnelbroker.c, and above.
 * For API details, see their forum posting at
 * https://forums.he.net/index.php?topic=3153.msg19774#msg19774 */
static ddns_system_t tunnelbroker = {
	.name         = "default@tunnelbroker.net",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "ipv4.tunnelbroker.net",
	.server_url   = "/nic/update"
};

/*
 * Securepoint DDNS service (SPDYN)
 * http://wiki.securepoint.de/index.php/SPDNS_FAQ
 */
static ddns_system_t spdyn = {
	.name         = "default@spdyn.de",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "checkip4.spdyn.de",
	.checkip_url  = "/",
	.checkip_ssl  = DDNS_CHECKIP_SSL_UNSUPPORTED,

	.server_name  = "update.spdyn.de",
	.server_url   = "/nic/update"
};

/* Note: below is IPv4 only. ipv6.nsupdate.info would work IPv6 only. */
static ddns_system_t nsupdate_info_ipv4 = {
	.name         = "ipv4@nsupdate.info",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "ipv4.nsupdate.info",
	.checkip_url  = "/myip",

	.server_name  = "ipv4.nsupdate.info",
	.server_url   = "/nic/update"
};

/*
 * Loopia supports HTTPS, for details on supported variables, see
 * https://support.loopia.com/wiki/About_the_DynDNS_support
 */
static ddns_system_t loopia = {
	.name         = "default@loopia.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "dns.loopia.se",
	.checkip_url  = "/checkip/checkip.php",

	.server_name  = "dns.loopia.se",
	.server_url   = "/XDynDNSServer/XDynDNS.php"
};

static ddns_system_t googledomains = {
	.name         = "default@domains.google.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "domains.google.com",
	.server_url   = "/nic/update"
};

static ddns_system_t dynu = {
	.name         = "default@dynu.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "checkip.dynu.com",
	.checkip_url  = "/",

	.server_name  = "api.dynu.com",
	.server_url   = "/nic/update"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	return common_request(ctx, info, alias);
}

static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	return common_response(trans, info, alias);
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&dyndns);
	plugin_register(&dnsomatic);
	plugin_register(&selfhost);
	plugin_register(&no_ip);
	plugin_register(&noip);
	plugin_register(&_3322);
	plugin_register(&henet);
	plugin_register(&tunnelbroker);
	plugin_register(&spdyn);
	plugin_register(&nsupdate_info_ipv4);
	plugin_register(&loopia);
	plugin_register(&googledomains);
	plugin_register(&dynu);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&dyndns);
	plugin_unregister(&dnsomatic);
	plugin_unregister(&selfhost);
	plugin_unregister(&no_ip);
	plugin_unregister(&noip);
	plugin_unregister(&_3322);
	plugin_unregister(&henet);
	plugin_unregister(&tunnelbroker);
	plugin_unregister(&spdyn);
	plugin_unregister(&nsupdate_info_ipv4);
	plugin_unregister(&loopia);
	plugin_unregister(&googledomains);
	plugin_unregister(&dynu);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
