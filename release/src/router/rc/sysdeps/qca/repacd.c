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
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
//#define RADAR_FAILOVER

#ifdef RTCONFIG_DETWAN
#define CONFIGURED_WAN_NIC	nvram_safe_get("detwan_ifname")
#else
#define CONFIGURED_WAN_NIC	DEFAULT_WAN_NIC
#endif

#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300)
#define	DEFAULT_WAN_NIC	"eth0"
#define	DEFAULT_LAN_NIC	"eth1"
#define	DEFAULT_WAN_PORT_SHIFT	5
#define	DEFAULT_LAN_PORT_SHIFT	4
#elif defined(MAPAC1750)
#define	DEFAULT_WAN_NIC	"vlan2"
#define	DEFAULT_LAN_NIC	"vlan1"
#define	DEFAULT_WAN_PORT_SHIFT	2
#define	DEFAULT_LAN_PORT_SHIFT	3
#else
#error Defaine Default WAN/LAN NIC!
#endif

#if defined(MAPAC2200)
////// *** NOTE: Disable this feature if hidden SSID backhaul implementation is done
#define	TRIBAND_CAPONLY_PREFER_HIGHBAND
#define	TRIBAND_HB_CHECK_PERIOD		300 /* 5 minutes */
#define	TRIBAND_HB_INITIAL_PERIOD	120 /* 2 minutes */
#endif

#ifdef RTCONFIG_WIFI_SON
//parameter control
#define wifi_son_mode 1
int hive_daisy_chain=1;  //if 0 , topology is star mode.

int deep_clone=1;
int dbdc_reptr_enable=1-wifi_son_mode;
int block_dfs_enable=0; 	//only for athX
int disable_steering=0;
int scaling_factor=70;
static int distance=0;
#ifdef RTCONFIG_DUAL_BACKHAUL
int sp_from_2g=0;
#endif
int running_cmd=0;
int self_restart=40;
int resolving=0;
int retry_2g=0;
#define  percent_rate_min 30
#define  percent_rate_max 70

#define RTCONFIG_ETHBH_MIXED_MODE
#define	UDPBCAST_SOCKET_RCV
#define	WSPLCD_CONF	"/tmp/wsplcd.conf"
#define	CMP_CONF	"/tmp/wscmp"
#define	CMP_CONF_NEW	"/tmp/wscmp_new"
#define AP_SCAN_FILE	"/tmp/apscan_file.txt"
#ifdef RTCONFIG_ETHBACKHAUL
#define	ETH_BH_MON_EXECNAME	"eth_bh_mon"
#define	UN_PATH_PORT_TO_DAEMON	"/tmp/eth_p2d.sock"
int eth_down_time=30;
#endif
int dbg_m=0;


int check_period=30;


//int multi_ap_enable=wifi_son_mode;
//int manage_mcsd=1-wifi_son_mode; //stop mcsd(Manage the Multicast Services Daemon) daemon when running wifi-son mode
//int lbd_enable=1-wifi_son_mode; //stop lbd daemon(also disable wifi steering) when running wifi-son mode


int cfg_re_syncing=0;
int cfg_changed=0;
#if defined(RTCONFIG_AUTHSUPP)
int re_scy_changed=0;
#endif
char asuscmd[30]="";
char QCA_DRV[] = "athr" ;
void detect_5gband_bssid(void);
int wifimon_check_hops(int band);
int get_mac(char *ifname,char *mac);
int detect_loop(void);
void wpa_cli_set_bssid(int band,char *mac);
extern char *get_qca_iwpriv(char *name, char *command);
extern int getSiteSurvey(int band,char* ofile);
extern int get_ap_mac(const char *ifname, struct iwreq *pwrq);
static void ind_ath(void);
static void set_hy_mcast(void);

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
			if(nvram_match("cfg_master", "1"))
				return 0;
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

	if (band > 10) /* 2.4G guest network check */
		sprintf(buf, "iwconfig ath00%d", band-10);
	else
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
	

		_dprintf("=> %s's hwmode=%s\n",iface,mode);
		for (i = 0; phy_to_std_map[i].hwmode; ++i) {
       		 	if (!strcmp(phy_to_std_map[i].hwmode, mode)) {
	       	     		return phy_to_std_map[i].wsp_std;
			}
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


int auth_support(int band)
{
       char *outpt=NULL;
        int auth=0;
        if ((outpt=get_qca_iwpriv(get_wififname(band), "get_authmode")))
        {
                auth= atoi(outpt);
                free(outpt);
        }
       return auth;
}

int wpa_support(int band)
{
       char *outpt=NULL;
        int wpa=0;
        if ((outpt=get_qca_iwpriv(get_wififname(band), "get_wpa")))
        {
                wpa= atoi(outpt);
                free(outpt);
        }
       return wpa;
}

int hostapd_exist(int band)
{
	int i;
	FILE *fp;
	char buf[1024],prefix[]="hostapd_athxxxxx.conf",*pt1;

	for(i=0;i<2;i++)
	{
		unlink("/tmp/ps.txt");
		doSystem("ps >> /tmp/ps.txt");
		snprintf(prefix, sizeof(prefix), "hostapd_ath%d.conf", band);

		if ((fp = fopen("tmp/ps.txt", "r")) == NULL)
			return 0;

 	      	memset(buf, 0, sizeof(buf));
		while(fgets(buf,1024,fp)!=NULL)
		{
			pt1=strstr(buf,prefix);
			if(pt1) 
			{
        			fclose(fp);
				return 1; //exist
			}
		}
		
        	fclose(fp);	
		sleep(6);
	}
	return 0;
}

int get_wpa_encry(int band)
{
       char buf[1024];
       FILE *fp;
       int len;
       char *pt1,*pt2 = NULL;

       if(!hostapd_exist(band))
		return 0;

       sprintf(buf, "hostapd_cli -i%s get_config", get_wififname(band));
       fp = popen(buf, "r");
       if (fp) {
               memset(buf, 0, sizeof(buf));
               len = fread(buf, 1, sizeof(buf), fp);
               pclose(fp);
               if (len > 1) {
                       buf[len-1] = '\0';
                       pt1 = strstr(buf, "pairwise_cipher=");
                       if (pt1) {
                               pt2 = pt1 + strlen("pairwise_cipher=");
                               chomp(pt2);
                       }
               }

       	       if(!strcmp(pt2,"TKIP"))
               		return 1;
       	       else if(!strcmp(pt2,"CCMP"))
               		return 2;
       	       else
               		return 3;
       }
       return 0;
}




char *get_auth_mode(int band)
{
       int auth=0;
       auth=auth_support(band);
       if(auth==0)
               return "";
       else if(auth==1)
               return "None";
       else if(auth==2)
               return "SharedAuthentication";
       else if(auth==3)
               return "EAPAuthentication";
       else
               return "PSKAuthentication";
}

char *get_encry_mode(int band)
{
       int auth=0,wpa=0;
       auth=auth_support(band);
       if(auth==1 || auth==2)
       {
               if(nvram_get_int("wl0_wep_x")|| nvram_get_int("wl1_wep_x"))
                       return "WEPEncryption";
               else
                       return "";
       }
       else
       {
               wpa=get_wpa_encry(band);
               if(wpa==1)
                       return "TKIPEncryption";
               else if(wpa==2)
                       return "AESEncryption";
               else if(wpa==3)
                       return "TKIPandAESEncryption";
               else
                       return "";
       }

}

char *get_beacon_type(int band)
{
        int auth=0,wpa=0;
       auth=auth_support(band);
       wpa=wpa_support(band);
       switch(auth)
       {
               case 0:
               case 3:
                       return "";
               case 1: //open
                       return "None";
               case 2: //shared
                       return "Basic";
               default:
                       if(wpa==1)
                               return "WPA";
                       else if(wpa==2)
                               return "11i";
                       else if(wpa==3)
                               return "WPAand11i";
                       else
                               return "";
       }

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

#if defined(RTCONFIG_AUTHSUPP)
        int wpa_2g,wpa_5g;
#if defined(MAPAC2200)
        int wpa_5g2;
#endif
#endif
	if (!(fp = fopen(filename, "w+")))
	{
		_dprintf("gen %s file fails!\n", filename);
		return;
	}

#if defined(RTCONFIG_AUTHSUPP)
        wpa_2g=wpa_support(0);
        wpa_5g=wpa_support(1);
#if defined(MAPAC2200)
        wpa_5g2=wpa_support(2);
#endif
#endif
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
#if defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X)
			if(strstr(word, "vlan"))
#else
			if(strstr(word, "eth"))
#endif
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
	fprintf(fp, "config_station=no\n");
	fprintf(fp, "search_timeout=60\n");
	fprintf(fp, "WPS_session_timeout=120\n");
	fprintf(fp, "WPS_retransmission_timeout=10\n");
	fprintf(fp, "WPS_per_message_timeout=15\n");
	fprintf(fp, "band_sel_enable=1\n");
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
	fprintf(fp, "repeat_timeout=3\n");
	fprintf(fp, "internal_timeout=15\n");
	fprintf(fp, "wait_wifi_config_secs_other=30\n");
	fprintf(fp, "wait_wifi_config_secs_first=30\n");
	fprintf(fp, "atf_config_en=0\n");
	fprintf(fp, "cfg_changed=%d\n",cfg_changed);
	fprintf(fp, "cfg_restart_long_timeout=%d\n",self_restart);
	fprintf(fp, "cfg_restart_short_timeout=20\n");
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
#if defined(RTCONFIG_AUTHSUPP)
     	if (!cmp_only) {
        	fprintf(fp, "WLAN.1.BeaconType=%s\n",get_beacon_type(0));
		fprintf(fp, "WLAN.1.BasicEncryptionModes=%s\n",wpa_2g==0?get_encry_mode(0):"");
		fprintf(fp, "WLAN.1.BasicAuthenticationMode=%s\n",wpa_2g==0?get_auth_mode(0):"");
		fprintf(fp, "WLAN.1.WPAEncryptionModes=%s\n",wpa_2g==1?get_encry_mode(0):"");
		fprintf(fp, "WLAN.1.WPAAuthenticationMode=%s\n",wpa_2g==1?get_auth_mode(0):"");
		fprintf(fp, "WLAN.1.IEEE11iEncryptionModes=%s\n",wpa_2g>1?get_encry_mode(0):"");
		fprintf(fp, "WLAN.1.IEEE11iAuthenticationMode=%s\n",wpa_2g>1?get_auth_mode(0):"");
		fprintf(fp, "WLAN.1.KeyPassphrase=%s\n",wpa_2g>0?nvram_get("wl0_wpa_psk"):""); 
	} /* end of cmp_only */
#else
	fprintf(fp, "WLAN.1.BeaconType=%s\n",translate_bcn("wl0_crypto"));
        fprintf(fp, "WLAN.1.BasicEncryptionModes=\n");
        fprintf(fp, "WLAN.1.BasicAuthenticationMode=\n");
        fprintf(fp, "WLAN.1.WPAEncryptionModes=\n");
        fprintf(fp, "WLAN.1.WPAAuthenticationMode=\n");
        fprintf(fp, "WLAN.1.IEEE11iEncryptionModes=%s\n",translate_crypto("wl0_crypto"));
        fprintf(fp, "WLAN.1.IEEE11iAuthenticationMode=%s\n",translate_auth("wl0_auth_mode_x"));
        fprintf(fp, "WLAN.1.KeyPassphrase=%s\n",nvram_get("wl0_wpa_psk"));
#endif
	fprintf(fp, "WLAN.1.DeviceOperationMode=RootAP\n"); //ap
	fprintf(fp, "WLAN.1.X_ATH-COM_APModuleEnable=1\n");
	fprintf(fp, "WLAN.1.X_ATH-COM_WPSPin=12345670\n");
	fprintf(fp, "WLAN.1.X_ATH-COM_VapIfname=%s\n",nvram_get("wl0_ifname")); //2g iface
	fprintf(fp, "WLAN.1.X_ATH-COM_WPSConfigured=CONFIGURED\n");
	fprintf(fp, "WLAN.1.X_ATH-COM_HT40Coexist=1\n");
	fprintf(fp, "WLAN.1.WsplcdUnmanaged=0\n");
	fprintf(fp, "WLAN.1.ASUSCMD=%s\n",asuscmd);  
#if defined(RTCONFIG_AUTHSUPP)
	fprintf(fp, "WLAN.1.SECURITY_TYPE=%d\n",security_type(0));  
	fprintf(fp, "WLAN.1.SECURITY_EXT=%s\n",nvram_get("wl0_wpa_psk"));  
#endif
	if (!cmp_only) {
	fprintf(fp, "WLAN.1.INFORE=%d\n",cfg_re_syncing);
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.1.GROUPKEY=%s\n",gkey);
#if defined(MAPAC2200)
	//fprintf(fp, "WLAN.1.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
	//fprintf(fp, "WLAN.1.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
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
#if defined(RTCONFIG_AUTHSUPP)
        fprintf(fp, "WLAN.3.BeaconType=%s\n",get_beacon_type(0));
        fprintf(fp, "WLAN.3.BasicEncryptionModes=%s\n",wpa_2g==0?get_encry_mode(0):"");
        fprintf(fp, "WLAN.3.BasicAuthenticationMode=%s\n",wpa_2g==0?get_auth_mode(0):"");
        fprintf(fp, "WLAN.3.WPAEncryptionModes=%s\n",wpa_2g==1?get_encry_mode(0):"");
        fprintf(fp, "WLAN.3.WPAAuthenticationMode=%s\n",wpa_2g==1?get_auth_mode(0):"");
        fprintf(fp, "WLAN.3.IEEE11iEncryptionModes=%s\n",wpa_2g>1?get_encry_mode(0):"");
        fprintf(fp, "WLAN.3.IEEE11iAuthenticationMode=%s\n",wpa_2g>1?get_auth_mode(0):"");
	fprintf(fp, "WLAN.3.KeyPassphrase=%s\n",wpa_2g>0?nvram_get("wl0_wpa_psk"):""); 
#else
        fprintf(fp, "WLAN.3.BeaconType=%s\n",translate_bcn("wl0_crypto"));
        fprintf(fp, "WLAN.3.BasicEncryptionModes=\n");
        fprintf(fp, "WLAN.3.BasicAuthenticationMode=\n");
        fprintf(fp, "WLAN.3.WPAEncryptionModes=\n");
        fprintf(fp, "WLAN.3.WPAAuthenticationMode=\n");
        fprintf(fp, "WLAN.3.IEEE11iEncryptionModes=%s\n",translate_crypto("wl0_crypto"));
        fprintf(fp, "WLAN.3.IEEE11iAuthenticationMode=%s\n",nvram_get("wl0_auth_mode_x"));
        fprintf(fp, "WLAN.3.KeyPassphrase=%s\n",nvram_get("wl0_wpa_psk"));  //use ath's key
#endif
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
	fprintf(fp, "WLAN.3.ASUSCMD=%s\n",asuscmd); 
#if defined(RTCONFIG_AUTHSUPP)
	fprintf(fp, "WLAN.3.SECURITY_TYPE=%d\n",security_type(0));  
	fprintf(fp, "WLAN.3.SECURITY_EXT=%s\n",nvram_get("wl0_wpa_psk"));  
#endif
	fprintf(fp, "WLAN.3.INFORE=%d\n",cfg_re_syncing);
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.3.GROUPKEY=%s\n",gkey);
	} /* end of cmp_only */
#if defined(MAPAC2200)
	//fprintf(fp, "WLAN.3.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
	//fprintf(fp, "WLAN.3.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
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

#if defined(RTCONFIG_AUTHSUPP)
        if (!cmp_only) {
	        fprintf(fp, "WLAN.2.BeaconType=%s\n",get_beacon_type(1));
       		fprintf(fp, "WLAN.2.BasicEncryptionModes=%s\n",wpa_5g==0?get_encry_mode(1):"");
        	fprintf(fp, "WLAN.2.BasicAuthenticationMode=%s\n",wpa_5g==0?get_auth_mode(1):"");
        	fprintf(fp, "WLAN.2.WPAEncryptionModes=%s\n",wpa_5g==1?get_encry_mode(1):"");
        	fprintf(fp, "WLAN.2.WPAAuthenticationMode=%s\n",wpa_5g==1?get_auth_mode(1):"");
        	fprintf(fp, "WLAN.2.IEEE11iEncryptionModes=%s\n",wpa_5g>1?get_encry_mode(1):"");
        	fprintf(fp, "WLAN.2.IEEE11iAuthenticationMode=%s\n",wpa_5g>1?get_auth_mode(1):"");
		fprintf(fp, "WLAN.2.KeyPassphrase=%s\n",wpa_5g>0?nvram_get("wl1_wpa_psk"):""); 
        } /* end of cmp_only */
#else
        fprintf(fp, "WLAN.2.BeaconType=%s\n",translate_bcn("wl1_crypto"));
        fprintf(fp, "WLAN.2.BasicEncryptionModes=\n");
        fprintf(fp, "WLAN.2.BasicAuthenticationMode=\n");
        fprintf(fp, "WLAN.2.WPAEncryptionModes=\n");
        fprintf(fp, "WLAN.2.WPAAuthenticationMode=\n");
        fprintf(fp, "WLAN.2.IEEE11iEncryptionModes=%s\n",translate_crypto("wl1_crypto"));
        fprintf(fp, "WLAN.2.IEEE11iAuthenticationMode=%s\n",translate_auth("wl1_auth_mode_x"));
        fprintf(fp, "WLAN.2.KeyPassphrase=%s\n",nvram_get("wl1_wpa_psk"));
#endif
	fprintf(fp, "WLAN.2.DeviceOperationMode=RootAP\n"); //ap
	fprintf(fp, "WLAN.2.X_ATH-COM_APModuleEnable=1\n");
	fprintf(fp, "WLAN.2.X_ATH-COM_WPSPin=12345670\n");
	fprintf(fp, "WLAN.2.X_ATH-COM_VapIfname=%s\n",nvram_get("wl1_ifname")); //5g iface
	fprintf(fp, "WLAN.2.X_ATH-COM_WPSConfigured=CONFIGURED\n");
	fprintf(fp, "WLAN.2.X_ATH-COM_HT40Coexist=1\n");
	fprintf(fp, "WLAN.2.WsplcdUnmanaged=0\n");
	fprintf(fp, "WLAN.2.ASUSCMD=%s\n",asuscmd);  
#if defined(RTCONFIG_AUTHSUPP)
	fprintf(fp, "WLAN.2.SECURITY_TYPE=%d\n",security_type(1));  
	fprintf(fp, "WLAN.2.SECURITY_EXT=%s\n",nvram_get("wl1_wpa_psk"));  
#endif
	if (!cmp_only) {
	fprintf(fp, "WLAN.2.INFORE=%d\n",cfg_re_syncing);
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.2.GROUPKEY=%s\n",gkey);
#if defined(MAPAC2200)
	//fprintf(fp, "WLAN.2.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
	//fprintf(fp, "WLAN.2.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
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

#if defined(RTCONFIG_AUTHSUPP)
        fprintf(fp, "WLAN.4.BeaconType=%s\n",get_beacon_type(1));
        fprintf(fp, "WLAN.4.BasicEncryptionModes=%s\n",wpa_5g==0?get_encry_mode(1):"");
        fprintf(fp, "WLAN.4.BasicAuthenticationMode=%s\n",wpa_5g==0?get_auth_mode(1):"");
        fprintf(fp, "WLAN.4.WPAEncryptionModes=%s\n",wpa_5g==1?get_encry_mode(1):"");
        fprintf(fp, "WLAN.4.WPAAuthenticationMode=%s\n",wpa_5g==1?get_auth_mode(1):"");
        fprintf(fp, "WLAN.4.IEEE11iEncryptionModes=%s\n",wpa_5g>1?get_encry_mode(1):"");
        fprintf(fp, "WLAN.4.IEEE11iAuthenticationMode=%s\n",wpa_5g>1?get_auth_mode(1):"");
	fprintf(fp, "WLAN.4.KeyPassphrase=%s\n",wpa_5g>0?nvram_get("wl1_wpa_psk"):""); 
#else
        fprintf(fp, "WLAN.4.BeaconType=%s\n",translate_bcn("wl1_crypto"));
        fprintf(fp, "WLAN.4.BasicEncryptionModes=\n");
        fprintf(fp, "WLAN.4.BasicAuthenticationMode=\n");
        fprintf(fp, "WLAN.4.WPAEncryptionModes=\n");
        fprintf(fp, "WLAN.4.WPAAuthenticationMode=\n");
        fprintf(fp, "WLAN.4.IEEE11iEncryptionModes=%s\n",translate_crypto("wl1_crypto"));
        fprintf(fp, "WLAN.4.IEEE11iAuthenticationMode=%s\n",translate_auth("wl1_auth_mode_x"));
        fprintf(fp, "WLAN.4.KeyPassphrase=%s\n",nvram_get("wl1_wpa_psk"));  //use ath's key
#endif
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
	fprintf(fp, "WLAN.4.ASUSCMD=%s\n",asuscmd);  
#if defined(RTCONFIG_AUTHSUPP)
	fprintf(fp, "WLAN.4.SECURITY_TYPE=%d\n",security_type(1));  
	fprintf(fp, "WLAN.4.SECURITY_EXT=%s\n",nvram_get("wl1_wpa_psk"));  
#endif	
	fprintf(fp, "WLAN.4.INFORE=%d\n",cfg_re_syncing);
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.4.GROUPKEY=%s\n",gkey);
	} /* end of cmp_only */
#if defined(MAPAC2200)
	//fprintf(fp, "WLAN.4.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#endif
#ifndef RTCONFIG_DUAL_BACKHAUL
	//fprintf(fp, "WLAN.4.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
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

#if defined(RTCONFIG_AUTHSUPP)
        if (!cmp_only) {
        	fprintf(fp, "WLAN.5.BeaconType=%s\n",get_beacon_type(2));
        	fprintf(fp, "WLAN.5.BasicEncryptionModes=%s\n",wpa_5g2==0?get_encry_mode(2):"");
        	fprintf(fp, "WLAN.5.BasicAuthenticationMode=%s\n",wpa_5g2==0?get_auth_mode(2):"");
        	fprintf(fp, "WLAN.5.WPAEncryptionModes=%s\n",wpa_5g2==1?get_encry_mode(2):"");
        	fprintf(fp, "WLAN.5.WPAAuthenticationMode=%s\n",wpa_5g2==1?get_auth_mode(2):"");
        	fprintf(fp, "WLAN.5.IEEE11iEncryptionModes=%s\n",wpa_5g2>1?get_encry_mode(2):"");
        	fprintf(fp, "WLAN.5.IEEE11iAuthenticationMode=%s\n",wpa_5g2>1?get_auth_mode(2):"");
		fprintf(fp, "WLAN.5.KeyPassphrase=%s\n",wpa_5g2>0?nvram_get("wl2_wpa_psk"):""); 
	} /* end of cmp_only */
#else
	fprintf(fp, "WLAN.5.BeaconType=%s\n",translate_bcn("wl2_crypto"));
        fprintf(fp, "WLAN.5.BasicEncryptionModes=\n");
        fprintf(fp, "WLAN.5.BasicAuthenticationMode=\n");
        fprintf(fp, "WLAN.5.WPAEncryptionModes=\n");
        fprintf(fp, "WLAN.5.WPAAuthenticationMode=\n");
        fprintf(fp, "WLAN.5.IEEE11iEncryptionModes=%s\n",translate_crypto("wl2_crypto"));
        fprintf(fp, "WLAN.5.IEEE11iAuthenticationMode=%s\n",translate_auth("wl2_auth_mode_x"));
        fprintf(fp, "WLAN.5.KeyPassphrase=%s\n",nvram_get("wl2_wpa_psk"));
#endif
	fprintf(fp, "WLAN.5.DeviceOperationMode=RootAP\n"); //ap
	fprintf(fp, "WLAN.5.X_ATH-COM_APModuleEnable=1\n");
	fprintf(fp, "WLAN.5.X_ATH-COM_WPSPin=12345670\n");
	fprintf(fp, "WLAN.5.X_ATH-COM_VapIfname=%s\n",nvram_get("wl2_ifname")); //5g iface
	fprintf(fp, "WLAN.5.X_ATH-COM_WPSConfigured=CONFIGURED\n");
	fprintf(fp, "WLAN.5.X_ATH-COM_HT40Coexist=1\n");
	//fprintf(fp, "WLAN.5.CH5G2=%d\n",get_ch(get_freq(2)));  //valid for CAP 5G-2 
#ifndef RTCONFIG_DUAL_BACKHAUL
	//fprintf(fp, "WLAN.5.CH2G=%d\n",get_ch(get_freq(0)));  //valid for CAP 2G
#endif
	fprintf(fp, "WLAN.5.ASUSCMD=%s\n",asuscmd);  
#if defined(RTCONFIG_AUTHSUPP)
	fprintf(fp, "WLAN.5.SECURITY_TYPE=%d\n",security_type(2));  
	fprintf(fp, "WLAN.5.SECURITY_EXT=%s\n",nvram_get("wl2_wpa_psk"));  
#endif
	//WLAN.2 (ap,5G-1), WLAN.5(ap,5G-2): One of them can do sync. Disable 5G-2 sync by wsplcdunmanaged=1!!
	if (!cmp_only) {
	fprintf(fp, "WLAN.5.WsplcdUnmanaged=1\n");
	if (gkey && strlen(gkey))
		fprintf(fp, "WLAN.5.GROUPKEY=%s\n",gkey);


	} /* end of cmp_only */

#endif

#if defined(RTCONFIG_AUTHSUPP)
	sprintf(tmp2, "%d", security_type(0));
	nvram_set("now_security",tmp2);
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
	sprintf(now, "%lu", uptime());
	nvram_set("hyd_cfg_time", now);
	if (!(fp = fopen("/tmp/hyd.conf", "w+")))
		return;
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
		if (strlen(word) > 5 && nvram_get_int("wl0.1_bss_enabled") && nvram_match("wl0.1_lanaccess", "off")) //ignore guest network
			_dprintf("hyd => ignore %\n",word);
		else
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
#ifdef RTCONFIG_ETHBACKHAUL
					if (nvram_get_int("eth_backl"))
						;//sprintf(tmp2,"%s%s",tmp2,DEFAULT_LAN_NIC);
					else
#endif
					sprintf(tmp2,"%s%s",tmp2,word);
				}
			}
			else  {
				if(role) { //RE: all port is set to one VLAN by switch
					if (strcmp(DEFAULT_WAN_NIC, word))
						sprintf(tmp1, "%s%s:ETHER",tmp1,word);
				} else { //CAP: remove upstream port in case of AP mode
					if (strcmp(CONFIGURED_WAN_NIC, word))
						sprintf(tmp1, "%s%s:ETHER",tmp1,word);
				}
			}
		}

        }
	fprintf(fp,"%s\n",tmp1);
	if(role)
	{
#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
		fprintf(fp, "NoRelayGroupList=%s%s\n",tmp2, (nvram_get_int("eth_backl")?DEFAULT_WAN_NIC:"")); //lan group
#else /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
		fprintf(fp, "NoRelayGroupList=%s\n",tmp2); //lan group
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
		fprintf(fp, "ForceGroupRelaying=1\n");
	}
	fprintf(fp, "SwitchLanVid=1\n"); //default 1
	fprintf(fp, "SwitchCpuPort=0\n"); //default 0
