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
#include "wlcsm_lib_api.h"
#include <bcmconfig.h>
#include <limits.h>

/*! @file  wlcsm_lib_dm.c
 *
 *  Data model abstraction layer implemenation
 *
 *
 * */

WLCSM_WLAN_WIFI_STRUCT g_wifi_obj;
WLCSM_WLAN_ADAPTER_STRUCT *gp_adapter_objs=NULL;
WLCSM_DM_HANDLER_STRUCT  *wlcsm_dm_handler=NULL;
WLCSM_DM_MNGR_CMD_HOOK_FUNC  g_wlcsm_dm_mngr_runtime_hook=NULL;

long _strtol_(char *str) {
    char *endptr;
    long val=0;
    errno=0;
    if(str) {
        val = strtol(str, &endptr,0 );
        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                || (errno != 0 && val == 0) || (endptr == str)) {
            fprintf(stderr, "Str:%s No digits were found, errno:%d\n",str,errno);
            return 0;
        }
    }
    return val;
}

long long _strtoll_(char *str) {
    char *endptr;
    long long val=0;
    errno=0;
    if(str) {
        val = strtol(str, &endptr, 0);
        if ((errno == ERANGE && (val == LLONG_MAX || val == LLONG_MIN))
                || (errno != 0 && val == 0) || (endptr == str)) {
            fprintf(stderr, "Str:%s No digits were found, errno:%d\n",str,errno);
            return 0;
        }
    }
    return val;
}

static inline int _wlcsm_dm_aquire_lock(void) {
    int lock=1;
    return  wlcsm_dm_handler->dm_query_info(WLCSM_DM_QUERY_LOCK,(void *)&lock);
}

static inline int _wlcsm_dm_release_lock(void) {
    return  wlcsm_dm_handler->dm_query_info(WLCSM_DM_QUERY_LOCK,NULL);
}

int wlcsm_dm_structure_restore_default(int idx, int sub_idx, unsigned int pos)
{
    int i=0,size=0,ret=0;
    WLCSM_MNGR_NAME_OFFSET *array;
    char *strval;
    char varValue[WL_MID_SIZE_MAX];
    switch(pos) {
    case WLMNGR_DATA_POS_WIFI_RADIO:
        array=g_WLMNGR_STRUCT_DEV2_WIFI_RADIO;
        size= WLCSM_MNGR_NAME_ENTRY_SIZE(g_WLMNGR_STRUCT_DEV2_WIFI_RADIO);
        break;

    case WLMNGR_DATA_POS_WIFI_SSID:
        array=g_WLMNGR_STRUCT_DEV2_WIFI_SSID;
        size=WLCSM_MNGR_NAME_ENTRY_SIZE(g_WLMNGR_STRUCT_DEV2_WIFI_SSID);
        break;
    case WLMNGR_DATA_POS_WIFI_ACCESS_POINT:
        array= g_WLMNGR_STRUCT_DEV2_WIFI_ACCESS_POINT;
        size= WLCSM_MNGR_NAME_ENTRY_SIZE(g_WLMNGR_STRUCT_DEV2_WIFI_ACCESS_POINT);
        break;
    case WLMNGR_DATA_POS_WIFI_ACCESS_POINT_SECURITY:
        array=g_WLMNGR_STRUCT_DEV2_WIFI_ACCESS_POINT_SECURITY;
        size= WLCSM_MNGR_NAME_ENTRY_SIZE(g_WLMNGR_STRUCT_DEV2_WIFI_ACCESS_POINT_SECURITY);
        break;
    case WLMNGR_DATA_POS_WIFI_ACCESS_POINT_WPS:
        array=g_WLMNGR_STRUCT_DEV2_WIFI_ACCESS_POINT_WPS;
        size= WLCSM_MNGR_NAME_ENTRY_SIZE(g_WLMNGR_STRUCT_DEV2_WIFI_ACCESS_POINT_WPS);
        break;
    default:
        WLCSM_TRACE(WLCSM_TRACE_ERR," WRONG POS!!!%d\r\n",pos);
        return -1;

    }
    for(; i<size; i++) {
        strval=array[i].default_value;
        switch(array[i].type) {
        case WLCSM_DT_STR2INT:
        {
            snprintf(varValue, WL_MID_SIZE_MAX, "%d",wlcsm_dm_get_mapper_int(g_wlcsm_nvram_mngr_mapping[array[i].mapper_index].mapper,strval,0,&ret));
            strval=varValue;
        }
        break;
        case WLCSM_DT_INT2STR:
            strval=wlcsm_dm_get_mapper_str(g_wlcsm_nvram_mngr_mapping[array[i].mapper_index].mapper,(int)_strtol_(strval),0,&ret);
            break;
        case WLCSM_DT_BOOL2STR:
            /**< DM value is bool, but runtime is string */
            snprintf(varValue,WL_MID_SIZE_MAX,"%s",_strtol_(strval)?"true":"false");
            strval=varValue;
            break;
        default:
            break;
        }
        ret=wlcsm_dm_mngr_set_value(idx,sub_idx,array[i].name,strval,pos);
        if(ret) return ret;
    }
    return ret;
}

