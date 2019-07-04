/*
 * Broadcom Home Gateway Reference Design
 * Broadcom WiFi Insight Webpage functions
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: vis_gui.c 661776 2016-09-27 09:57:18Z $
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

/* IOCtl version read from targeted driver */
static int g_zwdioctl_version;
/* IOCTL swapping mode for Big Endian host with Little Endian dongle.  Default to off */
/* The below macros handle endian mis-matches between wl utility and wl driver. */
static bool g_swap = FALSE;
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif /* IFNAMSIZ */

#define sys_restart() kill(1, SIGHUP)

/* Indicates the level of debug message to be printed on the console */
static int vis_debug_level = 0;

/* Header to download DB file */
char g_vis_download_db_hdr[] =
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Expires: 0\r\n"
"Content-Type: application/download\r\n"
"Content-Disposition: attachment ; filename=visdata.db"
;

#define VIS_REQ_NVRAM_GET	"VisGetNVRAM"
#define VIS_REQ_NVRAM_SET	"VisSetNVRAM"

#define VIS_REQ_ZWD		"VisReqZWD"
#define VIS_REQ_ZWD_ISSUPPORTED	"VisZWDIsSupported"
#define VIS_REQ_ZWD_GET_DFSCHANNEL	"VisZWDGetDFSChannels"
#define VIS_REQ_ZWD_GET_AP_STATUS	"VisZWDGetApStatus"
#define VIS_REQ_ZWD_MOVE_TO_DFS		"VisZWDMoveToDFS"
#define VIS_REQ_ZWD_SIMULATE_RADAR	"VisZWDSimulateRadar"

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
#define VIS_MAX_BAND_LEN		2
#define VIS_MAX_IP_LEN			16
#define VIS_MAX_NVRAM_LEN		100

#define VIS_ZWD_MAX_NRATE		256
#define VIS_ZWD_MAX_RATE		32
#define MILLISECONDS_SECONDS(x)		((x) * 0.001)
#define VIS_ZWD_OPMODE_MASK		0xFF
#define VIS_ZWD_OPMODE_RXNSS_MASK	0x70
#define VIS_ZWD_OPMODE_RXNSS_SHIFT	4

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

/* Zero wait dfs request arguments */
typedef struct vis_zwd_req_t_ {
	char wlname[IFNAMSIZ];
	char req[VIS_MAX_REQ_LEN];
	char chanbuf[CHANSPEC_STR_LEN];
	int core;
} vis_zwd_req_t;

/* Zero wait dfs primary or secondary core output */
typedef struct vis_zwd_core_status_t_ {
	chanspec_t channel;
	uint32 txchain;
	uint32 rxchain;
	uint32 elapsed;
	char phyratebuf[VIS_ZWD_MAX_RATE];
	char nratebuf[VIS_ZWD_MAX_NRATE];
} vis_zwd_core_status_t;

/* Zero wait dfs AP status */
typedef struct vis_zwd_ap_status_t_ {
	int isprocessing;
	vis_zwd_core_status_t primary;
	vis_zwd_core_status_t scancore;
} vis_zwd_ap_status_t;

/* dword align allocation */
static union {
	char bufdata[WLC_IOCTL_MAXLEN];
	uint32 alignme;
} bufstruct_wlu;
static char *g_vis_iovar_buf = (char*) &bufstruct_wlu.bufdata;

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
	VIS_ZWD_TYPE,
	VIS_ZWD_WLNAME,
	VIS_ZWD_CHANNEL,
	VIS_ZWD_CORE,
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
	{VIS_ZWD_TYPE, "ZWDType"},
	{VIS_ZWD_WLNAME, "wlname"},
	{VIS_ZWD_CHANNEL, "Channel"},
	{VIS_ZWD_CORE, "Core"}
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
		nbytes = read(sockfd, &(read_buf[totalread]), size);

		if (nbytes <= 0) {
			VIS_WARNING("Failed to read data error : %s\n", strerror(errno));
			return INVALID_SOCKET;
		}

		totalread += nbytes;
	}

	read_buf[totalread] = '\0';

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

