#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>
#include <bcmnvram.h>
#include <ralink.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include  <float.h>
#include <ap_priv.h>
#include <wlutils.h>
#include <roamast.h>

int xTxR = 0;

typedef struct _roaming_info {
       	char mac[19];
       	char rssi_xR[xR_MAX][7];
	char curRate[33];
       	char dump;
} roam;
typedef struct _roam_sta {
       	roam sta[128];
} roam_sta;

#if 0
int check_rssi_threshold(int bssidx, int vifidx)
{
	int i = 0;
	int threshold = 0;
	rast_sta_info_t *sta = bssinfo[bssidx].assoclist[vifidx];

	for (i=0;i<xTxR;i++) {
		if (sta-rssi_xR[i] < bssinfo[bssidx].user_low_rssi) {
			threshold = 1;
		}
		else{
			threshold = 0;
			break;
		}
	}
	return threshold;

}
#endif
void get_stainfo(int bssidx, int vifidx)
{
	char *sp, *op;
	char wlif_name[32], header[128], data[2048];
	int hdrLen, staCount=0, getLen;
	struct iwreq wrq;
	roam_sta *ssap;
	struct rast_sta_info *staInfo = NULL;
	char prefix[] = "wlXXXXXXXXXX_";
	char header_t[128]={0};
	int stream = 0;
	char tmp[128]={0};
	char rssinum[16]={0};
	unsigned long long cur_txrx_bytes = 0;
	int i = 0;
	int32 rssi_xR[xR_MAX];
	int rssi_total = 0;

	snprintf(prefix, sizeof(prefix), "wl%d_", bssidx);
	if (!(xTxR = nvram_get_int(strcat_r(prefix, "HT_RxStream", tmp))))
		return;

	if(xTxR > xR_MAX)
		xTxR = xR_MAX;


	if (vifidx > 0) {
		sprintf(data, "wl%d.%d_ifname", bssidx, vifidx);
		strcpy(wlif_name, nvram_safe_get(data));
	}
	else
		strcpy(wlif_name, bssinfo[bssidx].wlif_name);

	memset(data, 0x00, sizeof(data));
       	wrq.u.data.length = sizeof(data);
       	wrq.u.data.pointer = (caddr_t) data;
       	wrq.u.data.flags = ASUS_SUBCMD_GROAM;

	if (wl_ioctl(wlif_name, RTPRIV_IOCTL_ASUSCMD, &wrq) < 0) {
		RAST_INFO("[%s]: WI[%s] Access to StaInfo failure\n", __FUNCTION__, wlif_name);
		return;
	}

	memset(header, 0, sizeof(header));
	memset(header_t, 0, sizeof(header_t));
	hdrLen = sprintf(header_t, "%-19s", "MAC");
	strcpy(header, header_t);

	for (stream = 0; stream < xTxR; stream++) {
		sprintf(rssinum, "RSSI%d", stream);
		memset(header_t, 0, sizeof(header_t));
		hdrLen += sprintf(header_t, "%-7s", rssinum);
		strncat(header, header_t, strlen(header_t));
    }
	hdrLen += sprintf(header_t, "%-33s", "CURRATE");
	strncat(header, header_t, strlen(header_t));
	strcat(header,"\n");
	hdrLen++;


	if (wrq.u.data.length > 0 && data[0] != 0) {

		getLen = strlen(wrq.u.data.pointer + hdrLen);

		ssap = (roam_sta *)(wrq.u.data.pointer + hdrLen);
		op = sp = wrq.u.data.pointer + hdrLen;
		while (*sp && ((getLen - (sp-op)) >= 0)) {
			ssap->sta[staCount].mac[18]='\0';
			for (stream = 0; stream < xTxR; stream++) {
				ssap->sta[staCount].rssi_xR[stream][6]='\0';
			}
			ssap->sta[staCount].curRate[32]='\0';
			sp += hdrLen;
			staCount++;
		}

#ifdef RTCONFIG_WIRELESSREPEATER
		char *aif;
		unsigned char pap_bssid[18];
		struct iwreq wrq1;
		if(sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_band") == bssidx) {
			memset(header, 0, sizeof(header));
		       	aif = nvram_get( strcat_r(bssinfo[bssidx].prefix, "vifs", header) );
		       	if(wl_ioctl(aif, SIOCGIWAP, &wrq1)>=0); {
			       	wrq1.u.ap_addr.sa_family = ARPHRD_ETHER;
			       	sprintf(pap_bssid, "%02X:%02X:%02X:%02X:%02X:%02X",
					       	(unsigned char)wrq1.u.ap_addr.sa_data[0], (unsigned char)wrq1.u.ap_addr.sa_data[1],
					       	(unsigned char)wrq1.u.ap_addr.sa_data[2], (unsigned char)wrq1.u.ap_addr.sa_data[3],
					       	(unsigned char)wrq1.u.ap_addr.sa_data[4], (unsigned char)wrq1.u.ap_addr.sa_data[5]  );
		       	}
	       	}
#endif

		if( !staCount ) return;
		// add to assoclist //
		for(hdrLen=0; hdrLen< staCount; hdrLen++) {
#ifdef RTCONFIG_WIRELESSREPEATER
			// pap bssid,skip //
			if( !strncmp(pap_bssid, ssap->sta[hdrLen].mac, sizeof(pap_bssid))) {
				RAST_DBG("[%s]: P-AP BSSID[%s]\n", __FUNCTION__, pap_bssid);
				continue;
			}
#endif
		       	staInfo = rast_add_to_assoclist(bssidx, vifidx, ether_aton(ssap->sta[hdrLen].mac));

			for( getLen=0; getLen<xTxR; getLen++ ) {
				rssi_xR[getLen] = !atoi(ssap->sta[hdrLen].rssi_xR[getLen]) ? -100 : atoi(ssap->sta[hdrLen].rssi_xR[getLen]);
				rssi_total = rssi_total + rssi_xR[getLen];
			}
			memcpy(staInfo->mac_addr, ssap->sta[hdrLen].mac, sizeof(staInfo->mac_addr) );

		staInfo->rssi = rssi_total / xTxR ;
		cur_txrx_bytes = atoi(ssap->sta[hdrLen].curRate);
		staInfo->datarate = (float)((cur_txrx_bytes - staInfo->last_txrx_bytes) >> 7/* bytes to Kbits*/) / RAST_POLL_INTV_NORMAL/* Kbps */;
		staInfo->last_txrx_bytes = cur_txrx_bytes;
		staInfo->active = uptime();

		if (rast_dbg) {
			RAST_DBG("[%s]: [%s][%d][%s] RATE[%f]\t", __FUNCTION__, wlif_name, hdrLen,
					       	staInfo->mac_addr, staInfo->datarate);
			for (i=0;i<xTxR;i++) {
				RAST_DBG("RSSI[%d:%d]\t", i, rssi_xR[i]);
			}
			RAST_DBG(" RSSI Avg = %d\n", staInfo->rssi );
		}

		}
	}
	return;
}