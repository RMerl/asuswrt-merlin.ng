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
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <rc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <bcmnvram.h>
#include <shutils.h>

#include <stdarg.h>
#include <netdb.h>	// for struct addrinfo
#include <net/ethernet.h>
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX
#include <time.h>

#if defined(RTCONFIG_FBWIFI)
#include "../fb_wifi/fbwifi.h"
#endif

#ifdef RTCONFIG_PROTECTION_SERVER
#include <protect_srv.h>
#endif

#ifdef RTCONFIG_WIFI_SON
#include <qca.h>
#endif

#define WEBSTRFILTER 1
#define CONTENTFILTER 1

/* addr types */
#define TYPE_IP      0
#define TYPE_MAC     1
#define TYPE_IPRANGE 2

#define foreach_x(x)	for (i=0; i<nvram_get_int(x); i++)

#define ACCESS_WEBUI  0x01
#define ACCESS_SSH    0x02
#define ACCESS_TELNET 0x04
#define ACCESS_ALL    0x07


#define STOP_TIME  0
#define START_TIME 1

#ifdef RTCONFIG_IPV6
char wan6face[IFNAMSIZ + 1];
// RFC-4890, sec. 4.3.1
const int allowed_icmpv6[] = { 1, 2, 3, 4, 128, 129 };
// RFC-4890, sec. 4.4.1
const int allowed_local_icmpv6[] =
	{ 130, 131, 132, 133, 134, 135, 136,
	  141, 142, 143,
	  148, 149, 151, 152, 153 };
#endif

#ifdef RTCONFIG_VPN_FUSION
extern int write_vpn_fusion_nat(FILE *fp, const char* lan_ip);
#endif

char *mac_conv(char *mac_name, int idx, char *buf);	// oleg patch

void write_porttrigger(FILE *fp, char *wan_if, int is_nat);
#ifdef WEB_REDIRECT
void redirect_setting();
#endif

struct datetime{
	char start[7];		// start time
	char stop[7];		// stop time
	char tmpstop[7];	// cross-night stop time
} __attribute__((packed));

char *g_buf;
char g_buf_pool[1024];

void g_buf_init()
{
	g_buf = g_buf_pool;
}

char *g_buf_alloc(char *g_buf_now)
{
	g_buf += strlen(g_buf_now)+1;

	return (g_buf_now);
}

/**
 * Check whether DMZ is enabled or not.
 * @return:
 * 	0:	non of any DMZ are enabled.
 *  otherwizse:	one or more DMZ are enabled.
 */
static inline int dmz_enabled(void)
{
	int dmz_enabled = !illegal_ipv4_address(nvram_safe_get("dmz_ip")), dmz1_enabled = 0;

#if defined(RTCONFIG_MULTIWAN_CFG)
	dmz1_enabled = (get_nr_wan_unit() == 2) && nvram_match("wans_mode", "lb") && !nvram_match("dmz1_ip", "");
#endif

	return dmz_enabled || dmz1_enabled;
}

/* overwrite all '/' with '_' */
static void remove_slash(char *str)
{
	char *p = str;

	if (!str || *str == '\0')
		return;

	while ((p = strchr(str, '/')) != NULL)
		*p++ = '_';
}

int host_addr_info(const char *name, int af, struct sockaddr_storage *buf)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *p;
	int err;
	int addrtypes = 0;

	memset(&hints, 0, sizeof hints);
#ifdef RTCONFIG_IPV6
	switch (af & (IPT_V4 | IPT_V6)) {
	case IPT_V4:
		hints.ai_family = AF_INET;
		break;
	case IPT_V6:
		hints.ai_family = AF_INET6;
		break;
	//case (IPT_V4 | IPT_V6):
	//case 0: // error?
	default:
		hints.ai_family = AF_UNSPEC;
	}
#else
	hints.ai_family = AF_INET;
#endif
	hints.ai_socktype = SOCK_RAW;

	if ((err = getaddrinfo(name, NULL, &hints, &res)) != 0) {
		return addrtypes;
	}

	for(p = res; p != NULL; p = p->ai_next) {
		switch(p->ai_family) {
		case AF_INET:
			addrtypes |= IPT_V4;
			break;
		case AF_INET6:
			addrtypes |= IPT_V6;
			break;
		}
		if (buf && (hints.ai_family == p->ai_family) && p->ai_addrlen) {
			memcpy(buf, p->ai_addr, p->ai_addrlen);
		}
	}

	freeaddrinfo(res);
	return (addrtypes & af);
}

inline int host_addrtypes(const char *name, int af)
{
	return host_addr_info(name, af, NULL);
}

int ipt_addr_compact(const char *s, int af, int strict)
{
	char p[INET6_ADDRSTRLEN * 2];
	int r = 0;

	if ((s) && (*s))
	{
		if (sscanf(s, "%[0-9.]-%[0-9.]", p, p) == 2) {
			r = IPT_V4;
		}
#ifdef RTCONFIG_IPV6
		else if (sscanf(s, "%[0-9A-Fa-f:]-%[0-9A-Fa-f:]", p, p) == 2) {
			r = IPT_V6;
		}
#endif
		else {
			if (sscanf(s, "%[^/]/", p)) {
#ifdef RTCONFIG_IPV6
				r = host_addrtypes(p, strict ? af : (IPT_V4 | IPT_V6));
#else
				r = host_addrtypes(p, IPT_V4);
#endif
			}
		}
	}
	else
	{
		r = (IPT_V4 | IPT_V6);
	}

	return (r & af);
}

/*
void nvram_unsets(char *name, int count)
{
	char itemname_arr[32];
	int i;

	for (i=0; i<count; i++)
	{
		sprintf(itemname_arr, "%s%d", name, i);
		nvram_unset(itemname_arr);
	}
}
*/

static int addr_type_parse(const char *src, char *dst, int size)
{
	char addr[100], *next = addr;
	struct in_addr min_addr, max_addr;
	int c;

	if (!src || *src == '\0' || strcmp(src, "ALL") == 0)
		return -1;

	/* XX:XX:XX:XX:XX:XX */
	if (sscanf(src, "%2x:%2x:%2x:%2x:%2x:%2x", &c, &c, &c, &c, &c, &c) == 6) {
		strlcpy(dst, src, size);
		return TYPE_MAC;
	}

	strlcpy(addr, src, sizeof(addr));

	/* 0.0.0.0-255
	 * 0.0.0.0-255.255.255.255 */
	next = addr, strsep(&next, "-");
	if (next && *next) {
		min_addr.s_addr = inet_addr_(addr);
		max_addr.s_addr = (strchr(next, '.') != NULL) ? inet_addr_(next) :
			(min_addr.s_addr & htonl(0xffffff00)) | htonl(atoi(next) & 0xff);
		if (min_addr.s_addr == max_addr.s_addr &&
		    min_addr.s_addr == INADDR_ANY)
			return -1;
		if (ntohl(min_addr.s_addr) > ntohl(max_addr.s_addr))
			return -1;

		strlcpy(addr, inet_ntoa(min_addr), sizeof(addr));
		snprintf(dst, size, "%s-%s", addr, inet_ntoa(max_addr));
		return TYPE_IPRANGE;
	}

	/* 255.255.255.255
	 * 255.255.255.255/32 */
	next = addr, strsep(&next, "/");
	if (inet_addr_(addr) == INADDR_ANY)
		return -1;

	strlcpy(dst, src, size);
	return TYPE_IP;
}


char *proto_conv(char *proto, char *buf)
{
	if (!strncasecmp(proto, "BOTH", 3)) strcpy(buf, "both");
	else if (!strncasecmp(proto, "TCP", 3)) strcpy(buf, "tcp");
	else if (!strncasecmp(proto, "UDP", 3)) strcpy(buf, "udp");
	else if (!strncasecmp(proto, "OTHER", 5)) strcpy(buf, "other");
	else strcpy(buf,"tcp");
	return buf;
}

char *protoflag_conv(char *proto, char *buf, int isFlag)
{
	if (!isFlag)
	{
		if (strncasecmp(proto, "UDP", 3)==0) strcpy(buf, "udp");
		else strcpy(buf, "tcp");
	}
	else
	{
		if (strlen(proto)>3)
		{
			strcpy(buf, proto+4);
		}
		else strcpy(buf,"");
	}
	return (buf);
}
/*
char *portrange_ex_conv(char *port_name, int idx)
{
	char *port, *strptr;
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_get(itemname_arr);

	strcpy(g_buf, "");

	if (!strncmp(port, ">", 1)) {
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
	}
	else if (!strncmp(port, "=", 1)) {
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
	}
	else if (!strncmp(port, "<", 1)) {
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
	}
	//else if (strptr=strchr(port, ':'))
	else if ((strptr=strchr(port, ':')) != NULL) //2008.11 magic oleg patch
	{
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d-%d", atoi(itemname_arr), atoi(strptr+1));
	}
	else if (*port)
	{
		sprintf(g_buf, "%d-%d", atoi(port), atoi(port));
	}
	else
	{
		//sprintf(g_buf, "");
		g_buf[0] = 0;	// oleg patch
	}

	return (g_buf_alloc(g_buf));
}
*/

char *portrange_ex2_conv(char *port_name, int idx, int *start, int *end)
{
	char *port, *strptr;
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_get(itemname_arr);

	strcpy(g_buf, "");

	if (!strncmp(port, ">", 1))
	{
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
		*start=atoi(port+1);
		*end=65535;
	}
	else if (!strncmp(port, "=", 1))
	{
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
		*start=*end=atoi(port+1);
	}
	else if (!strncmp(port, "<", 1))
	{
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
		*start=1;
		*end=atoi(port+1);
	}
	//else if (strptr=strchr(port, ':'))
	else if ((strptr=strchr(port, ':')) != NULL) //2008.11 magic oleg patch
	{
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d-%d", atoi(itemname_arr), atoi(strptr+1));
		*start=atoi(itemname_arr);
		*end=atoi(strptr+1);
	}
	else if (*port)
	{
		sprintf(g_buf, "%d-%d", atoi(port), atoi(port));
		*start=atoi(port);
		*end=atoi(port);
	}
	else
	{
		//sprintf(g_buf, "");
		 g_buf[0] = 0;	// oleg patch
		*start=0;
		*end=0;
	}

	return (g_buf_alloc(g_buf));
}

char *portrange_ex2_conv_new(char *port_name, int idx, int *start, int *end)
{
	char *port, *strptr;
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_get(itemname_arr);

	strcpy(g_buf, "");

	if (!strncmp(port, ">", 1))
	{
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
		*start=atoi(port+1);
		*end=65535;
	}
	else if (!strncmp(port, "=", 1))
	{
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
		*start=*end=atoi(port+1);
	}
	else if (!strncmp(port, "<", 1))
	{
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
		*start=1;
		*end=atoi(port+1);
	}
	else if ((strptr=strchr(port, ':')) != NULL)
	{
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d:%d", atoi(itemname_arr), atoi(strptr+1));
		*start=atoi(itemname_arr);
		*end=atoi(strptr+1);
	}
	else if (*port)
	{
		sprintf(g_buf, "%d:%d", atoi(port), atoi(port));
		*start=atoi(port);
		*end=atoi(port);
	}
	else
	{
		//sprintf(g_buf, "");
		 g_buf[0] = 0;	// oleg patch
		*start=0;
		*end=0;
	}

	return (g_buf_alloc(g_buf));
}

char *portrange_conv(char *port_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", port_name, idx);
	strcpy(g_buf, nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}
/*
char *iprange_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	//strcpy(g_buf, "");
	 g_buf[0] = 0;	// 0313

	// scan all ip string
	i=j=k=0;

	while (*(ip+i))
	{
		if (*(ip+i)=='*')
		{
			startip[j++] = '1';
			endip[k++] = '2';
			endip[k++] = '5';
			endip[k++] = '4';
			// 255 is for broadcast
		}
		else
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		i++;
	}

	startip[j++] = 0;
	endip[k++] = 0;

	if (strcmp(startip, endip)==0)
		sprintf(g_buf, "%s", startip);
	else
		sprintf(g_buf, "%s-%s", startip, endip);
	return (g_buf_alloc(g_buf));
}

char *iprange_ex_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	int mask;

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	strcpy(g_buf, "");

	// scan all ip string
	i=j=k=0;
	mask=32;

	while (*(ip+i))
	{
		if (*(ip+i)=='*')
		{
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		i++;
	}

	startip[j++] = 0;
	endip[k++] = 0;

	if (mask==32)
		sprintf(g_buf, "%s", startip);
	else if (mask==0)
		strcpy(g_buf, "");
	else sprintf(g_buf, "%s/%d", startip, mask);

	return (g_buf_alloc(g_buf));
}
*/
char *ip_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));
	return (g_buf_alloc(g_buf));
}

char *general_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));
	return (g_buf_alloc(g_buf));
}

char *filter_conv(char *proto, char *flag, char *srcip, char *srcport, char *dstip, char *dstport)
{
	char newstr[64];

	_dprintf("filter : %s,%s,%s,%s,%s,%s\n", proto, flag, srcip, srcport, dstip, dstport);

	strcpy(g_buf, "");

	if (strcmp(proto, "")!=0)
	{
		sprintf(newstr, " -p %s", proto);
		strcat(g_buf, newstr);
	}

	if (strcmp(flag, "")!=0)
	{
		//sprintf(newstr, " --tcp-flags %s RST", flag);
		sprintf(newstr, " --tcp-flags %s %s", flag, flag);
		strcat(g_buf, newstr);
	}

	if (strcmp(srcip, "")!=0)
	{
		if (strchr(srcip , '-'))
			sprintf(newstr, " --src-range %s", srcip);
		else
			sprintf(newstr, " -s %s", srcip);
		strcat(g_buf, newstr);
	}

	if (strcmp(srcport, "")!=0)
	{
		sprintf(newstr, " --sport %s", srcport);
		strcat(g_buf, newstr);
	}

	if (strcmp(dstip, "")!=0)
	{
		if (strchr(dstip, '-'))
			sprintf(newstr, " --dst-range %s", dstip);
		else
			sprintf(newstr, " -d %s", dstip);
		strcat(g_buf, newstr);
	}

	if (strcmp(dstport, "")!=0)
	{
		sprintf(newstr, " --dport %s", dstport);
		strcat(g_buf, newstr);
	}
	return (g_buf);
	//printf("str: %s\n", g_buf);
}

/*
 * ret : 0 : invalid format
 * ret : 1 : valid format
 */

int timematch_conv(char *mstr, char *nv_date, char *nv_time)
{
	char *datestr[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char timestart[6], timestop[6];
	char *time, *date;
	char mstr2[128];
	int i, head;
	int ret = 1;

	date = nvram_safe_get(nv_date);
	time = nvram_safe_get(nv_time);


	if (strlen(date)!=7||strlen(time)!=8) {
		ret = 0;
		goto no_match;
	}

	if (strncmp(date, "0000000", 7)==0) {
		ret = 0;
		goto no_match;
	}

	if (strncmp(date, "1111111", 7)==0 &&
	    strncmp(time, "00002359", 8)==0) goto no_match;

	i=0;
	strncpy(timestart, time, 2);
	i+=2;
	timestart[i++] = ':';
	strncpy(timestart+i, time+2, 2);
	i+=2;
	timestart[i]=0;
	i=0;
	strncpy(timestop, time+4, 2);
	i+=2;
	timestop[i++] = ':';
	strncpy(timestop+i, time+6, 2);
	i+=2;
	timestop[i]=0;


	if(strcmp(timestart, timestop)==0) {
		ret = 0;
		goto no_match;
	}

	//sprintf(mstr, "-m time --timestart %s:00 --timestop %s:00 --days ",
	sprintf(mstr, "-m time --timestart %s --timestop %s " DAYS_PARAM,
			timestart, timestop);

	head=1;

	for (i=0;i<7;i++)
	{
		if (*(date+i)=='1')
		{
			if (head)
			{
				sprintf(mstr2, "%s %s", mstr, datestr[i]);
				strlcpy(mstr, mstr2, sizeof(mstr2));
				head=0;
			}
			else
			{
				sprintf(mstr2, "%s,%s", mstr, datestr[i]);
				strlcpy(mstr, mstr2, sizeof(mstr2));
			}
		}
	}
	return ret;

no_match:
	//sprintf(mstr, "");
	mstr[0] = 0;	// oleg patch
	return ret;
}

/*
 * transfer string to time string
 * ex. 110000 -> 11:00:00, 135959 -> 13:59:59
 *
 */

char *str2time(char *str, char *buf, int buf_size, int flag){
	if(START_TIME == flag)
#ifndef IPTABLES_LEGACY
		snprintf(buf, buf_size, "%c%c:%c%c:00", *(str), *(str+1), *(str+2), *(str+3));
#else
		snprintf(buf, buf_size, "%c%c:%c%c", *(str), *(str+1), *(str+2), *(str+3));
#endif
	else if(STOP_TIME == flag)
#ifndef IPTABLES_LEGACY
		snprintf(buf, buf_size, "%c%c:%c%c:59", *(str), *(str+1), *(str+2), *(str+3));
#else
		snprintf(buf, buf_size, "%c%c:%c%c", *(str), *(str+1), *(str+2), *(str+3));
#endif
	return (buf);
}

/*
 * ret : 0 : invalid format
 * ret : 1 : valid format
 */

int timematch_conv2(char *mstr, int mstr_size, char *nv_date, char *nv_time, char *nv_time2)
{
	char *datestr[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char timestart[6], timestop[6], timestart2[6], timestop2[6];
	char *time, *time2,  *date;
	int i;
	int ret = 1;
	int dow = 0; // day of week
	char buf[1024], buf2[1024];
	char mstr2[2048];

	struct datetime datetime[7];

	date = nvram_safe_get(nv_date);
	time = nvram_safe_get(nv_time);
	time2 = nvram_safe_get(nv_time2);

	if (strlen(date)!=7||strlen(time)!=8||strlen(time2)!=8) {
		ret = 0;
		goto no_match;
	}

	if (strncmp(date, "0000000", 7)==0) {
		ret = 0;
		goto no_match;
	}

	if (strncmp(date, "1111111", 7)==0 &&
	    strncmp(time, "00002359", 8)==0 &&
	    strncmp(time2, "00002359", 8)==0)
	{
		goto no_match;
	}
	// schedule day of week
	for(i=0;i<=6;i++){
		dow += (date[i]-'0') << (6-i);
	}
	memset(timestart, 0, sizeof(timestart));
	memset(timestop, 0, sizeof(timestop));
	memset(timestart2, 0, sizeof(timestart2));
	memset(timestop2, 0, sizeof(timestop2));

	// weekdays time
	strncpy(timestart, time, 4);
	strncpy(timestop, time+4, 4);
	// weekend time
	strncpy(timestart2, time2, 4);
	strncpy(timestop2, time2+4, 4);

	// initialize
	memset(datetime, 0, sizeof(datetime));
	//cprintf("%s: dow=%d, timestart=%d, timestop=%d, timestart2=%d, timestop2=%d, sizeof(datetime)=%d\n", __FUNCTION__, dow, atoi(timestart), atoi(timestop), atoi(timestart2), atoi(timestop2), sizeof(datetime)); //tmp test

	// Sunday
	if((dow & 0x40) != 0)
	{
		if(atoi(timestart2) < atoi(timestop2))
		{
			snprintf(datetime[0].start, sizeof(datetime[0].start), "%s00", timestart2);
			snprintf(datetime[0].stop, sizeof(datetime[0].stop), "%s59", timestop2);
		}
		else
		{
			snprintf(datetime[0].start, sizeof(datetime[0].start), "%s00", timestart2);
			strncpy(datetime[0].stop, "235959", 6);
			snprintf(datetime[1].tmpstop, sizeof(datetime[1].tmpstop), "%s59", timestop2);
		}
	}

	// Monday to Friday
	for(i=1;i<6;i++)
	{
		if((dow & 1<<(6-i)) != 0)
		{
			if(atoi(timestart) < atoi(timestop))
			{
				snprintf(datetime[i].start, sizeof(datetime[i].start), "%s00", timestart);
				snprintf(datetime[i].stop, sizeof(datetime[i].stop), "%s59", timestop);
			}
			else
			{
				snprintf(datetime[i].start, sizeof(datetime[i].start), "%s00", timestart);
				strncpy(datetime[i].stop, "235959", 6);
				snprintf(datetime[i+1].tmpstop, sizeof(datetime[i+1].tmpstop), "%s59", timestop);
			}
		}
	}

	// Saturday
	if((dow & 0x01) != 0)
	{
		if(atoi(timestart2) < atoi(timestop2))
		{
			snprintf(datetime[6].start, sizeof(datetime[6].start), "%s00", timestart2);
			snprintf(datetime[6].stop, sizeof(datetime[6].stop), "%s59", timestop2);
		}
		else
		{
			snprintf(datetime[6].start, sizeof(datetime[6].start), "%s00", timestart2);
			strncpy(datetime[6].stop, "235959", 6);
			snprintf(datetime[0].tmpstop, sizeof(datetime[0].tmpstop), "%s59", timestop2);
		}
	}

	for(i=0;i<7;i++)
	{
		//cprintf("%s: i=%d, start=%s, stop=%s, tmpstop=%s\n", __FUNCTION__, i, datetime[i].start, datetime[i].stop, datetime[i].tmpstop); //tmp test

		// cascade cross-night time
		if((strcmp(datetime[i].tmpstop, "")!=0)
			&& (strcmp(datetime[i].start, "")!=0) && (strcmp(datetime[i].stop, "")!=0))
		{
			if((atoi(datetime[i].tmpstop) >= atoi(datetime[i].start))
				&& (atoi(datetime[i].tmpstop) <= atoi(datetime[i].stop)))
			{
				strncpy(datetime[i].start, "000000", 6);
				strncpy(datetime[i].tmpstop, "", 6);
			}
			else if (atoi(datetime[i].tmpstop) > atoi(datetime[i].stop))
			{
				strncpy(datetime[i].start, "000000", 6);
				strncpy(datetime[i].stop, datetime[i].tmpstop, 6);
				strncpy(datetime[i].tmpstop, "", 6);
			}
		}

		//cprintf("%s: i=%d, start=%s, stop=%s, tmpstop=%s\n", __FUNCTION__, i, datetime[i].start, datetime[i].stop, datetime[i].tmpstop); //tmp test

		char tmp1[9], tmp2[9];
		memset(tmp1, 0, sizeof(tmp1));
		memset(tmp2, 0, sizeof(tmp2));
		memset(buf, 0, sizeof(buf));

		// cross-night period
		if(strcmp(datetime[i].tmpstop, "")!=0){
			snprintf(buf2, sizeof(buf2), "%s-m time --timestop %s" DAYS_PARAM "%s"
			, buf, str2time(datetime[i].tmpstop, tmp2, sizeof(tmp2), STOP_TIME), datestr[i]);

			strlcpy(buf, buf2, sizeof(buf));
		}

		// normal period
		if((strcmp(datetime[i].start, "")!=0) && (strcmp(datetime[i].stop, "")!=0)){
			if(strcmp(buf, "")!=0) {
				snprintf(buf2, sizeof(buf2), "%s>", buf); // add ">"
				strlcpy(buf, buf2, sizeof(buf));
			}
			if((strcmp(datetime[i].start, "000000")==0) && (strcmp(datetime[i].stop, "235959")==0))// whole day
			{
				snprintf(buf2, sizeof(buf2), "%s-m time" DAYS_PARAM "%s", buf, datestr[i]);
				strlcpy(buf, buf2, sizeof(buf));
			}
			else if((strcmp(datetime[i].start, "000000")!=0) && (strcmp(datetime[i].stop, "235959")==0))// start ~ 2359
			{
				snprintf(buf2, sizeof(buf2), "%s-m time --timestart %s" DAYS_PARAM "%s"
				, buf, str2time(datetime[i].start, tmp1, sizeof(tmp1), START_TIME), datestr[i]);

				strlcpy(buf, buf2, sizeof(buf));
			}
			else if((strcmp(datetime[i].start, "000000")==0) && (strcmp(datetime[i].stop, "235959")!=0))// 0 ~ stop
			{
				snprintf(buf2, sizeof(buf2), "%s-m time --timestop %s" DAYS_PARAM "%s"
				, buf, str2time(datetime[i].stop, tmp2, sizeof(tmp2), STOP_TIME), datestr[i]);

				strlcpy(buf, buf2, sizeof(buf));
			}
			else if((strcmp(datetime[i].start, "000000")!=0) && (strcmp(datetime[i].stop, "235959")!=0))// start ~ stop
			{
				snprintf(buf2, sizeof(buf2), "%s-m time --timestart %s --timestop %s" DAYS_PARAM "%s"
				, buf,  str2time(datetime[i].start, tmp1, sizeof(tmp1), START_TIME)
				, str2time(datetime[i].stop, tmp2, sizeof(tmp2), STOP_TIME), datestr[i]);
				strlcpy(buf, buf2, sizeof(buf));
			}
		}

		if(strcmp(buf, "")!=0){
			if(strcmp(mstr, "")==0)
				snprintf(mstr, mstr_size, "%s", buf);
			else {
				snprintf(mstr2, sizeof(mstr2), "%s>%s", mstr, buf); // add ">"
				strlcpy(mstr, mstr2, sizeof(mstr2));
			}
		}

		//cprintf("%s: mstr=%s, len=%d\n", __FUNCTION__, mstr, strlen(mstr)); //tmp test
	}

	return ret;

no_match:
	//sprintf(mstr, "");
	mstr[0] = 0;	// oleg patch
	return ret;
}


char *iprange_ex_conv(char *ip, char *buf)
{
	char startip[16]; //, endip[16];
	int i, j, k;
	int mask;

	strcpy(buf, "");

	//printf("## iprange_ex_conv: %s, %d, %s\n", ip_name, idx, ip);	// tmp test
	// scan all ip string
	i=j=k=0;
	mask=32;

	while (*(ip+i))
	{
		if (*(ip+i)=='*')
		{
			startip[j++] = '0';
//			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else
		{
			startip[j++] = *(ip+i);
//			endip[k++] = *(ip+i);
		}
		++i;
	}

	startip[j++] = 0;
//	endip[k++] = 0;

	if (mask==32)
		sprintf(buf, "%s", startip);
	else if (mask==0)
		strcpy(buf, "");
	else sprintf(buf, "%s/%d", startip, mask);

	//printf("\nmask is %d, g_buf is %s\n", mask, g_buf);	// tmp test
	return (buf);
}

void
p(int step)
{
	_dprintf("P: %d %s\n", step, g_buf);
}

void
ip2class(char *lan_ip, char *netmask, char *buf)
{
	unsigned int val, ip;
	struct in_addr in;
	int i=0;

	// only handle class A,B,C
	val = (unsigned int)inet_addr(netmask);
	ip = (unsigned int)inet_addr(lan_ip);
/*
	in.s_addr = ip & val;
	if (val==0xff00000) sprintf(buf, "%s/8", inet_ntoa(in));
	else if (val==0xffff0000) sprintf(buf, "%s/16", inet_ntoa(in));
	else sprintf(buf, "%s/24", inet_ntoa(in));
*/
	// oleg patch ~
	in.s_addr = ip & val;

	for (val = ntohl(val); val; i++)
		val <<= 1;

	sprintf(buf, "%s/%d", inet_ntoa(in), i);
	// ~ oleg patch
	//_dprintf("ip2class output: %s\n", buf);
}

void convert_routes(void)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *nv, *nvp, *b;
	char *ip, *netmask, *gateway, *metric, *interface;
	char wroutes[1024], lroutes[1024], mroutes[1024];
#ifdef RTCONFIG_VPNC
	char vroutes[1024];
#endif
	int wan_max_unit = WAN_UNIT_MAX;

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6)
		wan_max_unit = WAN_UNIT_MULTICAST_IPTV_MAX;
#endif

	/* Disable Static if it's not enable */
	wroutes[0] = 0;
	lroutes[0] = 0;
	mroutes[0] = 0;
#ifdef RTCONFIG_VPNC
	vroutes[0] = 0;
#endif

	if (nvram_match("sr_enable_x", "1")) {
		nv = nvp = strdup(nvram_safe_get("sr_rulelist"));
		if (nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				int i = 0;
				char *routes;
				int len;

				if ((vstrsep(b, ">", &ip, &netmask, &gateway, &metric, &interface) != 5))
					continue;
				else if (strcmp(interface, "WAN") == 0)
					routes = wroutes;
				else if (strcmp(interface, "MAN") == 0)
					routes = mroutes;
				else if (strcmp(interface, "LAN") == 0)
					routes = lroutes;
#ifdef RTCONFIG_VPNC
				else if (strcmp(interface, "VPN") == 0)
					routes = vroutes;
#endif
				else
					continue;

				len = strlen(routes);
				_dprintf("%x %s %s %s %s %s\n", ++i, ip, netmask, gateway, metric, interface);
				sprintf(routes + len, " %s:%s:%s:%d", ip, netmask, gateway, atoi(metric)+1);
			}
			free(nv);
		}
	}

	nvram_set("lan_route", lroutes);

	for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; unit++){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		nvram_set(strcat_r(prefix, "route", tmp), wroutes);
		nvram_set(strcat_r(prefix, "mroute", tmp), mroutes);
	}

#ifdef RTCONFIG_VPNC
	nvram_set("vpnc_route", vroutes);
#endif
}

/*
char *ipoffset(char *ip, int offset, char *tmp)
{
	unsigned int ip1, ip2, ip3, ip4;

	sscanf(ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	sprintf(tmp, "%d.%d.%d.%d", ip1, ip2, ip3, ip4+offset);

	_dprintf("ip : %s\n", tmp);
	return (tmp);
}
*/

#ifdef RTCONFIG_YANDEXDNS
void write_yandexdns_filter(FILE *fp, char *lan_if, char *lan_ip)
{
	unsigned char ea[ETHER_ADDR_LEN];
	char *name, *mac, *mode, *enable, *server[2];
	char *nv, *nvp, *b;
	char lan_class[32];
	int i, count, dnsmode, defmode = nvram_get_int("yadns_mode");

	/* Reroute all DNS requests from LAN */
	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
	fprintf(fp, "-A PREROUTING -s %s -p udp -m udp --dport 53 -m u32 --u32 0>>22&0x3c@8>>15&1=0 -j YADNS\n"
		    "-A PREROUTING -s %s -p tcp -m tcp --dport 53 -m u32 --u32 0>>22&0x3c@12>>26&0x3c@8>>15&1=0 -j YADNS\n",
		lan_class, lan_class);

	for (dnsmode = YADNS_FIRST; dnsmode < YADNS_COUNT; dnsmode++) {
		fprintf(fp, ":YADNS%u - [0:0]\n", dnsmode);
		if (dnsmode == defmode)
			continue;
		count = get_yandex_dns(AF_INET, dnsmode, server, sizeof(server)/sizeof(server[0]));
		if (count <= 0)
			continue;
		if (dnsmode == YADNS_BASIC)
			fprintf(fp, "-A YADNS%u ! -d %s -j ACCEPT\n", dnsmode, lan_ip);
		else for (i = 0; i < count; i++)
			fprintf(fp, "-A YADNS%u -d %s -j ACCEPT\n", dnsmode, server[i]);
		fprintf(fp, "-A YADNS%u -j DNAT --to-destination %s\n", dnsmode, server[0]);
	}

	/* Protection level per client */
	nv = nvp = strdup(nvram_safe_get("yadns_rulelist"));
	while (nv && (b = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(b, ">", &name, &mac, &mode, &enable) < 3)
			continue;
		if (enable && atoi(enable) == 0)
			continue;
		if (!*mac || !*mode || !ether_atoe(mac, ea))
			continue;
		dnsmode = atoi(mode);
		/* Skip incorrect and default levels */
		if (dnsmode < YADNS_FIRST || dnsmode >= YADNS_COUNT || dnsmode == defmode)
			continue;
		fprintf(fp, "-A YADNS -m mac --mac-source %s -j YADNS%u\n", mac, dnsmode);
	}
	free(nv);

	/* Default protection level */
	if (defmode != YADNS_DISABLED && defmode != YADNS_BASIC)
		fprintf(fp, "-A YADNS ! -d %s -j DNAT --to-destination %s\n", lan_ip, lan_ip);
}

