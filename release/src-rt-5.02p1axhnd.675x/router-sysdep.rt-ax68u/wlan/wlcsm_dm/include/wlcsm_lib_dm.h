/*************************************************
* <:copyright-BRCM:2013:proprietary:standard
*
*    Copyright (c) 2013 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

/**
 *	@file	 wlcsm_lib_dm.h
 *	@brief	 wlcsm data module header file
 *
 *	wlcsm data module header file,to be included in by all DM implementation and applications
 *	try to use dm. In our implementation, only Wlmngr will need it and 181 implementation will
 *	need to use it.
 *
 *	@date  	10/16/2014 12:39:49 PM
 */

#ifndef __WLCSM_LIB_DM_H__
#define __WLCSM_LIB_DM_H__

/** @defgroup wlcsm_dm Data model component
 *
 *   This module is a layered module to provide abstraction layer to any data module implementations
 *   such as TR181, TR98 etc. All APIs are included in <b>wlcsm_lib_dm.h</b>,for how to develop a customized
 *   datamodule library,please refer to  #WLCSM_CUSTOM_DM_TUTORIAL .
 *   \image html images/wlcsm-dm.png "WLCSM Datamodel Components"
 *
 */

/** \addtogroup wlcsm_dm
 * @{ */
#include <wlcsm_dm_generic.h>
#include <wlcsm_dm_mngr_strmapper.h>

typedef enum sync_direction {
    WL_SYNC_FROM_NVRAM,
    WL_SYNC_FROM_DM,
    WL_SYNC_TO_DM_DIRECTION,
    WL_SYNC_TO_NVRAM,
    WL_SYNC_TO_DM
} SYNC_DIRECTION;

typedef enum {
    WLCSM_DM_QUERY_BRIDGE_INFO,
    WLCSM_DM_QUERY_MNGR_ENTRY,
    WLCSM_DM_QUERY_SETDBGLEVEL,
    WLCSM_DM_QUERY_LOCK,
} WLCSM_DM_QUERY_CMD;


typedef int (*DM_INIT_FUNC_PTR)(void);
typedef int (*DM_OPER_FUNC_PTR)(int idx,int from);
typedef int (*DM_QUERY_FUNC_PTR)(WLCSM_DM_QUERY_CMD cmd,void *buf);


typedef struct {
    unsigned int oid;
    unsigned int offset;
    char *value;
    char *name;
} WLCSM_DM_OBJ_VALUE_SET;


typedef int (*WLCSM_DM_MNGR_EVENT_HANDLER_FUNC)(unsigned int rdio_idx,unsigned int sub_idx,void *);

typedef struct {
    WLCSM_DM_MNGR_EVENT evt;
    WLCSM_DM_MNGR_EVENT_HANDLER_FUNC func;
} WLCSM_DM_MNGR_EVENT_HANDLER;


extern WLCSM_DM_MNGR_EVENT_HANDLER  g_wlcsm_dm_mngr_handlers[];

/*************************************************************//**
 * @brief	dm implementation to register intersted event
 *
 * This API is used for dm implementation to register intersted event,such as STA associated
 * disassociated and obj change etc.
 *
 * @return  	int 0- successful other- failure
 ***************************************************************/
int wlcsm_dm_mngr_event_register(WLCSM_DM_MNGR_EVENT event,WLCSM_DM_MNGR_EVENT_HANDLER_FUNC func);

typedef struct  wlcsm_dm_handler_struct {
    int			b_dm_initialized;
    char*			dm_name;
    DM_INIT_FUNC_PTR 	dm_init;
    DM_INIT_FUNC_PTR 	dm_deinit;
    DM_INIT_FUNC_PTR 	dm_save_nvram;
    DM_OPER_FUNC_PTR 	dm_load;
    DM_OPER_FUNC_PTR 	dm_save;
    DM_QUERY_FUNC_PTR	dm_query_info;
    WLCSM_DM_MNGR_EVENT_HANDLER_FUNC dm_object_change_handler;
    WLCSM_DM_MNGR_EVENT_HANDLER_FUNC sta_change_handler;
    struct wlcsm_dm_handler_struct	*pre;
    struct wlcsm_dm_handler_struct	*next;
} WLCSM_DM_HANDLER_STRUCT;

extern WLCSM_DM_HANDLER_STRUCT  *wlcsm_dm_handler;

extern WLCSM_WLAN_ADAPTER_STRUCT *gp_adapter_objs;
extern WLCSM_WLAN_WIFI_STRUCT g_wifi_obj;

