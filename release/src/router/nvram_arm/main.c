/*
 * Frontend command-line utility for Linux NVRAM layer
 *
 * Copyright (C) 2012, Broadcom Corporation. All Rights Reserved.
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
 *
 * $Id: main.c 325698 2012-04-04 12:40:07Z $
 */

#define _GNU_SOURCE	// for stpcpy()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedefs.h>
#include <rtconfig.h>
#include <bcmnvram.h>

#include <shared.h>

#include "nvram_mode.h"

#define PROFILE_HEADER		"HDR1"
#ifdef RTCONFIG_DSL
#define PROFILE_HEADER_NEW	"N55U"
#else
#define PROFILE_HEADER_NEW	"HDR2"
#endif

#define PROTECT_CHAR	'x'
#define DEFAULT_LOGIN_DATA	"xxxxxxxx"

#ifdef RTCONFIG_JFFS_NVRAM
#define EXT_NVRAM_SPACE   0x40000
#else
#define EXT_NVRAM_SPACE   MAX_NVRAM_SPACE
#endif

extern int dev_nvram_getall(char *buf, int count);

static int export_mode(char* mode, char* buf_ap, char* buf)
{
	const char **conf = NULL, **hold = NULL;
	const wlc_conf_t *wlc = NULL;
	char *ptr, *item, *value;
	char name[128], nv[128];
	int i, len;

	if (!buf_ap || !buf)
		return -1;

	if (strcmp(mode, "save_ap") == 0) {
		conf = conf_ap;
		hold = hold_ap;
	} else if (strcmp(mode, "save_rp_2g") == 0) {
		conf = conf_rp_2g;
		hold = hold_rp_2g;
		wlc = wlc_rp_2g;
	} else if (strcmp(mode, "save_rp_5g") == 0) {
		conf = conf_rp_5g;
		hold = hold_rp_5g;
		wlc = wlc_rp_5g;
	} else if (strcmp(mode, "save_rp_5g2") == 0) {
		conf = conf_rp_5g;
		hold = hold_rp_5g;
		wlc = wlc_rp_5g2;
	} else
		return -1;

	ptr = buf_ap;

	for (i = 0; conf[i]; i++) {
#ifdef ASUS_DEBUG
		puts(conf[i]);
#endif
		ptr = stpcpy(ptr, conf[i]) + 1;
	}

	for (item = buf; *item; item += strlen(item) + 1) {
		value = strchr(item, '=');
		if (!value)
			continue;
		len = value - item;
		if (len < 0 || len > sizeof(name) - 1)
			continue;

		strncpy(name, item, len);
		name[len] = '\0';
		value++;

		for (i = 0; hold[i]; i++) {
			if (strcmp(name, hold[i]) == 0) {
				snprintf(nv, sizeof(nv), "%s=%s", name, value);
#ifdef ASUS_DEBUG
				puts(nv);
#endif
				ptr = stpcpy(ptr, nv) + 1;
				break;
			}
		}

		// save wlc settings
		if (wlc) {
			for (i = 0; wlc[i].key_assign; i++) {
				if (strcmp(name, wlc[i].key_from) == 0) {
					snprintf(nv, sizeof(nv), "%s=%s", wlc[i].key_assign, value);
#ifdef ASUS_DEBUG
					puts(nv);
#endif
					ptr = stpcpy(ptr, nv) + 1;
				}
			}
		}
	}

	return 0;
}

#ifdef RTCONFIG_NVRAM_ENCRYPT
static int nvram_dec_all(char* buf_ap, char* buf)
{
	struct nvram_tuple *t;
	extern struct nvram_tuple router_defaults[];
	char *ptr, *item, *value;
	char name[128], nv[65535];
	int len;
	char output[NVRAM_ENC_MAXLEN];
	memset(output, 0, sizeof(output));


	if (!buf_ap || !buf)
		return -1;

	ptr = buf_ap;

	for (item = buf; *item; item += strlen(item) + 1) {
		value = strchr(item, '=');
		if (!value)
			continue;
		len = value - item;
		if (len < 0 || len > sizeof(name) - 1)
			continue;

		strncpy(name, item, len);
		name[len] = '\0';
		value++;

		for (t = router_defaults; t->name; t++)
		{
			if (strcmp(name, t->name) == 0 && t->enc == 1) {
				dec_nvram(t->name, value, output);
				value = output;
			}
		}

		snprintf(nv, sizeof(nv), "%s=%s", name, value);
		ptr = stpcpy(ptr, nv) + 1;
#ifdef ASUS_DEBUG
				puts(nv);
#endif
	}

	return 0;
}
#endif

