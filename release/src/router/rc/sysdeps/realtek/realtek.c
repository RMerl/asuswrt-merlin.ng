/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>
#include <string.h>
#include <bcmnvram.h>
#include <net/if_arp.h>
#include <shutils.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <etioctl.h>
#include <rc.h>
typedef u_int64_t __u64;
typedef u_int32_t __u32;
typedef u_int16_t __u16;
typedef u_int8_t __u8;
#include <linux/sockios.h>
#include <linux/ethtool.h>
#if defined(RTCONFIG_REALTEK)
#include "../../../shared/sysdeps/realtek/realtek.h"
//TODO
#include "../../../../../realtek/rtl819x/linux-3.10/drivers/net/wireless/rtl8192cd/ieee802_mib.h"
#include "mib_adapter/rtk_wifi_drvmib.h"
#include "../../../../../realtek/rtl819x/linux-3.10/include/generated/autoconf.h"
#else
#include <ctype.h>
#endif
#include <wlutils.h>
#include <shared.h>
#include <wlscan.h>

#if defined(RTCONFIG_REALTEK)

#define IWCONTROL_PID_FILE "/var/run/iwcontrol.pid"
#define PATHSEL_PID_FILE1 "/var/run/pathsel-wlan0.pid"
#define PATHSEL_PID_FILE2 "/var/run/pathsel-wlan1.pid"
#define PATHSEL_PID_FILE3 "/var/run/pathsel-wlan-msh.pid"
#define MAX_CHECK_PID_NUM 10
#define FOR_DUAL_BAND 1

#define WIF_2G_IFNAME "wl0"
#define WIF_5G_IFNAME "wl1"

#define	MAX_STA_COUNT	128

struct apinfo apinfos[MAX_NUMBER_OF_APINFO];
char buf[WLC_IOCTL_MAXLEN];

/*
	led api for asus
	
	void set_led(int wl0_stage, int wl1_stage)

	when wl0_stage/wl1_stage is
	LED_OFF_ALL:	turn off all led
	LED_ON_ONE:		turn on one led
	LED_ON_TWO:		turn on two led
	LED_ON_ALL:		turn on all led
	LED_SHOW_SIG_STR: base on wl%d-vxd's rssi, 
		if rssi > 50 means Best signal quality, turn on all led
		else if rssi < 20 means Bad signal quality, turn on one led
		else means Workable signal quality , turn on two led
	LED_SHOW_SIG_STR2: base on wl%d's rssi, 
		if rssi > 50 means Best signal quality, turn on all led
		else if rssi < 20 means Bad signal quality, turn on one led
		else means Workable signal quality , turn on two led

	when wl1_stage is
	LED_BLINK_FAST:		0.1s on/0.1s off
	LED_BLINK_QUICK:	0.2s on/0.1s off
	LED_BLINK_NORNAL:	0.5s on/0.1s off
	LED_BLINK_SLOW:		1sec on/1sec off
	LED_BLINK_BOOT:		2sec on/2sec off

	ps. blink mode will only take the value of wl1_stage.
*/
void set_led(int wl0_stage, int wl1_stage)
{
#if defined(RPAC68U)
	char buf[32];
	sprintf(buf, "echo 'led %d %d' > /proc/asus_ate", wl0_stage, wl1_stage);
	system(buf);
#endif
	return;
}

#if 0	//move the below to router/shared/sysdeps/realtek/realtek.h
typedef struct {
	const char *name;
	unsigned char band_2G;
	unsigned char band_5G;
	unsigned short txpwr_lmt_index;
} reg_domain_t;

/*
 *
 * static const COUNTRY_IE_ELEMENT countryIEArray[]
 * realtek/rtl819x/linux-2.6.30/drivers/net/wireless/rtl8192cd/8192cd_11h.c
 * src-rt-6.x/router/shared/sysdeps/realtek/realtek.h
 *
 * the country and reg mapping in countryIEArray is not up to date.
 *
 */
static const reg_domain_t reg_domain[] = {
	{ "US", DOMAIN_FCC,  DOMAIN_FCC,  0  },
	{ "CA", DOMAIN_FCC,  DOMAIN_FCC,  3  },
	{ "EU", DOMAIN_ETSI, DOMAIN_ETSI, 0  },
//	{ "TW", DOMAIN_NCC,  DOMAIN_NCC,  0  },
	{ "TW", DOMAIN_FCC,  DOMAIN_FCC,  0  },
	{ "SG", DOMAIN_SG,   DOMAIN_SG,   0  },
	{ "CN", DOMAIN_CN,   DOMAIN_CN,   0  },
	{ "KR", DOMAIN_KR,   DOMAIN_KR,   0  },
	{ "JP", DOMAIN_MKK,  DOMAIN_MKK,  0  },
	{ "AU", DOMAIN_ETSI,  DOMAIN_FCC, 4  },
};
#endif

void chk_valid_country_code(char *country_code)
{
	if ((unsigned char)country_code[0]!=0x0) 
	{
		int i = 0, num = sizeof(reg_domain)/sizeof(reg_domain_t);
		int found = 0;
		while (i < num) {
			if (strcmp(country_code, reg_domain[i].name) == 0) {
				found = i;
				break;
			}
			i++;
		}

		if (!found)
			strcpy(country_code, "US");
	}
	else
	{
		strcpy(country_code, "US");
	}
}

void set_country_code()
{
	char country_code[3];
	int cc_offset = HW_SETTING_OFFSET;

	memset(country_code, 0, sizeof(country_code));
	cc_offset += sizeof(PARAM_HEADER_T);
	cc_offset += (int)(&((struct hw_setting *)0)->countryCode);

	rtk_flash_read(country_code, cc_offset, 2);
	
	chk_valid_country_code(country_code);
	nvram_set("wl_country_code", country_code);
	nvram_set("wl0_country_code", country_code);
#ifdef RTCONFIG_HAS_5G
	nvram_set("wl1_country_code", country_code);
#endif
}

#ifdef RTCONFIG_TCODE
void set_territory_code()
{
	unsigned char tc_buf[6];
	int tc_offset = HW_SETTING_OFFSET;

	memset(tc_buf, 0, sizeof(tc_buf));
	tc_offset += sizeof(PARAM_HEADER_T);
	tc_offset += (int)(&((struct hw_setting *)0)->territoryCode);

	rtk_flash_read(tc_buf, tc_offset, 5);
	if ((unsigned char)tc_buf[0] != 0x0)
		nvram_set("territory_code", tc_buf);
	else
		nvram_unset("territory_code");
}
#endif

void set_txpwr_lmt_index(char *wl_ifname)
{
	char *country_code = NULL;
	char prefix[] = "wlXXXXXXXXXX_";
	char tmp[128] = {0};

	snprintf(prefix, sizeof(prefix), "%s_", wl_ifname);
	country_code = nvram_safe_get(strcat_r(prefix, "country_code", tmp));

	if (country_code == NULL)
		return;

	int i = 0, num = sizeof(reg_domain)/sizeof(reg_domain_t);
	while (i < num) {
		if (strcmp(country_code, reg_domain[i].name) == 0) {

			sprintf(tmp, "iwpriv %s set_mib txpwr_lmt_index=%d", wl_ifname, reg_domain[i].txpwr_lmt_index);
			doSystem(tmp);
			return;
		}
		i++;
	}
}

void usb3_enable(int en) {
	if (en == 1) {
		system("echo write 0xb80000f0 0x0f000000 > /proc/rtl865x/memory");
		system("echo 'usb3_en 1' > /proc/asus_ate");
	}
	else {
		system("echo 'usb3_en 0' > /proc/asus_ate");
	}

	system("echo 'xhci 0' > /proc/usb");
	system("echo 'xhci 1' > /proc/usb");

	if(!en)
	{
		system("echo write 0xb80000f0 0x0e000000 > /proc/rtl865x/memory");
	}
}

int set40M_Channel_2G(char *channel)
{
}

int set40M_Channel_5G(char *channel)
{
}

int ResetDefault(void)
{
#if defined(RPAC68U) // NAND flash
	eval("rm", "-f", "/hw_setting/nvram.bin");
	eval("umount", "/hw_setting");

	eval("rm", "-f", "/hw_setting2/nvram.bin");
	eval("umount", "/hw_setting2");
#else // SPI flash
	eval("mtd-erase","-d","nvram");
#endif
	puts("1");
}

char *wlc_nvname(char *keyword)
{
	return(wl_nvname(keyword, nvram_get_int("wlc_band"), -1));
}

int setWlOffLed(void)
{
}

#ifdef RTCONFIG_WIRELESSREPEATER


int get_wlc_status(char *interface)
{
	bss_info bss_buf;
	char bssid[8];
	memset(&bss_buf,0,sizeof(bss_info));
	if (wl_ioctl(interface, WLC_GET_BSSID, bssid, ETHER_ADDR_LEN) < 0 ||
	    wl_ioctl(interface, WLC_GET_BSS_INFO, &bss_buf, sizeof(bss_buf)) < 0)
		return 0;
	//printf("*******%s %d interface=%s state=%d channel=%d txRate=%d rssi=%d ssid=%s bssid=%x%x%x%x%x%x********\n",__FUNCTION__,__LINE__,interface,bss_buf.state,bss_buf.channel,bss_buf.txRate,bss_buf.rssi,bss_buf.ssid,bss_buf.bssid[0],bss_buf.bssid[1],bss_buf.bssid[2],bss_buf.bssid[3],bss_buf.bssid[4],bss_buf.bssid[5]);
	return (bss_buf.state);
}

void update_wlc_nvram(int band, int ret)
{
	char buf[32] = {0};
	char *wlc_band = NULL;
	int sw_mode = sw_mode();
	if (band)
		sprintf(buf, "%s", "wlc1_state");
	else
		sprintf(buf, "%s", "wlc0_state");
	
	if (ret == 4)
		nvram_set_int(buf, WLC_STATE_CONNECTED);
	else
		nvram_set_int(buf, WLC_STATE_STOPPED);
	
#ifdef RTCONFIG_CONCURRENTREPEATER
	int wlc_express = nvram_get_int("wlc_express");
	if (sw_mode == SW_MODE_REPEATER && wlc_express != 0) { // express mode

		if (band) // 5G
			nvram_set_int("wlc0_state", WLC_STATE_STOPPED); // Set 2.4G
		else // 2.4G
			nvram_set_int("wlc1_state", WLC_STATE_STOPPED); // Set 5G
		return;
	}

	int ret2 = 0;
	char ifname[32] = {0};
	if (band) {
		sprintf(buf, "%s", "wlc0_state");
		if (sw_mode == SW_MODE_REPEATER)
			sprintf(ifname, "%s", "wl0-vxd");
		else
			sprintf(ifname, "%s", "wl0");
	}
	else {
		sprintf(buf, "%s", "wlc1_state");
		if (sw_mode == SW_MODE_REPEATER)
			sprintf(ifname, "%s", "wl1-vxd");
		else
			sprintf(ifname, "%s", "wl1");
	}

	ret2 = get_wlc_status(ifname);
	if (ret2 == 4)
		nvram_set_int(buf, WLC_STATE_CONNECTED);
	else
		nvram_set_int(buf, WLC_STATE_STOPPED);
#endif
}

int wlcconnect_core(void)
{	// return WLC connection status
	if (sw_mode() != SW_MODE_REPEATER && (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") != 1))
		return WLC_STATE_INITIALIZING;
	int ret = 0;
	char *band_tmp = NULL;
	int band;
	char vxdifname[16];
	char wl0_buf[16], wl1_buf[16];
	int wlc_express = nvram_get_int("wlc_express");

	if (sw_mode() == SW_MODE_REPEATER && wlc_express == 0) {
		strcpy(wl0_buf, "wl0-vxd");
		strcpy(wl1_buf, "wl1-vxd");
	}
	else {
		strcpy(wl0_buf, "wl0");
		strcpy(wl1_buf, "wl1");
	}

	band_tmp = nvram_safe_get("wlc_band");
	if (!strcmp(band_tmp, "0"))
		band = 0;
	else if (!strcmp(band_tmp, "1"))
		band = 1;
	else
		band = -1;

#ifdef RTCONFIG_CONCURRENTREPEATER
	if (wlc_express == 0)
#endif
	{
		if (band == 0 || band == 1) { // Single band check
			if(band == 1)
				strcpy(vxdifname,wl1_buf);
			else if (band == 0)
				strcpy(vxdifname,wl0_buf);

				ret = get_wlc_status(vxdifname);
		}
		else {
			band = 0; // renew the variable for update_wlc_nvram()
			strcpy(vxdifname, wl0_buf);
			ret = get_wlc_status(vxdifname);
			if (ret != 4) {
				band = 1; // renew the variable for update_wlc_nvram()
				strcpy(vxdifname, wl1_buf);
				ret = get_wlc_status(vxdifname);
			}	
		}
	}
#ifdef RTCONFIG_CONCURRENTREPEATER
	else {
		if (wlc_express == 1)
			strcpy(vxdifname, wl0_buf);
		else
			strcpy(vxdifname, wl1_buf);

		ret = get_wlc_status(vxdifname);
		band = wlc_express - 1; // renew the variable for update_wlc_nvram()
	}
#endif

	update_wlc_nvram(band, ret);
	if (ret == 4)
		return WLC_STATE_CONNECTED;
	else if (ret == 2)
		return WLC_STATE_CONNECTING;
	else
		return WLC_STATE_INITIALIZING;	
}

unsigned int get_conn_link_quality(int unit)
{
	int link_quality = 0;
	const char *aif;
	struct maclist *mac_list;
	int mac_list_size;
	scb_val_t scb_val;
	int wlc_express = nvram_get_int("wlc_express");

	if(unit == 0) {
		if (mediabridge_mode() || (repeater_mode() && wlc_express != 0)) 
			aif = WIF_2G_IFNAME;
		else
			aif = VXD_2G;
	}
	else {
		if (mediabridge_mode() || (repeater_mode() && wlc_express != 0))
			aif = WIF_5G_IFNAME;
		else
			aif = VXD_5G;
	}

	mac_list_size = sizeof(mac_list->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	mac_list = malloc(mac_list_size);
	if (!mac_list)
		return 0;

	memset(mac_list, 0, mac_list_size);
	/* query wl for authenticated sta list */
	strcpy((char*) mac_list, "authe_sta_list");
	if (wl_ioctl(aif, WLC_GET_VAR, mac_list, mac_list_size)) {
		free(mac_list);
		return 0;
	}

	if(mac_list->count == 0) {
		free(mac_list);
		return 0;
	}
			
	memcpy(&scb_val.ea, &mac_list->ea[0], ETHER_ADDR_LEN);
	if(wl_ioctl(aif, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t))) {
		free(mac_list);
		return 0;
	}

	link_quality = scb_val.val;
	free(mac_list);

	return link_quality;
}
#endif

static void __inline__ WRITE_WPA_FILE(int fh, unsigned char *buf)
{
	if ( write(fh, buf, strlen((char *)buf)) != strlen((char *)buf) ) {
		rtkerr("Write WPA config file error!\n");
		close(fh);
		exit(1);
	}
}

#define WRITE_WSC_PARAM(dst, tmp, str, val) {	\
	sprintf(tmp, str, val); \
	memcpy(dst, tmp, strlen(tmp)); \
	dst += strlen(tmp); \
}
static void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}

