/*
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/times.h>
#include <nvram/bcmnvram.h>
#include <semaphore_mfp.h>
#include <shutils.h>
#include <ralink.h>
#include <iwlib.h>
#include <rc.h>
#include <time.h>

#ifdef RTCONFIG_AMAS
#include <wps.h>
int obd_SetWpsResult(int n, char *wif);
#endif
extern int getWscProfile(char *interface, WSC_CONFIGURED_VALUE *data, int len);

//#define WPS_DEBUG
char wsc_file_name[32];
int ap_set(char *wif, const char *pv_pair);
int writefile(char *fname,char *content);
time_t t1, t2;
#define REWPSC_PID_FILE "/var/run/re_wpsc.pid"

int wps_first_success = 0;
int re_wpsc_main(void)
{


	char wif[256]={0}, *next = NULL;
	int i = 0, j =0;
	char tmp[100]={0}, prefix[] = "wlXXXXXXX_";
	char *aif = NULL;
	char status_buf[10];
	int WscStatus_old[3]={0}, WscStatus[3]={0};
	memset(WscStatus_old, 0x0, sizeof(WscStatus_old));
	memset(WscStatus, 0x0, sizeof(WscStatus));
	int timeout = 0;
	char rm_pid_file[64]={0};

	FILE *fp_pid=NULL;


	nvram_set("wps_cli_state", "1");

		/* write pid */
	if ((fp_pid = fopen(REWPSC_PID_FILE, "w")) != NULL)
		{
			fprintf(fp_pid, "%d", getpid());
			fclose(fp_pid);
		}

	//stop_wlcconnect();

	start_wlcscan();
	sleep(3);


	/* 0: Repeater. 1: Express way 2.4G 2: Express way 5G */
	int wlc_express = nvram_get_int("wlc_express");
#ifdef RTCONFIG_AMAS
    if (nvram_get_int("wps_enrollee") == 1) {
        wlc_express = 1; // Force use 2.4G to do WPS processing.
    }
