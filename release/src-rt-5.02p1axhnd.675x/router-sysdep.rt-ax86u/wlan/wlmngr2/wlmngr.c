/*
* <:copyright-BRCM:2011:proprietary:standard
*
*    Copyright (c) 2011 Broadcom
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

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <security_ipc.h>
#include "board.h"
#include <shutils.h>

#ifdef DSLCPE_1905
#include <wps_1905.h>
#endif
#include "odl.h"
#include <pthread.h>
#include <bcmnvram.h>
#include <bcmconfig.h>

#include "wlioctl.h"
#include "wlutils.h"
#include "wlmngr.h"
#include "wllist.h"

#include "wlsyscall.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include <wlcsm_lib_dm.h>
//#include "wlmngr_http.h"

#ifdef SUPPORT_WSC
#include <time.h>
#endif
#include <wlcsm_lib_api.h>
#include <wlcsm_lib_dm.h>
#include <wlcsm_linux.h>
// #define WL_WLMNGR_DBG

#if defined(HSPOT_SUPPORT)
#include <wlcsm_lib_hspot.h>
#endif
#define IFC_BRIDGE_NAME	 "br0"

char g_wifi_autorestart_enable=1;

extern int g_dm_loaded;
extern int dhd_probe(char *name);
extern bool wl_wlif_is_psta(char *ifname);
#if defined(CONFIG_HOSTAPD) && defined(BCA_CPEROUTER)
void stop_hapd_wpasupp();
#endif
#define WL_PREFIX "_wl"
pthread_mutex_t g_WLMNGR_THREAD_MUTEX= PTHREAD_MUTEX_INITIALIZER; /**< mutex for Synchronization between Thread */
unsigned int g_radio_idx=0;
char g_ifbuf[WL_SIZE_512_MAX] = {0};

pthread_mutex_t g_WLMNGR_REBOOT_MUTEX= PTHREAD_MUTEX_INITIALIZER; /**< mutex for sync reboot */
static int _wlmngr_handle_wlcsm_cmd_get_dmvar(t_WLCSM_MNGR_VARHDR  *hdr,char *varName,char *varValue);

void wlmngr_thread_lock(int lock)
{
    if(lock)
        pthread_mutex_lock(&g_WLMNGR_THREAD_MUTEX);
    else
        pthread_mutex_unlock(&g_WLMNGR_THREAD_MUTEX);
}

void wlmngr_get_thread_lock(void)
{
    pthread_mutex_lock(&g_WLMNGR_THREAD_MUTEX);
}

void wlmngr_release_thread_lock(void)
{
    pthread_mutex_unlock(&g_WLMNGR_THREAD_MUTEX);
}

bool cur_mbss_on = FALSE; /* current mbss status, off by default at init */
int act_wl_cnt = 0; /* total enabled wireless adapters */
int is_smp_system = 0;  /* set to 1 if we are on SMP system */

#ifdef SUPPORT_WSC
int wps_config_command;
int wps_action;
char wps_uuid[36];
char wps_unit[32];
int wps_method;
char wps_autho_sta_mac[sizeof("00:00:00:00:00:00")];
int wps_enr_auto = 0;
#endif
#define MAX_BR_NUM	8

#define WLHSPOT_NOT_SUPPORTED 2
/* These functions are to be used only within this file */
static void wlmngr_getVarFromNvram(void *var, const char *name, const char *type);
static bool wlmngr_detectApp(char *app);

int get_wps_env();
int set_wps_env(char *uibuf);


#ifdef BCM_WBD
static int _wlmngr_nv_wbd_need_adjust(const unsigned int idx) {

    char nvram_name[WL_SIZE_256_MAX]= {0};
    /*when wbd_ifnames is not defined and wbd_mode is 2(slave)  */

    if ((nvram_get("wbd_ifnames")==NULL)   && !strncmp(nvram_safe_get("wbd_mode"),"2",1)) {
        snprintf(nvram_name,WL_SIZE_256_MAX,"wl%d_dwds",idx);
        if(!strncmp(nvram_safe_get(nvram_name),"1",1))  {
            snprintf(nvram_name,WL_SIZE_256_MAX,"wl%d_mode",idx);
            if(!strncmp(nvram_safe_get(nvram_name),"sta",3))  {
                return 1;
            }
        }
    }
    return 0;
}

static void _wlmngr_nv_adjust_wbd(const unsigned int idx,int direction) {
    /*when dwds and sta setting on primary interface, use wlx.1 as the
     *wbd serving BSS */
    if(_wlmngr_nv_wbd_need_adjust(idx))
        wlcsm_dm_mngr_set_all_value(idx,1,"wlEnblSsid","1");
}
#endif

/* check if wlx is in the string or not, if wlx.y is in, it still
 * should return 0
 */
static inline int _interface_in_string(char *str,int index) {
    char ifname[16],*tempstr;
    int len;
    snprintf(ifname,16,"wl%d",index);
    len=strnlen(ifname,8);
    while((tempstr=strstr(str,ifname))) {
        if(tempstr[len]==' ' || tempstr[len] =='\0')
            return 1;
        str=tempstr+len;
    }
    return 0;
}

void wlmngr_enum_ifnames(unsigned int idx)
{
    char brlist[WL_SIZE_256_MAX];
    char name[32],nv_name[32],nv_value[64];
    char *next=NULL,*lnext=NULL;
    int index=0,bridgeIndex=0;
    char br_ifnames[WL_SIZE_512_MAX];
    if(g_ifbuf[0]!='\0') {
        /*rest lanx_ifnames */
        for (index = 1; index<MAX_BR_NUM; ++index) {
            sprintf(brlist,"lan%d_ifnames",index);
            if(nvram_get(brlist))
                wlcsm_nvram_unset(brlist);
        }

        /* reset wlx_vifs */
        sprintf(nv_name,"wl%d_vifs",idx);
        wlcsm_nvram_unset(nv_name);
        /* reset wlx.y_ifname, but leave wlx_ifname unchanged regardless */

        for (index=1; index<WL_RADIO_WLNUMBSS(idx); index++) {
            sprintf(brlist,"wl%d.%d_ifname",idx,index);
            if(wlcsm_nvram_get(brlist))
                wlcsm_nvram_unset(brlist);
        }

        for_each(brlist,g_ifbuf,lnext) {
            index=0;
            bridgeIndex=0;
            memset(br_ifnames,0,sizeof(br_ifnames));
            foreachcolon(name,brlist,next) {
                if(index++ == 0) {

                    /* here it is the bridge name,and we get index from it */
                    if(sscanf(name,"br%d",&bridgeIndex)) {
                        if (bridgeIndex == 0)
                            snprintf(nv_name, sizeof(nv_name), "lan_hwaddr");
                        else
                            snprintf(nv_name, sizeof(nv_name), "lan%d_hwaddr", bridgeIndex);

                        wlmngr_getHwAddr(0, name, nv_value);
                        nvram_set(nv_name, nv_value);

                        if (bridgeIndex == 0)
                            snprintf(nv_name, sizeof(nv_name), "lan_ifname");
                        else
                            snprintf(nv_name, sizeof(nv_name), "lan%d_ifname", bridgeIndex);
                        nvram_set(nv_name,name);

                    } else {
                        WLCSM_TRACE(WLCSM_TRACE_LOG," BRIDGE name is in different format??? \r\n" );
                    }

                } else if(strncmp(name,"usb",3)!=0)  {
                    if(strlen(br_ifnames))
                        snprintf(br_ifnames+strlen(br_ifnames), sizeof(br_ifnames)-strlen(br_ifnames), " %s",name);
                    else
                        snprintf(br_ifnames, sizeof(br_ifnames), "%s",name);
                }
            }
#ifdef BCM_WBD
            if(_wlmngr_nv_wbd_need_adjust(idx)) {
                snprintf(nv_name, sizeof(nv_name), "wl%d.1",idx);
                if(!strstr(br_ifnames,nv_name)) {
                    snprintf(br_ifnames+strlen(br_ifnames), sizeof(br_ifnames)-strlen(br_ifnames), " %s",nv_name);
                }
            }

            snprintf(nv_name, sizeof(nv_name), "br%d_ifnames", bridgeIndex);
            nvram_set(nv_name,br_ifnames);
#endif

            if (bridgeIndex == 0)
                snprintf(nv_name, sizeof(nv_name), "lan_ifnames");
            else
                snprintf(nv_name, sizeof(nv_name), "lan%d_ifnames", bridgeIndex);

            for (index = 0; index  <WL_WIFI_RADIO_NUMBER; ++index) {
                sprintf(nv_value,"wl%d",index);
                if( !_interface_in_string(br_ifnames,index) && WL_BSSID_BRIDGE_NAME(index,0) &&
                        (strlen(nv_value)+strlen(br_ifnames)+1<WL_SIZE_512_MAX)  &&
                        (WL_BSSID_BRIDGE_NAME(index,0)[2] - 0x30 == bridgeIndex)) {
                    strcat(br_ifnames," ");
                    strcat(br_ifnames,nv_value);
                }
            }
            WLCSM_TRACE(WLCSM_TRACE_DBG,"br_ifnames:%s\r\n",br_ifnames);
            nvram_set(nv_name,br_ifnames);
            bridgeIndex++;
        }

        /* construct wlx_vifs */
        brlist[0]='\0';
        for (index=1; index<WL_RADIO_WLNUMBSS(idx); index++) {
            if(WL_BSSID_WLENBLSSID(idx,index)) {
                snprintf(nv_name,sizeof(nv_name)," wl%d.%d",idx,index);
                strcat(brlist,nv_name);
            }
        }
        if(strlen(brlist)>1) {
            snprintf(nv_name,sizeof(nv_name),"wl%d_vifs",idx);
            wlcsm_nvram_set(nv_name,brlist);
        }
    }
}

/* hook function for DM layer to get runtime object pointer */
void *wlmngr_get_runtime_obj_pointer(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,WLCSM_DM_MNGR_CMD cmd,void *par)
{
    switch ( oid ) {
    case WLMNGR_DATA_POS_WIFI_ASSOCIATED_DEVICE:
        WLCSM_TRACE(WLCSM_TRACE_LOG," TODO: return the Assocaited device pointer, currently we are using different structure,to consolidate \r\n" );
        break;
    case WLMNGR_DATA_POS_WIFI_SSID_STATS:
        WLCSM_TRACE(WLCSM_TRACE_LOG," TODO:get WIFI SSID STATISTICS \r\n" );
        break;
    case WLMNGR_DATA_POS_WIFI_RADIO_STATS:
        WLCSM_TRACE(WLCSM_TRACE_LOG," TODOL get RADIO SATISTICS \r\n" );
        break;
    default:
        WLCSM_TRACE(WLCSM_TRACE_LOG,"ERROR: NO SUCH RUNTIME OBJECT!!!!!!!!!!!!!!!!!!!!\n" );
        break;
    }
    return NULL;
}


static int __wlmngr_handle_wlcsm_var_validate(t_WLCSM_MNGR_VARHDR  *hdr, char *varName,char *varValue)
{
    int ret=0;
    ret=wlmngr_handle_special_var(hdr,varName,varValue,WLMNGR_VAR_OPER_VALIDATE);
    WLCSM_TRACE(WLCSM_TRACE_DBG, "ret:%d",ret);
    return ret== WLMNGR_VAR_HANDLE_FAILURE?WLMNGR_VAR_HANDLE_FAILURE:0;
}

static int _wlmngr_handle_wlcsm_cmd_reg_dm_event(t_WLCSM_MNGR_VARHDR  *hdr, char *varName,char *varValue)
{
    /* sub_idx is the event number */
    WLCSM_TRACE(WLCSM_TRACE_LOG," REGISTER event:%d \r\n",hdr->sub_idx );
    return  wlcsm_dm_reg_event(hdr->sub_idx,WLCSM_MNGR_CMD_GET_SAVEDM(hdr->radio_idx));
}

static int _wlmngr_handle_wlcsm_cmd_setdmdbglevel_event(t_WLCSM_MNGR_VARHDR  *hdr, char *varName,char *varValue)
{
    return wlcsm_dm_set_dbglevel(varValue);
}

