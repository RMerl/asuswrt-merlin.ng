#ifndef __BCMDPI_H_INCLUDED__
#define __BCMDPI_H_INCLUDED__
/*
<:copyright-BRCM:2014:DUAL/GPL:standard

   Copyright (c) 2014 Broadcom 
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

#define NETLINK_DPI             27
#define DPI_APPID_INVALID       0

typedef struct
{
    int sock_nl;
    char *sock_buff;
} DpictlNlInfo_t;

typedef struct
{
    unsigned short type;
    unsigned short len;
} DpictlMsgHdr_t;

typedef enum
{
    DPICTL_NLMSG_BASE = 0,
    DPICTL_NLMSG_ENABLE,
    DPICTL_NLMSG_DISABLE,
    DPICTL_NLMSG_STATUS,
    DPICTL_NLMSG_CONFIG_OPT,
    DPICTL_NLMSG_ADD_PARENTAL,
    DPICTL_NLMSG_DEL_PARENTAL,
    DPICTL_NLMSG_TABLE_START,
    DPICTL_NLMSG_TABLE_ENTRY,
    DPICTL_NLMSG_TABLE_STOP,
    DPICTL_NLMSG_AVAIL_BW,
    DPICTL_NLMSG_MAX
} DpictlNlMsgType_t;

#define DPICTL_NL_SET_HDR_TYPE(x, v)  (((DpictlMsgHdr_t *)x)->type = v)
#define DPICTL_NL_SET_HDR_LEN(x, v)  (((DpictlMsgHdr_t *)x)->len = v)

typedef enum
{
    DPI_CONFIG_BASE = 0,
    DPI_CONFIG_PARENTAL,
    DPI_CONFIG_QOS,
    DPI_CONFIG_MAX
} DpictlConfigOpt_t;

typedef struct
{
    unsigned short option;
    unsigned short value;
} DpictlConfig_t;

typedef struct
{
    int appid;
//    char mac[];
//    char url[];
} DpictlParentalConfig_t;

typedef struct
{
    int avail_kbps;
    int remain_kbps;
} DpictlAvailBw_t;

typedef enum {
    DPICTL_TABLE_TYPE_BWT=0, /* Bandwidth Table */
    DPICTL_TABLE_TYPE_ACT,   /* Application Class Table */
    DPICTL_TABLE_TYPE_ACM,   /* Application Class Mapping Table */
    DPICTL_TABLE_TYPE_DCT,   /* Device Class Table */
    DPICTL_TABLE_TYPE_DCM,   /* Device Class Mapping Table */
    DPICTL_TABLE_TYPE_DPT,   /* Device Priority Table */
    DPICTL_TABLE_TYPE_KPT,   /* Known Device Priority Table */
    DPICTL_TABLE_TYPE_MAX
} DpictlTableType_t;

#define DPICTL_BWT_ENTRY_FMT        "%u,%u,%u,%u,%u\n"
#define DPICTL_BWT_ENTRY_FMT_ITEMS  5

/* DPI Engine Bandwidth Table */
typedef struct {
    unsigned char cat_id;  // Category ID
    unsigned short app_id; // Application ID
    unsigned char stream_type;  // Stream Type: 1=VOIP, 2=AUDIO, 4=VIDEO
    unsigned int resolution; // Resolution
    unsigned int kbps; // Bandwidth, in kbps
} DpictlBwTableEntry_t;

#define DPICTL_ACT_ENTRY_FMT        "%u,%u,%u,%u\n"
#define DPICTL_ACT_ENTRY_FMT_ITEMS  4

/* User-defined Application Classes */
typedef struct {
    int app_class_id;  // Application Class ID
    unsigned char uniquify;  // 1 = app queue; 0 = class queue;
    int app_class_priority; // Application Class priority
    int app_class_kbps; // Application Class bandwidth, in Kbps
} DpictlAppClassTableEntry_t;

#define DPICTL_ACM_ENTRY_FMT        "%u,%u,%u,%u\n"
#define DPICTL_ACM_ENTRY_FMT_ITEMS  4

/* Application Behavior (app_id + beh_id) -> Application Class */
typedef struct {
    unsigned char cat_id;  // Category ID
    unsigned short app_id; // Application ID
    unsigned char beh_id;  // Behavior ID
    int app_class_id;  // Application Class ID
} DpictlAppClassMappingEntry_t;

#define DPICTL_DCM_ENTRY_FMT        "%u,%u\n"
#define DPICTL_DCM_ENTRY_FMT_ITEMS  2

/* DPI Engine Device ID -> Device Class */
typedef struct {
    unsigned int dev_id; // Device ID (e.g. "iPhone 4")
    int dev_class_id;  // Device Class ID
} DpictlDevClassMappingEntry_t;

#define DPICTL_DPT_ENTRY_FMT        "%u,%u,%u\n"
#define DPICTL_DPT_ENTRY_FMT_ITEMS  3

/* Application Class + Device Class -> Device Priority */
typedef struct {
    int app_class_id;  // Application Class ID
    int dev_class_id;  // Device Class ID
    int dev_priority;  // Device Priority
} DpictlDevPriorityTableEntry_t;

#define DPICTL_KPT_ENTRY_FMT        "%u,%u,%u\n"
#define DPICTL_KPT_ENTRY_FMT_ITEMS  3

/* Application Class + Known Device Key -> Known Device Priority */
typedef struct {
    int app_class_id;  // Application Class ID
    unsigned short dev_key; // Known Device Key
    int kdev_priority; // Known Device priority
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

#define DPICTL_QOS_ENTRY_HEADER     "appId devKey queueIdx Kbps Mac Vendor OS Class Type Dev"
#define DPICTL_QOS_MAC_FMT          "%02x:%02x:%02x:%02x:%02x:%02x"
#define DPICTL_QOS_ENTRY_FMT        "%u %u %u %u " DPICTL_QOS_MAC_FMT " %u %u %u %u %u"
#define DPICTL_QOS_ENTRY_FMT_ITEMS  15

typedef struct {
    unsigned int app_id; // cat_id + app_id + beh_id
    unsigned short dev_key;
    unsigned short queue_index;
    unsigned int kbps;
    unsigned char mac[6]; // Device MAC Address
    unsigned short vendor_id; //!< Vendor (e.g. "Microsoft")
    unsigned short os_id; //!< OS/Device name (e.g. "Windows 8", or "iPhone 4")
    unsigned short class_id; //!< OS Class (e.g. "Windows")
    unsigned short type_id; //!< Device Type (e.g. "Phone")
    unsigned int dev_id; //!< Device Name (e.g. "iPhone 4")
} DpictlQosEntry_t;

#define dpi_get_app_id(cat_id, app_id, beh_id)  \
    (uint32_t) ( \
        (uint32_t) (((uint8_t) cat_id) << 24) | \
        (uint32_t) (((uint16_t) app_id) << 8) | \
        (uint32_t) ((uint8_t) beh_id) \
    )

#define dpi_get_unknown_app_id()    dpi_get_app_id(0, 0, 0)

#define dpi_get_app_id_cat_id(app_id)  (uint8_t)((uint32_t)(app_id) >> 24)
#define dpi_get_app_id_app_id(app_id)  (uint16_t)(((uint32_t)(app_id) >> 8) & 0xFFFF)
#define dpi_get_app_id_beh_id(app_id)  (uint8_t)((uint32_t)(app_id) & 0xFF)

#endif
