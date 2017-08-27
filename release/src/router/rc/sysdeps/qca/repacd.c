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

#include <rc.h>
#ifdef RTCONFIG_QCA
#include <stdio.h>
#include <fcntl.h>	
#include <qca.h>
#include <asm/byteorder.h>
#include <bcmnvram.h>
//#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if_arp.h>
#include <shutils.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mount.h>
#include <net/if.h>
#include <linux/mii.h>
//#include <linux/if.h>
#include <iwlib.h>
//#include <wps.h>
//#include <stapriv.h>
#include <shared.h>
#include "flash_mtd.h"
#include "ate.h"
#endif 

//#define DETECT_5G 1

#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VRZAC1300)
//parameter control
#define wifi_son_mode 1
int hive_daisy_chain=1;  //if 0 , topology is star mode.

int deep_clone=1;
int dbdc_reptr_enable=1-wifi_son_mode;
int block_dfs_enable=0; 	//only for athX
int disable_steering=0;
int scaling_factor=70;
int distance=0;
#ifdef RTCONFIG_DUAL_BACKHAUL
int disable_2g=0;
#endif
int self_restart=60;
int resolving=0;
#define  percent_rate_min 30
#define  percent_rate_max 70

#define	WSPLCD_CONF	"/tmp/wsplcd.conf"
#define	CMP_CONF	"/tmp/wscmp"
#define	CMP_CONF_NEW	"/tmp/wscmp_new"
#define AP_SCAN_FILE	"/tmp/apscan_file.txt"
#ifdef RTCONFIG_ETHBACKHAUL
int check_eth_time=20;  //more than wlready=1 time after watchdog start-up
int eth_down_time=25;
#endif
int dbg_m=0;


int check_period=30;


//int multi_ap_enable=wifi_son_mode;
//int manage_mcsd=1-wifi_son_mode; //stop mcsd(Manage the Multicast Services Daemon) daemon when running wifi-son mode
//int lbd_enable=1-wifi_son_mode; //stop lbd daemon(also disable wifi steering) when running wifi-son mode


int cfg_re_syncing=0;
int cfg_changed=0;
char QCA_DRV[] = "athr" ;

void detect_5gband_bssid(void);

struct {
    char *hwmode;
    char *wsp_std;
} phy_to_std_map[] = {
    { "11NGHT20",      "ng20" },
    { "11NGHT40MINUS", "ng40minus" },
    { "11NGHT40PLUS", "ng40plus" },
    { "11NGHT40", "ng40" },
    { "11NAHT20", "na20" },
    { "11NAHT40MINUS", "na40minus" },
    { "11NAHT40PLUS", "na40plus" },
    { "11NAHT40", "na40" },
    { "11ACVHT20", "acvht20" },
    { "11ACVHT40MINUS", "acvht40minus" },
    { "11ACVHT40PLUS", "acvht40plus" },
    { "11ACVHT40", "acvht40" },
    { "11ACVHT80", "acvht80" },
    { "11ACVHT160", "acvht160" },
    { "11ACVHT80_80", "acvht80_80" },
    { "AUTO", "auto" },
    { NULL, NULL }
};

int get_role(void)
{
	int ro;
	ro=nvram_get_int("sw_mode");
	switch(ro)
	{
		case 1:
			return 0;
		case 3:
			return 1;
		default:
			return -1;
	}
}

extern char *get_wififname(int band);

int get_freq(int band)
{
	char buf[1024];
        FILE *fp;
        int len,freq;
        char *pt1,*pt2;
        char tmp[10];
        sprintf(buf, "iwconfig %s",get_wififname(band));

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "Frequency:");
                        if (pt1)
                        {
                                pt2 = strstr(pt1, "GHz");
                                if(pt2)
                                {
                                        strncpy(tmp,pt1+strlen("Frequency:"),pt2-pt1-strlen("Frequency:"));
                                        chomp(tmp);
					freq=(int)(1000*atof(tmp));
                                        return freq;
				}
			}
		}
	}
	return 0;
}


int get_phyrate(int band)
{
	char buf[1024];
        FILE *fp;
        int len;
        char *pt1,*pt2;
        char tmp[10];
        sprintf(buf, "iwconfig %s",get_wififname(band));

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "Bit Rate:");
                        if (pt1)
                        {
                                pt2 = strstr(pt1, "Mb/s");
                                if(pt2)
                                {
                                        strncpy(tmp,pt1+strlen("Bit Rate:"),pt2-pt1-strlen("Bit Rate:"));
                                        chomp(tmp);
                                        return atoi(tmp);
				}
			}
		}
	}
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



char *get_stamac(int band)
{
        char buf[512];
        FILE *fp;
        int len;
        char *pt1,*pt2;

        sprintf(buf, "ifconfig %s", get_staifname(band));

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "HWaddr ");
                        if (pt1)
                        {
                                pt2 = pt1 + strlen("HWaddr ");
                                chomp(pt2);
                                return pt2;
                        }
                }
        }
        return NULL;
}


char *translate_hwmode(const char *iface)
{
	int i,len;
	FILE *fp;
	char buf[100],*pt1,*mode=NULL;
        sprintf(buf, "iwpriv %s get_mode",iface);

	_dprintf("=> mode=%s\n",buf);
        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "get_mode:");
			if(pt1)
			{
                                mode = pt1 + strlen("get_mode:");
                                chomp(mode);
			}
		}
	}

	_dprintf("=> %s's hwmode=%s\n",iface,mode);
	for (i = 0; phy_to_std_map[i].hwmode; ++i) {
        	if (!strcmp(phy_to_std_map[i].hwmode, mode)) {
            		return phy_to_std_map[i].wsp_std;
		}
	}

	return "";
}


char *translate_crypto(char* val)
{
	char *nvram;
	nvram=nvram_get(val);
	if(!strcmp(nvram,"tkip"))
		return "TKIPEncryption";
	else if(!strcmp(nvram,"aes"))
		return "AESEncryption";
	else if(!strcmp(nvram,"tkip+aes"))
		return "TKIPandAESEncryption";
	else
		return "";
}

char *translate_auth(char* val)
{
	char *nvram;
	nvram=nvram_get(val);
	if(!strcmp(nvram,"open"))
		return "None";
	else if(!strcmp(nvram,"shared"))
		return "SharedAuthentication";
	else if(!strcmp(nvram,"radius"))

		return "EAPAuthentication";
	else if(!strcmp(nvram,"wpa"))
		return "EAPAuthentication";
	else if(!strcmp(nvram,"psk"))
		return "PSKAuthentication";
	else if(!strcmp(nvram,"wpa2"))
		return "EAPAuthentication";
	else if(!strcmp(nvram,"psk2"))
		return "PSKAuthentication";
	else if(!strcmp(nvram,"wpawpa2"))
		return "EAPAuthentication";
	else if(!strcmp(nvram,"pskpsk2"))
		return "PSKAuthentication";
	else
	return "";
}

char *translate_bcn(char *val)
{
	char *nvram;
	nvram=nvram_get(val);
	if(!strcmp(nvram,"tkip"))
		return "WPA";
	else if(!strcmp(nvram,"aes"))
		return "11i";
	else if(!strcmp(nvram,"tkip+aes"))
		return "WPAand11i";
	else
		return "Basic";
}


