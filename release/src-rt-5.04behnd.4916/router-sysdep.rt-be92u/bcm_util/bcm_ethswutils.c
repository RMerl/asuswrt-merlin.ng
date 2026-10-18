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
#include <linux/ethtool.h>

#include "bcmnet.h"       // for ifreq_ext
#include "bcm_ethswutils.h"
#include "bcm_retcodes.h"
#include "bcm_ulog.h"

#define BIT_IDX_2_BIT32_VAL(bit_idx) (1 << ((bit_idx) % 32))
#define BIT_IDX_2_WORD_IDX(bit_idx) ((bit_idx) / 32)

typedef struct {
    uint32_t *ethtool_speed_type;
    uint32_t link_speed;
    uint32_t count;
}ethtool_link_speed_t;

typedef struct {
    struct ethtool_link_settings req;
    __u32 link_mode_data[3 * __SCHAR_MAX__];
} ecmd_t;

/**
 * @brief 
 * @note this array must be sorted in non-descending order by .link_speed field
 * @see find_link_speed
 */
static ethtool_link_speed_t ethtool_speeds[__TS_LAST] = {
    /*__TS_10_MBPS*/
   {
      .ethtool_speed_type = (uint32_t[]) {
         ETHTOOL_LINK_MODE_10baseT_Half_BIT,
         ETHTOOL_LINK_MODE_10baseT_Full_BIT
      },
      .link_speed = SPEED_10,
      .count = 2
   },
    /*__TS_100_MBPS*/
   {
      .ethtool_speed_type = (uint32_t[]) {
         ETHTOOL_LINK_MODE_100baseT_Half_BIT,
         ETHTOOL_LINK_MODE_100baseT_Full_BIT
      },
      .link_speed = SPEED_100,
      .count = 2
   },
   /*__TS_1000_MBPS*/
   {
      .ethtool_speed_type = (uint32_t[]) {
         ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
         ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
         ETHTOOL_LINK_MODE_1000baseKX_Full_BIT,
         ETHTOOL_LINK_MODE_1000baseX_Full_BIT
      },
      .link_speed = SPEED_1000,
      .count = 4
   },
   /*__TS_2500_MBPS*/
   {
      .ethtool_speed_type = (uint32_t[]) {
         ETHTOOL_LINK_MODE_2500baseX_Full_BIT,
         ETHTOOL_LINK_MODE_2500baseT_Full_BIT
      },
      .link_speed = SPEED_2500,
      .count = 2
   },
   /*__TS_5000_MBPS*/
   {
      .ethtool_speed_type = (uint32_t[]) {
         ETHTOOL_LINK_MODE_5000baseT_Full_BIT
      }, 
      .link_speed = SPEED_5000,
      .count = 1
   },
   /*__TS_10000_MBPS*/
   {
      .ethtool_speed_type = (uint32_t[]) {
         ETHTOOL_LINK_MODE_10000baseKX4_Full_BIT,
         ETHTOOL_LINK_MODE_10000baseKR_Full_BIT,
         ETHTOOL_LINK_MODE_10000baseER_Full_BIT,
         ETHTOOL_LINK_MODE_10000baseLRM_Full_BIT,
         ETHTOOL_LINK_MODE_10000baseLR_Full_BIT,
         ETHTOOL_LINK_MODE_10000baseSR_Full_BIT,
         ETHTOOL_LINK_MODE_10000baseCR_Full_BIT,
         ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
         ETHTOOL_LINK_MODE_10000baseR_FEC_BIT
      },
      .link_speed = SPEED_10000,
      .count = 9
   },  
};

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


BcmRet ethswUtil_getWanPreferredPortIfNameList(char **WanPreferredPortIfNameList)
{
#ifdef DESKTOP_LINUX
   *WanPreferredPortIfNameList = malloc(512);
   strcpy(*WanPreferredPortIfNameList, "eth1,eth3");
   return BCMRET_SUCCESS;
#else
   return getWanLanAttrList(WanPreferredPortIfNameList, SIOCGPORTWANPREFERRED);
#endif
}