#endif
	if (wlc_express < 0 || wlc_express > 2)
		wlc_express = 0;

		if (wlc_express == 0) {
			foreach(wif, nvram_safe_get("wl_ifnames"), next) {
					sprintf(prefix, "wl%d_", j);
					aif = nvram_safe_get(strcat_r(prefix, "vifs", tmp));

					sprintf(wsc_file_name,"/tmp/wsc.%s",aif);
					unlink(wsc_file_name);
					ap_set(aif, "ApCliEnable=0");
					ap_set(aif, "WscConfMode=1");
					ap_set(aif, "WscMode=2");
					ap_set(aif, "ApCliEnable=1");
					ap_set(aif, "WscGetConf=1");
					j++;
			}
	}
	else if ( wlc_express == 1) {

				strcpy(prefix, "wl0_");
				aif = nvram_safe_get(strcat_r(prefix, "vifs", tmp));
				sprintf(wsc_file_name,"/tmp/wsc.%s",aif);
				unlink(wsc_file_name);
				ap_set(aif, "ApCliEnable=0");
				ap_set(aif, "WscConfMode=1");
				ap_set(aif, "WscMode=2");
				ap_set(aif, "ApCliEnable=1");
				ap_set(aif, "WscGetConf=1");
	}
	else if (wlc_express == 2) {
				strcpy(prefix, "wl1_");
				aif = nvram_safe_get(strcat_r(prefix, "vifs", tmp));
				sprintf(wsc_file_name,"/tmp/wsc.%s",aif);
				unlink(wsc_file_name);
				ap_set(aif, "ApCliEnable=0");
				ap_set(aif, "WscConfMode=1");
				ap_set(aif, "WscMode=2");
				ap_set(aif, "ApCliEnable=1");
				ap_set(aif, "WscGetConf=1");
	}

	t1 = time(NULL);
	t2 = time(NULL);
	while (1)
		{
			wps_first_success = 0;
			i = 0;
			foreach(wif, nvram_safe_get("wl_ifnames"), next) {
			sprintf(prefix, "wl%d_", i);
			aif = nvram_safe_get(strcat_r(prefix, "vifs", tmp));
			sprintf(wsc_file_name,"/tmp/wsc.%s",aif);


			if (wlc_express == 0 || (wlc_express == 1 && i == 0) || (wlc_express == 2 && i == 1)) {
					WscStatus[i]=getWscStatusCli(aif);


					if (WscStatus_old[i] != WscStatus[i]) {
						sprintf(status_buf,"%d",WscStatus[i]);
						writefile(wsc_file_name, status_buf);
						WscStatus_old[i]=WscStatus[i];
					}

					if (WscStatus[i] == -1){
						t2 = time(NULL);
					}

					if (WscStatus[i] == 35 || WscStatus[i] == 36) {
						nvram_set_int("led_status", LED_WPS_SCANNING);
						t2 = time(NULL);
					}

				 	if (WscStatus[i] >= 10 || WscStatus[i] <= 29) {
						nvram_set_int("led_status", LED_WPS_PROCESSING);
						t2 = time(NULL);
					}

					if (WscStatus[i] == 2) {			/* Wsc Process failed */
						fprintf(stderr, "%s", "Error occured. Is the PIN correct?\n");
						nvram_set_int("led_status", LED_WPS_FAIL);
						//stop_wps_method();
						goto EXIT;
					}
					if (WscStatus[i] == 0x109) {		/* PBC_SESSION_OVERLAP */
						fprintf(stderr, "PBC_SESSION_OVERLAP\n");
						//stop_wps_method();
						nvram_set_int("led_status", LED_WPS_FAIL);
						goto EXIT;
					}
					if (WscStatus[i] >= 30 && WscStatus[i] <= 33) {
						fprintf(stderr, "WSC Failed\n");
						//stop_wps_method();
						nvram_set_int("led_status", LED_WPS_FAIL);
						goto EXIT;
					}
					if (WscStatus[i] == 34) {
						wps_first_success = 1;
						stop_wps_method();
#ifdef RTCONFIG_AMAS
                        if (nvram_get_int("wps_enrollee") == 1)
                            obd_SetWpsResult(i, aif);
                        else
#endif
    						mtk_set_wps_result(i, aif);
						nvram_set_int("led_status", LED_RESTART_WL);
						sleep(3);
						stop_lan_wl();
						sleep(2);
						start_lan_wl();
						unlink(REWPSC_PID_FILE);
						nvram_set("wps_cli_state", "2");
#ifdef RTCONFIG_AMAS
                        nvram_set_int("wps_e_success", 1);
				        nvram_set_int("obd_Setting", 1);
#endif
						sleep(30);
						nvram_set_int("led_status", LED_BOOTED);
						return 0;
					}
			}

			timeout = t2 - t1;
			if (timeout >= 120) {
				//stop_wps_method();
				nvram_set_int("led_status", LED_RESTART_WL_DONE);
				goto EXIT;
			}
			i++;
			}
		}
EXIT:
	stop_wps_method();
	nvram_set_int("led_status", LED_BOOTED);
	nvram_set("wps_cli_state", "2");
#ifdef RTCONFIG_AMAS
    nvram_set_int("wps_e_success", 0);
#endif
	unlink(REWPSC_PID_FILE);
	return 0;
}

void GuessSSIDProfile(int band_chk, char *ssidptr)
{
/*
 * format sample:
 * SSID                            = ASUS-Vic1
 * AuthType                        = OPEN
 * EncrypType                      = WEP
 * KeyIndex                        = 1
 * Key                             = 6162636465
 */
	char *strptr1;
	char ssidptr1[]="wlcXXXXXXXXXX_", ssidptr2[]="wlcXXXXXXXXXX_", tmp[128];

	snprintf(ssidptr1, sizeof(ssidptr1),"wlc%d_",  band_chk);
	snprintf(ssidptr2, sizeof(ssidptr2),"wlc%d_",  band_chk==0? 1:0 );
	//set SSID
	nvram_set(strcat_r(ssidptr1, "ssid", tmp), ssidptr);

	//AuthType
	strptr1 = nvram_get( strcat_r(ssidptr2, "auth_mode", tmp));
	nvram_set(strcat_r(ssidptr1, "auth_mode", tmp), strptr1);

	//EncrypType
	strptr1 = nvram_get( strcat_r(ssidptr2, "wep", tmp));
	nvram_set(strcat_r(ssidptr1, "wep", tmp), strptr1);

	strptr1 = nvram_get( strcat_r(ssidptr2, "crypto", tmp));
	nvram_set(strcat_r(ssidptr1, "crypto", tmp), strptr1);

	//Key
	//strptr1 = nvram_get( strcat_r(ssidptr2, "key", tmp));
	//nvram_set(strcat_r(ssidptr1, "key", tmp), strptr1);

	if(band_chk)
		nvram_set_int(strcat_r(ssidptr1, "key", tmp), band_chk);


	strptr1 = nvram_get( strcat_r(ssidptr2, "wep_key", tmp));
	nvram_set(strcat_r(ssidptr1, "wep_key", tmp), strptr1);

	//Wpa_Psk
	strptr1 = nvram_get( strcat_r(ssidptr2, "wpa_psk", tmp));
	nvram_set(strcat_r(ssidptr1, "wpa_psk", tmp), strptr1);

	return ;
}