static int
wl_send_dif_event(const char *ifname, uint32 event)
{
    static int s = -1;
    int len, n;
    struct sockaddr_in to;
    char data[IFNAMSIZ + sizeof(uint32)];

    /* create a socket to receive dynamic i/f events */
    if (s < 0) {
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0) {
            perror("socket");
            return -1;
        }
    }

    /* Init the message contents to send to eapd. Specify the interface
     * and the event that occured on the interface.
     */
    strncpy(data, ifname, IFNAMSIZ);
    *(uint32 *)(data + IFNAMSIZ) = event;
    len = IFNAMSIZ + sizeof(uint32);

    /* send to eapd */
    to.sin_addr.s_addr = inet_addr(EAPD_WKSP_UDP_ADDR);
    to.sin_family = AF_INET;
    to.sin_port = htons(EAPD_WKSP_DIF_UDP_PORT);

    n = sendto(s, data, len, 0, (struct sockaddr *)&to,
               sizeof(struct sockaddr_in));

    if (n != len) {
        perror("udp send failed\n");
        return -1;
    }

    dprintf("hotplug_net(): sent event %d\n", event);

    return n;
}

extern int wl_wlif_wds_ap_ifname(char *inteface,char* name);
extern void get_bridge_by_ifname(char *inteface,char** name);
static int _wlmngr_handle_wlcsm_cmd_nethotplug_event(t_WLCSM_MNGR_VARHDR  *hdr, char *interface,char *action)
{
    bool psta_if, dyn_if,wds_if, add_event, remove_event, monitor_if;
    char temp_s[256] = {0};
    char wdsap_ifname[32]= {0};
    char *lan_ifname=NULL;

    add_event = !WLCSM_STRCMP(action, "add");
    remove_event = !WLCSM_STRCMP(action, "remove");
    psta_if = wl_wlif_is_psta(interface);
    monitor_if = !strncmp(interface, "radiotap", 8);
    wds_if = !strncmp(interface, "wds", 3);
    dyn_if = wds_if || psta_if || monitor_if;

    WLCSM_TRACE(WLCSM_TRACE_DBG, "interface:%s and action:%s",interface,action);

    if (!dyn_if && !remove_event)
        return 0;

    if(wds_if) {
        if (!wl_wlif_wds_ap_ifname(interface, wdsap_ifname)) {
            get_bridge_by_ifname(wdsap_ifname, &lan_ifname);
        } else {
            /* Since wl_wlif_wds_ap_ifname() hasn't returned the 
             * parent interface this might be vlan interface crated on a
             * dwds interface, don't add it to default bridge br0.
             */
            return 0;
        }
    }

    /* default bridge to "br0" if no lan_ifname nvram specified */
    if(!lan_ifname && !(lan_ifname=nvram_get("lan_ifname")))
        lan_ifname="br0";

    if (add_event) {
        /* only need to do bridge opernation for wds dynamic interface */
        if(wds_if) {
            snprintf(temp_s,255,"brctl addif %s %s",lan_ifname, interface);
            system(temp_s);
        }
        snprintf(temp_s,255,"ifconfig %s up",interface);
        system(temp_s);
        if(psta_if)
            wl_send_dif_event(interface, 0);
        return 0;
    }

    if (remove_event) {
        /* Indicate interface delete event to eapd */
        snprintf(temp_s,255,"brctl delif %s %s",lan_ifname, interface);
        system(temp_s);
        if(psta_if)
            wl_send_dif_event(interface, 1);
    }

    return 0;
}
static int _wlmngr_handle_wlcsm_cmd_set_dmvar(t_WLCSM_MNGR_VARHDR  *hdr, char *varName,char *varValue)
{
    unsigned int pos=0;
    int ret=0;
    WLCSM_NAME_OFFSET *name_offset= wlcsm_dm_get_mngr_entry(hdr,varValue,&pos);
    if(!name_offset) {
        WLCSM_TRACE(WLCSM_TRACE_LOG," reurn not successull \r\n" );
        ret=-1;
    } else {
        WLCSM_TRACE(WLCSM_TRACE_LOG,"dm oid is matching to wlmngr %s and offset:%d\r\n",name_offset->name,name_offset->offset);
        ret=__wlmngr_handle_wlcsm_var_validate(hdr,name_offset->name,varValue);
        if(!ret) {
            WLCSM_TRACE(WLCSM_TRACE_LOG," set wlmngr var by DM OID \r\n" );
            WLCSM_MNGR_CMD_SET_CMD(hdr->radio_idx,0);
            WLCSM_MNGR_CMD_SET_DMOPER(hdr->radio_idx);
            ret=wlmngr_set_var(hdr,name_offset->name,varValue);
        }
    }
    return ret;
}

static int _wlmngr_handle_wlcsm_cmd_validate_dmvar(t_WLCSM_MNGR_VARHDR  *hdr, char *varName,char *varValue)
{

    unsigned int pos=0;
    int ret=0;
    WLCSM_NAME_OFFSET *name_offset= wlcsm_dm_get_mngr_entry(hdr,varValue,&pos);

    if(!name_offset) {
        WLCSM_TRACE(WLCSM_TRACE_LOG," reurn not successull \r\n" );
        ret=-1;
    } else {
        WLCSM_TRACE(WLCSM_TRACE_LOG,"dm oid is matching to wlmngr %s and offset:%d\r\n",name_offset->name,name_offset->offset);
        ret=__wlmngr_handle_wlcsm_var_validate(hdr,name_offset->name,varValue);
    }
    return ret;
}

//
//**************************************************************************
// Function Name: getGPIOverlays
// Description  : get the value of GPIO overlays
// Parameters   : interface idx.
// Returns      : none.
//**************************************************************************
unsigned long wlmngr_adjust_GPIOOverlays(const unsigned int idx )
{
    int f = open( "/dev/brcmboard", O_RDWR );
    unsigned long  GPIOOverlays = 0;
    if( f > 0 ) {
        BOARD_IOCTL_PARMS IoctlParms;
        memset( &IoctlParms, 0x00, sizeof(IoctlParms) );
        IoctlParms.result = -1;
        IoctlParms.string = (char *)&GPIOOverlays;
        IoctlParms.strLen = sizeof(GPIOOverlays);
        ioctl(f, BOARD_IOCTL_GET_GPIOVERLAYS, &IoctlParms);
        WL_RADIO(idx).GPIOOverlays=GPIOOverlays;
        WLCSM_TRACE(WLCSM_TRACE_LOG," ---SETTING GPIOOverlay---:%u \r\n",GPIOOverlays );
        close(f);
    }
    return GPIOOverlays;
}

//**************************************************************************
// Function Name: setASPM
// Description  : set aspm according to GPIOOverlays
// Parameters   : interface idx.
// Returns      : none.
//**************************************************************************
void wlmngr_setASPM(const unsigned int idx )
{

#ifdef IDLE_PWRSAVE
    {
        char cmd[WL_SIZE_132_MAX];
        int is_dhd=0;
        snprintf(cmd,sizeof(cmd),"wl%d",idx);
        is_dhd=!dhd_probe(cmd);
        /* Only enable L1 mode, because L0s creates EVM issues. The power savings are the same */
        if (WL_RADIO(idx).GPIOOverlays & BP_OVERLAY_PCIE_CLKREQ) {
            if(is_dhd)
                snprintf(cmd, sizeof(cmd), "dhd -i wl%d aspm 0x102", idx);
            else
                snprintf(cmd, sizeof(cmd), "wl -i wl%d aspm 0x102", idx);
        } else {
            if(is_dhd)
                snprintf(cmd, sizeof(cmd), "dhd -i wl%d aspm 0x2", idx);
            else
                snprintf(cmd, sizeof(cmd), "wl -i wl%d aspm 0x2", idx);
        }

        bcmSystem(cmd);
    }
#endif
}


static  void _wlmngr_nv_adjust_security(const unsigned int idx,SYNC_DIRECTION direction)
{
    char buf[WL_SIZE_512_MAX];

    nvram_set("wl_key1",WL_APSEC_WLKEY1(idx,0));
    nvram_set("wl_key2",WL_APSEC_WLKEY2(idx,0));
    nvram_set("wl_key3",WL_APSEC_WLKEY3(idx,0));
    nvram_set("wl_key4",WL_APSEC_WLKEY4(idx,0));
    snprintf(buf, sizeof(buf), "%d",WL_APSEC_WLKEYINDEX(idx,0));
    nvram_set("wl_key",buf);
    nvram_set("wl_wep",WL_APSEC_WLWEP(idx,0));
}

static  void _wlmngr_nv_adjust_wps(const unsigned int idx,SYNC_DIRECTION direction)
{
    int br,i;
    char cmd[WL_SIZE_256_MAX]= {0};

    if ( direction== WL_SYNC_FROM_DM ) {
        for(i=0; i<WL_RADIO_WLNUMBSS(idx); i++) {
            if(WL_BSSID_BRIDGE_NAME(idx,i)) {
                br = WL_BSSID_BRIDGE_NAME(idx,i)[2] - 0x30;
                if ( br == 0 )
                    snprintf(cmd, sizeof(cmd), "lan_wps_oob");
                else
                    snprintf(cmd, sizeof(cmd), "lan%d_wps_oob", br);

                if(WLCSM_STRCMP(WL_APWPS_WLWSCAPMODE(idx,i),"0"))
                    nvram_set(cmd,"disabled");
                else
                    nvram_set(cmd,"enabled");
            } else
                WLCSM_TRACE(WLCSM_TRACE_ERR,"ERROR: NO bridge interface name???? \r\n" );
        }
    }
}

void wlmngr_nvram_adjust(const unsigned int idx,int direction)
{

    _wlmngr_nv_adjust_wps(idx,direction);
    _wlmngr_nv_adjust_security(idx,direction);
#ifndef NO_CMS
    wlmngr_nv_adjust_chanspec(idx,direction);
#endif
#ifdef BCM_WBD
    _wlmngr_nv_adjust_wbd(idx,direction);
#endif

}

static  void _wlmngr_adjust_country_rev(const unsigned int idx, SYNC_DIRECTION  direction)
{
    char ccode[10]= {0}; /* make it bigger to tolerate the wrong counryrev */
    snprintf(ccode, sizeof(ccode), "%s/%d",WL_RADIO_COUNTRY(idx),WL_RADIO_REGREV(idx));
    wlcsm_strcpy(&WL_RADIO_COUNTRYREV(idx),ccode);
    WLCSM_TRACE(WLCSM_TRACE_LOG," countryrev:%s \r\n",WL_RADIO_COUNTRYREV(idx));
}

static  void _wlmngr_adjust_security(const unsigned int idx, SYNC_DIRECTION  direction)
{
    int i;
    for(i=0; i<WL_RADIO_WLNUMBSS(idx); i++) {
        WL_APSEC_WLSECMODE(idx,i)=
            !WL_APSEC_WLAUTHAKM(idx,i)?NWIFI_SECURITY_MODE_OPEN:(
                (!WLCSM_STRCMP(WL_APSEC_WLAUTHAKM(idx,i),STR_AKM_WPA_WPA2))?NWIFI_SECURITY_MODE_WPA_WPA2_Enterprise:(
                    (!WLCSM_STRCMP(WL_APSEC_WLAUTHAKM(idx,i),STR_AKM_WPA2))?NWIFI_SECURITY_MODE_WPA2_Enterprise:(
                        (!WLCSM_STRCMP(WL_APSEC_WLAUTHAKM(idx,i),STR_AKM_PSK_PSK2))?NWIFI_SECURITY_MODE_WPA_WPA2_Personal:(
                            (!WLCSM_STRCMP(WL_APSEC_WLAUTHAKM(idx,i),STR_AKM_PSK2))?NWIFI_SECURITY_MODE_WPA2_Personal:NWIFI_SECURITY_MODE_OPEN))));

        if(!WLCSM_STRCASECMP(WL_RADIO_WLNMODE(idx),STR_OFF) && !WLCSM_STRCASECMP(WL_APSEC_WLWEP(idx,i),STR_ENABLED))  {
            /*if nmode is off, there is possibility that the wlsecmode is WEP-64 */
            if(strlen(WL_APSEC_WLKEY1(idx,i))==5)
                WL_APSEC_WLSECMODE(idx,i)= NWIFI_SECURITY_MODE_WEP_64;
            else
                WL_APSEC_WLSECMODE(idx,i)= NWIFI_SECURITY_MODE_WEP_128;
            wlcsm_strcpy(&WL_APSEC_WLAUTHMODE(idx,i),STR_OPEN);
        } else {

            if(WL_APSEC_WLSECMODE(idx,i)==NWIFI_SECURITY_MODE_OPEN)
                wlcsm_strcpy(&WL_APSEC_WLAUTHMODE(idx,i),STR_OPEN);
            else
                wlcsm_strcpy(&WL_APSEC_WLAUTHMODE(idx,i),STR_NONE);
        }

    }
}

