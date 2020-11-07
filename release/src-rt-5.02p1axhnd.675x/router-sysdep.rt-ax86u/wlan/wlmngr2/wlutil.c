/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sched.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "prctl.h"

#include <bcmnvram.h>

//#include "wlapi.h"
#include "wlioctl.h"
#include "wlutils.h"
#include "wlmngr.h"

#ifdef DSLCPE_WLCSM_EXT
#include <wlcsm_lib_api.h>
#include <wlcsm_lib_dm.h>
#include <wlcsm_linux.h>
#endif
#include "wlsyscall.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include "wllist.h"

#ifdef SUPPORT_WSC
#include <time.h>

#include <sys/types.h>
#include <board.h>
#include "board.h"
#include "bcm_hwdefs.h"
#endif

#include <bcmendian.h>
#include <wlcsm_lib_api.h>
#include <wlcsm_lib_dm.h>
#include <wlcsm_linux.h>
#include "chipinfo_ioctl.h"

#include <linux/devctl_pwrmngt.h>
//#define WL_WLMNGR_DBG

extern int act_wl_cnt;

#ifdef SUPPORT_SES
#ifndef SUPPORT_NVRAM
#error BUILD_SES depends on BUILD_NVRAM
#endif
#endif


#define WEB_BIG_BUF_SIZE_MAX_WLAN (13000 * 2)

#ifdef SUPPORT_MIMO
#define MIMO_ENABLED   ((!WLCSM_STRCMP(WL_PHYTYPE(idx), WL_PHY_TYPE_N) || !WLCSM_STRCMP(WL_PHYTYPE(idx), WL_PHY_TYPE_AC)) && WLCSM_STRCMP(WL_RADIO_NMODE(idx), WL_OFF))
#endif
#define MIMO_PHY ((!WLCSM_STRCMP(WL_PHYTYPE(idx), WL_PHY_TYPE_N)) || (!WLCSM_STRCMP(WL_PHYTYPE(idx), WL_PHY_TYPE_AC)))

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

#define MBSSID_ENAB(mbssid, mbssid_band) ((mbssid == -1  && mbssid_band == 4) || mbssid == 1)

struct xml_esc_entry {
    char esc;  /**< character that needs to be escaped */
    char *seq; /**< escape sequence */
    UINT32 len;  /**< length of escape sequence */
};

struct xml_esc_entry xml_esc_table[] = {
    {'<', "&lt;", 4},
    {'>', "&gt;", 4},
    {'&', "&amp;", 5},
    {'%', "&#37;", 5},
    {' ', "&#32;", 5},
    {'\t', "&#09;", 5},
    {'\n', "&#10;", 5},
    {'\r', "&#13;", 5},
    {'"', "&quot;", 6},
};

#define NUM_XML_ESC_ENTRIES (sizeof(xml_esc_table)/sizeof(struct xml_esc_entry))



int wlmngr_isEscapeNeeded(const char *string)
{
    UINT32 len, i=0, e=0;
    int escapeNeeded = FALSE;

    if (string == NULL) {
        return FALSE;
    }

    len = strlen(string);

    /* look for characters which need to be escaped. */
    while (escapeNeeded == FALSE && i < len) {
        for (e=0; e < NUM_XML_ESC_ENTRIES; e++) {
            if (string[i] == xml_esc_table[e].esc) {
                escapeNeeded = TRUE;
                break;
            }
        }
        i++;
    }

    return escapeNeeded;
}


int wlmngr_escapeString(const char *string, char **escapedString)
{
    UINT32 len, len2, i=0, j=0, e=0, f=0;
    char *tmpStr;

    if (string == NULL) {
        return 0;
    }

    len = strlen(string);
    len2 = len;

    /* see how many characters need to be escaped and what the new length is */
    while (i < len) {
        for (e=0; e < NUM_XML_ESC_ENTRIES; e++) {
            if (string[i] == xml_esc_table[e].esc) {
                len2 += (xml_esc_table[e].len - 1);
                break;
            }
        }
        i++;
    }

    if ((tmpStr = malloc(len2+1)) == NULL) {
        fprintf(stderr,"failed to allocate %d bytes", len+1);
        return CMSRET_RESOURCE_EXCEEDED;
    }

    i=0;
    while (i < len) {
        int found;

        found = FALSE;
        /* see if we need to replace any characters with esc sequences */
        for (e=0; e < NUM_XML_ESC_ENTRIES; e++) {
            if (string[i] == xml_esc_table[e].esc) {
                for (f=0; f<xml_esc_table[e].len; f++) {
                    tmpStr[j++] = xml_esc_table[e].seq[f];
                    found = TRUE;
                }
                break;
            }
        }

        /* no replacement, then just copy over the original string */
        if (!found) {
            tmpStr[j++] = string[i];
        }

        i++;
    }

    *escapedString = tmpStr;

    return 0;
}


int wlmngr_isUnescapeNeeded(const char *escapedString)
{
    UINT32 len, i=0, e=0, f=0;
    int unescapeNeeded = FALSE;
    int matched=FALSE;

    if (escapedString == NULL) {
        return FALSE;
    }

    len = strlen(escapedString);

    while (unescapeNeeded == FALSE && i < len) {
        /* all esc sequences begin with &, so look for that first */
        if (escapedString[i] == '&') {
            for (e=0; e < NUM_XML_ESC_ENTRIES && !matched; e++) {
                if (i+xml_esc_table[e].len-1 < len) {
                    /* check for match against an individual sequence */
                    matched = TRUE;
                    for (f=1; f < xml_esc_table[e].len; f++) {
                        if (escapedString[i+f] != xml_esc_table[e].seq[f]) {
                            matched = FALSE;
                            break;
                        }
                    }
                }
            }
        }

        i++;

        /* if we saw a match, then unescape is needed */
        unescapeNeeded = matched;
    }

    return unescapeNeeded;
}

void wlmngr_escapeStringEx(const char * orignalString, char * outputEscapedString, SINT32 outputEscapedStringLen)
{
    /* Always copy the orignalString to the outputEscapedString since there might
    * not any escape characters in the orignalString
    */
    strncpy(outputEscapedString, orignalString, outputEscapedStringLen);

    if (wlmngr_isEscapeNeeded(orignalString)) {
        char *escapedString=NULL;

        wlmngr_escapeString(orignalString, &escapedString);
        strncpy(outputEscapedString, escapedString, outputEscapedStringLen);
        free(escapedString);
    }
}
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
// Function Name: wlIfcUp
// Description  : all wireless ifc up
// Parameters   : None.
// Returns      : None.
//**************************************************************************
void wlmngr_wlIfcUp(const unsigned int idx)
{
    char cmd[WL_SIZE_132_MAX];
    int i, j,up_if_cnt=0;
    char *next;
    char mac[32];

    for ( i=1; i<WL_RADIO(idx).wlNumBss; i++) {
        if (WL_BSSID_WLENBLSSID(idx,i)) {
            snprintf(cmd, sizeof(cmd), "ifconfig %s up 2>/dev/null", WL_BSSID_IFNAME(idx,i));
            bcmSystem(cmd);
            up_if_cnt++;
        } else {
            snprintf(cmd, sizeof(cmd), "ifconfig %s down 2>/dev/null", WL_BSSID_IFNAME(idx,i));
            bcmSystem(cmd);
        }
    }

    if(up_if_cnt || WL_BSSID_WLENBLSSID(idx,0)) {
        snprintf(cmd, sizeof(cmd), "ifconfig %s up 2>/dev/null", WL_BSSID_IFNAME(idx,0));
        bcmSystem(cmd);
    } else {
        snprintf(cmd, sizeof(cmd), "ifconfig %s down 2>/dev/null", WL_BSSID_IFNAME(idx,0));
        bcmSystem(cmd);
    }

    if (WL_RADIO_WLWDS(idx) != NULL) {
        j=1;
        for_each(mac,WL_RADIO_WLWDS(idx),next) {
            snprintf(cmd, sizeof(cmd), "ifconfig wds%d.%d up 2>/dev/null", idx, j);
            bcmSystem(cmd);
            j++;
        }
    }
}

