#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmendian.h>
#include <bcmnvram.h>
#include <wlutils.h>
#include "utils.h"
#include "shutils.h"
#include "shared.h"
#ifdef RTCONFIG_HND_ROUTER_AX
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || (defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(RTCONFIG_HND_ROUTER_AX_6710))
#else
#include <wlc_types.h>
#endif
#endif


#define __USE_GNU
#include <sched.h>

#ifdef LINUX26
#define GPIO_IOCTL
#endif

// --- move begin ---
#ifdef GPIO_IOCTL

#include <sys/ioctl.h>
#include <linux_gpio.h>
#include <time.h>

#ifdef RTCONFIG_AMAS
#include "amas_path.h"
#endif

typedef enum cmds_e {
        REGACCESS,
        PMDIOACCESS,
}ecmd_t;

extern uint64_t hnd_ethswctl(ecmd_t act, unsigned int val, int len, int wr, unsigned long long regdata);

static int _gpio_ioctl(int f, int gpioreg, unsigned int mask, unsigned int val)
{
	struct gpio_ioctl gpio;

	gpio.val = val;
	gpio.mask = mask;

	if (ioctl(f, gpioreg, &gpio) < 0) {
		_dprintf("Invalid gpioreg %d\n", gpioreg);
		return -1;
	}
	return (gpio.val);
}

static int _gpio_open()
{
	int f = open("/dev/gpio", O_RDWR);
	if (f < 0)
		_dprintf ("Failed to open /dev/gpio\n");
	return f;
}

int gpio_open(uint32_t mask)
{
	uint32_t bit;
	int i;
	int f = _gpio_open();

	if ((f >= 0) && mask) {
		for (i = 0; i <= 15; i++) {
			bit = 1 << i;
			if ((mask & bit) == bit) {
				_gpio_ioctl(f, GPIO_IOC_RESERVE, bit, bit);
				_gpio_ioctl(f, GPIO_IOC_OUTEN, bit, 0);
			}
		}
		close(f);
		f = _gpio_open();
	}

	return f;
}

void gpio_write(uint32_t bitvalue, int en)
{
	int f;
	uint32_t bit;

	bit = 1<< bitvalue;

	if ((f = gpio_open(0)) < 0) return;

	_gpio_ioctl(f, GPIO_IOC_RESERVE, bit, bit);
	_gpio_ioctl(f, GPIO_IOC_OUTEN, bit, bit);
	_gpio_ioctl(f, GPIO_IOC_OUT, bit, en ? bit : 0);
	close(f);
}

uint32_t _gpio_read(int f)
{
	uint32_t r;
//	r = _gpio_ioctl(f, GPIO_IOC_IN, 0xFFFF, 0);
	r = _gpio_ioctl(f, GPIO_IOC_IN, 0x07FF, 0);
	if (r < 0) r = ~0;
	return r;
}

uint32_t gpio_read(void)
{
	int f;
	uint32_t r;

	if ((f = gpio_open(0)) < 0) return ~0;
	r = _gpio_read(f);
	close(f);
	return r;
}

#else

int gpio_open(uint32_t mask)
{
	int f = open(DEV_GPIO(in), O_RDONLY|O_SYNC);
	if (f < 0)
		_dprintf ("Failed to open %s\n", DEV_GPIO(in));
	return f;
}

void gpio_write(uint32_t bitvalue, int en)
{
	int f;
	uint32_t r;
	uint32_t bit;

	bit = 1<<bitvalue;

	if ((f = open(DEV_GPIO(control), O_RDWR)) < 0) return;
	read(f, &r, sizeof(r));
	r &= ~bit;
	write(f, &r, sizeof(r));
	close(f);

	if ((f = open(DEV_GPIO(outen), O_RDWR)) < 0) return;
	read(f, &r, sizeof(r));
	r |= bit;
	write(f, &r, sizeof(r));
	close(f);

	if ((f = open(DEV_GPIO(out), O_RDWR)) < 0) return;
	read(f, &r, sizeof(r));
	if (en) r |= bit;
		else r &= ~bit;
	write(f, &r, sizeof(r));
	close(f);
}

uint32_t _gpio_read(int f)
{
	uint32_t v;
	return (read(f, &v, sizeof(v)) == sizeof(v)) ? v : ~0;
}

uint32_t gpio_read(void)
{
	int f;
	uint32_t r;

	if ((f = open(DEV_GPIO(in), O_RDONLY)) < 0) return ~0;
	r = _gpio_read(f);
	close(f);
	return r;
}


#endif

#ifdef RTCONFIG_AMAS
static bool g_swap = FALSE;
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
char *get_pap_bssid(int unit, char bssid_str[])
{
	unsigned char bssid[6];
	unsigned char bssid_null[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char ifname[IFNAMSIZ] = { 0 };

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	strlcpy(ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(ifname));

	memset(bssid_str, 0, 18);
	if (!wl_ioctl(ifname, WLC_GET_BSSID, bssid, sizeof(bssid))
		&& memcmp(bssid, bssid_null, ETHER_ADDR_LEN))
		ether_etoa((const unsigned char *) &bssid, bssid_str);

	return bssid_str;
}

sta_info_t *wl_sta_info(char *ifname, struct ether_addr *ea)
{
	static char buf[sizeof(sta_info_t)];
	sta_info_t *sta = NULL;

	strcpy(buf, "sta_info");
	memcpy(buf + strlen(buf) + 1, (void *)ea, ETHER_ADDR_LEN);

	if (!wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf))) {
		sta = (sta_info_t *)buf;
		sta->ver = dtoh16(sta->ver);

		/* Report unrecognized version */
		if (sta->ver > WL_STA_VER) {
			dbg(" ERROR: unknown driver station info version %d\n", sta->ver);
			return NULL;
		}

		sta->len = dtoh16(sta->len);
		sta->cap = dtoh16(sta->cap);
#ifdef RTCONFIG_BCMARM
		sta->aid = dtoh16(sta->aid);
#endif
		sta->flags = dtoh32(sta->flags);
		sta->idle = dtoh32(sta->idle);
		sta->rateset.count = dtoh32(sta->rateset.count);
		sta->in = dtoh32(sta->in);
		sta->listen_interval_inms = dtoh32(sta->listen_interval_inms);
#ifdef RTCONFIG_BCMARM
		sta->ht_capabilities = dtoh16(sta->ht_capabilities);
		sta->vht_flags = dtoh16(sta->vht_flags);
#endif
	}

	return sta;
}

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
#define	NVRAM_BUFSIZE	100

