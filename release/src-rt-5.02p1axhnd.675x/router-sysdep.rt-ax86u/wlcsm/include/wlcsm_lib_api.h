/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
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
 *
 ************************************************************************/

/** @defgroup wlcsm_nvram  wlcsm module
 *
 *     This module is a generic library being used by any user space application
 *     to retrieve/set the following wireless related information, and source code
 *     is located under <b>/userspace/private/libs/wlcsm</b>
 *     \image html images/wlcsm_functions.png "WLCSM nvram module functions"
 *
 *     -# nvram varaibles
 *
 *        wireless driver and user space application such as wlconf will mostly depends
 *        on nvram values to do wireless configuration, setting/getting right nvram value
 *        is the most important part to correctly configure wireles.
 *
 *     -# wireless configuration generic variables
 *
 *        This is the major part of the new wlmngr implemention, all user space application
 *        depends on wlcsm module to request wl configuration related information as it is
 *        centrally managed by wlmngr with the new design.
 *
 *     -# wireless misc information
 *
 *        this part provides a few generic wireless driver related information, so far
 *        the implemention is very limited, it can be extended to support any kind of iovar
 *        and ioctl etc.. request to wireless driver.
 *
 */


/**  @mainpage Broadcom Wireless Configuration System Module
 *
 *   @section Introduction
 *
 *   This document covers Broadcom WLAN Configuration and System
 *   Module(WLCSM). It includes two major mdoueles:
 *
 *   - \ref wlcsm_nvram "wlcsm components"
 *
 * 	This module is a generic library being used by any user space application to
 * 	retrieve/set nvram and any wireless related information. It is also the central
 * 	messaging system for WLCSM. This module implements netlink socket for applications
 * 	to communicate to/from kernel wlcsm module and wlmngr.
 *
 * 	The source code of this moduel inlcude two parts:
 *      <ul>
 *		<li> Kernel Module. This part is a separated kernel module(<b>wlcsm.ko</b>)
 *		<li> Kernel source is located under <b>/bcmdrivers/broadcom/net/wl/impl20/wl/wlcsm_ext</b>
 *		<li> Library module. This part is to be linked by applications for communicate with each other.(<b>libwlcsm.so</b>)
 *		<li> Library source is located under <b>/userspace/private/libs/wlcsm</b>
 *	</ul>
 *	<br/>
 *
 *   - \ref wlcsm_dm "wlcsm data model components"
 *
 * 	This module is a designed to provide data model support for wlmngr,which in turn
 * 	controls the behavior of the wireless adapters on the device by the settings in the
 * 	data model. Its primary goal is to provide an abstracted layer for wlmngr to support
 * 	any kind of data models without changing the implementation of wlmngr.  This moduel
 * 	builds on wlcsm module, more specically depends on wlcsm modules' netlink socket to
 * 	communicate between wlmngr and other applications.
 *
 * 	The source code of this moduel is located in private libary directory with name wlcsm.
 * 	<b>/userspace/private/apps/wlan/wlcsm_dm</b>
 *
 *  @file wlcsm_lib_api.h
 *  @brief shared wlcsm header file for API
 *
 *  this header file includes all general functions and structure definations
 *  used for this wlcsm module.
 *
 * */

#ifndef __WLCSM_API_H__
#define __WLCSM_API_H__
/**
 * \addtogroup wlcsm_nvram
 * @{
 * */
#include <sys/types.h>
#include <unistd.h>
#include <wlcsm_defs.h>
#include <wlcsm_lib_wl.h>
#include <wlcsm_lib_hspot.h>
#include <wlcsm_linux.h>
#include <stdio.h>
#define WLCSM_MNGR_VARNAME_MAX (256)
#define MACSTR_LEN (18)

#define MARK_IDX(idx) (idx|0x80)
#define REAL_IDX(idx) (idx&0x0f)
#define IS_IDX_MARKED(idx) (idx&0x80)

/** @brief NVRAM variable types enum */
typedef enum t_wlcsm_nvram_var_type {

    WLCSM_NVRAM_VAR_INT,                        /**< varible is integer value */
    WLCSM_NVRAM_VAR_STRING                     /**< variable is string value */

} t_WLCSM_NVRAM_VAR_TYPE;

