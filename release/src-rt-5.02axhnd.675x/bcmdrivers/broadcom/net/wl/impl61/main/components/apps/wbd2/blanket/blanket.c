/** @file blanket.c
 *  @brief Wi-Fi Blanket API 2.
 *
 * Broadcom Blanket library definitions
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * $Id: blanket.c 777559 2019-08-06 06:57:38Z $
 *
 *  @author
 *  @bug No known bugs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <shutils.h>
#include <bcmnvram.h>
#include <bcmendian.h>
#include <time.h>

#include <wlioctl.h>
#include <wlutils.h>
#include <wlif_utils.h>
#include <wlioctl_utils.h>
#include <common_utils.h>

#include "blanket.h"
#include "ieee1905.h"

/* Wi-Fi Blanket Error codes */
#define BKTE_OK			0
#define BKTE_MALLOC_FL		-2	/* Memory allocation failure */
#define BKTE_INV_ARG		-3	/* Invalid arguments */
#define BKTE_EMPTY_ASOCLST	-8	/* Assoclist empty or unavailable */
#define BKTE_INV_MAC		-9	/* Invalid MAC Address */
#define BKTE_WL_ERROR		-11	/* WL IOVAR error */
#define BKTE_INV_IFNAME		-81	/* Invalid interface name */

/* Wi-Fi Blanket Band Types */
#define	BKT_BAND_LAN_INVALID	0	/* 0 - auto-select */
#define	BKT_BAND_LAN_2G		1	/* 1 - 2.4 Ghz */
#define	BKT_BAND_LAN_5GL	2	/* 2 - 5 Ghz LOW */
#define	BKT_BAND_LAN_5GH	3	/* 3 - 5 Ghz HIGH */
#define	BKT_BAND_LAN_ALL	4	/* 4 - all bands */

/* Wi-Fi Blanket Buffer Sizes */
#define BKT_MAX_BUF_16		16
#define BKT_MAX_BUF_32		32
#define BKT_MAX_BUF_64		64
#define BKT_MAX_BUF_128		128
#define BKT_MAX_BUF_256		256
#define BKT_MAX_BUF_4096	4096
#define BKT_MAX_PROCESS_NAME	30	/* Maximum length of process NAME */

/* Macros for NSS value */
#define BKT_NSS_2		2
#define BKT_NSS_3		3
#define BKT_NSS_4		4
#define BKT_NSS_8		8

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Wi-Fi Blanket NVRAMs */
#define NVRAM_BCN_REQ_SUB_ELE		"bcn_req_sub_ele"
#define NVRAM_MAP_CERT			"map_cert"

#ifndef WL_STA_GBL_RCLASS
#define WL_STA_GBL_RCLASS		0x00400000
#endif /* WL_STA_GBL_RCLASS */

/* Default Beacon request Sub Elements */
#define DEF_BCN_REQ_SUB_ELE		"01020000020100"
/* 05(Category: Radio Measurement) 00 (Action: Radio Request) 00(Token) 0000(No. of Repetions)
 * 26(Member ID: measure req) 17(Len) 00(Token) 00(Measurement Request Mode)
 * 05(Measurement Type: Beacon Request) 00(Operating Class) 00(chan Number) 0000(interval)
 * 0001(Duration) 01(Measuremnt Mode: Active) ffffffffffff(BSSID: Wildcard)
 * 01020000(Beacon reporting subelement) 020100(Reporting detail subelement)
 */

/* The following defines are copied from wlc_txbf.h */
#define TXBF_HE_SU_BFR_CAP      0x04
#define TXBF_HE_MU_BFR_CAP      0x08

/* The following defines are copied from wlc_pub.h */
#define MU_FEATURES_MUTX        (1 << 0)
#define MU_FEATURES_MURX        (1 << 1)

/* Macros to handle endianess between dongle and host. */
/* Global variable to indicate endianess, set by wl_endian_probe on wireless interface */
extern bool gg_swap;
#define htod64(i) (gg_swap ? bcmswap64(i) : (uint64)(i))
#define htod32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))
#define htod16(i) (gg_swap ? bcmswap16(i) : (uint16)(i))
#define dtoh64(i) (gg_swap ? bcmswap64(i) : (uint64)(i))
#define dtoh32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))
#define dtoh16(i) (gg_swap ? bcmswap16(i) : (uint16)(i))
#define htodchanspec(i) (gg_swap ? htod16(i) : i)
#define dtohchanspec(i) (gg_swap ? dtoh16(i) : i)
#define htodenum(i) (gg_swap ? ((sizeof(i) == 4) ? htod32(i) :\
	((sizeof(i) == 2) ? htod16(i) : i)) : i)
#define dtohenum(i) (gg_swap ? ((sizeof(i) == 4) ? dtoh32(i) :\
	((sizeof(i) == 2) ? dtoh16(i) : i)) : i)

/* -------------------------- Blanket Debug Print Macros ------------------------ */

unsigned int bkt_msglevel = BKT_DEBUG_DEFAULT;
char g_bkt_process_name[BKT_MAX_PROCESS_NAME];

