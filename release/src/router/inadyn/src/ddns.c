/* DDNS client updater main implementation file
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2021  Joachim Wiberg <troglobit@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <net/if.h>

#include "ddns.h"
#include "cache.h"
#include "log.h"
#include "base64.h"
#include "md5.h"
#include "sha1.h"

/* Conversation with the checkip server */
#define DYNDNS_CHECKIP_HTTP_REQUEST  					\
	"GET %s HTTP/1.0\r\n"						\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n\r\n"

/* Used to preserve values during reset at SIGHUP.  Time also initialized from cache file at startup. */
static int cached_num_iterations = 0;
extern ddns_info_t *conf_info_iterator(int first);


static int wait_for_cmd(ddns_t *ctx)
{
	int counter;
	ddns_cmd_t old_cmd;

	if (!ctx)
		return RC_INVALID_POINTER;

	old_cmd = ctx->cmd;
	if (old_cmd != NO_CMD)
		return 0;

	counter = ctx->update_period / ctx->cmd_check_period;
	while (counter--) {
		if (ctx->cmd != old_cmd)
			break;

		sleep(ctx->cmd_check_period);
	}

	return 0;
}

static int shell_transaction(ddns_t *ctx, ddns_info_t *info, const char *cmd)
{
	FILE *pipe;
	int rc = 0;

	snprintf(ctx->request_buf, ctx->request_buflen,
		"INADYN_PROVIDER=\"%s\" INADYN_USER=\"%s\" %s",
		info->system->name, info->creds.username, cmd);

	logit(LOG_DEBUG, "Starting command to get my public IP#: %s", ctx->request_buf);

	pipe = popen(ctx->request_buf, "r");
	if (!pipe) {
		logit(LOG_ERR, "Cannot run '%s': %s", ctx->request_buf, strerror(errno));
		return errno;
	}

	/* TODO: timeout on fread */
	rc = fread(ctx->work_buf, 1, ctx->work_buflen, pipe);
	if (rc < 0) {
		logit(LOG_ERR, "Error running '%s': %s", ctx->request_buf, strerror(errno));
		ctx->work_buf[0] = 0;
		rc = errno;
	} else if (rc == 0) {
		logit(LOG_ERR, "Error running '%s': 0 bytes read", ctx->request_buf);
		ctx->work_buf[0] = 0;
		rc = RC_INVALID_POINTER;
	} else {
		logit(LOG_DEBUG, "Command '%s' returns %d bytes", ctx->request_buf, rc);
		ctx->work_buf[rc] = 0;
		rc = 0;
	}
	pclose(pipe);

	return rc;
}

static int get_req_for_ip_server(ddns_t *ctx, ddns_info_t *info)
{
	return snprintf(ctx->request_buf, ctx->request_buflen,
			DYNDNS_CHECKIP_HTTP_REQUEST, info->checkip_url,
			info->checkip_name.name, info->user_agent);
}

/*
 * Send req to IP server and get the response
 */
static int server_transaction(ddns_t *ctx, ddns_info_t *provider)
{
	int rc = 0;
	http_t *client;
	http_trans_t *trans;

	if (!provider) {
		logit(LOG_ERR, "Cannot query our address, invalid DDNS provider data!");
		return RC_INVALID_POINTER;
	}

	client = &provider->checkip;
	client->ssl_enabled = provider->checkip_ssl;
	DO(http_init(client, "Checking for IP# change", strstr(provider->system->name, "ipv6") ? TCP_FORCE_IPV6 : TCP_FORCE_IPV4));

	/* Prepare request for IP server */
	memset(ctx->work_buf, 0, ctx->work_buflen);
	memset(ctx->request_buf, 0, ctx->request_buflen);
	memset(&ctx->http_transaction, 0, sizeof(ctx->http_transaction));

	trans              = &ctx->http_transaction;
	trans->req_len     = get_req_for_ip_server(ctx, provider);
	trans->req         = ctx->request_buf;
	trans->rsp         = ctx->work_buf;
	trans->max_rsp_len = ctx->work_buflen - 1;	/* Save place for terminating \0 in string. */

	logit(LOG_DEBUG, "Querying DDNS checkip server for my public IP#: %s", ctx->request_buf);

	rc = http_transaction(client, &ctx->http_transaction);
	if (trans->status != 200)
		rc = RC_DDNS_INVALID_CHECKIP_RSP;

	http_exit(client);
	logit(LOG_DEBUG, "Server response: %s", trans->rsp);
	logit(LOG_DEBUG, "Checked my IP, return code %d: %s", rc, error_str(rc));

	return rc;
}