/*******************************************************************
* NAME: _secure_romfile
* AUTHOR: Andy Chiu
* CREATE DATE: 2015/06/08
* DESCRIPTION: replace account /password by PROTECT_CHAR with the same string length
* INPUT:  path: the rom file path
* OUTPUT:
* RETURN:  0: success, -1:failed
* NOTE: Andy Chiu, 2015/12/18. Add new tokens.
*           Andy Chiu, 2015/12/24. Add new tokens.
*	     Andy Chiu, 2016/02/18. Add new token, wtf_username.
*******************************************************************/
/* memset list field.
 * member mask is bitmask: 0 - 1st, 2 - 2nd, ... 0x80000000 - 31th member */
static char *nvlist_memset(char *list, int c, unsigned int member_mask)
{
	char *item, *end, *next;
	char *mitem, *mend, *mnext;
	unsigned int mask;

	if (!list || member_mask == 0)
		return list;

	for (item = list; *item; ) {
		end = strchr(item, '<');
		if (end)
			next = end + 1;
		else	next = end = strchr(item, 0);
		for (mitem = item, mask = member_mask;
		     mitem < end && mask; mask >>= 1) {
		        mend = memchr(mitem, '>', end - mitem);
			if (mend)
				mnext = mend + 1;
			else	mnext = mend = end;
			if (mask & 1)
				memset(mitem, c, mend - mitem);
			mitem = mnext;
		}
		item = next;
	}

	return list;
}