/* For getting ioctl version and g_swap value */
static int
vis_zwd_wl_check(void *wl)
{
	int ret, val;

	if ((ret = wl_ioctl(wl, WLC_GET_MAGIC, &val, sizeof(int))) < 0) {
		VIS_DEBUG("ZWD : %s WLC_GET_MAGIC failed. Err : %d\n", (char*)wl, ret);
		return ret;
	}

	/* Detect if IOCTL swapping is necessary */
	if (val == (int)bcmswap32(WLC_IOCTL_MAGIC))
	{
		val = bcmswap32(val);
		g_swap = TRUE;
	}
	if (val != WLC_IOCTL_MAGIC)
		return -1;
	if ((ret = wl_ioctl(wl, WLC_GET_VERSION, &val, sizeof(int))) < 0) {
		VIS_DEBUG("ZWD : %s WLC_GET_VERSION failed. Err : %d\n", (char*)wl, ret);
		return ret;
	}
	g_zwdioctl_version = dtoh32(val);

	return 0;
}

/* Return a legacy chanspec given a new chanspec
 * Returns INVCHANSPEC on error
 */
static chanspec_t
wl_chspec_to_legacy(chanspec_t chspec)
{
	chanspec_t lchspec;

	if (wf_chspec_malformed(chspec)) {
		VIS_ERROR("wl_chspec_to_legacy: input chanspec (0x%04X) malformed\n",
			chspec);
		return INVCHANSPEC;
	}

	/* get the channel number */
	lchspec = CHSPEC_CHANNEL(chspec);

	/* convert the band */
	if (CHSPEC_IS2G(chspec)) {
		lchspec |= WL_LCHANSPEC_BAND_2G;
	} else {
		lchspec |= WL_LCHANSPEC_BAND_5G;
	}

	/* convert the bw and sideband */
	if (CHSPEC_IS20(chspec)) {
		lchspec |= WL_LCHANSPEC_BW_20;
		lchspec |= WL_LCHANSPEC_CTL_SB_NONE;
	} else if (CHSPEC_IS40(chspec)) {
		lchspec |= WL_LCHANSPEC_BW_40;
		if (CHSPEC_CTL_SB(chspec) == WL_CHANSPEC_CTL_SB_L) {
			lchspec |= WL_LCHANSPEC_CTL_SB_LOWER;
		} else {
			lchspec |= WL_LCHANSPEC_CTL_SB_UPPER;
		}
	} else {
		/* cannot express the bandwidth */
		char chanbuf[CHANSPEC_STR_LEN];
		VIS_ERROR("wl_chspec_to_legacy: unable to convert chanspec %s (0x%04X) "
			"to pre-11ac format\n",
			wf_chspec_ntoa(chspec, chanbuf), chspec);
		return INVCHANSPEC;
	}

	return lchspec;
}

/* given a chanspec value, do the endian and chanspec version conversion to
 * a chanspec_t value in a 32 bit integer
 * Returns INVCHANSPEC on error
 */
static uint32
wl_chspec32_to_driver(chanspec_t chanspec)
{
	uint32 val;

	if (g_zwdioctl_version == 1) {
		chanspec = wl_chspec_to_legacy(chanspec);
		if (chanspec == INVCHANSPEC) {
			return chanspec;
		}
	}
	val = htod32((uint32)chanspec);

	return val;
}

/* Replace special characters from string. Some special characters are replaced by some hexadecimal
 * values from web UI input. This function to remove those
 */
static void
vis_zwd_replace_special_characters(char *str, char replacechar)
{
	int i, j = 0;

	for (i = 0; i < strlen(str); i++, j++) {
		if (str[i] == '%') {
			/* One special character is replaced with 3 characters starting
			 * from %. For example  / will come as %2F
			 */
			i += 2;
			str[j] = replacechar;
		}
		else
			str[j] = str[i];
	}
	str[j] = '\0';
}