/*
 * IP address validator, discards empty, local, loopback and other
 * globally invalid addresses
 */
static int is_address_valid(int family, const char *host)
{
	/*
	 * cloudflare would return requested hostname before client's ip address
	 * block cloudflare ips so that https://1.1.1.1/cdn-cgi/trace would work
	 * even if 1.1.1.1 is the first ip in the response body
	 */
	static const char *except[] = {
		"1.1.1.1",
		"1.0.0.1",
		"2606:4700:4700::1111",
		"2606:4700:4700::1001",
		"1.1.1.2",
		"1.0.0.2",
		"2606:4700:4700::1112",
		"2606:4700:4700::1002",
		"1.1.1.3",
		"1.0.0.3",
		"2606:4700:4700::1113",
		"2606:4700:4700::1003",
		"2606:4700:4700::64",
		"2606:4700:4700::6400"
	};
	size_t i;

	for (i = 0; i < NELEMS(except); i++) {
		if (!strncmp(host, except[i], strlen(host))) {
			return 0;
		}
	}

	if (!verify_addr) {
		logit(LOG_DEBUG, "IP address validation disabled, %s is thus valid.", host);
		return 1;
	}

	if (family == AF_INET) {
		in_addr_t addr;
		struct in_addr address;

		logit(LOG_DEBUG, "Checking IPv4 address %s ...", host);
		if (!inet_pton(family, host, &address))
			goto error;

		addr = ntohl(address.s_addr);
		if (IN_ZERONET(addr)   || IN_LOOPBACK(addr) || IN_LINKLOCAL(addr) ||
		    IN_MULTICAST(addr) || IN_EXPERIMENTAL(addr))
			goto error;

		logit(LOG_DEBUG, "IPv4 address %s is valid.", host);
		return 1;
	}

	if (!allow_ipv6) {
		logit(LOG_INFO, "IPv6 address disallowed, enable with 'allow-ipv6 = true'");
		return 0;
	}

	if (family == AF_INET6) {
		struct in6_addr address, *addr = &address;

		logit(LOG_DEBUG, "Checking IPv6 address %s ...", host);
		if (!inet_pton(family, host, &address))
			goto error;

		if (IN6_IS_ADDR_UNSPECIFIED(addr) || IN6_IS_ADDR_LOOPBACK(addr) ||
		    IN6_IS_ADDR_LINKLOCAL(addr)   || IN6_IS_ADDR_SITELOCAL(addr))
			goto error;

		logit(LOG_DEBUG, "IPv6 address %s is valid.", host);
		return 1;
	}

error:
	logit(LOG_WARNING, "IP%s address %s is not a valid Internet address.",
	      family == AF_INET ? "v4" : family == AF_INET6 ? "v6" : "", host);
	return 0;
}

static int parse_ipv4_address(char *buffer, char *address, size_t len)
{
	int found = 0;
	static const char *accept = "0123456789.";
	char *needle, *haystack, *end;
	struct in_addr  addr;

	haystack = buffer;
	needle   = haystack;
	end      = haystack + strlen(haystack) - 1;
	while (needle && haystack < end) {
		char ch;
		size_t num = 0;

		needle = strpbrk(haystack, accept);
		if (needle) {
			num = strspn(needle, accept);
			if (num) {
				ch = needle[num];
				needle[num] = 0;

				if (inet_pton(AF_INET, needle, &addr) == 1) {
					inet_ntop(AF_INET, &addr, address, len);
					if (is_address_valid(AF_INET, address)) {
						found = 1;
						break;
					}
				}

				needle[num] = ch;
			}
		}

		/* nothing yet, skip to next search point */
		haystack = needle + num + 1;
	}

	return found;
}