#if defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X)
	fprintf(fp, "VlanIds=vlan1.1,vlan2.2\n");
#else
	fprintf(fp, "VlanIds=eth1.1,eth0.2\n");
#endif

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
	if (nvram_get_int("wl0.1_bss_enabled") && nvram_match("wl0.1_lanaccess", "off"))
	{
		fprintf(fp, "PERIODIC_QUERY_INTERVAL=15\n");
		fprintf(fp, "ENABLE_NOTIFICATION_UNICAST=1\n");
	}
	else
	{
		fprintf(fp, "PERIODIC_QUERY_INTERVAL=60\n");
		fprintf(fp, "ENABLE_NOTIFICATION_UNICAST=0\n");
	}

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
	fprintf(fp, "GenerateLLDP=1\n");

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
		if (strlen(word) > 5 && nvram_get_int("wl0.1_bss_enabled") && nvram_match("wl0.1_lanaccess", "off")) //ignore guest network
			_dprintf("hyd => ignore %\n",word);
		else
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
	if(role)
		fprintf(fp, "LowRSSIAPSteeringThreshold=%d\n",nvram_get_int("lyra_ap_steering")+10);
	else
		fprintf(fp, "LowRSSIAPSteeringThreshold=%d\n",nvram_get_int("lyra_ap_steering"));
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
	if(role)
		fprintf(fp, "LowRSSIAPSteeringThreshold=%d\n",nvram_get_int("lyra_ap_steering")+10);
	else
		fprintf(fp, "LowRSSIAPSteeringThreshold=%d\n",nvram_get_int("lyra_ap_steering"));
	
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
	if(role)
		fprintf(fp, "LowRSSIAPSteeringThreshold_W2=%d\n",nvram_get_int("lyra_ap_steering")+10);
	else
		fprintf(fp, "LowRSSIAPSteeringThreshold_W2=%d\n",nvram_get_int("lyra_ap_steering"));
	if(role)
		fprintf(fp, "LowRSSIAPSteeringThreshold_W5=%d\n",nvram_get_int("lyra_ap_steering")+10);
	else
		fprintf(fp, "LowRSSIAPSteeringThreshold_W5=%d\n",nvram_get_int("lyra_ap_steering"));

	fprintf(fp, "[BANDMON]\n");
	fprintf(fp, "MUOverloadThreshold_W2=70\n");
	fprintf(fp, "MUOverloadThreshold_W5=70\n");
	fprintf(fp, "MUSafetyThreshold_W2=50\n");
	fprintf(fp, "MUSafetyThreshold_W5=60\n");
	fprintf(fp, "RSSISafetyThreshold=20\n");
	fprintf(fp, "RSSIMaxAge=5\n");
	fprintf(fp, "ProbeCountThreshold=1\n");
	if(!role)
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
	if(role)
		 fprintf(fp,"APSteerToRootMinRSSIIncThreshold=5\n");
	fprintf(fp, "APSteerToLeafMinRSSIIncThreshold=10\n");
	if(role)
		 fprintf(fp,"APSteerToPeerMinRSSIIncThreshold=10\n");
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
	set_hy_mcast();
        sleep(2);
	//run daemon
	
	if(nvram_get_int("hive_dbg"))
                doSystem("hyd -C /tmp/hyd.conf -d 2>&1 | colog -p hyd -o &");
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
	if ((fp4 = fopen(conf, "w+")) == NULL)
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
	char pid_file[sizeof("/var/run/wifi-staX.pidXXXXXX")];
		
	sprintf(conf, "/tmp/wpa_supplicant-sta%d.conf",band);
	if ((fp = fopen(conf, "w+")) == NULL)
        	return;
	fprintf(fp, "ctrl_interface=/var/run/wpa_supplicant-sta%d\n",band);
        fprintf(fp, "update_config=1\n");
        fclose(fp);
	get_wpa_supplicant_pidfile(get_staifname(band), pid_file, sizeof(pid_file));
	
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
	char *temp=NULL,tmp[50],prefix_wl[]="wlxxxx_";
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

void set_hops_count(int specific)
{
	if (specific!=-1)
		distance=specific;
	else {
#ifdef RTCONFIG_DUAL_BACKHAUL
		distance=short_distance();
#else
		distance=wifimon_check_hops(1);
#endif
#ifdef RTCONFIG_ETHBACKHAUL
		if (nvram_get_int("eth_backl")) {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
			distance=nvram_get_int("eth_bh_up_cost"); /* overwrite RE distance */
#else
			distance=255; /* overwrite RE distance  */
#endif
		}
#endif
	}
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

	_dprintf("Distance form stax is %d, apply to athx !!\n",distance);
#ifdef RTCONFIG_ETHBACKHAUL
	nvram_set_int("mesh_dist", distance);
#endif
}

static void set_hy_mcast(void)
{
	doSystem("hyctl setmc %s retag disable",nvram_get("lan_ifname"));
	doSystem("hyctl setmc %s state enable",nvram_get("lan_ifname"));
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
	doSystem("iwpriv %s mcastenhance 5", get_wififname(band));
	if(!band)   //in dual-backhaul, we will concern and discuss 2G's athnewind
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
		if(!band)   //in dual-backhaul, we will concern and discuss 2G's athnewind
			doSystem("iwpriv %s athnewind 1",get_staifname(band));
		doSystem("iwpriv %s shortgi 1",get_staifname(band));
		doSystem("iwpriv %s set_whc_dist 255", get_staifname(band));
		//doSystem("iwpriv %s mode auto",get_staifname(band));
	//	if(nvram_get_int("dfs_check_period") && band)
	//		doSystem("iwpriv wifi%d staDFSEn 1",band);
		if(nvram_get_int("wl0.1_bss_enabled"))
		{
			doSystem("vconfig add %s 55",get_staifname(band));
			doSystem("ifconfig %s.55 up",get_staifname(band));
			doSystem("brctl addif %s %s.55",BR_GUEST,get_staifname(band));
			sleep(2);
			doSystem("ifconfig %s up",BR_GUEST);
		}
	}
#ifdef RTCONFIG_ETHBACKHAUL
	if(role) //range extender
		nvram_set_int("mesh_dist", 255);
	else
		nvram_set_int("mesh_dist", 0);
#endif
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
	if ((fp = fopen(conf, "r")) == NULL)
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

#if defined(RTCONFIG_AUTHSUPP)
	restart_wifi(0);
#else
	gen_qca_wifi_cfgs();
#endif
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
		//int status;
		int quality;
		p1 = strstr(line, ifname);
		if(p1 == NULL || *(p1+len) != ':' || (p1 != line && *(p1-1) != ' '))
			continue;
		p1 = p1 + len + 1;
		while(*p1 == ' ')
			p1++;
		strtoul(p1, &p2, 16);
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


void wifimon_up(char *sec)
{
	pid_t pid;
	char *wifimon[]={"wifimon_check","20",NULL};
	wifimon[1]=sec;
	doSystem("killall -9 wifimon_check");
	_eval(wifimon, ">>/dev/null", 0, &pid);
}

#ifdef RTCONFIG_ETHBACKHAUL
static void eth_bh_mon_up(void)
{
	pid_t pid;
	char *ethmon[]={ETH_BH_MON_EXECNAME,NULL};
	_eval(ethmon, NULL, 0, &pid);
}
#endif

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
	get_wpa_supplicant_pidfile(get_staifname(band), pid_file, sizeof(pid_file));
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
	get_wpa_supplicant_pidfile(get_staifname(band), pid_file, sizeof(pid_file));
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

int start_wsplcd(void) //violence disable/enable
{
	doSystem("killall wsplcd");
	doSystem("rm -rf "WSPLCD_CONF);
	doSystem("rm -rf /tmp/wsplcd.apply");
        gen_wsplcd_conf(get_role(), WSPLCD_CONF, 0);
	wsplcd_exec=1;
        wsplcd_enable();
        return 0;
}

int start_hyd(void) //violaence disable/enable
{
 	doSystem("killall -9 hyd");
	doSystem("rm -rf /tmp/hyd.conf");
	hyd_exec=1;
	hyd_start(get_role());
	return 0;
}


#ifdef RTCONFIG_ETHBACKHAUL
static void eth_mon_pre_action(int do_poweroff);
#endif
// return 0: normal case
// return 1: nothing changed
int start_cap(int c)
{
#if defined(RTCONFIG_AUTHSUPP)
	char tmp[20];
#endif

#ifdef RTCONFIG_ETHBACKHAUL
	if (c==2 || c==0) { /*restart or init*/
		doSystem("killall ethbh_peer_detect");
		doSystem("killall "ETH_BH_MON_EXECNAME);
		sleep(1);
		doSystem("killall -9 ethbh_peer_detect");
		doSystem("killall -9 "ETH_BH_MON_EXECNAME);
		unlink(UN_PATH_PORT_TO_DAEMON);
		eth_mon_pre_action(1);
	}
#endif
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
		set_hy_mcast();
		gen_wsplcd_conf(0, CMP_CONF, 1);
		cfg_re_syncing=0;
		cfg_changed=0;  
		gen_wsplcd_conf(0, WSPLCD_CONF, 0);
		wsplcd_enable();
		hyd_start(0);
		//wps_enable(0);
#if defined(RTCONFIG_AUTHSUPP)
		nvram_set("cap_security_old",nvram_get("now_security"));
#endif
	}
	else if(c==1) //config change , for user
	{
		_dprintf("CAP: restart after config change\n");
#if defined(RTCONFIG_AUTHSUPP)
		_dprintf("scy-now=%d,scy-next=%d\n",nvram_get_int("now_security"),security_type(0));
		if(nvram_get_int("now_security")!=security_type(0) && nvram_get_int("now_security")!=0)
		{
			nvram_set("cap_security_old",nvram_get("now_security"));
			sprintf(tmp, "%d", security_type(0));
			nvram_set("now_security",tmp);
		}	
#endif			
		gen_wsplcd_conf(0, CMP_CONF_NEW, 1);
		if (cmp_file(CMP_CONF, CMP_CONF_NEW)) {
			unlink(CMP_CONF);
			rename(CMP_CONF_NEW, CMP_CONF);
			if(running_cmd)
				cfg_re_syncing=2;
			else
				cfg_re_syncing=1;
			cfg_changed=1;
			wsplcd_stop();
			gen_wsplcd_conf(0, WSPLCD_CONF, 0);
			wsplcd_enable();
			sleep(6);
			nvram_set("cap_syncing","1");
#if defined(RTCONFIG_LP5523)
			lp55xx_leds_proc(LP55XX_GREENERY_LEDS, LP55XX_WIFI_PARAM_SYNC);
#elif defined(MAPAC1750)
			set_rgbled(RGBLED_BLUE_3ON1OFF);
#endif
		} else
		{
			_dprintf("CAP: ignore to change config\n");
			return 1;
		}
	}
	else if(c==2) //restart, for wsplcd daemon
	{
		_dprintf("CAP: restart wireless\n");
#if defined(RTCONFIG_AUTHSUPP)
		if(nvram_get_int("now_security")!=nvram_get_int("cap_security_old"))
		{
			_dprintf("CAP: change security mode\n");
			restart_wifi(1);
			sleep(10); //wait the time of postwifi.sh
		}
#endif
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
		strcpy(asuscmd,"");
		gen_wsplcd_conf(0, WSPLCD_CONF, 0);
		wsplcd_enable();
		nvram_commit();

#if defined(RTCONFIG_AUTHSUPP)
		if(nvram_get_int("now_security")==nvram_get_int("cap_security_old"))
		{
			_dprintf("CAP: simply restart wireless\n");
			restart_wifi(0);
		}
		nvram_set("cap_security_old",nvram_get("now_security"));
#else
		gen_qca_wifi_cfgs();
#endif
		nvram_set("cap_syncing","0");
		//sleep(30); //estimate wifi-restart time
		//_dprintf("=>CAP: config change stop\n");
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#elif defined(MAPAC1750)
		nvram_set("prelink_pap_status", "-1");
#endif
	}

#ifdef RTCONFIG_ETHBACKHAUL
	if (c==0 || c==2)
		eth_bh_mon_up();
#endif
	return 0;
}

void start_re(int c)
{
	int i;
	wsplcd_stop();
	
#ifdef RTCONFIG_ETHBACKHAUL
	if (c==2 || c==0) { /*restart or init*/
		doSystem("killall ethbh_peer_detect");
		doSystem("killall "ETH_BH_MON_EXECNAME);
		sleep(1);
		doSystem("killall -9 ethbh_peer_detect");
		doSystem("killall -9 "ETH_BH_MON_EXECNAME);
		unlink(UN_PATH_PORT_TO_DAEMON);
		eth_mon_pre_action(1);
	}
#endif
	if(c==3)
	{
		_dprintf("=>RE : ######wait and send msg to next RE#####\n");
		ind_ath();//keep ath1 alive	
		doSystem("ifconfig %s down",get_staifname(1));
#ifdef RTCONFIG_DUAL_BACKHAUL
		doSystem("iwpriv %s athnewind 1", get_wififname(0)); 
		doSystem("ifconfig %s down",get_staifname(0));
#endif

#if defined(RTCONFIG_AUTHSUPP)
		if(nvram_get_int("re_security_new")!=0 && 
			(nvram_get_int("re_security_new")!=nvram_get_int("now_security")))
		{
			_dprintf("RE: change security mode\n");
			resolve_security(nvram_get_int("re_security_new"));
			re_scy_changed=1;
		}
#endif
		if(nvram_get_int("re_syncing")==2)
		{
			cfg_re_syncing=2;
			strcpy(asuscmd,nvram_get("re_asuscmd"));
		}
		else
			cfg_re_syncing=1;
		cfg_changed=1;
		gen_wsplcd_conf(0, WSPLCD_CONF, 0);
		wsplcd_exec=1;
		wsplcd_enable();
		doSystem("killall -9 wifimon_check");
		//check_wsc_enrollee_status(10); //update current pap's bssid
		sleep(self_restart); //estimate waiting time
		_dprintf("=>RE : ###### stop to send msg #####\n");
		doSystem("killall -9 wsplcd");
		doSystem("rm -rf "WSPLCD_CONF);
		doSystem("rm -rf /tmp/wsplcd.apply");
		strcpy(asuscmd,"");
		doSystem("ifconfig %s up",get_staifname(1));
		doSystem("iwpriv %s athnewind 0", get_wififname(1)); //bind ath1 with sta1
#ifdef RTCONFIG_DUAL_BACKHAUL
		doSystem("ifconfig %s up",get_staifname(0));
		doSystem("iwpriv %s athnewind 0", get_wififname(0)); 
#endif
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

#if !defined(RTCONFIG_AUTHSUPP)
		gen_qca_wifi_cfgs();
#endif
		if(c==2 || c==3)//only for wsplcd restart
		{
  			nvram_commit();     
			_dprintf("run asus command=%s\n",nvram_get("re_asuscmd"));	
			if(!strcmp(nvram_get("re_asuscmd"),"reboot"))
				notify_rc("reboot");
			//else
			
			nvram_set("re_asuscmd","0"); //clear asuscmd

#if defined(RTCONFIG_AUTHSUPP)
			restart_wifi(re_scy_changed);
		}
		else
			restart_wifi(0);
#else
		}
#endif

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
//avoid incorrect blue-led casued by 2G connection
#ifdef RTCONFIG_DUAL_BACKHAUL
		doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 disable_network 0");
#endif
	}
#endif
	for(i=0;i<2;i++)  
        	doSystem("ifconfig %s down",get_wififname(i));

	sleep(5);
	doSystem("ifconfig %s up",nvram_get("lan_ifname"));
	doSystem("hyctl attach %s",nvram_get("lan_ifname")); //after br0 is up
	set_hy_mcast();
	
	if(c==2 || c==3) //only for wsplcd restart
	{
		nvram_set("re_syncing","0");

		wpa_cli_set_bssid(1,nvram_get("wl1_sta_bssid"));
		//sleep(10); //estimate
#ifdef RTCONFIG_DUAL_BACKHAUL
		wpa_cli_set_bssid(0,nvram_get("wl0_sta_bssid"));
#endif
		//_dprintf("RE: config change stop\n");
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#elif defined(MAPAC1750)
		nvram_set("prelink_pap_status", "-1");
#endif
		check_wsc_enrollee_status(10);
		cfg_changed=0;

#if defined(RTCONFIG_AUTHSUPP)
		re_scy_changed=0;
#endif
	}
	//gen config and run daemon
	gen_wsplcd_conf(1, WSPLCD_CONF, 0);
	wsplcd_enable();
	hyd_start(1);