#define BKT_PRINT(prefix, fmt, arg...) \
	printf(prefix"BKT-%s >> (%lu) %s: "fmt, g_bkt_process_name, (unsigned long)time(NULL), \
	__FUNCTION__, ##arg)

#define BKT_ERROR(fmt, arg...) \
	if (bkt_msglevel & BKT_DEBUG_ERROR) \
		BKT_PRINT("Err: ", fmt, ##arg)

#define BKT_WARNING(fmt, arg...) \
	if (bkt_msglevel & BKT_DEBUG_WARNING) \
		BKT_PRINT("Warn: ", fmt, ##arg)

#define BKT_INFO(fmt, arg...) \
	if (bkt_msglevel & BKT_DEBUG_INFO) \
		BKT_PRINT("Info: ", fmt, ##arg)

#define BKT_DEBUG(fmt, arg...) \
	if (bkt_msglevel & BKT_DEBUG_DETAIL) \
		BKT_PRINT("Dbg: ", fmt, ##arg)

#define BKT_TRACE(fmt, arg...) \
	if (bkt_msglevel & BKT_DEBUG_TRACE) \
		BKT_PRINT("Trace: ", fmt, ##arg)

#define BKT_ENTER()	BKT_TRACE("Enter...\n")
#define BKT_EXIT()	BKT_TRACE("Exit...\n")

#define BKT_WL_DUMP_ENAB	(bkt_msglevel & BKT_DEBUG_DETAIL)

#define BCM_REFERENCE(data)	((void)(data))
/* -------------------------- Wi-Fi Blanket Debug Print Macros ------------------------ */

/* -------------------------------------------------------------------------------- */
/* ------- ASSERT /CHECK Macros, to avoid frequent checks, gives try/catch type utility ------- */
#define BKT_ASSERT_ARG(arg, ERR) \
		do { \
			if (!arg) { \
				ret = ERR; \
				goto end; \
			} \
		} while (0)
#define BKT_ASSERT() \
		do { \
			if (ret != BKTE_OK) { \
				goto end; \
			} \
		} while (0)
#define BKT_ASSERT_ERR(error) \
		do { \
			if (ret != BKTE_OK) { \
				ret = (error); \
				goto end; \
			} \
		} while (0)
#define BKT_ASSERT_MSG(fmt, arg...) \
		do { \
			if (ret != BKTE_OK) { \
				BKT_WARNING(fmt, ##arg); \
				goto end; \
			} \
		} while (0)

#define BKT_IOVAR_ASSERT(ifname, iovar, ret) \
		do { \
			if ((ret) < 0) { \
				BKT_WARNING("%s : IOVAR (%s) FAILED. Error : %d\n", \
					(ifname), (iovar), (ret)); \
				goto end; \
			} \
		} while (0)
/* ------- ASSERT /CHECK Macros, to avoid frequent checks, gives try/catch type utility -------- */
/* --------------------------------------------------------------------------------- */

/* -------------------------------------- Utility Macros -------------------------------------- */
#define BKT_STRNCPY(dst, src, len)	 \
	do { \
		strncpy(dst, src, len -1); \
		dst[len - 1] = '\0'; \
	} while (0)
/* -------------------------------------- Utility Macros -------------------------------------- */

/* ------------------------------ Structure & Global Declarations ------------------------------ */
/* IOCtl version read from targeted driver */
int ioctl_version;

/* dword align allocation */
union {
	char bufdata[WLC_IOCTL_MAXLEN];
	uint32 alignme;
} bufstruct_wlu;

char *g_bkt_cmdoutbuf = (char*) &bufstruct_wlu.bufdata;
/* ------------------------------ Structure & Global Declarations ------------------------------ */
/* Get version of wl to check whether FW supports new version */
static int blanket_get_ver_major(char *ifname);

/* Add static neighbor entry via legacy rrm_add_nbr iovar */
static int blanket_add_nbr_legacy(char *ifname, blanket_nbr_info_t *nbr);

/* Add static neighbor entry via rrm_add_nbr_v1 */
static int blanket_add_nbr_v1(char *ifname, blanket_nbr_info_t *nbr);

/* Generic memory Allocation function for WBD app */
void*
bkt_malloc(unsigned int len, int *error)
{
	int ret = BKTE_MALLOC_FL;
	void* pbuffer = NULL;
	BKT_ENTER();

	if (len <= 0) {
		goto end;
	}

	pbuffer = calloc(1, len);
	if (pbuffer == NULL) {
		goto end;
	} else {
		ret = BKTE_OK;
	}

end:
	if (ret != BKTE_OK) {
		BKT_ERROR("len[%d] Memory allocation failure\n", len);
	}
	if (error) {
		*error = ret;
	}

	BKT_EXIT();
	return pbuffer;
}

/* Initialize blanket module */
int blanket_module_init(blanket_module_info_t *info)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(info, BKTE_INV_ARG);

	bkt_msglevel = info->msglevel;
end:
	BKT_EXIT();
	return ret;
}

/* Check if interface is valid BRCM radio or not, and get ioctl version */
int
blanket_probe(char *ifname)
{
	return (ioctl_version = wl_probe(ifname));
}

/* Blanket Node get the current MAC address of Interface on this node */
int
blanket_get_bss_mac(char *ifname, struct ether_addr *out_mac)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_mac, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "cur_etheraddr", out_mac, ETHER_ADDR_LEN);
	BKT_IOVAR_ASSERT(ifname, "get cur_etheraddr", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Blanket Node get the current MAC address of the Radio (Primary Interface) */
int
blanket_get_radio_mac(char* primary_prefix, struct ether_addr *out_mac)
{
	int ret = BKTE_OK;
	char *ifname = NULL;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_mac, BKTE_INV_ARG);

	/* Get the primary ifname from the primary_prefix */
	ifname = blanket_nvram_prefix_safe_get(primary_prefix, "ifname");
	if (strlen(ifname) <= 0) {
		goto end;
	}

	ret = wl_iovar_get(ifname, "cur_etheraddr", out_mac, ETHER_ADDR_LEN);
	BKT_IOVAR_ASSERT(ifname, "get cur_etheraddr", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Deauth sta with reason code */
int
blanket_deauth_sta(char* ifname, struct ether_addr* addr, int reason)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(addr, BKTE_INV_ARG);

	if (reason) {
		/* send deauth with a valid reason */
		scb_val_t scb_val;
		memset(&scb_val, 0, sizeof(scb_val));
		memcpy(&scb_val.ea, addr, sizeof(scb_val.ea));

		scb_val.val = reason;
		ret = wl_ioctl(ifname, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val,
				sizeof(scb_val));
	} else {
		/* reason code 0 is reserved hence send deauth without reason */
		ret = wl_ioctl(ifname, WLC_SCB_DEAUTHENTICATE, addr, ETHER_ADDR_LEN);
	}

	BKT_IOVAR_ASSERT(ifname, "Deauth", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Return a new chanspec given a legacy chanspec
 * Returns INVCHANSPEC on error
 */
static chanspec_t
wl_chspec_from_legacy(chanspec_t legacy_chspec)
{
	chanspec_t chspec;

	/* get the channel number */
	chspec = LCHSPEC_CHANNEL(legacy_chspec);

	/* convert the band */
	if (LCHSPEC_IS2G(legacy_chspec)) {
		chspec |= WL_CHANSPEC_BAND_2G;
	} else {
		chspec |= WL_CHANSPEC_BAND_5G;
	}

	/* convert the bw and sideband */
	if (LCHSPEC_IS20(legacy_chspec)) {
		chspec |= WL_CHANSPEC_BW_20;
	} else {
		chspec |= WL_CHANSPEC_BW_40;
		if (LCHSPEC_CTL_SB(legacy_chspec) == WL_LCHANSPEC_CTL_SB_LOWER) {
			chspec |= WL_CHANSPEC_CTL_SB_L;
		} else {
			chspec |= WL_CHANSPEC_CTL_SB_U;
		}
	}

	if (wf_chspec_malformed(chspec)) {
		BKT_WARNING("wl_chspec_from_legacy: output chanspec (0x%04X) malformed\n",
		        chspec);
		return INVCHANSPEC;
	}

	return chspec;
}

#ifndef ATE_BUILD
/* given a chanspec value from the driver, do the endian and chanspec version conversion to
 * a chanspec_t value
 * Returns INVCHANSPEC on error
 */
static chanspec_t
wl_chspec_from_driver(chanspec_t chanspec)
{
	chanspec = dtohchanspec(chanspec);
	if (ioctl_version == 1) {
		chanspec = wl_chspec_from_legacy(chanspec);
	}

	return chanspec;
}
#endif /* !ATE_BUILD */

/* Return a legacy chanspec given a new chanspec
 * Returns INVCHANSPEC on error
 */
static chanspec_t
wl_chspec_to_legacy(chanspec_t chspec)
{
	chanspec_t lchspec;

	if (wf_chspec_malformed(chspec)) {
		BKT_DEBUG("input chanspec (0x%04X) malformed\n", chspec);
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
		BKT_DEBUG("unable to convert chanspec %s (0x%04X) to pre-11ac format\n",
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

	if (ioctl_version == 1) {
		chanspec = wl_chspec_to_legacy(chanspec);
		if (chanspec == INVCHANSPEC) {
			return chanspec;
		}
	}
	val = htod32((uint32)chanspec);

	return val;
}

/* given a chanspec value from the driver in a 32 bit integer, do the endian and
 * chanspec version conversion to a chanspec_t value
 * Returns INVCHANSPEC on error
 */
static chanspec_t
wl_chspec32_from_driver(uint32 chanspec32)
{
	chanspec_t chanspec;

	chanspec = (chanspec_t)dtoh32(chanspec32);

	if (ioctl_version == 1) {
		chanspec = wl_chspec_from_legacy(chanspec);
	}

	return chanspec;
}

/* To get the BSS information of Interface */
int
blanket_get_bss_info(char* ifname, wl_bss_info_t **out_bss_info)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi;

	*(uint32*)g_bkt_cmdoutbuf = htod32(WLC_IOCTL_MAXLEN);
	wl_endian_probe(ifname);
	/* Try getting BSS Info */
	ret = wl_ioctl(ifname, WLC_GET_BSS_INFO,
		g_bkt_cmdoutbuf, WLC_IOCTL_MAXLEN);
	BKT_IOVAR_ASSERT(ifname, "get BSS_INFO", ret);

	bi = (wl_bss_info_t*)(g_bkt_cmdoutbuf + 4);

	if (dtoh32(bi->version) == WL_BSS_INFO_VERSION ||
		dtoh32(bi->version) == LEGACY2_WL_BSS_INFO_VERSION ||
		dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {

		/* Convert version 107 to 109 */
		if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
			bi->ie_length = old_bi->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		} else {
			/* do endian swap and format conversion for chanspec if we have
			 * not created it from legacy bi above
			 */
			bi->chanspec = wl_chspec_from_driver(bi->chanspec);
		}
	} else {

		BKT_WARNING("ifname[%s] Sorry, your driver has bss_info_version %d "
			"but this program supports only version %d.\n",
			ifname, bi->version, WL_BSS_INFO_VERSION);
		ret = BKTE_WL_ERROR;
		goto end;
	}

	if (out_bss_info) {
		*out_bss_info = bi;
	}
end:
	return ret;
}

/* set chanspec at the interface */
int
blanket_set_chanspec(char *ifname, chanspec_t chanspec)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	if (wf_chspec_malformed(chanspec)) {
		BKT_WARNING("ifname[%s] wbd_wl_chspec_from_legacy: output chanspec (0x%04X) "
			"malformed\n", ifname, chanspec);
		return INVCHANSPEC;
	}
	ret = wl_iovar_setint(ifname, "chanspec", htod32(chanspec));
	BKT_IOVAR_ASSERT(ifname, "set chanspec", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get interface's chanspec */
int
blanket_get_chanspec(char *ifname, chanspec_t* out_chanspec)
{
	int ret = BKTE_OK;
	uint32 val = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_chanspec, BKTE_INV_ARG);
	wl_endian_probe(ifname);

	ret = wl_iovar_getint(ifname, "chanspec", (int*)&val);
	BKT_IOVAR_ASSERT(ifname, "get chanspec", ret);
	val = dtoh32(val);
	*out_chanspec = wl_chspec32_from_driver(val);

end:
	BKT_EXIT();
	return ret;
}

/* Get list of chanspec supported by driver */
int
blanket_get_chanspecs_list(char* ifname, wl_uint32_list_t* chlist, int chlistlen)
{
	int ret = BKTE_OK;
	int count = 0;
	int i = 0;
	chanspec_t chanspec, input = 0x00;

	BKT_ENTER();

	wl_endian_probe(ifname);

	BKT_ASSERT_ARG(chlist, BKTE_INV_ARG);

	ret = wl_iovar_getbuf(ifname, "chanspecs", &input, sizeof(input), chlist, chlistlen);
	BKT_IOVAR_ASSERT(ifname, "chanspecs", ret);

	count = dtoh32(chlist->count);

	if (count == 0x00) {
		goto end;
	}

	chlist->count = count;
	/* Copy all chanspecs */
	while (i < count) {
		chanspec = wl_chspec32_from_driver(chlist->element[i]);
		chlist->element[i] = chanspec;
		i++;
	}

	/* Just to ptint the chanspecs list */
	if (BKT_WL_DUMP_ENAB) {
		BKT_DEBUG("ifname[%s] Number of chanspecs are %d  and Chanspecs[", ifname, count);
		for (i = 0; i < chlist->count; i++) {
			printf("0x%x ", chlist->element[i]);
		}
		printf("]\n");
	}
end:
	BKT_EXIT();
	return ret;
}

/* Get Interface specific chan_info */
int
blanket_get_chan_info(char* ifname, uint channel, uint *out_bitmap)
{
	int ret = BKTE_OK;
	uint8 buf[BKT_MAX_BUF_32] = {0};
	uint32 chanspec_arg;
	BKT_ENTER();

	wl_endian_probe(ifname);

	chanspec_arg = CH20MHZ_CHSPEC(channel);

	/* there should be no problem if converting to a legacy chanspec
	 * since chanspec_arg is created as 20MHz
	 */
	chanspec_arg = wl_chspec32_to_driver(chanspec_arg);
	ret = wl_iovar_getbuf(ifname, "per_chan_info", &chanspec_arg,
		sizeof(chanspec_arg), buf, sizeof(buf));
	BKT_IOVAR_ASSERT(ifname, "per_chan_info", ret);

	/* Copy the output bitmap */
	if (out_bitmap) {
		*out_bitmap = dtoh32(*(uint *)buf);
	}

end:
	BKT_EXIT();
	return ret;
}

/* Get STA Info */
int
blanket_get_sta_info(char* ifname, struct ether_addr* addr, sta_info_t *out_sta_info)
{
	int ret = BKTE_OK;
	char ioctl_buf[WLC_IOCTL_MEDLEN];
	BKT_ENTER();

	wl_endian_probe(ifname);
	/* Validate arg */
	BKT_ASSERT_ARG(out_sta_info, BKTE_INV_ARG);
	BKT_ASSERT_ARG(addr, BKTE_INV_ARG);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));

	ret = wl_iovar_getbuf(ifname, "sta_info", addr, sizeof(*addr), ioctl_buf,
		sizeof(ioctl_buf));
	BKT_IOVAR_ASSERT(ifname, "get STA INFO", ret);

	memcpy(out_sta_info, ioctl_buf, sizeof(*out_sta_info));
	out_sta_info->ver = dtoh16(out_sta_info->ver);

	/* Report unrecognized version */
	if (out_sta_info->ver > WL_STA_VER) {
		BKT_WARNING("ifname[%s] STA["MACF"] Unknown driver station info version %d\n",
			ifname, ETHERP_TO_MACF(addr), out_sta_info->ver);
		ret = BKTE_WL_ERROR;
		goto end;
	}

	out_sta_info->flags = dtoh32(out_sta_info->flags);
	out_sta_info->rateset.count = dtoh32(out_sta_info->rateset.count);
	out_sta_info->listen_interval_inms = dtoh32(out_sta_info->listen_interval_inms);
	out_sta_info->len = dtoh16(out_sta_info->len);

	/* Driver didn't return extended station info */
#if defined(WL_STA_VER_7)
	if (out_sta_info->ver < WL_STA_VER_7) {
#else
	if (out_sta_info->len < sizeof(*out_sta_info)) {
#endif // endif
		BKT_WARNING("ifname[%s] STA["MACF"] Version[%d] sta->len[%d] "
			"sizeof(sta_info_t)[%zu] Driver didn't return extended station info\n",
			ifname, ETHERP_TO_MACF(addr), out_sta_info->ver, out_sta_info->len,
			sizeof(*out_sta_info));
		ret = BKTE_WL_ERROR;
		goto end;
	}

end:
	BKT_EXIT();
	return ret;
}

/* Get the current Band of Interface */
int
blanket_get_band(char* ifname, int *out_band)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);
	/* Validate arg */
	BKT_ASSERT_ARG(out_band, BKTE_INV_ARG);

	ret = wl_ioctl(ifname, WLC_GET_BAND, out_band, sizeof(*out_band));
	*out_band = dtoh32(*out_band);
	BKT_IOVAR_ASSERT(ifname, "get band", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get the BSSID of the interface and return */
int
blanket_get_bssid(char *ifname, struct ether_addr *out_bssid)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_bssid, BKTE_INV_ARG);

	ret = wl_ioctl(ifname, WLC_GET_BSSID, out_bssid, ETHER_ADDR_LEN);
	BKT_IOVAR_ASSERT(ifname, "get BSSID", ret);

	/* The adapter is associated */
	BKT_DEBUG("ifname : %s BSSID : "MACF"\n", ifname, ETHER_TO_MACF((*out_bssid)));

end:
	BKT_EXIT();
	return ret;
}

/* Get the BSSID of Interface */
int
blanket_try_get_bssid(char *ifname, int max_try, int useconds_gap, struct ether_addr *out_bssid)
{
	int ret = BKTE_OK;
	int ncount = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_bssid, BKTE_INV_ARG);

	while (ncount < max_try) {
		ret = blanket_get_bssid(ifname, out_bssid);
		if (ETHER_ISNULLADDR(out_bssid)) {
			usleep(useconds_gap * 1000);
			ncount++;
			continue;
		}
		break;
	}
	BKT_IOVAR_ASSERT(ifname, "get BSSID", ret);

	/* The adapter is associated */
	BKT_DEBUG("ifname : %s BSSID : "MACF"\n", ifname, ETHER_TO_MACF((*out_bssid)));

	if (ETHER_ISNULLADDR(out_bssid)) {
		ret = BKTE_INV_MAC;
	}
end:
	BKT_EXIT();
	return ret;
}

/* Get the SSID */
int
blanket_get_ssid(char* ifname, wlc_ssid_t *out_ssid)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_ssid, BKTE_INV_ARG);

	ret = wl_ioctl(ifname, WLC_GET_SSID, out_ssid, sizeof(*out_ssid));
	BKT_IOVAR_ASSERT(ifname, "get SSID", ret);

	BKT_DEBUG("ifname : %s SSID : %s\n", ifname, out_ssid->SSID);

end:
	BKT_EXIT();
	return ret;
}

/* Join the SSID with assoc parameters provided */
int
blanket_join_ssid(char* ifname, wl_join_params_t *join_params, uint join_params_size)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(join_params, BKTE_INV_ARG);

	ret = wl_ioctl(ifname, WLC_SET_SSID, join_params, join_params_size);
	BKT_IOVAR_ASSERT(ifname, "set JOIN", ret);

	BKT_DEBUG("ifname : %s SSID : %s\n", ifname, join_params->ssid.SSID);

end:
	BKT_EXIT();
	return ret;
}

/* Get sta rssi */
int
blanket_get_rssi(char* ifname, struct ether_addr* addr, int* out_rssi)
{
	int ret = BKTE_OK;
	scb_val_t scb_val;
	BKT_ENTER();

	wl_endian_probe(ifname);
	/* Validate arg */
	BKT_ASSERT_ARG(out_rssi, BKTE_INV_ARG);

	memset(&scb_val, 0, sizeof(scb_val));
	if (addr) {
		memcpy(&scb_val.ea, addr, sizeof(scb_val.ea));
	}

	ret = wl_ioctl(ifname, WLC_GET_RSSI, &scb_val, sizeof(scb_val));
	BKT_IOVAR_ASSERT(ifname, "get RSSI", ret);

	*out_rssi = dtoh32(scb_val.val);

	BKT_DEBUG("ifname : %s RSSI : %d\n", ifname, *out_rssi);
end:
	return ret;
}

/* Get rate of interface */
int
blanket_get_rate(char *ifname, int *out_rate)
{
	int ret = BKTE_OK;
	int rate = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_rate, BKTE_INV_ARG);

	wl_endian_probe(ifname);
	ret = wl_ioctl(ifname, WLC_GET_RATE, &rate, sizeof(rate));
	BKT_IOVAR_ASSERT(ifname, "rate", ret);
	rate = dtoh32(rate);
	/* rate here comes in units of 500 Kbits/s
	 * and we will return it in units of 1000 Kbits/s
	 */
	*out_rate = rate / 2;
end:
	BKT_EXIT();
	return ret;
}

/* Get Regulatory class of Interface */
int
blanket_get_rclass(char* ifname, chanspec_t chspec, uint8* out_rclass)
{
	int ret = BKTE_OK;
	uint8 buf[BKT_MAX_BUF_16] = {0};
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_rclass, BKTE_INV_ARG);

	ret = wl_iovar_getbuf(ifname, "rclass", &chspec, sizeof(chspec), buf, BKT_MAX_BUF_16);
	BKT_IOVAR_ASSERT(ifname, "get rclass", ret);

	*out_rclass = *buf;
end:

	BKT_EXIT();
	return BKTE_OK;
}

