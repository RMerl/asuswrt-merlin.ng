/** @file blanket.c
 *  @brief Wi-Fi Blanket API 2.
 *
 * Broadcom Blanket library definitions
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * $Id: blanket.c 837830 2024-03-15 10:58:04Z $
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
#include <stdarg.h>

#include <shutils.h>
#include <bcmnvram.h>
#include <bcmendian.h>
#include <time.h>
#include <bcmutils.h>
#include <bcmparams.h>

#include <wlioctl.h>
#include <wlutils.h>
#include <wlif_utils.h>
#include <wlioctl_utils.h>
#include <wlioctl_defs.h>
#include <common_utils.h>
#include <bcmiov.h>
#include <bcmwifi_rspec.h>
#include <bcmwifi_rclass.h>

#include "blanket.h"
#include "ieee1905_datamodel_priv.h"
#include <fils.h>
/* Wi-Fi Blanket Error codes */
#define BKTE_OK			0
#define BKTE_MALLOC_FL		-2	/* Memory allocation failure */
#define BKTE_INV_ARG		-3	/* Invalid arguments */
#define BKTE_EMPTY_ASOCLST	-8	/* Assoclist empty or unavailable */
#define BKTE_INV_MAC		-9	/* Invalid MAC Address */
#define BKTE_WL_ERROR		-11	/* WL IOVAR error */
#define BKTE_INV_IFNAME		-81	/* Invalid interface name */

/* Wi-Fi Blanket Buffer Sizes */
#define BKT_MAX_BUF_16		16
#define BKT_MAX_BUF_32		32
#define BKT_MAX_BUF_64		64
#define BKT_MAX_BUF_128		128
#define BKT_MAX_BUF_256		256
#define BKT_MAX_BUF_512		512
#define BKT_MAX_BUF_4096	4096

/* Macros for NSS value */
#define BKT_NSS_2		2
#define BKT_NSS_3		3
#define BKT_NSS_4		4
#define BKT_NSS_8		8

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Wi-Fi Blanket NVRAMs */
#define NVRAM_BCN_REQ_SUB_ELE		"bcn_req_sub_ele"
#define NVRAM_MAP_CERT			"map_cert"
#define NVRAM_MAP_MAX_BSS		"map_max_bss"
#define NVRAM_MAP_AP_CAPS		"map_ap_caps"
#define NVRAM_MAP_SKIP_UNII4_CHANNELS	"map_skip_unii4_channels"

/* NVRAMs for Channel Scan Feature Configuration */
#define NVRAM_MAP_CHSCAN_SCANTYPE_FLAGS	"map_chscan_scantype_flags"
#define NVRAM_MAP_CHSCAN_IMPACT		"map_chscan_impact"
#define NVRAM_MAP_CHSCAN_MIN_SCAN_INT	"map_chscan_min_scan_int"
#define NVRAM_MAP_CHSCAN_PARAM_CONFIG	"map_chscan_param_config"

/* NVRAM for getting log_default_nvrams */
#define BKT_NVRAM_FILE_WRITE		"wbd_log_default_nvrams"

/* BKT INFO FLAGS Settings */
#define BKT_INFO_FLAGS_NVRAM_FILE_WRITE_ENABLED		0x0001

#define DEF_NVRAM_FILE_WRITE		0	/* default value of log_default_nvrams */

#ifndef WL_STA_GBL_RCLASS
#define WL_STA_GBL_RCLASS		0x00400000
#endif /* WL_STA_GBL_RCLASS */

#define DEF_MAX_BSS_SUPPORTED		(WL_MAXBSSCFG - 1)	/* Max BSS Supported per radio */

#define BKT_TXPWR_BUF_LEN		(2 * 1024)

/* Default AP Capability.
 * 6 Support unassociated STA link matrix on channels its BSS are not operating - Not Supported
 * 7 Support unassociated STA link matrix on channels its BSS are operating - Supported
 */
#define DEF_MAP_AP_CAPS			IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT

/* Channel Scan : Scantypes Supported Flags Options */
#define MAP_CHSCAN_SCANTYPE_DO_ONBOOT	0x0001	/* Agent performs OnBoot scans */
#define MAP_CHSCAN_SCANTYPE_DO_REQ	0x0002	/* Agent performs Requested scans */

/* Check whether Channel Scan Capability is ONLY OnBoot Scans */
#define MAP_CHSCAN_DO_ONBOOT(flags)	((flags) & (MAP_CHSCAN_SCANTYPE_DO_ONBOOT))
/* Check whether Channel Scan Performs OnBoot Scans or not */
#define MAP_CHSCAN_DO_REQ(flags)	((flags) & (MAP_CHSCAN_SCANTYPE_DO_REQ))
/* Check whether Channel Scan Performs Requested Scans or not */
#define MAP_CHSCAN_ONLY_ONBOOT(flags)	((flags) == (MAP_CHSCAN_SCANTYPE_DO_ONBOOT))

/* Channel Scan NVRAMs default values */
#define DEF_MAP_CHSCAN_SCANTYPE_FLAGS	(MAP_CHSCAN_SCANTYPE_DO_ONBOOT | \
					MAP_CHSCAN_SCANTYPE_DO_REQ)
					/* Agent performs both OnBoot & Requested scans */

#define DEF_MAP_CHSCAN_IMPACT		MAP_CHSCAN_CAP_SCAN_IMPACT_TMSLICE /* Radio
	* may go off channel for a series of short intervals perform Channel Scans
	*/
#define DEF_MAP_CHSCAN_MIN_SCAN_INT	900 /* Default minimum scan interval */

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

/** Maps a user space bitmap to an EHT mcs code, doet not take MCS14/15 into account */
#ifndef EHT_MCS_MAP_TO_MAX_MCS
#define EHT_MCS_MAP_TO_MAX_MCS(mcs_map)	\
	({								\
	 uint16 _mcs_map = (mcs_map) & WL_EHT_CAP_MCS_0_13_MAP;		\
	 (_mcs_map) == WL_EHT_CAP_MCS_0_7_MAP ? EHT_MCS_CODE_0_7 :	\
	 (_mcs_map) == WL_EHT_CAP_MCS_0_9_MAP ? EHT_MCS_CODE_0_9 :	\
	 (_mcs_map) == WL_EHT_CAP_MCS_0_11_MAP ? EHT_MCS_CODE_0_11 :	\
	 (_mcs_map) == WL_EHT_CAP_MCS_0_13_MAP ? EHT_MCS_CODE_0_13 :	\
	 EHT_MCS_CODE_NONE;						\
	 })
#endif /* EHT_MCS_MAP_TO_MAX_MCS */

#ifndef EHT_MAX_MCS_TO_INDEX
#define EHT_MAX_MCS_TO_INDEX(mcs_map) \
	((mcs_map == EHT_MCS_CODE_0_13) ? 13 : \
	(mcs_map == EHT_MCS_CODE_0_11) ? 11 : \
	(mcs_map == EHT_MCS_CODE_0_9) ? 9 : \
	(mcs_map == EHT_MCS_CODE_0_7) ? 7 : \
	7)
#endif /* EHT_MAX_MCS_TO_INDEX */

#ifndef ASSERT
#define ASSERT(exp) assert(exp)
#endif /* ASSERT */

/* File Macros */
#define BKT_DEFAULT_NVRAM_FILE    "/tmp/wbd_%s_nvrams.txt"

/* -------------------------- Blanket Debug Print Macros ------------------------ */

static blanket_module_info_t g_bkt_info = {BKT_DEBUG_DEFAULT, 0};
char g_bkt_process_name[BKT_MAX_PROCESS_NAME];

