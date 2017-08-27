#include <sys/types.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <stdio.h>

#include "../../../../../linux/realtek/rtl819x/linux-3.10/drivers/net/wireless/rtl8192cd/ieee802_mib.h"

#define MAX_WLAN_MIB_NUM  12
#define MAX_IFNAME_LEN 16

static int __isDrvMibInit__ = 0;

static int _curWlanIndex = -1;
static struct wifi_mib* _pCurWlanMib = NULL;
static struct wifi_mib* _wlanMibs[MAX_WLAN_MIB_NUM] = {NULL};

#ifdef RTK_WIFI_IFNAME
static const char* _wlanIfnames[MAX_WLAN_MIB_NUM] = { "wlan0", "wlan0-va0", "wlan0-va1", "wlan0-va2", "wlan0-va3", "wlan0-vxd", 
													  "wlan1", "wlan1-va0", "wlan1-va1", "wlan1-va2", "wlan1-va3", "wlan1-vxd"};
#else
static const char* _wlanIfnames[MAX_WLAN_MIB_NUM] = { "wl0", "wl0.0", "wl0.1", "wl0.2", "wl0.3", "wl0-vxd", 
													  "wl1", "wl1.0", "wl1.1", "wl1.2", "wl1.3", "wl1-vxd"};
#endif

static int _sock = -1;
static struct iwreq* _pWrq = NULL;

void drvmib_release_rtkapi()
{
	int i;
   	for(i=0; i<MAX_WLAN_MIB_NUM; i++){
		free(_wlanMibs[i]);
		_wlanMibs[i] = NULL;
	}
	_pCurWlanMib = NULL;
	_curWlanIndex = -1;

	close(_sock);
	_sock = -1;
	free(_pWrq);
	_pWrq = NULL;
}

int drvmib_init_rtkapi()
{
	int i;
	for(i=0; i<MAX_WLAN_MIB_NUM; i++){
		_wlanMibs[i] = NULL;
	}
	__isDrvMibInit__ = 1;
    return i;
}

static int wlan_driver_ioctl(int index, int cmd, struct wifi_mib* para)
{
	if( _sock<0 ){
		_sock = socket(AF_INET, SOCK_DGRAM, 0);
		if( _sock<0 ){
			return -1; // sock err
		}
		_pWrq  =  (struct iwreq*)malloc(sizeof(struct iwreq));
		if( _pWrq==NULL ){
			close(_sock);
			return -1; //no free mem
		}
		memset(_pWrq, 0, sizeof(struct iwreq));
		_pWrq->u.data.pointer = (caddr_t) para;
		_pWrq->u.data.length = sizeof(struct wifi_mib);
	}
	memset(_pWrq, 0, sizeof(struct iwreq));
	_pWrq->u.data.pointer = (caddr_t) para;
	_pWrq->u.data.length = sizeof(struct wifi_mib);	
	strncpy(_pWrq->ifr_name, _wlanIfnames[index], MAX_IFNAME_LEN);
	if( ioctl(_sock, cmd, _pWrq) < 0 ){
		return -1;
	}
	return 0;
}

#define drvmib_get_all(index, pmib) wlan_driver_ioctl(index, 0x8B42, pmib)
#define drvmib_set_all(index, pmib) wlan_driver_ioctl(index, 0x8B43, pmib)

struct wifi_mib* drvmib_get_pmib(char* ifname)
{
	struct wifi_mib* pmib;
	int i;
   	for(i=0; i<MAX_WLAN_MIB_NUM; i++){
		if( strncmp(ifname, _wlanIfnames[i], MAX_IFNAME_LEN)==0 ){
			_curWlanIndex = i;
			break;
		}
	}
	if(i==MAX_IFNAME_LEN){
		return NULL; // ifname not exist
	}
	
	pmib = _wlanMibs[_curWlanIndex];
	if( pmib==NULL ){
		pmib = (struct wifi_mib*)malloc(sizeof(struct wifi_mib));
		if(pmib==NULL){
			return NULL; // no free mem
		}
		if( drvmib_get_all(_curWlanIndex, pmib)<0 ){
			free(pmib);
			return NULL; // ioctl err
		}
		_wlanMibs[_curWlanIndex]  =  pmib;
	}
	return pmib;
}