void wlmngr_adjust_radio_runtime(int idx)
{
    int i=0;
    char wlver[WL_VERSION_STR_LEN]= {0};
    char cmd[WL_SIZE_132_MAX]= {0};
    char buf[WL_CAP_STR_LEN]= {0};
    int ret;
    //int bw_cap=WLC_BW_CAP_20MHZ |WLC_BW_CAP_40MHZ;

    WL_RADIO(idx).wlCoreRev=wlmngr_getCoreRev(idx);
    WL_RADIO(idx).wlRate=wlmngr_getValidRate(idx, WL_RADIO(idx).wlRate);
    WL_RADIO(idx).wlMCastRate=  wlmngr_getValidRate(idx, WL_RADIO(idx).wlMCastRate);
    WL_RADIO(idx).wlBand= wlmngr_getValidBand(idx,WL_RADIO(idx).wlBand);
    /* auto channel always supported */
    WL_RADIO_AUTOCHANNELSUPPORTED(idx)=1;

    wlcsm_strcpy(&WL_PHYTYPE(idx),wlmngr_getPhyType(idx));
    wlcsm_strcpy(&WL_RADIO_SUPPORTEDSTANDARDS(idx),WL_PHYTYPE(idx));

    if(wlmngr_getVer(idx,wlver)) {
        wlcsm_strcpy(&(WL_RADIO(idx).wlVersion),wlver);
        WLCSM_TRACE(WLCSM_TRACE_LOG," version is:%s \r\n",WL_RADIO(idx).wlVersion );
    } else
        WLCSM_TRACE(WLCSM_TRACE_ERR," ERROR:could not get adapter version \r\n" );

    snprintf(cmd, sizeof(cmd), "wl%d", idx);
    ret = wl_iovar_get(cmd, "cap", (void *)buf, WL_CAP_STR_LEN);
    if (ret) {
        fprintf(stderr, "%s, get wl%d cap failed, ret=%d\n", __FUNCTION__, idx, ret);
    } else {

        if(strstr(buf, "afterburner")) {
            WL_RADIO(idx).wlHasAfterburner=TRUE;
        }
        if(strstr(buf, "ampdu")) {
            WL_RADIO(idx).wlAmpduSupported=TRUE;
        } else {
            /* A-MSDU doesn't work with AMPDU */
            if(strstr(buf, "amsdu")) {
                WL_RADIO(idx).wlAmsduSupported=TRUE;
            }
        }

        if(!strstr(buf, "wme")) {
            WL_RADIO_WLHASWME(idx)=FALSE;
            for(i=0; i<WL_RADIO_WLNUMBSS(idx); i++) {
                WL_AP_WLWME(idx,i)=FALSE;
            }
        } else
            WL_RADIO_WLHASWME(idx)=TRUE;

        if(strstr(buf, "mbss"))  {
            WL_RADIO_WLSUPPORTMBSS(idx)=TRUE;
        }

        if(strstr(buf, "sta")) {
            WL_RADIO_WLHASAPSTA(idx)=1;
        }

        if(strstr(buf, "vec")) {
            WL_RADIO_WLHASVEC(idx)=1;
        }
#if 0
        if(WL_RADIO_WLBAND(idx)==BAND_A && !WLCSM_STRCMP(WL_PHYTYPE(idx),WL_PHY_TYPE_AC)) {
            bw_cap|=WLC_BW_CAP_80MHZ;
            if(strstr(buf, "160")) {
                bw_cap|=WLC_BW_CAP_160MHZ;
            }
        }

        WL_RADIO_WLBWCAP(idx)=bw_cap;
#endif
    }

#ifdef WMF
    /* for WMF-- we enable then unconditionally for now because of 43602 dongle  */
    WL_RADIO_WLHASWMF(idx)=TRUE;
#endif

    for(i=0; i<WL_RADIO_WLNUMBSS(idx); i++) {
        if ( strncmp(WL_RADIO(idx).wlNmode, WL_OFF,strlen(WL_OFF)) &&
                ( strncmp(WL_PHYTYPE(idx), WL_PHY_TYPE_N,strlen(WL_PHY_TYPE_N)) ||
                  strncmp(WL_PHYTYPE(idx), WL_PHY_TYPE_AC,strlen(WL_PHY_TYPE_AC))) &&
                !strncmp(WL_APSEC(idx,i).wlWpa, "tkip",4))
            wlcsm_strcpy(&(WL_APSEC(idx,i).wlWpa), "tkip+aes");

    }
}


/*************************************************************//* *
  * @brief  internal API to write nvram by parameter values
  *
  * 	adjust wlmngr variables to solve the relationship between
  * 	variables and run_time configurationjust for one adapter,
  *
  * @return void
  ****************************************************************/
void wlmmgr_vars_adjust(const unsigned int idx,SYNC_DIRECTION direction)
{

    _wlmngr_adjust_country_rev(idx, direction);
    _wlmngr_adjust_security(idx, direction);

    if ( direction==WL_SYNC_FROM_DM ) {

        wlmngr_adjust_GPIOOverlays(idx); /* GPIOOverlay is run time, only for this FROM DM direction */

    } else {

        WLCSM_TRACE(WLCSM_TRACE_LOG," !!!!TODO:--- Adjust VAR befor write to DM \r\n" );

    }
    wlmngr_adjust_radio_runtime(idx);
    wlmngr_update_possibble_channels(idx,direction);

#ifdef WLTEST
    if(!nvram_get("_default_restored_")) {
        if (WL_RADIO_WLBAND(idx) == BAND_A)
            WL_RADIO_WLCHANNEL(idx) = 36;
        else
            WL_RADIO_WLCHANNEL(idx) = 1;
    }
#endif
}

void wlmngr_handle_bridge_setup(void) {
    char cmd[WL_SIZE_132_MAX];
    int i,j;
    for (i = 0; i < WL_WIFI_RADIO_NUMBER; i++) {
        for (j = 0 ; j<WL_RADIO_WLMAXMBSS(i); j++) {
            if ( (j==0 || WL_BSSID_WLENBLSSID(i,j)) && WL_RADIO_WLENBL(i))  {
                snprintf(cmd,sizeof(cmd),"brctl addif %s %s 2>/dev/null",WL_BSSID_WLBRNAME(i,j),WL_BSSID_IFNAME(i,j));
                bcmSystem(cmd);
            }
        }
    }
}

/*************************************************************//* *
  * @brief
  *     Parse "br0:eth0:wl0 br1:eth2:wl1" and Construct bridge group string
  *     by use gp_adapter_objs(NVRAM) or Data Model depend on SYNC direction.
  *     When WL_SYNC_FROM_NVRAM, NVRAM have most up to date bridge information
  *     about wifi interface. Use bridge information in lan%d_ifnames over DM .
  *
  * @return void
  ****************************************************************/

void wlcsm_dm_get_bridge_info_adjust(SYNC_DIRECTION direction,char *buf)
{
    int i=0,j=0;
    char brlist[WL_SIZE_256_MAX];
    char name[32];
    char *next=NULL,*lnext=NULL;
    int index=0,bridgeIndex=0;
    int hit=0;
    char br_ifname[128];
    unsigned short brlist_map=0x00;
    char DM_ifbuf[WL_SIZE_512_MAX] = {0};

    wlcsm_dm_get_bridge_info(buf);

    switch (direction) {
    case WL_SYNC_TO_DM:
    {
        /* use NVRAM(WLMNGR) bridge config
         * parse "br0:eth0:wl0 br1:eth2:wl1"
         * reconstruct new list with NVRAM(WLMNGR) setting
         */

        strcpy(DM_ifbuf,buf);
        buf[0]='\0';
        for_each(brlist,DM_ifbuf,lnext) {
            bridgeIndex= -1;
            memset(br_ifname,0,sizeof(br_ifname));

            foreachcolon(name,brlist,next) {
                if(strncmp(name,"br",2) == 0) {
                    sscanf(name,"br%d",&bridgeIndex);
                    brlist_map |= (1<<(bridgeIndex)); //log the bridge already handled
                    strcpy(br_ifname,name);
                    strcat(buf,name);
                }
                else if(strncmp(name,"wl",2) == 0) {
                    /* SKIP , will handle later */
                }
                else {
                    /* nonWL,just append : eth0 , eth2.0 .... */
                    strcat(buf,":");
                    strcat(buf,name);
                }
            }

            /* Add wl that in this bridge */
            if(strlen(br_ifname)!=0) {
                for (i = 0; i < WL_WIFI_RADIO_NUMBER; i++) {
                    for (j = 0 ; j<WL_RADIO_WLMAXMBSS(i); j++) {
                        if(WL_BSSID_WLENBLSSID(i,j) && WL_RADIO_WLENBL(i)) {
                            if(!strcmp(br_ifname,WL_BSSID_WLBRNAME(i,j))) {
                                strcat(buf,":");
                                strcat(buf,WL_BSSID_IFNAME(i,j));
                            }
                        }
                    }
                }
            }

            strcat(buf," ");
        }

        /* Handle the bridge that have wifi interface only */
        for (index = 0; index<MAX_BR_NUM; ++index) {
            if( ((brlist_map) & (1<<index)) == 0 ) {
                int new_br=0;

                sprintf(br_ifname,"br%d",index);

                for (i = 0; i < WL_WIFI_RADIO_NUMBER; i++) {
                    for (j = 0 ; j<WL_RADIO_WLMAXMBSS(i); j++) {
                        if(WL_BSSID_WLENBLSSID(i,j) && WL_RADIO_WLENBL(i)) {
                            if(strcmp(br_ifname,WL_BSSID_WLBRNAME(i,j)) == 0 ) {
                                if(new_br == 0) {
                                    new_br = 1;
                                    strcat(buf,br_ifname);
                                }
                                strcat(buf,":");
                                strcat(buf,WL_BSSID_IFNAME(i,j));
                            }
                        }
                    }
                }

                if(new_br)
                    strcat(buf," ");
            }
        }

        /* Trim last space char */
        for(i=(strlen(g_ifbuf)-1); i>0; i--)
        {
            if(g_ifbuf[i] != ' ')
            {
                g_ifbuf[i+1] = '\0';
                break;
            }
        }
    }
    break;

    default:
    {
        /* use DM bridge config
         * parse "br0:eth0:wl0 br1:eth2:wl1"
         * restore list to NVRAM(WLMNGR) setting
         */

        strcpy(DM_ifbuf,buf);
        for_each(brlist,DM_ifbuf,lnext) {
            bridgeIndex= -1;
            memset(br_ifname,0,sizeof(br_ifname));

            foreachcolon(name,brlist,next) {
                if(strncmp(name,"br",2) == 0) {
                    sscanf(name,"br%d",&bridgeIndex);
                    strcpy(br_ifname,name);
                }
                else if(strncmp(name,"wl",2) == 0) {
                    /* SYNC DM bridge setting back to WLMNGR */
                    for (hit = 0,i = 0; (i<WL_WIFI_RADIO_NUMBER) && (hit == 0); i++) {
                        for (j = 0 ; (j<WL_RADIO_WLMAXMBSS(i)) && (hit == 0); j++) {
                            if(!strcmp(name,WL_BSSID_IFNAME(i,j)))
                            {   // Update bridge name that get from Data Model
                                wlcsm_dm_mngr_set_all_value(i,j,"wlBrName",br_ifname);
                                hit=1;
                                break;
                            }
                        }
                    }

                }
                else {
                    /* nonWL,SKIP : eth0 , eth2.0 .... */
                }
            }
        }
    }


    break;
    }
}


void _wlmngr_bridge_ipalias(void)
{
    #define NVRAM_IPALIAS_ENABLE "ipalias_en"
    char cmd[WL_SIZE_132_MAX]="";
    char ifname[24]="";
    char ipaddr[32]="";
    char ipalias_en[36]="";    
    char *lan_ifname = nvram_safe_get("lan_ifname");

    /* XXX Bring up a virtual interface if required. Ex br0:0 */
    /* XXX This is required since 47189 MoCA has only one ethernet port and Multi-AP */
    /* XXX certification expects 2 IP addresses - for control and test networks */
    sprintf(ifname, "%s:%d", lan_ifname, 0);
    sprintf(ipaddr, "%s_ipaddr", ifname);

    sprintf(ipalias_en, "%s_%s", ifname,NVRAM_IPALIAS_ENABLE);
    
    /* XXX If br0:0_ipaddr nvram is set, configure brX:X interface with that ip address */
    if (WLCSM_STRCMP(nvram_safe_get(ipaddr), "")) {
        fprintf(stderr, "Bringing up %s with ip %s\n", ifname, nvram_safe_get(ipaddr));
		snprintf(cmd,sizeof(cmd),"ifconfig %s %s %s 2>/dev/null",ifname, nvram_safe_get(ipaddr),"up");
		bcmSystem(cmd);
        nvram_set(ipalias_en,"1"); //indicate brX:X alias created by nvram
    }else if(!WLCSM_STRCMP(nvram_safe_get(ipalias_en),"1"))
    { 
       /* unset alias,bring down brX:X */
        fprintf(stderr, "Bringing down %s\n", ifname);
		snprintf(cmd,sizeof(cmd),"ifconfig %s %s 2>/dev/null",ifname,"down");
		bcmSystem(cmd);
        nvram_unset(ipalias_en); // clear indicate ...
    }
}