/** @brief NVRAM variable struct to tell the name and type */
typedef struct t_wlcsm_nvram_var {

    char *name;                      /**< variable name */
    int type;                        /**< varaible type */
    char *value;

} t_WLCSM_NVRAM_VAR;


/** NOTE, THE ORDER OF THE COMMANDS SHOULD BE COHERENT TO THE HANDLER ARRAY
* ASSIGNEMENT
* ~~~
*    static MNGR_CMD_HANDLER g_mngr_cmd_handlers[]= {
*            _wlmngr_handle_wlcsm_cmd_restart,
*    };
* ~~~
*  so far there is only one command
**/

typedef enum {
    WLCSM_MNGR_CMD_VALIDATE_DM_VAR=1, 	/**< applicaiton request wlmngr to validate dm var which in oid object*/
    WLCSM_MNGR_CMD_GET_DM_VAR, 	/**< applicaiton request wlmngr to get dm var which in oid object*/
    WLCSM_MNGR_CMD_SET_DM_VAR, 	/**< applicaiton request wlmngr to set dm var which in oid object*/
    WLCSM_MNGR_CMD_UPDATE_STALIST, 	/**< applicaiton request wlmngr to update it STA lists- from wlevt*/

    WLCSM_MNGR_CMD_LOCK_DELIMITER,  /**< delimiter to seperate cmd needs lock in setvar or not */
    WLCSM_MNGR_CMD_RESTART, 		/**< applicaiton request wlmngr to restart */

    WLCSM_MNGR_CMD_REG_DM_EVENT, 	/**< register/deregister event to dm*/
    WLCSM_MNGR_CMD_DMSETDBGLEVEL, 	/**< dm set debug level*/
    WLCSM_MNGR_CMD_NETHOTPLUG, 	/**< NETwork hotplug event*/
    WLCSM_MNGR_CMD_HALT_RESTART, 	/**< halt restart until it is commanded to resume*/
    WLCSM_MNGR_CMD_RESUME_RESTART, 	/**< resume normal wl restart routine */
    WLCSM_MNGR_CMD_LAST,
} WLCSM_MNGR_CMD;

/** @brief holder structure for a few nvram variables which share the same prefix.  */

typedef struct t_wlcsm_nvram_names_set {
    char prefix[32];
    struct t_wlcsm_nvram_var vars[];
} t_WLCSM_NVRAM_NAMES_SET;


/** @brief CSM events which can fired to invoking application
 *
 * CSM implements notification mechanism to have invoking applice be aware
 * of interesting events which it may has to response.
 *
 */

typedef enum t_wlcsm_event_type {

    /** @brief event triggered when nvram item set or unset
     *
     *  the event handler for this will include three arguments:
     *    ___
     *    1. arg1: char * name
     *    2. arg2: char * value to be changed [NULL when unset]
     *    3. arg3: char * oldvalue
     */
    WLCSM_EVT_NVRAM_CHANGED,
    /** @brief  event when nvram is commited
     *
     * the event handler's three arguments will all be NULL
     * as no any value is required for this event.
     *
     * */
    WLCSM_EVT_NVRAM_COMMITTED,
    WLCSM_EVT_LAST

} t_WLCSM_EVENT_TYPE;

/** following STA event is from common/include/proto/bcmevent.h */

#define WLC_E_DEAUTH            5       /**<  802.11 DEAUTH request */
#define WLC_E_DEAUTH_IND        6       /**<  802.11 DEAUTH indication */
#define WLC_E_ASSOC             7       /**<  802.11 ASSOC request */
#define WLC_E_ASSOC_IND         8       /**<  802.11 ASSOC indication */
#define WLC_E_REASSOC           9       /**<  802.11 REASSOC request */
#define WLC_E_REASSOC_IND       10      /**<  802.11 REASSOC indication */
#define WLC_E_DISASSOC          11      /**<  802.11 DISASSOC request */
#define WLC_E_DISASSOC_IND      12      /**<  802.11 DISASSOC indication */