#ifdef RTCONFIG_IPV6
void write_yandexdns_filter6(FILE *fp, char *lan_if, char *lan_ip)
{
	unsigned char ea[ETHER_ADDR_LEN];
	char *name, *mac, *mode, *enable, *server[2];
	char *nv, *nvp, *b;
	int i, count, dnsmode, defmode = nvram_get_int("yadns_mode");

	/* Reroute all DNS requests from LAN */
	fprintf(fp, "-A INPUT -i %s -p udp -m udp --dport 53 -m u32 --u32 48>>15&1=0 -j YADNSI\n"
		    "-A INPUT -i %s -p tcp -m tcp --dport 53 -m u32 --u32 52>>26&0x3c@8>>15&1=0 -j YADNSI\n"
		    "-A FORWARD -i %s -p udp -m udp --dport 53 -m u32 --u32 48>>15&1=0 -j YADNSF\n"
		    "-A FORWARD -i %s -p tcp -m tcp --dport 53 -m u32 --u32 52>>26&0x3c@8>>15&1=0 -j YADNSF\n",
		lan_if, lan_if, lan_if, lan_if);

	for (dnsmode = YADNS_FIRST; dnsmode < YADNS_COUNT; dnsmode++) {
		fprintf(fp, ":YADNS%u - [0:0]\n", dnsmode);
		if (dnsmode == defmode)
			continue;
		count = get_yandex_dns(AF_INET6, dnsmode, server, sizeof(server)/sizeof(server[0]));
		if (count <= 0)
			continue;
		if (dnsmode == YADNS_BASIC)
			fprintf(fp, "-A YADNS%u -j ACCEPT\n", dnsmode);
		else for (i = 0; i < count; i++)
			fprintf(fp, "-A YADNS%u -d %s -j ACCEPT\n", dnsmode, server[i]);
		fprintf(fp, "-A YADNS%u -j DROP\n", dnsmode);
	}

	/* Protection level per client */
	nv = nvp = strdup(nvram_safe_get("yadns_rulelist"));
	while (nv && (b = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(b, ">", &name, &mac, &mode, &enable) < 3)
			continue;
		if (enable && atoi(enable) == 0)
			continue;
		if (!*mac || !*mode || !ether_atoe(mac, ea))
			continue;
		dnsmode = atoi(mode);
		/* Skip incorrect and default levels */
		if (dnsmode < YADNS_FIRST || dnsmode >= YADNS_COUNT || dnsmode == defmode)
			continue;
		fprintf(fp, "-A YADNSI -m mac --mac-source %s -j DROP\n", mac);
		fprintf(fp, "-A YADNSF -m mac --mac-source %s -j YADNS%u\n", mac, dnsmode);
	}
	free(nv);

	/* Default protection level */
	if (defmode != YADNS_DISABLED && defmode != YADNS_BASIC)
		fprintf(fp, "-A YADNSF -j DROP\n");
}
#endif /* RTCONFIG_IPV6 */
#endif /* RTCONFIG_YANDEXDNS */

#ifdef RTCONFIG_REDIRECT_DNAME
void redirect_nat_setting(void)
{
	FILE *fp;
	char *lan_ip = nvram_safe_get("lan_ipaddr");
	char name[PATH_MAX];

	sprintf(name, "%s_%s", NAT_RULES,nvram_safe_get("productid"));
	if((fp = fopen(name, "w")) == NULL)
		return;

	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":PUPNP - [0:0]\n"
		":VUPNP - [0:0]\n");
	if(nvram_match("wifison_ready", "1"))
	{
#ifdef RTCONFIG_WIFI_SON
		if(sw_mode() == SW_MODE_AP && nvram_match("cfg_master", "1")){
			fprintf(fp, "-A PREROUTING -p udp --dport 53 -i %s -j DNAT --to-destination %s:53\n",BR_GUEST, APMODE_BRGUEST_IP);
			fprintf(fp, "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:53\n", lan_ip);
			fprintf(fp, "-A POSTROUTING -o %s -j MASQUERADE\n",nvram_safe_get("lan_ifname"));
		}
#else
		_dprintf("no wifison feature\n");
#endif
	}
	else
		fprintf(fp, "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:53\n", lan_ip);

	fprintf(fp, "COMMIT\n");

	fclose(fp);

	unlink(NAT_RULES);
	symlink(name, NAT_RULES);

}
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
void repeater_nat_setting(){
	FILE *fp;
	char *lan_ip = nvram_safe_get("lan_ipaddr");
	int lan_port = /*nvram_get_int("http_lanport") ? :*/ 80;
	char name[PATH_MAX];

	sprintf(name, "%s_repeater", NAT_RULES);
	if((fp = fopen(name, "w")) == NULL)
		return;

	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":PUPNP - [0:0]\n"
		":VUPNP - [0:0]\n");

	fprintf(fp, "-A PREROUTING -d 10.0.0.1 -p tcp --dport 80 -j DNAT --to-destination %s:%d\n", lan_ip, lan_port);
	fprintf(fp, "-A PREROUTING -d %s -p tcp --dport 80 -j DNAT --to-destination %s:%d\n", nvram_default_get("lan_ipaddr"), lan_ip, lan_port);
#ifdef RTCONFIG_REDIRECT_DNAME
	if (nvram_invmatch("redirect_dname", "0"))
		fprintf(fp, "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:53\n", lan_ip);
#endif
	fprintf(fp, "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:18018\n", lan_ip);
	fprintf(fp, "COMMIT\n");

	fclose(fp);

	unlink(NAT_RULES);
	symlink(name, NAT_RULES);

	return;
}

#ifdef RTCONFIG_RESTRICT_GUI
void repeater_filter_setting(int mode){
	FILE *fp;
	char lan_if[128];
	char word[PATH_MAX], *next_word;
	int evalRet;

	if((fp = fopen("/tmp/filter_rules", "w")) == NULL)
		return;

	snprintf(lan_if, sizeof(lan_if), "%s", nvram_safe_get("lan_ifnames"));

	fprintf(fp, "*filter\n"
			":INPUT ACCEPT [0:0]\n"
			);

	if(nvram_get_int("fw_restrict_gui")){
		if(mode){
			foreach(word, lan_if, next_word){
				SKIP_ABSENT_FAKE_IFACE(word);
				if(!strncmp(word, "vlan", 4))
					continue;

				fprintf(fp, "-A INPUT -i %s -m mark --mark %s -d %s -p tcp -m multiport --dport 23,%d,%d,9999 -j DROP\n",
						word, BIT_RES_GUI, nvram_safe_get("lan_ipaddr"),
						nvram_get_int("http_lanport") ? : 80,
						nvram_get_int("https_lanport") ? : 443);
			}
		}
		else{
			fprintf(fp, "-A INPUT -i %s -m mark --mark %s -d %s -p tcp -m multiport --dport 23,%d,%d,9999 -j DROP\n",
					nvram_safe_get("lan_ifname"), BIT_RES_GUI, nvram_safe_get("lan_ipaddr"),
					nvram_get_int("http_lanport") ? : 80,
					nvram_get_int("https_lanport") ? : 443);
		}
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	evalRet = eval("iptables-restore", "/tmp/filter_rules");
	rule_apply_checking("firewall", __LINE__, "/tmp/filter_rules", evalRet);
}
#endif
#endif

void write_port_forwarding(FILE *fp, char *config, char *lan_ip, char *lan_if)
{
	char *proto, *protono, *port, *lport, *srcip, *dstip, *desc;
	char *nv, *nvp, *b;
	char srcips[64], dstips[64];
	char *chain;
#ifdef RTCONFIG_MULTIWAN_CFG
	char *wanx_ip;
#endif
	int wan_port;
	int cnt;
#if defined(RTCONFIG_MULTIWAN_CFG)
	int unit, wanx_rules, wan_max_unit = WAN_UNIT_MAX;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
#endif

	if (strcmp(config, "vts_rulelist") == 0)
		chain = "VSERVER";
	else if (strcmp(config, "game_vts_rulelist") == 0)
		chain = "GAME_VSERVER";
	else
		return;

	if (strcmp(config, "vts_rulelist") == 0) {
		// need multiple instance for tis?
		if (nvram_get_int("misc_http_x")) {
#ifdef RTCONFIG_HTTPS
			int enable = nvram_get_int("http_enable");
			if (enable != 0) {
				wan_port = nvram_get_int("misc_httpsport_x") ? : 8443;
				fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%d\n",
					wan_port, lan_ip, nvram_get_int("https_lanport") ? : 443);
			}
			/* do not support http (enable != 1) */
#else
			{
				wan_port = nvram_get_int("misc_httpport_x") ? : 8080;
				fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%d\n",
					wan_port, lan_ip, nvram_get_int("http_lanport") ? : 80);
			}
#endif
		}
	}

#if !defined(RTCONFIG_MULTIWAN_CFG)
	if (is_nat_enabled() && nvram_match("vts_enable_x", "1")) {
		nvp = nv = strdup(nvram_safe_get(config));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			char *portv, *portp, *c;

			if ((cnt = vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto, &srcip)) < 5)
				continue;
			else if (cnt < 6)
				srcip = "";

			// Handle source type format
			srcips[0] = '\0';
			if (srcip && *srcip) {
				char srcAddr[32];
				int src_type = addr_type_parse(srcip, srcAddr, sizeof(srcAddr));
				if (src_type == TYPE_IP)
					snprintf(srcips, sizeof(srcips), "-s %s", srcAddr);
				else if (src_type == TYPE_MAC)
					snprintf(srcips, sizeof(srcips), "-m mac --mac-source %s", srcAddr);
				else if (src_type == TYPE_IPRANGE)
					snprintf(srcips, sizeof(srcips), "-m iprange --src-range %s", srcAddr);
			}

			// Handle port1,port2,port3 format
			portp = portv = strdup(port);
			while (portv && (c = strsep(&portp, ",")) != NULL) {
				if (lport && *lport)
					snprintf(dstips, sizeof(dstips), "--to-destination %s:%s", dstip, lport);
				else
					snprintf(dstips, sizeof(dstips), "--to %s", dstip);

				if (strcmp(proto, "TCP") == 0 || strcmp(proto, "BOTH") == 0)
					fprintf(fp, "-A %s %s -p tcp -m tcp --dport %s -j DNAT %s\n", chain, srcips, c, dstips);
				if (strcmp(proto, "UDP") == 0 || strcmp(proto, "BOTH") == 0)
					fprintf(fp, "-A %s %s -p udp -m udp --dport %s -j DNAT %s\n", chain, srcips, c, dstips);
				// Handle raw protocol in port field, no val1:val2 allowed
				if (strcmp(proto, "OTHER") == 0) {
					protono = strsep(&c, ":");
					fprintf(fp, "-A %s %s -p %s -j DNAT --to %s\n", chain, srcips, protono, dstip);
				}
			}
			free(portv);
		}
		free(nv);
	}
#else
	// Port forwarding or Virtual Server for multiple WAN configurations
	if (is_nat_enabled() && nvram_match("vts_enable_x", "1")) {
		int i, mtwancfg = 0;

		if (get_nr_wan_unit() == 2 && nvram_match("wans_mode", "lb"))
			mtwancfg = 1;

		if (mtwancfg) {
			/* dualwan + load-balance */
			for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit) {
				char vts_nv[sizeof("vts_rulelistXXXXXX")];
				char dst_ip[sizeof("-d 111.222.333.444XXXXXX")];
				char *wan_iface[3];	/* 0: br0; 1: wan_if; 2: wanx_if; */

				snprintf(prefix, sizeof(prefix), "wan%d_", unit);
				if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
					continue;

				snprintf(dst_ip, sizeof(dst_ip), "-d %s", nvram_pf_safe_get(prefix, "ipaddr"));
				wanx_rules = 1;
				wan_iface[0] = lan_if;
				wan_iface[1] = get_wan_ifname(unit);
				wan_iface[2] = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
				wanx_ip = nvram_safe_get(strcat_r(prefix, "xipaddr", tmp));

				if (strcmp(wan_iface[0], wan_iface[1]) && inet_addr_(wanx_ip))
					wanx_rules = 2;

				/* FIXME: Below statments ignore @config. */
				if (unit)
					snprintf(vts_nv, sizeof(vts_nv), "vts%d_rulelist", unit);
				else
					strlcpy(vts_nv, "vts_rulelist", sizeof(vts_nv));
				nvp = nv = strdup(nvram_safe_get(vts_nv));
				while (nv && (b = strsep(&nvp, "<")) != NULL) {
					char *portv, *portp, *c;

					if ((cnt = vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto, &srcip)) < 5)
						continue;
					else if (cnt < 6)
						srcip = "";

					// Handle source type format
					srcips[0] = '\0';
					if (srcip && *srcip) {
						char srcAddr[32];
						int src_type = addr_type_parse(srcip, srcAddr, sizeof(srcAddr));
						if (src_type == TYPE_IP)
							snprintf(srcips, sizeof(srcips), "-s %s", srcAddr);
						else if (src_type == TYPE_MAC)
							snprintf(srcips, sizeof(srcips), "-m mac --mac-source %s", srcAddr);
						else if (src_type == TYPE_IPRANGE)
							snprintf(srcips, sizeof(srcips), "-m iprange --src-range %s", srcAddr);
					}

					// Handle port1,port2,port3 format
					portp = portv = strdup(port);
					while (portv && (c = strsep(&portp, ",")) != NULL) {
						if (lport && *lport)
							snprintf(dstips, sizeof(dstips), "--to-destination %s:%s", dstip, lport);
						else
							snprintf(dstips, sizeof(dstips), "--to %s", dstip);

						if (!strcmp(proto, "TCP") || !strcmp(proto, "BOTH")){
							for (i = 0; i <= wanx_rules; ++i) {
								fprintf(fp, "-A %s %s -i %s %s -p tcp -m tcp --dport %s -j DNAT %s\n",
									chain, srcips, wan_iface[i], i? "" : dst_ip, c, dstips);
							}
						}
						if (!strcmp(proto, "UDP") || !strcmp(proto, "BOTH")) {
							for (i = 0; i <= wanx_rules; ++i) {
								fprintf(fp, "-A %s %s -i %s %s -p udp -m udp --dport %s -j DNAT %s\n",
									chain, srcips, wan_iface[i], i? "" : dst_ip, c, dstips);
							}
						}
						// Handle raw protocol in port field, no val1:val2 allowed
						if (strcmp(proto, "OTHER") == 0) {
							protono = strsep(&c, ":");
							for (i = 0; i <= wanx_rules; ++i) {
								fprintf(fp, "-A %s %s -i %s %s -p %s -j DNAT --to %s\n",
									chain, srcips, wan_iface[i], i? "" : dst_ip, protono, dstip);
							}
						}
					}
					free(portv);
				}
				free(nv);
			}
		} else {
			/* singlewan or dualwan + failover/fallback, only primary available. */
			nvp = nv = strdup(nvram_safe_get(config));
			while (nv && (b = strsep(&nvp, "<")) != NULL) {
				char *portv, *portp, *c;

				if ((cnt = vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto, &srcip)) < 5)
					continue;
				else if (cnt < 6)
					srcip = "";

				// Handle source type format
				srcips[0] = '\0';
				if (srcip && *srcip) {
					char srcAddr[32];
					int src_type = addr_type_parse(srcip, srcAddr, sizeof(srcAddr));
					if (src_type == TYPE_IP)
						snprintf(srcips, sizeof(srcips), "-s %s", srcAddr);
					else if (src_type == TYPE_MAC)
						snprintf(srcips, sizeof(srcips), "-m mac --mac-source %s", srcAddr);
					else if (src_type == TYPE_IPRANGE)
						snprintf(srcips, sizeof(srcips), "-m iprange --src-range %s", srcAddr);
				}

				// Handle port1,port2,port3 format
				portp = portv = strdup(port);
				while (portv && (c = strsep(&portp, ",")) != NULL) {
					if (lport && *lport)
						snprintf(dstips, sizeof(dstips), "--to-destination %s:%s", dstip, lport);
					else
						snprintf(dstips, sizeof(dstips), "--to %s", dstip);

					if (strcmp(proto, "TCP") == 0 || strcmp(proto, "BOTH") == 0){
						fprintf(fp, "-A %s %s -p tcp -m tcp --dport %s -j DNAT %s\n",
							chain, srcips, c, dstips);
					}
					if (strcmp(proto, "UDP") == 0 || strcmp(proto, "BOTH") == 0)
						fprintf(fp, "-A %s %s -p udp -m udp --dport %s -j DNAT %s\n",
							chain, srcips, c, dstips);
					// Handle raw protocol in port field, no val1:val2 allowed
					if (strcmp(proto, "OTHER") == 0) {
						protono = strsep(&c, ":");
						fprintf(fp, "-A %s %s -p %s -j DNAT --to %s\n",
							chain, srcips, protono, dstip);
					}
				}
				free(portv);
			}
			free(nv);
		}
	}
#endif	/* RTCONFIG_MULTIWAN_CFG */
#if defined(RTAX56_XD4) || defined(RTAC59_CD6N) || defined(PLAX56_XP4)
#if defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D"))
#endif // XP4
	write_extra_port_forwarding(fp, lan_ip);
#endif
}

void nat_setting(char *wan_if, char *wan_ip, char *wanx_if, char *wanx_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)	// oleg patch
{
	FILE *fp;
	char lan_class[32];
	char name[PATH_MAX];
	int wan_unit;
#ifdef RTCONFIG_MULTIWAN_CFG
	int wanx_rules = 0;
#endif
#ifdef RTCONFIG_TOR
	char addr_new[32];
	int addr_type;
	char *nv, *b;
#endif
#ifdef RTCONFIG_WIFI_SON
	char g_lan_class[32];
	char g_lan_ip[20];
	unsigned int dip;
	struct in_addr gst;

	dip = ntohl(inet_addr(lan_ip)) + 0x100;
	gst.s_addr = htonl(dip);
	strcpy(g_lan_ip, inet_ntoa(gst));
#endif

	sprintf(name, "%s_%s_%s", NAT_RULES, wan_if, wanx_if);
	remove_slash(name + strlen(NAT_RULES));
	if ((fp=fopen(name, "w"))==NULL) return;

	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":VSERVER - [0:0]\n"
		":LOCALSRV - [0:0]\n"
		":PUPNP - [0:0]\n"
		":VUPNP - [0:0]\n");
#ifdef RTCONFIG_DNSFILTER
	fprintf(fp,
		":DNSFILTER - [0:0]\n");
#endif
#ifdef RTCONFIG_OPEN_NAT
	fprintf(fp,
		":GAME_VSERVER - [0:0]\n");
#endif
#ifdef RTCONFIG_YANDEXDNS
	fprintf(fp,
		":YADNS - [0:0]\n");
#endif
#ifdef RTCONFIG_PARENTALCTRL
	fprintf(fp,
		":PCREDIRECT - [0:0]\n");
#endif
#if defined(RTCONFIG_FBWIFI)
	if(sw_mode() == SW_MODE_ROUTER){
		fprintf(fp,
			":CLIENT_TO_INTERNET - [0:0]\n");
	}
#endif
#ifdef RTCONFIG_VPN_FUSION
	fprintf(fp,
		":VPN_FUSION - [0:0]\n");
#endif

	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);

	_dprintf("writting prerouting %s %s %s %s %s %s\n", wan_if, wan_ip, wanx_if, wanx_ip, lan_if, lan_ip);

	//Log
	//if (nvram_match("misc_natlog_x", "1"))
	// 	fprintf(fp, "-A PREROUTING -i %s -j LOG --log-prefix ALERT --log-level 4\n", wan_if);

#if 0
#ifdef RTCONFIG_AUTOCOVER_SIP
	if(nvram_get_int("atcover_sip") == 1 && !strcmp(lan_ip, nvram_default_get("lan_ipaddr")) && strcmp(lan_ip, nvram_safe_get("atcover_sip_ip"))){
		int dst_port;

		if(nvram_get_int("atcover_sip_type") == 1)
			dst_port = 80;
		else
			dst_port = 18017;

		fprintf(fp, "-A PREROUTING -d %s -p tcp --dport 80 -j DNAT --to-destination %s:%d\n",
				nvram_safe_get("atcover_sip_ip"),
				lan_ip,
				dst_port
				);
	}
#endif
#endif

	/* VSERVER chain */
	if (inet_addr_(wan_ip)) {
#ifdef RTCONFIG_OPEN_NAT
		fprintf(fp, "-A PREROUTING -d %s -j GAME_VSERVER\n", wan_ip);
#endif
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wan_ip);
	}
	/* prerouting physical WAN port connection (DHCP+PPP case) */
	if (strcmp(wan_if, wanx_if) && inet_addr_(wanx_ip)) {
#ifdef RTCONFIG_MULTIWAN_CFG
		wanx_rules = 1;
#endif
#ifdef RTCONFIG_OPEN_NAT
		fprintf(fp, "-A PREROUTING -d %s -j GAME_VSERVER\n", wanx_ip);
#endif
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wanx_ip);
	}

#ifdef RTCONFIG_CAPTIVE_PORTAL
	if(nvram_match("chilli_enable", "1")){
		char *net = nvram_get("chilli_net")? : nvram_default_get("chilli_net");	/* 192.168.182.0/24 */

		fprintf(fp, "-I PREROUTING 1 --dst %s -p tcp --dport 80 -j REDIRECT --to-ports 3990\n", net);
		fprintf(fp, "-I PREROUTING 2 --src %s --dst %s -p tcp --dport 80 -j REDIRECT --to-ports 8083\n", net, lan_ip);
//	 	fprintf(fp, "-I PREROUTING 2 --src 192.168.182.0/24 --dst 192.168.182.1 -p tcp --dport 50000 -j DNAT --to %s:8082\n", lan_ip);
	}
	if(nvram_match("cp_enable", "1")){
		char *net = nvram_get("cp_net")? : nvram_default_get("cp_net");		/* 192.168.183.0/24 */

		fprintf(fp, "-I PREROUTING 1 --dst %s -p tcp --dport 80 -j REDIRECT --to-ports 3998\n", net);
		fprintf(fp, "-I PREROUTING 2 --src %s --dst %s -p tcp --dport 80 -j REDIRECT --to-ports 8083\n", net, lan_ip);
//	 	fprintf(fp, "-I PREROUTING 2 --src 192.168.182.0/24 --dst 192.168.182.1 -p tcp --dport 50000 -j DNAT --to %s:8082\n", lan_ip);
	}


#endif

#ifdef RTCONFIG_VPN_FUSION
        write_vpn_fusion_nat(fp, lan_ip);
#endif

#ifdef RTCONFIG_YANDEXDNS
	if (nvram_get_int("yadns_enable_x"))
		write_yandexdns_filter(fp, lan_if, lan_ip);
#endif

#ifdef RTCONFIG_DNSFILTER
	dnsfilter_settings(fp, lan_ip);
#endif

#ifdef RTCONFIG_NTPD
	if (nvram_get_int("ntpd_enable") && nvram_get_int("ntpd_server_redir")) {
		fprintf(fp, "-A PREROUTING -i %s -p udp -m udp --dport 123 -j REDIRECT --to-port 123\n", lan_if);
	}
#endif

#ifdef RTCONFIG_PARENTALCTRL
	pc_s *pc_list = NULL;
	int pc_count;

	get_all_pc_list(&pc_list);
	pc_count = count_pc_rules(pc_list, 1);
	free_pc_list(&pc_list);

	if(nvram_get_int("MULTIFILTER_ALL") != 0 && pc_count > 0){
		config_blocking_redirect(fp);
	}
#endif

#ifdef RTCONFIG_FREERADIUS
	if(nvram_get_int("radius_serv_enable")){
		fprintf(fp, "-A VSERVER -p udp --dport %d -j DNAT --to-destination %s:%d\n", 1812, lan_ip, 1812);
	}
#endif
	// Port forwarding or Virtual Server
	write_port_forwarding(fp, "vts_rulelist", lan_ip, lan_if);
#ifdef RTCONFIG_OPEN_NAT
	write_port_forwarding(fp, "game_vts_rulelist", lan_ip, lan_if);
#endif

	if (is_nat_enabled() && nvram_get_int("upnp_enable"))
	{
		/* call UPNP chain */
		fprintf(fp, "-A VSERVER -j VUPNP\n");
		fprintf(fp, "-A POSTROUTING -o %s -j PUPNP\n", wan_if);
	}

	/* Trigger port setting */
	if (is_nat_enabled() && nvram_match("autofw_enable_x", "1"))
		write_porttrigger(fp, wan_if, 1);

#if 0
	if (is_nat_enabled() && !nvram_match("sp_battle_ips", "0") && inet_addr_(wan_ip))	// oleg patch
	{
#define BASEPORT 6112
#define BASEPORT_NEW 10000

		/* run starcraft patch anyway */
		fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", wan_ip, BASEPORT, lan_class);

		fprintf(fp, "-A POSTROUTING -p udp -s %s --dport %d -j NETMAP --to %s\n", lan_class, BASEPORT, wan_ip);

		//fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n",
		//			BASEPORT, logaccept);	// oleg patch
	}
#endif

	/* Exposed station */
	if (is_nat_enabled() && dmz_enabled()) {
		fprintf(fp, "-A VSERVER -j LOCALSRV\n");

#ifdef RTCONFIG_IPSEC
		/* Accept stateless IPSec tunnel */
		if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
		 ) {
			int isakmp_port = nvram_get_int("ipsec_isakmp_port") ? : 500;
			int nat_t_port = nvram_get_int("ipsec_nat_t_port") ? : 4500;

			fprintf(fp, "-A LOCALSRV -p udp -m multiport --dport %d,%d -j ACCEPT\n", isakmp_port, nat_t_port);
			fprintf(fp, "-A LOCALSRV -p esp -j ACCEPT\n");
			fprintf(fp, "-A LOCALSRV -p ah -j ACCEPT\n");
		}
#endif

#if 1 /* def RTCONFIG_WEBDAV */
		if (nvram_match("webdav_aidisk", "1")) {
			int port;

			port = nvram_get_int("webdav_https_port");
			if (!port || port >= 65536)
				port = 443;
			fprintf(fp, "-A LOCALSRV -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%d\n", port, lan_ip, port);
			port = nvram_get_int("webdav_http_port");
			if (!port || port >= 65536)
				port = 8082;
			fprintf(fp, "-A LOCALSRV -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%d\n", port, lan_ip, port);
		}
#endif

#ifdef RTCONFIG_MULTIWAN_CFG
		if (get_nr_wan_unit() == 2 && nvram_match("wans_mode", "lb")) {
			/* dualwan + load-balance */
			if (!illegal_ipv4_address(nvram_safe_get("dmz_ip"))) {
				fprintf(fp, "-A VSERVER -i %s -j DNAT --to %s\n", wan_if, nvram_safe_get("dmz_ip"));
				if (wanx_rules)
					fprintf(fp, "-A VSERVER -i %s -j DNAT --to %s\n", wanx_if, nvram_safe_get("dmz_ip"));
			}
			if (!illegal_ipv4_address(nvram_safe_get("dmz_ip"))) {
				fprintf(fp, "-A VSERVER -i %s -j DNAT --to %s\n", wan_if, nvram_safe_get("dmz1_ip"));
				if (wanx_rules)
					fprintf(fp, "-A VSERVER -i %s -j DNAT --to %s\n", wanx_if, nvram_safe_get("dmz1_ip"));
			}
		} else
#endif
		{
			/* singlewan or dualwan + failover/fallback, only primary available. */
			fprintf(fp, "-A VSERVER -j DNAT --to %s\n", nvram_safe_get("dmz_ip"));
		}
	}

	if (is_nat_enabled())
	{
		char *p = "";
#ifdef RTCONFIG_IPV6
		switch (get_ipv6_service()) {
		case IPV6_6IN4:
			// avoid NATing proto-41 packets when using 6in4 tunnel
			p = "! -p 41";
			break;
		}
#endif
#ifdef RTCONFIG_IPSEC
		if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
			)
			fprintf(fp, "-A POSTROUTING -m policy --dir out --pol ipsec -j ACCEPT\n");
#endif
		if (inet_addr_(wan_ip))
#ifdef HND_ROUTER
			fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE --mode %s\n", p, wan_if, wan_ip, (nvram_get_int("nat_type") ? "fullcone" : "symmetric"));
#else
			fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE\n", p, wan_if, wan_ip);
#endif

		/* masquerade physical WAN port connection */
		if (strcmp(wan_if, wanx_if) && inet_addr_(wanx_ip))
#ifdef HND_ROUTER
			fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE --mode %s\n", p, wanx_if, wanx_ip, (nvram_get_int("nat_type") ? "fullcone" : "symmetric"));
#else
			fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE\n", p, wanx_if, wanx_ip);
#endif

		/* masquerade lan to lan */
			fprintf(fp, "-A POSTROUTING %s -o %s -s %s -d %s -j MASQUERADE\n", p, lan_if, lan_class, lan_class);

#ifdef RTCONFIG_WIFI_SON
		if (sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
			ip2class(g_lan_ip, nvram_safe_get("lan_netmask"), g_lan_class);
			fprintf(fp, "-A POSTROUTING %s -o %s -s %s -d %s -j MASQUERADE\n", p, BR_GUEST, g_lan_class, g_lan_class);
		}
#endif
	}

#ifdef RTCONFIG_FBWIFI
	if(sw_mode() == SW_MODE_ROUTER){
		fbwifi_nat(fp);
	}
#endif

#ifdef RTCONFIG_TOR
	 if(nvram_match("Tor_enable", "1")){
		nv = strdup(nvram_safe_get("Tor_redir_list"));
		if (strlen(nv) == 0) {
		fprintf(fp, "-I PREROUTING -i %s -p udp --dport 53 -j REDIRECT --to-ports %s\n", lan_if, nvram_safe_get("Tor_dnsport"));
		fprintf(fp, "-I PREROUTING -i %s -p udp --dport 123 -j REDIRECT --to-ports 123\n", lan_if); // requires an NTP server
		fprintf(fp, "-I PREROUTING -i %s -p tcp --syn ! -d %s -j REDIRECT --to-ports %s\n", lan_if, lan_class, nvram_safe_get("Tor_transport"));
		}
		else{
			while ((b = strsep(&nv, "<")) != NULL) {
				if (strlen(b)==0) continue;
				memset(addr_new, 0, sizeof(addr_new));
				addr_type = addr_type_parse(b, addr_new, sizeof(addr_new));
				if (addr_type == TYPE_IP){
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 53 -s %s -j REDIRECT --to-ports %s\n", lan_if, addr_new, nvram_safe_get("Tor_dnsport"));
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 123 -s %s -j REDIRECT --to-ports 123\n", lan_if, addr_new); // requires an NTP server
					fprintf(fp, "-I PREROUTING -i %s -p tcp --syn ! -d %s -s %s -j REDIRECT --to-ports %s\n", lan_if, lan_class, addr_new, nvram_safe_get("Tor_transport"));
				}
				else if (addr_type == TYPE_MAC){
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 53 -m mac --mac-source %s -j REDIRECT --to-ports %s\n", lan_if, addr_new, nvram_safe_get("Tor_dnsport"));
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 123 -m mac --mac-source %s -j REDIRECT --to-ports 123\n", lan_if, addr_new); // requires an NTP server
					fprintf(fp, "-I PREROUTING -i %s -p tcp --syn ! -d %s -m mac --mac-source %s -j REDIRECT --to-ports %s\n", lan_if, lan_class, addr_new, nvram_safe_get("Tor_transport"));
				}
				else if (addr_type == TYPE_IPRANGE){
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 53 -m iprange --src-range %s -j REDIRECT --to-ports %s\n", lan_if, addr_new, nvram_safe_get("Tor_dnsport"));
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 123 -m iprange --src-range %s -j REDIRECT --to-ports 123\n", lan_if, addr_new); // requires an NTP server
					fprintf(fp, "-I PREROUTING -i %s -p tcp --syn ! -d %s -m iprange --src-range %s -j REDIRECT --to-ports %s\n", lan_if, lan_class, addr_new, nvram_safe_get("Tor_transport"));
				}
			}
			free(nv);
		}
	}