#if 0
static int _secure_conf(char* buf)
{
	char name[128], *value, *item;
	int len, i;

	//name contains token
	const char *keyword_token[] = {"http_username", "passwd", "password",
		NULL};	//Andy Chiu, 2015/12/18

	//name is token
	const char *token1[] = {"wan_pppoe_passwd", "modem_pass", "modem_pincode",
		"http_passwd", "wan0_pppoe_passwd", "dslx_pppoe_passwd", "ddns_passwd_x",
		"wl_wpa_psk",	"wlc_wpa_psk",  "wlc_wep_key",
		"wl0_wpa_psk", "wl0.1_wpa_psk", "wl0.2_wpa_psk", "wl0.3_wpa_psk",
		"wl1_wpa_psk", "wl1.1_wpa_psk", "wl1.2_wpa_psk", "wl1.3_wpa_psk",
		"wl0.1_key1", "wl0.1_key2", "wl0.1_key3", "wl0.1_key4",
		"wl0.2_key1", "wl0.2_key2", "wl0.2_key3", "wl0.2_key4",
		"wl0.3_key1", "wl0.3_key2", "wl0.3_key3", "wl0.3_key4",
		"wl0_key1", "wl0_key2", "wl0_key3", "wl0_key4",
		"wl1.1_key1", "wl1.1_key2", "wl1.1_key3", "wl1.1_key4",
		"wl1.2_key1", "wl1.2_key2", "wl1.2_key3", "wl1.2_key4",
		"wl1.3_key1", "wl1.3_key2", "wl1.3_key3", "wl1.3_key4",
		"wl_key1", "wl_key2", "wl_key3", "wl_key4",
		"wl1_key1", "wl1_key2", "wl1_key3", "wl1_key4",
		"wl0_phrase_x", "wl0.1_phrase_x", "wl0.2_phrase_x", "wl0.3_phrase_x",
		"wl1_phrase_x", "wl1.1_phrase_x", "wl1.2_phrase_x", "wl1.3_phrase_x",
		"wl_phrase_x", "vpnc_openvpn_pwd", "PM_SMTP_AUTH_USER", "PM_MY_EMAIL",
		"PM_SMTP_AUTH_PASS", "wtf_username", "ddns_hostname_x", "ddns_username_x",
		"shell_username", "shell_passwd",
		NULL};

	//name is token
	//value is [<]username>password<username...
	const char *token2[] = {"acc_list", "pptpd_clientlist", "vpn_serverx_clientlist",
		NULL};

	//name is token
	//valus is [<]desc>type>index>username>password<desc...
	const char vpnc_token[] = "vpnc_clientlist";

	//name is token
	//value is [<]type>username>password>url>rule>dir>enable<type...
	const char cloud_token[] = "cloud_sync";

	//name is token
	//value is username@domain.tld
	const char pppoe_username_token[] = "pppoe_username";

	if (!buf)
		return -1;

	for (item = buf; *item; item += strlen(item) + 1) {
		value = strchr(item, '=');
		if (!value)
			continue;
		len = value - item;
		if (len < 0 || len > sizeof(name) - 1)
			continue;

		strncpy(name, item, len);
		name[len] = '\0';
		value++;

		//check the password keyword token
		for (i = 0; keyword_token[i]; i++) {
			if (strstr(name, keyword_token[i]) != NULL) {
				memset(value, PROTECT_CHAR, strlen(value));
				goto next;
			}
		}

		//check the first token group
		for (i = 0; token1[i]; i++) {
			if (strcmp(name, token1[i]) == 0) {
				memset(value, PROTECT_CHAR, strlen(value));
				goto next;
			}
		}

		//check the 2nd token group
		//value is [<]username>password<username...
		for (i = 0; token2[i]; i++) {
			if (strcmp(name, token2[i]) == 0) {
				nvlist_memset(value, PROTECT_CHAR, (1u << 1));
				goto next;
			}
		}

		//check vpnc token
		//valus is [<]desc>type>index>username>password<desc...
		if (strcmp(name, vpnc_token) == 0) {
			nvlist_memset(value, PROTECT_CHAR, (1u << 4));
			goto next;
		}

		//check cloud sync token
		//value is [<]type>xxx1>xxx2>xxx3>xxx4>xxx5>xxx6<type...
		if (strcmp(name, cloud_token) == 0) {
			nvlist_memset(value, PROTECT_CHAR, (1u << 2)|(1u << 3)|(1u << 5));
			goto next;
		}

		//check
		//value is password@domain.tld
		if (strstr(name, pppoe_username_token) != NULL) {
			char *e = strchr(value, '@') ? : strchr(value, 0);
			memset(value, PROTECT_CHAR, e - value);
		}

	next:
		continue;
	}

	return 0;
}
#else
static int _convert_data(const char *name, char *value, size_t value_len)
{
	int i;

	//name contains token
	const char *http_token[] = {"http_username", "http_passwd", NULL};
	const char password_token[] = "password";
	const char acclist_token[] = "acc_list";

	//name is token
	const char *token1[] = {"wan_pppoe_passwd", "modem_pass", "modem_pincode",
		"http_passwd", "wan0_pppoe_passwd", "dslx_pppoe_passwd", "ddns_passwd_x",
		"wl_wpa_psk",	"wlc_wpa_psk",  "wlc_wep_key",
		"wl0_wpa_psk", "wl0.1_wpa_psk", "wl0.2_wpa_psk", "wl0.3_wpa_psk",
		"wl1_wpa_psk", "wl1.1_wpa_psk", "wl1.2_wpa_psk", "wl1.3_wpa_psk",
		"wl2_wpa_psk", "wl2.1_wpa_psk", "wl2.2_wpa_psk", "wl2.3_wpa_psk",
		"wl0.1_key1", "wl0.1_key2", "wl0.1_key3", "wl0.1_key4",
		"wl0.2_key1", "wl0.2_key2", "wl0.2_key3", "wl0.2_key4",
		"wl0.3_key1", "wl0.3_key2", "wl0.3_key3", "wl0.3_key4",
		"wl0_key1", "wl0_key2", "wl0_key3", "wl0_key4",
		"wl1.1_key1", "wl1.1_key2", "wl1.1_key3", "wl1.1_key4",
		"wl1.2_key1", "wl1.2_key2", "wl1.2_key3", "wl1.2_key4",
		"wl1.3_key1", "wl1.3_key2", "wl1.3_key3", "wl1.3_key4",
		"wl1_key1", "wl1_key2", "wl1_key3", "wl1_key4",
		"wl2.1_key1", "wl2.1_key2", "wl2.1_key3", "wl2.1_key4",
		"wl2.2_key1", "wl2.2_key2", "wl2.2_key3", "wl2.2_key4",
		"wl2.3_key1", "wl2.3_key2", "wl2.3_key3", "wl2.3_key4",
		"wl2_key1", "wl2_key2", "wl2_key3", "wl2_key4",
		"wl_key1", "wl_key2", "wl_key3", "wl_key4",
		"wl0_phrase_x", "wl0.1_phrase_x", "wl0.2_phrase_x", "wl0.3_phrase_x",
		"wl1_phrase_x", "wl1.1_phrase_x", "wl1.2_phrase_x", "wl1.3_phrase_x",
		"wl2_phrase_x", "wl2.1_phrase_x", "wl2.2_phrase_x", "wl2.3_phrase_x",
		"wl_phrase_x", "vpnc_openvpn_pwd", "PM_SMTP_AUTH_USER", "PM_MY_EMAIL",
		"PM_SMTP_AUTH_PASS", "wtf_username", "ddns_hostname_x", "ddns_username_x",
		"shell_username", "shell_passwd",
		NULL};

	//name is token
	//value is [<]username>password<username...
	const char *token2[] = {"pptpd_clientlist", "vpn_serverx_clientlist", 	NULL};

	//name is token
	//valus is [<]desc>type>index>username>password<desc...
	const char vpnc_token[] = "vpnc_clientlist";

	//name is token
	//value is [<]type>username>password>url>rule>dir>enable<type...
	const char cloud_token[] = "cloud_sync";

	//name is token
	//value is username@domain.tld
	const char pppoe_username_token[] = "pppoe_username";

	if(!value)
		return 0;

	//change http login username and password as xxxxxxxx
	for(i = 0; http_token[i]; ++i)
	{
		if(!strcmp(name, http_token[i]))
		{
			strlcpy(value, DEFAULT_LOGIN_DATA, value_len);
			return 1;
		}		
	}

	//check the password keyword token
	if (strstr(name, password_token) != NULL) 
	{
		memset(value, PROTECT_CHAR, strlen(value));
		return 1;
	}

	//check the first token group
	for (i = 0; token1[i]; i++) 
	{
		if (strcmp(name, token1[i]) == 0) 
		{
			memset(value, PROTECT_CHAR, strlen(value));
			return 1;
		}
	}

	//check the 2nd token group
	//value is [<]username>password<username...
	for (i = 0; token2[i]; i++)
	{
		if (strcmp(name, token2[i]) == 0)
		{
			nvlist_memset(value, PROTECT_CHAR, (1u << 1));
			return 1;
		}
	}

	//check vpnc token
	//valus is [<]desc>type>index>username>password<desc...
	if (strcmp(name, vpnc_token) == 0) 
	{
		nvlist_memset(value, PROTECT_CHAR, (1u << 4));
		return 1;
	}

	//check cloud sync token
	//value is [<]type>xxx1>xxx2>xxx3>xxx4>xxx5>xxx6<type...
	if (strcmp(name, cloud_token) == 0) 
	{
		nvlist_memset(value, PROTECT_CHAR, (1u << 2)|(1u << 3)|(1u << 5));
		return 1;
	}

	//check
	//value is password@domain.tld
	if (strstr(name, pppoe_username_token) != NULL) 
	{
		char *e = strchr(value, '@') ? : strchr(value, 0);
		memset(value, PROTECT_CHAR, e - value);
		return 1;
	}

	//convert acc_list as DEFAULT_LOGIN_DATA>DEFAULT_LOGIN_DATA
	if(strcmp(name, acclist_token) == 0)
	{
		snprintf(value, value_len, "%s>%s", DEFAULT_LOGIN_DATA, DEFAULT_LOGIN_DATA);
		return 1;
	}

	return 0;
}

