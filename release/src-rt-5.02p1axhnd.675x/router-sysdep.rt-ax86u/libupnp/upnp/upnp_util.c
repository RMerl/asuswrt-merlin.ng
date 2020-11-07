/*
 * Broadcom UPnP library utilities
 *
 * Copyright 2019 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: upnp_util.c 738255 2017-12-27 22:36:47Z $
 */
#include <upnp.h>
#include <errno.h>

static const char cb64[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[] =
	"|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
 * Base64 block encoding,
 * encode 3 8-bit binary bytes as 4 '6-bit' characters
 */
void
upnp_base64_encode_block(unsigned char in[3], unsigned char out[4], int len)
{
	switch (len) {
	case 3:
		out[0] = cb64[ in[0] >> 2 ];
		out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
		out[2] = cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ];
		out[3] = cb64[ in[2] & 0x3f ];
		break;
	case 2:
		out[0] = cb64[ in[0] >> 2 ];
		out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
		out[2] = cb64[ ((in[1] & 0x0f) << 2) ];
		out[3] = (unsigned char) '=';
		break;
	case 1:
		out[0] = cb64[ in[0] >> 2 ];
		out[1] = cb64[ ((in[0] & 0x03) << 4)  ];
		out[2] = (unsigned char) '=';
		out[3] = (unsigned char) '=';
		break;
	default:
		break;
		/* do nothing */
	}
}

/*
 * Base64 encode a stream adding padding and line breaks as per spec.
 * input	- stream to encode
 * inputlen	- length of the input stream
 * target	- stream encoded with null ended.
 *
 * Returns The length of the encoded stream.
 */
int
upnp_base64_encode(unsigned char* input, const int inputlen, unsigned char *target)
{
	unsigned char *out;
	unsigned char *in;

	out = target;
	in  = input;

	if (input == NULL || inputlen == 0)
		return 0;

	while ((in+3) <= (input+inputlen)) {
		upnp_base64_encode_block(in, out, 3);
		in += 3;
		out += 4;
	}

	if ((input+inputlen) - in == 1) {
		upnp_base64_encode_block(in, out, 1);
		out += 4;
	}
	else {
		if ((input+inputlen)-in == 2) {
			upnp_base64_encode_block(in, out, 2);
			out += 4;
		}
	}

	*out = 0;
	return (int)(out - target);
}

/*
 * Base64 block encoding,
 * Decode 4 '6-bit' characters into 3 8-bit binary bytes
 */
void
upnp_decode_block(unsigned char in[4], unsigned char out[3])
{
	out[0] = (unsigned char)(in[0] << 2 | in[1] >> 4);
	out[1] = (unsigned char)(in[1] << 4 | in[2] >> 2);
	out[2] = (unsigned char)(((in[2] << 6) & 0xc0) | in[3]);
}

/*
 * Decode a base64 encoded stream discarding padding, line breaks and noise.
 * input 	- stream to decode
 * inputlen	- length of the input stream
 * target	- stream decoded with null ended.
 *
 * Returns The length of the decoded stream.
 */
int
upnp_base64_decode(unsigned char *input, const int inputlen, unsigned char *target)
{
	unsigned char* inptr;
	unsigned char* out;
	unsigned char v;
	unsigned char in[4];
	int i, len;

	if (input == NULL || inputlen == 0)
		return 0;

	out = target;
	inptr = input;

	while (inptr <= (input+inputlen)) {
		for (len = 0, i = 0; i < 4 && inptr <= (input+inputlen); i++) {
			v = 0;
			while (inptr <= (input+inputlen) && v == 0) {

				v = (unsigned char) *inptr;
				inptr++;

				v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
				if (v) {
					v = (unsigned char) ((v == '$') ? 0 : v - 61);
				}
			}

			if (inptr <= (input+inputlen)) {
				len++;

				if (v) {
					in[ i ] = (unsigned char) (v - 1);
				}
			}
			else {
				in[i] = 0;
			}
		}

		if (len) {
			upnp_decode_block(in, out);
			out += len-1;
		}
	}

	*out = 0;
	return (int)(out - target);
}

/* init/deinit tlv structure */
void
upnp_tlv_init(UPNP_TLV *tlv, int type)
{
	memset(tlv, 0, sizeof(*tlv));

	tlv->type = type;

	switch (type) {
	case UPNP_TYPE_BIN_BASE64:
	case UPNP_TYPE_STR:
		break;
	default:
		tlv->text = tlv->buf;
		break;
	}
	return;
}