//**************************************************************************
// Function Name: wlIfcDown
// Description  : all wireless ifc down
// Parameters   : None.
// Returns      : None.
//**************************************************************************
void wlmngr_wlIfcDown(const unsigned int idx)
{
    char cmd[WL_SIZE_132_MAX];
    int i, j;
    char *next;
    char mac[32];

    /* reset dongle */
    snprintf(cmd, sizeof(cmd), "wl -i %s reboot 2>/dev/null", WL_BSSID_IFNAME(idx,0));
    bcmSystem(cmd);

    if (WL_RADIO_WLWDS(idx) != NULL) {
        j=1;
        for_each(mac,WL_RADIO_WLWDS(idx),next) {
            snprintf(cmd, sizeof(cmd), "ifconfig wds%d.%d down 2>/dev/null", idx, j);
            bcmSystem(cmd);
            j++;
        }
    }

    fprintf(stderr,"bringdown interface:%d\n",idx);
    for (i=0; i<WL_RADIO(idx).wlNumBss; i++) {
        snprintf(cmd, sizeof(cmd), "ifconfig %s down 2>/dev/null", WL_BSSID_IFNAME(idx,i));
        bcmSystem(cmd);
    }
}




//**************************************************************************
// Function Name: wlmngr_getPwrSaveStatus
// Description  : verify wether wireless is in low power mode.
// Parameters   : idx, *varValue.
// Returns      : *varValue.
//**************************************************************************
void wlmngr_getPwrSaveStatus(const unsigned int idx, char *varValue)
{
    UINT32 rxchain = 0;
    UINT32 radiopwr = 0;
    char buf[1024];
    FILE *fp = NULL;
    UINT32 curLine = 0;

    rxchain =  WL_RADIO_WLRXCHAINPWRSAVEENABLE(idx);
    fp = fopen("/var/wlpwrsave1", "a+b");
    if ( fp != NULL ) {
        if (rxchain == 1) {
            snprintf(buf, sizeof(buf), "wlctl -i %s rxchain_pwrsave >/var/wlpwrsave1",  WL_BSSID_IFNAME(idx,0));
            BCMWL_WLCTL_CMD(buf);
            curLine = ftell(fp);
            fgets(buf, sizeof(buf), fp);
            rxchain = atoi(buf);
        }

        radiopwr = WL_RADIO_WLRADIOPWRSAVEENABLE(idx);
        if (radiopwr == 1 ) {
            /* erase the line */
            fseek(fp, curLine, SEEK_SET);
            snprintf(buf, sizeof(buf), "wlctl -i %s radio_pwrsave >/var/wlpwrsave1",  WL_BSSID_IFNAME(idx,0));
            BCMWL_WLCTL_CMD(buf);
            curLine = ftell(fp);
            fgets(buf, sizeof(buf), fp);
            radiopwr = atoi(buf);
        }
        fclose(fp);
    } else {
#ifdef WL_WLMNGR_DBG
        printf("/var/wlpwrsave1 open error\n");
#endif
    }

    if ( (rxchain == 1) || (radiopwr == 1) )
        sprintf(varValue, "1" );
    else
        sprintf(varValue, "0" );
    return;
}

//**************************************************************************
// Function Name: doSecurity
// Description  : setup security for wireless interface.
// Parameters   : none.
// Returns      : None.
//**************************************************************************
void wlmngr_doSecurity(const unsigned int idx)
{
    extern int wlconf_security(char *name);
    char name[64];
#ifndef  DSLCPE_WLCONF_CMD
    snprintf(name, sizeof(name), "%s", WL_BSSID_IFNAME(idx,0));
    wlconf_security(name);
#else
    snprintf(name, sizeof(name), "wlconf %s security", WL_BSSID_IFNAME(idx,0));
    bcmSystem(name);
#endif
}




//**************************************************************************
// Function Name: doWdsSec
// Description  : WDS security (WEP) setup for wireless interface.
// Parameters   : none.
// Returns      : None.
//**************************************************************************
#ifdef DSLCPE_WLCSM_EXT
void wlmngr_doWdsSec(const unsigned int idx)
{
    char cmd[WL_SIZE_132_MAX];
    int i=0;
    if ( WL_RADIO_WLENBL(idx) != TRUE )
        return;

    // reset wds_wsec and wdswsec_enable
    snprintf(cmd, sizeof(cmd), "wlctl -i %s wdswsec 0", WL_BSSID_IFNAME(idx,0));
    BCMWL_WLCTL_CMD(cmd);
    snprintf(cmd, sizeof(cmd), "wlctl -i %s wdswsec_enable 0", WL_BSSID_IFNAME(idx,0));
    BCMWL_WLCTL_CMD(cmd);

    if(WL_RADIO(idx).wlWdsSecEnable) {

        for ( i = 0; i < WL_WDS_NUM; i++ ) {
            WLCSM_TRACE(WLCSM_TRACE_DBG," TODO: WDS related handling here \r\n" );
        }
    } else {
#ifdef DSLCPE_WLCSM_EXT
        WLCSM_TRACE(WLCSM_TRACE_DBG," TODO: WDS related handling here \r\n" );
#endif
        for ( i = 0; i < WL_WDS_NUM; i++ ) {
        }
    }
}

#endif

#define SCAN_WDS 0
#define SCAN_WPS 1
#define NUMCHANS 64
#define SCAN_DUMP_BUF_LEN (32 * 1024)
#define MAX_AP_SCAN_LIST_LEN 50
#define SCAN_RETRY_TIMES 5

static char scan_result[SCAN_DUMP_BUF_LEN];
static char * wlmngr_getScanResults(char *ifname)
{
    int ret, retry_times = 0;
    wl_scan_params_t *params;
    wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
    int params_size = WL_SCAN_PARAMS_FIXED_SIZE + NUMCHANS * sizeof(uint16);
    int org_scan_time = 20, scan_time = 40;
#ifdef DSLCPE_ENDIAN
    int org_scan_time_tmp = 20;
#endif

#ifdef DSLCPE_ENDIAN
    wl_endian_probe(ifname);
#endif

    params = (wl_scan_params_t*)malloc(params_size);
    if (params == NULL) {
        return NULL;
    }

    memset(params, 0, params_size);
    params->bss_type = DOT11_BSSTYPE_ANY;
    memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
    params->scan_type = -1;

#ifdef DSLCPE_ENDIAN
    params->nprobes = htoe32(-1);
    params->active_time = htoe32(-1);
    params->passive_time = htoe32(-1);
    params->home_time = htoe32(-1);
    params->channel_num = htoe32(0);
#else
    params->nprobes = -1;
    params->active_time = -1;
    params->passive_time = -1;
    params->home_time = -1;
    params->channel_num = 0;
#endif

    /* extend scan channel time to get more AP probe resp */
#ifdef DSLCPE_ENDIAN
    wl_ioctl(ifname, WLC_GET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));
    org_scan_time = etoh32(org_scan_time);
#endif

    if (org_scan_time < scan_time) {
#ifdef DSLCPE_ENDIAN
        scan_time = htoe32(scan_time);
#endif
        wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &scan_time, sizeof(scan_time));
    }

retry:
    ret = wl_ioctl(ifname, WLC_SCAN, params, params_size);
    if (ret < 0) {
        if (retry_times++ < SCAN_RETRY_TIMES) {
            printf("set scan command failed, retry %d\n", retry_times);
            sleep(1);
            goto retry;
        }
    }

    sleep(2);

#ifdef DSLCPE_ENDIAN
    list->buflen = htoe32(SCAN_DUMP_BUF_LEN);
#else
    list->buflen = SCAN_DUMP_BUF_LEN;
#endif
    ret = wl_ioctl(ifname, WLC_SCAN_RESULTS, scan_result, SCAN_DUMP_BUF_LEN);
    if (ret < 0 && retry_times++ < SCAN_RETRY_TIMES) {
        printf("get scan result failed, retry %d\n", retry_times);
        sleep(1);
        goto retry;
    }

    free(params);

    /* restore original scan channel time */
#ifdef DSLCPE_ENDIAN
    org_scan_time = htoe32(org_scan_time_tmp);
#endif
    wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));

    if (ret < 0)
        return NULL;

    return scan_result;
}

