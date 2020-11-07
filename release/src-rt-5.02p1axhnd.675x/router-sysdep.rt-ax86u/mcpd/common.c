/*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

/***************************************************************************
 * File Name  : util.c
 *
 * Description: API for utilities dealing with kernel and debug
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <bcm_local_kernel_include/linux/if_bridge.h>
#include <bcm_local_kernel_include/linux/sockios.h>
#include <linux/if_addr.h>
#include <bridgeutil.h>
#include "mcpd.h"
#include "common.h"

extern t_MCPD_ROUTER mcpd_router;
extern t_MCPD_OBJ_TYPE mcpd_malloced_objs[MCPD_MAX_OBJ];
extern char mcpd_igmp_upstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_igmp_downstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_igmp_mcast_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_strict_wan_associations[MCPD_MAX_IFS][IFNAMSIZ * 2 + 1];
#ifdef SUPPORT_MLD
extern char mcpd_mld_upstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_mld_downstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_mld_mcast_interface[MCPD_MAX_IFS][IFNAMSIZ];
#endif

char mcpd_malloced_obj_names[MCPD_MAX_OBJ][24] = {{"IF_OBJ"},
                                                  {"INTERFACE_OBJ"},
                                                  {"IPV4_ADDR_OBJ"},
                                                  {"IPV6_ADDR_OBJ"},
                                                  {"IGMP_GRP_OBJ"},
                                                  {"IGMP_REP_OBJ"},
                                                  {"IGMP_SRC_OBJ"},
                                                  {"IGMP_SRC_REP_OBJ"},
                                                  {"IGMP_REP_SRC_OBJ"},
                                                  {"MLD_GRP_OBJ"},
                                                  {"MLD_REP_OBJ"},
                                                  {"MLD_SRC_OBJ"},
                                                  {"MLD_SRC_REP_OBJ"},
                                                  {"MLD_REP_SRC_OBJ"}};

void mcpd_init_mem_utility(void)
{
    int i;

    for(i = 0; i < MCPD_MAX_OBJ; i++)
	    mcpd_malloced_objs[i] = 0;

    return;
} /* mcpd_init_mem_utility */

void mcpd_display_mem_usage(void)
{
    int i;

    printf("\nMCPD Object Memory Usage\n");
    for(i = 0; i < MCPD_MAX_OBJ; i++)
    {
        printf("%16s = %d\n", mcpd_malloced_obj_names[i],
                              mcpd_malloced_objs[i]);
    }

    return;
} /* mcpd_init_mem_utility */

void *mcpd_alloc(int obj_type, int size)
{
    void *ptr = NULL;

    ptr = malloc(size);

    if(ptr)
	    mcpd_malloced_objs[obj_type]++;

    return ptr;
} /* mcpd_alloc */

void mcpd_free(int obj_type, void *ptr)
{
    if(ptr)
    {
        mcpd_malloced_objs[obj_type]--;
        free(ptr);
    }

    return;
} /* mcpd_free */

char * mcpd_ifindex_to_name(int sockfd, unsigned int ifindex, char *ifname)
{
    struct ifreq ifr;
    int status;

    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_ifindex = ifindex;

    status = ioctl(sockfd, SIOCGIFNAME, &ifr);

    if (status < 0)
    {
        return NULL;
    }
    else
        return strncpy(ifname,ifr.ifr_name,IFNAMSIZ);
} /* mcpd_ifindex_to_name */


int mcpd_ifname_to_index(int sockfd, unsigned int *ifindex, const char *ifname )
{
    struct ifreq ifr;

    if (NULL == ifname)
        return -1;

    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
    {
        return -1;
    }

    *ifindex = ifr.ifr_ifindex;
    return 0;
}

t_MCPD_BOOL mcpd_is_wan_service_associated_with_bridge(t_MCPD_INTERFACE_OBJ *ifp_wan, t_MCPD_INTERFACE_OBJ *ifp_bridge)
{
  int i = 0;

  if (mcpd_router.igmp_config.strict_wan == 0) {
    return MCPD_TRUE;
  }

  for ( ; i < MCPD_MAX_IFS ; i ++) {
    if ( ( mcpd_strict_wan_associations[i][0] != '\0' ) &&
         ( strstr(mcpd_strict_wan_associations[i], ifp_bridge->if_name)) && 
         ( strstr(mcpd_strict_wan_associations[i], ifp_wan->if_name)   ) ) {
      MCPD_TRACE(MCPD_TRC_INFO, "Yes: %s is assoc with %s", ifp_wan->if_name, ifp_bridge->if_name);
      return MCPD_TRUE;
    }
  }
  return MCPD_FALSE;
}