struct save_fuple {
	int length;
	char *cmppart;
	char *setpart1;
	char *setpart2;
};
static int comparetmp( char *arraylist[], int sizelist, char ssidptr1[], char *ssidptr2) {
	int sizetmp = 0;
	char ssidcat[128];
	memset(ssidcat, 0x0, sizeof(ssidcat));
	strcpy ( ssidcat, ssidptr1 );
	strcat ( ssidcat , ssidptr2 );
	while( sizetmp < sizelist) {
		if( !strcmp( arraylist[sizetmp], ssidcat ) ) {
			strcat ( ssidptr1, ssidptr2 );
			return 1;
		}
		sizetmp ++;
	}
	return 0;
}
static void auto_detect_ssid(unit) {
	char file_name[128]={0}, substrl[128]={0}, strNULL[]="";
	char *ssid_buf=NULL, *getptr1=NULL, *getptr2=NULL, *substrr=NULL, *gettmp[128];
	int fsize=0, idlength=0, cmpresult=0;
	int band = 0;
	struct save_fuple *bandlist=NULL;
	if (unit == 0)
		band = 1;
	else
		band = 0;

	struct save_fuple getSsidRule0[] = {
		{ 5	, "-2.4G"  , "-5G"	, ""  },
		{ 5	, "_2.4G"  , "_5G"	, ""  },
		{ 5	, ".2.4G"  , ".5G"	, ""  },
		{ 5	, " 2.4G"  , " 5G"	, ""  },
		{ 5	, "-2.4g"  , "-5g"	, ""  },
		{ 5	, "_2.4g"  , "_5g"	, ""  },
		{ 5	, ".2.4g"  , ".5g"      , ""  },
		{ 5	, " 2.4g"  , " 5g"      , ""  },
		{ 4     , "2.4G"   , "5G"       , ""  },
		{ 4	, "2.4g"   , "5g"  	, ""  },
		{ 4	, "-2.4"   , "-5"       , ""  },
		{ 4	, "_2.4"   , "_5"       , ""  },
		{ 4	, ".2.4"   , ".5"       , ""  },
		{ 4	, " 2.4"   , " 5"       , ""  },
		{ 3	, "2.4"    , "5"        , ""  },
		{ 3	, "-2G"    , "-5G"      , ""  },
		{ 3	, "_2G"    , "_5G"      , ""  },
		{ 3	, ".2G"    , ".5G"      , ""  },
		{ 3	, " 2G"    , " 5G"      , ""  },
		{ 3     , "-2g"    , "-5g"      , ""  },
		{ 0     , "_2g"    , "_5g"      , ""  },
		{ 3     , ".2g"    , ".5g"      , ""  },
		{ 3     , " 2g"    , " 5g"      , ""  },
		{ 2	, "2G"     , "5G"       , ""  },
		{ 2     , "2g"     , "5g"       , ""  },
		{ 2	, "-2"     , "-5"       , ""  },
		{ 2	, "_2"     , "_5"       , ""  },
		{ 2	, ".2"     , ".5"       , ""  },
		{ 2	, " 2"     , " 5"       , ""  },
		{ 1	, "2"      , "5"        , ""  },
		{ 0	, ""	   , "-5G"      , ""  },
		{ 0	, ""	   , "_5G"      , ""  },
		{ 0	, ""	   , ".5G"      , ""  },
		{ 0	, ""	   , " 5G"      , ""  },
		{ 0	, ""	   , "-5g"      , ""  },
		{ 0	, ""	   , "_5g"      , ""  },
		{ 0	, ""	   , ".5g"      , ""  },
		{ 0	, ""	   , " 5g"      , ""  },
		{ 0	, ""	   , "5G"       , ""  },
		{ 0	, ""	   , "5g"       , ""  },
		{ 0	, ""	   , "-5"       , ""  },
		{ 0	, ""	   , "_5"       , ""  },
		{ 0	, ""	   , ".5"       , ""  },
		{ 0	, ""	   , " 5"       , ""  },
		{ 0	, ""	   , "5"        , ""  },
		{ 0	, ""	   , ""         , ""  },
		{ 99	, ""	   , ""         , ""  }
	};
	struct save_fuple getSsidRule1[] = {
	    { 3	, "-5G"	, "-2G"	, "-2.4G"  },
		{ 3	, "_5G"	, "_2G"	, "_2.4G"  },
		{ 3	, ".5G"	, ".2G"	, ".2.4G"  },
		{ 3	, " 5G"	, " 2G"	, " 2.4G"  },
		{ 3	, "-5g"	, "-2g"	, "-2.4g"  },
		{ 3	, "_5g"	, "_2g"	, "_2.4g"  },
		{ 3	, ".5g"	, ".2g"	, ".2.4g"  },
		{ 3	, " 5g"	, " 2g"	, " 2.4g"  },
		{ 2	, "5G"	, "2G"	, "2.4G"   },
		{ 2	, "5g"	, "2g" 	, "2.4g"   },
		{ 2	, "-5"	, "-2"	, "-2.4"   },
		{ 2	, "_5"	, "_2"	, "_2.4"   },
		{ 2	, ".5"	, ".2"	, ".2.4"   },
		{ 2	, " 5"	, " 2"	, " 2.4"   },
		{ 1	, "5"	, "2" 	, "2.4"    },
		{ 0	, ""	, "-2G"	, "-2.4G"  },
		{ 0     , ""    , "_2G" , "_2.4G"  },
		{ 0     , ""    , ".2G" , ".2.4G"  },
		{ 0     , ""    , " 2G" , " 2.4G"  },
		{ 0     , ""    , "-2g" , "-2.4g"  },
		{ 0     , ""    , "_2g" , "_2.4g"  },
		{ 0     , ""    , ".2g" , ".2.4g"  },
		{ 0     , ""    , " 2g" , " 2.4g"  },
		{ 0     , ""    , "2G" 	, "2.4G"   },
		{ 0     , ""    , "2g"  , "2.4g"   },
		{ 0     , ""    , "-2"  , "-2.4"   },
		{ 0     , ""    , "_2"  , "_2.4"   },
		{ 0     , ""    , ".2"  , ".2.4"   },
		{ 0     , ""    , " 2"  , " 2.4"   },
		{ 0     , ""    , "2"  	, "2.4"    },
		{ 0     , ""    , ""  	, ""       },
		{ 99    , ""    , ""    , ""       }
	};

	//Get another band's SSID list
	sprintf(file_name, "/tmp/ssidList/ssid%d.txt", band );

	if( access( file_name, F_OK ) != -1 ) {
		getptr1 = readfile(file_name, &fsize);
		getptr2 = strstr(getptr1, "\n");
		while( getptr2 != NULL ) {
			*getptr2 = '\0';
			gettmp[idlength] = getptr1;
			getptr1 = getptr2 + 1;
			getptr2 = strstr(getptr1, "\n");
			idlength ++;
		}
	}


	free( file_name );
	sprintf(file_name, "wlc%d_ssid", unit);
	ssid_buf = nvram_safe_get(file_name);

	if (band)
		bandlist = getSsidRule0;
	else
		bandlist = getSsidRule1;
	//compare the SSID with SCAN LIST
	while( bandlist->length != 99 ) {

		memset(substrl, 0x0, sizeof(substrl));

		if ( strlen(ssid_buf) > bandlist->length  ){
			substrr = ssid_buf + strlen(ssid_buf) - bandlist->length;
		}
		else {
			bandlist++;
			continue;
		}

		if( bandlist->length == 0 ) {
			strcpy( substrl, ssid_buf );
			if ( comparetmp( gettmp, idlength, substrl, bandlist->setpart1 ) ) {
				cmpresult=1;
				break ;
			}
			if ( bandlist->setpart2 != "" ) {
				if ( comparetmp( gettmp, idlength, substrl, bandlist->setpart2 ) ) {
					cmpresult=1;
					break;
				}
			}
		}
		else if(  !strcmp(substrr, bandlist->cmppart) ){
			strncpy( substrl, ssid_buf, strlen(ssid_buf)-bandlist->length );
			if( comparetmp( gettmp, idlength, substrl, bandlist->setpart1 ) ) {
				cmpresult=1;
				break;
			}
			if ( bandlist->setpart2 != "" ) {
				if ( comparetmp( gettmp, idlength, substrl, bandlist->setpart2 ) ) {
					cmpresult=1;
					break;
				}
			}
			if ( comparetmp( gettmp, idlength, substrl, strNULL ) ) {
				cmpresult = 1;
				break;
			}
		}
		bandlist++;
	}

	if( cmpresult == 1 ) {
		dbG("=== Find the SSID : [ %s ]\n", substrl);
		GuessSSIDProfile(band, substrl);
	}
	else
		dbG("=== Can't found the SSID (connected ssid is %s) \n", ssid_buf);

	return;
}