void
upnp_tlv_deinit(UPNP_TLV *tlv)
{
	switch (tlv->type) {
	case UPNP_TYPE_STR:
		if (tlv->val.str && tlv->val.str != tlv->buf)
			free(tlv->val.str);

		tlv->val.str = tlv->text = 0;
		return;

	case UPNP_TYPE_BIN_BASE64:
		if (tlv->val.bin) {
			free(tlv->val.bin);
			tlv->val.bin = 0;
		}
		if (tlv->text) {
			free(tlv->text);
			tlv->text = 0;
		}
		tlv->len = 0;
		return;

	case UPNP_TYPE_BOOL:
	case UPNP_TYPE_UI1:
	case UPNP_TYPE_UI2:
	case UPNP_TYPE_UI4:
	case UPNP_TYPE_I1:
	case UPNP_TYPE_I2:
	case UPNP_TYPE_I4:
	default:
		break;
	}

	return;
}

/* Translate tlv to string according to data type */
char *
upnp_tlv_translate(UPNP_TLV *tlv)
{
	char *buf = NULL;

	/* Use embeded buf by default */
	if (tlv->type != UPNP_TYPE_STR && tlv->type != UPNP_TYPE_BIN_BASE64)
		buf = tlv->text = tlv->buf;

	switch (tlv->type) {
	case UPNP_TYPE_STR:
		tlv->text = tlv->val.str;
		break;

	case UPNP_TYPE_BIN_BASE64:
		if (tlv->text) {
			free(tlv->text);
			tlv->text = NULL;
		}

		if (tlv->len == 0) {
			tlv->text = NULL;
			break;
		}

		tlv->text = (char *)malloc(2 * tlv->len + 8);
		if (tlv->text == NULL)
			break;

		upnp_base64_encode((unsigned char *)tlv->val.bin, tlv->len,
			(unsigned char *)tlv->text);
		break;

	case UPNP_TYPE_BOOL:
		tlv->val.ival = (tlv->val.ival ? 1 : 0);
		snprintf(buf, sizeof(tlv->buf), "%d", (char)tlv->val.ival);
		break;

	case UPNP_TYPE_I1:
		tlv->val.ival &= 0xff;
		snprintf(buf, sizeof(tlv->buf), "%d", (char)tlv->val.ival);
		break;

	case UPNP_TYPE_I2:
		tlv->val.ival &= 0xffff;
		snprintf(buf, sizeof(tlv->buf), "%d", (short)tlv->val.ival);
		break;

	case UPNP_TYPE_I4:
		snprintf(buf, sizeof(tlv->buf), "%ld", tlv->val.ival);
		break;

	case UPNP_TYPE_UI1:
		tlv->val.uval &= 0xff;
		snprintf(buf, sizeof(tlv->buf), "%u", (unsigned char)tlv->val.uval);
		break;

	case UPNP_TYPE_UI2:
		tlv->val.uval &= 0xffff;
		snprintf(buf, sizeof(tlv->buf), "%u", (unsigned short)tlv->val.uval);
		break;

	case UPNP_TYPE_UI4:
		snprintf(buf, sizeof(tlv->buf), "%lu", tlv->val.uval);
		break;

	default:
		/* should not be reached */
		*buf = '\0';
		break;
	}

	/* Setup text strlen for sending out */
	if (tlv->text)
		tlv->text_strlen = strlen(tlv->text);
	else
		tlv->text_strlen = 0;

	return tlv->text;
}

static int
upnp_tlv_set_str(UPNP_TLV *tlv, char *str)
{
	int len;

	if (!str)
		return SOAP_ARGUMENT_VALUE_INVALID;

	/* Align to 16 */
	len = strlen(str) + 1;

	/* Free previous print buf */
	upnp_tlv_deinit(tlv);

	/* Use embeded buffer if possible  */
	if (len <= sizeof(tlv->buf))
		tlv->val.str = tlv->text = tlv->buf;
	else {
		tlv->val.str = tlv->text = (char *)malloc(len);
		if (tlv->val.str == NULL)
			return SOAP_OUT_OF_MEMORY;
	}

	/* Do copy */
	tlv->len = len;
	memcpy(tlv->val.str, str, len);
	return 0;
}

