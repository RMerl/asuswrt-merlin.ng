/*
 * Wireless network adapter utilities
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wl.c 787328 2020-05-26 20:17:46Z $
 */
#include <typedefs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#if	defined(__ECOS)
#include <sys/socket.h>
#endif
#include <net/if.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <wlutils.h>
#include <bcmconfig.h>
#ifdef RTCONFIG_HND_ROUTER_AX
#include <bcmiov.h>
#endif

#ifndef MAX_WLAN_ADAPTER
#define MAX_WLAN_ADAPTER	16
#endif	/* MAX_WLAN_ADAPTER */

typedef struct {
	uint16 id;
	uint16 len;
	uint32 val;
} he_xtlv_v32;

/* Global swap variable */
bool gg_swap = FALSE;

/*
 * Probes the wireless interface for endianness.
 * Maintains a static array for each interface.
 * First probe for the adapter will set the swap variable alongwith
 * the static array for that particular interface index, subsequent invokations
 * will not probe the adapter, swap variable will be set based on the
 * value present in array for the interface index.
 */
int
wl_endian_probe(char *name)
{
	int ret = 0, val;
	char *c = name;	/* Can be wl0, wl1, wl0.1, wl1.0 etc. */
	uint8 idx = MAX_WLAN_ADAPTER;
	static int endian_stat[MAX_WLAN_ADAPTER] = {0};

	while (*c != '\0') {
	        if (isdigit(*c)) {
		        idx = *c - '0';
		        break;
		} else {
			c++;
		}
	}

	if (idx >= MAX_WLAN_ADAPTER) {
		fprintf(stderr, "Error: WLAN adapter %s index out of range in %s at line %d\n",
			name, __FUNCTION__, __LINE__);
		goto end;
	} else if (endian_stat[idx] != 0) {
		gg_swap = endian_stat[idx] > 0 ? TRUE : FALSE;
		goto end;
	}

	if ((ret = wl_ioctl(name, WLC_GET_MAGIC, &val, sizeof(int))) < 0) {
		gg_swap = FALSE;
		endian_stat[idx] = -1;
		goto end;
	}

	if (val == (int)bcmswap32(WLC_IOCTL_MAGIC)) {
		gg_swap = TRUE;
		endian_stat[idx] = 1;
	} else {
		gg_swap = FALSE;
		endian_stat[idx] = -1;
	}

end:
	return ret;
}

int
wl_probe(char *name)
{
	int ret, val;

#if defined(linux) || defined(__ECOS)
	char buf[DEV_TYPE_LEN];
	if ((ret = wl_get_dev_type(name, buf, DEV_TYPE_LEN)) < 0)
		return ret;
	/* Check interface */
	if (strncmp(buf, "wl", 2))
		return -1;
#else
	/* Check interface */
	if ((ret = wl_ioctl(name, WLC_GET_MAGIC, &val, sizeof(val))))
		return ret;
#endif // endif
	if ((ret = wl_ioctl(name, WLC_GET_VERSION, &val, sizeof(val))))
		return ret;
	if (val > WLC_IOCTL_VERSION)
		return -1;

	return ret;
}

#ifdef __CONFIG_DHDAP__
#include <dhdioctl.h>
/*
 * Probe the specified interface.
 * @param	name	interface name
 * @return	0       if using dhd driver
 *          <0      otherwise
 */
int
dhd_probe(char *name)
{
	int ret, val;
	val = 0;
	/* Check interface */
	ret = dhd_ioctl(name, DHD_GET_MAGIC, &val, sizeof(val));
	if (val == WLC_IOCTL_MAGIC) {
		ret = 1; /* is_dhd = !dhd_probe(), so ret 1 for WL */
	} else if (val == DHD_IOCTL_MAGIC) {
		ret = 0;
	} else {
		if (ret < 0) {
			perror("dhd_ioctl");
		}
		ret = 1; /* default: WL mode */
	}
	return ret;
}
#endif /* __CONFIG_DHDAP__ */

int
wl_iovar_getbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1;	 /* length of iovar name plus null */
	iolen = namelen + paramlen;

	/* check for overflow */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8*)bufptr + namelen, param, paramlen);

	err = wl_ioctl(ifname, WLC_GET_VAR, bufptr, buflen);

	return (err);
}