int get_psta_status(int unit)
{
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char ifname[IFNAMSIZ] = { 0 };
	struct maclist *mac_list = NULL;
	int mac_list_size;
	struct ether_addr bssid;
	unsigned char bssid_null[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
#if 0
	char macaddr[18];
#endif
	int ret = 0;
	int debug = nvram_get_int("psta_status_debug");

	if (unit == -1) return 0;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if (!is_psta(unit) && !is_psr(unit))
		goto PSTA_ERR;

	strlcpy(ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(ifname));

	if (wl_ioctl(ifname, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN) != 0)
		goto PSTA_ERR;
	else if (!memcmp(&bssid, bssid_null, 6))
		goto PSTA_ERR;

	//if (debug) dbg("[wlc] wl-associated\n");

	/* buffers and length */
	mac_list_size = sizeof(mac_list->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	mac_list = malloc(mac_list_size);

	if (!mac_list)
		goto PSTA_ERR;

	/* query wl for authorized sta list */

	if (nvram_match(strcat_r(prefix, "akm", tmp), ""))
		ret = 2;
	else
	{
		ret = 1;

		strcpy((char*)mac_list, "autho_sta_list");
		if (wl_ioctl(ifname, WLC_GET_VAR, mac_list, mac_list_size)) {
			free(mac_list);
			goto PSTA_ERR;
		}

		if (mac_list->count)
			ret = 2;
	}

PSTA_ERR:
	if (mac_list) free(mac_list);

	if (ret == 2) {
#if 0
		if (debug) dbg("[wlc] authorized\n");
		ether_etoa((const unsigned char *) &bssid, macaddr);
		if (debug) dbg("psta send keepalive nulldata to %s\n", macaddr);
		eval("wl", "-i", ifname, "send_nulldata", macaddr);
#endif
	}
	else if (ret == 1) {
		if (debug) dbg("[wlc] not authorized\n");
	} else {
		if (debug) dbg("[wlc] not associated\n");
	}

	return ret;
}

void wait_connection_finished(int band)
{
    int wait_time = 0;
    int conn_stat = 0;
	int wlc_conn_time = nvram_get_int("wlc_conn_time") ? : 10;

    while (wait_time++ < wlc_conn_time)
    {
    	conn_stat = get_psta_status(band);
    	//dbG("[%s] (wait_time = %d) conn_stat[band%d] = %d\n", __FUNCTION__, wait_time, band, conn_stat);
        if ( conn_stat == WLC_STATE_CONNECTED)
            break;
        sleep(1);
    }
}
#else

int get_psta_status(int unit)
{
	return 0;
}

void sync_control_channel(int unit, int channel, int bw, int nctrlsb)
{
	// TBD
}

#endif

static int is_hex(char c)
{
	return (((c >= '0') && (c <= '9')) ||
		((c >= 'A') && (c <= 'F')) ||
		((c >= 'a') && (c <= 'f')));
} /* End of is_hex */

int string2hex(const char *a, unsigned char *e, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = a[idx];
		tmpBuf[1] = a[idx+1];
		tmpBuf[2] = 0;
		if ( !is_hex(tmpBuf[0]) || !is_hex(tmpBuf[1]))
			return 0;
		e[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
} /* End of string2hex */

/**
 * @brief add beacon vise by unit and subunit
 *
 * @param unit band index
 * @param subunit mssid index
 * @param hexdata vise string
 */
void add_beacon_vsie_by_unit(int unit, int subunit, char *hexdata)
{
	unsigned char value[512];
	int pktflag = VNDR_IE_BEACON_FLAG | VNDR_IE_PRBRSP_FLAG;
	int len = 0;

	memset(value, 0, sizeof(value));
	len = DOT11_OUI_LEN + strlen(hexdata)/2;

	if (string2hex(hexdata, value, strlen(hexdata)))
		wl_add_ie(unit, subunit, pktflag, len, (uchar *) OUI_ASUS, value);
}

/**
 * @brief add guest vsie
 *
 * @param hexdata vsie string
 */
void add_beacon_vsie_guest(char *hexdata)
{
    int unit = 0, subunit = 0;
    char word[100], *next;
    char wl_ifnames[32] = { 0 };

    strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
    foreach (word, wl_ifnames, next) {
        if (nvram_get_int("re_mode") == 1)  // RE
            subunit = 3;
        else  // CAP/Router
            subunit = 2;
        for (; subunit <=  num_of_mssid_support(unit); subunit++) {
			char buf[] = "wlXX.XX_ifname";
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf), "wl%d.%d_ifname", unit, subunit);
            if (is_intf_up(nvram_safe_get(buf)) != -1)  // interface exist
                add_beacon_vsie_by_unit(unit, subunit, hexdata);
        }
        unit++;
    }
}

void add_beacon_vsie(char *hexdata)
{
	unsigned char value[512];
	int pktflag = VNDR_IE_BEACON_FLAG | VNDR_IE_PRBRSP_FLAG;
	int len = 0;
#ifdef RTCONFIG_BHCOST_OPT
	int unit = 0;
	char word[100], *next;
	char wl_ifnames[32] = { 0 };
#endif

	memset(value, 0, sizeof(value));
	len = DOT11_OUI_LEN + strlen(hexdata)/2;

	if (string2hex(hexdata, value, strlen(hexdata))) {
#ifdef RTCONFIG_BHCOST_OPT
		strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
		foreach (word, wl_ifnames, next) {
			wl_add_ie(unit, 0, pktflag, len, (uchar *) OUI_ASUS, value);
			unit++;
		}
#else
		wl_add_ie(0, 0, pktflag, len, (uchar *) OUI_ASUS, value);
#endif
	}
}

/**
 * @brief remove beacon vsie by unit and subunit
 *
 * @param unit band index
 * @param subunit mssid index
 * @param hexdata vsie string
 */
void del_beacon_vsie_by_unit(int unit, int subunit, char *hexdata)
{
	wl_del_ie_with_oui(unit, subunit, (uchar *) OUI_ASUS);
}

/**
 * @brief remove guest beacon vsie
 *
 * @param hexdata vsie string
 */
void del_beacon_vsie_guest(char *hexdata)
{
    int unit = 0, subunit = 0;
    char word[100], *next;
    char wl_ifnames[32] = { 0 };

    strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
    foreach (word, wl_ifnames, next) {
        if (nvram_get_int("re_mode") == 1)  // RE
            subunit = 3;
        else  // CAP/Router
            subunit = 2;
        for (; subunit <= num_of_mssid_support(unit); subunit++) {
			char buf[] = "wlXX.XX_ifname";
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf), "wl%d.%d_ifname", unit, subunit);
            if (is_intf_up(nvram_safe_get(buf)) != -1)  // interface exist
                del_beacon_vsie_by_unit(unit, subunit, hexdata);
        }
        unit++;
    }
}

void del_beacon_vsie(char *hexdata)
{
#ifdef RTCONFIG_BHCOST_OPT
	int unit = 0;
	char word[100], *next;
	char wl_ifnames[32] = { 0 };

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		wl_del_ie_with_oui(unit, 0, (uchar *) OUI_ASUS);
		unit++;
	}
#else
	wl_del_ie_with_oui(0, 0, (uchar *) OUI_ASUS);
#endif
}

int add_interface_for_acsd(int unit) {
    char wlc_status[] = "wlcXXX_status", amas_wl_noacsd[] = "amas_wlXXX_noacsd", wl_chsync[] = "wlXXX_chsync";
    int ret = 0;

    if (nvram_get_int("re_mode") == 1 && !nvram_get_int("wlready"))
        return 0;

    snprintf(wlc_status, sizeof(wlc_status), "wlc%d_status", get_wlc_bandindex_by_unit(unit));
    snprintf(amas_wl_noacsd, sizeof(amas_wl_noacsd), "amas_wl%d_noacsd", unit);
    snprintf(wl_chsync, sizeof(wl_chsync), "wl%d_chsync", unit);

    if (nvram_get_int(amas_wl_noacsd) == 1) {
        ret = 0;
    } else if (nvram_get(wlc_status)) {
        switch (nvram_get_int(wlc_status)) {
            case CH_SYNC_NO_USE:
            case CH_SYNC_NO_COONECT:
            case CH_SYNC_ETH_BHL:
            case CH_SYNC_WIFI_BHL:
                if (nvram_get_int(wl_chsync) == 0)
                    ret = 1;
                break;
            default:
                ret = 0;
                break;
        }
    }

    dbg("add_interface_for_acsd(%d), ret(%d)\n", unit, ret);
    return ret;
}

int need_to_start_acsd()
{
	int ret = 0, i = 0, max_items = 4;
	char *nv, *nvp, *b;

	nv = nvp = strdup(nvram_safe_get("sta_priority"));
	if (nv) {
		while ((b = strsep(&nvp, " ")) != NULL) {
			i++;

			if (i == max_items) {	/* use */
				if (atoi(b) == 0) {
					ret = 1;
					break;
				}

				/* reset count */
				i = 0;
			}

			/* Checking 6G band */
			if (i == 1 && atoi(b) == 6) {
				ret = 1;
				break;
			}

		}
		free(nv);
	}

	dbg("need_to_start_acsd(%d)\n", ret);

	return ret;
}

