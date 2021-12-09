#ifndef __BCMDPI_H_INCLUDED__
#define __BCMDPI_H_INCLUDED__
/*
 * <:copyright-BRCM:2017:DUAL/GPL:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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

#define DPI_APPID_INVALID       0
#define DPICTL_MAC_FMT          "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx"

/* put netlink defines here since toolchain does not have updated
 * uapi headers */
#define CTA_DPI                 24
#define CTA_DPI_APP_ID          1
#define CTA_DPI_MAC             2
#define CTA_DPI_STATUS          3
#define CTA_DPI_URL             4
#define CTA_DPI_MAX             4

typedef struct
{
    uint16_t type;
    uint16_t len;
} DpictlMsgHdr_t;

typedef enum
{
    DPICTL_NLMSG_BASE = 0,
    DPICTL_NLMSG_ENABLE,
    DPICTL_NLMSG_DISABLE,
    DPICTL_NLMSG_STATUS,
    DPICTL_NLMSG_TABLE_START,
    DPICTL_NLMSG_TABLE_ENTRY,
    DPICTL_NLMSG_TABLE_STOP,
    DPICTL_NLMSG_AVAIL_BW,
    DPICTL_NLMSG_DEFAULT_BW,
    DPICTL_NLMSG_MAXPKT,
    DPICTL_NLMSG_RESET_STATS,
    DPICTL_NLMSG_MAX
} DpictlNlMsgType_t;

typedef struct
{
    uint32_t avail_kbps;
    uint32_t default_kbps;
    uint32_t remain_kbps;
} DpictlBwConfig_t;

typedef enum {
    DPICTL_TABLE_TYPE_BWT,   /* Bandwidth Table */
    DPICTL_TABLE_TYPE_ACT,   /* Application Class Table */
    DPICTL_TABLE_TYPE_ACM,   /* Application Class Mapping Table */
    DPICTL_TABLE_TYPE_DCT,   /* Device Class Table */
    DPICTL_TABLE_TYPE_DCM,   /* Device Class Mapping Table */
    DPICTL_TABLE_TYPE_DPT,   /* Device Priority Table */
    DPICTL_TABLE_TYPE_KPT,   /* Known Device Priority Table */
    DPICTL_TABLE_TYPE_MAX
} DpictlTableType_t;

/* DPI Engine Bandwidth Table */
#define DPICTL_BWT_ENTRY_FMT        "%hhu,%hu,%hhu,%u,%u"
#define DPICTL_BWT_ENTRY_FMT_ITEMS  5
typedef struct {
    uint8_t  cat_id;      /* Category ID */
    uint16_t app_id;      /* Application ID */
    uint8_t  stream_type; /* Stream Type: 1=VOIP, 2=AUDIO, 4=VIDEO */
    uint32_t resolution;  /* Resolution */
    uint32_t kbps;        /* Bandwidth, in kbps */
} DpictlBwTableEntry_t;

/* User-defined Application Classes */
#define DPICTL_ACT_ENTRY_FMT        "%u,%hhu,%d,%u,%d"
#define DPICTL_ACT_ENTRY_FMT_ITEMS  5
typedef struct {
    uint32_t app_class_id;       /* Application Class ID */
    uint8_t  uniquify;           /* 1 = app queue; 0 = class queue; */
    int32_t  app_class_priority; /* Application Class priority */
    uint32_t app_class_kbps;     /* Application Class bandwidth, in Kbps */
    int32_t  app_class_us_queue; /* Application Class upstream queue id */
} DpictlAppClassTableEntry_t;

/* Application Behavior (app_id + beh_id) -> Application Class */
#define DPICTL_ACM_ENTRY_FMT        "%hhu,%hu,%hhu,%u"
#define DPICTL_ACM_ENTRY_FMT_ITEMS  4
typedef struct {
    uint8_t  cat_id;       /* Category ID */
    uint16_t app_id;       /* Application ID */
    uint8_t  beh_id;       /* Behavior ID */
    uint32_t app_class_id; /* Application Class ID */
} DpictlAppClassMappingEntry_t;

/* DPI Engine Device ID -> Device Class */
#define DPICTL_DCM_ENTRY_FMT        "%u,%u"
#define DPICTL_DCM_ENTRY_FMT_ITEMS  2
typedef struct {
    uint32_t dev_id;        /* Device ID (e.g. "iPhone 4") */
    uint32_t dev_class_id;  /* Device Class ID */
} DpictlDevClassMappingEntry_t;

/* Application Class + Device Class -> Device Priority */
#define DPICTL_DPT_ENTRY_FMT        "%u,%u,%d"
#define DPICTL_DPT_ENTRY_FMT_ITEMS  3
typedef struct {
    uint32_t app_class_id;  /* Application Class ID */
    uint32_t dev_class_id;  /* Device Class ID */
    int32_t  dev_priority;  /* Device Priority */
} DpictlDevPriorityTableEntry_t;

/* Application Class + Known Device Key -> Known Device Priority */
#define DPICTL_KPT_ENTRY_FMT        "%u," DPICTL_MAC_FMT ",%d"
#define DPICTL_KPT_ENTRY_FMT_ITEMS  8
typedef struct {
    uint32_t app_class_id;  /* Application Class ID */
    uint8_t  mac[6];        /* Known Device MAC */
    int32_t  kdev_priority;  /* Known Device priority */
} DpictlKnownDevPriorityTableEntry_t;

typedef struct {
    DpictlTableType_t type;
    union {
        int alloc_entries;
        DpictlBwTableEntry_t bwt;
        DpictlAppClassTableEntry_t act;
        DpictlAppClassMappingEntry_t acm;
        DpictlDevClassMappingEntry_t dcm;
        DpictlDevPriorityTableEntry_t dpt;
        DpictlKnownDevPriorityTableEntry_t kpt;
    } data;
} DpictlTableMsg_t;

#define DPICTL_QOS_ENTRY_HEADER     "app_id mac ds_queue kbps us_queue flow_count"
#define DPICTL_QOS_ENTRY_FMT        "%u " DPICTL_MAC_FMT " %d %u %d %u"
#define DPICTL_QOS_ENTRY_FMT_ITEMS  11

#define dpi_get_app_id(cat_id, app_id, beh_id)  \
    (uint32_t) ( \
        (uint32_t) (((uint8_t) cat_id) << 24) | \
        (uint32_t) (((uint16_t) app_id) << 8) | \
        (uint32_t) ((uint8_t) beh_id) \
    )

#define dpi_get_unknown_app_id()       dpi_get_app_id(0, 0, 0)

#define dpi_get_app_id_cat_id(app_id)  (uint8_t)((uint32_t)(app_id) >> 24)
#define dpi_get_app_id_app_id(app_id)  (uint16_t)(((uint32_t)(app_id) >> 8) & 0xFFFF)
#define dpi_get_app_id_beh_id(app_id)  (uint8_t)((uint32_t)(app_id) & 0xFF)

#endif