/* go through all services
 * Is the service a WAN service associated with the bridge?
 * Does the WAN service have IGMP enabled?
 * As long as there is one IGMP-enabled WAN service which is a member of the bridge --> SUCCESS
 */
t_MCPD_BOOL mcpd_is_bridge_associated_with_mcast_wan_service (t_MCPD_INTERFACE_OBJ *ifp_bridge, int proto)
{
  t_MCPD_INTERFACE_OBJ *ifp_wan = mcpd_router.interfaces;
  int proxyFlag = 0;

  if (proto == MCPD_PROTO_IGMP) {
    if (mcpd_router.igmp_config.strict_wan == 0) {
      return MCPD_TRUE;
    }
    proxyFlag = MCPD_IPV4_MCAST_ENABLE;
  }
#ifdef SUPPORT_MLD
  else if (proto == MCPD_PROTO_MLD) {
    if (mcpd_router.mld_config.strict_wan == 0) {
      return MCPD_TRUE;
    }
    proxyFlag = MCPD_IPV6_MCAST_ENABLE;
  }
#endif
  else {
    return MCPD_FALSE;
  }

  for ( ; ifp_wan; ifp_wan = ifp_wan->next)
  {
    
    if ((ifp_wan->if_dir & MCPD_UPSTREAM) &&
        (ifp_wan->if_type & MCPD_IF_TYPE_ROUTED) &&
        ( mcpd_is_wan_service_associated_with_bridge (ifp_wan, ifp_bridge) == MCPD_TRUE) &&
        ( ifp_wan->proto_enable & proxyFlag) ) {
      MCPD_TRACE(MCPD_TRC_INFO, "Yes: %s is %s Proxy enabled and assoc with %s", ifp_wan->if_name, 
                                                                                 (proto == MCPD_PROTO_IGMP) ? "IGMP" : "MLD",
                                                                                 ifp_bridge->if_name);
      return MCPD_TRUE;
    }
  }
  
  MCPD_TRACE(MCPD_TRC_INFO, "No: Bridge %s has no %s Proxy enabled WAN services", ifp_bridge->if_name,
                                                                                  (proto == MCPD_PROTO_IGMP) ? "IGMP" : "MLD");
  return MCPD_FALSE;
}

int mcpd_mcast_interface_lookup(char *s, t_MCPD_PROTO_TYPE proto)
{
    int interface;
    for (interface = 0; interface < MCPD_MAX_IFS; interface++)
    {
        if ( proto != MCPD_PROTO_MLD )
        {
            if (0 == strncmp(s, mcpd_igmp_mcast_interface[interface], IFNAMSIZ))
            {
               return MCPD_TRUE;
            }
        }
#ifdef SUPPORT_MLD
        if ( proto != MCPD_PROTO_IGMP )
        {
            if (0 == strncmp(s, mcpd_mld_mcast_interface[interface], IFNAMSIZ))
            {
               return MCPD_TRUE;
            }
        }
#endif
    }
    return MCPD_FALSE;

} /* mcpd_mcast_interface_lookup */

int mcpd_upstream_interface_lookup(char *s, t_MCPD_PROTO_TYPE proto)
{
    int interface;
    for (interface = 0; interface < MCPD_MAX_IFS; interface++)
    {
        if ( proto != MCPD_PROTO_MLD )
        {
            if (0 == strncmp(s, mcpd_igmp_upstream_interface[interface], IFNAMSIZ))
            {
               return MCPD_TRUE;
            }
        }
#ifdef SUPPORT_MLD
        if ( proto != MCPD_PROTO_IGMP )
        {
            if (0 == strncmp(s, mcpd_mld_upstream_interface[interface], IFNAMSIZ))
            {
               return MCPD_TRUE;
            }
        }
#endif
    }
    return MCPD_FALSE;
} /* mcpd_upstream_interface_lookup */