int get_wlan_service_status(int bssidx, int vifidx)
{
	char tmp[128] = {0}, prefix[] = "wlXXXXXXXXXX_";
	char ifname[IFNAMSIZ] = { 0 };
	int ret = 0;
	int result = 0;
	int bsscfg_idx = 0;
	char data_buf[WLC_IOCTL_MAXLEN];
	char wl_radio[] = "wlXXXX_radio";

	if (nvram_get_int("wlready") == 0)
		return -1;

	snprintf(wl_radio, sizeof(wl_radio), "wl%d_radio", bssidx);
	if (nvram_get_int(wl_radio) == 0)
		return -2;

	if (vifidx > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", bssidx, vifidx);
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", bssidx);

	strlcpy(ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(ifname));

	bsscfg_idx = htod32(vifidx);

	ret = wl_iovar_getbuf(ifname, "bss", &bsscfg_idx, sizeof(bsscfg_idx),
		data_buf, WLC_IOCTL_MAXLEN);
	if (ret < 0) {
		dbg("failed to get bss on %s\n", ifname);
		return -1;
	}

	result = *(int*)data_buf;
        result = dtoh32(result);

	//dbg("result: %d\n", result);

	return result;
}

void set_wlan_service_status(int bssidx, int vifidx, int enabled)
{

	char tmp[128]={0}, prefix[] = "wlXXXXXXXXXX_";
	char ifname[IFNAMSIZ] = { 0 };
	int val, ret;
	struct ether_addr addr = {{255, 255, 255, 255, 255, 255}};
	struct {int bsscfg_idx; int enable;} setbuf;
	char wl_radio[] = "wlXXXX_radio";

#ifdef RTCONFIG_BRCM_HOSTAPD
	int hapd_is_ready = 0;
#endif	// RTCONFIG_BRCM_HOSTAPD

        if (nvram_get_int("wlready") == 0)
                return;

	snprintf(wl_radio, sizeof(wl_radio), "wl%d_radio", bssidx);
	if (nvram_get_int(wl_radio) == 0)
		return;

	if (vifidx > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", bssidx, vifidx);
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", bssidx);

	strlcpy(ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(ifname));
	if (ifname == NULL || strlen(ifname) == 0) {
		_dprintf("Getting bssidx(%d) vifidx(%d) ifname fail.\n", bssidx, vifidx);
		return;
	}

#ifdef RTCONFIG_BRCM_HOSTAPD
        FILE *fp1 = NULL;
        char cmd1[64], buf1[256];
        if(nvram_match("hapd_enable", "1")) {
                snprintf(cmd1, sizeof(cmd1), "hostapd_cli -i %s ping", ifname);
                fp1 = popen(cmd1, "r");
                if(fp1) {
                        while (fgets(buf1, sizeof(buf1), fp1) != NULL) {
                                if(strstr(buf1, "PONG") != NULL)
                                {
                                    hapd_is_ready = 1;
                                    break;
                                }
                        }
                        pclose(fp1);
                }
        }
#endif	// RTCONFIG_BRCM_HOSTAPD

#ifdef RTCONFIG_BRCM_HOSTAPD
	if (hapd_is_ready) {
#endif	// RTCONFIG_BRCM_HOSTAPD

	if (enabled == 0) {

		/* deauthe all sta */
		if (wl_ioctl(ifname, WLC_SCB_DEAUTHENTICATE, &addr, ETHER_ADDR_LEN) < 0) {
			dbg("deauth all sta failed on %s\n", ifname);
		}
#if defined(RTCONFIG_HND_ROUTER_AX)
                val = 0;
#elif defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
                val = 9;
#else
                val = 0;
#endif
	}
	else
	{
#if defined(RTCONFIG_HND_ROUTER_AX)
                val = 1;
#elif defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
                val = 8;
#else
                val = 1;
#endif
	}

	setbuf.bsscfg_idx = htod32(vifidx);
	setbuf.enable = htod32(val);

	dbg("set bss to %d on %s\n", val, ifname);

#if defined(RTAC86U) || defined(GTAC2900)
	int isup = 0;
	char buf[16] = { 0 };
	int bsscfg_idx = htod32(vifidx);

	if (!wl_iovar_getbuf(ifname, "bss", &bsscfg_idx, sizeof(bsscfg_idx), buf, sizeof(buf))) {
		isup = *(int *) buf;
		isup = dtoh32(isup);
		dbg("ifname %s bsscfg_idx %d bss %s\n", ifname, bsscfg_idx, isup ? "up" : "down");
		if (!isup) {
			dbg("reinit ifname %s\n", ifname);
			doSystem("wl -i %s reinit", ifname);
		}
	}
#endif

	ret = wl_iovar_set(ifname, "bss", &setbuf, sizeof(setbuf));
	if (ret) {
		dbg("failed to set bss to %d on %s\n", val, ifname);
	}

#ifdef RTCONFIG_BRCM_HOSTAPD
	} else {
		dbg("%s: Hostapd not running\n", ifname);
	}
#endif	// RTCONFIG_BRCM_HOSTAPD


#ifdef RTCONFIG_BRCM_HOSTAPD
	FILE *fp = NULL;
	char cmd[64], buf[256];
	int hapd_is_running = 0;
	if(nvram_match("hapd_enable", "1")) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli -i %s ping", ifname);
		fp = popen(cmd, "r");
		if(fp) {
			while (fgets(buf, sizeof(buf), fp) != NULL) {
				if(strstr(buf, "PONG") != NULL)
				{
				    hapd_is_running = 1;
				    break;
				}
			}
			pclose(fp);
		}

		// restart hostapd in case previous start hostapd operation is unsuccess since bss is down
		if(!hapd_is_running && enabled) {
			snprintf(cmd, sizeof(cmd), "hostapd %s /tmp/%s_hapd.conf &",
				(nvram_match("hapd_dbg", "1") ? "-ddt" : "-B"), ifname);
			system(cmd);
		}
	}
#endif
}

#ifdef RTCONFIG_BHCOST_OPT
unsigned int test_get_uplinkports_linkrate(char *ifname)
{
	unsigned int link_rate = 1000;

	//TODO for getting link rate

	return link_rate;
}

#if defined(RTCONFIG_EXT_RTL8370MB)
#define MAX_RTL_PORTS 8
#else
#define MAX_RTL_PORTS 4
#endif

#if defined(EBG19)
unsigned int
get_uplinkports_linkrate(char *ifname)
{
	return hnd_get_phy_speed(ifname);
}
#elif defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
unsigned int
get_uplinkports_linkrate(char *ifname)
{
	int i, ret;
	char out_buf[64];
	int len;
	int verbose = nvram_get_int("verbose");
	int lan_ports=4;
	int ports[lan_ports+1];
	int lrate[lan_ports+1];
	char pif[lan_ports+1][8];

	for (i=0; i<lan_ports+1; i++) {
		lrate[i] = 0;
		sprintf(pif[i], "%s", "X");
	}

	int model, mask;

	model = get_model();
	switch(model) {
	case MODEL_RTAX55:
	case MODEL_RTAX58U_V2:
	case MODEL_RTAX3000N:
	case MODEL_BR63:
#if defined(RTAX1800) || defined(BR63) && defined(NEW_SWITCH_ORDER)
		/* WAN L4 L3 L2 L1 */
		ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3; ports[4]=4;

		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth1");
		sprintf(pif[2], "%s", "eth1");
		sprintf(pif[3], "%s", "eth1");
		sprintf(pif[4], "%s", "eth1");
#else
		/* WAN L1 L2 L3 L4 */
		ports[0]=0; ports[1]=4; ports[2]=3; ports[3]=2; ports[4]=1;

		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth1");
		sprintf(pif[2], "%s", "eth1");
		sprintf(pif[3], "%s", "eth1");
		sprintf(pif[4], "%s", "eth1");
#endif
		break;
	default:
		sprintf(pif[0], "%s", "eth0");
		break;
	}

	memset(out_buf, 0, 64);
	len = 0;
	for (i=0; i<lan_ports+1; i++) {
		mask = 0;
		mask |= 0x0001<<ports[i];

		if (hnd_get_phy_status(ports[i])==0) /*Disconnect*/
		{
			if (i==0) {
				len  = sprintf(out_buf, "W0=X;");
				lrate[0] = 0;
			} else {
				len += sprintf(out_buf + len, "L%d=X;", i);
				lrate[i] = 0;
			}
		}
		else { /*Connect, keep check speed*/
			mask = 0;
			mask |= (0x0003<<(ports[i]*2));
			ret = hnd_get_phy_speed(ports[i]);
			if (i==0) {
				len = sprintf(out_buf, "W0=%s;", (ret == 2500) ? "Q" : ((ret == 1000) ? "G" : "M"));
				lrate[i] = ret;
			}
			else {
				len += sprintf(out_buf + len, "L%d=%s;", i, (ret == 2500) ? "Q" : ((ret == 1000) ? "G" : "M"));
				lrate[i] = ret;
			}
		}
	}

	if(verbose) {
		for( i=0; i<lan_ports+1 && strcmp(pif[i], "X"); ++i)
			printf("[%d] Portif=%s, lrate=%d\n", i, pif[i], lrate[i]);
		printf("\n");
	}

	for( i=0; i<lan_ports+1; ++i) {
		if(strcmp(pif[i], ifname) == 0)
			return lrate[i];
	}

	return 0;
}
#else
unsigned int
get_uplinkports_linkrate(char *ifname)
{
	int i, ret;
	char out_buf[64];
	int lret=0;
	int len;
	int verbose = nvram_get_int("verbose");
#if defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(BCM4912) || defined(BCM6756) || defined(BCM4906_504)
	int lan_ports = 16;
	int lrate[lan_ports+1];
	char pif[lan_ports+1][8];
	char word[256], *next;
#if defined(GTAXE11000)
	char lanports_seq[64] = {"eth1 eth4 eth2 eth3 eth5"};   /* L1 L2 L3 L4 L5 */
#elif defined(RTAX86U)
	char *lanports_seq;
	char lanports_seq1[64] = {"eth4 eth3 eth2 eth1 eth5"};   /* L1 L2 L3 L4 L5 */
	char lanports_seq2[64] = {"eth4 eth3 eth2 eth1"};   /* L1 L2 L3 L4 */

	if(!strcmp(get_productid(), "RT-AX86S"))
		lanports_seq = lanports_seq2;
	else
		lanports_seq = lanports_seq1;
#elif defined(RTAX68U)
	char lanports_seq[64] = {"eth4 eth3 eth2 eth1"};   /* L1 L2 L3 L4 */
#elif defined(RTAX86U_PRO)
	char lanports_seq[64] = {"eth1 eth2 eth3 eth4 eth5"};   /* L1 L2 L3 L4 L5 */
#elif defined(GTAX6000) || defined(RTAX88U_PRO)
	char lanports_seq[64] = {"eth1 eth2 eth3 eth4 eth5"};   /* L1 L2 L3 L4 L5 */
#elif defined(GTAX11000_PRO)
	char lanports_seq[64] = {"eth1 eth2 eth3 eth4 eth5"};   /* L1 L2 L3 L4 L5 */
#elif defined(GTAXE16000)
	char lanports_seq[64] = {"eth1 eth2 eth3 eth4 eth5 eth6"};   /* L1 L2 L3 L4 L5 L6 */
#elif defined(ET12) || defined(XT12)
	char lanports_seq[64] = {"eth1 eth2 eth3"};   /* L1 L2 L3 */
#endif

	for (i=0; i<lan_ports+1; i++) {
		lrate[i] = 0;
		sprintf(pif[i], "%s", "X");
	}

	foreach(word, nvram_safe_get("wan_ifname"), next){
		ret = hnd_get_phy_status(word);
		if(ret == 0) {
			sprintf(out_buf, "W0=X;");
			lrate[0] = 0;
		} else {
			ret = hnd_get_phy_speed(word);
			sprintf(out_buf, "W0=%s;",
#ifdef RTCONFIG_EXTPHY_BCM84880
					(ret == 2500)? "Q" :
#endif
							(ret == 1000) ? "G" : "M");
			lrate[0] = (ret == 2500)? 2500 : (ret == 1000) ? 1000 : 100;
			lret |= 1;
		}

		break;
	}

	// original WAN port
	if(!is_router_mode() && (!*nvram_safe_get("wan_ifname"))) {  // ap/re mode
		if(verbose)
			_dprintf("set first if as %s\n", WAN_IF_ETH);
		sprintf(pif[0], "%s", WAN_IF_ETH);	// here the report follows lan_ifnames
		ret = hnd_get_phy_status(pif[0]);
		if(ret == 0) {
			lrate[0] = 0;
		} else {
			ret = hnd_get_phy_speed(pif[0]);
			lrate[0] = (ret == 10000) ? 10000 : ((ret == 5000) ? 5000 : ((ret == 2500) ? 2500 : ((ret == 1000) ? 1000 : 100)));
		}
	}
	i = 1;

	// LAN ports
	len = strlen(out_buf);
#if defined(GTAXE11000) || defined(GTAX6000) || defined(RTAX88U_PRO) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(ET12) || defined(XT12) \
		|| defined(RTAX86U) || defined(RTAX68U) || defined(RTAX86U_PRO)
	foreach(word, lanports_seq, next)
#else
	foreach(word, nvram_safe_get("wired_ifnames"), next)
#endif
	{
		if (!wl_probe(word))		// skip wireless interface
			continue;
		if( i > lan_ports ) {
			if(verbose)
				_dprintf("out of ifnum\n");
			break;
		}
		sprintf(pif[i], "%s", word);	// here the report follows lan_ifnames
		ret = hnd_get_phy_status(word);
		if(ret == 0) {
			len += sprintf(out_buf + len, "L%d=X;", i);
			lrate[i] = 0;
		} else{
			ret = hnd_get_phy_speed(word);
			len += sprintf(out_buf + len, "L%d=%s;", i,
					(ret == 10000) ? "T" : ((ret == 5000) ? "H" : ((ret == 2500)? "Q" :((ret == 1000) ? "G" : "M"))));
			lret |= 1 << i;
			lrate[i] = (ret == 10000) ? 10000 : ((ret == 5000) ? 5000 : ((ret == 2500) ? 2500 : ((ret == 1000) ? 1000 : 100)));
		}

		++i;
	}

	//return lret > 0 ? lret : 0;
#else // RTCONFIG_HND_ROUTER_AX_6710
#if defined(HND_ROUTER) && !defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(BCM6855) && !defined(BCM6750)
	unsigned int regv=0, pmdv=0, regv2=0, pmdv2=0;
#endif
#ifdef RTCONFIG_EXT_BCM53134
#if defined(RTAX95Q) || defined(BM68) || defined(RTAXE95Q)
	int lan_ports=3;
#elif defined(RTAX56_XD4) || defined(CTAX56_XD4)
	int lan_ports=1;
#elif defined(RPAX56)
        int lan_ports=0;
#elif defined(RTAX56U)
	int lan_ports=4;
#else
	int lan_ports=8;
#endif
#elif defined(RTCONFIG_EXTPHY_BCM84880)
	int lan_ports=5;
#elif defined(RTAX82_XD6) || defined(XD6_V2) || defined(GT10)
	int lan_ports=3;
#elif defined(RTAX82_XD6S)
	int lan_ports=1;
#else
	int lan_ports=4;
#endif

#if defined(RTAX56_XD4)
	if(nvram_match("HwId", "A") || nvram_match("HwId", "C")){
		lan_ports = 1;
	} else {
		lan_ports = 0;
	}
#endif
	int ports[lan_ports+1];
	int lrate[lan_ports+1];
	char pif[lan_ports+1][8];
#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
	int ext = 0;
	int exrate[MAX_RTL_PORTS];
	char ex_pif[MAX_RTL_PORTS][8];
#endif
	for (i=0; i<lan_ports+1; i++) {
		lrate[i] = 0;
		sprintf(pif[i], "%s", "X");
	}
#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
	for (i=0; i<MAX_RTL_PORTS; i++) {
		exrate[i] = 0;
		sprintf(ex_pif[i], "%s", "X");
	}
#endif
	int ext_lret=0, model, mask;
	int extra_p0=0;

	model = get_model();
	switch(model) {
#ifndef HND_ROUTER
	case MODEL_RTN14UHP:
		/* WAN L1 L2 L3 L4 */
		ports[0]=4; ports[1]=0; ports[2]=1, ports[3]=2; ports[4]=3;
		sprintf(pif[0], "%s", "eth0");
		break;
	case MODEL_RTN53:
	case MODEL_RTN15U:
	case MODEL_RTN12:
	case MODEL_RTN12B1:
	case MODEL_RTN12C1:
	case MODEL_RTN12D1:
	case MODEL_RTN12VP:
	case MODEL_RTN12HP:
	case MODEL_RTN12HP_B1:
	case MODEL_APN12HP:
	case MODEL_RTN10P:
	case MODEL_RTN10D1:
	case MODEL_RTN10PV2:
		/* WAN L1 L2 L3 L4 */
		ports[0]=4; ports[1]=3; ports[2]=2, ports[3]=1; ports[4]=0;
		sprintf(pif[0], "%s", "eth0");
		break;
	case MODEL_RTN16:
	case MODEL_RTN10U:
		/* WAN L1 L2 L3 L4 */
		ports[0]=0; ports[1]=4; ports[2]=3, ports[3]=2; ports[4]=1;
		sprintf(pif[0], "%s", "eth0");
		break;
	case MODEL_RTAC88U:
	case MODEL_RTAC3100:
		/* WAN L1 L2 L3 L4 */
		ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
		ext = 1;
#endif
		sprintf(pif[0], "%s", "vlan2");
		break;
	case MODEL_RTAC56S:
	case MODEL_RTAC56U:
		/* WAN L1 L2 L3 L4 */
		ports[0]=4; ports[1]=0; ports[2]=1; ports[3]=2; ports[4]=3;
		sprintf(pif[0], "%s", "eth0");
		break;

	case MODEL_RTAC87U:
		/* WAN L1 L2 L3 L4 */
		ports[0]=0; ports[1]=5; ports[2]=3; ports[3]=2; ports[4]=1;
		sprintf(pif[0], "%s", "eth0");
		break;

	case MODEL_DSLAC68U:
	case MODEL_RTAC68U:
	case MODEL_RTAC3200:
	case MODEL_RTN18U:
	case MODEL_RTAC53U:
	case MODEL_RTN66U:
	case MODEL_RTAC66U:
	case MODEL_RTAC1200G:
	case MODEL_RTAC1200GP:
		/* WAN L1 L2 L3 L4 */
		ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3; ports[4]=4;
		sprintf(pif[0], "%s", "vlan2");
		sprintf(pif[1], "%s", "");
		sprintf(pif[2], "%s", "");
		sprintf(pif[3], "%s", "");
		sprintf(pif[4], "%s", "");
		break;
	case MODEL_RTAC5300:
		/* WAN L1 L2 L3 L4 */
		ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3; ports[4]=4;
#ifdef RTCONFIG_EXT_RTL8365MB
		ext = 1;
#endif
		sprintf(pif[0], "%s", "vlan2");
		sprintf(pif[1], "%s", "");
		sprintf(pif[2], "%s", "");
		sprintf(pif[3], "%s", "");
		sprintf(pif[4], "%s", "");
		break;
#else
#if !defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(BCM6855) && !defined(BCM6750)
	case MODEL_RTAC86U:
		/* WAN L4 L3 L2 L1 */
		ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
		regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0);
		regv2 = hnd_ethswctl(REGACCESS, 0x0104, 4, 0, 0);
//		printf("phystatus: [%x][%x]\n", regv, regv2);
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth4");
		sprintf(pif[2], "%s", "eth3");
		sprintf(pif[3], "%s", "eth2");
		sprintf(pif[4], "%s", "eth1");
		break;
	case MODEL_GTAC5300:
		/*
			  1 0 s3 s2	   L1 L2 L3 L4
			7 3 2 s1 s0	W0 L5 L6 L7 L8
 		 */
		extra_p0 = S_53134;
		ports[0]=7; ports[1]=1; ports[2]=0; ports[3]=3+extra_p0; ports[4]=2+extra_p0;
		ports[5]=3; ports[6]=2; ports[7]=1+extra_p0; ports[8]=extra_p0;
		regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
		pmdv = hnd_ethswctl(PMDIOACCESS, 0x0100, 2, 0, 0);
#endif
		regv2 = hnd_ethswctl(REGACCESS, 0x0104, 4, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
		pmdv2 = hnd_ethswctl(PMDIOACCESS, 0x0104, 4, 0, 0);
#endif
//		printf("phystatus: [%x][%x][%x][%x]\n", regv, pmdv, regv2, pmdv2);
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth4");
		sprintf(pif[2], "%s", "eth3");
		sprintf(pif[3], "%s", "eth2");
		sprintf(pif[4], "%s", "eth1");
		sprintf(pif[5], "%s", "eth5");
		sprintf(pif[6], "%s", "eth5");
		sprintf(pif[7], "%s", "eth5");
		sprintf(pif[8], "%s", "eth5");
		break;
	case MODEL_GTAX11000:
#ifdef RTCONFIG_EXT_BCM53134
		/*
			  1 0 s3 s2	   L1 L2 L3 L4
			7 3 2 s1 s0	W0 L5 L6 L7 L8
 		 */
		extra_p0 = S_53134;
		ports[0]=7; ports[1]=1; ports[2]=0; ports[3]=3+extra_p0; ports[4]=2+extra_p0;
		ports[5]=3; ports[6]=2; ports[7]=1+extra_p0; ports[8]=extra_p0;
#elif defined(RTCONFIG_EXTPHY_BCM84880)
		/*
			7 4 3 2 1 0 	L5(2.5G) W0 L1 L2 L3 L4
		*/
		ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
		ports[5]=7;
#endif
		regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
		pmdv = hnd_ethswctl(PMDIOACCESS, 0x0100, 2, 0, 0);
#endif
		regv2 = hnd_ethswctl(REGACCESS, 0x0104, 4, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
		pmdv2 = hnd_ethswctl(PMDIOACCESS, 0x0104, 4, 0, 0);
#endif
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth4");
		sprintf(pif[2], "%s", "eth3");
		sprintf(pif[3], "%s", "eth2");
		sprintf(pif[4], "%s", "eth1");
		sprintf(pif[5], "%s", "eth5");
		break;
	case MODEL_EBG15:
	case MODEL_EBP15:
		/*
			7 3 2 1 0	W0 L1 L2 L3 L4
 		 */
		ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
		regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
		pmdv = hnd_ethswctl(PMDIOACCESS, 0x0100, 2, 0, 0);
#endif
		regv2 = hnd_ethswctl(REGACCESS, 0x0104, 4, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
		pmdv2 = hnd_ethswctl(PMDIOACCESS, 0x0104, 4, 0, 0);
#endif
//		printf("phystatus: [%x][%x][%x][%x]\n", regv, pmdv, regv2, pmdv2);

		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth4");
		sprintf(pif[2], "%s", "eth3");
		sprintf(pif[3], "%s", "eth2");
		sprintf(pif[4], "%s", "eth1");

		break;
	case MODEL_BC109:
	case MODEL_EBG19:
	case MODEL_RTAX88U:
		/*
			7 3 2 1 0 s3 s2 s1 s0	W0 L1 L2 L3 L4 L5 L6 L7 L8
 		 */
		/*
			? P7 shall be for ebg19 eth5      :   Up   1 Gbps 00:1F:C6:27:1A:C9 ExtSw:P7 Lgcl:15 LAN
					  ax88u eth5: <Ext sw port: 7> <Logical : 15> MAC : 7C:10:C9:E0:4F:C8
		*/
		extra_p0 = S_53134;
		ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
		ports[5]=3+extra_p0; ports[6]=2+extra_p0; ports[7]=1+extra_p0; ports[8]=extra_p0;
		regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
		pmdv = hnd_ethswctl(PMDIOACCESS, 0x0100, 2, 0, 0);
#endif
		regv2 = hnd_ethswctl(REGACCESS, 0x0104, 4, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
		pmdv2 = hnd_ethswctl(PMDIOACCESS, 0x0104, 4, 0, 0);
#endif
//		printf("phystatus: [%x][%x][%x][%x]\n", regv, pmdv, regv2, pmdv2);

		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth4");
		sprintf(pif[2], "%s", "eth3");
		sprintf(pif[3], "%s", "eth2");
		sprintf(pif[4], "%s", "eth1");
		sprintf(pif[5], "%s", "eth5");
		sprintf(pif[6], "%s", "eth5");
		sprintf(pif[7], "%s", "eth5");
		sprintf(pif[8], "%s", "eth5");
		break;
	case MODEL_RTAX92U:
		/* WAN L4 L3 L2 L1 */
		ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
		regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0) & 0xf;
		regv2 = hnd_ethswctl(REGACCESS, 0x0104, 4, 0, 0);
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth4");
		sprintf(pif[2], "%s", "eth3");
		sprintf(pif[3], "%s", "eth2");
		sprintf(pif[4], "%s", "eth1");
		break;
#else	// RTCONFIG_HND_ROUTER_AX_675X
	case MODEL_RTAX95Q:
	case MODEL_BM68:
	case MODEL_RTAXE95Q:
		/*
			0 1 2 3 W0 L1 L2 L3
 		 */
		ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3;
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth1");
		sprintf(pif[2], "%s", "eth2");
		sprintf(pif[3], "%s", "eth3");
		break;
	case MODEL_RTAX56_XD4:
		/*
			0 1 W0 L1
 		 */
		if(nvram_match("HwId", "A") || nvram_match("HwId", "C")){
			ports[0]=0; ports[1]=1;
		} else {
			ports[0]=0;;
		}
		sprintf(pif[0], "%s", "eth0");
		break;
	case MODEL_CTAX56_XD4:
		/*
			0 1 W0 L1
		 */
		ports[0]=0; ports[1]=1;
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth1");
		break;
	case MODEL_DSLAX82U:
		/* WAN L4 L3 L2 L1 */
		ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
		sprintf(pif[0], "%s", "eth4");
		sprintf(pif[1], "%s", "eth3");
		sprintf(pif[2], "%s", "eth2");
		sprintf(pif[3], "%s", "eth1");
		sprintf(pif[4], "%s", "eth0");
		break;
	case MODEL_RTAX58U:
	case MODEL_RTAX82U_V2:
	case MODEL_TUFAX5400_V2:
	case MODEL_RTAX5400:
	case MODEL_XD6_V2:
#if defined(RTAX82_XD6) || defined(XD6_V2)
		/* WAN L1 L2 L3 */
		ports[0]=4; ports[1]=2; ports[2]=1; ports[3]=0;

		sprintf(pif[0], "%s", "eth4");
		sprintf(pif[1], "%s", "eth3");
		sprintf(pif[2], "%s", "eth2");
		sprintf(pif[3], "%s", "eth1");
#else
		/* WAN L1 L2 L3 L4 */
		ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;

		sprintf(pif[0], "%s", "eth4");
		sprintf(pif[1], "%s", "eth3");
		sprintf(pif[2], "%s", "eth2");
		sprintf(pif[3], "%s", "eth1");
		sprintf(pif[4], "%s", "eth0");
#endif
		break;
	case MODEL_RTAX82_XD6S:
		/* WAN L1 */
		ports[0]=1; ports[1]=0;

		sprintf(pif[0], "%s", "eth1");
		sprintf(pif[1], "%s", "eth0");
		break;
	case MODEL_GT10:
		if (!nvram_get_int("wans_extwan")) {
		/* 0 1 2 3 W0 L1 L2 L3 */
		ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3;
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth1");
		sprintf(pif[2], "%s", "eth2");
		sprintf(pif[3], "%s", "eth3");
		} else {
		/* 1 0 2 3 W0 L1 L2 L3 */
		ports[0]=1; ports[1]=0; ports[2]=2; ports[3]=3;
		sprintf(pif[0], "%s", "eth1");
		sprintf(pif[1], "%s", "eth0");
		sprintf(pif[2], "%s", "eth2");
		sprintf(pif[3], "%s", "eth3");
		}
		break;
	case MODEL_RTAX9000:
		/* 0 1 2 3 4 5 W0 L1 L2 L3 L4 L5 */
		ports[0]=0; ports[1]=5; ports[2]=1; ports[3]=2; ports[4]=3; ports[5]=4;
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth1");
		sprintf(pif[2], "%s", "eth2");
		sprintf(pif[3], "%s", "eth3");
		sprintf(pif[4], "%s", "eth4");
		sprintf(pif[5], "%s", "eth5");
		break;
	case MODEL_RTAX55:
	case MODEL_RTAX58U_V2:
	case MODEL_RTAX3000N:
	case MODEL_BR63:
#if defined(RTAX1800) || defined(BR63) && defined(NEW_SWITCH_ORDER)
		/* WAN L4 L3 L2 L1 */
		ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3; ports[4]=4;

		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth1");
		sprintf(pif[2], "%s", "eth1");
		sprintf(pif[3], "%s", "eth1");
		sprintf(pif[4], "%s", "eth1");
#else
		/* WAN L1 L2 L3 L4 */
		ports[0]=0; ports[1]=4; ports[2]=3; ports[3]=2; ports[4]=1;

		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth1");
		sprintf(pif[2], "%s", "eth1");
		sprintf(pif[3], "%s", "eth1");
		sprintf(pif[4], "%s", "eth1");
#endif
		break;
	case MODEL_RTAX56U:
		/* WAN L4 L3 L2 L1 */
		ports[0]=0; ports[1]=4; ports[2]=3; ports[3]=2; ports[4]=1;
		sprintf(pif[0], "%s", "eth0");
		sprintf(pif[1], "%s", "eth4");
		sprintf(pif[2], "%s", "eth3");
		sprintf(pif[3], "%s", "eth2");
		sprintf(pif[4], "%s", "eth1");
		break;
	case MODEL_RPAX56:
		/* LAN */
		ports[0]=0;
		sprintf(pif[0], "%s", "eth0");
		break;

#endif	// RTCONFIG_HND_ROUTER_AX_675X
#endif	// HND_ROUTER
	default:
		sprintf(pif[0], "%s", "eth0");
		break;
	}

	memset(out_buf, 0, 64);
	len = 0;
	for (i=0; i<lan_ports+1; i++) {
		mask = 0;
		mask |= 0x0001<<ports[i];

#ifndef HND_ROUTER
		if (get_phy_status(mask)==0) /*Disconnect*/
#else
#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(BCM6855) || defined(BCM6750)
		if (hnd_get_phy_status(ports[i])==0) /*Disconnect*/
#else
		if (hnd_get_phy_status(ports[i], extra_p0, regv, pmdv)==0) /*Disconnect*/
#endif
#endif
		{
			if (i==0) {
				len  = sprintf(out_buf, "W0=X;");
				lrate[0] = 0;
			} else {
				len += sprintf(out_buf + len, "L%d=X;", i);
				lrate[i] = 0;
			}
		}
		else { /*Connect, keep check speed*/
			mask = 0;
			mask |= (0x0003<<(ports[i]*2));
#ifndef HND_ROUTER
			ret=get_phy_speed(mask);
			ret>>=(ports[i]*2);
#else
#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(BCM6855) || defined(BCM6750)
			ret = hnd_get_phy_speed(ports[i]);
#else
			ret = hnd_get_phy_speed(ports[i], extra_p0, regv2, pmdv2);
#endif
#endif
			if (i==0) {
#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(BCM6855) || defined(BCM6750)
				len = sprintf(out_buf, "W0=%s;", (ret == 2500) ? "Q" : ((ret == 1000) ? "G" : "M"));
				lrate[i] = ret;
#else
				len = sprintf(out_buf, "W0=%s;",
#ifdef RTCONFIG_EXTPHY_BCM84880
						(ret & 4)? "Q" :
#endif
						(ret & 2)
						? "G" : "M");
				lrate[i] = (ret&4)?2500:(ret&2)?1000:100;
#endif
#ifdef HND_ROUTER
				lret |= 1 << i;
#endif
			}
			else {
				lret |= 1 << i;

				if (ports[i] >= extra_p0)
					ext_lret = 1;

#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(BCM6855) || defined(BCM6750)
				len += sprintf(out_buf + len, "L%d=%s;", i, (ret == 2500) ? "Q" : ((ret == 1000) ? "G" : "M"));
				lrate[i] = ret;
#else
				len += sprintf(out_buf + len, "L%d=%s;", i,
#ifdef RTCONFIG_EXTPHY_BCM84880
					(ret & 4)? "Q" :
#endif
					(ret & 2)
					? "G" : "M");
				lrate[i] = (ret&4)?2500:(ret&2)?1000:100;
#endif
			}
		}
	}
#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
	if (ext) {
		ext_lret = ext_rtk_phyState(0, &exrate[0], NULL);
		if(ext_lret) lret |= ext_lret << (lan_ports+1);
	}
#endif

#endif // RTCONFIG_HND_ROUTER_AX_6710

	if(verbose) {
		for( i=0; i<lan_ports+1 && strcmp(pif[i], "X"); ++i)
			printf("[%d] Portif=%s, lrate=%d\n", i, pif[i], lrate[i]);
		printf("\n");
#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
		for( i=0; i<MAX_RTL_PORTS; ++i)
			printf("[%d] exif=%s, exrate=%d\n", i, ex_pif[i], exrate[i]);
		printf("\n");
#endif
	}

	for( i=0; i<lan_ports+1; ++i) {
		if(strcmp(pif[i], ifname) == 0)
			return lrate[i];
	}
#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
	for( i=0; i<MAX_RTL_PORTS; ++i) {
		if(strcmp(ex_pif[i], ifname) == 0)
			return exrate[i];
	}
#endif

	return 0;
}
#endif	// defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
#endif	/* RTCONFIG_BHCOST_OPT */
#endif  /* RTCONFIG_AMAS */

#ifdef RTCONFIG_CFGSYNC
void update_macfilter_relist()
{
	char maclist_buf[4096] = {0};
	struct maclist *maclist = NULL;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char word[256], *next;
	char mac2g[32], mac5g[32], mac6g[32], *next_mac;
	int unit = 0;
	char wlif_name[IFNAMSIZ] = { 0 };
	struct ether_addr *ea;
	unsigned char sta_ea[6] = {0};
	int ret = 0;
	char *nv, *nvp, *b;
	char *reMac, *maclist2g, *maclist5g, *timestamp;
	char *maclist6g, *reserved1, *reserved2;
	char stamac2g[18] = {0}, stamac5g[18] = {0}, stamac6g[18] = {0};
	char wl_ifnames[32] = { 0 };
	int nband = 0;

	if (is_cfg_relist_exist())
	{
#ifdef RTCONFIG_AMAS
		if (nvram_get_int("re_mode") == 1) {
			/* in cfg_relist */
			nv = nvp = get_cfg_relist(0);
			if (nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if ((vstrsep(b, ">", &reMac, &maclist2g, &maclist5g, &timestamp) != 4))
						continue;
					/* first mac for sta 2g of dut */
					foreach_44 (mac2g, maclist2g, next_mac)
						break;
					/* first mac for sta 5g of dut */
					foreach_44 (mac5g, maclist5g, next_mac)
						break;

					if (strcmp(reMac, get_lan_hwaddr()) == 0) {
						snprintf(stamac2g, sizeof(stamac2g), "%s", mac2g);
						dbg("dut 2g sta (%s)\n", stamac2g);
						snprintf(stamac5g, sizeof(stamac5g), "%s", mac5g);
						dbg("dut 5g sta (%s)\n", stamac5g);
						break;
					}
				}
				free(nv);
			}

			/* in cfg_relist_x */
			nv = nvp = get_cfg_relist(1);
			if (nv) {
				while ((b = strsep(&nvp, "<")) != NULL) {
					if ((vstrsep(b, ">", &reMac, &maclist6g, &reserved1, &reserved2) != 4))
						continue;
					/* first mac for sta 6g of dut */
					foreach_44 (mac6g, maclist6g, next_mac)
						break;

					if (strcmp(reMac, get_lan_hwaddr()) == 0) {
						if (strlen(mac6g)) {
							snprintf(stamac6g, sizeof(stamac6g), "%s", mac6g);
							dbg("dut 6g sta (%s)\n", stamac6g);
							break;
						}
					}
				}
				free(nv);
			}
		}
#endif

		strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
		foreach (word, wl_ifnames, next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(unit);

#ifdef RTCONFIG_AMAS
			if (nvram_get_int("re_mode") == 1)
				snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
			else
#endif
				snprintf(prefix, sizeof(prefix), "wl%d_", unit);

			strlcpy(wlif_name, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(wlif_name));
			maclist = (struct maclist *)maclist_buf;
			memset(maclist_buf, 0, sizeof(maclist_buf));
			ea = &(maclist->ea[0]);

			if (nvram_match(strcat_r(prefix, "macmode", tmp), "allow")) {
				nv = nvp = strdup(nvram_safe_get(strcat_r(prefix, "maclist_x", tmp)));
				if (nv) {
					while ((b = strsep(&nvp, "<")) != NULL) {
						if (strlen(b) == 0) continue;

#ifdef RTCONFIG_AMAS
						if (nvram_get_int("re_mode") == 1) {
							if (strcmp(b, stamac2g) == 0 ||
								strcmp(b, stamac5g) == 0 ||
								strcmp(b, stamac6g) == 0)
								continue;
						}
#endif
						dbg("maclist sta (%s) in %s\n", b, wlif_name);
						ether_atoe(b, sta_ea);
						memcpy(ea, sta_ea, sizeof(struct ether_addr));
						maclist->count++;
						ea++;
					}
					free(nv);
				}

				snprintf(prefix, sizeof(prefix), "wl%d_", unit);
				nband = nvram_get_int(strcat_r(prefix, "nband", tmp));

				/* in cfg_relist */
				nv = nvp = get_cfg_relist(0);
				if (nv) {
					while ((b = strsep(&nvp, "<")) != NULL) {
						if ((vstrsep(b, ">", &reMac, &maclist2g, &maclist5g, &timestamp) != 4))
							continue;

						if (strcmp(reMac, get_lan_hwaddr()) == 0)
							continue;

						if (nband == 2) {
							foreach_44 (mac2g, maclist2g, next_mac) {
								if (check_re_in_macfilter(unit, mac2g))
									continue;
								dbg("relist sta (%s) in %s\n", mac2g, wlif_name);
								ether_atoe(mac2g, sta_ea);
								memcpy(ea, sta_ea, sizeof(struct ether_addr));
								maclist->count++;
								ea++;
							}
						}
						else if (nband == 1)
						{
							foreach_44 (mac5g, maclist5g, next_mac) {
								if (check_re_in_macfilter(unit, mac5g))
									continue;
								dbg("relist sta (%s) in %s\n", mac5g, wlif_name);
								ether_atoe(mac5g, sta_ea);
								memcpy(ea, sta_ea, sizeof(struct ether_addr));
								maclist->count++;
								ea++;
							}
						}
					}
					free(nv);
				}

				/* in cfg_relist_x */
				nv = nvp = get_cfg_relist(1);
				if (nv) {
					while ((b = strsep(&nvp, "<")) != NULL) {
						if ((vstrsep(b, ">", &reMac, &maclist6g, &reserved1, &reserved2) != 4))
							continue;

						if (strcmp(reMac, get_lan_hwaddr()) == 0)
							continue;

						if (nband == 4) {
							foreach_44 (mac6g, maclist6g, next_mac) {
								if (check_re_in_macfilter(unit, mac6g))
									continue;
								dbg("relist sta (%s) in %s\n", mac6g, wlif_name);
								ether_atoe(mac6g, sta_ea);
								memcpy(ea, sta_ea, sizeof(struct ether_addr));
								maclist->count++;
								ea++;
							}
						}
					}
					free(nv);
				}

				dbg("maclist count[%d]\n", maclist->count);

				ret = wl_ioctl(wlif_name, WLC_SET_MACLIST, maclist, sizeof(maclist_buf));
				if (ret < 0)
					dbg("[%s] set maclist failed\n", wlif_name);
			}

			unit++;
		}
	}
}

static int _wl_bw_cap(char *ifname, uint32_t band, uint32_t* bw_cap)
{
	union ioval_u {
		char buf[WLC_IOCTL_MAXLEN];
		uint32_t val;
	} u;

        struct {
                uint32_t band;
                uint32_t bw_cap;
        } param = { 0, 0 };

	strcpy(u.buf, "bw_cap");
	param.band = band;
	memcpy(u.buf + strlen(u.buf) + 1, (void *)&param, sizeof(param));

	if (wl_ioctl(ifname, WLC_GET_VAR, u.buf, sizeof(u.buf)) < 0) {
		dbg("error to read bw_cap");
		return (-1);
	}

	*bw_cap =  dtoh32(u.val);
	return 0;
}

int wl_get_bw_cap(int unit, int *bwcap)
{
	uint32_t band = 0;
	uint32_t val = 0;
	char *ifname = NULL, prefix[16] = {0}, tmp[64] = {0};
	int nband = 0;

	if (unit >= 0) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		nband = nvram_get_int(strcat_r(prefix, "nband", tmp));
	}
	else
	{
		dbg("unit is invalid");
		return -1;
	}

	/* find correct band for driver */
	if (nband == 2)
		band = WLC_BAND_2G;
	else if (nband == 1)
		band = WLC_BAND_5G;
#ifdef RTCONFIG_WIFI6E
	else if (nband == 4)
		band = WLC_BAND_6G;
#endif

	if (band == 0) {
		dbg("can't find correct band");
		return -1;
	}

	if(_wl_bw_cap(ifname, band, &val) < 0) {
		dbg("fail to get bw cap");
		return -1;
	}

	*bwcap = val;

	return (0);
}
#endif

int wl_cap(int unit, char *cap_check)
{
	char ifname[NVRAM_MAX_PARAM_LEN];
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_MEDLEN];
	char *next = NULL;

	wl_ifname(unit, 0, ifname);

	if (!wl_iovar_get(ifname, "cap", (void *)caps, sizeof(caps))) {
		foreach(cap, caps, next) {
			if (!strcmp(cap, cap_check))
				return 1;
		}
	}
#ifdef RPAX56
	if(strcmp(cap_check, "11ax") == 0)
		return 1;
#endif

	return 0;
}

