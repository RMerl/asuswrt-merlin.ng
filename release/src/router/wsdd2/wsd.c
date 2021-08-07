/*
   WSDD - Web Service Dynamic Discovery protocol server

   WSD protocol handler

	Copyright (c) 2016 NETGEAR
	Copyright (c) 2016 Hiro Sugawara

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   This code is based on the wsdd implementation for Samba by
   community contributers.

   The original copyright comment follows.
 */

/*
   Unix SMB/CIFS implementation.

   Web Services for Devices (WSD) helper functions

   https://msdn.microsoft.com/library/windows/desktop/aa826001(v=vs.85).aspx
   https://msdn.microsoft.com/library/windows/hardware/jj123472.aspx

   Copyright (C) Tobias Waldvogel 2013
   Copyright (C) Jose M. Prieto 2015

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE // asprintf()

#include "wsdd.h" // struct endpoint, DEBUG()
#include "wsd.h" // struct wsd_req_info, WSD_ACTION_HELLO

#include <stdbool.h> // bool
#include <stdio.h> // FILE, fopen(), fscanf(), snprintf(), asprintf()
#include <stdlib.h> // srand48()
#include <unistd.h> // usleep()
#include <string.h> // strcmp(), strdup()
#include <ctype.h> // isdigit(), isspace()
#include <time.h> // time_t, time()
#include <errno.h> // errno
#include <sys/socket.h> // sendto()
#include <arpa/inet.h> // inet_ntop()

#define UUIDLEN	37

static time_t wsd_instance;
static char wsd_sequence[UUIDLEN], wsd_endpoint[UUIDLEN];

static void set_seed(void)
{
	FILE *fp = fopen("/etc/machine-id", "r");
	unsigned long seed;

	time((time_t *)&seed);

	if (fp) {
		unsigned long s;

		while (fscanf(fp, "%8lx", &s) > 0)
			seed ^= s;
		fclose(fp);
	}
	srand48(seed);
}

static void uuid_random(char *uuid, size_t len)
{
	static bool seeded = false;

	if (!seeded) {
		set_seed();
		seeded = true;
	}

	snprintf(uuid, len, "%08lx-%04lx-%04lx-%04lx-%08lx%04lx",
			mrand48() & 0xffffffff,
			mrand48() & 0xffff,
			mrand48() & 0xffff,
			mrand48() & 0xffff,
			mrand48() & 0xffffffff,
			mrand48() & 0xffff);
}

static void uuid_endpoint(char uuid[UUIDLEN])
{
	FILE *fp = fopen("/etc/machine-id", "r");
	int c, i = 0;

	if (!fp) {
		fp = fopen("/proc/sys/kernel/random/boot_id", "r");
	}

	if (!fp) {
		DEBUG(0, W, "Can't open required '/etc/machine-id' or '/proc/sys/kernel/random/boot_id'");
		return;
	}

	while (i < 36 && (c = getc(fp)) != EOF &&
		((c == '-') || isdigit(c) || (islower(c) && isxdigit(c)))) {
		if ((c != '-') && (i == 8 || i == 13 || i == 18 || i == 23))
			uuid[i++] = '-';
		uuid[i++] = c;
	}
	fclose(fp);

	if (i == 36) {
		uuid[i] = '\0';
	} else {
		uuid[0] = '\0';
		errno = EINVAL;
	}
}

static struct {
	const char *key, *_default;
	char *value;
} bootinfo[] = {
	{ .key	= "vendor:",	._default = "unknown"},
	{ .key	= "model:",	._default = "unknown"},
	{ .key	= "serial:",	._default = "0"},
	{ .key	= "sku:",	._default = "unknown"},
	{ .key	= "vendorurl:",	._default = NULL},
	{ .key	= "modelurl:",	._default = NULL},
	{ .key	= "presentationurl:",	._default = NULL},
	{}
};

int set_getresp(const char *str, const char **next)
{
	int i;
	const char *p, *val;
	size_t keylen, vallen;

	if (str == NULL) {
		return -1;
	}

	if (*str == '\0') {
		return -1;
	}

	/* Trim leading space. */
	while (*str && isspace(*str))
		str++;
	if (!*str) {
		*next = str;
		return 0;
	}

	/* Look for end of key. */
	for (p = str; *p && !isspace(*p) && *p != ':' && *p != ','; p++)
		;

	if (!*p || (!isspace(*p) && *p != ':'))
		goto exit;
	keylen = p++ - str;

	/* Look for value. */
	while (*p && isspace(*p))
		p++;
	if (!*p || *p == ':')
		goto exit;
	val = p;

	/* Look for end of value. */
	while (*p && *p != ':' && *p != ',')
		p++;
	if (*p && *p != ',')
		goto exit;

	vallen = p - val;
	if (next)
		*next = *p ? ++p : NULL;;

	/* Look for key in lookup table. */
	for (i = 0; bootinfo[i].key; i++)
		if (!strncasecmp(bootinfo[i].key, str, keylen))
			break;

	if (!bootinfo[i].key)
		return -1;

	if (!bootinfo[i].value)
		bootinfo[i].value = strndup(val, vallen + 1);

	return 0;

