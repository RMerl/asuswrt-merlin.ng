/* Asuswrt-Merlin Wireless web display code for Broadcom platform */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>

#include <shared.h>
#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <bcmendian.h>
#include <bcmparams.h>
#ifdef HND_ROUTER
#include "bcmwifi_rates.h"
#include "wlioctl_defs.h"
#endif
#ifdef RTCONFIG_HND_ROUTER_AX
#include <wlc_types.h>
#include <802.11ax.h>
#endif
#include <wlutils.h>
#include <linux/types.h>
#include <wlscan.h>
#include <bcmdevs.h>
#include <sysinfo.h>

const char *phy_type_str_fmt[7] = {
        "a",
        "b",
        "g",
        "n",
        "ac",
        "ax",
        "be"
};

const char *wl_bw_str_fmt[6] = {
	"",
	"20MHz",
	"40MHz",
	"80MHz",
	"160MHz",
	"320MHz",
};

/* The below macros handle endian mis-matches between wl utility and wl driver. */
#if defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_BCM_7114) || defined(RTCONFIG_BCMWL6)
static bool g_swap = FALSE;
#ifndef htod16
#define htod16(i) (g_swap?bcmswap16(i):(uint16)(i))
#endif
#ifndef htod32
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#endif
#ifndef dtoh16
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
#endif
#ifndef dtoh32
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#endif
#ifndef htodchanspec
#define htodchanspec(i) (g_swap?htod16(i):i)
#endif
#ifndef dtohchanspec
#define dtohchanspec(i) (g_swap?dtoh16(i):i)
#endif
#endif

#define SSID_FMT_BUF_LEN 4*32+1 /* Length for SSID format string */
#define MAX_STA_COUNT   128

#define CHANIMSTR(a, b, c, d) ((a) ? ((b) ? c : d) : "")
static const uint wf_chspec_bw_mhz[] = {5, 10, 20, 40, 80, 160, 320};

#define WF_NUM_BW \
        (sizeof(wf_chspec_bw_mhz)/sizeof(uint))

#ifdef RTCONFIG_MULTILAN_CFG
#define MERGED_LEASE_FILE "/tmp/dnsmasq-merged.leases"
#endif

#ifdef RTCONFIG_MLO
void get_mld_name(void *wl, char *scb_client, char *mld_client);
#endif

/* From web-broadcom.c */
extern int wl_format_ssid(char* ssid_buf, uint8* ssid, int ssid_len);
extern int wl_control_channel(int unit);
extern sta_info_t *wl_sta_info(char *ifname, struct ether_addr *ea);
extern int wl_sta_info_nss(void *buf, int unit);
extern int wl_sta_info_phy(void *buf, int unit);
extern int wl_sta_info_bw(void *buf);

static uint
bw_chspec_to_mhz(chanspec_t chspec)
{
	uint bw;

	bw = (chspec & WL_CHANSPEC_BW_MASK) >> WL_CHANSPEC_BW_SHIFT;
	return (bw >= WF_NUM_BW ? 0 : wf_chspec_bw_mhz[bw]);
}

