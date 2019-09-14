#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <sys/ioctl.h>
#include <qca.h>
#include <iwlib.h>
#include "utils.h"
#include "shutils.h"
#include <shared.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <iwlib.h>
#ifndef O_BINARY
#define O_BINARY 	0
#endif
#include <image.h>
#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif
#include <linux_gpio.h>

typedef uint32_t __u32;

/////// copy from qca-wifi
#define IEEE80211_CHAN_MAX      255
#define IEEE80211_IOCTL_GETCHANINFO     (SIOCIWFIRSTPRIV+7)
typedef unsigned int	u_int;

struct ieee80211_channel {
    u_int16_t       ic_freq;        /* setting in Mhz */
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
    u_int64_t       ic_flags;       /* see below */
#else
    u_int32_t       ic_flags;       /* see below */
#endif
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
    u_int16_t        ic_flagext;     /* see below */
#else
    u_int8_t        ic_flagext;     /* see below */
#endif
    u_int8_t        ic_ieee;        /* IEEE channel number */
    int8_t          ic_maxregpower; /* maximum regulatory tx power in dBm */
    int8_t          ic_maxpower;    /* maximum tx power in dBm */
    int8_t          ic_minpower;    /* minimum tx power in dBm */
    u_int8_t        ic_regClassId;  /* regClassId of this channel */ 
    u_int8_t        ic_antennamax;  /* antenna gain max from regulatory */
    u_int8_t        ic_vhtop_ch_freq_seg1;         /* Channel Center frequency */
    u_int8_t        ic_vhtop_ch_freq_seg2;         /* Channel Center frequency applicable
                                                  * for 80+80MHz mode of operation */ 
};

struct ieee80211req_chaninfo {
	u_int	ic_nchans;
	struct ieee80211_channel ic_chans[IEEE80211_CHAN_MAX];
};

u_int
ieee80211_mhz2ieee(u_int freq)
{
#define IS_CHAN_IN_PUBLIC_SAFETY_BAND(_c) ((_c) > 4940 && (_c) < 4990)
	if (freq == 2484)
        return 14;
    if (freq < 2484)
        return (freq - 2407) / 5;
    if (freq < 5000) {
        if (IS_CHAN_IN_PUBLIC_SAFETY_BAND(freq)) {
            return ((freq * 10) +   
                (((freq % 5) == 2) ? 5 : 0) - 49400)/5;
        } else if (freq > 4900) {
            return (freq - 4000) / 5;
        } else {
            return 15 + ((freq - 2512) / 20);
        }
    }
    if (freq >= 58320 && freq <= 69120) {	/* 802.11ad Wigig */
	    return (freq - 58320) / 2160 + 1;
    }
    return (freq - 5000) / 5;
}
/////////////

#if defined(RTCONFIG_WIFI_QCA9557_QCA9882) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X) || defined(RTCONFIG_SOC_IPQ40XX)
const char WIF_5G[] = "ath1";
const char WIF_2G[] = "ath0";
const char STA_5G[] = "sta1";
const char STA_2G[] = "sta0";
const char VPHY_5G[] = "wifi1";
const char VPHY_2G[] = "wifi0";
const char WSUP_DRV[] = "athr";
const char BR_GUEST[] = "brg0";
const char WIF_5G_BH[] = "ath101";
const char APMODE_BRGUEST_IP[]="192.168.55.1";
#elif defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
      defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
      defined(RTCONFIG_WIFI_QCN5024_QCN5054)
#if defined(GTAXY16000) || defined(GTAX6000N) || defined(RTAX89U) || defined(RTAC88N)
const char WIF_5G[] = "ath0";
const char WIF_2G[] = "ath1";
const char STA_5G[] = "sta0";
const char STA_2G[] = "sta1";
const char VPHY_5G[] = "wifi0";
const char VPHY_2G[] = "wifi1";
#elif defined(RTAD7200)
/* RTAD7200 */
const char WIF_5G[] = "ath1";
const char WIF_2G[] = "ath0";
const char STA_5G[] = "sta1";
const char STA_2G[] = "sta0";
const char VPHY_5G[] = "wifi1";
const char VPHY_2G[] = "wifi0";
#else
/* BRT-AC828, RT-AC88S */
const char WIF_5G[] = "ath1";
const char WIF_2G[] = "ath0";
const char STA_5G[] = "sta1";
const char STA_2G[] = "sta0";
const char VPHY_5G[] = "wifi1";
const char VPHY_2G[] = "wifi0";
#endif
const char WSUP_DRV[] = "athr";
#else
#error Define WiFi 2G/5G interface name!
#endif

#if defined(RTCONFIG_HAS_5G_2)
const char WIF_5G2[] = "ath2";
const char STA_5G2[] = "sta2";
const char VPHY_5G2[] = "wifi2";
#else
const char WIF_5G2[] = "xxx";
const char STA_5G2[] = "xxx";
const char VPHY_5G2[] = "xxx";
#endif

#if defined(RTCONFIG_WIGIG)
const char WIF_60G[] = "wlan0";
const char STA_60G[] = "wlan0";
const char VPHY_60G[] = "phy0";
const char WSUP_DRV_60G[] = "nl80211";
#else
const char WIF_60G[] = "xxx";
const char STA_60G[] = "xxx";
const char VPHY_60G[] = "xxx";
const char WSUP_DRV_60G[] = "xxx";
#endif

const char *max_2g_ax_mode = "11GHE";	/* B,G,N,AX */
const char *max_5g_ax_mode = "11AHE";	/* A,N,AC,AX */
const char *max_2g_n_mode = "11NG";	/* B,G,N */
const char *max_5g_ac_mode = "11ACV";	/* A,N,AC */

/* [0]: 11AC
 * [1]: 11AX
 */
const char *bw20[2] = { "HT20", "20" };
const char *bw40[2] = { "HT40", "40" };
const char *bw80[2] = { "HT80", "80" };
const char *bw80_80_tbl[2] = { "HT80_80", "80_80" };
const char *bw160_tbl[2] = { "HT160", "160" };

#define GPIOLIB_DIR	"/sys/class/gpio"
#ifdef RTCONFIG_LEDS_CLASS
#define LEDSLIB_DIR	"/sys/class/leds"
#endif

/* Export specified GPIO
 * @return:
 * 	0:	success
 *  otherwise:	fail
 */
static int __export_gpio(uint32_t gpio)
{
	char gpio_path[PATH_MAX], export_path[PATH_MAX], gpio_str[] = "999XXX";

	if (!d_exists(GPIOLIB_DIR)) {
		_dprintf("%s does not exist!\n", __func__);
		return -1;
	}
	snprintf(gpio_path, sizeof(gpio_path),"%s/gpio%d", GPIOLIB_DIR, gpio);
	if (d_exists(gpio_path))
		return 0;

	snprintf(export_path, sizeof(export_path), "%s/export", GPIOLIB_DIR);
	snprintf(gpio_str, sizeof(gpio_str), "%d", gpio);
	f_write_string(export_path, gpio_str, 0, 0);

	return 0;
}

uint32_t gpio_dir(uint32_t gpio, int dir)
{
	char path[PATH_MAX], v[10], *dir_str = "in";

	if (dir == GPIO_DIR_OUT) {
		dir_str = "out";		/* output, low voltage */
		*v = '\0';
		snprintf(path, sizeof(path), "%s/gpio%d/value", GPIOLIB_DIR, gpio);
		if (f_read_string(path, v, sizeof(v)) > 0 && safe_atoi(v) == 1)
			dir_str = "high";	/* output, high voltage */
	}

	__export_gpio(gpio);
	snprintf(path, sizeof(path), "%s/gpio%d/direction", GPIOLIB_DIR, gpio);
	f_write_string(path, dir_str, 0, 0);

	return 0;
}

uint32_t get_gpio(uint32_t gpio)
{
	char path[PATH_MAX], value[10];

	snprintf(path, sizeof(path), "%s/gpio%d/value", GPIOLIB_DIR, gpio);
	f_read_string(path, value, sizeof(value));

	return safe_atoi(value);
}

uint32_t set_gpio(uint32_t gpio, uint32_t value)
{
	char path[PATH_MAX], val_str[10];

	snprintf(val_str, sizeof(val_str), "%d", !!value);
#ifdef RTCONFIG_LEDS_CLASS
	snprintf(path, sizeof(path), "%s/led%d/brightness", LEDSLIB_DIR, gpio);
#else
	snprintf(path, sizeof(path), "%s/gpio%d/value", GPIOLIB_DIR, gpio);
#endif
	f_write_string(path, val_str, 0, 0);

	return 0;
}

int get_switch_model(void)
{
	// TODO
	return SWITCH_UNKNOWN;
}

uint32_t get_phy_status(uint32_t portmask)
{
	return 1;		/* FIXME */
}

uint32_t get_phy_speed(uint32_t portmask)
{
	// TODO
	return 1;		/* FIXME */
}

uint32_t set_phy_ctrl(uint32_t portmask, int ctrl)
{
	// TODO
	return 1;		/* FIXME */
}

int get_imageheader_size(void)
{
	return sizeof(image_header_t);
}

int wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq)
{
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
	if ((ret = ioctl(s, cmd, pwrq)) < 0)
		perror(pwrq->ifr_name);

	/* cleanup */
	close(s);
	return ret;
}

