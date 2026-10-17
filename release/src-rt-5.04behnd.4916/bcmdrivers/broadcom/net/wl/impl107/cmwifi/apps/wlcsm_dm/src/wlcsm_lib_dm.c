#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <wlcsm_lib_wl.h>
#include "wlcsm_linux.h"
#include "wlcsm_lib_dm.h"
#include "wlcsm_lib_nvram.h"

/*! @file  wlcsm_lib_dm.c
 *
 *  Data model implemenation header file
 *
 *
 * */
#define DM_TR181 (1)
#define SELECTED_DM (DM_TR181)
WLCSM_WLAN_WIFI_STRUCT g_wifi_obj;
WLCSM_WLAN_ADAPTER_STRUCT *gp_adapter_objs=NULL;
WLCSM_DM_HANDLER_STRUCT  *wlcsm_dm_handler=NULL;
WLCSM_DM_MNGR_CMD_HOOK_FUNC  g_wlcsm_dm_mngr_runtime_hook=NULL;

WLCSM_AUTHMODE_DM_MNGR_MAPPER authModeMapper[] =
{
    {"None"                 , WL_AUTH_OPEN         },
    {"None"                 , "*DEL*"              },
    {"WPA-Personal"         , WL_AUTH_WPA_PSK      },
    {"WPA2-Personal"        , WL_AUTH_WPA2_PSK     },
    {"WPA-WPA2-Personal"    , WL_AUTH_WPA2_PSK_MIX },
    {"WPA-Enterprise"       , WL_AUTH_WPA          },
    {"WPA2-Enterprise"      , WL_AUTH_WPA2         },
    {"WPA-WPA2-Enterprise"  , WL_AUTH_WPA2_MIX     }
};

#define WLCSM_AUTHMODE_DM_MNGR_MAPPER_ENTRIES_NUM     (sizeof(authModeMapper)/sizeof(WLCSM_AUTHMODE_DM_MNGR_MAPPER))

int wlcsm_dm_adjust_security(char *dm_value, int dm_buf_len, char *wlmngr_value, int wlmngr_buf_len, unsigned char loading)
{
    int i = 0;

    if(loading)
    {
        strncpy(wlmngr_value, authModeMapper[0].wlmngr_value, wlmngr_buf_len);
        for(i=0;i<WLCSM_AUTHMODE_DM_MNGR_MAPPER_ENTRIES_NUM;i++)
        {
            if(!strcmp(dm_value, authModeMapper[i].dm_value))
            {
                strncpy(wlmngr_value, authModeMapper[i].wlmngr_value, wlmngr_buf_len);
                break;
            }
        }
    }
    else
    {
        strncpy(dm_value, authModeMapper[0].dm_value, dm_buf_len);
        for(i=0;i<WLCSM_AUTHMODE_DM_MNGR_MAPPER_ENTRIES_NUM;i++)
        {
            if(!strcmp(wlmngr_value, authModeMapper[i].wlmngr_value))
            {
                strncpy(dm_value, authModeMapper[i].dm_value, dm_buf_len);
                break;
            }
        }
    }

    return 0;
}

int wlcsm_dm_is_runtime_oid(unsigned int oid)
{
    int i=g_WLMNGR_RUNTIME_OIDS_NUM-1;
    while(i>=0) {
        if(g_WLMNGR_RUNTIME_OIDS[i]==oid) return 1;
    }
    return 0;
}

static WLCSM_MNGR_NAME_OFFSET *_wlcsm_dm_mngr_get_mapper(unsigned int pos, unsigned int *num_of_entries)
{
    int i=0;
    int count=g_WLMNGR_OIDS_NUM+g_WLMNGR_RUNTIME_OIDS_NUM;
    for (i = 0; i < count; i++) {
        if(g_WLMNGR_OID_MAPPER[i].oid==pos) {
            *num_of_entries=g_WLMNGR_OID_MAPPER[i].size;
            return g_WLMNGR_OID_MAPPER[i].name_offset;
        }
    }
    return NULL;
}

char *_wlcsm_dm_mngr_get_src(unsigned int idx,unsigned int sub_idx,unsigned int mngr_oid)
{

    char *src=NULL;
    WLCSM_WLAN_ADAPTER_STRUCT *adapter=&(gp_adapter_objs[idx]);

    switch ( mngr_oid ) {
    case WLMNGR_DATA_POS_WIFI:
        src=(char *)&g_wifi_obj;
        break;
    case WLMNGR_DATA_POS_WIFI_RADIO:
        src=(char *)&(adapter->radio);
        break;
    case WLMNGR_DATA_POS_WIFI_SSID   :
        src=(char *)&(adapter->bssids[sub_idx]);
        break;
    case WLMNGR_DATA_POS_WIFI_ACCESS_POINT:
        src=(char *)&(adapter->ssids[sub_idx].accesspoint);
        break;
    case WLMNGR_DATA_POS_WIFI_ACCESS_POINT_WPS:
        src=(char *)&(adapter->ssids[sub_idx].wps);
        break;
    case WLMNGR_DATA_POS_WIFI_ACCESS_POINT_SECURITY:
        src=(char *)&(adapter->ssids[sub_idx].security);
        break;
    default:
        if(wlcsm_dm_is_runtime_oid(mngr_oid))
            return g_wlcsm_dm_mngr_runtime_hook?g_wlcsm_dm_mngr_runtime_hook(idx,sub_idx,mngr_oid,WLCSM_DM_MNGR_CMD_GET_SOURCE,NULL):NULL;
        break;
    }
    return src;
}

