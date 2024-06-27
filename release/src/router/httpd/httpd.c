/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* milli_httpd - pretty small HTTP server
** A combination of
** micro_httpd - really small HTTP server
** and
** mini_httpd - small HTTP server
**
** Copyright ?1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <ctype.h>

typedef unsigned int __u32;   // 1225 ham

#include <httpd.h>
#include <common.h>
//2008.08 magic{
#include <bcmnvram.h>	//2008.08 magic
#include <arpa/inet.h>	//2008.08 magic

#if defined(__GLIBC__) || defined(__UCLIBC__)		//musl doesn't have error.h
#include <error.h>
#endif	/* __GLIBC__||__UCLIBC__ */
#include <signal.h>
#include <sys/wait.h>
#include <shared.h>
#include <shutils.h>

#define ETCPHYRD	14
#define SIOCGETCPHYRD   0x89FE
//#include "etioctl.h"

#include <sys/file.h>

#ifdef RTCONFIG_HTTPS
#include <syslog.h>
#include <mssl.h>
//#include <shutils.h>
#define SERVER_PORT_SSL	443
#endif
#include "bcmnvram_f.h"

/* A multi-family sockaddr. */
struct usockaddr {
	union {
		struct sockaddr sa;
		struct sockaddr_in sa_in;
#ifdef RTCONFIG_IPV6
		struct sockaddr_in6 sa_in6;
#endif
	};
	socklen_t sa_len;
};

#include "queue.h"
#define MAX_CONN_ACCEPT 128
#define MAX_CONN_TIMEOUT 5

typedef struct conn_item {
	TAILQ_ENTRY(conn_item) entry;
	int fd;
	usockaddr usa;
	time_t deadline;
} conn_item_t;

typedef struct conn_list {
	TAILQ_HEAD(, conn_item) head;
	int count;
} conn_list_t;

/* Globals. */
static FILE *conn_fp;
char host_name[64];
char referer_host[64];
char current_page_name[128];
char user_agent[1024];
char gen_token[32];
time_t login_timestamp_cache = 0;
int url_do_auth = 0;
int spam_count = 0;
int auth_result = 1;
char cookies_buf[4096];
int do_ssl = 0; 	// use Global for HTTPS upgrade judgment in web.c
int ssl_stream_fd; 	// use Global for HTTPS stream fd in web.c
int json_support = 0;
char wl_band_list[8][8] = {{0}};
char pidfile[32];
char HTTPD_LOGIN_FAIL_LAN[32] = {0};
char HTTPD_LOGIN_FAIL_WAN[32] = {0};
char HTTPD_LAST_LOGIN_TS[32] = {0};
char HTTPD_LAST_LOGIN_TS_W[32] = {0};
char CAPTCHA_FAIL_NUM[32] = {0};
char HTTPD_LOCK_NUM[32] = {0};

#ifdef TRANSLATE_ON_FLY
char Accept_Language[16];

struct language_table language_tables[] = {
	{"br", "BR"},
	{"pt-BR", "BR"},
	{"zh-cn", "CN"},
	{"zh-Hans-CN", "CN"},
	{"cs", "CZ"},
	{"cs-cz", "CZ"},
	{"da", "DA"},
	{"da-DK", "DA"},
	{"de", "DE"},
	{"de-at", "DE"},
	{"de-ch", "DE"},
	{"de-de", "DE"},
	{"de-li", "DE"},
	{"de-lu", "DE"},
	{"en", "EN"},
	{"en-us", "EN"},
	{"es", "ES"},
	{"es-ec", "ES"},
	{"es-py", "ES"},
	{"es-pa", "ES"},
	{"es-ni", "ES"},
	{"es-gt", "ES"},
	{"es-do", "ES"},
	{"es-es", "ES"},
	{"es-hn", "ES"},
	{"es-ve", "ES"},
	{"es-pr", "ES"},
	{"es-ar", "ES"},
	{"es-bo", "ES"},
	{"es-us", "ES"},
	{"es-co", "ES"},
	{"es-cr", "ES"},
	{"es-uy", "ES"},
	{"es-pe", "ES"},
	{"es-cl", "ES"},
	{"es-mx", "ES"},
	{"es-sv", "ES"},
	{"fi", "FI"},
	{"fi-FI", "FI"},
	{"fr", "FR"},
	{"fr-fr", "FR"},
	{"fr-BE", "FR"},
	{"fr-CA", "FR"},
	{"fr-CH", "FR"},
	{"fr-MC", "FR"},
	{"fr-LU", "FR"},
	{"hu-hu", "HU"},
	{"hu", "HU"},
	{"it", "IT"},
	{"it-it", "IT"},
	{"it-ch", "IT"},
	{"ja", "JP"},
	{"ja-JP", "JP"},
	{"ko", "KR"},
	{"ko-kr", "KR"},
	{"ms", "MS"},
	{"ms-MY", "MS"},
	{"ms-BN", "MS"},
	{"nl", "NL"},
	{"nl-NL", "NL"},
	{"no", "NO"},
	{"nb", "NO"},
	{"nn", "NO"},
	{"nb-NO", "NO"},
	{"nn-NO", "NO"},
	{"pl-pl", "PL"},
	{"pl", "PL"},
	{"ru", "RU"},
	{"ru-ru", "RU"},
	{"ro", "RO"},
	{"ro-ro", "RO"},
	{"sl", "SL"},
	{"sl-SI", "SL"},
	{"sv", "SV"},
	{"sv-FI", "SV"},
	{"sv-SE", "SV"},
	{"th", "TH"},
	{"th-TH", "TH"},
	{"th-TH-TH", "TH"},
	{"tr", "TR"},
	{"tr-TR", "TR"},
	{"zh", "TW"},
	{"zh-tw", "TW"},
	{"zh-Hant-TW", "TW"},
	{"zh-hk", "TW"},
	{"uk", "UK"},
	{NULL, NULL}
};

#endif //TRANSLATE_ON_FLY

/* Forwards. */
static int initialize_listen_socket(int family, usockaddr* usa, const char *ifname);
int check_noauth_referrer(char* referer, int fromapp_flag);
char *get_referrer(char *referer, char *auth_referer, size_t length);
static void send_error( int status, char* title, char* extra_header, char* text );
//#ifdef RTCONFIG_CLOUDSYNC
void send_page( int status, char* title, char* extra_header, char* text , int fromapp);
//#endif
static void send_headers( int status, char* title, char* extra_header, char* mime_type, int fromapp);
static void send_token_headers( int status, char* title, char* extra_header, char* mime_type, int fromapp);
static void handle_request(void);
void send_login_page(int fromapp_flag, int error_status, char* url, char* file, int lock_time, int logintry);
void page_default_redirect(int fromapp_flag, char* url);
int check_user_agent(char* user_agent);
#if defined(RTCONFIG_IFTTT) || defined(RTCONFIG_ALEXA) || defined(RTCONFIG_GOOGLE_ASST)
void add_ifttt_flag(void);
#endif

int check_current_ip_is_lan_or_wan();

/* added by Joey */
//2008.08 magic{
//int redirect = 1;
int redirect = 0;
int change_passwd = 0;
int x_Setting = 0;
int skip_auth = 0;
char url[128];
int http_port = 0;
char *http_ifname = NULL;
#ifdef RTCONFIG_IPV6
int http_ipv6_only = 0;
#else
const int http_ipv6_only = 0;
#endif
time_t login_dt=0;
char login_url[128];
int login_error_status = 0;
char cloud_file[256];
char indexpage[128];


/* Added by Joey for handle one people at the same time */
unsigned int login_ip = 0; // IPv6 compat: the logined ip
uaddr login_uip = {0}; // the logined ip
unsigned int app_login_ip = 0; // IPv6 compat: the logined ip
time_t login_timestamp=0; // the timestamp of the logined ip
time_t login_timestamp_tmp=0; // the timestamp of the current session.
time_t last_login_timestamp=0; // the timestamp of the current session.
unsigned int login_ip_tmp = 0; // IPv6 compat: the ip of the current session.
uaddr login_uip_tmp = {0}; // the ip of the current session.
usockaddr login_usa_tmp = {{{0}}};
unsigned int login_try=0;
//Add by Andy for handle the login block mechanism by LAN/WAN
time_t login_timestamp_tmp_wan=0; // the timestamp of the current session.
time_t last_login_timestamp_wan=0; // the timestamp of the current session.
time_t auth_check_dt=0;
unsigned int login_try_wan=0;
int cur_login_ip_type = -1;	//0:LAN, 1:WAN, -1:ERROR
int lock_flag = 0;

// 2008.08 magic {
time_t request_timestamp = 0;
time_t turn_off_auth_timestamp = 0;
int temp_turn_off_auth = 0;	// for QISxxx.htm pages

int amas_support = 0;
int HTS = 0;	//HTTP Transport Security

struct timeval alarm_tv;
time_t alarm_timestamp = 0;
int check_alive_flag = 0;

/* Const vars */
const int int_1 = 1;
const struct linger linger = { 1, 0 };

void http_login(uaddr *uip, char *url);
void app_http_login(uaddr *uip);
void http_login_timeout(uaddr *u, char *cookies, int fromapp_flag);
void http_logout(uaddr *u, char *cookies, int fromapp_flag);
int http_login_check(void);

#if 0
static int check_if_inviteCode(const char *dirpath){
	return 1;
}
#endif

#ifndef RTCONFIG_LIBASUSLOG
static int
log_pass_handler(char *url)
{
	struct log_pass_url_list *lp_handler;
	for (lp_handler = &log_pass_handlers[0]; lp_handler->pattern; lp_handler++) {
		if (match(lp_handler->pattern, url)){
			return 1;
		}
	}
	return 0;
}

void Debug2File(const char *path, const char *fmt, ...)
{
	FILE *fp;
	va_list args;
	time_t now;
	struct tm tm;
	char timebuf[100];

	if (nvram_get_int("HTTPD_DBG") <= 1) {
		if (log_pass_handler(url))
			return;
	}

	fp = fopen(path, "a+");
	if (fp) {
#ifndef RTCONFIG_AVOID_TZ_ENV
		setenv("TZ", nvram_safe_get_x("", "time_zone_x"), 1);
#endif
		now = time(NULL);
		localtime_r(&now, &tm);
		strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", &tm);
		fprintf(fp, "%s ", timebuf);
		va_start(args, fmt);
		vfprintf(fp, fmt, args);
		va_end(args);
		fclose(fp);
	} else
		fprintf(stderr, "Open %s Error!\n", path);
}
#endif

void sethost(const char *host)
{
	char *p = host_name;
	size_t len;

	if (!host || *host == '\0')
		goto error;

	while (*host == '.') host++;

	len = strcspn(host, "\r\n");
	while (len > 0 && strchr(" \t", host[len - 1]) != NULL)
		len--;
	if (len > sizeof(host_name) - 1)
		goto error;

	while (len-- > 0) {
		int c = *host++;
		if (((c | 0x20) < 'a' || (c | 0x20) > 'z') &&
		    ((c < '0' || c > '9')) &&
		    strchr(".-_:[]", c) == NULL) {
			p = host_name;
			break;
		}
		*p++ = c;
	}

error:
	*p = '\0';
}

char *gethost(void)
{
	return host_name[0] ? host_name : nvram_safe_get("lan_ipaddr");
}

#include <sys/sysinfo.h>
long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