#ifdef DSLCPE_1905
WLMNGR_1905_CREDENTIAL g_wlmngr_wl_credentials[MAX_WLAN_ADAPTER][WL_MAX_NUM_SSID];
typedef struct struct_value {
    int offset;
    char *name;
    char isInt;
} STRUCT_VALUE;


static int _wlmngr_check_config_status_change (const unsigned int idx, int *bssid,int *credential_changed, int *change_status)
{

    STRUCT_VALUE items[]= {
        {CREDENTIAL_VAROFF(wlSsid),	 	"ssid",		0},
        {CREDENTIAL_VAROFF(wlWpaPsk), 	        "wpa_psk",	0},
        {CREDENTIAL_VAROFF(wlAuthMode), 	"akm",		0},
        {CREDENTIAL_VAROFF(wlAuth),		"auth",		1},
        {CREDENTIAL_VAROFF(wlWep), 		"wep",		0},
        {CREDENTIAL_VAROFF(wlWpa), 		"crypto", 	0},
        {-1,NULL}
    };
    STRUCT_VALUE *item=items;
    WLMNGR_1905_CREDENTIAL    *pmssid;
    int i=0,ret=0;
    char buf[32],name[32],tmp[100],*str;
    int br=0;
    int oob=0;

    *bssid = 0;

    for (i = 0 ; i<WL_RADIO_WLMAXMBSS(idx); i++) {
        br = WL_BSSID_WLBRNAME(idx,i)[2] - 0x30;

        if ( br == 0 )
            snprintf(name, sizeof(name), "lan_wps_oob");
        else
            snprintf(name, sizeof(name), "lan%d_wps_oob", br);
        strncpy(buf, nvram_safe_get(name), sizeof(buf));
        if ( WLCSM_STRCMP(buf,"enabled"))
            oob=2;
        else
            oob=1;

        if(((g_wlmngr_wl_credentials[idx][i].lan_wps_oob)&0xf)!=oob) {
            if(oob==1)  *change_status= WPS_1905_CONF_TO_UNCONF;
            else *change_status= WPS_1905_UNCONF_TO_CONF;
            *bssid=i;
            *credential_changed=0;
            g_wlmngr_wl_credentials[idx][i].lan_wps_oob=oob;
            ret=1;
        }

        if(ret==0)	 {
            if(oob==1)  *change_status= WPS_1905_CONF_NOCHANGE_UNCONFIGURED;
            else *change_status= WPS_1905_CONF_NOCHANGE_CONFIGURED;
        }

        snprintf(name, sizeof(name), "%s_", WL_BSSID_IFNAME(idx,i));
        pmssid=(WLMNGR_1905_CREDENTIAL *)(&(g_wlmngr_wl_credentials[idx][i]));
        for(item=items; item->name!=NULL; item++) {
            str=nvram_get(strcat_r(name, item->name, tmp));
            /* for akm, it is kind of special*/
            if(!WLCSM_STRCMP(item->name,"akm") && (str==NULL||strlen(str)==0)) {
                str="open";
            }
            if(!item->isInt) {
                char *value=CREDENTIAL_STRVARVALUE(pmssid,item->offset);
                if((str==NULL && strlen(value)!=0)|| (str!=NULL && WLCSM_STRCMP(str,value))) {
                    *bssid=i;
                    *credential_changed=1;
                    ret= 1;
                }
                if(ret) {
                    if(!str)  value[0]='\0';
                    else strcpy(value,str);
                }
            } else {
                int *value=CREDENTIAL_INTVARPTR(pmssid,item->offset);
                if(str!=NULL && atoi(str)!=*value) {
                    *bssid=i;
                    ret= 1;
                    *credential_changed=1;
                    *value=atoi(str);
                }

            }
        }
    }
    /* use that bit to indicat if this wlmngr boot time, if not boot time, use return value */
    if(!(g_wlmngr_wl_credentials[idx][0].lan_wps_oob&0x10)) {
        g_wlmngr_wl_credentials[idx][0].lan_wps_oob|=0x10;
        return 0;
    }
    return ret;
}

/** @brief  checking if wlan credential changed
*
*   check if wireless lan credential changed when some application change it from TR69 etc APPs
*/

static int open_udp_socket(char *addr, uint16 port)
{
    int reuse = 1;
    int sock_fd;
    struct sockaddr_in sockaddr;

    /*  open loopback socket to communicate with EAPD */
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(addr);
    sockaddr.sin_port = htons(port);

    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        WLCSM_TRACE(WLCSM_TRACE_LOG, "Unable to create loopback socket\n");
        goto exit0;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
        WLCSM_TRACE(WLCSM_TRACE_LOG, "Unable to setsockopt to loopback socket %d.\n", sock_fd);
        goto exit1;
    }

    if (bind(sock_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
        WLCSM_TRACE(WLCSM_TRACE_LOG, "Unable to bind to loopback socket %d\n", sock_fd);
        goto exit1;
    }
    WLCSM_TRACE(WLCSM_TRACE_LOG, "opened loopback socket %d in port %d\n", sock_fd, port);
    return sock_fd;

    /*  error handling */
exit1:
    close(sock_fd);

exit0:
    WLCSM_TRACE(WLCSM_TRACE_LOG, "failed to open loopback socket\n");
    return -1;
}


/*
* ===  FUNCTION  ======================================================================
*         Name:  _wlmngr_send_notification_1905
*  Description:
* =====================================================================================
*/