unsigned int get_radio_status(char *ifname)
{
	struct ifreq ifr;
	int sfd;
	int ret;

	if ((sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0)
	{
		strcpy(ifr.ifr_name, ifname);
		ret = ioctl(sfd, SIOCGIFFLAGS, &ifr);
		close(sfd);
		if (ret == 0)
			return !!(ifr.ifr_flags & IFF_UP);
	}
	return 0;
}

int match_radio_status(int unit, int status)
{
	int sub = 0, rs = status;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX", athfix[]="athXXXXXX";

	do {
		if (sub > 0)
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, sub);
		else
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		strcpy(athfix, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

		if (*athfix != '\0') {
			if (status)
				rs &= get_radio_status(athfix);
			else
				rs |= get_radio_status(athfix);
		}
		sub++;
	} while (sub <= 3);

	return (status == rs);
}

int get_radio(int unit, int subunit)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";

	if (subunit > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if (subunit > 0)
		return nvram_match(strcat_r(prefix, "radio", tmp), "1");
	else
		return get_radio_status(nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
}

void set_radio(int on, int unit, int subunit)
{
	int onoff = (!on)? LED_OFF:LED_ON;
	int led = get_wl_led_id(unit);
	int sub = (subunit >= 0) ? subunit : 0;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX", athfix[]="athXXXXXX";
	char path[sizeof(NAWDS_SH_FMT) + 6], wds_iface[IFNAMSIZ] = "";

	if (unit < WL_2G_BAND || unit >= WL_NR_BANDS) {
		dbg("%s: wl%d is not supported!\n", __func__, unit);
		return;
	}

	strlcpy(wds_iface, get_wififname(unit), sizeof(wds_iface));

#if defined(RTCONFIG_WIGIG)
	if (unit == WL_60G_BAND) {
#if defined(RTCONFIG_SINGLE_HOSTAPD)
		if (on) {
			char bss_cfg[sizeof("bss_config=") + IFNAMSIZ + sizeof(":/etc/Wireless/conf/hostapd_XXX.conf") + IFNAMSIZ];

			snprintf(bss_cfg, sizeof(bss_cfg), "bss_config=%s:/etc/Wireless/conf/hostapd_%s.conf", wds_iface, wds_iface);
			eval(QWPA_CLI, "-g", QHOSTAPD_CTRL_IFACE, "raw", "ADD", bss_cfg);
		} else {
			eval(QWPA_CLI, "-g", QHOSTAPD_CTRL_IFACE, "raw", "REMOVE", wds_iface);
		}
#else
		char conf_path[sizeof("/etc/Wireless/conf/hostapd_athXXX.confYYYYYY")];
		char pid_path[sizeof("/var/run/hostapd_athXXX.pidYYYYYY")];
		char entropy_path[sizeof("/var/run/entropy_athXXX.binYYYYYY")];

		snprintf(pid_path, sizeof(pid_path), "/var/run/hostapd_%s.pid", wds_iface);
		if (on) {
			snprintf(conf_path, sizeof(conf_path), "/etc/Wireless/conf/hostapd_%s.conf", wds_iface);
			snprintf(entropy_path, sizeof(entropy_path), "/var/run/entropy_%s.bin", wds_iface);
			eval("hostapd", "-d", "-B", conf_path, "-P", pid_path, "-e", entropy_path);
		} else {
			kill_pidfile(pid_path);
		}
#endif
	}
#endif	/* RTCONFIG_WIGIG */

	do {
		if (sub > 0)
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, sub);
		else
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		strcpy(athfix, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

		if (*athfix != '\0' && strncmp(athfix, "sta", 3)) {
			/* all lan-interfaces except sta when running repeater mode */
			_dprintf("%s: unit %d-%d, %s\n", __func__, unit, sub, (on?"on":"off"));
			if (unit != WL_60G_BAND) {
				eval("ifconfig", athfix, on? "up":"down");
			}

			/* Reconnect to peer WDS AP */
			if (!sub) {
				snprintf(path, sizeof(path), NAWDS_SH_FMT, wds_iface);
				if (!nvram_match(strcat_r(prefix, "mode_x", tmp), "0") && f_exists(path))
					doSystem(path);
			}
		}
		sub++;
	} while (subunit < 0 && sub <= 3);

#if defined(RTCONFIG_WPS_ALLLED_BTN)
	if (nvram_match("AllLED", "1"))
#endif
		led_control(led, onoff);
}

char *wif_to_vif(char *wif)
{
	static char vif[32];
	int unit = 0, subunit = 0;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";

	vif[0] = '\0';

	for (unit = 0; unit < MAX_NR_WL_IF; unit++) {
		SKIP_ABSENT_BAND(unit);
		for (subunit = 1; subunit < MAX_NO_MSSID; subunit++) {
			snprintf(prefix, sizeof(prefix), "wl%d.%d", unit, subunit);

			if (nvram_match(strcat_r(prefix, "_ifname", tmp), wif)) {
				snprintf(vif, sizeof(vif), "%s", prefix);
				goto RETURN_VIF;
			}
		}
	}

RETURN_VIF:
	return vif;
}

/* get channel list via iw utility */
static int __get_channel_list_via_iw(int unit, char *buffer, int len)
{
	int l, r, found, freq, first = 1;
	FILE *fp;
	char *p = buffer, line[256], cmd[sizeof("iw phy0 infoXXXXXXXXXXXX")];

	if (buffer == NULL || len <= 0 || unit < 0 || unit >= MAX_NR_WL_IF)
		return -1;

	memset(buffer, 0, len);
	snprintf(cmd, sizeof(cmd), "iw %s info", get_vphyifname(unit));
	fp = popen(cmd, "r");
	if (!fp)
		return 0;

	/* Example:
	 * Wiphy phy0
	 *       Band 1:
	 *               Capabilities: 0x00
	 *                       HT20
	 *                       Static SM Power Save
	 *                       No RX STBC
	 *                       Max AMSDU length: 3839 bytes
	 *                       No DSSS/CCK HT40
	 *               Maximum RX AMPDU length 65535 bytes (exponent: 0x003)
	 *               Minimum RX AMPDU time spacing: 8 usec (0x06)
	 *               HT TX/RX MCS rate indexes supported: 1-12
	 *               Frequencies:
	 *                       * 58320 MHz [1] (0.0 dBm)
	 *                       * 60480 MHz [2] (0.0 dBm)
	 *                       * 62640 MHz [3] (0.0 dBm)
	 *               Bitrates (non-HT):
	 */
	r = found = 0;
	while (len > 0 && fgets(line, sizeof(line), fp)) {
		if (!found) {
			if (!strstr(line, "Frequencies:")) {
				continue;
			} else {
				found = 1;
				continue;
			}
		} else {
			if (strstr(line, "disabled"))
				continue;
			if (!strstr(line, "MHz") || (r = sscanf(line, "%*[^0-9]%d Mhz%*[^\n]", &freq)) != 1) {
				found = 0;
				continue;
			}
			l = snprintf(p, len, "%s%u", first? "" : ",", ieee80211_mhz2ieee(freq));
			p += l;
			len -= l;
			first = 0;
		}
	}
	pclose(fp);

	return (p - buffer);
}

/* get channel list via getchaninfo ioctl */
static int __get_channel_list_via_getchaninfo(int unit, char *buffer, int len)
{
	struct ieee80211req_chaninfo chans;
	struct iwreq wrq;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_", *ifname;
	int i, l = len;
	char *p;

	if (buffer == NULL || len <= 0 || unit < 0 || unit >= MAX_NR_WL_IF)
		return -1;

	memset(buffer, 0, len);
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	memset(&wrq, 0, sizeof(wrq));
	wrq.u.data.pointer = (void *)&chans;
	wrq.u.data.length = sizeof(chans);
	if (wl_ioctl(ifname, IEEE80211_IOCTL_GETCHANINFO, &wrq) < 0)
		return -1;

	for (i = 0, p=buffer; len > 0 && i < chans.ic_nchans ; i++) {
		if (i == 0)
			l = snprintf(p, len, "%u", ieee80211_mhz2ieee(chans.ic_chans[i].ic_freq));
		else
			l = snprintf(p, len, ",%u", ieee80211_mhz2ieee(chans.ic_chans[i].ic_freq));
		p += l;
		len -= l;
	}
	return (p - buffer);
}

/* get channel list via currently setting in wifi driver */
int get_channel_list_via_driver(int unit, char *buffer, int len)
{
	int r = 0;

	if (absent_band(unit))
		return 0;

	if (buffer == NULL || len <= 0 || unit < 0 || unit >= MAX_NR_WL_IF)
		return -1;

	switch (unit) {
	case WL_2G_BAND:	/* fall-through */
	case WL_5G_BAND:	/* fall-through */
	case WL_5G_2_BAND:
		r = __get_channel_list_via_getchaninfo(unit, buffer, len);
		break;
	case WL_60G_BAND:
		r = __get_channel_list_via_iw(unit, buffer, len);
		break;
	default:
		dbg("%s: Unknown wl%d band!\n", __func__, unit);
	}

	return r;
}

/* Verify QCA Wi-Fi EEPROM checksum
 * @eeprom:		pointer to a EEPROM data in ART/Factory.
 * @eeprom_length:	length of EEPROM data
 * @return:
 * 	0:		success
 *     -2:		invalid parameter
 *     -1:		wrong checksum
 */
int verify_qca_eeprom_csum(void *eeprom, unsigned int eeprom_length)
{
    unsigned short *p_half;
    unsigned short sum = 0;
    int i;

    if (!eeprom_length || (eeprom_length & 1))
	    return -1;

    p_half = (unsigned short *)eeprom;
    for (i = 0; i < eeprom_length / 2; i++) {
        sum ^= __le16_to_cpu(*p_half++);
    }
    if (sum != 0xffff) {
        return -1;
    }
    return 0;
}

/* Calculate checksum of a QCA EEPROM.
 * @ptr:		pointer to EEPROM data
 * @eeprom_size:	size of this eeprom
 * @eeprom_csum_offset:	offset of checksum
 * @return:
 * 	0:		success
 *     -1:		invalid parameter
 */
int calc_qca_eeprom_csum(void *ptr, unsigned int eeprom_size, unsigned int eeprom_csum_offset)
{
	int i;
	uint16_t *p = ptr, sum = 0, csum_idx;

	if (!ptr || (eeprom_size & 1) || (eeprom_csum_offset + 1 ) >= eeprom_size ||
	    (eeprom_csum_offset & 1))
	{
		_dprintf("%s: invalid param. (ptr %p, eeprom_size %u, eeprom_csum_offset %u)\n",
			__func__, ptr, eeprom_size, eeprom_csum_offset);
		return -1;
	}

	csum_idx = eeprom_csum_offset / 2;
	*(p + csum_idx) = 0;
	for (i = 0; i < (eeprom_size / 2); ++i, ++p)
		sum ^= __le16_to_cpu(*p);

	p = ptr;
	*(p + csum_idx) = __cpu_to_le16(sum ^ 0xFFFF);

	return 0;
}

/* get channel list via value of countryCode */
unsigned char A_BAND_REGION_0_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 149, 153, 157, 161, 165 };
unsigned char A_BAND_REGION_1_CHANNEL_LIST[] = { 36, 40, 44, 48 };

#ifdef RTCONFIG_LOCALE2012
unsigned char A_BAND_REGION_2_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165 };
unsigned char A_BAND_REGION_3_CHANNEL_LIST[] =
    { 52, 56, 60, 64, 149, 153, 157, 161, 165 };
