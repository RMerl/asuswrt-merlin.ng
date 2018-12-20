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

#ifdef LINUX26
#define GPIO_IOCTL
#endif

// --- move begin ---
#ifdef GPIO_IOCTL

#include <sys/ioctl.h>
#include <linux_gpio.h>
#include <time.h>

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
	char *name;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	memset(bssid_str, 0, 18);
	if (!wl_ioctl(name, WLC_GET_BSSID, bssid, sizeof(bssid))
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
	char tmp[NVRAM_BUFSIZE], tmp2[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	char *name = NULL;
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

	if (!nvram_match(strcat_r(prefix, "mode", tmp), "psta") &&
	    !nvram_match(strcat_r(prefix, "mode", tmp2), "psr"))
		goto PSTA_ERR;

	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (wl_ioctl(name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN) != 0)
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
		if (wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size)) {
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
		eval("wl", "-i", name, "send_nulldata", macaddr);
#endif
	}
	else if (ret == 1) {
		if (debug) dbg("[wlc] not authorized\n");
	} else {
		if (debug) dbg("[wlc] not associated\n");
	}

	return ret;
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

void add_beacon_vsie(char *hexdata)
{
	unsigned char value[256];
	int pktflag = VNDR_IE_BEACON_FLAG | VNDR_IE_PRBRSP_FLAG;
	int len = 0;

	len = DOT11_OUI_LEN + strlen(hexdata)/2;

	if (string2hex(hexdata, value, strlen(hexdata)))
		wl_add_ie(0, pktflag, len, (uchar *) OUI_ASUS, value);
}

void del_beacon_vsie(char *hexdata)
{
	wl_del_ie_with_oui(0, (uchar *) OUI_ASUS);
}
#endif

#ifdef RTCONFIG_CFGSYNC
void update_macfilter_relist()
{
	char maclist_buf[4096] = {0};
	struct maclist *maclist = NULL;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char word[256], *next;
	char mac2g[32], mac5g[32], *next_mac;
	int unit = 0;
	char *wlif_name = NULL;
	struct ether_addr *ea;
	unsigned char sta_ea[6] = {0};
	int ret = 0;
	char *nv, *nvp, *b;
	char *reMac, *maclist2g, *maclist5g, *timestamp;
	char stamac2g[18] = {0};
	char stamac5g[18] = {0};

	if (nvram_get("cfg_relist"))
	{
#ifdef RTCONFIG_AMAS
		if (nvram_get_int("re_mode") == 1) {
			nv = nvp = strdup(nvram_safe_get("cfg_relist"));
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
		}
#endif

		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(unit);

#ifdef RTCONFIG_AMAS
			if (nvram_get_int("re_mode") == 1)
				snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
			else
#endif
				snprintf(prefix, sizeof(prefix), "wl%d_", unit);

			wlif_name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
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
								strcmp(b, stamac5g) == 0)
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

				nv = nvp = strdup(nvram_safe_get("cfg_relist"));
				if (nv) {
					while ((b = strsep(&nvp, "<")) != NULL) {
						if ((vstrsep(b, ">", &reMac, &maclist2g, &maclist5g, &timestamp) != 4))
							continue;

						if (strcmp(reMac, get_lan_hwaddr()) == 0)
							continue;

						if (unit == 0) {
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
						else
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

				dbg("maclist count[%d]\n", maclist->count);

				ret = wl_ioctl(wlif_name, WLC_SET_MACLIST, maclist, sizeof(maclist_buf));
				if (ret < 0)
					dbg("[%s] set maclist failed\n", wlif_name);
			}

			unit++;
		}
	}
}
#endif