//generate wsplcd.conf 
static void gen_wsplcd_conf(int role, char *filename, int cmp_only)
{
	int t1,t2;
	char word[PATH_MAX], *next_word;
	char tmp1[200],tmp2[200];
	FILE *fp;
	char *gkey=NULL;

	if (!(fp = fopen(filename, "w+")))
	{
		_dprintf("gen %s file fails!\n", filename);
		return;
	}

	gkey=nvram_get("cfg_group");
	if (!cmp_only) {
	fprintf(fp, "# Config file for wsplcd, automatically created by script\n");
	fprintf(fp, "debug_level=%d\n",nvram_get_int("hive_dbg")?0:3); //normal: 3
	} /* end of cmp_only */
	fprintf(fp, "bridge=%s\n",nvram_get("lan_ifname"));
	memset(tmp1,0,sizeof(tmp1));
	memset(tmp2,0,sizeof(tmp2));
	sprintf(tmp1,"1905InterfacesWlan=");
	sprintf(tmp2,"1905InterfacesElse=");
	t1=strlen(tmp1);
	t2=strlen(tmp2);
	foreach(word, nvram_safe_get("lan_ifnames"), next_word){
		if(strlen(word)<5) //ignore guest network 	
		{
			if(strstr(word, "ath"))
			{
				if(strlen(tmp1)!=t1)
					sprintf(tmp1,"%s,",tmp1);
				sprintf(tmp1, "%s%s:WLAN",tmp1,word);
			}
			if(strstr(word, "eth"))
			{
				if(strlen(tmp2)!=t2)
					sprintf(tmp2,"%s,",tmp2);
				sprintf(tmp2, "%s%s:ETHER",tmp2,word);
			}
			if(role)
			{
				if(strstr(word, "sta"))
				{
					if(strlen(tmp1)!=t1)
						sprintf(tmp1,"%s,",tmp1);
					sprintf(tmp1, "%s%s:WLAN",tmp1,word);
				}
			}
		}		
	}

	fprintf(fp,"%s\n",tmp1);
	fprintf(fp,"%s\n",tmp2);
	//#role: 0=>register 1:emrollee 2:none
	if (!cmp_only) {
	fprintf(fp, "role=%d\n",role);
	fprintf(fp, "designated_pb_ap=0\n");
	fprintf(fp, "WPS_method=M2\n");
	fprintf(fp, "WPS_tx_mode=encrypted\n");
	fprintf(fp, "WPS_rx_mode=encrypted\n");
	fprintf(fp, "config_station=yes\n");
	fprintf(fp, "search_timeout=60\n");
	fprintf(fp, "WPS_session_timeout=120\n");
	fprintf(fp, "WPS_retransmission_timeout=5\n");
	fprintf(fp, "WPS_per_message_timeout=15\n");
	fprintf(fp, "band_sel_enable=0\n");
	fprintf(fp, "band_choice=5G\n");  //default: 5G
	fprintf(fp, "rm_collect_timeout=10\n");
	fprintf(fp, "deep_clone_enable=%d\n",deep_clone);
	fprintf(fp, "deep_clone_no_bssid=%d\n",role?(hive_daisy_chain?1:0):0);
	fprintf(fp, "manage_vap_ind=0\n");
	fprintf(fp, "wpa_passphrase_type=0\n");
	fprintf(fp, "config_methods=push_button virtual_push_button physical_push_button\n");
	fprintf(fp, "manufacturer=Qualcomm Atheros\n");
	fprintf(fp, "model_name=ModelNameHere\n");
	fprintf(fp, "model_number=ModelNumberHere\n");
	fprintf(fp, "serial_number=SerialNumberHere\n");
	fprintf(fp, "device_type=6-0050f204-1\n");
	fprintf(fp, "device_name=ExampleWpa\n");
	fprintf(fp, "os_version=01020300\n");
	fprintf(fp, "#AP Cloning 1.0 parameters\n");
	fprintf(fp, "APCloning=0\n");
	fprintf(fp, "button_mode=2\n");
	fprintf(fp, "clone_timeout=180\n"); //default:180 sec
	fprintf(fp, "walk_timeout=120\n");
	fprintf(fp, "repeat_timeout=2\n");
	fprintf(fp, "internal_timeout=15\n");
	fprintf(fp, "wait_wifi_config_secs_other=30\n");
	fprintf(fp, "wait_wifi_config_secs_first=30\n");
	fprintf(fp, "atf_config_en=0\n");
	fprintf(fp, "cfg_changed=%d\n",cfg_changed);
	fprintf(fp, "cfg_restart_long_timeout=%d\n",self_restart);
	fprintf(fp, "cfg_restart_short_timeout=30\n");
	fprintf(fp, "cfg_apply_timeout=%d\n",nvram_get_int("hive_apply_time")?20:10);
	} /* end of cmp_only */
	//wifi0
	fprintf(fp, "RADIO.1.Channel=%d\n",nvram_get_int("wl0_channel"));
	fprintf(fp, "RADIO.1.RadioEnabled=1\n"); 
	//2G,ath
	fprintf(fp, "WLAN.1.Enable=%s\n",nvram_get("wl0_radio")); 
	fprintf(fp, "WLAN.1.X_ATH-COM_RadioIndex=1\n");
	fprintf(fp, "WLAN.1.BSSID=%s\n",nvram_get("wl0_hwaddr")); 
	fprintf(fp, "WLAN.1.SSID=%s\n",nvram_get("wl0_ssid"));
	if (!cmp_only) {
	fprintf(fp, "WLAN.1.Standard=%s\n",translate_hwmode(WIF_2G));
	} /* end of cmp_only */
	//fprintf(fp, "WLAN.1.Standard=ng20\n");
	fprintf(fp, "WLAN.1.Channel=%d\n",nvram_get_int("wl0_channel"));
	fprintf(fp, "WLAN.1.BeaconType=%s\n",translate_bcn("wl0_crypto"));
	fprintf(fp, "WLAN.1.BasicEncryptionModes=\n");
	fprintf(fp, "WLAN.1.BasicAuthenticationMode=\n");
	fprintf(fp, "WLAN.1.WPAEncryptionModes=\n");
	fprintf(fp, "WLAN.1.WPAAuthenticationMode=\n");
	fprintf(fp, "WLAN.1.IEEE11iEncryptionModes=%s\n",translate_crypto("wl0_crypto"));
	fprintf(fp, "WLAN.1.IEEE11iAuthenticationMode=%s\n",translate_auth("wl0_auth_mode_x")); 
	fprintf(fp, "WLAN.1.KeyPassphrase=%s\n",nvram_get("wl0_wpa_psk")); 
	fprintf(fp, "WLAN.1.DeviceOperationMode=RootAP\n"); //ap
	fprintf(fp, "WLAN.1.X_ATH-COM_APModuleEnable=1\n");
	fprintf(fp, "WLAN.1.X_ATH-COM_WPSPin=12345670\n");
	fprintf(fp, "WLAN.1.X_ATH-COM_VapIfname=%s\n",nvram_get("wl0_ifname")); //2g iface
	fprintf(fp, "WLAN.1.X_ATH-COM_WPSConfigured=CONFIGURED\n");
	fprintf(fp, "WLAN.1.X_ATH-COM_HT40Coexist=1\n");
	fprintf(fp, "WLAN.1.WsplcdUnmanaged=0\n");
	if (!cmp_only) {
	fprintf(fp, "WLAN.1.INFORE=%d\n",cfg_re_syncing);
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.1.GROUPKEY=%s\n",gkey);
#if defined(MAPAC2200)
	fprintf(fp, "WLAN.1.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
	fprintf(fp, "WLAN.1.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
#endif
	//2G,sta
	fprintf(fp, "WLAN.3.Enable=%d\n",role?1:0); 
	fprintf(fp, "WLAN.3.X_ATH-COM_RadioIndex=1\n");
#ifdef RTCONFIG_DUAL_BACKHAUL
	if(role)
		fprintf(fp, "WLAN.3.BSSID=%s\n",get_stamac(0)?get_stamac(0):""); //self mac
	else
#endif
		fprintf(fp, "WLAN.3.BSSID=\n");
	fprintf(fp, "WLAN.3.SSID=%s\n",nvram_get("wl0_ssid"));  //use ath's ssid
#ifdef RTCONFIG_DUAL_BACKHAUL
	if(role)
		fprintf(fp, "WLAN.3.Standard=%s\n",translate_hwmode(get_staifname(0)));
	else
#endif
		fprintf(fp, "WLAN.3.Standard=\n");
	fprintf(fp, "WLAN.3.Channel=%d\n",nvram_get_int("wl0_channel"));
	fprintf(fp, "WLAN.3.BeaconType=%s\n",translate_bcn("wl0_crypto"));
	fprintf(fp, "WLAN.3.BasicEncryptionModes=\n");
	fprintf(fp, "WLAN.3.BasicAuthenticationMode=\n");
	fprintf(fp, "WLAN.3.WPAEncryptionModes=\n");
	fprintf(fp, "WLAN.3.WPAAuthenticationMode=\n");
	fprintf(fp, "WLAN.3.IEEE11iEncryptionModes=%s\n",translate_crypto("wl0_crypto")); 
	fprintf(fp, "WLAN.3.IEEE11iAuthenticationMode=%s\n",nvram_get("wl0_auth_mode_x")); 
	fprintf(fp, "WLAN.3.KeyPassphrase=%s\n",nvram_get("wl0_wpa_psk"));  //use ath's key
	fprintf(fp, "WLAN.3.DeviceOperationMode=WDSStation\n"); //wds station
	fprintf(fp, "WLAN.3.X_ATH-COM_APModuleEnable=%d\n",role?1:0);
	fprintf(fp, "WLAN.3.X_ATH-COM_WPSPin=12345670\n");
#ifdef RTCONFIG_DUAL_BACKHAUL
	if(role)
		fprintf(fp, "WLAN.3.X_ATH-COM_VapIfname=%s\n", get_staifname(0));
	else
#endif
		fprintf(fp, "WLAN.3.X_ATH-COM_VapIfname=\n");
	fprintf(fp, "WLAN.3.X_ATH-COM_WPSConfigured=CONFIGURED\n");
	fprintf(fp, "WLAN.3.X_ATH-COM_HT40Coexist=1\n");
	fprintf(fp, "WLAN.3.WsplcdUnmanaged=0\n");
	fprintf(fp, "WLAN.3.INFORE=%d\n",cfg_re_syncing);
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.3.GROUPKEY=%s\n",gkey);
	} /* end of cmp_only */
#if defined(MAPAC2200)
	fprintf(fp, "WLAN.3.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
	fprintf(fp, "WLAN.3.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
#endif

	//wifi1
	fprintf(fp, "RADIO.2.Channel=%d\n",nvram_get_int("wl1_channel"));
	fprintf(fp, "RADIO.2.RadioEnabled=1\n");
	//5G,ath
	fprintf(fp, "WLAN.2.Enable=%s\n",nvram_get("wl1_radio")); 
	fprintf(fp, "WLAN.2.X_ATH-COM_RadioIndex=2\n");
	fprintf(fp, "WLAN.2.BSSID=%s\n",nvram_get("wl1_hwaddr")); 
	fprintf(fp, "WLAN.2.SSID=%s\n",nvram_get("wl1_ssid"));
	if (!cmp_only) {
	fprintf(fp, "WLAN.2.Standard=%s\n",translate_hwmode(WIF_5G)); 
	//fprintf(fp, "WLAN.2.Standard=acvht80\n");
	} /* end of cmp_only */
	fprintf(fp, "WLAN.2.Channel=%d\n",nvram_get_int("wl1_channel"));
	fprintf(fp, "WLAN.2.BeaconType=%s\n",translate_bcn("wl1_crypto"));
	fprintf(fp, "WLAN.2.BasicEncryptionModes=\n");
	fprintf(fp, "WLAN.2.BasicAuthenticationMode=\n");
	fprintf(fp, "WLAN.2.WPAEncryptionModes=\n");
	fprintf(fp, "WLAN.2.WPAAuthenticationMode=\n");
	fprintf(fp, "WLAN.2.IEEE11iEncryptionModes=%s\n",translate_crypto("wl1_crypto")); 
	fprintf(fp, "WLAN.2.IEEE11iAuthenticationMode=%s\n",translate_auth("wl1_auth_mode_x")); 
	fprintf(fp, "WLAN.2.KeyPassphrase=%s\n",nvram_get("wl1_wpa_psk")); 
	fprintf(fp, "WLAN.2.DeviceOperationMode=RootAP\n"); //ap
	fprintf(fp, "WLAN.2.X_ATH-COM_APModuleEnable=1\n");
	fprintf(fp, "WLAN.2.X_ATH-COM_WPSPin=12345670\n");
	fprintf(fp, "WLAN.2.X_ATH-COM_VapIfname=%s\n",nvram_get("wl1_ifname")); //5g iface
	fprintf(fp, "WLAN.2.X_ATH-COM_WPSConfigured=CONFIGURED\n");
	fprintf(fp, "WLAN.2.X_ATH-COM_HT40Coexist=1\n");
	fprintf(fp, "WLAN.2.WsplcdUnmanaged=0\n");
	if (!cmp_only) {
	fprintf(fp, "WLAN.2.INFORE=%d\n",cfg_re_syncing);
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.2.GROUPKEY=%s\n",gkey);
#if defined(MAPAC2200)
	fprintf(fp, "WLAN.2.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
	fprintf(fp, "WLAN.2.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
#endif

	//5G,sta
	fprintf(fp, "WLAN.4.Enable=%d\n",role?1:0); 
	fprintf(fp, "WLAN.4.X_ATH-COM_RadioIndex=2\n");
	if(role)
		fprintf(fp, "WLAN.4.BSSID=%s\n",get_stamac(1)?get_stamac(1):""); //self mac 
	else
		fprintf(fp, "WLAN.4.BSSID=\n"); 
	fprintf(fp, "WLAN.4.SSID=%s\n",nvram_get("wl1_ssid")); //use ath's ssid
	if(role)
		fprintf(fp, "WLAN.4.Standard=%s\n",translate_hwmode(get_staifname(1)));
	else
		fprintf(fp, "WLAN.4.Standard=\n");
	fprintf(fp, "WLAN.4.Channel=%d\n",nvram_get_int("wl1_channel"));
	fprintf(fp, "WLAN.4.BeaconType=%s\n",translate_bcn("wl1_crypto"));
	fprintf(fp, "WLAN.4.BasicEncryptionModes=\n");
	fprintf(fp, "WLAN.4.BasicAuthenticationMode=\n");
	fprintf(fp, "WLAN.4.WPAEncryptionModes=\n");
	fprintf(fp, "WLAN.4.WPAAuthenticationMode=\n");
	fprintf(fp, "WLAN.4.IEEE11iEncryptionModes=%s\n",translate_crypto("wl1_crypto")); 
	fprintf(fp, "WLAN.4.IEEE11iAuthenticationMode=%s\n",translate_auth("wl1_auth_mode_x")); 
	fprintf(fp, "WLAN.4.KeyPassphrase=%s\n",nvram_get("wl1_wpa_psk"));  //use ath's key
	fprintf(fp, "WLAN.4.DeviceOperationMode=WDSStation\n"); //wds station
	fprintf(fp, "WLAN.4.X_ATH-COM_APModuleEnable=%d\n",role?1:0);
	fprintf(fp, "WLAN.4.X_ATH-COM_WPSPin=12345670\n");
	if(role)
		fprintf(fp, "WLAN.4.X_ATH-COM_VapIfname=%s\n", get_staifname(1));
	else
		fprintf(fp, "WLAN.4.X_ATH-COM_VapIfname=\n");
	fprintf(fp, "WLAN.4.X_ATH-COM_WPSConfigured=CONFIGURED\n");
	fprintf(fp, "WLAN.4.X_ATH-COM_HT40Coexist=1\n");
	fprintf(fp, "WLAN.4.WsplcdUnmanaged=0\n");
	fprintf(fp, "WLAN.4.INFORE=%d\n",cfg_re_syncing);
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.4.GROUPKEY=%s\n",gkey);
	} /* end of cmp_only */
#if defined(MAPAC2200)
	fprintf(fp, "WLAN.4.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
	fprintf(fp, "WLAN.4.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
#endif

#if defined(MAPAC2200)
	//wifi2
	fprintf(fp, "RADIO.3.Channel=%d\n",nvram_get_int("wl2_channel"));
	fprintf(fp, "RADIO.3.RadioEnabled=1\n");
	//5G-2,ath
	fprintf(fp, "WLAN.5.Enable=%s\n",nvram_get("wl2_radio")); 
	fprintf(fp, "WLAN.5.X_ATH-COM_RadioIndex=3\n");
	fprintf(fp, "WLAN.5.BSSID=%s\n",nvram_get("wl2_hwaddr")); 
	fprintf(fp, "WLAN.5.SSID=%s\n",nvram_get("wl2_ssid"));
	if (!cmp_only) {
	fprintf(fp, "WLAN.5.Standard=%s\n",translate_hwmode(WIF_5G2)); 
	//fprintf(fp, "WLAN.5.Standard=acvht80\n");
	} /* end of cmp_only */
	fprintf(fp, "WLAN.5.Channel=%d\n",nvram_get_int("wl2_channel"));
	fprintf(fp, "WLAN.5.BeaconType=%s\n",translate_bcn("wl2_crypto"));
	fprintf(fp, "WLAN.5.BasicEncryptionModes=\n");
	fprintf(fp, "WLAN.5.BasicAuthenticationMode=\n");
	fprintf(fp, "WLAN.5.WPAEncryptionModes=\n");
	fprintf(fp, "WLAN.5.WPAAuthenticationMode=\n");
	fprintf(fp, "WLAN.5.IEEE11iEncryptionModes=%s\n",translate_crypto("wl2_crypto")); 
	fprintf(fp, "WLAN.5.IEEE11iAuthenticationMode=%s\n",translate_auth("wl2_auth_mode_x")); 
	fprintf(fp, "WLAN.5.KeyPassphrase=%s\n",nvram_get("wl2_wpa_psk")); 
	fprintf(fp, "WLAN.5.DeviceOperationMode=RootAP\n"); //ap
	fprintf(fp, "WLAN.5.X_ATH-COM_APModuleEnable=1\n");
	fprintf(fp, "WLAN.5.X_ATH-COM_WPSPin=12345670\n");
	fprintf(fp, "WLAN.5.X_ATH-COM_VapIfname=%s\n",nvram_get("wl2_ifname")); //5g iface
	fprintf(fp, "WLAN.5.X_ATH-COM_WPSConfigured=CONFIGURED\n");
	fprintf(fp, "WLAN.5.X_ATH-COM_HT40Coexist=1\n");
	fprintf(fp, "WLAN.5.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#ifndef RTCONFIG_DUAL_BACKHAUL
	fprintf(fp, "WLAN.5.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
#endif
	//WLAN.2 (ap,5G-1), WLAN.5(ap,5G-2): One of them can do sync. Disable 5G-2 sync by wsplcdunmanaged=1!!
	if (!cmp_only) {
	fprintf(fp, "WLAN.5.WsplcdUnmanaged=1\n");
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.5.GROUPKEY=%s\n",gkey);
	} /* end of cmp_only */

#endif
	fclose(fp);
}

	
//generate hyd.conf 
void gen_hyd_conf(int role)
{
	char now[100];
	int t1,t2;
	char word[PATH_MAX], *next_word;
	char tmp1[200],tmp2[200];
	FILE *fp;
	if (!(fp = fopen("/tmp/hyd.conf", "w+")))
		return;
	sprintf(now, "%lu", uptime());
	nvram_set("watchdog_hyd", now);
	fprintf(fp, ";\n");
	fprintf(fp, ";  Automatically generated hyd configure file,do not change it.\n");
	fprintf(fp, ";\n");
	fprintf(fp, ";  INTERFACE:       interface manager\n");
	fprintf(fp, ";  HY:              hy manager\n");
	fprintf(fp, ";  WLAN:            wlan manager\n");
	fprintf(fp, ";  PLC:             plc manager\n");
	fprintf(fp, ";  ETH:             eth manager\n");
	fprintf(fp, ";  PATHCH:          pc service\n");
	fprintf(fp, ";  PATHCHWLAN:      pcw service\n");
	fprintf(fp, ";  PATHCHPLC:       pcp service\n");
	fprintf(fp, ";  PATHCHETH:       pce service\n");
	fprintf(fp, ";  TOPOLOGY:        td service\n");
	fprintf(fp, ";  HSPECEST:        he service\n");
	fprintf(fp, ";  PATHSELECT:      ps service\n");
	fprintf(fp, ";  LOGSETTINGS:     log service\n");
	fprintf(fp, ";  IEEE1905:        IEEE 1905.1 settings\n");
	fprintf(fp, ";  HCP:             HCP settings\n");
	fprintf(fp, ";\n");

	fprintf(fp, "[INTERFACE]\n");
	fprintf(fp, "HybridBridgeInterfaceName=%s\n",nvram_get("lan_ifname"));
	memset(tmp1,0,sizeof(tmp1));
	memset(tmp2,0,sizeof(tmp2));
        sprintf(tmp1,"ManagedInterfacesList=");
        t1=strlen(tmp1);
        t2=strlen(tmp2);
        foreach(word, nvram_safe_get("lan_ifnames"), next_word){
		//if(strlen(word)<5) //ignore guest network 	
		{

           	    	if(strlen(tmp1)!=t1)
			{
                      		sprintf(tmp1,"%s,",tmp1);
				t1=strlen(tmp1);
			}
           	    	if(strlen(tmp2)!=t2)
			{
                        	sprintf(tmp2,"%s,",tmp2);
				t2=strlen(tmp2);
			}
               		if(strstr(word, "ath"))
                        	sprintf(tmp1, "%s%s:WLAN",tmp1,word);

               		else if(strstr(word, "sta"))
			{
				if(role)//re
				{
                        		sprintf(tmp1, "%s%s:WLAN",tmp1,word);
					sprintf(tmp2,"%s%s",tmp2,word);
				}
			}
                	else 
                        	sprintf(tmp1, "%s%s:ETHER",tmp1,word);
			
		}

        }
	fprintf(fp,"%s\n",tmp1);
	if(role)
	{
		fprintf(fp, "NoRelayGroupList=%s\n",tmp2); //lan group
		fprintf(fp, "ForceGroupRelaying=1\n");
	}
	fprintf(fp, "SwitchLanVid=1\n"); //default 1
	fprintf(fp, "SwitchCpuPort=0\n"); //default 0
	fprintf(fp, "VlanIds=\n"); 

	fprintf(fp, "[HY]\n");
	fprintf(fp, "LoadBalancingSeamless=1\n");
	fprintf(fp, "ConstrainTCPMedium=0\n");
	fprintf(fp, "MaxLBReordTimeout=1500\n");
	fprintf(fp, "HActiveMaxAge=120000\n");

	fprintf(fp, "[PATHCHWLAN]\n");
	fprintf(fp, "UpdatedStatsInterval_W2=1\n");
	fprintf(fp, "StatsAgedOutInterval_W2=30\n");
	fprintf(fp, "MaxMediumUtilization_W2=70\n");
	fprintf(fp, "MediumChangeThreshold_W2=10\n");
	fprintf(fp, "LinkChangeThreshold_W2=10\n");
#ifdef RTCONFIG_DUAL_BACKHAUL
	fprintf(fp, "MaxMediumUtilizationForLC_W2=70\n");
#else
	fprintf(fp, "MaxMediumUtilizationForLC_W2=0\n");
#endif
	fprintf(fp, "CPULimitedTCPThroughput_W2=4294967295\n");
	fprintf(fp, "CPULimitedUDPThroughput_W2=4294967295\n");
	fprintf(fp, "PHYRateThresholdForMU_W2=2000\n");
	fprintf(fp, "ProbePacketInterval_W2=1\n");
	fprintf(fp, "ProbePacketSize_W2=64\n");
	fprintf(fp, "EnableProbe_W2=1\n");
	fprintf(fp, "AssocDetectionDelay_W2=5\n");
	fprintf(fp, "UpdatedStatsInterval_W5=1\n");
	fprintf(fp, "StatsAgedOutInterval_W5=30\n");
	fprintf(fp, "MaxMediumUtilization_W5=70\n");
	fprintf(fp, "MediumChangeThreshold_W5=10\n");
	fprintf(fp, "LinkChangeThreshold_W5=10\n");
#ifdef RTCONFIG_DUAL_BACKHAUL
	fprintf(fp, "MaxMediumUtilizationForLC_W5=70\n");
#else
	fprintf(fp, "MaxMediumUtilizationForLC_W5=99\n");
#endif
	fprintf(fp, "CPULimitedTCPThroughput_W5=4294967295\n");
	fprintf(fp, "CPULimitedUDPThroughput_W5=4294967295\n");
	fprintf(fp, "PHYRateThresholdForMU_W5=2000\n");
	fprintf(fp, "ProbePacketInterval_W5=1\n");
	fprintf(fp, "ProbePacketSize_W5=64\n");
	fprintf(fp, "EnableProbe_W5=1\n");
	fprintf(fp, "AssocDetectionDelay_W5=5\n");
	fprintf(fp, "LinkCapacityThreshold=20\n");
	fprintf(fp, "ScalingFactorHighRate_W5=750\n");
	fprintf(fp, "ScalingFactorHighRate_W2=200\n");
	fprintf(fp, "ScalingFactorLow=60\n");
	fprintf(fp, "ScalingFactorMedium=85\n");
	fprintf(fp, "ScalingFactorHigh=60\n");
	fprintf(fp, "ScalingFactorTCP=90\n");
	fprintf(fp, "UseWHCAlgorithm=1\n");

	fprintf(fp, "[PATHCHPLC]\n");
	fprintf(fp, "MaxMediumUtilization=80\n");
	fprintf(fp, "MediumChangeThreshold=10\n");
	fprintf(fp, "LinkChangeThreshold=10\n");
	fprintf(fp, "StatsAgedOutInterval=60\n");
	fprintf(fp, "UpdateStatsInterval=1\n");
	fprintf(fp, "EntryExpirationInterval=120\n");
	fprintf(fp, "MaxMediumUtilizationForLC=80\n");
	fprintf(fp, "LCThresholdForUnreachable=5\n");
	fprintf(fp, "LCThresholdForReachable=10\n");
	fprintf(fp, "HostPLCInterfaceSpeed=0\n");

	fprintf(fp, "[TOPOLOGY]\n");
	fprintf(fp, "BD_UPDATE_INTERVAL=3\n");
	fprintf(fp, "HOLDING_TIME=190\n");
	fprintf(fp, "TIMER_LOW_BOUND=7\n");
	fprintf(fp, "TIMER_UPPER_BOUND=11\n");
	fprintf(fp, "MSGID_DELTA=64\n");
	fprintf(fp, "HA_AGING_INTERVAL=120\n");
	fprintf(fp, "ENABLE_TD3=1\n");
	fprintf(fp, "ENABLE_BD_SPOOFING=1\n");
	fprintf(fp, "NOTIFICATION_THROTTLING_WINDOW=1\n");
	fprintf(fp, "PERIODIC_QUERY_INTERVAL=60\n");
	fprintf(fp, "ENABLE_NOTIFICATION_UNICAST=0\n");


	fprintf(fp, "[PATHSELECT]\n");
	fprintf(fp, "UpdateHDInterval=10\n");
	fprintf(fp, "LinkCapacityThreshold=20\n");
	fprintf(fp, "UDPInterfaceOrder=EP52\n");
	fprintf(fp, "NonUDPInterfaceOrder=EP52\n");
	fprintf(fp, "SerialflowIterations=10\n");
	fprintf(fp, "DeltaLCThreshold=10\n");

	fprintf(fp, "[HSPECEST]\n");
	fprintf(fp, "UpdateHSPECInterval=1\n");
	fprintf(fp, "NotificationThresholdLimit=10\n");
	fprintf(fp, "NotificationThresholdPercentage=20\n");
	fprintf(fp, "AlphaNumerator=3\n");
	fprintf(fp, "AlphaDenominator=8\n");
	fprintf(fp, "LocalFlowRateThreshold=2000000\n");
	fprintf(fp, "LocalFlowRatioThreshold=5\n");
	fprintf(fp, "MaxHActiveEntries=8192\n");

	fprintf(fp, "[LOGSETTINGS]\n");
	fprintf(fp, "EnableLog=0\n");
	fprintf(fp, "LogRestartIntervalSec=10\n");
	fprintf(fp, "LogServerIP=192.168.1.10\n");
	fprintf(fp, "LogServerPort=5555\n");
	fprintf(fp, "EnableLogPCW2=1\n");
	fprintf(fp, "EnableLogPCW5=1\n");
	fprintf(fp, "EnableLogPCP=1\n");
	fprintf(fp, "EnableLogTD=1\n");
	fprintf(fp, "EnableLogHE=1\n");
	fprintf(fp, "EnableLogPS=1\n");
	fprintf(fp, "LogHEThreshold1=200000\n");
	fprintf(fp, "LogHEThreshold2=10000000\n");

	fprintf(fp, "[IEEE1905]\n");
	fprintf(fp, "StrictIEEE1905Mode=0\n");
#ifdef RTCONFIG_ETHBACKHAUL
	fprintf(fp, "GenerateLLDP=1\n");
#else
	fprintf(fp, "GenerateLLDP=0\n");
#endif

	fprintf(fp, "[HCP]\n");
	fprintf(fp, "V1Compat=1\n");

	fprintf(fp, "[HCP]\n");
	fprintf(fp, "WlanCheckFreqInterval=10\n");

	fprintf(fp, "[STEERMSG]\n");
	fprintf(fp, "AvgUtilReqTimeout=1\n");
	fprintf(fp, "LoadBalancingCompleteTimeout=90\n");
	fprintf(fp, "RspTimeout=2\n");

	fprintf(fp, "[WLB]\n");
	fprintf(fp, "DisableSteering=%d\n",disable_steering);
	fprintf(fp, ";\n");

	fprintf(fp, "; \n"); 
	fprintf(fp, ";  Automatically generated Wi-Fi load balancing configuration\n");
	fprintf(fp, ";\n");
	fprintf(fp, ";WLANIF		list of wlan interfaces\n");
	fprintf(fp, ";WLANIF2G		wlan driver interface for 2.4 GHz band\n");
	fprintf(fp, ";WLANIF5G		wlan driver interface for 5 GHz band\n");
	fprintf(fp, ";STADB:		station database\n");
	fprintf(fp, ";STAMON:		station monitor\n");
	fprintf(fp, ";BANDMON:		band monitor\n");
	fprintf(fp, ";ESTIMATOR:		rate estimator\n");
	fprintf(fp, ";STEEREXEC:		steering executor\n");
	fprintf(fp, ";STEERALG:		steering algorithm\n");
	fprintf(fp, ";DIAGLOG:		diagnostic logging\n");

	fprintf(fp, "[WLANIF]\n");
	memset(tmp1,0,sizeof(tmp1));
        sprintf(tmp1,"WlanInterfaces=");
        t1=strlen(tmp1);
        foreach(word, nvram_safe_get("lan_ifnames"), next_word){
		//if(strlen(word)<5) //ignore guest network, such as ath001 	
		{
               	 	if(strlen(tmp1)!=t1 && strstr(word,"ath"))
			{
                 	       sprintf(tmp1,"%s,",tmp1);
			       t1=strlen(tmp1);
			}
                	if(strstr(word, "ath0"))
                       	       sprintf(tmp1, "%swifi0:%s",tmp1,word);
                	if(strstr(word, "ath1"))
                               sprintf(tmp1, "%swifi1:%s",tmp1,word);
#if defined(MAPAC2200)
                	if(strstr(word, "ath2"))
                               sprintf(tmp1, "%swifi2:%s",tmp1,word);
#endif
                               
		}
        }
	fprintf(fp,"%s\n",tmp1);

	fprintf(fp, "[WLANIF2G]\n");
	fprintf(fp, "InterferenceDetectionEnable=1\n");
	fprintf(fp, "InactIdleThreshold=10\n");
	fprintf(fp, "InactOverloadThreshold=10\n");
	fprintf(fp, "InactCheckInterval=1\n");
	fprintf(fp, "AuthAllow=0\n");
	fprintf(fp, "InactRSSIXingHighThreshold=35\n");
	fprintf(fp, "LowRSSIXingThreshold=10\n");
	fprintf(fp, "BcnrptActiveDuration=50\n");
	fprintf(fp, "BcnrptPassiveDuration=200\n");
	fprintf(fp, "HighTxRateXingThreshold=50000\n");
	fprintf(fp, "HighRateRSSIXingThreshold=30\n");
	fprintf(fp, "LowRSSIAPSteeringThreshold=20\n");
	fprintf(fp, "Delay24GProbeRSSIThreshold=35\n");
	fprintf(fp, "Delay24GProbeTimeWindow=0\n");
	fprintf(fp, "Delay24GProbeMinReqCount=0\n");

	fprintf(fp, "[WLANIF5G]\n");
	fprintf(fp, "InterferenceDetectionEnable=1\n");
	fprintf(fp, "InactIdleThreshold=10\n");
	fprintf(fp, "InactOverloadThreshold=10\n");
	fprintf(fp, "InactCheckInterval=1\n");
	fprintf(fp, "AuthAllow=0\n");
	fprintf(fp, "InactRSSIXingHighThreshold=20\n");
	fprintf(fp, "InactRSSIXingLowThreshold=0\n");
	fprintf(fp, "LowRSSIXingThreshold=10\n");
	fprintf(fp, "BcnrptActiveDuration=50\n");
	fprintf(fp, "BcnrptPassiveDuration=200\n");
	fprintf(fp, "LowTxRateXingThreshold=6000\n");
	fprintf(fp, "LowRateRSSIXingThreshold=0\n");
	fprintf(fp, "LowRSSIAPSteeringThreshold=20\n");
	
	fprintf(fp, "[STADB]\n");
	fprintf(fp, "IncludeOutOfNetwork=1\n");
	fprintf(fp, "AgingSizeThreshold=100\n");
	fprintf(fp, "AgingFrequency=60\n");
	fprintf(fp, "OutOfNetworkMaxAge=300\n");
	fprintf(fp, "InNetworkMaxAge=2592000\n");
	fprintf(fp, "ProbeMaxInterval=5\n");
	fprintf(fp, "NumRemoteBSSes=4\n");
	fprintf(fp, "MarkAdvClientAsDualBand=0\n");
	fprintf(fp, "PopulateNonServingPHYInfo=1\n");

	fprintf(fp, "[STAMON]\n");
	fprintf(fp, "RSSIMeasureSamples_W2=5\n");
	fprintf(fp, "RSSIMeasureSamples_W5=5\n");
	fprintf(fp, "AgeLimit=5\n");
	fprintf(fp, "HighTxRateXingThreshold=50000\n");
	fprintf(fp, "HighRateRSSIXingThreshold=30\n");
	fprintf(fp, "LowTxRateXingThreshold=6000\n");
	fprintf(fp, "LowRateRSSIXingThreshold=0\n");
	fprintf(fp, "RSSISteeringPoint_DG=5\n");
	fprintf(fp, "LowRSSIAPSteeringThreshold_W2=20\n");
	fprintf(fp, "LowRSSIAPSteeringThreshold_W5=20\n");

	fprintf(fp, "[BANDMON]\n");
	fprintf(fp, "MUOverloadThreshold_W2=70\n");
	fprintf(fp, "MUOverloadThreshold_W5=70\n");
	fprintf(fp, "MUSafetyThreshold_W2=50\n");
	fprintf(fp, "MUSafetyThreshold_W5=60\n");
	fprintf(fp, "RSSISafetyThreshold=20\n");
	fprintf(fp, "RSSIMaxAge=5\n");
	fprintf(fp, "ProbeCountThreshold=1\n");
	fprintf(fp, "MUReportPeriod=30\n");
	fprintf(fp, "LoadBalancingAllowedMaxPeriod=15\n");
	fprintf(fp, "NumRemoteChannels=3\n");

	fprintf(fp, "[ESTIMATOR]\n");
	fprintf(fp, "AgeLimit=5\n");
	fprintf(fp, "RSSIDiff_EstW5FromW2=-15\n");
	fprintf(fp, "RSSIDiff_EstW2FromW5=5\n");
	fprintf(fp, "ProbeCountThreshold=3\n");
	fprintf(fp, "StatsSampleInterval=1\n");
	fprintf(fp, "11kProhibitTimeShort=30\n");
	fprintf(fp, "11kProhibitTimeLong=300\n");
	fprintf(fp, "PhyRateScalingForAirtime=50\n");
	fprintf(fp, "EnableContinuousThroughput=0\n");
	fprintf(fp, "MaxPollutionTime=1200\n");
	fprintf(fp, "FastPollutionDetectBufSize=10\n");
	fprintf(fp, "NormalPollutionDetectBufSize=10\n");
	fprintf(fp, "PollutionDetectThreshold=60\n");
	fprintf(fp, "PollutionClearThreshold=40\n");
	fprintf(fp, "InterferenceAgeLimit=15\n");
	fprintf(fp, "IASLowRSSIThreshold=12\n");
	fprintf(fp, "IASMaxRateFactor=88\n");
	fprintf(fp, "IASMinDeltaPackets=10\n");
	fprintf(fp, "IASMinDeltaBytes=2000\n");

	fprintf(fp, "[STEEREXEC]\n");
	fprintf(fp, "SteeringProhibitTime=300\n");
	fprintf(fp, "TSteering=15\n");
	fprintf(fp, "InitialAuthRejCoalesceTime=2\n");
	fprintf(fp, "AuthRejMax=3\n");
	fprintf(fp, "SteeringUnfriendlyTime=600\n");
	fprintf(fp, "MaxSteeringUnfriendly=604800\n");
	fprintf(fp, "LowRSSIXingThreshold_W2=10\n");
	fprintf(fp, "LowRSSIXingThreshold_W5=10\n");
	fprintf(fp, "TargetLowRSSIThreshold_W2=5\n");
	fprintf(fp, "TargetLowRSSIThreshold_W5=15\n");
	fprintf(fp, "BlacklistTime=900\n");
	fprintf(fp, "BTMResponseTime=10\n");
	fprintf(fp, "BTMAssociationTime=6\n");
	fprintf(fp, "BTMAlsoBlacklist=1\n");
	fprintf(fp, "BTMUnfriendlyTime=600\n");
	fprintf(fp, "BTMSteeringProhibitShortTime=30\n");
	fprintf(fp, "MaxBTMUnfriendly=86400\n");
	fprintf(fp, "MaxBTMActiveUnfriendly=604800\n");
	fprintf(fp, "AgeLimit=5\n");
	fprintf(fp, "MinRSSIBestEffort=12\n");
	fprintf(fp, "IASUseBestEffort=0\n");
	fprintf(fp, "StartInBTMActiveState=0\n");

	fprintf(fp, "[STEERALG]\n");
	fprintf(fp, "InactRSSIXingThreshold_W2=5\n");
	fprintf(fp, "InactRSSIXingThreshold_W5=20\n");
	fprintf(fp, "HighTxRateXingThreshold=50000\n");
	fprintf(fp, "HighRateRSSIXingThreshold=30\n");
	fprintf(fp, "LowTxRateXingThreshold=6000\n");
	fprintf(fp, "LowRateRSSIXingThreshold=0\n");
	fprintf(fp, "MinTxRateIncreaseThreshold=53\n");
	fprintf(fp, "AgeLimit=5\n");
	fprintf(fp, "PHYBasedPrioritization=0\n");
	fprintf(fp, "RSSISafetyThreshold=20\n");
	fprintf(fp, "MaxSteeringTargetCount=1\n");
	fprintf(fp, "APSteerToLeafMinRSSIIncThreshold=10\n");
	fprintf(fp, "DownlinkRSSIThreshold_W5=-65\n");

	fprintf(fp, "[DIAGLOG]\n");
	fprintf(fp, "EnableLog=0\n");
	fprintf(fp, "LogServerIP=192.168.1.10\n");
	fprintf(fp, "LogServerPort=7788\n");
	fprintf(fp, "LogLevelWlanIF=2\n");
	fprintf(fp, "LogLevelBandMon=2\n");
	fprintf(fp, "LogLevelStaDB=2\n");
	fprintf(fp, "LogLevelSteerExec=2\n");
	fprintf(fp, "LogLevelStaMon=2\n");
	fprintf(fp, "LogLevelEstimator=2\n");
	fprintf(fp, "LogLevelDiagLog=2\n");
	
	fprintf(fp, "[PERSIST]\n");
 	fprintf(fp, "PersistPeriod=3600\n");
	fclose(fp);

}


int wsplcd_exec=1;
void wsplcd_enable(void)
{
	pid_t pid;
	char *wsplcd[]={"wsplcd","-c",WSPLCD_CONF,NULL};
	if(!wsplcd_exec)
	{
		_dprintf("WSPLCD: skip the start event\n");		
		return;
	}

        //run wsplcd
	doSystem("touch /var/run/wsplcd.lock");
	sleep(2);
	if(nvram_get_int("hive_dbg"))
		_eval(wsplcd, "| logger -s", 0, &pid);
	else
		_eval(wsplcd, ">>/dev/null", 0, &pid);
}

int hyd_exec=1;
void hyd_stop(void)
{
	char tmp[100];
	time_t now ,hyd_uptime;
	now = uptime();
	hyd_uptime = strtoul(nvram_safe_get("hyd_uptime"), NULL, 10);
	sprintf(tmp, "%lu", uptime());
	nvram_set("hyd_uptime", tmp);
	_dprintf("HYD:now_time=%lu,save_time=%lu\n",now,hyd_uptime);
	if(hyd_uptime==0) //just for first boot
		hyd_exec=1;
	else if ((now - hyd_uptime) <50)
	{
		_dprintf("HYD: skip the stop event\n");
		hyd_exec=0;
		return;
	}

        doSystem("echo 0 >> /proc/sys/net/bridge/bridge-nf-call-custom");
	doSystem("hyctl detach %s",nvram_get("lan_ifname")); 
        sleep(1);
	//kill daemon
 	doSystem("killall -9 hyd");
	doSystem("rm -rf /tmp/hyd.conf");
	hyd_exec=1;
}

void hyd_start(int role)
{	
	if(!hyd_exec)
	{
		_dprintf("HYD: skip the start event\n");
		return;
	}
	gen_hyd_conf(role);
        doSystem("echo 1 >> /proc/sys/net/bridge/bridge-nf-call-custom");
	doSystem("hyctl attach %s",nvram_get("lan_ifname")); //after br0 is up
        sleep(2);
	//run daemon
	if(nvram_get_int("hive_dbg"))
                doSystem("nohup hyd -C /tmp/hyd.conf -d 2>&1 | logger -s &");
	else
 		doSystem("hyd -C /tmp/hyd.conf");
}


void gen_sta_conf(int band)
{
	char *str = NULL;
	FILE *fp4;
	char conf[64],tmp[128];
	char prefix_mssid[] = "wlXXXXXXXXXX_mssid_"; 
	int flag_wep;	

	snprintf(prefix_mssid, sizeof(prefix_mssid), "wl%d_", band);

	flag_wep=0;
	sprintf(conf, "/tmp/wpa_supplicant-sta%d.conf",band);
	if ((fp4 = fopen(conf, "w+")) < 0) 
        	return;
	fprintf(fp4, "ctrl_interface=/var/run/wpa_supplicant-sta%d\n",band);
        fprintf(fp4, "update_config=1\n");
	fprintf(fp4, "network={\n");
	fprintf(fp4, "       ssid=\"%s\"\n",nvram_safe_get(strcat_r(prefix_mssid, "ssid", tmp)));

	if(!hive_daisy_chain)
	{
		str = nvram_safe_get(strcat_r(prefix_mssid, "sta_bssid", tmp));
		if (str && strlen(str))
			fprintf(fp4,"       bssid=%s\n",str);   
	}

	str = nvram_safe_get(strcat_r(prefix_mssid, "auth_mode_x", tmp));
	if (str && strlen(str))
	{
		//open none, fix???
		/*if (!strcmp(str, "open") && nvram_match("wlc_wep", "0"))
		{
			fprintf(fp4,"       key_mgmt=NONE\n");    //open/none	
		}
		else */if (!strcmp(str, "open"))
		{
			flag_wep = 1;
			fprintf(fp4,"       key_mgmt=NONE\n"); //open 
			fprintf(fp4,"       auth_alg=OPEN\n");
		}
		else if (!strcmp(str, "shared"))
		{
			flag_wep = 1;
			fprintf(fp4,"       key_mgmt=NONE\n"); //shared
			fprintf(fp4,"       auth_alg=SHARED\n");
		}      
		else if (!strcmp(str, "psk") || !strcmp(str, "psk2"))
		{
		   	fprintf(fp4,"       key_mgmt=WPA-PSK\n"); 
#if 0
			fprintf(fp4,"       proto=RSN\n"); 
#else
			if (!strcmp(str, "psk"))
			 	fprintf(fp4,"       proto=WPA\n");  //wpapsk
			else
			 	fprintf(fp4,"       proto=RSN\n");  //wpa2psk
#endif
			//EncrypType
			if (nvram_match(nvram_safe_get(strcat_r(prefix_mssid, "crypto", tmp)), "tkip"))
			{
				fprintf(fp4, "       pairwise=TKIP\n"); 
				fprintf(fp4, "       group=TKIP\n");
			}   
			else if (nvram_match(nvram_safe_get(strcat_r(prefix_mssid, "crypto", tmp)), "aes")) 
			{
				fprintf(fp4, "       pairwise=CCMP TKIP\n");
				fprintf(fp4, "       group=CCMP TKIP\n");
			}

			//key
			fprintf(fp4, "       psk=\"%s\"\n",nvram_safe_get(strcat_r(prefix_mssid, "wpa_psk", tmp)));
		}
		else
			fprintf(fp4,"       key_mgmt=NONE\n");    //open/none	
	}
	else
		fprintf(fp4,"       key_mgmt=NONE\n");   //open/none

	//EncrypType
	if (flag_wep)
	{
	/*
		for(p = 1 ; p <= 4; p++)
		{
			if(nvram_get_int("wlc_key")==p)
			{   
			   	if((strlen(nvram_safe_get("wlc_wep_key"))==5)||(strlen(nvram_safe_get("wlc_wep_key"))==13))
				{
					fprintf(fp4, "       wep_tx_keyidx=%d\n",p-1);
				  	fprintf(fp4, "       wep_key%d=\"%s\"\n",p-1,nvram_safe_get("wlc_wep_key"));
				}	
				else if((strlen(nvram_safe_get("wlc_wep_key"))==10)||(strlen(nvram_safe_get("wlc_wep_key"))==26))
				{   
					fprintf(fp4, "       wep_tx_keyidx=%d\n",p-1);
				   	fprintf(fp4, "       wep_key%d=%s\n",p-1,nvram_safe_get("wlc_wep_key"));
				}
				else
				{   
					fprintf(fp4, "       wep_tx_keyidx=%d\n",p-1);
				   	fprintf(fp4, "       wep_key%d=0\n",p-1);
				}	
			}
		} 
	*/ 
	}
	fprintf(fp4, "}\n");
        fclose(fp4);
}
#if 0
void wpa_cli_enable(int band)
{
	FILE *fp;
	char conf[64];
	char pid_file[sizeof("/var/run/hive-staX.pidXXXXXX")];
		
	sprintf(conf, "/tmp/wpa_supplicant-sta%d.conf",band);
	if ((fp = fopen(conf, "w+")) < 0) 
        	return;
	fprintf(fp, "ctrl_interface=/var/run/wpa_supplicant-sta%d\n",band);
        fprintf(fp, "update_config=1\n");
        fclose(fp);
	sprintf(pid_file, "/var/run/hive-%s.pid", get_staifname(band));
	
        eval("/usr/bin/wpa_supplicant", "-B", "-P", pid_file, "-D", (char*) QCA_DRV, "-i",get_staifname(band), "-b", nvram_get("lan_ifname"), "-c", conf);               
	sleep(2);
	doSystem("wpa_cli -p /var/run/wpa_supplicant-sta%d wps_pbc",band);
	_dprintf("=>run wpa_cli\n");
}


void wps_enable(int role)
{
	int i;
	for(i=0;i<2;i++)
	{
		if(role) //range extender
			wpa_cli_enable(i);
		else 	 //central ap
			doSystem("hostapd_cli -i ath%d -p /var/run/hostapd-wifi%d/ wps_pbc &",i,i);
	}
	
}
#endif 


//only for backhaul 2G/5G
void set_wifi_otherband_bssid(int band)
{
	char *temp=NULL,tmp[50],prefix_wl[]="wlxxxx_",cmd[128];
#if defined(MAPAC2200)
		if(band==2)
			return ;
#endif
	snprintf(prefix_wl, sizeof(prefix_wl), "wl%d_", band);
	temp = nvram_safe_get(strcat_r(prefix_wl, "hwaddr", tmp));
	doSystem("nvram set wl%d_part1=`echo %s | sed -e \"s/\\://g\" | cut -b 1-8`",band?1:0,temp);
	doSystem("nvram set wl%d_part2=`echo %s | sed -e \"s/\\://g\" | cut -b 9-12`",band?1:0,temp);
	doSystem("iwpriv ath%d otherband_bssid 0x%s 0x%s",band?0:1,
		 nvram_safe_get(strcat_r(prefix_wl, "part1", tmp)),nvram_safe_get(strcat_r(prefix_wl, "part2", tmp)));
}


#ifdef RTCONFIG_DUAL_BACKHAUL
int short_distance(void)
{
	int dist_2g=255,dist_5g=255;
	dist_2g=wifimon_check_hops(0);	
	dist_5g=wifimon_check_hops(1);
	if(dist_2g < dist_5g)
		return dist_2g;
	else
		return dist_5g;
}
#endif

void set_hops_count(void)
{
#ifdef RTCONFIG_DUAL_BACKHAUL
	distance=short_distance();
#else
	distance=wifimon_check_hops(1);	
#endif
	if(distance<256) //0~255
	{
		doSystem("iwpriv %s set_whc_dist %d", get_wififname(0),distance);
		doSystem("iwpriv %s set_whc_dist %d", get_wififname(1),distance);
#if defined(MAPAC2200)
		doSystem("iwpriv %s set_whc_dist %d", get_wififname(2),distance);
#endif	
	}

	if(distance==255)
	{
		doSystem("iwpriv %s set_whc_ul_rate 0", get_staifname(0));
		doSystem("iwpriv %s set_whc_ul_rate 0", get_staifname(1));
	}

	_dprintf("Distance form sta1 is %d, apply to athx !!\n",distance);
}

//4 vaps: ath0(pre-created) ath1(pre-crated) 
//      : sta0(pre-created) sta1(pre-crated) 
void set_vap(int role, int band)
{

	doSystem("iwpriv wifi%d dbdc_enable 0",band);
	//ath
	doSystem("iwpriv %s rrm 1", get_wififname(band));
	doSystem("iwpriv %s wds 1", get_wififname(band));
	doSystem("iwpriv %s extap 0", get_wififname(band));
	doSystem("iwpriv %s blockdfschan %d", get_wififname(band), block_dfs_enable);
	doSystem("iwpriv %s son 1", get_wififname(band));
	 if(!nvram_get_int("dfs_check_period"))   
		doSystem("iwpriv %s athnewind 1", get_wififname(band));
	if(role) //range extender
		doSystem("iwpriv %s set_whc_dist 255", get_wififname(band));
	else
		doSystem("iwpriv %s set_whc_dist 0", get_wififname(band));
	set_wifi_otherband_bssid(band);

	if(role) //range extender
	{
#if defined(MAPAC2200)
		if(band==2)
			return ;
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
		if(band==0)
			return;
#endif
		doSystem("iwpriv %s wds 1",get_staifname(band));
		doSystem("iwpriv %s set_whc_sfactor %d",get_staifname(band),scaling_factor);
	 	if(!nvram_get_int("dfs_check_period"))   
			doSystem("iwpriv %s athnewind 1",get_staifname(band));
		doSystem("iwpriv %s shortgi 1",get_staifname(band));
		doSystem("iwpriv %s set_whc_dist 255", get_staifname(band));
		//doSystem("iwpriv %s mode auto",get_staifname(band));
	//	if(nvram_get_int("dfs_check_period") && band)
	//		doSystem("iwpriv wifi%d staDFSEn 1",band);
	}
}

//search keyword from wpa_supplicant-sta0.conf/wpa_supplicant-sta1.conf
#define WSC_INFO "/tmp/wsc.result"
int getWscStatus_keywd(int band, char *word, char* res)
{
	char *result, *p;
	char buf[200];
	memset(res,0,sizeof(res));
	unlink(WSC_INFO);
        sprintf(buf, "wpa_cli -i %s -p /var/run/wpa_supplicant-sta%d  status | grep -wr %s >> %s", get_staifname(band),band,word,WSC_INFO);
	doSystem(buf);
        sleep(1);
	if (!f_exists(WSC_INFO) || !(result = file2str(WSC_INFO)))
                        return -1;
	if((p=strstr(result,word)) != NULL)
	{
		strcpy(res,p+strlen(word)+1);
		chomp(res);
		free(result);
		return 0;				
	}	
	free(result);
	return -1;
}


void setWscInfo_enrollee(int band)
{
	FILE *fp;
        char buf[1024];
	char conf[64]="/tmp/wpa_supplicant-staxxxxx.conf";
	char cmp[12],tmp[70];
        int i;        
        char *pt1;
	char str[6][10]={"ssid","psk","proto","key_mgmt","pairwise","auth_alg"};
	char data[6][70];
	int unknown;

	sprintf(conf, "/tmp/wpa_supplicant-sta%d.conf",band);
	if ((fp = fopen(conf, "r")) < 0) 
        	return;

	memset(data,0,sizeof(data));
        memset(buf, 0, sizeof(buf));
	while(fgets(buf,1024,fp)!=NULL)
	{
		for(i=0;i<6;i++)
		{
			memset(cmp,0,sizeof(cmp));
			sprintf(cmp,"%s=",str[i]);
			pt1=strstr(buf,cmp);
			if(pt1)
			{
				strcpy(data[i],pt1+strlen(cmp));
                        	data[i][strlen(data[i])-1] = '\0';
			}
		}
	}
        fclose(fp);
	if(strlen(data[0])) //set ssid
	{
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,data[0]+1,strlen(data[0])-2); //skip " "
		nvram_set("wl0_ssid",tmp);
		nvram_set("wl1_ssid",tmp);
	}

	if(strlen(data[1])) //set psk
	{
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,data[1]+1,strlen(data[1])-2); //skip " "
		nvram_set("wl0_wpa_psk",tmp);
		nvram_set("wl1_wpa_psk",tmp);
	}

	unknown=0;
	if(strlen(data[3])) //set key mgmt, proto, auth_alg
	{
		if(strstr(data[3],"NONE"))
		{
			if(!strlen(data[5]) || strstr(data[5],"OPEN")) //auth_alg
			{
				nvram_set("wl0_auth_mode_x","open");
				nvram_set("wl1_auth_mode_x","open");
			}	  
			else if(strstr(data[5],"SHARED"))
			{
				nvram_set("wl0_auth_mode_x","shared");
				nvram_set("wl1_auth_mode_x","shared");
			}	  
			else
				unknown=1;

			//fix????
			//open-wep key, shared-wep key
		}
		else if(strstr(data[3],"WPA-PSK"))
		{
			if(strstr(data[2],"WPA"))  //proto
			{
				nvram_set("wl0_auth_mode_x","psk");
				nvram_set("wl1_auth_mode_x","psk");
			}
			else if(strstr(data[2],"RSN"))
			{
				nvram_set("wl0_auth_mode_x","psk2");
				nvram_set("wl1_auth_mode_x","psk2");
			}
			else
				unknown=1;	

			if(strlen(data[1])) //set psk
			{
				memset(tmp,0,sizeof(tmp));
				strncpy(tmp,data[1]+1,strlen(data[1])-2); //skip " "
				nvram_set("wl0_wpa_psk",tmp);
				nvram_set("wl1_wpa_psk",tmp);
			}

		}
		else if(strstr(data[3],"WPA-EAP"))
		{
			nvram_set("wl0_auth_mode_x","wpawpa2");
			nvram_set("wl1_auth_mode_x","wpawpa2");
		}
		else //IEEE8021X
			unknown=1;	
	
		if(unknown)
		{
			nvram_set("wl0_auth_mode_x","open");
			nvram_set("wl1_auth_mode_x","open");
		}	
	}



	if(strlen(data[4])) //set pairwise
	{
		if(strstr(data[4],"TKIP") && strstr(data[4],"CCMP"))
		{
			nvram_set("wl0_crypto","tkip+aes");
			nvram_set("wl1_crypto","tkip+aes");
		}
		else if(strstr(data[4],"TKIP"))
		{
			nvram_set("wl0_crypto","tkip");
			nvram_set("wl1_crypto","tkip");
		}
		else if(strstr(data[4],"CCMP"))
		{
			nvram_set("wl0_crypto","aes");
			nvram_set("wl1_crypto","aes");

		}

	}
  	nvram_commit();     
	gen_qca_wifi_cfgs();

	_dprintf("upgrade wsc information\n");
	
}

#if defined(MAPAC2200)
int get_wifi_quality(const char *ifname)
{
	FILE *fp;
	char line[256];
	int ret = -1;
	int len;


	if((len = strlen(ifname)) <= 0)
		return -1;
	if((fp = fopen("/proc/net/wireless", "r")) == NULL)
		return -1;

	while(fgets(line, sizeof(line), fp) != NULL)
	{
		char *p1, *p2;
		int status;
		int quality;
		p1 = strstr(line, ifname);
		if(p1 == NULL || *(p1+len) != ':' || (p1 != line && *(p1-1) != ' '))
			continue;
		p1 = p1 + len + 1;
		while(*p1 == ' ')
			p1++;
		status = strtoul(p1, &p2, 16);
		while(*p2 == ' ')
			p2++;
		quality = strtoul(p2, NULL, 0);
		ret = quality;
		break;
	}
	fclose(fp);
	return ret;
}


void set_5g_antenna(void)
{
	int sum;
	int max_idx, max_sum;
	int i,j;
	int quality;
	int retry;

	struct _gpio_ {
		unsigned char gpio;
		unsigned char value;
	};
	struct _gpio_ gpio_arr[][4] = {
			{{44, 0}, {45, 1}, {46, 0}, {47, 1}},	//DPDT type 0
//			{{44, 1}, {45, 0}, {46, 0}, {47, 1}},	//DPDT type 1
			{{44, 1}, {45, 0}, {46, 1}, {47, 0}},	//DPDT type 2
//			{{44, 0}, {45, 1}, {46, 1}, {47, 0}},	//DPDT type 3
	};


	max_idx = 0;
	max_sum = 0;
	for(i = 0; i < ARRAY_SIZE(gpio_arr); i++)
	{
		for(j = 0; j < ARRAY_SIZE(gpio_arr[0]); j++)
		{
			set_gpio(gpio_arr[i][j].gpio, gpio_arr[i][j].value);
		}
		sleep(2);	//wait for getting the applied link quality

		sum = 0;
		for(retry = 0; retry < 10; retry++)
		{
#define CHK_INTERVAL_MS	400
			if(retry != 0)
				usleep(CHK_INTERVAL_MS * 1000);
			quality = get_wifi_quality(STA_5G);
			sum += quality;
		}
		//cprintf("## i(%d) sum(%04u) max_sum(%04u) max_idx(%d)\n", i, sum, max_sum, max_idx);
		if(sum > max_sum)
		{
			max_sum = sum;
			max_idx = i;
		}
	}

	{
		char ant[32], *p = ant;
		for(j = 0; j < ARRAY_SIZE(gpio_arr[0]); j++)
		{
			set_gpio(gpio_arr[max_idx][j].gpio, gpio_arr[max_idx][j].value);
			p += sprintf(p, "%u", gpio_arr[max_idx][j].value);
		}
		logmessage("dpdt", "antenna set to %d (%d), %d (%s)\n", max_idx, ARRAY_SIZE(gpio_arr), max_sum, ant);
		nvram_set("dpdt_ant", ant);
	}
}

int dpdt_ant_main(int argc, char **argv)
{
	//cprintf("## run %s()\n", __func__);
	while(1)
	{
		if(chk_assoc(STA_5G) > 0)
		{
			set_5g_antenna();
			return 0;
		}
		sleep(5);
	}
	return -1;
}
#endif	/* MAPAC2200 */

//check the status of [sta0/]sta1 connection ,max_sec> 6
//if connect, update [2G/]5G current bssid and save them as wl1_sta_bssid
int check_wsc_enrollee_status(int max_sec)
{
	char res[30],tmp[128];
	int i,j,reduce,flag[2]={0,0};
	
	if(max_sec<6) 
		return -1;
	reduce=0;
	for(i=0;i<(max_sec-reduce);i++)
	{	
		//polling sta0 and sta1
#ifdef RTCONFIG_DUAL_BACKHAUL
		for(j=0;j<2;j++)
#else
		j=1;
#endif
		{
			if(chk_assoc(get_staifname(j))> 0)
			{
				char prefix_mssid[] = "wlXXXXXXXXXX_";
			//	_dprintf("[[STA%d is CONNECTED]]\n",j);
				flag[j]=1;
				getWscStatus_keywd(j,"bssid",res);
				snprintf(prefix_mssid, sizeof(prefix_mssid), "wl%d_", j);
				nvram_set(strcat_r(prefix_mssid, "sta_bssid", tmp),res);
				
				if(i<(max_sec-5) && !reduce)
					reduce=max_sec-5-i; //remain 10 sec
			}
		}
		sleep(2);
#ifdef RTCONFIG_DUAL_BACKHAUL
		if(flag[0] && flag[1])
			return 0;
#endif
	}
#ifdef RTCONFIG_DUAL_BACKHAUL
	_dprintf("[STA0 status:%d, STA1 status:%d]\n",flag[0],flag[1]);
	if(flag[0])
		return 0;
	else if(flag[1])
		return 1;
	else
		return -1;
#else
	_dprintf("[STA1 status:%d]\n",flag[1]);
	if(flag[1])
		return 1;
	else
		return -1;
#endif
}


void wifimon_up(void)
{
	pid_t pid;
	char *wifimon[]={"wifimon_check","20",NULL};
	doSystem("killall -9 wifimon_check");
	_eval(wifimon, ">>/dev/null", 0, &pid);
}

void wsplcd_stop(void)
{
	char tmp[100];
	time_t now,wsplcd_uptime;
	now = uptime();
	wsplcd_uptime = strtoul(nvram_safe_get("wsplcd_uptime"), NULL, 10);
	sprintf(tmp, "%lu", uptime());
	nvram_set("wsplcd_uptime", tmp);
	_dprintf("WSPLCD:now_time=%lu,save_time=%lu\n",now,wsplcd_uptime);

	if(wsplcd_uptime==0) //just for first boot
		wsplcd_exec=1;
	else if ((now - wsplcd_uptime) <50)
	{
		_dprintf("WSPLCD: skip the stop event\n");
		wsplcd_exec=0;
		return;
	}

	doSystem("killall wsplcd");
	doSystem("rm -rf "WSPLCD_CONF);
	doSystem("rm -rf /tmp/wsplcd.apply");
	wsplcd_exec=1;
}

void wpa_supplicant_stop(int band)
{
	char pid_file[128], conf[128];
	sprintf(pid_file, "/var/run/hive-%s.pid", get_staifname(band));
	kill_pidfile_tk(pid_file);
	sprintf(conf, "/tmp/wpa_supplicant-%s.conf", get_staifname(band));
	unlink(conf);
	sleep(1);
        doSystem("ifconfig %s down",get_wififname(band));
}


void wpa_supplicant_start(int band)
{
	char conf[64],pid_file[128];
	gen_sta_conf(band);
	sprintf(pid_file, "/var/run/hive-%s.pid", get_staifname(band));
	sprintf(conf, "/tmp/wpa_supplicant-%s.conf",get_staifname(band));
        eval("/usr/bin/wpa_supplicant", "-B", "-P", pid_file, "-D", (char*) QCA_DRV, "-i",get_staifname(band), "-b", nvram_get("lan_ifname"), "-c", conf);               
	if(band) 
	{
		_dprintf("=> RE: try high quality 5G bssid...\n");
		detect_5gband_bssid(); 

	}

	if(nvram_get_int("dfs_check_period"))
		_dprintf("RE=> DFS, avoid ath1 into cac-time..[wpa_supplicant]!!\n");
	else
	{
		sleep(1);
        	doSystem("ifconfig %s up",get_wififname(band));
	}

}

void update_info(int band)
{
	if(band==-1) //inactive or scanning
		return;
	
        setWscInfo_enrollee(band);
}

//duplicate ssid/passwd/authmode/encrypt only
void duplicate_5g2(void)
{	
	nvram_set("wl2_ssid",nvram_get("wl1_ssid"));
	nvram_set("wl2_wpa_psk",nvram_get("wl1_wpa_psk"));
	nvram_set("wl2_auth_mode_x",nvram_get("wl1_auth_mode_x"));
	nvram_set("wl2_crypto",nvram_get("wl1_crypto"));
	_dprintf("=> duplicate 5G-1 to 5G-2\n");
}

// return 0 : file content is same
// return -1 : readfile error
// return 1 : content is different
extern char *readfile(char *fname,int *fsize);

static int cmp_file(char *fname1, char *fname2)
{
	char *buf1, *buf2;
	int fsize1, fsize2;
	int ret = -1;
	buf1 = readfile(fname1, &fsize1);
	buf2 = readfile(fname2, &fsize2);
	if (buf1 && buf2) {
		if ((fsize1 == fsize2) && memcmp(buf1, buf2, fsize1)==0)
			ret = 0;
		else
			ret = 1;
	}
	if (buf1)
		free(buf1);
	if (buf2)
		free(buf2);
	return ret;
}

// return 0: normal case
// return 1: nothing changed
int start_cap(int c)
{

	if(c==0) //first start, for user
	{
		_dprintf("starting WHC auto-configuration for CAP\n");
		wsplcd_stop();
		//wpa_supplicant_stop(0);
		//wpa_supplicant_stop(1);
		hyd_stop();

		set_vap(0,0); //config for 2G CAP
       	 	set_vap(0,1); //config for 5G CAP
#if defined(MAPAC2200)
       	 	set_vap(0,2); //config for 5G-2 CAP
#endif

		sleep(3);
		doSystem("ifconfig %s up",nvram_get("lan_ifname"));
		doSystem("hyctl attach %s",nvram_get("lan_ifname")); //after br0 is up
	
		gen_wsplcd_conf(0, CMP_CONF, 1);
		cfg_re_syncing=0;
		cfg_changed=0;  
		gen_wsplcd_conf(0, WSPLCD_CONF, 0);
		wsplcd_enable();
		hyd_start(0);
		//wps_enable(0);
	}
	else if(c==1) //config change , for user
	{
		_dprintf("CAP: restart after config change\n");
		gen_wsplcd_conf(0, CMP_CONF_NEW, 1);
		if (cmp_file(CMP_CONF, CMP_CONF_NEW)) {
			unlink(CMP_CONF);
			rename(CMP_CONF_NEW, CMP_CONF);
			cfg_re_syncing=1;
			cfg_changed=1;
			wsplcd_stop();
			gen_wsplcd_conf(0, WSPLCD_CONF, 0);
			wsplcd_enable();
			sleep(6);
			nvram_set("cap_syncing","1");
			lp55xx_leds_proc(LP55XX_GREENERY_LEDS, LP55XX_WIFI_PARAM_SYNC);
		} else
			return 1;
	}
	else if(c==2) //restart, for wsplcd daemon
	{
		_dprintf("CAP: restart wireless\n");
		set_vap(0,0); //config for 2G CAP
       	 	set_vap(0,1); //config for 5G CAP
#if defined(MAPAC2200)
       	 	set_vap(0,2); //config for 5G-2 CAP
		duplicate_5g2();
#endif
		wsplcd_stop();
		gen_wsplcd_conf(0, CMP_CONF, 1);
		cfg_re_syncing=0;
		cfg_changed=0;
		gen_wsplcd_conf(0, WSPLCD_CONF, 0);
		wsplcd_enable();
		nvram_commit();
		gen_qca_wifi_cfgs();
		nvram_set("cap_syncing","0");
		sleep(30); //estimate wifi-restart time
		//_dprintf("=>CAP: config change stop\n");
		lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
	}
	return 0;
}

void start_re(int c)
{
	int i;
	wsplcd_stop();
	if(c==3)
	{
		_dprintf("=>RE : ######wait timeout#####\n");
		cfg_re_syncing=1;
		gen_wsplcd_conf(0, WSPLCD_CONF, 0);
		wsplcd_exec=1;
		wsplcd_enable();
		doSystem("killall -9 wifimon_check");
		check_wsc_enrollee_status(10); //update current pap's bssid
		sleep(self_restart); //estimate waiting time
		doSystem("killall -9 wsplcd");
		doSystem("rm -rf "WSPLCD_CONF);
		doSystem("rm -rf /tmp/wsplcd.apply");
		cfg_re_syncing=0;
	}	
	else
		cfg_re_syncing=0;

#ifdef RTCONFIG_DUAL_BACKHAUL
	wpa_supplicant_stop(0);
#endif
	wpa_supplicant_stop(1);
	hyd_stop();

	_dprintf("[run repacd process with previous configuration]\n");

	//gen_qca_wifi_cfgs();
	set_vap(1,0); //config for 2G range extender
        set_vap(1,1); //config for 5G range extender
#if defined(MAPAC2200)
       	set_vap(1,2); //config for 5G-2 range extender
	if(c==2 || c==3)//only for wsplcd restart
	{
		duplicate_5g2();
  		nvram_commit();     
	}
#endif



#if 0 //for wps test
	{
		wps_enable(1);
		//update info as the same as /lib/wifi/wps-supplicant-update-uci		
		update_info(check_wsc_enrollee_status(20));
	}
#else
	{

		
		gen_qca_wifi_cfgs();

		if(nvram_get_int("dfs_check_period"))
		{
			while(1)
			{
				if(f_exists("/var/run/hostapd_ath1.pid"))
				{
					doSystem("ifconfig ath1 down");
					_dprintf("RE=> DFS, avoid ath1 into cac-time..[start_re]!!\n");
					break;
				}
				sleep(1);
			}
		}

#ifdef RTCONFIG_DUAL_BACKHAUL
		for(i=0;i<2;i++)
#else
		i=1;
#endif
		{

			doSystem("ifconfig %s up",get_staifname(i));
			wpa_supplicant_start(i);
			sleep(1);
		}

	}
#endif
	for(i=0;i<2;i++)  
        	doSystem("ifconfig %s down",get_wififname(i));

	sleep(5);
	doSystem("ifconfig %s up",nvram_get("lan_ifname"));
	doSystem("hyctl attach %s",nvram_get("lan_ifname")); //after br0 is up

	if(c==2 || c==3) //only for wsplcd restart
	{
		nvram_set("re_syncing","0");
		wpa_cli_set_bssid(1,nvram_get("wl1_sta_bssid"));
		sleep(10); //estimate
#ifdef RTCONFIG_DUAL_BACKHAUL
		wpa_cli_set_bssid(0,nvram_get("wl0_sta_bssid"));
#endif
		//_dprintf("RE: config change stop\n");
		lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
		check_wsc_enrollee_status(20);
	}
	//gen config and run daemon
	gen_wsplcd_conf(1, WSPLCD_CONF, 0);
	wsplcd_enable();
	hyd_start(1);
	wifimon_up();

}


void start_hyfi(void)
{
	int i,role;
	role=get_role();
	if(role==0)
	{
		set_vap(0,0); //config for 2G CAP
       	 	set_vap(0,1); //config for 5G CAP
#if defined(MAPAC2200)
        	set_vap(0,2); //config for 5G-2 CAP
#endif
		hyd_start(0);
		gen_wsplcd_conf(0, CMP_CONF, 1);
		gen_wsplcd_conf(0, WSPLCD_CONF, 0);
	}
	else if(role==1)
	{
#ifdef RTCONFIG_DUAL_BACKHAUL
		for(i=0;i<2;i++)
			wpa_supplicant_start(i);
#else
		wpa_supplicant_start(1);
#endif
		set_vap(1,0); //config for 2G range extender
        	set_vap(1,1); //config for 5G range extender
#if defined(MAPAC2200)
      		set_vap(1,2); //config for 5G-2 range extender
#endif
		hyd_start(1);
		gen_wsplcd_conf(1, WSPLCD_CONF, 0);
		//wifimon_up();
	}
	else
		_dprintf("error mode!!\n");
	wsplcd_enable();
}

void stop_hyfi(void)
{
#ifdef RTCONFIG_DUAL_BACKHAUL
	int i;
	for(i=0;i<2;i++)
		wpa_supplicant_stop(i);
#else
	wpa_supplicant_stop(1);
#endif
	hyd_stop();
	wsplcd_stop();
}


#ifdef RTCONFIG_ETHBACKHAUL
int eth_backl=0;
int lldpcli_det(void)
{
	char buf[2048];
        FILE *fp;
        int len;
        char *pt1,*pt2;
	char bssid[18];
        sprintf(buf, "lldpcli show neighbors");
	
        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
			if(get_role()) //RE
                        	pt1 = strstr(buf, "MAP-CAP"); //get 
			else
                        	pt1 = strstr(buf, "MAP-RE"); //get 
                        if (pt1)
                               return 1;
		}
	}
	return 0;
}

void wpacli_reset()
{
	doSystem("wpa_cli -p /var/run/wpa_supplicant-sta1 disable_network 0");

	if(strlen(nvram_safe_get("wl1_sta_bssid")) && strcmp(nvram_safe_get("wl1_sta_bssid"),"00:00:00:00:00:00")!=0)
	{
		_dprintf("==> wpacli set %s\n",nvram_safe_get("wl1_sta_bssid"));
		doSystem("wpa_cli -p /var/run/wpa_supplicant-sta1 set_network 0 bssid %s",nvram_safe_get("wl1_sta_bssid"));
	}
	doSystem("wpa_cli -p /var/run/wpa_supplicant-sta1 enable_network 0");
}

void start_eth(int c)
{
        char ifname[32];
        char *next;
	int i=0;
	if(c) //eth up
	{
		_dprintf("=> RE: eth backhaul up\n");
		wpacli_reset();
		sleep(eth_down_time); //for green led
	}
	else
	{
		_dprintf("=> RE: eth backhaul down\n");
		ifconfig("eth1", 0, NULL, NULL);
		doSystem("killall -9 hyd");
		doSystem("hyd -C /tmp/hyd.conf");
		wpacli_reset();

		if (pids("udhcpc"))
                {
                        killall("udhcpc", SIGUSR2);
                        killall("udhcpc", SIGTERM);
                        unlink("/tmp/udhcpc_lan");
                }
                
		char *dhcp_argv[] = { "udhcpc",
                                        "-i", "br0",
                                        "-p", "/var/run/udhcpc_lan.pid",
                                        "-s", "/tmp/udhcpc_lan",
                                        NULL };
                pid_t pid;

                symlink("/sbin/rc", "/tmp/udhcpc_lan");
                _eval(dhcp_argv, NULL, 0, &pid);

		sleep(eth_down_time);
		ifconfig("eth1", IFUP, NULL, NULL);
		foreach(ifname, nvram_safe_get("wl_ifnames"), next) 
		{
			ifconfig(ifname, 0, NULL, NULL);
			sleep(1);
			ifconfig(ifname, IFUP, NULL, NULL);
		}
	}
}
#endif

//for daisy chain
int sta_is_assoc(int band)
{
        char buf[1024];
        FILE *fp;
        int len,ch;
        char *pt1,*temp=NULL,*iface=NULL;
	char tmp[128],prefix_wl[]="wlxxxx_",tmpch[5];

	snprintf(prefix_wl, sizeof(prefix_wl), "wl%d_", band);
	temp = nvram_safe_get(strcat_r(prefix_wl, "channel", tmp));
	ch=atoi(temp);
	if(ch!=0) 
		if((!band && ch>14) || (band && ch<36))
			ch=0xff;
	

        sprintf(buf, "iwconfig %s", get_staifname(band));

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "Not-Associated");
                        if (pt1)
                                return 0; //not assoc
			else 
			{
				if(get_role()) //RE
				{
					if(ch!=0xff && ch!=get_ch(get_freq(band)))
					{
						iface = nvram_safe_get(strcat_r(prefix_wl, "ifname", tmp));
						 if(!nvram_get_int("dfs_check_period"))   
							doSystem("iwconfig %s channel %d",iface,get_ch(get_freq(band)));
						_dprintf("=> RE: channel conflict on ath%d/sta%d. fix it.\n",band,band);
						sprintf(tmp,"wl%d_channel",band);
						sprintf(tmpch,"%d",get_ch(get_freq(band)));
						nvram_set(tmp,tmpch);
					}
				}
				return 1;
			}
                }
        }
        return 1;
}

//determine the distance (in terms of hops) from the CAP.
//if daisy chain, calculate 5G only.
int wifimon_check_hops(int band)
{
        char buf[1024];
        FILE *fp;
        int len;
        char *pt1,*dist=NULL;
	
        sprintf(buf, "iwpriv %s get_whc_dist", get_staifname(band));

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "get_whc_dist:");
			if(pt1)
			{
                                dist = pt1 + strlen("get_whc_dist:");
                                chomp(dist);
			}
                }
        }
	if(dbg_m)
		_dprintf("=> RE: %s's distance from CAP is=%d\n",get_staifname(band),atoi(dist));
	
        return atoi(dist);
}