#else
unsigned char A_BAND_REGION_2_CHANNEL_LIST[] = { 36, 40, 44, 48 };
unsigned char A_BAND_REGION_3_CHANNEL_LIST[] = { 149, 153, 157, 161 };
#endif
unsigned char A_BAND_REGION_4_CHANNEL_LIST[] = { 149, 153, 157, 161, 165 };
unsigned char A_BAND_REGION_5_CHANNEL_LIST[] = { 149, 153, 157, 161 };

#ifdef RTCONFIG_LOCALE2012
unsigned char A_BAND_REGION_6_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 132, 136, 140, 149, 153, 157, 161, 165 };
#else
unsigned char A_BAND_REGION_6_CHANNEL_LIST[] = { 36, 40, 44, 48 };
#endif
unsigned char A_BAND_REGION_7_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, 169, 173 };
unsigned char A_BAND_REGION_8_CHANNEL_LIST[] = { 52, 56, 60, 64 };

#ifdef RTCONFIG_LOCALE2012
unsigned char A_BAND_REGION_9_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132 };
#else
unsigned char A_BAND_REGION_9_CHANNEL_LIST[] = { 36, 40, 44, 48 };
#endif
unsigned char A_BAND_REGION_10_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 149, 153, 157, 161, 165 };
unsigned char A_BAND_REGION_11_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 149, 153, 157, 161 };
unsigned char A_BAND_REGION_12_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 };
unsigned char A_BAND_REGION_13_CHANNEL_LIST[] =
    { 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161 };
unsigned char A_BAND_REGION_14_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165 };
unsigned char A_BAND_REGION_15_CHANNEL_LIST[] =
    { 149, 153, 157, 161, 165, 169, 173 };
unsigned char A_BAND_REGION_16_CHANNEL_LIST[] =
    { 52, 56, 60, 64, 149, 153, 157, 161, 165 };
unsigned char A_BAND_REGION_17_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 149, 153, 157, 161 };
unsigned char A_BAND_REGION_18_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140 };
unsigned char A_BAND_REGION_19_CHANNEL_LIST[] =
    { 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161 };
unsigned char A_BAND_REGION_20_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 149, 153, 157, 161 };
unsigned char A_BAND_REGION_21_CHANNEL_LIST[] =
    { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161 };

unsigned char G_BAND_REGION_0_CHANNEL_LIST[] =
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
unsigned char G_BAND_REGION_1_CHANNEL_LIST[] =
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
unsigned char G_BAND_REGION_5_CHANNEL_LIST[] =
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

/* Temporarilly workaround. */
unsigned char AD_BAND_TMP_REGION_CHANNEL_LIST[] =
    { 1, 2, 3, };

#define A_BAND_REGION_0				0
#define A_BAND_REGION_1				1
#define A_BAND_REGION_2				2
#define A_BAND_REGION_3				3
#define A_BAND_REGION_4				4
#define A_BAND_REGION_5				5
#define A_BAND_REGION_6				6
#define A_BAND_REGION_7				7
#define A_BAND_REGION_8				8
#define A_BAND_REGION_9				9
#define A_BAND_REGION_10			10
#define A_BAND_REGION_11			11
#define A_BAND_REGION_12			12
#define A_BAND_REGION_13			13
#define A_BAND_REGION_14			14
#define A_BAND_REGION_15			15
#define A_BAND_REGION_16			16
#define A_BAND_REGION_17			17
#define A_BAND_REGION_18			18
#define A_BAND_REGION_19			19
#define A_BAND_REGION_20			20
#define A_BAND_REGION_21			21

#define G_BAND_REGION_0				0
#define G_BAND_REGION_1				1
#define G_BAND_REGION_2				2
#define G_BAND_REGION_3				3
#define G_BAND_REGION_4				4
#define G_BAND_REGION_5				5
#define G_BAND_REGION_6				6

typedef struct CountryCodeToCountryRegion {
	unsigned char IsoName[3];
	unsigned char RegDomainNum11A;
	unsigned char RegDomainNum11G;
} COUNTRY_CODE_TO_COUNTRY_REGION;

COUNTRY_CODE_TO_COUNTRY_REGION allCountry[] = {
	/* {Country Number, ISO Name, Country Name, Support 11A, 11A Country Region, Support 11G, 11G Country Region} */
	{"DB", A_BAND_REGION_7, G_BAND_REGION_5},
	{"AL", A_BAND_REGION_0, G_BAND_REGION_1},
	{"DZ", A_BAND_REGION_0, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"AR", A_BAND_REGION_0, G_BAND_REGION_1},
	{"AM", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"AR", A_BAND_REGION_3, G_BAND_REGION_1},
	{"AM", A_BAND_REGION_2, G_BAND_REGION_1},
#endif
	{"AU", A_BAND_REGION_0, G_BAND_REGION_1},
	{"AT", A_BAND_REGION_1, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"AZ", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"AZ", A_BAND_REGION_2, G_BAND_REGION_1},
#endif
	{"BH", A_BAND_REGION_0, G_BAND_REGION_1},
	{"BY", A_BAND_REGION_0, G_BAND_REGION_1},
	{"BE", A_BAND_REGION_1, G_BAND_REGION_1},
	{"BZ", A_BAND_REGION_4, G_BAND_REGION_1},
	{"BO", A_BAND_REGION_4, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"BR", A_BAND_REGION_4, G_BAND_REGION_1},
#else
	{"BR", A_BAND_REGION_1, G_BAND_REGION_1},
#endif
	{"BN", A_BAND_REGION_4, G_BAND_REGION_1},
	{"BG", A_BAND_REGION_1, G_BAND_REGION_1},
	{"CA", A_BAND_REGION_0, G_BAND_REGION_0},
	{"CL", A_BAND_REGION_0, G_BAND_REGION_1},
	{"CN", A_BAND_REGION_4, G_BAND_REGION_1},
	{"CO", A_BAND_REGION_0, G_BAND_REGION_0},
	{"CR", A_BAND_REGION_0, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"HR", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"HR", A_BAND_REGION_2, G_BAND_REGION_1},
#endif
	{"CY", A_BAND_REGION_1, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"CZ", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"CZ", A_BAND_REGION_2, G_BAND_REGION_1},
#endif
	{"DK", A_BAND_REGION_1, G_BAND_REGION_1},
	{"DO", A_BAND_REGION_0, G_BAND_REGION_0},
	{"EC", A_BAND_REGION_0, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"EG", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"EG", A_BAND_REGION_2, G_BAND_REGION_1},
#endif
	{"SV", A_BAND_REGION_0, G_BAND_REGION_1},
	{"EE", A_BAND_REGION_1, G_BAND_REGION_1},
	{"FI", A_BAND_REGION_1, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"FR", A_BAND_REGION_1, G_BAND_REGION_1},
	{"GE", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"FR", A_BAND_REGION_2, G_BAND_REGION_1},
	{"GE", A_BAND_REGION_2, G_BAND_REGION_1},
#endif
	{"DE", A_BAND_REGION_1, G_BAND_REGION_1},
	{"GR", A_BAND_REGION_1, G_BAND_REGION_1},
	{"GT", A_BAND_REGION_0, G_BAND_REGION_0},
	{"HN", A_BAND_REGION_0, G_BAND_REGION_1},
	{"HK", A_BAND_REGION_0, G_BAND_REGION_1},
	{"HU", A_BAND_REGION_1, G_BAND_REGION_1},
	{"IS", A_BAND_REGION_1, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"IN", A_BAND_REGION_2, G_BAND_REGION_1},
#else
	{"IN", A_BAND_REGION_0, G_BAND_REGION_1},
#endif
	{"ID", A_BAND_REGION_4, G_BAND_REGION_1},
	{"IR", A_BAND_REGION_4, G_BAND_REGION_1},
	{"IE", A_BAND_REGION_1, G_BAND_REGION_1},
	{"IL", A_BAND_REGION_0, G_BAND_REGION_1},
	{"IT", A_BAND_REGION_1, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"JP", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"JP", A_BAND_REGION_9, G_BAND_REGION_1},
#endif
	{"JO", A_BAND_REGION_0, G_BAND_REGION_1},
	{"KZ", A_BAND_REGION_0, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"KP", A_BAND_REGION_1, G_BAND_REGION_1},
	{"KR", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"KP", A_BAND_REGION_5, G_BAND_REGION_1},
	{"KR", A_BAND_REGION_5, G_BAND_REGION_1},
#endif
	{"KW", A_BAND_REGION_0, G_BAND_REGION_1},
	{"LV", A_BAND_REGION_1, G_BAND_REGION_1},
	{"LB", A_BAND_REGION_0, G_BAND_REGION_1},
	{"LI", A_BAND_REGION_1, G_BAND_REGION_1},
	{"LT", A_BAND_REGION_1, G_BAND_REGION_1},
	{"LU", A_BAND_REGION_1, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"MO", A_BAND_REGION_4, G_BAND_REGION_1},
#else
	{"MO", A_BAND_REGION_0, G_BAND_REGION_1},
#endif
	{"MK", A_BAND_REGION_0, G_BAND_REGION_1},
	{"MY", A_BAND_REGION_0, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"MX", A_BAND_REGION_2, G_BAND_REGION_0},
	{"MC", A_BAND_REGION_1, G_BAND_REGION_1},
#else
	{"MX", A_BAND_REGION_0, G_BAND_REGION_0},
	{"MC", A_BAND_REGION_2, G_BAND_REGION_1},
#endif
	{"MA", A_BAND_REGION_0, G_BAND_REGION_1},
	{"NL", A_BAND_REGION_1, G_BAND_REGION_1},
	{"NZ", A_BAND_REGION_0, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"NO", A_BAND_REGION_1, G_BAND_REGION_0},
#else
	{"NO", A_BAND_REGION_0, G_BAND_REGION_0},
#endif
	{"OM", A_BAND_REGION_0, G_BAND_REGION_1},
	{"PK", A_BAND_REGION_0, G_BAND_REGION_1},
	{"PA", A_BAND_REGION_0, G_BAND_REGION_0},
	{"PE", A_BAND_REGION_4, G_BAND_REGION_1},
	{"PH", A_BAND_REGION_4, G_BAND_REGION_1},
	{"PL", A_BAND_REGION_1, G_BAND_REGION_1},
	{"PT", A_BAND_REGION_1, G_BAND_REGION_1},
	{"PR", A_BAND_REGION_0, G_BAND_REGION_0},
	{"QA", A_BAND_REGION_0, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"RO", A_BAND_REGION_1, G_BAND_REGION_1},
	{"RU", A_BAND_REGION_6, G_BAND_REGION_1},
#else
	{"RO", A_BAND_REGION_0, G_BAND_REGION_1},
	{"RU", A_BAND_REGION_0, G_BAND_REGION_1},
#endif
	{"SA", A_BAND_REGION_0, G_BAND_REGION_1},
	{"SG", A_BAND_REGION_0, G_BAND_REGION_1},
	{"SK", A_BAND_REGION_1, G_BAND_REGION_1},
	{"SI", A_BAND_REGION_1, G_BAND_REGION_1},
	{"ZA", A_BAND_REGION_1, G_BAND_REGION_1},
	{"ES", A_BAND_REGION_1, G_BAND_REGION_1},
	{"SE", A_BAND_REGION_1, G_BAND_REGION_1},
	{"CH", A_BAND_REGION_1, G_BAND_REGION_1},
	{"SY", A_BAND_REGION_0, G_BAND_REGION_1},
	{"TW", A_BAND_REGION_3, G_BAND_REGION_0},
	{"TH", A_BAND_REGION_0, G_BAND_REGION_1},
#ifdef RTCONFIG_LOCALE2012
	{"TT", A_BAND_REGION_1, G_BAND_REGION_1},
	{"TN", A_BAND_REGION_1, G_BAND_REGION_1},
	{"TR", A_BAND_REGION_1, G_BAND_REGION_1},
	{"UA", A_BAND_REGION_9, G_BAND_REGION_1},
#else
	{"TT", A_BAND_REGION_2, G_BAND_REGION_1},
	{"TN", A_BAND_REGION_2, G_BAND_REGION_1},
	{"TR", A_BAND_REGION_2, G_BAND_REGION_1},
	{"UA", A_BAND_REGION_0, G_BAND_REGION_1},
#endif
	{"AE", A_BAND_REGION_0, G_BAND_REGION_1},
	{"GB", A_BAND_REGION_1, G_BAND_REGION_1},
	{"US", A_BAND_REGION_0, G_BAND_REGION_0},
#ifdef RTCONFIG_LOCALE2012
	{"UY", A_BAND_REGION_0, G_BAND_REGION_1},
#else
	{"UY", A_BAND_REGION_5, G_BAND_REGION_1},
#endif
	{"UZ", A_BAND_REGION_1, G_BAND_REGION_0},
#ifdef RTCONFIG_LOCALE2012
	{"VE", A_BAND_REGION_4, G_BAND_REGION_1},
#else
	{"VE", A_BAND_REGION_5, G_BAND_REGION_1},
#endif
	{"VN", A_BAND_REGION_0, G_BAND_REGION_1},
	{"YE", A_BAND_REGION_0, G_BAND_REGION_1},
	{"ZW", A_BAND_REGION_0, G_BAND_REGION_1},
	{"", 0, 0}
};

