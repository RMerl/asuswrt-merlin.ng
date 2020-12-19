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

#include "md5.h"
#include "base64.h"
#include "plugin.h"

#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>

#ifdef ASUSWRT
#include <bcmnvram.h>
#endif

#define ASUSDDNS_IP_SERVER	"ns1.asuscomm.com"
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

#ifdef USE_IPV6
#define ASUSDDNS_IP_HTTP_REQUEST_MYIPV6		\
	"myipv6=%s&"
#endif

#define ASUSDDNS_IP_HTTP_REQUEST_2		\
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
	.name         = "update@asus.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response_update,

	.checkip_name = ASUSDDNS_IP_SERVER,
	.checkip_url  = ASUSDDNS_CHECKIP_URL,

	.server_name  = ASUSDDNS_IP_SERVER,
	.server_url   = "/ddns/update.jsp"
};

static ddns_system_t asus_register = {
	.name         = "register@asus.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response_register,

	.checkip_name = ASUSDDNS_IP_SERVER,
	.checkip_url  = ASUSDDNS_CHECKIP_URL,

	.server_name  = ASUSDDNS_IP_SERVER,
	.server_url   = "/ddns/register.jsp"
};

static ddns_system_t asus_unregister = {
	.name         = "unregister@asus.com",

	.request      = (req_fn_t)request_unregister,
	.response     = (rsp_fn_t)response_register,

	.checkip_name = ASUSDDNS_IP_SERVER,
	.checkip_url  = ASUSDDNS_CHECKIP_URL,

	.server_name  = ASUSDDNS_IP_SERVER,
	.server_url   = "/ddns/register.jsp"
};

#ifdef USE_IPV6
#define IPV6_ADDR_GLOBAL        0x0000U
static char * _get_ipv6_addr(const char *ifname, char *ipv6addr, const size_t len)
{
	FILE *f;
	int ret = -1, scope, prefix;
	unsigned char ipv6[16];
	char dname[IFNAMSIZ], address[INET6_ADDRSTRLEN];

	if(!ifname || !ipv6addr)
		return ret;

	f = fopen("/proc/net/if_inet6", "r");
	if(!f)
		return ret;

	while (19 == fscanf(f,
                        " %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx %*x %x %x %*x %s",
                        &ipv6[0], &ipv6[1], &ipv6[2], &ipv6[3], &ipv6[4], &ipv6[5], &ipv6[6], &ipv6[7], &ipv6[8], &ipv6[9], &ipv6[10], 
                        &ipv6[11], &ipv6[12], &ipv6[13], &ipv6[14], &ipv6[15], &prefix, &scope, dname))
	{
		if(strcmp(ifname, dname))
		{
			continue;
		}

		if(inet_ntop(AF_INET6, ipv6, address, sizeof(address)) == NULL)
		{
			continue;
	       }

		if(scope == IPV6_ADDR_GLOBAL)
		{
			strlcpy(ipv6addr, address, len);
			ret =0;
			break;
		}
	}

	fclose(f);
	return ret;
}
#endif

#define MD5_DIGEST_BYTES 16
static void
hmac_md5( const unsigned char *input, size_t ilen, const unsigned char *key, size_t klen, unsigned char output[MD5_DIGEST_BYTES] )
{
	int i;
	md5_context ctx;
	unsigned char k_ipad[64], k_opad[64], tk[MD5_DIGEST_BYTES];

	/* if key is longer than 64 bytes reset it to key=MD5(key) */
	if (klen > 64) {
		md5(key, klen, tk);
		key = tk;
		klen = MD5_DIGEST_BYTES;
	}

	/* start out by storing key in pads */
	memset(k_ipad, 0, sizeof(k_ipad));
	memset(k_opad, 0, sizeof(k_opad));
	memcpy(k_ipad, key, klen);
	memcpy(k_opad, key, klen);

	/*xor key with ipad and opad values */
	for (i = 0; i < 64; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/* inner MD5 */
	md5_starts( &ctx );
	md5_update( &ctx, k_ipad, 64 );
	md5_update( &ctx, input, ilen );
	md5_finish( &ctx, output );

	/* outter MD5 */
	md5_starts( &ctx );
	md5_update( &ctx, k_opad, 64 );
	md5_update( &ctx, output, MD5_DIGEST_BYTES );
	md5_finish( &ctx, output );

	memset( &ctx, 0, sizeof( md5_context ) );
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

	/* prepare password, reuse request_buf */
	p_tmp = alias->name;
	for (i = 0; i < ctx->request_buflen - 1 && *p_tmp; p_tmp++) {
		if (isalnum(*p_tmp))
			ctx->request_buf[i++] = *p_tmp;
	}
	p_tmp = alias->address;
	for (; i < ctx->request_buflen - 1 && *p_tmp; p_tmp++) {
		if (isalnum(*p_tmp))
			ctx->request_buf[i++] = *p_tmp;
	}
	ctx->request_buf[i] = '\0';
	hmac_md5((unsigned char *)ctx->request_buf, strlen(ctx->request_buf),
		(unsigned char *)info->creds.password, strlen(info->creds.password), digest);
	for (i = 0; i < MD5_DIGEST_BYTES; i++)
		p_auth += sprintf(p_auth, "%02X", digest[i]);

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
#ifdef USE_IPV6
	char ip6_addr[INET6_ADDRSTRLEN] = {0};

	if(!_get_ipv6_addr(iface, ip6_addr, sizeof(ip6_addr)))
			logit(LOG_WARNING, "%s ipv6 address=<%s>", iface, ip6_addr);
#endif

	logit(LOG_WARNING, "alias address=<%s>", alias->address);

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
#ifdef USE_IPV6
	if(ip6_addr[0] != '\0')
	{
		snprintf(ctx->request_buf + strlen(ctx->request_buf), ctx->request_buflen - strlen(ctx->request_buf),
				ASUSDDNS_IP_HTTP_REQUEST_MYIPV6,
				ip6_addr
			);
	}
#endif
	snprintf(ctx->request_buf + strlen(ctx->request_buf), ctx->request_buflen - strlen(ctx->request_buf),
			ASUSDDNS_IP_HTTP_REQUEST_2,
			nvram_safe_get("productid"),
			fwver,
			info->creds.encoded_password ? : "",
			info->server_name.name,
			info->user_agent);
	//logit(LOG_WARNING, "request<%s>", ctx->request_buf);
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

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

#ifdef ASUSWRT
	if (info->system == &asus_unregister) {
		snprintf(ret_buf, sizeof(ret_buf), "%s,%d", "unregister", trans->status);
		nvram_set("asusddns_reg_result", ret_buf);
		if(trans->status == 200)
			nvram_set("ddns_enable_x", "0");
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

PLUGIN_INIT(plugin_init)
{
	plugin_register(&asus_update);
	plugin_register(&asus_register);
	plugin_register(&asus_unregister);
}

PLUGIN_EXIT(plugin_exit)
{
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