static char* _get_attr(const char *buf, char *name, size_t name_len, char *value, size_t value_len)
{
	char *p, *e,  *v;
	if(!buf || !name || !value)
		return NULL;

	memset(name, 0, name_len);
	memset(value, 0, value_len);
	
	p = strchr(buf, '=');
	if(p)
	{
		strlcpy(name, buf, ((p - buf + 1) > name_len)? name_len:  (p - buf + 1));

		v = p + 1;
		e = strchr(v, '\0');
		if(e)
		{
			strlcpy(value, v, ((e - v + 1) > value_len)? value_len:  (e - v + 1));
			return e + 1;
		}
		else	//last line
		{
			strlcpy(value, v, value_len);
			return 0;
		}
		
	}
	return NULL;
}

static int _secure_conf(char* buf, size_t len)
{
	char *tmp, *p = buf, *b;
	char name[256], *value;
	int tmp_len;

	if(!buf || !len)
		return -1;

	tmp = malloc(len);
	if(!tmp)
	{
		fprintf(stderr, "Can NOT alloc memory!!!");
		return -1;
	}

	value = malloc(CKN_STR_MAX);
	if(!value)
	{
		fprintf(stderr, "Can NOT alloc memory!!!");
		free(tmp);
		return -1;
	}

	memset(tmp, 0, len);
	b = tmp;
	
	while(1)
	{
		p = _get_attr(p, name, sizeof(name), value, CKN_STR_MAX);

		if(name[0] != '\0')
		{
			//handle data
			_convert_data(name, value, CKN_STR_MAX);

			//write data in the new buffer
			if(value)
				tmp_len = snprintf(b, len - (b - tmp), "%s=%s", name, value);
			else
				tmp_len = snprintf(b, len - (b - tmp), "%s=", name);
			
			b += (tmp_len + 1);	//Add NULL at the end of the value
		}
		else
			break;
	}

	memcpy(buf, tmp, len);

	free(tmp);
	free(value);
	return 0;
}
#endif