#define NUM_OF_COUNTRIES	(sizeof(allCountry)/sizeof(COUNTRY_CODE_TO_COUNTRY_REGION))

int get_channel_list_via_country(int unit, const char *country_code,
				 char *buffer, int len)
{
	unsigned char *pChannelListTemp = NULL;
	int index, num, i;
	char *p = buffer;
	int band = unit, l = len;

	if (buffer == NULL || len <= 0)
		return -1;

	memset(buffer, 0, len);
	if (band < 0 || band >= MAX_NR_WL_IF)
		return -1;

	for (index = 0; index < NUM_OF_COUNTRIES; index++) {
		if (strncmp((char *)allCountry[index].IsoName, country_code, 2)
		    == 0)
			break;
	}

	if (index >= NUM_OF_COUNTRIES)
		return 0;

	if (band == WL_60G_BAND) {
		num = ARRAY_SIZE(AD_BAND_TMP_REGION_CHANNEL_LIST);
		pChannelListTemp = AD_BAND_TMP_REGION_CHANNEL_LIST;
	} else if (band == 1)
		switch (allCountry[index].RegDomainNum11A) {
		case A_BAND_REGION_0:
			num =
			    sizeof(A_BAND_REGION_0_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_0_CHANNEL_LIST;
			break;
		case A_BAND_REGION_1:
			num =
			    sizeof(A_BAND_REGION_1_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_1_CHANNEL_LIST;
			break;
		case A_BAND_REGION_2:
			num =
			    sizeof(A_BAND_REGION_2_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_2_CHANNEL_LIST;
			break;
		case A_BAND_REGION_3:
			num =
			    sizeof(A_BAND_REGION_3_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_3_CHANNEL_LIST;
			break;
		case A_BAND_REGION_4:
			num =
			    sizeof(A_BAND_REGION_4_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_4_CHANNEL_LIST;
			break;
		case A_BAND_REGION_5:
			num =
			    sizeof(A_BAND_REGION_5_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_5_CHANNEL_LIST;
			break;
		case A_BAND_REGION_6:
			num =
			    sizeof(A_BAND_REGION_6_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_6_CHANNEL_LIST;
			break;
		case A_BAND_REGION_7:
			num =
			    sizeof(A_BAND_REGION_7_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_7_CHANNEL_LIST;
			break;
		case A_BAND_REGION_8:
			num =
			    sizeof(A_BAND_REGION_8_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_8_CHANNEL_LIST;
			break;
		case A_BAND_REGION_9:
			num =
			    sizeof(A_BAND_REGION_9_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_9_CHANNEL_LIST;
			break;
		case A_BAND_REGION_10:
			num =
			    sizeof(A_BAND_REGION_10_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_10_CHANNEL_LIST;
			break;
		case A_BAND_REGION_11:
			num =
			    sizeof(A_BAND_REGION_11_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_11_CHANNEL_LIST;
			break;
		case A_BAND_REGION_12:
			num =
			    sizeof(A_BAND_REGION_12_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_12_CHANNEL_LIST;
			break;
		case A_BAND_REGION_13:
			num =
			    sizeof(A_BAND_REGION_13_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_13_CHANNEL_LIST;
			break;
		case A_BAND_REGION_14:
			num =
			    sizeof(A_BAND_REGION_14_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_14_CHANNEL_LIST;
			break;
		case A_BAND_REGION_15:
			num =
			    sizeof(A_BAND_REGION_15_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_15_CHANNEL_LIST;
			break;
		case A_BAND_REGION_16:
			num =
			    sizeof(A_BAND_REGION_16_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_16_CHANNEL_LIST;
			break;
		case A_BAND_REGION_17:
			num =
			    sizeof(A_BAND_REGION_17_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_17_CHANNEL_LIST;
			break;
		case A_BAND_REGION_18:
			num =
			    sizeof(A_BAND_REGION_18_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_18_CHANNEL_LIST;
			break;
		case A_BAND_REGION_19:
			num =
			    sizeof(A_BAND_REGION_19_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_19_CHANNEL_LIST;
			break;
		case A_BAND_REGION_20:
			num =
			    sizeof(A_BAND_REGION_20_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_20_CHANNEL_LIST;
			break;
		case A_BAND_REGION_21:
			num =
			    sizeof(A_BAND_REGION_21_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = A_BAND_REGION_21_CHANNEL_LIST;
			break;
		default:	// Error. should never happen
			dbg("countryregionA=%d not support",
			    allCountry[index].RegDomainNum11A);
			break;
	} else if (band == 0)
		switch (allCountry[index].RegDomainNum11G) {
		case G_BAND_REGION_0:
			num =
			    sizeof(G_BAND_REGION_0_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = G_BAND_REGION_0_CHANNEL_LIST;
			break;
		case G_BAND_REGION_1:
			num =
			    sizeof(G_BAND_REGION_1_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = G_BAND_REGION_1_CHANNEL_LIST;
			break;
		case G_BAND_REGION_5:
			num =
			    sizeof(G_BAND_REGION_5_CHANNEL_LIST) /
			    sizeof(unsigned char);
			pChannelListTemp = G_BAND_REGION_5_CHANNEL_LIST;
			break;
		default:	// Error. should never happen
			dbg("countryregionG=%d not support",
			    allCountry[index].RegDomainNum11G);
			break;
		}

	if (pChannelListTemp != NULL) {
		for (i = 0; len > 0 && i < num; i++) {
#if 0
			if (i == 0)
				l = snprintf(p, len, "\"%d\"", pChannelListTemp[i]);
			else
				l = snprintf(p, len, ", \"%d\"",
					     pChannelListTemp[i]);
#else
			if (i == 0)
				l = snprintf(p, len, "%d", pChannelListTemp[i]);
			else
				l = snprintf(p, len, ",%d", pChannelListTemp[i]);
#endif
			p += l;
			len -= l;
		}
	}

	return (p - buffer);
}

#ifdef RTCONFIG_POWER_SAVE
#define SYSFS_CPU	"/sys/devices/system/cpu"
#define CPUFREQ		"cpufreq"

void set_cpufreq_attr(char *attr, char *val)
{
	int cpu;
	char path[128], prefix[128];

	if (!attr || !val)
		return;

	for (cpu = 0; cpu < 16; ++cpu) {
		snprintf(prefix, sizeof(prefix), "%s/cpu%d", SYSFS_CPU, cpu);
		if (!d_exists(prefix))
			continue;

		snprintf(path, sizeof(path), "%s/%s/%s", prefix, CPUFREQ, attr);
		if (!f_exists(path)) {
			_dprintf("%s: %s not exist!\n", __func__, path);
			continue;
		}

		f_write_string(path, val, 0, 0);
	}
}

static void set_cpu_power_save_mode(void)
{
#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
	char path[128];

	snprintf(path, sizeof(path), "%s/cpu%d/%s", SYSFS_CPU, 0, CPUFREQ);
	if (!d_exists(path)) {
		_dprintf("%s: cpufreq is not enabled!\n", __func__);
		return;
	}

	switch (nvram_get_int("pwrsave_mode")) {
	case 2:
		/* CPU: powersave - min. freq */
		set_cpufreq_attr("scaling_governor", "powersave");
		break;
	case 1:
		/* CPU: On Demand - auto */
		set_cpufreq_attr("scaling_governor", "ondemand");
		break;
	default:
		/* CPU: performance - max. freq */
		set_cpufreq_attr("scaling_governor", "performance");
		break;
	}
#endif
}

#define PROC_NSS_CLOCK	"/proc/sys/dev/nss/clock"
static void set_nss_power_save_mode(void)
{
#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
	int r;
	unsigned long nss_min_freq = 0, nss_max_freq = 0;
	char path[128], buf[128] = "", nss_freq[16] = "";

	/* NSS */
	if (!d_exists(PROC_NSS_CLOCK)) {
		_dprintf("%s: NSS is not enabled\n", __func__);
		return;
	}

	/* The /proc/sys/dev/nss/clock/freq_table is not readable.
	 * Hardcode NSS min/max freq. based on max. CPU freq. instead.
	 */
	snprintf(path, sizeof(path), "%s/cpu%d/%s/cpuinfo_max_freq", SYSFS_CPU, 0, CPUFREQ);
	if ((r = f_read_string(path, buf, sizeof(buf))) <= 0) {
		return;
	}

#if defined(RTCONFIG_SOC_IPQ8064)
	nss_min_freq = 110 * 1000000;
	if (safe_atoi(buf) == 1400000) {
		nss_max_freq = 733 * 1000000;	/* IPQ8064 */
	} else {
		nss_max_freq = 800 * 1000000;	/* IPQ8065 */
	}
#elif defined(RTCONFIG_SOC_IPQ8074)
	nss_min_freq = 187200000;
	nss_max_freq = nss_min_freq * 8;
#else
#error Unknown NSS frequency.
#endif

	_dprintf("%s: NSS min/max freq: %lu/%lu\n", __func__, nss_min_freq, nss_max_freq);
	if (!nss_min_freq || !nss_max_freq)
		return;

	switch (nvram_get_int("pwrsave_mode")) {
	case 2:
		/* NSS: powersave - min. freq */
		snprintf(path, sizeof(path), "%s/auto_scale", PROC_NSS_CLOCK);
		f_write_string(path, "0", 0, 0);
		snprintf(nss_freq, sizeof(nss_freq), "%lu", nss_min_freq);
		snprintf(path, sizeof(path), "%s/current_freq", PROC_NSS_CLOCK);
		f_write_string(path, nss_freq, 0, 0);
		break;
	case 1:
		/* NSS: On Demand - auto */
		snprintf(path, sizeof(path), "%s/auto_scale", PROC_NSS_CLOCK);
		f_write_string(path, "1", 0, 0);
		break;
	default:
		/* NSS: performance - max. freq */
		snprintf(path, sizeof(path), "%s/auto_scale", PROC_NSS_CLOCK);
		f_write_string(path, "0", 0, 0);
		snprintf(nss_freq, sizeof(nss_freq), "%lu", nss_max_freq);
		snprintf(path, sizeof(path), "%s/current_freq", PROC_NSS_CLOCK);
		f_write_string(path, nss_freq, 0, 0);
		break;
	}
#endif	/* RTCONFIG_SOC_IPQ8064 || RTCONFIG_SOC_IPQ8074 */
}

void set_power_save_mode(void)
{
	set_cpu_power_save_mode();
	set_nss_power_save_mode();
}
#endif	/* RTCONFIG_POWER_SAVE */

/* Return wan_base_if for start_vlan() and selectable upstream port for IPTV.
 * @return:	pointer to base interface name for start_vlan().
 */
char *get_wan_base_if(void)
{
	static char wan_base_if[IFNAMSIZ] = "eth0";

	if (__get_wan_base_if) {
		__get_wan_base_if(wan_base_if);
		return wan_base_if;
	}

#if defined(RTCONFIG_DETWAN)
	char *detwan_ifname;

	if((detwan_ifname = nvram_get("detwan_ifname")) != NULL) {
		strlcpy(wan_base_if, detwan_ifname, sizeof(wan_base_if));
	}
#elif defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2) || defined(RTN19)
	strlcpy(wan_base_if, "eth1", sizeof(wan_base_if));	/* lan_1, WAN interface if IPTV is enabled. */
#endif	/* RTCONFIG_DETWAN */

	return wan_base_if;
}

/* Return nvram variable name, e.g. et1macaddr, which is used to repented as LAN MAC.
 * @return:
 */
char *get_lan_mac_name(void)
{
	char *mac_name = "et1macaddr";	/* Use 5G(+x) MAC address as LAN MAC address. */
	return mac_name;
}

/* Return nvram variable name, e.g. et0macaddr, which is used to repented as WAN MAC.
 * @return:
 */
char *get_wan_mac_name(void)
{
	char *mac_name = "et0macaddr";	/* Use 2G(+x) MAC address as WAN MAC address. */
	return mac_name;
}

char *get_2g_hwaddr(void)
{
#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
	static char mac_str[sizeof("00:00:00:00:00:00XXX")];
	unsigned char mac[ETH_ALEN];

	ether_atoe(nvram_safe_get(get_wan_mac_name()), mac);
	mac[5] &= 0xFC;
	ether_etoa(mac, mac_str);
	return mac_str;
#else
#if defined(RTCONFIG_QCA_VAP_LOCALMAC)
        return nvram_safe_get("wl0macaddr");
#else
        return nvram_safe_get(get_wan_mac_name());
#endif
#endif
}

char *get_5g_hwaddr(void)
{
#if defined(RTCONFIG_SOC_IPQ8064)
	static char mac_str[sizeof("00:00:00:00:00:00XXX")];
	unsigned char mac[ETH_ALEN];

	ether_atoe(nvram_safe_get(get_lan_mac_name()), mac);
	mac[5] &= 0xFC;
	ether_etoa(mac, mac_str);
	return mac_str;
#else
#if defined(RTCONFIG_QCA_VAP_LOCALMAC)
        return nvram_safe_get("wl1macaddr");
#else
        return nvram_safe_get(get_lan_mac_name());
#endif
#endif
}

char *get_label_mac()
{
	return get_2g_hwaddr();
}

char *get_lan_hwaddr(void)
{
        return nvram_safe_get(get_lan_mac_name());
}

char *get_wan_hwaddr(void)
{
        return nvram_safe_get(get_wan_mac_name());
}

/**
 * Generate interface name based on @band and @subunit. (@subunit is NOT y in wlX.Y)
 * @band:
 * @subunit:
 * @buf:
 * @return:
 */
char *__get_wlifname(int band, int subunit, char *buf)
{
	if (!buf)
		return buf;

	if (!subunit)
		strcpy(buf, get_wififname(band));
	else
		sprintf(buf, "%s0%d", get_wififname(band), subunit);

	return buf;
}

/**
 * Check wlX.Y_bss_enabled nvram variable and generate interface name based on
 * wlX.Y_bss_enabled, @band,  and @subunit. (@subunit is NOT y in wlX.Y)
 * @unit:
 * @subunit:
 * @subunit_x:
 * @buf:
 * @return:
 */
char *get_wlifname(int unit, int subunit, int subunit_x, char *buf)
{
#if 1
	char wifbuf[32];
	char prefix[] = "wlXXXXXX_", tmp[100];
	int wlc_band = nvram_get_int("wlc_band");
#if defined(RTCONFIG_WIRELESSREPEATER)
	if (sw_mode() == SW_MODE_REPEATER
#if !defined(RTCONFIG_CONCURRENTREPEATER)
	    && wlc_band == unit
#endif
		 && subunit == 1) {
		strcpy(buf, get_staifname(unit));
		return buf;
	} else
#endif /* RTCONFIG_WIRELESSREPEATER */
#if defined(RTCONFIG_AMAS)
	if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
		/*
		 * wlX.0_bss_enabled: the interface to Upper.
		 * wlX.1_bss_enabled: the interface to Lowwer (main)
		 * wlX.2_bss_enabled: the interface for guest networks 1
		 */
		if (subunit <= 1) {
			strcpy(buf, "");
			return buf;
		}
	}
#endif	/* RTCONFIG_AMAS */
	{
		__get_wlifname(unit, 0, wifbuf);
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
			sprintf(buf, "%s0%d", wifbuf, subunit_x);
		else
			sprintf(buf, "%s", "");
	}
	return buf;
#else
	return __get_wlifname(unit, subunit, buf);
#endif
}

/**
 * Generate VAP interface name of wlX.Y for Guest network, Free Wi-Fi, and Facebook Wi-Fi
 * @x:		X of wlX.Y, aka unit
 * @y:		Y of wlX.Y
 * @buf:	Pointer to buffer of VAP interface name. Must greater than or equal to IFNAMSIZ
 * @return:
 * 	NULL	Invalid @buf
 * 	""	Invalid parameters
 *  otherwise	VAP interface name of wlX.Y
 */
char *get_wlxy_ifname(int x, int y, char *buf)
{
	int i, sidx;
	char prefix[sizeof("wlX.Yxxx")];

	if (!buf)
		return buf;

	if (x < 0 || y < 0 || y >= MAX_NO_MSSID)
		return "";

	if (y == 0) {
		__get_wlifname(x, 0, buf);
		return buf;
	}

	*buf = '\0';
	for (i = 1, sidx = 1; i < MAX_NO_MSSID; ++i) {
		if (i == y) {
			__get_wlifname(x, sidx, buf);
			break;
		}

		snprintf(prefix, sizeof(prefix), "wl%d.%d_", x, i);
		if (nvram_pf_match(prefix, "bss_enabled", "1"))
			sidx++;
	}

	return buf;
}

char *get_wififname(int band)
{
	const char *wif[] = { WIF_2G, WIF_5G, WIF_5G2, WIF_60G };
	if (band < 0 || band >= ARRAY_SIZE(wif)) {
		dbg("%s: Invalid wl%d band!\n", __func__, band);
		band = 0;
	}
	return (char*) wif[band];
}

char *get_staifname(int band)
{
	const char *sta[] = { STA_2G, STA_5G, STA_5G2, STA_60G };
	if (band < 0 || band >= ARRAY_SIZE(sta)) {
		dbg("%s: Invalid wl%d band!\n", __func__, band);
		band = 0;
	}
	return (char*) sta[band];
}

char *get_vphyifname(int band)
{
	const char *vphy[] = { VPHY_2G, VPHY_5G, VPHY_5G2, VPHY_60G };
	if (band < 0 || band >= ARRAY_SIZE(vphy)) {
		dbg("%s: Invalid wl%d band!\n", __func__, band);
		band = 0;
	}
	return (char *) vphy[band];
}

#ifdef RTCONFIG_HAS_5G_2
const char *get_5ghigh_ifname(int *unit)
{
	if(unit != NULL)
		*unit = 1;
	return WIF_5G;
}
#endif


/**
 * Return true if @ifname is main/guest VAP interface name and the band is supported.
 * @return:
 * 	0:	@ifname is not VAP interface name
 * 	1:	@ifname is VAP interface name.
 *  otherwise:	not defined.
 */
int is_vap_ifname(const char *ifname)
{
	int band;
	const char *wif[] = { WIF_2G, WIF_5G, WIF_5G2, WIF_60G };

	if (!ifname)
		return 0;
	for (band = 0; band < min(MAX_NR_WL_IF, ARRAY_SIZE(wif)); ++band) {
		SKIP_ABSENT_BAND(band);

		if (!strncmp(ifname, wif[band], strlen(wif[band])))
			return 1;
	}
	return 0;
}

/**
 * Return true if @ifname is STA interface name and the band is supported.
 * @return:
 * 	0:	@ifname is not STA interface name
 * 	1:	@ifname is STA interface name.
 *  otherwise:	not defined.
 */
int is_sta_ifname(const char *ifname)
{
	int band;
	const char *sta[] = { STA_2G, STA_5G, STA_5G2, STA_60G };

	if (!ifname)
		return 0;
	for (band = 0; band < min(MAX_NR_WL_IF, ARRAY_SIZE(sta)); ++band) {
		SKIP_ABSENT_BAND(band);

		if (!strncmp(ifname, sta[band], strlen(sta[band])))
			return 1;
	}
	return 0;
}

/**
 * Return true if @ifname is main/guest VPHY interface name and the band is supported.
 * @return:
 * 	0:	@ifname is not VPHY interface name
 * 	1:	@ifname is VPHY interface name.
 *  otherwise:	not defined.
 */
int is_vphy_ifname(const char *ifname)
{
	int band;
	const char *vphy[] = { VPHY_2G, VPHY_5G, VPHY_5G2, VPHY_60G };

	if (!ifname)
		return 0;
	for (band = 0; band < min(MAX_NR_WL_IF, ARRAY_SIZE(vphy)); ++band) {
		SKIP_ABSENT_BAND(band);

		if (!strcmp(ifname, vphy[band]))
			return 1;
	}
	return 0;
}

/**
 * Input @band and @ifname and return Y of wlX.Y.
 * Last digit of VAP interface name of guest is NOT always equal to Y of wlX.Y,
 * if guest network is not enabled continuously.
 * @band:
 * @ifname:	ath0, ath1, ath001, ath002, ath103, etc
 * @return:	index of guest network configuration. (wlX.Y: X = @band, Y = @return)
 * 		If both main 2G/5G, 1st/3rd 2G guest network, and 2-nd 5G guest network are enabled,
 * 		return value should as below:
 * 		ath0:	0
 * 		ath001:	1
 * 		ath002: 3
 * 		ath1:	0
 * 		ath101: 2
 */
int get_wlsubnet(int band, const char *ifname)
{
	int subnet, sidx;
	char buf[32];

	for (subnet = 0, sidx = 0; subnet < MAX_NO_MSSID; subnet++)
	{
#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			if(subnet == 1)
				subnet++;
		}
#endif	/* RTCONFIG_AMAS */
		if(!nvram_match(wl_nvname("bss_enabled", band, subnet), "1")) {
			if (!subnet)
				sidx++;
			continue;
		}

		if(strcmp(ifname, __get_wlifname(band, sidx, buf)) == 0)
			return subnet;

		sidx++;
	}
	return -1;
}

int get_wlif_unit(const char *wlifname, int *unit, int *subunit)
{
	int i;
	int _unit = -1, _subunit = -1;
	char *wlif;
	int cmp;
	int len;

	for (i = WL_2G_BAND; i < MAX_NR_WL_IF; ++i) {
		SKIP_ABSENT_BAND(i);

		wlif = get_wififname(i);
		len = strlen(wlif);
		cmp = strncmp(wlifname, wlif, len);
		if(cmp)		/* wlifname is less than wlif */
			continue;
		_unit = i;
		break;
	}
	if (_unit < 0 || absent_band(_unit))
		return -1;

	if(subunit == NULL || strlen(wlifname) == len) {
		_subunit = 0;
	}
	else {
		_subunit = get_wlsubnet(_unit, wlifname);
		if (_subunit < 0)
			_subunit = 0;
	}

	if(unit)
		*unit = _unit;
	if(subunit)
		*subunit = _subunit;

	return 0;
}

#define PROC_NET_WIRELESS  "/proc/net/wireless"


int get_wl_status_ioctl(const char *ifname, int *status, int *quality, int *signal, int *noise, unsigned int *update)
{
	iwstats stats;
	struct iwreq wrq;

	if(ifname == NULL || ifname[0] == '\0' || status == NULL)
		return -1;

	*status = 0;
	if(quality)
		*quality = 0;
	if(signal)
		*signal = 0;
	if(noise)
		*noise = 0;
	if(update)
		*update = 0;

	memset(&stats, 0, sizeof(stats));
	wrq.u.data.pointer = (caddr_t) &stats;
	wrq.u.data.length = sizeof(struct iw_statistics);
	wrq.u.data.flags = 1;					/* Clear updated flag */
	if(wl_ioctl(ifname, SIOCGIWSTATS, &wrq) < 0)
		return -2;

	*status = stats.status;
	if(quality) {
		if(stats.qual.updated & IW_QUAL_QUAL_INVALID)
			*quality = 0;
		else
			*quality = stats.qual.qual;
		if(update)
			*update |= (stats.qual.updated & 1);
	}

	if(signal) {
		if(stats.qual.updated & IW_QUAL_LEVEL_INVALID)
			*signal = 0;
		else if(stats.qual.updated & IW_QUAL_RCPI) {
			double    rcpivalue = (stats.qual.level / 2.0) - 110.0;
			*signal = (int) rcpivalue;
		}
		else {
			*signal = stats.qual.level;
			if(*signal >= 64)
				*signal -= 0x100;	/* convert from unsigned to signed */
		}
		if(update)
			*update |= (stats.qual.updated & 2);
	}

	if(noise) {
		if(stats.qual.updated & IW_QUAL_NOISE_INVALID)
			*noise = 0;
		else if(stats.qual.updated & IW_QUAL_RCPI) {
			double    rcpivalue = (stats.qual.level / 2.0) - 110.0;
			*noise = (int) rcpivalue;
		}
		else {
			*noise = stats.qual.noise;
			if(*noise >= 64)
				*noise -= 0x100;	/* convert from unsigned to signed */
		}
		if(update)
			*update |= (stats.qual.updated & 4);
	}

	return *status;
}

int get_wl_status_proc(const char *ifname, int *status, int *quality, int *signal, int *noise, unsigned int *update)
{
	char line[512];
	FILE *fp;
	int size;
	char *p1, *p2;
	int value;
	int len;

	if(ifname == NULL || ifname[0] == '\0' || status == NULL)
		return -1;

	*status = 0;

	if((fp = fopen(PROC_NET_WIRELESS, "r")) == NULL)
		return -2;

	len = strlen(ifname);
	size = sizeof(line)-1;
	line[size] = '\0';
	if(update)
		*update = 0;

	while(fgets(line, size, fp)) {
		p1 = line;

		strip_new_line(p1);
		skip_space(p1);

		if(strncmp(p1, ifname, len) != 0 || p1[len] != ':')
			continue;

		p1 = p1 + len + 1;		// point to the next char of ':'
		skip_space(p1);
		*status = strtoul(p1, &p2, 16);

		/* Quality */
		p1 = p2;
		skip_space(p1);
		value = strtoul(p1, &p2, 0);
		if(quality) {
			*quality = value;
			if(update && *p2 == '.') {
				p2++;
				*update |= 1;
			}
		}

		/* Signal level */
		p1 = p2;
		skip_space(p1);
		value = strtoul(p1, &p2, 0);
		if(signal) {
			*signal = value;
			if(update && *p2 == '.') {
				p2++;
				*update |= 2;
			}
		}

		/* Noise level */
		p1 = p2;
		skip_space(p1);
		value = strtoul(p1, &p2, 0);
		if(noise) {
			*noise = value;
			if(update && *p2 == '.') {
				p2++;
				*update |= 4;
			}
		}

		break;
	}
	fclose(fp);
	return *status;
}

int get_wl_status(const char *ifname, int *status, int *quality, int *signal, int *noise, unsigned int *update)
{
	int ret;
	pid_t pid;
	if((ret = get_wl_status_ioctl(ifname, status, quality, signal, noise, update)) >= 0)
		return ret;

	pid = getpid();
	cprintf("get_wl_status_ioctl(%s) pid(%s) failed ret(%d)\n", ifname, get_process_name_by_pid(pid), ret);
	if((ret = get_wl_status_proc(ifname, status, quality, signal, noise, update)) >= 0)
		return ret;

	cprintf("get_wl_status_proc(%s) pid(%s) failed ret(%d)\n", ifname, get_process_name_by_pid(pid), ret);
	return ret;
}


int get_ap_mac(const char *ifname, struct iwreq *pwrq)
{
	return wl_ioctl(ifname, SIOCGIWAP, pwrq);
}

const unsigned char ether_zero[6]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const unsigned char ether_bcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

int chk_assoc(const char *ifname)
{
	struct iwreq wrq;
	int ret;

	if((ret = get_ap_mac(ifname, &wrq)) < 0)
		return ret;

#if 0
cprintf("## %s(): ret(%d) ap_addr(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, ret
, wrq.u.ap_addr.sa_data[0], wrq.u.ap_addr.sa_data[1], wrq.u.ap_addr.sa_data[2]
, wrq.u.ap_addr.sa_data[3], wrq.u.ap_addr.sa_data[4], wrq.u.ap_addr.sa_data[5]);
#endif
	if(memcmp(&(wrq.u.ap_addr.sa_data), ether_zero, 6) == 0)
		return 0;	// Not-Associated
	else if(memcmp(&(wrq.u.ap_addr.sa_data), ether_bcast, 6) == 0)
		return -1;	// Invalid

	return 1;
}

#if defined(RTCONFIG_CFG80211)
/**
 * Get PHY name of a cfg80211 based VAP interface.
 * @unit:	wl_unit
 * @iwphy:	buffer that is used to store PHY name.
 * @size:	sizeof @iwphy
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *  otherwise:	error
 */
int get_iwphy_name(int unit, char *iwphy, size_t size)
{
	int r;
	char *p, path[sizeof("/sys/class/net/wifiX/phy80211/nameXXXXXX")];

	if (unit < 0 || unit >= MAX_NR_WL_IF || !iwphy || !size)
		return -1;

	snprintf(path, sizeof(path), "%s/%s/phy80211/name", SYS_CLASS_NET, get_vphyifname(unit));
	r = f_read_string(path, iwphy, size);
	for (p = iwphy + strlen(iwphy) - 1; p >= iwphy; --p) {
		if (isalnum(*p))
			break;
		if (*p == '\r' || *p == '\n')
			*p = 0;
	}

	return (r <= 0)? -2 : 0;
}
#endif

/**
 * Create a VAP interface onto specified VPHY unit.
 * @ifname:	VAP interface name.
 * @unit:	VPHY unit number.
 * @mode:	VAP mode. "ap", or "sta".
 * @return:
 * 	0:	success
 *     -1:	invalid parameter.
 *  otherwise:	error
 */
int create_vap(char *ifname, int unit, char *mode)
{
	char vphy[IFNAMSIZ] = { 0 };
	char *wlanargv[10] = { "wlanconfig", ifname, "create", "wlandev", vphy, "wlanmode", mode, NULL }, **v = &wlanargv[7];
#if defined(RTCONFIG_CFG80211)
	char iwmode[sizeof("managedXXXXX")] = { 0 };
	char iwphy[IFNAMSIZ] = { 0 };
	char *iwargv[] = { "iw", "phy", iwphy, "interface", "add", ifname, "type", iwmode, NULL };
#endif

	if (!ifname || !mode || unit < 0 || unit >= MAX_NR_WL_IF)
		return -1;

#if defined(RTCONFIG_CFG80211)
	if (!strcmp(mode, "ap")) {
	       strlcpy(iwmode, "__ap", sizeof(iwmode));
	} else if (!strcmp(mode, "sta")) {
		strlcpy(iwmode, "managed", sizeof(iwmode));
		*v++ = "nosbeacon";
	} else
		return -1;
#else
	if (strcmp(mode, "ap") && strcmp(mode, "sta"))
		return -1;
#endif

	strlcpy(vphy, get_vphyifname(unit), sizeof(vphy));
#if defined(RTCONFIG_CFG80211)
	*v++ = "-cfg80211";	/* must be last parameter. */
#endif
	*v++ = NULL;

#if defined(RTCONFIG_CFG80211)
	get_iwphy_name(unit, iwphy, sizeof(iwphy));
	dbG("\ncreate a wifi node %s from %s,%s\n", ifname, vphy, iwphy);
	_eval(wlanargv, NULL, 0, NULL);
	_eval(iwargv, NULL, 0, NULL);
#else
	dbG("\ncreate a wifi node %s from %s\n", ifname, vphy);
	_eval(wlanargv, NULL, 0, NULL);
#endif

	return 0;
}

/**
 * Destroy a VAP interface.
 * @ifname:	VAP interface name.
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 */
int destroy_vap(char *ifname)
{
	if (!ifname)
		return -1;

#if defined(RTCONFIG_CFG80211)
	eval("iw", ifname, "del");
#else
	eval("wlanconfig", ifname, "destroy");
#endif

	return 0;
}

int get_ch(int freq)
{
#define IS_CHAN_IN_PUBLIC_SAFETY_BAND(_c) ((_c) > 4940 && (_c) < 4990)
	if (freq < 2412)
		return 0;
	if (freq == 2484)
		return 14;
	if (freq < 2484)
		return (freq - 2407) / 5;
	if (freq < 5000) {
		if (IS_CHAN_IN_PUBLIC_SAFETY_BAND(freq)) {
			return ((freq * 10) +
				(((freq % 5) == 2) ? 5 : 0) - 49400)/5;
		} else if (freq > 4900) {
			return (freq - 4000) / 5;
		} else {
			return 15 + ((freq - 2512) / 20);
		}
	}
	return (freq - 5000) / 5;
}

#ifndef pow
#define pow(v,e) ({double d=1; int i; for(i=0;i<e;i++) d*=v; d;})
#endif

int iwfreq_to_ch(const iwfreq *fr)
{
	double frd;
	int freq;

	frd = ((double) fr->m) * pow(10,fr->e);
	if(frd < 1e3)
		return (int)frd;
	else if(frd < 1e9)
		return -2;

	freq = (int)(frd / 1e6);
	return get_ch(freq);
}

int get_channel(const char *ifname)
{
	struct iwreq wrq;
	const iwfreq *fr;

	if(ifname == NULL)
		return -1;

	if(wl_ioctl(ifname, SIOCGIWFREQ, &wrq))
		return -1;

	fr = &(wrq.u.freq);
	return iwfreq_to_ch(fr);
}

unsigned long long get_bitrate(const char *ifname)
{
	struct iwreq wrq;
	double ratio = 1;

#if defined(RTCONFIG_QCA_BIGRATE_WIFI)
	ratio = 1000;
#endif

	if (ifname == NULL)
		return -1;

	if (wl_ioctl(ifname, SIOCGIWRATE, &wrq))
		return -1;

	return ratio * wrq.u.bitrate.value;
}

/*
 * get_channel_list(ifname, ch_list[], size)
 *
 * get channel list from wifi driver via wl_ioctl(SIOCGIWRANGE)
 * and store the channel list to ch_list[]
 *
 * ch_list[]: an array to save the list of channel that currently used by ifname interface.
 *
 * size: the number of the ch_list[] that can use.
 *
 * return value: a native value for error OR the count of channel in the ch_list[].
 *
 */
int get_channel_list(const char *ifname, int ch_list[], int size)
{
	struct iwreq wrq;
	struct iw_range *range;
	unsigned char buffer[sizeof(iwrange) * 2];	/* Large enough */
	int i;

	if(wl_ioctl(ifname, SIOCGIWNAME, &wrq))		/* check wireless extensions. */
		return -1;

	memset(buffer, 0, sizeof(buffer));
	wrq.u.data.pointer = (caddr_t) buffer;
	wrq.u.data.length = sizeof(buffer);
	wrq.u.data.flags = 0;

	if(wl_ioctl(ifname, SIOCGIWRANGE, &wrq)) {
		cprintf("NOT SUPPORT SIOCGIWRANGE in %s\n", ifname);
		return -1;
	}

	range = (struct iw_range *) buffer;
	if(wrq.u.data.length < 300 || range->we_version_compiled <= 15) {
		cprintf("Wireless Extensions is TOO OLD length(%d) ver(%d)\n", wrq.u.data.length, range->we_version_compiled);
		return -2;
	}

	if (range->num_frequency < size)
		size = range->num_frequency;

	for(i = 0; i < size; i++) {
		//_dprintf("# freq[%2d].i(%d) ch(%d)\n", i, range->freq[i].i, iwfreq_to_ch(&(range->freq[i])));
		ch_list[i] = range->freq[i].i;
	}
	return size;
}

/* dfs channel data from radartool */
#ifndef IEEE80211_CHAN_MAX
#define IEEE80211_CHAN_MAX      1023
#endif

struct dfsreq_nolelem {
    u_int16_t        nol_freq;          /* NOL channel frequency */
    u_int16_t        nol_chwidth;
    unsigned long    nol_start_ticks;   /* OS ticks when the NOL timer started */
    u_int32_t        nol_timeout_ms;    /* Nol timeout value in msec */
};

struct dfsreq_nolinfo {
    u_int32_t   ic_nchans;
    struct      dfsreq_nolelem dfs_nol[IEEE80211_CHAN_MAX];
};

int get_radar_channel_list(const char *vphy, int radar_list[], int size)
{
	const char *dfs_file = "/tmp/dfsreq_nolinfo";
	FILE *fp = NULL;
	int cnt;
	struct dfsreq_nolinfo *nol = NULL;

	if(vphy == NULL || radar_list == NULL || size < 0)
		return -1;

	unlink(dfs_file);
	doSystem("radartool -i %s getnol %s", vphy, dfs_file);

	if((nol = (struct dfsreq_nolinfo *) malloc(sizeof(struct dfsreq_nolinfo))) == NULL)
		return -2;

	if((fp = fopen(dfs_file, "r")) == NULL) {
		free(nol);
		return -3;
	}

	memset(nol, 0, sizeof(struct dfsreq_nolinfo));
	fread(nol, sizeof(struct dfsreq_nolinfo), 1, fp);
	fclose(fp);

	for(cnt = 0; cnt < nol->ic_nchans; cnt++) {
		radar_list[cnt] = get_ch(nol->dfs_nol[cnt].nol_freq);
#if 0
		nol->nol_chwidth
		nol->nol_start_ticks
		nol->nol_timeout_ms
#endif
	}
	free(nol);
	return cnt;
}


char *qca_iwpriv_one_line(const char *ifname, const char *cmd, char *line, int len)
{
	FILE *fp;
	char *p;
	char *answer = NULL;

	snprintf(line, len, IWPRIV " %s %s", ifname, cmd);
	if((fp = popen(line, "r")) != NULL) {
		while(fgets(line, len, fp)) {
			strip_new_line(line);
			if((p = strstr(line, cmd)) == NULL)
				continue;
			if(p == line || !isspace(*(p-1)) || *(p += strlen(cmd)) != ':')
				continue;
			answer = p+1;
			break;
		}
		pclose(fp);
	}
	return answer;
}

/* 
 * int get_bw_nctrlsb(const char *ifname, int *bw, int *nctrlsb)
 *
 * *bw: 
 * 	return the bandwitdh value, could be 20/40/80/160.
 *
 * *nctrlsb:
 * 	return the side band when in HT40 mode
 * 	1: the control sideband is upper
 * 	0: the control sideband is lower
 * 	-1: invalid
 *
 */
int get_bw_nctrlsb(const char *ifname, int *bw, int *nctrlsb)
{
	char line[256];
	char *p;
	char *p2;

	if(ifname == NULL || bw == NULL || nctrlsb == NULL)
		return -1;

	*bw = 0;
	*nctrlsb = -1;
	if((p = qca_iwpriv_one_line(ifname, "get_mode", line, sizeof(line))) == NULL)
		return -2;

	if((p = strstr(p, "11")) == NULL)
		return -3;
	p += 2;

	if((p2 = strstr(p, "HE")) != NULL) {	/* 11AHE, 11GHE */
	}
	else
	if((p2 = strstr(p, "HT")) == NULL) {	/* 11A, 11B, 11G */
		*bw = 20;
		return *bw;
	}
	p = p2 + 2;

	if(memcmp(p, "20", 2) == 0)		/* 11NGHT20, 11NAHT20, 11ACVHT20 */
		*bw = 20;
	else if(memcmp(p, "40", 2) == 0)	/* 11NGHT40, 11NAHT40, 11ACVHT40 */
		*bw = 40;
	else if(memcmp(p, "80_80", 5) == 0)	/* 11ACVHT80_80 */
		*bw = 160;
	else if(memcmp(p, "80", 2) == 0)	/* 11ACVHT80 */
		*bw = 80;
	else if(memcmp(p, "160", 3) == 0)	/* 11ACVHT160 */
		*bw = 160;
	else					/* 11A, 11B, 11G */
		*bw = 20;

	if (*bw == 40) {
		if(strstr(p, "MINUS") != NULL)			/* HT40MINUS */
			*nctrlsb = 1;	//extension channel is lower,  so the control SB is higher
		else if(strstr(p, "PLUS") != NULL)		/* HT40PLUS */
			*nctrlsb = 0;	//extension channel is higher, so the control SB is lower
		else
			return -4;
	}
	return *bw;
}


#if defined(RTAC58U)
extern char *readfile(char *fname,int *fsize);
/* check if /proc/nvram have same mid string
 * @return:
 * 	0:	not found
 * 	1:	matched
 */
int check_mid(char *mid)
{
	char *buf, *pt;
	int fsize, ret=0;
	buf = readfile("/proc/nvram", &fsize);
	if (!buf) return ret;
	pt = strstr(buf, "MID : ");
	if (pt) {
		int len = strlen(mid);
		pt += 6; // len of "MID : "
		if ((memcmp(pt, mid, len)==0) && (*(pt+len) ==  '\n'))
			ret = 1;
	}
	free(buf);
	return ret;
}
#endif

char * get_wpa_ctrl_sk(int band, char ctrl_sk[], int size)
{
	char *sta;
	if(band < 0 || band >= MAX_NR_WL_IF || ctrl_sk == NULL)
		return NULL;
	sta = get_staifname(band);
	snprintf(ctrl_sk, size, "/var/run/wpa_supplicant-%s", sta);
	return ctrl_sk;
}

#define WPA_CLI_REPLY_SIZE		32
#define QUERY_WPA_CLI_REPLY_TIMEOUT	10
#define QUERY_WPA_STATE_TIMEOUT		25
char *wpa_cli_reply(const char *fcmd, char *reply)
{
	FILE *fp;
	int rlen;

	fp = popen(fcmd, "r");
	if (fp) {
		rlen = fread(reply, 1, WPA_CLI_REPLY_SIZE, fp);
		pclose(fp);
		if (rlen > 1) {
			reply[rlen-1] = '\0';
			return reply;
		}
	}

	return "";
}

void set_wpa_cli_cmd(int band, const char *cmd, int chk_reply)
{
	char ctrl_sk[32];
	char *sta;
	char fcmd[128];
	char reply[WPA_CLI_REPLY_SIZE];
	int timeout = QUERY_WPA_CLI_REPLY_TIMEOUT;

	if(band < 0 || band >= MAX_NR_WL_IF || cmd == NULL || cmd[0] == '\0')
		return;

	get_wpa_ctrl_sk(band, ctrl_sk, sizeof(ctrl_sk));
	sta = get_staifname(band);
	if (chk_reply) {
		snprintf(fcmd, sizeof(fcmd), "/usr/bin/wpa_cli -p %s -i %s %s", ctrl_sk, sta, cmd);
		while (strcmp(wpa_cli_reply(fcmd, reply), "OK") && timeout--) {
			//dbg("%s: reply [%s] ...(%d/%d)\n", __func__, reply, timeout, QUERY_WPA_CLI_REPLY_TIMEOUT);
			sleep(1);
		};

		if (strcmp(cmd, "scan") == 0) {
			snprintf(fcmd, sizeof(fcmd), "/usr/bin/wpa_cli -p %s -i %s status | grep wpa_state=", ctrl_sk, sta);
			timeout = QUERY_WPA_STATE_TIMEOUT;
			while (strcmp(wpa_cli_reply(fcmd, reply), "wpa_state=INACTIVE") && timeout--) {
				//dbg("%s: reply [%s] ...(%d/%d)\n", __func__, reply, timeout, QUERY_WPA_STATE_TIMEOUT);
				sleep(1);
			};
		}
	}
	else
		eval("/usr/bin/wpa_cli", "-p", ctrl_sk, "-i", sta, (char*) cmd);
}

void disassoc_sta(char *ifname, char *sta_addr)
{
	if(ifname == NULL || *ifname == '\0' || sta_addr == NULL || *sta_addr == '\0')
		return;

	eval(IWPRIV, ifname, "kickmac", sta_addr);
}


/*
 * mode: 0:disable, 1:allow, 2:deny
 */
void set_maclist_mode(char *ifname, int mode)
{
	char qca_maccmd[32];
	char str_mode[16];
	char *sec = "";

	if(ifname == NULL || *ifname == '\0')
		return;

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		sec = "_sec";
#endif // WIFI_SON
	sprintf(qca_maccmd, "%s%s", QCA_MACCMD, sec);

	if (mode < 0 || mode > 3)
		mode = 0;
	snprintf(str_mode, sizeof(str_mode), "%d", mode);

	eval(IWPRIV, ifname, qca_maccmd, str_mode);
}

void set_maclist_add_kick(char *ifname, int mode, char *sta_addr)
{
	char qca_addmac[32];
	char *sec = "";

	if(ifname == NULL || *ifname == '\0' || sta_addr == NULL || *sta_addr == '\0')
		return;

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		sec = "_sec";
#endif // WIFI_SON
	snprintf(qca_addmac, sizeof(qca_addmac), "%s%s", QCA_ADDMAC, sec);

	eval(IWPRIV, ifname, qca_addmac, sta_addr);

	if(mode == 2)
		disassoc_sta(ifname, sta_addr);
}

void set_maclist_del_kick(char *ifname, int mode, char *sta_addr)
{
	char qca_delmac[32];
	char *sec = "";

	if(ifname == NULL || *ifname == '\0' || sta_addr == NULL || *sta_addr == '\0')
		return;

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		sec = "_sec";
#endif // WIFI_SON
	snprintf(qca_delmac, sizeof(qca_delmac), "%s%s", QCA_DELMAC, sec);

	eval(IWPRIV, ifname, qca_delmac, sta_addr);

	if(mode == 1)
		disassoc_sta(ifname, sta_addr);
}


void set_macfilter_unit(int unit, int subnet, FILE *fp)
{
	char prefix[sizeof("wlXXXXXX_")], tmp_prefix[sizeof("wlXXXXXX_")];
	char tmp[32];
	char athfix[8];
	char *sec = "";
	char qca_mac[32];
	int i, j, max_subnet, mode;	/* 0: disable, 1: allow, 2: deny */
	char *p;

#ifdef RTCONFIG_AMAS
	if (subnet <=0 && nvram_get_int("re_mode") == 1)
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
	else
#endif
	if (subnet > 0) {
		max_subnet = num_of_mssid_support(unit);
		for (j = 0, i = 1; i <= max_subnet; i++) {
			snprintf(tmp_prefix, sizeof(tmp_prefix), "wl%d.%d_", unit, i);
			if (!nvram_pf_match(tmp_prefix, "bss_enabled", "1"))
				continue;

			j++;
			if (j == subnet)
				strlcpy(prefix, tmp_prefix, sizeof(prefix));
		}
	} else
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	__get_wlifname(swap_5g_band(unit), subnet, athfix);

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1"))
		sec = "_sec";
#endif // WIFI_SON
	sprintf(qca_mac, "%s%s", QCA_MACCMD, sec);

	p = nvram_get(strcat_r(prefix, "macmode", tmp));
	/* mode: 0: disable, 1: allow, 2: deny */
	if (p && strcmp(p, "deny") == 0)
		mode = 2;
	else if (p && strcmp(p, "allow") == 0)
		mode = 1;
	else
		mode = 0;

	fprintf(fp, IWPRIV " %s %s %d\n", athfix, qca_mac, 3);	/* flush old list */
	fprintf(fp, IWPRIV " %s %s %d\n", athfix, qca_mac, mode);

	if (mode) {
		char *nv, *nvp, *mac;

		nv = nvp = strdup(nvram_safe_get(strcat_r(prefix, "maclist_x", tmp)));
		if (nv) {
			sprintf(qca_mac, "%s%s", QCA_ADDMAC, sec);
			while ((mac = strsep(&nvp, "<")) != NULL) {
				if(*mac == '\0')
					continue;
				fprintf(fp, IWPRIV " %s %s %s\n", athfix, qca_mac, mac);
			}
			free(nv);
		}
	}
}

void set_macfilter_all(FILE *fp)
{
	int unit;
	char *wl_ifnames;
	char word[16], *next;

	unit = 0;
	wl_ifnames = strdup(nvram_safe_get("wl_ifnames"));
	if(wl_ifnames == NULL)
		return;

	foreach (word, wl_ifnames, next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);

		set_macfilter_unit(unit, 0, fp);
		unit++;
	}
	free(wl_ifnames);
}

/** Return temperature of specific Wireless band via thermaltool.
 * @band:
 * @return:
 * 	>= 0	temperature of specific Wireless band.
 * 	<  0	error
 */
int get_wifi_temperature(enum wl_band_id band)
{
	int t = 0;
	char cmd[sizeof("thermaltool -i wifi0 -getXXXXXX")];

	if (band >= MAX_NR_WL_IF)
		return -1;

	/* Example:
	 * Thermal config for wifi0
	 *   enable: 1, dc: 100, dc_per_event: 2
	 *   level: 0, low thresold: -100, high thresold: 110, dcoffpercent: 0, queue priority 7, policy; 1
	 *   level: 1, low thresold: 100, high thresold: 120, dcoffpercent: 50, queue priority 7, policy; 1
	 *   level: 2, low thresold: 110, high thresold: 135, dcoffpercent: 90, queue priority 7, policy; 1
	 *   level: 3, low thresold: 125, high thresold: 150, dcoffpercent: 100, queue priority 7, policy; 1
	 * Thermal stats for wifi0
	 *   sensor temperature: 41, current level: 0
	 *   level: 0, entry count: 0, duty cycle spent: 1000
	 *   level: 1, entry count: 0, duty cycle spent: 0
	 *   level: 2, entry count: 0, duty cycle spent: 0
	 *   level: 3, entry count: 0, duty cycle spent: 0
	 */

	snprintf(cmd, sizeof(cmd), "thermaltool -i %s -get", get_vphyifname(band));
	if (exec_and_parse(cmd, "sensor temperature:", "%*[^:]:%d%*[^\n]", 1, &t))
		return 0;

	return t;
}