#ifdef RTCONFIG_AMAS
int obd_SetWpsResult(int n, char *wif)
{
    WSC_CONFIGURED_VALUE result;
    char tmp[128], buf[128] = {0}, prefix[] = "wlcXXXXXXXXXX_",
                   prefix_5g[] = "wlcXXXXXXXXXX_",
                   prefix_5g1[] = "wlcXXXXXXXXXX_";
    int is_psk = 0, is_nokey = 0, ret = 0;
    int SUMband = get_wl_count();

    strncpy(prefix, "wlc0_", sizeof(prefix));
    strncpy(prefix_5g, "wlc1_", sizeof(prefix_5g));
    strncpy(prefix_5g1, "wlc2_", sizeof(prefix_5g1));

    getWscProfile(wif, &result, sizeof(result));

#ifdef WPS_DEBUG
    /* Dump result */
    dbg("IFNAME(%s) SSID: %s, AuthMode: %d, EncrypType: %d, Default Key Index: "
        "%d, Key: "
        "%s",
        wif, result.WscSsid, result.WscAuthMode, result.WscEncrypType,
        result.DefaultKeyIdx, result.WscWPAKey);
#endif

    // SSID
    nvram_set(strcat_r(prefix, "ssid", tmp), result.WscSsid);
    nvram_set(strcat_r(prefix_5g, "ssid", tmp), result.WscSsid);
    if (SUMband == 3)
        nvram_set(strcat_r(prefix_5g1, "ssid", tmp), result.WscSsid);

    // AuthType
    if (result.WscAuthMode == 0x01) {  // Open
        nvram_set(strcat_r(prefix, "auth_mode", tmp), "open");
        nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "open");
        nvram_set(strcat_r(prefix_5g, "auth_mode", tmp), "open");
        nvram_set(strcat_r(prefix_5g, "auth_mode_x", tmp), "open");
        if (SUMband == 3) {
            nvram_set(strcat_r(prefix_5g1, "auth_mode", tmp), "open");
            nvram_set(strcat_r(prefix_5g1, "auth_mode_x", tmp), "open");
        }
    } else if (result.WscAuthMode == 0x02) {  // WPA-PSK
        nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk");
        nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk");
        nvram_set(strcat_r(prefix_5g, "auth_mode", tmp), "psk");
        nvram_set(strcat_r(prefix_5g, "auth_mode_x", tmp), "psk");
        if (SUMband == 3) {
            nvram_set(strcat_r(prefix_5g1, "auth_mode", tmp), "psk");
            nvram_set(strcat_r(prefix_5g1, "auth_mode_x", tmp), "psk");
        }
        is_psk = 1;
    } else if (result.WscAuthMode == 0x04) {  // Shared
        nvram_set(strcat_r(prefix, "auth_mode", tmp), "shared");
        nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "shared");
        nvram_set(strcat_r(prefix_5g, "auth_mode", tmp), "shared");
        nvram_set(strcat_r(prefix_5g, "auth_mode_x", tmp), "shared");
        if (SUMband == 3) {
            nvram_set(strcat_r(prefix_5g1, "auth_mode", tmp), "shared");
            nvram_set(strcat_r(prefix_5g1, "auth_mode_x", tmp), "shared");
        }
    } else if (result.WscAuthMode == 0x08) {  // WPA
        nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk");
        nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk");
        nvram_set(strcat_r(prefix_5g, "auth_mode", tmp), "psk");
        nvram_set(strcat_r(prefix_5g, "auth_mode_x", tmp), "psk");
        if (SUMband == 3) {
            nvram_set(strcat_r(prefix_5g1, "auth_mode", tmp), "psk");
            nvram_set(strcat_r(prefix_5g1, "auth_mode_x", tmp), "psk");
        }
    } else if (result.WscAuthMode == 0x10) {  // WPA2
        nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk2");
        nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk2");
        nvram_set(strcat_r(prefix_5g, "auth_mode", tmp), "psk2");
        nvram_set(strcat_r(prefix_5g, "auth_mode_x", tmp), "psk2");
        if (SUMband == 3) {
            nvram_set(strcat_r(prefix_5g1, "auth_mode", tmp), "psk2");
            nvram_set(strcat_r(prefix_5g1, "auth_mode_x", tmp), "psk2");
        }
    } else if (result.WscAuthMode == 0x20) {  // WPA2-PSK
        nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk2");
        nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk2");
        nvram_set(strcat_r(prefix_5g, "auth_mode", tmp), "psk2");
        nvram_set(strcat_r(prefix_5g, "auth_mode_x", tmp), "psk2");
        if (SUMband == 3) {
            nvram_set(strcat_r(prefix_5g1, "auth_mode", tmp), "psk2");
            nvram_set(strcat_r(prefix_5g1, "auth_mode_x", tmp), "psk2");
        }
        is_psk = 1;
    } else
        fprintf(stderr, "!! Invalid AuthType:%d\n", result.WscAuthMode);

    // EncrypType
    if (result.WscEncrypType == 0x01) {  // None
        nvram_set(strcat_r(prefix, "wep", tmp), "0");
        nvram_set(strcat_r(prefix_5g, "wep", tmp), "0");
        if (SUMband == 3) nvram_set(strcat_r(prefix_5g1, "wep", tmp), "0");
        is_nokey = 1;
    } else if (result.WscEncrypType == 0x02) {  // WEP
        nvram_set(strcat_r(prefix, "wep", tmp), "1");
        nvram_set(strcat_r(prefix_5g, "wep", tmp), "1");
        if (SUMband == 3) nvram_set(strcat_r(prefix_5g1, "wep", tmp), "1");
    } else if (result.WscEncrypType == 0x04) {  // TKIP
        nvram_set(strcat_r(prefix, "crypto", tmp), "tkip");
        nvram_set(strcat_r(prefix_5g, "crypto", tmp), "tkip");
        if (SUMband == 3)
            nvram_set(strcat_r(prefix_5g1, "crypto", tmp), "tkip");
    } else if (result.WscEncrypType == 0x08) {  // AES
        nvram_set(strcat_r(prefix, "crypto", tmp), "aes");
        nvram_set(strcat_r(prefix_5g, "crypto", tmp), "aes");
        if (SUMband == 3) nvram_set(strcat_r(prefix_5g1, "crypto", tmp), "aes");
    } else if (result.WscEncrypType == 0x0c) {  // TKIP+AES
        nvram_set(strcat_r(prefix, "crypto", tmp), "tkip+aes");
        nvram_set(strcat_r(prefix_5g, "crypto", tmp), "tkip+aes");
        if (SUMband == 3)
            nvram_set(strcat_r(prefix_5g1, "crypto", tmp), "tkip+aes");
    } else
        fprintf(stderr, "!! Invalid EncrypType Type:%d\n",
                result.WscEncrypType);

    if (is_nokey) {
        nvram_set(strcat_r(prefix, "key", tmp), "1");
        nvram_set(strcat_r(prefix_5g, "key", tmp), "1");
        if (SUMband == 3) nvram_set(strcat_r(prefix_5g1, "key", tmp), "1");
        ret = 1;
    } else {
        // KeyIndex
        nvram_set_int(strcat_r(prefix, "key", tmp), result.DefaultKeyIdx);
        nvram_set_int(strcat_r(prefix_5g, "key", tmp), result.DefaultKeyIdx);
        if (SUMband == 3)
            nvram_set_int(strcat_r(prefix_5g1, "key", tmp),
                          result.DefaultKeyIdx);
        // Key
        if (strlen(result.WscWPAKey) > 0) {
            nvram_set(strcat_r(prefix, "wep_key", tmp), result.WscWPAKey);
            nvram_set(strcat_r(prefix_5g, "wep_key", tmp), result.WscWPAKey);
            if (SUMband == 3)
                nvram_set(strcat_r(prefix_5g1, "wep_key", tmp),
                          result.WscWPAKey);
            if (is_psk) {
                nvram_set(strcat_r(prefix, "wpa_psk", tmp), result.WscWPAKey);
                nvram_set(strcat_r(prefix_5g, "wpa_psk", tmp),
                          result.WscWPAKey);
                if (SUMband == 3)
                    nvram_set(strcat_r(prefix_5g1, "wpa_psk", tmp),
                              result.WscWPAKey);
            }
        } else
            fprintf(stderr, "No key found!!\n");
        ret = 1;

        nvram_commit();
    }
    return ret;
}
#endif

