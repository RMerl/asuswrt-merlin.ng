/*************************************************
* <:copyright-BRCM:2013:proprietary:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/
/**
 *	@file	 wlcsm_lib_hspot.c
 *	@brief	 wlcsm hspot related functions
 *
 * 	wlcsm_lib_hspot provides necesary APIs for hspot app and as a central
 * 	place for Hspot customization.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/in.h>
#include <linux/if.h>
#include <errno.h>
#include  <pthread.h>
#include <sys/time.h>
#include  <sys/prctl.h>
#include "wlcsm_linux.h"
#include "wlcsm_lib_api.h"
#include "wlcsm_lib_nvram.h"

t_WLCSM_NVRAM_NAMES_SET g_hspot_nvram_conf_vars = {
    "wl0",
    {
        {"interface", WLCSM_NVRAM_VAR_STRING, "wl0"},
        {"hs_reset", WLCSM_NVRAM_VAR_INT, "0"},
        {"proxy_arp", WLCSM_NVRAM_VAR_INT, "1"},

        {"ip_add_type_avail", WLCSM_NVRAM_VAR_INT, "1"},
        {"conn_cap", WLCSM_NVRAM_VAR_INT, "1"},
        {"domain_list", WLCSM_NVRAM_VAR_STRING, "wifi.org"},
        {"nai_realm_list", WLCSM_NVRAM_VAR_INT, "1"},
        {"net_auth_type", WLCSM_NVRAM_VAR_INT, "1"},
        {"oper_name", WLCSM_NVRAM_VAR_INT, "1"},



	{ "hsflag",WLCSM_NVRAM_VAR_INT,		"1aa0"},	/* Passpoint Flags */
	{ "hs2cap",WLCSM_NVRAM_VAR_INT,		"1"},	/* Passpoint Realese 2 (1), Realese 1 (0) radio */
	{ "opercls",WLCSM_NVRAM_VAR_INT,		"3"},	/* Operating Class */
	{ "anonai",WLCSM_NVRAM_VAR_STRING,		"anonymous.com"},	/* Anonymous NAI */
	{ "wanmetrics",WLCSM_NVRAM_VAR_STRING,	"1:0:0=2500>384=0>0=0"}, /* WAN Metrics */
	{ "oplist",WLCSM_NVRAM_VAR_STRING,		"Fi Alliance!eng|"
	"\x57\x69\x2d\x46\x69\xe8\x81\x94\xe7\x9b\x9f!chi" }, /* Operator Friendly Name List */
	{ "homeqlist",WLCSM_NVRAM_VAR_STRING,	"mail.example.com:rfc4282"}, /* NAIHomeRealmQueryList */
	{ "osu_ssid",WLCSM_NVRAM_VAR_STRING,	"OSU"}, /* OSU SSID */
	{ "osu_frndname",WLCSM_NVRAM_VAR_STRING,"SP Red Test Only!eng|"
	"\x53\x50\x20\xEB\xB9\xA8\xEA\xB0\x95\x20\xED\x85\x8C"
	"\xEC\x8A\xA4\xED\x8A\xB8\x20\xEC\xA0\x84\xEC\x9A\xA9!kor"}, /* OSU Friendly Name */
	{ "osu_uri",WLCSM_NVRAM_VAR_STRING,
	"https://osu-server.r2-testbed.wi-fi.org/"}, /* OSU Server URI */
	{ "osu_nai",WLCSM_NVRAM_VAR_STRING,		""}, /* OSU NAI */
	{ "osu_method",WLCSM_NVRAM_VAR_INT,	"1"}, /* OSU Method */
	{ "osu_icons",WLCSM_NVRAM_VAR_STRING, 	"icon_red_zxx.png+icon_red_eng.png"}, /* OSU Icons */
	{ "osu_servdesc",WLCSM_NVRAM_VAR_STRING, "Free service for test purpose!eng|"
	"\xED\x85\x8C\xEC\x8A\xA4\xED\x8A\xB8\x20\xEB\xAA\xA9"
	"\xEC\xA0\x81\xEC\x9C\xBC\xEB\xA1\x9C\x20\xEB\xAC\xB4"
	"\xEB\xA3\x8C\x20\xEC\x84\x9C\xEB\xB9\x84\xEC\x8A\xA4!kor"}, /* OSU Serv Desc */
	{ "concaplist",WLCSM_NVRAM_VAR_STRING,	"1:0:0;6:20:1;6:22:0;" "6:80:1;6:443:1;6:1723:0;6:5060:0;" "17:500:1;17:5060:0;17:4500:1;50:0:1"}, /* Connection Capability List */
	{ "qosmapie",WLCSM_NVRAM_VAR_STRING, "35021606+8,15;0,7;255,255;16,31;32,39;255,255;40,47;255,255" },	/* QoS Map IE */
	{ "gascbdel",WLCSM_NVRAM_VAR_INT,	"0"},	/* GAS CB Delay */
	{ "iwnettype",WLCSM_NVRAM_VAR_INT,	"2"},	/* Select Access Network Type */
	{ "hessid",WLCSM_NVRAM_VAR_STRING,		"50:6F:9A:00:11:22"},	/* Interworking HESSID */
	{ "ipv4addr",WLCSM_NVRAM_VAR_INT,	"3"},	/* Select IPV4 Address Type Availability */
	{ "ipv6addr",WLCSM_NVRAM_VAR_INT,	"0"},	/* Select IPV6 Address Type Availability */
	{ "netauthlist",WLCSM_NVRAM_VAR_STRING, "accepttc=+" "httpred=https://tandc-server.wi-fi.org"},	/* Network Authentication Type List */
	{ "venuegrp",WLCSM_NVRAM_VAR_INT,	"2"},	/* Venue Group */
	{ "venuetype",WLCSM_NVRAM_VAR_INT,	"8"},	/* Venue Type  */
	{ "venuelist",WLCSM_NVRAM_VAR_STRING,
	"57692D466920416C6C69616E63650A"
	"3239383920436F7070657220526F61640A"
	"53616E746120436C6172612C2043412039"
	"353035312C2055534121656E677C"
	"57692D4669E88194E79B9FE5AE9EE9AA8CE5AEA40A"
	"E4BA8CE4B99DE585ABE4B99DE5B9B4E5BA93E69F8FE8B7AF0A"
	"E59CA3E5858BE68B89E68B892C20E58AA0E588A9E7A68FE5B0"
	"BCE4BA9A39353035312C20E7BE8EE59BBD21636869"},	/* Venue Name List */
	{ "ouilist",WLCSM_NVRAM_VAR_STRING,		"506F9A:1;001BC504BD:1"},	/* Roaming Consortium List */
	{ "3gpplist",WLCSM_NVRAM_VAR_STRING,	""},	/* 3GPP Cellular Network Information List */
	{ "domainlist",WLCSM_NVRAM_VAR_STRING,	""},	/* Domain Name List */
	{ "realmlist",WLCSM_NVRAM_VAR_STRING, "mail.example.com+0+21=2,4#5,7?""cisco.com+0+21=2,4#5,7?"
	"wi-fi.org+0+21=2,4#5,7;13=5,6?"
	"example.com+0+13=5,6"},	/* NAI Realm List */


        {NULL, WLCSM_NVRAM_VAR_STRING, "0"}
    }
};