exit:
	if (next)
		*next = p;
	return -1;
}

const char *get_getresp(const char *key)
{
	for (int i = 0; bootinfo[i].key; i++)
		if (!strcmp(bootinfo[i].key, key))
			return bootinfo[i].value ? bootinfo[i].value : bootinfo[i]._default;
	return "UNKNOWN";
}

void printBootInfoKeys(FILE *fp, int indent)
{
	for (int i = 0; bootinfo[i].key; i++) {
		fprintf(fp, "%*s%s %s\n", indent, "",
			bootinfo[i].key, get_getresp(bootinfo[i].key));
	}
}

void init_getresp(void)
{
	static bool getresp_inited = false;

	if (!getresp_inited) {
		FILE *fp = fopen("/proc/sys/dev/boot/info", "r");
		if (fp) {
			char buf[80];
			while (fgets(buf, sizeof(buf), fp))
				set_getresp(buf, NULL);
			fclose(fp);
		}
		getresp_inited = true;
	}
}

#define SOAP11_NS \
	"http://schemas.xmlsoap.org/soap/envelope/"
#define SOAP12_NS \
	"http://www.w3.org/2003/05/soap-envelope"
#define WSA_NS \
	"http://schemas.xmlsoap.org/ws/2004/08/addressing"
#define WSD_NS \
	"http://schemas.xmlsoap.org/ws/2005/04/discovery"
#define WXT_NS \
	"http://schemas.xmlsoap.org/ws/2004/09/transfer"
#define WSD_ACT_HELLO \
	"http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello"
#define WSD_ACT_BYE \
	"http://schemas.xmlsoap.org/ws/2005/04/discovery/Bye"
#define WSD_ACT_PROBE \
	"http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"
#define WSD_ACT_PROBEMATCH \
	"http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches"
#define WSD_ACT_RESOLVE \
	"http://schemas.xmlsoap.org/ws/2005/04/discovery/Resolve"
#define WSD_ACT_RESOLVEMATCH \
	"http://schemas.xmlsoap.org/ws/2005/04/discovery/ResolveMatches"
#define WXT_ACT_GET \
	"http://schemas.xmlsoap.org/ws/2004/09/transfer/Get"
#define WXT_ACT_GETRESPONSE \
	"http://schemas.xmlsoap.org/ws/2004/09/transfer/GetResponse"
#define WSD_TO_DISCOVERY \
	"urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define WSD_TO_ANONYMOUS \
	"http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous"

/*
 * macros
 */
#define RESET_BUFFER(buf, buflen) \
	if (buflen > 0) do { memset(buf, 0, buflen); buflen=0; } while(0)

#define COPY_STRING_TO_BUFFER(dst, dstlen, start, src, srclen) \
	do { \
		srclen = strlen(src); \
		if (((start + srclen) - dst) > dstlen) { \
			srclen = -1; \
			break; \
		} \
		strncpy(start, src, srclen); \
		start += srclen; \
	} while(0)

#define RESOLVE_TAG_AND_SAVE \
	do { \
		qn = xmlns_resolve_tag(table, tag); \
		if (qn) { \
			(*qnames_len)++; \
			qnames[(*qnames_len)-1] = qn; \
		} \
	} while(0)

/*
 * Simplified flat XML tag finder - practical enough for our purposes.
 */
static const char *wsd_tag_find(const char *xml,
				const char *tag[][2], size_t *len)
{
	int i;

	for (i = 0; tag[i][0]; i++) {
		const char *p = strstr(xml, tag[i][0]);

		if (!p)
			continue;
		p += strlen(tag[i][0]);
		while (isspace(*p))
			p++;

		const char *q = strstr(p, tag[i][1]);
		if (!q || strchr(p, '<') < q)
			continue;

		while(p < q && isspace(q[-1]))
			q--;
		*len = q - p;
		return p;
	}
	return NULL;
}