typedef enum {
    WLCSM_DM_MNGR_CMD_GET_SOURCE,
} WLCSM_DM_MNGR_CMD;

typedef void * (*WLCSM_DM_MNGR_CMD_HOOK_FUNC)(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,WLCSM_DM_MNGR_CMD cmd,void *par);

extern WLCSM_DM_MNGR_CMD_HOOK_FUNC  g_wlcsm_dm_mngr_runtime_hook;


#define WL_RADIO(idx) (gp_adapter_objs[(idx)].radio)
#define WL_PHYTYPE(idx) (gp_adapter_objs[(idx)].radio.wlPhyType)
#define WL_RADIO_ENABLE(idx) (gp_adapter_objs[(idx)].radio.enable)
#define WL_RADIO_NMODE(idx) (gp_adapter_objs[(idx)].radio.wlNmode)
#define WL_RADIO_COUNTRY(idx) (gp_adapter_objs[(idx)].radio.wlCountry)
#define WL_RADIO_REGREV(idx) (gp_adapter_objs[(idx)].radio.wlRegRev)
#define WL_RADIO_COUNTRYREV(idx) (gp_adapter_objs[(idx)].radio.wlCountryRev)
#define WL_RADIO_PTR(idx) (&(gp_adapter_objs[(idx)].radio))
#define WL_BSSID(idx,sub) (gp_adapter_objs[(idx)].bssids[(sub)])
#define WL_BSSID_IFNAME(idx,sub) (gp_adapter_objs[(idx)].bssids[(sub)].wlifname)
#define WL_BSSID_BRIDGE_NAME(idx,sub) (gp_adapter_objs[(idx)].bssids[(sub)].wlBrName)
#define WL_BSSID_ENABLE(idx,sub) (gp_adapter_objs[(idx)].bssids[(sub)].enable)
#define WL_BSSID_SSID(idx,sub) (gp_adapter_objs[(idx)].bssids[(sub)].SSID)
#define WL_BSSID_MACADDRESS(idx,sub) (gp_adapter_objs[(idx)].bssids[(sub)].MACAddress)
#define WL_AP(idx,sub) (gp_adapter_objs[(idx)].ssids[(sub)].accesspoint)
#define WL_APSEC(idx,sub) (gp_adapter_objs[(idx)].ssids[(sub)].security)
#define WL_APWPS(idx,sub) (gp_adapter_objs[(idx)].ssids[(sub)].wps)
#define WL_AP_STAS(idx,sub) (gp_adapter_objs[(idx)].ssids[(sub)].associated_stas)

#define MIMO_PHYY(idx) ((!strncmp(gp_adapter_objs[(idx)].radio.wlPhyType, WL_PHY_TYPE_N,strlen(WL_PHY_TYPE_N)+1)) || \
	   					(!strncmp(gp_adapter_objs[(idx)].radio.wlPhyType, WL_PHY_TYPE_AC,strlen(WL_PHY_TYPE_AC)+1)))


/** @brief A function pointerdefine for wlmngr var access lock
 *
 * when doing dm opertation and loading from dm, need to lock var access, but
 * it should have after getting dm lock, otherwize, othen DM applications may
 * request var from wlmngr to cause deadlock
 *
 * */
typedef void (*WLCSM_WLMNGR_VAR_LOCK_FN)(int lock);

/*************************************************************//**
 * @brief  write all configuration to persistant storage
 *
 * @return  void
 ***************************************************************/
int  wlcsm_dm_save_config(int idx,int from,WLCSM_WLMNGR_VAR_LOCK_FN lock_fn);


/*************************************************************//**
 * @brief query the datamodel for bridge configurations
 *
 *dm has bridge configuratoins, this API will query it to
 *get a list of bridges and ports by the format:
 *br0:eth0:wl0 br1:eth2:wl1
 *colon is usde to seperate bridges interfaces and space to
 *seperate bridges
 *
 * @return int
 ***************************************************************/
int wlcsm_dm_get_bridge_info(char *buf);

/*************************************************************//**
 * @brief setdbg level in dm implementations
 *
 * set dbg level in dm implementations
 *
 * @return int
 ***************************************************************/
int wlcsm_dm_set_dbglevel(char  *dbglevel);

char *wlcsm_dm_get_dmname(void);
int wlcsm_dm_cleanup(void);
char *wlcsm_mapper_get_mngr_value(unsigned int idx,unsigned int sub_idx,WLCSM_NVRAM_MNGR_MAPPING *mapping,char *varValue);
int wlcsm_dm_get_bssid_num(int idx);

