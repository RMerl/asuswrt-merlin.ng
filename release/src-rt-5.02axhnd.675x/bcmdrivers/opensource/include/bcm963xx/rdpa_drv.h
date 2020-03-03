/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
:>
*/

#ifndef __RDPA_DRV__H_INCLUDED__
#define __RDPA_DRV__H_INCLUDED__

#include <linux/ioctl.h>
#include "bcmtypes.h"
#include "rdpa_types.h"
#include "rdpa_filter.h"
#include "rdpa_vlan_action.h"
#include "rdpa_ingress_class_basic.h"

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908) && !defined(BCM63158)
#include "rdpa_iptv.h"
#endif

#define RDPA_DRV_VERSION         "0.2"
#define RDPA_DRV_VER_STR         "v" RDPA_DRV_VERSION
#define RDPA_DRV_MODNAME         "Broadcom Runner Packet Processor"

#define RDPA_DRV_NAME            "bcmrdpa"

/* RDPA_DRV Character Device */
#define RDPADRV_MAJOR            337
#define RDPADRV_NAME             RDPA_DRV_NAME
#define RDPADRV_DEVICE_NAME      "/dev/" RDPADRV_NAME

/* RDPA Control Utility Executable */
#define RDPA_CTL_UTILITY_PATH    "/bin/rdpactl"

/* RDPA Proc FS Directory Path */
#define RDPA_PROC_FS_DIR_PATH    RDPA_NAME

#define RDPA_IFNAME_SIZE         36

/* Port TM scheduling capability */
#define RDPA_TM_SP_CAPABLE       0x1
#define RDPA_TM_WRR_CAPABLE      0x2
#define RDPA_TM_WDRR_CAPABLE     0x4
#define RDPA_TM_WFQ_CAPABLE      0x8
#define RDPA_TM_SP_WRR_CAPABLE   0x10
/* Support rate limit + mixed (SP + WRR) with one egress TM level. */
#define RDPA_TM_1LEVEL_CAPABLE   0x20

#define RDPA_VLAN_ACTION_TPID_VALUE_DONT_CARE (0xffff)

typedef enum {
   RDPA_DRV_SUCCESS              = 0,
   RDPA_DRV_ERROR                = 1,
   RDPA_DRV_NO_ROOT              = 2,
   RDPA_DRV_PORT_NOT_ALLOC       = 3,
   RDPA_DRV_PORT_ID_NOT_VALID    = 4,
   RDPA_DRV_NO_MORE_TMS          = 5,
   RDPA_DRV_TM_NOT_ALLOC         = 6,
   RDPA_DRV_NEW_TM_ALLOC         = 7,
   RDPA_DRV_TM_GET               = 8,
   RDPA_DRV_TM_CFG_GET           = 9,
   RDPA_DRV_TM_CFG_SET           = 10,
   RDPA_DRV_TM_INDEX_GET         = 11,
   RDPA_DRV_PORT_GET             = 12,
   RDPA_DRV_Q_CFG_GET            = 13,
   RDPA_DRV_Q_CFG_SET            = 14,
   RDPA_DRV_Q_RATE_SET           = 15,
   RDPA_DRV_GET_ROOT_BY_IF       = 16,
   RDPA_DRV_SUBS_SET             = 17,
   RDPA_DRV_MODE_GET             = 18,
   RDPA_DRV_SH_DESTROY           = 19,
   RDPA_DRV_IC_ERROR             = 20,
   RDPA_DRV_IC_NOT_FOUND         = 21,
   RDPA_DRV_IC_FLOW_ERROR        = 22,
   RDPA_DRV_ORL_LINK             = 23,
   RDPA_DRV_ORL_UNLINK           = 24,
   RDPA_DRV_BR_GET               = 25,
   RDPA_DRV_BR_LOCAL_SWITCH_SET  = 26,
   RDPA_DRV_LLID_GET             = 27,
   RDPA_DRV_LLID_TM_SET          = 28,
   RDPA_DRV_LLID_TM_GET          = 29,
   RDPA_DRV_LLID_CTRL_TM_SET     = 30,
   RDPA_DRV_LLID_TM_ID_GET       = 31,
   RDPA_DRV_LLID_CTRL_EN_SET     = 32,
   RDPA_DRV_NEW_LLID_ALLOC       = 33,
   RDPA_DRV_QUEUE_STATS_GET      = 34,
   RDPA_DRV_TCONT_GET            = 35,
   RDPA_DRV_TCONT_TM_SET         = 36,
   RDPA_DRV_TCONT_TM_GET         = 37,
   RDPA_DRV_TCONT_TM_ID_GET      = 38,
   RDPA_DRV_Q_ID_NOT_VALID       = 39,
   RDPA_DRV_DSCP_TO_PBIT_GET     = 40,
   RDPA_DRV_D_TO_P_QOS_MAP_GET   = 41,
   RDPA_DRV_D_TO_P_QOS_MAP_SET   = 42,
   RDPA_DRV_D_TO_P_NO_CFG_BEFORE = 43,
   RDPA_DRV_NEW_D_TO_P_ALLOC     = 44,
   RDPA_DRV_NEW_D_TO_P_INDEX_GET = 45,
   RDPA_DRV_PBIT_TO_Q_GET        = 46,
   RDPA_DRV_P_TO_Q_QOS_MAP_GET   = 47,
   RDPA_DRV_P_TO_Q_QOS_MAP_SET   = 48,
   RDPA_DRV_NEW_P_TO_Q_ALLOC     = 49,
   RDPA_DRV_PKT_BASED_QOS_GET    = 50,
   RDPA_DRV_PKT_BASED_QOS_SET    = 51,
   RDPA_DRV_Q_OCCUPANCY_GET      = 52,
   RDPA_DRV_UNSUPPORTED          = 53
} rdpaDrvReturn_code_t;