static int
blanket_get_ver_major(char *ifname)
{
	int ret = BKTE_OK;
	wl_wlc_version_t wlc_ver;

	memset(&wlc_ver, 0, sizeof(wlc_ver));
	ret = wl_iovar_get(ifname, "wlc_ver", &wlc_ver, sizeof(wl_wlc_version_t));
	BKT_IOVAR_ASSERT(ifname, "wlc_ver", ret);

	BKT_INFO("major version for %s:%d\n", ifname, wlc_ver.wlc_ver_major);
	return wlc_ver.wlc_ver_major;
end:
	return BKTE_OK;
}

/* Add neighbor */
static int
blanket_add_nbr_v1(char* ifname, blanket_nbr_info_t *nbr)
{
	int ret = BKTE_OK;
	int buflen, command_len;
	char *param, ioctl_buf[BKT_MAX_BUF_128] = {0};
	nbr_rpt_elem_t *nbr_ie;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(nbr, BKTE_INV_ARG);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));

	BKT_STRNCPY(ioctl_buf, "rrm_nbr_add_nbr", strlen("rrm_nbr_add_nbr") + 1);
	buflen = strlen(ioctl_buf) + 1;

	param = (char *)(ioctl_buf + buflen);

	nbr_ie = (nbr_rpt_elem_t *)param;
	eacopy(&(nbr->bssid), &nbr_ie->bssid);
	htol32_ua_store(nbr->bssid_info, &(nbr_ie->bssid_info));
	nbr_ie->reg = nbr->reg;
	nbr_ie->channel = nbr->channel;
	nbr_ie->phytype = nbr->phytype;
	memcpy(&nbr_ie->ssid.SSID, &nbr->ssid.SSID, nbr->ssid.SSID_len);
	nbr_ie->ssid.SSID_len = nbr->ssid.SSID_len;
	nbr_ie->version = WL_RRM_NBR_RPT_VER;
	nbr_ie->id = DOT11_MNG_NEIGHBOR_REP_ID;
	nbr_ie->bss_trans_preference = 255; /* as statically added */
	nbr_ie->len = sizeof(nbr_rpt_elem_t);

	BKT_INFO("nbr->len[%d], nbr->id[%d], nbr->version[%d], ssid [%s], ssid len[%d] \n",
		nbr_ie->len, nbr_ie->id, nbr_ie->version, nbr_ie->ssid.SSID,
		nbr_ie->ssid.SSID_len);

	command_len = buflen + sizeof(*nbr_ie);

	ret = wl_ioctl(ifname, WLC_SET_VAR, ioctl_buf, command_len);
	BKT_IOVAR_ASSERT(ifname, "rrm_nbr_add_nbr", ret);
end:
	BKT_EXIT();
	return ret;
}

/* Add neighbor */
static int
blanket_add_nbr_legacy(char *ifname, blanket_nbr_info_t *nbr)
{
	int ret = BKTE_OK;
	int buflen, command_len;
	char *param, ioctl_buf[BKT_MAX_BUF_64] = {0};
	nbr_element_t *nbr_ie;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(nbr, BKTE_INV_ARG);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));

	BKT_STRNCPY(ioctl_buf, "rrm_nbr_add_nbr", strlen("rrm_nbr_add_nbr") + 1);
	buflen = strlen(ioctl_buf) + 1;

	param = (char *)(ioctl_buf + buflen);

	nbr_ie = (nbr_element_t *)param;
	nbr_ie->id = DOT11_MNG_NEIGHBOR_REP_ID;
	nbr_ie->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN;
	eacopy(&(nbr->bssid), &nbr_ie->bssid);
	htol32_ua_store(nbr->bssid_info, &(nbr_ie->bssid_info));
	nbr_ie->reg = nbr->reg;
	nbr_ie->channel = nbr->channel;
	nbr_ie->phytype = nbr->phytype;

	command_len = buflen + sizeof(*nbr_ie);

	ret = wl_ioctl(ifname, WLC_SET_VAR, ioctl_buf, command_len);
	BKT_IOVAR_ASSERT(ifname, "rrm_nbr_add_nbr", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Add neighbor */
int
blanket_add_nbr(char *ifname, blanket_nbr_info_t *bkt_nbr)
{
	int ret = BKTE_OK;

	BKT_ENTER();
	/* Validate arg */
	BKT_ASSERT_ARG(ifname, BKTE_INV_ARG);
	BKT_ASSERT_ARG(bkt_nbr, BKTE_INV_ARG);

	if (blanket_get_ver_major(ifname)) {
		ret = blanket_add_nbr_v1(ifname, bkt_nbr);
	} else {
		ret = blanket_add_nbr_legacy(ifname, bkt_nbr);
	}
end:
	BKT_EXIT();
	return ret;
}

/* Delete neighbor */
int
blanket_del_nbr(char *ifname, struct ether_addr *nbr)
{
	int ret = BKTE_OK;
	int buflen, command_len;
	char *param, ioctl_buf[BKT_MAX_BUF_32] = {0};
	struct ether_addr *ea;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(nbr, BKTE_INV_ARG);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));

	BKT_STRNCPY(ioctl_buf, "rrm_nbr_del_nbr", strlen("rrm_nbr_del_nbr") + 1);
	buflen = strlen(ioctl_buf) + 1;

	param = (char *)(ioctl_buf + buflen);
	ea = (struct ether_addr *)param;

	eacopy(nbr, ea);

	command_len = buflen + ETHER_ADDR_LEN;

	ret = wl_ioctl(ifname, WLC_SET_VAR, ioctl_buf, command_len);
	BKT_IOVAR_ASSERT(ifname, "rrm_nbr_del_nbr", ret);

end:
	BKT_EXIT();
	return ret;
}

/* To Check BSS Transition Supported or not */
int
blanket_is_bss_trans_supported(char *ifname, struct ether_addr *sta_mac)
{
	int ret = 0, sta_info_ret = 0;
	sta_info_t sta_info;

	sta_info_ret = blanket_get_sta_info(ifname, sta_mac, &sta_info);
	if (sta_info_ret == BKTE_OK) {
		if (sta_info.wnm_cap & WL_WNM_BSSTRANS) {
			ret = 1;
		}
	}

	return ret;
}

/* To Check global rclass Supported or not */
int
blanket_is_global_rclass_supported(char *ifname, struct ether_addr *sta_mac)
{
	int ret = 0, sta_info_ret = 0;
	sta_info_t sta_info;

	sta_info_ret = blanket_get_sta_info(ifname, sta_mac, &sta_info);
	if (sta_info_ret == BKTE_OK) {
		if (sta_info.flags & WL_STA_GBL_RCLASS) {
			ret = 1;
		}
	}

	return ret;
}

/* Get APSTA configuration of interface */
int
blanket_get_apsta_mode(char *ifname, uchar *apsta)
{
	int ret = BKTE_OK;
	int apsta_mode = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(apsta, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "apsta", &apsta_mode, sizeof(apsta_mode));
	BKT_IOVAR_ASSERT(ifname, "apsta", ret);

	*apsta = (unsigned char)apsta_mode;

end:
	BKT_EXIT();
	return ret;
}

/* Get HT Capabilities from bss_info */
int
blanket_get_ht_cap(char *ifname, wl_bss_info_t *in_bi, uint8 *out_ht_caps)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;
	int max_nss = 0;
	uint8 caps = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_ht_caps, BKTE_INV_ARG);

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, &bi);
		BKT_ASSERT();
	} else {
		bi = in_bi;
	}

	if (bi->n_cap == FALSE) {
		goto end;
	}

	/* Get max_nss */
	max_nss = wl_wlif_get_max_nss(bi);

	/* Bit 0 is reserved.
	 * 1	HT support for 40MHz.
	 * 2	SGI support for 40MHz.
	 * 3	SGI suppoprt for 20 mhz.
	 */
	if (bi->nbss_cap & HT_CAP_40MHZ) {
		caps |= IEEE1905_AP_HTCAP_40MHZ;
	}
	if (bi->nbss_cap & HT_CAP_SHORT_GI_40) {
		caps |= IEEE1905_AP_HTCAP_SGI_40MHZ;
	}
	if (bi->nbss_cap & HT_CAP_SHORT_GI_20) {
		caps |= IEEE1905_AP_HTCAP_SGI_20MHZ;
	}

	/* Bits 5-4 are used for max Rx Chains and 7-6 for Tx chains with below mapping:
	 *		Rx			Tx		NSS
	 * Bits		5  4			7  6		Value
	 *
	 *		0  0			0  0		1
	 *		0  1			0  1		2
	 *		1  0			1  0		3
	 *		1  1			1  1		4
	 */
	switch (max_nss) {
		case BKT_NSS_2:
			caps |= IEEE1905_AP_HTCAP_RX_NSS_2;
			caps |= IEEE1905_AP_HTCAP_TX_NSS_2;
		break;

		case BKT_NSS_3:
			caps |= IEEE1905_AP_HTCAP_RX_NSS_3;
			caps |= IEEE1905_AP_HTCAP_TX_NSS_3;
		break;

		case BKT_NSS_4:
			caps |= IEEE1905_AP_HTCAP_RX_NSS_4;
			caps |= IEEE1905_AP_HTCAP_TX_NSS_4;
		break;

		default:
		break;
	}

	*out_ht_caps = caps;

end:
	BKT_EXIT();
	return ret;
}

