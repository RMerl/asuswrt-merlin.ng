/*
* <:copyright-BRCM:2006:proprietary:standard
* 
*    Copyright (c) 2006 Broadcom 
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
:>
*/
/***************************************************************************
 * File Name  : mcpd.h
 *
 * Description:
 *
 ***************************************************************************/
#ifndef __MCPD_H__
#define __MCPD_H__

//#define MCPD_DEBUG

#include <stdio.h>
#include <bcmtypes.h>
#include <net/if.h>
#include <netinet/in.h>
#include <bcm_mcast_api.h>
#if defined(CONFIG_BCM_OVS_MCAST)
#include <mcpd_ovs.h>
#endif

#ifdef BRCM_CMS_BUILD
#include "cms_log.h"

// Align MCPD logging levels with CMS logging levels
typedef enum mcpd_trace_level
{
    MCPD_TRC_ERR=LOG_LEVEL_ERR,
    MCPD_TRC_LOG=LOG_LEVEL_DEBUG,
    MCPD_TRC_INFO=LOG_LEVEL_NOTICE,
} t_MCPD_TRACE_LEVEL;

#define MCPD_TRACE(level, args...)  log_log(level, __FUNCTION__, __LINE__, ##args)
#else

typedef enum mcpd_trace_level
{
    MCPD_TRC_ERR=1,
    MCPD_TRC_LOG=2,
    MCPD_TRC_INFO=3,
} t_MCPD_TRACE_LEVEL;

#define MCPD_DEBUG_LEVEL MCPD_TRC_ERR