int wl_get_bw(int unit)
{
	char ifname[NVRAM_MAX_PARAM_LEN];
	chanspec_t chspec = 0;
	int bw = 0;

	wl_ifname(unit, 0, ifname);

	if (wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit))) {
		_dprintf("%s: wl%d not up\n", __func__, unit);
		return 0;
	}

	wl_iovar_get(ifname, "chanspec", &chspec, sizeof(chanspec_t));

	if (wf_chspec_valid(chspec)) {
		if (CHSPEC_IS20(chspec))
			bw = 20;
		else if (CHSPEC_IS40(chspec))
			bw = 40;
		else if (CHSPEC_IS80(chspec))
			bw = 80;
#if defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_BW160M)
		else if (CHSPEC_IS160(chspec))
			bw = 160;
#endif
	}

	if(nvram_match("amas_status_syslog", "1")) {
		_dprintf("%s: ifname:%s(%d), valid:%d(%d/%d/%d/%d), bw=%d\n", __func__, ifname, unit, wf_chspec_valid(chspec), CHSPEC_IS20(chspec), CHSPEC_IS40(chspec), CHSPEC_IS80(chspec), CHSPEC_IS160(chspec), bw);
		//RAST_DBG("%s: ifname:%s(%d), valid:%d(%d/%d/%d/%d), bw=%d\n", __func__, ifname, unit, wf_chspec_valid(chspec), CHSPEC_IS20(chspec), CHSPEC_IS40(chspec), CHSPEC_IS80(chspec), CHSPEC_IS160(chspec), bw);
	}

	return bw;
}