#ifdef __CONFIG_DHDAP__
int
dhd_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1;	 /* length of iovar name plus null */
	iolen = namelen + paramlen;

	/* check for overflow */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8*)bufptr + namelen, param, paramlen);

	return dhd_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}
#endif /* __CONFIG_DHDAP__ */

int
wl_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1;	 /* length of iovar name plus null */
	iolen = namelen + paramlen;

	/* check for overflow */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8*)bufptr + namelen, param, paramlen);

	return wl_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}

#ifdef __CONFIG_DHDAP__
int
dhd_iovar_set(char *ifname, char *iovar, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	return dhd_iovar_setbuf(ifname, iovar, param, paramlen, smbuf, sizeof(smbuf));
}
#endif /* __CONFIG_DHDAP__ */

int
wl_iovar_set(char *ifname, char *iovar, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	return wl_iovar_setbuf(ifname, iovar, param, paramlen, smbuf, sizeof(smbuf));
}

int
wl_iovar_get(char *ifname, char *iovar, void *bufptr, int buflen)
{
	char smbuf[WLC_IOCTL_SMLEN];
	int ret;

	/* use the return buffer if it is bigger than what we have on the stack */
	if (buflen > sizeof(smbuf)) {
		ret = wl_iovar_getbuf(ifname, iovar, NULL, 0, bufptr, buflen);
	} else {
		ret = wl_iovar_getbuf(ifname, iovar, NULL, 0, smbuf, sizeof(smbuf));
		if (ret == 0)
			memcpy(bufptr, smbuf, buflen);
	}

	return ret;
}

#ifdef __CONFIG_DHDAP__
/*
 * set named driver variable to int value
 * calling example: dhd_iovar_setint(ifname, "arate", rate)
*/
int
dhd_iovar_setint(char *ifname, char *iovar, int val)
{
	return dhd_iovar_set(ifname, iovar, &val, sizeof(val));
}
#endif /* __CONFIG_DHDAP__ */

/*
 * set named driver variable to int value
 * calling example: wl_iovar_setint(ifname, "arate", rate)
*/
int
wl_iovar_setint(char *ifname, char *iovar, int val)
{
	return wl_iovar_set(ifname, iovar, &val, sizeof(val));
}

/*
 * get named driver variable to int value and return error indication
 * calling example: wl_iovar_getint(ifname, "arate", &rate)
 */
int
wl_iovar_getint(char *ifname, char *iovar, int *val)
{
	return wl_iovar_get(ifname, iovar, val, sizeof(int));
}

/*
 * format a bsscfg indexed iovar buffer
 */
static int
wl_bssiovar_mkbuf(char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen,
                  int *plen)
{
	char *prefix = "bsscfg:";
	int8* p;
	uint prefixlen;
	uint namelen;
	uint iolen;

	prefixlen = strlen(prefix);	/* length of bsscfg prefix */
	namelen = strlen(iovar) + 1;	/* length of iovar name + null */
	iolen = prefixlen + namelen + sizeof(int) + paramlen;

	/* check for overflow */
	if (buflen < 0 || iolen > (uint)buflen) {
		*plen = 0;
		return BCME_BUFTOOSHORT;
	}

	p = (int8*)bufptr;

	/* copy prefix, no null */
	memcpy(p, prefix, prefixlen);
	p += prefixlen;

	/* copy iovar name including null */
	memcpy(p, iovar, namelen);
	p += namelen;

	/* bss config index as first param */
	memcpy(p, &bssidx, sizeof(int32));
	p += sizeof(int32);

	/* parameter buffer follows */
	if (paramlen)
		memcpy(p, param, paramlen);

	*plen = iolen;
	return 0;
}

/*
 * set named & bss indexed driver variable to buffer value
 */
int
wl_bssiovar_setbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr,
                   int buflen)
{
	int err;
	int iolen;

	err = wl_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &iolen);
	if (err)
		return err;

	return wl_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}