#ifndef PHASE1_SEPARATE_NVRAM
/*************************************************************//* *
  *
  * @brief  internal API to write nvram from the global nvram setting
  *
  *	Idealy each single nvram entry should have one parameter in the
  *	mngr global structure, but it is not due to the fact nvram entries
  *	are continuously getting added, in this case, we can not catch all
  *	the time, thus, leave a global nvram for the purpose to make all
  *	nvram available. Most of the time, those nvrams are from wireless
  *	router default values when wlconf does configuration and found no
  *	default value. When wlmngr get chance to write back to persistent
  *	storage(dm like  tr181 to MDM), these value will be write to dm and
  *	be restore back by this API.
  *
  * @return void
  ****************************************************************/
static void _wlcsm_dm_load_nvram()
{

    char *pos,*str, *name=NULL, *val=NULL;
    int len, i;
    char pair[WL_LG_SIZE_MAX+WL_SIZE_132_MAX];
    int pair_len_byte = 2;
    char *format;

    str=g_wifi_obj.nvram;
    if (str) {

        /* tell if it is old format or new format */
        if(!strncmp(str,"FFFF",4)) {
            str+=4;
            format="%03x";
            pair_len_byte=3;
        } else {
            format="%02x";
        }

        for (pos=str; *pos; ) {
            if( strlen(pos)< pair_len_byte) {
                printf("nvram date corrupted[%s]..\n", pos);
                break;
            }

            strncpy(pair, pos, pair_len_byte);
            pair[pair_len_byte]='\0';

            if ( sscanf(pair, format, &len) !=1 ) {
                printf("len error [%s]\n", pair);
                break;
            }
            if (len < pair_len_byte || len>= WL_LG_SIZE_MAX+WL_SIZE_132_MAX) {
                printf("corrupted nvam...\n");
                break;
            }
            if (strlen(pos+pair_len_byte) < len ) {
                printf("nvram date corrupted[%s]..\n", pos);
                break;
            }

            strncpy(pair, pos+pair_len_byte, len);
            pair[len]='\0';
            pos += pair_len_byte+len;

            name = pair;
            val = strchr(pair, '=');
            if (val) {
                *val = '\0';
                val++;
                wlcsm_nvram_set(name, val);

                /* TODO: Why wlcsm_nvram_set don't trigger WLCSM_EVT_NVRAM_CHANGED event? */
                wlcsm_nvram_update_runtime_mngr(name, val);

            } else {
                printf("pair not patch.[%s]..\n", pair);
                break;
            }
        }

#if defined(CMWIFI) && defined(BUILD_NO_CMS)
		for(i=0;i<g_wifi_obj.radio_number;i++) {
			wlcsm_dm_init_wl_boot_time(i);
		}
#endif

        /* wifi nvram is not useful anymore after this. free it for memory */
        free(g_wifi_obj.nvram);
        g_wifi_obj.nvram=NULL;
    }
}
#endif /* PHASE1_SEPARATE_NVRAM */

#ifdef PHASE1_SEPARATE_NVRAM
/* Read all nvrams and update the g_wifi_obj, etc., runtime objects in wlmngr.
 */
int
wlcsm_dm_update_runtime_mngr_by_nvram_all(void)
{
	char *name, *val, *buf;
	int len;

	buf = malloc(MAX_NVRAM_SPACE);
	if (buf == NULL)
		return -1;

	if (wlcsm_nvram_getall(buf, MAX_NVRAM_SPACE) != WLCSM_SUCCESS) {
		free(buf);
		return -2;
	}

	len = strlen(buf);
	for (name = buf; *name; name += len) {
		len = strlen(name) + 1;
		val = strchr(name, '=');
		if (val == NULL)
			continue;
		*val++ = '\0';

		wlcsm_nvram_update_runtime_mngr(name, val);
	}
	free(buf);

	return 0;
}
#endif /* PHASE1_SEPARATE_NVRAM */

static inline  void  wlcsm_struct_modifystr_value(void *struct_addr,unsigned int offset,char *value)
{
    char **str_ptr_ptr=(char **)(struct_addr+offset);
    if(value!=*str_ptr_ptr) {
        /* at lest one of them is not NULL */
        if(value && *str_ptr_ptr && !strcmp(value,*str_ptr_ptr))
            return;
        else if(*str_ptr_ptr)
            free(*str_ptr_ptr); /* free the orignal value */

        if(value) {
            *str_ptr_ptr=malloc(strlen(value)+1);
            if(*str_ptr_ptr) {
                strncpy(*str_ptr_ptr,value,strlen(value)+1);
            } else {
                printf("JXUJXU:%s:%d  Allocate memrory error \r\n",__FUNCTION__,__LINE__ );
                *str_ptr_ptr=NULL;
            }
        } else
            *str_ptr_ptr=NULL;
    }
}
static inline char *wlcsm_struct_getstr_value(void *struct_addr,unsigned int offset)
{
    char **str_ptr_ptr=(char **)(struct_addr+offset);
    return *str_ptr_ptr;
}