static int
wl_extent_channel(int unit)
{
	int ret;
	struct ether_addr bssid;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;

        snprintf(prefix, sizeof(prefix), "wl%d_", unit);
        name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (unit != WL_2G_BAND) {
		if ((ret = wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
			/* The adapter is associated. */
			*(uint32*)buf = htod32(WLC_IOCTL_MAXLEN);
			if ((ret = wl_ioctl(name, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0)
				return 0;
			bi = (wl_bss_info_t*)(buf + 4);

			return bw_chspec_to_mhz(bi->chanspec);
		} else {
			return 0;
		}
	}

	if ((ret = wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
		/* The adapter is associated. */
		*(uint32*)buf = htod32(WLC_IOCTL_MAXLEN);
		if ((ret = wl_ioctl(name, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0)
			return 0;

		bi = (wl_bss_info_t*)(buf + 4);
		if (dtoh32(bi->version) == WL_BSS_INFO_VERSION ||
		   dtoh32(bi->version) == LEGACY2_WL_BSS_INFO_VERSION ||
		   dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION)
		{
			/* Convert version 107 to 109 */
			if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
				old_bi = (wl_bss_info_107_t *)bi;
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
				bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel, WL_CHANNEL_2G5G_BAND(old_bi->channel));
#else
				bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
#endif
				bi->ie_length = old_bi->ie_length;
				bi->ie_offset = sizeof(wl_bss_info_107_t);
			}
			if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap)
				return  CHSPEC_CHANNEL(bi->chanspec);
		}
	}
	return 0;
}

int
ej_wl_extent_channel(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;
	char word[256], *next;
	int count_wl_if = 0;
	char wl_ifnames[32] = { 0 };

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	ret = websWrite(wp, "[");

	foreach (word, wl_ifnames, next) {
		ret += websWrite(wp, "\"%d\",",  wl_extent_channel(count_wl_if++));
	}

	ret += websWrite(wp, "\"0\"]");
	return ret;
}


int
ej_wl_status_array(int eid, webs_t wp, int argc, char_t **argv)
{
	int retval = 0;
	int ii = 0;
	char nv_param[NVRAM_MAX_PARAM_LEN];
	char *temp;

	for (ii = 0; ii < DEV_NUMIFS; ii++) {
		sprintf(nv_param, "wl%d_unit", ii);
		temp = nvram_get(nv_param);

		if (temp && *temp)
		{
			retval += ej_wl_unit_status_array(eid, wp, argc, argv, ii);
			retval += websWrite(wp, "\n");
		}
	}

	return retval;
}

static int
#ifdef RTCONFIG_HND_ROUTER_AX
dump_bss_info_array(int eid, webs_t wp, int argc, char_t **argv, wl_bss_info_v109_1_t *bi, char *name)
#else
dump_bss_info_array(int eid, webs_t wp, int argc, char_t **argv, wl_bss_info_t *bi, char *name)
#endif
{
	char ssidbuf[SSID_FMT_BUF_LEN*2], ssidbuftmp[SSID_FMT_BUF_LEN];
	char chspec_str[CHANSPEC_STR_LEN];
	wl_bss_info_107_t *old_bi;
	int retval = 0;

	/* Convert version 107 to 109 */
	if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
		old_bi = (wl_bss_info_107_t *)bi;
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
		bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel, WL_CHANNEL_2G5G_BAND(old_bi->channel));
#else
		bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
#endif
		bi->ie_length = old_bi->ie_length;
		bi->ie_offset = sizeof(wl_bss_info_107_t);
	} else {
		/* do endian swap and format conversion for chanspec if we have
		* not created it from legacy bi above
		*/
		bi->chanspec = dtohchanspec(bi->chanspec);
	}

	wl_format_ssid(ssidbuftmp, bi->SSID, bi->SSID_len);

	if (str_escape_quotes(ssidbuf, ssidbuftmp, sizeof(ssidbuf)) == 0 )
		strlcpy(ssidbuf, ssidbuftmp, sizeof(ssidbuf));

	retval += websWrite(wp, "\"%s\",", ssidbuf);
	retval += websWrite(wp, "\"%d\",", (int16)(dtoh16(bi->RSSI)));

	/*
	 * SNR has valid value in only 109 version.
	 * So print SNR for 109 version only.
	 */
	if (dtoh32(bi->version) == WL_BSS_INFO_VERSION) {
		retval += websWrite(wp, "\"%d\",", (int16)(dtoh16(bi->SNR)));
	} else {
		retval += websWrite(wp, "\"?\",");
	}

	retval += websWrite(wp, "\"%d\",", bi->phy_noise);
	retval += websWrite(wp, "\"%s\",", wf_chspec_ntoa(dtohchanspec(bi->chanspec), chspec_str));
	retval += websWrite(wp, "\"%s\",", wl_ether_etoa(&bi->BSSID));

#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
#if defined(RTCONFIG_BCM_502L07P2)
	if (!dhd_probe(name))
#else
	if (1)
#endif
		retval += websWrite(wp, "\"%d\",", bi->qbss_chan_util * 100 / (uint8)WLC_QBSS_LOAD_CHAN_FREE_MAX);
	else
		retval += websWrite(wp, "\"-1\",");
#else
	retval += websWrite(wp, "\"-1\",");
#endif

	return retval;
}

static int
wl_status_array(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	int ret;
	struct ether_addr bssid;
	wlc_ssid_t ssid;
	char ssidbuf[SSID_FMT_BUF_LEN*2], ssidbuftmp[SSID_FMT_BUF_LEN];
#ifdef RTCONFIG_HND_ROUTER_AX
	wl_bss_info_v109_1_t *bi;
#else
	wl_bss_info_t *bi;
#endif
	int retval = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;

	snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if ((ret = wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
		/* The adapter is associated. */
		*(uint32*)buf = htod32(WLC_IOCTL_MAXLEN);
		if ((ret = wl_ioctl(name, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0) {
			retval += websWrite(wp, "\"?\",\"?\",\"?\",\"?\",\"?\",\"?\",");
			return retval;
		}
#ifdef RTCONFIG_HND_ROUTER_AX
		bi = (wl_bss_info_v109_1_t*)(buf + 4);
#else
		bi = (wl_bss_info_t*)(buf + 4);
#endif
		if (dtoh32(bi->version) == WL_BSS_INFO_VERSION ||
		    dtoh32(bi->version) == LEGACY2_WL_BSS_INFO_VERSION ||
		    dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION)
			retval += dump_bss_info_array(eid, wp, argc, argv, bi, name);
		else
			retval += websWrite(wp, "\"<error>\",\"\",\"\",\"\",\"\",\"\",");
	} else {
		retval += websWrite(wp, "\"Not associated. Last with ");

		if ((ret = wl_ioctl(name, WLC_GET_SSID, &ssid, sizeof(wlc_ssid_t))) < 0) {
			retval += websWrite(wp, "<unknown>\",\"\",\"\",\"\",\"\",\"\",");
			return 0;
		}

		wl_format_ssid(ssidbuftmp, ssid.SSID, dtoh32(ssid.SSID_len));

		if (str_escape_quotes(ssidbuf, ssidbuftmp, sizeof(ssidbuf)) == 0 )
			strlcpy(ssidbuf, ssidbuftmp, sizeof(ssidbuf));

		retval += websWrite(wp, "%s\",\"\",\"\",\"\",\"\",\"\",", ssidbuf);
	}

	return retval;
}

int
ej_wl_unit_status_array(int eid, webs_t wp, int argc, char_t **argv, int unit)
{
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	struct maclist *auth;
	int mac_list_size;
	int i, ii, val = 0, ret = 0, subunit = 0, maxunit;
	char *arplist = NULL, *arplistptr;
	char *leaselist = NULL, *leaselistptr;
	char *ipv6list = NULL, *ipv6listptr;
	char *line;
	char hostnameentry[65];
	char ipentry_arp[16], ipentry_dhcp[16], ipentry_ipv6[1024], macentry[18];
	unsigned int flagentry;
	int found, foundipv6 = 0, noclients = 0;
	char rxrate[12], txrate[12];
	char ea[ETHER_ADDR_STR_LEN];
#ifdef RTCONFIG_MLO
	char mld_mac[ETHER_ADDR_STR_LEN];
#endif
	scb_val_t scb_val;
	int hr, min, sec;
	sta_info_t *sta;
#ifdef RTCONFIG_BCMWL6
	wl_dfs_status_t *dfs_status;
	char chanspec_str[CHANSPEC_STR_LEN];
#endif
	char guestssid[SSID_FMT_BUF_LEN];
#ifdef RTCONFIG_MULTILAN_CFG
	int guestvlan;
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	char leasefile_path[128] = {0};
#endif

#ifdef RTCONFIG_PROXYSTA
	if (psta_exist_except(unit))
		return ret;
#endif

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	wl_ioctl(name, WLC_GET_RADIO, &val, sizeof(val));
	val &= WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE;

	if (val)
		return ret;

	ret += websWrite(wp, "dataarray%d = [", unit);

	if (nvram_match(strcat_r(prefix, "mode", tmp), "wds"))
		ret += websWrite(wp, "\"\",\"\",\"\",\"\",\"%d\",\"\",", wl_control_channel(unit));
	else
		ret += wl_status_array(eid, wp, argc, argv, unit);

	if (nvram_match(strcat_r(prefix, "mode", tmp), "ap"))
	{
		if (nvram_match(strcat_r(prefix, "lazywds", tmp), "1") ||
			nvram_invmatch(strcat_r(prefix, "wds", tmp), ""))
			ret += websWrite(wp, "\"Hybrid\"");
		else	ret += websWrite(wp, "\"AP\"");
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "wds"))
	{
		ret += websWrite(wp, "\"WDS\"");
		noclients = 1;
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "sta"))
	{
		ret += websWrite(wp, "\"Stations\"");
		noclients = 1;
	}
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "wet"))
	{
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER)
			&& (nvram_get_int("wlc_band") == unit))
			sprintf(prefix, "wl%d.%d_", unit, 1);
