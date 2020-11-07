/*
<:copyright-BRCM:2016:proprietary:standard

   Copyright (c) 2016 Broadcom
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/
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
#include <mntent.h>
#include "cms_util.h"
#include "prctl.h"
#include "cms_qos.h"
#include <wlmngr.h>
#include <bcmnvram.h>

#include "wlioctl.h"
#include "wlutils.h"
#include "wlmngr.h"
#include "wlmdm.h"

#include <wlsyscall.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <wlcsm_lib_hspot.h>
#include <wlcsm_lib_api.h>
#include <wlcsm_lib_dm.h>

#include "wllist.h"

#ifdef SUPPORT_WSC
#include <time.h>

#include <sys/types.h>
#include <board.h>
#include "board.h"
#include "bcm_hwdefs.h"
#endif

#include "chipinfo_ioctl.h"

#include <linux/devctl_pwrmngt.h>
//#define WL_WLMNGR_DBG

#include <assert.h>

#ifdef SUPPORT_SES
#ifndef SUPPORT_NVRAM
#error BUILD_SES depends on BUILD_NVRAM
#endif
#endif


#define WEB_BIG_BUF_SIZE_MAX_WLAN (13000 * 2)

#ifdef SUPPORT_MIMO
#define MIMO_ENABLED   ((!WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_N) || !WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_AC)) && WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlNmode, WL_OFF))
#endif
#define MIMO_PHY ((!WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_N)) || (!WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_AC)))

#ifdef SUPPORT_WSC
#define WSC_RANDOM_SSID_KEY_SUPPORT    1

#define WL_DEFAULT_SSID "BrcmAP"
#define WSC_SUCCESS  0
#define WSC_FAILURE   1
#define WSC_RANDOM_SSID_TAIL_LEN	4
#define WSC_MAX_SSID_LEN  32
#define WPS_UI_PORT			40500
extern int wps_config_command;
extern int wps_action;
extern char wps_uuid[36];
extern char wps_unit[32];
extern int wps_method;
extern char wps_autho_sta_mac[sizeof("00:00:00:00:00:00")];
#endif /* SUPPORT_WSC */

extern int wl_cnt;

//**************************************************************************
// Function Name: strcat_r
// Description  : A simple string strcat tweak
// Parameters   : string1, string2, and a buffer
// Returns      : string1 and string2 strcat to the buffer
//**************************************************************************
char *strcat_r( const char *s1, const char *s2, char *buf)
{
    strcpy(buf, s1);
    strcat(buf, s2);
    return buf;
}



//**************************************************************************
// Function Name: setWdsMacList
// Description  : set WDS list for wireless interface.
// Parameters   : buf , size
// Returns      : None.
//**************************************************************************
void wlcsm_dm_tr98_setWdsMacList(int idx, char *buf, int size)
{
    int i = 0;
    char mac[32], *next=NULL;
    for(i=0; i< WL_WDS_NUM; i++)
        memset(m_instance_wl[idx].m_wlVar.wlWds[i], '\0', sizeof(m_instance_wl[idx].m_wlVar.wlWds[i]));
    i=0;
    if(strlen(buf)>0) {
        for_each(mac, buf, next) {
            if (i >= WL_WDS_NUM)
                printf("setWdsMacList: mac list (%s) exceeds limit(%d)\n", buf, WL_WDS_NUM);
            else {
                strncpy(m_instance_wl[idx].m_wlVar.wlWds[i],mac, sizeof(m_instance_wl[idx].m_wlVar.wlWds[i]));
                i++;
            }
        }
    }
}


//**************************************************************************
// Function Name: getWdsMacList
// Description  : get WDS list for wireless interface.
// Parameters   : buf , size
// Returns      : None.
//**************************************************************************
void wlcsm_dm_tr98_getWdsMacList(int idx, char *maclist, int size)
{
    int offset =0;
    int i =0 ;
    memset(maclist, 0, size);
    for ( i = 0; i < WL_WDS_NUM; i++ ) {
        if ( m_instance_wl[idx].m_wlVar.wlWds[i][0] == '\0' )
            continue;
        if (offset + strlen(m_instance_wl[idx].m_wlVar.wlWds[i]) < size-2 ) {
            strcpy( (maclist+offset), m_instance_wl[idx].m_wlVar.wlWds[i]);
            offset += strlen(m_instance_wl[idx].m_wlVar.wlWds[i]);
            /* add a space character between mac addr */
            *(maclist+offset) = ' ';
            offset++;
        }
    }
}