//avoid incorrect blue-led casued by 2G connection
#ifdef RTCONFIG_DUAL_BACKHAUL
	doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 enable_network 0");
#endif
	//send RCSA to uplink/CAP/PAP when detect radar
	if(nvram_get_int("dfs_check_period"))
		doSystem("iwpriv wifi1 CSwOpts 0x30");


	if(c==2 || c==3) //only for wsplcd restart
		wifimon_up("0");
	else
		wifimon_up("20");
#ifdef RTCONFIG_ETHBACKHAUL
	if (c==0 || c==2)
		eth_bh_mon_up();
#endif
}


#if defined(RTCONFIG_AUTHSUPP)
int resolve_security(int val)
{
 	int auth,enc,i;
	char tmp[15],tmp2[15],tmp3[15],str[10],str2[10],str3[100];
	auth=(val>>4)&0xf;
	enc=val&0xf;
	if(auth==1)
		strcpy(str,"open");
	else if(auth==2)
		strcpy(str,"shared");
	else if(auth==3)
		strcpy(str,"psk");
	else if(auth==4)
		strcpy(str,"psk2");
	else if(auth==5)
		strcpy(str,"pskpsk2");
	else 
		strcpy(str,"");

	if(enc==1)
		strcpy(str2,"tkip");
	else if(enc==2)
		strcpy(str2,"aes");
	else if(enc==3)
		strcpy(str2,"tkip+aes");
	else
		strcpy(str2,"");

	for(i=0;i<2;i++)
	{
		sprintf(tmp, "wl%d_auth_mode_x",i);
		sprintf(tmp2, "wl%d_crypto",i);
		sprintf(tmp3, "wl%d_wpa_psk",i);
		nvram_set(tmp,str);
		nvram_set(tmp2,str2);
		nvram_set(tmp3,nvram_safe_get("re_security_ext"));
	}
	
#if defined(MAPAC2200)
	nvram_set("wl2_auth_mode_x",str);
	nvram_set("wl2_crypto",str2);
	nvram_set("wl2_wpa_psk",nvram_safe_get("re_security_ext"));
#endif
	_dprintf("==> wl0 auth=%s, wl1 auth=%s\n",nvram_safe_get("wl0_auth_mode_x"),nvram_safe_get("wl1_auth_mode_x"));
}
	

void restart_wifi(int option)
{
	if(option)
		restart_wireless();
	else
		gen_qca_wifi_cfgs();
}

int security_type(int band)
{
	int auth,enc;
        char prefix[] = "wlXXXXXX",tmp[128];
	char *str=NULL;
	snprintf(prefix, sizeof(prefix), "wl%d_", band);
	str = nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp));

	if (str && strlen(str))
	{
		if(!strcmp(str,"open"))
			auth=1;
		else if(!strcmp(str,"shared"))
			auth=2;
		else if(!strcmp(str,"psk"))
			auth=3;
		else if(!strcmp(str,"psk2"))
			auth=4;
		else if(!strcmp(str,"pskpsk2"))
			auth=5;
		else
			auth=6;
	}
	else 
		auth=0;
	
	str = nvram_safe_get(strcat_r(prefix, "crypto", tmp));
	if (str && strlen(str))
	{
		if(!strcmp(str,"tkip"))
			enc=1;
		else if(!strcmp(str,"aes"))
			enc=2;
		else if(!strcmp(str,"tkip+aes")||!strcmp(str,"aes+tkip"))
			enc=3;
		else
			enc=4;
	}
	else
		enc=0;
	return (auth<<4|enc);
}
#endif

int start_cmd(char *cmd)
{
	strncpy(asuscmd,cmd,strlen(cmd));
	running_cmd=1;
	start_cap(1);
	running_cmd=0;		
	return 0;
}

void start_hyfi(void)
{
	int role;
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

#if defined(RTCONFIG_AUTHSUPP)
		if(re_scy_changed)
		{
			set_vap(1,0); //config for 2G range extender
       		 	set_vap(1,1); //config for 5G range extender
#if defined(MAPAC2200)
     	 		set_vap(1,2); //config for 5G-2 range extender
#endif
			return;
		}
		else
#endif //AUTHSUPP
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

#ifdef	TRIBAND_CAPONLY_PREFER_HIGHBAND
static char *get_hyd_data(int sfd, int *rlen, char *terminate)
{
	char *recv_buf;
	int block_len, buf_len, recv_len;
	int ok = 0;
	int terminate_len;
	fd_set rmask;
	struct timeval select_timeout;

	block_len = 500;
	buf_len = recv_len = 0;
	recv_buf = NULL;
	if (terminate)
		terminate_len = strlen(terminate);
	else
		terminate_len = 0;
	while (1) {
		int len;
		char *tmp;

		if (recv_len + block_len >= buf_len) {
			buf_len += block_len;
			tmp = realloc(recv_buf, buf_len + 1);
			if (!tmp)
				break;
			recv_buf = tmp;
		}

		FD_ZERO(&rmask);
		FD_SET(sfd, &rmask);
		select_timeout.tv_sec = 1;
		select_timeout.tv_usec= 0;
		if  (select(sfd+1, &rmask, NULL, NULL, &select_timeout) <= 0)
			break;

		len = read(sfd, recv_buf+recv_len, block_len);
		if (len < 0)
			break;
		recv_len += len;
		if ((terminate_len) && (recv_len >= terminate_len)) {
			if (memcmp(recv_buf+recv_len-terminate_len, terminate, terminate_len) == 0)
				ok = 1;
		}
		if (ok) {
			recv_buf[recv_len] = '\0';
			break;
		}
	};
	if (!ok) {
		if (recv_buf)
			free(recv_buf);
		return NULL;
	}
	if (rlen)
		*rlen = recv_len;
	return recv_buf;
}

static int make_hyd_conn(char *ip, int port)
{
	int sfd, flags;
	struct sockaddr_in rsock;
	fd_set wmask;
	struct timeval select_timeout;

	sfd = socket(AF_INET,SOCK_STREAM,0);
	if ( sfd == -1 )
		return -1;

	flags = fcntl(sfd, F_GETFL, 0);
	fcntl(sfd, F_SETFL, flags | O_NONBLOCK);

	memset ((char *)&rsock,0,sizeof(rsock));
	rsock.sin_addr.s_addr = inet_addr(ip);
	rsock.sin_family = AF_INET;
	rsock.sin_port = htons(port);
	if ( connect(sfd,(struct sockaddr *)(&rsock),sizeof(rsock)) == -1 ) {
		if ( errno != EINPROGRESS )
			return -2;
	}

	FD_ZERO(&wmask);
	FD_SET(sfd, &wmask);
	select_timeout.tv_sec = 1;
	select_timeout.tv_usec= 500000;

	if (select(sfd+1, NULL, &wmask, NULL, &select_timeout) <= 0) {
		close(sfd);
		return -3;
	}

	fcntl(sfd, F_SETFL, flags&~O_NONBLOCK);
	return sfd;
}

/* tds */
static char *get_tds(void)
{
	int sock;
	char outbuf[100];
	char *got_buf=NULL;
	int len;

	sock = make_hyd_conn("127.0.0.1", 7777);
	if (sock < 0)
		return NULL;

	got_buf = get_hyd_data(sock, &len, "@ ");
	if (!got_buf)
		goto fail;
	free(got_buf);

	len = sprintf(outbuf, "td s\r\n");
	write(sock, outbuf, len);
	got_buf = get_hyd_data(sock, &len, "@ ");
	if (!got_buf)
		goto fail;

	len = sprintf(outbuf, "q q\r\n");
	write(sock, outbuf, len);
fail:
	close(sock);
	return got_buf;
}

static int get_re_count(void)
{
	char *tmpbuf;
	int count = -1;
	tmpbuf = get_tds();
	//_dprintf("%s", tmpbuf);
	if (tmpbuf) {
		char *pt;
		pt = strstr(tmpbuf, "-- DB (");
		if (pt) {
			pt += 7; /* size of "-- DB (" */
			count = atoi(pt);
		}
		free(tmpbuf);
	}
	return count;
}
#endif

#ifdef RTCONFIG_ETHBACKHAUL
static inline int get_my_dist(void) {
	return nvram_get_int("mesh_dist");
}
#define VV_DEBUG 1
/* following definition is according to platform & product */
#define	MAX_WANLAN_PORT		5

#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
#define UPSTREAM_DETECTING_DEFAULT_TIMEOUT	4
#define LLDP_UPSTREAM_DETECT_TIMEOUT		10
#define LLDP_DOWNSTREAM_DETECT_TIMEOUT		(UPSTREAM_DETECTING_DEFAULT_TIMEOUT + LLDP_UPSTREAM_DETECT_TIMEOUT)
#else /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
#define	PEER_NODE_DETECT_TIMEOUT		4
#define	ETH_RESET_PORT_COUNTDOWN		8 /* seconds */
#define	EXCHANGE_PORT				9413
#define EXCHANGE_BUF_LEN			64
#define	EXCHANGE_GROUP_LEN			8
#define	EXCHANGE_MAC_LEN			12
#define	MAX_RECORD_ETH_CLIENT			10
#define EXCHANGE_DAEMON_PERIOD		3	/* send beacon every X seconds */
#ifdef RTCONFIG_ETHBH_MIXED_MODE
#define	EXCHANGE_PEER_DIE_TIMEOUT	15	/* remove eth backhaul peer if receive no beacon over X seconds */
#else
#define	EXCHANGE_PEER_DIE_TIMEOUT	30	/* remove eth backhaul peer if receive no beacon over X seconds */
#endif
#define	EXCHANGE_DAEMON_ACT_SENDOUT	1
#define	EXCHANGE_DAEMON_ACT_UPDATE	2
#define	EXCHANGE_DAEMON_ACT_ENTETH_L	4
#define	EXCHANGE_DAEMON_ACT_ENTETH_D	8
#define	EXCHANGE_DAEMON_ACT_DEXIT_ETH	16
#define	EXCHANGE_DAEMON_ACT_LEXIT_ETH	32
#define	EXCHANGE_DAEMON_ACT_RESET_ALL	64	/* reset all LAN ports after 3 seconds */
#define	EXCHANGE_DAEMON_ACT_DIS_PORT	128	/* disable specific port casued by mesh dist */
#define	EXCHANGE_DAEMON_ACT_DIS_PORT2	256	/* disable specific port caused by invalid situation */
#define	EXCHANGE_DAEMON_ACT_BANG_PEER	512	/* response to NON ETH RE peer */

#define DAEMON_CMD_RESET_ALL		1
#define DAEMON_CMD_DISABLE_PORT		2
#define DAEMON_CMD_DISABLE_PORT2	3
#define DAEMON_CMD_LEAVE_ETH		4
#define DAEMON_CMD_ETH_PORT		5

#if 0
#define	HIVE_ETH_SCRIPT_TIMEOUT		12
#else
#define	HIVE_ETH_SCRIPT_TIMEOUT		2
#endif

typedef struct {
	char role; // C, R
	char mode; // E, N, (W)
	char msg_type; // L, D
	char dist_str[3+1]; // 000, 001, 255
	unsigned char group_key[EXCHANGE_GROUP_LEN]; // pre 8 hex-digit of group key
	unsigned char WIFI1_MAC[EXCHANGE_MAC_LEN]; // MAC address of 5G-1 interface
	unsigned char CAP_MAC[EXCHANGE_MAC_LEN]; // MAC address of CAP's 5G-1, or stands for upper NODE's MAC in mixed mode, & sta associated MAC in N mode(mesh_up_sta)
	unsigned char SRC_MAC[EXCHANGE_MAC_LEN]; // MAC address of src MAC
} __attribute__((packed)) exchange_message_t, *exchange_message_pt;

typedef struct {
	unsigned char is_used;
	unsigned char from_port; // 255: by daemon
#ifdef RTCONFIG_ETHBH_MIXED_MODE
	unsigned char distance;
#endif
	time_t update_time;
	unsigned char WIFI1_MAC[EXCHANGE_MAC_LEN+1]; // MAC address of 5G-1 interface
} __attribute__((packed)) daemon_record_t, *daemon_record_pt;

typedef struct {
	unsigned char action; // A(add), D(delete), E(add & enter ethbackl)
	unsigned char port_no; // happened on which port
	unsigned char distance; // peer_distance
	unsigned char WIFI1_MAC[EXCHANGE_MAC_LEN+1]; // MAC address of 5G-1 interface
} __attribute__((packed)) p2d_message_t, *p2d_message_pt;
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */

#if 0 // #ifdef KEEP_ALIVE_CHECK
/* clone Eason's original logic: check alive every 20 seconds, sample result in 10 seconds */
#define STREAM_CHECK_ALIVE_TIME		20
#define STREAM_CHECK_ALIVE_AVG_CNT		10
#endif

enum {
	NOLINK_STATE = 0,
	LINKUP_STABLE,
	HIVE_ETH_WAIT,
#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
	RE_DIRECTION_DETECTING,
	LLDP_UPSTREAM_DETECT,
	LLDP_DOWNSTREAM_DETECT,
	RE_DOWNSTREAM_ETHBACKHAUL_SETUPWAIT,
	RE_ETHBACKHAUL_TEARDOWNWAIT,
#if 0 //#ifdef KEEP_ALIVE_CHECK
	RE_ETHBACKHAUL_TEARDOWNWAIT2,
#endif
#else /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
	PEER_NODE_DETECTING,
	PEER_PORT_DISABLE,
	RE_PEER_RELINK,
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
};

#ifdef VV_DEBUG
static char *state2str(int state)
{
	char *str;	
	switch (state) {
		case NOLINK_STATE:
					str = "<no link>";
					break;
		case LINKUP_STABLE:
					str = "<changed to linkup>";
					break;
		case HIVE_ETH_WAIT:
					str = "<waiting eth script>";
					break;
#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
		case RE_DIRECTION_DETECTING:
					str = "<RE up/down detecting>";
					break;
		case LLDP_UPSTREAM_DETECT:
					str = "<LLDP upstream detect wait>";
					break;
		case LLDP_DOWNSTREAM_DETECT:
					str = "<LLDP downstream detect wait>";
					break;
		case RE_DOWNSTREAM_ETHBACKHAUL_SETUPWAIT:
					str = "<RE downstream eth setup wait>";
					break;
		case RE_ETHBACKHAUL_TEARDOWNWAIT:
					str = "<RE ETH_BACKL tear down wait>";
					break;
#if 0 //#ifdef KEEP_ALIVE_CHECK
		case RE_ETHBACKHAUL_TEARDOWNWAIT2:
					str = "<RE ETH_BACKL tear down wait2>";
					break;
#endif
#else /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
		case PEER_NODE_DETECTING:
					str = "<peer node detection>";
					break;
		case PEER_PORT_DISABLE:
					str = "<port disable>";
					break;
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
		default:
					str = "<INVALID!!!>";
					break;
	}
	return str;
}
#endif

/* sync this value from ipq40xx.c */
#define ISOLATED_VLAN_OFFSET    10

#define DETECT_SCRIPT_NAME	"eth_det_%d.sh"
#define DETECT_RESULT_NAME	"eth_det_%d_output"

#define DETECT_PEER_RESULT_NAME	"det_peer.%s"

int lldpcli_det(void)
{
	char buf[2048];
        FILE *fp;
        int len;
        char *pt1;
	//we listen eth0(upstream) for RE, eth1(downstream) for CAP

	if (get_role()) // RE
		pt1 = DEFAULT_WAN_NIC;
	else { /* CAP */
		if (strcmp(CONFIGURED_WAN_NIC, DEFAULT_WAN_NIC)==0)
			pt1 = DEFAULT_LAN_NIC;
		else
			pt1 = DEFAULT_WAN_NIC;
	}
	sprintf(buf, "lldpcli show neighbors ports %s", pt1);
	
        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
			if(get_role()) //RE
			{
                        	pt1 = strstr(buf, "MAP-CAP"); //get
				if(!pt1)	
                        		pt1 = strstr(buf, "MAP-RE"); //get 

			}
			else
                        	pt1 = strstr(buf, "MAP-RE"); //get 
                        if (pt1)
                               return 1;
		}
	}
	return 0;
}