int wlcsm_hspot_match_variable( char *name,char *interface, char*varname )
{
    char *prefix=wlcsm_prefix_match(name);
    char temp_name[128];
    int index=0;
    if(prefix) {
        char *var_name=g_hspot_nvram_conf_vars.vars[index++].name;
        while(var_name) {
            sprintf(temp_name,"%s%s",prefix,var_name);
            if(!strcmp(temp_name,name)) {
                strcpy(interface,prefix);
                interface[strlen(prefix)-1]='\0';
                strcpy(varname,var_name);
                return 1;
            }
            var_name=g_hspot_nvram_conf_vars.vars[index++].name;
        }
    }
    return 0;
}

void wlcsm_hspot_nvram_default(char *prefix,char b_overwrite)
{
    int index=0;
    char *var_value=NULL;
    char temp_name[128];
    t_WLCSM_NVRAM_VAR  var= g_hspot_nvram_conf_vars.vars[index++];
    while(var.name) {
        sprintf(temp_name,"%s%s",prefix,var.name);
        var_value=wlcsm_nvram_get(temp_name);
        if(b_overwrite || !var_value)
            wlcsm_nvram_set(temp_name,var.value);
        var= g_hspot_nvram_conf_vars.vars[index++];
    }
    return;
}


/*  check if variable is hspot variable and value is still default  */
int wlcsm_hspot_var_isdefault( char *name )
{

    int index=0;
    char *prefix=wlcsm_prefix_match(name);
    if(prefix) {
        char *var_name=g_hspot_nvram_conf_vars.vars[index++].name;
        name+=strlen(prefix);
        while(var_name) {
            if(!strncmp(name,var_name,strlen(var_name))) {
                if(strlen(name)>strlen(var_name)+1) {
                    if( !strcmp(g_hspot_nvram_conf_vars.vars[index-1].value,name+strlen(var_name)+1)) {
                        return 1;
                    } else {
                        WLCSM_TRACE(WLCSM_TRACE_DBG,"!!!!!!!!!!!!!!! NOT THE SAME  \r\n" );
                        break;

                    }
                } 
            }
            var_name=g_hspot_nvram_conf_vars.vars[index++].name;
        }
    }
    return 0;
}

