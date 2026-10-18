/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2018:DUAL/GPL:standard

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
 *
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>

#include "bcm_ulog.h"
#include "board.h"
#include "bcm_ethswutils.h"

#define MBPS_TO_KBPS(a) (a) * 1000

/* This function determines the local interface that could reach to the given
   server IP address and the data rate (in kbps) at which the gateway can
   transmit (and receive) to (and from) the given server IP. */
int bcmNet_getRouteInfoToServer(const char *serverIp, char* intfName,
                                char *intfIp, int *upSpeed, int *downSpeed)
{
    char buf[512], *bufp, cmd[128] = {0}, tmp[32], macAddr[18] = {0};
    FILE *p = NULL;
    int ret = 0, portNumber = -1;
    unsigned long int tmpSpeed = 0;
    char localLowerDevName[IFNAMSIZ] = {0};
    char *tmp_intfName;

    /* get the local interface that can reach to the given server IP */
    snprintf(buf, sizeof(buf), "/bin/ip route get %s", serverIp);
    p = popen(buf, "r");
    if ((NULL == p) || (NULL == fgets(buf, 512, p)))
    {
        bcmuLog_notice("Failed to get local interface that can reach %s", serverIp);
        ret = -1;
    }
    if (p)
        pclose(p);
    if (ret != -1)
    {
        bufp = strstr(buf, "dev");
        if (bufp)
            sscanf(bufp + strlen("dev"), " %s", intfName);

        bufp = strstr(buf, "src");
        if (bufp)
            sscanf(bufp + strlen("src"), " %s", intfIp);
    }

    if (strstr(intfName, "br") != NULL)
    {
        /* The interface that can reach the given server is through the bridge!
           Determine the port on the bridge that could reach the server by doing
           the steps below: */
        /* Step 1. ping the server IP once */
        snprintf(buf, sizeof(buf), "/bin/ping %s -c 1", serverIp);
        p = popen(buf, "r");
        if (p)
            pclose(p);

        /* Step 2. look up the arp table for the mac address of the corresponding IP */
        snprintf(cmd, sizeof(cmd), "/bin/ip neighbour | grep %s", serverIp);
        p = popen(cmd, "r");

        /* The input file's format is fixed, no concern. */
        /* coverity[dont_call] */
        if ((NULL == p) || (EOF == fscanf(p, "%s %s %s %s %s", tmp, tmp, tmp, tmp, macAddr)))
        {
            bcmuLog_notice("Failed to get mac address of the server from the arp table\n");
            ret = -1;
        }
        if (p)
            pclose(p);

        /* Step 3. find the port number in the bridge that saw the mac address of the server. */
        if (strlen(macAddr)) {

            /* try brctl to find the port number in the bridge */
            snprintf(cmd, sizeof(cmd), "/bin/brctl showmacs %s 2>/dev/null | grep %s", intfName, macAddr);
            p = popen(cmd, "r");
            if ((NULL == p) || (EOF == fscanf(p, "%d", &portNumber)))
            {
                bcmuLog_notice("Failed to get port number using brctl on the bridge that can reach  %s\n", serverIp);
                if (p)
                    pclose(p);

                /* try ovs-appctl to find the port number in the bridge */
                snprintf(cmd, sizeof(cmd), "/bin/ovs-appctl fdb/show %s 2>/dev/null | grep %s", intfName, macAddr);
                p = popen(cmd, "r");
                if ((NULL == p) || (EOF == fscanf(p, "%d", &portNumber)))
                {
                    bcmuLog_notice("Failed to get port number using ovs-appctl on the bridge that can reach  %s\n", serverIp);
                    ret = -1;
                }
            }
            if (p)
                pclose(p);
        }

        /* Step 4. Map the port number in the bridge table back to the interface name. */
        if (portNumber >= 0) {

            /* try brctl to map the port number back to the interface name */
            snprintf(cmd, sizeof(cmd), "/bin/brctl showstp %s 2>/dev/null | grep '(%d)'", intfName, portNumber);
            p = popen(cmd, "r");

            /* The input file's format is fixed, no concern. */
            /* coverity[dont_call] */
            if ((NULL == p) || (EOF == fscanf(p, "%s", intfName)))
            {
                bcmuLog_notice("Failed to get the interface name associated to the port on the bridge using brctl\n");
                if (p)
                    pclose(p);

                /* try to use ovs-ofctl to map the port number back to the interface name */
                snprintf(cmd, sizeof(cmd), "/bin/ovs-ofctl show %s 2>/dev/null | grep '%d('", intfName, portNumber);
                p = popen(cmd, "r");
                if ((NULL == p) || (EOF == fscanf(p, " %d(%[^)])", &portNumber, intfName)))
                {
                    bcmuLog_notice("Failed to get the interface name associated to the port on the bridge using ovs-ofctl\n");
                    ret = -1;
                }
            }
            if (p)
                pclose(p);
        }
        bcmuLog_notice("%s %s %d\n", intfName, macAddr, portNumber);
    }

    tmp_intfName = intfName;
    if (strstr(intfName, "ppp") != NULL)
    {
        snprintf(buf, sizeof(buf), "cat /sys/class/net/%s/lower_*/uevent|sed -n 's/^INTERFACE=//p'", intfName);
        p = popen(buf, "r");
        if ((NULL == p) || (NULL == fgets(buf, 512, p)))
        {
            bcmuLog_notice("Failed to get lower interface of %s", intfName);
            ret = -1;
        }
        if (p)
            pclose(p);

        if (ret != -1)
            sscanf(buf, "%s", localLowerDevName);

        bcmuLog_notice("Interface intfName %s's parent is %s\n", intfName, localLowerDevName);

        tmp_intfName = localLowerDevName;
    }

    if (strstr(tmp_intfName, "eth") != NULL) {
        *upSpeed = 1000; /* set speed to 1G if nothing works out */
        snprintf(cmd, sizeof(cmd), "/sys/class/net/%s/speed", tmp_intfName);
        p = fopen(cmd, "r");
        if ((NULL == p) || (EOF == fscanf(p, "%d", upSpeed)))
        {
            bcmuLog_notice("Failed to get speed for %s\n", tmp_intfName);
            ret = -1;
        }
        if (p)
            fclose(p);
        *downSpeed = *upSpeed = *upSpeed * 1000;
    }
    else if (strstr(tmp_intfName, "ptm") != NULL || strstr(tmp_intfName, "atm") != NULL)
    {
        int currUpSpeed = 0, currDownSpeed = 0; 
        *upSpeed = *downSpeed = 0;

        p = popen("/bin/xtm datapath --status", "r");
        if (NULL != p)
        {
            while (fgets(buf, sizeof(buf), p))
            {
                printf("%s", buf);
                bufp = strstr(buf, "US Rate");
                if (bufp) {
                    bufp = strstr(bufp, "=");
                    if ((bufp != NULL) && (EOF != sscanf(bufp, "= %lu bps", &tmpSpeed))) {
                        currUpSpeed = tmpSpeed/1000;
                    } else {
                        bcmuLog_notice("Failed to get US rate for %s\n", tmp_intfName);
                        ret = -1;
                    }
                }
                bufp = strstr(buf, "DS Rate");
                if (bufp) {
                    bufp = strstr(bufp, "=");
                    if ((bufp != NULL) && (EOF != sscanf(bufp, "= %lu bps", &tmpSpeed))) {
                        currDownSpeed = tmpSpeed/1000;
                    } else {
                        bcmuLog_notice("Failed to get DS rate for %s\n", tmp_intfName);
                        ret = -1;
                    }
                }
                if (*upSpeed < currUpSpeed)
                    *upSpeed = currUpSpeed;
                if (*downSpeed < currDownSpeed)
                    *downSpeed = currDownSpeed;
            }
            pclose(p);
            /* set speed to 1G if nothing works out */
            if (!*upSpeed)
                *upSpeed = 1000000;
            if (!*downSpeed)
                *downSpeed = 1000000; 
        }
        else
        {
            bcmuLog_notice("Failed to get speed for %s\n", tmp_intfName);
            ret = -1;
        }
    }
    else if (strstr(tmp_intfName, "veip") != NULL)
    {
        ret = bcm_phy_speed_get(tmp_intfName, 0, (uint32_t *)upSpeed);
        ret = ret ? ret : bcm_phy_speed_get(tmp_intfName, 1, (uint32_t *)downSpeed);
        if (ret)
        {
            *upSpeed = *downSpeed = 10000000; /* set speed to 10G if nothing works out */ 
            ret = -1;
        }
        else
        {
             *upSpeed = MBPS_TO_KBPS(*upSpeed);
             *downSpeed = MBPS_TO_KBPS(*downSpeed);
        }
    }
    else
    {
        *upSpeed = *downSpeed = 1000000; /* set speed to 1G if nothing works out */
        ret = -1;
    }
    bcmuLog_notice("Route info for %s: SrcIntf=%s SrcIP=%s upSpeed=%d downSpeed=%d, ret %d\n",
                   serverIp, tmp_intfName, intfIp, *upSpeed, *downSpeed, ret);

    return ret;
}

