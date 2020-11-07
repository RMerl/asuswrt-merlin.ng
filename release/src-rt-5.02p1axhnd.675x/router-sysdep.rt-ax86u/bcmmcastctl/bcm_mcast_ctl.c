/***********************************************************************
 *
 *  Copyright (c) 2015  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2015:proprietary:standard
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
 * :>
 *
 ***********************************************************************/

/*
 * BCM MCAST control utility
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <ctype.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <getopt.h>
#include <bcm_mcast_api.h>
#include "bcm_mcast_ctl.h"

static int bcm_mcast_ctl_get_ifindex(char *popt)
{
   FILE         *fp;
   unsigned int  ifi; 
   char          path[64];
   char         *str;
   char         *endptr;

   errno = 0;
   str = popt;
   ifi = strtol(str, &endptr, 10);
   if ( (errno != 0) || (endptr == str))
   {
      /* no digits were found */
      ifi = 0;
      snprintf(path, 64, "/sys/class/net/%s/ifindex", popt);
      fp = fopen(path, "r");
      if (fp) {
         int rc = fscanf(fp, "%d", &ifi);
         fclose(fp);
         if (rc != 1)
         {
            ifi = 0;
         }
      }
   }
   return ifi;
}

static int bcm_mcast_ctl_get_proto(char *popt)
{
   int   proto;
   char *str;
   char *endptr;

   errno = 0;
   str = popt;
   proto = strtol(str, &endptr, 10);
   if ( (errno != 0) || (endptr == str))
   {
      /* no digits were found */
      if ( popt[0] == 'i' )
      {
         proto = BCM_MCAST_PROTO_IPV4;
      }
      else if ( popt[0] == 'm' )
      {
         proto = BCM_MCAST_PROTO_IPV6;
      }  
      else
      {
         proto = -1;
      }
   }
   return proto;
}

static int bcm_mcast_ctl_get_mode(char *popt)
{
   int   mode;
   char *str;
   char *endptr;

   errno = 0;
   str = popt;
   mode = strtol(str, &endptr, 10);
   if ( (errno != 0) || (endptr == str))
   {
      /* no digits were found */
      if ( popt[0] == 'd' )
      {
         mode = BCM_MCAST_SNOOPING_DISABLED_FLOOD;
      }
      else if ( popt[0] == 's' )
      {
         mode = BCM_MCAST_SNOOPING_STANDARD_MODE;
      } 
      else if ( popt[0] == 'b' )
      {
         mode = BCM_MCAST_SNOOPING_BLOCKING_MODE;
      } 
      else
      {
         mode = -1;
      }
   }
   return mode;
}

static int bcm_mcast_ctl_get_enable(char *popt)
{
   int   enable;
   char *str;
   char *endptr;

   errno = 0;
   str = popt;
   enable = strtol(str, &endptr, 10);
   if ( (errno != 0) || (endptr == str))
   {
      /* no digits were found */
      if ( popt[0] == 'e' )
      {
         enable = BCM_MCAST_LAN2LAN_SNOOPING_ENABLE;
      }
      else if ( popt[0] == 'd' )
      {
         enable = BCM_MCAST_LAN2LAN_SNOOPING_DISABLE;
      } 
      else
      {
         enable = -1;
      }
   }
   return enable;
}

static int bcm_mcast_ctl_show_config(void *pCmd __attribute__((unused)), int argc, char *argv[])
{
   int    c;
   char  *optif = NULL;
   char  *optproto = NULL;
   int    ifi;
   int    proto;
   int    l2l = 0;
   int    mode = 0;
   int    rate = -1;
   
   while ( 1 )
   {
      static struct option long_options[] =
      {
         {"interface", required_argument, NULL, 'i'},
         {"proto",     required_argument, NULL, 'p'},
         { 0, 0, 0, 0 }
      };
      
      c = getopt_long(argc, argv, "i:p:", long_options, 0);
      if ( c < 0 )
      {
         break;
      }
      
      switch (c)
      {
         case 'i':
            optif = optarg; 
            break;

         case 'p': 
            optproto = optarg; 
            break;
      }
   }

   if ( optif )
   {
      ifi = bcm_mcast_ctl_get_ifindex(optif);
      if ( 0 == ifi )
      {
         printf("invalid interface specified %s\n", optif);
         return -1;
      }
   }
   else
   {
      ifi = 0;
   }

   if ( optproto )
   {
      proto = bcm_mcast_ctl_get_proto(optproto);
      if ( (0 == proto) ||
           ((proto & BCM_MCAST_PROTO_ALL) != proto) )
      {
         printf("Invalid protocol specified\n");
         return -1;
      }
   }
   else
   {
      proto = BCM_MCAST_PROTO_ALL;
   }

   if ( ifi )
   {
      if ( proto & BCM_MCAST_PROTO_IPV4 )
      {
         if ( bcm_mcast_api_get_snooping_cfg(ifi, BCM_MCAST_PROTO_IPV4, &mode, &l2l) < 0 )
         {
            return -1;
         }
         if ( bcm_mcast_api_get_proto_rate_limit(ifi, BCM_MCAST_PROTO_IPV4, &rate) < 0 )
         {
            return -1;
         }
         printf("Interface %s: IGMP snooping mode %d, L2L enable %d, rate limit %d\n", optif, mode, l2l, rate);
      }
      if ( proto & BCM_MCAST_PROTO_IPV6 )
      {
         if ( bcm_mcast_api_get_snooping_cfg(ifi, BCM_MCAST_PROTO_IPV6, &mode, &l2l) < 0 )
         {
            return -1;
         }
         if ( bcm_mcast_api_get_proto_rate_limit(ifi, BCM_MCAST_PROTO_IPV6, &rate) < 0 )
         {
            return -1;
         }
         printf("Interface %s: MLD  snooping mode %d, L2L enable %d, rate limit %d\n", optif, mode, l2l, rate);
      }
   }
   else
   {
      DIR           *dp;
      struct dirent *ep;
      char           path[1024];
      int            rc;
      struct stat    st;

      dp = opendir("/sys/class/net/");
      if (dp != NULL)
      {
         while ( 1 )
         {
            ep = readdir(dp);
            if ( NULL == ep )
            {
               break;
            }
            
            if ( ep->d_name[0] == '.' )
            {
               continue;
            }

            snprintf(path, sizeof(path), "/sys/class/net/%s/bridge", ep->d_name);
            rc = stat(path, &st);
            if ((0 != rc) || !S_ISDIR(st.st_mode))
            {
               continue;
            }
            else
            {
               ifi = bcm_mcast_ctl_get_ifindex(ep->d_name);
               if ( ifi )
               {
                  if ( proto & BCM_MCAST_PROTO_IPV4 )
                  {
                     if ( bcm_mcast_api_get_snooping_cfg(ifi, BCM_MCAST_PROTO_IPV4, &mode, &l2l) < 0 )
                     {
                        continue;
                     }
                     if ( bcm_mcast_api_get_proto_rate_limit(ifi, BCM_MCAST_PROTO_IPV4, &rate) < 0 )
                     {
                        return -1;
                     }
                     printf("Interface %s: IGMP snooping mode %d, L2L enable %d, rate limit %d\n", ep->d_name, mode, l2l, rate);
                  }
                  if ( proto & BCM_MCAST_PROTO_IPV6 )
                  {
                     if ( bcm_mcast_api_get_snooping_cfg(ifi, BCM_MCAST_PROTO_IPV6, &mode, &l2l) < 0 )
                     {
                        continue;
                     }
                     if ( bcm_mcast_api_get_proto_rate_limit(ifi, BCM_MCAST_PROTO_IPV6, &rate) < 0 )
                     {
                        return -1;
                     }
                     printf("Interface %s: MLD  snooping mode %d, L2L enable %d, rate limit %d\n", ep->d_name, mode, l2l, rate);
                  }
               }
            }
         }
         closedir(dp);
      }     
   }
   return 0;
}

static int bcm_mcast_ctl_set_snoop_mode(void *pCmd __attribute__((unused)), int argc, char *argv[])
{
   int    c;
   char  *optif = NULL;
   char  *optproto = NULL;
   char  *optmode = NULL;
   int    ifi;
   int    proto;
   int    mode;
   int    l2l;
   int    tmp;
   
   while ( 1 )
   {
      static struct option long_options[] =
      {
         {"interface", required_argument, NULL, 'i'},
         {"proto",     required_argument, NULL, 'p'},
         {"mode",      required_argument, NULL, 'm'},            
         { 0, 0, 0, 0 }
      };
      
      c = getopt_long(argc, argv, "i:p:m:", long_options, 0);
      if ( c < 0 )
      {
         break;
      }
      
      switch (c)
      {
         case 'i':
            optif = optarg; 
            break;

         case 'p': 
            optproto = optarg; 
            break;

         case 'm': 
            optmode = optarg; 
            break;
      }
   }

   if ( !optif || !optproto || !optmode )
   {
      return -1;
   }

   ifi = bcm_mcast_ctl_get_ifindex(optif);
   if ( 0 == ifi )
   {
      printf("invalid interface specified %s\n", optif);
      return -1;
   }

   proto = bcm_mcast_ctl_get_proto(optproto);
   if ( (0 == proto) ||
        ((proto & BCM_MCAST_PROTO_ALL) != proto) )
   {
      printf("Invalid protocol specified\n");
      return -1;
   }

   mode = bcm_mcast_ctl_get_mode(optmode);
   if ( (mode < BCM_MCAST_SNOOPING_DISABLED_FLOOD) || 
        (mode > BCM_MCAST_SNOOPING_BLOCKING_MODE) )
   {
      return -1;
   }
   
   if ( bcm_mcast_api_get_snooping_cfg(ifi, proto, &tmp, &l2l) < 0 )
   {
      return -1;
   }

   if ( bcm_mcast_api_set_snooping_cfg(-1, ifi, proto, mode, l2l) < 0 )
   {
      return -1;
   }

   return 0;
}

static int bcm_mcast_ctl_set_l2l_enable(void *pCmd __attribute__((unused)), int argc, char *argv[])
{
   int    c;
   char  *optif = NULL;
   char  *optproto = NULL;
   char  *opten = NULL;
   int    ifi;
   int    proto;
   int    l2l;
   int    mode;
   int    tmp;
   
   while ( 1 )
   {
      static struct option long_options[] =
      {
         {"interface", required_argument, NULL, 'i'},
         {"proto",     required_argument, NULL, 'p'},
         {"enable",    required_argument, NULL, 'e'},            
         { 0, 0, 0, 0 }
      };
      
      c = getopt_long(argc, argv, "i:p:e:", long_options, 0);
      if ( c < 0 )
      {
         break;
      }
      
      switch (c)
      {
         case 'i':
            optif = optarg; 
            break;

         case 'p': 
            optproto = optarg; 
            break;

         case 'e': 
            opten = optarg; 
            break;
      }
   }

   if ( !optif || !optproto || !opten )
   {
      return -1;
   }

   ifi = bcm_mcast_ctl_get_ifindex(optif);
   if ( 0 == ifi )
   {
      printf("invalid interface specified %s\n", optif);
      return -1;
   }

   proto = bcm_mcast_ctl_get_proto(optproto);
   if ( (0 == proto) ||
        ((proto & BCM_MCAST_PROTO_ALL) != proto) )
   {
      printf("Invalid protocol specified\n");
      return -1;
   }

   l2l = bcm_mcast_ctl_get_enable(opten);
   if ( (l2l != BCM_MCAST_LAN2LAN_SNOOPING_DISABLE) && 
        (l2l != BCM_MCAST_LAN2LAN_SNOOPING_ENABLE) )
   {
      return -1;
   }
   
   if ( bcm_mcast_api_get_snooping_cfg(ifi, proto, &mode, &tmp) < 0 )
   {
      return -1;
   }

   if ( bcm_mcast_api_set_snooping_cfg(-1, ifi, proto, mode, l2l) < 0 )
   {
      return -1;
   }

   return 0;
}

static int bcm_mcast_ctl_set_proto_rl(void *pCmd __attribute__((unused)), int argc, char *argv[])
{
   int    c;
   char  *optif = NULL;
   char  *optproto = NULL;
   char  *optrate = NULL;
   int    ifi;
   int    proto;
   int    rate;
   char  *str;
   char  *endptr;
   
   while ( 1 )
   {
      static struct option long_options[] =
      {
         {"interface", required_argument, NULL, 'i'},
         {"proto",     required_argument, NULL, 'p'},
         {"rate",      required_argument, NULL, 'r'},            
         { 0, 0, 0, 0 }
      };
      
      c = getopt_long(argc, argv, "i:p:r:", long_options, 0);
      if ( c < 0 )
      {
         break;
      }
      
      switch (c)
      {
         case 'i':
            optif = optarg; 
            break;

         case 'p': 
            optproto = optarg; 
            break;

         case 'r': 
            optrate = optarg; 
            break;
      }
   }

   if ( !optif || !optproto || !optrate )
   {
      return -1;
   }

   ifi = bcm_mcast_ctl_get_ifindex(optif);
   if ( 0 == ifi )
   {
      printf("invalid interface specified %s\n", optif);
      return -1;
   }

   proto = bcm_mcast_ctl_get_proto(optproto);
   if ( (0 == proto) ||
        ((proto & BCM_MCAST_PROTO_ALL) != proto) )
   {
      printf("Invalid protocol specified\n");
      return -1;
   }

   errno = 0;
   str = optrate;
   rate = strtol(str, &endptr, 10);
   if ( (errno != 0) || (endptr == str))
   {
      /* no digits were found */
      return -1;
   }

   if ( rate > BCM_MCAST_PROTO_RATE_LIMIT_MAX )
   {
      return -1;
   }

   if ( bcm_mcast_api_set_proto_rate_limit(-1, ifi, proto, rate) < 0 )
   {
      return -1;
   }

   return 0;
}

static int bcm_mcast_ctl_blog_enable(void *pCmd __attribute__((unused)), int argc, char *argv[])
{
   int    c;
   char  *opten = NULL;
   int    enable;

   while ( 1 )
   {
      static struct option long_options[] =
      {
         {"enable", required_argument, NULL, 'e'},
         { 0, 0, 0, 0 }
      };
      
      c = getopt_long(argc, argv, "e:", long_options, 0);
      if ( c < 0 )
      {
         break;
      }
      
      switch (c)
      {
         case 'e':
            opten = optarg; 
            break;
      }
   }

   if ( !opten )
   {
      return -1;
   }

   enable = bcm_mcast_ctl_get_enable(opten);
   if ( (enable != 0) && (enable != 1) )
   {
      printf("invalid enable setting specified %s\n", opten);
      return -1;
   }

   if ( bcm_mcast_api_blog_enable(-1, enable) < 0 )
   {
      return -1;
   }

   return 0;
}

t_BCM_MCAST_CTL_CMD bcm_mcast_ctl_cmds[] = {
    {"show", "Show configuration:     [-i <ifindex or ifname>] [-p <igmp(1), mld(2)>]", BCM_MCAST_CTL_SHOW_CONFIG, bcm_mcast_ctl_show_config, 0},
    {"mode", "Set snooping mode:       -i <ifindex or name>     -p <igmp(1), mld(2)> -m <disabled(0), standard(1), blocking(2)>", BCM_MCAST_CTL_SET_MODE, bcm_mcast_ctl_set_snoop_mode, 3},
    {"l2l" , "Set L2L snooping:        -i <ifindex or name>     -p <igmp(1), mld(2)> -e <disable(0), enable(1)>", BCM_MCAST_CTL_SET_L2L, bcm_mcast_ctl_set_l2l_enable, 3},
    {"rate", "Set protocol rate limit: -i <ifindex or name>     -p <igmp(1), mld(2)> -r <0 to 500>", BCM_MCAST_CTL_SET_PROTO_RL, bcm_mcast_ctl_set_proto_rl, 3},
    {"blog", "Enable/disable blog:     -e <disable(0), enable(1)>", BCM_MCAST_CTL_BLOG_EN, bcm_mcast_ctl_blog_enable, 1}
};

static void bcm_mcast_ctl_show_usage(char *prog_name)
{
    unsigned int i;
    printf("Usage: %s <option>\n", prog_name);
    for (i = 0; i < (sizeof(bcm_mcast_ctl_cmds)/sizeof(bcm_mcast_ctl_cmds[0])); i++) {
        printf("        %s  -  %s\n", bcm_mcast_ctl_cmds[i].cmdstr, bcm_mcast_ctl_cmds[i].description);
    }
}

t_BCM_MCAST_CTL_CMD *bcm_mcast_ctl_cmd_looup(const char *cmdName)
{
    unsigned int i;
    for (i = 0; i < (sizeof(bcm_mcast_ctl_cmds)/sizeof(bcm_mcast_ctl_cmds[0])); i++) {
        if (!strcmp(cmdName, bcm_mcast_ctl_cmds[i].cmdstr)) {
            return &bcm_mcast_ctl_cmds[i];
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
   t_BCM_MCAST_CTL_CMD *pCmd;

   if (argc == 1) {
     bcm_mcast_ctl_show_usage(argv[0]);
     return -1;
   }

   pCmd = bcm_mcast_ctl_cmd_looup(argv[1]);
   if (pCmd == NULL) {
      bcm_mcast_ctl_show_usage(argv[0]);
      return -1;
   }

   /* nargs is the minimum number of args 
      0xFFFF will mean "any number of arguments */
   if ((unsigned int) argc < (pCmd->nargs + 2))
   {
      printf("incorrect number of arguments\n");
      bcm_mcast_ctl_show_usage(argv[0]);
      return -1;
   }

   /* found a match - call handler */
   pCmd->func((void *)pCmd, argc, argv);

   return 0;
}