/*
 *------------------------------------------------------------------------------
 * Common defines for RDPA layers.
 *------------------------------------------------------------------------------
 */
#undef RDPA_DECL
#define RDPA_DECL(x) _IO(RDPADRV_MAJOR,x)

typedef enum {
    RDPA_IOC_TM = RDPA_DECL(0),
    RDPA_IOC_IPTV = RDPA_DECL(1),
    RDPA_IOC_IC = RDPA_DECL(2),
    RDPA_IOC_SYS = RDPA_DECL(3),
    RDPA_IOC_PORT = RDPA_DECL(4),
    RDPA_IOC_BRIDGE = RDPA_DECL(5),
    RDPA_IOC_LLID = RDPA_DECL(6),
    RDPA_IOC_DS_WAN_UDP_FILTER = RDPA_DECL(7),
    RDPA_IOC_RDPA_MW_SET_MCAST_DSCP_REMARK = RDPA_DECL(8),
    RDPA_IOC_TIME_SYNC = RDPA_DECL(10),
    RDPA_IOC_FILTERS = RDPA_DECL(11),
    RDPA_IOC_DSCP_TO_PBIT = RDPA_DECL(12),
    RDPA_IOC_PBIT_TO_Q = RDPA_DECL(13),
    RDPA_IOC_MISC = RDPA_DECL(14),
    RDPA_IOC_MAX = RDPA_DECL(15),
} rdpa_drv_ioctl_t;
    
#define RDPACTL_MCAST_REMARK_DISABLE -1

typedef enum {
   RDPA_IOCTL_DEV_PORT,
   RDPA_IOCTL_DEV_LLID,
   RDPA_IOCTL_DEV_TCONT,
   RDPA_IOCTL_DEV_XTM,
   RDPA_IOCTL_DEV_NONE,
   RDPA_IOCTL_DEV_TYPE_MAX
} rdpa_drv_ioctl_dev_type;

/** IP address family */
typedef enum {
    rdpactl_ip_family_ipv4,
    rdpactl_ip_family_ipv6
} rdpactl_ip_family;

/** IPv4 address */
typedef uint32_t rdpactl_ipv4;

/** IPv6 address */
typedef struct
{
    uint8_t data[16];
} rdpactl_ipv6_t;

/** IPv4 or IPv6 address */
typedef struct {
    bdmf_ip_family   family;      /**< Address family: IPv4 / IPv6 */
    union {
        rdpactl_ipv4 ipv4;         /**< IPv4 address */
        rdpactl_ipv6_t ipv6;       /**< IPv6 address */
    } addr;
} rdpactl_ip_t;