static int
initialize_listen_socket(int family, usockaddr* usa, const char *ifname)
{
	struct ifreq ifr;
	int fd;

	memset(usa, 0, sizeof(usockaddr));
	usa->sa.sa_family = family;
	switch (family) {
	case AF_INET:
		usa->sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
		usa->sa_in.sin_port = htons(http_port);
		usa->sa_len = sizeof(usa->sa_in);
		break;
#ifdef RTCONFIG_IPV6
	case AF_INET6:
		usa->sa_in6.sin6_addr = in6addr_any;
		usa->sa_in6.sin6_port = htons(http_port);
		usa->sa_len = sizeof(usa->sa_in6);
		break;
#endif
	default:
		return -1;
	}

	fd = socket(usa->sa.sa_family, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	fcntl(fd, F_SETFD, FD_CLOEXEC);
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &int_1, sizeof(int_1)) < 0) {
		perror("setsockopt");
		goto error;
	}
#ifdef RTCONFIG_IPV6
	if (usa->sa.sa_family == AF_INET6 &&
	    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &int_1, sizeof(int_1)) < 0) {
		perror("setsockopt");
		goto error;
	}
#endif

	if (ifname) {
		memset(&ifr, 0, sizeof(ifr));
		strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
		switch (usa->sa.sa_family) {
		case AF_INET:
			if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
				perror("ioctl");
				goto error;
			}
			usa->sa_in.sin_addr.s_addr = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr;
			break;
#ifdef RTCONFIG_IPV6
		case AF_INET6:
			/* IPv6 addresses are dynamic, bind to interface */
			if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
				perror("setsockopt");
				goto error;
			}
			break;
#endif
		}
	}
	if (bind(fd, &usa->sa, usa->sa_len) < 0) {
		perror("bind");
		goto error;
	}
	if (listen(fd, MAX_CONN_ACCEPT) < 0) {
		perror( "listen" );
		goto error;
	}

	return fd;

error:
	close(fd);
	return -1;
}

void 
page_default_redirect(int fromapp_flag, char* url)
{
	char inviteCode[256]={0};

	if(check_xss_blacklist(url, 1))
		strncpy(login_url, indexpage, sizeof(login_url));
	else
		strncpy(login_url, url, sizeof(login_url));

	if(fromapp_flag == 0){
		snprintf(inviteCode, sizeof(inviteCode), "<script>top.location.href='/page_default.cgi?url=%s';</script>", url);
	}
	send_page( 200, "OK", (char*) 0, inviteCode, fromapp_flag);
}

void
send_login_page(int fromapp_flag, int error_status, char* url, char* file, int lock_time, int logintry)
{
	char inviteCode[256]={0};
	char buf[128] = {0};
	//char url_tmp[64]={0};
	char *cp, *file_var=NULL;

	HTTPD_DBG("error_status = %d, logintry = %d\n", error_status, logintry);

	if(logintry){
		if(!cur_login_ip_type)
		{
			++login_try;
			if(error_status != LOGINLOCK)
				last_login_timestamp = login_timestamp_tmp;
		}
		else
		{
			++login_try_wan;
			if(error_status != LOGINLOCK)
				last_login_timestamp_wan= login_timestamp_tmp_wan;
		}
	}

	if(url == NULL)
		strncpy(login_url, indexpage, sizeof(login_url));
	else
		strncpy(login_url, url, sizeof(login_url));

	login_dt = lock_time;

	login_error_status = error_status;

	if(fromapp_flag == 0){
		if(strncmp(login_url, "cloud_sync.asp", strlen(login_url))==0){
			if(file != NULL){
				cp = strstr(file,"flag=");
				if(cp != (char*) 0){
					file_var = &cp[5];
					memset(cloud_file, 0, sizeof(cloud_file));
					strncpy(cloud_file, file_var, sizeof(cloud_file));
				}
			}
		}
		else if(strncmp(login_url, "cfg_onboarding.cgi", strlen(login_url))==0){
			if(file != NULL){
				cp = strstr(file,"id=");
				if(cp != (char*) 0){
					file_var = &cp[3];
					if(!check_cmd_whitelist(file_var) && (strlen(file_var) == 12)){
						memset(cloud_file, 0, sizeof(cloud_file));
						strlcpy(cloud_file, file_var, sizeof(cloud_file));
					}
				}
			}
		}
		snprintf(inviteCode, sizeof(inviteCode), "<script>window.top.location.href='/Main_Login.asp';</script>");
	}else{
		snprintf(inviteCode, sizeof(inviteCode), "\"error_status\":\"%d\", \"last_time_lock_warning\":\"%d\"", error_status, last_time_lock_warning());
		if(error_status == LOGINLOCK){
			snprintf(buf, sizeof(buf), ",\"remaining_lock_time\":\"%ld\"", max_lock_time - login_dt);
			strlcat(inviteCode, buf, sizeof(inviteCode));
		}
	}
	send_page( 200, "OK", (char*) 0, inviteCode, fromapp_flag);
}

char
*get_referrer(char *referer, char *auth_referer, size_t length)
{
	char *referer_t = NULL;
	char ip_addr[100] = {0}, path[100] = {0};

	if(referer == NULL)
		goto Error;

	if(strstr(referer,"\r") != (char*) 0)
		referer_t = strtok(referer, "\r");
	else
		referer_t = referer;

	if(referer_t == NULL)
		goto Error;

	int uri_scan_status = sscanf(referer_t, "%*[^:]%*[:/]%99[^/]%99s", ip_addr, path);

	if(uri_scan_status <= 0)
		strlcpy(auth_referer, referer_t, length);
	else
		strlcpy(auth_referer, ip_addr, length);

	return auth_referer;

Error:
	*auth_referer = '\0';
	return auth_referer;
}

static void
send_error( int status, char* title, char* extra_header, char* text )
{
	send_headers( status, title, extra_header, "text/html", 0);
	(void) fprintf( conn_fp, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title, status, title );
	(void) fprintf( conn_fp, "%s\n", text );
	(void) fprintf( conn_fp, "</BODY></HTML>\n" );
	(void) fflush( conn_fp );
}

//#ifdef RTCONFIG_CLOUDSYNC
void
send_page( int status, char* title, char* extra_header, char* text , int fromapp){
    if(fromapp == 0){
	send_headers( status, title, extra_header, "text/html", fromapp);
	(void) fprintf( conn_fp, "<HTML><HEAD>");
	(void) fprintf( conn_fp, "%s\n", text );
	(void) fprintf( conn_fp, "</HEAD></HTML>\n" );
    }else{
	send_headers( status, title, extra_header, "application/json;charset=UTF-8", fromapp );
	(void) fprintf( conn_fp, "{\n");
	(void) fprintf( conn_fp, "%s\n", text );
	(void) fprintf( conn_fp, "}\n" );	
    }
    (void) fflush( conn_fp );
}
//#endif

void
send_content_page( int status, char* title, char* extra_header, char* text , int fromapp){
	(void) fprintf( conn_fp, "<HTML><HEAD>");
	(void) fprintf( conn_fp, "%s\n", text );
	(void) fprintf( conn_fp, "</HEAD></HTML>\n" );
	(void) fflush( conn_fp );
}

