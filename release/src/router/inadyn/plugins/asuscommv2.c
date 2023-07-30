/* Plugin for asuscomm.com DDNS
 *
 * Copyright (C) 2012 Vladislav Grishenko <themiron@mail.ru>
 * Copyright (C) 2014 Andy Padavan <andy.padavan@gmail.com>
 * Copyright (C) 2018 Eric Sauvageau <rmerl@lostrealm.ca>
 * Based on Asus's ez-ipupdate implementation
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

//#include "md5.h"
#include "base64.h"
#include "plugin.h"

#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <openssl/md5.h>

#ifdef ASUSWRT
#include <bcmnvram.h>
#endif

#define ASUSDDNS_IP_SERVER	"ns1.asuscomm.com"
#define ASUSDDNS_IP_SERVER_CN	"ns1.asuscomm.cn"
//#define ASUSDDNS_IP_SERVER	"52.250.15.7"
#define ASUSDDNS_CHECKIP_URL	"/myip.php"

#if defined(ASUSWRT) && defined(ASUSWRT_LE)
#define ASUSDDNS_ARGS "%s%s"
#elif defined(ASUSWRT)
#define ASUSDDNS_ARGS "%s"
#else
#define ASUSDDNS_ARGS ""
#endif

#define ASUSDDNS_IP_HTTP_REQUEST					\
	"GET %s?"							\
	"hostname=%s&"							\
	ASUSDDNS_ARGS

#define ASUSDDNS_IP_HTTP_REQUEST_MYIP		\
	"myip=%s&"

#if defined(USE_IPV6) && defined(ASUSWRT)
#define ASUSDDNS_IP_HTTP_REQUEST_MYIPV6		\
	"myipv6=%s&"
#endif

#define ASUSDDNS_IP_HTTP_REQUEST_2		\
	"cusid=%s&"							\
	"ddnstoken=%s&"						\
	"model=%s&"							\
	"fw_ver=%s "							\
	"HTTP/1.0\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n\r\n"

#define ASUSDDNS_UNREG_HTTP_REQUEST					\
	"GET %s?"							\
	"hostname=%s&"							\
	"action=unregister "						\
	"HTTP/1.0\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n\r\n"

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias);
static int request_unregister(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias);
static int response_update(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);
static int response_register(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t asus_update = {
	.name         = "updatev2@asus.com",

	.setup = NULL,
	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response_update,

	.checkip_name = ASUSDDNS_IP_SERVER,
	.checkip_url  = ASUSDDNS_CHECKIP_URL,

	.server_name  = ASUSDDNS_IP_SERVER,
	.server_url   = "/ddnsv2/update.jsp"
};

static ddns_system_t asus_register = {
	.name         = "registerv2@asus.com",

	.setup = NULL,
	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response_register,

	.checkip_name = ASUSDDNS_IP_SERVER,
	.checkip_url  = ASUSDDNS_CHECKIP_URL,

	.server_name  = ASUSDDNS_IP_SERVER,
	.server_url   = "/ddnsv2/register.jsp"
};

static ddns_system_t asus_unregister = {
	.name         = "unregisterv2@asus.com",

	.setup = NULL,
	.request      = (req_fn_t)request_unregister,
	.response     = (rsp_fn_t)response_register,

	.checkip_name = ASUSDDNS_IP_SERVER,
	.checkip_url  = ASUSDDNS_CHECKIP_URL,

	.server_name  = ASUSDDNS_IP_SERVER,
	.server_url   = "/ddnsv2/register.jsp"
};

#define MD5_DIGEST_BYTES 16
static void hmac_md5( const unsigned char *input, size_t ilen, unsigned char *output)
{
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, input, ilen);
	MD5_Final(output, &ctx);
}

#ifdef ASUSWRT
static int get_transfer_macaddr(char *buf, size_t size)
{
	unsigned char ea[ETH_ALEN], s = 0;
	char *c = nvram_safe_get("ddns_transfer");
	int i = 0;

	for (;;) {
		unsigned char e = (unsigned char) strtoul(c, &c, 16);
		s |= e;
		ea[i++] = e;
		if (!*c++ || i == ETH_ALEN)
			break;
	}

	if (i != ETH_ALEN || s == 0 || (ea[0] & 3) != 0)
		return 0;

	snprintf(buf, size, "%02X%02X%02X%02X%02X%02X",
		ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);
	return 1;
}

#ifdef ASUSWRT_LE
static int get_acme_challenge(char *buf, size_t size)
{
	int fd, n;

	if (!nvram_match("le_enable", "1"))
		return 0;

	if ((fd = open("/tmp/acme.txt", O_RDONLY)) < 0)
		return 0;
	
	n = read(fd, buf, size - 1);
	close(fd);

	buf[n < 0 ? 0 : n] = '\0';
	*strchrnul(buf, '\n') = '\0';

	return *buf ? 1 : 0;
}
#endif
#endif

static void make_request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	unsigned char digest[MD5_DIGEST_BYTES];
	char auth[ETH_ALEN*2+1+MD5_DIGEST_BYTES*2+1];
	char *p_tmp, *p_auth = auth;
	size_t dlen = 0;
	int i;

	/* prepare username (MAC) */
	p_tmp = info->creds.username;
	for (i = 0; i < ETH_ALEN*2; i++) {
		while (*p_tmp && !isxdigit(*p_tmp))
			p_tmp++;
		*p_auth++ = *p_tmp ? toupper(*p_tmp++) : '0';
	}

	/* split username and password */
	*p_auth++ = ':';
	hmac_md5((unsigned char *)info->creds.username, strlen(info->creds.username), digest);
	for (i = 0; i < MD5_DIGEST_BYTES; i++)
		p_auth += sprintf(p_auth, "%02x", digest[i]);

	/*encode*/
	base64_encode(NULL, &dlen, (unsigned char *)auth, strlen(auth));
	p_tmp = malloc(dlen);
	if (p_tmp)
		base64_encode((unsigned char *)p_tmp, &dlen, (unsigned char *)auth, strlen(auth));

	if (info->creds.encoded_password)
		free(info->creds.encoded_password);

	info->creds.encoded_password = p_tmp;
	info->creds.encoded = p_tmp ? 1 : 0;
	info->creds.size = p_tmp ? strlen(p_tmp) : 0;
}

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
#ifdef ASUSWRT
	char ret_buf[64];
	char oldmac_arg[sizeof("oldmac=&")+ETH_ALEN*2] = "";