BcmRet ethswUtil_getLanWanPortIfNameList(char **LanWanPortIfNameList)
{
#ifdef DESKTOP_LINUX
   *LanWanPortIfNameList = malloc(512);
   strcpy(*LanWanPortIfNameList, "eth1,eth3");
   return BCMRET_SUCCESS;
#else
   return getWanLanAttrList(LanWanPortIfNameList, SIOCGPORTLANWAN);
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


BcmRet ethswUtil_getLANWanEthPortNumByPrefix(const char *prefix __attribute__((unused)),
  UINT32 prefixLen __attribute__((unused)), UINT32 *numP)
{
   char *pLANWanPortList = NULL;
   BcmRet ret = BCMRET_SUCCESS;

#ifdef DESKTOP_LINUX
   pLANWanPortList = malloc(512);
   strcpy(pLANWanPortList, "eth2,eth4");
   *numP = getPortListCountByPrefix(pLANWanPortList, NULL, 0);
   free(pLANWanPortList);
#else
   ret = ethswUtil_getLanWanPortIfNameList(&pLANWanPortList);
   *numP = getPortListCountByPrefix(pLANWanPortList, prefix, prefixLen);
   bcmuLog_debug("LAN WAN: %s, number: %d\n", pLANWanPortList, *numP);
   free(pLANWanPortList);
#endif

   return ret;
}

/**
 * @brief scans the static link speed container (sbi_list) either from bottom to up, 
 * or from up to bottom to get lowest or highest link speed respectively
 * @note Base assumption: sbi_list is sorted in non-descending order by .link_speed field
 * @note Since original ethtool ETHTOOL_GLINKSETTINGS ioctl (for eth# interfaces) returns a list of all supported link speeds,
 *  we'll need to pick up the highest one from it for LAN interfaces.
 *  However, when dealing with WAN interfaces (e/gpon) there is no such a thing as a 'list-of-link-speeds', there is always a pair of up & donw link speed, 
 *  where the matter of interest would be up stream only which happens to be a lower (or equal in case of symmetric PON) one in the pair.
 *  Therefore, while dealing with eth# interfaces we'd start scanning sbi_list array from the top to the bottom to get the highest link speed, 
 *  when processing e/gpon interface we'd do the opposite i.e. start scanning the array from bottom to top to get the lowest link speed from the pair
 * @param[i] is_max_speed_caps enables to decide whether to return the lowest (wan asymmetric interace) or the highest (either wan/lan ethernet) link speed
 * @param[i] sbi_list 
 * @param[i] length 
 * @param[i] ecmd 
 * @param[o] link_speed 
 * @return int 
 */
static int find_link_speed(const BOOL is_max_speed_caps,
                           const ethtool_link_speed_t *sbi_list, 
                           const unsigned length, 
                           const ecmd_t *ecmd, 
                           uint32_t *link_speed)
{
    typedef int  (*promote_idx_t)     (int);
    typedef BOOL (*is_not_end_cond_t) (int, int);

    /* scan from bottom to top callbacks */
    int  inc_idx          (int idx)               { return idx + 1; }
    BOOL is_not_end_up    (int idx, int end_cond) { return (idx < end_cond); }

    /* scan from top to bottom callbacks */
    int  dec_idx          (int idx)               { return idx - 1; }
    BOOL is_not_end_down  (int idx, int end_cond) { return (idx >= end_cond); }

    int idx, idx_end, rc = -1;
    uint32_t j;
    promote_idx_t promote_idx_cb;
    is_not_end_cond_t not_end_cond_cb;

    if (FALSE == is_max_speed_caps)
    {

        idx             = 0;
        idx_end         = (int)length;
        promote_idx_cb  = inc_idx;
        not_end_cond_cb = is_not_end_up;
    } 
    else
    {

        idx             = (int)(length - 1);
        idx_end         = 0;
        promote_idx_cb  = dec_idx;
        not_end_cond_cb = is_not_end_down;
    }

    for (; (-1 == rc) && (TRUE == not_end_cond_cb(idx, idx_end)); idx = promote_idx_cb(idx)) /* one match is enough*/
    {
        for (j = 0; (-1 == rc) && (j < sbi_list[idx].count); j++) /* one match is enough*/
        {
            if ((BIT_IDX_2_WORD_IDX(sbi_list[idx].ethtool_speed_type[j])) >= (uint32_t)ecmd->req.link_mode_masks_nwords)
            {
                fprintf(stderr,"sbi_list[%u].ethtool_speed_type[%u] exceeds boundaries of data storage %u\n",
                        idx, j, ecmd->req.link_mode_masks_nwords);
            }
            else if (0 != (ecmd->link_mode_data[BIT_IDX_2_WORD_IDX(sbi_list[idx].ethtool_speed_type[j])] &
                           BIT_IDX_2_BIT32_VAL(sbi_list[idx].ethtool_speed_type[j])))
            {
               *link_speed = sbi_list[idx].link_speed;
               rc = 0;
            }        
        }
    }

    return rc;
}

/*
 * Function:
 *    bcm_phy_speed_get
 * Description:
 *  This function tests ethtool speeds list against network interface capabilities bitmap aka 'supported'
 * Parameters:
 *    iface             (IN)  network interface OS name.
 *    is_max_speed_caps (IN) enables to decide whether to return the lowest (wan asymmetric interace) or the highest (either wan/lan ethernet) link speed
 *    link_speed        (OUT) network interface highest supported speed (Mbps)
 * Returns:
 *    0- on success, Negative otherwise
 */
int bcm_phy_speed_get(const char *iface, const char is_max_speed_caps, uint32_t *link_speed)
{

    int fd, rc = -1;//Assuming failure

    errno = 0;
    if (NULL == iface)
    {
        fprintf(stderr,"Interface name is NULL\n");
    }
    else if (0 > (fd = socket(AF_INET, SOCK_DGRAM, 0)))// Open control socket.
    {
        fprintf(stderr,"Failed to open %s control socket, reason:%s\n", iface, strerror(errno));
    }
    else
    {
        ecmd_t ecmd;
        struct ifreq ifr;

        memset(&ecmd, 0x00, sizeof(ecmd));
        memset(&ifr, 0x00, sizeof(struct ifreq));
        ecmd.req.cmd = ETHTOOL_GLINKSETTINGS;
        strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name));
        ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
        ifr.ifr_data = (typeof(ifr.ifr_data))&ecmd;
        errno = 0;
        if ((0 > (rc = ioctl(fd, SIOCETHTOOL, &ifr))) || 
            (0 <= ecmd.req.link_mode_masks_nwords)    || 
            (ETHTOOL_GLINKSETTINGS != ecmd.req.cmd)) 
        {
            fprintf(stderr,"Failed to ioctl %s, reason: %s\n", iface, strerror(errno));
        }
        else//Success path, test the iface capability/ies
        {
            ecmd.req.cmd = ETHTOOL_GLINKSETTINGS;
            ecmd.req.link_mode_masks_nwords = -ecmd.req.link_mode_masks_nwords;
            if ((0 > (rc = ioctl(fd, SIOCETHTOOL, &ifr))) || 
                (0 >= ecmd.req.link_mode_masks_nwords)    || 
                (ETHTOOL_GLINKSETTINGS != ecmd.req.cmd))
            {
                fprintf(stderr,"Failed to ioctl %s, reason: %s\n", iface, strerror(errno));
            }
            else
            {
                rc = find_link_speed(is_max_speed_caps, ethtool_speeds, __TS_LAST, &ecmd, link_speed);
            }
        }
        close(fd);
    }

    return rc;
}
