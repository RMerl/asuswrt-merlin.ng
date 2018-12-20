#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>
#include <netinet/ether.h>
#include <roamast.h>

#include <qca.h>

#ifdef RTCONFIG_ADV_RAST
rast_maclist_t *r_maclist_old_table[MAX_IF_NUM][MAX_SUBIF_NUM];
#endif

void get_stainfo(int bssidx, int vifidx)
{
	#define STA_LOW_RSSI_PATH "/tmp/low_rssi"
	#define STA_TXRX_BYTES_PATH "/tmp/txrx_bytes"
	FILE *fp1, *fp2;
	char wlif_name[32], line_buf[300];
	char tmp[64], *ptr;
	int cur_txrx_bytes, channf;
	WLANCONFIG_LIST *result;
	rast_sta_info_t *sta = NULL;

	__get_wlifname(swap_5g_band(bssidx), vifidx, wlif_name);

	result = malloc(sizeof(WLANCONFIG_LIST));
	channf = QCA_DEFAULT_NOISE_FLOOR;

	/* get MAC and RSSI of station */
	doSystem("wlanconfig %s list > %s", wlif_name, STA_LOW_RSSI_PATH);
	fp1 = fopen(STA_LOW_RSSI_PATH, "r");
	if (fp1) {
		int has_minrssi = 0;
		int has_rxnss = 0;
		char *pAcaps;
		char *pMode;

		fgets(line_buf, sizeof(line_buf), fp1); // ignore header
		if(strstr(line_buf, "MINRSSI") != NULL)
			has_minrssi = 1;
		if(strstr(line_buf, "RXNSS") != NULL)
			has_rxnss = 1;
		pAcaps = strstr(line_buf, "ACAPS");

		while (fgets(line_buf, sizeof(line_buf), fp1)) {
			if(pAcaps == NULL)
				break;

			if((pMode = strstr(line_buf, "IEEE80211_MODE")) == NULL)
				continue;

			*(pMode-1) = '\0';	//cut for IEs
			*(pAcaps-1) = '\0';	//cut for acaps could be ""

			memset(result, 0, sizeof(WLANCONFIG_LIST));
			/* get until acaps */
			if(has_minrssi) {
				sscanf(line_buf, "%s%u%u%s%s %u%u%u%u%u%u %s%s",
						result->addr,
						&result->aid,
						&result->chan,
						result->txrate,
						result->rxrate,
						&result->rssi,		//u
						&result->rssi_min,
						&result->rssi_max,
						&result->idle,
						&result->txseq,
						&result->rxseq,
						result->caps,		//s
						result->acaps);
			}
			else {
				sscanf(line_buf, "%s%u%u%s%s %u%u%u%u %s%s",
						result->addr,
						&result->aid,
						&result->chan,
						result->txrate,
						result->rxrate,
						&result->rssi,		//u
						&result->idle,
						&result->txseq,
						&result->rxseq,
						result->caps,		//s
						result->acaps);
			}
			/* get after ACAPS */
			sscanf(pAcaps, "%s%s %u%s%s %[^\n]",
						result->erp,
						result->state,
						&result->maxrate,	//u
						result->htcaps,
						result->assoctime,
						result->Ies);		//s
			/* get remaining */
			if (has_rxnss) {
				sscanf(pMode, "%s %u%u%u",
						result->mode,
						&result->psmode,	//u
						&result->rxnss,
						&result->txnss);
			}
			else {
				sscanf(pMode, "%s %u",
						result->mode,
						&result->psmode);	//u
			}

			RAST_DBG("[%s][%u][%u][%s][%s] [%u][%u][%u][%u][%u][%u] [%s][%s][%s][%s] [%u][%s][%s] [%s][%s] [%u][%u][%u]\n",
						result->addr,		//s
						result->aid,
						result->chan,
						result->txrate,
						result->rxrate,
						result->rssi,		//u
						result->rssi_min,
						result->rssi_max,
						result->idle,
						result->txseq,
						result->rxseq,
						result->caps,		//s
						result->acaps,
						result->erp,
						result->state,
						result->maxrate,	//u
						result->htcaps,
						result->assoctime,
						result->Ies,		//s
						result->mode,
						result->psmode,		//u
						result->rxnss,
						result->txnss);
			/* add to assoclist */
			sta = rast_add_to_assoclist(bssidx, vifidx, ether_aton(result->addr));
			sta->rssi = result->rssi + channf;
			sta->active = uptime();
			/* get Tx/Rx bytes of station */
			doSystem("80211stats -i %s %s > %s", wlif_name, result->addr, STA_TXRX_BYTES_PATH);
			fp2 = fopen(STA_TXRX_BYTES_PATH, "r");
			if (fp2) {
				cur_txrx_bytes = 0;
				while (fgets(tmp, sizeof(tmp), fp2)) {
					if ((ptr = strstr(tmp, "rx_bytes "))
							|| (ptr = strstr(tmp, "tx_bytes "))) {
						ptr += strlen("rx_bytes ");
						cur_txrx_bytes += atoi(ptr);
					}
				}
				sta->datarate = (float)((cur_txrx_bytes - sta->last_txrx_bytes) >> 7/* bytes to Kbits*/) / RAST_POLL_INTV_NORMAL/* Kbps */;
				sta->last_txrx_bytes = cur_txrx_bytes;
				fclose(fp2);
				unlink(STA_TXRX_BYTES_PATH);
			}
		}
		fclose(fp1);
		unlink(STA_LOW_RSSI_PATH);
	}
	free(result);
}