/*
 * Extremely simplified flat XML parser - practical enough for our purposes.
 */
static struct wsd_req_info *wsd_req_parse(const char *xml)
{
	static const char *action_tag[][2] = {
		{"<wsa:Action>", "</wsa:Action>"},
		{"<wsa:Action SOAP-ENV:mustUnderstand=\"true\">",
			"</wsa:Action>"},
		{},
	};
	static const char *msgid_tag[][2] = {
		{"<wsa:MessageID>", "</wsa:MessageID>"},
		{"<wsa:MessageID SOAP-ENV:mustUnderstand=\"true\">",
			"</wsa:MessageID>"},
		{},
	};

	size_t action_len;
	const char *action = wsd_tag_find(xml, action_tag, &action_len);
	if (!action)
		return NULL;

	size_t msgid_len;
	const char *msgid = wsd_tag_find(xml, msgid_tag, &msgid_len);
	if (!msgid)
		return NULL;

	struct wsd_req_info *info = calloc(sizeof *info, 1);
	if (!info)
		return NULL;

	info->action = strndup(action, action_len);
	info->msgid = strndup(msgid, msgid_len);

	static const char *address_tag[][2] = { {"<wsa:Address>", "</wsa:Address>"}, {} };
	size_t address_len;
	const char *address = wsd_tag_find(xml, address_tag, &address_len);
	info->address = address ? strndup(address, address_len) : NULL;

	return info;
}

static void wsd_req_destruct(struct wsd_req_info *info)
{
	if (!info) return;
	free(info->action);
	free(info->msgid);
	free(info->address);
	free(info);
}

enum wsd_action wsd_action_id(const struct wsd_req_info *info)
{
	if (!info || !info->action) {
		return WSD_ACTION_NONE;
	}

	if (strcmp(info->action, WSD_ACT_HELLO) == 0) {
		return WSD_ACTION_HELLO;
	}

	if (strcmp(info->action, WSD_ACT_BYE) == 0) {
		return WSD_ACTION_BYE;
	}

	if (strcmp(info->action, WSD_ACT_PROBE) == 0) {
		return WSD_ACTION_PROBE;
	}

	if (strcmp(info->action, WSD_ACT_PROBEMATCH) == 0) {
		return WSD_ACTION_PROBEMATCH;
	}

	if (strcmp(info->action, WSD_ACT_RESOLVE) == 0) {
		return WSD_ACTION_RESOLVE;
	}

	if (strcmp(info->action, WSD_ACT_RESOLVEMATCH) == 0) {
		return WSD_ACTION_RESOLVEMATCH;
	}

	if (strcmp(info->action, WXT_ACT_GET) == 0) {
		return WSD_ACTION_GET;
	}

	if (strcmp(info->action, WXT_ACT_GETRESPONSE) == 0) {
		return WSD_ACTION_GETRESPONSE;
	}

	return WSD_ACTION_NONE;
}

static int wsd_send_msg(int fd, struct endpoint *ep, const _saddr_t *sa,
			const char *msg, size_t msglen, unsigned int rwait)
{
	int ret;

	errno = 0;
	if (ep->type == SOCK_STREAM) {
		ret = send(fd, msg, msglen, MSG_NOSIGNAL);
	} else {
		if (rwait) {
			useconds_t us = random() % rwait;
			usleep(us);
		}
		ret = sendto(fd, msg, msglen, MSG_NOSIGNAL, (struct sockaddr *)sa,
			(ep->family == AF_INET) ? sizeof sa->in : sizeof sa->in6);
	}

	char ip[_ADDRSTRLEN];
	inet_ntop(sa->ss.ss_family, _SIN_ADDR(sa), ip, sizeof ip);
	DEBUG(3, W, "WSD-TO %s port %u (fd=%d,len=%ld,sent=%d) '%s'\n", ip, _SIN_PORT(sa), fd,
		msglen, ret, msg);

	return ret != (int) msglen;
}

/*
 * wsd soap fault
 */