int wlcsm_dm_save_nvram(void)
{
    if(wlcsm_dm_handler && g_wifi_obj.nvram) {
        return wlcsm_dm_handler->dm_save_nvram();
    }
    return 0;
}

int  wlcsm_dm_load(int idx)
{
    if(wlcsm_dm_handler) {
        wlcsm_dm_handler->dm_load(idx);
#ifndef PHASE1_SEPARATE_NVRAM
        /* load nvram after dm_load, since g_wifi_obj.nvram are loaded in dm_load. */
        if(idx==0)
            _wlcsm_dm_load_nvram();
        return 0;
#else
/* Below shall be done by wifi_setup.sh. */
#if defined(CMWIFI) && defined(BUILD_NO_CMS)
	{
	   int i;

	   for (i = 0; i< g_wifi_obj.radio_number; i++) {
		wlcsm_dm_init_wl_boot_time(i);
	   }
	}
#endif
	return wlcsm_dm_update_runtime_mngr_by_nvram_all();
#endif /* PHASE1_SEPARATE_NVRAM */
    }
    else
        return 1;
}

/* Get the system uptime in secs.
 * cat /proc/uptime gives (e.g) 1559.22 2919.44
 *   - the uptime of the system (1559.22 seconds), and
 *   - the amount of time spent in idle process (2919.44 seconds)
 * we only use to the granularity of secs (1559) now - ignoring the rest
 */
unsigned int wlcsm_dm_get_systemUpSecs()
{
	char cmd[80] = {0};
	FILE *fp = NULL;
	unsigned int upSecs;

	snprintf(cmd, sizeof(cmd), "/bin/cat /proc/uptime > /var/systemUptimeDM.txt");
	system(cmd);

	upSecs = 0;
	snprintf(cmd, sizeof(cmd), "/var/systemUptimeDM.txt");
	fp = fopen(cmd, "r");
	if (fp != NULL) {
		fscanf(fp, "%d", &upSecs);
		WLCSM_TRACE(WLCSM_TRACE_DBG, "DBG-DM upSecs=%d ......\n", upSecs);
		fclose(fp);
	}

	return upSecs;
}

unsigned int wlcsm_dm_init_wl_boot_time(int idx)
{
	int i;
	char boot_time[WL_SM_SIZE_MAX] = {0};
	char value[WL_LG_SIZE_MAX] = {0};
	unsigned int upSecs = wlcsm_dm_get_systemUpSecs();

	memset(value, 0, sizeof(value));
	snprintf(value, sizeof(value), "%d", upSecs);

	snprintf(boot_time, sizeof(boot_time), "wl%d_boot_time", idx);
	WLCSM_TRACE(WLCSM_TRACE_ERR, "DBG-DM %s %s=%s idx=%d ...\n", __FUNCTION__, boot_time, value, idx);
	wlcsm_nvram_set(boot_time, value);
	wlcsm_nvram_update_runtime_mngr(boot_time, value);

	return(upSecs);
}

int  wlcsm_dm_save_config(int idx)
{

    if(wlcsm_dm_handler)
        return   wlcsm_dm_handler->dm_save(idx);
    else
        return 1;
}

int wlcsm_dm_reset_radio(int idx)
{
    if(wlcsm_dm_handler)
        return   wlcsm_dm_handler->dm_reset_radio(idx);
    else
        return 1;
}

int wlcsm_dm_reset_ap(int idx, int sub_idx)
{
    if(wlcsm_dm_handler)
        return   wlcsm_dm_handler->dm_reset_ap(idx, sub_idx);
    else
        return 1;
}

int wlcsm_dm_mngr_set_value(int idx,int sub_idx,char *name,char *value,unsigned int pos)
{
    unsigned int num_of_entry=0;
    int i=0;
    int offset=0;
    char *src=NULL;
    int set=1;
    WLCSM_DM_TYPE data_type;
    WLCSM_MNGR_NAME_OFFSET *obj_mapper= NULL;
    int to_update_nvram = !IS_IDX_MARKED(idx);
    idx=REAL_IDX(idx);

    if((obj_mapper=_wlcsm_dm_mngr_get_mapper(pos,&num_of_entry)) &&
       (src=_wlcsm_dm_mngr_get_src(idx,sub_idx,pos))) {

        for ( i=0; i<num_of_entry; i++ ) {
            if(!strcasecmp(obj_mapper[i].name,name)) {
                data_type=obj_mapper[i].type;
                offset=obj_mapper[i].offset;

                switch(data_type) {
                case WLCSM_DT_UINT: {
                    UINT32 num;
                    sscanf(value,"%u",&num);
                    NUMVAR_VALUE(src,offset,UINT32)=num;
                    break;
                }
                case WLCSM_DT_STR2INT:
                case WLCSM_DT_SINT32: {
                    SINT32 num;
                    sscanf(value,"%d",&num);
                    NUMVAR_VALUE(src,offset,SINT32)=num;
                    break;
                }
                case WLCSM_DT_BOOLREV:
                case WLCSM_DT_BOOL: {
                    SINT32 num;
                    sscanf(value,"%d",&num);
                    NUMVAR_VALUE(src,offset,UBOOL8)=num;
                    break;
                }
                case WLCSM_DT_UINT64: {
                    UINT64 num;
                    sscanf(value,"%llu",&num);
                    NUMVAR_VALUE(src,offset,UINT64)=num;
                    break;
                }
                case WLCSM_DT_SINT64: {
                    SINT64 num;
                    sscanf(value,"%lld",&num);
                    NUMVAR_VALUE(src,offset,SINT64)=num;
                    break;
                }
                case WLCSM_DT_STRING:
                case WLCSM_DT_BASE64:
                case WLCSM_DT_HEXBINARY:
                case WLCSM_DT_DATETIME:
                case WLCSM_DT_INT2STR:
                    wlcsm_struct_modifystr_value(src,offset,value);
                    break;
                default: {
                    set=0;
                    WLCSM_TRACE(WLCSM_TRACE_ERR,"%s:%d  NOT VALID TYPE \r\n",__FUNCTION__,__LINE__ );
                    return 1;
                }
                }
                if(g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_OBJ_MODIFIED].func) {
                    WLCSM_DM_OBJ_VALUE_SET value_set;
                    value_set.oid=pos;
                    value_set.offset=offset;
                    value_set.value=value;
                    g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_OBJ_MODIFIED].func(idx,sub_idx,&value_set);
                }
                /* to update the nvram after setting mngr vars */
                if(set && to_update_nvram) wlcsm_update_runtime_nvram(idx,sub_idx,name,value);
                return 0;

            }
        }
    }
    return 1;
}