/* Allocated global vis_rdata for storing the JSON object output */
static int
vis_zwd_allocate_output_buf(char *wlname)
{
	vis_rdata = (unsigned char*)malloc(sizeof(char) * MAX_READ_BUFFER);
	if (vis_rdata == NULL) {
		VIS_ERROR("ZWD: %s :Failed to allocate memory for vis_rdata\n",
			wlname);
		return -1;
	}

	return 0;
}

/* Get intiger value from driver */
static int
vis_zwd_get_iovar(void *wlname, char *cmd, int *val, int len)
{
	int err = 0;

	if ((err = wl_iovar_get(wlname, cmd, val, len)) < 0) {
		VIS_DEBUG("ZWD : %s : Failed to get %s - err : %d\n", (char*)wlname, cmd, err);
		return err;
	}

	return 0;
}

/* This is to create JSON status response */
static void
vis_zwd_create_status_response(vis_zwd_req_t *zwdreq, int status)
{
	/* Create output buffer based on the result */
	if (vis_zwd_allocate_output_buf(zwdreq->wlname) < 0)
		return;

	snprintf((char*)vis_rdata, MAX_READ_BUFFER,
		"{\"Req\":\"%s\",\"Status\":\"%s\"}",
		zwdreq->req, ((status < 0)?"Failed":"Success"));
	VIS_DEBUG("ZWD : Response : %s\n", vis_rdata);
}

/* Get the chip number from the driver */
static int
vis_zwd_get_chipnum(void *wl)
{
	int err;
	wlc_rev_info_t revinfo;

	memset(&revinfo, 0, sizeof(revinfo));

	if ((err = wl_ioctl(wl, WLC_GET_REVINFO, &revinfo, sizeof(revinfo))) < 0) {
		VIS_ERROR("ZWD: Failed to get Chip Num for wl : %s\n", (char*)wl);
		return err;
	}

	return dtoh32(revinfo.chipnum);
}

/* This is to check whether any interface supports Zero wait dfs or not
 * This function checks all the 5G interfaces and checks its chipnumber
 * If the chipnumber is 4366. Then returns the interface name
 */
