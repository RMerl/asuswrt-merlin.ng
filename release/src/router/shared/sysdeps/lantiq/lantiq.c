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
static bool g_swap = FALSE;
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
char *get_pap_bssid(int unit)
{
#if 0
	unsigned char bssid[6] = {0};
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name;
	static char bssid_str[sizeof("00:00:00:00:00:00XXX")];

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (wl_ioctl(name, WLC_GET_BSSID, bssid, sizeof(bssid)) == 0) {
		if ( !(!bssid[0] && !bssid[1] && !bssid[2] && !bssid[3] && !bssid[4] && !bssid[5]) ) {
			snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", 
				(unsigned char)bssid[0], (unsigned char)bssid[1],
				(unsigned char)bssid[2], (unsigned char)bssid[3],
				(unsigned char)bssid[4], (unsigned char)bssid[5]);
		}
	}

	return bssid_str;
#endif	
}

//sta_info_t *wl_sta_info(char *ifname, struct ether_addr *ea)
//{
#if 0
	static char buf[sizeof(sta_info_t)];
	sta_info_t *sta = NULL;

	strcpy(buf, "sta_info");
	memcpy(buf + strlen(buf) + 1, (void *)ea, ETHER_ADDR_LEN);

	if (!wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf))) {
		sta = (sta_info_t *)buf;
		sta->ver = dtoh16(sta->ver);

		/* Report unrecognized version */
		if (sta->ver > WL_STA_VER) {
			dbg(" ERROR: unknown driver station info version %d\n", sta->ver);
			return NULL;
		}

		sta->len = dtoh16(sta->len);
		sta->cap = dtoh16(sta->cap);
#ifdef RTCONFIG_BCMARM
		sta->aid = dtoh16(sta->aid);
#endif
		sta->flags = dtoh32(sta->flags);
		sta->idle = dtoh32(sta->idle);
		sta->rateset.count = dtoh32(sta->rateset.count);
		sta->in = dtoh32(sta->in);
		sta->listen_interval_inms = dtoh32(sta->listen_interval_inms);
#ifdef RTCONFIG_BCMARM
		sta->ht_capabilities = dtoh16(sta->ht_capabilities);
		sta->vht_flags = dtoh16(sta->vht_flags);
#endif
	}

	return sta;

#endif
//}

int get_wlan_service_status(int bssidx, int vifidx)
{
#if 0
	FILE *fp = NULL;
	char maxassoc_file[128]={0};
	char buf[64]={0};
	char maxassoc[64]={0};
	char tmp[128] = {0}, prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;

	if(vifidx > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", bssidx, vifidx);
	else
		snprintf(prefix, sizeof(prefix), "wl%d", bssidx);

	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));


	snprintf(maxassoc_file, sizeof(maxassoc_file), "/tmp/maxassoc.%s", ifname);

	doSystem("wl -i %s maxassoc > %s", ifname, maxassoc_file);

	if ((fp = fopen(maxassoc_file, "r")) != NULL) {
		fscanf(fp, "%s", buf);
		fclose(fp);
	}
	sscanf(buf, "%s", maxassoc);

	return atoi(maxassoc);
#endif
	return 0;
}