int wlcsm_dm_mngr_set_all_value(int idx,int sub_idx,char *name,char *value)
{

    int pos= g_WLMNGR_OIDS_NUM-1;
    int ret=0;

    for ( ; pos>=0; pos-- ) {
        ret=wlcsm_dm_mngr_set_value(idx,sub_idx,name,value,g_WLMNGR_OIDS[pos]);
        if(!ret) return 0;
    }
    WLCSM_TRACE(WLCSM_TRACE_ERR,"could not set %s with value:%s for %d:%d \r\n" ,name,value,idx,sub_idx);
    return 1;
}

int wlcsm_nvram_update_runtime_mngr(char *name,char *value)
{
    unsigned int idx,ssid_idx;
    int entries_num=sizeof(g_wlcsm_nvram_mngr_mapping)/sizeof(WLCSM_NVRAM_MNGR_MAPPING);
    char *nvram_var,*mngr_var;
    int index=0;
    int i=0;
    char *nv_name=wlcsm_nvram_name_parser(name,&idx,&ssid_idx);
    char lan_ifname[WL_SM_SIZE_MAX];

    if(nv_name) {

        if (idx >= g_wifi_obj.radio_number) {
            printf("Ignore: radio_idx hit limitation: name=<%s> value=<%s> idx=%d\n", name, value, idx);
            return 0;
        }

        if (ssid_idx >= WL_MAX_NUM_SSID)
            return 0;

        for ( index=0; index<entries_num; index++) {
            nvram_var= g_wlcsm_nvram_mngr_mapping[index].nvram_var;
            mngr_var= g_wlcsm_nvram_mngr_mapping[index].mngr_var;
            if(!strcmp(nv_name,nvram_var)) {
#if defined(CMWIFI) && defined(BUILD_NO_CMS)
                int pos = g_wlcsm_nvram_mngr_mapping[index].pos;
                unsigned int param_version = 0;
                unsigned int nvram_ver = 0;
                char *p_buf = NULL;

                if((p_buf = nvram_get("nvram_ver")))
                    nvram_ver = atoi(p_buf);

                if(WL_WIFI_WLNVRAMVERSION > nvram_ver)
                {
                    if(_wlcsm_dm_lattice_get_param_version(pos, idx, ssid_idx, mngr_var, &param_version) == 0)
                    {
                        if(param_version > nvram_ver)
                        {
                            printf(" !!!Parameter Upgrade: object[%d], idx[%d], ssid_idx[%d], varName[%s], param_version[%d], nvram_ver[%d]\n", pos, idx, ssid_idx, mngr_var, param_version, nvram_ver);
                            if (!strncmp(mngr_var, "wlBridgeInfo", sizeof(mngr_var))) {
                                /* EAPD_WKSP_MAX_NO_BRIDGE defined as 150 */
                                for (i = 0; i < 150; i++) {
                                    snprintf(lan_ifname, sizeof(lan_ifname), "lan%d_ifname", i);
                                    if (nvram_get(lan_ifname)) {
                                        nvram_unset(lan_ifname);
                                    }
                                }
                                nvram_unset("lan_ifnames");
                            }
                            return 0;
                        }
                    }
                }
#endif
                switch(g_wlcsm_nvram_mngr_mapping[index].data_type) {

                case WLCSM_DT_INT2STR: {
                    char tempValue[32];
                    /* mngr is int and nvram is str */
                    snprintf(tempValue, sizeof(tempValue), "%d",wlcsm_dm_get_mapper_int(g_wlcsm_nvram_mngr_mapping[index].mapper,value));
                    nvram_var=tempValue;
                    break;
                }
                case WLCSM_DT_STR2INT: {
                    /* mngr is str and nvram is number */
                    int num=0;
                    sscanf(value,"%d",&num);
                    nvram_var=wlcsm_dm_get_mapper_str(g_wlcsm_nvram_mngr_mapping[index].mapper,num);
                    break;
                }
                default:
                    nvram_var=value;
                }
                wlcsm_dm_mngr_set_all_value(MARK_IDX(idx),ssid_idx,mngr_var,nvram_var);
                return 0;
            }
        }
    }
    return 1;
}