#define WPS_PAP_PROFILE_PATH "/tmp/iwpriv.stat"
int mtk_set_wps_result(int n, char *wif)
/*
 * format sample:
 * SSID                            = ASUS-Vic1
 * MAC                             = F4:6D:04:DB:4E:CE
 * AuthType                        = OPEN
 * EncrypType                      = WEP
 * KeyIndex                        = 1
 * Key                             = 6162636465
 */
{


	char *fp = NULL;
	int fsize, ret = 0;
	char tmp[128], prefix[] = "wlcXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix),"wlc%d_", n);
	doSystem("iwpriv %s stat > %s", wif, WPS_PAP_PROFILE_PATH);

	sleep(2);
	fp = readfile(WPS_PAP_PROFILE_PATH, &fsize);

	if (fp) {
		char *pt1, *pt2, buf[128];
		int is_nokey = 0;
		int is_psk = 0;
		int key_idx;
		pt1 = strstr(fp, "Profile[0]:");
		if (pt1) {
			//SSID
			pt2 = pt1 + 11;
			pt1 = strstr(pt2, "= ");
			pt1 += 2;
			pt2 = strstr(pt1, "\n");
			*pt2 = '\0';
			nvram_set(strcat_r(prefix, "ssid", tmp), pt1);

#ifndef SWMODE_REPEATER_V2_SUPPORT
			if (wps_first_success) {
			sprintf(buf, "%s_RPT", pt1);
			nvram_set("wl0.1_ssid", buf);
			sprintf(buf, "%s_RPT5G", pt1);
			nvram_set("wl1.1_ssid", buf);
			}
#endif
			//AuthType
			pt2++;
			pt1 = strstr(pt2, "AuthType");
			pt2 = pt1 + 8;
			pt1 = strstr(pt2, "= ");
			pt1 += 2;
			pt2 = strstr(pt1, "\n");
			*pt2 = '\0';
			if (!strcmp(pt1, "OPEN")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "open");
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "open");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_auth_mode", "open");
					nvram_set("wl1.1_auth_mode", "open");
					nvram_set("wl0.1_auth_mode_x", "open");
					nvram_set("wl1.1_auth_mode_x", "open");
				}
#endif
			}
			else if (!strcmp(pt1, "WPAPSK")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk");
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk");
				is_psk = 1;
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_auth_mode", "psk");
					nvram_set("wl1.1_auth_mode", "psk");
					nvram_set("wl0.1_auth_mode_x", "psk");
					nvram_set("wl1.1_auth_mode_x", "psk");
				}
