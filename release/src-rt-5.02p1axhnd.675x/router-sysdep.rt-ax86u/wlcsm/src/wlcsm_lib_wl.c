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
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include "wlcsm_linux.h"
#include "wlcsm_lib_api.h"
#include "wlcsm_lib_netlink.h"
#include "wlcsm_lib_nvram.h"
#include "wlcsm_lib_wl.h"
#include "wlioctl_defs.h"
#include <sys/stat.h>

static void syserr(const char *s)
{
    perror(s);
    exit(errno);
}

//**************************************************************************
// Function Name: scanForAddr
// Description  : scan input string for mac address
// Parameters   : line - input string
//                isize - size of input string
//                start - return pointer to the address
//                size - size of mac address
// Returns      : 1 success, 0 failed
//**************************************************************************

char wlcsm_scanForAddr(char *line, int isize, char **start, int *size)
{
    int is = -1;
    int i;
    for(i=2; i<isize-5; i++) {
        *start = NULL;
        if (line[i]==':' && line[i+3]==':') {
            is = i-2;
            break;
        }
    }

    for (i+=3; i<isize-2; i+=3) {
        if (line[i] != ':') {
            break;
        }
    }

    if (is != -1) {
        *start = line+is;
        *size = i-is + 1;
        return 1;
    } else {
        *size = 0;
        return 0;
    }
}

//**************************************************************************
// Function Name: scanFileForMAC
// Description  : Look for an MAC address in a file
// Parameters   : fname - name of the file
//                mac - string for the MAC address
// Returns      : 1 - found, 0 - not found
//**************************************************************************
int wlcsm_scanFileForMAC(char *fname, char *mac)
{
    char buf[WL_MID_SIZE_MAX+60];
    FILE *fp = fopen(fname, "r");
    if ( fp != NULL ) {
        for (; fgets(buf, sizeof(buf), fp);) {
            if (strstr(buf, mac)) {
                fclose(fp);
                return 1;
            }
        }
        fclose(fp);
    }
    return 0;
}

int wlcsm_wl_sta_assoc_auth_status(char *ifname,char *mac, char *associated, char *authed) {
    static char wl_stafile[80];
    static char cmd[WL_SIZE_132_MAX];
    snprintf(wl_stafile, sizeof(wl_stafile), "/var/%s_assoc", ifname);
    snprintf(cmd, sizeof(cmd), "wl -i %s assoclist >%s",ifname,wl_stafile);
    system(cmd);
    *associated= wlcsm_scanFileForMAC(wl_stafile, mac);
    unlink(wl_stafile);
    snprintf(wl_stafile, sizeof(wl_stafile), "/var/%s_autho", ifname);
    snprintf(cmd, sizeof(cmd), "wl -i %s autho_sta_list > %s",ifname, wl_stafile);
    system(cmd);
    *authed= wlcsm_scanFileForMAC(wl_stafile, mac);
    unlink(wl_stafile);
    return 0;
}

