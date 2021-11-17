#if defined(CONFIG_BCM_KF_LOG)
/*
* <:copyright-BRCM:2010:DUAL/GPL:standard
* 
*    Copyright (c) 2010 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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
    BCM_FUN_ID_MAX
} bcmFunId_t;

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

#endif /* _BCM_LOG_MODULES_ */
#endif