#ifdef RTCONFIG_ADV_RAST
/*
 * rast_stamon_get_rssi(bssidx, addr)
 *
 * to get the RSSI value of an Non-Associated Client assigned by addr.
 *
 * In QCA platform, the bssid of the AP that the client connecting is required.
 * And is got by "nac_bssid" variable that set in caller.
 *
 * The return value should be a negative value in dBm or a 0 meaning NO data.
 *
 */
int rast_stamon_get_rssi(int bssidx, struct ether_addr *addr)
{
#if defined(RTCONFIG_QCA953X) || defined(RTCONFIG_SOC_QCA9557) || defined(RTCONFIG_QCA956X)
	return 0;
#else
	char athfix[8];
	int ch;
	int rssi = 0;
	FILE *fp;
	char line[256];
	int size;
	char mac[18];
	char *bssid;
	char tmp_bssid[32], tmp_client[32];
	int tmp_ch, tmp_rssi;
	int i;
	int channf;

	bssid = nvram_get("nac_bssid");

	size = sizeof(line)-1;
	line[size] = '\0';
	strcpy(mac, ether_ntoa(addr));

	if(bssid == NULL || strlen(bssid) != 17 || strcmp(bssid, "00:00:00:00:00:00") == 0)
		return 0;

	__get_wlifname(swap_5g_band(bssidx), 0, athfix);
	ch = get_channel(athfix);
	channf = QCA_DEFAULT_NOISE_FLOOR;

	doSystem("wlanconfig %s rssi_nac add bssid %s client %s channel %d", athfix, bssid, mac, ch);
	sleep(1);
	for(i = 0; i < 20 && rssi == 0; i++) {
		usleep(500*1000);
		snprintf(line, size, "wlanconfig %s rssi_nac show_rssi", athfix);
		if((fp = popen(line, "r")) != NULL) {
			int init = 0;
			int len;
			while(fgets(line, size, fp)) {
				strip_new_line(line);
				len = strlen(line);
				if (len > 58) {	/* 10 + 17 + 2 + 17 + 3 + ch + 8 + rssi = 58~60 + rssi */
					if(init == 0)
						init = len;
					else {
						sscanf(line, "%s %s %d %d", tmp_bssid, tmp_client, &tmp_ch, &tmp_rssi);
						if(tmp_rssi > 0)
							rssi = tmp_rssi + channf;
						break;
					}
				}
			}
			pclose(fp);
		}
	}
	doSystem("wlanconfig %s rssi_nac del bssid %s client %s", athfix, bssid, mac);
	return rssi;
#endif
}

#if 1
struct maclist {
	uint count;
	struct ether_addr ea[0];
};
#endif

/* 
 * rast_retrieve_static_maclist(bssidx, vifidx)
 *
 * getting the mac filter mode and mac list that has set in wifi interface(bssidx,vifidx).
 * and save to the struct in bssinfo[bssidx].static_maclist[vifidx].
 *
 * The list may include the mac filter the set in web UI and the RE list (when in allow mode).
 * The saved list is used in rast_set_maclist().
 * And tread as a basic list to handle add/del in bssinfo[bssidx].maclist[vifidx].
 *
 */