int wlcsm_nvram_from_kernel(char* name,int done) {
    char temp[512];
    static FILE *fp=NULL;
    char *equalstr;
    unsigned int idx,sub_idx;
    if(done) {
        if(fp) fclose(fp);
        fp=NULL;
        return 0;
    }
    equalstr= strchr(name,'=');
    /* if invalid string,return 1 to ignore */
    if(!equalstr)  {
        return 1;
    }
    if(wlcsm_nvram_name_parser(name,&idx,&sub_idx)) {
        /*if nvram name starts with wlx or wlx.y, it is not
         * nvram kernel */
        return 0;
    }

    if(!fp && (fp = fopen("/data/.kernel_nvram.setting", "r")) == NULL) {
        return 0;
    }

    fseek(fp,0,SEEK_SET);
    while(fgets(temp, 512, fp) != NULL) {
        if(temp[0]=='#') continue;
        if(!strncmp(temp,name,equalstr-name)) {
            return 1;
        }
    }
    return 0;
}

/* restore default, can overwrit nvram or not to */
int wlcsm_dm_restore_default(int write_nvram)
{
    int radio_idx,ssid_idx,idx,ret=0;
    char name[17],value[17];
    for(idx=0; idx<WL_WIFI_RADIO_NUMBER; idx++) {
        radio_idx=idx;
        if(!write_nvram) radio_idx=MARK_IDX(idx);
        ret=wlcsm_dm_structure_restore_default(radio_idx,0,WLMNGR_DATA_POS_WIFI_RADIO);
        if(ret) return ret;
        WL_RADIO_WLNUMBSS(idx)=wlcsm_wl_get_mbss_num(idx);
        for ( ssid_idx=0; ssid_idx<WL_RADIO_WLNUMBSS(idx); ssid_idx++ ) {
            ret = wlcsm_dm_structure_restore_default(radio_idx,ssid_idx,WLMNGR_DATA_POS_WIFI_SSID);
            ret |= wlcsm_dm_structure_restore_default(radio_idx,ssid_idx,WLMNGR_DATA_POS_WIFI_ACCESS_POINT);
            ret |= wlcsm_dm_structure_restore_default(radio_idx,ssid_idx,WLMNGR_DATA_POS_WIFI_ACCESS_POINT_SECURITY);
            ret |= wlcsm_dm_structure_restore_default(radio_idx,ssid_idx,WLMNGR_DATA_POS_WIFI_ACCESS_POINT_WPS);

            if(ssid_idx==0) {
                snprintf(name,16,"wl%d",idx);
                WL_BSSID_WLENBLSSID(idx,ssid_idx)=1;
                /*always set wlx_ifname */
                snprintf(value,16,"wl%d_ifname",idx);
                wlcsm_nvram_set(value,name);
            } else {
                snprintf(name,16,"wl%d.%d",idx,ssid_idx);
                WL_BSSID_WLENBLSSID(idx,ssid_idx)=0;
            }

            wlcsm_strcpy(&(WL_BSSID_IFNAME(idx,ssid_idx)),name);
            wlcsm_strcpy(&(WL_BSSID_BRIDGE_NAME(idx,ssid_idx)),"br0");

            if(ret) return ret;
        }
    }
    return ret;
}