static void
send_headers( int status, char* title, char* extra_header, char* mime_type, int fromapp)
{
    time_t now;
    char timebuf[100];
    (void) fprintf( conn_fp, "%s %d %s\r\n", PROTOCOL, status, title );
    (void) fprintf( conn_fp, "Server: %s\r\n", SERVER_NAME );
    (void) fprintf( conn_fp, "x-frame-options: SAMEORIGIN\r\n");
    (void) fprintf( conn_fp, "x-xss-protection: 1; mode=block\r\n");

    if (fromapp != 0){
	(void) fprintf( conn_fp, "Cache-Control: no-store\r\n");	
	(void) fprintf( conn_fp, "Pragma: no-cache\r\n");
	if(fromapp == FROM_DUTUtil){
		(void) fprintf( conn_fp, "AiHOMEAPILevel: %d\r\n", EXTEND_AIHOME_API_LEVEL );
		(void) fprintf( conn_fp, "Httpd_AiHome_Ver: %d\r\n", EXTEND_HTTPD_AIHOME_VER );
		(void) fprintf( conn_fp, "Model_Name: %s\r\n", get_productid() );
	}else if(fromapp == FROM_ASSIA){
		(void) fprintf( conn_fp, "ASSIA_API_Level: %d\r\n", EXTEND_ASSIA_API_LEVEL );
	}
    }
    now = time( (time_t*) 0 );
    (void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
    (void) fprintf( conn_fp, "Date: %s\r\n", timebuf );
    if ( extra_header != (char*) 0 )
	(void) fprintf( conn_fp, "%s\r\n", extra_header );
    if ( mime_type != (char*) 0 ){
	if(fromapp != FROM_BROWSER && fromapp != FROM_WebView)
		(void) fprintf( conn_fp, "Content-Type: %s\r\n", "application/json;charset=UTF-8" );		
	else
		(void) fprintf( conn_fp, "Content-Type: %s\r\n", mime_type );
    }

    (void) fprintf( conn_fp, "Connection: close\r\n" );
    (void) fprintf( conn_fp, "\r\n" );
}

static void
send_token_headers( int status, char* title, char* extra_header, char* mime_type, int fromapp)
{
	time_t now;
	char timebuf[100];
	char asus_token[32]={0}, token_cookie[128] = {0};

	gen_asus_token_cookie(asus_token, sizeof(asus_token), token_cookie, sizeof(token_cookie));

    (void) fprintf( conn_fp, "%s %d %s\r\n", PROTOCOL, status, title );
    (void) fprintf( conn_fp, "Server: %s\r\n", SERVER_NAME );
    if(fromapp == FROM_DUTUtil){
	(void) fprintf( conn_fp, "AiHOMEAPILevel: %d\r\n", EXTEND_AIHOME_API_LEVEL );
	(void) fprintf( conn_fp, "Httpd_AiHome_Ver: %d\r\n", EXTEND_HTTPD_AIHOME_VER );
	(void) fprintf( conn_fp, "Model_Name: %s\r\n", get_productid() );
    }else if(fromapp == FROM_ASSIA){
	(void) fprintf( conn_fp, "ASSIA_API_Level: %d\r\n", EXTEND_ASSIA_API_LEVEL );
    }
    now = time( (time_t*) 0 );
    (void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
    (void) fprintf( conn_fp, "Date: %s\r\n", timebuf );
    if ( extra_header != (char*) 0 )
	(void) fprintf( conn_fp, "%s\r\n", extra_header );
    if ( mime_type != (char*) 0 )
	(void) fprintf( conn_fp, "Content-Type: %s\r\n", mime_type );

    (void) fprintf( conn_fp, "Set-Cookie: %s\r\n", token_cookie);

    (void) fprintf( conn_fp, "Connection: close\r\n" );
    (void) fprintf( conn_fp, "\r\n" );
}

/* Simple shell-style filename matcher.  Only does ? * and **, and multiple
** patterns separated by |.  Returns 1 or 0.
*/
int
match( const char* pattern, const char* string )
    {
    const char* or;

    for (;;)
	{
	or = strchr( pattern, '|' );
	if ( or == (char*) 0 )
	    return match_one( pattern, strlen( pattern ), string );
	if ( match_one( pattern, or - pattern, string ) )
	    return 1;
	pattern = or + 1;
	}
    }


int
match_one( const char* pattern, int patternlen, const char* string )
    {
    const char* p;

    for ( p = pattern; p - pattern < patternlen; ++p, ++string )
	{
	if ( *p == '?' && *string != '\0' )
	    continue;
	if ( *p == '*' )
	    {
	    int i, pl;
	    ++p;
	    if ( *p == '*' )
		{
		/* Double-wildcard matches anything. */
		++p;
		i = strlen( string );
		}
	    else
		/* Single-wildcard matches anything but slash. */
		i = strcspn( string, "/" );
	    pl = patternlen - ( p - pattern );
	    for ( ; i >= 0; --i )
		if ( match_one( p, pl, &(string[i]) ) )
		    return 1;
	    return 0;
	    }
	if ( *p != *string )
	    return 0;
	}
    if ( *string == '\0' )
	return 1;
    return 0;
}

int web_write(const char *buffer, int len, FILE *stream)
{
	int n = len;
	int r = 0;

	while (n > 0) {
		r = fwrite(buffer, 1, n, stream);
		if (( r == 0) && (errno != EINTR)) return -1;
		buffer += r;
		n -= r;
	}
	return r;
}

#if defined(RTCONFIG_IFTTT) || defined(RTCONFIG_ALEXA) || defined(RTCONFIG_GOOGLE_ASST)
void add_ifttt_flag(void){

	memset(user_agent, 0, sizeof(user_agent));
	snprintf(user_agent, sizeof(user_agent), "%s",IFTTTUSERAGENT);
	return;
}
#endif

#if 0
void
do_file(char *path, FILE *stream)
{
	FILE *fp;
	char buf[1024];
	int nr;

	if (!(fp = fopen(path, "r")))
		return;
	while ((nr = fread(buf, 1, sizeof(buf), fp)) > 0) {
		web_write(buf, nr, stream);
	}
	fclose(fp);
}
#else
int do_fwrite(const char *buffer, int len, FILE *stream)
{
	int n = len;
	int r = 0;

	while (n > 0) {
		r = fwrite(buffer, 1, n, stream);
		if ((r == 0) && (errno != EINTR)) return -1;
		buffer += r;
		n -= r;
	}

	return r;
}

#if 0 //defined(GTAX6000)
#define MOD_URL_ODMPID	"GX-AC5400"
static const char *mod_filter = ".png";
static const char *mod_url_fn[] = { "/modem_plug.png", "/modem_unplug.png", "/WANunplug.png",
	"/WAN-connection-defaultPage.png", "/WAN-connection-type.png", "/GT-bg_header.png", NULL
};

/**
 * This function is used to select different files for different models
 * that share same firmware. If @path include one string of mod_url_fn,
 * insert /.MODEL_NAME/ before it.
 * If mod_filter is not NULL, ignore @path that doesn't contait it.
 */
static char *mod_url_path(char *path, char *tpath, size_t tpath_size)
{
	int l;
	char *_path = path;
	const char **v, *q, tmpl[] = "/." MOD_URL_ODMPID "/";

	if (!path || !tpath || !tpath_size)
		return path;

	if (!nvram_match("odmpid", MOD_URL_ODMPID) || (mod_filter && !strstr(path, mod_filter)))
		return path;

	l = strlen(path) + strlen(tmpl) + 1;	/* Insert "/.MODEL_NAME/ */
	for (v = &mod_url_fn[0]; *v != NULL; ++v) {
		if (!(q = strstr(path, *v)) || strstr(path, tmpl))
			continue;
		if (tpath_size < l) {
			if (!(tpath = malloc(l)))
				break;
			tpath_size = l;
		}

		strlcpy(tpath, path, q - path + 1);
		strlcat(tpath, tmpl, tpath_size);
		strlcat(tpath, q + 1, tpath_size);
		_path = tpath;
		break;
	}

	return _path;
}
#else
static inline char *mod_url_path(char *path, char *tpath, size_t tpath_size) { return path; }
#endif

void do_file(char *path, FILE *stream)
{
	FILE *fp;
	char buf[1024], tmp_path[256];
	char *_path = mod_url_path(path, tmp_path, sizeof(tmp_path));
	int nr;

	if ((fp = fopen(_path, "r")) != NULL) {
		while ((nr = fread(buf, 1, sizeof(buf), fp)) > 0)
			do_fwrite(buf, nr, stream);
		fclose(fp);
	}

	if (_path != path && _path != tmp_path)
		free(_path);
}

#endif

void set_referer_host(void)
{
	int port = 0;
	char *lan_ipaddr = nvram_safe_get("lan_ipaddr");
	const int d_len = strlen(DUT_DOMAIN_NAME);
	const int ip_len = strlen(lan_ipaddr);

	if(!strncmp(DUT_DOMAIN_NAME, host_name, d_len) && *(host_name + d_len) == ':' && (port = atoi(host_name + d_len + 1)) > 0 && port < 65536){	//transfer https domain to ip
		if(port == 80)
			strlcpy(referer_host, lan_ipaddr, sizeof(referer_host));
		else{
			memset(referer_host, 0, sizeof(referer_host));
			snprintf(referer_host,sizeof(referer_host),"%s:%d",lan_ipaddr, port);
		}
#if defined(RTAC68U) || defined(RPAX56) || defined(RPAX58)
	} else if (is_dpsta_repeater() && nvram_get_int("re_mode") == 0
		&& !strncmp("repeater.asus.com", host_name, strlen("repeater.asus.com")) && *(host_name + strlen("repeater.asus.com")) == ':' && (port = atoi(host_name + strlen("repeater.asus.com") + 1)) > 0 && port < 65536){//transfer https domain to ip
		if(port == 80)
			strlcpy(referer_host, lan_ipaddr, sizeof(referer_host));
		else{
			memset(referer_host, 0, sizeof(referer_host));
			snprintf(referer_host,sizeof(referer_host),"%s:%d",lan_ipaddr, port);
		}
#endif
	}else if(!strcmp(DUT_DOMAIN_NAME, host_name))	//transfer http domain to ip
		strlcpy(referer_host, lan_ipaddr, sizeof(referer_host));
#if defined(RTAC68U) || defined(RPAX56) || defined(RPAX58)
	else if (is_dpsta_repeater() && nvram_get_int("re_mode") == 0
		&& !strcmp("repeater.asus.com", host_name))   //transfer http domain to ip
		strlcpy(referer_host, lan_ipaddr, sizeof(referer_host));
#endif
	else if(!strncmp(lan_ipaddr, host_name, ip_len) && *(host_name + ip_len) == ':' && ((port = atoi(host_name + ip_len + 1)) == 80 || port == nvram_get_int("https_lanport")))	//filter send hostip:80/8443
		strlcpy(referer_host, host_name, sizeof(referer_host));
	else if(nvram_match("x_Setting", "0"))
		strlcpy(referer_host, lan_ipaddr, sizeof(referer_host));
	else
		strlcpy(referer_host, host_name, sizeof(referer_host));
}

int is_firsttime(void);

#define APPLYAPPSTR 	"applyapp.cgi"
#define APPGETCGI 	"appGet.cgi"

#ifdef RTCONFIG_ROG
#define APPLYROGSTR     "api.asp"
#endif


#ifdef RTCONFIG_LANTIQ
int wave_handle_flag(char *url)
{
	int ret = 0;

	if(strcmp(url, "qis/QIS_wireless.htm") == 0 ||
		strcmp(url, "QIS_wizard.htm") == 0){
		_dprintf("httpd_handle_request:[%s] from QIS\n", url);
		nvram_set_int("wave_flag", WAVE_FLAG_QIS);
	}else if(strcmp(url, "Advanced_Wireless_Content.asp") == 0){
		_dprintf("httpd_handle_request:[%s] from wireless\n", url);
		nvram_set_int("wave_flag", WAVE_FLAG_NORMAL);
	}else if(strcmp(url, "Advanced_WWPS_Content.asp") == 0){
		_dprintf("httpd_handle_request:[%s] for WPS\n", url);
		nvram_set_int("wave_flag", WAVE_FLAG_WPS);
	}else if(strcmp(url, "Advanced_WMode_Content.asp") == 0){
		_dprintf("httpd_handle_request:[%s] for WDS\n", url);
		nvram_set_int("wave_flag", WAVE_FLAG_WDS);
	}else if(strcmp(url, "Advanced_ACL_Content.asp") == 0){
		_dprintf("httpd_handle_request:[%s] for mac filter\n", url);
		nvram_set_int("wave_flag", WAVE_FLAG_ACL);
	}else if(strcmp(url, "Advanced_WAdvanced_Content.asp") == 0){
		_dprintf("httpd_handle_request:[%s] for advanced wireless\n", url);
		nvram_set_int("wave_flag", WAVE_FLAG_ADV);
	}else if(strcmp(url, "Guest_network.asp") == 0){
		_dprintf("httpd_handle_request:[%s] for guest network\n", url);
		nvram_set_int("wave_flag", WAVE_FLAG_VAP);
	}else if(strcmp(url, "device-map/router.asp") == 0){
		_dprintf("httpd_handle_request:[%s] for networkmap\n", url);
		nvram_set_int("wave_flag", WAVE_FLAG_NETWORKMAP);
	}
	return ret;
}
#endif

int max_lock_time = MAX_LOGIN_BLOCK_TIME;

static void
handle_request(void)
{
#if !defined(RTCONFIG_HND_ROUTER)
	static long flush_cache_t1 = 0;
#endif
	char line[10000], *cur;
	char *method, *path, *protocol, *authorization, *boundary, *alang, *cookies, *referer, *useragent;
	char *cp;
	char *file;
	int len;
	struct mime_handler *handler;
	struct except_mime_handler *exhandler;
	struct mime_referer *doreferer;
	int mime_exception, do_referer, login_state = -1;
	int fromapp=0;
	int cl = 0, flags;
	int referer_result = 1, lock_status = 0, add_try = 0;
#ifdef RTCONFIG_FINDASUS
	int i, isDeviceDiscovery=0;
	char id_local[32],prouduct_id[32];
#endif
	char inviteCode[512];

	/* Initialize the request variables. */
	auth_result = 1;
	url_do_auth = 0;
	authorization = boundary = cookies = referer = useragent = NULL;
	host_name[0] = '\0';
	bzero( line, sizeof line );

	/* Parse the first line of the request. */
	if (fgets(line, sizeof(line), conn_fp) == NULL)
		return;

	method = path = line;
	strsep(&path, " ");
	//while (*path == ' ') path++;
	while (path && *path == ' ') path++;	// oleg patch
	protocol = path;
	strsep(&protocol, " ");
	//while (*protocol == ' ') protocol++;
	while (protocol && *protocol == ' ') protocol++;    // oleg pat
	cp = protocol;
	strsep(&cp, " ");
	if ( !method || !path || !protocol ) {
		send_error( 400, "Bad Request", (char*) 0, "Can't parse request." );
		return;
	}
	cur = protocol + strlen(protocol) + 1;

#ifdef TRANSLATE_ON_FLY
	memset(Accept_Language, 0, sizeof(Accept_Language));
#endif

	/* Parse the rest of the request headers. */
	while ( fgets( cur, line + sizeof(line) - cur, conn_fp ) != (char*) 0 )
	{
		//_dprintf("handle_request:cur = %s\n",cur);
		if ( *cur == '\0' || strcmp( cur, "\n" ) == 0 || strcmp( cur, "\r\n" ) == 0 ) {
			break;
		}
#ifdef TRANSLATE_ON_FLY
		else if ( strncasecmp( cur, "Accept-Language:", 16) == 0 ) {
			if (change_preferred_lang(0)) {
				struct language_table *pLang;
				char lang_buf[256], *p, *saveptr;

				alang = cur + 16;
				strlcpy(lang_buf, alang, sizeof(lang_buf));

				p = lang_buf, strsep(&p, "\r\n");
				for (p = strtok_r(lang_buf, " ,;", &saveptr); p; p = strtok_r(NULL, " ,;", &saveptr))
				{
					for (pLang = language_tables; pLang->Lang != NULL; pLang++)
					{
						if (strcasecmp(p, pLang->Lang) == 0)
						{
							char dictname[32];
							_dprintf("handle_request: pLang->Lang = %s\n", pLang->Lang);
							if (!check_lang_support(pLang->Target_Lang))
								break;

							snprintf(dictname, sizeof(dictname), "%s.dict", pLang->Target_Lang);
							if (!check_if_file_exist(dictname))
								break;

							snprintf(Accept_Language, sizeof(Accept_Language), "%s", pLang->Target_Lang);
							break;
						}
					}

					if (*Accept_Language) {
						nvram_set("preferred_lang", Accept_Language);
						break;
					}
				}

				change_preferred_lang(1);
			}

			#ifdef RTCONFIG_DSL_TCLINUX
			if(is_firsttime()){
				if(nvram_match("preferred_lang", "CZ") || nvram_match("preferred_lang", "DE")) {
					int do_restart = 0;
					if( nvram_match("dslx_annex", "4")
						&& nvram_match("dsltmp_adslsyncsts", "down")
					){
						_dprintf("DSL: auto switch to annex b/j\n");
						nvram_set("dslx_annex", "6");
						do_restart = 1;
					}
					if(nvram_match("preferred_lang", "DE")
						&& nvram_match("dslx_vdsl_profile", "0")) {
						_dprintf("DSL: auto switch to 17a multi mode\n");
						nvram_set("dslx_vdsl_profile", "1");
						do_restart = 1;
					}
					if (do_restart)
						notify_rc("restart_dsl_setting");
				}
			}
			#endif
		}
#endif
		else if ( strncasecmp( cur, "Authorization:", 14 ) == 0 )
		{
			cp = &cur[14];
			cp += strspn( cp, " \t" );
			authorization = cp;
			cur = cp + strlen(cp) + 1;
		}
		else if ( strncasecmp( cur, "User-Agent:", 11 ) == 0 )
		{
			cp = &cur[11];
			cp += strspn( cp, " \t" );
			useragent = cp;
			cur = cp + strlen(cp) + 1;
		}
		else if ( strncasecmp( cur, "Cookie:", 7 ) == 0 )
		{
			cp = &cur[7];
			cp += strspn( cp, " \t" );
			cookies = cp;
			strlcpy(cookies_buf, cookies, sizeof(cookies_buf));
			cur = cp + strlen(cp) + 1;
		}
		else if ( strncasecmp( cur, "Referer:", 8 ) == 0 )
		{
			cp = &cur[8];
			cp += strspn( cp, " \t" );
			referer = cp;
			cur = cp + strlen(cp) + 1;
			//_dprintf("httpd referer = %s\n", referer);
		}
		else if ( strncasecmp( cur, "Host:", 5 ) == 0 )
		{
			cp = &cur[5];
			cp += strspn( cp, " \t" );
			sethost(cp);
			cur = cp + strlen(cp) + 1;
#ifdef RTCONFIG_FINDASUS
			snprintf(prouduct_id, sizeof(prouduct_id), "%s",get_productid());
			for(i = 0 ; i < strlen(prouduct_id) ; i++ ){
				prouduct_id[i] = tolower(prouduct_id[i]) ;
			}
			snprintf(id_local, sizeof(id_local), "%s.local",prouduct_id);
			if(!strncmp(cp, "findasus", 8) || !strncmp(cp, id_local,strlen(id_local)))
				isDeviceDiscovery = 1;
			else
				isDeviceDiscovery = 0;
#endif
		}
		else if (strncasecmp( cur, "Content-Length:", 15 ) == 0) {
			cp = &cur[15];
			cp += strspn( cp, " \t" );
			cl = strtoul( cp, NULL, 0 );
			if(cl < 0){
				send_error( 400, "Bad Request", (char*) 0, "Illegal HTTP Format." );
				return;
			}
		}
		else if (strncasecmp( cur, "Transfer-Encoding:", 18 ) == 0) {
			cp = &cur[18];
			cp += strspn( cp, " \t" );
			if(strstr( cp, "chunked" )){
				send_error( 400, "Bad Request", (char*) 0, "Illegal HTTP Format." );
				return;
			}
		}
		else if ((cp = strstr( cur, "boundary=" ))) {
			boundary = &cp[9];
			for ( cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++ );
			*cp = '\0';
			cur = ++cp;
		}
	}

	slowloris_check();

	if ( strcasecmp( method, "get" ) != 0 && strcasecmp(method, "post") != 0 && strcasecmp(method, "head") != 0 ) {
		send_error( 501, "Not Implemented", (char*) 0, "That method is not implemented." );
		return;
	}

	if ( path[0] != '/' ) {
		send_error( 400, "Bad Request", (char*) 0, "Bad filename." );
		return;
	}
	file = &(path[1]);
	len = strlen( file );
	if ( file[0] == '/' || strcmp( file, ".." ) == 0 || strncmp( file, "../", 3 ) == 0 || strstr( file, "/../" ) != (char*) 0 || strcmp( &(file[len-3]), "/.." ) == 0 ) {
		send_error( 400, "Bad Request", (char*) 0, "Illegal filename." );
		return;
	}

	if(HTS == 1 && strcmp(inet_ntoa(login_usa_tmp.sa_in.sin_addr), "127.0.0.1")){ //allow tunnel pass
		snprintf(inviteCode, sizeof(inviteCode), "<meta http-equiv=\"refresh\" content=\"0; https://%s:%d\">\r\n", gethost(), nvram_get_int("https_lanport"));
		send_page( 307, "Temporary Redirect", (char*) 0, inviteCode, 0);
		return;
	}

//2008.08 magic{
	if (file[0] == '\0' || (index(file, '?') == NULL && file[len-1] == '/')){
		if (is_firsttime()
#ifdef RTCONFIG_FINDASUS
		    && !isDeviceDiscovery
#endif
		   )
			//file = "QIS_wizard.htm";
			file = "QIS_default.cgi";
#ifdef RTCONFIG_FINDASUS
		else if(isDeviceDiscovery == 1)
			file = "find_device.asp";
#endif
		else
			file = indexpage;
	}

	char *query;
	int file_len;

	memset(url, 0, sizeof(url));
	if ((query = index(file, '?')) != NULL) {
		file_len = strlen(file)-strlen(query);

		if(file_len > sizeof(url))
			file_len = sizeof(url);

		strncpy(url, file, file_len);
	}
	else
	{
		strncpy(url, file, sizeof(url)-1);
	}

	if( (strstr(url, ".asp") || strstr(url, ".htm")) && !strstr(url, "update_networkmapd.asp") && !strstr(url, "update_clients.asp") && !strstr(url, "update_customList.asp") ) {
		memset(current_page_name, 0, sizeof(current_page_name));
		snprintf(current_page_name, sizeof(current_page_name), "%s", url);
	}

#if !defined(RTCONFIG_HND_ROUTER)
	if (!strncmp(file, "Main_Login.asp", 14) && (uptime() - flush_cache_t1) > 10 * 60) {
		/* free pagecahe when login, don't do it again in 10 minutes. */
		f_write_string("/proc/sys/vm/drop_caches", "1", 0, 0);
		flush_cache_t1 = uptime();
	}
#endif
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_QCA_LBD)
	if (nvram_match("smart_connect_x", "1") && !nvram_match("wl_unit", "0")
	 && strstr(url, "Advanced_Wireless_Content.asp")) {
		nvram_set("wl_unit", "0");
	}
#endif

	if(strncmp(url, APPLYAPPSTR, strlen(APPLYAPPSTR))==0 
#ifdef RTCONFIG_ROG
		|| strncmp(url, APPLYROGSTR, strlen(APPLYROGSTR))==0
#endif
	)
		fromapp=1;

	memset(user_agent, 0, sizeof(user_agent));
	if(useragent != NULL)
		strncpy(user_agent, useragent, sizeof(user_agent)-1);
	else
		strlcpy(user_agent, "", sizeof(user_agent));

	fromapp = check_user_agent(useragent);

	if(!strcmp(url, APPGETCGI))
		json_support = 1;
	else
		json_support = 0;

#if defined(RTCONFIG_IFTTT) || defined(RTCONFIG_ALEXA) || defined(RTCONFIG_GOOGLE_ASST)
	ifttt_log(url, file);
#endif

#ifdef RTCONFIG_UIDEBUG
        char sysdepPath[128];
	snprintf(sysdepPath, sizeof(sysdepPath), "sysdep/%s/www/", nvram_safe_get("productid"));
	strlcat(sysdepPath, url, sizeof(sysdepPath));
        if(check_if_file_exist(sysdepPath)){
// _dprintf("[httpd] ### GET ### sysdepPath: %s\n", sysdepPath);
		snprintf(sysdepPath, sizeof(sysdepPath), "sysdep/%s/www/", nvram_safe_get("productid"));
		strlcat(sysdepPath, file, sizeof(sysdepPath));
		file = sysdepPath;
// _dprintf("[httpd] file: %s\n", file);
        }
#endif
	HTTPD_DBG("IP(%s), file = %s\nUser-Agent: %s\n", inet_ntoa(login_usa_tmp.sa_in.sin_addr), file, user_agent);
	mime_exception = 0;
	do_referer = 0;

	if(!fromapp) {

		lock_status = check_lock_status(&login_dt);

		if(lock_status == FORCELOCK || lock_status == LOGINLOCK){
			if(strncmp(file, "Main_Login.asp", 14) && !strstr(url, ".png")){
				send_login_page(fromapp, lock_status, url, NULL, 0, NOLOGINTRY);
				return;
			}
		}

		http_login_timeout(&login_uip_tmp, cookies, fromapp);	// 2008.07 James.
		login_state = http_login_check();
		// for each page, mime_exception is defined to do exception handler

		// check exception first
		for (exhandler = &except_mime_handlers[0]; exhandler->pattern; exhandler++) {
			if(match(exhandler->pattern, url))
			{
				mime_exception = exhandler->flag;
				break;
			}
		}

		// check doreferer first
		for (doreferer = &mime_referers[0]; doreferer->pattern; doreferer++) {
			if(match(doreferer->pattern, url))
			{
				do_referer = doreferer->flag;
				break;
			}
		}
	}

	x_Setting = nvram_get_int("x_Setting");

	for (handler = &mime_handlers[0]; handler->pattern; handler++) {
#ifdef RTCONFIG_HTTPS
		if (do_ssl && !strcmp(url, "offline.htm"))
			continue;
#endif
		if (match(handler->pattern, url))
		{
#ifdef RTCONFIG_LANTIQ
			wave_handle_flag(url);
#endif
			nvram_set("httpd_handle_request", url);
			nvram_set_int("httpd_handle_request_fromapp", fromapp);
			if(login_state==3 && !fromapp) { // few pages can be shown even someone else login
				 if(handler->auth || (!strncmp(file, "Main_Login.asp", 14) && login_error_status != 9) || mime_exception&MIME_EXCEPTION_NOPASS)
				{
					if(strcasecmp(method, "post") == 0 && handler->input)	//response post request
						while (cl--) (void)fgetc(conn_fp);

					send_login_page(fromapp, NOLOGIN, NULL, NULL, 0, NOLOGINTRY);
					return;
				}
			}
			if (handler->auth) {
				url_do_auth = 1;
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX6000) || defined(GTAXE16000) || defined(GTAX11000_PRO) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2)
				switch_ledg(LEDG_QIS_FINISH);
#endif
				if ((mime_exception&MIME_EXCEPTION_NOAUTH_FIRST)&&!x_Setting) {
					//skip_auth=1;
				}
#ifdef RTCONFIG_WIFI_SON
				else if((!fromapp && !nvram_match("sw_mode", "1") && (nvram_match("sw_mode", "3") && !nvram_match("cfg_master", "1")) && strcmp(nvram_safe_get("hive_ui"), "") == 0) && nvram_match("wifison_ready","1")){
					snprintf(inviteCode, sizeof(inviteCode), "<meta http-equiv=\"refresh\" content=\"0; url=message.htm\">\r\n");
					send_page( 200, "OK", (char*) 0, inviteCode, 0);
				}
#endif
#ifdef RTCONFIG_AMAS
				//RD can do firmware upgrade, if re_upgrade set to 1.
				else if(!fromapp && (nvram_match("re_mode", "1") || nvram_match("disable_ui", "1")) &&
					nvram_get_int("re_upgrade") == 0 && !check_AiMesh_whitelist(file)){
					snprintf(inviteCode, sizeof(inviteCode), "<meta http-equiv=\"refresh\" content=\"0; url=message.htm\">\r\n");
					send_page( 200, "OK", (char*) 0, inviteCode, 0);
					return;
				}
#endif
				else if((mime_exception&MIME_EXCEPTION_NOAUTH_ALL)) {
				}
				else {
					if(do_referer&CHECK_REFERER){
						referer_result = referer_check(referer, fromapp);
						if(referer_result != 0){
							if(strcasecmp(method, "post") == 0 && handler->input)	//response post request
								while (cl--) (void)fgetc(conn_fp);

							send_login_page(fromapp, referer_result, NULL, NULL, 0, NOLOGINTRY);
							//if(!fromapp) http_logout(&login_uip_tmp, cookies);
							return;
						}
					}
					auth_result = auth_check(url, file, cookies, fromapp, &add_try);
					if (auth_result != 0)
					{
						if(strcasecmp(method, "post") == 0 && handler->input)	//response post request
							while (cl--) (void)fgetc(conn_fp);

						send_login_page(fromapp, auth_result, url, file, auth_check_dt, add_try);
						return;
					}
				}

				if(!fromapp) {
					if (	!strstr(url, "QIS_")
							&& !strstr(url, ".js")
							&& !strstr(url, ".css")
							&& !strstr(url, ".gif")
							&& !strstr(url, ".png")) {
#if defined(RTCONFIG_WIRELESSREPEATER) || defined(RTCONFIG_CONCURRENTREPEATER)					 
						if (nvram_match("x_Setting", "0") && (strstr(url, "start_apply2.htm") || strstr(url, "apscan.asp") || strstr(url, "data:image/")))
							;//TODO
						else
#endif
						http_login(&login_uip_tmp, url);
					}
				}else
					app_http_login(&login_uip_tmp);
			}else{
				if(do_referer&CHECK_REFERER){
					referer_result = check_noauth_referrer(referer, fromapp);
					if(referer_result != 0){
						if(strcasecmp(method, "post") == 0 && handler->input)	//response post request
							while (cl--) (void)fgetc(conn_fp);

						send_login_page(fromapp, referer_result, NULL, NULL, 0, NOLOGINTRY);
						//if(!fromapp) http_logout(&login_uip_tmp, cookies);
						return;
					}
				}
			}

			if(!strcmp(file, "Logout.asp")){
				http_logout(&login_uip_tmp, cookies, fromapp);
				send_login_page(fromapp, ISLOGOUT, NULL, NULL, 0, NOLOGINTRY);
				return;
			}
			if (strcasecmp(method, "post") == 0 && !handler->input) {
				send_error(501, "Not Implemented", NULL, "That method is not implemented.");
				return;
			}
// 2007.11 James. for QISxxx.htm pages }
			if (handler->input) {
				handler->input(file, conn_fp, cl, boundary);
#if defined(linux)
				if ((flags = fcntl(fileno(conn_fp), F_GETFL)) != -1 &&
						fcntl(fileno(conn_fp), F_SETFL, flags | O_NONBLOCK) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp) != EOF)
						(void)fgetc(conn_fp);

					fcntl(fileno(conn_fp), F_SETFL, flags);
				}
#elif defined(vxworks)
				flags = 1;
				if (ioctl(fileno(conn_fp), FIONBIO, (long) &flags) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp) != EOF)
						(void)fgetc(conn_fp);
					flags = 0;
					ioctl(fileno(conn_fp), FIONBIO, (long) &flags);
				}