void rast_retrieve_static_maclist(int bssidx, int vifidx)
{
	int size;
	char wlif_name[64];
	char athfix[8];
	char cmd[128];
	char line[256];
	FILE *fp;
	int len;
	char *p;
	const char *get_maccmd = "get_maccmd";
	const char *getmac = "getmac";
	struct maclist *static_maclist;


	if(vifidx > 0)
		snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
	else
		snprintf(wlif_name, sizeof(wlif_name), "%s", bssinfo[bssidx].wlif_name);

	__get_wlifname(bssidx, vifidx, athfix);

	size = sizeof(line)-1;
	line[size] = '\0';

	/* get current mac filter mode */
	snprintf(cmd, sizeof(cmd), "iwpriv %s %s", athfix, get_maccmd);
	if((fp = popen(cmd, "r")) == NULL)
		return;

	if(fgets(line, size, fp) != NULL && (p = strchr(line, ':')) != NULL) {
		bssinfo[bssidx].static_macmode[vifidx] = atoi(p+1);	/* 0: disable, 1: allow, 2: deny */
	}
	pclose(fp);

	/* get current mac filter list */
	snprintf(cmd, sizeof(cmd), "iwpriv %s %s", athfix, getmac);
	if((fp = popen(cmd, "r")) == NULL)
		return;

	if(bssinfo[bssidx].static_maclist[vifidx] == NULL) {
		int msize = sizeof(uint) + sizeof(struct ether_addr) * 128;
		if((bssinfo[bssidx].static_maclist[vifidx] = (struct maclist *)malloc(msize)) == NULL) {
			pclose(fp);
			return;
		}
	}

	static_maclist = bssinfo[bssidx].static_maclist[vifidx];
	static_maclist->count = 0;
	p = NULL;
	while(fgets(line, size, fp)) {
		if(p == NULL) {
			p = strchr(line, ':');
			if(p != NULL) {
				p += 1;	//point to the head of mac address
			}
		}
		if(p != NULL && (len = strlen(line)) >= (p - line) + 17) {
			struct ether_addr *addr;
			*(p + 17) = '\0';		//cut the string of mac address
			if((addr = ether_aton(p)) != NULL) {
				memcpy((void*)&static_maclist->ea[static_maclist->count], (void*)addr, 6);
				static_maclist->count++;
			}
		}
		else
		{
			_dprintf("## invalid line (%s)\n", line);
		}
	}
	pclose(fp);
}

/*
 * rast_set_maclist(bssidx, vifidx)
 *
 * use the basic maclist in bssinfo[bssidx].static_maclist[vifidx]
 * And handle the add/del maclist in bssinfo[bssidx].maclist[vifidx].
 *
 * The r_maclist_old_table[bssidx][vifidx] is used to check which mac is add/del in bssinfo[bssidx].maclist[vifidx].
 *
 * Since the QCA platform can add/del alone, without all list as Broadcom platform.
 * If the rast_set_maclist_add() and rast_set_maclist_del() are used in caller of roamast.c, the functions are easier to be understood.
 *
 */