int  wlcsm_dm_mngr_set_value(int idx,int sub_idx,char *name,char *value,unsigned int oid);
int  wlcsm_dm_mngr_set_all_value(int idx,int sub_idx,char *name,char *value);

char *wlcsm_dm_mngr_get_value(unsigned int idx,unsigned int sub_idx,char *name,char *varValue,unsigned int pos);
char *wlcsm_dm_mngr_get_all_value(unsigned int idx,unsigned int sub_idx,char *name,char*varValue);
int wlcsm_dm_is_runtime_oid(unsigned int oid);
long long _strtoll_(char *str);
long _strtol_(char *str);
int wlcsm_dm_restore_default(int write_nvram);
int wlcsm_dm_structure_restore_default(int idx, int sub_idx, unsigned int pos);
int wlcsm_dm_reg_event(WLCSM_DM_MNGR_EVENT evt,int to_register);
char * wlcsm_strcpy(char **dst,char *from);
int wlcsm_dm_get_mapper_int(int pos,char *name,int nvram,int *ret);
char *wlcsm_dm_get_mapper_str(int pos,int num,int nvram,int *ret);
int wlcsm_nvram_update_runtime_mngr(char *name,char *value);
void wlcsm_dm_register(WLCSM_DM_HANDLER_STRUCT *handler);
int wlcsm_dm_select_dm(char *name,unsigned int idx,WLCSM_WLMNGR_VAR_LOCK_FN lock_fn,int from);
int wlcsm_update_runtime_nvram(unsigned int idx,unsigned int ssid_idx,char *name,char *value);
int wlcsm_nvram_from_kernel(char* name,int done);
WLCSM_NAME_OFFSET *wlcsm_dm_get_mngr_entry(void *hdr,char *varValue,unsigned int *pos);
#define DECLARE_WLCSM_DM_ITEM(name) \
	 WLCSM_DM_HANDLER_STRUCT wlcsm_dm_##name##_handler = {\
	  0, \
	  #name ,\
	  _wlcsm_dm_##name##_init,\
	  _wlcsm_dm_##name##_deinit,\
	  _wlcsm_dm_##name##_save_nvram,\
	  _wlcsm_dm_##name##_load_config,\
	  _wlcsm_dm_##name##_save_config,\
	  _wlcsm_dm_##name##_query_info,\
	  _wlcsm_dm_##name##_obj_change_handler,\
	  _wlcsm_dm_##name##_sta_change_handler,\
	  NULL, \
	  NULL }

#define WLCSM_DM_ITEM(name) \
	(wlcsm_dm_name##_handler)

#define WLCSM_DM_ITEM_PTR(name) \
	&(wlcsm_dm_name##_handler)

/** @brief A macro being used in Apps to declare a datamodel
 *
 * When a customized DM is developed and want to use it, first
 * to declare before calling other macs.
*/

#define WLCSM_DM_DECLARE(name) \
	extern WLCSM_DM_HANDLER_STRUCT wlcsm_dm_##name##_handler

/** @brief A macro being used in Apps to register which datamodel
 *
 * When a customized DM is developed, it can be registered to
 * an application by using this Macro and then use WLCSM_DM_SELECT(name)
 * to choose which DM to use by name
 *
*/
#define DM_HANDLER_FUNC(dm,...) wlcsm_dm_##dm##__VA_ARGS__##_handler
#define WLCSM_DM_REGISTER(name) \
	wlcsm_dm_register(&(DM_HANDLER_FUNC(name)))

/** @brief A macro being used in Apps to select which datamodel
 *
 * In user application, this Macro will select the DM by name registered
 * in the application using WLCSM_DM_REGISTER(name).
 *
 * */
#define DM_NAME(dm) #dm
#define WLCSM_DM_SELECT(name,idx,lock_fn,ret) \
	ret=wlcsm_dm_select_dm(DM_NAME(name),idx,lock_fn,ret)


/** @brief How to develop specific Datamodel for wireless manager
 *
 * This is a tutorial of how to develop a customized data model which can be
 * used with wlcsm datamodel abstraction layer and further provide data to wlmngr.
 *
 * <h2>Familiar with the wlmngr structures</h2>
 *
 * The whole purpose of a customized data module is to map the content being organized in
 * a specific way to the geneneral structure which can be recoginzied by the wlmngr, so it
 * is extremly important to understand the wlmngr structure.  wlmngr global structure is
 * specified in wlcsm_dm_generic.h file.
 *
 * */
#define WLCSM_CUSTOM_DM_TUTORIAL

/** @}*/





#endif