unsigned char get_rand()
{
	unsigned char buf[1];
	FILE *fp;

	fp = fopen("/dev/urandom", "r");
	if (fp == NULL) {
#ifdef ASUS_DEBUG
		fprintf(stderr, "Could not open /dev/urandom.\n");
#endif
		return 0;
	}
	fread(buf, 1, 1, fp);
	fclose(fp);

	return buf[0];
}

int nvram_save_new(char *file, char *buf)
{
	FILE *fp;
	char *name;
	unsigned long count, filelen, i;
	unsigned char rand = 0, temp;

	if ((fp = fopen(file, "w")) == NULL) return -1;

	count = 0;
	for (name = buf; *name; name += strlen(name) + 1)
	{
#ifdef ASUS_DEBUG
		puts(name);
#endif
		count = count + strlen(name) + 1;
	}

	filelen = count + (1024 - count % 1024);
#if 1
	//workaround for do_upload_post waitfor() timeout
	filelen = filelen >= 1024*3 ? filelen : 1024*3;
#endif
	rand = get_rand() % 30;
#ifdef ASUS_DEBUG
	fprintf(stderr, "random number: %x\n", rand);
#endif
	fwrite(PROFILE_HEADER_NEW, 1, 4, fp);
	fwrite(&filelen, 1, 3, fp);
	fwrite(&rand, 1, 1, fp);
#ifdef ASUS_DEBUG
	for (i = 0; i < 4; i++)
	{
		fprintf(stderr, "%2x ", PROFILE_HEADER_NEW[i]);
	}
	for (i = 0; i < 3; i++)
	{
		fprintf(stderr, "%2x ", ((char *)&filelen)[i]);
	}
	fprintf(stderr, "%2x ", ((char *)&rand)[0]);
#endif
	for (i = 0; i < count; i++)
	{
		if (buf[i] == 0x0)
			buf[i] = 0xfd + get_rand() % 3;
		else
			buf[i] = 0xff - buf[i] + rand;
	}
	fwrite(buf, 1, count, fp);
#ifdef ASUS_DEBUG
	for (i = 0; i < count; i++)
	{
		if (i % 16 == 0) fprintf(stderr, "\n");
		fprintf(stderr, "%2x ", (unsigned char) buf[i]);
	}
#endif
	for (i = count; i < filelen; i++)
	{
		temp = 0xfd + get_rand() % 3;
		fwrite(&temp, 1, 1, fp);
#ifdef ASUS_DEBUG
		if (i % 16 == 0) fprintf(stderr, "\n");
		fprintf(stderr, "%2x ", (unsigned char) temp);
#endif
	}
	fclose(fp);
	return 0;
}

int issyspara(char *p)
{
	struct nvram_tuple *t;
	extern struct nvram_tuple router_defaults[];
	
	// skip checking for wl[]_, wan[], lan[]_
	if ((strstr(p, "wl") && strncmp(p+3, "_failed", 7)) || strstr(p, "wan") || strstr(p, "lan")
		|| strstr(p, "vpn_server") || strstr(p, "vpn_client")
#ifdef RTCONFIG_DSL
		|| !strncmp(p, "dsl", 3)
#endif
	)
		return 1;

	for (t = router_defaults; t->name; t++)
	{
		if (strstr(p, t->name))
			break;
	
	}

	if (t->name) return 1;
	else return 0;
}