#ifdef ASUSWRT_LE
	char acme_arg[sizeof("acme_challenge=1&txtdata=&")+64] = "";
#endif
#endif
	char fwver[32];

	logit(LOG_WARNING, "alias address=<%s>", alias->address);
#if defined(USE_IPV6) && defined(ASUSWRT)
	if(alias->ipv6_address[0] != '\0')
		logit(LOG_WARNING, "%s ipv6 address=<%s>", iface, alias->ipv6_address);
#endif

	make_request(ctx, info, alias);

#ifdef ASUSWRT
	if (get_transfer_macaddr(ret_buf, sizeof(ret_buf)))
		snprintf(oldmac_arg, sizeof(oldmac_arg), "oldmac=%s&", ret_buf);
#ifdef ASUSWRT_LE
	if (get_acme_challenge(ret_buf, sizeof(ret_buf)))
		snprintf(acme_arg, sizeof(acme_arg), "acme_challenge=1&txtdata=%s&", ret_buf);
#endif
#endif

	 snprintf(fwver, sizeof(fwver), "%s.%s_%s", nvram_safe_get("firmver"), nvram_safe_get("buildno"), nvram_safe_get("extendno"));
#if 0
	return snprintf(ctx->request_buf, ctx->request_buflen,
			ASUSDDNS_IP_HTTP_REQUEST,
			info->server_url,
			alias->name,
#ifdef ASUSWRT
			oldmac_arg,
#ifdef ASUSWRT_LE
			acme_arg,
#endif
#endif
			alias->address,
			nvram_safe_get("productid"),
			fwver,
			info->creds.encoded_password ? : "",
			info->server_name.name,
			info->user_agent);
#else
	snprintf(ctx->request_buf, ctx->request_buflen,
			ASUSDDNS_IP_HTTP_REQUEST,
			info->server_url,
			alias->name
#ifdef ASUSWRT
			,oldmac_arg
#ifdef ASUSWRT_LE
			,acme_arg
#endif
#endif
		);
	snprintf(ctx->request_buf + strlen(ctx->request_buf), ctx->request_buflen - strlen(ctx->request_buf),
			ASUSDDNS_IP_HTTP_REQUEST_MYIP,
			alias->address
		);
#if defined(USE_IPV6) && defined(ASUSWRT)
	if(alias->ipv6_address[0] != '\0')
	{
		snprintf(ctx->request_buf + strlen(ctx->request_buf), ctx->request_buflen - strlen(ctx->request_buf),
				ASUSDDNS_IP_HTTP_REQUEST_MYIPV6,
				alias->ipv6_address
			);
	}
#endif
	snprintf(ctx->request_buf + strlen(ctx->request_buf), ctx->request_buflen - strlen(ctx->request_buf),
			ASUSDDNS_IP_HTTP_REQUEST_2,
			nvram_safe_get("oauth_dm_cusid"),
			nvram_safe_get("asusddns_token"),
			nvram_safe_get("productid"),
			fwver,
			info->creds.encoded_password ? : "",
			info->server_name.name,
			info->user_agent);
	logit(LOG_WARNING, "request<%s>", ctx->request_buf);
	return strlen(ctx->request_buf);