int wlcsm_update_runtime_nvram(unsigned int idx,unsigned int ssid_idx,char *name,char *value)
{
    int entries_num=sizeof(g_wlcsm_nvram_mngr_mapping)/sizeof(WLCSM_NVRAM_MNGR_MAPPING);
    char *nvram_var,*mngr_var;
    int index=0;
    int i=0;
    for (index=0; index<entries_num; index++) {
        nvram_var = g_wlcsm_nvram_mngr_mapping[index].nvram_var;
        mngr_var = g_wlcsm_nvram_mngr_mapping[index].mngr_var;
        if (!strcmp(name,mngr_var)) {
            /* has nvram entry for this name, we will update nvram now */
            char fullname[128];
            char tempValue[32];
            if (!ssid_idx)
                snprintf(fullname, sizeof(fullname), "wl%d_%s",idx,nvram_var);
            else
                snprintf(fullname, sizeof(fullname), "wl%d.%d_%s",idx,ssid_idx,nvram_var);

            switch (g_wlcsm_nvram_mngr_mapping[index].data_type) {
            case WLCSM_DT_STR2INT:
                /* mngr is str and nvram is number */
                snprintf(tempValue, sizeof(tempValue), "%d",wlcsm_dm_get_mapper_int(g_wlcsm_nvram_mngr_mapping[index].mapper,value));
                nvram_var=tempValue;
                break;
            case WLCSM_DT_INT2STR: {
                /* mngr is str and nvram is number */
                int num=0;
                sscanf(value,"%d",&num);
                nvram_var=wlcsm_dm_get_mapper_str(g_wlcsm_nvram_mngr_mapping[index].mapper,num);
                break;
            }
            default:
                nvram_var=value;
            }

            /* When wlBridgeInfo is set, both wl0_brinfo and wl1_brinfo need to updated */
            if (!strcmp(name, "wlBridgeInfo")) {
                for (i = 0; i < WL_WIFI_RADIO_NUMBER; i++) {
                    snprintf(fullname, sizeof(fullname), "wl%d_brinfo",i);
                    wlcsm_nvram_set(fullname,nvram_var);
                }
            }
            else if (ssid_idx > 0 && !strcmp(name, "wlEnblSsid")) {
                //update the wlX_vifs list
                char wl_vif[] = "wlXXXXXXXXX_vifs", *wl_vif_value = NULL;
                char buf[128];
                char vif[] = "wlXXXXXXXXX";

                snprintf(wl_vif, sizeof(wl_vif),"wl%d_vifs", idx);
                memset(buf, 0, sizeof(buf));

                /* This logic here decides if updates to virtual interface list on the
                   parent is required */
                wl_vif_value = nvram_get(wl_vif);
                snprintf(vif, sizeof(vif), "wl%d.%d", idx, ssid_idx);
                if (wl_vif_value) {
                    int found = 0;
                    char name[32], *next = NULL;

                    /* virtual interface on the list already? */
                    foreach (name, wl_vif_value, next) {
                        if (!strcmp(name, vif)) {
                            found = 1;
                            break;
                        }
                    }

                    if (!strcmp(nvram_var, "1")) {
                        if (!found) {
                            snprintf(buf, sizeof(buf), "%s %s", wl_vif_value, vif);
                        }
                        else {
                            snprintf(buf, sizeof(buf), "%s", wl_vif_value);
                        }
                    }
                    else {
                        /* Purge interface from wl_vifs as its bss is now disabled */
                        /* vif present, delete from wl_vifs */
                        if (found) {
                            memset(buf, 0, sizeof(buf));
                            foreach (name, wl_vif_value, next) {
                                /* Copy all interfaces except the one to be removed */
                                if (strcmp(name, vif)) {
                                    if (*buf) strncat(buf, " ", 1);
                                    strncat(buf, name, strlen(name));
                                }
                            }
                        }
                        else {
                            snprintf(buf, sizeof(buf), "%s", wl_vif_value);
                        }
                    }
                }
                else {
                    if (!strcmp(nvram_var, "1")) {
                        snprintf(buf, sizeof(buf), "%s", vif);
                    }
                }

                /* Regenerate virtual interface list */
                if (*buf) wlcsm_nvram_set(wl_vif, buf);
                else wlcsm_nvram_unset(wl_vif);

                wlcsm_nvram_set(fullname, nvram_var);
            }
            else
                wlcsm_nvram_set(fullname, nvram_var);

            return 0;
        }
    }
    return 1;
}