int nvram_restore_new(char *file, char *buf)
{
	FILE *fp;
	char header[8], *p, *v;
	unsigned long count, filelen, *filelenptr, i;
	unsigned char rand, *randptr;

	if ((fp = fopen(file, "r+")) == NULL) return -1;

	count = fread(header, 1, 8, fp);
	if (count>=8 && strncmp(header, PROFILE_HEADER, 4) == 0)
	{
		filelenptr = (unsigned long *)(header + 4);
#ifdef ASUS_DEBUG
		fprintf(stderr, "restoring original text cfg of length %x\n", *filelenptr);
#endif
		fread(buf, 1, *filelenptr, fp);
	}
	else if (count>=8 && strncmp(header, PROFILE_HEADER_NEW, 4) == 0)
	{
		filelenptr = (unsigned long *)(header + 4);
		filelen = *filelenptr & 0xffffff;
#ifdef ASUS_DEBUG
		fprintf(stderr, "restoring non-text cfg of length %x\n", filelen);
#endif
		randptr = (unsigned char *)(header + 7);
		rand = *randptr;
#ifdef ASUS_DEBUG
		fprintf(stderr, "non-text cfg random number %x\n", rand);
#endif
		count = fread(buf, 1, filelen, fp);
#ifdef ASUS_DEBUG
		fprintf(stderr, "non-text cfg count %x\n", count);
#endif
		for (i = 0; i < count; i++)
		{
			if ((unsigned char) buf[i] > ( 0xfd - 0x1)){
				/* e.g.: to skip the case: 0x61 0x62 0x63 0x00 0x00 0x61 0x62 0x63 */
				if(i > 0 && buf[i-1] != 0x0)
					buf[i] = 0x0;
			}
			else
				buf[i] = 0xff + rand - buf[i];
		}
#ifdef ASUS_DEBUG
		for (i = 0; i < count; i++)
		{
			if (i % 16 == 0) fprintf(stderr, "\n");
			fprintf(stderr, "%2x ", (unsigned char) buf[i]);
		}

		for (i = 0; i < count; i++)
		{
			if (i % 16 == 0) fprintf(stderr, "\n");
			fprintf(stderr, "%c", buf[i]);
		}
#endif
	}
	else
	{
		fclose(fp);
		return 0;
	}
	fclose(fp);

	p = buf;

	while (*p || p-buf<=count)
	{
#if 1
		/* e.g.: to skip the case: 00 2e 30 2e 32 38 00 ff 77 61 6e */
		if(*p == '\0' || (unsigned char)*p < 32 || (unsigned char)*p > 127 ){
			p = p + 1;
			continue;
		}
#endif
		v = strchr(p, '=');

		if (v != NULL)
		{
			*v++ = '\0';

			if (issyspara(p)) {
				if (ate_brcm_factory_mode() && (!strcmp(p, "http_username") || !strcmp(p, "http_passwd")))
					nvram_set(p, "admin");
				else
					nvram_set(p, v);
			}

			p = v + strlen(v) + 1;
		}
		else
		{
			nvram_unset(p);
			p = p + 1;
		}
	}
	return 0;
}

void
usage(void)
{
	fprintf(stderr,
	        "usage: nvram [get name] [set name=value] "
	        "[unset name] [show] [commit] [save] [restore] [erase] [fb_save file]\n"
		"usage: nvram [save_ap file] [save_rp_2g file] [save_rp_5g file] [save_rp_5g2 file]\n");
	exit(0);
}