void *wlcsm_wl_get_sta_summary(unsigned int idx,unsigned int sub_idx)
{
    char ifname[WL_SIZE_8_MAX],ssid_nvram[WL_SM_SIZE_MAX],*ssidName=NULL;
    int numStations, stalist_size;
    WL_STALIST_SUMMARIES *sta_summaries=NULL;
    if(sub_idx)
        snprintf(ifname,WL_SIZE_8_MAX,"%s%d.%d",WLAN_PREFIX	,idx,sub_idx);
    else
        snprintf(ifname,WL_SIZE_8_MAX,"%s%d",WLAN_PREFIX	,idx);

    snprintf(ssid_nvram,WL_SM_SIZE_MAX,"%s_ssid",ifname);
    ssidName=wlcsm_nvram_get(ssid_nvram);
    if(!ssidName) {
        fprintf(stderr, "%s:%d:	 Error, no ssid ??\n",__FUNCTION__,__LINE__);
        return NULL;
    }
    numStations = wlcsm_wl_get_sta_count(ifname);
    stalist_size = numStations*sizeof(WL_STATION_LIST_ENTRY) + sizeof(WL_STALIST_SUMMARIES)+1;
    sta_summaries=(WL_STALIST_SUMMARIES *)malloc(stalist_size);

    if(sta_summaries) {
        WL_STATION_LIST_ENTRY *sta=sta_summaries->stalist_summary;
        char *buf;
        struct stat statbuf;
        int i=0,j;
        static char cmd[WL_SIZE_132_MAX];
        static char wl_authefile[80];
        static char wl_assocfile[80];
        static char wl_authofile[80];
        FILE *fp = NULL;
        int buflen= 0;
        snprintf(wl_authefile, sizeof(wl_authefile), "/var/%s_authe", ifname);
        snprintf(wl_assocfile, sizeof(wl_assocfile), "/var/%s_assoc", ifname);
        snprintf(wl_authofile, sizeof(wl_authofile), "/var/%s_autho", ifname);
        snprintf(cmd, sizeof(cmd), "wl -i %s authe_sta_list > %s",ifname,wl_authefile);
        system(cmd);
        snprintf(cmd, sizeof(cmd), "wl -i %s assoclist > %s",ifname,wl_assocfile);
        system(cmd);
        snprintf(cmd, sizeof(cmd), "wl -i %s autho_sta_list > %s",ifname, wl_authofile);
        system(cmd);

        if(!stat(wl_authefile, &statbuf)) {
            fp = fopen(wl_authefile, "r");
            if(fp) {
                buflen= statbuf.st_size;
                buf = (char*)malloc(buflen);
                if (buf ) {
                    int asize;
                    char *pa;
                    for (j=0; j<numStations; j++) {
                        if (fgets(buf, buflen, fp)) {

                            if (wlcsm_scanForAddr(buf, buflen, &pa, &asize)) {
                                asize= (asize > WL_MID_SIZE_MAX -1)?  ( WL_MID_SIZE_MAX -1):asize;
                                memcpy(sta->macAddress,pa,asize);
                                sta->macAddress[asize]='\0';
                                pa= &(sta->macAddress[asize-1]);
                                if (*pa == '\n' || *pa == '\r') *pa='\0';
                                sta->associated=wlcsm_scanFileForMAC(wl_assocfile, sta->macAddress);
                                sta->authorized=wlcsm_scanFileForMAC(wl_authofile, sta->macAddress);
                                sta->radioIndex=idx;
                                sta->ssidIndex =sub_idx;
                                snprintf(sta->ifcName,WL_SM_SIZE_MAX,"%s",ifname);
                                snprintf(sta->ssid,WL_SSID_SIZE_MAX,"%s",ssidName);
                                sta++;
                                i++;
                            }
                        }
                    }
                    free(buf);
                }
                fclose(fp);
            }
        }
        unlink(wl_authefile);
        unlink(wl_assocfile);
        unlink(wl_authofile);
        sta_summaries->num_of_stas=i;
        sta_summaries->radioIdx=idx;
        sta_summaries->ssidIdx=sub_idx;
        if(i!=numStations) {
            fprintf(stderr, "%s:%d:	!!!! Oh?? not the same i:%d and numStations:%d\n",__FUNCTION__,__LINE__,i,numStations);
        }
        sta_summaries->type=UPDATE_STA_ALL;
        return (void *)sta_summaries;
    } else
        return NULL;
}

char g_wl_buf[WLC_IOCTL_MEDLEN];
int wlcsm_wl_get_sta_count(void *wl) {
    char assocfile[64];
    FILE *fp;
    snprintf(assocfile,sizeof(assocfile),"/var/%s_assoclist",(char *)wl);
    snprintf(g_wl_buf,sizeof(g_wl_buf),"wl -i %s assoclist >%s",(char *)wl,assocfile);
    system(g_wl_buf);
    fp=fopen(assocfile,"r");
    int count=0;
    if(fp) {
        while(fgets(g_wl_buf,sizeof(g_wl_buf),fp)) count++;
        fclose(fp);
        unlink(assocfile);
    }
    return count;
}



#if 0
static int wl_ioctl(void *wl, int cmd, void *buf, int len, char set)
{
    struct ifreq *ifr = (struct ifreq *) wl;
    wl_ioctl_t ioc;
    int ret = WLCSM_SUCCESS;
    int s;

    /* open socket to kernel */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        syserr("socket");

    /* do it */
    ioc.cmd = cmd;
    ioc.buf = buf;
    ioc.len = len;
    ioc.set = set;
    ifr->ifr_data = (caddr_t) &ioc;
    if ((ret = ioctl(s, SIOCDEVPRIVATE, ifr)) < 0) {
        if (cmd != WLC_GET_MAGIC) {
            ret = WLCSM_GEN_ERR;
        }
    }

    /* cleanup */
    close(s);
    return ret;
}
#endif