#ifdef RTCONFIG_GEFORCENOW
int wl_set_wifiscan(char *ifname, int val)
{
	char buf[48] = {0};
	snprintf(buf, sizeof(buf), "wl -i %s scansuppress %d", ifname, val);
	system(buf);

	return 0;
}

int wl_set_mcsindex(char *ifname, int *is_auto, int *idx, char *idx_type, int *stream)
{
	char buf[128] = {0};
	char *rate = NULL;

	if (!strcmp(ifname, nvram_safe_get("wl0_ifname"))) {
		rate = "2g_rate";
	}
	else {
		rate = "5g_rate";
	}

	if (*is_auto) {
		snprintf(buf, sizeof(buf), "wl -i %s %s auto", ifname, rate);
	}
	else {
		if (!strcmp(idx_type, "vht")) {
			if (*idx > 9) *idx = 9;
			if (*idx < 1) *idx = 1;
			snprintf(buf, sizeof(buf), "wl -i %s %s -v %d -s %d", ifname, rate, *idx, *stream);
		}
		else if (!strcmp(idx_type, "ht")) {
			if (*idx > 23) *idx = 23;
			if (*idx < 1) *idx = 1;
			/* HT can't work with stream parameter */
			*stream = 0;
			snprintf(buf, sizeof(buf), "wl -i %s %s -h %d", ifname, rate, *idx);
		}
		else {
			NVGFN_DBG("(%s) illegal format!\n", ifname);
		}
	}
	system(buf);
	NVGFN_DBG("(%s) buf=%s\n", ifname, buf);

	return 0;
}
#endif