/* This function gets the uboot ipaddr/netmask environment parameters.
 * If the netmaks isn't specified, it will assigned it accordingly.
 * e.g.  192.168.1.x->255.255.255.0; 10.x.x.x->255.0.0.0; 172.x->255.240.0.0; others->255.255.0.0
 * If failed to get the values from uboot, it assigned the default setting which is 
 * 192.168.1.1/255.255.255.0
 */
void bcmNet_getDefaultLanIpInfo(char *ipaddr, int iplen, char* netmask, int masklen)
{
   FILE* fs;  
   /* Try to use the ipaddress/netmask from uboot environment parameters */
   fs = fopen("/proc/environment/ipaddr", "r");
   if ( fs != NULL ) {
       if ( fgets(ipaddr, iplen, fs) != NULL )
           bcmuLog_notice("Failed to get ipaddr.");
       fclose(fs);
#if 0  //TODO: move cmsUtl_isValidIpv4Address to bcmUtl.
       if ( ! cmsUtl_isValidIpv4Address(ipaddr))
       {
           snprintf(ipaddr, iplen, "%s", "192.168.1.1");
       }
#endif
   }
   else
   {
       snprintf(ipaddr, iplen, "%s", "192.168.1.1");
   }
   fs = fopen("/proc/environment/netmask", "r");
   if ( fs != NULL ) {
       if ( fgets(netmask, masklen, fs) != NULL )
           bcmuLog_notice("Failed to get netmask.");
       fclose(fs);
#if 0  //TODO: move cmsUtl_isValidIpv4Address to bcmUtl.
       if ( ! cmsUtl_isValidIpv4Address(netmask))
       {
           snprintf(netmask, masklen, "%s", "255.255.255.0");
       }
#endif
   }
   else
   {
       if ( !strncmp(ipaddr, "10.", strlen("10.")))
       {
           snprintf(netmask, masklen, "%s", "255.0.0.0");
       }
       else if ( !strncmp(ipaddr, "172.16.", strlen("172.16.")))
       {
           snprintf(netmask, masklen, "%s", "255.255.0.0");
       }
       else
       {
           snprintf(netmask, masklen, "%s", "255.255.255.0");
       }
   }
   return;
}