static int parse_ipv6_address(char *buffer, char *address, size_t len)
{
	int found = 0;
	static const char *accept = "0123456789abcdefABCDEF:";
	char *needle, *haystack, *end;
	struct in6_addr addr;

	haystack = buffer;
	needle   = haystack;
	end      = haystack + strlen(haystack) - 1;
	while (needle && haystack < end) {
		char ch;
		size_t num = 0;

		needle = strpbrk(haystack, accept);
		if (needle) {
			num = strspn(needle, accept);
			if (num) {
				ch = needle[num];
				needle[num] = 0;

				if (inet_pton(AF_INET6, needle, &addr) == 1) {
					inet_ntop(AF_INET6, &addr, address, len);
					if (is_address_valid(AF_INET6, address)) {
						found = 1;
						break;
					}
				}

				needle[num] = ch;
			}
		}

		/* nothing yet, skip to next search point */
		haystack = needle + num + 1;
	}

	return found;
}

static int parse_my_address(char *buffer, char *address, size_t len)
{
	if (parse_ipv6_address(buffer, address, len))
		return 0;

	return !parse_ipv4_address(buffer, address, len);
}

static int get_address_remote(ddns_t *ctx, ddns_info_t *info, char *address, size_t len)
{
	if (!info->server_url[0])
		return 1;

	DO(server_transaction(ctx, info));
	if (!ctx || ctx->http_transaction.rsp_len <= 0 || !ctx->http_transaction.rsp)
		return RC_INVALID_POINTER;

	logit(LOG_DEBUG, "IP server response:");
	logit(LOG_DEBUG, "%s", ctx->work_buf);

	DO(parse_my_address(ctx->http_transaction.rsp_body, address, len));

	return 0;
}

static int get_address_cmd(ddns_t *ctx, ddns_info_t *info, char *address, size_t len)
{
	if (!info->checkip_cmd || !info->checkip_cmd[0])
		return 1;

	DO(shell_transaction(ctx, info, info->checkip_cmd));
	logit(LOG_DEBUG, "Command response:");
	logit(LOG_DEBUG, "%s", ctx->work_buf);

	DO(parse_my_address(ctx->work_buf, address, len));

	return 0;
}

static int get_ipv4_address_iface(const char *ifname, char *address, size_t len)
{
	int sd, result;
	struct ifreq ifr;

	sd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sd < 0) {
		logit(LOG_WARNING, "Failed opening network socket: %s", strerror(errno));
		return 1;
	}

	logit(LOG_DEBUG, "Reading IPv4 address of %s using SIOCGIFADDR ...", ifname);
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifname);
	result = ioctl(sd, SIOCGIFADDR, &ifr);
	close(sd);

	if (result < 0) {
		logit(LOG_ERR, "Failed reading IP address of interface %s: %s", ifname, strerror(errno));
		return 1;
	}

	if (!inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, address, len))
		return 1;

	if (!is_address_valid(AF_INET, address)) {
		logit(LOG_INFO, "Interface %s has an invalid/local IP# %s", ifname, address);
		return 1;
	}

	return 0;
}

static int get_address_iface(ddns_t *ctx, const char *ifname, char *address, size_t len)
{
	char *ptr, trailer[IFNAMSIZ + 2];
	struct ifaddrs *ifaddr, *ifa;

	if (!ifname || !ifname[0])
		return 1;

	/* Trailer to strip, if set by getnameinfo() */
	snprintf(trailer, sizeof(trailer), "%%%s", ifname);

	logit(LOG_INFO, "Checking for IP# change, querying interface %s", ifname);
	if (getifaddrs(&ifaddr))
		return get_ipv4_address_iface(ifname, address, len);

	memset(ctx->work_buf, 0, ctx->work_buflen);
	for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
		int result, family;
		char host[NI_MAXHOST] = "";
		size_t pos;

		if (!ifa->ifa_addr)
			continue;

		if (!string_compare(ifa->ifa_name, ifname))
			continue;

		pos = strlen(ctx->work_buf);
		family = ifa->ifa_addr->sa_family;
		if (family == AF_INET || family == AF_INET6) {
			result = getnameinfo(ifa->ifa_addr, ((family == AF_INET)
							     ? sizeof(struct sockaddr_in)
							     : sizeof(struct sockaddr_in6)),
					     host, NI_MAXHOST,
					     NULL, 0, NI_NUMERICHOST);

			if (result) {
				logit(LOG_ERR, "getnameinfo() failed: %s", gai_strerror(result));
				continue;
			}

			ptr = strstr(host, trailer);
			if (ptr)
				*ptr = 0;

			if (!is_address_valid(family, host)) {
				logit(LOG_INFO, "Invalid/local address %s for %s, skipping ...", host, ifname);
				continue;
			}

			snprintf(&ctx->work_buf[pos], ctx->work_buflen - pos, "%s\n", host);
		}
	}

	freeifaddrs(ifaddr);
	DO(parse_my_address(ctx->work_buf, address, len));

	return 0;
}