#endif
			}
			if(!strstr(file, ".cgi") && !strstr(file, "syslog.txt") && !(strstr(file,"uploadIconFile.tar")) && !(strstr(file,"backup_jffs.tar")) && !(strstr(file,"networkmap.tar")) && !(strstr(file,".CFG")) && !(strstr(file,".log")) && !check_if_file_exist(file)
#ifdef RTCONFIG_USB_MODEM
					&& !strstr(file, "modemlog.txt")
#endif
#ifdef RTCONFIG_DSL_TCLINUX
					&& !strstr(file, "TCC.log")
#endif
#ifdef RTCONFIG_IPSEC
					&& !strstr(file, "ipsec.log")
#endif
#if defined(RTCONFIG_IFTTT) || defined(RTCONFIG_ALEXA) || defined(RTCONFIG_GOOGLE_ASST)
					&& !strstr(file, "asustitle.png")
#endif
					&& !strstr(file,"cert.crt")
					&& !strstr(file,"cert_key.tar")
					&& !strstr(file,"cert.tar")
#ifdef RTCONFIG_OPENVPN
					&& !strstr(file, "server_ovpn.cert")
#endif
#ifdef RTCONFIG_CAPTCHA
					&& !strstr(file, "captcha.gif")
#endif
#ifdef RTCONFIG_IPSEC
					&& !strstr(file, "renew_ikev2_cert_mobile.pem") && !strstr(file, "ikev2_cert_mobile.pem")
					&& !strstr(file, "renew_ikev2_cert_windows.der") && !strstr(file, "ikev2_cert_windows.der")
#endif
#ifdef RTCONFIG_WIREGUARD
					&& !strstr(file, "wgs_client.")
#endif
#if defined(DSL_AX82U)
					|| (is_ax5400_i1() &&
						(strstr(file, "Advanced_TR069_Content.asp")
						 || strstr(file, "Advanced_OAM_Content.asp")
						 || strstr(file, "Advanced_ADSL_Content.asp")
						))
#endif
					){
				send_error( 404, "Not Found", (char*) 0, "File not found." );
				return;
			}
			if(nvram_match("x_Setting", "0") && (strcmp(url, "QIS_default.cgi")==0 || strcmp(url, "page_default.cgi")==0 || !strcmp(websGetVar(file, "x_Setting", ""), "1"))){
				if(!fromapp) set_referer_host();
				send_token_headers( 200, "OK", handler->extra_header, handler->mime_type, fromapp);

			}else if(strncmp(url, "login.cgi", strlen(url))!=0){
				send_headers( 200, "OK", handler->extra_header, handler->mime_type, fromapp);
			}
			if (strcasecmp(method, "head") != 0 && handler->output) {
				handler->output(file, conn_fp);
			}
			break;
		}
	}

	if (!handler->pattern){
		if(strlen(file) > 50 && !(strstr(file, "findasus")) && !(strstr(file, "acme-challenge"))){
			memset(cloud_file, 0, sizeof(cloud_file));
			if(!check_xss_blacklist(file, 0))
				strlcpy(cloud_file, file, sizeof(cloud_file));

			snprintf(inviteCode, sizeof(inviteCode), "<meta http-equiv=\"refresh\" content=\"0; url=cloud_sync.asp?flag=%s\">\r\n", cloud_file);
			send_page( 200, "OK", (char*) 0, inviteCode, 0);
		}
		else
			send_error( 404, "Not Found", (char*) 0, "File not found." );
	}
	nvram_unset("httpd_handle_request");
	nvram_unset("httpd_handle_request_fromapp");
}