#ifdef RTCONFIG_NEW_PHYMAP
/* phy port related start */
phy_port_mapping get_phy_port_mapping(void)
{
	static phy_port_mapping port_mapping = {
#if defined(RTCONFIG_HND_ROUTER_AX_6710)
#if defined(RTCONFIG_EXTPHY_BCM84880)
		.count = 6,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = 7, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth5" }
#else
		.count = 5,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" }
#endif // #if defined(RTCONFIG_EXTPHY_BCM84880)
#else // #if defined(RTCONFIG_HND_ROUTER_AX_6710)
#ifndef HND_ROUTER
#if defined(RTN14UHP)
		.count = 5,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 100, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 0, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = NULL },
		.port[2] = { .phy_port_id = 1, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = NULL },
		.port[3] = { .phy_port_id = 2, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = NULL },
		.port[4] = { .phy_port_id = 3, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = NULL }
#elif defined(RTN53) || defined(RTN15U) || defined(RTN12) || defined(RTN12B1) || defined(RTN12C1) || \
		defined(RTN12D1) || defined(RTN12VP) || defined(RTN12HP) || defined(RTN12HP_B1) || defined(APN12HP) || \
		defined(RTN10P) || defined(RTN10D1) || defined(RTN10PV2)
		.count = 5,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 100, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = NULL },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = NULL },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = NULL },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = NULL }
