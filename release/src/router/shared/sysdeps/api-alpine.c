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
#include <alpine.h>
#include <iwlib.h>
#include "utils.h"
#include "shutils.h"
#include <shared.h>
#include <trxhdr.h>
#include <bcmutils.h>
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
    u_int32_t       ic_flags;       /* see below */
    u_int8_t        ic_flagext;     /* see below */
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
    return (freq - 5000) / 5;
}
/////////////

#if defined(RTCONFIG_ALPINE)
const char WIF_5G[] = "wifi2_0";
const char WIF_2G[] = "wifi0_0";
const char STA_5G[] = "wifi2_0";
const char STA_2G[] = "wifi0_0";
const char VPHY_5G[] = "wifi2_0";
const char VPHY_2G[] = "wifi0_0";
#else
#error Define WiFi 2G/5G interface name!
#endif

#define GPIOLIB_DIR	"/sys/class/gpio"

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
	sprintf(gpio_path, "%s/gpio%d", GPIOLIB_DIR, gpio);
	if (d_exists(gpio_path))
		return 0;

	sprintf(export_path, "%s/export", GPIOLIB_DIR);
	sprintf(gpio_str, "%d", gpio);
	f_write_string(export_path, gpio_str, 0, 0);

	return 0;
}

uint32_t gpio_dir(uint32_t gpio, int dir)
{
	char path[PATH_MAX], v[10], *dir_str = "in";

	if (dir == GPIO_DIR_OUT) {
		dir_str = "out";		/* output, low voltage */
		*v = '\0';
		sprintf(path, "%s/gpio%d/value", GPIOLIB_DIR, gpio);
		if (f_read_string(path, v, sizeof(v)) > 0 && atoi(v) == 1)
			dir_str = "high";	/* output, high voltage */
	}

	__export_gpio(gpio);
	sprintf(path, "%s/gpio%d/direction", GPIOLIB_DIR, gpio);
	f_write_string(path, dir_str, 0, 0);

	return 0;
}

uint32_t get_gpio(uint32_t gpio)
{
	char path[PATH_MAX], value[10];

	sprintf(path, "%s/gpio%d/value", GPIOLIB_DIR, gpio);
	f_read_string(path, value, sizeof(value));

	return atoi(value);
}

uint32_t set_gpio(uint32_t gpio, uint32_t value)
{
	char path[PATH_MAX], val_str[10];

	sprintf(val_str, "%d", !!value);
	sprintf(path, "%s/gpio%d/value", GPIOLIB_DIR, gpio);
	f_write_string(path, val_str, 0, 0);

	return 0;
}

int get_switch_model(void)
{
	// TODO
	return SWITCH_UNKNOWN;
}

uint32_t get_phy_status(int wan_unit)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_ifname;
	char buf[32];

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
	wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if(strlen(wan_ifname) <= 0)
		return 0;

	snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/operstate", wan_ifname);

	f_read_string(tmp, buf, sizeof(buf));
	if(!strncmp(buf, "up", 2))
		return 1;
	else
		return 0;
}

uint32_t get_phy_speed(int wan_unit)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_ifname;
	char buf[32];

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
	wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if(strlen(wan_ifname) <= 0)
		return 0;

	snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/speed", wan_ifname);

	f_read_string(tmp, buf, sizeof(buf));
	return strtoul(buf, NULL, 10);
}

uint32_t set_phy_ctrl(int wan_unit, int ctrl)
{
	// TODO
	return 1;		/* FIXME */
}

/* 0: it is not a legal image
 * 1: it is legal image
 */
int check_imageheader(char *buf, long *filelen)
{
	long aligned;

#ifdef HND_ROUTER
	return 1;
#endif
	if (strncmp(buf, IMAGE_HEADER, sizeof(IMAGE_HEADER) - 1) == 0)
	{
		memcpy(&aligned, buf + sizeof(IMAGE_HEADER) - 1, sizeof(aligned));
		*filelen = aligned;
#ifdef RTCONFIG_DSL_TCLINUX
		*filelen+=0x790000;
#endif
		_dprintf("image len: %x[%08X]\n", aligned, aligned);
		return 1;
	}
	else return 0;
}