int set_wlan_service_status(int bssidx, int vifidx, int enabled)
{

#if 0
	char tmp[128]={0}, prefix[] = "wlXXXXXXXXXX_", wlprefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;

	if(vifidx > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", bssidx, vifidx);
	else
		snprintf(prefix, sizeof(prefix), "wl%d", bssidx);

	memset(wlprefix, 0x00, sizeof(wlprefix));
	snprintf(wlprefix, sizeof(wlprefix), "wl%d_", bssidx);

	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (enabled == 0) {
		doSystem("wl -i %s maxassoc 0", ifname);
	}
	else {
#ifdef HND_ROUTER
		doSystem("wl -i %s maxassoc %d", ifname, nvram_get_int(strcat_r(wlprefix, "cfg_maxassoc", tmp)));
#else
		doSystem("wl -i %s maxassoc %d", ifname, nvram_get_int(strcat_r(wlprefix, "maxassoc", tmp)));
#endif
	}
#endif
	return 0;
}

int get_maxassoc(char *ifname)
{
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
}

void set_wl_macfilter_test(int bssidx, int vifidx, struct maclist_r *client_list, int block)
{
#if 0
		int ret = 0, size = 0, i = 0, cnt = 0, match = 0;
		char *wlif_name = NULL;
		char tmp[128], prefix[] = "wlXXXXXXXXXX_";
		struct ether_addr *ea = NULL;

		int macmode = 0;
		char maclist_buf[WLC_IOCTL_MAXLEN] = {0};
		struct maclist *maclist = (struct maclist *) maclist_buf;
		struct maclist *static_maclist= NULL;

		if(vifidx > 0)
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", bssidx, vifidx);
		else
			snprintf(prefix, sizeof(prefix), "wl%d", bssidx);

		wlif_name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));


		ret = wl_ioctl(wlif_name, WLC_GET_MACMODE, &macmode, sizeof(macmode));
		if(ret < 0) 
		{
			dbG("[WARNING] %s get macmode error!!!\n", wlif_name);
			return;
		}

		cprintf("[%s] macmode = %s\n",
		wlif_name,
		macmode==WLC_MACMODE_DISABLED ? "DISABLE" :
		macmode==WLC_MACMODE_DENY ? "DENY" : "ALLOW");

		ret = wl_ioctl(wlif_name, WLC_GET_MACLIST, (void *)maclist, sizeof(maclist_buf));
		if(ret < 0) 
		{
			cprintf("[WARNING] %s get maclist error!!!\n", wlif_name);
			return;
		}

		if (maclist->count > 0 && maclist->count < 128) 
		{
			size = sizeof(uint) + sizeof(struct ether_addr) * (maclist->count);

			static_maclist = (struct maclist *)malloc(size);

			if (!(static_maclist)) 
			{
				cprintf("%s malloc [%d] failure... \n", __FUNCTION__, size);
				return;
			}

			memcpy(static_maclist, maclist, size);
			static_maclist->count = maclist->count;
			memset(maclist, 0x00, sizeof(maclist_buf));
			//maclist = static_maclist;

			for (size = 0; size < maclist->count; size++) {
				cprintf("%d [%s] (%d)mac:"MACF"\n",__LINE__, wlif_name, size, ETHER_TO_MACF(maclist->ea[size]));
			}	

		} 
		else if (maclist->count != 0) 
		{
			cprintf("Err: %s maclist cnt [%d] too large\n",
			wlif_name, maclist->count);
			return;
		}
		else if (maclist->count == 0) {
			cprintf("maclist is empty\n");
		}

		if (block == 0 && macmode == WLC_MACMODE_DISABLED)
			macmode = WLC_MACMODE_DISABLED;
		else if (macmode == WLC_MACMODE_DENY || macmode == WLC_MACMODE_DISABLED)
			macmode = WLC_MACMODE_DENY;
		else
			macmode = WLC_MACMODE_ALLOW;


		if (macmode == WLC_MACMODE_DENY || macmode == WLC_MACMODE_DISABLED) 
		{
			if (block == 1) 
			{              	
				if (static_maclist) 
				{
					cprintf("Deny mode: Adding static maclist\n");
					maclist->count = static_maclist->count;
					memcpy(maclist_buf, static_maclist,
					sizeof(uint) + ETHER_ADDR_LEN * (maclist->count));

					ea = &(maclist->ea[maclist->count]);

					for (cnt = 0; cnt < client_list->count; cnt++) {
						cprintf("Deny mode: static mac[%d] addr:"MACF"\n", cnt,ETHER_TO_MACF(client_list->ea[cnt]));
						match = 0;
						for  (i = 0; i < static_maclist->count ; i++) {
							cprintf("Checking "MACF"\n", ETHER_TO_MACF(static_maclist->ea[i]));
							if (memcmp(&(client_list->ea[cnt]), &(static_maclist->ea[i]), sizeof(struct ether_addr)) == 0) {
								cprintf("MATCH maclist "MACF"\n", ETHER_TO_MACF(client_list->ea[cnt]));
								match = 1;
								break;
							}
						}
						if (!match) {
							memcpy(ea, &(client_list->ea[cnt]), sizeof(struct ether_addr));
							maclist->count++;
							cprintf("Adding to Deny list: cnt[%d] addr:"MACF"\n",maclist->count, ETHERP_TO_MACF(ea));
							ea++;
						}
					}
				}
				else {
					if (client_list->count > 0) {
					maclist->count = client_list->count;
					memcpy(maclist, client_list, sizeof(uint) + ETHER_ADDR_LEN * (client_list->count));	                			                	
					}
				}
			}
			else if (block == 0) 
			{

				ea = &(maclist->ea[0]);
				maclist->count = 0;
				if (static_maclist) 
				{

					for (cnt = 0; cnt < static_maclist->count; cnt++) {
						cprintf("%d:Deny mode: static mac[%d] addr:"MACF"\n",__LINE__, cnt,
						ETHER_TO_MACF(static_maclist->ea[cnt]));
						match = 0;
						for  (i = 0; i < client_list->count ; i++) {
							cprintf("Checking "MACF"\n", ETHER_TO_MACF(client_list->ea[i]));
							if (memcmp(&(client_list->ea[i]), &(static_maclist->ea[cnt]),  sizeof(struct ether_addr)) == 0) 
							{
								cprintf("MATCH maclist "MACF"\n", ETHER_TO_MACF(client_list->ea[i]));
								match = 1;
								break;
							}
							else {
								cprintf("Remove from Deny list: cnt[%d] addr:"MACF"\n",
								i,ETHER_TO_MACF(client_list->ea[i]));
							}
						}
						if (!match) 
						{
							memcpy(ea, &(static_maclist->ea[cnt]), sizeof(struct ether_addr));
							maclist->count++;
							cprintf("Adding to Deny list: cnt[%d] addr:"MACF"\n",
							maclist->count, ETHERP_TO_MACF(ea));
							ea++;
						}	                
					}
				}
			}      
		}
		else {  //ALLOW MODE

			if (block == 1) 
			{
				ea = &(maclist->ea[0]);
				maclist->count = 0;

				if (static_maclist) 
				{

					for (cnt = 0; cnt < static_maclist->count; cnt++) {
						cprintf("%d:Allow mode: static mac[%d] addr:"MACF"\n",__LINE__, cnt,
						ETHER_TO_MACF(static_maclist->ea[cnt]));
						match = 0;
						for  (i = 0; i < client_list->count ; i++) {
							cprintf("%d Checking client_list :"MACF", static_maclist:"MACF"\n",__LINE__, ETHER_TO_MACF(client_list->ea[i]), ETHER_TO_MACF(static_maclist->ea[cnt]));
							if (memcmp(&(client_list->ea[i]), &(static_maclist->ea[cnt]),  sizeof(struct ether_addr)) == 0) {
							cprintf("%d MATCH maclist "MACF"\n", __LINE__,ETHER_TO_MACF(static_maclist->ea[cnt]));
							match = 1;
							break;
							}
						}	                        
						if (!match) {
							memcpy(ea, &(static_maclist->ea[cnt]), sizeof(struct ether_addr));
							maclist->count++;
							cprintf("%d Adding to Allow list: cnt[%d] addr:"MACF"\n",__LINE__,
							maclist->count, ETHERP_TO_MACF(ea));
							ea++;
						}	                
					}
				}
			}
			else if (block == 0) 
			{
				if (static_maclist) 
				{
					cprintf("Allow mode: Adding static maclist\n");
					maclist->count = static_maclist->count;
					memcpy(maclist_buf, static_maclist,
					sizeof(uint) + ETHER_ADDR_LEN * (maclist->count));


					ea = &(maclist->ea[maclist->count]);

					for (cnt = 0; cnt < client_list->count; cnt++) {
						cprintf("Allow mode: static mac[%d] addr:"MACF"\n", cnt,ETHER_TO_MACF(client_list->ea[cnt]));
						match = 0;
						
						for  (i = 0; i < static_maclist->count ; i++) {
							cprintf("Checking "MACF"\n", ETHER_TO_MACF(static_maclist->ea[i]));
							if (memcmp(&(client_list->ea[cnt]), &(static_maclist->ea[i]), sizeof(struct ether_addr)) == 0) {
								cprintf("MATCH maclist "MACF"\n", ETHER_TO_MACF(client_list->ea[cnt]));
								match = 1;
								break;
							}
						}
						if (!match) {
							memcpy(ea, &(client_list->ea[cnt]), sizeof(struct ether_addr));
							maclist->count++;
							cprintf("Adding to Allow list: cnt[%d] addr:"MACF"\n",
							maclist->count, ETHERP_TO_MACF(ea));
							ea++;
						}
					}
				}
				else {
					if (client_list->count > 0) {
						maclist->count = client_list->count;
						memcpy(maclist, client_list, sizeof(uint) + ETHER_ADDR_LEN * (client_list->count));	                			                	
					}
				}
			}
		}

		cprintf("maclist count[%d] \n", maclist->count);
		for (cnt = 0; cnt < maclist->count; cnt++) {
			cprintf("%d maclist: "MACF"\n",__LINE__,
			ETHER_TO_MACF(maclist->ea[cnt]));
		}

		doSystem("wl -i %s mac none",wlif_name);

		if (maclist->count > 0) {
			ret = wl_ioctl(wlif_name, WLC_SET_MACMODE, &macmode, sizeof(macmode));
			if(ret < 0) {
				cprintf("[WARNING] %s set macmode[%d] error!!!\n", wlif_name, macmode);
				goto err;
			}

			ret = wl_ioctl(wlif_name, WLC_SET_MACLIST, maclist, sizeof(maclist_buf));
			if (ret < 0) {
				cprintf("Err: [%s] set maclist...\n", wlif_name);
			}
		}
err:
		if (static_maclist != NULL)
			free(static_maclist);
#endif			
}

int get_psta_status(int unit)
{
return 0;
}
#endif