static int _wlmngr_send_notification_1905 (const unsigned int idx,int bssid,int credential_changed, int conf_status)
{
    struct sockaddr_in sockAddr;
    int rc;
    int port;
    char *portnum=nvram_safe_get("1905_com_socket");
    char nvramVar[32];
    rc = sscanf(portnum,"%d",&port);
    if ( rc == 1 ) {
        int sock_fd;

        sock_fd=open_udp_socket(WPS_1905_ADDR,WPS_1905_PORT);
        if(sock_fd>=0) {
            int buflen = sizeof(WPS_1905_MESSAGE) + sizeof(WPS_1905_NOTIFY_MESSAGE);
            WPS_1905_NOTIFY_MESSAGE notify_msg;
            WPS_1905_MESSAGE *pmsg = (WPS_1905_MESSAGE *)malloc(buflen);
            if(pmsg) {
                notify_msg.confStatus=conf_status;
                notify_msg.credentialChanged=credential_changed;
                snprintf(notify_msg.ifName, sizeof(notify_msg.ifName), "%s",WL_BSSID_IFNAME(idx,bssid));

                memset(pmsg,'\0',buflen);
                pmsg->cmd=WPS_1905_NOTIFY_CLIENT_RESTART;
                pmsg->len=sizeof(WPS_1905_NOTIFY_MESSAGE);
                pmsg->status=1;
                memcpy((char *)(pmsg+1),&notify_msg,sizeof(WPS_1905_NOTIFY_MESSAGE));

                /*  kernel address */
                memset(&sockAddr, 0, sizeof(sockAddr));
                sockAddr.sin_family      = AF_INET;
                sockAddr.sin_addr.s_addr = htonl(0x7f000001); /*  127.0.0.1 */
                sockAddr.sin_port        = htons(port);

                rc = sendto(sock_fd, pmsg, buflen, 0, (struct sockaddr *)&sockAddr,sizeof(sockAddr));
                free(pmsg);
                close(sock_fd);
                if (buflen != rc) {
                    printf("%s: sendto failed", __FUNCTION__);
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }
    return 1;
}


#endif
/*************************************************************//* *
  * @brief  internal API to write nvram by parameter values
  *
  *	some parameters mapping to nvram entries in the system where
  *	wlconf depends on to configure wl interfaces. This API will read
  *	the mapping and write each single Nvram entry.
  *
  * @return void
  ****************************************************************/
void wlmngr_write_wl_nvram(const unsigned int idx)
{
    char name[128];
    int i=0,j=0,entries_num=0;

    WLCSM_NVRAM_MNGR_MAPPING *mapping;
    WLCSM_WLAN_ADAPTER_STRUCT *adapter;

    adapter=&(gp_adapter_objs[idx]);
    char *value,tmp_str[1024];
    entries_num=sizeof(g_wlcsm_nvram_mngr_mapping)/sizeof(WLCSM_NVRAM_MNGR_MAPPING);

    for(i=0; i<adapter->radio.wlNumBss; i++) {

        for ( j=0; j<entries_num; j++ ) {

            mapping= &(g_wlcsm_nvram_mngr_mapping[j]);
            if(mapping->type==MNGR_GENERIC_VAR) continue;
            else if((i==0 || ((i>0) && mapping->type==MNGR_SSID_SPECIFIC_VAR))) {
                if(!i)
                    snprintf(name, sizeof(name), "wl%d_%s",idx,mapping->nvram_var);
                else
                    snprintf(name, sizeof(name), "wl%d.%d_%s",idx,i,mapping->nvram_var);

                WLCSM_TRACE(WLCSM_TRACE_LOG," j:%d,name of nvram is:%s \r\n",j,name);

                value=wlcsm_mapper_get_mngr_value(idx,i, mapping,tmp_str);
                if(value) {
                    /* if bw_cap is -1, we don't overwritten it as only manual opertaion can set it to -1
                     * make this change for UTF testing to make it exactly work as HNDrouter */
                    if(!WLCSM_STRCMP(mapping->nvram_var,"bw_cap") && !WLCSM_STRCMP(wlcsm_nvram_get(name),"-1"))
                        continue;
                    wlcsm_nvram_set(name,value);
                }
                WLCSM_TRACE(WLCSM_TRACE_LOG," j:%d,name of nvram is:%s and value:%s \r\n",j,name,value?value:"NULL");
            }
        }
    }
}


#if defined(HSPOT_SUPPORT)

static void wlmngr_HspotCtrl(void)
{

    /* First to kill all hspotap process if already start*/
    bcmSystem("killall -q -15 hspotap 2>/dev/null");
    bcmSystem("hspotap&");
}
#endif

static bool enableBSD(void)
{
    int i=0,ret=FALSE;
    char buf[WL_MID_SIZE_MAX];
    nvram_set("bsd_role","0");
    if( act_wl_cnt == 0 ) return FALSE;
    for (i=0; i<WL_WIFI_RADIO_NUMBER; i++) {
        if( WL_RADIO_WLENBL(i) == TRUE && WL_RADIO_BSDROLE(i) > 0 ) {
            snprintf(buf, sizeof(buf), "%d", WL_RADIO_BSDROLE(i));
            nvram_set("bsd_role",buf);
            snprintf(buf, sizeof(buf), "%d", WL_RADIO_BSDPPORT(i));
            nvram_set("bsd_pport",buf);
            snprintf(buf, sizeof(buf), "%d", WL_RADIO_BSDHPORT(i));
            nvram_set("bsd_hpport",buf);
            snprintf(buf, sizeof(buf), "%s", WL_RADIO_BSDHELPER(i));
            nvram_set("bsd_helper",buf);
            snprintf(buf, sizeof(buf), "%s", WL_RADIO_BSDPRIMARY(i));
            nvram_set("bsd_primary",buf);
            ret=TRUE;
            break;
        }
    }

#ifdef BCM_WBD
    ret=TRUE;
#endif
    return ret;
}

static void wlmngr_BSDCtrl(void )
{
    /* First to kill all bsd process if already start*/
    bcmSystem("killall -q -15 bsd 2>/dev/null");
    if ( enableBSD() == TRUE )
        bcmSystem("bsd&");
}

static bool enableSSD()
{
    int i=0;
    char buf[WL_MID_SIZE_MAX];
    nvram_set("ssd_enable","0");
    if( act_wl_cnt == 0 ) return FALSE;
    for (i=0; i<WL_WIFI_RADIO_NUMBER; i++)
        if( WL_RADIO_WLENBL(i) == TRUE && WL_RADIO_SSDENABLE(i) > 0 ) {
            snprintf(buf, sizeof(buf), "%d", WL_RADIO_SSDENABLE(i));
            nvram_set("ssd_enable",buf);
            return TRUE;
        }
    return FALSE;
}

static void wlmngr_SSDCtrl(void)
{
    /* First to kill all ssd process if already start*/
    bcmSystem("killall -q -15 ssd 2>/dev/null");
    if ( enableSSD() == TRUE )
        bcmSystem("ssd&");
}

static unsigned int _bits_count(unsigned int  n)
{
    unsigned int count = 0;
    while ( n > 0 ) {
        if ( n & 1 )
            count++;
        n >>= 1;
    }
    return count;
}

void wlmngr_startAcsd(void)
{
    int timeout = 0;
    char cmd[WL_SIZE_132_MAX];
    int i=0,j=0,enabled=0;
    for(i=0; i<WL_WIFI_RADIO_NUMBER; i++) {
        for (j = 0 ; j<WL_RADIO_WLMAXMBSS(i); j++) {
            if(WL_BSSID_WLENBLSSID(i,j) && WL_RADIO_WLENBL(i)) {
                enabled=1;
                break;
            }
        }
        if(enabled) break;
    }
    if(!enabled) return;
    if (wlmngr_detectApp("acsd2") && nvram_match("acs_version","2"))  {
        bcmSystem("acsd2");
    } else if (wlmngr_detectApp("acsd")) {
        bcmSystem("acsd");
        for(i=0; i<WL_WIFI_RADIO_NUMBER; i++) {
            timeout = WL_RADIO(i).wlCsScanTimer*60;
            if(timeout) {
                snprintf(cmd, sizeof(cmd), "acs_cli -i %s acs_cs_scan_timer %d", WL_BSSID_IFNAME(i,0), timeout);
                bcmSystem(cmd);
            }
        }
    }
}

#if defined(BCM_APPEVENTD)
int wlmngr_start_appeventd(void)
{
    if (nvram_match("appeventd_enable", "1"))
    {
        bcmSystem("appeventd&");
    }

    return 1;
}

int wlmngr_stop_appeventd(void)
{
    bcmSystem("killall -q -9 appeventd 2>/dev/null");
    return 1;
}
#endif

static bool wlmngr_detectApp(char *app)
{
    FILE *fp;
    char *files[]= {"/bin","/usr/sbin" };
    char temp[128];
    int i=0;
    for(; i<2; i++) {
        snprintf(temp,128,"%s/%s",files[i],app);
        fp = fopen(temp, "r");
        if ( fp != NULL ) {
            fclose(fp);
            return TRUE;
        }
    }
    return FALSE;
}


#ifdef WL_AIR_IQ
static void wlmngr_startAirIQ(void) {
    int index=0;
    char temp[128];
    if(wlmngr_detectApp("airiq_service") && (nvram_match("airiq_service_enable", "1") ||
            nvram_match("airiq_service_enable", "y"))) {
        bcmSystem("/usr/sbin/airiq_service -c /usr/sbin/airiq_service.cfg"
                  "	-pfs /usr/sbin/flash_policy.xml &");
        sleep(2);
        for (index = 0; index  <WL_WIFI_RADIO_NUMBER; ++index) {
            snprintf(temp,128,"%s_airiq_enable",WL_BSSID_IFNAME(index,0));
            if(nvram_match(temp,"y")||nvram_match(temp,"1")) {
                snprintf(temp,128,"airiq_app -i %s >/dev/null &",WL_BSSID_IFNAME(index,0));
                bcmSystem(temp);
            }
        }
    }
}

static void wlmngr_stopAirIQ(void) {
    if(wlmngr_detectApp("airiq_service") && (nvram_match("airiq_service_enable", "1") ||
            nvram_match("airiq_service_enable", "y"))) {
        bcmSystem("killall -q -9 airiq_service");
        bcmSystem("killall  airiq_app");
    }
}
#endif

#if defined(WL_BSTREAM_IQOS)
static int
start_broadstream_iqos(void)
{
    if (!nvram_match("broadstream_iqos_enable", "1")) {
        return 0;
    }

    bcmSystem("bcmiqosd start");
    return 0;
}

static void
stop_broadstream_iqos(void)
{
    bcmSystem("bcmiqosd stop > /dev/null");
}
#endif /* WL_BSTREAM_IQOS */
void wlmngr_postStart_Service(void)
{
#if defined(CONFIG_HOSTAPD) && defined(BCA_CPEROUTER)
    bcmSystem("wlhostapd");
#endif
#if defined(HSPOT_SUPPORT)
    wlmngr_HspotCtrl();
#endif
#ifdef __CONFIG_TOAD__
    bcmSystem("toad");
#endif
#ifdef __CONFIG_VISUALIZATION__
    bcmSystem("vis-dcon");
    bcmSystem("vis-datacollector");
#endif
#if defined(BCM_APPEVENTD)
    wlmngr_start_appeventd();
#endif
#if defined(WL_BSTREAM_IQOS)
    start_broadstream_iqos();
#endif
#ifdef WL_AIR_IQ
    wlmngr_startAirIQ();
#endif
}


#ifdef BCM_WBD
void  wlmngr_start_wbd(void)
{
    bcmSystem("wbd_master");
    bcmSystem("wbd_slave");
}
#endif


void wlmngr_pre_setup(const unsigned int idx )
{

    char *restart;
    restart = nvram_get("wps_force_restart");
    if ( restart!=NULL && (strncmp(restart, "y", 1)!= 0) &&  (strncmp(restart, "Y", 1)!= 0)  ) {
        nvram_set("wps_force_restart", "Y");
    } else {
        wlmngr_wlIfcDown(idx);
        wlmngr_WlConfDown(idx);
    }
}


static int wlmngr_stop_wps(void)
{
    int ret = 0;
    FILE *fp = NULL;
    char saved_pid[32],cmd[64];
    int i, wait_time = 3;
    pid_t pid;
#if defined(CONFIG_HOSTAPD) && defined(BCA_CPEROUTER)
    /*kill possible wps_pbcd process as the same image
     * can enable and disable hostapd, so need to stop
     * wps_monitor and wps_pbcd */
    bcmSystem("killall wps_pbcd");
#endif
    if (((fp = fopen("/tmp/wps_monitor.pid", "r")) != NULL) &&
            (fgets(saved_pid, sizeof(saved_pid), fp) != NULL)) {
        /* remove new line first */
        for (i = 0; i < sizeof(saved_pid); i++) {
            if (saved_pid[i] == '\n')
                saved_pid[i] = '\0';
        }
        saved_pid[sizeof(saved_pid) - 1] = '\0';
        snprintf(cmd,64,"kill %s",saved_pid);
        bcmSystem(cmd);

        do {
            if ((pid = get_pid_by_name("/bin/wps_monitor")) <= 0)
                break;
            wait_time--;
            sleep(1);
        } while (wait_time);

        if (wait_time == 0) {
            printf("Unable to kill wps_monitor!\n");
            ret=1;
        }
    }
    if (fp) {
        fclose(fp);
        if(!ret)
            bcmSystem("rm -rf /tmp/wps_monitor.pid");
    }

    return ret;
}

void wlmngr_setup(unsigned int idx )
{

    char cmd[WL_SIZE_132_MAX];
#ifdef SUPPORT_WSC
    char *restart;
#endif
    //  wlmngr_devicemode_overwrite(idx);
    snprintf(cmd, sizeof(cmd), "wl -i wl%d phy_watchdog 0", idx);
    WLCSM_TRACE(WLCSM_TRACE_LOG," cmd:%s \r\n",cmd );
    BCMWL_WLCTL_CMD(cmd);
#ifdef SUPPORT_WSC
    restart = nvram_get("wps_force_restart");
    if ( restart!=NULL && (strncmp(restart, "y", 1)!= 0) &&  (strncmp(restart, "Y", 1)!= 0)  ) {
        nvram_set("wps_force_restart", "Y");
    }
#endif
    wlmngr_setupMbssMacAddr(idx);
    wlmngr_enum_ifnames(idx);
    if (!wlmngr_detectApp("acsd") && !wlmngr_detectApp("acsd2"))
        wlmngr_autoChannel(idx);
    while(!g_wlmngr_ready_for_event)
        sleep(1);
    wlmngr_doWlConf(idx);
    wlmngr_setup_if_mac(idx);
    wlmngr_doSecurity(idx);
    wlmngr_doWdsSec(idx);
    wlmngr_wlIfcUp(idx);
}

//**************************************************************************
// Function Name: doQoS
// Description  : setup ebtables marking for wireless interface.
// Parameters   : none.
// Returns      : None.
//**************************************************************************
static void wlmngr_doQoS(unsigned int idx)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG," TODO: wlmngr_doQOS, how to .... \r\n" );
    if (!WL_AP_WLWME(idx,0))
        return;

#if 0
#ifdef DMP_QOS_1
    /* all of this code assumes the old TR98 way of doing things.
     * In TR181, the interface between wlan and CMS will be cleaner.
     */
    if (m_instance_wl[idx].m_wlVar.wlWme) {
        for (i=0; i<WL_RADIO(idx).wlNumBss; i++) {
            if (WL_BSSID_WLENBLSSID(idx,i) && m_instance_wl[idx].m_wlVar.wlEnbl) {
                /* enable all the default queues associated with this ssid. */
                if ((ret = dalQos_setDefaultWlQueuesLock(WL_BSSID_IFNAME(idx,i), TRUE)) != CMSRET_SUCCESS) {
                    printf("dalQos_setDefaultWlQueuesLock() returns error. ret=%d\n", ret);
                    return;
                }
            } else {
                /* disable all the default queues associated with this ssid. */
                if ((ret = dalQos_setDefaultWlQueuesLock(WL_BSSID_IFNAME(idx,i), FALSE)) != CMSRET_SUCCESS) {
                    printf("dalQos_setDefaultWlQueuesLock() returns error. ret=%d\n", ret);
                    return;
                }
            }
        }
    } else {
        /* disable all the default wireless queues */
        for (i=0; i<WL_RADIO(idx).wlNumBss; i++) {
            if ((ret = dalQos_setDefaultWlQueuesLock(WL_BSSID_IFNAME(idx,i), FALSE)) != CMSRET_SUCCESS) {
                printf("dalQos_setDefaultWlQueuesLock() returns error. ret=%d\n", ret);
                return;
            }
        }
    }

#endif /* DMP_QOS_1 */
#endif /* DMP_QOS_1 */
}


void wlmngr_post_setup(unsigned int idx ) {

    char cmd[64];
    wlmngr_WlConfStart(idx);
    if ( WL_RADIO_WLENBL(idx) == TRUE ) {
        wlmngr_wlIfcUp(idx);
    }

    wlmngr_doQoS(idx);
    snprintf(cmd, sizeof(cmd), "wl -i wl%d phy_watchdog 1", idx);
    BCMWL_WLCTL_CMD(cmd);

    snprintf(cmd, sizeof(cmd), "wl -i wl%d fcache 1", idx);
    BCMWL_WLCTL_CMD(cmd);

    // send ARP packet with bridge IP and hardware address to device
    // this piece of code is -required- to make br0's mac work properly
    // in all cases
    snprintf(cmd, sizeof(cmd), "/sbin/sendarp -s %s -d %s", WL_BSSID(idx,0).wlBrName,WL_BSSID(idx,0).wlBrName);
    bcmSystem(cmd);

    if( wlmngr_getCoreRev(idx) >= 40 ) {
        char tmp[100], prefix[] = "wlXXXXXXXXXX_";
        int txchain;
        snprintf(prefix, sizeof(prefix), "%s_", WL_BSSID_IFNAME(idx,0));
        wlmngr_getVarFromNvram(&txchain, strcat_r(prefix, "txchain", tmp), "int");

        if( _bits_count((unsigned int) txchain) > 1)
            WL_RADIO_WLTXBFCAPABLE(idx)=1;
    }
    wlmngr_setASPM(idx);
}



//**************************************************************************
// Function Name: clearSesLed
// Description  : clear SES LED.
// Parameters   : none.
// Returns      : none.
//**************************************************************************
void wlmngr_clearSesLed(void )
{
    int f = open( "/dev/brcmboard", O_RDWR );
    /* set led off */
    if( f > 0 ) {
        int  led = 0;
        BOARD_IOCTL_PARMS IoctlParms;
        memset( &IoctlParms, 0x00, sizeof(IoctlParms) );
        IoctlParms.result = -1;
        IoctlParms.string = (char *)&led;
        IoctlParms.strLen = sizeof(led);
        ioctl(f, BOARD_IOCTL_SET_SES_LED, &IoctlParms);
        close(f);
    }
}


#if defined(SUPPORT_WSC)
void wlmngr_startWsc(void)
{
    int i=0, j=0;
    char buf[64];
    char *buf1;

    int br;
    char ifnames[128];

    strncpy(buf, nvram_safe_get("wl_unit"), sizeof(buf));

    if (buf[0] == '\0') {
        nvram_set("wl_unit", "0");
        i = 0;
        j = 0;
    } else {
        if ((buf[1] != '\0') && (buf[2] != '\0')) {
            buf[3] = '\0';
            j = isdigit(buf[2])? atoi(&buf[2]):0;
        } else {
            j = 0;
        }

        buf[1] = '\0';
        i = isdigit(buf[0]) ? atoi(&buf[0]):0;
    }

    WLCSM_TRACE(WLCSM_TRACE_LOG," .............. \r\n" );
    nvram_set("wps_mode", WL_APWPS_WLWSCMODE(i,j)); //enabled/disabled
    nvram_set("wl_wps_config_state", WL_APWPS_WLWSCAPMODE(i,j)); // 1/0
    nvram_set("wl_wps_reg",         "enabled");
    if (strlen(nvram_safe_get("wps_version2")) == 0)
        nvram_set("wps_version2", "enabled");
    /* Since 5.22.76 release, WPS IR is changed to per Bridge. Previous IR enabled/disabled is
    Per Wlan Intf */

    for ( br=0; br<MAX_BR_NUM; br++ ) {
        if ( br == 0 )
            snprintf(buf, sizeof(buf), "lan_ifnames");
        else
            snprintf(buf, sizeof(buf), "lan%d_ifnames", br);
        buf1=nvram_get(buf);
        if(!buf1) continue;
        else  strncpy(ifnames, buf1, sizeof(ifnames));

        if (ifnames[0] =='\0')
            continue;
        if ( br == 0 )
            snprintf(buf, sizeof(buf), "lan_wps_reg");
        else
            snprintf(buf, sizeof(buf), "lan%d_wps_reg", br);
        nvram_set(buf, "enabled");
    }

    if (nvram_get("wps_config_method") == NULL) { /* initialization */
        set_wps_config_method_default(); /* for DTM 1.1 test */
        if (nvram_match("wps_version2", "enabled"))
            nvram_set("_wps_config_method", "sta-pin"); /* This var is only for WebUI use, default to sta-pin */
        else
            nvram_set("_wps_config_method", "pbc"); /* This var is only for WebUI use, default to PBC */
    }

    nvram_set("wps_uuid",           "0x000102030405060708090a0b0c0d0ebb");
    nvram_set("wps_device_name",    "BroadcomAP");
    nvram_set("wps_mfstring",       "Broadcom");
    nvram_set("wps_modelname",      "Broadcom");
    nvram_set("wps_modelnum",       "123456");
    nvram_set("boardnum",           "1234");
    nvram_set("wps_timeout_enable",	"0");

    nvram_set("wps_config_command", "0");
    nvram_set("wps_status", "0");
    nvram_set("wps_method", "1");
    nvram_set("wps_config_command", "0");
    nvram_set("wps_proc_mac", "");
    nvram_set("wps_sta_pin", "00000000");
    nvram_set("wps_currentband", "");
    nvram_set("wps_autho_sta_mac", "00:00:00:00:00:00");
	nvram_set("router_disable", "0");

    if (strlen(nvram_safe_get("wps_device_pin")) != 8)
        wl_wscPinGen();

    if (nvram_match("wps_restart", "1")) {
        nvram_set("wps_restart", "0");
    } else {
        nvram_set("wps_restart", "0");
        nvram_set("wps_proc_status", "0");
    }

#if defined(CONFIG_HOSTAPD) && defined(BCA_CPEROUTER)
    if (nvram_match("hapd_enable", "0"))
#endif
        bcmSystem("wps_monitor&");
}

#endif //end of SUPPORT_WSC


#if defined(SUPPORT_WSC)
void set_wps_config_method_default(void)
{
    if (nvram_match("wps_version2", "enabled")) {
#if defined(CONFIG_HOSTAPD) && defined(BCA_CPEROUTER)
        if (nvram_match("hapd_enable", "0")) {
#endif
            nvram_set("wps_config_method", "0x228c");
#if defined(CONFIG_HOSTAPD) && defined(BCA_CPEROUTER)
        } else {
            nvram_unset("wps_config_method");
        }
#endif
        /* WPS_UI_MEHTOD_PBC */
        nvram_set("wps_method", "2");

    } else
        nvram_set("wps_config_method", "0x84");
}
#endif /* SUPPORT_WSC */

#ifdef  __CONFIG_RPCAPD__
int
start_rpcapd(void)
{
    int i=0,ret=0,rpcapd_enabled=0;
    char temp[64], *nv_value;
    char to_restart=0;
    for (i=0; i<WL_WIFI_RADIO_NUMBER; i++) {
        if (!WLCSM_STRCMP(WL_BSSID_WLMODE(i,0), "monitor")) {
            rpcapd_enabled=1;
            snprintf(temp,64,"dhd%d_rnr_rxoffl",i);
            nv_value=wlcsm_nvram_get(temp);
            if(!nv_value || !strncmp(nv_value,"1",1)) {
                /*need to use system shell to set nvram in order to
                 *available to kernel,need to adjust  */
                snprintf(temp,64,"nvram set dhd%d_rnr_rxoffl=0",i);
                bcmSystem(temp);
                /*mark it changed and restore back if change back
                 *to other mode */
                snprintf(temp,64,"nvram set dhd%d_rnr_rxoffl_changed=1",i);
                bcmSystem(temp);
                to_restart=1;
            }
        } else {
            snprintf(temp,64,"dhd%d_rnr_rxoffl_changed",i);
            /*restore it back to rxoffl enable mode*/
            if(wlcsm_nvram_get(temp)) {
                snprintf(temp,64,"nvram unset dhd%d_rnr_rxoffl",i);
                bcmSystem(temp);
                snprintf(temp,64,"nvram unset dhd%d_rnr_rxoffl_changed",i);
                bcmSystem(temp);
                to_restart=1;
            }
        }
    }
    if(to_restart) {
        bcmSystem("nvram kcommit");
        wlcsm_dm_save_config(0,0,wlmngr_thread_lock);
        fprintf(stderr, "Board reboots to enable runner offload...\n");
        bcmSystem("reboot");
    } else if(rpcapd_enabled) {

        ret=eval("rpcapd", "-d", "-n");
    }
    return ret;
}

int
stop_rpcapd(void)
{
    int ret = eval("killall", "rpcapd");
    return ret;
}
#endif /* __CONFIG_RPCAPD__ */

//**************************************************************************
// Function Name: wlmngr_stopMonitor
// Description  : stop dhd (debug) monitor
// Parameters   : None
// Returns      : None
//**************************************************************************
void wlmngr_stopMonitor(void)
{
    /* remove debug_monitor directory */
    bcmSystem("rm -rf /tmp/dm");

    /* Don't kill debug_monitor here */

    return;
}

//**************************************************************************
// Function Name: wlmngr_startMonitor
// Description  : start dhd (debug) monitor
// Parameters   : None
// Returns      : result of starting the monitor application
//**************************************************************************
int wlmngr_startMonitor(void)
{
    char monitor[16] = "debug_monitor";
    char cmd[128];
    char *crash_log_backup_dir;
    int ret;

    /*
     * Check which monitor to use debug or dhd ?  default is debug
     * BISON uses dhd_monitor and KUDU uses debug_monitor
     * Check can be removed if BISON move to debug_monitor
     */
    if ((wlmngr_detectApp("debug_monitor") == FALSE) &&
            (wlmngr_detectApp("dhd_monitor") == TRUE)) {
        snprintf(monitor, sizeof(monitor), "%s", "dhd_monitor");
    }

    /* Kill previous instance of monitor before invoking new one */
    snprintf(cmd, sizeof(cmd), "killall -q -9 %s 2>/dev/null", monitor);
    ret = bcmSystem(cmd);
    usleep(300000);

    /* Get the backup directory setting from nvram */
    crash_log_backup_dir = nvram_safe_get("crash_log_backup_dir");
    if(*crash_log_backup_dir == '\0') {
        cprintf("WARNING: nvram crash_log_backup_dir for %s is not set!\n", monitor);
    } else {
        snprintf(cmd, sizeof(cmd), "mkdir -p %s",crash_log_backup_dir);
        ret = bcmSystem(cmd);
    }

    /* make sure the directoy is there to mute possible warning */
    bcmSystem("mkdir -p /tmp/dm");
    // handle the things to mount/umount if crash_log_backup_mtd is set/unset
    wlmngr_crash_log_backup_init();
    /* Start the monitor */
    snprintf(cmd, sizeof(cmd), "%s %s", monitor, crash_log_backup_dir);
    ret = bcmSystem(cmd);

    return ret;
}

//**************************************************************************
// Function Name: stopServices
// Description  : stop deamon services
//                which is required/common for each reconfiguration
// Parameters   : None
// Returns      : None
//**************************************************************************
void wlmngr_stopServices(void)
{
    wlmngr_stopMonitor();

#ifdef SUPPORT_WSC
    bcmSystem("killall -q -9 wps_ap 2>/dev/null");
    bcmSystem("killall -q -9 wps_enr 2>/dev/null");
    wlmngr_stop_wps();
    wlmngr_clearSesLed();
#endif
#if defined(CONFIG_HOSTAPD) && defined(BCA_CPEROUTER)
    stop_hapd_wpasupp();
#endif
    bcmSystem("killall -q -9 nas 2>/dev/null");
    bcmSystem("killall -q -9 eapd 2>/dev/null");
    bcmSystem("killall ceventd  2>/dev/null");
    bcmSystem("killall -q -9 acsd 2>/dev/null");
    bcmSystem("killall -q -9 acsd2 2>/dev/null");
#ifdef BCMWAPI_WAI
    bcmSystem("killall -q -15 wapid");
#endif
#if defined(HSPOT_SUPPORT)
    bcmSystem("killall -q -15 hspotap");
#endif

    bcmSystem("killall -q -15 bsd");
    bcmSystem("killall -q -15 ssd");
#ifdef __CONFIG_TOAD__
    bcmSystem("killall -q -15 toad");
#endif
#ifdef __CONFIG_VISUALIZATION__
    bcmSystem("killall -q -9 vis-datacollector");
    bcmSystem("killall -q -9 vis-dcon");
#endif
#ifdef BCM_WBD
    bcmSystem("killall wbd_master");
    bcmSystem("killall wbd_slave");
#endif
#if defined(BCM_APPEVENTD)
    wlmngr_stop_appeventd();
#endif
#ifdef  __CONFIG_RPCAPD__
    stop_rpcapd();
#endif /* __CONFIG_RPCAPD__ */
#if defined(WL_BSTREAM_IQOS)
    stop_broadstream_iqos();
#endif

#ifdef WL_AIR_IQ
    wlmngr_stopAirIQ();
#endif
    usleep(300000);

}


//**************************************************************************
// Function Name: startService
// Description  : start deamon services
//                which is required/common for each reconfiguration
// Parameters   : None
// Returns      : None
//**************************************************************************
void wlmngr_startServices(void)
{

    if (act_wl_cnt == 0)
        return;  /* all adapters are disabled */

    bcmSystem("eapd");
    if(nvram_match("ceventd_enable","1") && wlmngr_detectApp("ceventd"))
        bcmSystem("ceventd");

#if defined(CONFIG_HOSTAPD) && defined(BCA_CPEROUTER)
    if (nvram_match("hapd_enable", "0"))
#endif
        bcmSystem("nas");

    wlmngr_startMonitor();


#ifdef SUPPORT_WSC
    wlmngr_startWsc();
#endif
#ifdef BCMWAPI_WAI
    bcmSystem("wapid");
#endif
    wlmngr_BSDCtrl();
    wlmngr_SSDCtrl();

#ifdef __CONFIG_EXTACS__
    /* Usermode autochannel */
    wlmngr_startAcsd();
#endif

#ifdef  __CONFIG_RPCAPD__
    start_rpcapd();
#endif
}

//**************************************************************************
// Function Name: getVar
// Description  : get value by variable name.
// Parameters   : var - variable name.
// Returns      : value - variable value.
//**************************************************************************
void wlmngr_getVar(const unsigned int idx, char *varName, char *varValue)

{
    WLCSM_TRACE(WLCSM_TRACE_LOG," TODO:get var \r\n" );
}



//**************************************************************************
// Function Name: wlmngr_getVarFromNvram
// Description  : retrieve var from nvram by name
// Parameters   : var, name, and type
// Returns      : none
//**************************************************************************
void wlmngr_getVarFromNvram(void *var, const char *name, const char *type)
{
    char temp_s[256] = {0};
    int len;

    strncpy(temp_s, nvram_safe_get(name), sizeof(temp_s));

    if(!WLCSM_STRCMP(type,"int")) {
        if(*temp_s)
            *(int*)var = atoi(temp_s);
        else
            *(int*)var = 0;
    } else if(!WLCSM_STRCMP(type,"string")) {
        if(*temp_s) {
            len = strlen(temp_s);

            /* Don't truncate tail-space if existed for SSID string */
            if (strstr(name, "ssid") == NULL) {
                if ((strstr(name, "wpa_psk") == NULL) && (len > 0) && (temp_s[len - 1] == ' '))
                    temp_s[len - 1] = '\0';
            }
            strcpy((char*)var,temp_s);
        } else {
            *(char*)var = 0;
        }
    } else {
        printf("wlmngr_getVarFromNvram:type not found\n");
    }
}

/*** STALIST implementations ***/

#define NEXT_STA(sta)   (((WLCSM_WLAN_AP_STA_STRUCT *)(sta))->next)
#define PREV_STA(sta)   (((WLCSM_WLAN_AP_STA_STRUCT *)(sta))->prev)

#ifdef WLCSM_DEBUG
static void  _wlmngr_print_ap_stalist(int radio_idx,int sub_idx) {
    WLCSM_WLAN_AP_STA_STRUCT  *cur_sta;
    cur_sta= WL_AP_STAS(radio_idx,sub_idx);
    while(cur_sta!=NULL) {
        fprintf(stderr, " sta:%s associated:%d and authorized:%d\n",cur_sta->macAddress,cur_sta->associated,cur_sta->authorized );
        cur_sta=NEXT_STA(cur_sta);
    }
}
#endif

static inline WLCSM_WLAN_AP_STA_STRUCT *
_wlmngr_find_sta(const unsigned int radio_idx,unsigned int sub_idx,char *mac) {
    WLCSM_WLAN_AP_STA_STRUCT  *cur_sta = WL_AP_STAS(radio_idx,sub_idx);
    while(cur_sta!=NULL) {
        if(!WLCSM_STRCMP(cur_sta->macAddress,mac)) return cur_sta;
        cur_sta=NEXT_STA(cur_sta);
    }
    return NULL;
}

static inline void _wlmngr_add_sta(int radio_idx,int sub_idx, WLCSM_WLAN_AP_STA_STRUCT *associated_sta) {
    WLCSM_WLAN_AP_STA_STRUCT *cur_sta= WL_AP_STAS(radio_idx,sub_idx);
    if(cur_sta) {
        cur_sta->prev=associated_sta;
    }
    associated_sta->next=cur_sta;
    associated_sta->prev=NULL;
    WL_AP_STAS(radio_idx,sub_idx)=associated_sta;
    WL_AP_NUMSTATIONS(radio_idx,sub_idx)++;
}

static inline void _wlmngr_remove_sta(int radio_idx,int sub_idx, WLCSM_WLAN_AP_STA_STRUCT *associated_sta) {
    if(PREV_STA(associated_sta)==NULL) { /*To delete the stalist head */
        WL_AP_STAS(radio_idx,sub_idx)=NEXT_STA(associated_sta);
        if(NEXT_STA(associated_sta)) {
            PREV_STA(WL_AP_STAS(radio_idx,sub_idx))=NULL;
        }
    } else { /*To delete sta from the middle or end of the list */
        NEXT_STA(PREV_STA(associated_sta))=NEXT_STA(associated_sta);
        if(NEXT_STA(associated_sta)) {
            PREV_STA(NEXT_STA(associated_sta))=PREV_STA(associated_sta);
        }
    }
    free(associated_sta->macAddress);
    free(associated_sta);
    WL_AP_NUMSTATIONS(radio_idx,sub_idx)--;
}

static int _wlmngr_clear_ap_stalist(int radio_idx,int sub_idx) {

    WLCSM_WLAN_AP_STA_STRUCT  *cur_sta;
    WLCSM_TRACE(WLCSM_TRACE_DBG, "wl%d.%d has STA:%d\n",radio_idx,sub_idx,WL_AP_NUMSTATIONS(radio_idx,sub_idx));
    while((cur_sta=WL_AP_STAS(radio_idx,sub_idx))!=NULL) {
        WLCSM_TRACE(WLCSM_TRACE_DBG, "remote:%p\n",cur_sta);
        _wlmngr_remove_sta(radio_idx,sub_idx,cur_sta);
    }
    return WL_AP_NUMSTATIONS(radio_idx,sub_idx)!=0;
}

static inline int _wlmngr_fresh_ap_stalist(int radio_idx,int sub_idx) {
    WL_STALIST_SUMMARIES *sta_summaries = wlcsm_wl_get_sta_summary(radio_idx,sub_idx);
    WLCSM_WLAN_AP_STA_STRUCT *associated_sta = NULL;
    int assigned=0,ret=0,i=0;
    if(sta_summaries) {

        WLCSM_TRACE(WLCSM_TRACE_DBG, "wl%d.%d has STA:%d\n",radio_idx,sub_idx,WL_AP_NUMSTATIONS(radio_idx,sub_idx));
        for(i=0; i<sta_summaries->num_of_stas; i++) {
            assigned=0;
            associated_sta = (WLCSM_WLAN_AP_STA_STRUCT *)malloc(sizeof(WLCSM_WLAN_AP_STA_STRUCT));
            if(associated_sta) {
                associated_sta->macAddress=malloc(MACSTR_LEN);
                if(associated_sta->macAddress) {
                    WLCSM_TRACE(WLCSM_TRACE_DBG, "summary sta mac:%s\n",sta_summaries->stalist_summary[i].macAddress);
                    memcpy(associated_sta->macAddress,sta_summaries->stalist_summary[i].macAddress,MACSTR_LEN);
                    WLCSM_TRACE(WLCSM_TRACE_DBG, "associated_sta->macAddress:%s\n",associated_sta->macAddress);
                    associated_sta->associated=sta_summaries->stalist_summary[i].associated;
                    associated_sta->authorized=sta_summaries->stalist_summary[i].authorized;
                    assigned=1;
                    WLCSM_TRACE(WLCSM_TRACE_DBG, "wl%d.%d has STA:%d\n",radio_idx,sub_idx,WL_AP_NUMSTATIONS(radio_idx,sub_idx));
                    _wlmngr_add_sta(radio_idx,sub_idx,associated_sta);
                }
            }
            if(!assigned) {
                fprintf(stderr, "%s:%d: could not allocate memory for STAS	\n",__FUNCTION__,__LINE__);
                ret=1;
            }
        }
    }
    return ret;
}

void wlmngr_update_stalist(int radio_idx) {

    WL_STA_EVENT_DETAIL sta_summary;
    int sub_idx;
    if(g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_STAS_CHANGED].func) {
        for (sub_idx=0; sub_idx<WL_RADIO(radio_idx).wlNumBss; sub_idx++) {
            if(_wlmngr_clear_ap_stalist(radio_idx,sub_idx))
                fprintf(stderr, "%s:%d:	 AP's stalist is not fully removed?????\n",__FUNCTION__,__LINE__);
            if (WL_BSSID_WLENBLSSID(radio_idx,sub_idx)) {
                sta_summary.event_type=UPDATE_STA_ALL;
                _wlmngr_fresh_ap_stalist(radio_idx,sub_idx);
            } else  {
                sta_summary.event_type=UPDATE_STA_REMOVE_ALL;
            }

            g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_STAS_CHANGED].func(radio_idx,sub_idx, &sta_summary);
        }
    }