int mcpd_downstream_interface_lookup(char *s, t_MCPD_PROTO_TYPE proto)
{
    int interface;
    for (interface = 0; interface < MCPD_MAX_IFS; interface++)
    {
        if ( proto != MCPD_PROTO_MLD )
        {
            if (0 == strncmp(s, mcpd_igmp_downstream_interface[interface], IFNAMSIZ))
            {
               return MCPD_TRUE;
            }
        }
#ifdef SUPPORT_MLD
        if ( proto != MCPD_PROTO_IGMP )
        {
            if (0 == strncmp(s, mcpd_mld_downstream_interface[interface], IFNAMSIZ))
            {
               return MCPD_TRUE;
            }
        }
#endif
    }
    return MCPD_FALSE;
} /* mcpd_downstream_interface_lookup */


unsigned short mcpd_in_cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    answer = ~sum;

    return (answer);
} /*mcpd_in_cksum */

static char *mcpd_get_name(char *p)
{
    /* Extract <name> from nul-terminated string
     * format <  name>: after leading whitespace.
     * NULL is returned if ':' is not found or name 
     * is larger than IFNAMSIZ
     */
    char *nameend;
    char *namestart = p;

    /* move past initial whitespace */
    while ((*namestart == ' ') || (*namestart == '\t'))
    {
        namestart++;
    }
    
    nameend = namestart;
    while(*nameend && !(*nameend == ':' || isspace(*nameend)) )
    {
        nameend++;
    }

    if (*nameend == ':')
    {
        if( (nameend - namestart) > IFNAMSIZ)
        {
            return NULL;
        }
        *nameend = '\0';
        return namestart;
    }
    else
    {
        return NULL;
    }
}

t_MCPD_IFINFO_OBJ *mcpd_get_ifinfo(short flags, short unflags)
{
    char *p;
    t_MCPD_IFINFO_OBJ *ifp, *ifprev, *list;
    struct sockaddr *psa;
    struct ifreq ifr;
    int sockfd;
    int err;
#ifdef SUPPORT_MLD
    int sockfd6;
    struct sockaddr_in6 psa6;
    int err6;
#endif /* SUPPORT_MLD */
    int wan_bridge_if;
    FILE *fh;
    char line[512];
    unsigned int ifIndex;
    int haveRoutedInterface = 0;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "can't open ipv4 socket");
        return NULL;
    }

#ifdef SUPPORT_MLD
    sockfd6 = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sockfd6 < 0)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "can't open ipv6 socket");
        close(sockfd);
        return NULL;
    }
#endif

    fh = fopen(_PATH_PROCNET_DEV, "r");
    if (NULL == fh)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "unable to get interface list");
        close(sockfd);
#ifdef SUPPORT_MLD
        close(sockfd6);