static int get_address_backend(ddns_t *ctx, ddns_info_t *info, char *address, size_t len)
{
	char name[sizeof(info->checkip_name.name)];
	char url[sizeof(info->checkip_url)];
	int ssl, rc;

	logit(LOG_DEBUG, "Get address for %s", info->system->name);
	memset(address, 0, len);

	if (!get_address_cmd   (ctx, info, address, len))
		return 0;

	/* Check info specific interface */
	if (!get_address_iface (ctx, info->ifname, address, len))
		return 0;

	/* Check the global interface */
	if (!get_address_iface (ctx, iface, address, len))
		return 0;

	if (!get_address_remote(ctx, info, address, len))
		return 0;

	logit(LOG_WARNING, "Communication with checkip server %s failed, "
	      "run again with 'inadyn -l debug' if problem persists",
	      info->checkip_name.name);

	/* Skip fallback if it's the same server ... option: flip SSL bit? */
	if (strstr(info->checkip_name.name, DDNS_MY_IP_SERVER))
		goto error;

	logit(LOG_WARNING, "Retrying with built-in 'default', http://ifconfig.me/ip ...");

	/* keep backup, for now, future extension: count failures and replace permanently */
	strlcpy(name, info->checkip_name.name, sizeof(name));
	strlcpy(url, info->checkip_url, sizeof(url));
	ssl = info->checkip_ssl;

	/* Retry with http(s)://ifconfig.me/ip */
	strlcpy(info->checkip_name.name, DDNS_MY_IP_SERVER, sizeof(info->checkip_name.name));
	strlcpy(info->checkip_url, DDNS_MY_CHECKIP_URL, sizeof(info->checkip_url));
	info->checkip_ssl = DDNS_MY_IP_SSL;
	rc = get_address_remote(ctx, info, address, len);

	/* restore backup, for now, the official server may just have temporary problems */
	strlcpy(info->checkip_name.name, name, sizeof(info->checkip_name.name));
	strlcpy(info->checkip_url, url, sizeof(info->checkip_url));
	info->checkip_ssl = ssl;

	if (!rc) {
		logit(LOG_WARNING, "Please note, http%s://%s%s seems unstable, consider overriding it in"
		      "your configuration with 'checkip-server = default'", info->checkip_ssl ? "s" : "",
		      info->checkip_name.name, info->checkip_url);
		return 0;
	}

error:
	logit(LOG_ERR, "Failed to get IP address for %s, giving up!", info->system->name);

	return 1;
}

/*
 * Fetch IP, using any of the backends for each DDNS provider,
 * then check for address change.
 */
static int get_address(ddns_t *ctx)
{
	char address[MAX_ADDRESS_LEN];
	ddns_info_t *info;

	info = conf_info_iterator(1);
	while (info) {
		int anychange = 0;
		size_t i;

		if (get_address_backend(ctx, info, address, sizeof(address)))
			goto next;

		for (i = 0; i < info->alias_count; i++) {
			ddns_alias_t *alias = &info->alias[i];

			alias->ip_has_changed = strncmp(alias->address, address, sizeof(alias->address)) != 0;
			if (alias->ip_has_changed) {
				anychange++;
				strlcpy(alias->address, address, sizeof(alias->address));
			}

#ifdef ENABLE_SIMULATION
			logit(LOG_WARNING, "In simulation, forcing IP# change ...");
			alias->ip_has_changed = 1;
#endif
		}

		if (!anychange)
			logit(LOG_INFO, "No IP# change detected for %s, still at %s", info->system->name, address);
		else
			logit(LOG_INFO, "Current IP# %s at %s", address, info->system->name);

	next:
		info = conf_info_iterator(0);
	}

	return 0;
}