/* This function gets the uboot nodhcp environment parameter. 
 * Return 0 if failed to read the value from uboot env, or the value is neither true nor false.
 * Return 1 if found, and nodhcp holds value "true" or "false".
 */
int bcmNet_getNodhcpVar(char* nodhcp, int len)
{
   FILE* fs;
   int found = 0;

   /* Try to read nodhcp from uboot environment parameters */
   fs = fopen("/proc/environment/nodhcp", "r");
   if ( fs != NULL )
   {
      if ( fgets(nodhcp, len, fs) == NULL )
      {
         bcmuLog_notice("Failed read nodhcp variable.");
      }
      else
      {
         if ((!strcasecmp(nodhcp, "true")) || (!strcasecmp(nodhcp, "false")))
         {
            found = 1;
         }
      }
      fclose(fs);
   }
   return (found);
}

/**
 *  Function: This function will return wanType string.
 *  
 **/
int bcmNet_getWanTypeStr(char *wanType, int size)
{
    FILE *p = NULL;
    char rdpaWanType[16];
    int rc = 0;

    memset(rdpaWanType, 0, sizeof(rdpaWanType));
    p = popen("/bin/pspctl dump RdpaWanType", "r");    
    if ((NULL == p) || (fgets(rdpaWanType, sizeof(rdpaWanType), p) == NULL))
    {
        fprintf(stderr, "%s: get RdpaWanType failed!\n", __FUNCTION__);
        rc = -1;
        goto exit;
    }

    if ((!strncmp(rdpaWanType, RDPA_WAN_TYPE_VALUE_GPON, strlen(RDPA_WAN_TYPE_VALUE_GPON)))    ||
        (!strncmp(rdpaWanType, RDPA_WAN_TYPE_VALUE_XGPON1, strlen(RDPA_WAN_TYPE_VALUE_XGPON1))) ||
        (!strncmp(rdpaWanType, RDPA_WAN_TYPE_VALUE_NGPON2, strlen(RDPA_WAN_TYPE_VALUE_NGPON2))) ||
        (!strncmp(rdpaWanType, RDPA_WAN_TYPE_VALUE_XGS, strlen(RDPA_WAN_TYPE_VALUE_XGS))))
    {
        strncpy(wanType, "gpon", (size - 1));
    }
    else if ((!strncmp(rdpaWanType, RDPA_WAN_TYPE_VALUE_NONE, strlen(RDPA_WAN_TYPE_VALUE_NONE))) ||
             (!strncmp(rdpaWanType, RDPA_WAN_TYPE_VALUE_GBE, strlen(RDPA_WAN_TYPE_VALUE_GBE))))
    {
        strncpy(wanType, "eth", (size - 1));
    }
    else if (!strncmp(rdpaWanType, RDPA_WAN_TYPE_VALUE_XDSL, strlen(RDPA_WAN_TYPE_VALUE_XDSL)))
    {
        strncpy(wanType, "dsl", (size - 1));
    }
    else if (!strncmp(rdpaWanType, RDPA_WAN_TYPE_VALUE_EPON, strlen(RDPA_WAN_TYPE_VALUE_EPON)))
    {
        strncpy(wanType, "epon", (size - 1));
    }
    else
    {
        rc = -1;
    }

exit:
    if (p)
    {
        pclose(p);
    }

    return rc;
}