#endif

	fprintf(fp, "COMMIT\n");
	fclose(fp);

	unlink(NAT_RULES);
	symlink(name, NAT_RULES);

	wan_unit = wan_ifunit(wan_if);
	if(is_phy_connect2(wan_unit)){
		/* force nat update */
		nvram_set_int("nat_state", NAT_STATE_UPDATE);
_dprintf("nat_rule: start_nat_rules 1.\n");
		start_nat_rules();
	}
	else
_dprintf("nat_rule: skip nat_rules because of no PHY.\n");
}

#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV) // RTCONFIG_DUALWAN || RTCONFIG_MULTICAST_IPTV
void nat_setting2(char *lan_if, char *lan_ip, char *logaccept, char *logdrop)	// oleg patch
{
	FILE *fp = NULL;	// oleg patch
	char lan_class[32];	// oleg patch
	char *wan_if = "", *wan_ip;
	char *wanx_if = "", *wanx_ip;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char name[PATH_MAX];
	int wan_max_unit = WAN_UNIT_MAX;
#ifdef RTCONFIG_MULTIWAN_CFG
	int wanx_rules = 0;
	int need_dmz = 0;
#endif
#ifdef RTCONFIG_TOR
	char addr_new[32];
	int addr_type;
	char *nv, *b;
#endif
#ifdef RTCONFIG_WIFI_SON
	char g_lan_class[32];
	char g_lan_ip[20];
	unsigned int dip;
	struct in_addr gst;

	dip = ntohl(inet_addr(lan_ip)) + 0x100;
	gst.s_addr = htonl(dip);
	strcpy(g_lan_ip, inet_ntoa(gst));
#endif

 	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6)
		wan_max_unit = WAN_UNIT_MULTICAST_IPTV_MAX;
#endif

	for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
		if(!is_wan_connect(unit))
			continue;

		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		wan_if = get_wan_ifname(unit);
		wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
		wanx_if = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		wanx_ip = nvram_safe_get(strcat_r(prefix, "xipaddr", tmp));

		if (!fp) {
			sprintf(name, "%s_%d_%s_%s", NAT_RULES, unit, wan_if, wanx_if);
			remove_slash(name + strlen(NAT_RULES));
			if ((fp=fopen(name, "w"))==NULL) return;

			fprintf(fp, "*nat\n"
				":PREROUTING ACCEPT [0:0]\n"
				":POSTROUTING ACCEPT [0:0]\n"
				":OUTPUT ACCEPT [0:0]\n"
				":VSERVER - [0:0]\n"
				":LOCALSRV - [0:0]\n"
				":PUPNP - [0:0]\n"
				":VUPNP - [0:0]\n");
#ifdef RTCONFIG_OPEN_NAT
			fprintf(fp,
				":GAME_VSERVER - [0:0]\n");
#endif
#ifdef RTCONFIG_YANDEXDNS
			fprintf(fp,
				":YADNS - [0:0]\n");
#endif
#ifdef RTCONFIG_DNSFILTER
			fprintf(fp,
				":DNSFILTER - [0:0]\n");
#endif
#ifdef RTCONFIG_PARENTALCTRL
			fprintf(fp,
				":PCREDIRECT - [0:0]\n");
#endif
#if defined(RTCONFIG_FBWIFI)
			if(sw_mode() == SW_MODE_ROUTER){
				fprintf(fp,
					":CLIENT_TO_INTERNET - [0:0]\n");
			}
#endif
		}

		_dprintf("writting prerouting 2 %s %s %s %s %s %s\n", wan_if, wan_ip, wanx_if, wanx_ip, lan_if, lan_ip);

		//Log
		//if (nvram_match("misc_natlog_x", "1"))
		//	fprintf(fp, "-A PREROUTING -i %s -j LOG --log-prefix ALERT --log-level 4\n", wan_if);

		/* VSERVER chain */
		if(inet_addr_(wan_ip)) {
#ifdef RTCONFIG_OPEN_NAT
			fprintf(fp, "-A PREROUTING -d %s -j GAME_VSERVER\n", wan_ip);
#endif
			fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wan_ip);
		}

		// wanx_if != wan_if means DHCP+PPP exist?
		if (dualwan_unit__nonusbif(unit) && strcmp(wan_if, wanx_if) && inet_addr_(wanx_ip)) {
#ifdef RTCONFIG_OPEN_NAT
			fprintf(fp, "-A PREROUTING -d %s -j GAME_VSERVER\n", wanx_ip);
#endif
			fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wanx_ip);
		}

#ifdef RTCONFIG_CAPTIVE_PORTAL
		if(nvram_match("chilli_enable", "1")){
			char *net = nvram_get("chilli_net")? : nvram_default_get("chilli_net");	/* 192.168.182.0/24 */

			fprintf(fp, "-I PREROUTING 1 --dst %s -p tcp --dport 80 -j REDIRECT --to-ports 3990\n", net);
			fprintf(fp, "-I PREROUTING 2 --src %s --dst %s -p tcp --dport 80 -j REDIRECT --to-ports 8083\n", net, lan_ip);
//	 		fprintf(fp, "-I PREROUTING 2 --src 192.168.182.0/24 --dst 192.168.182.1 -p tcp --dport 50000 -j DNAT --to %s:8082\n", lan_ip);
		}
		if(nvram_match("cp_enable", "1")){
			char *net = nvram_get("cp_net")? : nvram_default_get("cp_net");		/* 192.168.183.0/24 */

			fprintf(fp, "-I PREROUTING 1 --dst %s -p tcp --dport 80 -j REDIRECT --to-ports 3998\n", net);
			fprintf(fp, "-I PREROUTING 2 --src %s --dst %s -p tcp --dport 80 -j REDIRECT --to-ports 8083\n", net, lan_ip);
//	 		fprintf(fp, "-I PREROUTING 2 --src 192.168.182.0/24 --dst 192.168.182.1 -p tcp --dport 50000 -j DNAT --to %s:8082\n", lan_ip);
		}


#endif

	}
	if (!fp) {
		sprintf(name, "%s___", NAT_RULES);
		remove_slash(name + strlen(NAT_RULES));
		if ((fp=fopen(name, "w"))==NULL) return;

		fprintf(fp, "*nat\n"
				":PREROUTING ACCEPT [0:0]\n"
				":POSTROUTING ACCEPT [0:0]\n"
				":OUTPUT ACCEPT [0:0]\n"
				":VSERVER - [0:0]\n"
				":LOCALSRV - [0:0]\n"
				":PUPNP - [0:0]\n"
				":VUPNP - [0:0]\n");
#ifdef RTCONFIG_OPEN_NAT
		fprintf(fp,
				":GAME_VSERVER - [0:0]\n");
#endif
#ifdef RTCONFIG_YANDEXDNS
		fprintf(fp,
				":YADNS - [0:0]\n");
#endif
#ifdef RTCONFIG_PARENTALCTRL
		fprintf(fp,
				":PCREDIRECT - [0:0]\n");
#endif
#if defined(RTCONFIG_FBWIFI)
		if(sw_mode() == SW_MODE_ROUTER){
			fprintf(fp,
				":CLIENT_TO_INTERNET - [0:0]\n");
		}
#endif
	}

#ifdef RTCONFIG_YANDEXDNS
	if (nvram_get_int("yadns_enable_x"))
		write_yandexdns_filter(fp, lan_if, lan_ip);
#endif

#ifdef RTCONFIG_DNSFILTER
	dnsfilter_settings(fp, lan_ip);
#endif

#ifdef RTCONFIG_NTPD
	if (nvram_get_int("ntpd_enable") && nvram_get_int("ntpd_server_redir")) {
		fprintf(fp, "-A PREROUTING -i %s -p udp -m udp --dport 123 -j REDIRECT --to-port 123\n", lan_if);
	}
#endif

#ifdef RTCONFIG_PARENTALCTRL
	pc_s *pc_list = NULL;
	int pc_count;

	get_all_pc_list(&pc_list);
	pc_count = count_pc_rules(pc_list, 1);
	free_pc_list(&pc_list);

	if(nvram_get_int("MULTIFILTER_ALL") != 0 && pc_count > 0){
		config_blocking_redirect(fp);
	}
#endif

#ifdef RTCONFIG_FREERADIUS
	if(nvram_get_int("radius_serv_enable")){
		fprintf(fp, "-A VSERVER -p udp --dport %d -j DNAT --to-destination %s:%d\n", 1812, lan_ip, 1812);
	}
#endif
	// Port forwarding or Virtual Server
	write_port_forwarding(fp, "vts_rulelist", lan_ip, lan_if);
#ifdef RTCONFIG_OPEN_NAT
	write_port_forwarding(fp, "game_vts_rulelist", lan_ip, lan_if);
#endif

	if (is_nat_enabled() && nvram_get_int("upnp_enable"))
	{
		/* call UPNP chain */
		fprintf(fp, "-A VSERVER -j VUPNP\n");
		for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; unit++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			wan_if = get_wan_ifname(unit);
			fprintf(fp, "-A POSTROUTING -o %s -j PUPNP\n", wan_if);
		}
	}

	/* Trigger port setting */
	if (is_nat_enabled() && nvram_match("autofw_enable_x", "1"))
	for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
		if(!is_wan_connect(unit))
			continue;

		wan_if = get_wan_ifname(unit);
		write_porttrigger(fp, wan_if, 1);
	}

#if 0
	if (is_nat_enabled() && !nvram_match("sp_battle_ips", "0") && inet_addr_(wan_ip))	// oleg patch
	{
#define BASEPORT 6112
#define BASEPORT_NEW 10000

		/* run starcraft patch anyway */
		fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", wan_ip, BASEPORT, lan_class);

		fprintf(fp, "-A POSTROUTING -p udp -s %s --dport %d -j NETMAP --to %s\n", lan_class, BASEPORT, wan_ip);

		//fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n",
		//			BASEPORT, logaccept);	// oleg patch
	}
#endif

	/* Exposed station */
	if (is_nat_enabled() && dmz_enabled()) {
		fprintf(fp, "-A VSERVER -j LOCALSRV\n");

#ifdef RTCONFIG_IPSEC
		/* Accept stateless IPSec tunnel */
		if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
			) {
			int isakmp_port = nvram_get_int("ipsec_isakmp_port") ? : 500;
			int nat_t_port = nvram_get_int("ipsec_nat_t_port") ? : 4500;

			fprintf(fp, "-A LOCALSRV -p udp -m multiport --dport %d,%d -j ACCEPT\n", isakmp_port, nat_t_port);
			fprintf(fp, "-A LOCALSRV -p esp -j ACCEPT\n");
			fprintf(fp, "-A LOCALSRV -p ah -j ACCEPT\n");
		}
#endif

#if 1 /* def RTCONFIG_WEBDAV */
		if (nvram_match("webdav_aidisk", "1")) {
			int port;

			port = nvram_get_int("webdav_https_port");
			if (!port || port >= 65536)
				port = 443;
			fprintf(fp, "-A LOCALSRV -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%d\n", port, lan_ip, port);
			port = nvram_get_int("webdav_http_port");
			if (!port || port >= 65536)
				port = 8082;
			fprintf(fp, "-A LOCALSRV -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%d\n", port, lan_ip, port);
		}
#endif

#ifdef RTCONFIG_MULTIWAN_CFG
		for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
			int i;
			char dst_ip[sizeof("-d 111.222.333.444XXXXXX")];
			char dmz_nv[sizeof("dmzXXX_ip")], *wan_iface[3];	/* 0: br0; 1: wan_if; 2: wanx_if; */

			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(nvram_get_int(strcat_r(prefix, "state_t", tmp)) != WAN_STATE_CONNECTED)
				continue;

			snprintf(dst_ip, sizeof(dst_ip), "-d %s", nvram_pf_safe_get(prefix, "ipaddr"));
			wanx_rules = 1;
			wan_iface[0] = lan_if;
			wan_iface[1] = get_wan_ifname(unit);
			wan_iface[2] = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			wanx_ip = nvram_safe_get(strcat_r(prefix, "xipaddr", tmp));

			if (strcmp(wan_iface[1], wan_iface[2]) && inet_addr_(wanx_ip))
				wanx_rules = 2;

			if (get_nr_wan_unit() == 2 && nvram_match("wans_mode", "lb")) {
				/* dualwan + load-balance */
				if (!unit)
					strlcpy(dmz_nv, "dmz_ip", sizeof(dmz_nv));
				else
					snprintf(dmz_nv, sizeof(dmz_nv), "dmz%d_ip", unit);

				for (i = 0; i <= wanx_rules && !nvram_match(dmz_nv, ""); ++i) {
					fprintf(fp, "-A VSERVER -i %s %s -j DNAT --to %s\n",
						wan_iface[i], i? "" : dst_ip, nvram_safe_get(dmz_nv));
				}
			} else
				need_dmz = 1;
		}
		if (need_dmz)
#endif
		{
			/* singlewan or dualwan + failover/fallback, only primary available. */
			fprintf(fp, "-A VSERVER -j DNAT --to %s\n", nvram_safe_get("dmz_ip"));
		}
	}

	if (is_nat_enabled())
	{
		char *p = "";
#ifdef RTCONFIG_IPV6
		switch (get_ipv6_service()) {
		case IPV6_6IN4:
			// avoid NATing proto-41 packets when using 6in4 tunnel
			p = "! -p 41";
			break;
		}
#endif
#ifdef RTCONFIG_IPSEC
		if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
			)
			fprintf(fp, "-A POSTROUTING -m policy --dir out --pol ipsec -j ACCEPT\n");
#endif
		for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
#ifdef RTCONFIG_MULTICAST_IPTV
			// avoid Movistar vlan interface skip by no link_wan nvram
			if (nvram_get_int("switch_stb_x") > 6 && (unit == WAN_UNIT_IPTV || unit == WAN_UNIT_VOIP)) {
				if (get_wan_state(unit) != WAN_STATE_CONNECTED) continue;
			}
			else
#endif
			if(!is_wan_connect(unit))
				continue;

			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			wan_if = get_wan_ifname(unit);
			wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
			wanx_if = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			wanx_ip = nvram_safe_get(strcat_r(prefix, "xipaddr", tmp));

			if(inet_addr_(wan_ip))
#ifdef HND_ROUTER
				fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE --mode %s\n", p, wan_if, wan_ip, (nvram_get_int("nat_type") ? "fullcone" : "symmetric"));
#else
				fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE\n", p, wan_if, wan_ip);
#endif

			/* masquerade physical WAN port connection */
			if (dualwan_unit__nonusbif(unit) && strcmp(wan_if, wanx_if) && inet_addr_(wanx_ip))
#ifdef HND_ROUTER
				fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE --mode %s\n", p, wanx_if, wanx_ip, (nvram_get_int("nat_type") ? "fullcone" : "symmetric"));
#else
				fprintf(fp, "-A POSTROUTING %s -o %s ! -s %s -j MASQUERADE\n", p, wanx_if, wanx_ip);
#endif

		}

		// masquerade lan to lan
		fprintf(fp, "-A POSTROUTING %s -o %s -s %s -d %s -j MASQUERADE\n", p, lan_if, lan_class, lan_class);

#ifdef RTCONFIG_WIFI_SON
		if (sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
			ip2class(g_lan_ip, nvram_safe_get("lan_netmask"), g_lan_class);
			fprintf(fp, "-A POSTROUTING %s -o %s -s %s -d %s -j MASQUERADE\n", p, BR_GUEST, g_lan_class, g_lan_class);
		}
#endif
	}
#ifdef RTCONFIG_FBWIFI
	{
		fbwifi_nat(fp);
	}
#endif

#ifdef RTCONFIG_FBWIFI
	if(sw_mode() == SW_MODE_ROUTER){
		fbwifi_nat(fp);
	}
#endif

#ifdef RTCONFIG_TOR
	 if(nvram_match("Tor_enable", "1")){
		nv = strdup(nvram_safe_get("Tor_redir_list"));
		if (strlen(nv) == 0) {
			fprintf(fp, "-I PREROUTING -i %s -p udp --dport 53 -j REDIRECT --to-ports %s\n", lan_if, nvram_safe_get("Tor_dnsport"));
			fprintf(fp, "-I PREROUTING -i %s -p udp --dport 123 -j REDIRECT --to-ports 123\n", lan_if); // requires an NTP server
			fprintf(fp, "-I PREROUTING -i %s -p tcp --syn ! -d %s -j REDIRECT --to-ports %s\n", lan_if, lan_class, nvram_safe_get("Tor_transport"));
		}
		else{
			while ((b = strsep(&nv, "<")) != NULL) {
				if (strlen(b)==0) continue;
				memset(addr_new, 0, sizeof(addr_new));
				addr_type = addr_type_parse(b, addr_new, sizeof(addr_new));
				if (addr_type == TYPE_IP){
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 53 -s %s -j REDIRECT --to-ports %s\n", lan_if, addr_new, nvram_safe_get("Tor_dnsport"));
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 123 -s %s -j REDIRECT --to-ports 123\n", lan_if, addr_new); // requires an NTP server
					fprintf(fp, "-I PREROUTING -i %s -p tcp --syn ! -d %s -s %s -j REDIRECT --to-ports %s\n", lan_if, lan_class, addr_new, nvram_safe_get("Tor_transport"));
				}
				else if (addr_type == TYPE_MAC){
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 53 -m mac --mac-source %s -j REDIRECT --to-ports %s\n", lan_if, addr_new, nvram_safe_get("Tor_dnsport"));
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 123 -m mac --mac-source %s -j REDIRECT --to-ports 123\n", lan_if, addr_new); // requires an NTP server
					fprintf(fp, "-I PREROUTING -i %s -p tcp --syn ! -d %s -m mac --mac-source %s -j REDIRECT --to-ports %s\n", lan_if, lan_class, addr_new, nvram_safe_get("Tor_transport"));
				}
				else if (addr_type == TYPE_IPRANGE){
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 53 -m iprange --src-range %s -j REDIRECT --to-ports %s\n", lan_if, addr_new, nvram_safe_get("Tor_dnsport"));
					fprintf(fp, "-I PREROUTING -i %s -p udp --dport 123 -m iprange --src-range %s -j REDIRECT --to-ports 123\n", lan_if, addr_new); // requires an NTP server
					fprintf(fp, "-I PREROUTING -i %s -p tcp --syn ! -d %s -m iprange --src-range %s -j REDIRECT --to-ports %s\n", lan_if, lan_class, addr_new, nvram_safe_get("Tor_transport"));
				}
			}
			free(nv);
		}
	}
#endif

	fprintf(fp, "COMMIT\n");
	fclose(fp);

	unlink(NAT_RULES);
	symlink(name, NAT_RULES);

	for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit) {
		if(is_phy_connect(unit)){
			/* force nat update */
			nvram_set_int("nat_state", NAT_STATE_UPDATE);
_dprintf("nat_rule: start_nat_rules 2.\n");
			start_nat_rules();
			break;
		}
		else
_dprintf("nat_rule: skip nat_rules because of no PHY.\n");
	}
}
#endif // RTCONFIG_DUALWAN

#ifdef WEB_REDIRECT
void redirect_setting(void)
{
	FILE *nat_fp, *redirect_fp;
	char tmp_buf[1024];
	char lan_class[32];
	char *lan_ipaddr_t, *lan_netmask_t;

#if defined(RTCONFIG_WIRELESSREPEATER) || defined(RTCONFIG_PROXYSTA)
/* [MUST]: Need to Clarify ... */
	if (client_mode() && !nvram_match("lan_proto", "static")) {
		lan_ipaddr_t = nvram_default_get("lan_ipaddr");
		lan_netmask_t = nvram_default_get("lan_netmask");
	}
	else
#endif
	{
		lan_ipaddr_t = nvram_safe_get("lan_ipaddr");
		lan_netmask_t = nvram_safe_get("lan_netmask");
	}
	ip2class(lan_ipaddr_t, lan_netmask_t, lan_class);

	if ((redirect_fp = fopen(REDIRECT_RULES, "w+")) == NULL) {
		fprintf(stderr, "*** Can't make the file of the redirect rules! ***\n");
		return;
	}

	if(is_router_mode() && (nat_fp = fopen(NAT_RULES, "r")) != NULL) {
		while ((fgets(tmp_buf, sizeof(tmp_buf), nat_fp)) != NULL
				&& strncmp(tmp_buf, "COMMIT", 6) != 0) {
#ifdef RTCONFIG_TOR
			if(nvram_match("Tor_enable", "1")){
				if(strstr(tmp_buf, "PREROUTING") && strstr(tmp_buf, "--to-ports 9053"))
					continue;
				if(strstr(tmp_buf, "PREROUTING") && strstr(tmp_buf, "--to-ports 9040"))
					continue;
			}
#endif
			fprintf(redirect_fp, "%s", tmp_buf);
		}
		fclose(nat_fp);
	}
	else{
		fprintf(redirect_fp, "*nat\n"
				":PREROUTING ACCEPT [0:0]\n"
				":POSTROUTING ACCEPT [0:0]\n"
				":OUTPUT ACCEPT [0:0]\n"
				":VSERVER - [0:0]\n"
				":PUPNP - [0:0]\n"
				":VUPNP - [0:0]\n");
#ifdef RTCONFIG_YANDEXDNS
		fprintf(redirect_fp,
				":YADNS - [0:0]\n");
#endif
#ifdef RTCONFIG_DNSFILTER
		fprintf(redirect_fp,
				":DNSFILTER - [0:0]\n");
#endif

#if 0
#ifdef RTCONFIG_AUTOCOVER_SIP
		if(nvram_get_int("atcover_sip") == 1 && !strcmp(lan_ipaddr_t, nvram_default_get("lan_ipaddr")) && strcmp(lan_ipaddr_t, nvram_safe_get("atcover_sip_ip"))){
			int dst_port;

			if(nvram_get_int("atcover_sip_type") == 1)
				dst_port = 80;
			else
				dst_port = 18017;

			fprintf(redirect_fp, "-A PREROUTING -d %s -p tcp --dport 80 -j DNAT --to-destination %s:%d\n",
					nvram_safe_get("atcover_sip_ip"),
					lan_ipaddr_t,
					dst_port
					);
		}
#endif
#endif
	}

	fprintf(redirect_fp,
		"-A PREROUTING ! -d %s -p tcp --dport 80 -j DNAT --to-destination %s:18017\n"
		"-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:18018\n",
		lan_class, lan_ipaddr_t,
		lan_ipaddr_t);
#ifdef RTCONFIG_YANDEXDNS
	fprintf(redirect_fp,
		"-I YADNS 1 -p udp -j DNAT --to-destination %s:18018\n", lan_ipaddr_t);
#endif
#ifdef RTCONFIG_ATEFROMWAN
	if(repeater_mode() && !nvram_get_int("x_Setting")) {
		fprintf(redirect_fp, "-I PREROUTING -i %s -p tcp -m tcp --dport 80 -j ACCEPT\n",
		nvram_safe_get("wan_ifnames"));
	}
#endif

#if defined(RTAX56_XD4) || defined(RTAC59_CD6N) || defined(PLAX56_XP4)
#if defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D"))
#endif // XP4
	write_extra_port_forwarding(redirect_fp, lan_ipaddr_t);
#endif
	fprintf(redirect_fp, "COMMIT\n");

	fclose(redirect_fp);
}
#endif

void write_access_restriction(FILE *fp)
{
	char *nv, *nvp, *b;
	char *enable, *srcip, *accessType;
	int  https_port = 0;
	int  http_port = 0;
	int  cnt = 0;
	char webports[16];
	char sshport[8];

	if (nvram_match("enable_acc_restriction", "1"))
	{
#ifdef RTCONFIG_HTTPS
		int en = nvram_get_int("http_enable");
		if (en != 0)
			https_port = nvram_get_int("https_lanport") ? : 443;

		if (en != 1)
#endif
		{
			http_port = nvram_get_int("http_lanport") ? : 80;
		}
		if (http_port != 0 && https_port != 0)
			snprintf(webports, sizeof(webports), "%d,%d", http_port, https_port);
		else
			snprintf(webports, sizeof(webports), "%d", http_port != 0 ? http_port : https_port);

#ifdef RTCONFIG_SSH
		if (nvram_get_int("sshd_enable") != 0)
			snprintf(sshport, sizeof(sshport), ",%d", nvram_get_int("sshd_port") ? : 22);
		else
#endif
			strcpy(sshport, "");

		fprintf(fp, "-A INPUT -p tcp -m multiport --dport %s%s%s -j ACCESS_RESTRICTION\n",
			    webports, sshport, nvram_get_int("telnetd_enable") == 1 ? ",23" : "");

		nvp = nv = strdup(nvram_safe_get("restrict_rulelist"));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			if ((vstrsep(b, ">", &enable, &srcip, &accessType) != 3)) continue;
			if (!strcmp(enable, "0")) continue;

			cnt++;
			if (!(atoi(accessType) ^ ACCESS_ALL)) {
#ifdef RTCONFIG_PROTECTION_SERVER
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -j RETURN\n", srcip);
#else
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -j ACCEPT\n", srcip);
#endif
				continue;
			}
				if (atoi(accessType) & ACCESS_WEBUI) {
					fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp -m multiport --dport %s -j ACCEPT\n", srcip, webports);
				}
#ifdef RTCONFIG_SSH
			if ((atoi(accessType) & ACCESS_SSH) && nvram_get_int("sshd_enable") != 0 ) {
#ifdef RTCONFIG_PROTECTION_SERVER
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp --dport %d -j RETURN\n", srcip, nvram_get_int("sshd_port") ? : 22);
#else
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp --dport %d -j ACCEPT\n", srcip, nvram_get_int("sshd_port") ? : 22);
#endif
			}
#endif
			if ((atoi(accessType) & ACCESS_TELNET) && nvram_get_int("telnetd_enable") == 1) {
#ifdef RTCONFIG_PROTECTION_SERVER
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp --dport 23 -j RETURN\n", srcip);
#else
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp --dport 23 -j ACCEPT\n", srcip);
#endif
			}
		}
		free(nv);
		if(cnt) {
			fprintf(fp, "-A ACCESS_RESTRICTION -j DROP\n");
		}
	}
}

/* Rules for LW Filter and MAC Filter
 * MAC ACCEPT
 *     ACCEPT -> MACS
 *             -> LW Disabled
 *                MACS ACCEPT
 *             -> LW Default Accept:
 *                MACS DROP in rules
 *                MACS ACCEPT Default
 *             -> LW Default Drop:
 *                MACS ACCEPT in rules
 *                MACS DROP Default
 *     DROP   -> FORWARD DROP
 *
 * MAC DROP
 *     DROP -> FORWARD DROP
 *     ACCEPT -> FORWARD ACCEPT
 */

void 	// 0928 add
start_default_filter(int lanunit)
{
	// TODO: handle multiple lan
	FILE *fp;
	char *nv, *nvp, *b;
	char *enable, *srcip, *accessType;
	char *lan_if = nvram_safe_get("lan_ifname");
	int evalRet;

	if (!is_routing_enabled())
		return;

	if ((fp = fopen("/tmp/filter.default", "w")) == NULL)
		return;
	fprintf(fp, "*filter\n"
		":INPUT DROP [0:0]\n"
		":FORWARD DROP [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":FUPNP - [0:0]\n"
		":ACCESS_RESTRICTION - [0:0]\n"
#ifdef RTCONFIG_OPENVPN
		":OVPN - [0:0]\n"
#endif
		":logaccept - [0:0]\n"
		":logdrop - [0:0]\n");
#ifdef RTCONFIG_PROTECTION_SERVER
	fprintf(fp, ":%sWAN - [0:0]\n", PROTECT_SRV_RULE_CHAIN);
	fprintf(fp, ":%sLAN - [0:0]\n", PROTECT_SRV_RULE_CHAIN);
#endif

#ifdef RTCONFIG_RESTRICT_GUI
	char word[PATH_MAX], *next_word;

	if(nvram_get_int("fw_restrict_gui")){
		foreach(word, nvram_safe_get("wl_ifnames"), next_word){
			SKIP_ABSENT_FAKE_IFACE(word);
			eval("ebtables", "-t", "broute", "-D", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "ACCEPT");
			eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "ACCEPT");
		}

		fprintf(fp, "-A INPUT -i %s -m mark --mark %s -d %s -p tcp -m multiport --dport 23,%d,%d,9999 -j DROP\n",
				lan_if, BIT_RES_GUI, nvram_safe_get("lan_ipaddr"),
				nvram_get_int("http_lanport") ? : 80,
				nvram_get_int("https_lanport") ? : 443);
	}
#endif

#ifdef RTCONFIG_PORT_BASED_VLAN
	/* Deny none br0 to access br0 subnet */
	if (vlan_enable() && strlen(nvram_safe_get("subnet_rulelist")))
		fprintf(fp, "-A INPUT ! -i br0 -d %s/%s -j DROP\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

	/* Write input rule for vlan */
	vlan_subnet_filter_input(fp);
#endif

#ifdef RTCONFIG_TAGGED_BASED_VLAN
	/* Deny none br0 to access br0 subnet */
	vlan_subnet_deny_input(fp);

	/* Write input rule for vlan */
	vlan_subnet_filter_input(fp);
#endif

#ifdef RTCONFIG_AMAS_WGN
	wgn_filter_input(fp);
#endif

	if (nvram_match("enable_acc_restriction", "1"))
	{
		int  https_port = 0;
		int  http_port = 0;
		int  cnt = 0;
		char webports[16];
		char sshport[8];
#ifdef RTCONFIG_HTTPS
		int en = nvram_get_int("http_enable");
		if (en != 0)
			https_port = nvram_get_int("https_lanport") ? : 443;

		if (en != 1)
#endif
		{
			http_port = nvram_get_int("http_lanport") ? : 80;
		}
		if (http_port != 0 && https_port != 0)
			snprintf(webports, sizeof(webports), "%d,%d", http_port, https_port);
		else
			snprintf(webports, sizeof(webports), "%d", http_port != 0 ? http_port : https_port);

#ifdef RTCONFIG_SSH
		if (nvram_get_int("sshd_enable") != 0)
			snprintf(sshport, sizeof(sshport), ",%d", nvram_get_int("sshd_port") ? : 22);
		else
#endif
			strcpy(sshport, "");

		fprintf(fp, "-A INPUT -p tcp -m multiport --dport %s%s%s -j ACCESS_RESTRICTION\n",
			    webports, sshport, nvram_get_int("telnetd_enable") == 1 ? ",23" : "");

		nvp = nv = strdup(nvram_safe_get("restrict_rulelist"));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			if ((vstrsep(b, ">", &enable, &srcip, &accessType) != 3)) continue;
			if (!strcmp(enable, "0")) continue;

			cnt++;
			if (!(atoi(accessType) ^ ACCESS_ALL)) {
#ifdef RTCONFIG_PROTECTION_SERVER
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -j RETURN\n", srcip);
#else
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -j ACCEPT\n", srcip);
#endif
				continue;
			}
				if (atoi(accessType) & ACCESS_WEBUI) {
					fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp -m multiport --dport %s -j ACCEPT\n", srcip, webports);
				}
#ifdef RTCONFIG_SSH
			if ((atoi(accessType) & ACCESS_SSH) && nvram_get_int("sshd_enable") != 0 ) {
#ifdef RTCONFIG_PROTECTION_SERVER
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp --dport %d -j RETURN\n", srcip, nvram_get_int("sshd_port") ? : 22);
#else
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp --dport %d -j ACCEPT\n", srcip, nvram_get_int("sshd_port") ? : 22);
#endif
			}
#endif
			if ((atoi(accessType) & ACCESS_TELNET) && nvram_get_int("telnetd_enable") == 1) {
#ifdef RTCONFIG_PROTECTION_SERVER
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp --dport 23 -j RETURN\n", srcip);
#else
				fprintf(fp, "-A ACCESS_RESTRICTION -s %s -p tcp --dport 23 -j ACCEPT\n", srcip);
#endif
			}
		}
		free(nv);
		if(cnt) {
			fprintf(fp, "-A ACCESS_RESTRICTION -j DROP\n");
		}
	}

	fprintf(fp, "-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n");
	fprintf(fp, "-A INPUT -m state --state INVALID -j DROP\n");

	/* Specific IP access restriction */
	write_access_restriction(fp);