typedef enum {
   RDPA_IOCTL_TM_CMD_GET_ROOT_TM       = 0,
   RDPA_IOCTL_TM_GET_BY_QID            = 1,
   RDPA_IOCTL_TM_CMD_ROOT_TM_CONFIG    = 2,
   RDPA_IOCTL_TM_CMD_TM_CONFIG         = 3,
   RDPA_IOCTL_TM_CMD_ROOT_TM_REMOVE    = 4,
   RDPA_IOCTL_TM_CMD_TM_REMOVE         = 5,
   RDPA_IOCTL_TM_CMD_QUEUE_CONFIG      = 6,
   RDPA_IOCTL_TM_CMD_QUEUE_REMOVE      = 7,
   RDPA_IOCTL_TM_CMD_GET_ROOT_SP_TM    = 8,
   RDPA_IOCTL_TM_CMD_GET_ROOT_WRR_TM   = 9,
   RDPA_IOCTL_TM_CMD_GET_PORT_ORL      = 10,
   RDPA_IOCTL_TM_CMD_ORL_CONFIG        = 11,
   RDPA_IOCTL_TM_CMD_ORL_REMOVE        = 12,
   RDPA_IOCTL_TM_CMD_ORL_LINK          = 13,
   RDPA_IOCTL_TM_CMD_ORL_UNLINK        = 14,
   RDPA_IOCTL_TM_CMD_TM_RL_CONFIG      = 15,
   RDPA_IOCTL_TM_CMD_GET_QUEUE_CONFIG  = 16,
   RDPA_IOCTL_TM_CMD_GET_TM_CAPS       = 17,
   RDPA_IOCTL_TM_CMD_QUEUE_ALLOCATE    = 18,
   RDPA_IOCTL_TM_CMD_QUEUE_DISLOCATE   = 19,
   RDPA_IOCTL_TM_CMD_GET_QUEUE_STATS   = 20,
   RDPA_IOCTL_TM_CMD_GET_TM_CONFIG     = 21,
   RDPA_IOCTL_TM_CMD_SET_Q_DROP_ALG    = 22,
   RDPA_IOCTL_TM_CMD_SET_Q_SIZE        = 23,
   RDPA_IOCTL_TM_CMD_SET_Q_SHAPER      = 24,
   RDPA_IOCTL_TM_CMD_SVC_Q_ENABLE_GET  = 25,
   RDPA_IOCTL_TM_CMD_SVC_Q_ENABLE_SET  = 26,
   RDPA_IOCTL_TM_CMD_GET_BEST_EFFORT_TM_ID = 27,
   RDPA_IOCTL_TM_CMD_GET_TM_SUBSIDIARY = 28,
   RDPA_IOCTL_TM_CMD_MAX
} rdpa_drv_ioctl_tm_cmd_t;

typedef struct {
   rdpa_drv_ioctl_tm_cmd_t cmd;
   rdpa_drv_ioctl_dev_type dev_type;
   uint32_t dev_id;
   uint32_t root_tm_id;
   uint32_t tm_id;
   uint32_t dir;
   uint32_t q_id;
   uint32_t index;
   uint32_t level;            /* Next TM level */
   uint32_t arbiter_mode;
   uint32_t rl_mode;
   uint32_t priority;
   uint32_t qsize;
   uint32_t min_rate;
   uint32_t shaping_rate;
   uint32_t burst;
   uint32_t weight;
   uint32_t minBufs;
   uint32_t port_sched_caps;  /* port tm setting */
   uint32_t max_queues;       /* port tm setting */
   uint32_t max_sp_queues;    /* port tm setting */
   uint32_t drop_alg;
   uint32_t red_min_thr_lo;
   uint32_t red_max_thr_lo;
   uint32_t red_drop_percent_lo;
   uint32_t red_min_thr_hi;
   uint32_t red_max_thr_hi;
   uint32_t red_drop_percent_hi;
   uint32_t priority_mask_0;
   uint32_t priority_mask_1;
   uint32_t cfg_flags;
   BOOL     port_shaper;      /* port tm setting */
   BOOL     queue_shaper;     /* port tm setting */
   BOOL     orl_linked;
   BOOL     found;
   BOOL     service_queue;
   BOOL     q_clean;
   BOOL     best_effort;
   rdpa_stat_1way_t qstats;
} rdpa_drv_ioctl_tm_t;