//return 0 if current bssid is the same with new bssid 
int diff_current_bssid(int band,char *new_mac)
{
  	char *now_mac = NULL;
        char tmp[128];
	int i,diff;
        char prefix_mssid[] = "wlXXXXXXXXXX_mssid_";
	snprintf(prefix_mssid, sizeof(prefix_mssid), "wl%d_", band);
	now_mac = nvram_safe_get(strcat_r(prefix_mssid, "sta_bssid", tmp));
	if (now_mac && strlen(now_mac))
	{
		
		for(i=0;i<17;i++)
		{
			diff=abs((int)(*(now_mac+i)-*(new_mac+i)));
			if(diff==0 || diff==32)
				continue;
			else
			{
				if(dbg_m)
					_dprintf("=> RE: change %s's bssid from %s to %s\n",band?"5G":"2G",now_mac,new_mac);
				return 1;
			}
				
		}
		if(dbg_m)
			_dprintf("=> RE: current serving-ap and the best serving-ap are the same, no restart required.\n");
		return 0;
	}
	else		
		_dprintf("=> RE: can not get %s's current pap bssid!!\n",band?"5G":"2G");

 	return 1;
}

//set new bssid by wpa_cli tools.
void wpa_cli_set_bssid(int band,char *mac)
{
	if(diff_current_bssid(band,mac))
	{
		//Restart the network with configured BSSID
		doSystem("wpa_cli -p /var/run/wpa_supplicant-sta%d disable_network 0",band);
		doSystem("wpa_cli -p /var/run/wpa_supplicant-sta%d set_network 0 bssid %s",band,mac);
		doSystem("wpa_cli -p /var/run/wpa_supplicant-sta%d enable_network 0",band);
		sleep(6);
		_dprintf("=> RE: wpacli set sta%d's bssid as %s\n",band,mac);
	}
}