#define BKT_PRINT(prefix, fmt, arg...) \
	printf(prefix"BKT-%s >> (%lld) %s: "fmt, g_bkt_process_name, (long long)time(NULL), \
	__FUNCTION__, ##arg)

#define BKT_ERROR(fmt, arg...) \
	if (g_bkt_info.msglevel & BKT_DEBUG_ERROR) \
		BKT_PRINT("Err: ", fmt, ##arg)

#define BKT_WARNING(fmt, arg...) \
	if (g_bkt_info.msglevel & BKT_DEBUG_WARNING) \
		BKT_PRINT("Warn: ", fmt, ##arg)

#define BKT_INFO(fmt, arg...) \
	if (g_bkt_info.msglevel & BKT_DEBUG_INFO) \
		BKT_PRINT("Info: ", fmt, ##arg)

#define BKT_DEBUG(fmt, arg...) \
	if (g_bkt_info.msglevel & BKT_DEBUG_DETAIL) \
		BKT_PRINT("Dbg: ", fmt, ##arg)

#define BKT_TRACE(fmt, arg...) \
	if (g_bkt_info.msglevel & BKT_DEBUG_TRACE) \
		BKT_PRINT("Trace: ", fmt, ##arg)

#define BKT_ENTER()	BKT_TRACE("Enter...\n")
#define BKT_EXIT()	BKT_TRACE("Exit...\n")

#define BKT_WL_DUMP_ENAB	(g_bkt_info.msglevel & BKT_DEBUG_DETAIL)

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
		memcpy(dst, src, len -1); \
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

/* Convert chanspec bandwidth to wlc bandwidth */
static uint32 blanket_chanspec_bw_to_wlc_bw(chanspec_t chanspec);

/* Get regulated tx power for given chanspec */
static int blanket_get_reg_txpwr(chanspec_t chanspec, wl_chanspec_reg_txpwr_t *txpwr_list);

/* Get regulated tx power list for given band and band width of chanspec */
static int blanket_get_reg_txpwr_list(char *ifname, uint16 band_bw,
		wl_chanspec_reg_txpwr_t *txpwr_list, int txpwr_list_len);

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

	g_bkt_info.msglevel = info->msglevel;
end:
	BKT_EXIT();
	return ret;
}

/* Set message level for printing debug messages */
void blanket_set_msglevel(unsigned int msglevel)
{
	g_bkt_info.msglevel = msglevel;
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

		scb_val.val = htod32((uint32)reason);
		ret = wl_ioctl(ifname, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val,
				sizeof(scb_val));
		BKT_INFO("wl -i %s WLC_SCB_DEAUTHENTICATE_FOR_REASON "MACF" %d\n", ifname,
			ETHERP_TO_MACF(addr), reason);
	} else {
		/* reason code 0 is reserved hence send deauth without reason */
		ret = wl_ioctl(ifname, WLC_SCB_DEAUTHENTICATE, addr, ETHER_ADDR_LEN);
		BKT_INFO("wl -i %s WLC_SCB_DEAUTHENTICATE "MACF"\n", ifname, ETHERP_TO_MACF(addr));
	}

	BKT_IOVAR_ASSERT(ifname, "Deauth", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Send Disassoc to the sta */
int
blanket_disassoc_sta(char* ifname, struct ether_addr* addr)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* If address is provided, then send disassoc to the STA else self disassoc */
	if (addr) {
		ret = wl_ioctl(ifname, WLC_DISASSOC, addr, ETHER_ADDR_LEN);
	} else {
		ret = wl_ioctl(ifname, WLC_DISASSOC, NULL, 0);
	}

	BKT_IOVAR_ASSERT(ifname, "WLC_DISASSOC", ret);

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
		        wf_chspec_ntoa_ex(chspec, chanbuf), chspec);
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
			/* legacy BSS info version is not used for 6GHz devices */
			bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel,
					WL_CHANNEL_2G5G_BAND(old_bi->channel));
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

/* Remove all UNII4 Chanspecs from Chanspecs List */
static uint32
blanket_skip_unii4_ch_fm_chlist(uint32 *chlist, uint32 count)
{
	chanspec_t unii4_chanspecs[] = {
		        0xd0a9, 0xd0ad, 0xd0b1,	/*         169     173     177     */
		0xd8a7, 0xd9a7, 0xd8af, 0xd9af,	/* 165l    169u    173l    177u    */
		0xe0ab, 0xe1ab, 0xe2ab, 0xe3ab,	/* 165/80  169/80  173/80  177/80  */
		0xeca3, 0xeda3, 0xeea3, 0xefa3,	/* 165/160 169/160 173/160 177/160 */
		0xe8a3, 0xe9a3, 0xeaa3, 0xeba3	/* 149/160 153/160 157/160 161/160 */
	};
	uint32 needle = 0, iter, iter_unii;
	uint32 new_count = 0, unii4_count = ARRAYSIZE(unii4_chanspecs);

	for (iter = 0; iter < count; iter++) {
		for (iter_unii = 0; iter_unii < unii4_count; iter_unii++) {
			if (chlist[iter] == unii4_chanspecs[iter_unii]) {
				chlist[iter] = needle;
				break;
			}
		}
	}
	for (iter = 0; iter < count; iter++) {
		if (chlist[iter] != needle) {
			chlist[new_count++] = chlist[iter];
		}
	}
	return new_count;
}

/* Get list of chanspec supported by driver */
int
blanket_get_chanspecs_list(char* ifname, wl_uint32_list_t* chlist, int chlistlen)
{
	int ret = BKTE_OK;
	uint32 count = 0;
	int i = 0, wlc_band = 0;
	chanspec_t chanspec, input = 0x00;
	bool skip_unii4 = (strtol(nvram_safe_get(NVRAM_MAP_CERT), NULL, 0) ||
		strtol(nvram_safe_get(NVRAM_MAP_SKIP_UNII4_CHANNELS), NULL, 0));
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

	/* Skip UNII4 Chanspecs, if asked */
	if (skip_unii4) {
		ret = blanket_get_band(ifname, &wlc_band);
		BKT_IOVAR_ASSERT(ifname, "get band", ret);
		if (wlc_band == WLC_BAND_5G) {
			count = blanket_skip_unii4_ch_fm_chlist(chlist->element, count);
			chlist->count = count;
		}
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
blanket_get_chan_info(char* ifname, uint channel, uint band, uint *out_bitmap)
{
	int ret = BKTE_OK;
	uint8 buf[BKT_MAX_BUF_32] = {0};
	uint32 chanspec_arg;
	BKT_ENTER();

	wl_endian_probe(ifname);

	chanspec_arg = CH20MHZ_CHSPEC(channel, band);

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

/* Set Interface specific chan_info */
int
blanket_set_chan_info(char* ifname, chanspec_t chanspec, uint32 per_chan_info)
{
	int ret = BKTE_OK;
	uint channel, last_channel;
	uint8 buf[BKT_MAX_BUF_32] = {0};
	wl_set_chan_info_t chan_info;
	BKT_ENTER();

	wl_endian_probe(ifname);

	memset(&chan_info, 0, sizeof(chan_info));
	chan_info.version = htod16(WL_SET_CHAN_INFO_VER);
	chan_info.length = htod16(sizeof(wl_set_chan_info_t));
	chan_info.type = htod16(WL_SET_CHAN_INFO_TYPE);
	chan_info.per_chan_info = htod32(per_chan_info);

	FOREACH_20_SB_EFF(chanspec, channel, last_channel) {
		chan_info.chanspec = htod16(CH20MHZ_CHSPEC(channel, CHSPEC_BAND(chanspec)));
		ret = wl_iovar_setbuf(ifname, "per_chan_info", &chan_info,
				sizeof(chan_info), buf, sizeof(buf));
		if (ret != BCME_OK) {
			BKT_ERROR("setting chan_info(%d) for ch %d returned %d\n",
				per_chan_info, channel, ret);
			goto end;
		}
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
/* Get max rate of interface */
int
blanket_get_max_rate(char *ifname, int *out_max_rate)
{
	int ret = BKTE_OK;
	int max_rate = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_max_rate, BKTE_INV_ARG);

	wl_endian_probe(ifname);
	ret = wl_ioctl(ifname, WLC_GET_MAX_RATE, &max_rate, sizeof(max_rate));
	BKT_IOVAR_ASSERT(ifname, "max_rate", ret);
	max_rate = dtoh32(max_rate);
	/* max_rate here comes in units of 500 Kbits/s
	 * and we will return it in units of 1000 Kbits/s
	 */
	*out_max_rate = max_rate / 2;
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

	BKT_DEBUG("major version for %s:%d\n", ifname, wlc_ver.wlc_ver_major);
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
	nbr_ie->chanspec = nbr->chanspec;
	nbr_ie->bss_trans_preference = 64; /* as statically added */
	nbr_ie->len = sizeof(nbr_rpt_elem_t);
	if (CHSPEC_IS6G(nbr->chanspec)) {
		nbr_ie->bss_params = TBTT_INFO_BSS_PARAMS_MBSSID;
		nbr_ie->tpe_psd = 0xfe;
	}

	BKT_DEBUG("nbr->len[%d], nbr->id[%d], nbr->version[%d], ssid [%s], ssid len[%d] \n",
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

/* Get HT Capabilities from bss_info */
int
blanket_get_ht_cap(char *ifname, wl_bss_info_t *in_bi, unsigned char *out_ht_caps)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;
	int max_nss = 0;
	unsigned char caps = 0;
	uint32 nbss_cap;
	BKT_ENTER();

	memset(out_ht_caps, 0, sizeof(*out_ht_caps));

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
	nbss_cap = dtoh32(bi->nbss_cap);
	if (nbss_cap & HT_CAP_40MHZ) {
		caps |= IEEE1905_AP_HTCAP_40MHZ;
	}
	if (nbss_cap & HT_CAP_SHORT_GI_40) {
		caps |= IEEE1905_AP_HTCAP_SGI_40MHZ;
	}
	if (nbss_cap & HT_CAP_SHORT_GI_20) {
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
blanket_get_vht_cap(char *ifname, wl_bss_info_t *in_bi, ieee1905_vht_caps_type *out_vht_caps)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;
	int max_nss = 0, vhtmode = 0;
	uint32 nbss_cap;
	BKT_ENTER();

	memset(out_vht_caps, 0, sizeof(*out_vht_caps));

	/* Validate arg */
	BKT_ASSERT_ARG(out_vht_caps, BKTE_INV_ARG);

	/* get vhtmode for this Interface */
	ret = blanket_get_vhtmode(ifname, &vhtmode);
	BKT_ASSERT();

	if (!vhtmode) {
		goto end;
	}

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, &bi);
		BKT_ASSERT();
	} else {
		bi = in_bi;
	}

	out_vht_caps->Valid = 1;

	/* Get max_nss */
	max_nss = wl_wlif_get_max_nss(bi);

	/* Set caps, bits 0-3 are reserved.
	 * 4	MU Beamformer capable.
	 * 5	SU Beamformer capable.
	 * 6	VHT support for 160 MHz.
	 * 7	VHT support for 80+80 MHz.
	 */
	nbss_cap = dtoh32(bi->nbss_cap);
	if (nbss_cap & VHT_BI_CAP_MU_BEAMFMR) {
		out_vht_caps->Caps |= IEEE1905_AP_VHTCAP_MU_BEAMFMR;
	}
	if (nbss_cap & VHT_BI_CAP_SU_BEAMFMR) {
		out_vht_caps->Caps |= IEEE1905_AP_VHTCAP_SU_BEAMFMR;
	}
	if (nbss_cap & VHT_BI_160MHZ) {
		out_vht_caps->Caps |= IEEE1905_AP_VHTCAP_160MHZ;
	}
	if (nbss_cap & VHT_BI_8080MHZ) {
		out_vht_caps->Caps |= IEEE1905_AP_VHTCAP_80p80MHZ;
	}

	/* Set caps_ex.
	 * 0	SGI support for 160 MHz and 80+80 MHz.
	 * 1	SGI support for 80 MHz.
	 */
	if (nbss_cap & VHT_BI_SGI_160MHZ) {
		out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_SGI_160MHZ;
	}
	if (nbss_cap & VHT_BI_SGI_80MHZ) {
		out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_SGI_80MHZ;
	}
	/* Bits 4-2 are used for max Rx Chains and 7-5 for Tx chains with below mapping:
	 *		Rx			Tx		NSS
	 * Bits 	4  3  2 		7  6  5 	Value
	 *
	 *		0  0  0 		0  0  0 	1
	 *		0  0  1 		0  0  1 	2
	 *		0  1  0 		0  1  0 	3
	 *		0  1  1 		0  1  1 	4
	 *		1  1  1 		1  1  1 	8
	 */
	switch (max_nss) {
		case BKT_NSS_2:
			out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_RX_NSS_2;
			out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_TX_NSS_2;
		break;

		case BKT_NSS_3:
			out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_RX_NSS_3;
			out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_TX_NSS_3;
		break;

		case BKT_NSS_4:
			out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_RX_NSS_4;
			out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_TX_NSS_4;
		break;

		case BKT_NSS_8:
			out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_RX_NSS_8;
			out_vht_caps->CapsEx |= IEEE1905_AP_VHTCAP_TX_NSS_8;
		break;

		default:
		break;
	}

	/* Fill the vht_tx/rx mcs map. */
	out_vht_caps->TxMCSMap = dtoh16(bi->vht_txmcsmap);
	out_vht_caps->RxMCSMap = dtoh16(bi->vht_rxmcsmap);

end:
	BKT_EXIT();
	return ret;
}

/* Get the txbf_bfr_cap of the Radio (Primary Interface) */
int
blanket_get_bfr_cap(char *ifname, int *out_bfr_cap)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_bfr_cap, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "txbf_bfr_cap", out_bfr_cap, sizeof(*out_bfr_cap));
	BKT_IOVAR_ASSERT(ifname, "get txbf_bfr_cap", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get the Max_mu clientss of the Radio (Primary Interface) */
int
blanket_get_ofdma(char *ifname, max_clients_t *mu)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(mu, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "max_muclients", mu, sizeof(*mu));
	BKT_IOVAR_ASSERT(ifname, "get max_muclients", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get the Mu bssid enabled value off the Radio (Primary Interface) */
int
blanket_get_mbssid(char *ifname, wl_mbssid_t *cfg)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(cfg,  BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "mbssid", cfg, sizeof(*cfg));
	BKT_IOVAR_ASSERT(ifname, "get mbssid", ret);

end:
	BKT_EXIT();
	return ret;
}

/* To get the  TWT_REQ  Enabled value in IOVAR */
int
blanket_get_twt_req_cap(char *ifname, int *twt_req)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	ret = wl_iovar_get(ifname, "twt_req_cap", twt_req, sizeof(*twt_req));
	BKT_IOVAR_ASSERT(ifname, "twt_req_cap", ret);

end:
	BKT_EXIT();
	return 0;
}

/* To get the  TWT_ RESP  Enabled value in IOVAR */
int
blanket_get_twt_resp_cap(char *ifname, int *twt_resp)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	ret = wl_iovar_get(ifname, "twt_resp_cap", twt_resp, sizeof(*twt_resp));
	BKT_IOVAR_ASSERT(ifname, "twt_resp_cap", ret);

end:
	BKT_EXIT();
	return 0;
}

/* To get the RTS Enabled value in IOVAR */
int
blanket_get_rts_cap(char *ifname, int *rts)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	ret = wl_iovar_get(ifname, "rtsthresh", rts, sizeof(*rts));
	BKT_IOVAR_ASSERT(ifname, "rtsthresh", ret);

end:
	BKT_EXIT();
	return 0;
}

/* Get the He_features of the Radio Interface */
int
blanket_get_cmd_he_features(char *ifname, int *he_features, uint8 he_cmd)
{
	int ret = BKTE_OK;
	uint8 buf[BKT_MAX_BUF_16] = {0};
	bcm_xtlv_t *getbuf = NULL;
	BKT_ENTER();

	getbuf = (bcm_xtlv_t *)bkt_malloc(sizeof(*getbuf), &ret);
	BKT_ASSERT();

	getbuf->id = he_cmd;
	ret = wl_iovar_getbuf(ifname, "he", getbuf, sizeof(*getbuf), buf, sizeof(buf));
	*he_features = *buf;

end:
	if (getbuf)
		free(getbuf);
	BKT_EXIT();
	return 0;
}

/* Get the txbf_bfe_cap of the Radio (Primary Interface) */
int
blanket_get_bfe_cap(char *ifname, int *out_bfe_cap)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_bfe_cap, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "txbf_bfe_cap", out_bfe_cap, sizeof(*out_bfe_cap));
	BKT_IOVAR_ASSERT(ifname, "get txbf_bfe_cap", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get the mu_features of the Radio (Primary Interface) */
int
blanket_get_mu_features(char *ifname, int *out_mu_features)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_mu_features, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "mu_features", out_mu_features, sizeof(int));
	BKT_IOVAR_ASSERT(ifname, "get mu_features", ret);

	*out_mu_features = dtoh32(*out_mu_features);

end:
	BKT_EXIT();
	return ret;
}

/* Get HE MCS Capabilities from bss_info */
int
blanket_get_he_cap(char *ifname, wl_bss_info_t *in_bi, ieee1905_he_caps_type *out_he_caps)
{
	int ret = BKTE_OK;
	wl_bss_info_v109_1_t *bi;
	int max_nss = 0, bfr_cap = 0, mu_features = 0;
	BKT_ENTER();

	memset(out_he_caps, 0, sizeof(*out_he_caps));

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
		out_he_caps->Caps |= IEEE1905_AP_HECAP_SU_BEAMFMR;
	}
	if (bfr_cap & TXBF_HE_MU_BFR_CAP) {
		out_he_caps->Caps |= IEEE1905_AP_HECAP_MU_BEAMFMR;
	}
	if (mu_features & MU_FEATURES_MURX) {
		out_he_caps->Caps |= IEEE1905_AP_HECAP_UL_MUMIMO;
	}

	if (mu_features & MU_FEATURES_MURX) {
		out_he_caps->Caps |= IEEE1905_AP_HECAP_UL_MUMIMO_OFDMA;
	}
	if (mu_features & MU_FEATURES_MUTX) {
		out_he_caps->Caps |= IEEE1905_AP_HECAP_DL_MUMIMO_OFDMA;
	}
	out_he_caps->Caps |= IEEE1905_AP_HECAP_UL_OFDMA;
	out_he_caps->Caps |= IEEE1905_AP_HECAP_DL_OFDMA;

	/* Set 160MHz and 80p80MHz bits if valid respective mcs maps are present */
	if ((bi->he_sup_bw160_tx_mcs != 0xffff) || (bi->he_sup_bw160_rx_mcs != 0xffff)) {
		out_he_caps->CapsEx |= IEEE1905_AP_HECAP_160MHZ;
	}
	if ((bi->he_sup_bw80p80_tx_mcs != 0xffff) || (bi->he_sup_bw80p80_rx_mcs != 0xffff)) {
		out_he_caps->CapsEx |= IEEE1905_AP_HECAP_80P80MHZ;
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
			out_he_caps->CapsEx |= IEEE1905_AP_HECAP_RX_NSS_2;
			out_he_caps->CapsEx |= IEEE1905_AP_HECAP_TX_NSS_2;
		break;

		case BKT_NSS_3:
			out_he_caps->CapsEx |= IEEE1905_AP_HECAP_RX_NSS_3;
			out_he_caps->CapsEx |= IEEE1905_AP_HECAP_TX_NSS_3;
		break;

		case BKT_NSS_4:
			out_he_caps->CapsEx |= IEEE1905_AP_HECAP_RX_NSS_4;
			out_he_caps->CapsEx |= IEEE1905_AP_HECAP_TX_NSS_4;
		break;

		case BKT_NSS_8:
			out_he_caps->CapsEx |= IEEE1905_AP_HECAP_RX_NSS_8;
			out_he_caps->CapsEx |= IEEE1905_AP_HECAP_TX_NSS_8;
		break;

		default:
		break;
	}

	/* Fill the he tx/rx mcs map. */
	out_he_caps->TxBW80MCSMap = dtoh16(bi->he_sup_bw80_tx_mcs);
	out_he_caps->RxBW80MCSMap = dtoh16(bi->he_sup_bw80_rx_mcs);
	out_he_caps->TxBW160MCSMap = dtoh16(bi->he_sup_bw160_tx_mcs);
	out_he_caps->RxBW160MCSMap = dtoh16(bi->he_sup_bw160_rx_mcs);
	out_he_caps->TxBW80p80MCSMap = dtoh16(bi->he_sup_bw80p80_tx_mcs);
	out_he_caps->RxBW80p80MCSMap = dtoh16(bi->he_sup_bw80p80_rx_mcs);

	out_he_caps->Valid = 1;

end:
	BKT_EXIT();
	return ret;
}

/* Gets EHT MCS Capabilities from bss_info */
int
blanket_get_eht_cap(char *ifname, wl_bss_info_t *in_bi, ieee1905_eht_caps_type *out_eht_caps)
{
	int ret = BKTE_OK;
	wl_bss_info_v109_1_t *bi;
	bool eht_cap = FALSE;
	uint32 version, length, ie_length;
	uint32 ie_length_rounded_up; /* compensates for trailing padding bytes */
	uint16 (*p)[WL_RXTX_SZ][WL_EHT_BW_SZ][EHT_MCS_MAP_NSS_MAX];
	static const uint16 zero[WL_RXTX_SZ][WL_EHT_BW_SZ][EHT_MCS_MAP_NSS_MAX];
	enum wl_eht_bw_e bw_idx;
	int max_nss = 1;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_eht_caps, BKTE_INV_ARG);

	memset(out_eht_caps, 0, sizeof(*out_eht_caps));

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, (wl_bss_info_t **)&bi);
		BKT_ASSERT();
	} else {
		bi = (wl_bss_info_v109_1_t *)in_bi;
	}

	version = dtoh32(bi->version);
	length = dtoh32(bi->length);
	ie_length = dtoh32(bi->ie_length);
	ie_length_rounded_up = ROUNDUP(ie_length, 4);

#if WL_EHT_IOCTL_VER >= 2 /* then struct wl_bss_info has EHT members */
	if (version != LEGACY_WL_BSS_INFO_VERSION) {
		/* ROUNDUP to compensate for wlc_bss2wl_bss() */
		int sizeof_struct = length - ie_length_rounded_up;
		if (sizeof_struct > 0) {
			if ((uint)sizeof_struct > OFFSETOF(wl_bss_info_v109_1_t, eht_cap)) {
				eht_cap = bi->eht_cap;
			}
		}
	}

	if (!eht_cap) {
		goto end;
	}

	p = &bi->eht_sup;
	if (*p == NULL || !memcmp(*p, &zero, sizeof(zero))) {
		BKT_DEBUG("%s eht capabilities are not present\n", ifname);
		goto end;
	}

	for (bw_idx = WL_EHT_BW204080; bw_idx <= WL_EHT_BW320; bw_idx++) {
		enum wlc_eht_mcs_code_e mcs_code = EHT_MCS_CODE_NONE;
		int nss, tx;
		uint16 mcs_bitmap;
		uint32 mcs_map = WLC_EHT_MCS_MAP_MCS_NONE;
		enum wlc_eht_mcs_code_e mc;
		for (nss = 1; nss <= 8; nss++) {
			for (tx = WL_RX; tx < WL_RXTX_SZ; tx++) {
				mcs_bitmap = dtoh16((*p)[tx][bw_idx][nss-1]);
				mcs_code = EHT_MCS_MAP_TO_MAX_MCS(mcs_bitmap);
				if (mcs_code == EHT_MCS_CODE_NONE || mcs_code > EHT_MCS_CODE_0_13) {
					break;
				}
				mc = mcs_code;
				do {
					WLC_EHT_RS_SET_NSS(mcs_map, tx, mc, nss);
				} while (--mc >= EHT_MCS_CODE_0_7);
			}

			if (mcs_code == EHT_MCS_CODE_NONE || mcs_code > EHT_MCS_CODE_0_13) {
				break;
			}

			if (nss > max_nss)
				max_nss = nss;
		}

		switch (bw_idx) {
			case WL_EHT_BW204080:
				out_eht_caps->BWle80MCSMap = mcs_map;
				break;

			case WL_EHT_BW160:
				if (mcs_map != WLC_EHT_MCS_MAP_MCS_NONE) {
					out_eht_caps->BW160MCSMap = mcs_map;
					out_eht_caps->Caps |= IEEE1905_AP_EHTCAP_160MHZ;
				}
				break;

			case WL_EHT_BW320:
				if (mcs_map != WLC_EHT_MCS_MAP_MCS_NONE) {
					out_eht_caps->BW320MCSMap = mcs_map;
					out_eht_caps->Caps |= IEEE1905_AP_EHTCAP_320MHZ;
				}
				break;

			default:
				BKT_ERROR("Invalid bw idx %d \n", bw_idx);
				break;
		}
	}

	out_eht_caps->MaxNSS = max_nss;
	out_eht_caps->Valid = 1;