typedef struct {
    unsigned char wlEnbl;
    char wlSsid[35];
    unsigned char wlHide;
    unsigned char wlIsolation;
    unsigned char wlDisableWme;
    unsigned char wlWme;
    unsigned char wlEnableWmf;
    unsigned char wmfSupported;
    unsigned char wlEnableHspot;
    char  bssid[18];
    int  max_clients;
} WL_BSSID_SUMMARY;

typedef struct {
    int  num_of_bssid;
    int  wlSupportHspot;
    WL_BSSID_SUMMARY bssid_summary[];
} WL_BSSID_SUMMARIES;


#define STA_DISCONNECTED (0)
#define STA_CONNECTED (1)
#define UPDATE_STA_ALL (2)
#define UPDATE_STA_REMOVE_ALL (3)

typedef struct {
    int type;
    int  num_of_stas;
    int radioIdx;
    int ssidIdx;
    WL_STATION_LIST_ENTRY stalist_summary[];
} WL_STALIST_SUMMARIES;

typedef struct {
    char mac[WL_MID_SIZE_MAX];
    char ssid[WL_SSID_SIZE_MAX];
} WL_WDSAP_LIST_ENTRY, *PWL_WDSAP_LIST_ENTRY;

typedef struct {
    int  num_of_aps;
    WL_WDSAP_LIST_ENTRY wdsaplist_summary[];
} WL_WDSAPLIST_SUMMARIES;


typedef struct {
    int event_type;
    unsigned char mac[MACSTR_LEN]; /* xx:xx:xx:xx:xx:xx */
} WL_STA_EVENT_DETAIL;

#define WLCSM_MNGR_RESTART_SAVEDM    (1)
#define WLCSM_MNGR_RESTART_NOSAVEDM  (0)

typedef enum {
    WLCSM_MNGR_RESTART_HTTPD,   /**< restart coming from HTTPD */
    WLCSM_MNGR_RESTART_NVRAM, 	/**< restart coming from NVRAM */
    WLCSM_MNGR_RESTART_FROM_MDM, /**< beyond this are MDM changed for restart */
    WLCSM_MNGR_RESTART_TR69C,	/**< restart coming from TR69C */
    WLCSM_MNGR_RESTART_MDM,	/**< restart coming from  MDM */
    WLCSM_MNGR_RESTART_MDM_ALL,	/**< restart coming from MDM and restart all */
} WLCMS_MNGR_RESTART_ENUM;


/** DM related event   */
typedef enum {
    /*  NOTE: have to start from 0, DO NOT CHANGE IT!!! */
    WLCSM_DM_MNGR_EVENT_STAS_CHANGED=0, /**<  when associated device number changes */
    WLCSM_DM_MNGR_EVENT_OBJ_MODIFIED, /**< when any parameter get changed */
    WLCSM_DM_MNGR_EVENT_LAST
} WLCSM_DM_MNGR_EVENT;

/* sub_idx format whne it is in cmd [0-7]sub_idx cmd[8-15]command, [16-23]source[optional] */
#define WLCSM_MNGR_CMD_GET_IDX(idx)                            ((idx)&0xff)
#define WLCSM_MNGR_CMD_SET_IDX(target,idx)                      (target) = (((target)&(~0xff))|idx)

#define WLCSM_MNGR_CMD_GET_CMD(idx)                             ((idx>>8)&0xff)
#define WLCSM_MNGR_CMD_SET_CMD(target,idx)                      (target) = (((target)&(~0xff00))|(idx<<8))

#define WLCSM_MNGR_CMD_GET_SOURCE(idx)                          ((idx>>16)&0xff)
#define WLCSM_MNGR_CMD_SET_SOURCE(target,idx)                   (target) = (((target)&(~0xff0000))|(idx<<16))

#define WLCSM_MNGR_CMD_GET_WAIT(idx)                            (idx &0x1000000)
#define WLCSM_MNGR_CMD_SET_WAIT(target)                         (target) = ((target)|(1<<24))