void wlcsm_dm_tr98_getFltMacList(int idx,int bssidx, char *maclist, int size)
{
    WL_FLT_MAC_ENTRY *entry = NULL;
    WL_FLT_MAC_ENTRY *node = NULL;
    int offset =0;
    WIRELESS_MSSID_VAR *m_wlMssidVarPtr;

    /* Set the MAC list */
    memset(maclist, 0, size);

    node = NULL;
    m_wlMssidVarPtr =  &(m_instance_wl[idx].m_wlMssidVar[bssidx]);
    list_get_next(node,  m_wlMssidVarPtr->m_tblFltMac, entry);
    while ( entry != NULL ) {

        if (offset + strlen(entry->macAddress) < size-2 ) {
            strcpy( (maclist+offset), entry->macAddress);
            offset += strlen(entry->macAddress);
            *(maclist+offset) = ' ';
            offset++;
        }
        node = entry;
        list_get_next(node,  m_wlMssidVarPtr->m_tblFltMac, entry);
    }

}

#define SCAN_WDS 0
#define SCAN_WPS 1
#define NUMCHANS 64
#define SCAN_DUMP_BUF_LEN (32 * 1024)
#define MAX_AP_SCAN_LIST_LEN 50
#define SCAN_RETRY_TIMES 5

extern char *bcm_ether_ntoa(const struct ether_addr *ea, char *buf);


//**************************************************************************
// Function Name: getValidBand
// Description  : get valid band matching hardware
// Parameters   : band -- band
// Returns      : valid band
//**************************************************************************
int wlcsm_dm_tr98_getValidBand(int idx, int band)
{
    int vbands = wlcsm_dm_tr98_getBands( idx);
    if ((vbands & BAND_A) && (vbands & BAND_B)) {
        if (!(band == BAND_A || band == BAND_B)) {
            band = BAND_B; // 2.4G
        }
    } else if (vbands & BAND_B) {
        if (!(band == BAND_B)) {
            band = BAND_B; // 2.4G
        }
    } else if (vbands & BAND_A) {
        if (!(band == BAND_A)) {
            band = BAND_A; // 5G
        }
    }
    return band;
}

//**************************************************************************
// Function Name: getValidChannel
// Description  : get valid channel number which is in country range.
// Parameters   : channel -- current channel number.
// Returns      : valid channel number.
//**************************************************************************
int wlcsm_dm_tr98_getValidChannel(int idx, int channel)
{

    int vc, found;
    char cmd[WL_LG_SIZE_MAX];


    snprintf(cmd, sizeof(cmd), "wlctl -i wl%d channels > /var/wl%dchlist", idx, idx );
    BCMWL_WLCTL_CMD(cmd);
    snprintf(cmd, sizeof(cmd), "/var/wl%dchlist", idx );

    FILE *fp = fopen(cmd, "r");
    if ( fp != NULL ) {

        for (found=0;;) {
            if (fscanf(fp, "%d", &vc) != 1) {
                break;
            }
            if (vc == channel) {
                found = 1;
                break;
            }
        }

        if (!found) {
            channel = 0; // Auto
        }
        fclose(fp);
        unlink(cmd);
    }
    return channel;
}

