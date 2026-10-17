/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom 
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

#ifndef ___TC_INGRESS_H___
#define ___TC_INGRESS_H___

#include <rdpa_api.h>
#include <linux/bcm_log.h>

//#define CC_TC_DEBUG_ENABLE

#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_TC, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_TC, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_TC, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_TC, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_TC, fmt, ##arg)

#define __print(fmt, arg...) bcm_print(fmt, ##arg)

#if defined(CC_TC_DEBUG_ENABLE)
#define __debug(fmt, arg...)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
            __print("%s:%d: " fmt, __FUNCTION__, __LINE__, ##arg); )

#else
#define __debug(fmt, arg...)
#endif

#define TC_VERSION              "1.0"
#if defined (__KERNEL__)
#define TC_VER_STR              "v" TC_VERSION
#else
#define TC_VER_STR              "v" TC_VERSION " " __DATE__ " " __TIME__
#endif
#define TC_MODNAME              "Broadcom TC Driver"

int tc_runner_create_ic_object(rdpa_traffic_dir dir, rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow);
int tc_runner_del_ic_rule(rdpa_traffic_dir dir, rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow);
int tc_runner_get_ic_rule_stats(rdpa_traffic_dir dir, rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow,
    uint64_t *bytes, uint64_t *packets);

int tc_runner_set_if_rule(bdmf_object_handle port, int filter_idx, rdpa_filter_ctrl_t *filter_ctrl, uint32_t filter_opaque);

int tc_ingress_init(void);
void tc_ingress_exit(void);

#endif /* ___TC_INGRESS_H___ */