#ifdef IDLE_PWRSAVE
    wlmngr_togglePowerSave();
#endif

}

/****** end of stalist implementation *****/

#ifdef HSPOT_SUPPORT
void wlmngr_hspot_default(void) {
    int i=0, j;
    char temp_buf[32]= {0};
    wlcsm_hspot_nvram_default("wl_",0);
    for (i = 0; i < WL_WIFI_RADIO_NUMBER; i++) {
        snprintf(temp_buf, sizeof(temp_buf), "wl%d_",i);
        wlcsm_hspot_nvram_default(temp_buf,0);
        for (j = 1; j <WL_RADIO_WLNUMBSS(i); j++) {
            snprintf(temp_buf, sizeof(temp_buf), "wl%d.%d_",i,j);
            wlcsm_hspot_nvram_default(temp_buf,0);
        }
    }
}
#endif


static void _wlmngr_updateStationList(const unsigned int idx,unsigned int sub_idx,void *buf)
{
    WL_STA_EVENT_DETAIL *psta_detail=(WL_STA_EVENT_DETAIL *)buf;
    WLCSM_WLAN_AP_STA_STRUCT  *associated_sta;
    int found=0;
    char associated=0,authorized=0;

    if((associated_sta=_wlmngr_find_sta(idx,sub_idx,(char *)psta_detail->mac))!=NULL)
        found=1;

    if(psta_detail->event_type==STA_DISCONNECTED) {
        if(found) {
            _wlmngr_remove_sta(idx,sub_idx,associated_sta);
        }
    } else {
        if(!found) {
            /* add new one to the list */
            WLCSM_TRACE(WLCSM_TRACE_DBG, "....\n");
            associated_sta = (WLCSM_WLAN_AP_STA_STRUCT *)malloc(sizeof(WLCSM_WLAN_AP_STA_STRUCT));
            if(associated_sta) {
                associated_sta->macAddress=malloc(MACSTR_LEN);
                if(associated_sta->macAddress) {
                    memcpy(associated_sta->macAddress,psta_detail->mac,MACSTR_LEN);
                    associated_sta->authorized=0;
                    associated_sta->associated=0;
                    WLCSM_TRACE(WLCSM_TRACE_DBG, "sta:%s get added \n",psta_detail->mac);
                    _wlmngr_add_sta(idx,sub_idx,associated_sta);
                } else {
                    WLCSM_TRACE(WLCSM_TRACE_DBG, "....\n");
                    free(associated_sta);
                    return;
                }
            } else {
                return;
            }
        }
        if(!wlcsm_wl_sta_assoc_auth_status((char *)WL_BSSID_IFNAME(idx,sub_idx),
                                           (char *)associated_sta->macAddress,&associated,&authorized)) {
            associated_sta->authorized=authorized;
            associated_sta->associated=associated;
        } else  {
            fprintf(stderr, " problem to get STA associated status !!%s:%d:    \n",__FUNCTION__,__LINE__);
        }
    }
    WLCSM_TRACE(WLCSM_TRACE_DBG, "....\n");
    WLCSM_TRACE(WLCSM_TRACE_DBG, "assoicated_sta number:%d\n", WL_AP_NUMSTATIONS(idx,sub_idx));
    return;
}