#endif
		ret += websWrite(wp, "\"Repeater ( SSID local: %s )\"", nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
	}
#ifdef RTCONFIG_PROXYSTA
	else if (nvram_match(strcat_r(prefix, "mode", tmp), "psta"))
	{
		if ((nvram_get_int("sw_mode") == SW_MODE_AP) &&
			(nvram_get_int("wlc_psta") == 1) &&
			(nvram_get_int("wlc_band") == unit))
		ret += websWrite(wp, "\"Media Bridge\"");
	}
#endif

// Close dataarray
	ret += websWrite(wp, "];\n");

// DFS status
#ifdef RTCONFIG_BCMWL6
	if (!nvram_match(strlcat_r(prefix, "reg_mode", tmp, sizeof(tmp)), "h"))
		goto sta_list;

	memset(buf, 0, sizeof(buf));
	strcpy(buf, "dfs_status");

	if (wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf)) < 0)
		goto sta_list;

	dfs_status = (wl_dfs_status_t *) buf;
	dfs_status->state = dtoh32(dfs_status->state);
	dfs_status->duration = dtoh32(dfs_status->duration);
	dfs_status->chanspec_cleared = dtohchanspec(dfs_status->chanspec_cleared);

	const char *dfs_cacstate_str[WL_DFS_CACSTATES] = {
		"Idle",
		"PRE-ISM Channel Availability Check (CAC)",
		"In-Service Monitoring (ISM)",
		"Channel Switching Announcement (CSA)",
		"POST-ISM Channel Availability Check",
		"PRE-ISM Ouf Of Channels (OOC)",
		"POST-ISM Out Of Channels (OOC)"
	};

	ret += websWrite(wp, "var dfs_statusarray%d = [\"%s\", \"%d s\", \"%s\"];\n",
		unit,
		(dfs_status->state >= WL_DFS_CACSTATES ? "Unknown" : dfs_cacstate_str[dfs_status->state]),
		dfs_status->duration/1000,
		(dfs_status->chanspec_cleared ? wf_chspec_ntoa(dfs_status->chanspec_cleared, chanspec_str) : "None"));