#endif
			}
			else if (!strcmp(pt1, "SHARED")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "shared");
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "shared");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_auth_mode", "shared");
					nvram_set("wl1.1_auth_mode", "shared");
					nvram_set("wl0.1_auth_mode_x", "shared");
					nvram_set("wl1.1_auth_mode_x", "shared");
				}
#endif
			}
			else if (!strcmp(pt1, "WPA")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk");
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_auth_mode", "psk");
					nvram_set("wl1.1_auth_mode", "psk");
					nvram_set("wl0.1_auth_mode_x", "psk");
					nvram_set("wl1.1_auth_mode_x", "psk");
				}
#endif
			}
			else if (!strcmp(pt1, "WPA2")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk2");
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk2");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_auth_mode", "psk2");
					nvram_set("wl1.1_auth_mode", "psk2");
					nvram_set("wl0.1_auth_mode_x", "psk2");
					nvram_set("wl1.1_auth_mode_x", "psk2");
				}
#endif
			}
			else if (!strcmp(pt1, "WPAPSKWPA2PSK")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk2");
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk2");
				is_psk = 1;
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_auth_mode", "psk2");
					nvram_set("wl1.1_auth_mode", "psk2");
					nvram_set("wl0.1_auth_mode_x", "psk2");
					nvram_set("wl1.1_auth_mode_x", "psk2");
				}