//**************************************************************************
// Function Name: getValidRate
// Description  : get valid rate which is in b range or g range.
// Parameters   : rate -- current rate in the flash.
// Returns      : valid rate.
//**************************************************************************
long wlcsm_dm_tr98_getValidRate(int idx, long rate)
{
    int i = 0, bMax = 4, gMax = 12, aMax = 8;
    long gRates[] = { 1000000,  2000000,  5500000,  6000000,
                      9000000,  11000000, 12000000, 18000000,
                      24000000, 36000000, 48000000, 54000000
                    };
    long bRates[] = { 1000000, 2000000, 5500000, 11000000 };
    long aRates[] = { 6000000, 9000000, 12000000, 18000000, 24000000, 36000000, 48000000, 54000000};

    long ret = 0;

#ifdef SUPPORT_MIMO
    if ( WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_G) == 0 || MIMO_PHY) {
#else
    if ( WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_G) == 0 ) {
#endif
        for ( i = 0; i < gMax; i++ ) {
            if ( rate == gRates[i] )
                break;
        }
        if ( i < gMax )
            ret = rate;
    } else if ( WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_B) == 0 ) {
        for ( i = 0; i < bMax; i++ ) {
            if ( rate == bRates[i] )
                break;
        }
        if ( i < bMax )
            ret = rate;
    } else if ( WLCSM_STRCMP(m_instance_wl[idx].m_wlVar.wlPhyType, WL_PHY_TYPE_A) == 0 ) {
        for ( i = 0; i < aMax; i++ ) {
            if ( rate == aRates[i] )
                break;
        }
        if ( i < aMax )
            ret = rate;
    }

    return ret;
}

//**************************************************************************
// Function Name: getCoreRev
// Description  : get current core revision.
// Parameters   : none.
// Returns      : revsion.
// Note		: wlctl revinfo output may change at each wlan release
//                if only tkip is supported in WebUI, this function
//                need to be reviewed
//**************************************************************************
int wlcsm_dm_tr98_getCoreRev(int idx)
{
    int  found=0;
    unsigned int corerev;
    char cmd[80];
    FILE *fp = NULL;
    char match_str[]="corerev";

    // get core revision

    snprintf(cmd, sizeof(cmd), "wlctl -i wl%d revinfo > /var/wl%d", idx, idx);
    BCMWL_WLCTL_CMD(cmd);
    snprintf(cmd, sizeof(cmd), "/var/wl%d",  idx);


    fp = fopen(cmd, "r");
    if (fp != NULL) {
        while ( !feof(fp) ) {
            fscanf(fp, "%s 0x%x\n", cmd, &corerev);
            if(!WLCSM_STRCMP(match_str,cmd)) {
                found=1;
                break;
            }
        }
        fclose(fp);
    }

    if(!found) {
        corerev=0;
    }

    return corerev;
}

//**************************************************************************
// Function Name: getPhytype
// Description  : get wireless physical type.
// Parameters   : none.
// Returns      : physical type (b, g or a).
// Note		: wlctl wlctl phytype output may change at each wlan release
//**************************************************************************
char *wlcsm_dm_tr98_getPhyType(int idx )
{
    char  *phytype = WL_PHY_TYPE_B;
    char cmd[WL_LG_SIZE_MAX];
    FILE *fp = NULL;

    // get phytype

    snprintf(cmd, sizeof(cmd), "wlctl -i wl%d phytype > /var/wl%d", idx,idx);
    BCMWL_WLCTL_CMD(cmd);
    snprintf(cmd, sizeof(cmd), "/var/wl%d",  idx);

    // parse the phytype
    fp = fopen(cmd, "r");
    if ( fp != NULL ) {
        if (fgets(cmd, WL_LG_SIZE_MAX-1, fp)) {
            switch(atoi(cmd)) {
            case WL_PHYTYPE_A:
                phytype = WL_PHY_TYPE_A;
                break;
            case WL_PHYTYPE_B:
                phytype = WL_PHY_TYPE_B;
                break;
            case WL_PHYTYPE_G:
            case WL_PHYTYPE_LP: /*phytype = WL_PHY_TYPE_LP;*/
                //it does not make difference for webui at this moment
                phytype = WL_PHY_TYPE_G;
                break;
            case WL_PHYTYPE_N:
            case WL_PHYTYPE_HT:
            case WL_PHYTYPE_LCN:
                phytype = WL_PHY_TYPE_N;
                break;
            case WL_PHYTYPE_AC:
                phytype = WL_PHY_TYPE_AC;
                break;
            }
        }
        fclose(fp);
    }

    WLCSM_TRACE(WLCSM_TRACE_DBG,"return phytype:%s\r\n",phytype);
    return phytype;
}