#elif defined(RTN16) || defined(RTN10U)
		.count = 5,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 4, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[2] = { .phy_port_id = 3, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[3] = { .phy_port_id = 2, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[4] = { .phy_port_id = 1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL }
#elif defined(RTAC88U) || defined(RTAC3100)
#if defined(RTCONFIG_EXT_RTL8365MB)
		.count = 9,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[5] = { .phy_port_id = S_RTL8365MB+0, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[6] = { .phy_port_id = S_RTL8365MB+1, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[7] = { .phy_port_id = S_RTL8365MB+2, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[8] = { .phy_port_id = S_RTL8365MB+3, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" }
#elif RTCONFIG_EXT_RTL8370MB
		.count = 9,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = S_RTL8365MB+2, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[2] = { .phy_port_id = S_RTL8365MB+3, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[3] = { .phy_port_id = S_RTL8365MB+4, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[4] = { .phy_port_id = S_RTL8365MB+0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[5] = { .phy_port_id = S_RTL8365MB+1, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[6] = { .phy_port_id = S_RTL8365MB+5, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[7] = { .phy_port_id = S_RTL8365MB+6, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[8] = { .phy_port_id = S_RTL8365MB+7, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" }
#else
		.count = 5,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" }
#endif
#elif defined(RTAC56S) || defined(RTAC56U)
		.count = 5,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 0, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[2] = { .phy_port_id = 1, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[3] = { .phy_port_id = 2, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[4] = { .phy_port_id = 3, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL }
#elif defined(RTAC87U)
		.count = 5,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 5, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[2] = { .phy_port_id = 3, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[3] = { .phy_port_id = 2, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[4] = { .phy_port_id = 1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL }
#elif defined(DSLAC68U) || defined(RTAC68U) || defined(RTN18U) || defined(RTAC53U) || defined(RTN66U) || \
		defined(RTAC66U) || defined(RTAC1200G) || defined(RTAC1200GP)
		.count = 5,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 100, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = "vlan1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = "vlan1" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = "vlan1" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 100, .ifname = "vlan1" }
#elif defined(RTAC3200)
		.count = 5,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 4, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[2] = { .phy_port_id = 3, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[3] = { .phy_port_id = 2, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[4] = { .phy_port_id = 1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL }
#elif defined(RTAC5300)
#ifdef RTCONFIG_EXT_RTL8365MB
		.count = 9,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[5] = { .phy_port_id = S_RTL8365MB+3, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[6] = { .phy_port_id = S_RTL8365MB+2, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[7] = { .phy_port_id = S_RTL8365MB+1, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[8] = { .phy_port_id = S_RTL8365MB+0, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL }
#else
		.count = 5,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "vlan1" }
#endif
#endif
#else //#ifndef HND_ROUTER
#ifndef RTCONFIG_HND_ROUTER_AX_675X
#if defined(RTAC86U) || defined(GTAC2900)
		.count = 5,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" }
#elif defined(GTAC5300)
		.count = 9,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[2] = { .phy_port_id = 0, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[3] = { .phy_port_id = S_53134+3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[4] = { .phy_port_id = S_53134+2, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[5] = { .phy_port_id = 3, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[6] = { .phy_port_id = 2, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[7] = { .phy_port_id = S_53134+1, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[8] = { .phy_port_id = S_53134, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[9] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
		.port[10] = { .phy_port_id = -1, .label_name = "U2", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#elif defined(EBG15) || defined(EBP15)
		.count = 5,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
#elif defined(EBG19)
		.count = 11,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = S_53134+3, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "ethsw_3" }, /* eth5 */
		.port[6] = { .phy_port_id = S_53134+2, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "ethsw_2" }, /* eth5 */
		.port[7] = { .phy_port_id = S_53134+1, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "ethsw_1" }, /* eth5 */
		.port[8] = { .phy_port_id = S_53134, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "ethsw_0" },   /* eth5 */ 
		.port[9] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
		.port[10] = { .phy_port_id = -1, .label_name = "U2", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#elif defined(RTAX88U) || defined(BC109)
		.count = 11,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = S_53134+3, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[6] = { .phy_port_id = S_53134+2, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[7] = { .phy_port_id = S_53134+1, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[8] = { .phy_port_id = S_53134, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" }, 
		.port[9] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
		.port[10] = { .phy_port_id = -1, .label_name = "U2", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#elif defined(RTAX92U)
		.count = 7,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
		.port[6] = { .phy_port_id = -1, .label_name = "U2", .cap = PHY_PORT_CAP_USB, .max_rate = 480, .ifname = NULL }
#elif defined(GTAX11000)
#ifdef RTCONFIG_EXT_BCM53134
		.count = 9,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[2] = { .phy_port_id = 0, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[3] = { .phy_port_id = S_53134+3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[4] = { .phy_port_id = S_53134+2, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[5] = { .phy_port_id = 3, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[6] = { .phy_port_id = 2, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[7] = { .phy_port_id = S_53134+1, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL },
		.port[8] = { .phy_port_id = S_53134, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = NULL }
#elif defined(RTCONFIG_EXTPHY_BCM84880)
		.count = 6,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = 7, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth5" }
#endif
#elif defined(GTAXE11000)
		.count = 6,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 0, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 3, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 2, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[5] = { .phy_port_id = 7, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth5" }
#elif defined(RTAX88U)
		.count = 9,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = S_53134+3, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[6] = { .phy_port_id = S_53134+2, .label_name = "L6", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[7] = { .phy_port_id = S_53134+1, .label_name = "L7", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" },
		.port[8] = { .phy_port_id = S_53134, .label_name = "L8", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth5" }
#elif defined(RTAX92U)
		.count = 5,
		.port[0] = { .phy_port_id = 7, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" }
#elif defined(GTAX6000)
		.count = 6,
		.port[0] = { .phy_port_id = 5, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[5] = { .phy_port_id = 6, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth5" },
		.port[6] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
		.port[7] = { .phy_port_id = -1, .label_name = "U2", .cap = PHY_PORT_CAP_USB, .max_rate = 480, .ifname = NULL }
#elif defined(RTAX88U_PRO)
		.count = 7,
		.port[0] = { .phy_port_id = 5, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[5] = { .phy_port_id = 6, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth5" },
		.port[6] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
#elif defined(GTAX11000_PRO)
		.count = 6,
		.port[0] = { .phy_port_id = 5, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[5] = { .phy_port_id = 6, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 10000, .ifname = "eth5" },
#elif defined(GTAXE16000)
		.count = 7,
		.port[0] = { .phy_port_id = 5, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[5] = { .phy_port_id = 6, .label_name = "L5", .cap = PHY_PORT_CAP_LAN | PHY_PORT_CAP_WAN, .max_rate = 10000, .ifname = "eth5" },
		.port[6] = { .phy_port_id = 7, .label_name = "L6", .cap = PHY_PORT_CAP_LAN | PHY_PORT_CAP_WAN, .max_rate = 10000, .ifname = "eth6" }
#elif defined(ET12) || defined(XT12)
		.count = 4,
		.port[0] = { .phy_port_id = 5, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 6, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth3" },
#elif defined(RTAX86U_PRO)
		.count = 6,
		.port[0] = { .phy_port_id = 5, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[5] = { .phy_port_id = 6, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth5" },
#endif
#else //#ifndef RTCONFIG_HND_ROUTER_AX_675X
#if defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2)
		.count = 4,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" }
#elif defined(RTAX56_XD4)
		.count = 1,
		.port[0] = { .phy_port_id = 0, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
#elif defined(XD4PRO)
		.count = 2,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 0, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
#elif defined(CTAX56_XD4)
		.count = 2,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 0, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
#elif defined(RTAX82_XD6) || defined(XD6_V2)
		.count = 4,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth4" },
		.port[1] = { .phy_port_id = 2, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[2] = { .phy_port_id = 1, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[3] = { .phy_port_id = 0, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth0" },
#elif defined(RTAX82_XD6S)
		.count = 2,
		.port[0] = { .phy_port_id = 1, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth1" },
		.port[1] = { .phy_port_id = 0, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth0" },
#elif defined(BCM6750) || defined(BCM63178)
		.count = 5,
		.port[0] = { .phy_port_id = 4, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth4" },
		.port[1] = { .phy_port_id = 3, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 1, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[4] = { .phy_port_id = 0, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth0" }
#elif defined(RTAX1800) || defined(BR63) && defined(NEW_SWITCH_ORDER)
#ifdef BR63
		.count = 7,
#else
		.count = 5,
#endif
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
#ifdef BR63
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
		.port[6] = { .phy_port_id = -1, .label_name = "U2", .cap = PHY_PORT_CAP_USB, .max_rate = 480, .ifname = NULL }
#else
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" }
#endif
#elif defined(RTAX55) || defined(RTAX3000N) || defined(BR63) && !defined(NEW_SWITCH_ORDER)
#ifdef BR63
		.count = 7,
#else
		.count = 5,
#endif
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 4, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 3, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[3] = { .phy_port_id = 2, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
#ifdef BR63
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL },
		.port[6] = { .phy_port_id = -1, .label_name = "U2", .cap = PHY_PORT_CAP_USB, .max_rate = 480, .ifname = NULL }
#else
		.port[4] = { .phy_port_id = 1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" }
#endif
#elif defined(RTAX58U_V2)
		.count = 6,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 4, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 3, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[3] = { .phy_port_id = 2, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[4] = { .phy_port_id = 1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[5] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#elif defined(TUFAX3000_V2) || defined(RTAXE7800)
		.count = 5,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[5] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#elif defined(RTAXE7800)
		.count = 6,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN | PHY_PORT_CAP_WANLAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN | PHY_PORT_CAP_WANLAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[4] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[5] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#elif defined(GT10)
		.count = 5,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN | PHY_PORT_CAP_WANLAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN | PHY_PORT_CAP_WANLAN, .max_rate = 1000, .ifname = "eth1" },
		.port[2] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[3] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[4] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#elif defined(RTAX9000)
		.count = 7,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN | PHY_PORT_CAP_WANLAN, .max_rate = 2500, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 5, .label_name = "L5", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth5" },
		.port[2] = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" },
		.port[3] = { .phy_port_id = 2, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 3, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[5] = { .phy_port_id = 4, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[6] = { .phy_port_id = -1, .label_name = "U1", .cap = PHY_PORT_CAP_USB, .max_rate = 5000, .ifname = NULL }
#elif defined(RTAX56U)
		.count = 5,
		.port[0] = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" },
		.port[1] = { .phy_port_id = 4, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth4" },
		.port[2] = { .phy_port_id = 3, .label_name = "L2", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth3" },
		.port[3] = { .phy_port_id = 2, .label_name = "L3", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth2" },
		.port[4] = { .phy_port_id = 1, .label_name = "L4", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" }
#endif
#endif //#ifndef HND_ROUTER
#endif //#ifndef RTCONFIG_HND_ROUTER_AX_675X
#endif // #if defined(RTCONFIG_HND_ROUTER_AX_6710)
	};

	port_mapping.extsw_count = 0;
#if defined(RTCONFIG_EXT_RTL8365MB) 
	port_mapping.extsw_count = 4;
#elif defined(RTCONFIG_EXT_RTL8370MB)
	port_mapping.extsw_count = 8;
#endif

#if defined(RTAX56_XD4)
	if(nvram_match("HwId", "A") || nvram_match("HwId", "C")) {
		phy_port port_0 = { .phy_port_id = 0, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth0" };
		phy_port port_1 = { .phy_port_id = 1, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 1000, .ifname = "eth1" };
		port_mapping.count = 2;
		port_mapping.port[0] = port_0;
		port_mapping.port[1] = port_1;
	}
#endif

#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(GT10)
	if (nvram_get_int("wans_extwan")) {
		phy_port port_0 = { .phy_port_id = 0, .label_name = "L1", .cap = PHY_PORT_CAP_LAN, .max_rate = 2500, .ifname = "eth0" };
		phy_port port_1 = { .phy_port_id = 1, .label_name = "W0", .cap = PHY_PORT_CAP_WAN, .max_rate = 1000, .ifname = "eth1" };
		port_mapping.port[0] = port_0;
		port_mapping.port[1] = port_1;
	}
#endif

	return port_mapping;
}
#endif
/* phy port related end.*/

#if defined(RTCONFIG_OWE_TRANS)
void append_owe_trans_vif()
{
    int unit = 0;
    char word[100], tmp[100], *next, prefix[]="wlXXXXX_";
    char wl_ifnames[32] = {0};
    char wl_vif[8] = {0};
    char wl_vifnames[128]= {0};

    strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
    foreach (word, wl_ifnames, next) {
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	if (nvram_get_int(strcat_r(prefix, "nband", tmp)) != 4) { // 6GHz band does not support OWE-Transition
		snprintf(tmp, sizeof(tmp), "wl%d_owe_transition_ifname", unit);
		snprintf(wl_vif, sizeof(wl_vif), "wl%d.%d", unit, num_of_mssid_support(unit)+1);
		nvram_set(tmp, wl_vif);
		_dprintf("%s(%d): assign wl%d owe transition ifname [%s] \n", __FUNCTION__, __LINE__, unit, wl_vif);
		snprintf(tmp, sizeof(tmp), "wl%d_vifnames", unit);
		strcpy(wl_vifnames, nvram_safe_get(tmp));
		add_to_list(wl_vif, wl_vifnames, sizeof(wl_vifnames));
		nvram_set(tmp, wl_vifnames);
		_dprintf("%s(%d): update wl%d_vifnames [%s] \n", __FUNCTION__, __LINE__, unit, wl_vifnames);
	}
	else {
		snprintf(tmp, sizeof(tmp), "wl%d_owe_transition_ifname", unit);
		if(strlen(nvram_safe_get(tmp)))
			nvram_unset(tmp);
	}
        unit++;
    }
    nvram_commit();
}
#endif

#if defined(RTCONFIG_HND_ROUTER_AX_675X)
void process_affinity(pid_t pid, unsigned int cpumask)
{
	cpu_set_t cpuset;
	int i = 0;

	CPU_ZERO(&cpuset);
	do {
		if (cpumask & 1)
			CPU_SET(i, &cpuset);
		i++;
		cpumask >>= 1;
	} while (cpumask != 0);

	sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
}
#endif
