/******************************************************************************
 *
 * <:copyright-BRCM:2017:proprietary:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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
 *
 *****************************************************************************/
#ifndef __DPICTL_API_H
#define __DPICTL_API_H

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/netfilter.h>
#include <bcmdpi.h>

#define DPICTL_URL_LENGTH           64

#define DPICTL_UPDATE_DB            0x8000
#define DPICTL_UPDATE_APPINSTS      0x0001
#define DPICTL_UPDATE_URLS          0x0002
#define DPICTL_UPDATE_QOS_INFO      0x0004
#define DPICTL_UPDATE_QOS_QUEUES    0x0008
#define DPICTL_UPDATE_QOS_APPINSTS  0x0010
#define DPICTL_UPDATE_DEVS          0x0020

#define DPICTL_UPDATE_ALL_RUNTIME   0x003F
#define DPICTL_UPDATE_ALL           ~0

struct dc_traffic {
   uint64_t pkts;
   uint64_t bytes;
};

struct dc_app {
   /* contains app id, appcat id, and behaviour id (see macros in bcmdpi.h) */
   uint32_t app_id;
};

struct dc_app_stat {
   struct dc_app     app;
   struct dc_traffic us;
   struct dc_traffic ds;
};

struct dc_dev {
   uint32_t dev_id;
   uint32_t vendor_id;
   uint32_t os_id;
   uint32_t class_id;
   uint32_t type_id;
   uint8_t  mac[6];
};

struct dc_dev_stat {
   struct dc_dev     dev;
   struct dc_traffic us;
   struct dc_traffic ds;
};

struct dc_appinst {
   struct dc_app     app;
   struct dc_dev     dev;
   struct dc_traffic us;
   struct dc_traffic ds;
   uint32_t          flow_count;
};

struct dc_flow_info {
   uint64_t           pkts;
   uint64_t           bytes;

   /* used for nfct */
   uint16_t           type;
   union nf_inet_addr l3src;
   union nf_inet_addr l3dst;
   uint8_t            l3protonum;
   uint16_t           l4src;
   uint16_t           l4dst;
   uint8_t            l4protonum;
};

struct dc_flow {
   struct dc_app       app;
   struct dc_dev       dev;
   char                url[DPICTL_URL_LENGTH];
   uint32_t            status;
   struct dc_flow_info ds;
   struct dc_flow_info us;
};

struct dc_qos_appinst {
   struct dc_app app;
   struct dc_dev dev;
   int32_t       ds_queue;
   uint32_t      kbps;
   int32_t       us_queue;
   uint32_t      flow_count;
};

struct dc_qos_appinst_stat {
   struct dc_qos_appinst appinst;
   struct dc_traffic     us;
   struct dc_traffic     ds;
};

struct dc_qos_queue {
   int32_t  id;
   uint32_t valid;
   uint32_t min_kbps;
   uint32_t max_kbps;
};

struct dc_qos_info_dir {
   int num_queues;
   int default_queue;
   int bypass_queue;
   int overall_bw;
};

struct dc_qos_info {
   struct dc_qos_info_dir us;
   struct dc_qos_info_dir ds;
};

struct dc_id_name_map {
   uint32_t  id;
   char     *name;
};

struct dc_url {
   char *url;
};

struct dc_table {
   int   entries;
   char  data[0];
};

enum db_id {
   db_app,
   db_appcat,
   db_dev,
   db_dev_vendor,
   db_dev_os,
   db_dev_class,
   db_dev_type,
   db_dev_family,
};

enum dpictl_type {
   dpictl_classification,
   dpictl_qos
};


/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_update
 * Description  : Update DPI info before query.
 * Arguments    : updates - bitmap of updates to perform:
 *                  DPICTL_UPDATE_DB           - signature database
 *                  DPICTL_UPDATE_APPINSTS     - application instances
 *                  DPICTL_UPDATE_QOS_INFO     - qos system bandwidth & info
 *                  DPICTL_UPDATE_QOS_QUEUES   - qos queue bandwidths
 *                  DPICTL_UPDATE_QOS_APPINSTS - qos appinstances & queue
 *                                               assignments
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpictl_update(unsigned int updates);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_enable
 * Description  : Enable/disable DPI feature.
 * Arguments    : type - from enum dpictl_type
 *                enable - enable or disable
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpictl_enable(int type, int enable);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_status
 * Description  : Show DPI classification enabled status.
 * Arguments    : type - from enum dpictl_type
 * Returns      : status on success, -1 on error
 *------------------------------------------------------------------------------
 */
int dpictl_status(int type);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_reset_stats
 * Description  : Reset all appinst stats. Note that this does not reset current
 *                flow stats.
 * Returns      : 0 on success, -1 on error
 *------------------------------------------------------------------------------
 */