static int time_to_check(ddns_t *ctx, ddns_alias_t *alias)
{
	time_t past_time = time(NULL) - alias->last_update;

	return alias->force_addr_update ||
		(past_time > ctx->forced_update_period_sec);
}

static int check_alias_update_table(ddns_t *ctx)
{
	ddns_info_t *info;

	/* Uses fix test if ip of server 0 has changed.
	 * That should be OK even if changes check_address() to
	 * iterate over servernum, but not if it's fix set to =! 0 */
	info = conf_info_iterator(1);
	while (info) {
		size_t i;

		for (i = 0; i < info->alias_count; i++) {
			int override;
			ddns_alias_t *alias = &info->alias[i];

/* XXX: TODO time_to_check() will return false positive if the cache
 *     file is missing => causing unnecessary update.  We should save
 *     the cache file with the current IP instead and fall back to
 *     standard update interval!
 */
			override = time_to_check(ctx, alias);
			if (!alias->ip_has_changed && !override) {
				alias->update_required = 0;
				continue;
			}

			alias->update_required = 1;
			logit(LOG_NOTICE, "Update %s for alias %s, new IP# %s",
			      override ? "forced" : "needed", alias->name, alias->address);
		}

		info = conf_info_iterator(0);
	}

	return 0;
}

static int send_update(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias, int *changed)
{
	int            rc;
	http_trans_t   trans;
	http_t        *client = &info->server;

	if (info->system->setup)
		DO(info->system->setup(ctx, info, alias));

	client->ssl_enabled = info->ssl_enabled;
	rc = http_init(client, "Sending IP# update to DDNS server", strstr(info->system->name, "ipv6") ? TCP_FORCE_IPV6 : TCP_FORCE_IPV4);
	if (rc) {
		/* Update failed, force update again in ctx->cmd_check_period seconds */
		alias->force_addr_update = 1;
		return rc;
	}

	memset(ctx->work_buf, 0, ctx->work_buflen);
	memset(ctx->request_buf, 0, ctx->request_buflen);
	memset(&trans, 0, sizeof(trans));

	trans.req_len     = info->system->request(ctx, info, alias);
	trans.req         = (char *)ctx->request_buf;
	trans.rsp         = (char *)ctx->work_buf;
	trans.max_rsp_len = ctx->work_buflen - 1;	/* Save place for a \0 at the end */

	if (trans.req_len < 0) {
		logit(LOG_ERR, "Invalid HTTP GET request in %s provider, cannot update.", info->system->name);
		rc = RC_ERROR;

		goto exit;
	}

	ctx->request_buf[trans.req_len] = 0;
	logit(LOG_DEBUG, "Sending alias table update to DDNS server: %s", ctx->request_buf);

#ifdef ENABLE_SIMULATION
	logit(LOG_WARNING, "In simulation, skipping update to server ...");
	goto exit;
#endif
	rc = http_transaction(client, &trans);
	if (rc) {
		/* Update failed, force update again in ctx->cmd_check_period seconds */
		logit(LOG_WARNING, "HTTP(S) Transaction failed, error %d: %s", rc, error_str(rc));
		logit(LOG_INFO, "Update failed, forced update/retry in %d sec ...", ctx->cmd_check_period);
		alias->force_addr_update = 1;
		goto exit;
	}
	logit(LOG_DEBUG, "DDNS server response: %s", trans.rsp);

	rc = info->system->response(&trans, info, alias);
	if (rc) {
		logit(LOG_WARNING, "%s error in DDNS server response: %s",
		      rc == RC_DDNS_RSP_RETRY_LATER || rc == RC_DDNS_RSP_TOO_FREQUENT ? "Temporary" : "Fatal", error_str(rc));
//		logit(LOG_WARNING, "[%d %s]", trans.status, trans.status_desc);
		logit(LOG_DEBUG, "%s", trans.rsp_body != trans.rsp ? trans.rsp_body : "");

		/* Update failed, force update again in ctx->cmd_check_period seconds */
		alias->force_addr_update = 1;
	} else {
		logit(LOG_INFO, "Successful alias table update for %s => new IP# %s",
		      alias->name, alias->address);

		alias->force_addr_update = 0;
		if (changed)
			(*changed)++;
	}

exit:
	http_exit(client);

	return rc;
}