#define WLCSM_MNGR_CMD_GET_SAVEDM(idx)                          (idx &0x2000000)
#define WLCSM_MNGR_CMD_SET_SAVEDM(target)                       (target) = ((target)|(1<<25))

#define WLCSM_MNGR_CMD_GET_DMOPER(idx)                          (idx &0x4000000)
#define WLCSM_MNGR_CMD_SET_DMOPER(target)                       (target) = ((target)|(1<<26))

#define for_each(word, wordlist, next) \
    for (next = &wordlist[strspn(wordlist, " ")], \
         strncpy(word, next, sizeof(word)), \
         word[strcspn(word, " ")] = '\0', \
         word[sizeof(word) - 1] = '\0', \
         next = strchr(next, ' '); \
         strlen(word); \
         next = next ? &next[strspn(next, " ")] : (char*)"", \
         strncpy(word, next, sizeof(word)), \
         word[strcspn(word, " ")] = '\0', \
         word[sizeof(word) - 1] = '\0', \
         next = strchr(next, ' '))

#define foreachcolon(word, wordlist, next) \
    for (next = &wordlist[strspn(wordlist, ":")], \
         strncpy(word, next, sizeof(word)), \
         word[strcspn(word, ":")] = '\0', \
         word[sizeof(word) - 1] = '\0', \
         next = strchr(next, ':'); \
         strlen(word); \
         next = next ? &next[strspn(next, ":")] : (char*)"", \
         strncpy(word, next, sizeof(word)), \
         word[strcspn(word, ":")] = '\0', \
         word[sizeof(word) - 1] = '\0', \
         next = strchr(next, ':'))
/** @brief event hook function prototype for specific event
 *
 * For all specific event, invoking function can define hook function to
 * respond to certain event, the functions always carry three arguments regardless
 * of event type. Check each event for the meaning of reach arguments.
 *
 */
typedef void (*WLCSM_EVENT_HOOK_FUNC)(char *arg1,char *arg2,char* arg3);

/** @brief event hook function prototype for responding to __ALL__ event
 *
 * For responding to all events, the arguements of the function will be variable
 * in hook function, it has to be analyzed event by event,
 * refer to ::wlcsm_register_event_hook for example.
 */
typedef void (*WLCSM_EVENT_HOOK_GENERICFUNC)(t_WLCSM_EVENT_TYPE event,...);

/*************************************************************//**
 * @brief get nvram value
 *
 *  get nvram item value by name
 *
 **/
char *wlcsm_nvram_get(char *name);

/*************************************************************//**
 * @brief set nvram value
 *
 *  set nvram item value by name
 *
 **/
#ifdef  WLCSM_DEBUG
int  wlcsm_nvram_set_dbg(char *name,char *value,const char *func,int line);
#else
int  wlcsm_nvram_set(char *name,char *value);
#endif

/*************************************************************//**
 * @brief get nvram bit value
 *
 *  get nvram item bit value by name and bit position
 *
 **/
char * wlcsm_nvram_get_bitflag(char *name, int bit);

/*************************************************************//**
 * @brief replace nvram  bit value
 *
 *  set nvram item value by name and bit position
 *
 **/

int wlcsm_nvram_set_bitflag(char *name,int bit, int value);

/*************************************************************//**
 * @brief unset nvram value
 *
 * unset nvram item value by name
 *
 **/
int  wlcsm_nvram_unset(char *name);


/*************************************************************//**
 * @brief getall nvram value
 *
 *  retrieve all nvram item values into buf with maximum length of count
 *
 **/
int wlcsm_nvram_getall(char *buf, int count);

/*************************************************************//**
 * @brief commit all nvram values into storage
 *
 * once commit, event will send over to APPs which are listening on the event, APPs should respond
 * to this event with corresponding actions.
 *
 **/
int wlcsm_nvram_commit (void);

/*************************************************************//**
 * @brief set trace debug
 *
 * This API will take string input such as:
 *     +log [enable user space debugging log]
 *     -err
 *     k+log [kernel debugging level]
 ***************************************************************/
int wlcsm_set_trace_level(char *tl);