static int wsd_send_soap_fault(int fd, struct endpoint *ep, _saddr_t *sa,
			int code, const char *reason, const char *detail)
{
	static const char soap_fault_fmt[] =
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	"<soap:Envelope "
	"xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" "
	"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">"
	"<soap:Header>"
	"<wsa:Action>"
	"http://schemas.xmlsoap.org/ws/2004/09/transfer/fault"
	"</wsa:Action>"
	"</soap:Header>"
	"<soap:Body>"
	"<soap:Fault>"
	"<soap:Code>"
	"<soap:Value>%d</soap:Value>"
	"<soap:Subcode>"
	"<soap:Value>%d</soap:Value>"
	"<soap:Subcode>"
	"<soap:Value>%d</soap:Value>"
	"</soap:Subcode>"
	"</soap:Subcode>"
	"</soap:Code>"
	"<soap:Reason>"
	"<soap:Text xml:lang=\"en\">%s</soap:Text>"
	"</soap:Reason>"
	"<soap:Detail>%s</soap:Detail>"
	"</soap:Fault>"
	"</soap:Body>"
	"</soap:Envelope>";

	char *s;
	int rv;
	ssize_t len = asprintf(&s, soap_fault_fmt, code, 0, 0, reason, detail);

	if (len <= 0) {
		ep->errstr = "wsd_send_soap_fault: asprintf";
		ep->_errno = ENOMEM;
		return -1;
	}

	rv = wsd_send_msg(fd, ep, sa, s, len, 0);

	free(s);
	return rv;
}

/*
 * complete and generate the whole WSD SOAP message
 */
static int wsd_send_soap_msg(int fd, struct endpoint *ep,
				const _saddr_t *sa,
				const char *to,
				const char *action, const char *relates,
				const char *body,
				int (*header)(int fd, struct endpoint *ep,
						const _saddr_t *sa,
						int status, size_t len),
				int status)
{
#define __FUNCTION__ "wsd_send_soap_msg"
	static unsigned int msg_no;
	static const char soap_msg_templ[] =
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	"<soap:Envelope "
	"xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" "
	"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
	"xmlns:wsd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
	"xmlns:wsx=\"http://schemas.xmlsoap.org/ws/2004/09/mex\" "
	"xmlns:wsdp=\"http://schemas.xmlsoap.org/ws/2006/02/devprof\" "
	"xmlns:un0=\"http://schemas.microsoft.com/windows/pnpx/2005/10\" "
	"xmlns:pub=\"http://schemas.microsoft.com/windows/pub/2005/07\">"
	"<soap:Header>"
	"<wsa:To>%s</wsa:To>"
	"<wsa:Action>%s</wsa:Action>"
	"<wsa:MessageID>urn:uuid:%s</wsa:MessageID>"
	"<wsd:AppSequence InstanceId=\"%lu\" SequenceId=\"urn:uuid:%s\" "
	"MessageNumber=\"%u\" />"
	"%s"
	"</soap:Header>"
	"%s"
	"</soap:Envelope>";

	char msg_id[UUIDLEN], *soap_relates, *msg;

	uuid_random(msg_id, sizeof msg_id);

	if (relates) {
		if (asprintf(&soap_relates,
			"<wsa:RelatesTo>%s</wsa:RelatesTo>", relates) <= 0) {
			ep->errstr = __FUNCTION__ ": asprintf";
			ep->_errno = errno;
			return -1;
		}
	} else {
		if (!(soap_relates = strdup(""))) {
			ep->errstr = __FUNCTION__ ": strdup";
			ep->_errno = errno;
			return -1;
		}
	}

	ssize_t msglen = asprintf(&msg, soap_msg_templ, to, action, msg_id,
				wsd_instance, wsd_sequence,
				++msg_no, soap_relates,
				body);
	free(soap_relates);

	if (msglen <= 0) {
		ep->errstr = __FUNCTION__ ": asprintf";
		ep->_errno = errno;
		return -1;
	}

	int rv = 0;
	if (header)
		rv = header(fd, ep, sa, status, msglen);

	if (rv == 0) {
		rv = wsd_send_msg(fd, ep, sa, msg, msglen, 0);
		if (rv) {
			ep->errstr = __FUNCTION__ ": send";
			ep->_errno = errno;
			rv = -1;
		}
	}

	free(msg);
	return rv;
#undef __FUNCTION__
}

static int wsd_send_hello(struct endpoint *ep)
{
	static const char body_templ[] =
		"<soap:Body>"
		"<wsd:Hello>"
		"<wsa:EndpointReference>"
		"<wsa:Address>urn:uuid:%s</wsa:Address>"
		"</wsa:EndpointReference>"
		"<wsd:Types>wsdp:Device pub:Computer</wsd:Types>"
		"<wsd:MetadataVersion>2</wsd:MetadataVersion>"
		"</wsd:Hello>"
		"</soap:Body>";
	char *body;

	if (asprintf(&body, body_templ, wsd_endpoint) <= 0) {
		ep->errstr = "wsd_send_hello: asprintf";
		ep->_errno = ENOMEM;
		return -1;
	}

	int rv = wsd_send_soap_msg(ep->sock, ep, &ep->mcast, WSD_TO_DISCOVERY,
				WSD_ACT_HELLO, NULL, body, NULL, 0);

	free(body);
	return rv;
}