#ifdef RTCONFIG_PROTECTION_SERVER
	if (nvram_get_int("telnetd_enable") != 0
#ifdef RTCONFIG_SSH
	    || nvram_get_int("sshd_enable") != 0
#endif
	) {
		fprintf(fp, "-A INPUT ! -i %s -j %sWAN\n", lan_if, PROTECT_SRV_RULE_CHAIN);
		fprintf(fp, "-A INPUT -i %s -j %sLAN\n", lan_if, PROTECT_SRV_RULE_CHAIN);
	}
#endif
	if(nvram_match("wifison_ready", "1"))
	{
#ifdef RTCONFIG_WIFI_SON
		if (sw_mode() != SW_MODE_REPEATER)
			fprintf(fp, "-A INPUT -i %s -m state --state NEW -j ACCEPT\n", "br+");
#else
		_dprintf("no wifison feature\n");
#endif
	}
	else
		fprintf(fp, "-A INPUT -i %s -m state --state NEW -j ACCEPT\n", lan_if);
	fprintf(fp, "-A INPUT -i %s -m state --state NEW -j ACCEPT\n", "lo");
	//fprintf(fp, "-A FORWARD -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT\n");
#ifdef RTCONFIG_PORT_BASED_VLAN
	/* Write forward rule for deny lan */
	vlan_subnet_deny_forward(fp);
#endif
#ifdef RTCONFIG_TAGGED_BASED_VLAN
	/* Write forward rule for deny lan */
	vlan_subnet_deny_forward(fp);
#endif
	fprintf(fp, "-A FORWARD -i %s -o %s -j ACCEPT\n", lan_if, lan_if);
	fprintf(fp, "-A FORWARD -i %s -o %s -j ACCEPT\n", "lo", "lo");

#ifdef RTCONFIG_WIFI_SON
	if (sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
		fprintf(fp, "-A FORWARD -i %s -o %s -j DROP\n", lan_if, BR_GUEST);
		fprintf(fp, "-A FORWARD -i %s -o %s -j DROP\n", BR_GUEST, lan_if);
		fprintf(fp, "-A FORWARD -i %s -j ACCEPT\n", BR_GUEST);
	}
#endif

	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");
	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	evalRet = eval("iptables-restore", "/tmp/filter.default");
	rule_apply_checking("firewall", __LINE__, "/tmp/filter.default", evalRet);

#ifdef RTCONFIG_IPV6
	if ((fp = fopen("/tmp/filter_ipv6.default", "w")) == NULL)
		return;
	fprintf(fp, "*filter\n"
		":INPUT DROP [0:0]\n"
		":FORWARD DROP [0:0]\n"
		":OUTPUT %s [0:0]\n"
		":logaccept - [0:0]\n"
		":logdrop - [0:0]\n",
		ipv6_enabled() ? "ACCEPT" : "DROP");

	if (ipv6_enabled()) {
		fprintf(fp, "-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n");
		fprintf(fp, "-A INPUT -m state --state INVALID -j DROP\n");
		fprintf(fp, "-A INPUT -i %s -m state --state NEW -j ACCEPT\n", lan_if);
		fprintf(fp, "-A INPUT -i %s -m state --state NEW -j ACCEPT\n", "lo");
		//fprintf(fp, "-A FORWARD -m state --state INVALID -j DROP\n");
		fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT\n");
		fprintf(fp, "-A FORWARD -i %s -o %s -j ACCEPT\n", lan_if, lan_if);
		fprintf(fp, "-A FORWARD -i %s -o %s -j ACCEPT\n", "lo", "lo");
	}

	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");
	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	evalRet = eval("ip6tables-restore", "/tmp/filter_ipv6.default");
	rule_apply_checking("firewall", __LINE__, "/tmp/filter_ipv6.default", evalRet);
#endif
}

#ifdef RTCONFIG_WIFI_SON
void set_cap_apmode_filter(void)
{
	const char *cap_rules = "/tmp/cap_apmode_filter.default";
	FILE *fp;
	char lan_class[32];
	int evalRet;

	if((fp = fopen(cap_rules, "w")) == NULL)
		return;
	if (sw_mode() == SW_MODE_REPEATER)
		return;

	ip2class(nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), lan_class);

	fprintf(fp, "*filter\n"
		":INPUT ACCEPT [0:0]\n"
		":FORWARD ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		);

	fprintf(fp, "-A INPUT -d %s/32 -i %s -j ACCEPT\n",APMODE_BRGUEST_IP, BR_GUEST);
	fprintf(fp, "-A INPUT -d %s -i %s -j DROP\n", lan_class, BR_GUEST);
	fprintf(fp, "-A FORWARD -d %s/32 -i %s -o %s -j ACCEPT\n", nvram_safe_get("lan_gateway"), BR_GUEST, nvram_safe_get("lan_ifname"));
	fprintf(fp, "-A FORWARD -d %s -i %s -o %s -j DROP\n", lan_class, BR_GUEST, nvram_safe_get("lan_ifname"));
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);
	evalRet = eval("iptables-restore", cap_rules);
	rule_apply_checking("firewall", __LINE__, cap_rules, evalRet);
#ifdef RTCONFIG_IPV6
	evalRet = eval("ip6tables-restore", cap_rules);
	rule_apply_checking("firewall", __LINE__, cap_rules, evalRet);
#endif	/* RTCONFIG_IPV6 */
}
#endif

#if defined(RTCONFIG_WIFI_SON) \
|| (defined(RTCONFIG_PRELINK) && defined(RTCONFIG_QCA))
void reset_filter(void)
{
	const char *filter_rules = "/tmp/hive_filter.default";
	FILE *fp;
	int evalRet;
	if ((fp = fopen(filter_rules, "w")) == NULL)
		return;
	fprintf(fp, "*filter\n"
		":INPUT ACCEPT [0:0]\n"
		":FORWARD ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		);
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	evalRet = eval("iptables-restore", filter_rules);
	rule_apply_checking("firewall", __LINE__, filter_rules, evalRet);
#ifdef RTCONFIG_IPV6
	evalRet = eval("ip6tables-restore", filter_rules);
	rule_apply_checking("firewall", __LINE__, filter_rules, evalRet);
#endif	/* RTCONFIG_IPV6 */
}
#endif


#ifdef WEBSTRFILTER
/* url filter scheduler */
int makeTimestr(char *tf, int size)
{
	if (!nvram_match("url_enable_x", "1")) return 0;

	// TODO : add new logic with url filter scheduler
	/* time condition nvram use "url_sched" */
	memset(tf, 0, size);

	return 1;
}
#endif

#ifdef CONTENTFILTER
/* keyword filter scheduler */
int makeTimestr_content(char *tf, int size)
{
	if (!nvram_match("keyword_enable_x", "1")) return 0;

	// TODO : add new logic with keyword filter scheduler
	/* time condition nvram use "keyword_sched" */
	memset(tf, 0, size);

	return 1;
}
#endif

#ifdef WEBSTRFILTER
#ifdef RTCONFIG_IPV6
void write_UrlFilter(char *chain, char *lan_if, char *lan_ip, char *logdrop, FILE *fp, FILE *fp_ipv6)
#else
void write_UrlFilter(char *chain, char *lan_if, char *lan_ip, char *logdrop, FILE *fp)
#endif
{
	char *nv, *nvp, *b;
	char *enable, *addr, *url;
	char srcips[64];
	char config_rule[20];
	char timef[256];

	char *ptr, *p, *pvalue;
	char list[256], list2[256];
	int first;

	snprintf(config_rule, sizeof(config_rule), "-d %s", lan_ip);
#ifdef RTCONFIG_IPV6
	char config_rulev6[128];
	snprintf(config_rulev6, sizeof(config_rulev6), "-d %s", nvram_safe_get("ipv6_rtr_addr"));

	/* Prevent v6 lan ip is NULL */
	if (strlen(config_rulev6) <= 3)
		strcpy(config_rulev6, "");

#endif
	if (makeTimestr(timef, sizeof(timef))) {
		nv = nvp = strdup(nvram_safe_get("url_rulelist"));
		while(nvp)
		{
			if ((b = strsep(&nvp, "<")) == NULL) break;
			if ((vstrsep(b, ">", &enable, &addr, &url)) != 3) continue;
			if (!strcmp(enable, "0"))
				continue;

			// Handle source type format
			srcips[0] = '\0';
			if (addr && *addr)
			{
				char srcAddr[64];
				int src_type = addr_type_parse(addr, srcAddr, sizeof(srcAddr));
				if (src_type == TYPE_IP)
					snprintf(srcips, sizeof(srcips), "-s %s", srcAddr);
				else if (src_type == TYPE_MAC)
					snprintf(srcips, sizeof(srcips), "-m mac --mac-source %s", srcAddr);
				else if (src_type == TYPE_IPRANGE)
					snprintf(srcips, sizeof(srcips), "-m iprange --src-range %s", srcAddr);
			}

			if (!strcmp(chain, "FORWARD") && nvram_match("url_mode_x", "0"))
				fprintf(fp, "-I FORWARD %s -p tcp %s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", srcips, timef, url);

			// Handle xxx.yyy.* format
			if (strstr(url, ".")) {
				memset(list, 0, sizeof(list));
				memset(list2, 0, sizeof(list2));

				ptr = pvalue = strdup(url);
				first = 1;
				while (pvalue && (p = strsep(&pvalue, ".")) != NULL) {
					if (!strlen(p)) {
						pvalue++;
						continue;
					}

					if (first) {
						first = 0;
						if (!strncasecmp(p, "www", 3))
							continue;
					}

					snprintf(list2, sizeof(list2), "%s|%02x|%s", list, strlen(p), p);
					strlcpy(list, list2, sizeof(list));
				}

				fprintf(fp, "%s %s -i %s %s %s -p udp --dport 53 %s -m string --icase --hex-string \"%s\" --algo bm -j %s\n",
				(strcmp(chain, "INPUT") == 0) ? "-A" : "-I", chain, lan_if, srcips,
				(strcmp(chain, "INPUT") == 0 && nvram_match("url_mode_x", "0")) ? config_rule : "", timef, list,
				(nvram_get_int("url_mode_x") == 0) ? logdrop : "ACCEPT");

				free(ptr);
			} else {
				fprintf(fp, "%s %s -i %s %s %s -p udp --dport 53 %s -m string --icase --string \"%s\" --algo bm -j %s\n",
				(strcmp(chain, "INPUT") == 0) ? "-A" : "-I", chain, lan_if, srcips,
				(strcmp(chain, "INPUT") == 0 && nvram_match("url_mode_x", "0")) ? config_rule : "", timef, url,
				(nvram_get_int("url_mode_x") == 0) ? logdrop : "ACCEPT");
			}
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled())
			{
				if (!strcmp(chain, "FORWARD") && nvram_match("url_mode_x", "0"))
					fprintf(fp_ipv6, "-I FORWARD -p tcp %s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", timef, url);

				// Handle xxx.yyy.* format
				if (strstr(url, ".")) {
					memset(list, 0, sizeof(list));
					memset(list2, 0, sizeof(list2));

					ptr = pvalue = strdup(url);
					first = 1;
					while (pvalue && (p = strsep(&pvalue, ".")) != NULL) {
						if (!strlen(p)) {
							pvalue++;
							continue;
						}

						if (first) {
							first = 0;
							if (!strncasecmp(p, "www", 3))
								continue;
						}

						snprintf(list2, sizeof(list2), "%s|%02x|%s", list, strlen(p), p);
						strlcpy(list, list2, sizeof(list));
					}

					fprintf(fp_ipv6, "%s %s -i %s %s -p udp --dport 53 %s -m string --icase --hex-string \"%s\" --algo bm -j %s\n",
					(strcmp(chain, "INPUT") == 0) ? "-A" : "-I", chain, lan_if,
					(strcmp(chain, "INPUT") == 0 && nvram_match("url_mode_x", "0")) ? config_rulev6 : "", timef, list,
					(nvram_get_int("url_mode_x") == 0) ? logdrop : "ACCEPT");

					free(ptr);
				} else {
					fprintf(fp_ipv6, "%s %s -i %s %s -p udp --dport 53 %s -m string --icase --string \"%s\" --algo bm -j %s\n",
					(strcmp(chain, "INPUT") == 0) ? "-A" : "-I", chain, lan_if,
					(strcmp(chain, "INPUT") == 0 && nvram_match("url_mode_x", "0")) ? config_rulev6 : "", timef, url,
					(nvram_get_int("url_mode_x") == 0) ? logdrop : "ACCEPT");
				}
			}
#endif
		}
		if (nv) free(nv);
	}

	if (nvram_match("url_mode_x", "1") && nvram_match("url_enable_x", "1")) {
		fprintf(fp, "-A %s -i %s -p udp --dport 53 -m string --icase --hex-string \"|04|asus|03|com|00|\" --algo bm -j ACCEPT\n", chain, lan_if);
		fprintf(fp, "-A %s -i br0 -p udp --dport 53 -j DROP\n", chain);
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled())
		{
			fprintf(fp_ipv6, "-A %s -i %s -p udp --dport 53 -m string --icase --hex-string \"|04|asus|03|com|00|\" --algo bm -j ACCEPT\n", chain, lan_if);
			fprintf(fp_ipv6, "-A %s -i br0 -p udp --dport 53 -j DROP\n", chain);
		}
#endif
	}
}
#endif

/**
 * If static route is defined (LAN -> Route), accept skbs that is coming from LAN/WAN/MAN, depends on route type,
 * and destination is defined in static route, with INVALID state in FORWARD chain.
 * @fp:
 * @prefix:	e.g., "lan_", "wan0_", "wan1_", etc.
 * @var:	e.g. "route", "mroute". Reference to caller of add_routes().
 * @ifname:
 * @addr:	IP address of @ifname
 * @nmask:	netmask of @ifname
 */
void __allow_sroutes(FILE *fp, char *prefix, char *var, char *ifname, char *addr, char *nmask)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];

	if (!fp || !prefix || !var || !ifname || !addr || !nmask)
		return;

	if (illegal_ipv4_address(addr) || illegal_ipv4_netmask(nmask))
		return;

	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {

		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;

		fprintf(fp, "-A FORWARD -i %s -o %s -s %s/%s -d %s/%s -j ACCEPT\n",
			ifname, ifname, addr, nmask, ipaddr, netmask);
	}

	return;
}

void allow_sroutes(FILE *fp)
{
	if (!fp || !nvram_match("sr_enable_x", "1"))
		return;

	/* LAN routes */
	__allow_sroutes(fp, "lan_", "route", nvram_get("lan_ifname"), nvram_get("lan_ipaddr"), nvram_get("lan_netmask"));
}

#if defined(RTCONFIG_SOC_IPQ8074) && \
    (defined(RTCONFIG_VPNC) || defined(RTCONFIG_VPN_FUSION))
int enable_gre_for_ecm(int unit, FILE *fp)
{
	char prefix[sizeof("wan0_XXXXXX")];

	if (unit < WAN_UNIT_FIRST || unit >= WAN_UNIT_MAX || !fp)
		return -1;

	if (!is_nat_enabled())
		return 0;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	if (!nvram_pf_match(prefix, "proto", "pptp"))
		return 0;

	fprintf(fp, "-A INPUT -i %s -p gre -j ACCEPT\n", nvram_pf_safe_get(prefix, "ifname"));

	return 0;
}
#else
static inline int enable_gre_for_ecm(int unit, FILE *fp) { return 0; }
#endif

void
filter_setting(int wan_unit, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	FILE *fp;	// oleg patch
	char *proto, *srcip, *srcport, *dstip, *dstport;
	char *nv, *nvp, *b;
	char *setting = NULL;
	char chain[32];
#ifdef RTCONFIG_PC_SCHED_V3
	char macdrop[32];
#else
	char macaccept[32];
#endif
	char prefix[32], tmp[100], *wan_proto;
#ifdef RTCONFIG_IPV6
	int i;
	FILE *fp_ipv6 = NULL;
	int n;
	char *ip, *protono;
#endif
	int v4v6_ok = IPT_V4;
#ifdef RTCONFIG_TOR
	char addr_new[32];
	int addr_type;
#endif
//2008.09 magic{
#ifdef WEBSTRFILTER
	char timef[256], *filterstr;
#endif
//2008.09 magic}
	char *wan_if, *wan_ip;
	char *wanx_if, *wanx_ip;
#ifdef RTCONFIG_WIFI_SON
	char lan_class[32];

	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
#endif
	int evalRet;

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);

	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	wan_if = get_wan_ifname(wan_unit);
	if (*wan_if == '\0')
		wan_if = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
	wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
	wanx_if = get_wanx_ifname(wan_unit);
	wanx_ip = nvram_safe_get(strcat_r(prefix, "xipaddr", tmp));

	//if(!strlen(wan_proto)) return;

	if ((fp=fopen("/tmp/filter_rules", "w"))==NULL) return;
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		if ((fp_ipv6 = fopen("/tmp/filter_rules_ipv6", "w"))==NULL) {
			fclose(fp);
			return;
		}
	}
#endif

	fprintf(fp, "*filter\n"
	    ":INPUT ACCEPT [0:0]\n"
	    ":FORWARD %s [0:0]\n"
	    ":OUTPUT ACCEPT [0:0]\n"
	    ":INPUT_PING - [0:0]\n"
	    ":INPUT_ICMP - [0:0]\n"
	    ":FUPNP - [0:0]\n"
	    ":SECURITY - [0:0]\n"
	    ":ACCESS_RESTRICTION - [0:0]\n"
#ifdef RTCONFIG_INTERNETCTRL
	    ":ICAccept - [0:0]\n"
	    ":ICDrop - [0:0]\n"
#endif
	    ":other2wan - [0:0]\n"
#ifdef RTCONFIG_OPENVPN
	    ":OVPN - [0:0]\n"
#endif
#ifdef RTCONFIG_DNSFILTER
	    ":DNSFILTER_DOT - [0:0]\n"
#endif
	    ":NSFW - [0:0]\n"
#ifdef RTCONFIG_PARENTALCTRL
	    ":PControls - [0:0]\n"
#endif
#if defined(WEB_REDIRECT)
	    ":default_block - [0:0]\n"
#endif

	    ":logaccept - [0:0]\n"
	    ":logdrop - [0:0]\n",
	    nvram_match("fw_enable_x", "1") ? "DROP" : "ACCEPT");

#ifdef RTCONFIG_PROTECTION_SERVER
	fprintf(fp, ":%sWAN - [0:0]\n", PROTECT_SRV_RULE_CHAIN);
	fprintf(fp, ":%sLAN - [0:0]\n", PROTECT_SRV_RULE_CHAIN);
#endif
#ifdef RTCONFIG_GN_WBL
	add_GN_WBL_ChainRule(fp);
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		fprintf(fp_ipv6, "*filter\n"
		    ":INPUT ACCEPT [0:0]\n"
		    ":FORWARD %s [0:0]\n"
		    ":OUTPUT ACCEPT [0:0]\n"
#ifdef RTCONFIG_INTERNETCTRL
		    ":ICAccept - [0:0]\n"
		    ":ICDrop - [0:0]\n"
#endif
		    ":UPNP - [0:0]\n"
#ifdef RTCONFIG_PARENTALCTRL
		    ":PControls - [0:0]\n"
#endif
		    ":NSFW - [0:0]\n"
		    ":logaccept - [0:0]\n"
		    ":logdrop - [0:0]\n",
		nvram_match("ipv6_fw_enable", "1") ? "DROP" : "ACCEPT");
	}
#endif

#ifdef RTCONFIG_PC_SCHED_V3
	strcpy(macdrop, "");
#else
	strcpy(macaccept, "");
#endif

// Setup traffic accounting
#if !defined(HND_ROUTER)
	if (nvram_match("cstats_enable", "1")) {
		fprintf(fp, ":ipttolan - [0:0]\n:iptfromlan - [0:0]\n");
		ipt_account(fp, NULL);
	}
#endif

#if defined(WEB_REDIRECT)
	/* Below rules are supposed to be used if below conditions are true
	 * and output interface should be WAN interface.
	 * 1. NAT is enabled.
	 * 2. DUT hasn't been configured. (x_Setting = 0)
	 * 3. defpsk is not enabled.
	 */
	if (is_nat_enabled() && nvram_match("x_Setting", "0") &&
			!find_word(nvram_safe_get("rc_support"), "defpsk"))
	{
		if (*wan_if) {
			/* Block all TCP ports, except 80 and 443. */
			fprintf(fp,
				"-A default_block -o %s -p tcp --dport 80 -j %s\n"
				"-A default_block -o %s -p tcp --dport 443 -j %s\n"
				"-A default_block -o %s -p tcp -j %s\n",
				wan_if, logaccept,
				wan_if, logaccept,
				wan_if, logdrop);
			/* Block ICMP echo request. */
			fprintf(fp,
				"-A default_block -o %s -p icmp ! --icmp-type echo-request -j %s\n"
				"-A default_block -o %s -p icmp -j %s\n",
				wan_if, logaccept,
				wan_if, logdrop);
		}
	}
#endif

#ifdef RTCONFIG_INTERNETCTRL
	ic_s *ic_list = NULL;
	int ic_count;

	get_all_ic_list(&ic_list);
	ic_count = count_ic_rules(ic_list);

	if(/*nvram_get_int("MULTIFILTER_ALL") != 0 && */ic_count > 0){
TRACE_PT("writing Internet Control\n");
		config_ic_rule_string(ic_list, fp, logaccept, logdrop, 1);

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()){
			config_ic_rule_string(ic_list, fp_ipv6, logaccept, logdrop, 1);
		}
#endif

		//strcpy(macaccept, "AControls");
	}
	free_ic_list(&ic_list);
#endif

#ifdef RTCONFIG_PARENTALCTRL
	pc_s *pc_list = NULL;
	int pc_count;

	get_all_pc_tmp_list(&pc_list);
	pc_count = count_pc_rules(pc_list, 1);
	pc_count += count_pc_rules(pc_list, 2);

	if(pc_count > 0){
TRACE_PT("writing temporary Parental Control\n");
		config_pause_block_string(pc_list, fp, logaccept, logdrop, 2);
		config_daytime_string(pc_list, fp, logaccept, logdrop, 1);

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()){
			config_pause_block_string(pc_list, fp_ipv6, logaccept, logdrop, 2);
			config_daytime_string(pc_list, fp_ipv6, logaccept, logdrop, 1);
		}
#endif

#ifdef RTCONFIG_PC_SCHED_V3
		strcpy(macdrop, "PControls");
#else
		strcpy(macaccept, "PControls");
#endif
	}
	free_pc_list(&pc_list);

	get_all_pc_list(&pc_list);
	pc_count = count_pc_rules(pc_list, 1);
	pc_count += count_pc_rules(pc_list, 2);

	if(nvram_get_int("MULTIFILTER_ALL") != 0 && pc_count > 0){
TRACE_PT("writing Parental Control\n");
		config_pause_block_string(pc_list, fp, logaccept, logdrop, 0);
		config_daytime_string(pc_list, fp, logaccept, logdrop, 0);

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()){
			config_pause_block_string(pc_list, fp_ipv6, logaccept, logdrop, 0);
			config_daytime_string(pc_list, fp_ipv6, logaccept, logdrop, 0);
		}
#endif

#ifdef RTCONFIG_PC_SCHED_V3
		strcpy(macdrop, "PControls");
#else
		strcpy(macaccept, "PControls");
#endif
	}
	free_pc_list(&pc_list);
#endif

#ifdef RTCONFIG_RESTRICT_GUI
	if(nvram_get_int("fw_restrict_gui")){
		char word[PATH_MAX], *next_word;

		foreach(word, nvram_safe_get("wl_ifnames"), next_word){
			SKIP_ABSENT_FAKE_IFACE(word);
			eval("ebtables", "-t", "broute", "-D", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "ACCEPT");
			eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "ACCEPT");
		}

		fprintf(fp, "-A INPUT -i %s -m mark --mark %s -d %s -p tcp -m multiport --dport 23,%d,%d,9999 -j DROP\n",
				lan_if, BIT_RES_GUI, lan_ip,
				nvram_get_int("http_lanport") ? : 80,
				nvram_get_int("https_lanport") ? : 443);
	}
#endif

#ifdef RTCONFIG_FREERADIUS
		if (nvram_get_int("radius_serv_enable")) {
			fprintf(fp, "-A INPUT -m conntrack --ctstate DNAT -p udp -d %s --dport %d -j ACCEPT\n", lan_ip, 1812);
		}
#endif

	if (nvram_match("fw_enable_x", "1")) {
		/* Drop ICMP before ESTABLISHED state */
		if (!nvram_get_int("misc_ping_x")) {
			fprintf(fp, "-A %s -p icmp --icmp-type %d -j %s\n", "INPUT", 8, "INPUT_PING");
#ifdef RTCONFIG_IPV6
			if (get_ipv6_service() == IPV6_6IN4) {
				/* accept ICMP requests from the remote tunnel endpoint */
				ip = nvram_safe_get(ipv6_nvname("ipv6_tun_v4end"));
				if (ip && *ip && inet_addr_(ip) != INADDR_ANY)
					fprintf(fp, "-A %s -s %s -p icmp -j %s\n", "INPUT_PING", ip, logaccept);
				/* accept ICMP requests from he.net checker */
				fprintf(fp, "-A %s -s %s -p icmp -j %s\n", "INPUT_PING", "66.220.2.74", logaccept);
			}
#endif
#ifdef RTCONFIG_IPSEC
			if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
			 || nvram_get_int("ipsec_ig_enable")
#endif
				)
				fprintf(fp, "-A %s -p icmp -m policy --dir in --pol ipsec -j %s\n", "INPUT_PING", "RETURN");
#endif
			fprintf(fp, "-A %s -i %s -p icmp -j %s\n", "INPUT_PING", wan_if, logdrop);
			if (strcmp(wanx_if, wan_if) && inet_addr_(wanx_ip) && dualwan_unit__nonusbif(wan_unit))
				fprintf(fp, "-A %s -i %s -p icmp -j %s\n", "INPUT_PING", wanx_if, logdrop);
		}

#ifdef RTCONFIG_IPSEC
		/* Accept stateless IPSec tunnel */
		if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
			) {
			int isakmp_port = nvram_get_int("ipsec_isakmp_port") ? : 500;
			int nat_t_port = nvram_get_int("ipsec_nat_t_port") ? : 4500;

			fprintf(fp, "-A INPUT -p udp -m multiport --dport %d,%d -j %s\n", isakmp_port, nat_t_port, "ACCEPT");
			fprintf(fp, "-A INPUT -p esp -j %s\n", "ACCEPT");
			fprintf(fp, "-A INPUT -p ah -j %s\n", "ACCEPT");
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled()) {
				fprintf(fp_ipv6, "-A INPUT -p udp -m multiport --dport %d,%d -j %s\n", isakmp_port, nat_t_port, "ACCEPT");
				fprintf(fp_ipv6, "-A INPUT -p esp -j %s\n", "ACCEPT");
			}
#endif
		}
#endif

#ifdef RTCONFIG_IPV6
		write_UrlFilter("INPUT", lan_if, lan_ip, logdrop, fp, fp_ipv6);
#else
		write_UrlFilter("INPUT", lan_if, lan_ip, logdrop, fp);
#endif

		/* Filter known SPI state */
		fprintf(fp, "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n", logaccept);
		fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n", logdrop);

#ifdef RTCONFIG_TAGGED_BASED_VLAN
		/* Deny none brX to access brX subnet */
		vlan_subnet_deny_input(fp);
#endif

		/* Specific IP access restriction */
		write_access_restriction(fp);

#ifdef RTCONFIG_PROTECTION_SERVER
		if (nvram_get_int("telnetd_enable") != 0
#ifdef RTCONFIG_SSH
		    || nvram_get_int("sshd_enable") != 0
#endif
   		) {
			fprintf(fp, "-A INPUT ! -i %s -j %sWAN\n", lan_if, PROTECT_SRV_RULE_CHAIN);
			fprintf(fp, "-A INPUT -i %s -j %sLAN\n", lan_if, PROTECT_SRV_RULE_CHAIN);
		}
#endif
		if (nvram_match("wifison_ready", "1"))
		{
#ifdef RTCONFIG_WIFI_SON
			if (sw_mode() != SW_MODE_REPEATER) {
				fprintf(fp, "-A INPUT -i %s -d %s -j DROP\n", BR_GUEST, lan_class);
				fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n", "br+", "ACCEPT");
			}
			else
				fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n", lan_if, "ACCEPT");
#else
			_dprintf("no wifison feature\n");
#endif
		}
		else
			fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n", lan_if, "ACCEPT");
#ifdef RTCONFIG_IPSEC
		if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
			)
			fprintf(fp, "-A INPUT -m policy --dir in --pol ipsec -m state --state NEW -j %s\n", "ACCEPT");
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
		if (nvram_get_int("pptpd_enable"))
			fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n", "pptp+", "ACCEPT");
#endif
		fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n", "lo", "ACCEPT");
#ifdef RTCONFIG_OPENVPN
		fprintf(fp, "-A INPUT -m state --state NEW -j OVPN\n");
#endif
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()) {
			fprintf(fp_ipv6, "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n", logaccept);
			fprintf(fp_ipv6, "-A INPUT -i %s -m state --state NEW -j %s\n", lan_if, "ACCEPT");
			fprintf(fp_ipv6, "-A INPUT -i %s -m state --state NEW -j %s\n", "lo", "ACCEPT");
			fprintf(fp_ipv6, "-A INPUT -m state --state INVALID -j %s\n", logdrop);
		}
#endif

		/* Pass multicast */
		if (nvram_get_int("mr_enable_x") || nvram_get_int("udpxy_enable_x")) {
			fprintf(fp, "-A INPUT -p 2 -d 224.0.0.0/4 -j %s\n", logaccept);
			fprintf(fp, "-A INPUT -p udp -d 224.0.0.0/4 ! --dport 1900 -j %s\n", logaccept);
		}
