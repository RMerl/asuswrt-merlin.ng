#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>
#include <wlutils.h>
#include <ctype.h>
#include "utils.h"
#include "shutils.h"
#include "shared.h"
#include "amas_path.h"

#if 0
int is_wlsta_exist(int unit, int vidx)
{
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name = NULL;
	struct json_object *allwlClientListObj = NULL;
	json_object *stamacobj= NULL;
	char stamac[32] = {0};
	char ifAlias[16] = {0};
	memset(prefix, 0x00, sizeof(prefix));
	snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, vidx);

	if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")) {

			name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

			memset(ifAlias, 0, sizeof(ifAlias));
		    if_nametoalias(name, &ifAlias[0], sizeof(ifAlias)); 	
			allwlClientListObj = json_object_from_file(WLSTA_JSON_FILE);
			if (allwlClientListObj) {
				json_object_object_foreach(allwlClientListObj, key, val) {
					if (json_object_object_get_ex(val, ifAlias , &stamacobj) == TRUE){
						memset(stamac, 0x00, sizeof(stamac));
						snprintf(stamac, sizeof(stamac), "%s", json_object_get_string(stamacobj));
						//dbG("%s:%s:%d stamac = %s\n",__FILE__, __FUNCTION__, __LINE__, stamac);
						return 1;
					}
					else
						return 0;
				}
			json_object_put(allwlClientListObj);
			}
	}
	return 0;
}


int is_wlsta_connect(int unit, int vidx, char *macaddr)
{
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *name = NULL;
	struct json_object *allwlClientListObj = NULL;
	json_object *stamacobj= NULL;
	json_object *clientmacobj= NULL;
	char stamac[32] = {0};
	char ifAlias[16] = {0};
	memset(prefix, 0x00, sizeof(prefix));
	snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, vidx);

	if (!get_wl_sta_list()){
		return 0;
	}

	if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")) {

			name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

			memset(ifAlias, 0, sizeof(ifAlias));
		    if_nametoalias(name, &ifAlias[0], sizeof(ifAlias)); 	
			allwlClientListObj = json_object_from_file(WLSTA_JSON_FILE);
			if (allwlClientListObj) {
				json_object_object_foreach(allwlClientListObj, key, val) {

					if (json_object_object_get_ex(val, ifAlias , &stamacobj) == TRUE)
					{
						memset(stamac, 0x00, sizeof(stamac));
						snprintf(stamac, sizeof(stamac), "%s", json_object_get_string(stamacobj));

						if (json_object_object_get_ex(stamacobj, macaddr , &clientmacobj) == TRUE)
						{
								return 1;
								break;
						}		
					}
					
				}	
			json_object_put(allwlClientListObj);
			}
	}
	return 0;
}

void set_deauth_sta(int bssidx, int vifidx, char *mac_addr)
{

	char *wlif_name = NULL;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char cmd[256]={0};

	if (vifidx > 0) {
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", bssidx, vifidx);
	}
	else 
		snprintf(prefix, sizeof(prefix), "wl%d_", bssidx);

	wlif_name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));


#if defined(RTCONFIG_RALINK)
	//cprintf("[%s]: [DISCONNECT] WI[%s], Sta[%s] \n", __FUNCTION__, wlif_name,  mac_addr);
	//snprintf(cmd, sizeof(cmd), "iwpriv %s set DisConnectSta=%s", wlif_name, mac_addr);
#elif defined(RTCONFIG_QCA)
	//cprintf("DEAUTHENTICATE ["MACF"] from %s\n", ETHER_TO_MACF(sta->addr), wlif_name);
	//sprintf(cmd, "iwpriv %s kickmac "MACF, wlif_name, ETHER_TO_MACF(sta->addr));
#elif defined(RTCONFIG_REALTEK)
	//cprintf("DEAUTHENTICATE ["MACF"] from %s\n", ETHER_TO_MACF(sta->addr), wlif_name);
	//sprintf(cmd, "iwpriv %s del_sta %02x%02x%02x%02x%02x%02x", wlif_name, ETHER_TO_MACF(sta->addr));
	//system(cmd);
#else /* BCM */
	
	sprintf(cmd, "wl -i  %s deauthenticate %s", wlif_name, mac_addr);
	system(cmd);


#endif
}
#endif

int cal_space(char *s1)
{

  printf("s1 = %s\n", s1);

  int space = 0;

    if(space == 0 && isspace(*s1)) {
        printf("config format is incorrect.\n");
        return 0;
    }

    while (*s1)
    {
       if (isspace(*s1))
       {
           space++;
       }
       s1++;
    }
    s1--;
    if(isspace(*s1)) {
        printf("config format is incorrect.\n");
        return 0;
    }

    printf("parameter count = %d\n", space+1);
   return space + 1;
}


int get_wl_count()
{
	char wif[256]={0}, *next = NULL;
	int SUMband = 0;

	//Get number of bands and set initial status for wlc.
	foreach(wif, nvram_safe_get("sta_ifnames"), next) {
		SUMband++;
	}

	return SUMband;
}

int get_eth_count()
{
	char eif[256]={0}, *next = NULL;
	int SUMeth = 0;
	//Get number of bands and set initial status for wlc.
	foreach(eif, nvram_safe_get("eth_ifnames"), next) {
		SUMeth++;
	}

	return SUMeth;
}