#ifdef WLCSM_DEBUG
#define  WLCSM_SET_TRACE(proc)  wlcsm_set_trace(proc)
#else
#define  WLCSM_SET_TRACE(proc)
#endif
/*************************************************************//**
 * @brief set userpace trace debug
 *
 ***************************************************************/
void wlcsm_set_trace(char *procname);

/*************************************************************//**
 * @brief enable listenging thread for event handling
 *
 * When an app often conduct nvram read/write operation, to improve performance
 * invoke this API at the beginning of the APP will enable local copy of nvram
 * value and it will get updated in time. If the application has registered event
 * , this API is being invoked automatically. [so do not need to call it again]
 *
 ***************************************************************/
int wlcsm_init(void);

/*************************************************************//**
 * @brief reinit wlcsm resources
 *
 * When applications using wlcsm creates child process with fork()
 * calls, there is a chance netlink sockets has been created, child
 * process and parent process will share the same socket, which could
 * led to problem when nvram operation is frequent.
 * SUGGESTING TO CALL ::wlcsm_reinit() after fork() call to create child
 * process socket of its own.
 **************************************************************/
int wlcsm_reinit(void);

/*************************************************************//**
 * @brief resource clean up api
 *
 * This API should be used as pair with ::wlcsm_init when app quite.
 **************************************************************/
int wlcsm_shutdown(void);


/*************************************************************//**
 * @brief API for app to register hook for specific event
 *
 * For some applications, they maybe only interested in certain events,but not
 * all events. For example, some application will be only interested in nvram
 * change event, in this case, the applications can use this function to register
 * funtion to respond to this specific event.
 *
 * ~~~~~
 *
 * void nvram_changed(char *name, char *value, char *oldvalue) {
 *        printf("nvram changed noticed:%s,from %s to %s \r\n",namme,oldvalue,value );
 * }
 *
 * wlcsm_register_event_hook(WLCSM_EVT_NVRAM_CHANGED,nvram_changed);
 * ~~~~~
 *
 * How to respond to this event is solely invoking application's responsibility.But take
 * notice that this function is invoked when the event happens, the hook function should
 * not occupy too long for processing.
 *
 * @return void
 ***************************************************************/
void wlcsm_register_event_hook(t_WLCSM_EVENT_TYPE type,WLCSM_EVENT_HOOK_FUNC hook);

/*************************************************************//**
 * @brief API for app to register hook to respond __ALL__ event.
 *
 * In rare case, one application is interested in all the events. In such a case, one
 * applicatoin can use this API to register its event handler.
 * @anchor allevent
 * ~~~~
 *    void wlcsm_event_handler(t_WLCSM_EVENT_TYPE type,...)
 *    {
 *        va_list arglist;
 *
 *        va_start(arglist,type);
 *
 *        switch ( type ) {
 *
 *        case WLCSM_EVT_NVRAM_CHANGED: {
 *
 *        char * name=va_arg(arglist,char *);
 *        char * value=va_arg(arglist,char *);
 *        char * oldvalue=va_arg(arglist,char *);
 *        printf("change received name:%s,value:%s,oldvalue:%s\r\n",name,value,oldvalue);
 *        }
 *        break;
 *
 *        case WLCSM_EVT_NVRAM_COMMITTED:
 *        printf("nvram commited, restart or not? \r\n");
 *        break;
 *
 *        default:
 *        break;
 *        }
 *
 *        va_end(arglist);
 *    }
 *
 *     wlcsm_register_event_generic_hook(wlcsm_event_handler);
 *~~~~
 * @return void
 ***************************************************************/
void     wlcsm_register_event_generic_hook(WLCSM_EVENT_HOOK_GENERICFUNC hook);


/*************************************************************//**
 * @brief register userspace process to wlcsm kernel to handle wlmngr or wldebug
 *
 * wlcsm netlink kernel need to know the pid of wlmngr (or wldebug) when it
 * receives mngr request or debug request. this API is used to register the
 * userspace application to kernel.
 **************************************************************/
int     wlcsm_register_process (char *process_name);


/*************************************************************//**
 * @brief calling wl to  halt restart
 *
 * This API is used when other applications change the configuraiton and
 * need to halt wifi auto restart from MDM.
 **************************************************************/