#ifdef RTCONFIG_IPV6
		/* not necesary for now, udpxy is ipv4-only
		if (ipv6_enabled() && (nvram_get_int("mr_enable_x") & 2)) {
			fprintf(fp_ipv6, "-A INPUT -p udp -d ff00::/8 ! --dport 1900 -j %s\n", logaccept);
		}
		*/
#endif

		/* enable incoming packets from broken dhcp servers, which are sending replies
		 * from addresses other than used for query, this could lead to lower level
		 * of security, but it does not work otherwise (conntrack does not work) :-(
		 */
		if (strcmp(wan_proto, "dhcp") == 0 || inet_addr_(wan_ip) == INADDR_ANY ||
		    nvram_get_int(strcat_r(prefix, "dhcpenable_x", tmp))) {
			fprintf(fp, "-A INPUT -p udp --sport 67 --dport 68 -j %s\n", logaccept);
		}

		// Firewall between WAN and Local
		if (nvram_get_int("misc_http_x")) {
#ifdef RTCONFIG_HTTPS
			int enable = nvram_get_int("http_enable");
			if (enable != 0) {
				fprintf(fp, "-A INPUT -m conntrack --ctstate DNAT -p tcp -m tcp -d %s --dport %d -j %s\n",
					lan_ip, nvram_get_int("https_lanport") ? : 443, logaccept);
			}
			/* do not support http (enable != 1) */
#else
			{
				fprintf(fp, "-A INPUT -m conntrack --ctstate DNAT -p tcp -m tcp -d %s --dport %d -j %s\n",
					lan_ip, nvram_get_int("http_lanport") ? : 80, logaccept);
			}
#endif
		}

#ifdef RTCONFIG_SSH
		// Open ssh to WAN
		if (nvram_get_int("sshd_enable") == 1)
		{
			if (nvram_match("sshd_bfp", "1"))
			{
				fprintf(fp, "-N SSHBFP\n");
				fprintf(fp, "-A SSHBFP -m recent --set --name SSH --rsource\n");
				fprintf(fp, "-A SSHBFP -m recent --update --seconds 60 --hitcount 4 --name SSH --rsource -j %s\n", logdrop);
				fprintf(fp, "-A SSHBFP -j %s\n", logaccept);
				fprintf(fp, "-A INPUT -p tcp --dport %d -m state --state NEW -j SSHBFP\n",
				        nvram_get_int("sshd_port") ? : 22);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled())
				{
					fprintf(fp_ipv6, "-N SSHBFP\n");
					fprintf(fp_ipv6, "-A SSHBFP -m recent --set --name SSH --rsource\n");
					fprintf(fp_ipv6, "-A SSHBFP -m recent --update --seconds 60 --hitcount 4 --name SSH --rsource -j %s\n", logdrop);
					fprintf(fp_ipv6, "-A SSHBFP -j %s\n", logaccept);
					fprintf(fp_ipv6, "-A INPUT -p tcp --dport %d -m state --state NEW -j SSHBFP\n",
						nvram_get_int("sshd_port") ? : 22);
				}
#endif

			}
			else
			{
				fprintf(fp, "-A INPUT -p tcp --dport %d -j %s\n",
				        nvram_get_int("sshd_port") ? : 22, logaccept);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled())
					fprintf(fp_ipv6, "-A INPUT -p tcp --dport %d -j %s\n",
						nvram_get_int("sshd_port") ? : 22, logaccept);

#endif

			}
	}
#endif

#ifdef RTCONFIG_FTP
		if ((nvram_get_int("enable_ftp")) && (nvram_get_int("ftp_wanac")))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport 21 -j %s\n", logaccept);
			int passive_port = nvram_get_int("ftp_pasvport");
			if (passive_port)
				fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d:%d -j %s\n", passive_port, passive_port+30, logaccept);

#ifdef RTCONFIG_IPV6
			if (ipv6_enabled() && nvram_match("ipv6_fw_enable", "1"))
			{
				fprintf(fp_ipv6, "-A INPUT -p tcp -m tcp --dport 21 -j %s\n", logaccept);
				if (passive_port)
					fprintf(fp_ipv6, "-A INPUT -p tcp -m tcp --dport %d:%d -j %s\n", passive_port, passive_port+30, logaccept);
			}
#endif
		}
#endif

#ifdef RTCONFIG_WEBDAV
		if (nvram_match("enable_webdav", "1"))
		{
			if(nvram_get_int("st_webdav_mode")!=1) {
				fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j DROP\n", nvram_safe_get("webdav_http_port"));	// oleg patch
			}

			if(nvram_get_int("st_webdav_mode")!=0) {
				fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("webdav_https_port"), logaccept);	// oleg patch
			}
		}
#endif

		/* Pass ICMP */
		if (!nvram_get_int("misc_ping_x")) {
			fprintf(fp, "-A %s -p icmp -j %s\n", "INPUT", "INPUT_ICMP");
			fprintf(fp, "-A %s -p icmp --icmp-type %d -j %s\n", "INPUT_ICMP", 8, "RETURN");
			fprintf(fp, "-A %s -p icmp --icmp-type %d -j %s\n", "INPUT_ICMP", 13, "RETURN");
			fprintf(fp, "-A %s -p icmp -j %s\n", "INPUT_ICMP", logaccept);
		} else
			fprintf(fp, "-A %s -p icmp -j %s\n", "INPUT", logaccept);

		if (!nvram_match("misc_lpr_x", "0"))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 515, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 9100, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 3838, logaccept);	// oleg patch
		}

#ifdef RTCONFIG_MULTICAST_IPTV
		if (nvram_get_int("switch_stb_x") > 6) {
			char *movistar_wan;
			if (nvram_invmatch("wan0_pppoe_ifname", ""))
				movistar_wan = nvram_safe_get("wan0_pppoe_ifname");
			else
				movistar_wan = nvram_safe_get("wan0_ifname");
			if (nvram_match("switch_wantag", "movistar")) {
				fprintf(fp, "-A INPUT -i %s -s 10.0.0.0/255.0.0.0 -j DROP\n", movistar_wan);
				fprintf(fp, "-A INPUT -i %s -s 172.16.0.0/255.255.15.0 -j DROP\n", movistar_wan);
				fprintf(fp, "-A INPUT -i vlan2 -s 172.16.0.0/255.255.15.0 -j ACCEPT\n");
				fprintf(fp, "-A INPUT -i vlan3 -s 10.31.255.134/255.255.255.255 -j ACCEPT\n");
				fprintf(fp, "-A INPUT -p udp --dport 520 -j %s\n", "ACCEPT");
			}
		}
#endif

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
		//Add for pptp server
		if (nvram_get_int("pptpd_enable")) {
			fprintf(fp, "-A INPUT -p tcp --dport %d -j %s\n", 1723, logaccept);
			fprintf(fp, "-A INPUT -p 47 -j %s\n", logaccept);
		}
#endif

		enable_gre_for_ecm(wan_unit, fp);

		//Add for snmp daemon
		if (nvram_match("snmpd_enable", "1") && nvram_match("snmpd_wan", "1"))
			fprintf(fp, "-A INPUT -p udp  --dport 161:162 -j %s\n", logaccept);

#ifdef RTCONFIG_IPV6
		switch (get_ipv6_service()) {
		case IPV6_6IN4:
		case IPV6_6TO4:
		case IPV6_6RD:
			fprintf(fp, "-A INPUT -p 41 -j %s\n", "ACCEPT");
			break;
		}
#endif

#ifdef RTCONFIG_PORT_BASED_VLAN
		/* Deny none br0 to access br0 subnet */
		if (vlan_enable() && strlen(nvram_safe_get("subnet_rulelist")))
			fprintf(fp, "-A INPUT ! -i br0 -d %s/%s -j DROP\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

		/* Write input rule for vlan */
		vlan_subnet_filter_input(fp);
#endif
#ifdef RTCONFIG_TAGGED_BASED_VLAN
		/* Write input rule for vlan */
		vlan_subnet_filter_input(fp);
#endif

#ifdef RTCONFIG_TR069
		if (nvram_get_int("tr_enable")) {
			char tr_acs_acl[512] = {0};
			char word[64] = {0};
			char *next = NULL;
			int tr_conn_port = nvram_get_int("tr_conn_port");

			nvram_safe_get_r("tr_acs_acl", tr_acs_acl, sizeof(tr_acs_acl));
			if (strlen(tr_acs_acl)) {
				foreach_44(word, tr_acs_acl, next) {
					fprintf(fp, "-A INPUT -s %s -p tcp -m tcp --dport %d -j %s\n",
						word, tr_conn_port, logaccept);
					fprintf(fp, "-A INPUT -s %s -p udp -m udp --dport %d -j %s\n",
						word, tr_conn_port, logaccept);
				}
			}
			else {
				fprintf(fp, "-A INPUT -p udp -m udp --dport %d -j %s\n",
					nvram_get_int("tr_conn_port"), logaccept);
			}

			if(strlen(nvram_safe_get("tr_acs_url"))) {
				struct addrinfo *res, *ressave;
				int ret;
				ret=url2ip(nvram_safe_get("tr_acs_url"), &res);
				ressave = res;

				if( (ret != -1) && (res!=NULL) && (res->ai_addr !=NULL) )
				{
					do {
						if(res->ai_family == AF_INET)
						if(res->ai_family == AF_INET)
						{
							//ipv4
							char ipv4str[INET_ADDRSTRLEN] = {0};
							if (inet_ntop(AF_INET, &(((struct sockaddr_in *)(res->ai_addr))->sin_addr),ipv4str, INET_ADDRSTRLEN) == NULL)
								perror("inet_ntop"); 
							fprintf(fp, "-A INPUT -s %s -p tcp -m tcp --dport %d -j %s\n", ipv4str, nvram_get_int("tr_conn_port"), logaccept);						
						}
						else if(res->ai_family == AF_INET6)
						{
							char ipv6str[INET6_ADDRSTRLEN];
							if(inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr),ipv6str, INET6_ADDRSTRLEN) == NULL)
								perror("inet_ntop"); 
								//TODO
								//add ipv6 addr to ip6tables
						}

					} while( ( res = res->ai_next ) != NULL );

					freeaddrinfo( ressave );
				}
			}
		}
#endif

#ifdef RTCONFIG_AMAS_WGN
		wgn_filter_input(fp);
#endif

#if defined(RTCONFIG_WIFI_SON) && defined(RTCONFIG_ETHBACKHAUL)
		if (sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1"))
			fprintf(fp, "-A INPUT -p udp --sport 9413 --dport 9413 -j ACCEPT\n");
#endif
		fprintf(fp, "-A INPUT -j %s\n", logdrop);
	}

/* apps_dm DHT patch */
	if (nvram_match("apps_dl_share", "1"))
	{
		fprintf(fp, "-I INPUT -p udp --dport 6881 -j ACCEPT\n");	// DHT port
		// port range
		fprintf(fp, "-I INPUT -p udp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
		fprintf(fp, "-I INPUT -p tcp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
	}

	/* Pass multicast */
	if (nvram_get_int("mr_enable_x"))
		fprintf(fp, "-A FORWARD -p udp -d 224.0.0.0/4 -j ACCEPT\n");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && (nvram_get_int("mr_enable_x") & 2))
		fprintf(fp_ipv6, "-A FORWARD -p udp -d ff00::/8 -j ACCEPT\n");
#endif

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if (nvram_get_int("jumbo_frame_enable") ||
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	    nvram_get_int("pptpd_enable") ||
#endif
#if defined(RTCONFIG_USB_MODEM)
	    dualwan_unit__usbif(wan_unit) ||
#endif
	    strcmp(wan_proto, "pppoe") == 0 ||
	    strcmp(wan_proto, "pptp") == 0 ||
	    strcmp(wan_proto, "l2tp") == 0) {
		fprintf(fp, "-A FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
#ifdef RTCONFIG_PC_SCHED_V3
		if (*macdrop)
			fprintf(fp, "-A %s -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", macdrop);
#else
		if (*macaccept)
			fprintf(fp, "-A %s -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", macaccept);
#endif
	}
#ifdef RTCONFIG_IPV6
	switch (get_ipv6_service()) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		if (!nvram_get_int("jumbo_frame_enable") &&
#if defined(RTCONFIG_USB_MODEM)
		    dualwan_unit__nonusbif(wan_unit) &&
#endif
		    !(strcmp(wan_proto, "dhcp") != 0 && strcmp(wan_proto, "static") != 0 &&
		      nvram_match(ipv6_nvname("ipv6_ifdev"), "ppp")))
			break;
		/* fall through */
	case IPV6_6IN4:
	case IPV6_6TO4:
	case IPV6_6RD:
#ifdef RTCONFIG_PC_SCHED_V3
		if (*macdrop)
			fprintf(fp_ipv6, "-A %s -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", macdrop);
#else
 				fprintf(fp_ipv6, "-A FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
		if (*macaccept)
			fprintf(fp_ipv6, "-A %s -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", macaccept);
#endif
		break;
	}
#endif

	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && nvram_match("ipv6_fw_enable", "1"))
		fprintf(fp_ipv6, "-A FORWARD -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
#endif
#ifdef RTCONFIG_IPSEC
	if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
		)
		fprintf(fp, "-A FORWARD -m policy --dir in --pol ipsec -j %s\n", "ACCEPT");
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	if (nvram_get_int("pptpd_enable"))
		fprintf(fp, "-A FORWARD -i %s -j %s\n", "pptp+", "ACCEPT");
#endif

	/* Filter out invalid WAN->WAN connections */
#ifdef RTCONFIG_PORT_BASED_VLAN
	/* Write forward rule for vlan */
	vlan_subnet_filter_forward(fp, wan_if);

	/* Write forward rule for deny lan */
	vlan_subnet_deny_forward(fp);
#endif

#ifdef RTCONFIG_TAGGED_BASED_VLAN
	/* Write forward rule for vlan */
	vlan_subnet_filter_forward(fp, wan_if);

	/* Write forward rule for deny lan */
	vlan_subnet_deny_forward(fp);
#endif

#ifdef RTCONFIG_AMAS_WGN
	wgn_filter_forward(fp);
#endif

	if(nvram_match("wifison_ready", "1"))
	{
#ifdef RTCONFIG_WIFI_SON
		if (sw_mode() != SW_MODE_REPEATER) {
			fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wan_if, "br+", "other2wan");
		}
		else
			fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wan_if, lan_if, "other2wan");
#else
		_dprintf("no wifison feature\n");
#endif
	}
	else
		fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wan_if, lan_if, "other2wan");

	if (strcmp(wanx_if, wan_if) && inet_addr_(wanx_ip) && dualwan_unit__nonusbif(wan_unit)) {
		if (nvram_match("wifison_ready", "1"))
		{
#ifdef RTCONFIG_WIFI_SON
			if (sw_mode() != SW_MODE_REPEATER) {
				fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wanx_if, "br+", "other2wan");
			}
			else
				fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wanx_if, lan_if, "other2wan");
#else
			_dprintf("no wifison feature\n");
#endif
		}
		else
			fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wanx_if, lan_if, logdrop);
	}

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && *wan6face) {
		if (nvram_match("ipv6_fw_enable", "1")) {
			fprintf(fp_ipv6, "-A FORWARD -o %s -i %s -j %s\n", wan6face, lan_if, logaccept);
		} else {	// The default DROP rule from the IPv6 firewall would take care of it
	fprintf(fp_ipv6, "-A FORWARD -o %s ! -i %s -j %s\n", wan6face, lan_if, logdrop);
		}
	}
#endif

#ifdef RTCONFIG_GN_WBL
	add_GN_WBL_ForwardRule(fp);
#endif

	fprintf(fp, "-A other2wan -i tun+ -j RETURN\n");	// Let OVPN traffic through
	fprintf(fp, "-A other2wan -j %s\n", logdrop);		// Drop other foreign traffic

	/* Accept the redirect, might be seen as INVALID, packets */
	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
#ifdef RTCONFIG_PC_SCHED_V3
	if (*macdrop)
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", macdrop, lan_if, lan_if, logdrop);
#else
	if (*macaccept)
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", macaccept, lan_if, lan_if, logaccept);
#endif
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		fprintf(fp_ipv6, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
#ifdef RTCONFIG_PC_SCHED_V3
		if (*macdrop)
			fprintf(fp_ipv6, "-A %s -i %s -o %s -j %s\n", macdrop, lan_if, lan_if, logdrop);
#else
	if (*macaccept)
			fprintf(fp_ipv6, "-A %s -i %s -o %s -j %s\n", macaccept, lan_if, lan_if, logaccept);
#endif
	}
#endif

	allow_sroutes(fp);

	/* Drop the wrong state, INVALID, packets */
	fprintf(fp, "-A FORWARD -m state --state INVALID -j %s\n", logdrop);
#ifdef RTCONFIG_PC_SCHED_V3
	if (*macdrop)
		fprintf(fp, "-A %s -m state --state INVALID -j %s\n", macdrop, logdrop);
#else
	if (*macaccept)
		fprintf(fp, "-A %s -m state --state INVALID -j %s\n", macaccept, logdrop);
#endif
//#if 0
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		fprintf(fp_ipv6, "-A FORWARD -m state --state INVALID -j %s\n", logdrop);
#ifdef RTCONFIG_PC_SCHED_V3
		if (*macdrop)
			fprintf(fp_ipv6, "-A %s -m state --state INVALID -j %s\n", macdrop, logdrop);
#else
	if (*macaccept)
			fprintf(fp_ipv6, "-A %s -m state --state INVALID -j %s\n", macaccept, logdrop);
#endif
	}
#endif
//#endif

#ifdef RTCONFIG_WIFI_SON
	if (sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
		fprintf(fp, "-A FORWARD -i %s -o %s -j DROP\n", lan_if, BR_GUEST);
		fprintf(fp, "-A FORWARD -i %s -o %s -j DROP\n", BR_GUEST, lan_if);
		fprintf(fp, "-A FORWARD -i %s -j ACCEPT\n", BR_GUEST);
	}
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
		fprintf(fp_ipv6, "-A FORWARD -p ipv6-nonxt -m length --length 40 -j ACCEPT\n");

		// ICMPv6 rules
		for (i = 0; i < sizeof(allowed_icmpv6)/sizeof(int); ++i) {
			fprintf(fp_ipv6, "-A FORWARD -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_icmpv6[i], logaccept);
		}
	}
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
#if 0
		fprintf(fp_ipv6,
			/* "-A INPUT -m state --state INVALID -j DROP\n" */
			"-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n",
			logaccept);
#endif
		fprintf(fp_ipv6, "-A INPUT -p ipv6-nonxt -m length --length 40 -j ACCEPT\n");

		fprintf(fp_ipv6,
			"-A INPUT -i %s -j ACCEPT\n" // anything coming from LAN
			"-A INPUT -i lo -j ACCEPT\n",
				lan_if);

		switch (get_ipv6_service()) {
#ifdef RTCONFIG_6RELAYD
		case IPV6_PASSTHROUGH:
			/* allow responses from the dhcpv6 server to relay, not used so far
			fprintf(fp_ipv6, "-A INPUT -p udp --sport 547 --dport 547 -j %s\n", logaccept); */
			/* fall through */
#endif
		case IPV6_NATIVE_DHCP:
			/* allow responses from the dhcpv6 server */
			fprintf(fp_ipv6, "-A INPUT -p udp --sport 547 --dport 546 -j %s\n", logaccept);
			break;
		}

		// ICMPv6 rules
		for (n = 0; n < sizeof(allowed_icmpv6)/sizeof(int); n++) {
			fprintf(fp_ipv6, "-A INPUT -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_icmpv6[n], logaccept);
		}
		for (n = 0; n < sizeof(allowed_local_icmpv6)/sizeof(int); n++) {
			fprintf(fp_ipv6, "-A INPUT -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_local_icmpv6[n], logaccept);
		}

		// default policy: DROP
		// if logging
		fprintf(fp_ipv6, "-A INPUT -j %s\n", logdrop);

#ifdef RTCONFIG_IGD2
		if (nvram_match("upnp_enable", "1") && nvram_match("upnp_pinhole_enable", "1")) {
			fprintf(fp_ipv6, "-A FORWARD -j UPNP\n");
		}
#endif

		// IPv6 firewall - allowed traffic
		if (nvram_match("ipv6_fw_enable", "1")) {
			nvp = nv = strdup(nvram_safe_get("ipv6_fw_rulelist"));
			while (nv && (b = strsep(&nvp, "<")) != NULL) {
				char *portv, *portp, *port, *desc, *dstports;
				char srciprule[64], dstiprule[64];
				if ((vstrsep(b, ">", &desc, &srcip, &dstip, &port, &proto) != 5))
					continue;
				if (srcip[0] != '\0')
					snprintf(srciprule, sizeof(srciprule), "-s %s", srcip);
				else
					srciprule[0] = '\0';
				if (strlen(dstip) > 2) {
					if (dstip[0] == ':' && dstip[1] == ':' && dstip[2] != '\0' && dstip[2] != '/') // dstip is EUI64 address
						snprintf(dstiprule, sizeof(dstiprule), "-d %s/::ffff:ffff:ffff:ffff", dstip);
					else
						snprintf(dstiprule, sizeof(dstiprule), "-d %s", dstip);
				} else {
					dstiprule[0] = '\0';
				}
				portp = portv = strdup(port);
				while (portv && (dstports = strsep(&portp, ",")) != NULL) {
					if (strcmp(proto, "TCP") == 0 || strcmp(proto, "BOTH") == 0)
						fprintf(fp_ipv6, "-A FORWARD -m state --state NEW -p tcp -m tcp %s %s --dport %s -j %s\n", srciprule, dstiprule, dstports, logaccept);
					if (strcmp(proto, "UDP") == 0 || strcmp(proto, "BOTH") == 0)
						fprintf(fp_ipv6, "-A FORWARD -m state --state NEW -p udp -m udp %s %s --dport %s -j %s\n", srciprule, dstiprule, dstports, logaccept);
					// Handle raw protocol in port field, no val1:val2 allowed
					if (strcmp(proto, "OTHER") == 0) {
						protono = strsep(&dstports, ":");
						fprintf(fp_ipv6, "-A FORWARD -p %s %s %s -j %s\n", protono, srciprule, dstiprule, logaccept);
					}
				}
				free(portv);
			}
		}
	}
#endif

	/* DoS protection */
	if (nvram_get_int("fw_enable_x") && nvram_get_int("fw_dos_x"))
		fprintf(fp, "-A FORWARD -i %s -j %s\n", wan_if, "SECURITY");

	// FILTER from LAN to WAN
	// Rules for MAC Filter and LAN to WAN Filter
	// Drop rules always before Accept
	strcpy(chain, "NSFW");  // Less code changes by using a var like the original code did
#ifdef RTCONFIG_PARENTALCTRL
	if(nvram_get_int("MULTIFILTER_ALL") != 0 && pc_count > 0)
		fprintf(fp, "-A PControls -j %s\n", chain);
#endif
	// chain used for other things like pptp filtering, so always jump to it
	fprintf(fp, "-A FORWARD -j %s\n", chain);

	/*
	network service filter
	*/
	if (nvram_match("fw_lw_enable_x", "1"))
	{
		char lanwan_timematch[2048];
		char lanwan_buf[2048];
		char ptr[32], *icmplist;
		char *ftype, *dtype;
		char protoptr[16], flagptr[16];
		char srcipbuf[32], dstipbuf[32];
		int apply;
		char *p, *g;

		memset(lanwan_timematch, 0, sizeof(lanwan_timematch));
		memset(lanwan_buf, 0, sizeof(lanwan_buf));
		apply = timematch_conv2(lanwan_timematch, sizeof(lanwan_timematch), "filter_lw_date_x", "filter_lw_time_x", "filter_lw_time2_x");

		if (nvram_match("filter_lw_default_x", "DROP"))	// Whitelist
		{
			dtype = logdrop;
			ftype = "RETURN";
		}
		else	// Blacklist
		{
			dtype = "RETURN";
			ftype = logdrop;
		}

		if(apply) {
			v4v6_ok = IPT_V4;

			// LAN/WAN filter
			nv = nvp = strdup(nvram_safe_get("filter_lwlist"));

			if(nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if((vstrsep(b, ">", &srcip, &srcport, &dstip, &dstport, &proto) !=5 )) continue;
					(void)protoflag_conv(proto, protoptr, 0);
					(void)protoflag_conv(proto, flagptr, 1);
					g_buf_init(); // need to modified

					setting = filter_conv(protoptr, flagptr, iprange_ex_conv(srcip, srcipbuf), srcport, iprange_ex_conv(dstip, dstipbuf), dstport);
					if (srcip) v4v6_ok = ipt_addr_compact(srcipbuf, v4v6_ok, (v4v6_ok == IPT_V4));
					if (dstip) v4v6_ok = ipt_addr_compact(dstipbuf, v4v6_ok, (v4v6_ok == IPT_V4));

					/* separate lanwan timematch */
					strcpy(lanwan_buf, lanwan_timematch);
					p = lanwan_buf;
					while(p){
						if((g=strsep(&p, ">")) != NULL){
							//cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
							if (v4v6_ok & IPT_V4)
							fprintf(fp, "-A %s %s -i %s -o %s %s -j %s\n", chain, g, lan_if, wan_if, setting, ftype);
#ifdef RTCONFIG_IPV6
							if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
							fprintf(fp_ipv6, "-A %s %s -i %s -o %s %s -j %s\n", chain, g, lan_if, wan6face, setting, ftype);
#endif
						}
					}
				}
				if(nv) free(nv);
			}
		}
		// ICMP
		foreach(ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
		{
			/* separate lanwan timematch */
			strcpy(lanwan_buf, lanwan_timematch);
			p = lanwan_buf;
			while(p){
				if((g=strsep(&p, ">")) != NULL){
					//cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
					fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, g, lan_if, wan_if, ptr, ftype);
#ifdef RTCONFIG_IPV6
					if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
					fprintf(fp_ipv6, "-A %s %s -i %s -o %s %s -j %s\n", chain, g, lan_if, wan6face, setting, ftype);
#endif
				}
			}
		}

		// Default
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan_if, dtype);
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && *wan6face)
		fprintf(fp_ipv6, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan6face, dtype);
#endif
	}

#ifdef RTCONFIG_INTERNETCTRL
#ifdef RTCONFIG_PC_SCHED_V3
	// MAC address in list and in time period -> ACCEPT.
	fprintf(fp, "-A PControls -j %s\n", logdrop);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
		fprintf(fp_ipv6, "-A PControls -j %s\n", logdrop);
#endif
#else
	// MAC address in list and in time period -> ACCEPT.
	fprintf(fp, "-A ICAccept -j %s\n", logaccept);
	fprintf(fp, "-A ICDrop -j %s\n", logdrop);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		fprintf(fp_ipv6, "-A ICAccept -j %s\n", logaccept);
		fprintf(fp_ipv6, "-A ICDrop -j %s\n", logdrop);
	}
#endif
#endif

#ifdef RTCONFIG_PARENTALCTRL
	// MAC address in list and in time period -> ACCEPT.
	fprintf(fp, "-A PControls -j %s\n", logaccept);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
		fprintf(fp_ipv6, "-A PControls -j %s\n", logaccept);
#endif
#endif
#endif

	// Block VPN traffic
	if (nvram_match("fw_pt_pptp", "0")) {
		fprintf(fp, "-I %s -i %s -o %s -p tcp --dport %d -j %s\n", chain, lan_if, wan_if, 1723, "DROP");
		fprintf(fp, "-I %s -i %s -o %s -p 47 -j %s\n", chain, lan_if, wan_if, "DROP");
	}
	if (nvram_match("fw_pt_l2tp", "0"))
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 1701, "DROP");
	if (nvram_match("fw_pt_ipsec", "0")) {
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 500, "DROP");
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 4500, "DROP");
		fprintf(fp, "-I %s -i %s -o %s -p 50 -j %s\n", chain, lan_if, wan_if, "DROP");
		fprintf(fp, "-I %s -i %s -o %s -p 51 -j %s\n", chain, lan_if, wan_if, "DROP");
	}

	//Filter chilli
#ifdef RTCONFIG_CAPTIVE_PORTAL
	if (nvram_match("chilli_enable", "1") || nvram_match("hotss_enable", "1"))
	{
		fprintf(fp, "-I INPUT -i tun22 -m state --state NEW -j ACCEPT\n");
		fprintf(fp, "-I FORWARD -i tun22 -m state --state NEW -j ACCEPT\n");
		fprintf(fp, "-I FORWARD -i tun22 -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
	}
	if (nvram_match("cp_enable", "1") || nvram_match("hotss_enable", "1"))
	{
		fprintf(fp, "-I INPUT -i tun23 -m state --state NEW -j ACCEPT\n");
		fprintf(fp, "-I FORWARD -i tun23 -m state --state NEW -j ACCEPT\n");
		fprintf(fp, "-I FORWARD -i tun23 -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
	}
#endif

	// Filter from WAN to LAN
#if 0 /* Never used */
	if (nvram_match("fw_wl_enable_x", "1"))
	{
		char wanlan_timematch[128];
		char ptr[32], *icmplist;
		char /**dtype,*/ *ftype, *flag;
		int apply;

		apply = timematch_conv(wanlan_timematch, "filter_wl_date_x", "filter_wl_time_x");

		if (nvram_match("filter_wl_default_x", "DROP"))
		{
//			dtype = logdrop;
			ftype = logaccept;
		}
		else
		{
//			dtype = logaccept;
			ftype = logdrop;
		}

		if(apply) {
			v4v6_ok = IPT_V4;

			// WAN/LAN filter
			nv = nvp = strdup(nvram_safe_get("filter_wllist"));

			if(nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if ((vstrsep(b, ">", &proto, &flag, &srcip, &srcport, &dstip, &dstport) != 6)) continue;

					setting=filter_conv(proto, flag, srcip, srcport, dstip, dstport);
					if (srcip) v4v6_ok = ipt_addr_compact(srcip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (dstip) v4v6_ok = ipt_addr_compact(dstip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (v4v6_ok & IPT_V4)
		 			fprintf(fp, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan_if, lan_if, setting, ftype);
#ifdef RTCONFIG_IPV6
					if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
					fprintf(fp_ipv6, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan6face, lan_if, setting, ftype);
#endif
				}
				if(nv) free(nv);
			}
		}

		// ICMP
		foreach(ptr, nvram_safe_get("filter_wl_icmp_x"), icmplist)
		{
			fprintf(fp, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan_if, lan_if, ptr, ftype);
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled() && *wan6face)
			fprintf(fp_ipv6, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan6face, lan_if, ptr, ftype);
#endif
		}

		// thanks for Oleg
		// Default
		// fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, dtype);
	}
#endif

TRACE_PT("write porttrigger\n");

	/* Trigger port setting */
	if (is_nat_enabled() && nvram_match("autofw_enable_x", "1"))
		write_porttrigger(fp, wan_if, 0);

#if 0
	if (is_nat_enabled() && !nvram_match("sp_battle_ips", "0"))
	{
		fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n", BASEPORT, logaccept);	// oleg patch
	}
#endif

	// Allow from LAN
	if (nvram_get_int("fw_enable_x"))
		fprintf(fp, "-A FORWARD -i %s -j %s\n", lan_if, logaccept);

	/* Enable Virtual Servers
	 * Accepts all DNATed connection, including VSERVER, UPNP, BATTLEIPs, etc
	 * Don't bother to do explicit dst checking, 'coz it's done in PREROUTING */
	if (is_nat_enabled())
		fprintf(fp, "-A FORWARD -m conntrack --ctstate DNAT -j %s\n", logaccept);

#if 0 /* Never used */
TRACE_PT("write wl filter\n");

	if (nvram_match("fw_wl_enable_x", "1")) // Thanks for Oleg
	{
		// Default
		fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if,
			nvram_match("filter_wl_default_x", "DROP") ? logdrop : logaccept);
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && *wan6face)
		fprintf(fp_ipv6, "-A FORWARD -i %s -o %s -j %s\n", wan6face, lan_if,
			nvram_match("filter_wl_default_x", "DROP") ? logdrop : logaccept);
#endif
	}
