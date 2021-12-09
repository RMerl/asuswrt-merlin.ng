#if defined(CONFIG_BCM_KF_LOG)
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
    BCM_LOG_ID_FAP,
    BCM_LOG_ID_FAPPROTO,
    BCM_LOG_ID_FAP4KE,
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
        {.logId = BCM_LOG_ID_GPON_SERDES, .name = "gponSerdes", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_FAP, .name = "fap", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_FAPPROTO, .name = "fapProto", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_FAP4KE, .name = "fap4ke", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
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
#if defined(CONFIG_BCM_GMAC)
    BCM_FUN_ID_ENET_GMAC_ACTIVE,
    BCM_FUN_ID_ENET_GMAC_PORT,
#endif
    BCM_FUN_ID_ENET_IS_WAN_PORT, /* Take Logical port number as argument */
    BCM_FUN_ID_ENET_IS_SWSWITCH_PORT,
    BCM_FUN_ID_ENET_LAG_PORT_GET,
    BCM_FUN_ID_ENET_PORT_ROLE_NOTIFY,
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
    /* The arguments of the FAP functions are defined in fap.h */
    BCM_FUN_ID_FAP_PERF_ENABLE,
    BCM_FUN_ID_FAP_PERF_DISABLE,
    BCM_FUN_ID_FAP_PERF_SET_GENERATOR,
    BCM_FUN_ID_FAP_PERF_SET_ANALYZER,
    BCM_FUN_ID_FAP_PERF_GET_RESULTS,
    /* Kernel Bonding Driver related function */
    BCM_FUN_ID_ENET_MAX_BONDS,
    BCM_FUN_ID_ENET_BONDING_CHANGE,
    BCM_FUN_ID_BOND_GET_MSTR_INFO,
    BCM_FUN_ID_BOND_CLR_SLAVE_STAT,
    BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT, /* Expects Logical port number as argument */
    BCM_FUN_ID_BOND_IS_DEV_IN_SLAVE_PATH, 
    BCM_FUN_ID_BOND_RX_HANDLER, 
    /* DSL Runner Hooks */
    BCM_FUN_ID_RUNNER_PREPEND,
    BCM_FUN_ID_TCPSPDTEST_CONNECT,
    BCM_FUN_ID_MAX
} bcmFunId_t;

/* Structures passed in above function calls */
typedef struct {
    struct net_device *dev;         /* Input */
    struct net_device *mstr_dev;    /* Output */
}BCM_BondingMasterInfo;

typedef struct {
    uint16_t is_join;               /* Input */
    uint16_t bonding_group_id;      /* Input */
    struct net_device *slave_dev;   /* Input */
    struct net_device *bond_dev;    /* Input */
}BCM_EnetBondingInfo;

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

#define BCM_RUNNER_PREPEND_SIZE_MAX  32 /* Increasing the size of the prepend data buffer will require
                                           recompiling the pktrunner driver files released as binary */

typedef struct {
    void *blog_p;      /* INPUT: Pointer to the Blog_t structure that triggered the Runner flow creation */
    uint8_t data[BCM_RUNNER_PREPEND_SIZE_MAX]; /* INPUT: The data that will be be prepended to all packets
                                                  forwarded by Runner that match the given Blog/Flow.
                                                  The data must be stored in NETWWORK BYTE ORDER */
    unsigned int size; /* OUTPUT: Size of the prepend data, up to 32 bytes long.
                          When no data is to be prepended, specify size = 0 */
} BCM_runnerPrepend_t;

typedef struct {
    unsigned int mac_id;
    unsigned int is_wan;
}BCM_EnetPortRole_t;

#endif /* _BCM_LOG_MODULES_ */

#endif