#endif
}

static int request_unregister(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	logit(LOG_WARNING, "do request_unregister");
	make_request(ctx, info, alias);
	return snprintf(ctx->request_buf, ctx->request_buflen,
			ASUSDDNS_UNREG_HTTP_REQUEST,
			info->server_url,
			alias->name,
			info->creds.encoded_password ? : "",
			info->server_name.name,
			info->user_agent);
}

static int response_update(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *p, *p_rsp;
	char domain[256] = {0};
#ifdef ASUSWRT
	char ret_buf[64];
#endif
	p_rsp = trans->rsp_body;

	if(trans->rsp)
		logit(LOG_WARNING, "[%s]%s", __FUNCTION__, trans->rsp);

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

#ifdef ASUSWRT
	snprintf(ret_buf, sizeof(ret_buf), "%s,%d", "", trans->status);
	nvram_set("ddns_return_code", ret_buf);
	nvram_set("ddns_return_code_chk", ret_buf);
#endif

	switch (trans->status) {
	case 200:		/* update success */
	case 220:		/* update same domain success -- unused?? */
		return RC_OK;
	case 203:		/* update/reg/unreg failed */
		logit(LOG_WARNING, "Domain already in use, suggested domain '%s'", domain);
#ifdef ASUSWRT
		nvram_set("ddns_suggest_name", domain);
#endif
		return RC_DDNS_RSP_DOMAIN_IN_USE_REG;
	case 230:
		logit(LOG_WARNING, "New domain update success, old domain '%s'", domain);
#ifdef ASUSWRT
		nvram_set("ddns_old_name", domain);
#endif
		return RC_OK;
	case 233:		/* update failed */
		logit(LOG_WARNING, "Domain already in use, current domain '%s'", domain);
#ifdef ASUSWRT
		nvram_set("ddns_old_name", domain);
#endif
		return RC_DDNS_RSP_DOMAIN_IN_USE_UPDATE;
	case 297:		/* invalid hostname */
		logit(LOG_WARNING, "Invalid hostname");
		return RC_DDNS_INVALID_HOSTNAME;
	case 298:		/* invalid domain name */
		logit(LOG_WARNING, "Invalid domain name");
		return RC_DDNS_INVALID_DOMAIN_NAME;
	case 299:		/* invalid ip format */
		logit(LOG_WARNING, "Invalid IP address");
		return RC_DDNS_INVALID_IP;
	case 401:		/* authentication failure */
		logit(LOG_WARNING, "Authentication failure");
		return RC_DDNS_RSP_AUTH_FAIL;
	case 402:
		logit(LOG_WARNING, "Registration blocked");
		return RC_DDNS_RSP_REG_BLOCK;
	case 407:		/* proxy authentication required */
		logit(LOG_WARNING, "Proxy authenticatio blocked");
		return RC_DDNS_RSP_PROXY_AUTH_REQ;
	}

	if (trans->status >= 500 && trans->status < 600)
		return RC_DDNS_RSP_RETRY_LATER;

	return RC_DDNS_RSP_NOTOK;
}