#endif

#ifdef RTCONFIG_OPENVPN
	fprintf(fp, "-A FORWARD -m state --state NEW -j OVPN\n");
#endif
	/* SECURITY chain */
	/* Skip DMZ */
	if ((dstip = nvram_safe_get("dmz_ip")) && *dstip && inet_addr_(dstip))
		fprintf(fp, "-A SECURITY -d %s -j %s\n", dstip, "RETURN");
#if defined(RTCONFIG_MULTIWAN_CFG)
	/* FIXME */
	if (get_nr_wan_unit() == 2 && nvram_match("wans_mode", "lb")) {
		if ((dstip = nvram_safe_get("dmz_ip")) && *dstip && inet_addr_(dstip))
			fprintf(fp, "-A SECURITY -d %s -j %s\n", dstip, "RETURN");
	}
#endif
	/* Sync-flood protection */
	fprintf(fp, "-A SECURITY -p tcp --syn -m limit --limit 1/s -j %s\n", "RETURN");
	fprintf(fp, "-A SECURITY -p tcp --syn -j %s\n", logdrop);
	/* Furtive port scanner */
	fprintf(fp, "-A SECURITY -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", "RETURN");
	fprintf(fp, "-A SECURITY -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j %s\n", logdrop);
	/* UDP flooding */
//	fprintf(fp, "-A SECURITY -p udp -m limit --limit 5/s -j %s\n", "RETURN");
//	fprintf(fp, "-A SECURITY -p udp -j %s\n", logdrop);
	/* Ping of death */
	fprintf(fp, "-A SECURITY -p icmp --icmp-type 8 -m limit --limit 1/s -j %s\n", "RETURN");
	fprintf(fp, "-A SECURITY -p icmp --icmp-type 8 -j %s\n", logdrop);
	/* Skip the rest */
	fprintf(fp, "-A SECURITY -j RETURN\n");

	// logaccept chain
	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	fprintf(fp_ipv6, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");
#endif

	// logdrop chain
	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	fprintf(fp_ipv6, "-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");
#endif

#ifdef RTCONFIG_IPV6
	write_UrlFilter("FORWARD", lan_if, lan_ip, logdrop, fp, fp_ipv6);
#else
	write_UrlFilter("FORWARD", lan_if, lan_ip, logdrop, fp);
#endif

#ifdef CONTENTFILTER
	if (makeTimestr_content(timef, sizeof(timef))) {
		nv = nvp = strdup(nvram_safe_get("keyword_rulelist"));
		while(nvp && (b = strsep(&nvp, "<")) != NULL)
		{
			if(vstrsep(b, ">", &filterstr) != 1)
				continue;
			if(*filterstr)
			{
#ifndef HND_ROUTER
				fprintf(fp, "-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j REJECT --reject-with tcp-reset\n", timef, filterstr);
#else
				fprintf(fp, "-I FORWARD -p tcp --sport 80 %s -m string --string %s --algo bm -j REJECT --reject-with tcp-reset\n",timef, filterstr);
#endif

#ifdef RTCONFIG_IPV6
				if (ipv6_enabled())
#ifndef HND_ROUTER
					fprintf(fp_ipv6, "-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j REJECT --reject-with tcp-reset\n", timef, filterstr);
#else
					fprintf(fp_ipv6, "-I FORWARD -p tcp --sport 80 %s -m string --string %s --algo bm -j REJECT --reject-with tcp-reset\n", timef, filterstr);
#endif
#endif
			}
		}
		if(nv) free(nv);
	}
#endif

#if defined(WEB_REDIRECT)
	/* If NAT is enabled and defpsk is not enabled.
	 * Block ICMP echo-request and all TCP ports if DUT hasn't been configured.
	 */
	if (is_nat_enabled() && nvram_match("x_Setting", "0") &&
			!find_word(nvram_safe_get("rc_support"), "defpsk"))
	{
		fprintf(fp, "-I FORWARD -j default_block\n");
	}
#endif

#ifdef RTCONFIG_TOR
	 // for TOR users, block anything that cannot be routed through the TOR network (i.e. UDP and ICMP ping)
	 // otherwise, it's a leak that could reveal your router's IP address
	 if(nvram_match("Tor_enable", "1")){
		nv = strdup(nvram_safe_get("Tor_redir_list"));
		if (strlen(nv) == 0) {
		fprintf(fp, "-I FORWARD -i %s -j DROP\n", lan_if);
		}
		else{
			while ((b = strsep(&nv, "<")) != NULL) {
				if (strlen(b)==0) continue;
				memset(addr_new, 0, sizeof(addr_new));
				addr_type = addr_type_parse(b, addr_new, sizeof(addr_new));
				if (addr_type == TYPE_IP){
					fprintf(fp, "-I FORWARD -i %s -s %s -j DROP\n", lan_if, addr_new);
				}
				else if (addr_type == TYPE_MAC){
					fprintf(fp, "-I FORWARD -i %s -m mac --mac-source %s -j DROP\n", lan_if, addr_new);
				}
				else if (addr_type == TYPE_IPRANGE){
					fprintf(fp, "-I FORWARD -i %s -m iprange --src-range %s -j DROP\n", lan_if, addr_new);
				}
			}
			free(nv);
		}
	}
#endif

#ifdef RTCONFIG_DNSFILTER
	dnsfilter_dot_rules(fp, lan_if);
#endif

	// Default rule
	if (nvram_get_int("fw_enable_x"))
		fprintf(fp, "-A FORWARD -j %s\n", logdrop);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && nvram_match("ipv6_fw_enable", "1"))
		fprintf(fp_ipv6, "-A FORWARD -j %s\n", logdrop);
#endif

	// extra filter
	//write_extra_filter(fp);

	fprintf(fp, "COMMIT\n\n");
	if (fp) fclose(fp);
	evalRet = eval("iptables-restore", "/tmp/filter_rules");
	rule_apply_checking("firewall", __LINE__, "/tmp/filter_rules", evalRet);

#ifdef RTCONFIG_PROTECTION_SERVER
	kill_pidfile_s(PROTECT_SRV_PID_PATH, SIGUSR1);
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
		// extra filter
		//write_extra_filter6(fp_ipv6);

		fprintf(fp_ipv6, "COMMIT\n\n");
		if (fp_ipv6) fclose(fp_ipv6);
		evalRet = eval("ip6tables-restore", "/tmp/filter_rules_ipv6");
		rule_apply_checking("firewall", __LINE__, "/tmp/filter_rules_ipv6", evalRet);
	}
#endif
}

#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV) // RTCONFIG_DUALWAN || RTCONFIG_MULTICAST_IPTV
void
filter_setting2(char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	FILE *fp;	// oleg patch
#ifdef RTCONFIG_IPV6
	FILE *fp_ipv6 = NULL;
	char *protono;
#endif
	char *proto, *srcip, *srcport, *dstip, *dstport;
	char *nv, *nvp, *b;
	char *setting;
	char chain[32];
#ifdef RTCONFIG_PC_SCHED_V3
	char macdrop[32];
#else
	char macaccept[32];
#endif
	char *wan_proto = "";
	int i;
//2008.09 magic{
#ifdef WEBSTRFILTER
	char timef[256], *filterstr;
#endif
//2008.09 magic}
	char *wan_if, *wan_ip;
	char *wanx_if, *wanx_ip;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
#ifdef RTCONFIG_IPV6
	int n;
	char *ip;
#endif
	int v4v6_ok = IPT_V4;
	int wan_max_unit = WAN_UNIT_MAX;
	int evalRet;

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6)
		wan_max_unit = WAN_UNIT_MULTICAST_IPTV_MAX;
#endif

	if ((fp=fopen("/tmp/filter_rules", "w"))==NULL) return;
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		if ((fp_ipv6 = fopen("/tmp/filter_rules_ipv6", "w"))==NULL) {
			fclose(fp);
			return;
		}
	}
#endif

	fprintf(fp, "*filter\n"
	    ":INPUT ACCEPT [0:0]\n"
	    ":FORWARD ACCEPT [0:0]\n"
	    ":OUTPUT ACCEPT [0:0]\n"
	    ":INPUT_PING - [0:0]\n"
	    ":INPUT_ICMP - [0:0]\n"
	    ":FUPNP - [0:0]\n"
	    ":SECURITY - [0:0]\n"
	    ":ACCESS_RESTRICTION - [0:0]\n"
#ifdef RTCONFIG_INTERNETCTRL
	    ":ICAccept - [0:0]\n"
	    ":ICDrop - [0:0]\n"
#endif
#ifdef RTCONFIG_OPENVPN
	    ":OVPN - [0:0]\n"
#endif
#ifdef RTCONFIG_DNSFILTER
	   ":DNSFILTER_DOT - [0:0]\n"
#endif
	   ":other2wan - [0:0]\n"
#ifdef RTCONFIG_PARENTALCTRL
	    ":PControls - [0:0]\n"
#endif
#if defined(WEB_REDIRECT)
	    ":default_block - [0:0]\n"
#endif

	    ":logaccept - [0:0]\n"
	    ":logdrop - [0:0]\n");

#ifdef RTCONFIG_PROTECTION_SERVER
	fprintf(fp, ":%sWAN - [0:0]\n", PROTECT_SRV_RULE_CHAIN);
	fprintf(fp, ":%sLAN - [0:0]\n", PROTECT_SRV_RULE_CHAIN);
#endif
#ifdef RTCONFIG_GN_WBL
	add_GN_WBL_ChainRule(fp);
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		fprintf(fp_ipv6, "*filter\n"
		    ":INPUT ACCEPT [0:0]\n"
		    ":FORWARD %s [0:0]\n"
		    ":OUTPUT ACCEPT [0:0]\n"
#ifdef RTCONFIG_INTERNETCTRL
		    ":ICAccept - [0:0]\n"
		    ":ICDrop - [0:0]\n"
#endif
#ifdef RTCONFIG_PARENTALCTRL
		    ":PControls - [0:0]\n"
#endif
		    ":logaccept - [0:0]\n"
		    ":logdrop - [0:0]\n",
		nvram_match("ipv6_fw_enable", "1") ? "DROP" : "ACCEPT");
	}
#endif

#ifdef RTCONFIG_PC_SCHED_V3
	strcpy(macdrop, "");
#else
	strcpy(macaccept, "");
#endif

#if defined(WEB_REDIRECT)
	/* Below rules are supposed to be used if below conditions are true
	 * and output interface should be WAN interfaces.
	 * 1. NAT is enabled.
	 * 2. DUT hasn't been configured. (x_Setting = 0)
	 * 3. defpsk is not enabled.
	 */
	if (is_nat_enabled() && nvram_match("x_Setting", "0") &&
			!find_word(nvram_safe_get("rc_support"), "defpsk"))
	{
		int unit;
		char prefix[sizeof("wanXXX_")], wan_iface[IFNAMSIZ];

		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			strlcpy(wan_iface, nvram_pf_get(prefix, "ifname"), sizeof(wan_iface));
			if (*wan_iface == '\0')
				continue;

			/* Block all TCP ports, except 80 and 443. */
			fprintf(fp,
				"-A default_block -o %s -p tcp --dport 80 -j %s\n"
				"-A default_block -o %s -p tcp --dport 443 -j %s\n"
				"-A default_block -o %s -p tcp -j %s\n",
				wan_iface, logaccept,
				wan_iface, logaccept,
				wan_iface, logdrop);
			/* Block ICMP echo request. */
			fprintf(fp,
				"-A default_block -o %s -p icmp ! --icmp-type echo-request -j %s\n"
				"-A default_block -o %s -p icmp -j %s\n",
				wan_iface, logaccept,
				wan_iface, logdrop);
		}
	}
#endif

#ifdef RTCONFIG_INTERNETCTRL
	ic_s *ic_list = NULL;
	int ic_count;

	get_all_ic_list(&ic_list);
	ic_count = count_ic_rules(ic_list);

	if(/*nvram_get_int("MULTIFILTER_ALL") != 0 && */ic_count > 0){
TRACE_PT("writing Internet Control\n");
		config_ic_rule_string(ic_list, fp, logaccept, logdrop, 1);

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()){
			config_ic_rule_string(ic_list, fp_ipv6, logaccept, logdrop, 1);
		}
#endif

		//strcpy(macaccept, "AControls");
	}
	free_pc_list(&ic_list);
#endif

#ifdef RTCONFIG_PARENTALCTRL
	pc_s *pc_list = NULL;
	int pc_count;

	get_all_pc_tmp_list(&pc_list);
	pc_count = count_pc_rules(pc_list, 1);
	 pc_count += count_pc_rules(pc_list, 2);

	if(pc_count > 0){
TRACE_PT("writing temporary Parental Control\n");
		config_pause_block_string(pc_list, fp, logaccept, logdrop, 2);
		config_daytime_string(pc_list, fp, logaccept, logdrop, 1);

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()){
			config_pause_block_string(pc_list, fp_ipv6, logaccept, logdrop, 2);
			config_daytime_string(pc_list, fp_ipv6, logaccept, logdrop, 1);
			config_daytime_string(pc_list, fp_ipv6, logaccept, logdrop, 1);
		}
#endif

#ifdef RTCONFIG_PC_SCHED_V3
		strcpy(macdrop, "PControls");
#else
		strcpy(macaccept, "PControls");
#endif
	}
	free_pc_list(&pc_list);

	get_all_pc_list(&pc_list);
	pc_count = count_pc_rules(pc_list, 1);
	pc_count += count_pc_rules(pc_list, 2);

	if(nvram_get_int("MULTIFILTER_ALL") != 0 && pc_count > 0){
TRACE_PT("writing Parental Control\n");
		config_pause_block_string(pc_list, fp, logaccept, logdrop, 0);
		config_daytime_string(pc_list, fp, logaccept, logdrop, 0);

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()){
			config_pause_block_string(pc_list, fp_ipv6, logaccept, logdrop, 0);
			config_daytime_string(pc_list, fp_ipv6, logaccept, logdrop, 0);
		}
#endif

#ifdef RTCONFIG_PC_SCHED_V3
		strcpy(macdrop, "PControls");
#else
		strcpy(macaccept, "PControls");
#endif
	}
	free_pc_list(&pc_list);
#endif

#ifdef RTCONFIG_RESTRICT_GUI
	char word[PATH_MAX], *next_word;

	if(nvram_get_int("fw_restrict_gui")){
		foreach(word, nvram_safe_get("wl_ifnames"), next_word){
			SKIP_ABSENT_FAKE_IFACE(word);
			eval("ebtables", "-t", "broute", "-D", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "ACCEPT");
			eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "ACCEPT");
		}

		fprintf(fp, "-A INPUT -i %s -m mark --mark %s -d %s -p tcp -m multiport --dport 23,%d,%d,9999 -j DROP\n",
				lan_if, BIT_RES_GUI, lan_ip,
				nvram_get_int("http_lanport") ? : 80,
				nvram_get_int("https_lanport") ? : 443);
	}
#endif

#ifdef RTCONFIG_FREERADIUS
		if (nvram_get_int("radius_serv_enable")) {
			fprintf(fp, "-A INPUT -m conntrack --ctstate DNAT -p udp -d %s --dport %d -j ACCEPT\n", lan_ip, 1812);
		}
#endif

	if (nvram_match("fw_enable_x", "1")) {
		/* Drop ICMP before ESTABLISHED state */
		if (!nvram_get_int("misc_ping_x")) {
			fprintf(fp, "-A %s -p icmp --icmp-type %d -j %s\n", "INPUT", 8, "INPUT_PING");
#ifdef RTCONFIG_IPV6
			if (get_ipv6_service() == IPV6_6IN4) {
				/* accept ICMP requests from the remote tunnel endpoint */
				ip = nvram_safe_get(ipv6_nvname("ipv6_tun_v4end"));
				if (ip && *ip && inet_addr_(ip) != INADDR_ANY)
					fprintf(fp, "-A %s -s %s -p icmp -j %s\n", "INPUT_PING", ip, logaccept);
				/* accept ICMP requests from he.net checker */
				fprintf(fp, "-A %s -s %s -p icmp -j %s\n", "INPUT_PING", "66.220.2.74", logaccept);
			}
#endif
#ifdef RTCONFIG_IPSEC
			if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
				)
				fprintf(fp, "-A %s -p icmp -m policy --dir in --pol ipsec -j %s\n", "INPUT_PING", "RETURN");
#endif
			for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; unit++) {
				if (!is_wan_connect(unit))
					continue;
				wan_if = get_wan_ifname(unit);
				fprintf(fp, "-A %s -i %s -p icmp -j %s\n", "INPUT_PING", wan_if, logdrop);
			}
		}

#ifdef RTCONFIG_IPSEC
		/* Accept stateless IPSec tunnel */
		if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
			) {
			int isakmp_port = nvram_get_int("ipsec_isakmp_port") ? : 500;
			int nat_t_port = nvram_get_int("ipsec_nat_t_port") ? : 4500;

			for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
				if(!is_wan_connect(unit))
					continue;

				wan_if = get_wan_ifname(unit);

				fprintf(fp, "-A INPUT -i %s -p udp -m multiport --dport %d,%d -j %s\n", wan_if, isakmp_port, nat_t_port, "ACCEPT");
				fprintf(fp, "-A INPUT -i %s -p esp -j %s\n", wan_if, "ACCEPT");
				fprintf(fp, "-A INPUT -i %s -p ah -j %s\n", wan_if, "ACCEPT");
			}
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled()) {
				fprintf(fp_ipv6, "-A INPUT -i %s -p udp -m multiport --dport %d,%d -j %s\n", "any", isakmp_port, nat_t_port, "ACCEPT");
				fprintf(fp_ipv6, "-A INPUT -i %s -p esp -j %s\n", "any", "ACCEPT");
			}
#endif
		}
#endif

#ifdef RTCONFIG_IPV6
		write_UrlFilter("INPUT", lan_if, lan_ip, logdrop, fp, fp_ipv6);
#else
		write_UrlFilter("INPUT", lan_if, lan_ip, logdrop, fp);
#endif

		/* Filter known SPI state */
		fprintf(fp, "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n", logaccept);
		fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n", logdrop);

#ifdef RTCONFIG_TAGGED_BASED_VLAN
		/* Deny none brX to access brX subnet */
		vlan_subnet_deny_input(fp);
#endif

		/* Specific IP access restriction */
		write_access_restriction(fp);

#ifdef RTCONFIG_PROTECTION_SERVER
		if (nvram_get_int("telnetd_enable") != 0
#ifdef RTCONFIG_SSH
		    || nvram_get_int("sshd_enable") != 0
#endif
   		) {
			fprintf(fp, "-A INPUT ! -i %s -j %sWAN\n", lan_if, PROTECT_SRV_RULE_CHAIN);
			fprintf(fp, "-A INPUT -i %s -j %sLAN\n", lan_if, PROTECT_SRV_RULE_CHAIN);
		}
#endif
		fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n", lan_if, "ACCEPT");
#ifdef RTCONFIG_IPSEC
		if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
			)
			fprintf(fp, "-A INPUT -m policy --dir in --pol ipsec -m state --state NEW -j %s\n", "ACCEPT");
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
		if (nvram_get_int("pptpd_enable"))
			fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n", "pptp+", "ACCEPT");
#endif
		fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n", "lo", "ACCEPT");
#ifdef RTCONFIG_OPENVPN
		fprintf(fp, "-A INPUT -m state --state NEW -j OVPN\n");
#endif
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()) {
			fprintf(fp_ipv6, "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n", logaccept);
			fprintf(fp_ipv6, "-A INPUT -i %s -m state --state NEW -j %s\n", lan_if, "ACCEPT");
			fprintf(fp_ipv6, "-A INPUT -i %s -m state --state NEW -j %s\n", "lo", "ACCEPT");
			fprintf(fp_ipv6, "-A INPUT -m state --state INVALID -j %s\n", logdrop);
		}
#endif

		/* Pass multicast */
		if (nvram_get_int("mr_enable_x") || nvram_get_int("udpxy_enable_x")) {
			fprintf(fp, "-A INPUT -p 2 -d 224.0.0.0/4 -j %s\n", logaccept);
			fprintf(fp, "-A INPUT -p udp -d 224.0.0.0/4 ! --dport 1900 -j %s\n", logaccept);
		}
#ifdef RTCONFIG_IPV6
		/* not necesary for now, udpxy is ipv4-only
		if (ipv6_enabled() && (nvram_get_int("mr_enable_x") & 2)) {
			fprintf(fp_ipv6, "-A INPUT -p udp -d ff00::/8 ! --dport 1900 -j %s\n", logaccept);
		}
		*/
#endif

//#ifdef RTCONFIG_MULTICAST_IPTV
		if (nvram_get_int("switch_stb_x") > 6) {
			char *movistar_wan;
			if (nvram_invmatch("wan0_pppoe_ifname", ""))
				movistar_wan = nvram_safe_get("wan0_pppoe_ifname");
			else
				movistar_wan = nvram_safe_get("wan0_ifname");
			if (nvram_match("switch_wantag", "movistar")) {
				fprintf(fp, "-A INPUT -i %s -s 10.0.0.0/255.0.0.0 -j DROP\n", movistar_wan);
				fprintf(fp, "-A INPUT -i %s -s 172.16.0.0/255.255.15.0 -j DROP\n", movistar_wan);
				fprintf(fp, "-A INPUT -i vlan2 -s 172.16.0.0/255.255.15.0 -j ACCEPT\n");
				fprintf(fp, "-A INPUT -i vlan3 -s 10.31.255.134/255.255.255.255 -j ACCEPT\n");
				fprintf(fp, "-A INPUT -p udp --dport 520 -j %s\n", "ACCEPT");
			}
		}
//#endif


		/* enable incoming packets from broken dhcp servers, which are sending replies
		 * from addresses other than used for query, this could lead to lower level
		 * of security, but it does not work otherwise (conntrack does not work) :-(
		 */
		for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
			if(!is_wan_connect(unit))
				continue;

			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
			wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));

			if (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_ip, "0.0.0.0") == 0 ||
			    nvram_get_int(strcat_r(prefix, "dhcpenable_x", tmp))) {
				fprintf(fp, "-A INPUT -p udp --sport 67 --dport 68 -j %s\n", logaccept);
			}

			break; // set one time.
		}

		// Firewall between WAN and Local
		if (nvram_get_int("misc_http_x")) {
#ifdef RTCONFIG_HTTPS
			int enable = nvram_get_int("http_enable");
			if (enable != 0) {
				fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n",
					lan_ip, nvram_get_int("https_lanport") ? : 443, logaccept);
			}
			/* do not support http (enable != 1) */
#else
			{
				fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n",
					lan_ip, nvram_get_int("http_lanport") ? : 80, logaccept);
			}
#endif
		}
#ifdef RTCONFIG_SSH
		if (nvram_get_int("sshd_enable") == 1)
		{
			if (nvram_match("sshd_bfp", "1"))
			{
				fprintf(fp, "-N SSHBFP\n");
				fprintf(fp, "-A SSHBFP -m recent --set --name SSH --rsource\n");
				fprintf(fp, "-A SSHBFP -m recent --update --seconds 60 --hitcount 4 --name SSH --rsource -j %s\n", logdrop);
				fprintf(fp, "-A SSHBFP -j %s\n", logaccept);
				fprintf(fp, "-A INPUT -p tcp --dport %d -m state --state NEW -j SSHBFP\n",
					nvram_get_int("sshd_port") ? : 22);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled())
				{
					fprintf(fp_ipv6, "-N SSHBFP\n");
					fprintf(fp_ipv6, "-A SSHBFP -m recent --set --name SSH --rsource\n");
					fprintf(fp_ipv6, "-A SSHBFP -m recent --update --seconds 60 --hitcount 4 --name SSH --rsource -j %s\n", logdrop);
					fprintf(fp_ipv6, "-A SSHBFP -j %s\n", logaccept);
					fprintf(fp_ipv6, "-A INPUT -p tcp --dport %d -m state --state NEW -j SSHBFP\n",
						nvram_get_int("sshd_port") ? : 22);
				}
#endif
			}
                        else
			{
				fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n",
					nvram_get_int("sshd_port") ? : 22, logaccept);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled())
					fprintf(fp_ipv6, "-A INPUT -p tcp --dport %d -j %s\n",
						nvram_get_int("sshd_port") ? : 22, logaccept);
#endif
			}
		}
#endif

#ifdef RTCONFIG_FTP
		if ((nvram_get_int("enable_ftp")) && (nvram_get_int("ftp_wanac")))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport 21 -j %s\n", logaccept);
#ifdef RTCONFIG_IPV6
			if (ipv6_enabled() && nvram_match("ipv6_fw_enable", "1"))
				fprintf(fp_ipv6, "-A INPUT -p tcp -m tcp --dport 21 -j %s\n", logaccept);
#endif
		}
#endif

#ifdef RTCONFIG_WEBDAV
		if (nvram_match("enable_webdav", "1"))
		{
			if(nvram_get_int("st_webdav_mode")!=1) {
				fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j DROP\n", nvram_safe_get("webdav_http_port"));	// oleg patch
			}

			if(nvram_get_int("st_webdav_mode")!=0) {
				fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("webdav_https_port"), logaccept);	// oleg patch
			}
		}
#endif

		/* Pass ICMP */
		if (!nvram_get_int("misc_ping_x")) {
			fprintf(fp, "-A %s -p icmp -j %s\n", "INPUT", "INPUT_ICMP");
			fprintf(fp, "-A %s -p icmp --icmp-type %d -j %s\n", "INPUT_ICMP", 8, "RETURN");
			fprintf(fp, "-A %s -p icmp --icmp-type %d -j %s\n", "INPUT_ICMP", 13, "RETURN");
			fprintf(fp, "-A %s -p icmp -j %s\n", "INPUT_ICMP", logaccept);
		} else
			fprintf(fp, "-A %s -p icmp -j %s\n", "INPUT", logaccept);

		if (!nvram_match("misc_lpr_x", "0"))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 515, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 9100, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 3838, logaccept);	// oleg patch
		}

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
		//Add for pptp server
		if (nvram_get_int("pptpd_enable")) {
			fprintf(fp, "-A INPUT -p tcp --dport %d -j %s\n", 1723, logaccept);
			fprintf(fp, "-A INPUT -p 47 -j %s\n", logaccept);
		}
#endif
#if defined(RTCONFIG_SOC_IPQ8074) || \
    (defined(RTCONFIG_VPNC) || defined(RTCONFIG_VPN_FUSION))
		for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit) {
			if(!is_wan_connect(unit))
				continue;
			enable_gre_for_ecm(unit, fp);
		}
#endif

#ifdef RTCONFIG_IPV6
		switch (get_ipv6_service()) {
		case IPV6_6IN4:
		case IPV6_6TO4:
		case IPV6_6RD:
			fprintf(fp, "-A INPUT -p 41 -j %s\n", "ACCEPT");
			break;
		}
#endif

#ifdef RTCONFIG_PORT_BASED_VLAN
		/* Deny none br0 to access br0 subnet */
		if (vlan_enable() && strlen(nvram_safe_get("subnet_rulelist")))
			fprintf(fp, "-A INPUT ! -i br0 -d %s/%s -j DROP\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

		/* Write input rule for vlan */
		vlan_subnet_filter_input(fp);
#endif

#ifdef RTCONFIG_TAGGED_BASED_VLAN
		/* Write input rule for vlan */
		vlan_subnet_filter_input(fp);
#endif

#ifdef RTCONFIG_TR069
		if (nvram_get_int("tr_enable")) {
			char tr_acs_acl[512] = {0};
			char word[64] = {0};
			char *next = NULL;
			int tr_conn_port = nvram_get_int("tr_conn_port");

			nvram_safe_get_r("tr_acs_acl", tr_acs_acl, sizeof(tr_acs_acl));
			if (strlen(tr_acs_acl)) {
				foreach_44(word, tr_acs_acl, next) {
					fprintf(fp, "-A INPUT -s %s -p tcp -m tcp --dport %d -j %s\n",
						word, tr_conn_port, logaccept);
					fprintf(fp, "-A INPUT -s %s -p udp -m udp --dport %d -j %s\n",
						word, tr_conn_port, logaccept);
				}
			}
			else {
				fprintf(fp, "-A INPUT -p udp -m udp --dport %d -j %s\n",
					nvram_get_int("tr_conn_port"), logaccept);
			}

			if(strlen(nvram_safe_get("tr_acs_url"))) {
				struct addrinfo *res, *ressave;
				int ret;
				ret=url2ip(nvram_safe_get("tr_acs_url"), &res);
				ressave = res;

				if( (ret != -1) && (res!=NULL) && (res->ai_addr !=NULL) )
				{
					do {
						if(res->ai_family == AF_INET)
						if(res->ai_family == AF_INET)
						{
							//ipv4
							char ipv4str[INET_ADDRSTRLEN] = {0};
							if (inet_ntop(AF_INET, &(((struct sockaddr_in *)(res->ai_addr))->sin_addr),ipv4str, INET_ADDRSTRLEN) == NULL)
								perror("inet_ntop"); 
							fprintf(fp, "-A INPUT -s %s -p tcp -m tcp --dport %d -j %s\n", ipv4str, nvram_get_int("tr_conn_port"), logaccept);						
						}
						else if(res->ai_family == AF_INET6)
						{
							char ipv6str[INET6_ADDRSTRLEN];
							if(inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr),ipv6str, INET6_ADDRSTRLEN) == NULL)
								perror("inet_ntop"); 
								//TODO
								//add ipv6 addr to ip6tables
						}

					} while( ( res = res->ai_next ) != NULL );

					freeaddrinfo( ressave );
				}
			}
		}
#endif

#ifdef RTCONFIG_AMAS_WGN
		wgn_filter_input(fp);
#endif
		fprintf(fp, "-A INPUT -j %s\n", logdrop);
	}

/* apps_dm DHT patch */
	if (nvram_match("apps_dl_share", "1"))
	{
		fprintf(fp, "-I INPUT -p udp --dport 6881 -j ACCEPT\n");	// DHT port
		// port range
		fprintf(fp, "-I INPUT -p udp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
		fprintf(fp, "-I INPUT -p tcp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
	}

	/* Pass multicast */
	if (nvram_get_int("mr_enable_x"))
		fprintf(fp, "-A FORWARD -p udp -d 224.0.0.0/4 -j ACCEPT\n");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && (nvram_get_int("mr_enable_x") & 2))
		fprintf(fp_ipv6, "-A FORWARD -p udp -d ff00::/8 -j ACCEPT\n");
#endif

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if (nvram_get_int("jumbo_frame_enable"))
		goto clamp_mss;

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	if (nvram_get_int("pptpd_enable"))
		goto clamp_mss;
#endif
	for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; unit++) {
		if (!is_wan_connect(unit))
			continue;

		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
		if (
#if defined(RTCONFIG_USB_MODEM)
		    dualwan_unit__usbif(unit) ||
#endif
		    strcmp(wan_proto, "pppoe") == 0 ||
		    strcmp(wan_proto, "pptp") == 0 ||
		    strcmp(wan_proto, "l2tp") == 0) {
		clamp_mss:
			fprintf(fp, "-A FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
#ifdef RTCONFIG_PC_SCHED_V3
			if (*macdrop)
				fprintf(fp, "-A %s -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", macdrop);
#else
			if (*macaccept)
				fprintf(fp, "-A %s -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", macaccept);
#endif
			break; // set one time
		}
	}
#ifdef RTCONFIG_IPV6
	switch (get_ipv6_service()) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		if (!nvram_get_int("jumbo_frame_enable") &&
#if defined(RTCONFIG_USB_MODEM)
		    dualwan_unit__nonusbif(wan_primary_ifunit_ipv6()) &&
#endif
		    !(strcmp(wan_proto, "dhcp") != 0 && strcmp(wan_proto, "static") != 0 &&
		      nvram_match(ipv6_nvname("ipv6_ifdev"), "ppp")))
			break;
		/* fall through */
	case IPV6_6IN4:
	case IPV6_6TO4:
	case IPV6_6RD:
		fprintf(fp_ipv6, "-A FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
#ifdef RTCONFIG_PC_SCHED_V3
		if (*macdrop)
			fprintf(fp_ipv6, "-A %s -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", macdrop);
#else
		if (*macaccept)
			fprintf(fp_ipv6, "-A %s -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", macaccept);
#endif
		break;
	}
#endif

	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && nvram_match("ipv6_fw_enable", "1"))
		fprintf(fp_ipv6, "-A FORWARD -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
#endif
#ifdef RTCONFIG_IPSEC
	if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
		)
		fprintf(fp, "-A FORWARD -m policy --dir in --pol ipsec -j %s\n", "ACCEPT");
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	if (nvram_get_int("pptpd_enable"))
		fprintf(fp, "-A FORWARD -i %s -j %s\n", "pptp+", "ACCEPT");