static int _wlmngr_handle_wlcsm_wl_sta_event (t_WLCSM_MNGR_VARHDR *hdr,char *varName,char *varValue)
{
    WLCSM_TRACE(WLCSM_TRACE_DBG, "..WLEVENT happen...\n");
    WLCSM_TRACE(WLCSM_TRACE_DBG, " to handle sta:%s\n",((char *)((WL_STA_EVENT_DETAIL *)psta_detail)->mac));
    _wlmngr_updateStationList( WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx),hdr->sub_idx,varValue);
    if(g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_STAS_CHANGED].func)
        g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_STAS_CHANGED].func(WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx),hdr->sub_idx, varValue);
#ifdef IDLE_PWRSAVE
    wlmngr_togglePowerSave();
#endif
    return 0;
}


typedef  int (*MNGR_CMD_HANDLER)(t_WLCSM_MNGR_VARHDR  *hdr,char *name,char *value);

static MNGR_CMD_HANDLER g_mngr_cmd_handlers[]= {
    _wlmngr_handle_wlcsm_cmd_validate_dmvar,
    _wlmngr_handle_wlcsm_cmd_get_dmvar,
    _wlmngr_handle_wlcsm_cmd_set_dmvar,
    _wlmngr_handle_wlcsm_wl_sta_event,
    NULL,
    wlmngr_handle_wlcsm_cmd_restart,
    _wlmngr_handle_wlcsm_cmd_reg_dm_event,
    _wlmngr_handle_wlcsm_cmd_setdmdbglevel_event,
    _wlmngr_handle_wlcsm_cmd_nethotplug_event,
    wlmngr_handle_wlcsm_cmd_halt_restart,
    wlmngr_handle_wlcsm_cmd_resume_restart
};


