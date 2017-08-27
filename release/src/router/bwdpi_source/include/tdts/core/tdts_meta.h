/*
 * Copyright (C)2014. Trend Micro Incorporated. All Rights Reserved. This
 * source code is the property of Trend Micro Incorporated and is protected by
 * intellectual property laws and international treaty provisions. No parties
 * may use, copy, distribute, sublicense, transfer, sell, rent, lease, modify,
 * adapt, translate or create derivative works of, reverse engineer, decompile
 * or disassemble, or attempt to reconstruct the source code without the express
 * written authorization of Trend Micro Incorporated. Any unauthorized use,
 * reproduction or distribution is subject to civil and criminal penalties.
 *
 * THIS SOURCE CODE AND RELATED INFORMATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT OF THIRD
 * PARTY RIGHTS AND/OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 * TREND MICRO, ITS AFFILIATES AND SUBSIDIARIES, AND THEIR RESPECTIVE PARTNERS
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS
 * SOURCE CODE AND RELATED INFORMATION, EVEN IF TREND MICRO HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE AND REGARDLESS OF THE FORM OF ACTION.
 *
 * This copyright notice must not be removed from the source code.
 */

#ifndef __TDTS_META_H__
#define __TDTS_META_H__

enum
{
	TDTS_META_TYPE_BNDWTH = 1,
	TDTS_META_TYPE_PAID_CONN,
	TDTS_META_TYPE_URL,
	TDTS_META_TYPE_UPNP,
	TDTS_META_TYPE_DNS_REQ,
	TDTS_META_TYPE_DNS_REPLY,
	TDTS_META_TYPE_DEVID_UN,	/*unknown devid id */

	/* File extraction */
	TDTS_META_TYPE_FTP_FILE,
	TDTS_META_TYPE_POP3_MIME_FILE,	/* POP3 file after MIME decode */
	TDTS_META_TYPE_POP3_FILE,
	TDTS_META_TYPE_SMTP_MIME_FILE,	/* SMTP file after MIME decode */
	TDTS_META_TYPE_SMTP_FILE,
	TDTS_META_TYPE_HTTP_MIME_FILE,	/* HTTP file after MIME decode */
	TDTS_META_TYPE_HTTP_FILE,
	TDTS_META_TYPE_IMAP4_MIME_FILE,	/* IMAP4 file after MIME decode */
	TDTS_META_TYPE_IMAP4_FILE,

	TDTS_META_TYPE_TLS_SERVER_NAME,
	/* TDTS_META_TYPE_TLS_CERTIFICATE, */
	TDTS_META_TYPE_MAX,
};

typedef int (*meta_extract)(char *, unsigned int, tdts_pkt_parameter_t *);


extern int meta_cb_register(meta_extract cb, int type);
extern int meta_cb_unregister(meta_extract cb, int type);

/* Return type TDTS_META_TYPE_BNDWTH */
enum
{
	TDTS_META_STREAM_TYPE_VOIP = 1,
	TDTS_META_STREAM_TYPE_AUDIO = 2,
	TDTS_META_STREAM_TYPE_VIDEO = 4,
};

/* Return data structure of TDTS_META_TYPE_BNDWTH */
typedef struct tdts_meta_bndwth_info
{
	unsigned char stm_type;
	unsigned short quality;
	unsigned short bndwth;

#if TMCFG_E_CORE_RULE_FORMAT_V2
	unsigned int bndwth_info_id;
#else
	unsigned char *description;
#endif

} tdts_meta_bndwth_info_t;

/* Return data structure of TDTS_META_TYPE_PAID_CONN */
typedef struct tdts_meta_paid_info
{
	/* 1 for ture */
	unsigned char paid;

} tdts_meta_paid_info_t;

/* Return data structure of TDTS_META_TYPE_URL */
typedef struct tdts_meta_url_info
{
	/* Domain */
	unsigned char *domain;
	unsigned int domain_len;

	/* Path */
	unsigned char *path;
	unsigned int path_len;

	/* Referer */
	unsigned char *referer;
	unsigned int referer_len;

} tdts_meta_url_info_t;