uaddr *uaddr_ston(const usockaddr *u, uaddr *uip)
{
	memset(uip, 0, sizeof(*uip));
	uip->family = u->sa.sa_family;

	switch (u->sa.sa_family) {
	case AF_INET:
		uip->in = u->sa_in.sin_addr;
		return uip;
#ifdef RTCONFIG_IPV6
	case AF_INET6:
		uip->in6 = u->sa_in6.sin6_addr;
		if (IN6_IS_ADDR_V4MAPPED(&uip->in6)) {
			uip->family = AF_INET;
			uip->in.s_addr = uip->in6.s6_addr32[3];
		}
		return uip;
#endif
	}

	return NULL;
}

uaddr *uaddr_pton(const char *src, uaddr *uip)
{
	memset(uip, 0, sizeof(*uip));

	if (inet_pton(AF_INET, src, &uip->in) > 0) {
		uip->family = AF_INET;
		return uip;
	}
#ifdef RTCONFIG_IPV6
	else if (inet_pton(AF_INET6, src, &uip->in6) > 0) {
		uip->family = AF_INET6;
		if (IN6_IS_ADDR_V4MAPPED(&uip->in6)) {
			uip->family = AF_INET;
			uip->in.s_addr = uip->in6.s6_addr32[3];
		}
		return uip;
	}
#endif

	return NULL;
}

char *uaddr_ntop(const uaddr *uip, char *dst, size_t cnt)
{
	return (char *)inet_ntop(uip->family, &uip->in, dst, cnt);
}

/* IPv6 compat */
unsigned int uaddr_addr(uaddr *uip)
{
	switch (uip->family) {
	case AF_INET:
		return uip->in.s_addr;
#ifdef RTCONFIG_IPV6
	case AF_INET6:
		return uip->in6.s6_addr32[3];
#endif
	}

	return 0;
}

int uaddr_is_unspecified(uaddr *uip)
{
	switch (uip->family) {
	case AF_UNSPEC:
		return 1;
	case AF_INET:
		return (uip->in.s_addr == 0);
#ifdef RTCONFIG_IPV6
	case AF_INET6:
		return IN6_IS_ADDR_UNSPECIFIED(&uip->in6);
#endif
	}

	return 0;
}

int uaddr_is_localhost(uaddr *uip)
{
	switch (uip->family) {
	case AF_INET:
		return (uip->in.s_addr == htonl(INADDR_LOOPBACK));
#ifdef RTCONFIG_IPV6
	case AF_INET6:
		return IN6_IS_ADDR_LOOPBACK(&uip->in6);
#endif
	}

	return 0;
}

int uaddr_is_equal(uaddr *a, uaddr *b)
{
	if (a->family != b->family)
		return 0;
	switch (a->family) {
	case AF_UNSPEC:
		return 1;
	case AF_INET:
		return (a->in.s_addr == b->in.s_addr);
#ifdef RTCONFIG_IPV6
	case AF_INET6:
		return IN6_ARE_ADDR_EQUAL(&a->in6, &b->in6);
#endif
	}

	return 0;
}