//**************************************************************************
// Function Name: getHwAddr
// Description  : get Mac address
// Parameters   : ifname - interface name
//                hwaddr - MAC address
// Returns      : None
//**************************************************************************
void wlcsm_dm_tr98_getHwAddr(int idx, char *ifname, char *hwaddr)
{
    char buf[WL_MID_SIZE_MAX+60];
    char cmd[WL_LG_SIZE_MAX];

    int i=0, j=0;
    char *p=NULL;

    strcpy(hwaddr, "");

    snprintf(cmd, sizeof(cmd), "ifconfig %s > /var/hwaddr ", ifname);
    bcmSystemMute(cmd);

    FILE *fp = fopen("/var/hwaddr", "r");
    if ( fp != NULL ) {
        if (fgets(buf, sizeof(buf), fp)) {
            for(i=2; i < (int)(sizeof(buf)-5); i++) {
                if (buf[i]==':' && buf[i+3]==':') {
                    p = buf+(i-2);
                    for(j=0; j < WL_MID_SIZE_MAX-1; j++, p++) {
                        if (isalnum(*p) || *p ==':') {
                            hwaddr[j] = *p;
                        } else {
                            break;
                        }
                    }
                    hwaddr[j] = '\0';
                    break;
                }
            }
        }
        fclose(fp);
    }
}



//**************************************************************************
// Function Name: getBands
// Description  : get Avaiable Bands (2.4G/5G)
// Parameters   : None
// Returns      : Bands
//**************************************************************************
int wlcsm_dm_tr98_getBands(int idx)
{
    char buf[WL_LG_SIZE_MAX];
    int bands = 0;
    char cmd[80];

    snprintf(cmd, sizeof(cmd), "wlctl -i wl%d bands > /var/wl%dbands", idx, idx );
    BCMWL_WLCTL_CMD(cmd);
    snprintf(cmd, sizeof(cmd), "/var/wl%dbands", idx );

    FILE *fp = fopen(cmd, "r");
    if ( fp != NULL ) {
        if (fgets(buf, sizeof(buf), fp)) {
            if (strchr(buf, 'a')) {
                bands |= BAND_A;
            }
            if (strchr(buf, 'b')) {
                bands |= BAND_B;
            }
        }
        fclose(fp);
    }
    m_instance_wl[idx].m_bands = bands;
    return bands;
}

//**************************************************************************
// Function Name: getVer
// Description  : get wireless driver version
// Parameters   : ver - version string
//                size - max string size
// Returns      : None
//**************************************************************************

void wlcsm_dm_tr98_getVer(int idx)
{
    char *tag = "version ";
    char buf[WL_LG_SIZE_MAX];
    char *p;

    FILE *fp = NULL;

    snprintf(buf, sizeof(buf), "wlctl -i %s  ver > /var/wlver ", m_instance_wl[idx].m_ifcName[0]);
    BCMWL_WLCTL_CMD(buf);
    fp = fopen("/var/wlver", "r");
    if ( fp != NULL ) {
        fgets(buf, sizeof(buf), fp);
        fgets(buf, sizeof(buf), fp);
        p = strstr(buf, tag);
        if (p != NULL) {
            p += strlen(tag);
            sscanf(p, "%32s", m_instance_wl[idx].wlVer);
        }
        fclose(fp);
    } else {
        printf("/var/wlver open error\n");
    }
}



/***************************************************************************
// Function Name: getMaxMbss.
// Description  : getMaxMbss
// Parameters   : none.
// Returns      : n/a.
****************************************************************************/
int wlcsm_dm_tr98_getMaxMbss(int idx)
{

    if (m_instance_wl[idx].maxMbss==0)
        wlcsm_dm_tr98_initVars(idx);

    return m_instance_wl[idx].maxMbss;
}


