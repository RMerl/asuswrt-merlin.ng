/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

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


/*!\file bcm_ethswutil.c
 * \brief Some utility functions for querying the ethernet switch.  Does not
 *        depend on ethswctl though.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "bcmnet.h"       // for ifreq_ext
#include "bcm_retcodes.h"
#include "bcm_ulog.h"





#ifndef DESKTOP_LINUX

// NOTE: there is another copy of this function in ethswctl_api.c.  Need to keep these copies in sync.
// TODO: Delete this copy in 5.04L.03.
static SINT32 ethswctl_init(struct ifreq *ifr)
{
   SINT32  skfd;

   /* Open a basic socket */
   if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      bcmuLog_error("Error openning socket for getting the LAN Only enet port list");
      return -1;
   }

    /* Get the name -> if_index mapping for ethswctl */
    memset((void *) ifr, 0, sizeof(*ifr));
    strcpy(ifr->ifr_name, "bcmsw");
    if (ioctl(skfd, SIOCGIFINDEX, ifr) < 0)
    {
        strcpy(ifr->ifr_name, "eth0");
        if (ioctl(skfd, SIOCGIFINDEX, ifr) < 0)
        {
            close(skfd);
            bcmuLog_error("bcmsw/eth0 interface does not exist.  Error: %d", errno);
            return -1;
        }
    }
    return skfd;
}

#ifndef MAX_SYS_ETH_PORT
#define MAX_SYS_ETH_PORT 64
#endif

// NOTE: there is another copy of this function in ethswctl_api.c.  Need to keep these copies in sync.
// TODO: Delete this copy in 5.04L.03.
static BcmRet getWanLanAttrList(char **nameList, int opcode)
{
   SINT32  skfd;
   struct ifreq ifr;
   struct ifreq_ext ifx;

   memset((void *) &ifr, 0, sizeof(ifr));
   memset((void *) &ifx, 0, sizeof(ifx));

   if ((skfd = ethswctl_init(&ifr)) < 0)
   {
      bcmuLog_error("Error openning socket for getting the opcode 0x%x", opcode);
      return BCMRET_INTERNAL_ERROR;
   }

   /* Allocate dynamic memory to hold max interface names (eth0,eth1,..eth10<cr>)*/
   ifx.bufLen = (MAX_SYS_ETH_PORT * (IFNAMSIZ + 1)) + 2;
   if ((*nameList = calloc(1, ifx.bufLen)) == NULL)
   {
      bcmuLog_error("Fail to alloc mem (%d bytes) for opcode 0x%x", ifx.bufLen, opcode);
      close(skfd);
      return BCMRET_RESOURCE_EXCEEDED;
   }

   ifx.stringBuf = *nameList;
   ifx.opcode = opcode;
   ifr.ifr_data = (void *)&ifx;
   if (ioctl(skfd, SIOCIFREQ_EXT, &ifr) < 0)
   {
      bcmuLog_error("SIOCIFREQ_EXT opcode=0x%x failed, errno=%d", opcode, errno);
      close(skfd);
      free(*nameList);
      return BCMRET_INTERNAL_ERROR;
   }

   close(skfd);

   bcmuLog_debug("Opcode: 0x%x, WAN/LAN If list=%s, strlen=%d", opcode, *nameList, strlen(*nameList));

   return BCMRET_SUCCESS;
}
#endif


BcmRet ethswUtil_getLANOnlyEthPortIfNameList(char **LANOnlyEthPortIfNameList)
{
#ifdef DESKTOP_LINUX
   *LANOnlyEthPortIfNameList = malloc(512);
   strcpy(*LANOnlyEthPortIfNameList, "eth4");
   return BCMRET_SUCCESS;
#else
   return getWanLanAttrList(LANOnlyEthPortIfNameList, SIOCGPORTLANONLY);
#endif
}


BcmRet ethswUtil_getWANOnlyEthPortIfNameList(char **WANOnlyEthPortIfNameList)
{
#ifdef DESKTOP_LINUX
   *WANOnlyEthPortIfNameList = malloc(512);
   strcpy(*WANOnlyEthPortIfNameList, "eth0");
   return BCMRET_SUCCESS;
#else
   return getWanLanAttrList(WANOnlyEthPortIfNameList, SIOCGPORTWANONLY);
#endif
}


BcmRet ethswUtil_getLanWanPortIfNameList(char **LanWanPortIfNameList)
{
#ifdef DESKTOP_LINUX
   *LanWanPortIfNameList = malloc(512);
   strcpy(*LanWanPortIfNameList, "eth1,eth3");
   return BCMRET_SUCCESS;
#else
   return getWanLanAttrList(LanWanPortIfNameList, SIOCGPORTWANPREFERRED);
#endif
}


static UINT32 getPortListCountByPrefix(const char *ifNameList,
                                       const char *prefix, UINT32 prefixLen)
{
    char *tmpList;
    char *curIfName;
    char *ptr;
    char *savePtr = NULL;
    UINT32 count = 0;

    tmpList = strdup(ifNameList);
    if (!tmpList)
    {
        bcmuLog_error("Memory allocation failure");
        return 0;
     }

    ptr = strtok_r(tmpList, ",", &savePtr);

    while (ptr)
    {
        curIfName = ptr;
        while ((isspace((UINT8)*curIfName)) && (*curIfName != 0))
        {
            /* Skip possible white space after comma. */
            curIfName++;
        }

        if ((prefix == NULL) ||
          ((prefix != NULL) && (!strncmp(curIfName, prefix, prefixLen))))
        {
            count++;
        }
        ptr = strtok_r(NULL, ",", &savePtr);
    }

    free(tmpList);
    return count;
}


BcmRet ethswUtil_getLANOnlyEthPortNumByPrefix(const char *prefix __attribute__((unused)),
  UINT32 prefixLen __attribute__((unused)), UINT32 *numP)
{
   char *pLANOnlyPortList = NULL;
   BcmRet ret = BCMRET_SUCCESS;

#ifdef DESKTOP_LINUX
   pLANOnlyPortList = malloc(512);
   strcpy(pLANOnlyPortList, "eth2,eth4");
   *numP = getPortListCountByPrefix(pLANOnlyPortList, NULL, 0);
   free(pLANOnlyPortList);
#else
   ret = ethswUtil_getLANOnlyEthPortIfNameList(&pLANOnlyPortList);
   *numP = getPortListCountByPrefix(pLANOnlyPortList, prefix, prefixLen);
   bcmuLog_debug("LAN only: %s, number: %d\n", pLANOnlyPortList, *numP);
   free(pLANOnlyPortList);
#endif

   return ret;
}
