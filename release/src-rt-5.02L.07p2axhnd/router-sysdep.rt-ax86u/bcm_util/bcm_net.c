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


/* The returned data from the command 'pspctl dump WanRate' has
   the format of XXYY, where XX represents the downstream rate and
   YY represents the upstream rate. If XX or YY is equal to 25, it
   means that data rate is 2.5G. If XX or YY is equal to 10 or 1,
   it translates to 10G or 1G, respectively. This function takes
   the "XX" or "YY" as an input string and returns the corresponding
   data rate in kbps */
static inline int __resolveVeipSpeed(char *buf)
{
    int tmpSpeed = atoi(buf);
    if (tmpSpeed == 25)
        tmpSpeed = 2500000;
    else
        tmpSpeed = tmpSpeed * 1000000;
    return tmpSpeed;
}


/* This function determines the local interface that could reach to the given
   server IP address and the data rate (in kbps) at which the gateway can
   transmit (and receive) to (and from) the given server IP. */
int bcmNet_getRouteInfoToServer(const char *serverIp, char* intfName,
                                char *intfIp, int *upSpeed, int *downSpeed)
{
    char buf[512], cmd[128] = {0}, tmp[32], macAddr[18] = {0};
    FILE *p = NULL;
    int ret = 0, portNumber = -1;
    unsigned long int tmpSpeed = 0;

    /* get the local interface that can reach to the given server IP */
    snprintf(buf, sizeof(buf), "/bin/ip route get %s", serverIp);
    p = popen(buf, "r");
    if ((NULL == p) || (EOF == fscanf(p, "%s dev %s  src %s", tmp, intfName, intfIp)))
    {
        bcmuLog_notice("Failed to get local interface that can reach %s", serverIp);
        ret = -1;
    }
    if (p)
        pclose(p);

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


    if (strstr(intfName, "eth") != NULL) {
        *upSpeed = 1000; /* set speed to 1G if nothing works out */
        snprintf(cmd, sizeof(cmd), "/sys/class/net/%s/speed", intfName);
        p = fopen(cmd, "r");
        if ((NULL == p) || (EOF == fscanf(p, "%d", upSpeed)))
        {
            bcmuLog_notice("Failed to get speed for %s\n", intfName);
            ret = -1;
        }
        if (p)
            fclose(p);
        *downSpeed = *upSpeed = *upSpeed * 1000;
    }
    else if (strstr(intfName, "ptm") != NULL)
    {
        *upSpeed = *downSpeed = 100000; /* set DSL max speed to 100M if nothing works out */
        p = popen("/bin/xtm datapath --status", "r");
        if (NULL != p)
        {
            while (fgets(buf, sizeof(buf), p))
            {
                printf("%s", buf);
                if (strstr(buf, "US Rate")) {
                    if (EOF != sscanf(buf, "US Rate = %lu bps", &tmpSpeed)) {
                        *upSpeed = tmpSpeed/1000;
                    } else {
                        bcmuLog_notice("Failed to get US rate for %s\n", intfName);
                        ret = -1;
                    }
                }
                if (strstr(buf, "DS Rate")) {
                    if (EOF != sscanf(buf, "DS Rate = %lu bps", &tmpSpeed)) {
                        *downSpeed = tmpSpeed/1000;
                    } else {
                        bcmuLog_notice("Failed to get DS rate for %s\n", intfName);
                        ret = -1;
                    }
                }
            }
            pclose(p);
        }
        else
        {
            bcmuLog_notice("Failed to get speed for %s\n", intfName);
            ret = -1;
        }
    }
    else if (strstr(intfName, "veip") != NULL)
    {
        *upSpeed = *downSpeed = 1000000; /* set speed to 1G if nothing works out */
        p = popen("/bin/pspctl dump WanRate", "r");
        if ((NULL == p) || (EOF == fscanf(p, "%s", buf)))
        {
            bcmuLog_notice("Failed to get speed for %s\n", intfName);
            ret = -1;
        }
        else
        {
            *upSpeed = __resolveVeipSpeed(buf+2);
            buf[2] = '\0';
            *downSpeed = __resolveVeipSpeed(buf);
        }
        if (p)
            pclose(p);

    }
    else
    {
        *upSpeed = *downSpeed = 1000000; /* set speed to 1G if nothing works out */
    }
    bcmuLog_notice("Route info for %s: SrcIntf=%s SrcIP=%s upSpeed=%d downSpeed=%d\n",
                   serverIp, intfName, intfIp, *upSpeed, *downSpeed);

    return ret;
}