#ifdef __CONFIG_DHDAP__
int
dhd_ioctl(char *name, int cmd, void *buf, int len)
{
	struct ifreq ifr;
	dhd_ioctl_t ioc;
	int ret = 0;
	int s;
	char buffer[WLC_IOCTL_SMLEN];

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	/* do it */
	if (cmd == WLC_SET_VAR) {
		cmd = DHD_SET_VAR;
	} else if (cmd == WLC_GET_VAR) {
		cmd = DHD_GET_VAR;
	}

	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = FALSE;
	ioc.driver = DHD_IOCTL_MAGIC;
	ioc.used = 0;
	ioc.needed = 0;

	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

	ifr.ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0)
		if (cmd != WLC_GET_MAGIC && cmd != WLC_GET_BSSID) {
			if ((cmd == WLC_GET_VAR) || (cmd == WLC_SET_VAR)) {
				snprintf(buffer, sizeof(buffer), "%s: WLC_%s_VAR(%s)", name,
				         cmd == WLC_GET_VAR ? "GET" : "SET", (char *)buf);
			} else {
				snprintf(buffer, sizeof(buffer), "%s: cmd=%d", name, cmd);
			}
			perror(buffer);
		}
	/* cleanup */
	close(s);
	return ret;
}

/*
 * set named & bss indexed driver variable to buffer value
 */
int
dhd_bssiovar_setbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr,
                   int buflen)
{
	int err;
	int iolen;

	err = wl_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &iolen);
	if (err)
		return err;

	return dhd_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}

/*
 * get named & bss indexed driver variable to buffer value
 */
int
dhd_bssiovar_getbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr,
                   int buflen)
{
	int err, iolen;

	err = wl_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &iolen);
	if (err)
		return err;

	return dhd_ioctl(ifname, WLC_GET_VAR, bufptr, iolen);
}
#endif /* __CONFIG_DHDAP__ */

/*
 * get named & bss indexed driver variable buffer value
 */
int
wl_bssiovar_getbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr,
                   int buflen)
{
	int err;
	int iolen;

	err = wl_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &iolen);
	if (err)
		return err;

	return wl_ioctl(ifname, WLC_GET_VAR, bufptr, buflen);
}

/*
 * set named & bss indexed driver variable to buffer value
 */
int
wl_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	return wl_bssiovar_setbuf(ifname, iovar, bssidx, param, paramlen, smbuf, sizeof(smbuf));
}

#ifdef __CONFIG_DHDAP__
/*
 * set named & bss indexed driver variable to buffer value
 */
int
dhd_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	return dhd_bssiovar_setbuf(ifname, iovar, bssidx, param, paramlen, smbuf, sizeof(smbuf));
}

/*
 * set named & bss indexed driver variable to buffer value
 */
int
dhd_bssiovar_get(char *ifname, char *iovar, int bssidx, void *outbuf, int len)
{
	int err;
	char smbuf[WLC_IOCTL_SMLEN];

	/* use the return buffer if it is bigger than what we have on the stack */
	if (len > (int)sizeof(smbuf)) {
		err = dhd_bssiovar_getbuf(ifname, iovar, bssidx, NULL, 0, outbuf, len);
	} else {
		memset(smbuf, 0, sizeof(smbuf));
		err = dhd_bssiovar_getbuf(ifname, iovar, bssidx, NULL, 0, smbuf, sizeof(smbuf));
		if (err == 0)
			memcpy(outbuf, smbuf, len);
	}

	return err;
}
#endif /* __CONFIG_DHDAP__ */

/*
 * get named & bss indexed driver variable buffer value
 */
int
wl_bssiovar_get(char *ifname, char *iovar, int bssidx, void *outbuf, int len)
{
	char smbuf[WLC_IOCTL_SMLEN];
	int err;

	/* use the return buffer if it is bigger than what we have on the stack */
	if (len > (int)sizeof(smbuf)) {
		err = wl_bssiovar_getbuf(ifname, iovar, bssidx, NULL, 0, outbuf, len);
	} else {
		memset(smbuf, 0, sizeof(smbuf));
		err = wl_bssiovar_getbuf(ifname, iovar, bssidx, NULL, 0, smbuf, sizeof(smbuf));
		if (err == 0)
			memcpy(outbuf, smbuf, len);
	}

	return err;
}

/*
 * set named & bss indexed driver variable to int value
 */