static int
_wl_get_dev_type(char *name, void *buf, int len)
{
    int s;
    int ret;
    struct ifreq ifr;
    struct ethtool_drvinfo info;

    /* open socket to kernel */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        syserr("socket");

    /* get device type */
    memset(&info, 0, sizeof(info));
    info.cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (caddr_t)&info;
    strncpy(ifr.ifr_name, name, IFNAMESIZE-1);
    if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

        /* print a good diagnostic if not superuser */
        if (errno == EPERM)
            syserr("_wl_get_dev_type");

        *(char *)buf = '\0';
    } else {
        strncpy(buf, info.driver, len);
    }

    close(s);
    return ret;
}

static inline int _wl_get_intf_num(char b_adapter)
{
    char proc_net_dev[] = "/proc/net/dev";
    FILE *fp;
    char buf[1000], *c, *name;
    char dev_type[DEV_TYPE_LEN];
    int wlif_num=0;

    if (!(fp = fopen(proc_net_dev, "r")))
        return WLCSM_GEN_ERR;

    /* eat first two lines */
    if (!fgets(buf, sizeof(buf), fp) ||
            !fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return WLCSM_GEN_ERR;
    }

    while (fgets(buf, sizeof(buf), fp)) {
        c = buf;
        while (isspace(*c))
            c++;
        if (!(name = strsep(&c, ":")))
            continue;
        WLCSM_TRACE(WLCSM_TRACE_LOG,"JXUJXU:%s:%d  name:%s \r\n",__FUNCTION__,__LINE__,name);
        if ( (_wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 && !strncmp(dev_type, "wl", 2))
#if defined(BUILD_BCM_WLAN_DPDCTL)
             || (!strncmp(name, "wl", 2))
#endif /* BUILD_BCM_WLAN_DPDCTL */
        ) {
            WLCSM_TRACE(WLCSM_TRACE_LOG,"JXUJXU:%s:%d  it is wireless interface and devtype:%s \r\n",__FUNCTION__,__LINE__,dev_type );
            if(!b_adapter ||( b_adapter && !strstr(name,".")))
                wlif_num++;
            if (wlif_num >= MAX_WLIF_NUM)
                break;
        }
    }
    fclose(fp);
    return wlif_num;
}

int wlcsm_wl_get_adapter_num(void)
{
    return _wl_get_intf_num(1);
}


/* -countryrev <input> three char like USI
 * -ccode <output>  First two Char.
 * -regrev <output> third char
 */
int wlcsm_wl_parse_countryrev(char *countryrev,char *ccode,char *regrev) {
    if(!countryrev || strlen(countryrev)>3) return -1;
    *regrev=countryrev[2];
    ccode[0]=countryrev[0];
    ccode[1]=countryrev[1];
    return 0;
}

int wlcsm_wl_get_mbss_num(int idx)
{

    char buf[128],cmd[32];
    FILE *fp;
    /* default to system maximum */
    int num=WL_DEFAULT_NUM_SSID ;

    sprintf(cmd, "wlctl -i wl%d cap > /var/wl%dcap", idx, idx);
    system(cmd);

    sprintf(cmd, "/var/wl%dcap", idx);

    fp = fopen(cmd, "r");

    if ( fp != NULL ) {
        for (; fgets(buf, sizeof(buf), fp);) {
            if(strstr(buf, "1ssid")) {
                num= 1;
                break;
            }
            if(strstr(buf, "mbss4")) {
                num= 4;
                break;
            }
            if(strstr(buf, "mbss8")) {
                num= 8;
                break;

            } else if(strstr(buf, "mbss16")) {
                num= 16;
                break;
            }
        }
        fclose(fp);
    }

    /* limit the max MBSS to the maximum */
    return num>WL_MAX_NUM_SSID? WL_MAX_NUM_SSID:num;
}

int wl_get_br_names(char *brnames)
{

#if 0
    struct ifreq *ifr = (struct ifreq *) wl;
    wl_ioctl_t ioc;
    int s;
    unsigned long args[4];
    ifr.ifr_data=(char *)&args;
    jkk

    /* open socket to kernel */
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        syserr("socket");
#endif
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO....\n");
    strcpy(brnames,"br0");
    return 0;
}

int wl_get_br_ifnames(char *br,char *ifnames)
{

#if 0
    struct ifreq *ifr = (struct ifreq *) wl;
    wl_ioctl_t ioc;
    int s;
    unsigned long args[4];
    ifr.ifr_data=(char *)&args;
    jkk

    /* open socket to kernel */
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        syserr("socket");
#endif
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO....\n");
    strcpy(ifnames,"eth0 eth1 eth2 eth3 wl0 usb0");
    return 0;
}
