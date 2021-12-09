/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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

#include <linux/bcm_log.h>
#include <rdpa_api.h>

int blog_parse_mcast_result_get(Blog_t *blog, rdpa_ic_result_t *mcast_result);
void blog_parse_mcast_result_put(rdpa_ic_result_t *mcast_result);
rdpa_if rdpa_mw_root_dev2rdpa_if(struct net_device *root_dev);
uint8_t rdpa_mw_root_dev2rdpa_ssid(struct net_device *root_dev);
rdpa_if blog_parse_egress_port_get(Blog_t *blog);
rdpa_if blog_parse_ingress_port_get(Blog_t *blog);
#if defined(CONFIG_BCM_PON)
void blog_parse_policer_get(Blog_t *blog_p, bdmf_object_handle *policer);
#endif