int
wl_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val)
{
	return wl_bssiovar_set(ifname, iovar, bssidx, &val, sizeof(int));
}

#ifdef __CONFIG_DHDAP__
/*
 * set named & bss indexed driver variable to int value
 */
int
dhd_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val)
{
	return dhd_bssiovar_set(ifname, iovar, bssidx, &val, sizeof(int));
}

static int
dhd_get(void *dhd, int cmd, void *buf, int len)
{
	return dhd_ioctl(dhd, cmd, buf, len);
}

static int
dhd_bssiovar_mkbuf(const char *iovar, int bssidx, void *param,
	int paramlen, void *bufptr, int buflen, int *perr)
{
	const char *prefix = "bsscfg:";
	int8* p;
	uint prefixlen;
	uint namelen;
	uint iolen;

	prefixlen = strlen(prefix);	/* length of bsscfg prefix */
	namelen = strlen(iovar) + 1;	/* length of iovar name + null */
	iolen = prefixlen + namelen + sizeof(int) + paramlen;

	/* check for overflow */
	if (buflen < 0 || iolen > (uint)buflen) {
		*perr = BCME_BUFTOOSHORT;
		return 0;
	}

	p = (int8*)bufptr;

	/* copy prefix, no null */
	memcpy(p, prefix, prefixlen);
	p += prefixlen;

	/* copy iovar name including null */
	memcpy(p, iovar, namelen);
	p += namelen;

	/* bss config index as first param */
	bssidx = bssidx;
	memcpy(p, &bssidx, sizeof(int32));
	p += sizeof(int32);

	/* parameter buffer follows */
	if (paramlen) {
		memcpy(p, param, paramlen);
	}

	*perr = 0;
	return iolen;
}

int
dhd_bssiovar_getint(void *dhd, const char *iovar, int bssidx, int *pval)
{
	int ret;

	ret = dhd_bssiovar_get(dhd, iovar, bssidx, pval, sizeof(int));
	return ret;
}
#endif

/*
void
wl_printlasterror(char *name)
{
	char err_buf[WLC_IOCTL_SMLEN];
	strcpy(err_buf, "bcmerrstr");

	fprintf(stderr, "Error: ");
	if ( wl_ioctl(name, WLC_GET_VAR, err_buf, sizeof (err_buf)) != 0)
		fprintf(stderr, "Error getting the Errorstring from driver\n");
	else
		fprintf(stderr, err_buf);
}
*/

#ifdef RTCONFIG_HND_ROUTER_AX
/*
 * Set he subcommand to int value
 */
int
wl_heiovar_setint(char *ifname, char *iovar, char *subcmd, int val)
{
	char smbuf[WLC_IOCTL_SMLEN] = {0};
	he_xtlv_v32 v32;
	char *p = smbuf;
	int namelen, subcmd_len, iolen;

	if (strcmp(iovar, "he") != 0) {
		return BCME_BADARG;
	}

	/* length of iovar name + null */
	namelen = strlen(iovar) + 1;

	memset(&v32, 0, sizeof(v32));

	if (strcmp(subcmd, "features") == 0) {
		v32.id = WL_HE_CMD_FEATURES;
		v32.len = 4;
		v32.val = (uint32)val;

		subcmd_len = sizeof(v32.id) + sizeof(v32.len) + v32.len;
	} else if (strcmp(subcmd, "enab") == 0) {
		v32.id = WL_HE_CMD_ENAB;
		v32.len = 4;
		v32.val = (uint32)val;

		subcmd_len = sizeof(v32.id) + sizeof(v32.len) + v32.len;
	} else if (strcmp(subcmd, "bssaxmode") == 0) {
		v32.id = WL_HE_CMD_AXMODE;
		v32.len = 4;
		v32.val = (uint32)val;

		subcmd_len = sizeof(v32.id) + sizeof(v32.len) + v32.len;
	} else {
		return BCME_UNSUPPORTED;
	}

	iolen = namelen + subcmd_len;

	/* check for overflow */
	if (iolen > sizeof(smbuf)) {
		return BCME_BUFTOOSHORT;
	}

	/* copy iovar name including null */
	memcpy(p, iovar, namelen);
	p += namelen;

	/* copy he subcommand structure */
	memcpy(p, (void *)&v32, subcmd_len);

	return wl_ioctl(ifname, WLC_SET_VAR, (void *)smbuf, iolen);
}

