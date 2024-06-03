/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
 
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "types.h"
#include "os.h"
#include "util.h"

#define BUF_SIZE 512

int  mtk_copy_file (const char *src, const char *dst)
{
    FILE *from, *to;
    char buf[BUF_SIZE];
    size_t size = 0;

    // Open
    if ((from = fopen (src, "rb")) == NULL) {
        printf ("can not open %s\n", src);
        return 1;
    }
    if ((to = fopen (dst, "wb")) ==  NULL) {
		fclose (from);
        return 2;
    }

    // Copy
    while ((size = fread (buf, 1, BUF_SIZE, from)) > 0) {
        fwrite (buf, 1, size, to);
    //    printf (".");
    }
    printf ("\n");

    // Close
    if (fclose (from) == EOF) {
        printf ("closing %s failed.\n", src);
		fclose (to);
        return 5;
    }
    if (fclose (to) == EOF) {
        printf ("closing %s failed.\n", dst);
        return 6;
    }

    return 0;
}

u8 BtoH(char ch)
{
    if (ch >= '0' && ch <= '9') return (ch - '0');        /* Handle numerals*/
    if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  /* Handle capitol hex digits*/
    if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  /* Handle small hex digits*/
    return(255);
}

void AtoH(char *src, char *dest, int destlen)
{
    char *srcptr;
    char *destTemp;

    srcptr = src;   
    destTemp = (char *) dest; 

    while(destlen--)
    {
        *destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
        *destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
        destTemp++;
    }
}

void * __hide_aliasing_typecast(void *foo)         
{                                                                       
    return foo;        
}

/**
 * os_strlcpy - Copy a string with size bound and NUL-termination
 * @dest: Destination
 * @src: Source
 * @siz: Size of the target buffer
 * Returns: Total length of the target string (length of src) (not including
 * NUL-termination)
 *
 * This function matches in behavior with the strlcpy(3) function in OpenBSD.
 */
size_t os_strlcpy(char *dest, const char *src, size_t siz)
{
        const char *s = src;
        size_t left = siz;

        if (left) {
                /* Copy string up to the maximum size of the dest buffer */
                while (--left != 0) {
                        if ((*dest++ = *s++) == '\0')
                                break;
                }
        }

        if (left == 0) {
                /* Not enough room for the string; force NUL-termination */
                if (siz != 0)
                        *dest = '\0';
                while (*s++)
                        ; /* determine total src string length */
        }

        return s - src - 1;
}

/**
 * os_memdup - Allocate duplicate of passed memory chunk
 * @src: Source buffer to duplicate
 * @len: Length of source buffer
 * Returns: %NULL if allocation failed, copy of src buffer otherwise
 *
 * This function allocates a memory block like os_malloc() would, and
 * copies the given source buffer into it.
 */
void * os_memdup(const void *src, size_t len)
{
        void *r = os_malloc(len);

        if (r)
                os_memcpy(r, src, len);
        return r;
}

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

/**
 * hwaddr_aton2 - Convert ASCII string to MAC address (in any known format)
 * @txt: MAC address as a string (e.g., 00:11:22:33:44:55 or 0011.2233.4455)
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: Characters used (> 0) on success, -1 on failure
 */
int hwaddr_aton2(const char *txt, u8 *addr)
{
	int i;
	const char *pos = txt;

	for (i = 0; i < 6; i++) {
		int a, b;

		while (*pos == ':' || *pos == '.' || *pos == '-')
			pos++;

		a = hex2num(*pos++);
		if (a < 0)
			return -1;
		b = hex2num(*pos++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
	}

	return pos - txt;
}

/**
 * hexstr2bin - Convert ASCII hex string into binary data
 * @hex: ASCII hex string (e.g., "01ab")
 * @buf: Buffer for the binary data
 * @len: Length of the text to convert in bytes (of buf); hex will be double
 * this size
 * Returns: 0 on success, -1 on failure (invalid hex string)
 */
int hexstr2bin(const char *hex, u8 *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	u8 *opos = buf;

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}

static inline int _os_snprintf_hex(char *buf, size_t buf_size, const u8 *data,
				    size_t len, int uppercase)
{
	size_t i;
	char *pos = buf, *end = buf + buf_size;
	int ret;
	if (buf_size == 0)
		return 0;
	for (i = 0; i < len; i++) {
		ret = os_snprintf(pos, end - pos, uppercase ? "%02X" : "%02x",
				  data[i]);
		if (os_snprintf_error(end - pos, ret)) {
			end[-1] = '\0';
			return pos - buf;
		}
		pos += ret;
	}
	end[-1] = '\0';
	return pos - buf;
}

/**
 * os_snprintf_hex - Print data as a hex string into a buffer
 * @buf: Memory area to use as the output buffer
 * @buf_size: Maximum buffer size in bytes (should be at least 2 * len + 1)
 * @data: Data to be printed
 * @len: Length of data in bytes
 * Returns: Number of bytes written
 */
int os_snprintf_hex(char *buf, size_t buf_size, const u8 *data, size_t len)
{
	return _os_snprintf_hex(buf, buf_size, data, len, 0);
}

/**
 * os_ssid_txt - Convert SSID to a printable string
 * @ssid: SSID (32-octet string)
 * @ssid_len: Length of ssid in octets
 * Returns: Pointer to a printable string
 *
 * This function can be used to convert SSIDs into printable form. In most
 * cases, SSIDs do not use unprintable characters, but IEEE 802.11 standard
 * does not limit the used character set, so anything could be used in an SSID.
 *
 * This function uses a static buffer, so only one call can be used at the
 * time, i.e., this is not re-entrant and the returned buffer must be used
 * before calling this again.
 */
const char * os_ssid_txt(const u8 *ssid, size_t ssid_len)
{
	static char ssid_txt[SSID_MAX_LEN * 4 + 1];

	if (ssid == NULL) {
		ssid_txt[0] = '\0';
		return ssid_txt;
	}

	printf_encode(ssid_txt, sizeof(ssid_txt), ssid, ssid_len);
	return ssid_txt;
}

int is_hex(const u8 *data, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		if (data[i] < 32 || data[i] >= 127)
			return 1;
	}
	return 0;
}

void str_clear_free(char *str)
{
	if (str) {
		size_t len = os_strlen(str);
		os_memset(str, 0, len);
		os_free(str);
	}
}


void bin_clear_free(void *bin, size_t len)
{
	if (bin) {
		os_memset(bin, 0, len);
		os_free(bin);
	}
}

int is_ctrl_char(char c)
{
	return c > 0 && c < 32;
}

char * get_param(const char *cmd, const char *param)
{
	const char *pos, *end;
	char *val;
	size_t len;

	pos = os_strstr(cmd, param);
	if (!pos)
		return NULL;

	pos += os_strlen(param);
	end = os_strchr(pos, ' ');
	if (end)
		len = end - pos;
	else
		len = os_strlen(pos);
	val = os_malloc(len + 1);
	if (!val)
		return NULL;
	os_memcpy(val, pos, len);
	val[len] = '\0';
	return val;
}