/*
 * 0: legal image
 * 1: illegal image
 * 2: new trx format validation failure
 *
 * check product id, crc ..
 */

int check_imagefile(char *fname)
{
	FILE *fp;
	struct tail_t {
		version_t kernel;		/* Kernel version */
		version_t fs;			/* Filsystem version */
#ifdef HND_ROUTER
		uint16_t  sn;
		uint16_t  en;
		char pid[MAX_PID_LEN];
		uint8_t pad[36];
#else
		uint8_t pid[MAX_PID_LEN];	/* Product Id */
		uint8_t hw[MAX_HW_COUNT][4];	/* Compatible hw list lo maj.min, hi maj.min */
#ifdef TRX_NEW
		uint16_t sn;
		uint16_t en;
		uint8_t key;
#endif
		uint8_t	pad[27];		/* Padding up to MAX_TAIL_LEN */
#endif
	} tail;
#ifdef HND_ROUTER
	WFI_TAG wt;
#endif
	int i, model = get_model();

	fp = fopen(fname, "r");
	if (fp == NULL)
		return 1;

#ifdef HND_ROUTER
	fseek(fp, -(MAX_TAIL_LEN + TOKEN_LEN), SEEK_END);
#else
	fseek(fp, -MAX_TAIL_LEN, SEEK_END);
#endif
	fread(&tail, 1, MAX_TAIL_LEN, fp);
#ifdef HND_ROUTER
	fread(&wt, 1, TOKEN_LEN, fp);
#endif
	fclose(fp);

	_dprintf("productid field in image: %.12s\n", tail.pid);

	for (i = 0; i < sizeof(tail); i++)
		_dprintf("%02x ", ((uint8_t *)&tail)[i]);
	_dprintf("\n");

	/* safe strip trailing spaces */
	for (i = 0; i < MAX_PID_LEN && tail.pid[i] != '\0'; i++);
	for (i--; i >= 0 && tail.pid[i] == '\x20'; i--)
		tail.pid[i] = '\0';

#ifdef HND_ROUTER
	dumpWfiTag(&wt);

	if (wt.wfiCrc != img_crc_hnd(fname)) {
		_dprintf("check crc error!!!\n");
		return 1;
	}
#else
	if (!checkcrc(fname)) {
		_dprintf("check crc error!!!\n");
		return 1;
	}
#endif

#ifdef TRX_NEW
	if (!check_trx(fname, tail.key))
		return 2;

#ifdef RTCONFIG_BCMARM
	doSystem("nvram set cpurev=`cat /dev/mtd0 | grep cpurev | cut -d \"=\" -f 2`");
	if (nvram_match("cpurev", "c0") &&
	   (!tail.sn ||
	    !tail.en ||
	     tail.sn < 380 ||
	    (tail.sn == 380 && tail.en < 738)))
	{
		dbg("version check fail!\n");
		return 2;
	}
#endif
#endif

	/* compare up to the first \0 or MAX_PID_LEN
	 * nvram productid or hw model's original productid */
	if (strncmp(nvram_safe_get("productid"), (char *) tail.pid, MAX_PID_LEN) == 0
#ifndef RTCONFIG_NONASUS
	 || strncmp(get_modelid(model), (char *) tail.pid, MAX_PID_LEN) == 0
#elif defined(RTCONFIG_ODMPID)
	 && strcmp(nvram_safe_get("odmpid"), "") == 0
#endif
	)
	{
		_dprintf("correct model name\n");
		return 0;
	}

	/* common RT-N12 productid FW image */
	if ((model == MODEL_RTN12B1 || model == MODEL_RTN12C1 ||
	     model == MODEL_RTN12D1 || model == MODEL_RTN12VP || model == MODEL_RTN12HP || model == MODEL_RTN12HP_B1 ||model == MODEL_APN12HP) &&
	     strncmp(get_modelid(MODEL_RTN12), (char *) tail.pid, MAX_PID_LEN) == 0)
		return 0;

	return 1;
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

char *wl_vifname_qtn(int unit, int subunit)
{
	static char tmp[128];
	int qtn_unit = 0; // by default

	if(unit == 0)
		qtn_unit=2;	/* 2G */
	else if(unit ==1)
		qtn_unit=0;	/* 5G */

	if ((subunit > 0) && (subunit < 4))
	{
		sprintf(tmp, "wifi%d_%d", qtn_unit, subunit);
		return strdup(tmp);
	}else{
		sprintf(tmp, "wifi%d_0", qtn_unit);
		return strdup(tmp);
	}
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
	int led = (!unit)? LED_2G:LED_5G, onoff = (!on)? LED_OFF:LED_ON;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX", athfix[]="athXXXXXX";
	char path[sizeof(NAWDS_SH_FMT) + 6];

	if (subunit > 0)
	{   
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
//		snprintf(athfix, sizeof(athfix), "ath%d0%d", unit, subunit);
	}	
	else
	{   
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
//		snprintf(athfix, sizeof(athfix), "ath%d", unit);
	}
	strcpy(athfix, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

	if (*athfix != '\0')
	{   
	   	if(!strstr(athfix,"sta")) //all lan-interfaces except sta when running repeater mode
			eval("ifconfig", athfix, on? "up" : "down");

		/* Reconnect to peer WDS AP */
		sprintf(path, NAWDS_SH_FMT, unit? WIF_5G : WIF_2G);
		if (!subunit && !nvram_match(strcat_r(prefix, "mode_x", tmp), "0") && f_exists(path))
			doSystem(path);
	}

	led_control(led, onoff);
}

char *wif_to_vif(char *wif)
{
	static char vif[32];
	int unit = 0, subunit = 0;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";

	vif[0] = '\0';

	for (unit = 0; unit < 2; unit++) {
		for (subunit = 1; subunit < 4; subunit++) {
			snprintf(prefix, sizeof(prefix), "wl%d.%d", unit, subunit);

			if (nvram_match(strcat_r(prefix, "_ifname", tmp), wif)) {
				sprintf(vif, "%s", prefix);
				goto RETURN_VIF;
			}
		}
	}

RETURN_VIF:
	return vif;
}

/* get channel list via currently setting in wifi driver */
int get_channel_list_via_driver(int unit, char *buffer, int len)
{
	struct ieee80211req_chaninfo chans;
	struct iwreq wrq;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_", *ifname;
	int i;
	char *p;

	if (buffer == NULL || len <= 0)
		return -1;

	memset(buffer, 0, len);
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	memset(&wrq, 0, sizeof(wrq));
	wrq.u.data.pointer = (void *)&chans;
	wrq.u.data.length = sizeof(chans);
	if (wl_ioctl(ifname, IEEE80211_IOCTL_GETCHANINFO, &wrq) < 0)
		return -1;

	for (i = 0, p=buffer; i < chans.ic_nchans ; i++) {
		if (i == 0)
			p += sprintf(p, "%u", ieee80211_mhz2ieee(chans.ic_chans[i].ic_freq));
		else
			p += sprintf(p, ",%u", ieee80211_mhz2ieee(chans.ic_chans[i].ic_freq));
	}
	return (p - buffer);
}

int qc98xx_verify_checksum(void *eeprom)
{
#if 1
	printf("skip qc98xx_verify_checksum()\n");
#else
    unsigned short *p_half;
    unsigned short sum = 0;
    int i;

    p_half = (unsigned short *)eeprom;
    for (i = 0; i < QC98XX_EEPROM_SIZE_LARGEST / 2; i++) {
        sum ^= __le16_to_cpu(*p_half++);
    }
    if (sum != 0xffff) {
        return -1;
    }
#endif
    return 0;
}

int calc_qca_eeprom_csum(void *ptr, unsigned int eeprom_size)
{
#if 1
	printf("skip calc_qca_eeprom_csum()\n");
#else
	int i;
	uint16_t *p = ptr, sum = 0;

	if (!ptr || (eeprom_size & 1)) {
		_dprintf("%s: invalid param. (ptr %p, eeprom_size %u)\n",
			__func__, ptr, eeprom_size);
		return -1;
	}

	*(p + 1) = 0;
	for (i = 0; i < (eeprom_size / 2); ++i, ++p)
		sum ^= __le16_to_cpu(*p);

	p = ptr;
	*(p + 1) = __cpu_to_le16(sum ^ 0xFFFF);
#endif

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
	int band = unit;

	if (buffer == NULL || len <= 0)
		return -1;

	memset(buffer, 0, len);
	if (band != 0 && band != 1)
		return -1;

	for (index = 0; index < NUM_OF_COUNTRIES; index++) {
		if (strncmp((char *)allCountry[index].IsoName, country_code, 2)
		    == 0)
			break;
	}

	if (index >= NUM_OF_COUNTRIES)
		return 0;

	if (band == 1)
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
		for (i = 0; i < num; i++) {
#if 0
			if (i == 0)
				p += sprintf(p, "\"%d\"", pChannelListTemp[i]);
			else
				p += sprintf(p, ", \"%d\"",
					     pChannelListTemp[i]);
#else
			if (i == 0)
				p += sprintf(p, "%d", pChannelListTemp[i]);
			else
				p += sprintf(p, ",%d", pChannelListTemp[i]);
#endif
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
#if defined(RTCONFIG_SOC_IPQ8064)
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
#if defined(RTCONFIG_SOC_IPQ8064)
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

	nss_min_freq = 110 * 1000000;
	if (atoi(buf) == 1400000) {
		nss_max_freq = 733 * 1000000;	/* IPQ8064 */
	} else {
		nss_max_freq = 800 * 1000000;	/* IPQ8065 */
	}

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
#endif	/* RTCONFIG_SOC_IPQ8064 */
}

void set_power_save_mode(void)
{
	set_cpu_power_save_mode();
	set_nss_power_save_mode();
}
#endif	/* RTCONFIG_POWER_SAVE */

/* Return nvram variable name, e.g. et0macaddr, which is used to repented as LAN MAC.
 * @return:
 */
char *get_lan_mac_name(void)
{
	int model = get_model();
	char *mac_name = "et0macaddr";

	/* Check below configuration in convert_wan_nvram() too. */
	switch (model) {
	case MODEL_GTAC9600:	/* fall-through */
		/* Use 5G MAC address as LAN MAC address. */
		mac_name = "et0macaddr";
		break;
	default:
		dbg("%s: Define LAN MAC address for model %d\n", __func__, model);
		mac_name = "et0macaddr";
		break;
	};

	return mac_name;
}

char *get_label_mac()
{
	return get_2g_hwaddr();
}

/* Return nvram variable name, e.g. et0macaddr, which is used to repented as WAN MAC.
 * @return:
 */
char *get_wan_mac_name(void)
{
	int model = get_model();
	char *mac_name = "et0macaddr";

	/* Check below configuration in convert_wan_nvram() too. */
	switch (model) {
	case MODEL_GTAC9600:	/* fall-through */
		/* Use 2G MAC address as LAN MAC address. */
		mac_name = "et0macaddr";
		break;
	default:
		dbg("%s: Define WAN MAC address for model %d\n", __func__, model);
		mac_name = "et0macaddr";
		break;
	};

	return mac_name;
}

char *get_lan_hwaddr(void)
{
        return nvram_safe_get(get_lan_mac_name());
}

char *get_2g_hwaddr(void)
{
	return nvram_safe_get(get_lan_mac_name());
}

char *get_wan_hwaddr(void)
{
        return nvram_safe_get(get_wan_mac_name());
}

char *get_wlifname(int unit, int subunit, int subunit_x, char *buf)
{
	sprintf(buf, "wl%d.%d", unit, subunit);

	return buf;
}

/**
 * Generate VAP interface name of wlX.Y for Guest network, Free Wi-Fi, and Facebook Wi-Fi
 * @x:		X of wlX.Y, aka unit
 * @y:		Y of wlX.Y
 * @buf:	Pointer to buffer of VAP interface name. Must greater than or equal to IFNAMSIZ
 * @return:
 */
char *get_wlxy_ifname(int x, int y, char *buf)
{
	return get_wlifname(x, y, y, buf);
}

#ifdef RTCONFIG_AMAS
void add_beacon_vsie(char *hexdata)
{
}

void del_beacon_vsie(char *hexdata)
{
}

void add_obd_probe_req_vsie(char *hexdata)
{
}

void del_obd_probe_req_vsie(char *hexdata)
{
}
#endif /* RTCONFIG_AMAS */