static int update_alias_table(ddns_t *ctx)
{
	int rc = 0, remember = 0;
	int anychange = 0;
	ddns_info_t *info;

	/* Issue #15: On external trig. force update to random addr. */
	if (ctx->forced_update_fake_addr) {
		/* If the DDNS server responds with an error, we ignore it here,
		 * since this is just to fool the DDNS server to register a a
		 * change, i.e., an active user. */
		info = conf_info_iterator(1);
		while (info) {
			size_t i;

			for (i = 0; i < info->alias_count; i++) {
				ddns_alias_t *alias = &info->alias[i];
				if (alias->force_addr_update) {
					char backup[sizeof(alias->address)];

					strlcpy(backup, alias->address, sizeof(backup));

					/* Picking random address in 203.0.113.0/24 ... */
					snprintf(alias->address, sizeof(alias->address), "203.0.113.%d", (rand() + 1) % 255);
					TRY(send_update(ctx, info, alias, NULL));

					strlcpy(alias->address, backup, sizeof(alias->address));
				}
			}

			info = conf_info_iterator(0);
		}

		/* Play nice with server, wait a bit before sending actual IP */
		sleep(3);
	}

	info = conf_info_iterator(1);
	while (info) {
		size_t i;

		for (i = 0; i < info->alias_count; i++) {
			ddns_alias_t *alias = &info->alias[i];
			char *event = "update";
			rc = 0;

			if (!alias->update_required) {
				if (exec_mode == EXEC_MODE_COMPAT)
					continue;
				event = "nochg";
			} else if ((rc = send_update(ctx, info, alias, &anychange))) {
				if (exec_mode == EXEC_MODE_COMPAT)
					break;
				event = "error";
			} else {
				/* Only reset if send_update() succeeds. */
				alias->update_required = 0;
				alias->last_update = time(NULL);

				/* Update cache file for this entry */
				write_cache_file(alias, info->system->name);
			}

			/* Run command or script on successful update. */
			if (script_exec)
				os_shell_execute(script_exec, alias->address, alias->name, event, rc);
		}

		if (RC_DDNS_RSP_NOTOK == rc || RC_DDNS_RSP_AUTH_FAIL == rc)
			remember = rc;

		if ((RC_DDNS_RSP_RETRY_LATER == rc || rc == RC_DDNS_RSP_TOO_FREQUENT) && !remember)
			remember = rc;

		info = conf_info_iterator(0);
	}

	return remember;
}

static int get_encoded_user_passwd(void)
{
	int rc = 0;
	char *buf = NULL;
	size_t len;
	ddns_info_t *info;

	/* Take base64 encoding into account when allocating buf */
	len = sizeof(info->creds.password) + sizeof(info->creds.username) + 2;
	len = (len / 3 + ((len % 3) ? 1 : 0)) * 4; /* output length = 4 * [input len / 3] */

	buf = calloc(len, sizeof(char));
	if (!buf)
		return RC_OUT_OF_MEMORY;

	info = conf_info_iterator(1);
	while (info) {
		int rc2;
		char *encode;
		size_t dlen = 0;

		info->creds.encoded = 0;

		/*
		 * Concatenate username and password with a ':', without
		 * snprintf(), since that can cause information loss if
		 * the password has "\=" or similar in it, issue #57
		 */
		strlcpy(buf, info->creds.username, len);
		strlcat(buf, ":", len);
		strlcat(buf, info->creds.password, len);

		/* query required buffer size for base64 encoded data */
		base64_encode(NULL, &dlen, (unsigned char *)buf, strlen(buf));

		encode = malloc(dlen);
		if (!encode) {
			logit(LOG_WARNING, "Out of memory base64 encoding user:pass for %s!", info->system->name);
			rc = RC_OUT_OF_MEMORY;
			break;
		}

//		logit(LOG_DEBUG, "Base64 encode %s for %s ...", buf, info->system->name);
		rc2 = base64_encode((unsigned char *)encode, &dlen, (unsigned char *)buf, strlen(buf));
		if (rc2) {
			logit(LOG_WARNING, "Failed base64 encoding user:pass for %s!", info->system->name);
			free(encode);
			rc = RC_BUFFER_OVERFLOW;
			break;
		}

//		logit(LOG_DEBUG, "Base64 encoded string: %s", encode);
		info->creds.encoded_password = encode;
		info->creds.encoded = 1;
		info->creds.size = strlen(info->creds.encoded_password);

		info = conf_info_iterator(0);
	}

	memset(buf, 0, len);
	free(buf);

	return rc;
}

