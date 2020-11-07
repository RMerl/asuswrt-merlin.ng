/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef _PWRCTL_API_H_
#define _PWRCTL_API_H_

int pwr_api_wifi_suspend_get(char *param, int *enable);
int pwr_api_wifi_suspend_set(char *param, int enable);
int pwr_api_disk_suspend_get(char *param, int *enable);
int pwr_api_disk_suspend_set(char *param, int enable);
int pwr_api_pcie_aspm_get(char *param, int *enable);
int pwr_api_pcie_aspm_set(char *param, int enable);
int pwr_api_ubus_dcm_get(char *param, int *enable);
int pwr_api_ubus_dcm_set(char *param, int enable);
int pwr_api_cpu_off_get(char *param, int *enable);
int pwr_api_cpu_off_set(char *param, int enable);
int pwr_api_cpu_speed_get(char *param, int *enable);
int pwr_api_cpu_speed_set(char *param, int enable);
int pwr_api_cpu_wait_get(char *param, int *enable);
int pwr_api_cpu_wait_set(char *param, int enable);
int pwr_api_xrdp_clock_gate_get(char *param, int *enable);
int pwr_api_xrdp_clock_gate_set(char *param, int enable);
int pwr_api_net_down_get(char *param, int *enable);
int pwr_api_net_down_set(char *param, int enable);
int pwr_api_net_down_show(char *param);
int pwr_api_phy_down_get(char *param, int *enable);
int pwr_api_phy_down_set(char *param, int enable);
int pwr_api_phy_down_show(char *param);
int pwr_api_phy_eee_get(char *param, int *enable);
int pwr_api_phy_eee_set(char *param, int enable);
int pwr_api_phy_apd_get(char *param, int *enable);
int pwr_api_phy_apd_set(char *param, int enable);
int pwr_api_sf2_dgm_get(char *param, int *enable);
int pwr_api_sf2_dgm_set(char *param, int enable);
int pwr_api_dram_sr_get(char *param, int *enable);
int pwr_api_dram_sr_set(char *param, int enable);
int pwr_api_avs_get(char *param, int *enable);
int pwr_api_avs_set(char *param, int enable);
int pwr_api_wldpd_get(char *param, int *enable);
int pwr_api_wldpd_set(char *param, int enable);
int pwr_api_wldpd_show(char *param);
int pwr_api_pmd_get(char *param, int *enable);
int pwr_api_pmd_set(char *param, int enable);


typedef enum
{
    PWR_TYPE_UNKNOWN,
    PWR_TYPE_WIFI,
    PWR_TYPE_DISK,
    PWR_TYPE_PCI,
    PWR_TYPE_UBUS,
    PWR_TYPE_CPU_OFF,
    PWR_TYPE_CPU_SPEED,
    PWR_TYPE_CPU_WAIT,
    PWR_TYPE_XRDP,
    PWR_TYPE_NET,
    PWR_TYPE_PHY,
    PWR_TYPE_EEE,
    PWR_TYPE_APD,
    PWR_TYPE_DGM,
    PWR_TYPE_SR,
    PWR_TYPE_AVS,
    PWR_TYPE_WLDPD,
    PWR_TYPE_PMD
} pwr_type_t;

typedef struct
{
    pwr_type_t type;
    char *name;
    int (*enable_set)(char *param, int enable);
    int (*enable_get)(char *param, int *enable);
    int (*enable_show)(char *param);
} pwr_drv_t;

typedef struct
{
    pwr_type_t type;
    char *param;
    int enable;
} pwr_entry_t;

int pwr_enable_set(pwr_type_t type, char *param, int enable);
int pwr_profile_activate(pwr_entry_t *profile);
int pwr_status_show(pwr_type_t type, char *param);

#endif