#endif

	for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
		if(!is_wan_connect(unit))
			continue;

		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		wan_if = get_wan_ifname(unit);
		wanx_if = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		wanx_ip = nvram_safe_get(strcat_r(prefix, "xipaddr", tmp));

#ifdef RTCONFIG_TAGGED_BASED_VLAN
	/* Write forward rule for vlan */
	vlan_subnet_filter_forward(fp, wan_if);
#endif

#ifdef RTCONFIG_AMAS_WGN
	wgn_filter_forward(fp);
#endif
// ~ oleg patch
		/* Filter out invalid WAN->WAN connections */


		fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wan_if, lan_if, "other2wan");
#ifdef RTCONFIG_IPV6
		 if (ipv6_enabled() && *wan6face) {
			if (nvram_match("ipv6_fw_enable", "1")) {
				fprintf(fp_ipv6, "-A FORWARD -o %s -i %s -j %s\n", wan6face, lan_if, logaccept);
			} else {	// The default DROP rule from the IPv6 firewall would take care of it
			fprintf(fp_ipv6, "-A FORWARD -o %s ! -i %s -j %s\n", wan6face, lan_if, logdrop);
			}
		}
#endif
		if (strcmp(wanx_if, wan_if) && inet_addr_(wanx_ip) && dualwan_unit__nonusbif(unit))
			fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wanx_if, lan_if, "other2wan");
	}
	fprintf(fp, "-A other2wan -i tun+ -j RETURN\n");	// Let OVPN traffic through
	fprintf(fp, "-A other2wan -j %s\n", logdrop);		// Drop other foreign traffic

#ifdef RTCONFIG_TAGGED_BASED_VLAN
	/* Write forward rule for deny lan */
	vlan_subnet_deny_forward(fp);
#endif
#ifdef RTCONFIG_GN_WBL
	add_GN_WBL_ForwardRule(fp);
#endif

	/* Accept the redirect, might be seen as INVALID, packets */
	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
#ifdef RTCONFIG_PC_SCHED_V3
	if (*macdrop)
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", macdrop, lan_if, lan_if, logdrop);
#else
	if (*macaccept)
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", macaccept, lan_if, lan_if, logaccept);
#endif
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		fprintf(fp_ipv6, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
#ifdef RTCONFIG_PC_SCHED_V3
		if (*macdrop)
			fprintf(fp_ipv6, "-A %s -i %s -o %s -j %s\n", macdrop, lan_if, lan_if, logdrop);
#else
		if (*macaccept)
			fprintf(fp_ipv6, "-A %s -i %s -o %s -j %s\n", macaccept, lan_if, lan_if, logaccept);
#endif
	}
#endif

	allow_sroutes(fp);

	/* Drop the wrong state, INVALID, packets */
	fprintf(fp, "-A FORWARD -m state --state INVALID -j %s\n", logdrop);
#ifdef RTCONFIG_PC_SCHED_V3
	if (*macdrop)
		fprintf(fp, "-A %s -m state --state INVALID -j %s\n", macdrop, logdrop);
#else
	if (*macaccept)
		fprintf(fp, "-A %s -m state --state INVALID -j %s\n", macaccept, logdrop);
#endif
#if 0
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		fprintf(fp_ipv6, "-A FORWARD -m state --state INVALID -j %s\n", logdrop);
		if (*macaccept)
			fprintf(fp_ipv6, "-A %s -m state --state INVALID -j %s\n", macaccept, logdrop);
	}
#endif
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
		fprintf(fp_ipv6, "-A FORWARD -p ipv6-nonxt -m length --length 40 -j ACCEPT\n");

		// ICMPv6 rules
		for (i = 0; i < sizeof(allowed_icmpv6)/sizeof(int); ++i) {
			fprintf(fp_ipv6, "-A FORWARD -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_icmpv6[i], logaccept);
		}
	}
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
#if 0
		fprintf(fp_ipv6,
			/* "-A INPUT -m state --state INVALID -j DROP\n" */
			"-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n",
			logaccept);
#endif
		fprintf(fp_ipv6, "-A INPUT -p ipv6-nonxt -m length --length 40 -j ACCEPT\n");

		fprintf(fp_ipv6,
			"-A INPUT -i %s -j ACCEPT\n" // anything coming from LAN
			"-A INPUT -i lo -j ACCEPT\n",
				lan_if);

		switch (get_ipv6_service()) {
#ifdef RTCONFIG_6RELAYD
		case IPV6_PASSTHROUGH:
			/* allow responses from the dhcpv6 server to relay, not used so far
			fprintf(fp_ipv6, "-A INPUT -p udp --sport 547 --dport 547 -j %s\n", logaccept); */
			/* fall through */
#endif
		case IPV6_NATIVE_DHCP:
			/* allow responses from the dhcpv6 server to client */
			fprintf(fp_ipv6, "-A INPUT -p udp --sport 547 --dport 546 -j %s\n", logaccept);
			break;
		}

		// ICMPv6 rules
		for (n = 0; n < sizeof(allowed_icmpv6)/sizeof(int); n++) {
			fprintf(fp_ipv6, "-A INPUT -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_icmpv6[n], logaccept);
		}
		for (n = 0; n < sizeof(allowed_local_icmpv6)/sizeof(int); n++) {
			fprintf(fp_ipv6, "-A INPUT -p ipv6-icmp --icmpv6-type %i -j %s\n", allowed_local_icmpv6[n], logaccept);
		}

		// default policy: DROP
		// if logging
		fprintf(fp_ipv6, "-A INPUT -j %s\n", logdrop);

		// IPv6 firewall allowed traffic
		if (nvram_match("ipv6_fw_enable", "1")) {
			nvp = nv = strdup(nvram_safe_get("ipv6_fw_rulelist"));
			while (nv && (b = strsep(&nvp, "<")) != NULL) {
				char *portv, *portp, *port, *desc, *dstports;
				char srciprule[64], dstiprule[64];
				if ((vstrsep(b, ">", &desc, &srcip, &dstip, &port, &proto) != 5))
					continue;
				if (srcip[0] != '\0')
					snprintf(srciprule, sizeof(srciprule), "-s %s", srcip);
				else
					srciprule[0] = '\0';
				if (strlen(dstip) > 2) {
					if (dstip[0] == ':' && dstip[1] == ':' && dstip[2] != '\0' && dstip[2] != '/') // dstip is EUI64 address
						snprintf(dstiprule, sizeof(dstiprule), "-d %s/::ffff:ffff:ffff:ffff", dstip);
					else
						snprintf(dstiprule, sizeof(dstiprule), "-d %s", dstip);
				} else {
					dstiprule[0] = '\0';
				}
				portp = portv = strdup(port);
				while (portv && (dstports = strsep(&portp, ",")) != NULL) {
					if (strcmp(proto, "TCP") == 0 || strcmp(proto, "BOTH") == 0)
						fprintf(fp_ipv6, "-A FORWARD -m state --state NEW -p tcp -m tcp %s %s --dport %s -j %s\n",
							srciprule, dstiprule, dstports, logaccept);
					if (strcmp(proto, "UDP") == 0 || strcmp(proto, "BOTH") == 0)
						fprintf(fp_ipv6, "-A FORWARD -m state --state NEW -p udp -m udp %s %s --dport %s -j %s\n",
							srciprule, dstiprule, dstports, logaccept);
					// Handle raw protocol in port field, no val1:val2 allowed
					if (strcmp(proto, "OTHER") == 0) {
						protono = strsep(&dstports, ":");
						fprintf(fp_ipv6, "-A FORWARD -p %s %s %s -j %s\n", protono, srciprule, dstiprule, logaccept);
					}
				}
				free(portv);
			}
		}
	}
#endif

	/* DoS protection */
	if (nvram_get_int("fw_enable_x") && nvram_get_int("fw_dos_x"))
	for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
		if(!is_wan_connect(unit))
			continue;

		wan_if = get_wan_ifname(unit);

		fprintf(fp, "-A FORWARD -i %s -j %s\n", wan_if, "SECURITY");
	}

	// FILTER from LAN to WAN
	// Rules for MAC Filter and LAN to WAN Filter
	// Drop rules always before Accept
#ifdef RTCONFIG_PARENTALCTRL
	if(nvram_get_int("MULTIFILTER_ALL") != 0 && pc_count > 0)
		strcpy(chain, "PControls");
	else
#endif
	strcpy(chain, "FORWARD");

	/*
	network service filter
	*/
	if (nvram_match("fw_lw_enable_x", "1"))
	{
		char lanwan_timematch[2048];
		char lanwan_buf[2048];
		char ptr[32], *icmplist;
		char *ftype, *dtype;
		char protoptr[16], flagptr[16];
		char srcipbuf[32], dstipbuf[32];
		int apply;
		char *p, *g;

		memset(lanwan_timematch, 0, sizeof(lanwan_timematch));
		memset(lanwan_buf, 0, sizeof(lanwan_buf));
		apply = timematch_conv2(lanwan_timematch, sizeof(lanwan_timematch), "filter_lw_date_x", "filter_lw_time_x", "filter_lw_time2_x");

		if (nvram_match("filter_lw_default_x", "DROP"))
		{
			dtype = logdrop;
			ftype = logaccept;

		}
		else
		{
			dtype = logaccept;
			ftype = logdrop;
		}

		if(apply) {
			v4v6_ok = IPT_V4;

			// LAN/WAN filter
			nv = nvp = strdup(nvram_safe_get("filter_lwlist"));

			if(nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if((vstrsep(b, ">", &srcip, &srcport, &dstip, &dstport, &proto) !=5 )) continue;
					(void)protoflag_conv(proto, protoptr, 0);
					(void)protoflag_conv(proto, flagptr, 1);
					g_buf_init(); // need to modified

					setting = filter_conv(protoptr, flagptr, iprange_ex_conv(srcip, srcipbuf), srcport, iprange_ex_conv(dstip, dstipbuf), dstport);
					if (srcip) v4v6_ok = ipt_addr_compact(srcipbuf, v4v6_ok, (v4v6_ok == IPT_V4));
					if (dstip) v4v6_ok = ipt_addr_compact(dstipbuf, v4v6_ok, (v4v6_ok == IPT_V4));
					for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
						if(!is_wan_connect(unit))
							continue;

						wan_if = get_wan_ifname(unit);

						/* separate lanwan timematch */
						strcpy(lanwan_buf, lanwan_timematch);
						p = lanwan_buf;
						while(p){
							if((g=strsep(&p, ">")) != NULL){
								//cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
								if (v4v6_ok & IPT_V4)
								fprintf(fp, "-A %s %s -i %s -o %s %s -j %s\n", chain, g, lan_if, wan_if, setting, ftype);
							}
						}
					}

#ifdef RTCONFIG_IPV6
					/* separate lanwan timematch */
					strcpy(lanwan_buf, lanwan_timematch);
					p = lanwan_buf;
					while(p){
						if((g=strsep(&p, ">")) != NULL){
							//cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
							if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
							fprintf(fp_ipv6, "-A %s %s -i %s -o %s %s -j %s\n", chain, g, lan_if, wan6face, setting, ftype);
						}
					}
#endif
				}
				if(nv) free(nv);
			}
		}

		// ICMP
		foreach(ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
		{
			for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
				if(!is_wan_connect(unit))
					continue;

				wan_if = get_wan_ifname(unit);

				/* separate lanwan timematch */
				strcpy(lanwan_buf, lanwan_timematch);
				p = lanwan_buf;
				while(p){
					if((g=strsep(&p, ">")) != NULL){
						//cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
						if (v4v6_ok & IPT_V4)
						fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, g, lan_if, wan_if, ptr, ftype);
					}
				}
			}

#ifdef RTCONFIG_IPV6
			/* separate lanwan timematch */
			strcpy(lanwan_buf, lanwan_timematch);
			p = lanwan_buf;
			while(p){
				if((g=strsep(&p, ">")) != NULL){
					//cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
					if (ipv6_enabled() && *wan6face)
					fprintf(fp_ipv6, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, g, lan_if, wan6face, ptr, ftype);
				}
			}
#endif
		}

		// Default
		for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
			if(!is_wan_connect(unit))
				continue;

			wan_if = get_wan_ifname(unit);

			fprintf(fp, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan_if, dtype);
		}

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && *wan6face)
			fprintf(fp_ipv6, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan6face, dtype);
#endif
	}

#ifdef RTCONFIG_INTERNETCTRL
	// MAC address in list and in time period -> ACCEPT.
	fprintf(fp, "-A ICAccept -j %s\n", logaccept);
	fprintf(fp, "-A ICDrop -j %s\n", logdrop);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		fprintf(fp_ipv6, "-A ICAccept -j %s\n", logaccept);
		fprintf(fp_ipv6, "-A ICDrop -j %s\n", logdrop);
	}
#endif
#endif

#ifdef RTCONFIG_PARENTALCTRL
#ifdef RTCONFIG_PC_SCHED_V3
	// MAC address in list and in time period -> ACCEPT.
	fprintf(fp, "-A PControls -j %s\n", logdrop);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
		fprintf(fp_ipv6, "-A PControls -j %s\n", logdrop);
#endif
#else
	// MAC address in list and in time period -> ACCEPT.
	fprintf(fp, "-A PControls -j %s\n", logaccept);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
		fprintf(fp_ipv6, "-A PControls -j %s\n", logaccept);
#endif
#endif
#endif

	// Block VPN traffic
	for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
		if(!is_wan_connect(unit))
			continue;

		wan_if = get_wan_ifname(unit);

		if (nvram_match("fw_pt_pptp", "0")) {
			fprintf(fp, "-I %s -i %s -o %s -p tcp --dport %d -j %s\n", chain, lan_if, wan_if, 1723, "DROP");
			fprintf(fp, "-I %s -i %s -o %s -p 47 -j %s\n", chain, lan_if, wan_if, "DROP");
		}
		if (nvram_match("fw_pt_l2tp", "0"))
			fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 1701, "DROP");
		if (nvram_match("fw_pt_ipsec", "0")) {
			fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 500, "DROP");
			fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 4500, "DROP");
			fprintf(fp, "-I %s -i %s -o %s -p 50 -j %s\n", chain, lan_if, wan_if, "DROP");
			fprintf(fp, "-I %s -i %s -o %s -p 51 -j %s\n", chain, lan_if, wan_if, "DROP");
		}
	}

	//Filter chilli
#ifdef RTCONFIG_CAPTIVE_PORTAL
	if (nvram_match("chilli_enable", "1") || nvram_match("hotss_enable", "1"))
	{
		fprintf(fp, "-I INPUT -i tun22 -m state --state NEW -j ACCEPT\n");
		fprintf(fp, "-I FORWARD -i tun22 -m state --state NEW -j ACCEPT\n");
		fprintf(fp, "-I FORWARD -i tun22 -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
	}
	if (nvram_match("cp_enable", "1") || nvram_match("hotss_enable", "1"))
	{
		fprintf(fp, "-I INPUT -i tun23 -m state --state NEW -j ACCEPT\n");
		fprintf(fp, "-I FORWARD -i tun23 -m state --state NEW -j ACCEPT\n");
		fprintf(fp, "-I FORWARD -i tun23 -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
	}
#endif

	// Filter from WAN to LAN
#if 0 /* Never used */
	if (nvram_match("fw_wl_enable_x", "1"))
	{
		char wanlan_timematch[128];
		char ptr[32], *icmplist;
		char /**dtype,*/ *ftype, *flag;
		int apply;

		apply = timematch_conv(wanlan_timematch, "filter_wl_date_x", "filter_wl_time_x");

		if (nvram_match("filter_wl_default_x", "DROP"))
		{
//			dtype = logdrop;
			ftype = logaccept;
		}
		else
		{
//			dtype = logaccept;
			ftype = logdrop;
		}

		if(apply) {
			v4v6_ok = IPT_V4;

			// WAN/LAN filter
			nv = nvp = strdup(nvram_safe_get("filter_wllist"));

			if(nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if ((vstrsep(b, ">", &proto, &flag, &srcip, &srcport, &dstip, &dstport) != 6)) continue;

					setting=filter_conv(proto, flag, srcip, srcport, dstip, dstport);
					if (srcip) v4v6_ok = ipt_addr_compact(srcip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (dstip) v4v6_ok = ipt_addr_compact(dstip, v4v6_ok, (v4v6_ok == IPT_V4));
					if (v4v6_ok & IPT_V4)
						for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
							if(!is_wan_connect(unit))
								continue;

							wan_if = get_wan_ifname(unit);

				 			fprintf(fp, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan_if, lan_if, setting, ftype);
			 			}

#ifdef RTCONFIG_IPV6
					if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
						fprintf(fp_ipv6, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan6face, lan_if, setting, ftype);
#endif
				}
				free(nv);
			}
		}

		// ICMP
		foreach(ptr, nvram_safe_get("filter_wl_icmp_x"), icmplist)
		{
			for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
				if(!is_wan_connect(unit))
					continue;

				wan_if = get_wan_ifname(unit);

				fprintf(fp, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan_if, lan_if, ptr, ftype);
			}

#ifdef RTCONFIG_IPV6
			if (ipv6_enabled() && (v4v6_ok & IPT_V6) && *wan6face)
				fprintf(fp_ipv6, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan6face, lan_if, ptr, ftype);
#endif
		}

		// thanks for Oleg
		// Default
		// fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, dtype);
	}
#endif

TRACE_PT("write porttrigger\n");

	/* Trigger port setting */
	if (is_nat_enabled() && nvram_match("autofw_enable_x", "1"))
	for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
		if(!is_wan_connect(unit))
			continue;

		wan_if = get_wan_ifname(unit);
		write_porttrigger(fp, wan_if, 0);
	}

#if 0
	if (is_nat_enabled() && !nvram_match("sp_battle_ips", "0"))
	{
		fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n", BASEPORT, logaccept);	// oleg patch
	}
#endif

	// Allow from LAN
	if (nvram_get_int("fw_enable_x"))
		fprintf(fp, "-A FORWARD -i %s -j %s\n", lan_if, logaccept);

	/* Enable Virtual Servers
	 * Accepts all DNATed connection, including VSERVER, UPNP, BATTLEIPs, etc
	 * Don't bother to do explicit dst checking, 'coz it's done in PREROUTING */
	if (is_nat_enabled())
		fprintf(fp, "-A FORWARD -m conntrack --ctstate DNAT -j %s\n", logaccept);

#if 0 /* Never used */
TRACE_PT("write wl filter\n");

	for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
		if(!is_wan_connect(unit))
			continue;

		wan_if = get_wan_ifname(unit);

		if (nvram_match("fw_wl_enable_x", "1")) // Thanks for Oleg
			// Default
			fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, nvram_match("filter_wl_default_x", "DROP") ? logdrop : logaccept);


#ifdef RTCONFIG_IPV6
		if ((unit == wan_primary_ifunit_ipv6()) &&
			ipv6_enabled() && *wan6face)
			fprintf(fp_ipv6, "-A FORWARD -i %s -o %s -j %s\n", wan6face, lan_if, nvram_match("filter_wl_default_x", "DROP") ? logdrop : logaccept);
#endif
	}
#endif

#ifdef RTCONFIG_OPENVPN
	fprintf(fp, "-A FORWARD -m state --state NEW -j OVPN\n");
#endif

	/* SECURITY chain */
	/* Skip DMZ */
	if ((dstip = nvram_safe_get("dmz_ip")) && *dstip && inet_addr_(dstip))
		fprintf(fp, "-A SECURITY -d %s -j %s\n", dstip, "RETURN");
#if defined(RTCONFIG_MULTIWAN_CFG)
	/* FIXME */
	if (get_nr_wan_unit() == 2 && nvram_match("wans_mode", "lb")) {
		if ((dstip = nvram_safe_get("dmz_ip")) && *dstip && inet_addr_(dstip))
			fprintf(fp, "-A SECURITY -d %s -j %s\n", dstip, "RETURN");
	}
#endif
	/* Sync-flood protection */
	fprintf(fp, "-A SECURITY -p tcp --syn -m limit --limit 1/s -j %s\n", "RETURN");
	fprintf(fp, "-A SECURITY -p tcp --syn -j %s\n", logdrop);
	/* Furtive port scanner */
	fprintf(fp, "-A SECURITY -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", "RETURN");
	fprintf(fp, "-A SECURITY -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j %s\n", logdrop);
	/* UDP flooding */
//	fprintf(fp, "-A SECURITY -p udp -m limit --limit 5/s -j %s\n", "RETURN");
//	fprintf(fp, "-A SECURITY -p udp -j %s\n", logdrop);
	/* Ping of death */
	fprintf(fp, "-A SECURITY -p icmp --icmp-type 8 -m limit --limit 1/s -j %s\n", "RETURN");
	fprintf(fp, "-A SECURITY -p icmp --icmp-type 8 -j %s\n", logdrop);
	/* Skip the rest */
	fprintf(fp, "-A SECURITY -j RETURN\n");

	// logaccept chain
	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	fprintf(fp_ipv6, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");
#endif

	// logdrop chain
	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	fprintf(fp_ipv6, "-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");
#endif

#ifdef RTCONFIG_IPV6
	write_UrlFilter("FORWARD", lan_if, lan_ip, logdrop, fp, fp_ipv6);
#else
	write_UrlFilter("FORWARD", lan_if, lan_ip, logdrop, fp);
#endif

#ifdef CONTENTFILTER
	if (makeTimestr_content(timef, sizeof(timef))) {
		nv = nvp = strdup(nvram_safe_get("keyword_rulelist"));
		while(nvp && (b = strsep(&nvp, "<")) != NULL)
		{
			if(vstrsep(b, ">", &filterstr) != 1)
				continue;
			if(*filterstr)
			{
#ifndef HND_ROUTER
				fprintf(fp, "-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j REJECT --reject-with tcp-reset\n", timef, filterstr);
#else
				fprintf(fp, "-I FORWARD -p tcp --sport 80 %s -m string --string %s --algo bm -j REJECT --reject-with tcp-reset\n", timef, filterstr);
#endif
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled())
#ifndef HND_ROUTER
					fprintf(fp_ipv6, "-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j REJECT --reject-with tcp-reset\n", timef, filterstr);
#else
					fprintf(fp_ipv6, "-I FORWARD -p tcp --sport 80 %s -m string --string %s --algo bm -j REJECT --reject-with tcp-reset\n", timef, filterstr);
#endif
#endif
			}
		}
		if(nv) free(nv);
	}
#endif

#if defined(WEB_REDIRECT)
	/* If NAT is enabled and defpsk is not enabled.
	 * Block ICMP echo-request and all TCP ports if DUT hasn't been configured.
	 */
	if (is_nat_enabled() && nvram_match("x_Setting", "0") &&
			!find_word(nvram_safe_get("rc_support"), "defpsk"))
	{
		fprintf(fp, "-I FORWARD -j default_block\n");
	}
#endif

#ifdef RTCONFIG_DNSFILTER
	dnsfilter_dot_rules(fp, lan_if);
#endif

	// Default rule
	if (nvram_get_int("fw_enable_x"))
		fprintf(fp, "-A FORWARD -j %s\n", logdrop);
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && nvram_match("ipv6_fw_enable", "1"))
		fprintf(fp_ipv6, "-A FORWARD -j %s\n", logdrop);
#endif

	// extra filter
	//write_extra_filter(fp);

	fprintf(fp, "COMMIT\n\n");
	if (fp) fclose(fp);
	evalRet = eval("iptables-restore", "/tmp/filter_rules");
	rule_apply_checking("firewall", __LINE__, "/tmp/filter_rules", evalRet);

#ifdef RTCONFIG_PROTECTION_SERVER
	kill_pidfile_s(PROTECT_SRV_PID_PATH, SIGUSR1);
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled())
	{
		// extra filter
		//write_extra_filter6(fp_ipv6);

		fprintf(fp_ipv6, "COMMIT\n\n");
		if (fp_ipv6) fclose(fp_ipv6);
		evalRet = eval("ip6tables-restore", "/tmp/filter_rules_ipv6");
		rule_apply_checking("firewall", __LINE__, "/tmp/filter_rules_ipv6", evalRet);
	}
#endif
}
#endif // RTCONFIG_DUALWAN

void
write_porttrigger(FILE *fp, char *wan_if, int is_nat)
{
	char *nv, *nvp, *b;
	char *out_proto, *in_proto, *out_port, *in_port, *desc;
	char out_protoptr[16], in_protoptr[16];
	char chain[sizeof("triggers_") + IFNAMSIZ];
	int first = 1;

	if (is_nat) {
		fprintf(fp, "-A VSERVER -j TRIGGER --trigger-type dnat\n");
		return;
	}

	nvp = nv = strdup(nvram_safe_get("autofw_rulelist"));
	while (nv && (b = strsep(&nvp, "<")) != NULL) {
		char *p;

		if ((vstrsep(b, ">", &desc, &out_port, &out_proto, &in_port, &in_proto) != 5))
			continue;

		if (first) {
			snprintf(chain, sizeof(chain), "triggers_%s", wan_if);
			fprintf(fp, ":%s - [0:0]\n", chain);
			fprintf(fp, "-A FORWARD -o %s -j %s\n", wan_if, chain);
			fprintf(fp, "-A FORWARD -i %s -j TRIGGER --trigger-type in\n", wan_if);
			first = 0;
		}
		(void)proto_conv(in_proto, in_protoptr);
		(void)proto_conv(out_proto, out_protoptr);

		/* parse_ports() of libipt_TRIGGER.c only accepts '-', not ':'. */
		if ((p = strchr(in_port, ':')) != NULL)
			*p = '-';
		fprintf(fp, "-A %s -p %s -m %s --dport %s "
			"-j TRIGGER --trigger-type out --trigger-proto %s --trigger-match %s --trigger-relate %s\n",
			chain,
			out_protoptr, out_protoptr, out_port,
			in_protoptr, out_port, in_port);
	}
	free(nv);
}

void
mangle_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
#ifdef RTCONFIG_IPV6
	int unit;
	char chain[sizeof("QOSOxXXX")];
#endif
#ifdef CONFIG_BCMWL5 /* the only use so far */
	char lan_class[32];

	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
#endif

	if(IS_NON_AQOS() || IS_ROG_QOS()){
			add_iQosRules(wan_if);
	}
	else {
		eval("iptables", "-t", "mangle", "-F");
#ifdef RTCONFIG_IPV6
		eval("ip6tables", "-t", "mangle", "-F");
#endif
	}

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && IS_TQOS()) {
		/* Create QOSOx chain for TQoS */
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			snprintf(chain, sizeof(chain), "QOSO%d", unit);
			eval("ip6tables", "-t", "mangle", "-N", chain);
		}
	}
#endif

/* Workaround for neighbour solicitation flood from Comcast */
#ifdef RTCONFIG_IPV6
	if (nvram_get_int("ipv6_ns_drop")) {
		eval("ip6tables", "-t", "mangle", "-A", "PREROUTING", "-p", "icmpv6", "--icmpv6-type", "neighbor-solicitation",
		     "-i", wan_if, "-d", "ff02::1:ff00:0/104", "-j", "DROP");
	}
#endif

#ifdef RTCONFIG_FBWIFI
	if(sw_mode() == SW_MODE_ROUTER){
		fbwifi_mangle();
	}
#endif

#if defined(RTAC58U) || defined(RTAC59U) || defined(RTAC88U) || defined(RTAX58U) || defined(RTAX56U)
	if (nvram_match("switch_wantag", "stuff_fibre")) {
		eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-p", "udp", "--dport", "53", "-j", "CLASSIFY", "--set-class", "0:3");
		eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-d", "27.111.14.67", "-j", "CLASSIFY", "--set-class", "0:3");
	}
#endif

#ifdef RTCONFIG_YANDEXDNS
#ifdef RTCONFIG_IPV6
	if (nvram_get_int("yadns_enable_x") && ipv6_enabled()) {
		FILE *fp;

		fp = fopen("/tmp/mangle_rules_ipv6.yadns", "w");
		if (fp != NULL) {
			int evalRet;
			fprintf(fp, "*mangle\n"
			    ":YADNSI - [0:0]\n"
			    ":YADNSF - [0:0]\n");

			write_yandexdns_filter6(fp, lan_if, lan_ip);

			fprintf(fp, "COMMIT\n");
			fclose(fp);

			evalRet = eval("ip6tables-restore", "/tmp/mangle_rules_ipv6.yadns");
			rule_apply_checking("firewall", __LINE__, "/tmp/mangle_rules_ipv6.yadns", evalRet);
		}
	}
#endif /* RTCONFIG_IPV6 */
#endif /* RTCONFIG_YANDEXDNS */

#ifdef RTCONFIG_DNSFILTER
#ifdef RTCONFIG_IPV6
	if (nvram_get_int("dnsfilter_enable_x") && ipv6_enabled()) {
		FILE *fp;

		fp = fopen("/tmp/mangle_rules_ipv6.dnsfilter", "w");
		if (fp != NULL) {
			fprintf(fp, "*mangle\n"
			    ":DNSFILTERI - [0:0]\n"
			    ":DNSFILTERF - [0:0]\n"
			    ":DNSFILTER_DOT - [0:0]\n");

			dnsfilter6_settings(fp, lan_if, lan_ip);

			fprintf(fp, "COMMIT\n");
			fclose(fp);

			eval("ip6tables-restore", "/tmp/mangle_rules_ipv6.dnsfilter");
		}
	}
#endif /* RTCONFIG_IPV6 */
#endif /* RTCONFIG_DNSFILTER */

	/* In Bangladesh, ISPs force the packet TTL as 1 at modem side to block ip sharing.
	 * Increase the TTL once the packet come at WAN with TTL=1 */
	if (nvram_get_int("ttl_inc_enable")) {
		eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_if,
		     "-m", "ttl", "--ttl-eq", "1",
		     "-j", "TTL", "--ttl-set", "64");
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && *wan6face)
			eval("ip6tables", "-t", "mangle", "-A", "PREROUTING", "-i", wan6face,
			     "-m", "hl", "--hl-eq", "1",
			     "-j", "HL", "--hl-set", "64");