static int get_wsc_auth(int wl_index,int *wsc_auth, int* wsc_encrypt)
{
	char tmp[128], prefix[] = "wlXXXXXXX_";
	char *auth_mode,*crypto_mode;
	rtk_printf("%s \n",__FUNCTION__);
	if(!(wl_index == 0||wl_index == 1)|| wsc_auth == NULL || wsc_encrypt == NULL)
	{
		return -1;
	}
	sprintf(prefix,"wl%d_",wl_index);
	auth_mode = nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp));
	crypto_mode = nvram_safe_get(strcat_r(prefix,"crypto",tmp));
	if(!strcmp(auth_mode,"open"))/*open*/
	{
		*wsc_auth = WSC_AUTH_OPEN;
		*wsc_encrypt = WSC_ENCRYPT_NONE;
	}
	else if(!strcmp(auth_mode,"pskpsk2"))/*WPA auto personal*/
	{
		*wsc_auth = WSC_AUTH_WPA2PSKMIXED;
		if(!strcmp(crypto_mode,"aes"))/*WPA auto AES*/
		{
			*wsc_encrypt = WSC_ENCRYPT_AES;
		}
		else if(!strcmp(crypto_mode,"tkip+aes"))/*WPA auto TKIP+AES*/
		{
			*wsc_encrypt = WSC_ENCRYPT_TKIPAES;
		}
		else
		{
			return -1;
		}
	}
	else if(!strcmp(auth_mode,"psk2"))/*WPA2 AES*/
	{
		*wsc_auth = WSC_AUTH_WPA2PSK;
		*wsc_encrypt = WSC_ENCRYPT_AES;
	}
	else
	{
		return -1;
	}
}
static int updateWscConf(char *in, char *out, int genpin, char *wlanif_name,int client_mode)
{
	rtklog("%s \n",__FUNCTION__);
    int fh;
    struct stat status;
    char *buf, *ptr, *nvram_ptr;
    int intVal;
	int mixed;
    //int intVal2;
    int wlan0_mode=0;
    int wlan1_mode=0;    
    int is_config, is_registrar, len, is_wep=0;
    char tmpbuf[100], tmp1[100];
	int sw_mode = 0;
	int wlc_psta = 0;
    //int is_repeater_enabled=0;
    int isUpnpEnabled=0, wsc_method = 0, wsc_auth=0, wsc_enc=0;
    int wlan_network_type=0, wsc_manual_enabled=0, wlan_wep=0;
    int wlan_chan_num=0, wsc_config_by_ext_reg=0;
    char crypto[100], wlan_ssid[100], device_name[100], wsc_pin[100], wlan_wpa_psk[100],auth_mode[100];
    rtklog("\n\n       wlanif_name=[%s]  \n\n\n",wlanif_name);
    // 1104	
    int wlan0_wlan_disabled=0;	
    int wlan1_wlan_disabled=0;		
    int wlan0_wsc_disabled=0;
	int wps_band = 0;
	int wlc_band = 0;
	int wps_configured = 0;
#ifdef FOR_DUAL_BAND	
    int wlan1_wsc_disabled=0;
#endif
	unsigned char PIN[9];

	/*for detial mixed mode info*/ 
#define WSC_WPA_TKIP		1
#define WSC_WPA_AES			2
#define WSC_WPA2_TKIP		4
#define WSC_WPA2_AES		8

    int wlan0_encrypt=0;		
    int wlan0_wpa_cipher=0;
    int wlan0_wpa2_cipher=0;

    int wlan1_encrypt=0;		
    int wlan1_wpa_cipher=0;
    int wlan1_wpa2_cipher=0;	
    /*for detial mixed mode info*/ 
    char *token=NULL, *token1=NULL, *savestr1=NULL;   
	sw_mode = sw_mode();
	wlc_psta = nvram_get_int("wlc_psta");
	if(sw_mode != SW_MODE_REPEATER && (sw_mode == SW_MODE_AP && wlc_psta != 1))/*not repeater mode and media bridge mode */
	{
	    if(wlanif_name != NULL) {
	        token = strtok_r(wlanif_name," ", &savestr1);
	        if(token)
	            token1 = strtok_r(NULL," ", &savestr1);
	    }
	    else {
	        token = "wl0";
	    }
	}
	else
	{
		token = wlanif_name;
	}
	memset(PIN, 0, sizeof(PIN));
	int offset = HW_SETTING_OFFSET;
	offset += sizeof(PARAM_HEADER_T);
	offset += (int)(&((struct hw_setting *)0)->wlan);
	offset += (int)(&((struct hw_wlan_setting *)0)->wscPin);
	rtk_flash_read(PIN,offset,8);

	wsc_manual_enabled = 0;
	nvram_ptr = nvram_safe_get("productid");
	strcpy(device_name,nvram_ptr);

	rtklog("device_name=[%s]\n",device_name);
	int multi_band = nvram_get_int("wps_multiband");
	if(sw_mode != SW_MODE_REPEATER && (sw_mode == SW_MODE_AP && wlc_psta != 1))/*not repeater mode and media bridge mode */
	{
		isUpnpEnabled = 1;
		if(multi_band)
		{
			wlan0_wsc_disabled = 0;
			wlan1_wsc_disabled = 0;
		}
		else
		{
			wps_band = nvram_get_int("wps_band_x");
			if(wps_band ==0)
			{
				wlan1_wsc_disabled = 1;
			}
			else
			{
				wlan0_wsc_disabled = 1;
			}
		}
	}
	else if(sw_mode == SW_MODE_ROUTER)/*router mode*/
	{
		if(nvram_get_int("wps_enable") == 1)
		{
			wps_band = nvram_get_int("wps_band_x");
			if(wps_band == 0)
			{
				wlan0_wsc_disabled = 0;
				wlan1_wsc_disabled = 1;
			}
			else if(wps_band == 1)
			{
				wlan0_wsc_disabled = 1;
				wlan1_wsc_disabled = 0;
			}
		}
	}
	else/*repeater mode and media bridge*/
	{
		wlc_band = nvram_get_int("wlc_band");
		if(client_mode)/*current interface is client interface*/
		{
			rtklog("%s client mode\n",out);
			isUpnpEnabled = 0;
			if(!strcmp(wlanif_name,"wl0-vxd") || !strcmp(wlanif_name, "wl0"))
			{
				wlan1_wsc_disabled = 1;
			}
			else
			{
				wlan0_wsc_disabled = 1;
			}
		}
		else/*current interface is not client interface*/
		{
			rtklog("%s not client mode\n",out);
			isUpnpEnabled = 1;
			rtklog("wlc_band:%d\n",wlc_band);
			if(wlc_band == 0)
			{
				wlan0_wsc_disabled = 1;
			}
			else
			{
				wlan1_wsc_disabled = 1;
			}
		}
	}
	
	rtklog("out file :%s wlan0_wsc_disabled:%d,wlan1_wsc_disabled:%d",
			out,wlan0_wsc_disabled,wlan1_wsc_disabled);
	
	if(strcmp(nvram_get("wl_wps_reg"),"enabled")==0)
	{
		is_registrar = 1;
	}
	wsc_method = 0;

	wsc_config_by_ext_reg = 0;


    if (stat(in, &status) < 0) {
        rtkerr("stat() error [%s]!\n", in);
        return -1;
    }

    buf = malloc(status.st_size+2048);
    if (buf == NULL) {
        rtkerr("malloc() error [%d]!\n", (int)status.st_size+2048);
        return -1;		
    }

    ptr = buf;
	wps_configured = nvram_get_int("w_Setting");
	if(client_mode == 0)/*AP interface*/
	{
		if(wps_configured)
		{
			if(is_registrar)
			{
				intVal = MODE_AP_PROXY_REGISTRAR;
			}
			else
			{
				intVal = MODE_AP_PROXY;
			}
		}
		else
		{
			intVal = MODE_AP_UNCONFIG;
		}
	}
	else
	{
		intVal = MODE_CLIENT_UNCONFIG;
	}
    WRITE_WSC_PARAM(ptr, tmpbuf, "mode = %d\n", intVal);

    WRITE_WSC_PARAM(ptr, tmpbuf, "upnp = %d\n", isUpnpEnabled);
	
	wsc_method = 3;
	intVal = wsc_method;
#ifdef CONFIG_RTL_P2P_SUPPORT
    if(wlan0_mode == P2P_SUPPORT_MODE){

        intVal = ( CONFIG_METHOD_PIN | CONFIG_METHOD_PBC | CONFIG_METHOD_DISPLAY |CONFIG_METHOD_KEYPAD);


    }else
#endif
    {
        //Ethernet(0x2)+Label(0x4)+PushButton(0x80) Bitwise OR
        if (intVal == 1) //Pin+Ethernet
            intVal = (CONFIG_METHOD_KEYPAD | CONFIG_METHOD_VIRTUAL_PIN);
        else if (intVal == 2) //PBC+Ethernet
            intVal = (CONFIG_METHOD_KEYPAD | CONFIG_METHOD_PHYSICAL_PBC | CONFIG_METHOD_VIRTUAL_PBC);
        if (intVal == 3) //Pin+PBC+Ethernet
            intVal = (CONFIG_METHOD_KEYPAD | CONFIG_METHOD_VIRTUAL_PIN | CONFIG_METHOD_PHYSICAL_PBC | CONFIG_METHOD_VIRTUAL_PBC);
    }
	WRITE_WSC_PARAM(ptr, tmpbuf, "config_method = %d\n", intVal);

#if 1
	if ((sw_mode == SW_MODE_REPEATER && client_mode) || (sw_mode == SW_MODE_AP && wlc_psta == 1 && client_mode))
	{
		if (wlan_network_type == 0)
			intVal = 1;
		else
			intVal = 2;
	}
	else
		intVal = 1;


    WRITE_WSC_PARAM(ptr, tmpbuf, "connection_type = %d\n", intVal);

    WRITE_WSC_PARAM(ptr, tmpbuf, "manual_config = %d\n", wsc_manual_enabled);

    WRITE_WSC_PARAM(ptr, tmpbuf, "pin_code = %s\n", PIN);
#endif


	intVal = 2;//always set to 2
    WRITE_WSC_PARAM(ptr, tmpbuf, "rf_band = %d\n", intVal);
	nvram_ptr = nvram_get("wl0_auth_mode_x");
	strcpy(auth_mode,nvram_ptr);
	nvram_ptr = nvram_get("wl0_crypto");//encryption
	strcpy(crypto,nvram_ptr);
	nvram_ptr = nvram_get("wl0_wpa_psk");
	strcpy(wlan_wpa_psk,nvram_ptr);
	nvram_ptr = nvram_get("wl0_ssid");
	strcpy(wlan_ssid,nvram_ptr);
	get_wsc_auth(0,&wsc_auth,&wsc_enc);
	mixed = 0;
	if(wsc_auth == WSC_AUTH_WPA2PSKMIXED)
	{
		mixed = 1;
	}
    WRITE_WSC_PARAM(ptr, tmpbuf, "#=====wlan0 start==========%d\n", wlan0_wsc_disabled);    
    // 1104
    WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_wsc_disabled = %d\n", wlan0_wsc_disabled);

    WRITE_WSC_PARAM(ptr, tmpbuf, "auth_type = %d\n", wsc_auth);

    WRITE_WSC_PARAM(ptr, tmpbuf, "encrypt_type = %d\n", wsc_enc);

	WRITE_WSC_PARAM(ptr, tmpbuf, "mixedmode = %d\n", mixed);	
	//rtk_printf("[%s %d],mixedmode = 0x%02x\n", __FUNCTION__,__LINE__,intVal);	
	/*for detial mixed mode info*/ 


	WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", wlan_wpa_psk);		

	WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = \"%s\"\n", wlan_ssid);

    WRITE_WSC_PARAM(ptr, tmpbuf, "#=====wlan0 end==========:%d\n", wlan0_wsc_disabled);


#ifdef FOR_DUAL_BAND
	
	nvram_ptr = nvram_get("wl1_auth_mode_x");
	strcpy(auth_mode,nvram_ptr);
	nvram_ptr = nvram_get("wl1_wpa_psk");
	strcpy(wlan_wpa_psk,nvram_ptr);
	nvram_ptr = nvram_get("wl1_ssid");
	strcpy(wlan_ssid,nvram_ptr);

	mixed = 0;
	get_wsc_auth(1,&wsc_auth,&wsc_enc);
	if(wsc_auth == WSC_AUTH_WPA2PSKMIXED)
	{
		mixed = 1;
	}

    WRITE_WSC_PARAM(ptr, tmpbuf, "#=====wlan1 start==========%d\n",wlan1_wsc_disabled);
    WRITE_WSC_PARAM(ptr, tmpbuf, "ssid2 = \"%s\"\n",wlan_ssid );	
    WRITE_WSC_PARAM(ptr, tmpbuf, "auth_type2 = %d\n", wsc_auth);
    WRITE_WSC_PARAM(ptr, tmpbuf, "encrypt_type2 = %d\n", wsc_enc);
	WRITE_WSC_PARAM(ptr, tmpbuf, "mixedmode2 = %d\n", mixed);	

	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan1_wsc_disabled = %d\n", wlan1_wsc_disabled);		
	WRITE_WSC_PARAM(ptr, tmpbuf, "network_key2 = %s\n", wlan_wpa_psk);		

	WRITE_WSC_PARAM(ptr, tmpbuf, "#=====wlan1 end==========%d\n", wlan1_wsc_disabled);

	/*sync the PIN code of wlan0 and wlan1*/
	//apmib_set(MIB_HW_WSC_PIN, (void *)wsc_pin);//TODO

	