typedef enum
{
    RDPA_L3_PROTOCOL_OTHER = 0, /**< IC L3 Protocol field = Other */
    RDPA_L3_PROTOCOL_IPV4  = 1, /**< IC L3 Protocol field = IPv4 */
    RDPA_L3_PROTOCOL_IPV6  = 2  /**< IC L3 Protocol field = IPv6 */
} rdpa_l3_protocol_t;

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908) && !defined(BCM63158)
typedef enum {
    RDPA_IOCTL_IPTV_CMD_LOOKUP_METHOD_SET,
    RDPA_IOCTL_IPTV_CMD_LOOKUP_METHOD_GET,
    RDPA_IOCTL_IPTV_CMD_ENTRY_ADD,
    RDPA_IOCTL_IPTV_CMD_ENTRY_REMOVE,
    RDPA_IOCTL_IPTV_CMD_ENTRY_FLUSH,
    RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_SET,
    RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_GET,	
    RDPA_IOCTL_IPTV_CMD_MAX
} rdpa_drv_ioctl_iptv_cmd_t;

typedef struct {
    bdmf_ip_family ip_family;
    union {
        uint8_t  mac[6];
        uint32_t ipv4;
        uint8_t  ipv6[16];
    } group; /**< multicast group */

    union {
        uint32_t ipv4;
        uint8_t  ipv6[16];
    } src_ip; /**< multicast ssm ip address */

    uint16_t  vid;
} rdpa_drv_ioctl_iptv_key_t;

typedef struct {
    rdpa_vlan_command            action;
    uint16_t                     vid;
} rdpa_drv_ioctl_iptv_vlan_entry_t;

typedef struct {
    rdpa_drv_ioctl_iptv_key_t         key;
    rdpa_drv_ioctl_iptv_vlan_entry_t  vlan;
} rdpa_drv_ioctl_iptv_entry_t;

typedef struct {
    rdpa_drv_ioctl_iptv_cmd_t           cmd;
    rdpa_iptv_lookup_method             method;
    rdpa_mcast_filter_method            filter_method;
    uint32_t                            egress_port;
    rdpa_drv_ioctl_iptv_entry_t         entry;
    uint32_t                            index;
} rdpa_drv_ioctl_iptv_t;
#endif

typedef enum
{
    RDPACTL_IF_LAN0 = 2,           /**< LAN0 port */
    RDPACTL_IF_LAN1,           /**< LAN1 port */
    RDPACTL_IF_LAN2,           /**< LAN2 port */
    RDPACTL_IF_LAN3,           /**< LAN3 port */
    RDPACTL_IF_LAN4,           /**< LAN4 port */
    RDPACTL_IF_LAN5,           /**< LAN5 port */
    RDPACTL_IF_LAN6,           /**< LAN6 port */
    RDPACTL_IF_LAN7,           /**< LAN7 port */   
    
    RDPACTL_IF_MAX,
} rdpactl_ingress_port;

typedef enum
{
    RDPACTL_IC_TRAP_REASON_0 = 0,  /** user defined 0 */
    RDPACTL_IC_TRAP_REASON_1 = 1,  /** user defined 1 */
    RDPACTL_IC_TRAP_REASON_2 = 2,  /** user defined 2 */
    RDPACTL_IC_TRAP_REASON_3 = 3,  /** user defined 3 */
    RDPACTL_IC_TRAP_REASON_4 = 4,   /** user defined 4 */
    RDPACTL_IC_TRAP_REASON_5 = 5,  /** user defined 5 */
    RDPACTL_IC_TRAP_REASON_6 = 6,  /** user defined 6 */
    RDPACTL_IC_TRAP_REASON_7 = 7   /** user defined 7 */
} rdpactl_ic_trap_reason;

#define RDPACTL_QUEUEID_BITS_NUMBER 16
#define RDPACTL_WANFLOW_MASK ((~0) << RDPACTL_QUEUEID_BITS_NUMBER)
#define RDPACTL_SERVICEACT_Q_MASK (0x10000UL)
#define RDPACTL_SERVICEQUEUE_MASK (0xFFFFUL)