#endif
        return NULL;
    }

    list = ifp = ifprev = NULL;
    while ( fgets(line, sizeof(line), fh) ) 
    {
        p = mcpd_get_name(line);
        if (NULL == p)
        {
            continue;
        }

        if (mcpd_ifname_to_index(sockfd, &ifIndex, p) < 0 )
        {
            continue;
        }
        wan_bridge_if = MCPD_FALSE;

        /* not a downstream interface and not a WAN interface - ignore */
        if((mcpd_downstream_interface_lookup(p, MCPD_PROTO_MAX) == MCPD_FALSE) &&
            (MCPD_FALSE == mcpd_is_wan_interface(p)))
            continue;

        memset(&ifr, 0, sizeof(struct ifreq));
        strncpy(ifr.ifr_name, p, IFNAMSIZ);
        err = ioctl(sockfd, SIOCGIFADDR, (void*)&ifr);
        psa = &ifr.ifr_ifru.ifru_addr;
#ifdef SUPPORT_MLD
        memset(&psa6, 0, sizeof(struct sockaddr_in6));
        err6 = mcpd_get_ipv6_addr_by_ifidx(ifIndex, &psa6);
        if (err6 < 0)
            MCPD_TRACE(MCPD_TRC_LOG, "error can't get ip addr by idx = %d", ifIndex);
#endif
        if ( mcpd_is_wan_interface(p) )
        {
           if ( ((err < 0) || (psa->sa_family != AF_INET))
#ifdef SUPPORT_MLD
                && ((err6 < 0) || (psa6.sin6_family != AF_INET6))
#endif
              )
           {
              if ( ( MCPD_TRUE == mcpd_wan_is_bridge_member(ifIndex)) &&
                   ( MCPD_TRUE == mcpd_mcast_interface_lookup(p, MCPD_PROTO_MAX)) )
              {
                 short ifflags = mcpd_get_interface_flags(p);
                 if ( ifflags & IFF_RUNNING )
                 {
                     wan_bridge_if = MCPD_TRUE;
                 }
                 else
                 {
                     continue;
                 }
              }
              else 
              {
                 continue;
              }
               
           }
           else
           {
              /* WAN interface with either a V4 or V6 IP must be
                 listed as an mcast interface for MCPD to learn it */
              if ( ( MCPD_FALSE == mcpd_mcast_interface_lookup(p, MCPD_PROTO_MAX)) &&
                   ( MCPD_FALSE == mcpd_upstream_interface_lookup(p, MCPD_PROTO_MAX)) )
              {
                 continue;
              }
              haveRoutedInterface = 1;
           }
        }
        else
        {
           if ( 0 == mcpd_is_bridge(p))
           {
               continue;
           }
           wan_bridge_if = mcpd_bridge_includes_wan_interface(p);
        }

        err = ioctl(sockfd, SIOCGIFFLAGS, (void*)&ifr);
        if (err == -1)
        {
            continue;
        }

        if (((ifr.ifr_flags & flags) != flags) ||
            ((ifr.ifr_flags & unflags) != 0))
        {
            continue;
        }

        ifp = (t_MCPD_IFINFO_OBJ *) malloc(sizeof(*ifp));
        if (ifp)
        {
            memset(ifp, 0, sizeof(t_MCPD_IFINFO_OBJ));
            ifp->index = ifIndex;
            strncpy(ifp->name, ifr.ifr_name, IFNAMSIZ);
            memcpy(&ifp->addr, psa, sizeof(*psa));
#ifdef SUPPORT_MLD
            memcpy(&ifp->addr6, &psa6, sizeof(struct sockaddr_in6));
#endif
            if(wan_bridge_if == MCPD_TRUE)
            {
                ifp->iftype = MCPD_IF_TYPE_BRIDGED;
            }
            else
            {
               ifp->iftype = MCPD_IF_TYPE_ROUTED;
            }

            ifp->next = NULL;

            if (list == NULL)
                list = ifp;

            if (ifprev != NULL)
                ifprev->next = ifp;

            ifprev = ifp;
        }
    }

#if defined(MCPD_ASSOCIATE_DS_TO_ALL_UP)
    for(ifp = list; ifp != NULL; ifp = ifp->next)
    {   
       /* downstream interface needs to be associated
          with a routed interface if a routed upstream interface 
          is present */
       if (mcpd_downstream_interface_lookup(ifp->name, MCPD_PROTO_MAX))
       {
          if (haveRoutedInterface)
          {
              ifp->iftype |= MCPD_IF_TYPE_ROUTED;
          }
       }
    }
#endif

    close(sockfd);
#ifdef SUPPORT_MLD
    close(sockfd6);
#endif
    fclose(fh);

    return list;
} /* mcpd_get_ifinfo */

void mcpd_free_ifinfo(t_MCPD_IFINFO_OBJ *ifl)
{
    t_MCPD_IFINFO_OBJ *ifp = ifl;

    while (ifp)
    {
        ifl = ifp;
        ifp = ifp->next;
        free(ifl);
    }

    return;
} /* mcpd_free_ifinfo */

short mcpd_get_interface_flags(char *ifname)
{
    struct ifreq ifr;
    int sockfd, err;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        return -1;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    err = ioctl(sockfd, SIOCGIFFLAGS, (void*)&ifr);

    close(sockfd);

    if (err == -1)
        return -1;

    return ifr.ifr_flags;
} /* mcpd_get_interface_flags */