char *wlcsm_dm_mngr_get_value(unsigned int idx,unsigned int sub_idx,char *name,char *varValue,unsigned int pos)
{

    unsigned int num_of_entry=0;
    int i=0;
    int offset=0;
    char *src=NULL;
    WLCSM_DM_TYPE data_type;
    WLCSM_MNGR_NAME_OFFSET *obj_mapper= NULL;
    //WLCSM_TRACE(WLCSM_TRACE_DBG," name:%s ,oid:%u\r\n",name,pos );

    //obj_mapper=_wlcsm_dm_mngr_get_mapper(pos,&num_of_entry);
    //src=_wlcsm_dm_mngr_get_src(idx,sub_idx,pos);

    if((obj_mapper=_wlcsm_dm_mngr_get_mapper(pos,&num_of_entry)) &&
       (src=_wlcsm_dm_mngr_get_src(idx,sub_idx,pos))) {

        //if(obj_mapper && src) {

        for ( i=0; i<num_of_entry; i++ ) {
            if(!strcasecmp(obj_mapper[i].name,name)) {
                //WLCSM_TRACE(WLCSM_TRACE_DBG," found in mapper  and src:%p, obj_mapper:%p\r\n",src, obj_mapper);

                data_type=obj_mapper[i].type;
                offset=obj_mapper[i].offset;

                switch(data_type) {
                case WLCSM_DT_UINT: {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%u", NUMVAR_VALUE(src,offset,UINT32));
                    break;
                }
                case WLCSM_DT_STR2INT:
                case WLCSM_DT_SINT32: {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d", NUMVAR_VALUE(src,offset,SINT32));
                    break;
                }
                case WLCSM_DT_UINT64:
                    snprintf(varValue, WL_MID_SIZE_MAX, "%llu", NUMVAR_VALUE(src,offset,UINT64));
                    break;
                case WLCSM_DT_SINT64:
                    snprintf(varValue, WL_MID_SIZE_MAX, "%lld",  NUMVAR_VALUE(src,offset,SINT64));
                    break;
                case WLCSM_DT_BOOLREV:
                case WLCSM_DT_BOOL:
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d",  NUMVAR_VALUE(src,offset,UBOOL8));
                    break;
                case WLCSM_DT_STRING:
                case WLCSM_DT_BASE64:
                case WLCSM_DT_HEXBINARY:
                case WLCSM_DT_DATETIME:
                case WLCSM_DT_INT2STR: {
                    char ** p_str_pptr=(char **) (src+offset);
                    if((*p_str_pptr)!=NULL) {
                        sprintf(varValue,"%s",*p_str_pptr);
                    } else
                        return NULL;
                }
                break;

                default:
                    return NULL;
                }
                return varValue;
            }
        }
    }
    return NULL;
}

char *wlcsm_dm_mngr_get_all_value(unsigned int idx,unsigned int sub_idx,char *name,char*varValue)

{

    char *value=NULL;
    int pos= g_WLMNGR_OIDS_NUM-1;

    for ( ; pos>=0; pos-- ) {
        value=wlcsm_dm_mngr_get_value(idx,sub_idx,name,varValue,g_WLMNGR_OIDS[pos]);
        if(value) return value;
    }
    WLCSM_TRACE(WLCSM_TRACE_ERR," Failed to get value for, should check runtime? %s \r\n",name );
    return NULL;
}

char *wlcsm_mapper_get_mngr_value(unsigned int idx,unsigned int sub_idx,WLCSM_NVRAM_MNGR_MAPPING *mapping,char *varValue)
{

    WLCSM_MNGR_VAR_POS pos=mapping->pos;
    int offset=mapping->mngr_var_offset;
    WLCSM_DM_TYPE data_type= mapping->data_type;

    WLCSM_WLAN_ADAPTER_STRUCT *adapter;
    char *src=NULL;
    adapter=&(gp_adapter_objs[idx]);
    switch (pos) {
    case MNGR_POS_WIFI:
        src = (char *)&g_wifi_obj;
        break;
    case MNGR_POS_RADIO:
        src=(char *)&(adapter->radio);
        break;
    case MNGR_POS_BSSID:
        src=(char *)&(adapter->bssids[sub_idx]);
        break;
    case MNGR_POS_AP:
        src=(char *)&(adapter->ssids[sub_idx].accesspoint);
        break;
    case MNGR_POS_AP_WPS:
        src=(char *)&(adapter->ssids[sub_idx].wps);
        break;
    case MNGR_POS_AP_SEC:
        src=(char *)&(adapter->ssids[sub_idx].security);
        break;
    default:
        return NULL;
    }

    switch(data_type) {
    case WLCSM_DT_UINT: {
        snprintf(varValue, WL_MID_SIZE_MAX, "%u", NUMVAR_VALUE(src,offset,UINT32));
        break;
    }
    case WLCSM_DT_SINT32: {
        snprintf(varValue, WL_MID_SIZE_MAX, "%d", NUMVAR_VALUE(src,offset,SINT32));
        break;
    }
    case WLCSM_DT_UINT64:
        snprintf(varValue, WL_MID_SIZE_MAX, "%llu", NUMVAR_VALUE(src,offset,UINT64));
        break;
    case WLCSM_DT_SINT64:
        snprintf(varValue, WL_MID_SIZE_MAX, "%lld",  NUMVAR_VALUE(src,offset,SINT64));
        break;
    case WLCSM_DT_BOOLREV:
    case WLCSM_DT_BOOL:
        snprintf(varValue, WL_MID_SIZE_MAX, "%d",  NUMVAR_VALUE(src,offset,UBOOL8));
        break;
    case WLCSM_DT_STRING:
    case WLCSM_DT_BASE64:
    case WLCSM_DT_HEXBINARY:
    case WLCSM_DT_DATETIME: {
        char **p_str_pptr=(char **)(src+offset);
        if((*p_str_pptr)!=NULL) {
            sprintf(varValue,"%s",*p_str_pptr);
        } else
            return NULL;
    }
    break;
    case WLCSM_DT_STR2INT: {
        char **p_str_pptr=(char **)(src+offset);
        if((*p_str_pptr)!=NULL) {
            snprintf(varValue, WL_MID_SIZE_MAX, "%d",wlcsm_dm_get_mapper_int(mapping->mapper,*p_str_pptr));
        } else
            return NULL;
    }
    break;
    case WLCSM_DT_INT2STR: {

        char *tmp_str_ptr=wlcsm_dm_get_mapper_str(mapping->mapper,NUMVAR_VALUE(src,offset,SINT32));
        if(tmp_str_ptr)
            snprintf(varValue, WL_MID_SIZE_MAX, "%s",tmp_str_ptr);
        break;
    }
    default:
        return NULL;
    }
    return varValue;
}