#endif /* WL_EHT_IOCTL_VER */

	BKT_INFO("%s mcs_map_bwle80[0x%08x] mcs_map_bw160[0x%08x] mcs_map_bw320[0x%08x] "
		"max_nss[%u] caps[0x%2x] valid[%d]\n", ifname,
		out_eht_caps->BWle80MCSMap, out_eht_caps->BW160MCSMap,
		out_eht_caps->BW320MCSMap, out_eht_caps->MaxNSS,
		out_eht_caps->Caps, out_eht_caps->Valid);

end:
	BKT_EXIT();
	return ret;
}

/* Get Basic Capabilities */
int
blanket_get_basic_cap(uint8 *out_basic_caps)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	if (out_basic_caps) {
		*out_basic_caps = (uint8)blanket_get_config_val_uint(NULL, NVRAM_MAP_AP_CAPS,
			DEF_MAP_AP_CAPS);
		BKT_DEBUG("AP Capability : 0x%02x\n", *out_basic_caps);
	}

	BKT_EXIT();
	return ret;
}

static uint16 rc_map[] =
{
	/* 2g 20MHz */
	81 /* regulatory class */, 13 /* Number of chanspecs */,
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
	125, 8,
	0xd095, 0xd099, 0xd09d, 0xd0a1, 0xd0a5, 0xd0a9, 0xd0ad, 0xd0b1,

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
	126, 4,
	0xd897, 0xd89f, 0xd8a7, 0xd8af,

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
	127, 4,
	0xd997, 0xd99f, 0xd9a7, 0xd9af,
	/* 5g 80Mhz */
	128, 28,
	0xe02a, 0xe03a, 0xe06a, 0xe07a, 0xe08a, 0xe09b, 0xe0ab,
	0xe12a, 0xe13a, 0xe16a, 0xe17a, 0xe18a, 0xe19b, 0xe1ab,
	0xe22a, 0xe23a, 0xe26a, 0xe27a, 0xe28a, 0xe29b, 0xe2ab,
	0xe32a, 0xe33a, 0xe36a, 0xe37a, 0xe38a, 0xe39b, 0xe3ab,

	/* 5g 160Mhz */
	129, 24,
	0xe832, 0xe932, 0xea32, 0xeb32, 0xec32, 0xed32, 0xee32, 0xef32,
	0xe872, 0xe972, 0xea72, 0xeb72, 0xec72, 0xed72, 0xee72, 0xef72,
	0xe8a3, 0xe9a3, 0xeaa3, 0xeba3, 0xeca3, 0xeda3, 0xeea3, 0xefa3,

	/* 6G, 20 */
	131, 59,
	(0x5001), (0x5005), (0x5009), (0x500d), (0x5011), (0x5015),
	(0x5019), (0x501d), (0x5021), (0x5025), (0x5029), (0x502d), (0x5031),
	(0x5035), (0x5039), (0x503d), (0x5041), (0x5045), (0x5049), (0x504d),
	(0x5051), (0x5055), (0x5059), (0x505d), (0x5061), (0x5065), (0x5069),
	(0x506d), (0x5071), (0x5075), (0x5079), (0x507d), (0x5081), (0x5085),
	(0x5089), (0x508d), (0x5091), (0x5095), (0x5099), (0x509d), (0x50a1),
	(0x50a5), (0x50a9), (0x50ad), (0x50b1), (0x50b5), (0x50b9), (0x50bd),
	(0x50c1), (0x50c5), (0x50c9), (0x50cd), (0x50d1), (0x50d5), (0x50d9),
	(0x50dd), (0x50e1), (0x50e5), (0x50e9),

	/* 6g, 40 */
	132, 58,
	(0x5803), (0x5903), (0x580b), (0x590b), (0x5813), (0x5913), (0x581b),
	(0x591b), (0x5823), (0x5923), (0x582b), (0x592b), (0x5833), (0x5933),
	(0x583b), (0x593b), (0x5843), (0x5943), (0x584b), (0x594b), (0x5853),
	(0x5953), (0x585b), (0x595b), (0x5863), (0x5963), (0x586b), (0x596b),
	(0x5873), (0x5973), (0x587b), (0x597b), (0x5883), (0x5983), (0x588b),
	(0x598b), (0x5893), (0x5993), (0x589b), (0x599b), (0x58a3), (0x59a3),
	(0x58ab), (0x59ab), (0x58b3), (0x59b3), (0x58bb), (0x59bb), (0x58c3),
	(0x59c3), (0x58cb), (0x59cb), (0x58d3), (0x59d3), (0x58db), (0x59db),
	(0x58e3), (0x59e3),

	/* 6g, 80 */
	133, 56,
	(0x6007), (0x6107), (0x6207), (0x6307), (0x6017), (0x6117), (0x6217),
	(0x6317), (0x6027), (0x6127), (0x6227), (0x6327), (0x6037), (0x6137),
	(0x6237), (0x6337), (0x6047), (0x6147), (0x6247), (0x6347), (0x6057),
	(0x6157), (0x6257), (0x6357), (0x6067), (0x6167), (0x6267), (0x6367),
	(0x6077), (0x6177), (0x6277), (0x6377), (0x6087), (0x6187), (0x6287),
	(0x6387), (0x6097), (0x6197), (0x6297), (0x6397), (0x60a7), (0x61a7),
	(0x62a7), (0x63a7), (0x60b7), (0x61b7), (0x62b7), (0x63b7), (0x60c7),
	(0x61c7), (0x62c7), (0x63c7), (0x60d7), (0x61d7), (0x62d7), (0x63d7),

	/* 6g, 160 */
	134, 56,
	(0x680f), (0x690f), (0x6a0f), (0x6b0f), (0x6c0f), (0x6d0f), (0x6e0f),
	(0x6f0f), (0x682f), (0x692f), (0x6a2f), (0x6b2f), (0x6c2f), (0x6d2f),
	(0x6e2f), (0x6f2f), (0x684f), (0x694f), (0x6a4f), (0x6b4f), (0x6c4f),
	(0x6d4f), (0x6e4f), (0x6f4f), (0x686f), (0x696f), (0x6a6f), (0x6b6f),
	(0x6c6f), (0x6d6f), (0x6e6f), (0x6f6f), (0x688f), (0x698f), (0x6a8f),
	(0x6b8f), (0x6c8f), (0x6d8f), (0x6e8f), (0x6f8f), (0x68af), (0x69af),
	(0x6aaf), (0x6baf), (0x6caf), (0x6daf), (0x6eaf), (0x6faf), (0x68cf),
	(0x69cf), (0x6acf), (0x6bcf), (0x6ccf), (0x6dcf), (0x6ecf), (0x6fcf),

	/* 6g, 20 (channel 2) */
	136, 1,
	0x5002,

	/* 6g, 320 */
	137, 96,
	(0x7000), (0x7040), (0x7080), (0x70c0), (0x7100), (0x7140), (0x7180),
	(0x71c0), (0x7200), (0x7240), (0x7280), (0x72c0), (0x7300), (0x7340),
	(0x7380), (0x73c0), (0x7001), (0x7041), (0x7081), (0x70c1), (0x7101),
	(0x7141), (0x7181), (0x71c1), (0x7201), (0x7241), (0x7281), (0x72c1),
	(0x7301), (0x7341), (0x7381), (0x73c1), (0x7002), (0x7042), (0x7082),
	(0x70c2), (0x7102), (0x7142), (0x7182), (0x71c2), (0x7202), (0x7242),
	(0x7282), (0x72c2), (0x7302), (0x7342), (0x7382), (0x73c2), (0x7003),
	(0x7043), (0x7083), (0x70c3), (0x7103), (0x7143), (0x7183), (0x71c3),
	(0x7203), (0x7243), (0x7283), (0x72c3), (0x7303), (0x7343), (0x7383),
	(0x73c3), (0x7004), (0x7044), (0x7084), (0x70c4), (0x7104), (0x7144),
	(0x7184), (0x71c4), (0x7204), (0x7244), (0x7284), (0x72c4), (0x7304),
	(0x7344), (0x7384), (0x73c4), (0x7005), (0x7045), (0x7085), (0x70c5),
	(0x7105), (0x7145), (0x7185), (0x71c5), (0x7205), (0x7245), (0x7285),
	(0x72c5), (0x7305), (0x7345), (0x7385), (0x73c5)
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

	for (idx = 0; ((idx + 2) < rc_max); idx++) {
		uint16 num_of_entries = rc_map[idx+1];

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
	} while (idx < (rc_max - 2));

	return -1;
}

/* Return band of the given operating class */
int
blanket_opclass_to_band(uint opclass)
{
	int band = -1;

	if ((opclass >= REGCLASS_24G_FIRST) && (opclass <= REGCLASS_6G_LAST)) {
		if (opclass <= REGCLASS_24G_LAST) {
			band = WL_CHANSPEC_BAND_2G;
		} else if ((opclass >= REGCLASS_6G_FIRST)) {
			band = WL_CHANSPEC_BAND_6G;
		} else {
			band = WL_CHANSPEC_BAND_5G;
		}
	}
	return band;
}

/* Prepare chanspec based on given channel, opclass, bandwidth and band */
chanspec_t
blanket_prepare_chanspec(uint channel, uint opclass, uint bw, int band)
{
	chanspec_t chanspec;

	if (bw == 0) {
		blanket_get_bw_from_rc(opclass, &bw);
	}
	if (band == 0) {
		band = blanket_opclass_to_band(opclass);
	}

	if (opclass <= REGCLASS_5G_40MHZ_LAST) {
		chanspec = wf_channel2chspec(channel, bw, band);
	} else {
		/* 6G 20MHz also will come here.
		 * It is ok because both center and control channel are same in 20MHz
		 */
		chanspec = CHBW_CHSPEC(bw, channel, band);
	}

	return chanspec;
}

/* Mask out side band details from the given chanspec */
chanspec_t
blanket_mask_chanspec_sb(chanspec_t chspec)
{
	return (~CHSPEC_CTL_SB(chspec)) & chspec;
}

/* Extract rc_map pointer for given Operating Class */
uint16*
blanket_get_chspeclist_fm_rc_map(uint8 rc)
{
	uint idx = 0;

	for (idx = 0; ((idx + 1) < rc_max); idx++) {
		uint16 num_of_entries = rc_map[idx+1];

		/* Regulatory class matches */
		if (rc_map[idx] == rc) {
			return &rc_map[idx];
		}
		idx++; /* One for number of entries */
		idx += num_of_entries;
	}

	BKT_DEBUG("Invalid regclass: %d\n", rc);
	return NULL;
}

/* Add a Chanspec to Chanspec list */
int
blanket_add_chspec_to_list(ieee1905_glist_t *list, chanspec_t chspec)
{
	int ret = BKTE_OK;
	i5_chspec_item_t *new_node = NULL;
	BKT_ENTER();

	/* Create a Chanspec item */
	new_node = (i5_chspec_item_t*)bkt_malloc(sizeof(*new_node), &ret);
	BKT_ASSERT();

	/* Fill & Initialize Chanspec item data */
	new_node->chspec = chspec;

	/* In the end, Add this new Chanspec item to Chanspec list */
	ieee1905_glist_append(list, (dll_t *) new_node);
end:
	BKT_EXIT();
	return ret;
}