/*
 * Set msched subcommand to int value
 */
int
wl_msched_iovar_setint(char *ifname, char *iovar, char *subcmd, int val)
{
	int ret = BCME_OK;
	wl_musched_cmd_params_t *musched_params = NULL;
	uint musched_cmd_param_size = sizeof(wl_musched_cmd_params_t);

	if ((musched_params = (wl_musched_cmd_params_t *)malloc(musched_cmd_param_size)) == NULL) {
		fprintf(stderr, "Error allocating %d bytes for musched_cmd params\n",
			musched_cmd_param_size);
		ret = BCME_NOMEM;
		goto fail;
	}

	/* Init musched_params */
	memset(musched_params, 0, musched_cmd_param_size);
	musched_params->bw = -1;
	musched_params->ac = -1;
	musched_params->row = -1;
	musched_params->col = -1;

	/* Init flags based on the one passed from caller */
	if (strcmp(iovar, "msched") == 0) {
		WL_MUSCHED_FLAGS_DL_SET(musched_params);
	} else if (strcmp(iovar, "umsched") == 0) {
		WL_MUSCHED_FLAGS_UL_SET(musched_params);
	}

	strncpy(musched_params->keystr, subcmd,
		MIN(strlen(subcmd), WL_MUSCHED_CMD_KEYSTR_MAXLEN));

	musched_params->vals[musched_params->num_vals++] = (int16)val;

	ret = wl_iovar_set(ifname, iovar, (void *)musched_params, musched_cmd_param_size);

fail:
	if (musched_params)
		free(musched_params);

	return ret;
}

int
wl_iovar_xtlv_setbuf(char *ifname, char *iovar, uint8* param, uint16 paramlen, uint16 version,
	uint16 cmd_id, uint16 xtlv_id, bcm_xtlv_opts_t opts, uint8 *bufptr, uint16 buf_len)
{
	bcm_iov_buf_t *iov_buf = NULL;
	uint8 *pxtlv = NULL;
	uint16 buflen = 0, buflen_start = 0;
	uint16 iovlen = 0;
	uint namelen;
	uint iolen;

	iov_buf = (bcm_iov_buf_t *)calloc(1, WLC_IOCTL_MEDLEN);

	/* fill header */
	iov_buf->version = version;
	iov_buf->id = cmd_id;

	pxtlv = (uint8 *)&iov_buf->data[0];
	buflen = buflen_start = WLC_IOCTL_MEDLEN - sizeof(bcm_iov_buf_t);
	if ((bcm_pack_xtlv_entry(&pxtlv, &buflen, xtlv_id, paramlen, param, opts)) != BCME_OK) {
		return BCME_ERROR;
	}
	iov_buf->len = buflen_start - buflen;
	iovlen = sizeof(bcm_iov_buf_t) + iov_buf->len;

	namelen = strlen(iovar) + 1;	 /* length of iovar name plus null */
	iolen = namelen + iovlen;

	/* check for overflow */
	if (iolen > buf_len) {
		return (BCME_BUFTOOSHORT);
	}

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8*)bufptr + namelen, (void*)iov_buf, iovlen);

	free(iov_buf);
	return wl_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}

int
wl_iovar_xtlv_set(char *ifname, char *iovar, uint8 *param, uint16 paramlen, uint16 version,
	uint16 cmd_id, uint16 xtlv_id, bcm_xtlv_opts_t opts)
{
	uint8 smbuf[WLC_IOCTL_SMLEN];

	return wl_iovar_xtlv_setbuf(ifname, iovar, param, paramlen, version, cmd_id, xtlv_id,
		opts, smbuf, sizeof(smbuf));
}

int
wl_iovar_xtlv_setint(char *ifname, char *iovar, int32 val, uint16 version,
	uint16 cmd_id, uint16 xtlv_id)
{
	return wl_iovar_xtlv_set(ifname, iovar, (uint8*)&val, sizeof(val), version,
		cmd_id, xtlv_id, BCM_XTLV_OPTION_ALIGN32);
}
#endif