/* NVRAM utility */
int
main(int argc, char **argv)
{
	char *name, *value, buf[EXT_NVRAM_SPACE];
	int size;

	/* Skip program name */
	--argc;
	++argv;

	if (!*argv)
		usage();

	/* Process the arguments */
	for (; *argv; ++argv) {
		if (!strcmp(*argv, "get")) {
			if (*++argv) {
#ifdef RTCONFIG_NVRAM_ENCRYPT
				if(invalid_nvram_get_name(*argv))
					return 0;
#endif
				if ((value = nvram_get(*argv)))
					puts(value);
			}
		} else if (!strcmp(*argv, "set")) {
			if (*++argv) {
				strncpy(value = buf, *argv, sizeof(buf));
				name = strsep(&value, "=");
				nvram_set(name, value);
			}
		} else if (!strcmp(*argv, "unset")) {
			if (*++argv)
				nvram_unset(*argv);
		} else if (!strcmp(*argv, "commit")) {
			nvram_commit();
		} else if (!strcmp(*argv, "save")) {
			if (*++argv)
			{
				nvram_getall(buf, sizeof(buf));
#ifdef RTCONFIG_NVRAM_ENCRYPT
				char *tmp_dnv = malloc(EXT_NVRAM_SPACE);
				if (!tmp_dnv) {
					fprintf(stderr, "Can NOT alloc memory!!!");
					return 0;
				}
				memset(tmp_dnv, 0, EXT_NVRAM_SPACE);
				nvram_dec_all(tmp_dnv, buf);
				nvram_save_new(*argv, tmp_dnv);
				free(tmp_dnv);
#else
				nvram_save_new(*argv, buf);
#endif
			}
		} else if (!strncmp(*argv, "save_ap", 7) ||
			   !strncmp(*argv, "save_rp", 7)) {
			char *mode = *argv;
			if (*++argv) {
				char *tmp_export = malloc(EXT_NVRAM_SPACE);
				if (!tmp_export) {
					fprintf(stderr, "Can NOT alloc memory!!!");
					return 0;
				}
				memset(tmp_export, 0, EXT_NVRAM_SPACE);
				nvram_getall(buf, sizeof(buf));
#ifdef RTCONFIG_NVRAM_ENCRYPT
				char *tmp_dnv = malloc(EXT_NVRAM_SPACE);
				if (!tmp_dnv) {
					fprintf(stderr, "Can NOT alloc memory!!!");
					free(tmp_export);
					return 0;
				}
				memset(tmp_dnv, 0, EXT_NVRAM_SPACE);
				nvram_dec_all(tmp_dnv, buf);
				export_mode(mode, tmp_export, tmp_dnv);
				free(tmp_dnv);
#else
				export_mode(mode, tmp_export, buf);
#endif
				nvram_save_new(*argv, tmp_export);
				free(tmp_export);
			}
		//Andy Chiu, 2015/06/09
		} else if (!strncmp(*argv, "fb_save", 7)) {
			if (*++argv) {
				char *tmpbuf = malloc(EXT_NVRAM_SPACE);
				if (!tmpbuf) {
					fprintf(stderr, "Can NOT alloc memory!!!");
					return 0;
				}
				nvram_getall(buf, sizeof(buf));
#ifdef RTCONFIG_NVRAM_ENCRYPT
				memset(tmpbuf, 0, EXT_NVRAM_SPACE);
				nvram_dec_all(tmpbuf, buf);
#else
				memcpy(tmpbuf, buf, EXT_NVRAM_SPACE);
#endif
				_secure_conf(tmpbuf, EXT_NVRAM_SPACE);
				nvram_save_new(*argv, tmpbuf);
				free(tmpbuf);
			}
		} else if (!strcmp(*argv, "restore")) {
			if (*++argv) 
				nvram_restore_new(*argv, buf);
		} else if (!strcmp(*argv, "erase")) {
			system("nvram_erase");
		} else if (!strcmp(*argv, "show") ||
		           !strcmp(*argv, "dump")) {
#ifdef RTCONFIG_NVRAM_ENCRYPT
			char name_tmp[128] = {0};
			char *name_t = NULL, *value;
#endif
			dev_nvram_getall(buf, sizeof(buf));

			for (name = buf; *name; name += strlen(name) + 1){
#ifdef RTCONFIG_NVRAM_ENCRYPT
				strlcpy(name_tmp, name, sizeof(name_tmp));
				name_t = value = name_tmp;
				name_t = strsep(&value, "=");

				if(invalid_nvram_get_name(name_t))
					continue;
#endif
				puts(name);
			}
			size = sizeof(struct nvram_header) + (int) name - (int) buf;
			if (**argv != 'd')
				fprintf(stderr, "size: %d bytes (%d left)\n",
				        size, MAX_NVRAM_SPACE - size);
#ifdef RTCONFIG_JFFS_NVRAM
			jffs_nvram_getall(name - buf, buf, sizeof(buf));
			for ( ; *name; name += strlen(name) + 1)
				puts(name);
#endif
#ifdef RTCONFIG_VAR_NVRAM
			memset(buf, 0, sizeof(buf));
			var_nvram_getall(buf, sizeof(buf));
			for (name = buf; *name; name += strlen(name) + 1)
				puts(name);
#endif
		} else
			usage();
	}

	return 0;
}