int lldpcli_det2(void)
{
	char buf[2048];
        FILE *fp;
        int len;
        char *pt1;
	//we listen LAN(downstream) for RE
        sprintf(buf, "lldpcli show neighbors ports "DEFAULT_LAN_NIC);
	
        fp = popen(buf, "r");
        if (fp) {
                memset(buf, 0, sizeof(buf));
                len = fread(buf, 1, sizeof(buf), fp);
                pclose(fp);
                if (len > 1) {
                        buf[len-1] = '\0';
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

static void org_start_eth(int c)
{
        char ifname[32];
        char *next;
#if 0
	int i=0;
#endif
	if(c==3) //RE eth1 up
	{
		_dprintf("=> RE: eth downstream backhaul up\n");
                foreach(ifname, nvram_safe_get("wl_ifnames"), next)
                	ifconfig(ifname, 0, NULL, NULL);
                sleep(15);
                foreach(ifname, nvram_safe_get("wl_ifnames"), next)
                       	ifconfig(ifname, IFUP, NULL, NULL);
	}
	else if(c==2) //RE LAN down
	{
		_dprintf("=> RE: eth downstream backhaul down\n");
                ifconfig(DEFAULT_LAN_NIC, 0, NULL, NULL);
		sleep(15);
		doSystem("killall -9 hyd");
		doSystem("hyd -C /tmp/hyd.conf");
                ifconfig(DEFAULT_LAN_NIC, IFUP, NULL, NULL);
	}
	else if(c==1) //RE LAN up
	{
		_dprintf("=> RE: eth backhaul up\n");
#if 0
                ifconfig(DEFAULT_LAN_NIC, 0, NULL, NULL);
#else
		doSystem("killall wifimon_check");
		kill_wifi_wpa_supplicant(-1);
		wpa_supplicant_stop(1);
#endif
		gen_hyd_conf(1);
		doSystem("killall -9 hyd");
		doSystem("echo 1 > /sys/class/net/br0/bridge/flush");
		doSystem("echo 1 > /sys/class/net/brg0/bridge/flush");
#if 0
		doSystem("hyd -C /tmp/hyd.conf");
		sleep(2);
                ifconfig(DEFAULT_LAN_NIC, IFUP, NULL, NULL);
		// add if up for athnewind config changed
#else
		ifconfig(get_wififname(1), IFUP, NULL, NULL);
#endif
	}
	else if(c==0) //RE LAN down
	{
		_dprintf("=> RE: eth backhaul down\n");
#if 0
                ifconfig(DEFAULT_LAN_NIC, 0, NULL, NULL);
                ifconfig("br0",  0, NULL, NULL);
		sleep(2);
#endif
		gen_hyd_conf(1);
		doSystem("killall -9 hyd");
		doSystem("echo 1 > /sys/class/net/br0/bridge/flush");
		doSystem("echo 1 > /sys/class/net/brg0/bridge/flush");
#if 0
                ifconfig("br0",  IFUP, NULL, NULL);
        	doSystem("echo 1 >> /proc/sys/net/bridge/bridge-nf-call-custom");
		doSystem("hyctl attach %s",nvram_get("lan_ifname")); //after br0 is up
		set_hy_mcast();
#endif
		wpa_supplicant_start(1);

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
#if 0
		sleep(eth_down_time);

		ifconfig(DEFAULT_LAN_NIC, IFUP, NULL, NULL);
		foreach(ifname, nvram_safe_get("wl_ifnames"), next) 
		{
			ifconfig(ifname, 0, NULL, NULL);
			sleep(1);
			ifconfig(ifname, IFUP, NULL, NULL);
		}
#else
		wifimon_up("5");
#endif
	}
}

static void cap_start_eth(int c)
{
	if (c==1) {
		char ifname[32];
		char *next;
		foreach(ifname, nvram_safe_get("wl_ifnames"), next)
			ifconfig(ifname, 0, NULL, NULL);
		sleep(15);
		foreach(ifname, nvram_safe_get("wl_ifnames"), next)
			ifconfig(ifname, IFUP, NULL, NULL);
	} else if (c==0) {
		ifconfig(DEFAULT_LAN_NIC, 0, NULL, NULL);
		sleep(eth_down_time);
		ifconfig(DEFAULT_LAN_NIC, IFUP, NULL, NULL);
		doSystem("killall -9 hyd");
		notify_rc("start_dnsmasq");
	}
}

/*
eth_queue_cmd
0: initial
1: act for daemon hive_eth 0
2: act for daemon hive_eth 1
3: wait to do daemon hive_eth 0
4: wait to do daemon hive_eth 1
9: busy
*/
void start_eth(int c)
{
	if((c==0) || (c==1)) { // duplicated check
		int queue_cmd;
		queue_cmd = nvram_get_int("eth_queue_cmd");
do_again:
		/* queue_cmd should be 1 or 2 here */
		nvram_set("eth_queue_cmd", "9");
		if (get_role())
			org_start_eth(c);
		else
			cap_start_eth(c);
		/* check queue again */
		queue_cmd = nvram_get_int("eth_queue_cmd");
		switch(queue_cmd)
		{
			case 3:
				nvram_set("eth_backl","0");
				queue_cmd = 1;
				break;
			case 4:
				nvram_set("eth_backl","1");
				set_hops_count(-1);
				queue_cmd = 2;
				break;
			case 1:
			case 2:
			case 9:
			default:
				nvram_unset("eth_queue_cmd"); /* set to 0 */
				queue_cmd = 0;
				break;
		}
		if ((queue_cmd) && (queue_cmd-1) != c) {
			c = queue_cmd-1;
			goto do_again;
		}
	} else
		org_start_eth(c);
}

unsigned int isolated_vlan_create( unsigned int mask, char * nic);
unsigned int get_all_portmask(void);
unsigned int get_portlink_bymask(unsigned int portmask);
void power_onoff_port(int portno, int state);
void move_port_to(int portno, char *nic_device);
void isolate_port(int portno);

#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
static void re_detect_direction_on_port(int portno)
{
	FILE *fp;
	char fname[50];
	int vlanid = portno+ISOLATED_VLAN_OFFSET;
	sprintf(fname, "/tmp/"DETECT_RESULT_NAME, portno);
	unlink(fname);
	sprintf(fname, "/tmp/"DETECT_SCRIPT_NAME, portno);
	if (fp = fopen(fname, "w")) {
		fprintf(fp, "#!/bin/sh\n"); 
		fprintf(fp, "ifconfig vlan%d 0.0.0.0 up\n", vlanid); 
                fprintf(fp, "if udhcpc -i vlan%d -t %d -T1 -n -q -R > /dev/null 2>&1; then\n", vlanid, UPSTREAM_DETECTING_DEFAULT_TIMEOUT);
                fprintf(fp, "    echo -n 1 > /tmp/"DETECT_RESULT_NAME"\n", portno);
                fprintf(fp, "else\n");
                fprintf(fp, "    echo -n 0 > /tmp/"DETECT_RESULT_NAME"\n", portno);
                fprintf(fp, "fi\n");
		fprintf(fp, "ifconfig vlan%d down\n", vlanid); 
		fclose(fp);
		chmod(fname, 0755);
		eval(fname);
	}
}

static void re_abort_detect_dir_on_port(int portno)
{
	FILE *fp;
	char fname[50];
	int vlanid = portno+ISOLATED_VLAN_OFFSET;
	sprintf(fname, "killall -9 "DETECT_SCRIPT_NAME, portno);
	system(fname);
	sprintf(fname, "/tmp/"DETECT_RESULT_NAME, portno);
	unlink(fname);
}

/* return 1 for upstream, 0 for downstream */
static int get_direction_detect_result(int portno)
{
	char fname[50];
	char value[5];
	int result;
	sprintf(fname, "/tmp/"DETECT_RESULT_NAME, portno);
	value[0]='\0';
        f_read_string(fname, value, sizeof(value));
	if (value[0]=='1')
		return 1;
	else if (value[0]=='0')
		return 0;
	else {
		_dprintf("BBBBUG!!!!! [%s:%d]: read[%s],result[%s]", \
		__func__, __LINE__, fname, value);
	}
}

inline static void re_move_port_upstream(int portno)
{
	move_port_to(portno, DEFAULT_WAN_NIC); /* group 1 */
}

inline static void re_move_port_downstream(int portno)
{
	move_port_to(portno, DEFAULT_LAN_NIC); /* group 2 */
}

#else /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */

static pid_t peer_detect_on_port(int portno, int timeout, unsigned char *gk_prefix, unsigned char *my_mac)
{
	pid_t pid;
	char vlanstr[10], time_str[10], tmpbuf[30];
	char our_msg[EXCHANGE_BUF_LEN];
	char *det_argv[] = { "ethbh_peer_detect", NULL, NULL, NULL, NULL };
	exchange_message_pt mpt;
	
	sprintf(vlanstr, "vlan%d", portno+ISOLATED_VLAN_OFFSET);
	sprintf(time_str, "%d", timeout);

	memset(our_msg, 0x00, sizeof(our_msg));
	mpt = (exchange_message_pt)&our_msg[0];
	if (get_role()) //RE
		mpt->role='R';
	else
		mpt->role='C';
	if (nvram_get_int("eth_backl"))
		mpt->mode='E';
	else
		mpt->mode='N';
	mpt->msg_type = 'L';
	if (get_role()) //RE
		sprintf(mpt->dist_str, "%03d", get_my_dist());
	else
		sprintf(mpt->dist_str, "%03d", 0);
	mpt->dist_str[sizeof(mpt->dist_str)-1]='S';

	memcpy(mpt->group_key, gk_prefix, sizeof(mpt->group_key));
	memcpy(mpt->WIFI1_MAC, my_mac, sizeof(mpt->WIFI1_MAC));
	if (get_role()) { // RE
		if (nvram_get("eth_bh_CAPMAC"))
			strcpy(tmpbuf, nvram_get("eth_bh_CAPMAC"));
		else
			sprintf(tmpbuf, "000000000000");
	} else
		sprintf(tmpbuf, "000000000000");
	memcpy(mpt->CAP_MAC, tmpbuf, sizeof(mpt->CAP_MAC));
	sprintf(tmpbuf, "000000000000");
	memcpy(mpt->SRC_MAC, tmpbuf, sizeof(mpt->SRC_MAC));
	det_argv[1]=vlanstr;
	det_argv[2]=time_str;
	det_argv[3]=our_msg;

	_eval(det_argv, NULL, 0, &pid);
	return pid;
}

static void detect_daemonon_run(char *lan, unsigned char *gk_prefix, unsigned char *my_mac)
{
	pid_t pid;
	char tmpbuf[30];
	char our_msg[EXCHANGE_BUF_LEN];
	char *det_argv[] = { "ethbh_peer_detect", NULL, NULL, NULL, NULL };
	exchange_message_pt mpt;

	memset(our_msg, 0x00, sizeof(our_msg));
	mpt = (exchange_message_pt)&our_msg[0];
	if (get_role()) //RE
		mpt->role='R';
	else
		mpt->role='C';
	mpt->mode='E';
	mpt->msg_type = 'D';
	/* updated this in while loop */
	sprintf(mpt->dist_str, "%03dS", 0);
	memcpy(mpt->group_key, gk_prefix, sizeof(mpt->group_key));
	memcpy(mpt->WIFI1_MAC, my_mac, sizeof(mpt->WIFI1_MAC));
	sprintf(tmpbuf, "000000000000"); /* it will be updated in daemon */
	memcpy(mpt->CAP_MAC, tmpbuf, sizeof(mpt->CAP_MAC));
	memcpy(mpt->SRC_MAC, tmpbuf, sizeof(mpt->SRC_MAC));
	det_argv[1]=lan;
	det_argv[2]="0"; // indicate to be daemon
	det_argv[3]=our_msg;

	_eval(det_argv, NULL, 0, &pid);
	return;
}

/*
  return -1: no detect result
  return  0: got exact result
  return  1: got chaos case, just disable this port
*/
static int get_peer_detect_result(int portno, unsigned char *key, char *outbuf, int outbuf_size)
{
	char fname[50];
	char vlanstr[10];
	char msgbuf[MAX_RECORD_ETH_CLIENT][sizeof(exchange_message_t)+1];
	int client_count, len, i, cap_count, cap_index;
	exchange_message_pt mpt;
	FILE *fp;

	if (outbuf_size < sizeof(exchange_message_t)) {
		_dprintf("[%s] buffer too small[%d], need[%d]!!!\n", __func__, outbuf_size, sizeof(exchange_message_t));
		return 1;
	}

	sprintf(vlanstr, "vlan%d", portno+ISOLATED_VLAN_OFFSET);
	sprintf(fname, "/tmp/"DETECT_PEER_RESULT_NAME, vlanstr);
	if (!f_exists(fname)) return -1;

	fp = fopen(fname, "rb");
	if (!fp) return -1;

	client_count=0;
	while (client_count < MAX_RECORD_ETH_CLIENT) {
		len = fread(msgbuf[client_count], 1, sizeof(exchange_message_t)+1, fp);
		msgbuf[client_count][sizeof(exchange_message_t)]='\0';
		if (len >= sizeof(exchange_message_t))
			client_count++;
		else
			break;
	}
	fclose(fp);
	if (client_count==0)
		return -1;
	cap_index=0;
	cap_count=0;
	for ( i=0; i<client_count; i++) {
		mpt = (exchange_message_pt)&msgbuf[i][0];
		if (memcmp(mpt->group_key, key, sizeof(mpt->group_key))!=0) {
			return 1;
		}
		if (mpt->role=='C') {
			cap_count++;
			cap_index=i;
		}
		if ((mpt->msg_type=='L') && (client_count>1)) {
			return 1;
		}
	}
	if (cap_count > 1) {
		return 1;
	} else if (cap_count==1) {
		memcpy(outbuf, msgbuf[cap_index], sizeof(exchange_message_t));
	} else { /* cap_count==0 */
		if (client_count!=1) {
			return 1;
		}
		memcpy(outbuf, msgbuf[0], sizeof(exchange_message_t));
	}
	return 0;
}
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */

static void h_chld(int signo)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


static void eth_mon_pre_action(int do_poweroff)
{
	unsigned int all_portmask;
	unsigned char all_portbuf[20];
	char *lan_device=DEFAULT_LAN_NIC;

	nvram_set_int("mesh_dist", 255);
	nvram_set("eth_detect_proc","0");
	nvram_set("eth_backl","0");
	nvram_unset("eth_bh_CAPMAC");
	nvram_unset("eth_bh_resetLAN");
	nvram_unset("eth_queue_cmd");
#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
	nvram_set("re_eth_backl","0");
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
	nvram_set("upstream_port","0");
	nvram_set("downstream_port","0");

	if (get_role()) // RE
		all_portmask = isolated_vlan_create(0, lan_device);
	else {
#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300)
		if (IPTV_ports_cnt() >= 2)
			return; /* No ETH backhaul needed */
#endif	/* MAPAC1300 || MAPAC2200 || VZWAC1300 */

		all_portmask = get_all_portmask();
		/* we don't handle CAP's WAN port */
		if (strcmp(CONFIGURED_WAN_NIC, DEFAULT_WAN_NIC)==0)
			all_portmask &= ~(1 << DEFAULT_WAN_PORT_SHIFT);
		else if (strcmp(CONFIGURED_WAN_NIC, DEFAULT_LAN_NIC)==0) {
			all_portmask &= ~(1 << DEFAULT_LAN_PORT_SHIFT);
			lan_device = DEFAULT_WAN_NIC;
		}

////// => TBD. add bitmask of IPTV occupied port here!!! ///////
#ifndef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
		/* isloated the LAN port */
		isolated_vlan_create(all_portmask, lan_device);
#endif
	}
	snprintf(all_portbuf, sizeof(all_portbuf), "%X", all_portmask);

	nvram_set("tmp_eb_allport", all_portbuf);
	nvram_set("tmp_eb_lan", lan_device);

	if (do_poweroff) {
		unsigned int tmp_shift_mask;
		int i;
		for (i=0; i < MAX_WANLAN_PORT; i++) { /* handle power state */
			tmp_shift_mask = 1 << (i+1);
			if (!(tmp_shift_mask & all_portmask)) continue;
			power_onoff_port(i+1, 0);
		}
	}

	f_write_string("/proc/sys/net/bridge/lan_nic", lan_device, 0, 0);
}

void start_eth_bh_mon(void)
{
	int i, reset_port, role;
	unsigned int all_portmask, change2_linkdown_mask, change2_linkup_mask;
#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
	unsigned int re_upstream_cap_downstream_port_mask, re_downstream_port_mask;
#endif
	unsigned int cur_linkstate, last_linkstate, tmp_state, tmp_shift_mask;
#ifdef	UDPBCAST_SOCKET_RCV
	unsigned int under_link_detect_mask = 0;
#endif
	unsigned int port_state_machine[MAX_WANLAN_PORT] = { NOLINK_STATE };
	unsigned int port_state_machine_timeout[MAX_WANLAN_PORT] = { 0 };
#ifndef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
	unsigned int port_detected_ethbackl[MAX_WANLAN_PORT] = { 0 }; /* 0: none, 1: normal eth, 2: downstream-peer, 3: re-peer-disabled */
	unsigned int port_power_down_count[MAX_WANLAN_PORT] = { 0 };
	unsigned char port_eth_bh_neighbor[MAX_WANLAN_PORT][EXCHANGE_MAC_LEN+1];
#endif
	unsigned char mesh_my_mac[EXCHANGE_MAC_LEN+1];
	char lan_device[20];
	unsigned char our_group_key[EXCHANGE_GROUP_LEN+1];
	int s_un;
	p2d_message_t p2d_buf;
	int last_eth_backl, tmp_eth_backl;
	unsigned int eth_backl_cnt = 0;
	unsigned int linkchange_cnt = 0;
	unsigned int last_port_state_machine[MAX_WANLAN_PORT] = { NOLINK_STATE };
	int ready_run = 0;
	int last_my_dist, my_dist;
#ifdef	TRIBAND_CAPONLY_PREFER_HIGHBAND
	int hb_count_down = TRIBAND_HB_INITIAL_PERIOD; // initial countdown
#endif

#if 0 //#ifdef KEEP_ALIVE_CHECK
	int stream_avg=0, stream_avg_cnt=0, downstream_alive_check=0, upstream_alive_check=0;
#endif
#ifndef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
	pid_t peer_detect_pid[MAX_WANLAN_PORT] = { 0 };
#endif

#ifdef UDPBCAST_SOCKET_RCV
	nvram_set_int("eth_under_detmask", under_link_detect_mask);
#endif
	cur_linkstate = last_linkstate = 0;
	role=get_role();
#ifdef RTCONFIG_ETHBH_MIXED_MODE
	if (role)
		nvram_set_int("eth_bh_up_cost", 0);
#endif
#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
	re_upstream_cap_downstream_port_mask = re_downstream_port_mask = 0;
#endif

	if (nvram_get_int("wl0.1_bss_enabled") && (get_phyrate(11)==0)) {
		ifconfig("ath001", 0, NULL, NULL);
		ifconfig("ath001", IFUP, NULL, NULL);
	}
#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300)
	if ( !role && IPTV_ports_cnt() >= 2)
		return; /* No ETH backhaul needed */
#endif	/* MAPAC1300 || MAPAC2200 || VZWAC1300 */
	if (1)
	{
		pid_t mypid, *pid_list;

		mypid = getpid();
		pid_list = find_pid_by_name(ETH_BH_MON_EXECNAME);
		for (i=0; pid_list[i]!=0; i++) {
			if (pid_list[i] != mypid) {
				_dprintf("[%s] is already running, pid:%u, mypid:%u\n", ETH_BH_MON_EXECNAME, pid_list[i], mypid);
				if ((pid_list[i] < mypid) && (mypid-pid_list[i] < 3) /*ppid*/) {
					_dprintf("[%s]           ignore near pid!!!\n");
					continue;
				}
				_dprintf("EEEEEEEEEEEEError[%s]:%d!!\n", __func__, __LINE__);
				nvram_set("chaos_eth_daemon", "1");
				return;
			}
		}
	}
	if (1)
	{
		unsigned char MAC[6];

		if (ether_atoe(nvram_safe_get("wl1macaddr"), MAC))
			sprintf(mesh_my_mac, "%02X%02X%02X%02X%02X%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
		else {
			_dprintf("!!!!!!Error!!!!!![%s] Cannot get wl1macaddr!!\n", __func__);
			sprintf(mesh_my_mac, "000000000000");
		}
	}
	strcpy(lan_device, nvram_safe_get("tmp_eb_lan"));
	all_portmask = strtoul(nvram_safe_get("tmp_eb_allport"), NULL, 16);
	//nvram_unset("tmp_eb_lan");
	//nvram_unset("tmp_eb_allport");
	for (i=0; i < MAX_WANLAN_PORT; i++) { /* handle power down count */
		char vlan_name[10];
		tmp_shift_mask = 1 << (i+1);
		if (!(tmp_shift_mask & all_portmask)) continue;
		snprintf(vlan_name, sizeof(vlan_name), "vlan%d", i+1+ISOLATED_VLAN_OFFSET);
		eval("iptables", "-D", "INPUT", "-i", vlan_name, "-j", "ACCEPT"); // clear previous one if any
		eval("iptables", "-D", "INPUT", "-i", vlan_name, "-j", "DROP"); // clear previous one if any
		if(role) //RE, no any initial rule in AP mode
			eval("iptables", "-A", "INPUT", "-i", vlan_name, "-j", "DROP");
		port_power_down_count[i] = 2;
	}

	signal(SIGCHLD, h_chld);
	our_group_key[EXCHANGE_GROUP_LEN]='\0';
	memcpy(our_group_key, nvram_safe_get("cfg_group"), EXCHANGE_GROUP_LEN);
	i=0;
	while (strlen(our_group_key)!=EXCHANGE_GROUP_LEN) {
		sleep(1);
		memcpy(our_group_key, nvram_safe_get("cfg_group"), EXCHANGE_GROUP_LEN);
		if (i++ > 10) {
			_dprintf("EEEEEEEEEEEEError[%s] Cannot get key!!\n", __func__);
			nvram_set("chaos_eth_daemon", "1");
			return;
		}
	}

	detect_daemonon_run(lan_device, our_group_key, mesh_my_mac);
	last_eth_backl = tmp_eth_backl =nvram_get_int("eth_backl");
	last_my_dist = 255;
	if (1)
	{
		struct sockaddr_un uaddr;
		int try=0;

		sleep(2);
		while (try++<3) {
			if (f_exists(UN_PATH_PORT_TO_DAEMON)) break;
			sleep(1);
		}

		if ((s_un=socket(AF_LOCAL,SOCK_DGRAM,0)) == -1) {
			_dprintf("[%s]local socket open error!\n", __func__);
			return;
		}

		memset ((char *)&uaddr,0,sizeof(uaddr));
		uaddr.sun_family=AF_LOCAL;
		strcpy(uaddr.sun_path, UN_PATH_PORT_TO_DAEMON);

		if ( connect(s_un,(struct sockaddr*)(&uaddr),sizeof(uaddr)) == -1 ) {
			close(s_un);
			_dprintf("[%s]local socket connect error!\n", __func__);
			return;
		}
	}

	while (1) {
		if (!ready_run) {
			if (nvram_match("x_Setting", "1") && nvram_get_int("wlready")) {
				ready_run = 1;
				continue;
			}
		}
		else {
			if (!nvram_get_int("wlready")) { /* under restart wireless */
				if (ready_run == 1) {
					if(nvram_get_int("eth_backl"))
						nvram_set("eth_backl","0");
					ready_run = 2; /* waiting next wlready */
					doSystem("killall ethbh_peer_detect");
				}
			} else if (ready_run == 2) { /* wifi ready again */
				doSystem("killall ethbh_peer_detect"); /* kill again */
				eth_mon_pre_action(1);
				sleep(1);
				doSystem("killall -9 ethbh_peer_detect");
				unlink(UN_PATH_PORT_TO_DAEMON);
				nvram_set("chaos_eth_daemon", "1");
				// make assurance
				wifimon_up("0");
				break; /* leave daemon */
			}
			tmp_eth_backl=nvram_get_int("eth_backl");
			my_dist = get_my_dist();
			change2_linkdown_mask = change2_linkup_mask = 0;
			cur_linkstate = get_portlink_bymask(all_portmask);
			if ( cur_linkstate != last_linkstate ) { /* something changed */
				tmp_state = cur_linkstate ^ last_linkstate;
				for (i=0; i < MAX_WANLAN_PORT; i++) { /* handle state machine */
					tmp_shift_mask = 1 << (i+1);
					if ( tmp_state & tmp_shift_mask ) { /* port link state changed */
						if ( tmp_shift_mask & last_linkstate ) /* link down */
							change2_linkdown_mask |= tmp_shift_mask;
						else if ( tmp_shift_mask & cur_linkstate ) /* link up */
							change2_linkup_mask |= tmp_shift_mask;
						else /* BUG!! */
							_dprintf("BBBBUG!!!!! [%s:%d]: last:0x%02x, cur:0x%02x, tmp_shift:0x%02x\n", \
								__func__, __LINE__, last_linkstate, cur_linkstate, tmp_shift_mask);
					}
				}
				last_linkstate = cur_linkstate;
				linkchange_cnt = 10;
			}
			for (i=0; i < MAX_WANLAN_PORT; i++) { /* handle state machine */
				tmp_shift_mask = 1 << (i+1);
				if (last_port_state_machine[i]!=port_state_machine[i]) {
					if ( eth_backl_cnt ) /* reset count down */
						eth_backl_cnt = ETH_RESET_PORT_COUNTDOWN;
					_dprintf("<<State Changed>>: port[%d]:%s, port_mask:0x%02x\n", i+1, state2str(port_state_machine[i]), tmp_shift_mask);
				}
				last_port_state_machine[i]=port_state_machine[i];
				switch (port_state_machine[i]) {
					case	NOLINK_STATE:
								if ( tmp_shift_mask & change2_linkup_mask ) { /* link up, go to upstream detecting */
#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
									if (role) {//RE
										re_detect_direction_on_port(i+1);
										port_state_machine[i]=RE_DIRECTION_DETECTING;
										port_state_machine_timeout[i]=UPSTREAM_DETECTING_DEFAULT_TIMEOUT;
									} else { // CAP
										/* move_port_downstream(tmp_shift_mask); */ /* CAP don't need to move port */
										if (nvram_get_int("eth_backl")==0) {
											port_state_machine[i]=LLDP_DOWNSTREAM_DETECT;
											port_state_machine_timeout[i]=LLDP_DOWNSTREAM_DETECT_TIMEOUT; /* wait for being stable */
										}
									}
#else /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
									char vlan_name[10];
									snprintf(vlan_name, sizeof(vlan_name), "vlan%d", i+1+ISOLATED_VLAN_OFFSET);
									eval("iptables", "-I", "INPUT", "-i", vlan_name, "-j", "ACCEPT");
#ifdef UDPBCAST_SOCKET_RCV
									under_link_detect_mask |= (1<<i);
									nvram_set_int("eth_under_detmask", under_link_detect_mask);
#endif
									peer_detect_pid[i]=peer_detect_on_port(i+1, PEER_NODE_DETECT_TIMEOUT, our_group_key, mesh_my_mac);
									port_state_machine[i]=PEER_NODE_DETECTING;
									port_state_machine_timeout[i]=PEER_NODE_DETECT_TIMEOUT+1;
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
								} else if ( tmp_shift_mask & change2_linkdown_mask ) {
									_dprintf("BBBBUG!!!!! [%s:%d]: port:%d, c2_linkup:0x%02x, c2_linkdown:0x%02x\n", \
										__func__, __LINE__, i+1, change2_linkup_mask, change2_linkdown_mask);
								}
								break;
					case	LINKUP_STABLE:
								if ( tmp_shift_mask & change2_linkdown_mask ) { /* link down, isolate port */
									isolate_port(i+1);
									port_state_machine[i]=NOLINK_STATE;
#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
									if (re_upstream_cap_downstream_port_mask == tmp_shift_mask) {
										re_upstream_cap_downstream_port_mask = 0;
										if(nvram_get_int("eth_backl")) {
											nvram_set("eth_backl","0");
											eval("hive_eth","0");
											nvram_set_int("prelink_pap_status", -1); // trigger LED to change
										}
									} else if (re_downstream_port_mask == tmp_shift_mask) {
										re_downstream_port_mask = 0;
										if(nvram_get_int("re_eth_backl"))
										{
											ifconfig(lan_device, 0, NULL, NULL);
											port_state_machine[i]=RE_ETHBACKHAUL_TEARDOWNWAIT; /* over write state */
											port_state_machine[i]=eth_down_time;
										}
									}
#else /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
									port_state_machine[i]=NOLINK_STATE;
									if (port_detected_ethbackl[i]==1) {
#if 0
										int j, count;
										port_detected_ethbackl[i]=0;

										if(role) { //RE
											count=0;
											nvram_unset("eth_bh_CAPMAC");
											for (j=0;j<MAX_WANLAN_PORT;j++)
												if ( port_detected_ethbackl[j]==2 ) {
													port_power_down_count[j]=2;
													power_onoff_port(j+1, 0);
												}
										} else { // CAP
											for (j=0, count=0;j<MAX_WANLAN_PORT;j++)
												if ( port_detected_ethbackl[j]==1 ) count++;
										}

										if (count==0) {
											_dprintf("<<ETH BACKL>>: leave eth backhaul, port[%d]\n", i+1);
											nvram_set("eth_backl","0");
											eval("hive_eth","0");
											nvram_set_int("prelink_pap_status", -1); // trigger LED to change
										}
#else
										port_detected_ethbackl[i]=0;
										if (tmp_eth_backl==0) {
											_dprintf("<<ETH BACKL>>: chaos status!!! at port[%d]\n", i+1);
										} else {
											p2d_buf.action='D';
											p2d_buf.port_no=(unsigned char)(i+1);
											if(role) //RE
												memcpy(p2d_buf.WIFI1_MAC, nvram_safe_get("eth_bh_CAPMAC"), EXCHANGE_MAC_LEN+1);
											else
												memcpy(p2d_buf.WIFI1_MAC, port_eth_bh_neighbor[i], EXCHANGE_MAC_LEN+1);
											send(s_un , &p2d_buf, sizeof(p2d_buf), 0); /* notify daemon to handle eth_backl */
										}
#endif
									} else
										port_detected_ethbackl[i]=0;
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
								} else if ( tmp_shift_mask & change2_linkup_mask ) {
									_dprintf("BBBBUG!!!!! [%s:%d]: port:%d, c2_linkup:0x%02x, c2_linkdown:0x%02x\n", \
										__func__, __LINE__, i+1, change2_linkup_mask, change2_linkdown_mask);
								} else {
#if 0 // #ifdef KEEP_ALIVE_CHECK
									if (upstream_port_mask == tmp_shift_mask) { /* stability check, only for RE upstream */
										if ( upstream_alive_check == 0 ) {
											if ((stream_avg%2)==0) {
												if(lldpcli_det())
													stream_avg_cnt++;
											}
											if (--stream_avg == 0) {
												if (stream_avg_cnt <= 2) {
													_dprintf("[EE] upstream port=0x%02x without lyra,disable eth-backhaul\n", tmp_shift_mask);
													nvram_set("eth_backl","0");
													eval("hive_eth","0");
													nvram_set_int("prelink_pap_status", -1); // trigger LED to change
												} else {
													upstream_alive_check = STREAM_CHECK_ALIVE_TIME;
												}
											}
										} else {
											upstream_alive_check--;
											if ( upstream_alive_check == 0) {
												stream_avg = STREAM_CHECK_ALIVE_AVG_CNT;
												stream_avg_cnt = 0; 
											}
										}
									} else if (role && downstream_port_mask == tmp_shift_mask) { /* stability check, only for RE */
										if ( downstream_alive_check == 0 ) {
											if ((stream_avg%2)==0) {
												if(lldpcli_det2())
													stream_avg_cnt++;
											}
											if (--stream_avg == 0) {
												if (stream_avg_cnt <= 2) {
													_dprintf("[EE] downstream port=0x%02x without lyra,disable eth-backhaul\n", tmp_shift_mask);
													ifconfig(lan_device, 0, NULL, NULL);
													port_state_machine[i]=RE_ETHBACKHAUL_TEARDOWNWAIT2; /* over write state */
													port_state_machine[i]=eth_down_time;
												} else {
													downstream_alive_check = STREAM_CHECK_ALIVE_TIME;
												}
											}
										} else {
											downstream_alive_check--;
											if ( downstream_alive_check == 0) {
												stream_avg = STREAM_CHECK_ALIVE_AVG_CNT;
												stream_avg_cnt = 0; 
											}
										}

									}
#endif
								}
								break;
					case HIVE_ETH_WAIT:
								if ( tmp_shift_mask & change2_linkdown_mask ) { /* link down */
									port_state_machine[i]=NOLINK_STATE;
									port_detected_ethbackl[i]=0;
									p2d_buf.action='D';
									p2d_buf.port_no=(unsigned char)(i+1);
									if(role) //RE
										memcpy(p2d_buf.WIFI1_MAC, nvram_safe_get("eth_bh_CAPMAC"), EXCHANGE_MAC_LEN+1);
									else
										memcpy(p2d_buf.WIFI1_MAC, port_eth_bh_neighbor[i], EXCHANGE_MAC_LEN+1);
									send(s_un , &p2d_buf, sizeof(p2d_buf), 0); /* notify daemon to handle eth_backl */
								} else {
									if (port_state_machine_timeout[i]==0) {
										move_port_to(i+1, lan_device);
										port_state_machine[i]=LINKUP_STABLE;
									} else
										port_state_machine_timeout[i]--;
								}
								break;
#if 0 //#ifdef RTCONFIG_QCA_ORG_UPDOWN_SEPARATE
					case	RE_DIRECTION_DETECTING: /* only RE */
								if ( tmp_shift_mask & change2_linkdown_mask ) { /* link down, isolate port */
									re_abort_detect_dir_on_port(i+1);
									isolate_port(i+1);
									port_state_machine[i]=NOLINK_STATE;
								}
								else {
									if (port_state_machine_timeout[i]==0) {
										if (get_direction_detect_result(i+1)) { /* upstream */
											re_move_port_upstream(i+1);
											if (nvram_get_int("eth_backl")==0) {
												port_state_machine[i]=LLDP_UPSTREAM_DETECT;
												port_state_machine_timeout[i]=LLDP_UPSTREAM_DETECT_TIMEOUT; /* wait for being stable */
											} else
												port_state_machine[i]=LINKUP_STABLE;
										} else { /* downstream */
											re_move_port_downstream(i+1);
											if (nvram_get_int("re_eth_backl")==0) {
												port_state_machine[i]=LLDP_DOWNSTREAM_DETECT;
												port_state_machine_timeout[i]=LLDP_DOWNSTREAM_DETECT_TIMEOUT; /* wait for being stable */
											} else
												port_state_machine[i]=LINKUP_STABLE;
										}
									} else
										port_state_machine_timeout[i]--;
								}
								break;
					case	LLDP_UPSTREAM_DETECT: /* only RE */
								if ( tmp_shift_mask & change2_linkdown_mask ) { /* link down, isolate port */
									// if(role) //RE
									isolate_port(i+1);
									port_state_machine[i]=NOLINK_STATE;
								}
								else {
									if (port_state_machine_timeout[i]==0) {
										if(nvram_get_int("eth_backl")==0) {
											if(lldpcli_det()) {
												nvram_set("eth_backl","1");
												eval("hive_eth","1");
												nvram_set_int("prelink_pap_status", -1); // trigger LED to change
												re_upstream_cap_downstream_port_mask = tmp_shift_mask;
#if 0 //#ifdef KEEP_ALIVE_CHECK
												upstream_alive_check = STREAM_CHECK_ALIVE_TIME;
#endif
											}
										}
										port_state_machine[i]=LINKUP_STABLE;
									} else
										port_state_machine_timeout[i]--;
								}
								break;
					case	RE_DOWNSTREAM_ETHBACKHAUL_SETUPWAIT:
								if ( tmp_shift_mask & change2_linkdown_mask ) { /* link down, isolate port */
									char ifname[32];
									char *next;
									foreach(ifname, nvram_safe_get("wl_ifnames"), next)
										ifconfig(ifname, IFUP, NULL, NULL);
									nvram_set("re_eth_backl","0");
									isolate_port(i+1);
									port_state_machine[i]=NOLINK_STATE;
								} else {
									if (port_state_machine_timeout[i]==0) {
										char ifname[32];
										char *next;
										foreach(ifname, nvram_safe_get("wl_ifnames"), next)
											ifconfig(ifname, IFUP, NULL, NULL);
#if 0 // #ifdef KEEP_ALIVE_CHECK
										upstream_alive_check = STREAM_CHECK_ALIVE_TIME;
#endif
										port_state_machine[i]=LINKUP_STABLE;
									} else
										port_state_machine_timeout[i]--;
								}
								break;
#if 0 // #ifdef KEEP_ALIVE_CHECK
					case	RE_ETHBACKHAUL_TEARDOWNWAIT2:
#endif
					case	RE_ETHBACKHAUL_TEARDOWNWAIT:
								if (port_state_machine_timeout[i]==0) {
									ifconfig(lan_device, IFUP, NULL, NULL);
									nvram_set("re_eth_backl","0");
									doSystem("killall -9 hyd");
									notify_rc("start_dnsmasq");
									if ( port_state_machine[i]==RE_ETHBACKHAUL_TEARDOWNWAIT)
										port_state_machine[i]=NOLINK_STATE;
								} else
									port_state_machine_timeout[i]--;
								break;
					case	LLDP_DOWNSTREAM_DETECT:
								if ( tmp_shift_mask & change2_linkdown_mask ) { /* link down, isolate port */
									if(role) //RE
										isolate_port(i+1);
									port_state_machine[i]=NOLINK_STATE;
								}
								else {
									if (port_state_machine_timeout[i]==0) {
										if(role) { //RE
											if(nvram_get_int("re_eth_backl")==0) {
												if(lldpcli_det2()) {
													char ifname[32];
													char *next;
													nvram_set("re_eth_backl","1");
													re_downstream_port_mask = tmp_shift_mask;
													foreach(ifname, nvram_safe_get("wl_ifnames"), next)
														ifconfig(ifname, 0, NULL, NULL);
													port_state_machine[i]=RE_DOWNSTREAM_ETHBACKHAUL_SETUPWAIT;
													port_state_machine_timeout[i]=15; /* old code */
												} else {
													port_state_machine[i]=LINKUP_STABLE;
												}
											}
										} else { // CAP
											if(nvram_get_int("eth_backl")==0) {
												if(lldpcli_det()) {
													nvram_set("eth_backl","1");
													eval("hive_eth","1");
													nvram_set_int("prelink_pap_status", -1); // trigger LED to change
													re_upstream_cap_downstream_port_mask = tmp_shift_mask;
#if 0 // #ifdef KEEP_ALIVE_CHECK
													downstream_alive_check = STREAM_CHECK_ALIVE_TIME;
#endif
												}
											}
											port_state_machine[i]=LINKUP_STABLE;
										}
									} else
										port_state_machine_timeout[i]--;
								}
								break;
#else /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
					case 	PEER_NODE_DETECTING:
								if ( tmp_shift_mask & change2_linkdown_mask ) { /* link down, isolate port */
									char vlan_name[10];
									snprintf(vlan_name, sizeof(vlan_name), "vlan%d", i+1+ISOLATED_VLAN_OFFSET);
									eval("iptables", "-D", "INPUT", "-i", vlan_name, "-j", "ACCEPT");
									if (peer_detect_pid[i]) {
										kill(peer_detect_pid[i], SIGTERM);
										peer_detect_pid[i]=0;
									}
									isolate_port(i+1);
#ifdef UDPBCAST_SOCKET_RCV
									under_link_detect_mask &= ~(1<<i);
									nvram_set_int("eth_under_detmask", under_link_detect_mask);
#endif
									port_state_machine[i]=NOLINK_STATE;
								}
								else {
									if (port_state_machine_timeout[i]==0) {
										char peer_buf[EXCHANGE_BUF_LEN];
										int detect_result;
										char vlan_name[10];

										peer_detect_pid[i]=0;
										snprintf(vlan_name, sizeof(vlan_name), "vlan%d", i+1+ISOLATED_VLAN_OFFSET);
										eval("iptables", "-D", "INPUT", "-i", vlan_name, "-j", "ACCEPT");
#ifdef UDPBCAST_SOCKET_RCV
										under_link_detect_mask &= ~(1<<i);
										nvram_set_int("eth_under_detmask", under_link_detect_mask);
#endif
										detect_result = get_peer_detect_result(i+1, our_group_key, peer_buf, EXCHANGE_BUF_LEN);
										if ( detect_result == 0 ) { /* got message */
											exchange_message_pt mpt;
											int result=0; /* 0: disable, 1: stable link, 3: stable lin+eth backhaul */
											unsigned char CAP_MAC[EXCHANGE_MAC_LEN+1];
											unsigned char neighbor_MAC[EXCHANGE_MAC_LEN+1];
#ifdef RTCONFIG_ETHBH_MIXED_MODE
											int peer_dist;
#endif
											mpt = (exchange_message_pt)&peer_buf[0];
											mpt->dist_str[sizeof(mpt->dist_str)-1]='\0';
#ifdef RTCONFIG_ETHBH_MIXED_MODE
											peer_dist=atoi(mpt->dist_str);
#endif
											CAP_MAC[EXCHANGE_MAC_LEN]='\0';
											neighbor_MAC[EXCHANGE_MAC_LEN]='\0';
											memcpy(CAP_MAC, mpt->CAP_MAC, EXCHANGE_MAC_LEN);
											memcpy(neighbor_MAC, mpt->WIFI1_MAC, EXCHANGE_MAC_LEN);

											if (tmp_eth_backl==0) { // not yet eth
												if (role) { //RE
													if (mpt->role=='C') {
														result = 3;
													} else if (mpt->role=='R') {
														if (mpt->mode=='N') {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
															if (my_dist == peer_dist) {
																_dprintf("<<ETH BACKL>>: disable NonETH RE on port[%d], with same dist:%s\n", i+1, mpt->dist_str);
																port_detected_ethbackl[i] = 3; // wake it up when we enter ETH
															} else if (my_dist > peer_dist) { /* we got another short-cut */
																if (strcmp(mesh_my_mac, CAP_MAC)==0) {
																	_dprintf("<<ETH BACKL>>: Loop detection on port[%d], with dist:%d, mydist:%d, disable it!\n", i+1, peer_dist, my_dist);
																} else {
																	_dprintf("<<ETH BACKL>>: detect another NonETH RE on port[%d], with dist:%d, mydist:%d, enable ETH\n", i+1, peer_dist, my_dist);
																	result = 3;
																}
															} else { /* we become upstream node */
																//TBD. accroding to Eason's re_eth_backl  maybe we need to down/up LAN nic here
																// port_detected_ethbackl[i] = 2;
																result = 4;
															}
#else
															port_detected_ethbackl[i] = 3; // wake it up when we enter ETH
#endif
														} else if (mpt->mode=='E') {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
															_dprintf("<<ETH BACKL>>: detect new ETH RE on port[%d], dist:%d\n", i+1, peer_dist);
															if ((peer_dist == 1) || (my_dist > peer_dist)) { /* we got another short-cut */
																// if peer_dist is 1, we should connect to CAP directly after negotiation
																_dprintf("               enter ETH BACKL, mydist:%d, peer_dist:%d\n", my_dist, peer_dist);
																result = 3;
															} else {
																_dprintf("               disable port, mydist:%d, peer_dist:%d\n", my_dist, peer_dist);
																port_detected_ethbackl[i] = 3; // wake it up when we enter ETH
															}
#else
															result = 3;
#endif
														}
													}
												} else { // CAP
													if (mpt->role=='R') {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
														result = 4;
#else
														if (mpt->mode=='N') {
															result = 3;
														}
#endif
													}
												}
											} else { /* we are already under eth_backl */
												if (role) { //RE
													if (mpt->role=='R') {
														if (mpt->mode=='N') { /* daisy chain case~~ */
#ifdef RTCONFIG_ETHBH_MIXED_MODE
															_dprintf("<<ETH BACKL>>: detect another RE[%s] on port[%d], dist:%d\n", neighbor_MAC, i+1, peer_dist);
															if ((my_dist != 1) && (my_dist >= peer_dist)) { /* disable this port */
																_dprintf("               disable port, mydist:%d, peer_dist:%d\n", my_dist, peer_dist);
															} else { /* peer will enter eth backhaul */
																result = 4;
																// dont' care => port_detected_ethbackl[i]=2; // re down stream
																//TBD. accroding to Eason's re_eth_backl maybe we need to down/up LAN nic here
															}
#else
															result = 1;
															port_detected_ethbackl[i]=2; // re down stream
															// note: we cannot set this port as ethbackl!!
#endif
														}
													}
												} else { // CAP
													if (mpt->role=='R') {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
														result=4;
#else
														if (mpt->mode=='N') {
															result = 1;
															port_detected_ethbackl[i]=1;
														}
#endif
													}
												}
											}
											if (result & 1) {
												memcpy(port_eth_bh_neighbor[i], neighbor_MAC, EXCHANGE_MAC_LEN+1);
												if (result & 2) {
													p2d_buf.action='E';
													port_state_machine_timeout[i]=HIVE_ETH_SCRIPT_TIMEOUT; /* old code */
													port_state_machine[i]=HIVE_ETH_WAIT;
													port_detected_ethbackl[i]=1;
													if (role) { //RE
#if 0 /* move to other place */
														int j;
														for (j=0;j<MAX_WANLAN_PORT;j++)
															if ( port_detected_ethbackl[j]==3 ) {
																port_power_down_count[j]=2;
																power_onoff_port(j+1, 0);
															}
#endif
#ifdef RTCONFIG_ETHBH_MIXED_MODE
														if (mpt->role=='C')
															nvram_set_int("eth_bh_up_cost", 1);
														else {
															if (mpt->mode=='E') {
																nvram_set_int("eth_bh_up_cost", peer_dist);
																p2d_buf.distance=peer_dist;
															}
															else {
																nvram_set_int("eth_bh_up_cost", peer_dist+1);
																p2d_buf.distance=peer_dist+1;
															}
														}
														nvram_set("mesh_dist", nvram_get("eth_bh_up_cost"));
														if (memcmp(CAP_MAC, "000000000000", 12)==0)
															nvram_set("eth_bh_CAPMAC", neighbor_MAC);
														else
															nvram_set("eth_bh_CAPMAC", CAP_MAC);
#else
														nvram_set("eth_bh_CAPMAC", CAP_MAC);
#endif
													}
													/* move to daemon
													nvram_set("eth_backl","1");
													eval("hive_eth","1");
													nvram_set_int("prelink_pap_status", -1); // trigger LED to change
													*/ 
												} else {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
													_dprintf("<<ETH BACKL>>: ERR, no such case[%d]!!!\n", i+1);
#else
													p2d_buf.action='A';
													move_port_to(i+1, lan_device);
													port_state_machine[i]=LINKUP_STABLE;
#endif
												}
												p2d_buf.port_no=(unsigned char)(i+1);
												if (role) //  RE
													memcpy(p2d_buf.WIFI1_MAC, CAP_MAC, EXCHANGE_MAC_LEN+1);
												else
													memcpy(p2d_buf.WIFI1_MAC, neighbor_MAC, EXCHANGE_MAC_LEN+1);
												send(s_un , &p2d_buf, sizeof(p2d_buf), 0);
											}
#ifdef RTCONFIG_ETHBH_MIXED_MODE
											else if (result & 4) {
												_dprintf("<<ETH BACKL>>: act as ETH upstream on port[%d]\n", i+1);
												move_port_to(i+1, lan_device);
												port_state_machine[i]=LINKUP_STABLE;
											}
#endif
											else {
												if (tmp_eth_backl && port_detected_ethbackl[i]==1) { /* detected by Daemon */
													_dprintf("<<ETH BACKL>>: detected by daemon on port[%d]\n", i+1);
													move_port_to(i+1, lan_device);
													port_state_machine[i]=LINKUP_STABLE;
												} else {
													port_state_machine[i]=PEER_PORT_DISABLE; /* default case */
												}
											}
										} else if ( detect_result == 1 ) { /* chaos */
											port_state_machine[i]=PEER_PORT_DISABLE;
										} else { /* other client */
											move_port_to(i+1, lan_device);
											port_state_machine[i]=LINKUP_STABLE;
										}
									} else
										port_state_machine_timeout[i]--;
								}
								break;
					case 	PEER_PORT_DISABLE:
								if ( tmp_shift_mask & change2_linkdown_mask ) { /* link down, isolate port */
									isolate_port(i+1);
									port_state_machine[i]=NOLINK_STATE;
									port_detected_ethbackl[i]=0;
								}
								break;
#endif /* RTCONFIG_QCA_ORG_UPDOWN_SEPARATE */
					default :
								break;
				}
			}
			for (i=0; i < MAX_WANLAN_PORT; i++) { /* handle power state */
				if (!port_power_down_count[i]) continue;
				port_power_down_count[i]--;
				if (!port_power_down_count[i]) {
					power_onoff_port(i+1, 1);
				}
			}

			reset_port=nvram_get_int("eth_bh_resetLAN");
			if (reset_port) {
				unsigned int reset_act = reset_port & 0xf;
				unsigned int port_id = (reset_port&0xf0)>>4;
				nvram_unset("eth_bh_resetLAN");
				switch (reset_act) {
					case DAEMON_CMD_LEAVE_ETH: // leave ETH
						if (port_id) {
							//port_power_down_count[port_id-1] = 2;
							//power_onoff_port(port_id, 0);
							port_detected_ethbackl[port_id-1] = 0;
						}
#if 0 /* move to other place */
						for (i=0; i < MAX_WANLAN_PORT; i++) {
							if ( role && port_detected_ethbackl[i] ) {
								port_power_down_count[i] = 4; /* reset related port */
								power_onoff_port(i+1, 0);
							}
							port_detected_ethbackl[i]=0;
						}
#endif
						break;
					case DAEMON_CMD_DISABLE_PORT: // disable specific port
					case DAEMON_CMD_DISABLE_PORT2: // disable specific port
						if (port_id) {
							isolate_port(port_id);
							port_state_machine[port_id-1] = PEER_PORT_DISABLE;
							if (reset_act == DAEMON_CMD_DISABLE_PORT)
								port_detected_ethbackl[port_id-1] = 3;
#ifdef VV_DEBUG
							_dprintf("<<State Changed>>: disable port[%d] from daemon\n", port_id);
#endif
						}
						break;
					case DAEMON_CMD_RESET_ALL: // reset all LAN port
						for (i=0; i < MAX_WANLAN_PORT; i++) { /* handle power down count */
							tmp_shift_mask = 1 << (i+1);
							if (!(tmp_shift_mask & all_portmask)) continue;
							power_onoff_port(i+1, 0);
							port_power_down_count[i] = port_id;
						}
						break;
					case DAEMON_CMD_ETH_PORT: // daemon enter ETH by this port
						if (port_id)
							port_detected_ethbackl[port_id-1] = 1;
						break;
					default:
						_dprintf("<<Invalid eth_bh_resetLAN>>: %x\n", reset_port);
						break;
				}
			}
			if (last_eth_backl != tmp_eth_backl)
				eth_backl_cnt = ETH_RESET_PORT_COUNTDOWN;
			last_eth_backl=tmp_eth_backl;
			if (eth_backl_cnt) {
				eth_backl_cnt--;
				if (eth_backl_cnt == 0) {
					int j;
					for (j=0;j<MAX_WANLAN_PORT;j++) {
						if ((tmp_eth_backl && port_detected_ethbackl[j] != 1) ||
						     (!tmp_eth_backl && (port_state_machine[j] == PEER_PORT_DISABLE && port_detected_ethbackl[j] == 3))) {
							port_power_down_count[j] = 2;
							power_onoff_port(j+1, 0);
							port_detected_ethbackl[j] = 0;
						}
					}
				} else {
					if (tmp_eth_backl && (eth_backl_cnt%2 == 0)) {
						int channel;
						doSystem("iwconfig %s ESSID \"\"", get_staifname(1));
						ifconfig(get_staifname(1), 0, NULL, NULL);
#ifdef RTCONFIG_DUAL_BACKHAUL
						doSystem("iwconfig %s ESSID \"\"", get_staifname(0));
						ifconfig(get_staifname(0), 0, NULL, NULL);
#endif
						kill_wifi_wpa_supplicant(-1);
						if (get_phyrate(1)==0) {
							ifconfig(get_wififname(1), 0, NULL, NULL);
							ifconfig(get_wififname(1), IFUP, NULL, NULL);
						}
						channel=atoi(nvram_safe_get("wl1_channel"));
						if (channel)
							doSystem("iwconfig %s channel %d", get_wififname(1), channel);
					}
				}
			}
			if (linkchange_cnt) {
				linkchange_cnt--;
				if (linkchange_cnt == 0) {
					 /* add workaround for legacy device */
					doSystem("echo 1 > /sys/class/net/br0/bridge/flush");
					doSystem("echo 1 > /sys/class/net/brg0/bridge/flush");
				}
			}
			if (role) { // RE, check my_dist
				if (!tmp_eth_backl && (last_my_dist < 255) && (my_dist == 255)) { /* upstream wifi is broken now */
					int j;
					for (j=0;j<MAX_WANLAN_PORT;j++) {
						port_power_down_count[j] = 2;
						power_onoff_port(j+1, 0);
						port_detected_ethbackl[j] = 0;
					}
				}
			}
			last_my_dist = my_dist;
		}
#ifdef	TRIBAND_CAPONLY_PREFER_HIGHBAND
		if (!role) { //CAP
			if (hb_count_down <= 0) {
				int count = get_re_count();
				if (count == 0) { /* single CAP case, enable wifi0, wifi2 NCB */
					doSystem("iwpriv wifi0 ncb_enable 5137"); /* 0x1411 */
					doSystem("iwpriv wifi2 ncb_enable 5137"); /* 0x1411 */
				} else { /* disable wifi0, wifi2 NCB */
					doSystem("iwpriv wifi0 ncb_enable 0");
					doSystem("iwpriv wifi2 ncb_enable 0");
				}
				hb_count_down = TRIBAND_HB_CHECK_PERIOD;
			} else
				hb_count_down--;
		}
#endif
		sleep(1);
	} /* end of while(1) */
}

static unsigned short cksum(unsigned char *buf, int sz)
{
	int sum = 0;
	unsigned short ans;
	unsigned short *w = (unsigned short *)buf;

	while (sz > 0) {
		sum += *w++;
		sz -= 2;
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return ans;
}

#if defined(MAPAC1750) /* 833x, controlled by swconfig  */
static unsigned int find_port(char *mac_str)
{
	char buf[128];
	doSystem("swconfig dev eth0 get dump_arl | grep %s | awk '{print $4}' > /tmp/ff.txt", mac_str);
        f_read_string("/tmp/ff.txt", buf, sizeof(buf));
	if ((buf[0]=='\0') || (buf[0]=='\n'))
		return 0;
	else {
		unsigned int mask, portno=0;
		mask = strtol(buf, NULL, 16);
		while (mask) {
			if (mask & 1)
				break;
			else
				mask >>= 1;
			portno++;
		}
		return portno;
	}
}
#else /* 807x, controlled by ssdk_sh */
static unsigned int find_port(char *mac_str)
{
	char buf[128];
	char *pt;
	doSystem("ssdk_sh fdb entry show | grep %s | awk '{print $4}' | sed '/:0/d' > /tmp/ff.txt", mac_str);
        f_read_string("/tmp/ff.txt", buf, sizeof(buf));
	pt = strstr(buf, "[dest_port]:");
	if (pt) {
		pt += strlen("[dest_port]:");
		return atoi(pt);
	}
	return 0;
}
#endif

#include <linux/if_packet.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <sys/timeb.h>
#define IP_HDR_SIZE		(sizeof(struct iphdr))
#define	UDP_HDR_SIZE		(sizeof(struct udphdr))
#define	ETH_HDR_SIZE		14
int ethbh_peer_detect(char *nic, char *timestr, char *msg)
{
	int ss;
	struct ifreq ifr;
	int ifindex, i, msglen;
	char out_buffer[EXCHANGE_BUF_LEN+ETH_HDR_SIZE+IP_HDR_SIZE+UDP_HDR_SIZE];
	char final_result[MAX_RECORD_ETH_CLIENT][EXCHANGE_BUF_LEN];
	int get_peer, timeout;
	char get_frame[1514];
	unsigned char exchange_magic[] = { 0xa2, 0x82, 0xf1, 0x90 };
	char *pt;
	time_t target_time;
	struct timeb tnow, tlast;
	unsigned short port_network_order;
	unsigned char SMAC[6];
	unsigned char BMAC[6];
	struct sockaddr_ll me;
	struct udphdr *udphdr_pt;
	struct iphdr *iphdr_pt;
	int found;
	char smacbuf[EXCHANGE_MAC_LEN+1];

	timeout = atoi(timestr);
	if (timeout)
		ss = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	else
		ss = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

	if (ss < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, nic, IFNAMSIZ-1);
	if (ioctl(ss, SIOCGIFINDEX, &ifr) < 0)
		return -2;

	ifindex = ifr.ifr_ifindex;
	if (ioctl(ss, SIOCGIFFLAGS, (char*)&ifr))
		return -3;

	if (timeout) {
		if (!(ifr.ifr_flags&IFF_UP)) {
			ifr.ifr_flags |= IFUP;
			if (ioctl(ss, SIOCSIFFLAGS, (char*)&ifr))
				return -4;
		}
	}

	if (timeout) {
		me.sll_family = AF_PACKET;
		me.sll_ifindex = ifindex;
		me.sll_protocol = htons(ETH_P_ALL);
		if (bind(ss, (struct sockaddr*)&me, sizeof(me)) == -1)
			return -5;
	} else {
		const int on=1;
		setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		setsockopt(ss, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

		if (setsockopt(ss, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0)
		return -5;
	}

        if (ioctl(ss, SIOCGIFHWADDR, &ifr))
		return -6;

	memcpy(SMAC, (const unsigned char *)ifr.ifr_hwaddr.sa_data, 6);

#if 0
	SMAC[0]=0x02; // locally administered
	SMAC[1]=0x03;
	SMAC[2]=0x22;
#endif
	memset(BMAC, 0xff, sizeof(BMAC));

	port_network_order = htons(EXCHANGE_PORT);

	memset(out_buffer, 0x00, sizeof(out_buffer));

	pt = &out_buffer[0];
	if (timeout) {
		memcpy(pt, BMAC, 6);
		memcpy(pt+6, SMAC, 6);
		pt[12]=0x08;
		pt+=ETH_HDR_SIZE;
	}
	iphdr_pt = (struct iphdr *)pt;
	iphdr_pt->version = 4;
	iphdr_pt->ihl = 5;
	if (timeout) {
		iphdr_pt->tot_len = htons(EXCHANGE_BUF_LEN+IP_HDR_SIZE+UDP_HDR_SIZE);
		iphdr_pt->id = 0x2312;
	}
	iphdr_pt->ttl = 5;
	iphdr_pt->protocol = 17;
	if (!timeout)
		iphdr_pt->saddr = inet_addr("169.254.0.0");
	iphdr_pt->daddr = -1;

	if (timeout)
		iphdr_pt->check = cksum(pt, IP_HDR_SIZE);
	pt+=IP_HDR_SIZE;

	udphdr_pt = (struct udphdr *)pt;
	udphdr_pt->source = port_network_order;
	udphdr_pt->dest = port_network_order;
	udphdr_pt->len = htons(EXCHANGE_BUF_LEN + sizeof(struct udphdr));
	pt+=UDP_HDR_SIZE;

	memcpy(pt, exchange_magic, sizeof(exchange_magic));
	pt+=sizeof(exchange_magic);
	msglen = strlen(msg); // should be the same with sizeof(exchange_message_t)
	memcpy(pt, msg, msglen);
	{
		exchange_message_pt mpt_tmp;
		mpt_tmp = (exchange_message_pt)pt;
		sprintf(smacbuf, "%02x%02x%02x%02x%02x%02x", SMAC[0], SMAC[1], SMAC[2], SMAC[3], SMAC[4], SMAC[5]);
		memcpy(mpt_tmp->SRC_MAC, smacbuf, EXCHANGE_MAC_LEN);
	}

	get_peer = 0;
	if (timeout) {
		ftime(&tnow);
		tlast=tnow;
		tlast.time-=1; /* trigger first packet */
		target_time = tnow.time + timeout;
		for ( i=0; i<MAX_RECORD_ETH_CLIENT; i++)
			final_result[i][0] = '\0';

		while ( tnow.time < target_time ) {
			if (((tnow.time-tlast.time)*1000 + tnow.millitm - tlast.millitm) > 300) {
				send(ss, out_buffer, sizeof(out_buffer), 0);
				tlast=tnow;
			}
			if (recv(ss, get_frame, sizeof(get_frame), MSG_DONTWAIT) > 0) {
				exchange_message_pt mpt1, mpt2;
				pt = &get_frame[0];
				found=0;
				if ((memcmp(pt, BMAC, sizeof(BMAC))==0) && (memcmp(pt+6, SMAC, sizeof(SMAC))!=0) && pt[12]==0x08) {
					pt += ETH_HDR_SIZE;
					iphdr_pt = (struct iphdr *)pt;
					pt+=IP_HDR_SIZE;
					udphdr_pt = (struct udphdr *)pt;
					if ((iphdr_pt->daddr == -1) && (udphdr_pt->dest == port_network_order) &&  (udphdr_pt->source == port_network_order)) {
						pt+=UDP_HDR_SIZE;
						if (memcmp(pt, exchange_magic, sizeof(exchange_magic))==0) { /* the same peer */
							pt += sizeof(exchange_magic);
							mpt2 = (exchange_message_pt)pt;
							if (memcmp(mpt2->SRC_MAC, smacbuf, EXCHANGE_MAC_LEN)==0) {
								; /* ignore loopback packet */
							} else
								found=1;
						}
					}
				}
				if (found) {
					int msg_type_offset;
					found=0;
					for (i=0; i<get_peer; i++) {
						if (memcmp(final_result[i], pt, msglen)==0) { // got the same client
							found=1;
							break;
						}
						mpt1 = (exchange_message_pt)&final_result[i][0];
						mpt2 = (exchange_message_pt)pt;
						msg_type_offset = &mpt2->msg_type - pt;
						if ((mpt1->role==mpt2->role) && (memcmp(&final_result[i][0]+msg_type_offset, pt+msg_type_offset, sizeof(exchange_message_t)-msg_type_offset)==0)) {
							mpt1->mode = mpt2->mode;
							found=1;
							break;
						}
					}
					if (!found) { /* add new record */
						if (get_peer<MAX_RECORD_ETH_CLIENT) {
							memcpy(final_result[i], pt, msglen);
							final_result[i][msglen] = '\0';
							get_peer++;
						}
					}
				}
			}
			ftime(&tnow);
		}

		/* down the interface */
		ifr.ifr_flags &= ~IFUP;
		ioctl(ss, SIOCSIFFLAGS, (char*)&ifr);
	} else { /* daemon */
		int tmp_eth_backl;
#ifndef RTCONFIG_ETHBH_MIXED_MODE
		int last_eth_backl;
#endif
		int role=get_role();
		fd_set rmask;
		struct timeval select_timeout;
#ifdef UDPBCAST_SOCKET_RCV
		struct timeb t_recv_last;
		unsigned int recv_last_diffmini;
		int recv_backoff;
#endif
		int select_ret, action, sendit;
#ifndef RTCONFIG_ETHBH_MIXED_MODE
		unsigned char CAP_MAC[EXCHANGE_MAC_LEN+1];
#endif
		exchange_message_pt mpt_out, mpt_in;
		daemon_record_pt record_pt;
		int s_un, max_select_fd;
#ifdef	UDPBCAST_SOCKET_RCV
		int sr;
#endif
		p2d_message_t p2d_buf;
		struct sockaddr_un uaddr;
#ifndef RTCONFIG_ETHBH_MIXED_MODE
		int rm_count, add_count;
#endif
		time_t uptime_now;
		unsigned int disabled_port;
		unsigned char src_mac_fdb[EXCHANGE_MAC_LEN+1+5]; /* format: 60-45-cb-68-74-fb */
#ifdef RTCONFIG_ETHBH_MIXED_MODE
		int my_dist=0, peer_dist=0, last_dist=0;
		int under_queue_eth=0;
		time_t trigger_eth_stamp=0, eth_queue_time=0;
#endif
		struct sockaddr_in addrs_to;

		bzero((char *) &addrs_to, sizeof(addrs_to));
		addrs_to.sin_addr.s_addr = inet_addr("255.255.255.255");
		addrs_to.sin_family = AF_INET;
		addrs_to.sin_port = port_network_order;

		signal(SIGCHLD, h_chld);
		unlink(UN_PATH_PORT_TO_DAEMON);
		if ((s_un=socket(AF_LOCAL,SOCK_DGRAM,0)) == -1) {
			goto err_out;
		} else {
			const int on=1;
			setsockopt(s_un, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		}

		memset ((char *)&uaddr,0,sizeof(uaddr));
		uaddr.sun_family=AF_LOCAL;
		strcpy(uaddr.sun_path, UN_PATH_PORT_TO_DAEMON);

		if ( bind(s_un,(struct sockaddr *)&uaddr,sizeof(uaddr)) == -1 ) {
			close(s_un);
			goto err_out;
		}

#ifdef	UDPBCAST_SOCKET_RCV
		if ((sr = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
			goto err_out;
		} else {
			const int on=1;
			struct sockaddr_in addrs;
			setsockopt(sr, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
			//setsockopt(sr, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

			bzero((char *) &addrs, sizeof(addrs));
			addrs.sin_addr.s_addr = inet_addr("255.255.255.255");
			addrs.sin_family = AF_INET;
			addrs.sin_port = port_network_order;
			if (bind(sr, (struct sockaddr *)&addrs,sizeof(addrs)) < 0) {
				_dprintf("%s:%d bind fail!!!\n", __func__, __LINE__);
				goto err_out;
			}
		}
#endif
		mpt_out = (exchange_message_pt)pt;

#ifndef RTCONFIG_ETHBH_MIXED_MODE
		last_eth_backl=-1;
#endif
		ftime(&tnow);
		tlast=tnow;
#ifdef UDPBCAST_SOCKET_RCV
		t_recv_last=tnow;
		recv_backoff=0;
#endif
		tlast.time -= EXCHANGE_DAEMON_PERIOD; /* trigger first packet */
		// init
		for ( i=0; i<MAX_RECORD_ETH_CLIENT; i++) {
			record_pt = (daemon_record_pt)final_result[i];
			record_pt->is_used = 0;
			record_pt->from_port = 255;
		}
#ifdef	UDPBCAST_SOCKET_RCV
		max_select_fd = (s_un > sr)? s_un:sr;
#else
		max_select_fd = (s_un > ss)? s_un:ss;
#endif
		max_select_fd += 1;
		while (1) {
			FD_ZERO(&rmask);
#ifdef	UDPBCAST_SOCKET_RCV
			FD_SET(sr, &rmask);
#else
			FD_SET(ss, &rmask);
#endif
			FD_SET(s_un, &rmask);
			select_timeout.tv_sec = EXCHANGE_DAEMON_PERIOD;
			select_timeout.tv_usec = 0;
			select_ret=select(max_select_fd, &rmask, NULL, NULL, &select_timeout);
			tmp_eth_backl=nvram_get_int("eth_backl");
			ftime(&tnow);
			uptime_now = uptime();
			found = 0;
			action = 0;
#ifndef RTCONFIG_ETHBH_MIXED_MODE
			rm_count = 0;
			add_count = 0;
#endif
			if (select_ret > 0) {
#ifdef	UDPBCAST_SOCKET_RCV
				if ( FD_ISSET(sr, &rmask) )
#else
				if ( FD_ISSET(ss, &rmask) )
#endif
				{
					// retrieve packet
#ifdef	UDPBCAST_SOCKET_RCV
					if (recv(sr, get_frame, sizeof(get_frame), MSG_DONTWAIT) > 0)
#else
					if (recv(ss, get_frame, sizeof(get_frame), MSG_DONTWAIT) > 0)
#endif
					{
						pt = &get_frame[0];

#ifndef	UDPBCAST_SOCKET_RCV
						if ((memcmp(pt, BMAC, sizeof(BMAC))==0) && (memcmp(pt+6, SMAC, sizeof(SMAC))!=0) && pt[12]==0x08) {
							pt += ETH_HDR_SIZE;
							iphdr_pt = (struct iphdr *)pt;
							pt+=IP_HDR_SIZE;
							udphdr_pt = (struct udphdr *)pt;
							if ((iphdr_pt->daddr == -1) && (udphdr_pt->dest == port_network_order) &&  (udphdr_pt->source == port_network_order)) {
								pt+=UDP_HDR_SIZE;
#endif
								if (memcmp(pt, exchange_magic, sizeof(exchange_magic))==0) { /* the same peer */
									exchange_message_pt mpt_tmp;
									pt += sizeof(exchange_magic);
									mpt_tmp = (exchange_message_pt)pt;
									if ( memcmp(mpt_tmp->WIFI1_MAC, mpt_out->WIFI1_MAC, EXCHANGE_MAC_LEN) == 0) {
									} else
										found=1;
								}
#ifndef	UDPBCAST_SOCKET_RCV
							}
						}
#endif

						if (found) {
							mpt_in = (exchange_message_pt)pt;
							for ( i=0; i<6; i++) {
								src_mac_fdb[i*3] = mpt_in->SRC_MAC[i*2];
								src_mac_fdb[i*3+1] = mpt_in->SRC_MAC[i*2+1];
								if (i != 5)
#if defined(MAPAC1750)
									src_mac_fdb[i*3+2] = ':';
#else
									src_mac_fdb[i*3+2] = '-';
#endif
								else
									src_mac_fdb[i*3+2] = '\0';
							}
							mpt_in->dist_str[sizeof(mpt_in->dist_str)-1]='\0';
							peer_dist=atoi(mpt_in->dist_str);
#ifdef UDPBCAST_SOCKET_RCV
							found = 0;
							if (tmp_eth_backl) {
								/* we can retrieve which port the packet is coming to enhance the check */
								record_pt = (daemon_record_pt)final_result[0];
								if ((record_pt->is_used) && (memcmp(record_pt->WIFI1_MAC, mpt_in->WIFI1_MAC, EXCHANGE_MAC_LEN)==0))
									found = 1;
							}
							if (!found) {
								if (nvram_get_int("eth_under_detmask")) // special check
									recv_backoff = 0;
								else {
									/* back off mechanism for delayed port link detection */
									recv_last_diffmini = (tnow.time-t_recv_last.time)*1000 + tnow.millitm - t_recv_last.millitm;
									if (!recv_backoff) {
										if ( recv_last_diffmini > (EXCHANGE_DAEMON_PERIOD * 2 * 1000) ) {
											/* 1st */
											t_recv_last=tnow;
											recv_backoff = 1;
											//_dprintf("VVVVVVVVVVVVV[enter recv_backoff!]\n");
										} else {
											/* 4th & normal */
											t_recv_last=tnow;
											found = 1;
										}
									} else {
										if ( recv_last_diffmini >= 1000 ) {
											/* 2nd */
											recv_backoff = 0;
											t_recv_last=tnow;
											found = 1;
											//_dprintf("VVVVVVVVVVVVV[leave recv_backoff!]\n");
										} else {
											/* 3rd */
											//_dprintf("VVVVVVVVVVVVV[skip under recv_backoff!]\n");
										}
									}
								}
							}
#endif
						}
					}
				}

				if ( FD_ISSET(s_un, &rmask) ) {
					if (recv(s_un, &p2d_buf, sizeof(p2d_buf), 0) > 0) {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
						if (p2d_buf.action == 'E')
#else
						if ((p2d_buf.action == 'E') || (!role && p2d_buf.action == 'A'))
#endif
						{
#ifdef RTCONFIG_ETHBH_MIXED_MODE
							record_pt = (daemon_record_pt)final_result[0];
							record_pt->is_used = 1;
							record_pt->from_port = p2d_buf.port_no;
							record_pt->distance = p2d_buf.distance;
							record_pt->update_time = uptime_now + HIVE_ETH_SCRIPT_TIMEOUT; // time stamp in futhur;
							memcpy(record_pt->WIFI1_MAC, mpt_in->WIFI1_MAC, EXCHANGE_MAC_LEN);
							record_pt->WIFI1_MAC[EXCHANGE_MAC_LEN]='\0';
							action |= EXCHANGE_DAEMON_ACT_ENTETH_L;
#else
							for ( i=0; i<MAX_RECORD_ETH_CLIENT; i++) {
								record_pt = (daemon_record_pt)final_result[i];
								if (record_pt->is_used) continue;
								record_pt->is_used = 1;
								record_pt->from_port = p2d_buf.port_no;
								record_pt->update_time = uptime_now + HIVE_ETH_SCRIPT_TIMEOUT; // time stamp in futhur
								memcpy(record_pt->WIFI1_MAC, p2d_buf.WIFI1_MAC, EXCHANGE_MAC_LEN);
								record_pt->WIFI1_MAC[EXCHANGE_MAC_LEN]='\0';
								add_count++;
								if (p2d_buf.action == 'E')
									action |= EXCHANGE_DAEMON_ACT_ENTETH_L;
								break;
							}
#endif
						} else if (p2d_buf.action == 'D') {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
							record_pt = (daemon_record_pt)final_result[0];
							record_pt->is_used=0;
							action |= EXCHANGE_DAEMON_ACT_LEXIT_ETH;
#ifdef VV_DEBUG
							_dprintf("<<LINK ETH BACKL>>: RE leave backhaul at port L:[%d]R:[%d]!\n", p2d_buf.port_no, record_pt->from_port);
#endif
#else
							for ( i=0; i<MAX_RECORD_ETH_CLIENT; i++) {
								record_pt = (daemon_record_pt)final_result[i];
								if (!record_pt->is_used) continue;
								if ((memcmp(record_pt->WIFI1_MAC, p2d_buf.WIFI1_MAC, EXCHANGE_MAC_LEN)==0) && (record_pt->from_port == p2d_buf.port_no)) {
									record_pt->is_used=0;
									rm_count++;
									if (role) {
										action |= EXCHANGE_DAEMON_ACT_LEXIT_ETH;
#ifdef VV_DEBUG
										_dprintf("<<LINK ETH BACKL>>: RE leave backhaul!\n");
#endif
									}
									break;
								}
							}
#endif
						}
					}
				}
			}
			if (!nvram_get_int("wlready"))
				continue;
			disabled_port = 0;
			if (!role) { // CAP
				if (found) {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
					if (memcmp(mpt_in->group_key, mpt_out->group_key, EXCHANGE_GROUP_LEN)==0)
						action |= EXCHANGE_DAEMON_ACT_SENDOUT;
#else
					if (tmp_eth_backl) {
						if (mpt_in->msg_type == 'L')
							action |= EXCHANGE_DAEMON_ACT_SENDOUT;
						if (memcmp(mpt_in->group_key, mpt_out->group_key, EXCHANGE_GROUP_LEN)==0) {
							if (mpt_in->role == 'R') {
								if (mpt_in->msg_type == 'D')
									action |= EXCHANGE_DAEMON_ACT_UPDATE;
							}
							else if (mpt_in->role == 'C') {
								action |= EXCHANGE_DAEMON_ACT_DIS_PORT2;
								disabled_port = find_port(src_mac_fdb);
							}
						}
					} else {
						if (mpt_in->msg_type == 'L')
							action |= EXCHANGE_DAEMON_ACT_SENDOUT;
						if (memcmp(mpt_in->group_key, mpt_out->group_key, EXCHANGE_GROUP_LEN)==0) {
							if (mpt_in->role == 'R')
								action |= EXCHANGE_DAEMON_ACT_ENTETH_D;
							else if (mpt_in->role == 'C') {
								action |= EXCHANGE_DAEMON_ACT_DIS_PORT2;
								disabled_port = find_port(src_mac_fdb);
							}
						} else {
							if ((mpt_in->msg_type == 'D') && (mpt_in->role == 'C')) {
								action |= EXCHANGE_DAEMON_ACT_DIS_PORT2;
								disabled_port = find_port(src_mac_fdb);
							}
						}
					}
#endif
				}
			} else { // RE
#ifndef RTCONFIG_ETHBH_MIXED_MODE
				if (tmp_eth_backl != last_eth_backl) {
					strcpy(CAP_MAC, "000000000000");
					if (tmp_eth_backl) {
						if (nvram_get("eth_bh_CAPMAC"))
							strcpy(mpt_out->CAP_MAC, nvram_get("eth_bh_CAPMAC"));
					}
					memcpy(mpt_out->CAP_MAC, CAP_MAC, EXCHANGE_MAC_LEN);
				}
#endif
#ifdef RTCONFIG_ETHBH_MIXED_MODE
				my_dist=get_my_dist();
				if (my_dist!=last_dist) {
					sprintf(mpt_out->dist_str, "%03d", my_dist);
					mpt_out->dist_str[3]='S';
					last_dist=my_dist;
				}
#endif
				if (found) {
					if (tmp_eth_backl) {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
						if (memcmp(mpt_in->group_key, mpt_out->group_key, EXCHANGE_GROUP_LEN)!=0) {
							action |= EXCHANGE_DAEMON_ACT_DIS_PORT2;
							disabled_port = find_port(src_mac_fdb);
						}
						else {
							if (mpt_in->msg_type == 'L')
								; // ignore Link message when we are in eth_backl mode, parent node(CAR or ETH root RE) will handle it
							else if (mpt_in->msg_type == 'D')
								action |= EXCHANGE_DAEMON_ACT_UPDATE;
							record_pt = (daemon_record_pt)final_result[0];
							if ((record_pt->is_used) && (memcmp(record_pt->WIFI1_MAC, mpt_in->WIFI1_MAC, EXCHANGE_MAC_LEN)==0)) {
								// check if our parent's distance become 255
								if (peer_dist == 255) {
#ifdef VV_DEBUG
									_dprintf("<<DAEMON ETH BACKL>>: RE leave backhaul!(parent)\n");
#endif
									action |= EXCHANGE_DAEMON_ACT_DEXIT_ETH;
								}
							}
						}
#else
						if ((mpt_in->role == 'C') && (mpt_in->msg_type == 'D') && (memcmp(mpt_in->group_key, mpt_out->group_key, EXCHANGE_GROUP_LEN)==0))
							action |= EXCHANGE_DAEMON_ACT_UPDATE;
#endif
					} else {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
						int go_eth=0;
						action |= EXCHANGE_DAEMON_ACT_SENDOUT;
						if (memcmp(mpt_in->group_key, mpt_out->group_key, EXCHANGE_GROUP_LEN)!=0) {
							action |= EXCHANGE_DAEMON_ACT_DIS_PORT2;
							disabled_port = find_port(src_mac_fdb);
						}
						else { // (mpt_in->msg_type == 'L') or (mpt_in->msg_type == 'D')
							if (mpt_in->role == 'C')
								go_eth=1;
							else { /* peer is RE */
								if (mpt_in->mode=='N') {
									if (my_dist == peer_dist) {
										disabled_port = find_port(src_mac_fdb);
										_dprintf("<<Daemon>>: disable NonETH RE with same dist:%s, disabled_port:%d\n", mpt_in->dist_str, disabled_port);
										if (disabled_port == 0 && mpt_in->msg_type == 'D')
											action |= EXCHANGE_DAEMON_ACT_RESET_ALL;
										else
											action |= EXCHANGE_DAEMON_ACT_DIS_PORT;
									} else if (my_dist > peer_dist) { /* we got another short-cut */
										if (strcmp(mpt_out->WIFI1_MAC, mpt_in->CAP_MAC)==0) {
											disabled_port = find_port(src_mac_fdb);
											_dprintf("<<Daemon>>: Loop detection on port[%d], with dist:%d, mydist:%d, disable it!\n", disabled_port, peer_dist, my_dist);
											if (disabled_port == 0 && mpt_in->msg_type == 'D')
												action |= EXCHANGE_DAEMON_ACT_RESET_ALL;
											else
												action |= EXCHANGE_DAEMON_ACT_DIS_PORT;
										} else {
											_dprintf("<<Daemon>>: detect another NonETH RE with dist:%d, mydist:%d, enable ETH\n", peer_dist, my_dist);
											go_eth=1;
										}
									} else { /* we become upstream node */
										//TBD. accroding to Eason's re_eth_backl  maybe we need to down/up LAN nic here
									}
								} else if (mpt_in->mode=='E') {
									if ((peer_dist == 1) || (my_dist > peer_dist)) { /* we got another short-cut */
										// if peer_dist is 1, we should connect to CAP directly after negotiation
										_dprintf("<<Daemon>>:  enter ETH BACKL, mydist:%d, peer_dist:%d\n", my_dist, peer_dist);
										go_eth=1; /* we will update CAPMAC after following D meesage */
									} else {
										_dprintf("<<Daemon>>: disable ETH RE with mydist:%d, peer_dist:%d\n", my_dist, peer_dist);
										disabled_port = find_port(src_mac_fdb);
										if (disabled_port == 0 && mpt_in->msg_type == 'D')
											action |= EXCHANGE_DAEMON_ACT_RESET_ALL;
										else
											action |= EXCHANGE_DAEMON_ACT_DIS_PORT;
									}
								}
							}
						}
						if (go_eth) {
							if (uptime_now - trigger_eth_stamp > HIVE_ETH_SCRIPT_TIMEOUT) {
								action |= EXCHANGE_DAEMON_ACT_ENTETH_D;
								if (mpt_in->role=='C')
									nvram_set_int("eth_bh_up_cost", 1);
								else {
									if (mpt_in->mode=='E')
										nvram_set_int("eth_bh_up_cost", peer_dist);
									else
										nvram_set_int("eth_bh_up_cost", peer_dist+1);
								}
								nvram_set("mesh_dist", nvram_get("eth_bh_up_cost"));
								trigger_eth_stamp = uptime_now;
							} else {
								_dprintf("<<Daemon>>:  Already under Trans-ETH process...\n");
							}
						}
#else
						if (memcmp(mpt_in->group_key, mpt_out->group_key, EXCHANGE_GROUP_LEN)==0) {
							if ((mpt_in->role == 'C') || (mpt_in->mode == 'E')) /* got some new route */
								action |= EXCHANGE_DAEMON_ACT_RESET_ALL; /* we should enter ethbackl after reset */
							else if (mpt_in->msg_type = 'L')
								action |= EXCHANGE_DAEMON_ACT_BANG_PEER;
							else
								; /* if we can distinguish this message came from which port, disable it here! */
						}
#endif
					}
				}
			} // end of RE
			sendit=0;
			if ((action & EXCHANGE_DAEMON_ACT_SENDOUT) || (action & EXCHANGE_DAEMON_ACT_BANG_PEER))
				sendit=1;
#ifdef RTCONFIG_ETHBH_MIXED_MODE
			else if ( (!role)/*CAP*/ || !tmp_eth_backl/*none ETH*/ )
#else
			else if ( (!role)/*CAP*/ || tmp_eth_backl/*RE in ETH*/ )
#endif
			{
				if (((tnow.time-tlast.time)*1000 + tnow.millitm - tlast.millitm) >= (EXCHANGE_DAEMON_PERIOD * 1000))
					sendit=1;
			}
			if ( sendit ) {
				if (tmp_eth_backl) {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
					char *CAP_MAC= nvram_get("eth_bh_CAPMAC");
					if (CAP_MAC)
						memcpy(mpt_out->CAP_MAC, CAP_MAC, EXCHANGE_MAC_LEN);
					else
						memcpy(mpt_out->CAP_MAC, "000000000000", EXCHANGE_MAC_LEN);
#endif
					mpt_out->mode='E';
				}
				else {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
					char *CAP_MAC= nvram_get("mesh_up_sta");
					if (CAP_MAC)
						memcpy(mpt_out->CAP_MAC, CAP_MAC, EXCHANGE_MAC_LEN);
					else
						memcpy(mpt_out->CAP_MAC, "000000000000", EXCHANGE_MAC_LEN);
#endif
					mpt_out->mode='N';
				}
				sendto(ss, out_buffer, sizeof(out_buffer), 0, (const struct sockaddr *)&addrs_to, sizeof(addrs_to));
				tlast=tnow;
			}

			if (action & EXCHANGE_DAEMON_ACT_ENTETH_D) {
				action |= EXCHANGE_DAEMON_ACT_UPDATE;
			}

			if (action & EXCHANGE_DAEMON_ACT_UPDATE) {
#ifdef RTCONFIG_ETHBH_MIXED_MODE
				found = 0;
				record_pt = (daemon_record_pt)final_result[0];
				if (record_pt->is_used) {
					if (memcmp(record_pt->WIFI1_MAC, mpt_in->WIFI1_MAC, EXCHANGE_MAC_LEN)==0) {
						record_pt->update_time = uptime_now;
						found = 1;
					} else {
						if (peer_dist > record_pt->distance)
							found = 1;
					}
				}
				if (found == 0) { /* update record */
					unsigned int upstream_port;
					upstream_port = find_port(src_mac_fdb);
					record_pt->is_used = 1;
					if (upstream_port)
						record_pt->from_port = upstream_port;
					else
						record_pt->from_port = 255;
					record_pt->distance = peer_dist;
					record_pt->update_time = uptime_now;
					memcpy(record_pt->WIFI1_MAC, mpt_in->WIFI1_MAC, EXCHANGE_MAC_LEN);
					record_pt->WIFI1_MAC[EXCHANGE_MAC_LEN]='\0';
					// update distance
					nvram_set_int("eth_bh_up_cost", peer_dist+1);
					nvram_set("mesh_dist", nvram_get("eth_bh_up_cost"));
				}
#else
				int first_empty = -1;
				found = 0;
				for ( i=0; i<MAX_RECORD_ETH_CLIENT; i++) {
					record_pt = (daemon_record_pt)final_result[i];
					if (record_pt->is_used) {
						if (memcmp(record_pt->WIFI1_MAC, mpt_in->WIFI1_MAC, EXCHANGE_MAC_LEN)==0) {
							record_pt->update_time = uptime_now;
							found = 1;
							break;
						}
					} else {
						if (first_empty == -1)
							first_empty = i;
					}
				}
				if (!found) {
					if (first_empty != -1) {
						record_pt = (daemon_record_pt)final_result[first_empty];
						record_pt->is_used = 1;
						record_pt->from_port = 255;
						record_pt->update_time = uptime_now;
						memcpy(record_pt->WIFI1_MAC, mpt_in->WIFI1_MAC, EXCHANGE_MAC_LEN);
						record_pt->WIFI1_MAC[EXCHANGE_MAC_LEN]='\0';
						add_count++;
					}
				}
#endif
			}

			if (action & EXCHANGE_DAEMON_ACT_DIS_PORT) {
				unsigned int val = (disabled_port<<4) | DAEMON_CMD_DISABLE_PORT;
				nvram_set_int("eth_bh_resetLAN", val);
				/* add two message here for more assurance before port disable*/
				if (disabled_port) {
					send(ss, out_buffer, sizeof(out_buffer), 0);
					send(ss, out_buffer, sizeof(out_buffer), 0);
				}
			}

			if (action & EXCHANGE_DAEMON_ACT_DIS_PORT2) {
				unsigned int val = (disabled_port<<4) | DAEMON_CMD_DISABLE_PORT2;
				nvram_set_int("eth_bh_resetLAN", val);
				/* add two message here for more assurance before port disable*/
				if (disabled_port) {
					send(ss, out_buffer, sizeof(out_buffer), 0);
					send(ss, out_buffer, sizeof(out_buffer), 0);
				}
			}

			if (action & EXCHANGE_DAEMON_ACT_RESET_ALL) {
				unsigned int val = ( 3/* reset wait */<<4) | DAEMON_CMD_RESET_ALL;
				nvram_set_int("eth_bh_resetLAN", val);
			}

			if (!role) { /* CAP */
#ifndef RTCONFIG_ETHBH_MIXED_MODE
/* CAP & root node won't maintain neighbor list in mixed mode */
				int count=0;
				for ( i=0; i<MAX_RECORD_ETH_CLIENT; i++) {
					record_pt = (daemon_record_pt)final_result[i];
					if (!record_pt->is_used) continue;
					if (record_pt->update_time+EXCHANGE_PEER_DIE_TIMEOUT < uptime_now) { /* RE timeout */
						record_pt->is_used=0;
						rm_count++;
					} else
						count++;
				}
				if ((add_count) || (rm_count)) {
					if (!(action & (EXCHANGE_DAEMON_ACT_ENTETH_L | EXCHANGE_DAEMON_ACT_ENTETH_D)))
						doSystem("killall -9 hyd");
				} else if ((count==0) && (tmp_eth_backl)) { /* leave ETH backl */
#ifdef VV_DEBUG
					_dprintf("<<DAEMON ETH BACKL>>: CAP leave backhaul!\n");
#endif
					action |= EXCHANGE_DAEMON_ACT_DEXIT_ETH;
				}
#endif
			} else { /* RE */
				/* check CAP timeout */
				record_pt = (daemon_record_pt)final_result[0];
				if ((record_pt->is_used) && (record_pt->update_time+EXCHANGE_PEER_DIE_TIMEOUT < uptime_now)) { /* CAP timeout */
					record_pt->is_used=0;
					if (tmp_eth_backl) {
#ifdef VV_DEBUG
						_dprintf("<<DAEMON ETH BACKL>>: RE leave backhaul!(timeout)\n");
#endif
						action |= EXCHANGE_DAEMON_ACT_DEXIT_ETH;
					}
				}
			}

#ifdef RTCONFIG_ETHBH_MIXED_MODE
			if (under_queue_eth && ((uptime_now - eth_queue_time) > (HIVE_ETH_SCRIPT_TIMEOUT*2))) {
				if (nvram_get_int("eth_queue_cmd")) {
					nvram_unset("eth_queue_cmd");
					if (under_queue_eth == 3)
						action |= EXCHANGE_DAEMON_ACT_DEXIT_ETH;
					else if (under_queue_eth == 4)
						action |= EXCHANGE_DAEMON_ACT_ENTETH_D;
					under_queue_eth = 0;
				}
			}
#endif
			if ((action & EXCHANGE_DAEMON_ACT_LEXIT_ETH) || (action & EXCHANGE_DAEMON_ACT_DEXIT_ETH)) {
				int queue_cmd;
				if ((action & EXCHANGE_DAEMON_ACT_ENTETH_L) || (action & EXCHANGE_DAEMON_ACT_ENTETH_D))
					_dprintf("<<DAEMON ETH ERR>>: action is [%08x]\n", action);
				set_hops_count(255); // force everything to reset
				nvram_set("mesh_up_sta", "000000000000");
				nvram_set_int("eth_bh_resetLAN", DAEMON_CMD_LEAVE_ETH);
				queue_cmd = nvram_get_int("eth_queue_cmd");
				if ( queue_cmd == 0 ) {
					pid_t pid;
					char *ent_eth[]={"hive_eth","0",NULL};

#ifdef RTCONFIG_ETHBH_MIXED_MODE
					under_queue_eth = 0;
#endif
					nvram_set("eth_backl","0");
					tmp_eth_backl=0;
					nvram_set("eth_queue_cmd", "1");
					_eval(ent_eth, NULL, 0, &pid);
				} else {
#ifdef VV_DEBUG
					_dprintf("<<DAEMON ETH BACKL>>: QUEUE to leave ETH BACKL1, last cmd[%d]\n", queue_cmd);
#endif
					nvram_set("eth_queue_cmd", "3"); /* queue */
#ifdef RTCONFIG_ETHBH_MIXED_MODE
					under_queue_eth = 3;
					eth_queue_time = uptime_now;
#endif
				}
			} else if ((action & EXCHANGE_DAEMON_ACT_ENTETH_L) || (action & EXCHANGE_DAEMON_ACT_ENTETH_D)) {
				int queue_cmd;
				unsigned int upstream_port, val;

				if (action & EXCHANGE_DAEMON_ACT_ENTETH_L)
					upstream_port = p2d_buf.port_no;
				else
					upstream_port = find_port(src_mac_fdb);
				if (action & EXCHANGE_DAEMON_ACT_ENTETH_D) {
					val = (upstream_port<<4) | DAEMON_CMD_ETH_PORT;
					nvram_set_int("eth_bh_resetLAN", val);
				}
#ifdef VV_DEBUG
#ifdef RTCONFIG_ETHBH_MIXED_MODE
				_dprintf("<<DAEMON ETH BACKL>>: enter ETH backhaul[%04x], at port[%d]!\n", action, upstream_port);
#else
				_dprintf("<<DAEMON ETH BACKL>>: enter ETH backhaul[%04x]!\n", action);
#endif
#endif
				queue_cmd = nvram_get_int("eth_queue_cmd");
				if ( queue_cmd == 0 ) {
					pid_t pid;
					char *ent_eth[]={"hive_eth","1",NULL};

#ifdef RTCONFIG_ETHBH_MIXED_MODE
					under_queue_eth = 0;
#endif
					nvram_set("eth_backl","1");
					set_hops_count(-1);
					tmp_eth_backl=1;
					nvram_set("eth_queue_cmd", "2");
					_eval(ent_eth, NULL, 0, &pid);
				} else {
#ifdef VV_DEBUG
					_dprintf("<<DAEMON ETH BACKL>>: QUEUE to enter ETH\n");
#endif
					nvram_set("eth_queue_cmd", "4"); /* queue */
#ifdef RTCONFIG_ETHBH_MIXED_MODE
					under_queue_eth = 4;
					eth_queue_time = uptime_now;
#endif
				}
				//nvram_set_int("prelink_pap_status", -1); // trigger LED to change
			}
#ifndef RTCONFIG_ETHBH_MIXED_MODE
			last_eth_backl=tmp_eth_backl;
#endif
		} // end of while(1)
	}

err_out:
	close(ss);

	if (timeout) {
		sprintf(get_frame, "/tmp/"DETECT_PEER_RESULT_NAME, nic);
		unlink(get_frame);
		if (get_peer) {
			FILE *fp=fopen(get_frame, "wb");
			if (fp) {
				for (i=0; i<get_peer; i++)
					fprintf(fp,"%s\n", final_result[i]);
				fclose(fp);
			}
		}
	}
	return 0;
}
#endif

//for daisy chain
int sta_is_assoc(int band)
{
        char buf[1024];
        FILE *fp;
        int len,ch;
        char *pt1,*temp=NULL,*iface=NULL;
	char tmp[128],prefix_wl[]="wlxxxx_";

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
					int ath1, sta1;

					ath1 = get_channel(get_wififname(band));
					sta1 = get_channel(get_staifname(band));

					if(ath1 <= 0 || sta1 <= 0)
					{
						logmessage((char *)__func__, "RE: get_channel error. band(%d) wififname(%s): %d staifname(%s): %d\n", band, get_wififname(band), ath1, get_staifname(band), sta1);
						return 0;
					}

					if(ch!=0xff && ch != sta1)
					{
						sprintf(tmp,"wl%d_channel",band);
						nvram_set_int(tmp, sta1);
					}

					if(ath1 != sta1)
					{
						logmessage((char *)__func__, "RE: channel conflict on band(%d) wififname(%s): %d staifname(%s): %d. fix it.\n", band, get_wififname(band), ath1, get_staifname(band), sta1);
						iface = nvram_safe_get(strcat_r(prefix_wl, "ifname", tmp));
						doSystem("iwconfig %s channel %d",iface, sta1);
						_dprintf("RE: channel conflict on band(%d) wififname(%s): %d staifname(%s): %d. fix it.\n", band, get_wififname(band), ath1, get_staifname(band), sta1);
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
		_dprintf("=> RE: %s's distance from CAP is=%d\n",get_staifname(band),dist?atoi(dist):255);
	
	if(dist==NULL)
		return 255;
	else
        	return atoi(dist);
}

//return 0 if current bssid is the same with new bssid 
int diff_current_bssid(int band,char *new_mac)
{
  	char now_mac[18];
	int i,diff;
        //char tmp[128];
	//char *now_mac=NULL;
       // char prefix_mssid[] = "wlXXXXXXXXXX_mssid_";
	//snprintf(prefix_mssid, sizeof(prefix_mssid), "wl%d_", band);
	//now_mac = nvram_safe_get(strcat_r(prefix_mssid, "sta_bssid", tmp));
	//if (now_mac && strlen(now_mac))
 	get_mac(get_staifname(band),now_mac);
	if (strcmp(now_mac,"00:00:00:00:00:00")!=0)
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


int get_mac(char *ifname,char *mac)
{
	struct iwreq wrq;
	int ret;
	if((ret = get_ap_mac(ifname, &wrq)) < 0)
        	return ret;
	sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",
	wrq.u.ap_addr.sa_data[0], wrq.u.ap_addr.sa_data[1], wrq.u.ap_addr.sa_data[2]
	, wrq.u.ap_addr.sa_data[3], wrq.u.ap_addr.sa_data[4], wrq.u.ap_addr.sa_data[5]);
/*
_dprintf("!!!!!!!!%s!!!!!!!ap_addr(%02x:%02x:%02x:%02x:%02x:%02x)\n",ifname
, wrq.u.ap_addr.sa_data[0], wrq.u.ap_addr.sa_data[1], wrq.u.ap_addr.sa_data[2]
, wrq.u.ap_addr.sa_data[3], wrq.u.ap_addr.sa_data[4], wrq.u.ap_addr.sa_data[5]);
*/
	return 1;
}


//set new bssid by wpa_cli tools.
void wpa_cli_set_bssid(int band,char *mac)
{
	if(chk_assoc(get_staifname(band))==0)  
		goto wpaset;
	else
	{
		if(diff_current_bssid(band,mac))
			goto wpaset;
		else 
			return;
	}

wpaset:	
	//Restart the network with configured BSSID
	doSystem("wpa_cli -p /var/run/wpa_supplicant-sta%d disable_network 0",band);
	doSystem("wpa_cli -p /var/run/wpa_supplicant-sta%d set_network 0 bssid %s",band,mac);
	doSystem("wpa_cli -p /var/run/wpa_supplicant-sta%d enable_network 0",band);
	sleep(6);
	_dprintf("=> RE: wpacli set sta%d's bssid as %s\n",band,mac);
	
	return;
}


int find_pap_bssid(int band,int config)
{
	char tmp[20];
#ifdef RTCONFIG_ETHBACKHAUL
	int i;
	unsigned char up_tmp[EXCHANGE_MAC_LEN+5+1], up_mac[EXCHANGE_MAC_LEN+1];
#endif
	doSystem("iwpriv %s sendprobereq 1",get_staifname(band));
	nvram_set("sta_bssid_tmp","00:00:00:00:00:00"); //reset 
	if(band)
		nvram_set("sta_bssid_bk","00:00:00:00:00:00"); //reset 
	unlink(AP_SCAN_FILE);
	doSystem("iwpriv %s scan_son 1", band ? VPHY_5G : VPHY_2G);
	getSiteSurvey(band,AP_SCAN_FILE); //scan band and update sta_bssid_tmp
	doSystem("iwpriv %s scan_son 0", band ? VPHY_5G : VPHY_2G);
	_dprintf("=>sta_bssid_tmp=%s\n",nvram_safe_get("sta_bssid_tmp"));
	if(strcmp(nvram_safe_get("sta_bssid_tmp"),"00:00:00:00:00:00")!=0)
	{
		_dprintf("=> get pap's %s bssid\n",band?"5G":"2G");
		if(config)
		{
			wpa_cli_set_bssid(band,nvram_safe_get("sta_bssid_tmp"));
			sprintf(tmp,"wl%d_sta_bssid",band);
			nvram_set(tmp,nvram_safe_get("sta_bssid_tmp"));
#ifdef RTCONFIG_ETHBACKHAUL
			if(sw_mode() != SW_MODE_REPEATER) {
				strcpy(up_tmp, nvram_safe_get("sta_bssid_tmp"));
				for ( i=0; i<6; i++) {
					up_mac[i*2] = up_tmp[i*3];
					up_mac[i*2+1] = up_tmp[i*3+1];
				}
				up_mac[EXCHANGE_MAC_LEN]='\0';
				nvram_set("mesh_up_sta", up_mac);
			}
#endif
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
int find_pap_bssid_and_config(band)
{
        char buf[1024];
        FILE *fp;
        int len,i;
        char *pt1,*pt2=NULL;
	char mac[17];

        sprintf(buf, "iwpriv sta%d g_whc_ob_bssid",band?0:1);

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
	else
		return 0;

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
			_dprintf("=> RE: can not resolve 2G bssid ! disable 2g..\n");
			doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 disable_network 0");
		}
		return 0;
	}	

	_dprintf("=> RE: get %dG bssid=%s from sta%d\n",band?5:2,mac,band?0:1);
	wpa_cli_set_bssid(band,mac);
	if(band)
		nvram_set("wl1_sta_bssid",mac);
	else
		nvram_set("wl0_sta_bssid",mac);
        return 1;
}
/*
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
	else
		return 0;
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
		return 0;
	}	

	_dprintf("=> RE: get 2G bssid=%s from sta1\n",mac);
	wpa_cli_set_bssid(0,mac);
	nvram_set("wl0_sta_bssid",mac);
        return 1;

}
*/
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
	else
		return -1;
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

#if defined(RTCONFIG_HIDDEN_BACKHAUL)
	if(sw_mode() != SW_MODE_REPEATER) {
		_dprintf("=> workaround ....\n");
		set_hops_count(255); // force everything to reset
		ifconfig(get_staifname(1), 0, NULL, NULL);
		sleep(2);
		ifconfig(get_staifname(1), IFUP, NULL, NULL);
		return ;
	}
#endif
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
	else
		return -1;

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
	int j,cmp=0;
	char mac1[18]="",mac2[18]="";
	if((chk_assoc(STA_2G)>0) && (chk_assoc(STA_5G)>0))
	{

 		get_mac(get_staifname(0),mac1);
 		get_mac(get_staifname(1),mac2);

		for(j=0;j<15;j++)
		{
			cmp=abs(*(mac1+j)-*(mac2+j));
			if(cmp!=0 && cmp!=32) //upper or lower 
				return 0;
		}

		cmp=abs(*(mac1+15)-*(mac2+15));
		if(cmp!=0 && cmp!=32) //upper or lower 
			return 0;
		else
			return 1;
	}
	else
		return 0;	
/*
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
	cmp=abs(*(mac1+15)-*(mac2+15));
	if(cmp!=0 && cmp!=32) //upper or lower 
		return 0;
	else
		return 1;
*/
}


void force_down_2g(void)
{
#ifdef RTCONFIG_DUAL_BACKHAUL
	_dprintf("=> RE: temporarily disable 2G...\n");
	resolving=0;
	retry_2g=0;
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
#ifdef DETECT_5G		
int change_5g=0;
int detect_5g=1;
#endif
int wifimon_check_assoc(int retry_2g_times)
{
#ifndef RTCONFIG_DUAL_BACKHAUL
	int ch_2g;
#endif

#ifdef RTCONFIG_DUAL_BACKHAUL
	if(nvram_get_int("dfs_check_period"))
	{
		if(sp_from_2g)
		{
			_dprintf("=> RE: connect 2G first...\n");
			if(sta_is_assoc(0))
			{
				resolving=1;
				find_pap_bssid_and_config(1); //set 5G bssid from sta0				
				check_wsc_enrollee_status(7); //update current pap's bssid
				sp_from_2g=0;
				_dprintf("=> RE: connect 2G ok. config 5G bssid\n");
				return 1;
			}
			else
				return 3;
		}
	}
#endif
	if(sta_is_assoc(1))
	{
		check_wsc_enrollee_status(7); //update current pap's bssid
		if(detect_loop())
		{
#ifdef RTCONFIG_DUAL_BACKHAUL
			if(nvram_get_int("dfs_check_period"))
			{
				if(sta_is_assoc(0) && pap_db_mac()) //2G loop,too...
				{
					_dprintf("=> RE: detect loop on 5G & 2G, reset!\n");
					reset_loop();
					force_down_2g();
					return 3;
				}
				else	
					find_pap_bssid_and_config(1); //set 5G bssid from sta0				
				
			}
			else
#endif
			{
				_dprintf("=> RE: detect loop on 5G, reset!\n");
				reset_loop();
				force_down_2g();
				return 3;
			}
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
			if(!find_pap_bssid_and_config(0))//find 2G bssid,if y, config it
				return 1;		
			else
				resolving=1;
		}
		else
		{
			if(retry_2g_times!=0xff && !sta_is_assoc(0))  //try to resolve 2G until 2G is down
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
			ind_ath();			
			if(!pap_db_mac())
			{
				if(nvram_get_int("dfs_check_period"))
				{
					_dprintf("=> RE: detcet diff bssid between 2G and 5G, we reconfig 5G bssid because of NOL\n");
					find_pap_bssid_and_config(1);
				}
				else
				{
					force_down_2g();
					_dprintf("=> RE: detcet diff bssid between 2G and 5G\n");
				}
				return 4;
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

//for DFS 5G
static void ind_ath(void)
{
	char *outpt=NULL;
	int num=0;
	if((outpt=get_qca_iwpriv("ath1", "get_athnewind"))) 
	{
		num= atoi(outpt);
		free(outpt);
	}
	if(!num)
		doSystem("iwpriv %s athnewind 1", get_wififname(1)); //keep ath1 alive now

	if(chk_assoc(get_wififname(1))==0)
	{
		_dprintf("RE => for indep, wake up ath1\n");
		doSystem("ifconfig %s down", get_wififname(1)); 
		doSystem("ifconfig %s up", get_wififname(1)); 
	}
}

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
	if(cac==NULL)
		return 0;
	else
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
	else
		return 0;
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

int check_nol(void)
{
	char *outpt=NULL;
	int nolnum=0;
	if ((outpt=get_qca_iwpriv("wifi1", "get_nolcnt"))) 
	{
		nolnum= atoi(outpt);
		free(outpt);
	}
	//_dprintf("RE => get nolnum=[%d]\n", nolnum);
	return nolnum;	
}

void start_wifimon_check(int delay)
{
	int i,state, assoc_timeout;
	int restart_process=0;
//#ifdef RTCONFIG_ETHBACKHAUL
#if 0
	unsigned int eth_state_cnt=0;
#endif
	doSystem("iwpriv wifi0 ignore_dbg 1");

	if(nvram_get_int("wifimon_dbg"))
		dbg_m=1;
	else 
		dbg_m=0;

	if(nvram_get_int("dfs_check_period"))
		check_period=60;
	else
		check_period=30;

	retry_2g=0;
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
#ifdef RTCONFIG_ETHBACKHAUL
		if(nvram_get_int("wlready") && nvram_get_int("eth_backl"))
			break;
#endif
		state=wifimon_check_assoc(retry_2g);
		switch(state)
		{
			case 0:
			case 1:
//#ifdef RTCONFIG_ETHBACKHAUL
#if 0
				eth_state_cnt=0;
#endif
				if(assoc_timeout==-1 || assoc_timeout>=1 
#ifdef RTCONFIG_DUAL_BACKHAUL
				|| distance != short_distance())
#else
				|| distance != wifimon_check_hops(1))   
#endif
					set_hops_count(-1);
				assoc_timeout=0;
//#ifdef RTCONFIG_ETHBACKHAUL
#if 0
				if(nvram_get_int("wlready") && nvram_get_int("eth_backl")) { // something wrong, fix it
					int channel;
					_dprintf("<<ETH workaround>> killall wpa_supplicant!\n");
					/* workaround for wifi_mon race condition */
					kill_wifi_wpa_supplicant(-1);
					doSystem("iwconfig %s ESSID \"\"", get_staifname(1));
					ifconfig(get_staifname(1), 0, NULL, NULL);
					ifconfig(get_wififname(1), 0, NULL, NULL);
#ifdef RTCONFIG_DUAL_BACKHAUL
					doSystem("iwconfig %s ESSID \"\"", get_staifname(0));
					ifconfig(get_staifname(0), 0, NULL, NULL);
					ifconfig(get_wififname(0), 0, NULL, NULL);
#endif
					sleep(1);
					ifconfig(get_wififname(1), IFUP, NULL, NULL);
					channel=atoi(nvram_safe_get("wl1_channel"));
					if (channel)
						doSystem("iwconfig %s channel %d", get_wififname(1), channel);
#ifdef RTCONFIG_DUAL_BACKHAUL
					ifconfig(get_wififname(0), IFUP, NULL, NULL);
					channel=atoi(nvram_safe_get("wl0_channel"));
					if (channel)
						doSystem("iwconfig %s channel %d", get_wififname(0), channel);
#endif
					assoc_timeout=-2;
					distance = 255; // set local variable in wifi_mon process
				}
#endif
				break;
			case 2: //5G not-assoc, 2G unknown
			case 3: //5G not-assoc, 2G not-assoc 
			case 4: //redirect 2G or 5G ...
				if(assoc_timeout==-1 || assoc_timeout==0) 
					set_hops_count(-1);
				if(assoc_timeout<0)  //fix assoc value if unknown or eth-backhaul case
					assoc_timeout=0;
				assoc_timeout++;
				
#ifdef RTCONFIG_DUAL_BACKHAUL
				if(assoc_timeout==1)
				{
					if(state==2) //5G not-assoc but 2G is unknown.
					{
						if(nvram_get_int("dfs_check_period"))
						{
							if(chk_assoc(STA_2G) > 0)
							{
			 					find_pap_bssid_and_config(1);
								_dprintf("=> RE: config 5G bssid and keep sta0 alive !!!!!!\n");
							}
							//_dprintf("=> RE: avoid getting stuck..\n");
							//doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 disable_network 0");
							//sleep(1);
							//doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 enable_network 0");
						}
					}
				}
#endif	
//#ifdef RTCONFIG_ETHBACKHAUL
#if 0
				_dprintf("=> RE: eth_backl=%d\n",nvram_get_int("eth_backl"));
				if(nvram_get_int("eth_detect_proc") || nvram_get_int("eth_backl")) { //eth backhaul process, ignore wifimon
					if (nvram_get_int("wlready")&&(eth_state_cnt<10)) {
						int channel;
						if (get_phyrate(1)==0) {
							ifconfig(get_wififname(1), 0, NULL, NULL);
							ifconfig(get_wififname(1), IFUP, NULL, NULL);
						}
						channel=atoi(nvram_safe_get("wl1_channel"));
						if (channel)
							doSystem("iwconfig %s channel %d", get_wififname(1), channel);
						eth_state_cnt++;
					}
					assoc_timeout=-2;
					distance = 255; // set local variable in wifi_mon process
				}
#endif
				break;
			default:
//#ifdef RTCONFIG_ETHBACKHAUL
#if 0
				eth_state_cnt=0;
#endif
				break;
		}

		if(nvram_get_int("dfs_check_period")&& assoc_timeout!=0) //not-assoc in DFS
		{
			sleep(check_period-30);
			if(sta_is_assoc(1))
			{
				_dprintf("=> RE: avoid long waiting-time, bring ath1 up!!!\n");
				if(chk_assoc(get_wififname(1))==0)
        				doSystem("ifconfig %s up",get_wififname(1));
#ifdef RTCONFIG_DUAL_BACKHAUL
				if( distance != short_distance())
#else
				if(distance != wifimon_check_hops(1))   
#endif
					set_hops_count(-1);

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
  		                       if(nvram_get_int("dfs_check_period"))
                  		       {
                                		if(sta_is_assoc(0) && pap_db_mac()) //2G loop,too...
                                		{
							_dprintf("=> RE: detect loop on 5G & 2G, reset2!\n");
                                        		reset_loop();
                                        		force_down_2g();
                                		}
                                		else
                                        		find_pap_bssid_and_config(1); //set 5G bssid from sta0                          
                         
                        		}
                       			else
                        		{

						_dprintf("=> RE: detect loop on 5G, reset2!\n");
                                		reset_loop();
                                		force_down_2g();
                        		}

				}
					
				if(!sta_is_assoc(0))
				{
					if(retry_2g>2)
					{
						_dprintf("=> RE: 2G is down ...\n");
						force_down_2g();
						resolving=1;
						retry_2g=0xff;
					}
					else
						retry_2g++;
				}
				else if(!pap_db_mac())
				{	
					if(nvram_get_int("dfs_check_period"))
					{
						_dprintf("=> RE: detcet diff bssid between 2G and 5G, we reconfig 5G bssid because of NOL!\n");
						find_pap_bssid_and_config(1);
					}
					else
					{
						force_down_2g();
						_dprintf("=> RE: detcet diff bssid between 2G and 5G !!!\n");
					}
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

#ifdef RADAR_FAILOVER
					if(nvram_get_int("dfs_check_period") && check_nol())
					{

						_dprintf("=> RE: restart wireless for radar fail-over\n");
#if defined(RTCONFIG_AUTHSUPP)
						restart_wifi(1);
#else
						restart_wireless();
#endif
						force_down_2g();
					}
					else
#endif
#ifdef RTCONFIG_DUAL_BACKHAUL
					if(nvram_get_int("dfs_check_period"))
					{
						ind_ath();
						if(chk_assoc(STA_2G) > 0)
						{
			 				find_pap_bssid_and_config(1);
							_dprintf("=> RE: donot force down 2G!!\n");
						}
						else
						{
							_dprintf("=> RE: because 5G disconnect, we can not get 2G info. try 2G and stop 5G...\n");
							force_down_2g();
							doSystem("wpa_cli -p /var/run/wpa_supplicant-sta0 enable_network 0");
							doSystem("wpa_cli -p /var/run/wpa_supplicant-sta1 disable_network 0");
							sp_from_2g=1;
						}

					}
					else

#endif
					{
						wpa_supplicant_stop(1);
						wpa_supplicant_start(1);
						force_down_2g();
					}
#ifdef DETECT_5G
					detect_5g=1;
#endif
				}
			}

#ifdef RTCONFIG_DUAL_BACKHAUL
			if(nvram_get_int("dfs_check_period") && sta_is_assoc(0))
				restart_process=0;
			else
#endif
				restart_process=1;
			assoc_timeout=-1;
		}
		else
			restart_process=0;
		
		if(!pids("watchdog"))
		{
#if defined(RTCONFIG_LP5523)
                         lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#elif defined(MAPAC1750)
			nvram_set("prelink_pap_status", "-1");
#endif
			 start_watchdog();
		}

	}
}
#endif
