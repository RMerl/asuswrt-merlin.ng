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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <linux/if.h>
#include "bcmnet.h"
#include "bcm/bcmswapitypes.h"
#include "bcmpwrmngtcfg.h"
#include "devctl_pwrmngt.h"
#include "pwrctl_api.h"

static int file_exist(char *filename)
{
    struct stat buffer;   
    return (stat(filename, &buffer) == 0);
}

static int pwr_api_file_read_line(char *filename, char *line, int line_num)
{
    int ret = 0;
    int i;
    FILE *file;

    if (!file_exist(filename))
        return -1;

    file = fopen(filename, "r");
    if (file == NULL)
        return -1;

    for (i = 0; i < line_num; i++)
    {
        if (fgets(line, 256, file) == NULL)
            ret = -1;
    }

    fclose(file);
    return ret;
}

static int pwr_api_file_write_line(char *filename, char *line)
{
    char command[256];

    if (!file_exist(filename))
        return -1;

    snprintf(command, sizeof(command), "echo %s > %s", line, filename);
    return system(command);
}

static int pwr_api_file_read_int(char *filename, int *num)
{
    char line[256];

    if (pwr_api_file_read_line(filename, line, 1))
        return -1;

    *num = atoi(line);

    return 0;
}

static int pwr_api_file_write_int(char *filename, int num)
{
    char line[256];

    snprintf(line, sizeof(line), "%d", num);
    return pwr_api_file_write_line(filename, line);
}