/* Convert tlv from string according to data type */
int
upnp_tlv_convert(UPNP_TLV *tlv, char *text_str)
{
	int len;
	int ival;
	unsigned int uval;

	switch (tlv->type) {
	case UPNP_TYPE_STR:
		return upnp_tlv_set_str(tlv, text_str);

	case UPNP_TYPE_BIN_BASE64:
		len = strlen(text_str);
		/* Free previous one */
		if (tlv->val.bin)
			free(tlv->val.bin);
		tlv->val.bin = (char *)malloc(len + 8);	/* Round off and null end decode */
		if (tlv->val.bin == NULL)
			return SOAP_OUT_OF_MEMORY;

		/* Do base64 decode */
		tlv->len = upnp_base64_decode((unsigned char *)text_str, len,
			(unsigned char *)tlv->val.bin);
		if (tlv->len <= 0)
			return SOAP_ARGUMENT_VALUE_INVALID;
		break;

	case UPNP_TYPE_BOOL:
		/* 0, false, no for false; 1, true, yes for true */
		if (strcmp(text_str, "0") == 0 ||
			strcmp(text_str, "false") == 0 ||
			strcmp(text_str, "no") == 0) {

			tlv->val.ival = 0;
		}
		else if (strcmp(text_str, "1") == 0 ||
			strcmp(text_str, "true") == 0 ||
			strcmp(text_str, "yes") == 0) {
			tlv->val.ival = 1;
		}
		else {
			return SOAP_ARGUMENT_VALUE_INVALID;
		}

		tlv->len = 1;
		break;

	case UPNP_TYPE_I1:
		ival = atoi(text_str);
		if ((ival & 0xffffff00) != 0)
			return SOAP_ARGUMENT_VALUE_INVALID;

		tlv->val.ival = ival;
		tlv->len = 1;
		break;

	case UPNP_TYPE_I2:
		ival = atoi(text_str);
		if ((ival & 0xffff0000) != 0)
			return SOAP_ARGUMENT_VALUE_INVALID;

		tlv->val.ival = ival;
		tlv->len = 2;
		break;

	case UPNP_TYPE_I4:
		ival = atoi(text_str);
		tlv->val.ival = ival;
		tlv->len = 4;
		break;

	case UPNP_TYPE_UI1:
		uval = strtoul(text_str, NULL, 10);
		if (uval > 0xff)
			return SOAP_ARGUMENT_VALUE_INVALID;

		tlv->val.uval = uval;
		tlv->len = 1;
		break;

	case UPNP_TYPE_UI2:
		uval = strtoul(text_str, NULL, 10);
		if (uval > 0xffff)
			return SOAP_ARGUMENT_VALUE_INVALID;

		tlv->val.uval = uval;
		tlv->len = 2;
		break;

	case UPNP_TYPE_UI4:
		uval = strtoul(text_str, NULL, 10);

		tlv->val.uval = uval;
		tlv->len = 4;
		break;

	default:
		/* Not supported yet */
		return SOAP_ARGUMENT_VALUE_INVALID;
	}

	return 0;
}

int
upnp_tlv_set_bin(UPNP_TLV *tlv, uintptr_t data, int len)
{
	switch (tlv->type) {
	case UPNP_TYPE_BIN_BASE64:
		if (!data)
			return -1;

		upnp_tlv_deinit(tlv);

		tlv->val.bin = (char *)malloc(len);
		if (tlv->val.bin == NULL)
			return -1;

		memcpy(tlv->val.bin, (char *)data, len);
		tlv->len = len;
		break;

	default:
		break;
	}

	return 0;
}

/* Set tlv */
int
upnp_tlv_set(UPNP_TLV *tlv, uintptr_t data)
{
	/* Specially take care of STR and BIN */
	switch (tlv->type) {
	case UPNP_TYPE_STR:
		return upnp_tlv_set_str(tlv, (char *)data);

	case UPNP_TYPE_BOOL:
		tlv->len = 1;
		tlv->val.ival = data ? 1 : 0;
		break;

	case UPNP_TYPE_I1:
		tlv->len = 1;
		tlv->val.ival = ((char)data & 0xff);
		break;

	case UPNP_TYPE_I2:
		tlv->len = 2;
		tlv->val.ival = ((short)data & 0xffff);
		break;

	case UPNP_TYPE_I4:
		tlv->len = 4;
		tlv->val.ival = (long)data;
		break;

	case UPNP_TYPE_UI1:
		tlv->len = 1;
		tlv->val.uval = ((unsigned char)data & 0xff);
		break;

	case UPNP_TYPE_UI2:
		tlv->len = 2;
		tlv->val.uval = ((unsigned short)data & 0xffff);
		break;

	case UPNP_TYPE_UI4:
		tlv->len = 4;
		tlv->val.uval = (unsigned long)data;
		break;

	default:
		break;
	}

	return 0;
}

/*
 * Search the service table by the target control URL
 */
UPNP_SERVICE *
upnp_get_service_by_control_url(UPNP_CONTEXT *context, char *control_url)
{
	UPNP_SERVICE	*service;

	for (service = context->focus_ifp->device->service_table;
		service && service->control_url;
		service++) {
		if (strcmp(control_url, service->control_url) == 0) {
			return service;
		}
	}

	return NULL;
}

