/*
* <:copyright-BRCM:2010:DUAL/GPL:standard
* 
*    Copyright (c) 2010 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>

*/

#ifndef _BCM_LOG_MODULES_
#define _BCM_LOG_MODULES_

typedef enum {
    BCM_LOG_LEVEL_ERROR=0,
    BCM_LOG_LEVEL_NOTICE,
    BCM_LOG_LEVEL_INFO,
    BCM_LOG_LEVEL_DEBUG,
    BCM_LOG_LEVEL_MAX
} bcmLogLevel_t;

/* To support a new module, create a new log ID in bcmLogId_t,
   and a new entry in BCM_LOG_MODULE_INFO */


typedef enum {
    BCM_LOG_ID_LOG=0,
    BCM_LOG_ID_VLAN,
    BCM_LOG_ID_GPON,
    BCM_LOG_ID_PLOAM,
    BCM_LOG_ID_PLOAM_FSM,
    BCM_LOG_ID_PLOAM_HAL,
    BCM_LOG_ID_PLOAM_PORT,
    BCM_LOG_ID_PLOAM_ALARM,
    BCM_LOG_ID_OMCI,
    BCM_LOG_ID_I2C,
    BCM_LOG_ID_ENET,
    BCM_LOG_ID_GPON_SERDES,
    BCM_LOG_ID_AE,
    BCM_LOG_ID_XTM,
    BCM_LOG_ID_IQ,
    BCM_LOG_ID_BPM,
    BCM_LOG_ID_ARL,
    BCM_LOG_ID_EPON,
    BCM_LOG_ID_GMAC,   
    BCM_LOG_ID_RDPA,
    BCM_LOG_ID_RDPA_CMD_DRV,
    BCM_LOG_ID_PKTRUNNER,
    BCM_LOG_ID_SIM_CARD,
    BCM_LOG_ID_PMD,
    BCM_LOG_ID_TM,
    BCM_LOG_ID_SPDSVC,
    BCM_LOG_ID_MCAST,
    BCM_LOG_ID_DPI,
    BCM_LOG_ID_CMDLIST,
    BCM_LOG_ID_ARCHER,
    BCM_LOG_ID_TOD,
    BCM_LOG_ID_PON_PWM,
    BCM_LOG_ID_OPTICALDET,
    BCM_LOG_ID_WANTYPEDET,
    BCM_LOG_ID_XPORT,
    BCM_LOG_ID_BCMLIBS_BIT_POOL,
    BCM_LOG_ID_MPM,
    BCM_LOG_ID_BP3,
    BCM_LOG_ID_FPI,
    BCM_LOG_ID_HWF,
    BCM_LOG_ID_MAX
} bcmLogId_t;