/* Get VHT MCS Capabilities from bss_info */
int
blanket_get_vht_cap(char *ifname, wl_bss_info_t *in_bi, blanket_vht_caps_t *out_vht_caps)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;
	int max_nss = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_vht_caps, BKTE_INV_ARG);

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, &bi);
		BKT_ASSERT();
	} else {
		bi = in_bi;
	}

	if (bi->vht_cap == FALSE) {
		goto end;
	}

	out_vht_caps->valid = 1;

	/* Get max_nss */
	max_nss = wl_wlif_get_max_nss(bi);

	/* Set caps, bits 0-3 are reserved.
	 * 4	MU Beamformer capable.
	 * 5	SU Beamformer capable.
	 * 6	VHT support for 160 MHz.
	 * 7	VHT support for 80+80 MHz.
	 */
	if (bi->nbss_cap & VHT_BI_CAP_MU_BEAMFMR) {
		out_vht_caps->caps |= IEEE1905_AP_VHTCAP_MU_BEAMFMR;
	}
	if (bi->nbss_cap & VHT_BI_CAP_SU_BEAMFMR) {
		out_vht_caps->caps |= IEEE1905_AP_VHTCAP_SU_BEAMFMR;
	}
	if (bi->nbss_cap & VHT_BI_160MHZ) {
		out_vht_caps->caps |= IEEE1905_AP_VHTCAP_160MHZ;
	}
	if (bi->nbss_cap & VHT_BI_8080MHZ) {
		out_vht_caps->caps |= IEEE1905_AP_VHTCAP_80p80MHZ;
	}

	/* Set caps_ex.
	 * 0	SGI support for 160 MHz and 80+80 MHz.
	 * 1	SGI support for 80 MHz.
	 */
	if (bi->nbss_cap & VHT_BI_SGI_160MHZ) {
		out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_SGI_160MHZ;
	}
	if (bi->nbss_cap & VHT_BI_SGI_80MHZ) {
		out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_SGI_80MHZ;
	}
	/* Bits 4-2 are used for max Rx Chains and 7-5 for Tx chains with below mapping:
	 *		Rx			Tx		NSS
	 * Bits		4  3  2			7  6  5		Value
	 *
	 *		0  0  0			0  0  0		1
	 *		0  0  1			0  0  1		2
	 *		0  1  0			0  1  0		3
	 *		0  1  1			0  1  1		4
	 *		1  1  1			1  1  1		8
	 */
	switch (max_nss) {
		case BKT_NSS_2:
			out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_RX_NSS_2;
			out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_TX_NSS_2;
		break;

		case BKT_NSS_3:
			out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_RX_NSS_3;
			out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_TX_NSS_3;
		break;

		case BKT_NSS_4:
			out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_RX_NSS_4;
			out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_TX_NSS_4;
		break;

		case BKT_NSS_8:
			out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_RX_NSS_8;
			out_vht_caps->caps_ex |= IEEE1905_AP_VHTCAP_TX_NSS_8;
		break;

		default:
		break;
	}

	/* Fill the vht_tx/rx mcs map. */
	out_vht_caps->vht_txmcsmap = dtoh16(bi->vht_txmcsmap);
	out_vht_caps->vht_rxmcsmap = dtoh16(bi->vht_rxmcsmap);

end:
	BKT_EXIT();
	return ret;
}

/* Get the txbf_bfr_cap of the Radio (Primary Interface) */
static int
blanket_get_bfr_cap(char *ifname, int *out_bfr_cap)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_bfr_cap, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "txbf_bfr_cap", out_bfr_cap, sizeof(int));
	BKT_IOVAR_ASSERT(ifname, "get txbf_bfr_cap", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get the mu_features of the Radio (Primary Interface) */
static int
blanket_get_mu_features(char *ifname, int *out_mu_features)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_mu_features, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "mu_features", out_mu_features, sizeof(int));
	BKT_IOVAR_ASSERT(ifname, "get mu_features", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get HE MCS Capabilities from bss_info */
int
blanket_get_he_cap(char *ifname, wl_bss_info_t *in_bi, blanket_he_caps_t *out_he_caps)
{
	int ret = BKTE_OK;
	wl_bss_info_v109_1_t *bi;
	int max_nss = 0, bfr_cap = 0, mu_features = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_he_caps, BKTE_INV_ARG);

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, (wl_bss_info_t **)&bi);
		BKT_ASSERT();
	} else {
		bi = (wl_bss_info_v109_1_t *)in_bi;
	}

	if (!bi->he_cap) {
		goto end;
	}

	/* Get bfr Cap */
	(void)blanket_get_bfr_cap(ifname, &bfr_cap);
	/* Get MU Features */
	(void)blanket_get_mu_features(ifname, &mu_features);

	/* Set Caps, bit 0 is reserved
	 * 7	SU Beamformer Capable
	 * 6	MU Beamformer Capable
	 * 5	UL MU-MIMO Capable
	 * 4	UL MU-MIMO + OFDMA Capable
	 * 3	DL MU-MIMO + OFDMA Capable
	 * 2	UL OFDMA Capable
	 * 1	DL OFDMA Capable
	 * 0	Reserved
	*/

	if (bfr_cap & TXBF_HE_SU_BFR_CAP) {
		out_he_caps->caps |= IEEE1905_AP_HECAP_SU_BEAMFMR;
	}
	if (bfr_cap & TXBF_HE_MU_BFR_CAP) {
		out_he_caps->caps |= IEEE1905_AP_HECAP_MU_BEAMFMR;
	}
	if (mu_features & MU_FEATURES_MURX) {
		out_he_caps->caps |= IEEE1905_AP_HECAP_UL_MUMIMO;
	}

	/* XXX: To Do. currently wl msched iovar doesnt return a structure instead a printable
	 * buffer. once iovar is in place then we will get current ofdma cap from firmware
	*/
	if (mu_features & MU_FEATURES_MURX) {
		out_he_caps->caps |= IEEE1905_AP_HECAP_UL_MUMIMO_OFDMA;
	}
	if (mu_features & MU_FEATURES_MUTX) {
		out_he_caps->caps |= IEEE1905_AP_HECAP_DL_MUMIMO_OFDMA;
	}
	out_he_caps->caps |= IEEE1905_AP_HECAP_UL_OFDMA;
	out_he_caps->caps |= IEEE1905_AP_HECAP_DL_OFDMA;

	/* Set 160MHz and 80p80MHz bits if valid respective mcs maps are present */
	if ((bi->he_sup_bw160_tx_mcs != 0xffff) || (bi->he_sup_bw160_rx_mcs != 0xffff)) {
		out_he_caps->caps_ex |= IEEE1905_AP_HECAP_160MHZ;
	}
	if ((bi->he_sup_bw80p80_tx_mcs != 0xffff) || (bi->he_sup_bw80p80_rx_mcs != 0xffff)) {
		out_he_caps->caps_ex |= IEEE1905_AP_HECAP_80P80MHZ;
	}

	/* Get max_nss */
	max_nss = wl_wlif_get_max_nss((wl_bss_info_t *)bi);

	BKT_DEBUG("max_nss[%x] bfr_cap[%x] mu_features[%x]\n", max_nss, bfr_cap, mu_features);
	/* Bits 4-2 are used for max Rx Chains and 7-5 for Tx chains with below mapping:
	 *		Rx			Tx		NSS
	 * Bits		4  3  2			7  6  5		Value
	 *
	 *		0  0  0			0  0  0		1
	 *		0  0  1			0  0  1		2
	 *		0  1  0			0  1  0		3
	 *		0  1  1			0  1  1		4
	 *		1  1  1			1  1  1		8
	 */
	switch (max_nss) {
		case BKT_NSS_2:
			out_he_caps->caps_ex |= IEEE1905_AP_HECAP_RX_NSS_2;
			out_he_caps->caps_ex |= IEEE1905_AP_HECAP_TX_NSS_2;
		break;

		case BKT_NSS_3:
			out_he_caps->caps_ex |= IEEE1905_AP_HECAP_RX_NSS_3;
			out_he_caps->caps_ex |= IEEE1905_AP_HECAP_TX_NSS_3;
		break;

		case BKT_NSS_4:
			out_he_caps->caps_ex |= IEEE1905_AP_HECAP_RX_NSS_4;
			out_he_caps->caps_ex |= IEEE1905_AP_HECAP_TX_NSS_4;
		break;

		case BKT_NSS_8:
			out_he_caps->caps_ex |= IEEE1905_AP_HECAP_RX_NSS_8;
			out_he_caps->caps_ex |= IEEE1905_AP_HECAP_TX_NSS_8;
		break;

		default:
		break;
	}

	/* Fill the he tx/rx mcs map. */
	out_he_caps->he_bw80_txmcsmap = dtoh16(bi->he_sup_bw80_tx_mcs);
	out_he_caps->he_bw80_rxmcsmap = dtoh16(bi->he_sup_bw80_rx_mcs);
	out_he_caps->he_bw160_txmcsmap = dtoh16(bi->he_sup_bw160_tx_mcs);
	out_he_caps->he_bw160_rxmcsmap = dtoh16(bi->he_sup_bw160_rx_mcs);
	out_he_caps->he_bw80p80_txmcsmap = dtoh16(bi->he_sup_bw80p80_tx_mcs);
	out_he_caps->he_bw80p80_rxmcsmap = dtoh16(bi->he_sup_bw80p80_rx_mcs);

	out_he_caps->valid = 1;

end:
	BKT_EXIT();
	return ret;
}

/* Get Basic Capabilities */
int
blanket_get_basic_cap(uint8 *out_basic_caps)
{
	int ret = BKTE_OK;
	uint8 caps = 0x0;
	BKT_ENTER();

	/* Bits 0-4 are reserved.
	 * 5	Support agent-initiated RSSI based steering - Unsupported
	 * 6	Support unassociated STA link matrix on channels its BSS are not operating
	 *	- Supported
	 * 7	Support unassociated STA link matrix on channels its BSS are operating - Supported
	 */

	caps |= IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT_NON_CH;
	caps |= IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT;

	if (out_basic_caps) {
		*out_basic_caps = caps;
	}

	BKT_EXIT();
	return ret;
}

/* Regulatory class to chspec map.
 * Note : Functions "blanket_get_radio_cap" and "blanket_get_bw_from_rc" depends on this
 * "rc_map" variable. Take care of these two functions upon modifying these values
 */
static uint16 rc_map[] =
{
	/* 2g 20MHz */
	81, 13,
	0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007, 0x1008,
	0x1009, 0x100a,	0x100b, 0x100c, 0x100d,

	82, 1,
	0x100e,

	/* 2g 40MHz */
	83, 9,
	0x1803, 0x1804, 0x1805, 0x1806, 0x1807, 0x1808, 0x1809, 0x180a,
	0x180b,

	84, 9,
	0x1903, 0x1904, 0x1905, 0x1906, 0x1907, 0x1908, 0x1909, 0x190a,
	0x190b,

	/* 5g 20Mhz UNII-1 */
	115, 4,
	0xd024, 0xd028, 0xd02c, 0xd030,

	/* 5g 20Mhz UNII-2 (DFS) */
	118, 4,
	0xd034, 0xd038, 0xd03c, 0xd040,

	/* 5g 20Mhz UNII-2ext (DFS) */
	121, 12,
	0xd064, 0xd068, 0xd06c, 0xd070, 0xd074, 0xd078, 0xd07c, 0xd080,
	0xd084, 0xd088, 0xd08c, 0xd090,

	/* 5g 20Mhz UNII-3 */
	124, 4,
	0xd095, 0xd099, 0xd09d, 0xd0a1,

	/* 5g 20Mhz UNII-3 */
	125, 6,
	0xd095, 0xd099, 0xd09d, 0xd0a1, 0xd0a5, 0xd0a9,

	/* 5g 40Mhz L UNII-1 */
	116, 2,
	0xd826, 0xd82e,

	/* 5g 40Mhz L UNII-2 (DFS) */
	119, 2,
	0xd836, 0xd83e,

	/* 5g 40Mhz L UNII-2ext (DFS) */
	122, 6,
	0xd866, 0xd86e, 0xd876, 0xd87e, 0xd886, 0xd88e,

	/* 5g 40Mhz L UNII-3 */
	126, 2,
	0xd897, 0xd89f,

	/* 5g 40Mhz U UNII-1 */
	117, 2,
	0xd926, 0xd92e,

	/* 5g 40Mhz U UNII-2 (DFS) */
	120, 2,
	0xd936, 0xd93e,

	/* 5g 40Mhz U UNII-2ext (DFS) */
	123, 6,
	0xd966, 0xd96e, 0xd976, 0xd97e, 0xd986, 0xd98e,

	/* 5g 40Mhz U UNII-3 */
	127, 2,
	0xd997, 0xd99f,
	/* 5g 80Mhz */
	128, 24,
	0xe02a, 0xe03a, 0xe06a, 0xe07a, 0xe08a, 0xe09b, 0xe12a, 0xe13a,
	0xe16a, 0xe17a, 0xe18a, 0xe19b, 0xe22a, 0xe23a, 0xe26a, 0xe27a,
	0xe28a, 0xe29b, 0xe32a, 0xe33a, 0xe36a, 0xe37a, 0xe38a, 0xe39b,

	/* 5g 160Mhz */
	129, 16,
	0xe832, 0xe932, 0xea32, 0xeb32, 0xec32, 0xed32, 0xee32, 0xef32,
	0xe872, 0xe972, 0xea72, 0xeb72, 0xec72, 0xed72, 0xee72, 0xef72
};