int find_pap_bssid(int band,int config)
{
	char tmp[20];
	doSystem("iwpriv %s sendprobereq 1",get_staifname(band));
	nvram_set("sta_bssid_tmp","00:00:00:00:00:00"); //reset 
	if(band)
		nvram_set("sta_bssid_bk","00:00:00:00:00:00"); //reset 
	unlink(AP_SCAN_FILE);
	getSiteSurvey(band,AP_SCAN_FILE); //scan band and update sta_bssid_tmp
	_dprintf("=>sta_bssid_tmp=%s\n",nvram_safe_get("sta_bssid_tmp"));
	if(strcmp(nvram_safe_get("sta_bssid_tmp"),"00:00:00:00:00:00")!=0)
	{
		_dprintf("=> get pap's %s bssid\n",band?"5G":"2G");
		if(config)
		{
			wpa_cli_set_bssid(band,nvram_safe_get("sta_bssid_tmp"));
			sprintf(tmp,"wl%d_sta_bssid",band);
			nvram_set(tmp,nvram_safe_get("sta_bssid_tmp"));
			if(band)
				nvram_set("wl1_sig",nvram_safe_get("sig_tmp"));
		}
	}
	return 0;
}

void detect_5gband_bssid(void)
{	
	find_pap_bssid(1,1);
}