#endif
			}
			else if (!strcmp(pt1, "WPA2PSK")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk2");
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk2");
				is_psk = 1;
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_auth_mode", "psk2");
					nvram_set("wl1.1_auth_mode", "psk2");
					nvram_set("wl0.1_auth_mode_x", "psk2");
					nvram_set("wl1.1_auth_mode_x", "psk2");
				}
#endif
			}
			else
				fprintf(stderr, "!! Invalid AuthType:%s\n", pt1);
			//EncrypType
			pt2++;
			pt1 = strstr(pt2, "= ");
			pt1 += 2;
			pt2 = strstr(pt1, "\n");
			*pt2 = '\0';
			if (!strcmp(pt1, "NONE")) {
				nvram_set(strcat_r(prefix, "wep", tmp), "0");
				is_nokey = 1;
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_wep", "0");
					nvram_set("wl1.1_wep", "0");
				}
#endif
			}
			else if (!strcmp(pt1, "WEP")) {
				nvram_set(strcat_r(prefix, "wep", tmp), "1");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_wep", "1");
					nvram_set("wl1.1_wep", "1");
				}
#endif
			}
			else if (!strcmp(pt1, "TKIP")) {
				nvram_set(strcat_r(prefix, "crypto", tmp), "tkip");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_crypto", "tkip");
					nvram_set("wl1.1_crypto", "tkip");
				}