static int init_context(ddns_t *ctx)
{
	ddns_info_t *info;
	struct timeval tv;

	if (!ctx)
		return RC_INVALID_POINTER;

	if (ctx->initialized == 1)
		return 0;

	/* Seed prng, used in silly IP randomizer */
	gettimeofday(&tv, NULL);
	srand((unsigned int)tv.tv_usec);

	info = conf_info_iterator(1);
	while (info) {
		http_t *checkip = &info->checkip;
		http_t *update  = &info->server;

		if (strlen(info->proxy_name.name)) {
			http_set_port(checkip, info->proxy_name.port);
			http_set_port(update,  info->proxy_name.port);

			http_set_remote_name(checkip, info->proxy_name.name);
			http_set_remote_name(update,  info->proxy_name.name);
		} else {
			http_set_port(checkip, info->checkip_name.port);
			http_set_port(update,  info->server_name.port);

			http_set_remote_name(checkip, info->checkip_name.name);
			http_set_remote_name(update,  info->server_name.name);
		}

		info = conf_info_iterator(0);
	}

	/* Restore values, if reset by SIGHUP.  Initialize time from cache file at startup. */
	ctx->num_iterations = cached_num_iterations;

	ctx->initialized = 1;

	return 0;
}

static int check_address(ddns_t *ctx)
{
	if (!ctx)
		return RC_INVALID_POINTER;

	/* Get IP address from any of the different backends */
	DO(get_address(ctx));

	/* Step through aliases list, resolve them and check if they point to my IP */
	DO(check_alias_update_table(ctx));

	/* Update IPs marked as not identical with my IP */
	DO(update_alias_table(ctx));

	return 0;
}

/*
 * Error filter.  Some errors are to be expected in a network
 * application, some we can recover from, wait a shorter while and try
 * again, whereas others are terminal, e.g., some OS errors.
 */
static int check_error(ddns_t *ctx, int rc)
{
	const char *errstr = "Error response from DDNS server";

	switch (rc) {
	case RC_OK:
		ctx->update_period = ctx->normal_update_period_sec;
		break;

	/* dyn_dns_update_ip() failed, inform the user the (network) error
	 * is not fatal and that we will retry again in a short while. */
	case RC_TCP_INVALID_REMOTE_ADDR: /* Probably temporary DNS error. */
	case RC_TCP_CONNECT_FAILED:      /* Cannot connect to DDNS server atm. */
	case RC_TCP_SEND_ERROR:
	case RC_TCP_RECV_ERROR:
	case RC_OS_INVALID_IP_ADDRESS:
	case RC_DDNS_RSP_RETRY_LATER:
	case RC_DDNS_INVALID_CHECKIP_RSP:
	case RC_DDNS_RSP_TOO_FREQUENT:
		ctx->update_period = ctx->error_update_period_sec;
		logit(LOG_WARNING, "Will retry again in %d sec ...", ctx->update_period);
		break;

	case RC_DDNS_RSP_NOTOK:
	case RC_DDNS_RSP_AUTH_FAIL:
		if (ignore_errors) {
			logit(LOG_WARNING, "%s, ignoring ...", errstr);
			break;
		}
		logit(LOG_ERR, "%s, exiting!", errstr);
		return 1;

	/* All other errors, socket creation failures, invalid pointers etc.  */
	default:
		logit(LOG_ERR, "Unrecoverable error %d, exiting ...", rc);
		return 1;
	}

	return 0;
}