int wlcsm_dm_get_bssid_num(int idx)
{
    WLCSM_WLAN_ADAPTER_STRUCT *adapter;
    adapter=&(gp_adapter_objs[idx-1]);
    if (adapter->radio.wlNumBss > WL_MAX_NUM_SSID) {
        WLCSM_TRACE(WLCSM_TRACE_ERR," !!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!! number of BSS id not right:%d \r\n",adapter->radio.wlNumBss );
        return 1;
    } else
        return adapter->radio.wlNumBss;
}

static int _wlcsm_dm_free_gp_adatper(void)
{
    if(gp_adapter_objs) {
        WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO: free all resources allocated for gp_adapter \r\n" );
        free(gp_adapter_objs);
        gp_adapter_objs=NULL;
    }
    return 0;
}

static int _wlcsm_dm_deinit(void)
{
    if(wlcsm_dm_handler) {
        wlcsm_dm_handler->b_dm_initialized = wlcsm_dm_handler->dm_deinit();
        return wlcsm_dm_handler->b_dm_initialized;
    }
    return 0;
}

int wlcsm_dm_cleanup(void)
{
    _wlcsm_dm_deinit();
    return _wlcsm_dm_free_gp_adatper();
}

int wlcsm_dm_get_bridge_info(char *buf)
{
    if(wlcsm_dm_handler)
        return  wlcsm_dm_handler->dm_query_info(WLCSM_DM_QUERY_BRIDGE_INFO,buf);
    else

        return -1;
}

WLCSM_NAME_OFFSET *wlcsm_dm_get_mngr_entry(void  *phdr,char *varValue,unsigned int *pos)
{
    int ret=0;
    unsigned int nums[2];
    t_WLCSM_MNGR_VARHDR  *hdr=(t_WLCSM_MNGR_VARHDR *)phdr;
    WLCSM_NAME_OFFSET **name_offset=(WLCSM_NAME_OFFSET **)nums;
    if(wlcsm_dm_handler) {
        nums[0]=hdr->dm_oid;
        nums[1]=hdr->offset;
        ret=wlcsm_dm_handler->dm_query_info(WLCSM_DM_QUERY_MNGR_ENTRY,nums);

        WLCSM_TRACE(WLCSM_TRACE_DBG," POS is :%d \r\n",ret );
        if(ret >0) {
            WLCSM_TRACE(WLCSM_TRACE_DBG," return POS :%s\r\n",(*name_offset)->name );
            *pos=(unsigned int)ret;
            return *name_offset;
        } else {
            WLCSM_TRACE(WLCSM_TRACE_DBG," pos is not good \r\n" );
            return NULL;

        }
    } else
        return NULL;
}

/**
*
* 	init data model with default value, in mdm case,
* 	it should init mdm if the configuration is not there
* 	,something like hardwareAdjusting. after this function
* 	,data model should have at least the default configruation
* 	 or loaded saved configuration.
*/
static int _wlcsm_dm_init(void)
{

    int i=0;

    if(!wlcsm_dm_handler) {
        WLCSM_TRACE(WLCSM_TRACE_ERR,"!!! SHOULD SELECT A DM BEFORE INIT  \r\n" );
        return -1;
    } else if(wlcsm_dm_handler->b_dm_initialized) return 0;
    else {
        /* calling dm module real initialization to init global wifi and then  */
        if(!wlcsm_dm_handler->dm_init())
            wlcsm_dm_handler->b_dm_initialized=1;
        else {
            WLCSM_TRACE(WLCSM_TRACE_ERR,"!!! DM init failure \r\n" );
            return -1;
        }
    }

#ifndef CMWIFI /* radion_number and bssid_number have already been updated in dm_init() \
	*/
    g_wifi_obj.radio_number=wlcsm_wl_get_adapter_no();
    g_wifi_obj.bssid_number=0;
    for(; i<g_wifi_obj.radio_number; i++)
        g_wifi_obj.bssid_number+=wlcsm_wl_get_intf_no(i);
    g_wifi_obj.ssid_number=g_wifi_obj.bssid_number;
#endif

    WLCSM_TRACE(WLCSM_TRACE_DBG," g_wifi_obj bssid_number:%d \r\n",g_wifi_obj.bssid_number );

    /* allocated global unique data structure for management if not yet allocated */
    if(!gp_adapter_objs) {

        if((gp_adapter_objs = malloc( sizeof(WLCSM_WLAN_ADAPTER_STRUCT) * g_wifi_obj.radio_number))!=NULL) {
            memset(gp_adapter_objs, 0, sizeof(WLCSM_WLAN_ADAPTER_STRUCT) * g_wifi_obj.radio_number);
        } else {
            WLCSM_TRACE(WLCSM_TRACE_ERR," allocated memory for global structure failure  \r\n" );
            return -1;
        }
    }

    return 0;
}