#endif	// END of FOR_DUAL_BAND
	WRITE_WSC_PARAM(ptr, tmpbuf, "device_name = \"%s\"\n", device_name);
	
	WRITE_WSC_PARAM(ptr, tmpbuf, "config_by_ext_reg = %d\n", wsc_config_by_ext_reg);

	if (sw_mode == SW_MODE_AP && wlc_psta != 1) { // AP mode

		int disable_auto_gen_ssid = 1; // Disable auto gen SSID.
		WRITE_WSC_PARAM(ptr, tmpbuf, "disable_auto_gen_ssid = %d\n", disable_auto_gen_ssid);
	}

	len = (int)(((long)ptr)-((long)buf));
	
	fh = open(in, O_RDONLY);
	if (fh == -1) {
		rtkerr("open() error [%s]!\n", in);
		return -1;
	}

	lseek(fh, 0L, SEEK_SET);
	if (read(fh, ptr, status.st_size) != status.st_size) {		
		rtkerr("read() error [%s]!\n", in);
		return -1;	
	}
	close(fh);

	// search UUID field, replace last 12 char with hw mac address
	ptr = strstr(ptr, "uuid =");
	if (ptr) {
		char tmp2[100];
		unsigned char buffer[6];
		char macaddr[18];
		int offset = HW_SETTING_OFFSET;
		memset(buffer, 0, sizeof(buffer));
		memset(macaddr, 0, sizeof(macaddr));
		
		offset += sizeof(PARAM_HEADER_T);
		offset += (int)(&((struct hw_setting *)0)->wlan);
		offset += sizeof(struct hw_wlan_setting);
		offset += (int)(&((struct hw_wlan_setting *)0)->macAddr);
		rtk_flash_read(buffer,offset,6);
		convert_bin_to_str(buffer, 6, tmp2);
		memcpy(ptr+27, tmp2, 12);		
	}

	fh = open(out, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		rtk_printf("open() error [%s]!\n", out);
		return -1;
	}

	if (write(fh, buf, len+status.st_size) != len+status.st_size ) {
		rtk_printf("Write() file error [%s]!\n", out);
		return -1;
	}
	close(fh);
	free(buf);
	
	return 0;
}
static int updateWpaConf(char* iface, char* outputFile, int isWds)
{
	int fh, intVal = 0, encrypt, enable1x, wep;
	unsigned char buf1[1024], buf2[1024];
	int index = 0;
	char nvram_para[64] = {0};
	char *temp;
	
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
	int wlan_mode;
#endif
	if(iface == NULL || outputFile == NULL)
	{
		rtkerr("Invalid parameter!\n");
		return -1;
	}
	if(strcmp(iface,"wl0") == 0)
	{
		index = 0;
	}
	else if(strcmp(iface,"wl1") == 0)
	{
		index  = 1;
	}
	else
	{
		rtkerr("Invalid interface name!\n");
		return -1;
	}
	
	fh = open(outputFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		rtkerr("Create WPA config file error!\n");
		return;
	}

	
	if (!isWds) {

	sprintf(nvram_para,"wl%d_auth_mode_x",index);
	if(nvram_match(nvram_para, "wpa"))
		sprintf((char *)buf2, "encryption = 2\n");
	else if(nvram_match(nvram_para, "wpa2"))
		sprintf((char *)buf2, "encryption = 4\n");
	else if(nvram_match(nvram_para, "wpawpa2"))
		sprintf((char *)buf2, "encryption = 6\n");	
	else if(nvram_match(nvram_para, "radius"))
		sprintf((char *)buf2, "encryption = 1\n");	
	WRITE_WPA_FILE(fh, buf2);
	
	sprintf(nvram_para,"wl%d_ssid",index);
	temp = nvram_get(nvram_para);
	sprintf((char *)buf2, "ssid = \"%s\"\n", temp);
	WRITE_WPA_FILE(fh, buf2);

	sprintf(nvram_para,"wl%d_auth_mode_x",index);
	if(nvram_match(nvram_para, "wpa2") || 
	   nvram_match(nvram_para, "wpawpa2")||
	   nvram_match(nvram_para, "wpa")||
	   nvram_match(nvram_para, "radius"))
		sprintf((char *)buf2, "enable1x = 1\n");
	else
		sprintf((char *)buf2, "enable1x = 0\n");	
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_MAC_AUTH_ENABLED, (void *)&intVal);
	sprintf((char *)buf2, "enableMacAuth = %d\n", intVal);//Disable
	WRITE_WPA_FILE(fh, buf2);

#ifdef CONFIG_IEEE80211W	
	//apmib_get( MIB_WLAN_IEEE80211W, (void *)&intVal);
	sprintf((char *)buf2, "ieee80211w = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
	
	//apmib_get( MIB_WLAN_SHA256_ENABLE, (void *)&intVal);
	sprintf((char *)buf2, "sha256 = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
#endif
	//apmib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal);
	//if (intVal)
		//apmib_get( MIB_WLAN_SUPP_NONWPA, (void *)&intVal);
		
	sprintf((char *)buf2, "supportNonWpaClient = %d\n", intVal);//TODO
	WRITE_WPA_FILE(fh, buf2);

	sprintf((char *)buf2, "wepKey = 1\n");//TODO
	WRITE_WPA_FILE(fh, buf2);

	if ( encrypt==1 && enable1x ) {
		if (wep == 1) {
			//apmib_get( MIB_WLAN_WEP64_KEY1, (void *)buf1);//TODO
			sprintf((char *)buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x\"\n", buf1[0],buf1[1],buf1[2],buf1[3],buf1[4]);
		}
		else {
			//apmib_get( MIB_WLAN_WEP128_KEY1, (void *)buf1);//TODO
			sprintf((char *)buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"\n",
				buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],
				buf1[5],buf1[6],buf1[7],buf1[8],buf1[9],
				buf1[10],buf1[11],buf1[12]);
		}
	}
	else
		strcpy((char *)buf2, "wepGroupKey = \"\"\n");
	WRITE_WPA_FILE(fh, buf2);

	sprintf(nvram_para,"wl%d_auth_mode_x",index);
	if(nvram_match(nvram_para, "wpa2") || 
	   nvram_match(nvram_para, "wpawpa2")||
	   nvram_match(nvram_para, "wpa"))
		sprintf((char *)buf2, "authentication = 1\n");//DOT11_AuthKeyType_RSN
	 else if(nvram_match(nvram_para, "radius"))
		 sprintf((char *)buf2, "authentication = 2\n");
	WRITE_WPA_FILE(fh, buf2);


	sprintf(nvram_para,"wl%d_crypto",index);
	if(nvram_match(nvram_para, "tkip+aes"))
		sprintf((char *)buf2, "unicastCipher = 3\n");
	else
		sprintf((char *)buf2, "unicastCipher = 2\n");
	WRITE_WPA_FILE(fh, buf2);


	sprintf(nvram_para,"wl%d_crypto",index);
	if(nvram_match(nvram_para, "aes"))
		sprintf((char *)buf2, "wpa2UnicastCipher = 2\n");//AES only
	else if(nvram_match(nvram_para, "tkip+aes"))
		sprintf((char *)buf2, "wpa2UnicastCipher = 3\n"); // TKIP+AES
	
	WRITE_WPA_FILE(fh, buf2);

	sprintf((char *)buf2, "enablePreAuth = 0\n");
	WRITE_WPA_FILE(fh, buf2);

	if (intVal==0)
		sprintf((char *)buf2, "usePassphrase = 1\n");
	else
		sprintf((char *)buf2, "usePassphrase = 0\n");
	WRITE_WPA_FILE(fh, buf2);

	sprintf(nvram_para,"wl%d_wpa_psk",index);
	temp = nvram_get(nvram_para);
	sprintf((char *)buf2, "psk = \"%s\"\n", temp);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
	sprintf(nvram_para,"wl%d_wpa_gtk_rekey",index);
	temp = nvram_get(nvram_para);
	sprintf((char *)buf2, "groupRekeyTime = %s\n", temp);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_RS_REAUTH_TO, (void *)&intVal);
	sprintf((char *)buf2, "rsReAuthTO = %d\n", intVal);//TODO
	WRITE_WPA_FILE(fh, buf2);

#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
	if (wlan_mode == CLIENT_MODE) { // wlan client mode		
		//apmib_get( MIB_WLAN_EAP_TYPE, (void *)&intVal);
		sprintf(buf2, "eapType = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		//apmib_get( MIB_WLAN_EAP_INSIDE_TYPE, (void *)&intVal);
		sprintf(buf2, "eapInsideType = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		//apmib_get( MIB_WLAN_EAP_USER_ID, (void *)buf1);
		sprintf(buf2, "eapUserId = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);

		//apmib_get( MIB_WLAN_RS_USER_NAME, (void *)buf1);
		sprintf(buf2, "rsUserName = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);

		//apmib_get( MIB_WLAN_RS_USER_PASSWD, (void *)buf1);
		sprintf(buf2, "rsUserPasswd = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);

		//apmib_get( MIB_WLAN_RS_USER_CERT_PASSWD, (void *)buf1);
		sprintf(buf2, "rsUserCertPasswd = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);

		//apmib_get( MIB_WLAN_RS_BAND_SEL, (void *)&intVal);
		sprintf(buf2, "rsBandSel = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		//Patch for auth daemon at wlan client mode
		// 127.0.0.1 : 12345
		if(intVal == 2)/*PHYBAND_5G*/
			sprintf(buf2, "rsPort = %d\n", 12344);
		else
			sprintf(buf2, "rsPort = %d\n", 12345);
		WRITE_WPA_FILE(fh, buf2);

		sprintf(buf2, "rsIP = %s\n", "127.0.0.1");
		WRITE_WPA_FILE(fh, buf2);
		//End patch.
	}
	else
#endif
	{
	sprintf(nvram_para,"wl%d_radius_port",index);
	intVal = nvram_get_int(nvram_para);
	sprintf((char *)buf2, "rsPort = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	sprintf(nvram_para,"wl%d_radius_ipaddr",index);
	temp = nvram_get(nvram_para);
	sprintf((char *)buf2, "rsIP = %s\n", temp);
	WRITE_WPA_FILE(fh, buf2);

	sprintf(nvram_para,"wl%d_radius_key",index);
	temp = nvram_get(nvram_para);
	sprintf((char *)buf2, "rsPassword = \"%s\"\n", temp);
	WRITE_WPA_FILE(fh, buf2);

	intVal = 3;
	sprintf((char *)buf2, "rsMaxReq = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	intVal = 5;
	sprintf((char *)buf2, "rsAWhile = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
#ifdef CONFIG_APP_AUTH_2NDSRV
	//apmib_get( MIB_WLAN_RS2_PORT, (void *)&intVal);
	sprintf((char *)buf2, "rs2Port = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_RS2_IP, (void *)buf1);
	sprintf((char *)buf2, "rs2IP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_RS2_PASSWORD, (void *)buf1);
	sprintf((char *)buf2, "rs2Password = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);
#endif

	//apmib_get( MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&intVal);
	intVal = 0;//TBD
	sprintf((char *)buf2, "accountRsEnabled = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_PORT, (void *)&intVal);
	intVal = 0;//TBD
	sprintf((char *)buf2, "accountRsPort = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_IP, (void *)buf1);
	strcpy(buf1,"0.0.0.0");//TBD
	sprintf((char *)buf2, "accountRsIP = %s\n", buf);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)buf1);
	strcpy(buf1,"");//TBD
	sprintf((char *)buf2, "accountRsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

#ifdef CONFIG_APP_AUTH_2NDSRV
	//apmib_get( MIB_WLAN_ACCOUNT_RS2_PORT, (void *)&intVal);
	sprintf((char *)buf2, "accountRs2Port = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS2_IP, (void *)buf1);
	sprintf((char *)buf2, "accountRs2IP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS2_PASSWORD, (void *)buf1);
	sprintf((char *)buf2, "accountRs2Password = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);
#endif

	//apmib_get( MIB_WLAN_ACCOUNT_RS_UPDATE_ENABLED, (void *)&intVal);
	intVal = 0;//TBD
	sprintf((char *)buf2, "accountRsUpdateEnabled = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_UPDATE_DELAY, (void *)&intVal);
	intVal = 0;//TBD
	sprintf((char *)buf2, "accountRsUpdateTime = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_MAXRETRY, (void *)&intVal);
	intVal = 0;//TBD
	sprintf((char *)buf2, "accountRsMaxReq = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
	
	//apmib_get( MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&intVal);
	intVal = 0;//TBD
	sprintf((char *)buf2, "accountRsAWhile = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
	}
	}

	else {
#if 0//TBD
		apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt);
		if (encrypt == WDS_ENCRYPT_TKIP)		
			encrypt = ENCRYPT_WPA;
		else if (encrypt == WDS_ENCRYPT_AES)		
			encrypt = ENCRYPT_WPA2;		
		else
			encrypt = 0;
#else
		encrypt = 0;
#endif
		sprintf((char *)buf2, "encryption = %d\n", encrypt);
		WRITE_WPA_FILE(fh, buf2);
		WRITE_WPA_FILE(fh, (unsigned char *)"ssid = \"REALTEK\"\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"enable1x = 1\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"enableMacAuth = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"supportNonWpaClient = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"wepKey = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"wepGroupKey = \"\"\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"authentication = 2\n");
		
#if 0//TBD
		if (encrypt == ENCRYPT_WPA)
			intVal = WPA_CIPHER_TKIP;
		else
			intVal = WPA_CIPHER_AES;
#else
		intVal = 0;
#endif
		sprintf((char *)buf2, "unicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		sprintf((char *)buf2, "wpa2UnicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, (unsigned char *)"enablePreAuth = 0\n");

		//apmib_get( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal);
		intVal = 0;//TBD
		if (intVal==0)
			sprintf((char *)buf2, "usePassphrase = 1\n");
		else
			sprintf((char *)buf2, "usePassphrase = 0\n");
		WRITE_WPA_FILE(fh, buf2);

		//apmib_get( MIB_WLAN_WDS_PSK, (void *)buf1);
		strcpy(buf1,"");//TBD
		sprintf((char *)buf2, "psk = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, (unsigned char *)"groupRekeyTime = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsPort = 1812\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsIP = 192.168.1.1\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsAWhile = 10\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsEnabled = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsPort = 1813\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsIP = 192.168.1.1\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsUpdateEnabled = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsUpdateTime = 1000\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsAWhile = 1\n");
	}

	close(fh);
}

static int DoCmd(char *const argv[], char *file)
{    
	pid_t pid;
	int status;
	int fd;
	char _msg[30];
	switch (pid = fork()) {
			case -1:	/* error */
				perror("fork");
				return errno;
			case 0:	/* child */
				
				signal(SIGINT, SIG_IGN);
				if(file){
					if((fd = open(file, O_RDWR | O_CREAT))==-1){ /*open the file */
						sprintf(_msg, "open %s", file); 
  						perror(_msg);
  						exit(errno);
					}
					dup2(fd,STDOUT_FILENO); /*copy the file descriptor fd into standard output*/
					dup2(fd,STDERR_FILENO); /* same, for the standard error */
					close(fd); /* close the file descriptor as we don't need it more  */
				}else{
			#ifndef SYS_DEBUG		
					close(2); //do not output error messages
			#endif
				}
				setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
				execvp(argv[0], argv);
				perror(argv[0]);
				exit(errno);
			default:	/* parent */
			{
				
				waitpid(pid, &status, 0);
			#ifdef SYS_DEBUG	
				if(status != 0)
					printf("parent got child's status:%d, cmd=%s %s %s\n", status, argv[0], argv[1], argv[2]);
			#endif
				if (WIFEXITED(status)){
			#ifdef SYS_DEBUG	
					printf("parent will return :%d\n", WEXITSTATUS(status));
			#endif
					return WEXITSTATUS(status);
				}else{
					
					return status;
				}
			}
	}
}
void rtk_start_auth(void)
{
	rtklog("%s\n",__FUNCTION__);
	char wlan_role[30] = {0};
	int _use_rs=0;
	int daemon_created = 0, wlan_wpa_auth = 0;
	char enable_1x = 0;
	char tmpBuff[100] = {0};
	int check_cnt;
	if(sw_mode()!= SW_MODE_AP)
	{
		return;
	}
	if(nvram_match("wl0_auth_mode_x", "wpa2") || 
	   nvram_match("wl0_auth_mode_x", "wpawpa2")||
	   nvram_match("wl0_auth_mode_x", "wpa")||
	   nvram_match("wl0_auth_mode_x", "radius"))
	{
		enable_1x |= 1;
		sprintf(tmpBuff, "/var/wpa-wl0.conf");//wl0 encrytp conf file
		updateWpaConf("wl0",tmpBuff,0);
		doSystem("auth wl0 br0 auth %s",tmpBuff);
		check_cnt = 0;
		do{
			if(isFileExist("/var/run/auth-wl0.pid"))
			{//check pid file is exist or not
				break;
			}
			else
			{
				sleep(1);
			}
			check_cnt++;
		}while(check_cnt < MAX_CHECK_PID_NUM);
	}

	if(nvram_match("wl1_auth_mode_x", "wpa2") || 
	   nvram_match("wl1_auth_mode_x", "wpawpa2")||
	   nvram_match("wl1_auth_mode_x", "wpa")||
	   nvram_match("wl1_auth_mode_x", "radius"))
	{
		enable_1x |= 2;
		sprintf(tmpBuff, "/var/wpa-wl1.conf");//wl1 encrytp conf file
		updateWpaConf("wl1",tmpBuff,0);
		doSystem("auth wl1 br0 auth %s",tmpBuff);
		check_cnt = 0;
		do{
			if(isFileExist("/var/run/auth-wl1.pid"))
			{//check pid file is exist or not
				break;
			}
			else
			{
				sleep(1);
			}
			check_cnt++;
		}while(check_cnt < MAX_CHECK_PID_NUM);
	}

	if(enable_1x) {		
		printf("iwcontrol\n");
		strcpy(tmpBuff, "iwcontrol");
		if((enable_1x&0x1))
			strcat(tmpBuff, " wl0");
		if((enable_1x&0x2))
			strcat(tmpBuff, " wl1");
//		doSystem("sleep 1");
		doSystem(tmpBuff);
	}
	
}

static int start_wsc_deamon(char * wlan_interface, int mode, int WSC_UPNP_Enabled, char * bridge_iface) 
{
	rtklog("%s\n",__FUNCTION__);
    char *cmd_opt[16]={0};
    int cmd_cnt = 0;
    char tempbuf[40];
    char * arg_buff[40];
    char *token=NULL,*token1=NULL, *savestr1=NULL;
    int wps_debug=0, use_iwcontrol=1;
    char wsc_pin_local[16]={0},wsc_pin_peer[16]={0};
    FILE *fp;
    char wscFifoFile[40];
    char wscFifoFile1[40];
    char wscConfFile[40];
	char pbc_state_file[128];
    int wait_fifo=0;
	char cmd[256] = {0};
	int i;
    if(wlan_interface == NULL) {
        return;
    }
    
    memset(wscFifoFile, 0, sizeof(wscFifoFile));
    memset(wscFifoFile1, 0, sizeof(wscFifoFile1));
    memset(cmd_opt, 0x00, 16);
    cmd_cnt=0;
    wps_debug=0;
    use_iwcontrol=1;


    sprintf(arg_buff,"%s", wlan_interface);
    token = strtok_r(arg_buff," ", &savestr1);
    if(token)
        token1 = strtok_r(NULL," ", &savestr1);

    doSystem("ln -s /var/wps /etc/simplecfg");
    cmd_opt[cmd_cnt++] = "wscd";
    if(isFileExist("/var/wps/simplecfgservice.xml")==0){ //file does not exist
        if(isFileExist("/var/wps"))
			doSystem("rm -rf /var/wps");
		doSystem("mkdir /var/wps");
		doSystem("cp /etc/simplecfg*.xml /var/wps");
    }
    
    if(mode == 1) /*cleint*/
    {
        WSC_UPNP_Enabled=0;
        cmd_opt[cmd_cnt++] = "-mode";
        cmd_opt[cmd_cnt++] = "2";
    }else{
        cmd_opt[cmd_cnt++] = "-start";
    }
    
    if(WSC_UPNP_Enabled==1){
		sprintf(cmd,"route del -net 239.255.255.250 netmask 255.255.255.255 %s",bridge_iface);
		doSystem(cmd);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"route add -net 239.255.255.250 netmask 255.255.255.255 %s",bridge_iface);
		doSystem(cmd);
    }
    
    sprintf(wscConfFile,"/var/wsc-%s", token);
    if(token1) {
        strcat(wscConfFile, "-");            
        strcat(wscConfFile, token1);
    }
    strcat(wscConfFile, ".conf");
	rtklog("%s wscConfFile:%s\n",__FUNCTION__,wscConfFile);
	rtklog("%s wlan_interface:%s\n",__FUNCTION__,wlan_interface);
	updateWscConf("/etc/wscd.conf",wscConfFile,0,wlan_interface,mode);
    
    
    cmd_opt[cmd_cnt++] = "-c";
    cmd_opt[cmd_cnt++] = wscConfFile;
    if(token[2] == '0')
        cmd_opt[cmd_cnt++] = "-w";    
    else
        cmd_opt[cmd_cnt++] = "-w2";

    cmd_opt[cmd_cnt++] = token;

    if(token1) {
        if(token1[2] == '0')
            cmd_opt[cmd_cnt++] = "-w";       
        else
            cmd_opt[cmd_cnt++] = "-w2";
        cmd_opt[cmd_cnt++] = token1;    
    }
    
    if(wps_debug==1){
        /* when you would like to open debug, you should add define in wsc.h for debug mode enable*/
        cmd_opt[cmd_cnt++] = "-debug";
    }
    if(use_iwcontrol==1){
        if(token[2] == '0')
            cmd_opt[cmd_cnt++] = "-fi";
        else
            cmd_opt[cmd_cnt++] = "-fi2";
        snprintf(wscFifoFile,sizeof(wscFifoFile),"/var/wscd-%s.fifo",token);
        cmd_opt[cmd_cnt++] = wscFifoFile;

        if(token1) {
            if(token1[2] == '0')
                cmd_opt[cmd_cnt++] = "-fi";
            else
                cmd_opt[cmd_cnt++] = "-fi2";
            snprintf(wscFifoFile1,sizeof(wscFifoFile1),"/var/wscd-%s.fifo",token1);
            cmd_opt[cmd_cnt++] = wscFifoFile1;
        }
    }
    if(isFileExist("/var/wps_start_pbc")){
        cmd_opt[cmd_cnt++] = "-start_pbc";
        unlink("/var/wps_start_pbc");
    }
    if(isFileExist("/var/wps_start_pin")){
        cmd_opt[cmd_cnt++] = "-start";
        unlink("/var/wps_start_pin");
    }
    if(isFileExist("/var/wps_local_pin")){
        fp=fopen("/var/wps_local_pin", "r");
        if(fp != NULL){
            fscanf(fp, "%s", tempbuf);
            fclose(fp);
        }
        snprintf(wsc_pin_local, sizeof(wsc_pin_local), "%s", tempbuf);
        cmd_opt[cmd_cnt++] = "-local_pin";
        cmd_opt[cmd_cnt++] = wsc_pin_local;
        unlink("/var/wps_local_pin");
    }
    if(isFileExist("/var/wps_peer_pin")){
        fp=fopen("/var/wps_peer_pin", "r");
        if(fp != NULL){
            fscanf(fp, "%s", tempbuf);
            fclose(fp);
        }
        snprintf(wsc_pin_peer, sizeof(wsc_pin_peer), "%s", tempbuf);
        cmd_opt[cmd_cnt++] = "-peer_pin";
        cmd_opt[cmd_cnt++] = wsc_pin_peer;
        unlink("/var/wps_peer_pin");
    }
	if(mode == 1)
	{
		if(!strcmp(wlan_interface,"wl0-vxd") || !strcmp(wlan_interface, "wl0"))
		{
			cmd_opt[cmd_cnt++] = "-pbc_state_file";
			cmd_opt[cmd_cnt++] = "/var/pbc_state1";
		}
		else
		{
			cmd_opt[cmd_cnt++] = "-pbc_state_file";
			cmd_opt[cmd_cnt++] = "/var/pbc_state2";
		}
	}
	else
	{
		cmd_opt[cmd_cnt++] = "-pbc_state_file";
		cmd_opt[cmd_cnt++] = "/var/pbc_state1";
	}
    cmd_opt[cmd_cnt++] = "-daemon";
    
    cmd_opt[cmd_cnt++] = 0;
    //for (wps_debug=0; wps_debug<cmd_cnt;wps_debug++)
        //rtk_printf("cmd index=%d, opt=%s \n", wps_debug, cmd_opt[wps_debug]);

    DoCmd(cmd_opt, 0);


    if(use_iwcontrol) {
        wait_fifo=5;
        do{        
            if(isFileExist(wscFifoFile) && (wscFifoFile1[0] == 0 || isFileExist(wscFifoFile1)))
            {
                wait_fifo=0;
            }else{
                wait_fifo--;
                sleep(1);
            }
            
        }while(wait_fifo !=0);
    }
    
    return use_iwcontrol;
}

void rtk_stop_wsc(void)
{
	rtklog("%s\n",__FUNCTION__);
	char cmd_str [128] = {0};
	int pid = -1;
	if(isFileExist(IWCONTROL_PID_FILE)){
		pid=getPid_fromFile(IWCONTROL_PID_FILE);
		if(pid != -1){
			memset(cmd_str,0,sizeof(cmd_str));
			sprintf(cmd_str, "kill -9 %d", pid);
			doSystem(cmd_str);
		}
		unlink(IWCONTROL_PID_FILE);
	}

    /* release WPS */
    do{
        if(isFileExist("/var/run/wscd-wl0.pid"))
        {
            pid=getPid_fromFile("/var/run/wscd-wl0.pid");
            if(pid != -1){
				memset(cmd_str,0,sizeof(cmd_str));
				sprintf(cmd_str,"kill -9 %d",pid);
				doSystem(cmd_str);
            }
            else
                break;
            unlink("/var/run/wscd-wl0.pid");
            sleep(1);                   
        }
        else if(isFileExist("/var/run/wscd-wl1.pid"))
        {
            pid=getPid_fromFile("/var/run/wscd-wl1.pid");
            if(pid != -1){
				memset(cmd_str,0,sizeof(cmd_str));
				sprintf(cmd_str,"kill -9 %d",pid);
				doSystem(cmd_str);
            }
            else
                break;
            unlink("/var/run/wscd-wl1.pid");
            sleep(1);                   
        }
        else if(isFileExist("/var/run/wscd-wl0-wl1.pid"))
        {
            pid=getPid_fromFile("/var/run/wscd-wl0-wl1.pid");
            if(pid != -1){
				memset(cmd_str,0,sizeof(cmd_str));
				sprintf(cmd_str,"kill -9 %d",pid);
				doSystem(cmd_str);
            }
            else
                break;
            unlink("/var/run/wscd-wl0-wl1.pid");
            sleep(1);                   
        }
        else if(isFileExist("/var/run/wscd-wl0-vxd.pid"))
        {
            pid=getPid_fromFile("/var/run/wscd-wl0-vxd.pid");
            if(pid != -1){
				memset(cmd_str,0,sizeof(cmd_str));
				sprintf(cmd_str,"kill -9 %d",pid);
				doSystem(cmd_str);
            }
            else
                break;
            unlink("/var/run/wscd-wl0-vxd.pid");
            sleep(1);                   
        }
        else if(isFileExist("/var/run/wscd-wl1-vxd.pid"))
        {
            pid=getPid_fromFile("/var/run/wscd-wl1-vxd.pid");
            if(pid != -1){
				memset(cmd_str,0,sizeof(cmd_str));
				sprintf(cmd_str,"kill -9 %d",pid);
				doSystem(cmd_str);
            }
            else
                break;
            unlink("/var/run/wscd-wl1-vxd.pid");
            sleep(1);                   
        }       

        else
            break;
    }while(find_pid_by_name("wscd") != NULL);

    
	doSystem("rm -f /var/*.fifo");
}

void rtk_start_wsc(void)
{
	rtklog("%s \n",__FUNCTION__);
	int WSC_UPNP_Enabled=1;
	char ap_interface[20];
	char client_interface[20];
	char valid_wlan_interface[200] = {0};
	char bridge_iface[30]={0};
	int deamon_created = 0;
	char *ptr;
	int multi_band = 0;
	int wps_band = 0;
	int sw_mode = 0;	
	int wlc_express = nvram_get_int("wlc_express");

	rtk_stop_wsc();

	rtk_printf("Start Realtek WPS monitor...\n");
	memset(ap_interface,0,sizeof(ap_interface));
	memset(client_interface,0,sizeof(client_interface));
	//get root setting first//no this operate in script
	ptr = nvram_get("lan_ifname");
	strcpy(bridge_iface,ptr);
	rtklog("%s bridge_iface:%s\n",__FUNCTION__,bridge_iface);
	sw_mode = sw_mode();
	if(sw_mode == SW_MODE_REPEATER)/*repeater mode*/
	{
#ifdef RTCONFIG_CONCURRENTREPEATER
		if (wlc_express == 0) {
			deamon_created = start_wsc_deamon("wl0-vxd", 1, 0, NULL);
			deamon_created = start_wsc_deamon("wl1-vxd", 1, 0, NULL);
			if(deamon_created == 1)
			{
				doSystem("iwcontrol wl0-vxd wl1-vxd");
			}
		}
		else if (wlc_express == 1) {
			deamon_created = start_wsc_deamon("wl0", 1, 0, NULL);
			if(deamon_created == 1)
			{
				doSystem("iwcontrol wl0");
			}
		}
		else if (wlc_express == 2) {
			deamon_created = start_wsc_deamon("wl1", 1, 0, NULL);
			if(deamon_created == 1)
			{
				doSystem("iwcontrol wl1");
			}
		}
#else
		wps_band = nvram_get_int("wlc_band");
		sprintf(client_interface,"wl%d-vxd",wps_band);
		deamon_created = start_wsc_deamon(client_interface, 1, 0, NULL);
		if(deamon_created == 1)
		{
			doSystem("iwcontrol wl%d-vxd",wps_band);
		}
#endif
	}
	else if (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") == 1) { // media bridge mode
#ifdef RTCONFIG_CONCURRENTREPEATER
		deamon_created = start_wsc_deamon("wl0", 1, 0, NULL);
		deamon_created = start_wsc_deamon("wl1", 1, 0, NULL);
		if(deamon_created == 1)
		{
			doSystem("iwcontrol wl0 wl1");
		}
#else
		wps_band = nvram_get_int("wlc_band");
		sprintf(client_interface,"wl%d",wps_band);
		deamon_created = start_wsc_deamon(client_interface, 1, 0, NULL);
		if(deamon_created == 1)
		{
			doSystem("iwcontrol wl%d",wps_band);
		}
#endif
	}
	else
	{
		wps_band = nvram_get_int("wps_band_x");
		multi_band = nvram_get_int("wps_multiband");	
		if(multi_band == 1)
		{
			strcpy(ap_interface,"wl0 wl1");
		}
		else
		{
			sprintf(ap_interface,"wl%d",wps_band);
		}
		deamon_created= start_wsc_deamon(ap_interface, 0, 1, bridge_iface);
		if(deamon_created==1)
		{
			doSystem("iwcontrol %s",ap_interface);
		}
	}
	return 0;		
}

struct wifi_mib* set_wlan_rtkmib(int is5g, int vind)
{
	struct wifi_mib* pmib;
	char ifname[32];
	if(is5g){
		if(vind==-0xd){
			snprintf(ifname, sizeof(ifname), WIF_5G_IFNAME"-vxd");
		}else if(vind<0){
			snprintf(ifname, sizeof(ifname), WIF_5G_IFNAME);		
		}else{
			snprintf(ifname, sizeof(ifname), WIF_5G_IFNAME".%d", vind);			
		}
	}else{
		if(vind==-0xd){
			snprintf(ifname, sizeof(ifname), WIF_2G_IFNAME"-vxd");
		}else if(vind<0){
			snprintf(ifname, sizeof(ifname), WIF_2G_IFNAME);		
		}else{
			snprintf(ifname, sizeof(ifname), WIF_2G_IFNAME".%d", vind);
		}
	}

	pmib = drvmib_get_pmib(ifname);
	if(pmib==NULL){
		rtk_printf("%s[%d], get driver pmib failed!\n", __FUNCTION__, __LINE__);
	}
	return pmib;
}

int char2hex(char c)
{
	if(c>='0'&&c<='9') return c-'0';
	if(c>='a'&&c<='f') return 0xa+c-'a';
	if(c>='A'&&c<='F') return 0xA+c-'A';
	return 0;
}
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *str, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = str[idx];
		tmpBuf[1] = str[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;
		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

int set_wep_key(char* key, char* str, int keylen)
{
	int len = strlen(str);
	if( len!=10 && len!=26 ){
		memcpy(key, str, keylen);
	}else{
		string_to_hex(str, key, keylen*2);
	}
	return 0;
}
#if defined(CONFIG_RTL_DOT11K_SUPPORT) 
#define NEIGHBOR_REPORT_FILE "/proc/%s/rm_neighbor_report"
#endif

int gen_realtek_config(int band, int val)
{
	FILE *fp;
	char *str = NULL;
	char *str2 = NULL;
	const char* suf;
	int  i;
	int ssid_num = 1;
	char wmm_enable[8];
	char wmm_noack[8];
	char list[2048];
	int flag_8021x = 0;
	int wsc_configure = 0;
	int warning = 0;
	int ChannelNumMax_2G = 11;
	char tmp[128], prefix[] = "wlXXXXXXX_";
	char temp[128], prefix_mssid[] = "wlXXXXXXXXXX_mssid_";
	char tmpstr[128];
	int j;
	char *nv, *nvp, *b;
	int wl_key_type[MAX_NO_MSSID];
	int mcast_phy = 0, mcast_mcs = 0;
	int mac_filter[MAX_NO_MSSID];
#if defined(VHT_SUPPORT)
	int VHTBW_MAX = 0;
#endif
	struct wifi_mib* pmib;
	int sw_mode  = sw_mode();
	int wlc_band = nvram_get_int("wlc_band");
#ifdef RTCONFIG_CONCURRENTREPEATER
	int wlc_express = nvram_get_int("wlc_express");
	int wlc_psta = nvram_get_int("wlc_psta");
#endif
	int is_root = 0;

	printf("@@**********************************************@@\n");
	
	if(val==0){
		is_root = 1;
#ifdef RTCONFIG_CONCURRENTREPEATER
		if (sw_mode == SW_MODE_AP && wlc_psta == 1) // media bridge mode
			snprintf(prefix, sizeof(prefix), "wlc%d_", band);
		else if (sw_mode == SW_MODE_REPEATER && wlc_express == (band+1)) // express mode
			snprintf(prefix, sizeof(prefix), "wlc%d_", band);
		else
#endif
			snprintf(prefix, sizeof(prefix), "wl%d_", band);

		pmib = set_wlan_rtkmib(band, -1);
#ifdef RTCONFIG_WIRELESSREPEATER		
	}else if(val==0xd && sw_mode == SW_MODE_REPEATER 
#ifdef RTCONFIG_CONCURRENTREPEATER
		&& wlc_express == 0
#else
		&& wlc_band == band
#endif
	){
#ifdef RTCONFIG_CONCURRENTREPEATER
		snprintf(prefix, sizeof(prefix), "wlc%d_",band);
#else
		snprintf(prefix, sizeof(prefix), "wlc_");
#endif
		pmib = set_wlan_rtkmib(band, -0xd);	
#endif
	}else{
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", band, val);
		pmib = set_wlan_rtkmib(band, val);
	}

#define NVRAM_GET(var, mib_suffix)	var = nvram_safe_get(strcat_r(prefix, suf=mib_suffix, tmp));if(var && var[0]!='\0')
	NVRAM_GET(str, "country_code"){
		strncpy(pmib->dot11dCountry.dot11CountryString, str, 3);
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		strncpy(pmib->dot11dCountry.dot11CountryString, "US", 3);	
	}

#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_CONCURRENTREPEATER	
	if (sw_mode == SW_MODE_REPEATER && wlc_express == 0 && val==0xd)
#else
	if (sw_mode == SW_MODE_REPEATER && wlc_band == band && val==0xd)
#endif
	{
		NVRAM_GET(str, "ssid"){
			memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
			memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
			memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID,str,strlen(str));
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen(str);			
			memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan,str,strlen(str));
			pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = strlen(str);	
			//interface client
			pmib->dot11OperationEntry.opmode = 8;
		}else{
			rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
			memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
			memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);		
			snprintf(pmib->dot11StationConfigEntry.dot11DesiredSSID, 32, "ASUS%d", val);
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen(pmib->dot11StationConfigEntry.dot11DesiredSSID);
			snprintf(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 32, "%s", str);
			pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = strlen(str);
		}
	}
	else
#endif	/* RTCONFIG_WIRELESSREPEATER */
	{
		NVRAM_GET(str, "ssid"){
			memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
			memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);			
			memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID,str,strlen(str));
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen(str);
			memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan,str,strlen(str));
			pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = strlen(str);
		}else{
			rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
			memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
			memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);		
			snprintf(pmib->dot11StationConfigEntry.dot11DesiredSSID, 32, "ASUS%d", val);
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen(pmib->dot11StationConfigEntry.dot11DesiredSSID);
			snprintf(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 32, "%s", str);
			pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = strlen(str);
		}
	}

	//Network Mode
	NVRAM_GET(str, "nmode_x"){
		int mode = atoi(str);
		if(band){ // 5G
			if(mode==0){ //Auto
				pmib->dot11StationConfigEntry.legacySTADeny = 0;
				pmib->dot11BssType.net_work_type = 76;
			}else if(mode==1){ // N Only
				pmib->dot11StationConfigEntry.legacySTADeny = 4;
				pmib->dot11BssType.net_work_type = 12;			
			}else if(mode==8){ // AC
				pmib->dot11StationConfigEntry.legacySTADeny = 12;
				pmib->dot11BssType.net_work_type = 76;
			}else if(mode==2){ //A
				pmib->dot11StationConfigEntry.legacySTADeny = 0;
				pmib->dot11BssType.net_work_type = 4;
			}else{
#if defined(VHT_SUPPORT)
//A+AN+AC
				pmib->dot11StationConfigEntry.legacySTADeny = 0;
				pmib->dot11BssType.net_work_type = 76;
#else
//A+AN
				pmib->dot11StationConfigEntry.legacySTADeny = 0;
				pmib->dot11BssType.net_work_type = 12;
#endif
			}

			pmib->dot11StationConfigEntry.dot11SupportedRates = 4080;
			pmib->dot11StationConfigEntry.dot11BasicRates = 4080;
			
		}else{  // 2G
			if(mode==0){ //BGN
				pmib->dot11StationConfigEntry.legacySTADeny = 0;
				pmib->dot11BssType.net_work_type = 11;
#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RPAC68U
                                pmib->dot11BssType.net_work_type |= 64;
#endif
#endif
			}else if(mode==2){ //BG
				pmib->dot11StationConfigEntry.legacySTADeny = 0;
				pmib->dot11BssType.net_work_type = 3;			
			}else if(mode==1){ //N
				pmib->dot11StationConfigEntry.legacySTADeny = 3;
				pmib->dot11BssType.net_work_type = 11;
#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RPAC68U
                                pmib->dot11BssType.net_work_type |= 64;
#endif
#endif
			}else if(mode==4){ //G
				pmib->dot11StationConfigEntry.legacySTADeny = 1;
				pmib->dot11BssType.net_work_type = 3;		
			}else if(mode==1){ //B
				pmib->dot11StationConfigEntry.legacySTADeny = 0;
				pmib->dot11BssType.net_work_type = 1;
			}else{ //BGN
				pmib->dot11StationConfigEntry.legacySTADeny = 0;
				pmib->dot11BssType.net_work_type = 11;
#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RPAC68U
                                pmib->dot11BssType.net_work_type |= 64;
#endif
#endif
			}
			
			pmib->dot11StationConfigEntry.dot11SupportedRates = 4095;
			pmib->dot11StationConfigEntry.dot11BasicRates = 15;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		if(band == 0){//2.4G
			pmib->dot11StationConfigEntry.legacySTADeny = 0;
			pmib->dot11BssType.net_work_type = 11;
#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RPAC68U
                        pmib->dot11BssType.net_work_type |= 64;
#endif
#endif
			
			pmib->dot11StationConfigEntry.dot11SupportedRates = 4095;
			pmib->dot11StationConfigEntry.dot11BasicRates = 15;
		}
		else if(band == 1){//5G
			pmib->dot11StationConfigEntry.legacySTADeny = 0;
			pmib->dot11BssType.net_work_type = 76;
			
			pmib->dot11StationConfigEntry.dot11SupportedRates = 4080;
			pmib->dot11StationConfigEntry.dot11BasicRates = 4080;
		}
	}
	//Channel
	NVRAM_GET(str, "channel"){
		pmib->dot11RFEntry.dot11channel = atoi(str);
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11RFEntry.dot11channel = 0;
	}

	//BasicRate
	if(!band){
		NVRAM_GET(str, "rateset"){
			if (!strcmp(str, "default"))	// 1, 2, 5.5, 11
				pmib->dot11StationConfigEntry.dot11BasicRates =	0xf;
			else if (!strcmp(str, "all"))	// 1, 2, 5.5, 6, 11, 12, 24
				pmib->dot11StationConfigEntry.dot11BasicRates =	0xff0;	
			else if (!strcmp(str, "12"))	// 1, 2
				pmib->dot11StationConfigEntry.dot11BasicRates =	0x3;
			else
				pmib->dot11StationConfigEntry.dot11BasicRates =	0xf;	
		}else{
			rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
			pmib->dot11StationConfigEntry.dot11BasicRates =	0xf;
		}
	}

	//BeaconPeriod
	NVRAM_GET(str, "bcn"){
		int intv = atoi(str);
		if (intv > 1000 || intv < 20){
			nvram_set(strcat_r(prefix, "bcn", tmp), "100");
			pmib->dot11StationConfigEntry.dot11BeaconPeriod = 100;
		}else{
			pmib->dot11StationConfigEntry.dot11BeaconPeriod = intv;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11StationConfigEntry.dot11BeaconPeriod = 100;
	}
	
	//DTIM Period
	NVRAM_GET(str, "dtim"){
		int intv = atoi(str);
		pmib->dot11StationConfigEntry.dot11DTIMPeriod = intv;
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11StationConfigEntry.dot11DTIMPeriod = 1;
	}

	//radio
	NVRAM_GET(str, "radio"){
		if (strcmp(str, "0")==0){
			pmib->miscEntry.func_off = 1; //root if only
		}else{
			pmib->miscEntry.func_off = 0;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->miscEntry.func_off = 0;
	}
	
	//DisableOLBC
	pmib->dot11StationConfigEntry.olbcDetectDisabled = 0;

	//BGProtection
	NVRAM_GET(str, "gmode_protection"){
		if (!strcmp(str, "auto"))
			pmib->dot11StationConfigEntry.protectionDisabled = 0;
		else
			pmib->dot11StationConfigEntry.protectionDisabled = 2;
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11StationConfigEntry.protectionDisabled = 1;
	}

	//TxPreamble
	NVRAM_GET(str, "plcphdr"){
		if (str && strcmp(str, "long") == 0)
			pmib->dot11RFEntry.shortpreamble = 0;
		else if (str && strcmp(str, "short") == 0)
			pmib->dot11RFEntry.shortpreamble = 1;
		else
			pmib->dot11RFEntry.shortpreamble = 0;
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11RFEntry.shortpreamble = 0;
	}

	//RTSThreshold	Default=2347
	NVRAM_GET(str, "rts"){
		pmib->dot11OperationEntry.dot11RTSThreshold = atoi(str);
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11OperationEntry.dot11RTSThreshold = 2347;	
	}

	//FragThreshold  Default=2346
	NVRAM_GET(str, "frag"){
		pmib->dot11OperationEntry.dot11FragmentationThreshold = atoi(str);
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11OperationEntry.dot11FragmentationThreshold = 2346;	
	}

	//WmmCapable
/*	memset(tmpstr, 0x0, sizeof(tmpstr));
	memset(wmm_enable, 0x0, sizeof(wmm_enable));

	str = nvram_safe_get(strcat_r(prefix, "nmode_x", tmp));
	if (str && atoi(str) == 1)	// always enable WMM in N only mode
		sprintf(wmm_enable+strlen(wmm_enable), "%d", 1);
	else
		sprintf(wmm_enable+strlen(wmm_enable), "%d", strcmp(nvram_safe_get(strcat_r(prefix, "wme", tmp)), "off") ? 1 : 0);

	for (i = 0; i < ssid_num; i++)
	{
		if (i)
			sprintf(tmpstr, "%s;", tmpstr);

		sprintf(tmpstr, "%s%s", tmpstr, wmm_enable);
	}
	fprintf(fp, "WmmCapable=%s\n", tmpstr);

	fprintf(fp, "APAifsn=3;7;1;1\n");
	fprintf(fp, "APCwmin=4;4;3;2\n");
	fprintf(fp, "APCwmax=6;10;4;3\n");
	fprintf(fp, "APTxop=0;0;94;47\n");
	fprintf(fp, "APACM=0;0;0;0\n");
	fprintf(fp, "BSSAifsn=3;7;2;2\n");
	fprintf(fp, "BSSCwmin=4;4;3;2\n");
	fprintf(fp, "BSSCwmax=10;10;4;3\n");
	fprintf(fp, "BSSTxop=0;0;94;47\n");
	fprintf(fp, "BSSACM=0;0;0;0\n");
*/	
	NVRAM_GET(str, "wme"){
		if(strcmp(str, "off")==0){
			pmib->dot11QosEntry.dot11QosEnable = 0;
		}else if(strcmp(str, "on")==0){
			pmib->dot11QosEntry.dot11QosEnable = 1;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11QosEntry.dot11QosEnable = 1;
	}
	NVRAM_GET(str, "wme_apsd"){
		if(strcmp(str, "off")==0){
			pmib->dot11QosEntry.dot11QosAPSD = 0;
		}else if(strcmp(str, "on")==0){
			pmib->dot11QosEntry.dot11QosAPSD = 1;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11QosEntry.dot11QosAPSD = 1;
	}
	NVRAM_GET(str, "wme_no_ack"){
		if(strcmp(str, "off")==0){
			pmib->dot11nConfigEntry.dot11nTxNoAck = 0;
		}else if(strcmp(str, "on")==0){
			pmib->dot11nConfigEntry.dot11nTxNoAck = 1;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11nConfigEntry.dot11nTxNoAck = 0;
	}

	NVRAM_GET(str, "nmode_x"){
		if(atoi(str)==1){
			pmib->dot11QosEntry.dot11QosEnable = 1; //N mode fore enable QOS
		}
	}

	//PktAggregate
	NVRAM_GET(str, "PktAggregate"){
		int value = atoi(str);
		if(value==0){
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}else if(value ==1){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}
	}

		//ampdu
	NVRAM_GET(str, "ampdu"){
		if(strcmp(str, "off")==0){
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
		}else if(strcmp(str, "on")==0){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
		}else if(strcmp(str, "auto")==0){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
		}else 
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11nConfigEntry.dot11nAMPDU = 1;
	}

	//amsdu
	NVRAM_GET(str, "amsdu"){
		if(strcmp(str, "off")==0){
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}else if(strcmp(str, "on")==0){
			pmib->dot11nConfigEntry.dot11nAMSDU = 2;
		}else if(strcmp(str, "auto")==0){
			pmib->dot11nConfigEntry.dot11nAMSDU = 2;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11nConfigEntry.dot11nAMSDU = 2;
	}

	//HideSSID
	NVRAM_GET(str, "bss_enabled"){
		pmib->miscEntry.vap_enable = atoi(str);//root ap only
	}else{
		rtk_printf("NVRAM: %s not set!!\n", prefix);	
		pmib->miscEntry.vap_enable = 0;
	}
	NVRAM_GET(str, "closed"){
		pmib->dot11OperationEntry.hiddenAP = atoi(str);
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot11OperationEntry.hiddenAP = 0;	
	}

	//ShortGI
	NVRAM_GET(str, "HT_GI"){
		int shortgi = atoi(str);
		pmib->dot11nConfigEntry.dot11nShortGIfor20M = shortgi;
		pmib->dot11nConfigEntry.dot11nShortGIfor40M = shortgi;
#ifndef RPAC68U
		pmib->dot11nConfigEntry.dot11nShortGIfor80M = shortgi;
#endif
	}else{
		pmib->dot11nConfigEntry.dot11nShortGIfor20M = 1;
		pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;
#ifndef RPAC68U
		pmib->dot11nConfigEntry.dot11nShortGIfor80M = 1;
#endif		
	}

if(is_root)
{
#ifdef CONFIG_RTL_DOT11K_SUPPORT
    pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated = 0;
	char iface[16] = {0};
    /*if Fast BSS Transition is enabled,  activate 11k and set 11K neighbor report*/
	sprintf(iface,"wl%d",band);
	doSystem("echo delall > "NEIGHBOR_REPORT_FILE, iface); 
    pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated = 1; 
#endif
	NVRAM_GET(str, "bw"){
		int value = atoi(str);
		if(value==0){ //20M
			pmib->dot11nConfigEntry.dot11nUse40M = 0;
			pmib->dot11nConfigEntry.dot11nCoexist = 0;
		}else if(value==1){//20-40M
			if(band == 0)
				pmib->dot11nConfigEntry.dot11nUse40M = 1;
			else if(band == 1)
				pmib->dot11nConfigEntry.dot11nUse40M = 2;
			pmib->dot11nConfigEntry.dot11nCoexist = 1;			
		}else if(value==2){//40M
			pmib->dot11nConfigEntry.dot11nUse40M = 1;
			pmib->dot11nConfigEntry.dot11nCoexist = 0;		
		}else if(value=3){//80M
			pmib->dot11nConfigEntry.dot11nUse40M = 2;
			pmib->dot11nConfigEntry.dot11nCoexist = 0;			
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		if(band){
			pmib->dot11nConfigEntry.dot11nUse40M = 1;		
			pmib->dot11nConfigEntry.dot11nCoexist = 1;
		}else{
			pmib->dot11nConfigEntry.dot11nUse40M = 0;
			pmib->dot11nConfigEntry.dot11nCoexist = 0;
		}
	}

	if(pmib->dot11nConfigEntry.dot11nUse40M != 0)
	{
	NVRAM_GET(str, "nctrlsb"){
		int lower = strcmp(str, "lower") ? 0 : 1;
		NVRAM_GET(str, "channel"){
			int channel =atoi(str);
		switch(band)
		{
			case 0:
			{
				if(channel >=1 && channel <=4)
				{
					if(!lower)
					{
						rtk_printf("channel %d 2ndchoffset cannot set to upper!!!!!\n",channel);
						nvram_set("wl0_nctrlsb","lower");
					}
					pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;
				}
				else if(channel >=10 && channel <=14)
				{
					if(lower)
					{
						rtk_printf("channel %d 2ndchoffset cannot set to lower!!!!\n",channel);
						nvram_set("wl0_nctrlsb","upper");
					}
					pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
				}
				else
				{
					if(lower)
					{
						pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;
					}
					else
					{
					pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
					}
				}
				break;
			}
			case 1:
			{
				if(channel==36 || channel==44 || channel==52 || channel==60
				|| channel==100 || channel==108 || channel==116 || channel==124
				|| channel==132 || channel==140 || channel==149 || channel==157
				|| channel==165 || channel==173)
					pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;
				else
					pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
				break;
			}
			default:
			{
				rtk_printf("Invalid band!!!!\n");
				break;
			}
		}
	}
	}
	else{
			rtk_printf("NVRAM:%s%s not set!\n",prefix,suf);
	}
	}
	else
	{
		pmib->dot11nConfigEntry.dot11n2ndChOffset = 0;
	}
}
	NVRAM_GET(str, "crypto"){
			if(strcmp(str, "aes")==0){
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11WPACipher = 8;
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = 8;
		}else if(strcmp(str, "tkip")==0){
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11WPACipher = 2;
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = 2;
		}else if(strcmp(str, "tkip+aes")==0){
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11WPACipher = 10;
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = 10; 		
		}else if(strcmp(str, "aes+tkip")==0){
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11WPACipher = 10;
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = 10; 		
		}else if(strcmp(str, "none")==0){
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			pmib->dot1180211AuthEntry.dot11WPACipher = 0;
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0; 			
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
		pmib->dot1180211AuthEntry.dot11WPACipher = 0;
		pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0; 	
	}
#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_CONCURRENTREPEATER	
		if (sw_mode == SW_MODE_REPEATER && wlc_express == 0 && val==0xd)
#else
		if (sw_mode == SW_MODE_REPEATER && wlc_band == band && val==0xd)
#endif
		str = nvram_safe_get(strcat_r(prefix, "auth_mode", tmp));
#ifdef RTCONFIG_CONCURRENTREPEATER
		else if (sw_mode == SW_MODE_AP && wlc_psta == 1)
			str = nvram_safe_get(strcat_r(prefix, "auth_mode", tmp));
		else if (sw_mode == SW_MODE_REPEATER && wlc_express == (band+1))
			str = nvram_safe_get(strcat_r(prefix, "auth_mode", tmp));
#endif
	else
		str = nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp));
	if(str && str[0]!='\0')
#else
	NVRAM_GET(str, "auth_mode_x")
#endif
	{
		if(strcmp(str, "open")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
			pmib->dot1180211AuthEntry.dot11WPACipher = 0;
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;
#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_CONCURRENTREPEATER	
			if (sw_mode == SW_MODE_REPEATER && wlc_express == 0 && val==0xd)
#else
			if (sw_mode == SW_MODE_REPEATER && wlc_band == band && val==0xd)
#endif
				str2 = nvram_safe_get(strcat_r(prefix, "wep", tmp));
#ifdef RTCONFIG_CONCURRENTREPEATER
			else if (sw_mode == SW_MODE_AP && wlc_psta == 1)
				str2 = nvram_safe_get(strcat_r(prefix, "wep", tmp));
			else if (sw_mode == SW_MODE_REPEATER && wlc_express == (band+1))
				str2 = nvram_safe_get(strcat_r(prefix, "wep", tmp));
#endif
			else
				str2 = nvram_safe_get(strcat_r(prefix, "wep_x", tmp));
			if(str2 && str2[0]!='\0')
#else
			NVRAM_GET(str2, "wep_x")
#endif
			{
				int value = atoi(str2);
				if(value==1){
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1; //40
				}else if(value==2){
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5; //104
				}
			}			
		}else if(strcmp(str, "shared")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 1;
			pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
			pmib->dot1180211AuthEntry.dot11WPACipher = 0;
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;	
#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_CONCURRENTREPEATER	
			if (sw_mode == SW_MODE_REPEATER && wlc_express == 0 && val==0xd)
#else
			if (sw_mode == SW_MODE_REPEATER && wlc_band == band && val==0xd)
#endif
				str2 = nvram_safe_get(strcat_r(prefix, "wep", tmp));
#ifdef RTCONFIG_CONCURRENTREPEATER
			else if (sw_mode == SW_MODE_AP && wlc_psta == 1)
				str2 = nvram_safe_get(strcat_r(prefix, "wep", tmp));
			else if (sw_mode == SW_MODE_REPEATER && wlc_express == (band+1))
				str2 = nvram_safe_get(strcat_r(prefix, "wep", tmp));
#endif
			else
				str2 = nvram_safe_get(strcat_r(prefix, "wep_x", tmp));
			if(str2 && str2[0]!='\0')
#else
			NVRAM_GET(str2, "wep_x")
#endif
			{
				int value = atoi(str2);
				if(value==1){
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1; //40
				}else if(value==2){
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5; //104
				}
			}else{
				rtk_printf("NVRAM: %s%s not set!! using 2\n", prefix, suf);			
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5;
			}
		}else if(strcmp(str, "psk")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;			
			pmib->dot1180211AuthEntry.dot11EnablePSK = 1;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
		}else if(strcmp(str, "psk2")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11EnablePSK = 2;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
		}else if(strcmp(str, "pskpsk2")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11EnablePSK = 3;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
		}else if(strcmp(str, "wpa")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;
		}else if(strcmp(str, "wpa2")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;
		}else if(strcmp(str, "wpawpa2")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;			
		}else if(strcmp(str, "radius")==0){
			pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1;
			pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;			
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!!\n", prefix, suf);
		pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
		pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
		pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
		pmib->dot1180211AuthEntry.dot11WPACipher = 0;
		pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;
	}

	if(strcmp(str, "psk2")==0 || strcmp(str, "pskpsk2")==0 || strcmp(str, "psk")==0){
		NVRAM_GET(str, "wpa_psk"){
			strncpy(pmib->dot1180211AuthEntry.dot11PassPhrase, str, sizeof(pmib->dot1180211AuthEntry.dot11PassPhrase));
		}else{
			rtk_printf("NVRAM: %s%s not set!! using 11111111\n", prefix, suf);	
			strncpy(pmib->dot1180211AuthEntry.dot11PassPhrase, "11111111", sizeof(pmib->dot1180211AuthEntry.dot11PassPhrase));		
		}
	}

	NVRAM_GET(str, "key"){
		int value = atoi(str);
		pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = value - 1;
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 1\n", prefix, suf);
		pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = 0;
	}
#ifdef RTCONFIG_WIRELESSREPEATER
	if((val==0xd && sw_mode == SW_MODE_REPEATER && wlc_band == band) || (sw_mode == SW_MODE_AP && wlc_psta == 1) || (sw_mode == SW_MODE_REPEATER && wlc_express == (band+1))){
		NVRAM_GET(str, "wep_key"){
			pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = 0;
			if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==1){
				memset(&(pmib->dot11DefaultKeysTable.keytype[0]),0,sizeof(pmib->dot11DefaultKeysTable.keytype[0]));
				set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[0]), str, 5);
			}else if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==5){
				set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[0]), str, 13);
			}
		}else{
			rtk_printf("NVRAM: %s%s not set!! using 1111111111111\n", prefix, suf);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[0]), "1111111111111", 13);
		}
	}
	else
#endif
	{
	NVRAM_GET(str, "key1"){
		if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==1){
			set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[0]), str, 5);
 		}else if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==5){
			set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[0]), str, 13);
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 1111111111111\n", prefix, suf);
		memcpy(&(pmib->dot11DefaultKeysTable.keytype[0]), "1111111111111", 13);
	}
	NVRAM_GET(str, "key2"){
		if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==1){
			set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[1]), str, 5);
 		}else if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==5){
			set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[1]), str, 13);
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 1111111111111\n", prefix, suf);
		memcpy(&(pmib->dot11DefaultKeysTable.keytype[1]), "1111111111111", 13);
	}
	NVRAM_GET(str, "key3"){
		if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==1){
			set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[2]), str, 5);
 		}else if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==5){
			set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[2]), str, 13);
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 1111111111111\n", prefix, suf);
		memcpy(&(pmib->dot11DefaultKeysTable.keytype[2]), "1111111111111", 13);
	}
	NVRAM_GET(str, "key4"){
		if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==1){
			set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[3]), str, 5);
 		}else if(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==5){
			set_wep_key(&(pmib->dot11DefaultKeysTable.keytype[3]), str, 13);
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 1111111111111\n", prefix, suf);
		memcpy(&(pmib->dot11DefaultKeysTable.keytype[3]), "1111111111111", 13);
	}
	}

	if(strcmp(str, "psk2")==0 || strcmp(str, "pskpsk2")==0){
		NVRAM_GET(str, "wpa_gtk_rekey"){
			pmib->dot1180211AuthEntry.dot11GKRekeyTime = atoi(str);
		}else{
			rtk_printf("NVRAM: %s%s not set!! using 0\n", prefix, suf);
			pmib->dot1180211AuthEntry.dot11GKRekeyTime = 0;
		}
	} else
		pmib->dot1180211AuthEntry.dot11GKRekeyTime = 0;

	NVRAM_GET(str, "mode_x"){
		int value = atoi(str);
		if(value==0){ //AP
			pmib->dot11OperationEntry.opmode = 0x10;
		}else if(value==1){ //WDS
			pmib->dot11OperationEntry.opmode = 0x1000;		
		}else if(value==2){ //AP+WDS
			pmib->dot11OperationEntry.opmode = 0x1010;
		}
#ifdef RTCONFIG_CONCURRENTREPEATER	
	if ((sw_mode == SW_MODE_REPEATER && wlc_express == 0 && val==0xd)
		|| (sw_mode == SW_MODE_REPEATER && wlc_express == (band+1))
		|| (sw_mode == SW_MODE_AP && wlc_psta == 1))
#else
	if (sw_mode == SW_MODE_REPEATER && wlc_band == band && val==0xd)
#endif
		{
			pmib->dot11OperationEntry.opmode = 0x08;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 0\n", prefix, suf);
		pmib->dot11OperationEntry.opmode = 0x10;
#ifdef RTCONFIG_CONCURRENTREPEATER	
		if ((sw_mode == SW_MODE_REPEATER && wlc_express == 0 && val==0xd)
				|| (sw_mode == SW_MODE_REPEATER && wlc_express == (band+1))
				|| (sw_mode == SW_MODE_AP && wlc_psta == 1))
#else
		if (sw_mode == SW_MODE_REPEATER && wlc_band == band && val==0xd)
#endif
		{
			//cprintf("*****%s %d*****\n",__FUNCTION__,__LINE__);
			pmib->dot11OperationEntry.opmode = 0x08;
		}
	}
	NVRAM_GET(str, "ap_isolate"){
		pmib->dot11OperationEntry.block_relay = atoi(str);
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 0\n", prefix, suf);
		pmib->dot11OperationEntry.block_relay = 0;
	}

	/*	ASUS UI Mrate definition
	 *	HTMIX 6.5/15	14
	 *	HTMIX 13/30	15
	 * 	HTMIX 19.5/45	16
	 *  	HTMIX 13/30	17
	 *   	HTMIX 26/60	18
	 *    	HTMIX 130/144	13
	 *     	OFDM 6		4
	 *     	OFDM 9		5
	 *      OFDM 12		7
	 *	OFDM 18		8
	 *	OFDM 24		9
	 *	OFDM 36		10
	 *	OFDM 48		11
	 *	OFDM 54		12
	 *	CCK 1		1
	 *	CCK 2		2
	 *	CCK 5.5		3
	 *	CCK 11		6
	 */

	NVRAM_GET(str, "mrate_x"){
		switch (atoi(str)) {
			case 0: //Auto
				pmib->dot11StationConfigEntry.lowestMlcstRate = 0;
				break;
			case 1: //CCK 1
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1;
				break;
			case 2: //CCK 2
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 1;
				break;
			case 3: //CCK 5.5
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 2;
				break;
			case 6: //CCK 11
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 3;
				break;
			case 4: //OFDM 6
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 4;
				break;
			case 5: //OFDM 9
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 5;
				break;
			case 7: //OFDM 12
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 6;
				break;
			case 8: //OFDM 18
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 7;
				break;
			case 9: //OFDM 24
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 8;
				break;
			case 10: //OFDM 36
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 9;
				break;
			case 11: //OFDM 48
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 10;
				break;
			case 12: //OFDM 54
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 11;
				break;
			case 14: // MCS0
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 12;
				break;
			case 15: // MCS1
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 13;
				break;
			case 16: // MCS2
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 14;
				break;
			case 17: // MCS8
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 20;
				break;
			case 18: // MCS9
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 21;
				break;
			case 13: // MCS15
				pmib->dot11StationConfigEntry.lowestMlcstRate = 1 << 27;
				break;
			default:
				pmib->dot11StationConfigEntry.lowestMlcstRate = 0;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 0\n", prefix, suf);
		pmib->dot11StationConfigEntry.lowestMlcstRate = 0;
	}

	// ACL
	NVRAM_GET(str, "macmode"){
		if(strcmp(str, "accept")==0||strcmp(str, "allow")==0){
			pmib->dot11StationConfigEntry.dot11AclMode = 1;
		}else if(strcmp(str, "deny")==0){
			pmib->dot11StationConfigEntry.dot11AclMode = 2;		
		}else{
			pmib->dot11StationConfigEntry.dot11AclMode = 0;
		}
	}else{
		rtk_printf("NVRAM: %s%s not set!! using disabled\n", prefix, suf);
		pmib->dot11StationConfigEntry.dot11AclMode = 0;	
	}

	NVRAM_GET(str, "maclist_x"){
		char* mac;
		char* maclist = strdup(str);
		pmib->dot11StationConfigEntry.dot11AclNum = 0;
		while ((mac = strsep(&maclist, "<")) != NULL) {
			if (strlen(mac)==0) continue;
			if (pmib->dot11StationConfigEntry.dot11AclNum >= NUM_ACL) break;
			pmib->dot11StationConfigEntry.dot11AclAddr[pmib->dot11StationConfigEntry.dot11AclNum][0] = (char2hex(mac[0])<<4)+char2hex(mac[1]);
			pmib->dot11StationConfigEntry.dot11AclAddr[pmib->dot11StationConfigEntry.dot11AclNum][1] = (char2hex(mac[3])<<4)+char2hex(mac[4]);
			pmib->dot11StationConfigEntry.dot11AclAddr[pmib->dot11StationConfigEntry.dot11AclNum][2] = (char2hex(mac[6])<<4)+char2hex(mac[7]);
			pmib->dot11StationConfigEntry.dot11AclAddr[pmib->dot11StationConfigEntry.dot11AclNum][3] = (char2hex(mac[9])<<4)+char2hex(mac[10]);
			pmib->dot11StationConfigEntry.dot11AclAddr[pmib->dot11StationConfigEntry.dot11AclNum][4] = (char2hex(mac[12])<<4)+char2hex(mac[13]);
			pmib->dot11StationConfigEntry.dot11AclAddr[pmib->dot11StationConfigEntry.dot11AclNum][5] = (char2hex(mac[15])<<4)+char2hex(mac[16]);
			pmib->dot11StationConfigEntry.dot11AclNum++;
		}
		free(maclist);
	}else{
		pmib->dot11StationConfigEntry.dot11AclNum = 0;	
	}

	// STA MONITOR
	NVRAM_GET(str, "monitor_sta") {
		if (strcmp(str, "enable") == 0) {
			pmib->dot11StationConfigEntry.monitor_sta_enabled = 1;
		} else {
			pmib->dot11StationConfigEntry.monitor_sta_enabled = 0;
		}
	} else{
		rtk_printf("NVRAM: %s%s not set!! using disabled\n", prefix, suf);
		pmib->dot11StationConfigEntry.monitor_sta_enabled = 0;
	}

	NVRAM_GET(str, "monitor_sta_table") {
		char* mac;
		char* maclist = strdup(str);
		pmib->dot11StationConfigEntry.monitor_sta_num = 0;
		while ((mac = strsep(&maclist, "<")) != NULL) {
			if (strlen(mac) == 0) continue;
			if (pmib->dot11StationConfigEntry.monitor_sta_num >= NUM_MONITOR) break;
			pmib->dot11StationConfigEntry.monitor_sta_addr[pmib->dot11StationConfigEntry.monitor_sta_num][0] = (char2hex(mac[0]) << 4) + char2hex(mac[1]);
			pmib->dot11StationConfigEntry.monitor_sta_addr[pmib->dot11StationConfigEntry.monitor_sta_num][1] = (char2hex(mac[3]) << 4) + char2hex(mac[4]);
			pmib->dot11StationConfigEntry.monitor_sta_addr[pmib->dot11StationConfigEntry.monitor_sta_num][2] = (char2hex(mac[6]) << 4) + char2hex(mac[7]);
			pmib->dot11StationConfigEntry.monitor_sta_addr[pmib->dot11StationConfigEntry.monitor_sta_num][3] = (char2hex(mac[9]) << 4) + char2hex(mac[10]);
			pmib->dot11StationConfigEntry.monitor_sta_addr[pmib->dot11StationConfigEntry.monitor_sta_num][4] = (char2hex(mac[12]) << 4) + char2hex(mac[13]);
			pmib->dot11StationConfigEntry.monitor_sta_addr[pmib->dot11StationConfigEntry.monitor_sta_num][5] = (char2hex(mac[15]) << 4) + char2hex(mac[16]);
			pmib->dot11StationConfigEntry.monitor_sta_num++;
		}
		free(maclist);
	} else{
		pmib->dot11StationConfigEntry.monitor_sta_num = 0;
	}

	// WDS
	NVRAM_GET(str, "wdsapply_x"){
		pmib->dot11WdsInfo.wdsEnabled = atoi(str);
		pmib->dot11WdsInfo.wdsPrivacy = 0; //disable encrypt 
		NVRAM_GET(str, "auth_mode_x"){
			if(!strcmp(str, "open")){ //wep open
				NVRAM_GET(str, "wep_x"){
					int value = atoi(str);
					if(value==1){
						pmib->dot11WdsInfo.wdsPrivacy = 1;
						NVRAM_GET(str, "wep_key"){
							string_to_hex((char *)str, &(pmib->dot11WdsInfo.wdsWepKey[0]), 10);
						}
					}										
				}
			}
			else if(!strcmp(str, "shared")){ //wep shared
				NVRAM_GET(str, "wep_x"){
					int value = atoi(str);
					if(value==2){
						pmib->dot11WdsInfo.wdsPrivacy = 5; 
						NVRAM_GET(str, "wep_key"){
							strcpy(&(pmib->dot11WdsInfo.wdsWepKey[0]), str);
							string_to_hex((char *)str, &(pmib->dot11WdsInfo.wdsWepKey[0]), 10);
						}
					}											
				}
			}
			else if((!strcmp(str, "psk")) ){ //wpa
				pmib->dot11WdsInfo.wdsPrivacy = 2;
				NVRAM_GET(str, "wpa_psk"){
					strcpy((char *)pmib->dot11WdsInfo.wdsPskPassPhrase, (char *)str);
				}
			}
			else if((!strcmp(str, "psk2")) ){ //wpa2 
				pmib->dot11WdsInfo.wdsPrivacy = 4;
				NVRAM_GET(str, "wpa_psk"){
					strcpy((char *)pmib->dot11WdsInfo.wdsPskPassPhrase, (char *)str);
				}
			}
			else if(!strcmp(str, "pskpsk2")){ //wpa mixed
				pmib->dot11WdsInfo.wdsPrivacy = 6;
				NVRAM_GET(str, "wpa_psk"){
					strcpy((char *)pmib->dot11WdsInfo.wdsPskPassPhrase, (char *)str);
				}
			}
		}	
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 0\n", prefix, suf);
		pmib->dot11WdsInfo.wdsEnabled = 0;
	}
	
	NVRAM_GET(str, "wdslist"){
		char* mac;
		char* wdslist = strdup(str);		
		pmib->dot11WdsInfo.wdsNum = 0;
		while ((mac = strsep(&wdslist, "<")) != NULL) {
			if (strlen(mac)==0) continue;
			if (pmib->dot11WdsInfo.wdsNum >= NUM_WDS) break;
			pmib->dot11WdsInfo.entry[pmib->dot11WdsInfo.wdsNum].txRate = 0;
			pmib->dot11WdsInfo.entry[pmib->dot11WdsInfo.wdsNum].macAddr[0] = (char2hex(mac[0])<<4)+char2hex(mac[1]);
			pmib->dot11WdsInfo.entry[pmib->dot11WdsInfo.wdsNum].macAddr[1] = (char2hex(mac[3])<<4)+char2hex(mac[4]);
			pmib->dot11WdsInfo.entry[pmib->dot11WdsInfo.wdsNum].macAddr[2] = (char2hex(mac[6])<<4)+char2hex(mac[7]);
			pmib->dot11WdsInfo.entry[pmib->dot11WdsInfo.wdsNum].macAddr[3] = (char2hex(mac[9])<<4)+char2hex(mac[10]);
			pmib->dot11WdsInfo.entry[pmib->dot11WdsInfo.wdsNum].macAddr[4] = (char2hex(mac[12])<<4)+char2hex(mac[13]);
			pmib->dot11WdsInfo.entry[pmib->dot11WdsInfo.wdsNum].macAddr[5] = (char2hex(mac[15])<<4)+char2hex(mac[16]);
			pmib->dot11WdsInfo.wdsNum++;
		}		
		free(wdslist);
	}else{
		pmib->dot11WdsInfo.wdsNum = 0;
	}
	
	NVRAM_GET(str, "hwaddr"){
		char* mac=str;
		pmib->dot11OperationEntry.hwaddr[0] = (char2hex(mac[0])<<4)+char2hex(mac[1]);
		pmib->dot11OperationEntry.hwaddr[1] = (char2hex(mac[3])<<4)+char2hex(mac[4]);
		pmib->dot11OperationEntry.hwaddr[2] = (char2hex(mac[6])<<4)+char2hex(mac[7]);
		pmib->dot11OperationEntry.hwaddr[3] = (char2hex(mac[9])<<4)+char2hex(mac[10]);
		pmib->dot11OperationEntry.hwaddr[4] = (char2hex(mac[12])<<4)+char2hex(mac[13]);
		pmib->dot11OperationEntry.hwaddr[5] = (char2hex(mac[15])<<4)+char2hex(mac[16]);		
	}else{
		rtk_printf("NVRAM: %s%s not set!! using 00:e0:4c:81:96:c1\n", prefix, suf);
		memcpy(pmib->dot11OperationEntry.hwaddr, "\x0\xe0\x4c\x81\x96\xc1", 6);
	}

	/* TxBF */
	NVRAM_GET(str, "txbf"){

		if (atoi(str) == 1) {	/* TxBF is enabled */
			//doSystem("iwpriv wl%d set_mib txbf=1", band);
			pmib->dot11RFEntry.txbf = 1;
			pmib->dot11RFEntry.txbfer = 1;
			pmib->dot11RFEntry.txbfee = 1;
			doSystem("iwpriv wl%d set_mib txbf_pwrlmt=1", band);
		}
		else if (atoi(str) == 2) {	/* TxBF is auto */
			//doSystem("iwpriv wl%d set_mib txbf=1", band);
			pmib->dot11RFEntry.txbf = 2;
			pmib->dot11RFEntry.txbfer = 1;
			pmib->dot11RFEntry.txbfee = 1;
			doSystem("iwpriv wl%d set_mib txbf_pwrlmt=2", band);
		}
		else {	/* TxBF is disabled */
			//doSystem("iwpriv wl%d set_mib txbf=0", band);
#ifdef RPAC68U
			pmib->dot11RFEntry.txbf = 1;
			pmib->dot11RFEntry.txbfer = 0;
			pmib->dot11RFEntry.txbfee = 1;
#else
			pmib->dot11RFEntry.txbf = 0;
			pmib->dot11RFEntry.txbfer = 0;
			pmib->dot11RFEntry.txbfee = 0;
#endif
			doSystem("iwpriv wl%d set_mib txbf_pwrlmt=0", band);
		}
	}else{
		printf("NVRAM: %s%s not set!! using 0\n", prefix, suf);
#ifdef RPAC68U
		pmib->dot11RFEntry.txbf = 1;
		pmib->dot11RFEntry.txbfer = 0;
		pmib->dot11RFEntry.txbfee = 1;
#else
		pmib->dot11RFEntry.txbf = 0;
		pmib->dot11RFEntry.txbfer = 0;
		pmib->dot11RFEntry.txbfee = 0;
#endif
	}

#if defined(RTCONFIG_MUMIMO_2G) || defined(RTCONFIG_MUMIMO_5G)	
	/* MU-MIMO */
	NVRAM_GET(str, "mumimo") {
		if (atoi(str) == 1) /* MU-MIMO is enabled */
			pmib->dot11RFEntry.txbf_mu = 1;
		else
			pmib->dot11RFEntry.txbf_mu = 0;
	} else {
		printf("NVRAM: %s%s not set!! using 0\n", prefix, suf);
		pmib->dot11RFEntry.txbf_mu = 0;
	}
#endif

	/* igmp snooping */
	NVRAM_GET(str, "igs"){
		if (atoi(str) == 1)	/* igmp snooping is enabled */
			doSystem("iwpriv wl%d set_mib mc2u_disable=0", band);
		else	/* igmp snooping is disabled */
			doSystem("iwpriv wl%d set_mib mc2u_disable=1", band);
	} else {
		if (!val)	/* root AP */
			doSystem("iwpriv wl%d set_mib mc2u_disable=1", band);
	}

	/* WiFi proxy */
	NVRAM_GET(str, "wifipxy") {
		if (atoi(str) == 1) /* WiFi proxy is enabled */
			pmib->ethBrExtInfo.macclone_enable = 1;
		else
			pmib->ethBrExtInfo.macclone_enable = 0;
	} else {
		printf("NVRAM: %s%s not set!! using 0\n", prefix, suf);
		pmib->ethBrExtInfo.macclone_enable = 0;

	}

	//set realtek default wlan mib value that nvram setting don't defined.
	{
		pmib->dot11OperationEntry.dot11ShortRetryLimit = 0;
		pmib->dot11OperationEntry.expiretime = 30000;
		pmib->dot11nConfigEntry.dot11nSTBC = 1;
		pmib->dot11nConfigEntry.dot11nLDPC = 1;
		pmib->dot11OperationEntry.tdls_prohibited = 0;
		pmib->dot11OperationEntry.tdls_cs_prohibited = 0;
		pmib->miscEntry.ack_timeout = 0;
		pmib->dot11OperationEntry.iapp_enable = 1;
		//pmib->dot11RFEntry.txbf = 1;
		pmib->dot11OperationEntry.wifi_specific = 2;
		//pmib->ethBrExtInfo.macclone_enable = 0;
		pmib->wscEntry.wsc_enable = 0;
		pmib->dot11StationConfigEntry.autoRate = 1;
		//pmib->dot11RFEntry.phyBandSelect = 0;
		//pmib->dot11RFEntry.macPhyMode = 0;
		pmib->dot11OperationEntry.guest_access = 0;
		pmib->dot118021xAuthEntry.acct_enabled = 0;	
		pmib->gbwcEntry.GBWCMode = 0;
		pmib->gbwcEntry.GBWCThrd_tx = 0;
		pmib->gbwcEntry.GBWCThrd_rx = 0;
		
	}
		
	return 0;
}



void gen_rtk_config(const char* wif)
{
	char word[256], *next;
	char* vifnames;
	int i;
	char prefix[]="wlXXXXXX_", tmp[32];
	drvmib_init_rtkapi();
	rtk_printf("[%s] wif:%s\n",__FUNCTION__,wif);
	if (!strcmp(wif, nvram_safe_get("wl0_ifname"))) // 2.4G
	{
		gen_realtek_config(0, 0);
		vifnames = nvram_safe_get("wl0_vifnames");
		if( vifnames!=NULL && vifnames[0]!='\0' ){
			foreach (word, vifnames, next) {
				snprintf(prefix, sizeof(prefix), "%s_", word);
				if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
					gen_realtek_config(0, atoi(&word[4]));
			}
		}
	}
	else if (!strcmp(wif, nvram_safe_get("wl1_ifname"))) // 5G
	{
		gen_realtek_config(1, 0);
		vifnames = nvram_safe_get("wl1_vifnames");		
		if( vifnames!=NULL && vifnames[0]!='\0' ){
			foreach (word, vifnames, next) {
				snprintf(prefix, sizeof(prefix), "%s_", word);
				if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
					gen_realtek_config(1, atoi(&word[4]));
			}
		}		
	}
#ifdef RTCONFIG_WIRELESSREPEATER	
	if (!strcmp(wif, nvram_safe_get("wl0_vxdifnames"))) //wl0 repeater
	{
		gen_realtek_config(0, 0xd);
	}
	else if (!strcmp(wif, nvram_safe_get("wl1_vxdifnames"))) //wl1 repeater
	{
		gen_realtek_config(1, 0xd);
	}
#endif

	drvmib_apply_rtkapi();
	drvmib_release_rtkapi();
}

/* Check wlc ssid nvram
 *
 * return:
 * 0 ssid nvram is be set a value.
 * 1 ssid nvram is null or space.
 */
int rtk_chk_wlc_ssid(const char *wif)
{
	int wlc_express = nvram_get_int("wlc_express");
	char tmp[100] = {0}, prefix[]="wlXXXXX_";
	const char* suf;
	char *str = NULL;

	if (repeater_mode() && wlc_express == 0) {
		if (!strcmp(wif, VXD_2G))
			snprintf(prefix, sizeof(prefix), "wlc0_");
		else if (!strcmp(wif, VXD_5G))
			snprintf(prefix, sizeof(prefix), "wlc1_");
		else /* ignore non wlc interface */
			return 1;
	}
	else if (repeater_mode() && wlc_express != 1) {
		snprintf(tmp, sizeof(tmp), "wl%d", wlc_express - 1);
		if (!strcmp(wif, tmp))
			snprintf(prefix, sizeof(prefix), "wlc%d_", wlc_express - 1);
		else /* ignore non wlc interface */
			return 1;
	}
	else if (mediabridge_mode()) {
		if (!strcmp(wif, WIF_2G))
			snprintf(prefix, sizeof(prefix), "wlc0_");
		else if (!strcmp(wif, WIF_5G))
			snprintf(prefix, sizeof(prefix), "wlc1_");
		else /* ignore non wlc interface */
			return 1;
	}
	else /* ignore non wlc interface */
		return 1;

	NVRAM_GET(str, "ssid") {
		return 1;
	}

	return 0;
}

static int radio_rtk(const char *wif, int band, int ctrl)
{
	char tmp[100], prefix[]="wlXXXXX_";

	snprintf(prefix, sizeof(prefix), "wl%d", band);

	if (wif == NULL) return -1;

	if (!ctrl)
	{
		doSystem("iwpriv %s set func_off=1", wif);
		doSystem("iwpriv %s del_sta all", wif); // kick all STA.
	}
	else
	{
		if (nvram_match(strcat_r(prefix, "radio", tmp), "1")) {
			doSystem("iwpriv %s set func_off=1", wif);
			doSystem("iwpriv %s del_sta all", wif); // kick all STA.
		}
	}

	return 0;
}

static void set_wlpara_rtk(const char* wif, int band)
{	/* set driver Tx power*/
	char tmp[100], prefix[]="wlXXXXXXX_";
	int offset = HW_SETTING_OFFSET;
	char buffer[sizeof(HW_WLAN_SETTING_T)];
	unsigned char boardVer;

	offset += sizeof(PARAM_HEADER_T);
	offset += (int)(&((struct hw_setting *)0)->wlan);
	if (!strcmp(wif,WIF_2G_IFNAME))
		offset += sizeof(struct hw_wlan_setting);
	else if (strcmp(wif,WIF_5G_IFNAME))
		return;

	snprintf(prefix, sizeof(prefix), "wl%d_", band);

	if (nvram_match(strcat_r(prefix, "radio", tmp), "0"))
		radio_rtk(wif, band, 0);
	else
	{
		int txpower = nvram_get_int(strcat_r(prefix, "txpower", tmp));
		if ((txpower >= 0) && (txpower <= 100))
		{
			TRACE_PT("moify power here\n");
			//doSystem("iwpriv %s set TxPower=%d",wif, txpower);
			rtk_flash_read(buffer,offset,sizeof(HW_WLAN_SETTING_T));
			set_tx_calibration(buffer,wif,txpower);
		}
		offset = HW_SETTING_OFFSET;
		offset += sizeof(PARAM_HEADER_T);
		offset += (int)(&((struct hw_setting *)0)->boardVer);
		rtk_flash_read(&boardVer,offset,1);
		TRACE_PT("Check Board Ver\n");
		set_mimo_tr_mode(boardVer,wif);
	}

	set_txpwr_lmt_index(wif);
}

int get_mac_addr_from_flash(const char *wif,void* macaddr)
{	/*get wlan mac addr from flash*/
	unsigned char buffer[6];
	int offset = HW_SETTING_OFFSET;
	memset(buffer, 0, sizeof(buffer));
	memset(macaddr, 0, sizeof(macaddr));
	offset += sizeof(PARAM_HEADER_T);
	
	if (!strncmp(wif, WIF_2G_IFNAME, 3) || !strncmp(wif, WIF_5G_IFNAME, 3))
	{
		offset += (int)(&((struct hw_setting *)0)->wlan);
		if (!strncmp(wif, WIF_2G_IFNAME, 3)) // 2.4G band
			offset += sizeof(struct hw_wlan_setting);

		if (strstr(wif, ".1")) // 1st Guest Network.
			offset += (int)(&((struct hw_wlan_setting *)0)->macAddr1);
		else if (strstr(wif, ".2")) // 2nd Guest Network.
			offset += (int)(&((struct hw_wlan_setting *)0)->macAddr2);
		else if (strstr(wif, ".3")) // 3rd Guest Network.
			offset += (int)(&((struct hw_wlan_setting *)0)->macAddr3);
		else
			offset += (int)(&((struct hw_wlan_setting *)0)->macAddr);
	}
	else if((!strcmp(wif,"br0")) || (!strcmp(wif,"eth0")) || (!strcmp(wif,"eth2")) || (!strcmp(wif,"eth3")) || (!strcmp(wif,"eth4"))) 
	{	
		offset += (int)(&((struct hw_setting *)0)->nic0Addr);
	}
	else if(!strcmp(wif,"eth1"))
	{
		offset += (int)(&((struct hw_setting *)0)->nic1Addr);
	}
	else
		return 0;
	rtk_flash_read(buffer,offset,6);
	ether_etoa(buffer, macaddr);
	rtklog("%s:%d wif=%s offset=%d macaddr=%s\n",__FUNCTION__,__LINE__,wif,offset,macaddr);
	return 1;
}

int set_rtk_hw_default()
{
	HW_SETTING_T hwmib={0};
	int idx=0;
	int offset = HW_SETTING_OFFSET;
	offset+=sizeof(PARAM_HEADER_T);
	
	cprintf("%s:%d set_rtk_hw_default\n",__FUNCTION__,__LINE__);
	hwmib.boardVer = 1;
	memcpy(hwmib.nic0Addr, "\x0\xe0\x4c\x81\x96\xc1", 6);
	memcpy(hwmib.nic1Addr, "\x0\xe0\x4c\x81\x96", 6);
	hwmib.nic1Addr[5] = 0xc1 + 8;
	for (idx=0; idx<NUM_WLAN_INTERFACE; idx++)
	{
		memcpy(hwmib.wlan[idx].macAddr, "\x0\xe0\x4c\x81\x96", 5);		
		hwmib.wlan[idx].macAddr[5] = 0xc1 + (idx*16);			
		
		memcpy(hwmib.wlan[idx].macAddr1, "\x0\xe0\x4c\x81\x96", 5);		
		hwmib.wlan[idx].macAddr1[5] = hwmib.wlan[idx].macAddr[5] + 1;	
		
		memcpy(hwmib.wlan[idx].macAddr2, "\x0\xe0\x4c\x81\x96", 5);		
		hwmib.wlan[idx].macAddr2[5] = hwmib.wlan[idx].macAddr[5] + 2;	
		
		memcpy(hwmib.wlan[idx].macAddr3, "\x0\xe0\x4c\x81\x96", 5);		
		hwmib.wlan[idx].macAddr3[5] = hwmib.wlan[idx].macAddr[5] + 3;	
		
		memcpy(hwmib.wlan[idx].macAddr4, "\x0\xe0\x4c\x81\x96", 5);		
		hwmib.wlan[idx].macAddr4[5] = hwmib.wlan[idx].macAddr[5] + 4;			

		memcpy(hwmib.wlan[idx].macAddr5, "\x0\xe0\x4c\x81\x96", 5);		
		hwmib.wlan[idx].macAddr5[5] = hwmib.wlan[idx].macAddr[5] + 5;	
		
		memcpy(hwmib.wlan[idx].macAddr6, "\x0\xe0\x4c\x81\x96", 5);		
		hwmib.wlan[idx].macAddr6[5] = hwmib.wlan[idx].macAddr[5] + 6;	
		
		memcpy(hwmib.wlan[idx].macAddr7, "\x0\xe0\x4c\x81\x96", 5);		
		hwmib.wlan[idx].macAddr7[5] = hwmib.wlan[idx].macAddr[5] + 7;		
		hwmib.wlan[idx].regDomain = FCC;
		hwmib.wlan[idx].rfType = 10;
	}
	
	rtklog("%s:%d offset=0x%x size=%d\n",__FUNCTION__,__LINE__,offset,sizeof(HW_SETTING_T));
	rtk_flash_write(&hwmib,offset,sizeof(HW_SETTING_T));
	sleep(1);
	rtk_flash_write(HW_SETTING_HEADER_TAG,HW_SETTING_OFFSET,strlen(HW_SETTING_HEADER_TAG));
	return 0;
}
int check_rtk_hw_invaild()
{
	char buff[8]={0};
	int offset = HW_SETTING_OFFSET;
	offset+= (int)(&((PARAM_HEADER_Tp)0)->signature);
	rtk_flash_read(buff,offset,SIGNATURE_LEN);
	rtklog("%s:%d offset=0x%x buff=%s\n",__FUNCTION__,__LINE__,offset,buff);

	if(strncmp(buff,HW_SETTING_HEADER_TAG,strlen(HW_SETTING_HEADER_TAG))!=0)
		return 1;
	return 0;
}
int set_nvram_pincode_from_flash()
{
	unsigned char PIN[9];
	memset(PIN, 0, sizeof(PIN));
	int offset = HW_SETTING_OFFSET;
	offset += sizeof(PARAM_HEADER_T);
	offset += (int)(&((struct hw_setting *)0)->wlan);
	offset += (int)(&((struct hw_wlan_setting *)0)->wscPin);
	rtk_flash_read(PIN,offset,8);
	if(PIN[0])
		nvram_set("secret_code",PIN);
	else
		nvram_set("secret_code","12345670");
}
int set_nvram_mac_addr_from_flash()
{	//set nvram mac addr from flash
	char macaddr[32];
	char word[PATH_MAX], *next_word, tmp[100];
	get_mac_addr_from_flash("br0",macaddr);
	nvram_set("lan_hwaddr", macaddr);
	nvram_set("eth2_hwaddr", macaddr);
	nvram_set("eth3_hwaddr", macaddr);
	nvram_set("eth4_hwaddr", macaddr);
	nvram_set("et0macaddr", macaddr);
	get_mac_addr_from_flash("eth1",macaddr);
	nvram_set("wan0_hwaddr", macaddr);
	nvram_set("et1macaddr", macaddr);
	get_mac_addr_from_flash("wl0",macaddr);
	nvram_set("wl0_hwaddr", macaddr);
	get_mac_addr_from_flash("wl1",macaddr);
	nvram_set("wl1_hwaddr", macaddr);

	foreach(word, nvram_safe_get("wl0_vifnames"), next_word) {
		get_mac_addr_from_flash(word, macaddr);
		nvram_set(strcat_r(word, "_hwaddr", tmp), macaddr);
	}
	foreach(word, nvram_safe_get("wl1_vifnames"), next_word) {
		get_mac_addr_from_flash(word, macaddr);
		nvram_set(strcat_r(word, "_hwaddr", tmp), macaddr);
	}
#ifdef RTCONFIG_WIRELESSREPEATER
	get_mac_addr_from_flash("wl0-vxd",macaddr);
	nvram_set("wl0-vxd_hwaddr", macaddr);
	get_mac_addr_from_flash("wl1-vxd",macaddr);
	nvram_set("wl1-vxd_hwaddr", macaddr);
#endif
}

int wlconf_rtk(const char* wif)
{	
	int unit = 0;
	char word[256], *next;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char macaddr[32];
	char interf[16];
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

		if (access_point_mode())
			nvram_set(strcat_r(prefix,"mode",tmp),"ap");
#ifdef RTCONFIG_WIRELESSREPEATER
		else if(sw_mode() == SW_MODE_REPEATER)
		{
			if (nvram_get_int("wlc_express") == 0) // Repeater mode
				nvram_set(strcat_r(prefix,"mode",tmp),"wet");
			else if (nvram_get_int("wlc_express") == (unit+1)) // Express way.
				nvram_set(strcat_r(prefix,"mode",tmp),"wet");
			else
				nvram_set(strcat_r(prefix,"mode",tmp),"ap");
		}
#if defined(RTCONFIG_PROXYSTA)
		else if (mediabridge_mode())
			nvram_set(strcat_r(prefix,"mode",tmp),"psta");
#endif
#endif
		if (!strcmp(word, wif))
		{			
			if (get_mac_addr_from_flash(wif,macaddr))
			{
				nvram_set(strcat_r(prefix, "hwaddr", tmp), macaddr);
				nvram_commit();
				doSystem("ifconfig %s hw ether %s",wif,macaddr);
			}

			if (!strcmp(word, "wl1"))
				set_wlpara_rtk(wif, 1);
			else if (!strcmp(word, "wl0"))
				set_wlpara_rtk(wif, 0);		
		}
		unit++;
	}
#ifdef RTCONFIG_WIRELESSREPEATER
	if ((!strcmp(wif, "wl0-vxd")) || (!strcmp(wif, "wl1-vxd")) ){
		if (!strcmp(wif, "wl0-vxd"))
			strcpy(interf,"wl0");
		else
			strcpy(interf,"wl1");
		snprintf(prefix, sizeof(prefix), "%s_", wif);
		if(get_mac_addr_from_flash(interf,macaddr)){
			nvram_set(strcat_r(prefix, "hwaddr", tmp), macaddr);
			nvram_commit();
			doSystem("ifconfig %s hw ether %s",wif,macaddr);
		}
	}
#endif
	return 0;
}