int ddns_main_loop(ddns_t *ctx)
{
	int rc = 0;
	ddns_info_t *info;
	static int first_startup = 1;

	if (!ctx)
		return RC_INVALID_POINTER;

	/* On first startup only, optionally wait for network and any NTP daemon
	 * to set system time correctly.  Intended for devices without battery
	 * backed real time clocks as initialization of time since last update
	 * requires the correct time.  Sleep can be interrupted with the usual
	 * signals inadyn responds too. */
	if (first_startup && startup_delay) {
		logit(LOG_NOTICE, "Startup delay: %d sec ...", startup_delay);
		first_startup = 0;

		/* Now sleep a while. Using the time set in update_period data member */
		ctx->update_period = startup_delay;
		wait_for_cmd(ctx);

		if (ctx->cmd == CMD_STOP) {
			logit(LOG_NOTICE, "STOP command received, exiting.");
			return 0;
		}
		if (ctx->cmd == CMD_RESTART) {
			logit(LOG_INFO, "RESTART command received, restarting.");
			return RC_RESTART;
		}
		if (ctx->cmd == CMD_FORCED_UPDATE) {
			logit(LOG_INFO, "FORCED_UPDATE command received, updating now.");
			info = conf_info_iterator(1);
			while (info) {
				size_t i;
				for (i = 0; i < info->alias_count; i++) {
					ddns_alias_t *alias = &info->alias[i];
					alias->force_addr_update = 1;
				}
				info = conf_info_iterator(0);
			}
			ctx->cmd = NO_CMD;
		} else if (ctx->cmd == CMD_CHECK_NOW) {
			logit(LOG_INFO, "CHECK_NOW command received, leaving startup delay.");
			ctx->cmd = NO_CMD;
		}
	}

	DO(init_context(ctx));
	DO(read_cache_file(ctx));
	DO(get_encoded_user_passwd());

	if (once && force) {
			info = conf_info_iterator(1);
			while (info) {
				size_t i;
				for (i = 0; i < info->alias_count; i++) {
					ddns_alias_t *alias = &info->alias[i];
					alias->force_addr_update = 1;
				}
				info = conf_info_iterator(0);
			}
	}
	/* Initialization done, create pidfile to indicate we are ready to communicate */
	if (once == 0 && pidfile_name[0] && pidfile(pidfile_name))
		logit(LOG_WARNING, "Failed creating pidfile: %s", strerror(errno));

	/* DDNS client main loop */
	while (1) {
		rc = check_address(ctx);
		if (RC_OK == rc) {
			if (ctx->total_iterations != 0 &&
			    ++ctx->num_iterations >= ctx->total_iterations)
				break;
		}

		if (ctx->cmd == CMD_RESTART) {
			logit(LOG_INFO, "RESTART command received. Restarting.");
			ctx->cmd = NO_CMD;
			rc = RC_RESTART;
			break;
		}

		/* On error, check why, possibly need to retry sooner ... */
		if (check_error(ctx, rc))
			break;

		/* Now sleep a while. Using the time set in update_period data member */
		wait_for_cmd(ctx);

		if (ctx->cmd == CMD_STOP) {
			logit(LOG_NOTICE, "STOP command received, exiting.");
			rc = 0;
			break;
		}
		if (ctx->cmd == CMD_RESTART) {
			logit(LOG_INFO, "RESTART command received, restarting.");
			rc = RC_RESTART;
			break;
		}
		if (ctx->cmd == CMD_FORCED_UPDATE) {
			logit(LOG_INFO, "FORCED_UPDATE command received, updating now.");

			info = conf_info_iterator(1);
			while (info) {
				size_t i;
				for (i = 0; i < info->alias_count; i++) {
					ddns_alias_t *alias = &info->alias[i];
					alias->force_addr_update = 1;
				}
				info = conf_info_iterator(0);
			}
			ctx->cmd = NO_CMD;
			continue;
		}

		if (ctx->cmd == CMD_CHECK_NOW) {
			logit(LOG_INFO, "CHECK_NOW command received, checking ...");
			ctx->cmd = NO_CMD;
			continue;
		}
	}

	/* Save old value, if restarted by SIGHUP */
	cached_num_iterations = ctx->num_iterations;

	return rc;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