/* Return data structure of TDTS_META_TYPE_UPNP */
typedef struct tdts_meta_upnp_info
{
	/* Location */
	unsigned char *location;
	unsigned int loc_len;

} tdts_meta_upnp_info_t;

/* Return data structure of TDTS_META_TYPE_DNS_REQ */
typedef struct tdts_meta_dns_req_info
{
	unsigned char *domain_name;
	unsigned int domain_name_len;

} tdts_meta_dns_req_info_t;

/* Return data structure of TDTS_META_TYPE_DNS_REPLY */
typedef struct tdts_meta_dns_ans_info
{
	unsigned int ttl;
	unsigned char ip_ver;
	unsigned char ip[16];

} tdts_meta_dns_ans_info_t;

typedef struct tdts_meta_dns_reply_info
{
	unsigned short quest_len;
	unsigned char *quest_name;
	
	/* answer count */
	unsigned char ans_cnt;
	tdts_meta_dns_ans_info_t *answer;

} tdts_meta_dns_reply_info_t;

/* Return data structure of TDTS_META_TYPE_DEVID_UN */
enum
{
	TDTS_META_DEVID_UN_TYPE_BOOTP = 1,
	TDTS_META_DEVID_UN_TYPE_HTTP_UA,
};

/* Same as bootp_hdr_opt_t */
typedef struct tdts_meta_bootp_hdr_opt
{
	uint8_t code;
	uint8_t len;
	uint8_t value[0];
} __attribute__((packed)) tdts_meta_bootp_hdr_opt_t;

typedef struct tdts_meta_bootp_un_data
{
	/* ether hdr */
	unsigned char *etherhdr;
	/* IPv4 or IPv6 type */
	unsigned char *iphdr;
	/* bootp hdr */
	unsigned char *bootphdr;

	/* raw data size */
	unsigned int siz;
#define MAX_BOOTP_UN_OPT		8
#define MAX_BOOTP_OPT_VAL_SIZ		64
#define MAX_BOOTP_OPT_SIZ		\
		(MAX_BOOTP_UN_OPT * (sizeof(tdts_meta_bootp_hdr_opt_t) + MAX_BOOTP_OPT_VAL_SIZ))
	/* BOOTP hdr option (MAX_BOOTP_UN_OPT) */
	unsigned char raw[MAX_BOOTP_OPT_SIZ];

} tdts_meta_bootp_un_data_t;

typedef struct tdts_meta_http_ua_un_data
{
	unsigned int ua_len;
	unsigned char *ua;

} tdts_meta_http_ua_un_data_t;

typedef struct tdts_meta_devid_un
{
	unsigned char type;

	union
	{
		tdts_meta_bootp_un_data_t bootp;
		tdts_meta_http_ua_un_data_t ua;
	};

} tdts_meta_devid_un_t;

/* Return data structure of TDTS_META_TYPE file extraction */

/* Using in flag of the tdts_meta_file_t */
#define TDTS_META_FLAG_FILE_START	0x0001
#define TDTS_META_FLAG_FILE_CONTINUE	0x0002
#define TDTS_META_FLAG_FILE_END		0x0004

typedef struct tdts_meta_extract_file
{
	/* file header */
	unsigned int file_name_len;
	unsigned char *file_name;

	/* For decoder to save file id to identify.
	 * This is because multiple file may use 
	 * the same connection or multiple connection
	 * may send parts of file for acceleration */
	unsigned int file_id;
	unsigned int file_len;
	unsigned int proto_type;
	unsigned short flag;

	/* file content */
	unsigned int content_len;
	unsigned char *content;

} tdts_meta_extract_file_t;

/* the structure for mapping TDTS_META_TYPE_TLS_SERVER_NAME */
typedef struct tls_server_name_indication_st
{
	unsigned short	list_len;
	unsigned char	name_type;
	unsigned short	name_len;
	unsigned char	name[0];
}__attribute__((packed))tls_server_name_indication_t;

#endif