short mcpd_set_interface_flags(char *ifname, short flags)
{
    struct ifreq ifr;
    int sockfd, err;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
         return -1;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags = flags;

    err = ioctl(sockfd, SIOCSIFFLAGS, (void*)&ifr);

    close(sockfd);

    if (err == -1)
        return -1;

    return 0;
} /* mcpd_set_interface_flags */

int mcpd_get_interface_mtu(char *ifname)
{
    struct ifreq ifr;
    int sockfd, err;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        return -1;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    err = ioctl(sockfd, SIOCGIFMTU, (void*)&ifr);

    close(sockfd);

    if (err == -1)
        return -1;

    return ifr.ifr_mtu;
} /* mcpd_get_interface_mtu */

int mcpd_is_valid_ipaddr(char *ifname)
{
    struct sockaddr *psa;
    struct ifreq ifr;
    int sockfd;
    int err;
    struct sockaddr_in *psin;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        return MCPD_FALSE;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    err = ioctl(sockfd, SIOCGIFADDR, (void*)&ifr);
    psa = &ifr.ifr_ifru.ifru_addr;

    if (err == -1 || psa->sa_family != AF_INET || psa->sa_family != PF_INET)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "can't get Addr");
        close(sockfd);
        return MCPD_FALSE;
    }
    close(sockfd);

    psin = (struct sockaddr_in *)psa;

    if(0 == psin->sin_addr.s_addr)
        return MCPD_FALSE;
    else
        return MCPD_TRUE;
} /* mcpd_is_valid_ipaddr */

int mcpd_is_wan_interface(char *ifname)
{
    struct ifreq ifr;
    int sockfd, err;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        return -1;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    err = ioctl(sockfd, SIOCDEVISWANDEV, (void*)&ifr);
    close(sockfd);

    if((err != -1) && (ifr.ifr_flags != 0))
    {
        return MCPD_TRUE;
    }
    else
    {
        return MCPD_FALSE;
    }
}

/** Get all WAN interfaces in the bridge
 *
 * @param *brName         (IN) Bridge
 *
 * @param ifindices[]     (OUT) WAN If Indices array
 * 
 * @param wancnt          (OUT) Number of WAN ifindices
 * 
 * @return 0 - Success, -1 - Otherwise
 *
 */
int mcpd_get_bridge_wan_interfaces(char *brname, 
                                   int ifindices[], 
                                   unsigned int *wancnt)
{
    int rt;

    rt = br_util_get_bridge_wan_interfaces(brname, ifindices, wancnt);
#if defined(CONFIG_BCM_OVS_MCAST)
   if (rt || wancnt == 0) 
   {
       /* Check if this is a OVS bridge and the WAN interface is
          part of ovs bridge */
       rt = mcpd_ovs_get_bridge_wan_interfaces(brname, ifindices, wancnt);
   }
#endif
   return rt;
}

/** Check if the input bridge contains a WAN interface
 *
 * @param *brName         (IN) Bridge
 *
 * @return TRUE - Bridge contains WAN interface
 *         FALSE - Bridge does not contain WAN interface
 *
 */
int mcpd_bridge_includes_wan_interface(char *brname)
{
   int          ifindices[BRIDGE_MAX_IFS];
   unsigned int wancnt = BRIDGE_MAX_IFS;
   char         ifname[IFNAMSIZ];
   int          rt;
   unsigned int wanidx;

   rt = mcpd_get_bridge_wan_interfaces(brname, ifindices, &wancnt);

   if ((0 == rt) && (wancnt > 0 ))
   {
      for ( wanidx = 0; wanidx < wancnt; wanidx++ )
      {
         int i;
         if (NULL == mcpd_ifindex_to_name(mcpd_router.sock_igmp, ifindices[wanidx], ifname))
         {
            continue;
         }

         /* if the bridge port is listed as a proxy interface then ignore it */
         for(i = 0; i < MCPD_MAX_IFS; i++)
         {
            /* proxy enabled. so routed interface ex: ppp part of bridge */
            if(0 == strncmp(ifname, mcpd_igmp_upstream_interface[i], IFNAMSIZ))
            {
                break;
            }
         }
         if (i < MCPD_MAX_IFS)
         {
            continue;
         }

#ifdef SUPPORT_MLD
         for(i = 0; i < MCPD_MAX_IFS; i++)
         {
            if (strlen(mcpd_mld_upstream_interface[i]) > 1) 
            {
               if(0 == strncmp(ifname, 
                               mcpd_mld_upstream_interface[i], IFNAMSIZ))
               {
                  break;
               }
            }
         }
         if (i < MCPD_MAX_IFS)
         {
            continue;
         }
#endif
         /* if the bridge port has a valid ip address ignore it */
         if(MCPD_TRUE == mcpd_is_valid_ipaddr(ifname)) 
         {
            continue;
         }

         /* WAN bridge found */
         return MCPD_TRUE;
      }
   }
   return MCPD_FALSE;
} /* mcpd_bridge_includes_wan_interface */