typedef struct
{
    rdpa_ic_type type;
    rdpa_traffic_dir dir;
    //Classification Mask & rule priority
    uint8_t prty;  /**< Defined the priority of classifier. value between 0 - 256, 0 is highest priority */
    uint32_t field_mask; /**< Fields used for classification. A combination of rdpactl_ic_fields */
    uint16_t port_mask;

    rdpa_ic_gen_rule_cfg_t gen_rule_cfg1;
    rdpa_ic_gen_rule_cfg_t gen_rule_cfg2;
    rdpa_filter_location_t generic_filter_location; /**< All\Missed traffic */

    //Classification key
    bdmf_ip_family  ip_family;
    union {
        uint32_t ipv4;
        uint8_t ipv6[16];
    } src_ip; /**< source ipv4/ipv6 ip */
    union {
        uint32_t ipv4;
        uint8_t ipv6[16];
    } dst_ip;                   /**< dest ipv4/ipv6 ip */
    uint16_t src_port;          /**< source port */
    uint16_t dst_port;          /**< destination port */
    uint8_t protocol; /**< IP protocols. For example, UDP(17) */
    uint16_t outer_vid; /**< Outer VID */
    uint16_t inner_vid;   /**< Inner VID */
    uint8_t dst_mac[6];  /**<DA  MAC address */
    uint8_t src_mac[6];  /**<SA MAC address */
    uint16_t etype;  /**< Ethernet type */
    uint8_t dscp; /**< dscp val */
    uint8_t ingress_port_id; /**<DS- GEM or LLID index US - ingress port index */
    uint8_t outer_pbits;  /**< Outer pbit */
    uint8_t inner_pbits;  /**< Inner PBIT */
    uint8_t number_of_vlans; /**< number of vlans */
    uint32_t ipv6_label; /**< ipv6 label */
    uint16_t outer_tpid;  /**< Outer tpid */
    uint16_t inner_tpid;  /**< Inner tpid */
    uint8_t version;  
    uint32_t gen_rule_key_1;   /**< Key for first generic field matching */
    uint32_t gen_rule_key_2;   /**< Key for second generic field matching */    
    uint8_t ingress_wan_flow; /**<DS- GEM or LLID index */
    uint32_t generic_mask;    /**< mask per flow for generic key1 / used for XRDP */
    uint32_t generic_mask_2;  /**< mask per flow for generic key2 / used for XRDP */
    
    //Classification result
    uint8_t qos_method;
    uint8_t wan_flow;
    rdpa_forward_action action; /*< frame action */
    uint8_t forw_mode;
    uint8_t egress_port;
    uint32_t queue_id; /**< Egress queue id and wan flow */
    rdpa_vlan_action_cfg_t vlan_action;
    int8_t opbit_remark; /*-1: no remark */
    int8_t ipbit_remark; /*-1: no remark */
    int8_t dscp_remark; /*-1: no remark */
    int8_t pbit_to_gem;
    uint32_t shaping_rate;  //to be done
    uint32_t shaping_burst_size; //to be done
    uint32_t service_queue_info; 
    rdpactl_ic_trap_reason trap_reason;
} rdpactl_classification_rule_t ;

typedef enum {
    RDPA_IOCTL_IC_CMD_ADD_CLASSIFICATION_RULE   = 0,
    RDPA_IOCTL_IC_CMD_DEL_CLASSIFICATION_RULE   = 1,
    RDPA_IOCTL_IC_CMD_ADD   = 2,
    RDPA_IOCTL_IC_CMD_DEL   = 3,
    RDPA_IOCTL_IC_CMD_FIND  = 4,
} rdpa_drv_ioctl_ic_cmd_t;

typedef union {
        BCM_IOC_PTR(rdpactl_classification_rule_t*, rule);
        //a ingress classifier may be used by several flows. return the ic priority
        uint8_t prty;
} rdpa_drv_ioctl_ic_param_t;

typedef struct {
        rdpa_drv_ioctl_ic_cmd_t cmd;
        rdpa_drv_ioctl_ic_param_t param;
} rdpa_drv_ioctl_ic_t;


