/***********************************************************************
 *
 * Copyright (c) 2019  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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
 *
 ************************************************************************/

#ifndef _VLAN_SUBIF_H_
#define _VLAN_SUBIF_H_

#include <stdint.h>

/*
 * This header file defines an API that is implemented by 2 libraries:
 * userspace/public/libs/vlan_subif-linux_std
 * userspace/private/libs/vlan_subif-vlanctl
 */
typedef enum {
    VLANSUBIF_TYPE_NORMAL = 0,
    VLANSUBIF_TYPE_PPP_BRIDGE,
    VLANSUBIF_TYPE_MAX
} vlansubif_if_type_t;

typedef struct vlansubif_options
{
    const char *name;               /* If != NULL, this name is used instead of the default name lowerDevName.vlanId */
    uint8_t pbit;                   /* pbit */
    uint16_t tpid;                  /* TPID. Only 802.1q(8100) and 80.1ad(88a8) are supported */
    int is_routed;                  /* 1 for routed interface, 0 for bridged interface.
                                       This option is currently ignored in the linux-std implementation. */
    int is_multicast_disabled;      /* 1 if multicast traffic is not allowed */
    int is_ok_if_exists;            /* 1 if interface might exist when created. In this case it is reset.
                                       0 if interface must not exist when created */
    int is_phyif_other_owned;       /* 1 if the physical interface is owned by a non-Gateway
                                       domain protocol (such as OMCI). */
    vlansubif_if_type_t subif_type; /* This option is only for the linux-std implementation. */
    /* Optional etype filters are applied only to untagged interfaces */
#define VLANSUBIF_MAX_ETYPE_FILTERS 8
    int num_etype_filters;
    uint16_t etype_filter[VLANSUBIF_MAX_ETYPE_FILTERS];
} vlansubif_options_t;

typedef struct vlansubif_qos_options
{
    const char *tx_if_name;         /* If != NULL, this name is used to filter the tx interface */
    uint32_t flow_id;               /* It's skb flow id, it is used to filter the skb flow id */
    uint16_t tpid;                  /* tpid, it is used to push vlan */
    uint8_t pbit;                   /* pbit, it is used to do the pbit remark */
    int16_t vid;                   /* VLAN ID. it is the new vlan tag we need to add */
} vlansubif_qos_options_t;

/** Initialize vlanSubif library.
    Calling this function is optional, as an optimization.
    if this function is not called explicitly, vlanSubif
    library is initialized and cleaned up automatically upon every
    \vlanSubif_createVlanInterface(), \vlanSubif_deleteVlanInterface() call
    \returns 0 if suvccessful or error code < 0
*/
int vlanSubif_init(void);

/* Cleanup vlanSubif library.
   This function should be called if vlanSubif_init was called explicitly.
*/
void vlanSubif_cleanup(void);

/** Create VLAN sub-interface
 * \param[in]   lowerDevName   lower-level device
 * \param[in]   vlanId         VLAN Id. >=0-tagged, <0-untagged
 * \param[in]   qos            QoS mapping parameters or NULL
 * \returns 0 if successful or error code < 0
 */
int vlanSubif_createVlanInterface(const char *lowerDevName, signed int vlanId,
    const vlansubif_options_t *options);

/** Delete VLAN sub-interface created by vlanSubif_createVlanInterface() function
 * \param[in]   vlanDevName     VLAN sub-interface device name
 * \returns 0 if successful or error code < 0
 */
int vlanSubif_deleteVlanInterface(const char *vlanDevName);

/** Construct VLAN sub-interface device name.
 * \param[in]       lowerDevName   lower-level device
 * \param[in]       vlanId         VLAN Id. >=0-tagged, <0-untagged
 * \param[in,out]   intf_name      Buffer to return the interface name in
 * \param[in]       intf_name_size intf_name buffer size
 * \returns intf_name
 */
const char *vlanSubif_IntfName(const char *lowerDevName, signed int vlanId, char *intf_name,
    size_t intf_name_size);


/** Add qos tag rule at VLAN sub-interface
 * \param[in]   vlanDevName    VLAN sub-interface device name
 * \param[in]   options        The filter of Tx interface name, filter of skb flow id; pbit of remark, vlan id to add
 * \returns 0 if successful or error code < 0
 */