static int
vis_zwd_get_supported_interface(char *outname, int outnamesz, int *chipnum)
{
	char name[IFNAMSIZ], *next = NULL;
	char wl_name[IFNAMSIZ], os_name[IFNAMSIZ];
	int unit = -1;
	char ifnames[256];
	int bandtype, isvalidchip = 0;

	snprintf(ifnames, sizeof(ifnames), "%s %s %s",
		nvram_safe_get("lan_ifnames"),
		nvram_safe_get("wan_ifnames"),
		nvram_safe_get("lan1_ifnames"));

	if (!remove_dups(ifnames, sizeof(ifnames))) {
		return -1;
	}

	foreach(name, ifnames, next) {

		if (nvifname_to_osifname(name, os_name, sizeof(os_name)) < 0)
			continue;

		if (wl_probe(os_name) ||
			wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
			continue;

		/* Convert eth name to wl name */
		if (osifname_to_nvifname(name, wl_name, sizeof(wl_name)) != 0) {
			return -1;
		}

		/* Get configured phy type */
		if (wl_ioctl(name, WLC_GET_BAND, &bandtype, sizeof(bandtype)) < 0)
			continue;
		/* Only in 5G band */
		if (bandtype == WLC_BAND_5G) {
			*chipnum = vis_zwd_get_chipnum(name);
			VIS_DEBUG("ZWD : chip number : 0x%X wlname : %s\n", *chipnum, name);
			/* Zero wait dfs supported only in 4366 */
			isvalidchip = ((*chipnum == 0x4365) || (*chipnum == 0x4366) ||
				(*chipnum == 0x43664) || (*chipnum == 0x43666) ||
				(*chipnum == 0x43465) || (*chipnum == 0x43525));
			if (isvalidchip) {
				/* Store the wl name */
				snprintf(outname, outnamesz, "%s", name);
				outname[outnamesz-1] = '\0';
				return 0;
			}
		}
	}

	return -1;
}

/* To get only DFS channels(Radar) */
static int
vis_zwd_get_dfs_channels(vis_zwd_req_t *zwdreq, uint bw)
{
	uint bitmap;
	uint channel;
	uint32 chanspec_arg;
	int buflen, err, first, dfschidx = 0, bufidx = 0;
	char *param;
	uint dfschannels[MAXCHANNEL] = {0};

	/* Allocate memory for response buffer */
	if (vis_zwd_allocate_output_buf(zwdreq->wlname) < 0)
		return -1;

	bufidx = snprintf((char*)vis_rdata, MAX_READ_BUFFER,
			"{\"Req\":\"%s\",\"DFSChannels\":[ ", zwdreq->req);

	for (first = 0; first <= MAXCHANNEL; first++) {
		channel = first;
		chanspec_arg = CH20MHZ_CHSPEC(channel);

		strncpy(g_vis_iovar_buf, "per_chan_info", WLC_IOCTL_MAXLEN);
		buflen = strlen(g_vis_iovar_buf) + 1;
		param = (char *)(g_vis_iovar_buf + buflen);

		chanspec_arg = wl_chspec32_to_driver(chanspec_arg);
		memcpy(param, (char*)&chanspec_arg, sizeof(chanspec_arg));

		if ((err = wl_ioctl((void*)zwdreq->wlname, WLC_GET_VAR, g_vis_iovar_buf,
			WLC_IOCTL_MAXLEN)) < 0) {
			VIS_DEBUG("ZWD:wl : %s Failed to get Channel Info for channel %d\n",
				zwdreq->wlname, channel);
			goto end;
		}

		bitmap = dtoh32(*(uint *)g_vis_iovar_buf);
		if (!(bitmap & WL_CHAN_VALID_HW))
			continue;
		if (!(bitmap & WL_CHAN_VALID_SW))
			continue;
		/* If it is radar sensitive */
		if (bitmap & WL_CHAN_RADAR) {
			dfschannels[dfschidx++] = channel;
		}
	}
end:
	/* Now add the DFS channels to output buffer */
	for (first = 0; first < dfschidx; first++) {
		/* Now convert control channel to chanspec and then to string */
		chanspec_t chspec = 0;
		char chanbuf[CHANSPEC_STR_LEN];

		chspec = wf_channel2chspec(dfschannels[first], bw);
		bufidx += snprintf((char*)vis_rdata+bufidx, MAX_READ_BUFFER, "\"%s\",",
			wf_chspec_ntoa(chspec, chanbuf));
	}
	/* To Remove comma in the end */
	if (dfschidx > 0)
		vis_rdata[strlen((char*)vis_rdata) - 1] = '\0';
	strncat((char*)vis_rdata, "]}", MAX_READ_BUFFER);
	vis_rdata[MAX_READ_BUFFER - 1] = '\0';

	VIS_DEBUG("ZWD:vis_rdata : %s\n", vis_rdata);

	return 0;
}

/* Send radar signal on particular core */
static int
vis_zwd_simulate_radar(vis_zwd_req_t *zwdreq)
{
	int err = 0;

	if ((err = wl_iovar_setint((void*)zwdreq->wlname, "radar", zwdreq->core)) < 0) {
		VIS_ERROR("ZWD : %s : Failed simulate radar. Err : %d\n", zwdreq->wlname, err);
	}

	vis_zwd_create_status_response(zwdreq, err);

	return err;
}

/* Move to DFS channel */
static int
vis_zwd_move_to_dfs(vis_zwd_req_t *zwdreq)
{
	uint32 val = 0;
	chanspec_t chanspec = 0;
	int err = 0;

	/* Get the chanspec from the input */
	chanspec = wf_chspec_aton(zwdreq->chanbuf);
	val = wl_chspec32_to_driver(chanspec);
	if (val != INVCHANSPEC) {
		if ((err = wl_iovar_setint((void*)zwdreq->wlname, "dfs_ap_move", val)) < 0) {
			VIS_ERROR("ZWD : %s dfs_ap_move failed. Err %d\n", zwdreq->wlname, err);
		}
	} else {
		VIS_ERROR("ZWD : %s Invalid chanspec supplied\n", zwdreq->wlname);
		err = -1;
	}

	vis_zwd_create_status_response(zwdreq, err);

	return err;
}

/* Get the DFS status */
static int
vis_zwd_get_dfs_status(vis_zwd_req_t *zwdreq, vis_zwd_ap_status_t *apstatus)
{
	int err = 0;

	if ((err = wl_iovar_getbuf((void*)zwdreq->wlname, "dfs_status", NULL, 0,
		g_vis_iovar_buf, WLC_IOCTL_SMLEN)) < 0) {
		VIS_ERROR("ZWD: %s Failed to get dfs_status. Err : %d\n", zwdreq->wlname, err);
	} else {
		wl_dfs_status_t *dfs_status = (wl_dfs_status_t*)g_vis_iovar_buf;

		/* Convert the elapsed time to seconds from milli seconds */
		apstatus->scancore.elapsed = MILLISECONDS_SECONDS(dtoh32(dfs_status->duration));
	}

	return err;
}

/* Counts number of binary digits */
static unsigned int
bits_count(unsigned int n)
{
	unsigned int count = 0;

	while (n > 0) {
		if (n & 1)
			count++;
		n >>= 1;
	}

	return count;
}

/* Sets the cnt number of bits and returns the number */
static unsigned int
set_bits(unsigned int cnt)
{
	int i = 0, num = 0;

	for (i = 0; i < cnt; i++) {
		num |= 1 << i;
	}

	return num;
}

/* Formats the nrate output to buffer */
static void
vis_zwd_format_nrate_output(uint32 rspec, char *nratebuf, int nratebuf_len)
{
	const char * rspec_auto = "auto";
	uint encode, rate, txexp = 0, bw_val;
	const char* stbc = "";
	const char* ldpc = "";
	const char* sgi = "";
	const char* bw = "";
	int stf;

	if (rspec == 0) {
		encode = WL_RSPEC_ENCODE_RATE;
	} else if (g_zwdioctl_version == 1) {
		encode = (rspec & OLD_NRATE_MCS_INUSE) ? WL_RSPEC_ENCODE_HT : WL_RSPEC_ENCODE_RATE;
		stf = (int)((rspec & OLD_NRATE_STF_MASK) >> OLD_NRATE_STF_SHIFT);
		rate = (rspec & OLD_NRATE_RATE_MASK);

		if (rspec & OLD_NRATE_OVERRIDE) {
			if (rspec & OLD_NRATE_OVERRIDE_MCS_ONLY)
				rspec_auto = "fixed mcs only";
			else
				rspec_auto = "fixed";
		}
	} else {
		int siso;
		encode = (rspec & WL_RSPEC_ENCODING_MASK);
		rate = (rspec & WL_RSPEC_RATE_MASK);
		txexp = (rspec & WL_RSPEC_TXEXP_MASK) >> WL_RSPEC_TXEXP_SHIFT;
		stbc  = ((rspec & WL_RSPEC_STBC) != 0) ? " stbc" : "";
		ldpc  = ((rspec & WL_RSPEC_LDPC) != 0) ? " ldpc" : "";
		sgi   = ((rspec & WL_RSPEC_SGI)  != 0) ? " sgi"  : "";
		bw_val = (rspec & WL_RSPEC_BW_MASK);

		if (bw_val == WL_RSPEC_BW_20MHZ) {
			bw = "bw20";
		} else if (bw_val == WL_RSPEC_BW_40MHZ) {
			bw = "bw40";
		} else if (bw_val == WL_RSPEC_BW_80MHZ) {
			bw = "bw80";
		} else if (bw_val == WL_RSPEC_BW_160MHZ) {
			bw = "bw160";
		}

		/* initialize stf mode to an illegal value and
		 * fix to a backward compatable value if possible
		 */
		stf = -1;
		/* for stf calculation, determine if the rate is single stream.
		 * Legacy rates WL_RSPEC_ENCODE_RATE are single stream, and
		 * HT rates for mcs 0-7 are single stream
		 */
		siso = (encode == WL_RSPEC_ENCODE_RATE) ||
			((encode == WL_RSPEC_ENCODE_HT) && rate < 8);

		/* calc a value for nrate stf mode */
		if (txexp == 0) {
			if ((rspec & WL_RSPEC_STBC) && siso) {
				/* STF mode STBC */
				stf = OLD_NRATE_STF_STBC;
			} else {
				/* STF mode SISO or SDM */
				stf = (siso) ? OLD_NRATE_STF_SISO : OLD_NRATE_STF_SDM;
			}
		} else if (txexp == 1 && siso) {
			/* STF mode CDD */
			stf = OLD_NRATE_STF_CDD;
		}

		if (rspec & WL_RSPEC_OVERRIDE_RATE) {
			rspec_auto = "fixed";
		}
	}

	if (encode == WL_RSPEC_ENCODE_RATE) {
		if (rspec == 0) {
			snprintf(nratebuf, nratebuf_len, "auto");
		} else {
			snprintf(nratebuf, nratebuf_len,
				"legacy rate %d%s Mbps stf mode %d %s",
				rate/2, (rate % 2)?".5":"", stf, rspec_auto);
		}
	} else if (encode == WL_RSPEC_ENCODE_HT) {
		snprintf(nratebuf, nratebuf_len, "mcs index %d stf mode %d %s",
			rate, stf, rspec_auto);
	} else {
		uint vht = (rspec & WL_RSPEC_VHT_MCS_MASK);
		uint Nss = (rspec & WL_RSPEC_VHT_NSS_MASK) >> WL_RSPEC_VHT_NSS_SHIFT;
		snprintf(nratebuf, nratebuf_len,
			"vht mcs %d Nss %d Tx Exp %d %s%s%s%s %s",
			vht, Nss, txexp, bw, stbc, ldpc, sgi, rspec_auto);
	}
	nratebuf[nratebuf_len - 1] = '\0';
}

/* convert rate internal 500 Kbits/s units to string in Mbits/s format, like "11", "5.5" */
static char*
rate_int2string(int val, char *rate_buf, int rate_buf_len)
{
	if ((val == -1) || (val == 0))
		snprintf(rate_buf, rate_buf_len, "auto");
	else
		snprintf(rate_buf, rate_buf_len, "%d%s Mbps", (val / 2), (val & 1) ? ".5" : "");
	rate_buf[rate_buf_len-1] = '\0';

	return (rate_buf);
}

/* Get the wl rate */
static int
vis_zwd_get_rate(vis_zwd_req_t *zwdreq, vis_zwd_ap_status_t *apstatus)
{
	int err = 0, val = 0;

	if ((err = wl_ioctl(zwdreq->wlname, WLC_GET_RATE, &val, sizeof(int))) < 0) {
		VIS_DEBUG("ZWD : %s error getting rate. err : %d\n", zwdreq->wlname, err);
		return err;
	}
	val = dtoh32(val);
	rate_int2string(val, apstatus->primary.phyratebuf, sizeof(apstatus->primary.phyratebuf));

	return 0;
}

/* Get rxchain from oper_mode */
static int
vis_zwd_get_rxchain_from_oper_mode(void *wlname, int *rxchain)
{
	int tmpmode = 0, err = 0;
	uint16 oper_mode = 0x0, rxnss = 0x0;

	/* get the oper_mode */
	if ((err = vis_zwd_get_iovar(wlname, "oper_mode", &tmpmode, sizeof(int))) != 0)
		return err;

	/* In oper_mode first byte (MSB) - 0 (disable) / 1 (enable) and
	 * second byte (LSB) - is,
	 * B0 and B1 is channel width
	 * B2 and B3 is reserved
	 * B4 B5 and B6 is Rx NSS
	 * B7 is RX NSS type
	 */
	oper_mode = (uint16)(tmpmode & VIS_ZWD_OPMODE_MASK);
	/* Get rxNss from oper_mode */
	rxnss = (oper_mode & VIS_ZWD_OPMODE_RXNSS_MASK) >> VIS_ZWD_OPMODE_RXNSS_SHIFT;
	/* Actual number of chains is +1 of rxnss */
	rxnss++;
	/* Set number of bits */
	*rxchain = set_bits(rxnss);

	return 0;
}

/* Get the AP status from the driver. This intern calls all the IOVARs to get the status
 * such as channel, chains, rate and dfs status etc.
 */
static int
vis_zwd_get_ap_status(vis_zwd_req_t *zwdreq)
{
	vis_zwd_ap_status_t apstatus = {0};
	char chanbuf_pri[CHANSPEC_STR_LEN] = {0}, chanbuf_scan[CHANSPEC_STR_LEN] = {0};
	int phymode = 0;
	uint32 rspec = 0;

	/* get the txchain */
	vis_zwd_get_iovar(zwdreq->wlname, "txchain", &apstatus.primary.txchain, sizeof(uint32));
	/* get the rxchain */
	vis_zwd_get_iovar(zwdreq->wlname, "rxchain", &apstatus.primary.rxchain, sizeof(uint32));
	/* Get Chanspec */
	vis_zwd_get_iovar(zwdreq->wlname, "chanspec", &apstatus.primary.channel, sizeof(chanspec_t));
	/* Get DFS Status */
	vis_zwd_get_dfs_status(zwdreq, &apstatus);
	/* Get Phymode */
	vis_zwd_get_iovar(zwdreq->wlname, "phymode", &phymode, sizeof(int));
	if (phymode == 31) { /* DFS move processing */
		apstatus.isprocessing = 1;
		apstatus.scancore.rxchain = apstatus.scancore.txchain = 1;

		/* If the phymode is 31 means DFS move is happening,
		 * get the rxchain from opermode
		 */
		vis_zwd_get_rxchain_from_oper_mode(zwdreq->wlname, (int*)&apstatus.primary.rxchain);
		apstatus.primary.txchain = apstatus.primary.rxchain;
		/* Get sc_chan. i.e scan core channel */
		vis_zwd_get_iovar(zwdreq->wlname, "sc_chan", (&apstatus.scancore.channel, sizeof(chanspec_t));
	}

	/* Get Nrate buffer */
	if (vis_zwd_get_iovar(zwdreq->wlname, "nrate", &rspec, sizeof(uint32)) == 0) {
		vis_zwd_format_nrate_output(rspec, apstatus.primary.nratebuf,
			sizeof(apstatus.primary.nratebuf));
	}

	/* Get the phy rate */
	vis_zwd_get_rate(zwdreq, &apstatus);

	/* Allocate memory for response buffer */
	if (vis_zwd_allocate_output_buf(zwdreq->wlname) < 0)
		return -1;

	/* Create JSON object for ap status output */
	snprintf((char*)vis_rdata, MAX_READ_BUFFER,
			"{\"Req\":\"%s\",\"Status\":\"Success\",\"APStatus\":{"
			"\"PrimaryCore\":{"
			"\"Channel\":\"%s\",\"Chains\":\"%d X %d\","
			"\"PhyRate\":\"%s\",\"nrate\":\"%s\","
			"\"Elapsed\":%d},"
			"\"DFSProcessing\":%d,"
			"\"ScanCore\":{"
			"\"Channel\":\"%s\",\"Chains\":\"%d X %d\","
			"\"Elapsed\":%d}}}",
			zwdreq->req, wf_chspec_ntoa(apstatus.primary.channel, chanbuf_pri),
			bits_count(apstatus.primary.txchain),
			bits_count(apstatus.primary.rxchain),
			apstatus.primary.phyratebuf, apstatus.primary.nratebuf,
			apstatus.primary.elapsed, apstatus.isprocessing,
			wf_chspec_ntoa(apstatus.scancore.channel, chanbuf_scan),
			bits_count(apstatus.scancore.txchain),
			bits_count(apstatus.scancore.rxchain),
			apstatus.scancore.elapsed);
	vis_rdata[MAX_READ_BUFFER - 1] = '\0';
	VIS_DEBUG("ZWD: %s vis_rdata : %s\n", zwdreq->wlname, vis_rdata);

	return 0;
}

/* Zero wait DFS request operation. Calls specific function based on request */
static void
vis_do_zero_wait_dfs_op(vis_zwd_req_t *zwdreq)
{
	int ret = 0;

	if (strcmp(zwdreq->req, VIS_REQ_ZWD_GET_DFSCHANNEL) == 0) {
		chanspec_t chspec = 0;

		/* Get the current chanspec to get the bandwidth */
		if (vis_zwd_get_iovar(zwdreq->wlname, "chanspec", &chspec, sizeof(chanspec_t)) == 0)
			ret = vis_zwd_get_dfs_channels(zwdreq, CHSPEC_BW(chspec));
	} else if (strcmp(zwdreq->req, VIS_REQ_ZWD_ISSUPPORTED) == 0) {
		int chipnum = 0, issuport = 0;

		ret = vis_zwd_get_supported_interface(zwdreq->wlname, sizeof(zwdreq->wlname),
			&chipnum);
		if (ret == 0 && chipnum == 0x4365) {
			issuport = 1;
		}
		/* To get the ioctl version and g_swap variable */
		if (strlen(zwdreq->wlname) > 0) {
			if (vis_zwd_wl_check((void*)zwdreq->wlname) < 0) {
				VIS_DEBUG("ZWD: vis_zwd_wl_check() failed\n");
			}
		}

		if (vis_zwd_allocate_output_buf(zwdreq->wlname) < 0)
			return;
		snprintf((char*)vis_rdata, MAX_READ_BUFFER,
				"{\"Req\":\"%s\",\"Status\":\"Success\","
				"\"IsSupported\":\"%d\",\"wlname\":\"%s\"}",
				zwdreq->req, issuport, zwdreq->wlname);
		vis_rdata[MAX_READ_BUFFER - 1] = '\0';
		VIS_DEBUG("ZWD:Respone : %s\n", vis_rdata);
	} else if (strcmp(zwdreq->req, VIS_REQ_ZWD_GET_AP_STATUS) == 0) {
		ret = vis_zwd_get_ap_status(zwdreq);
	} else if (strcmp(zwdreq->req, VIS_REQ_ZWD_MOVE_TO_DFS) == 0) {
		/* From UI the / will come as %2F. So replace it with / */
		vis_zwd_replace_special_characters(zwdreq->chanbuf, '/');
		vis_zwd_move_to_dfs(zwdreq);
	} else if (strcmp(zwdreq->req, VIS_REQ_ZWD_SIMULATE_RADAR) == 0) {
		/* 2 is to simulate radar on primary core */
		ret = vis_zwd_simulate_radar(zwdreq);
	} else {
		VIS_DEBUG("Invalid request : %s\n", zwdreq->req);
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

	/* For zero wait dfs request arguments */
	vis_zwd_req_t zwdreq;

	memset(&zwdreq, 0, sizeof(zwdreq));

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
			case VIS_ZWD_TYPE: /* Type of Zero DFS Wait request */
				snprintf(zwdreq.req, sizeof(zwdreq.req), "%s", pch);
				break;
			case VIS_ZWD_WLNAME: /* Name of the interface */
				snprintf(zwdreq.wlname, sizeof(zwdreq.wlname), "%s", pch);
				break;
			case VIS_ZWD_CHANNEL: /* Channel for DFS AP Move ZWD */
				snprintf(zwdreq.chanbuf, sizeof(zwdreq.chanbuf), "%s", pch);
				break;
			case VIS_ZWD_CORE: /* Primary or secondary core for ZWD */
				zwdreq.core = atoi(pch);
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
	} else if (strcmp(request, VIS_REQ_ZWD) == 0) { /* If it is zero wait dfs request */
		vis_do_zero_wait_dfs_op(&zwdreq);
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