#ifdef RTCONFIG_DUAL_BACKHAUL
int find_pap_2g_bssid_and_config(void)
{
        char buf[1024];
        FILE *fp;
        int len,i;
        char *pt1,*pt2=NULL;
	char mac[17];

        sprintf(buf, "iwpriv sta1 g_whc_ob_bssid");

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "g_whc_ob_bssid:");
			if(pt1)
			{
                                pt2= pt1 + strlen("g_whc_ob_bssid:");
                                chomp(pt2);
			}
                }
        }

	memset(mac,0,sizeof(mac));
	if(strcmp(pt2,"000000000000"))
	{
		for(i=0;i<6;i++)
		{
			if(i==0)
				sprintf(mac,"%c%c",*(pt2+2*i),*(pt2+2*i+1));
			else
				sprintf(mac,"%s:%c%c",mac,*(pt2+2*i),*(pt2+2*i+1));
		}
	}
	else
	{
		_dprintf("=> RE: can not resolve 2G bssid ! disable 2g..\n");
		doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 disable_network 0");
		disable_2g=1;
		return 0;
	}	

	_dprintf("=> RE: get 2G bssid=%s from sta1\n",mac);
	wpa_cli_set_bssid(0,mac);
	disable_2g=0;
	nvram_set("wl0_sta_bssid",mac);
        return 1;

}
#endif