/* Extract Chanspec from rc_map for given Operating Class & Channel */
int
blanket_add_chspecs_fm_rc_map(unsigned char rc, unsigned char channel,
	ieee1905_glist_t *list_excl)
{
	unsigned int in_count = list_excl->count;
	BKT_ENTER();

	/* In 2G and 5G all 20/40MHz channels has control channel in E-4.
	 * In 6G control channel is available only for 20MHz
	 */
	if (rc <= REGCLASS_5G_40MHZ_LAST || rc == REGCLASS_6G_FIRST || rc == REGCLASS_6G_LAST) {
		/* channel is control channel */
		uint bw = 0;
		blanket_get_bw_from_rc(rc, &bw);
		blanket_add_chspec_to_list(list_excl, wf_channel2chspec(channel, bw,
			blanket_opclass_to_band(rc)));

	} else {
		/* channel is center channel */
		uint idx = 0;
		uint16 *rc_chspec_map = NULL;
		uint8 num_of_entries = 0;

		/* Extract rc_map pointer for given Operating Class */
		rc_chspec_map = blanket_get_chspeclist_fm_rc_map(rc);
		if (!rc_chspec_map) {
			BKT_ERROR("Invalid regclass: %d\n", rc);
			goto end;
		}
		idx++;
		num_of_entries = rc_chspec_map[idx++];

		/* Extract Non-operable Chanspecs from rc_map, matching Center Channel */
		while (num_of_entries) {
			if (channel == CHSPEC_CHANNEL(rc_chspec_map[idx])) {
				blanket_add_chspec_to_list(list_excl, rc_chspec_map[idx]);
			}
			num_of_entries--;
			idx++;
		}
	}

end:
	BKT_EXIT();
	return (list_excl->count - in_count);
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

/* Convert chanspec bandwidth to wlc bandwidth */
static uint32
blanket_chanspec_bw_to_wlc_bw(chanspec_t chanspec)
{
	uint32 wlc_bw = 0;
	chanspec_bw_t chanspec_bw;

	chanspec_bw = chanspec & WL_CHANSPEC_BW_MASK;
	switch (chanspec_bw) {
		case WL_CHANSPEC_BW_20:
			wlc_bw = WLC_BW_20MHZ_BIT;
		break;

		case WL_CHANSPEC_BW_40:
			wlc_bw = WLC_BW_40MHZ_BIT;
		break;

		case WL_CHANSPEC_BW_80:
			wlc_bw = WLC_BW_80MHZ_BIT;
		break;

		case WL_CHANSPEC_BW_160:
			wlc_bw = WLC_BW_160MHZ_BIT;
		break;

		case WL_CHANSPEC_BW_320:
			wlc_bw = WLC_BW_320MHZ_BIT;
		break;

		default:
		break;
	}

	return wlc_bw;
}

/* Get regulated tx power for given chanspec */
static int
blanket_get_reg_txpwr(chanspec_t chanspec, wl_chanspec_reg_txpwr_t *txpwr_list)
{
	int32 i;

	if (!txpwr_list) {
		BKT_WARNING("txpwr_list is NULL\n");
		return 0;
	}

	for (i = 0; i < txpwr_list->count; i++) {
		if (chanspec == txpwr_list->reg_txpwr[i].chanspec) {
			return txpwr_list->reg_txpwr[i].reg_txpwr;
		}
	}

	BKT_DEBUG("no matching reg_txpwr for 0x%04x\n", chanspec);
	return 0;
}

/* Get regulated tx power list for given band and bandwidth of chanspec */
static int
blanket_get_reg_txpwr_list(char *ifname, uint16 band_bw,
		wl_chanspec_reg_txpwr_t *txpwr_list, int txpwr_list_len)
{
	wl_chanspec_reg_txpwr_t txpwr_param = {0};
	int i, ret = BKTE_OK;

	BKT_ASSERT_ARG(txpwr_list, BKTE_INV_ARG);
	BKT_ASSERT_ARG(txpwr_list_len, BKTE_INV_ARG);

	txpwr_param.ver = htod16(WL_CHANSPEC_REG_TXPWR_VER);
	txpwr_param.len = htod16(WL_CHANSPEC_REG_TXPWR_LEN);
	txpwr_param.count = htod32(1);
	txpwr_param.reg_txpwr[0].chanspec = htod16(band_bw);

	ret = wl_iovar_getbuf(ifname, "chanspec_reg_txpwr",
		&txpwr_param, sizeof(wl_chanspec_reg_txpwr_t),
		txpwr_list, txpwr_list_len);
	BKT_IOVAR_ASSERT(ifname, "chanspec_reg_txpwr", ret);

	txpwr_list->len = dtoh16(txpwr_list->len);
	txpwr_list->count = dtoh32(txpwr_list->count);
	for (i = 0; i < txpwr_list->count; i++) {
		txpwr_list->reg_txpwr[i].chanspec = dtoh16(txpwr_list->reg_txpwr[i].chanspec);
	}
end:
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
blanket_get_radio_cap(char *ifname, ieee1905_radio_caps_type *out_radio_caps)
{
	int32 ret = BKTE_OK, ch_idx, in_idx, k, rc, start, endpos, nrc, out_idx;
	wl_uint32_list_t *list = NULL;
	wl_uint32_list_t *tmplist = NULL;
	char *buf = NULL;
	chanspec_t input = 0x0;
	/* non operable chan bit map per regclass */
	uint8 invalid_chan_bitmap[(MAXCHANNEL + 7) / NBBY] = {0};
	uint8 *tmp_ptr_ch_count;
	uint32 bw_cap;
	wl_chanspec_reg_txpwr_t *txpwr_list = NULL;
	uint16 rc_band_bw = 0, kept_txpwr_band_bw = 0;

	BKT_ENTER();

	memset(out_radio_caps, 0, sizeof(*out_radio_caps));

	/* Validate arg */
	BKT_ASSERT_ARG(out_radio_caps, BKTE_INV_ARG);

	out_radio_caps->maxBSSSupported = (unsigned char)blanket_get_config_val_int(NULL,
		NVRAM_MAP_MAX_BSS, DEF_MAX_BSS_SUPPORTED);

	/* Allocate buffer for getting List of operating class */
	out_radio_caps->ListSize = BKT_MAX_BUF_512;
	out_radio_caps->List = (unsigned char *)bkt_malloc(out_radio_caps->ListSize, &ret);
	BKT_ASSERT();

	buf = (char *)bkt_malloc(WLC_IOCTL_MAXLEN, &ret);
	BKT_ASSERT();

	ret = wl_iovar_getbuf(ifname, "chanspecs", &input, sizeof(input), buf, WLC_IOCTL_MAXLEN);
	BKT_IOVAR_ASSERT(ifname, "chanspecs", ret);

	tmplist = (wl_uint32_list_t*)buf;
	tmplist->count = dtoh32(tmplist->count);

	/* Now get the bandwidth cap and based on bandwidth cap choose the chanspecs */
	blanket_get_chanspec(ifname, &input);
	bw_cap = blanket_get_bw_cap(ifname, CHSPEC_BANDTYPE(input));

	list = (wl_uint32_list_t *)bkt_malloc((sizeof(wl_uint32_list_t) * tmplist->count), &ret);
	BKT_ASSERT();

	for (ch_idx = 0, in_idx = 0; ch_idx < tmplist->count; ch_idx++) {
		if ((CHSPEC_IS20(tmplist->element[ch_idx]) && WL_BW_CAP_20MHZ(bw_cap)) ||
			(CHSPEC_IS40(tmplist->element[ch_idx]) && WL_BW_CAP_40MHZ(bw_cap)) ||
			(CHSPEC_IS80(tmplist->element[ch_idx]) && WL_BW_CAP_80MHZ(bw_cap)) ||
			(CHSPEC_IS160(tmplist->element[ch_idx]) && WL_BW_CAP_160MHZ(bw_cap)) ||
			(CHSPEC_IS320(tmplist->element[ch_idx]) && WL_BW_CAP_320MHZ(bw_cap))) {
			list->element[in_idx++] = tmplist->element[ch_idx];
		}
	}
	list->count = in_idx;

	in_idx = 0;
	out_idx = 1;	/* 0th will contain the count of rclass. */

	txpwr_list = (wl_chanspec_reg_txpwr_t *)bkt_malloc(BKT_TXPWR_BUF_LEN, &ret);
	BKT_ASSERT();

	/* Traverse rc_map for all chanspecs correspond to each opclass. If chanspec
	 * matches with firmware's list of chanspec(list->element), exclude corresponding
	 * channel(center channel for 128, 129 and 130) from list of channels to be
	 * prepared for radio capability(store in "invalid_chan_bitmap" and use this bitmap
	 * to prepare radio capability per opclass for interface).
	 */

	do {
		uint8 valid = 0, ch_count = 0;
		int txpwr = 0, max_txpwr = 0;
		uint8 chan = 0;
		rc = rc_map[in_idx++];
		nrc = rc_map[in_idx++];
		start = in_idx;
		endpos = start + nrc;

		memset(invalid_chan_bitmap, 0, sizeof(invalid_chan_bitmap));

		if ((input & WL_CHANSPEC_BAND_MASK) != (rc_map[start] & WL_CHANSPEC_BAND_MASK) ||
			!(bw_cap & blanket_chanspec_bw_to_wlc_bw(rc_map[start]))) {
			in_idx = endpos;
			BKT_DEBUG("%s rc %d isn't supported band or bandwidth\n",
					ifname, rc);
			continue;
		}

		/* Get regulated TX power chanspec list per band and bandwidth */
		rc_band_bw = rc_map[start] & (WL_CHANSPEC_BAND_MASK | WL_CHANSPEC_BW_MASK);

		if (kept_txpwr_band_bw != rc_band_bw) {
			ret = blanket_get_reg_txpwr_list(ifname, rc_band_bw,
				txpwr_list, BKT_TXPWR_BUF_LEN);
			BKT_ASSERT();
			kept_txpwr_band_bw = rc_band_bw;
			BKT_DEBUG("%s rc %d chanspec_band_bw 0x%04x count %d\n",
					ifname, rc, rc_band_bw, txpwr_list->count);
		}

		/* first pass see if all channels of this class are valid */
		for (; in_idx < endpos && in_idx < rc_max; in_idx++) {

			for (ch_idx = 0; ch_idx < (int32) list->count; ch_idx++) {
				/* Ignore operable chanspec */
				if (rc_map[in_idx] == list->element[ch_idx]) {
					valid++;
					/* Find the highest TX pwr */
					txpwr = blanket_get_reg_txpwr(rc_map[in_idx], txpwr_list);
					if (txpwr > max_txpwr) {
						max_txpwr = txpwr;
						BKT_DEBUG("%s rc %d txpwr %d\n",
							ifname, rc, max_txpwr);
					}
					break;
				}
			}

			if (ch_idx == (int32)list->count) {
				/* All list->elemnet traversed, rc_map's chanspec not
				 * found in list.
				 * Non operable chanspec candidate, extract center/control channel
				 * from chanspec and set bit(center/control channel) in
				 * invalid_chan_bitmap.
				 */
				if (CHSPEC_IS80(rc_map[in_idx]) || CHSPEC_IS160(rc_map[in_idx]) ||
					CHSPEC_IS320(rc_map[in_idx])) {
					chan = CHSPEC_CHANNEL(rc_map[in_idx]);
				} else {
					chan = wf_chspec_ctlchan(rc_map[in_idx]);
				}

				if (!isset(invalid_chan_bitmap, chan)) {
					setbit(invalid_chan_bitmap, chan);
				}
			}
		}

		if (valid == 0) { /* none in this class are valid */
			continue;
		}

		if (out_idx + 3 >= out_radio_caps->ListSize) {
			break;
		}

		out_radio_caps->List[0]++;
		out_radio_caps->List[out_idx++] = rc;
		out_radio_caps->List[out_idx++] = (int8)max_txpwr;
		/* Fill the channel count in the end */
		tmp_ptr_ch_count = &out_radio_caps->List[out_idx++];

		if (valid == nrc) { /* all in this class are valid */
			continue;
		}

		if (out_idx + (nrc - valid) >= out_radio_caps->ListSize) {
			break;
		}
		/* Add all set bit in invalid_chan_bit as non operable channels
		 * for particular opclass for radio
		 */
		for (k = 0; k < (sizeof(invalid_chan_bitmap) * 8); k++) {
			if (isset(invalid_chan_bitmap, k)) {
				out_radio_caps->List[out_idx++] = k;
				ch_count++;
			}
		}
		/* Fill the channel count */
		*tmp_ptr_ch_count = ch_count;

	} while (in_idx < (rc_max - 2) && out_idx < out_radio_caps->ListSize);

	out_radio_caps->Len = out_idx;
	out_radio_caps->Valid = TRUE;
end:

	/* Free buffer for getting List of operating class, in case of error */
	if (out_radio_caps->List && !out_radio_caps->Valid) {
		free(out_radio_caps->List);
		memset(out_radio_caps, 0, sizeof(*out_radio_caps));
	}

	if (buf) {
		free(buf);
		buf = NULL;
	}
	if (txpwr_list) {
		free(txpwr_list);
		txpwr_list = NULL;
	}
	if (list) {
		free(list);
		list = NULL;
	}
	BKT_EXIT();
	return ret;
}

/* Channel validity check aganist provided preference */
int
blanket_check_channel_validity(ieee1905_chan_pref_rc_map *rc_map, unsigned char rc_count,
	uint8 rclass, uint8 channel, uint8 min_pref)
{
	int i, j;
	int ret = TRUE;

	BKT_ENTER();
	if (!rc_map)
		goto end;

	BKT_DEBUG("Input: Regulatory Class:%d Channel:%d\n", rclass, channel);
	for (i = 0; i < rc_count; i++) {
		if (rc_map[i].regclass != rclass)
			continue;
		for (j = 0; j < rc_map[i].count; j++) {
			if (rc_map[i].channel[j] == channel) {
				break;
			}
		}
		if (rc_map[i].count == 0 || j < rc_map[i].count) {
			BKT_DEBUG("Preference Required:%d Current:%d\n",
				min_pref, rc_map[i].pref);
			if (rc_map[i].pref < min_pref) {
				ret = FALSE;
			}
			break;
		}
	}
end:
	BKT_EXIT();
	return ret;
}

/* Control channel validity check aganist provided preference */
int
blanket_check_control_channel_validity(uint8 band, uint8 ctrl_channel,
	ieee1905_chan_pref_rc_map *rc_map, unsigned char rc_count)
{
	int i, j, k;
	uint8 rclass5g_20mhz[] = {
		REGCLASS_5GL_20MHZ_1, REGCLASS_5GL_20MHZ_2,
		REGCLASS_5GH_20MHZ_1, REGCLASS_5GH_20MHZ_2};
	const i5_dm_rc_chan_map_type *rc_chan_map;
	unsigned int reg_class_count = 0;

	BKT_ENTER();

	/* Please note, the NULL rc_map case (all operating classes have preference 15 implicitly)
	 * is handled properly in the rest of this function. Do not check a NULL case simply here.
	 */

	rc_chan_map = i5DmGetRCChannelMap(&reg_class_count);

	if (band & BAND_6G) {
		BKT_DEBUG("band: 6G channel: %d\n", ctrl_channel);
		return blanket_check_channel_validity(rc_map, rc_count, REGCLASS_6G_20MHZ,
		ctrl_channel, IEEE1905_CHAN_PREF_1);
	}
	BKT_DEBUG("band: 5G(0x%x) channel: %d\n", band, ctrl_channel);

	for (i = 0; i < sizeof(rclass5g_20mhz); i++) {
		for (j = 0; j < reg_class_count; j++) {
			if (rclass5g_20mhz[i] != rc_chan_map[j].regclass) {
				continue;
			}
			for (k = 0; k < rc_chan_map[j].count; k++) {
				if (ctrl_channel == rc_chan_map[j].channel[k]) {
					BKT_DEBUG("Check validity: opclass %d control channel %d\n",
					rclass5g_20mhz[i], ctrl_channel);
					return blanket_check_channel_validity(rc_map, rc_count,
					rclass5g_20mhz[i], ctrl_channel,
					IEEE1905_CHAN_PREF_1);
				}
			}
		}
	}

	BKT_EXIT();
	return FALSE;
}

/* Check whether the rclass is 20MHz or not */
int
blanket_is_20MHz_opclass(uint8 rclass)
{
	const bcmwifi_rclass_info_t *rcinfo;
	int max_iter;

	FOREACH_RCLASS_INFO(BCMWIFI_RCLASS_TYPE_GBL, rcinfo, max_iter) {
		if (rcinfo->rclass != rclass) {
			continue;
		}
		if (rcinfo->bw == BCMWIFI_BW_20) {
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/* Update STATIC Non-operable Chanspec List of an interface */
int
blanket_update_static_nonoperable_chanspec_list(uint8 band, ieee1905_radio_caps_type
	*in_radio_caps, ieee1905_glist_t *list_excl)
{
	int ret = BKTE_OK, iter_rc = 0, iter_rcmap = 0, iter_chan = 0;
	int rcaps_rc_count = 0;
	uint8 rcaps_rclist[I5_MAX_INTF_RCS] = {0};
	uint8 *ptr = NULL;
	uint16 bytes_to_rd = 0, bytes_rd = 0;
	BKT_ENTER();

	BKT_ASSERT_ARG(list_excl, BKTE_INV_ARG);
	BKT_ASSERT_ARG(in_radio_caps, BKTE_INV_ARG);
	ptr = in_radio_caps->List;
	rcaps_rc_count = in_radio_caps->List ? in_radio_caps->List[0] : 0;
	BKT_ASSERT_ARG(rcaps_rc_count, BKTE_INV_ARG);

	if (rcaps_rc_count > I5_MAX_INTF_RCS) {
		BKT_ERROR("rclasses in radio capability[%d] is more than the number of rclasses "
			"possible in an interface[%d]. Considering only the [%d] rclasses and "
			"skipping the rest\n",
			rcaps_rc_count, I5_MAX_INTF_RCS, I5_MAX_INTF_RCS);
		/* Only processing the maximum regulatory classes expected in a band locked radio */
		rcaps_rc_count = I5_MAX_INTF_RCS;
	}

	/* skip first byte, as it contain number of opclass */
	ptr++;
	bytes_to_rd = in_radio_caps->Len;
	bytes_rd = 0;

	/* Reset Static Exclude List of this Radio */
	i5DmGlistCleanup(list_excl);
	ieee1905_glist_init(list_excl);

	/* [1] ----- For PRESENT regclasses ----- */
	/* Traverse Radio Capabilities, for each regclass PRESENT in it */
	for (iter_rc = 0; (iter_rc < rcaps_rc_count) && (bytes_rd < bytes_to_rd); iter_rc++) {

		radio_cap_sub_info_t *radio_sub_info = (radio_cap_sub_info_t *)ptr;

		/* Add regclass PRESENT in RadioCaps to a TEMP list */
		rcaps_rclist[iter_rc] = radio_sub_info->regclass;

		/* Extract Non-operable Chanspecs from rc_map, matching IFR's operating band */
		for (iter_chan = 0; iter_chan < radio_sub_info->n_chan; iter_chan++) {
			if (band & (ieee1905_get_band_from_channel(radio_sub_info->regclass,
				radio_sub_info->list_chan[iter_chan]))) {
				blanket_add_chspecs_fm_rc_map(radio_sub_info->regclass,
					radio_sub_info->list_chan[iter_chan], list_excl);
			}
		}
		/* Increament the Radio Caps Parsing pointers */
		ptr += sizeof(radio_cap_sub_info_t) + radio_sub_info->n_chan;
		bytes_rd += sizeof(radio_cap_sub_info_t) + radio_sub_info->n_chan;
	}

	/* [2] ----- For ABSENT regclasses ----- */
	/* Traverse rc_map, for each regclass in rc_map */
	for (iter_rcmap = 0; iter_rcmap < (rc_max - 1);) {

		int32 num_chspec = 0, curr_opclass = 0, rc_present = 0;
		curr_opclass = rc_map[iter_rcmap++];
		num_chspec = rc_map[iter_rcmap++];

		/* Find rc_map regclass in Radio Caps regclass PRESENT List */
		for (iter_rc = 0; iter_rc < rcaps_rc_count; iter_rc++) {
			if (rcaps_rclist[iter_rc] == curr_opclass) {
				rc_present = 1;
				break;
			}
		}
		if (rc_present) {
			iter_rcmap += num_chspec;
			continue; /* IF regclass PRESENT in Radio Caps, Skip this regclass */
		}

		/* Extract Non-operable Chanspecs from rc_map, matching IFR's operating band */
		while ((num_chspec > 0) && (iter_rcmap < rc_max)) {
			if (band & (ieee1905_get_band_from_channel(curr_opclass,
				CHSPEC_CHANNEL(rc_map[iter_rcmap])))) {
				blanket_add_chspec_to_list(list_excl, rc_map[iter_rcmap]);
			}
			num_chspec--;
			iter_rcmap++;
		}
	}

end:
	BKT_EXIT();
	return ret;
}

/* Update DYNAMIC Non-operable Chanspec List of an interface */
int
blanket_update_dynamic_nonoperable_chanspec_list(uint8 band, char *ifname,
	ieee1905_chan_pref_rc_map_array *in_chan_prefs, ieee1905_glist_t *list_excl)
{
	int i, ret = 0;
	wl_uint32_list_t *chspec_list = NULL;
	int chspec_list_sz;

	BKT_ENTER();

	BKT_ASSERT_ARG(ifname, BKTE_INV_ARG);
	BKT_ASSERT_ARG(list_excl, BKTE_INV_ARG);
	BKT_ASSERT_ARG(in_chan_prefs, BKTE_INV_ARG);

	/* Reset Dynamic Exclude List of this Radio */
	i5DmGlistCleanup(list_excl);
	ieee1905_glist_init(list_excl);

	chspec_list_sz = (MAXCHANNEL + 1) * sizeof(uint32);
	chspec_list = (wl_uint32_list_t *)bkt_malloc(chspec_list_sz, &ret);
	BKT_ASSERT_MSG("Failed to allocate memory for chanspecs\n");

	/* Get all supported chanspecs by radio */
	ret = blanket_get_chanspecs_list(ifname, chspec_list, chspec_list_sz);
	BKT_ASSERT_MSG("Failed getting chanspecs\n");

	/* Go over all chanspecs and check if they have non-operable preference */
	for (i = 0; i < chspec_list->count; i++) {
		chanspec_t chspec;
		uint8 op_class;
		int is_operable = FALSE;

		chspec = chspec_list->element[i];

		if (blanket_get_global_rclass(chspec, &op_class) != 0) {
			BKT_INFO("Chspec 0x%02x isn't in the global operating class\n", chspec);
			continue;
		}

		if (!BLANKET_CENTER_CH_OPCLASS_CHSPEC(chspec)) {
			/* If a chanspec belongs to operating classes with a primary channel,
			 * check whether a channel has an operable preference for a given
			 * preference map.
			 */
			is_operable = blanket_check_channel_validity(in_chan_prefs->rc_map,
				in_chan_prefs->rc_count, op_class,
				wf_chspec_ctlchan(chspec), IEEE1905_CHAN_PREF_1);
		} else {
			/* If a chanspec belongs to operating classes with a center channel, check
			 * whether both center and control channel have an operable preference.
			 */
			is_operable = blanket_check_channel_validity(in_chan_prefs->rc_map,
				in_chan_prefs->rc_count, op_class,
				CHSPEC_CHANNEL(chspec), IEEE1905_CHAN_PREF_1) &&
				blanket_check_control_channel_validity(band,
				wf_chspec_ctlchan(chspec), in_chan_prefs->rc_map,
				in_chan_prefs->rc_count);
		}

		if (!is_operable) {
			BKT_INFO("Non-operable chspec 0x%02x\n", chspec);
			blanket_add_chspec_to_list(list_excl, chspec);
		}
	}

end:
	if (chspec_list) {
		free(chspec_list);
	}

	BKT_EXIT();
	return ret;
}

/* Fill Channel Scan Capability list:
 * 1. Get the list of chanpecs supported by this radio based on bandwidth cap.
 * 2. Get the list of supported operating classes by this radio.
 * 3. Get the list of all chanspecs present in those operating classes.
 * 4. Take chanspecs which are Supported.
 */
int
blanket_get_channel_scan_cap(char *ifname, ieee1905_channel_scan_caps_type *out_chscan_caps)
{
	int32 ret = BKTE_OK, iter_chspec, out_idx, in_idx, iter_rcmap;
	int32 first_chspec, last_chspec, num_chspec;
	int32 curr_opclass;
	wl_uint32_list_t *chspec_list = NULL, *tmplist = NULL;
	char *buf = NULL;
	chanspec_t input = 0x0;
	uint8 bitmap[((sizeof(rc_map)/sizeof(rc_map[0])) + 7)/NBBY] = {0};
	uint8 *tmp_ptr_ch_count;
	char prefix[IFNAMSIZ];
	uint8 scantype_flags = 0x00, chscan_impact = 0x00;

	BKT_ENTER();

	memset(out_chscan_caps, 0, sizeof(*out_chscan_caps));

	/* Validate arg */
	BKT_ASSERT_ARG(out_chscan_caps, BKTE_INV_ARG);

	/* Get prefix of the interface from Driver */
	blanket_get_interface_prefix(ifname, prefix, sizeof(prefix));

	/* Get NVRAM setting for Channel Scan ScanType Flags */
	scantype_flags = blanket_get_config_val_uint(NULL,
		NVRAM_MAP_CHSCAN_SCANTYPE_FLAGS, DEF_MAP_CHSCAN_SCANTYPE_FLAGS);
	if (MAP_CHSCAN_ONLY_ONBOOT(scantype_flags)) {
		out_chscan_caps->chscan_cap_flag |= MAP_CHSCAN_CAP_ONBOOT_ONLY;
	}
	chscan_impact = blanket_get_config_val_int(prefix,
		NVRAM_MAP_CHSCAN_IMPACT, DEF_MAP_CHSCAN_IMPACT);

	/* If the "On boot only" bit to 1, the Scan Impact field shall be set to 0x00 */
	if (MAP_CHSCAN_ONLY_ONBOOT(scantype_flags)) {
		chscan_impact = 0x00;
	}

	if (chscan_impact) {
		out_chscan_caps->chscan_cap_flag |=
			(chscan_impact << MAP_CHSCAN_CAP_SCAN_IMPACT_SHIFT);
	}

	/* Get NVRAM setting for Minimum Scan Interval */
	out_chscan_caps->min_scan_interval = blanket_get_config_val_int(prefix,
		NVRAM_MAP_CHSCAN_MIN_SCAN_INT, DEF_MAP_CHSCAN_MIN_SCAN_INT);

	/* Allocate buffer for getting List of Operating Classes */
	out_chscan_caps->ListSize = BKT_MAX_BUF_512;
	out_chscan_caps->List = (unsigned char *)bkt_malloc(out_chscan_caps->ListSize, &ret);
	BKT_ASSERT();

	buf = (char *)bkt_malloc(WLC_IOCTL_MAXLEN, &ret);
	BKT_ASSERT();

	/* Get list of chanspec supported by driver */
	ret = wl_iovar_getbuf(ifname, "chanspecs", &input, sizeof(input), buf, WLC_IOCTL_MAXLEN);
	BKT_IOVAR_ASSERT(ifname, "chanspecs", ret);

	tmplist = (wl_uint32_list_t*)buf;
	tmplist->count = dtoh32(tmplist->count);

	chspec_list = (wl_uint32_list_t *)bkt_malloc(
		(sizeof(wl_uint32_list_t) * tmplist->count), &ret);
	BKT_ASSERT();

	/* Loop through all chanspec supported by driver,
	* choose chanspecs with 20MHz bw_cap only, fm list of chanspecs supported by driver
	*/
	for (iter_chspec = 0, in_idx = 0; iter_chspec < tmplist->count; iter_chspec++) {

		/* Pick chanspecs only with 20MHz bw_cap */
		if (CHSPEC_IS20(tmplist->element[iter_chspec])) {
			chspec_list->element[in_idx++] = tmplist->element[iter_chspec];
		}
	}
	chspec_list->count = in_idx;

	/* Start Preparing Out List. Reserve 0th element, Number of Operating Classes. Skip it. */
	out_idx = 1;

	/* Loop through all the rc_map Operation Class entries */
	iter_rcmap = 0;
	do {
		uint8 num_valid_chspec = 0, ch_count = 0;

		/* Get the next Operating class from rc_map */
		curr_opclass = rc_map[iter_rcmap++];

		/* Get the Number of Chanspecs present in Current Operating class fm rc_map */
		num_chspec = rc_map[iter_rcmap++];

		/* Get positions of First & Last Chanspecs in Current Operating class fm rc_map */
		first_chspec = iter_rcmap;
		last_chspec = first_chspec + num_chspec;

		/* First pass, Check if all channels in Current Operating class of rc_map
		* are present in the list of chanspecs supported by driver
		*/

		/* Loop through all channels in Current Operating class, also move rc_map ptr */
		for (; iter_rcmap < last_chspec && iter_rcmap < rc_max; iter_rcmap++) {

			/* Loop through list of channels supported by driver */
			for (iter_chspec = 0; iter_chspec < (int32) chspec_list->count;
				iter_chspec++) {

				/* Check if rc_map chanspec for Current Opclass is present */
				if (rc_map[iter_rcmap] == chspec_list->element[iter_chspec]) {

					/* Increament Num of Valid Chanspec count & set bitmap */
					num_valid_chspec++;
					setbit(bitmap, iter_rcmap);

					/* Go for next Chanspec in Current Opclass of rc_map */
					break;
				}
			}
		}

		/* If Zero Chanspecs in Current Operating class are present,
		* Go for next Operating class in rc_map
		*/
		if (num_valid_chspec == 0) {
			continue;
		}

		/* Check : Out List current index + Operating Class Octet +
		 * Number of Channels Octet is greater than or equal to Total Out List Size
		 */
		if (out_idx + 2 >= out_chscan_caps->ListSize) {
			break;
		}

		/* Increament Number of Operating Classes : value of 0th element */
		out_chscan_caps->List[0]++;

		/* Store Currnet Operating class Octet in Out List */
		out_chscan_caps->List[out_idx] = curr_opclass;
		out_idx++;

		/* Store Number of Channels Octet in Out List, not now, in the end */
		tmp_ptr_ch_count = &out_chscan_caps->List[out_idx];
		*tmp_ptr_ch_count = 0;
		out_idx++;

		/* If All Chanspecs in Current Operating class are present, Number of Channels
		* Octet in Out List, should be = 0, and Channel List should be empty
		*/
		if (num_valid_chspec == num_chspec) {
			continue;
		}

		/* Check : Out List current index + Number of Valid Chanspecs
		 * is greater than or equal to Total Out List Size
		 */
		if (out_idx + num_valid_chspec >= out_chscan_caps->ListSize) {
			break;
		}

		/* Second pass, since not all Chanspecs of Current Operating class of rc_map
		* are valid, Store the Valid Chanspecs to Out List
		*/

		/* Loop through all channels in Current Operating class */
		for (iter_chspec = first_chspec;
			iter_chspec < last_chspec && iter_chspec < rc_max; iter_chspec++) {

			/* If Chanspec is Valid & only with 20MHz bw_cap, Store it to Out List */
			if (isset(bitmap, iter_chspec) && CHSPEC_IS20(rc_map[iter_chspec])) {

				/* For Valid Chanspecs, Store Control Channel to Out List */
				out_chscan_caps->List[out_idx] =
					wf_chspec_ctlchan(rc_map[iter_chspec]);

				out_idx++;
				ch_count++;
			}
		}

		/* Fill the Out List Final Channel Count for this Operating Class */
		*tmp_ptr_ch_count = ch_count;

	} while (iter_rcmap < (rc_max - 2) && out_idx < out_chscan_caps->ListSize);

	out_chscan_caps->Len = out_idx;
	out_chscan_caps->Valid = TRUE;
end:

	/* Free buffer for getting List of operating class, in case of error */
	if (out_chscan_caps->List && !out_chscan_caps->Valid) {
		free(out_chscan_caps->List);
		memset(out_chscan_caps, 0, sizeof(*out_chscan_caps));
	}
	if (chspec_list) {
		free(chspec_list);
		chspec_list = NULL;
	}
	if (buf) {
		free(buf);
		buf = NULL;
	}
	BKT_EXIT();
	return ret;
}

/* To get the phytype of slave */
int
blanket_get_phy_type(char *ifname, wl_bss_info_t *in_bi, uint8 *out_phy_type)
{
	int ret = BKTE_OK;
	wl_bss_info_v109_1_t *bi;

	/* Validate arg */
	BKT_ASSERT_ARG(out_phy_type, BKTE_INV_ARG);

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = blanket_get_bss_info(ifname, (wl_bss_info_t **)&bi);
		BKT_ASSERT();
	} else {
		bi = (wl_bss_info_v109_1_t *)in_bi;
	}

	if (bi->he_cap) {
		*out_phy_type = (uint8) DOT11_PHYTYPE_HE;
	} else if (bi->vht_cap) {
		*out_phy_type = (uint8) DOT11_PHYTYPE_VHT;
	} else if (bi->n_cap) {
		*out_phy_type = (uint8) DOT11_PHYTYPE_HT;
	} else {
		*out_phy_type = 0;
	}

	BKT_DEBUG("out_phy_type = %x\n", *out_phy_type);

end:
	return ret;
}

/* To get the FBT Enabled value in IOVAR */
int
blanket_get_fbt(char *ifname)
{
	int ret = BKTE_OK, fbt = 0;

	ret = wl_iovar_get(ifname, "fbt", &fbt, sizeof(fbt));
	BKT_IOVAR_ASSERT(ifname, "fbt", ret);

	BKT_DEBUG("FBT on %s:%d\n", ifname, fbt);
	return fbt;
end:
	return 0;
}

/* To get the bssid_info of slave */
int
blanket_get_bssid_info_field(char *ifname, wl_bss_info_t *in_bi, uint32 *out_bssid_info)
{
	int ret = BKTE_OK;
	wl_bss_info_t *bi;
	wl_bss_info_v109_1_t *he_bi;	/* For HE feature */
	int wme, wme_apsd;

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
	/* HE capability is in wl_bss_info_v109_1_t structure */
	he_bi = (wl_bss_info_v109_1_t*)bi;

	/* Get QoS and APSD */
	blanket_get_wme(ifname, &wme);
	blanket_get_wme_apsd(ifname, &wme_apsd);
	bi->capability = dtoh16(bi->capability);

	*out_bssid_info =  DOT11_NGBR_BI_REACHABILTY;
	*out_bssid_info |= DOT11_NGBR_BI_SEC;
	*out_bssid_info |= DOT11_NGBR_BI_KEY_SCOPE;
	*out_bssid_info |= wme ? DOT11_NGBR_BI_CAP_QOS : 0;
	*out_bssid_info |= wme_apsd ? DOT11_NGBR_BI_CAP_APSD : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_RRM) ? DOT11_NGBR_BI_CAP_RDIO_MSMT : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_DELAY_BA) ? DOT11_NGBR_BI_CAP_DEL_BA : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_IMMEDIATE_BA) ? DOT11_NGBR_BI_CAP_IMM_BA : 0;
	*out_bssid_info |= (bi->n_cap) ? DOT11_NGBR_BI_HT : 0;
	*out_bssid_info |= (bi->vht_cap) ? DOT11_NGBR_BI_VHT : 0;
	*out_bssid_info |= (he_bi->he_cap) ? DOT11_NGBR_BI_HE : 0;

	if (blanket_get_fbt(ifname)) {
		*out_bssid_info |= DOT11_NGBR_BI_MOBILITY;
	}

	BKT_DEBUG("out_bssid_info = %x\n", *out_bssid_info);

end:
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

	/* Round off pwr_percent to multiple of 10 */
	pwr_percent = (((pwr_percent + 5) / 10) *10);
	BKT_INFO("ifname: %s Setting txpwr_percent = %d\n", ifname, pwr_percent);

	pwr_percent = htod32(pwr_percent);
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
		counters->txmcastframe = dtoh32(macstat_wlc->txmulti);
		counters->rxmcastframe = dtoh32(macstat_wlc->rxmulti);
		counters->txbcastframe = dtoh32(macstat_wlc->txbcast);
		counters->rxbcastframe = dtoh32(macstat_wlc->rxbcast);
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

/* Get Primary Interface(radio) prefix wlX_, from OS specific MBSS(Virtual) interface name
 * based on NVRAM that means without calling driver
 */
int
blanket_get_radio_prefix_from_nvram(char* ifname, char *radio_prefix, int prefix_len)
{
	int unit = -1;
	int ret = BKTE_OK;
	char nvram_name[BKT_MAX_BUF_16] = {0};
	BKT_ENTER();

	/* get the nvram name of the interface */
	if ((osifname_to_nvifname(ifname, nvram_name, sizeof(nvram_name))) != 0) {
		goto end;
	}

	if (get_ifname_unit(nvram_name, &unit, NULL) < 0) {
		ret = BKTE_INV_IFNAME;
		BKT_WARNING("ifname %s nvname %s Failed to get main index\n", ifname, nvram_name);
		goto end;
	}

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
	bkt_cmp_option_t cmp_option)
{
	char *nvram_value = NULL;
	int mismatch = 0;

	/* Get NVRAM Value */
	nvram_value = blanket_nvram_prefix_safe_get(prefix, nvram);

	/* Compare NVRAM and New value with/without case */
	if (cmp_option == BKT_CMP_LIST_WITH_CASE) {
		mismatch = compare_lists(nvram_value, new_value);
	} else if (cmp_option == BKT_CMP_STR_WITH_CASE) {
		mismatch = strcmp(nvram_value, new_value);
	} else {
		mismatch = strcasecmp(nvram_value, new_value);
	}

	/* And if mismatch, set New value in NVRAM */
	if (mismatch) {

		BKT_DEBUG("Prefix[%s] NVRAM[%s] NVRAMVal[%s] != NewVal[%s]."
			" Needs rc restart\n", prefix ? prefix : "", nvram, nvram_value, new_value);

		/* Set New Value in NVRAM */
		blanket_nvram_prefix_set(prefix, nvram, new_value);

		/* Value is Overidden, Return 1 */
		return 1;
	}
	return 0;
}

/* Gets the string config val from NVARM, if not found applies the default value */
void
blanket_get_config_val_str(char* prefix, const char *nvram, char *def, char **val)
{
	BKT_ENTER();
	char *str = NULL;
	char final_nvram[BKT_MAX_BUF_256];

	snprintf(final_nvram, sizeof(final_nvram), "%s%s", prefix ? prefix : "", nvram);
	final_nvram[sizeof(final_nvram) - 1] = '\0';
	str = blanket_nvram_safe_get(final_nvram);

	if (str && (str[0] != '\0')) {
		*val = str;
	} else {
		BKT_DEBUG("NVRAM is not defined: %s%s \n", (prefix ? prefix : ""), nvram);
		*val = def;
	}

	blanket_log_default_nvram("%s=%s\n", final_nvram, *val);

	BKT_EXIT();
	return;
}

/* Gets the unsigned integer config val from NVARM, if not found applies the default value */
uint32
blanket_get_config_val_uint(char* prefix, const char *nvram, uint32 def)
{
	char *val = NULL;
	uint32 ret = def;
	char  final_nvram[BKT_MAX_BUF_256];
	BKT_ENTER();

	snprintf(final_nvram, sizeof(final_nvram), "%s%s", prefix ? prefix : "", nvram);
	final_nvram[sizeof(final_nvram) - 1] = '\0';
	val = blanket_nvram_safe_get(final_nvram);

	if (val && (val[0] != '\0')) {
		ret = (uint32)strtoul(val, NULL, 0);
	} else {
		BKT_DEBUG("NVRAM is not defined: %s%s \n", (prefix ? prefix : ""), nvram);
	}

	blanket_log_default_nvram("%s=0x%x\n", final_nvram, ret);

	BKT_EXIT();
	return ret;
}

/* Gets the integer config val from NVARM, if not found applies the default value */
int
blanket_get_config_val_int(char* prefix, const char *nvram, int def)
{
	char *val = NULL;
	int ret = def;
	char final_nvram[BKT_MAX_BUF_256];
	BKT_ENTER();

	snprintf(final_nvram, sizeof(final_nvram), "%s%s", prefix ? prefix : "", nvram);
	final_nvram[sizeof(final_nvram) - 1] = '\0';
	val = blanket_nvram_safe_get(final_nvram);

	if (val && (val[0] != '\0')) {
		ret = (int)strtol(val, NULL, 0);
	} else {
		BKT_DEBUG("NVRAM is not defined: %s%s \n", (prefix ? prefix : ""), nvram);
	}

	blanket_log_default_nvram("%s=%d\n", final_nvram, ret);

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

/* Calculate Off-Channel Channel Utilization Using Chanim Stats cca values */
unsigned char
blanket_calc_chanutil_offchan(char *ifname, uint8 *ccastats)
{
	int ret = BKTE_OK, busy = 0;
	unsigned char out_chanutil = 0;
	BKT_ENTER();
	BCM_REFERENCE(ret);

	/* Validate arg */
	BKT_ASSERT_ARG(ifname, BKTE_INV_ARG);
	BKT_ASSERT_ARG(ccastats, BKTE_INV_ARG);

	/* Calculate Busy : 100 - TxOp */
	busy = 100 - ccastats[CCASTATS_TXOP];

	/* Calculate Chan Util Fm Busy */
	out_chanutil = (unsigned char)((busy * 255) / 100);
	BKT_DEBUG("Ifname[%s] Busy[%d] Chan_Util[%d]\n", ifname, busy, out_chanutil);
end:
	BKT_EXIT();
	return out_chanutil;
}

/* Calculate Channel Utilization Using Chanim Stats cca values */
unsigned char
blanket_calc_chanutil(char *ifname, bool cca0_txdur_needed, uint8 *ccastats)
{
	int ret = BKTE_OK, busy = 0;
	unsigned char out_chanutil = 0;
	BKT_ENTER();
	BCM_REFERENCE(ret);

	/* Validate arg */
	BKT_ASSERT_ARG(ifname, BKTE_INV_ARG);
	BKT_ASSERT_ARG(ccastats, BKTE_INV_ARG);

	/* Calculate Busy :  INBSS + OBSS + NOCTG + NOPKT + GOODTXDUR + BADTXDUR */
	busy = ccastats[CCASTATS_INBSS] + ccastats[CCASTATS_OBSS] +
		ccastats[CCASTATS_NOCTG] + ccastats[CCASTATS_NOPKT] +
		ccastats[CCASTATS_GDTXDUR] + ccastats[CCASTATS_BDTXDUR];

	/* Check If Tx Duration needs to be added to Busy */
	if (cca0_txdur_needed) {
		busy += ccastats[CCASTATS_TXDUR];
	}

	/* Calculate Chan Util Fm Busy */
	out_chanutil = (unsigned char)((busy * 255) / 100);
	BKT_DEBUG("Ifname[%s] Busy[%d] Chan_Util[%d]\n", ifname, busy, out_chanutil);
end:
	BKT_EXIT();
	return out_chanutil;
}

/* Get Chaim Stats */
int
blanket_get_chanim_stats(char* ifname, uint32 count, char* data_buf, int buflen)
{
	int ret = 0;
	wl_chanim_stats_t *list;
	wl_chanim_stats_t param = {0, WL_CHANIM_STATS_VERSION, 0, {{0}}};
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
	struct maclist* list = NULL;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(assoc_count, BKTE_INV_ARG);

	list = (struct maclist*)bkt_malloc(WLC_IOCTL_MAXLEN, &ret);
	BKT_ASSERT();

	/* Get assoclist on this interface */
	ret = blanket_get_assoclist(ifname, list, WLC_IOCTL_MAXLEN);
	BKT_ASSERT();

	/* Get count of associated sta */
	*assoc_count = list->count;

end:
	if (list) {
		free(list);
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
	ampdu_tid_ba_wsize_t tid_ba_wsize;
	char *ioctl_buf = NULL;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(ba_wsize, BKTE_INV_ARG);

	wl_endian_probe(ifname);

	memset(&tid_ba_wsize, 0, sizeof(tid_ba_wsize));
	tid_ba_wsize.tid = (int8)-1;

	ioctl_buf = (char *)bkt_malloc(WLC_IOCTL_SMLEN, &ret);
	BKT_ASSERT();

	ret = wl_iovar_getbuf(ifname, "ampdu_ba_wsize", &tid_ba_wsize, sizeof(tid_ba_wsize),
		ioctl_buf, WLC_IOCTL_SMLEN);
	BKT_IOVAR_ASSERT(ifname, "ampdu_ba_wsize", ret);

	memcpy(&tid_ba_wsize, ioctl_buf, sizeof(tid_ba_wsize));
	*ba_wsize = dtoh16(tid_ba_wsize.wsize);

end:
	if (ioctl_buf) {
		free(ioctl_buf);
	}

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
	{ 8, 5, 6 }, /* MCS  9: MOD: 256QAM, CR 5/6 */
	{ 10, 3, 4 }, /* MCS  10: MOD: 1024QAM, CR 3/4 */
	{ 10, 5, 6 }, /* MCS  11: MOD: 1024QAM, CR 5/6 */
};

static uint
wl_mcs2rate(uint mcs, uint nss, uint bw, int sgi)
{
	const int ksps = 250; /* kilo symbols per sec, 4 us sym */
	const int Nsd_20MHz = 52;
	const int Nsd_40MHz = 108;
	const int Nsd_80MHz = 234;
	const int Nsd_160MHz = 468;
	const int Nsd_320MHz = 936;
	uint rate;

	if (mcs == 32) {
		/* just return fixed values for mcs32 instead of trying to parametrize */
		rate = (sgi == 0) ? 6000 : 6700;
	} else if (mcs < ARRAY_SIZE(wlu_mcs_info)) {
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
		} else if (bw == 320) {
			rate = Nsd_320MHz;
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

static void
blanket_scb_bs_data_convert_v2(iov_bs_data_struct_t *v2)
{
	/* This only take care of endianess between driver and application */
	int argn;
	for (argn = 0; argn < v2->structure_count; ++argn) {
		iov_bs_data_record_t *rec;
		iov_bs_data_counters_t *ctr;

		rec = &v2->structure_record[argn];
		ctr = &rec->station_counters;

		rec->station_flags = dtoh16(rec->station_flags);

#define DEVICE_TO_HOST64(xyzzy) ctr->xyzzy = dtoh64(ctr->xyzzy)
#define DEVICE_TO_HOST32(xyzzy) ctr->xyzzy = dtoh32(ctr->xyzzy)
		DEVICE_TO_HOST64(throughput);
		DEVICE_TO_HOST64(txrate_main);
		DEVICE_TO_HOST64(txrate_succ);
		DEVICE_TO_HOST32(retry_drop);
		DEVICE_TO_HOST32(rtsfail);
		DEVICE_TO_HOST32(retry);
		DEVICE_TO_HOST32(acked);
		DEVICE_TO_HOST32(ru_acked);
		DEVICE_TO_HOST32(mu_acked);
		DEVICE_TO_HOST32(time_delta);
		DEVICE_TO_HOST32(airtime);
		DEVICE_TO_HOST32(txbw);
		DEVICE_TO_HOST32(txnss);
		DEVICE_TO_HOST32(txmcs);
#undef DEVICE_TO_HOST64
#undef DEVICE_TO_HOST32
	}
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
	int max_count = 0;
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

	/* Calculating the maximum number of stations that data can hold  */
	max_count = (BKT_MAX_BUF_4096 - OFFSETOF(iov_bs_data_struct_t, structure_record)) /
		sizeof(iov_bs_data_record_t);
	if (data->structure_count > max_count) {
		BKT_WARNING("Ifname[%s] No Data or count exceed %d \n", ifname,
			data->structure_count);
		goto end;
	}

	if (data->structure_version == SCB_BS_DATA_STRUCT_VERSION) {
		blanket_scb_bs_data_convert_v2(data);
	} else {
		BKT_DEBUG("blanket / wl driver mismatch, expect V%d format, got %d.\n",
			SCB_BS_DATA_STRUCT_VERSION, data->structure_version);
		goto end;
	}

	for (argn = 0; argn < data->structure_count; ++argn) {
		rec = &data->structure_record[argn];
		ctr = &rec->station_counters;

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

/* Disable(Bring down) or enable(Bring up) BSS */
static int
blanket_enable_or_disable_bss(char *ifname, int bsscfg_idx, int enable)
{
	int ret = BKTE_OK;
	struct {int bsscfg_idx; int enable;} setbuf;

	setbuf.bsscfg_idx = bsscfg_idx;
	setbuf.enable = enable;

	ret = wl_iovar_set(ifname, "bss", &setbuf, sizeof(setbuf));
	BKT_IOVAR_ASSERT(ifname, "bss", ret);

	BKT_INFO("ifname : %s BSS %d %s\n", ifname, bsscfg_idx, enable ? "Enabled" : "Disabled");

end:
	return ret;
}

/* Enable(Bring up) the BSS */
int
blanket_bss_enable(char *ifname, int bsscfg_idx)
{
	return blanket_enable_or_disable_bss(ifname, bsscfg_idx, 1);
}

/* Disable(Bring down) the BSS */
int
blanket_bss_disable(char *ifname, int bsscfg_idx)
{
	return blanket_enable_or_disable_bss(ifname, bsscfg_idx, 0);
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

	BKT_INFO("wl -i %s keep_ap_up %d\n", ifname, up);
end:
	BKT_EXIT();
	return ret;
}

/* Enable/diable raoming of STA interface */
int blanket_set_cfg80211_roam(char* ifname, int enable)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	ret = wl_iovar_setint(ifname, "cfg80211_roam", htod32(enable));
	BKT_IOVAR_ASSERT(ifname, "set cfg80211_roam", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Set roam trigger(RSSI) */
int blanket_set_roam_trigger(char* ifname, int rssi_thld)
{
	int ret = BKTE_OK;
	struct {
		int val;
		int band;
	} roam;
	BKT_ENTER();

	roam.val = htod32(rssi_thld);
	roam.band = htod32(WLC_BAND_ALL);

	/* Set roam trigger */
	ret = wl_ioctl(ifname, WLC_SET_ROAM_TRIGGER, &roam, sizeof(roam));
	BKT_IOVAR_ASSERT(ifname, "WLC_SET_ROAM_TRIGGER", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Set roam delta */
int
blanket_set_roam_delta(char* ifname, int delta)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	delta = htod32(delta);
	ret = wl_ioctl(ifname, WLC_SET_ROAM_DELTA, &delta, sizeof(delta));
	BKT_IOVAR_ASSERT(ifname, "WLC_SET_ROAM_DELTA", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Get Beacon Period */
int blanket_get_beacon_period(char *ifname, uint16 *out_beacon_period)
{
	int ret = BKTE_OK;
	int beacon_period;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_beacon_period, BKTE_INV_ARG);

	ret = wl_ioctl(ifname, WLC_GET_BCNPRD, &beacon_period, sizeof(beacon_period));
	BKT_IOVAR_ASSERT(ifname, "Get Beacon Period", ret);

	*out_beacon_period = dtoh16(beacon_period);

	BKT_DEBUG("ifname : %s Beacon Period : %d\n", ifname, *out_beacon_period);
end:
	return ret;
}
/* Get revinfo of the interface */
int
blanket_get_revinfo(char *ifname, wlc_rev_info_t *out_rev)
{
	return wl_ioctl(ifname, WLC_GET_REVINFO, out_rev, sizeof(*out_rev));
}

#define BKT_CS_SCAN_DWELL_LT128		110 /* ms */
#define BKT_CS_SCAN_DWELL_ACTIVE_LT128 50 /* ms */

/* Set default scan home time */
int
blanket_set_scan_home_time(char *ifname, int scan_home_time)
{
	int ret = BKTE_OK;

	scan_home_time = htod32(scan_home_time);
	ret = wl_ioctl(ifname, WLC_SET_SCAN_HOME_TIME, &scan_home_time,
			sizeof(scan_home_time));
	BKT_IOVAR_ASSERT(ifname, "scan_home_time", ret);
end:
	return ret;
}

/* Prepare scan params from channel scan request */
int
blanket_escan_prep_cs(char *ifname, ieee1905_per_radio_opclass_list *ifr,
	void *params, int *params_size, uint16 version)
{
	int ret = -1, idx_chan;
	int scount = 0;
	wlc_rev_info_t rev = {0};
	ieee1905_per_opclass_chan_list *opcls = NULL;
	wl_scan_params_v2_t *params_v2 = NULL;
	char *str;
	int num = 0;

	wl_scan_params_v1_t *params_v1 = (wl_scan_params_v1_t*)params;
	if (version == WL_SCAN_VERSION_MAJOR_V2) {
		params_v2 = (wl_scan_params_v2_t*) params;
	}
	BKT_ENTER();

	memcpy(&params_v1->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params_v1->bss_type = DOT11_BSSTYPE_ANY;
	params_v1->nprobes = -1;

	/* Get NVRAM : channel scan parameter */
	str = blanket_nvram_prefix_safe_get(NULL, NVRAM_MAP_CHSCAN_PARAM_CONFIG);
	if (str[0] != '\0') {
		int scan_type, active_time, passive_time;
		num = sscanf(str, "%d %d %d", &scan_type, &active_time, &passive_time);
		params_v1->scan_type = (int8)scan_type; /* 0: ACTIVE SCAN; */
		params_v1->active_time = (int32)active_time;
		params_v1->passive_time = (int32)passive_time;
	}
	if (num != 3) {
		params_v1->scan_type = 0;	/* ACTIVE SCAN; */
		params_v1->active_time = -1;	/* Use firmware default value */
		params_v1->passive_time = -1;	/* Use firmware default value */
	}
	params_v1->home_time = -1;
	params_v1->channel_num = 0;
	params_v1->nprobes = htod32(params_v1->nprobes);

	blanket_get_revinfo((char *)ifname, &rev);
	if (rev.corerev < 128) {
		params_v1->active_time = BKT_CS_SCAN_DWELL_ACTIVE_LT128;
		params_v1->passive_time = BKT_CS_SCAN_DWELL_LT128;
	}
	BKT_DEBUG("ifname = %s corerev = %d scan  active time  = %d, passive time = %d,\n",
		ifname, rev.corerev, params_v1->active_time, params_v1->passive_time);
	params_v1->active_time = htod32(params_v1->active_time);
	params_v1->passive_time = htod32(params_v1->passive_time);
	params_v1->home_time = htod32(params_v1->home_time);

	/* Extract details for each Operating Class */
	foreach_iglist_item(opcls, ieee1905_per_opclass_chan_list, ifr->opclass_list) {

		if (!opcls->supported) {
			continue;
		}

		/* Extract Channel List */
		for (idx_chan = 0; idx_chan < opcls->num_of_channels; idx_chan++) {
			uint16 chanspec;

		/* Drop Channels which are not supported
		 * or for which { FRESH } or { ONBOOT } Scan is not Required
		 */
			if (!isset(&opcls->supported_chan_list, idx_chan) ||
				!isset(&opcls->scan_required, idx_chan)) {
				continue;
			}
			chanspec = wf_channel2chspec(opcls->chan_list[idx_chan],
				WL_CHANSPEC_BW_20, blanket_opclass_to_band(opcls->opclass_val));
			BKT_DEBUG("opclass[%d] Tot_chans[%d] supported[%s] "
				"scan_req[%s] chan[%d]=[%d] Chanspec = [%x]\n",
				opcls->opclass_val, opcls->num_of_channels,
				isset(&opcls->supported_chan_list, idx_chan) ? "Y" : "N",
				isset(&opcls->scan_required, idx_chan) ? "Y" : "N",
				idx_chan, opcls->chan_list[idx_chan], chanspec);

			if (version == WL_SCAN_VERSION_MAJOR_V2) {
				params_v2->channel_list[scount++] = htodchanspec(chanspec);
				params_v2->scan_type_2 = htod32(WLC_SCANFLAGS_FULL >> 17);
			} else {
				params_v1->channel_list[scount++] = htodchanspec(chanspec);
			}

			/* Atleast One Channel is Set */
			ret = 0;
		}
	}

	params_v1->channel_num = htod32(scount & WL_SCAN_PARAMS_COUNT_MASK);
	if (version == WL_SCAN_VERSION_MAJOR_V2) {
		*params_size = WL_SCAN_PARAMS_FIXED_SIZE_V2 + scount * sizeof(uint16);
	} else {
		*params_size = WL_SCAN_PARAMS_FIXED_SIZE_V1 + scount * sizeof(uint16);
	}
	BKT_DEBUG("escan_channel number: %d params size: %d\n", params_v1->channel_num,
		*params_size);

	BKT_EXIT();
	return ret;
}

/* Get the scan version */
int
blanket_get_escan_ver(char *ifname, wl_scan_version_t *ver)
{
	return wl_iovar_get(ifname, "scan_ver", ver, sizeof(*ver));
}

/* Start the escan */
int
blanket_escan_start(char *ifname, void *params, int params_size)
{
	char buf[BKT_MAX_BUF_512];
	return wl_iovar_setbuf(ifname, "escan", params, params_size, buf, sizeof(buf));
}

/* Abort scan */
int
blanket_escan_abort(char *ifname)
{
	return wl_iovar_set(ifname, "scanabort", NULL, 0);
}

/* get driver capability */
int
blanket_get_driver_capability(char *ifname, char *pbuf, uint16 buflen)
{
	int ret = BKTE_OK;

	BKT_ENTER();

	ret = wl_iovar_get(ifname, "cap", pbuf, buflen);
	BKT_IOVAR_ASSERT(ifname, "cap", ret);
end:
	BKT_EXIT();
	return ret;
}

/* is_edcrs_eu */
int
blanket_get_is_edcrs_eu(char *ifname, uint32 *is_edcrs_eu)
{
	int ret = BKTE_OK;

	BKT_ENTER();

	ret = wl_iovar_get(ifname, "is_edcrs_eu", is_edcrs_eu, sizeof(*is_edcrs_eu));
	BKT_IOVAR_ASSERT(ifname, "is_edcrs_eu", ret);
end:
	BKT_EXIT();
	return ret;
}

/* Get the backhaul STA MAC address for the WDS interface name using IOVAR WLC_GET_WDSLIST */
static int
blanket_get_bh_sta_mac_from_wdslist(char *ifname, char *wds_ifname, unsigned char *mac)
{
	int ret = BKTE_OK, i;
	char buf[BKT_MAX_BUF_256];
	struct wds_maclist *wds_maclist = (struct wds_maclist *) buf;
	uint max = (sizeof(buf) - (sizeof(wds_maclist_t) - sizeof(wds_client_info_t)))
		/ sizeof(wds_client_info_t);

	wds_maclist->count = htod32(max);
	ret = wl_ioctl(ifname, WLC_GET_WDSLIST, wds_maclist, sizeof(buf));
	BKT_IOVAR_ASSERT(ifname, "WLC_GET_WDSLIST", ret);
	wds_maclist->count = dtoh32(wds_maclist->count);

	BKT_DEBUG("WLC_GET_WDSLIST count %d\n", wds_maclist->count);
	for (i = 0; i < wds_maclist->count && i < max; i++) {
		if (strcmp(wds_ifname, wds_maclist->client_list[i].ifname) == 0) {
			memcpy(mac, &wds_maclist->client_list[i].ea, ETHER_ADDR_LEN);
			BKT_DEBUG("WLC_GET_WDSLIST count %d MAC["MACF"] Ifname %s\n",
				wds_maclist->count, ETHER_TO_MACF(wds_maclist->client_list[i].ea),
				wds_maclist->client_list[i].ifname);
			goto end;
		}
	}

	ret = BKTE_INV_ARG;
end:
	return ret;
}

/* Get the backhaul STA MAC address for the WDS interface name using IOVAR WLC_GET_DWDSLIST */
static int
blanket_get_bh_sta_mac_from_dwdslist(char *ifname, char *wds_ifname, unsigned char *mac)
{
	int ret = BKTE_OK, i;
	char buf[BKT_MAX_BUF_256];
	struct dwds_maclist *dwds_maclist = (struct dwds_maclist*)buf;
	struct wds_client_info *client_info;
	uint16 data_offset;

	ret = wl_ioctl(ifname, WLC_GET_DWDSLIST, dwds_maclist, sizeof(buf));
	BKT_IOVAR_ASSERT(ifname, "WLC_GET_DWDSLIST", ret);

	dwds_maclist->count = dtoh16(dwds_maclist->count);
	data_offset = dtoh16(dwds_maclist->data_offset);
	client_info = (struct wds_client_info *)((uint8 *)dwds_maclist + data_offset);
	BKT_DEBUG("WLC_GET_DWDSLIST count %d\n", dwds_maclist->count);
	for (i = 0; i < dwds_maclist->count; i++) {
		if (strcmp(wds_ifname, client_info[i].ifname) == 0) {
			memcpy(mac, &client_info[i].ea, ETHER_ADDR_LEN);
			BKT_DEBUG("WLC_GET_DWDSLIST count %d MAC["MACF"] Ifname %s\n",
				dwds_maclist->count, ETHER_TO_MACF(client_info[i].ea),
				client_info[i].ifname);
			goto end;
		}
	}

	ret = BKTE_INV_ARG;
end:
	return ret;
}

/* Get the backhaul STA MAC address for the WDS interface name */
int
blanket_get_bh_sta_mac_from_wds(char *ifname, char *wds_ifname, unsigned char *mac)
{
	int ret = BKTE_OK;

	ret = blanket_get_bh_sta_mac_from_dwdslist(ifname, wds_ifname, mac);
	if (ret != BKTE_OK) {
		BKT_DEBUG("Failed to get BH STA MAC for WDS ifname using DWDS list. Get it from "
			"WDS list\n");
		ret = blanket_get_bh_sta_mac_from_wdslist(ifname, wds_ifname, mac);
	}

	return ret;
}

/* Get the Channel Utilization & Station Count fields Present in QBSS LOAD IE */
int blanket_get_qbss_load_element(wl_bss_info_t *in_bi,
	unsigned char *chan_util, unsigned short *sta_count)
{
	int ret = BKTE_OK;
	uint8 *tlvs = NULL;
	uint16 tlvs_len = 0;
	dot11_qbss_load_ie_t *qbss_load_ie = NULL;
	BKT_ENTER();

	BKT_ASSERT_ARG(in_bi, BKTE_INV_ARG);
	BKT_ASSERT_ARG(chan_util, BKTE_INV_ARG);
	BKT_ASSERT_ARG(sta_count, BKTE_INV_ARG);

	tlvs = (uint8 *)(((uint8 *)in_bi) + (in_bi->ie_offset));
	tlvs_len = in_bi->ie_length;

	qbss_load_ie = (dot11_qbss_load_ie_t *)
		bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_QBSS_LOAD_ID);

	if (qbss_load_ie == NULL ||
		qbss_load_ie->length != (sizeof(*qbss_load_ie) - TLV_HDR_LEN)) {
		BKT_DEBUG("NBR_BSSID["MACF"] doesn't support QBSS Load IE.\n",
			ETHERP_TO_MACF(in_bi->BSSID.octet));
		return BKTE_WL_ERROR;
	}

	*chan_util = (unsigned char)qbss_load_ie->channel_utilization;
	*sta_count = (unsigned short)qbss_load_ie->station_count;
	BKT_DEBUG("NBR_BSSID["MACF"] chan_util[%d] sta_count[%d] Read from QBSS Load IE.\n",
		ETHERP_TO_MACF(in_bi->BSSID.octet), *chan_util, *sta_count);

end:
	BKT_EXIT();
	return ret;
}

/* set or unset Association Disallowed attribute in the Beacon.
 * Possible values for reason
 * 0 - Allow association
 * 1 - Reason unspecified
 * 2 - Max sta limit reached
 * 3 - Air interface overload
 * 4 - Auth server overload
 * 5 - Insufficient RSSI
 */
int
blanket_mbo_assoc_disallowed(char *ifname, uint8 reason)
{
	int ret = BKTE_OK;
	bcm_iov_buf_t *iov_buf = NULL;
	uint8 *pxtlv = NULL;
	uint16 buflen = 0, buflen_start = 0;
	uint16 iovlen = 0;

	iov_buf = (bcm_iov_buf_t *)bkt_malloc(WLC_IOCTL_MEDLEN, &ret);
	BKT_ASSERT();

	/* fill header */
	iov_buf->version = WL_MBO_IOV_VERSION;
	iov_buf->id = WL_MBO_CMD_AP_ASSOC_DISALLOWED;

	pxtlv = (uint8*)&iov_buf->data[0];
	buflen = buflen_start = WLC_IOCTL_MEDLEN - sizeof(bcm_iov_buf_t);

	ret = bcm_pack_xtlv_entry(&pxtlv, &buflen, WL_MBO_XTLV_AP_ASSOC_DISALLOWED,
		sizeof(reason), &reason, BCM_XTLV_OPTION_ALIGN32);
	if (ret != BKTE_OK) {
		goto end;
	}

	iov_buf->len = buflen_start - buflen;
	iovlen = sizeof(bcm_iov_buf_t) + iov_buf->len;
	ret = wl_iovar_set(ifname, "mbo", (void *)iov_buf, iovlen);

end:
	if (iov_buf) {
		free(iov_buf);
	}
	return ret;
}

/* get dfs status to check whether any CAC is runing or not */
int
blanket_get_dfs_status_all(char *ifname, wl_dfs_status_all_t *dfs_status_all, int len)
{
	int ret = BKTE_OK;
	int count;

	BKT_ENTER();

	ret = wl_iovar_getbuf(ifname, "dfs_status_all", NULL, 0, dfs_status_all, len);
	BKT_IOVAR_ASSERT(ifname, "dfs_status_all", ret);

	dfs_status_all->version = dtoh16(dfs_status_all->version);
	dfs_status_all->num_sub_status = dtoh16(dfs_status_all->num_sub_status);

	if (dfs_status_all->version != WL_DFS_STATUS_ALL_VERSION) {
		BKT_WARNING("ifname[%s]  Unknown dfs status version %d\n",
			ifname, dfs_status_all->version);
		ret = BKTE_WL_ERROR;
		goto end;
	}

	/* Maximum number of cores possible in a radio is 2 incase of supporting BGDFS */
	if (dfs_status_all->num_sub_status > 2) {
		ret = BKTE_WL_ERROR;
		goto end;
	}

	for (count = 0; count < dfs_status_all->num_sub_status; ++count) {
		wl_dfs_sub_status_t *sub = &dfs_status_all->dfs_sub_status[count];

		sub->state = dtoh32(sub->state);
		sub->duration = dtoh32(sub->duration);
		sub->chanspec = wl_chspec_from_driver(sub->chanspec);
		sub->chanspec_last_cleared =
			wl_chspec_from_driver(sub->chanspec_last_cleared);
		sub->sub_type = dtoh16(sub->sub_type);

		if (sub->state >= WL_DFS_CACSTATES) {
			BKT_ERROR("Unknown dfs state %d.\n", sub->state);
			ret = BKTE_WL_ERROR;
			goto end;
		}
	}
end:
	BKT_EXIT();
	return ret;
}

/* Get Primary VLAN ID */
int blanket_get_primary_vlan_id(char *ifname, unsigned short *vlan_id)
{
	int ret = BKTE_OK;
	int tmp_vlan_id = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(vlan_id, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "map_8021q_settings", &tmp_vlan_id, sizeof(tmp_vlan_id));
	BKT_IOVAR_ASSERT(ifname, "map_8021q_settings", ret);

	*vlan_id = (unsigned short)tmp_vlan_id;
	BKT_DEBUG("ifname[%s] vlanID[%d]\n", ifname, tmp_vlan_id);

end:
	BKT_EXIT();
	return ret;
}

/* Returns mcs index from eht mcs map */
static uint32
blanket_eht_max_mcs_idx(uint32 mcs_map)
{
	uint8 nss;	/**< a value of 0 means: MCS_CODE is not supported */
	enum wlc_eht_mcs_code_e mcs_code;

	for (mcs_code = EHT_MCS_CODE_0_13; mcs_code >= EHT_MCS_CODE_0_7; mcs_code--) {
		nss = WLC_EHT_RS_GET_NSS(mcs_map, WL_TX, mcs_code);
		if (nss > 0) {
			break;
		}
	}

	return (EHT_MAX_MCS_TO_INDEX(mcs_code));
}

/* Gets the txrate from eht mcs, nss and bw */
int
blanket_get_txrate_from_eht_mcs(uint32 mcs_map, int nss, int bw_val, uint32 *out_rate)
{
	uint32 tx_rate, mcs_idx;
	enum wl_bw_e bw;
	enum wl_rspec_eht_gi_e gi;
	int ret = BKTE_OK;

	BKT_ENTER();

	BKT_ASSERT_ARG(out_rate, BKTE_INV_ARG);

	*out_rate = 0;

	switch (bw_val) {
		case 320:
			bw = BW_320MHZ;
			break;

		case 160:
			bw = BW_160MHZ;
			break;

		case 80:
			bw = BW_80MHZ;
			break;

		case 40:
			bw = BW_40MHZ;
			break;

		case 20:
			bw = BW_20MHZ;
			break;

		default:
			BKT_ERROR("Invalid BW %d \n", bw_val);
			return BKTE_INV_ARG;
	}

	mcs_idx = blanket_eht_max_mcs_idx(mcs_map);
	gi = WL_RSPEC_EHT_4x_LTF_GI_0_8us;
	tx_rate = wf_eht_mcs_to_kbps(mcs_idx, nss, bw, gi);
	tx_rate = tx_rate/1000;
	*out_rate = tx_rate;

	BKT_INFO("mcs_map[0x%x] mcs_idx[%d] nss[%d] bw[%d] txrate %u\n",
		mcs_map, mcs_idx, nss, bw_val, tx_rate);

end:
	BKT_EXIT();
	return ret;
}

/* get the txrate from vht mcs, nss and bw */
int
blanket_get_txrate(uint16 mcs_map, int nss, int bw, uint32 *out_rate, bool he)
{
	int ret = BKTE_OK;
	int mcs = 0, mcs_cap;
	uint32 rate;

	BKT_ENTER();
	BKT_ASSERT_ARG(out_rate, BKTE_INV_ARG);

	if (he) {
		mcs_cap = HE_CAP_MAX_MCS_NSS_GET_MCS(nss, mcs_map);
		mcs = (mcs_cap == HE_CAP_MAX_MCS_0_11) ? 11 :
			(mcs_cap == HE_CAP_MAX_MCS_0_9) ? 9 : 7;

	} else {
		/* If mcs map is not provided default mcs 7 is assumed */
		mcs_cap	= mcs_map ? VHT_MCS_MAP_GET_MCS_PER_SS(1, mcs_map) : VHT_CAP_MCS_MAP_0_7;

		if (mcs_cap == VHT_CAP_MCS_MAP_0_7)
			mcs = VHT_CAP_MCS_MAP_0_7_MAX_IDX - 1;
		else if (mcs_cap == VHT_CAP_MCS_MAP_0_8)
			mcs = VHT_CAP_MCS_MAP_0_8_MAX_IDX - 1;
		else if (mcs_cap == VHT_CAP_MCS_MAP_0_9)
			mcs = VHT_CAP_MCS_MAP_0_9_MAX_IDX - 1;
	}

	rate = wl_mcs2rate(mcs, nss, bw, 1);
	rate /= 1000;
	*out_rate = rate;

	BKT_DEBUG("mcs map[0x%x] mcs [%d] nss[%d] bw[%d] rate[%d]\n", mcs_map, mcs, nss, bw, rate);

end:
	BKT_EXIT();
	return ret;
}

/* gets the current country code for the given interface */
int
blanket_get_country(char *ifname, wl_country_t *out_country)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_country, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "country", out_country, sizeof(*out_country));
	BKT_IOVAR_ASSERT(ifname, "get country", ret);

	/* ensure null termination before using */
	out_country->country_abbrev[WLC_CNTRY_BUF_SZ - 1] = '\0';
	out_country->ccode[WLC_CNTRY_BUF_SZ - 1] = '\0';
end:
	BKT_EXIT();
	return ret;
}

/* create log default nvrams filename */
void
blanket_log_get_default_nvram_filename(char *fname, int len)
{
	snprintf(fname, len, BKT_DEFAULT_NVRAM_FILE, (g_wbd_process_name));
	fname[len - 1] = '\0';
}

/* get log of the default nvram names with the values to a file */
void
blanket_log_default_nvram(const char *fmt, ...)
{
	FILE *fp;
	char fname[BKT_MAX_BUF_256];
	va_list arglist;
	BKT_ENTER();

	/* check file write enabled/disabled */
	if (!(g_bkt_info.flags & BKT_INFO_FLAGS_NVRAM_FILE_WRITE_ENABLED)) {
		goto end;
	}

	/* Copy and set file name */
	blanket_log_get_default_nvram_filename(fname, sizeof(fname));

	/* Open file for writing the nvrams */
	if (!(fp = fopen(fname, "a"))) {
		BKT_INFO("Error opening file: %s: %s \n", fname, strerror(errno));
		goto end;
	}

	va_start(arglist, fmt);
	vfprintf(fp, fmt, arglist);
	va_end(arglist);

	fclose(fp);
end:
	BKT_EXIT();
	return;
}

/* get nvram settings to enable/disable file operation */
void
init_blanket_info(blanket_module_info_t *info)
{
	int check;
	BKT_ENTER();

	if (!info) {
		BKT_INFO("Invalid argument:\n");
		goto end;
	}

	/* Read nvram settings */
	check = blanket_get_config_val_int(NULL, BKT_NVRAM_FILE_WRITE, DEF_NVRAM_FILE_WRITE);

	/* nvram settings enabled set blanket flags */
	if (check == 1) {
		info->flags |= BKT_INFO_FLAGS_NVRAM_FILE_WRITE_ENABLED;
	}

	g_bkt_info.flags = info->flags;
end:
	BKT_EXIT();
	return;
}

/* Get vhtmode of Interface */
int
blanket_get_vhtmode(char* ifname, int* out_vhtmode)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_vhtmode, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "vhtmode", out_vhtmode, sizeof(*out_vhtmode));
	BKT_IOVAR_ASSERT(ifname, "get vhtmode", ret);
end:

	BKT_EXIT();
	return BKTE_OK;
}

/* Get wme of Interface */
int
blanket_get_wme(char* ifname, int* out_wme)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_wme, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "wme", out_wme, sizeof(*out_wme));
	BKT_IOVAR_ASSERT(ifname, "get wme", ret);
end:

	BKT_EXIT();
	return BKTE_OK;
}

/* Get wme_apsd of Interface */
int
blanket_get_wme_apsd(char* ifname, int* out_wme_apsd)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_wme_apsd, BKTE_INV_ARG);

	ret = wl_iovar_get(ifname, "wme_apsd", out_wme_apsd, sizeof(*out_wme_apsd));
	BKT_IOVAR_ASSERT(ifname, "get wme_apsd", ret);
end:

	BKT_EXIT();
	return BKTE_OK;
}

/* Enable/disable bgdfs on the apsta interface */
int
blanket_set_allow_bgdfs_on_apsta(char* ifname, int enable)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	ret = wl_iovar_setint(ifname, "allow_bgdfs_on_apsta", htod32(enable));
	BKT_IOVAR_ASSERT(ifname, "allow_bgdfs_on_apsta", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Check whether the 20MHz channel is in the chanspec */
int
blanket_is_20mhz_channel_in_chanspec(chanspec_t chanspec, uint8 ctrl_chn)
{
	uint8 channel, last_channel;

	FOREACH_20_SB_EFF(chanspec, channel, last_channel) {
		if (channel == ctrl_chn) {
			return 1;
		}
	}

	return 0;
}

/* Enable/disable sending CSA on the Client (bSTA) */
int
blanket_set_client_csa(char* ifname, int enable)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	ret = wl_iovar_setint(ifname, "client_csa", htod32(enable));
	BKT_IOVAR_ASSERT(ifname, "client_csa", ret);

end:
	BKT_EXIT();
	return ret;
}

/* Check whether ifname is a member of multiple bssid set */
int
blanket_is_bss_partof_mbssid_set(char *ifname, int *out_val)
{
	int ret = BKTE_OK;
	uint8 buf[BKT_MAX_BUF_128] = {0};
	wl_mbssid_t mbssid = {0};
	wl_mbssid_t *ptr;

	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_val, BKTE_INV_ARG);

	ret = wl_iovar_getbuf(ifname, "mbssid", &mbssid, sizeof(mbssid), buf, sizeof(buf));
	BKT_IOVAR_ASSERT(ifname, "get mbssid", ret);

	ptr = (wl_mbssid_t*)buf;
	if (ptr->version >= WL_MBSSID_VER) {
		*out_val = (ptr->status == 1);
	}

end:
	BKT_EXIT();
	return ret;
}

/* Check whether ifname is a transmitted bss */
int
blanket_is_bss_transmitted_bss(char *ifname, int *out_val)
{
	int ret = BKTE_OK;
	int transmitted_bss = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_val, BKTE_INV_ARG);

	ret = wl_iovar_getint(ifname, "transmit_bss", &transmitted_bss);
	BKT_IOVAR_ASSERT(ifname, "get transmit_bss", ret);
	*out_val = transmitted_bss;

end:
	BKT_EXIT();
	return ret;
}

int blanket_set_fbt_reassoc_time(char *ifname, int reassoc_time)
{
	int ret = BKTE_OK;
	BKT_ENTER();

	wl_endian_probe(ifname);

	ret = wl_iovar_setint(ifname, "fbt_reassoc_time", htod32(reassoc_time));
	BKT_IOVAR_ASSERT(ifname, "set fbt_reassoc_time", ret);

end:
	BKT_EXIT();
	return ret;
}

/* set tx power in qdBm at the interface */
int
blanket_set_txpwr(char *ifname, int txpwr)
{
	int ret = BKTE_OK, new_txpwr;
	BKT_ENTER();

	wl_endian_probe(ifname);

	if (txpwr > WLC_TXPWR_MAX) {
		BKT_WARNING("ifname[%s] tx power %d out of range\n", ifname, txpwr);
		return BKTE_INV_ARG;
	}

	if (txpwr < WL_RATE_DISABLED) {
		txpwr = WL_RATE_DISABLED;
	}

	/* Need to keep sign bit in low byte */
	new_txpwr = (uint8)txpwr;

	BKT_INFO("ifname: %s Setting txpwr %ddBm %dqdBm \n", ifname, txpwr/4, txpwr);

	ret = wl_iovar_setint(ifname, "qtxpower", htod32(new_txpwr));
	BKT_IOVAR_ASSERT(ifname, "set txpwr", ret);

end:
	return ret;
}

int blanket_get_mlo_enab(char* ifname, uint8* out_enab)
{
	int ret = BKTE_OK;
	uint8 buf[BKT_MAX_BUF_16] = {0};
	bcm_xtlv_t mybuf;
	BKT_ENTER();

	bzero(&mybuf, sizeof(mybuf));
	mybuf.id = WL_MLO_CMD_ENAB;
	mybuf.len = 0u;

	/* Validate arg */
	BKT_ASSERT_ARG(out_enab, BKTE_INV_ARG);

	ret = wl_iovar_getbuf(ifname, "mlo", &mybuf, sizeof(mybuf), buf, BKT_MAX_BUF_16);
	BKT_IOVAR_ASSERT(ifname, "get mlo", ret);

	*out_enab = *buf;
end:
	BKT_EXIT();
	return BKTE_OK;
}

/* Get int iovar */
int
blanket_iovar_getint(char *ifname, char *iovar, int *out_val)
{
	int ret = BKTE_OK;
	int temp = 0;
	BKT_ENTER();

	/* Validate arg */
	BKT_ASSERT_ARG(out_val, BKTE_INV_ARG);
	BKT_ASSERT_ARG(iovar, BKTE_INV_ARG);

	ret = wl_iovar_getint(ifname, iovar, &temp);
	BKT_IOVAR_ASSERT(ifname, iovar, ret);
	*out_val = temp;

end:
	BKT_EXIT();
	return ret;
}

/* check ifname in mldx_ifnames based on whether MLO is eanbled or not */
int
blanket_nvram_prefix_match_mld_ifnames(int8 mld_unit, char *ifname, bool is_mlo_enabled)
{
	char nvname[NVRAM_MAX_PARAM_LEN], *nvval, *entry;
	int ret = 1;
	BKT_ENTER();

	snprintf(nvname, sizeof(nvname), "mld%d_ifnames", mld_unit);
	nvval = blanket_nvram_safe_get(nvname);
	entry = find_in_list(nvval, ifname);

	/* if MLO is enabled, then the ifname should be present in nvval
	 * if MLO is disabled, then the ifname should not be present in nvval
	 */
	if (is_mlo_enabled) {
		if (entry == NULL) {
			BKT_INFO("MLO not enabled on %s, [%s=%s]. Need to enable MLO. So return "
				"not matching\n", ifname, nvname, nvval);
			goto end;
		}
	} else {
		if (entry != NULL) {
			BKT_INFO("MLO enabled on %s, [%s=%s]. Need to disable MLO. So return "
				"not matching\n", ifname, nvname, nvval);
			goto end;
		}
	}
	ret = 0;

end:
	BKT_EXIT();
	return ret;
}

/* Get MLD Address of the interface/bss */
int
blanket_get_ap_mld(char *ifname, struct ether_addr *out_ap_mld)
{
	int ret = BKTE_OK;
	uint8 mybuf[BKT_MAX_BUF_64];
	uint8 *rem = mybuf;
	uint16 rem_len = sizeof(mybuf);
	wl_mlo_info_v1_t *mlo_info = NULL;
	uint16 in_len = BCM_XTLV_HDR_SIZE;
	uint8 *resp = NULL;

	resp = (uint8 *)bkt_malloc(WLC_IOCTL_MAXLEN, &ret);
	BKT_ASSERT();

	ret = bcm_pack_xtlv_entry(&rem, &rem_len, WL_MLO_CMD_INFO,
		in_len, (uint8 *)&mlo_info, BCM_XTLV_OPTION_ALIGN32);
	if (ret != BKTE_OK) {
		goto end;
	}

	ret = wl_iovar_getbuf(ifname, "mlo", &mybuf, sizeof(mybuf), resp, WLC_IOCTL_MAXLEN);
	if (ret != BKTE_OK) {
		goto end;
	}

	if (resp != NULL) {
		mlo_info = (wl_mlo_info_v1_t *)resp;
		mlo_info->len = dtoh16(mlo_info->len);

		if (mlo_info->ver > WL_MLO_INFO_VER) {
			BKT_ERROR("Supported mlo_info version: %d but received version: %d\n",
				WL_MLO_INFO_VER, mlo_info->ver);
			return BKTE_WL_ERROR;
		}

		if (mlo_info->len > WLC_IOCTL_MAXLEN || (mlo_info->len < (sizeof(*mlo_info) +
			mlo_info->no_of_mlo_scb * sizeof(*(mlo_info->msi))))) {
			return BKTE_WL_ERROR;
		}

		eacopy(&mlo_info->self_mld_addr, out_ap_mld);
	}

end:
	if (resp) {
		free(resp);
	}

	BKT_EXIT();
	return ret;
}

/* Blanket Node get the peer MLD MAC address of STA */
int
blanket_get_peer_mld_mac(char *ifname, struct ether_addr *sta_mac,
	struct ether_addr *out_peer_mld)
{
	int ret = BKTE_OK;
	sta_info_t sta_info;
	BKT_ENTER();

	ret = blanket_get_sta_info(ifname, sta_mac, &sta_info);
	BKT_ASSERT();

	if (sta_info.flags & WL_STA_MLO_CAP) {
		eacopy(&sta_info.peer_mld, out_peer_mld);
	} else {
		ret = BKTE_INV_MAC;
	}

end:
	BKT_EXIT();
	return ret;
}

/* Get MLO link stats icluding RSSI */
int
blanket_get_mlo_link_stats(char *ifname, struct ether_addr *sta_mac,
	ieee1905_mlo_sta_link_stats *out_link_stats, int arr_size)
{
	int ret = BKTE_OK;
	uint i, rssi_window_sz = 0;
	uint8 mybuf[WLC_IOCTL_MEDLEN];
	uint8 *rem = mybuf;
	uint16 rem_len = sizeof(mybuf);
	wlc_scb_mlo_stats_v2_t *pstats;
	uint8 *resp = NULL;

	if (!sta_mac || !out_link_stats) {
		ret = BKTE_INV_ARG;
		goto end;
	}

	resp = (uint8 *)bkt_malloc(WLC_IOCTL_MAXLEN, &ret);
	if (resp == NULL) {
		goto end;
	}

	ret = bcm_pack_xtlv_entry(&rem, &rem_len, WL_MLO_CMD_SCB_STATS, ETHER_ADDR_LEN,
		(uint8 *)sta_mac, BCM_XTLV_OPTION_ALIGN32);
	if (ret != BKTE_OK) {
		goto end;
	}

	ret = wl_iovar_getbuf(ifname, "mlo", &mybuf, sizeof(mybuf), resp, WLC_IOCTL_MAXLEN);
	if (ret != BKTE_OK) {
		goto end;
	}

	pstats = (wlc_scb_mlo_stats_v2_t *)resp;
	pstats->ver = dtoh16(pstats->ver);

	/* Report unrecognized version */
	if (pstats->ver > WL_SCB_MLO_STATS_VER_2) {
		BKT_ERROR("ERROR: unknown driver station info version %d\n", pstats->ver);
		ret = BKTE_WL_ERROR;
		goto end;
	}

	for (i = 0; i < arr_size; i++) {
		int avg_rssi = 0, cnt = 0, j;
		if (!(pstats->active_link_bmp & (1 << i))) {
			continue;
		}

		/* Get Average RSSI */
		rssi_window_sz = dtoh32(pstats->rssi_perlink[i].rssi_window_sz);
		for (j = 0; j < rssi_window_sz; j++) {
			if (pstats->rssi_perlink[i].rssi_window[j] != WLC_RSSI_INVALID) {
				avg_rssi += pstats->rssi_perlink[i].rssi_window[j];
				cnt++;
			}
		}
		if (cnt) {
			avg_rssi = avg_rssi/cnt;
		}
		out_link_stats[i].tx_pkts = pstats->tx_pkts[i];
		out_link_stats[i].tx_failures = pstats->tx_failures[i];
		out_link_stats[i].tx_pkts_total = pstats->tx_pkts_total[i];
		out_link_stats[i].rx_pkts = pstats->rx_pkts[i];
		out_link_stats[i].rssi = avg_rssi;
	}
end:
	if (resp) {
		free(resp);
	}
	if (ret != BKTE_OK) {
		BKT_ERROR("Failed to get MLO link RSSI for %s. err [%d]\n", ifname, ret);
	}
	return ret;
}