#endif

	if (noclients)
		return ret;

sta_list:

// Open client array
	ret += websWrite(wp, "wificlients%d = [", unit);

	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);

	if (!auth)
		goto exit;

	/* Obtain mac + IP list */
	arplist = read_whole_file("/proc/net/arp");

	/* Obtain lease list - we still need the arp list for
	   cases where a device uses a static IP rather than DHCP */
#ifdef RTCONFIG_MULTILAN_CFG
	/* Merge all SDN lease files into one */
	doSystem("cp /var/lib/misc/dnsmasq.leases %s", MERGED_LEASE_FILE);
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan(pmtl, &mtl_sz);
		for (i = 1; i < mtl_sz; i++) {
			if (pmtl[i].enable && pmtl[i].nw_t.dhcp_enable) {
				snprintf(leasefile_path, sizeof(leasefile_path), "/var/lib/misc/dnsmasq-%d.leases", pmtl[i].sdn_t.sdn_idx);
				doSystem("cat %s >> %s", leasefile_path, MERGED_LEASE_FILE);
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
	leaselist = read_whole_file(MERGED_LEASE_FILE);
	unlink(MERGED_LEASE_FILE);
#else
	leaselist = read_whole_file("/var/lib/misc/dnsmasq.leases");
#endif

#ifdef RTCONFIG_IPV6
	/* Obtain IPv6 info */
	if (ipv6_enabled()) {
		get_ipv6_client_info();
		get_ipv6_client_list();
		ipv6list = read_whole_file("/tmp/ipv6_client_list");
	}
#endif


/*** Do all subunit client lists - subunit 1 = main interface ***/

	maxunit = wl_max_no_vifs(unit);
	for (subunit = 1; subunit < maxunit; subunit++) {
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER)
			&& (unit == nvram_get_int("wlc_band")) && (subunit == 1))
			break;
#endif
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
		{
			snprintf(name_vif, sizeof(name_vif), "wl%d.%d", unit, subunit);

// Not primary interface - retrieve ssid and VLAN
			if (subunit != 1) {
				strlcpy(guestssid, nvram_pf_safe_get(prefix, "ssid"), sizeof(guestssid));
#ifdef RTCONFIG_MULTILAN_CFG
				guestvlan = get_apg_vid_by_ifname(name_vif);
#endif
			}

			memset(auth, 0, mac_list_size);

			/* query wl for authenticated sta list */
			strcpy((char*)auth, "authe_sta_list");
			if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
				goto exit;

			for (ii = 0; ii < auth->count; ii++) {
				sta = wl_sta_info(name_vif, &auth->ea[ii]);
				if (!sta) continue;

				ether_etoa((void *)&auth->ea[ii], ea);
				ret += websWrite(wp, "[\"%s\",", ea);

#ifdef RTCONFIG_MLO
				*mld_mac = '\0';
				get_mld_name(name_vif, ea, mld_mac);

				if (*mld_mac)
					strlcpy(ea, mld_mac, sizeof(ea));
#endif

				found = 0;
				if (arplist) {
					arplistptr = strdup(arplist);
					line = strtok(arplistptr, "\n");
					while (line) {
						if ( (sscanf(line,"%15s %*s %x %17s", ipentry_arp, &flagentry, macentry) == 3) &&
						     (!strcasecmp(macentry, ea)) &&
						     (flagentry != 0) ) {
						         found = 1;
						         break;
						} else
							line  = strtok(NULL, "\n");
					}
					if (arplistptr) free(arplistptr);

					if (found || !leaselist)
						ret += websWrite(wp, "\"%s\",", (found ? ipentry_arp : ""));
				} else {
					ret += websWrite(wp, "\"\",");
				}

				// Retrieve hostname from dnsmasq leases
				if (leaselist) {
					leaselistptr = strdup(leaselist);
					line = strtok(leaselistptr, "\n");
					while (line) {
						if ( (sscanf(line,"%*s %17s %15s %32s %*s", macentry, ipentry_dhcp, tmp) == 3) &&
						     ( (found > 0 && !strcmp(ipentry_arp, ipentry_dhcp)) ||
						       (!strcasecmp(macentry, ea))
						     )
						   ) {
							found += 2;
							break;
						} else
							line = strtok(NULL, "\n");
					}
					if (leaselistptr) free(leaselistptr);

					if (found) {
						if (str_escape_quotes(hostnameentry, tmp, sizeof(hostnameentry)) == 0 )
							strlcpy(hostnameentry, tmp, sizeof(hostnameentry));
					} else {
						*hostnameentry = '\0';
					}

					switch (found) {
					case 0:	// Not in arplist nor in leaselist
						ret += websWrite(wp, "\"\",\"\",");
						break;
					case 1:	// Only in arplist (static IP)
						ret += websWrite(wp, "\"\",");
						break;
					case 2:	// Only in leaselist (dynamic IP that has not communicated with router for a while)
						ret += websWrite(wp, "\"%s\",\"%s\",", ipentry_dhcp, hostnameentry);
						break;
					case 3:	// In both arplist and leaselist (dynamic IP)
						ret += websWrite(wp, "\"%s\",", hostnameentry);
						break;
					}
				} else {
					ret += websWrite(wp, "\"\",");
				}

#ifdef RTCONFIG_IPV6
// Retrieve IPv6
				if (ipv6list) {
					ipv6listptr = ipv6list;
					foundipv6 = 0;
					while ((ipv6listptr < ipv6list+strlen(ipv6list)-2) && (sscanf(ipv6listptr,"%*s %17s %1023s", macentry, ipentry_ipv6) == 2)) {
						if (strcasecmp(macentry, ea) == 0) {
							ret += websWrite(wp, "\"%s\",", ipentry_ipv6);
							foundipv6 = 1;
							break;
						} else {
							ipv6listptr = strstr(ipv6listptr,"\n")+1;
						}
					}
				}
#endif

				if (foundipv6 == 0) {
					ret += websWrite(wp, "\"\",");
				}
// RSSI
				memcpy(&scb_val.ea, &auth->ea[ii], ETHER_ADDR_LEN);
				if (wl_ioctl(name_vif, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
					ret += websWrite(wp, "\"?\",");
				else
					ret += websWrite(wp, "\"%d\",", scb_val.val);

				if (sta->flags & WL_STA_SCBSTATS)
				{
// Rate
					if ((int)sta->rx_rate > 0)
						sprintf(rxrate,"%d", sta->rx_rate / 1000);
					else
						strcpy(rxrate,"??");

					if ((int)sta->tx_rate > 0)
						sprintf(txrate,"%d", sta->tx_rate / 1000);
					else
						sprintf(txrate,"??");

					ret += websWrite(wp, "\"%s\",\"%s\",", rxrate, txrate);

// Connect time
					hr = sta->in / 3600;
					min = (sta->in % 3600) / 60;
					sec = sta->in - hr * 3600 - min * 60;
					ret += websWrite(wp, "\"%3d:%02d:%02d\",", hr, min, sec);

// NSS
#if (WL_STA_VER >= 5)
					val =  wl_sta_info_nss(sta, unit);
					if (val > 0)
						ret += websWrite(wp, "\"%d\",", val);
					else
#endif
						ret += websWrite(wp, "\"\",");

// PHY
					ret += websWrite(wp, "\"%s\",", phy_type_str_fmt[wl_sta_info_phy(sta, unit)]);

// Bandwidth
#if (WL_STA_VER >= 7)
					if (sta->flags & WL_STA_SCBSTATS)
						ret += websWrite(wp, "\"%s\",", wl_bw_str_fmt[wl_sta_info_bw(sta)]);
					else
#endif
						ret += websWrite(wp, "\"\",");

// Flags
#ifdef RTCONFIG_BCMARM
					ret += websWrite(wp, "\"%s%s%s",
						(sta->flags & WL_STA_PS) ? "P" : "_",
						((sta->ht_capabilities & WL_STA_CAP_SHORT_GI_20) || (sta->ht_capabilities & WL_STA_CAP_SHORT_GI_40)) ? "S" : "_",
						((sta->ht_capabilities & WL_STA_CAP_TX_STBC) || (sta->ht_capabilities & WL_STA_CAP_RX_STBC_MASK)) ? "T" : "_");
#ifdef RTCONFIG_MUMIMO
					ret += websWrite(wp, "%s",
						((sta->vht_flags & WL_STA_MU_BEAMFORMER) || (sta->vht_flags & WL_STA_MU_BEAMFORMEE)) ? "M" : "_");
#endif
#else
					ret += websWrite(wp, "\"%s",
						(sta->flags & WL_STA_PS) ? "P" : "_");
#endif
				}

// Auth/Ass (and Guest) flags
				ret += websWrite(wp, "%s%s\",",
					(sta->flags & WL_STA_ASSOC) ? "A" : "_",
					(sta->flags & WL_STA_AUTHO) ? "U" : "_");

// If not a Guest Network then don't push SSID and VLAN in client list
				if (subunit == 1)
			                ret += websWrite(wp, "\"\",\"\"],");
				else {
// SSID (for Guest Networks identification)
					ret += websWrite(wp, "\"%s\",", guestssid);
// VLAN
#ifdef RTCONFIG_MULTILAN_CFG
					ret += websWrite(wp, "\"%d\"],", guestvlan);
#else
					ret += webswrite(wp, "\"\"],");
#endif
				}
			}
		}
	}
	/* error/exit */
exit:
	ret += websWrite(wp, "\"-1\"];");
	if (auth) free(auth);
	if (arplist) free(arplist);
	if (leaselist) free(leaselist);
	if (ipv6list) free(ipv6list);

	return ret;
}


#ifdef RTCONFIG_MLO
void get_mld_name(void *wl, char *scb_client, char *mld_client)
{
	int iter;
	char scb_macaddr[64] = {0};
	uint8 mybuf[64];
	uint8 *rem = mybuf;
	uint16 rem_len = sizeof(mybuf);
	uint16 in_len = BCM_XTLV_HDR_SIZE;
	wl_mlo_info_v1_t *mlo_info = NULL;
	uint8 *resp = NULL;

	resp = (uint8 *)malloc(WLC_IOCTL_MAXLEN);

	if (bcm_pack_xtlv_entry(&rem, &rem_len, WL_MLO_CMD_INFO, in_len, (uint8 *)&mlo_info, BCM_XTLV_OPTION_ALIGN32))
		goto END;

	if (wl_iovar_getbuf(wl, "mlo", &mybuf, sizeof(mybuf), resp, WLC_IOCTL_MAXLEN))
		goto END;

	if (resp != NULL) {
		mlo_info = (wl_mlo_info_v1_t *)resp;
		mlo_info->len = dtoh16(mlo_info->len);

		if (mlo_info->ver > WL_MLO_INFO_VER ||
		    mlo_info->len > WLC_IOCTL_MAXLEN || (mlo_info->len < (sizeof(*mlo_info) + mlo_info->no_of_mlo_scb * sizeof(*(mlo_info->msi))))) {
			goto END;
		}

		for (iter = 0; iter < mlo_info->no_of_mlo_scb; iter++) {
			// MLO SCB address
			snprintf(scb_macaddr, sizeof(scb_macaddr), "%s", wl_ether_etoa(&mlo_info->msi[iter].ea));

			// Compare input MAC with Driver SCB
			if(!strcmp(scb_client, scb_macaddr)) {
				// Return MLD mac address
				snprintf(mld_client, 64, "%s", wl_ether_etoa(&mlo_info->msi[iter].peer_mld_addr));
			}

		}
	}
END:
	if(resp)
		free(resp);
}
#endif