int    wlcsm_mngr_halt_restart(void);

/*************************************************************//**
 * @brief calling wl to  resume restart
 *
 * This API is used when other applications change the configuraiton and
 * need to resume wifi restart to reconfigure
 **************************************************************/
int    wlcsm_mngr_resume_restart(void);

/*************************************************************//**
 * @brief calling wl restart all wifi radio from other applications
 *
 * This API is used when other applications change the configuraiton and
 * need a wireless reconfiguration to restart. it will call this API.
 * it is usually used with wlcsm_mngr_halt_restart and wlcms_mngr_resume_restart
 * and controlled to restart wifi. If changes are in MDM,
 * savedm should be one, if changes are only nvrams, savedm should
 * be 0
 **************************************************************/
int   wlcsm_mngr_restart_all(int savedm,int wait);

/*************************************************************//**
 * @brief calling wl restart from other applications
 *
 * This API is used when other applications change the configuraiton and
 * need a wireless reconfiguration to restart. it will call this API.
 **************************************************************/
int    wlcsm_mngr_restart(unsigned int radio_idx,unsigned int source,int savedm,int wait);

/*************************************************************//**
 * @brief calling wlmngr to update its maintained STA lists
 *
 * This API is used when other application(mostly wlevt) noticed associated STA
 * changes, it will notifiy wlmngr to update the associated lists.
 **************************************************************/
int wlcsm_mngr_update_stalist(unsigned int radio_idx,unsigned int sub_idx,void *value,int len);

/*************************************************************//**
 * @brief hotplug notify to wlmngr about network interface changes
 *
 * This API is used when hotplug detected network interface changes
 **************************************************************/
int wlcsm_mngr_nethotplug_notify(char *net,char *action);

/*************************************************************//**
 * @brief calling wlmngr to run certian cmds
 *
 * This API is used for applications to send commands to wireless
 * lan manager,the result will returned in the value paramater.
 *
 **************************************************************/
int wlcsm_mngr_run_cmd(unsigned int radio_idx,unsigned int sub_idx,unsigned int source,WLCSM_MNGR_CMD cmd,char *value);

/*************************************************************//**
 * @brief setting wireless configuration parameters
 *
 * This API is used when other applications try to set wireless parameters values,certainly
 * based on the name of each parameters.
 *
 * TODO: provide a list of parameter names with description. [This can be genreated from TR181 data
 * model which new wlmngr is based on]
 **************************************************************/
int     wlcsm_mngr_set(unsigned int radio_idx,unsigned int sub_idx,char * name, char *value);

/*************************************************************//**
 * @brief setting wireless configuration parameters by datamodel oid and offset
 *
 * This API is used when other applications try to set wireless parameters values,
 * based on the datamodel's oid and offset,this set will first do validation and set
 * return success(0) if ok, return error when validation fails
 *
 **************************************************************/
int      wlcsm_mngr_dm_set(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,unsigned short offset, char *value);


/*************************************************************//**
 * @brief set dm implementations dbg level
 *
 **************************************************************/
int wlcsm_mngr_dm_set_dbglevel(int dbglevel);

/*************************************************************//**
 * @brief validate wireless configuration parameters by datamodel oid and offset
 *
 * This API is used when other applications try to validate wireless parameters values,
 * based on the datamodel's oid and offset,it won't change the value at all
 *
 **************************************************************/
int      wlcsm_mngr_dm_validate(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,unsigned short offset, char *value);

/*************************************************************//**
 * @brief get wireless configuration parameters by datamodel oid and offset
 *
 * This API is used when other applications try to get wireless parameters values,
 * based on the datamodel's oid and offset,it won't change the value at all
 *
 * Note: There are some runtime information may not updated in DM, but it definitely
 * can be available from wlmngr, thus this API will be able to get the values.
 *
 **************************************************************/
char 	*wlcsm_mngr_dm_get(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,unsigned short offset,char *varValue);


/*************************************************************//**
 * @brief getting wireless configuration parameters
 *
 * This API is used when other applications try to retrive wireless parameters values,certainly
 * based on the name of each parameters.
 *
 **************************************************************/