#define MCPD_TRACE(level, format, args...)                   \
      do {                                                   \
         if ( level <= MCPD_DEBUG_LEVEL ) {                  \
            printf("%s:" format "\n", __FUNCTION__, ##args); \
         }                                                   \
      } while ( 0 )

#endif /* BRCM_CMS_BUILD */

#define MCPD_ASSERT(x)                                                  \
                       do                                               \
                       {                                                \
                          if(x)                                         \
                          {                                             \
                             /* Do Nothing */                           \
                          }                                             \
                          else                                          \
                          {                                             \
                             fprintf(stderr,                            \
                              " MCPD assert (%s) at %s:%d %s()\n",      \
                                #x , __FILE__, __LINE__, __func__);     \
                          }                                             \
                       } while (0)
/* when this is defined MCPD will associate a downstream interfaces with all
   routed interfaces and all bridged upstream interfaces that are part of
   the dowstream interface 
   when this is not defined, MCPD will associate a downstream interface with
   all bridged interfaces that are part of the downstream interface. If no
   there are no bridged upstream interfaces in the dowsatream interface then 
   the downstream interface will be associated with all routed upstream 
   interfaces */
#define MCPD_ASSOCIATE_DS_TO_ALL_UP

/******************************************************************************
 *
 *                           GLOBAL DEFINITIONS
 *
 *****************************************************************************/
#define _MLD_PATH_PROCNET_IFINET6           "/proc/net/if_inet6"
#define _PATH_PROCNET_DEV                   "/proc/net/dev"

#define CLI_MAX_BUF_SZ       256

#define MCPD_FMODE_INCLUDE   1
#define MCPD_FMODE_EXCLUDE   0

#define MCPD_DOWNSTREAM      1
#define MCPD_UPSTREAM        2

#define MCPD_MFC_ADD          0x10
#define MCPD_MFC_DEL          0x20

#define MCPD_SNOOP_IN_ADD         BCM_MCAST_SNOOP_IN_ADD
#define MCPD_SNOOP_IN_CLEAR       BCM_MCAST_SNOOP_IN_CLEAR
#define MCPD_SNOOP_EX_ADD         BCM_MCAST_SNOOP_EX_ADD
#define MCPD_SNOOP_EX_CLEAR       BCM_MCAST_SNOOP_EX_CLEAR

#define MCPD_IPV4_ADDR_SIZE  4
#define MCPD_IPV6_ADDR_SIZE  16

#define MAX_PORTS            24
#define MCPD_MAX_IFS         BCM_MCAST_MAX_SRC_IF

#define MSECS_IN_SEC  1000
#define USECS_IN_MSEC 1000

typedef enum
{
    MCPD_FALSE = 0,
    MCPD_TRUE  = 1,
} t_MCPD_BOOL;

typedef enum mcpd_obj_type
{
    MCPD_IF_OBJ=0,
    MCPD_INTERFACE_OBJ,
    MCPD_IPV4_ADDR_OBJ,
    MCPD_IPV6_ADDR_OBJ,
    MCPD_IGMP_GRP_OBJ,
    MCPD_IGMP_REP_OBJ,
    MCPD_IGMP_SRC_OBJ,
    MCPD_IGMP_SRC_REP_OBJ,
    MCPD_IGMP_REP_SRC_OBJ,
    MCPD_MLD_GRP_OBJ,
    MCPD_MLD_REP_OBJ,
    MCPD_MLD_SRC_OBJ,
    MCPD_MLD_SRC_REP_OBJ,
    MCPD_MLD_REP_SRC_OBJ,
    MCPD_EXCEPTION_OBJ,
    MCPD_MAX_OBJ,
} t_MCPD_OBJ_TYPE;

typedef enum mcpd_ret_code
{
    MCPD_RET_OK = 0,
    MCPD_RET_GENERR = 1,
    MCPD_RET_MEMERR = 2,
    MCPD_RET_ACCEPT = 3,
    MCPD_RET_DROP   = 4
} t_MCPD_RET_CODE;

typedef enum mcpd_proto_type
{
    MCPD_PROTO_IGMP = 0,
    MCPD_PROTO_MLD  = 1,
    MCPD_PROTO_MAX  = 2,
} t_MCPD_PROTO_TYPE;

typedef enum mcpd_if_status
{
    MCPD_IF_STATUS_DOWN = 0,
    MCPD_IF_STATUS_UP   = 1
} t_MCPD_IF_STATUS;

typedef enum mcpd_admission_type
{
    MCPD_ADMISSION_JOIN = 0,
    MCPD_ADMISSION_RE_JOIN,
    MCPD_ADMISSION_LEAVE
} t_MCPD_ADMISSION_TYPE;

typedef enum e_flood_type
{
    FLOOD_TYPE_ALL    = 0, 
    FLOOD_TYPE_OTHERS = 1,   /* only update flooding status on other ports */
} t_MCPD_FLOOD_TYPE;

/* Definition for interface info object */
typedef struct t_mcpd_ifinfo_obj
{
    char                      name[IFNAMSIZ];
    short                     index;
    int                       iftype;
    struct sockaddr_in        addr;
    struct sockaddr_in6       addr6;
    struct t_mcpd_ifinfo_obj *next;
} t_MCPD_IFINFO_OBJ;

/* Definition for multicast source object */
typedef struct mcpd_src_obj
{
    UINT8                    *addr;   /* source ip addr */
    int                       fmode;  /* filter mode */
    struct mcpd_src_rep_obj  *srep;   /* ptr to source reporter */
    struct mcpd_group_obj    *gp;
    struct mcpd_src_obj      *next;
} t_MCPD_SRC_OBJ;

/* Definition for multicast reporter object */
typedef struct mcpd_rep_obj
{
    void                     *parent_group;
    UINT8                     version;
    UINT8                    *addr;
    UINT16                    rep_ifi;
    struct mcpd_rep_src_obj  *rsrc;
    struct mcpd_rep_obj      *next;
} t_MCPD_REP_OBJ;

/* Definition for multicast source reporter object */
typedef struct mcpd_src_rep_obj
{
    t_MCPD_REP_OBJ             *rep;
    struct mcpd_src_rep_obj    *next;
} t_MCPD_SRC_REP_OBJ;

/* Definition for multicast reporter source object */
typedef struct mcpd_rep_src_obj
{
    t_MCPD_SRC_OBJ            *src;
    struct mcpd_rep_src_obj   *next;
} t_MCPD_REP_SRC_OBJ;

/* Definition for multicast group object */
typedef struct mcpd_group_obj
{
    UINT8                     *addr;
    int                        fmode;
    int                        version;
    t_MCPD_REP_OBJ            *members;
    t_MCPD_SRC_OBJ            *in_sources;
    t_MCPD_SRC_OBJ            *ex_sources;
    int                        v2_host_prsnt_timer;
    int                        v1_host_prsnt_timer;
    int                        leaveQueriesLeft;
    struct mcpd_interface_obj *ifp;
    struct mcpd_group_obj     *next;
} t_MCPD_GROUP_OBJ;

typedef struct t_igmp_proxy_obj
{
    t_MCPD_GROUP_OBJ            *groups;
    int                          version;
    int                          is_querier;
    int                          query_interval;
    int                          query_resp_interval;
    int                          gmi;        /* group membership interval */
    int                          oqp;    /* other querier present timer*/
    int                          rv;   /* robustness variable */
    int                          ti_qi;  /* timer: query interval */
    int                          igmpi_comp_v1_timer;
    int                          igmpi_comp_v2_timer;
} t_IGMP_PROXY_OBJ;

#ifdef SUPPORT_MLD
typedef struct t_mld_proxy_obj
{
    t_MCPD_GROUP_OBJ            *groups;
    struct mcpd_membership_db   *membership_db;
    int                          version;
    int                          is_querier;
    int                          query_interval;
    int                          query_resp_interval;
    int                          gmi;        /* group membership interval */
    int                          oqp;    /* other querier present timer*/
    int                          rv;   /* robustness variable */
    int                          ti_qi;  /* timer: query interval */
    int                          mld_comp_v1_timer;
} t_MLD_PROXY_OBJ;
#endif

#define MCPD_MAX_UPSTREAM_SSM_SRS   24

enum e_multicast_group_mode {
    MULTICAST_MODE_IANA = 0,
    MULTICAST_MODE_FIRST_IN,
    MULTICAST_MODE_INVALID
};
/* MULTICAST_MODE_IANA - 
 *       IANA officially limits SSM addresses to 
 *           232.x.x.x for IPV4
 *           FF3x::/32 for IPV6
 * Users wishing to ignore the IANA limits should change the #define below to:
 *
 * MULTICAST_MODE_FIRST_IN
 * In that case, the IANA limits are disabled and each multicast group will be
 * determined to be ASM or SSM based on the first Join Report
 * Once defined as ASM, MCPD will reject SSM reports for that group
 * Once defined as SSM, MCPD will reject ASM reports for that group
 * If all reporters Leave the group, the group is free to be reassigned as
 * SSM or ASM based on the next Join report.
 */
#define IANA_MULTICAST_DEFAULT_GROUP_MODE MULTICAST_MODE_IANA

enum e_multicast_admission_filter_mode {
    MULTICAST_ADMISSION_FILTER_MODE_OFF = 0,
    MULTICAST_ADMISSION_FILTER_MODE_ON,
    MULTICAST_ADMISSION_FILTER_MODE_ON_WITH_FILTER,
};
#define MULTICAST_DEFAULT_ADMISSION_FILTER_MODE MULTICAST_ADMISSION_FILTER_MODE_OFF

enum e_multicast_type {
    MULTICAST_TYPE_ASM = 0,
    MULTICAST_TYPE_SSM,
    MULTICAST_TYPE_BOTH, /* INCLUDE with no sources - valid for both SSM and ASM */
    MULTICAST_TYPE_UNK
};

typedef t_BCM_MCAST_WAN_INFO_ARRAY t_MCPD_WAN_INFO_ARRAY;
typedef t_BCM_MCAST_PKT_INFO t_MCPD_PKT_INFO;

typedef struct mcpd_igmp_addr_obj
{
    struct in_addr addr;
    struct mcpd_igmp_addr_obj *next;
} t_MCPD_IGMP_ADDR_OBJ;

/* igmp upstream SSM filter Definition */
typedef struct mcpd_igmp_upstream_ssm
{
    struct in_addr group;
    int fmode;
    int numsources;
    struct in_addr sources[MCPD_MAX_UPSTREAM_SSM_SRS];
} t_MCPD_IGMP_UPSTREAM_SSM;

#ifdef SUPPORT_MLD
typedef struct mcpd_mld_addr_obj
{
    struct in_addr addr;
    struct mcpd_mld_addr_obj *next;
} t_MCPD_MLD_ADDR_OBJ;

typedef struct mcpd_mld_upstream_ssm
{
    struct in6_addr group;
    int fmode;
    int numsources;
    struct in6_addr sources[MCPD_MAX_UPSTREAM_SSM_SRS];
} t_MCPD_MLD_UPSTREAM_SSM;
#endif

/* interface Definition */
typedef struct mcpd_interface_obj
{
    struct in_addr               if_addr;
#ifdef SUPPORT_MLD
    struct in6_addr              if_addr6;
#endif
    char                         if_name[IFNAMSIZ];
    UINT16                       if_index;
#define MCPD_IF_TYPE_UNKWN      BCM_MCAST_IF_UNKNOWN
#define MCPD_IF_TYPE_BRIDGED    BCM_MCAST_IF_BRIDGED
#define MCPD_IF_TYPE_ROUTED     BCM_MCAST_IF_ROUTED
    int                          if_type;
    int                          if_dir; /*interface type:upstream/downstream*/
    struct sch_query            *sch_group_query;
    struct mcpd_interface_obj   *next;
    short                        setFlag;
#define MCPD_IGMP_PROXY_ENABLE     0x00000001
#define MCPD_IGMP_SNOOPING_ENABLE  0x00000010
#define MCPD_MLD_PROXY_ENABLE      0x00000100
#define MCPD_MLD_SNOOPING_ENABLE   0x00001000
#define MCPD_IPV4_MCAST_ENABLE     0x00010000
#define MCPD_IPV6_MCAST_ENABLE     0x00100000
    UINT32                       proto_enable;
    t_IGMP_PROXY_OBJ             igmp_proxy;
    unsigned short               vifi;
    int                          audit_done;
#ifdef SUPPORT_MLD
    t_MLD_PROXY_OBJ              mld_proxy;
    unsigned short               mifi;
#endif
} t_MCPD_INTERFACE_OBJ;

typedef struct {
    char brPortName[IFNAMSIZ];
    int port_ifi;
    UINT8 maxGroupsForPort;
} t_MCPD_PORT_MAX_GROUP;

typedef struct mcpd_filter_exception {
    struct mcpd_filter_exception *next;
    struct in6_addr address;
    struct in6_addr mask;
} t_MCPD_FILTER_EXCEPTION;

typedef struct mcpd_igmp_config
{
    UINT8                     default_version;
    UINT32                    query_interval;        /* seconds */
    UINT32                    query_resp_interval;   /* tenths of a second */
    UINT32                    lmqi;                  /* last-member-query-interval, (tenths of a second) */
    UINT32                    robust_val;            /* igmp-robustness-value, also last-member-query-count */
    UINT32                    max_groups;
    UINT32                    max_sources;
    UINT32                    max_members;
    UINT8                     fast_leave_enable;
    UINT8                     strict_wan;
    UINT32                    startup_query_interval; /* seconds */
    UINT32                    startup_query_count;
    UINT8                     admission_required;
    UINT8                     admission_bridging_filter;
    t_MCPD_FILTER_EXCEPTION*  filter_list;
    UINT8                     flood_enable;  /* flood multicast data to all ports which under same bridge */
} t_MCPD_IGMP_CONFIG;

typedef struct mcpd_mld_config
{
    UINT8                     default_version;
    UINT32                    query_interval;          /* seconds */
    UINT32                    query_resp_interval;     /* tenths of a second */
    UINT32                    lmqi;                    /* last-member-query-interval , (tenths of a second) */
    UINT32                    robust_val;              /* mld-robustness-value */
    UINT32                    max_groups;
    UINT32                    max_sources;
    UINT32                    max_members;
    UINT8                     fast_leave_enable;
    UINT8                     strict_wan;
    UINT8                     admission_required;
    UINT8                     admission_bridging_filter;
    t_MCPD_FILTER_EXCEPTION*  filter_list;
    UINT8                     flood_enable;  /* flood multicast data to all ports which under same bridge */
} t_MCPD_MLD_CONFIG;

typedef t_MCPD_RET_CODE (*func_cmp)(UINT8 const *, UINT8 const *);
typedef t_MCPD_RET_CODE (*func_chg_mfc)(UINT8 *, UINT8 *);
typedef t_MCPD_RET_CODE (*func_update_snooping_info)(t_MCPD_INTERFACE_OBJ *,
                                                     t_MCPD_GROUP_OBJ *,
                                                     t_MCPD_REP_OBJ *,
                                                     UINT8 *,
                                                     int,
                                                     t_MCPD_PKT_INFO *);
typedef t_MCPD_RET_CODE (*func_update_flooding_info)(t_MCPD_INTERFACE_OBJ *,
                                                     t_MCPD_GROUP_OBJ *,
                                                     t_MCPD_REP_OBJ *,
                                                     UINT8 *,
                                                     int,
                                                     t_MCPD_PKT_INFO *,
                                                     t_MCPD_FLOOD_TYPE);
typedef t_MCPD_RET_CODE (*func_update_rep_tmr)(t_MCPD_REP_OBJ *);
typedef t_MCPD_RET_CODE (*func_update_src_tmr)(t_MCPD_SRC_OBJ *);
typedef t_MCPD_RET_CODE (*func_krnl_update_ssm_filters)(t_MCPD_INTERFACE_OBJ *,
                                                       t_MCPD_GROUP_OBJ *,
                                                       UINT8 *,
                                                       t_MCPD_PKT_INFO *);
typedef void (*func_membership_query)(t_MCPD_INTERFACE_OBJ *,
                                      t_MCPD_GROUP_OBJ *,
                                      UINT8 *,
                                      int numsrc);
typedef t_MCPD_RET_CODE (*func_update_upstream_ssm)(t_MCPD_GROUP_OBJ *,
                                                    UINT8 *);
typedef t_MCPD_RET_CODE (*func_admission_control)(int msg_type,
                                                  int rxdev_ifi,
                                                  UINT8 *gp,
                                                  UINT8 *src,
                                                  UINT8 *rep,
                                                  unsigned short tci,
                                                  UINT8 rep_proto_ver);
typedef t_MCPD_RET_CODE (*func_krnl_drop_membership)(t_MCPD_INTERFACE_OBJ *,
                                                     t_MCPD_GROUP_OBJ *);

#if defined(CONFIG_BCM_OVS_MCAST)
typedef struct mcpd_ovs_info_struct
{
    int sock_ovs;
    int sock_ovs_con;
    char *sock_ovs_buff;
    t_ovs_mcpd_brcfg_info brcfg;
}t_mcpd_ovs_info;
#endif

typedef struct t_mcpd_router
{
    t_MCPD_INTERFACE_OBJ     *interfaces;
    int                       sock_nl;
    char                     *sock_buff;
    int                       sock_igmp;
    int                       sock_ctl;
    int                       sock_ctl_con;
    t_MCPD_IGMP_CONFIG        igmp_config;
    unsigned int              vifiBits;
#ifdef SUPPORT_MLD
    int                       sock_mld;
    t_MCPD_MLD_CONFIG         mld_config;
    unsigned int              mifiBits;
#endif
#if defined(CONFIG_BCM_OVS_MCAST)
    t_mcpd_ovs_info           ovs_info;
#endif
    t_MCPD_PORT_MAX_GROUP     max_group_port_list[MAX_PORTS];
    func_cmp                  cmp_ip_obj_func[MCPD_PROTO_MAX];
    func_chg_mfc              chg_mfc_func[MCPD_PROTO_MAX];
    func_update_snooping_info update_snooping_info_func[MCPD_PROTO_MAX];
    func_update_flooding_info update_flooding_info_func[MCPD_PROTO_MAX];
    func_update_rep_tmr       update_rep_tmr_func[MCPD_PROTO_MAX];
    func_update_src_tmr       update_src_tmr_func[MCPD_PROTO_MAX];
    func_krnl_update_ssm_filters krnl_update_ssm_filters_func[MCPD_PROTO_MAX];
    func_membership_query     membership_query_func[MCPD_PROTO_MAX];
    func_update_upstream_ssm  update_upstream_ssm_func[MCPD_PROTO_MAX];
    func_admission_control    admission_control_func[MCPD_PROTO_MAX];
    func_krnl_drop_membership krnl_drop_membership_func[MCPD_PROTO_MAX];
} t_MCPD_ROUTER;

#define MCPD_ALLOC(x, y) mcpd_alloc(x, y)
#define MCPD_FREE(x, y)  mcpd_free(x, y)

#endif /* __MCPD_H__ */