/*
 * Search the service table by the target event URL
 */
UPNP_SERVICE *
upnp_get_service_by_event_url(UPNP_CONTEXT *context, char *event_url)
{
	UPNP_SERVICE	*service;

	/*
	 * Loop for all the UPnP device, and find out the
	 * UPnP service matches the event_url.
	 */
	for (service = context->focus_ifp->device->service_table;
		service && service->event_url;
		service++) {
		if (strcmp(service->event_url, event_url) == 0) {
			return service;
		}
	}

	return 0;
}

/*
 * Search the service table by service name
 */
UPNP_SERVICE *
upnp_get_service_by_name(UPNP_CONTEXT *context, char *name)
{
	UPNP_SERVICE	*service;

	/*
	 * Loop for all the UPnP device, and find out the
	 * UPnP service matches the event_url.
	 */
	for (service = context->focus_ifp->device->service_table;
	     service && service->name;
	     service++) {
		if (strcmp(service->name, name) == 0) {
			return service;
		}
	}

	return 0;
}

UPNP_ADVERTISE *
upnp_get_advertise_by_name(UPNP_CONTEXT *context, char *name)
{
	UPNP_ADVERTISE	*advertise;

	for (advertise = context->focus_ifp->advlist;
	     advertise && advertise->name;
	     advertise++) {
		if (strcmp(name, advertise->name) == 0) {
			return advertise;
		}
	}

	return NULL;
}

/*
 * Search input argument list for a arguement
 * and return its tlv.
 */
UPNP_TLV *
upnp_get_in_tlv(UPNP_CONTEXT *context, char *arg_name)
{
	int i;
	IN_ARGUMENT *temp = NULL;

	for (i = 0; i < context->in_arg_num; i++) {
		temp = context->in_args + i;
		if (strcmp(temp->name, arg_name) == 0)
			return &temp->tlv;
	}

	return NULL;
}

/*
 * Search output argument list for a arguement
 * and return its tlv.
 */
UPNP_TLV *
upnp_get_out_tlv(UPNP_CONTEXT *context, char *arg_name)
{
	int i;
	OUT_ARGUMENT *temp = NULL;

	for (i = 0; i < context->out_arg_num; i++) {
		temp = context->out_args + i;
		if (strcmp(temp->name, arg_name) == 0)
			return &temp->tlv;
	}

	return NULL;
}

/*
 *  Return the host address with the foramt "xxx.xxx.xxx.xxx:xxxxx",
 * such as, "192.168.0.1:1780". And if the port number is 80, only
 * return "192.168.0.1".
 */
void
upnp_host_addr(unsigned char *host_addr,
	struct in_addr ipaddr, unsigned short port)
{
	register unsigned long addr = ntohl(ipaddr.s_addr);

	snprintf((char *)host_addr, sizeof("255.255.255.255") + 1,
		"%lu.%lu.%lu.%lu",
		(addr >> 24) & 0xff,
		(addr >> 16) & 0xff,
		(addr >> 8) & 0xff,
		addr & 0xff);

	if (port != 80) {
		char myport[sizeof(":65535") + 1];

		snprintf(myport, sizeof(myport),  ":%d", port);
		strcat((char *)host_addr, myport);
	}

	return;
}

/* Get current GMT time */
int
upnp_gmt_time(char *time_str)
{
	struct tm btime;
	time_t curr_time;

	static char *day_name[] =
	{
		"Sun",  "Mon",  "Tue",  "Wed",  "Thu",  "Fri",  "Sat"
	};

	static char *mon_name[] =
	{
		"Jan",  "Feb",  "Mar",  "Apr",  "May",  "Jun",
		"Jul",  "Aug",  "Sep",  "Oct",  "Nov",  "Dec"
	};

	curr_time = time(0);
	gmtime_r(&curr_time, &btime);

	snprintf(time_str, TIME_BUF_LEN,
		"%.3s, %.2d %.3s %d %.2d:%.2d:%.2d GMT",
		day_name[btime.tm_wday],
		btime.tm_mday,
		mon_name[btime.tm_mon],
		1900 + btime.tm_year,
		btime.tm_hour,
		btime.tm_min,
		btime.tm_sec);

	return 0;
}
/* returns random uuid in uuid_buf */
int upnp_get_url_randomstring(char *randomstring)
{
	FILE *fp;
	int numread;

	fp = fopen(UPNP_RANDOM_STRING_FILE, "r");
	if (fp == NULL)	{
		return -1;
	}
	numread = fread(randomstring, 1, UPNP_URL_UUID_LEN, fp);
	if (numread > 0) {
		randomstring[numread] = '\0';
	} else {
		return -1;
	}
	return 0;
}