//get CAP's 2G bssid and try configuring 2.4G backhaul bssid.
/*
int config_cap_bssid(int band)
{
        char buf[1024];
        FILE *fp;
        int len,i;
        char *pt1,*pt2=NULL;
	char mac[17];
        sprintf(buf, "iwpriv %s g_whc_cap_bssid", get_staifname(band));

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "g_whc_cap_bssid:");
			if(pt1)
			{
                                pt2= pt1 + strlen("g_whc_cap_bssid:");
                                chomp(pt2);
			}
                }
        }

	memset(mac,0,sizeof(mac));
	if(strcmp(pt2,"000000000000"))
	{
		for(i=0;i<6;i++)
		{
			if(i==0)
				sprintf(mac,"%c%c",*(pt2+2*i),*(pt2+2*i+1));
			else
				sprintf(mac,"%s:%c%c",mac,*(pt2+2*i),*(pt2+2*i+1));
		}
	}
	else
	{
		if(!band)
		{
			_dprintf("=> RE: can not resolve CAP's 2G bssid ! disable 2g..\n");
			doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 disable_network 0");
#ifdef RTCONFIG_DUAL_BACKHAUL
			disable_2g=1;
#endif
		}
		return -1;
	}	

	_dprintf("=> %s's get_whc_cap_mac=%s\n",get_staifname(band),mac);
	wpa_cli_set_bssid(band,mac);
        return 1;

}
*/

