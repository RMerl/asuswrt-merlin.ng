/*
 * Broadcom Home Gateway Reference Design
 * Broadcom WiFi Insight Webpage functions
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * $Id: vis_gui.c 602657 2015-11-27 02:24:30Z $
 */
#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>
#endif /* WEBS */
#include <netdb.h>
#ifdef HND_ROUTER
#include "bcmwifi_rates.h"
#include "wlioctl_defs.h"
#endif
#include <wlioctl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include "shutils.h"
#include "wlutils.h"

#include "vis_gui.h"
#include <proto/ethernet.h>
#include <bcmnvram.h>

#ifdef __CONFIG_VISUALIZATION__

#include <signal.h>


#define sys_restart() kill(1, SIGHUP)

/* Indicates the level of debug message to be printed on the console */
static int vis_debug_level = 0;

#define VIS_REQ_NVRAM_GET	"VisGetNVRAM"
#define VIS_REQ_NVRAM_SET	"VisSetNVRAM"


#define INVALID_SOCKET			-1
#define VIS_SOCKET_WAIT_TIMEOUT		15
#define MAX_READ_BUFFER			1449
#define VISUALIZATION_SERVER_PORT	8888
#define VISUALIZATION_SERVER_ADDRESS	"127.0.0.1"
#define VIS_LEN_FIELD_SZ		4 /* Number of bytes used to specify the length */
#define VIS_MAX_XML_LEN			1024
#define VIS_MAX_REQ_LEN			50
/* MAC address from WEB has %3A for every : character so length as 30 */
#define VIS_MAX_MAC_LEN			30
#define VIS_MAX_BAND_LEN		3
#define VIS_MAX_IP_LEN			16
#define VIS_MAX_NVRAM_LEN		100


/* Visualization error printing defines */
#define VIS_DEBUG_ERROR			0x0001
#define VIS_DEBUG_WARNING		0x0002
#define VIS_DEBUG_INFO			0x0004
#define VIS_DEBUG_DETAIL		0x0008