void rast_set_maclist(int bssidx, int vifidx)
{
	char wlif_name[64];
	rast_maclist_t *r_maclist = bssinfo[bssidx].maclist[vifidx];
	rast_maclist_t *r_maclist_old = r_maclist_old_table[bssidx][vifidx];
	struct maclist *static_maclist = bssinfo[bssidx].static_maclist[vifidx];
	int static_macmode = bssinfo[bssidx].static_macmode[vifidx];
	int ret, val;
	int cnt, match, exist;
	char athfix[8];
	char *p;
	rast_maclist_t *r_maclist_tmp;

	if(vifidx > 0)
		snprintf(wlif_name, sizeof(wlif_name), "wl%d.%d", bssidx, vifidx);
	else
		snprintf(wlif_name, sizeof(wlif_name), "%s", bssinfo[bssidx].wlif_name);

	if (static_macmode == WLC_MACMODE_DENY || static_macmode == WLC_MACMODE_DISABLED)
		val = WLC_MACMODE_DENY;
	else
		val = WLC_MACMODE_ALLOW;

	__get_wlifname(swap_5g_band(bssidx), vifidx, athfix);
	ret = doSystem("iwpriv %s %s %d", athfix, QCA_MACCMD, val);
	if(ret < 0) {
		RAST_INFO("[WARNING] %s %s error!!!\n", athfix, QCA_MACCMD);
		return;
	}

	/* handle del in maclist */
	while(r_maclist_old)
	{
		match = 0;
		exist = 0;
		r_maclist = bssinfo[bssidx].maclist[vifidx];
		while(r_maclist) {
			if ( memcmp( &(r_maclist->addr), &(r_maclist_old->addr) ,sizeof(struct ether_addr)) == 0) {
				match = 1;
				break;
			}
			r_maclist = r_maclist->next;
		}
		if(!match) {
			if(static_maclist) {
				for (cnt = 0; cnt < static_maclist->count; cnt++) {
					if (memcmp(&(r_maclist_old->addr), &(static_maclist->ea[cnt]),sizeof(struct ether_addr)) == 0) {
						exist = 1;
						break;
					}
				}
			}
			/* action for remove */
			p = ether_ntoa(&(r_maclist_old->addr));
			if (static_macmode == WLC_MACMODE_DISABLED || static_macmode == WLC_MACMODE_DENY) {
				if(!exist)
					set_maclist_del_kick(athfix, 2, p);
			}
			else {
				if(exist)
					set_maclist_add_kick(athfix, 1, p);	/* add back */
			}
			/* release current r_maclist_old */
			if(r_maclist_old == r_maclist_old_table[bssidx][vifidx])
				r_maclist_old_table[bssidx][vifidx] = r_maclist_old->next;
			r_maclist_tmp = r_maclist_old;
			r_maclist_old = r_maclist_old->next;
			free(r_maclist_tmp);
			continue;
		}
		r_maclist_old = r_maclist_old->next;
	}
	/* handle add in maclist */
	r_maclist = bssinfo[bssidx].maclist[vifidx];
	while (r_maclist) {
		match = 0;
		exist = 0;
		r_maclist_old = r_maclist_old_table[bssidx][vifidx];
		while(r_maclist_old) {
			if (memcmp( &(r_maclist->addr), &(r_maclist_old->addr) ,sizeof(struct ether_addr)) == 0) {
				match = 1;
				break;
			}
			r_maclist_old = r_maclist_old->next;
		}
		if(!match) {
			if( static_maclist ) {
				for (cnt = 0; cnt < static_maclist->count; cnt++) {
					RAST_DBG("Checking "MACF"\n", ETHER_TO_MACF(r_maclist->addr));
					if (memcmp(&(r_maclist->addr), &(static_maclist->ea[cnt]),sizeof(struct ether_addr)) == 0) {
						RAST_DBG("MATCH maclist "MACF"\n", ETHER_TO_MACF(r_maclist->addr));
						exist = 1;
						break;
					}
				}
			}
			/* action for new */
			p = ether_ntoa(&(r_maclist_old->addr));
			if (static_macmode == WLC_MACMODE_DISABLED || static_macmode == WLC_MACMODE_DENY) {
				if(!exist)
					set_maclist_add_kick(athfix, 2, p);	/* add to deny */
			}
			else {
				if(exist)
					set_maclist_del_kick(athfix, 1, p);	/* remove for block */
			}
			/* add to list */
			r_maclist_tmp = calloc(1,sizeof(rast_maclist_t));
			memcpy(&r_maclist_tmp->addr, &r_maclist->addr, sizeof(struct ether_addr));
			r_maclist_tmp->next = r_maclist_old_table[bssidx][vifidx];
			r_maclist_old_table[bssidx][vifidx] = r_maclist_tmp;
		}
		r_maclist = r_maclist->next;
	}
}

#if 0
uint8 rast_get_rclass(int bssidx, int vifidx)
{

}

uint8 rast_get_channel(int bssidx, int vifidx)
{

}

int rast_send_bsstrans_req(int bssidx, int vifidx, struct ether_addr *sta_addr, struct ether_addr *nbr_bssid)
{

}
#endif
#endif