uaddr *uaddr_getpeer(webs_t wp, uaddr *uip)
{
	usockaddr peer;
	int fd;

#ifdef RTCONFIG_HTTPS
	if (do_ssl) {
		fd = ssl_stream_fd;
	} else
#endif
	{
		fd = fileno((FILE *)wp);
	}

	peer.sa_len = sizeof(peer);
	if (getpeername(fd, &peer.sa, &peer.sa_len) < 0) {
		perror("getpeername");
		return NULL;
	}

	return uaddr_ston(&peer, uip);
}

//2008 magic{
void http_login_cache(usockaddr *u)
{
	login_timestamp_cache = uptime();
	uaddr_ston(u, &login_uip_tmp);
	login_ip_tmp = uaddr_addr(&login_uip_tmp); /* IPv6 compat */
	login_usa_tmp = *u;
	cur_login_ip_type = check_current_ip_is_lan_or_wan();
	if(cur_login_ip_type == -1)
		_dprintf("[%s, %d]ERROR! Can not check the remote ip!\n", __FUNCTION__, __LINE__);
}

void app_http_login(uaddr *uip)
{
	char tmp[100];

	if ((http_port != SERVER_PORT
	  && http_port != nvram_get_int("http_lanport")
#ifdef RTCONFIG_HTTPS
	  && http_port != SERVER_PORT_SSL
	  && http_port != nvram_get_int("https_lanport")
#endif
	    ) || uaddr_is_localhost(uip))
		return;

	app_login_ip = uaddr_addr(uip); /* IPv6 compat */

	snprintf(tmp, sizeof(tmp), "%lu", uptime());
	nvram_set("app_login_timestamp", tmp);
}

void http_login(uaddr *uip, char *url)
{
	char tmp[100], *login_ip_str;

	if (strncmp(url, "Main_Login.asp", strlen(url)) == 0)
		return;

	if ((http_port != SERVER_PORT
	  && http_port != nvram_get_int("http_lanport")
#ifdef RTCONFIG_HTTPS
	  && http_port != SERVER_PORT_SSL
	  && http_port != nvram_get_int("https_lanport")
#endif
	    ) || uaddr_is_localhost(uip))
		return;

	login_ip = uaddr_addr(uip); /* IPv6 compat */
	snprintf(tmp, sizeof(tmp), "%u", login_ip); /* IPv6 compat */
	nvram_set("login_ip", tmp); /* IPv6 compat */

	login_uip = *uip;
	login_ip_str = uaddr_ntop(&login_uip, tmp, sizeof(tmp));
	nvram_set("login_ip_str", login_ip_str ? : "");

	snprintf(tmp, sizeof(tmp), "%lu", uptime());
	nvram_set("login_timestamp", tmp);
}

// 0: can not login, 1: can login, 2: loginer, 3: not loginer
int http_login_check(void)
{
	if ((http_port != SERVER_PORT
	  && http_port != nvram_get_int("http_lanport")
#ifdef RTCONFIG_HTTPS
	  && http_port != SERVER_PORT_SSL
	  && http_port != nvram_get_int("https_lanport")
#endif
	    ) || uaddr_is_localhost(&login_uip_tmp))
		//return 1;
		return 0;	// 2008.01 James.

	if (uaddr_is_unspecified(&login_uip))
		return 1;
	else if (uaddr_is_equal(&login_uip, &login_uip_tmp))
		return 2;

	return 3;
}

void http_login_timeout(uaddr *uip, char *cookies, int fromapp_flag)
{
	time_t now, login_ts;

//	time(&now);
	now = uptime();
	login_ts = atol(nvram_safe_get("login_timestamp"));

	if ((!uaddr_is_unspecified(&login_uip) && !uaddr_is_equal(&login_uip, uip)) &&
	    ((unsigned long)(now-login_ts) > 60)) //one minitues
	{
		http_logout(&login_uip, cookies, fromapp_flag);
	}
}

void http_logout(uaddr *uip, char *cookies, int fromapp_flag)
{
	if ((uaddr_is_unspecified(uip) || uaddr_is_equal(uip, &login_uip)) && fromapp_flag == 0) {
		login_ip = 0; /* IPv6 compat */
		memset(&login_uip, 0, sizeof(login_uip));
		login_timestamp = 0;

		nvram_set("login_ip", ""); /* IPv6 compat */
		nvram_set("login_ip_str", "");
		nvram_set("login_timestamp", "");
		memset(referer_host, 0, sizeof(referer_host));
		delete_logout_from_list(cookies);
// 2008.03 James. {
		if (change_passwd == 1) {
			change_passwd = 0;
		}
// 2008.03 James. }
	}else if(fromapp_flag != 0){
		delete_logout_from_list(cookies);
}
}
//2008 magic}
//

int is_firsttime(void)
{
#if defined(RTAC58U) || defined(RTAC59U) || defined(RTAX58U) || defined(RTAX56U)
	if (!strncmp(nvram_safe_get("territory_code"), "CX/01", 5)
	 || !strncmp(nvram_safe_get("territory_code"), "CX/05", 5))
		return 0;
	else
#endif
	if (strcmp(nvram_get("x_Setting"), "1")==0)
		return 0;
	else
		return 1;
}

/* str_replace
* @param {char*} source
* @param {char*} find
* @param {char*} rep
* */
char *config_model_name(char *source, char *find,  char *rep){
   int find_L=strlen(find);
   int rep_L=strlen(rep);
   int length=strlen(source)+1;
   int gap=0;

   char *result_t = NULL;
   char *result = (char*)malloc(sizeof(char) * length);
   if(result == NULL)
   	return NULL;
   else
	strlcpy(result, source, length);

   char *former=source;
   char *location= strstr(former, find);

	/* stop searching when there is no finding string */
   while(location!=NULL){
       gap+=(location - former);
       result[gap]='\0';

       length+=(rep_L-find_L);
       result_t = (char*)realloc(result, length * sizeof(char));
       if(result_t == NULL){
       	free(result);
         	return NULL;
         }else
         	result = result_t;
       strlcat(result, rep, length);
       gap+=rep_L;

       former=location+find_L;
       strlcat(result, former, length);

       location= strstr(former, find);
   }
   return result;
}

#ifdef TRANSLATE_ON_FLY
/* Whether a language support should be enabled or not.
 * @lang:
 * @return:
 * 	0:	lang should not be supported.
 *     <0:	invalid parameter.
 *     >0:	lang can be supported.
 */

#ifdef RTCONFIG_AUTODICT
int
load_dictionary (char *lang, pkw_t pkw)
{
	char dfn[16];
	char dummy_buf[16];
	int dict_item_idx;
	char* tmp_ptr;
	int dict_item; // number of dict item, now get from text file
	char *q;
	FILE *dfp = NULL;
	int remain_dict;
	int dict_size = 0;
//	struct timeval tv1, tv2;
	const char *eng_dict = "EN.dict";
#ifndef RELOAD_DICT
	static char loaded_dict[12] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
#endif  // RELOAD_DICT
#ifdef RTCONFIG_DYN_DICT_NAME
	char *dyn_dict_buf;
	char *dyn_dict_buf_new=NULL;
#endif

//printf ("lang=%s\n", lang);

//	gettimeofday (&tv1, NULL);
	if(lang == NULL || strlen(lang) != 2 || !strstr(ALL_LANGS, lang)){
		// if "lang" is invalid, use English as default
		snprintf (dfn, sizeof (dfn), eng_dict);
	} else {
		snprintf (dfn, sizeof (dfn), "%s.dict", lang);
	}

#ifndef RELOAD_DICT
//	printf ("loaded_dict (%s) v.s. dfn (%s)\n", loaded_dict, dfn);
	if (strcmp (dfn, loaded_dict) == 0) {
		return 1;
	}
	release_dictionary (pkw);
#endif  // RELOAD_DICT

	do {
//		 printf("Open (%s) dictionary file.\n", dfn);
//
// Now DICT files all use UTF-8, it is no longer a text file
// it need to use open as binary
//
		dfp = fopen (dfn, "rb");
		if (dfp != NULL)	{
#ifndef RELOAD_DICT
			snprintf (loaded_dict, sizeof (loaded_dict), "%s", dfn);
#endif  // RELOAD_DICT
			break;
		}

//		printf ("Open (%s) failure. errno %d (%s)\n", dfn, errno, strerror (errno));
		if (dfp == NULL && strcmp (dfn, eng_dict) == 0) {
			return 0;
		} else {
			// If we can't open specified language file, use English as default
			snprintf (dfn, sizeof (dfn), eng_dict);
		}
	} while (1);

	memset (pkw, 0, sizeof (kw_t));
	fseek (dfp, 0L, SEEK_END);
	dict_size = ftell (dfp) + 128;
	// skip BOM header length
	dict_size -= 3;
	printf ("dict_size %d\n", dict_size);

#ifdef RTCONFIG_DYN_DICT_NAME
	dyn_dict_buf = (char *) malloc(dict_size);
	fseek (dfp, 0L, SEEK_SET);
	// skip BOM
	fread (dummy_buf, 1, 3, dfp);
	// read to dict string buffer
	memset(dyn_dict_buf, 0, dict_size);
	fread (dyn_dict_buf, 1, dict_size, dfp);
	dyn_dict_buf_new = config_model_name(dyn_dict_buf, "ZVDYNMODELVZ", nvram_safe_get("productid"));

	free(dyn_dict_buf);

	if(dyn_dict_buf_new){
		dict_size = sizeof(char) * strlen(dyn_dict_buf_new);
		pkw->buf = (unsigned char *) (q = malloc (dict_size));
		strlcpy(pkw->buf, dyn_dict_buf_new, dict_size);
		free(dyn_dict_buf_new);
	}
#else
	pkw->buf = (char *) (q = malloc (dict_size));

	fseek (dfp, 0L, SEEK_SET);
	// skip BOM
	fread (dummy_buf, 1, 3, dfp);
	// read to dict string buffer
	memset(pkw->buf, 0, dict_size);
	fread (pkw->buf, 1, dict_size, dfp);
#endif
	// calc how many dict item , dict_item
	remain_dict = dict_size;
	tmp_ptr = (char *) pkw->buf;
	dict_item = 0;
	while (remain_dict>0) {
		if (*tmp_ptr == 0x0a) {
			dict_item++;
			tmp_ptr++;
			remain_dict--;
		}
		else if (*tmp_ptr == 0) {
			break;
		}
		else {
			tmp_ptr++;
			remain_dict--;
		}
	}
	// allocate memory according dict_item
	pkw->idx = malloc (dict_item * sizeof(unsigned char*));

	printf ("dict_item %d\n", dict_item);

	// get all string start and put to pkw->idx
	remain_dict = dict_size;
	for (dict_item_idx = 0; dict_item_idx < dict_item; dict_item_idx++) {
		pkw->idx[dict_item_idx] = (char *) q;
		while (remain_dict>0) {
			if (*q == 0x0a) {
				*q=0;
				q++;
				remain_dict--;
				break;
			}
			if (*q == 0) {
				break;
			}
			q++;
			remain_dict--;
		}
	}

	pkw->len = dict_item;

	fclose (dfp);

	return 1;
}