typedef enum {
    RDPA_IOCTL_SYS_CMD_WANTYPE_GET,
    RDPA_IOCTL_SYS_CMD_IN_TPID_GET,
    RDPA_IOCTL_SYS_CMD_IN_TPID_SET,
    RDPA_IOCTL_SYS_CMD_OUT_TPID_GET,
    RDPA_IOCTL_SYS_CMD_OUT_TPID_SET,
    RDPA_IOCTL_SYS_CMD_EPON_MODE_SET,
    RDPA_IOCTL_SYS_CMD_EPON_MODE_GET,
    RDPA_IOCTL_SYS_CMD_EPON_STATUS_GET,
    RDPA_IOCTL_SYS_CMD_ALWAYS_TPID_SET,
    RDPA_IOCTL_SYS_CMD_FORCE_DSCP_GET,
    RDPA_IOCTL_SYS_CMD_FORCE_DSCP_SET,
    RDPA_IOCTL_SYS_CMD_CAR_MODE_SET,
    RDPA_IOCTL_SYS_CMD_DETECT_TPID_SET,
    RDPA_IOCTL_SYS_CMD_MAX
} rdpa_drv_ioctl_sys_cmd_t;

typedef struct {
    uint16_t    dir;
    BOOL        enable;
} rdpactl_force_dscp_t;

typedef struct {
    uint16_t    tpid;
    BOOL        is_inner;
} rdpactl_detect_tpid_t;

typedef union {
    int         rdpa_if;
    uint16_t    wan_type;
    uint16_t    inner_tpid;
    uint16_t    outer_tpid;
    uint16_t    epon_mode;
    uint16_t    epon_enable;
    uint16_t    always_tpid;
    rdpactl_force_dscp_t force_dscp;
    BOOL        car_mode;
    rdpactl_detect_tpid_t detect_tpid;
} rdpa_drv_ioctl_sys_param_t;

typedef struct {
    rdpa_drv_ioctl_sys_cmd_t    cmd;
    rdpa_drv_ioctl_sys_param_t  param;
} rdpa_drv_ioctl_sys_t;

typedef struct
{
    uint16_t max_sa;                        /**< Max number of SAs that can be learnt on the port */
    uint16_t num_sa;                        /**< RO: Number of SAs learnt on the port. Ignored when setting configuration */
} rdpactl_port_sa_limit_t;

typedef enum {
    RDPA_IOCTL_PORT_CMD_SA_LIMIT_GET,
    RDPA_IOCTL_PORT_CMD_SA_LIMIT_SET,
    RDPA_IOCTL_PORT_CMD_PARAM_GET,
    RDPA_IOCTL_PORT_CMD_PARAM_SET,
    RDPA_IOCTL_PORT_CMD_MAX
} rdpa_drv_ioctl_port_cmd_t;

typedef struct {
    BOOL        sal_enable;
    BOOL        dal_enable;
    uint8_t     sal_miss_action; /**< SA miss action */
    uint8_t     dal_miss_action; /**< DA miss action */
    rdpactl_port_sa_limit_t    sa_limit;
} rdpa_drv_ioctl_port_param_t;

typedef struct {
    rdpa_drv_ioctl_port_cmd_t       cmd;
    uint32_t                        port_idx;
    rdpa_drv_ioctl_port_param_t     param;
} rdpa_drv_ioctl_port_t;

typedef enum {
    RDPA_IOCTL_BR_CMD_FIND_OBJ,
    RDPA_IOCTL_BR_CMD_LOCAL_SWITCH_SET,
    RDPA_IOCTL_BR_CMD_MAX
} rdpa_drv_ioctl_br_cmd_t;

typedef struct {
    rdpa_drv_ioctl_br_cmd_t       cmd;
    uint8_t                       br_index;
    BOOL                          found;
    BOOL                          local_switch;
} rdpa_drv_ioctl_br_t;

typedef enum {
    RDPA_IOCTL_LLID_CMD_NEW,
    RDPA_IOCTL_LLID_CMD_MAX
} rdpa_drv_ioctl_llid_cmd_t;

typedef struct {
    rdpa_drv_ioctl_llid_cmd_t     cmd;
    uint8_t                       llid_index;
} rdpa_drv_ioctl_llid_t;

typedef enum {
    RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_ADD,
    RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_DELETE,
    RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_GET,
    RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_MAX
} rdpa_drv_ioctl_ds_wan_udp_filter_cmd_t;