static int wsd_send_bye(struct endpoint *ep)
{
	static const char body_templ[] =
		"<soap:Body>"
		"<wsd:Bye>"
		"<wsa:EndpointReference>"
		"<wsa:Address>urn:uuid:%s</wsa:Address>"
		"</wsa:EndpointReference>"
		"<wsd:Types>wsdp:Device pub:Computer</wsd:Types>"
		"<wsd:MetadataVersion>2</wsd:MetadataVersion>"
		"</wsd:Bye>"
		"</soap:Body>";
	char *body;

	if (asprintf(&body, body_templ, wsd_endpoint) <= 0) {
		ep->errstr = "wsd_send_bye: asprintf";
		ep->_errno = ENOMEM;
		return -1;
	}

	int rv = wsd_send_soap_msg(ep->sock, ep, &ep->mcast, WSD_TO_DISCOVERY,
				WSD_ACT_BYE, NULL, body, NULL, 0);

	free(body);
	return rv;
}

static int wsd_send_probe_match(int fd,
				struct endpoint *ep,
				const _saddr_t *sa,
				const struct wsd_req_info *info,
				const char *ip)
{
	static const char body_templ[] =
		"<soap:Body>"
		"<wsd:ProbeMatches>"
		"<wsd:ProbeMatch>"
		"<wsa:EndpointReference>"
		"<wsa:Address>urn:uuid:%s</wsa:Address>"
		"</wsa:EndpointReference>"
		"<wsd:Types>wsdp:Device pub:Computer</wsd:Types>"
		"<wsd:XAddrs>http://%s:%u/%s</wsd:XAddrs>"
		"<wsd:MetadataVersion>2</wsd:MetadataVersion>"
		"</wsd:ProbeMatch>"
		"</wsd:ProbeMatches>"
		"</soap:Body>";
	char *body, *uri_ip = ip2uri(ip);

	if (!uri_ip || asprintf(&body, body_templ, wsd_endpoint, uri_ip, ep->port, wsd_endpoint) <= 0) {
		ep->errstr = "wsd_send_probe_match: ip2uri/asprintf";
		ep->_errno = errno;
		free(uri_ip);
		return -1;
	}

	int rv = wsd_send_soap_msg(fd, ep, sa, WSD_TO_ANONYMOUS,
				WSD_ACT_PROBEMATCH, info->msgid, body, NULL, 0);

	free(uri_ip);
	free(body);
	return rv;
}

static int wsd_send_resolve_match(int fd,
				struct endpoint *ep,
				const _saddr_t *sa,
				const struct wsd_req_info *info,
				const char *ip)
{
	const char body_templ[] =
		"<soap:Body>"
		"<wsd:ResolveMatches>"
		"<wsd:ResolveMatch>"
		"<wsa:EndpointReference>"
		"<wsa:Address>urn:uuid:%s</wsa:Address>"
		"</wsa:EndpointReference>"
		"<wsd:Types>wsdp:Device pub:Computer</wsd:Types>"
		"<wsd:XAddrs>http://%s:%u/%s</wsd:XAddrs>"
		"<wsd:MetadataVersion>2</wsd:MetadataVersion>"
		"</wsd:ResolveMatch>"
		"</wsd:ResolveMatches>"
		"</soap:Body>";
	char *body, *uri_ip = ip2uri(ip);

	if (!uri_ip || asprintf(&body, body_templ, wsd_endpoint, uri_ip, ep->port, wsd_endpoint) <= 0) {
		ep->errstr = "wsd_send_resolve_match: ip2uri/asprintf";
		ep->_errno = errno;
		free(uri_ip);
		return -1;
	}

	int err = wsd_send_soap_msg(fd, ep, sa, WSD_TO_ANONYMOUS,
				WSD_ACT_RESOLVEMATCH, info->msgid, body, NULL, 0);

	free(uri_ip);
	free(body);
	return err;
}

/*
 * Send HTTP response header
 */