/** Check if the ifindex belongs to any of the bridges
 *  in the input array
 *
 * @param *ifindex         (IN) Port Ifindex
 * 
 * @param brifindices[]    (IN) Array of bridge ifindices
 * 
 * @param numbr            (IN) number of bridges in array
 *   
 * @return TRUE  - ifindex belongs to one of the input bridges
 *         FALSE - ifindex does not belong to any of the input
 *                 bridges
 *
 */
int mcpd_is_ifindex_bridge_member(int brifindices[], int numbr, int ifindex)
{
    int bridx;
    char         brname[IFNAMSIZ];
    int          ret = MCPD_FALSE;

    for ( bridx = 0; bridx < numbr; bridx++ )
    {
        if (NULL == mcpd_ifindex_to_name(mcpd_router.sock_igmp, brifindices[bridx], brname))
        {
            MCPD_TRACE(MCPD_TRC_ERR, "Error retrieving name for ifindex %d",
                       brifindices[bridx]);
            continue;
        }

        ret = mcpd_is_bridge_member(brname, ifindex);
        if ( ret == MCPD_TRUE )
        {
            break;
        }
    }
    return ret;
}

/** Check if the input wan ifindex belongs to any bridge
 *
 * @param *ifindex         (IN) WAN Ifindex
 * 
 * @return TRUE  - WAN belongs to a bridge
 *         FALSE - WAN does not belong to a bridge
 *
 */
int mcpd_wan_is_bridge_member(int ifindex)
{
   int          ifindices[MCPD_MAX_IFS];
   unsigned int numbr = MCPD_MAX_IFS;
   int          rt;
   int          membership_found = MCPD_FALSE;

   rt = br_util_get_bridges(ifindices, &numbr);

   if ( (0 == rt) && (numbr > 0) )
   {
       membership_found = mcpd_is_ifindex_bridge_member(ifindices, numbr, ifindex);
   }

#if defined(CONFIG_BCM_OVS_MCAST)
   if (!membership_found) 
   {
       /* Check if ifindex is part of OvS bridges */
       numbr = 0;
       rt = mcpd_ovs_get_bridges(ifindices, &numbr);

       if ( (0 == rt) && (numbr > 0) )
       {
           membership_found = mcpd_is_ifindex_bridge_member(ifindices, numbr, ifindex);
       }
   }
#endif

   return membership_found;
} /* mcpd_wan_is_bridge_member */

/** Get all the ports in the bridge
 *
 * @param *brName         (IN) Bridge
 * 
 * @param ifindices[]     (OUT) port ifindices array
 * 
 * @param num             (OUT) number of ports
 *   
 * @return 0 - Success, -1 - Otherwise
 *
 */
int mcpd_get_bridge_members(char *brName, 
                            int ifindices[], 
                            unsigned int *numprt)
{
   int rt;

   rt = br_util_get_bridge_port_interfaces(brName, ifindices, numprt);
#if defined(CONFIG_BCM_OVS_MCAST)
   if (rt || *numprt == 0) 
   {
       /* Check if this is a OvS bridge. If then get OvS bridge members */
       rt = mcpd_ovs_get_bridge_port_interfaces(brName, ifindices, numprt);
   }
#endif
   return rt;
}

/** Check if the port belongs to the bridge
 *
 * @param *brName         (IN) Bridge
 * 
 * @param *ifIndex        (IN) port ifindex
 *  
 * @return TRUE - Port belongs to the bridge
 *         FALSE - Port does not belong to the bridge
 *
 */