static uint8*
wps_enr_parse_wsc_tlvs(uint8 *tlv_buf, int buflen, uint16 type)
{
    uint8 *cp;
    uint16 *tag;
    int totlen;
    uint16 len;
    uint8 buf[4];

    cp = tlv_buf;
    totlen = buflen;

    /* TLV: 2 bytes for T, 2 bytes for L */
    while (totlen >= 4) {
        memcpy(buf, cp, 4);
        tag = (uint16 *)buf;

        if (ntohs(tag[0]) == type)
            return (cp + 4);
        len = ntohs(tag[1]);
        cp += (len + 4);
        totlen -= (len + 4);
    }

    return NULL;
}

static uint8 *
wps_enr_parse_ie_tlvs(uint8 *tlv_buf, int buflen, uint key)
{
    uint8 *cp;
    int totlen;

    cp = tlv_buf;
    totlen = buflen;

    /* find tagged parameter */
    while (totlen >= 2) {
        uint tag;
        int len;

        tag = *cp;
        len = *(cp +1);

        /* validate remaining totlen */
        if ((tag == key) && (totlen >= (len + 2)))
            return (cp);

        cp += (len + 2);
        totlen -= (len + 2);
    }

    return NULL;
}

static bool
wps_enr_wl_is_wps_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len, char *configured)
{
    uint8 *ie = *wpaie;
    uint8 *data;

    /* If the contents match the WPA_OUI and type=1 */
    if ((ie[1] >= 6) && !memcmp(&ie[2], WPA_OUI "\x04", 4)) {
        ie += 6;
        data = wps_enr_parse_wsc_tlvs(ie, *tlvs_len-6, 0x1044);
        if (data && *data == 0x01)
            *configured = FALSE;
        else
            *configured = TRUE;
        return TRUE;
    }

    /* point to the next ie */
    ie += ie[1] + 2;
    /* calculate the length of the rest of the buffer */
    *tlvs_len -= (int)(ie - *tlvs);
    /* update the pointer to the start of the buffer */
    *tlvs = ie;

    return FALSE;
}