typedef struct {
    int32_t  index;
    uint32_t offset;
    uint32_t value;
    uint32_t mask;
    uint32_t hits;
} rdpactl_ds_wan_udp_filter_t;

typedef struct {
    rdpa_drv_ioctl_ds_wan_udp_filter_cmd_t cmd;
    rdpactl_ds_wan_udp_filter_t filter;
} rdpa_drv_ioctl_ds_wan_udp_filter_t;


typedef enum {
     RDPA_IOCTL_FILTER_CMD_ADD_ENTRY,
     RDPA_IOCTL_FILTER_CMD_GLOBAL_CFG,
     RDPA_IOCTL_FILTER_CMD_ETYPE_UDEF_CFG,
     RDPA_IOCTL_FILTER_CMD_TPID_VALS_CFG,
     RDPA_IOCTL_FILTER_CMD_OUI_CFG,
     RDPA_IOCTL_FILTER_CMD_GET_STAT,
 } rdpa_drv_ioctl_filter_cmd_t;

typedef struct {
    rdpa_filter_tpid_vals_t tpid_vals;
    BOOL tpid_direction;
    rdpa_filter_global_cfg_t global_cfg;
    rdpa_filter_ctrl_t ctrl;
    rdpa_filter_key_t key;
    rdpa_filter_oui_val_key_t oui_val_key;
    uint32_t oui_val;
    uint32_t udef_inx;
    uint64_aligned udef_val;
    rdpa_filter_stats_key_t stats_params;
    int64_aligned stats_val;
 } rdpa_drv_ioctl_filter_param_t;

typedef struct {
    rdpa_drv_ioctl_filter_cmd_t  cmd;
    rdpa_drv_ioctl_filter_param_t param;
} rdpa_drv_ioctl_filter_t;

typedef enum {
   RDPA_IOCTL_D_TO_P_CMD_GET        = 0,
   RDPA_IOCTL_D_TO_P_CMD_SET        = 1,
   RDPA_IOCTL_D_TO_P_CMD_MAX
} rdpa_drv_ioctl_dscp_to_pbit_cmd_t;

typedef struct {
   rdpa_drv_ioctl_dscp_to_pbit_cmd_t cmd;
   BOOL     found;
   uint32_t dscp_pbit_map[64];
} rdpa_drv_ioctl_dscp_to_pbit_t;

typedef enum {
   RDPA_IOCTL_P_TO_Q_CMD_GET  = 0,
   RDPA_IOCTL_P_TO_Q_CMD_SET  = 1,
   RDPA_IOCTL_P_TO_Q_CMD_MAX
} rdpa_drv_ioctl_pbit_to_q_cmd_t;

typedef struct {
   rdpa_drv_ioctl_dscp_to_pbit_cmd_t cmd;
   rdpa_drv_ioctl_dev_type dev_type;
   uint32_t dev_id;
   BOOL     found;
   uint32_t pbit_q_map[8];
} rdpa_drv_ioctl_pbit_to_q_t;

typedef enum {
   RDPA_IOCTL_MISC_CMD_PKT_BASED_QOS_GET  = 0,
   RDPA_IOCTL_MISC_CMD_PKT_BASED_QOS_SET  = 1,
   RDPA_IOCTL_MISC_CMD_MAX
} rdpa_drv_ioctl_misc_cmd_t;

typedef struct {
   rdpa_drv_ioctl_misc_cmd_t cmd;
   uint32_t dir;
   uint32_t type;
   BOOL     enable;
} rdpa_drv_ioctl_misc_t;

/* Same definition as bcmVlan_dpCode_t. */
typedef enum
{
    RDPADRV_DP_CODE_NONE = 0,
    RDPADRV_DP_CODE_INTERNAL,
    RDPADRV_DP_CODE_DEI,
    RDPADRV_DP_CODE_PCP8P0D,
    RDPADRV_DP_CODE_PCP7P1D,
    RDPADRV_DP_CODE_PCP6P2D,
    RDPADRV_DP_CODE_PCP5P3D,
    RDPADRV_DP_CODE_DSCPAF,
    RDPADRV_DP_CODE_MAX
} rdpadrv_dp_code;


#endif /* __RDPA_DRV__H_INCLUDED__ */