int mcpd_is_bridge_member(char *brName, int ifIndex)
{
   int          ifindices[BRIDGE_MAX_IFS];
   unsigned int numprt = BRIDGE_MAX_IFS;
   int          rt;
   unsigned int prtidx;

   rt = mcpd_get_bridge_members(brName, ifindices, &numprt);

   if ( (0 == rt) && (numprt > 0) )
   {
      for( prtidx = 0; prtidx < numprt; prtidx++ )
      {
         if (ifIndex == ifindices[prtidx])
         {
            return MCPD_TRUE;
         }
      }
   }
   return MCPD_FALSE;
} /* mcpd_is_bridge_member */


/** Check if the input interface is a bridge
 *
 * @param *ifname         (IN) Interface name
 * 
 * @return TRUE - Interface is a bridge
 *         FALSE - Interface is not a bridge
 *
 */
int mcpd_is_bridge(char *ifname)
{
#if defined(CONFIG_BCM_OVS_MCAST)
   return (br_util_is_bridge(ifname) || mcpd_is_ovs_bridge(ifname, NULL));
#else
   return (br_util_is_bridge(ifname));
#endif
}

#ifdef SUPPORT_MLD
void mcpd_print_ipv6_addr(struct in6_addr *addr6)
{
    printf("\n\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", 
            ntohs(addr6->s6_addr16[0]), ntohs(addr6->s6_addr16[1]),
            ntohs(addr6->s6_addr16[2]), ntohs(addr6->s6_addr16[3]),
            ntohs(addr6->s6_addr16[4]), ntohs(addr6->s6_addr16[5]),
            ntohs(addr6->s6_addr16[6]), ntohs(addr6->s6_addr16[7]));

    return;
} /* mld_print_ipv6_addr */

int mcpd_check_ipv6_dad_status(int if_index)
{
    FILE *f;
    char addr6p[8][5];
    char devname[20];
    int if_idx, scope, plen, dad_status;
    int status = -1;

    f = fopen(_MLD_PATH_PROCNET_IFINET6, "r");
    if (f != NULL)
    {
        while(fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
                      addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
                      addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
                      &dad_status, devname) != EOF)
        {
            /* check only link local address */
            if( (if_index == if_idx) && (0x20 & scope))
            {
                if ((IFA_F_TENTATIVE & dad_status) == 0)
                {
                    status = 0;
                    break;
                }
            }
        }
        fclose(f);
    }

    return status;
}

int mcpd_get_ipv6_addr_by_ifidx(int if_index, struct sockaddr_in6 *saddr6)
{
    FILE *f;
    char addr6p[8][5];
    char addr6[40], devname[20];
    struct sockaddr_in6 sap;
    int if_idx, scope, plen, dad_status;
    int status = -1;

    f = fopen(_MLD_PATH_PROCNET_IFINET6, "r");
    if (f != NULL)
    {
        while(fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
                      addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
                      addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
                      &dad_status, devname) != EOF)
        {
            /* ignore link local address */
            if( (if_index == if_idx) && (0 == (0x20 & scope)))
            {
                sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
                        addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                        addr6p[4], addr6p[5], addr6p[6], addr6p[7]);

                memset(&sap, 0, sizeof(struct sockaddr_in6));
                inet_pton(AF_INET6, addr6, (struct sockaddr *) &sap.sin6_addr);
                sap.sin6_family = AF_INET6;
                memcpy(saddr6, &sap, sizeof(struct sockaddr_in6));
                status = 0;
                break;
            }
        }
        fclose(f);
    }

    return status;
} /* mcpd_get_ipv6_addr_by_ifidx */
#endif

#ifdef MCPD_DEBUG
void mcpd_dump_buf(char *buf, int len)
{
    int i;

    puts("==========================MCPD START================================");

    for(i =0; i < len; i++)
    {
        printf("%02x", (unsigned char)buf[i]);
        if(!((i+1)%2))
            printf(" ");
        if(!((i+1)%16))
            printf("\n");
    }
    printf("\n");

    puts("==========================MCPD END==================================");

    return;
} /* mcpd_dump_buf */
#else
void mcpd_dump_buf(char *buf __attribute__((unused)), int len __attribute__((unused)))
{
    return;
}
#endif /* MCPD_DEBUG */