const int rc_max = sizeof(rc_map)/sizeof(rc_map[0]);

/* Get Banwidth from regulatory class */
int
blanket_get_bw_from_rc(uint8 rc, uint *bw)
{
	uint idx = 0;
	chanspec_t chspec = 0;

	if (!bw) {
		return -1;
	}

	for (idx = 0; idx < rc_max; idx++) {
		uint8 num_of_entries = rc_map[idx+1];

		/* Regulatory class matches */
		if (rc_map[idx] == rc && num_of_entries > 0) {
			/* Get one chanspec to get the bandwidth from it */
			chspec = rc_map[idx+2];
			*bw = CHSPEC_BW(chspec);
			return 0;
		}
		idx++; /* One for number of entries */
		idx += num_of_entries;
	}

	return -1;
}

/* Get global operating class based on chanspec */
int
blanket_get_global_rclass(chanspec_t chanspec, uint8 *out_rclass)
{
	uint idx = 0, start, endpos, nrc;
	uint8 rc;

	if (!out_rclass) {
		return -1;
	}
	*out_rclass = 0;

	do {
		rc = rc_map[idx++];
		nrc = rc_map[idx++];
		start = idx;
		endpos = start + nrc;
		for (; idx < endpos && idx < rc_max; idx++) {
			if (chanspec == rc_map[idx]) {
				*out_rclass = rc;
				return 0;
			}
		}
	} while (idx < rc_max);

	return -1;
}

/* Get Maximum Tx Power of Interface */
int
blanket_get_txpwr_target_max(char *ifname, int *txpower)
{
	int ret = BKTE_OK, tx = 0;
	int val = 0;
	txpwr_target_max_t tx_pwr;
	int8 iter;
	BKT_ENTER();

	ret = wl_ioctl(ifname, WLC_GET_UP, &val, sizeof(val));
	BKT_IOVAR_ASSERT(ifname, "WLC_GET_UP", ret);

	ret = wl_iovar_get(ifname, "txpwr_target_max", &tx_pwr, sizeof(tx_pwr));
	BKT_IOVAR_ASSERT(ifname, "txpwr_target_max", ret);

	/* Get the Maximum of Tx Power of all Antenas */
	for (iter = 0; iter < WL_STA_ANT_MAX; iter++) {
		tx = ((tx < tx_pwr.txpwr[iter])? tx_pwr.txpwr[iter] : tx);
	}
	/* Convert qdBm(Quarter) to dBm */
	tx /= 4;

end:
	if (txpower) {
		*txpower = tx;
	}

	BKT_EXIT();
	return ret;
}

/* Fill radio capability list:
 * 1. Get the list of chanpecs supported by this radio based on bandwidth cap
 * 2. Get the list of supported operating classes by this radio
 * 3. Get the list of all chanspecs present in those operating classes
 * 4. See if there is any difference between the chanspecs lists 1 & 3
 *    and report those as unsupported chanspecs.
 */
int
blanket_get_radio_cap(char *ifname, blanket_radio_caps_t *out_radio_caps, int max_list_len)
{
	int32 ret = BKTE_OK, ch_idx, in_idx, k, rc, start, endpos, nrc, out_idx;
	wl_uint32_list_t *list = NULL;
	wl_uint32_list_t *tmplist = NULL;
	char *buf = NULL;
	chanspec_t input = 0x0;
	uint8 bitmap[((sizeof(rc_map)/sizeof(rc_map[0])) + 7)/NBBY] = {0};
	uint8 *tmp_ptr_ch_count;
	uint32 bw_cap;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_radio_caps, BKTE_INV_ARG);

	buf = (char *)bkt_malloc(WLC_IOCTL_MAXLEN, &ret);
	BKT_ASSERT();

	ret = wl_iovar_getbuf(ifname, "chanspecs", &input, sizeof(input), buf, WLC_IOCTL_MAXLEN);
	BKT_IOVAR_ASSERT(ifname, "chanspecs", ret);

	tmplist = (wl_uint32_list_t*)buf;

	/* Now get the bandwidth cap and based on bandwidth cap choose the chanspecs */
	blanket_get_chanspec(ifname, &input);
	bw_cap = blanket_get_bw_cap(ifname, CHSPEC2WLC_BAND(input));

	list = (wl_uint32_list_t *)bkt_malloc((sizeof(wl_uint32_list_t) * tmplist->count), &ret);
	BKT_ASSERT();

	for (ch_idx = 0, in_idx = 0; ch_idx < tmplist->count; ch_idx++) {
		if ((CHSPEC_IS20(tmplist->element[ch_idx]) && WL_BW_CAP_20MHZ(bw_cap)) ||
			(CHSPEC_IS40(tmplist->element[ch_idx]) && WL_BW_CAP_40MHZ(bw_cap)) ||
			(CHSPEC_IS80(tmplist->element[ch_idx]) && WL_BW_CAP_80MHZ(bw_cap)) ||
			(CHSPEC_IS160(tmplist->element[ch_idx]) && WL_BW_CAP_160MHZ(bw_cap))) {
			list->element[in_idx++] = tmplist->element[ch_idx];
		}
	}
	list->count = in_idx;

	in_idx = 0;
	out_idx = 1;	/* 0th will contain the count of rclass. */
	do {
		uint8 valid = 0, ch_count = 0;
		int txpwr = 0;
		rc = rc_map[in_idx++];
		nrc = rc_map[in_idx++];
		start = in_idx;
		endpos = start + nrc;
		/* first pass see if all channels of this class are valid */
		for (; in_idx < endpos && in_idx < rc_max; in_idx++) {

			for (ch_idx = 0; ch_idx < (int32) list->count; ch_idx++) {
				if (rc_map[in_idx] == list->element[ch_idx]) {
					valid++;
					setbit(bitmap, in_idx);
					break;
				}
			}
		}

		if (valid == 0) { /* none in this class are valid */
			continue;
		}

		if (out_idx + 3 >= max_list_len) {
			break;
		}

		out_radio_caps->list[0]++;
		out_radio_caps->list[out_idx++] = rc;
		(void)blanket_get_txpwr_target_max(ifname, &txpwr);
		out_radio_caps->list[out_idx++] = (int8)txpwr;
		/* Fill the channel count in the end */
		tmp_ptr_ch_count = &out_radio_caps->list[out_idx++];

		if (valid == nrc) { /* all in this class are valid */
			continue;
		}

		if (out_idx + (nrc - valid) >= max_list_len) {
			break;
		}

		/* second pass since not all channels of this class are valid */
		for (k = start; k < endpos && k < rc_max; k++) {
			if (!isset(bitmap, k)) {
				/* Only for 80MHz/160MHz store center channel */
				if (CHSPEC_IS80(rc_map[k]) || CHSPEC_IS160(rc_map[k])) {
					/* prepare centre channel list for 80/80+80/160 */
					out_radio_caps->list[out_idx++] =
						CHSPEC_CHANNEL(rc_map[k]);
					ch_count++;
				} else {
					out_radio_caps->list[out_idx++] =
						wf_chspec_ctlchan(rc_map[k]);
					ch_count++;
				}
			}
		}
		/* Fill the channel count */
		*tmp_ptr_ch_count = ch_count;

	} while (in_idx < rc_max && out_idx < max_list_len);

	out_radio_caps->len = out_idx;
	out_radio_caps->valid = TRUE;
end:
	if (buf) {
		free(buf);
		buf = NULL;
	}
	if (list) {
		free(list);
		list = NULL;
	}
	BKT_EXIT();
	return ret;
}

/* To get the phytype of slave */
int
blanket_get_phy_type(char *ifname, wl_bss_info_t *in_bi, uint8 *out_phy_type)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;

	/* Validate arg */
	BKT_ASSERT_ARG(out_phy_type, BKTE_INV_ARG);

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, &bi);
		BKT_ASSERT();
	} else {
		bi = in_bi;
	}

	if (bi->vht_cap) {
		*out_phy_type = WBD_DOT11_PHYTYPE_VHT;
	} else if (bi->n_cap) {
		*out_phy_type = WBD_DOT11_PHYTYPE_HT;
	} else {
		*out_phy_type = 0;
	}

	BKT_DEBUG("out_phy_type = %x\n", *out_phy_type);

end:
	return ret;
}

/* To get the bssid_info of slave */
int
blanket_get_bssid_info_field(char *ifname, wl_bss_info_t *in_bi, uint32 *out_bssid_info)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;

	/* Validate arg */
	BKT_ASSERT_ARG(out_bssid_info, BKTE_INV_ARG);

	wl_endian_probe(ifname);

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, &bi);
		BKT_ASSERT();
	} else {
		bi = in_bi;
	}

	bi->capability = dtoh16(bi->capability);

	*out_bssid_info =  DOT11_NGBR_BI_REACHABILTY;
	*out_bssid_info |= DOT11_NGBR_BI_SEC;
	*out_bssid_info |= DOT11_NGBR_BI_KEY_SCOPE;
	*out_bssid_info |= (bi->capability & DOT11_CAP_QOS) ? DOT11_NGBR_BI_CAP_QOS : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_APSD) ? DOT11_NGBR_BI_CAP_APSD : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_RRM) ? DOT11_NGBR_BI_CAP_RDIO_MSMT : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_DELAY_BA) ? DOT11_NGBR_BI_CAP_DEL_BA : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_IMMEDIATE_BA) ? DOT11_NGBR_BI_CAP_IMM_BA : 0;
	*out_bssid_info |= (bi->n_cap) ? DOT11_NGBR_BI_HT : 0;
	*out_bssid_info |= (bi->vht_cap) ? DOT11_NGBR_BI_VHT : 0;

	BKT_DEBUG("out_bssid_info = %x\n", *out_bssid_info);

end:
	return ret;
}

/* Get max assoclist the Interface */
int
blanket_get_max_assoc(char* ifname, int *maxassoc)
{
	int ret = BKTE_OK;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(maxassoc, BKTE_INV_ARG);

	wl_endian_probe(ifname);

	ret = wl_iovar_getint(ifname, "maxassoc", maxassoc);
	BKT_IOVAR_ASSERT(ifname, "max assoc", ret);

	*maxassoc = dtoh32(*maxassoc);

end:
	BKT_EXIT();
	return ret;
}

/* Get max NSS value of the Interface */
int
blanket_get_max_nss(char* ifname, wl_bss_info_t *in_bi, int *max_nss)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(max_nss, BKTE_INV_ARG);

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, &bi);
		BKT_ASSERT();
	} else {
		bi = in_bi;
	}

	/* Get max_nss */
	*max_nss = wl_wlif_get_max_nss(bi);

end:
	BKT_EXIT();
	return ret;
}

/* Get Assoc list of Interface - caller should free the out pointer */
int
blanket_get_assoclist(char* ifname, struct maclist *assoclist, uint32 listlen)
{
	int ret = BKTE_OK;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(assoclist, BKTE_INV_ARG);

	wl_endian_probe(ifname);

	assoclist->count = htod32((listlen - sizeof(uint32))/ETHER_ADDR_LEN);

	ret = wl_ioctl(ifname, WLC_GET_ASSOCLIST, assoclist, listlen);
	BKT_IOVAR_ASSERT(ifname, "get assoclist", ret);

	assoclist->count = dtoh32(assoclist->count);

	if (assoclist->count == 0) {
		/* ASSOC list is empty or No assoc list in driver */
		BKT_INFO("ifname[%s] Assoclist empty or unavailable\n", ifname);
		ret = BKTE_EMPTY_ASOCLST;
		goto end;
	}

end:
	BKT_EXIT();
	return ret;
}

/* Get power percentage */
int
blanket_get_pwr_percent(char* ifname, int *out_pwr_percent)
{
	int ret = BKTE_OK;
	int pwr_percent;
	BKT_ENTER();

	wl_endian_probe(ifname);
	/* Validate arg */
	BKT_ASSERT_ARG(out_pwr_percent, BKTE_INV_ARG);

	ret = wl_ioctl(ifname, WLC_GET_PWROUT_PERCENTAGE, &pwr_percent, sizeof(pwr_percent));
	BKT_IOVAR_ASSERT(ifname, "get power percent", ret);

	*out_pwr_percent = dtoh32(pwr_percent);

	BKT_DEBUG("ifname : %s power percent : %d\n", ifname, *out_pwr_percent);
end:
	return ret;
}