#define BCM_LOG_MODULE_INFO                             \
    {                                                   \
        {.logId = BCM_LOG_ID_LOG, .name = "bcmlog", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_VLAN, .name = "vlan", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_GPON, .name = "gpon", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM, .name = "ploam", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM_FSM, .name = "ploamFsm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM_HAL, .name = "ploamHal", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM_PORT, .name = "ploamPort", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM_ALARM, .name = "ploamAlarm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_OMCI, .name = "omci", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_I2C, .name = "i2c", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_ENET, .name = "enet", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_GPON_SERDES, .name = "ponserdes", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_AE, .name = "ae", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_XTM, .name = "xtm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_IQ, .name = "iq", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_BPM, .name = "bpm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_ARL, .name = "arl", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_EPON, .name = "eponlue", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_GMAC, .name = "gmac", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_RDPA, .name = "rdpa", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_RDPA_CMD_DRV, .name = "rdpadrv", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PKTRUNNER, .name = "pktrunner", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_SIM_CARD, .name = "sim_card", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PMD, .name = "pmd", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_TM, .name = "tm", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_SPDSVC, .name = "spdsvc", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_MCAST, .name = "mcast", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_DPI, .name = "dpi", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_CMDLIST, .name = "cmdlist", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_ARCHER, .name = "archer", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_TOD, .name = "tod", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PON_PWM, .name = "pon_pwm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_OPTICALDET, .name = "opticaldet", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_WANTYPEDET, .name = "wantypedet", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_XPORT, .name = "xport", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_BCMLIBS_BIT_POOL, .name = "bitpool", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_MPM, .name = "mpm", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_BP3, .name = "bp3", .logLevel = BCM_LOG_LEVEL_DEBUG}, \
        {.logId = BCM_LOG_ID_FPI, .name = "fpi", .logLevel = BCM_LOG_LEVEL_DEBUG}, \
        {.logId = BCM_LOG_ID_HWF, .name = "hwf", .logLevel = BCM_LOG_LEVEL_DEBUG}, \
    }

/* To support a new registered function,
 * create a new BCM_FUN_ID */

typedef enum {
    BCM_FUN_ID_RESET_SWITCH=0,
    BCM_FUN_ID_ENET_LINK_CHG,
    BCM_FUN_ID_ENET_CHECK_SWITCH_LOCKUP,
    BCM_FUN_ID_ENET_GET_PORT_BUF_USAGE,
    BCM_FUN_ID_GPON_GET_GEM_PID_QUEUE,
    BCM_FUN_ID_ENET_HANDLE,
    BCM_FUN_ID_EPON_HANDLE,
    BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY,
    BCM_FUN_ID_ENET_IS_WAN_PORT, /* Take Logical port number as argument */
    BCM_FUN_ID_ENET_IS_SWSWITCH_PORT,
    BCM_FUN_ID_ENET_PORT_ROLE_NOTIFY,
    BCM_FUN_ID_ENET_BOND_RX_PORT_MAP,
    BCM_FUN_ID_ENET_SYSPORT_CONFIG,
    BCM_FUN_ID_ENET_SYSPORT_QUEUE_MAP,
    BCM_FUN_ID_ENET_REMAP_TX_QUEUE,
    BCM_FUN_ID_ENET_PHY_SPEED_SET,
    BCM_FUN_ID_ENET_TM_EN_SET,
    /* The arguments of the BCM TM functions are defined by bcmTmDrv_arg_t */
    BCM_FUN_ID_TM_REGISTER,
    BCM_FUN_ID_TM_PORT_CONFIG,
    BCM_FUN_ID_TM_PORT_ENABLE,
    BCM_FUN_ID_TM_ARBITER_CONFIG,
    BCM_FUN_ID_TM_QUEUE_CONFIG,
    BCM_FUN_ID_TM_APPLY,
    BCM_FUN_ID_TM_ENQUEUE,
    /* The arguments of the Speed Service functions are defined in spdsvc_defs.h */
    BCM_FUN_ID_SPDSVC_TRANSMIT,
    BCM_FUN_ID_SPDSVC_RECEIVE,
    /* Kernel Bonding Driver related function */
    BCM_FUN_ID_BOND_CLR_SLAVE_STAT,
    BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT, /* Expects Logical port number as argument */
    BCM_FUN_ID_ENET_IS_DEV_IN_SLAVE_PATH, 
    BCM_FUN_ID_BOND_RX_HANDLER, 
    BCM_FUN_ID_TCPSPDTEST_CONNECT,
    /* Archer Hooks */
    BCM_FUN_ID_ARCHER_HOST_BIND,
    BCM_FUN_ID_ARCHER_XTMRT_BIND,
    BCM_FUN_ID_ARCHER_WFD_BIND,
    BCM_FUN_ID_ARCHER_WFD_CONFIG,
    BCM_FUN_ID_ARCHER_WLAN_RX_REGISTER,
    BCM_FUN_ID_ARCHER_WLAN_RX_SEND,
    BCM_FUN_ID_ARCHER_WLAN_BIND,
    BCM_FUN_ID_ARCHER_WLAN_UNBIND,
    BCM_FUN_ID_ARCHER_DHD_BDMF_NEW_AND_SET,
    BCM_FUN_ID_ARCHER_DHD_INIT_CFG_SET,
    BCM_FUN_ID_ARCHER_DHD_FLUSH_SET,
    BCM_FUN_ID_ARCHER_DHD_FLOW_RING_ENABLE_SET,
    BCM_FUN_ID_ARCHER_DHD_FLOW_RING_UPDATE_SET,
    BCM_FUN_ID_ARCHER_DHD_RX_POST_INIT,
    BCM_FUN_ID_ARCHER_DHD_RX_POST_UNINIT,
    BCM_FUN_ID_ARCHER_DHD_RX_POST_REINIT,
    BCM_FUN_ID_ARCHER_DHD_CPU_GET,
    BCM_FUN_ID_ARCHER_DHD_CPU_NUM_QUEUES_GET,
    BCM_FUN_ID_ARCHER_DHD_CPU_RXQ_CFG_SET,
    BCM_FUN_ID_ARCHER_DHD_CPU_RXQ_EMPTY,
    BCM_FUN_ID_ARCHER_DHD_CPU_INDEX_GET,
    BCM_FUN_ID_ARCHER_DHD_CPU_INT_ENABLE,
    BCM_FUN_ID_ARCHER_DHD_CPU_INT_DISABLE,
    BCM_FUN_ID_ARCHER_DHD_CPU_INT_CLEAR,
    BCM_FUN_ID_ARCHER_DHD_WAKEUP_INFORMATION_GET,
    BCM_FUN_ID_ARCHER_DHD_COMPLETE_RING_CREATE,
    BCM_FUN_ID_ARCHER_DHD_COMPLETE_RING_DESTROY,
    BCM_FUN_ID_ARCHER_DHD_SEND_PACKET_TO_DONGLE,
    BCM_FUN_ID_ARCHER_DHD_CPU_PACKET_GET,
    BCM_FUN_ID_ARCHER_DHD_COMPLETE_MESSAGE_GET,
    BCM_FUN_ID_ARCHER_DHD_COMPLETE_WAKEUP,
    BCM_FUN_ID_ARCHER_DHD_BDMF_SYSB_RECYCLE,
    BCM_FUN_ID_ARCHER_DHD_BDMF_SYSB_DATABUF_FREE,
    BCM_FUN_ID_VLAN_LOOKUP_DP,
    /* WLAN Hooks */
    BCM_FUN_ID_WLAN_QUERY_BRIDGEFDB,
    BCM_FUN_ID_WLAN_UPDATE_BRIDGEFDB,
    BCM_FUN_ID_WLAN_PKTC_DEL_BY_MAC,
    BCM_FUN_ID_SPDT_RNR_TRANSMIT,
    /* hook for SPU offload session params */
    BCM_FUN_ID_SPUOFFLOAD_SESSION_PARM,
    BCM_FUN_ID_PMD_PRBS,
    BCM_FUN_ID_WAN_SERDES_CONFIG,
    BCM_FUN_ID_SEND_MESSAGE_TO_PON_DRV_TASK,
    BCM_FUN_ID_WAN_SERDES_TYPE_GET,
    BCM_FUN_ID_NETLINK_INVOKE_SERDES_JOB_WITH_OUTPUT,
    BCM_FUN_ID_NETLINK_INVOKE_SERDES_JOB,
    BCM_FUN_ID_EYESCOPE_MSG,
    BCM_FUN_ID_SYNCE_ETH_LINK_CHANGE,
    BCM_FUN_ID_WAN_SERDES_RESET_TXFIFO,
    BCM_FUN_ID_WAN_SERDES_SYNC_LOSS,
    BCM_FUN_ID_GPIO_REQUEST,
    BCM_FUN_ID_GPIO_DIR_OUT,
    BCM_FUN_ID_NCO_SW_HOLD_VAL_GET,
    /* hook for SPU offload stats update */
    BCM_FUN_ID_SPUOFFLOAD_STATS_UPDATE,
    BCM_FUN_ID_LICENSE_CHECK,
    /* GDX hooks to read/write prepend header */
    BCM_FUN_ID_HWACC_GDX_BIND,
    BCM_FUN_ID_HWACC_GDX_UNBIND,
    BCM_FUN_ID_PREPEND_FILL_BUF,
    BCM_FUN_ID_PREPEND_FILL_INFO_FROM_BUF,
    /* hooks for Crossbow CSPU offload */
    BCM_FUN_ID_ARCHER_CRYPTO_PROGRAM_PARM,
    BCM_FUN_ID_ARCHER_CRYPTO_STATS,
    BCM_FUN_ID_ARCHER_CRYPTO_US_SEND,
    BCM_FUN_ID_ARCHER_CRYPTO_OFFLOAD_BIND,
    BCM_FUN_ID_ARCHER_CRYPTO_OFFLOAD_UNBIND,
    /* hooks for Flow Stats */
    BCM_FUN_ID_FLOW_STAT_NF_UPDATE_SLOW,
    /* SPU session cleanup */
    BCM_FUN_ID_SPU_SESSION_DELETE,
    BCM_FUN_ID_SPU_PREPEND_HDR,
    BCM_FUN_ID_MAX
} bcmFunId_t;

/* Structures passed in above function calls */
typedef struct {
    struct net_device *slave_dev;   /* Input */
    struct net_device **bond_dev;    /* Input/Output */
}BCM_BondDevInfo;

/* Structures passed in above function calls */
typedef struct {
    uint16_t gemPortIndex; /* Input */
    uint16_t gemPortId;    /* Output */
    uint8_t  usQueueIdx;   /* Output */
}BCM_GponGemPidQueueInfo;

typedef enum {
    BCM_ENET_FUN_TYPE_LEARN_CTRL = 0,
    BCM_ENET_FUN_TYPE_ARL_WRITE,
    BCM_ENET_FUN_TYPE_AGE_PORT,
    BCM_ENET_FUN_TYPE_UNI_UNI_CTRL,
    BCM_ENET_FUN_TYPE_PORT_RX_CTRL,
    BCM_ENET_FUN_TYPE_GET_VPORT_CNT,
    BCM_ENET_FUN_TYPE_GET_IF_NAME_OF_VPORT,
    BCM_ENET_FUN_TYPE_GET_UNIPORT_MASK,
    BCM_ENET_FUN_TYPE_MAX
} bcmFun_Type_t;

typedef struct {
    uint16_t vid;
    uint16_t val;
    uint8_t mac[6];
} arlEntry_t;

typedef struct {
    bcmFun_Type_t type; /* Action Needed in Enet Driver */
    union {
        uint8_t port;
        uint8_t uniport_cnt;
        uint16_t portMask;
        arlEntry_t arl_entry;
    };
    char name[16];
    uint8_t enable;
}BCM_EnetHandle_t;

typedef enum {
    BCM_EPON_FUN_TYPE_UNI_UNI_CTRL = 0,
    BCM_EPON_FUN_TYPE_MAX
} bcmEponFun_Type_t;

typedef struct {
    bcmEponFun_Type_t type; /* Action Needed in Epon Driver */
    uint8_t enable;
}BCM_EponHandle_t;

typedef struct {
    uint8_t port; /* switch port */
    uint8_t enable; /* enable/disable the clock */
}BCM_CmfFfeClk_t;

#define BCM_HW_ACCEL_PREPEND_SIZE_MAX  120  /* Changing the size of the prepend data buffer will require
                                             recompiling the cmdlist driver files released as binary */

typedef struct {
    void *blog_p;      /* INPUT: Pointer to the Blog_t structure that triggered the Runner flow creation */
    uint8_t data[BCM_HW_ACCEL_PREPEND_SIZE_MAX]; /* INPUT: The data that will be be prepended to all packets
                                                  forwarded by Runner that match the given Blog/Flow.
                                                  The data must be stored in NETWWORK BYTE ORDER */
    unsigned int size; /* OUTPUT: Size of the prepend data, up to 120 bytes long.
                          When no data is to be prepended, specify size = 0 */
    union
    {
        uint32_t flags_union;
        struct
        {
            unsigned int extention_header_present : 1;
            unsigned int g9111_header_present : 1;
            unsigned int reserved : 30;
        };
    };
} BCM_HwAccelPrepend_t;

typedef struct {
    unsigned int sysport;
    unsigned int switch_id;
    unsigned int port;
    unsigned int is_wan;
} BCM_EnetPortRole_t;

#define BCM_ENET_SYSPORT_INTF_MAX       2
#define BCM_ENET_SYSPORT_BLOG_CHNL_MAX  8

typedef enum {
    BCM_ENET_SYSPORT_MODE_INVALID = 0,
    BCM_ENET_SYSPORT_MODE_PORT,
    BCM_ENET_SYSPORT_MODE_INTERNAL_BRCM_SW,
    BCM_ENET_SYSPORT_MODE_EXTERNAL_BRCM_SW,
    BCM_ENET_SYSPORT_MODE_STACKED_BRCM_SW,
    BCM_ENET_SYSPORT_MODE_STACKED_VLAN_SW,
    BCM_ENET_SYSPORT_MODE_MAX
} bcmSysport_Mode_t;

typedef struct {
    bcmSysport_Mode_t mode;
} bcmSysport_Sysport_t;

typedef struct {
    struct net_device *dev;
    int sysport;
    int switch_id;
    int port;
    int nbr_of_queues;
} bcmSysport_BlogChnl_t;

typedef struct {
    int nbr_of_sysports;
    bcmSysport_Sysport_t sysport[BCM_ENET_SYSPORT_INTF_MAX];
    int nbr_of_blog_channels;
    bcmSysport_BlogChnl_t blog_chnl[BCM_ENET_SYSPORT_BLOG_CHNL_MAX];
    int switch_parent_port;     /* parent port num if external switch exists, otherwise NO_EXT_SWITCH */
#define         NO_EXT_SWITCH           -1
    int ls_port_q_offset;       /* external switch port lightstacking port tx q start offset */
} bcmSysport_Config_t;

#define BCM_ENET_SYSPORT_QUEUE_MAP_PRIORITY_MAX  8

typedef struct {
    int blog_chnl;
    uint8_t priority_to_switch_queue[BCM_ENET_SYSPORT_QUEUE_MAP_PRIORITY_MAX];
} bcmSysport_QueueMap_t;

typedef struct {
    int blog_chnl;
    int blog_chnl_rx;
} bcmEnet_BondRxPortMap_t;

/* Structure used with BCM_FUN_ID_ENET_REMAP_TX_QUEUE */
typedef struct {
    uint8_t tx_queue;
    void *dev;
} bcmEnet_QueueReMap_t;

typedef struct {
    struct net_device *dev;
    int kbps;
} bcmSysport_PhySpeed_t;

typedef struct {
	uint16_t enable;
	uint8_t prbs_mode;
}pmd_pbrs_param;

typedef struct
{
    uint8_t *prepend_data; 
    void *blog_p; 
    int max_prep_size;
}GDX_prepend_fill_args_t;

typedef struct {
    int initialized;
    int (* gdx_acc_send_pkt)(void *arg_p);
    int (* gdx_miss_pkt_handler_cb)(void *arg_p);
    int (* gdx_hit_pkt_handler_cb)(void *arg_p, uint32_t val32);
    int (* gdx_fwd_pkt_done_cb)(void);
} gdx_acc_bind_arg_t;
#endif /* _BCM_LOG_MODULES_ */