static bool g_swap = FALSE;
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define dtohchanspec(i) (g_swap?dtoh16(i):i)


static int wpa_key_mgmt_to_bitfield(const unsigned char *s)
{
	if (memcmp(s, WPA_AUTH_KEY_MGMT_UNSPEC_802_1X, WPA_SELECTOR_LEN) == 0)
		return WPA_KEY_MGMT_IEEE8021X_;
	if (memcmp(s, WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X, WPA_SELECTOR_LEN) ==
	    0)
		return WPA_KEY_MGMT_PSK_;
	if (memcmp(s, WPA_AUTH_KEY_MGMT_NONE, WPA_SELECTOR_LEN) == 0)
		return WPA_KEY_MGMT_WPA_NONE_;
	return 0;
}

static int rsn_key_mgmt_to_bitfield(const unsigned char *s)
{
	if (memcmp(s, RSN_AUTH_KEY_MGMT_UNSPEC_802_1X, RSN_SELECTOR_LEN) == 0)
		return WPA_KEY_MGMT_IEEE8021X2_;
	if (memcmp(s, RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X, RSN_SELECTOR_LEN) ==
	    0)
		return WPA_KEY_MGMT_PSK2_;
	return 0;
}

static int wpa_selector_to_bitfield(const unsigned char *s)
{
	if (memcmp(s, WPA_CIPHER_SUITE_NONE, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_NONE_;
	if (memcmp(s, WPA_CIPHER_SUITE_WEP40, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_WEP40_;
	if (memcmp(s, WPA_CIPHER_SUITE_TKIP, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_TKIP_;
	if (memcmp(s, WPA_CIPHER_SUITE_CCMP, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_CCMP_;
	if (memcmp(s, WPA_CIPHER_SUITE_WEP104, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_WEP104_;
	return 0;
}

static int rsn_selector_to_bitfield(const unsigned char *s)
{
	if (memcmp(s, RSN_CIPHER_SUITE_NONE, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_NONE_;
	if (memcmp(s, RSN_CIPHER_SUITE_WEP40, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_WEP40_;
	if (memcmp(s, RSN_CIPHER_SUITE_TKIP, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_TKIP_;
	if (memcmp(s, RSN_CIPHER_SUITE_CCMP, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_CCMP_;
	if (memcmp(s, RSN_CIPHER_SUITE_WEP104, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_WEP104_;
	return 0;
}


static const char * wpa_key_mgmt_txt(int key_mgmt, int proto)
{
	switch (key_mgmt) {
	case WPA_KEY_MGMT_IEEE8021X_:
/*
		return proto == WPA_PROTO_RSN_ ?
			"WPA2/IEEE 802.1X/EAP" : "WPA/IEEE 802.1X/EAP";
*/
		return "WPA-Enterprise";
	case WPA_KEY_MGMT_IEEE8021X2_:
		return "WPA2-Enterprise";
	case WPA_KEY_MGMT_PSK_:
/*
		return proto == WPA_PROTO_RSN_ ?
			"WPA2-PSK" : "WPA-PSK";
*/
		return "WPA-Personal";
	case WPA_KEY_MGMT_PSK2_:
		return "WPA2-Personal";
	case WPA_KEY_MGMT_NONE_:
		return "NONE";
	case WPA_KEY_MGMT_IEEE8021X_NO_WPA_:
//		return "IEEE 802.1X (no WPA)";
		return "IEEE 802.1X";
	default:
		return "Unknown";
	}
}

static const char * wpa_cipher_txt(int cipher)
{
	switch (cipher) {
	case WPA_CIPHER_NONE_:
		return "NONE";
	case WPA_CIPHER_WEP40_:
		return "WEP-40";
	case WPA_CIPHER_WEP104_:
		return "WEP-104";
	case WPA_CIPHER_TKIP_:
		return "TKIP";
	case WPA_CIPHER_CCMP_:
//		return "CCMP";
		return "AES";
	case (WPA_CIPHER_TKIP_|WPA_CIPHER_CCMP_):
		return "TKIP+AES";
	default:
		return "Unknown";
	}
}

static int wpa_parse_wpa_ie_wpa(const unsigned char *wpa_ie, size_t wpa_ie_len,
				struct wpa_ie_data *data)
{
	const struct wpa_ie_hdr *hdr;
	const unsigned char *pos;
	int left;
	int i, count;

	data->proto = WPA_PROTO_WPA_;
	data->pairwise_cipher = WPA_CIPHER_TKIP_;
	data->group_cipher = WPA_CIPHER_TKIP_;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X_;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;

	if (wpa_ie_len == 0) {
		/* No WPA IE - fail silently */
		return -1;
	}

	if (wpa_ie_len < sizeof(struct wpa_ie_hdr)) {
//		fprintf(stderr, "ie len too short %lu", (unsigned long) wpa_ie_len);
		return -1;
	}

	hdr = (const struct wpa_ie_hdr *) wpa_ie;

	if (hdr->elem_id != DOT11_MNG_WPA_ID ||
	    hdr->len != wpa_ie_len - 2 ||
	    memcmp(&hdr->oui, WPA_OUI_TYPE_ARR, WPA_SELECTOR_LEN) != 0 ||
	    WPA_GET_LE16(hdr->version) != WPA_VERSION_) {
//		fprintf(stderr, "malformed ie or unknown version");
		return -1;
	}

	pos = (const unsigned char *) (hdr + 1);
	left = wpa_ie_len - sizeof(*hdr);

	if (left >= WPA_SELECTOR_LEN) {
		data->group_cipher = wpa_selector_to_bitfield(pos);
		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;
	} else if (left > 0) {
//		fprintf(stderr, "ie length mismatch, %u too much", left);
		return -1;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
//			fprintf(stderr, "ie count botch (pairwise), "
//				   "count %u left %u", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= wpa_selector_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
//		fprintf(stderr, "ie too short (for key mgmt)");
		return -1;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
//			fprintf(stderr, "ie count botch (key mgmt), "
//				   "count %u left %u", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= wpa_key_mgmt_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
//		fprintf(stderr, "ie too short (for capabilities)");
		return -1;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left > 0) {
//		fprintf(stderr, "ie has %u trailing bytes", left);
		return -1;
	}

	return 0;
}

static int wpa_parse_wpa_ie_rsn(const unsigned char *rsn_ie, size_t rsn_ie_len,
				struct wpa_ie_data *data)
{
	const struct rsn_ie_hdr *hdr;
	const unsigned char *pos;
	int left;
	int i, count;

	data->proto = WPA_PROTO_RSN_;
	data->pairwise_cipher = WPA_CIPHER_CCMP_;
	data->group_cipher = WPA_CIPHER_CCMP_;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X2_;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;

	if (rsn_ie_len == 0) {
		/* No RSN IE - fail silently */
		return -1;
	}

	if (rsn_ie_len < sizeof(struct rsn_ie_hdr)) {
//		fprintf(stderr, "ie len too short %lu", (unsigned long) rsn_ie_len);
		return -1;
	}

	hdr = (const struct rsn_ie_hdr *) rsn_ie;

	if (hdr->elem_id != DOT11_MNG_RSN_ID ||
	    hdr->len != rsn_ie_len - 2 ||
	    WPA_GET_LE16(hdr->version) != RSN_VERSION_) {
//		fprintf(stderr, "malformed ie or unknown version");
		return -1;
	}

	pos = (const unsigned char *) (hdr + 1);
	left = rsn_ie_len - sizeof(*hdr);

	if (left >= RSN_SELECTOR_LEN) {
		data->group_cipher = rsn_selector_to_bitfield(pos);
		pos += RSN_SELECTOR_LEN;
		left -= RSN_SELECTOR_LEN;
	} else if (left > 0) {
//		fprintf(stderr, "ie length mismatch, %u too much", left);
		return -1;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
//			fprintf(stderr, "ie count botch (pairwise), "
//				   "count %u left %u", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= rsn_selector_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
//		fprintf(stderr, "ie too short (for key mgmt)");
		return -1;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
//			fprintf(stderr, "ie count botch (key mgmt), "
//				   "count %u left %u", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= rsn_key_mgmt_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
//		fprintf(stderr, "ie too short (for capabilities)");
		return -1;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left >= 2) {
		data->num_pmkid = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (left < data->num_pmkid * PMKID_LEN) {
//			fprintf(stderr, "PMKID underflow "
//				   "(num_pmkid=%d left=%d)", data->num_pmkid, left);
			data->num_pmkid = 0;
		} else {
			data->pmkid = pos;
			pos += data->num_pmkid * PMKID_LEN;
			left -= data->num_pmkid * PMKID_LEN;
		}
	}

	if (left > 0) {
//		fprintf(stderr, "ie has %u trailing bytes - ignored", left);
	}

	return 0;
}

int wpa_parse_wpa_ie(const unsigned char *wpa_ie, size_t wpa_ie_len,
		     struct wpa_ie_data *data)
{
	if (wpa_ie_len >= 1 && wpa_ie[0] == DOT11_MNG_RSN_ID)
		return wpa_parse_wpa_ie_rsn(wpa_ie, wpa_ie_len, data);
	else
		return wpa_parse_wpa_ie_wpa(wpa_ie, wpa_ie_len, data);
}

#if defined(RTCONFIG_WLCSCAN_RSSI)
/**
 * Get specific SSID signal quility via sitesurvey.
 * @param  band 0: 2.4G. 1: 5G.
 * @param  ssid Specific SSID for RSSI
 * @return      Not for anything.
 */
int wlcscan_ssid_rssi(int band, char *ssid)
{

	if (band < 0 || band > 1 || ssid == NULL || *ssid == '\0') {
		nvram_set_int("wlcscan_ssid_rssi_state", -1);
		return -1;
	}

	nvram_set_int("wlcscan_ssid_rssi_state", 1);

	int ret, i, k;
	int retval = 0, ap_count = 0, count = 0;
	unsigned char bssid[6];
	wl_scan_results_t *result;
	wl_scan_params_t *params;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + NUMCHANS * sizeof(uint16);
	char wif[8] = {0}, prefix[] = "wlXXXXXXXXXX_", tmp[100] = {0};

	snprintf(prefix, sizeof(prefix), "wl%d_", band);
	strncpy(wif, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(wif));

	retval = 0;

	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL)
		return retval;

	memset(params, 0, params_size);
	params->bss_type = DOT11_BSSTYPE_INFRASTRUCTURE;
	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->scan_type = DOT11_SCANTYPE_ACTIVE;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	while ((ret = wl_ioctl(wif, WLC_SCAN, params, params_size)) < 0 &&
	count++ < 15) {
		sleep(1);
	}

	free(params);

	count = 0;

	result = (wl_scan_results_t *)buf;
	result->buflen = WLC_IOCTL_MAXLEN - sizeof(result);

	while ((ret = wl_ioctl(wif, WLC_SCAN_RESULTS, apinfos, sizeof(apinfos))) < 0 && count++ < 2)
	{
		sleep(1);
	}

	if (ret == 0)
	{
		for (i = 0; i < APINFO_MAX; i++)
		{
			if (apinfos[i].SSID[0] == 0)
				break;
		}
		ap_count = i;
	}
	if (ap_count == 0) {
		//dbg("[wlc] No AP found!\n");
	} else{
		for (k = 0; k < ap_count; k++)
		{
			if (!strcmp(apinfos[k].SSID, ssid)) {
				snprintf(tmp, sizeof(tmp), "wlc%d_scan_rssi", band);
				nvram_set_int(tmp, apinfos[k].RSSI_Quality);
				nvram_set_int("wlcscan_ssid_rssi_state", 2);
				return retval;
			}
		}
	}

	snprintf(tmp, sizeof(tmp), "wlc%d_scan_rssi", band);
	nvram_set_int(tmp, 0);
	nvram_set_int("wlcscan_ssid_rssi_state", -1);
	return retval;
}
#endif

int wlcscan_core(char *ofile, char *wif)
{
	int ret, i, k, left, ht_extcha;
	int retval = 0, ap_count = 0, idx_same = -1, count = 0;
	unsigned char *bssidp;
	char *info_b;
	unsigned char rate;
	unsigned char bssid[6];
	char macstr[18];
	char ure_mac[18];
	char ssid_str[256];
	wl_scan_results_t *result;
	wl_bss_info_t *info;
	wl_bss_info_107_t *old_info;
	struct bss_ie_hdr *ie;
	NDIS_802_11_NETWORK_TYPE NetWorkType;
	struct maclist *authorized;
	int maclist_size;
	int max_sta_count = 128;
	int wl_authorized = 0;
	wl_scan_params_t *params;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + NUMCHANS * sizeof(uint16);
	FILE *fp;

	retval = 0;

	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL)
		return retval;

	memset(params, 0, params_size);
	params->bss_type = DOT11_BSSTYPE_INFRASTRUCTURE;
	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
//	params->scan_type = -1;
	params->scan_type = DOT11_SCANTYPE_ACTIVE;
//	params->scan_type = DOT11_SCANTYPE_PASSIVE;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;
	dbg("%s:%d\n",__FUNCTION__,__LINE__);

	while ((ret = wl_ioctl(wif, WLC_SCAN, params, params_size)) < 0 &&
				count++ < 15){
		dbg("[rc] set scan command failed, retry %d\n", count);
		sleep(1);
	}
	dbg("%s:%d\n",__FUNCTION__,__LINE__);

	free(params);

	count = 0;
	dbg("%s:%d\n",__FUNCTION__,__LINE__);

	result = (wl_scan_results_t *)buf;
	result->buflen = WLC_IOCTL_MAXLEN - sizeof(result);

	while ((ret = wl_ioctl(wif, WLC_SCAN_RESULTS, apinfos, sizeof(apinfos))) < 0 && count++ < 2)
	{
		dbg("[rc] set scan results command failed, retry %d\n", count);
		sleep(1);
	}
	//dbg("%s:%d count=%d\n",__FUNCTION__,__LINE__,result->count);

	if (ret == 0)
	{
		for(i=0;i<APINFO_MAX;i++)
		{
			dbg("%s:%d type=%d\n",__FUNCTION__,__LINE__,apinfos[i].NetworkType);
			if(apinfos[i].SSID[0]==0)
				break;
		}
		ap_count=i;	
	}
	dbg("%s:%d ap_count=%d\n",__FUNCTION__,__LINE__,ap_count);

	/* Print scanning result to console */
	if (ap_count == 0){
		dbg("[wlc] No AP found!\n");
	}else{
		dbg("%-4s%4s%-33s%-18s%-9s%-16s%-9s%8s%3s%3s\n",
				"idx", "CH ", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "CC", "EC");
		for (k = 0; k < ap_count; k++)
		{
			dbg("%2d. ", k + 1);
			dbg("%3d ", apinfos[k].ctl_ch);
			dbg("%-33s", apinfos[k].SSID);
			dbg("%-18s", apinfos[k].BSSID);

			if (apinfos[k].wpa == 1)
				dbg("%-9s%-16s", wpa_cipher_txt(apinfos[k].wid.pairwise_cipher), wpa_key_mgmt_txt(apinfos[k].wid.key_mgmt, apinfos[k].wid.proto));
			else if (apinfos[k].wep == 1)
				dbg("WEP      Unknown         ");
			else
				dbg("NONE     Open System     ");
			dbg("%9d ", apinfos[k].RSSI_Quality);

			if (apinfos[k].NetworkType == Ndis802_11FH || apinfos[k].NetworkType == Ndis802_11DS)
				dbg("%-7s", "11b");
			else if (apinfos[k].NetworkType == Ndis802_11OFDM5)
				dbg("%-7s", "11a");
			else if (apinfos[k].NetworkType == Ndis802_11OFDM5_VHT)
				dbg("%-7s", "11ac");
			else if (apinfos[k].NetworkType == Ndis802_11OFDM5_N)
				dbg("%-7s", "11a/n");
			else if (apinfos[k].NetworkType == Ndis802_11OFDM24)
				dbg("%-7s", "11b/g");
			else if (apinfos[k].NetworkType == Ndis802_11OFDM24_N)
				dbg("%-7s", "11b/g/n");
			else
				dbg("%-7s", "unknown");

			dbg("%3d", apinfos[k].ctl_ch);

			if (	((apinfos[k].NetworkType == Ndis802_11OFDM5_VHT) ||
				 (apinfos[k].NetworkType == Ndis802_11OFDM5_N) ||
				 (apinfos[k].NetworkType == Ndis802_11OFDM24_N)) &&
					(apinfos[k].channel != apinfos[k].ctl_ch)){
				if (apinfos[k].ctl_ch < apinfos[k].channel)
					ht_extcha = 1;
				else
					ht_extcha = 0;

				dbg("%3d", ht_extcha);
			}

			dbg("\n");
		}
	}

	ret = wl_ioctl(wif, WLC_GET_BSSID, bssid, sizeof(bssid));
	memset(ure_mac, 0x0, 18);
	if (!ret){
		if ( !(!bssid[0] && !bssid[1] && !bssid[2] && !bssid[3] && !bssid[4] && !bssid[5]) ){
			sprintf(ure_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
										(unsigned char)bssid[0],
										(unsigned char)bssid[1],
										(unsigned char)bssid[2],
										(unsigned char)bssid[3],
										(unsigned char)bssid[4],
										(unsigned char)bssid[5]);
		}
	}

	if (strstr(nvram_safe_get(wlc_nvname("akm")), "psk")){
		maclist_size = sizeof(authorized->count) + max_sta_count * sizeof(struct ether_addr);
		authorized = malloc(maclist_size);

		// query wl for authorized sta list
		strcpy((char*)authorized, "autho_sta_list");
		if (!wl_ioctl(wif, WLC_GET_VAR, authorized, maclist_size)){
			if (authorized->count > 0) wl_authorized = 1;
		}

		if (authorized) free(authorized);
	}

	/* Print scanning result to web format */
	if (ap_count > 0){
		/* write pid */
		if ((fp = fopen(ofile, "a")) == NULL){
			printf("[wlcscan] Output %s error\n", ofile);
		}else{
			for (i = 0; i < ap_count; i++){
				/*if(apinfos[i].ctl_ch < 0 ){
					fprintf(fp, "\"ERR_BNAD\",");
				}else */if( apinfos[i].ctl_ch > 0 &&
							 apinfos[i].ctl_ch < 14){
					fprintf(fp, "\"2G\",");
				}else if( apinfos[i].ctl_ch > 14 &&
							 apinfos[i].ctl_ch < 166){
					fprintf(fp, "\"5G\",");
				}else{
					fprintf(fp, "\"ERR_BNAD\",");
				}

				if (strlen(apinfos[i].SSID) == 0){
					fprintf(fp, "\"\",");
				}else{
					memset(ssid_str, 0, sizeof(ssid_str));
					char_to_ascii(ssid_str, apinfos[i].SSID);
					fprintf(fp, "\"%s\",", ssid_str);
				}

				fprintf(fp, "\"%d\",", apinfos[i].ctl_ch);

				if (apinfos[i].wpa == 1){
					if (apinfos[i].wid.key_mgmt == WPA_KEY_MGMT_IEEE8021X_)
						fprintf(fp, "\"%s\",", "WPA-Enterprise");
					else if (apinfos[i].wid.key_mgmt == WPA_KEY_MGMT_IEEE8021X2_)
						fprintf(fp, "\"%s\",", "WPA2-Enterprise");
					else if (apinfos[i].wid.key_mgmt == WPA_KEY_MGMT_PSK_)
						fprintf(fp, "\"%s\",", "WPA-Personal");
					else if (apinfos[i].wid.key_mgmt == WPA_KEY_MGMT_PSK2_)
						fprintf(fp, "\"%s\",", "WPA2-Personal");
					else if (apinfos[i].wid.key_mgmt == WPA_KEY_MGMT_NONE_)
						fprintf(fp, "\"%s\",", "NONE");
					else if (apinfos[i].wid.key_mgmt == WPA_KEY_MGMT_IEEE8021X_NO_WPA_)
						fprintf(fp, "\"%s\",", "IEEE 802.1X");
					else
						fprintf(fp, "\"%s\",", "Unknown");
				}else if (apinfos[i].wep == 1){
					fprintf(fp, "\"%s\",", "Unknown");
				}else{
					fprintf(fp, "\"%s\",", "Open System");
				}

				if (apinfos[i].wpa == 1){
					if (apinfos[i].wid.pairwise_cipher == WPA_CIPHER_NONE_)
						fprintf(fp, "\"%s\",", "NONE");
					else if (apinfos[i].wid.pairwise_cipher == WPA_CIPHER_WEP40_)
						fprintf(fp, "\"%s\",", "WEP");
					else if (apinfos[i].wid.pairwise_cipher == WPA_CIPHER_WEP104_)
						fprintf(fp, "\"%s\",", "WEP");
					else if (apinfos[i].wid.pairwise_cipher == WPA_CIPHER_TKIP_)
						fprintf(fp, "\"%s\",", "TKIP");
					else if (apinfos[i].wid.pairwise_cipher == WPA_CIPHER_CCMP_)
						fprintf(fp, "\"%s\",", "AES");
					else if (apinfos[i].wid.pairwise_cipher == (WPA_CIPHER_TKIP_|WPA_CIPHER_CCMP_))
						fprintf(fp, "\"%s\",", "TKIP+AES");
					else
						fprintf(fp, "\"%s\",", "Unknown");
				}else if (apinfos[i].wep == 1){
					fprintf(fp, "\"%s\",", "WEP");
				}else{
					fprintf(fp, "\"%s\",", "NONE");
				}

				fprintf(fp, "\"%d\",", apinfos[i].RSSI_Quality);
				fprintf(fp, "\"%s\",", apinfos[i].BSSID);

				if (apinfos[i].NetworkType == Ndis802_11FH || apinfos[i].NetworkType == Ndis802_11DS)
					fprintf(fp, "\"%s\",", "b");
				else if (apinfos[i].NetworkType == Ndis802_11OFDM5)
					fprintf(fp, "\"%s\",", "a");
				else if (apinfos[i].NetworkType == Ndis802_11OFDM5_N)
					fprintf(fp, "\"%s\",", "an");
				else if (apinfos[i].NetworkType == Ndis802_11OFDM5_VHT)
					fprintf(fp, "\"%s\",", "ac");
				else if (apinfos[i].NetworkType == Ndis802_11OFDM24)
					fprintf(fp, "\"%s\",", "bg");
				else if (apinfos[i].NetworkType == Ndis802_11OFDM24_N)
					fprintf(fp, "\"%s\",", "bgn");
				else
					fprintf(fp, "\"%s\",", "");

				if (strcmp(nvram_safe_get(wlc_nvname("ssid")), apinfos[i].SSID)){
					if (strcmp(apinfos[i].SSID, ""))
						fprintf(fp, "\"%s\"", "0");				// none
					else if (!strcmp(ure_mac, apinfos[i].BSSID)){
						// hidden AP (null SSID)
						if (strstr(nvram_safe_get(wlc_nvname("akm")), "psk")){
							if (wl_authorized){
								// in profile, connected
								fprintf(fp, "\"%s\"", "4");
							}else{
								// in profile, connecting
								fprintf(fp, "\"%s\"", "5");
							}
						}else{
							// in profile, connected
							fprintf(fp, "\"%s\"", "4");
						}
					}else{
						// hidden AP (null SSID)
						fprintf(fp, "\"%s\"", "0");				// none
					}
				}else if (!strcmp(nvram_safe_get(wlc_nvname("ssid")), apinfos[i].SSID)){
					if (!strlen(ure_mac)){
						// in profile, disconnected
						fprintf(fp, "\"%s\",", "1");
					}else if (!strcmp(ure_mac, apinfos[i].BSSID)){
						if (strstr(nvram_safe_get(wlc_nvname("akm")), "psk")){
							if (wl_authorized){
								// in profile, connected
								fprintf(fp, "\"%s\"", "2");
							}else{
								// in profile, connecting
								fprintf(fp, "\"%s\"", "3");
							}
						}else{
							// in profile, connected
							fprintf(fp, "\"%s\"", "2");
						}
					}else{
						fprintf(fp, "\"%s\"", "0");				// impossible...
					}
				}else{
					// wl0_ssid is empty
					fprintf(fp, "\"%s\"", "0");
				}

				if (i == ap_count - 1){
					fprintf(fp, "\n");
				}else{
					fprintf(fp, "\n");
				}
			}	/* for */
			fclose(fp);
		}
	}	/* if */

	return retval;
}

/*
 * input mib name,like [hwname], wlan0_[wlanname],bluetooth_[btname]
 * output mib value in buffer
 */
int rtk_apmib_get(char* name,char* buffer)
{
	unsigned int offset,size,i;
	MIB_TYPE_T type;
	if(!name||!buffer) {
		fprintf(stderr,"Invalid input!!\n");
		return -1;
	}
	if(flash_get_mib_info(name,&offset,&size,&type)!=0) {
		fprintf(stderr,"get mib fail!\n");
		return -1;
	}
	if(rtk_flash_read(buffer,offset,size)<=0) {
		fprintf(stderr,"read mib from flash fail!\n");
		return -1;
	}
	return 0;
}

/*
 * input mib name,like [hwname], wlan0_[wlanname],bluetooth_[btname]
 * and mib value in buffer
 */
int rtk_apmib_set(char* name,char* buffer)
{
	unsigned int offset,size,i;
	MIB_TYPE_T type;
	if(!name||!buffer) {
		fprintf(stderr,"Invalid input!!\n");
		return -1;
	}
	if(flash_get_mib_info(name,&offset,&size,&type)!=0) {
		fprintf(stderr,"get mib fail!\n");
		return -1;
	}
	if(rtk_flash_write(buffer,offset,size)<=0) {
		fprintf(stderr,"read mib from flash fail!\n");
		return -1;
	}
	return 0;
}

#if defined(RTCONFIG_BT_CONN)
#define RTK_BT_CFG_FILE "/var/firmware/rtlbt/rtl8822b_config"
void gen_rtlbt_fw_config(void)
{
	int fd;
	int count;
	int total_len;
	int i,j;
	unsigned char hw_btaddr[6] = {0};
	unsigned char hw_txpwr_idx[6] = {0};
	unsigned char hw_xtal_cap_val = 0;
	rtk_apmib_get("bluetooth_btAddr",hw_btaddr);
	rtklog("bt hw addr:\n");
	rtklog("%02X:%02X:%02X:%02X:%02X:%02X\n",
		hw_btaddr[0],
		hw_btaddr[1],
		hw_btaddr[2],
		hw_btaddr[3],
		hw_btaddr[4],
		hw_btaddr[5]);
	rtk_apmib_get("bluetooth_txPowerIdx",hw_txpwr_idx);
	rtklog("bt hw tx pwr index:\n");
	for(i=0;i<6;i++)
	{
		rtklog("%d ",hw_txpwr_idx[i]);
	}
	rtklog("\n");
	rtk_apmib_get("wlan0_xCap",&hw_xtal_cap_val);
	rtklog("hw_xtal_cap_val:%d\n",hw_xtal_cap_val);
	system("mkdir /var/firmware");
	system("mkdir /var/firmware/rtlbt");
	system("cp /rom/etc/rtl8822b_fw /var/firmware/rtlbt/");
	unsigned char header[6] = {0x55,0xab,0x23,0x87,0x29,0x00};
	unsigned char baudrate[19] = {0x0c,0x00,0x10,\
		0x0a,0xc0,0x52,0x02,\
		0x50,0xc5,0xea,0x19,\
		0xe1,0x1b,0xfd,0xaf,\
		0x5f,0x01,0xa4,0x0b};
	unsigned char bt_addr[9] = {0x44,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned char txpwr_idx[9] = {0x5a,0x01,0x06,0x19,0x16,0x16,0x16,0x19,0x01};
	unsigned char xtal_cap_val[4] = {0xe6,0x01,0x01,0x00};
	for(i=0,j=5;i<6;i++)
	{
		bt_addr[3+i] = hw_btaddr[j];
		j--;
	}
	for(i=0;i<5;i++)
	{
		if(hw_txpwr_idx[i] != 0)
		{
			txpwr_idx[3+i] = hw_txpwr_idx[i];
		}
	}
	xtal_cap_val[3] = hw_xtal_cap_val;
	fd = open(RTK_BT_CFG_FILE,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if(fd >= 0)
	{
		count = write(fd,header,sizeof(header));
		if(count < 0)
		{
			printf("Failed to write bt config file header!\n");
			goto end;
		}
		count = write(fd,baudrate,sizeof(baudrate));
		if(count < 0)
		{
			printf("Failed to write bt config file baudrate!\n");
			goto end;
		}
		count = write(fd,bt_addr,sizeof(bt_addr));
		{
			if(count < 0)
			{
				printf("Failed to write bt config file BT address!\n");
				goto end;
			}
		}
		count = write(fd,txpwr_idx,sizeof(txpwr_idx));
		if(count < 0)
		{
			printf("Failed to write bt config file txpwr_idx!\n");
			goto end;
		}
		count = write(fd,xtal_cap_val,sizeof(xtal_cap_val));
		if(count < 0)
		{
			printf("Failed to write bt config file xtal_cap_val!\n");
			goto end;
		}
	}
end:
	if(fd >= 0)
	{
		close(fd);
	}
}
#endif
#endif