/* set power percentage at the interface */
int
blanket_set_pwr_percent(char *ifname, int pwr_percent)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	if (pwr_percent < 0 || pwr_percent > 100) {
		BKT_WARNING("ifname[%s] power percent %d out of range\n", ifname, pwr_percent);
		return BKTE_INV_ARG;
	}
	ret = wl_ioctl(ifname, WLC_SET_PWROUT_PERCENTAGE, &pwr_percent, sizeof(pwr_percent));
	BKT_IOVAR_ASSERT(ifname, "set pwr_percent", ret);

end:
	return ret;
}

/* Enables STA monitoring feature */
int
blanket_stamon_enable(char *ifname)
{
	int ret = BKTE_OK;
	wlc_stamon_sta_config_t stamon_cfg;
	BKT_ENTER();

	wl_endian_probe(ifname);

	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = htod16(STAMON_CFG_CMD_ENB);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);

	/* Enable STA monitor module in Firmware */
	ret = wl_iovar_set(ifname, "sta_monitor", &stamon_cfg, sizeof(stamon_cfg));
	BKT_IOVAR_ASSERT(ifname, "sta_monitor Enable", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Disbles STA monitoring feature */
int
blanket_stamon_disable(char *ifname)
{
	int ret = BKTE_OK;
	wlc_stamon_sta_config_t stamon_cfg;
	BKT_ENTER();

	wl_endian_probe(ifname);

	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = htod16(STAMON_CFG_CMD_DSB);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);

	/* Disable STA monitor module in Firmware */
	ret = wl_iovar_set(ifname, "sta_monitor", &stamon_cfg, sizeof(stamon_cfg));
	BKT_IOVAR_ASSERT(ifname, "sta_monitor Disable", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Add a STA's mac address to monitored STA's list */
int
blanket_stamon_add_mac(char *ifname, struct ether_addr *sta_mac,
	chanspec_t offchan_chspec, uint32 offchan_time)
{
	int ret = BKTE_OK;
	wlc_stamon_sta_config_t stamon_cfg;
	BKT_ENTER();

	wl_endian_probe(ifname);

	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = htod16(STAMON_CFG_CMD_ADD);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);
	stamon_cfg.chanspec = htod32(offchan_chspec);
	stamon_cfg.offchan_time = htod32(offchan_time);
	memcpy(&stamon_cfg.ea, sta_mac, sizeof(stamon_cfg.ea));

	ret = wl_iovar_set(ifname, "sta_monitor", &stamon_cfg, sizeof(stamon_cfg));
	BKT_IOVAR_ASSERT(ifname, "sta_monitor Add", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Delete a STA's mac address from monitored STA's list */
int
blanket_stamon_delete_mac(char *ifname, struct ether_addr *sta_mac)
{
	int ret = BKTE_OK;
	wlc_stamon_sta_config_t stamon_cfg;
	BKT_ENTER();

	wl_endian_probe(ifname);

	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = htod16(STAMON_CFG_CMD_DEL);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);
	memcpy(&stamon_cfg.ea, sta_mac, sizeof(stamon_cfg.ea));

	ret = wl_iovar_set(ifname, "sta_monitor", &stamon_cfg, sizeof(stamon_cfg));
	BKT_IOVAR_ASSERT(ifname, "sta_monitor Delete", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get STAs' monitoed statastics from driver */
int
blanket_stamon_get_stats(char *ifname, stamon_info_t *sta_list, int listlen)
{
	int ret = BKTE_OK;
	wlc_stamon_sta_config_t stamon_cfg;
	BKT_ENTER();

	wl_endian_probe(ifname);

	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = htod16(STAMON_CFG_CMD_GET_STATS);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);

	memset(sta_list, 0, listlen);
	memcpy(&stamon_cfg.ea, &sta_list->sta_data[0].ea, sizeof(stamon_cfg.ea));

	ret = wl_iovar_getbuf(ifname, "sta_monitor", &stamon_cfg, sizeof(stamon_cfg),
			sta_list, listlen);
	BKT_IOVAR_ASSERT(ifname, "sta_monitor Get_Stats", ret);

end:
	BKT_EXIT();
	return ret;
}

#define MCSTCNT_WLC_FROM_CNTBUF(cntbuf) (wl_cnt_wlc_t *) \
	bcm_get_data_from_xtlv_buf(((wl_cnt_info_t *)cntbuf)->data,     \
		((wl_cnt_info_t *)cntbuf)->datalen,                     \
		WL_CNT_XTLV_WLC, NULL,                      \
		BCM_XTLV_OPTION_ALIGN32)

/* Gets the counters from driver */
int
blanket_get_counters(char *ifname, blanket_counters_t *counters)
{
	int ret = BKTE_OK;
	uint16 ver;
	uint8 *cntbuf;
	wl_cnt_info_t *cntinfo;
	uint32 corerev = 0;
	wl_cnt_wlc_t *macstat_wlc;

	BKT_ENTER();

	BCM_REFERENCE(ver);

	wl_endian_probe(ifname);

	cntbuf = (uint8 *)bkt_malloc(WLC_IOCTL_MEDLEN, &ret);
	BKT_ASSERT();

	ret = wl_iovar_get(ifname, "counters", cntbuf, WLC_IOCTL_MEDLEN);
	BKT_IOVAR_ASSERT(ifname, "counters", ret);

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	ver = cntinfo->version;
	if (cntinfo->datalen + OFFSETOF(wl_cnt_info_t, data) > WLC_IOCTL_MEDLEN) {
	    BKT_ERROR("ifname[%s]: IOVAR buffer short!\n", ifname);
	    goto end;
	}

	/* Translate traditional (ver <= 10) counters struct to new xtlv type struct */
	ret = wl_cntbuf_to_xtlv_format(NULL, cntbuf, WLC_IOCTL_MEDLEN, corerev);
	BKT_IOVAR_ASSERT(ifname, "WLC_IOCTL_MEDLEN", ret);

	if ((macstat_wlc = MCSTCNT_WLC_FROM_CNTBUF(cntbuf)) != NULL) {
		counters->txframe = dtoh32(macstat_wlc->txframe);
		counters->txbyte = dtoh32(macstat_wlc->txbyte);
		counters->txerror = dtoh32(macstat_wlc->txerror);
		counters->rxframe = dtoh32(macstat_wlc->rxframe);
		counters->rxbyte = dtoh32(macstat_wlc->rxbyte);
		counters->rxerror = dtoh32(macstat_wlc->rxerror);
	}

end:
	BKT_EXIT();
	free(cntbuf);
	return ret;
}

/* Check if Interface is Virtual or not */
bool
blanket_is_interface_virtual(char *ifname)
{
	int unit = -1, subunit = -1;

	if (get_ifname_unit(ifname, &unit, &subunit) < 0) {
		return FALSE;
	}
	if (unit < 0) {
		return FALSE;
	}
	if (subunit >= 0) {
		return TRUE;
	}

	return FALSE;
}

/* Check if Interface and its Primary Radio is enabled or not */
int
blanket_is_interface_enabled(char *ifname, bool validate_vif, int *error)
{
	int ret = BKTE_OK, unit = 0, ifr_enabled = 0;
	char buf[BKT_MAX_BUF_256];
	BKT_ENTER();

	/* Check interface (fail for non-wl interfaces) */
	if ((ret = wl_probe(ifname))) {
		ret = BKTE_INV_IFNAME;
		goto end;
	}

	wl_endian_probe(ifname);

	/* Get instance */
	ret = wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
	unit = dtoh32(unit);
	BKT_IOVAR_ASSERT(ifname, "get instance", ret);

	/* Check if Primary Radio of given Interface is enabled or not */
	snprintf(buf, sizeof(buf), "wl%d_radio", unit);
	ifr_enabled = blanket_get_config_val_int(NULL, buf, 0);

	if (!ifr_enabled) {
		goto end;
	}

	/* Check if interface is vifs */
	if ((validate_vif) && (blanket_is_interface_virtual(ifname))) {
		/* Check if vifs is enabled or not */
		snprintf(buf, sizeof(buf), "%s_bss_enabled", ifname);
		ifr_enabled = blanket_get_config_val_int(NULL, buf, 0);
	}
end:
	if (error) {
		*error = ret;
	}
	BKT_EXIT();
	return ifr_enabled;
}

/* Get wlX_ or wlX.y_ Prefix from OS specific interface name */
int
blanket_get_interface_prefix(char *ifname, char *prefix, int prefix_len)
{
	int ret = BKTE_OK;
	char wl_name[IFNAMSIZ];
	BKT_ENTER();

	/* Convert eth name to wl name - returns 0 if success */
	ret = osifname_to_nvifname(ifname, wl_name, sizeof(wl_name));
	BKT_ASSERT_ERR(BKTE_INV_IFNAME);

	/* Get prefix of the interface from Driver */
	make_wl_prefix(prefix, prefix_len, 1, wl_name);
	BKT_DEBUG("Interface: %s, wl_name: %s, Prefix: %s\n", ifname, wl_name, prefix);

end:
	BKT_EXIT();
	return ret;
}

/* Get instance of interface i.e. 0/1/2 */
int
blanket_get_radio_prefix(char* ifname, char *radio_prefix, int prefix_len)
{
	int unit = -1;
	int ret = BKTE_OK;
	char nvram_name[BKT_MAX_BUF_16] = {0};
	BKT_ENTER();

	wl_endian_probe(ifname);

	/* get the nvram name of the interface */
	if ((osifname_to_nvifname(ifname, nvram_name, sizeof(nvram_name))) != 0) {
		goto end;
	}

	/* Get instance */
	ret = wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
	unit = dtoh32(unit);
	BKT_IOVAR_ASSERT(ifname, "get instance", ret);
	snprintf(radio_prefix, prefix_len, "wl%d_", unit);

end:
	BKT_EXIT();
	return ret;
}

/* Blanket API to get the NVRAM value. */
char*
blanket_nvram_safe_get(const char *nvram)
{
	return nvram_safe_get(nvram);
}

/* Blanket API to get the NVRAM value for specific BSS prefix */
char*
blanket_nvram_prefix_safe_get(const char *prefix, const char *nvram)
{
	char data[BKT_MAX_BUF_256];
	memset(data, 0, sizeof(data));

	if (prefix) {
		return nvram_safe_get(strcat_r(prefix, nvram, data));
	}
	return nvram_safe_get(nvram);
}

/* Blanket API to set the NVRAM value for specific BSS prefix */
int
blanket_nvram_prefix_set(const char *prefix, const char *nvram, const char *nvramval)
{
	char data[BKT_MAX_BUF_256];
	memset(data, 0, sizeof(data));

	if (prefix) {
		return nvram_set(strcat_r(prefix, nvram, data), nvramval);
	}
	return nvram_set(nvram, nvramval);
}

/* Blanket API to unset the NVRAM value for specific BSS prefix */
int
blanket_nvram_prefix_unset(const char *prefix, const char *nvram)
{
	char data[BKT_MAX_BUF_256];
	memset(data, 0, sizeof(data));

	if (prefix) {
		return nvram_unset(strcat_r(prefix, nvram, data));
	}
	return nvram_unset(nvram);
}

/* Match NVRAM and ARG value, and if mismatch, Set new value in NVRAM */
int
blanket_nvram_prefix_match_set(const char* prefix, char* nvram, char* new_value,
	bool matchcase)
{
	char *nvram_value = NULL;
	int mismatch = 0;

	/* Get NVRAM Value */
	nvram_value = blanket_nvram_prefix_safe_get(prefix, nvram);

	/* Compare NVRAM and New value with/without case */
	if (matchcase) {
		mismatch = strcmp(nvram_value, new_value);
	} else {
		mismatch = strcasecmp(nvram_value, new_value);
	}

	/* And if mismatch, set New value in NVRAM */
	if (mismatch) {

		BKT_INFO("Prefix[%s] NVRAM[%s] NVRAMVal[%s] != NewVal[%s]."
			" Needs rc restart\n", prefix ? prefix : "", nvram, nvram_value, new_value);

		/* Set New Value in NVRAM */
		blanket_nvram_prefix_set(prefix, nvram, new_value);

		/* Value is Overidden, Return 1 */
		return 1;
	}
	return 0;
}

/* Gets the unsigned integer config val from NVARM, if not found applies the default value */
uint16
blanket_get_config_val_uint(char* prefix, const char *nvram, uint16 def)
{
	char *val = NULL;
	uint16 ret = def;
	BKT_ENTER();

	if (prefix) {
		val = blanket_nvram_prefix_safe_get(prefix, nvram);
	} else {
		val = blanket_nvram_safe_get(nvram);
	}

	if (val && (val[0] != '\0')) {
		ret = (uint16)strtoul(val, NULL, 0);
	} else {
		BKT_INFO("NVRAM is not defined: %s%s \n", (prefix ? prefix : ""), nvram);
	}

	BKT_EXIT();
	return ret;
}

/* Gets the integer config val from NVARM, if not found applies the default value */
int
blanket_get_config_val_int(char* prefix, const char *nvram, int def)
{
	char *val = NULL;
	int ret = def;
	BKT_ENTER();

	if (prefix) {
		val = blanket_nvram_prefix_safe_get(prefix, nvram);
	} else {
		val = blanket_nvram_safe_get(nvram);
	}

	if (val && (val[0] != '\0')) {
		ret = (int)strtol(val, NULL, 0);
	} else {
		BKT_INFO("NVRAM is not defined: %s%s \n", (prefix ? prefix : ""), nvram);
	}

	BKT_EXIT();
	return ret;
}

/* Enable/Disable BSS_LOAD Information Element in Beacon & Probe_Resp */
int
blanket_enable_bssload_ie(char* ifname, int enable)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	ret = wl_iovar_setint(ifname, "bssload", htod32(enable));
	BKT_IOVAR_ASSERT(ifname, "set bssload IE", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get Chaim Stats */
int
blanket_get_chanim_stats(char* ifname, uint32 count, char* data_buf, int buflen)
{
	int ret = 0;
	wl_chanim_stats_t *list;
	wl_chanim_stats_t param;
	BKT_ENTER();

	wl_endian_probe(ifname);

	/* Validate arg */
	BKT_ASSERT_ARG(data_buf, BKTE_INV_ARG);
	list = (wl_chanim_stats_t *) data_buf;

	param.buflen = htod32(buflen);
	param.count = htod32(count);

	ret = wl_iovar_getbuf(ifname, "chanim_stats", &param, sizeof(param), data_buf, buflen);
	BKT_IOVAR_ASSERT(ifname, "get chanim_stats", ret);

	list->buflen = dtoh32(list->buflen);
	list->version = dtoh32(list->version);
	list->count = dtoh32(list->count);

	BKT_DEBUG("buflen: %d, version: %d count: %d\n",
		list->buflen, list->version, list->count);

	if (list->buflen == 0) {
		list->version = 0;
		list->count = 0;
	} else if (list->version > WL_CHANIM_STATS_VERSION) {
		BKT_ERROR("Err, your driver has wl_chanim_stats version %d "
			"but this program supports version upto %d.\n",
				list->version, WL_CHANIM_STATS_VERSION);
		list->buflen = 0;
		list->count = 0;
	}

end:
	BKT_EXIT();
	return ret;
}

/* Get count of associated sta */
int
blanket_get_assoc_sta_count(char *ifname, int *assoc_count)
{
	int ret = BKTE_OK;
	uint8 *assoclist = NULL;
	int assoclistlen = 0;
	struct maclist* list = NULL;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(assoc_count, BKTE_INV_ARG);

	/* Get Max Assoclist len */
	ret = blanket_get_max_assoc(ifname, &assoclistlen);
	BKT_ASSERT();

	assoclistlen = assoclistlen * ETHER_ADDR_LEN + sizeof(uint32);

	assoclist = (uint8 *)bkt_malloc(assoclistlen, &ret);
	BKT_ASSERT();

	list = (struct maclist *)assoclist;

	/* Get assoclist on this interface */
	ret = blanket_get_assoclist(ifname, list, assoclistlen);
	BKT_ASSERT();

	/* Get count of associated sta */
	*assoc_count = list->count;

end:
	if (assoclist) {
		free(assoclist);
	}

	BKT_EXIT();
	return ret;
}

/* Sends beacon request to assocociated STA */
int
blanket_send_beacon_request(char *ifname, blanket_bcnreq_t *bcnreq_extn,
	uint8 *sub_element_data, int sub_element_len, int *ret_token)
{
	int ret = BKTE_OK;
	wl_action_frame_t * action_frame;
	wl_af_params_t * af_params;
	char ioctl_buf[WLC_IOCTL_MAXLEN];
	char *str;
	uint namelen, ioctl_len;
	uint16 af_len, optional_len;
	dot11_rmreq_t *rmreq_ptr;
	dot11_rmreq_bcn_t *rmreq_bcn_ptr;
	static int static_tocken = 0;
	uint8 optional_data[BKT_MAX_BUF_256];
	bcnreq_t *bcnreq = &bcnreq_extn->bcnreq;
	BKT_ENTER();

	wl_endian_probe(ifname);

	af_params = (wl_af_params_t *) bkt_malloc(WL_WIFI_AF_PARAMS_SIZE, &ret);
	BKT_ASSERT_MSG("ifname[%s]. Failed to allocate %zu memory for actframe buff\n",
		ifname, WL_WIFI_AF_PARAMS_SIZE);

	/* Update Action Frame's token */
	static_tocken = (static_tocken > BKT_MAX_BUF_128) ? 0 : static_tocken;

	/* Update Action Frame's parameters */
	af_params->channel = 0;
	af_params->dwell_time = 0;
	action_frame = &af_params->action_frame;
	action_frame->packetId = (uint32)(uintptr)action_frame;

	/* Update STA MAC */
	memcpy(&action_frame->da, &(bcnreq->da), sizeof(action_frame->da));

	/* Update Source BSSID */
	memcpy(&af_params->BSSID, &(bcnreq_extn->src_bssid), sizeof(af_params->BSSID));

	/* Update Action Frame's Length */
	af_len = DOT11_RMREQ_LEN + DOT11_RMREQ_BCN_LEN;
	action_frame->len = htod16(af_len);

	/* Update Action Frame's Basic data */
	rmreq_ptr = (dot11_rmreq_t *)&action_frame->data[0];
	rmreq_ptr->category = DOT11_ACTION_CAT_RRM;
	rmreq_ptr->action = DOT11_RM_ACTION_RM_REQ;
	rmreq_ptr->token = ++static_tocken;
	BKT_DEBUG("Action Frame Token[%d] For STA["MACF"]\n", static_tocken,
		ETHER_TO_MACF(bcnreq->da));
	rmreq_ptr->reps = bcnreq->reps;

	/* Update Action Frame's data specific to Beacon Request Frame Type */
	rmreq_bcn_ptr = (dot11_rmreq_bcn_t *)&rmreq_ptr->data[0];
	rmreq_bcn_ptr->id = DOT11_MNG_MEASURE_REQUEST_ID;

	/* Update Beacon Request Element's Length */
	rmreq_bcn_ptr->len = DOT11_RMREQ_BCN_LEN - TLV_HDR_LEN;

	rmreq_bcn_ptr->token = ++static_tocken;
	if (ret_token) {
		*ret_token = static_tocken;
	}
	BKT_DEBUG("Beacon Frame Token[%d] For STA["MACF"]\n", static_tocken,
		ETHER_TO_MACF(bcnreq->da));
	rmreq_bcn_ptr->mode = 0;
	rmreq_bcn_ptr->type = DOT11_MEASURE_TYPE_BEACON;
	rmreq_bcn_ptr->reg = bcnreq_extn->opclass;
	rmreq_bcn_ptr->channel = bcnreq->channel;
	rmreq_bcn_ptr->interval = htod16(bcnreq->random_int);
	rmreq_bcn_ptr->duration = htod16(bcnreq->dur);
	rmreq_bcn_ptr->bcn_mode = bcnreq->bcn_mode;

	/* Update Target BSSID */
	if (ETHER_ISNULLADDR(&(bcnreq_extn->target_bssid))) {
		memset(&rmreq_bcn_ptr->bssid.octet, 0xFF, ETHER_ADDR_LEN);
	} else {
		memcpy(&rmreq_bcn_ptr->bssid, &(bcnreq_extn->target_bssid), ETHER_ADDR_LEN);
	}

	/* Get Subelement data from fn argument in uint8 form directly */
	if (sub_element_data) {

		assert(sizeof(optional_data) > sub_element_len);

		memcpy(&optional_data[0], sub_element_data, sub_element_len);
		optional_len = sub_element_len;

	/* Get Subelement data from NVRAM and convert it from string to uint8 */
	} else {
		str = blanket_nvram_safe_get(NVRAM_BCN_REQ_SUB_ELE);
		if (str[0] == '\0') {
			str = DEF_BCN_REQ_SUB_ELE;
		}
		optional_len = strlen(str) / 2;
		get_hex_data((uchar *)str, &optional_data[0], optional_len);
	}

	/* Append optional subelement TLV at the end of Beacon Request data */
	memcpy(&action_frame->data[af_len], &optional_data[0], optional_len);

	/* Update Action Frame's Length - AGAIN */
	af_len += optional_len;
	action_frame->len = htod16(af_len);
	/* Update Beacon Request Element's Length - AGAIN */
	rmreq_bcn_ptr->len += optional_len;

	/* Append SSID optional subelement data */
	if (bcnreq->ssid.SSID_len) {

		/* Append SSID TLV to the frame */
		action_frame->data[af_len] = 0;
		action_frame->data[af_len + 1] =  bcnreq->ssid.SSID_len;
		memcpy(&action_frame->data[af_len + TLV_HDR_LEN],
			bcnreq->ssid.SSID, bcnreq->ssid.SSID_len);

		/* Update Action Frame's Length - AGAIN */
		af_len += bcnreq->ssid.SSID_len + TLV_HDR_LEN;
		action_frame->len = htod16(af_len);
		/* Update Beacon Request Element's Length - AGAIN */
		rmreq_bcn_ptr->len += bcnreq->ssid.SSID_len + TLV_HDR_LEN;
	}

	/* Get length of IOVAR Name + Null */
	namelen = strlen("actframe") + 1;

	/* Copy IOVAR Name including Null to IOVAR Buffer */
	memcpy(ioctl_buf, "actframe", namelen);

	/* Copy Action Frame Data to IOVAR Buffer */
	memcpy((int8*)ioctl_buf + namelen, af_params, WL_WIFI_AF_PARAMS_SIZE);

	/* Get length of IOVAR Buffer in total */
	ioctl_len = namelen + WL_WIFI_AF_PARAMS_SIZE;

	/* Send Action Frame */
	ret = wl_ioctl(ifname, WLC_SET_VAR, ioctl_buf, ioctl_len);
	BKT_IOVAR_ASSERT(ifname, "actframe", ret);

end:
	if (af_params) {
		free(af_params);
	}

	BKT_EXIT();
	return ret;
}

/* Get amsdu enabled/disabled */
int
blanket_is_amsdu_enabled(char *ifname, int *amsdu)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(amsdu, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "amsdu", amsdu, sizeof(*amsdu));
	BKT_IOVAR_ASSERT(ifname, "amsdu", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get AMPDU enabled/disabled */
int
blanket_is_ampdu_enabled(char *ifname, int *ampdu)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(ampdu, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "ampdu", ampdu, sizeof(*ampdu));
	BKT_IOVAR_ASSERT(ifname, "ampdu", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get AMPDU block Ack window size */
int
blanket_get_ampdu_ba_wsize(char *ifname, int *ba_wsize)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(ba_wsize, BKTE_INV_ARG);

	wl_endian_probe(ifname);

	ret = wl_iovar_get(ifname, "ampdu_ba_wsize", ba_wsize, sizeof(*ba_wsize));
	BKT_IOVAR_ASSERT(ifname, "ampdu_ba_wsize", ret);

	*ba_wsize = dtoh32(*ba_wsize);

end:

	BKT_EXIT();
	return ret;
}

struct d11_mcs_rate_info {
	uint8 constellation_bits;
	uint8 coding_q;
	uint8 coding_d;
};

static const struct d11_mcs_rate_info wlu_mcs_info[] = {
	{ 1, 1, 2 }, /* MCS  0: MOD: BPSK,   CR 1/2 */
	{ 2, 1, 2 }, /* MCS  1: MOD: QPSK,   CR 1/2 */
	{ 2, 3, 4 }, /* MCS  2: MOD: QPSK,   CR 3/4 */
	{ 4, 1, 2 }, /* MCS  3: MOD: 16QAM,  CR 1/2 */
	{ 4, 3, 4 }, /* MCS  4: MOD: 16QAM,  CR 3/4 */
	{ 6, 2, 3 }, /* MCS  5: MOD: 64QAM,  CR 2/3 */
	{ 6, 3, 4 }, /* MCS  6: MOD: 64QAM,  CR 3/4 */
	{ 6, 5, 6 }, /* MCS  7: MOD: 64QAM,  CR 5/6 */
	{ 8, 3, 4 }, /* MCS  8: MOD: 256QAM, CR 3/4 */
	{ 8, 5, 6 }  /* MCS  9: MOD: 256QAM, CR 5/6 */
};

static uint
wl_mcs2rate(uint mcs, uint nss, uint bw, int sgi)
{
	const int ksps = 250; /* kilo symbols per sec, 4 us sym */
	const int Nsd_20MHz = 52;
	const int Nsd_40MHz = 108;
	const int Nsd_80MHz = 234;
	const int Nsd_160MHz = 468;
	uint rate;

	if (mcs == 32) {
		/* just return fixed values for mcs32 instead of trying to parametrize */
		rate = (sgi == 0) ? 6000 : 6700;
	} else if (mcs <= ARRAY_SIZE(wlu_mcs_info)) {
		/* This calculation works for 11n HT and 11ac VHT if the HT mcs values
		 * are decomposed into a base MCS = MCS % 8, and Nss = 1 + MCS / 8.
		 * That is, HT MCS 23 is a base MCS = 7, Nss = 3
		 */

		/* find the number of complex numbers per symbol */
		if (bw == 20) {
			rate = Nsd_20MHz;
		} else if (bw == 40) {
			rate = Nsd_40MHz;
		} else if (bw == 80) {
			rate = Nsd_80MHz;
		} else if (bw == 160) {
			rate = Nsd_160MHz;
		} else {
			rate = 1;
		}

		/* multiply by bits per number from the constellation in use */
		rate = rate * wlu_mcs_info[mcs].constellation_bits;

		/* adjust for the number of spatial streams */
		rate = rate * nss;

		/* adjust for the coding rate given as a quotient and divisor */
		rate = (rate * wlu_mcs_info[mcs].coding_q) / wlu_mcs_info[mcs].coding_d;

		/* multiply by Kilo symbols per sec to get Kbps */
		rate = rate * ksps;

		/* adjust the symbols per sec for SGI
		 * symbol duration is 4 us without SGI, and 3.6 us with SGI,
		 * so ratio is 10 / 9
		 */
		if (sgi) {
			/* add 4 for rounding of division by 9 */
			rate = ((rate * 10) + 4) / 9;
		}
	} else {
		rate = 0;
	}

	return rate;
}

#define VHT_CAP_MCS_MAP_0_7_MAX_IDX 8
#define VHT_CAP_MCS_MAP_0_8_MAX_IDX 9
#define VHT_CAP_MCS_MAP_0_9_MAX_IDX 10
#define VHT_CAP_MCS_MAP_0_11_MAX_IDX 12

/* From wlc_rate.[ch] */
#define MCS_TABLE_SIZE	33

/* get the Max rate of an interface */
int
blanket_get_max_rate(char *ifname, wl_bss_info_t *in_bi, int *outmaxrate)
{
	int ret = BKTE_OK, rate = 0, maxrate = 0, tmprate = 0;
	int i = 0, mcs_idx = 0;
	int nbw = 0;
	int mcs = 0, sgi = 0, isht = 0;
	wl_bss_info_t *bi;

	/* Validate arg */
	BKT_ASSERT_ARG(outmaxrate, BKTE_INV_ARG);

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, &bi);
		BKT_ASSERT();
	} else {
		bi = in_bi;
	}

	if (CHSPEC_IS80(bi->chanspec))
		nbw = 80;
	else if (CHSPEC_IS40(bi->chanspec))
		nbw = 40;
	else if (CHSPEC_IS20(bi->chanspec))
		nbw = 20;

	if (wl_iovar_get(ifname, "sgi_tx", &sgi, sizeof(sgi)) < 0) {
		sgi = 0;
	}
	if (sgi != 0) {
		sgi = 1;
	}

	/* Copy the supported rates */
	for (i = 0; i < bi->rateset.count; i++) {
		if (bi->rateset.rates[i]) {
			tmprate = ((bi->rateset.rates[i])&0x7F)/2;
			if (tmprate > maxrate)
				maxrate = tmprate;
		}
	}

	if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
		if (bi->vht_cap) {
			int nss = 0, rate = 0;
			uint mcs_cap = 0, mcs_cap_map = 0;

			for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
				mcs_cap = VHT_MCS_MAP_GET_MCS_PER_SS(i,
					ltoh16(bi->vht_txmcsmap));
				if (mcs_cap != VHT_CAP_MCS_MAP_NONE) {
					nss++; /* Calculate the number of streams */
					mcs_cap_map = mcs_cap; /* Max valid mcs cap map */
				}
			}

			if (nss) {
				if (mcs_cap_map == VHT_CAP_MCS_MAP_0_7)
					mcs = VHT_CAP_MCS_MAP_0_7_MAX_IDX - 1;
				else if (mcs_cap_map == VHT_CAP_MCS_MAP_0_8)
					mcs = VHT_CAP_MCS_MAP_0_8_MAX_IDX - 1;
				else if (mcs_cap_map == VHT_CAP_MCS_MAP_0_9)
					mcs = VHT_CAP_MCS_MAP_0_9_MAX_IDX - 1;

				rate = wl_mcs2rate(mcs, nss, nbw, sgi);
				tmprate = rate/1000;
				if (tmprate > maxrate)
					maxrate = tmprate;
			}
		}

		/* For 802.11n networks, use MCS table */
		for (mcs_idx = 0; mcs_idx < (MCSSET_LEN * 8); mcs_idx++) {
			if (isset(bi->basic_mcs, mcs_idx) && mcs_idx < MCS_TABLE_SIZE) {
				mcs = mcs_idx;
				tmprate = rate/1000;
				if (tmprate > maxrate)
					maxrate = tmprate;
				isht = 1;
			}
		}

		if (isht) {
			int nss = 0;

			if (mcs > 32) {
				printf("MCS is Out of range \n");
			} else if (mcs == 32) {
				rate = wl_mcs2rate(mcs, 1, nbw, sgi);
			} else {
				nss = 1 + (mcs / 8);
				mcs = mcs % 8;
				rate = wl_mcs2rate(mcs, nss, nbw, sgi);
			}
			tmprate = rate/1000;
			if (tmprate > maxrate)
				maxrate = tmprate;
		}
	}
	*outmaxrate = maxrate;

end:
	return 0;
}

/* Get the STA counters from bs_data */
int
blanket_get_bs_data_counters(char *ifname, struct ether_addr *sta_mac,
	iov_bs_data_counters_t *out_ctr)
{
	int ret = BKTE_EMPTY_ASOCLST, argn;
	char* ioctl_buf = NULL;
	uint32 flag_bits = 0;
	iov_bs_data_struct_t *data;
	iov_bs_data_record_t *rec;
	iov_bs_data_counters_t *ctr;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(ifname, BKTE_INV_ARG);
	BKT_ASSERT_ARG(sta_mac, BKTE_INV_ARG);
	BKT_ASSERT_ARG(out_ctr, BKTE_INV_ARG);

	/* Set the flag to not to reset the counter after reading */
	flag_bits |= SCB_BS_DATA_FLAG_NO_RESET;
	flag_bits = htod32(flag_bits);

	ioctl_buf = (char*)bkt_malloc(BKT_MAX_BUF_4096, &ret);
	BKT_ASSERT();

	BKT_STRNCPY(ioctl_buf, "bs_data", (strlen("bs_data")+1));
	data = (iov_bs_data_struct_t*)ioctl_buf;

	ret = wl_iovar_getbuf(ifname, "bs_data", &flag_bits, sizeof(flag_bits),
		ioctl_buf, BKT_MAX_BUF_4096);
	if (ret < 0) {
		BKT_ERROR("Err to read bs_data: %s, err is %d\n", ifname, ret);
		goto end;
	}

	BKT_DEBUG("ifname=%s data->structure_count=%d\n", ifname, data->structure_count);

	for (argn = 0; argn < data->structure_count; ++argn) {
		rec = &data->structure_record[argn];
		ctr = &rec->station_counters;

#define DEVICE_TO_HOST(xyzzy) ctr->xyzzy = dtoh32(ctr->xyzzy)
		DEVICE_TO_HOST(retry_drop);
		DEVICE_TO_HOST(rtsfail);
		DEVICE_TO_HOST(retry);
		DEVICE_TO_HOST(txrate_main);
		DEVICE_TO_HOST(txrate_succ);
		DEVICE_TO_HOST(acked);
		DEVICE_TO_HOST(throughput);
		DEVICE_TO_HOST(time_delta);
		DEVICE_TO_HOST(airtime);
#undef DEVICE_TO_HOST

		if (ctr->acked == 0) {
			continue;
		}

		if (!eacmp(rec->station_address.octet, sta_mac->octet)) {
			memcpy(out_ctr, ctr, sizeof(*out_ctr));
			ret = BKTE_OK;
			goto end;
		}
	}

end:
	if (ioctl_buf) {
		free(ioctl_buf);
	}

	BKT_EXIT();
	return ret;
}

/* Get the bandwidth cap */
uint32
blanket_get_bw_cap(char *ifname, uint32 in_band)
{
	int ret = BKTE_OK;
	uint32 bw_cap = 0;
	struct {
		uint32 band;
		uint32 bw_cap;
	} param = { 0, 0 };
	char buf[BKT_MAX_BUF_16] = {0};

	param.band = in_band;

	ret = wl_iovar_getbuf(ifname, "bw_cap", &param, sizeof(param), buf, sizeof(buf));
	BKT_IOVAR_ASSERT(ifname, "bw_cap", ret);
	bw_cap = *buf;

end:
	return bw_cap;
}

/* Check if the BSS is up or down */
int blanket_is_bss_enabled(char *ifname, int bsscfg_idx)
{
	int ret = BKTE_OK, isup = 0;
	char buf[BKT_MAX_BUF_16] = {0};

	bsscfg_idx = htod32(bsscfg_idx);
	ret = wl_iovar_getbuf(ifname, "bss", &bsscfg_idx, sizeof(bsscfg_idx), buf, sizeof(buf));
	BKT_IOVAR_ASSERT(ifname, "bss", ret);

	isup = *(int*)buf;
	isup = dtoh32(isup);
	BKT_DEBUG("ifname %s bsscfg_idx %d bss %s\n", ifname, bsscfg_idx, isup ? "up" : "down");

end:
	return isup;
}

/* Set dfs pref chan list */
int
blanket_set_dfs_forced_chspec(char* ifname, wl_dfs_forced_t* dfs_frcd, int ioctl_size)
{
	int ret = 0, idx = 0;
	BKT_ENTER();

	BKT_ASSERT_ARG(ifname, BKTE_INV_ARG);
	BKT_ASSERT_ARG(dfs_frcd, BKTE_INV_ARG);

	wl_endian_probe(ifname);

	for (idx = 0; idx < dfs_frcd->chspec_list.num; idx++) {
		dfs_frcd->chspec_list.list[idx] = htod16(dfs_frcd->chspec_list.list[idx]);
	}
	dfs_frcd->chspec_list.num = htod32(dfs_frcd->chspec_list.num);
	dfs_frcd->chspec = htod16(dfs_frcd->chspec);
	dfs_frcd->version = htod16(dfs_frcd->version);
	ret = wl_iovar_set(ifname, "dfs_channel_forced", dfs_frcd, ioctl_size);
	BKT_IOVAR_ASSERT(ifname, "set dfs_channel_forced", ret);
end:
	BKT_EXIT();
	return ret;
}

/* Keep virtual APs up, even if the primary sta is not associated */
int
blanket_set_keep_ap_up(char* ifname, int up)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	ret = wl_iovar_setint(ifname, "keep_ap_up", htod32(up));
	BKT_IOVAR_ASSERT(ifname, "set keep_ap_up", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Enable/diable raoming of STA interface */
int
blanket_set_roam_off(char* ifname, int enable)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	ret = wl_iovar_setint(ifname, "roam_off", htod32(enable));
	BKT_IOVAR_ASSERT(ifname, "set roam_off", ret);

end:
	BKT_EXIT();
	return ret;
}