#endif
			}
			else if (!strcmp(pt1, "AES")) {
				nvram_set(strcat_r(prefix, "crypto", tmp), "aes");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_crypto", "aes");
					nvram_set("wl1.1_crypto", "aes");
				}
#endif
			}
			else if (!strcmp(pt1, "TKIPAES")) {
				nvram_set(strcat_r(prefix, "crypto", tmp), "tkip+aes");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_crypto", "tkip+aes");
					nvram_set("wl1.1_crypto", "tkip+aes");
				}
#endif
			}
			else
				fprintf(stderr, "!! Invalid EncrypType Type:%s\n", pt1);


			if (is_nokey) {
				nvram_set(strcat_r(prefix, "key", tmp), "1");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_key", "1");
					nvram_set("wl1.1_key", "1");
				}
#endif
				ret = 1;
			}
			else {
				//KeyIndex
				pt2++;
				pt1 = strstr(pt2, "= ");
				pt1 += 2;
				pt2 = strstr(pt1, "\n");
				*pt2 = '\0';
				nvram_set(strcat_r(prefix, "key", tmp), pt1);
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_key", pt1);
					nvram_set("wl1.1_key", pt1);
				}
#endif
				key_idx = atoi(pt1);

				//Key
				pt2++;
				pt1 = strstr(pt2, "= ");
				if (pt1) {
					pt1 += 2;
					pt2 = strstr(pt1, "\n");
					*pt2 = '\0';
					//sprintf(buf, "%s%d", strcat_r(prefix, "wep_key", tmp), key_idx);
					//nvram_set(buf, pt1);
					nvram_set(strcat_r(prefix, "wep_key", tmp), pt1);
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_wep_key", pt1);
					nvram_set("wl1.1_wep_key", pt1);
				}
#endif
					if (is_psk) {
						// fix Ralink wireless driver bug
						char _wpa_psk[65];
						int _len = strlen(pt1);
						if (_len > 64)
							_len = 64;
						memset(_wpa_psk, 0x0, sizeof(_wpa_psk));
						strncpy(_wpa_psk, pt1, _len);

						nvram_set(strcat_r(prefix, "wpa_psk", tmp), _wpa_psk);
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0.1_wpa_psk", pt1);
					nvram_set("wl1.1_wpa_psk", pt1);
				}
#endif
					}
				}
				else
					fprintf(stderr, "No key found!!\n");
				ret = 1;
			}

			if (nvram_match("wlc_express", "0"))
				auto_detect_ssid(n);
			/* Finish WPS while dut is default, change the x_Setting as 1 */
			if(nvram_match("x_Setting", "0")) {
				nvram_set("x_Setting", "1");
			}
			nvram_commit();
		}
		free(fp);
	}
	//unlink(WPS_PAP_PROFILE_PATH);

	return ret;
}