void
release_dictionary (pkw_t pkw)
{
	if (pkw == NULL)	{
		return;
	}

	//pkw->len = pkw->tlen = 0;
	pkw->len = 0;

	if (pkw->idx != NULL)   {
		free (pkw->idx);
		pkw->idx = NULL;
	}

	if (pkw->buf != NULL)   {
		free (pkw->buf);
		pkw->buf = NULL;
	}
}

char*
search_desc (pkw_t pkw, char *name)
{
	int i;
	char *ret = NULL;
	int dict_idx;
	char name_buf[128];

/*
	printf("search_desc:");
	printf(name);
	printf("\n");
*/

	if (pkw == NULL || (pkw != NULL && pkw->len <= 0)) {
		return NULL;
	}

	// remove equal
	memset(name_buf,0,sizeof(name_buf));
	// minus one for reserver one char for string zero char
	for (i = 0; i<sizeof(name_buf)-1; i++)  {
		if (*name == 0 || *name == '=') {
			break;
		}
		name_buf[i]=*name++;
	}

/*
	for (i = 0; i < pkw->len; ++i)  {
		char *p;
		p = pkw->idx[i];
		if (strncmp (name, p, strlen (name)) == 0) {
			ret = p + strlen (name);
			break;
		}
	}
*/

/*
	printf("search_desc:");
	printf(name_buf);
	printf("\n");
*/

	dict_idx = atoi(name_buf);
//	printf("%d , %d\n",dict_idx,pkw->len);
	if (dict_idx < pkw->len) {
		ret = (char *) pkw->idx[dict_idx];
	}
	else {
		ret = (char *) pkw->idx[0];
	}

/*
	printf("ret:");
	printf(ret);
	printf("\n");
*/

	return ret;
}
#else
int
load_dictionary (char *lang, pkw_t pkw)
{
	char dfn[16];
	char *p, *q;
	FILE *dfp = NULL;
	int dict_size = 0;
//	struct timeval tv1, tv2;
	const char *eng_dict = "EN.dict";
#ifndef RELOAD_DICT
	static char loaded_dict[12] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
#endif  // RELOAD_DICT

//	gettimeofday (&tv1, NULL);
	if(lang == NULL || strlen(lang) != 2 || !strstr(ALL_LANGS, lang)){
		// if "lang" is invalid, use English as default
		snprintf (dfn, sizeof (dfn), eng_dict);
	} else {
		snprintf (dfn, sizeof (dfn), "%s.dict", lang);
	}

#ifndef RELOAD_DICT
//	printf ("loaded_dict (%s) v.s. dfn (%s)\n", loaded_dict, dfn);
	if (strcmp (dfn, loaded_dict) == 0) {
		return 1;
	}
	release_dictionary (pkw);
#endif  // RELOAD_DICT

	do {
//		 printf("Open (%s) dictionary file.\n", dfn);
		dfp = fopen (dfn, "r");
		if (dfp != NULL)	{
#ifndef RELOAD_DICT
			snprintf (loaded_dict, sizeof (loaded_dict), "%s", dfn);
#endif  // RELOAD_DICT
			break;
		}

//		printf ("Open (%s) failure. errno %d (%s)\n", dfn, errno, strerror (errno));
		if (dfp == NULL && strcmp (dfn, eng_dict) == 0) {
			return 0;
		} else {
			// If we can't open specified language file, use English as default
			snprintf (dfn, sizeof (dfn), eng_dict);
		}
	} while (1);

	memset (pkw, 0, sizeof (kw_t));
	fseek (dfp, 0L, SEEK_END);
	dict_size = ftell (dfp) + 128;
//	printf ("dict_size %d\n", dict_size);
	REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));
	pkw->buf = q = malloc (dict_size);

	fseek (dfp, 0L, SEEK_SET);
#if 0
	while (!feof (dfp)) {
		// if pkw->idx is not enough, add 32 item to pkw->idx
		REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));

		fscanf (dfp, "%[^\n]%*c", q);
		if ((p = strchr (q, '=')) != NULL) {
			pkw->idx[pkw->len] = q;
			pkw->len++;
			q = p + strlen (p);
			*q = '\0';
			q++;
		}
	}
#else
	while ((fscanf(dfp, "%[^\n]", q)) != EOF) {
		fgetc(dfp);

		// if pkw->idx is not enough, add 32 item to pkw->idx
		REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));

		if ((p = strchr (q, '=')) != NULL) {
			pkw->idx[pkw->len] = q;
			pkw->len++;
			q = p + strlen (p);
			*q = '\0';
			q++;
		}
	}

#endif

	fclose (dfp);
//	gettimeofday (&tv2, NULL);
//	printf("Load %d keywords spent %ldms\n", pkw->len, ((tv2.tv_sec * 1000000 + tv2.tv_usec) - (tv1.tv_sec * 1000000 + tv1.tv_usec)) / 1000);

	return 1;
}


void
release_dictionary (pkw_t pkw)
{
	if (pkw == NULL)	{
		return;
	}

	pkw->len = pkw->tlen = 0;

	if (pkw->idx != NULL)   {
		free (pkw->idx);
		pkw->idx = NULL;
	}

	if (pkw->buf != NULL)   {
		free (pkw->buf);
		pkw->buf = NULL;
	}
}

char*
search_desc (pkw_t pkw, char *name)
{
	int i;
	char *p, *ret = NULL;

	if (pkw == NULL || (pkw != NULL && pkw->len <= 0)) {
		return NULL;
	}
	for (i = 0; i < pkw->len; ++i)  {
		p = pkw->idx[i];
		if (strncmp (name, p, strlen (name)) == 0) {
			ret = p + strlen (name);
			break;
		}
	}

	return ret;
}
#endif
#endif //TRANSLATE_ON_FLY

void check_alive()
{
	check_alive_flag = 1;
	static int check_alive_count = 0;

	if(alarm_timestamp != alarm_tv.tv_sec){
		alarm_timestamp = alarm_tv.tv_sec;
		check_alive_count = 0;
	}
	else if(check_alive_count > 20){
		struct in_addr ip_addr, temp_ip_addr, app_temp_ip_addr;
		ip_addr.s_addr = login_ip;
		app_temp_ip_addr.s_addr = app_login_ip;
		temp_ip_addr.s_addr = login_ip_tmp;
		//dbg("slow_post_read_count(%d) > 3\n", slow_post_read_count);
		HTTPD_FB_DEBUG("login_ip = %s(%lu), app_login_ip = %s(%lu)\n", inet_ntoa(ip_addr), login_ip, inet_ntoa(app_temp_ip_addr), app_login_ip);
		HTTPD_FB_DEBUG("login_ip_tmp = %s(%lu), url = %s\n", inet_ntoa(temp_ip_addr), login_ip_tmp, url);
		logmessage("HTTPD", "waitting 10 minitues and restart\n");
		check_lock_state();
		notify_rc("restart_httpd");
	}
	else{
		slow_post_read_check();
		check_alive_count++;
	}

	alarm(20);
}

void httpd_exit(int sig)
{
        remove(pidfile);
        exit(0);
}

int enabled_http_ifname()
{
#ifdef DSL_AX82U
	if (nvram_get_int("http_enable") == 1 && http_port == SERVER_PORT && is_ax5400_i1()){
		HTS = 1; //HTTP Transport Security
		return 1;
	}
#endif
#ifdef RTCONFIG_AIHOME_TUNNEL
	if (nvram_get_int("http_enable") == 1 && http_port == SERVER_PORT)
		return 0;
#endif

	return 1;
}

int main(int argc, char **argv)
{
	usockaddr usa;
	int listen_fd[3];
	fd_set active_rfds;
	conn_list_t pool;
	int i, c;
	//int do_ssl = 0;

	do_ssl = 0; // default

	/* set initial TZ to avoid mem leaks
	 * it suppose to be convert after applying
	 * time_zone_x_mapping(); */
#ifndef RTCONFIG_AVOID_TZ_ENV
	setenv("TZ", nvram_safe_get_x("", "time_zone_x"), 1);
#endif

#ifdef RTCONFIG_LETSENCRYPT
	nvram_unset("le_restart_httpd");
#endif

	if (nvram_get_int("HTTPD_DBG") > 0)
		eval("touch", HTTPD_DEBUG);

#if defined(RTCONFIG_SW_HW_AUTH)
	//if(!httpd_sw_hw_check()) return 0;
#endif
	// usage : httpd -s -p [port]
	while ((c = getopt(argc, argv, "sp:i:")) != -1) {
		switch (c) {
		case 's':
#ifdef RTCONFIG_HTTPS
			do_ssl = 1;
#endif
			break;
		case 'p':
			http_port = atoi(optarg);
			break;
		case 'i':
			http_ifname = optarg;
			break;
#ifdef RTCONFIG_IPV6
		case '6':
			http_ipv6_only = 1;
			break;
#endif
		default:
			fprintf(stderr, "ERROR: unknown option %c\n", c);
			break;
		}
	}

	/* -p might be specified before -s */
	if (http_port == 0) {
#ifdef RTCONFIG_HTTPS
		if (do_ssl) {
			http_port = SERVER_PORT_SSL;
		} else
#endif
		{
			http_port = SERVER_PORT;
		}
	}

	//websSetVer();
	//2008.08 magic
	nvram_unset("login_timestamp");
	nvram_unset("login_ip"); /* IPv6 compat */
	nvram_unset("login_ip_str");

	/* Ignore broken pipes */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, chld_reap);
	signal(SIGUSR1, update_wlan_log);
	signal(SIGALRM, check_alive);
	signal(SIGTERM, httpd_exit);

	alarm(20);

	/* Initialize listen socket */
	for (i = 0; i < ARRAY_SIZE(listen_fd); i++)
		listen_fd[i] = -1;
	i = 0;

	{
		if (
#ifdef RTCONFIG_IPV6
			!http_ipv6_only &&
#endif
			enabled_http_ifname() && (listen_fd[i++] = initialize_listen_socket(AF_INET, &usa, http_ifname)) < 0) {
			fprintf(stderr, "can't bind to %s ipv4 address\n", http_ifname ? : "any");
			return errno;
		}
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && enabled_http_ifname() &&
		    (listen_fd[i++] = initialize_listen_socket(AF_INET6, &usa, http_ifname)) < 0) {
			fprintf(stderr, "can't bind to %s ipv6 address\n", http_ifname ? : "any");
			return errno;
		}
#endif
	}
	if ((http_ifname && strcmp(http_ifname, "lo") != 0) &&
	    (listen_fd[i++] = initialize_listen_socket(AF_INET, &usa, "lo")) < 0) {
		fprintf(stderr, "can't bind to %s address\n", "loopback");
		/* allow fail if previous bind to interface was ok */
		/* return errno; */
	}

	FILE *pid_fp;
	if (http_port == SERVER_PORT)
		strlcpy(pidfile, "/var/run/httpd.pid", sizeof(pidfile));
	else
		snprintf(pidfile, sizeof(pidfile), "/var/run/httpd-%s-%d.pid", http_ifname, http_port);
	if (!(pid_fp = fopen(pidfile, "w"))) {
		perror(pidfile);
		return errno;
	}
	fprintf(pid_fp, "%d", getpid());
	fclose(pid_fp);

	/* Init connection pool */
	FD_ZERO(&active_rfds);
	TAILQ_INIT(&pool.head);
	pool.count = 0;

	/* handler global variable */
	get_index_page(indexpage, sizeof(indexpage));
	get_wl_nband_list();