static  int  _wlmngr_get_var(t_WLCSM_MNGR_VARHDR *hdr,char *varName,char *varValue,int oid)
{
    char *idxStr;
    char *inputVarName=varName;
    char varNameBuf[WL_MID_SIZE_MAX];
    char ifcName[WL_MID_SIZE_MAX];
    char *next;
    int ret=0;
    int i=0;
    int num=0;
    char mac[32];

    unsigned int idx=WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx);
    char *tmp = NULL;
    unsigned int wlcsm_mngr_cmd=WLCSM_MNGR_CMD_GET_CMD(hdr->radio_idx);
    if(wlcsm_mngr_cmd) {
        if(wlcsm_mngr_cmd>=WLCSM_MNGR_CMD_LAST) {
            fprintf(stderr," WLCSM CMD IS TOO BIG, NO SUCH COMMDN \r\n" );
            return 0;
        } else
            return g_mngr_cmd_handlers[wlcsm_mngr_cmd-1](hdr,varName,varValue);
    } else {

        if((varName==NULL || *varName=='\0')||
                ((hdr->sub_idx) >= WL_RADIO_WLMAXMBSS(WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx)))) {
            fprintf(stderr," Invalid name!! or idx is too big :%d\r\n",hdr->radio_idx );
            return 0;
        }

        strncpy(varNameBuf,varName,sizeof(varNameBuf));
        varName = varNameBuf;
        {
            /* regulare varName handling here */
            int sub_idx=hdr->sub_idx;
            idxStr=strstr(varName, WL_PREFIX);
            if(idxStr) {
                sub_idx = atoi(idxStr+strlen(WL_PREFIX)+2); /* wlXx */
                *idxStr = '\0';
            }

            ret=wlmngr_handle_special_var(hdr,varName,varValue,WLMNGR_VAR_OPER_GET);
            if(ret) return ret;
            else {
                /* most of the special vars followings are for old GUI,will remove
                 * when old GUI are completely phased out */
                if ( WLCSM_STRCMP(varName, "wlCurIdxIfcName") == 0 ) {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%s", WL_BSSID_IFNAME(idx,0));
#ifdef WLCSM_DEBUG
                } else if (WLCSM_STRCMP(varName, "restartingstatus") == 0 ) {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d", wlmngr_get_restarting_status());
                } else if(WLCSM_STRCMP(varName, "stalist") == 0 ) {
                    _wlmngr_print_ap_stalist(WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx),sub_idx);
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d", WL_AP_NUMSTATIONS(WLCSM_MNGR_CMD_GET_IDX(hdr->radio_idx),sub_idx));
#endif
                } else if (WLCSM_STRCMP(varName, "wlBands") == 0 ) {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d", wlmngr_getBands(idx));
                } else if (WLCSM_STRCMP(varName, "wlChanImState") == 0) {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d", wlmngr_getChannelImState(idx));
                } else if (WLCSM_STRCMP(varName, "wlCurrentBw") == 0 ) {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d", ((wlmngr_getCurrentChSpec(idx) & WL_CHANSPEC_BW_MASK) >> WL_CHANSPEC_BW_SHIFT));
                } else if (WLCSM_STRCMP(varName, "wlpwrsave") == 0) {
                    wlmngr_getPwrSaveStatus(idx, varValue);
#if defined(__CONFIG_HSPOT__)
                }  else if ( WLCSM_STRCMP(varName, "wlEnableHspot") == 0 ) {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%s", "2");
                }  else if ( WLCSM_STRCMP(varName, "wlPassPoint") == 0 ) {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%s", "1");
#endif
                } else if (WLCSM_STRCMP(varName,"wdsscan_summaries") == 0) {
                    WL_WDSAPLIST_SUMMARIES *wdsap_summaries=(WL_WDSAPLIST_SUMMARIES *)varValue;
                    int ret_size=0;
                    wlmngr_scanWdsResult(idx);
                    if (WL_RADIO(idx).m_tblScanWdsMac != NULL) {
                        WL_FLT_MAC_ENTRY *entry = NULL;
                        int ap_count=0;
                        list_for_each(entry, (WL_RADIO(idx).m_tblScanWdsMac) ) {
                            WLCSM_TRACE(WLCSM_TRACE_LOG," mac:%s,ssid:%s \r\n",entry->macAddress,entry->ssid );
                            memcpy(wdsap_summaries->wdsaplist_summary[ap_count].mac,entry->macAddress,WL_MID_SIZE_MAX);
                            memcpy(wdsap_summaries->wdsaplist_summary[ap_count].ssid,entry->ssid,WL_SSID_SIZE_MAX);
                            WLCSM_TRACE(WLCSM_TRACE_LOG," mac:%s,ssid:%s \r\n",wdsap_summaries->wdsaplist_summary[ap_count].mac,
                                        wdsap_summaries->wdsaplist_summary[ap_count].ssid);
                            ap_count++;
                        }
                        wdsap_summaries->num_of_aps=ap_count;
                        WLCSM_TRACE(WLCSM_TRACE_LOG," there are :%d ap scanned \r\n",wdsap_summaries->num_of_aps );
                        ret_size=(wdsap_summaries->num_of_aps)*sizeof(WL_WDSAP_LIST_ENTRY);
                    } else {
                        wdsap_summaries->num_of_aps=0;
                        WLCSM_TRACE(WLCSM_TRACE_LOG," there is no scan ap find \r\n" );
                    }
                    ret_size+=sizeof(WL_WDSAPLIST_SUMMARIES);
                    varValue[ret_size]='\0';
                    return ret_size+1;
                } else if (WLCSM_STRCMP(varName,"wl_mbss_summaries") == 0) {
                    int num_of_mbss= WL_RADIO_WLNUMBSS(idx);
                    WL_BSSID_SUMMARIES *bss_summaries=(WL_BSSID_SUMMARIES *)varValue;
                    WL_BSSID_SUMMARY  *summary=bss_summaries->bssid_summary;
                    bss_summaries->num_of_bssid=num_of_mbss-1;
#if !defined(HSPOT_SUPPORT)
                    bss_summaries->wlSupportHspot=WLHSPOT_NOT_SUPPORTED;
#else
                    bss_summaries->wlSupportHspot=1;
#endif
                    for (i = 0; i < num_of_mbss-1; i++) {
                        summary[i].wlEnbl=WL_BSSID_WLENBLSSID(idx,i+1);
                        summary[i].wlHide=WL_AP_WLHIDE(idx,i+1);
                        summary[i].wlIsolation=WL_AP_WLAPISOLATION(idx,i+1);
                        summary[i].wlWme=WL_AP_WLWME(idx,i+1);
                        summary[i].wlDisableWme=WL_AP_WLDISABLEWME(idx,i+1);
                        summary[i].wlEnableWmf=WL_BSSID_WLENABLEWMF(idx,i+1);
                        summary[i].wmfSupported=WL_RADIO_WLHASWMF(idx);
#if !defined(HSPOT_SUPPORT)
                        summary[i].wlEnableHspot=WLHSPOT_NOT_SUPPORTED;
#else
                        summary[i].wlEnableHspot=WL_AP_WLENABLEHSPOT(idx,i+1);
#endif
                        summary[i].max_clients=WL_AP_WLMAXASSOC(idx,i+1);

                        if(WL_BSSID_WLBSSID(idx,i+1))
                            strncpy(summary[i].bssid,WL_BSSID_WLBSSID(idx,i+1), sizeof(summary[i].bssid));
                        else
                            summary[i].bssid[0]='\0';

                        if(WL_BSSID_WLSSID(idx,i+1))
                            strncpy(summary[i].wlSsid,WL_BSSID_WLSSID(idx,i+1), sizeof(summary[i].wlSsid));
                        else
                            summary[i].wlSsid[0]='\0';

                    }

                    /* we only have Virual BSS filled up,so minus one */
                    varValue[sizeof(WL_BSSID_SUMMARIES)+(num_of_mbss-1)*sizeof(WL_BSSID_SUMMARY)]='\0';
                    return sizeof(WL_BSSID_SUMMARIES)+(num_of_mbss-1)*sizeof(WL_BSSID_SUMMARY)+1;

                }

                else if (!strncmp(varName, "wlWds",5) && sscanf(varName,"wlWds%d",&num)) {

                    if( WL_RADIO_WLWDS(idx)) {
                        WLCSM_TRACE(WLCSM_TRACE_LOG," WLWDS str:%s \r\n",WL_RADIO_WLWDS(idx));
                        for_each(mac,WL_RADIO_WLWDS(idx),next) {
                            if(i++==num)
                                snprintf(varValue, WL_MID_SIZE_MAX, "%s", mac);
                        }
                    } else {
                        varValue[0]='\0';
                        WLCSM_TRACE(WLCSM_TRACE_LOG," wlWDS is NULL, the varName is:%s \r\n",varName );
                    }
                }

                else if (nvram_match("wps_version2", "enabled") && WLCSM_STRCMP(varName, "wlWscAuthoStaMac") == 0) {
                    tmp = nvram_safe_get("wps_autho_sta_mac");
                    snprintf(varValue, WL_MID_SIZE_MAX, "%s", tmp);

                } else if (WLCSM_STRCMP(varName, "wlSsidList") == 0 ) {
                    wlmngr_printSsidList(idx, varValue);
                } else if(WLCSM_STRCMP(varName, "wlCurrentChannel") == 0 ) {
                    wlmngr_getCurrentChannel(idx);
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d", WL_RADIO_WLCURRENTCHANNEL(idx));
                } else if ( WLCSM_STRCMP(varName, "wlInterface") == 0 ) {
                    bcmGetWlName(0, 0, ifcName);
                    if ( bcmIsValidWlName(ifcName) == TRUE )
                        strcpy(varValue, "1");
                    else
                        strcpy(varValue, "0");
                } else if ( WLCSM_STRCMP(varName, "lan_hwaddr") == 0 ) {
                    wlmngr_getHwAddr(idx, IFC_BRIDGE_NAME, varValue);
                    bcmProcessMarkStrChars(varValue);
                } else if(WLCSM_STRCMP(varName, "wlInfo") == 0 ) {
                    wlmngr_getWlInfo(idx, varValue, "wlcap");
#ifndef SUPPORT_SES
                } else if(WLCSM_STRCMP(varName, "wlSesAvail") == 0 ) {
                    strcpy(varValue, "0");
#endif
                } else {
                    char *temp=NULL;
                    if (WLCSM_STRCMP(varName, "wlSsid_2") == 0 )
                        varName="wlSsid";
                    else if (WLCSM_STRCMP(varName, "wlEnbl_2") == 0 )
                        varName="wlEnblSsid";
                    if(!oid)
                        temp = wlcsm_dm_mngr_get_all_value(hdr->radio_idx,sub_idx,varName,varValue);
                    else
                        temp=wlcsm_dm_mngr_get_value(hdr->radio_idx,sub_idx,varName,varValue,oid);
                    if(!temp)
                        return 0;
                }

            }
        }
    }
    varName = inputVarName;
    ret=strlen(varValue)+1;
    return ret;
}

static int _wlmngr_handle_wlcsm_cmd_get_dmvar(t_WLCSM_MNGR_VARHDR  *hdr,char *varName,char *varValue)
{
    unsigned int mngr_oid=0;
    WLCSM_NAME_OFFSET *name_offset= wlcsm_dm_get_mngr_entry(hdr,varValue,&mngr_oid);

    if(!name_offset) {
        WLCSM_TRACE(WLCSM_TRACE_LOG," reurn not successull \r\n" );
        return 0;
    } else {
        WLCSM_TRACE(WLCSM_TRACE_LOG," mngr_oid:%u \r\n",mngr_oid);
        WLCSM_MNGR_CMD_SET_CMD(hdr->radio_idx,0);
        WLCSM_MNGR_CMD_SET_DMOPER(hdr->radio_idx);
        return _wlmngr_get_var(hdr,name_offset->name,varValue,mngr_oid);
    }
}


int wlmngr_get_var(t_WLCSM_MNGR_VARHDR *hdr,char *varName,char *varValue)
{
    return _wlmngr_get_var(hdr,varName,varValue,0);
}


int wlmngr_set_var(t_WLCSM_MNGR_VARHDR *hdr,char *varName,char *varValue)
{
    unsigned int wlcsm_mngr_cmd=WLCSM_MNGR_CMD_GET_CMD(hdr->radio_idx);
    int ret=0;
    if(wlcsm_mngr_cmd) {
        if(wlcsm_mngr_cmd>=WLCSM_MNGR_CMD_LAST) {
            fprintf(stderr," WLCSM CMD IS TOO BIG, NO SUCH COMMAND \r\n" );
            ret = -1;
        } else
            ret = g_mngr_cmd_handlers[wlcsm_mngr_cmd-1](hdr,varName,varValue);
    } else {
        ret=wlmngr_handle_special_var(hdr,varName,varValue,WLMNGR_VAR_OPER_SET);
        if(ret) ret = 0;
        else
            ret = wlcsm_dm_mngr_set_all_value(hdr->radio_idx,
                                              hdr->sub_idx,varName,varValue);
    }
    return ret;
}
/* End of file */