void reset_loop(void) //only for 5G
{
	int i;
	char loop_mac[18],new_mac1[18],new_mac2[18];
	strcpy(loop_mac,nvram_safe_get("wl1_sta_bssid"));
	detect_5gband_bssid();
	strcpy(new_mac1,nvram_safe_get("sta_bssid_tmp"));
	strcpy(new_mac2,nvram_safe_get("sta_bssid_bk"));
	doSystem("ifconfig %s down",get_wififname(1));
	_dprintf("[new_mac1=%s] [new_mac2=%s]\n",new_mac1,new_mac2);
	if(strcmp(new_mac1,"00:00:00:00:00:00")!=0)
	{
		for(i=0;i<17;i++)
		{
			if(abs(*(loop_mac+i)-*(new_mac1+i))!=0 && abs(*(loop_mac+i)-*(new_mac1+i))!=32) 
			{	
				nvram_set("wl1_sta_bssid",new_mac1);
				wpa_cli_set_bssid(1,nvram_safe_get("wl1_sta_bssid"));
				_dprintf("=> RE:5G old bssid=%s,use new bssid=%s\n",loop_mac,nvram_safe_get("wl1_sta_bssid"));
				doSystem("ifconfig %s up",get_wififname(1));
				return ;
			}
		}
	}

	if(strcmp(new_mac2,"00:00:00:00:00:00")!=0)
	{
		for(i=0;i<17;i++)
		{
			if(abs(*(loop_mac+i)-*(new_mac2+i))!=0 && abs(*(loop_mac+i)-*(new_mac2+i))!=32) 
			{	
				nvram_set("wl1_sta_bssid",new_mac2);
				wpa_cli_set_bssid(1,nvram_safe_get("wl1_sta_bssid"));
				_dprintf("=> RE:5G old bssid=%s,use new bssid=%s\n",loop_mac,nvram_safe_get("wl1_sta_bssid"));
				doSystem("ifconfig %s up",get_wififname(1));
				return ;
			}
		}
	}
	
	_dprintf("=> RE: 5G get new bssid fail!!\n");
	
}

//get best serving ap
//if daisy chain, 5G only
int find_best_bssid(int band)
{
        char buf[1024];
        FILE *fp;
        int len,i;
        char *pt1,*pt2=NULL;
	char mac[17];
        sprintf(buf, "iwpriv %s get_whc_bssid", get_staifname(band));

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "get_whc_bssid:");
			if(pt1)
			{
                                pt2= pt1 + strlen("get_whc_bssid:");
                                chomp(pt2);
			}
                }
        }

	memset(mac,0,sizeof(mac));
	if(strcmp(pt2,"000000000000"))
	{
		for(i=0;i<6;i++)
		{
			if(i==0)
				sprintf(mac,"%c%c",*(pt2+2*i),*(pt2+2*i+1));
			else
				sprintf(mac,"%s:%c%c",mac,*(pt2+2*i),*(pt2+2*i+1));
		}
	}
	else
	{
		if(dbg_m)
			_dprintf("=> RE: can not resolve best bssid for %s!!\n",band?"5G":"2G");
		return -1;
	}	

	if(dbg_m)	
		_dprintf("=> RE: find best ap %s for %s !\n",mac,get_staifname(band));
	wpa_cli_set_bssid(band,mac);
        return 1;
}

//return 1 if the association is stable
int wait_assoc_stable(int band, int link_check_count)
{
	int i;
	for(i=0;i<link_check_count;i++)
	{
		sleep(2);
		if(sta_is_assoc(band))
		{
			if(dbg_m)
				_dprintf("=> %s is assoc ready\n",band?"5G":"2G");
			return 1;
		}
	}
	if(dbg_m)
		_dprintf("=> %s is assoc unstable\n",band?"5G":"2G");
	return 0;
}

int pap_db_mac(void) //check if the 2G/5G connection to the same pap
{
	int j,macd1=0,macd2=0,cmp=0;
	char mac1[18]="",mac2[18]="",tm[5]="";
	if(!strlen(nvram_safe_get("wl0_sta_bssid")) || !strlen(nvram_safe_get("wl1_sta_bssid")))
		return 0;

	memset(mac1,0,sizeof(mac1));
        strcpy(mac1,nvram_safe_get("wl0_sta_bssid"));
	memset(mac2,0,sizeof(mac2));
        strcpy(mac2,nvram_safe_get("wl1_sta_bssid"));

	for(j=0;j<15;j++)
	{
		cmp=abs(*(mac1+j)-*(mac2+j));
		if(cmp!=0 && cmp!=32) //upper or lower 
			return 0;
	}
#if 0
	//last macfield
	memset(tm,0,sizeof(tm));
	sprintf(tm,"%c%c",*(mac1+15),*(mac1+16));
	sscanf(tm,"%x",&macd1);
	memset(tm,0,sizeof(tm));
	sprintf(tm,"%c%c",*(mac2+15),*(mac2+16));
	sscanf(tm,"%x",&macd2);

	if(abs(macd1-macd2)==2 || abs(macd1-macd2)==4)
		return 1;
	else	
		return 0;	
#else
	cmp=abs(*(mac1+15)-*(mac2+15));
	if(cmp!=0 && cmp!=32) //upper or lower 
		return 0;
	else
		return 1;
#endif
}

void force_down_2g(void)
{
#ifdef RTCONFIG_DUAL_BACKHAUL
	_dprintf("=> RE: temporarily disable 2G...\n");
	disable_2g=1;
	resolving=0;
	nvram_unset("wl0_sta_bssid");
	doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 disable_network 0");
#endif
}

void check_iface_all(void)
{
        char buf[1024],ifname[32];
        FILE *fp;
        int len;
        char *pt1,*next;
	_dprintf("=>RE: check all iface ...\n");
	foreach(ifname, nvram_safe_get("wl_ifnames"), next) 
	{
        	sprintf(buf, "iwconfig %s", ifname);
        	fp = popen(buf, "r");
        	if (fp) {
                	memset(buf, 0, sizeof(buf));
                	len = fread(buf, 1, sizeof(buf), fp);
               		pclose(fp);
                	if (len > 1) {
                       	 	buf[len-1] = '\0';
                        	pt1 = strstr(buf, "Not-Associated");
                        	if (pt1)
				{
					//not assoc
					_dprintf("=>RE: find %s disable, enable it\n",ifname);
        				doSystem("ifconfig %s down",ifname);
					sleep(5);
        				doSystem("ifconfig %s up",ifname);
				}
			}
                }
        }
}


//only RE
#if defined(RTCONFIG_CSU2_2)
#ifdef DETECT_5G		
int change_5g=0;
int detect_5g=1;
#endif
int wifimon_check_assoc(int retry)
{
#ifndef RTCONFIG_DUAL_BACKHAUL
	int ch_2g;
#endif
	if(sta_is_assoc(1))
	{
		check_wsc_enrollee_status(7); //update current pap's bssid
		if(detect_loop())
		{
			_dprintf("=> RE: detect loop on 5G, reset!\n");
			reset_loop();
			force_down_2g();
			return 2;
		}
#ifdef  DETECT_5G		
		if(getStaXRssi(1)<-80) //distance/sig level/link quality: 25m/-80dbm/30, 12m/-65dbm/80
		{
			if(change_5g>2 && detect_5g)
			{
				find_pap_bssid(1,0);
				if(nvram_get_int("siglv_tmp")<60)
				{
			
			_dprintf("=> RE: find high-rssi pap bssid ...\n");
					if(strcmp(nvram_safe_get("sta_bssid_tmp"),nvram_safe_get("wl1_sta_bssid")))
					{
						if(strcmp(nvram_safe_get("sta_bssid_tmp"),"00:00:00:00:00:00"))
						{
					    _dprintf("=> RE: o_quality=%d, n_quality=-%d,detect better pap ... restart\n",getStaXRssi(1),nvram_get_int("siglv_tmp"));
								wpa_cli_set_bssid(1,nvram_safe_get("sta_bssid_tmp"));
						}
						force_down_2g();
						change_5g=0;
						return 1;
					}
				}
				else
				{
					_dprintf("=>RE: no other pap is found\n");
					detect_5g=0;
				}
			}
			else
			{
				if(detect_5g==0)
				 	change_5g=0;	
				else
					change_5g++;
			}
		}
#endif

#ifdef RTCONFIG_DUAL_BACKHAUL
		if(!resolving)
		{
			_dprintf("=> RE: 5G assoc ok. try to resolve 2G\n");
			if(!find_pap_2g_bssid_and_config())//find 2G bssid,if y, config it
				return 1;		
			else
				resolving=1;
		}
		else
		{
			if(retry!=0xff && !sta_is_assoc(0))  //try resolve 2G until 2G is down
			{
				resolving=0;
				nvram_unset("wl0_sta_bssid");
				_dprintf("=> RE: still resolve 2G...\n");
			}
			else
				_dprintf("=> RE: do not resolve 2G\n");

		}

		if(sta_is_assoc(0))
		{
			if(!pap_db_mac())
			{
				force_down_2g();
				_dprintf("=> RE: detcet diff bssid between 2G and 5G\n");
				return 2;
			}
		}
#else
		ch_2g=nvram_get_int("wl0_channel");
                if(ch_2g!=0 && ch_2g!=get_ch(get_freq(0)))
                {
                        _dprintf("=> RE: sync ath0 ch!!\n");
                        doSystem("iwconfig %s channel %d",nvram_get("wl0_ifname"),ch_2g);
                }

#endif
		check_iface_all();	
		_dprintf("=> RE: done\n");
		return 0;
	}	
	return 2;	
}