#if defined(RTCONFIG_SW_HW_AUTH) && defined(RTCONFIG_AMAS)
	amas_support = getAmasSupportMode();
#endif
	if(nvram_get_int("x_Setting") == 0){
		save_ui_support_to_file();
		save_iptvSettings_to_file();
	}
#ifdef RTCONFIG_JFFS2USERICON
	renew_upload_icon();
#endif

#ifdef RTCONFIG_HTTPS
	if(do_ssl){
		strlcpy(HTTPD_LOGIN_FAIL_LAN, "httpds_login_fail_lan", sizeof(HTTPD_LOGIN_FAIL_LAN));
		strlcpy(HTTPD_LOGIN_FAIL_WAN, "httpds_login_fail_wan", sizeof(HTTPD_LOGIN_FAIL_WAN));
		strlcpy(HTTPD_LAST_LOGIN_TS, "httpds_last_login_ts", sizeof(HTTPD_LAST_LOGIN_TS));
		strlcpy(HTTPD_LAST_LOGIN_TS_W, "httpds_last_login_ts_w", sizeof(HTTPD_LAST_LOGIN_TS_W));
		strlcpy(CAPTCHA_FAIL_NUM, "httpds_captcha_fail_num", sizeof(CAPTCHA_FAIL_NUM));
		strlcpy(HTTPD_LOCK_NUM, "httpds_lock_num", sizeof(HTTPD_LOCK_NUM));
	}
	else
#endif
	{
		strlcpy(HTTPD_LOGIN_FAIL_LAN, "httpd_login_fail_lan", sizeof(HTTPD_LOGIN_FAIL_LAN));
		strlcpy(HTTPD_LOGIN_FAIL_WAN, "httpd_login_fail_wan", sizeof(HTTPD_LOGIN_FAIL_WAN));
		strlcpy(HTTPD_LAST_LOGIN_TS, "httpd_last_login_ts", sizeof(HTTPD_LAST_LOGIN_TS));
		strlcpy(HTTPD_LAST_LOGIN_TS_W, "httpd_last_login_ts_w", sizeof(HTTPD_LAST_LOGIN_TS_W));
		strlcpy(CAPTCHA_FAIL_NUM, "httpd_captcha_fail_num", sizeof(CAPTCHA_FAIL_NUM));
		strlcpy(HTTPD_LOCK_NUM, "httpd_lock_num", sizeof(HTTPD_LOCK_NUM));
	}

#ifdef RTCONFIG_HTTPS
reload_cert:
	if (do_ssl)
		start_ssl(http_port);
#endif

	/* Loop forever handling requests */
	for (;;) {
		const static struct timeval timeout = { .tv_sec = MAX_CONN_TIMEOUT, .tv_usec = 0 };
		struct timeval tv;
		fd_set rfds;
		conn_item_t *item, *next;
		int max_fd, count;

		/* record alive flag */
		if(check_alive_flag == 1){
			clean_ban_ip_timeout();
			alarm_tv.tv_sec = uptime();
			check_alive_flag = 0;
		}

		memcpy(&rfds, &active_rfds, sizeof(rfds));
		max_fd = -1;
		if (pool.count < MAX_CONN_ACCEPT) {
			for (i = 0; i < ARRAY_SIZE(listen_fd); i++) {
				if (listen_fd[i] < 0)
					continue;
				FD_SET(listen_fd[i], &rfds);
				max_fd = max(listen_fd[i], max_fd);
			}
		}
		TAILQ_FOREACH(item, &pool.head, entry)
			max_fd = max(item->fd, max_fd);

		/* Wait for new connection or incoming request */
		tv = timeout;
		while ((count = select(max_fd + 1, &rfds, NULL, NULL, &tv)) < 0 && errno == EINTR)
			continue;
#ifdef RTCONFIG_HTTPS
		if (do_ssl) {
			int reload_cert = 0;
#if defined(RTCONFIG_IPV6)
			if (http_ipv6_only)
				reload_cert = nvram_get_int("httpds6_reload_cert");
			else
#endif
				reload_cert = nvram_get_int("httpds_reload_cert");

			if (reload_cert == 1
			 || (reload_cert == 2 && *nvram_safe_get("login_ip") == '\0')) {
				mssl_ctx_free();
				goto reload_cert;
			}
		}
#endif
		if (count < 0) {
			HTTPD_DBG("count = %d : return\n", count);
			perror("select");
			return errno;
		}

		/* Reuse timestamp */
		tv.tv_sec = uptime();

		/* Check and accept new connection */
		for (i = 0; count && i < ARRAY_SIZE(listen_fd); i++) {
			if (listen_fd[i] < 0 || !FD_ISSET(listen_fd[i], &rfds))
				continue;

			count--;

			item = malloc(sizeof(*item));
			if (item == NULL) {
				HTTPD_DBG("malloc fail\n");
				perror("malloc");
				return errno;
			}
			item->usa.sa_len = sizeof(item->usa);
			while ((item->fd = accept(listen_fd[i], &item->usa.sa, &item->usa.sa_len)) < 0 && errno == EINTR)
				continue;
			if (item->fd < 0) {
				HTTPD_DBG("item->fd = %d (<0): continue\n", item->fd);
				perror("accept");
				free(item);
				continue;
			}
			/* Set receive/send timeouts */
			setsockopt(item->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
			setsockopt(item->fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

			/* Set the KEEPALIVE option to cull dead connections */
			setsockopt(item->fd, SOL_SOCKET, SO_KEEPALIVE, &int_1, sizeof(int_1));
			item->deadline = tv.tv_sec + MAX_CONN_TIMEOUT;

			/* Add to active connections */
			FD_SET(item->fd, &active_rfds);
			TAILQ_INSERT_TAIL(&pool.head, item, entry);
			pool.count++;
		}
		/* Continue waiting over again */
		if (count == 0)
			continue;

		/* Check and process pending or expired requests */
		TAILQ_FOREACH_SAFE(item, &pool.head, entry, next) {
			if (item->deadline > tv.tv_sec && !FD_ISSET(item->fd, &rfds))
				continue;

			/* Delete from active connections */
			FD_CLR(item->fd, &active_rfds);
			TAILQ_REMOVE(&pool.head, item, entry);
			pool.count--;

			/* Process request if any */
			if (FD_ISSET(item->fd, &rfds)) {
#ifdef RTCONFIG_HTTPS
				if (do_ssl) {
					ssl_stream_fd = item->fd;
					if (!(conn_fp = ssl_server_fopen(item->fd))) {
						HTTPD_DBG("fdopen(ssl): skip\n");
						perror("fdopen(ssl)");
						goto reset;
					}
				} else
#endif
				if (!(conn_fp = fdopen(item->fd, "r+"))) {
					HTTPD_DBG("fdopen: skip\n");
					perror("fdopen");
					goto reset;
				}

				http_login_cache(&item->usa);

				if(filter_ban_ip())
				{
					handle_request();
				}
				fflush(conn_fp);
#ifdef RTCONFIG_HTTPS
				if (!do_ssl)
#endif
				{
					shutdown(item->fd, SHUT_RDWR);
					item->fd = -1;
				}
				fclose(conn_fp);
			} else {
			/* Reset connection */
			reset:
				setsockopt(item->fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
			}

			/* Close timed out and/or still alive */
			if (item->fd >= 0) {
				shutdown(item->fd, SHUT_RDWR);
				close(item->fd);
			}

			free(item);
		}
	}

	for (i = 0; i < ARRAY_SIZE(listen_fd); i++) {
		if (listen_fd[i] < 0)
			continue;
		shutdown(listen_fd[i], 2);
		close(listen_fd[i]);

#ifdef RTCONFIG_HTTPS
		if (do_ssl) mssl_ctx_free();
#endif
	}

	return 0;
}

#ifdef RTCONFIG_HTTPS
void start_ssl(int http_port)
{
	int lock;
	int ok;
	int save;
	int i;
	int retry;
	unsigned long long sn;
	char t[32];

	lock = file_lock("httpd");

	// Avoid collisions if another httpd instance is initializing SSL cert
	for (i = 1; i < 5; i++) {
		if (lock < 0) {
			//logmessage("httpd", "Conflict, waiting %d", i);
			sleep(i*i);
		} else {
			i = 5;
		}
	}

	if (f_exists(HTTPS_CA_JFFS) && (!f_exists(HTTPD_ROOTCA_CERT) || !f_exists(HTTPD_ROOTCA_KEY) || !f_exists(HTTPD_CERT) || !f_exists(HTTPD_KEY)))
		restore_cert();

	if (nvram_match("https_crt_gen", "1"))
		erase_cert();

	retry = 1;
	while (1) {
		save = nvram_match("https_crt_save", "1");

		/* check selected key/cert pairs */
		if (!f_exists(HTTPD_CERT) || !f_exists(HTTPD_KEY)
		 || !mssl_cert_key_match(HTTPD_CERT, HTTPD_KEY)
		) {
			ok = 0;
			if (save) {
				logmessage("httpd", "Restore saved SSL certificate...%d", http_port);
				if (restore_cert())
					ok = 1;
			}
			if (!ok) {
				erase_cert();
				logmessage("httpd", "Generating SSL certificate...%d", http_port);
				// browsers seems to like this when the ip address moves...	-- zzz
				f_read("/dev/urandom", &sn, sizeof(sn));

				snprintf(t, sizeof(t), "%llu", sn & 0x7FFFFFFFFFFFFFFFULL);
				GENCERT_SH(t);
			}
		}

		if (mssl_init(HTTPD_CERT, HTTPD_KEY)) {
			logmessage("httpd", "Succeed to init SSL certificate...%d", http_port);
			/* Backup certificates if httpds initialization successful. */
			if (save)
				save_cert();

			/* Unset reload flag if set */
#if defined(RTCONFIG_IPV6)
			if (!http_ipv6_only && nvram_get("httpds_reload_cert"))
				nvram_unset("httpds_reload_cert");
			else if (http_ipv6_only && nvram_get("httpds6_reload_cert"))
				nvram_unset("httpds6_reload_cert");
#else
			if (nvram_get("httpds_reload_cert"))
				nvram_unset("httpds_reload_cert");
#endif
			file_unlock(lock);
			return;
		}

		logmessage("httpd", "Failed to initialize SSL, generating new key/cert...%d", http_port);
		erase_cert();

		if (!retry) {
			logmessage("httpd", "Unable to start in SSL mode, exiting! %d", http_port);
			file_unlock(lock);
			exit(1);
		}
		retry = 0;
	}
}
#endif

//return value: 0: LAN,  1: WAN,  -1: ERROR
int check_current_ip_is_lan_or_wan()
{
	struct in_addr lan, mask;

	if (uaddr_is_unspecified(&login_uip_tmp))
		return -1;

	switch (login_uip_tmp.family) {
	case AF_INET:
		if (inet_aton(nvram_safe_get("lan_ipaddr"), &lan) == 0 ||
		    inet_aton(nvram_safe_get("lan_netmask"), &mask) == 0)
			return -1;
		return ((lan.s_addr & mask.s_addr) == (login_uip_tmp.in.s_addr & mask.s_addr))?0:1;
#ifdef RTCONFIG_IPV6
	case AF_INET6:
		/* IPv6 addresses are dynamic, must be bind to bind to interface */
		return 0;
#endif
	}

	return -1;
}