#endif
	}

	/* Several ISPs restrict Internet sharing by checking TTL value,
	 * i.e allow Phones/Tablets only. Fix WAN outgoing packets with
	 * router's TTL, default is 64  */
	if (nvram_get_int("ttl_spoof_enable")) {
		char value[sizeof("255")];
		int wan_ttl = 0;

		if (f_read_string("/proc/sys/net/ipv4/ip_default_ttl", value, sizeof(value)) > 0)
			wan_ttl = atoi(value);
		snprintf(value, sizeof(value), "%d", wan_ttl);

		if (wan_ttl > 1) {
			eval("iptables", "-t", "mangle", "-A", "FORWARD", "-o", wan_if,
			    "-m", "ttl", "--ttl-gt", "30",
			    "-m", "ttl", "--ttl-lt", "254",
			    "-j", "TTL", "--ttl-set", value);
		}
		eval("iptables", "-t", "mangle", "-A", "FORWARD", "-o", wan_if,
		    "-m", "ttl", "--ttl-eq", "254",
		    "-j", "TTL", "--ttl-set", "255");
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && *wan6face) {
			int wan_hlim = ipv6_getconf(wan6face, "hop_limit") ? : ipv6_getconf("default", "hop_limit");
			if (wan_hlim > 1) {
				snprintf(value, sizeof(value), "%d", wan_hlim);
				eval("ip6tables", "-t", "mangle", "-A", "FORWARD", "-o", wan6face,
				     "-m", "hl", "--hl-gt", "30",
				     "-m", "hl", "--hl-lt", "254",
				     "-j", "HL", "--hl-set", value);
			}
			eval("ip6tables", "-t", "mangle", "-A", "FORWARD", "-o", wan6face,
			     "-m", "hl", "--hl-eq", "254",
			     "-j", "HL", "--hl-set", "255");
		}
#endif
	}

#ifdef CONFIG_BCMWL5
#ifdef HND_ROUTER
	/* bypass flow cache learning */
	if (nvram_match("fc_disable", "0")) {
#else
	/* mark connect to bypass CTF */
	if (nvram_match("ctf_disable", "0")) {
#endif
#if defined(RTCONFIG_BWDPI)
		/* DPI engine */
		if (IS_AQOS()) {
				eval("iptables", "-t", "mangle", "-N", "BWDPI_FILTER");
				eval("iptables", "-t", "mangle", "-F", "BWDPI_FILTER");
				eval("iptables", "-t", "mangle", "-A", "BWDPI_FILTER", "-i", wan_if, "-p", "udp", "--sport", "68", "--dport", "67", "-j", "DROP");
//				eval("iptables", "-t", "mangle", "-A", "BWDPI_FILTER", "-i", wan_if, "-p", "udp", "--sport", "67", "--dport", "68", "-j", "DROP");
				eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_if, "-p", "udp", "-j", "BWDPI_FILTER");
		}
#endif
		/* mark 80 port connection */
		if (nvram_match("url_enable_x", "1") || nvram_match("keyword_enable_x", "1")) {
#ifdef HND_ROUTER
			if (nvram_match("fc_pt_war", "1"))
#endif
			eval("iptables", "-t", "mangle", "-A", "FORWARD",
			     "-p", "tcp", "--dport", "80",
			     "-m", "state", "--state", "NEW", "-j", "MARK", "--set-mark", "0x01/0x7");
		}

		/* mark VTS loopback connections */
		if (nvram_match("vts_enable_x", "1") || dmz_enabled() ||
			(is_nat_enabled() && nvram_get_int("upnp_enable"))) {
#ifdef HND_ROUTER
			if (nvram_match("fc_pt_war", "1"))
#endif
			eval("iptables", "-t", "mangle", "-A", "FORWARD",
			     "-o", lan_if, "-s", lan_class, "-d", lan_class,
			     "-j", "MARK", "--set-mark", "0x01/0x7");
		}
#ifdef RTCONFIG_BCMARM
		/* mark STUN connection*/
#ifdef HND_ROUTER
		if (nvram_match("fc_pt_udp_war", "1"))
#else
		if (nvram_match("ctf_pt_udp", "1"))
#endif
			eval("iptables", "-t", "mangle", "-A", "FORWARD",
			     "-p", "udp",
			     "-m", "state", "--state", "NEW", "-j", "MARK", "--set-mark", "0x01/0x7");
#endif
#ifdef RTCONFIG_IPV6
		if (get_ipv6_service() == IPV6_6IN4) {
#ifdef RTCONFIG_BCMARM
#ifdef HND_ROUTER
			if (nvram_match("fc_pt_war", "1"))
#endif
			eval("ip6tables", "-t", "mangle", "-A", "FORWARD",
			     "-m", "state", "--state", "NEW", "-j", "MARK", "--set-mark", "0x01/0x7");
#else
			eval("ip6tables", "-t", "mangle", "-A", "FORWARD",
			     "-m", "state", "--state", "NEW", "-j", "SKIPLOG");
#endif
		}
#endif
	}
#endif
#if defined(RTCONFIG_LANTIQ)
	eval("iptables", "-t", "mangle", "-A", "INPUT",
	     "-p", "tcp", "-m", "state", "--state", "ESTABLISHED",
			"-j", "EXTMARK", "--set-mark", "0x80000000/0x80000000");
#endif
}

#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV) // RTCONFIG_DUALWAN || RTCONFIG_MULTICAST_IPTV
void
mangle_setting2(char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	int unit;
	char *wan_if;
	char chain[sizeof("QOSOxXXX")];
	int wan_max_unit = WAN_UNIT_MAX;
#ifdef CONFIG_BCMWL5 /* the only use so far */
	char lan_class[32];

	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
#endif

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6)
		wan_max_unit = WAN_UNIT_MULTICAST_IPTV_MAX;
#endif

	if(IS_NON_AQOS() || IS_ROG_QOS()){
		for(unit = WAN_UNIT_FIRST; unit < wan_max_unit; ++unit){
			if(!is_wan_connect(unit))
				continue;

			wan_if = get_wan_ifname(unit);
//#ifdef RTCONFIG_MULTICAST_IPTV
			if (nvram_get_int("switch_stb_x") > 6 &&
			    strstr(nvram_safe_get("iptv_wan_ifnames"), wan_if) != NULL)
				continue;
//#endif
			add_iQosRules(wan_if);
		}
	}
	else {
		eval("iptables", "-t", "mangle", "-F");
#ifdef RTCONFIG_IPV6
		eval("ip6tables", "-t", "mangle", "-F");
#endif
	}

#ifdef RTCONFIG_DNSFILTER
#ifdef RTCONFIG_IPV6
	if (nvram_get_int("dnsfilter_enable_x") && ipv6_enabled()) {
		FILE *fp;

		fp = fopen("/tmp/mangle_rules_ipv6.dnsfilter", "w");
		if (fp != NULL) {
			fprintf(fp, "*mangle\n"
				":DNSFILTERI - [0:0]\n"
				":DNSFILTERF - [0:0]\n"
				":DNSFILTER_DOT - [0:0]\n");

			dnsfilter6_settings(fp, lan_if, lan_ip);

			fprintf(fp, "COMMIT\n");
			fclose(fp);

			eval("ip6tables-restore", "/tmp/mangle_rules_ipv6.dnsfilter");
		}
	}
#endif /* RTCONFIG_IPV6 */
#endif /* RTCONFIG_DNSFILTER */


/* Workaround for neighbour solicitation flood from Comcast */
#ifdef RTCONFIG_IPV6
	if (nvram_get_int("ipv6_ns_drop")) {
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
			eval("ip6tables", "-t", "mangle", "-A", "PREROUTING", "-p", "icmpv6", "--icmpv6-type", "neighbor-solicitation",
			     "-i", get_wan_ifname(unit), "-d", "ff02::1:ff00:0/104", "-j", "DROP");
		}
	}
#endif

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && IS_TQOS()) {
		/* Create QOSOx chain for TQoS */
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			snprintf(chain, sizeof(chain), "QOSO%d", unit);
			eval("ip6tables", "-t", "mangle", "-N", chain);
		}
	}
#endif

#ifdef RTCONFIG_FBWIFI
	if(sw_mode() == SW_MODE_ROUTER){
		fbwifi_mangle();
	}
#endif

#ifdef RTCONFIG_YANDEXDNS
#ifdef RTCONFIG_IPV6
	if (nvram_get_int("yadns_enable_x") && ipv6_enabled()) {
		FILE *fp;

		fp = fopen("/tmp/mangle_rules_ipv6.yadns", "w");
		if (fp != NULL) {
			int evalRet;
			fprintf(fp, "*mangle\n"
			    ":YADNSI - [0:0]\n"
			    ":YADNSF - [0:0]\n");

			write_yandexdns_filter6(fp, lan_if, lan_ip);

			fprintf(fp, "COMMIT\n");
			fclose(fp);

			evalRet = eval("ip6tables-restore", "/tmp/mangle_rules_ipv6.yadns");
			rule_apply_checking("firewall", __LINE__, "/tmp/mangle_rules_ipv6.yadns", evalRet);
		}
	}
#endif /* RTCONFIG_IPV6 */
#endif /* RTCONFIG_YANDEXDNS */

	/* In Bangladesh, ISPs force the packet TTL as 1 at modem side to block ip sharing.
	 * Increase the TTL once the packet come at WAN with TTL=1 */
	if (nvram_get_int("ttl_inc_enable")) {
		for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; unit++) {
			wan_if = get_wan_ifname(unit);
			eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_if,
			     "-m", "ttl", "--ttl-eq", "1",
			     "-j", "TTL", "--ttl-set", "64");
#ifdef RTCONFIG_IPV6
			if ((wan_if = get_wan6_ifname(unit)) && *wan_if)
				eval("ip6tables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_if,
				     "-m", "hl", "--hl-eq", "1",
				     "-j", "HL", "--hl-set", "64");
#endif
		}
	}

	/* Several ISPs restrict Internet sharing by checking TTL value,
	 * i.e allow Phones/Tablets only. Fix WAN outgoing packets with
	 * router's TTL, default is 64  */
	if (nvram_get_int("ttl_spoof_enable")) {
		char value[sizeof("255")];
		int wan_ttl = 0;

		if (f_read_string("/proc/sys/net/ipv4/ip_default_ttl", value, sizeof(value)) > 0)
			wan_ttl = atoi(value);
		snprintf(value, sizeof(value), "%d", wan_ttl);

		for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; unit++) {
			wan_if = get_wan_ifname(unit);
			if (wan_ttl > 1) {
				eval("iptables", "-t", "mangle", "-A", "FORWARD", "-o", wan_if,
				    "-m", "ttl", "--ttl-gt", "30",
				    "-m", "ttl", "--ttl-lt", "254",
				    "-j", "TTL", "--ttl-set", value);
			}
			eval("iptables", "-t", "mangle", "-A", "FORWARD", "-o", wan_if,
			    "-m", "ttl", "--ttl-eq", "254",
			    "-j", "TTL", "--ttl-set", "255");
#ifdef RTCONFIG_IPV6
			if ((wan_if = get_wan6_ifname(unit)) && *wan_if) {
				int wan_hlim = ipv6_getconf(wan_if, "hop_limit") ? : ipv6_getconf("default", "hop_limit");
				if (wan_hlim > 1) {
					snprintf(value, sizeof(value), "%d", wan_hlim);
					eval("ip6tables", "-t", "mangle", "-A", "FORWARD", "-o", wan_if,
					     "-m", "hl", "--hl-gt", "30",
					     "-m", "hl", "--hl-lt", "254",
					     "-j", "HL", "--hl-set", value);
				}
				eval("ip6tables", "-t", "mangle", "-A", "FORWARD", "-o", wan_if,
				     "-m", "hl", "--hl-eq", "254",
				     "-j", "HL", "--hl-set", "255");
			}
#endif
		}
	}

#ifdef CONFIG_BCMWL5
#ifdef HND_ROUTER
	/* bypass flow cache learning */
	if (nvram_match("fc_disable", "0")) {
#else
	/* mark connect to bypass CTF */
	if (nvram_match("ctf_disable", "0")) {
#endif
#if defined(RTCONFIG_BWDPI)
		/* DPI engine */
		if (IS_AQOS()) {
				eval("iptables", "-t", "mangle", "-N", "BWDPI_FILTER");
				eval("iptables", "-t", "mangle", "-F", "BWDPI_FILTER");

			// for dual-wan case, it has mutli-mangle rules
			for (unit = WAN_UNIT_FIRST; unit < wan_max_unit; unit++) {
				wan_if = get_wan_ifname(unit);
//#ifdef RTCONFIG_MULTICAST_IPTV
				if (nvram_get_int("switch_stb_x") > 6 &&
				    strstr(nvram_safe_get("iptv_wan_ifnames"), wan_if) != NULL)
					continue;
//#endif
				eval("iptables", "-t", "mangle", "-A", "BWDPI_FILTER", "-i", wan_if, "-p", "udp", "--sport", "68", "--dport", "67", "-j", "DROP");
//				eval("iptables", "-t", "mangle", "-A", "BWDPI_FILTER", "-i", wan_if, "-p", "udp", "--sport", "67", "--dport", "68", "-j", "DROP");
				eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_if, "-p", "udp", "-j", "BWDPI_FILTER");
			}
		}
#endif
		/* mark 80 port connection */
		if (nvram_match("url_enable_x", "1") || nvram_match("keyword_enable_x", "1")) {
#ifdef HND_ROUTER
			if (nvram_match("fc_pt_war", "1"))
#endif
			eval("iptables", "-t", "mangle", "-A", "FORWARD",
			     "-p", "tcp", "--dport", "80",
			     "-m", "state", "--state", "NEW", "-j", "MARK", "--set-mark", "0x01/0x7");
		}

		/* mark VTS loopback connections */
		if (nvram_match("vts_enable_x", "1") || dmz_enabled() ||
			(is_nat_enabled() && nvram_get_int("upnp_enable"))) {
#ifdef HND_ROUTER
			if (nvram_match("fc_pt_war", "1"))
#endif
			eval("iptables", "-t", "mangle", "-A", "FORWARD",
			     "-o", lan_if, "-s", lan_class, "-d", lan_class,
			     "-j", "MARK", "--set-mark", "0x01/0x7");
		}
#ifdef RTCONFIG_BCMARM
		/* mark STUN connection*/
#ifdef HND_ROUTER
		if (nvram_match("fc_pt_udp_war", "1"))
#else
		if (nvram_match("ctf_pt_udp", "1"))
#endif
			eval("iptables", "-t", "mangle", "-A", "FORWARD",
			     "-p", "udp",
			     "-m", "state", "--state", "NEW", "-j", "MARK", "--set-mark", "0x01/0x7");
#endif
#ifdef RTCONFIG_IPV6
		if (get_ipv6_service() == IPV6_6IN4) {
#ifdef RTCONFIG_BCMARM
#ifdef HND_ROUTER
			if (nvram_match("fc_pt_war", "1"))
#endif
			eval("ip6tables", "-t", "mangle", "-A", "FORWARD",
			     "-m", "state", "--state", "NEW", "-j", "MARK", "--set-mark", "0x01/0x7");
#else
			eval("ip6tables", "-t", "mangle", "-A", "FORWARD",
			     "-m", "state", "--state", "NEW", "-j", "SKIPLOG");
#endif
		}
#endif
	}
#endif	/* CONFIG_BCMWL5 */

#ifdef RTCONFIG_DUALWAN
	if(!nvram_get_int("stop_adv_lb"))
		set_load_balance();
#endif
}
#endif // RTCONFIG_DUALWAN

#if 0
#ifdef RTCONFIG_BCMARM
void
del_samba_rules(void)
{
	char ifname[IFNAMSIZ];
	char *lan_ip = nvram_safe_get("lan_ipaddr");

	strncpy(ifname, nvram_safe_get("lan_ifname"), IFNAMSIZ);

	/* delete existed rules */
	eval("iptables", "-t", "raw", "-D", "PREROUTING", "-i", ifname, "-d", lan_ip, "-p", "tcp",
		"--dport", "137:139", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-D", "PREROUTING", "-i", ifname, "-d", lan_ip, "-p", "tcp",
		"--dport", "445", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-D", "PREROUTING", "-i", ifname, "-d", lan_ip, "-p", "udp",
		"--dport", "137:139", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-D", "PREROUTING", "-i", ifname, "-d", lan_ip, "-p", "udp",
		"--dport", "445", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-D", "OUTPUT", "-p", "tcp",
		"--sport", "137:139", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-D", "OUTPUT", "-p", "tcp",
		"--sport", "445", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-D", "OUTPUT", "-p", "udp",
		"--sport", "137:139", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-D", "OUTPUT", "-p", "udp",
		"--sport", "445", "-j", "NOTRACK");
/*
	eval("iptables", "-t", "filter", "-D", "INPUT", "-i", ifname, "-p", "udp",
		"--dport", "137:139", "-j", "ACCEPT");
	eval("iptables", "-t", "filter", "-D", "INPUT", "-i", ifname, "-p", "udp",
		"--dport", "445", "-j", "ACCEPT");
	eval("iptables", "-t", "filter", "-D", "INPUT", "-i", ifname, "-p", "tcp",
		"--dport", "137:139", "-j", "ACCEPT");
	eval("iptables", "-t", "filter", "-D", "INPUT", "-i", ifname, "-p", "tcp",
		"--dport", "445", "-j", "ACCEPT");
*/
}

add_samba_rules(void)
{
	char ifname[IFNAMSIZ];
	char *lan_ip = nvram_safe_get("lan_ipaddr");

	strncpy(ifname, nvram_safe_get("lan_ifname"), IFNAMSIZ);

	/* Add rules to disable conntrack on SMB ports to reduce CPU loading
	 * for SAMBA storage application
	 */
	eval("iptables", "-t", "raw", "-A", "PREROUTING", "-i", ifname, "-d", lan_ip, "-p", "tcp",
		"--dport", "137:139", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-A", "PREROUTING", "-i", ifname, "-d", lan_ip, "-p", "tcp",
		"--dport", "445", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-A", "PREROUTING", "-i", ifname, "-d", lan_ip, "-p", "udp",
		"--dport", "137:139", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-A", "PREROUTING", "-i", ifname, "-d", lan_ip, "-p", "udp",
		"--dport", "445", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-A", "OUTPUT", "-p", "tcp",
		"--sport", "137:139", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-A", "OUTPUT", "-p", "tcp",
		"--sport", "445", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-A", "OUTPUT", "-p", "udp",
		"--sport", "137:139", "-j", "NOTRACK");
	eval("iptables", "-t", "raw", "-A", "OUTPUT", "-p", "udp",
		"--sport", "445", "-j", "NOTRACK");
/*
	eval("iptables", "-t", "filter", "-I", "INPUT", "-i", ifname, "-p", "udp",
		"--dport", "137:139", "-j", "ACCEPT");
	eval("iptables", "-t", "filter", "-I", "INPUT", "-i", ifname, "-p", "udp",
		"--dport", "445", "-j", "ACCEPT");
	eval("iptables", "-t", "filter", "-I", "INPUT", "-i", ifname, "-p", "tcp",
		"--dport", "137:139", "-j", "ACCEPT");
	eval("iptables", "-t", "filter", "-I", "INPUT", "-i", ifname, "-p", "tcp",
		"--dport", "445", "-j", "ACCEPT");
*/
}
#endif
#endif

#ifdef RTCONFIG_MULTISERVICE_WAN
void add_mswan_rules(char *logaccept, char *logdrop)
{
	int unit = 0;
	char wan_prefix[16] = {0};
	char wan_ifname[16] = {0};
	char wan_ip[16] = {0};
	int misc_ping_x = nvram_get_int("misc_ping_x");

	for (unit = WAN_UNIT_FIRST_MULTISRV_BASE; unit < WAN_UNIT_MULTISRV_MAX; unit++)
	{
		snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", unit);
		if (nvram_pf_get_int(wan_prefix, "enable"))
		{
			if (!is_wan_connect(unit))
				continue;
			if(nvram_pf_match(wan_prefix, "proto", "bridge"))
				continue;

			snprintf(wan_ifname, sizeof(wan_ifname), "%s", get_wan_ifname(unit));
			snprintf(wan_ip, sizeof(wan_ip), "%s", nvram_pf_safe_get(wan_prefix, "ipaddr"));

			// filter
			if (!misc_ping_x)
			{
				eval("iptables", "-A", "INPUT_PING"
					, "-i", wan_ifname, "-p", "icmp", "-j", logdrop);
			}

			// nat
			if (nvram_pf_get_int(wan_prefix, "nat_x"))
			{
				eval("iptables", "-t", "nat", "-A", "PREROUTING"
					, "-d", wan_ip, "-j", "VSERVER");
				eval("iptables", "-t", "nat", "-A", "POSTROUTING"
					, "-o", wan_ifname, "!", "-s", wan_ip
					, "-j", "MASQUERADE");
			}
		}
	}
}
#endif

//int start_firewall(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip)
int start_firewall(int wanunit, int lanunit)
{
	DIR *dir;
	struct dirent *file;
	FILE *fp;
	char name[NAME_MAX];
	//char logaccept[32], logdrop[32];
	//oleg patch ~
	char logaccept[32], logdrop[32];
	char *mcast_ifname;
	char wan_if[IFNAMSIZ+1], wan_ip[32], lan_if[IFNAMSIZ+1], lan_ip[32];
	char wanx_if[IFNAMSIZ+1], wanx_ip[32], wan_proto[16];
	char prefix[] = "wanXXXXXXXXXX_", tmp[100];
	int lock;

	if (!is_routing_enabled())
		return -1;

	if (getpid() != 1 && getuid() != 0) {
		snprintf(tmp, sizeof(tmp), "start_firewall %d %d", wanunit, lanunit);
		notify_rc(tmp);
		return 0;
	}

	lock = file_lock("firewall");

	snprintf(prefix, sizeof(prefix), "wan%d_", wanunit);

	//(void)wan_ifname(wanunit, wan_if);
	strcpy(wan_if, get_wan_ifname(wanunit));
	strcpy(wan_proto, nvram_safe_get(strcat_r(prefix, "proto", tmp)));

	strcpy(wan_ip, nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
	strcpy(wanx_if, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	strcpy(wanx_ip, nvram_safe_get(strcat_r(prefix, "xipaddr", tmp)));

#ifdef RTCONFIG_IPV6
	strlcpy(wan6face, get_wan6face(), sizeof(wan6face));
#endif

	// handle one only
	strcpy(lan_if, nvram_safe_get("lan_ifname"));
	strcpy(lan_ip, nvram_safe_get("lan_ipaddr"));

#ifdef RTCONFIG_EMF
	/* No limitations now, allow IPMPv3 for LAN at least */
//	/* Force IGMPv2 due EMF limitations */
//	if (nvram_get_int("emf_enable")) {
//		f_write_string("/proc/sys/net/ipv4/conf/default/force_igmp_version", "2", 0, 0);
//		f_write_string("/proc/sys/net/ipv4/conf/all/force_igmp_version", "2", 0, 0);
//	}
#endif

	/* Mcast needs rp filter to be turned off only for non default iface */
	if (nvram_get_int("mr_enable_x") || nvram_get_int("udpxy_enable_x")) {
		char wan_prefix[sizeof("wanXXXXXXXXXX_")];
		char *wan_ifname = get_wan_ifname(wan_primary_ifunit());

		snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());
		mcast_ifname = nvram_safe_get(strcat_r(wan_prefix, "ifname", tmp));

#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_DUALWAN
		if (get_dualwan_primary() == WANS_DUALWAN_IF_DSL && nvram_get_int("dslx_config_num") > 1)
			mcast_ifname = "br1";
#else
		mcast_ifname = "br1";
#endif
#elif defined(RTCONFIG_MULTICAST_IPTV)
		if (nvram_get_int("switch_stb_x") > 6) {
			if (nvram_match("switch_wantag", "maxis_fiber_sp_iptv") ||
			    nvram_match("switch_wantag", "maxis_fiber_iptv") ||
			    nvram_match("switch_wantag", "starhub") ||
			    nvram_match("switch_wantag", "movistar"))
				mcast_ifname = nvram_safe_get("iptv_ifname");
		}
#endif
		/* Don't turn off rp_filter for default interface */
		if (wan_ifname && strcmp(wan_ifname, mcast_ifname) == 0)
			mcast_ifname = NULL;
	} else
		mcast_ifname = NULL;

	/* Block obviously spoofed IP addresses */
	if ((dir = opendir("/proc/sys/net/ipv4/conf")) != NULL) {
		while ((file = readdir(dir)) != NULL) {
			if (strncmp(file->d_name, ".", NAME_MAX) != 0 &&
			    strncmp(file->d_name, "..", NAME_MAX) != 0) {
				sprintf(name, "/proc/sys/net/ipv4/conf/%s/rp_filter", file->d_name);
				f_write_string(name, mcast_ifname &&
					strncmp(file->d_name, mcast_ifname, NAME_MAX) == 0 ? "0" :
					strncmp(file->d_name, "all", NAME_MAX) == 0 ? "-1" : "1", 0, 0);
			}
		}
		closedir(dir);
	} else
		perror("/proc/sys/net/ipv4/conf");

	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strcpy(logaccept, "logaccept");
	else strcpy(logaccept, "ACCEPT");

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strcpy(logdrop, "logdrop");
	else strcpy(logdrop, "DROP");

#ifdef RTCONFIG_IPV6
	if (get_ipv6_service() != IPV6_DISABLED) {
		if (!f_exists("/proc/sys/net/netfilter/nf_conntrack_frag6_timeout"))
			modprobe("nf_conntrack_ipv6");
#ifndef HND_ROUTER
		modprobe("ip6t_REJECT");
		modprobe("ip6t_ROUTE");
		modprobe("ip6t_LOG");
#endif
		modprobe("xt_length");
	} else {
		modprobe_r("xt_length");
#ifndef HND_ROUTER
		modprobe_r("ip6t_LOG");
		modprobe_r("ip6t_ROUTE");
		modprobe_r("ip6t_REJECT");
#endif
		modprobe_r("nf_conntrack_ipv6");
	}
#endif
	if (nvram_get_int("ttl_inc_enable") || nvram_get_int("ttl_spoof_enable")) {
		modprobe("xt_HL");
		modprobe("xt_hl");
	}
	/* nat setting */
 #ifdef RTCONFIG_DUALWAN // RTCONFIG_DUALWAN
	if (nvram_match("wans_mode", "lb")) {
 		nat_setting2(lan_if, lan_ip, logaccept, logdrop);

#ifdef WEB_REDIRECT
		redirect_setting();
#endif

		filter_setting2(lan_if, lan_ip, logaccept, logdrop);

		mangle_setting2(lan_if, lan_ip, logaccept, logdrop);
	}
	else
#endif // RTCONFIG_DUALWAN
#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6 &&
	    (nvram_match("switch_wantag", "movistar") || nvram_match("switch_wantag", "starhub"))) {
 		nat_setting2(lan_if, lan_ip, logaccept, logdrop);

#ifdef WEB_REDIRECT
		redirect_setting();
#endif

		filter_setting2(lan_if, lan_ip, logaccept, logdrop);

		mangle_setting2(lan_if, lan_ip, logaccept, logdrop);
	}
	else
#endif // RTCONFIG_MULTICAST_IPTV
	{
		if(wanunit != wan_primary_ifunit()
#ifdef RTCONFIG_MULTISERVICE_WAN
			&& wanunit < WAN_UNIT_MAX
#endif
		)
			goto leave;

 		nat_setting(wan_if, wan_ip, wanx_if, wanx_ip, lan_if, lan_ip, logaccept, logdrop);

#ifdef WEB_REDIRECT
		redirect_setting();
#endif

		filter_setting(wanunit, lan_if, lan_ip, logaccept, logdrop);

		mangle_setting(wan_if, wan_ip, lan_if, lan_ip, logaccept, logdrop);
	}

#ifdef RTCONFIG_IPV6
	if (!ipv6_enabled())
	{
		eval("ip6tables", "-F");
		eval("ip6tables", "-t", "mangle", "-F");
	}
#endif

	enable_ip_forward();

	/* Tweak NAT performance... */
/*
	if ((fp=fopen("/proc/sys/net/core/netdev_max_backlog", "w+")))
	{
		fputs("2048", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/core/somaxconn", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_max_syn_backlog", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/core/rmem_default", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/rmem_max", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/wmem_default", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/wmem_max", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/ipv4/tcp_rmem", "w+")))
	{
		fputs("8192 131072 262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_wmem", "w+")))
	{
		fputs("8192 131072 262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh1", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh2", "w+")))
	{
		fputs("2048", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh3", "w+")))
	{
		fputs("4096", fp);
		fclose(fp);
	}
*/
	if ((fp=fopen("/proc/sys/net/ipv4/tcp_fin_timeout", "w+")))
	{
		fputs("40", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_intvl", "w+")))
	{
		fputs("30", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_probes", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_time", "w+")))
	{
		fputs("1800", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_retries2", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syn_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_synack_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_recycle", "w+")))
	{
#if defined(RTCONFIG_BCMARM) || defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK)
		fputs("0", fp);
#else
		fputs("1", fp);
#endif
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_reuse", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	/* Tweak DoS-related... */
	if ((fp=fopen("/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_rfc1337", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syncookies", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	setup_ftp_conntrack(nvram_get_int("vts_ftpport"));
	setup_pt_conntrack();

#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
	if(strcmp(nvram_safe_get("apps_dev"), "") != 0)
		run_app_script(NULL, "firewall-start");
#endif

#ifdef RTCONFIG_OPENVPN
	ovpn_run_fw_scripts();
#endif

	if (!nvram_get_int("ttl_inc_enable") && !nvram_get_int("ttl_spoof_enable")) {
		modprobe_r("xt_HL");
		modprobe_r("xt_hl");
	}

#ifdef RTCONFIG_IPSEC
	if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		 || nvram_get_int("ipsec_ig_enable")
#endif
		)
		run_ipsec_firewall_scripts();
#endif
#ifdef RTCONFIG_LETSENCRYPT
	run_le_fw_script();
#endif

#ifdef RTCONFIG_MULTISERVICE_WAN
	add_mswan_rules(logaccept, logdrop);
#endif

	/* Assuming wan interface doesn't change */
	reload_upnp();

leave:
	file_unlock(lock);

	run_custom_script("firewall-start", 0, wan_if, NULL);

	return 0;
}

void enable_ip_forward(void)
{
	/*
		ip_forward - BOOLEAN
			0 - disabled (default)
			not 0 - enabled

			Forward Packets between interfaces.

			This variable is special, its change resets all configuration
			parameters to their default state (RFC1122 for hosts, RFC1812
			for routers)
	*/
	f_write_string("/proc/sys/net/ipv4/ip_forward", "1", 0, 0);

#ifdef RTCONFIG_HND_ROUTER_AX_6710
	f_write_string("/proc/sys/net/ipv4/ip_dynaddr", "1", 0, 0);
#endif

#if 0
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) {
		f_write_string("/proc/sys/net/ipv6/conf/all/forwarding", "1", 0, 0);
	} else {
		f_write_string("/proc/sys/net/ipv6/conf/all/forwarding", "0", 0, 0);
	}
#endif
#endif
}

#define ERR_RULES_PATH "/tmp/err_rules"
void rule_apply_checking(char *caller, int line, char *rule_path, int ret) {
	if (ret) {
		char dst_file[64];
		logmessage(caller, "apply rules error(%d)", line);
		if (!check_if_dir_exist(ERR_RULES_PATH))
			mkdir(ERR_RULES_PATH, 0777);
		snprintf(dst_file, sizeof(dst_file), "%s/%s_%u", ERR_RULES_PATH, basename(rule_path), (unsigned int)time(NULL));
		eval("cp", rule_path, dst_file);
	}
}