int wlcsm_dm_is_runtime_oid(unsigned int oid)
{
    int i=g_WLMNGR_RUNTIME_OIDS_NUM-1;
    while(i>=0) {
        if(g_WLMNGR_RUNTIME_OIDS[i]==oid) return 1;
    }
    return 0;
}

static inline WLCSM_MNGR_NAME_OFFSET *_wlcsm_dm_mngr_get_mapper(unsigned int pos, unsigned int *num_of_entries)
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
    int len;
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
                *(val++) = '\0';
                /* for back compatibility, to compare value with *DEL* */
                if(strncmp(val,"*DEL*",5))
                    wlcsm_nvram_set(name, val);

            } else {
                printf("pair not patch.[%s]..\n", pair);
                break;
            }
        }
        /* wifi nvram is not useful anymore after this. free it for memory */
        free(g_wifi_obj.nvram);
        g_wifi_obj.nvram=NULL;
    }
}

static inline  void  wlcsm_struct_modifystr_value(void *struct_addr,unsigned int offset,char *value)
{
    char **str_ptr_ptr=(char **)(struct_addr+offset);
    if(value!=*str_ptr_ptr) {
        /* at lest one of them is not NULL */
        if(value && *str_ptr_ptr && !WLCSM_STRCMP(value,*str_ptr_ptr))
            return;
        else if(*str_ptr_ptr)
            free(*str_ptr_ptr); /* free the orignal value */

        if(value) {
            *str_ptr_ptr=malloc(strlen(value)+1);
            if(*str_ptr_ptr) {
                strncpy(*str_ptr_ptr,value,strlen(value)+1);
            } else {
                printf("%s:%d  Allocate memrory error \r\n",__FUNCTION__,__LINE__ );
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

int  wlcsm_dm_load(int idx,int from)
{
	int i=idx,radios;
	if(idx==0)
		_wlcsm_dm_load_nvram();
	if(wlcsm_dm_handler && (!wlcsm_dm_handler->dm_load(idx,from))) {
		/* We fix the max mbss number by using "WL_MAX_NUM_SSID" 
		 * instead of from DM */
		if(idx>0) {
			i=idx-1;
			radios=idx;
		} else {
			i=0;
			radios=WL_WIFI_RADIO_NUMBER;
		}
		for(;i<radios; i++) {
			WL_RADIO_WLNUMBSS(i) = wlcsm_wl_get_mbss_num(i);
			WL_RADIO_WLMAXMBSS(i) = WL_RADIO_WLNUMBSS(i);
		}
		return 0;
	} 
	return 1;
}

int  wlcsm_dm_save_config(int idx,int from,WLCSM_WLMNGR_VAR_LOCK_FN lock_fn)
{
    int ret=0;
    if(wlcsm_dm_handler) {
        ret=_wlcsm_dm_aquire_lock();
        if(!ret) {
            if(lock_fn) lock_fn(1);
            ret=wlcsm_dm_handler->dm_save(idx,from);
            if(!ret) {
                ret=wlcsm_dm_handler->dm_save_nvram();
                if(g_wifi_obj.nvram) {
                    free(g_wifi_obj.nvram);
                    g_wifi_obj.nvram=NULL;
                }
            }
            if(lock_fn) lock_fn(0);
            _wlcsm_dm_release_lock();
        }
        return ret;
    }
    else
        return 1;
}

int wlcsm_dm_mngr_set_value(int idx,int sub_idx,char *name,char *value,unsigned int pos)
{
    unsigned int num_of_entry=0;
    int i=0,ret;
    int offset=0;
    char *src=NULL;
    int set=1;
    int dm_set=WLCSM_MNGR_CMD_GET_DMOPER(idx);
    WLCSM_DM_TYPE data_type;
    WLCSM_MNGR_NAME_OFFSET *obj_mapper= NULL;
    int to_update_nvram = !IS_IDX_MARKED(idx);
    idx=WLCSM_MNGR_CMD_GET_IDX(idx);
    idx=REAL_IDX(idx);

    if((obj_mapper=_wlcsm_dm_mngr_get_mapper(pos,&num_of_entry)) &&
            (src=_wlcsm_dm_mngr_get_src(idx,sub_idx,pos))) {

        for ( i=0; i<num_of_entry; i++ ) {
            if(!WLCSM_STRCASECMP(obj_mapper[i].name,name)) {
                data_type=obj_mapper[i].type;
                offset=obj_mapper[i].offset;

                switch(data_type) {
                case WLCSM_DT_UINT:
                    NUMVAR_VALUE(src,offset,UINT32)=(UINT32)_strtol_(value);
                    break;
                case WLCSM_DT_STR2INT:
                    if(dm_set) {
                        /* input is DM value string, this need to map to int */
                        NUMVAR_VALUE(src,offset,SINT32)=
                            wlcsm_dm_get_mapper_int(g_wlcsm_nvram_mngr_mapping[obj_mapper[i].mapper_index].mapper,value,0,&ret);
                        break;
                    }
                /*else pass through to SINT32*/
                case WLCSM_DT_SINT32:
                    NUMVAR_VALUE(src,offset,SINT32)=(SINT32)_strtol_(value);
                    break;
                case WLCSM_DT_BOOLREV:
                case WLCSM_DT_BOOL:
                    NUMVAR_VALUE(src,offset,UBOOL8)=(SINT32)_strtol_(value);
                    break;
                case WLCSM_DT_UINT64:
                    NUMVAR_VALUE(src,offset,UINT64)=(UINT64)_strtoll_(value);
                    break;
                case WLCSM_DT_SINT64:
                    NUMVAR_VALUE(src,offset,SINT64)=(SINT64)_strtoll_(value);
                    break;
                case WLCSM_DT_INT2STR:
                    if(dm_set) {
                        /*incoming is integer and need to first change to str */
                        char *strval=wlcsm_dm_get_mapper_str(g_wlcsm_nvram_mngr_mapping[obj_mapper[i].mapper_index].mapper,(int)_strtol_(value),0,&ret);
                        if(strval)
                            wlcsm_struct_modifystr_value(src,offset,strval);
                        break;
                    }
                /*else pass through */
                case WLCSM_DT_STRING:
                case WLCSM_DT_BASE64:
                case WLCSM_DT_HEXBINARY:
                case WLCSM_DT_DATETIME:
                case WLCSM_DT_BOOL2STR:
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
                    value_set.name=name;
                    g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_OBJ_MODIFIED].func(idx,sub_idx,&value_set);
                }
                /* to update the nvram after setting mngr vars */
                if(set && to_update_nvram) {
                    wlcsm_update_runtime_nvram(idx,sub_idx,name,value);
                }

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
    int index=0,ret=0;
    char *nv_name=wlcsm_nvram_name_parser(name,&idx,&ssid_idx);
    /* the idx has to be in the range */
    if((idx<WL_WIFI_RADIO_NUMBER) && (ssid_idx<WL_RADIO_WLNUMBSS(idx)) && (nv_name)) {
        for ( index=0; index<entries_num; index++) {
            nvram_var= g_wlcsm_nvram_mngr_mapping[index].nvram_var;
            mngr_var= g_wlcsm_nvram_mngr_mapping[index].mngr_var;
            if(!WLCSM_STRCMP(nv_name,nvram_var)) {

                switch(g_wlcsm_nvram_mngr_mapping[index].data_type) {

                case WLCSM_DT_BOOL2STR:
                case WLCSM_DT_INT2STR: {
                    char tempValue[32];
                    /* mngr is int and nvram is str */
                    snprintf(tempValue, sizeof(tempValue), "%d",wlcsm_dm_get_mapper_int(g_wlcsm_nvram_mngr_mapping[index].mapper,value,1,&ret));
                    nvram_var=tempValue;
                    break;
                }
                case WLCSM_DT_STR2INT: {
                    /* mngr is str and nvram is number */
                    if(value) {
                        nvram_var=wlcsm_dm_get_mapper_str(g_wlcsm_nvram_mngr_mapping[index].mapper,(int)_strtol_(value),0,&ret);
                    } else
                        nvram_var=NULL;
                    break;
                }
                default:
                    nvram_var=value;
                }
                if(!ret)
                    wlcsm_dm_mngr_set_all_value(MARK_IDX(idx),ssid_idx,mngr_var,nvram_var);
                else
                    fprintf(stderr, "%s:%d: %s's value:%s is invalide \n",__FUNCTION__,__LINE__,name,value);
                return 0;
            }
        }
    }
    return -1;
}

int wlcsm_update_runtime_nvram(unsigned int idx,unsigned int ssid_idx,char *name,char *value)
{
    int entries_num=sizeof(g_wlcsm_nvram_mngr_mapping)/sizeof(WLCSM_NVRAM_MNGR_MAPPING);
    char *nvram_var,*mngr_var;
    int index=0,ret=0;
    for ( index=0; index<entries_num; index++) {
        nvram_var= g_wlcsm_nvram_mngr_mapping[index].nvram_var;
        mngr_var= g_wlcsm_nvram_mngr_mapping[index].mngr_var;
        if(!WLCSM_STRCASECMP(name,mngr_var)) {
            /* has nvram entry for this name, we will update nvram now */
            char fullname[128];
            char tempValue[32];
            if(!ssid_idx)
                snprintf(fullname, sizeof(fullname), "wl%d_%s",idx,nvram_var);
            else
                snprintf(fullname, sizeof(fullname), "wl%d.%d_%s",idx,ssid_idx,nvram_var);

            switch(g_wlcsm_nvram_mngr_mapping[index].data_type) {

            case WLCSM_DT_STR2INT:
                /* mngr is str and nvram is number */
                snprintf(tempValue, sizeof(tempValue), "%d",wlcsm_dm_get_mapper_int(g_wlcsm_nvram_mngr_mapping[index].mapper,value,0,&ret));
                nvram_var=tempValue;
                break;
            case WLCSM_DT_INT2STR:
            case WLCSM_DT_BOOL2STR: {
                /* mngr is num and nvram is string */
                nvram_var=wlcsm_dm_get_mapper_str(g_wlcsm_nvram_mngr_mapping[index].mapper,(int)_strtol_(value),1,&ret);
                break;
            }
            default:
                nvram_var=value;
            }
            if(!ret)
                wlcsm_nvram_set(fullname,nvram_var);
            else
                fprintf(stderr, "%s:%d: %s's value:%s is invalide \n",__FUNCTION__,__LINE__,name,value);

            return 0;
        }
    }
    return 1;
}



char *wlcsm_dm_mngr_get_value(unsigned int idx,unsigned int sub_idx,char *name,char *varValue,unsigned int pos)
{

    unsigned int num_of_entry=0;
    int i=0,ret=0;
    int offset=0;
    char *src=NULL,*tempstr;
    int dm_get=WLCSM_MNGR_CMD_GET_DMOPER(idx);
    WLCSM_DM_TYPE data_type;
    WLCSM_MNGR_NAME_OFFSET *obj_mapper= NULL;
    idx=WLCSM_MNGR_CMD_GET_IDX(idx);

    if((obj_mapper=_wlcsm_dm_mngr_get_mapper(pos,&num_of_entry)) &&
            (src=_wlcsm_dm_mngr_get_src(idx,sub_idx,pos))) {

        //if(obj_mapper && src) {

        for ( i=0; i<num_of_entry; i++ ) {
            if(!WLCSM_STRCASECMP(obj_mapper[i].name,name)) {
                //WLCSM_TRACE(WLCSM_TRACE_DM," found in mapper  and src:%p, obj_mapper:%p\r\n",src, obj_mapper);

                data_type=obj_mapper[i].type;
                offset=obj_mapper[i].offset;

                switch(data_type) {
                case WLCSM_DT_UINT: {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%u", NUMVAR_VALUE(src,offset,UINT32));
                    break;
                }
                case WLCSM_DT_STR2INT: {
                    if(dm_get) {
                        tempstr= wlcsm_dm_get_mapper_str(g_wlcsm_nvram_mngr_mapping[obj_mapper[i].mapper_index].mapper,NUMVAR_VALUE(src,offset,SINT32),0,&ret);
                        if(!ret)
                            snprintf(varValue, WL_MID_SIZE_MAX, "%s",tempstr);
                        else {
                            fprintf(stderr, "Could not get string value from int in mapper\n");
                            return NULL;
                        }
                    }
                    else
                        snprintf(varValue, WL_MID_SIZE_MAX, "%d", NUMVAR_VALUE(src,offset,SINT32));
                    break;
                }
                case WLCSM_DT_SINT32: {
                    snprintf(varValue, WL_MID_SIZE_MAX, "%d", NUMVAR_VALUE(src,offset,SINT32));
                    break;
                }
                case WLCSM_DT_UINT64:
                    snprintf(varValue, WL_MID_SIZE_MAX, "%" PRIu64, NUMVAR_VALUE(src,offset,UINT64));
                    break;
                case WLCSM_DT_SINT64:
                    snprintf(varValue, WL_MID_SIZE_MAX, "%" PRId64,  NUMVAR_VALUE(src,offset,SINT64));
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
                    if( dm_get && data_type==WLCSM_DT_INT2STR) {
                        snprintf(varValue, WL_MID_SIZE_MAX, "%d",
                                 wlcsm_dm_get_mapper_int(g_wlcsm_nvram_mngr_mapping[obj_mapper[i].mapper_index].mapper,varValue,0,&ret));
                    }
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
    WLCSM_TRACE(WLCSM_TRACE_ERR," Failed to get value for:%s, should check runtime?  \r\n",name );
    return NULL;
}

char *wlcsm_mapper_get_mngr_value(unsigned int idx,unsigned int sub_idx,WLCSM_NVRAM_MNGR_MAPPING *mapping,char *varValue)
{

    WLCSM_MNGR_VAR_POS pos=mapping->pos;
    int offset=mapping->mngr_var_offset;
    WLCSM_DM_TYPE data_type= mapping->data_type;
    int ret=0;

    WLCSM_WLAN_ADAPTER_STRUCT *adapter;
    char *src=NULL;
    adapter=&(gp_adapter_objs[idx]);
    switch (pos) {
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
        snprintf(varValue, WL_MID_SIZE_MAX, "%" PRIu64, NUMVAR_VALUE(src,offset,UINT64));
        break;
    case WLCSM_DT_SINT64:
        snprintf(varValue, WL_MID_SIZE_MAX, "%" PRIu64,  NUMVAR_VALUE(src,offset,SINT64));
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
            snprintf(varValue, WL_MID_SIZE_MAX, "%d",wlcsm_dm_get_mapper_int(mapping->mapper,*p_str_pptr,0,&ret));
            if(ret) {
                varValue=NULL;
                fprintf(stderr, "%s:%d: %s has no mapper int value for it\n",__FUNCTION__,__LINE__,*p_str_pptr);
            }
        } else
            return NULL;
    }
    break;
    case WLCSM_DT_INT2STR: {

        char *tmp_str_ptr=wlcsm_dm_get_mapper_str(mapping->mapper,NUMVAR_VALUE(src,offset,SINT32),1,&ret);
        if(tmp_str_ptr && !ret)
            snprintf(varValue, WL_MID_SIZE_MAX, "%s",tmp_str_ptr);
        break;
    }
    case WLCSM_DT_BOOL2STR: {
        char *tmp_str_ptr=wlcsm_dm_get_mapper_str(mapping->mapper,NUMVAR_VALUE(src,offset,UBOOL8),1,&ret);
        if(tmp_str_ptr && !ret)
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
    if(adapter->radio.wlNumBss>WL_MAX_NUM_SSID) {
        WLCSM_TRACE(WLCSM_TRACE_ERR," !!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!! number of BSS id not right:%d \r\n",adapter->radio.wlNumBss );
        return 1;
    } else
        return adapter->radio.wlNumBss;
}

static int _wlcsm_dm_free_gp_adatper(void)
{
    if(gp_adapter_objs) {
        WLCSM_TRACE(WLCSM_TRACE_DM,"TODO: free all resources allocated for gp_adapter \r\n" );
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

int wlcsm_dm_set_dbglevel(char *dbglevel)
{
    if(wlcsm_dm_handler) {
        return  wlcsm_dm_handler->dm_query_info(WLCSM_DM_QUERY_SETDBGLEVEL,dbglevel);
    }
    else
        return -1;
}

WLCSM_NAME_OFFSET *wlcsm_dm_get_mngr_entry(void  *phdr,char *varValue,unsigned int *pos)
{
    int ret=0;
    unsigned int nums[32];
    t_WLCSM_MNGR_VARHDR  *hdr=(t_WLCSM_MNGR_VARHDR *)phdr;
    WLCSM_NAME_OFFSET **name_offset=(WLCSM_NAME_OFFSET **)nums;
    if(wlcsm_dm_handler) {
        nums[0]=hdr->dm_oid;
        nums[1]=hdr->offset;
        ret=wlcsm_dm_handler->dm_query_info(WLCSM_DM_QUERY_MNGR_ENTRY,nums);

        WLCSM_TRACE(WLCSM_TRACE_DM," POS is :%d \r\n",ret );
        if(ret >0) {
            WLCSM_TRACE(WLCSM_TRACE_DM," return POS :%s\r\n",(*name_offset)->name );
            *pos=(unsigned int)ret;
            return *name_offset;
        } else {
            WLCSM_TRACE(WLCSM_TRACE_DM," pos is not good \r\n" );
            return NULL;

        }
    } else
        return NULL;
}

static void _wlcsm_dm_default_nvram()
{
    WLCSM_TRACE(WLCSM_TRACE_DBG,"TODO:to restore all default values\r\n");
	wlcsm_nvram_set("ure_disable","1");	
#ifdef __CONFIG_MFP__
    wlcsm_nvram_set("wl_mfp","-1");
#endif
}

__attribute__ ((constructor))
static void __wlcsm_dm_preinit(void)
{
    WL_WIFI_RADIO_NUMBER= wlcsm_wl_get_adapter_num();
    WL_WIFI_BSSID_NUMBER=0;
    // int i=0;//,num_of_bssids;
    int i=0;
    WLCSM_TRACE(WLCSM_TRACE_DBG,"!!!!!!!!!!!!!!! DM PRE_INI!!!!!!!!!!!!!!!!!!!\r\n");

    for(i=0; i<WL_WIFI_RADIO_NUMBER; i++)
        WL_WIFI_BSSID_NUMBER += wlcsm_wl_get_mbss_num(i);

    WL_WIFI_SSID_NUMBER=WL_WIFI_BSSID_NUMBER;


    WLCSM_TRACE(WLCSM_TRACE_DM," g_wifi_obj bssid_number:%d \r\n",WL_WIFI_BSSID_NUMBER );

    return ;

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
    int num_of_bssids;

    if(!wlcsm_dm_handler) {
        WLCSM_TRACE(WLCSM_TRACE_ERR,"!!! SHOULD SELECT A DM BEFORE INIT  \r\n" );
        return -1;
    } else if(wlcsm_dm_handler->b_dm_initialized)  {
        return 0;
    } else {
        /* calling dm module real initialization to init global wifi and then  */
        if(!wlcsm_dm_handler->dm_init())
            wlcsm_dm_handler->b_dm_initialized=1;
        else {
            WLCSM_TRACE(WLCSM_TRACE_ERR,"!!! DM init failure \r\n" );
            return -1;
        }
    }
    WL_WIFI_RADIO_NUMBER= wlcsm_wl_get_adapter_num();
    WL_WIFI_BSSID_NUMBER=0;
    for(i=0; i<WL_WIFI_RADIO_NUMBER; i++)
        WL_WIFI_BSSID_NUMBER += wlcsm_wl_get_mbss_num(i);
    WL_WIFI_SSID_NUMBER=WL_WIFI_BSSID_NUMBER;

    WLCSM_TRACE(WLCSM_TRACE_DM," g_wifi_obj bssid_number:%d \r\n",WL_WIFI_BSSID_NUMBER );

    /* allocated global unique data structure for management if not yet allocated */
    if(!gp_adapter_objs) {

        if((gp_adapter_objs = malloc( sizeof(WLCSM_WLAN_ADAPTER_STRUCT) * WL_WIFI_RADIO_NUMBER))!=NULL) {
            memset(gp_adapter_objs, 0, sizeof(WLCSM_WLAN_ADAPTER_STRUCT) * WL_WIFI_RADIO_NUMBER);
            g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_STAS_CHANGED].func=wlcsm_dm_handler->sta_change_handler;
            g_wlcsm_dm_mngr_handlers[WLCSM_DM_MNGR_EVENT_OBJ_MODIFIED].func=wlcsm_dm_handler->dm_object_change_handler;
            /* now to allocate space for bssid and ssid */
            for(i=0; i<WL_WIFI_RADIO_NUMBER; i++) {
                num_of_bssids=wlcsm_wl_get_mbss_num(i);
                WL_RADIO_WLNUMBSS(i)=num_of_bssids;
                if(!( gp_adapter_objs[i].bssids=malloc(num_of_bssids* sizeof(WLCSM_WLAN_BSSID_STRUCT))) ||
                        !(gp_adapter_objs[i].ssids=malloc(num_of_bssids* sizeof(WLCSM_WLAN_ACCESSPOINT_STRUCT))))
                    goto error;
                memset(gp_adapter_objs[i].bssids,0,num_of_bssids*sizeof(WLCSM_WLAN_BSSID_STRUCT));
                memset(gp_adapter_objs[i].ssids,0,num_of_bssids*sizeof(WLCSM_WLAN_ACCESSPOINT_STRUCT));
            }
            _wlcsm_dm_default_nvram();
            /* first init all structures with default does not matter if there is default or not */
            wlcsm_dm_restore_default(1);
            return 0;
        }
    }
error:
    WLCSM_TRACE(WLCSM_TRACE_ERR," allocated memory for global structure failure  \r\n" );
    wlcsm_dm_cleanup();
    return -1;
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


int wlcsm_dm_get_mapper_int(int pos,char *name,int nvram,int* ret)
{
    unsigned int index=pos>>_WLCSM_MNGR_STRMAPPER_SHIFT;
    unsigned int number=pos&((1<<_WLCSM_MNGR_STRMAPPER_SHIFT)-1);
    WLCSM_MNGR_STRMAPPER_SET *mapper;
    int i=0;
    number+=index;
    for(i=index; i<number; i++) {
        mapper=&g_wlcsm_mngr_strmapper[i];
        if((nvram && mapper->nvram_str_value && (!WLCSM_STRCASECMP(mapper->nvram_str_value,name))) ||
                ((!WLCSM_STRCASECMP(mapper->str_value,name)))) {
            *ret=0;
            return mapper->int_value;
        }
    }
    *ret=1;
    return 0;
}

char *wlcsm_dm_get_mapper_str(int pos,int num,int nvram_value, int *ret)
{
    unsigned int index=pos>>_WLCSM_MNGR_STRMAPPER_SHIFT;
    unsigned int number=pos&((1<<_WLCSM_MNGR_STRMAPPER_SHIFT)-1);
    WLCSM_MNGR_STRMAPPER_SET *mapper;
    int i=0;
    number+=index;
    for(i=index; i<number; i++) {
        mapper=&g_wlcsm_mngr_strmapper[i];
        if(mapper->int_value==num) {
            *ret=0;
            if(nvram_value && mapper->nvram_str_value)
                return mapper->nvram_str_value;
            else
                return mapper->str_value;
        }
    }
    *ret=1;
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

int wlcsm_dm_select_dm(char *name,unsigned int idx,WLCSM_WLMNGR_VAR_LOCK_FN lock_fn,int from)
{
    /* rewind to the top */
    WLCSM_DM_HANDLER_STRUCT *tempHandler=wlcsm_dm_handler;
    int ret=0;
    while(tempHandler && tempHandler->pre) tempHandler = tempHandler->pre;
    do {
        if(!WLCSM_STRCMP(tempHandler->dm_name,name)) {
            /* found the dm, quite successfully */
            if(wlcsm_dm_handler!=tempHandler) _wlcsm_dm_deinit();
            wlcsm_dm_handler=tempHandler;
            if(!_wlcsm_dm_init()) {
                ret=_wlcsm_dm_aquire_lock();
                if(!ret) {
                    if(lock_fn) lock_fn(1);
                    ret=wlcsm_dm_load(idx,from);
                    if(lock_fn) lock_fn(0);
                    _wlcsm_dm_release_lock();
                }
                return ret;
            }
            else {
                WLCSM_TRACE(WLCSM_TRACE_DM," DM INIT PROBLEM!!! \r\n" );
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