#else //NOT CSU2.2
#ifdef RTCONFIG_DUAL_BACKHAUL
int once=0;
#endif
//only RE
int wifimon_check_assoc(void)
{
	int dist;
#ifdef RTCONFIG_DUAL_BACKHAUL
	if(sta_is_assoc(0) && sta_is_assoc(1))
#else
	int ch_2g;
	if(sta_is_assoc(1))
#endif
	{
#ifdef RTCONFIG_DUAL_BACKHAUL
		//_dprintf("=> RE: 2G is assoc and 5G is assoc\n");
#endif
		check_wsc_enrollee_status(7); //update current pap's bssid
#ifdef RTCONFIG_DUAL_BACKHAUL
		if(sta_is_assoc(1) && detect_loop())
#else
		if(detect_loop())
#endif
		{
			_dprintf("=> RE: detect loop on 5G, reset!\n");
			reset_loop();
			return 2;
		}
#ifdef RTCONFIG_DUAL_BACKHAUL
		_dprintf("=> RE: check pap's 2G mac and pap's 5G's mac.\n");
		if(!pap_db_mac())
			once=0;
		else
			once=1;
		dist=wifimon_check_hops(1);
		if(dist==1) //5G assoc at CAP
		{
			if(once)
				return 0;
			if(!disable_2g)
			{

				_dprintf("=>5G on CAP, try to redirect 2G on CAP\n");
				config_cap_bssid(0); //get 2.4G CAP's bssid and restart it.
				wait_assoc_stable(0,10);
			}
			else
				_dprintf("=>5G on CAP, ignore 2G (disable 2g)\n");
			once=1;
			return 0;
		}
		else //5G assoc at RE , dist>1
		{
			if(once)
				return 1;
			if(!disable_2g)
			{
				_dprintf("=>5G on RE, try to redirect 2G on RE\n");
				find_pap_2gband_bssid(); //find pap's 2G bssid and restart it.
				wait_assoc_stable(0,10);
			}
			else
				_dprintf("=>5G on CAP, ignore 2G (disable 2g)\n");
			once=1;
			return 1;
		}
#else
		ch_2g=nvram_get_int("wl0_channel");
		if(ch_2g!=0 && ch_2g!=get_ch(get_freq(0)))
		{
			_dprintf("=> RE: sync ath0 ch!!\n");
                   	doSystem("iwconfig %s channel %d",nvram_get("wl0_ifname"),ch_2g);
                }
		check_iface_all();      

		return 0;
#endif
	}	
	else
	{
#ifdef RTCONFIG_DUAL_BACKHAUL
		_dprintf("=> RE: 2G not-assoc or 5G not-assoc");
		once=0;
#else
		_dprintf("=> RE: 5G not-assoc");
#endif
	}
	return 2;	
}

#endif

int get_cac_state(void)
{
        char buf[1024];
        FILE *fp;
        int len;
        char *pt1,*cac=NULL;
       
        sprintf(buf, "iwpriv ath1 get_cac_state");

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "get_cac_state:");
                        if(pt1)
                        {
                                cac= pt1 + strlen("get_cac_state:");
                                chomp(cac);
                        }
                }
        }
        return atoi(cac);
}

//measure the rate to the serving AP.
//get the phyrate of the associated sta interface and calculate the min and max threshold rates.
//only the 5 GHz link is measured for daisy chain
int monitor_rate(int band)
{
        char buf[1024];
        FILE *fp;
        int len;
        char *pt1,*rate=NULL;
	int phyrate,whc_rate;
        sprintf(buf, "iwpriv %s get_whc_rate", get_staifname(band));

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, "get_whc_rate:");
			if(pt1)
			{
                                rate= pt1 + strlen("get_whc_rate:");
                                chomp(rate);
			}
                }
        }
	whc_rate=atoi(rate);
	phyrate=get_phyrate(band);
	if(dbg_m)
		_dprintf("=> %s: whc_rate=%d (phyrate=%d, max-threshold=%d min-threshold=%d)\n", \
			get_staifname(band),whc_rate,phyrate,(int)(percent_rate_max*phyrate/100.0),(int)(percent_rate_min*phyrate/100.0));

	if(phyrate)
	{
		if(dbg_m)
		{
			if(whc_rate > (int)(percent_rate_max*phyrate/100.0))
				_dprintf("=> RE: Move farther\n");
			else if(whc_rate < (int)(percent_rate_min*phyrate/100.0))
				_dprintf("=> RE: Move closer\n");
			else
				return 0;
		}
		else
		{
			if((whc_rate< (int)(percent_rate_max*phyrate/100.0)) && (whc_rate>(int)(percent_rate_min*phyrate/100.0)))
				return 0;
		}
	}

        return 1; //find the best serving AP

}

int detect_loop(void) //only for 5G
{
	char buf[2048];
        FILE *fp;
        int len;
        char *pt1,*pt2;
	char bssid[18];
        sprintf(buf, "wlanconfig %s list",get_wififname(1));
	
	pt2=nvram_safe_get("wl1_sta_bssid");

	if(strlen(pt2))
	{
             strncpy(bssid,pt2+2,15);
	     chomp(bssid);
	}
	else
		return 0;

        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
                        pt1 = strstr(buf, bssid); //compare with ':xx:xx:xx:xx:xx' 
                        if (pt1)
                               return 1;
		}
	}

	return 0;
}

#if defined(RTCONFIG_CSU2_2)
void start_wifimon_check(int delay)
{
	int i,state, assoc_timeout;
	int restart_process=0;
	int retry;
	doSystem("iwpriv wifi0 ignore_dbg 1");

	if(nvram_get_int("wifimon_dbg"))
		dbg_m=1;
	else 
		dbg_m=0;

	if(nvram_get_int("dfs_check_period"))
		check_period=60;
	else
		check_period=30;

	retry=0;
	assoc_timeout=-1; //unknown
	//temporarily disable 2G
	force_down_2g();

	if(dbg_m)
		_dprintf("=> wifi monitor check: delay %d sec ...\n",delay);
	sleep(delay);


	if(nvram_get_int("dfs_check_period"))
	{
		_dprintf("RE=> DFS, avoid ath1 into cac-time..[wifimon_check]!!\n");
	}
	else
	{
		for(i=0;i<2;i++)
		{
	       	 	doSystem("ifconfig %s down",get_wififname(i));
			sleep(1);
	       	 	doSystem("ifconfig %s up",get_wififname(i));
		}
	}
	/*assoc_timeout: 
			eth-backhaul:-2(eth-backhaul)
			wifi-backhaul:-1(init or unknown), 0(assoc), >0(not-assoc)
	*/
	while(1)
	{
		if(dbg_m)
			_dprintf("=> RE: wifi monitor check ...\n");
		state=wifimon_check_assoc(retry);
		switch(state)
		{
			case 0:
			case 1:
				if(assoc_timeout==-1 || assoc_timeout>=1 
#ifdef RTCONFIG_DUAL_BACKHAUL
				|| distance != short_distance())
#else
				|| distance != wifimon_check_hops(1))   
#endif
					set_hops_count();
				assoc_timeout=0;
				break;
			case 2: //5G not-assoc, 2G unknown
				if(assoc_timeout==-1 || assoc_timeout==0) 
					set_hops_count();
				if(assoc_timeout<0)  //fix assoc value if unknown or eth-backhaul case
					assoc_timeout=0;
				assoc_timeout++;
#ifdef RTCONFIG_ETHBACKHAUL
				_dprintf("=> RE: eth_backl=%d\n",nvram_get_int("eth_backl"));
				if(nvram_get_int("eth_detect_proc") || nvram_get_int("eth_backl")) //eth backhaul process, ignore wifimon
					assoc_timeout=-2;
#endif
				break;
			default:
				break;
		}
	
		if(nvram_get_int("dfs_check_period")&& assoc_timeout!=0) //not-assoc in DFS
		{
			sleep(check_period-30);
			if(sta_is_assoc(1))
			{
				_dprintf("=> RE: avoid long waiting-time, bring ath1 up!!!\n");
				check_iface_all(); //avoid long waiting-time for bring-up ath1    
#ifdef RTCONFIG_DUAL_BACKHAUL
				if( distance != short_distance())
#else
				if(distance != wifimon_check_hops(1))   
#endif
					set_hops_count();

			}  
			sleep(30);
		}
		else
			sleep(check_period);
		
		if(restart_process)
		{
			_dprintf("=> RE:restart hyd/wsplcd/wifimon process after reset 2G or 5G\n");
			wsplcd_stop();
			hyd_stop();
			gen_wsplcd_conf(1, WSPLCD_CONF, 0);
                	wsplcd_enable();
			hyd_start(1);
		}

		if(assoc_timeout==0)
		{
			restart_process=0;
		}
		else if(assoc_timeout>=2)  //2G or 5G not assoc for more than 60 seconds
		{

#ifdef RTCONFIG_DUAL_BACKHAUL
			if(sta_is_assoc(1))
			{
				if(detect_loop())
				{
					_dprintf("=> RE: detect loop on 5G, reset2!\n");
					reset_loop();
					force_down_2g();
				}
					
				if(!sta_is_assoc(0))
				{
					if(retry>2)
					{
						_dprintf("=> RE: 2G is down ...\n");
						force_down_2g();
						resolving=1;
						retry=0xff;
					}
					else
						retry++;
				}
				else if(!pap_db_mac())
				{	
					force_down_2g();
					_dprintf("=> RE: detcet diff bssid between 2G and 5G !!!\n");
				}
				
			}
			else
#endif
			{

#ifndef RTCONFIG_DUAL_BACKHAUL
				if(!sta_is_assoc(1))
#endif
				{
					_dprintf("=> RE: 5G is not assoc.Reset wpa_supplicant for 5G!!\n");
					wpa_supplicant_stop(1);
					wpa_supplicant_start(1);
					force_down_2g();
#ifdef DETECT_5G
					detect_5g=1;
#endif
				}
			}
			restart_process=1;
			assoc_timeout=-1;
		}
		else
			restart_process=0;
		
		if(!pids("watchdog"))
		{
			 nvram_set("wps_syncing","0");
                         lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
			 start_watchdog();
		}

	}
}
#else //NOT CSU2.2

void start_wifimon_check(int delay)
{
	int i,state, assoc_timeout;
	int restart_process=0;
	int retry_times;
	doSystem("iwpriv wifi0 ignore_dbg 1");

	if(nvram_get_int("wifimon_dbg"))
		dbg_m=1;
	else 
		dbg_m=0;

	if(nvram_get_int("dfs_check_period"))
		check_period=60;
	else
		check_period=30;

	assoc_timeout=0;
	retry_times=0;
#ifdef RTCONFIG_DUAL_BACKHAUL
	disable_2g=0;
#endif
	if(dbg_m)
		_dprintf("=> wifi monitor check: delay %d sec ...\n",delay);
	sleep(delay);

	for(i=0;i<2;i++)
	{
        	doSystem("ifconfig %s down",get_wififname(i));
		sleep(1);
        	doSystem("ifconfig %s up",get_wififname(i));
	}
	while(1)
	{
		if(dbg_m)
			_dprintf("=> RE: wifi monitor check ...\n");
		state=wifimon_check_assoc();
		switch(state)
		{
			case 0:
#ifdef RTCONFIG_DUAL_BACKHAUL
			case 1:
#endif
				assoc_timeout=0;
				retry_times=0;
				break;
			case 2: //2G not-assoc or 5G not-assoc
				assoc_timeout++;
				break;
			default:
				break;
		}
		sleep(check_period);

		if(restart_process)
		{
#ifdef RTCONFIG_DUAL_BACKHAUL
			_dprintf("=> RE:restart hyd/wsplcd/wifimon process after reset 2G or 5G\n");
#else
			_dprintf("=> RE:restart hyd/wsplcd/wifimon process after reset 5G\n");
#endif
			wsplcd_stop();
			hyd_stop();
			gen_wsplcd_conf(1, WSPLCD_CONF, 0);
                	wsplcd_enable();
			hyd_start(1);
		}

		if(assoc_timeout==0)
		{
			restart_process=0;
		}
		else if(assoc_timeout>=2)  //5G not assoc for more than 60 seconds
		{
#ifdef RTCONFIG_DUAL_BACKHAUL
			if(sta_is_assoc(1))
			{
			//	if(dbg_m)
					_dprintf("=> RE: 5G assoc but 2G is not assoc.\n");
#if 0
				wpa_supplicant_stop(0);
				wpa_supplicant_start(0);
#else
				check_wsc_enrollee_status(10); //update 5g bssid

				if(detect_loop())
				{
					_dprintf("=> RE: detect loop on 5G, reset2!\n");
					reset_loop();
				}
				else
				{
					if(!disable_2g)
					{
						_dprintf("=> RE: try to find pap's 2G bssid\n");
						find_pap_2gband_bssid(); //find pap's 2G bssid and restart it.

						if(wait_assoc_stable(0,10)==0) //2G unstable
						{
							retry_times++;
							if(retry_times>1) //120s
							{
								_dprintf("=> RE:try to disable 2G ....\n");
								doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 disable_network 0");
								disable_2g=1;
								retry_times=0;
							}
						}
					}
/*
					else
					{
						retry_times++;

						if(retry_times>3) //240s
						{
								_dprintf("=> RE: try to wakeup 2G.....\n");
								doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 enable_network 0");
								disable_2g=0;
								retry_times=0;
						}
					}
*/
				} //detect loop
#endif
			}
			else
			{
			//	if(dbg_m)
					_dprintf("=> RE: 5G is not assoc.Reset wpa_supplicant for 5G!!\n");
#if 1
				wpa_supplicant_stop(1);
				wpa_supplicant_start(1);
#else
				wpacli_restart(1);
#endif
				if(!sta_is_assoc(0))
				{
			//		if(dbg_m)
						_dprintf("=> RE:2G is not assoc.Reset wpa_supplicant for 2G!!\n");
#if 1
					wpa_supplicant_stop(0);
					wpa_supplicant_start(0);
#else
				wpacli_restart(0);
#endif
					disable_2g=0;
					retry_times=0;

				}
			}
#else
			if(!sta_is_assoc(1))
			{
				_dprintf("=> RE: 5G is not assoc.Reset wpa_supplicant for 5G!!\n");
				wpa_supplicant_stop(1);
				wpa_supplicant_start(1);
			}
#endif
			restart_process=1;
			assoc_timeout=0;
		}
		else
			restart_process=0;
		
		if(!pids("watchdog"))
		{
			 nvram_set("wps_syncing","0");
                         lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
			 start_watchdog();
		}

	}
}
#endif
#endif