int vlanSubif_addQosVlanTagRule(const char *vlanDevName, const vlansubif_qos_options_t *options);

/** Del qos tag rule at VLAN sub-interface
 * \param[in]   vlanDevName    VLAN sub-interface device name
 * \param[in]   options        The filter of Tx interface name, filter of skb flow id
 * \returns 0 if successful or error code < 0
 */
int vlanSubif_delQosVlanTagRule(const char *vlanDevName, const vlansubif_qos_options_t *options);

/*
 * Logging support
 */

/* Define if vlan_subif logging is supported */
#define VLANSUBIF_LOG_SUPPORTED

/* Define to enable log redirection and logging at DEBUG level */
//#define VLANSUBIF_DEBUG

/*
 *------------------------------------------------------------------------------
 * Color encodings for console printing:
 *
 * To enable  color coded console printing: #define COLOR(clr_code)  clr_code
 * To disable color coded console printing: #define COLOR(clr_code)
 *
 * You may select a color specific to your subsystem by:
 *  #define CLRsys CLRg
 *
 *------------------------------------------------------------------------------
 */
#include <log_colors.h>
#include <bcmctl_syslogdefs.h>

#if defined(VLANSUBIF_LOG_SUPPORTED)
#define VLANSUBIF_LOGCODE(code)    code
#else
#define VLANSUBIF_LOGCODE(code)
#endif /*defined(VLANSUBIF_LOG_SUPPORTED)*/

typedef enum {
    VLANSUBIF_LOG_LEVEL_ERROR=0,
    VLANSUBIF_LOG_LEVEL_INFO,
    VLANSUBIF_LOG_LEVEL_DEBUG,
    VLANSUBIF_LOG_LEVEL_MAX
} vlansubif_logLevel_t;

/**
 * Logging API: Activate by #defining VLANSUBIF_LOG_SUPPORTED
 **/

#ifdef VLANSUBIF_DEBUG
/* If VLAN_SUBIF_LOG_REDIRECT_TO_FILE is defined, stdout is redirected to the specified file */
#define VLAN_SUBIF_LOG_REDIRECT_TO_FILE     "/var/log/vlan_subif.log"
#endif

#define VLANSUBIF_LOG_NAME "vlansubif"

#define VLANSUBIF_LOG_DEBUG(fmt, arg...)                                \
{ \
    VLANSUBIF_LOGCODE( vlanSubif_log(VLANSUBIF_LOG_LEVEL_DEBUG,         \
                         CLRg "[DBG " "%s" "] %-10s: " fmt CLRnl,       \
                         VLANSUBIF_LOG_NAME, __FUNCTION__, ##arg); )    \
    BCMCTL_SYSLOGCODE(vlansubif, LOG_DEBUG, fmt, ##arg); \
}

#define VLANSUBIF_LOG_INFO(fmt, arg...)                                 \
{ \
    VLANSUBIF_LOGCODE( vlanSubif_log(VLANSUBIF_LOG_LEVEL_INFO,          \
                         CLRm "[INF " "%s" "] %-10s: " fmt CLRnl,       \
                         VLANSUBIF_LOG_NAME, __FUNCTION__, ##arg); )    \
    BCMCTL_SYSLOGCODE(vlansubif, LOG_INFO, fmt, ##arg); \
}

#define VLANSUBIF_LOG_ERROR(fmt, arg...)                                \
{ \
    VLANSUBIF_LOGCODE( vlanSubif_log(VLANSUBIF_LOG_LEVEL_ERROR,             \
                         CLRerr "[ERROR " "%s" "] %-10s, %d: " fmt CLRnl,   \
                         VLANSUBIF_LOG_NAME, __FUNCTION__, __LINE__, ##arg); ) \
    BCMCTL_SYSLOGCODE(vlansubif, LOG_ERR, fmt, ##arg); \
}

void vlanSubif_log(vlansubif_logLevel_t level, const char *fmt, ...);
int vlanSubif_setLogLevel(vlansubif_logLevel_t logLevel);
vlansubif_logLevel_t vlanSubif_getLogLevel(void);
int vlansubif_logLevelIsEnabled(vlansubif_logLevel_t logLevel);

#endif /* _VLANSUBIF_API_H_ */

