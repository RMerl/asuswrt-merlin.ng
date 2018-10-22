#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>
#include <wlutils.h>
#include "utils.h"
#include "shutils.h"
#include "shared.h"

#ifdef RTCONFIG_AMAS
#include <net/ethernet.h>
#include <amas_path.h>
#endif

#ifdef LINUX26
#define GPIO_IOCTL
#endif

//in rc/sysdeps/lantiq/lantiq_common.h
#define VAP_2G_START 5
#define VAP_5G_START 8

// --- move begin ---
#ifdef GPIO_IOCTL

#include <sys/ioctl.h>
#include <linux_gpio.h>

static int _gpio_ioctl(int f, int gpioreg, unsigned int mask, unsigned int val)
{

}

static int _gpio_open()
{

}

int gpio_open(uint32_t mask)
{

}

void gpio_write(uint32_t bitvalue, int en)
{

}

uint32_t _gpio_read(int f)
{

}

uint32_t gpio_read(void)
{

}

#else

int gpio_open(uint32_t mask)
{

}

void gpio_write(uint32_t bitvalue, int en)
{

}

uint32_t _gpio_read(int f)
{

}

uint32_t gpio_read(void)
{

}

#endif

#ifdef RTCONFIG_AMAS 
char *get_pap_bssid(int unit, char bssid_str[])
{
	char buf[8192];
	FILE *fp;
	int len;
	char *pt1, *pt2;

	memset(bssid_str, 0, 18);

	snprintf(buf, sizeof(buf), "iwconfig %s", get_staifname(unit));
	fp = popen(buf, "r");
	if(fp){
		memset(buf, 0, sizeof(buf));
		len = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if(len > 1){
			buf[len-1] = '\0';
			pt1 = strstr(buf, "Access Point:");
			if(pt1){
				pt2 = pt1 + strlen("Access Point: ");
				pt1 = strstr(pt2, "Not-Associated");
				if(!pt1) 
				{
					strncpy(bssid_str,pt2,17);
				}
			}
		}
	}

	//_dprintf("[get_pap_bssid in shared]%s:[%s]\n",get_staifname(unit),bssid_str);

	return bssid_str;

}

//int get_maxassoc(char *ifname)
//{
#if 0
	FILE *fp = NULL;
	char maxassoc_file[128]={0};
	char buf[64]={0};
	char maxassoc[64]={0};

	snprintf(maxassoc_file, sizeof(maxassoc_file), "/tmp/maxassoc.%s", ifname);

	doSystem("wl -i %s maxassoc > %s", ifname, maxassoc_file);

	if ((fp = fopen(maxassoc_file, "r")) != NULL) {
		fscanf(fp, "%s", buf);
		fclose(fp);
	}
	sscanf(buf, "%s", maxassoc);

	return atoi(maxassoc);
#endif
//}

void set_wl_macfilter(int bssidx, int vifidx, struct maclist_r *client_list, int block)
{
	_dprintf("shared %s %d\n",__FUNCTION__,__LINE__);
return ;
}

int get_psta_status(int unit)
{
	char buf[8192];
	FILE *fp;
	int len;
	char *pt1, *pt2;

	snprintf(buf, sizeof(buf), "iwconfig %s", get_staifname(unit));
	fp = popen(buf, "r");
	if(fp){
		memset(buf, 0, sizeof(buf));
		len = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if(len > 1){
			buf[len-1] = '\0';
			pt1 = strstr(buf, "Access Point:");
			if(pt1){
				pt2 = pt1 + strlen("Access Point:");
				pt1 = strstr(pt2, "Not-Associated");
				if(pt1) 
				{
					snprintf(buf, sizeof(buf), "ifconfig | grep %s", get_staifname(unit));
					fp = popen(buf, "r");
					if(fp)
					{
						memset(buf, 0, sizeof(buf));
						len = fread(buf, 1, sizeof(buf), fp);
						pclose(fp);
						if(len>=1)
							return 0;
						else
							return 3;
					}
					else
						return 0; //init
				}
				else
					return 2; //connect and auth ?????
			}
		}
	}

	return 3; // stop
}

void add_beacon_vsie(char *hexdata)
{
	// 0: Beacon
	// 1: ProbeRequest
	// 2: ProbeResponse
	// 3: AuthenticationRequest
	// 4: AuthenticationRespnse
	// 5: AssocationRequest
	// 6: AssociationResponse
	// 7: ReassociationRequest
	// 8: ReassociationResponse
#if 0
	char cmd[300] = {0};
	int pktflag = 0x0;
	int len = 0;
	char *ifname = NULL;
	strlen(ifname);

	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	ifname = get_wififname(0); // TODO: Should we get the band from nvram?
	
	//_dprintf("%s: wl0_ifname=%s\n", __func__, ifname);

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli set_vsie -i%s %d DD%02X%02X%02X%02X%s", 
			ifname, pktflag, (uint8_t)len, (uint8_t)OUI_ASUS[0],  (uint8_t)OUI_ASUS[1],  (uint8_t)OUI_ASUS[2], hexdata);
		_dprintf("%s: cmd=%s\n", __func__, cmd);
		system(cmd);
	}
#endif
	nvram_set("amas_add_beacon_vsie", hexdata);
	trigger_wave_monitor_and_wait(__func__, __LINE__, WAVE_ACTION_ADD_BEACON_VSIE, 1);
}

void del_beacon_vsie(char *hexdata)
{
	// 0: Beacon
	// 1: ProbeRequest
	// 2: ProbeResponse
	// 3: AuthenticationRequest
	// 4: AuthenticationRespnse
	// 5: AssocationRequest
	// 6: AssociationResponse
	// 7: ReassociationRequest
	// 8: ReassociationResponse
#if 0
	char cmd[300] = {0};
	int pktflag = 0x0;
	int len = 0;
	char *ifname = NULL;

	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	ifname = get_wififname(0); // TODO: Should we get the band from nvram?

	//_dprintf("%s: wl0_ifname=%s\n", __func__, ifname);

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli del_vsie -i%s %d DD%02X%02X%02X%02X%s",
			ifname, pktflag, (uint8_t)len,  (uint8_t)OUI_ASUS[0],  (uint8_t)OUI_ASUS[1],  (uint8_t)OUI_ASUS[2], hexdata);
		_dprintf("%s: cmd=%s\n", __func__, cmd);
		system(cmd);
	}
#endif
	nvram_set("amas_del_beacon_vsie", hexdata);
	trigger_wave_monitor_and_wait(__func__, __LINE__, WAVE_ACTION_DEL_BEACON_VSIE, 1);
}
#endif /* RTCONFIG_AMAS */