static bool
wps_enr_is_wps_ies(uint8* cp, uint len, char *configured)
{
    uint8 *parse = cp;
    uint parse_len = len;
    uint8 *wpaie;

    while ((wpaie = wps_enr_parse_ie_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
        if (wps_enr_wl_is_wps_ie(&wpaie, &parse, &parse_len, configured))
            break;
    if (wpaie)
        return TRUE;
    else
        return FALSE;
}

extern char *bcm_ether_ntoa(const struct ether_addr *ea, char *buf);

void wlmngr_scanResult(const unsigned int idx, int chose)
{
    char ssid[WL_SSID_SIZE_MAX];
    WL_FLT_MAC_ENTRY *entry;
    char name[32];
    wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
    wl_bss_info_t *bi;
    wl_bss_info_107_t *old_bi;
    uint i, ap_count = 0;
    char configured = 0;
    int current_chanspec;

    snprintf(name, sizeof(name), "wl%d", idx );

    if (wlmngr_getScanResults(name) == NULL)
        return;

#ifdef DSLCPE_ENDIAN
    list->buflen = etoh32(list->buflen);
    list->version = etoh32(list->version);
    list->count = etoh32(list->count);
#endif

    if (list->count == 0)
        return;

    current_chanspec=wlmngr_getCurrentChSpec(idx);
    WLCSM_TRACE(WLCSM_TRACE_DBG,"  found 5G APs count:%d \r\n",list->count );


    bi = list->bss_info;
    for (i = 0; i < list->count; i++) {
        /* Convert version 107 to 108 */
#ifdef DSLCPE_ENDIAN
        bi->version = etoh32(bi->version);
        bi->length = etoh32(bi->length);
        bi->chanspec = etoh16(bi->chanspec);
        bi->beacon_period = etoh16(bi->beacon_period);
        bi->capability = etoh16(bi->capability);
        bi->atim_window = etoh16(bi->atim_window);
        bi->RSSI = etoh16(bi->RSSI);
        bi->nbss_cap = etoh32(bi->nbss_cap);
        bi->vht_rxmcsmap = etoh16(bi->vht_rxmcsmap);
        bi->vht_txmcsmap = etoh16(bi->vht_txmcsmap);
        bi->ie_length = etoh32(bi->ie_length);
        bi->ie_offset = etoh16(bi->ie_offset);
        bi->SNR = etoh16(bi->SNR);
#endif
        WLCSM_TRACE(WLCSM_TRACE_DBG," ssid:%s	 \r\n",(char *)(bi->SSID));
        if (bi->version == LEGACY_WL_BSS_INFO_VERSION) {
            old_bi = (wl_bss_info_107_t *)bi;
	    /* legacy BSS info version is not used for 6GHz devices */
	    /* not using CH20MHZ_CHSPEC to stay compatible with older twigs of KUDU_BRANCH_17_10 */
	    bi->chanspec = (old_bi->channel | WL_CHANSPEC_BW_20 |
		((old_bi->channel <= CH_MAX_2G_CHANNEL) ? WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G));
            bi->ie_length = old_bi->ie_length;
            bi->ie_offset = sizeof(wl_bss_info_107_t);
        }
        if (bi->ie_length) {
            if(ap_count < MAX_AP_SCAN_LIST_LEN) {
                if (((chose == SCAN_WDS) && (bi->chanspec==current_chanspec)) ||
                        ((chose == SCAN_WPS) && (TRUE == wps_enr_is_wps_ies((uint8 *)(((uint8 *)bi)+bi->ie_offset), bi->ie_length, &configured)))) {

                    entry = malloc( sizeof(WL_FLT_MAC_ENTRY));
                    if ( entry != NULL ) {
                        memset(entry, 0, sizeof(WL_FLT_MAC_ENTRY) );
                        strncpy(ssid, (const char *)bi->SSID, sizeof(ssid));
                        if (strncmp(ssid, "\\x00", 4) == 0) {
                            strncpy (entry->ssid, "", sizeof(entry->ssid)); // 0 means empty ssid
                        } else {
                            strncpy(entry->ssid, ssid, sizeof(entry->ssid));
                        }
                        bcm_ether_ntoa(&bi->BSSID, entry->macAddress);
                        strncpy(entry->ifcName,WL_BSSID_IFNAME(idx,MAIN_BSS_IDX), sizeof(entry->ifcName));
                        if (chose == SCAN_WPS) {
                            entry->privacy = bi->capability & DOT11_CAP_PRIVACY;
                            entry->wpsConfigured = configured;
                            entry->wps = TRUE;
                        }
                        list_add( entry, WL_RADIO(idx).m_tblScanWdsMac, struct wl_flt_mac_entry);
                    } else {
                        printf("%s@%d Could not allocat memmro to add WDS Entry\n", __FUNCTION__, __LINE__);
                    }
                }
                ap_count++;
            }
        }
        bi = (wl_bss_info_t*)((int8*)bi + bi->length);
    }
}
//**************************************************************************
// Function Name: scanWdsResult
// Description  : scan WDS AP and return to AP mode.
// Returns      : none.
//**************************************************************************
void wlmngr_scanWdsResult(const unsigned int idx)
{
    wlmngr_scanResult(idx, SCAN_WDS);
}



/***************************************************************************
// Function Name: getWlInfo.
// Description  : create WlMngr object
// Parameters   : none.
// Returns      : n/a.
****************************************************************************/
void wlmngr_getWlInfo(const unsigned int idx, char *buf, char *id)
{
#define CAP_STR_LEN 1024
    char cmd[WL_MID_SIZE_MAX];
    snprintf(cmd, sizeof(cmd), "wl%d", idx);
    wl_iovar_get(cmd, "cap", (void *)buf, CAP_STR_LEN);

    if (!WLCSM_STRCMP("wlcap",id)) {
        if(buf[0])
            buf[strlen(buf)-1]='\0'; // the last char is a '\n', remove it
    }
#undef CAP_STR_LEN
}

//**************************************************************************
// Function Name: getValidBand
// Description  : get valid band matching hardware
// Parameters   : band -- band
// Returns      : valid band
//**************************************************************************
int wlmngr_getValidBand(const unsigned int idx, int band)
{
    int vbands = wlmngr_getBands( idx);
    /* only change if band is not valid */
    if ((band & vbands) == 0) {
        band = (vbands & BAND_B);
        if (band == 0) {
            band = (vbands & BAND_A);
        }
        if (band == 0) {
            band = (vbands & BAND_6G);
        }
    }
    return band;
}

//**************************************************************************
// Function Name: getValidRate
// Description  : get valid rate which is in b range or g range.
// Parameters   : rate -- current rate in the flash.
// Returns      : valid rate.
//**************************************************************************
#ifdef DSLCPE_WLCSM_EXT
long wlmngr_getValidRate(const unsigned int idx, long rate)
{

    int i = 0, bMax = 4, gMax = 12, aMax = 8;
    long gRates[] = { 1000000,  2000000,  5500000,  6000000,
                      9000000,  11000000, 12000000, 18000000,
                      24000000, 36000000, 48000000, 54000000
                    };
    long bRates[] = { 1000000, 2000000, 5500000, 11000000 };
    long aRates[] = { 6000000, 9000000, 12000000, 18000000, 24000000, 36000000, 48000000, 54000000};

    int len=0;
    long ret = 0;
//    idx--;

    WLCSM_TRACE(WLCSM_TRACE_LOG," idx:%d  \r\n",idx );
    char *phytype=gp_adapter_objs[idx].radio.wlPhyType;
    if(!phytype) {

        WLCSM_TRACE(WLCSM_TRACE_ERR," NULL PHYTYPE???? \r\n" );
        return 0;
    }

    len=strlen(phytype);
#ifdef SUPPORT_MIMO
    if ( strncmp(phytype, WL_PHY_TYPE_G,len) == 0 || MIMO_PHYY(idx)) {
#else
    if ( strncmp(phytype, WL_PHY_TYPE_G,len) == 0 ) {
#endif
        for ( i = 0; i < gMax; i++ ) {
            if ( rate == gRates[i] )
                break;
        }
        if ( i < gMax )
            ret = rate;
    } else if ( strncmp(phytype, WL_PHY_TYPE_B,len) == 0 ) {
        for ( i = 0; i < bMax; i++ ) {
            if ( rate == bRates[i] )
                break;
        }
        if ( i < bMax )
            ret = rate;
    } else if ( strncmp(phytype, WL_PHY_TYPE_A,len) == 0 ) {
        for ( i = 0; i < aMax; i++ ) {
            if ( rate == aRates[i] )
                break;
        }
        if ( i < aMax )
            ret = rate;
    }

    return ret;
}
#endif
//**************************************************************************
// Function Name: getCoreRev
// Description  : get current core revision.
// Parameters   : none.
// Returns      : revsion.
// Note		: wlctl revinfo output may change at each wlan release
//                if only tkip is supported in WebUI, this function
//                need to be reviewed
//**************************************************************************
int wlmngr_getCoreRev(const unsigned int idx)
{
    int  found=0;
    unsigned int corerev;
    char cmd[80];
    FILE *fp = NULL;
    char match_str[]="corerev";
    snprintf(cmd, sizeof(cmd), "wlctl -i wl%d revinfo > /var/wl%d_corerev", idx, idx);
    BCMWL_WLCTL_CMD(cmd);
    snprintf(cmd, sizeof(cmd), "/var/wl%d_corerev",  idx);


    fp = fopen(cmd, "r");
    if (fp != NULL) {
        while ( !feof(fp) ) {
            if(fscanf(fp, "%s 0x%x\n", cmd, &corerev)==2 && (!WLCSM_STRCMP(match_str,cmd)))  {
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
char *wlmngr_getPhyType(const unsigned int idx )
{
    char  *phytype = WL_PHY_TYPE_B;
    FILE *fp = NULL;
    char cmd[WL_SIZE_132_MAX];
    snprintf(cmd, WL_SIZE_132_MAX, "wlctl -i wl%d phytype > /var/wl%d", idx,idx);
    BCMWL_WLCTL_CMD(cmd);
    snprintf(cmd, WL_SIZE_132_MAX, "/var/wl%d",  idx);

    // parse the phytype
    fp = fopen(cmd, "r");
    if ( fp != NULL ) {
        if (fgets(cmd, WL_SIZE_132_MAX-1, fp)) {
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
#ifdef SUPPORT_MIMO
            case WL_PHYTYPE_N:
            case WL_PHYTYPE_HT:
            case WL_PHYTYPE_LCN:
                phytype = WL_PHY_TYPE_N;
                break;
            case WL_PHYTYPE_AC:
                phytype = WL_PHY_TYPE_AC;
                break;
#endif
            }
        }
        fclose(fp);
    }

    return phytype;
}



//**************************************************************************
// Function Name: getChannelIm
// Description  : get current channel interference measurement
// Returns      : none.
//**************************************************************************
#ifdef DSLCPE_WLCSM_EXT

int wlmngr_getChannelImState(const unsigned int idx)
{
    char cmd[WL_SIZE_132_MAX];
    FILE *fp = NULL;
    char *cp = NULL;
    int sb;
    char addi[8] = {0};
    chanspec_t chanspec = wlmngr_getCurrentChSpec(idx);

    if ((chanspec & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40) {
        sb = chanspec & WL_CHANSPEC_CTL_SB_MASK;
        if (sb == WL_CHANSPEC_CTL_SB_LOWER)
            strncpy(addi, "l", sizeof(addi));
        else
            strncpy(addi, "u", sizeof(addi));
    }
    else if ((chanspec & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_80) {
        strncpy(addi, "/80", sizeof(addi));
    } else if ((chanspec & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_160) {
        strncpy(addi, "/160", sizeof(addi));
    }

    snprintf(cmd, sizeof(cmd), "wlctl -i wl%d chanim_state %d%s > /var/chanim_state", idx, WL_RADIO_WLCURRENTCHANNEL(idx), addi);
    BCMWL_WLCTL_CMD(cmd);

    snprintf(cmd, sizeof(cmd), "/var/chanim_state");
    fp = fopen(cmd, "r");

    if (fp != NULL)
        cp = fgets(cmd, sizeof(cmd), fp);

    fclose(fp);
    unlink(cmd);

    if ((cp != NULL) && (*cp == '1'))
        return 1;
    else
        return 0;
}

#endif
//**************************************************************************
// Function Name: getCurrentChSpec
// Description  : get current chanspec .
// Returns      : none.
//**************************************************************************
int wlmngr_getCurrentChSpec(const unsigned int idx)
{
    int chanspec = 0;
    char name[32];
    snprintf(name, sizeof(name), "%s", WL_BSSID_IFNAME(idx,0));
    wl_iovar_getint(name, "chanspec", &chanspec);
    WLCSM_TRACE(WLCSM_TRACE_DBG," ----- GET CHANNEL SPEC is %d \r\n",chanspec);


    return chanspec;
}



#ifdef DSLCPE_WLCSM_EXT

void wlmngr_getCurrentChannel(const unsigned int idx )
{
    int sb;
    chanspec_t chanspec;
    chanspec = wlmngr_getCurrentChSpec(idx);
    if ((chanspec & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_80) {
        sb = chanspec & WL_CHANSPEC_CTL_SB_MASK;
        switch (sb) {
        case WL_CHANSPEC_CTL_SB_LL:
            WL_RADIO_WLCURRENTCHANNEL(idx) = (chanspec & WL_CHANSPEC_CHAN_MASK) - 6;
            break;
        case WL_CHANSPEC_CTL_SB_LU:
            WL_RADIO_WLCURRENTCHANNEL(idx) = (chanspec & WL_CHANSPEC_CHAN_MASK) - 2;
            break;
        case WL_CHANSPEC_CTL_SB_UL:
            WL_RADIO_WLCURRENTCHANNEL(idx) = (chanspec & WL_CHANSPEC_CHAN_MASK) + 2;
            break;
        case WL_CHANSPEC_CTL_SB_UU:
            WL_RADIO_WLCURRENTCHANNEL(idx) = (chanspec & WL_CHANSPEC_CHAN_MASK) + 6;
            break;
        default:
            WL_RADIO_WLCURRENTCHANNEL(idx) = (chanspec & WL_CHANSPEC_CHAN_MASK) - 6;
            break;
        }
    } else if ((chanspec & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40) {
        sb = chanspec & WL_CHANSPEC_CTL_SB_MASK;
        WL_RADIO_WLCURRENTCHANNEL(idx) = (chanspec & WL_CHANSPEC_CHAN_MASK) + ((sb == WL_CHANSPEC_CTL_SB_LOWER)?(-2):(2));
    } else {
        WL_RADIO_WLCURRENTCHANNEL(idx) = chanspec & WL_CHANSPEC_CHAN_MASK;
    }

    if(WL_RADIO_WLCHANNEL(idx)==0)
        WL_RADIO_AUTOCHANNELENABLE(idx)=1;
    else
        WL_RADIO_AUTOCHANNELENABLE(idx)=0;

}

void wlmngr_getHwAddr(const unsigned int idx, char *ifname, char *hwaddr)
{
    char buf[WL_MID_SIZE_MAX+60];
    char cmd[WL_SIZE_132_MAX];

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
// Function Name: setupMm_instance_wl[idx].bssMacAddr
// Description  : setup mac address for virtual interfaces
// Parameters   : None
// Returns      : None
//**************************************************************************

void wlmngr_setupMbssMacAddr(const unsigned int idx)
{
    char buf[WL_MID_SIZE_MAX];
    char buf2[WL_MID_SIZE_MAX];
    int i,j, start = GUEST_BSS_IDX,collision=FALSE;
    char macs[WL_SIZE_256_MAX]= {0};
    int mbssid = -1, bandtype = 0;
    char *mbssid_str, *bandtype_str;
    char mbssid_name[64];
    char bandtype_name[64];

    /*Always read Mac from kernel*/
    memset(buf,0,WL_MID_SIZE_MAX);
    memset(buf2,0,WL_MID_SIZE_MAX);
    wlmngr_getHwAddr(idx, WL_BSSID_IFNAME(idx,MAIN_BSS_IDX), buf);
    wlcsm_dm_mngr_set_all_value(idx,MAIN_BSS_IDX,"wlBssid",buf);
    WLCSM_TRACE(WLCSM_TRACE_LOG," mac:%s	\r\n", WL_BSSID_WLBSSID(idx,MAIN_BSS_IDX) );

    snprintf(mbssid_name, sizeof(mbssid_name), "wl%d_%s", idx, "mbssid");
    mbssid_str = nvram_get(mbssid_name);
    if (mbssid_str && mbssid_str[0]) {
	mbssid = atoi(mbssid_str);
    }
    snprintf(bandtype_name, sizeof(bandtype_name), "wl%d_%s", idx, "nband");
    bandtype_str = nvram_get(bandtype_name);
    if (bandtype_str && bandtype_str[0]) {
	bandtype = atoi(bandtype_str);
    }

    // make local address
    // change proper spacing and starting of the first byte to make own local address
    bcmMacStrToNum(buf,WL_BSSID_WLBSSID(idx,MAIN_BSS_IDX));
    if (!MBSSID_ENAB(mbssid, bandtype)) {
	buf[0] = 96 + (buf[5]%(WL_RADIO_WLNUMBSS(idx)-1) * 8);
	buf[0] |= 0x2;
    }

    for (i=start; i<WL_RADIO_WLNUMBSS(idx); i++) {
        if(WL_BSSID_WLBSSID(idx,i)) {
            wlcsm_dm_mngr_set_all_value(idx,i,"BSSID",WL_BSSID_WLBSSID(idx,i));
        } else {
            // construct virtual hw addr
            buf[5] = (buf[5]&~(WL_RADIO_WLNUMBSS(idx)-1))|((WL_RADIO_WLNUMBSS(idx)-1)&(buf[5]+1));
            do {
                collision = FALSE;
                for(j=0; j<(i-start); j++) {
                    if(macs[j]== buf[5])  {
                        collision=TRUE;
                        buf[5]++;
                        break;
                    }
                }
            } while(collision);
            macs[i-start]=buf[5];
            bcmMacNumToStr(buf,buf2);
            wlcsm_dm_mngr_set_all_value(idx,i,"wlBssid",buf2);
            wlcsm_dm_mngr_set_all_value(idx,i,"BSSID",buf2);
        }
        if(WL_BSSID_WLENBLSSID(idx,i)) {
            WL_AP_APENABLE(idx,i)=1;
            wlcsm_strcpy(&WL_AP_APSTATUS(idx,i),"Enabled");
            wlcsm_strcpy(&WL_BSSID_SSIDSTATUS(idx,i),"Up");
        } else {
            WL_AP_APENABLE(idx,i)=0;
            wlcsm_strcpy(&WL_AP_APSTATUS(idx,i),"Disabled");
            wlcsm_strcpy(&WL_BSSID_SSIDSTATUS(idx,i),"Down");
        }
    }
}

void wlmngr_setup_if_mac(const unsigned int idx)
{
    int i;
    char cmd[WL_SIZE_132_MAX];
    char tmp[WL_SIZE_132_MAX], *mac;
    for (i=0; i<WL_RADIO_WLNUMBSS(idx); i++) {
        snprintf(tmp,sizeof(tmp),"%s_hwaddr", WL_BSSID_IFNAME(idx,i));
        mac=wlcsm_nvram_get(tmp);
        if(mac)
            snprintf(cmd, sizeof(cmd), "ifconfig %s hw ether %s 2>/dev/null", WL_BSSID_IFNAME(idx,i), mac);
        else
            snprintf(cmd, sizeof(cmd), "ifconfig %s hw ether %s 2>/dev/null", WL_BSSID_IFNAME(idx,i), WL_BSSID_WLBSSID(idx,i));
#ifdef WLCSM_DEBUG
        if(bcmSystem(cmd))
            fprintf(stderr, "%s:%d:	run cmd error :%s\n",__FUNCTION__,__LINE__,cmd);
#else
        bcmSystem(cmd);
#endif
    }
}

//**************************************************************************
// Function Name: getBands
// Description  : get Avaiable Bands (2.4G/5G/6G)
// Parameters   : None
// Returns      : Bands
//**************************************************************************

int wlmngr_getBands(const unsigned int idx)
{
    char buf[WL_SIZE_132_MAX];
    char buf2[WL_SIZE_132_MAX];
    int bands = 0,len;
    char cmd[80];
    snprintf(cmd, sizeof(cmd), "wlctl -i wl%d bands > /var/wl%dbands", idx, idx );
    BCMWL_WLCTL_CMD(cmd);
    snprintf(cmd, sizeof(cmd), "/var/wl%dbands", idx );

    buf2[0] = '\0';
    FILE *fp = fopen(cmd, "r");
    if ( fp != NULL ) {
        if (fgets(buf, sizeof(buf), fp)) {
            if (strchr(buf, 'a')) {
                bands |= BAND_A;
                snprintf(buf2, sizeof(buf2), "%s", "5GHz");
            }
            if (strchr(buf, 'b')) {
                bands |= BAND_B;
                snprintf(buf2, sizeof(buf2), "%s%s", strlen(buf2) ? "," : "",
                    "2.4GHz");
            }
            if (strstr(buf, "6g")) {
                bands |= BAND_6G;
                snprintf(buf2, sizeof(buf2), "%s%s", strlen(buf2) ? "," : "",
                    "6GHz");
            }
        }
        fclose(fp);
    }
    WL_RADIO_WLMBANDS(idx)=bands;
    if(WL_RADIO_WLFREQBANDS(idx))
        free(WL_RADIO_WLFREQBANDS(idx));
    len=strlen(buf2)+1;
    WL_RADIO_WLFREQBANDS(idx)=malloc(len);
    if(WL_RADIO_WLFREQBANDS(idx))
        strncpy(WL_RADIO_WLFREQBANDS(idx),buf2,len);
    return bands;
}
#endif


//**************************************************************************
// Function Name: scanForAddr
// Description  : scan input string for mac address
// Parameters   : line - input string
//                isize - size of input string
//                start - return pointer to the address
//                size - size of mac address
// Returns      : 1 success, 0 failed
//**************************************************************************

char wlmngr_scanForAddr(char *line, int isize, char **start, int *size)
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
int wlmngr_scanFileForMAC(char *fname, char *mac)
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

#ifdef IDLE_PWRSAVE

static int  _wlmngr_radio_has_associated_sta(const unsigned int idx)
{
    int sub_idx=0;
    WLCSM_WLAN_AP_STA_STRUCT  *associated_sta;
    for (sub_idx=0; sub_idx<WL_RADIO_WLNUMBSS(idx) && sub_idx < WL_MAX_NUM_SSID; sub_idx++)  {
        associated_sta = WL_AP_STAS(idx,sub_idx);
        while(associated_sta) {
            if(associated_sta->associated)
                return 1;
            associated_sta=(WLCSM_WLAN_AP_STA_STRUCT *)associated_sta->next;
        }
    }
    return 0;
}

static inline void wlmngr_togglePowerSave_aspm(const unsigned int idx, char assoc, char *cmd)
{

    if (!WLCSM_STRCMP(WL_PHYTYPE(idx), WL_PHY_TYPE_AC)) {
        if (assoc) {
            sprintf(cmd, "wlctl -i wl%d aspm 0", idx);
        } else {
            /* Only enable L1 mode, because L0s creates EVM issues. The power savings are the same */
            if (WL_RADIO_GPIOOVERLAYS(idx) & BP_OVERLAY_PCIE_CLKREQ)
                sprintf(cmd, "wlctl -i wl%d aspm 0x102", idx);
            else
                sprintf(cmd, "wlctl -i wl%d aspm 0x2", idx);
        }
        bcmSystem(cmd);
    }
}

//**************************************************************************
// Function Name: togglePowerSave
// Description  : Disable PowerSave feature if any STA is associated, otherwise leave as current setting
// Parameters   : None
// Returns      : None
//**************************************************************************
void wlmngr_togglePowerSave()
{

    char cmd[WL_LG_SIZE_MAX];
    char assoc=0, associated[WL_MAX_NUM_RADIO]= {0, 0}, anyAssociated = 0;
    int i;
    for (i=0; i<act_wl_cnt; i++) {
        if( _wlmngr_radio_has_associated_sta(i)) {
            associated[i] = 1;  /* for each adapter */
            anyAssociated = 1;  /* for global association of all adapters */
            break; /* associated sta found */
        }
    }
    for (i=0; i<act_wl_cnt; i++) {
        if (associated[i]) {
            WLCSM_TRACE(WLCSM_TRACE_DBG,"disable rxchain_pwrsave_enable on interface :%s\r\n",WL_BSSID_IFNAME(i,0));
            snprintf(cmd, sizeof(cmd), "wlctl -i %s rxchain_pwrsave_enable 0",WL_BSSID_IFNAME(i,0));
            BCMWL_WLCTL_CMD(cmd);
        } else {
            /* restore back to the user setting */
            snprintf(cmd, sizeof(cmd), "wlctl -i %s rxchain_pwrsave_enable %d", WL_BSSID_IFNAME(i,0),WL_RADIO_WLRXCHAINPWRSAVEENABLE(i));
            BCMWL_WLCTL_CMD(cmd);
        }

        wlmngr_togglePowerSave_aspm(i, anyAssociated, cmd);
    }
}
#endif

#ifdef DSLCPE_WLCSM_EXT
char *wlmngr_getVer(const unsigned int idx,char *version)
{
    char *tag = "version ";
    char buf[WL_SIZE_132_MAX];
    char *p;
    char format[8]= {0};

    FILE *fp = NULL;

    snprintf(format,sizeof(format),"%%%ds",WL_VERSION_STR_LEN-1);
    snprintf(buf, sizeof(buf), "wlctl -i %s  ver > /var/wlver ",WL_BSSID_IFNAME(idx,0));
    WLCSM_TRACE(WLCSM_TRACE_LOG," run wlctl command:%s \r\n",buf );
    BCMWL_WLCTL_CMD(buf);
    fp = fopen("/var/wlver", "r");
    if ( fp != NULL ) {
        fgets(buf, sizeof(buf), fp);
        fgets(buf, sizeof(buf), fp);
        p = strstr(buf, tag);
        if (p != NULL) {
            p += strlen(tag);
            sscanf(p, format,version);
            fclose(fp);
            return version;
        }
        fclose(fp);
    } else {
        printf("/var/wlver open error\n");
    }
    return NULL;
}
#endif


//**************************************************************************
// Function Name: setupChannel
// Description  : Setup channel
// Parameters   : None
// Returns      : None
//**************************************************************************
#ifdef DSLCPE_WLCSM_EXT
void wlmngr_autoChannel(const unsigned int idx)
{
    nvram_set("acs_mode", "legacy");
}
#endif

//**************************************************************************
// Function Name: setupRegulatory
// Description  : Setup regulatory
// Parameters   : None
// Returns      : None
//**************************************************************************
void wlmngr_setupRegulatory(const unsigned int idx)
{
    return;

}

#ifdef DSLCPE_WLCSM_EXT
//**************************************************************************
// Function Name: printSsidList
// Description  : Print SSID list for multiple set of options in security page
// Parameters   : None
// Returns      : None
//**************************************************************************
void wlmngr_printSsidList(const unsigned int idx, char *text)
{
    int i, wsize;
    char *prtloc = text;
    for (i=0; i<WL_RADIO(idx).wlNumBss; i++) {
        char ssidStr[BUFLEN_64];

        /* If there are any escape characters in the ssid, cmsXml_escapeStringEx will replace them; */
        wlmngr_escapeStringEx(WL_BSSID_WLSSID(idx,i), ssidStr, sizeof(ssidStr)-1);

        sprintf(prtloc, "<option value='%d'>%s</option>\n%n", i, ssidStr, &wsize);
        prtloc +=wsize;
    }
    *prtloc = 0;
}
#endif




void wlmngr_WlConfDown(const unsigned int idx)
{
    extern int wlconf_down(char *name);
    char name[32];

#ifndef  DSLCPE_WLCONF_CMD
    snprintf(name, sizeof(name), "%s", WL_BSSID_IFNAME(idx,0));
    wlconf_down(name);
#else
    snprintf(name, sizeof(name), "wlconf %s down", WL_BSSID_IFNAME(idx,0));
    bcmSystem(name);
#endif
}

void wlmngr_WlConfStart(const unsigned int idx)
{
    extern int wlconf_start(char *name);
    char name[32];

    snprintf(name, sizeof(name), "%s", WL_BSSID_IFNAME(idx,0));
#ifndef  DSLCPE_WLCONF_CMD
    snprintf(name, sizeof(name), "%s", WL_BSSID_IFNAME(idx,0));
    wlconf_start(name);
#else
    snprintf(name, sizeof(name), "wlconf %s start", WL_BSSID_IFNAME(idx,0));
    bcmSystem(name);
#endif
}

//**************************************************************************
// Function Name: doWlConf
// Description  : basic setup for wireless interface.
// Parameters   : none.
// Returns      : None.
//**************************************************************************
void wlmngr_doWlConf(const unsigned int idx)
{
    extern int wlconf(char *name);
    extern int wlconf_start(char *name);
    char name[32];

    if (MIMO_PHY) {
        // n required, i.e. if 1, supports only 11n STA, 11g STA cannot not assoc
        if(WLCSM_STRCMP(WL_RADIO_NMODE(idx), WL_OFF)) {
            snprintf(name, sizeof(name), "wlctl -i %s nreqd %d", WL_BSSID_IFNAME(idx,0), WL_AP(idx,0).wlNReqd);
            BCMWL_WLCTL_CMD(name);

        }
    }

    snprintf(name, sizeof(name), "wlconf %s up", WL_BSSID_IFNAME(idx,0));
    bcmSystem(name);
}

int  wlmngr_strtol(char *str,int base,long *val)
{

    char *endptr;
    *val = strtol(str,&endptr, base);

    /*  Check for various possible errors */

    if ((errno == ERANGE && (*val == LONG_MAX || *val == LONG_MIN))
            || (errno != 0 && *val == 0)) {
        return -1;
    }

    if (endptr == str) {
        return -1;
    }
    return 0;
}
//**************************************************************************
// Function Name: getpidbyname
// Description  : given a name of a process/thread, return the pid.  This
//                local version is used only if the libcms_util does not
//                have this function.
// Parameters   : name of the process/thread
// Returns      : pid or -1 if not found
//**************************************************************************
#include <fcntl.h>
#include <dirent.h>
int wlmngr_getPidByName(const char *name)
{
    DIR *dir;
    FILE *fp;
    struct dirent *dent;
    UBOOL8 found=FALSE;
    long pid;
    int  rc, p, i;
    int rval = CMS_INVALID_PID;
    char filename[BUFLEN_256];
    char processName[BUFLEN_256];

    if (NULL == (dir = opendir("/proc"))) {
        WLCSM_TRACE(WLCSM_TRACE_ERR,"could not open /proc");
        return rval;
    }

    while (!found && (dent = readdir(dir)) != NULL) {
        /*
         * Each process has its own directory under /proc, the name of the
         * directory is the pid number.
         */
        if ((dent->d_type == DT_DIR) &&
                (!wlmngr_strtol(dent->d_name, 10, &pid))) {
            snprintf(filename, sizeof(filename), "/proc/%ld/stat", pid);
            if ((fp = fopen(filename, "r")) == NULL) {
                WLCSM_TRACE(WLCSM_TRACE_DBG,"could not open %s", filename);
            } else {
                /* Get the process name, format: 913 (consoled) */
                memset(processName, 0, sizeof(processName));
                rc = fscanf(fp, "%d (%s", &p, processName);
                fclose(fp);

                if (rc >= 2) {
                    i = strlen(processName);
                    if (i > 0) {
                        /* strip out the trailing ) character */
                        if (processName[i-1] == ')')
                            processName[i-1] = 0;
                    }
                }

                if (!strncmp(processName, name,strlen(name))) {
                    rval = pid;
                    found = TRUE;
                }
            }
        }
    }

    closedir(dir);

    return rval;
}



//**************************************************************************
// Function Name: wlmngr_detectSMP
// Description  : check if we are running on a SMP system
// Parameters   : None.
// Returns      : 1 if we are on SMP system, 0 otherwise.
//**************************************************************************
#define TMP_WL_SMP_FILENAME "/tmp/wl_smp"

int wlmngr_detectSMP()
{
    char tmpBuf[256]= {0};
    int fd;
    int is_smp=0;

    snprintf(tmpBuf, sizeof(tmpBuf), "cat /proc/version > %s",
             TMP_WL_SMP_FILENAME);
    bcmSystemEx(tmpBuf, 1);
    fd = open(TMP_WL_SMP_FILENAME, O_RDONLY);
    if (fd > 0) {
        int rc;
        memset(tmpBuf, 0, sizeof(tmpBuf));
        rc = read(fd, tmpBuf, sizeof(tmpBuf)-1);
        close(fd);

        if (rc && (strstr(tmpBuf, "SMP")))
            is_smp = 1;
    }
    unlink(TMP_WL_SMP_FILENAME);

    return is_smp;
}

#ifdef SUPPORT_WSC
int wlmngr_wscPincheck(char *pin_string)
{
    unsigned long PIN = strtoul(pin_string, NULL, 10 );;
    unsigned long int accum = 0;
    unsigned int len = strlen(pin_string);

#ifdef WL_WLMNGR_DBG
    printf("pin_string=%s PIN=%lu\n", pin_string, PIN );
#endif

    if (len != 4 && len != 8)
        return 	-1;

    if (len == 8) {
        accum += 3 * ((PIN / 10000000) % 10);
        accum += 1 * ((PIN / 1000000) % 10);
        accum += 3 * ((PIN / 100000) % 10);
        accum += 1 * ((PIN / 10000) % 10);
        accum += 3 * ((PIN / 1000) % 10);
        accum += 1 * ((PIN / 100) % 10);
        accum += 3 * ((PIN / 10) % 10);
        accum += 1 * ((PIN / 1) % 10);

#ifdef WL_WLMNGR_DBG
        printf("accum=%lu\n", accum );
#endif
        if (0 == (accum % 10))
            return 0;
    } else if (len == 4)
        return 0;

    return -1;
}


int  wl_wscPinGen( void )
{
    unsigned long PIN=0;
    unsigned long accum = 0;
    int digit;
    char devPwd[32] = {0};
    unsigned char rand_bytes[8];

    srand(time((time_t *)NULL));
    if(!wlcsm_get_random(rand_bytes,8)) {
        snprintf( devPwd, sizeof(devPwd), "%08lu", *(long unsigned int*)rand_bytes);
        devPwd[7] = '\0';
        PIN = strtoul( devPwd, NULL, 10 );
        PIN *= 10;
        accum += 3 * ((PIN / 10000000) % 10);
        accum += 1 * ((PIN / 1000000) % 10);
        accum += 3 * ((PIN / 100000) % 10);
        accum += 1 * ((PIN / 10000) % 10);
        accum += 3 * ((PIN / 1000) % 10);
        accum += 1 * ((PIN / 100) % 10);
        accum += 3 * ((PIN / 10) % 10);
        digit = (accum % 10);
        accum = (10 - digit) % 10;
        PIN += accum;
        snprintf( devPwd, sizeof(devPwd), "%08lu", PIN );
        devPwd[8] = '\0';
    }

    printf("WPS Device PIN = %s\n", devPwd);
    if(strlen(devPwd)>1)
        nvram_set("wps_device_pin", devPwd);
    else
        nvram_set("wps_device_pin", "12345670");
    return 0;
}



//**************************************************************************
// Function Name: setupAll
// Description  : all setup for wireless interface.
// Parameters   : none.
// Returns      : None.
//**************************************************************************
int wlmngr_get_random_bytes(char *rand, int len)
{
    int dev_random_fd;
    dev_random_fd = open("/dev/urandom", O_RDONLY|O_NONBLOCK);
    if ( dev_random_fd < 0 ) {
        printf("Could not open /dev/urandom\n");
        return WSC_FAILURE;
    }

    read(dev_random_fd, rand, len);
    close(dev_random_fd);
    return WSC_SUCCESS;
}

/*This function is used to generate the SSID and PSK key */
/*when WSC is enabled and unconfig mode*/
void  wlmngr_genWscRandSssidPsk(const unsigned int idx )
{
    WLCSM_TRACE(WLCSM_TRACE_DBG," TODO???????????? \r\n" );
}

static int
write_to_wps(int fd, char *cmd)
{
    int n;
    int len;
    struct sockaddr_in to;

    len = strlen(cmd)+1;

    /* open loopback socket to communicate with wps */
    memset(&to, 0, sizeof(to));
    to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    to.sin_family = AF_INET;
    to.sin_port = htons(WPS_UI_PORT);

    n = sendto(fd, cmd, len, 0, (struct sockaddr *)&to,
               sizeof(struct sockaddr_in));



    /* Sleep 100 ms to make sure
       WPS have received socket */
    sleep(1); // USLEEP(100*1000);
    return n;
}

static int
read_from_wps(int fd, char *databuf, int datalen)
{
    int n, max_fd = -1;
    fd_set fdvar;
    struct timeval timeout;
    int recvBytes;
    struct sockaddr_in addr;
    socklen_t size = sizeof(struct sockaddr);

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    FD_ZERO(&fdvar);

    /* get ui fd */
    if (fd >= 0) {
        FD_SET(fd, &fdvar);
        max_fd = fd;
    }

    if (max_fd == -1) {
        fprintf(stderr, "wps ui utility: no fd set!\n");
        return -1;
    }

    n = select(max_fd + 1, &fdvar, NULL, NULL, &timeout);

    if (n < 0) {
        return -1;
    }

    if (n > 0) {
        if (fd >= 0) {
            if (FD_ISSET(fd, &fdvar)) {
                recvBytes = recvfrom(fd, databuf, datalen,
                                     0, (struct sockaddr *)&addr, &size);

                if (recvBytes == -1) {
                    fprintf(stderr,
                            "wps ui utility:recv failed, recvBytes = %d\n", recvBytes);
                    return -1;
                }
                return recvBytes;
            }

            return 0;
        }
    }

    return -1;
}

int
parse_wps_env(char *buf)
{
    char *argv[32] = {0};
    char *item, *p, *next;
    char *name, *value;
    int i;
    /*
    int unit, subunit;
    char nvifname[IFNAMSIZ];
           */

    /* Seperate buf into argv[] */
    for (i = 0, item = buf, p = item;
            item && item[0];
            item = next, p = 0, i++) {
        /* Get next token */
        strtok_r(p, " ", &next);
        argv[i] = item;
    }

    /* Parse message */
    wps_config_command = 0;
    wps_action = 0;
    wps_method = 0; /* Add in PF #3 */
    memset(wps_autho_sta_mac, 0, sizeof(wps_autho_sta_mac)); /* Add in PF #3 */

    for (i = 0; argv[i]; i++) {
        value = argv[i];
        name = strsep(&value, "=");
        if (name) {
            if (!WLCSM_STRCMP(name, "wps_config_command"))
                wps_config_command = atoi(value);
            else if (!WLCSM_STRCMP(name, "wps_action"))
                wps_action = atoi(value);
            else if (!WLCSM_STRCMP(name, "wps_uuid"))
                strncpy(wps_uuid,value, sizeof(wps_uuid));
            else if (!WLCSM_STRCMP(name, "wps_method")) /* Add in PF #3 */
                wps_method = atoi(value);
            else if (!WLCSM_STRCMP(name, "wps_autho_sta_mac")) /* Add in PF #3 */
                memcpy(wps_autho_sta_mac, value, sizeof(wps_autho_sta_mac));
        }
    }

    return 0;
}

int
get_wps_env()
{
    int fd = -1;
    char databuf[256];
    int datalen = sizeof(databuf);
    if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        fprintf(stderr, "wps ui utility: failed to open loopback socket\n");
        return -1;
    }

    write_to_wps(fd, "GET");

    /* Receive response */
    if (read_from_wps(fd, databuf, datalen) > 0)
        parse_wps_env(databuf);
    else
        /* Show error message ? */
        fprintf(stderr,  "read_from_wps() failure\n");

    close(fd);
    return 0;
}


int
set_wps_env(char *uibuf)
{
    int wps_fd = -1;
    struct sockaddr_in to;
    int sentBytes = 0;
    uint32 uilen = strlen(uibuf);

    if ((wps_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {

        goto exit;
    }

    /* send to WPS */
    to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    to.sin_family = AF_INET;
    to.sin_port = htons(WPS_UI_PORT);

    sentBytes = sendto(wps_fd, uibuf, uilen, 0, (struct sockaddr *) &to,
                       sizeof(struct sockaddr_in));

    if (sentBytes != uilen) {
        goto exit;
    }

    /* Sleep 100 ms to make sure
       WPS have received socket */
    sleep(1); // USLEEP(100*1000);
    return 0;

exit:
    if (wps_fd > 0)
        close(wps_fd);

    /* Show error message ?  */
    return -1;
}





/***************************************************************************
// Function Name: BcmWl_getScanWdsMacSSID
// Description  : retrieve WDS AP's MAC address *and SSID*
// Parameters   : pVoid - point to start position in the list
//                mac - buffer for mac addr
//                ssid - buffer for ssid
// Returns      : WdsMacNode or NULL if not found
****************************************************************************/

void *BcmWl_getScanWdsMacSSID(const unsigned int idx,void *pVoid, char *mac, char *ssid)
{
    WL_FLT_MAC_ENTRY *entry = NULL;

    list_get_next( ((WL_FLT_MAC_ENTRY *)pVoid),WL_RADIO(idx).m_tblScanWdsMac, entry);
    if ( entry != NULL ) {
        strncpy( mac, entry->macAddress, WL_MID_SIZE_MAX );
        strncpy(ssid, entry->ssid, WL_SSID_SIZE_MAX);
    }
    return (void *)entry;
}
#endif /* SUPPORT_WSC */

#define CHIPINFO_DEVICE_NAME  "/dev/chipinfo"

/*read STBC value from chipinfo*/
int wlmngr_ReadChipInfoStbc(void)
{
    int chipinfoFd;
    int rc;

    CHIPINFO_IOCTL_PARMS ioctlParms;
    ioctlParms.result = -1;

    chipinfoFd = open(CHIPINFO_DEVICE_NAME, O_RDWR);
    if ( chipinfoFd != -1 ) {
        ioctlParms.action = CAN_STBC;
        rc = ioctl(chipinfoFd, CHIPINFO_IOCTL_GET_CHIP_CAPABILITY, &ioctlParms);
        if (rc < 0) {
            printf("%s@%d Read CHIP INFO Failure\n", __FUNCTION__, __LINE__);
        }
    } else {
        printf("Unable to open device %s", CHIPINFO_DEVICE_NAME);
    }

    close(chipinfoFd);

    /* possible return values: 1 - STBC supported, 0 - STBC not supported, -EINVAL (-22)/or (-1) - no STBC in chip info */

    return ioctlParms.result;
}

void wlmngr_crash_log_backup_init(void)
{
    int needUnmount = FALSE, needMount = FALSE;
    char cmd[WL_SIZE_512_MAX]={0};
    static char currDbgMonitorMtd[WL_MID_SIZE_MAX]={0};
    static char currDbgMonitorDir[WL_SIZE_256_MAX]={0};
    char *currentMtd = NULL;
    char *currentDir = NULL;
    char *dbgMonitorMtd = NULL;
    char *dbgMonitorDir = NULL;

    if (strlen(currDbgMonitorMtd) > 0)
    {
        currentMtd = currDbgMonitorMtd;
    }
    if (strlen(currDbgMonitorDir) > 0)
    {
        currentDir = currDbgMonitorDir;
    }

    dbgMonitorMtd = wlcsm_nvram_get("crash_log_backup_mtd");
    dbgMonitorDir = wlcsm_nvram_get("crash_log_backup_dir");
    WLCSM_TRACE(WLCSM_TRACE_DBG,"curMtd:%s curDir:%s \r\n", currentMtd, currentDir);
    WLCSM_TRACE(WLCSM_TRACE_DBG,"dbgMonitorMtd:%s dbgMonitorDir:%s \r\n", dbgMonitorMtd, dbgMonitorDir);
    if ((currentMtd && currentDir) && !(dbgMonitorMtd && dbgMonitorDir)) //unset mtd or dir
    {
        needUnmount = TRUE;
    }
    if (dbgMonitorMtd && dbgMonitorDir)
    {
        needMount = TRUE;
        if (currentMtd && currentDir)
        {
            if (strcmp(currentMtd, dbgMonitorMtd) || strcmp(currentDir, dbgMonitorDir))
               needUnmount = TRUE; 
            if (strcmp(currentMtd, dbgMonitorMtd)==0 && strcmp(currentDir, dbgMonitorDir)==0) // all the same
               needMount=FALSE;
        }
    }
    WLCSM_TRACE(WLCSM_TRACE_DBG, "mount:%d unmount:%d \r\n", needMount, needUnmount);
    if (needMount)
    {
        snprintf(cmd, WL_SIZE_512_MAX, "mkdir -p %s", dbgMonitorDir);
        bcmSystem(cmd);
    }
    if (needUnmount && needMount)
    {
        snprintf(cmd, WL_SIZE_512_MAX, "hnddm.sh %s %s %s %s 2> /dev/null", dbgMonitorMtd, dbgMonitorDir, currentMtd, currentDir);
        bcmSystem(cmd);
    }
    else if (needUnmount) // only unmount
    {
        snprintf(cmd, WL_SIZE_512_MAX, "hnddm.sh \"\" \"\" %s %s 2> /dev/null", currentMtd, currentDir);
        bcmSystem(cmd);
    }
    else if (needMount) // only mount
    {
        snprintf(cmd, WL_SIZE_512_MAX, "hnddm.sh %s %s 2> /dev/null", dbgMonitorMtd, dbgMonitorDir);
        bcmSystem(cmd);
    }
    memset(currDbgMonitorMtd, '\0', WL_MID_SIZE_MAX);
    strncpy(currDbgMonitorMtd, nvram_safe_get("crash_log_backup_mtd"), WL_MID_SIZE_MAX-1);
    memset(currDbgMonitorDir, '\0', WL_SIZE_256_MAX);
    strncpy(currDbgMonitorDir, nvram_safe_get("crash_log_backup_dir"), WL_SIZE_256_MAX-1);
}

//end of File