static int response_register(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *p, *p_rsp;
	char domain[256] = {0};
#ifdef ASUSWRT
	char ret_buf[64];
#endif

	p_rsp = trans->rsp_body;

	if(trans->rsp)
		logit(LOG_WARNING, "[%s]%s", __FUNCTION__, trans->rsp);

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

#ifdef ASUSWRT
	if (info->system == &asus_unregister) {
		snprintf(ret_buf, sizeof(ret_buf), "%s,%d", "unregister", trans->status);
		nvram_set("asusddns_reg_result", ret_buf);
		if(trans->status == 200) {
			nvram_set("ddns_enable_x", "0");
			nvram_set("ddns_server_x", "");
			nvram_set("ddns_server_x_old", "");
			nvram_set("ddns_hostname_x", "");
			nvram_set("ddns_hostname_old", "");
			nvram_set("ddns_cache", "");
			nvram_set("ddns_ipaddr", "");
#ifdef RTCONFIG_IPV6
			nvram_set("ddns_ipv6_ipaddr", "");
#endif
			nvram_commit();
		}
	} else {
		snprintf(ret_buf, sizeof(ret_buf), "%s,%d", "register", trans->status);
		nvram_set("ddns_return_code", ret_buf);
		nvram_set("ddns_return_code_chk", ret_buf);
	}
#endif

	switch (trans->status) {
	case 200:		/* registration success */
	case 220:		/* registration same domain success*/
		return RC_OK;
	case 203:		/* registration failed */
		logit(LOG_WARNING, "Domain already in use, suggested domain '%s'", domain);
#ifdef ASUSWRT
		nvram_set("ddns_suggest_name", domain);
#endif
		return RC_DDNS_RSP_DOMAIN_IN_USE_REG;
	case 230:		/* registration new domain success */
		logit(LOG_WARNING, "Registration success, previous domain '%s'", domain);
#ifdef ASUSWRT
		nvram_set("ddns_old_name", domain);
#endif
		return RC_OK;
	case 233:		/* registration failed */
		logit(LOG_WARNING, "Domain already in use, current domain '%s'", domain);
#ifdef ASUSWRT
		nvram_set("ddns_old_name", domain);
#endif
		return RC_DDNS_RSP_DOMAIN_IN_USE_UPDATE;
	case 297:		/* invalid hostname */
		logit(LOG_WARNING, "Invalid hostname");
		return RC_DDNS_INVALID_HOSTNAME;
	case 298:		/* invalid domain name */
		logit(LOG_WARNING, "Invalid domain name");
		return RC_DDNS_INVALID_DOMAIN_NAME;
	case 299:		/* invalid ip format */
		logit(LOG_WARNING, "Invalid IP address");
		return RC_DDNS_INVALID_IP;
	case 401:		/* authentication failure */
		logit(LOG_WARNING, "Authentication failure");
		return RC_DDNS_RSP_AUTH_FAIL;
	case 402:
		logit(LOG_WARNING, "Registration blocked");
		return RC_DDNS_RSP_REG_BLOCK;
	case 407:		/* proxy authentication required */
		logit(LOG_WARNING, "Proxy authenticatio blocked");
		return RC_DDNS_RSP_PROXY_AUTH_REQ;
	}

	return RC_DDNS_RSP_NOTOK;
}

#ifdef RTCONFIG_ACCOUNT_BINDING
static char ddns_server[64] = {0};
#endif
PLUGIN_INIT(plugin_init)
{
#ifdef RTCONFIG_ACCOUNT_BINDING
	if (is_account_bound() && nvram_match("ddns_replace_status", "1") &&
		((strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.com") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.com")))
		|| (strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.cn") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.cn"))))) {
		snprintf(ddns_server, sizeof(ddns_server), "%s", nvram_safe_get("aae_ddnsinfo"));
		if(strlen(ddns_server) > 0) {
			asus_update.checkip_name = ddns_server;
			asus_update.server_name = ddns_server;
			asus_register.checkip_name = ddns_server;
			asus_register.server_name = ddns_server;
			asus_unregister.checkip_name = ddns_server;
			asus_unregister.server_name = ddns_server;
		}
	} else
#endif
	if (nvram_match("ddns_server_x", "WWW.ASUS.COM.CN")) {
		asus_update.checkip_name = ASUSDDNS_IP_SERVER_CN;
		asus_update.server_name = ASUSDDNS_IP_SERVER_CN;
		asus_register.checkip_name = ASUSDDNS_IP_SERVER_CN;
		asus_register.server_name = ASUSDDNS_IP_SERVER_CN;
		asus_unregister.checkip_name = ASUSDDNS_IP_SERVER_CN;
		asus_unregister.server_name = ASUSDDNS_IP_SERVER_CN;
	}
	plugin_register(&asus_update);
	plugin_register(&asus_register);
	plugin_register(&asus_unregister);
}

PLUGIN_EXIT(plugin_exit)
{
#ifdef RTCONFIG_ACCOUNT_BINDING
	if (is_account_bound() && nvram_match("ddns_replace_status", "1") &&
		((strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.com") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.com")))
		|| (strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.cn") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.cn"))))) {
		snprintf(ddns_server, sizeof(ddns_server), "%s", nvram_safe_get("aae_ddnsinfo"));
		if(strlen(ddns_server) > 0) {
			asus_update.checkip_name = ddns_server;
			asus_update.server_name = ddns_server;
			asus_register.checkip_name = ddns_server;
			asus_register.server_name = ddns_server;
			asus_unregister.checkip_name = ddns_server;
			asus_unregister.server_name = ddns_server;
		}
	} else
#endif
	if (nvram_match("ddns_server_x", "WWW.ASUS.COM.CN")) {
		asus_update.checkip_name = ASUSDDNS_IP_SERVER_CN;
		asus_update.server_name = ASUSDDNS_IP_SERVER_CN;
		asus_register.checkip_name = ASUSDDNS_IP_SERVER_CN;
		asus_register.server_name = ASUSDDNS_IP_SERVER_CN;
		asus_unregister.checkip_name = ASUSDDNS_IP_SERVER_CN;
		asus_unregister.server_name = ASUSDDNS_IP_SERVER_CN;
	}
	plugin_unregister(&asus_update);
	plugin_unregister(&asus_register);
	plugin_unregister(&asus_unregister);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */

