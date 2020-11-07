/***********************************************************************
 *
 *  Copyright (c) 2009-2010  Broadcom Corporation
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

/***************************************************************************
 * File Name  : mcpd_config.c
 *
 * Description:
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include "mcpd.h"
#include "prctl.h"
#include "mcpd_config.h"
#include "common.h"
#include "errno.h"
#include <arpa/inet.h>

#define MCPD_CONFIG_FILE  "/var/mcpd.conf"

//extern int isblank(int c);

extern t_MCPD_ROUTER mcpd_router;
extern char mcpd_igmp_upstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_igmp_downstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_igmp_mcast_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_strict_wan_associations[MCPD_MAX_IFS][IFNAMSIZ * 2 + 1];
#ifdef SUPPORT_MLD
extern char mcpd_mld_upstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_mld_downstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
extern char mcpd_mld_mcast_interface[MCPD_MAX_IFS][IFNAMSIZ];
#endif

int mcpd_writeToProc(const char *procFilename, const char *s)
{
   int ret = 0;

   SINT32 fd, rc;

   if ((fd = open(procFilename, O_RDWR)) < 0)
   {
      printf("could not open %s", procFilename);
      return -1;
   }

   rc = write(fd, s, strlen(s));

   if (rc < (SINT32) strlen(s))
   {
      printf("write %s to %s failed, rc=%d", s, procFilename, rc);
      ret = -1;
   }

   close(fd);

   return ret;
}

int mcpd_config_write(t_MCPD_IGMP_CONFIG *igmp_config __attribute__((unused)),
                      t_MCPD_MLD_CONFIG  *mld_config __attribute__((unused)))
{
    FILE *fp = NULL;

    fp = fopen(MCPD_CONFIG_FILE, "w");

    if(fp == NULL)
    {
        perror("MCPD can't open file\n");
        return -1;
    }

    fprintf(fp, "#\n");
    fprintf(fp, "#Begin IGMP configuration\n");
    fprintf(fp, "#\n");
    fprintf(fp, "igmp-default-version %d\n", 3);
    fprintf(fp, "igmp-query-interval %d\n", 20);
    fprintf(fp, "igmp-query-response-interval %d\n", 10);
    fprintf(fp, "igmp-last-member-query-interval %d\n", 10);
    fprintf(fp, "igmp-robustness-value %d\n", 2);
    fprintf(fp, "igmp-max-groups %d\n", 10);
    fprintf(fp, "igmp-max-sources %d\n", 10);
    fprintf(fp, "igmp-max-members %d\n", 10);
    fprintf(fp, "igmp-fast-leave %d\n", 1);
    fprintf(fp, "igmp-admission-required %d\n", 0);
    fprintf(fp, "igmp-admission-bridging-filter %d\n", 0);
    fprintf(fp, "igmp-proxy-interfaces %s\n", "atm0");
    fprintf(fp, "igmp-snooping-interfaces %s\n", "br0");
    fprintf(fp, "igmp-mcast-interfaces %s\n", "atm0");
    fprintf(fp, "#\n");
    fprintf(fp, "#End IGMP configuration\n");
    fprintf(fp, "#\n");

#ifdef SUPPORT_MLD
    fprintf(fp, "#\n");
    fprintf(fp, "#Begin MLD configuration\n");
    fprintf(fp, "#\n");

    fprintf(fp, "mld-default-version %d\n", 2);
    fprintf(fp, "mld-query-interval %d\n", 125);
    fprintf(fp, "mld-query-response-interval %d\n", 10);
    fprintf(fp, "mld-last-member-query-interval %d\n", 10);
    fprintf(fp, "mld-robustness-value %d\n", 2);
    fprintf(fp, "mld-max-groups %d\n", 10);
    fprintf(fp, "mld-max-sources %d\n", 10);
    fprintf(fp, "mld-max-members %d\n", 10);
    fprintf(fp, "mld-fast-leave %d\n", 1);
    fprintf(fp, "mld-admission-required %d\n", 0);
    fprintf(fp, "mld-admission-bridging-filter %d\n", 0);
    fprintf(fp, "mld-proxy-enable %d\n", 1);
    fprintf(fp, "mld-snooping-enable %d\n", 1);
    fprintf(fp, "mld-proxy-interfaces %s\n", "atm0");
    fprintf(fp, "mld-snooping-interfaces %s\n", "br0");
    fprintf(fp, "mld-mcast-interfaces %s\n", "atm0");
    fprintf(fp, "#\n");
    fprintf(fp, "#End MLD configuration\n");
    fprintf(fp, "#\n");
#endif /* SUPPORT_MLD */

    fclose(fp);

    return 0;
} /* mcpd_config_write */

void mcpd_config_apply_igmp_version(int version)
{
    char cmd[CLI_MAX_BUF_SZ];
    int ret;

    sprintf(cmd,"%d",version);
    ret = mcpd_writeToProc("/proc/sys/net/ipv4/conf/all/force_igmp_version", cmd);
    if ( ret == -1 )
    {
       MCPD_TRACE(MCPD_TRC_ERR, "Failed to set IGMP version");
    }

    return;
} /* mcpd_config_apply_igmp_version */

#ifdef SUPPORT_MLD
void mcpd_config_apply_mld_version(int version)
{
    char cmd[10];
    int ret;

    sprintf(cmd,"%d",version);
    ret = mcpd_writeToProc("/proc/sys/net/ipv6/conf/all/force_mld_version", cmd);
    if ( ret == -1 )
    {
       MCPD_TRACE(MCPD_TRC_ERR, "Failed to set MLD version");
    }

    return;
} /* mcpd_config_apply_mld_version */
#endif /* SUPPORT_MLD */

void mcpd_config_to_kernel (t_MCPD_IGMP_CONFIG *igmp_config,
                          t_MCPD_MLD_CONFIG  *mld_config)
{
    char cmd[10];

    sprintf(cmd, "%d", igmp_config->max_sources);
    mcpd_writeToProc("/proc/sys/net/ipv4/igmp_max_msf", cmd);
    sprintf(cmd, "%d", igmp_config->max_groups + 5);
    mcpd_writeToProc("/proc/sys/net/ipv4/igmp_max_memberships", cmd);
#ifdef SUPPORT_MLD
    sprintf(cmd, "%d", mld_config->max_sources);
    mcpd_writeToProc("/proc/sys/net/ipv6/mld_max_msf", cmd);
#endif /* SUPPORT_MLD */
}

void mcpd_config_validate(t_MCPD_IGMP_CONFIG *igmp_config,
                          t_MCPD_MLD_CONFIG  *mld_config)
{       
    if((igmp_config->default_version < 1) ||
        (igmp_config->default_version > 3))
        igmp_config->default_version = 3;

    if(igmp_config->robust_val < 1) {
        igmp_config->robust_val = 2;
    }

    if(igmp_config->max_sources < 1) {
        igmp_config->max_sources = MCPD_DFLT_MAX_SOURCES;
    }    

    if(igmp_config->max_groups < 1) {
        igmp_config->max_groups = MCPD_DFLT_MAX_GROUPS;
    }

    if(igmp_config->max_members < 1) {
        igmp_config->max_members = MCPD_DFLT_MAX_REPORTERS;
    }

    if ((igmp_config->admission_bridging_filter > 1) || (igmp_config->admission_required == 0)) {
      igmp_config->admission_bridging_filter = 0;
    }

    if (igmp_config->flood_enable > 1) {
        igmp_config->flood_enable = 1;
    }


#ifdef SUPPORT_MLD
    if((mld_config->default_version < 1) ||
        (mld_config->default_version > 2)) {
        mld_config->default_version = 2;
    }

    if(mld_config->robust_val < 1) {
        mld_config->robust_val = 2;
    }

    if(mld_config->max_sources < 1) {
        mld_config->max_sources = MCPD_DFLT_MAX_SOURCES;   
    }

    if(mld_config->max_groups < 1) {
        mld_config->max_groups = MCPD_DFLT_MAX_GROUPS;
    }

    if(mld_config->max_members < 1) {
        mld_config->max_members = MCPD_DFLT_MAX_REPORTERS;
    }

    if ((mld_config->admission_bridging_filter > 1) || (mld_config->admission_required == 0)) {
      mld_config->admission_bridging_filter = 0;
    }

    if (mld_config->flood_enable > 1) {
        mld_config->flood_enable = 1;
    }

#endif /* SUPPORT_MLD */

    return;
}
    
t_MCPD_IGMP_CONFIG igmp_defaults = 
{
  .default_version       = 3,
  .query_interval        = 125,         /* seconds */
  .query_resp_interval   = 100,         /* tenths of a second */
  .lmqi                  = 10,          /* last-member-query-interval, (tenths of a second) */
  .robust_val            = 2,           /* igmp-robustness-value, also last-member-query-count */
  .max_groups            = 25,
  .max_sources           = 25,
  .max_members           = 25,
  .fast_leave_enable     = 1,
  .strict_wan            = 0,
  .startup_query_interval= 20,          /* seconds */
  .startup_query_count   = 7,
  .admission_required    = 0,
  .admission_bridging_filter = 0,
  .flood_enable          = 0,
};

#ifdef SUPPORT_MLD
t_MCPD_MLD_CONFIG mld_defaults = 
{
  .default_version       = 2,
  .query_interval        = 125,         /* seconds */
  .query_resp_interval   = 100,         /* tenths of a second */
  .lmqi                  = 10,          /* last-member-query-interval, (tenths of a second) */
  .robust_val            = 2,           /* igmp-robustness-value, also last-member-query-count */
  .max_groups            = 25,
  .max_sources           = 25,
  .max_members           = 25,
  .fast_leave_enable     = 1,
  .strict_wan            = 0,
  .admission_required    = 0,
  .admission_bridging_filter = 0,
  .flood_enable          = 0,
};
#endif

static void mcpd_config_defaults(t_MCPD_IGMP_CONFIG *igmp_config,
                     t_MCPD_MLD_CONFIG  *mld_config)
{
    if (igmp_config) {
        memcpy (igmp_config, &igmp_defaults, sizeof(t_MCPD_IGMP_CONFIG));
        strcpy(mcpd_igmp_downstream_interface[0], "br0");
    }
#ifdef SUPPORT_MLD
    if (mld_config) {
        memcpy (mld_config, &mld_defaults, sizeof(t_MCPD_MLD_CONFIG));
        strcpy(mcpd_mld_downstream_interface[0], "br0");
    }
#endif
}

int mcpd_config_read(t_MCPD_IGMP_CONFIG *igmp_config,
                     t_MCPD_MLD_CONFIG  *mld_config)
{
    FILE *fp = NULL;
    char buffer[1024], *token, *line;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    int i = 0;
    t_MCPD_FILTER_EXCEPTION *exceptionObj = mcpd_router.igmp_config.filter_list;
    t_MCPD_FILTER_EXCEPTION *nextExceptionObj = NULL;

    fp = fopen(MCPD_CONFIG_FILE, "r");
    if (fp == NULL)
    {
       MCPD_TRACE(MCPD_TRC_LOG, "MCPD can't open file mcpd.conf error = %d", errno);
       mcpd_config_defaults(igmp_config, mld_config);
       return -1;
    }

    while (exceptionObj)
    {
        nextExceptionObj = exceptionObj->next;
        MCPD_FREE(MCPD_EXCEPTION_OBJ, exceptionObj);
        exceptionObj = nextExceptionObj;
    }
    mcpd_router.igmp_config.filter_list = NULL;
#ifdef SUPPORT_MLD
    exceptionObj = mcpd_router.mld_config.filter_list;
    while (exceptionObj)
    {
        nextExceptionObj = exceptionObj->next;
        MCPD_FREE(MCPD_EXCEPTION_OBJ, exceptionObj);
        exceptionObj = nextExceptionObj;
    }
    mcpd_router.mld_config.filter_list = NULL;
#endif

    for(i = 0; i < MCPD_MAX_IFS; i++)
    {
        mcpd_igmp_upstream_interface[i][0] = '\0';
        mcpd_igmp_downstream_interface[i][0] = '\0';
        mcpd_igmp_mcast_interface[i][0] = '\0';
#ifdef SUPPORT_MLD
        mcpd_mld_upstream_interface[i][0] = '\0';
        mcpd_mld_downstream_interface[i][0] = '\0';
        mcpd_mld_mcast_interface[i][0] = '\0';
#endif
    }
    bzero (&mcpd_router.max_group_port_list, sizeof(t_MCPD_PORT_MAX_GROUP) * MAX_PORTS);
    
    igmp_config->admission_bridging_filter = (MULTICAST_DEFAULT_ADMISSION_FILTER_MODE == MULTICAST_ADMISSION_FILTER_MODE_ON_WITH_FILTER) ? 1 : 0;
    igmp_config->admission_required = (MULTICAST_DEFAULT_ADMISSION_FILTER_MODE == MULTICAST_ADMISSION_FILTER_MODE_OFF) ? 0 : 1;
#ifdef SUPPORT_MLD
    mld_config->admission_bridging_filter = (MULTICAST_DEFAULT_ADMISSION_FILTER_MODE == MULTICAST_ADMISSION_FILTER_MODE_ON_WITH_FILTER) ? 1 : 0;
    mld_config->admission_required = (MULTICAST_DEFAULT_ADMISSION_FILTER_MODE == MULTICAST_ADMISSION_FILTER_MODE_OFF) ? 0 : 1;
#endif

    if (flock(fileno(fp), LOCK_EX )) 
    {
       MCPD_TRACE(MCPD_TRC_ERR, "MCPD can't get file lock for mcpd.conf error = %d", errno);
       fclose(fp);
       return -1;
    }

    while(fgets(buffer, sizeof(buffer), fp))
    {
        if(strchr(buffer, '\n'))
            *(strchr(buffer, '\n')) = '\0';

        if(strchr(buffer, '#'))
            *(strchr(buffer, '#')) = '\0';

        token = buffer + strspn(buffer, " \t");
        if (*token == '\0') continue;

        line = token + strcspn(token, " \t=");
        if (*line == '\0') continue;

        *line = '\0';
        line++;

        line = line + strspn(line, " \t=");
        if (*line == '\0') continue;

        if(strcasecmp(token, "igmp-default-version") == 0)
        {
            igmp_config->default_version = strtoul(line, NULL, 0);
        }

        if(strcasecmp(token, "igmp-query-interval") == 0)
        {
            igmp_config->query_interval = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-query-response-interval") == 0)
        {
            igmp_config->query_resp_interval = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-last-member-query-interval") == 0)
        {
            igmp_config->lmqi = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-robustness-value") == 0)
        {
            igmp_config->robust_val = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-max-groups") == 0)
        {
            igmp_config->max_groups = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-max-sources") == 0)
        {
            igmp_config->max_sources = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-max-members") == 0)
        {
            igmp_config->max_members = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-fast-leave") == 0)
        {
            igmp_config->fast_leave_enable = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mcpd-strict-wan") == 0)
        {
            igmp_config->strict_wan = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-admission-required") == 0)
        {
            igmp_config->admission_required = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-admission-bridging-filter") == 0)
        {
            igmp_config->admission_bridging_filter= strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-flood-enable") == 0)
        {
            igmp_config->flood_enable = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "igmp-proxy-interfaces") == 0)
        {
            i = 0;
            ptr1 = strtok_r(line, " ", &ptr2);
            while((ptr1) && (i < MCPD_MAX_IFS))
            {
                strncpy(mcpd_igmp_upstream_interface[i], ptr1, IFNAMSIZ);
                i++;
                ptr1 = strtok_r(NULL, " ", &ptr2);
                if(!ptr1) break;
            }
        }
        else if(strcasecmp(token, "igmp-snooping-interfaces") == 0)
        {
            i = 0;
            ptr1 = strtok_r(line, " ", &ptr2);
            while((ptr1) && (i < MCPD_MAX_IFS))
            {
                strncpy(mcpd_igmp_downstream_interface[i], ptr1, IFNAMSIZ);
                i++;
                ptr1 = strtok_r(NULL, " ", &ptr2);
                if(!ptr1) break;
            }
        }
        else if(strcasecmp(token, "igmp-mcast-interfaces") == 0)
        {
            i = 0;
            ptr1 = strtok_r(line, " ", &ptr2);
            while((ptr1) && (i < MCPD_MAX_IFS))
            {
                strncpy(mcpd_igmp_mcast_interface[i], ptr1, IFNAMSIZ);
                i++;
                ptr1 = strtok_r(NULL, " ", &ptr2);
                if(!ptr1) break;
            }
        }
        else if(strcasecmp(token, "igmp-mcast-snoop-exceptions") == 0)
        {
            ptr1 = strtok_r(line, " ", &ptr2);
            while(ptr1)
            {
                char *slashPtr = NULL;
                int result = 0;
                struct in6_addr exceptionAddress;
                struct in6_addr maskAddress;
                int maskFound = 0;
                
                MCPD_TRACE(MCPD_TRC_INFO, "MCPD SnoopException Rule - %s",ptr1);
                slashPtr = strchr(ptr1, '/');
                if (slashPtr)
                {
                  maskFound = inet_pton (AF_INET, slashPtr+1, &maskAddress);
                  slashPtr[0] = '\0';
                }
                if (1 != maskFound)
                {
                  maskAddress.s6_addr32[0] = 0xffffffff;
                }
                result = inet_pton (AF_INET, ptr1, &exceptionAddress);

                if (( 1 == result ) && 
                    ( (maskAddress.s6_addr32[0]      & htonl(0xF0000000)) == htonl(0xF0000000) )&&
                    ( (exceptionAddress.s6_addr32[0] & htonl(0xF0000000)) == htonl(0xE0000000) ) ) 
                {
                  // Add to linked list
                  exceptionObj = (t_MCPD_FILTER_EXCEPTION *) MCPD_ALLOC(MCPD_EXCEPTION_OBJ, sizeof(t_MCPD_FILTER_EXCEPTION));
                  if (NULL == exceptionObj)
                  {
                    MCPD_TRACE(MCPD_TRC_ERR, "MCPD SnoopException Rule - MEMORY FAIL");
                  }
                  else
                  {
                    // Insertion at head of list
                    exceptionObj->address.s6_addr32[0] = exceptionAddress.s6_addr32[0];
                    exceptionObj->mask.s6_addr32[0]    = maskAddress.s6_addr32[0];
                    exceptionObj->next = mcpd_router.igmp_config.filter_list;
                    mcpd_router.igmp_config.filter_list = exceptionObj;
                  }
                }
                else
                {
                  MCPD_TRACE(MCPD_TRC_ERR, "MCPD SnoopException Rule REJECTED");
                }
                ptr1 = strtok_r(NULL, " ", &ptr2);
            }
        }
        else if(strcasecmp(token, "mcpd-strict-wan-assoc") == 0)
        {
            char *start = line;
            char *endOfPair = NULL;
            i = 0;
            do {
               // find the delimiter and mark it NULL
               endOfPair = strchr(start, '|');
               if (endOfPair != NULL) {
                   endOfPair[0] = '\0';
               }
               // copy over the pairing               
               strncpy(mcpd_strict_wan_associations[i], start, IFNAMSIZ * 2);
               mcpd_strict_wan_associations[i][IFNAMSIZ*2] = '\0';
               // move the start pointer to the char after the pipe
               if (endOfPair != NULL) {
                   start = endOfPair + 1;
               }
               i++;
            } while ((endOfPair != NULL) && (i < MCPD_MAX_IFS));
            // Clear out any lingering associations
            for ( ; i<MCPD_MAX_IFS ; i++) {
               mcpd_strict_wan_associations[i][0] = '\0';
            }
        }


        else if(strcasecmp(token, "mcast-max-groups-port-list") == 0)
        {
            int port_ifi = -1;
            i = 0;
            ptr1 = strtok_r(line, " ", &ptr2);
            while((ptr1) && (i < MAX_PORTS))
            {
                strncpy(mcpd_router.max_group_port_list[i].brPortName, ptr1, IFNAMSIZ);
                ptr1 = strtok_r(NULL, ",", &ptr2);                
                if(!ptr1) {
                  mcpd_router.max_group_port_list[i].brPortName[0] = '\0';
                  break;
                }
                mcpd_router.max_group_port_list[i].maxGroupsForPort = strtoul(ptr1, NULL, 0);
                ptr1 = strtok_r(NULL, " ", &ptr2);

                port_ifi = if_nametoindex(mcpd_router.max_group_port_list[i].brPortName);
                if (0 == port_ifi) 
                {
                  MCPD_TRACE(MCPD_TRC_ERR, "Unable to get ifindex for %s", mcpd_router.max_group_port_list[i].brPortName);
                  mcpd_router.max_group_port_list[i].brPortName[0] = '\0';
                  mcpd_router.max_group_port_list[i].maxGroupsForPort= 0;
                } 
                else 
                {
                  MCPD_TRACE(MCPD_TRC_INFO, "%s converted ifindex %d", mcpd_router.max_group_port_list[i].brPortName, port_ifi);
                  mcpd_router.max_group_port_list[i].port_ifi = port_ifi;
                  i++;
                }
            }
        }

#ifdef SUPPORT_MLD
        if(strcasecmp(token, "mld-default-version") == 0)
        {
            mld_config->default_version = strtoul(line, NULL, 0);
        }
        if(strcasecmp(token, "mld-query-interval") == 0)
        {
            mld_config->query_interval = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-query-response-interval") == 0)
        {
            mld_config->query_resp_interval = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-last-member-query-interval") == 0)
        {
            mld_config->lmqi = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-robustness-value") == 0)
        {
            mld_config->robust_val = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-max-groups") == 0)
        {
            mld_config->max_groups = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-max-sources") == 0)
        {
            mld_config->max_sources = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-max-members") == 0)
        {
            mld_config->max_members = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-fast-leave") == 0)
        {
            mld_config->fast_leave_enable = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-admission-required") == 0)
        {
            mld_config->admission_required = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-admission-bridging-filter") == 0)
        {
            mld_config->admission_bridging_filter= strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-flood-enable") == 0)
        {
            mld_config->flood_enable = strtoul(line, NULL, 0);
        }
        else if(strcasecmp(token, "mld-proxy-interfaces") == 0)
        {
            i = 0;
            ptr1 = strtok_r(line, " ", &ptr2);
            while((ptr1) && (i < MCPD_MAX_IFS))
            {
                strncpy(mcpd_mld_upstream_interface[i], ptr1, IFNAMSIZ);
                i++;
                ptr1 = strtok_r(NULL, " ", &ptr2);
                if(!ptr1) break;
            }
        }
        else if(strcasecmp(token, "mld-snooping-interfaces") == 0)
        {
            i = 0;
            ptr1 = strtok_r(line, " ", &ptr2);
            while((ptr1) && (i < MCPD_MAX_IFS))
            {
                strncpy(mcpd_mld_downstream_interface[i], ptr1, IFNAMSIZ);
                i++;
                ptr1 = strtok_r(NULL, " ", &ptr2);
                if(!ptr1) break;
            }
        }
        else if(strcasecmp(token, "mld-mcast-interfaces") == 0)
        {
            i = 0;
            ptr1 = strtok_r(line, " ", &ptr2);
            while((ptr1) && (i < MCPD_MAX_IFS))
            {
                strncpy(mcpd_mld_mcast_interface[i], ptr1, IFNAMSIZ);
                i++;
                ptr1 = strtok_r(NULL, " ", &ptr2);
                if(!ptr1) break;
            }
        }
        else if(strcasecmp(token, "mld-mcast-snoop-exceptions") == 0)
        {

            ptr1 = strtok_r(line, " ", &ptr2);
            while(ptr1)
            {
                char *slashPtr = NULL;
                int result = 0;
                struct in6_addr exceptionAddress;
                struct in6_addr maskAddress;
                int maskFound = 0;
                
                MCPD_TRACE(MCPD_TRC_INFO, "MCPD SnoopException Rule - %s",ptr1);
                slashPtr = strchr(ptr1, '/');
                if (slashPtr)
                {
                    maskFound = inet_pton (AF_INET6, slashPtr+1, &maskAddress);
                    slashPtr[0] = '\0';
                }
                if (1 != maskFound)
                {
                    memset (&maskAddress, 0xff, 16);
                }
                result = inet_pton (AF_INET6, ptr1, &exceptionAddress);

                if (( 1 == result ) && 
                    ( (maskAddress.s6_addr32[0]      & htonl(0xFF000000)) == htonl(0xFF000000) )&&
                    ( (exceptionAddress.s6_addr32[0] & htonl(0xFF000000)) == htonl(0xFF000000) ) ) 
                {
                    // Add to linked list
                    exceptionObj = (t_MCPD_FILTER_EXCEPTION *) MCPD_ALLOC(MCPD_EXCEPTION_OBJ, sizeof(t_MCPD_FILTER_EXCEPTION));
                    if (NULL == exceptionObj)
                    {
                        MCPD_TRACE(MCPD_TRC_ERR, "MCPD SnoopException Rule - MEMORY FAIL");
                    }
                    else
                    {
                        // Insertion at head of list
                        memcpy (&exceptionObj->address, &exceptionAddress, 16);
                        memcpy (&exceptionObj->mask,    &maskAddress, 16);
                        exceptionObj->next = mcpd_router.mld_config.filter_list;
                        mcpd_router.mld_config.filter_list = exceptionObj;
                    }
                }
                else
                {
                    MCPD_TRACE(MCPD_TRC_ERR, "MCPD SnoopException Rule REJECTED");
                }
                ptr1 = strtok_r(NULL, " ", &ptr2);
            }  
        }

#endif /* SUPPORT_MLD */
    }
    fclose(fp);

    mcpd_config_validate (igmp_config, mld_config);
    mcpd_config_to_kernel(igmp_config, mld_config);
      
    mcpd_config_apply_igmp_version(igmp_config->default_version);
#ifdef SUPPORT_MLD
    mcpd_config_apply_mld_version(mld_config->default_version);
#endif /* SUPPORT_MLD */

    if(igmp_config->query_interval)
        igmp_config->startup_query_interval = (igmp_config->query_interval/4);
    igmp_config->startup_query_count    = igmp_config->robust_val;

    return 0;
} /* mcpd_config_read*/

void mcpd_config_display(t_MCPD_IGMP_CONFIG *igmp_config,
                         t_MCPD_MLD_CONFIG *mld_config)
{
    int i;
    t_MCPD_FILTER_EXCEPTION* currentIgmpFilter = mcpd_router.igmp_config.filter_list;
#ifdef SUPPORT_MLD
    t_MCPD_FILTER_EXCEPTION* currentMldFilter = mcpd_router.mld_config.filter_list;
#endif

    printf("\nIGMP Configuration \n");
    printf("default_version             = %d\n", igmp_config->default_version);
    printf("query_interval              = %d\n", igmp_config->query_interval);
    printf("query_resp_interval         = %d\n", igmp_config->query_resp_interval);
    printf("lmqi                        = %d\n", igmp_config->lmqi);
    printf("robustness value            = %d\n", igmp_config->robust_val);
    printf("max_groups                  = %d\n", igmp_config->max_groups);
    printf("max_sources                 = %d\n", igmp_config->max_sources);
    printf("max_members                 = %d\n", igmp_config->max_members);
    printf("fast_leave_enable           = %d\n", igmp_config->fast_leave_enable);
    printf("flood_enable                = %d\n", igmp_config->flood_enable);
    printf("admission required          = %d\n", igmp_config->admission_required);
    printf("admission bridging filter   = %d\n", igmp_config->admission_bridging_filter);

    printf("igmp proxy interfaces:");
    for (i = 0 ; i < MCPD_MAX_IFS; i++)
    {
        if (strcmp(mcpd_igmp_upstream_interface[i], ""))
        {
             printf(" %s", mcpd_igmp_upstream_interface[i]);
        }
    }
    printf("\n");

    printf("igmp snooping interfaces:");
    for (i = 0 ; i < MCPD_MAX_IFS; i++)
    {
        if (strcmp(mcpd_igmp_downstream_interface[i], ""))
        {
             printf(" %s", mcpd_igmp_downstream_interface[i]);
        }
    }
    printf("\n");

    printf("IPv4 multicast source interfaces:");
    for (i = 0 ; i < MCPD_MAX_IFS; i++)
    {
        if (strcmp(mcpd_igmp_mcast_interface[i], ""))
        {
             printf(" %s", mcpd_igmp_mcast_interface[i]);
        }
    }
    printf("\n");

    printf("igmp multicast snooping exceptions:");
    while (currentIgmpFilter)
    {
        printf(" %d.%d.%d.%d/%d.%d.%d.%d", 
               currentIgmpFilter->address.s6_addr[0],
               currentIgmpFilter->address.s6_addr[1],
               currentIgmpFilter->address.s6_addr[2],
               currentIgmpFilter->address.s6_addr[3],
               currentIgmpFilter->mask.s6_addr[0],
               currentIgmpFilter->mask.s6_addr[1],
               currentIgmpFilter->mask.s6_addr[2],
               currentIgmpFilter->mask.s6_addr[3]);
        currentIgmpFilter = currentIgmpFilter->next;
    }
    printf("\n");

#ifdef SUPPORT_MLD
    if(mld_config)
    {
        printf("\nMLD Configuration \n");
        printf("default_version             = %d\n", mld_config->default_version);
        printf("query_interval              = %d\n", mld_config->query_interval);
        printf("query_resp_interval         = %d\n", mld_config->query_resp_interval);
        printf("lmqi                        = %d\n", mld_config->lmqi);
        printf("robustness value            = %d\n", mld_config->robust_val);
        printf("max_groups                  = %d\n", mld_config->max_groups);
        printf("max_sources                 = %d\n", mld_config->max_sources);
        printf("max_members                 = %d\n", mld_config->max_members);
        printf("fast_leave_enable           = %d\n", mld_config->fast_leave_enable);
        printf("flood_enable                = %d\n", mld_config->flood_enable);
        printf("admission required          = %d\n", mld_config->admission_required);
        printf("admission bridging filter   = %d\n", mld_config->admission_bridging_filter);
    
        printf("mld proxy interfaces:");
        for (i = 0 ; i < MCPD_MAX_IFS; i++)
        {
            if (strcmp(mcpd_mld_upstream_interface[i], ""))
            {
                 printf(" %s", mcpd_mld_upstream_interface[i]);
            }
        }
        printf("\n");
    
        printf("mld snooping interfaces:");
        for (i = 0 ; i < MCPD_MAX_IFS; i++)
        {
            if (strcmp(mcpd_mld_downstream_interface[i], ""))
            {
                 printf(" %s", mcpd_mld_downstream_interface[i]);
            }
        }
        printf("\n");
    
        printf("IPv6 multicast source interfaces:");
        for (i = 0 ; i < MCPD_MAX_IFS; i++)
        {
            if (strcmp(mcpd_mld_mcast_interface[i], ""))
            {
                 printf(" %s", mcpd_mld_mcast_interface[i]);
            }
        }
        printf("\n");
        printf("mld multicast snooping exceptions:");
        while (currentMldFilter)
        {
            printf(" %04x.%04x.%04x.%04x.%04x.%04x.%04x.%04x/%04x.%04x.%04x.%04x.%04x.%04x.%04x.%04x", 
                   htons(currentMldFilter->address.s6_addr16[0]),
                   htons(currentMldFilter->address.s6_addr16[1]),
                   htons(currentMldFilter->address.s6_addr16[2]),
                   htons(currentMldFilter->address.s6_addr16[3]),
                   htons(currentMldFilter->address.s6_addr16[4]),
                   htons(currentMldFilter->address.s6_addr16[5]),
                   htons(currentMldFilter->address.s6_addr16[6]),
                   htons(currentMldFilter->address.s6_addr16[7]),
                   htons(currentMldFilter->mask.s6_addr16[0]),
                   htons(currentMldFilter->mask.s6_addr16[1]),
                   htons(currentMldFilter->mask.s6_addr16[2]),
                   htons(currentMldFilter->mask.s6_addr16[3]),
                   htons(currentMldFilter->mask.s6_addr16[4]),
                   htons(currentMldFilter->mask.s6_addr16[5]),
                   htons(currentMldFilter->mask.s6_addr16[6]),
                   htons(currentMldFilter->mask.s6_addr16[7])
                   );
            currentMldFilter = currentMldFilter->next;
        }
        printf("\n");
    }

#endif /* SUPPORT_MLD */

    printf("\nBridge port multicast group limits:");
    for (i = 0 ; i < MAX_PORTS; i++)
    {
        if (strcmp(mcpd_router.max_group_port_list[i].brPortName, ""))
        {
            printf(" %s(%d),", mcpd_router.max_group_port_list[i].brPortName, mcpd_router.max_group_port_list[i].maxGroupsForPort);
        }
        else
        {
            break;
        }
    }
    printf("\n");


    return;
} /* mcpd_config_display */