#define VIS_ERROR(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_ERROR) \
			printf("VIS-ERROR >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_WARNING(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_WARNING) \
			printf("VIS-WARNING >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_INFO(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_INFO) \
			printf("VIS-INFO >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

#define VIS_DEBUG(fmt, arg...) \
		do { if (vis_debug_level & VIS_DEBUG_DETAIL) \
			printf("VIS-DEBUG >> %s : "fmt, __FUNCTION__, ##arg); } while (0)

typedef struct vis_xml_strings_t_ {
	int nallocated;	/* Number bytes allocated */
	int nlength;	/* Number of bytes filled */
	char *str;	/* string to hold the xml data */
} vis_xml_strings_t;


enum vis_enum_req_args {
	VIS_UNKNOWN,
	VIS_REQUEST,
	VIS_DUTMAC,
	VIS_STAMAC,
	VIS_FREQBAND,
	VIS_INTERVAL,
	VIS_DBSIZE,
	VIS_STARTSTOP,
	VIS_TOTAL,
	VIS_GRAPHNAME,
	VIS_DCONIP,
	VIS_ISREMOTE_ENABLE,
	VIS_DOSCAN,
	VIS_ISOVERWRITEDB,
	VIS_ISAUTOSTART,
	VIS_WEEKDAYS,
	VIS_FROMTM,
	VIS_TOTM,
	VIS_NVRAMNAME,
	VIS_NVRAMVALUE,
	};

typedef struct {
	enum vis_enum_req_args id; /* ID of the request argument */
	char *req_name; /* Name of the argument */
} vis_req_args;

vis_req_args vis_req_args_list[] = {
	{VIS_UNKNOWN, "Unknown"},
	{VIS_REQUEST, "Req"},
	{VIS_DUTMAC, "DutMac"},
	{VIS_STAMAC, "StaMac"},
	{VIS_FREQBAND, "FreqBand"},
	{VIS_INTERVAL, "Interval"},
	{VIS_DBSIZE, "DBSize"},
	{VIS_STARTSTOP, "StartStop"},
	{VIS_TOTAL, "Total"},
	{VIS_GRAPHNAME, "graphname"},
	{VIS_DCONIP, "DCONIP"},
	{VIS_ISREMOTE_ENABLE, "IsRemoteDebugEnabled"},
	{VIS_DOSCAN, "DoScan"},
	{VIS_ISOVERWRITEDB, "OverWriteDB"},
	{VIS_ISAUTOSTART, "AutoStart"},
	{VIS_WEEKDAYS, "WeekDays"},
	{VIS_FROMTM, "FromTm"},
	{VIS_TOTM, "ToTm"},
	{VIS_NVRAMNAME, "NVRAMName"},
	{VIS_NVRAMVALUE, "NVRAMValue"},
};

/* This will check whether to reallocate memory for XML buffer or not */
#define CHECK_XML_BUFF(xml_strings, nlenreq) if (xml_strings->str == NULL ||\
		(xml_strings->nallocated < (xml_strings->nlength+nlenreq))) {\
		if (vis_xml_allocate_buffer(xml_strings, nlenreq) != 0)\
			return; }

static unsigned char *vis_rdata = NULL;

/* Visualization WebUI handling starts here */

/* Closes the socket given the file descriptor */
static void
close_socket(int *sockfd)
{
	if (*sockfd == INVALID_SOCKET)
		return;

	close(*sockfd);
	*sockfd = INVALID_SOCKET;
}

/* Sends the buffer to server */
static int
send_data(int sockfd, char *data, unsigned int len)
{
	int	nret = 0;
	int	totalsize = len, totalsent = 0;

	while (totalsent < totalsize) {
		fd_set WriteFDs, ExceptFDs;
		struct timeval tv;

		FD_ZERO(&WriteFDs);
		FD_ZERO(&ExceptFDs);

		if (sockfd == INVALID_SOCKET)
			return INVALID_SOCKET;

		FD_SET(sockfd, &WriteFDs);

		tv.tv_sec = VIS_SOCKET_WAIT_TIMEOUT;
		tv.tv_usec = 0;
		if (select(sockfd+1, NULL, &WriteFDs, &ExceptFDs, &tv) > 0) {
			if (!FD_ISSET(sockfd, &WriteFDs)) {
				VIS_WARNING("Write descriptor is not set. Not ready to write\n");
				return INVALID_SOCKET;
			}
		}

		nret = send(sockfd, &(data[totalsent]), len, 0);
		if (nret < 0) {
			VIS_WARNING("Failed to send data error :  %s\n", strerror(errno));
			return INVALID_SOCKET;
		}
		totalsent += nret;
		len -= nret;
		nret = 0;
	}

	return totalsent;
}

/* Adds the 4 bytes length and then data to a buffer and sends it to
 * server
 */
static int
add_length_and_send_data(int sockfd, char *data, unsigned int len)
{
	int	ret = 0;
	char	*sdata = NULL;
	int	totlen = VIS_LEN_FIELD_SZ + len;

	sdata = (char*)malloc(sizeof(char) * (totlen));
	if (sdata == NULL) {
		VIS_DEBUG("Failed to allocate sdata buffer of size : %d\n", totlen);
		return -1;
	}

	memcpy(sdata, &totlen, VIS_LEN_FIELD_SZ);

	memcpy(sdata+VIS_LEN_FIELD_SZ, data, len);

	ret = send_data(sockfd, sdata, totlen);

	free(sdata);

	return ret;
}

/* Recieves the 'size' number of data */
static int
recv_data(int sockfd, unsigned char *read_buf, uint32 size)
{
	uint32		nbytes, totalread = 0;
	fd_set		ReadFDs, ExceptFDs;
	struct timeval	tv;

	FD_ZERO(&ReadFDs);
	FD_ZERO(&ExceptFDs);
	FD_SET(sockfd, &ReadFDs);
	FD_SET(sockfd, &ExceptFDs);
	tv.tv_sec = VIS_SOCKET_WAIT_TIMEOUT;
	tv.tv_usec = 0;

	while (totalread < size) {
		if (select(sockfd+1, &ReadFDs, NULL, &ExceptFDs, &tv) > 0) {
			if (!FD_ISSET(sockfd, &ReadFDs)) {
				VIS_WARNING("Read descriptor is not set. Not ready to read\n");
				return INVALID_SOCKET;
			}
		}
		nbytes = read(sockfd, read_buf, size);

		if (nbytes <= 0) {
			VIS_WARNING("Failed to read data error : %s\n", strerror(errno));
			return INVALID_SOCKET;
		}

		totalread += nbytes;
		read_buf += totalread;
	}

	*read_buf = '\0';

	return totalread;
}

/* Recieves the data from server
 * return value contains the number of bytes read or -1 for error
 * free the buffer after reading the data
 */
static int
on_receive(int sockfd, unsigned char **data)
{
	uint32		sz, size = 0;
	unsigned char	szbuff[VIS_LEN_FIELD_SZ+1] = {0};
	unsigned char	*read_buf = NULL;

	sz = recv_data(sockfd, szbuff, VIS_LEN_FIELD_SZ);
	if (sz <= 0) {
		return INVALID_SOCKET;
	}

	if (sz >= VIS_LEN_FIELD_SZ) {
		memcpy(&size, szbuff, sizeof(size));
	} else {
		VIS_DEBUG("Failed to receive length field\n");
		return -1;
	}

	read_buf = (unsigned char *)malloc(sizeof(unsigned char) * ((size-VIS_LEN_FIELD_SZ) + 1));
	if (read_buf == NULL) {
		VIS_DEBUG("Failed to allocate read_buf buffer of size : %d\n",
			((size-VIS_LEN_FIELD_SZ) + 1));
		return -1;
	}

	sz = recv_data(sockfd, read_buf, (size-VIS_LEN_FIELD_SZ));
	if (sz <= 0) {
		if (read_buf != NULL) {
			free(read_buf);
			read_buf = NULL;
		}
		return INVALID_SOCKET;
	}

	*data = read_buf;

	return sz;
}

/* Connects to server given the port and address */
static int
connect_to_server(uint32 nport, char *straddrs)
{
	int			res, valopt, sockfd;
	long			arg;
	socklen_t		lon;
	fd_set			readfds;
	struct timeval		tv;
	struct hostent		*host;
	struct sockaddr_in	server_addr;

	sockfd = INVALID_SOCKET;
	memset(&server_addr, 0, sizeof(server_addr));

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		VIS_DEBUG("Failed to create socket error : %s\n", strerror(errno));
		goto vis_connect_err;
	}

	if ((host = gethostbyname(straddrs)) == NULL) {
		VIS_DEBUG("gethostbyname error : %s\n", strerror(errno));
		goto vis_connect_err;
	}

	/* Set nonblock on the socket so we can timeout */
	if ((arg = fcntl(sockfd, F_GETFL, NULL)) < 0 ||
		fcntl(sockfd, F_SETFL, arg | O_NONBLOCK) < 0) {
			VIS_DEBUG("fcntl error : %s\n", strerror(errno));
			goto vis_connect_err;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nport);
	server_addr.sin_addr = *((struct in_addr*)host -> h_addr);
	res = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if (res < 0) {
		if (errno == EINPROGRESS) {
			tv.tv_sec = 30;
			tv.tv_usec = 0;
			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);
			if (select(sockfd+1, NULL, &readfds, NULL, &tv) > 0) {
				lon = sizeof(int);
				getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
				if (valopt) {
					VIS_DEBUG("Error in connection() %d - %s\n",
						valopt, strerror(valopt));
					goto vis_connect_err;
				}
			} else {
				VIS_DEBUG("Timeout or error() %d - %s\n", valopt, strerror(valopt));
				goto vis_connect_err;
			}
		} else {
			VIS_DEBUG("Error connecting %d - %s\n", errno, strerror(errno));
			goto vis_connect_err;
		}
	}

	return sockfd;

vis_connect_err:
	close_socket(&sockfd);
	return INVALID_SOCKET;
}

/* Sends the XML data to server */
static int
send_ui_frontend_data(char *xmldata)
{
	int	sockfd = INVALID_SOCKET;
	int	ret = 0;

	sockfd = connect_to_server(VISUALIZATION_SERVER_PORT, VISUALIZATION_SERVER_ADDRESS);
	if (sockfd != INVALID_SOCKET) {
		ret = add_length_and_send_data(sockfd, xmldata, strlen(xmldata));
		if (ret != INVALID_SOCKET) {
			vis_rdata = NULL;
			ret = on_receive(sockfd, &vis_rdata);
			if (ret <= 0) {
				VIS_DEBUG("Failed to recieve data\n");
			}
		} else {
			VIS_DEBUG("Failed to send XML data = %s\n", xmldata);
		}
	} else {
		VIS_DEBUG("Failed to Connect to server : %s Port : %d\n",
			VISUALIZATION_SERVER_ADDRESS, VISUALIZATION_SERVER_PORT);
	}

	close_socket(&sockfd);

	return ret;
}

/* Write json answer to stream */
void
vis_do_json_get(char *url, FILE *stream)
{
	if (vis_rdata != NULL)
		fputs((const char*)vis_rdata, stream);

	if (vis_rdata != NULL) {
		free(vis_rdata);
		vis_rdata = NULL;
	}
}

/* Allocates memory for XML data buffer */
static inline int
vis_xml_allocate_buffer(vis_xml_strings_t *xml_strings, int nreq)
{
	int nallocate = VIS_MAX_XML_LEN;

	if (xml_strings->nallocated > 0) { /* realloc */
		if ((xml_strings->nallocated + nallocate) < (xml_strings->nlength + nreq + 1))
			nallocate = xml_strings->nlength + nreq + 1;
		else
			nallocate += xml_strings->nallocated;
		char *tmpptr = (char*)realloc(xml_strings->str, nallocate);
		if (tmpptr == NULL) {
			VIS_DEBUG("Memory reallocation failed of : %d for XML buffer\n", nallocate);
			return -1;
		}
		xml_strings->str = tmpptr;
		xml_strings->nallocated = nallocate;
	} else {
		if (nallocate < (nreq+1))
			nallocate = nreq+1;

		xml_strings->str = (char*)malloc(sizeof(char)*nallocate);
		if (xml_strings->str == NULL) {
			VIS_DEBUG("Memory allocation failed of : %d for XML buffer\n", nallocate);
			return -1;
		}
		xml_strings->nallocated = nallocate;
	}

	return 0;
}

/* Frees the XML data buffer */
static inline void
vis_xml_free_data_buffer(vis_xml_strings_t *xml_strings)
{
	if (xml_strings->str) {
		free(xml_strings->str);
		xml_strings->str = NULL;
	}
}

/* Add only start or end tag */
static inline void
vis_xml_add_only_tag(vis_xml_strings_t *xml_strings, char *tag, bool isstarttag)
{
	int nlenreq = 0;

	/* total len required for xml tag is 1Tag's + 2open brackets */
	nlenreq = strlen(tag) + 2 + 1;
	if (!isstarttag)
		nlenreq += 1; /* For end tag need / */
	CHECK_XML_BUFF(xml_strings, nlenreq);

	if (isstarttag) {
		xml_strings->nlength += snprintf(xml_strings->str+xml_strings->nlength,
			xml_strings->nallocated-xml_strings->nlength, "<%s>", tag);
	} else {
		xml_strings->nlength += snprintf(xml_strings->str+xml_strings->nlength,
			xml_strings->nallocated-xml_strings->nlength, "</%s>", tag);
	}
}

/* Add only the value into tag */
static inline void
vis_xml_add_only_value(vis_xml_strings_t *xml_strings, char *value)
{
	int nlenreq = 0;

	/* total len required for xml tag is 1Tag's + 2open brackets */
	nlenreq = strlen(value) + 1;
	CHECK_XML_BUFF(xml_strings, nlenreq);

	xml_strings->nlength += snprintf(xml_strings->str+xml_strings->nlength,
		xml_strings->nallocated-xml_strings->nlength, "%s", value);
}

/* Adds TAG and Value pair to the xml data buffer */
static inline void
vis_xml_add_tag_value(vis_xml_strings_t *xml_strings, char *tag, char *value)
{
	int nlenreq = 0;

	/* total len required for one xml tag is
	 * 2Tag's + 2open brackets + 2closing brackets + one slash + one value
	 */
	nlenreq = (2 * strlen(tag)) + 5 + strlen(value) + 1;
	CHECK_XML_BUFF(xml_strings, nlenreq);

	xml_strings->nlength += snprintf(xml_strings->str+xml_strings->nlength,
		xml_strings->nallocated-xml_strings->nlength, "<%s>%s</%s>",
		tag, value, tag);
}

/* Adds TAG and attribute to the xml data buffer */
static inline void
vis_xml_add_tag_and_attribute(vis_xml_strings_t *xml_strings, char *tag,
	char *attribname, char *attribvalue)
{
	int nlenreq = 0;

	/* total len required for one xml tag is
	 * 1Tag's + 2 brackets
	 * + one space + one attribute name + one attribute value + one =
	 */
	nlenreq = strlen(tag) + 4 + strlen(attribname) + strlen(attribvalue) + 1;
	CHECK_XML_BUFF(xml_strings, nlenreq);

	xml_strings->nlength += snprintf(xml_strings->str+xml_strings->nlength,
		xml_strings->nallocated-xml_strings->nlength, "<%s %s=%s>",
		tag, attribname, attribvalue);
}

/* Does NVRAM set or get based on the request */
static void
vis_do_nvram_operation(const char *request, const char *name, char *value, const int nvalue_sz)
{
	if (strcmp(request, VIS_REQ_NVRAM_GET) == 0) {
		char *tmpvalue = nvram_get(name);

		if (tmpvalue != NULL)
			snprintf(value, nvalue_sz, "%s", tmpvalue);
		else
			value[0] = '0'; /* Default is 0 */
		/* Allocate memory for response buffer */
		vis_rdata = (unsigned char*)malloc(sizeof(char) * MAX_READ_BUFFER);
		snprintf((char*)vis_rdata, MAX_READ_BUFFER,
			"{\"Req\":\"%s\",\"NVRAMName\":\"%s\",\"NVRAMValue\":\"%s\"}",
			request, name, value);
		vis_rdata[MAX_READ_BUFFER - 1] = '\0';
	} else if (strcmp(request, VIS_REQ_NVRAM_SET) == 0) {
		nvram_set(name, value);
		nvram_commit();
		sys_restart();
	}
}


/* Read query from stream in json format treate it and create answer string
 * The query is a list of commands and treated by 'do_json_command'
 */
static void
create_xml_string(const char *querystring, vis_xml_strings_t *xmlstring)
{
	char *pch;
	char request[VIS_MAX_REQ_LEN] = {0};
	char dutmac[VIS_MAX_MAC_LEN] = {0};
	char stamac[VIS_MAX_MAC_LEN] = {0};
	char band[VIS_MAX_BAND_LEN] = {0};
	char dcon_ip[VIS_MAX_IP_LEN] = {0};
	char name_nvram[VIS_MAX_NVRAM_LEN] = {0};
	char value_nvram[VIS_MAX_NVRAM_LEN] = {0};
	int isenabled = 0;
	static int donvramread = 1;
	vis_xml_strings_t configxml = {0};
	vis_xml_strings_t graphxml = {0};
	vis_xml_strings_t dutset = {0};
	vis_xml_strings_t pkthdr = {0};

	/* Read NVRAM variable for debug print. Read only once, that is why checking for 1
	 * From next time onwards default value 0 will be set to static var
	 */
	if (donvramread == 1) {
		char *tmpdebugflag = nvram_get("vis_debug_level");
		if (tmpdebugflag)
			vis_debug_level = strtoul(tmpdebugflag, NULL, 0);
		donvramread = 0;
	}

	VIS_DEBUG("Request : %s\n", querystring);

	vis_xml_add_tag_and_attribute(&pkthdr, "PacketVersion", "Name", "\"1\"");
	vis_xml_add_only_tag(&pkthdr, "PacketHeader", TRUE); /* Open packet header tag */
	vis_xml_add_tag_value(&pkthdr, "PacketType", "1");
	vis_xml_add_tag_value(&pkthdr, "From", "2");

	vis_xml_add_only_tag(&configxml, "Config", TRUE); /* Open Config tag */
	vis_xml_add_only_tag(&graphxml, "EnabledGraphs", TRUE); /* Open enabled graphs tag */

	pch = strtok((char*)querystring, "?=&");
	while (pch != NULL) {
		int i;
		enum vis_enum_req_args id = VIS_UNKNOWN;

		for (i = 0; i < (sizeof(vis_req_args_list)/sizeof(vis_req_args)); i++) {
			if (strstr(pch, vis_req_args_list[i].req_name) != NULL) {
				pch = strtok(NULL, "?=&");
				if (pch == NULL)
					break;
				id = vis_req_args_list[i].id;
				break;
			}
		}
		switch (id) {
			case VIS_REQUEST: /* Get the request name */
				snprintf(request, sizeof(request), "%s", pch);
				vis_xml_add_tag_value(&pkthdr, "ReqRespType", request);
				break;
			case VIS_DUTMAC: /* Get the DUTMAC */
				snprintf(dutmac, sizeof(dutmac), "%s", pch);
				break;
			case VIS_STAMAC: /* Get the STAMAC */
				snprintf(stamac, sizeof(stamac), "%s", pch);
				break;
			case VIS_FREQBAND: /* Get the FreqBand */
				snprintf(band, sizeof(band), "%s", pch);
				vis_xml_add_tag_value(&pkthdr, "FreqBand", band);
				break;
			case VIS_INTERVAL: /* Get Interval for config settings */
				vis_xml_add_tag_value(&configxml, "SampleInterval", pch);
				break;
			case VIS_DBSIZE: /* Get Max database size for config settings */
				vis_xml_add_tag_value(&configxml, "dbsize", pch);
				break;
			case VIS_STARTSTOP: /* Tells to stop or start the data collection */
				vis_xml_add_tag_value(&configxml, "startstop", pch);
				break;
			case VIS_TOTAL: /* Get total graph names for config settings */
				vis_xml_add_tag_value(&graphxml, "Total", pch);
				break;
			case VIS_GRAPHNAME: /* Get graphname for config settings */
				vis_xml_add_tag_value(&graphxml, "graphname", pch);
				break;
			case VIS_DCONIP: /* Server IP for remote debug */
				snprintf(dcon_ip, sizeof(dcon_ip), "%s", pch);
				break;
			case VIS_ISREMOTE_ENABLE: /* Flag for remote debugging enabled */
				isenabled = atoi(pch);
				break;
			case VIS_DOSCAN: /* Flag for DO scan flag */
				vis_xml_add_only_tag(&dutset, "DUTSet", TRUE);
				vis_xml_add_tag_value(&dutset, "DoScan", pch);
				vis_xml_add_only_tag(&dutset, "DUTSet", FALSE);
				break;
			case VIS_ISOVERWRITEDB: /* Get Overwrite DB flag for config settings */
				vis_xml_add_tag_value(&configxml, "overwrtdb", pch);
				break;
			case VIS_ISAUTOSTART: /* Get Is Auto Start flag for config settings */
				vis_xml_add_tag_value(&configxml, "autost", pch);
				break;
			case VIS_WEEKDAYS: /* Get Weekdays for config settings */
				vis_xml_add_tag_value(&configxml, "wkdays", pch);
				break;
			case VIS_FROMTM: /* Get From Time for config settings */
				vis_xml_add_tag_value(&configxml, "frmtm", pch);
				break;
			case VIS_TOTM: /* Get To Time for config settings */
				vis_xml_add_tag_value(&configxml, "totm", pch);
				break;
			case VIS_NVRAMNAME: /* Name of the NVRAM variable */
				snprintf(name_nvram, sizeof(name_nvram), "%s", pch);
				break;
			case VIS_NVRAMVALUE: /* Value to be set on the NVRAM */
				snprintf(value_nvram, sizeof(value_nvram), "%s", pch);
				break;
			case VIS_UNKNOWN:
				break;
		}
		pch = strtok(NULL, "?=&");
	}

	if (strcmp(request, "RemoteSettings") == 0) {
		char *tmpdcon_ip = nvram_get("vis_dcon_ipaddr");
		if (tmpdcon_ip == NULL)
			goto vis_fun_end;
		char *value = nvram_get("vis_do_remote_dcon");
		if (value)
			isenabled = atoi(value);
		vis_rdata = (unsigned char*)malloc(sizeof(char) * MAX_READ_BUFFER);
		memset(vis_rdata, 0, MAX_READ_BUFFER);
		snprintf((char*)vis_rdata, MAX_READ_BUFFER,
			"{\"IsEnabled\":%d,\"ServerIP\":\"%s\"}",
			isenabled, tmpdcon_ip);
		goto vis_fun_end;
	} else if (strcmp(request, "SetRemoteDebug") == 0) {
		char tmpbuf[5] = {0};

		nvram_set("vis_dcon_ipaddr", dcon_ip);
		snprintf(tmpbuf, sizeof(tmpbuf), "%d", isenabled);
		nvram_set("vis_do_remote_dcon", tmpbuf);
		nvram_commit();
		sys_restart();
		goto vis_fun_end;
	} else if ((strcmp(request, VIS_REQ_NVRAM_GET) == 0) ||
		(strcmp(request, VIS_REQ_NVRAM_SET) == 0)) {
		vis_do_nvram_operation(request, name_nvram, value_nvram, sizeof(value_nvram));
		goto vis_fun_end;
	}
	vis_xml_add_only_tag(&graphxml, "EnabledGraphs", FALSE); /* Close EnabledGraphs Tag */
	if (graphxml.str)
		vis_xml_add_only_value(&configxml, graphxml.str);
	vis_xml_add_only_tag(&configxml, "Config", FALSE); /* Close Config Tag */
	vis_xml_add_only_tag(&pkthdr, "PacketHeader", FALSE); /* Close Packet Header Tag */

	vis_xml_add_only_tag(xmlstring, "?xml version=\"1.0\" encoding=\"utf-8\"?", TRUE);
	if (configxml.str)
		vis_xml_add_only_value(&pkthdr, configxml.str);
	vis_xml_add_only_tag(&pkthdr, "DUT", TRUE); /* Open DUT tag */
	vis_xml_add_tag_value(&pkthdr, "MAC", dutmac);
	vis_xml_add_tag_value(&pkthdr, "STAMAC", stamac);
	vis_xml_add_only_tag(&pkthdr, "DUT", FALSE); /* Close DUT Tag */
	if (dutset.str)
		vis_xml_add_only_value(&pkthdr, dutset.str);
	vis_xml_add_only_tag(&pkthdr, "PacketVersion", FALSE); /* Clsoe PacketVersion Tag */
	if (pkthdr.str)
		vis_xml_add_only_value(xmlstring, pkthdr.str);

vis_fun_end:
	/* Free the XML data buffers */
	vis_xml_free_data_buffer(&configxml);
	vis_xml_free_data_buffer(&graphxml);
	vis_xml_free_data_buffer(&dutset);
	vis_xml_free_data_buffer(&pkthdr);
}

/* Read query from stream in json format treate it and create answer string
 * The query is a list of commands and treated by 'do_json_command'
 */
void
vis_do_json_set(const char *url, FILE *stream, int len, const char *boundary)
{
	vis_xml_strings_t xmlstring = {0};

	/* create the xml string out of the query string */
	create_xml_string(url, &xmlstring);

	/* send this xml string to the backend via socket */
	if (xmlstring.str != NULL) {
		send_ui_frontend_data(xmlstring.str);
		vis_xml_free_data_buffer(&xmlstring);
	}
}

/*
 * Create XML and send it to dcon to get the DB file and
 * then send it to Web UI
 */
void
vis_do_visdbdwnld_cgi(char *url, FILE *stream)
{
	vis_xml_strings_t xmlstring = {0};
	char tmpurl[] = "json.cgi?Req=GetDBFile";
	int ret = 0;

	/* create the xml string out of the query string */
	create_xml_string(tmpurl, &xmlstring);
	if (xmlstring.str != NULL) {
		ret = send_ui_frontend_data(xmlstring.str);
		if ((ret > 0) && (vis_rdata != NULL))
			fwrite(vis_rdata, ret, 1, stream);

		if (vis_rdata != NULL) {
			free(vis_rdata);
			vis_rdata = NULL;
		}
		vis_xml_free_data_buffer(&xmlstring);
	}

	websDone(stream, 200);
}

#endif /* __CONFIG_VISUALIZATION__ */