static int _drvmib_apply_one(int index)
{
	struct wifi_mib* pmib = _wlanMibs[index];
	int opmode;

	opmode = pmib->dot11OperationEntry.opmode;
	pmib->dot11StationConfigEntry.sc_enabled = opmode==0x08 || opmode==0x20;
	if(opmode==0x1000){
		pmib->dot11WdsInfo.wdsPure = 1;
	}else{
		pmib->dot11WdsInfo.wdsPure = 0;
	}
	
	if ((pmib->dot11StationConfigEntry.dot11DesiredSSIDLen == 3) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'A') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'a')) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'N') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'n')) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'Y') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'y'))) {
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = 0;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
	} else {
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = pmib->dot11StationConfigEntry.dot11DesiredSSIDLen;;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
		memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan, pmib->dot11StationConfigEntry.dot11DesiredSSID, pmib->dot11StationConfigEntry.dot11DesiredSSIDLen);
	}

	int band = pmib->dot11BssType.net_work_type;
	int legacy = 0;
	if( band==2 && !pmib->dot11StationConfigEntry.sc_enabled && pmib->dot11OperationEntry.wifi_specific ){
		band = 3;
	}
	if( band==8 ){
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
		if( pmib->dot11RFEntry.phyBandSelect==2 ){
			band += 4;
			legacy = 4;
		}else if( pmib->dot11RFEntry.phyBandSelect==1 )
#endif
			{
				band += 3;
				legacy =3;
			}
	}else if( band==2 ){
		band += 1;
		legacy = 1;
	}else if( band==10 ){
		band += 1;
		legacy = 1;		
	}else if( band==64 ){
		band += 12;
		legacy = 12;		
	}else if( band==72 ){
		band += 4;
		legacy = 4;
	}
	pmib->dot11StationConfigEntry.legacySTADeny = legacy;
	pmib->dot11BssType.net_work_type = band;

	if(pmib->dot11StationConfigEntry.fixedTxRate==0){
		pmib->dot11StationConfigEntry.autoRate = 1;
	}else{
		pmib->dot11StationConfigEntry.autoRate = 0;
	}

	if( pmib->gbwcEntry.GBWCThrd_rx==0 ){
		if( pmib->gbwcEntry.GBWCThrd_tx==0 ){
			pmib->gbwcEntry.GBWCMode = 0;
		}else{
			pmib->gbwcEntry.GBWCMode = 3;
		}
	}else{
		if( pmib->gbwcEntry.GBWCThrd_tx==0 ){
			pmib->gbwcEntry.GBWCMode = 4;
		}else{
			pmib->gbwcEntry.GBWCMode = 5;
		}
	}

	if( pmib->dot11WdsInfo.wdsEnabled ){
		char cmd[128];
		int i;
		for(i=0;i<pmib->dot11WdsInfo.wdsNum; i++){
			unsigned char* mac = &(pmib->dot11WdsInfo.entry[i].macAddr[0]);
			sprintf(cmd, "ifconfig %s-wds%d hw ether %02x%02x%02x%02x%02x%02x", _wlanIfnames[index], i, (int)mac[0], (int)mac[1], (int)mac[2], (int)mac[3], (int)mac[4], (int)mac[5]);
			doSystem(cmd);
		}

	}

	if( index>0 && index<MAX_WLAN_MIB_NUM/2 ){
		struct wifi_mib* root_pmib;
		root_pmib = drvmib_get_pmib(_wlanIfnames[0]);
		if(root_pmib)
			memcpy(&pmib->dot11RFEntry, &root_pmib->dot11RFEntry, sizeof(pmib->dot11RFEntry));
	}else if( index>MAX_WLAN_MIB_NUM/2 ){
		struct wifi_mib* root_pmib;
		root_pmib = drvmib_get_pmib(_wlanIfnames[MAX_WLAN_MIB_NUM/2]);
		if(root_pmib)
			memcpy(&pmib->dot11RFEntry, &root_pmib->dot11RFEntry, sizeof(pmib->dot11RFEntry));	
	}

	if( drvmib_set_all(index, pmib)<0 ){
		return -1;
	}
	return 0;
}

int drvmib_apply_rtkapi()
{
	int i;
	int count = 0;
   	for(i=0; i<MAX_WLAN_MIB_NUM; i++){
		if( _wlanMibs[i] != NULL){
			if( _drvmib_apply_one(i)<0 ){
				printf("WRANING:if %s drvmib apply failed!\n", _wlanIfnames[i]);
			}
			count++;
		}
	}
	return count;
}