char    *wlcsm_mngr_get(unsigned int radio_idx,unsigned int sub_idx,char * name,char *varValue);
char    *wlcsm_mngr_safe_get(unsigned int radio_idx,unsigned int sub_idx,char * name,char *varValue);


/*************************************************************//**
 * @brief  used to send cmd request to wlmngr,such as restart and request
 *
 * This API is used when other applications want to send request to wlmngr
 * for wireless related controls.
 *
 **************************************************************/
void    *wlcsm_mngr_cmd(unsigned int radio_idx,unsigned int sub_idx,WLCSM_MNGR_CMD cmd,void *value);


/*************************************************************//**
 * @brief get to know the datamodel name
 *
 **************************************************************/
char    *wlcsm_mngr_get_dmname(char *varValue);


/*************************************************************//**
 * @brief  trim tailing space.
 *
 **************************************************************/
char *wlcsm_trim_str(char *value);

char *wlcsm_nvram_xfr(char *buf);

char *wlcsm_nvram_xfr(char *buf);

#ifdef DUMP_PREV_OOPS_MSG
int wlcsm_dump_prev_oops(void);
#endif

/*************************************************************//**
 * @brief bianry MAC address comparation
 *
 **************************************************************/
static inline int WLCSM_MAC_MATCH(char *mac1,char *mac2) {
    int i=0;
    do {
        if(mac1[i]!=mac2[i]) {
            return 0;
        }
    } while(++i<6);
    return 1;
}

extern char g_MAC_STR[];
char *BINMAC_STR_CONVERT(char *data);

static inline  void STRMAC_BIN_CONVERT(char *strmac,char *hexmac) {
    int i=0;
    unsigned int num;
    for (; i<6; i++) {
        sscanf(strmac+i*3,"%02X",&num);
        *(hexmac+i)=num;
    }
}

int wlcsm_strtol(char *str,int base,long *val);
int wlcsm_binaryBufToHexString(const char *binaryBuf, int binaryBufLen, char **hexStr);
int wlcsm_hexStringToBinaryBuf(const char *hexStr, char **binaryBuf, int *binaryBufLen);
int wlcsm_get_random(unsigned char *rand, int len);
#ifdef WLCSM_DEBUG
void wlcsm_delta_time(char *func,int line,char *prompt);
void wlcsm_restart_time(char *func,int line);
void wlcsm_total_time(char *func,int line);
#define WLCSM_START_TIME() wlcsm_restart_time(__FUNCTION__,__LINE__)
#define WLCSM_DELTA_TIME(prt) wlcsm_delta_time(__FUNCTION__,__LINE__,prt)
#define WLCSM_TOTAL_TIME() wlcsm_total_time(__FUNCTION__,__LINE__)
#define wlcsm_nvram_set(name,value) wlcsm_nvram_set_dbg(name,value,__FUNCTION__,__LINE__)
#else
#define WLCSM_START_TIME()
#define WLCSM_DELTA_TIME(prt)
#define WLCSM_TOTAL_TIME()
#endif

/*!
 \def WLCSM_STRCASECMP(s1,s2)
  strncasecmp of two string \a s1 and \a s2
 \def WLCSM_STRCMP(s1,s2)
  strncmp of two string \a s1 and \a s2
*/
#define WLCSM_STRCASECMP(s1,s2) strncasecmp((s1),(s2),strlen((s1))+1)
#define WLCSM_STRCMP(s1,s2) strncmp((s1),(s2),strlen((s1))+1)

int wlcsm_replace_str(char *dst,char *src);
#define WLCSM_STRREPLACE(dst,src)  wlcsm_replace_str(dst,src)

/*************************************************************//**
 * @brief  used to enable/disable notification to dm module
 *
 * This API is used when an application tries to enable/disable DM notification
 * when some event happent, these events are defined in ::WLCSM_DM_MNGR_EVENT .
 *
 **************************************************************/
int wlcsm_mngr_dm_register_event(WLCSM_DM_MNGR_EVENT event,int to_reigster);
/** @} */
#endif