int dpictl_reset_stats(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_set_max_pkt
 * Description  : Set max number of packets used for DPI classification.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpictl_set_max_pkt(int pkt);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_name
 * Description  : Lookup the name of the given id
 * Arguments    : id    - id of item to look up
 *                db_id - database id (from enum db_id)
 * Returns      : name - success, NULL - error
 * Note         : string returned must be freed by caller
 *------------------------------------------------------------------------------
 */
char *dpictl_get_name(uint32_t id, int db_id);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_db
 * Description  : Get id-name map for a given database
 * Arguments    : db_id - database id (from enum db_id)
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_id_name_map.
 *                struct returned must be freed by caller, along with each
 *                entry's name string.
 *------------------------------------------------------------------------------
 */
struct dc_table *dpictl_get_db(int db_id);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_appinst
 * Description  : Get application instance stats for given appinst.
 * Returns      : struct - success, NULL - error
 * Note         : struct returned must be freed by caller
 *------------------------------------------------------------------------------
 */
struct dc_appinst *dpictl_get_appinst(uint32_t app_id, uint8_t *mac);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_appinsts
 * Description  : Get table of all appinsts in the system
 * Arguments    : active_only - set to 1 to include only appinsts with active
 *                              flows
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_appinst.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *dpictl_get_appinsts(int active_only);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_appinsts_filter
 * Description  : Get table of all appinsts in the system filtered by the given
 *                callback function.
 * Arguments    : cb          - callback returning nonzero on match
 *                data        - user data passed to cb in addition to appinst
 *                active_only - set to 1 to include only appinsts with active
 *                              flows
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_appinst.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *
dpictl_get_appinsts_filter(int (*cb)(struct dc_appinst *appinst, void *data),
                           void *data, int active_only);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_dev
 * Description  : Get device statistics for given mac
 * Returns      : struct - success, NULL - error
 * Note         : struct returned must be freed by caller
 *------------------------------------------------------------------------------
 */
struct dc_dev_stat *dpictl_get_dev(uint8_t *mac);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_devs
 * Description  : Get table of all devices in the system
 * Arguments    : active_only - set to 1 to include only appinsts with active
 *                              flows
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_dev_stat.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *dpictl_get_devs(int active_only);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_devs_filter
 * Description  : Get table of all devices in the system filtered by the given
 *                callback function.
 * Arguments    : cb          - callback returning nonzero on match
 *                data        - user data passed to cb in addition to dev
 *                active_only - set to 1 to include only appinsts with active
 *                              flows
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_dev_stat.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *
dpictl_get_devs_filter(int (*cb)(struct dc_dev_stat *dev, void *data),
                       void *data, int active_only);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_app
 * Description  : Get application statistics for given app_id
 * Returns      : struct - success, NULL - error
 * Note         : struct returned must be freed by caller
 *------------------------------------------------------------------------------
 */
struct dc_app_stat *dpictl_get_app(uint32_t app_id);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_apps
 * Description  : Get table of all apps in the system
 * Arguments    : active_only - set to 1 to include only appinsts with active
 *                              flows
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_app_stat.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *dpictl_get_apps(int active_only);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_apps_filter
 * Description  : Get table of all apps in the system filtered by the given
 *                callback function.
 * Arguments    : cb          - callback returning nonzero on match
 *                data        - user data passed to cb in addition to app
 *                active_only - set to 1 to include only appinsts with active
 *                              flows
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_app_stat.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *
dpictl_get_apps_filter(int (*cb)(struct dc_app *app, void *data),
                       void *data, int active_only);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_flows
 * Description  : Get table of all flows in the system
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_flow.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *dpictl_get_flows(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_flows_filter
 * Description  : Get table of all flows in the system filtered by the given
 *                callback function.
 * Arguments    : cb   - callback returning nonzero on match
 *                data - user data passed to cb in addition to flow
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_flow.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *
dpictl_get_flows_filter(int (*cb)(struct dc_flow *flow, void *data),
                        void *data);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_urls
 * Description  : Get table of all encountered urls
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_url.
 *                struct returned must be freed by caller, along with each
 *                entry's name string.
 *------------------------------------------------------------------------------
 */
struct dc_table *dpictl_get_urls(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_block_flow
 * Description  : Block a given flow from being forwarded
 * Arguments    : flow  - pointer to flow object to block
 *                block - 1 to block flow, 0 to unblock flow
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int dpictl_block_flow(struct dc_flow *flow, int block);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_qos_appinst
 * Description  : Get qos application instance info for given appinst. Includes
 *                statistics from current flows.
 * Returns      : struct - success, NULL - error
 * Note         : struct returned must be freed by caller
 *------------------------------------------------------------------------------
 */
struct dc_qos_appinst_stat *dpictl_get_qos_appinst(uint32_t app_id,
                                                   uint8_t *mac);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_qos_appinsts
 * Description  : Get table of all qos appinsts in the system
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_qos_appinst_stat.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *dpictl_get_qos_appinsts(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_qos_info
 * Description  : Get info about qos and bandwidth
 * Returns      : struct - success, NULL - error
 * Note         : struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_qos_info *dpictl_get_qos_info(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_qos_queues
 * Description  : Get table of all qos queues in the system
 * Returns      : struct - success, NULL - error
 * Note         : dc_table data contains array of struct dc_qos_queue.
 *                struct returned must be freed by caller.
 *------------------------------------------------------------------------------
 */
struct dc_table *dpictl_get_qos_queues(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_table
 * Description  : Program a new QoS table
 * Returns      : 0 - success, -1 - error
 *------------------------------------------------------------------------------
 */
int dpictl_table(DpictlTableType_t type, char *file);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_set_avail_bw
 * Description  : Set the available bandwidth for the system
 * Returns      : 0 - success, -1 - error
 *------------------------------------------------------------------------------
 */
int dpictl_set_avail_bw(unsigned int kbps);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_set_default_bw
 * Description  : Set the default bandwidth for the system
 * Returns      : 0 - success, -1 - error
 *------------------------------------------------------------------------------
 */
int dpictl_set_default_bw(unsigned int kbps);

/*
 *------------------------------------------------------------------------------
 * Function Name: dpictl_get_bw_cfg
 * Description  : Retrieve the current bandwidth configs
 * Returns      : struct - success, NULL - error
 * Note         : struct returned must be freed by caller
 *------------------------------------------------------------------------------
 */
DpictlBwConfig_t *dpictl_get_bw_cfg(void);

#endif  /* defined(__DPICTL_API_H) */