static int send_http_resp_header(int fd, struct endpoint *ep,
				const _saddr_t *sa, int status, size_t length)
{
	const char resp_hdr_fmt[] =
		"HTTP/1.1 %s\r\n"
		"Server: NETGEAR WSD Server\r\n"
		"Date: %s\r\n"
		"Connection: close\r\n"
		"Content-Type: application/soap+xml\r\n"
		"Content-Length: %zu\r\n"
		"\r\n";

	const char *status_str;
	int rv = 0;

	/*
	 * HTTP status codes
	 * RFC 2616: https://tools.ietf.org/html/rfc2616
	 */
	switch(status) {
	case 200:
		status_str = "200 OK";
		break;
	case 400:
		status_str = "400 Bad Request";
		break;
	case 405:
		status_str = "405 Method Not Allowed";
		break;
	case 500:
		status_str = "500 Internal Server Error";
		break;
	default:
		status_str = "404 Not Found";
		break;
	}

	char time_str[32];
	time_t t;

	time(&t);
	strftime(time_str, sizeof(time_str), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

	char *s;
	ssize_t len = asprintf(&s, resp_hdr_fmt, status_str, time_str, length);

	if (len <= 0) {
		ep->errstr = "send_http_resp_header: asprintf";
		ep->_errno = errno;
		return -1;
	}

	DEBUG(4, W, "---------- HEADER:\n%s----------\n", s);

	if (wsd_send_msg(fd, ep, sa, s, len, WSD_RANDOM_DELAY) != 0) {
		ep->errstr = "send_http_resp_header: send";
		ep->_errno = errno;
		rv = -1;
	}

	free(s);
	return rv;
}

static int wsd_send_get_response(int fd,
				struct endpoint *ep,
				const _saddr_t *sa,
				const struct wsd_req_info *info,
				const char *ip)
{
	char *body;
	const char body_templ[] =
		"<soap:Body>"
		"<wsx:Metadata>"
		"<wsx:MetadataSection Dialect=\""
		"http://schemas.xmlsoap.org/ws/2006/02/devprof/ThisDevice\">"
		"<wsdp:ThisDevice>"
		"<wsdp:FriendlyName>%s</wsdp:FriendlyName>"
		"<wsdp:FirmwareVersion>%s</wsdp:FirmwareVersion>"
		"<wsdp:SerialNumber>%s</wsdp:SerialNumber>"
		"</wsdp:ThisDevice>"
		"</wsx:MetadataSection>"
		"<wsx:MetadataSection Dialect=\""
		"http://schemas.xmlsoap.org/ws/2006/02/devprof/ThisModel\">"
		"<wsdp:ThisModel>"
		"<wsdp:Manufacturer>%s</wsdp:Manufacturer>"
		"<wsdp:ManufacturerUrl>%s</wsdp:ManufacturerUrl>"
		"<wsdp:ModelName>%s</wsdp:ModelName>"
		"<wsdp:ModelNumber>1</wsdp:ModelNumber>"
		"<wsdp:ModelUrl>%s</wsdp:ModelUrl>"
		"<wsdp:PresentationUrl>%s</wsdp:PresentationUrl>"
		"<un0:DeviceCategory>Computers</un0:DeviceCategory>"
		"</wsdp:ThisModel>"
		"</wsx:MetadataSection>"
		"<wsx:MetadataSection Dialect=\""
		"http://schemas.xmlsoap.org/ws/2006/02/devprof/Relationship\">"
		"<wsdp:Relationship Type=\""
		"http://schemas.xmlsoap.org/ws/2006/02/devprof/host\">"
		"<wsdp:Host>"
		"<wsa:EndpointReference>"
		"<wsa:Address>urn:uuid:%s</wsa:Address>"
		"</wsa:EndpointReference>"
		"<wsdp:Types>pub:Computer</wsdp:Types>"
		"<wsdp:ServiceId>urn:uuid:%s</wsdp:ServiceId>"
		"<pub:Computer>%s/Workgroup:%s</pub:Computer>"
		"</wsdp:Host>"
		"</wsdp:Relationship>"
		"</wsx:MetadataSection>"
		"</wsx:Metadata>"
		"</soap:Body>";

	ssize_t len = asprintf(&body, body_templ,
				"Microsoft Publication Service Device Host",
				"1.0",
				"20050718",
				get_getresp("vendor:"),
				get_getresp("vendorurl:"),
				get_getresp("model:"),
				get_getresp("modelurl:"),
				get_getresp("presentationurl:"),
				wsd_endpoint,
				wsd_endpoint,
				netbiosname,
				workgroup
			);

	(void) ip; // silent "unused" warning

	if (len <= 0) {
		ep->errstr = "wsd_send_get_response: asprintf";
		ep->_errno = errno;
		return -1;
	}

	int rv = wsd_send_soap_msg(fd, ep, sa, WSD_TO_ANONYMOUS,
				WXT_ACT_GETRESPONSE, info->msgid, body, send_http_resp_header, 200);

	free(body);
	return rv;
}

/*
 * Validate HTTP POST header lines.
 * Re-fill buffer with HTTP body.
 * Return MIME error code.
 */
static int wsd_parse_http_header(int fd, struct endpoint *ep,
				char *buf, size_t len, size_t bsize)
{
#define __FUNCTION__	"wsd_pars_http_header"
	size_t contentlength = 0;
	char *p = buf;
	char *eol = strstr(p, "\r\n");
	static size_t endpointlen;

	if (!endpointlen)
		endpointlen = strlen(wsd_endpoint);

	*eol = '\0';
	if (strncmp(p, "POST /", 6)) {
		ep->errstr = __FUNCTION__ ": Only POST method supported";
		return 405;
	}
	if (strncmp(p + 6, wsd_endpoint, endpointlen)) {
		ep->errstr = __FUNCTION__ ": Invalid endpoint UUID";
		return 404;
	}
	if (strncmp(p + 6 + endpointlen, " HTTP/", 6)) {
		ep->errstr = __FUNCTION__ ": Must be HTTP/1.0 and up";
		return 405;
	}

	p = eol + 2;

again:
#define HEADER_IS(p,h) (strncasecmp((p), (h), strlen((h))) ? NULL : (p) + strlen(h))
	while (*p && (eol = strstr(p, "\r\n")) != p) {
		const char *val;

		if (!eol) break;
		*eol = '\0';

		if ((val = HEADER_IS(p, "Content-Type:"))) {
			while (*val == ' ' || *val == '\t' || *val == '\r' || *val == '\n')
				val++; // skip LWS
			if (strcmp(val, "application/soap+xml")) {
				ep->errstr = __FUNCTION__ ": Unsupported Content-Type";
				return 400;
			}
		} else if ((val = HEADER_IS(p, "Content-Length:"))) {
			while (*val == ' ' || *val == '\t' || *val == '\r' || *val == '\n')
				val++; // skip LWS
			if ((contentlength = atoi(val)) <= 0) {
				ep->errstr = __FUNCTION__ ": Invalid Content-Length";
				return 400;
			}
		}
		p = eol + 2;
	}

	bool eoh = false;
	if (eol == p) {
		p = eol + 2;
		eoh = true;
	}
	memmove(buf, p, len -= (p - buf));
	buf[len] = '\0';

	if (!eoh) { /* Have not reached end of header. */
		ssize_t len2 = recv(fd, buf + len, bsize - len - 1, 0);

		if (len2 <= 0) {
			ep->errstr = __FUNCTION__ ": Data receiving error";
			return 500;
		}
		len += len2;
		buf[len] = '\0';
		goto again;
	}

	if (!contentlength) {
		ep->errstr = __FUNCTION__ ": Invalid Content-Length";
		return 400;
	}

	if (len < contentlength) {
		if (contentlength >= bsize) {
			ep->errstr = __FUNCTION__ ": Content-Length too large";
			return 500;
		}

		ssize_t len2 = recv(fd, buf + len, contentlength - len, 0);

		if (len2 < (ssize_t) (contentlength - len)) {
			ep->errstr = __FUNCTION__ ": Data receiving error";
			return 500;
		}
	}

	buf[contentlength] = '\0';
	return 200;
#undef	__FUNCTION__
}

int wsd_init(struct endpoint *ep)
{
	if (!wsd_instance)
		time(&wsd_instance);
	if (!wsd_sequence[0])
		uuid_random(wsd_sequence, sizeof(wsd_sequence));
	if (!wsd_endpoint[0]) {
		uuid_endpoint(wsd_endpoint);
		if (!wsd_endpoint[0]) {
			ep->errstr = "wsd_init: uuid_endpoint";
			ep->_errno = errno;
			return -1;
		}
	}

	return wsd_send_hello(ep);
}

static int wsd_recv_action(int (*f)(int fd,
				struct endpoint *ep,
				const _saddr_t *sa,
				const struct wsd_req_info *info,
				const char *ip),
			int fd,
			struct endpoint *ep,
			const _saddr_t *sa,
			const struct wsd_req_info *info)
{
	_saddr_t ci;

	if (connected_if(sa, &ci)) {
		ep->errstr = "wsd_recv_action: connected_if";
		ep->_errno = errno;
		return -1;
	}

	char ip[_ADDRSTRLEN];
	void *src = (sa->ss.ss_family == AF_INET) ? (void *)&ci.in.sin_addr : (void *)&ci.in6.sin6_addr;

	if (!inet_ntop(sa->ss.ss_family, src, ip, sizeof ip)) {
		ep->errstr = "wsd_recv_action: inet_ntop";
		ep->_errno = errno;
		return -1;
	}

	return f(fd, ep, sa, info, ip);
}

int wsd_recv(struct endpoint *ep)
{
	char buf[10000];
	_saddr_t sa = {};
	socklen_t slen = (ep->family == AF_INET) ? sizeof sa.in : sizeof sa.in6;
	int fd = ep->sock;
	ssize_t len;

	if (ep->type == SOCK_STREAM) {
		struct timeval to = { .tv_sec = 1, .tv_usec = 0};

		fd = accept(ep->sock, (struct sockaddr *)&sa, &slen);
		if (fd < 0) {
			ep->errstr = "wsd_recv: accept";
			ep->_errno = errno;
			return -1;
		}
		if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to) < 0) {
			DEBUG(3, W, "Unable to set receive timeout\n");
		}
		len = recv(fd, buf, sizeof buf - 1, 0);
	} else {
		len = recvfrom(fd, buf, sizeof buf - 1, 0, (struct sockaddr *)&sa, &slen);
	}

	if (len <= 0) {
		if (ep->sock != fd)
			close(fd);
		return len;
	}

	buf[len] = '\0';

	{
		char ip[_ADDRSTRLEN];
		inet_ntop(sa.ss.ss_family, _SIN_ADDR(&sa), ip, sizeof ip);
		DEBUG(3, W, "WSD-FROM %s port %u (fd=%d,len=%ld): '%s'\n", ip, _SIN_PORT(&sa), fd, len, buf);
	}

	if (ep->type == SOCK_STREAM && strncmp(buf, "POST ", 5) == 0) {
		int status = wsd_parse_http_header(fd, ep, buf, len, sizeof buf);

		{
			char ip[_ADDRSTRLEN];
			inet_ntop(sa.ss.ss_family, _SIN_ADDR(&sa), ip, sizeof ip);
			DEBUG(3, W, "WSD-BODY %s port %u (fd=%d,status=%d,len=%ld): '%s'\n", ip, _SIN_PORT(&sa),
				fd, status, strlen(buf), buf);
		}

		if (status > 200) {
			send_http_resp_header(fd, ep, &sa, status, 0);
			if (status >= 400)
				wsd_send_soap_fault(fd, ep, &sa, status, ep->errstr, ep->errstr);
			if (ep->sock != fd)
				close(fd);
			return 0;
		}
	}

	int rv = 0;
	struct wsd_req_info *info = wsd_req_parse(buf);

	{
		char src[_ADDRSTRLEN];
		inet_ntop(sa.ss.ss_family, _SIN_ADDR(&sa), src, sizeof src);
		const char *action = info && info->action ? strrchr(info->action, '/') : "NONE";
		if (!action) action = info->action;
		const char *address = info && info->address ? info->address : NULL;
		DEBUG(2, W, "WSD-ACTION %s port %u %s %s %s", src, _SIN_PORT(&sa),
			ep->service->name, action, address);
	}

	switch (wsd_action_id(info)) {
	case WSD_ACTION_PROBE:
		rv = wsd_recv_action(wsd_send_probe_match, fd, ep, &sa, info);
		break;
	case WSD_ACTION_RESOLVE:
		rv = wsd_recv_action(wsd_send_resolve_match, fd, ep, &sa, info);
		break;
	case WSD_ACTION_GET:
		rv = wsd_recv_action(wsd_send_get_response, fd, ep, &sa, info);
		break;
	default:
		DEBUG(2, W, "wsd_recv: Unsupported query");
		break;
	}

	if (rv)
		DEBUG(1, W, "wsd_recv: %s: %s", ep->errstr, strerror(ep->_errno));

	wsd_req_destruct(info);
	if (ep->sock != fd)
		close(fd);
	return 0;
}

void wsd_exit(struct endpoint *ep)
{
	wsd_send_bye(ep);
}