static int sock_init(char *ifname, int *skfd, struct ifreq *ifr)
{
    int ret = -1;

    if ((*skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        goto Exit;

    if (ifname)
        strcpy(ifr->ifr_name, ifname);
    else
        strcpy(ifr->ifr_name, "bcmsw");

    if (ioctl(*skfd, SIOCGIFINDEX, ifr) < 0)
        goto Exit;

    ret = 0;

Exit:

    return ret;
}

/* WIFI Suspend */
#define WIFI_SCRIPT         "/etc/init.d/wifi.sh"
#define TMP_WIFI_MODULES    "/tmp/wifi_modules"
int pwr_api_wifi_suspend_get(char *param, int *enable)
{
    char command[256];
    char line[256];

    if (!file_exist(WIFI_SCRIPT))
        return -1;

    snprintf(command, sizeof(command), "%s %s > %s", WIFI_SCRIPT, "modules", TMP_WIFI_MODULES);
    system(command);

    *enable = pwr_api_file_read_line(TMP_WIFI_MODULES, line, 1) ? 1 : 0;
    return 0;
}

int pwr_api_wifi_suspend_set(char *param, int enable)
{
    int en;
    char command[256];

    if (pwr_api_wifi_suspend_get(NULL, &en))
        return -1;

    if (enable == en)
        return -1;

    snprintf(command, sizeof(command), "%s %s > /dev/null", WIFI_SCRIPT, enable ? "suspend" : "resume");
    return system(command);
}

/* Disk Suspend: USB, SATA etc. */
#define DISK_SCRIPT         "/etc/init.d/disk.sh"
#define TMP_DISK_MODULES    "/tmp/disk_modules"
int pwr_api_disk_suspend_get(char *param, int *enable)
{
    char command[256];
    char line[256];

    if (!file_exist(DISK_SCRIPT))
        return -1;

    snprintf(command, sizeof(command), "%s %s > %s", DISK_SCRIPT, "modules", TMP_DISK_MODULES);
    system(command);

    *enable = pwr_api_file_read_line(TMP_DISK_MODULES, line, 1) ? 1 : 0;
    return 0;
}

int pwr_api_disk_suspend_set(char *param, int enable)
{
    int en;
    char command[256];

    if (pwr_api_disk_suspend_get(NULL, &en))
        return -1;

    if (enable == en)
        return -1;

    snprintf(command, sizeof(command), "%s %s > /dev/null", DISK_SCRIPT, enable ? "suspend" : "resume");
    return system(command);
}

/* PCI ASPM: Active State Power Management */
#define MODULE_PCIE_ASPM    "/sys/module/pcie_aspm/parameters/policy"
int pwr_api_pcie_aspm_get(char *param, int *enable)
{
    char policy[256];
    char *p1, *p2;

    if (pwr_api_file_read_line(MODULE_PCIE_ASPM, policy, 1))
        return -1;

    p1 = strstr(policy, "[");
    p2 = strstr(policy, "]");

    if (!p1 || !p2)
        return -1;

    p1++;
    *p2 = 0;
    *enable = strstr(p1, "l1_powersave") ? 1 : 0;

    return 0;
}

int pwr_api_pcie_aspm_set(char *param, int enable)
{
    return pwr_api_file_write_line(MODULE_PCIE_ASPM, enable ? "l1_powersave" : "default");
}

/* UBUS DCM */
#define MODULE_UBUS_DCM     "/sys/module/ubus4_dcm/parameters/enable"
int pwr_api_ubus_dcm_get(char *param, int *enable)
{
    return pwr_api_file_read_int(MODULE_UBUS_DCM, enable);
}

int pwr_api_ubus_dcm_set(char *param, int enable)
{
    return pwr_api_file_write_int(MODULE_UBUS_DCM, enable);
}

/* CPU Off */
#define MODULE_CPU_OFF      "/sys/module/bcm_cpuoff/parameters/enable"
int pwr_api_cpu_off_get(char *param, int *enable)
{
    return pwr_api_file_read_int(MODULE_CPU_OFF, enable);
}

int pwr_api_cpu_off_set(char *param, int enable)
{
    return pwr_api_file_write_int(MODULE_CPU_OFF, enable);
}

/* XRDP Clock Gating */
#define TMP_CLOCK_GATE      "/tmp/clock_gate"
int pwr_api_xrdp_clock_gate_get(char *param, int *enable)
{
    char line[256];
    char *p;

    if (system("/bin/bs /b/e system clock_gate > "TMP_CLOCK_GATE" 2>&1"))
        return -1;

    if (pwr_api_file_read_line(TMP_CLOCK_GATE, line, 3))
        return -1;

    p = strstr(line, ":");
    if (!p)
        return -1;

    *enable = !strncmp(p + 2, "yes", 3) ? 1 : 0;

    return 0;
}

int pwr_api_xrdp_clock_gate_set(char *param, int enable)
{
    char command[256];

    snprintf(command, sizeof(command), "/bin/bs /b/c system clock_gate=%s", enable ? "yes" : "no");
    return system(command);
}

/* Network device down */
static int pwr_api_net_down_get_one(char *ifname, int *enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};

    if (sock_init(ifname, &skfd, &ifr) < 0)
        goto Exit;

    if ((ret = ioctl(skfd, SIOCGIFFLAGS, &ifr)))
        goto Exit;

    *enable = ifr.ifr_flags & IFF_UP ? 0 : 1;

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_net_down_set_one(char *ifname, int enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};

    if (sock_init(ifname, &skfd, &ifr) < 0)
        goto Exit;

    if ((ret = ioctl(skfd, SIOCGIFFLAGS, &ifr)))
        goto Exit;

    if (enable)
        ifr.ifr_flags &= ~IFF_UP;
    else
        ifr.ifr_flags |= IFF_UP;

    if ((ret = ioctl(skfd, SIOCSIFFLAGS, &ifr)))
        goto Exit;

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_net_down_get_global(int *enable)
{
    int i, en;
    char ifname[5];

    *enable = 1;

    for (i = 0; i < 8; i++)
    {
        snprintf(ifname, sizeof(ifname), "eth%d", i);
        if (pwr_api_net_down_get_one(ifname, &en))
            continue;

        if (!en)
            *enable = 0;
    }

    return 0;
}

static int pwr_api_net_down_set_global(int enable)
{
    int i;
    char ifname[5];

    for (i = 0; i < 8; i++)
    {
        snprintf(ifname, sizeof(ifname), "eth%d", i);
        pwr_api_net_down_set_one(ifname, enable);
    }

    return 0;
}

int pwr_api_net_down_get(char *param, int *enable)
{
    if (param)
        return pwr_api_net_down_get_one(param, enable);
    else
        return pwr_api_net_down_get_global(enable);
}

int pwr_api_net_down_set(char *param, int enable)
{
    if (param)
        return pwr_api_net_down_set_one(param, enable);
    else
        return pwr_api_net_down_set_global(enable);
}

/* PHY Power Down */
static int pwr_api_phy_down_get_one(char *ifname, int *enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethctl_data ethctl = {};

    if (sock_init(ifname, &skfd, &ifr) < 0)
        goto Exit;

    ethctl.op = ETHGETPHYPWR; 
    ifr.ifr_data = (void *)&ethctl;

    ret = ioctl(skfd, SIOCETHCTLOPS, &ifr);
    *enable = ethctl.ret_val ? 0 : 1;

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_phy_down_set_one(char *ifname, int enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethctl_data ethctl = {};

    if (sock_init(ifname, &skfd, &ifr) < 0)
        goto Exit;

    ethctl.op = enable ? ETHSETPHYPWROFF : ETHSETPHYPWRON; 
    ifr.ifr_data = (void *)&ethctl;

    ret = ioctl(skfd, SIOCETHCTLOPS, &ifr);

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_phy_down_get_global(int *enable)
{
    int i, en;
    char ifname[5];

    *enable = 1;

    for (i = 0; i < 8; i++)
    {
        snprintf(ifname, sizeof(ifname), "eth%d", i);
        if (pwr_api_phy_down_get_one(ifname, &en))
            continue;

        if (!en)
            *enable = 0;
    }

    return 0;
}

static int pwr_api_phy_down_set_global(int enable)
{
    int i;
    char ifname[5];

    for (i = 0; i < 8; i++)
    {
        snprintf(ifname, sizeof(ifname), "eth%d", i);
        pwr_api_phy_down_set_one(ifname, enable);
    }

    return 0;
}

int pwr_api_phy_down_get(char *param, int *enable)
{
    if (param)
        return pwr_api_phy_down_get_one(param, enable);
    else
        return pwr_api_phy_down_get_global(enable);
}

int pwr_api_phy_down_set(char *param, int enable)
{
    if (param)
        return pwr_api_phy_down_set_one(param, enable);
    else
        return pwr_api_phy_down_set_global(enable);
}

/* PHY EEE: Energy Efficient Ethernet */
static int pwr_api_phy_eee_get_one(char *ifname, int *enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethctl_data ethctl = {};

    if (sock_init(ifname, &skfd, &ifr) < 0)
        goto Exit;

    ethctl.op = ETHGETPHYEEE; 
    ifr.ifr_data = (void *)&ethctl;

    ret = ioctl(skfd, SIOCETHCTLOPS, &ifr);
    *enable = ethctl.ret_val ? 1 : 0;

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_phy_eee_set_one(char *ifname, int enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethctl_data ethctl = {};

    if (sock_init(ifname, &skfd, &ifr) < 0)
        goto Exit;

    ethctl.op = enable ? ETHSETPHYEEEON : ETHSETPHYEEEOFF; 
    ifr.ifr_data = (void *)&ethctl;

    ret = ioctl(skfd, SIOCETHCTLOPS, &ifr);

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_phy_eee_get_global(int *enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethswctl_data ethswctl = {};

    if (sock_init(NULL, &skfd, &ifr) < 0)
        goto Exit;

    ethswctl.op = ETHSWPHYEEE; 
    ethswctl.type = TYPE_GET;
    ifr.ifr_data = (void *)&ethswctl;

    ret = ioctl(skfd, SIOCETHSWCTLOPS, &ifr);
    *enable = ethswctl.val ? 1 : 0;

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_phy_eee_set_global(int enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethswctl_data ethswctl = {};

    if (sock_init(NULL, &skfd, &ifr) < 0)
        goto Exit;

    ethswctl.op = ETHSWPHYEEE; 
    ethswctl.type = TYPE_SET;
    ethswctl.val = enable ? 1 : 0;
    ifr.ifr_data = (void *)&ethswctl;

    ret = ioctl(skfd, SIOCETHSWCTLOPS, &ifr);

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

int pwr_api_phy_eee_get(char *param, int *enable)
{
    if (param)
        return pwr_api_phy_eee_get_one(param, enable);
    else
        return pwr_api_phy_eee_get_global(enable);
}

int pwr_api_phy_eee_set(char *param, int enable)
{
    if (param)
        return pwr_api_phy_eee_set_one(param, enable);
    else
        return pwr_api_phy_eee_set_global(enable);
}

/* PHY APD: Auto Power Down */
static int pwr_api_phy_apd_get_one(char *ifname, int *enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethctl_data ethctl = {};

    if (sock_init(ifname, &skfd, &ifr) < 0)
        goto Exit;

    ethctl.op = ETHGETPHYAPD; 
    ifr.ifr_data = (void *)&ethctl;

    ret = ioctl(skfd, SIOCETHCTLOPS, &ifr);
    *enable = ethctl.ret_val ? 1 : 0;

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_phy_apd_set_one(char *ifname, int enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethctl_data ethctl = {};

    if (sock_init(ifname, &skfd, &ifr) < 0)
        goto Exit;

    ethctl.op = enable ? ETHSETPHYAPDON : ETHSETPHYAPDOFF; 
    ifr.ifr_data = (void *)&ethctl;

    ret = ioctl(skfd, SIOCETHCTLOPS, &ifr);

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_phy_apd_get_global(int *enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethswctl_data ethswctl = {};

    if (sock_init(NULL, &skfd, &ifr) < 0)
        goto Exit;

    ethswctl.op = ETHSWPHYAPD; 
    ethswctl.type = TYPE_GET;
    ifr.ifr_data = (void *)&ethswctl;

    ret = ioctl(skfd, SIOCETHSWCTLOPS, &ifr);
    *enable = ethswctl.val ? 1 : 0;

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

static int pwr_api_phy_apd_set_global(int enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethswctl_data ethswctl = {};

    if (sock_init(NULL, &skfd, &ifr) < 0)
        goto Exit;

    ethswctl.op = ETHSWPHYAPD; 
    ethswctl.type = TYPE_SET;
    ethswctl.val = enable ? 1 : 0;
    ifr.ifr_data = (void *)&ethswctl;

    ret = ioctl(skfd, SIOCETHSWCTLOPS, &ifr);

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

int pwr_api_phy_apd_get(char *param, int *enable)
{
    if (param)
        return pwr_api_phy_apd_get_one(param, enable);
    else
        return pwr_api_phy_apd_get_global(enable);
}

int pwr_api_phy_apd_set(char *param, int enable)
{
    if (param)
        return pwr_api_phy_apd_set_one(param, enable);
    else
        return pwr_api_phy_apd_set_global(enable);
}

/* SF2 DGM: Deep Green Mode */
int pwr_api_sf2_dgm_get(char *param, int *enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethswctl_data ethswctl = {};

    if (sock_init(NULL, &skfd, &ifr) < 0)
        goto Exit;

    ethswctl.op = ETHSWDEEPGREENMODE; 
    ethswctl.type = TYPE_GET;
    ifr.ifr_data = (void *)&ethswctl;

    ret = ioctl(skfd, SIOCETHSWCTLOPS, &ifr);
    *enable = ethswctl.val ? 1 : 0;

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

int pwr_api_sf2_dgm_set(char *param, int enable)
{
    int ret = -1;
    int skfd;
    struct ifreq ifr = {};
    struct ethswctl_data ethswctl = {};

    if (sock_init(NULL, &skfd, &ifr) < 0)
        goto Exit;

    ethswctl.op = ETHSWDEEPGREENMODE; 
    ethswctl.type = TYPE_SET;
    ethswctl.val = enable ? 1 : 0;
    ifr.ifr_data = (void *)&ethswctl;

    ret = ioctl(skfd, SIOCETHSWCTLOPS, &ifr);

Exit:
    if (skfd >= 0)
        close(skfd);

    return ret;
}

/* CPU Wait */
int pwr_api_cpu_wait_get(char *param, int *enable)
{
    PWRMNGT_CONFIG_PARAMS cfg = {};

    if (PwrMngtCtl_GetConfig(&cfg, PWRMNGT_CFG_PARAM_CPU_R4K_WAIT_MASK))
        return -1;

    *enable = cfg.cpur4kwait == PWRMNGT_ENABLE ? 1 : 0;
    return 0;
}

int pwr_api_cpu_wait_set(char *param, int enable)
{
    PWRMNGT_CONFIG_PARAMS cfg = {};

    cfg.cpur4kwait = enable ? PWRMNGT_ENABLE : PWRMNGT_DISABLE;
    return PwrMngtCtl_SetConfig(&cfg, PWRMNGT_CFG_PARAM_CPU_R4K_WAIT_MASK, NULL);
}

/* CPU Speed */
int pwr_api_cpu_speed_get(char *param, int *enable)
{
    PWRMNGT_CONFIG_PARAMS cfg = {};

    if (PwrMngtCtl_GetConfig(&cfg, PWRMNGT_CFG_PARAM_CPUSPEED_MASK))
        return -1;

    *enable = (cfg.cpuspeed==256) ? 1 : 0;
    return 0;
}

int pwr_api_cpu_speed_set(char *param, int enable)
{
    PWRMNGT_CONFIG_PARAMS cfg = {};
    uint32_t speed = enable ? 256 : 1;

    cfg.cpuspeed = param ? atoi(param) : speed;
    return PwrMngtCtl_SetConfig(&cfg, PWRMNGT_CFG_PARAM_CPUSPEED_MASK, NULL);
}

/* DRAM SR: Self Refresh */
int pwr_api_dram_sr_get(char *param, int *enable)
{
    PWRMNGT_CONFIG_PARAMS cfg = {};

    if (PwrMngtCtl_GetConfig(&cfg, PWRMNGT_CFG_PARAM_MEM_SELF_REFRESH_MASK))
        return -1;

    *enable = cfg.dramSelfRefresh == PWRMNGT_ENABLE ? 1 : 0;
    return 0;
}

int pwr_api_dram_sr_set(char *param, int enable)
{
    PWRMNGT_CONFIG_PARAMS cfg = {};

    cfg.dramSelfRefresh = enable ? PWRMNGT_ENABLE : PWRMNGT_DISABLE;
    return PwrMngtCtl_SetConfig(&cfg, PWRMNGT_CFG_PARAM_MEM_SELF_REFRESH_MASK, NULL);
}

/* AVS: Adaptive Voltage Scaling */
int pwr_api_avs_get(char *param, int *enable)
{
    PWRMNGT_CONFIG_PARAMS cfg = {};

    if (PwrMngtCtl_GetConfig(&cfg, PWRMNGT_CFG_PARAM_MEM_AVS_MASK))
        return -1;

    *enable = cfg.avs == PWRMNGT_ENABLE ? 1 : 0;
    return 0;
}

int pwr_api_avs_set(char *param, int enable)
{
    PWRMNGT_CONFIG_PARAMS cfg = {};

    cfg.avs = enable ? PWRMNGT_ENABLE : PWRMNGT_DISABLE;
    return PwrMngtCtl_SetConfig(&cfg, PWRMNGT_CFG_PARAM_MEM_AVS_MASK, NULL);
}

/* Power saving drivers list */
static pwr_drv_t pwr_drivers[] = {
    { PWR_TYPE_WIFI,        "WIFI Off",     pwr_api_wifi_suspend_set,       pwr_api_wifi_suspend_get },
    { PWR_TYPE_DISK,        "DISK Off",     pwr_api_disk_suspend_set,       pwr_api_disk_suspend_get },
    { PWR_TYPE_PCI,         "PCI ASPM",     pwr_api_pcie_aspm_set,          pwr_api_pcie_aspm_get },
    { PWR_TYPE_UBUS,        "UBUS DCM",     pwr_api_ubus_dcm_set,           pwr_api_ubus_dcm_get },
    { PWR_TYPE_CPU_OFF,     "CPU Off",      pwr_api_cpu_off_set,            pwr_api_cpu_off_get },
    { PWR_TYPE_CPU_WAIT,    "CPU Wait",     pwr_api_cpu_wait_set,           pwr_api_cpu_wait_get },
    { PWR_TYPE_CPU_SPEED,   "CPU Speed",    pwr_api_cpu_speed_set,          pwr_api_cpu_speed_get },
    { PWR_TYPE_XRDP,        "XRDP Gate",    pwr_api_xrdp_clock_gate_set,    pwr_api_xrdp_clock_gate_get },
    { PWR_TYPE_NET,         "NET Down",     pwr_api_net_down_set,           pwr_api_net_down_get },
    { PWR_TYPE_PHY,         "PHY Down",     pwr_api_phy_down_set,           pwr_api_phy_down_get },
    { PWR_TYPE_EEE,         "PHY EEE",      pwr_api_phy_eee_set,            pwr_api_phy_eee_get },
    { PWR_TYPE_APD,         "PHY APD",      pwr_api_phy_apd_set,            pwr_api_phy_apd_get },
    { PWR_TYPE_DGM,         "SF2 DGM",      pwr_api_sf2_dgm_set,            pwr_api_sf2_dgm_get },
    { PWR_TYPE_SR,          "DRAM SR",      pwr_api_dram_sr_set,            pwr_api_dram_sr_get},
    { PWR_TYPE_AVS,         "AVS",          pwr_api_avs_set,                pwr_api_avs_get},
};

static pwr_drv_t *pwr_driver_get(pwr_type_t type)
{
    int i;

    for (i = 0; i < sizeof(pwr_drivers) / sizeof(pwr_drivers[0]); i++)
    {
        if (pwr_drivers[i].type == type)
            return &pwr_drivers[i];
    }

    return NULL;
}

int pwr_enable_set(pwr_type_t type, char *param, int enable)
{
    pwr_drv_t *pwr_drv;

    if (!(pwr_drv = pwr_driver_get(type)))
        return 0;

    printf("%s (%s) ==> %s\n", pwr_drv->name, param ? param : "*", enable ? "Enable" : "Disable");

    return pwr_drv->enable_set(param, enable);
}

int pwr_profile_activate(pwr_entry_t *profile)
{
    int ret = 0;

    while (profile->type != PWR_TYPE_UNKNOWN)
    {
        ret |= pwr_enable_set(profile->type, profile->param, profile->enable);
        profile++;
    }

    return ret;
}

int pwr_status_show(void)
{
    int i, enable;

    for (i = 0; i < sizeof(pwr_drivers) / sizeof(pwr_drivers[0]); i++)
    {
        printf("%-20s", pwr_drivers[i].name);

        if (pwr_drivers[i].enable_get && !pwr_drivers[i].enable_get(NULL, &enable))
            printf("%s\n", enable ? "Enabled" : "Disabled");
        else
            printf("%s\n", "N/A");
    }

    return 0;
}