void wlcsm_dm_tr98_write_nvram(void)
{
    char *str = malloc(MAX_NVRAM_SPACE*sizeof(char));
    char *buf = malloc(MAX_NVRAM_SPACE*sizeof(char));
    char *name;
    char pair[WL_LG_SIZE_MAX+WL_SIZE_132_MAX];
    char format[]="%0?x%s%d";
    *str = '\0';

    sprintf(format, "%s%d%s", "%0", DEFAULT_PAIR_LEN_BYTE, "x%s%d");// format set to "%02x%s%d"
    snprintf(pair, sizeof(pair), format, strlen("pair_len_byte=?"), "pair_len_byte=", PAIR_LEN_BYTE);
    strcat(str, pair);
    sprintf(format,"%s%d%s","%0", PAIR_LEN_BYTE, "X%s");

    nvram_getall(buf, MAX_NVRAM_SPACE*sizeof(char));
    for (name = buf; *name && (strlen(str)<MAX_NVRAM_SPACE); name += strlen(name) + 1) {
        if(wlcsm_nvram_from_kernel(name,0)) {
            continue;
        }
        if(!strncmp(name,"wps_force_restart", strlen("wps_force_restart"))
                || !strncmp(name, "pair_len_byte=", strlen("pair_len_byte="))
                || !strncmp(name, "acs_ifnames", strlen("acs_ifnames"))
                || !strncmp(name, "wl_unit", strlen("wl_unit"))
                || strstr(name, "_dpd") || strstr(name, "pcie_"))
            continue;
#if defined(HSPOT_SUPPORT)
        if(! wlcsm_hspot_var_isdefault(name)) {
#endif
            snprintf(pair, sizeof(pair), format, strlen(name), name);
            if(strlen(str)+strlen(pair)<MAX_NVRAM_SPACE)
                strcat(str, pair);
            else
                fprintf(stderr, "%s:%d:ERROR:nvram %s can not be saved \
                        due to buffer full\n",__FUNCTION__,__LINE__,name);

#if defined(HSPOT_SUPPORT)
        }
#endif
    }
    WLCSM_TRACE(WLCSM_TRACE_DBG," size:0x%08x,and left:%d\r\n",strlen(str),MAX_NVRAM_SPACE-strlen(str) );
    wlWriteNvram(str);
    free(buf);
    free(str);
    wlcsm_nvram_from_kernel(NULL,1);
    return;
}

void wlcsm_dm_tr98_nvram_set(const char *name, const char *value)
{
    wlcsm_nvram_set((char *)name,(char *)value);
    wlcsm_nvram_update_runtime_mngr((char *)name,(char *)value);
}

/***************************************************************************
// Function Name: BcmWl_removeAllFilterMac.
// Description  : remove all the MAC addresses filter out of the table.
// Parameters   : the filter object.
// Returns      : operation status.
****************************************************************************/
WL_STATUS BcmWl_removeAllFilterMac(int wl_idx, int idx)
{
    WIRELESS_MSSID_VAR *m_wlMssidVarPtr;
    m_wlMssidVarPtr =  &(m_instance_wl[wl_idx].m_wlMssidVar[idx]);
    if(m_wlMssidVarPtr!=NULL && m_wlMssidVarPtr->m_tblFltMac!=NULL) {
        fprintf(stderr,"removeAllFilterMac works?\n");
        list_del_all(m_wlMssidVarPtr->m_tblFltMac,struct wl_flt_mac_entry);
    }
    return WL_STS_OK;
}


/***************************************************************************
// Function Name: BcmWl_addFilterMac2.
// Description  : add the MAC address filter to the table.
// Parameters   : mac, ssid, ifcName, wlan idx, ssid.
// Returns      : operation status.
****************************************************************************/
WL_STATUS BcmWl_addFilterMac2(char *mac, char *ssid, char *ifcName,int wl_idx, int idx)
{
    struct wl_flt_mac_entry *entry;

    WIRELESS_MSSID_VAR *m_wlMssidVarPtr;
    m_wlMssidVarPtr =  &(m_instance_wl[wl_idx].m_wlMssidVar[idx]);

    entry = malloc( sizeof(WL_FLT_MAC_ENTRY));
    if ( entry != NULL ) {
        memset(entry, 0, sizeof(WL_FLT_MAC_ENTRY));
        strncpy(entry->macAddress, mac, sizeof(entry->macAddress));
        if(ssid && *ssid)
            strncpy(entry->ssid, ssid, sizeof(entry->ssid));

        if(ifcName && *ifcName) {
            strncpy(entry->ifcName, ifcName, sizeof(entry->ifcName));
        }

        list_add( entry, m_wlMssidVarPtr->m_tblFltMac, struct wl_flt_mac_entry);

        return WL_STS_OK;
    }

    return WL_STS_ERR_GENERAL;
}