char * wlcsm_strcpy(char **dst,char *from)
{
    if(from) {
        int len=strlen(from)+1;
        if(*dst)	free(*dst);
        *dst=malloc(len);
        if(*dst)
            return  memcpy(*dst,from,len);

    }
    return NULL;
}

int wlcsm_dm_get_mapper_int(int pos,char *name)
{
    unsigned int index=pos>>_WLCSM_MNGR_STRMAPPER_SHIFT;
    unsigned int number=pos&((1<<_WLCSM_MNGR_STRMAPPER_SHIFT)-1);
    WLCSM_MNGR_STRMAPPER_SET *mapper;
    int i=0;
    number+=index;
    for(i=index; i<number; i++) {
        mapper=&g_wlcsm_mngr_strmapper[i];
        if(!strcasecmp(mapper->str_value,name))
            return mapper->int_value;
    }
    return 0;
}

char *wlcsm_dm_get_mapper_str(int pos,int num)
{
    unsigned int index=pos>>_WLCSM_MNGR_STRMAPPER_SHIFT;
    unsigned int number=pos&((1<<_WLCSM_MNGR_STRMAPPER_SHIFT)-1);
    WLCSM_MNGR_STRMAPPER_SET *mapper;
    int i=0;
    number+=index;
    for(i=index; i<number; i++) {
        mapper=&g_wlcsm_mngr_strmapper[i];
        if(mapper->int_value==num)
            return mapper->str_value;
    }
    return NULL;
}

char *wlcsm_dm_get_dmname(void)
{
    if(wlcsm_dm_handler)
        return wlcsm_dm_handler->dm_name;
    else return NULL;
}

void wlcsm_dm_register(WLCSM_DM_HANDLER_STRUCT *handler)
{
    if(wlcsm_dm_handler) {
        WLCSM_DM_HANDLER_STRUCT *tempHandler=wlcsm_dm_handler;
        /* we don't expect double register when it has the same name ,so just
         * move to the end*/
        while(tempHandler->next);
        tempHandler->next=handler;
        handler->next=NULL;
        handler->pre=tempHandler;
    } else  {
        wlcsm_dm_handler=handler;
        wlcsm_dm_handler->next=NULL;
        wlcsm_dm_handler->pre=NULL;
    }
}

int wlcsm_dm_select_dm(char *name)
{
    /* rewind to the top */
    WLCSM_DM_HANDLER_STRUCT *tempHandler=wlcsm_dm_handler;
    while(tempHandler && tempHandler->pre) tempHandler = tempHandler->pre;
    do {
        if(!strcmp(tempHandler->dm_name,name)) {
            /* found the dm, quite successfully */
#if defined(CMWIFI) && defined(BUILD_NO_CMS)
            if(tempHandler->b_dm_initialized) _wlcsm_dm_deinit();
#else
            if(wlcsm_dm_handler!=tempHandler) _wlcsm_dm_deinit();
#endif
            wlcsm_dm_handler=tempHandler;
            if(!_wlcsm_dm_init()) return wlcsm_dm_load(0);
            else {
                WLCSM_TRACE(WLCSM_TRACE_DBG," DM INIT PROBLEM!!! \r\n" );
                return -1;
            }
        }
    } while(tempHandler && (tempHandler=tempHandler->next));
    return -1;
}

WLCSM_DM_MNGR_EVENT_HANDLER  g_wlcsm_dm_mngr_handlers[]= {
    /* TO make sure the order is the exactly the same as WLCSM_DM_MNGR_EVENT defination */
    { WLCSM_DM_MNGR_EVENT_STAS_CHANGED,NULL},
    { WLCSM_DM_MNGR_EVENT_OBJ_MODIFIED,NULL}
};

int wlcsm_dm_mngr_event_register(WLCSM_DM_MNGR_EVENT event,WLCSM_DM_MNGR_EVENT_HANDLER_FUNC func)
{
    if(event> WLCSM_DM_MNGR_EVENT_LAST) return -1;
    else {
        g_wlcsm_dm_mngr_handlers[event].func=func;
        return 0;
    }
}

int wlcsm_dm_reg_event(WLCSM_DM_MNGR_EVENT evt,int to_register)
{
    if(!wlcsm_dm_handler || evt>= WLCSM_DM_MNGR_EVENT_LAST) return -1;

    switch (evt) {
    case WLCSM_DM_MNGR_EVENT_OBJ_MODIFIED:
        if(to_register)
            g_wlcsm_dm_mngr_handlers[evt].func=wlcsm_dm_handler->dm_object_change_handler;
        else
            g_wlcsm_dm_mngr_handlers[evt].func=NULL;
        break;
    case WLCSM_DM_MNGR_EVENT_STAS_CHANGED:
        if(to_register)
            g_wlcsm_dm_mngr_handlers[evt].func=wlcsm_dm_handler->sta_change_handler;
        else
            g_wlcsm_dm_mngr_handlers[evt].func=NULL;
        break;
    default:
        return -1;
    }				/* -----  end switch  ----- */
    return 0;
}
